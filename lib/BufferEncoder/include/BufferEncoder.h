#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <cstdint>

class BufferEncoder {

    public:
    size_t maxBufferLength( const size_t count);
    int decodeSampleBuffer( const size_t inputBufferSize, const uint8_t *base64Buffer, uint32_t *samples );
    int encodeSampleBuffer( const size_t count, uint8_t *base64Buffer, const uint32_t *samples);

};

#endif