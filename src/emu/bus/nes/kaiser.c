// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Kaiser PCBs


 Here we emulate the following Kaiser bootleg PCBs

 * Kaiser KS202
 * Kaiser KS7012
 * Kaiser KS7013B
 * Kaiser KS7017
 * Kaiser KS7022
 * Kaiser KS7032
 * Kaiser KS7058

 The Kaiser KS7057 bootleg board is emulated in nes_mmc3_clones.c


 TODO:
 - FCEUmm lists more Kaiser PCBs:
   * KS7030 (for Yume Koujou Doki Doki Panic by Kaiser?)
   * KS7037
   but there seem to be no available dumps...

 ***********************************************************************************************************/


#include "emu.h"
#include "kaiser.h"

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

const device_type NES_KS7058 = &device_creator<nes_ks7058_device>;
const device_type NES_KS7022 = &device_creator<nes_ks7022_device>;
const device_type NES_KS7032 = &device_creator<nes_ks7032_device>;
const device_type NES_KS202 = &device_creator<nes_ks202_device>;
const device_type NES_KS7017 = &device_creator<nes_ks7017_device>;
const device_type NES_KS7012 = &device_creator<nes_ks7012_device>;
const device_type NES_KS7013B = &device_creator<nes_ks7013b_device>;
const device_type NES_KS7031 = &device_creator<nes_ks7031_device>;


nes_ks7058_device::nes_ks7058_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KS7058, "NES Cart Kaiser KS-7058 PCB", tag, owner, clock, "nes_ks7058", __FILE__)
{
}

nes_ks7022_device::nes_ks7022_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KS7022, "NES Cart Kaiser KS-7022 PCB", tag, owner, clock, "nes_ks7022", __FILE__)
{
}

nes_ks7032_device::nes_ks7032_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: nes_nrom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

nes_ks7032_device::nes_ks7032_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KS7032, "NES Cart Kaiser KS-7032 PCB", tag, owner, clock, "nes_ks7032", __FILE__)
{
}

nes_ks202_device::nes_ks202_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_ks7032_device(mconfig, NES_KS202, "NES Cart Kaiser KS-202 PCB", tag, owner, clock, "nes_ks202", __FILE__)
{
}

nes_ks7017_device::nes_ks7017_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KS7017, "NES Cart Kaiser KS-7017 PCB", tag, owner, clock, "nes_ks7017", __FILE__)
{
}

nes_ks7012_device::nes_ks7012_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KS7012, "NES Cart Kaiser KS-7012 PCB", tag, owner, clock, "nes_ks7012", __FILE__)
{
}

nes_ks7013b_device::nes_ks7013b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KS7013B, "NES Cart Kaiser KS-7013B PCB", tag, owner, clock, "nes_ks7013b", __FILE__)
{
}

nes_ks7031_device::nes_ks7031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KS7031, "NES Cart Kaiser KS-7031 PCB", tag, owner, clock, "nes_ks7031", __FILE__)
{
}




void nes_ks7058_device::device_start()
{
	common_start();
}

void nes_ks7058_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0);
	chr8(0, m_chr_source);
}

void nes_ks7022_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_ks7022_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_ks7032_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_reg));
}

void nes_ks7032_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
	memset(m_reg, 0, sizeof(m_reg));
	prg_update();
}

void nes_ks7017_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_status));
}

void nes_ks7017_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(2);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_status = 0;
}

void nes_ks7012_device::device_start()
{
	common_start();
}

void nes_ks7012_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32(0xff);
	chr8(0, m_chr_source);
}

void nes_ks7013b_device::device_start()
{
	common_start();
}

void nes_ks7013b_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_ks7031_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
}

void nes_ks7031_device::pcb_reset()
{
	prg32(0);   // not really used...
	chr8(0, CHRRAM);

	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_reg[3] = 0;
}





/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Kaiser Board KS7058

 Games: Tui Do Woo Ma Jeung

 Writes to 0xf000-0xffff set 4k chr banks. Namely, if
 offset&0x80 is 0 the lower 4k are set, if it is 1 the
 upper 4k are set.

 iNES: mapper 171

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ks7058_device::write_h)
{
	LOG_MMC(("ks7058 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7080)
	{
		case 0x7000:
			chr4_0(data, CHRROM);
			break;
		case 0x7080:
			chr4_4(data, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 Kaiser Board KS7022

 Games: 15 in 1

 iNES: mapper 175

 In MESS: Supported?

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ks7022_device::write_h)
{
	LOG_MMC(("ks7022 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0)
		set_nt_mirroring(BIT(data, 2) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (offset == 0x2000)
		m_latch = data & 0x0f;
}

READ8_MEMBER(nes_ks7022_device::read_h)
{
	LOG_MMC(("ks7022 read_h, offset: %04x\n", offset));

	if (offset == 0x7ffc)
	{
		chr8(m_latch, CHRROM);
		prg16_89ab(m_latch);
		prg16_cdef(m_latch);
	}

	return hi_access_rom(offset);
}

/*-------------------------------------------------

 Kaiser Board KS7032

 Games: A few FDS conversions like Bubble Bobble
        or SMB2

 iNES:

 TODO: available dumps do not seem to use WRAM...
 yet m_reg[4] should switch WRAM bank... investigate!

 In MESS: Supported?

 -------------------------------------------------*/

void nes_ks7032_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (m_irq_count == 0xffff)
			{
				m_irq_enable = 0;
				m_irq_count = 0;
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			}
			else
				m_irq_count++;
		}
	}
}

void nes_ks7032_device::prg_update()
{
	prg8_89(m_reg[1]);
	prg8_ab(m_reg[2]);
	prg8_cd(m_reg[3]);
}

WRITE8_MEMBER(nes_ks7032_device::ks7032_write)
{
	LOG_MMC(("ks7032_write, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_irq_count = (m_irq_count & 0xfff0) | (data & 0x0f);
			break;
		case 0x1000:
			m_irq_count = (m_irq_count & 0xff0f) | ((data & 0x0f) << 4);
			break;
		case 0x2000:
			m_irq_count = (m_irq_count & 0xf0ff) | ((data & 0x0f) << 8);
			break;
		case 0x3000:
			m_irq_count = (m_irq_count & 0x0fff) | ((data & 0x0f) << 12);
			break;
		case 0x4000:
			m_irq_enable = 1;
			break;
		case 0x6000:
			m_latch = data & 0x07;
			break;
		case 0x7000:
			m_reg[m_latch] = data;
			prg_update();
			break;
	}
}

READ8_MEMBER(nes_ks7032_device::read_m)
{
	LOG_MMC(("ks7032 read_m, offset: %04x\n", offset));
	return m_prg[((m_reg[4] * 0x2000) + (offset & 0x1fff)) & (m_prg_size - 1)];
}

/*-------------------------------------------------

 Kaiser Board KS202

 Games: Super Mario Bros. 3 (Pirate, Alt)

 iNES:

 In MESS: Supported?

 -------------------------------------------------*/


WRITE8_MEMBER(nes_ks202_device::write_h)
{
	LOG_MMC(("ks202 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x7000:
			m_reg[m_latch] = data;
			prg_update();
			switch (offset & 0xc00)
			{
				case 0x800:
					set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
					break;
				case 0xc00:
					chr1_x(offset & 0x07, data, CHRROM);
					break;
			}
			break;
		default:
			ks7032_write(space, offset, data, mem_mask);
			break;
	}
}

READ8_MEMBER(nes_ks202_device::read_m)
{
	LOG_MMC(("ks202 read_m, offset: %04x\n", offset));
	return m_prgram[offset & 0x1fff];
}

/*-------------------------------------------------

 Kaiser Board KS7017

 Games: Almana no Kiseki FDS conversion

 iNES:

 In MESS: Supported.

 -------------------------------------------------*/

void nes_ks7017_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			if (!m_irq_count)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_enable = 0;
				m_irq_status |= 0x01;
			}
			else
				m_irq_count--;
		}
	}
}

WRITE8_MEMBER(nes_ks7017_device::write_l)
{
	LOG_MMC(("ks7017 write_l, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;

	if (offset >= 0xa00 && offset < 0xb00)
		m_latch = ((offset >> 2) & 0x03) | ((offset >> 4) & 0x04);

	if (offset >= 0x1100 && offset < 0x1200)
		prg16_89ab(m_latch);
}

WRITE8_MEMBER(nes_ks7017_device::write_ex)
{
	LOG_MMC(("ks7017 write_ex, offset: %04x, data: %02x\n", offset, data));
	offset += 0x20;

	if (offset == 0x0020) // 0x4020
		m_irq_count = (m_irq_count & 0xff00) | data;

	if (offset == 0x0021) // 0x4021
	{
		m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
		m_irq_enable = 1;
	}

	if (offset == 0x0025) // 0x4025
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

READ8_MEMBER(nes_ks7017_device::read_ex)
{
	LOG_MMC(("ks7017 read_ex, offset: %04x\n", offset));
	offset += 0x20;

	if (offset == 0x0030) // 0x4030
	{
		int temp = m_irq_status;
		m_irq_status &= ~0x01;
		return temp;
	}

	return m_open_bus;   // open bus
}

/*-------------------------------------------------

 Kaiser Board KS7012

 Games: Zanac FDS Conversion

 iNES:

 In MESS: Not working

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ks7012_device::write_h)
{
	LOG_MMC(("ks7012 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x60a0)
		prg32(0);
	if (offset == 0x6e36)
		prg32(1);
}


/*-------------------------------------------------

 Kaiser Board KS7013B

 Games: Highway Star FDS Conversion

 iNES:

 In MESS: Supported.

 -------------------------------------------------*/

WRITE8_MEMBER(nes_ks7013b_device::write_m)
{
	LOG_MMC(("ks7013b write_l, offset: %04x, data: %02x\n", offset, data));
	prg16_89ab(data);
}

WRITE8_MEMBER(nes_ks7013b_device::write_h)
{
	LOG_MMC(("ks7013b write_h, offset: %04x, data: %02x\n", offset, data));
	set_nt_mirroring((data & 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}


/*-------------------------------------------------

 Kaiser Board KS7031

 Games: Dracula II FDS Conversion

 This board is quite weird. It handles 2K PRG chunks
 and the chip contains chunks in reverse order, so
 that the first 2K are actually loaded at the top
 of the 0x8000-0xffff region. Main bank is fixed, while
 the 8K mapped at 0x6000-0x7fff varies with reg writes.

 TODO: understand how SRAM is handled...

 iNES:

 In MESS: Supported.

 -------------------------------------------------*/

READ8_MEMBER(nes_ks7031_device::read_m)
{
//  LOG_MMC(("ks7031 read_m, offset: %04x\n", offset));
	return m_prg[(m_reg[(offset >> 11) & 3] * 0x0800) + (offset & 0x7ff)];
}

READ8_MEMBER(nes_ks7031_device::read_h)
{
	// here the first 32K are accessed, but in 16x2K blocks loaded in reverse order
	int accessed_2k = (offset >> 11) & 0x0f;
	return m_prg[((0x0f - accessed_2k) * 0x0800) + (offset & 0x7ff)];
}

WRITE8_MEMBER(nes_ks7031_device::write_h)
{
	LOG_MMC(("ks7031 write_h, offset: %04x, data: %02x\n", offset, data));
	m_reg[(offset >> 11) & 3] = data & 0x3f;
}
