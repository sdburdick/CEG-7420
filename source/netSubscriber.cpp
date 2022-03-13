#include "..\include\netSubscriber.h"

#define WIN32_LEAN_AND_MEAN
static const char* DEFAULT_IP = "127.0.0.1";

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace ceg7420 {
    NetSubscriber::NetSubscriber()
    {
        for (int i = 0; i < DATA_ARRAY_SIZE; i++) {
            values[i] = 0;
        }
    }

    bool NetSubscriber::pullSocket()
    {        
        std::cout << "Pulling Socket" << std::endl;
        // initialize Winsock2
        WSADATA wsaData;
        WORD wVersionRequested{ MAKEWORD(2, 2) };
        // initiate the use of Winsock DLL
        int err{ ::WSAStartup(wVersionRequested, &wsaData) };
        if (err != 0) {
            std::cerr << "WSAStartup() FAILED" << std::endl;
        }

        u_short recvPort = 5432;

        static const int DEFAULT_BUFLEN = 512;
        const char* DEFAULT_PORT = "5432";

        int iResult;

        SOCKET ListenSocket = INVALID_SOCKET;
        SOCKET ClientSocket = INVALID_SOCKET;

        struct addrinfo* result = NULL;
        struct addrinfo hints;

        int iSendResult;
        char recvbuf[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;//sock stream - tcp

        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        iResult = getaddrinfo(DEFAULT_IP, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed with error: %d\n", iResult);
            WSACleanup();
            return true;
        }

        // Create a SOCKET for connecting to server
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return true;
        }
        
        // Setup the TCP listening socket
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf("bind failed with error: %d\n", WSAGetLastError());
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return true;
        }

        freeaddrinfo(result);
        std::cout << "Blocking Listen on NetSubscriber..." << std::endl;
        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return true;
        }
        std::cout << "NetScubscriber Connected!" << std::endl;
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return true;
        }

        // No longer need server socket
        closesocket(ListenSocket);

        // Receive until the peer shuts down the connection
        do {

            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                printf("Bytes received: %d\n", iResult);
                //copy the received data into the integer buffer
                //clearly not a safe way to do it, but it gets the job done
                memcpy(values, (int*)recvbuf, 100 * sizeof(int));
                std::cout << std::endl;
                
            }
        } while (iResult > 0);

        // shutdown the connection since we're done
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return true;
        }
        // cleanup
        closesocket(ClientSocket);
        WSACleanup();
        return false;
    }
}
