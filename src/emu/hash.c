/*********************************************************************

    hash.c

    Function to handle hash functions (checksums)

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    Started by Farfetch'd

*********************************************************************/

/*
 * Changelog:
 *
 * 20030314:  Farfetch'd
 *    First release
 *
 */

/*
 * DONE:
 *
 * hash.c/h: New files, implement the new hashing engine with flexible
 *    support for more functions (for now, CRC, SHA1 and MD5).
 *
 * common.h: transparently support the new RomModule structure through
 *    ROM_* macros, so that old the legacy code still work
 *
 * common.c: updated ROM loading engine to support the new hash engine,
 *    using it to verify ROM integrity at load-time. Updated printromlist()
 *    (-listroms) to dump all the available checksums, and if a ROM is
 *    known to be bad or not.
 *
 * info.c: -listinfo now supports any hashing function correctly
 *    (both text and XML mode). Notice that XML header should be
 *    rewritten to automatically define the new tags when new
 *    functions are added, but I couldn't be bothered for now.
 *    It also displays informations about baddump/nodump
 *
 * audit.c/h: Updated audit engine (-verifyroms) to use the new
 *    hash functions.
 *
 * fileio.c/h: Updated file engine to use the new hash functions.
 *    It is now possible to load by any specified checksum (in case
 *    later we support other archivers with SHA1 signatures or
 *    equivalent). If the file is open with flag VERIFY_ONLY and
 *    the file is within an archive (zip), only the checksums
 *    available in the archive header are used.
 *
 * windows/fronthlp.c:  Updated -identrom to the new hash engine, now
 *    support any hash function available.
 *    Added -listsha1 and -listmd5. It would be possible to add
 *    also a -listbad now, to list bad dumps (ROMS we need a
 *    redump for)
 *    Updated -listdupcrc to check for all the available checksums.
 *    The output is also a bit more useful and readable, even if it
 *    is still not optimal.
 *    Update -listwrongmerge to check for all the available checksums.
 *
 * windows/fileio.c: Removed check for FILE_TYPE_NOCRC (does not exist
 *    anymore).
 *
 *
 *
 * Technical details:
 *
 * Checksum informations are now stored inside a string. They are
 * stored in "printable hex format", which means that they use
 * more memory than before (since a CRC needs 8 characters to
 * be printed, instead of 4 bytes of raw information). In the
 * driver, they are defined with handy macros which rely on
 * automatic string pasting.
 *
 * Additional flags can also be stored in the string: for now we
 * support NO_DUMP and BAD_DUMP, which replace, respectively,
 * a CRC of 0 and a bit-inverted CRC.
 *
 * All the code that handles hash data is in hash.c. The rest of
 * the core treats the data as an 'opaque type', so that the
 * pointers are just passed along through functions but no
 * operation is performed on the data outside hash.c. This
 * is important in case we want to change the string
 * representation later in the future.
 *
 * When loading a ROM, MAME will calculate and compare the
 * checksum using any function for which the driver has declared
 * an expected checksum. This happens because it would be useless
 * to calculate a checksum if we cannot verify its correctness.
 * For developers, it also means that MAME will not compute the
 * SHA1 for you unless you specify a bogus one in the driver
 * (like SHA1(0)).
 *
 * When verifying a ROM, MAME will use only the checksums available
 * in the archive header (if zip, CRC). This is by design because
 * -verifyroms has always been very fast. It is feasible to add
 * a -fullverifyroms at a later moment, which will decompress the
 * files and compute every checksum that has been declared in the
 * driver.
 *
 * I have also prepared a little tool (SHA1Merger) which takes care
 * of the following tasks:
 *
 * - Given an existing driver in old syntax (0.66 compatible), it will
 *   convert all the existing ROM_LOAD entries in the new format, and
 *   it will automatically compute and add SHA1 checksum for you if
 *   it can find the romset.
 *
 * - Given a romset (ZIP file), it will prepare a ROM definition
 *   skeleton for a driver, containing already rom names, lengths, and
 *   checksums (both CRC and SHA1).
 *
 * The tool is available on www.mame.net as platform-independent source code
 * (in Python), or win32 standalone executable.
 *
 */

#include <stddef.h>
#include <ctype.h>
#include <zlib.h>
#include "hash.h"
#include "md5.h"
#include "sha1.h"
#include "mame.h"
#include "romload.h"

#define ASSERT(x)

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

struct _hash_function_desc
{
	const char* name;           // human-readable name
	char code;                  // single-char code used within the hash string
	unsigned int size;          // checksum size in bytes

	// Functions used to calculate the hash of a memory block
	void (*calculate_begin)(void);
	void (*calculate_buffer)(const void* mem, unsigned long len);
	void (*calculate_end)(UINT8* bin_chksum);

};
typedef struct _hash_function_desc hash_function_desc;

static void h_crc_begin(void);
static void h_crc_buffer(const void* mem, unsigned long len);
static void h_crc_end(UINT8* chksum);

static void h_sha1_begin(void);
static void h_sha1_buffer(const void* mem, unsigned long len);
static void h_sha1_end(UINT8* chksum);

static void h_md5_begin(void);
static void h_md5_buffer(const void* mem, unsigned long len);
static void h_md5_end(UINT8* chksum);

static const hash_function_desc hash_descs[HASH_NUM_FUNCTIONS] =
{
	{
		"crc", 'c', 4,
		h_crc_begin,
		h_crc_buffer,
		h_crc_end
	},

	{
		"sha1", 's', 20,
		h_sha1_begin,
		h_sha1_buffer,
		h_sha1_end
	},

	{
		"md5", 'm', 16,
		h_md5_begin,
		h_md5_buffer,
		h_md5_end
	},
};

static const char* info_strings[] =
{
	"$ND$",       // No dump
	"$BD$"        // Bad dump
};

static const char binToStr[] = "0123456789abcdef";


static const hash_function_desc* hash_get_function_desc(unsigned int function)
{
	unsigned int idx = 0;

	// Calling with zero in here is mostly an internal error
	ASSERT(function != 0);

	// Compute the index of only one function
	while (!(function & 1))
	{
		idx++;
		function >>= 1;
	}

	// Specify only one bit or die
	ASSERT(function == 1);

	return &hash_descs[idx];
}

const char* hash_function_name(unsigned int function)
{
	const hash_function_desc* info = hash_get_function_desc(function);

	return info->name;
}

int hash_data_has_checksum(const char* data, unsigned int function)
{
	const hash_function_desc* info = hash_get_function_desc(function);
	char str[3];
	const char* res;

	str[0] = info->code;
	str[1] = ':';
	str[2] = '\0';

	// Check if the specified hash function is used within this data
	res = strstr(data, str);

	if (!res)
		return 0;

	// Return the offset within the string where the checksum begins
	return (res - data + 2);
}

static int hash_data_add_binary_checksum(char* d, unsigned int function, const UINT8* checksum)
{
	const hash_function_desc* desc = hash_get_function_desc(function);
	char* start = d;
	unsigned i;

	*d++ = desc->code;
	*d++ = ':';

	for (i=0;i<desc->size;i++)
	{
		UINT8 c = *checksum++;

		*d++ = binToStr[(c >> 4) & 0xF];
		*d++ = binToStr[(c >> 0) & 0xF];
	}

	*d++ = '#';

	// Return the number of written bytes
	return (d - start);
}


static int hash_compare_checksum(const char* chk1, const char* chk2, int length)
{
	char c1, c2;

	// The printable format is twice as longer
	length *= 2;

	// This is basically a case-insensitive string compare
	while (length--)
	{
		c1 = *chk1++;
		c2 = *chk2++;

		if (tolower(c1) != tolower(c2))
			return 0;
		if (!c1)
			return 0;
	}

	return 1;
}


// Compare two hashdata
int hash_data_is_equal(const char* d1, const char* d2, unsigned int functions)
{
	int i;
	char incomplete = 0;
	char ok = 0;

	// If no function is specified, it means we need to check for all
	//  of them
	if (!functions)
		functions = ~functions;

	for (i=1; i != (1<<HASH_NUM_FUNCTIONS); i<<=1)
		if (functions & i)
		{
			int offs1, offs2;

			// Check if both hashdata contain the current function's checksum
			offs1 = hash_data_has_checksum(d1, i);
			offs2 = hash_data_has_checksum(d2, i);

			if (offs1 && offs2)
			{
				const hash_function_desc* info = hash_get_function_desc(i);

				if (!hash_compare_checksum(d1+offs1, d2+offs2, info->size))
					return 0;

				ok = 1;
			}
			// If the function was contained only in one, remember that our comparison
			//  is incomplete
			else if (offs1 || offs2)
			{
				incomplete = 1;
			}
		}

	// If we could not compare any function, return error
	if (!ok)
		return 0;

	// Return success code
	return (incomplete ? 2 : 1);
}


int hash_data_extract_printable_checksum(const char* data, unsigned int function, char* checksum)
{
	unsigned int i;
	const hash_function_desc* info;
	int offs;

	// Check if the hashdata contains the requested function
	offs = hash_data_has_checksum(data, function);

	if (!offs)
		return 0;

	// Move to the beginning of the checksum
	data += offs;

	info = hash_get_function_desc(function);

	// Return the number of required bytes
	if (!checksum)
		return info->size*2+1;

	// If the terminator is not found at the right position,
	//  return a full-zero checksum and warn about it. This is mainly
	//  for developers putting checksums of '0' or '1' to ask MAME
	//  to compute the correct values for them.
	if (data[info->size*2] != '#')
	{
		memset(checksum, '0', info->size*2);
		checksum[info->size*2] = '\0';
		return 2;
	}

	// If it contains invalid hexadecimal characters,
	//  treat the checksum as zero and return warning
	for (i=0;i<info->size*2;i++)
		if (!(data[i]>='0' && data[i]<='9') &&
			!(data[i]>='a' && data[i]<='f') &&
			!(data[i]>='A' && data[i]<='F'))
		{
			memset(checksum, '0', info->size*2);
			checksum[info->size*2] = '\0';
			return 2;
		}

	// Copy the checksum (and make it lowercase)
	for (i=0;i<info->size*2;i++)
		checksum[i] = tolower(data[i]);

	checksum[info->size*2] = '\0';

	return 1;
}

static int hex_string_to_binary(unsigned char* binary, const char* data, int size)
{
	unsigned int i;
	char c;

	for (i = 0; i < size * 2; i++)
	{
		c = tolower(*data++);

		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else
			return 1;

		if (i % 2 == 0)
			binary[i / 2] = c * 16;
		else
			binary[i / 2] += c;
	}
	return 0;
}

int hash_data_extract_binary_checksum(const char* data, unsigned int function, unsigned char* checksum)
{
	const hash_function_desc* info;
	int offs;

	// Check if the hashdata contains the requested function
	offs = hash_data_has_checksum(data, function);

	if (!offs)
		return 0;

	// Move to the beginning of the checksum
	data += offs;

	info = hash_get_function_desc(function);

	// Return the number of required bytes
	if (!checksum)
		return info->size;

	// Clear the checksum array
	memset(checksum, 0, info->size);

	// If the terminator is not found at the right position,
	//  return a full-zero checksum and warn about it. This is mainly
	//  for developers putting checksums of '0' or '1' to ask MAME
	//  to compute the correct values for them.
	if (data[info->size*2] != '#')
	{
		memset(checksum, '\0', info->size);
		return 2;
	}

	// Convert hex string into binary
	if (hex_string_to_binary(checksum, data, info->size))
		{
			// Invalid character: the checksum is treated as zero,
			//  and a warning is returned
			memset(checksum, '\0', info->size);
			return 2;
		}

	return 1;
}

int hash_data_has_info(const char* data, unsigned int info)
{
	char* res = strstr(data, info_strings[info]);

	if (!res)
		return 0;

	return 1;
}

void hash_data_copy(char* dst, const char* src)
{
	// Copying string is enough
	strcpy(dst, src);
}

void hash_data_clear(char* dst)
{
	// Clear the buffer
	memset(dst, 0, HASH_BUF_SIZE);
}

unsigned int hash_data_used_functions(const char* data)
{
	int i;
	unsigned int res = 0;

	if (!data)
		return 0;

	for (i=0;i<HASH_NUM_FUNCTIONS;i++)
		if (hash_data_has_checksum(data, 1<<i))
			res |= 1<<i;

	return res;
}

int hash_data_insert_printable_checksum(char* d, unsigned int function, const char* checksum)
{
	const hash_function_desc* desc;
	UINT8 binary_checksum[20];

	desc = hash_get_function_desc(function);

	ASSERT(desc->size <= sizeof(binary_checksum));

	if (hex_string_to_binary(binary_checksum, checksum, desc->size))
		return 2;

	return hash_data_insert_binary_checksum(d, function, binary_checksum);
}

int hash_data_insert_binary_checksum(char* d, unsigned int function, const UINT8* checksum)
{
	int offset;

	offset = hash_data_has_checksum(d, function);

	if (!offset)
	{
		d += strlen(d);
		d += hash_data_add_binary_checksum(d, function, checksum);
		*d = '\0';

		return 1;
	}
	else
	{
		// Move to the start of the whole checksum signature, not only to the checksum
		// itself
		d += offset - 2;

		// Overwrite previous checksum with new one
		hash_data_add_binary_checksum(d, function, checksum);

		return 2;
	}
}

void hash_compute(char* dst, const unsigned char* data, unsigned long length, unsigned int functions)
{
	int i;

	hash_data_clear(dst);

	// Zero means use all the functions
	if (functions == 0)
		functions = ~functions;

	for (i=0;i<HASH_NUM_FUNCTIONS;i++)
	{
		unsigned func = 1 << i;

		if (functions & func)
		{
			const hash_function_desc* desc = hash_get_function_desc(func);
			UINT8 chksum[256];

			desc->calculate_begin();
			desc->calculate_buffer(data, length);
			desc->calculate_end(chksum);

			dst += hash_data_add_binary_checksum(dst, func, chksum);
		}
	}

	*dst = '\0';
}

void hash_data_print(const char* data, unsigned int functions, char* buffer)
{
	int i, j;
	char first = 1;

	if (functions == 0)
		functions = ~functions;

	buffer[0] = '\0';

	for (i=0;i<HASH_NUM_FUNCTIONS;i++)
	{
		unsigned func = 1 << i;

		if ((functions & func) && hash_data_has_checksum(data, func))
		{
			char temp[256];

			if (!first)
				strcat(buffer, " ");
			first = 0;

			strcpy(temp, hash_function_name(func));
			for (j = 0; temp[j]; j++)
				temp[j] = toupper(temp[j]);
			strcat(buffer, temp);
			strcat(buffer, "(");

			hash_data_extract_printable_checksum(data, func, temp);
			strcat(buffer, temp);
			strcat(buffer, ")");
		}
	}
}

int hash_verify_string(const char *hash)
{
	int len, i;

	if (!hash)
		return FALSE;

	while(*hash)
	{
		if (*hash == '$')
		{
			if (memcmp(hash, NO_DUMP, 4) && memcmp(hash, BAD_DUMP, 4))
				return FALSE;
			hash += 4;
		}
		else
		{
			/* first make sure that the next char is a colon */
			if (hash[1] != ':')
				return FALSE;

			/* search for a hash function for this code */
			for (i = 0; i < sizeof(hash_descs) / sizeof(hash_descs[0]); i++)
			{
				if (*hash == hash_descs[i].code)
					break;
			}
			if (i >= sizeof(hash_descs) / sizeof(hash_descs[0]))
				return FALSE;

			/* we have a proper code */
			len = hash_descs[i].size * 2;
			hash += 2;

			for (i = 0; (hash[i] != '#') && (i < len); i++)
			{
				if (!isxdigit(hash[i]))
					return FALSE;
			}
			if (hash[i] != '#')
				return FALSE;

			hash += i+1;
		}
	}
	return TRUE;
}



/*********************************************************************
    Hash functions - Wrappers
 *********************************************************************/

static UINT32 crc;

static void h_crc_begin(void)
{
	crc = 0;
}

static void h_crc_buffer(const void* mem, unsigned long len)
{
	crc = crc32(crc, (UINT8*)mem, len);
}

static void h_crc_end(UINT8* bin_chksum)
{
	bin_chksum[0] = (UINT8)(crc >> 24);
	bin_chksum[1] = (UINT8)(crc >> 16);
	bin_chksum[2] = (UINT8)(crc >> 8);
	bin_chksum[3] = (UINT8)(crc >> 0);
}


static struct sha1_ctx sha1ctx;

static void h_sha1_begin(void)
{
	sha1_init(&sha1ctx);
}

static void h_sha1_buffer(const void* mem, unsigned long len)
{
	sha1_update(&sha1ctx, len, (UINT8*)mem);
}

static void h_sha1_end(UINT8* bin_chksum)
{
	sha1_final(&sha1ctx);
	sha1_digest(&sha1ctx, 20, bin_chksum);
}


static struct MD5Context md5_ctx;

static void h_md5_begin(void)
{
	MD5Init(&md5_ctx);
}

static void h_md5_buffer(const void* mem, unsigned long len)
{
	MD5Update(&md5_ctx, (md5byte*)mem, len);
}

static void h_md5_end(UINT8* bin_chksum)
{
	MD5Final(bin_chksum, &md5_ctx);
}
