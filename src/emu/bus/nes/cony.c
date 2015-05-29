// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Cony/Yoko PCBs


 Here we emulate the following PCBs

 * UNL-CONY [mapper 83]
 * UNL-YOKO

 TODO: fix glitches and emulate properly the variants

 ***********************************************************************************************************/


#include "emu.h"
#include "cony.h"

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

const device_type NES_CONY = &device_creator<nes_cony_device>;
const device_type NES_YOKO = &device_creator<nes_yoko_device>;


nes_cony_device::nes_cony_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_cony_device::nes_cony_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_CONY, "NES Cart Cony PCB", tag, owner, clock, "nes_cony", __FILE__)
{
}

nes_yoko_device::nes_yoko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_cony_device(mconfig, NES_YOKO, "NES Cart Yoko PCB", tag, owner, clock, "nes_yoko", __FILE__)
{
}




void nes_cony_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));

	save_item(NAME(m_low_reg));
	save_item(NAME(m_reg));
	save_item(NAME(m_extra1));
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}

void nes_cony_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg8_cd(0x1e);
	prg8_ef(0x1f);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;

	m_latch1 = 0;
	m_latch2 = 0;
	m_extra1 = 0;

	memset(m_low_reg, 0, sizeof(m_low_reg));
	memset(m_reg, 0, sizeof(m_reg));
	m_reg[9] = 0x0f;
}

void nes_yoko_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));

	save_item(NAME(m_low_reg));
	save_item(NAME(m_reg));
	save_item(NAME(m_extra1));
	save_item(NAME(m_extra2));
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
}

void nes_yoko_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg8_cd(0x1e);
	prg8_ef(0x1f);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;

	m_latch1 = 0;
	m_latch2 = 0;
	m_extra1 = 0;
	m_extra2 = 0;

	memset(m_low_reg, 0, sizeof(m_low_reg));
	memset(m_reg, 0, sizeof(m_reg));
	m_reg[9] = 0x0f;
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Cony Bootleg Board

 Games: Dragon Ball Party, Fatal Fury 2, Street Blaster II
 Pro, World Heroes 2

 iNES: mapper 83

 In MESS: Supported.

 -------------------------------------------------*/

void nes_cony_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (!m_irq_count)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_enable = 0;
				m_irq_count = 0xffff;
			}
			else
				m_irq_count--;
		}
	}
}

WRITE8_MEMBER(nes_cony_device::write_l)
{
	LOG_MMC(("cony write_l, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x1000 && offset < 0x1103) // from 0x5100-0x51ff
		m_low_reg[offset & 0x03] = data;
}

READ8_MEMBER(nes_cony_device::read_l)
{
	LOG_MMC(("cony read_l, offset: %04x\n", offset));

	if (offset == 0x0f00)   // 0x5000
	{
		// read dipswitch bit! - currently unimplemented
	}
	if (offset >= 0x1000 && offset < 0x1103) // from 0x5100-0x51ff
		return m_low_reg[offset & 0x03];
	else
		return 0x00;
}

void nes_cony_device::set_prg()
{
	prg16_89ab(m_reg[8] & 0x3f);
	prg16_cdef((m_reg[8] & 0x30) | 0x0f);
}

void nes_cony_device::set_chr()
{
	// FIXME: here we emulate at least 3 different boards!!!
	// one board switches 1k VROM banks only
	// one writes to 0x8000 and then switches 2k VROM banks only
	// one writes to 0x831n (n=2,3,4,5) and then switches 2k VROM banks only
	// we should split them and possibly document the proper behavior of each variant
	if (m_latch1 && !m_latch2)
	{
		chr2_0(m_reg[0], CHRROM);
		chr2_2(m_reg[1], CHRROM);
		chr2_4(m_reg[6], CHRROM);
		chr2_6(m_reg[7], CHRROM);
	}
	else
	{
		chr1_0(m_reg[0] | ((m_reg[8] & 0x30) << 4), CHRROM);
		chr1_1(m_reg[1] | ((m_reg[8] & 0x30) << 4), CHRROM);
		chr1_2(m_reg[2] | ((m_reg[8] & 0x30) << 4), CHRROM);
		chr1_3(m_reg[3] | ((m_reg[8] & 0x30) << 4), CHRROM);
		chr1_4(m_reg[4] | ((m_reg[8] & 0x30) << 4), CHRROM);
		chr1_5(m_reg[5] | ((m_reg[8] & 0x30) << 4), CHRROM);
		chr1_6(m_reg[6] | ((m_reg[8] & 0x30) << 4), CHRROM);
		chr1_7(m_reg[7] | ((m_reg[8] & 0x30) << 4), CHRROM);
	}
}

WRITE8_MEMBER(nes_cony_device::write_h)
{
	LOG_MMC(("cony write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			m_latch1 = 1;
		case 0x3000:
		case 0x30ff:
		case 0x31ff:
			m_reg[8] = data;
			set_prg();
			set_chr();
			break;
		case 0x0100:
			m_extra1 = data & 0x80;
			switch (data & 0x03)
			{
				case 0:
					set_nt_mirroring(PPU_MIRROR_VERT);
					break;
				case 1:
					set_nt_mirroring(PPU_MIRROR_HORZ);
					break;
				case 2:
					set_nt_mirroring(PPU_MIRROR_LOW);
					break;
				case 3:
					set_nt_mirroring(PPU_MIRROR_HIGH);
					break;
			}
			break;
		case 0x0200:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x0201:
			m_irq_enable = m_extra1;
			m_irq_count = (data << 8) | (m_irq_count & 0xff);
			break;
		case 0x0300:
			prg8_89(data);
			break;
		case 0x0301:
			prg8_ab(data);
			break;
		case 0x0302:
			prg8_cd(data);
			break;
		case 0x0312:
		case 0x0313:
		case 0x0314:
		case 0x0315:
			m_latch2 = 1;
		case 0x0310:
		case 0x0311:
		case 0x0316:
		case 0x0317:
			m_reg[offset - 0x0310] = data;
			set_chr();
			break;
		case 0x0318:
			m_reg[9] = data;    // unused?
			set_prg();
			break;
	}
}

/*-------------------------------------------------

 Yoko Bootleg Board

 Games: Mortal Kombat II, Master Figther VI'


 Very similar to Cony board

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_yoko_device::write_l)
{
	LOG_MMC(("yoko write_l, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x1300) // from 0x5400
		m_low_reg[offset & 0x03] = data;
}

READ8_MEMBER(nes_yoko_device::read_l)
{
	LOG_MMC(("yoko read_l, offset: %04x\n", offset));

	if (offset >= 0x0f00 && offset < 0x1300)    // 0x5000
	{
		// read dipswitch bit! - currently unimplemented
	}
	if (offset >= 0x1300) // from 0x5400
		return m_low_reg[offset & 0x03];
	else
		return 0x00;
}

void nes_yoko_device::set_prg()
{
	if (m_extra1 & 0x10)
	{
		int base = (m_extra2 & 0x08) << 1;
		prg8_89(base | (m_reg[0] & 0x0f));
		prg8_ab(base | (m_reg[1] & 0x0f));
		prg8_cd(base | (m_reg[2] & 0x0f));
		prg8_ef(base | 0x0f);
	}
	else if (m_extra1 & 0x08)
		prg32(m_extra2 >> 1);
	else
	{
		prg16_89ab(m_extra2);
		prg16_cdef(0xff);
	}
}

void nes_yoko_device::set_chr()
{
	chr2_0(m_reg[4], CHRROM);
	chr2_2(m_reg[5], CHRROM);
	chr2_4(m_reg[6], CHRROM);
	chr2_6(m_reg[7], CHRROM);
}

WRITE8_MEMBER(nes_yoko_device::write_h)
{
	LOG_MMC(("yoko write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x0c17)
	{
		case 0x0000:
			m_extra2 = data;
			set_prg();
			break;
		case 0x400:
			m_extra1 = data;
			if (data & 1)
				set_nt_mirroring(PPU_MIRROR_HORZ);
			else
				set_nt_mirroring(PPU_MIRROR_VERT);
			set_prg();
			break;
		case 0x0800:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x0801:
			m_irq_enable = m_extra1 & 0x80;
			m_irq_count = (data << 8) | (m_irq_count & 0xff);
			break;
		case 0x0c00:
		case 0x0c01:
		case 0x0c02:
			m_reg[offset & 3] = data;
			set_prg();
			break;
		case 0x0c10:
		case 0x0c11:
		case 0x0c16:
		case 0x0c17:
			m_reg[4 + (offset & 3)] = data;
			set_chr();
			break;
	}
}
