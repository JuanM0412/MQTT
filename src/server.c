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
#include "../include/send_packets_to_client.h"
#include "../include/utils.h"
#include "../include/handle_tree.h"

FILE *log_file = NULL;
char serverIP[MAX];

void disconnect_client(int connfd) {
    close(connfd);
    printf("Connection closed.\n");
}

void handle_subscribe_packet(MQTT_Packet packet, int connfd) {
    int num_topics = 0;

    for (int i = 0; i < packet.remaining_length - 2; ) {
        // Read topic length
        int topic_length = (packet.payload[i] << 8) | packet.payload[i + 1];

        // Read topic
        char *topic = malloc(topic_length + 1);
        for (int j = 0; j < topic_length; j++) {
            topic[j] = packet.payload[i + 2 + j];
        }
        topic[topic_length] = '\0';
        insert_subscribe(topic, connfd);
        free(topic);

        // Move to the next topic
        i += 3 + topic_length;
        num_topics++;
    }
    
    unsigned int packet_id = (packet.variable_header[0] >> 8) | packet.variable_header[1];
    MQTT_Packet suback_packet = create_suback_packet(packet_id, num_topics);
    send_suback_to_client(connfd, suback_packet);

    free_packet(&suback_packet);
}

void handle_publish_packet(MQTT_Packet packet, int connfd) {
    size_t topic_length = (packet.variable_header[0] << 8) | packet.variable_header[1];
    size_t payload_length = packet.remaining_length - (topic_length + 4);

    char *topic = malloc(topic_length + 1);
    memcpy(topic, &packet.variable_header[2], topic_length);
    topic[topic_length] = '\0';

    for (int i = 0; i <= topic_length; i++) {
        if (topic[i] == '+' || topic[i] == '#') {
            printf("Wildcard character");
        }
    }

    char *message = (char *)packet.payload;
    insert_publish(topic, message);
    free(topic);
}

void handle_connect_packet(MQTT_Packet packet, int connfd) {
    u_int8_t return_code = 0x00;

    if (packet.variable_header[1] != 0x04 || packet.variable_header[2] != 'M' ||
        packet.variable_header[3] != 'Q' || packet.variable_header[4] != 'T' ||
        packet.variable_header[5] != 'T' || packet.variable_header[6] != 0x04)
        return_code = 0x01;

    MQTT_Packet connack_packet = create_connack_packet(return_code);
    send_connack_to_client(connfd, connack_packet);
}

void identify_packet(MQTT_Packet packet, int connfd) {
    if (packet.fixed_header == MQTT_FIXED_HEADER_CONNECT){
        handle_connect_packet(packet, connfd);
    }
    else if (packet.fixed_header == MQTT_FIXED_HEADER_PUBLISH){
        handle_publish_packet(packet, connfd);
    }
    else if (packet.fixed_header == MQTT_FIXED_HEADER_SUBSCRIBE)
        handle_subscribe_packet(packet, connfd);
    else{
        disconnect_client(connfd);
    }
}

MQTT_Packet receive_packet_from_client(int connfd) {
    MQTT_Packet received_packet;
    unsigned char buffer[MAX];

    memset(buffer, 0, MAX);

    ssize_t bytes_received = read(connfd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;
    size_t payload_length = 0;

    received_packet.fixed_header = buffer[offset++];
    received_packet.remaining_length = buffer[offset++];
    
    if (received_packet.fixed_header == MQTT_FIXED_HEADER_PUBLISH) {
        logger_server("Publish packet recieved", connfd);

        size_t topic_length = (buffer[offset++] << 8) | buffer[offset++];
        received_packet.variable_header = malloc(4 + topic_length);

        received_packet.variable_header[0] = topic_length >> 8;
        received_packet.variable_header[1] = topic_length & 0xFF;
        
        memcpy(&received_packet.variable_header[2], buffer + offset, topic_length);
        offset += topic_length;

        received_packet.variable_header[topic_length + 3] = (buffer[offset++] >> 8) & 0xFF;
        received_packet.variable_header[topic_length + 4] = buffer[offset++] & 0xFF;

        payload_length = received_packet.remaining_length - (topic_length + 4);
        received_packet.payload = malloc(payload_length);
        memcpy(received_packet.payload, buffer + offset, payload_length);

        received_packet.payload[payload_length] = '\0';

        printf("Contenido del payload:\n");
        for (size_t i = 0; i < payload_length; i++) {
            printf("%02X ", received_packet.payload[i]);
        }
        printf("\n");
    } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_CONNECT) {
        logger_server("Connect packet recieved", connfd);

        received_packet.variable_header = malloc(10);
        memcpy(received_packet.variable_header, buffer + offset, 10);
        offset += 10;

        payload_length = received_packet.remaining_length - 10;
        received_packet.payload = malloc(payload_length);
        memcpy(received_packet.payload, buffer + offset, payload_length);

        printf("Contenido del payload:\n");
        for (size_t i = 0; i < payload_length; i++) {
            printf("%02X ", received_packet.payload[i]);
        }
        printf("\n");
    } else if (received_packet.fixed_header == MQTT_FIXED_HEADER_SUBSCRIBE) {
        logger_server("Subscribe packet recieved", connfd);

        received_packet.variable_header = malloc(2);

        received_packet.variable_header[0] = buffer[offset++];
        received_packet.variable_header[1] = buffer[offset++];

        payload_length = received_packet.remaining_length - 2;
        received_packet.payload = malloc(payload_length);
        memcpy(received_packet.payload, buffer + offset, payload_length);
        received_packet.payload[payload_length] = '\0';

        printf("Contenido del payload:\n");
        for (size_t i = 0; i < payload_length; i++) {
            printf("%02X ", received_packet.payload[i]);
        }
        printf("\n");
    }

    return received_packet;
}

// Function to process connection in a separate thread
void *process_connection(void *arg) {
    int connfd = *((int*)arg);

    while (1) {
        MQTT_Packet received_packet = receive_packet_from_client(connfd);
        identify_packet(received_packet, connfd);

        free_packet(&received_packet);
    }
    
    return NULL;
}

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
        } else
            printf("server accept the client...\n"); 
        
        pthread_t tid;
        pthread_create(&tid, NULL, process_connection, &connfd);
    }

    fclose(log_file);
    close(sockfd);
    return 0;
}