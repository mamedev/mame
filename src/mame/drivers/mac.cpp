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
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/m6805/m6805.h"
#include "machine/6522via.h"
#include "machine/applefdc.h"
#include "machine/swim.h"
#include "machine/sonydriv.h"
#include "formats/ap_dsk35.h"
#include "machine/ram.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/scsicd.h"
#include "sound/asc.h"
#include "sound/awacs.h"
#include "sound/cdda.h"

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
#include "bus/nubus/pds30_cb264.h"
#include "bus/nubus/pds30_procolor816.h"
#include "bus/nubus/pds30_sigmalview.h"
#include "bus/nubus/pds30_30hr.h"
#include "bus/nubus/pds30_mc30.h"

// 68000 PDS cards
#include "bus/macpds/pds_tpdfpd.h"

#include "includes/mac.h"
#include "softlist.h"
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

INTERRUPT_GEN_MEMBER(mac_state::mac_rbv_vbl)
{
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
				if ((m_montype ? m_montype->read() : 2) == 1)
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
				if ((m_montype ? m_montype->read() : 2) == 1)
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
		return ((m_montype ? m_montype->read() : 6)<<4);
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
	UINT8 slot_irqs = (~m_rbv_regs[2]) & 0x78;
	slot_irqs &= (m_rbv_regs[0x12] & 0x78);

	if (slot_irqs)
	{
		m_rbv_regs[3] |= 2; // any slot
	}
	else    // no slot irqs, clear the pending bit
	{
		m_rbv_regs[3] &= ~2; // any slot
	}

	UINT8 ifr = (m_rbv_regs[3] & m_rbv_ier) & 0x1b; //m_rbv_regs[0x13]);

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
			data |= ((m_montype ? m_montype->read() : 2)<<3);
//            printf("rbv_r montype: %02x (PC %x)\n", data, space.cpu->safe_pc());
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
	UINT8 rv = (UINT8)(m_hmc_shiftout&1);
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
		UINT64 temp = (data & 1) ? U64(0x400000000) : U64(0x0);
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

static ADDRESS_MAP_START(mac512ke_map, AS_PROGRAM, 16, mac_state )
	AM_RANGE(0x800000, 0x9fffff) AM_READ(mac_scc_r)
	AM_RANGE(0xa00000, 0xbfffff) AM_WRITE(mac_scc_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE(mac_iwm_r, mac_iwm_w)
	AM_RANGE(0xe80000, 0xefffff) AM_READWRITE(mac_via_r, mac_via_w)
	AM_RANGE(0xfffff0, 0xffffff) AM_READWRITE(mac_autovector_r, mac_autovector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(macplus_map, AS_PROGRAM, 16, mac_state )
	AM_RANGE(0x580000, 0x5fffff) AM_READWRITE(macplus_scsi_r, macplus_scsi_w)
	AM_RANGE(0x800000, 0x9fffff) AM_READ(mac_scc_r)
	AM_RANGE(0xa00000, 0xbfffff) AM_WRITE(mac_scc_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE(mac_iwm_r, mac_iwm_w)
	AM_RANGE(0xe80000, 0xefffff) AM_READWRITE(mac_via_r, mac_via_w)
	AM_RANGE(0xfffff0, 0xffffff) AM_READWRITE(mac_autovector_r, mac_autovector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(macse_map, AS_PROGRAM, 16, mac_state )
	AM_RANGE(0x580000, 0x5fffff) AM_READWRITE(macplus_scsi_r, macplus_scsi_w)
	AM_RANGE(0x900000, 0x9fffff) AM_READ(mac_scc_r)
	AM_RANGE(0xb00000, 0xbfffff) AM_WRITE(mac_scc_w)
	AM_RANGE(0xd00000, 0xdfffff) AM_READWRITE(mac_iwm_r, mac_iwm_w)
	AM_RANGE(0xe80000, 0xefffff) AM_READWRITE(mac_via_r, mac_via_w)
	AM_RANGE(0xfffff0, 0xffffff) AM_READWRITE(mac_autovector_r, mac_autovector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(macprtb_map, AS_PROGRAM, 16, mac_state )
	AM_RANGE(0x900000, 0x93ffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0c0000)
	AM_RANGE(0xf60000, 0xf6ffff) AM_READWRITE(mac_iwm_r, mac_iwm_w)
	AM_RANGE(0xf70000, 0xf7ffff) AM_READWRITE(mac_via_r, mac_via_w)
	AM_RANGE(0xf90000, 0xf9ffff) AM_READWRITE(macplus_scsi_r, macplus_scsi_w)
	AM_RANGE(0xfa8000, 0xfaffff) AM_RAM AM_SHARE("vram16")  // VRAM
	AM_RANGE(0xfb0000, 0xfbffff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffff)
	AM_RANGE(0xfc0000, 0xfcffff) AM_READ(mac_config_r)
	AM_RANGE(0xfd0000, 0xfdffff) AM_READWRITE(mac_scc_r, mac_scc_2_w)
	AM_RANGE(0xfffff0, 0xffffff) AM_READWRITE(mac_autovector_r, mac_autovector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(maclc_map, AS_PROGRAM, 32, mac_state )
	ADDRESS_MAP_GLOBAL_MASK(0x80ffffff) // V8 uses bit 31 and 23-0 for address decoding only

	AM_RANGE(0xa00000, 0xafffff) AM_ROM AM_REGION("bootrom", 0) // ROM (in 32-bit mode)

	AM_RANGE(0xf00000, 0xf01fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff)
	AM_RANGE(0xf04000, 0xf05fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff)
	AM_RANGE(0xf06000, 0xf07fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w)
	AM_RANGE(0xf10000, 0xf11fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff)
	AM_RANGE(0xf12000, 0xf13fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w)
	AM_RANGE(0xf14000, 0xf15fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff)
	AM_RANGE(0xf16000, 0xf17fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff)
	AM_RANGE(0xf24000, 0xf24003) AM_READWRITE(rbv_ramdac_r, ariel_ramdac_w)
	AM_RANGE(0xf26000, 0xf27fff) AM_READWRITE8(mac_rbv_r, mac_rbv_w, 0xffffffff)    // VIA2 (V8)
	AM_RANGE(0xf40000, 0xfbffff) AM_RAM AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(maclc3_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x400fffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff00000)

	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50004000, 0x50005fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50006000, 0x50007fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50010000, 0x50011fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50012000, 0x50013fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50014000, 0x50015fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50016000, 0x50017fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50024000, 0x50025fff) AM_WRITE( ariel_ramdac_w ) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50026000, 0x50027fff) AM_READWRITE8(mac_rbv_r, mac_rbv_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50028000, 0x50028003) AM_READWRITE8(mac_sonora_vctl_r, mac_sonora_vctl_w, 0xffffffff) AM_MIRROR(0x00f00000)

	AM_RANGE(0x5ffffffc, 0x5fffffff) AM_READ(mac_read_id)

	AM_RANGE(0x60000000, 0x600fffff) AM_RAM AM_MIRROR(0x0ff00000) AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(macii_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x4003ffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ffc0000)

	// MMU remaps I/O without the F
	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50002000, 0x50003fff) AM_READWRITE16(mac_via2_r, mac_via2_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50004000, 0x50005fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50006000, 0x50006003) AM_WRITE(macii_scsi_drq_w) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50006060, 0x50006063) AM_READ(macii_scsi_drq_r) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50010000, 0x50011fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50012060, 0x50012063) AM_READ(macii_scsi_drq_r) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50014000, 0x50015fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50016000, 0x50017fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50040000, 0x50041fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(maciici_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x4007ffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff80000)

	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50004000, 0x50005fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50006000, 0x50007fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50010000, 0x50011fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50012060, 0x50012063) AM_READ(macii_scsi_drq_r) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50014000, 0x50015fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50016000, 0x50017fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50024000, 0x50024007) AM_WRITE( rbv_ramdac_w ) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50026000, 0x50027fff) AM_READWRITE8(mac_rbv_r, mac_rbv_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50040000, 0x50041fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(macse30_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x4003ffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ffc0000)

	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50002000, 0x50003fff) AM_READWRITE16(mac_via2_r, mac_via2_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50004000, 0x50005fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50006000, 0x50007fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50010000, 0x50011fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50012060, 0x50012063) AM_READ(macii_scsi_drq_r) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50014000, 0x50015fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50016000, 0x50017fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50040000, 0x50041fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000) // mirror

	AM_RANGE(0xfe000000, 0xfe00ffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xfee00000, 0xfee0ffff) AM_RAM AM_SHARE("vram") AM_MIRROR(0x000f0000)
	AM_RANGE(0xfeffe000, 0xfeffffff) AM_ROM AM_REGION("se30vrom", 0x0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(maciifx_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x4007ffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff80000)

	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50004000, 0x50005fff) AM_READWRITE8(scciop_r, scciop_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x5000a000, 0x5000bfff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x5000c060, 0x5000c063) AM_READ(macii_scsi_drq_r) AM_MIRROR(0x00f00000)
	AM_RANGE(0x5000d000, 0x5000d003) AM_WRITE(macii_scsi_drq_w) AM_MIRROR(0x00f00000)
	AM_RANGE(0x5000d060, 0x5000d063) AM_READ(macii_scsi_drq_r) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50010000, 0x50011fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50012000, 0x50013fff) AM_READWRITE8(swimiop_r, swimiop_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50018000, 0x50019fff) AM_READWRITE(biu_r, biu_w) AM_MIRROR(0x00f00000)
	AM_RANGE(0x5001a000, 0x5001bfff) AM_READWRITE8(oss_r, oss_w, 0xffffffff) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50024000, 0x50027fff) AM_READ(buserror_r) AM_MIRROR(0x00f00000)   // must bus error on access here so ROM can determine we're an FMC
	AM_RANGE(0x50040000, 0x50041fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00f00000)
ADDRESS_MAP_END

// ROM detects the "Jaws" ASIC by checking for I/O space mirrored at 0x01000000 boundries
static ADDRESS_MAP_START(macpb140_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x400fffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff00000)

	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50002000, 0x50003fff) AM_READWRITE16(mac_via2_r, mac_via2_w, 0xffffffff) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50004000, 0x50005fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50006000, 0x50007fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50010000, 0x50011fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50012060, 0x50012063) AM_READ(macii_scsi_drq_r) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50014000, 0x50015fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50016000, 0x50017fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50024000, 0x50027fff) AM_READ(buserror_r) AM_MIRROR(0x01f00000)   // bus error here to make sure we aren't mistaken for another decoder

	AM_RANGE(0xfee08000, 0xfeffffff) AM_RAM AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(macpb160_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x400fffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff00000)

	AM_RANGE(0x50f00000, 0x50f01fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff)
	AM_RANGE(0x50f02000, 0x50f03fff) AM_READWRITE16(mac_via2_r, mac_via2_w, 0xffffffff)
	AM_RANGE(0x50f04000, 0x50f05fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff)
	AM_RANGE(0x50f06000, 0x50f07fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w)
	AM_RANGE(0x50f10000, 0x50f11fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff)
	AM_RANGE(0x50f12060, 0x50f12063) AM_READ(macii_scsi_drq_r)
	AM_RANGE(0x50f14000, 0x50f15fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff)
	AM_RANGE(0x50f16000, 0x50f17fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff)
	AM_RANGE(0x50f20000, 0x50f21fff) AM_READWRITE8(mac_gsc_r, mac_gsc_w, 0xffffffff)
	AM_RANGE(0x50f24000, 0x50f27fff) AM_READ(buserror_r)   // bus error here to make sure we aren't mistaken for another decoder

	AM_RANGE(0x60000000, 0x6001ffff) AM_RAM AM_SHARE("vram") AM_MIRROR(0x0ffe0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(macpb165c_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x400fffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff00000)

	AM_RANGE(0x50f00000, 0x50f01fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff)
	AM_RANGE(0x50f02000, 0x50f03fff) AM_READWRITE16(mac_via2_r, mac_via2_w, 0xffffffff)
	AM_RANGE(0x50f04000, 0x50f05fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff)
	AM_RANGE(0x50f06000, 0x50f07fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w)
	AM_RANGE(0x50f10000, 0x50f11fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff)
	AM_RANGE(0x50f12060, 0x50f12063) AM_READ(macii_scsi_drq_r)
	AM_RANGE(0x50f14000, 0x50f15fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff)
	AM_RANGE(0x50f16000, 0x50f17fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff)
	AM_RANGE(0x50f20000, 0x50f21fff) AM_READ(buserror_r)    // bus error here to detect we're not the grayscale 160/165/180
	AM_RANGE(0x50f24000, 0x50f27fff) AM_READ(buserror_r)   // bus error here to make sure we aren't mistaken for another decoder

	// on-board color video on 165c/180c
	AM_RANGE(0xfc000000, 0xfc07ffff) AM_RAM AM_SHARE("vram") AM_MIRROR(0x00380000) // 512k of VRAM
	AM_RANGE(0xfc400000, 0xfcefffff) AM_READWRITE(macwd_r, macwd_w)
// fc4003c8 = DAC control, fc4003c9 = DAC data
// fc4003da bit 3 is VBL
	AM_RANGE(0xfcff8000, 0xfcffffff) AM_ROM AM_REGION("vrom", 0x0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(macpd210_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x400fffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff00000)

	AM_RANGE(0x50f00000, 0x50f01fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff)
	AM_RANGE(0x50f02000, 0x50f03fff) AM_READWRITE16(mac_via2_r, mac_via2_w, 0xffffffff)
	AM_RANGE(0x50f04000, 0x50f05fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff)
	AM_RANGE(0x50f06000, 0x50f07fff) AM_READWRITE(macii_scsi_drq_r, macii_scsi_drq_w)
	AM_RANGE(0x50f10000, 0x50f11fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, 0xffffffff)
	AM_RANGE(0x50f12060, 0x50f12063) AM_READ(macii_scsi_drq_r)
	AM_RANGE(0x50f14000, 0x50f15fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff)
	AM_RANGE(0x50f16000, 0x50f17fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff)
	AM_RANGE(0x50f20000, 0x50f21fff) AM_READWRITE8(mac_gsc_r, mac_gsc_w, 0xffffffff)
	AM_RANGE(0x50f24000, 0x50f27fff) AM_READ(buserror_r)   // bus error here to make sure we aren't mistaken for another decoder

	AM_RANGE(0x5ffffffc, 0x5fffffff) AM_READ(mac_read_id)

	AM_RANGE(0x60000000, 0x6001ffff) AM_RAM AM_SHARE("vram") AM_MIRROR(0x0ffe0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(quadra700_map, AS_PROGRAM, 32, mac_state )
	AM_RANGE(0x40000000, 0x400fffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0ff00000)

	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00fc0000)
	AM_RANGE(0x50002000, 0x50003fff) AM_READWRITE16(mac_via2_r, mac_via2_w, 0xffffffff) AM_MIRROR(0x00fc0000)
// 50008000 = Ethernet MAC ID PROM
// 5000a000 = Sonic (DP83932) ethernet
// 5000f000 = SCSI cf96, 5000f402 = SCSI #2 cf96
	AM_RANGE(0x5000f000, 0x5000f3ff) AM_READWRITE8(mac_5396_r, mac_5396_w, 0xffffffff) AM_MIRROR(0x00fc0000)
	AM_RANGE(0x5000c000, 0x5000dfff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, 0xffffffff) AM_MIRROR(0x00fc0000)
	AM_RANGE(0x50014000, 0x50015fff) AM_DEVREADWRITE8("asc", asc_device, read, write, 0xffffffff) AM_MIRROR(0x00fc0000)
	AM_RANGE(0x5001e000, 0x5001ffff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, 0xffffffff) AM_MIRROR(0x00fc0000)

	AM_RANGE(0x50040000, 0x50041fff) AM_READWRITE16(mac_via_r, mac_via_w, 0xffffffff) AM_MIRROR(0x00fc0000)
	// f9800000 = VDAC / DAFB
	AM_RANGE(0xf9000000, 0xf91fffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xf9800000, 0xf98001ff) AM_READWRITE(dafb_r, dafb_w)
	AM_RANGE(0xf9800200, 0xf980023f) AM_READWRITE(dafb_dac_r, dafb_dac_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(pwrmac_map, AS_PROGRAM, 64, mac_state )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM // 8 MB standard

	AM_RANGE(0x40000000, 0x403fffff) AM_ROM AM_REGION("bootrom", 0) AM_MIRROR(0x0fc00000)

	AM_RANGE(0x50000000, 0x50001fff) AM_READWRITE16(mac_via_r, mac_via_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50004000, 0x50005fff) AM_READWRITE16(mac_scc_r, mac_scc_2_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	// 50008000 = ethernet ID PROM
	// 5000a000 = MACE ethernet controller
	AM_RANGE(0x50010000, 0x50011fff) AM_READWRITE16(macplus_scsi_r, macii_scsi_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	// 50014000 = sound registers (AWACS)
	AM_RANGE(0x50014000, 0x50015fff) AM_DEVREADWRITE8("awacs", awacs_device, read, write, U64(0xffffffffffffffff)) AM_MIRROR(0x01f00000)
	AM_RANGE(0x50016000, 0x50017fff) AM_READWRITE16(mac_iwm_r, mac_iwm_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50024000, 0x50025fff) AM_WRITE32( ariel_ramdac_w, U64(0xffffffffffffffff) ) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50026000, 0x50027fff) AM_READWRITE16(mac_via2_r, mac_via2_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50028000, 0x50028007) AM_READWRITE8(mac_sonora_vctl_r, mac_sonora_vctl_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	// 5002a000 = interrupt controller
	// 5002c000 = diagnostic registers
	AM_RANGE(0x5002c000, 0x5002dfff) AM_READ8(pmac_diag_r, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50031000, 0x50032fff) AM_READWRITE8(amic_dma_r, amic_dma_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	AM_RANGE(0x50040000, 0x5004000f) AM_READWRITE8(hmc_r, hmc_w, U64(0xffffffffffffffff)) AM_MIRROR(0x00f00000)
	AM_RANGE(0x5ffffff8, 0x5fffffff) AM_READ32(mac_read_id, U64(0xffffffffffffffff))

	AM_RANGE(0xffc00000, 0xffffffff) AM_ROM AM_REGION("bootrom", 0)
ADDRESS_MAP_END

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

static SLOT_INTERFACE_START(mac_nubus_cards)
	SLOT_INTERFACE("m2video", NUBUS_M2VIDEO)    /* Apple Macintosh II Video Card */
	SLOT_INTERFACE("48gc", NUBUS_48GC)      /* Apple 4*8 Graphics Card */
	SLOT_INTERFACE("824gc", NUBUS_824GC)    /* Apple 8*24 Graphics Card */
	SLOT_INTERFACE("cb264", NUBUS_CB264)    /* RasterOps ColorBoard 264 */
	SLOT_INTERFACE("vikbw", NUBUS_VIKBW)    /* Moniterm Viking board */
	SLOT_INTERFACE("image", NUBUS_IMAGE)    /* Disk Image Pseudo-Card */
	SLOT_INTERFACE("specpdq", NUBUS_SPECPDQ)    /* SuperMac Spectrum PDQ */
	SLOT_INTERFACE("m2hires", NUBUS_M2HIRES)    /* Apple Macintosh II Hi-Resolution Card */
	SLOT_INTERFACE("spec8s3", NUBUS_SPEC8S3)    /* SuperMac Spectrum/8 Series III */
//  SLOT_INTERFACE("thundergx", NUBUS_THUNDERGX)        /* Radius Thunder GX (not yet) */
	SLOT_INTERFACE("radiustpd", NUBUS_RADIUSTPD)        /* Radius Two Page Display */
	SLOT_INTERFACE("asmc3nb", NUBUS_ASNTMC3NB)  /* Asante MC3NB Ethernet card */
	SLOT_INTERFACE("portrait", NUBUS_WSPORTRAIT)    /* Apple Macintosh II Portrait video card */
	SLOT_INTERFACE("enetnb", NUBUS_APPLEENET)   /* Apple NuBus Ethernet */
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(mac_pds030_cards)
	SLOT_INTERFACE("cb264", PDS030_CB264SE30)   // RasterOps Colorboard 264/SE30
	SLOT_INTERFACE("pc816", PDS030_PROCOLOR816) // Lapis ProColor Server 8*16 PDS
	SLOT_INTERFACE("lview", PDS030_LVIEW)       // Sigma Designs L-View
	SLOT_INTERFACE("30hr",  PDS030_XCEED30HR)   // Micron/XCEED Technology Color 30HR
	SLOT_INTERFACE("mc30",  PDS030_XCEEDMC30)   // Micron/XCEED Technology MacroColor 30
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(mac_sepds_cards)
	SLOT_INTERFACE("radiusfpd", PDS_SEDISPLAY)  // Radius Full Page Display card for SE
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(mac_lcpds_cards)
SLOT_INTERFACE_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static const floppy_interface mac_floppy_interface =
{
	FLOPPY_STANDARD_3_5_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_mac),
	"floppy_3_5"
};

static MACHINE_CONFIG_START( mac512ke, mac_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, C7M)        /* 7.8336 MHz */
	MCFG_CPU_PROGRAM_MAP(mac512ke_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.15)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1260))
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_SIZE(MAC_H_TOTAL, MAC_V_TOTAL)
	MCFG_SCREEN_VISIBLE_AREA(0, MAC_H_VIS-1, 0, MAC_V_VIS-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_mac)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(mac_state,mac)

	MCFG_VIDEO_START_OVERRIDE(mac_state,mac)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", MAC_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* devices */
	MCFG_RTC3430042_ADD("rtc", XTAL_32_768kHz)
	MCFG_IWM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, 1000000)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	MCFG_MACKBD_ADD(MACKBD_TAG)
#ifdef MAC_USE_EMULATED_KBD
	MCFG_MACKBD_DATAOUT_HANDLER(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
	MCFG_MACKBD_CLKOUT_HANDLER(WRITELINE(mac_state, mac_kbd_clk_in))
#endif

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mac128k, mac512ke )

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( macplus, mac512ke )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(macplus_map)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_MODIFY(mac_floppy_interface)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,2560K,4M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop35_list","mac_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( macse, macplus )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(macse_map)

	MCFG_DEVICE_REMOVE("via6522_0")
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 1000000)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_bbadb))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("2M,2560K,4M")

	MCFG_MACKBD_REMOVE(MACKBD_TAG)

	MCFG_MACPDS_BUS_ADD("sepds", "maincpu")
	MCFG_MACPDS_SLOT_ADD("sepds", "pds", mac_sepds_cards, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( macclasc, macplus )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(macse_map)

	MCFG_DEVICE_REMOVE("via6522_0")
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 1000000)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_bbadb))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("2M,2560K,4M")

	MCFG_MACKBD_REMOVE(MACKBD_TAG)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( macprtb, mac_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, C15M)
	MCFG_CPU_PROGRAM_MAP(macprtb_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.15)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1260))
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_SIZE(700, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 399)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macprtb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(mac_state,mac)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macprtb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ASC_ADD("asc", C15M, ASC_TYPE_ASC, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_RTC3430042_ADD("rtc", XTAL_32_768kHz)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_IWM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a_pmu))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b_pmu))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a_pmu))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_pmu))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")
	MCFG_RAM_EXTRA_OPTIONS("1M,3M,5M,7M,9M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( macii, mac_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68020PMMU, C15M)
	MCFG_CPU_PROGRAM_MAP(macii_map)

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ASC_ADD("asc", C15M, ASC_TYPE_ASC, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_RTC3430042_ADD("rtc", XTAL_32_768kHz)
	MCFG_DEVICE_ADD("nubus", NUBUS, 0)
	MCFG_NUBUS_CPU("maincpu")
	MCFG_NUBUS_OUT_IRQ9_CB(WRITELINE(mac_state, nubus_irq_9_w))
	MCFG_NUBUS_OUT_IRQA_CB(WRITELINE(mac_state, nubus_irq_a_w))
	MCFG_NUBUS_OUT_IRQB_CB(WRITELINE(mac_state, nubus_irq_b_w))
	MCFG_NUBUS_OUT_IRQC_CB(WRITELINE(mac_state, nubus_irq_c_w))
	MCFG_NUBUS_OUT_IRQD_CB(WRITELINE(mac_state, nubus_irq_d_w))
	MCFG_NUBUS_OUT_IRQE_CB(WRITELINE(mac_state, nubus_irq_e_w))
	MCFG_NUBUS_SLOT_ADD("nubus","nb9", mac_nubus_cards, "48gc")
	MCFG_NUBUS_SLOT_ADD("nubus","nba", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbb", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbc", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbd", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbe", mac_nubus_cards, NULL)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE3, "cdrom", SCSICD, SCSI_ID_4)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_IWM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, C7M/10)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_bbadb))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, C7M/10)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state, mac_via2_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state, mac_via2_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via2_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via2_out_b))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via2_irq))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("8M,32M,64M,96M,128M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop35_list","mac_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maciihmu, macii )
	MCFG_CPU_REPLACE("maincpu", M68020HMMU, C15M)
	MCFG_CPU_PROGRAM_MAP(macii_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( maciifx, mac_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68030, 40000000)
	MCFG_CPU_PROGRAM_MAP(maciifx_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ASC_ADD("asc", C15M, ASC_TYPE_ASC, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_RTC3430042_ADD("rtc", XTAL_32_768kHz)
	MCFG_DEVICE_ADD("nubus", NUBUS, 0)
	MCFG_NUBUS_CPU("maincpu")
	MCFG_NUBUS_OUT_IRQ9_CB(WRITELINE(mac_state, nubus_irq_9_w))
	MCFG_NUBUS_OUT_IRQA_CB(WRITELINE(mac_state, nubus_irq_a_w))
	MCFG_NUBUS_OUT_IRQB_CB(WRITELINE(mac_state, nubus_irq_b_w))
	MCFG_NUBUS_OUT_IRQC_CB(WRITELINE(mac_state, nubus_irq_c_w))
	MCFG_NUBUS_OUT_IRQD_CB(WRITELINE(mac_state, nubus_irq_d_w))
	MCFG_NUBUS_OUT_IRQE_CB(WRITELINE(mac_state, nubus_irq_e_w))
	MCFG_NUBUS_SLOT_ADD("nubus","nb9", mac_nubus_cards, "48gc")
	MCFG_NUBUS_SLOT_ADD("nubus","nba", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbb", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbc", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbd", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbe", mac_nubus_cards, NULL)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_IWM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, C7M/10)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,16M,32M,64M,96M,128M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop35_list","mac_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maclc, macii )

	MCFG_CPU_REPLACE("maincpu", M68020HMMU, C15M)
	MCFG_CPU_PROGRAM_MAP(maclc_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(256)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macv8)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macv8)
	MCFG_DEFAULT_LAYOUT(layout_mac)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("4M,6M,8M,10M")

	MCFG_DEVICE_MODIFY("via6522_0")
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_egadb))

	MCFG_NUBUS_SLOT_REMOVE("nb9")
	MCFG_NUBUS_SLOT_REMOVE("nba")
	MCFG_NUBUS_SLOT_REMOVE("nbb")
	MCFG_NUBUS_SLOT_REMOVE("nbc")
	MCFG_NUBUS_SLOT_REMOVE("nbd")
	MCFG_NUBUS_SLOT_REMOVE("nbe")
	MCFG_DEVICE_REMOVE("nubus")

	MCFG_DEVICE_ADD("pds", NUBUS, 0)
	MCFG_NUBUS_CPU("maincpu")
	MCFG_NUBUS_OUT_IRQ9_CB(WRITELINE(mac_state, nubus_irq_9_w))
	MCFG_NUBUS_OUT_IRQA_CB(WRITELINE(mac_state, nubus_irq_a_w))
	MCFG_NUBUS_OUT_IRQB_CB(WRITELINE(mac_state, nubus_irq_b_w))
	MCFG_NUBUS_OUT_IRQC_CB(WRITELINE(mac_state, nubus_irq_c_w))
	MCFG_NUBUS_OUT_IRQD_CB(WRITELINE(mac_state, nubus_irq_d_w))
	MCFG_NUBUS_OUT_IRQE_CB(WRITELINE(mac_state, nubus_irq_e_w))
	MCFG_NUBUS_SLOT_ADD("pds","lcpds", mac_lcpds_cards, NULL)

	MCFG_ASC_REPLACE("asc", C15M, ASC_TYPE_V8, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_EGRET_ADD(EGRET_341S0850)
	MCFG_EGRET_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_EGRET_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_EGRET_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_EGRET_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maclc2, maclc )

	MCFG_CPU_REPLACE("maincpu", M68030, C15M)
	MCFG_CPU_PROGRAM_MAP(maclc_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("6M,8M,10M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maccclas, maclc2 )

	MCFG_EGRET_REMOVE()
	MCFG_CUDA_ADD(CUDA_341S0788)    // should be 0417, but that version won't sync up properly with the '030 right now
	MCFG_CUDA_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_CUDA_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_CUDA_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_CUDA_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))

	MCFG_DEVICE_MODIFY("via6522_0")
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_cdadb))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maclc3, maclc )

	MCFG_CPU_REPLACE("maincpu", M68030, 25000000)
	MCFG_CPU_PROGRAM_MAP(maclc3_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macsonora)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macsonora)

	MCFG_SCREEN_MODIFY(MAC_SCREEN_NAME)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macsonora)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,16M,32M,48M,64M,80M")

	MCFG_ASC_REPLACE("asc", C15M, ASC_TYPE_SONORA, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_EGRET_REPLACE(EGRET_341S0851)
	MCFG_EGRET_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_EGRET_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_EGRET_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_EGRET_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maclc520, maclc3 )

	MCFG_EGRET_REMOVE()
	MCFG_CUDA_ADD(CUDA_341S0060)
	MCFG_CUDA_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_CUDA_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_CUDA_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_CUDA_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))

	MCFG_DEVICE_MODIFY("via6522_0")
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_cdadb))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maciivx, maclc )

	MCFG_CPU_REPLACE("maincpu", M68030, C32M)
	MCFG_CPU_PROGRAM_MAP(maclc3_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macv8)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	MCFG_SCREEN_MODIFY(MAC_SCREEN_NAME)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macrbvvram)

	MCFG_DEVICE_ADD("nubus", NUBUS, 0)
	MCFG_NUBUS_CPU("maincpu")
	MCFG_NUBUS_OUT_IRQ9_CB(WRITELINE(mac_state, nubus_irq_9_w))
	MCFG_NUBUS_OUT_IRQA_CB(WRITELINE(mac_state, nubus_irq_a_w))
	MCFG_NUBUS_OUT_IRQB_CB(WRITELINE(mac_state, nubus_irq_b_w))
	MCFG_NUBUS_OUT_IRQC_CB(WRITELINE(mac_state, nubus_irq_c_w))
	MCFG_NUBUS_OUT_IRQD_CB(WRITELINE(mac_state, nubus_irq_d_w))
	MCFG_NUBUS_OUT_IRQE_CB(WRITELINE(mac_state, nubus_irq_e_w))
	MCFG_NUBUS_SLOT_ADD("nubus","nbc", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbd", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbe", mac_nubus_cards, NULL)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,12M,16M,20M,24M,28M,32M,36M,40M,44M,48M,52M,56M,60M,64M")

	MCFG_EGRET_REPLACE(EGRET_341S0851)
	MCFG_EGRET_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_EGRET_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_EGRET_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_EGRET_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maciivi, maclc )

	MCFG_CPU_REPLACE("maincpu", M68030, C15M)
	MCFG_CPU_PROGRAM_MAP(maclc3_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macv8)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	MCFG_SCREEN_MODIFY(MAC_SCREEN_NAME)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macrbvvram)

	MCFG_DEVICE_ADD("nubus", NUBUS, 0)
	MCFG_NUBUS_CPU("maincpu")
	MCFG_NUBUS_OUT_IRQ9_CB(WRITELINE(mac_state, nubus_irq_9_w))
	MCFG_NUBUS_OUT_IRQA_CB(WRITELINE(mac_state, nubus_irq_a_w))
	MCFG_NUBUS_OUT_IRQB_CB(WRITELINE(mac_state, nubus_irq_b_w))
	MCFG_NUBUS_OUT_IRQC_CB(WRITELINE(mac_state, nubus_irq_c_w))
	MCFG_NUBUS_OUT_IRQD_CB(WRITELINE(mac_state, nubus_irq_d_w))
	MCFG_NUBUS_OUT_IRQE_CB(WRITELINE(mac_state, nubus_irq_e_w))
	MCFG_NUBUS_SLOT_ADD("nubus","nbc", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbd", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbe", mac_nubus_cards, NULL)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,12M,16M,20M,24M,28M,32M,36M,40M,44M,48M,52M,56M,60M,64M")

	MCFG_EGRET_REPLACE(EGRET_341S0851)
	MCFG_EGRET_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_EGRET_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_EGRET_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_EGRET_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maciix, macii )

	MCFG_CPU_REPLACE("maincpu", M68030, C15M)
	MCFG_CPU_PROGRAM_MAP(macii_map)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("8M,32M,64M,96M,128M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maciicx, maciix )    // IIcx is a IIx with only slots 9/a/b
	MCFG_NUBUS_SLOT_REMOVE("nbc")
	MCFG_NUBUS_SLOT_REMOVE("nbd")
	MCFG_NUBUS_SLOT_REMOVE("nbe")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( macse30, mac_state )

	MCFG_CPU_ADD("maincpu", M68030, C15M)
	MCFG_CPU_PROGRAM_MAP(macse30_map)

	/* video hardware */
	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60.15)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1260))
	MCFG_SCREEN_SIZE(MAC_H_TOTAL, MAC_V_TOTAL)
	MCFG_SCREEN_VISIBLE_AREA(0, MAC_H_VIS-1, 0, MAC_V_VIS-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macse30)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(mac_state,mac)

	MCFG_VIDEO_START_OVERRIDE(mac_state,mac)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ASC_ADD("asc", C15M, ASC_TYPE_ASC, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_RTC3430042_ADD("rtc", XTAL_32_768kHz)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_DEVICE_ADD("pdss", NUBUS, 0)
	MCFG_NUBUS_CPU("maincpu")
	MCFG_NUBUS_OUT_IRQ9_CB(WRITELINE(mac_state, nubus_irq_9_w))
	MCFG_NUBUS_OUT_IRQA_CB(WRITELINE(mac_state, nubus_irq_a_w))
	MCFG_NUBUS_OUT_IRQB_CB(WRITELINE(mac_state, nubus_irq_b_w))
	MCFG_NUBUS_OUT_IRQC_CB(WRITELINE(mac_state, nubus_irq_c_w))
	MCFG_NUBUS_OUT_IRQD_CB(WRITELINE(mac_state, nubus_irq_d_w))
	MCFG_NUBUS_OUT_IRQE_CB(WRITELINE(mac_state, nubus_irq_e_w))
	MCFG_NUBUS_SLOT_ADD("pds","pds030", mac_pds030_cards, NULL)

	MCFG_SWIM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_bbadb))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state, mac_via2_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state, mac_via2_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via2_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via2_out_b))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via2_irq))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("8M,16M,32M,48M,64M,96M,128M")

	MCFG_SOFTWARE_LIST_ADD("flop35_list","mac_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( macpb140, mac_state )

	MCFG_CPU_ADD("maincpu", M68030, C15M)
	MCFG_CPU_PROGRAM_MAP(macpb140_map)

	/* video hardware */
	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60.15)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1260))
	MCFG_SCREEN_SIZE(700, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 399)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macpb140)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(mac_state,mac)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macprtb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ASC_ADD("asc", C15M, ASC_TYPE_ASC, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_SWIM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b_via2pmu))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_via2pmu))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state, mac_via2_in_a_pmu))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state, mac_via2_in_b_pmu))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via2_out_a_pmu))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via2_out_b_pmu))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via2_irq))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("4M,6M,8M")

	MCFG_SOFTWARE_LIST_ADD("flop35_list","mac_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

// PowerBook 145 = 140 @ 25 MHz (still 2MB RAM - the 145B upped that to 4MB)
static MACHINE_CONFIG_DERIVED( macpb145, macpb140 )
	MCFG_CPU_REPLACE("maincpu", M68030, 25000000)
	MCFG_CPU_PROGRAM_MAP(macpb140_map)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("6M,8M")
MACHINE_CONFIG_END

// PowerBook 170 = 140 @ 25 MHz with an active-matrix LCD (140/145/145B were passive)
static MACHINE_CONFIG_DERIVED( macpb170, macpb140 )
	MCFG_CPU_REPLACE("maincpu", M68030, 25000000)
	MCFG_CPU_PROGRAM_MAP(macpb140_map)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("6M,8M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( macpb160, mac_state )

	MCFG_CPU_ADD("maincpu", M68030, 25000000)
	MCFG_CPU_PROGRAM_MAP(macpb160_map)

	/* video hardware */
	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60.15)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1260))
	MCFG_SCREEN_SIZE(700, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 399)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macpb160)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(mac_state,macgsc)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macprtb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ASC_ADD("asc", C15M, ASC_TYPE_ASC, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_SWIM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b_via2pmu))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_via2pmu))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state, mac_via2_in_a_pmu))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state, mac_via2_in_b_pmu))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via2_out_a_pmu))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via2_out_b_pmu))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via2_irq))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,12M,16M")

	MCFG_SOFTWARE_LIST_ADD("flop35_list","mac_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( macpb180, macpb160 )
	MCFG_CPU_REPLACE("maincpu", M68030, 33000000)
	MCFG_CPU_PROGRAM_MAP(macpb160_map)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,12M,16M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( macpb180c, macpb160 )
	MCFG_CPU_REPLACE("maincpu", M68030, 33000000)
	MCFG_CPU_PROGRAM_MAP(macpb165c_map)

	MCFG_SCREEN_MODIFY(MAC_SCREEN_NAME)
	MCFG_SCREEN_SIZE(800, 525)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macpbwd)
	MCFG_SCREEN_NO_PALETTE

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,12M,16M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( macpd210, macpb160 )
	MCFG_CPU_REPLACE("maincpu", M68030, 25000000)
	MCFG_CPU_PROGRAM_MAP(macpd210_map)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,12M,16M,20M,24M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( macclas2, maclc )
	MCFG_CPU_REPLACE("maincpu", M68030, C15M)
	MCFG_CPU_PROGRAM_MAP(maclc_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macv8)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,maceagle)

	MCFG_SCREEN_MODIFY(MAC_SCREEN_NAME)
	MCFG_SCREEN_SIZE(MAC_H_TOTAL, MAC_V_TOTAL)
	MCFG_SCREEN_VISIBLE_AREA(0, MAC_H_VIS-1, 0, MAC_V_VIS-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macrbv)

	MCFG_ASC_REPLACE("asc", C15M, ASC_TYPE_EAGLE, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("10M")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,6M,8M,10M")

	MCFG_EGRET_REPLACE(EGRET_341S0851)
	MCFG_EGRET_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_EGRET_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_EGRET_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_EGRET_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maciici, macii )

	MCFG_CPU_REPLACE("maincpu", M68030, 25000000)
	MCFG_CPU_PROGRAM_MAP(maciici_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(256)

	// IIci only has slots c/d/e
	MCFG_NUBUS_SLOT_REMOVE("nb9")
	MCFG_NUBUS_SLOT_REMOVE("nba")
	MCFG_NUBUS_SLOT_REMOVE("nbb")

	MCFG_VIDEO_START_OVERRIDE(mac_state,macrbv)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(640, 870)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macrbv)
	MCFG_DEFAULT_LAYOUT(layout_mac)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("4M,8M,16M,32M,48M,64M,128M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maciisi, macii )

	MCFG_CPU_REPLACE("maincpu", M68030, 20000000)
	MCFG_CPU_PROGRAM_MAP(maciici_map)
	MCFG_CPU_VBLANK_INT_DRIVER(MAC_SCREEN_NAME, mac_state,  mac_rbv_vbl)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(256)

	MCFG_NUBUS_SLOT_REMOVE("nb9")
	MCFG_NUBUS_SLOT_REMOVE("nba")
	MCFG_NUBUS_SLOT_REMOVE("nbb")
	MCFG_NUBUS_SLOT_REMOVE("nbc")
	MCFG_NUBUS_SLOT_REMOVE("nbd")
	MCFG_NUBUS_SLOT_REMOVE("nbe")
	MCFG_DEVICE_REMOVE("nubus")

	MCFG_VIDEO_START_OVERRIDE(mac_state,macrbv)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(640, 870)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macrbv)
	MCFG_DEFAULT_LAYOUT(layout_mac)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("4M,8M,16M,32M,48M,64M,128M")

	MCFG_DEVICE_MODIFY("via6522_0")
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_egadb))

	MCFG_EGRET_ADD(EGRET_344S0100)
	MCFG_EGRET_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_EGRET_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_EGRET_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_EGRET_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pwrmac, mac_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC601, 60000000)
	MCFG_CPU_PROGRAM_MAP(pwrmac_map)

	/* video hardware */
	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	// dot clock, htotal, hstart, hend, vtotal, vstart, vend
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_SIZE(1024, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macrbv)

	MCFG_PALETTE_ADD("palette", 256)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macsonora)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macrbv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_AWACS_ADD("awacs", 44100)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD("ncr5380", NCR5380, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR5380_IRQ_CB(WRITELINE(mac_state, mac_scsi_irq))

	MCFG_IWM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_cdadb))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, 783360)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state, mac_via2_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state, mac_via2_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via2_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via2_out_b))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via2_irq))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8M")
	MCFG_RAM_EXTRA_OPTIONS("16M,32M,64M,128M")

	MCFG_CUDA_ADD(CUDA_341S0060)
	MCFG_CUDA_RESET_CALLBACK(WRITELINE(mac_state, cuda_reset_w))
	MCFG_CUDA_LINECHANGE_CALLBACK(WRITELINE(mac_state, adb_linechange_w))
	MCFG_CUDA_VIA_CLOCK_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_CUDA_VIA_DATA_CALLBACK(DEVWRITELINE("via6522_0", via6522_device, write_cb2))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( macqd700, mac_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68040, 25000000)
	MCFG_CPU_PROGRAM_MAP(quadra700_map)

	MCFG_SCREEN_ADD(MAC_SCREEN_NAME, RASTER)
	MCFG_SCREEN_REFRESH_RATE(75.08)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1260))
	MCFG_SCREEN_SIZE(1152, 870)
	MCFG_SCREEN_VISIBLE_AREA(0, 1152-1, 0, 870-1)
	MCFG_SCREEN_UPDATE_DRIVER(mac_state, screen_update_macdafb)

	MCFG_VIDEO_START_OVERRIDE(mac_state,macdafb)
	MCFG_VIDEO_RESET_OVERRIDE(mac_state,macdafb)

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ASC_ADD("asc", C15M, ASC_TYPE_EASC, WRITELINE(mac_state, mac_asc_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MCFG_RTC3430042_ADD("rtc", XTAL_32_768kHz)
	MCFG_DEVICE_ADD("nubus", NUBUS, 0)
	MCFG_NUBUS_CPU("maincpu")
	MCFG_NUBUS_OUT_IRQ9_CB(WRITELINE(mac_state, nubus_irq_9_w))
	MCFG_NUBUS_OUT_IRQA_CB(WRITELINE(mac_state, nubus_irq_a_w))
	MCFG_NUBUS_OUT_IRQB_CB(WRITELINE(mac_state, nubus_irq_b_w))
	MCFG_NUBUS_OUT_IRQC_CB(WRITELINE(mac_state, nubus_irq_c_w))
	MCFG_NUBUS_OUT_IRQD_CB(WRITELINE(mac_state, nubus_irq_d_w))
	MCFG_NUBUS_OUT_IRQE_CB(WRITELINE(mac_state, nubus_irq_e_w))
	MCFG_NUBUS_SLOT_ADD("nubus","nbd", mac_nubus_cards, NULL)
	MCFG_NUBUS_SLOT_ADD("nubus","nbe", mac_nubus_cards, NULL)

	MCFG_IWM_ADD("fdc", mac_iwm_interface)
	MCFG_LEGACY_FLOPPY_SONY_2_DRIVES_ADD(mac_floppy_interface)

	MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(mac_state, set_scc_interrupt))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, C7M/10)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state,mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state,mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via_out_b_bbadb))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(mac_state,mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via_irq))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, C7M/10)
	MCFG_VIA6522_READPA_HANDLER(READ8(mac_state, mac_via2_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(mac_state, mac_via2_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(mac_state,mac_via2_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(mac_state,mac_via2_out_b))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(mac_state,mac_via2_irq))

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD(MAC_539X_1_TAG, NCR539X, C7M)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_NCR539X_OUT_IRQ_CB(WRITELINE(mac_state, irq_539x_1_w))
	MCFG_NCR539X_OUT_DRQ_CB(WRITELINE(mac_state, drq_539x_1_w))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("8M,16M,32M,64M,68M,72M,80M,96M,128M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop35_list","mac_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "mac_hdd")
MACHINE_CONFIG_END

static INPUT_PORTS_START( macplus )
	PORT_START("MOUSE0") /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START("MOUSE1") /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE2") /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	/* R Nabet 000531 : pseudo-input ports with keyboard layout */
	/* we only define US layout for keyboard - international layout is different! */
	/* note : 16 bits at most per port! */

	/* main keyboard pad */

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
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)    /* escape: */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)    /* ??? */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Command") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Option") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNUSED)    /* Control: */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)    /* keypad pseudo-keycode */
	PORT_BIT(0xE000, IP_ACTIVE_HIGH, IPT_UNUSED)    /* ??? */

	/* keypad */
	PORT_START("KEY4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)          PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x0038, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)          PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Clear") PORT_CODE(/*KEYCODE_NUMLOCK*/KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad =") PORT_CODE(/*CODE_OTHER*/KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x0E00, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)         PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)             PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)             PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0xE000, IP_ACTIVE_HIGH, IPT_UNUSED)

	/* Arrow keys */
	PORT_START("KEY6")
	PORT_BIT(0x0003, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Arrow") PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x0038, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Arrow") PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down Arrow") PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x1E00, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up Arrow") PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0xC000, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

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
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Clear") PORT_CODE(/*KEYCODE_NUMLOCK*/KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))    // 0x47
	PORT_BIT(0x0700, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x48, 49, 4a
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)         PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // 0x4b
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) // 0x4c
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x4d
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // 0x4e
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x4f

	PORT_START("KEY5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)    // 0x50
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad =") PORT_CODE(/*CODE_OTHER*/KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) // 0x51
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

/*
ROM_START( mactw )
    ROM_REGION16_BE(0x20000, "bootrom", 0)
    ROM_LOAD( "rom4.3t_07-04-83.bin", 0x0000, 0x10000, CRC(d2c42f18) SHA1(f868c09ca70383a69751c37a5a3110a9597462a4) )
ROM_END
*/

ROM_START( mac128k )
	ROM_REGION16_BE(0x20000, "bootrom", 0)
	ROM_LOAD16_WORD( "mac128k.rom",  0x00000, 0x10000, CRC(6d0c8a28) SHA1(9d86c883aa09f7ef5f086d9e32330ef85f1bc93b) )
ROM_END

ROM_START( mac512k )
	ROM_REGION16_BE(0x20000, "bootrom", 0)
	ROM_LOAD16_WORD( "mac512k.rom",  0x00000, 0x10000, CRC(cf759e0d) SHA1(5b1ced181b74cecd3834c49c2a4aa1d7ffe944d7) )
ROM_END

ROM_START( unitron )
	ROM_REGION16_BE(0x20000, "bootrom", 0)
	ROM_LOAD16_WORD( "unitron_512.rom", 0x00000, 0x10000, CRC(1eabd37f) SHA1(a3d3696c08feac6805effb7ee07b68c2bf1a8dd7) )
ROM_END

ROM_START( mac512ke )
	ROM_REGION16_BE(0x20000, "bootrom", 0)
	ROM_LOAD16_WORD( "macplus.rom",  0x00000, 0x20000, CRC(b2102e8e) SHA1(7d2f808a045aa3a1b242764f0e2c7d13e288bf1f))
ROM_END


ROM_START( macplus )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "v3", "Loud Harmonicas")
	ROMX_LOAD( "macplus.rom",  0x00000, 0x20000, CRC(b2102e8e) SHA1(7d2f808a045aa3a1b242764f0e2c7d13e288bf1f), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "v2", "Lonely Heifers")
	ROMX_LOAD( "23512-1007__342-0342-a.rom-lo.u7d", 0x000000, 0x010000, CRC(5aaa4a2f) SHA1(5dfbfbe279ddadfae691c95f552fd9db41e3ed90), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "23512-1010__342-0341-b.rom-hi.u6d", 0x000001, 0x010000, CRC(65341487) SHA1(bf43fa4f5a3dcbbac20f1fe1deedee0895454379), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "v1", "Lonely Hearts")
	ROMX_LOAD( "4d1eeee1 - macplus v1.rom", 0x000000, 0x020000, CRC(4fa5b399) SHA1(e0da7165b92dee90d8b1522429c033729fa73fd2), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "romdisk", "mac68k.info self-boot (1/1/2015)")
	ROMX_LOAD( "modplus-harp2.bin", 0x000000, 0x028000, CRC(ba56078d) SHA1(debdf328ac73e1662d274a044d8750224f47edef), ROM_GROUPWORD | ROM_BIOS(4) )
ROM_END


ROM_START( macse )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD16_WORD( "macse.rom",  0x00000, 0x40000, CRC(0f7ff80c) SHA1(58532b7d0d49659fd5228ac334a1b094f0241968))
ROM_END

ROM_START( macsefd )
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "be06e171.rom", 0x000000, 0x040000, CRC(f530cb10) SHA1(d3670a90273d12e53d86d1228c068cb660b8c9d1) )
ROM_END

ROM_START( macclasc )
	ROM_REGION16_BE(0x80000, "bootrom", 0)
	ROM_LOAD( "a49f9914.rom", 0x000000, 0x080000, CRC(510d7d38) SHA1(ccd10904ddc0fb6a1d216b2e9effd5ec6cf5a83d) )
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
	ROMX_LOAD( "9779d2c4.rom", 0x000000, 0x040000, CRC(4df6d054) SHA1(db6b504744281369794e26ba71a6e385cf6227fa), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "original", "rev. A")
	ROMX_LOAD( "97851db6.rom", 0x000000, 0x040000, CRC(8c8b9d03) SHA1(5c264fe976f1e8495d364947c932a5e8309b4300), ROM_BIOS(2) )
ROM_END

ROM_START( maciihmu )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "default", "rev. B")
	ROMX_LOAD( "9779d2c4.rom", 0x000000, 0x040000, CRC(4df6d054) SHA1(db6b504744281369794e26ba71a6e385cf6227fa), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "original", "rev. A")
	ROMX_LOAD( "97851db6.rom", 0x000000, 0x040000, CRC(8c8b9d03) SHA1(5c264fe976f1e8495d364947c932a5e8309b4300), ROM_BIOS(2) )
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
		ROM_LOAD( "368cadfe.rom", 0x000000, 0x080000, CRC(46adbf74) SHA1(b54f9d2ed16b63c49ed55adbe4685ebe73eb6e80) )
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
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "3193670e.rom", 0x000000, 0x080000, CRC(96d2e1fd) SHA1(50df69c1b6e805e12a405dc610bc2a1471b2eac2) )
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

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT     COMPANY          FULLNAME */
//COMP( 1983, mactw,    0,        0,  mac128k,  macplus, mac_state,  mac128k512k,  "Apple Computer", "Macintosh (4.3T Prototype)",  MACHINE_NOT_WORKING )
COMP( 1984, mac128k,  0,        0,  mac128k,  macplus, mac_state,  mac128k512k,  "Apple Computer", "Macintosh 128k",  MACHINE_NOT_WORKING )
COMP( 1984, mac512k,  mac128k,  0,  mac512ke, macplus, mac_state,  mac128k512k,  "Apple Computer", "Macintosh 512k",  MACHINE_NOT_WORKING )
COMP( 1986, mac512ke, macplus,  0,  mac512ke, macplus, mac_state,  mac512ke,      "Apple Computer", "Macintosh 512ke", 0 )
COMP( 1985, unitron,  macplus,  0,  mac512ke, macplus, mac_state,  mac512ke,     "bootleg (Unitron)", "Mac 512",  MACHINE_NOT_WORKING )
COMP( 1986, macplus,  0,        0,  macplus,  macplus, mac_state,  macplus,   "Apple Computer", "Macintosh Plus",  0 )
COMP( 1987, macse,    0,        0,  macse,    macadb, mac_state,   macse,         "Apple Computer", "Macintosh SE",  0 )
COMP( 1987, macsefd,  0,        0,  macse,    macadb, mac_state,   macse,         "Apple Computer", "Macintosh SE (FDHD)",  0 )
COMP( 1987, macii,    0,        0,  macii,    macadb, mac_state,   macii,         "Apple Computer", "Macintosh II",  0 )
COMP( 1987, maciihmu, macii,    0,  maciihmu, macadb, mac_state,   macii,         "Apple Computer", "Macintosh II (w/o 68851 MMU)", 0 )
COMP( 1988, mac2fdhd, 0,        0,  macii,    macadb, mac_state,   maciifdhd,     "Apple Computer", "Macintosh II (FDHD)",  0 )
COMP( 1988, maciix,   mac2fdhd, 0,  maciix,   macadb, mac_state,   maciix,        "Apple Computer", "Macintosh IIx",  0 )
COMP( 1989, macprtb,  0,        0,  macprtb,  macadb, mac_state,   macprtb,   "Apple Computer", "Macintosh Portable", MACHINE_NOT_WORKING )
COMP( 1989, macse30,  mac2fdhd, 0,  macse30,  macadb, mac_state,   macse30,   "Apple Computer", "Macintosh SE/30",  0 )
COMP( 1989, maciicx,  mac2fdhd, 0,  maciicx,  macadb, mac_state,   maciicx,   "Apple Computer", "Macintosh IIcx",  0 )
COMP( 1989, maciici,  0,        0,  maciici,  maciici, mac_state,  maciici,   "Apple Computer", "Macintosh IIci", 0 )
COMP( 1990, maciifx,  0,        0,  maciifx,  macadb, mac_state,   maciifx,      "Apple Computer", "Macintosh IIfx",  MACHINE_NOT_WORKING )
COMP( 1990, macclasc, 0,        0,  macclasc, macadb, mac_state,   macclassic,    "Apple Computer", "Macintosh Classic",  0 )
COMP( 1990, maclc,    0,        0,  maclc,    maciici, mac_state,  maclc,         "Apple Computer", "Macintosh LC", MACHINE_IMPERFECT_SOUND )
COMP( 1990, maciisi,  0,        0,  maciisi,  maciici, mac_state,  maciisi,   "Apple Computer", "Macintosh IIsi", 0 )
COMP( 1991, macpb100, 0,        0,  macprtb,  macadb, mac_state,   macprtb,   "Apple Computer", "Macintosh PowerBook 100", MACHINE_NOT_WORKING )
COMP( 1991, macpb140, 0,        0,  macpb140, macadb, mac_state,   macpb140,      "Apple Computer", "Macintosh PowerBook 140", MACHINE_NOT_WORKING )
COMP( 1991, macpb170, macpb140, 0,  macpb170, macadb, mac_state,   macpb140,      "Apple Computer", "Macintosh PowerBook 170", MACHINE_NOT_WORKING )
COMP( 1991, macqd700, macpb140, 0,  macqd700, macadb, mac_state,   macquadra700, "Apple Computer", "Macintosh Quadra 700", MACHINE_NOT_WORKING )
COMP( 1991, macclas2, 0,        0,  macclas2, macadb, mac_state,   macclassic2,  "Apple Computer", "Macintosh Classic II", MACHINE_IMPERFECT_SOUND )
COMP( 1991, maclc2,   0,        0,  maclc2,   maciici, mac_state,  maclc2,        "Apple Computer", "Macintosh LC II",  MACHINE_IMPERFECT_SOUND )
COMP( 1992, macpb145, macpb140, 0,  macpb145, macadb, mac_state,   macpb140,      "Apple Computer", "Macintosh PowerBook 145", MACHINE_NOT_WORKING )
COMP( 1992, macpb160, 0,        0,  macpb160, macadb, mac_state,   macpb160,      "Apple Computer", "Macintosh PowerBook 160", MACHINE_NOT_WORKING )
COMP( 1992, macpb180, macpb160, 0,  macpb180, macadb, mac_state,   macpb160,      "Apple Computer", "Macintosh PowerBook 180", MACHINE_NOT_WORKING )
COMP( 1992, macpb180c,macpb160, 0,  macpb180c,macadb, mac_state,   macpb160,     "Apple Computer", "Macintosh PowerBook 180c", MACHINE_NOT_WORKING )
COMP( 1992, macpd210, 0,        0,  macpd210, macadb, mac_state,   macpd210,     "Apple Computer", "Macintosh PowerBook Duo 210", MACHINE_NOT_WORKING )
COMP( 1993, maccclas, 0,        0,  maccclas, macadb, mac_state,   maclrcclassic,"Apple Computer", "Macintosh Color Classic", MACHINE_NOT_WORKING )
COMP( 1992, macpb145b,macpb140, 0,  macpb170, macadb, mac_state,   macpb140,      "Apple Computer", "Macintosh PowerBook 145B", MACHINE_NOT_WORKING )
COMP( 1993, maclc3,   0,        0,  maclc3,   maciici, mac_state,  maclc3,        "Apple Computer", "Macintosh LC III",  MACHINE_IMPERFECT_SOUND )
COMP( 1993, maciivx,  0,        0,  maciivx,  maciici, mac_state,  maciivx,   "Apple Computer", "Macintosh IIvx", MACHINE_IMPERFECT_SOUND )
COMP( 1993, maciivi,  maciivx,  0,  maciivi,  maciici, mac_state,  maciivx,   "Apple Computer", "Macintosh IIvi", MACHINE_IMPERFECT_SOUND )
COMP( 1993, maclc520, 0,        0,  maclc520, maciici, mac_state,  maclc520,     "Apple Computer", "Macintosh LC 520",  MACHINE_NOT_WORKING )
COMP( 1994, pmac6100, 0,        0,  pwrmac,   macadb, mac_state,   macpm6100,     "Apple Computer", "Power Macintosh 6100/60",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
