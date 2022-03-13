#ifndef PublisherCEG
#define PublisherCEG

#include "utility.h"
#include <iostream>

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

namespace ceg4350 {

    class Publisher : public Utility{
    public:
        Publisher();

        static DWORD WINAPI staticThreadFunc(LPVOID lpParam);
        bool isDone() { return processDone; }
        void setSeed(int newSeed) {
            seed = newSeed;
        }

    protected:
        void pushSocket();
        void pushSharedMem();//use semaphores
        void pushNamedPipe();
        void pushMailBox();
        void generateValues();
    private:
        int seed;
    };
}
#endif
