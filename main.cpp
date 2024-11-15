#include <stdio.h>
#include <stdint.h>
#include "cxlEC.h"
#include <set>
#include <string.h> // strlen 함수를 사용하기 위해 추가

#define PAGESIZE 4096

int main() {
    CXL_EC_SYSTEM cxl_eco(274877906944ULL, 4, 25769803776ULL, 8589934592ULL, PAGESIZE);  // local 256GB, 4 cxl dev, cxl dev 24GB, parity 8GB

    FILE *file = fopen("/home/kkm/trace", "r");
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    double fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("file size는 %lf bytes입니다.\n", fileSize);

    double percent = 0;
    double totalReadBytes = 0;
    int load_count = 0;
    int store_count = 0;
    int migrate_count = 0;

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if(line[0] == 'W'){ // Store
            char type = 'w';
            char *hexAddStr = strtok(line +1, " ");

            if(hexAddStr != NULL){
                uint32_t addr = strtoull(hexAddStr, nullptr, 16);
                if(addr > Local_End_Frame && addr < CXL_Start_Frame)
                    continue; 

                cxl_eco.REQUEST(addr, type);
            }
        }

        else if(line[0] == 'R'){ // Load
            char type = 'r';
            char *hexAddStr = strtok(line +1, " ");

            if(hexAddStr != NULL){
                uint64_t addr = strtoull(hexAddStr, nullptr, 16);
                if(addr > Local_End_Frame && addr < CXL_Start_Frame)
                    continue; 

                cxl_eco.REQUEST(addr, type);
            }
        }

        else if(line[0] == 'M'){ // Migration
            char type = 'm';
            char *from = strtok(line+1, "");
            char *to = strtok(nullptr, " ");

            if(from != NULL && to != NULL){
                uint64_t From = strtoull(from, nullptr, 16);
                if(From > Local_End_Frame && From < CXL_Start_Frame)
                    continue; 
                
                uint64_t To = strtoull(to, nullptr, 16);
                if(To > Local_End_Frame && To < CXL_Start_Frame)
                    continue; 

                cxl_eco.MIGRATE(From, To, type);
            }
        }

        else if(line[0] == 'F'){ // Recovery
            char type = 'r';

        }
        
        totalReadBytes += strlen(line); // 수정: fgets로 읽은 데이터의 실제 크기를 계산
        double progress = totalReadBytes / fileSize * 100;
        if (progress >= percent) {
            printf("%lf%% 진행되었습니다. (읽은 바이트: %lf / 전체 크기: %lf)\n", percent, totalReadBytes, fileSize);
            percent += 10;

            printf("==================================\n");
        }
    }
    printf("Load: %d\n",load_count);
    printf("Store: %d\n",store_count);
    printf("Migrate: %d\n", migrate_count);
    fclose(file);

    return 0;
}
