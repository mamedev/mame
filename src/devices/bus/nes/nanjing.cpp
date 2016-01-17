// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nanjing PCBs

 TODO:
 - Emulate the variants often assigned to mapper 162/164 (and investigate connection with Waixing FS-304)!

 ***********************************************************************************************************/


#include "emu.h"
#include "nanjing.h"

#include "video/ppu2c0x.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_NANJING = &device_creator<nes_nanjing_device>;


nes_nanjing_device::nes_nanjing_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NANJING, "NES Cart Nanjing PCB", tag, owner, clock, "nes_nanjing", __FILE__),
	m_count(0),
	m_latch1(0),
	m_latch2(0)
				{
}




void nes_nanjing_device::device_start()
{
	common_start();
	save_item(NAME(m_count));
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
	save_item(NAME(m_reg));
}

void nes_nanjing_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 2);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_count = 0xff;
	m_latch1 = 0;
	m_latch2 = 0;
	m_reg[0] = 0xff;
	m_reg[1] = 0;
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Nanjing

 Games: A lot of pirate originals

 iNES: mapper 163

 In MESS: Unsupported.

 -------------------------------------------------*/

void nes_nanjing_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (BIT(m_reg[0], 7))
	{
		if (scanline == 127)
		{
			chr4_0(1, CHRRAM);
			chr4_4(1, CHRRAM);
		}

		if (scanline == 239)
		{
			chr4_0(0, CHRRAM);
			chr4_4(0, CHRRAM);
		}
	}

}

WRITE8_MEMBER(nes_nanjing_device::write_l)
{
	LOG_MMC(("nanjing write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;

	if (offset < 0x1000)
		return;

	if (offset == 0x1100)   // 0x5100
	{
		if (data == 6)
			prg32(3);
		return;
	}

	if (offset == 0x1101)   // 0x5101
	{
		UINT8 temp = m_count;
		m_count = data;

		if ((temp & ~data) & 1)
		{
			m_latch2 ^= 0xff;
		}
	}

	switch (offset & 0x300)
	{
		case 0x000:
		case 0x200:
			m_reg[BIT(offset, 9)] = data;
			if (!BIT(m_reg[0], 7) && machine().device<ppu2c0x_device>("ppu")->get_current_scanline() <= 127)
				chr8(0, CHRRAM);
			break;
		case 0x300:
			m_latch1 = data;
			break;
	}

	prg32((m_reg[0] & 0x0f) | ((m_reg[1] & 0x0f) << 4));
}

READ8_MEMBER(nes_nanjing_device::read_l)
{
	UINT8 value = 0;
	LOG_MMC(("nanjing read_l, offset: %04x\n", offset));

	offset += 0x100;

	if (offset < 0x1000)
		return 0;

	switch (offset & 0x700)
	{
		case 0x100:
			value = m_latch1;
			break;
		case 0x500:
			value = m_latch2 & m_latch1;
			break;
		case 0x000:
		case 0x200:
		case 0x300:
		case 0x400:
		case 0x600:
		case 0x700:
			value = 4;
			break;
	}
	return value;
}
