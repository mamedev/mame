// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/****************************************************************************

    drivers/mac.c
    Macintosh family emulation

    Nate Woods, Raphael Nabet, R. Belmont

        0x000000 - 0x3fffff     RAM/ROM (switches based on overlay)
        0x400000 - 0x4fffff     ROM
        0x580000 - 0x5fffff     5380 NCR/Symbios SCSI peripherals chip (Mac Plus only)
        0x600000 - 0x6fffff     RAM
        0x800000 - 0x9fffff     Zilog 8530 SCC (Serial Control Chip) Read
        0xa00000 - 0xbfffff     Zilog 8530 SCC (Serial Control Chip) Write
        0xc00000 - 0xdfffff     IWM (Integrated Woz Machine; floppy)
        0xe80000 - 0xefffff     Rockwell 6522 VIA
        0xf00000 - 0xffffef     ??? (the ROM appears to be accessing here)
        0xfffff0 - 0xffffff     Auto Vector


    Interrupts:
        M68K:
            Level 1 from VIA
            Level 2 from SCC
            Level 4 : Interrupt switch (not implemented)

        VIA:
            CA1 from VBLANK
            CA2 from 1 Hz clock (RTC)
            CB1 from Keyboard Clock
            CB2 from Keyboard Data
            SR  from Keyboard Data Ready

        SCC:
            PB_EXT  from mouse Y circuitry
            PA_EXT  from mouse X circuitry

    NOTES:
        - pmac6100: with recent PPC fixes now gets into the 68000 emulator and executes part of the 680x0 startup code.
          'g 6802c73c' to get to the interesting part (wait past the boot chime).  PPC register r24 is the 68000 PC.
          when the PC hits GetCPUID, the move.l (a2), d0 at PC = 0x10000 will cause an MMU fault (jump to 0xFFF00300).  why?
          a2 = 0x5ffffffc (the CPU ID register).  MMU is unable to resolve this; defect in the MMU emulation probable.

****************************************************************************/


#include "emu.h"
#include "includes/mac.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/m6805/m6805.h"
#include "machine/applefdc.h"
#include "machine/swim.h"
#include "machine/sonydriv.h"
#include "formats/ap_dsk35.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/scsicd.h"
#include "sound/volt_reg.h"

// NuBus and 030/040 PDS cards
#include "bus/nubus/nubus_48gc.h"
#include "bus/nubus/nubus_cb264.h"
#include "bus/nubus/nubus_vikbw.h"
#include "bus/nubus/nubus_specpdq.h"
#include "bus/nubus/nubus_m2hires.h"
#include "bus/nubus/nubus_spec8.h"
#include "bus/nubus/nubus_radiustpd.h"
#include "bus/nubus/nubus_wsportrait.h"
#include "bus/nubus/nubus_asntmc3b.h"
#include "bus/nubus/nubus_image.h"
#include "bus/nubus/nubus_m2video.h"
#include "bus/nubus/bootbug.h"
#include "bus/nubus/quadralink.h"
#include "bus/nubus/pds30_cb264.h"
#include "bus/nubus/pds30_procolor816.h"
#include "bus/nubus/pds30_sigmalview.h"
#include "bus/nubus/pds30_30hr.h"
#include "bus/nubus/pds30_mc30.h"

// 68000 PDS cards
#include "bus/macpds/pds_tpdfpd.h"

#include "machine/macadb.h"
#include "softlist.h"
#include "speaker.h"
#include "mac.lh"

#define C7M (7833600)
#define C15M    (C7M*2)
#define C32M    (C15M*2)

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

READ32_MEMBER( mac_state::rbv_ramdac_r )
{
	return 0;
}

WRITE32_MEMBER( mac_state::rbv_ramdac_w )
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

WRITE32_MEMBER( mac_state::ariel_ramdac_w ) // this is for the "Ariel" style RAMDAC
{
	if (mem_mask == 0xff000000)
	{
		m_rbv_clutoffs = data>>24;
		m_rbv_count = 0;
	}
	else if (mem_mask == 0x00ff0000)
	{
		m_rbv_colors[m_rbv_count++] = data>>16;

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
	else if (mem_mask == 0x0000ff00)
	{
		// config reg
//      printf("Ariel: %02x to config\n", (data>>8)&0xff);
	}
	else    // color key reg
	{
	}
}

READ8_MEMBER( mac_state::mac_sonora_vctl_r )
{
	if (offset == 2)
	{
//        printf("Sonora: read monitor ID at PC=%x\n", m_maincpu->pc());
		return (m_montype.read_safe(6)<<4);
	}

	return m_sonora_vctl[offset];
}

WRITE8_MEMBER( mac_state::mac_sonora_vctl_w )
{
//  printf("Sonora: %02x to vctl %x\n", data, offset);
	m_sonora_vctl[offset] = data;
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

READ8_MEMBER ( mac_state::mac_rbv_r )
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

WRITE8_MEMBER ( mac_state::mac_rbv_w )
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

// Portable/PB100 video
VIDEO_START_MEMBER(mac_state,macprtb)
{
}

READ16_MEMBER(mac_state::mac_config_r)
{
	return 0xffff;  // returns nonzero if no PDS RAM expansion, 0 if present
}

// IIfx
READ32_MEMBER(mac_state::biu_r)
{
//  printf("biu_r @ %x, mask %08x\n", offset, mem_mask);
	return 0;
}

WRITE32_MEMBER(mac_state::biu_w)
{
//  printf("biu_w %x @ %x, mask %08x\n", data, offset, mem_mask);
}

READ8_MEMBER(mac_state::oss_r)
{
//  printf("oss_r @ %x\n", offset);
//  if (offset <= 0xe)  // for interrupt mask registers, we're intended to return something different than is written in the low 3 bits (?)
//  {
//      return m_oss_regs[offset]<<4;
//  }

	return m_oss_regs[offset];
}

WRITE8_MEMBER(mac_state::oss_w)
{
//  printf("oss_w %x @ %x\n", data, offset);
	m_oss_regs[offset] = data;
}

READ32_MEMBER(mac_state::buserror_r)
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
}

READ8_MEMBER(mac_state::scciop_r)
{
//  printf("scciop_r @ %x (PC=%x)\n", offset, m_maincpu->pc());
	return 0;
}

WRITE8_MEMBER(mac_state::scciop_w)
{
//  printf("scciop_w %x @ %x (PC=%x)\n", data, offset, m_maincpu->pc());
}

READ8_MEMBER(mac_state::swimiop_r)
{
//  printf("swimiop_r @ %x (PC=%x)\n", offset, m_maincpu->pc());
	return 0;
}

WRITE8_MEMBER(mac_state::swimiop_w)
{
//  printf("swimiop_w %x @ %x (PC=%x)\n", data, offset, m_maincpu->pc());
}

READ8_MEMBER(mac_state::pmac_diag_r)
{
	switch (offset)
	{
		case 0: // return 0 here to get the 'car crash' sound after the boot bong, 1 otherwise
			return 1;
	}

	return 0;
}

READ8_MEMBER(mac_state::amic_dma_r)
{
	return 0;
}

WRITE8_MEMBER(mac_state::amic_dma_w)
{
//  printf("amic_dma_w: %02x at %x (PC=%x)\n", data, offset+0x1000, m_maincpu->pc());
}

// HMC has one register: a 35-bit shift register which is accessed one bit at a time (see pmac6100 code at 4030383c which makes this obvious)
READ8_MEMBER(mac_state::hmc_r)
{
	uint8_t rv = (uint8_t)(m_hmc_shiftout&1);
	m_hmc_shiftout>>= 1;
	return rv;
}

WRITE8_MEMBER(mac_state::hmc_w)
{
	// writes to xxx8 reset the bit shift position
	if ((offset&0x8) == 8)
	{
		m_hmc_shiftout = m_hmc_reg;
	}
	else
	{
		uint64_t temp = (data & 1) ? 0x400000000U : 0x0U;
		m_hmc_reg >>= 1;
		m_hmc_reg |= temp;
	}
}

READ8_MEMBER(mac_state::mac_gsc_r)
{
	if (offset == 1)
	{
		return 5;
	}

	return 0;
}

WRITE8_MEMBER(mac_state::mac_gsc_w)
{
}

READ8_MEMBER(mac_state::mac_5396_r)
{
	if (offset < 0x100)
	{
		return m_539x_1->read(space, offset>>4);
	}
	else    // pseudo-DMA: read from the FIFO
	{
		return m_539x_1->read(space, 2);
	}

	// never executed
	//return 0;
}

WRITE8_MEMBER(mac_state::mac_5396_w)
{
	if (offset < 0x100)
	{
		m_539x_1->write(space, offset>>4, data);
	}
	else    // pseudo-DMA: write to the FIFO
	{
		m_539x_1->write(space, 2, data);
	}
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mac_state::mac512ke_map(address_map &map)
{
	map(0x800000, 0x9fffff).r(FUNC(mac_state::mac_scc_r));
	map(0xa00000, 0xbfffff).w(FUNC(mac_state::mac_scc_w));
	map(0xc00000, 0xdfffff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0xe80000, 0xefffff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0xfffff0, 0xffffff).rw(FUNC(mac_state::mac_autovector_r), FUNC(mac_state::mac_autovector_w));
}

void mac_state::macplus_map(address_map &map)
{
	map(0x580000, 0x5fffff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macplus_scsi_w));
	map(0x800000, 0x9fffff).r(FUNC(mac_state::mac_scc_r));
	map(0xa00000, 0xbfffff).w(FUNC(mac_state::mac_scc_w));
	map(0xc00000, 0xdfffff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0xe80000, 0xefffff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0xfffff0, 0xffffff).rw(FUNC(mac_state::mac_autovector_r), FUNC(mac_state::mac_autovector_w));
}

void mac_state::macse_map(address_map &map)
{
	map(0x580000, 0x5fffff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macplus_scsi_w));
	map(0x900000, 0x9fffff).r(FUNC(mac_state::mac_scc_r));
	map(0xb00000, 0xbfffff).w(FUNC(mac_state::mac_scc_w));
	map(0xd00000, 0xdfffff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0xe80000, 0xefffff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0xfffff0, 0xffffff).rw(FUNC(mac_state::mac_autovector_r), FUNC(mac_state::mac_autovector_w));
}

void mac_state::macprtb_map(address_map &map)
{
	map(0x900000, 0x93ffff).rom().region("bootrom", 0).mirror(0x0c0000);
	map(0xf60000, 0xf6ffff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0xf70000, 0xf7ffff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0xf90000, 0xf9ffff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macplus_scsi_w));
	map(0xfa8000, 0xfaffff).ram().share("vram16");  // VRAM
	map(0xfb0000, 0xfbffff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0xfc0000, 0xfcffff).r(FUNC(mac_state::mac_config_r));
	map(0xfd0000, 0xfdffff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w));
	map(0xfffff0, 0xffffff).rw(FUNC(mac_state::mac_autovector_r), FUNC(mac_state::mac_autovector_w));
}

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
	map(0x50028000, 0x50028003).rw(FUNC(mac_state::mac_sonora_vctl_r), FUNC(mac_state::mac_sonora_vctl_w)).mirror(0x00f00000);

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
	map(0x50012060, 0x50012063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00f00000);
	map(0x50040000, 0x50041fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
}

void mac_state::maciici_map(address_map &map)
{
	map(0x40000000, 0x4007ffff).rom().region("bootrom", 0).mirror(0x0ff80000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x00f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x50012060, 0x50012063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
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
	map(0x50012060, 0x50012063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
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
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::scciop_r), FUNC(mac_state::scciop_w)).mirror(0x00f00000);
	map(0x5000a000, 0x5000bfff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x5000c060, 0x5000c063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
	map(0x5000d000, 0x5000d003).w(FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x5000d060, 0x5000d063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(mac_state::swimiop_r), FUNC(mac_state::swimiop_w)).mirror(0x00f00000);
	map(0x50018000, 0x50019fff).rw(FUNC(mac_state::biu_r), FUNC(mac_state::biu_w)).mirror(0x00f00000);
	map(0x5001a000, 0x5001bfff).rw(FUNC(mac_state::oss_r), FUNC(mac_state::oss_w)).mirror(0x00f00000);
	map(0x50024000, 0x50027fff).r(FUNC(mac_state::buserror_r)).mirror(0x00f00000);   // must bus error on access here so ROM can determine we're an FMC
	map(0x50040000, 0x50041fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
}

// ROM detects the "Jaws" ASIC by checking for I/O space mirrored at 0x01000000 boundries
void mac_state::macpb140_map(address_map &map)
{
	map(0x40000000, 0x400fffff).rom().region("bootrom", 0).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x01f00000);
	map(0x50002000, 0x50003fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w)).mirror(0x01f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x01f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x01f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x01f00000);
	map(0x50012060, 0x50012063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x01f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x01f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x01f00000);
	map(0x50024000, 0x50027fff).r(FUNC(mac_state::buserror_r)).mirror(0x01f00000);   // bus error here to make sure we aren't mistaken for another decoder

	map(0xfee08000, 0xfeffffff).ram().share("vram");
}

void mac_state::macpb160_map(address_map &map)
{
	map(0x40000000, 0x400fffff).rom().region("bootrom", 0).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(mac_state::macii_scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0x50f20000, 0x50f21fff).rw(FUNC(mac_state::mac_gsc_r), FUNC(mac_state::mac_gsc_w));
	map(0x50f24000, 0x50f27fff).r(FUNC(mac_state::buserror_r));   // bus error here to make sure we aren't mistaken for another decoder

	map(0x60000000, 0x6001ffff).ram().share("vram").mirror(0x0ffe0000);
}

void mac_state::macpb165c_map(address_map &map)
{
	map(0x40000000, 0x400fffff).rom().region("bootrom", 0).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(mac_state::macii_scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0x50f20000, 0x50f21fff).r(FUNC(mac_state::buserror_r));    // bus error here to detect we're not the grayscale 160/165/180
	map(0x50f24000, 0x50f27fff).r(FUNC(mac_state::buserror_r));   // bus error here to make sure we aren't mistaken for another decoder

	// on-board color video on 165c/180c
	map(0xfc000000, 0xfc07ffff).ram().share("vram").mirror(0x00380000); // 512k of VRAM
	map(0xfc400000, 0xfcefffff).rw(FUNC(mac_state::macwd_r), FUNC(mac_state::macwd_w));
// fc4003c8 = DAC control, fc4003c9 = DAC data
// fc4003da bit 3 is VBL
	map(0xfcff8000, 0xfcffffff).rom().region("vrom", 0x0000);
}

void mac_state::macpd210_map(address_map &map)
{
	map(0x40000000, 0x400fffff).rom().region("bootrom", 0).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(mac_state::macii_scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w));
	map(0x50f20000, 0x50f21fff).rw(FUNC(mac_state::mac_gsc_r), FUNC(mac_state::mac_gsc_w));
	map(0x50f24000, 0x50f27fff).r(FUNC(mac_state::buserror_r));   // bus error here to make sure we aren't mistaken for another decoder

	map(0x5ffffffc, 0x5fffffff).r(FUNC(mac_state::mac_read_id));

	map(0x60000000, 0x6001ffff).ram().share("vram").mirror(0x0ffe0000);
}

void mac_state::quadra700_map(address_map &map)
{
	map(0x40000000, 0x400fffff).rom().region("bootrom", 0).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00fc0000);
	map(0x50002000, 0x50003fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w)).mirror(0x00fc0000);
// 50008000 = Ethernet MAC ID PROM
// 5000a000 = Sonic (DP83932) ethernet
// 5000f000 = SCSI cf96, 5000f402 = SCSI #2 cf96
	map(0x5000f000, 0x5000f3ff).rw(FUNC(mac_state::mac_5396_r), FUNC(mac_state::mac_5396_w)).mirror(0x00fc0000);
	map(0x5000c000, 0x5000dfff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x00fc0000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00fc0000);
	map(0x5001e000, 0x5001ffff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00fc0000);

	// f9800000 = VDAC / DAFB
	map(0xf9000000, 0xf91fffff).ram().share("vram");
	map(0xf9800000, 0xf98001ff).rw(FUNC(mac_state::dafb_r), FUNC(mac_state::dafb_w));
	map(0xf9800200, 0xf980023f).rw(FUNC(mac_state::dafb_dac_r), FUNC(mac_state::dafb_dac_w));
}

void mac_state::pwrmac_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram(); // 8 MB standard

	map(0x40000000, 0x403fffff).rom().region("bootrom", 0).mirror(0x0fc00000);

	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_2_w)).mirror(0x00f00000);
	// 50008000 = ethernet ID PROM
	// 5000a000 = MACE ethernet controller
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	// 50014000 = sound registers (AWACS)
	map(0x50014000, 0x50015fff).rw(m_awacs, FUNC(awacs_device::read), FUNC(awacs_device::write)).mirror(0x01f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00f00000);
	map(0x50024000, 0x50025fff).w(FUNC(mac_state::ariel_ramdac_w)).mirror(0x00f00000);
	map(0x50026000, 0x50027fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w)).mirror(0x00f00000);
	map(0x50028000, 0x50028007).rw(FUNC(mac_state::mac_sonora_vctl_r), FUNC(mac_state::mac_sonora_vctl_w)).mirror(0x00f00000);
	// 5002a000 = interrupt controller
	// 5002c000 = diagnostic registers
	map(0x5002c000, 0x5002dfff).r(FUNC(mac_state::pmac_diag_r)).mirror(0x00f00000);
	map(0x50031000, 0x50032fff).rw(FUNC(mac_state::amic_dma_r), FUNC(mac_state::amic_dma_w)).mirror(0x00f00000);
	map(0x50040000, 0x5004000f).rw(FUNC(mac_state::hmc_r), FUNC(mac_state::hmc_w)).mirror(0x00f00000);
	map(0x5ffffff8, 0x5fffffff).r(FUNC(mac_state::mac_read_id));

	map(0xffc00000, 0xffffffff).rom().region("bootrom", 0);
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static const applefdc_interface mac_iwm_interface =
{
	sony_set_lines,
	mac_fdc_set_enable_lines,

	sony_read_data,
	sony_write_data,
	sony_read_status
};

static void mac_nubus_cards(device_slot_interface &device)
{
	device.option_add("m2video", NUBUS_M2VIDEO);    /* Apple Macintosh II Video Card */
	device.option_add("48gc", NUBUS_48GC);      /* Apple 4*8 Graphics Card */
	device.option_add("824gc", NUBUS_824GC);    /* Apple 8*24 Graphics Card */
	device.option_add("cb264", NUBUS_CB264);    /* RasterOps ColorBoard 264 */
	device.option_add("vikbw", NUBUS_VIKBW);    /* Moniterm Viking board */
	device.option_add("image", NUBUS_IMAGE);    /* Disk Image Pseudo-Card */
	device.option_add("specpdq", NUBUS_SPECPDQ);    /* SuperMac Spectrum PDQ */
	device.option_add("m2hires", NUBUS_M2HIRES);    /* Apple Macintosh II Hi-Resolution Card */
	device.option_add("spec8s3", NUBUS_SPEC8S3);    /* SuperMac Spectrum/8 Series III */
//  device.option_add("thundergx", NUBUS_THUNDERGX);        /* Radius Thunder GX (not yet) */
	device.option_add("radiustpd", NUBUS_RADIUSTPD);        /* Radius Two Page Display */
	device.option_add("asmc3nb", NUBUS_ASNTMC3NB);  /* Asante MC3NB Ethernet card */
	device.option_add("portrait", NUBUS_WSPORTRAIT);    /* Apple Macintosh II Portrait video card */
	device.option_add("enetnb", NUBUS_APPLEENET);   /* Apple NuBus Ethernet */
	device.option_add("bootbug", NUBUS_BOOTBUG);    /* Brigent BootBug debugger card */
	device.option_add("quadralink", NUBUS_QUADRALINK);  /* AE Quadralink serial card */
}

static void mac_pds030_cards(device_slot_interface &device)
{
	device.option_add("cb264", PDS030_CB264SE30);   // RasterOps Colorboard 264/SE30
	device.option_add("pc816", PDS030_PROCOLOR816); // Lapis ProColor Server 8*16 PDS
	device.option_add("lview", PDS030_LVIEW);       // Sigma Designs L-View
	device.option_add("30hr",  PDS030_XCEED30HR);   // Micron/XCEED Technology Color 30HR
	device.option_add("mc30",  PDS030_XCEEDMC30);   // Micron/XCEED Technology MacroColor 30
}

static void mac_sepds_cards(device_slot_interface &device)
{
	device.option_add("radiusfpd", PDS_SEDISPLAY);  // Radius Full Page Display card for SE
}

static void mac_lcpds_cards(device_slot_interface &device)
{
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static const floppy_interface mac_floppy_interface =
{
	FLOPPY_STANDARD_3_5_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_mac),
	"floppy_3_5"
};

void mac_state::add_base_devices(machine_config &config, bool rtc, bool super_woz)
{
	if (rtc)
		RTC3430042(config, m_rtc, XTAL(32'768));

	if (super_woz)
		SWIM(config, m_fdc, &mac_iwm_interface);
	else
		IWM(config, m_fdc, &mac_iwm_interface);
	sonydriv_floppy_image_device::legacy_2_drives_add(config, &mac_floppy_interface);

	SCC8530(config, m_scc, C7M);
	m_scc->intrq_callback().set(FUNC(mac_state::set_scc_interrupt));
}

void mac_state::add_mackbd(machine_config &config)
{
#ifdef MAC_USE_EMULATED_KBD
	MACKBD(config, m_mackbd, 0);
	m_mackbd->dataout_handler().set(m_via, FUNC(via6522_device::write_cb2));
	m_mackbd->clkout_handler().set(FUNC(mac_state::mac_kbd_clk_in));
#else
	MACKBD(config, m_mackbd, 0);
#endif
}

void mac_state::add_scsi(machine_config &config, bool cdrom)
{
	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));
	scsibus.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_5));
	if (cdrom)
		scsibus.set_slot_device(3, "cdrom", SCSICD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));

	NCR5380(config, m_ncr5380, C7M);
	m_ncr5380->set_scsi_port("scsi");
	m_ncr5380->irq_callback().set(FUNC(mac_state::mac_scsi_irq));

	SOFTWARE_LIST(config, "hdd_list").set_type("mac_hdd", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

void mac_state::add_via1_adb(machine_config &config, bool macii)
{
	VIA6522(config, m_via1, C7M/10);
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
	VIA6522(config, m_via2, C7M/10);
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
	m_egret->linechange_callback().set(FUNC(mac_state::adb_linechange_w));
	m_egret->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_egret->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	config.m_perfect_cpu_quantum = subtag("maincpu");
}

void mac_state::add_cuda(machine_config &config, int type)
{
	CUDA(config, m_cuda, type);
	m_cuda->reset_callback().set(FUNC(mac_state::cuda_reset_w));
	m_cuda->linechange_callback().set(FUNC(mac_state::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_cuda->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
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

void mac_state::add_pb1xx_screen(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(700, 480);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_palette(m_palette);
}

void mac_state::add_pb1xx_vias(machine_config &config)
{
	VIA6522(config, m_via1, 783360);
	m_via1->readpa_handler().set(FUNC(mac_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b_via2pmu));
	m_via1->writepa_handler().set(FUNC(mac_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_via2pmu));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_adb_via_out_cb2));
	m_via1->irq_handler().set(FUNC(mac_state::mac_via_irq));

	VIA6522(config, m_via2, 783360);
	m_via2->readpa_handler().set(FUNC(mac_state::mac_via2_in_a_pmu));
	m_via2->readpb_handler().set(FUNC(mac_state::mac_via2_in_b_pmu));
	m_via2->writepa_handler().set(FUNC(mac_state::mac_via2_out_a_pmu));
	m_via2->writepb_handler().set(FUNC(mac_state::mac_via2_out_b_pmu));
	m_via2->irq_handler().set(FUNC(mac_state::mac_via2_irq));
}

void mac_state::mac512ke_base(machine_config &config)
{
	M68000(config, m_maincpu, C7M);       /* 7.8336 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::mac512ke_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));
	config.m_minimum_quantum = attotime::from_hz(60);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(C7M*2, MAC_H_TOTAL, 0, MAC_H_VIS, MAC_V_TOTAL, 0, MAC_V_VIS);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_mac));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	MCFG_VIDEO_START_OVERRIDE(mac_state,mac)

	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_PWM(config, m_dac, 0);
	m_dac->add_route(ALL_OUTPUTS, "speaker", 0.25); // 2 x ls161
	voltage_regulator_device &vreg(VOLTAGE_REGULATOR(config, "vref", 0));
	vreg.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	add_base_devices(config, true, false);

	VIA6522(config, m_via1, 1000000);
	m_via1->readpa_handler().set(FUNC(mac_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(mac_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_via_out_cb2));
	m_via1->irq_handler().set(FUNC(mac_state::mac_via_irq));

	RAM(config, m_ram);
	m_ram->set_default_size("512K");
}

void mac_state::mac512ke(machine_config &config)
{
	mac512ke_base(config);
	add_mackbd(config);
}

void mac_state::add_macplus_additions(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macplus_map);

	add_scsi(config);

	/* internal ram */
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("1M,2M,2560K,4M");

	// software list
	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

void mac_state::add_nubus(machine_config &config, bool bank1, bool bank2)
{
	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_cputag("maincpu");
	nubus.out_irq9_callback().set(FUNC(mac_state::nubus_irq_9_w));
	nubus.out_irqa_callback().set(FUNC(mac_state::nubus_irq_a_w));
	nubus.out_irqb_callback().set(FUNC(mac_state::nubus_irq_b_w));
	nubus.out_irqc_callback().set(FUNC(mac_state::nubus_irq_c_w));
	nubus.out_irqd_callback().set(FUNC(mac_state::nubus_irq_d_w));
	nubus.out_irqe_callback().set(FUNC(mac_state::nubus_irq_e_w));
	if (bank1)
	{
		NUBUS_SLOT(config, "nb9", "nubus", mac_nubus_cards, "48gc");
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
	nubus.set_cputag("maincpu");
	nubus.out_irq9_callback().set(FUNC(mac_state::nubus_irq_9_w));
	nubus.out_irqa_callback().set(FUNC(mac_state::nubus_irq_a_w));
	nubus.out_irqb_callback().set(FUNC(mac_state::nubus_irq_b_w));
	nubus.out_irqc_callback().set(FUNC(mac_state::nubus_irq_c_w));
	nubus.out_irqd_callback().set(FUNC(mac_state::nubus_irq_d_w));
	nubus.out_irqe_callback().set(FUNC(mac_state::nubus_irq_e_w));
	NUBUS_SLOT(config, slot_tag, "pds", std::forward<T>(opts), nullptr);
}

void mac_state::macplus(machine_config &config)
{
	mac512ke_base(config);
	add_macplus_additions(config);
	add_mackbd(config);
}

void mac_state::macse(machine_config &config)
{
	mac512ke_base(config);
	add_macplus_additions(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macse_map);

	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_bbadb));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_adb_via_out_cb2));

	/* internal ram */
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("2M,2560K,4M");

	MACPDS(config, "sepds", "maincpu");
	MACPDS_SLOT(config, "pds", "sepds", mac_sepds_cards, nullptr);
}

void mac_state::macclasc(machine_config &config)
{
	mac512ke_base(config);
	add_macplus_additions(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macse_map);

	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_bbadb));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_adb_via_out_cb2));

	/* internal ram */
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("2M,2560K,4M");
}

void mac_state::macprtb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macprtb_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	config.m_minimum_quantum = attotime::from_hz(60);

	add_pb1xx_screen(config);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macprtb));

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	MCFG_VIDEO_START_OVERRIDE(mac_state,macprtb)

	/* devices */
	add_base_devices(config);
	add_scsi(config);
	add_asc(config, asc_device::asc_type::ASC);

	VIA6522(config, m_via1, 783360);
	m_via1->readpa_handler().set(FUNC(mac_state::mac_via_in_a_pmu));
	m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b_pmu));
	m_via1->writepa_handler().set(FUNC(mac_state::mac_via_out_a_pmu));
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_pmu));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_via_out_cb2));
	m_via1->irq_handler().set(FUNC(mac_state::mac_via_irq));

	RAM(config, m_ram);
	m_ram->set_default_size("1M");
	m_ram->set_extra_options("1M,3M,5M,7M,9M");
}

void mac_state::macii(machine_config &config, bool cpu, asc_device::asc_type asc_type, bool nubus, bool nubus_bank1, bool nubus_bank2)
{
	if (cpu)
	{
		M68020PMMU(config, m_maincpu, C15M);
		m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
		m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));
	}

	PALETTE(config, m_palette).set_entries(256);

	add_asc(config, asc_type);
	add_base_devices(config);
	add_scsi(config, true);
	if (nubus)
		add_nubus(config, nubus_bank1, nubus_bank2);

	add_via1_adb(config, true);
	add_via2(config);

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

void mac_state::maciihmu(machine_config &config)
{
	macii(config, false);

	M68020HMMU(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));
}

void mac_state::maciifx(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 40000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maciifx_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	add_asc(config, asc_device::asc_type::ASC);
	add_base_devices(config);
	add_scsi(config);

	VIA6522(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(mac_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b_ii));
	m_via1->writepa_handler().set(FUNC(mac_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b));
	m_via1->cb2_handler().set(FUNC(mac_state::mac_adb_via_out_cb2));
	m_via1->irq_handler().set(FUNC(mac_state::mac_via_irq));

	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	add_nubus(config);
}

void mac_state::maclc(machine_config &config, bool cpu, bool egret, asc_device::asc_type asc_type)
{
	macii(config, false, asc_type, false);

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
	config.set_default_layout(layout_mac);

	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M,10M");

	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_egadb));

	add_nubus_pds(config, "lcpds", mac_lcpds_cards);

	if (egret)
		add_egret(config, EGRET_341S0850);
}

void mac_state::maclc2(machine_config &config, bool egret)
{
	maclc(config, false, egret);

	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maclc_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M,10M");
}

void mac_state::maccclas(machine_config &config)
{
	maclc2(config, false);
	add_cuda(config, CUDA_341S0788); // should be 0417, but that version won't sync up properly with the '030 right now
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_cdadb));
}

void mac_state::maclc3(machine_config &config, bool egret)
{
	maclc(config, false, false, asc_device::asc_type::SONORA);

	M68030(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::maclc3_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	MCFG_VIDEO_START_OVERRIDE(mac_state,macsonora)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macsonora)

	m_screen->set_screen_update(FUNC(mac_state::screen_update_macsonora));

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,48M,64M,80M");

	if (egret)
		add_egret(config, EGRET_341S0851);
}

void mac_state::maclc520(machine_config &config)
{
	maclc3(config, false);
	add_cuda(config, CUDA_341S0060);
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_cdadb));
}

void mac_state::maciivx(machine_config &config)
{
	maclc(config, false);

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

	add_base_devices(config, true, true);
	add_asc(config, asc_device::asc_type::ASC);
	add_scsi(config);

	add_nubus_pds(config, "pds030", mac_pds030_cards);

	add_via1_adb(config, false);
	add_via2(config);

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,16M,32M,48M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

void mac_state::macpb140(machine_config &config)
{
	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macpb140_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	add_pb1xx_screen(config);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macpb140));

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	MCFG_VIDEO_START_OVERRIDE(mac_state,macprtb)

	add_asc(config, asc_device::asc_type::ASC);
	add_scsi(config);
	add_base_devices(config, false, true);
	add_pb1xx_vias(config);

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M");

	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

// PowerBook 145 = 140 @ 25 MHz (still 2MB RAM - the 145B upped that to 4MB)
void mac_state::macpb145(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25000000);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

// PowerBook 170 = 140 @ 25 MHz with an active-matrix LCD (140/145/145B were passive)
void mac_state::macpb170(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25000000);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

void mac_state::macpb160(machine_config &config)
{
	M68030(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macpb160_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	add_pb1xx_screen(config);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macpb160));

	PALETTE(config, m_palette, FUNC(mac_state::macgsc_palette), 16);

	MCFG_VIDEO_START_OVERRIDE(mac_state,macprtb)

	add_asc(config, asc_device::asc_type::ASC);
	add_scsi(config);
	add_base_devices(config, false, true);
	add_pb1xx_vias(config);

	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,12M,16M");

	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

void mac_state::macpb180(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33000000);
}

void mac_state::macpb180c(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macpb165c_map);

	m_screen->set_size(800, 525);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macpbwd));
	m_screen->set_palette(finder_base::DUMMY_TAG);
}

void mac_state::macpd210(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macpd210_map);

	m_ram->set_extra_options("8M,12M,16M,20M,24M");
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
	macii(config, false, asc_device::asc_type::ASC, true, false, true);

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
	config.set_default_layout(layout_mac);

	/* internal ram */
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,8M,16M,32M,48M,64M,128M");
}

void mac_state::maciisi(machine_config &config)
{
	macii(config, false, asc_device::asc_type::ASC, false);

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
	config.set_default_layout(layout_mac);

	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,8M,16M,32M,48M,64M,128M");

	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_egadb));

	add_egret(config, EGRET_344S0100);
}

void mac_state::pwrmac(machine_config &config)
{
	PPC601(config, m_maincpu, 60000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::pwrmac_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// dot clock, htotal, hstart, hend, vtotal, vstart, vend
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(1024, 768);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macrbv));

	PALETTE(config, m_palette).set_entries(256);

	MCFG_VIDEO_START_OVERRIDE(mac_state,macsonora)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AWACS(config, m_awacs, 44100);
	m_awacs->add_route(0, "lspeaker", 1.0);
	m_awacs->add_route(1, "rspeaker", 1.0);

	add_scsi(config);
	add_base_devices(config, false, false);

	add_via1_adb(config, false);
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b_cdadb));

	add_via2(config);

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M,32M,64M,128M");

	add_cuda(config, CUDA_341S0060);
}

void mac_state::macqd700(machine_config &config)
{
	/* basic machine hardware */
	M68040(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::quadra700_map);
	m_maincpu->set_dasm_override(FUNC(mac_state::mac_dasm_override));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(75.08);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_size(1152, 870);
	m_screen->set_visarea(0, 1152-1, 0, 870-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macdafb));

	MCFG_VIDEO_START_OVERRIDE(mac_state,macdafb)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macdafb)

	PALETTE(config, m_palette).set_entries(256);

	add_asc(config, asc_device::asc_type::EASC);
	add_base_devices(config, true, false);

	add_nubus(config, false, false);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

	add_via1_adb(config, false);
	m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b));

	add_via2(config);

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));
	scsibus.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_5));

	NCR539X(config, m_539x_1, C7M);
	m_539x_1->set_scsi_port("scsi");
	m_539x_1->irq_callback().set(FUNC(mac_state::irq_539x_1_w));
	m_539x_1->drq_callback().set(FUNC(mac_state::drq_539x_1_w));

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,64M,68M,72M,80M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_type("mac_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);
}

static INPUT_PORTS_START( macadb )
	PORT_START("MOUSE0") /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START("MOUSE1") /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE2") /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

		/* This handles the standard (not Extended) Apple ADB keyboard, which is similar to the IIgs's */
	/* main keyboard */

	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)    /* extra key on ISO : */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("KEY3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)    /* keyboard Enter : */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")     PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Command / Open Apple") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Option / Solid Apple") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Arrow") PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Arrow") PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down Arrow") PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up Arrow") PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)    /* ??? */

	/* keypad */
	PORT_START("KEY4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x40
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   // 0x41
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x42
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)          PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  // 0x43
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x44
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) // 0x45
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x46
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Clear") PORT_CODE(/*KEYCODE_NUMLOCK*/KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))    // 0x47
	PORT_BIT(0x0700, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x48, 49, 4a
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)         PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // 0x4b
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) // 0x4c
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x4d
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // 0x4e
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x4f

	PORT_START("KEY5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x50
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(/*CODE_OTHER*/KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD)) // 0x51
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) // 0x52
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) // 0x53
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) // 0x54
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) // 0x55
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) // 0x56
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) // 0x57
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) // 0x58
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) // 0x59
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x5a
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) // 0x5b
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) // 0x5c
	PORT_BIT(0xE000, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

INPUT_PORTS_START( maciici )
	PORT_INCLUDE(macadb)

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

ROM_START( macse )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD16_WORD( "macse.rom",  0x00000, 0x40000, CRC(0f7ff80c) SHA1(58532b7d0d49659fd5228ac334a1b094f0241968))
ROM_END

ROM_START( macsefd )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "be06e171.rom", 0x000000, 0x040000, CRC(f530cb10) SHA1(d3670a90273d12e53d86d1228c068cb660b8c9d1) )
ROM_END

ROM_START( macclasc )
	ROM_REGION16_BE(0x80000, "bootrom", 0) // a49f9914, second half of chip dump is the 6.0.3 XO rom disk
	// this dump is big endian
	ROM_LOAD( "341-0813__=c=1983-90_apple__japan__910d_d.27c4096_be.ue1", 0x000000, 0x080000, CRC(510d7d38) SHA1(ccd10904ddc0fb6a1d216b2e9effd5ec6cf5a83d) )
ROM_END

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

ROM_START( maclc3 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
		ROM_LOAD( "ecbbc41c.rom", 0x000000, 0x100000, CRC(e578f5f3) SHA1(c77df3220c861f37a2c553b6ee9241b202dfdffc) )
ROM_END

ROM_START( pmac6100 )
	ROM_REGION64_BE(0x400000, "bootrom", 0)
	ROM_LOAD( "9feb69b3.rom", 0x000000, 0x400000, CRC(a43fadbc) SHA1(6fac1c4e920a077c077b03902fef9199d5e8f2c3) )
ROM_END

ROM_START( macprtb )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD16_WORD( "93ca3846.rom", 0x000000, 0x040000, CRC(497348f8) SHA1(79b468b33fc53f11e87e2e4b195aac981bf0c0a6) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv1.bin", 0x000000, 0x001800, CRC(01dae148) SHA1(29d2fca7426c31f2b9334832ed3d257974a61bb1) )
ROM_END

ROM_START( macpb100 )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD16_WORD( "96645f9c.rom", 0x000000, 0x040000, CRC(29ac7ee9) SHA1(7f3acf40b1f63612de2314a2e9fcfeafca0711fc) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv1.bin", 0x000000, 0x001800, CRC(01dae148) SHA1(29d2fca7426c31f2b9334832ed3d257974a61bb1) )
ROM_END

ROM_START( macpb140 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv2.bin",    0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92) )
ROM_END

ROM_START( macpb145 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv2.bin",    0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92) )
ROM_END

ROM_START( macpb145b )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv2.bin",    0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92) )
ROM_END

ROM_START( macpb170 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv2.bin",    0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92) )
ROM_END

ROM_START( macqd700 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )
ROM_END

ROM_START( macpb160 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv3.bin",    0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa) )
ROM_END

ROM_START( macpb180 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac) )

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv3.bin",    0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa) )
ROM_END

ROM_START( macpb180c )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac) )

	ROM_REGION32_BE(0x8000, "vrom", 0)
	ROM_LOAD( "pb180cvrom.bin", 0x0000, 0x8000, CRC(810c75ad) SHA1(3a936e97dee5ceeb25e50197ef504e514ae689a4))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD( "pmuv3.bin",    0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa) )
ROM_END

ROM_START( maccclas )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "ecd99dc0.rom", 0x000000, 0x100000, CRC(c84c3aa5) SHA1(fd9e852e2d77fe17287ba678709b9334d4d74f1e) )
ROM_END

ROM_START( macpd210 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "ecfa989b.rom", 0x000000, 0x100000, CRC(b86ed854) SHA1(ed1371c97117a5884da4a6605ecfc5abed48ae5a) )
ROM_END

ROM_START( maclc520 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "ede66cbd.rom", 0x000000, 0x100000, CRC(a893cb0f) SHA1(c54ee2f45020a4adeb7451adce04cd6e5fb69790) )
ROM_END

/*    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT    CLASS      INIT                COMPANY           FULLNAME */
//COMP( 1983, mactw,     0,        0,      mac128k,  macplus, mac_state, init_mac128k512k,   "Apple Computer", "Macintosh (4.3T Prototype)",  MACHINE_NOT_WORKING )
COMP( 1987, macse,     0,        0,      macse,    macadb,  mac_state, init_macse,         "Apple Computer", "Macintosh SE",  0 )
COMP( 1987, macsefd,   0,        0,      macse,    macadb,  mac_state, init_macse,         "Apple Computer", "Macintosh SE (FDHD)",  0 )
COMP( 1987, macii,     0,        0,      macii,    macadb,  mac_state, init_macii,         "Apple Computer", "Macintosh II",  0 )
COMP( 1987, maciihmu,  macii,    0,      maciihmu, macadb,  mac_state, init_macii,         "Apple Computer", "Macintosh II (w/o 68851 MMU)", 0 )
COMP( 1988, mac2fdhd,  0,        0,      macii,    macadb,  mac_state, init_maciifdhd,     "Apple Computer", "Macintosh II (FDHD)",  0 )
COMP( 1988, maciix,    mac2fdhd, 0,      maciix,   macadb,  mac_state, init_maciix,        "Apple Computer", "Macintosh IIx",  0 )
COMP( 1989, macprtb,   0,        0,      macprtb,  macadb,  mac_state, init_macprtb,       "Apple Computer", "Macintosh Portable", MACHINE_NOT_WORKING )
COMP( 1989, macse30,   mac2fdhd, 0,      macse30,  macadb,  mac_state, init_macse30,       "Apple Computer", "Macintosh SE/30",  0 )
COMP( 1989, maciicx,   mac2fdhd, 0,      maciicx,  macadb,  mac_state, init_maciicx,       "Apple Computer", "Macintosh IIcx",  0 )
COMP( 1989, maciici,   0,        0,      maciici,  maciici, mac_state, init_maciici,       "Apple Computer", "Macintosh IIci", 0 )
COMP( 1990, maciifx,   0,        0,      maciifx,  macadb,  mac_state, init_maciifx,       "Apple Computer", "Macintosh IIfx",  MACHINE_NOT_WORKING )
COMP( 1990, macclasc,  0,        0,      macclasc, macadb,  mac_state, init_macclassic,    "Apple Computer", "Macintosh Classic",  0 )
COMP( 1990, maclc,     0,        0,      maclc,    maciici, mac_state, init_maclc,         "Apple Computer", "Macintosh LC", MACHINE_IMPERFECT_SOUND )
COMP( 1990, maciisi,   0,        0,      maciisi,  maciici, mac_state, init_maciisi,       "Apple Computer", "Macintosh IIsi", 0 )
COMP( 1991, macpb100,  0,        0,      macprtb,  macadb,  mac_state, init_macprtb,       "Apple Computer", "Macintosh PowerBook 100", MACHINE_NOT_WORKING )
COMP( 1991, macpb140,  0,        0,      macpb140, macadb,  mac_state, init_macpb140,      "Apple Computer", "Macintosh PowerBook 140", MACHINE_NOT_WORKING )
COMP( 1991, macpb170,  macpb140, 0,      macpb170, macadb,  mac_state, init_macpb140,      "Apple Computer", "Macintosh PowerBook 170", MACHINE_NOT_WORKING )
COMP( 1991, macqd700,  macpb140, 0,      macqd700, macadb,  mac_state, init_macquadra700,  "Apple Computer", "Macintosh Quadra 700", MACHINE_NOT_WORKING )
COMP( 1991, macclas2,  0,        0,      macclas2, macadb,  mac_state, init_macclassic2,   "Apple Computer", "Macintosh Classic II", MACHINE_IMPERFECT_SOUND )
COMP( 1991, maclc2,    0,        0,      maclc2,   maciici, mac_state, init_maclc2,        "Apple Computer", "Macintosh LC II",  MACHINE_IMPERFECT_SOUND )
COMP( 1992, macpb145,  macpb140, 0,      macpb145, macadb,  mac_state, init_macpb140,      "Apple Computer", "Macintosh PowerBook 145", MACHINE_NOT_WORKING )
COMP( 1992, macpb160,  0,        0,      macpb160, macadb,  mac_state, init_macpb160,      "Apple Computer", "Macintosh PowerBook 160", MACHINE_NOT_WORKING )
COMP( 1992, macpb180,  macpb160, 0,      macpb180, macadb,  mac_state, init_macpb160,      "Apple Computer", "Macintosh PowerBook 180", MACHINE_NOT_WORKING )
COMP( 1992, macpb180c, macpb160, 0,      macpb180c,macadb,  mac_state, init_macpb160,      "Apple Computer", "Macintosh PowerBook 180c", MACHINE_NOT_WORKING )
COMP( 1992, macpd210,  0,        0,      macpd210, macadb,  mac_state, init_macpd210,      "Apple Computer", "Macintosh PowerBook Duo 210", MACHINE_NOT_WORKING )
COMP( 1993, maccclas,  0,        0,      maccclas, macadb,  mac_state, init_maclrcclassic, "Apple Computer", "Macintosh Color Classic", MACHINE_NOT_WORKING )
COMP( 1992, macpb145b, macpb140, 0,      macpb170, macadb,  mac_state, init_macpb140,      "Apple Computer", "Macintosh PowerBook 145B", MACHINE_NOT_WORKING )
COMP( 1993, maclc3,    0,        0,      maclc3,   maciici, mac_state, init_maclc3,        "Apple Computer", "Macintosh LC III",  MACHINE_IMPERFECT_SOUND )
COMP( 1993, maciivx,   0,        0,      maciivx,  maciici, mac_state, init_maciivx,       "Apple Computer", "Macintosh IIvx", MACHINE_IMPERFECT_SOUND )
COMP( 1993, maciivi,   maciivx,  0,      maciivi,  maciici, mac_state, init_maciivx,       "Apple Computer", "Macintosh IIvi", MACHINE_IMPERFECT_SOUND )
COMP( 1993, maclc520,  0,        0,      maclc520, maciici, mac_state, init_maclc520,      "Apple Computer", "Macintosh LC 520",  MACHINE_NOT_WORKING )
COMP( 1994, pmac6100,  0,        0,      pwrmac,   macadb,  mac_state, init_macpm6100,     "Apple Computer", "Power Macintosh 6100/60",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
