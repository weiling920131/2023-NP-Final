#include <stdio.h>
#include <stdlib.h>

int main() {
    // set to 80*25 color mode
    printf("\033[=3h");
    // erase screen
    printf("\033[2J");


    printf("\033[8;24;80t");

    printf("\033[3;37H");

    // printf("\033[2;10H");

    // // 輸出文字
    printf("\033[1;33mSLITHER\n\033[22m");

    while(1) {

    }

    return 0;
}