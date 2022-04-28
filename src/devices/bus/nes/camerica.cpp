// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Camerica/Codemasters PCBs


 Here we emulate the following PCBs

 * Camerica BF9093, BF9097, BF909X [mapper 71, two variants]
 * Camerica BF9096 Boards [mapper 232]
 * Camerica Golden Five [mapper 104]

 Aladdin Deck Enhancer pass-thru cart and the corresponding minicarts
 (ALGNV11 & ALGQV11 PCBs) are emulated in a separate source file.


 TODO:
 - check what causes flickering from PPU in Fire Hawk, Pogie and Big Nose Freaks Out (same PPU issue as Back to Future 2&3?)
 - mapper 232 games need properly dumped to determine if the "alt" BF9096 board really exists.

 ***********************************************************************************************************/


#include "emu.h"
#include "camerica.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_BF9093,  nes_bf9093_device,  "nes_bf9093",  "NES Cart Camerica BF9093 PCB")
DEFINE_DEVICE_TYPE(NES_BF9096,  nes_bf9096_device,  "nes_bf9096",  "NES Cart Camerica BF9096 PCB")
DEFINE_DEVICE_TYPE(NES_BF9096A, nes_bf9096a_device, "nes_bf9096a", "NES Cart Camerica BF9096 Alt PCB")
DEFINE_DEVICE_TYPE(NES_GOLDEN5, nes_golden5_device, "nes_golden5", "NES Cart Camerica Golden 5 PCB")


nes_bf9093_device::nes_bf9093_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BF9093, tag, owner, clock)
{
}

nes_bf9096_device::nes_bf9096_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool page_swap)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_reg(0), m_page_swap(page_swap)
{
}

nes_bf9096_device::nes_bf9096_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bf9096_device(mconfig, NES_BF9096, tag, owner, clock, false)
{
}

nes_bf9096a_device::nes_bf9096a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bf9096_device(mconfig, NES_BF9096A, tag, owner, clock, true)
{
}

nes_golden5_device::nes_golden5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_GOLDEN5, tag, owner, clock), m_lock(0), m_reg(0)
{
}




void nes_bf9093_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(PPU_MIRROR_LOW);
}

void nes_bf9096_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_bf9096_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(3);

	m_reg = 0;
}

void nes_golden5_device::device_start()
{
	common_start();
	save_item(NAME(m_lock));
	save_item(NAME(m_reg));

	// these are not cleared on reset
	m_lock = 0;
	m_reg = 0;
}

void nes_golden5_device::pcb_reset()
{
	prg16_89ab(m_reg);
	prg16_cdef(m_reg | 0x0f);
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Camerica Boards (BF9093, BF9097, BF909X, ALGNV11)

 Games: Linus Spacehead's Cosmic Crusade, Micro Machines,
 Mig-29, Stunt Kids

 To emulate NT mirroring for BF9097 board (missing in BF9093)
 we use crc_hack, however Fire Hawk is broken (but without
 mirroring there would be no helicopter graphics).

 iNES: mapper 71

 In MAME: Partially supported.

 -------------------------------------------------*/

void nes_bf9093_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bf9093 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x0000:
			if (m_pcb_ctrl_mirror)
				set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;
		case 0x4000:
		case 0x6000:
			prg16_89ab(data);
			break;
	}
}

/*-------------------------------------------------

 Camerica BF9096 & ALGQV11 Boards

 Games: Quattro Adventure, Quattro Arcade, Quattro Sports

 Writes to 0x8000-0xbfff set prg block to (data&0x18)>>1,
 writes to 0xc000-0xffff set prg page to data&3. selected
 prg are: prg16_89ab = block|page, prg_cdef = 3|page.
 For more info on the hardware to bypass the NES lockout, see
 Kevtris' Camerica Mappers documentation.

 iNES: mapper 232

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bf9096_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("bf9096 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		m_reg = (m_reg & 0x03) | (data & 0x18) >> 1;
		if (m_page_swap)
			m_reg = bitswap<4>(m_reg, 2, 3, 1, 0);
		prg16_89ab(m_reg);
		prg16_cdef(m_reg | 0x03);
	}
	else
	{
		m_reg = (m_reg & 0x0c) | (data & 0x03);
		prg16_89ab(m_reg);
	}
}

/*-------------------------------------------------

 Camerica Golden Five board

 Games: Pegasus 5 in 1

 iNES: mapper 104

 In MAME: Supported.

 -------------------------------------------------*/

void nes_golden5_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("golden5 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x4000)
	{
		m_reg = (m_reg & 0x70) | (data & 0x0f);
		prg16_89ab(m_reg);
	}
	else if (!m_lock)
	{
		m_lock = BIT(data, 3);
		m_reg = (m_reg & 0x0f) | (data & 0x07) << 4;
		prg16_89ab(m_reg);
		prg16_cdef(m_reg | 0x0f);
	}
}
