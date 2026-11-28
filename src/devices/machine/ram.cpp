// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Dirk Best
/*************************************************************************

RAM device

Provides a configurable amount of RAM to drivers

**************************************************************************/

#include "emu.h"
#include "ram.h"

#include "emuopts.h"

#include "corestr.h"

#include <cstdio>
#include <cstdlib>
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
	, m_default_fill{ 0xff, 0xff, 1, false }
	, m_extra_options_string(nullptr)
{
}

ram_device &ram_device::set_default_fill(fill_pattern pattern, u32 run)
{
	switch (pattern)
	{
	case fill_pattern::ZERO:        m_default_fill = fill_spec{ 0x00, 0x00, run, false }; break;
	case fill_pattern::ONES:        m_default_fill = fill_spec{ 0xff, 0xff, run, false }; break;
	case fill_pattern::ALT_00FF:    m_default_fill = fill_spec{ 0x00, 0xff, run, false }; break;
	case fill_pattern::ALT_FF00:    m_default_fill = fill_spec{ 0xff, 0x00, run, false }; break;
	case fill_pattern::ALT_55AA:    m_default_fill = fill_spec{ 0x55, 0xaa, run, false }; break;
	case fill_pattern::ALT_AA55:    m_default_fill = fill_spec{ 0xaa, 0x55, run, false }; break;
	case fill_pattern::RANDOM:      m_default_fill = fill_spec{ 0x00, 0x00, run, true }; break;
	}
	return *this;
}

void ram_device::device_validity_check(validity_checker& valid) const
{
	if (m_bits != 8 && m_bits != 16 && m_bits != 32 && m_bits != 64)
		osd_printf_error("Invalid RAM bits: %u\n", m_bits);
	if (!m_default_fill.run)
		osd_printf_error("Invalid RAM fill run length: %u\n", m_default_fill.run);
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

		// the command line takes priority over the driver's choice
		fill_spec fill(m_default_fill);
		emu_options const &options(mconfig().options());
		if (options.has_slot_option(slot_name()))
		{
			std::string const &option(options.slot_option(slot_name()).argument());
			if (!option.empty() && !parse_fill_option(option, fill))
				throw emu_fatalerror("Invalid fill for RAM slot %s: %s (expected zero, ones, random, a hex byte such as 55, or two hex bytes with optional run length such as 00ff:64)\n", slot_name(), option);
		}

		fill_ram(fill, reinterpret_cast<u8 *>(m_pointer.get()));
	}
}

void ram_device::fill_ram(fill_spec const &fill, u8 *base)
{
	if (fill.random)
	{
		for (u32 i = 0; i < m_size; i++)
			base[i] = u8(machine().rand());
	}
	else if (fill.first == fill.second)
	{
		std::fill_n(base, m_size, fill.first);
	}
	else
	{
		bool use_second(false);
		for (u64 offset = 0; offset < m_size; offset += fill.run, use_second = !use_second)
			std::fill_n(base + offset, std::min<u64>(fill.run, m_size - offset), use_second ? fill.second : fill.first);
	}
}

bool ram_device::parse_fill_option(std::string_view option, fill_spec &fill)
{
	if (!core_stricmp(option, "zero"))
	{
		fill = fill_spec{ 0x00, 0x00, 1, false };
		return true;
	}
	if (!core_stricmp(option, "ones"))
	{
		fill = fill_spec{ 0xff, 0xff, 1, false };
		return true;
	}
	if (!core_stricmp(option, "random"))
	{
		fill = fill_spec{ 0x00, 0x00, 1, true };
		return true;
	}

	// otherwise it must be XX, XXYY or XXYY:run
	auto const colon(option.find(':'));
	std::string_view const bytes(option.substr(0, colon));
	if ((bytes.length() != 2) && (bytes.length() != 4))
		return false;
	if (!std::all_of(bytes.begin(), bytes.end(), [] (char c) { return std::isxdigit(u8(c)); }))
		return false;
	unsigned long const value(std::strtoul(std::string(bytes).c_str(), nullptr, 16));

	unsigned long run(1);
	if (colon != std::string_view::npos)
	{
		// limiting the digits keeps the value within range of a u32
		std::string_view const digits(option.substr(colon + 1));
		if ((bytes.length() != 4) || digits.empty() || (digits.length() > 9))
			return false;
		if (!std::all_of(digits.begin(), digits.end(), [] (char c) { return std::isdigit(u8(c)); }))
			return false;
		run = std::strtoul(std::string(digits).c_str(), nullptr, 10);
		if (!run)
			return false;
	}

	if (bytes.length() == 2)
		fill = fill_spec{ u8(value), u8(value), 1, false };
	else
		fill = fill_spec{ u8(value >> 8), u8(value), u32(run), false };
	return true;
}

DEFINE_DEVICE_TYPE(RAM, ram_device, "ram_slot", "RAM Slot")
DEFINE_DEVICE_TYPE_PRIVATE(RAM_OPTION, device_ram_interface, ram_option_device, "ram", "RAM")
