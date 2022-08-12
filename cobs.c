#include "cobs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// this macro replaces a NULL (or the header) with the ctr to next code_ptr
#define FINISH_BLOCK(X, flag) (  block_flag = flag,     \
                                 *code_ptr = (X),       \
                                 code_ptr = output++,   \
                                 code = 0x01       )

size_t cobs_encode(const uint8_t * restrict input, size_t length, uint8_t * restrict output)
{
    const uint8_t * end = input + length;         // remember the end point
    uint8_t * start_of_output = output;           // remember where we started
    uint8_t * code_ptr = output++;                // initially, the header byte, thereafter the next NULL to replace
    uint8_t code = 0x01;                          // count number of nun-NULL bytes
    bool block_flag = false;                      // prevent doing FINISH_BLOCK twice for cerrtain edge cases
                                                  // unit tests from wikipedia ex. 6 and 7 fail without this

    while(input < end)                            // loop over input
    {
        block_flag = false;
        if (*input == 0) FINISH_BLOCK(code, false);         // found a NULL? do what is needed.
        else
        {
            *output++ = *input;                             // just copy input to output and inc te counter
            code++;
            if (code == 0xFF) FINISH_BLOCK(code, true);     // if we hit this at the end, we do NOT want to do it again below!
        }
        input++;
    }
    if (block_flag == false) FINISH_BLOCK(code, false);
	
    //*code_ptr = 0;                                          // append the NULL byte
    //return code_ptr - start_of_output + 1;                  // return position of the NULL
	
    return code_ptr - start_of_output;                        // return position not including
}

size_t cobs_decode(const uint8_t * restrict input, size_t length, uint8_t * restrict output)
{
    size_t read_index = 0;
    size_t write_index = 0;
    uint8_t code;
    uint8_t i;

    while(read_index < length)
    {
        code = input[read_index];
        if(read_index + code > length && code != 1) // overrun
        {
            return 0;
        }
        if (code == 0)  // we can't be having NULL here, error
        {
            return 0;
        }
        read_index++;
        for(i = 1; i < code; i++)
        {
            uint8_t data_byte = input[read_index++];
            if (data_byte == 0)
            {
                return 0; // we can't be having NULL here, either
            }
            else
            {
                output[write_index++] = data_byte;
            }
        }
        if(code != 0xFF && read_index != length)
        {
            output[write_index++] = '\0';
        }
    }

    return write_index;
}
