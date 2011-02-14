/*********************************************************************

    hashfile.h

    Code for parsing hash info (*.hsi) files

*********************************************************************/

#ifndef __HASHFILE_H__
#define __HASHFILE_H__

#include "emu.h"
#include "hash.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _hash_info hash_info;
struct _hash_info
{
	hash_collection hashes;
	const char *longname;
	const char *manufacturer;
	const char *year;
	const char *playable;
	const char *pcb;
	const char *extrainfo;
};

typedef struct _hash_file hash_file;

typedef void (*hashfile_error_func)(const char *message);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* opens a hash file; if is_preload is non-zero, the entire file is preloaded */
hash_file *hashfile_open(core_options &options, const char *sysname, int is_preload, hashfile_error_func error_proc);

/* closes a hash file and associated resources */
void hashfile_close(hash_file *hashfile);

/* looks up information in a hash file */
const hash_info *hashfile_lookup(hash_file *hashfile, const hash_collection &hashes);

/* performs a syntax check on a hash file */
int hashfile_verify(const char *sysname, void (*error_proc)(const char *message));

/* returns the functions used in this hash file */
const char *hashfile_functions_used(hash_file *hashfile, iodevice_t devtype);


#endif /* __HASHFILE_H__ */
