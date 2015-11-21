// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-2 & MMC-4 PCBs


 Here we emulate the Nintendo PxROM and FxROM PCBs [mapper 9 & 10]

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc2.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_PXROM = &device_creator<nes_pxrom_device>;
const device_type NES_FXROM = &device_creator<nes_fxrom_device>;


nes_pxrom_device::nes_pxrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), 
	m_latch1(0), 
	m_latch2(0)
				{
}

nes_pxrom_device::nes_pxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_PXROM, "NES Cart PxROM (MMC-2) PCB", tag, owner, clock, "nes_pxrom", __FILE__), m_latch1(0), m_latch2(0)
				{
}

nes_fxrom_device::nes_fxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_pxrom_device(mconfig, NES_FXROM, "NES Cart FxROM (MMC-4) PCB", tag, owner, clock, "nes_fxrom", __FILE__)
{
}




void nes_pxrom_device::device_start()
{
	common_start();
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
	save_item(NAME(m_reg));
}

void nes_pxrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg8_89(0);
	prg8_ab((m_prg_chunks << 1) - 3);
	prg8_cd((m_prg_chunks << 1) - 2);
	prg8_ef((m_prg_chunks << 1) - 1);
	chr8(0, m_chr_source);

	m_reg[0] = m_reg[2] = 0;
	m_reg[1] = m_reg[3] = 0;
	m_latch1 = m_latch2 = 0xfe;
}

void nes_fxrom_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_reg[0] = m_reg[2] = 0;
	m_reg[1] = m_reg[3] = 0;
	m_latch1 = m_latch2 = 0xfe;
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 PxROM (MMC2 based) board emulation

 Games: Punch Out!!, Mike Tyson's Punch Out!!

 iNES: mapper 9

 In MESS: Supported

 -------------------------------------------------*/

void nes_pxrom_device::ppu_latch(offs_t offset)
{
	if ((offset & 0x3ff0) == 0x0fd0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 0 low): %02x\n", m_reg[0]));
		m_latch1 = 0xfd;
		chr4_0(m_reg[0], CHRROM);
	}
	else if ((offset & 0x3ff0) == 0x0fe0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 0 high): %02x\n", m_reg[1]));
		m_latch1 = 0xfe;
		chr4_0(m_reg[1], CHRROM);
	}
	else if ((offset & 0x3ff0) == 0x1fd0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 1 low): %02x\n", m_reg[2]));
		m_latch2 = 0xfd;
		chr4_4(m_reg[2], CHRROM);
	}
	else if ((offset & 0x3ff0) == 0x1fe0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 0 high): %02x\n", m_reg[3]));
		m_latch2 = 0xfe;
		chr4_4(m_reg[3], CHRROM);
	}
}

WRITE8_MEMBER( nes_pxrom_device::pxrom_write )
{
	LOG_MMC(("pxrom write_h, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x7000)
	{
		case 0x2000:
			prg8_89(data);
			break;
		case 0x3000:
			m_reg[0] = data;
			if (m_latch1 == 0xfd)
				chr4_0(m_reg[0], CHRROM);
			break;
		case 0x4000:
			m_reg[1] = data;
			if (m_latch1 == 0xfe)
				chr4_0(m_reg[1], CHRROM);
			break;
		case 0x5000:
			m_reg[2] = data;
			if (m_latch2 == 0xfd)
				chr4_4(m_reg[2], CHRROM);
			break;
		case 0x6000:
			m_reg[3] = data;
			if (m_latch2 == 0xfe)
				chr4_4(m_reg[3], CHRROM);
			break;
		case 0x7000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		default:
			LOG_MMC(("MMC2 uncaught w: %04x:%02x\n", offset, data));
			break;
	}
}

/*-------------------------------------------------

 FxROM (MMC4 based) board emulation

 Games: Famicom Wars, Fire Emblem, Fire Emblem Gaiden

 This is a small hardware variants of MMC2 (additional
 prg bankswitch line)

 iNES: mapper 10

 In MESS: Supported

 -------------------------------------------------*/

WRITE8_MEMBER( nes_fxrom_device::write_h )
{
	LOG_MMC(("fxrom write_h, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x7000)
	{
		case 0x2000:
			prg16_89ab(data);
			break;
		default:
			pxrom_write(space, offset, data, mem_mask);
			break;
	}
}
