/***************************************************************************

    audit.h

    ROM, disk, and sample auditing functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
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
#define CORRECT							0
#define BEST_AVAILABLE					1
#define INCORRECT						2
#define NOTFOUND						3

/* image types for audit_record.type */
#define AUDIT_FILE_ROM					0
#define AUDIT_FILE_DISK					1
#define AUDIT_FILE_SAMPLE				2

/* status values for audit_record.status */
#define AUDIT_STATUS_GOOD				0
#define AUDIT_STATUS_FOUND_INVALID		1
#define AUDIT_STATUS_NOT_FOUND			2
#define AUDIT_STATUS_ERROR				3

/* substatus values for audit_record.substatus */
#define SUBSTATUS_GOOD					0
#define SUBSTATUS_GOOD_NEEDS_REDUMP		1
#define SUBSTATUS_FOUND_NODUMP			2
#define SUBSTATUS_FOUND_BAD_CHECKSUM	3
#define SUBSTATUS_FOUND_WRONG_LENGTH	4
#define SUBSTATUS_NOT_FOUND				5
#define SUBSTATUS_NOT_FOUND_NODUMP		6
#define SUBSTATUS_NOT_FOUND_OPTIONAL	7
#define SUBSTATUS_NOT_FOUND_PARENT		8
#define SUBSTATUS_NOT_FOUND_BIOS		9
#define SUBSTATUS_ERROR					100



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
	const char * 	exphash;        		/* expected hash data */
	char 			hash[HASH_BUF_SIZE];	/* actual hash information */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int audit_images(core_options *options, const game_driver *gamedrv, UINT32 validation, audit_record **audit);
int audit_samples(core_options *options, const game_driver *gamedrv, audit_record **audit);
int audit_summary(const game_driver *gamedrv, int count, const audit_record *records, int output);


#endif	/* __AUDIT_H__ */
