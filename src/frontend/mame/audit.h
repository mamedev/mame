// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    audit.h

    ROM, disk, and sample auditing functions.

***************************************************************************/
#ifndef MAME_FRONTEND_AUDIT_H
#define MAME_FRONTEND_AUDIT_H

#pragma once

#include <iosfwd>
#include <list>
#include <utility>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// hashes to use for validation
#define AUDIT_VALIDATE_FAST             "R"     /* CRC only */
#define AUDIT_VALIDATE_FULL             "RS"    /* CRC + SHA1 */



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// forward declarations
class driver_enumerator;
class software_list_device;



// ======================> media_auditor

// class which manages auditing of items
class media_auditor
{
public:
	enum class media_type
	{
		ROM = 0,
		DISK,
		SAMPLE
	};

	// status values
	enum class audit_status
	{
		GOOD = 0,
		FOUND_INVALID,
		NOT_FOUND,
		UNVERIFIED = 100
	};

	// substatus values
	enum class audit_substatus
	{
		GOOD = 0,
		GOOD_NEEDS_REDUMP,
		FOUND_NODUMP,
		FOUND_BAD_CHECKSUM,
		FOUND_WRONG_LENGTH,
		NOT_FOUND,
		NOT_FOUND_NODUMP,
		NOT_FOUND_OPTIONAL,
		UNVERIFIED = 100
	};

	// summary values
	enum summary
	{
		CORRECT = 0,
		NONE_NEEDED,
		BEST_AVAILABLE,
		INCORRECT,
		NOTFOUND
	};

	// holds the result of auditing a single item
	class audit_record
	{
	public:
		// media types
		// construction/destruction
		audit_record(const rom_entry &media, media_type type);
		audit_record(const char *name, media_type type);
		audit_record(const audit_record &) = default;
		audit_record(audit_record &&) = default;
		audit_record &operator=(const audit_record &) = default;
		audit_record &operator=(audit_record &&) = default;

		// getters
		media_type type() const { return m_type; }
		audit_status status() const { return m_status; }
		audit_substatus substatus() const { return m_substatus; }
		const std::string &name() const { return m_name; }
		uint64_t expected_length() const { return m_explength; }
		uint64_t actual_length() const { return m_length; }
		const util::hash_collection &expected_hashes() const { return m_exphashes; }
		const util::hash_collection &actual_hashes() const { return m_hashes; }
		std::add_pointer_t<device_type> shared_device() const { return m_shared_device; }

		// setters
		void set_status(audit_status status, audit_substatus substatus)
		{
			m_status = status;
			m_substatus = substatus;
		}

		void set_actual(const util::hash_collection &hashes, uint64_t length = 0)
		{
			m_hashes = hashes;
			m_length = length;
		}

		void set_actual(util::hash_collection &&hashes, uint64_t length = 0)
		{
			m_hashes = std::move(hashes);
			m_length = length;
		}

		void set_shared_device(std::add_pointer_t<device_type> shared_device)
		{
			m_shared_device = shared_device;
		}

	private:
		// internal state
		media_type                      m_type;             // type of item that was audited
		audit_status                    m_status;           // status of audit on this item
		audit_substatus                 m_substatus;        // finer-detail status
		std::string                     m_name;             // name of item
		uint64_t                        m_explength;        // expected length of item
		uint64_t                        m_length;           // actual length of item
		util::hash_collection           m_exphashes;        // expected hash data
		util::hash_collection           m_hashes;           // actual hash information
		std::add_pointer_t<device_type> m_shared_device;    // device that shares the ROM
	};
	using record_list = std::list<audit_record>;

	// construction/destruction
	media_auditor(const driver_enumerator &enumerator);

	// getters
	const record_list &records() const { return m_record_list; }

	// audit operations
	summary audit_media(const char *validation = AUDIT_VALIDATE_FULL);
	summary audit_device(device_t &device, const char *validation = AUDIT_VALIDATE_FULL);
	summary audit_software(software_list_device &swlist, const software_info &swinfo, const char *validation = AUDIT_VALIDATE_FULL);
	summary audit_samples();
	summary summarize(const char *name, std::ostream *output = nullptr) const;

private:
	// internal helpers
	template <typename T> void audit_regions(T do_audit, const rom_entry *region, std::size_t &found, std::size_t &required);
	audit_record &audit_one_rom(const std::vector<std::string> &searchpath, const rom_entry *rom);
	template <typename... T> audit_record &audit_one_disk(const rom_entry *rom, T &&... args);
	void compute_status(audit_record &record, const rom_entry *rom, bool found);

	// internal state
	record_list                 m_record_list;
	const driver_enumerator &   m_enumerator;
	const char *                m_validation;
};


#endif  // MAME_FRONTEND_AUDIT_H
