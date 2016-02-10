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

#include "sound/samples.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_NROM = &device_creator<nes_nrom_device>;
const device_type NES_NROM368 = &device_creator<nes_nrom368_device>;
const device_type NES_FCBASIC = &device_creator<nes_fcbasic_device>;
const device_type NES_AXROM = &device_creator<nes_axrom_device>;
const device_type NES_BXROM = &device_creator<nes_bxrom_device>;
const device_type NES_CNROM = &device_creator<nes_cnrom_device>;
const device_type NES_CPROM = &device_creator<nes_cprom_device>;
const device_type NES_GXROM = &device_creator<nes_gxrom_device>;
const device_type NES_UXROM = &device_creator<nes_uxrom_device>;
const device_type NES_UXROM_CC = &device_creator<nes_uxrom_cc_device>;
const device_type NES_UN1ROM = &device_creator<nes_un1rom_device>;
const device_type NES_NOCHR = &device_creator<nes_nochr_device>;


nes_nrom_device::nes_nrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_nes_cart_interface( mconfig, *this )
{
}

nes_nrom_device::nes_nrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, NES_NROM, "NES Cart NROM PCB", tag, owner, clock, "nes_nrom", __FILE__),
						device_nes_cart_interface( mconfig, *this )
{
}

nes_nrom368_device::nes_nrom368_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NROM368, "NES Cart NROM-368 PCB", tag, owner, clock, "nes_nrom368", __FILE__)
{
}

nes_fcbasic_device::nes_fcbasic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_FCBASIC, "NES Cart Famicom BASIC PCB", tag, owner, clock, "nes_fcbasic", __FILE__)
{
}

nes_axrom_device::nes_axrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_AXROM, "NES Cart AxROM PCB", tag, owner, clock, "nes_axrom", __FILE__)
{
}

nes_bxrom_device::nes_bxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_BXROM, "NES Cart BxROM PCB", tag, owner, clock, "nes_bxrom", __FILE__)
{
}

nes_cnrom_device::nes_cnrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_chr_open_bus(0)
				{
}

nes_cnrom_device::nes_cnrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CNROM, "NES Cart CNROM PCB", tag, owner, clock, "nes_cnrom", __FILE__), m_chr_open_bus(0)
				{
}

nes_cprom_device::nes_cprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CPROM, "NES Cart CPROM PCB", tag, owner, clock, "nes_cprom", __FILE__)
{
}

nes_gxrom_device::nes_gxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_GXROM, "NES Cart GxROM PCB", tag, owner, clock, "nes_gxrom", __FILE__)
{
}

nes_uxrom_device::nes_uxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_UXROM, "NES Cart UxROM PCB", tag, owner, clock, "nes_uxrom", __FILE__)
{
}

nes_uxrom_cc_device::nes_uxrom_cc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_UXROM_CC, "NES Cart UNROM M5 PCB (Crazy Climber)", tag, owner, clock, "nes_uxrom_cc", __FILE__)
{
}

nes_un1rom_device::nes_un1rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_UN1ROM, "NES Cart UN1ROM PCB", tag, owner, clock, "nes_un1rom", __FILE__)
{
}

nes_nochr_device::nes_nochr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NOCHR, "NES Cart NoCash NOCHR PCB", tag, owner, clock, "nes_nochr", __FILE__)
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

	// open bus
	save_item(NAME(m_open_bus));
}

void nes_nrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_axrom_device::device_start()
{
	common_start();
}

void nes_axrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);

	set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_bxrom_device::device_start()
{
	common_start();
}

void nes_bxrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
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

void nes_cprom_device::device_start()
{
	common_start();
}

void nes_cprom_device::pcb_reset()
{
	m_chr_source = CHRRAM;
	prg32(0);
	chr4_0(0, m_chr_source);
	chr4_4(0, m_chr_source);
}

void nes_gxrom_device::device_start()
{
	common_start();
}

void nes_gxrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_uxrom_device::device_start()
{
	common_start();
}

void nes_uxrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_uxrom_cc_device::device_start()
{
	common_start();
}

void nes_uxrom_cc_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_un1rom_device::device_start()
{
	common_start();
}

void nes_un1rom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
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

READ8_MEMBER(nes_nrom368_device::read_l)
{
	LOG_MMC(("nrom368 read_l, offset: %04x\n", offset));
	offset += 0x100;
	if (offset >= 0x800)
		return m_prg[offset - 0x800];
	else
		return m_open_bus;
}

READ8_MEMBER(nes_nrom368_device::read_m)
{
	LOG_MMC(("nrom368 read_m, offset: %04x\n", offset));
	return m_prg[0x1800 + (offset & 0x1fff)];
}

READ8_MEMBER(nes_nrom368_device::read_h)
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

WRITE8_MEMBER(nes_axrom_device::write_h)
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

WRITE8_MEMBER(nes_bxrom_device::write_h)
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

WRITE8_MEMBER(nes_cnrom_device::write_h)
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

READ8_MEMBER(nes_cnrom_device::chr_r)
{
	int bank = offset >> 10;

	// a few CNROM boards contained copy protection schemes through
	// suitably configured diodes, so that subsequent CHR reads can
	// give actual VROM content or open bus values.
	// For most boards, chr_open_bus remains always zero.
	if (m_chr_open_bus)
		return m_open_bus;

	return m_chr_access[bank][offset & 0x3ff];
}


/*-------------------------------------------------

 CPROM board emulation

 Games: Videomation

 writes to 0x8000-0xffff change CHR 4K lower banks

 iNES: mapper 13

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_cprom_device::write_h)
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

WRITE8_MEMBER(nes_gxrom_device::write_h)
{
	LOG_MMC(("gxrom write_h, offset %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg32((data & 0xf0) >> 4);
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

WRITE8_MEMBER(nes_uxrom_device::write_h)
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

WRITE8_MEMBER(nes_uxrom_cc_device::write_h)
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

WRITE8_MEMBER(nes_un1rom_device::write_h)
{
	LOG_MMC(("un1rom write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab(data >> 2);
}

/*-------------------------------------------------

 NoCash NOCHR board emulation

 This is an homebrew PCB design on a single chip
 (+possibly CIC) which uses the NTRAM as CHRRAM!

 iNES: mapper 218

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_nochr_device::chr_w)
{
	int mirr = get_mirroring();
	if (mirr == PPU_MIRROR_HIGH)
		m_ciram[(offset & 0x3ff) + 0x000] = data;
	else if (mirr == PPU_MIRROR_LOW)
		m_ciram[(offset & 0x3ff) + 0x400] = data;
	else
		m_ciram[offset & 0x7ff] = data; // not sure here, since there is no software to test...
}

READ8_MEMBER(nes_nochr_device::chr_r)
{
	int mirr = get_mirroring();
	if (mirr == PPU_MIRROR_HIGH)
		return m_ciram[(offset & 0x3ff) + 0x000];
	else if (mirr == PPU_MIRROR_LOW)
		return m_ciram[(offset & 0x3ff) + 0x400];
	else
		return m_ciram[offset & 0x7ff]; // not sure here, since there is no software to test...
}
