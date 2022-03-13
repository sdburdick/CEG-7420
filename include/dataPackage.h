#include <iostream>
#include <Windows.h>
#define RECEIVE_ARRAY_SIZE 100

namespace ceg4350 {

    class Subscriber {
    public:
        Subscriber();
        void threadStartLoop();

        static DWORD WINAPI staticThreadFunc(LPVOID lpParam);
        bool isDone() {return processDone; }

    protected:
        bool pullSocket();
        bool pullSharedMem();//use semaphores
        bool pullPipe();
        bool pullMailBox();


        void reportValues();

    private:
        int values[RECEIVE_ARRAY_SIZE];
        bool processDone;
    };

}

