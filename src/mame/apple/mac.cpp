// license:BSD-3-Clause
// copyright-holders: Nathan Woods, Raphael Nabet, R. Belmont, O. Galibert
/****************************************************************************

    drivers/mac.cpp
    Macintosh II family emulation

    TODO:
        - V8 and friends (LC, LC2, Classic 2, Color Classic) to own driver
        - IIvx / IIvi to own driver

****************************************************************************/

#include "emu.h"
#include "mac.h"

#include "macadb.h"

#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "machine/applepic.h"
#include "machine/iwm.h"
#include "machine/swim1.h"
#include "machine/swim2.h"

#include "layout/generic.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/ap_dsk35.h"


#define C32M    (31.3344_MHz_XTAL)
#define C15M    (C32M/2)
#define C7M     (C32M/4)

// do this here - screen_update is called each scanline when stepping in the
// debugger, which means you can't escape the VIA2 IRQ handler
//
// RBV/MDU bits in IER/IFR:
//
// CA1: any slot interrupt = 0x02
// CA2: SCSI interrupt     = 0x01
// CB1: ASC interrupt      = 0x10

WRITE_LINE_MEMBER(mac_state::mac_rbv_vbl)
{
	if (!state)
		return;

	m_rbv_regs[2] &= ~0x40; // set vblank signal
	m_rbv_vbltime = 10;

//  printf("RBV: raising VBL!\n");

	if (m_rbv_regs[0x12] & 0x40)
	{
		rbv_recalc_irqs();
	}
}

uint32_t mac_state::rbv_ramdac_r()
{
	return 0;
}

void mac_state::rbv_ramdac_w(offs_t offset, uint32_t data)
{
	if (!offset)
	{
		m_rbv_clutoffs = data>>24;
		m_rbv_count = 0;
	}
	else
	{
		m_rbv_colors[m_rbv_count++] = data>>24;

		if (m_rbv_count == 3)
		{
			// for portrait display, force monochrome by using the blue channel
			if (m_model != MODEL_MAC_CLASSIC_II)
			{
				// Color Classic has no MONTYPE so the default gets us 512x384, which is right
				if (m_montype.read_safe(2) == 1)
				{
					m_palette->set_pen_color(m_rbv_clutoffs, rgb_t(m_rbv_colors[2], m_rbv_colors[2], m_rbv_colors[2]));
					m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[2], m_rbv_colors[2], m_rbv_colors[2]);
					m_rbv_clutoffs++;
					m_rbv_count = 0;
				}
				else
				{
					m_palette->set_pen_color(m_rbv_clutoffs, rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]));
					m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]);
					m_rbv_clutoffs++;
					m_rbv_count = 0;
				}
			}
		}
	}
}

void mac_state::ariel_ramdac_w(offs_t offset, uint32_t data, uint32_t mem_mask) // this is for the "Ariel" style RAMDAC
{
	if (mem_mask == 0xff000000)
	{
		m_rbv_clutoffs = data>>24;
		m_rbv_count = 0;
	}
	else if (mem_mask == 0x00ff0000)
	{
		m_rbv_colors[m_rbv_count++] = (data>>16) & 0xff;

		if (m_rbv_count == 3)
		{
			// for portrait display, force monochrome by using the blue channel
			if (m_model != MODEL_MAC_CLASSIC_II)
			{
				// Color Classic has no MONTYPE so the default gets us 512x384, which is right
				if (m_montype.read_safe(2) == 1)
				{
					m_palette->set_pen_color(m_rbv_clutoffs, rgb_t(m_rbv_colors[2], m_rbv_colors[2], m_rbv_colors[2]));
					m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[2], m_rbv_colors[2], m_rbv_colors[2]);
					m_rbv_clutoffs = (m_rbv_clutoffs + 1) & 0xff;
					m_rbv_count = 0;
				}
				else
				{
					m_palette->set_pen_color(m_rbv_clutoffs, rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]));
					m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]);
					m_rbv_clutoffs = (m_rbv_clutoffs + 1) & 0xff;
					m_rbv_count = 0;
				}
			}
		}
	}
	else if (mem_mask == 0x0000ff00)
	{
		// config reg
//      printf("Ariel: %02x to config\n", (data>>8)&0xff);
	}
	else    // color key reg
	{
	}
}

void mac_state::rbv_recalc_irqs()
{
	// check slot interrupts and bubble them down to IFR
	uint8_t slot_irqs = (~m_rbv_regs[2]) & 0x78;
	slot_irqs &= (m_rbv_regs[0x12] & 0x78);

	if (slot_irqs)
	{
		m_rbv_regs[3] |= 2; // any slot
	}
	else    // no slot irqs, clear the pending bit
	{
		m_rbv_regs[3] &= ~2; // any slot
	}

	uint8_t ifr = (m_rbv_regs[3] & m_rbv_ier) & 0x1b; //m_rbv_regs[0x13]);

//  printf("ifr = %02x (reg3 %02x reg13 %02x)\n", ifr, m_rbv_regs[3], m_rbv_regs[0x13]);
	if (ifr != 0)
	{
		m_rbv_regs[3] = ifr | 0x80;
		m_rbv_ifr = ifr | 0x80;

//      printf("VIA2 raise\n");
		set_via2_interrupt(1);
	}
	else
	{
//      printf("VIA2 lower\n");
		set_via2_interrupt(0);
	}
}

uint8_t mac_state::mac_rbv_r(offs_t offset)
{
	int data = 0;

	if (offset < 0x100)
	{
		data = m_rbv_regs[offset];

		if (offset == 0x10)
		{
			data &= ~0x38;
			data |= (m_montype.read_safe(2)<<3);
//            printf("%s rbv_r montype: %02x\n", machine().describe_context().c_str(), data);
		}

		// bit 7 of these registers always reads as 0 on RBV
		if ((offset == 0x12) || (offset == 0x13))
		{
			data &= ~0x80;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
			case 13:    // IFR
//              printf("Read IER = %02x (PC=%x) 2=%02x\n", m_rbv_ier, m_maincpu->pc(), m_rbv_regs[2]);
				data = m_rbv_ifr;
				break;

			case 14:    // IER
//              printf("Read IFR = %02x (PC=%x) 2=%02x\n", m_rbv_ifr, m_maincpu->pc(), m_rbv_regs[2]);
				data = m_rbv_ier;
				break;

			default:
				logerror("rbv_r: Unknown extended RBV VIA register %d access\n", offset);
				break;
		}
	}

//  printf("rbv_r: %x = %02x (PC=%x)\n", offset, data, m_maincpu->pc());

	return data;
}

void mac_state::mac_rbv_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
//      if (offset == 0x10)
//      printf("rbv_w: %02x to offset %x (PC=%x)\n", data, offset, m_maincpu->pc());
		switch (offset)
		{
			case 0x00:
				if (m_model == MODEL_MAC_LC)
				{
					m68000_base_device *m68k = downcast<m68000_base_device *>(m_maincpu.target());
					m68k->set_hmmu_enable((data & 0x8) ? M68K_HMMU_DISABLE : M68K_HMMU_ENABLE_LC);
				}
				break;

			case 0x01:
				if (((data & 0xc0) != (m_rbv_regs[1] & 0xc0)) && (m_rbv_type == RBV_TYPE_V8))
				{
					m_rbv_regs[1] = data;
					this->v8_resize();
				}
				break;

			case 0x02:
				data &= 0x40;
				m_rbv_regs[offset] &= ~data;
				rbv_recalc_irqs();
				break;

			case 0x03:  // write here to ack
				if (data & 0x80)    // 1 bits write 1s
				{
					m_rbv_regs[offset] |= data & 0x7f;
					m_rbv_ifr |= data & 0x7f;
				}
				else            // 1 bits write 0s
				{
					m_rbv_regs[offset] &= ~(data & 0x7f);
					m_rbv_ifr &= ~(data & 0x7f);
				}
				rbv_recalc_irqs();
				break;

			case 0x10:
				if (data != 0)
				{
					m_rbv_immed10wr = 1;
				}
				m_rbv_regs[offset] = data;
				break;

			case 0x12:
				if (data & 0x80)    // 1 bits write 1s
				{
					m_rbv_regs[offset] |= data & 0x7f;
				}
				else            // 1 bits write 0s
				{
					m_rbv_regs[offset] &= ~(data & 0x7f);
				}
				rbv_recalc_irqs();
				break;

			case 0x13:
				if (data & 0x80)    // 1 bits write 1s
				{
					m_rbv_regs[offset] |= data & 0x7f;

					if (data == 0xff) m_rbv_regs[offset] = 0x1f;    // I don't know why this is special, but the IIci ROM's POST demands it
				}
				else            // 1 bits write 0s
				{
					m_rbv_regs[offset] &= ~(data & 0x7f);
				}
				break;

			default:
				m_rbv_regs[offset] = data;
				break;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
			case 13:    // IFR
//              printf("%02x to IFR (PC=%x)\n", data, m_maincpu->pc());
				if (data & 0x80)
				{
					data = 0x7f;
				}
				rbv_recalc_irqs();
				break;

			case 14:    // IER
//              printf("%02x to IER (PC=%x)\n", data, m_maincpu->pc());
				if (data & 0x80)    // 1 bits write 1s
				{
					m_rbv_ier |= data & 0x7f;
				}
				else        // 1 bits write 0s
				{
					m_rbv_ier &= ~(data & 0x7f);
				}
				rbv_recalc_irqs();
				break;

			default:
				logerror("rbv_w: Unknown extended RBV VIA register %d access\n", offset);
				break;
		}
	}
}

uint16_t mac_state::mac_config_r()
{
	return 0xffff;  // returns nonzero if no PDS RAM expansion, 0 if present
}

// IIfx
uint32_t mac_state::biu_r(offs_t offset, uint32_t mem_mask)
{
//  printf("biu_r @ %x, mask %08x\n", offset, mem_mask);
	return 0;
}

void mac_state::biu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("biu_w %x @ %x, mask %08x\n", data, offset, mem_mask);
}

template <int N> WRITE_LINE_MEMBER(mac_state::oss_interrupt)
{
	if (state == ASSERT_LINE)
		m_oss_regs[N >= 8 ? 0x202 : 0x203] |= 1 << (N & 7);
	else
		m_oss_regs[N >= 8 ? 0x202 : 0x203] &= ~(1 << (N & 7));

	int take_interrupt = 0;
	for (int n = 0; n < 8; n++)
	{
		if (BIT(m_oss_regs[0x203], n) && take_interrupt < m_oss_regs[n])
			take_interrupt = m_oss_regs[n];
		if (BIT(m_oss_regs[0x202], n) && take_interrupt < m_oss_regs[8 + n])
			take_interrupt = m_oss_regs[8 + n];
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
		m_oss_regs[0x200] &= 0x7f;
	}

	if (take_interrupt > 0)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
		m_oss_regs[0x200] |= 0x80;
	}
}

TIMER_CALLBACK_MEMBER(mac_state::oss_6015_tick)
{
	m_via1->write_ca1(0);
	m_via1->write_ca1(1);
	oss_interrupt<10>(ASSERT_LINE);
}

uint8_t mac_state::oss_r(offs_t offset)
{
//  printf("oss_r @ %x\n", offset);
//  if (offset <= 0xe)  // for interrupt mask registers, we're intended to return something different than is written in the low 3 bits (?)
//  {
//      return m_oss_regs[offset]<<4;
//  }

	if (offset < std::size(m_oss_regs))
		return m_oss_regs[offset];
	else
		return 0;
}

void mac_state::oss_w(offs_t offset, uint8_t data)
{
//  printf("oss_w %x @ %x\n", data, offset);
	if (offset == 0x207)
		oss_interrupt<10>(CLEAR_LINE);
	else if (offset < std::size(m_oss_regs))
		m_oss_regs[offset] = data;
}

uint32_t mac_state::buserror_r()
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
}

uint8_t mac_state::maciifx_8010_r()
{
	return 0x40;
}

uint8_t mac_state::maciifx_8040_r()
{
	return 0;
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mac_state::maclc_map(address_map &map)
{
	map.global_mask(0x80ffffff); // V8 uses bit 31 and 23-0 for address decoding only

	map(0xa00000, 0xafffff).rom().region("bootrom", 0); // ROM (in 32-bit mode)

	map(0xf00000, 0xf01fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0xf04000, 0xf05fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w));
	map(0xf06000, 0xf07fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w));
	map(0xf10000, 0xf11fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w));
	map(0xf12000, 0xf13fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w));
	map(0xf14000, 0xf15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0xf16000, 0xf17fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0xf24000, 0xf24003).rw(FUNC(mac_state::rbv_ramdac_r), FUNC(mac_state::ariel_ramdac_w));
	map(0xf26000, 0xf27fff).rw(FUNC(mac_state::mac_rbv_r), FUNC(mac_state::mac_rbv_w));    // VIA2 (V8)
	map(0xf40000, 0xfbffff).ram().share("vram");
}

void mac_state::maclc3_map(address_map &map)
{
	map(0x40000000, 0x400fffff).rom().region("bootrom", 0).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x00f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00f00000);
	map(0x50024000, 0x50025fff).w(FUNC(mac_state::ariel_ramdac_w)).mirror(0x00f00000);
	map(0x50026000, 0x50027fff).rw(FUNC(mac_state::mac_rbv_r), FUNC(mac_state::mac_rbv_w)).mirror(0x00f00000);

	map(0x5ffffffc, 0x5fffffff).r(FUNC(mac_state::mac_read_id));

	map(0x60000000, 0x600fffff).ram().mirror(0x0ff00000).share("vram");
}

void mac_state::macii_map(address_map &map)
{
	map(0x40000000, 0x4003ffff).rom().region("bootrom", 0).mirror(0x0ffc0000);

	// MMU remaps I/O without the F
	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50002000, 0x50003fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x00f00000);
	map(0x50006000, 0x50006003).w(FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50006060, 0x50006063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00f00000);
	map(0x50040000, 0x50041fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
}

void mac_state::maciici_map(address_map &map)
{
	map(0x40000000, 0x4007ffff).r(FUNC(mac_state::rom_switch_r)).mirror(0x0ff80000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x00f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00f00000);
	map(0x50024000, 0x50024007).w(FUNC(mac_state::rbv_ramdac_w)).mirror(0x00f00000);
	map(0x50026000, 0x50027fff).rw(FUNC(mac_state::mac_rbv_r), FUNC(mac_state::mac_rbv_w)).mirror(0x00f00000);
	map(0x50040000, 0x50041fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
}

void mac_state::macse30_map(address_map &map)
{
	map(0x40000000, 0x4003ffff).rom().region("bootrom", 0).mirror(0x0ffc0000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50002000, 0x50003fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x00f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00f00000);
	map(0x50040000, 0x50041fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000); // mirror

	map(0xfe000000, 0xfe00ffff).ram().share("vram");
	map(0xfee00000, 0xfee0ffff).ram().share("vram").mirror(0x000f0000);
	map(0xfeffe000, 0xfeffffff).rom().region("se30vrom", 0x0);
}

void mac_state::maciifx_map(address_map &map)
{
	map(0x40000000, 0x4007ffff).rom().region("bootrom", 0).mirror(0x0ff80000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw("sccpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0xff00ff00);
	map(0x50004000, 0x50005fff).rw("sccpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0x00ff00ff);
	map(0x50008010, 0x50008010).r(FUNC(mac_state::maciifx_8010_r)).mirror(0x00f00000);
	map(0x50008040, 0x50008040).r(FUNC(mac_state::maciifx_8040_r)).mirror(0x00f00000);
	map(0x5000a000, 0x5000bfff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x5000c060, 0x5000c063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
	map(0x5000d000, 0x5000d003).w(FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x5000d060, 0x5000d063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw("swimpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0xff00ff00);
	map(0x50012000, 0x50013fff).rw("swimpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0x00ff00ff);
	map(0x50018000, 0x50019fff).rw(FUNC(mac_state::biu_r), FUNC(mac_state::biu_w)).mirror(0x00f00000);
	map(0x5001a000, 0x5001bfff).rw(FUNC(mac_state::oss_r), FUNC(mac_state::oss_w)).mirror(0x00f00000);
	map(0x50024000, 0x50027fff).r(FUNC(mac_state::buserror_r)).mirror(0x00f00000);   // must bus error on access here so ROM can determine we're an FMC
	map(0x50040000, 0x50041fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static void mac_lcpds_cards(device_slot_interface &device)
{
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void mac_state::add_base_devices(machine_config &config, bool rtc, int woz_version)
{
	if (rtc)
	{
		RTC3430042(config, m_rtc, XTAL(32'768));
		m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));
	}

	switch (woz_version) {
	case 0:
		IWM(config, m_fdc, C15M);
		m_fdc->devsel_cb().set(FUNC(mac_state::devsel_w));
		break;
	case 1:
		SWIM1(config, m_fdc, C15M);
		m_fdc->devsel_cb().set(FUNC(mac_state::devsel_w));
		break;
	case 2:
		SWIM2(config, m_fdc, C15M);
		m_fdc->hdsel_cb().set(FUNC(mac_state::hdsel_w));
		m_fdc->devsel_cb().set(FUNC(mac_state::devsel_w));
		break;
	}

	m_fdc->phases_cb().set(FUNC(mac_state::phases_w));
	m_fdc->sel35_cb().set(FUNC(mac_state::sel35_w));

	if (woz_version == 0)
	{
		applefdintf_device::add_35(config, m_floppy[0]);
		applefdintf_device::add_35_nc(config, m_floppy[1]);
	}
	else
	{
		applefdintf_device::add_35_hd(config, m_floppy[0]);
		applefdintf_device::add_35_nc(config, m_floppy[1]);

		SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
	}

	SCC8530(config, m_scc, C7M);
	m_scc->intrq_callback().set(FUNC(mac_state::set_scc_interrupt));
}

void mac_state::add_scsi(machine_config &config, bool cdrom)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, cdrom ? "cdrom" : nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.irq_handler().set(*this, FUNC(mac_state::mac_scsi_irq));
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(mac_state::scsi_berr_w));

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
}

void mac_state::add_via1_adb(machine_config &config, bool macii)
{
	R65NC22(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(mac_state::mac_via_in_a));
	if (macii)
		m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b_ii));
	else
		m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(mac_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_bbadb));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_adb_via_out_cb2));
	m_via1->irq_handler().set(FUNC(mac_state::mac_via_irq));
}

void mac_state::add_via2(machine_config &config)
{
	R65NC22(config, m_via2, C7M/10);
	m_via2->readpa_handler().set(FUNC(mac_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(mac_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(mac_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(mac_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(mac_state::mac_via2_irq));
}

void mac_state::add_egret(machine_config &config, int type)
{
	EGRET(config, m_egret, type);
	m_egret->reset_callback().set(FUNC(mac_state::cuda_reset_w));
	m_egret->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_egret->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_egret->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->set_mcu_mode(true);
	m_macadb->adb_data_callback().set(m_egret, FUNC(egret_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);
}

void mac_state::add_cuda(machine_config &config, int type)
{
	CUDA(config, m_cuda, type);
	m_cuda->reset_callback().set(FUNC(mac_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_cuda->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->set_mcu_mode(true);
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);
}

void mac_state::add_asc(machine_config &config, asc_device::asc_type type)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, C15M, type);
	m_asc->irqf_callback().set(FUNC(mac_state::mac_asc_irq));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);
}

void mac_state::add_nubus(machine_config &config, bool bank1, bool bank2)
{
	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(FUNC(mac_state::nubus_irq_9_w));
	nubus.out_irqa_callback().set(FUNC(mac_state::nubus_irq_a_w));
	nubus.out_irqb_callback().set(FUNC(mac_state::nubus_irq_b_w));
	nubus.out_irqc_callback().set(FUNC(mac_state::nubus_irq_c_w));
	nubus.out_irqd_callback().set(FUNC(mac_state::nubus_irq_d_w));
	nubus.out_irqe_callback().set(FUNC(mac_state::nubus_irq_e_w));
	if (bank1)
	{
		NUBUS_SLOT(config, "nb9", "nubus", mac_nubus_cards, "mdc824");
		NUBUS_SLOT(config, "nba", "nubus", mac_nubus_cards, nullptr);
		NUBUS_SLOT(config, "nbb", "nubus", mac_nubus_cards, nullptr);
	}
	if (bank2)
	{
		NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
		NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
		NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);
	}
}

template <typename T> void mac_state::add_nubus_pds(machine_config &config, const char *slot_tag, T &&opts)
{
	nubus_device &nubus(NUBUS(config, "pds", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(FUNC(mac_state::nubus_irq_9_w));
	nubus.out_irqa_callback().set(FUNC(mac_state::nubus_irq_a_w));
	nubus.out_irqb_callback().set(FUNC(mac_state::nubus_irq_b_w));
	nubus.out_irqc_callback().set(FUNC(mac_state::nubus_irq_c_w));
	nubus.out_irqd_callback().set(FUNC(mac_state::nubus_irq_d_w));
	nubus.out_irqe_callback().set(FUNC(mac_state::nubus_irq_e_w));
	NUBUS_SLOT(config, slot_tag, "pds", std::forward<T>(opts), nullptr);
}

void mac_state::macii(machine_config &config, bool cpu, asc_device::asc_type asc_type, bool nubus, bool nubus_bank1, bool nubus_bank2, int woz_version)
{
	if (cpu)
	{
		M68020PMMU(config, m_maincpu, C15M);
		m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
		m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));
	}

	PALETTE(config, m_palette).set_entries(256);

	add_asc(config, asc_type);
	add_base_devices(config, true, woz_version);
	add_scsi(config, true);
	if (nubus)
		add_nubus(config, nubus_bank1, nubus_bank2);

	add_via1_adb(config, true);
	add_via2(config);

	MACADB(config, m_macadb, C15M);
	m_macadb->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_macadb->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->adb_irq_callback().set(FUNC(mac_state::adb_irq_w));

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
}

void mac_state::maciihmu(machine_config &config)
{
	macii(config, false);

	M68020HMMU(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));
}

void mac_state::maciihd(machine_config &config)
{
	macii(config, true);

	SWIM1(config.replace(), m_fdc, C15M);
	m_fdc->phases_cb().set(FUNC(mac_state::phases_w));
	m_fdc->sel35_cb().set(FUNC(mac_state::sel35_w));
	m_fdc->devsel_cb().set(FUNC(mac_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_hd(config, m_floppy[1]);

	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
}

void mac_state::maciifx(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 40000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maciifx_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	add_asc(config, asc_device::asc_type::ASC);
	add_base_devices(config, true, 1);
	add_scsi(config);

	m_asc->irqf_callback().set(FUNC(mac_state::oss_interrupt<8>));
	subdevice<nscsi_connector>("scsi:7")->set_option_machine_config("ncr5380", [this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.irq_handler().set(*this, FUNC(mac_state::oss_interrupt<9>));
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	R65NC22(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(mac_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b_ii));
	m_via1->writepa_handler().set(FUNC(mac_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_adb_via_out_cb2));
	m_via1->irq_handler().set(FUNC(mac_state::oss_interrupt<11>));

	applepic_device &sccpic(APPLEPIC(config, "sccpic", C15M));
	sccpic.prd_callback().set(m_scc, FUNC(scc8530_legacy_device::reg_r));
	sccpic.pwr_callback().set(m_scc, FUNC(scc8530_legacy_device::reg_w));
	sccpic.hint_callback().set(FUNC(mac_state::oss_interrupt<7>));

	m_scc->intrq_callback().set("sccpic", FUNC(applepic_device::pint_w));
	//m_scc->dtr_reqa_callback().set("sccpic", FUNC(applepic_device::reqa_w));
	//m_scc->dtr_reqb_callback().set("sccpic", FUNC(applepic_device::reqb_w));

	applepic_device &swimpic(APPLEPIC(config, "swimpic", C15M));
	swimpic.prd_callback().set(m_fdc, FUNC(applefdintf_device::read));
	swimpic.pwr_callback().set(m_fdc, FUNC(applefdintf_device::write));
	swimpic.hint_callback().set(FUNC(mac_state::oss_interrupt<6>));

	m_fdc->dat1byte_cb().set("swimpic", FUNC(applepic_device::reqa_w));

	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");

	add_nubus(config);
	nubus_device &nubus(*subdevice<nubus_device>("nubus"));
	nubus.out_irq9_callback().set(FUNC(mac_state::oss_interrupt<0>));
	nubus.out_irqa_callback().set(FUNC(mac_state::oss_interrupt<1>));
	nubus.out_irqb_callback().set(FUNC(mac_state::oss_interrupt<2>));
	nubus.out_irqc_callback().set(FUNC(mac_state::oss_interrupt<3>));
	nubus.out_irqd_callback().set(FUNC(mac_state::oss_interrupt<4>));
	nubus.out_irqe_callback().set(FUNC(mac_state::oss_interrupt<5>));
}

void mac_state::maclc(machine_config &config, bool cpu, bool egret, asc_device::asc_type asc_type, int woz_version)
{
	macii(config, false, asc_type, false, true, true, woz_version);

	if (cpu)
	{
		M68020HMMU(config, m_maincpu, C15M);
		m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maclc_map);
		m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));
	}

	MCFG_VIDEO_START_OVERRIDE(mac_state,macv8)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_size(1024, 768);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macv8));
	m_screen->screen_vblank().set(FUNC(mac_state::mac_rbv_vbl));
	config.set_default_layout(layout_monitors);

	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M,10M");

	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_egadb));

	add_nubus_pds(config, "lcpds", mac_lcpds_cards);

	if (egret)
	{
		add_egret(config, EGRET_341S0850);
	}
}

void mac_state::maclc2(machine_config &config, bool egret, int woz_version)
{
	maclc(config, false, egret, asc_device::asc_type::V8, woz_version);

	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maclc_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M,10M");
}

void mac_state::maccclas(machine_config &config)
{
	maclc(config, false, false, asc_device::asc_type::VASP, 2);

	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maclc_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M,10M");

	add_cuda(config, CUDA_341S0788); // should be 0417, but that version won't sync up properly with the '030 right now
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_cdadb));
}

void mac_state::maciivx(machine_config &config)
{
	maclc(config, false, true, asc_device::asc_type::VASP);

	M68030(config, m_maincpu, C32M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maclc3_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	MCFG_VIDEO_START_OVERRIDE(mac_state,macv8)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	m_screen->set_screen_update(FUNC(mac_state::screen_update_macrbvvram));

	add_nubus(config, false);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,12M,16M,20M,24M,28M,32M,36M,40M,44M,48M,52M,56M,60M,64M");

	m_egret->set_type(EGRET_341S0851);
}

void mac_state::maciivi(machine_config &config)
{
	maciivx(config);
	m_maincpu->set_clock(C15M);
}

void mac_state::maciix(machine_config &config, bool nubus_bank1, bool nubus_bank2)
{
	macii(config, false, asc_device::asc_type::ASC, true, nubus_bank1, nubus_bank2);

	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	SWIM1(config.replace(), m_fdc, C15M);
	m_fdc->phases_cb().set(FUNC(mac_state::phases_w));
	m_fdc->sel35_cb().set(FUNC(mac_state::sel35_w));
	m_fdc->devsel_cb().set(FUNC(mac_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_hd(config, m_floppy[1]);

	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");

	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,32M,64M,96M,128M");
}

void mac_state::maciicx(machine_config &config)    // IIcx is a IIx with only slots 9/a/b
{
	maciix(config, true, false);
}

void mac_state::macse30(machine_config &config)
{
	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macse30_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_size(MAC_H_TOTAL, MAC_V_TOTAL);
	m_screen->set_visarea(0, MAC_H_VIS-1, 0, MAC_V_VIS-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macse30));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	MCFG_VIDEO_START_OVERRIDE(mac_state,mac)

	add_base_devices(config, true, 1);
	add_asc(config, asc_device::asc_type::ASC);
	add_scsi(config);

	add_nubus_pds(config, "pds030", mac_pds030_cards);

	add_via1_adb(config, false);
	add_via2(config);

	MACADB(config, m_macadb, C15M);
	m_macadb->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_macadb->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->adb_irq_callback().set(FUNC(mac_state::adb_irq_w));

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,16M,32M,48M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
}

void mac_state::macclas2(machine_config &config)
{
	maclc(config, false, true, asc_device::asc_type::EAGLE);

	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maclc_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	MCFG_VIDEO_START_OVERRIDE(mac_state,macv8)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,maceagle)

	m_screen->set_size(MAC_H_TOTAL, MAC_V_TOTAL);
	m_screen->set_visarea(0, MAC_H_VIS-1, 0, MAC_V_VIS-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macrbv));

	m_asc->set_type(asc_device::asc_type::EAGLE);

	m_ram->set_default_size("10M");
	m_ram->set_extra_options("2M,4M,6M,8M,10M");

	m_egret->set_type(EGRET_341S0851);
}

void mac_state::maciici(machine_config &config)
{
	macii(config, false, asc_device::asc_type::ASC, true, false, true, 1);

	M68030(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maciici_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	MCFG_VIDEO_START_OVERRIDE(mac_state,macrbv)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_size(640, 870);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macrbv));
	m_screen->screen_vblank().set(FUNC(mac_state::mac_rbv_vbl));
	config.set_default_layout(layout_monitors);

	/* internal ram */
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,8M,16M,32M,48M,64M,128M");
}

void mac_state::maciisi(machine_config &config)
{
	macii(config, false, asc_device::asc_type::ASC, false, true, true, 1);

	M68030(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maciici_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	MCFG_VIDEO_START_OVERRIDE(mac_state,macrbv)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_size(640, 870);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macrbv));
	m_screen->screen_vblank().set(FUNC(mac_state::mac_rbv_vbl));
	config.set_default_layout(layout_monitors);

	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,8M,16M,32M,48M,64M,128M");

	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_egadb));

	add_egret(config, EGRET_344S0100);
}

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

INPUT_PORTS_START( maciici )
	PORT_START("MONTYPE")
	PORT_CONFNAME(0x0f, 0x06, "Connected monitor")
	PORT_CONFSETTING( 0x01, "15\" Portrait Display (640x870)")
	PORT_CONFSETTING( 0x02, "12\" RGB (512x384)")
	PORT_CONFSETTING( 0x06, "13\" RGB (640x480)")
INPUT_PORTS_END

/***************************************************************************

  Game driver(s)

  The Mac driver uses a convention of placing the BIOS in "bootrom"

***************************************************************************/

ROM_START( maclc )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("350eacf0.rom", 0x000000, 0x080000, CRC(71681726) SHA1(6bef5853ae736f3f06c2b4e79772f65910c3b7d4))

	ROM_REGION(0x1100, "egret", 0)
	ROM_LOAD( "341s0851.bin", 0x000000, 0x001100, CRC(ea9ea6e4) SHA1(8b0dae3ec66cdddbf71567365d2c462688aeb571) )
ROM_END

ROM_START( macii )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "default", "rev. B")
	ROMX_LOAD( "9779d2c4.rom", 0x000000, 0x040000, CRC(4df6d054) SHA1(db6b504744281369794e26ba71a6e385cf6227fa), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "original", "rev. A")
	ROMX_LOAD( "97851db6.rom", 0x000000, 0x040000, CRC(8c8b9d03) SHA1(5c264fe976f1e8495d364947c932a5e8309b4300), ROM_BIOS(1) )
ROM_END

ROM_START( maciihmu )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "default", "rev. B")
	ROMX_LOAD( "9779d2c4.rom", 0x000000, 0x040000, CRC(4df6d054) SHA1(db6b504744281369794e26ba71a6e385cf6227fa), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "original", "rev. A")
	ROMX_LOAD( "97851db6.rom", 0x000000, 0x040000, CRC(8c8b9d03) SHA1(5c264fe976f1e8495d364947c932a5e8309b4300), ROM_BIOS(1) )
ROM_END

ROM_START( mac2fdhd )   // same ROM for II FDHD, IIx, IIcx, and SE/30
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )
ROM_END

ROM_START( maciix )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )
ROM_END

ROM_START( maciicx )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )
ROM_END

ROM_START( macse30 )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )

	ROM_REGION32_BE(0x2000, "se30vrom", 0)
	ROM_LOAD( "se30vrom.uk6", 0x000000, 0x002000, CRC(b74c3463) SHA1(584201cc67d9452b2488f7aaaf91619ed8ce8f03) )
ROM_END

ROM_START( maciifx )
	ROM_REGION32_BE(0x80000, "bootrom", 0)
	ROM_LOAD( "4147dd77.rom", 0x000000, 0x080000, CRC(ef441bbd) SHA1(9fba3d4f672a630745d65788b1d1119afa2c6728) )
ROM_END

ROM_START( maciici )
	ROM_REGION32_BE(0x80000, "bootrom", 0)
	ROM_LOAD32_BYTE( "341-0736.um12", 0x000000, 0x020000, CRC(7a1906e6) SHA1(3e39c80b52f40798502fcbdfc97b315545c4c4d3) )
	ROM_LOAD32_BYTE( "341-0735.um11", 0x000001, 0x020000, CRC(a8942189) SHA1(be9f653cab04c304d7ee8d4ec312c23ff5d47efc) )
	ROM_LOAD32_BYTE( "342-0734.um10", 0x000002, 0x020000, CRC(07f56402) SHA1(e11ca97181faf26cd0d05bd639d65998805c7822) )
	ROM_LOAD32_BYTE( "342-0733.um9",  0x000003, 0x020000, CRC(20c28451) SHA1(fecf849c9ac9717c18c13184e24a471888028e46) )
ROM_END

ROM_START( maciisi )
	ROM_REGION32_BE(0x80000, "bootrom", 0)
	ROM_LOAD( "36b7fb6c.rom", 0x000000, 0x080000, CRC(f304d973) SHA1(f923de4125aae810796527ff6e25364cf1d54eec) )
ROM_END

ROM_START( maciivx )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "4957eb49.rom", 0x000000, 0x100000, CRC(61be06e5) SHA1(560ce203d65178657ad09d03f532f86fa512bb40) )
ROM_END

ROM_START( maciivi )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "4957eb49.rom", 0x000000, 0x100000, CRC(61be06e5) SHA1(560ce203d65178657ad09d03f532f86fa512bb40) )
ROM_END

ROM_START( macclas2 )
	ROM_REGION32_BE(0x100000, "bootrom", 0) // 3193670e
	//ROM_LOAD( "3193670e.rom", 0x000000, 0x080000, CRC(96d2e1fd) SHA1(50df69c1b6e805e12a405dc610bc2a1471b2eac2) )
	ROM_LOAD32_BYTE( "341-0867__ba16__=c=apple_91.romhh.27c010.u25", 0x000000, 0x020000, CRC(88230887) SHA1(8f45f6d7eb6a8ec9242a46db4773af1d154409c6) )
	ROM_LOAD32_BYTE( "341-0866__5be9__=c=apple_91.rommh.27c010.u24", 0x000001, 0x020000, CRC(eae68c36) SHA1(e6ce79647dfe7e66590a012836d0b6e985ff672b) )
	ROM_LOAD32_BYTE( "341-0865__821e__=c=apple_91.romml.27c010.u23", 0x000002, 0x020000, CRC(cb306c01) SHA1(4d6e409995fd9a4aa9afda0fd790a5b09b1c2aca) )
	ROM_LOAD32_BYTE( "341-0864__6fc6__=c=apple_91.romll.27c010.u22", 0x000003, 0x020000, CRC(21a51e72) SHA1(bb513c1a5b8a41c7534d66aeacaeea47f58dae92) )
ROM_END

ROM_START( maclc2 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD32_BYTE( "341-0476_ue2-hh.bin", 0x000000, 0x020000, CRC(0c3b0ce4) SHA1(e4e8c883d7f2e002a3f7b7aefaa3840991e57025) )
	ROM_LOAD32_BYTE( "341-0475_ud2-mh.bin", 0x000001, 0x020000, CRC(7b013595) SHA1(0b82d8fac570270db9774f6254017d28611ae756) )
	ROM_LOAD32_BYTE( "341-0474_uc2-ml.bin", 0x000002, 0x020000, CRC(2ff2f52b) SHA1(876850df61d0233c1dd3c00d48d8d6690186b164) )
	ROM_LOAD32_BYTE( "341-0473_ub2-ll.bin", 0x000003, 0x020000, CRC(8843c37c) SHA1(bb5104110507ca543d106f11c6061245fd90c1a7) )
ROM_END

ROM_START( maccclas )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "ecd99dc0.rom", 0x000000, 0x100000, CRC(c84c3aa5) SHA1(fd9e852e2d77fe17287ba678709b9334d4d74f1e) )
ROM_END


/*    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT    CLASS      INIT                COMPANY           FULLNAME */
COMP( 1987, macii,     0,        0,      macii,    macadb,  mac_state, init_macii,         "Apple Computer", "Macintosh II",  MACHINE_SUPPORTS_SAVE )
COMP( 1987, maciihmu,  macii,    0,      maciihmu, macadb,  mac_state, init_macii,         "Apple Computer", "Macintosh II (w/o 68851 MMU)", MACHINE_SUPPORTS_SAVE )
COMP( 1988, mac2fdhd,  0,        0,      maciihd,  macadb,  mac_state, init_maciifdhd,     "Apple Computer", "Macintosh II (FDHD)",  MACHINE_SUPPORTS_SAVE )
COMP( 1988, maciix,    mac2fdhd, 0,      maciix,   macadb,  mac_state, init_maciix,        "Apple Computer", "Macintosh IIx",  MACHINE_SUPPORTS_SAVE )
COMP( 1989, macse30,   mac2fdhd, 0,      macse30,  macadb,  mac_state, init_macse30,       "Apple Computer", "Macintosh SE/30",  MACHINE_SUPPORTS_SAVE )
COMP( 1989, maciicx,   mac2fdhd, 0,      maciicx,  macadb,  mac_state, init_maciicx,       "Apple Computer", "Macintosh IIcx",  MACHINE_SUPPORTS_SAVE )
COMP( 1989, maciici,   0,        0,      maciici,  maciici, mac_state, init_maciici,       "Apple Computer", "Macintosh IIci", MACHINE_SUPPORTS_SAVE )
COMP( 1990, maciifx,   0,        0,      maciifx,  macadb,  mac_state, init_maciifx,       "Apple Computer", "Macintosh IIfx",  MACHINE_NOT_WORKING )
COMP( 1990, maclc,     0,        0,      maclc,    maciici, mac_state, init_maclc,         "Apple Computer", "Macintosh LC", MACHINE_IMPERFECT_SOUND )
COMP( 1990, maciisi,   0,        0,      maciisi,  maciici, mac_state, init_maciisi,       "Apple Computer", "Macintosh IIsi", MACHINE_SUPPORTS_SAVE )
COMP( 1991, macclas2,  0,        0,      macclas2, macadb,  mac_state, init_macclassic2,   "Apple Computer", "Macintosh Classic II", MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND )
COMP( 1991, maclc2,    0,        0,      maclc2,   maciici, mac_state, init_maclc2,        "Apple Computer", "Macintosh LC II",  MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )
COMP( 1993, maccclas,  0,        0,      maccclas, macadb,  mac_state, init_maclrcclassic, "Apple Computer", "Macintosh Color Classic", MACHINE_NOT_WORKING )
COMP( 1993, maciivx,   0,        0,      maciivx,  maciici, mac_state, init_maciivx,       "Apple Computer", "Macintosh IIvx", MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND )
COMP( 1993, maciivi,   maciivx,  0,      maciivi,  maciici, mac_state, init_maciivi,       "Apple Computer", "Macintosh IIvi", MACHINE_SUPPORTS_SAVE|MACHINE_IMPERFECT_SOUND )
