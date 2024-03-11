#include <stdio.h>
#include <stdint.h>
#include "cxlEC.h"

int main() {
    CXL_EC_SYSTEM cxl_eco(0, 0, 0);

    FILE *file = fopen("wa-128_128-fc10-fl100.vout", "r");
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return 1;
    }

    char type;
    uint64_t addr;

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '[' && (line[1] == 'D' && (line[2] == 'R' || line[2] == 'W'))) {
            type = (line[2] == 'W') ? 'w' : 'r';
            sscanf(line + 4, "%lx", &addr);

            //pasing ends
            printf("c = %c, addr = %lx\n", type, addr);
            
        }
    }

    fclose(file);

    return 0;
}