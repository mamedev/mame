// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "sram.h"

/*
 * Generic ROM + SRAM (Sega style)
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_SRAM, megadrive_rom_sram_device, "megadrive_rom_sram", "Megadrive Sega ROM + SRAM cart")

megadrive_rom_sram_device::megadrive_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, type, tag, owner, clock)
	, m_sram_view(*this, "sram_view")
{
}

megadrive_rom_sram_device::megadrive_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_sram_device(mconfig, MEGADRIVE_ROM_SRAM, tag, owner, clock)
{
}


std::error_condition megadrive_rom_sram_device::load()
{
	const auto err = megadrive_rom_device::load();
	if (err)
		return image_error::BADSOFTWARE;
	memory_region *const nvramregion(cart_sram_region());

	if (nvramregion)
	{
		m_nvram_base = reinterpret_cast<u8 *>(nvramregion->base());
		m_nvram_size = nvramregion->bytes();
		m_nvram_mask = m_nvram_size - 1;

		if (m_nvram_size & m_nvram_mask)
			return image_error::BADSOFTWARE;

		save_pointer(NAME(m_nvram_base), m_nvram_size);
		battery_load(m_nvram_base, m_nvram_size, nullptr);
	}
	else
	{
		osd_printf_error("sram region not found\n");
		return image_error::BADSOFTWARE;
	}

	return std::error_condition();
}


void megadrive_rom_sram_device::unload()
{
	memory_region *const nvramregion(this->cart_sram_region());
	if (nvramregion && nvramregion->bytes())
		this->battery_save(nvramregion->base(), nvramregion->bytes());
}

void megadrive_rom_sram_device::device_start()
{
	megadrive_rom_device::device_start();
	memory_region *const nvramregion(cart_sram_region());
	// FIXME: initialize this thru load fn
	if (!nvramregion)
		throw emu_fatalerror("Missing SRAM region, cannot initialize\n");
	m_nvram_size = nvramregion->bytes();
	m_nvram_mask = m_nvram_size - 1;
	save_item(NAME(m_nvram_write_protect));
}

void megadrive_rom_sram_device::device_reset()
{
	megadrive_rom_device::device_reset();
	m_nvram_write_protect = false;
	m_sram_view.select(0);
}


void megadrive_rom_sram_device::cart_map(address_map &map)
{
	megadrive_rom_device::cart_map(map);
	// TODO: most if not all of them really uses 8-bit interface
	map(0x20'0000, 0x20'0001 | m_nvram_mask).view(m_sram_view);
	m_sram_view[0](0x20'0000, 0x20'0001 | m_nvram_mask).lrw8(
		NAME([this] (offs_t offset) {
			return m_nvram_base[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (!m_nvram_write_protect)
				m_nvram_base[offset] = data;
		})
	);
}

void megadrive_rom_sram_device::time_io_map(address_map &map)
{
	// TODO: does all Sega games have this?
	// there must be a threshold where this is a thing vs. where is not ...
	map(0xf1, 0xf1).lw8(NAME([this] (offs_t offset, u8 data) {
		if (BIT(data, 1))
			m_sram_view.disable();
		else
			m_sram_view.select(0);
		m_nvram_write_protect = !!BIT(data, 0);
	}));
}

/*
 * Sonic 3
 * Uses it's own (earlier?) version where bit 0 is the view select, swapped, and no write protect
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_SONIC3, megadrive_rom_sonic3_device, "megadrive_rom_sonic3", "Megadrive Sonic 3 ROM + SRAM cart")

megadrive_rom_sonic3_device::megadrive_rom_sonic3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_sram_device(mconfig, MEGADRIVE_ROM_SONIC3, tag, owner, clock)
{
}

void megadrive_rom_sonic3_device::time_io_map(address_map &map)
{
	map(0xf1, 0xf1).lw8(NAME([this] (offs_t offset, u8 data) {
		if (BIT(data, 0))
			m_sram_view.select(0);
		else
			m_sram_view.disable();
	}));
}

/*
 * Triple Play '96
 *
 * 8-bit NVRAM with NO write protection control lanes
 * At title screen hold A+B+C then press start, SFX plays, release all to get prompted for NVRAM
 * reinitialize.
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_TPLAY96, megadrive_rom_tplay96_device, "megadrive_rom_tplay96", "Megadrive Triple Play '96 cart")

megadrive_rom_tplay96_device::megadrive_rom_tplay96_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, type, tag, owner, clock)
	, m_nvram(*this, "nvram")
{
}

megadrive_rom_tplay96_device::megadrive_rom_tplay96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_tplay96_device(mconfig, MEGADRIVE_ROM_TPLAY96, tag, owner, clock)
{
}


void megadrive_rom_tplay96_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

u16 megadrive_rom_tplay96_device::get_nvram_length()
{
	return 0x8000;
}

void megadrive_rom_tplay96_device::device_start()
{
	megadrive_rom_device::device_start();
	const u32 nvram_size = get_nvram_length();
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
	m_nvram_mask = nvram_size - 1;
}

u16 megadrive_rom_tplay96_device::nvram_r(offs_t offset)
{
	const u32 nvram_offset = offset & m_nvram_mask;
	return 0xff00 | m_nvram_ptr[nvram_offset];
}

void megadrive_rom_tplay96_device::nvram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		const u32 nvram_offset = offset & m_nvram_mask;
		m_nvram_ptr[nvram_offset] = data & 0xff;
	}
}

void megadrive_rom_tplay96_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x20'0000, 0x20'ffff).rw(FUNC(megadrive_rom_tplay96_device::nvram_r), FUNC(megadrive_rom_tplay96_device::nvram_w));
}

/*
 * Hardball '95
 * Relocated NVRAM
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_HARDBALL95, megadrive_rom_hardball95_device, "megadrive_rom_hardball95", "Megadrive Hardball '95 cart")

megadrive_rom_hardball95_device::megadrive_rom_hardball95_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_tplay96_device(mconfig, MEGADRIVE_ROM_HARDBALL95, tag, owner, clock)
{
}

void megadrive_rom_hardball95_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x30'0000, 0x30'ffff).rw(FUNC(megadrive_rom_hardball95_device::nvram_r), FUNC(megadrive_rom_hardball95_device::nvram_w));
}

/*
 * Barkley Shut Up and Jam 2
 * header indicates $20'0001 - $20'0fff as SRAM but in-game it uses a mirror at $23'xxxx
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_BARKLEY2, megadrive_rom_barkley2_device, "megadrive_rom_barkley2", "Megadrive Barkley Shut Up and Jam 2 cart")

megadrive_rom_barkley2_device::megadrive_rom_barkley2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_tplay96_device(mconfig, MEGADRIVE_ROM_BARKLEY2, tag, owner, clock)
{
}

u16 megadrive_rom_barkley2_device::get_nvram_length()
{
	return 0x800;
}

void megadrive_rom_barkley2_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).bankr(m_rom);
	map(0x20'0000, 0x3f'ffff).rw(FUNC(megadrive_rom_barkley2_device::nvram_r), FUNC(megadrive_rom_barkley2_device::nvram_w));
}



/*
 * San Guo Zhi V / Tun Shi Tian Di III
 *
 * sanguo5 https://segaretro.org/San_Guo_Zhi_V
 * tunshi / tunshi1 https://segaretro.org/Tun_Shi_Tian_Di_III
 *
 * SKOB published games with invalid header and SRAM at $20'0000
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_UNL_SANGUO5, megadrive_unl_sanguo5_device, "megadrive_unl_sanguo5", "Megadrive San Guo Zhi V cart")


megadrive_unl_sanguo5_device::megadrive_unl_sanguo5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, MEGADRIVE_UNL_SANGUO5, tag, owner, clock)
	, m_nvram(*this, "nvram")
{
}

void megadrive_unl_sanguo5_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void megadrive_unl_sanguo5_device::device_start()
{
	megadrive_rom_device::device_start();
	const u32 nvram_size = 0x2000;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);

	save_pointer(NAME(m_nvram_ptr), nvram_size);
}

u16 megadrive_unl_sanguo5_device::nvram_r(offs_t offset)
{
	const u32 nvram_offset = offset & 0x1fff;
	return 0xff00 | m_nvram_ptr[nvram_offset];
}

void megadrive_unl_sanguo5_device::nvram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		const u32 nvram_offset = offset & 0x1fff;
		m_nvram_ptr[nvram_offset] = data & 0xff;
	}
}

void megadrive_unl_sanguo5_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).bankr(m_rom);
	map(0x20'0000, 0x20'3fff).rw(FUNC(megadrive_unl_sanguo5_device::nvram_r), FUNC(megadrive_unl_sanguo5_device::nvram_w));
}


