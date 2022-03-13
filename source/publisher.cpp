#include "..\include\publisher.h"

//IpAddress Tools
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

//message queue
#include <Windows.h>

#pragma comment(lib,"ws2_32") //add ws2_32 to the list of linked libs
#pragma comment(lib, "iphlpapi.lib")

static const char* hostIp = "127.0.0.1";
static const char* destIp = "127.0.0.1";

namespace ceg4350 {
    Publisher::Publisher()
    {
        for (int i = 0; i < DATA_ARRAY_SIZE; i++) {
            values[i] = 0;
        }
        processDone = false;
        seed = 0;
    }


    DWORD WINAPI Publisher::staticThreadFunc(LPVOID lpParam) {
        Publisher* temp = (Publisher*)lpParam;
        //fill in our test set
        temp->generateValues();
        //print out initial test set
        temp->reportGeneration("Publisher");
        if (temp) {
            //send each of the four IPC calls
            temp->pushSharedMem();
            temp->pushSocket();
            temp->pushMailBox();
            temp->pushNamedPipe();
        }
        //let main() know we are done
        temp->setProcessDone(true);
        return 0;
    }

    void Publisher::pushSocket()
    {
        std::cout << "Pushing Socket" << std::endl;
        //publisher can use any socket for transmit, but we need to know the socket and ip of the destination
        
        // initialize Winsock2
        WSADATA wsaData;
        WORD wVersionRequested{ MAKEWORD(2, 2) };
        // initiate the use of Winsock DLL
        int err{ ::WSAStartup(wVersionRequested, &wsaData) };
        if (err != 0) {
            std::cerr << "WSAStartup() FAILED" << std::endl;
        }

        u_short sendPort = 5543;
        u_short recvPort = 5432;

        SOCKET WSAAPI tcpSocket = socket(AF_INET,
            SOCK_STREAM, //tcp/ip
            IPPROTO_TCP);

        sockaddr_in sendAddr;
        IN_ADDR ipSendFrom;
        //translate ip address from numbers to machine
        //note this was fixed from deprecated calls in the microsoft API documentation
        inet_pton(AF_INET, hostIp, &(ipSendFrom));

        //a bunch of calls that establish the connection rules, port, etc.
        sendAddr.sin_family = AF_INET;
        sendAddr.sin_addr.S_un.S_addr = ipSendFrom.S_un.S_addr;
        sendAddr.sin_port = htons(sendPort);

        const char* destIpPattern = destIp;
        IN_ADDR destAddr;
        inet_pton(AF_INET, destIpPattern, &(destAddr));

        sockaddr_in recvAddrRemote;
        recvAddrRemote.sin_family = AF_INET;
        recvAddrRemote.sin_port = htons(recvPort);
        recvAddrRemote.sin_addr.S_un.S_addr = destAddr.S_un.S_addr;

        //the connect call -- this was missing from my first thousand attempts to get this to work.
        bool connected = connect(tcpSocket, (SOCKADDR*)&recvAddrRemote, sizeof(recvAddrRemote));

        //was originally using bind(), which is for the other side        

        //we are connected, so send all the values as one big data block
        if (sendto(tcpSocket, (char*)values, sizeof(values), 0,
            (sockaddr*)&recvAddrRemote, sizeof(recvAddrRemote)) < 0) {
            //very unhelpful error messages here, making it difficult to debug
            std::cout << errno << std::endl;
            perror("tcp cannot send message");
        }
        //hang up
        closesocket(tcpSocket);
        std::cout << "finished tcp send" << std::endl;
        //we can fit the entire data payload in one package 
    }


    void Publisher::pushSharedMem()
    {
        std::cout << "Pushing shared mem" << std::endl;
        //e the shared memoryand semaphores for the implementation of the logical ring -
        //buffer(that can store up to 10 data items) and the synchronization.•For the use
        //of shared memory, refer to Sections 3.7.1 in 10thedition(and Section 3.5.1 in 9th 
        //edition).•For the use of semaphores, refer to Sections 7.3 and 7.4 in 10thedition(or 
        //Section 5.9 in 9th edition)
        //CreateFileMapping();

        TCHAR szName[] = TEXT("Local\\MyFileMappingObject");
       
        HANDLE hMapFile;
        RingIntBuffer* pBuf;
        #define BUF_SIZE sizeof(RingIntBuffer)
        //pub side responsible for creating the shared memory space.  make it as our RingIntBuffer object
        //(defined in utility.cpp)
        hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            BUF_SIZE,                // maximum object size (low-order DWORD)
            szName);                 // name of mapping object

        if (hMapFile == NULL)
        {
            printf("Could not create file mapping object (%d).\n", GetLastError());
        }
        //accessor to the shared memory
        pBuf = (RingIntBuffer*)MapViewOfFile(hMapFile,   // handle to map object
            FILE_MAP_ALL_ACCESS, // read/write permission
            0,
            0,
            BUF_SIZE);
        //so we didn't actually create one, and i relied on constructor for data instantiation. 
        //added this to zeroize the data
        pBuf->initializeData();

        wchar_t buffer[256];
        HANDLE ghSemaphore[10];
        for (int i = 0; i < 10; i++) {
            wsprintfW(buffer, L"%d", i);
            //same as the sub side, an array of semaphores for each ring buffer block
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
            

        if (pBuf == NULL)
        {
            CloseHandle(hMapFile);
        }

        else {
            for (int action = 0; action < 100; action++) {
                //watch for open slots and add to them
                int nextIdx = pBuf->getNextPushIndex();
                if (nextIdx >= 0 && nextIdx < 10) {
                    //wait for our turn
                    WaitForSingleObject(ghSemaphore[nextIdx], INFINITE);
                    //our critical section, add data
                    if (pBuf->addInt(values[action]) == false) {
                        //if it fails to add (no space), sleep and try again
                        Sleep(20);
                        //we didnt get to add data, it was full.  try this location again
                        action--;
                    }
                    std::cout << "**** released " << action << std::endl;
                    //we are done done, so let the semaphore die
                    ReleaseSemaphore(ghSemaphore[nextIdx], 1, NULL);           
                }
            }
            UnmapViewOfFile(pBuf);
            CloseHandle(hMapFile);
        }
    }


    void Publisher::pushNamedPipe() {
        HANDLE pipeHandle;
        DWORD dwWritten;

        //i thought i could name the pipe anything, but the . is required as server name
        pipeHandle = CreateFile(TEXT("\\\\.\\pipe\\PipeName"),
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            0,
            NULL);
        if (pipeHandle != INVALID_HANDLE_VALUE)
        {
            //that's all that is necessary, create pipe, write pipe, all 100 ints
            WriteFile(pipeHandle,
                values,
                sizeof(values),
                &dwWritten,
                NULL);
            CloseHandle(pipeHandle);
        }
        //time delay for syncing
        Sleep(5000);
    }



    void Publisher::pushMailBox()
    {
        //it all came down to one line to make it work.
        //Microsoft Message Queue was a nightmare.
        //i abandoned it and moved onto this simple call.
        //once i pushed the thread id (rather than a blind message send through window routing)
        //it became a simple implementation
        MSG vals;
        vals.message = 4;
        //Next i tried to work with SendMessage, but turns out that needs to have actual windows, not a shared cmd prompt
        for (int i = 0; i < 20; i++) {
            Sleep(1000);
            //added the sleep as a short cut.  Dont want to end up blocked or missing messages
            std::cout << "push message time " << "num" << std::endl;
            //due to timing, just sending the message a few times to make sure it gets caught
            PostThreadMessage(otherThreadIdVal, 111, (WPARAM)values, 0);
        }
        std::cout << "done sending mailbox" << std::endl;
    }

    void Publisher::generateValues()
    {
        //build the publisher data array
        srand(seed);
        for (int i = 0; i < DATA_ARRAY_SIZE; i++) {
            values[i] = rand() % 100;
        }
    }
}
