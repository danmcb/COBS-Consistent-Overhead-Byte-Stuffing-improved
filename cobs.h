/* Copyright 2022, Daniel McBrearty. All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted, with or without modification.
 * The correctness of this software is NOT guaranteed and the user uses it entirely at their own risk.
 *
 */
#ifndef COBS_H
#define COBS_H

#include <stdint.h>
#include <stddef.h>

// in principle it SHOULD be possible to define a terminator other than zero but this has not been tested.
#define COBS_TERMINATOR 0x00

// #define COBS_ENCODE_ADD_TERMINATOR

// ENCODE length bytes from the input, and return the number of bytes in the encoded output.
// if COBS_ENCODE_ADD_TERMINATOR is defined, the return value will include that otherwise it 
// is the responsibility of the caller to assign output[length] = 0. It is also the responsibility
// of the caller to ensure that a vuffer of sufficient size has been allocated.
size_t cobs_encode(const uint8_t * restrict input, size_t length, uint8_t * restrict output);

// DECODE length bytes of input. In this case it is expected that the receive code has already detected
// the trailing 0, and it need not be included in the input. Returns the number of bytes in the decoded 
// output, or 0 if the input is not valid, which can occur for one of two reasons:
//   1. a 0 has been detected in the input
//   2. a "marker byte" points past the end of the input buffer.
size_t cobs_decode(const uint8_t * restrict input, size_t length, uint8_t * restrict output);

#endif
