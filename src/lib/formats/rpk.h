// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

	rpk.h

	RPK format support

***************************************************************************/

#ifndef MAME_FORMATS_RPK_H
#define MAME_FORMATS_RPK_H

#pragma once

#include "unzip.h"
#include "hash.h"
#include "xmlfile.h"

#include <exception>
#include <optional>

namespace util {
/***************************************************************************
	TYPE DEFINITIONS
***************************************************************************/

class archive_file;
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
	rpk_socket(rpk_file &rpk, std::string &&id, socket_type type, std::string &&filename, std::optional<hash_collection> &&hashes, std::optional<std::uint32_t> length);
	rpk_socket(const rpk_socket &) = delete;
	rpk_socket(rpk_socket &&) = delete;
	~rpk_socket();

	// accessors
	const std::string &id() const { return m_id; }
	socket_type type() const { return m_type; }
	const std::string &filename() const { return m_filename; }
	std::uint32_t length() const { return m_length.value(); }

	// methods
	std::vector<std::uint8_t> read_file() const;

private:
	rpk_file &						m_rpk;
	std::string						m_id;
	socket_type						m_type;
	std::string						m_filename;
	std::optional<hash_collection>	m_hashes;
	std::optional<std::uint32_t>	m_length;
};


// ======================> rpk_file

class rpk_file
{
	friend class rpk_reader;
	friend class rpk_socket;

public:
	// ctor/dtor
	rpk_file(const rpk_file &) = delete;
	rpk_file(rpk_file &&) = default;
	~rpk_file();

	// accessors
	int pcb_type() const { return m_pcb_type; }
	const std::list<rpk_socket> &sockets() const { return m_sockets; }

private:
	archive_file::ptr		m_zipfile;
	int						m_pcb_type;
	std::list<rpk_socket>	m_sockets;

	// ctor
	rpk_file(archive_file::ptr &&zipfile, int pcb_type);

	// accesors
	archive_file &zipfile() { return *m_zipfile; }

	// methods
	void add_rom_socket(std::string &&id, const util::xml::data_node &rom_resource_node);
	void add_ram_socket(std::string &&id, const util::xml::data_node &ram_resource_node);
};


// ======================> rpk_reader

class rpk_reader
{
public:
	enum class error
	{
		NOT_ZIP_FORMAT = 1,
		XML_ERROR,
		INVALID_FILE_REF,
		ZIP_ERROR,
		ZIP_UNSUPPORTED,
		MISSING_RAM_LENGTH,
		INVALID_RAM_SPEC,
		INVALID_RESOURCE_REF,
		INVALID_LAYOUT,
		MISSING_LAYOUT,
		UNKNOWN_PCB_TYPE,
		UNSUPPORTED_RPK_FEATURE
	};

	// ctor/dtor
	rpk_reader(const char **pcb_types, bool supports_ram);
	rpk_reader(const rpk_reader &) = delete;
	rpk_reader(rpk_reader &&) = delete;

	// methods
	rpk_file read(const std::string &filename) const;

private:
	const char **	m_pcb_types;
	bool			m_supports_ram;
};


// ======================> rpk_exception

class rpk_exception : public std::exception
{
public:
	rpk_exception(rpk_reader::error error);
	rpk_exception(rpk_reader::error error, std::string_view details);
	rpk_exception(archive_file::error ziperr);
	virtual const char *what() const noexcept override;

private:
	std::string m_what;

	static const char *error_message(rpk_reader::error error);
};


};

#endif // MAME_FORMATS_RPK_H
