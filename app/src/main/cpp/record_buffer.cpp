#include <cstdint>
#include "record_buffer.h"

RecordBuffer::RecordBuffer(int buffersize) {
    buffer = new uint16_t* [2];
    for (int i = 0; i < 2; i++) {
        buffer[i] = new uint16_t[buffersize];
    }
}

RecordBuffer::~RecordBuffer() {

}

uint16_t *RecordBuffer::getRecordBuffer() {
    index++;
    if (index > 1) {
        index = 0;
    }
    return buffer[index];
}

uint16_t *RecordBuffer::getNowBuffer() {
    return buffer[index];
}