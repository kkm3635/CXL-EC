#include "switch.h"
#include "cxlEC.h"
#include <vector>
#include <functional>
#include <cmath>

int* CXL_SWITCH::ConsistentHashing(const std::vector<uint32_t>& VPNs) {
    // uint32_t groupID = 0;

    std::stringstream ss;
    for(auto vpn : VPNs){
        ss << vpn;
        // printf("%d\n",vpn);  // VPNs 출력되게 함
    }
    std::string key_str = ss.str();
    // std::cout << key_str << std::endl; // VPNs 이어붙인거 출력

    std::hash<std::string> hash_fun;
    std::size_t CG_ID = hash_fun(key_str); // VPNs 이어붙인걸 key로 hash에 넣음. 

    int num_digits = static_cast<int>(std::log10(CG_ID+1));
    float divisor = std::pow(10.0f, num_digits);
    float result = static_cast<float>(CG_ID) / divisor / 10;

    // std::cout << CG_ID << std::endl; // key (VPNs)를 hash에 넣은 결과
    // std::cout << result << std::endl; // 최종 CG_ID
    // printf("-------------\n");
    
    return switchRing.get_node(result);
}
