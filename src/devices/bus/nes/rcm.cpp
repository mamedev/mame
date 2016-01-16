// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for RCM PCBs


 Here we emulate the following PCBs

 * RCM GS2015 [mapper 216]
 * RCM GS2004
 * RCM GS2013
 * RCM Tetris Family 9in1 [mapper 61]
 * RCM 3D Block

 TODO:
 - investigate why 3D Block does not work

 ***********************************************************************************************************/


#include "emu.h"
#include "rcm.h"

#include "cpu/m6502/m6502.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_GS2015 = &device_creator<nes_gs2015_device>;
const device_type NES_GS2004 = &device_creator<nes_gs2004_device>;
const device_type NES_GS2013 = &device_creator<nes_gs2013_device>;
const device_type NES_TF9IN1 = &device_creator<nes_tf9_device>;
const device_type NES_3DBLOCK = &device_creator<nes_3dblock_device>;


nes_gs2015_device::nes_gs2015_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_GS2015, "NES Cart RCM GS-2015 PCB", tag, owner, clock, "nes_gs2015", __FILE__)
{
}

nes_gs2004_device::nes_gs2004_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_GS2004, "NES Cart RCM GS-2004 PCB", tag, owner, clock, "nes_gs2004", __FILE__)
{
}

nes_gs2013_device::nes_gs2013_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_GS2013, "NES Cart RCM GS-2013 PCB", tag, owner, clock, "nes_gs2013", __FILE__)
{
}

nes_tf9_device::nes_tf9_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TF9IN1, "NES Cart RCM Tetris Family 9 in 1 PCB", tag, owner, clock, "nes_tetrisfam", __FILE__)
{
}

nes_3dblock_device::nes_3dblock_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_3DBLOCK, "NES Cart RCM 3D Block PCB", tag, owner, clock, "nes_3dblock", __FILE__), m_irq_count(0)
				{
}




void nes_gs2015_device::device_start()
{
	common_start();
}

void nes_gs2015_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_gs2004_device::device_start()
{
	common_start();
}

void nes_gs2004_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);
}

void nes_gs2013_device::device_start()
{
	common_start();
}

void nes_gs2013_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);
}

void nes_tf9_device::device_start()
{
	common_start();
}

void nes_tf9_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_3dblock_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_irq_count));
}

void nes_3dblock_device::pcb_reset()
{
	prg32(0);
	chr8(0, CHRRAM);
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_reg[3] = 0;
	m_irq_count = 0;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 RCM GS2015 Board

 Games: Bonza, Magic Jewelry 2

 Very simple mapper: writes to 0x8000-0xffff sets prg32
 to offset and chr8 to offset>>1 (when chrrom is present)

 iNES: mapper 216

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_gs2015_device::write_h)
{
	LOG_MMC(("gs2015 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(offset);
	chr8(offset >> 1, m_chr_source);
}

READ8_MEMBER(nes_gs2015_device::read_m)
{
	LOG_MMC(("gs2015 read_m, offset: %04x\n", offset));
	return 0;   // Videopoker Bonza needs this (sort of protection? or related to inputs?)
}

/*-------------------------------------------------

 Board BMC-GS2004

 Games: Tetris Family 6-in-1

 In MESS: Preliminary Support. It also misses WRAM handling
 (we need reads from 0x6000-0x7fff)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_gs2004_device::write_h)
{
	LOG_MMC(("gs2004 write_h, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
}

/*-------------------------------------------------

 Board BMC-GS2013

 Games: Tetris Family 12-in-1

 In MESS: Preliminary Support. It also misses WRAM handling
 (we need reads from 0x6000-0x7fff)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_gs2013_device::write_h)
{
	LOG_MMC(("gs2013 write_h, offset: %04x, data: %02x\n", offset, data));

	if (data & 0x08)
		prg32(data & 0x09);
	else
		prg32(data & 0x07);
}

/*-------------------------------------------------

 Bootleg Board by RCM for Tetris Family

 Games: Tetris Family 9 in 1, 20 in 1

 Simple Mapper: prg/chr/nt are swapped depending on the offset
 of writes in 0x8000-0xffff. offset&0x80 set NT mirroring,
 when (offset&0x30) is 0,3 prg32 is set; when it is 1,2
 two 16k prg banks are set. See below for the values used in
 these banks.

 iNES: mapper 61

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_tf9_device::write_h)
{
	LOG_MMC(("tetrisfam write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x30)
	{
		case 0x00:
		case 0x30:
			prg32(offset & 0x0f);
			break;
		case 0x10:
		case 0x20:
			prg16_89ab(((offset & 0x0f) << 1) | ((offset & 0x20) >> 4));
			prg16_cdef(((offset & 0x0f) << 1) | ((offset & 0x20) >> 4));
			break;
	}
	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*-------------------------------------------------

 Bootleg Board by RCM for 3-D Block

 Games: 3-D Block Hwang Shinwei version

 iNES:

 In MESS: Very Preliminary Support. What is the purpose
 of the writes to $4800-$4900-$4a00? These writes
 also happens on the RCM version, which however works
 (probably an unused leftover code in that version)

 FCEUmm suggests it might be IRQ related, but
 it does not seem to help much...

 -------------------------------------------------*/

void nes_3dblock_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (m_irq_count)
	{
		m_irq_count--;
		if (!m_irq_count)
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
}

WRITE8_MEMBER(nes_3dblock_device::write_l)
{
	LOG_MMC(("3dblock write_l, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset)
	{
		case 0x800: // $4800
			m_reg[0] = data;
			break;
		case 0x900: // $4900
			m_reg[1] = data;
			break;
		case 0xa00: // $4a00
			m_reg[2] = data;
			break;
		case 0xe00: // $4e00
			m_reg[3] = data; m_irq_count = 0x10;
			break;
	}
}
