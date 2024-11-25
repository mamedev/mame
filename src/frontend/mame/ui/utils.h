// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/utils.h

    Internal UI user interface.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_UTILS_H
#define MAME_FRONTEND_UI_UTILS_H

#pragma once

#include "softlist.h"
#include "unicode.h"

// FIXME: allow OSD module headers to be included in a less ugly way
#include "../osd/modules/lib/osdlib.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>


class mame_ui_manager;
class render_container;


// TODO: namespace these things

struct ui_system_info
{
	ui_system_info(ui_system_info const &) = default;
	ui_system_info(ui_system_info &&) = default;
	ui_system_info &operator=(ui_system_info const &) = default;
	ui_system_info &operator=(ui_system_info &&) = default;

	ui_system_info() { }
	ui_system_info(game_driver const &d, int i, bool a) : driver(&d), index(i), available(a) { }

	game_driver const *driver = nullptr;
	int index;
	bool is_clone = false;
	bool available = false;

	std::string description;
	std::string parent;

	std::wstring reading_description;
	std::wstring reading_parent;

	std::u32string ucs_shortname;
	std::u32string ucs_description;
	std::u32string ucs_reading_description;
	std::u32string ucs_manufacturer_description;
	std::u32string ucs_manufacturer_reading_description;
	std::u32string ucs_default_description;
	std::u32string ucs_manufacturer_default_description;
};

struct ui_software_info
{
	ui_software_info() { }

	// info for software list item
	ui_software_info(
			software_info const &sw,
			software_part const &p,
			game_driver const &d,
			std::string const &li,
			std::string const &is,
			std::string const &de);

	// info for starting empty
	ui_software_info(game_driver const &d);

	// copyable/movable
	ui_software_info(ui_software_info const &that);
	ui_software_info(ui_software_info &&that) = default;
	ui_software_info &operator=(ui_software_info const &that);
	ui_software_info &operator=(ui_software_info &&) = default;

	bool operator==(ui_software_info const &r) const
	{
		// compares all fields except info (fragile), alttitles (included in info) and available (environmental)
		return (shortname == r.shortname) && (longname == r.longname) && (parentname == r.parentname)
			   && (year == r.year) && (publisher == r.publisher) && (supported == r.supported)
			   && (part == r.part) && (driver == r.driver) && (listname == r.listname)
			   && (interface == r.interface) && (instance == r.instance) && (startempty == r.startempty)
			   && (parentlongname == r.parentlongname) && (devicetype == r.devicetype);
	}

	std::string shortname;
	std::string longname;
	std::string parentname;
	std::string year;
	std::string publisher;
	software_support supported = software_support::SUPPORTED;
	std::string part;
	game_driver const *driver = nullptr;
	std::string listname;
	std::string interface;
	std::string instance;
	uint8_t startempty = 0;
	std::string parentlongname;
	std::string infotext;
	std::string devicetype;
	std::vector<software_info_item> info;
	std::vector<std::reference_wrapper<std::string const> > alttitles;
	bool available = false;
};


void swap(ui_system_info &a, ui_system_info &b) noexcept;


namespace ui {

class machine_filter_data;
class software_filter_data;


template <class Impl, typename Entry>
class filter_base
{
public:
	typedef std::unique_ptr<Impl> ptr;

	virtual ~filter_base() { }

	virtual char const *config_name() const = 0;
	virtual char const *display_name() const = 0;
	virtual char const *filter_text() const = 0;

	virtual bool apply(Entry const &info) const = 0;

	virtual void show_ui(mame_ui_manager &mui, render_container &container, std::function<void (Impl &)> &&handler) = 0;

	virtual bool wants_adjuster() const = 0;
	virtual char const *adjust_text() const = 0;
	virtual uint32_t arrow_flags() const = 0;
	virtual bool adjust_left() = 0;
	virtual bool adjust_right() = 0;

	virtual void save_ini(util::core_file &file, unsigned indent) const = 0;

	template <typename InputIt, class OutputIt>
	void apply(InputIt first, InputIt last, OutputIt dest) const
	{
		std::copy_if(first, last, dest, [this] (auto const &info) { return this->apply(info); });
	}

protected:
	using entry_type = Entry;

	filter_base() { }

	bool apply(Entry const *info) const { return apply(*info); }
};


class machine_filter : public filter_base<machine_filter, ui_system_info>
{
public:
	enum type : uint16_t
	{
		ALL = 0,
		AVAILABLE,
		UNAVAILABLE,
		WORKING,
		NOT_WORKING,
		MECHANICAL,
		NOT_MECHANICAL,
		CATEGORY,
		FAVORITE,
		BIOS,
		NOT_BIOS,
		PARENTS,
		CLONES,
		MANUFACTURER,
		YEAR,
		SOURCE_FILE,
		SAVE,
		NOSAVE,
		CHD,
		NOCHD,
		VERTICAL,
		HORIZONTAL,
		CUSTOM,

		COUNT,
		FIRST = 0,
		LAST = COUNT - 1
	};

	virtual type get_type() const = 0;
	virtual std::string adorned_display_name(type n) const = 0;

	static ptr create(type n, machine_filter_data const &data) { return create(n, data, nullptr, nullptr, 0); }
	static ptr create(util::core_file &file, machine_filter_data const &data) { return create(file, data, 0); }
	static char const *config_name(type n);
	static char const *display_name(type n);

	using filter_base<machine_filter, ui_system_info>::config_name;
	using filter_base<machine_filter, ui_system_info>::display_name;

protected:
	machine_filter();

	static ptr create(type n, machine_filter_data const &data, char const *value, util::core_file *file, unsigned indent);
	static ptr create(util::core_file &file, machine_filter_data const &data, unsigned indent);
};

DECLARE_ENUM_INCDEC_OPERATORS(machine_filter::type)


class software_filter : public filter_base<software_filter, ui_software_info>
{
public:
	enum type : uint16_t
	{
		ALL = 0,
		AVAILABLE,
		UNAVAILABLE,
		FAVORITE,
		PARENTS,
		CLONES,
		YEAR,
		PUBLISHERS,
		DEVELOPERS,
		DISTRIBUTORS,
		AUTHORS,
		PROGRAMMERS,
		SUPPORTED,
		PARTIAL_SUPPORTED,
		UNSUPPORTED,
		REGION,
		DEVICE_TYPE,
		LIST,
		CUSTOM,

		COUNT,
		FIRST = 0,
		LAST = COUNT - 1
	};

	virtual type get_type() const = 0;
	virtual std::string adorned_display_name(type n) const = 0;

	static ptr create(type n, software_filter_data const &data) { return create(n, data, nullptr, nullptr, 0); }
	static ptr create(util::core_file &file, software_filter_data const &data) { return create(file, data, 0); }
	static char const *config_name(type n);
	static char const *display_name(type n);

	using filter_base<software_filter, ui_software_info>::config_name;
	using filter_base<software_filter, ui_software_info>::display_name;

protected:
	software_filter();

	static ptr create(type n, software_filter_data const &data, char const *value, util::core_file *file, unsigned indent);
	static ptr create(util::core_file &file, software_filter_data const &data, unsigned indent);
};

DECLARE_ENUM_INCDEC_OPERATORS(software_filter::type)


class machine_filter_data
{
public:
	std::vector<std::string> const &manufacturers()     const { return m_manufacturers; }
	std::vector<std::string> const &years()             const { return m_years; }
	std::vector<std::string> const &source_files()      const { return m_source_files; }

	// adding entries
	void add_manufacturer(std::string const &manufacturer);
	void add_year(std::string const &year);
	void add_source_file(std::string_view path);

	void finalise();

	// use heuristics to extract meaningful parts from machine metadata
	static std::string extract_manufacturer(std::string const &manufacturer);

	// the selected filter
	machine_filter::type get_current_filter_type() const { return m_current_filter; }
	void set_current_filter_type(machine_filter::type type) { m_current_filter = type; }

	// managing current filters
	void set_filter(machine_filter::ptr &&filter);
	auto const &get_filters() { return m_filters; }
	machine_filter &get_filter(machine_filter::type type);
	machine_filter *get_current_filter()
	{
		auto const it(m_filters.find(m_current_filter));
		return (m_filters.end() != it) ? it->second.get() : nullptr;
	}
	std::string get_config_string() const;
	bool load_ini(util::core_file &file);

private:
	using filter_map = std::map<machine_filter::type, machine_filter::ptr>;

	std::vector<std::string>    m_manufacturers;
	std::vector<std::string>    m_years;
	std::vector<std::string>    m_source_files;

	machine_filter::type        m_current_filter = machine_filter::ALL;
	filter_map                  m_filters;
};


class software_filter_data
{
public:
	std::vector<std::string> const &regions()           const { return m_regions; }
	std::vector<std::string> const &publishers()        const { return m_publishers; }
	std::vector<std::string> const &years()             const { return m_years; }
	std::vector<std::string> const &developers()        const { return m_developers; }
	std::vector<std::string> const &distributors()      const { return m_distributors; }
	std::vector<std::string> const &authors()           const { return m_authors; }
	std::vector<std::string> const &programmers()       const { return m_programmers; }
	std::vector<std::string> const &device_types()      const { return m_device_types; }
	std::vector<std::string> const &list_names()        const { return m_list_names; }
	std::vector<std::string> const &list_descriptions() const { return m_list_descriptions; }

	// adding entries
	void add_region(std::string const &longname);
	void add_publisher(std::string const &publisher);
	void add_year(std::string const &year);
	void add_info(software_info_item const &info);
	void add_device_type(std::string const &device_type);
	void add_list(std::string const &name, std::string const &description);
	void finalise();

	// use heuristics to extract meaningful parts from software list fields
	static std::string extract_region(std::string const &longname);
	static std::string extract_publisher(std::string const &publisher);

private:
	std::vector<std::string>    m_regions;
	std::vector<std::string>    m_publishers;
	std::vector<std::string>    m_years;
	std::vector<std::string>    m_developers;
	std::vector<std::string>    m_distributors;
	std::vector<std::string>    m_authors;
	std::vector<std::string>    m_programmers;
	std::vector<std::string>    m_device_types;
	std::vector<std::string>    m_list_names, m_list_descriptions;
};

} // namespace ui

#define MAX_CHAR_INFO            256

enum
{
	SHOW_PANELS = 0,
	HIDE_LEFT_PANEL,
	HIDE_RIGHT_PANEL,
	HIDE_BOTH
};

// FIXME: this stuff shouldn't all be globals

// GLOBAL CLASS
struct ui_globals
{
	static uint8_t      curdats_view, curdats_total, cur_sw_dats_view, cur_sw_dats_total;
	static bool         reset;
};

// GLOBAL FUNCTIONS
char* chartrimcarriage(char str[]);
int getprecisionchr(const char* s);
std::vector<std::string> tokenize(const std::string &text, char sep);


namespace ui {

//-------------------------------------------------
//  input_character - inputs a typed character
//  into a buffer
//-------------------------------------------------

template <typename F>
bool input_character(std::string &buffer, std::string::size_type size, char32_t unichar, F &&filter)
{
	auto const buflen(buffer.length());
	if ((unichar == 8) || (unichar == 0x7f))
	{
		// backspace
		if (0 < buflen)
		{
			auto const buffer_oldend(buffer.c_str() + buflen);
			auto const buffer_newend(utf8_previous_char(buffer_oldend));
			buffer.resize(buffer_newend - buffer.c_str());
			return true;
		}
	}
	else if ((unichar >= ' ') && filter(unichar))
	{
		// append this character - check against the size first
		char utf8char[UTF8_CHAR_MAX];
		auto const utf8len(utf8_from_uchar(utf8char, std::size(utf8char), unichar));
		if ((0 < utf8len) && (size >= utf8len) && ((size - utf8len) >= buflen))
		{
			buffer.append(utf8char, utf8len);
			return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  input_character - inputs a typed character
//  into a buffer
//-------------------------------------------------

template <typename F>
bool input_character(std::string &buffer, char32_t unichar, F &&filter)
{
	auto const size(std::numeric_limits<std::string::size_type>::max());
	return input_character(buffer, size, unichar, std::forward<F>(filter));
}


//-------------------------------------------------
//  paste_text - paste text from clipboard into a
//  buffer, ignoring invalid characters
//-------------------------------------------------

template <typename F>
bool paste_text(std::string &buffer, std::string::size_type size, F &&filter)
{
	std::string const clip(osd_get_clipboard_text());
	std::string_view text(clip);
	bool updated(false);
	int codelength;
	char32_t unichar;
	while ((codelength = uchar_from_utf8(&unichar, text)) != 0)
	{
		text.remove_prefix((0 < codelength) ? codelength : 1);
		if ((0 < codelength) && filter(unichar))
		{
			char utf8char[UTF8_CHAR_MAX];
			auto const utf8len(utf8_from_uchar(utf8char, std::size(utf8char), unichar));
			if (0 < utf8len)
			{
				if ((size < utf8len) || ((size - utf8len) < buffer.length()))
				{
					return updated;
				}
				else
				{
					buffer.append(utf8char, utf8len);
					updated = true;
				}
			}
		}
	}
	return updated;
}


//-------------------------------------------------
//  paste_text - paste text from clipboard into a
//  buffer, ignoring invalid characters
//-------------------------------------------------

template <typename F>
bool paste_text(std::string &buffer, F &&filter)
{
	auto const size(std::numeric_limits<std::string::size_type>::max());
	return paste_text(buffer, size, std::forward<F>(filter));
}

} // namespace ui

#endif // MAME_FRONTEND_UI_UTILS_H
