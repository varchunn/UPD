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
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

vector<string> chat_history;
vector<pair<string, sockaddr_in>> clients;

void broadcastMessage(const string &message, int server_sock) {
    chat_history.push_back(message);
    for (auto &client : clients) {
        sendto(server_sock, message.c_str(), message.size(), 0, (struct sockaddr *)&client.second, sizeof(client.second));
    }
}

void server() {
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    char buffer[BUFFER_SIZE];
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        
        string message(buffer);
        if (message.substr(0, 5) == "JOIN:") {
            string nickname = message.substr(5);
            clients.push_back({nickname, client_addr});
            string join_msg = nickname + " приєднався до чату.";
            broadcastMessage(join_msg, server_sock);
            for (const auto &msg : chat_history) {
                sendto(server_sock, msg.c_str(), msg.size(), 0, (struct sockaddr *)&client_addr, addr_len);
            }
        } else if (message.substr(0, 6) == "LEAVE:") {
            string nickname = message.substr(6);
            string leave_msg = nickname + " вийшов із чату.";
            broadcastMessage(leave_msg, server_sock);
        } else {
            broadcastMessage(message, server_sock);
        }
    }
    close(server_sock);
}
