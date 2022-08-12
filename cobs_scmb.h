#ifndef COBS_SCMB_H
#define COBS_SCMB_H

#include <stdint.h>

void stuff_data(const uint8_t * restrict ptr, size_t length, uint8_t * restrict dst);
void unstuff_data(const uint8_t * restrict ptr, size_t length, uint8_t * restrict dst);

#endif
