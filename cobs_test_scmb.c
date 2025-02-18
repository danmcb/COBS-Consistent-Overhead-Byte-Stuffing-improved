/* Copyright 2011, Jacques Fortier. All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted, with or without modification.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cobs_scmb.h"

#define MARKER_BYTE 0
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

#define FWD_TEST	\
	stuff_data( test_data, sizeof(test_data), working_buffer ); \
	ASSERT_EQUAL_MEM( "FWD", working_buffer, expected, sizeof(expected) ); \
	for (uint16_t i = sizeof(expected); i < MAX_TEST_SIZE; i++) \
	{	\
		if (working_buffer[i] != MARKER_BYTE)	\
		{	\
			printf("Failed: encoding overwrote buffer at pos %d in %s (%d)\n", i, __func__, working_buffer[i]); \
			return false; \
		}	\
	}


#define REV_TEST	\
	memset(working_buffer, MARKER_BYTE, sizeof(working_buffer));	\
	unstuff_data( expected, sizeof(expected), working_buffer ); \
	ASSERT_EQUAL_MEM( "REV", working_buffer, test_data, sizeof(test_data) );	\
	for (uint16_t i = sizeof(test_data); i < MAX_TEST_SIZE; i++) \
	{	\
		if (working_buffer[i] != MARKER_BYTE)	\
		{	\
			printf("Failed: decoding overwrote buffer at pos %d in %s (%d)\n", i, __func__, working_buffer[i]); \
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
	
	printf("ran %d COBS unit tests\n", test_count);

    return 0;
}
