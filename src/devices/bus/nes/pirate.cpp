// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for pirate cart PCBs


 Here we emulate the various PCBs used by Asian & Korean pirate games

 TODO:
 - Are the scrolling glitches (check status bar) in Magic Dragon correct? FWIW, NEStopia behaves similarly

 ***********************************************************************************************************/


#include "emu.h"
#include "pirate.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE
#include "screen.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_AGCI_50282,  nes_agci_device,        "nes_agci50282",   "NES Cart AGCI 50282 PCB")
DEFINE_DEVICE_TYPE(NES_DREAMTECH01, nes_dreamtech_device,   "nes_dreamtech",   "NES Cart Dreamtech01 PCB")
DEFINE_DEVICE_TYPE(NES_FUKUTAKE,    nes_fukutake_device,    "nes_fukutake",    "NES Cart Fukutake Study Box PCB")
DEFINE_DEVICE_TYPE(NES_FUTUREMEDIA, nes_futuremedia_device, "nes_futuremedia", "NES Cart FutureMedia PCB")
DEFINE_DEVICE_TYPE(NES_MAGSERIES,   nes_magseries_device,   "nes_magseries",   "NES Cart Magical Series PCB")
DEFINE_DEVICE_TYPE(NES_DAOU306,     nes_daou306_device,     "nes_daou306",     "NES Cart Daou 306 PCB")
DEFINE_DEVICE_TYPE(NES_CC21,        nes_cc21_device,        "nes_cc21",        "NES Cart CC-21 PCB")
DEFINE_DEVICE_TYPE(NES_XIAOZY,      nes_xiaozy_device,      "nes_xiaozy",      "NES Cart Xiao Zhuan Yuan PCB")
DEFINE_DEVICE_TYPE(NES_EDU2K,       nes_edu2k_device,       "nes_edu2k",       "NES Cart Educational Computer 2000 PCB")
DEFINE_DEVICE_TYPE(NES_JY830623C,   nes_jy830623c_device,   "nes_jy830623c",   "NES Cart JY830623C PCB")
DEFINE_DEVICE_TYPE(NES_43272,       nes_43272_device,       "nes_43272",       "NES Cart UNL-43272 PCB")
DEFINE_DEVICE_TYPE(NES_EH8813A,     nes_eh8813a_device,     "nes_eh8813a",     "NES Cart UNL-EH8813A PCB")


nes_agci_device::nes_agci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_AGCI_50282, tag, owner, clock)
{
}

nes_dreamtech_device::nes_dreamtech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_DREAMTECH01, tag, owner, clock)
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

nes_daou306_device::nes_daou306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
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
	: nes_nrom_device(mconfig, NES_JY830623C, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0)
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




void nes_agci_device::device_start()
{
	common_start();
}

void nes_agci_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_dreamtech_device::device_start()
{
	common_start();
}

void nes_dreamtech_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(8);
	chr8(0, m_chr_source);
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
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 2);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_LOW);

	memset(m_reg, 0, sizeof(m_reg));
}

void nes_xiaozy_device::device_start()
{
	common_start();
}

void nes_xiaozy_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_jy830623c_device::pcb_reset()
{
	m_latch = 0;
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	update_banks();
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
}

void nes_43272_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_43272_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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

 In MESS: Supported.

 -------------------------------------------------*/

void nes_agci_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("agci write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	uint8_t temp = account_bus_conflict(offset, 0xff);
	data = (data & temp) | (temp & 1);

	chr8(data >> 4, CHRROM);
	prg32(data);
}

/*-------------------------------------------------

 Board DREAMTECH01

 Games: Korean Igo

 NES 2.0: mapper 521

 In MAME: Supported.

 -------------------------------------------------*/

void nes_dreamtech_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("dreamtech write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1020)   /* 0x5020 */
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
	LOG_MMC(("fukutake write_l, offset: %04x, data: %02x\n", offset, data));
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
	LOG_MMC(("fukutake read_l, offset: %04x\n", offset));
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
	LOG_MMC(("fukutake write_m, offset: %04x, data: %02x\n", offset, data));
	m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
}

uint8_t nes_fukutake_device::read_m(offs_t offset)
{
	LOG_MMC(("fukutake read_m, offset: %04x\n", offset));
	return m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)];
}

/*-------------------------------------------------

 Bootleg Board by Future Media

 Games: Crayon Shin-chan (C), San Guo Zhi 4 - Chi Bi Feng Yun

 iNES: mapper 117

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_futuremedia_device::hblank_irq(int scanline, int vblank, int blanked)
{
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
	LOG_MMC(("futuremedia write_h, offset: %04x, data: %02x\n", offset, data));

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
	LOG_MMC(("magseries write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 1);
	chr8(data, CHRROM);
}

/*-------------------------------------------------

 Open Corp DAOU306 board

 Games: Metal Force (K)

 iNES: mapper 156

 In MESS: Supported.

 Notes: Metal Force and Buzz & Waldog only use the first 4
 regs and no mirroring. Janggun ui Adeul uses all features

 -------------------------------------------------*/

void nes_daou306_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("daou306 write_h, offset: %04x, data: %02x\n", offset, data));
	int reg = BIT(offset, 2) ? 8 : 0;

	switch (offset)
	{
		case 0x4000:
		case 0x4004:
			m_reg[reg + 0] = data;
			chr1_0(m_reg[0] | (m_reg[8] << 8), CHRROM);
			break;
		case 0x4001:
		case 0x4005:
			m_reg[reg + 1] = data;
			chr1_1(m_reg[1] | (m_reg[9] << 8), CHRROM);
			break;
		case 0x4002:
		case 0x4006:
			m_reg[reg + 2] = data;
			chr1_2(m_reg[2] | (m_reg[10] << 8), CHRROM);
			break;
		case 0x4003:
		case 0x4007:
			m_reg[reg + 3] = data;
			chr1_3(m_reg[3] | (m_reg[11] << 8), CHRROM);
			break;
		case 0x4008:
		case 0x400c:
			m_reg[reg + 4] = data;
			chr1_4(m_reg[4] | (m_reg[12] << 8), CHRROM);
			break;
		case 0x4009:
		case 0x400d:
			m_reg[reg + 5] = data;
			chr1_5(m_reg[5] | (m_reg[13] << 8), CHRROM);
			break;
		case 0x400a:
		case 0x400e:
			m_reg[reg + 6] = data;
			chr1_6(m_reg[6] | (m_reg[14] << 8), CHRROM);
			break;
		case 0x400b:
		case 0x400f:
			m_reg[reg + 7] = data;
			chr1_7(m_reg[7] | (m_reg[15] << 8), CHRROM);
			break;
		case 0x4010:
			prg16_89ab(data);
			break;
		case 0x4014:
			if (data & 1)
				set_nt_mirroring(PPU_MIRROR_HORZ);
			else
				set_nt_mirroring(PPU_MIRROR_VERT);
			break;
	}
}

/*-------------------------------------------------

 Board UNL-CC-21

 Games: Mi Hun Che

 iNES: mapper 27 (overlaps with incompatible World Hero)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cc21_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("cc21 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(offset, 0) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr4_0(BIT(offset, 0), CHRROM);
	chr4_4(BIT(offset, 0), CHRROM);
}

/*-------------------------------------------------

 Bootleg Board for Xiao Zhuan Yuan

 Games: Shu Qi Yu - Zhi Li Xiao Zhuan Yuan

 Very simple mapper: writes to 0x5ff1 set prg32 (to data>>1),
 while writes to 0x5ff2 set chr8

 iNES: mapper 176

 In MESS: Supported.

 -------------------------------------------------*/

void nes_xiaozy_device::write_l(offs_t offset, uint8_t data)
{
	LOG_MMC(("xiaozy write_l, offset: %04x, data: %02x\n", offset, data));

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
	LOG_MMC(("edu2k write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(data & 0x1f);
	m_latch = (data & 0xc0) >> 6;
}

void nes_edu2k_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("edu2k write_m, offset: %04x, data: %02x\n", offset, data));
	m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
}

uint8_t nes_edu2k_device::read_m(offs_t offset)
{
	LOG_MMC(("edu2k read_m, offset: %04x\n", offset));
	return m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)];
}

/*-------------------------------------------------

 Bootleg Board JY830623C

 Games: Mortal Kombat II, Street Fighter III, Super Mario
 Kart Rider

 iNES: mapper 91

 In MAME: Partially supported.

 FIXME: IRQ is fixed length but not every 7 scanlines
 as done here. Rather it triggers once every 64 rises
 of PPU A12. Various games have very obvious raster-
 split issues from this bug. Also note there is
 another submapper with cycle-based IRQ.

 -------------------------------------------------*/

void nes_jy830623c_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
	{
		int prior_count = m_irq_count;
		if (m_irq_count)
			m_irq_count--;
		else
			m_irq_count = m_irq_count_latch;

		if (m_irq_enable && !blanked && !m_irq_count && prior_count)
		{
			LOG_MMC(("irq fired, scanline: %d\n", scanline));
			set_irq_line(ASSERT_LINE);
		}
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
	LOG_MMC(("jy830623c write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x1003)
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
		case 0x1002:
			m_irq_enable = 0;
			m_irq_count = 0;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x1003:
			m_irq_enable = 1;
			m_irq_count = 7;
			break;
	}
}

void nes_jy830623c_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("jy830623c write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x2000)
	{
		m_latch = offset;
		update_banks();
	}
}

/*-------------------------------------------------

 UNL-43272

 Games: Gaau Hok Gwong Cheung

 In MESS: Preliminary Supported

 -------------------------------------------------*/

void nes_43272_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("unl_43272 write_h, offset: %04x, data: %02x\n", offset, data));

	if ((m_latch & 0x81) == 0x81)
		prg32((m_latch & 0x38) >> 3);

	m_latch = offset & 0xffff;
}


uint8_t nes_43272_device::read_h(offs_t offset)
{
	uint8_t mask = (m_latch & 0x400) ? 0xfe : 0xff;
	LOG_MMC(("unl_43272 read_h, offset: %04x\n", offset));

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
	LOG_MMC(("unl_eh8813a write_h, offset: %04x, data: %02x\n", offset, data));

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
	LOG_MMC(("unl_eh8813a read_h, offset: %04x\n", offset));

	if (BIT(m_latch, 6))
		offset = (offset & ~0x0f) | m_jumper;  // TODO: jumper setting that controls which menu appears is 0 for now
	return hi_access_rom(offset);
}


#ifdef UNUSED_FUNCTION
/*-------------------------------------------------

 FUJIYA Board - mapper 170 according to NEStopia

 Is this possibly for the Fujiya Famikase series
 of educational titles? Is there any dump around?

 -------------------------------------------------*/

void nes_fujiya_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_fujiya_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_fujiya_device::write_m(offs_t offset, uint8_t data)
{
	LOG_MMC(("fujiya write_m, offset: %04x, data: %02x\n", offset, data));
	offset += 0x6000;

	if (offset == 0x6502 || offset == 0x7000)
		m_latch = (data & 0x40) << 1;
}

uint8_t nes_fujiya_device::read_m(offs_t offset)
{
	LOG_MMC(("fujiya read_m, offset: %04x\n", offset));
	offset += 0x6000;

	if (offset == 0x7001 || offset == 0x7777)
		return m_latch | ((offset >> 8) & 0x7f);

	return get_open_bus();  // open bus
}
#endif
