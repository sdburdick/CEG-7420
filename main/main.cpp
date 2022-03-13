#include <iostream>
//mixing windows.h and ipcalls forces us to use this #define
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>//allows for access to system calls to start a thread
#undef WIN32_LEAN_AND_MEAN
#endif

#include "../include/subscriber.h"
#include "../include/netSubscriber.h"
#include "../include/publisher.h"

#include <shellapi.h>
#pragma comment(lib, "shell32.lib") //add shell32 to the linked libs

const char* subarg = "netSub";

int main(int argc, char* argv[]) {
    LPCWSTR window = (TEXT("test.exe"));
    SetConsoleTitle(window);
    std::cout << "Producer / Consumer Threading Application\n" << std::endl;
    for (int i = 0; i < argc; i++) {
        if (strlen(argv[i]) == strlen(subarg) && strncmp(argv[i], subarg, strlen(subarg)) == 0) {
            std::cout << "child launched" << std::endl;
            ceg7420::NetSubscriber netsub;
            bool socketfailed = netsub.pullSocket();
            if (socketfailed) {
                std::cout << "NetSub socket failed" << std::endl;
            }
            else {
                //received data
                netsub.reportGeneration("NetSubscriber");
            }
            return 0;
            //we are the child process.  Using to run as the net subscriber only.
        }
    }
    
    


    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    TCHAR temp[500] = TEXT("cmd /k CEG7420_FinalProj.exe netSub");

    //// Start the child process. This launches a second command prompt which gets caught and executes the netSubscriber function
    CreateProcess(
        NULL,   // No module name (use command line)
        (LPWSTR)temp,             // Command line
        NULL,             // Process handle not inheritable
        NULL,             // Thread handle not inheritable
        FALSE,            // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,                //Creation flags
        NULL,             // Use parent's environment block
        NULL,             // Use parent's starting directory 
        &si,              // Pointer to STARTUPINFO structure
        &pi               // Pointer to PROCESS_INFORMATION structure
    );

    //instances of the publisher and subscriber
    ceg7420::Subscriber sub;
    ceg7420::Publisher pub;

    //just a simple test to see if the target pc is capable of multi-thread or multi-process operations
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    unsigned short numProcessors = static_cast<unsigned short>(sysInfo.dwNumberOfProcessors);
    std::cout << "NumProcessors (though actually threads): " << numProcessors << std::endl;
    if (numProcessors < 2) {
        std::cerr << "Can not run true test - single processor available" << std::endl;
    }

    int userSeed = 3; //just the seed generator for the publisher int[] filler
    //std::cout << "Please enter a seed value for Random Generator:";
    //std::cin >> userSeed;//cast whatever is entered as an int

    //spawn a thread and give it a function (pub control)
    DWORD dwCreationFlags = 0;

    DWORD pubThreadId = NULL;
    HANDLE theProducerThread;
    theProducerThread = CreateThread( // create thread, use shared memory space
        NULL, //no security attributes
        0,    //stack size in bytes (0=use default size)
        pub.staticThreadFunc,   //pointer to function executed by this thread
        (LPVOID*)&pub,               //argument passed to function
        dwCreationFlags,    //variable above - creation flags (0=run right away)
        &pubThreadId                //returns the thread identifier
    );

    DWORD subThreadId = NULL;
    HANDLE theConsumerThread;
    theConsumerThread = CreateThread( // create thread, use shared memory space
        NULL, //no security attributes
        0,    //stack size in bytes (0=use default size)
        sub.staticThreadFunc,   //pointer to function executed by this thread
        (LPVOID*)&sub,               //argument passed to function
        dwCreationFlags,    //variable above - creation flags (0=run right away)
        &subThreadId                //returns the thread identifier
    );

    //this was used to allow the mailbox to find the other process.  Initially put in both to allow receipt messages
    pub.setOtherThreadId(subThreadId);
    sub.setOtherThreadId(pubThreadId);

    std::cout << "Pub Thread ID: " << pubThreadId << " SubThreadId: " << subThreadId << std::endl;

    //we've spun off the two processes.  Let main monitor what is going on (and spin)
    long int delayer = 0;

    //this wasn't necessary, but would have been useful for program management outside the dedicated pub/sub threads.
    while (!pub.isDone() && !sub.isDone()) {
        //basically just checks on the threads and terminates the program when they are done
        if (delayer % 100 == 0) {
            std::cout << ++delayer << " main " << std::endl;
        }
        if (pub.isDone()) {
            std::cout << "Pub Done! " << std::endl;
        }
        if (sub.isDone()) {
            std::cout << "Sub Done! " << std::endl;
        }
        Sleep(20);
    }
    
    //close the launched process
    TerminateProcess(pi.hProcess, 0);

    //// Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    //
    return 0;
}