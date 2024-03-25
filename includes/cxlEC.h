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
        evict_count = 0;
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

    uint32_t evict_count;
};

class CXL_EC_SYSTEM{
    private:
        int _pageSize;
        uint64_t _Local_size;
        uint32_t _CXL_mem_num;
        uint64_t _CXL_mem_size;
        uint32_t _Local_Range; //The number of Local DRAM pages
        uint32_t _Local_Page_num;
        uint32_t _CXL_Page_num;
        uint32_t _PTE_size;
        std::queue<uint32_t> LOCAL_DRAM_free;
        std::queue<uint32_t>* CXL_DEVS_free;
        PAGE* LOCAL_DRAM;
        PAGE** CXL_DEVS;      

        PTE* pageTable;
        std::list<uint32_t> LRUlist; //coldest: front 
        CXL_SWITCH switch1;
        std::unordered_map<uint32_t, std::vector<uint32_t>> CGmap;
        std::unordered_map<uint32_t, std::vector<uint32_t>> Pmap;

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

        double calculateStandardDeviation(const std::vector<double>& data) {
            int n = data.size();
            if (n <= 1) {
                std::cerr << "표준편차를 계산하기 위해서는 최소한 두 개의 데이터가 필요합니다." << std::endl;
                return 0.0;
            }

            // 평균 계산
            double sum = 0.0;
            for (double val : data) {
                sum += val;
            }
            double mean = sum / n;

            // 제곱 합 계산
            double sq_sum = 0.0;
            for (double val : data) {
                sq_sum += (val - mean) * (val - mean);
            }

            // 표준편차 계산
            double variance = sq_sum / n;
            return std::sqrt(variance);
        }

        void PrintfreeCXL(){
            std::vector<double> sigma;
            printf("LOCAL DRAM available pages: %ld, percentage: %lf%%\n", LOCAL_DRAM_free.size(), (double)LOCAL_DRAM_free.size()/(_Local_Page_num)*100);
            for(int i = 0; i < _CXL_mem_num; ++i){
                printf("CXL DEV %d available pages: %ld, percentage: %lf%%\n", i, 
                CXL_DEVS_free[i].size(), (double)CXL_DEVS_free[i].size()/(_CXL_mem_size/_pageSize)*100);
                sigma.push_back((double)CXL_DEVS_free[i].size());
            }
            double std_dev = calculateStandardDeviation(sigma);
            printf("표준편차: %lf\n", std_dev);
        }

        void PrintPageTable(){
            for(uint32_t i = 0; i < _PTE_size; ++i){
                if(pageTable[i].getvalid()){
                    printf("VPN %d evicted %d times\n", i, pageTable[i].evict_count);
                }
            }
        }


        std::set<uint32_t> VPNset; //debug
        std::map<uint32_t, uint32_t> VPNtoPFN; //debug
        uint32_t PF_PROMOTE_COUNTER;
        uint32_t EVICT_COUNTER;

};

#endif