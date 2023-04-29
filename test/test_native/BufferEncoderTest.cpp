#include <unity.h>
#include <string.h>
#include <BufferEncoder.h>
#include <cstdio>

BufferEncoder bufferEncoder;

// This array of samples is the worst case where for every 4 bytes there are
// 5 bytes in the output buffer.
uint32_t maximalEncode[] = { 1U<<31, 1U<<31, 1U<<31, 1U<<31  };

// Example typical array of samples
uint32_t decodedSamples[] = {
        199, 201, 202, 198, 135, 134, 199, 200, 3121, 137, 258, 143, 260, 138, 130, 137, 260, 141, 
        260, 140, 130, 137, 261, 141, 262, 138, 130, 137, 260, 142, 260, 140, 130, 136, 259, 142, 
        262, 137, 131, 139, 258, 141
    };

// Encoded output of the typical array
const char *encodedSamples = "Y2VmYiMiY2TNFyWeASugASYeJaABKaABKB4loQEpogEmHiWgASqgASgeJJ8BKqIBJR8nngEp";

void testMaxBufferLength() {
    size_t max = bufferEncoder.maxBufferLength(4);
    uint8_t outputBuffer[max];

    int size = bufferEncoder.encodeSampleBuffer(4, outputBuffer, maximalEncode);
    TEST_ASSERT_EQUAL_INT32(max, size + 1);
}

void testDecodeSampleBuffer() {
    uint32_t samples[42];

    int size = bufferEncoder.decodeSampleBuffer(strlen(encodedSamples), (const uint8_t *)encodedSamples, samples);
    
    TEST_ASSERT_EQUAL_INT32( 42, size );
    TEST_ASSERT_EQUAL_INT32_ARRAY(decodedSamples, samples, 42 );
}

void testEncodeSampleBuffer() {
    uint8_t outputBuffer[bufferEncoder.maxBufferLength((const size_t)42)];

    int size = bufferEncoder.encodeSampleBuffer(42, outputBuffer, decodedSamples);

    TEST_ASSERT_EQUAL_INT32( strlen(encodedSamples), size );
    TEST_ASSERT_EQUAL_CHAR_ARRAY(encodedSamples, outputBuffer, strlen(encodedSamples) );
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(testMaxBufferLength);
    RUN_TEST(testDecodeSampleBuffer);
    RUN_TEST(testEncodeSampleBuffer);
    UNITY_END();
}