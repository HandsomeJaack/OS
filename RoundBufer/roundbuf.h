#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define BUFFER_SIZE 16

typedef struct {
    size_t first;
    size_t last;
    uint8_t data[BUFFER_SIZE];
} RoundBuffer;

void ClearBuf(RoundBuffer* pBuf){
    pBuf->first = 0;
    pBuf->last = 0;
}

// ReadByte читает байт из буфера.  если в буфере нет данных, возвращает -1.
int ReadByte(RoundBuffer* pBuf){
    if (pBuf->first == pBuf->last)
        return -1;
    int result = pBuf->data[pBuf->first];
    pBuf->first = (pBuf->first + 1) & (BUFFER_SIZE - 1);
    return result;
}

// пишет байт в буфер, возвращает true если запись прошла успешно
bool WriteByte(RoundBuffer* pBuf, uint8_t value){
    size_t next = (pBuf->last + 1) & (BUFFER_SIZE - 1);
    if (next == pBuf->first)
        return false;
    pBuf->data[pBuf->last] = value;
    pBuf->last = next;
    return true;
}


void PrintBuffer(RoundBuffer* pBuf)
{
    if (pBuf->first == pBuf->last)
        printf(" Empty");
    for (size_t pos = pBuf->first; pos != pBuf->last; pos = (pos + 1) & (BUFFER_SIZE - 1))
        printf(" %02x", pBuf->data[pos]);
    printf("\n");
}

