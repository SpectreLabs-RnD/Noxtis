
#include <sodium.h>

#include <stdint.h>

#include <stddef.h>

#include <stdlib.h>

#include <stdio.h>

#include <string.h>


#include "algorithm_local.h"


#define OUTSZ 1472

#define SALT 16

#define HDR 24

#define BODY (OUTSZ - HDR)

#define MAXIN (BODY - 2)


static uint8_t root[32];

static int root_ok;

static uint8_t tx_salt[16], tx_key[32], tx_npfx[4];

static uint64_t tx_seq, tx_mask;

static int tx_ok;

static uint8_t rx_salt[16], rx_key[32], rx_npfx[4];

static uint64_t rx_mask;

static int rx_ok;


static void put64(uint8_t *p, uint64_t v) {

	for(int i = 0; i < 8; i++) {

		p[i] = (uint8_t)(v >> (i * 8));

	}

}


static uint64_t get64(const uint8_t *p) {

	uint64_t v = 0;

	for(int i = 0; i < 8; i++) {

		v |= ((uint64_t)p[i]) << (i * 8);

		return v;

	}

}



static int load_key(void) {

	const char *k;

	size_t n = 0;

	if(root_ok) {

		return 0;

	}

	if(sodium_init() < 0) {

		return -1;

	}

	k = getenv("NOXTIS_KEY");

	if(!k || sodium_base642bin(root, 32, k, strlen(k), NULL, &n, NULL, sodium_base64_VARIANT_ORIGINAL) || n != 32) {

		fprintf(stderr, "Set NOXTIS_KEY with: openssl rand -base64 32\n");

		return -1;

	}


	root_ok = 1;

	return 0;

}


static void hash32(uint8_t out[32], const char *tag, const uint8_t salt[16]) {

	crypto_generichash_state st;

	crypto_generichash_init(&st, root, 32, 32);

	crypto_generichash_update(&st, (const uint8_t *)tag, strlen(tag));

	crypto_generichash_update(&st, salt, 16);

	crypto_generichash_final(&st, out, 32);

}

static void make_session(const uint8_t salt[16], uint8_t key[32], uint8_t npfx[4], uint64_t *mask) {

	uint8_t tmp[32];

	hash32(key, "key", salt);

	hash32(tmp, "nonce", salt);

	memcpy(npfx, tmp, 4);

	hash32(tmp, "mask", salt);

	*mask = get64(tmp);

}


static void make_nonce(uint8_t nonce[12], const uint8_t npfx[4], uint64_t seq) {

	memcpy(nonce, npfx, 4);

	put64(nonce + 4, seq);

}

static int tx_init(void) {

	if(tx_ok) {

		return 0;

	}

	if(load_key()) {

		return -1;

	}

	randombytes_buf(tx_salt, 16);

	make_session(tx_salt, tx_key, tx_npfx, &tx_mask);

	tx_seq = 0;

	tx_ok = 1;

	return 0;

}

void xor_data_fast(uint8_t *data, size_t len) {

	(void)data;

	(void)len;

}

int noxtis_pad_packet(uint8_t *p, size_t *len) {

	uint64_t seq, masked;

	uint8_t nonce[12];

	size_t l;

	if(!p || !len || tx_init()) {

		return -1;

	}

	l = *len;

	if(l == 0 || l > MAXIN) {

		return -1;

	}

	seq = tx_seq++;

	masked = seq ^ tx_mask;

	memmove(p + HDR + 2, p, l);

	memcpy(p, tx_salt, 16);

	put64(p + 16, masked);

	p[HDR] = (uint8_t)l;

	p[HDR + 1] = (uint8_t)(l >> 8);

	memset(p + HDR + 2 + l, 0, BODY - 2 - l);

	make_nonce(nonce, tx_npfx, seq);

	crypto_stream_chacha20_ietf_xor(p + HDR, p + HDR, BODY, nonce, tx_key);

	*len = OUTSZ;

	return 0;

}

int noxtis_unpad_packet(uint8_t *p, size_t *len) {

	uint64_t seq, masked;

	uint8_t nonce[12];

	size_t l;

	if(!p || !len || *len != OUTSZ || load_key()) {

		return -1;

	}

	if(!rx_ok || memcmp(rx_salt, p, 16) != 0) {

		memcpy(rx_salt, p, 16);

		make_session(rx_salt, rx_key, rx_npfx, &rx_mask);

		rx_ok = 1;

	}


	masked = get64(p + 16);

	seq = masked ^ rx_mask;

	make_nonce(nonce, rx_npfx, seq);

	crypto_stream_chacha20_ietf_xor(p + HDR, p + HDR, BODY, nonce, rx_key);

	l = (size_t)p[HDR] | ((size_t)p[HDR + 1] << 8);

	if(l == 0 || l > MAXIN) {

		return -1;

	}

	memmove(p, p + HDR + 2, l);

	*len = l;

	return 0;

}
