// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for PCBs mostly based on discrete components

 Here we emulate the following PCBs

 * PCB with IC 74x161x161x32 [mapper 70 & 152]
 * PCB with IC 74x139x74 [mapper 87]
 * PCB with IC 74x377 [mapper 11]
 * PCB with IC 74x161x138 [mapper 38]

 TODO:
 - Investigating missing inputs in Crime Busters

 ***********************************************************************************************************/


#include "emu.h"
#include "discrete.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_74X161X161X32 = &device_creator<nes_74x161x161x32_device>;
const device_type NES_74X139X74 = &device_creator<nes_74x139x74_device>;
const device_type NES_74X377 = &device_creator<nes_74x377_device>;
const device_type NES_74X161X138 = &device_creator<nes_74x161x138_device>;


nes_74x161x161x32_device::nes_74x161x161x32_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_74X161X161X32, "NES Cart Discrete Logic (74*161/161/32) PCB", tag, owner, clock, "nes_74x161", __FILE__)
{
}

nes_74x139x74_device::nes_74x139x74_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_74X139X74, "NES Cart Discrete Logic (74*139/74) PCB", tag, owner, clock, "nes_74x139", __FILE__)
{
}

nes_74x377_device::nes_74x377_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_74X377, "NES Cart Discrete Logic (74*377) PCB", tag, owner, clock, "nes_74x377", __FILE__)
{
}

nes_74x161x138_device::nes_74x161x138_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_74X161X138, "NES Cart Discrete Logic (74*161/138) PCB", tag, owner, clock, "nes_bitcorp_dis", __FILE__)
{
}




void nes_74x161x161x32_device::device_start()
{
	common_start();
}

void nes_74x161x161x32_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_74x139x74_device::device_start()
{
	common_start();
}

void nes_74x139x74_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_74x377_device::device_start()
{
	common_start();
}

void nes_74x377_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_74x161x138_device::device_start()
{
	common_start();
}

void nes_74x161x138_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Discrete Logic board IC 74x161x161x32

 There are two variants (one with hardwired mirroring, the
 other with a mirroring control), making necessary two distinct
 mappers & pcb_id

 iNES: mappers 70 & 152

 -------------------------------------------------*/

// there are two 'variants' depending on hardwired or mapper ctrl mirroring
WRITE8_MEMBER(nes_74x161x161x32_device::write_h)
{
	LOG_MMC(("74x161x161x32 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8(data, CHRROM);
	prg16_89ab(data >> 4);
}

/*-------------------------------------------------

 Discrete Logic board IC 74x139x74 by Konami & Jaleco

 iNES: mapper 87

 -------------------------------------------------*/

WRITE8_MEMBER(nes_74x139x74_device::write_m)
{
	LOG_MMC(("74x139x74 write_m, offset: %04x, data: %02x\n", offset, data));

	chr8(((data & 0x02) >> 1) | ((data & 0x01) << 1), CHRROM);
}

/*-------------------------------------------------

 Discrete Logic board IC 74x377 by Color Dreams / Nina-007 emulation

 Games: many Color Dreams and Wisdom Tree titles

 iNES: mapper 11

 In MESS: Supported

 Note: bit2 & bit3 are actually related to CIC lockout
 defeating, and real Color Dreams titles use only
 bit0 & bit1 for PRG switching. Our usage allows for support
 of extended PRG games (e.g. homebrew)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_74x377_device::write_h)
{
	LOG_MMC(("74x377 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict, but not the prototype of Secret Scout, which actually breaks in case of conflict...
	data = account_bus_conflict(offset, data);

	chr8(data >> 4, m_chr_source);
	prg32(data & 0x0f);
}

/*-------------------------------------------------

 Discrete Logic board IC 74x161x138

 Games: Crime Busters

 iNES: mapper 38

 -------------------------------------------------*/

WRITE8_MEMBER(nes_74x161x138_device::write_m)
{
	LOG_MMC(("74x161x138 write_m, offset: %04x, data: %02x\n", offset, data));

	chr8(data >> 2, CHRROM);
	prg32(data);
}
