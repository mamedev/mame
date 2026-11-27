// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for pirate cart PCBs

 iNES mappers: 27, 91, 107, 117, 144, 156, 170, 176, 186, 329, 515, 519, 521, 526


 Here we emulate the various PCBs used by Asian & Korean pirate games

 TODO:
 - Are the scrolling glitches (check status bar) in Magic Dragon correct? FWIW, NEStopia behaves similarly

 ***********************************************************************************************************/


#include "emu.h"
#include "pirate.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_AGCI_50282,  nes_agci_device,        "nes_agci50282",   "NES Cart AGCI 50282 PCB")
DEFINE_DEVICE_TYPE(NES_DREAMTECH01, nes_dreamtech_device,   "nes_dreamtech",   "NES Cart Dreamtech01 PCB")
DEFINE_DEVICE_TYPE(NES_FUJIYA,      nes_fujiya_device,      "nes_fujiya",      "NES Cart Fujiya NROM + Security PCB")
DEFINE_DEVICE_TYPE(NES_FUKUTAKE,    nes_fukutake_device,    "nes_fukutake",    "NES Cart Fukutake Study Box PCB")
DEFINE_DEVICE_TYPE(NES_KKACHI,      nes_kkachi_device,      "nes_kkachi",      "NES Cart Kkachi-wa Norae Chingu PCB")
DEFINE_DEVICE_TYPE(NES_NORAEBANG,   nes_noraebang_device,   "nes_noraebang",   "NES Cart Family Noraebang PCB")
DEFINE_DEVICE_TYPE(NES_FUTUREMEDIA, nes_futuremedia_device, "nes_futuremedia", "NES Cart FutureMedia PCB")
DEFINE_DEVICE_TYPE(NES_MAGSERIES,   nes_magseries_device,   "nes_magseries",   "NES Cart Magical Series PCB")
DEFINE_DEVICE_TYPE(NES_DAOU306,     nes_daou306_device,     "nes_daou306",     "NES Cart Daou 306 PCB")
DEFINE_DEVICE_TYPE(NES_CC21,        nes_cc21_device,        "nes_cc21",        "NES Cart CC-21 PCB")
DEFINE_DEVICE_TYPE(NES_XIAOZY,      nes_xiaozy_device,      "nes_xiaozy",      "NES Cart Xiao Zhuan Yuan PCB")
DEFINE_DEVICE_TYPE(NES_EDU2K,       nes_edu2k_device,       "nes_edu2k",       "NES Cart Educational Computer 2000 PCB")
DEFINE_DEVICE_TYPE(NES_JY830623C,   nes_jy830623c_device,   "nes_jy830623c",   "NES Cart JY830623C PCB")
DEFINE_DEVICE_TYPE(NES_43272,       nes_43272_device,       "nes_43272",       "NES Cart UNL-43272 PCB")
DEFINE_DEVICE_TYPE(NES_EH8813A,     nes_eh8813a_device,     "nes_eh8813a",     "NES Cart UNL-EH8813A PCB")
DEFINE_DEVICE_TYPE(NES_BJ56,        nes_bj56_device,        "nes_bj56",        "NES Cart UNL-BJ-56 PCB")


nes_agci_device::nes_agci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_AGCI_50282, tag, owner, clock)
{
}

nes_kkachi_device::nes_kkachi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_KKACHI, tag, owner, clock)
{
}

nes_noraebang_device::nes_noraebang_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_NORAEBANG, tag, owner, clock)
	, m_ym2413(*this, "ym2413")
{
}

nes_dreamtech_device::nes_dreamtech_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_DREAMTECH01, tag, owner, clock)
{
}

nes_fujiya_device::nes_fujiya_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_FUJIYA, tag, owner, clock), m_latch(0)
{
}

nes_fukutake_device::nes_fukutake_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_FUKUTAKE, tag, owner, clock), m_latch(0)
{
}

nes_futuremedia_device::nes_futuremedia_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_FUTUREMEDIA, tag, owner, clock), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0)
{
}

nes_magseries_device::nes_magseries_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_MAGSERIES, tag, owner, clock)
{
}

nes_daou306_device::nes_daou306_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_DAOU306, tag, owner, clock)
{
}

nes_cc21_device::nes_cc21_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_CC21, tag, owner, clock)
{
}

nes_xiaozy_device::nes_xiaozy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_XIAOZY, tag, owner, clock)
{
}

nes_edu2k_device::nes_edu2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_EDU2K, tag, owner, clock)
{
}

nes_jy830623c_device::nes_jy830623c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_JY830623C, tag, owner, clock), m_submapper(0), m_latch(0), m_irq_enable(0), m_a12_count(0), m_m2_count(0), m_m2_prescaler(0)
{
}

nes_43272_device::nes_43272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_43272, tag, owner, clock), m_latch(0)
{
}

nes_eh8813a_device::nes_eh8813a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_EH8813A, tag, owner, clock), m_jumper(0), m_latch(0), m_reg(0)
{
}




void nes_dreamtech_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(8);
}

void nes_fukutake_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));

	// 2816 (?) bytes of RAM
	save_item(NAME(m_ram));
}

void nes_fukutake_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_futuremedia_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_futuremedia_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRROM);

	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
}

void nes_daou306_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_daou306_device::pcb_reset()
{
	prg16_89ab(m_prg_chunks - 2);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_LOW);

	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
}

void nes_cc21_device::pcb_reset()
{
	prg32(0);
	chr4_0(0, CHRROM);
	chr4_4(0, CHRROM);
	set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_xiaozy_device::pcb_reset()
{
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, m_chr_source);
}

void nes_edu2k_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_edu2k_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_jy830623c_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_a12_count));
	save_item(NAME(m_a12_last));
	save_item(NAME(m_m2_count));
	save_item(NAME(m_m2_prescaler));
}

void nes_jy830623c_device::pcb_reset()
{
	m_latch = 0;
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	update_banks();
	// Submapper 0's mirroring is hard-wired (NESdev) and vertical: every submapper 0 dump in the owner's verified set
	// has a vertical header except Super Mario & Sonic 2 (Asia) (Ja) (Pirate), whose header says horizontal but which
	// only renders with vertical mirroring. Submapper 1 starts vertical and selects it with $6004/$6005.
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_enable = 0;
	m_a12_count = 0;
	m_a12_last = 0;
	m_m2_count = 0;
	m_m2_prescaler = 0;
}

void nes_43272_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_43272_device::pcb_reset()
{
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, m_chr_source);

	m_latch = 0x81;
}

void nes_eh8813a_device::device_start()
{
	common_start();
	save_item(NAME(m_jumper));
	save_item(NAME(m_latch));
	save_item(NAME(m_reg));
}

void nes_eh8813a_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRROM);

	m_jumper = 0;
	m_latch = 0;
	m_reg = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 AGCI 50282 bootleg board emulation

 Games: Death Race

 iNES: mapper 144

 In MAME: Supported.

 -------------------------------------------------*/

void nes_agci_device::write_h(offs_t offset, u8 data)
{
	LOG("agci write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	// bit 0 is always determined by the ROM value at the offset due to a resistor on the board
	data = account_bus_conflict(offset, data | 1);

	chr8(data >> 4, CHRROM);
	prg32(data);
}

/*-------------------------------------------------

 Board DREAMTECH01

 Games: Korean Igo

 NES 2.0: mapper 521

 In MAME: Supported.

 -------------------------------------------------*/

/*-------------------------------------------------

 Kkachi-wa Norae Chingu board

 Games: Kkachi-wa Norae Chingu (Korean karaoke)

 UNROM banking without bus conflicts: a write to $8000-$FFFF selects the 16 KiB bank at $8000, the last bank is
 fixed at $C000. The cartridge's microphone socket goes to an ADC read at $6000 and $6001 (NESdev NES 2.0 Mapper
 517); the ADC is not documented and reads as open bus here.

 NES 2.0: mapper 517

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_kkachi_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

u8 nes_kkachi_device::read_m(offs_t offset)
{
	LOG("kkachi read_m, offset: %04x\n", offset);
	return get_open_bus();
}

void nes_kkachi_device::write_h(offs_t offset, u8 data)
{
	LOG("kkachi write_h, offset: %04x, data: %02x\n", offset, data);
	prg16_89ab(data);
}

/*-------------------------------------------------

 Family Noraebang (Korean karaoke cartridge, no UNIF board)

 Games: Family Noraebang (DS-H352, DS-H353, DS-H354, DS-H355 and the
 plain release)

 UNROM-style PRG banking with a slot for an expansion cartridge, a
 YM2413 clone (K-663A) for audio and an SPI ADC for a microphone.
 $8000 (mask $E00F): R BBBBBB. R=1 selects the internal ROM, the last
 1 MiB of the image, R=0 the expansion cartridge ROM, the rest of the
 image before it. B is the 16 KiB bank at $8000-$BFFF; $C000-$FFFF is
 always the last bank of the internal ROM. There are no bus conflicts.
 $6000/$6001 (mask $E003): YM2413 address and data.
 $6003 bit 7 (mask $E003): the ADC's data out. NESdev gives the chip
 select ($C002 bit 7) and clock ($C003 bit 7) writes without a mask; the
 ROMs use exactly $C002 and $C003 (an 8-bit conversion: chip select low,
 four clocks, eight clocks read after each falling edge, chip select high).

 NES 2.0: mapper 515

 In MAME: Preliminary supported. The YM2413's clock and output level are
 estimates, the microphone reads as silence (mid-scale), and the sound
 needs the YM2413's instrument ROM (ym2413_instruments.bin) in the rompath.

 -------------------------------------------------*/

void nes_noraebang_device::device_start()
{
	nes_nrom_device::device_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_spi_cs));
	save_item(NAME(m_spi_clk));
	save_item(NAME(m_spi_count));
	save_item(NAME(m_spi_out));
}

void nes_noraebang_device::device_add_mconfig(machine_config &config)
{
	// the K-663A is a YM2413 clone; its clock and the output level are not known
	YM2413(config, m_ym2413, XTAL(3'579'545)).add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0);
}

void nes_noraebang_device::pcb_reset()
{
	nes_nrom_device::pcb_reset();
	m_reg = 0x80;
	m_spi_cs = 1;           // deselected, clock idle high (the ROM ends every conversion that way)
	m_spi_clk = 1;
	m_spi_count = 0;
	m_spi_out = 0;
}

u8 nes_noraebang_device::read_m(offs_t offset)
{
	switch (offset & 3)
	{
		case 3:     // microphone ADC data out
			return (m_spi_out << 7) | (get_open_bus() & 0x7f);
	}
	return get_open_bus();
}

void nes_noraebang_device::write_m(offs_t offset, u8 data)
{
	LOG("noraebang write_m, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 3)
	{
		case 0:
			m_ym2413->address_w(data);
			break;
		case 1:
			m_ym2413->data_w(data);
			break;
	}
}

u8 nes_noraebang_device::read_h(offs_t offset)
{
	// the internal ROM is the last 1 MiB of the image, the expansion cartridge's ROM whatever comes before it
	const u32 ext_size = (m_prg_size > 0x100000) ? (m_prg_size - 0x100000) : 0;
	const u32 int_size = m_prg_size - ext_size;

	if (offset >= 0x4000)       // $C000-$FFFF: the last bank of the internal ROM
		return m_prg[ext_size + int_size - 0x4000 + (offset & 0x3fff)];

	const u32 bank = m_reg & 0x3f;
	if (BIT(m_reg, 7))
		return m_prg[ext_size + ((bank * 0x4000 + offset) % int_size)];
	if (ext_size)
		return m_prg[(bank * 0x4000 + offset) % ext_size];
	return get_open_bus();      // no expansion cartridge
}

void nes_noraebang_device::write_h(offs_t offset, u8 data)
{
	LOG("noraebang write_h, offset: %04x, data: %02x\n", offset, data);

	if ((offset & 0x600f) == 0x0000)        // $8000-$9FFF, mask $E00F
		m_reg = data;
	else if (offset == 0x4002)              // $C002: ADC chip select (bit 7, active low)
	{
		const u8 cs = BIT(data, 7);
		if (!cs && m_spi_cs)
			m_spi_count = 0;                // selected: a conversion starts
		if (cs)
			m_spi_out = 0;
		m_spi_cs = cs;
	}
	else if (offset == 0x4003)              // $C003: ADC clock (bit 7)
	{
		// What the ROM does: chip select low, four clocks, then eight clocks with a read of $6003 after each falling
		// edge. The data out line changes on the falling edges: the fifth to twelfth carry the 8-bit sample, MSB
		// first. No microphone exists here, so the sample is mid-scale (silence).
		const u8 clk = BIT(data, 7);
		if (!m_spi_cs && m_spi_clk && !clk)
		{
			m_spi_count++;
			m_spi_out = (m_spi_count >= 5 && m_spi_count <= 12) ? BIT(0x80, 12 - m_spi_count) : 0;
		}
		m_spi_clk = clk;
	}
}

void nes_dreamtech_device::write_l(offs_t offset, u8 data)
{
	LOG("dreamtech write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
		prg16_89ab(data);
}

/*-------------------------------------------------

 Bootleg Board by Fukutake

 Games: Study Box

 iNES: mapper 186

 In MAME: Unsupported.

 -------------------------------------------------*/

void nes_fukutake_device::write_l(offs_t offset, uint8_t data)
{
	LOG("fukutake write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	if (offset >= 0x200 && offset < 0x400)
	{
		if (offset & 1)
			prg16_89ab(data);
		else
			m_latch = data >> 6;
	}
	else if (offset >= 0x400 && offset < 0xf00)
		m_ram[offset - 0x400] = data;
}

uint8_t nes_fukutake_device::read_l(offs_t offset)
{
	LOG("fukutake read_l, offset: %04x\n", offset);
	offset += 0x100;

	if (offset >= 0x200 && offset < 0x400)
	{
		if (offset == 0x200 || offset == 0x201 || offset == 0x203)
			return 0x00;
		else if (offset == 0x202)
			return 0x40;
		else
			return 0xff;
	}
	else if (offset >= 0x400 && offset < 0xf00)
		return m_ram[offset - 0x400];

	return 0;
}

void nes_fukutake_device::write_m(offs_t offset, uint8_t data)
{
	LOG("fukutake write_m, offset: %04x, data: %02x\n", offset, data);
	m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
}

uint8_t nes_fukutake_device::read_m(offs_t offset)
{
	LOG("fukutake read_m, offset: %04x\n", offset);
	return m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)];
}

/*-------------------------------------------------

 Bootleg Board by Future Media

 Games: Crayon Shin-chan (C), San Guo Zhi 4 - Chi Bi Feng Yun

 iNES: mapper 117

 The IRQ counter is clocked by the filtered rising edges of PPU A12 (Mesen2, Nestopia; NESdev only says it selects the
 scanline "minus $14 (?)"). The register decode is Mesen2's ($c001 latch, $c002 acknowledge, $c003 load, $d000 mirroring,
 $e000 enable), NESdev lists other addresses. Not run against hardware.

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_futuremedia_device::ppu_a12_rise()
{
	if (m_irq_enable && m_irq_count)
	{
		m_irq_count--;
		if (!m_irq_count)
			set_irq_line(ASSERT_LINE);
	}
}

void nes_futuremedia_device::hblank_irq(int scanline, bool vblank, bool blanked)
{
	if (m_a12_driven)
		return;

	//  if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
	{
		if (m_irq_enable && m_irq_count)
		{
			m_irq_count--;
			if (!m_irq_count)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_futuremedia_device::write_h(offs_t offset, u8 data)
{
	LOG("futuremedia write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset)
	{
		case 0x0000:
		case 0x0001:
		case 0x0002:
		case 0x0003:
			prg8_x(offset & 0x03, data);
			break;
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
		case 0x2004:
		case 0x2005:
		case 0x2006:
		case 0x2007:
			chr1_x(offset & 0x07, data, CHRROM);
			break;
		case 0x4001:
			m_irq_count_latch = data;
			break;
		case 0x4002:
			set_irq_line(CLEAR_LINE);
			break;
		case 0x4003:
			m_irq_count = m_irq_count_latch;
			break;
		case 0x5000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x6000:
			m_irq_enable = data & 0x01;
			set_irq_line(CLEAR_LINE);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by Magic Series

 Games: Magic Dragon

 Very simple mapper: writes to 0x8000-0xffff set prg32 and chr8
 banks

 iNES: mapper 107

 In MAME: Supported.

 -------------------------------------------------*/

void nes_magseries_device::write_h(offs_t offset, u8 data)
{
	LOG("magseries write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(data >> 1);
	chr8(data, CHRROM);
}

/*-------------------------------------------------

 Open Corp DAOU306 board

 Games: Metal Force (K)

 iNES: mapper 156

 In MAME: Supported.

 Notes: Metal Force and Buzz & Waldog only use the first 4
 regs and no mirroring. Janggun ui Adeul uses all features

 -------------------------------------------------*/

void nes_daou306_device::write_h(offs_t offset, u8 data)
{
	LOG("daou306 write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x4000 && offset < 0x4010)
	{
		int reg = bitswap<4>(offset, 2, 3, 1, 0);
		m_reg[reg] = data;
		reg &= 0x07;
		chr1_x(reg, m_reg[reg] | (m_reg[reg | 8] << 8), CHRROM);
	}
	else if (offset == 0x4010)
		prg16_89ab(data);
	else if (offset == 0x4014)
		set_nt_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Board UNL-CC-21

 Games: Mi Hun Che

 iNES: mapper 27 (overlaps with incompatible World Hero)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cc21_device::write_h(offs_t offset, u8 data)
{
	LOG("cc21 write_h, offset: %04x, data: %02x\n", offset, data);

	set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr4_0(BIT(offset, 0), CHRROM);
	chr4_4(BIT(offset, 0), CHRROM);
}

/*-------------------------------------------------

 Bootleg Board for Xiao Zhuan Yuan

 Games: Shu Qi Yu - Zhi Li Xiao Zhuan Yuan

 Very simple mapper: writes to 0x5ff1 set prg32 (to data>>1),
 while writes to 0x5ff2 set chr8.

 Legacy iNES: mapper 176 (iNES 1.0 images that carry no submapper).
 This board is unrelated to the NES 2.0 mapper 176 8025 enhanced-MMC3
 family (Super24in1SC03, FK23C/FK23CA, FS005, JX9003B, ...): NES 2.0
 images are routed by submapper to the MMC3 clone implementations in
 mmc3_clones.cpp, see nes_ines.hxx.

 In MAME: Supported.

 -------------------------------------------------*/

void nes_xiaozy_device::write_l(offs_t offset, uint8_t data)
{
	LOG("xiaozy write_l, offset: %04x, data: %02x\n", offset, data);

	switch (offset)
	{
		case 0x1ef1:    /* 0x5ff1 */
			prg32(data >> 1);
			break;
		case 0x1ef2:    /* 0x5ff2 */
			chr8(data, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 UNL-EDU2000

 -------------------------------------------------*/

void nes_edu2k_device::write_h(offs_t offset, uint8_t data)
{
	LOG("edu2k write_h, offset: %04x, data: %02x\n", offset, data);

	prg32(data & 0x1f);
	m_latch = BIT(data, 6, 2);
}

void nes_edu2k_device::write_m(offs_t offset, uint8_t data)
{
	LOG("edu2k write_m, offset: %04x, data: %02x\n", offset, data);
	m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
}

uint8_t nes_edu2k_device::read_m(offs_t offset)
{
	LOG("edu2k read_m, offset: %04x\n", offset);
	return m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)];
}

/*-------------------------------------------------

 Bootleg Board JY830623C

 Games: Mortal Kombat II, Street Fighter III, Super Mario
 Kart Rider

 iNES: mapper 91

 In MAME: Partially supported.

 Submapper 0 (JY830623C, YY840238C): the IRQ counter counts 64 unfiltered rises of PPU A12 after $7007 and then
 asserts the IRQ until $7006 stops and clears it; the mirroring is hard-wired vertical. Submapper 1 (EJ-006-1, Super
 Fighter III): $6004/$6005 select horizontal/vertical mirroring and $6006/$6007 load a 16-bit IRQ counter that M2
 counts down by five every fourth cycle; the IRQ is asserted while it is enabled and the counter is 0 or below
 (NESdev INES Mapper 091; the counter details as puNES has them).

 -------------------------------------------------*/

void nes_jy830623c_device::ppu_a12_level(int state)
{
	// submapper 0: a fixed count of 64 unfiltered PPU A12 rises (every 0 -> 1 change of the raw level)
	const bool rise = state && !m_a12_last;
	m_a12_last = state ? 1 : 0;
	if (rise && m_irq_enable && (m_a12_count < 64) && (++m_a12_count == 64))
	{
		LOG("irq fired after 64 A12 rises\n");
		set_irq_line(ASSERT_LINE);
	}
}

void nes_jy830623c_device::m2_w(int state, offs_t address, bool write)
{
	// submapper 1: every fourth M2 cycle the counter goes down by five
	if (!state)
		return;
	m_m2_prescaler = (m_m2_prescaler + 1) & 3;
	if (!m_m2_prescaler)
	{
		m_m2_count -= 5;
		if (m_irq_enable && (m_m2_count <= 0))
			set_irq_line(ASSERT_LINE);
	}
}

void nes_jy830623c_device::update_banks()
{
	prg8_89((m_latch & 0x06) << 3 | m_reg[4]);
	prg8_ab((m_latch & 0x06) << 3 | m_reg[5]);
	prg16_cdef((m_latch & 0x06) << 2 | 0x07);

	for (int i = 0; i < 4; i++)
		chr2_x(2 * i, (m_latch & 1) << 8 | m_reg[i], CHRROM);
}

void nes_jy830623c_device::write_m(offs_t offset, u8 data)
{
	LOG("jy830623c write_m, offset: %04x, data: %02x\n", offset, data);

	// registers mask $F003 on submapper 0 and $F007 on submapper 1
	switch (offset & ((m_submapper == 1) ? 0x1007 : 0x1003))
	{
		case 0x0000:
		case 0x0001:
		case 0x0002:
		case 0x0003:
		case 0x1000:
		case 0x1001:
			m_reg[bitswap<3>(offset, 12, 1, 0)] = data;
			update_banks();
			break;
		case 0x0004:            // submapper 1
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;
		case 0x0005:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case 0x0006:
			m_m2_count = (m_m2_count & 0xff00) | data;
			break;
		case 0x0007:
			m_m2_count = (m_m2_count & 0x00ff) | (data << 8);
			break;
		case 0x1002:            // IRQ stop/acknowledge ($7006 on submapper 1)
		case 0x1006:
			m_irq_enable = 0;
			m_a12_count = 0;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x1003:            // IRQ start/reset ($7007 on submapper 1)
		case 0x1007:
			// NESdev INES Mapper 091, "IRQ Start/Reset": the count of A12 rises starts again from 0
			m_irq_enable = 1;
			m_a12_count = 0;
			m_m2_prescaler = 3;
			set_irq_line(CLEAR_LINE);
			break;
	}
}

void nes_jy830623c_device::write_h(offs_t offset, u8 data)
{
	LOG("jy830623c write_h, offset: %04x, data: %02x\n", offset, data);

	if ((m_submapper == 0) && (offset < 0x2000))     // outer bank register, submapper 0 only
	{
		m_latch = offset;
		update_banks();
	}
}

/*-------------------------------------------------

 UNL-43272

 Games: Gaau Hok Gwong Cheung

 In MAME: Preliminary Supported

 -------------------------------------------------*/

void nes_43272_device::write_h(offs_t offset, uint8_t data)
{
	LOG("unl_43272 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((m_latch & 0x81) == 0x81)
		prg32(BIT(m_latch, 3, 3));

	m_latch = offset & 0xffff;
}


uint8_t nes_43272_device::read_h(offs_t offset)
{
	uint8_t mask = (m_latch & 0x400) ? 0xfe : 0xff;
	LOG("unl_43272 read_h, offset: %04x\n", offset);

	return hi_access_rom(offset & mask);
}

/*-------------------------------------------------

 UNL-EH8813A

 Games: Dr. Mario II, 1996 English CAI 3 in 1,
 Elementary School Math CAI

 NES 2.0: mapper 519

 In MAME: Supported.

 -------------------------------------------------*/

void nes_eh8813a_device::write_h(offs_t offset, u8 data)
{
	LOG("unl_eh8813a write_h, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_latch, 8))
	{
		m_latch = offset;
		m_reg = data;

		u8 bank = m_latch & 0x3f;
		u8 mode = !BIT(m_latch, 7);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);

		set_nt_mirroring(BIT(m_reg, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}

	chr8((m_reg & 0x7c) | (data & 0x03), CHRROM);
}

u8 nes_eh8813a_device::read_h(offs_t offset)
{
	LOG("unl_eh8813a read_h, offset: %04x\n", offset);

	if (BIT(m_latch, 6))
		offset = (offset & ~0x0f) | m_jumper;  // TODO: jumper setting that controls which menu appears is 0 for now
	return hi_access_rom(offset);
}


/*-------------------------------------------------

 FUJIYA Board - NROM with a protection register

 Games: Fujiya Famicasse Series 3 - Thinking Game Collection

 NROM-256 and 8 KiB of CHR. A write to $6502 or $7000 sets a
 register to (data << 1) & 0x80, and a read of $7001 or $7777
 returns it with the rest of the bits open bus (the address high
 byte for an absolute read). Nestopia (board FUJIYA NROM
 +SECURITY), Mesen2 and FCEUmm implement the same registers; no
 NESdev page was found, so the board's own purpose is unknown.

 iNES: mapper 170

 In MAME: Supported.

 -------------------------------------------------*/

void nes_fujiya_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_fujiya_device::pcb_reset()
{
	prg32(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_fujiya_device::write_m(offs_t offset, uint8_t data)
{
	LOG("fujiya write_m, offset: %04x, data: %02x\n", offset, data);
	offset += 0x6000;

	if (offset == 0x6502 || offset == 0x7000)
		m_latch = (data & 0x40) << 1;
}

uint8_t nes_fujiya_device::read_m(offs_t offset)
{
	LOG("fujiya read_m, offset: %04x\n", offset);
	offset += 0x6000;

	if (offset == 0x7001 || offset == 0x7777)
		return m_latch | (get_open_bus() & 0x7f);

	return get_open_bus();
}


/*-------------------------------------------------

 UNL-BJ-56 board

 Games: Sangoku III (bootleg of Namco's Sangokushi: Chuugen
 no Hasha)

 NESdev NES 2.0 Mapper 526 (register mask "probably $800F"):
 - $8000-$8007: 1 KiB CHR-ROM banks at PPU $0000-$1FFF
 - $8008-$800B: 8 KiB PRG-ROM banks at CPU $8000-$FFFF
 - $800D, $800F: one acknowledges the IRQ, the other resets
   the IRQ counter to 0; games always write both, one after
   the other, so it is not known which does which. Here
   either write does both.
 - The IRQ counter counts every M2 cycle. /IRQ is low while
   its bit 12 is set: an IRQ 4096 cycles after a reset of the
   counter that acknowledges itself 4096 cycles later.
 Mirroring seems to be hard-wired (to vertical).
 The battery-backed RAM at $6000-$7FFF is the header's
 PRG-NVRAM.

 Not documented, so not emulated: the edge of M2 the counter
 counts (the falling edge here), the counter's width (bit 12
 behaves the same for any width above 12 bits), the power-on
 banks (here the last two 8 KiB banks at $C000-$FFFF, so the
 reset vector is reached).

 NES 2.0: mapper 526

 In MAME: Supported.

 -------------------------------------------------*/

nes_bj56_device::nes_bj56_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BJ56, tag, owner, clock), m_irq_count(0)
{
}

void nes_bj56_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_count));
}

void nes_bj56_device::pcb_reset()
{
	prg8_89(0);
	prg8_ab(1);
	prg8_cd((m_prg_chunks << 1) - 2);
	prg8_ef((m_prg_chunks << 1) - 1);
	chr8(0, CHRROM);
	set_nt_mirroring(PPU_MIRROR_VERT);
	m_irq_count = 0;
	set_irq_line(CLEAR_LINE);
}

void nes_bj56_device::m2_w(int state, offs_t address, bool write)
{
	if (state)
		return;
	m_irq_count++;
	set_irq_line(BIT(m_irq_count, 12) ? ASSERT_LINE : CLEAR_LINE);
}

void nes_bj56_device::write_h(offs_t offset, u8 data)
{
	LOG("bj-56 write_h, offset: %04x, data: %02x\n", offset, data);

	const u8 reg = offset & 0x0f;
	if (reg < 8)
		chr1_x(reg, data, CHRROM);
	else if (reg < 12)
		prg8_x(reg - 8, data);
	else if (reg == 0x0d || reg == 0x0f)
	{
		m_irq_count = 0;
		set_irq_line(CLEAR_LINE);
	}
}
