// license:BSD-3-Clause
// copyright-holders:Kaz, Fabio Priuli
/***********************************************************************************************************

 NES/Famicom cartridge emulation for Subor PCBs

 TODO:
 - Implement Type 2 variant for Subor Karaoke.
   (Subor Karaoke updates banks differently.)
 - Check and verify CHR banking in Type 2 boards
 - Investigate connection with DANCE 2000 board; likely made by ex-Subor staff!

 ***********************************************************************************************************/

#include "emu.h"
#include "subor.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(...) do { if (VERBOSE) logerror(__VA_ARGS__); } while (0)

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_SUBOR0 = device_creator<nes_subor0_device>;
const device_type NES_SUBOR1 = device_creator<nes_subor1_device>;
const device_type NES_SUBOR2 = device_creator<nes_subor2_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_subor0_device - constructor
//-------------------------------------------------

nes_subor0_device::nes_subor0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: nes_nrom_device(mconfig, NES_SUBOR0, "NES Cart Subor Type 0 PCB", tag, owner, clock, "nes_subor0", __FILE__)
{
}

//-------------------------------------------------
//  nes_subor1_device - constructor
//-------------------------------------------------

nes_subor1_device::nes_subor1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: nes_nrom_device(mconfig, NES_SUBOR1, "NES Cart Subor Type 1 PCB", tag, owner, clock, "nes_subor1", __FILE__)
{
}

//-------------------------------------------------
//  nes_subor2_device - constructor
//-------------------------------------------------

nes_subor2_device::nes_subor2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_SUBOR2, "NES Cart Subor Type 2 PCB", tag, owner, clock, "nes_subor2", __FILE__),
	m_switch_reg(0),
	m_bank_reg(0),
	m_chr_banking(0),
	m_page(0)
{
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void nes_subor0_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_subor1_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_subor2_device::device_start()
{
	common_start();

	save_item(NAME(m_switch_reg));
	save_item(NAME(m_bank_reg));
	save_item(NAME(m_chr_banking));
}

/*-------------------------------------------------
    pcb_reset
-------------------------------------------------*/

void nes_subor0_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0x20);
	chr8(0, m_chr_source);

	memset(m_reg, 0, sizeof(m_reg));
}

void nes_subor1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0x07);
	chr8(0, m_chr_source);

	memset(m_reg, 0, sizeof(m_reg));
}

void nes_subor2_device::pcb_reset()
{
	m_switch_reg = 0;
	m_bank_reg = 0;
	m_chr_banking = true;
	m_page = 0;

	prg16_89ab(0);
	prg16_cdef(0);
}

/*-------------------------------------------------
    mapper specific handlers
-------------------------------------------------*/

/*-------------------------------------------------

 Subor educational cartridge board Type 0:
 iNES Mapper 167

 Subor educational cartridge board Type 1:
 iNES Mapper 166

 Subor educational cartridge board Type 2:
 No iNES mapper as of yet.

 Notes on Type 2 variants:
 Implementation based upon VirtuaNESUp source code
 and studying VirtuaNESEx. The latter makes the
 read at 0x5300 return 0x8F, perhaps as a form
 of copy protection?

 There are two revisions of Type 2 that are
 currently known;

 Subor v5.0- 16k/32k PRG banking combo

 Subor Karaoke- 32k PRG banking

-------------------------------------------------*/

/*-------------------------------------------------
    ppu_latch
-------------------------------------------------*/

void nes_subor2_device::ppu_latch(offs_t offset)
{
	/* CHR banks are conditionally changed midframe */
	/* If this is split off onto the external PPU latch, every edge case works */
	if (m_chr_banking)
	{
		if ( (m_page == 2) || (m_page == 1 && m_switch_reg == 2) )
		{
			chr4_0(1, CHRRAM);
		}
		else
		{
			chr4_0(0, CHRRAM);
		}
	}
}

/*-------------------------------------------------
    nt
-------------------------------------------------*/

READ8_MEMBER(nes_subor2_device::nt_r)
{
	int page = ((offset & 0xc00) >> 10);

	/* Nametable reads report the current page; this seems to work without issues */
	m_page = page;

	return m_nt_access[page][offset & 0x3ff];
}

/*-------------------------------------------------
    update_banks
-------------------------------------------------*/

void nes_subor2_device::update_banks()
{
	switch (m_switch_reg)
	{
		case 0x00:
		case 0x02:
			m_chr_banking = true;
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case 0x01:
		case 0x03:
			m_chr_banking = true;
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;
		case 0x05:
			/* Subor v11.0 needs this, and VirtuaNESEx keeps it specific to this cart */
			/* But leaving it as it is doesn't seem to be an issue for now */
			m_chr_banking = false;
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;
	}

	if (m_switch_reg >= 0x04)
	{
		prg32(m_bank_reg);
	}
	else
	{
		prg16_89ab(m_bank_reg);
		prg16_cdef(0);
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(nes_subor2_device::read_l)
{
	LOG_MMC("subor2 read_l, offset: %04x\n", offset);

	if (offset == 0x1200)
	{
		return 0x8F;
	}
	return m_open_bus;
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(nes_subor0_device::write_h)
{
	uint8_t subor_helper1, subor_helper2;
	LOG_MMC("subor0 write_h, offset: %04x, data: %02x\n", offset, data);

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

WRITE8_MEMBER(nes_subor1_device::write_h)
{
	uint8_t subor_helper1, subor_helper2;
	LOG_MMC("subor1 write_h, offset: %04x, data: %02x\n", offset, data);

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

WRITE8_MEMBER(nes_subor2_device::write_l)
{
	LOG_MMC("subor2 write_l, offset: %04x, data: %02x\n", offset, data);

	switch (offset)
	{
		case 0x0F00:
			m_bank_reg = data;
			update_banks();
			break;
		case 0x1100:
			m_switch_reg = data & 7;
			update_banks();
			break;
	}
}
