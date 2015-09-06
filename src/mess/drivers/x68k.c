// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl

// Preliminary X68000 driver for MESS
// Started 18/11/2006
// Written by Barry Rodewald

/*
    *** Basic memory map

    0x000000 - 0xbfffff     RAM (Max 12MB), vector table from ROM at 0xff0000 maps to 0x000000 at reset only
    0xc00000 - 0xdfffff     Graphic VRAM
    0xe00000 - 0xe1ffff     Text VRAM Plane 1
    0xe20000 - 0xe3ffff     Text VRAM Plane 2
    0xe40000 - 0xe5ffff     Text VRAM Plane 3
    0xe60000 - 0xe7ffff     Text VRAM Plane 4
    0xe80000                CRTC
    0xe82000                Video Controller
    0xe84000                DMA Controller
    0xe86000                Supervisor Area set
    0xe88000                MFP
    0xe8a000                RTC
    0xe8c000                Printer
    0xe8e000                System Port (?)
    0xe90000                FM Sound source
    0xe92000                ADPCM
    0xe94000                FDC
    0xe96000                HDC
    0xe96021                SCSI (internal model)
    0xe98000                SCC
    0xe9a000                Serial I/O (PPI)
    0xe9c000                I/O controller

    [Expansions]
    0xe9c000 / 0xe9e000     FPU (Optional, X68000 only)
    0xea0000                SCSI
    0xeaf900                FAX
    0xeafa00 / 0xeafa10     MIDI (1st/2nd)
    0xeafb00                Serial
    0xeafc00/10/20/30       EIA232E
    0xeafd00                EIA232E
    0xeafe00                GPIB (?)
    0xec0000 - 0xecffff     User I/O Expansion

    0xeb0000 - 0xeb7fff     Sprite registers
    0xeb8000 - 0xebffff     Sprite VRAM
    0xed0000 - 0xed3fff     SRAM
    0xf00000 - 0xfb0000     ROM  (CGROM.DAT)
    0xfe0000 - 0xffffff     ROM  (IPLROM.DAT)


    *** System hardware

    CPU : X68000: 68000 at 10MHz
          X68000 XVI: 68000 at 16MHz
          X68030: 68EC030 at 25MHz

    RAM : between 1MB and 4MB stock, expandable to 12MB

    FDD : 2x 5.25", Compact models use 2x 3.5" drives.
    FDC : NEC uPD72065

    HDD : HD models have up to an 81MB HDD.
    HDC : Fujitsu MB89352A (SCSI)

    SCC : Serial controller - Zilog z85C30  (Dual channel, 1 for RS232, 1 for mouse)
    PPI : Parallel controller  - NEC 8255   (Printer, Joystick)

    Sound : FM    - YM2151, with YM3012 DAC
            ADPCM - Okidata MSM6258

    DMA : Hitachi HD63450, DMA I/O for FDD, HDD, Expansion slots, and ADPCM

    MFP : Motorola MC68901 - monitor sync, serial port, RTC, soft power, FM synth, IRQs, keyboard

    RTC : Ricoh RP5C15

    ...plus a number of custom chips for video and other stuff...


    *** Current status (28/12/08)
    MFP : Largely works, as far as the X68000 goes.

    PPI : Joystick controls work okay.

    HDC/HDD : SCSI is not implemented, not a requirement at this point.

    RTC : Seems to work. (Tested using SX-Window's Timer application)

    DMA : Works fine.

    Sound : FM works, ADPCM mostly works (timing(?) issues in a few games).

    SCC : Works enough to get the mouse running, although only with the IPL v1.0 BIOS

    Video : Text mode works, but is rather slow, especially scrolling up (uses multple "raster copy" commands).
            Graphic layers work.
            BG tiles and sprites work, but many games have the sprites offset by a small amount (some by a lot :))
            Still a few minor priority issues around.

    Other issues:
      Bus error exceptions are a bit late at times.  Currently using a fake bus error for MIDI expansion checks.  These
      are used determine if a piece of expansion hardware is present.
      Keyboard doesn't work properly (MFP USART).
      Supervisor area set isn't implemented.

    Some minor game-specific issues:
      Salamander:    System error when using keys in-game.  No error if a joystick is used.
                     Some text is drawn incorrectly.
      Dragon Buster: Text is black and unreadable. (Text layer actually covers it)
      Tetris:        Black dots over screen (text layer).
      Parodius Da!:  Black squares in areas.

    More detailed documentation at http://x68kdev.emuvibes.com/iomap.html - if you can stand broken english :)

*/

#include "emu.h"
#include "machine/mc68901.h"
#include "machine/upd765.h"
#include "sound/okim6258.h"
#include "machine/rp5c15.h"
#include "machine/mb89352.h"
#include "formats/xdf_dsk.h"
#include "formats/dim_dsk.h"
#include "machine/x68k_hdc.h"
#include "machine/x68k_kbd.h"
#include "includes/x68k.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "bus/x68k/x68kexp.h"
#include "bus/x68k/x68k_neptunex.h"
#include "bus/x68k/x68k_scsiext.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "x68000.lh"



void x68k_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_X68K_LED:
		x68k_led_callback(ptr, param);
		break;
	case TIMER_X68K_SCC_ACK:
		x68k_scc_ack(ptr, param);
		break;
	case TIMER_MD_6BUTTON_PORT1_TIMEOUT:
		md_6button_port1_timeout(ptr, param);
		break;
	case TIMER_MD_6BUTTON_PORT2_TIMEOUT:
		md_6button_port2_timeout(ptr, param);
		break;
	case TIMER_X68K_BUS_ERROR:
		x68k_bus_error(ptr, param);
		break;
	case TIMER_X68K_NET_IRQ:
		x68k_net_irq(ptr, param);
		break;
	case TIMER_X68K_CRTC_OPERATION_END:
		x68k_crtc_operation_end(ptr, param);
		break;
	case TIMER_X68K_HSYNC:
		x68k_hsync(ptr, param);
		break;
	case TIMER_X68K_CRTC_RASTER_END:
		x68k_crtc_raster_end(ptr, param);
		break;
	case TIMER_X68K_CRTC_RASTER_IRQ:
		x68k_crtc_raster_irq(ptr, param);
		break;
	case TIMER_X68K_CRTC_VBLANK_IRQ:
		x68k_crtc_vblank_irq(ptr, param);
		break;
	case TIMER_X68K_FDC_TC:
		m_upd72065->tc_w(ASSERT_LINE);
		m_upd72065->tc_w(CLEAR_LINE);
		break;
	case TIMER_X68K_ADPCM:
		m_hd63450->drq3_w(1);
		m_hd63450->drq3_w(0);
		break;
	default:
		assert_always(FALSE, "Unknown id in x68k_state::device_timer");
	}
}

// LED timer callback
TIMER_CALLBACK_MEMBER(x68k_state::x68k_led_callback)
{
	int drive;
	if(m_led_state == 0)
		m_led_state = 1;
	else
		m_led_state = 0;
	if(m_led_state == 1)
	{
		for(drive=0;drive<4;drive++)
			output_set_indexed_value("ctrl_drv",drive,m_fdc.led_ctrl[drive] ? 0 : 1);
	}
	else
	{
		for(drive=0;drive<4;drive++)
			output_set_indexed_value("ctrl_drv",drive,1);
	}

}


// mouse input
// port B of the Z8530 SCC
// typically read from the SCC data port on receive buffer full interrupt per byte
int x68k_state::x68k_read_mouse()
{
	char val = 0;
	char ipt = 0;

	if(!(m_scc->get_reg_b(5) & 0x02))
		return 0xff;

	switch(m_mouse.inputtype)
	{
	case 0:
		ipt = m_mouse1->read();
		break;
	case 1:
		val = m_mouse2->read();
		ipt = val - m_mouse.last_mouse_x;
		m_mouse.last_mouse_x = val;
		break;
	case 2:
		val = m_mouse3->read();
		ipt = val - m_mouse.last_mouse_y;
		m_mouse.last_mouse_y = val;
		break;
	}
	m_mouse.inputtype++;
	if(m_mouse.inputtype > 2)
	{
		int i_val = m_scc->get_reg_b(0);
		m_mouse.inputtype = 0;
		m_mouse.bufferempty = 1;
		i_val &= ~0x01;
		m_scc->set_reg_b(0, i_val);
		logerror("SCC: mouse buffer empty\n");
	}

	return ipt;
}

/*
    0xe98001 - Z8530 command port B
    0xe98003 - Z8530 data port B  (mouse input)
    0xe98005 - Z8530 command port A
    0xe98007 - Z8530 data port A  (RS232)
*/
READ16_MEMBER(x68k_state::x68k_scc_r )
{
	offset %= 4;
	switch(offset)
	{
	case 0:
		return m_scc->reg_r(space, 0);
	case 1:
		return x68k_read_mouse();
	case 2:
		return m_scc->reg_r(space, 1);
	case 3:
		return m_scc->reg_r(space, 3);
	default:
		return 0xff;
	}
}

WRITE16_MEMBER(x68k_state::x68k_scc_w )
{
	offset %= 4;

	switch(offset)
	{
	case 0:
		m_scc->reg_w(space, 0,(UINT8)data);
		if((m_scc->get_reg_b(5) & 0x02) != m_scc_prev)
		{
			if(m_scc->get_reg_b(5) & 0x02)  // Request to Send
			{
				int val = m_scc->get_reg_b(0);
				m_mouse.bufferempty = 0;
				val |= 0x01;
				m_scc->set_reg_b(0,val);
			}
		}
		break;
	case 1:
		m_scc->reg_w(space, 2,(UINT8)data);
		break;
	case 2:
		m_scc->reg_w(space, 1,(UINT8)data);
		break;
	case 3:
		m_scc->reg_w(space, 3,(UINT8)data);
		break;
	}
	m_scc_prev = m_scc->get_reg_b(5) & 0x02;
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_scc_ack)
{
	if(m_mouse.bufferempty != 0)  // nothing to do if the mouse data buffer is empty
		return;

//  if((m_ioc.irqstatus & 0xc0) != 0)
//      return;

	// hard-code the IRQ vector for now, until the SCC code is more complete
	if((m_scc->get_reg_a(9) & 0x08) || (m_scc->get_reg_b(9) & 0x08))  // SCC reg WR9 is the same for both channels
	{
		if((m_scc->get_reg_b(1) & 0x18) != 0)  // if bits 3 and 4 of WR1 are 0, then Rx IRQs are disabled on this channel
		{
			if(m_scc->get_reg_b(5) & 0x02)  // RTS signal
			{
				m_mouse.irqactive = 1;
				m_current_vector[5] = 0x54;
				m_current_irq_line = 5;
				m_maincpu->set_input_line_and_vector(5,ASSERT_LINE,0x54);
			}
		}
	}
}

void x68k_state::x68k_set_adpcm()
{
	UINT32 rate = 0;

	switch(m_adpcm.rate & 0x0c)
	{
		case 0x00:
			rate = 7812/2;
			break;
		case 0x04:
			rate = 10417/2;
			break;
		case 0x08:
			rate = 15625/2;
			break;
		default:
			logerror("PPI: Invalid ADPCM sample rate set.\n");
			rate = 15625/2;
	}
	if(m_adpcm.clock != 0)
		rate = rate/2;
	m_adpcm_timer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
}

// Megadrive 3 button gamepad
// According to XM6, bits 4 and 7 are always 1 (is this correct?)
// Bits 4 and 5 of PPI port C control each controller's multiplexer
// Button inputs (Start, A, B and C) are read in bits 5 and 6 (rather than 4
// and 5 like on a Megadrive)

UINT8 x68k_state::md_3button_r(int port)
{
	if(port == 1)
	{
		UINT8 porta = m_md3b->read() & 0xff;
		UINT8 portb = (m_md3b->read() >> 8) & 0xff;
		if(m_mdctrl.mux1 & 0x10)
		{
			return porta | 0x90;
		}
		else
		{
			return (portb & 0x60) | (porta & 0x03) | 0x90;
		}
	}
	if(port == 2)
	{
		UINT8 porta = (m_md3b->read() >> 16) & 0xff;
		UINT8 portb = (m_md3b->read() >> 24) & 0xff;
		if(m_mdctrl.mux2 & 0x20)
		{
			return porta | 0x90;
		}
		else
		{
			return (portb & 0x60) | (porta & 0x03) | 0x90;
		}
	}
	return 0xff;
}

// Megadrive 6 button gamepad
TIMER_CALLBACK_MEMBER(x68k_state::md_6button_port1_timeout)
{
	m_mdctrl.seq1 = 0;
}

TIMER_CALLBACK_MEMBER(x68k_state::md_6button_port2_timeout)
{
	m_mdctrl.seq2 = 0;
}

void x68k_state::md_6button_init()
{
	m_mdctrl.io_timeout1 = timer_alloc(TIMER_MD_6BUTTON_PORT1_TIMEOUT);
	m_mdctrl.io_timeout2 = timer_alloc(TIMER_MD_6BUTTON_PORT2_TIMEOUT);
}

UINT8 x68k_state::md_6button_r(int port)
{
	if(port == 1)
	{
		UINT8 porta = m_md6b->read() & 0xff;
		UINT8 portb = (m_md6b->read() >> 8) & 0xff;
		UINT8 extra = m_md6b_extra->read() & 0x0f;

		switch(m_mdctrl.seq1)
		{
			case 1:
			default:
				if(m_mdctrl.mux1 & 0x10)
				{
					return porta | 0x90;
				}
				else
				{
					return (portb & 0x60) | (porta & 0x03) | 0x90;
				}
			case 2:
				if(m_mdctrl.mux1 & 0x10)
				{
					return porta | 0x90;
				}
				else
				{
					return (portb & 0x60) | 0x90;
				}
			case 3:
				if(m_mdctrl.mux1 & 0x10)
				{
					return (porta & 0x60) | (extra & 0x0f) | 0x90;
				}
				else
				{
					return (portb & 0x60) | 0x9f;
				}
		}
	}
	if(port == 2)
	{
		UINT8 porta = (m_md6b->read() >> 16) & 0xff;
		UINT8 portb = (m_md6b->read() >> 24) & 0xff;
		UINT8 extra = (m_md6b_extra->read() >> 4) & 0x0f;

		switch(m_mdctrl.seq2)
		{
			case 1:
			default:
				if(m_mdctrl.mux2 & 0x20)
				{
					return porta | 0x90;
				}
				else
				{
					return (portb & 0x60) | (porta & 0x03) | 0x90;
				}
			case 2:
				if(m_mdctrl.mux2 & 0x20)
				{
					return porta | 0x90;
				}
				else
				{
					return (portb & 0x60) | 0x90;
				}
			case 3:
				if(m_mdctrl.mux2 & 0x20)
				{
					return (porta & 0x60) | (extra & 0x0f) | 0x90;
				}
				else
				{
					return (portb & 0x60) | 0x9f;
				}
		}
	}
	return 0xff;
}

// XPD-1LR dual D-pad controller.
// Sold with Video Game Anthology Vol 4: Libble Rabble.
// Also compatible with Video Game Anthology Vol 5: Crazy Climber 1 & 2
// Uses the same input multiplexer hardware as Megadrive controllers
// Output is the same as for standard controllers, but when ctl is high,
// the directions refer to the right D-pad, and when low, the left D-pad
// The buttons are read the same as normal, regardless of ctl.
UINT8 x68k_state::xpd1lr_r(int port)
{
	if(port == 1)
	{
		UINT8 porta = m_xpd1lr->read() & 0xff;
		UINT8 portb = (m_xpd1lr->read() >> 8) & 0xff;
		if(m_mdctrl.mux1 & 0x10)
		{
			return porta;
		}
		else
		{
			return portb | (porta & 0x60);
		}
	}
	if(port == 2)
	{
		UINT8 porta = (m_xpd1lr->read() >> 16) & 0xff;
		UINT8 portb = (m_xpd1lr->read() >> 24) & 0xff;
		if(m_mdctrl.mux2 & 0x20)
		{
			return porta;
		}
		else
		{
			return portb | (porta & 0x60);
		}
	}
	return 0xff;
}

// Judging from the XM6 source code, PPI ports A and B are joystick inputs
READ8_MEMBER(x68k_state::ppi_port_a_r)
{
	int ctrl = m_ctrltype->read() & 0x0f;

	switch(ctrl)
	{
		case 0x00:  // standard MSX/FM-Towns joystick
			if(m_joy.joy1_enable == 0)
				return m_joy1->read();
			else
				return 0xff;
		case 0x01:  // 3-button Megadrive gamepad
			return md_3button_r(1);
		case 0x02:  // 6-button Megadrive gamepad
			return md_6button_r(1);
		case 0x03:  // XPD-1LR
			return xpd1lr_r(1);
	}

	return 0xff;
}

READ8_MEMBER(x68k_state::ppi_port_b_r)
{
	int ctrl = m_ctrltype->read() & 0xf0;

	switch(ctrl)
	{
		case 0x00:  // standard MSX/FM-Towns joystick
			if(m_joy.joy2_enable == 0)
				return m_joy2->read();
			else
				return 0xff;
		case 0x10:  // 3-button Megadrive gamepad
			return md_3button_r(2);
		case 0x20:  // 6-button Megadrive gamepad
			return md_6button_r(2);
		case 0x30:  // XPD-1LR
			return xpd1lr_r(2);
	}

	return 0xff;
}

READ8_MEMBER(x68k_state::ppi_port_c_r)
{
	return m_ppi_port[2];
}

/* PPI port C (Joystick control, R/W)
   bit 7    - IOC7 - Function B operation of joystick 1 (?)
   bit 6    - IOC6 - Function A operation of joystick 1 (?)
   bit 5    - IOC5 - Enable Joystick 2
   bit 4    - IOC4 - Enable Joystick 1
   bits 3,2 - ADPCM Sample rate
   bits 1,0 - ADPCM Pan
*/
WRITE8_MEMBER(x68k_state::ppi_port_c_w)
{
	// ADPCM / Joystick control
	m_ppi_port[2] = data;
	if((data & 0x0f) != (m_ppi_prev & 0x0f))
	{
		m_adpcm.pan = data & 0x03;
		m_adpcm.rate = data & 0x0c;
		x68k_set_adpcm();
		m_okim6258->set_divider((data >> 2) & 3);
	}

	// The joystick enable bits also handle the multiplexer for various controllers
	m_joy.joy1_enable = data & 0x10;
	m_mdctrl.mux1 = data & 0x10;
	if((m_ppi_prev & 0x10) == 0x00 && (data & 0x10) == 0x10)
	{
		m_mdctrl.seq1++;
		m_mdctrl.io_timeout1->adjust(m_maincpu->cycles_to_attotime(8192));
	}

	m_joy.joy2_enable = data & 0x20;
	m_mdctrl.mux2 = data & 0x20;
	if((m_ppi_prev & 0x20) == 0x00 && (data & 0x20) == 0x20)
	{
		m_mdctrl.seq2++;
		m_mdctrl.io_timeout2->adjust(m_maincpu->cycles_to_attotime(8192));
	}
	m_ppi_prev = data;

	m_joy.ioc6 = data & 0x40;
	m_joy.ioc7 = data & 0x80;
}


// NEC uPD72065 at 0xe94000
WRITE16_MEMBER(x68k_state::x68k_fdc_w)
{
	unsigned int drive, x;
	switch(offset)
	{
	case 0x00:  // drive option signal control
		x = data & 0x0f;
		for(drive=0;drive<4;drive++)
		{
			if(m_fdc.control_drives & (1 << drive))
			{
				if(!(x & (1 << drive)))  // functions take place on 1->0 transitions of drive bits only
				{
					m_fdc.led_ctrl[drive] = data & 0x80;  // blinking drive LED if no disk inserted
					m_fdc.led_eject[drive] = data & 0x40;  // eject button LED (on when set to 0)
					output_set_indexed_value("eject_drv",drive,(data & 0x40) ? 1 : 0);
					if((data & 0x60) == 0x20)  // ejects disk
						m_fdc.floppy[drive]->unload();
				}
			}
		}
		m_fdc.control_drives = data & 0x0f;
		logerror("FDC: signal control set to %02x\n",data);
		break;
	case 0x01: {
		x = data & 3;
		m_upd72065->set_floppy(m_fdc.floppy[x]);
		m_upd72065->set_rate((data & 0x10) ? 300000 : 500000);
		m_fdc.motor = data & 0x80;

		for(int i = 0; i < 4; i++)
			if(m_fdc.floppy[i]->exists())
				m_fdc.floppy[i]->mon_w(!BIT(data, 7));

		output_set_indexed_value("access_drv",x,0);
		if(x != m_fdc.select_drive)
			output_set_indexed_value("access_drv",m_fdc.select_drive,1);
		m_fdc.select_drive = x;
		logerror("FDC: Drive #%i: Drive selection set to %02x\n",x,data);
		break;
		}
	}
}

READ16_MEMBER(x68k_state::x68k_fdc_r)
{
	unsigned int ret;
	int x;

	switch(offset)
	{
	case 0x00:
		ret = 0x00;
		for(x=0;x<4;x++)
		{
			if(m_fdc.control_drives & (1 << x))
			{
				ret = 0x00;
				if(m_fdc.floppy[x]->exists())
				{
					ret |= 0x80;
				}
				// bit 7 = disk inserted
				// bit 6 = disk error (in insertion, presumably)
				logerror("FDC: Drive #%i Disk check - returning %02x\n",x,ret);
			}
		}
		return ret;
	case 0x01:
		logerror("FDC: IOC selection is write-only\n");
		return 0xff;
	}
	return 0xff;
}

WRITE_LINE_MEMBER( x68k_state::fdc_irq )
{
	if((m_ioc.irqstatus & 0x04) && state)
	{
		m_current_vector[1] = m_ioc.fdcvector;
		m_ioc.irqstatus |= 0x80;
		m_current_irq_line = 1;
		logerror("FDC: IRQ triggered\n");
		m_maincpu->set_input_line_and_vector(1, ASSERT_LINE, m_current_vector[1]);
	}
	else
		m_maincpu->set_input_line(1, CLEAR_LINE);
}

WRITE8_MEMBER(x68k_state::x68k_ct_w)
{
	// CT1 and CT2 bits from YM2151 port 0x1b
	// CT1 - ADPCM clock - 0 = 8MHz, 1 = 4MHz
	// CT2 - 1 = Set ready state of FDC
	if(data & 1)
	{
		m_upd72065->set_ready_line_connected(0);
		m_upd72065->ready_w(0);
	}
	else
		m_upd72065->set_ready_line_connected(1);

	m_adpcm.clock = data & 0x02;
	x68k_set_adpcm();
	m_okim6258->set_clock(data & 0x02 ? 4000000 : 8000000);
}

/*
 Custom I/O controller at 0xe9c000
 0xe9c001 (R) - Interrupt status
 0xe9c001 (W) - Interrupt mask (low nibble only)
                - bit 7 = FDC interrupt
                - bit 6 = FDD interrupt
                - bit 5 = Printer Busy signal
                - bit 4 = HDD interrupt
                - bit 3 = HDD interrupts enabled
                - bit 2 = FDC interrupts enabled
                - bit 1 = FDD interrupts enabled
                - bit 0 = Printer interrupts enabled
 0xe9c003 (W) - Interrupt vector
                - bits 7-2 = vector
                - bits 1,0 = device (00 = FDC, 01 = FDD, 10 = HDD, 11 = Printer)
*/
WRITE16_MEMBER(x68k_state::x68k_ioc_w)
{
	switch(offset)
	{
	case 0x00:
		m_ioc.irqstatus = data & 0x0f;
		logerror("I/O: Status register write %02x\n",data);
		break;
	case 0x01:
		switch(data & 0x03)
		{
		case 0x00:
			m_ioc.fdcvector = data & 0xfc;
			logerror("IOC: FDC IRQ vector = 0x%02x\n",data & 0xfc);
			break;
		case 0x01:
			m_ioc.fddvector = data & 0xfc;
			logerror("IOC: FDD IRQ vector = 0x%02x\n",data & 0xfc);
			break;
		case 0x02:
			m_ioc.hdcvector = data & 0xfc;
			logerror("IOC: HDD IRQ vector = 0x%02x\n",data & 0xfc);
			break;
		case 0x03:
			m_ioc.prnvector = data & 0xfc;
			logerror("IOC: Printer IRQ vector = 0x%02x\n",data & 0xfc);
			break;
		}
		break;
	}
}

READ16_MEMBER(x68k_state::x68k_ioc_r)
{
	switch(offset)
	{
	case 0x00:
		logerror("I/O: Status register read\n");
		return (m_ioc.irqstatus & 0xdf) | 0x20;
	default:
		return 0x00;
	}
}

/*
 System ports at 0xe8e000
 Port 1 (0xe8e001) - Monitor contrast (bits 3-0)
 Port 2 (0xe8e003) - Display / 3D scope control
                     - bit 3 - Display control signal (0 = on)
                     - bit 1 - 3D scope left shutter (0 = closed)
                     - bit 0 - 3D scope right shutter
 Port 3 (0xe8e005) - Colour image unit control (bits 3-0)
 Port 4 (0xe8e007) - Keyboard/NMI/Dot clock
                     - bit 3 - (R) 1 = Keyboard connected, (W) 1 = Key data can be transmitted
                     - bit 1 - NMI Reset
                     - bit 0 - HRL - high resolution dot clock - 1 = 1/2, 1/4, 1/8, 0 = 1/2, 1/3, 1/6 (normal)
 Port 5 (0xe8e009) - ROM (bits 7-4)/DRAM (bits 3-0) wait, X68030 only
 Port 6 (0xe8e00b) - CPU type and clock speed (XVI or later only, X68000 returns 0xFF)
                     - bits 7-4 - CPU Type (1100 = 68040, 1101 = 68030, 1110 = 68020, 1111 = 68000)
                     - bits 3-0 - clock speed (1001 = 50MHz, 40, 33, 25, 20, 16, 1111 = 10MHz)
 Port 7 (0xe8e00d) - SRAM write enable - if 0x31 is written to this port, writing to SRAM is allowed.
                                         Any other value, then SRAM is read only.
 Port 8 (0xe8e00f) - Power off control - write 0x00, 0x0f, 0x0f sequentially to switch power off.
*/
WRITE16_MEMBER(x68k_state::x68k_sysport_w)
{
	switch(offset)
	{
	case 0x00:
		m_sysport.contrast = data & 0x0f;  // often used for screen fades / blanking
		m_screen->set_brightness(m_sysport.contrast * 0x11);
		break;
	case 0x01:
		m_sysport.monitor = data & 0x08;
		break;
	case 0x03:
		m_sysport.keyctrl = data & 0x08;  // bit 3 = enable keyboard data transmission
		break;
	case 0x06:
		COMBINE_DATA(&m_sysport.sram_writeprotect);
		break;
	default:
//      logerror("SYS: [%08x] Wrote %04x to invalid or unimplemented system port %04x\n",space.device().safe_pc(),data,offset);
		break;
	}
}

READ16_MEMBER(x68k_state::x68k_sysport_r)
{
	int ret = 0;
	switch(offset)
	{
	case 0x00:  // monitor contrast setting (bits3-0)
		return m_sysport.contrast;
	case 0x01:  // monitor control (bit3) / 3D Scope (bits1,0)
		ret |= m_sysport.monitor;
		return ret;
	case 0x03:  // bit 3 = key control (is 1 if keyboard is connected)
		return 0x08;
	case 0x05:  // CPU type and speed
		return m_sysport.cputype;
	default:
		logerror("Read from invalid or unimplemented system port %04x\n",offset);
		return 0xff;
	}
}

WRITE16_MEMBER(x68k_state::x68k_ppi_w)
{
	m_ppi->write(space,offset & 0x03,data);
}

READ16_MEMBER(x68k_state::x68k_ppi_r)
{
	return m_ppi->read(space,offset & 0x03);
}


WRITE16_MEMBER(x68k_state::x68k_sram_w)
{
	if(m_sysport.sram_writeprotect == 0x31)
	{
		COMBINE_DATA(&m_nvram[offset]);
	}
}

READ16_MEMBER(x68k_state::x68k_sram_r)
{
	// HACKS!
//  if(offset == 0x5a/2)  // 0x5a should be 0 if no SASI HDs are present.
//      return 0x0000;
	if(offset == 0x08/2)
		return m_ram->size() >> 16;  // RAM size
	return m_nvram[offset];
}

WRITE16_MEMBER(x68k_state::x68k_vid_w)
{
	switch(offset)
	{
	case 0x000:
		COMBINE_DATA(m_video.reg);
		break;
	case 0x080:  // priority levels
		COMBINE_DATA(m_video.reg+1);
		if(ACCESSING_BITS_0_7)
		{
			m_video.gfxlayer_pri[0] = data & 0x0003;
			m_video.gfxlayer_pri[1] = (data & 0x000c) >> 2;
			m_video.gfxlayer_pri[2] = (data & 0x0030) >> 4;
			m_video.gfxlayer_pri[3] = (data & 0x00c0) >> 6;
		}
		if(ACCESSING_BITS_8_15)
		{
			m_video.gfx_pri = (data & 0x0300) >> 8;
			m_video.text_pri = (data & 0x0c00) >> 10;
			m_video.sprite_pri = (data & 0x3000) >> 12;
			if(m_video.gfx_pri == 3)
				m_video.gfx_pri--;
			if(m_video.text_pri == 3)
				m_video.text_pri--;
			if(m_video.sprite_pri == 3)
				m_video.sprite_pri--;
		}
		break;
	case 0x100:
		COMBINE_DATA(m_video.reg+2);
		break;
	default:
		logerror("VC: Invalid video controller write (offset = 0x%04x, data = %04x)\n",offset,data);
	}
}

READ16_MEMBER(x68k_state::x68k_vid_r)
{
	switch(offset)
	{
	case 0x000:
		return m_video.reg[0];
	case 0x080:
		return m_video.reg[1];
	case 0x100:
		return m_video.reg[2];
	default:
		logerror("VC: Invalid video controller read (offset = 0x%04x)\n",offset);
	}

	return 0xff;
}

READ16_MEMBER(x68k_state::x68k_areaset_r)
{
	// register is write-only
	return 0xffff;
}

WRITE16_MEMBER(x68k_state::x68k_areaset_w)
{
	// TODO
	logerror("SYS: Supervisor area set: 0x%02x\n",data & 0xff);
}

WRITE16_MEMBER(x68k_state::x68k_enh_areaset_w )
{
	// TODO
	logerror("SYS: Enhanced Supervisor area set (from %iMB): 0x%02x\n",(offset + 1) * 2,data & 0xff);
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_bus_error)
{
	m_bus_error = false;
}

void x68k_state::set_bus_error(UINT32 address, bool write, UINT16 mem_mask)
{
	if(m_bus_error)
		return;
	if(!ACCESSING_BITS_8_15)
		address++;
	m_bus_error = true;
	m_maincpu->set_buserror_details(address, write, m_maincpu->get_fc());
	m_maincpu->mmu_tmp_buserror_address = address; // Hack for x68030
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	m_bus_error_timer->adjust(m_maincpu->cycles_to_attotime(16)); // let rmw cycles complete
	logerror("%s: Bus error: Unused RAM access [%08x]\n", machine().describe_context(), address);
}

READ16_MEMBER(x68k_state::x68k_rom0_r)
{
	/* this location contains the address of some expansion device ROM, if no ROM exists,
	   then access causes a bus error */
	if((m_options->read() & 0x02) && !space.debugger_access())
		set_bus_error((offset << 1) + 0xbffffc, 0, mem_mask);
	return 0xff;
}

WRITE16_MEMBER(x68k_state::x68k_rom0_w)
{
	/* this location contains the address of some expansion device ROM, if no ROM exists,
	   then access causes a bus error */
	if((m_options->read() & 0x02) && !space.debugger_access())
		set_bus_error((offset << 1) + 0xbffffc, 1, mem_mask);
}

READ16_MEMBER(x68k_state::x68k_emptyram_r)
{
	/* this location is unused RAM, access here causes a bus error
	   Often a method for detecting amount of installed RAM, is to read or write at 1MB intervals, until a bus error occurs */
	if((m_options->read() & 0x02) && !space.debugger_access())
		set_bus_error((offset << 1), 0, mem_mask);
	return 0xff;
}

WRITE16_MEMBER(x68k_state::x68k_emptyram_w)
{
	/* this location is unused RAM, access here causes a bus error
	   Often a method for detecting amount of installed RAM, is to read or write at 1MB intervals, until a bus error occurs */
	if((m_options->read() & 0x02) && !space.debugger_access())
		set_bus_error((offset << 1), 1, mem_mask);
}

READ16_MEMBER(x68k_state::x68k_exp_r)
{
	/* These are expansion devices, if not present, they cause a bus error */
	if((m_options->read() & 0x02) && !space.debugger_access())
		set_bus_error((offset << 1) + 0xeafa00, 0, mem_mask);
	return 0xff;
}

WRITE16_MEMBER(x68k_state::x68k_exp_w)
{
	/* These are expansion devices, if not present, they cause a bus error */
	if((m_options->read() & 0x02) && !space.debugger_access())
		set_bus_error((offset << 1) + 0xeafa00, 1, mem_mask);
}

void x68k_state::dma_irq(int channel)
{
	m_current_vector[3] = m_hd63450->get_vector(channel);
	m_current_irq_line = 3;
	logerror("DMA#%i: DMA End (vector 0x%02x)\n",channel,m_current_vector[3]);
	m_maincpu->set_input_line_and_vector(3,ASSERT_LINE,m_current_vector[3]);
}

WRITE8_MEMBER(x68k_state::dma_end)
{
	if(data != 0)
	{
		dma_irq(offset);
	}
	if(offset == 0)
	{
		m_fdc_tc->adjust(attotime::from_usec(1), 0, attotime::never);
	}
}

WRITE8_MEMBER(x68k_state::dma_error)
{
	if(data != 0)
	{
		m_current_vector[3] = m_hd63450->get_error_vector(offset);
		m_current_irq_line = 3;
		logerror("DMA#%i: DMA Error (vector 0x%02x)\n",offset,m_current_vector[3]);
		m_maincpu->set_input_line_and_vector(3,ASSERT_LINE,m_current_vector[3]);
	}
}

WRITE_LINE_MEMBER(x68k_state::x68k_fm_irq)
{
	if(state == CLEAR_LINE)
	{
		m_mfpdev->i3_w(1);
	}
	else
	{
		m_mfpdev->i3_w(0);
	}
}

WRITE8_MEMBER(x68k_state::x68030_adpcm_w)
{
	switch(offset)
	{
		case 0x00:
			m_okim6258->okim6258_ctrl_w(space,0,data);
			break;
		case 0x01:
			m_okim6258->okim6258_data_w(space,0,data);
			break;
	}
}

WRITE_LINE_MEMBER(x68k_state::mfp_irq_callback)
{
	if(m_mfp_prev == CLEAR_LINE && state == CLEAR_LINE)  // eliminate unnecessary calls to set the IRQ line for speed reasons
		return;
	if(state != CLEAR_LINE)
		state = HOLD_LINE;  // to get around erroneous spurious interrupt
//  if((m_ioc.irqstatus & 0xc0) != 0)  // if the FDC is busy, then we don't want to miss that IRQ
//      return;
	m_maincpu->set_input_line(6, state);
	m_current_vector[6] = 0;
	m_mfp_prev = state;
}

IRQ_CALLBACK_MEMBER(x68k_state::x68k_int_ack)
{
	if(irqline == 6)  // MFP
	{
		if(m_current_vector[6] != 0x4b && m_current_vector[6] != 0x4c)
			m_current_vector[6] = m_mfpdev->get_vector();
		else
			m_maincpu->set_input_line_and_vector(irqline,CLEAR_LINE,m_current_vector[irqline]);
		logerror("SYS: IRQ acknowledged (vector=0x%02x, line = %i)\n",m_current_vector[6],irqline);
		return m_current_vector[6];
	}

	m_maincpu->set_input_line_and_vector(irqline,CLEAR_LINE,m_current_vector[irqline]);
	if(irqline == 1)  // IOSC
	{
		m_ioc.irqstatus &= ~0xf0;
	}
	if(irqline == 5)  // SCC
	{
		m_mouse.irqactive = 0;
	}

	logerror("SYS: IRQ acknowledged (vector=0x%02x, line = %i)\n",m_current_vector[irqline],irqline);
	return m_current_vector[irqline];
}

WRITE_LINE_MEMBER(x68k_state::x68k_scsi_irq)
{
	// TODO : Internal SCSI IRQ vector 0x6c, External SCSI IRQ vector 0xf6, IRQs go through the IOSC (IRQ line 1)
	if(state != 0)
	{
		m_current_vector[1] = 0x6c;
		m_current_irq_line = 1;
		m_maincpu->set_input_line_and_vector(1,ASSERT_LINE,m_current_vector[1]);
	}
}

WRITE_LINE_MEMBER(x68k_state::x68k_scsi_drq)
{
	// TODO
}

static ADDRESS_MAP_START(x68k_map, AS_PROGRAM, 16, x68k_state )
	AM_RANGE(0x000000, 0xbffffb) AM_READWRITE(x68k_emptyram_r, x68k_emptyram_w)
	AM_RANGE(0xbffffc, 0xbfffff) AM_READWRITE(x68k_rom0_r, x68k_rom0_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE(x68k_gvram_r, x68k_gvram_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_READWRITE(x68k_tvram_r, x68k_tvram_w)
	AM_RANGE(0xe80000, 0xe81fff) AM_READWRITE(x68k_crtc_r, x68k_crtc_w)
	AM_RANGE(0xe82000, 0xe821ff) AM_DEVREADWRITE("gfxpalette", palette_device, read, write) AM_SHARE("gfxpalette")
	AM_RANGE(0xe82200, 0xe823ff) AM_DEVREADWRITE("pcgpalette", palette_device, read, write) AM_SHARE("pcgpalette")
	AM_RANGE(0xe82400, 0xe83fff) AM_READWRITE(x68k_vid_r, x68k_vid_w)
	AM_RANGE(0xe84000, 0xe85fff) AM_DEVREADWRITE("hd63450", hd63450_device, read, write)
	AM_RANGE(0xe86000, 0xe87fff) AM_READWRITE(x68k_areaset_r, x68k_areaset_w)
	AM_RANGE(0xe88000, 0xe89fff) AM_DEVREADWRITE8(MC68901_TAG, mc68901_device, read, write, 0x00ff)
	AM_RANGE(0xe8a000, 0xe8bfff) AM_DEVREADWRITE8(RP5C15_TAG, rp5c15_device, read, write, 0x00ff)
//  AM_RANGE(0xe8c000, 0xe8dfff) AM_READWRITE(x68k_printer_r, x68k_printer_w)
	AM_RANGE(0xe8e000, 0xe8ffff) AM_READWRITE(x68k_sysport_r, x68k_sysport_w)
	AM_RANGE(0xe90000, 0xe91fff) AM_DEVREADWRITE8("ym2151", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0xe92000, 0xe92001) AM_DEVREADWRITE8("okim6258", okim6258_device, okim6258_status_r, okim6258_ctrl_w, 0x00ff)
	AM_RANGE(0xe92002, 0xe92003) AM_DEVREADWRITE8("okim6258", okim6258_device, okim6258_status_r, okim6258_data_w, 0x00ff)
	AM_RANGE(0xe94000, 0xe94003) AM_DEVICE8("upd72065", upd72065_device, map, 0x00ff)
	AM_RANGE(0xe94004, 0xe94007) AM_READWRITE(x68k_fdc_r, x68k_fdc_w)
	AM_RANGE(0xe96000, 0xe9601f) AM_DEVREADWRITE("x68k_hdc", x68k_hdc_image_device, hdc_r, hdc_w)
	AM_RANGE(0xe98000, 0xe99fff) AM_READWRITE(x68k_scc_r, x68k_scc_w)
	AM_RANGE(0xe9a000, 0xe9bfff) AM_READWRITE(x68k_ppi_r, x68k_ppi_w)
	AM_RANGE(0xe9c000, 0xe9dfff) AM_READWRITE(x68k_ioc_r, x68k_ioc_w)
	AM_RANGE(0xe9e000, 0xe9e3ff) AM_READWRITE(x68k_exp_r, x68k_exp_w)  // FPU (Optional)
	AM_RANGE(0xea0000, 0xea1fff) AM_READWRITE(x68k_exp_r, x68k_exp_w)  // external SCSI ROM and controller
	AM_RANGE(0xeafa00, 0xeafa1f) AM_READWRITE(x68k_exp_r, x68k_exp_w)
	AM_RANGE(0xeafa80, 0xeafa89) AM_READWRITE(x68k_areaset_r, x68k_enh_areaset_w)
	AM_RANGE(0xeb0000, 0xeb7fff) AM_READWRITE(x68k_spritereg_r, x68k_spritereg_w)
	AM_RANGE(0xeb8000, 0xebffff) AM_READWRITE(x68k_spriteram_r, x68k_spriteram_w)
	AM_RANGE(0xece000, 0xece3ff) AM_READWRITE(x68k_exp_r, x68k_exp_w)  // User I/O
	AM_RANGE(0xed0000, 0xed3fff) AM_READWRITE(x68k_sram_r, x68k_sram_w)
	AM_RANGE(0xed4000, 0xefffff) AM_NOP
	AM_RANGE(0xf00000, 0xfbffff) AM_ROM
	AM_RANGE(0xfc0000, 0xfdffff) AM_READWRITE(x68k_exp_r, x68k_exp_w)  // internal SCSI ROM
	AM_RANGE(0xfe0000, 0xffffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(x68kxvi_map, AS_PROGRAM, 16, x68k_state )
	AM_RANGE(0x000000, 0xbffffb) AM_READWRITE(x68k_emptyram_r, x68k_emptyram_w)
	AM_RANGE(0xbffffc, 0xbfffff) AM_READWRITE(x68k_rom0_r, x68k_rom0_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE(x68k_gvram_r, x68k_gvram_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_READWRITE(x68k_tvram_r, x68k_tvram_w)
	AM_RANGE(0xe80000, 0xe81fff) AM_READWRITE(x68k_crtc_r, x68k_crtc_w)
	AM_RANGE(0xe82000, 0xe821ff) AM_DEVREADWRITE("gfxpalette", palette_device, read, write) AM_SHARE("gfxpalette")
	AM_RANGE(0xe82200, 0xe823ff) AM_DEVREADWRITE("pcgpalette", palette_device, read, write) AM_SHARE("pcgpalette")
	AM_RANGE(0xe82400, 0xe83fff) AM_READWRITE(x68k_vid_r, x68k_vid_w)
	AM_RANGE(0xe84000, 0xe85fff) AM_DEVREADWRITE("hd63450", hd63450_device, read, write)
	AM_RANGE(0xe86000, 0xe87fff) AM_READWRITE(x68k_areaset_r, x68k_areaset_w)
	AM_RANGE(0xe88000, 0xe89fff) AM_DEVREADWRITE8(MC68901_TAG, mc68901_device, read, write, 0x00ff)
	AM_RANGE(0xe8a000, 0xe8bfff) AM_DEVREADWRITE8(RP5C15_TAG, rp5c15_device, read, write, 0x00ff)
//  AM_RANGE(0xe8c000, 0xe8dfff) AM_READWRITE(x68k_printer_r, x68k_printer_w)
	AM_RANGE(0xe8e000, 0xe8ffff) AM_READWRITE(x68k_sysport_r, x68k_sysport_w)
	AM_RANGE(0xe90000, 0xe91fff) AM_DEVREADWRITE8("ym2151", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0xe92000, 0xe92001) AM_DEVREADWRITE8("okim6258", okim6258_device, okim6258_status_r, okim6258_ctrl_w, 0x00ff)
	AM_RANGE(0xe92002, 0xe92003) AM_DEVREADWRITE8("okim6258", okim6258_device, okim6258_status_r, okim6258_data_w, 0x00ff)
	AM_RANGE(0xe94000, 0xe94003) AM_DEVICE8("upd72065", upd72065_device, map, 0x00ff)
	AM_RANGE(0xe94004, 0xe94007) AM_READWRITE(x68k_fdc_r, x68k_fdc_w)
//  AM_RANGE(0xe96000, 0xe9601f) AM_DEVREADWRITE("x68k_hdc", x68k_hdc_image_device, hdc_r, hdc_w)
	AM_RANGE(0xe96020, 0xe9603f) AM_DEVREADWRITE8("mb89352",mb89352_device,mb89352_r,mb89352_w,0x00ff)
	AM_RANGE(0xe98000, 0xe99fff) AM_READWRITE(x68k_scc_r, x68k_scc_w)
	AM_RANGE(0xe9a000, 0xe9bfff) AM_READWRITE(x68k_ppi_r, x68k_ppi_w)
	AM_RANGE(0xe9c000, 0xe9dfff) AM_READWRITE(x68k_ioc_r, x68k_ioc_w)
	AM_RANGE(0xe9e000, 0xe9e3ff) AM_READWRITE(x68k_exp_r, x68k_exp_w)  // FPU (Optional)
	AM_RANGE(0xea0000, 0xea1fff) AM_READWRITE(x68k_exp_r, x68k_exp_w)  // external SCSI ROM and controller
	AM_RANGE(0xeafa00, 0xeafa1f) AM_READWRITE(x68k_exp_r, x68k_exp_w)
	AM_RANGE(0xeafa80, 0xeafa89) AM_READWRITE(x68k_areaset_r, x68k_enh_areaset_w)
	AM_RANGE(0xeb0000, 0xeb7fff) AM_READWRITE(x68k_spritereg_r, x68k_spritereg_w)
	AM_RANGE(0xeb8000, 0xebffff) AM_READWRITE(x68k_spriteram_r, x68k_spriteram_w)
	AM_RANGE(0xece000, 0xece3ff) AM_READWRITE(x68k_exp_r, x68k_exp_w)  // User I/O
	AM_RANGE(0xed0000, 0xed3fff) AM_READWRITE(x68k_sram_r, x68k_sram_w)
	AM_RANGE(0xed4000, 0xefffff) AM_NOP
	AM_RANGE(0xf00000, 0xfbffff) AM_ROM
	AM_RANGE(0xfc0000, 0xfdffff) AM_ROM  // internal SCSI ROM
	AM_RANGE(0xfe0000, 0xffffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(x68030_map, AS_PROGRAM, 32, x68k_state )
	ADDRESS_MAP_GLOBAL_MASK(0x00ffffff)  // Still only has 24-bit address space
	AM_RANGE(0x000000, 0xbffffb) AM_READWRITE16(x68k_emptyram_r, x68k_emptyram_w,0xffffffff)
	AM_RANGE(0xbffffc, 0xbfffff) AM_READWRITE16(x68k_rom0_r, x68k_rom0_w,0xffffffff)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE16(x68k_gvram_r, x68k_gvram_w, 0xffffffff)
	AM_RANGE(0xe00000, 0xe7ffff) AM_READWRITE16(x68k_tvram_r, x68k_tvram_w, 0xffffffff)
	AM_RANGE(0xe80000, 0xe81fff) AM_READWRITE16(x68k_crtc_r, x68k_crtc_w,0xffffffff)
	AM_RANGE(0xe82000, 0xe821ff) AM_DEVREADWRITE("gfxpalette", palette_device, read, write) AM_SHARE("gfxpalette")
	AM_RANGE(0xe82200, 0xe823ff) AM_DEVREADWRITE("pcgpalette", palette_device, read, write) AM_SHARE("pcgpalette")
	AM_RANGE(0xe82400, 0xe83fff) AM_READWRITE16(x68k_vid_r, x68k_vid_w,0xffffffff)
	AM_RANGE(0xe84000, 0xe85fff) AM_DEVREADWRITE16("hd63450", hd63450_device, read, write, 0xffffffff)
	AM_RANGE(0xe86000, 0xe87fff) AM_READWRITE16(x68k_areaset_r, x68k_areaset_w,0xffffffff)
	AM_RANGE(0xe88000, 0xe89fff) AM_DEVREADWRITE8(MC68901_TAG, mc68901_device, read, write, 0x00ff00ff)
	AM_RANGE(0xe8a000, 0xe8bfff) AM_DEVREADWRITE8(RP5C15_TAG, rp5c15_device, read, write, 0x00ff00ff)
//  AM_RANGE(0xe8c000, 0xe8dfff) AM_READWRITE(x68k_printer_r, x68k_printer_w)
	AM_RANGE(0xe8e000, 0xe8ffff) AM_READWRITE16(x68k_sysport_r, x68k_sysport_w,0xffffffff)
	AM_RANGE(0xe90000, 0xe91fff) AM_DEVREADWRITE8("ym2151", ym2151_device, read, write, 0x00ff00ff)
	AM_RANGE(0xe92000, 0xe92003) AM_DEVREAD8("okim6258", okim6258_device, okim6258_status_r, 0x00ff00ff) AM_WRITE8(x68030_adpcm_w, 0x00ff00ff)
	AM_RANGE(0xe94000, 0xe94003) AM_DEVICE8("upd72065", upd72065_device, map, 0x00ff00ff)
	AM_RANGE(0xe94004, 0xe94007) AM_READWRITE16(x68k_fdc_r, x68k_fdc_w,0xffffffff)
//  AM_RANGE(0xe96000, 0xe9601f) AM_DEVREADWRITE16("x68k_hdc", x68k_hdc_image_device, hdc_r, hdc_w, 0xffffffff)
	AM_RANGE(0xe96020, 0xe9603f) AM_DEVREADWRITE8("mb89352",mb89352_device,mb89352_r,mb89352_w,0x00ff00ff)
	AM_RANGE(0xe98000, 0xe99fff) AM_READWRITE16(x68k_scc_r, x68k_scc_w,0xffffffff)
	AM_RANGE(0xe9a000, 0xe9bfff) AM_READWRITE16(x68k_ppi_r, x68k_ppi_w,0xffffffff)
	AM_RANGE(0xe9c000, 0xe9dfff) AM_READWRITE16(x68k_ioc_r, x68k_ioc_w,0xffffffff)
	AM_RANGE(0xe9e000, 0xe9e3ff) AM_READWRITE16(x68k_exp_r, x68k_exp_w,0xffffffff)  // FPU (Optional)
	AM_RANGE(0xea0000, 0xea1fff) AM_NOP//AM_READWRITE16(x68k_exp_r, x68k_exp_w,0xffffffff)  // external SCSI ROM and controller
	AM_RANGE(0xeafa00, 0xeafa1f) AM_READWRITE16(x68k_exp_r, x68k_exp_w,0xffffffff)
	AM_RANGE(0xeafa80, 0xeafa8b) AM_READWRITE16(x68k_areaset_r, x68k_enh_areaset_w,0xffffffff)
	AM_RANGE(0xeb0000, 0xeb7fff) AM_READWRITE16(x68k_spritereg_r, x68k_spritereg_w,0xffffffff)
	AM_RANGE(0xeb8000, 0xebffff) AM_READWRITE16(x68k_spriteram_r, x68k_spriteram_w,0xffffffff)
	AM_RANGE(0xece000, 0xece3ff) AM_READWRITE16(x68k_exp_r, x68k_exp_w,0xffffffff)  // User I/O
	AM_RANGE(0xed0000, 0xed3fff) AM_READWRITE16(x68k_sram_r, x68k_sram_w, 0xffffffff)
	AM_RANGE(0xed4000, 0xefffff) AM_NOP
	AM_RANGE(0xf00000, 0xfbffff) AM_ROM
	AM_RANGE(0xfc0000, 0xfdffff) AM_ROM  // internal SCSI ROM
	AM_RANGE(0xfe0000, 0xffffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( x68000 )
	PORT_START("ctrltype")
	PORT_CONFNAME(0x0f, 0x00, "Joystick Port 1")
	PORT_CONFSETTING(0x00, "Standard 2-button MSX/FM-Towns joystick")
	PORT_CONFSETTING(0x01, "3-button Megadrive gamepad")
	PORT_CONFSETTING(0x02, "6-button Megadrive gamepad")
	PORT_CONFSETTING(0x03, "XPD-1LR dual D-pad gamepad")
	PORT_CONFNAME(0xf0, 0x00, "Joystick Port 2")
	PORT_CONFSETTING(0x00, "Standard 2-button MSX/FM-Towns joystick")
	PORT_CONFSETTING(0x10, "3-button Megadrive gamepad")
	PORT_CONFSETTING(0x20, "6-button Megadrive gamepad")
	PORT_CONFSETTING(0x30, "XPD-1LR dual D-pad gamepad")
// TODO: Sharp Cyber Stick (CZ-8NJ2) support

	PORT_START( "joy1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_CODE(JOYCODE_Y_UP_SWITCH)  PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)    PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(JOYCODE_BUTTON1)  PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(JOYCODE_BUTTON2)  PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x00)

	PORT_START( "joy2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_CODE(JOYCODE_Y_UP_SWITCH)  PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)    PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(JOYCODE_BUTTON1)  PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(JOYCODE_BUTTON2)  PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x00)

	PORT_START("options")
	PORT_CONFNAME( 0x02, 0x02, "Enable fake bus errors")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ))
	PORT_CONFSETTING(   0x02, DEF_STR( On ))
	PORT_CONFNAME( 0x04, 0x04, "Enable partial updates on each HSync")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ))
	PORT_CONFSETTING(   0x04, DEF_STR( On ))

	PORT_START("mouse1")  // mouse buttons
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("mouse2")  // X-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("mouse3")  // Y-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	// 3-button Megadrive gamepad
	PORT_START("md3b")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("MD Pad 1 Up") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("MD Pad 1 Down") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("MD Pad 1 Left") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("MD Pad 1 Right") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 B Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 C Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)

	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 A Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 Start Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("MD Pad 2 Up") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("MD Pad 2 Down") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("MD Pad 2 Left") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("MD Pad 2 Right") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 B Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 C Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)

	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 A Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 Start Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)

	// 6-button Megadrive gamepad
	PORT_START("md6b")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("MD Pad 1 Up") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("MD Pad 1 Down") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("MD Pad 1 Left") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("MD Pad 1 Right") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 B Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 C Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)

	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 A Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 Start Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("MD Pad 2 Up") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("MD Pad 2 Down") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("MD Pad 2 Left") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("MD Pad 2 Right") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 B Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 C Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)

	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 A Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 Start Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)

	// extra inputs
	PORT_START("md6b_extra")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 Z Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 Y Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 X Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("MD Pad 1 Mode Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x02)

	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 Z Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 Y Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 X Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("MD Pad 2 Mode Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x20)

	// Dempa/Micomsoft XPD-1LR (dual D-pad gamepad sold with Libble Rabble)
	PORT_START("xpd1lr")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_NAME("XPD Pad 1 Left/Up") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("XPD Pad 1 Left/Down") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_NAME("XPD Pad 1 Left/Left") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_NAME("XPD Pad 1 Left/Right") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("XPD Pad 1 B Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("XPD Pad 1 A Button") PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)

	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("XPD Pad 1 Right/Up") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("XPD Pad 1 Right/Down") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_NAME("XPD Pad 1 Right/Left") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("XPD Pad 1 Right/Right") PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x03)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_NAME("XPD Pad 2 Left/Up") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("XPD Pad 2 Left/Down") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_NAME("XPD Pad 2 Left/Left") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_NAME("XPD Pad 2 Left/Right") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("XPD Pad 2 B Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("XPD Pad 2 A Button") PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)

	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("XPD Pad 2 Right/Up") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("XPD Pad 2 Right/Down") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_NAME("XPD Pad 2 Right/Left") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("XPD Pad 2 Right/Right") PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x30)

INPUT_PORTS_END

void x68k_state::floppy_load_unload(bool load, floppy_image_device *dev)
{
	dev->mon_w(m_fdc.motor && !load);
	if(m_ioc.irqstatus & 0x02)
	{
		m_current_vector[1] = 0x61;
		m_ioc.irqstatus |= 0x40;
		m_current_irq_line = 1;
		m_maincpu->set_input_line_and_vector(1,ASSERT_LINE,m_current_vector[1]);  // Disk insert/eject interrupt
		logerror("IOC: Disk image inserted\n");
	}
}

int x68k_state::floppy_load(floppy_image_device *dev)
{
	floppy_load_unload(true, dev);
	return IMAGE_INIT_PASS;
}

void x68k_state::floppy_unload(floppy_image_device *dev)
{
	floppy_load_unload(false, dev);
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_net_irq)
{
	m_current_vector[2] = 0xf9;
	m_current_irq_line = 2;
	m_maincpu->set_input_line_and_vector(2,ASSERT_LINE,m_current_vector[2]);
}

WRITE_LINE_MEMBER(x68k_state::x68k_irq2_line)
{
	if(state==ASSERT_LINE)
	{
		m_net_timer->adjust(attotime::from_usec(16));
	}
	else
		m_maincpu->set_input_line_and_vector(2,CLEAR_LINE,m_current_vector[2]);
	logerror("EXP: IRQ2 set to %i\n",state);

}

static SLOT_INTERFACE_START(x68000_exp_cards)
	SLOT_INTERFACE("neptunex",X68K_NEPTUNEX) // Neptune-X ethernet adapter (ISA NE2000 bridge)
	SLOT_INTERFACE("cz6bs1",X68K_SCSIEXT)  // Sharp CZ-6BS1 SCSI-1 controller
SLOT_INTERFACE_END

MACHINE_RESET_MEMBER(x68k_state,x68000)
{
	/* The last half of the IPLROM is mapped to 0x000000 on reset only
	   Just copying the inital stack pointer and program counter should
	   more or less do the same job */

	int drive;
	UINT8* romdata = memregion("user2")->base();
	attotime irq_time;

	memset(m_ram->pointer(),0,m_ram->size());
	memcpy(m_ram->pointer(),romdata,8);

	// initialise CRTC, set registers to defaults for the standard text mode (768x512)
	m_crtc.reg[0] = 137;  // Horizontal total  (in characters)
	m_crtc.reg[1] = 14;   // Horizontal sync end
	m_crtc.reg[2] = 28;   // Horizontal start
	m_crtc.reg[3] = 124;  // Horizontal end
	m_crtc.reg[4] = 567;  // Vertical total
	m_crtc.reg[5] = 5;    // Vertical sync end
	m_crtc.reg[6] = 40;   // Vertical start
	m_crtc.reg[7] = 552;  // Vertical end
	m_crtc.reg[8] = 27;   // Horizontal adjust

	m_scanline = machine().first_screen()->vpos();// = m_crtc.reg[6];  // Vertical start

	// start VBlank timer
	m_crtc.vblank = 1;
	irq_time = m_screen->time_until_pos(m_crtc.reg[6],2);
	m_vblank_irq->adjust(irq_time);

	// start HBlank timer
	m_scanline_timer->adjust(m_screen->scan_period(), 1);

	/// TODO: get callbacks to trigger these
	m_mfpdev->i0_w(1); // alarm
	m_mfpdev->i1_w(1); // expon
	m_mfpdev->i2_w(0); // pow sw
	m_mfpdev->i3_w(1); // fmirq
	m_mfpdev->i4_w(1); // v-disp
	m_mfpdev->i5_w(1); // unused (always set)
	m_mfpdev->i6_w(1); // cirq
	m_mfpdev->i7_w(1); // h-sync

	// reset output values
	output_set_value("key_led_kana",1);
	output_set_value("key_led_romaji",1);
	output_set_value("key_led_code",1);
	output_set_value("key_led_caps",1);
	output_set_value("key_led_insert",1);
	output_set_value("key_led_hiragana",1);
	output_set_value("key_led_fullsize",1);
	for(drive=0;drive<4;drive++)
	{
		output_set_indexed_value("eject_drv",drive,1);
		output_set_indexed_value("ctrl_drv",drive,1);
		output_set_indexed_value("access_drv",drive,1);
	}
	m_fdc.select_drive = 0;

	// reset CPU
	m_maincpu->reset();
}

MACHINE_START_MEMBER(x68k_state,x68000)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	/*  Install RAM handlers  */
	m_spriteram = (UINT16*)(memregion("user1")->base());
	space.install_readwrite_bank(0x000000,m_ram->size()-1,0xffffffff,0,"bank1");
	membank("bank1")->set_base(m_ram->pointer());

	// start mouse timer
	m_mouse_timer->adjust(attotime::zero, 0, attotime::from_msec(1));  // a guess for now
	m_mouse.inputtype = 0;

	// start LED timer
	m_led_timer->adjust(attotime::zero, 0, attotime::from_msec(400));

	for(int drive=0;drive<4;drive++)
	{
		char devname[16];
		sprintf(devname, "%d", drive);
		floppy_image_device *floppy = m_upd72065->subdevice<floppy_connector>(devname)->get_device();
		m_fdc.floppy[drive] = floppy;
		if(floppy) {
			floppy->setup_load_cb(floppy_image_device::load_cb(FUNC(x68k_state::floppy_load), this));
			floppy->setup_unload_cb(floppy_image_device::unload_cb(FUNC(x68k_state::floppy_unload), this));
		}
	}
	m_fdc.motor = 0;
}

DRIVER_INIT_MEMBER(x68k_state,x68000)
{
	unsigned char* rom = memregion("maincpu")->base();
	unsigned char* user2 = memregion("user2")->base();

	machine().device<nvram_device>("nvram")->set_base(&m_nvram[0], m_nvram.size()*sizeof(m_nvram[0]));

#ifdef USE_PREDEFINED_SRAM
	{
		unsigned char* ramptr = memregion("user3")->base();
		memcpy(m_sram,ramptr,0x4000);
	}
#endif

	// copy last half of BIOS to a user region, to use for inital startup
	memcpy(user2,(rom+0xff0000),0x10000);

	m_scanline_timer = timer_alloc(TIMER_X68K_HSYNC);
	m_raster_irq = timer_alloc(TIMER_X68K_CRTC_RASTER_IRQ);
	m_vblank_irq = timer_alloc(TIMER_X68K_CRTC_VBLANK_IRQ);
	m_mouse_timer = timer_alloc(TIMER_X68K_SCC_ACK);
	m_led_timer = timer_alloc(TIMER_X68K_LED);
	m_net_timer = timer_alloc(TIMER_X68K_NET_IRQ);
	m_fdc_tc = timer_alloc(TIMER_X68K_FDC_TC);
	m_adpcm_timer = timer_alloc(TIMER_X68K_ADPCM);
	m_bus_error_timer = timer_alloc(TIMER_X68K_BUS_ERROR);

	// Initialise timers for 6-button MD controllers
	md_6button_init();

	m_sysport.cputype = 0xff;  // 68000, 10MHz
	m_is_32bit = false;

	save_item(NAME(m_tvram));
	save_item(NAME(m_gvram));
	save_item(NAME(m_spritereg));
}

DRIVER_INIT_MEMBER(x68k_state,x68kxvi)
{
	DRIVER_INIT_CALL( x68000 );
	m_sysport.cputype = 0xfe; // 68000, 16MHz
	m_is_32bit = false;
}

DRIVER_INIT_MEMBER(x68k_state,x68030)
{
	DRIVER_INIT_CALL( x68000 );
	m_sysport.cputype = 0xdc; // 68030, 25MHz
	m_is_32bit = true;
}

FLOPPY_FORMATS_MEMBER( x68k_state::floppy_formats )
	FLOPPY_XDF_FORMAT,
	FLOPPY_DIM_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( x68k_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(keyboard)
	SLOT_INTERFACE("x68k", X68K_KEYBOARD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( x68000, x68k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)  /* 10 MHz */
	MCFG_CPU_PROGRAM_MAP(x68k_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(x68k_state,x68k_int_ack)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(x68k_state, x68000 )
	MCFG_MACHINE_RESET_OVERRIDE(x68k_state, x68000 )

	/* device hardware */
	MCFG_DEVICE_ADD(MC68901_TAG, MC68901, 4000000)
	MCFG_MC68901_TIMER_CLOCK(4000000)
	MCFG_MC68901_RX_CLOCK(0)
	MCFG_MC68901_TX_CLOCK(0)
	MCFG_MC68901_OUT_IRQ_CB(WRITELINE(x68k_state, mfp_irq_callback))
	MCFG_MC68901_OUT_TBO_CB(DEVWRITELINE(MC68901_TAG, mc68901_device, clock_w))
	MCFG_MC68901_OUT_SO_CB(DEVWRITELINE("keyboard", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("keyboard", keyboard, "x68k")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(MC68901_TAG, mc68901_device, write_rx))

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(x68k_state, ppi_port_a_r))
	MCFG_I8255_IN_PORTB_CB(READ8(x68k_state, ppi_port_b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(x68k_state, ppi_port_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(x68k_state, ppi_port_c_w))

	MCFG_DEVICE_ADD("hd63450", HD63450, 0)
	MCFG_HD63450_CPU("maincpu") // CPU - 68000
	MCFG_HD63450_CLOCKS(attotime::from_usec(2), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625/2))
	MCFG_HD63450_BURST_CLOCKS(attotime::from_usec(2), attotime::from_nsec(450), attotime::from_nsec(50), attotime::from_nsec(50))
	MCFG_HD63450_DMA_END_CB(WRITE8(x68k_state, dma_end))
	MCFG_HD63450_DMA_ERROR_CB(WRITE8(x68k_state, dma_error))
	MCFG_HD63450_DMA_READ_0_CB(DEVREAD8("upd72065", upd72065_device, mdma_r))
	MCFG_HD63450_DMA_WRITE_0_CB(DEVWRITE8("upd72065", upd72065_device, mdma_w))

	MCFG_DEVICE_ADD("scc", SCC8530, 5000000)

	MCFG_DEVICE_ADD(RP5C15_TAG, RP5C15, XTAL_32_768kHz)
	MCFG_RP5C15_OUT_ALARM_CB(DEVWRITELINE(MC68901_TAG, mc68901_device, i0_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55.45)
	MCFG_SCREEN_SIZE(1096, 568)  // inital setting
	MCFG_SCREEN_VISIBLE_AREA(0, 767, 0, 511)
	MCFG_SCREEN_UPDATE_DRIVER(x68k_state, screen_update_x68000)

	MCFG_GFXDECODE_ADD("gfxdecode", "pcgpalette", empty)

	MCFG_PALETTE_ADD("gfxpalette", 256)
	MCFG_PALETTE_FORMAT_CLASS(2, x68k_state, GGGGGRRRRRBBBBBI)
	MCFG_PALETTE_ADD("pcgpalette", 256)
	MCFG_PALETTE_FORMAT_CLASS(2, x68k_state, GGGGGRRRRRBBBBBI)

	MCFG_VIDEO_START_OVERRIDE(x68k_state, x68000 )

	MCFG_DEFAULT_LAYOUT( layout_x68000 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_YM2151_ADD("ym2151", 4000000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(x68k_state,x68k_fm_irq))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(x68k_state,x68k_ct_w))  // CT1, CT2 from YM2151 port 0x1b
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_OKIM6258_ADD("okim6258", 4000000)
	MCFG_OKIM6258_DIVIDER(FOSC_DIV_BY_512)
	MCFG_OKIM6258_ADPCM_TYPE(TYPE_4BITS)
	MCFG_OKIM6258_OUT_BITS(OUTPUT_10BITS)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_UPD72065_ADD("upd72065", true, false)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(x68k_state, fdc_irq))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("hd63450", hd63450_device, drq0_w))
	MCFG_FLOPPY_DRIVE_ADD("upd72065:0", x68k_floppies, "525hd", x68k_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd72065:1", x68k_floppies, "525hd", x68k_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd72065:2", x68k_floppies, "525hd", x68k_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd72065:3", x68k_floppies, "525hd", x68k_state::floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("flop_list","x68k_flop")

	MCFG_DEVICE_ADD("exp", X68K_EXPANSION_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(x68000_exp_cards, NULL, false)
	MCFG_X68K_EXPANSION_SLOT_OUT_IRQ2_CB(WRITELINE(x68k_state, x68k_irq2_line))
	MCFG_X68K_EXPANSION_SLOT_OUT_IRQ4_CB(INPUTLINE("maincpu", M68K_IRQ_4))
	MCFG_X68K_EXPANSION_SLOT_OUT_NMI_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,3M,5M,6M,7M,8M,9M,10M,11M,12M")

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_X68KHDC_ADD( "x68k_hdc" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( x68ksupr, x68000 )
	MCFG_DEVICE_REMOVE("x68k_hdc")

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(x68kxvi_map)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_1)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE3, "harddisk", SCSIHD, SCSI_ID_2)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE4, "harddisk", SCSIHD, SCSI_ID_3)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE5, "harddisk", SCSIHD, SCSI_ID_4)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE6, "harddisk", SCSIHD, SCSI_ID_5)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE7, "harddisk", SCSIHD, SCSI_ID_6)

	MCFG_DEVICE_ADD("mb89352", MB89352A, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_MB89352A_IRQ_CB(WRITELINE(x68k_state, x68k_scsi_irq))
	MCFG_MB89352A_DRQ_CB(WRITELINE(x68k_state, x68k_scsi_drq))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( x68kxvi, x68ksupr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(16000000)  /* 16 MHz */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( x68030, x68ksupr )
	MCFG_CPU_REPLACE("maincpu", M68030, 25000000)  /* 25 MHz 68EC030 */
	MCFG_CPU_PROGRAM_MAP(x68030_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(x68k_state,x68k_int_ack)
MACHINE_CONFIG_END

ROM_START( x68000 )
	ROM_REGION16_BE(0x1000000, "maincpu", 0)  // 16MB address space
	ROM_DEFAULT_BIOS("ipl10")
	ROM_LOAD( "cgrom.dat",  0xf00000, 0xc0000, CRC(9f3195f1) SHA1(8d72c5b4d63bb14c5dbdac495244d659aa1498b6) )
	ROM_SYSTEM_BIOS(0, "ipl10",  "IPL-ROM V1.0 (87/05/07)")
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(4) )
	ROM_REGION(0x8000, "user1",0)  // For Background/Sprite decoding
	ROM_FILL(0x0000,0x8000,0x00)
	ROM_REGION(0x20000, "user2", 0)
	ROM_FILL(0x000,0x20000,0x00)
ROM_END

ROM_START( x68ksupr )
	ROM_REGION16_BE(0x1000000, "maincpu", 0)  // 16MB address space
	ROM_DEFAULT_BIOS("ipl11")
	ROM_LOAD( "cgrom.dat",  0xf00000, 0xc0000, CRC(9f3195f1) SHA1(8d72c5b4d63bb14c5dbdac495244d659aa1498b6) )
	ROM_SYSTEM_BIOS(0, "ipl10",  "IPL-ROM V1.0 (87/05/07)")
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(4) )
	ROM_LOAD("scsiinsu.bin",0xfc0000, 0x002000, CRC(f65a3e24) SHA1(15a17798839a3f7f361119205aebc301c2df5967) )  // Dumped from an X68000 Super HD
//  ROM_LOAD("scsiexrom.dat",0xea0000, 0x002000, NO_DUMP )
	ROM_REGION(0x8000, "user1",0)  // For Background/Sprite decoding
	ROM_FILL(0x0000,0x8000,0x00)
	ROM_REGION(0x20000, "user2", 0)
	ROM_FILL(0x000,0x20000,0x00)
ROM_END

ROM_START( x68kxvi )
	ROM_REGION16_BE(0x1000000, "maincpu", 0)  // 16MB address space
	ROM_DEFAULT_BIOS("ipl11")
	ROM_LOAD( "cgrom.dat",  0xf00000, 0xc0000, CRC(9f3195f1) SHA1(8d72c5b4d63bb14c5dbdac495244d659aa1498b6) )
	ROM_SYSTEM_BIOS(0, "ipl10",  "IPL-ROM V1.0 (87/05/07)")
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(4) )
	ROM_LOAD("scsiinco.bin",0xfc0000, 0x002000, CRC(2485e14d) SHA1(101a9bba8ea4bb90965c144bcfd7182f889ab958) )  // Dumped from an X68000 XVI Compact
//  ROM_LOAD("scsiexrom.dat",0xea0000, 0x002000, NO_DUMP )
	ROM_REGION(0x8000, "user1",0)  // For Background/Sprite decoding
	ROM_FILL(0x0000,0x8000,0x00)
	ROM_REGION(0x20000, "user2", 0)
	ROM_FILL(0x000,0x20000,0x00)
ROM_END

ROM_START( x68030 )
	ROM_REGION16_BE(0x1000000, "maincpu", 0)  // 16MB address space
	ROM_DEFAULT_BIOS("ipl13")
	ROM_LOAD( "cgrom.dat",  0xf00000, 0xc0000, CRC(9f3195f1) SHA1(8d72c5b4d63bb14c5dbdac495244d659aa1498b6) )
	ROM_SYSTEM_BIOS(0, "ipl10",  "IPL-ROM V1.0 (87/05/07)")
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(4) )
	ROM_LOAD("scsiinrom.dat",0xfc0000, 0x002000, CRC(1c6c889e) SHA1(3f063d4231cdf53da6adc4db96533725e260076a) BAD_DUMP )
//  ROM_LOAD("scsiexrom.dat",0xea0000, 0x002000, NO_DUMP )
	ROM_REGION(0x8000, "user1",0)  // For Background/Sprite decoding
	ROM_FILL(0x0000,0x8000,0x00)
	ROM_REGION(0x20000, "user2", 0)
	ROM_FILL(0x000,0x20000,0x00)
ROM_END


/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY     FULLNAME        FLAGS */
COMP( 1987, x68000, 0,      0,      x68000, x68000, x68k_state, x68000, "Sharp",    "X68000", MACHINE_IMPERFECT_GRAPHICS )
COMP( 1990, x68ksupr,x68000, 0,     x68ksupr,x68000, x68k_state,x68000, "Sharp",    "X68000 Super", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
COMP( 1991, x68kxvi,x68000, 0,      x68kxvi,x68000, x68k_state, x68kxvi,"Sharp",    "X68000 XVI", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
COMP( 1993, x68030, x68000, 0,      x68030, x68000, x68k_state, x68030, "Sharp",    "X68030", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
