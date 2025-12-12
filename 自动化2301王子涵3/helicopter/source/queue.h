#ifndef QUEUE_H
#define QUEUE_H

#include "c8051f020.h"

typedef struct
{
    uchar base[129];
    uchar front, rear;
		uchar number;

} Queue;

void InitQueue(Queue *Q);
void EnQueue(Queue *Q, uchar rows);
void ClearQueue(Queue *Q);

#endif