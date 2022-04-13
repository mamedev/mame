// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************
 Virtual Boy cartridges
 ***************************************************************************/
#include "emu.h"
#include "rom.h"

#include "bus/generic/slot.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VBOY_FLAT_ROM,      vboy_flat_rom_device,      "vboy_flatrom",      "Nintendo Virtual Boy Flat ROM Cartridge")
DEFINE_DEVICE_TYPE(VBOY_FLAT_ROM_SRAM, vboy_flat_rom_sram_device, "vboy_flatrom_sram", "Nintendo Virtual Boy Flat ROM Cartridge with Backup SRAM")



//**************************************************************************
//  vboy_flat_rom_device
//**************************************************************************

vboy_flat_rom_device::vboy_flat_rom_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	vboy_flat_rom_device(mconfig, VBOY_FLAT_ROM, tag, owner, clock)
{
}


vboy_flat_rom_device::vboy_flat_rom_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vboy_cart_interface(mconfig, *this)
{
}


image_init_result vboy_flat_rom_device::load()
{
	// if the host has supplied a ROM space, install with appropriate mirroring
	memory_region *const romregion(memregion("^rom"));
	if (rom_space() && romregion)
	{
		// the slot should have verified that the ROM is a supported size
		assert(!(romregion->bytes() & 0x0000'0003U));
		assert(romregion->bytes() <= 0x0100'0000U);

		// this should fail a validity check
		assert(!(rom_base() & 0x00ff'ffff));

		// This is a simplification that improves performance.  In
		// reality, cartridge ROM is 16 bits wide and 32-bit accesses
		// are split up automatically.  MAME doesn't really support
		// dynamic bus sizing, and it's a lot faster to execute from a
		// ROM handler than to go through the trampolines necessary to
		// emulate 16-bit accesses.
		device_generic_cart_interface::install_non_power_of_two<2>(
				romregion->bytes() >> 2,
				0x00ff'ffff >> 2,
				0,
				0,
				rom_base(),
				[this, rom = reinterpret_cast<u32 *>(romregion->base())] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
				{
					LOG(
							"Install ROM 0x%08X-0x%08X at 0x%08X-0x%08X mirror %08X\n",
							src << 2,
							(src << 2) + (end - begin),
							begin,
							end,
							mirror);
					rom_space()->install_rom(begin, end, mirror, &rom[src]);
				});
	}

	return image_init_result::PASS;
}


void vboy_flat_rom_device::device_start()
{
}



//**************************************************************************
//  vboy_flat_rom_sram_device
//**************************************************************************

vboy_flat_rom_sram_device::vboy_flat_rom_sram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	vboy_flat_rom_device(mconfig, VBOY_FLAT_ROM_SRAM, tag, owner, clock)
{
}


image_init_result vboy_flat_rom_sram_device::load()
{
	image_init_result const result(vboy_flat_rom_device::load());
	if (image_init_result::PASS != result)
		return result;

	memory_region *const sramregion(memregion("^sram"));
	if (sramregion)
	{
		// this should fail a validity check
		assert(!(chip_base() & 0x00ff'ffff));

		switch (sramregion->bitwidth())
		{
		case 8:
			if (chip_space())
			{
				device_generic_cart_interface::install_non_power_of_two<2>(
						sramregion->bytes() >> 1,
						0x00ff'ffff >> 2,
						0,
						0,
						chip_base(),
						[this, sramregion] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
						{
							LOG(
									"Install SRAM 0x%08X-0x%08X at 0x%08X-0x%08X mirror %08X\n",
									src << 1,
									(src << 1) + ((end - begin) >> 1),
									begin,
									end,
									mirror);
							u8 *const base(&reinterpret_cast<u8 *>(sramregion->base())[src << 1]);
							chip_space()->install_readwrite_handler(
									begin,
									end,
									0,
									mirror,
									0,
									read8sm_delegate(*this, NAME([base] (offs_t offset) { return base[offset]; })),
									write8sm_delegate(*this, NAME([base] (offs_t offset, u8 data) { base[offset] = data; })),
									0x00ff'00ff);
						});
			}
			save_pointer(NAME(&sramregion->as_u8()), sramregion->bytes());
			break;

		case 16:
			if (chip_space())
			{
				device_generic_cart_interface::install_non_power_of_two<2>(
						sramregion->bytes() >> 2,
						0x00ff'ffff >> 2,
						0,
						0,
						chip_base(),
						[this, sramregion] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
						{
							LOG(
									"Install SRAM 0x%08X-0x%08X at 0x%08X-0x%08X mirror %08X\n",
									src << 2,
									(src << 2) + (end - begin),
									begin,
									end,
									mirror);
							u16 *const base(&reinterpret_cast<u16 *>(sramregion->base())[src << 1]);
							chip_space()->install_readwrite_handler(
									begin,
									end,
									0,
									mirror,
									0,
									read16s_delegate(*this, NAME([base] (offs_t offset, u16 mem_mask) { return base[offset]; })),
									write16s_delegate(*this, NAME([base] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(base + offset); })),
									0xffff'ffff);
						});
			}
			save_pointer(NAME(&sramregion->as_u16()), sramregion->bytes() >> 1);
			break;

		default:
			throw emu_fatalerror("Unsupported Virtual Boy cartridge backup RAM width\n");
		}

		battery_load(sramregion->base(), sramregion->bytes(), nullptr);
	}

	return image_init_result::PASS;
}


void vboy_flat_rom_sram_device::unload()
{
	vboy_flat_rom_device::unload();

	memory_region *const sramregion(memregion("^sram"));
	if (sramregion)
		battery_save(sramregion->base(), sramregion->bytes());
}
