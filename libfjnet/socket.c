#include "socket.h"
#include <assert.h>
#include <errno.h>

/*
enum WSockErr {eSuccess, eFailure, eNotConnected, eAlreadyConnected};
*/
const char *ExplainError_Socket(enum WSockErr err){
    switch (err){
        case eSuccess:
            return "No Error.";
        case eFailure:
            return "Unkown Error.";
        case eNotConnected:
            return "Not Connected.";
        case eRefused:
            return "Connection Refused.";
        case eTimeout:
            return "Connection Timed Out.";
        case eAlreadyConnected:
            return "Already Connected.";
        default:
        ;
    }
    return "Bad Error Value";
}

#if !(defined USE_BSDSOCK) && !(defined USE_WINSOCK) && !(defined USE_CYGSOCK)
#error No sutiable socket backend.
#endif

#if defined(USE_BSDSOCK)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

typedef int FJNET_SOCKET;

void InitSock(){}

#define MakeNonBlocking(S) fcntl(S, F_SETFL, O_NONBLOCK)

#define CLOSE_SOCKET close

#define PRINT_LAST_ERROR perror

static int GetPendingBytes(FJNET_SOCKET socket, unsigned long *len){
    unsigned int llen = sizeof(unsigned long);
	return getsockopt(socket, SOL_SOCKET, SO_NREAD, len, &llen);
}

#elif defined (USE_WINSOCK)

#include <Winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef SOCKET FJNET_SOCKET;

static void WSACleanupWrapper_Local(void){
	WSACleanup();
}

static void InitSock(){
    static int Inited = 0;
	if(!Inited){
		Inited = 1;
	    WSADATA data;
		WSAStartup(MAKEWORD(2,2), &data);
		atexit(WSACleanupWrapper_Local);
	}
}

static void MakeNonBlocking(FJNET_SOCKET socket){
	unsigned long m=1;
	ioctlsocket(socket, FIONBIO, &m);
}

#define PRINT_LAST_ERROR(STR) printf(STR " %ld\n", WSAGetLastError())

#define CLOSE_SOCKET(S) shutdown(S, SD_SEND); closesocket(S)

static int GetPendingBytes(FJNET_SOCKET socket, unsigned long *len){
	return ioctlsocket(socket, FIONREAD, len);
}


#elif defined (USE_CYGSOCK)
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

typedef int FJNET_SOCKET;

void InitSock(){}

const int SO_NREAD = 0x1020;

#define MakeNonBlocking(S) fcntl(S, F_SETFL, O_NONBLOCK)

#define CLOSE_SOCKET close

#define PRINT_LAST_ERROR perror

static int GetPendingBytes(FJNET_SOCKET socket, unsigned long *len){
	struct pollfd pfd;
	{
		pfd.fd = socket;
		pfd.events = 0;

		poll(&pfd, 1, 10);
	}
	{
		int n, err = ioctl(socket, FIONREAD, &n);
		*len = n;
		return err;
	}
}

#endif

struct WSocket{
    char hostname[0xFF];
    struct sockaddr_in *sockaddr;
    struct hostent *host;
    FJNET_SOCKET sock;
};

struct WSocket *Create_Socket(void){

    struct WSocket *lSock = malloc(sizeof(struct WSocket));
    lSock->sockaddr = malloc(sizeof(struct sockaddr_in));
    memset(lSock->sockaddr, 0, sizeof(struct sockaddr_in));
    return lSock;
}

void Destroy_Socket(struct WSocket *aSocket){

    assert(aSocket);

    free(aSocket->sockaddr);
    free(aSocket);
}

enum WSockErr Connect_Socket(struct WSocket *aSocket, const char *aTo, unsigned long aPortNum, long timeout){

    int err = 0;
    struct in_addr *lAddr = NULL;
    assert(aTo!=NULL);
    assert(aSocket!=NULL);

	InitSock();

    {
        unsigned long len = strlen(aTo);
        assert(len<=0xFE);
        strncpy(aSocket->hostname, aTo, 0xFE);
        printf("Connection to %s\n", aSocket->hostname);
    }

    aSocket->host = gethostbyname(aSocket->hostname);
    if(!(aSocket->host))
      return eFailure;

    aSocket->sockaddr->sin_family = AF_INET;
    aSocket->sockaddr->sin_port = htons(aPortNum);


    lAddr = (void *)(aSocket->host->h_addr_list[0]);
    aSocket->sockaddr->sin_addr.s_addr = inet_addr(inet_ntoa(*lAddr));

    aSocket->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(
#if defined USE_WINSOCK
	(aSocket->sock==INVALID_SOCKET)
#elif defined USE_BSDSOCK || defined USE_CYGSOCK
	(aSocket->sock<0)
#endif
	){

        PRINT_LAST_ERROR("Error Creating Socket");
        return eFailure;
    }

    if(timeout>=0)
      MakeNonBlocking(aSocket->sock);

    err = connect(aSocket->sock, (const void *)aSocket->sockaddr, sizeof(struct sockaddr_in));

    if(timeout<0)
      MakeNonBlocking(aSocket->sock);

    if((timeout>=0)
#if defined USE_WINSOCK
	&& (WSAGetLastError()==WSAEWOULDBLOCK)
#elif defined USE_BSDSOCK
	&& (errno==EINPROGRESS)
#endif
	){
        struct timeval time;
        fd_set set;
        fprintf(stderr, "Waiting for connection...\n");

        FD_ZERO(&set);
        FD_SET(aSocket->sock,&set);

        time.tv_sec=timeout/1000;
        time.tv_usec=(timeout%1000)*1000;

        err = select(aSocket->sock+1, NULL, &set, NULL, &time);
        fprintf(stderr, "err %i\n", err);
        if(err==0){
            CLOSE_SOCKET(aSocket->sock);
            return eTimeout;
        }
    }


    if(err<0){

		PRINT_LAST_ERROR("Error occured attempting to connect");

        CLOSE_SOCKET(aSocket->sock);

        switch(errno){
        case ECONNREFUSED:
          return eRefused;
        case EISCONN:
          return eAlreadyConnected;
        }

        return eFailure;
    }

    printf("Connected to %s on port number %lu. Using socket %i.\n", aTo, aPortNum, aSocket->sock);

    return eSuccess;
}

enum WSockErr Disconnect_Socket(struct WSocket *aSocket){

    assert(aSocket!=NULL);
    if(aSocket->sock){
		InitSock();
        CLOSE_SOCKET(aSocket->sock);
        aSocket->sock = 0;
    }
    return eSuccess;
}

/* char streams are NUL terminated. */
enum WSockErr Read_Socket(struct WSocket *aSocket, char **aTo){

    unsigned long l = Length_Socket(aSocket);

    assert(aSocket!=NULL);
    assert(aSocket->sock!=0);

    *aTo = realloc(*aTo, l+1);
    if(!(*aTo)){
        *aTo = NULL;
        return eFailure;
    }
    if(l!=0){
		InitSock();
		if(recv(aSocket->sock, *aTo, l, 0)!=l){
			perror("Read_Socket failure");
			return eFailure;
			return eFailure;
		}
	}

    (*aTo)[l] = '\0';

    return eSuccess;
}

enum WSockErr Write_Socket(struct WSocket *aSocket, const char *aToWrite){

    long err = 0;
    unsigned long len;
    assert(aSocket!=NULL);
    assert(aSocket->sock!=0);
    assert(aToWrite!=NULL);

    len = strlen(aToWrite);

    if(len==0)
        return eSuccess;

	InitSock();
    err = send(aSocket->sock, aToWrite, strlen(aToWrite), 0);

    if(err<0){
		perror("Write_Socket failure");
		return eFailure;
	}
    return eSuccess;
}

/* Gets the number of pending bytes. This can increase at any time, so
 you should only trust this to see if there are any pending bytes at all.
*/
unsigned long Length_Socket(struct WSocket *aSocket){

    unsigned long len = 0;
    const unsigned int llen = sizeof(unsigned long);
    unsigned long f = 0;

	int r;

	InitSock();

    assert(aSocket!=NULL);
    assert(aSocket->sock!=0);

	r = GetPendingBytes(aSocket->sock, &len);

    if(r<0){
		perror("ioctl error in GetPendingBytes");
        return 0;
	}

    memcpy(&f, &len, llen);

    return f;
}
