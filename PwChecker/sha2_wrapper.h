/**
 * Wrapper for SHA2 hash functions.
 */

#pragma once

#include <unistd.h>


/**
 * SHA256 always produces hash of 256 bits long, aka. 32 bytes.
 */
#define SHA256_HASH_LENGTH 32


/**
 * SHA256 always produces hash of 256 bits long, aka. 64 hexadecimal characters.
 */
#define SHA256_HASH_HEX_LENGTH 64


/**
 * A hash result in hexadecimal format.
 */
typedef struct
{
    char hex_hash[SHA256_HASH_HEX_LENGTH + 1];
} hex_hash_result_t;


/**
 * Initialise a hex hash result.
 * @param result pointer to the result structure
 */
void
hex_hash_result_init (hex_hash_result_t *result);


/**
 * Calculate the SHA256 hash of a char array.
 * @param message message
 * @param len length of the message
 * @param dest destination where the raw result will be stored
 */
void
sha256_signed (const char *message, size_t len, char *dest);


/**
 * Calculate the SHA256 hash of a char array.
 * @param message message
 * @param len length of the message
 * @param dest destination where the hex string result will be stored
 */
void
sha256_signed_hex (const char *message, size_t len, hex_hash_result_t *dest);
