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
#include "x68k.h"
#include "x68k_hdc.h"
#include "x68k_kbd.h"
#include "x68k_mouse.h"

#include "machine/input_merger.h"
#include "machine/nvram.h"

#include "bus/x68k/x68k_neptunex.h"
#include "bus/x68k/x68k_scsiext.h"
#include "bus/x68k/x68k_midi.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/cd.h"

#include "softlist.h"
#include "speaker.h"

#include "formats/dim_dsk.h"
#include "formats/xdf_dsk.h"

#include "x68000.lh"

#define LOG_FDC (1U << 1)
#define LOG_SYS (1U << 2)
#define LOG_IRQ (1U << 3)
//#define VERBOSE (LOG_FDC | LOG_SYS | LOG_IRQ)
#include "logmacro.h"


static constexpr uint32_t adpcm_clock[2] = { 8000000, 4000000 };
static constexpr uint32_t adpcm_div[4] = { 1024, 768, 512, /* Reserved */512 };

TIMER_CALLBACK_MEMBER(x68k_state::floppy_tc_tick)
{
	m_upd72065->tc_w(ASSERT_LINE);
	m_upd72065->tc_w(CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(x68k_state::adpcm_drq_tick)
{
	m_hd63450->drq3_w(1);
	m_hd63450->drq3_w(0);
}

// LED timer callback
TIMER_CALLBACK_MEMBER(x68k_state::led_callback)
{
	m_led_state = !m_led_state ? 1 : 0;
	if(m_led_state)
	{
		for(int drive=0; drive<4; drive++)
			m_ctrl_drv_out[drive] = m_fdc.led_ctrl[drive] ? 0 : 1;
	}
	else
	{
		std::fill(std::begin(m_ctrl_drv_out), std::end(m_ctrl_drv_out), 1);
	}

}

void x68k_state::set_adpcm()
{
	uint32_t rate = adpcm_div[m_adpcm.rate];
	uint32_t res_clock = adpcm_clock[m_adpcm.clock]/2;
	attotime newperiod = attotime::from_ticks(rate, res_clock);
	attotime newremain = newperiod;
	if((m_adpcm_timer->period() != attotime::never) && (m_adpcm_timer->period() != attotime::zero))
		newremain = newperiod * (m_adpcm_timer->remaining().as_double() / m_adpcm_timer->period().as_double());
	m_adpcm_timer->adjust(newremain, 0, newperiod);
}

// PPI ports A and B are joystick inputs
uint8_t x68k_state::ppi_port_a_r()
{
	// first read the joystick inputs
	uint8_t const input = m_joy[0]->read();
	uint8_t result = 0x90 | (BIT(input, 4, 2) << 5) | BIT(input, 0, 4);

	// trigger lines can be pulled down by port C outputs
	result &= ~(BIT(m_ppi_portc, 6, 2) << 5);

	return result;
}

uint8_t x68k_state::ppi_port_b_r()
{
	uint8_t const input = m_joy[1]->read();
	return 0x90 | (BIT(input, 4, 2) << 5) | BIT(input, 0, 4);
}

uint8_t x68k_state::ppi_port_c_r()
{
	return m_ppi_portc;
}

/* PPI port C (Joystick control, R/W)
   bit 7    - IOC7 - Pull down joystick 1 trigger B (JS pin 7)
   bit 6    - IOC6 - Pull down joystick 1 trigger A (JS pin 6)
   bit 5    - IOC5 - Joystick 2 strobe (JT pin 8)
   bit 4    - IOC4 - Joystick 1 strobe (JS pin 8)
   bits 3,2 - ADPCM Sample rate
   bits 1,0 - ADPCM Pan (00 = Both, 01 = Right only, 10 = Left only, 11 = Off)
*/
void x68k_state::ppi_port_c_w(uint8_t data)
{
	// ADPCM / Joystick control
	if((data & 0x03) != (m_ppi_portc & 0x03))
	{
		m_adpcm.pan = data & 0x03;
		m_adpcm_out[0]->set_gain((m_adpcm.pan & 1) ? 0.0f : 1.0f);
		m_adpcm_out[1]->set_gain((m_adpcm.pan & 2) ? 0.0f : 1.0f);
	}
	if((data & 0x0c) != (m_ppi_portc & 0x0c))
	{
		m_adpcm.rate = (data & 0x0c) >> 2;
		if (m_adpcm.rate == 3)
			LOGMASKED(LOG_SYS, "PPI: Invalid ADPCM sample rate set.\n");

		set_adpcm();
		m_okim6258->set_divider(m_adpcm.rate);
	}

	// Set joystick outputs
	if(BIT(data, 6) != BIT(m_ppi_portc, 6))
		m_joy[0]->pin_6_w(BIT(~data, 6));
	if(BIT(data, 7) != BIT(m_ppi_portc, 7))
		m_joy[0]->pin_7_w(BIT(~data, 7));
	if(BIT(data, 4) != BIT(m_ppi_portc, 4))
		m_joy[0]->pin_8_w(BIT(data, 4));
	if(BIT(data, 5) != BIT(m_ppi_portc, 5))
		m_joy[1]->pin_8_w(BIT(data, 5));

	// update saved value
	m_ppi_portc = data;
}


// NEC uPD72065 at 0xe94000
void x68k_state::fdc_w(offs_t offset, uint16_t data)
{
	unsigned int drive, x;
	switch(offset)
	{
	case 0x00:  // drive option signal control
		x = data & 0x0f;
		for(drive=0;drive<4;drive++)
		{
			if(BIT(m_fdc.control_drives, drive) && m_fdc.floppy[drive])
			{
				if(!BIT(x, drive))  // functions take place on 1->0 transitions of drive bits only
				{
					m_fdc.led_ctrl[drive] = data & 0x80;  // blinking drive LED if no disk inserted
					m_fdc.led_eject[drive] = data & 0x40;  // eject button LED (on when set to 0)
					m_eject_drv_out[drive] = BIT(data, 6);
					if((data & 0x60) == 0x20)  // ejects disk
						m_fdc.floppy[drive]->unload();
				}
			}
		}
		m_fdc.control_drives = data & 0x0f;
		LOGMASKED(LOG_FDC, "FDC: signal control set to %02x\n",data);
		break;
	case 0x01: {
		x = data & 3;
		m_upd72065->set_floppy(m_fdc.floppy[x]);
		m_upd72065->set_rate((data & 0x10) ? 300000 : 500000);
		m_fdc.motor = data & 0x80;

		for(int i = 0; i < 4; i++)
			if(m_fdc.floppy[i] && m_fdc.floppy[i]->exists())
				m_fdc.floppy[i]->mon_w(!BIT(data, 7));

		m_access_drv_out[x] = 0;
		if(x != m_fdc.select_drive)
			m_access_drv_out[m_fdc.select_drive] = 1;
		m_fdc.select_drive = x;
		LOGMASKED(LOG_FDC, "FDC: Drive #%i: Drive selection set to %02x\n",x,data);
		break;
		}
	}
}

uint16_t x68k_state::fdc_r(offs_t offset)
{
	unsigned int ret;
	int x;

	switch(offset)
	{
	case 0x00:
		ret = 0x00;
		for(x=0;x<4;x++)
		{
			if(BIT(m_fdc.control_drives, x))
			{
				ret = 0x00;
				if(m_fdc.floppy[x] && m_fdc.floppy[x]->exists())
				{
					ret |= 0x80;
				}
				// bit 7 = disk inserted
				// bit 6 = disk error (in insertion, presumably)
				LOGMASKED(LOG_FDC, "FDC: Drive #%i Disk check - returning %02x\n",x,ret);
			}
		}
		return ret;
	case 0x01:
		LOGMASKED(LOG_FDC, "FDC: IOC selection is write-only\n");
		return 0xff;
	}
	return 0xff;
}

void x68k_state::ct_w(uint8_t data)
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

	m_adpcm.clock = (data & 0x02) >> 1;
	set_adpcm();
	m_okim6258->set_unscaled_clock(adpcm_clock[m_adpcm.clock]);
}

enum ioc_irq_number : unsigned
{
	IOC_FDC_INT = 7,
	IOC_FDD_INT = 6,
	IOC_PRT_INT = 5,
	IOC_HDD_INT = 4,
	IOC_HDD_IEN = 3,
	IOC_FDC_IEN = 2,
	IOC_FDD_IEN = 1,
	IOC_PRT_IEN = 0,
};
template <unsigned N> void x68k_state::ioc_irq(int state)
{
	if (state)
		m_ioc.irqstatus |= 1U << N;
	else
		m_ioc.irqstatus &= ~(1U << N);

	bool const irq_state =
		(BIT(m_ioc.irqstatus, IOC_HDD_INT) && BIT(m_ioc.irqstatus, IOC_HDD_IEN)) ||
		(BIT(m_ioc.irqstatus, IOC_PRT_INT) && BIT(m_ioc.irqstatus, IOC_PRT_IEN)) ||
		(BIT(m_ioc.irqstatus, IOC_FDD_INT) && BIT(m_ioc.irqstatus, IOC_FDD_IEN)) ||
		(BIT(m_ioc.irqstatus, IOC_FDC_INT) && BIT(m_ioc.irqstatus, IOC_FDC_IEN));

	m_maincpu->set_input_line(INPUT_LINE_IRQ1, irq_state ? ASSERT_LINE : CLEAR_LINE);
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
void x68k_state::ioc_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
		m_ioc.irqstatus = (m_ioc.irqstatus & 0xf0) | (data & 0x0f);
		LOGMASKED(LOG_SYS, "I/O: Status register write %02x\n",data);
		break;
	case 0x01:
		LOGMASKED(LOG_IRQ, "IOC: IRQ vector = 0x%02x\n", data & 0xfc);
		m_ioc.vector = data & 0xfc;
		break;
	}
}

uint8_t x68k_state::ioc_r(offs_t offset)
{
	switch(offset)
	{
	case 0x00:
		LOGMASKED(LOG_SYS, "I/O: Status register read\n");
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
void x68k_state::sysport_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
//      LOGMASKED(LOG_SYS, "SYS: [%08x] Wrote %04x to invalid or unimplemented system port %04x\n",m_maincpu->pc(),data,offset);
		break;
	}
}

uint16_t x68k_state::sysport_r(offs_t offset)
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
		LOGMASKED(LOG_SYS, "Read from invalid or unimplemented system port %04x\n",offset);
		return 0xff;
	}
}

void x68k_state::ppi_w(offs_t offset, uint16_t data)
{
	m_ppi->write(offset & 0x03,data);
}

uint16_t x68k_state::ppi_r(offs_t offset)
{
	return m_ppi->read(offset & 0x03);
}


void x68k_state::sram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(m_sysport.sram_writeprotect == 0x31)
	{
		COMBINE_DATA(&m_nvram[offset]);
	}
}

uint16_t x68k_state::sram_r(offs_t offset)
{
	// HACKS!
//  if(offset == 0x5a/2)  // 0x5a should be 0 if no SASI HDs are present.
//      return 0x0000;
	if(offset == 0x08/2)
		return m_ram->size() >> 16;  // RAM size
	return m_nvram[offset];
}

void x68k_state::vid_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(offset < 0x80)
		COMBINE_DATA(m_video.reg);
	else if(offset < 0x100)
	{
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
	}
	else if(offset < 0x180)
		COMBINE_DATA(m_video.reg+2);
	else
		LOGMASKED(LOG_SYS, "VC: Invalid video controller write (offset = 0x%04x, data = %04x)\n",offset,data);
}

uint16_t x68k_state::vid_r(offs_t offset)
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
		LOGMASKED(LOG_SYS, "VC: Invalid video controller read (offset = 0x%04x)\n",offset);
	}

	return 0xff;
}

uint16_t x68k_state::areaset_r()
{
	// register is write-only
	return 0xffff;
}

void x68k_state::areaset_w(uint16_t data)
{
	// TODO
	LOGMASKED(LOG_SYS, "SYS: Supervisor area set: 0x%02x\n",data & 0xff);
}

void x68k_state::enh_areaset_w(offs_t offset, uint16_t data)
{
	// TODO
	LOGMASKED(LOG_SYS, "SYS: Enhanced Supervisor area set (from %iMB): 0x%02x\n",(offset + 1) * 2,data & 0xff);
}

TIMER_CALLBACK_MEMBER(x68k_state::bus_error)
{
	m_bus_error = false;
}

void x68k_state::set_bus_error(uint32_t address, bool rw, uint16_t mem_mask)
{
	LOGMASKED(LOG_SYS, "%s: Bus error: Unused RAM access [%08x]\n", machine().describe_context(), address);
	if(!m_maincpu->executing())
	{
		m_hd63450->bec_w(0, hd63450_device::ERR_BUS);
		return;
	}
	if(m_maincpu->type() == M68000) {
		downcast<m68000_device *>(m_maincpu.target())->trigger_bus_error();
		return;
	}

	if(m_bus_error)
		return;
	if(!ACCESSING_BITS_8_15)
		address++;
	m_bus_error = true;

	m68000_musashi_device *cpuptr = downcast<m68000_musashi_device *>(m_maincpu.target());
	cpuptr->set_buserror_details(address, rw, cpuptr->get_fc());
	cpuptr->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	cpuptr->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	m_bus_error_timer->adjust(cpuptr->cycles_to_attotime(16)); // let rmw cycles complete
}

uint16_t x68k_state::rom0_r(offs_t offset, uint16_t mem_mask)
{
	/* this location contains the address of some expansion device ROM, if no ROM exists,
	   then access causes a bus error */
	if((m_options->read() & 0x02) && !machine().side_effects_disabled())
		set_bus_error((offset << 1) + 0xbffffc, true, mem_mask);
	return 0xff;
}

void x68k_state::rom0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* this location contains the address of some expansion device ROM, if no ROM exists,
	   then access causes a bus error */
	if((m_options->read() & 0x02) && !machine().side_effects_disabled())
		set_bus_error((offset << 1) + 0xbffffc, false, mem_mask);
}

uint16_t x68k_state::emptyram_r(offs_t offset, uint16_t mem_mask)
{
	/* this location is unused RAM, access here causes a bus error
	   Often a method for detecting amount of installed RAM, is to read or write at 1MB intervals, until a bus error occurs */
	if((m_options->read() & 0x02) && !machine().side_effects_disabled())
		set_bus_error((offset << 1), 0, mem_mask);
	return 0xff;
}

void x68k_state::emptyram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* this location is unused RAM, access here causes a bus error
	   Often a method for detecting amount of installed RAM, is to read or write at 1MB intervals, until a bus error occurs */
	if((m_options->read() & 0x02) && !machine().side_effects_disabled())
		set_bus_error((offset << 1), 1, mem_mask);
}

uint16_t x68k_state::exp_r(offs_t offset, uint16_t mem_mask)
{
	/* These are expansion devices, if not present, they cause a bus error */
	if((m_options->read() & 0x02) && !machine().side_effects_disabled())
		set_bus_error((offset << 1) + 0xeafa00, 0, mem_mask);
	return 0xff;
}

void x68k_state::exp_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* These are expansion devices, if not present, they cause a bus error */
	if((m_options->read() & 0x02) && !machine().side_effects_disabled())
		set_bus_error((offset << 1) + 0xeafa00, 1, mem_mask);
}

void x68k_state::dma_end(offs_t offset, uint8_t data)
{
	if(offset == 0)
	{
		m_fdc_tc->adjust(attotime::from_usec(1), 0, attotime::never);
	}
}

void x68k_state::fm_irq(int state)
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

void x68k_state::adpcm_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
		m_okim6258->ctrl_w(data);
		break;
	case 0x01:
		m_okim6258->data_w(data);
		break;
	}
}

uint8_t x68k_state::iack1()
{
	uint8_t vector = 0x18;
	if (BIT(m_ioc.irqstatus, IOC_FDC_INT) && BIT(m_ioc.irqstatus, IOC_FDC_IEN))
	{
		vector = m_ioc.vector | 0;
		if (!machine().side_effects_disabled())
			ioc_irq<IOC_FDC_INT>(0);
	}
	else if (BIT(m_ioc.irqstatus, IOC_FDD_INT) && BIT(m_ioc.irqstatus, IOC_FDD_IEN))
	{
		vector = m_ioc.vector | 1;
		if (!machine().side_effects_disabled())
			ioc_irq<IOC_FDD_INT>(0);
	}
	else if (BIT(m_ioc.irqstatus, IOC_PRT_INT) && BIT(m_ioc.irqstatus, IOC_PRT_IEN))
	{
		vector = m_ioc.vector | 3;
		if (!machine().side_effects_disabled())
			ioc_irq<IOC_PRT_INT>(0);
	}
	else if (BIT(m_ioc.irqstatus, IOC_HDD_INT) && BIT(m_ioc.irqstatus, IOC_HDD_IEN))
	{
		// TODO: Internal SCSI IRQ vector 0x6c, External SCSI IRQ vector 0xf6 (really?)
		vector = m_ioc.vector | 2;
		if (!machine().side_effects_disabled())
			ioc_irq<IOC_HDD_INT>(0);
	}

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IRQ, "IOC: IRQ1 acknowledged with vector = %02X\n", vector);
	return vector;
}

template <int N>
void x68k_state::irq2_line(int state)
{
	m_exp_irq2[N] = (state != CLEAR_LINE);
	LOGMASKED(LOG_IRQ, "IRQ2-%d %s\n", N + 1, m_exp_irq2[N] ? "asserted" : "cleared");
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, (m_exp_irq2[0] || m_exp_irq2[1]) ? ASSERT_LINE : CLEAR_LINE);
}

template <int N>
void x68k_state::irq4_line(int state)
{
	m_exp_irq4[N] = (state != CLEAR_LINE);
	LOGMASKED(LOG_IRQ, "IRQ4-%d %s\n", N + 1, m_exp_irq4[N] ? "asserted" : "cleared");
	m_maincpu->set_input_line(INPUT_LINE_IRQ4, (m_exp_irq4[0] || m_exp_irq4[1]) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t x68k_state::iack2()
{
	// IACK2-1 has higher priority than IACK2-2
	if (m_exp_irq2[0])
		return m_expansion[0]->iack2();
	else if (m_exp_irq2[1])
		return m_expansion[1]->iack2();
	else
		return m68000_base_device::autovector(0); // spurious interrupt
}

uint8_t x68k_state::iack4()
{
	// IACK4-1 has higher priority than IACK4-2
	if (m_exp_irq4[0])
		return m_expansion[0]->iack4();
	else if (m_exp_irq4[1])
		return m_expansion[1]->iack4();
	else
		return m68000_base_device::autovector(0); // spurious interrupt
}

void x68k_state::cpu_space_map(address_map &map)
{
	map.global_mask(0xffffff);
	map(0xfffff3, 0xfffff3).r(FUNC(x68k_state::iack1));
	map(0xfffff5, 0xfffff5).r(FUNC(x68k_state::iack2));
	map(0xfffff7, 0xfffff7).r(m_hd63450, FUNC(hd63450_device::iack));
	map(0xfffff9, 0xfffff9).r(FUNC(x68k_state::iack4));
	map(0xfffffb, 0xfffffb).lr8(NAME([this]() { return m_scc->m1_r(); }));
	map(0xfffffd, 0xfffffd).r(m_mfpdev, FUNC(mc68901_device::get_vector));
	map(0xffffff, 0xffffff).lr8(NAME([] () { return m68000_base_device::autovector(7); }));
}

void x68ksupr_state::scsi_unknown_w(uint8_t data)
{
	// Documentation claims SSTS register is read-only, but x68030 boot code writes #$05 to this address anyway.
	// Is this an undocumented MB89352 feature, an ASIC register, an original code bug or a bad dump?
}

void x68k_state::x68k_base_map(address_map &map)
{
	map(0x000000, 0xbffffb).rw(FUNC(x68k_state::emptyram_r), FUNC(x68k_state::emptyram_w));
	map(0xbffffc, 0xbfffff).rw(FUNC(x68k_state::rom0_r), FUNC(x68k_state::rom0_w));
	map(0xc00000, 0xdfffff).rw(m_crtc, FUNC(x68k_crtc_device::gvram_r), FUNC(x68k_crtc_device::gvram_w));
	map(0xe00000, 0xe7ffff).rw(m_crtc, FUNC(x68k_crtc_device::tvram_r), FUNC(x68k_crtc_device::tvram_w));
	map(0xe80000, 0xe81fff).rw(m_crtc, FUNC(x68k_crtc_device::crtc_r), FUNC(x68k_crtc_device::crtc_w));
	map(0xe82400, 0xe83fff).rw(FUNC(x68k_state::vid_r), FUNC(x68k_state::vid_w));
	map(0xe84000, 0xe85fff).rw(m_hd63450, FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0xe86000, 0xe87fff).rw(FUNC(x68k_state::areaset_r), FUNC(x68k_state::areaset_w));
	map(0xe88000, 0xe89fff).rw(m_mfpdev, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xe8a000, 0xe8bfff).rw(m_rtc, FUNC(rp5c15_device::read), FUNC(rp5c15_device::write)).umask16(0x00ff);
//  map(0xe8c000, 0xe8dfff).rw(FUNC(x68k_state::x68k_printer_r), FUNC(x68k_state::x68k_printer_w));
	map(0xe8e000, 0xe8ffff).rw(FUNC(x68k_state::sysport_r), FUNC(x68k_state::sysport_w));
	map(0xe90000, 0xe91fff).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0xe94000, 0xe94003).m(m_upd72065, FUNC(upd72065_device::map)).umask16(0x00ff);
	map(0xe94004, 0xe94007).rw(FUNC(x68k_state::fdc_r), FUNC(x68k_state::fdc_w));
	map(0xe98000, 0xe99fff).rw(m_scc, FUNC(scc8530_device::ab_dc_r), FUNC(scc8530_device::ab_dc_w)).umask16(0x00ff);
	map(0xe9a000, 0xe9bfff).rw(FUNC(x68k_state::ppi_r), FUNC(x68k_state::ppi_w));
	map(0xe9c000, 0xe9dfff).rw(FUNC(x68k_state::ioc_r), FUNC(x68k_state::ioc_w)).umask16(0x00ff);
	map(0xe9e000, 0xe9e3ff).rw(FUNC(x68k_state::exp_r), FUNC(x68k_state::exp_w));  // FPU (Optional)
	map(0xeafa00, 0xeafa1f).rw(FUNC(x68k_state::exp_r), FUNC(x68k_state::exp_w));
	map(0xeb0000, 0xeb7fff).rw(FUNC(x68k_state::spritereg_r), FUNC(x68k_state::spritereg_w));
	map(0xeb8000, 0xebffff).rw(FUNC(x68k_state::spriteram_r), FUNC(x68k_state::spriteram_w));
	map(0xece000, 0xece3ff).rw(FUNC(x68k_state::exp_r), FUNC(x68k_state::exp_w));  // User I/O
	map(0xed0000, 0xed3fff).rw(FUNC(x68k_state::sram_r), FUNC(x68k_state::sram_w));
	map(0xed4000, 0xefffff).noprw();
	map(0xf00000, 0xfbffff).rom();
	map(0xfe0000, 0xffffff).rom();
}

void x68k_state::x68k_map(address_map &map)
{
	x68k_base_map(map);
	map(0xe82000, 0xe821ff).rw(m_gfxpalette, FUNC(palette_device::read16), FUNC(palette_device::write16)).share("gfxpalette");
	map(0xe82200, 0xe823ff).rw(m_pcgpalette, FUNC(palette_device::read16), FUNC(palette_device::write16)).share("pcgpalette");
	map(0xe92001, 0xe92001).rw(m_okim6258, FUNC(okim6258_device::status_r), FUNC(okim6258_device::ctrl_w));
	map(0xe92003, 0xe92003).rw(m_okim6258, FUNC(okim6258_device::status_r), FUNC(okim6258_device::data_w));
	map(0xe96000, 0xe9601f).rw("x68k_hdc", FUNC(x68k_hdc_image_device::hdc_r), FUNC(x68k_hdc_image_device::hdc_w));
	map(0xea0000, 0xea1fff).rw(FUNC(x68k_state::exp_r), FUNC(x68k_state::exp_w));  // external SCSI ROM and controller
	map(0xeafa80, 0xeafa89).rw(FUNC(x68k_state::areaset_r), FUNC(x68k_state::enh_areaset_w));
	map(0xfc0000, 0xfdffff).rw(FUNC(x68k_state::exp_r), FUNC(x68k_state::exp_w));  // internal SCSI ROM
}

void x68ksupr_state::x68kxvi_map(address_map &map)
{
	x68k_base_map(map);
	map(0xe82000, 0xe821ff).rw(m_gfxpalette, FUNC(palette_device::read16), FUNC(palette_device::write16)).share("gfxpalette");
	map(0xe82200, 0xe823ff).rw(m_pcgpalette, FUNC(palette_device::read16), FUNC(palette_device::write16)).share("pcgpalette");
	map(0xe92001, 0xe92001).rw(m_okim6258, FUNC(okim6258_device::status_r), FUNC(okim6258_device::ctrl_w));
	map(0xe92003, 0xe92003).rw(m_okim6258, FUNC(okim6258_device::status_r), FUNC(okim6258_device::data_w));
	map(0xe96020, 0xe9603f).m(m_scsictrl, FUNC(mb89352_device::map)).umask16(0x00ff);
	map(0xea0000, 0xea1fff).rw(FUNC(x68ksupr_state::exp_r), FUNC(x68ksupr_state::exp_w));  // external SCSI ROM and controller
	map(0xeafa80, 0xeafa89).rw(FUNC(x68ksupr_state::areaset_r), FUNC(x68ksupr_state::enh_areaset_w));
	map(0xfc0000, 0xfdffff).rom();  // internal SCSI ROM
}

void x68030_state::x68030_map(address_map &map)
{
	map.global_mask(0x00ffffff);  // Still only has 24-bit address space
	x68k_base_map(map);
	map(0xe82000, 0xe821ff).rw(m_gfxpalette, FUNC(palette_device::read32), FUNC(palette_device::write32)).share("gfxpalette");
	map(0xe82200, 0xe823ff).rw(m_pcgpalette, FUNC(palette_device::read32), FUNC(palette_device::write32)).share("pcgpalette");
//  map(0xe8c000, 0xe8dfff).rw(FUNC(x68k_state::x68k_printer_r), FUNC(x68k_state::x68k_printer_w));
	map(0xe92000, 0xe92003).r(m_okim6258, FUNC(okim6258_device::status_r)).umask32(0x00ff00ff).w(FUNC(x68030_state::adpcm_w)).umask32(0x00ff00ff);

	map(0xe96020, 0xe9603f).m(m_scsictrl, FUNC(mb89352_device::map)).umask32(0x00ff00ff);
	map(0xe9602d, 0xe9602d).w(FUNC(x68030_state::scsi_unknown_w));
	map(0xea0000, 0xea1fff).noprw();//.rw(FUNC(x68030_state::exp_r), FUNC(x68030_state::exp_w));  // external SCSI ROM and controller
	map(0xeafa80, 0xeafa8b).rw(FUNC(x68030_state::areaset_r), FUNC(x68030_state::enh_areaset_w));
	map(0xfc0000, 0xfdffff).rom();  // internal SCSI ROM
}

static INPUT_PORTS_START( x68000 )
// TODO: Sharp Cyber Stick (CZ-8NJ2) support

	PORT_START("options")
	PORT_CONFNAME( 0x02, 0x02, "Enable fake bus errors")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ))
	PORT_CONFSETTING(   0x02, DEF_STR( On ))
INPUT_PORTS_END

void x68k_state::floppy_load_unload(bool load, floppy_image_device *dev)
{
	dev->mon_w(!(m_fdc.motor && load));

	ioc_irq<IOC_FDD_INT>(1);
}

void x68k_state::floppy_load(floppy_image_device *dev)
{
	floppy_load_unload(true, dev);
}

void x68k_state::floppy_unload(floppy_image_device *dev)
{
	floppy_load_unload(false, dev);
}

static void x68000_exp_cards(device_slot_interface &device)
{
	device.option_add("neptunex", X68K_NEPTUNEX);   // Neptune-X ethernet adapter (ISA NE2000 bridge)
	device.option_add("cz6bs1", X68K_SCSIEXT);      // Sharp CZ-6BS1 SCSI-1 controller
	device.option_add("x68k_midi", X68K_MIDI);      // X68000 MIDI interface
}

void x68k_state::machine_reset()
{
	/* The last half of the IPLROM is mapped to 0x000000 on reset only
	   Just copying the initial stack pointer and program counter should
	   more or less do the same job */

	uint8_t *const romdata = memregion("user2")->base();

	memset(m_ram->pointer(),0,m_ram->size());
	memcpy(m_ram->pointer(),romdata,8);

	/// TODO: get callbacks to trigger these
	m_mfpdev->i0_w(1); // alarm
	m_mfpdev->i1_w(1); // expon
	m_mfpdev->i2_w(0); // pow sw
	m_mfpdev->i3_w(1); // fmirq
	//m_mfpdev->i4_w(1); // v-disp
	m_mfpdev->i5_w(1); // unused (always set)
	//m_mfpdev->i6_w(1); // cirq
	//m_mfpdev->i7_w(1); // h-sync

	// reset output values
	std::fill(std::begin(m_eject_drv_out), std::end(m_eject_drv_out), 1);
	std::fill(std::begin(m_ctrl_drv_out), std::end(m_ctrl_drv_out), 1);
	std::fill(std::begin(m_access_drv_out), std::end(m_access_drv_out), 1);
	m_fdc.select_drive = 0;
}

void x68k_state::machine_start()
{
	// resolve outputs
	m_eject_drv_out.resolve();
	m_ctrl_drv_out.resolve();
	m_access_drv_out.resolve();

	address_space &space = m_maincpu->space(AS_PROGRAM);
	// install RAM handlers
	m_spriteram = (uint16_t*)(memregion("user1")->base());
	space.install_ram(0x000000,m_ram->size()-1,m_ram->pointer());

	// start LED timer
	m_led_timer->adjust(attotime::zero, 0, attotime::from_msec(400));

	for(int drive=0;drive<4;drive++)
	{
		char devname[16];
		sprintf(devname, "%d", drive);
		floppy_image_device *floppy = m_upd72065->subdevice<floppy_connector>(devname)->get_device();
		m_fdc.floppy[drive] = floppy;
		if(floppy) {
			floppy->setup_load_cb(floppy_image_device::load_cb(&x68k_state::floppy_load, this));
			floppy->setup_unload_cb(floppy_image_device::unload_cb(&x68k_state::floppy_unload, this));
		}
	}
	m_fdc.motor = 0;

	m_exp_irq2[0] = m_exp_irq2[1] = false;
	m_exp_irq4[0] = m_exp_irq4[1] = false;
	m_ioc.irqstatus = 0;
	m_adpcm.rate = 0;
	m_adpcm.clock = 0;
	m_sysport.sram_writeprotect = 0;
	m_sysport.monitor = 0;
	m_bus_error = false;
	m_led_state = 0;
}

void x68k_state::driver_start()
{
	unsigned char* rom = memregion("maincpu")->base();
	unsigned char* user2 = memregion("user2")->base();

	subdevice<nvram_device>("nvram")->set_base(&m_nvram[0], m_nvram.size()*sizeof(m_nvram[0]));

#ifdef USE_PREDEFINED_SRAM
	{
		unsigned char* ramptr = memregion("user3")->base();
		memcpy(m_sram,ramptr,0x4000);
	}
#endif

	// copy last half of BIOS to a user region, to use for initial startup
	memcpy(user2,(rom+0xff0000),0x10000);

	m_led_timer = timer_alloc(FUNC(x68ksupr_state::led_callback), this);
	m_fdc_tc = timer_alloc(FUNC(x68ksupr_state::floppy_tc_tick), this);
	m_adpcm_timer = timer_alloc(FUNC(x68ksupr_state::adpcm_drq_tick), this);
	m_bus_error_timer = timer_alloc(FUNC(x68ksupr_state::bus_error), this);

	m_sysport.cputype = 0xff;  // 68000, 10MHz
	m_is_32bit = false;

	save_item(NAME(m_tvram));
	save_item(NAME(m_gvram));
	save_item(NAME(m_spritereg));
}

void x68ksupr_state::driver_start()
{
	x68k_state::driver_start();
	m_sysport.cputype = 0xfe; // 68000, 16MHz
	m_is_32bit = false;
}

void x68030_state::driver_start()
{
	x68k_state::driver_start();
	m_sysport.cputype = 0xdc; // 68030, 25MHz
	m_is_32bit = true;
}

void x68k_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_XDF_FORMAT);
	fr.add(FLOPPY_DIM_FORMAT);
}

static void x68k_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

static void keyboard_devices(device_slot_interface &device)
{
	device.option_add("x68k", X68K_KEYBOARD);
}

static void mouse_devices(device_slot_interface &device)
{
	device.option_add("x68k", X68K_MOUSE);
}

void x68k_state::x68000_base(machine_config &config)
{
	config.set_maximum_quantum(attotime::from_hz(60));

	/* device hardware */
	MC68901(config, m_mfpdev, 16_MHz_XTAL / 4);
	m_mfpdev->set_timer_clock(16_MHz_XTAL / 4);
	m_mfpdev->out_irq_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
	m_mfpdev->out_tbo_cb().set(m_mfpdev, FUNC(mc68901_device::tc_w));
	m_mfpdev->out_tbo_cb().append(m_mfpdev, FUNC(mc68901_device::rc_w));
	m_mfpdev->out_so_cb().set("keyboard", FUNC(rs232_port_device::write_txd));

	rs232_port_device &keyboard(RS232_PORT(config, "keyboard", keyboard_devices, "x68k"));
	keyboard.rxd_handler().set(m_mfpdev, FUNC(mc68901_device::si_w));

	MSX_GENERAL_PURPOSE_PORT(config, m_joy[0], msx_general_purpose_port_devices, "townspad");
	MSX_GENERAL_PURPOSE_PORT(config, m_joy[1], msx_general_purpose_port_devices, "townspad");

	I8255A(config, m_ppi, 0);
	m_ppi->in_pa_callback().set(FUNC(x68k_state::ppi_port_a_r));
	m_ppi->in_pb_callback().set(FUNC(x68k_state::ppi_port_b_r));
	m_ppi->in_pc_callback().set(FUNC(x68k_state::ppi_port_c_r));
	m_ppi->out_pc_callback().set(FUNC(x68k_state::ppi_port_c_w));

	HD63450(config, m_hd63450, 40_MHz_XTAL / 4, "maincpu");
	m_hd63450->set_clocks(attotime::from_usec(2), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625/2));
	m_hd63450->set_burst_clocks(attotime::from_usec(2), attotime::from_nsec(450), attotime::from_nsec(450), attotime::from_nsec(50));
	m_hd63450->irq_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	m_hd63450->dma_end().set(FUNC(x68k_state::dma_end));
	m_hd63450->dma_read<0>().set("upd72065", FUNC(upd72065_device::dma_r));
	m_hd63450->dma_write<0>().set("upd72065", FUNC(upd72065_device::dma_w));

	SCC8530(config, m_scc, 40_MHz_XTAL / 8);
	m_scc->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ5);

	rs232_port_device &mouse(RS232_PORT(config, "mouse_port", mouse_devices, "x68k"));
	mouse.rxd_handler().set(m_scc, FUNC(scc8530_device::rxb_w));
	m_scc->out_rtsb_callback().set(mouse, FUNC(rs232_port_device::write_rts));

	RP5C15(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->alarm().set(m_mfpdev, FUNC(mc68901_device::i0_w));
	m_rtc->set_year_offset(20);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(69.55199_MHz_XTAL / 2, 1096, 0, 768, 568, 0, 512);  // initial setting
	m_screen->set_screen_update(FUNC(x68k_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "pcgpalette", gfxdecode_device::empty);

	PALETTE(config, m_gfxpalette).set_format(2, &x68k_state::GGGGGRRRRRBBBBBI, 256);
	PALETTE(config, m_pcgpalette).set_format(2, &x68k_state::GGGGGRRRRRBBBBBI, 256);

	config.set_default_layout(layout_x68000);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
	YM2151(config, m_ym2151, 16_MHz_XTAL / 4);
	m_ym2151->irq_handler().set(FUNC(x68k_state::fm_irq));
	m_ym2151->port_write_handler().set(FUNC(x68k_state::ct_w));  // CT1, CT2 from YM2151 port 0x1b
	m_ym2151->add_route(0, "speaker", 0.50, 0);
	m_ym2151->add_route(1, "speaker", 0.50, 1);

	OKIM6258(config, m_okim6258, 16_MHz_XTAL / 4);
	m_okim6258->set_start_div(okim6258_device::FOSC_DIV_BY_512);
	m_okim6258->set_type(okim6258_device::TYPE_4BITS);
	m_okim6258->set_outbits(okim6258_device::OUTPUT_10BITS);
	m_okim6258->add_route(ALL_OUTPUTS, "adpcm_outl", 0.50);
	m_okim6258->add_route(ALL_OUTPUTS, "adpcm_outr", 0.50);

	FILTER_VOLUME(config, m_adpcm_out[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	FILTER_VOLUME(config, m_adpcm_out[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	UPD72065(config, m_upd72065, 16_MHz_XTAL / 2, true, false); // clocked through SED9420CAC
	m_upd72065->intrq_wr_callback().set(FUNC(x68k_state::ioc_irq<IOC_FDC_INT>));
	m_upd72065->drq_wr_callback().set(m_hd63450, FUNC(hd63450_device::drq0_w));
	FLOPPY_CONNECTOR(config, "upd72065:0", x68k_floppies, "525hd", x68k_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd72065:1", x68k_floppies, "525hd", x68k_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd72065:2", x68k_floppies, "525hd", x68k_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd72065:3", x68k_floppies, "525hd", x68k_state::floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("x68k_flop");

	input_merger_any_high_device &nmi(INPUT_MERGER_ANY_HIGH(config, "nmi"));
	nmi.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ7);

	X68K_EXPANSION_SLOT(config, m_expansion[0], x68000_exp_cards, nullptr);
	m_expansion[0]->set_space(m_maincpu, AS_PROGRAM);
	m_expansion[0]->out_irq2_callback().set(FUNC(x68k_state::irq2_line<0>));
	m_expansion[0]->out_irq4_callback().set(FUNC(x68k_state::irq4_line<0>));
	m_expansion[0]->out_nmi_callback().set(nmi, FUNC(input_merger_any_high_device::in_w<0>));
	m_expansion[0]->out_dtack_callback().set(m_hd63450, FUNC(hd63450_device::dtack_w));
	m_hd63450->own().append(m_expansion[0], FUNC(x68k_expansion_slot_device::exown_w));

	X68K_EXPANSION_SLOT(config, m_expansion[1], x68000_exp_cards, nullptr);
	m_expansion[1]->set_space(m_maincpu, AS_PROGRAM);
	m_expansion[1]->out_irq2_callback().set(FUNC(x68k_state::irq2_line<1>));
	m_expansion[1]->out_irq4_callback().set(FUNC(x68k_state::irq4_line<1>));
	m_expansion[1]->out_nmi_callback().set(nmi, FUNC(input_merger_any_high_device::in_w<1>));
	m_expansion[1]->out_dtack_callback().set(m_hd63450, FUNC(hd63450_device::dtack_w));
	m_hd63450->own().append(m_expansion[1], FUNC(x68k_expansion_slot_device::exown_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("4M").set_extra_options("1M,2M,3M,5M,6M,7M,8M,9M,10M,11M,12M");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void x68k_state::x68000(machine_config &config)
{
	add_cpu(config, M68000, &x68k_state::x68k_map, 40_MHz_XTAL / 4);
	x68000_base(config);

	VINAS(config, m_crtc, 38.86363_MHz_XTAL);
	m_crtc->set_clock_69m(69.55199_MHz_XTAL);
	m_crtc->set_screen("screen");
	m_crtc->vdisp_cb().set(m_mfpdev, FUNC(mc68901_device::i4_w));
	m_crtc->vdisp_cb().append(m_mfpdev, FUNC(mc68901_device::tai_w));
	m_crtc->rint_cb().set(m_mfpdev, FUNC(mc68901_device::i6_w));
	m_crtc->hsync_cb().set(m_mfpdev, FUNC(mc68901_device::i7_w));
	m_crtc->tvram_read_cb().set(FUNC(x68k_state::tvram_read));
	m_crtc->tvram_write_cb().set(FUNC(x68k_state::tvram_write));
	m_crtc->gvram_read_cb().set(FUNC(x68k_state::gvram_read));
	m_crtc->gvram_write_cb().set(FUNC(x68k_state::gvram_write));

	X68KHDC(config, "x68k_hdc", 0);
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void x68ksupr_state::x68ksupr_base(machine_config &config)
{
	x68000_base(config);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("spc", MB89352).machine_config(
		[this](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(40_MHz_XTAL / 8);
			spc.out_irq_callback().set(*this, FUNC(x68ksupr_state::ioc_irq<IOC_HDD_INT>));
			// TODO: duplicate DMA glue from CZ-6BS1
		});

	VICON(config, m_crtc, 38.86363_MHz_XTAL);
	m_crtc->set_clock_69m(69.55199_MHz_XTAL);
	m_crtc->set_screen("screen");
	m_crtc->vdisp_cb().set(m_mfpdev, FUNC(mc68901_device::i4_w));
	m_crtc->vdisp_cb().append(m_mfpdev, FUNC(mc68901_device::tai_w));
	m_crtc->rint_cb().set(m_mfpdev, FUNC(mc68901_device::i6_w));
	m_crtc->hsync_cb().set(m_mfpdev, FUNC(mc68901_device::i7_w));
	m_crtc->tvram_read_cb().set(FUNC(x68ksupr_state::tvram_read));
	m_crtc->tvram_write_cb().set(FUNC(x68ksupr_state::tvram_write));
	m_crtc->gvram_read_cb().set(FUNC(x68ksupr_state::gvram_read));
	m_crtc->gvram_write_cb().set(FUNC(x68ksupr_state::gvram_write));
}

void x68ksupr_state::x68ksupr(machine_config &config)
{
	add_cpu(config, M68000, &x68ksupr_state::x68kxvi_map, 40_MHz_XTAL / 4);
	x68ksupr_base(config);
}

void x68ksupr_state::x68kxvi(machine_config &config)
{
	add_cpu(config, M68000, &x68ksupr_state::x68kxvi_map, 33.33_MHz_XTAL / 2); /* 16 MHz (nominally) */
	x68ksupr_base(config);
}

void x68030_state::x68030(machine_config &config)
{
	add_cpu(config, M68030, &x68030_state::x68030_map, 50_MHz_XTAL / 2);  /* 25 MHz 68EC030 */
	x68ksupr_base(config);

	m_hd63450->set_clock(50_MHz_XTAL / 4);
	m_scc->set_clock(20_MHz_XTAL / 4);
	//m_scsictrl->set_clock(20_MHz_XTAL / 4);

	m_crtc->set_clock_50m(50.35_MHz_XTAL);
}

ROM_START( x68000 )
	ROM_REGION16_BE(0x1000000, "maincpu", 0)  // 16MB address space
	ROM_DEFAULT_BIOS("cz600ce")
	ROM_LOAD( "cgrom.dat",  0xf00000, 0xc0000, CRC(9f3195f1) SHA1(8d72c5b4d63bb14c5dbdac495244d659aa1498b6) )
	ROM_SYSTEM_BIOS(0, "ipl10",  "IPL-ROM V1.0 (87/05/07)")
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "cz600ce",  "CZ-600CE IPL-ROM V1.0 (87/03/18)")
	ROMX_LOAD( "rh-ix0897cezz.ic12", 0xfe0000, 0x010000, CRC(cdc95995) SHA1(810cae207ffd29926e604cf1eb964ae8ea1fadb5), ROM_BIOS(4) | ROM_SKIP(1) )
	ROMX_LOAD( "rh-ix0898cezz.ic11", 0xfe0001, 0x010000, CRC(e60e09a8) SHA1(f3d4a6506493ea3ac7b9c8e441d781fbdd61abd5), ROM_BIOS(4) | ROM_SKIP(1) )
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
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "cz600ce",  "CZ-600CE IPL-ROM V1.0 (87/03/18)")
	ROMX_LOAD( "rh-ix0897cezz.ic12", 0xfe0000, 0x010000, CRC(cdc95995) SHA1(810cae207ffd29926e604cf1eb964ae8ea1fadb5), ROM_BIOS(4) | ROM_SKIP(1) )
	ROMX_LOAD( "rh-ix0898cezz.ic11", 0xfe0001, 0x010000, CRC(e60e09a8) SHA1(f3d4a6506493ea3ac7b9c8e441d781fbdd61abd5), ROM_BIOS(4) | ROM_SKIP(1) )
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
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "cz600ce",  "CZ-600CE IPL-ROM V1.0 (87/03/18)")
	ROMX_LOAD( "rh-ix0897cezz.ic12", 0xfe0000, 0x010000, CRC(cdc95995) SHA1(810cae207ffd29926e604cf1eb964ae8ea1fadb5), ROM_BIOS(4) | ROM_SKIP(1) )
	ROMX_LOAD( "rh-ix0898cezz.ic11", 0xfe0001, 0x010000, CRC(e60e09a8) SHA1(f3d4a6506493ea3ac7b9c8e441d781fbdd61abd5), ROM_BIOS(4) | ROM_SKIP(1) )
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
	ROMX_LOAD( "iplrom.dat", 0xfe0000, 0x20000, CRC(72bdf532) SHA1(0ed038ed2133b9f78c6e37256807424e0d927560), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ipl11",  "IPL-ROM V1.1 (91/01/11)")
	ROMX_LOAD( "iplromxv.dat", 0xfe0000, 0x020000, CRC(00eeb408) SHA1(e33cdcdb69cd257b0b211ef46e7a8b144637db57), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "ipl12",  "IPL-ROM V1.2 (91/10/24)")
	ROMX_LOAD( "iplromco.dat", 0xfe0000, 0x020000, CRC(6c7ef608) SHA1(77511fc58798404701f66b6bbc9cbde06596eba7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "ipl13",  "IPL-ROM V1.3 (92/11/27)")
	ROMX_LOAD( "iplrom30.dat", 0xfe0000, 0x020000, CRC(e8f8fdad) SHA1(239e9124568c862c31d9ec0605e32373ea74b86a), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "cz600ce",  "CZ-600CE IPL-ROM V1.0 (87/03/18)")
	ROMX_LOAD( "rh-ix0897cezz.ic12", 0xfe0000, 0x010000, CRC(cdc95995) SHA1(810cae207ffd29926e604cf1eb964ae8ea1fadb5), ROM_BIOS(4) | ROM_SKIP(1) )
	ROMX_LOAD( "rh-ix0898cezz.ic11", 0xfe0001, 0x010000, CRC(e60e09a8) SHA1(f3d4a6506493ea3ac7b9c8e441d781fbdd61abd5), ROM_BIOS(4) | ROM_SKIP(1) )
	ROM_LOAD("scsiinrom.dat",0xfc0000, 0x002000, CRC(1c6c889e) SHA1(3f063d4231cdf53da6adc4db96533725e260076a) BAD_DUMP )
//  ROM_LOAD("scsiexrom.dat",0xea0000, 0x002000, NO_DUMP )
	ROM_REGION(0x8000, "user1",0)  // For Background/Sprite decoding
	ROM_FILL(0x0000,0x8000,0x00)
	ROM_REGION(0x20000, "user2", 0)
	ROM_FILL(0x000,0x20000,0x00)
ROM_END


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS           INIT        COMPANY  FULLNAME        FLAGS
COMP( 1987, x68000,   0,      0,      x68000,   x68000, x68k_state,     empty_init, "Sharp", "X68000",       MACHINE_IMPERFECT_GRAPHICS )
COMP( 1990, x68ksupr, x68000, 0,      x68ksupr, x68000, x68ksupr_state, empty_init, "Sharp", "X68000 Super", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
COMP( 1991, x68kxvi,  x68000, 0,      x68kxvi,  x68000, x68ksupr_state, empty_init, "Sharp", "X68000 XVI",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
COMP( 1993, x68030,   x68000, 0,      x68030,   x68000, x68030_state,   empty_init, "Sharp", "X68030",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
