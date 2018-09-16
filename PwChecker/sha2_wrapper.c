/**
 * Password checker, Operating System, NYU Shanghai, Prof. Olivier Marin.
 *
 * Wrapper for SHA2 hash functions.
 */

#include <stdio.h>
#include <memory.h>
#include "sha2_wrapper.h"
#include "sha2.h"


void
hex_hash_result_init (hex_hash_result_t *result)
{
    // defensive initialisation with every byte emptied
    memset (result->hex_hash, 0, SHA256_HASH_HEX_LENGTH);
    result->hex_hash[SHA256_HASH_HEX_LENGTH] = 0;
}


void
sha256_signed (const char *message, size_t len, char *dest)
{
    sha256 ((const unsigned char *) message, (unsigned int) len, (unsigned char *) dest);
}


void
sha256_signed_hex (const char *message, size_t len, hex_hash_result_t *dest)
{
    unsigned char raw_digest[SHA256_HASH_LENGTH];
    int i;
    sha256_signed (message, len, (char *) raw_digest);
    char *dest_head = dest->hex_hash;
    for (i = 0; i < SHA256_HASH_LENGTH; i++)
        sprintf (dest_head + i * 2, "%02x", raw_digest[i]);
}
