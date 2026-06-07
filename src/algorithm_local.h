
#ifndef ALGORITHM_H

#define ALGORITHM_H

#include <stdint.h>

#include <stddef.h>


void xor_data_fast(uint8_t *buf, size_t len);

int noxtis_pad_packet(uint8_t *packet, size_t *packet_len);

int noxtis_unpad_packet(uint8_t *packet, size_t *packet_len);

#endif
