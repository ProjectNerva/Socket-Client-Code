#include <pthread.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 9090
#define MAX_LEN 200

using namespace std;

bool exit_flag = false;
pthread_t thread1;
pthread_t t_send, t_recv;

void * ThreadFunction(void *arguments);

void catch_ctrl_c(int signal);
void * send_message(void *socket);
void *recv_message(int *socket);

int main() {
  
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in clientAddress;
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(PORT);
    clientAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(clientAddress.sin_zero, 0, sizeof(clientAddress.sin_zero)); // init the bytes


    signal(SIGINT, catch_ctrl_c); // catching the ctrl c and respond to the client quitting, if not, it will leave no message that they have quit
    char name[MAX_LEN]; // buffer
    cout << "Enter your name: "; // the MAX_LEN is for buffering
    cin.getline(name, MAX_LEN);
    send(clientSocket, name, sizeof(name), 0);

    cout << " ======== Welcome to Chatroom ======== " << endl;
  // Creating the Location where the ID of the newly created 
  // thread will be stored

  printf("Calling Thread \n");
  
  // Creating the thread
  pthread_create(&thread1, NULL, ThreadFunction, NULL);
  pthread_create(&t_send, NULL, send_message, NULL);
  return 0;
}
void * ThreadFunction(void *arguments)
{
    printf("Thread Running\n");
    return NULL;
}

void catch_ctrl_c(int signal)
{

}

// Function to send messages
void * send_message(void *socket) {
    int clientSocket = *(int *)socket;

    while (1) {
        char message[MAX_LEN];
        cout << "You: ";
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

// Function to receive messages
void *recv_message(void *socket) {
    int clientSocket = *(int *)socket;

    while (1) {
        if (exit_flag) break;

        char message[MAX_LEN];
        int bytes_received = recv(clientSocket, message, sizeof(message), 0);

        if (bytes_received > 0) {
            message[bytes_received] = '\0'; // Null-terminate the string
            cout << "\rServer: " << message << endl;
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