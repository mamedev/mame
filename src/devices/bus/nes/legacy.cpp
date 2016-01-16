// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for FFE PCBs


 Here we emulate the mappers used by available Far Front East copier hacked games [mappers 6, 8, 17]

 TODO:
 - investigate IRQ mechanism (current code is broken)
 - replace this with proper copier emulation, using disk images...

 ***********************************************************************************************************/


#include "emu.h"
#include "legacy.h"

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

const device_type NES_FFE3 = &device_creator<nes_ffe3_device>;
const device_type NES_FFE4 = &device_creator<nes_ffe4_device>;
const device_type NES_FFE8 = &device_creator<nes_ffe8_device>;


nes_ffe3_device::nes_ffe3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_FFE3, "NES Cart FFE-3 PCB", tag, owner, clock, "nes_ff3", __FILE__)
{
}

nes_ffe4_device::nes_ffe4_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr), m_latch(0), m_exram_enabled(0), m_exram_bank(0)
				{
}

nes_ffe4_device::nes_ffe4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_FFE4, "NES Cart FFE-4 PCB", tag, owner, clock, "nes_ff4", __FILE__), m_irq_count(0), m_irq_enable(0), irq_timer(nullptr), m_latch(0), m_exram_enabled(0), m_exram_bank(0)
				{
}

nes_ffe8_device::nes_ffe8_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_ffe4_device(mconfig, NES_FFE8, "NES Cart FFE-8 PCB", tag, owner, clock, "nes_ff8", __FILE__)
{
}



void nes_ffe3_device::device_start()
{
	common_start();
}

void nes_ffe3_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}


void nes_ffe4_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_exram));
	save_item(NAME(m_exram_enabled));
	save_item(NAME(m_exram_bank));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_latch));
}

void nes_ffe4_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(7);
	chr8(0, m_chr_source);

	m_exram_enabled = 0;
	m_exram_bank = 0;

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
}


void nes_ffe8_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(0xff);
	chr8(0, m_chr_source);

	// extra vram is not used by this board, so these will remain always zero
	m_exram_enabled = 0;
	m_exram_bank = 0;

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Mapper 8

 Known Boards: FFE3 Copier Board
 Games: Hacked versions of games

 In MESS: Supported? (I have no games to test this)

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ffe3_device::write_h)
{
	LOG_MMC(("mapper8 write_h, offset: %04x, data: %02x\n", offset, data));

	chr8(data & 0x07, CHRROM);
	prg16_89ab(data >> 3);
}

/*-------------------------------------------------

 Mapper 6

 Known Boards: FFE4 Copier Board
 Games: Hacked versions of games

 In MESS: Supported? Not sure if we could also have ExRAM or not...
 However, priority is pretty low for this mapper.

 -------------------------------------------------*/

void nes_ffe4_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0xffff)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
				m_irq_count = 0;
				m_irq_enable = 0;
			}
			else
				m_irq_count++;
		}
	}
}

WRITE8_MEMBER(nes_ffe4_device::write_l)
{
	LOG_MMC(("mapper6 write_l, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1fe:
			m_latch = data & 0x80;
			set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;
		case 0x1ff:
			set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x401:
			m_irq_enable = 0;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;
		case 0x402:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x403:
			m_irq_enable = 1;
			m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			break;
	}
}

WRITE8_MEMBER(nes_ffe4_device::chr_w)
{
	int bank = offset >> 10;
	if (m_exram_enabled)
		m_exram[(m_exram_bank * 0x2000) + (bank * 0x400) + (offset & 0x3ff)] = data;

	if (m_chr_src[bank] == CHRRAM)
		m_chr_access[bank][offset & 0x3ff] = data;
}

READ8_MEMBER(nes_ffe4_device::chr_r)
{
	int bank = offset >> 10;
	if (m_exram_enabled)
		return m_exram[(m_exram_bank * 0x2000) + (bank * 0x400) + (offset & 0x3ff)];

	return m_chr_access[bank][offset & 0x3ff];
}


WRITE8_MEMBER(nes_ffe4_device::write_h)
{
	LOG_MMC(("mapper6 write_h, offset: %04x, data: %02x\n", offset, data));

	if (!m_latch)  // when in "FFE mode" we are forced to use CHRRAM/EXRAM bank?
	{
		prg16_89ab(data >> 2);

		// This part is not fully documented, so we proceed a bit blindly...
		if ((data & 0x03) == 0)
		{
			m_exram_enabled = 0;
			chr8(0, CHRRAM);
		}
		else
		{
			m_exram_enabled = 1;
			m_exram_bank = data & 0x03;
		}
	}
	else // otherwise, we use CHRROM (shall we check if it's present?)
		chr8(data, CHRROM);
}

/*-------------------------------------------------

 Mapper 17

 Known Boards: FFE8 Copier Board
 Games: Hacked versions of games

 In MESS: Partially Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ffe8_device::write_l)
{
	LOG_MMC(("mapper17 write_l, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1fe:
			set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;
		case 0x1ff:
			set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x401:
			m_irq_enable = 0;
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;
		case 0x402:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x403:
			m_irq_enable = 1;
			m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			break;

		case 0x404:
			prg8_89(data);
			break;
		case 0x405:
			prg8_ab(data);
			break;
		case 0x406:
			prg8_cd(data);
			break;
		case 0x407:
			prg8_ef(data);
			break;

		case 0x410:
		case 0x411:
		case 0x412:
		case 0x413:
		case 0x414:
		case 0x415:
		case 0x416:
		case 0x417:
			chr1_x(offset & 7, data, CHRROM);
			break;
	}
}
