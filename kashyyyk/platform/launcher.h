#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    struct PlatformLauncher_Wrap;

    struct PlatformLauncher_Wrap *Kashyyyk_AllocLauncher();
    void Kashyyyk_InitLauncher(struct PlatformLauncher_Wrap *, void *launcher);
    void Kashyyyk_CloseLauncher(struct PlatformLauncher_Wrap *);
    void Kashyyyk_DestroyLauncher(struct PlatformLauncher_Wrap *);

    void Kashyyyk_LauncherNewWindow(void *);
    void Kashyyyk_LauncherDirectConnect(void *);
    void Kashyyyk_LauncherJoinChannel(void *);
    void Kashyyyk_LauncherServerList(void *);
    void Kashyyyk_LauncherPreferences(void *);
    void Kashyyyk_LauncherQuit(void *);


#ifdef __cplusplus
}
#endif

typedef void Launcher;

#ifdef __cplusplus
extern "C"
#endif
Launcher *CreateLauncher(void /*Thread::TaskGroup*/ *);
