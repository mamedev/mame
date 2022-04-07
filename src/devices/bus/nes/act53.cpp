// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Action 53


 Here we emulate the Multi-Discrete PCB designed by Tepples for
 this homebrew multicart [mapper 28]

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

DEFINE_DEVICE_TYPE(NES_ACTION53, nes_action53_device, "nes_action53", "NES Cart Action 53 PCB")


nes_action53_device::nes_action53_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_ACTION53, tag, owner, clock)
	, m_sel(0)
{
}



void nes_action53_device::device_start()
{
	common_start();
	save_item(NAME(m_sel));
	save_item(NAME(m_reg));
}

void nes_action53_device::pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted)
{
	device_nes_cart_interface::pcb_start(machine, ciram_ptr, cart_mounted);
	// at power on last 16K of ROM is mapped to $C000-$FFFF
	m_reg[0] = m_reg[1] = m_reg[2] = 0;
	m_reg[3] = (m_prg_chunks - 1) >> 1;
	update_prg();
}

void nes_action53_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	// register content is not touched by reset
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Board ACTION 53

 iNES: mapper 28

 In MAME: Preliminary supported.

 This board uses 4 registers (reg is selected by writes to 0x5xxx)
 Info from nesdev wiki

 R:$00:  [...M ..CC]
     C = CHR Reg
     M = Mirroring
         This bit overwrites bit 0 of R:$80, but only if bit 1 of
         R:$80 is clear

 R:$01:  [...M PPPP]
     P = Inner PRG Reg
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

 R:$81:  [BBBB BBBB]
     Outer PRG Reg


 -------------------------------------------------*/

void nes_action53_device::update_prg()
{
	u16 prg_lo, prg_hi;
	u8 size = BIT(m_reg[2], 4, 2);            // Game size
	u16 mask = ~0 << (size + 1);              // Bits to be taken from PRG regs
	u8 b32k = !BIT(m_reg[2], 3);              // 32K mode bit
	u16 outer = m_reg[3] << 1;                // Outer PRG reg bits
	u8 inner = (m_reg[1] << b32k) & ~mask;    // Inner PRG reg bits

	prg_hi = prg_lo = (outer & mask) | inner;
	if (b32k)                     // 32K mode
		prg_hi++;
	else if (BIT(m_reg[2], 2))    // 16K mode with fixed HI
		prg_hi = ++outer;
	else                          // 16K mode with fixed LO
		prg_lo = outer;

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

void nes_action53_device::write_l(offs_t offset, u8 data)
{
	LOG_MMC(("action 53 write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;
	if (offset >= 0x1000)
		m_sel = bitswap<2>(data, 7, 0);
}


void nes_action53_device::write_h(offs_t offset, u8 data)
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
