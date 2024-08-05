#include <stdio.h>
#include <stdint.h>
#include "cxlEC.h"
#include <set>
#include <string.h> // strlen 함수를 사용하기 위해 추가

#define PAGESIZE 4096

int main() {
    CXL_EC_SYSTEM cxl_eco(1048576ULL, 16, 262144ULL, PAGESIZE);

    FILE *file = fopen("/home/kkm/vlog.vout", "r");
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

    uint32_t old_PF = 0;
    uint32_t old_Evict = 0;
    bool start_FLAG = 0;
    char type;
    uint64_t addr;
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '[' && (line[1] == 'D' || (line[1] == 'R' || line[1] == 'W'))) {
            type = (line[2] == 'W') ? 'w' : 'r';
            sscanf(line + 4, "%lx", &addr);

            //parsing ends
            //printf("c = %c, addr = %lx\n", type, addr);
            cxl_eco.MMU(addr, type);

            // 진행률 확인
        }
        totalReadBytes += strlen(line); // 수정: fgets로 읽은 데이터의 실제 크기를 계산
        double progress = totalReadBytes / fileSize * 100;
        if (progress >= percent) {
            printf("%lf%% 진행되었습니다. (읽은 바이트: %lf / 전체 크기: %lf)\n", percent, totalReadBytes, fileSize);
            // percent를 10 증가시킴으로써 다음 10%를 검사할 준비를 합니다.
            percent += 10;
            printf("page fault는 과연?: 누적 %d번 발생, 이전과 %d번 차이남\n", cxl_eco.PF_PROMOTE_COUNTER, cxl_eco.PF_PROMOTE_COUNTER-old_PF);
            old_PF = cxl_eco.PF_PROMOTE_COUNTER;
            printf("누적 Eviction %d, 이전과 %d번 차이남\n", cxl_eco.EVICT_COUNTER, cxl_eco.EVICT_COUNTER-old_Evict);
            old_Evict = cxl_eco.EVICT_COUNTER;
            printf("====Load balancing====\n");
            cxl_eco.PrintfreeCXL();
        }
    }
    cxl_eco.PrintPageTable();
    fclose(file);

    return 0;
}
