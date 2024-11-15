#include "cxlEC.h"
#include <fec.h> 
#include <tuple>
#include <list>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <bits/stdc++.h>

CXL_EC_SYSTEM::CXL_EC_SYSTEM(uint64_t Local_size, uint32_t CXL_mem_num, uint64_t CXL_mem_size, uint64_t CXL_parity_size, int pageSize){
    _pageSize = pageSize;
    _Local_size = Local_size;
    _CXL_mem_num = CXL_mem_num;
    _CXL_mem_size = CXL_mem_size + CXL_parity_size;
    _CXL_parity_size = CXL_parity_size;

    // Memory allocator - Local DRAM
    LOCAL_DRAM_free = 67014929;
   
    printf("LOCAL DRAM # of PAGES %d\n", LOCAL_DRAM_free);

    // Memory allocator (Data) - CXL DRAM
    CXL_DEVS_Data_free = new deque<uint32_t>[CXL_mem_num];
    for(int i=0; i<CXL_mem_num; ++i){
        for(int j=0; j<Data_Frame; ++j){
            CXL_DEVS_Data_free[i].push_back(CXL_Start_Frame + j + i*CXL_Total_Frame);
        }
    }
    
    // Memory allocator (Parity) - CXL DRAM
    CXL_DEVS_Parity_free = new deque<uint32_t>[CXL_mem_num];
    for(int i=0; i<CXL_mem_num; ++i){
        for(int j=0; j<Parity_Frame; ++j){
            CXL_DEVS_Parity_free[i].push_back(CXL_Start_Frame + Data_Frame + j + i*CXL_Total_Frame);
        }
    }

    printf("CXL DRAM PAGE INDIVIDUAL %ld\n", CXL_DEVS_Data_free[0].size() + CXL_DEVS_Parity_free[0].size());
    printf("TOTAL CXL DRAM PAGE NUM: %ld\n", (CXL_DEVS_Data_free[0].size() + CXL_DEVS_Parity_free[0].size()) * CXL_mem_num);

    Frame = new (std::nothrow) PAGE[End_Frame];
    if(Frame == nullptr){
        std::cerr << "Error: Failed to allocate memory for Frame" << std::endl;
        exit(1);
    }

    MIGRATE_COUNTER=0;
}

CXL_EC_SYSTEM::~CXL_EC_SYSTEM(){
    delete[] Frame;
}

uint32_t CXL_EC_SYSTEM::PFN_TRANSLATE(uint32_t PFN){
    uint32_t pfn = PFN;
    
    if(PFN <= Local_End_Frame){
        pfn = pfn;
    }
    else{
        int a =  (pfn - CXL_Start_Frame) / Data_Frame;
        pfn = pfn + Parity_Frame * a;
    }

    return pfn;
}

void CXL_EC_SYSTEM::REQUEST(uint32_t addr, char type){
    uint32_t pageSize = _pageSize;
    uint32_t OFFmask = (1ULL << 12) - 1; 
    uint32_t offset = addr & OFFmask; 
    uint32_t PFN = addr/pageSize;
    int32_t section = offset / 1024;

    PFN = PFN_TRANSLATE(PFN);
    
    if(PFN <= Local_End_Frame){  // Local
        if(type == 'r'){  // Local Load
            LOAD(PFN, section);
        }
        else{  // Local Store
            STORE(PFN, section);
        }
    }
    else{ // CXL
        vector<uint32_t> Coding_Group = getCodingGroup(PFN);
        int Dev = (PFN - CXL_Start_Frame) / CXL_Total_Frame;

        if(Coding_Group.empty()){
            auto it = find(CXL_DEVS_Data_free[Dev].begin(), CXL_DEVS_Data_free[Dev].end(), PFN);
            if(it != CXL_DEVS_Data_free[Dev].end()){
                CXL_DEVS_Data_free[Dev].erase(it);
            }

            vector<uint32_t> assinged_PFNs = CXL_MEM_Alloc(PFN, Dev);
            addCodingGroup(assinged_PFNs);
        } 

        if(type == 'r'){ // CXL Load
            LOAD(PFN, section);
        }
        else{ // CXL Store
            STORE(PFN, section);

            // Parity Update
            Coding_Group = getCodingGroup(PFN);
            uint32_t Parity_PFN = Coding_Group.back();
            Coding_Group.pop_back();
            int* Parity = ENCODE(Coding_Group);

            for(int i=0; i<4; ++i){
                Frame[Parity_PFN].updateDATA(Parity[i], i);
            }
        }
    }
}

void CXL_EC_SYSTEM::STORE(uint32_t PFN, int32_t section){
    srand(static_cast<unsigned int>(std::time(0)));
    int32_t data = std::rand() % 7;
    Frame[PFN].updateDATA(data, section);
}

void CXL_EC_SYSTEM::LOAD(uint32_t PFN, int32_t section) {

}

void CXL_EC_SYSTEM::MIGRATE(uint64_t From, uint64_t To, char type){
    uint32_t pageSize = _pageSize;
    uint32_t From_PFN = From / pageSize;
    uint32_t To_PFN = To / pageSize;

    From_PFN = PFN_TRANSLATE(From_PFN);
    To_PFN = PFN_TRANSLATE(To_PFN);

    if(From > Local_End_Frame){  // 2 -> 0 
        Frame[To_PFN].setDATA(Frame[From_PFN].getData(), 1);
        Frame[From_PFN].setDATA(Frame[From_PFN].getData(), 0);

        vector<uint32_t> Coding_Group = getCodingGroup(From_PFN);
        if(Coding_Group.empty()){
            printf("Empty Coding Group\n");
            abort();
        }
        uint32_t Parity_PFN = Coding_Group.back();
        Coding_Group.pop_back();
        int* Parity = ENCODE(Coding_Group);

        for(int i=0; i<4; ++i){
            Frame[Parity_PFN].updateDATA(Parity[i], i);
        }
    }
    else{  // 0 -> 2
        Frame[To_PFN].setDATA(Frame[From_PFN].getData(), 1);
        Frame[From_PFN].setDATA(Frame[From_PFN].getData(), 0);

        vector<uint32_t> Coding_Group = getCodingGroup(To_PFN);
        int Dev = (To_PFN - CXL_Start_Frame) / CXL_Total_Frame;

        if(Coding_Group.empty()){
            auto it = find(CXL_DEVS_Data_free[Dev].begin(), CXL_DEVS_Data_free[Dev].end(), To_PFN);
            if(it != CXL_DEVS_Data_free[Dev].end()){
                CXL_DEVS_Data_free[Dev].erase(it);
            }

            vector<uint32_t> assinged_PFNs = CXL_MEM_Alloc(To_PFN, Dev);
            addCodingGroup(assinged_PFNs);

            Coding_Group = getCodingGroup(To_PFN);
            uint32_t Parity_PFN = Coding_Group.back();
            Coding_Group.pop_back();
            int* Parity = ENCODE(Coding_Group);

            for(int i=0; i<4; ++i){
                Frame[Parity_PFN].updateDATA(Parity[i], i);
            }
        }
        else{  // 덮어쓰기
            Coding_Group = getCodingGroup(To_PFN);
            uint32_t Parity_PFN = Coding_Group.back();
            Coding_Group.pop_back();
            int* Parity = ENCODE(Coding_Group);

            for(int i=0; i<4; ++i){
                Frame[Parity_PFN].updateDATA(Parity[i], i);
            }
        }
    }
}

vector<uint32_t> CXL_EC_SYSTEM::getCodingGroup(uint32_t frame){
    if(table1.find(frame) != table1.end()){
        CodingGroup* group = table1[frame];
        return group->frames;
    }else{
        return {};
    }
}

void CXL_EC_SYSTEM::addCodingGroup(const vector<uint32_t>& frames){
    CodingGroup* group = new CodingGroup(frames);

    for(int frame : frames){
        table1[frame] = group;
    }
    
    table2[group] = frames;
}


vector<uint32_t> CXL_EC_SYSTEM::CXL_MEM_Alloc(uint32_t PFN, int Dev){ // Memory allocation
    vector<uint32_t> frame;
    frame.push_back(PFN);

    vector<pair<int, int>> Enough_CXL_DEV;
    for(int i=0; i< _CXL_mem_num; ++i){
        if(i == Dev)
            continue;

        Enough_CXL_DEV.push_back({CXL_DEVS_Data_free[i].size() + CXL_DEVS_Parity_free[i].size(), i}); 
    }

    sort(Enough_CXL_DEV.begin(), Enough_CXL_DEV.end(), greater<>());

    for(int i=0; i < DATA -1; ++i){
        if(Enough_CXL_DEV[i].first == 0){  // Is enough cxl dram memory ???
            printf("THERE'S NO AVAILABLE MEMORY IN CXL DEVICE %d\n", Enough_CXL_DEV[i].second);
            abort();
        }

        int dev = Enough_CXL_DEV[i].second;
        frame.push_back(CXL_DEVS_Data_free[dev].front());
        CXL_DEVS_Data_free[dev].pop_front();
    }

    int dev = Enough_CXL_DEV[DATA].second;
    frame.push_back(CXL_DEVS_Parity_free[dev].front());
    CXL_DEVS_Parity_free[dev].pop_front();
 
    return frame;
}

void CXL_EC_SYSTEM::DECODE(std::vector<uint32_t> Decode_CG){
    
}

void CXL_EC_SYSTEM::CALCULATE_PARITY(vector<vector<int>>& Original_Data, int* Parity){
    for(int i=0;i<DATA;i++){
        for(int j=0;j<4;j++){
            Parity[j]+=(i+1)*Original_Data[i][j];
            if(Parity[j] > 7){
                Parity[j] = Parity[j] % 7;
            }
        }   
    }
}

int* CXL_EC_SYSTEM::ENCODE(vector<uint32_t> Coding_Group){
    int* parity = new int[4]{0,0,0,0};
    vector<vector<int>> data_matrix(4, vector<int>(4));

    for(int i=0; i < DATA; ++i){ // Original data -> matrix
        int PFN = Coding_Group[i];
        int* data = Frame[PFN].getData();
        for(int j=0; j<4; ++j){
            data_matrix[i][j] = data[j];
        }
    }

    CALCULATE_PARITY(data_matrix, parity);

    return parity;
}

void CXL_EC_SYSTEM::RECOVERY(uint32_t failure_dev){
//     int lower_bound = _Local_size/_pageSize + _CXL_mem_size/_pageSize * (failure_dev-1);
//     int upper_bound = _Local_size/_pageSize + _CXL_mem_size/_pageSize * (failure_dev);    

//     // failure device를 전부 비우고 다시 채워넣음으로써 새 cxl device 할당받은 것 처럼 동작
//     while (CXL_DEVS_free[failure_dev].size()==0) {
//         CXL_DEVS_free[failure_dev].pop();    
//     }
//     for(int i = lower_bound;i<upper_bound;i++){
//         CXL_DEVS_free[failure_dev].push(i);
//     }

//     for(auto& pair : CGmap){ // CGmap scanning 
//         std::vector<uint32_t> Coding_Group = pair; // Particular Coding Group
//         for(auto& member : Coding_Group){ // member: Coding group's data or parity
//             if(member >= lower_bound && member < upper_bound){ // failure device의 data가 포함된 coding group 발견
//                 DECODE(Coding_Group);

//             }
//         }
//     }

}
