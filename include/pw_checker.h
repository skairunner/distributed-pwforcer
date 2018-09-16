/**
 * Password checker, Operating System, NYU Shanghai, Prof. Olivier Marin.
 *
 * Checks if the conjectured password matches.
 */

#pragma once

#include <stdio.h>
#include <stdbool.h>


/**
 * Check if the conjectured password matches the given hash.
 * @param conjectured_pw conjectured password
 * @param pw_length length of the conjectured password
 * @param hash known hash
 * @param hash_length length of the hash
 * @return whether they match
 */
bool
check_pw (const char *conjectured_pw, size_t pw_length, const char *hash);


/**
 * Check if the conjectured password and salt matches the given hash.
 * @param conjectured_pw conjectured password
 * @param pw_length length of the conjectured password
 * @param salt salt
 * @param salt_length length of the salt
 * @param hash known hash
 * @param hash_length length of the hash
 * @return whether they match
 */
bool
check_pw_with_salt (const char *conjectured_pw, size_t pw_length,
    const char *salt, size_t salt_length,
    const char *hash);
