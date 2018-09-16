/**
 * Password checker, Operating System, NYU Shanghai, Prof. Olivier Marin.
 *
 * Checks if the conjectured password matches.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pw_checker.h"
#include "sha2_wrapper.h"



bool
check_pw (const char *conjectured_pw, size_t pw_length, const char *hash)
{
    hex_hash_result_t computed_hash;
    hex_hash_result_init (&computed_hash);
    sha256_signed_hex (conjectured_pw, pw_length, &computed_hash);
    return memcmp (computed_hash.hex_hash, hash, SHA256_HASH_HEX_LENGTH) == 0;
}


bool
check_pw_with_salt (const char *conjectured_pw, size_t pw_length,
    const char *salt, size_t salt_length,
    const char *hash)
{
    char *salted_password_buffer = malloc (1024);
    hex_hash_result_t computed_hash;
    hex_hash_result_init (&computed_hash);
    strncpy (salted_password_buffer, conjectured_pw, pw_length);
    strncpy (salted_password_buffer + pw_length, salt, salt_length);
    salted_password_buffer[pw_length + salt_length] = 0;
    sha256_signed_hex (salted_password_buffer, pw_length + salt_length, &computed_hash);
    free (salted_password_buffer);
    return memcmp (computed_hash.hex_hash, hash, SHA256_HASH_HEX_LENGTH) == 0;
}
