// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Tengen PCBs


 Here we emulate the following PCBs

 * Tengen 800032 [mapper 64]
 * Tengen 800037 [mapper 158]

 Note, Tetris' Tengen 800008 [mapper 148] is implemented in sachen.cpp.

 ***********************************************************************************************************/


#include "emu.h"
#include "tengen.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_TENGEN_800032, nes_tengen032_device, "nes_tengen032", "NES Cart Tengen 800032 PCB")
DEFINE_DEVICE_TYPE(NES_TENGEN_800037, nes_tengen037_device, "nes_tengen037", "NES Cart Tengen 800037 PCB")


nes_tengen032_device::nes_tengen032_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0), m_irq_count(0), m_irq_count_latch(0), m_irq_mode(0), m_irq_reset(0), m_irq_enable(0), m_irq_pending(0), irq_timer(nullptr)
{
}

nes_tengen032_device::nes_tengen032_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_tengen032_device(mconfig, NES_TENGEN_800032, tag, owner, clock)
{
}

nes_tengen037_device::nes_tengen037_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_tengen032_device(mconfig, NES_TENGEN_800037, tag, owner, clock)
{
}




void nes_tengen032_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	timer_freq = clocks_to_attotime(4);
	irq_timer->adjust(attotime::zero, 0, timer_freq);

	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_latch));

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_reset));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_tengen032_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	std::fill(std::begin(m_mmc_prg_bank), std::end(m_mmc_prg_bank), 0x00);
	std::fill(std::begin(m_mmc_vrom_bank), std::end(m_mmc_vrom_bank), 0x00);

	m_latch = 0;
	m_irq_mode = 0;
	m_irq_reset = 0;
	m_irq_enable = 0;
	m_irq_pending = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0xff;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Tengen 800032 Board

 Games: Klax, Road Runner, Rolling Thunder, Shinobi, Skulls
 & Crossbones, Xybots

 This is very similar to MMC-3 (or more probably to the
 Namcot predecessor of MMC-3), but with more registers
 and with an alternative IRQ mode

 iNES: mapper 64

 In MAME: Partially supported.

 TODO: There are issues with IRQ. Skull & Crossbones
 probably doesn't work because of this? It alone uses
 the cycle-based IRQ (and it constantly switches
 between IRQ modes).

 -------------------------------------------------*/

inline void nes_tengen032_device::irq_clock(int blanked)
{
	// From NESdev wiki: Regardless of the mode used to clock the counter, every time the counter is clocked,
	// the following actions occur:
	// - If Reset reg ($C001) was written to after previous clock, reload IRQ counter with IRQ Reload + 1
	// - Otherwise, if IRQ Counter is 0, reload IRQ counter with IRQ Reload value
	// - Otherwise, first decrement IRQ counter by 1, then if IRQ counter is now 0 and IRQs are enabled,
	//   trigger IRQ
	if (m_irq_reset)
	{
		m_irq_reset = 0;
		m_irq_count = m_irq_count_latch | (m_irq_count_latch ? 1 : 0);
	}
	else if (!m_irq_count)
		m_irq_count = m_irq_count_latch;
	else
		m_irq_count--;

	if (m_irq_enable && !blanked && !m_irq_count)
		m_irq_pending = 1;
}

// we use the HBLANK IRQ latch from PPU for the scanline based IRQ mode
// and a timer for the cycle based IRQ mode, which both call irq_clock

void nes_tengen032_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_pending)
		{
			set_irq_line(ASSERT_LINE);
			m_irq_pending = 0;
		}

		if (m_irq_mode)
			irq_clock(0);
	}
}


void nes_tengen032_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (!m_irq_mode) // we are in scanline mode!
	{
		if (scanline <= ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
		{
			irq_clock(blanked);
		}
	}
}

void nes_tengen032_device::set_prg()
{
	u8 prg_flip = (m_latch & 0x40) >> 5;

	prg8_89(m_mmc_prg_bank[0 ^ prg_flip]);
	prg8_ab(m_mmc_prg_bank[1]);
	prg8_cd(m_mmc_prg_bank[2 ^ prg_flip]);
}

void nes_tengen032_device::set_chr()
{
	u8 chr_flip = (m_latch & 0x80) >> 5;

	if (m_latch & 0x20)
	{
		chr1_x(0 ^ chr_flip, m_mmc_vrom_bank[0], CHRROM);
		chr1_x(1 ^ chr_flip, m_mmc_vrom_bank[6], CHRROM);
		chr1_x(2 ^ chr_flip, m_mmc_vrom_bank[1], CHRROM);
		chr1_x(3 ^ chr_flip, m_mmc_vrom_bank[7], CHRROM);
	}
	else
	{
		chr2_x(0 ^ chr_flip, m_mmc_vrom_bank[0] >> 1, CHRROM);
		chr2_x(2 ^ chr_flip, m_mmc_vrom_bank[1] >> 1, CHRROM);
	}

	chr1_x(4 ^ chr_flip, m_mmc_vrom_bank[2], CHRROM);
	chr1_x(5 ^ chr_flip, m_mmc_vrom_bank[3], CHRROM);
	chr1_x(6 ^ chr_flip, m_mmc_vrom_bank[4], CHRROM);
	chr1_x(7 ^ chr_flip, m_mmc_vrom_bank[5], CHRROM);
}

void nes_tengen032_device::write_h(offs_t offset, u8 data)
{
	u8 helper, cmd;
	LOG_MMC(("tengen032 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			helper = m_latch ^ data;
			m_latch = data;

			// Has PRG Mode changed?
			if (helper & 0x40)
				set_prg();

			// Has CHR Mode changed?
			if (helper & 0xa0)
				set_chr();
			break;

		case 0x0001:
			cmd = m_latch & 0x0f;
			switch (cmd)
			{
				case 0: case 1:
				case 2: case 3:
				case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					set_chr();
					break;
				case 6: case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg();
					break;
				case 8: case 9:
					m_mmc_vrom_bank[cmd - 2] = data;
					set_chr();
					break;
				case 0x0f:
					m_mmc_prg_bank[2] = data;
					set_prg();
					break;
			}
			break;

		case 0x2000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x4000:
			m_irq_count_latch = data;
			break;

		case 0x4001: // $c001 - IRQ scanline latch
			m_irq_mode = data & 0x01;
			if (m_irq_mode)
				irq_timer->adjust(attotime::zero, 0, timer_freq);
			m_irq_reset = 1;
			break;

		case 0x6000:
			m_irq_enable = 0;
			m_irq_pending = 0;
			set_irq_line(CLEAR_LINE);
			break;

		case 0x6001:
			m_irq_enable = 1;
			break;

		default:
			LOG_MMC(("Tengen 800032 write. addr: %04x value: %02x\n", offset + 0x8000, data));
			break;
	}
}

/*-------------------------------------------------

 Tengen 800037 Board

 Games: Alien Syndrome

 Same as above (mapper chip RAMBO-1) but CHR A17 output
 is connected to CIRAM A10, so this differs from 800032
 exactly in the same way as TLSROM differs from plain
 MMC-3

 iNES: mapper 158

 In MAME: Supported.

 -------------------------------------------------*/

void nes_tengen037_device::set_chr()
{
	nes_tengen032_device::set_chr();

	// do nametables
	static constexpr u8 bank[8] = { 0, 0, 1, 1, 2, 3, 4, 5 };
	int start = (m_latch & 0x80) >> 5;

	for (int i = 0; i < 4; i++)
		set_nt_page(i, CIRAM, BIT(m_mmc_vrom_bank[bank[start + i]], 7), 1);
}

void nes_tengen037_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("tengen037 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2000:
			break;
		default:
			nes_tengen032_device::write_h(offset, data);
			break;
	}
}
