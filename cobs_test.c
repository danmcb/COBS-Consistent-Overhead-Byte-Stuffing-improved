/* Copyright 2011, Jacques Fortier. All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted, with or without modification.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cobs.h"

#define ASSERT_EQUAL_LUINT(value, expected) \
    do {\
        if( (value) != (expected) ) { \
            printf( "%30s: Failed, %s != %s. Expected %lu, got %lu\n", __func__, #value, #expected, (unsigned long)(expected), (unsigned long)(value) ); \
            return false; \
        } \
    } while(0)


#define ASSERT_LTE_LUINT(value, expected) \
    do {\
        if( !( (value) <= (expected) ) ) { \
            printf( "%30s: Failed, %s > %s. Expected <= %lu, got %lu\n", __func__, #value, #expected, (unsigned long)(expected), (unsigned long)(value) ); \
            return false; \
        } \
    } while(0)

#define ASSERT_EQUAL_UINT8_HEX(value, expected) \
    do {\
        if( (value) != (expected) ) { \
            printf( "%30s: Failed, %s != %s. Expected 0x%02X, got 0x%02X\n", __func__, #value, #expected, (expected), (value) ); \
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

#define ROUNDTRIP_TEST_RUNNER( input, length ) \
    do { \
        size_t working_buffer_length = (length) + (length) / 254 + 2; \
        uint8_t *working_buffer = malloc(working_buffer_length); \
        uint8_t *decoded_buffer = malloc(length + 1); \
        memset( working_buffer, MARKER_BYTE, working_buffer_length ); \
        decoded_buffer[length] = MARKER_BYTE; \
\
        size_t encoded_length = cobs_encode( input, length, working_buffer ); \
        ASSERT_LTE_LUINT(encoded_length, length + (length)/254 + 1); \
        ASSERT_EQUAL_UINT8_HEX( working_buffer[encoded_length], MARKER_BYTE ); \
        ASSERT_EQUAL_UINT8_HEX( working_buffer[working_buffer_length - 1], MARKER_BYTE ); \
\
        size_t decoded_length = cobs_decode( working_buffer, encoded_length, decoded_buffer ); \
\
/*      printf("Original:" ); \
        for( size_t i = 0; i < length; i++ ) printf(" %02X", (input)[i] ); \
        printf("\nEncoded:" ); \
        for( size_t i = 0; i < working_buffer_length; i++ ) printf(" %02X", working_buffer[i] ); \
        printf("\nDecoded:" ); \
        for( size_t i = 0; i < length; i++ ) printf(" %02X", decoded_buffer[i] ); \
        printf("\n" ); */\
        ASSERT_EQUAL_LUINT( decoded_length, length ); \
        ASSERT_EQUAL_MEM( decoded_buffer, input, length ); \
        ASSERT_EQUAL_UINT8_HEX( decoded_buffer[length], MARKER_BYTE ); \
\
        free(working_buffer); /* Not run if the tests fail, oh well */ \
        free(decoded_buffer); \
\
        return true; \
    } while(0)

#define MARKER_BYTE 0xAB
#define MAX_TEST_SIZE 260		

static unsigned int test_count = 0;
static uint8_t working_buffer[MAX_TEST_SIZE];

#define SETUP_TEST 	\
	do {	\
		test_count++;	\
		memset(working_buffer, MARKER_BYTE, sizeof(working_buffer));	\
	} while(0)


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

/*

bool test_255_bytes_null_end( void )
{
	SETUP_TEST;
    uint8_t buffer[255];
    for( unsigned int i = 0; i < sizeof(buffer) - 1; i++ )
    {
        buffer[i] = i + 1;
    }
    buffer[sizeof(buffer) - 1] = 0;

    uint8_t working_buffer[257];
    uint8_t expected_buffer[257];

    expected_buffer[0] = 0xFF;
    for( int i = 0; i < 254; i++ )
    {
        expected_buffer[i + 1] = i + 1;
    }
    expected_buffer[256] = 1;
    expected_buffer[257] = MARKER_BYTE;

    working_buffer[256] = MARKER_BYTE;

    size_t encoded_length = cobs_encode( buffer, sizeof(buffer), working_buffer );

    ASSERT_EQUAL_MEM( working_buffer, expected_buffer, sizeof(working_buffer) );
    ASSERT_EQUAL_LUINT( encoded_length, 256 );

    return true;
}

bool test_hex1_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[] = { 1 };

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_single_null_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[] = { 0 };

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_two_nulls_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[] = { 0, 0 };

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_null_one_null_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[] = { 0, 1, 0 };

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_null_two_null_one_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[] = { 0, 2, 0, 1 };

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_254_bytes_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[254];
    for( unsigned int i = 0; i < sizeof(buffer); i++ )
    {
        buffer[i] = i + 1;
    }

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_254_bytes_null_end_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[254];
    for( unsigned int i = 0; i < sizeof(buffer) - 1; i++ )
    {
        buffer[i] = i + 1;
    }
    buffer[sizeof(buffer) - 1] = 0;

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_255_bytes_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[255];
    for( unsigned int i = 0; i < sizeof(buffer); i++ )
    {
        buffer[i] = i % 254 + 1;
    }

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_255_bytes_null_end_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[255];
    for( unsigned int i = 0; i < sizeof(buffer) - 1; i++ )
    {
        buffer[i] = i + 1;
    }
    buffer[sizeof(buffer) - 1] = 0;

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_256_bytes_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[256];
    for( unsigned int i = 0; i < sizeof(buffer); i++ )
    {
        buffer[i] = i % 254 + 1;
    }

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

bool test_256_bytes_null_end_rt( void )
{
	SETUP_TEST;
    uint8_t buffer[256];
    for( unsigned int i = 0; i < sizeof(buffer) - 1; i++ )
    {
        buffer[i] = i + 1;
    }
    buffer[sizeof(buffer) - 1] = 0;

    ROUNDTRIP_TEST_RUNNER( buffer, sizeof(buffer) );
}

*/

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

/*
    test_hex1_rt();
    test_single_null_rt();
    test_two_nulls_rt();
    test_null_one_null_rt();
    test_null_two_null_one_rt();
    test_254_bytes_rt();
    test_254_bytes_null_end_rt();
    test_255_bytes_rt();
    test_255_bytes_null_end_rt();
    test_256_bytes_rt();
    test_256_bytes_null_end_rt();
	*/
	
	printf("ran %d COBS unit tests\n", test_count);

    return 0;
}
