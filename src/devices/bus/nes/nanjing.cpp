// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nanjing PCBs

 iNES mappers: 163

 Mappers 162 and 164 are emulated by the Waixing FS-304 and FFV boards (waixing.cpp). NESdev lists three Nanjing
 games under mapper 162 on a compatible board, and this board shares the FS-304's automatic 4 KiB CHR-RAM switch.

 VERIFY: the feedback register ($5100/$5500) has not been exercised by any game.

 ***********************************************************************************************************/


#include "emu.h"
#include "nanjing.h"

#include "video/ppu2c0x.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NANJING, nes_nanjing_device, "nes_nanjing", "NES Cart Nanjing PCB")


nes_nanjing_device::nes_nanjing_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_NANJING, tag, owner, clock)
	, m_reg5000(0), m_reg5200(0), m_mode(0), m_feedback(0), m_last_nt(false), m_chr_a12(0)
{
}





void nes_nanjing_device::device_start()
{
	common_start();
	save_item(NAME(m_reg5000));
	save_item(NAME(m_reg5200));
	save_item(NAME(m_mode));
	save_item(NAME(m_feedback));
	save_item(NAME(m_last_nt));
	save_item(NAME(m_chr_a12));
}

// NESdev mapper 163: all registers are cleared on reset, so games boot in 32 KiB PRG-ROM bank #3 (A15/A16 forced to 1)
void nes_nanjing_device::pcb_reset()
{
	m_reg5000 = m_reg5200 = m_mode = m_feedback = 0;
	m_last_nt = false;
	m_chr_a12 = 0;
	chr8(0, m_chr_source);
	update_prg();
}

/*-------------------------------------------------

 Nanjing FC-001 board (iNES mapper 163), per NESdev:
 $5000: C... PPPP  C = automatic 4 KiB CHR-RAM switch, PPPP = PRG A18..A15
 $5200: .... ..PP  PRG A20..A19
 $5300: .... .A.B  A = PRG A15/A16 from $5000 (0: forced to 11b), B = swap D0/D1 on writes to $5000-$5200
 $5100/$5101 (mask $FF01): feedback latch; read back inverted in bit 2 at $5500 (mask $F300)

 -------------------------------------------------*/

void nes_nanjing_device::update_prg()
{
	uint8_t low = m_reg5000 & 0x0f;
	if (!BIT(m_mode, 2))
		low |= 0x03;
	uint8_t high = m_reg5200 & 0x03;
	// NESdev: 1 MiB games connect both ASIC PRG A19 and A20 outputs to ROM A19, exempting this register from the bit-swap
	if (m_prg_chunks <= 0x40 && high)
		high = 1;
	prg32(low | (high << 4));
}

void nes_nanjing_device::write_l(offs_t offset, uint8_t data)
{
	offset += 0x100;
	if ((offset & 0xf000) != 0x1000)
		return;

	const uint8_t swapped = BIT(m_mode, 0) ? bitswap<8>(data, 7, 6, 5, 4, 3, 2, 0, 1) : data;
	switch (offset & 0x0f00)
	{
		case 0x000:   // $5000
			m_reg5000 = swapped;
			update_prg();
			break;
		case 0x100:   // $5100/$5101: A0=0 latches F and E; A0=1 flips the latched F when E (D0) is set
			if (!BIT(offset, 0))
				m_feedback = swapped & 0x05;
			else if (BIT(swapped, 0))
				m_feedback ^= 0x04;
			break;
		case 0x200:   // $5200
			m_reg5200 = swapped;
			update_prg();
			break;
		case 0x300:   // $5300, not subject to the swap
			m_mode = data;
			update_prg();
			break;
	}
}

uint8_t nes_nanjing_device::read_l(offs_t offset)
{
	offset += 0x100;
	if ((offset & 0xf300) == 0x1100)   // $5100/$5500/... (mask $F300): inverted F in bit 2
		return (get_open_bus() & ~0x04) | (BIT(m_feedback, 2) ? 0x00 : 0x04);
	return get_open_bus();
}

// automatic 4 KiB CHR-RAM switch ($5000.7), as on mapper 162: CHR A12 = PPU A9 latched on the last rise of PPU A13
uint8_t nes_nanjing_device::nt_r(offs_t offset)
{
	if (!m_last_nt)
		m_chr_a12 = BIT(offset, 9);
	m_last_nt = true;
	return nes_nrom_device::nt_r(offset);
}

void nes_nanjing_device::nt_w(offs_t offset, uint8_t data)
{
	if (!m_last_nt)
		m_chr_a12 = BIT(offset, 9);
	m_last_nt = true;
	nes_nrom_device::nt_w(offset, data);
}

uint8_t nes_nanjing_device::chr_r(offs_t offset)
{
	m_last_nt = false;
	if (BIT(m_reg5000, 7) && !m_vram.empty())
		return m_vram[((m_chr_a12 << 12) | (offset & 0x0fff)) & (m_vram.size() - 1)];
	return nes_nrom_device::chr_r(offset);
}

void nes_nanjing_device::chr_w(offs_t offset, uint8_t data)
{
	m_last_nt = false;
	if (BIT(m_reg5000, 7) && !m_vram.empty())
		m_vram[((m_chr_a12 << 12) | (offset & 0x0fff)) & (m_vram.size() - 1)] = data;
	else
		nes_nrom_device::chr_w(offset, data);
}
