// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Shared Game Boy cartridge helpers

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_CARTBASE_IPP
#define MAME_BUS_GAMEBOY_CARTBASE_IPP

#pragma once

#include "cartbase.h"

#include "bus/generic/slot.h"


namespace bus::gameboy {

//**************************************************************************
//  8 KiB flat RAM helper
//**************************************************************************

template <typename Base>
void flat_ram_device_base<Base>::unload()
{
	memory_region *const nvramregion(this->cart_nvram_region());
	if (nvramregion && nvramregion->bytes())
		this->battery_save(nvramregion->base(), nvramregion->bytes());
}


template <typename Base>
bool flat_ram_device_base<Base>::check_ram(std::string &message)
{
	memory_region *const ramregion(this->cart_ram_region());
	memory_region *const nvramregion(this->cart_nvram_region());
	auto const rambytes(ramregion ? ramregion->bytes() : 0);
	auto const nvrambytes(nvramregion ? nvramregion->bytes() : 0);
	if (rambytes && nvrambytes && ((rambytes > nvrambytes) || ((nvrambytes - 1) & nvrambytes)))
	{
		message = "Unsupported cartridge RAM size (if not all RAM is battery-backed, battery backed RAM size must be a power of two, and at least half the total RAM must be battery-backed)";
		return false;
	}

	auto const ramtotal(rambytes + nvrambytes);
	if (0x2000 < ramtotal)
	{
		message = "Unsupported cartridge RAM size (must be no larger than 8 KiB)";
		return false;
	}

	return true;
}


template <typename Base>
void flat_ram_device_base<Base>::install_ram()
{
	memory_region *const ramregion(this->cart_ram_region());
	memory_region *const nvramregion(this->cart_nvram_region());
	auto const rambytes(ramregion ? ramregion->bytes() : 0);
	auto const nvrambytes(nvramregion ? nvramregion->bytes() : 0);

	// assume battery-backed RAM is mapped low if present
	if (nvrambytes)
	{
		if (0x2000 < nvrambytes)
		{
			osd_printf_warning(
					"[%s] Cartridge RAM banking is unsupported - only 0x2000 bytes of 0x%X bytes will be accessible\n",
					this->tag(),
					nvrambytes);
		}
		u8 *const nvrambase(&nvramregion->as_u8());
		device_generic_cart_interface::install_non_power_of_two<0>(
				nvrambytes,
				0x1fff,
				0,
				0,
				0xa000,
				[this, nvrambase] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
				{
					this->logerror(
							"Install NVRAM 0x%04X-0x%04X at 0x%04X-0x%04X mirror 0x%04X\n",
							src,
							src + (end - begin),
							begin,
							end,
							mirror);
					this->cart_space()->install_ram(begin, end, mirror, &nvrambase[src]);
				});
		this->save_pointer(NAME(nvrambase), nvrambytes);
		this->battery_load(nvrambase, nvrambytes, nullptr);
	}

	// install the rest of the RAM if possible
	if (rambytes)
	{
		u8 *const rambase(&ramregion->as_u8());
		if (!nvrambytes)
		{
			if (0x2000 < rambytes)
			{
				osd_printf_warning(
						"[%s] Cartridge RAM banking is unsupported - only 0x2000 bytes of 0x%X bytes will be accessible\n",
						this->tag(),
						rambytes);
			}
			device_generic_cart_interface::install_non_power_of_two<0>(
					rambytes,
					0x1fff,
					0,
					0,
					0xa000,
					[this, rambase] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
					{
						this->logerror(
								"Install RAM 0x%04X-0x%04X at 0x%04X-0x%04X mirror 0x%04X\n",
								src,
								src + (end - begin),
								begin,
								end,
								mirror);
						this->cart_space()->install_ram(begin, end, mirror, &rambase[src]);
					});
		}
		else if ((0x2000 > nvrambytes) && !((nvrambytes - 1) & nvrambytes) && (nvrambytes >= rambytes))
		{
			device_generic_cart_interface::install_non_power_of_two<0>(
					rambytes,
					nvrambytes - 1,
					0,
					0,
					0xa000 | nvrambytes,
					[this, rambase, highmirror = 0x1fff & ~(nvrambytes | (nvrambytes - 1))] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
					{
						this->logerror(
								"Install RAM 0x%04X-0x%04X at 0x%04X-0x%04X mirror 0x%04X\n",
								src,
								src + (end - begin),
								begin,
								end,
								highmirror | mirror);
						this->cart_space()->install_ram(begin, end, highmirror | mirror, &rambase[src]);
					});
		}
		else
		{
			osd_printf_warning(
					"[%s] Cannot install 0x%X bytes of RAM in addition to 0x%X bytes of battery-backed RAM\n",
					this->tag(),
					rambytes,
					nvrambytes);
		}
		this->save_pointer(NAME(rambase), rambytes);
	}
}



//**************************************************************************
//  8 KiB switchable RAM helper
//**************************************************************************

template <typename Base>
void mbc_ram_device_base<Base>::unload()
{
	memory_region *const nvramregion(this->cart_nvram_region());
	if (nvramregion && nvramregion->bytes())
		this->battery_save(nvramregion->base(), nvramregion->bytes());
}


template <typename Base>
mbc_ram_device_base<Base>::mbc_ram_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	Base(mconfig, type, tag, owner, clock),
	m_bank_ram(*this, "ram"),
	m_bank_bits_ram(0U),
	m_bank_mask_ram(0U),
	m_bank_sel_ram(0U),
	m_bank_ram_mask(0xffU)
{
}


template <typename Base>
bool mbc_ram_device_base<Base>::check_ram(std::string &message)
{
	memory_region *const ramregion(this->cart_ram_region());
	memory_region *const nvramregion(this->cart_nvram_region());
	auto const rambytes(ramregion ? ramregion->bytes() : 0);
	auto const nvrambytes(nvramregion ? nvramregion->bytes() : 0);
	if (rambytes && nvrambytes && ((rambytes & (PAGE_RAM_SIZE - 1)) || (nvrambytes & (PAGE_RAM_SIZE - 1)) || ((nvrambytes - 1) & nvrambytes)))
	{
		message = "Unsupported cartridge RAM size (if not all RAM is battery-backed, battery backed RAM size must be a power of two no smaller than 8 KiB, and total RAM size must be a multiple of 8 KiB)";
		return false;
	}

	auto const ramtotal(rambytes + nvrambytes);
	if (((PAGE_RAM_SIZE < ramtotal) && (ramtotal & (PAGE_RAM_SIZE - 1))) || ((u32(PAGE_RAM_SIZE) << m_bank_bits_ram) < ramtotal))
	{
		message = util::string_format(
				"Unsupported cartridge RAM size (must be no larger than 8 KiB, or a multiple of 8 KiB no larger than %u pages)",
				1U << m_bank_bits_ram);
		return false;
	}

	return true;
}


template <typename Base>
void mbc_ram_device_base<Base>::install_ram(address_space_installer &space)
{
	install_ram(nullptr, &space);
}


template <typename Base>
void mbc_ram_device_base<Base>::install_ram(
		address_space_installer &rospace,
		address_space_installer &rwspace)
{
	install_ram(&rospace, &rwspace);
}


template <typename Base>
bool mbc_ram_device_base<Base>::configure_bank_ram(std::string &message)
{
	memory_region *const ramregion(this->cart_ram_region());
	memory_region *const nvramregion(this->cart_nvram_region());
	auto const rambytes(ramregion ? ramregion->bytes() : 0);
	auto const nvrambytes(nvramregion ? nvramregion->bytes() : 0);
	auto const ramtotal(rambytes + nvrambytes);
	if ((rambytes & (PAGE_RAM_SIZE - 1)) || (nvrambytes & (PAGE_RAM_SIZE - 1)) || ((u32(PAGE_RAM_SIZE) << m_bank_bits_ram) < ramtotal))
	{
		message = util::string_format(
				"Unsupported cartridge RAM size (battery-backed RAM and additional RAM must be multiples of 8 KiB with a total size no larger than %u pages)",
				1U << m_bank_bits_ram);
		return false;
	}
	else if (rambytes && nvrambytes && ((nvrambytes - 1) & nvrambytes))
	{
		message = "Unsupported cartridge RAM size (if not all RAM is battery-backed, battery backed RAM size must be a power of two no smaller than 8 KiB, and total RAM size must be a multiple of 8 KiB)";
		return false;
	}


	if (!ramtotal)
	{
		// need to avoid fatal errors
		m_bank_mask_ram = 0U;
	}
	else
	{
		// configure banked RAM, assuming lower pages are battery-backed
		u8 *const rambase(rambytes ? &ramregion->as_u8() : nullptr);
		u8 *const nvrambase(nvrambytes ? &nvramregion->as_u8() : nullptr);
		m_bank_mask_ram = device_generic_cart_interface::map_non_power_of_two(
				unsigned(ramtotal / PAGE_RAM_SIZE),
				[this, nvrampages = nvrambytes / PAGE_RAM_SIZE, rambase, nvrambase] (unsigned entry, unsigned page)
				{
					bool const nonvolatile(page < nvrampages);
					u8 *const base(nonvolatile ? nvrambase : rambase);
					auto const offset((page - (nonvolatile ? 0 : nvrampages)) * PAGE_RAM_SIZE);
					this->logerror(
							"Install %s 0x%05X-0x%05X in bank entry %u\n",
							nonvolatile ? "NVRAM" : "RAM",
							offset,
							offset + (PAGE_RAM_SIZE - 1),
							entry);
					m_bank_ram->configure_entry(entry, &base[offset]);
				});

		if (nvrambytes)
		{
			this->save_pointer(NAME(nvrambase), nvrambytes);
			this->battery_load(nvrambase, nvrambytes, nullptr);
		}
		if (rambytes)
			this->save_pointer(NAME(rambase), rambytes);
	}

	return true;
}


template <typename Base>
void mbc_ram_device_base<Base>::device_start()
{
	Base::device_start();

	this->save_item(NAME(m_bank_sel_ram));
	this->save_item(NAME(m_bank_mask_ram));
}


template <typename Base>
void mbc_ram_device_base<Base>::set_bank_ram(u8 entry)
{
	m_bank_sel_ram = entry;
	if (m_bank_mask_ram)
	{
		entry = bank_ram_entry();
		//this->logerror("%s: Set RAM bank = 0x%05X\n", machine().describe_context(), entry * PAGE_RAM_SIZE);
		m_bank_ram->set_entry(entry);
	}
}


template <typename Base>
void mbc_ram_device_base<Base>::set_bank_ram_mask(u8 mask)
{
	m_bank_ram_mask = mask;
	if (m_bank_mask_ram)
	{
		u8 const entry(bank_ram_entry());
		//this->logerror("%s: Set RAM bank = 0x%05X\n", machine().describe_context(), entry * PAGE_RAM_SIZE);
		m_bank_ram->set_entry(entry);
	}
}


template <typename Base>
void mbc_ram_device_base<Base>::install_ram(
		address_space_installer *rospace,
		address_space_installer *rwspace)
{
	memory_region *const ramregion(this->cart_ram_region());
	memory_region *const nvramregion(this->cart_nvram_region());
	auto const rambytes(ramregion ? ramregion->bytes() : 0);
	auto const nvrambytes(nvramregion ? nvramregion->bytes() : 0);
	auto const ramtotal(rambytes + nvrambytes);
	u8 *const rambase(rambytes ? &ramregion->as_u8() : nullptr);
	u8 *const nvrambase(nvrambytes ? &nvramregion->as_u8() : nullptr);

	if (!ramtotal)
	{
		// need to avoid fatal errors
		m_bank_mask_ram = 0U;
	}
	else if (PAGE_RAM_SIZE >= ramtotal)
	{
		// install small amounts of RAM directly - mixed volatile and non-volatile RAM is not supported
		device_generic_cart_interface::install_non_power_of_two<0>(
				nvrambytes ? nvrambytes : rambytes,
				PAGE_RAM_SIZE - 1,
				0,
				0,
				0xa000,
				[this, rospace, rwspace, base = nvrambase ? nvrambase : rambase] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
				{
					this->logerror(
							"Install RAM 0x%05X-0x%05X at 0x%04X-0x%04X mirror 0x%04X\n",
							src,
							src + (end - begin),
							begin,
							end,
							mirror);
					if (rospace)
						rospace->install_rom(begin, end, mirror, &base[src]);
					if (rwspace)
						rwspace->install_ram(begin, end, mirror, &base[src]);
				});
		m_bank_mask_ram = 0U;
	}
	else
	{
		// install banked RAM, assuming lower pages are battery-backed
		m_bank_mask_ram = device_generic_cart_interface::map_non_power_of_two(
				unsigned(ramtotal / PAGE_RAM_SIZE),
				[this, nvrampages = nvrambytes / PAGE_RAM_SIZE, rambase, nvrambase] (unsigned entry, unsigned page)
				{
					bool const nonvolatile(page < nvrampages);
					u8 *const base(nonvolatile ? nvrambase : rambase);
					auto const offset((page - (nonvolatile ? 0 : nvrampages)) * PAGE_RAM_SIZE);
					this->logerror(
							"Install %s 0x%05X-0x%05X in bank entry %u\n",
							nonvolatile ? "NVRAM" : "RAM",
							offset,
							offset + (PAGE_RAM_SIZE - 1),
							entry);
					m_bank_ram->configure_entry(entry, &base[offset]);
				});
		this->logerror("Installing banked RAM at 0xA000-0xBFFF\n");
		if (rospace)
			rospace->install_read_bank(0xa000, 0xbfff, m_bank_ram);
		if (rwspace)
			rwspace->install_readwrite_bank(0xa000, 0xbfff, m_bank_ram);
	}

	if (nvrambytes)
	{
		this->save_pointer(NAME(nvrambase), nvrambytes);
		this->battery_load(nvrambase, nvrambytes, nullptr);
	}
	if (rambytes)
		this->save_pointer(NAME(rambase), rambytes);
}

} // namespace bus::gameboy

#endif // MAME_BUS_GAMEBOY_CARTBASE_IPP
