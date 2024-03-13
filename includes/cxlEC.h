#ifndef CXL_EC_H
#define CXL_EC_H

#include <cstdlib>
#include <cstdint>
#include <list>
#include <any>
#include <vector>
#include <unordered_map>
#include <queue>
#include "switch.h"

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
    private:
    uint32_t PFN;
    bool isCXL;
    bool valid;
    public:
    PTE(){
        //printf("set 0 0 0\n");
        PFN = 0;
        isCXL = 0;
        valid = 0;
    }
    bool getvalid(){
        return valid;
    }
    uint32_t getPFN(){
        return PFN;
    }
    bool getisCXL(){
        return isCXL;
    }
    void setPFN(uint32_t PFN){
        this->PFN = PFN;
        //printf("SET %d\n", this->PFN);

    }
    void setisCXL(bool isCXL){
        this->isCXL=isCXL;
    }
    void setvalid(bool valid){
        this->valid=valid;
    }
};

class CXL_EC_SYSTEM{
    private:
        int _pageSize;
        uint64_t _Local_size;
        uint32_t _CXL_mem_num;
        uint64_t _CXL_mem_size;
        uint32_t _Local_Range; //The number of Local DRAM pages
        std::queue<uint32_t> LOCAL_DRAM_free;
        std::queue<uint32_t>* CXL_DEVS_free;
        PAGE* LOCAL_DRAM;
        PAGE** CXL_DEVS;      

        PTE* pageTable;
        std::list<uint32_t> LRUlist; //coldest: front 
        CXL_SWITCH switch1;
        std::unordered_map<uint32_t, std::vector<uint32_t>> CGmap;

    public:
        CXL_EC_SYSTEM(uint64_t Local_size, uint32_t CXL_mem_num, uint64_t CXL_mem_size, int pageSize);
        ~CXL_EC_SYSTEM();

        void MMU(uint32_t addr, char type);
        void STAMP(uint32_t PFN);
        void STORE(uint32_t PFN, uint32_t offset);
        void LOAD(uint32_t PFN, uint32_t offset);
        void PF_PROMOTE(uint32_t VPN); //migrate pages accessing now to Local
        std::vector<uint32_t> GET_CGmap(uint32_t VPN);
        void EVICT_COLD();

        void PrintfreeCXL(){
            for(int i = 0; i < _CXL_mem_num; ++i){
                printf("CXL DEV %d available pages: %ld, percentage: %lf%%\n", i, 
                CXL_DEVS_free[i].size(), (double)CXL_DEVS_free[i].size()/(_CXL_mem_size/_pageSize)*100);
            }
        }

        std::set<uint32_t> VPNset; //debug
        std::map<uint32_t, uint32_t> VPNtoPFN; //debug
        uint32_t PF_PROMOTE_COUNTER;

};

#endif