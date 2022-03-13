#ifndef UtilityCEG
#define UtilityCEG


#include <iostream>
#include <winsock2.h>

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#define DATA_ARRAY_SIZE 100

namespace ceg7420 {
    //utility is just a shared inheritance class since i got tired of implementing things twice.
    //there was a lot of shared definitions between pub and the subs, it's all here.
    class Utility {
    public:
        Utility();

        bool isDone() { return processDone; }
        void setProcessDone(bool isDone) {
            processDone = isDone;
        };
        virtual void reportGeneration(const char* reporter);
        void setOtherThreadId(DWORD subThreadId) {
            otherThreadIdVal = subThreadId;
        }

    protected:
        DWORD otherThreadIdVal;
        int values[DATA_ARRAY_SIZE];
        bool processDone;
    
    };

    //RingIntBuffer is my implementation of the ring loop buffer.
    //at first i wanted the semaphores at this level, but ended up finding a better
    //path leaving them in the child classes
    class RingIntBuffer {
    public:
        RingIntBuffer();
        void initializeData();
        bool addInt(int n);
        int getInt();
        int getNextPullIndex() {
            return (lastPulled + 1) % 10;
        }
        int getNextPushIndex() {
            return (lastPushed + 1) % 10;
        }
    private:
        struct Storage {
            int storageInt;
            //needed a pair of data - the data int and whether it was placed or erased 
            bool valid;
        };
        //the 10 int/valid pairs
        Storage rBuf[10];
        //status trackers
        int lastPushed;
        int lastPulled;
    };
}

#endif // !UtilityCEG