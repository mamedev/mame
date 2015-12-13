// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Action 53


 Here we emulate the Multi-Discrete PCB designed by Tepples for
 this homebew multicart [mapper 28]

 ***********************************************************************************************************/


#include "emu.h"
#include "act53.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_ACTION53 = &device_creator<nes_action53_device>;


nes_action53_device::nes_action53_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_ACTION53, "NES Cart Action 53 PCB", tag, owner, clock, "nes_action53", __FILE__),
	m_sel(0)
				{
}



void nes_action53_device::device_start()
{
	common_start();
	save_item(NAME(m_sel));
	save_item(NAME(m_reg));
	m_reg[0] = 0x00;
	m_reg[1] = 0x0f;
	m_reg[2] = 0x00;
	m_reg[3] = 0x3f;
}

void nes_action53_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	// register content is not touched by reset
	update_prg();
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Board ACTION 53

 In MESS: *VERY* preliminary support.

 This board uses 4 registers (reg is selected by writes to 0x5xxx)
 Info from nesdev wiki

 R:$00:  [...M ..CC]
     C = CHR Reg
     M = Mirroring
         This bit overwrites bit 0 of R:$80, but only if bit 1 of
         R:$80 is clear

 R:$01:  [...M PPPP]
     P = PRG Reg
     M = Mirroring
         This bit overwrites bit 0 of R:$80, but only if bit 1 of
         R:$80 is clear

 R:$80:  [..GG PSMM]
     G = Game Size (0=32K, 1=64K, 2=128K, 3=256K)
     P = PRG Size (0=32k mode, 1=16k mode)
     S = Slot select:
         0 = $C000 swappable, $8000 fixed to bottom of 32K outer bank
         1 = $8000 swappable, $C000 fixed to top of 32K outer bank
         This bit is ignored when 'P' is clear (32k mode)
     M = Mirroring control:
         %00 = 1ScA
         %01 = 1ScB
         %10 = Vert
         %11 = Horz

 R:$81:  [..BB BBBB]
     Outer PRG Reg


 -------------------------------------------------*/

void nes_action53_device::update_prg()
{
	UINT8 prg_lo = 0, prg_hi = 0, helper = 0;
	UINT8 out = (m_reg[3] & 0x3f) << 1;     // Outer PRG reg
	UINT8 size = (m_reg[2] & 0x30) >> 4;    // Game size
	UINT8 mask = (1 << (size + 1)) - 1;     // Bits to be taken from PRG reg

	if (!BIT(m_reg[2], 3))
	{
		helper = (out & ~mask) | ((m_reg[1] << 1) & mask);
		//32K mode
		prg_lo = (helper & 0xfe);
		prg_hi = (helper | 0x01);
	}
	else
	{
		helper = (out & ~mask) | (m_reg[1] & mask);
		if (BIT(m_reg[2], 2))
		{
			//16K mode with fixed HI
			prg_lo = helper;
			prg_hi = (out | 0x01);
		}
		else
		{
			//16K mode with fixed LO
			prg_lo = (out & 0xfe);
			prg_hi = helper;
		}
	}

//  printf("banks : 0x%2X - 0x%2X\n", prg_lo, prg_hi);
	prg16_89ab(prg_lo);
	prg16_cdef(prg_hi);
}

void nes_action53_device::update_mirr()
{
	switch (m_reg[2] & 0x03)
	{
		case 0:
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		case 1:
			set_nt_mirroring(PPU_MIRROR_HIGH);
			break;
		case 2:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case 3:
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;
	}
}

WRITE8_MEMBER(nes_action53_device::write_l)
{
	LOG_MMC(("action 53 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;
	if (offset >= 0x1000)
		m_sel = BIT(data, 0) | (BIT(data, 7) << 1);
}


WRITE8_MEMBER(nes_action53_device::write_h)
{
	LOG_MMC(("action 53 write_h, offset: %04x, data: %02x\n", offset, data));

	if (m_reg[m_sel] != data)
	{
		m_reg[m_sel] = data;

		switch (m_sel)
		{
			case 0:
				if (!BIT(m_reg[2],1))
				{
					m_reg[2] &= 0xfe;
					m_reg[2] |= BIT(data,4);
					update_mirr();
				}
				chr8(m_reg[0] & 0x03, m_chr_source);
				break;
			case 1:
				if (!BIT(m_reg[2],1))
				{
					m_reg[2] &= 0xfe;
					m_reg[2] |= BIT(data,4);
					update_mirr();
				}
				update_prg();
				break;
			case 2:
				update_prg();
				update_mirr();
				break;
			case 3:
				update_prg();
				break;
		}
	}
}
