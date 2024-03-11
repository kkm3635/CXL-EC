#include "switch.h"
#include "cxlEC.h"
#include <vector>

int* SWITCH::TAKE_PAGES(std::vector<uint32_t> VPNs) {
    uint32_t groupID = 0;
    
    for(auto it=VPNs.begin(); it!=VPNs.end(); ++it){
        groupID += *it;
    }
    return switchRing.get_node(groupID);
}

void SWITCH::ErasureCoding(PAGE** victims, int n){
    //libfec으로 구현
    
}