#include "..\include\utility.h"

//IpAddress Tools
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <sstream>

#pragma comment(lib,"ws2_32") //add ws2_32 to the list of linked libs
#pragma comment(lib, "iphlpapi.lib")

//end Ip

namespace ceg7420 {

    Utility::Utility()
    {
        processDone = false;
    }

    void Utility::reportGeneration(const char* reporter)
    {//allows each instance to generate their version of the data
        std::cout << reporter << std::endl;
        for (int i = 0; i < DATA_ARRAY_SIZE; i++) {
            if (i % 10 == 0)
                std::cout << std::endl;
            std::cout << " [" << i << "]=" << values[i];
        }
        std::cout << std::endl;
    }
 

    RingIntBuffer::RingIntBuffer() {
        for (int i = 0; i < 10; i++) {
            rBuf[i].storageInt = -1;
            rBuf[i].valid = false;
        }
        lastPushed = -1;
        lastPulled = -1;
    }

    void RingIntBuffer::initializeData() {
        //initialize call, when i discovered that my sharedmem wasn't calling the constructor
        for (int i = 0; i < 10; i++) {
            rBuf[i].storageInt = -1;
            rBuf[i].valid = false;
        }
        lastPushed = -1;
        lastPulled = -1;
    }
    //add int and get int are the access interfaces on the ring buffer instance
    bool RingIntBuffer::addInt(int newInt) {
        //tried to manage the semaphore here, but ran into issues.
        bool returnVal = false;
        int storageLocation = (lastPushed + 1) % 10; 
        std::cout << "[setting] acuired " << storageLocation << std::endl;
        if (rBuf[storageLocation].valid == false) {
            rBuf[storageLocation].storageInt = newInt;
            rBuf[storageLocation].valid = true;
            lastPushed = storageLocation;
            returnVal = true;
            std::cout << "[setting] set " << storageLocation << std::endl;
        }
        else {
            std::cout << "[setting] failed " << storageLocation << std::endl;
        }
        std::cout << "[setting] released " << storageLocation << std::endl;
        return returnVal;
    }
    int RingIntBuffer::getInt() {
        int storageLocation = (lastPulled + 1) % 10;
        std::cout << "aquired [getting] " << storageLocation << std::endl;
        int returnVal = -1;
        if (rBuf[storageLocation].valid == true) {
            returnVal = rBuf[storageLocation].storageInt;
            lastPulled = storageLocation;
            rBuf[storageLocation].valid = false;
            Sleep(100);
            std::cout << "retrieved [getting] " << storageLocation << std::endl;
        }
        else {
            std::cout << "not valid [getting] " << storageLocation;
        }
        std::cout << "released [getting] " << storageLocation << std::endl;
        return returnVal;
    }
}
