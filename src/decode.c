#include <stdio.h>
#include <stdlib.h>
#include "../include/decode.h"

int bytesPerCharacterUTF8(unsigned char byte) {
    if ((byte & 0x80) == 0) {
        return 1;
    } else if ((byte & 0xE0) == 0xC0) {
        return 2;
    } else if ((byte & 0xF0) == 0xE0) {
        return 3;
    } else if ((byte & 0xF8) == 0xF0) {
        return 4;
    }
    return 0;
}

unsigned int decodeCharacterUTF8(unsigned char *bytes, int length) {
    unsigned int codepoint = 0;
    switch (length) {
        case 1:
            codepoint = bytes[0];
            break;
        case 2:
            codepoint = ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
            break;
        case 3:
            codepoint = ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
            break;
        case 4:
            codepoint = ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) | ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
            break;
    }
    return codepoint;
}

char* decodeUTF8(const unsigned char *bytes) {
    int strLength = 0;
    int i = 0;

    // Calcular la longitud de la cadena decodificada
    while (bytes[i] != '\0') {
        int bytesPerChar = bytesPerCharacterUTF8(bytes[i]);
        if (bytesPerChar == 0) {
            fprintf(stderr, "Error: Invalid UTF-8 byte sequence.\n");
            exit(EXIT_FAILURE);
        }
        unsigned int codepoint = decodeCharacterUTF8((unsigned char*)bytes + i, bytesPerChar);
        if (codepoint > 0x10FFFF) {
            fprintf(stderr, "Error: Invalid Unicode code point.\n");
            exit(EXIT_FAILURE);
        }
        strLength++;
        i += bytesPerChar;
    }

    char *str = (char*)malloc(strLength + 1);
    if (!str) {
        fprintf(stderr, "Error: Failed to allocate memory for the string.\n");
        exit(EXIT_FAILURE);
    }

    int offset = 0;
    i = 0;
    while (bytes[i] != '\0') {
        int bytesPerChar = bytesPerCharacterUTF8(bytes[i]);
        unsigned int codepoint = decodeCharacterUTF8((unsigned char*)bytes + i, bytesPerChar);
        if (codepoint <= 0xFFFF) {
            str[offset++] = (char)codepoint;
        }
        i += bytesPerChar;
    }
    str[offset] = '\0';

    return str;
}