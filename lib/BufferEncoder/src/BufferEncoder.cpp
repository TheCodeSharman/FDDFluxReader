#include "BufferEncoder.h"
#include <base64.hpp>

size_t BufferEncoder::maxBufferLength( const size_t count) {
    return (size_t) encode_base64_length(count*5)+1;
}

int BufferEncoder::decodeSampleBuffer( const size_t inputBufferSize, const uint8_t *base64Buffer, uint32_t *samples ) {
    uint8_t outBuffer[decode_base64_length(base64Buffer, inputBufferSize)+1];

    int decodedSize = decode_base64(base64Buffer, inputBufferSize, outBuffer);

    int i = 0;
    for( int p = 0; p < decodedSize;) {
        uint32_t sample25ns = 100;
        uint8_t byte = 0x80;
        int shift = 0;
        while(byte & 0x80) {
            byte = outBuffer[p++];
            sample25ns += (byte & 0x7f) << shift;
            shift += 7;
        }

        samples[i++] = sample25ns;
    }

    return i;
}

int BufferEncoder::encodeSampleBuffer( const size_t count, uint8_t *base64Buffer, const uint32_t *samples)
{
    // There is a maximum of 5 bytes per word
    uint8_t outBuffer[count*5];

    // Multi byte decoding:
    //   0. set c = 0
    //   1. read byte b, c = c + (b & 7F)
    //   2. if b&80 == 1, goto 1
    int p = 0;
    for( int i = 0; i < count; i++) {
        uint32_t sample25ns = samples[i];

        // Discard pulses less than 2.5us - we don't need them
        if ( sample25ns > 100 ) {
            sample25ns = sample25ns - 100; // gives lesss than a byte per sample most of the time.

            // While the sample has more bits keep ading bytes to the output buffer.
            // These bytes have the most sigificnat bit set to indicate more bytes to 
            // follow.
            while(sample25ns > 0) {
                uint8_t byte = sample25ns & 0x7F;
                sample25ns = sample25ns >> 7;

                // The last byte has most significant bit clear to indicate no more bytes 
                // to follow.
                outBuffer[p++] = (sample25ns > 0 ? 0x80 : 0) | byte; 
            }
        }
    }

    // Base64 encoding the buffer adds significant overhead but it means
    // that the UART protocol is ASCII which displays a little easier in
    // serial monitors. 
    //
    // Once we stop using using terminals to send commands for testing
    // this makes no sense - so will probably remove it.
    return encode_base64(outBuffer, p, base64Buffer);
}