// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for AVE PCBs


 Here we emulate the following PCBs

   * AVE Nina-001 [mapper 34]
   * AVE Nina-006/Nina-003/MB-91 [mapper 79]
   * AVE Maxi 15 [mapper 234]


 ***********************************************************************************************************/


#include "emu.h"
#include "ave.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_NINA001 = &device_creator<nes_nina001_device>;
const device_type NES_NINA006 = &device_creator<nes_nina006_device>;
const device_type NES_MAXI15 = &device_creator<nes_maxi15_device>;


nes_nina001_device::nes_nina001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NINA001, "NES Cart AVE Nina-001 PCB", tag, owner, clock, "nes_nina001", __FILE__)
{
}

nes_nina006_device::nes_nina006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_NINA006, "NES Cart AVE Nina-006 PCB", tag, owner, clock, "nes_nina006", __FILE__)
{
}

nes_maxi15_device::nes_maxi15_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_MAXI15, "NES Cart AVE Maxi 15 PCB", tag, owner, clock, "nes_maxi15", __FILE__)
{
}




void nes_nina001_device::device_start()
{
	common_start();
}

void nes_nina001_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_nina006_device::device_start()
{
	common_start();
}

void nes_nina006_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}


void nes_maxi15_device::device_start()
{
	common_start();
	save_item(NAME(m_bank));
	save_item(NAME(m_reg));
}

void nes_maxi15_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_reg = 0;
	m_bank = 0;
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 AVE NINA-001 board emulation

 iNES: mapper 34

 Notice that in this board the bankswitch regs
 overlaps WRAM, so that writes to the regs are
 then readable back in WRAM (WRAM is tested by
 Impossible Mission II at start)

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_nina001_device::write_m)
{
	LOG_MMC(("nina-001 write_m, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ffd:
			prg32(data);
			break;
		case 0x1ffe:
			chr4_0(data, CHRROM);
			break;
		case 0x1fff:
			chr4_4(data, CHRROM);
			break;
	}

	m_prgram[offset] = data;
}

/*-------------------------------------------------

 AVE NINA-003, NINA-006 and MB-91 boards emulation

 Games: Krazy Kreatures, Poke Block, Puzzle, Pyramid,
 Solitaire, Ultimate League Soccer

 iNES: mapper 79

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_nina006_device::write_l)
{
	LOG_MMC(("nina-006 write_l, offset: %04x, data: %02x\n", offset, data));

	if (!(offset & 0x0100))
	{
		prg32(data >> 3);
		chr8(data & 7, CHRROM);
	}
}

/*-------------------------------------------------

 AVE Maxi 15 boards emulation

 Games: Maxi 15

 iNES: mapper 234

 In MESS: Partially Supported.

 -------------------------------------------------*/

void nes_maxi15_device::update_banks()
{
	if (m_bank & 0x40)
	{
		prg32((m_bank & 0x0e) | (m_reg & 1));
		chr8(((m_bank & 0x0e) << 2) | ((m_reg >> 4) & 7), m_chr_source);
	}
	else
	{
		prg32(m_bank & 0x0f);
		chr8(((m_bank & 0x0f) << 2) | ((m_reg >> 4) & 3), m_chr_source);
	}
}

READ8_MEMBER(nes_maxi15_device::read_h)
{
	LOG_MMC(("Maxi 15 read_h, offset: %04x\n", offset));

	if (offset >= 0x7f80 && offset < 0x7fa0)
	{
		m_bank = hi_access_rom(offset);
		set_nt_mirroring(BIT(m_bank, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		update_banks();
		return m_bank;
	}
	if (offset >= 0x7fe8 && offset < 0x7ff8)
	{
		m_reg = hi_access_rom(offset);
		update_banks();
		return m_reg;
	}

	return hi_access_rom(offset);
}
