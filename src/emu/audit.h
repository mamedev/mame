/***************************************************************************

    audit.h

    ROM, disk, and sample auditing functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __AUDIT_H__
#define __AUDIT_H__

#include "mamecore.h"
#include "hash.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* hashes to use for validation */
#define AUDIT_VALIDATE_FAST				(HASH_CRC)
#define AUDIT_VALIDATE_FULL				(HASH_CRC | HASH_SHA)

/* return values from audit_verify_roms and audit_verify_samples */
enum
{
	CORRECT = 0,
	BEST_AVAILABLE,
	INCORRECT,
	NOTFOUND
};

/* image types for audit_record.type */
enum
{
	AUDIT_FILE_ROM = 0,
	AUDIT_FILE_DISK,
	AUDIT_FILE_SAMPLE
};

/* status values for audit_record.status */
enum
{
	AUDIT_STATUS_GOOD = 0,
	AUDIT_STATUS_FOUND_INVALID,
	AUDIT_STATUS_NOT_FOUND,
	AUDIT_STATUS_ERROR
};

/* substatus values for audit_record.substatus */
enum
{
	SUBSTATUS_GOOD = 0,
	SUBSTATUS_GOOD_NEEDS_REDUMP,
	SUBSTATUS_FOUND_NODUMP,
	SUBSTATUS_FOUND_BAD_CHECKSUM,
	SUBSTATUS_FOUND_WRONG_LENGTH,
	SUBSTATUS_NOT_FOUND,
	SUBSTATUS_NOT_FOUND_NODUMP,
	SUBSTATUS_NOT_FOUND_OPTIONAL,
	SUBSTATUS_NOT_FOUND_PARENT,
	SUBSTATUS_NOT_FOUND_BIOS,
	SUBSTATUS_ERROR = 100
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _audit_record audit_record;
struct _audit_record
{
	UINT8			type;					/* type of item that was audited */
	UINT8			status;					/* status of audit on this item */
	UINT8			substatus;				/* finer-detail status */
	const char *	name;					/* name of item */
	UINT32			explength;				/* expected length of item */
	UINT32			length;					/* actual length of item */
	const char *	exphash;        		/* expected hash data */
	char			hash[HASH_BUF_SIZE];	/* actual hash information */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int audit_images(core_options *options, const game_driver *gamedrv, UINT32 validation, audit_record **audit);
int audit_samples(core_options *options, const game_driver *gamedrv, audit_record **audit);
int audit_summary(const game_driver *gamedrv, int count, const audit_record *records, int output);


#endif	/* __AUDIT_H__ */
