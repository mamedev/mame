// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Hosenkan PCBs


 ***********************************************************************************************************/


#include "emu.h"
#include "hosenkan.h"

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

const device_type NES_HOSENKAN = &device_creator<nes_hosenkan_device>;


nes_hosenkan_device::nes_hosenkan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_HOSENKAN, "NES Cart HOSENKAN PCB", tag, owner, clock, "nes_hosenkan", __FILE__), 
	m_irq_count(0), 
	m_irq_count_latch(0), 
	m_irq_clear(0), 
	m_irq_enable(0), 
	m_latch(0)
				{
}




void nes_hosenkan_device::device_start()
{
	common_start();
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_irq_clear));
	save_item(NAME(m_latch));
}

void nes_hosenkan_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg32((m_prg_chunks - 1) >> 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	m_irq_enable = 0;
	m_irq_count = m_irq_count_latch = 0;
	m_irq_clear = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Hosenkan

 Games: Pocahontas, Super Donkey Kong

 iNES: mapper 182

 In MESS: Supported.

 -------------------------------------------------*/

// same as MMC3!
void nes_hosenkan_device::hblank_irq( int scanline, int vblank, int blanked )
{
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		int prior_count = m_irq_count;
		if ((m_irq_count == 0) || m_irq_clear)
			m_irq_count = m_irq_count_latch;
		else
			m_irq_count--;

		if (m_irq_enable && !blanked && (m_irq_count == 0) && (prior_count || m_irq_clear))
		{
			LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
						machine().first_screen()->vpos(), machine().first_screen()->hpos()));
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
	}
	m_irq_clear = 0;
}

WRITE8_MEMBER(nes_hosenkan_device::write_h)
{
	LOG_MMC(("hosenkan write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0001:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			m_latch = data;
			break;
		case 0x4000:
			switch (m_latch)
			{
				case 0:
					chr2_0(data >> 1, CHRROM);
					break;
				case 1:
					chr1_5(data, CHRROM);
					break;
				case 2:
					chr2_2(data >> 1, CHRROM);
					break;
				case 3:
					chr1_7(data, CHRROM);
					break;
				case 4:
					prg8_89(data);
					break;
				case 5:
					prg8_ab(data);
					break;
				case 6:
					chr1_4(data, CHRROM);
					break;
				case 7:
					chr1_6(data, CHRROM);
					break;
			}
			break;
		case 0x6003:
			if (data)
			{
				m_irq_count = data;
				m_irq_enable = 1;
			}
			else
				m_irq_enable = 0;
			break;
	}
}
