#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert and set the server's IP address
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported." << std::endl;
        close(client_socket);
        return -1;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Connection failed." << std::endl;
        close(client_socket);
        return -1;
    }

    std::cout << "Connected to the server!" << std::endl;

    // Communication loop
    while (true) {
        // Send message to the server
        std::cout << "You: ";
        std::string message;
        std::getline(std::cin, message);

        send(client_socket, message.c_str(), message.size(), 0);

        // Break if the user types "exit"
        if (message == "exit") {
            std::cout << "Disconnecting..." << std::endl;
            break;
        }

        // Receive complete message from the server
        std::cout << "Server: ";
        std::string received_message;
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                std::cout << "Server disconnected." << std::endl;
                close(client_socket);
                return 0;
            }

            // Append received data to the full message
            received_message.append(buffer, bytes_received);

            // Check for message termination (optional: server could send a delimiter, e.g., '\n')
            if (bytes_received < BUFFER_SIZE) { // All data has been received
                break;
            }
        }

        // Print the complete message
        std::cout << received_message << std::endl;
    }

    // Close the socket
    close(client_socket);
    return 0;
}