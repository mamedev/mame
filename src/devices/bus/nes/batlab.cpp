// license:BSD-3-Clause
// copyright-holders: kmg, Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Batlab Electronics and related PCBs


 Here we emulate the following homebrew PCBs

 * Batlab BATMAP-000 [mapper 399]
 * Batlab BATMAP-SRR-X [mapper 413]

 ***********************************************************************************************************/


#include "emu.h"
#include "batlab.h"

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

DEFINE_DEVICE_TYPE(NES_BATMAP_000,  nes_batmap_000_device,  "nes_batmap_000",  "NES Cart Batlab BATMAP-000 PCB")
DEFINE_DEVICE_TYPE(NES_BATMAP_SRRX, nes_batmap_srrx_device, "nes_batmap_srrx", "NES Cart Batlab BATMAP-SRR-X PCB")


nes_batmap_000_device::nes_batmap_000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BATMAP_000, tag, owner, clock)
{
}

nes_batmap_srrx_device::nes_batmap_srrx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_BATMAP_SRRX, tag, owner, clock), m_reg(0), m_dpcm_addr(0), m_dpcm_ctrl(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0)
{
}



void nes_batmap_000_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	mmc3_common_initialize(0x0f, 0xff, 0);

	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRRAM);
}

void nes_batmap_srrx_device::device_start()
{
	common_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_dpcm_addr));
	save_item(NAME(m_dpcm_ctrl));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_batmap_srrx_device::pcb_reset()
{
	prg8_89(0);
	prg8_ab(0);
	prg8_cd(3);              // 0xd000-0xffff fixed PRG bank
	prg8_ef(4);
	chr4_0(0, CHRROM);
	chr4_4(0x3d, CHRROM);    // fixed CHR bank

	m_reg = 0;
	m_dpcm_addr = 0;
	m_dpcm_ctrl = 0;

	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Batlab BATMAP-000 board

 Games: Star Versus, Anamanaguchi carts? (not dumped?)

 These boards have been seen using a Xilinx XC9572XL CPLD.
 This provides a clone of MMC3-style IRQ and mirroring
 (not sure how it may differ) with simpler PRG/CHR banking.
 Boards are marked as having 1MB PRG, swappable in 8K banks
 at 0xa000 and 0xc000, and 32KB CHR, swappable in 4K banks.

 NES 2.0: mapper 399

 In MAME: Supported.

 -------------------------------------------------*/

void nes_batmap_000_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("batmap_000 write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			chr4_x(BIT(data, 7) << 2, data & 0x07, CHRRAM);
			break;
		case 0x0001:
			prg8_x(BIT(data, 7) + 1, data & 0x7f);
			break;
		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Batlab BATMAP-SRR-X board

 Games: Super Russian Roulette

 According to dumper it uses a CPLD. This provides PRG
 and CHR banking, MMC3-like scanline IRQ, and access
 to an 8MB ROM of DPCM for streaming digitized speech.

 The memory map for reads is as follows:

   0x4800~    read DPCM data byte (and data pointer++)
   0x5000~    4K fixed to PRG 0x1000
   0x6000~    8K, reg 0
   0x8000~    8K, reg 1
   0xA000~    8K, reg 2
   0xC000~    Same as 0x4800
   0xD000~    12K fixed to PRG 0x7000-0xffff

 The DPCM ROM data pointer is at least 23-bits wide
 and is written serially, by bit, at 0xc000 using
 the MSB of each write. There is a control register (?)
 at 0xd000. It's not clear what it does since for each
 complete sound played it is always written with the
 sequence 0x01, 0x00, 0x02. Other sources say bit 1 is
 a flag that turns on/off incrementing of the pointer.

 NES 2.0: mapper 413

 In MAME: Supported.

 -------------------------------------------------*/

// IRQ based on our MMC3 implementation, details for SRR are unclear
void nes_batmap_srrx_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
	{
		if (!m_irq_count)
			m_irq_count = m_irq_count_latch;
		else
			m_irq_count--;

		if (m_irq_enable && !blanked && !m_irq_count)
		{
			LOG_MMC(("irq fired, scanline: %d\n", scanline));
			set_irq_line(ASSERT_LINE);
		}
	}
}

u8 nes_batmap_srrx_device::read_dpcm()
{
	m_dpcm_addr &= m_misc_rom_size - 1;
	return m_misc_rom[m_dpcm_addr++];
}

u8 nes_batmap_srrx_device::read_l(offs_t offset)
{
//	LOG_MMC(("batmap_srrx read_l, offset: %04x", offset));

	offset += 0x100;
	switch (offset & 0x1800)
	{
		case 0x0000:
			return get_open_bus();
		case 0x0800:
			return read_dpcm();
		default:
			return m_prg[offset];    // fixed to second 4K
	}
}

u8 nes_batmap_srrx_device::read_m(offs_t offset)
{
//	LOG_MMC(("batmap_srrx read_m, offset: %04x", offset));
	return m_prg[(m_reg * 0x2000 + offset) & (m_prg_size - 1)];
}

u8 nes_batmap_srrx_device::read_h(offs_t offset)
{
	LOG_MMC(("batmap_srrx read_h, offset: %04x", offset));

	if ((offset & 0x7000) == 0x4000)
		return read_dpcm();
	else
		return hi_access_rom(offset);
}

void nes_batmap_srrx_device::write_h(offs_t offset, u8 data)
{
	LOG_MMC(("batmap_srrx write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_irq_count_latch = data;
			break;
		case 0x1000:
			m_irq_count = 0;
			break;
		case 0x2000:
		case 0x3000:
			m_irq_enable = BIT(offset, 12);
			if (!m_irq_enable)
				set_irq_line(CLEAR_LINE);
			break;
		case 0x4000:
			m_dpcm_addr = (m_dpcm_addr << 1 | data >> 7);
			break;
		case 0x5000:
			m_dpcm_ctrl = data;
			break;
		case 0x6000:
		case 0x7000:
			switch (data >> 6)
			{
				case 0: m_reg = data & 0x3f; break;
				case 1: prg8_89(data & 0x3f); break;
				case 2: prg8_ab(data & 0x3f); break;
				case 3: chr4_0(data & 0x3f, CHRROM); break;
			}
			break;
	}
}
