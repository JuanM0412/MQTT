#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "../include/encode.h"
#include "../include/decode.h"
#include "../include/packet.h"
#include "../include/tree.h"
#include "../include/server.h"

// Function to get the singleton tree instance
Tree* get_tree() {
    static Tree singleton;
    static int initialized = 0;

    if (!initialized) {
        singleton.tree = createTreeNode("/");
        pthread_mutex_init(&singleton.mutex, NULL);
        initialized = 1;
    }

    return &singleton;
}

// Function to handle the tree based on the received topic and message
void handle_tree(int request, const char *topic, const char *message) {
    Tree *singleton_tree = get_tree();

    pthread_mutex_lock(&singleton_tree->mutex);

    publish(singleton_tree->tree, topic, message);
    printTree(singleton_tree->tree, 0);

    pthread_mutex_unlock(&singleton_tree->mutex);
}

// Function to disconnect a client
void disconnect_client(int connfd) {
    close(connfd);
    printf("Connection closed.");
}

// Function to handle the subscribe packet
void handle_subscribe_packet(MQTT_Packet packet, int connfd) {
    for (int i = 0; i < packet.remaining_length; ) {
        // Read topic length
        int topic_length = (packet.payload[i] << 8) | packet.payload[i + 1];

        // Read topic
        char *topic = malloc(topic_length);
        for (int j = 0; j < topic_length; j++) {
            topic[j] = packet.payload[i + 2 + j];
        }
        printf("Topic: %s\n", topic);
        free(topic);
        printf("\n");

        // Move to the next topic
        i += 2 + topic_length;
    }

    disconnect_client(connfd);
}

// Function to handle the publish packet
void handle_publish_packet(MQTT_Packet packet, int connfd) {
    size_t topic_length = (packet.variable_header[0] << 8) | packet.variable_header[1];
    size_t payload_length = packet.remaining_length - topic_length;
    printf("Topic length: %zu\n", topic_length);
    printf("Payload length: %zu\n", payload_length);

    size_t total_length = packet.remaining_length + strlen(packet.payload);
    unsigned char *serialized_packet = malloc(total_length);
    memcpy(serialized_packet, packet.variable_header, packet.remaining_length);
    memcpy(serialized_packet + packet.remaining_length, packet.payload, strlen(packet.payload));
    char *decoded_topic = decodeUTF8(serialized_packet + 2);
    char *decoded_message = decodeUTF8(serialized_packet + packet.remaining_length);

    for (int i = 0; i <= topic_length; i++) {
        if (decoded_topic[i] == '+' || decoded_topic[i] == '#') {
            printf("Wildcard character");
        }
    }
    
    handle_tree(1, decoded_topic, decoded_message);

    disconnect_client(connfd);
}

// Function to handle the connect packet
void handle_connect_packet(MQTT_Packet packet, int connfd) {
    size_t expected_length = 10 + (packet.variable_header[10] << 8) + packet.variable_header[11];

    if (packet.remaining_length != expected_length)
        printf("Remaining length");

    if (packet.variable_header[1] != 0x04 || packet.variable_header[2] != 'M' ||
        packet.variable_header[3] != 'Q' || packet.variable_header[4] != 'T' ||
        packet.variable_header[5] != 'T' || packet.variable_header[6] != 0x04)
        printf("Variable header");

    printf("CONNECT");

    disconnect_client(connfd);
}

// Function to identify the packet type and handle it accordingly
void identify_packet(MQTT_Packet packet, int connfd) {
    if (packet.fixed_header == MQTT_FIXED_HEADER_CONNECT)
        handle_connect_packet(packet, connfd);
    else if (packet.fixed_header == MQTT_FIXED_HEADER_PUBLISH)
        handle_publish_packet(packet, connfd);
    else if (packet.fixed_header == MQTT_FIXED_HEADER_SUBSCRIBE)
        handle_subscribe_packet(packet, connfd);
    else
        disconnect_client(connfd);
}

// Function to receive packet from client
MQTT_Packet receive_packet_from_client(int connfd) {
    MQTT_Packet received_packet;

    // Read data from socket
    unsigned char buffer[MAX];
    ssize_t bytes_received = read(connfd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        exit(EXIT_FAILURE);
    }

    // Parse data from buffer and fill MQTT_Packet structure
    size_t offset = 0;

    memcpy(&received_packet.fixed_header, buffer + offset, sizeof(received_packet.fixed_header));
    offset += sizeof(received_packet.fixed_header);

    memcpy(&received_packet.remaining_length, buffer + offset, sizeof(received_packet.remaining_length));
    offset += sizeof(received_packet.remaining_length);

    received_packet.variable_header = malloc(received_packet.remaining_length);
    memcpy(received_packet.variable_header, buffer + offset, received_packet.remaining_length);
    offset += received_packet.remaining_length;

    // Calculate payload size
    size_t payload_size = bytes_received - offset;
    if (received_packet.fixed_header == MQTT_FIXED_HEADER_SUBSCRIBE) {
        received_packet.payload = malloc(received_packet.remaining_length);
        memcpy(received_packet.payload, buffer + 2, received_packet.remaining_length);
    } else {
        received_packet.payload = malloc(payload_size);
        memcpy(received_packet.payload, buffer + offset, payload_size);
    }

    return received_packet;
}

// Function to process connection in a separate thread
void *process_connection(void *arg) { 
    int connfd = *((int*)arg);
    MQTT_Packet received_packet = receive_packet_from_client(connfd);

    // Free memory
    free_packet(&received_packet);
    
    return NULL;
}

// Main function 
int main(int argc, char *argv[]) { 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
    char ip[MAX], log_path[MAX];
    int port;

    if (argc == 4) {
        strcpy(ip, argv[1]);
        port = atoi(argv[2]);
        strcpy(log_path, argv[3]);
    } else
        return 1;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } else
        printf("Socket successfully created..\n"); 

    printf("Number socket: %d" ,sockfd);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(ip); 
    servaddr.sin_port = htons(port); 

    // Bind socket to IP and PORT
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } else
        printf("Socket successfully binded..\n"); 

    // Listen for incoming connections
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else
        printf("Server listening..\n"); 

    len = sizeof(cli); 
    Tree *singleton_tree = get_tree();

    while (1) {
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if (connfd < 0) { 
            printf("server accept failed...\n"); 
            exit(0); 
        } else
            printf("server accept the client...\n"); 
        
        pthread_t tid;
        printf("Thread ID: %lu\n", (unsigned long)tid);
        pthread_create(&tid, NULL, process_connection, &connfd);
    }

    close(sockfd);
    return 0;
}