#include "cxlEC.h"
#include "switch.h"
#include <fec.h> //sudo apt-get install libfec-dev
#include <tuple>

CXL_EC_SYSTEM::CXL_EC_SYSTEM(uint64_t Local_size, uint32_t CXL_mem_num, uint64_t CXL_mem_size, int pageSize){
    _pageSize = pageSize;
    _Local_size = Local_size;
    _CXL_mem_num = CXL_mem_num;
    _CXL_mem_size = CXL_mem_size;
    _Local_Range = Local_size/pageSize;
    _Local_Page_num = _Local_size/_pageSize;
    _CXL_Page_num = _CXL_mem_size/_pageSize;
    _PTE_size = 107374182400/4096;

    //LOCAL_DRAM = new PAGE[CXL_mem_size/pageSize];
    for(uint64_t i = 0; i < _Local_size/pageSize; ++i){
        LOCAL_DRAM_free.push(i);
    }
    printf("LOCAL DRAM # of PAGES %ld\n", LOCAL_DRAM_free.size());

    CXL_DEVS_free = new std::queue<uint32_t>[CXL_mem_num];
    //CXL_DEVS = new PAGE*[CXL_mem_num];
    for(uint64_t i = 0; i < CXL_mem_num; ++i){
        //CXL_DEVS[i] = new PAGE[CXL_mem_size/pageSize];
        for(uint64_t j = 0; j < _CXL_mem_size/pageSize; ++j){
            CXL_DEVS_free[i].push(_Local_size/pageSize + i*CXL_mem_size/pageSize + j);
        }
    }
    printf("CXL DRAM PAGE INDIVIDUAL %ld\n", CXL_DEVS_free[0].size());
    printf("TOTAL CXL DRAM PAGE NUM: %ld\n", CXL_DEVS_free[0].size()*CXL_mem_num);
    // int PTEsize = 107374182400/4096;
    printf("PTEsize %d\n", _PTE_size);
    pageTable = new PTE[_PTE_size];
    switch1.ADD_NODE(CXL_mem_num);
    PF_PROMOTE_COUNTER=0;
    EVICT_COUNTER=0;
}

CXL_EC_SYSTEM::~CXL_EC_SYSTEM(){
    delete[] CXL_DEVS_free;
}

void CXL_EC_SYSTEM::MMU(uint32_t addr, char type){
    //address translation
    uint32_t pageSize = _pageSize;
    uint32_t PFmask = ~(pageSize-1);
    uint32_t VPN = (addr & PFmask)/pageSize;
    VPNset.insert(VPN);
    uint32_t OFFmask = (1ULL << 12) - 1;
    uint32_t offset = addr & OFFmask;
    uint32_t PFN = -1;
    if(pageTable[VPN].getvalid()){
        PFN = pageTable[VPN].getPFN();
        if(type=='r'){
            if(!pageTable[VPN].getisCXL()){ //DRAM이면 stamp
                //STAMP(VPN);
            }
            LOAD(PFN, offset);
        }
        //isCXL check
        else if(type=='w'){
            if(pageTable[VPN].getisCXL()){
                //page fault
                PF_PROMOTE(VPN);
                //retrieve pageTable again to get PFN of LocalDRAM
                PFN = pageTable[VPN].getPFN();
                STAMP(VPN);
                STORE(PFN, offset);
            }
            else{
                STAMP(VPN);
                STORE(PFN, offset);
            }
        }
    }
    else {
        //////////////////////first assign implementation
        if(LOCAL_DRAM_free.empty()){
            EVICT_COUNTER++;
            EVICT_COLD();
        }
        PFN = LOCAL_DRAM_free.front();
        LOCAL_DRAM_free.pop();
        if(VPNtoPFN.find(VPN) != VPNtoPFN.end()){
        }
        else {
            VPNtoPFN.insert({VPN, PFN});
        }
        //printf("POP!!! %d\n", PFN);
        pageTable[VPN].setPFN(PFN);
        pageTable[VPN].setvalid(1);
        //printf("MMU first assign:\tVPN %x\n", VPN);
        LRUlist.push_back(VPN);
        //printf("push %x\n", LRUlist.back());
        //STAMP(VPN);
        if(type == 'r')
            LOAD(PFN, offset);
        else
            STORE(PFN, offset);
    }

}

void CXL_EC_SYSTEM::STAMP(uint32_t VPN){
    for(auto it = LRUlist.begin(); it != LRUlist.end(); ++it){
        if(*it == VPN){
            it = LRUlist.erase(it);
            break;
        }
    }
    LRUlist.push_back(VPN);
}  

void CXL_EC_SYSTEM::STORE(uint32_t PFN, uint32_t offset){
    //to be implemented
}

void CXL_EC_SYSTEM::LOAD(uint32_t PFN, uint32_t offset) {
    //to be implemented
}

void CXL_EC_SYSTEM::PF_PROMOTE(uint32_t VPN){
    PF_PROMOTE_COUNTER ++;
    std::vector<uint32_t> ECset;
    ECset = GET_CGmap(VPN);

    //여기에 구현 다시해야함 ECset으로 불러온거 CXL_DRAM에 reclaim을 안하고 있다
    for(int i = 0; i < CGSIZE; ++i) {
        if(LOCAL_DRAM_free.empty()){
            EVICT_COLD();
        }
        //get one free page
        uint32_t myPage = LOCAL_DRAM_free.front();
        LOCAL_DRAM_free.pop();
        uint32_t oldPage = pageTable[ECset[i]].getPFN();
        uint32_t relDev = (oldPage-_Local_Page_num)/(_CXL_Page_num);

        CXL_DEVS_free[relDev].push(oldPage);

        pageTable[ECset[i]].setPFN(myPage);
        pageTable[ECset[i]].setisCXL(0);
        //delete mapping
        CGmap.erase(ECset[i]);
    }
    std::vector<uint32_t> Pset;
    Pset = Pmap[ECset[0]];
    for(int i = 0; i < PARITY; ++i){
        uint32_t oldPage = Pset[i];
        uint32_t relDev = (oldPage-_Local_Page_num)/(_CXL_Page_num);
        CXL_DEVS_free[relDev].push(oldPage);
    }
    Pmap.erase(ECset[0]);
    
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
        victim_VPNs.push_back(candVPN);
        pageTable[candVPN].evict_count++;
        LRUlist.pop_front(); //return cold VPN
        uint32_t candPFN = pageTable[candVPN].getPFN();
        victims.push_back(&LOCAL_DRAM[candPFN]);
        LOCAL_DRAM_free.push(candPFN); //여기 미리 free한 page를 넣어뒀음. 시뮬레이션 상 허용
    }
    //encoding 했다고 친다

    int* assigned_devices;
    assigned_devices = switch1.ConsistentHashing(victim_VPNs);
    // for(int i = 0; i < CGSIZE+PARITY; ++i){
    //     printf("%d, ", assigned_devices[i]);
    // }
    // printf("\n=============\n");
    //page table
    for(int i = 0; i < CGSIZE; ++i){
        if(CXL_DEVS_free[assigned_devices[i]].size()<=0){
            printf("THERE'S NO AVAILABLE MEMORY IN CXL DEVICE %d\n", assigned_devices[i]);
            abort();
        }
        pageTable[victim_VPNs[i]].setPFN(CXL_DEVS_free[assigned_devices[i]].front());
        CXL_DEVS_free[assigned_devices[i]].pop();
        pageTable[victim_VPNs[i]].setisCXL(1);
    }
    std::vector<uint32_t> Ppage;
    for(int i = 0; i < PARITY; ++i){
        Ppage.push_back(CXL_DEVS_free[assigned_devices[i]].front());
        CXL_DEVS_free[assigned_devices[i]].pop();
    }
    Pmap.insert({victim_VPNs[0], Ppage});


    //device에 실제로 적었다고 친다

    CGmap.insert({victim_VPNs[0], victim_VPNs});
    //printf("CGmap LEADER INFO UPDATE %d, {", victim_VPNs[0]);
    // for(int i = 0; i < CGSIZE; ++i){
    //     printf("%d ", CGmap[victim_VPNs[0]][i]);
    // }
    //printf("}\n");
    for(int i = 1; i < CGSIZE; ++i){
        CGmap.insert({victim_VPNs[i], std::vector<uint32_t>{victim_VPNs[0]}});
        //printf("CGmap INFO UPDATE %d, cont: %d, size: %d\n", victim_VPNs[i], CGmap[victim_VPNs[i]].front(), CGmap[victim_VPNs[i]].size());
    }
}