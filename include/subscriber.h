#ifndef SubscriberCEG
#define SubscriberCEG

#include "utility.h"
#include <iostream>
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
//#include <Mq.h>

namespace ceg4350 {

    class Subscriber : public Utility{
    public:
        Subscriber();

        static DWORD WINAPI staticThreadFunc(LPVOID lpParam);
        bool isDone() {return processDone; }

    protected:

        bool pullSharedMem();//use semaphores
        bool pullNamedPipe();
        bool pullMailBox();
        
    private:
        void clearDataRepo();
        
        
    };

}

#endif
