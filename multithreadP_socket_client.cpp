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
#include <atomic>
#include <limits>

// #define PORT 8081
#define MAX_LEN 200

using namespace std;

// TODO: Fix naming issue

int clientSocket;
atomic<bool> exit_flag(false); // read and written by both threads
pthread_t t_send, t_recv;

void catch_ctrl_c(int signal);
int  eraseText(int code);
void *send_message(void *socket);
void *recv_message(void *socket);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "usage: port" <<  endl;
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
        cout << "Socket creation success..." << endl;
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
        cout << "Connection success..." << endl;
    }

    cout << "Connected to chatroom!" << endl;

    signal(SIGPIPE, SIG_IGN); // sending after the server closes shouldn't crash the client
    signal(SIGINT, catch_ctrl_c); // catching the ctrl c and respond to the client quitting, if not, it will leave no message that they have quit
    char name[MAX_LEN] = {0}; // buffer
    cout << "Enter your name: "; // the MAX_LEN is for buffering
    cin.getline(name, MAX_LEN);
    if (cin.fail() && !cin.eof()) {
        // name was too long: keep what fit and drop the rest of the line
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    send(clientSocket, name, sizeof(name), 0);

    cout << " ======== Welcome to Chatroom ======== " << endl;

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

// only uses calls that are safe inside a signal handler (no cout, locks, or pthread_cancel)
void catch_ctrl_c(int signal)
{
    const char exit_message[] = "#exit";
    send(clientSocket, exit_message, strlen(exit_message), 0);
    close(clientSocket);

    const char leave_message[] = "\n You have left the chat.\n";
    write(STDOUT_FILENO, leave_message, strlen(leave_message));
    _exit(0);
}

// send messages
void *send_message(void *socket) {
    int clientSocket = *(int *)socket;

    while (1) {
        char message[MAX_LEN];
        cout << "You: ";
        cin.getline(message, MAX_LEN);

        if (cin.fail() && cin.eof()) {
            strcpy(message, "#exit"); // Ctrl+D (end of input) leaves the chat
        } else if (cin.fail()) {
            // line was too long: getline kept the first MAX_LEN - 1 characters,
            // so clear the error and drop the rest or every later read fails
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        if (exit_flag) break; // server closed the connection while we were typing

        if (send(clientSocket, message, strlen(message), 0) < 0) {
            exit_flag = true;
            break;
        }

        if (strcmp(message, "#exit") == 0) {
            exit_flag = true;
            shutdown(clientSocket, SHUT_RDWR); // wakes up the receive thread so it can exit on its own
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

        // the server sends every message as exactly MAX_LEN bytes, so wait for all of them
        char message[MAX_LEN + 1];
        int bytes_received = recv(clientSocket, message, MAX_LEN, MSG_WAITALL);

        if (exit_flag) break;

        if (bytes_received > 0) {
            message[bytes_received] = '\0'; // Null-terminate the string
            cout << "\r " << message << endl;
            cout << "You: ";
            fflush(stdout);
        } else {
            // Connection closed by the server (or an error)
            cout << "\nServer has closed the connection." << endl;
            exit_flag = true;
            break;
        }
    }
    return NULL;
}