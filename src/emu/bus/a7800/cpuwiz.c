// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A7800 CPUWIZ's homebrew boards (MegaCart+ and VersaBoard)

 Here we emulate the base configurations of these two boards:

 MegaCart+ = up to 512K (31 banks at $8000, 1 at $C000) of ROM and 2 x 16K RAM @ $4000
 VersaBoard = up to 256K of ROM and 2 x 16K RAM

 Plus, for the moment, a VersaBoard with POKEY mapped at 0x0450 and support for 144K ROM,
 since a few demo homebrew programs seems to use this to combine compatibility with
 XBoarD & XM expansions

 Note that the VersaBoard can be configured to work with different banking hardware
 e.g. with SG 9bank games or with SG + RAM (so to allow reproduction of games which
 could have worked on old carts without sacrifying original carts), but games running
 on those "standard" variants can be emulated with the standard code from rom.c ;-)


 TO DO:
 - investigate whether the POKEY detection routines in homebrew do fail due to emulation
   issues or not

***********************************************************************************************************/


#include "emu.h"
#include "cpuwiz.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type A78_ROM_VERSABOARD = &device_creator<a78_versaboard_device>;
const device_type A78_ROM_MEGACART = &device_creator<a78_megacart_device>;

const device_type A78_ROM_P450_VB = &device_creator<a78_rom_p450_vb_device>;


a78_versaboard_device::a78_versaboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: a78_rom_sg_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

a78_versaboard_device::a78_versaboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_VERSABOARD, "Atari 7800 VersaBoard Cart", tag, owner, clock, "a78_versaboard", __FILE__)
{
}


a78_megacart_device::a78_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_versaboard_device(mconfig, A78_ROM_MEGACART, "Atari 7800 MegaCart+", tag, owner, clock, "a78_megacart", __FILE__)
{
}


a78_rom_p450_vb_device::a78_rom_p450_vb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_versaboard_device(mconfig, A78_ROM_P450_VB, "Atari 7800 VersaBoard + POKEY @ 0x0450 Cart", tag, owner, clock, "a78_versapokey", __FILE__),
						m_pokey450(*this, "pokey450")
{
}


void a78_versaboard_device::device_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_ram_bank));
}

void a78_versaboard_device::device_reset()
{
	m_bank = 0;
	m_ram_bank = 0;
}


// VersaBoard

READ8_MEMBER(a78_versaboard_device::read_40xx)
{
	if (offset < 0x4000)
		return m_ram[offset + (m_ram_bank * 0x4000)];
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];   // last bank
}

WRITE8_MEMBER(a78_versaboard_device::write_40xx)
{
	if (offset < 0x4000)
		m_ram[offset + (m_ram_bank * 0x4000)] = data;
	else if (offset < 0x8000)
	{
		// hardware allows up to 256K ROM
		m_bank = (data & 0x0f) & m_bank_mask;
		m_ram_bank = BIT(data, 5);
	}
}


// MegaCart+

WRITE8_MEMBER(a78_megacart_device::write_40xx)
{
	if (offset < 0x4000)
		m_ram[offset + (m_ram_bank * 0x4000)] = data;
	else if (offset < 0x8000)
	{
		// hardware allows up to 512K ROM
		m_bank = (data & 0x1f) & m_bank_mask;
		m_ram_bank = BIT(data, 5);
	}
}


// VersaBoard + POKEY @ 0x0450

static MACHINE_CONFIG_FRAGMENT( a78_pokeyvb )
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("pokey450", POKEY, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 1.00)
MACHINE_CONFIG_END

machine_config_constructor a78_rom_p450_vb_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_pokeyvb );
}
