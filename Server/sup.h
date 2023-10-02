#ifndef _SUP_H_
#define _SUP_H_

#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <chrono>

#define PORT 1069
#define IP_LEN 16
#define CODE_SCREEN 0x01
#define SIZE_CLIENT_DATA sizeof(clientOnlyData)

struct clientData {
    SOCKET socket;
    char ip[IP_LEN + 1];
    wchar_t domain[256];
    wchar_t userName[256];
    wchar_t compName[MAX_COMPUTERNAME_LENGTH + 1];
    char* buffer = nullptr;
    long sizeBuf = 0;
};

struct clientOnlyData {
    char ip[IP_LEN + 1];
    wchar_t domain[256];
    wchar_t userName[256];
    wchar_t compName[MAX_COMPUTERNAME_LENGTH + 1];
};

struct screenData {
    unsigned long size;
    long width;
    long height;
    byte *data;
};

char *getIP();

bool getClientData(clientData &client);

void saveClient(clientData const&client);

bool checkClientAvail(const wchar_t *userName, const wchar_t *compName);

void addLastTimeClient(const wchar_t *userName, const wchar_t *compName);

clientData loadClient(clientData const&client);

#endif //_SUP_H_
