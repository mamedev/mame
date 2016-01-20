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

#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access PPU_BOTTOM_VISIBLE_SCANLINE


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_AGCI_50282 = &device_creator<nes_agci_device>;
const device_type NES_DREAMTECH01 = &device_creator<nes_dreamtech_device>;
const device_type NES_FUKUTAKE = &device_creator<nes_fukutake_device>;
const device_type NES_FUTUREMEDIA = &device_creator<nes_futuremedia_device>;
const device_type NES_MAGSERIES = &device_creator<nes_magseries_device>;
const device_type NES_DAOU306 = &device_creator<nes_daou306_device>;
const device_type NES_SUBOR0 = &device_creator<nes_subor0_device>;
const device_type NES_SUBOR1 = &device_creator<nes_subor1_device>;
const device_type NES_CC21 = &device_creator<nes_cc21_device>;
const device_type NES_XIAOZY = &device_creator<nes_xiaozy_device>;
const device_type NES_EDU2K = &device_creator<nes_edu2k_device>;
const device_type NES_T230 = &device_creator<nes_t230_device>;
const device_type NES_MK2 = &device_creator<nes_mk2_device>;
const device_type NES_WHERO = &device_creator<nes_whero_device>;
const device_type NES_43272 = &device_creator<nes_43272_device>;
const device_type NES_TF1201 = &device_creator<nes_tf1201_device>;
const device_type NES_CITYFIGHT = &device_creator<nes_cityfight_device>;


nes_agci_device::nes_agci_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_AGCI_50282, "NES Cart AGCI 50282 PCB", tag, owner, clock, "nes_agci50282", __FILE__)
{
}

nes_dreamtech_device::nes_dreamtech_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_DREAMTECH01, "NES Cart Dreamtech01 PCB", tag, owner, clock, "nes_dreamtech", __FILE__)
{
}

nes_fukutake_device::nes_fukutake_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_FUKUTAKE, "NES Cart Fukutake Study Box PCB", tag, owner, clock, "nes_fukutake", __FILE__), m_latch(0)
				{
}

nes_futuremedia_device::nes_futuremedia_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_FUTUREMEDIA, "NES Cart FutureMedia PCB", tag, owner, clock, "nes_futuremedia", __FILE__), m_irq_count(0), m_irq_count_latch(0), m_irq_clear(0), m_irq_enable(0)
				{
}

nes_magseries_device::nes_magseries_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_MAGSERIES, "NES Cart Magical Series PCB", tag, owner, clock, "nes_magseries", __FILE__)
{
}

nes_daou306_device::nes_daou306_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_DAOU306, "NES Cart Daou 306 PCB", tag, owner, clock, "nes_daou306", __FILE__)
{
}

nes_subor0_device::nes_subor0_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUBOR0, "NES Cart Subor Type 0 PCB", tag, owner, clock, "nes_subor0", __FILE__)
{
}

nes_subor1_device::nes_subor1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_SUBOR1, "NES Cart Subor Type 1 PCB", tag, owner, clock, "nes_subor1", __FILE__)
{
}

nes_cc21_device::nes_cc21_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CC21, "NES Cart CC-21 PCB", tag, owner, clock, "nes_cc21", __FILE__)
{
}

nes_xiaozy_device::nes_xiaozy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_XIAOZY, "NES Cart Xiao Zhuan Yuan PCB", tag, owner, clock, "nes_xiaozy", __FILE__)
{
}

nes_edu2k_device::nes_edu2k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_EDU2K, "NES Cart Educational Computer 2000 PCB", tag, owner, clock, "nes_edu2k", __FILE__), m_latch(0)
				{
}

nes_t230_device::nes_t230_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_T230, "NES Cart T-230 PCB", tag, owner, clock, "nes_t230", __FILE__), m_irq_count(0), m_irq_count_latch(0), m_irq_mode(0), m_irq_enable(0), m_irq_enable_latch(0)
				{
}

nes_mk2_device::nes_mk2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_MK2, "NES Cart Mortal Kombat 2 PCB", tag, owner, clock, "nes_mk2", __FILE__), m_irq_count(0), m_irq_count_latch(0), m_irq_clear(0), m_irq_enable(0)
				{
}

nes_whero_device::nes_whero_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_WHERO, "NES Cart World Heroes PCB", tag, owner, clock, "nes_whero", __FILE__), m_reg(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_enable_latch(0)
				{
}

nes_43272_device::nes_43272_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_43272, "NES Cart UNL-43272 PCB", tag, owner, clock, "nes_43272", __FILE__), m_latch(0)
				{
}

nes_tf1201_device::nes_tf1201_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TF1201, "NES Cart UNL-TF1201 PCB", tag, owner, clock, "nes_tf1201", __FILE__), m_prg(0), m_swap(0), m_irq_count(0), m_irq_enable(0), m_irq_enable_latch(0)
				{
}

nes_cityfight_device::nes_cityfight_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TF1201, "NES Cart City Fighter PCB", tag, owner, clock, "nes_cityfight", __FILE__), m_prg_reg(0), m_prg_mode(0), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr)
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
	save_item(NAME(m_irq_clear));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_futuremedia_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_clear = 0;
	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
}

void nes_magseries_device::device_start()
{
	common_start();
}

void nes_magseries_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
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

void nes_subor0_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_subor0_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0x20);
	chr8(0, m_chr_source);

	memset(m_reg, 0, sizeof(m_reg));
}

void nes_subor1_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_subor1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0x07);
	chr8(0, m_chr_source);

	memset(m_reg, 0, sizeof(m_reg));
}

void nes_cc21_device::device_start()
{
	common_start();
}

void nes_cc21_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
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

void nes_t230_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_t230_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_mode = 0;
	m_irq_enable = m_irq_enable_latch = 0;
	m_irq_count = m_irq_count_latch = 0;

	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
}

void nes_mk2_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_clear));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_mk2_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 1);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_irq_clear = 0;
	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
}


void nes_whero_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_whero_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0x100, m_chr_source);

	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;

	m_reg = 0;
	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
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

void nes_tf1201_device::device_start()
{
	common_start();
	save_item(NAME(m_prg));
	save_item(NAME(m_swap));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_tf1201_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
	m_prg = 0;
	m_swap = 0;
	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
}

void nes_cityfight_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_prg_reg));
	save_item(NAME(m_prg_mode));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_cityfight_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));
	m_prg_reg = 0;
	m_prg_mode = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
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

WRITE8_MEMBER(nes_agci_device::write_h)
{
	LOG_MMC(("agci write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	UINT8 temp = account_bus_conflict(offset, 0xff);
	data = (data & temp) | (temp & 1);

	chr8(data >> 4, CHRROM);
	prg32(data);
}

/*-------------------------------------------------

 Board DREAMTECH01

 Games: Korean Igo

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_dreamtech_device::write_l)
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

 In MESS: Unsupported.


 -------------------------------------------------*/

WRITE8_MEMBER(nes_fukutake_device::write_l)
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

READ8_MEMBER(nes_fukutake_device::read_l)
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

WRITE8_MEMBER(nes_fukutake_device::write_m)
{
	LOG_MMC(("fukutake write_m, offset: %04x, data: %02x\n", offset, data));
	m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
}

READ8_MEMBER(nes_fukutake_device::read_m)
{
	LOG_MMC(("fukutake read_m, offset: %04x\n", offset));
	return m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)];
}

/*-------------------------------------------------

 Bootleg Board by Future Media

 Games: Crayon Shin-chan (C), San Guo Zhi 4 - Chi Bi Feng Yun

 iNES: mapper 117

 In MESS: Unsupported.

 -------------------------------------------------*/

void nes_futuremedia_device::hblank_irq(int scanline, int vblank, int blanked)
{
	//  if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		if (m_irq_enable && m_irq_count)
		{
			m_irq_count--;
			if (!m_irq_count)
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
	}
}

WRITE8_MEMBER(nes_futuremedia_device::write_h)
{
	LOG_MMC(("futuremedia write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
			prg8_ab(data);
			break;
		case 0x0002:
			prg8_cd(data);
			break;
		case 0x0003:
			prg8_ef(data);
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

		case 0x5000:
			set_nt_mirroring(BIT(data, 0) ?  PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x4001:
			m_irq_count_latch = data;
			break;
		case 0x4002:
			// IRQ cleared
			break;
		case 0x4003:
			m_irq_count = m_irq_count_latch;
			break;
		case 0x6000:
			m_irq_enable = data & 0x01;
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by Magic Series

 Games: Magic Dragon

 Very simple mapper: writes to 0x8000-0xffff set prg32 and chr8
 banks

 iNES: mapper 107

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_magseries_device::write_h)
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

WRITE8_MEMBER(nes_daou306_device::write_h)
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

 Subor bootleg board Type 0

 iNES: mapper 167

 -------------------------------------------------*/

WRITE8_MEMBER(nes_subor0_device::write_h)
{
	UINT8 subor_helper1, subor_helper2;
	LOG_MMC(("subor0 write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[(offset >> 13) & 0x03] = data;
	subor_helper1 = ((m_reg[0] ^ m_reg[1]) << 1) & 0x20;
	subor_helper2 = ((m_reg[2] ^ m_reg[3]) << 0) & 0x1f;

	if (m_reg[1] & 0x08)
	{
		subor_helper1 += subor_helper2 & 0xfe;
		subor_helper2 = subor_helper1;
		subor_helper1 += 1;
	}
	else if (m_reg[1] & 0x04)
	{
		subor_helper2 += subor_helper1;
		subor_helper1 = 0x1f;
	}
	else
	{
		subor_helper1 += subor_helper2;
		subor_helper2 = 0x20;
	}

	prg16_89ab(subor_helper1);
	prg16_cdef(subor_helper2);
}

/*-------------------------------------------------

 Subor bootleg board Type 1

 iNES: mapper 166

 -------------------------------------------------*/

WRITE8_MEMBER(nes_subor1_device::write_h)
{
	UINT8 subor_helper1, subor_helper2;
	LOG_MMC(("subor1 write_h, offset: %04x, data: %02x\n", offset, data));

	m_reg[(offset >> 13) & 0x03] = data;
	subor_helper1 = ((m_reg[0] ^ m_reg[1]) << 1) & 0x20;
	subor_helper2 = ((m_reg[2] ^ m_reg[3]) << 0) & 0x1f;

	if (m_reg[1] & 0x08)
	{
		subor_helper1 += subor_helper2 & 0xfe;
		subor_helper2 = subor_helper1;
		subor_helper2 += 1;
	}
	else if (m_reg[1] & 0x04)
	{
		subor_helper2 += subor_helper1;
		subor_helper1 = 0x1f;
	}
	else
	{
		subor_helper1 += subor_helper2;
		subor_helper2 = 0x07;
	}

	prg16_89ab(subor_helper1);
	prg16_cdef(subor_helper2);
}

/*-------------------------------------------------

 Board UNL-CC-21

 Games: Mi Hun Che

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_cc21_device::write_h)
{
	LOG_MMC(("cc21 write_h, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(offset, 1) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8((offset & 0x01), CHRROM);
}

/*-------------------------------------------------

 Bootleg Board for Xiao Zhuan Yuan

 Games: Shu Qi Yu - Zhi Li Xiao Zhuan Yuan

 Very simple mapper: writes to 0x5ff1 set prg32 (to data>>1),
 while writes to 0x5ff2 set chr8

 iNES: mapper 176

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_xiaozy_device::write_l)
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

WRITE8_MEMBER(nes_edu2k_device::write_h)
{
	LOG_MMC(("edu2k write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(data & 0x1f);
	m_latch = (data & 0xc0) >> 6;
}

WRITE8_MEMBER(nes_edu2k_device::write_m)
{
	LOG_MMC(("edu2k write_m, offset: %04x, data: %02x\n", offset, data));
	m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
}

READ8_MEMBER(nes_edu2k_device::read_m)
{
	LOG_MMC(("edu2k read_m, offset: %04x\n", offset));
	return m_prgram[((m_latch * 0x2000) + offset) & (m_prgram.size() - 1)];
}

/*-------------------------------------------------

 Board UNL-T-230

 Games: Dragon Ball Z IV (Unl)

 This mapper appears to be similar to Konami VRC-2
 but the game has no VROM and only 1 VRAM bank, so we
 completely skip the chr bankswitch. If other games
 using the same board and using CHR should surface,
 we need to investigate this...

 In MESS: Supported

 -------------------------------------------------*/

// Identical to Konami IRQ
void nes_t230_device::hblank_irq(int scanline, int vblank, int blanked)
{
	/* Increment & check the IRQ scanline counter */
	if (m_irq_enable && (++m_irq_count == 0x100))
	{
		m_irq_count = m_irq_count_latch;
		m_irq_enable = m_irq_enable_latch;
		m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
}

WRITE8_MEMBER(nes_t230_device::write_h)
{
	LOG_MMC(("t230 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x700c)
	{
		case 0x0000:
			break;
		case 0x2000:
			prg16_89ab(data);
			break;
		case 0x1000:
		case 0x1004:
		case 0x1008:
		case 0x100c:
			switch (data & 0x03)
			{
				case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;

		case 0x7000:
			m_irq_count_latch &= ~0x0f;
			m_irq_count_latch |= data & 0x0f;
			break;
		case 0x7004:
			m_irq_count_latch &= ~0xf0;
			m_irq_count_latch |= (data << 4) & 0xf0;
			break;
		case 0x7008:
			m_irq_mode = data & 0x04;   // currently not implemented: 0 = prescaler mode / 1 = CPU mode
			m_irq_enable = data & 0x02;
			m_irq_enable_latch = data & 0x01;
			if (data & 0x02)
				m_irq_count = m_irq_count_latch;
			break;

		default:
			logerror("unl_t230_w uncaught offset: %04x value: %02x\n", offset, data);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board for MK2

 Games: Mortal Kombat II, Street Fighter III, Super Mario
 Kart Rider

 This board uses an IRQ system very similar to MMC3. We indeed
 use mapper4_irq, but there is some small glitch!

 iNES: mapper 91

 In MESS: Supported.

 -------------------------------------------------*/

// Same IRQ as MMC3
void nes_mk2_device::hblank_irq( int scanline, int vblank, int blanked )
{
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		int prior_count = m_irq_count;
		if ((m_irq_count == 0) || m_irq_clear)
			m_irq_count = m_irq_count_latch;
		else
			m_irq_count--;

		if (m_irq_enable && !blanked && (m_irq_count == 0) && (prior_count || m_irq_clear))
		{
			LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
						machine().first_screen()->vpos(), machine().first_screen()->hpos()));
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
	}
	m_irq_clear = 0;
}

WRITE8_MEMBER(nes_mk2_device::write_m)
{
	LOG_MMC(("mk2 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x1000)
	{
		case 0x0000:
			switch (offset & 0x03)
			{
				case 0x00: chr2_0(data, CHRROM); break;
				case 0x01: chr2_2(data, CHRROM); break;
				case 0x02: chr2_4(data, CHRROM); break;
				case 0x03: chr2_6(data, CHRROM); break;
			}
			break;
		case 0x1000:
			switch (offset & 0x03)
			{
				case 0x00: prg8_89(data); break;
				case 0x01: prg8_ab(data); break;
				case 0x02: m_irq_enable = 0; m_irq_count = 0; break;
				case 0x03: m_irq_enable = 1; m_irq_count = 7; break;
			}
			break;
		default:
			logerror("mk2 write_m, uncaught addr: %04x value: %02x\n", offset + 0x6000, data);
			break;
	}
}


/*-------------------------------------------------

 UNL-WOLRDHERO board emulation


 iNES:

 -------------------------------------------------*/

// Same as Konami VRC IRQ...
void nes_whero_device::hblank_irq(int scanline, int vblank, int blanked)
{
	/* Increment & check the IRQ scanline counter */
	if (m_irq_enable && (++m_irq_count == 0x100))
	{
		m_irq_count = m_irq_count_latch;
		m_irq_enable = m_irq_enable_latch;
		m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
}

WRITE8_MEMBER(nes_whero_device::write_h)
{
	int bank, shift, mask1, mask2;
	LOG_MMC(("World Hero write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x70c3)
	{
		case 0x0000:
			if (!m_reg)
				prg8_89(data);
			else
				prg8_cd(data);
			break;

		case 0x1000:
			switch (data & 0x03)
			{
				case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;

		case 0x1002:
		case 0x1080:
			if (m_reg != (data & 2))
			{
				m_reg = data & 2;
				// swap banks!
				prg8_89(m_prg_bank[2]);
				prg8_cd(m_prg_bank[0]);
			}
			break;

		case 0x2000:
			prg8_ab(data);
			break;

		case 0x3000: case 0x3001: case 0x3002: case 0x3003:
		case 0x4000: case 0x4001: case 0x4002: case 0x4003:
		case 0x5000: case 0x5001: case 0x5002: case 0x5003:
		case 0x6000: case 0x6001: case 0x6002: case 0x6003:
			bank = ((offset & 0x7000) - 0x3000) / 0x0800 + BIT(offset, 1);
			shift = (offset & 1) << 2;
			mask1 = shift ? 0x00f : 0xff0;
			mask2 = shift ? 0xff0 : 0x00f;
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & mask1) | ((data << shift) & mask2);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;

		case 0x7000:
			m_irq_count_latch = (m_irq_count_latch & 0xf0) | (data & 0x0f);
			break;

		case 0x7001:
			m_irq_count_latch = (m_irq_count_latch & 0x0f) | ((data & 0x0f) << 4);
			break;

		case 0x7002:
			//          m_irq_mode = data & 0x04;   // currently not implemented: 0 = prescaler mode / 1 = CPU mode
			m_irq_enable = data & 0x02;
			m_irq_enable_latch = data & 0x01;
			if (data & 0x02)
				m_irq_count = m_irq_count_latch;
			break;

		case 0x7003:
			m_irq_enable = m_irq_enable_latch;
			break;
	}
}


/*-------------------------------------------------

 UNL-43272

 Games: Gaau Hok Gwong Cheung

 In MESS: Preliminary Supported

 -------------------------------------------------*/

WRITE8_MEMBER(nes_43272_device::write_h)
{
	LOG_MMC(("unl_43272 write_h, offset: %04x, data: %02x\n", offset, data));

	if ((m_latch & 0x81) == 0x81)
		prg32((m_latch & 0x38) >> 3);

	m_latch = offset & 0xffff;
}


READ8_MEMBER(nes_43272_device::read_h)
{
	UINT8 mask = (m_latch & 0x400) ? 0xfe : 0xff;
	LOG_MMC(("unl_43272 read_h, offset: %04x\n", offset));

	return hi_access_rom(offset & mask);
}


/*-------------------------------------------------

 UNL-TF1201

 Games:

 In MESS: Preliminary Supported

 -------------------------------------------------*/

void nes_tf1201_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (m_irq_enable_latch != m_irq_enable && scanline < 240)
		m_irq_count -= 8;

	if (m_irq_enable)
	{
		m_irq_count++;
		if ((m_irq_count & 0xff) == 238)
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
}

void nes_tf1201_device::update_prg()
{
	prg8_89(m_swap ? 0xff : m_prg);
	prg8_cd(m_swap ? m_prg : 0xff );
}

WRITE8_MEMBER(nes_tf1201_device::write_h)
{
	int bank;
	LOG_MMC(("unl_tf1201 write_h, offset: %04x, data: %02x\n", offset, data));

	offset = (offset & 0x7003) | ((offset & 0x0c) >> 2);    // nestopia does not OR here...

	switch (offset & 0x7003)
	{
		case 0x0000:
			m_prg = data;
			update_prg();
			break;
		case 0x1000:
			set_nt_mirroring(BIT(data, 0) ?  PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x1001:
			m_swap = data & 0x03;
			update_prg();
			break;
		case 0x2000:
			prg8_ab(data);
			break;
		case 0x3000: case 0x3001: case 0x3002: case 0x3003:
		case 0x4000: case 0x4001: case 0x4002: case 0x4003:
		case 0x5000: case 0x5001: case 0x5002: case 0x5003:
		case 0x6000: case 0x6001: case 0x6002: case 0x6003:
			bank = (((offset - 0x3000) >> 11) | (offset & 0x1)) & 0x7;
			if (offset & 2)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | ((data & 0x0f) << 0);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		case 0x7000:
			m_irq_count = (m_irq_count & 0xf0) | (data & 0x0f);
			break;
		case 0x7001:
		case 0x7003:
			m_irq_enable_latch = m_irq_enable;
			m_irq_enable = BIT(data, 1);
			break;
		case 0x7002:
			m_irq_count = (m_irq_count & 0x0f) | ((data & 0x0f) << 4);
			break;
	}
}


/*-------------------------------------------------

 UNL-CITYFIGHT

 Games:

 Additional audio register not emulated yet!

 In MESS: Preliminary Supported

 -------------------------------------------------*/

void nes_cityfight_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (!m_irq_count)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_count = 0xffff;
			}
			else
				m_irq_count--;
		}
	}
}

void nes_cityfight_device::update_prg()
{
	prg32(m_prg_reg >> 2);
	if (!m_prg_mode)
		prg8_cd(m_prg_reg);
}

WRITE8_MEMBER(nes_cityfight_device::write_h)
{
	int bank;
	LOG_MMC(("unl_cityfight write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x700c)
	{
		case 0x1000:
			switch (data & 0x03)
			{
				case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			m_prg_reg = data & 0xc;
			break;
		case 0x1004:
		case 0x1008:
		case 0x100c:
			if (offset & 0x800)
				LOG_MMC(("Extended Audio write, data %x!", data & 0x0f));//pcmwrite(0x4011, (V & 0xf) << 3);
			else
				m_prg_reg = data & 0xc;
			break;


		case 0x2000: case 0x2004: case 0x2008: case 0x200c:
			bank = 2 + BIT(offset, 3);
			if (offset & 4)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | ((data & 0x0f) << 0);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		case 0x3000: case 0x3004: case 0x3008: case 0x300c:
			bank = 4 + BIT(offset, 3);
			if (offset & 4)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | ((data & 0x0f) << 0);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		case 0x5000: case 0x5004: case 0x5008: case 0x500c:
			bank = 0 + BIT(offset, 3);
			if (offset & 4)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | ((data & 0x0f) << 0);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		case 0x6000: case 0x6004: case 0x6008: case 0x600c:
			bank = 6 + BIT(offset, 3);
			if (offset & 4)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | ((data & 0x0f) << 0);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;

		case 0x4000:
		case 0x4004:
		case 0x4008:
		case 0x400c:
			m_prg_mode = data & 1;

		case 0x7000:
			m_irq_count = (m_irq_count & 0x1e0) | ((data & 0x0f) << 1);
			break;
		case 0x7004:
			m_irq_count = (m_irq_count & 0x1e) | ((data & 0x0f) << 5);
			break;
		case 0x7008:
			m_irq_enable = BIT(data, 1);
			break;
	}
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

WRITE8_MEMBER(nes_fujiya_device::write_m)
{
	LOG_MMC(("fujiya write_m, offset: %04x, data: %02x\n", offset, data));
	offset += 0x6000;

	if (offset == 0x6502 || offset == 0x7000)
		m_latch = (data & 0x40) << 1;
}

READ8_MEMBER(nes_fujiya_device::read_m)
{
	LOG_MMC(("fujiya read_m, offset: %04x\n", offset));
	offset += 0x6000;

	if (offset == 0x7001 || offset == 0x7777)
		return m_latch | ((offset >> 8) & 0x7f);

	return m_open_bus;  // open bus
}
#endif
