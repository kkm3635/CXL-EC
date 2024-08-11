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
    for(uint64_t i = 0; i < _Local_size/pageSize; ++i){  // queue에 local dram free page를 넣음
        LOCAL_DRAM_free.push(i);
    }
    printf("LOCAL DRAM # of PAGES %ld\n", LOCAL_DRAM_free.size());
 
    CXL_DEVS_free = new std::queue<uint32_t>[CXL_mem_num];  // cxl device마다 free page 담는 queue 객체 생성
    //CXL_DEVS = new PAGE*[CXL_mem_num];
    for(uint64_t i = 0; i < CXL_mem_num; ++i){ // 각 cxl dram마다 queue에 free page 넣음
        //CXL_DEVS[i] = new PAGE[CXL_mem_size/pageSize];
        for(uint64_t j = 0; j < _CXL_mem_size/pageSize; ++j){
            CXL_DEVS_free[i].push(_Local_size/pageSize + i*CXL_mem_size/pageSize + j); // page index를 안 겹치게 이어서 할당하고 있음
        }
    }
    printf("CXL DRAM PAGE INDIVIDUAL %ld\n", CXL_DEVS_free[0].size());
    printf("TOTAL CXL DRAM PAGE NUM: %ld\n", CXL_DEVS_free[0].size()*CXL_mem_num);
    // int PTEsize = 107374182400/4096;
    printf("PTEsize %d\n", _PTE_size);
    pageTable = new PTE[_PTE_size];  // page table 객체 생성
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
    uint32_t PFmask = ~(pageSize-1); // 주소에서 페이지 프레임 번호 부분을 위한 마스크 생성
    uint32_t VPN = (addr & PFmask)/pageSize; // 주소를 마스킹하고 페이지 크기로 나누어 가상 페이지 번호를 계산
    VPNset.insert(VPN);
    uint32_t OFFmask = (1ULL << 12) - 1; // 페이지 내 오프셋에 대한 마스크 생성 (12비트 오프셋 가정)
    uint32_t offset = addr & OFFmask; // 주소에서 페이지 내 오프셋을 추출함
    uint32_t PFN = -1;

    if(pageTable[VPN].getvalid()){ // first access가 아닌 경우
        PFN = pageTable[VPN].getPFN();
        if(type=='r'){
            if(!pageTable[VPN].getisCXL()){ // local dram load
                STAMP(VPN);
                LOAD(PFN, offset);
            } // cxl dram load
            LOAD(PFN, offset); // cxl dram load
        }
        else{
            if(pageTable[VPN].getisCXL()){  // cxl dram store
                //page fault
                PF_PROMOTE(VPN);
                //retrieve pageTable again to get PFN of LocalDRAM
                PFN = pageTable[VPN].getPFN();
                STORE(PFN, offset);
            }
            else{  // local dram store
                STAMP(VPN);
                STORE(PFN, offset);
            }
        }
    }
    else { //first access인 경우
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
        if(type == 'r')
            LOAD(PFN, offset);
        else
            STORE(PFN, offset);
    }

}

void CXL_EC_SYSTEM::STAMP(uint32_t VPN){ // 해당 VPN을 LRU에서 갱신하는 역할
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
    ECset = GET_CGmap(VPN);  // 해당 VPN과 관련된 coding group searching

    for(int i = 0; i < CGSIZE; ++i) {
        if(LOCAL_DRAM_free.empty()){
            EVICT_COLD();
        }
        //get one free page
        uint32_t myPage = LOCAL_DRAM_free.front();
        LOCAL_DRAM_free.pop();
        uint32_t oldPage = pageTable[ECset[i]].getPFN(); // CG의 VPN에 대한 PFN 가져옴
        uint32_t relDev = (oldPage-_Local_Page_num)/(_CXL_Page_num); // 해당 PFN이 몇 번째 cxl dram인지 계산

        CXL_DEVS_free[relDev].push(oldPage); // 해당 cxl dram에 PFN 큐에 추가

        LRUlist.push_back(ECset[i]);
        pageTable[ECset[i]].setPFN(myPage);
        pageTable[ECset[i]].setisCXL(0);
        //delete mapping
        CGmap.erase(ECset[i]);
    }

    std::vector<uint32_t> Pset;
    for(auto it : Pmap[ECset[0]]){
        Pset.push_back(it);
    }
    for(int i = 0; i < PARITY; ++i){
        uint32_t oldPage = Pset[i];
        uint32_t relDev = (oldPage-_Local_Page_num)/(_CXL_Page_num); // parity가 써져있는 cxl dram이 어디인지
        CXL_DEVS_free[relDev].push(oldPage); // 해당 cxl dram free queue에 빈 페이지 추가
    }
    Pmap.erase(ECset[0]);
    
}

std::vector<uint32_t> CXL_EC_SYSTEM::GET_CGmap(uint32_t VPN){
    std::unordered_map<uint32_t, std::vector<uint32_t>>::iterator iter;  // iter는 CGmap에서 주어진 VPN을 찾는 이터레이터
    iter = CGmap.find(VPN);
    if((iter->second).size() < 2){  // 즉, 대장이 아니면 
        iter = CGmap.find(iter->second[0]);  // "쫄따구 벡터의 첫 번째 요소 == 대장의 VPN" 으로 다시 찾음
    }
    printf("hihihi\n");
    return (iter->second); // 해당 vpn과 관련된 모든 vpn을 반환
}   

void CXL_EC_SYSTEM::EVICT_COLD(){
    std::vector<PAGE*> victims; // cold page들의 포인터를 저장하는 벡터
    std::vector<uint32_t> victim_VPNs; // cold page들의 vpn을 저장하는 벡터

    for(int i = 0; i < CGSIZE; ++i){  // LRU 리스트의 처음부터 CGSIZE 만큼의 페이지를 선택하여 eviction 후보로 선정
        uint32_t candVPN = *(LRUlist.begin());
        victim_VPNs.push_back(candVPN); // eviction page의 vpn 저장
        pageTable[candVPN].evict_count++; // eviction page의 evict_count 증가
        LRUlist.pop_front(); // eviction page LRU list에서 제거

        uint32_t candPFN = pageTable[candVPN].getPFN(); 
        victims.push_back(&LOCAL_DRAM[candPFN]); // evgiction page의 포인터 저장
        LOCAL_DRAM_free.push(candPFN); //여기 미리 free한 page를 넣어뒀음. 시뮬레이션 상 허용
    }

    //encoding 했다고 친다

    int* assigned_devices;
    assigned_devices = switch1.ConsistentHashing(victim_VPNs);  // consistenthashing으로 어느 device에 뿌릴 건 지 정해옴
    //printf("%d\n",assigned_devices[0]);

    for(int i = 0; i < CGSIZE; ++i){ // data 씀
        if(CXL_DEVS_free[assigned_devices[i]].size()<=0){ // 할당된 cxl dram에 사용할 수 있는 메모리가 있는지 확인
            for(int i = 0; i < _CXL_mem_num; ++i){
                printf("CXL DEV %d available pages: %ld, percentage: %lf%%\n", i, 
                CXL_DEVS_free[i].size(), (double)CXL_DEVS_free[i].size()/(_CXL_mem_size/_pageSize)*100);
            }
            printf("THERE'S NO AVAILABLE MEMORY IN CXL DEVICE %d\n", assigned_devices[i]);
            abort();
        }
        pageTable[victim_VPNs[i]].setPFN(CXL_DEVS_free[assigned_devices[i]].front()); // VPN-to-PFN mapping
        CXL_DEVS_free[assigned_devices[i]].pop(); // cxl free 
        pageTable[victim_VPNs[i]].setisCXL(1); // 해당 페이지가 cxl에 있음을 표시

    }

    std::vector<uint32_t> Ppage; // 어느 페이지가 parity인지 표현
    for(int i = 0; i < PARITY; ++i){ // encoding했다 치고 parity 씀
        if(CXL_DEVS_free[assigned_devices[i+4]].size()<=0){
            for(int i = 0; i < _CXL_mem_num; ++i){
                printf("CXL DEV %d available pages: %ld, percentage: %lf%%\n", i, 
                CXL_DEVS_free[i].size(), (double)CXL_DEVS_free[i].size()/(_CXL_mem_size/_pageSize)*100);
            }
            printf("THERE'S NO AVAILABLE MEMORY IN CXL DEVICE %d\n", assigned_devices[i]);
            abort();
        }

        Ppage.push_back(CXL_DEVS_free[assigned_devices[i+4]].front()); 
        CXL_DEVS_free[assigned_devices[i+4]].pop();
    }
    Pmap.insert({victim_VPNs[0], Ppage}); // 여기서 Ppage는 PFN
    CGmap.insert({victim_VPNs[0], victim_VPNs});  // 첫 VPN을 리더로하여 CGmap에 추가
    
    for(int i = 1; i < CGSIZE; ++i){  // 나머지 VPN들도 CGmap에 리더를 가리키도록 추가
        CGmap.insert({victim_VPNs[i], std::vector<uint32_t>{victim_VPNs[0]}});
        //printf("CGmap INFO UPDATE %d, cont: %d, size: %d\n", victim_VPNs[i], CGmap[victim_VPNs[i]].front(), CGmap[victim_VPNs[i]].size());
    }
}
