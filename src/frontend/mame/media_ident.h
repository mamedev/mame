// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    media_ident.h

    Media identify.

***************************************************************************/
#ifndef MAME_FRONTEND_MEDIA_IDENT_H
#define MAME_FRONTEND_MEDIA_IDENT_H

#include "romload.h"
#include <vector>


// media_identifier class identifies media by hash via a search in
// the driver database
class media_identifier
{
public:
	// construction/destruction
	media_identifier(emu_options &options);

	// getters
	unsigned total() const { return m_total; }
	unsigned matches() const { return m_matches; }
	unsigned nonroms() const { return m_nonroms; }

	// operations
	void reset() { m_total = m_matches = m_nonroms = 0; }
	void identify(const char *name);
	void identify_file(const char *name);
	void identify_data(const char *name, const uint8_t *data, std::size_t length);

private:
	enum class file_flavour
	{
		RAW,
		JED,
		CHD
	};

	class match_data
	{
	public:
		match_data(
				std::string &&shortname,
				std::string &&fullname,
				std::string &&romname,
				bool bad,
				bool device)
			: m_shortname(std::move(shortname))
			, m_fullname(std::move(fullname))
			, m_romname(std::move(romname))
			, m_bad(bad)
			, m_device(device)
		{
		}

		match_data(match_data const &) = default;
		match_data(match_data &&) = default;
		match_data &operator=(match_data const &) = default;
		match_data &operator=(match_data &&) = default;

		std::string const &shortname() const { return m_shortname; }
		std::string const &fullname() const { return m_fullname; }
		std::string const &romname() const { return m_romname; }
		bool bad() const { return m_bad; }
		bool device() const { return m_device; }

	private:
		std::string m_shortname;
		std::string m_fullname;
		std::string m_romname;
		bool        m_bad;
		bool        m_device;
	};

	class file_info
	{
	public:
		file_info(
				std::string &&name,
				std::uint64_t length,
				util::hash_collection &&hashes,
				file_flavour flavour)
			: m_name(std::move(name))
			, m_length(length)
			, m_hashes(std::move(hashes))
			, m_flavour(flavour)
		{
		}

		file_info(file_info const &) = default;
		file_info(file_info &&) = default;
		file_info &operator=(file_info const &) = default;
		file_info &operator=(file_info &&) = default;

		std::string const &name() const { return m_name; }
		std::uint64_t length() const { return m_length; }
		util::hash_collection const &hashes() const { return m_hashes; }
		file_flavour flavour() const { return m_flavour; }
		std::vector<match_data> const &matches() const { return m_matches; }

		void match(device_t const &device, romload::file const &rom, util::hash_collection const &hashes);
		void match(std::string const &list, software_info const &software, rom_entry const &rom, util::hash_collection const &hashes);

	private:
		std::string             m_name;
		std::uint64_t           m_length;
		util::hash_collection   m_hashes;
		file_flavour            m_flavour;
		std::vector<match_data> m_matches;
	};

	void collect_files(std::vector<file_info> &info, char const *path);
	void digest_file(std::vector<file_info> &info, char const *path);
	void digest_data(std::vector<file_info> &info, char const *name, void const *data, std::uint64_t length);
	void match_hashes(std::vector<file_info> &info);
	void print_results(std::vector<file_info> const &info);

	driver_enumerator       m_drivlist;
	unsigned                m_total;
	unsigned                m_matches;
	unsigned                m_nonroms;
};


#endif  /* MAME_FRONTEND_MEDIA_IDENT_H */
