#ifndef SWITCH_H
#define SWITCH_H

#include "ConsistentHashing.h"

class CXL_SWITCH{
    private:
        //char* inBuffer;
        int* hash_result;
        int device_num;
        ConsistentHashRing switchRing;

    public:
        CXL_SWITCH(){
            //inBuffer = new char[4096*5];
            hash_result = new int[CGSIZE];
            switchRing = ConsistentHashRing();
        }
        int* ConsistentHashing(const std::vector<uint32_t>& VPNs);
        void ADD_NODE(int node_num){
            switchRing.add_node(node_num);
        }
        //void ERASURE_CODING(PAGE** victims, int n);
}; 

#endif