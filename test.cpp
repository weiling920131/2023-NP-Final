#include "ansi.h"

#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

int main() {
    vector<int> emptyBoard(25, 2);
    char sendline[4096];
    init();
    printBoard(emptyBoard);
    printBoardPlayer();

    while(fgets(sendline, 4096, stdin)) {

    }

    return 0;
}