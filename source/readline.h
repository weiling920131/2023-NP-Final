#pragma once

#include    <stdio.h>
#include    <unistd.h>
#include    <string>
#include    <cstring>

using namespace std;

string receivedData;
ssize_t readline(int fd, char* buffer, size_t bufferSize) {
    while (receivedData.size() < bufferSize) {
        size_t newlinePos = receivedData.find('\n');
        if (newlinePos != string::npos) {
            string line = receivedData.substr(0, newlinePos + 1);

            receivedData.erase(0, newlinePos + 1);

            if (line.find('\n') != string::npos) {
                strcpy(buffer, line.c_str());
                break;
            }
        }
        size_t bytesRead = read(fd, buffer, 1);
        if (bytesRead <= 0) {
            return bytesRead;
        }
        buffer[bytesRead] = 0;

        receivedData.append(buffer, bytesRead);
    }
    return strlen(buffer);
}