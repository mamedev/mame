// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo xxROM PCBs

 iNES mappers: 0, 2, 3, 7, 13, 34, 66, 94, 180, 185, 218


 Here we emulate the following PCBs

 * Nintendo NROM [mapper 0]
 * Nintendo AxROM [mapper 7]
 * Nintendo BxROM [mapper 34]
 * Nintendo CNROM [mapper 3 & 185]
 * Nintendo CPROM [mapper 13]
 * Nintendo GxROM/MxROM [mapper 66]
 * Nintendo UxROM [mapper 2] + Crazy Climbers variant [mapper 180]
 * Nintendo UN1ROM [mapper 94]
 * NoCash NOCHR [mapper 218]

 Known issues on specific mappers:

 * 034 Titanic 1912 (pirate BxROM): an older note reported missing graphics; not rechecked, no dump of the game was
   found for testing

 ***********************************************************************************************************/


#include "emu.h"
#include "nxrom.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NROM,     nes_nrom_device,     "nes_nrom",     "NES Cart NROM PCB")
DEFINE_DEVICE_TYPE(NES_NROM368,  nes_nrom368_device,  "nes_nrom368",  "NES Cart NROM-368 PCB")
DEFINE_DEVICE_TYPE(NES_FCBASIC,  nes_fcbasic_device,  "nes_fcbasic",  "NES Cart Famicom BASIC PCB")
DEFINE_DEVICE_TYPE(NES_AXROM,    nes_axrom_device,    "nes_axrom",    "NES Cart AxROM PCB")
DEFINE_DEVICE_TYPE(NES_INX_007T, nes_inx_007t_device, "nes_inx_007t", "NES Cart INX_007T_V01 PCB")
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
	: nes_axrom_device(mconfig, NES_AXROM, tag, owner, clock)
{
}

nes_axrom_device::nes_axrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
{
}

nes_inx_007t_device::nes_inx_007t_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_axrom_device(mconfig, NES_INX_007T, tag, owner, clock), m_outer(0), m_inner(0)
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
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_axrom_device::pcb_reset()
{
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
	prg32(0);
	chr8(0, m_chr_source);

	m_chr_open_bus = 0;
}

void nes_cprom_device::pcb_reset()
{
	set_nt_mirroring(PPU_MIRROR_VERT);   // NESdev CPROM: the nametable arrangement is wired, vertical
	prg32(0);
	chr4_0(0, m_chr_source);
	chr4_4(0, m_chr_source);
}

void nes_uxrom_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_un1rom_device::pcb_reset()
{
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

 In MAME: Supported.

 No need of specific handlers or IRQ

 -------------------------------------------------*/

/*-------------------------------------------------

 NROM-368 board emulation

 iNES: mapper 0 with 3xPRG banks
 This is an homebrew extension to map linearly 46KB
 or PRG in boards with no PRG bankswitch logic

 In MAME: Supported.

 -------------------------------------------------*/

uint8_t nes_nrom368_device::read_l(offs_t offset)
{
	LOG("nrom368 read_l, offset: %04x\n", offset);
	offset += 0x100;
	if (offset >= 0x800)
		return m_prg[offset - 0x800];
	else
		return get_open_bus();
}

uint8_t nes_nrom368_device::read_m(offs_t offset)
{
	LOG("nrom368 read_m, offset: %04x\n", offset);
	return m_prg[0x1800 + (offset & 0x1fff)];
}

uint8_t nes_nrom368_device::read_h(offs_t offset)
{
	LOG("nrom368 read_h, offset: %04x\n", offset);
	return m_prg[0x3800 + (offset & 0x7fff)];
}

/*-------------------------------------------------

 AxROM board emulation

 Games: Arch Rivals, Battletoads, Cabal, Commando, Solstice

 writes to 0x8000-0xffff change PRG banks + sets mirroring

 The register is xxxM xPPP: M selects the one-screen mirroring page and the
 low three bits select the 32K PRG bank (bit 3 is also used for oversize ROMs).

 AMROM has bus conflict
 AOROM has generally bus conflict too, but some later pcbs
 added some discrete component to disable ROM and avoid
 conflicts. No AOROM games is known to glitch due to lack of
 bus conflict, so it seems safe to emulate AOROM
 without bus conflict. The wiki lists Double Dare and Wheel of Fortune as
 the games which break if bus conflicts are emulated, and ANROM boards have
 a 74HC02 which avoids them entirely.

 iNES: mapper 7

 In MAME: Supported.

 -------------------------------------------------*/

void nes_axrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG("axrom write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	prg32(data);
}

/*-------------------------------------------------

 INX_007T_V01 board emulation

 Games: Battletoads & Double Dragon (Retro-Bit re-release)

 NES 2.0: mapper 470

 NESdev (NES 2.0 Mapper 470): basically AOROM with an additional outer bank register at $5000-$5FFF whose data
 selects the 256 KiB outer bank. The outer bank register is cleared to $00 on reset. The AOROM part is the
 AxROM board above: the 32K bank and the single screen page come from a write to $8000-$FFFF, with bus conflicts.
 The outer bank changes the mapped bank at once and the AOROM register keeps its value.

 In MAME: Supported.

 The one dump, Battletoads-Double Dragon (USA, Europe) (Retro-Bit), has 1024K PRG and CHR-RAM.

 -------------------------------------------------*/

void nes_inx_007t_device::device_start()
{
	common_start();
	save_item(NAME(m_outer));
	save_item(NAME(m_inner));
}

void nes_inx_007t_device::pcb_reset()
{
	nes_axrom_device::pcb_reset();
	m_outer = 0;
	m_inner = 0;
	update_prg();
}

void nes_inx_007t_device::update_prg()
{
	// 8 32K banks in each 256K outer bank
	prg32((m_outer << 3) | m_inner);
}

void nes_inx_007t_device::write_l(offs_t offset, u8 data)
{
	LOG("inx_007t write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000 && offset < 0x2000)    // $5000-$5FFF
	{
		m_outer = data;
		update_prg();
	}
}

void nes_inx_007t_device::write_h(offs_t offset, u8 data)
{
	LOG("inx_007t write_h, offset: %04x, data: %02x\n", offset, data);

	// like AOROM this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	m_inner = data & 0x07;
	update_prg();
}

/*-------------------------------------------------

 BxROM board emulation

 writes to 0x8000-0xffff change PRG banks

 The real BNROM register is xxxx xxPP (a 74HC161 latch). We use all 8 bits,
 as the wiki recommends, to allow oversize ROMs. The board is always subject
 to bus conflict. The NINA-001 board, which shares this mapper number but has
 no bus conflicts, is emulated by its own device (NES 2.0 submapper 1 or the
 nes.hsi variant code), while NES 2.0 submapper 2 selects BNROM and turns
 bus conflicts on.

 iNES: mapper 34

 -------------------------------------------------*/

void nes_bxrom_device::write_h(offs_t offset, uint8_t data)
{
	/* This portion of the mapper is nearly identical to Mapper 7, except no one-screen mirroring */
	/* Deadly Towers is really a BxROM game - the demo screens look wrong using mapper 7. */
	LOG("bxrom write_h, offset: %04x, data: %02x\n", offset, data);

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

 iNES: mappers 3 & 185 (the latter for games using Pins as
 protection)

 Real boards have bus conflicts. For mapper 3 they are emulated only for NES 2.0
 submapper 2; submapper 1, submapper 0 and iNES 1.0 images have none (see
 nes_ines.hxx). NESdev's Submapper page recommends enforcing them when no
 submapper is given, but Solar Wars (World) (Aftermarket) (Unl), submapper 0 in
 No-Intro, draws garbage with them; Cybernoid needs them and is submapper 2.
 For mapper 185, NES 2.0 submappers 4-7 select the latch value (0-3) which
 enables CHR-ROM; without a submapper the nes.hsi variant code is used.

 Notice that BANDAI_PT554 board (Aerobics Studio) uses very
 similar hardware with an additional speech chip, which gets
 writes to 0x6000; pt554.cpp plays its speech from samples

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cnrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG("cxrom write_h, offset: %04x, data: %02x\n", offset, data);

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

 writes to 0x8000-0xffff (xxxx xxCC, subject to bus conflict) select the 4K CHR RAM
 bank at PPU 0x1000-0x1fff. The 4K bank at 0x0000-0x0fff is fixed to the first page
 and mirroring is hardwired vertical.

 iNES: mapper 13

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cprom_device::write_h(offs_t offset, uint8_t data)
{
	LOG("cprom write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	chr4_4(data, CHRRAM);
}


/*-------------------------------------------------

 GxROM/MxROM board emulation

 writes to 0x8000-0xffff change PRG and CHR banks

 The register is xxPP xxCC (bit 5 is unused on MHROM, which has only 64K PRG)
 and the real boards are subject to bus conflict.

 iNES: mapper 66

 -------------------------------------------------*/

void nes_gxrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG("gxrom write_h, offset %04x, data: %02x\n", offset, data);

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

 Real boards have bus conflicts. For iNES images they are enabled only by
 NES 2.0 submapper 2 (submapper 1 and the default have none), see nes_ines.hxx.

 iNES: mapper 2

 In MAME: Supported.

 -------------------------------------------------*/

void nes_uxrom_device::write_h(offs_t offset, uint8_t data)
{
	LOG("uxrom write_h, offset: %04x, data: %02x\n", offset, data);

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

 This is the UNROM PCB with an AND gate in place of the OR gate, so it has
 bus conflicts like UNROM does (enabled for iNES mapper 180 in nes_ines.hxx).

 iNES: mapper 180

 In MAME: Supported.

 -------------------------------------------------*/

void nes_uxrom_cc_device::write_h(offs_t offset, uint8_t data)
{
	LOG("uxrom_cc write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_cdef(data);
}

/*-------------------------------------------------

 UN1ROM board emulation

 Games: Senjou no Okami

 writes to 0x8000-0xffff (xxxP PPxx, subject to bus conflict) change the 16K PRG
 bank at 0x8000-0xbfff, while 0xc000-0xffff is fixed to the last bank.

 iNES: mapper 94

 In MAME: Supported.

 -------------------------------------------------*/

void nes_un1rom_device::write_h(offs_t offset, uint8_t data)
{
	LOG("un1rom write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg16_89ab((data >> 2) & 0x07);
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
