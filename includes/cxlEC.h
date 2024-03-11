#ifndef CXL_EC_H
#define CXL_EC_H

#include <cstdlib>
#include <cstdint>
#include <list>
#include <any>
#include <vector>
#include <unordered_map>
#include <queue>
#include "ConsistentHashing.h"

class SWITCH;

class PAGE{
    //size: 4KB
    public:
    char* _page;
    //uint32_t PFN;

    PAGE(){
        _page = new char[4096];
        //this->PFN = PFN;
    }
    ~PAGE(){
        delete[] _page;
    }
};

class PTE{
    public:
    uint32_t PFN;
    bool isCXL;
    PTE(){
        PFN = 0;
        isCXL = 0;
    }
};

class CXL_EC_SYSTEM{
    private:
        int _pageSize;
        int _Local_size;
        int _CXL_mem_num;
        int _CXL_mem_size;
        uint32_t _Local_Range; //The number of Local DRAM pages
        std::queue<uint32_t> LOCAL_DRAM_free;
        std::queue<uint32_t>* CXL_DEVS_free;
        PAGE* LOCAL_DRAM;
        PAGE** CXL_DEVS;      

        PTE* pageTable;
        std::list<uint32_t>LRUlist; //coldest: front 
        SWITCH switch1;
        SWITCH switch2;
        SWITCH switch3;
        std::unordered_map<uint32_t, std::vector<uint32_t>> CGmap;

    public:
        CXL_EC_SYSTEM(int Local_size, int CXL_mem_num, int CXL_mem_size, int pageSize=4096);
        ~CXL_EC_SYSTEM();

        void MMU(uint32_t addr, char type);
        void STAMP(uint32_t PFN);
        void STORE(uint32_t PFN, uint32_t offset);
        void LOAD(uint32_t PFN, uint32_t offset);
        void PF_PROMOTE(uint32_t VPN); //migrate pages accessing now to Local
        std::vector<uint32_t> GET_CGmap(uint32_t VPN);
        void EVICT_COLD();
};

#endif