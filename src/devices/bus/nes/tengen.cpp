// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Tengen PCBs


 Here we emulate the following PCBs

 * Tengen 800008
 * Tengen 800032 [mapper 64]
 * Tengen 800037 [mapper 158]

 TODO:
 - emulated the IRQ delay in 800032 (possibly reason of Skull & Crossbones not working?)

 ***********************************************************************************************************/


#include "emu.h"
#include "tengen.h"

#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access PPU_BOTTOM_VISIBLE_SCANLINE


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_TENGEN_800008 = &device_creator<nes_tengen008_device>;
const device_type NES_TENGEN_800032 = &device_creator<nes_tengen032_device>;
const device_type NES_TENGEN_800037 = &device_creator<nes_tengen037_device>;


nes_tengen008_device::nes_tengen008_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TENGEN_800008, "NES Cart Tengen 800008 PCB", tag, owner, clock, "nes_tengen008", __FILE__)
{
}

nes_tengen032_device::nes_tengen032_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_irq_count(0), m_irq_count_latch(0), m_irq_mode(0), m_irq_reset(0), m_irq_enable(0), m_latch(0), irq_timer(nullptr)
				{
}

nes_tengen032_device::nes_tengen032_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_TENGEN_800032, "NES Cart Tengen 800032 PCB", tag, owner, clock, "nes_tengen032", __FILE__), m_irq_count(0), m_irq_count_latch(0), m_irq_mode(0), m_irq_reset(0), m_irq_enable(0), m_latch(0), irq_timer(nullptr)
{
}

nes_tengen037_device::nes_tengen037_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_tengen032_device(mconfig, NES_TENGEN_800037, "NES Cart Tengen 800037 PCB", tag, owner, clock, "nes_tengen037", __FILE__)
{
}




void nes_tengen008_device::device_start()
{
	common_start();
}

void nes_tengen008_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_tengen032_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->reset();
	timer_freq = machine().device<cpu_device>("maincpu")->cycles_to_attotime(4);

	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_latch));

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_reset));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_tengen032_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 1);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	memset(m_mmc_prg_bank, 0, sizeof(m_mmc_prg_bank));
	memset(m_mmc_vrom_bank, 0, sizeof(m_mmc_vrom_bank));

	m_latch = 0;
	m_irq_mode = 0;
	m_irq_reset = 0;
	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Tengen 800008 Board

 iNES: mapper 3?

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_tengen008_device::write_h)
{
	LOG_MMC(("tengen008 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg32(data >> 3);
	chr8(data, CHRROM);
}

/*-------------------------------------------------

 Tengen 800032 Board

 Games: Klax, Road Runner, Rolling Thunder, Shinobi, Skulls
 & Crossbones, Xybots

 This is very similar to MMC-3 (or more probably to the
 Namcot predecessor of MMC-3), but with more registers
 and with an alternative IRQ mode

 iNES: mapper 64

 In MESS: Partially Supported (there should be a small
 delay between the IRQ and its execution, but that is not
 emulated yet: this is possibly the problem with Skulls
 & Crossbones)

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
		m_irq_count = m_irq_count_latch + 1;
	}
	else if (!m_irq_count)
		m_irq_count = m_irq_count_latch;

	m_irq_count--;
	if (m_irq_enable && !blanked && !m_irq_count)
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
}

// we use the HBLANK IRQ latch from PPU for the scanline based IRQ mode
// and a timer for the cycle based IRQ mode, which both call irq_clock

void nes_tengen032_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		irq_clock(0);
	}
}


void nes_tengen032_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (!m_irq_mode) // we are in scanline mode!
	{
		if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
		{
			irq_clock(blanked);
		}
	}
}

void nes_tengen032_device::set_prg()
{
	UINT8 prg_mode = m_latch & 0x40;

	prg8_89(m_mmc_prg_bank[prg_mode ? 2: 0]);
	prg8_ab(m_mmc_prg_bank[prg_mode ? 0: 1]);
	prg8_cd(m_mmc_prg_bank[prg_mode ? 1: 2]);
}

void nes_tengen032_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, bank, source);
}

void nes_tengen032_device::set_chr()
{
	UINT8 chr_page = (m_latch & 0x80) >> 5;

	if (m_latch & 0x20)
	{
		chr_cb(0 ^ chr_page, m_mmc_vrom_bank[0], CHRROM);
		chr_cb(1 ^ chr_page, m_mmc_vrom_bank[6], CHRROM);
		chr_cb(2 ^ chr_page, m_mmc_vrom_bank[1], CHRROM);
		chr_cb(3 ^ chr_page, m_mmc_vrom_bank[7], CHRROM);
	}
	else
	{
		chr_cb(0 ^ chr_page, m_mmc_vrom_bank[0] & ~0x01, CHRROM);
		chr_cb(1 ^ chr_page, m_mmc_vrom_bank[0] |  0x01, CHRROM);
		chr_cb(2 ^ chr_page, m_mmc_vrom_bank[1] & ~0x01, CHRROM);
		chr_cb(3 ^ chr_page, m_mmc_vrom_bank[1] |  0x01, CHRROM);
	}

	chr_cb(4 ^ chr_page, m_mmc_vrom_bank[2], CHRROM);
	chr_cb(5 ^ chr_page, m_mmc_vrom_bank[3], CHRROM);
	chr_cb(6 ^ chr_page, m_mmc_vrom_bank[4], CHRROM);
	chr_cb(7 ^ chr_page, m_mmc_vrom_bank[5], CHRROM);
}

WRITE8_MEMBER(nes_tengen032_device::tengen032_write)
{
	UINT8 helper, cmd;
	LOG_MMC(("tengen032_write, offset: %04x, data: %02x\n", offset, data));

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

		case 0x4001: /* $c001 - IRQ scanline latch */
			m_irq_mode = data & 0x01;
			if (m_irq_mode)
				irq_timer->adjust(attotime::zero, 0, timer_freq);
			else
				irq_timer->adjust(attotime::never);
			m_irq_reset = 1;
			break;

		case 0x6000:
			m_irq_enable = 0;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
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

 In MESS: Supported.

 -------------------------------------------------*/

void nes_tengen037_device::set_mirror()
{
	if (m_latch & 0x80)
	{
		set_nt_page(0, CIRAM, BIT(m_mmc_vrom_bank[2],7), 1);
		set_nt_page(1, CIRAM, BIT(m_mmc_vrom_bank[3],7), 1);
		set_nt_page(2, CIRAM, BIT(m_mmc_vrom_bank[4],7), 1);
		set_nt_page(3, CIRAM, BIT(m_mmc_vrom_bank[5],7), 1);
	}
	else
	{
		set_nt_page(0, CIRAM, BIT(m_mmc_vrom_bank[0],7), 1);
		set_nt_page(1, CIRAM, BIT(m_mmc_vrom_bank[0],7), 1);
		set_nt_page(2, CIRAM, BIT(m_mmc_vrom_bank[1],7), 1);
		set_nt_page(3, CIRAM, BIT(m_mmc_vrom_bank[1],7), 1);
	}
}

void nes_tengen037_device::chr_cb( int start, int bank, int source )
{
	set_mirror();   // we could probably update only for one (e.g. the first) call, to slightly optimize the code
	chr1_x(start, bank, source);
}


WRITE8_MEMBER(nes_tengen037_device::write_h)
{
	LOG_MMC(("tengen037 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2000:
			break;

		default:
			tengen032_write(space, offset, data, mem_mask);
			break;
	}
}
