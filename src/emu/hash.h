/*********************************************************************

    hash.h

    Function to handle hash functions (checksums)

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __HASH_H__
#define __HASH_H__

#define HASH_INFO_NO_DUMP	0
#define HASH_INFO_BAD_DUMP	1

#define HASH_CRC    (1 << 0)
#define HASH_SHA1   (1 << 1)
#define HASH_MD5    (1 << 2)

#define HASH_NUM_FUNCTIONS  3

// Standard size of a hash data buffer, all the manipulated buffers
//  must respect this size
#define HASH_BUF_SIZE       256

// Get function name of the specified function
const char* hash_function_name(unsigned int function);

// Check if const char* contains the checksum for a specific function
int hash_data_has_checksum(const char* d, unsigned int function);

// Extract the binary or printable checksum of a specific function from a hash data. If the checksum information
//  is not available, the functions return 0. If the pointer to the output buffer is NULL, the function will
//  return the minimum size of the output buffer required to store the informations. Otherwise, the buffer
//  will be filled and the function will return 1 as success code.
int hash_data_extract_binary_checksum(const char* d, unsigned int function, unsigned char* checksum);
int hash_data_extract_printable_checksum(const char* d, unsigned int function, char* checksum);

// Insert an already computed binary checksum inside a hash data. This is useful when we already have
//  checksum informations (e.g, from archive headers) and we want to prepare a hash data to compare
//  with another const char* (e.g. the expected checksums). Returns 0 in case of error, 1 if the checksum
//  was added correctly, 2 if the checksum was added overwriting a previously existing checksum for the
//  the same function
int hash_data_insert_binary_checksum(char* d, unsigned int function, const unsigned char* checksum);
int hash_data_insert_printable_checksum(char* d, unsigned int function, const char* checksum);

// Check if the hash data contains the requested info
int hash_data_has_info(const char* d, unsigned int info);

// Compare two hash data to check if they are the same. 'functions' can be either a combination of the
//  hash function bits (HASH_CRC, etc) or zero to ask to check for all the available checksums
int hash_data_is_equal(const char* d1, const char* d2, unsigned int functions);

// Print hash data informations in a standard format. 'functions' can be either a combination of the
//  hash function bits (HASH_CRC, etc) or zero to ask to print all the available checksums
void hash_data_print(const char* d, unsigned int functions, char* buffer);

// Copy hash data informations
void hash_data_copy(char* dst, const char* src);

// Clear hash data informations
void hash_data_clear(char* dst);

// Check which functions we have a checksum of inside the data
unsigned int hash_data_used_functions(const char* d);

// Compute hash of a data chunk in memory. Parameter 'functions' specifies which hashing functions
//  we want the checksum of.
void hash_compute(char* dst, const unsigned char* data, unsigned long length, unsigned int functions);

// Verifies that a hash string is valid
int hash_verify_string(const char *hash);


#endif	/* __HASH_H__ */
