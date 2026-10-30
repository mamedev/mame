// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best
/*************************************************************************

RAM device

Provides a configurable amount of RAM to drivers

**************************************************************************/

#include "emu.h"
#include "ram.h"

#include "corestr.h"

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
		{ "mib",    1024 * 1024 },
		{ "g",      1024 * 1024 * 1024 },
		{ "gb",     1024 * 1024 * 1024 },
		{ "gib",    1024 * 1024 * 1024 }
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

};

device_ram_interface::device_ram_interface(const machine_config& mconfig, device_t& device)
	: device_interface(device, "ram")
{
}

device_ram_interface::~device_ram_interface()
{
}

DECLARE_DEVICE_TYPE(RAM_OPTION, device_ram_interface)

class ram_option_device :
	public device_t,
	public device_ram_interface
{
public:
	ram_option_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
		: device_t(mconfig, RAM_OPTION, tag, owner, clock)
		, device_ram_interface(mconfig, *this)
	{
	}

	virtual u32 size() const override { return parse_string(basetag()); }

protected:
	virtual void device_validity_check(validity_checker& valid) const override
	{
		if (!size())
			osd_printf_error("Invalid RAM option: %s\n", basetag());
	}

	virtual void device_start() override
	{
	}
};


ram_device::ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RAM, tag, owner, clock)
	, device_single_card_slot_interface<device_ram_interface>(mconfig, *this)
	, m_size(0)
	, m_bits(8)
	, m_default_value(0xff)
	, m_extra_options_string(nullptr)
{
}

void ram_device::device_validity_check(validity_checker& valid) const
{
	if (m_bits != 8 && m_bits != 16 && m_bits != 32 && m_bits != 64)
		osd_printf_error("Invalid RAM bits: %u\n", m_bits);
}

void ram_device::device_config_complete()
{
	device_ram_interface* device = get_card_device();

	m_size = device ? device->size() : parse_string(m_default_size.c_str());
}

void ram_device::device_start()
{
	allocate_ram();

	if (m_bits == 8)
		save_pointer(reinterpret_cast<u8 *>(m_pointer.get()), "m_pointer", m_size);
	else if (m_bits == 16)
		save_pointer(reinterpret_cast<u16 *>(m_pointer.get()), "m_pointer", m_size / 2);
	else if (m_bits == 32)
		save_pointer(reinterpret_cast<u32 *>(m_pointer.get()), "m_pointer", m_size / 4);
	else if (m_bits == 64)
		save_pointer(reinterpret_cast<u64 *>(m_pointer.get()), "m_pointer", m_size / 8);
}

std::string ram_device::sanitise_option(std::string_view option)
{
	u32 const ram_option = parse_string(option.data());

	if (!ram_option)
		return std::string(option);
	else if (ram_option % 1024)
		return string_format("%u", ram_option);
	else if (ram_option % (1024 * 1024))
		return string_format("%uk", ram_option / 1024);
	else if (ram_option % (1024 * 1024 * 1024))
		return string_format("%um", ram_option / (1024 * 1024));
	else
		return string_format("%ug", ram_option / (1024 * 1024 * 1024));
}

void ram_device::update_options()
{
	option_reset();

	if (m_default_size.length())
	{
		option_add(m_default_size, RAM_OPTION);
		set_default_option(m_default_size.c_str());
	}

	if (m_extra_options_string)
	{
		std::string const options(m_extra_options_string);

		bool done(false);
		for (std::string::size_type start = 0, end = options.find_first_of(','); !done; start = end + 1, end = options.find_first_of(',', start))
		{
			while ((end > start) && (options.length() > start) && ((' ' == options[start]) || ('\t' == options[start])))
				++start;

			std::string ram_option_string(sanitise_option(options.substr(start, (end == -1) ? -1 : end - start)));

			if (ram_option_string != m_default_size)
				option_add(ram_option_string, RAM_OPTION);
			done = std::string::npos == end;
		}
	}
}

void ram_device::allocate_ram()
{
	if (!m_pointer)
	{
		m_pointer.reset(std::malloc(m_size));
		if (!m_pointer)
			throw emu_fatalerror("%s: error allocating memory", tag());
		std::fill_n(reinterpret_cast<u8*>(m_pointer.get()), m_size, m_default_value);
	}
}

DEFINE_DEVICE_TYPE(RAM, ram_device, "ram_slot", "RAM Slot")
DEFINE_DEVICE_TYPE_PRIVATE(RAM_OPTION, device_ram_interface, ram_option_device, "ram", "RAM")
