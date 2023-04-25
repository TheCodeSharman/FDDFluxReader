#include <unity.h>
#include <string.h>
#include "utils/buffer.h"
#include <base64.hpp>
#include <cstdio>

uint32_t decodedSamples[] = {
        199, 201, 202, 198, 135, 134, 199, 200, 3121, 137, 258, 143, 260, 138, 130, 137, 260, 141, 
        260, 140, 130, 137, 261, 141, 262, 138, 130, 137, 260, 142, 260, 140, 130, 136, 259, 142, 
        262, 137, 131, 139, 258, 141
    };

const char *encodedSamples = "Y2VmYiMiY2TNFyWeASugASYeJaABKaABKB4loQEpogEmHiWgASqgASgeJJ8BKqIBJR8nngEp";

void testDecodeSampleBuffer() {
    uint32_t samples[42];

    int size = decodeSampleBuffer(strlen(encodedSamples), (const uint8_t *)encodedSamples, samples);
    
    TEST_ASSERT_EQUAL_INT32( 42, size );
    TEST_ASSERT_EQUAL_INT32_ARRAY(decodedSamples, samples, 42 );
}

void testEncodeSampleBuffer() {
    uint8_t outputBuffer[encode_base64_length(42*4)+1];

    int size = encodeSampleBuffer(42, outputBuffer, decodedSamples);

    TEST_ASSERT_EQUAL_INT32( strlen(encodedSamples), size );
    TEST_ASSERT_EQUAL_CHAR_ARRAY(encodedSamples, outputBuffer, strlen(encodedSamples) );
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(testDecodeSampleBuffer);
    RUN_TEST(testEncodeSampleBuffer);
    UNITY_END();
}