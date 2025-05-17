#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

#define MAX_BUF_SIZE 1024
#define QUIT 113

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void print_format(const char* prefix, const char* message) {
    cout << "[" << prefix << "] " << message << endl;
}

void setup_sockaddr_in(SOCKADDR_IN *sockAddr) {
    (*sockAddr).sin_family = AF_INET;
    (*sockAddr).sin_port = htons(8080);
    (*sockAddr).sin_addr.s_addr = htonl(INADDR_ANY);
}

struct client_ref {
    SOCKET sock;
    thread recv_thread;
    atomic<int> *cmd;

    client_ref(SOCKET s, atomic<int> *cmd): sock(s), cmd(cmd) {}
    
    void start_recv() {
        recv_thread = thread(&client_ref::recv_worker, this);
    }

    void recv_worker() {
        char buf[MAX_BUF_SIZE];
        int recv_len;

        while (cmd->load() != QUIT) {
            recv_len = recv(sock, buf, MAX_BUF_SIZE - 1, 0);
            if (recv_len > 0) {
                buf[recv_len] = '\0';
                print_format("recv", buf);
            } else {
                print_format("recv", "client disconnected.");
                closesocket(sock);
                break;
            }
        }
    }
};

vector<unique_ptr<client_ref>> clients;
//mutex _mutex;

void accept_worker(SOCKET *sock, SOCKADDR_IN *sockAddr, atomic<int>* cmd) {
    SOCKET client_sock;
    SOCKADDR_IN client_sockAddr = {};
    int sockAddr_size = sizeof(client_sockAddr); 

    while (cmd->load() != QUIT) {
        client_sock = accept(
            *sock, 
            (SOCKADDR*)&client_sockAddr, 
            &sockAddr_size
        );
        
        if (client_sock == INVALID_SOCKET) {
            print_format("accept_worker", "client sock accept failed.");
            continue;
        }

        //_mutex.lock();
        print_format("accept_worker", "new client connected.");

        unique_ptr<client_ref> new_ref = make_unique<client_ref>(client_sock, cmd);
        new_ref->start_recv();

        clients.push_back(move(new_ref));
        
        //_mutex.unlock();
    }
}

int main(int argc, char **argv) {
    WSAData wsaData;
    SOCKET sock;
    SOCKADDR_IN sockAddr;

    atomic<int> cmd(0);

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

    if (bind(sock, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
        print_format("main", "bind failed.");
        return -1;
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        print_format("main", "listen failed.");
        return -1;
    }

    print_format("main", "success server open");

    thread accept_thread(accept_worker, &sock, &sockAddr, &cmd);

    while (cmd.load() != QUIT) {
        int c = getchar();
        if (c != EOF) cmd = c;
    }

    closesocket(sock);
    accept_thread.join();

    //_mutex.lock();
    for (auto& client : clients) {
        if (client->recv_thread.joinable())
            client->recv_thread.join();
    }
    //_mutex.unlock();

    print_format("main", "success close server.");

    WSACleanup();
    return 0;
}