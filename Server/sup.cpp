#include "sup.h"

using namespace std;

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

bool getClientData(clientData &client) {
    int bytesRead = recv(client.socket, ((char *) (&client)) + sizeof(SOCKET), sizeof(clientData) - sizeof(SOCKET), 0);
    if (bytesRead == SOCKET_ERROR) {
        cout << "Failed get info client" << endl;
        return false;
    } else if (bytesRead == 0) {
        cout << "disconect" << endl;
        return false;
    }

    return true;
}

void saveClient(clientData const &client) {
    ofstream file("sup.txt", ios::binary | ios::app);

    file.write((char *) &client + sizeof(SOCKET), SIZE_CLIENT_DATA);
    file.write("\0\0\0\0\0\0\0\0", sizeof(time_t));

    file.close();
}

bool checkClientAvail(const wchar_t *userName, const wchar_t *compName) {
    fstream file("sup.txt", ios::binary | ios::in | ios::out);
    clientOnlyData client;

    while (file.read((char *) &client, SIZE_CLIENT_DATA)) {
        if (!wcscmp(client.userName, userName)) {
            if (!wcscmp(client.compName, compName)) {
                file.close();
                return true;
            }
        }

        file.seekp(sizeof(time_t), file.cur);
    }

    file.close();
    return false;
}

void addLastTimeClient(const wchar_t *userName, const wchar_t *compName) {
    fstream file("sup.txt", ios::binary | ios::in | ios::out);
    clientOnlyData client;
    time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());

    while (file.read((char *) &client, SIZE_CLIENT_DATA)) {
        if (!wcscmp(client.userName, userName)) {
            if (!wcscmp(client.compName, compName)) {

                copy((char *) &t, (char *) &t + 8, ostreambuf_iterator<char> (file));
                file.close();
                return;
            }
        }

        file.seekp(sizeof(time_t), file.cur);
    }

    file.close();
}

clientData loadClient(clientData const &client) {


}