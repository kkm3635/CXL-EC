#include "switch.h"
#include "cxlEC.h"
#include <vector>

int* CXL_SWITCH::ConsistentHashing(const std::vector<uint32_t>& VPNs) {
    uint32_t groupID = 0;
    
    for(auto it=VPNs.begin(); it!=VPNs.end(); ++it){
        groupID += *it;
        groupID >> (*it)%4;
    }
    return switchRing.get_node(groupID);
}