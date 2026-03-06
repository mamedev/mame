// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best
/*************************************************************************

RAM device

Provides a configurable amount of RAM to drivers

TODO:
- add RAM size options to UI, eg. under Machine Configuration
- remove limitations due to hardcoded RAM_TAG:
  + *configurable* RAM device can only be added to root device
    (that is the driver device)
  + can only have one *configurable* RAM device per machine driver

**************************************************************************/

#include "emu.h"
#include "ram.h"

#include "corestr.h"
#include "emuopts.h"

#include <cstdio>
#include <cctype>

#include <algorithm>
#include <functional>
#include <set>


namespace {

//-------------------------------------------------
//  parse_string - convert a ram string to an
//  integer value
//-------------------------------------------------

u32 parse_string(const char *s)
{
	static constexpr struct
	{
		const char *suffix;
		unsigned multiple;
	} s_suffixes[] =
	{
		{ "",       1 },
		{ "k",      1024 },
		{ "kb",     1024 },
		{ "kib",    1024 },
		{ "m",      1024 * 1024 },
		{ "mb",     1024 * 1024 },
		{ "mib",    1024 * 1024 }
	};

	// parse the string
	unsigned ram = 0;
	char suffix[8] = { 0, };
	sscanf(s, "%u%7s", &ram, suffix);

	// perform the lookup
	auto const iter(std::find_if(
			std::begin(s_suffixes),
			std::end(s_suffixes),
			[&suffix](const auto &potential_suffix) { return !core_stricmp(suffix, potential_suffix.suffix); }));

	// identify the multiplier (or 0 if not recognized, signalling a parse failure)
	unsigned const multiplier((iter != std::end(s_suffixes)) ? iter->multiple : 0);

	// return the result
	return ram * multiplier;
}


//-------------------------------------------------
//  calculate_extra_options
//-------------------------------------------------

ram_device::extra_option_vector calculate_extra_options(const char *extra_options_string, std::string *bad_option)
{
	ram_device::extra_option_vector result;
	std::string const options(extra_options_string);

	bool done(false);
	for (std::string::size_type start = 0, end = options.find_first_of(','); !done; start = end + 1, end = options.find_first_of(',', start))
	{
		// ignore spaces
		while ((end > start) && (options.length() > start) && ((' ' == options[start]) || ('\t' == options[start])))
			++start;

		// parse the option
		std::string ram_option_string(options.substr(start, (end == -1) ? -1 : end - start));
		u32 const ram_option = parse_string(ram_option_string.c_str());
		if (ram_option == 0)
		{
			if (bad_option)
				*bad_option = std::move(ram_option_string);
			return result;
		}

		// and add it to the results
		result.emplace_back(std::move(ram_option_string), ram_option);
		done = std::string::npos == end;
	}
	return result;
}

};


/*****************************************************************************
    LIVE DEVICE
*****************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(RAM, ram_device, "ram", "RAM")


//-------------------------------------------------
//  ram_device - constructor
//-------------------------------------------------

ram_device::ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RAM, tag, owner, clock)
	, m_size(0)
	, m_default_size(0)
	, m_default_value(0xff)
	, m_extra_options_string(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ram_device::device_start()
{
	// the device named 'ram' can get ram options from command line
	u32 const defsize(default_size());
	m_size = 0;
	if (strcmp(tag(), ":" RAM_TAG) == 0)
	{
		char const *const ramsize_string(machine().options().ram_size());
		if (ramsize_string && *ramsize_string)
		{
			m_size = parse_string(ramsize_string);
			if (!is_valid_size(m_size))
			{
				extra_option_vector::const_iterator found(std::find_if(m_extra_options.begin(), m_extra_options.end(), [defsize] (extra_option const &opt) { return opt.second == defsize; }));
				std::ostringstream output;
				util::stream_format(output, "Cannot recognize the RAM option %s (valid options are ", ramsize_string);
				if (!m_extra_options_string)
					util::stream_format(output, "%s).\n", m_default_size);
				else if (m_extra_options.end() != found)
					util::stream_format(output, "%s).\n", m_extra_options_string);
				else
					util::stream_format(output, "%s,%s).\n", m_default_size, m_extra_options_string);

				osd_printf_error("%s", output.str());

				osd_printf_warning("Setting value to default %s\n", m_default_size);

				m_size = 0;
			}
		}
	}

	// if we didn't get a size yet, use the default
	if (m_size == 0)
		m_size = defsize;

	// allocate space for the ram
	m_pointer.reset(std::malloc(m_size));
	if (!m_pointer)
		throw emu_fatalerror("%s: error allocating memory", tag());
	std::fill_n(reinterpret_cast<u8 *>(m_pointer.get()), m_size, m_default_value);

	// register for state saving
	save_item(NAME(m_size));
	save_pointer(reinterpret_cast<u8 *>(m_pointer.get()), "m_pointer", m_size);
}


//-------------------------------------------------
//  device_validity_check - device-specific validity
//  checks
//-------------------------------------------------

void ram_device::device_validity_check(validity_checker &valid) const
{
	// verify default ram value
	if (default_size() == 0)
		osd_printf_error("Invalid default RAM option: %s\n", m_default_size);

	// calculate any extra options
	if (m_extra_options_string)
	{
		std::string bad_option;
		extra_option_vector const extras(calculate_extra_options(m_extra_options_string, &bad_option));

		// report any errors
		if (!bad_option.empty())
			osd_printf_error("Invalid RAM option: %s\n", bad_option);

		// report duplicates
		using extra_option_ref_set = std::set<std::reference_wrapper<extra_option const>, bool (*)(extra_option const &, extra_option const &)>;
		extra_option_ref_set sorted([] (extra_option const &a, extra_option const &b) { return a.second < b.second; });
		for (extra_option const &opt : extras)
		{
			auto const ins(sorted.emplace(opt));
			if (!ins.second)
				osd_printf_error("Duplicate RAM options: %s == %s (%u)\n", ins.first->get().first, opt.first, opt.second);
		}
	}
}


//-------------------------------------------------
//  is_valid_size
//-------------------------------------------------

bool ram_device::is_valid_size(u32 size) const
{
	return size == default_size()
		|| std::find_if(extra_options().begin(), extra_options().end(), [size] (extra_option const &opt) { return opt.second == size; }) != extra_options().end();
}


//-------------------------------------------------
//  default_size
//-------------------------------------------------

u32 ram_device::default_size() const
{
	return parse_string(m_default_size);
}


//-------------------------------------------------
//  extra_options
//-------------------------------------------------

const ram_device::extra_option_vector &ram_device::extra_options() const
{
	if (m_extra_options_string && m_extra_options.empty())
		m_extra_options = calculate_extra_options(m_extra_options_string, nullptr);
	return m_extra_options;
}
