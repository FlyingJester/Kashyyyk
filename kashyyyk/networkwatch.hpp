#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include "monitor.hpp"
#include "background.hpp"
#include "socket.h"
#include "poll.h"

namespace Kashyyyk {

class NetworkWatch {

    static void ThreadFunction(NetworkWatch *that);

    std::atomic<bool> live;
    Monitor monitor;

    struct SocketSet *socket_set;
    std::thread thread;



public:
    NetworkWatch(std::shared_ptr<struct Monitor::MutexHolder> &mutex);
    NetworkWatch(Thread::TaskGroup *group);
    ~NetworkWatch();

    void AddSocket(WSocket *socket);
    void DelSocket(WSocket *socket);

};


}
