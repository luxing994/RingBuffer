#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

int LX_InitRingBuffer(uint32_t size);
int LX_EnRingBuffer(uint8_t *data, uint32_t length);
int LX_DeRingBuffer(uint8_t *data, uint32_t size, uint16_t *length);

#endif