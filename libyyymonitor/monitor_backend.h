#pragma once

extern "C" {


    Kashyyyk::Monitor::Mutex *Create_RawMonitor();

    void Destroy_RawMonitor(Kashyyyk::Monitor::Mutex *m);

    void Lock_RawMonitor(Kashyyyk::Monitor::Mutex *m);
    void Unlock_RawMonitor(Kashyyyk::Monitor::Mutex *m);
    void Wait_RawMonitor(Kashyyyk::Monitor::Mutex *m);
    void Notify_RawMonitor(Kashyyyk::Monitor::Mutex *m);
    void NotifyAll_RawMonitor(Kashyyyk::Monitor::Mutex *m);

}
