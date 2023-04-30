#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <cstdint>

/**
 * Encodes an array of samples into a compressed ASCII string in order to reduce the bandwidth
 * needed to send down the serial line. Most samples are between 2uS and 7uS, this algorihtm
 * aims to exploit this to, on average, send one byte per sample, but allowing longer pulses to
 * occassionally be present.
 * 
 * This implementation does the following:
 *          - for each sample:
 *           -  subtracts 100 from the sample 
 *           -  if the sample < 0x7f then store it in a single byte
 *           -  if the sample >= 0x7f then store rest of the word in multiple bytes
 *           -  the MSB bit (0x80) of a byte is set when more bytes to follow
 *          - base64 encode the resulting 
 * 
 * Decoding is the reverse of the above.
 */

class BufferEncoder {
    public:
        size_t maxBufferLength( const size_t count);
        int decodeSampleBuffer( const size_t inputBufferSize, const uint8_t *base64Buffer, uint32_t *samples );
        int encodeSampleBuffer( const size_t count, uint8_t *base64Buffer, const uint32_t *samples);

};

#endif