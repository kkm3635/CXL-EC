#include "cxlEC.h"
#include "switch.h"
#include <fec.h> //sudo apt-get install libfec-dev
#include <tuple>

CXL_EC_SYSTEM::CXL_EC_SYSTEM(int Local_size, int CXL_mem_num, int CXL_mem_size, int pageSize){
    _pageSize = pageSize;
    _Local_size = Local_size;
    _CXL_mem_num = CXL_mem_num;
    _CXL_mem_size = CXL_mem_size;
    _Local_Range = Local_size/pageSize;

    LOCAL_DRAM = new PAGE[CXL_mem_size/pageSize];
    for(int i = 0; i < _Local_size/pageSize; ++i){
        LOCAL_DRAM_free.push(i);
    }

    CXL_DEVS_free = new std::queue<uint32_t>[CXL_mem_num];
    CXL_DEVS = new PAGE*[CXL_mem_num];
    for(int i = 0; i < CXL_mem_num; ++i){
        CXL_DEVS[i] = new PAGE[CXL_mem_size/pageSize];
        for(int j = 0; j < _CXL_mem_size/pageSize; ++j){
            CXL_DEVS_free[i].push(_Local_size/pageSize + i*CXL_mem_size/pageSize + j);
        }
    }
    pageTable = new PTE[(Local_size+CXL_mem_num*CXL_mem_size)/4096];
}

CXL_EC_SYSTEM::~CXL_EC_SYSTEM(){
    delete[] CXL_DEVS_free;
}

void CXL_EC_SYSTEM::MMU(uint32_t addr, char type){
    //address translation
    uint32_t pageSize = _pageSize;
    uint32_t PFmask = ~(pageSize-1);
    uint32_t VPN = (addr & PFmask) / pageSize;
    uint32_t OFFmask = (1ULL << 12) - 1;
    uint32_t offset = addr & OFFmask;
    uint32_t PFN = pageTable[VPN].PFN;

    //////////////////////first assign implementation

    if(type=='r'){
        if(!pageTable[VPN].isCXL){
            STAMP(VPN);
        }
        LOAD(PFN, offset);
    }
    //isCXL check
    else if(type=='w'){
        if(pageTable[VPN].isCXL){
            //page fault
            PF_PROMOTE(VPN);
            //retrieve pageTable again to get PFN of LocalDRAM
            PFN = pageTable[VPN].PFN;
            STAMP(VPN);
            STORE(PFN, offset);
        }
        else{
            STAMP(VPN);
            STORE(PFN, offset);
        }
    }

}

void CXL_EC_SYSTEM::STAMP(uint32_t VPN){
    for(auto it = LRUlist.begin(); it != LRUlist.end(); ++it){
        if(*it == VPN){
            it = LRUlist.erase(it);
        }
        LRUlist.push_back(VPN);
    }
}  

void CXL_EC_SYSTEM::STORE(uint32_t PFN, uint32_t offset){
    //to be implemented
}

void CXL_EC_SYSTEM::LOAD(uint32_t PFN, uint32_t offset) {
    //to be implemented
}

void CXL_EC_SYSTEM::PF_PROMOTE(uint32_t VPN){
    std::vector<uint32_t> ECset;
    ECset = GET_CGmap(VPN);
    for(int i = 0; i < CGSIZE; ++i) {
        if(LOCAL_DRAM_free.empty()){
            EVICT_COLD();
        }
        //get one free page
        uint32_t myPage = LOCAL_DRAM_free.front();
        LOCAL_DRAM_free.pop();
        pageTable[ECset[i]].PFN = myPage;
        //delete mapping
        CGmap.erase(ECset[i]);
    }
    
}

std::vector<uint32_t> CXL_EC_SYSTEM::GET_CGmap(uint32_t VPN){
    std::unordered_map<uint32_t, std::vector<uint32_t>>::iterator iter;
    iter = CGmap.find(VPN);
    if((iter->second).size() < 2){
        iter = CGmap.find(iter->second[0]);
    }
    return (iter->second);
}   

void CXL_EC_SYSTEM::EVICT_COLD(){
    std::vector<PAGE*> victims;
    std::vector<uint32_t> victim_VPNs;

    for(int i = 0; i < CGSIZE; ++i){
        uint32_t candVPN = *(LRUlist.begin());
        victim_VPNs[i] = candVPN;
        LRUlist.pop_front(); //return cold VPN
        uint32_t candPFN = pageTable[candVPN].PFN;
        victims[i] = &LOCAL_DRAM[candPFN];
        LOCAL_DRAM_free.push(candPFN); //여기 미리 free한 page를 넣어뒀음. 시뮬레이션 상 허용
    }
    //encoding 들어가기

    /////////////토요일 구현
    int* assigned_devices = new int[CGSIZE+PARITY];
    switch1.TAKE_PAGES(victim_VPNs);
}