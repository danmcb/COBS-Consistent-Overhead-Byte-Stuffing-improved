#include "cobs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FinishBlock(X) (*code_ptr = (X), \
						code_ptr = dst++, \
						code = 0x01 )

void stuff_data(const uint8_t * restrict ptr, size_t length, uint8_t * restrict dst)
{
	const uint8_t *end = ptr + length;
	uint8_t *code_ptr = dst++;
	uint8_t code = 0x01;
	while (ptr < end)
	{
		if (*ptr == 0) FinishBlock(code);
		else
		{
			*dst++ = *ptr;
			code++;
			if (code == 0xFF) FinishBlock(code);
		}
			ptr++;
		}
		FinishBlock(code);
}

void unstuff_data(const uint8_t * restrict ptr, size_t length, uint8_t * restrict dst)
{
	const uint8_t *end = ptr + length;
	while (ptr < end)
	{
		int i, code = *ptr++;
		for (i=1; i<code; i++) *dst++ = *ptr++;
		if (code < 0xFF) *dst++ = 0;
	}
}
