#ifndef SWITCH_H
#define SWITCH_H

#include "ConsistentHashing.h"

class PAGE;

class SWITCH{
    private:
        //char* inBuffer;
        int* hash_result;
        int device_num;
        ConsistentHashRing switchRing;

    public:
        SWITCH(){
            //inBuffer = new char[4096*5];
            hash_result = new int[CGsize];
            switchRing = ConsistentHashRing();
        }
        int* TAKE_PAGES(std::vector<uint32_t> VPNs);
        void ERASURE_CODING(PAGE** victims, int n);
        void consistent_hashing();
}; 

#endif