// license:BSD-3-Clause
// copyright-holders: Dirk Best
/*************************************************************************

    RAM device

    Provides a configurable amount of RAM to drivers

**************************************************************************/

#include "emu.h"
#include "ram.h"
#include "emuopts.h"

#include <stdio.h>
#include <ctype.h>
#include <algorithm>


/*****************************************************************************
    LIVE DEVICE
*****************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(RAM, ram_device, "ram", "RAM")


//-------------------------------------------------
//  ram_device - constructor
//-------------------------------------------------

ram_device::ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RAM, tag, owner, clock)
	, m_size(0)
	, m_default_size(0)
	, m_default_value(0xCD)
	, m_extra_options_string(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ram_device::device_start()
{
	// the device named 'ram' can get ram options from command line
	m_size = 0;
	if (strcmp(tag(), ":" RAM_TAG) == 0)
	{
		const char *ramsize_string = machine().options().ram_size();
		if (ramsize_string && *ramsize_string)
		{
			m_size = parse_string(ramsize_string);
			if (!is_valid_size(m_size))
			{
				std::ostringstream output;
				util::stream_format(output, "Cannot recognize the RAM option %s", ramsize_string);
				util::stream_format(output, " (valid options are %s", m_default_size);

				if (m_extra_options_string)
					util::stream_format(output, ",%s).\n", m_extra_options_string);
				else
					util::stream_format(output, ").\n");

				osd_printf_error("%s", output.str().c_str());

				osd_printf_warning("Setting value to default %s\n", m_default_size);

				m_size = 0;
			}
		}
	}

	// if we didn't get a size yet, use the default
	if (m_size == 0)
		m_size = default_size();

	// allocate space for the ram
	m_pointer.resize(m_size);
	memset(&m_pointer[0], m_default_value, m_size);

	// register for state saving
	save_item(NAME(m_size));
	save_item(NAME(m_pointer));
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
	std::vector<uint32_t> extra_options;
	std::string bad_option;
	if (m_extra_options_string)
		extra_options = calculate_extra_options(m_extra_options_string, &bad_option);

	// report any errors
	if (!bad_option.empty())
		osd_printf_error("Invalid RAM option: %s\n", bad_option.c_str());
}


//-------------------------------------------------
//  is_valid_size
//-------------------------------------------------

bool ram_device::is_valid_size(uint32_t size) const
{
	return size == default_size()
		|| std::find(extra_options().begin(), extra_options().end(), size) != extra_options().end();
}


//-------------------------------------------------
//  parse_string - convert a ram string to an
//  integer value
//-------------------------------------------------

uint32_t ram_device::parse_string(const char *s)
{
	uint32_t ram;
	char suffix = '\0';

	sscanf(s, "%u%c", &ram, &suffix);

	switch(tolower(suffix))
	{
		case 'k':
			/* kilobytes */
			ram *= 1024;
			break;

		case 'm':
			/* megabytes */
			ram *= 1024*1024;
			break;

		case '\0':
			/* no suffix */
			break;

		default:
			/* parse failure */
			ram = 0;
			break;
	}

	return ram;
}


//-------------------------------------------------
//  default_size
//-------------------------------------------------

uint32_t ram_device::default_size(void) const
{
	return parse_string(m_default_size);
}


//-------------------------------------------------
//  extra_options
//-------------------------------------------------

const std::vector<uint32_t> &ram_device::extra_options(void) const
{
	if (m_extra_options_string && m_extra_options.empty())
		m_extra_options = calculate_extra_options(m_extra_options_string, nullptr);
	return m_extra_options;
}


//-------------------------------------------------
//  calculate_extra_options
//-------------------------------------------------

std::vector<uint32_t> ram_device::calculate_extra_options(const char *extra_options_string, std::string *bad_option)
{
	std::vector<uint32_t> result;
	std::string options(extra_options_string);

	bool done = false;
	for (std::string::size_type start = 0, end = options.find_first_of(','); !done; start = end + 1, end = options.find_first_of(',', start))
	{
		// parse the option
		const std::string ram_option_string = options.substr(start, (end == -1) ? -1 : end - start);
		const uint32_t ram_option = parse_string(ram_option_string.c_str());
		if (ram_option == 0)
		{
			if (bad_option)
				*bad_option = std::move(ram_option_string);
			return result;
		}

		// and add it to the results
		result.push_back(ram_option);
		done = end == std::string::npos;
	}
	return result;
}
