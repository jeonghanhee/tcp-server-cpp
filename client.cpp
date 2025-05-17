#include <iostream>
#include <WinSock2.h>

#define MAX_BUF_SIZE 1024

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void print_format(const char* prefix, const char* message) {
    cout << "[" << prefix << "] " << message << endl;
}

void setup_sockaddr_in(SOCKADDR_IN *sockAddr) {
    (*sockAddr).sin_family = AF_INET;
    (*sockAddr).sin_port = htons(8080);
    (*sockAddr).sin_addr.s_addr = inet_addr("127.0.0.1");
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN sockAddr;

    char buf[MAX_BUF_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        print_format("main", "WSAStartup failed.");
        return -1;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        print_format("main", "socket creation failed.");
        WSACleanup();
        return -1;
    }

    setup_sockaddr_in(&sockAddr);

    if (connect(sock, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
        print_format("main", "connect failed.");
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    print_format("main", "connected to server.");

    // Send message.

    const char* msg = "Hello from client!";
    send(sock, msg, strlen(msg), 0);

    recv(sock, buf, MAX_BUF_SIZE - 1, 0);

    closesocket(sock);
    WSACleanup();
    return 0;
}