// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo xxROM PCBs


 Here we emulate the following PCBs

 * Nintendo NROM [mapper 0]
 * Nintendo AxROM [mapper 7]
 * Nintendo BxROM [mapper 34]
 * Nintendo CNROM [mapper 3 & 185]
 * Nintendo CPROM [mapper 13]
 * Nintendo GxROM/MxROM [mapper 66]
 * Nintendo UxROM [mapper 2] + Crazy Climbers variant [mapper 180]
 * Nintendo UN1ROM [mapper 94]

 Known issues on specific mappers:

 * 000 F1 Race requires more precise PPU timing. It currently has plenty of 1-line glitches.
 * 003 Firehouse Rescue has flashing graphics (same PPU issue as Back to the Future 2 & 3?)
 * 007 Marble Madness has small graphics corruptions
 * 034 Titanic 1912 (pirate BxROM) has missing gfx (same PPU issue of many Waixing titles almost for sure)

 ***********************************************************************************************************/


#include "emu.h"
#include "nxrom.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NROM,     nes_nrom_device,     "nes_nrom",     "NES Cart NROM PCB")
DEFINE_DEVICE_TYPE(NES_NROM368,  nes_nrom368_device,  "nes_nrom368",  "NES Cart NROM-368 PCB")
DEFINE_DEVICE_TYPE(NES_FCBASIC,  nes_fcbasic_device,  "nes_fcbasic",  "NES Cart Famicom BASIC PCB")
DEFINE_DEVICE_TYPE(NES_AXROM,    nes_axrom_device,    "nes_axrom",    "NES Cart AxROM PCB")
DEFINE_DEVICE_TYPE(NES_BXROM,    nes_bxrom_device,    "nes_bxrom",    "NES Cart BxROM PCB")
DEFINE_DEVICE_TYPE(NES_CNROM,    nes_cnrom_device,    "nes_cnrom",    "NES Cart CNROM PCB")
DEFINE_DEVICE_TYPE(NES_CPROM,    nes_cprom_device,    "nes_cprom",    "NES Cart CPROM PCB")
DEFINE_DEVICE_TYPE(NES_GXROM,    nes_gxrom_device,    "nes_gxrom",    "NES Cart GxROM PCB")
DEFINE_DEVICE_TYPE(NES_UXROM,    nes_uxrom_device,    "nes_uxrom",    "NES Cart UxROM PCB")
DEFINE_DEVICE_TYPE(NES_UXROM_CC, nes_uxrom_cc_device, "nes_uxrom_cc", "NES Cart UNROM M5 (Crazy Climber) PCB")
DEFINE_DEVICE_TYPE(NES_UN1ROM,   nes_un1rom_device,   "nes_un1rom",   "NES Cart UN1ROM PCB")
DEFINE_DEVICE_TYPE(NES_NOCHR,    nes_nochr_device,    "nes_nochr",    "NES Cart NoCash NOCHR PCB")


nes_nrom_device::nes_nrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_nes_cart_interface(mconfig, *this)
{
}

nes_nrom_device::nes_nrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NROM, tag, owner, clock)
{
}

nes_nrom368_device::nes_nrom368_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NROM368, tag, owner, clock)
{
}

nes_fcbasic_device::nes_fcbasic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_FCBASIC, tag, owner, clock)
{
}

nes_axrom_device::nes_axrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_AXROM, tag, owner, clock)
{
}

nes_bxrom_device::nes_bxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_BXROM, tag, owner, clock)
{
}

nes_cnrom_device::nes_cnrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_chr_open_bus(0)
{
}

nes_cnrom_device::nes_cnrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_cnrom_device(mconfig, NES_CNROM, tag, owner, clock)
{
}

nes_cprom_device::nes_cprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_CPROM, tag, owner, clock)
{
}

nes_gxrom_device::nes_gxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_GXROM, tag, owner, clock)
{
}

nes_uxrom_device::nes_uxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_UXROM, tag, owner, clock)
{
}

nes_uxrom_cc_device::nes_uxrom_cc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_UXROM_CC, tag, owner, clock)
{
}

nes_un1rom_device::nes_un1rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_UN1ROM, tag, owner, clock)
{
}

nes_nochr_device::nes_nochr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NOCHR, tag, owner, clock)
{
}


void nes_nrom_device::common_start()
{
	// PRG
	save_item(NAME(m_prg_bank));

	// CHR
	save_item(NAME(m_chr_source));
	save_item(NAME(m_chr_src));
	save_item(NAME(m_chr_orig));

	// NT
	save_item(NAME(m_mirroring));
	save_item(NAME(m_nt_src));
	save_item(NAME(m_nt_orig));
	save_item(NAME(m_nt_writable));
}

void nes_nrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_axrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_cnrom_device::device_start()
{
	common_start();
	save_item(NAME(m_chr_open_bus));
}

void nes_cnrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	m_chr_open_bus = 0;
}

void nes_cprom_device::pcb_reset()
{
	m_chr_source = CHRRAM;
	prg32(0);
	chr4_0(0, m_chr_source);
	chr4_4(0, m_chr_source);
}

void nes_uxrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_un1rom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_nochr_device::pcb_reset()
{
	prg32(0);

	switch (m_mirroring)
	{
		case PPU_MIRROR_VERT: m_ciram_a10 = 10; break;
		case PPU_MIRROR_HORZ: m_ciram_a10 = 11; break;
		case PPU_MIRROR_LOW:  m_ciram_a10 = 12; break;
		case PPU_MIRROR_HIGH: m_ciram_a10 = 13; break;
	}
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 NROM board emulation

 Games: Mario Bros., Super Mario Bros., Tennis and most of
 the first generation games

 iNES: mapper 0

 In MESS: Supported, no need of specific handlers or IRQ

 -------------------------------------------------*/

/*-------------------------------------------------

 NROM-368 board emulation

 iNES: mapper 0 with 3xPRG banks
 This is an homebrew extension to map linearly 46KB
 or PRG in boards with no PRG bankswitch logic

 In MESS: Supported

 -------------------------------------------------*/

uint8_t nes_nrom368_device::read_l(offs_t offset)
{
	LOG_MMC(("nrom368 read_l, offset: %04x\n", offset));
	offset += 0x100;
	if (offset >= 0x800)
		return m_prg[offset - 0x800];
	else
		return get_open_bus();
}

uint8_t nes_nrom368_device::read_m(offs_t offset)
{
	LOG_MMC(("nrom368 read_m, offset: %04x\n", offset));
	return m_prg[0x1800 + (offset & 0x1fff)];
}

uint8_t nes_nrom368_device::read_h(offs_t offset)
{
	LOG_MMC(("nrom368 read_h, offset: %04x\n", offset));
	return m_prg[0x3800 + (offset & 0x7fff)];
}

/*-------------------------------------------------

 AxROM board emulation

 Games: Arch Rivals, Battletoads, Cabal, Commando, Solstice

 writes to 0x8000-0xffff change PRG banks + sets mirroring

 AMROM has bus conflict
 AOROM has generally bus conflict too, but some later pcbs
 added some discrete component to disable ROM and avoid
 conflicts. No AOROM games is known to glitch due to lack of
 bus conflict, so it seems safe to emulate AOROM
 without bus conflict.

 iNES: mapper 7

 In MESS: Supported

 -------------------------------------------------*/

void nes_axrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("axrom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	prg32(data);
}

/*-------------------------------------------------

 BxROM board emulation

 writes to 0x8000-0xffff change PRG banks

 iNES: mapper 34

 -------------------------------------------------*/

void nes_bxrom_device::write_h(offs_t offset, uint8_t data)
{
	/* This portion of the mapper is nearly identical to Mapper 7, except no one-screen mirroring */
	/* Deadly Towers is really a BxROM game - the demo screens look wrong using mapper 7. */
	LOG_MMC(("bxrom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict, but the same is not true for some pirate variants
	data = account_bus_conflict(offset, data);

	prg32(data);
}

/*-------------------------------------------------

 CNROM board emulation

 Games: B-Wings, Mighty Bomb Jack, Seicross, Spy vs. Spy,
 Adventure Island, Flipull, Friday 13th, GeGeGe no
 Kitarou, Ghostbusters, Gradius, Hokuto no Ken, Milon's
 Secret Castle

 writes to 0x8000-0xffff change CHR 8K banks

 missing BC?

 iNES: mappers 3 & 185 (the latter for games using Pins as
 protection)

 Notice that BANDAI_PT554 board (Aerobics Studio) uses very
 similar hardware but with an additional sound chip which
 gets writes to 0x6000 (currently unemulated in MESS)

 In MESS: Supported

 -------------------------------------------------*/

void nes_cnrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("cxrom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict, but the same is not true for some pirate variants
	data = account_bus_conflict(offset, data);

	if (m_ce_mask)
	{
		//printf("mask %x state %x\n", m_ce_mask, m_ce_state);
		chr8(data & ~m_ce_mask, CHRROM);

		if ((data & m_ce_mask) == m_ce_state)
			m_chr_open_bus = 0;
		else
			m_chr_open_bus = 1;
	}
	else
		chr8(data, CHRROM);
}

uint8_t nes_cnrom_device::chr_r(offs_t offset)
{
	// a few CNROM boards contained copy protection schemes through
	// suitably configured diodes, so that subsequent CHR reads can
	// give actual VROM content or open bus values.
	// For most boards, chr_open_bus remains always zero.
	if (m_chr_open_bus)
		return get_open_bus();

	return device_nes_cart_interface::chr_r(offset);
}


/*-------------------------------------------------

 CPROM board emulation

 Games: Videomation

 writes to 0x8000-0xffff change CHR 4K lower banks

 iNES: mapper 13

 In MESS: Supported

 -------------------------------------------------*/

void nes_cprom_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("cprom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	chr4_4(data, CHRRAM);
}


/*-------------------------------------------------

 GxROM/MxROM board emulation

 writes to 0x8000-0xffff change PRG and CHR banks

 iNES: mapper 66

 -------------------------------------------------*/

void nes_gxrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("gxrom write_h, offset %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg32(data >> 4);
	chr8(data & 0x0f, CHRROM);
}

/*-------------------------------------------------

 UxROM board emulation

 Games: Castlevania, Dragon Quest II, Duck Tales, MegaMan,
 Metal Gear

 writes to 0x8000-0xffff change PRG 16K lower banks

 iNES: mapper 2

 In MESS: Supported.

 -------------------------------------------------*/

void nes_uxrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("uxrom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab(data);
}

/*-------------------------------------------------

 Nihon Bussan UNROM M5

 Games: Crazy Climber Jpn

 Very simple mapper: prg16_89ab is always set to bank 0,
 while prg16_cdef is set by writes to 0x8000-0xffff. The game
 uses a custom controller.

 iNES: mapper 180

 In MESS: Supported.

 -------------------------------------------------*/

void nes_uxrom_cc_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("uxrom_cc write_h, offset: %04x, data: %02x\n", offset, data));

	prg16_cdef(data);
}

/*-------------------------------------------------

 UN1ROM board emulation

 Games: Senjou no Okami

 writes to 0x8000-0xffff change PRG 16K lower banks

 iNES: mapper 94

 In MESS: Supported.

 -------------------------------------------------*/

void nes_un1rom_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("un1rom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab(data >> 2);
}

/*-------------------------------------------------

 NoCash NOCHR board emulation

 This is a homebrew PCB design on a single chip
 (+optional CIC) which uses the NTRAM as CHRRAM!
 One of PPU A10-A13 is tied directly to CIRAM A10,
 meaning the 16K PPU address space (save for the
 palette RAM at 0x3f00-0x3fff) appears as the first
 1K page of CIRAM 1, 2, 4, or 8 times followed by
 the second 1K page of CIRAM 1, 2, 4, or 8 times,
 and then mirrored as many times as necessary.

 iNES: mapper 218

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nochr_device::chr_w(offs_t offset, u8 data)
{
	offset = (offset & 0x3ff) | BIT(offset, m_ciram_a10) << 10;
	m_ciram[offset] = data;
}

u8 nes_nochr_device::chr_r(offs_t offset)
{
	offset = (offset & 0x3ff) | BIT(offset, m_ciram_a10) << 10;
	return m_ciram[offset];
}

void nes_nochr_device::nt_w(offs_t offset, u8 data)
{
	chr_w(offset + 0x2000, data);
}

u8 nes_nochr_device::nt_r(offs_t offset)
{
	return chr_r(offset + 0x2000);
}
