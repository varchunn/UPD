#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

using namespace std;

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

void client(const string &nickname, int color) {
    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVER_PORT);
    
    string join_msg = "JOIN:" + nickname;
    sendto(client_sock, join_msg.c_str(), join_msg.size(), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    thread recv_thread([&]() {
        char buffer[BUFFER_SIZE];
        sockaddr_in from;
        socklen_t from_len = sizeof(from);
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            recvfrom(client_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&from, &from_len);
            cout << buffer << endl;
        }
    });
    
    string message;
    while (true) {
        getline(cin, message);
        if (message == "exit") break;
        time_t now = time(0);
        char time_str[10];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", localtime(&now));
        string full_msg = "[" + string(time_str) + "] " + nickname + ": " + message;
        sendto(client_sock, full_msg.c_str(), full_msg.size(), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }
    
    string leave_msg = "LEAVE:" + nickname;
    sendto(client_sock, leave_msg.c_str(), leave_msg.size(), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    close(client_sock);
    recv_thread.detach();
}

int main() {
    cout << "Виберіть режим (1 - сервер, 2 - клієнт): ";
    int choice;
    cin >> choice;
    cin.ignore();
    
    if (choice == 1) {
        server();
    } else {
        string nickname;
        int color;
        cout << "Введіть нікнейм: ";
        getline(cin, nickname);
        cout << "Введіть номер кольору (1-15): ";
        cin >> color;
        cin.ignore();
        client(nickname, color);
    }
    return 0;
}

