# include <stdlib.h>
# include <ctype.h>
# include <string.h>
# include <stdio.h>
# include "../include/packet.h"
# include "../include/utils.h"
# include "../include/encode.h"

MQTT_Packet create_connect_packet() {

    MQTT_Packet connect_packet;

    connect_packet.fixed_header = 0x10;

    connect_packet.variable_header = malloc(10);

    connect_packet.variable_header[0] = 0x00;
    connect_packet.variable_header[1] = 0x04;
    connect_packet.variable_header[2] = 'M';
    connect_packet.variable_header[3] = 'Q';
    connect_packet.variable_header[4] = 'T';
    connect_packet.variable_header[5] = 'T';
    connect_packet.variable_header[6] = 0x04;
    connect_packet.variable_header[7] = 0XC2;
    connect_packet.variable_header[8] = 0x00;
    connect_packet.variable_header[9] = 0x00;

    char username[21], password[21];

    printf("Enter your username (up to 20 characters): ");
    fgets(username, sizeof(username), stdin);
    strtok(username, "\n");

    unsigned char *encoded_username = encodeMessageToUTF8(username);
    int encoded_username_length = strlen(username);
    
    printf("Enter your password (up to 20 characters): ");
    fgets(password, sizeof(password), stdin);
    strtok(password, "\n");

    unsigned char *encoded_password = encodeMessageToUTF8(password);
    int encoded_password_length = strlen(password);

    int payload_length = 1 + (1 + encoded_username_length) + (1 + encoded_password_length);

    connect_packet.payload = (unsigned char *)malloc(payload_length);
    if (connect_packet.payload == NULL) {
        printf("Memory allocation failed.\n");
        exit(0);
    }

    connect_packet.payload[0] = 0x00;
    
    connect_packet.payload[1] = encoded_username_length;
    memcpy(&connect_packet.payload[2], encoded_username, encoded_username_length);
    
    connect_packet.payload[1 + (1 + encoded_username_length)] = encoded_password_length;
    memcpy(&connect_packet.payload[1 + (1 + encoded_username_length) + 1], encoded_password, encoded_password_length);
    
    connect_packet.remaining_length = 10 + payload_length;

    return connect_packet;
}

MQTT_Packet create_publish_packet() {
    MQTT_Packet publish_packet;

    publish_packet.fixed_header = 0x30;
    
    unsigned int packet_id = get_packet_id();

    char answer, topic[255], message[20];

    printf("Enter your topic: ");
    fgets(topic, sizeof(topic), stdin);
    strtok(topic, "\n");

    unsigned char *encoded_topic = encodeMessageToUTF8(topic);
    int encoded_topic_length = 2 + strlen(topic);

    publish_packet.variable_header = (unsigned char *)malloc(encoded_topic_length);
    if (publish_packet.variable_header == NULL) {
        printf("Memory allocation failed.\n");
        exit(0);
    }

    publish_packet.variable_header[1] = encoded_topic_length & 0xFF;
    publish_packet.variable_header[0] = (encoded_topic_length >> 8) & 0XFF;

    memcpy(&publish_packet.variable_header[2], encoded_topic, encoded_topic_length);

    int variable_header_length = 2 + encoded_topic_length;

    printf("Enter your message: ");
    fgets(message, sizeof(message), stdin);
    strtok(message, "\n");

    unsigned char *encoded_message = encodeMessageToUTF8(message);
    int encoded_message_length = strlen(message);

    publish_packet.payload = (unsigned char *)malloc(encoded_message_length + 2);
    if (publish_packet.payload == NULL) {
        printf("Memory allocation failed.\n");
        exit(0);
    }

    publish_packet.payload[0] = (packet_id >> 8) & 0xFF;
    publish_packet.payload[1] = packet_id & 0xFF;

    memcpy(&publish_packet.payload[2], encoded_message, encoded_message_length);

    publish_packet.remaining_length = variable_header_length + encoded_message_length + 2; // Se suma 2 para los bytes del packet_id

    return publish_packet;
}


MQTT_Packet create_subscribe_packet() {
    MQTT_Packet subscribe_packet;

    subscribe_packet.fixed_header = 0x82;

    unsigned int packet_id = get_packet_id();

    subscribe_packet.variable_header = malloc(2);

    subscribe_packet.variable_header[1] = packet_id & 0xFF;
    subscribe_packet.variable_header[0] = (packet_id >> 8) & 0XFF;

    subscribe_packet.payload = NULL;
    subscribe_packet.remaining_length = 0;

    char topics[10][160 + 1];
    int num_topics;

    printf("How many topics do  you want to subscribe to? (up to 10): ");
    scanf("%d", &num_topics);
    getchar();

    if (num_topics < 1 || num_topics > 10) {
        printf("Invalid number of topics.\n");
        exit(0);
    }

    printf("Enter %d topics (each up to 160 characters):\n", num_topics);
    for (int i = 0; i < num_topics; i++) {
        printf("Topic %d: ", i + 1);
        fgets(topics[i], sizeof(topics[i]), stdin);
        strtok(topics[i], "\n");

        unsigned char *encoded_topic = encodeMessageToUTF8(topics[i]);
        strcpy(topics[i], encoded_topic);
        free(encoded_topic);
    }

    int payload_size = 0;
    for (int i = 0; i < num_topics; i++) {
        payload_size += 1 + strlen(topics[i]) + 1;
    }

    subscribe_packet.payload = (unsigned char *)malloc(payload_size);
    if (subscribe_packet.payload == NULL) {
        printf("Memory allocation failed.\n");
        return subscribe_packet;
    }

    int index = 0;
    for (int i = 0; i < num_topics; i++) {
        int topic_length = strlen(topics[i]);
        subscribe_packet.payload[index++] = (unsigned char)topic_length; 
        memcpy(&subscribe_packet.payload[index], topics[i], topic_length);
        subscribe_packet.payload[index + topic_length] = 0x00;
        index += topic_length + 1;
    }
    
    subscribe_packet.payload[payload_size - 1] = 0x00;

    subscribe_packet.remaining_length = 2 + payload_size;

    return subscribe_packet;
}

MQTT_Packet create_unsubscribe_packet() {

    MQTT_Packet unsubscribe_packet;

    unsubscribe_packet.fixed_header = 0xA2;
    
    unsigned int packet_id = get_packet_id();

    unsubscribe_packet.variable_header = malloc(2);

    unsubscribe_packet.variable_header[1] = packet_id & 0xFF;
    unsubscribe_packet.variable_header[0] = (packet_id >> 8) & 0XFF;

    unsubscribe_packet.payload = NULL;
    unsubscribe_packet.remaining_length = 0;

    char topics[10][160 + 1];
    int topics_amount;

    printf("How many topics do you want to unsubscribe to? (up to 10): ");
    scanf("%d", &topics_amount);
    getchar();

    while (topics_amount < 1 || topics_amount > 10) {
        printf("Enter a number between 1 and 10: ");
        scanf("%d", &topics_amount);
        getchar();
    }

    printf("Enter %d topics (each up to 160 characters):\n", topics_amount);
    for (int i = 0; i < topics_amount; i++) {
        printf("Topic %d: ", i + 1);
        fgets(topics[i], sizeof(topics[i]), stdin);
        strtok(topics[i], "\n");

        unsigned char *encoded_topic = encodeMessageToUTF8(topics[i]);
        strcpy(topics[i], encoded_topic);
        free(encoded_topic);
    }

    int payload_size = 0;
    for (int i = 0; i < topics_amount; i++) {
        payload_size += 1 + strlen(topics[i]) + 1;
    }

    unsubscribe_packet.payload = (unsigned char *)malloc(payload_size);
    if (unsubscribe_packet.payload == NULL) {
        printf("Memory allocation failed.\n");
        exit(0);
    }

    int index = 0;
    for (int i = 0; i < topics_amount; i++) {
        int topic_length = strlen(topics[i]);
        unsubscribe_packet.payload[index++] = (unsigned char)topic_length;
        memcpy(&unsubscribe_packet.payload[index], topics[i], topic_length);
        unsubscribe_packet.payload[index + topic_length] = 0x00;
        index += topic_length + 1;
    }

    unsubscribe_packet.remaining_length = 2 + payload_size;

    return unsubscribe_packet; 
}

MQTT_Packet create_disconnect_packet() {

    MQTT_Packet disconnect_packet;
    
    disconnect_packet.fixed_header = 0xE0;
    disconnect_packet.remaining_length = 0x00;
    disconnect_packet.variable_header = NULL;
    disconnect_packet.payload = NULL;

    return disconnect_packet;
    
}

void free_packet(MQTT_Packet* packet) {
  free(packet->variable_header);
  free(packet->payload);
}