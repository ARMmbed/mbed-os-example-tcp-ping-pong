#include "mbed.h"
#include "easy-connect.h"

#define HOST_IP			"192.168.43.69"
#define HOST_PORT		1885

TCPSocket socket;

// Receive data from the server
void receiveTCP() {
    // Allocate 2K of data
    char* data = (char*)malloc(2048);
    while (1) {
        // recvfrom blocks until there is data
        nsapi_size_or_error_t size = socket.recv(data, 2048);
        if (size <= 0) {
            if (size == NSAPI_ERROR_WOULD_BLOCK) {  // Would block... that's fine (no data on the line)
                wait_ms(100);
                continue;
            }

            printf("Error while receiving data from TCP socket, probably it's closed now? (%d)\r\n", size);
            // @todo: re-connect
            break;
        }

        // turn into valid C string
        data[size] = '\0';

        printf("Received %d bytes from TCP socket (%s)\r\n", size, data);
    }
}

int main() {
    printf("Welcome to Mbed TCP ping pong\n");

    NetworkInterface *network = easy_connect(true);
    if (!network) {
        printf("Failed to get a network connection... See serial output.\n");
        return 1;
    }

    // Open a TCP socket
    nsapi_error_t rt = socket.open(network);
    if (rt != NSAPI_ERROR_OK) {
        printf("Could not open TCP Socket (%d)\r\n", rt);
        return 1;
    }
    printf("Opened a TCP socket\n");

    socket.set_blocking(false);
    rt = socket.connect(HOST_IP, HOST_PORT);
    if (rt != NSAPI_ERROR_OK) {
        printf("Could not connect TCP socket (%d)\r\n", rt);
    }

    Thread socket_thread;
    socket_thread.start(&receiveTCP);

    while (1) {
        char buffer[] = "ping";
        socket.send(buffer, strlen(buffer));

        printf("Sent %d bytes over TCP socket (%s)\n", strlen(buffer), buffer);
        wait_ms(2000);
    }

    wait_ms(osWaitForever);
}
