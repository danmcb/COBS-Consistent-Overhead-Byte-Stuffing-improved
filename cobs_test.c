/* Copyright 2011, Jacques Fortier. All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted, with or without modification.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cobs.h"

#define MARKER_BYTE 0xAB
#define MAX_TEST_SIZE 260	

#define ASSERT_EQUAL_LUINT(value, expected) \
    do {\
        if( (value) != (expected) ) { \
            printf( "%30s: Failed, %s != %s. Expected %lu, got %lu\n", __func__, #value, #expected, (unsigned long)(expected), (unsigned long)(value) ); \
            return false; \
        } \
    } while(0)

#define ASSERT_EQUAL_MEM(path, value, expected, length) \
    do {\
        for( size_t i = 0; i < (length); i++ ) { \
            if( (value)[i] != (expected)[i] ) { \
                printf( "%30s: Failed, %s != %s. Expected %s[%lu] = 0x%02X, got 0x%02X (%s)\n", __func__, #value, #expected, #value, i, (expected)[i], (value)[i], path ); \
                return false; \
            } \
        } \
   } while(0)	

#define SETUP_TEST 	\
	do {	\
		test_count++;	\
		memset(working_buffer, MARKER_BYTE, sizeof(working_buffer));	\
	} while(0)

#ifdef COBS_ENCODE_ADD_TERMINATOR
#define FWD_TEST	\
	size_t encoded_length = cobs_encode( test_data, sizeof(test_data), working_buffer ); \
	ASSERT_EQUAL_LUINT( encoded_length, sizeof(expected) + 1); \
	ASSERT_EQUAL_MEM( "FWD", working_buffer, expected, sizeof(expected) ); \
	ASSERT_EQUAL_LUINT( working_buffer[encoded_length - 1], 0 );	\
	for (uint16_t i = sizeof(expected) + 1; i < MAX_TEST_SIZE; i++) \
	{	\
		if (working_buffer[i] != MARKER_BYTE)	\
		{	\
			printf("Failed: encoding overwrote buffer at pos %d in %s\n", i, __func__); \
			return false; \
		}	\
	}
#else
#define FWD_TEST	\
	size_t encoded_length = cobs_encode( test_data, sizeof(test_data), working_buffer ); \
	ASSERT_EQUAL_LUINT( encoded_length, sizeof(expected) ); \
	ASSERT_EQUAL_MEM( "FWD", working_buffer, expected, sizeof(expected) ); \
	for (uint16_t i = sizeof(expected); i < MAX_TEST_SIZE; i++) \
	{	\
		if (working_buffer[i] != MARKER_BYTE)	\
		{	\
			printf("Failed: encoding overwrote buffer at pos %d in %s\n", i, __func__); \
			return false; \
		}	\
	}
#endif

#define REV_TEST	\
	memset(working_buffer, MARKER_BYTE, sizeof(working_buffer));	\
	size_t decoded_length = cobs_decode( expected, sizeof(expected), working_buffer ); \
	ASSERT_EQUAL_LUINT( decoded_length, sizeof(test_data)); \
	ASSERT_EQUAL_MEM( "REV", working_buffer, test_data, sizeof(test_data) );	\
	for (uint16_t i = sizeof(test_data); i < MAX_TEST_SIZE; i++) \
	{	\
		if (working_buffer[i] != MARKER_BYTE)	\
		{	\
			printf("Failed: decoding overwrote buffer at pos %d in %s\n", i, __func__); \
			return false; \
		}	\
	}
	
static unsigned int test_count = 0;
static uint8_t working_buffer[MAX_TEST_SIZE];

// Wikipedia Example 1
bool test_single_null( void )
{	
	SETUP_TEST;
    uint8_t test_data[] = { 0 };
    uint8_t expected[] = { 1, 1 };
	FWD_TEST;
	REV_TEST;
	return true;
}

// Wikipedia Example 2
bool test_double_null( void )
{	
	SETUP_TEST;
    uint8_t test_data[] = { 0, 0 };
    uint8_t expected[] = { 1, 1, 1 };
	FWD_TEST;
	REV_TEST;
	return true;
}

// not in wikipedia
bool test_hex1( void )
{
	SETUP_TEST;
    uint8_t test_data[] = { 1 };
    uint8_t expected[] = { 2, 1 };
	FWD_TEST;
	REV_TEST;
    return true;
}

bool test_cobs_encode_three_bytes(void) // Wikipedia Example 3
{
	SETUP_TEST;
	uint8_t test_data[] = { 0x00, 0x11, 0x00 };
	uint8_t expected[] = { 0x01, 0x02, 0x11, 0x01 };
	FWD_TEST;
	REV_TEST;
	return true;
}

bool test_cobs_encode_short_with_null(void) // Wikipedia Example 4
{
	SETUP_TEST;
	uint8_t test_data[] = { 0x11, 0x22, 0x00, 0x33 };
	uint8_t expected[] = { 0x03, 0x11, 0x22, 0x02, 0x33 };
	FWD_TEST;
	REV_TEST;
	return true;
}

bool test_cobs_encode_short_no_null(void) // Wikipedia Example 5
{
	SETUP_TEST;
	uint8_t test_data[] = { 0x11, 0x22, 0x33, 0x44 };
	uint8_t expected[] = { 0x05, 0x11, 0x22, 0x33, 0x44 };
	FWD_TEST;
	REV_TEST;
	return true;
}

bool test_cobs_encode_short_successive_nulls(void) // Wikipedia Example 6
{
	SETUP_TEST;
	uint8_t test_data[] = { 0x11, 0x00, 0x00, 0x00 };
	uint8_t expected[] = { 0x02, 0x11, 0x01, 0x01, 0x01 };
	FWD_TEST;
	REV_TEST;
	return true;
}

bool test_cobs_encode_254_bytes_no_null(void) // Wikipedia Example 7
{
	SETUP_TEST;
    uint8_t test_data[254];
	for( uint8_t i = 0; i < 254; i++ )
    {
        test_data[i] = i + 1;  // 1 to 0xFE (254)
    }
	
	uint8_t expected[255];
    expected[0] = 0xFF;
    for( int i = 1; i <= 254; i++ )
    {
        expected[i] = i;
    }
    
	FWD_TEST;
	REV_TEST;
	return true;
	
}

bool test_cobs_encode_255_bytes_leading_null(void) // Wikipedia Example 8
{
	SETUP_TEST;
	uint8_t test_data[255];
    for( uint8_t i = 0; i < 255; i++ )
    {
        test_data[i] = i;  // 0x00 to 0xFE (254)
    }
	
    uint8_t expected[256]; 
    expected[0] = 0x01;
	expected[1] = 0xFF;
    for( uint8_t i = 1; i <= 254; i++ )
    {
        expected[i + 1] = i; // write 1 to 254 from index 2 to 255
    }

	FWD_TEST;
	REV_TEST;
	return true;
}

bool test_cobs_encode_255_bytes_no_null(void) // Wikipedia Example 9
{
	SETUP_TEST;
	uint8_t test_data[255];
    for( uint8_t i = 0; i < 255; i++ )
    {
        test_data[i] = i + 1;  // 0x01 to 0xFF(255)
    }
	
    uint8_t expected[257]; 
    expected[0] = 0xFF;
    for( int i = 1; i <= 254; i++ )
    {
        expected[i] = i; // write 0x01 to 0xFE from index 1 to 254
    }
	expected[255] = 0x02;
	expected[256] = 0xFF;

	FWD_TEST;
	REV_TEST;
	return true;
}

bool test_cobs_encode_254_bytes_trailing_null(void) // Wikipedia Example 10
{
	SETUP_TEST;
	uint8_t test_data[255];
    for( uint8_t i = 0; i <= 253; i++ )
    {
        test_data[i] = i + 2;  // 0x02 to 0xFF to indexes 0 to 253
    }
	test_data[254] = 0x00; // trailing null
	
    uint8_t expected[257];
    expected[0] = 0xFF;
    for( int i = 1; i <= 254; i++ )
    {
        expected[i] = i + 1; // write 0x02 to 0xFF from index 1 to 254
    }
	expected[255] = 0x01;
	expected[256] = 0x01;

	FWD_TEST;
	REV_TEST;
	return true;
}

bool test_cobs_encode_254_bytes_trailing_null_one(void) // Wikipedia Example 11
{
	SETUP_TEST;
	uint8_t test_data[255];
    for( uint8_t i = 0; i <= 252; i++ )
    {
        test_data[i] = i + 3;  // 0x03 to 0xFF to indexes 0 to 252
    }
	test_data[253] = 0x00;
	test_data[254] = 0x01;
	
    uint8_t expected[256];
    expected[0] = 0xFE;
    for( int i = 1; i <= 253; i++ )
    {
        expected[i] = i + 2; // write 0x03 to 0xFF from index 1 to 254
    }
	expected[254] = 0x02;
	expected[255] = 0x01;

	FWD_TEST;
	REV_TEST;
	return true;
}

// We're done testing the correctness of encode/decode. WHat remains now is to check that the decoder 
// returns zero when passed an invalid COBS packet - i.e. when the header (and/or the bytes it links
// to) extend beyond the length of the input.

bool test_utils_cobs_decode_header_too_large_1(void)
{
	SETUP_TEST;
	 // header byte 0x2d = 45 => overrun
	uint8_t input[] = 		{0x2d,0x51,0x32,0x30,0x32,0x31,0x30,0x31, //8
					   		 0x38,0x20,0x31,0x38,0x3a,0x32,0x39,0x3a, //16
					   		 0x32,0x39,0x20,0x2e,0x20,0x53,0x74,0x61, //24
					   		 0x72,0x74,0x20,0x6d,0x65,0x61,0x73,0x75, //32
					   		 0x72,0x65,0x6d,0x65,0x6e,0x74,0x20,0x5b, //40
					   		 0x39,0x5d,0x0a,0x12};                    //44 bytes

	uint8_t output[sizeof(input)];
	uint16_t res = cobs_decode(input, sizeof(input), output);
	ASSERT_EQUAL_LUINT(0, res); // signals that decoding failed
	return true;
}

bool test_utils_cobs_decode_header_too_large_2(void)
{
	SETUP_TEST;
	 // header byte 0x0f (15), byte 15 = 0xff (255) => overrun
	uint8_t input[] = 		{0x0f,0x51,0x32,0x30,0x32,0x31,0x30,0x31, //8
					   		 0x38,0x20,0x31,0x38,0x3a,0x32,0x39,0xff, //16
					   		 0x32,0x39,0x20,0x2e,0x20,0x53,0x74,0x61, //24
					   		 0x72,0x74,0x20,0x6d,0x65,0x61,0x73,0x75, //32
					   		 0x72,0x65,0x6d,0x65,0x6e,0x74,0x20,0x5b, //40
					   		 0x39,0x5d,0x0a,0x12};                    //44 bytes

	uint16_t res = cobs_decode(input, sizeof(input), working_buffer);
	ASSERT_EQUAL_LUINT(0, res); // signals that decoding failed
	return true;
}

bool test_utils_cobs_fail_on_null(void)
{
	SETUP_TEST;
	// decoding should fail if a NULL byte is encountered anywhere in the encoded COBS
	// we start with a small frame that decodes OK.
	uint8_t good_input[] = { 0x03, 0x11, 0x22, 0x03, 0x33, 0x44};
	uint16_t res = cobs_decode(good_input, sizeof(good_input), working_buffer);
	ASSERT_EQUAL_LUINT(5, res); // decoded OK
	// now we check that setting any byte to 0x00 fails
	uint8_t bad_input[sizeof(good_input)];
	for (uint8_t i = 0; i < sizeof(good_input); i++)
	{
		memcpy(bad_input, good_input, sizeof(good_input));
		bad_input[i] = 0;
		res = cobs_decode(bad_input,sizeof(good_input),working_buffer);
		ASSERT_EQUAL_LUINT(0, res); // fail
	}
	return true;
}

int main(int argc, char*argv[])
{
    test_single_null();
	test_double_null();
    test_hex1();
	test_cobs_encode_three_bytes();
	test_cobs_encode_short_with_null();
	test_cobs_encode_short_no_null();
	test_cobs_encode_short_successive_nulls();
	test_cobs_encode_254_bytes_no_null();
	test_cobs_encode_255_bytes_leading_null();
	test_cobs_encode_255_bytes_no_null();
	test_cobs_encode_254_bytes_trailing_null();
	test_cobs_encode_254_bytes_trailing_null_one();
	
	test_utils_cobs_decode_header_too_large_1();
	test_utils_cobs_decode_header_too_large_2();
	test_utils_cobs_fail_on_null();
	
	
	printf("ran %d COBS unit tests\n", test_count);

    return 0;
}
