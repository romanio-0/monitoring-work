#include <iostream>
#include <winsock2.h>
#include <thread>
#include <chrono>
#include <vector>

#include "image.h"
#include "sup.h"

using namespace std;

void clientThreadRead(vector<clientData> &client, vector<clientData>::iterator itClient) {
    cout << "client!" << endl;

    char buffer[1024];
    int bytesRead;

    while (true) {
        bytesRead = recv(itClient->socket, buffer, 1024, 0);
        if (bytesRead == SOCKET_ERROR) {
            cout << "error read client" << endl;

            addLastTimeClient(itClient->userName, itClient->compName);
            client.erase(itClient);

            return;

        } else if (bytesRead == 0) {
            cout << "disconect" << endl;

            addLastTimeClient(itClient->userName, itClient->compName);
            client.erase(itClient);

            return;
        } else if (buffer[0] == CODE_SCREEN) {
            if (itClient->buffer == nullptr) {
                continue;
            }

            recv(itClient->socket, itClient->buffer, itClient->sizeBuf, 0);
            this_thread::sleep_for(chrono::milliseconds(100));
            recv(itClient->socket, itClient->buffer, itClient->sizeBuf, 0);
        }
    }

}

void connectClient(vector<clientData> &clientDataArr, vector<thread> &clientThread, SOCKET serverSock) {
    clientData tmpClientData;

    while (true) {
        memset(&tmpClientData, 0, sizeof(clientData));

        tmpClientData.socket = accept(serverSock, nullptr, nullptr);
        if (tmpClientData.socket == INVALID_SOCKET) {
            cout << "Failed connect!\nError code: " << WSAGetLastError() << endl;
            continue;
        }

        if (!getClientData(tmpClientData)) {
            continue;
        }

        if (!checkClientAvail(tmpClientData.userName, tmpClientData.compName)) {
            saveClient(tmpClientData);
        }

        clientDataArr.push_back(tmpClientData);
        clientThread.emplace_back(clientThreadRead, ref(clientDataArr), clientDataArr.end() - 1);
    }
}

screenData *getScreen(vector<clientData>::iterator itClient) {
    screenData *screen;
    if (itClient->buffer != nullptr) {
        delete[] itClient->buffer;
        itClient->buffer = nullptr;
    }

    itClient->sizeBuf = sizeof(screenData);
    itClient->buffer = new char[itClient->sizeBuf]{127};

    int bytesSent = send(itClient->socket, "screen", (int) strlen("screen"), 0);
    if (bytesSent == SOCKET_ERROR) {
        delete[] itClient->buffer;
        itClient->buffer == nullptr;
        return nullptr;
    } else if (bytesSent == 0) {
        delete[] itClient->buffer;
        itClient->buffer == nullptr;
        return nullptr;
    }

    for (int i = 0; itClient->buffer[0] == 127; ++i) {
        this_thread::sleep_for(chrono::milliseconds(10));
        if (i >= 100) {
            delete[] itClient->buffer;
            itClient->buffer == nullptr;
            return nullptr;
        }
    }

    screen = (screenData *) itClient->buffer;

    itClient->sizeBuf = screen->size;
    itClient->buffer = new char[screen->size]{127};

    for (int i = 0; itClient->buffer[0] == 127; ++i) {
        this_thread::sleep_for(chrono::milliseconds(10));
        if (i >= 100) {
            delete[] (char *) screen;
            delete[] itClient->buffer;
            itClient->buffer == nullptr;
            return nullptr;
        }
    }

    screen->data = (::byte *) itClient->buffer;
    itClient->buffer == nullptr;

    return screen;
}

void printListClient(vector<clientData> const &clientArr) {
    int i = 1;
    for (auto &it: clientArr) {
        wcout << i << L" - "
              << it.domain << L"/"
              << it.compName << L"/"
              << it.ip << L"/"
              << it.userName << " - online" << endl;
        i++;
    }
}

void printListAllClient(vector<clientData> const &clientArr) {
    fstream file("sup.txt", ios::binary | ios::in);
    clientOnlyData clientFile;
    time_t t;
    tm *now;

    while (file.read((char *) &clientFile, SIZE_CLIENT_DATA)) {
        int i = 1;
        bool check = true;

        for (auto &it: clientArr) {
            if (!wcscmp(it.compName, clientFile.compName)) {
                if (!wcscmp(it.userName, clientFile.userName)) {
                    wcout << L"- " << it.domain << L"/"
                          << it.compName << L"/"
                          << it.ip << L"/"
                          << it.userName << L" - online" << endl;

                    check = false;
                    break;
                }
            }
            i++;
        }

        if (check) {
            file.read((char *) &t, sizeof(time_t));
            now = localtime(&t);
            wcout << L"- " << clientFile.domain << L"/"
                  << clientFile.compName << L"/"
                  << clientFile.ip << L"/"
                  << clientFile.userName << L" - "
                  << now->tm_mday << L"." << now->tm_mon + 1 << L" "
                  << now->tm_hour << L":" << now->tm_min << endl;
        } else {
            file.seekp(sizeof(time_t), file.cur);
        }
    }

    file.close();
}

int main() {
    setlocale(LC_ALL, "");

    WSAData wsaData = {0};
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    char *strIP = getIP();
    if (strIP == nullptr) {
        cout << "Failed get IP" << endl;
        return 1;
    }
    cout << strIP << endl;

    SOCKET serverSock;
    vector<clientData> clientDataArr;
    vector<thread> clientThreadArr;
    sockaddr_in serverAd;

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        cout << "Error code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    serverAd.sin_family = AF_INET;
    serverAd.sin_port = htons(PORT);
    serverAd.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to the server address
    if (bind(serverSock, (sockaddr *) &serverAd, sizeof(serverAd)) == SOCKET_ERROR) {
        cout << "Error code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 2;
    }

    //listening to incoming connections
    if (listen(serverSock, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Error code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 3;
    }

    thread connect(connectClient, ref(clientDataArr), ref(clientThreadArr), serverSock);

    string readCons;
    thread *threadWindow;
    while (true) {
        cout << ">>";
        readCons.clear();
        getline(cin, readCons);

        if (readCons == "list") {
            printListClient(clientDataArr);

        } else if (readCons == "full list") {
            printListAllClient(clientDataArr);

        } else if (readCons == "screen") {
            cout << "write number client -";
            readCons.clear();
            getline(cin, readCons);

            int num;
            try {
                num = stoi(readCons) - 1;
            } catch (exception &ex) {
                cout << "Enter valid value" << endl;
                continue;
            }
            if (clientDataArr.size() - 1 < num){
                cout << "there is no such client" << endl;
                continue;
            }

            screenData *imageScreen = getScreen(clientDataArr.begin() + num);


            if (imageScreen == nullptr) {
                cout << "error get screen" << endl;
                continue;
            }

            threadWindow = new thread(printScreen, imageScreen);
            threadWindow->detach();
            delete threadWindow;

            cout << "save?\n[y] - yes | [n] - no\n>>";
            readCons.clear();
            getline(cin, readCons);
            if (readCons[0] == 'y') {
                wstring wreadCons;
                cout << "paht(*.png) -";
                readCons.clear();
                getline(wcin, wreadCons);
                saveScreen(imageScreen, wreadCons.c_str());
            }
        }
    }

    WSACleanup();
    return 0;
}
