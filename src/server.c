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
#include "../include/handle_packets.h"
#include "../include/utils.h"

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

void insert_publish(const char *topic, const char *message) {
    Tree *singleton_tree = get_tree();

    pthread_mutex_lock(&singleton_tree->mutex);

    publish(singleton_tree->tree, topic, message);
    printTree(singleton_tree->tree, 0);

    pthread_mutex_unlock(&singleton_tree->mutex);
}

void insert_subscribe(const char *topic, int connfd) {
    Tree *singleton_tree = get_tree();

    pthread_mutex_lock(&singleton_tree->mutex);

    subscribe(singleton_tree->tree, topic, connfd, 0);
    printTree(singleton_tree->tree, 0);

    pthread_mutex_unlock(&singleton_tree->mutex);
}

void disconnect_client(int connfd) {
    close(connfd);
    printf("Connection closed.\n");
}

void handle_subscribe_packet(MQTT_Packet packet, int connfd) {
    printf("Remaining: %u\n", packet.remaining_length);
    int num_topics = 0;

    for (int i = 0; i < packet.remaining_length - 2; ) {
        // Read topic length
        printf("i (0): %u\n", i);
        int topic_length = (packet.payload[i] << 8) | packet.payload[i + 1];
        printf("Topic len: %u\n", topic_length);

        // Read topic
        char *topic = malloc(topic_length);
        for (int j = 0; j < topic_length; j++) {
            topic[j] = packet.payload[i + 2 + j];
        }
        printf("Topic: %s\n", decodeUTF8(topic));
        insert_subscribe(topic, connfd);
        free(topic);
        printf("\n");

        // Move to the next topic
        i += 3 + topic_length;
        num_topics++;
        printf("i (1): %u\n", i);
    }

    // unsigned int packet_id = (packet.variable_header[0] >> 8) | packet.variable_header[1];
    // MQTT_Packet suback_packet = create_suback_packet(packet_id, num_topics);
    // send_suback_to_client(connfd, suback_packet);

    // disconnect_client(connfd);
}

void handle_publish_packet(MQTT_Packet packet, int connfd) {
    size_t topic_length = (packet.variable_header[0] << 8) | packet.variable_header[1];
    size_t payload_length = packet.remaining_length - (topic_length + 4);

    char *topic = malloc(topic_length);
    memcpy(topic, &packet.variable_header[2], topic_length);

    char *payload = malloc(payload_length);
    memcpy(payload, packet.payload, payload_length);

    for (int i = 0; i <= topic_length; i++) {
        if (topic[i] == '+' || topic[i] == '#') {
            printf("Wildcard character");
        }
    }
    
    insert_publish(topic, payload);

    disconnect_client(connfd);
}

void handle_connect_packet(MQTT_Packet packet, int connfd) {
    printf("Fix3: %u\n", packet.fixed_header);
    u_int8_t return_code = 0x00;

    if (packet.variable_header[1] != 0x04 || packet.variable_header[2] != 'M' ||
        packet.variable_header[3] != 'Q' || packet.variable_header[4] != 'T' ||
        packet.variable_header[5] != 'T' || packet.variable_header[6] != 0x04)
        return_code = 0x01;

    printf("CONNECT2\n");

    MQTT_Packet connack_packet = create_connack_packet(return_code);
    send_connack_to_client(connfd, connack_packet);

    disconnect_client(connfd);
}

void identify_packet(MQTT_Packet packet, int connfd) {
    printf("identify_packet\n");
    if (packet.fixed_header == MQTT_FIXED_HEADER_CONNECT){
        printf("Fix2: %u\n", packet.fixed_header);
        handle_connect_packet(packet, connfd);
        printf("Fix4: %u\n", packet.fixed_header);
    }
    else if (packet.fixed_header == MQTT_FIXED_HEADER_PUBLISH){
        printf("Fix Pub (1): %u\n", packet.fixed_header);
        handle_publish_packet(packet, connfd);
        printf("Fix Pub (2): %u\n", packet.fixed_header);
    }
    else if (packet.fixed_header == MQTT_FIXED_HEADER_SUBSCRIBE)
        handle_subscribe_packet(packet, connfd);
    else{
        printf("DISCONNECT");
        disconnect_client(connfd);
    }
}

MQTT_Packet receive_packet_from_client(int connfd) {
    printf("receive_packet_from_client\n");
    MQTT_Packet received_packet;
    unsigned char buffer[MAX];

    ssize_t bytes_received = read(connfd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;
    size_t payload_length = 0;

    received_packet.fixed_header = buffer[offset++];
    received_packet.remaining_length = buffer[offset++];
    
    if (received_packet.fixed_header == MQTT_FIXED_HEADER_PUBLISH) {
        
        // logger("Publish packet recieved from client", serverIP, clientIP);

        printf("MQTT_FIXED_HEADER_PUBLISH\n");
        size_t topic_length = (buffer[offset++] << 8) | buffer[offset++];
        received_packet.variable_header = malloc(4 + topic_length);

        received_packet.variable_header[0] = topic_length >> 8;
        received_packet.variable_header[1] = topic_length & 0xFF;
        
        memcpy(&received_packet.variable_header[2], buffer + offset, topic_length);
        offset += topic_length;

        received_packet.variable_header[topic_length + 2] = (buffer[offset++] >> 8) & 0xFF;
        received_packet.variable_header[topic_length + 3] = buffer[offset++] & 0xFF;

        payload_length = received_packet.remaining_length - (topic_length + 4);
        received_packet.payload = malloc(payload_length);
        memcpy(received_packet.payload, buffer + offset, payload_length);
    } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_CONNECT) {
        received_packet.variable_header = malloc(10);
        memcpy(received_packet.variable_header, buffer + offset, 10);
        offset += 10;

        payload_length = received_packet.remaining_length - 10;
        received_packet.payload = malloc(payload_length);
        memcpy(received_packet.payload, buffer + offset, payload_length);
    } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_SUBSCRIBE) {
        received_packet.variable_header = malloc(2);

        received_packet.variable_header[0] = buffer[offset++];
        received_packet.variable_header[1] = buffer[offset++];

        payload_length = received_packet.remaining_length - 2;
        received_packet.payload = malloc(payload_length);
        memcpy(received_packet.payload, buffer + offset, payload_length);
    }

    return received_packet;
}

// Function to process connection in a separate thread
void *process_connection(void *arg) {
    printf("*process_connection\n");
    int connfd = *((int*)arg);
    MQTT_Packet received_packet = receive_packet_from_client(connfd);
    printf("Recibido\n");
    identify_packet(received_packet, connfd);

    //free_packet(&received_packet);
    
    return NULL;
}

char *get_socket_ip(int sock)
{
    struct sockaddr_in local_address;
    socklen_t address_length = sizeof(local_address);
    if (getsockname(sock, (struct sockaddr *)&local_address, &address_length) == -1)
    {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

    char *ip = inet_ntoa(local_address.sin_addr);
    return ip;
}


FILE *log_file = NULL;
char serverIP[MAX];


// Main function 
int main(int argc, char *argv[]) { 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
    char log_path[MAX];
    int port;

    if (argc == 4) {
        strcpy(serverIP, argv[1]);
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
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(serverIP); 
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
    } else {
        printf("Server listening..\n"); 
    }

    len = sizeof(cli); 
    Tree *singleton_tree = get_tree();
    log_file = fopen(log_path, "a");
    
    while (1) {
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if (connfd < 0) { 
            printf("server accept failed...\n"); 
            exit(0); 
        } else printf("server accept the client...\n"); 

        pthread_t tid;
        pthread_create(&tid, NULL, process_connection, &connfd);
    }

    fclose(log_file);
    close(sockfd);
    return 0;
}