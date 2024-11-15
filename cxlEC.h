#ifndef CXL_EC_H
#define CXL_EC_H

#include <cstdlib>
#include <cstdint>
#include <list>
#include <any>
#include <vector>
#include <unordered_map>
#include <queue>
#include <bits/stdc++.h>
#include <algorithm>

#define DATA 3
#define PARITY 1

#define Local_End_Frame 67633152
#define CXL_Start_Frame 134742016
#define End_Frame 168296447
#define Data_Frame 6291456
#define Parity_Frame 2097152
#define CXL_Total_Frame 8388608

using namespace std;

class PAGE{
    public:
    int Data[4];

    PAGE(){
        for(int i=0; i<4; ++i){
            Data[i] = 0;
        }
    }

    void setDATA(int* data, int option){
        if (option == 0){
            for(int i=0; i<4; ++i){
                this->Data[i] = 0;
            }
        }
        else{
            for(int i=0; i<4; ++i){
                this->Data[i] = data[i];
            }    
        }
    }

    void updateDATA(int32_t data, int32_t section){
        this->Data[section] = data;
    }

    int* getData(){
        return Data;
    }
};

class CodingGroup {
    public:
        vector<uint32_t> frames;
        CodingGroup(const std::vector<uint32_t>& frames) : frames(frames) {}
};

class CXL_EC_SYSTEM{
    private:
        int _pageSize;
        uint64_t _Local_size;
        uint32_t _CXL_mem_num;
        uint64_t _CXL_mem_size;
        uint32_t _CXL_parity_size;
        uint32_t _Local_Range; 
        uint32_t _Local_Page_num;
        uint32_t _CXL_Page_num;
        uint32_t _PTE_size;
        uint32_t LOCAL_DRAM_free; // Local Dram remaining memory 
        deque<uint32_t>* CXL_DEVS_Data_free; // CXL Devices remaining memory
        deque<uint32_t>* CXL_DEVS_Parity_free; // CXL Devices remaining memory

        PAGE* Frame;
        unordered_map<int, CodingGroup*> table1;
        unordered_map<CodingGroup*, vector<uint32_t>> table2;
        
    public:
        CXL_EC_SYSTEM(uint64_t Local_size, uint32_t CXL_mem_num, uint64_t CXL_mem_size, uint64_t CXL_parity_size, int pageSize);
        ~CXL_EC_SYSTEM();

        void REQUEST(uint32_t addr, char type);
        void STORE(uint32_t PFN, int32_t section);
        void LOAD(uint32_t PFN, int32_t section);
        void MIGRATE(uint64_t From, uint64_t To, char type); 
        void RECOVERY(uint32_t failure_dev);
        vector<uint32_t> CXL_MEM_Alloc(uint32_t PFN, int Dev);
        void DECODE(std::vector<uint32_t> Decode_CG);
        int* ENCODE(vector<uint32_t> victim_PFNs);
        void CALCULATE_PARITY(vector<vector<int>>& Original_Data, int* Parity);
        void addCodingGroup(const vector<uint32_t>& frames);
        uint32_t PFN_TRANSLATE(uint32_t PFN);
        vector<uint32_t> getCodingGroup(uint32_t frame);
        
        uint32_t MIGRATE_COUNTER;
};

#endif
