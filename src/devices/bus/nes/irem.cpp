// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for IREM PCBs


 Here we emulate the following PCBs

 * IREM LROG017 [mapper 77]
 * IREM Holy Diver [mapper 78]
 * IREM TAM-S1 [mapper 97]
 * IREM G101 [mapper 32]
 * IREM H3001 [mapper 65]

 ***********************************************************************************************************/


#include "emu.h"
#include "irem.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_LROG017,  nes_lrog017_device,  "nes_lrog017",  "NES Cart Irem Discrete 74*161/161/21/138 PCB")
DEFINE_DEVICE_TYPE(NES_HOLYDIVR, nes_holydivr_device, "nes_holydivr", "NES Cart Irem Holy Diver PCB")
DEFINE_DEVICE_TYPE(NES_TAM_S1,   nes_tam_s1_device,   "nes_tam_s1",   "NES Cart Irem TAM-S1 PCB")
DEFINE_DEVICE_TYPE(NES_G101,     nes_g101_device,     "nes_g101",     "NES Cart Irem G-101 PCB")
DEFINE_DEVICE_TYPE(NES_H3001,    nes_h3001_device,    "ns_h3001",     "NES Cart Irem H-3001 PCB")


nes_lrog017_device::nes_lrog017_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_LROG017, tag, owner, clock)
{
}

nes_holydivr_device::nes_holydivr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_HOLYDIVR, tag, owner, clock)
{
}

nes_tam_s1_device::nes_tam_s1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_TAM_S1, tag, owner, clock)
{
}

nes_g101_device::nes_g101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_G101, tag, owner, clock), m_latch(0)
{
}

nes_h3001_device::nes_h3001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_H3001, tag, owner, clock), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), irq_timer(nullptr)
{
}




void nes_lrog017_device::device_start()
{
	common_start();
}

void nes_lrog017_device::pcb_reset()
{
	prg32(0);
	chr2_0(0, CHRROM);
	chr2_2(0, CHRRAM);
	chr2_4(1, CHRRAM);
	chr2_6(2, CHRRAM);
}

void nes_holydivr_device::device_start()
{
	common_start();
}

void nes_holydivr_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
}

void nes_tam_s1_device::device_start()
{
	common_start();
}

void nes_tam_s1_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(m_prg_chunks - 1);
	prg16_cdef(0);
	chr8(0, m_chr_source);
}

void nes_g101_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
}

void nes_g101_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
}

void nes_h3001_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_h3001_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Irem LROG017 - Discrete board emulation (74*161/161/21/138)

 Games: Napoleon Senki

 iNES: mapper 77

 This board should use 2KB of CHRRAM as NTRAM, instead
 of using directly 4-screen mirroring, but for the
 moment we cheat in this way...

 -------------------------------------------------*/

void nes_lrog017_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("lrog017 write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	prg32(data);
	chr2_0((data >> 4), CHRROM);
}

/*-------------------------------------------------

 Irem Holy Diver board emulation

 iNES: mapper 78 (shared with JF-16)

 -------------------------------------------------*/

void nes_holydivr_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("holy diver write_h, offset: %04x, data: %02x\n", offset, data));

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	chr8(data >> 4, CHRROM);
	prg16_89ab(data);
}

/*-------------------------------------------------

 Irem TAM-S1 board emulation

 Games: Kaiketsu Yanchamaru

 iNES: mapper 97

 In MESS: Supported.

 -------------------------------------------------*/

void nes_tam_s1_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("tam s1 write_h, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		// this pcb is subject to bus conflict
		data = account_bus_conflict(offset, data);

		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
		prg16_cdef(data);
	}
}

/*-------------------------------------------------

 Irem G-101 board emulation

 Major League uses hardwired mirroring

 iNES: mapper 32

 -------------------------------------------------*/

void nes_g101_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("g101 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			if (m_latch)
			{
				prg8_89(0xfe);
				prg8_cd(data & 0x1f);
			}
			else
			{
				prg8_89(data & 0x1f);
				prg8_cd(0xfe);
			}
			break;
		case 0x1000:
			m_latch = BIT(data, 1);
			if (m_pcb_ctrl_mirror)
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			prg8_ab(data & 0x1f);
			break;
		case 0x3000:
			chr1_x(offset & 0x07, data & 0x7f, CHRROM);
			break;
	}
}

/*-------------------------------------------------

 Irem H-3001 board emulation

 Games: Daiku no Gen San 2 - Akage no Dan no Gyakushuu,
 Kaiketsu Yanchamaru 3, Spartan X 2

 iNES: mapper 65

 In MESS: Supported.

 -------------------------------------------------*/


void nes_h3001_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable)
		{
			// 16bit counter, IRQ fired when the counter reaches 0
			// after firing, the counter is *not* reloaded and does not wrap
			if (m_irq_count > 0)
				m_irq_count--;

			if (!m_irq_count)
			{
				set_irq_line(ASSERT_LINE);
				m_irq_enable = 0;
			}
		}
	}
}

void nes_h3001_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("h3001 write_h, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7fff)
	{
		case 0x0000:
			prg8_89(data);
			break;

		case 0x1001:
			set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x1003:
			m_irq_enable = data & 0x80;
			set_irq_line(CLEAR_LINE);
			break;

		case 0x1004:
			m_irq_count = m_irq_count_latch;
			set_irq_line(CLEAR_LINE);
			break;

		case 0x1005:
			m_irq_count_latch = (m_irq_count_latch & 0x00ff) | (data << 8);
			break;

		case 0x1006:
			m_irq_count_latch = (m_irq_count_latch & 0xff00) | data;
			break;

		case 0x2000:
			prg8_ab(data);
			break;

		case 0x3000: case 0x3001: case 0x3002: case 0x3003:
		case 0x3004: case 0x3005: case 0x3006: case 0x3007:
			chr1_x(offset & 0x07, data, CHRROM);
			break;

		case 0x4000:
			prg8_cd(data);
			break;

		default:
			break;
	}
}
