
#include <sodium.h>

#include <stdint.h>

#include <stddef.h>

#include <stdio.h>

#include "algorithm_remote.h"


#define PMTU_MAX 1472

static const uint8_t STATIC_KEY[32] = {

    0xA1, 0x24, 0x93, 0x55, 0x67, 0xAC, 0xDE, 0xF1,
    0x10, 0x8A, 0xC0, 0x72, 0x66, 0x33, 0xBA, 0x99,
    0xDE, 0x77, 0x43, 0x22, 0xFE, 0x18, 0x90, 0xAB,
    0x44, 0x2E, 0x78, 0x50, 0xCA, 0x99, 0x01, 0xCC

};

static const uint8_t STATIC_NONCE[12] = {

    0x01, 0x00, 0x00, 0x00,
    0x55, 0xAA, 0x55, 0xAA,
    0x10, 0x20, 0x30, 0x40

};

void xor_data_fast(uint8_t *data, size_t len) {

    crypto_stream_chacha20_xor_ic(data, data, len, STATIC_NONCE, 0, STATIC_KEY);

}

int noxtis_pad_packet(uint8_t *packet, size_t *packet_len) {

	size_t original_len = *packet_len;

	if(!original_len || original_len > 65535 || original_len + 2 > PMTU_MAX) {

		return -1;

	}

	size_t available_padding = PMTU_MAX - original_len - 2;

	size_t chosen_padding;

	if(original_len < 1000) {

		size_t high_padding_start = available_padding / 2;

		chosen_padding = high_padding_start + randombytes_uniform((uint32_t)(available_padding - high_padding_start + 1));

	}

	else {

		chosen_padding = randombytes_uniform((uint32_t)(available_padding + 1));

	}

	size_t padded_len = original_len + chosen_padding + 2;

	if(chosen_padding) {

		randombytes_buf(packet + original_len, chosen_padding);

		packet[padded_len - 2] = (uint8_t)original_len;

		packet[padded_len - 1] = (uint8_t)(original_len >> 8);

		xor_data_fast(packet, padded_len);

		*packet_len = padded_len;

		return 0;

	}

	return 0;
}

int noxtis_unpad_packet(uint8_t *packet, size_t *packet_len) {

	size_t received_len = *packet_len;

	if(received_len < 2) {

        	return -1;

	}

	xor_data_fast(packet, received_len);

	size_t original_len = (size_t)packet[received_len - 2] | ((size_t)packet[received_len - 1] << 8);

	if(!original_len || original_len > received_len - 2) {

		return -1;

	}

	*packet_len = original_len;

	return 0;

}
