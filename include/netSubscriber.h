#ifndef NetSubscriberCEG
#define NetSubscriberCEG

#include "utility.h"
#include <iostream>
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif


namespace ceg4350 {

    class NetSubscriber : public Utility{
    public:
        NetSubscriber();
     
        bool pullSocket(); //return 1 on failure, return 0 when done
        

    private:
        
    };

}

#endif
