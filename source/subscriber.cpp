#include "..\include\subscriber.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Winuser.h>

namespace ceg4350 {
    Subscriber::Subscriber()
    {
        for (int i = 0; i < DATA_ARRAY_SIZE; i++) {
            values[i] = 0;
        }
        processDone = false;
    }

    DWORD WINAPI Subscriber::staticThreadFunc(LPVOID lpParam) {
        const int cycles = 75;
        long int k = 0;

        Subscriber* subInstance = (Subscriber*)lpParam;
        if (subInstance) {
            bool tranferComplete = false;
            while (!tranferComplete) {
                //kick off each of the subscribers in order
                tranferComplete = subInstance->pullSharedMem();
                tranferComplete &= subInstance->pullMailBox();
                tranferComplete &= subInstance->pullNamedPipe();
            }
        }
        //print out a final listing of the data storage
        subInstance->reportGeneration("Done");
        //let main() know we are done by setting a flag
        subInstance->setProcessDone(true);
        return 0;
    }

    bool Subscriber::pullSharedMem()
    {
        //size of the shared mem space
        #define BUF_SIZE sizeof(RingIntBuffer)
        //ensure that Pub has had a chance to create the shared mem space
        Sleep(5000);
        TCHAR szName[] = TEXT("Local\\MyFileMappingObject");
        //windows API love to use generic HANDLE for everything
        HANDLE hMapFile;
        //defined in our parent class, utility.h
        RingIntBuffer* pBuf;

        //Windows API call that establishes connection to the shared memory space created by Pub
        hMapFile = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,   // read/write access
            FALSE,                 // do not inherit the name
            szName);               // name of mapping object

        if (hMapFile == NULL)
        {
            printf("Could not open file mapping object (%d).\n",
                GetLastError());
        }

        //cast the shared mem space to be a RingIntBuffer
        pBuf = (RingIntBuffer*)MapViewOfFile(hMapFile, // handle to map object
            FILE_MAP_ALL_ACCESS,  // read/write permission
            0,
            0,
            BUF_SIZE);
        
        if (pBuf == NULL)
        {
            CloseHandle(hMapFile);
            std::cout << " could not build buffer" << std::endl;
            return false;
        }

        wchar_t buffer[256];
        HANDLE ghSemaphore[10];
        //create a semaphore for each spot in the ring
        for (int i = 0; i < 10; i++) {
            wsprintfW(buffer, L"%d", i);            
            ghSemaphore[i] = CreateSemaphore(
                NULL,           // default security attributes
                1,  // initial count
                1,  // maximum count
                buffer);          // named semaphore
            
            if (ghSemaphore[i] == NULL)
            {
                printf("CreateSemaphore error: %d\n", GetLastError());
            }
        }

        int status = 0;
        while (status < 100) {
            int actionVal = -1;
            int nextIdx = pBuf->getNextPullIndex();
            if (nextIdx >= 0 && nextIdx < 10) {
                //if there is a collision (test is set up to force collisions
                //then wait on the semaphore
                WaitForSingleObject(ghSemaphore[nextIdx], INFINITE);
                //we are in, grab the integer
                actionVal = pBuf->getInt();
                //cause a collision by sleeping in the critical section, to prove it is working
                Sleep(20);
                //release the semaphore
                //note that i originally called closehandle() here, which caused a "non-blocking" like result, it was
                //obviously the wrong API call at this point in execution
                ReleaseSemaphore(ghSemaphore[nextIdx],1,NULL);
            }
            values[status++] = actionVal;
            std::cout << "&&&&& recv " << status << std::endl;
        }


        for (int i = 0; i < 10; i++) {
            CloseHandle(ghSemaphore[i]);
        }
        UnmapViewOfFile(pBuf);

        CloseHandle(hMapFile);

        reportGeneration("SharedMemory");
        clearDataRepo();
        return true;
    }

    bool Subscriber::pullNamedPipe() {

        HANDLE hPipe;
        DWORD dwRead;

        //the . in the CreateNamedPipe call is the name of the server
        //where the pipe is located.  This was not explained in the windows API documentation
        hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\PipeName"),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE |
            PIPE_READMODE_BYTE |
            PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            1024 * 16,//sizeof(values),
            1024 * 16,//sizeof(values),
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL);
        while (hPipe != INVALID_HANDLE_VALUE)
        {
            if (ConnectNamedPipe(hPipe, NULL) != FALSE)
            {
                //send it all in one message
                while (ReadFile(hPipe, values, sizeof(values), &dwRead, NULL) != FALSE)
                {
                    std::cout << "Pipe Has Received Data:" << std::endl;
                    reportGeneration("Pipe");
                    DisconnectNamedPipe(hPipe);
                    break;
                }
            }
            else {
                printf("connect pipe failed with error: %ld\n", WSAGetLastError());
            }
        }
        clearDataRepo();
        return true;
    }

    bool Subscriber::pullMailBox()
    {
        MSG msg;

        std::cout << "checking MailBox for message" << std::endl;
        BOOL bRet;
        while (bRet = GetMessage(&msg, NULL, 0, 0) != 0)
        {
            if (bRet == -1)
            {
                // handle the error 
                std::cout << "getmessage failed" << std::endl;
            }
            else
            {
                //another dirty data transfer implementation - just cast the memory into ints and save it off
                int* intPtr = (int*)msg.wParam;
                for (int i = 0; i < 100; i++) {
                    values[i] = *(intPtr + i);
                }
                break;
            }
        }
        reportGeneration("Mailbox");
        clearDataRepo();
        return true;
    }
    
    void Subscriber::clearDataRepo() {
        //reset the data repo for the next transfer IPC
        for (int i = 0; i < DATA_ARRAY_SIZE; i++) {
            values[i] = 0;
        }
    }

}
