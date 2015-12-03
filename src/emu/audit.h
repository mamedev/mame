// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    audit.h

    ROM, disk, and sample auditing functions.

***************************************************************************/

#pragma once

#ifndef __AUDIT_H__
#define __AUDIT_H__

#include "drivenum.h"
#include "hash.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// hashes to use for validation
#define AUDIT_VALIDATE_FAST             "R"     /* CRC only */
#define AUDIT_VALIDATE_FULL             "RS"    /* CRC + SHA1 */



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> audit_record

// holds the result of auditing a single item
class audit_record
{
	friend class simple_list<audit_record>;

public:
	// media types
	enum media_type
	{
		MEDIA_ROM = 0,
		MEDIA_DISK,
		MEDIA_SAMPLE
	};

	// status values
	enum audit_status
	{
		STATUS_GOOD = 0,
		STATUS_FOUND_INVALID,
		STATUS_NOT_FOUND,
		STATUS_ERROR
	};

	// substatus values
	enum audit_substatus
	{
		SUBSTATUS_GOOD = 0,
		SUBSTATUS_GOOD_NEEDS_REDUMP,
		SUBSTATUS_FOUND_NODUMP,
		SUBSTATUS_FOUND_BAD_CHECKSUM,
		SUBSTATUS_FOUND_WRONG_LENGTH,
		SUBSTATUS_NOT_FOUND,
		SUBSTATUS_NOT_FOUND_NODUMP,
		SUBSTATUS_NOT_FOUND_OPTIONAL,
		SUBSTATUS_ERROR = 100
	};

	// construction/destruction
	audit_record(const rom_entry &media, media_type type);
	audit_record(const char *name, media_type type);

	// getters
	audit_record *next() const { return m_next; }
	media_type type() const { return m_type; }
	audit_status status() const { return m_status; }
	audit_substatus substatus() const { return m_substatus; }
	const char *name() const { return m_name; }
	UINT64 expected_length() const { return m_explength; }
	UINT64 actual_length() const { return m_length; }
	const hash_collection &expected_hashes() const { return m_exphashes; }
	const hash_collection &actual_hashes() const { return m_hashes; }
	device_t *shared_device() const { return m_shared_device; }

	// setters
	void set_status(audit_status status, audit_substatus substatus)
	{
		m_status = status;
		m_substatus = substatus;
	}

	void set_actual(const hash_collection &hashes, UINT64 length = 0)
	{
		m_hashes = hashes;
		m_length = length;
	}

	void set_shared_device(device_t *shared_device)
	{
		m_shared_device = shared_device;
	}

private:
	// internal state
	audit_record *      m_next;
	media_type          m_type;                 /* type of item that was audited */
	audit_status        m_status;               /* status of audit on this item */
	audit_substatus     m_substatus;            /* finer-detail status */
	const char *        m_name;                 /* name of item */
	UINT64              m_explength;            /* expected length of item */
	UINT64              m_length;               /* actual length of item */
	hash_collection     m_exphashes;            /* expected hash data */
	hash_collection     m_hashes;               /* actual hash information */
	device_t *          m_shared_device;        /* device that shares the rom */
};


// ======================> media_auditor

// class which manages auditing of items
class media_auditor
{
public:
	// summary values
	enum summary
	{
		CORRECT = 0,
		NONE_NEEDED,
		BEST_AVAILABLE,
		INCORRECT,
		NOTFOUND
	};

	// construction/destruction
	media_auditor(const driver_enumerator &enumerator);

	// getters
	audit_record *first() const { return m_record_list.first(); }
	int count() const { return m_record_list.count(); }

	// audit operations
	summary audit_media(const char *validation = AUDIT_VALIDATE_FULL);
	summary audit_device(device_t *device, const char *validation = AUDIT_VALIDATE_FULL);
	summary audit_software(const char *list_name, software_info *swinfo, const char *validation = AUDIT_VALIDATE_FULL);
	summary audit_samples();
	summary summarize(const char *name,std::string *output = nullptr);

private:
	// internal helpers
	audit_record *audit_one_rom(const rom_entry *rom);
	audit_record *audit_one_disk(const rom_entry *rom, const char *locationtag = nullptr);
	void compute_status(audit_record &record, const rom_entry *rom, bool found);
	device_t *find_shared_device(device_t &device, const char *name, const hash_collection &romhashes, UINT64 romlength);

	// internal state
	simple_list<audit_record>   m_record_list;
	const driver_enumerator &   m_enumerator;
	const char *                m_validation;
	const char *                m_searchpath;
};


#endif  /* __AUDIT_H__ */
