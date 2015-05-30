// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for 2A03 Puritans Album


 Here we emulate the PCB designed by infiniteneslives and
 rainwarrior for this homebew multicart [mapper 30?]
 The main difference of this PCB compared to others is that it
 uses 4k PRG banks!

 ***********************************************************************************************************/


#include "emu.h"
#include "2a03pur.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_2A03PURITANS = &device_creator<nes_2a03pur_device>;


nes_2a03pur_device::nes_2a03pur_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_2A03PURITANS, "NES Cart 2A03 Puritans Album PCB", tag, owner, clock, "nes_2a03pur", __FILE__)
{
}



void nes_2a03pur_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	memset(m_reg, 0x00, sizeof(m_reg));
	m_reg[7] = 0xff;
}

void nes_2a03pur_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	chr8(0, m_chr_source);
	// register content is not touched by reset
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Board 2A03 Puritans Album

 In MESS: supported.

 This mapper supports up to 1MB of PRG-ROM, in 4k
 banks located at $8000, $9000, $A000, $B000, $C000,
 $D000, $E000, and $F000. Each bank is selected by n
 8-bit register at $5FF8, $5FF9, $5FFA, $5FFB, $5FFC,
 $5FFD, $5FFE, and $5FFF, respectively, just like NSF
 banking. These registers are mirrored across the
 entire $5000-$5FFF region (the register is selected
 by the low 3 bits), but it is recommended to use the
 original addresses. The mirroring is merely a
 convenience for the hardware implementation.

 The 8kb CHR region may be RAM or ROM. This project
 uses CHR-RAM, and the board used by infiniteneslives
 for this project may only support CHR-RAM.

 At power-on, the mapper automatically sets all bits
 in the $5FFF bank register, placing the highest bank
 in $F000. This occurs on power-on but not on reset,
 so any bank that is mapped to $F000 after power-on
 should contain a valid reset vector.

 At present, the project uses iNES mapper 30 to
 designate this mapper. No mapper number has been
 officially reserved yet.
 -------------------------------------------------*/

WRITE8_MEMBER(nes_2a03pur_device::write_l)
{
	LOG_MMC(("2a03 puritans write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;
	if (offset >= 0x1000)
		m_reg[offset & 7] = data;
}

READ8_MEMBER(nes_2a03pur_device::read_h)
{
	LOG_MMC(("2a03 puritans read_h, offset: %04x\n", offset));

	return m_prg[(m_reg[(offset >> 12) & 7] * 0x1000) + (offset & 0x0fff)];
}
