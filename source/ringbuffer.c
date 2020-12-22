#include <stdio.h>
#include <stdlib.h>
#include "ringbuffer.h"

#define HEADSIZE 2
#define LENGTHSIZE 2
#define FIRSTHEAD 0xA5
#define SECONDHEAD 0xBF
#define RINGBUFFERMAXSIZE 4096

typedef struct {
	int front;
	int rear;
	int size;
	uint8_t buffer[];
} BaseQueue;

typedef struct {
	uint8_t head[HEADSIZE];
	uint16_t length;
	uint8_t *num;
} StoreFormat;

BaseQueue* baseQueue = NULL;

static BaseQueue *LX_InitBaseQueue(uint32_t size)
{
	int ret;

	BaseQueue *queue = (BaseQueue *)malloc(sizeof(BaseQueue) + sizeof(uint8_t) * size);
	if (queue == NULL) {
		return NULL;
	}

	queue->front = queue->rear = 0;
	queue->size = size;
	return queue;
}

static int LX_EnQueue(BaseQueue *queue, uint8_t data)
{
	if ((queue == NULL) || (queue->size == 0)) {
		return -1;
	}

	if ((queue->rear + 1) % (queue->size) == (queue->front)) {
		return -1;
	}

	queue->buffer[queue->rear] = data;
	queue->rear = (queue->rear + 1) % (queue->size);
	return 0;
}

static int LX_DeQueue(BaseQueue *queue, uint8_t *data)
{
	if ((queue == NULL) || (queue->size == 0)) {
		return -1;
	}

	if ((queue->front) == (queue->rear)) {
		return -1;
	}

	*data = queue->buffer[queue->front];
	queue->front = (queue->front + 1) % (queue->size);
	return 0;
}

int LX_InitRingBuffer(uint32_t size)
{
	if (size > RINGBUFFERMAXSIZE) {
		return -1;
	}

	baseQueue = LX_InitBaseQueue(size);
	if (baseQueue == NULL) {
		return -1;
	}

	return 0;
}

int LX_EnRingBuffer(uint8_t *data, uint32_t length)
{
	StoreFormat newData;
	uint32_t newLength;
	uint8_t enData[RINGBUFFERMAXSIZE] = {0};
	int i, ret;

	if ((data == NULL) || (length == 0) || (baseQueue == NULL)) {
		return -1;
	}

	newLength = HEADSIZE + LENGTHSIZE + length;
	if (newLength > (baseQueue->size)) {
		return -1;
	}

	newData.head[0] = FIRSTHEAD;
	newData.head[1] = SECONDHEAD;
	newData.length = length;
	newData.num = data;
	memcpy(enData, &newData, HEADSIZE + LENGTHSIZE);
	memcpy(&enData[4], newData.num, newLength);

	for (i = 0; i < newLength; i++) {
		ret = LX_EnQueue(baseQueue, enData[i]);
		if (ret != 0) {
			// queue full or other
			break;
		}
	}

	return 0;
}

int LX_DeRingBuffer(uint8_t *data, uint32_t size, uint16_t *length)
{
	uint8_t curData = 0;
	uint8_t lastData = 0;
	uint8_t outData[RINGBUFFERMAXSIZE];
	int ret, i;

	if ((data == NULL) || (*length == NULL)) {
		return -1;
	}

	while (!((curData == SECONDHEAD) && (lastData == FIRSTHEAD))) {
		lastData = curData;
		ret = LX_DeQueue(baseQueue, &curData);
		if (ret != 0) {
			return -1;
		}
	}

	for (i = 0; i < LENGTHSIZE; i++) {
		ret = LX_DeQueue(baseQueue, &outData[i]);
		if (ret != 0) {
			return -1;
		}
	}

	*length = (outData[1] << 8) | outData[0];
	for (i = 0; i < (*length); i++) {
		ret = LX_DeQueue(baseQueue, &outData[i]);
		if (ret != 0) {
			return -1;
		}
	}
	memcpy(data, outData, *length);

	return 0;
}