// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    rpk.h

    RPK format support

***************************************************************************/

#ifndef MAME_FORMATS_RPK_H
#define MAME_FORMATS_RPK_H

#pragma once

#include "hash.h"
#include "unzip.h"

#include <cassert>
#include <list>
#include <optional>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rpk_reader;
class rpk_file;

// ======================> rpk_socket

class rpk_socket
{
	friend class rpk_reader;
public:
	enum class socket_type
	{
		ROM,
		RAM,
		PERSISTENT_RAM
	};

	// ctor/dtor
	rpk_socket(rpk_file &rpk, std::string &&id, socket_type type, std::string &&filename, std::optional<util::hash_collection> &&hashes, std::uint32_t length = ~0);
	rpk_socket(const rpk_socket &) = delete;
	rpk_socket(rpk_socket &&) = delete;
	~rpk_socket();

	// accessors
	const std::string &id() const noexcept { return m_id; }
	socket_type type() const noexcept { return m_type; }
	const std::string &filename() const noexcept { return m_filename; }
	std::uint32_t length() const noexcept { assert(m_type == socket_type::RAM || m_type == socket_type::PERSISTENT_RAM); return m_length; }

	// methods
	std::error_condition read_file(std::vector<std::uint8_t> &result) const;

private:
	rpk_file &                              m_rpk;
	std::string                             m_id;
	socket_type                             m_type;
	std::string                             m_filename;
	std::optional<util::hash_collection>    m_hashes;
	std::uint32_t                           m_length;
};


// ======================> rpk_file

class rpk_file
{
	friend class rpk_reader;
	friend class rpk_socket;

public:
	typedef std::unique_ptr<rpk_file> ptr;

	// ctor/dtor
	rpk_file(util::archive_file::ptr &&zipfile, int pcb_type);
	rpk_file(const rpk_file &) = delete;
	rpk_file(rpk_file &&) = delete;
	~rpk_file();

	// accessors
	int pcb_type() const { return m_pcb_type; }
	const std::list<rpk_socket> &sockets() const { return m_sockets; }

private:
	util::archive_file::ptr     m_zipfile;
	int                         m_pcb_type;
	std::list<rpk_socket>       m_sockets;

	// accesors
	util::archive_file &zipfile() { return *m_zipfile; }

	// methods
	std::error_condition add_rom_socket(std::string &&id, const util::xml::data_node &rom_resource_node);
	std::error_condition add_ram_socket(std::string &&id, const util::xml::data_node &ram_resource_node);
};


// ======================> rpk_reader

class rpk_reader
{
public:
	enum class error
	{
		XML_ERROR = 1,
		INVALID_FILE_REF,
		MISSING_RAM_LENGTH,
		INVALID_RAM_SPEC,
		INVALID_RESOURCE_REF,
		INVALID_LAYOUT,
		MISSING_LAYOUT,
		UNKNOWN_PCB_TYPE,
		UNSUPPORTED_RPK_FEATURE
	};

	// ctor/dtor
	rpk_reader(char const *const *pcb_types, bool supports_ram);
	rpk_reader(const rpk_reader &) = delete;
	rpk_reader(rpk_reader &&) = delete;

	// methods
	std::error_condition read(std::unique_ptr<util::random_read> &&stream, rpk_file::ptr &result) const;

private:
	char const *const * m_pcb_types;
	bool                m_supports_ram;
};


// error category for RPK errors
std::error_category const &rpk_category() noexcept;
inline std::error_condition make_error_condition(rpk_reader::error err) noexcept { return std::error_condition(int(err), rpk_category()); }

namespace std {
	template <> struct is_error_condition_enum<rpk_reader::error> : public std::true_type { };
} // namespace std

#endif // MAME_FORMATS_RPK_H
