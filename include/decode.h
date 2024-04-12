#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <stdlib.h>

int bytesPerCharacterUTF8(unsigned char byte);
unsigned int decodeCharacterUTF8(unsigned char *bytes, int length);
char* decodeUTF8(const unsigned char *bytes);

#endif /* DECODE_H */