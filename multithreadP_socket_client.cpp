#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <mutex>

#define MAX_LEN 200
#define NUM_COLORS 5

using namespace std;

// TODO: Fix naming issue

int clientSocket;
bool exit_flag = false;
pthread_t t_send, t_recv;

string norm_color = "\033[0m"; // normal color
string error_color = "\033[31m"; // error color
string success_color = "\033[32m";
string colors[] = {"\033[32m", "\033[33m", "\033[34m", "\033[35m","\033[36m"};

string color(int code);
void catch_ctrl_c(int signal);
int  eraseText(int code);
void *send_message(void *socket);
void *recv_message(void *socket);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << error_color << "usage: port" << norm_color << endl;
        exit(0);
    }

    int PORT = atoi(argv[1]);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1)
    {
        perror("socket");
        exit(-1);
    }
    else
    {
        cout << success_color << "Socket creation success..." << norm_color << endl;
    }

    sockaddr_in clientAddress;
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(PORT);
    clientAddress.sin_addr.s_addr = INADDR_ANY;
    memset(clientAddress.sin_zero, 0, sizeof(clientAddress.sin_zero)); // init the bytes

    if ((connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(sockaddr_in))) == -1)
    {
        perror("connect");
        exit(-1);
    }
    else
    {
        cout << success_color << "Connection success..." << norm_color << endl;
    }

    cout << "Connected to chatroom!" << endl;

    signal(SIGINT, catch_ctrl_c); // catching the ctrl c and respond to the client quitting, if not, it will leave no message that they have quit
    
    char name[MAX_LEN]; // buffer
    cout << "Enter your name: "; // the MAX_LEN is for buffering
    cin.getline(name, MAX_LEN);
    send(clientSocket, name, sizeof(name), 0);

    cout << colors[NUM_COLORS-1] << " ======== Welcome to Chatroom ======== " << endl << norm_color;

    // Create send and receive threads
    // pthread_create(&t_send, NULL, send_message, (void *)pclientSocket);
    pthread_create(&t_send, NULL, send_message, &clientSocket);
    pthread_create(&t_recv, NULL, recv_message, &clientSocket);

    // Join threads
    pthread_join(t_send, NULL);
    pthread_join(t_recv, NULL);

    close(clientSocket);
    return 0;
}

string color(int code)
{
    return colors[code % NUM_COLORS];
}

void catch_ctrl_c(int signal)
{
    char exit_message[MAX_LEN] = "#exit";
    send(clientSocket, exit_message, strlen(exit_message), 0);
    exit_flag = true;
    pthread_cancel(t_send);
    pthread_cancel(t_recv);
    close(clientSocket);
    cout << "\n You have left the chat." << endl;
    exit(signal);
}

// send messages
void *send_message(void *socket) {
    int clientSocket = *(int *)socket;

    while (1) {
        char message[MAX_LEN];
        cout << "You: "; // add
        cin.getline(message, MAX_LEN);

        send(clientSocket, message, strlen(message), 0);

        if (strcmp(message, "#exit") == 0) {
            exit_flag = true;
            pthread_cancel(t_recv);
            close(clientSocket);
            break;
        }
    }
    return NULL;
}

// receive messages
void *recv_message(void *socket) {
    int clientSocket = *(int *)socket;

    while (1) {
        if (exit_flag) break;

        char message[MAX_LEN];
        int bytes_received = recv(clientSocket, message, sizeof(message), 0);

        if (bytes_received > 0) {
            message[bytes_received] = '\0'; // Null-terminate the string
            cout << "\r" << message << endl;
            cout << "You: ";
            fflush(stdout);
        } else if (bytes_received == 0) {
            // Connection closed by the server
            cout << "\nServer has closed the connection." << endl;
            exit_flag = true;
            break;
        }
    }
    return NULL;
}