// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Shared Game Boy cartridge helpers

 ***************************************************************************/

#include "emu.h"
#include "cartbase.h"

#include "bus/generic/slot.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"



namespace bus::gameboy {

//**************************************************************************
//  16 KiB fixed at 0x0000, 16 KiB switchable at 0x4000
//**************************************************************************

mbc_device_base::mbc_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gb_cart_interface(mconfig, *this),
	m_bank_rom(*this, "rom"),
	m_bank_bits_rom(0U),
	m_bank_mask_rom(0U)
{
}


void mbc_device_base::device_start()
{
}


bool mbc_device_base::check_rom(std::string &message)
{
	memory_region *const romregion(cart_rom_region());
	if (!romregion || !romregion->bytes())
	{
		// setting default bank on reset causes a fatal error when no banks are configured
		message = "No ROM data found";
		return false;
	}

	auto const rombytes(romregion->bytes());
	if ((rombytes & (PAGE_ROM_SIZE - 1)) || ((u32(PAGE_ROM_SIZE) << m_bank_bits_rom) < rombytes))
	{
		message = util::string_format(
				"Unsupported cartridge ROM size (must be a multiple of 16 KiB and no larger than %u pages)",
				1U << m_bank_bits_rom);
		return false;
	}

	return true;
}


void mbc_device_base::install_rom()
{
	configure_bank_rom();

	logerror("Installing fixed ROM at 0x0000-0x3FFF and banked ROM at 0x4000-0x7FFF\n");
	cart_space()->install_rom(0x0000, 0x3fff, cart_rom_region()->base());
	cart_space()->install_read_bank(0x4000, 0x7fff, m_bank_rom);
}


void mbc_device_base::configure_bank_rom()
{
	memory_region *const romregion(cart_rom_region());
	m_bank_mask_rom = device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / PAGE_ROM_SIZE),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				LOG(
						"Install ROM 0x%06X-0x%06X in bank entry %u\n",
						page * PAGE_ROM_SIZE,
						(page * PAGE_ROM_SIZE) + (PAGE_ROM_SIZE - 1),
						entry);
				m_bank_rom->configure_entry(entry, &base[page * PAGE_ROM_SIZE]);
			});
}


void mbc_device_base::set_bank_rom(u16 entry)
{
	entry &= m_bank_mask_rom;
	LOG(
			"%s: Set ROM bank = 0x%06X\n",
			machine().describe_context(),
			entry * PAGE_ROM_SIZE);
	m_bank_rom->set_entry(entry);
}



//**************************************************************************
//  16 KiB switchable coarse at 0x0000, 16 KiB switchable fine at 0x4000
//**************************************************************************

mbc_dual_device_base::mbc_dual_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gb_cart_interface(mconfig, *this),
	m_bank_rom(*this, { "low", "high" }),
	m_bank_bits_rom{ 0U, 0U },
	m_bank_mask_rom{ 0U, 0U },
	m_bank_sel_rom{ 0U, 0U },
	m_bank_rom_coarse_mask{ 0xffffU, 0xffffU }
{
}


void mbc_dual_device_base::device_start()
{
	save_item(NAME(m_bank_sel_rom));
	save_item(NAME(m_bank_rom_coarse_mask));
}


bool mbc_dual_device_base::check_rom(std::string &message)
{
	memory_region *const romregion(cart_rom_region());
	if (!romregion || !romregion->bytes())
	{
		// setting default bank on reset causes a fatal error when no banks are configured
		message = "No ROM data found";
		return false;
	}

	auto const rombytes(romregion->bytes());
	auto const pages(u32(1) << (m_bank_bits_rom[0] + m_bank_bits_rom[1]));
	if ((rombytes & (PAGE_ROM_SIZE - 1)) || ((PAGE_ROM_SIZE * pages) < rombytes))
	{
		message = util::string_format(
				"Unsupported cartridge ROM size (must be a multiple of 16 KiB and no larger than %u pages)",
				pages);
		return false;
	}

	return true;
}


void mbc_dual_device_base::install_rom()
{
	configure_bank_rom();

	logerror("Installing banked ROM at 0x0000-0x3FFF and 0x4000-0x7FFF\n");
	cart_space()->install_read_bank(0x0000, 0x3fff, m_bank_rom[0]);
	cart_space()->install_read_bank(0x4000, 0x7fff, m_bank_rom[1]);
}


void mbc_dual_device_base::configure_bank_rom()
{
	memory_region *const romregion(cart_rom_region());
	auto const rombytes(romregion->bytes());
	u8 *const rombase(&romregion->as_u8());
	auto const multiplier(1U << m_bank_bits_rom[1]);

	m_bank_mask_rom[0] = device_generic_cart_interface::map_non_power_of_two(
			unsigned(((rombytes / PAGE_ROM_SIZE) + multiplier - 1) / multiplier),
			[this, rombase, multiplier] (unsigned entry, unsigned page)
			{
				LOG(
						"Install ROM 0x%06X-0x%06X in low bank entry %u\n",
						page * PAGE_ROM_SIZE * multiplier,
						(page * PAGE_ROM_SIZE * multiplier) + (PAGE_ROM_SIZE - 1),
						entry);
				m_bank_rom[0]->configure_entry(entry, &rombase[page * PAGE_ROM_SIZE * multiplier]);
			});

	m_bank_mask_rom[1] = device_generic_cart_interface::map_non_power_of_two(
			unsigned(rombytes / PAGE_ROM_SIZE),
			[this, rombase] (unsigned entry, unsigned page)
			{
				LOG(
						"Install ROM 0x%06X-0x%06X in high bank entry %u\n",
						page * PAGE_ROM_SIZE,
						(page * PAGE_ROM_SIZE) + (PAGE_ROM_SIZE - 1),
						entry);
				m_bank_rom[1]->configure_entry(entry, &rombase[page * PAGE_ROM_SIZE]);
			});
}


void mbc_dual_device_base::set_bank_rom_coarse(u16 entry)
{
	m_bank_sel_rom[0] = entry;
	u16 const lo(bank_rom_entry_low());
	u16 const hi(bank_rom_entry_high());
	LOG(
			"%s: Set ROM banks low = 0x%06X, high = 0x%06X\n",
			machine().describe_context(),
			(u32(lo) * PAGE_ROM_SIZE) << m_bank_bits_rom[1],
			u32(hi) * PAGE_ROM_SIZE);
	m_bank_rom[0]->set_entry(lo);
	m_bank_rom[1]->set_entry(hi);
}


void mbc_dual_device_base::set_bank_rom_fine(u16 entry)
{
	m_bank_sel_rom[1] = entry;
	u16 const hi(bank_rom_entry_high());
	LOG(
			"%s: Set ROM bank high = 0x%06X\n",
			machine().describe_context(),
			u32(hi) * PAGE_ROM_SIZE);
	m_bank_rom[1]->set_entry(hi);
}


void mbc_dual_device_base::set_bank_rom_low_coarse_mask(u16 mask)
{
	m_bank_rom_coarse_mask[0] = mask;
	u16 const lo(bank_rom_entry_low());
	LOG(
			"%s: Set ROM bank low = 0x%06X\n",
			machine().describe_context(),
			(u32(lo) * PAGE_ROM_SIZE) << m_bank_bits_rom[1]);
	m_bank_rom[0]->set_entry(lo);
}


void mbc_dual_device_base::set_bank_rom_high_coarse_mask(u16 mask)
{
	m_bank_rom_coarse_mask[1] = mask;
	u16 const hi(bank_rom_entry_high());
	LOG(
			"%s: Set ROM bank high = 0x%06X\n",
			machine().describe_context(),
			u32(hi) * PAGE_ROM_SIZE);
	m_bank_rom[1]->set_entry(hi);
}



//**************************************************************************
//  16 KiB switchable at 0x0000, 16 KiB switchable at 0x4000
//**************************************************************************

mbc_dual_uniform_device_base::mbc_dual_uniform_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gb_cart_interface(mconfig, *this),
	m_bank_rom(*this, { "low", "high" }),
	m_bank_bits_rom(0U),
	m_bank_mask_rom(0U)
{
}


void mbc_dual_uniform_device_base::device_start()
{
}


bool mbc_dual_uniform_device_base::check_rom(std::string &message)
{
	memory_region *const romregion(cart_rom_region());
	if (!romregion || !romregion->bytes())
	{
		// setting default bank on reset causes a fatal error when no banks are configured
		message = "No ROM data found";
		return false;
	}

	auto const rombytes(romregion->bytes());
	if ((rombytes & (PAGE_ROM_SIZE - 1)) || ((u32(PAGE_ROM_SIZE) << m_bank_bits_rom) < rombytes))
	{
		message = util::string_format(
				"Unsupported cartridge ROM size (must be a multiple of 16 KiB and no larger than %u pages)",
				1U << m_bank_bits_rom);
		return false;
	}

	return true;
}


void mbc_dual_uniform_device_base::install_rom()
{
	configure_bank_rom();

	logerror("Installing banked ROM at 0x0000-0x3FFF and 0x4000-0x7FFF\n");
	cart_space()->install_read_bank(0x0000, 0x3fff, m_bank_rom[0]);
	cart_space()->install_read_bank(0x4000, 0x7fff, m_bank_rom[1]);
}


void mbc_dual_uniform_device_base::configure_bank_rom()
{
	memory_region *const romregion(cart_rom_region());
	m_bank_mask_rom = device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / PAGE_ROM_SIZE),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				LOG(
						"Install ROM 0x%06X-0x%06X in bank entry %u\n",
						page * PAGE_ROM_SIZE,
						(page * PAGE_ROM_SIZE) + (PAGE_ROM_SIZE - 1),
						entry);
				m_bank_rom[0]->configure_entry(entry, &base[page * PAGE_ROM_SIZE]);
				m_bank_rom[1]->configure_entry(entry, &base[page * PAGE_ROM_SIZE]);
			});
}


void mbc_dual_uniform_device_base::set_bank_rom_low(u16 entry)
{
	entry &= m_bank_mask_rom;
	LOG(
			"%s: Set ROM bank low = 0x%06X\n",
			machine().describe_context(),
			entry * PAGE_ROM_SIZE);
	m_bank_rom[0]->set_entry(entry);
}


void mbc_dual_uniform_device_base::set_bank_rom_high(u16 entry)
{
	entry &= m_bank_mask_rom;
	LOG(
			"%s: Set ROM bank high = 0x%06X\n",
			machine().describe_context(),
			entry * PAGE_ROM_SIZE);
	m_bank_rom[1]->set_entry(entry);
}



//**************************************************************************
//  16 KiB fixed at 0x0000, 8 KiB switchable at 0x4000 and 0x6000
//**************************************************************************

mbc_8k_device_base::mbc_8k_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gb_cart_interface(mconfig, *this),
	m_bank_rom(*this, { "romlow", "romhigh" }),
	m_bank_bits_rom(0U),
	m_bank_mask_rom(0U)
{
}


void mbc_8k_device_base::device_start()
{
}


bool mbc_8k_device_base::check_rom(std::string &message)
{
	memory_region *const romregion(cart_rom_region());
	if (!romregion || !romregion->bytes())
	{
		// setting default bank on reset causes a fatal error when no banks are configured
		message = "No ROM data found";
		return false;
	}

	auto const rombytes(romregion->bytes());
	if ((rombytes & (PAGE_ROM_SIZE - 1)) || ((u32(PAGE_ROM_SIZE) << m_bank_bits_rom) < rombytes))
	{
		message = util::string_format(
				"Unsupported cartridge ROM size (must be a multiple of 8 KiB and no larger than %u pages)",
				1U << m_bank_bits_rom);
		return false;
	}

	return true;
}


void mbc_8k_device_base::install_rom(
		address_space_installer &fixedspace,
		address_space_installer &lowspace,
		address_space_installer &highspace)
{
	memory_region *const romregion(cart_rom_region());
	auto const rombytes(romregion->bytes());

	// install the fixed ROM, mirrored if it’s too small
	if (0x4000 > rombytes)
		fixedspace.install_rom(0x0000, 0x1fff, 0x2000, romregion->base());
	else
		fixedspace.install_rom(0x0000, 0x3fff, 0x0000, romregion->base());

	// configure both banks as views of the ROM
	m_bank_mask_rom = device_generic_cart_interface::map_non_power_of_two(
			unsigned(rombytes / PAGE_ROM_SIZE),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				LOG(
						"Install ROM 0x%06X-0x%06X in bank entry %u\n",
						page * PAGE_ROM_SIZE,
						(page * PAGE_ROM_SIZE) + (PAGE_ROM_SIZE - 1),
						entry);
				m_bank_rom[0]->configure_entry(entry, &base[page * PAGE_ROM_SIZE]);
				m_bank_rom[1]->configure_entry(entry, &base[page * PAGE_ROM_SIZE]);
			});
	lowspace.install_read_bank(0x4000, 0x5fff, m_bank_rom[0]);
	highspace.install_read_bank(0x6000, 0x7fff, m_bank_rom[1]);
}


void mbc_8k_device_base::set_bank_rom_low(u16 entry)
{
	entry &= m_bank_mask_rom;
	LOG(
			"%s: Set ROM bank low = 0x%06X\n",
			machine().describe_context(),
			entry * PAGE_ROM_SIZE);
	m_bank_rom[0]->set_entry(entry);
}


void mbc_8k_device_base::set_bank_rom_high(u16 entry)
{
	entry &= m_bank_mask_rom;
	LOG(
			"%s: Set ROM bank high = 0x%06X\n",
			machine().describe_context(),
			entry * PAGE_ROM_SIZE);
	m_bank_rom[1]->set_entry(entry);
}



//**************************************************************************
//  32 KiB switchable at 0x0000
//**************************************************************************

banked_32k_device_base::banked_32k_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gb_cart_interface(mconfig, *this),
	m_bank_rom(*this, "rom"),
	m_bank_bits_rom(0U),
	m_bank_mask_rom(0U)
{
}


void banked_32k_device_base::device_start()
{
}

bool banked_32k_device_base::check_rom(std::string &message)
{
	memory_region *const romregion(cart_rom_region());
	if (!romregion || !romregion->bytes())
	{
		// setting default bank on reset causes a fatal error when no banks are configured
		message = "No ROM data found";
		return false;
	}

	auto const rombytes(romregion->bytes());
	if ((rombytes & (PAGE_ROM_SIZE - 1)) || ((u32(PAGE_ROM_SIZE) << m_bank_bits_rom) < rombytes))
	{
		message = util::string_format(
				"Unsupported cartridge ROM size (must be a multiple of 32 KiB and no larger than %u pages)",
				1U << m_bank_bits_rom);
		return false;
	}

	return true;
}


void banked_32k_device_base::install_rom()
{
	configure_bank_rom();

	logerror("Installing banked ROM at 0x0000-0x7FFF\n");
	cart_space()->install_read_bank(0x0000, 0x7fff, m_bank_rom);
}


void banked_32k_device_base::configure_bank_rom()
{
	memory_region *const romregion(cart_rom_region());
	m_bank_mask_rom = device_generic_cart_interface::map_non_power_of_two(
			unsigned(romregion->bytes() / PAGE_ROM_SIZE),
			[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
			{
				LOG(
						"Install ROM 0x%06X-0x%06X in bank entry %u\n",
						page * PAGE_ROM_SIZE,
						(page * PAGE_ROM_SIZE) + (PAGE_ROM_SIZE - 1),
						entry);
				m_bank_rom->configure_entry(entry, &base[page * PAGE_ROM_SIZE]);
			});
}


void banked_32k_device_base::set_bank_rom(u16 entry)
{
	entry &= m_bank_mask_rom;
	LOG(
			"%s: Set ROM bank = 0x%06X\n",
			machine().describe_context(),
			entry * PAGE_ROM_SIZE);
	m_bank_rom->set_entry(entry);
}

} // namespace bus::gameboy
