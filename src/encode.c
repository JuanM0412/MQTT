#include <stdio.h>
#include <stdlib.h>
#include "../include/encode.h"

int necessaryBytes(unsigned int codePoint) {
    if (codePoint <= 0x7F) {
        return 1;
    } else if (codePoint <= 0x7FF) {
        return 2;
    } else if (codePoint <= 0xFFFF) {
        return 3;
    } else if (codePoint <= 0x10FFFF) {
        return 4;
    }
    return 0;
}

void encodeUTF8(unsigned int codePoint, unsigned char *buffer, int bytes) {
    switch (bytes) {
        case 1:
            buffer[0] = codePoint;
            break;
        case 2:
            buffer[0] = 0xC0 | (codePoint >> 6);
            buffer[1] = 0x80 | (codePoint & 0x3F);
            break;
        case 3:
            buffer[0] = 0xE0 | (codePoint >> 12);
            buffer[1] = 0x80 | ((codePoint >> 6) & 0x3F);
            buffer[2] = 0x80 | (codePoint & 0x3F);
            break;
        case 4:
            buffer[0] = 0xF0 | (codePoint >> 18);
            buffer[1] = 0x80 | ((codePoint >> 12) & 0x3F);
            buffer[2] = 0x80 | ((codePoint >> 6) & 0x3F);
            buffer[3] = 0x80 | (codePoint & 0x3F);
            break;
    }
}

int trailBytes(unsigned char c) {
    if (c > 0xF7) return 0;
    else if (c >= 0xF0) return 3;
    else if (c >= 0xE0) return 2;
    else if (c >= 0xC0) return 1;
    else return 0;
}

unsigned char* encodeMessageToUTF8(const char *message) {
    unsigned char *buffer = NULL;
    int bufferLength = 0;
    int messageLength = 0;

    while (message[messageLength] != '\0') {
        unsigned int codePoint = 0;
        int bytes = trailBytes(message[messageLength]) + 1;
        switch (bytes) {
            case 1:
                codePoint = message[messageLength];
                break;
            case 2:
                codePoint = ((message[messageLength] & 0x1F) << 6) |
                            (message[messageLength + 1] & 0x3F);
                break;
            case 3:
                codePoint = ((message[messageLength] & 0x0F) << 12) |
                            ((message[messageLength + 1] & 0x3F) << 6) |
                            (message[messageLength + 2] & 0x3F);
                break;
            case 4:
                codePoint = ((message[messageLength] & 0x07) << 18) |
                            ((message[messageLength + 1] & 0x3F) << 12) |
                            ((message[messageLength + 2] & 0x3F) << 6) |
                            (message[messageLength + 3] & 0x3F);
                break;
        }
        messageLength += bytes;
        bufferLength += necessaryBytes(codePoint);
    }

    buffer = (unsigned char*)malloc(bufferLength + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Not memory.\n");
        exit(EXIT_FAILURE);
    }

    int offset = 0;
    messageLength = 0;
    while (message[messageLength] != '\0') {
        unsigned int codePoint = 0;
        int bytes = trailBytes(message[messageLength]) + 1;
        switch (bytes) {
            case 1:
                codePoint = message[messageLength];
                break;
            case 2:
                codePoint = ((message[messageLength] & 0x1F) << 6) |
                            (message[messageLength + 1] & 0x3F);
                break;
            case 3:
                codePoint = ((message[messageLength] & 0x0F) << 12) |
                            ((message[messageLength + 1] & 0x3F) << 6) |
                            (message[messageLength + 2] & 0x3F);
                break;
            case 4:
                codePoint = ((message[messageLength] & 0x07) << 18) |
                            ((message[messageLength + 1] & 0x3F) << 12) |
                            ((message[messageLength + 2] & 0x3F) << 6) |
                            (message[messageLength + 3] & 0x3F);
                break;
        }
        messageLength += bytes;
        int bytesRequired = necessaryBytes(codePoint);
        encodeUTF8(codePoint, buffer + offset, bytesRequired);
        offset += bytesRequired;
    }

    buffer[offset] = '\0';

    return buffer;
}