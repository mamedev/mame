// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple2.c - Apple II/II Plus and clones

    Next generation driver written in September/October 2014 by R. Belmont.
    Thanks to the original Apple II series driver's authors: Mike Balfour, Nathan Woods, and R. Belmont
    Special thanks to the Apple II Documentation Project/Antoine Vignau and Peter Ferrie.

II: original base model.  RAM sizes of 4, 8, 12, 16, 20, 24, 32, 36, and 48 KB possible.
    8K of ROM at $E000-$FFFF, empty sockets for $D000-$D7FF and $D800-$DFFF.
    Programmer's Aid #1 was sold by Apple for $D000-$D7FF, some third-party ROMs
    were also available.

    Revision 0 (very rare) had only 4 hi-res colors (blue and orange were missing).
    Revision 0 boards also did not include a color killer in text mode, making text
    fringey on color TVs/monitors.

    ROM contains original non-autostart Monitor and Integer BASIC; apparently
    Autostart + Integer is also possible.

II Plus: RAM options reduced to 16/32/48 KB.
    ROM expanded to 12KB from $D000-$FFFF containing Applesoft BASIC and
    the Autostart Monitor.  Applesoft is a licensed version of Microsoft's
    6502 BASIC as also found in Commodore and many other computers.


    Users of both models often connected the SHIFT key to the paddle #2 button
    (mapped to $C063) in order to inform properly written software that characters
    were to be intended upper/lower case.

    Both models commonly included a RAM "language card" in slot 0 which added 16K
    of RAM which could be banked into the $D000-$FFFF space to replace the ROMs.
    This allowed running Applesoft on a II and Integer BASIC on a II Plus.
    A II Plus with this card installed is often called a "64K Apple II"; this is
    the base configuration required to run ProDOS and some larger games.

************************************************************************/

#include "emu.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/kb3600.h"
#include "sound/speaker.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "formats/ap2_dsk.h"
#include "cpu/m6502/m6502.h"
#include "video/apple2.h"

#include "bus/a2bus/a2bus.h"
#include "bus/a2bus/a2diskii.h"
#include "bus/a2bus/a2diskiing.h"
#include "bus/a2bus/a2mockingboard.h"
#include "bus/a2bus/a2cffa.h"
#include "bus/a2bus/a2memexp.h"
#include "bus/a2bus/a2scsi.h"
#include "bus/a2bus/a2thunderclock.h"
#include "bus/a2bus/a2softcard.h"
#include "bus/a2bus/a2videoterm.h"
#include "bus/a2bus/a2ssc.h"
#include "bus/a2bus/a2swyft.h"
#include "bus/a2bus/a2themill.h"
#include "bus/a2bus/a2sam.h"
#include "bus/a2bus/a2alfam2.h"
#include "bus/a2bus/laser128.h"
#include "bus/a2bus/a2echoii.h"
#include "bus/a2bus/a2arcadebd.h"
#include "bus/a2bus/a2midi.h"
#include "bus/a2bus/a2zipdrive.h"
#include "bus/a2bus/a2applicard.h"
#include "bus/a2bus/a2ultraterm.h"
#include "bus/a2bus/a2pic.h"
#include "bus/a2bus/a2corvus.h"
#include "bus/a2bus/a2mcms.h"
#include "bus/a2bus/a2dx1.h"
#include "bus/a2bus/timemasterho.h"
#include "bus/a2bus/mouse.h"
#include "bus/a2bus/ramcard16k.h"
#include "bus/a2bus/ramcard128k.h"
#include "bus/a2bus/ezcgi.h"

#define A2_CPU_TAG "maincpu"
#define A2_KBDC_TAG "ay3600"
#define A2_BUS_TAG "a2bus"
#define A2_SPEAKER_TAG "speaker"
#define A2_CASSETTE_TAG "tape"
#define A2_UPPERBANK_TAG "inhbank"
#define A2_VIDEO_TAG "a2video"

class napple2_state : public driver_device
{
public:
	napple2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, A2_CPU_TAG),
		m_ram(*this, RAM_TAG),
		m_ay3600(*this, A2_KBDC_TAG),
		m_video(*this, A2_VIDEO_TAG),
		m_a2bus(*this, A2_BUS_TAG),
		m_joy1x(*this, "joystick_1_x"),
		m_joy1y(*this, "joystick_1_y"),
		m_joy2x(*this, "joystick_2_x"),
		m_joy2y(*this, "joystick_2_y"),
		m_joybuttons(*this, "joystick_buttons"),
		m_kbspecial(*this, "keyb_special"),
		m_kbrepeat(*this, "keyb_repeat"),
		m_resetdip(*this, "reset_dip"),
		m_sysconfig(*this, "a2_config"),
		m_speaker(*this, A2_SPEAKER_TAG),
		m_cassette(*this, A2_CASSETTE_TAG),
		m_upperbank(*this, A2_UPPERBANK_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<ay3600_device> m_ay3600;
	required_device<a2_video_device> m_video;
	required_device<a2bus_device> m_a2bus;
	required_ioport m_joy1x, m_joy1y, m_joy2x, m_joy2y, m_joybuttons;
	required_ioport m_kbspecial;
	required_ioport m_kbrepeat;
	optional_ioport m_resetdip;
	required_ioport m_sysconfig;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<address_map_bank_device> m_upperbank;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(ay3600_repeat);

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_PALETTE_INIT(apple2);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(c000_r);
	DECLARE_WRITE8_MEMBER(c000_w);
	DECLARE_READ8_MEMBER(c080_r);
	DECLARE_WRITE8_MEMBER(c080_w);
	DECLARE_READ8_MEMBER(c100_r);
	DECLARE_WRITE8_MEMBER(c100_w);
	DECLARE_READ8_MEMBER(c800_r);
	DECLARE_WRITE8_MEMBER(c800_w);
	DECLARE_READ8_MEMBER(inh_r);
	DECLARE_WRITE8_MEMBER(inh_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_inh_w);
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);
	DECLARE_WRITE_LINE_MEMBER(ay3600_ako_w);

private:
	int m_speaker_state;
	int m_cassette_state;

	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;

	UINT16 m_lastchar, m_strobe;
	UINT8 m_transchar;
	bool m_anykeydown;

	int m_inh_slot;
	int m_cnxx_slot;

	bool m_page2;
	bool m_an0, m_an1, m_an2, m_an3;

	UINT8 *m_ram_ptr;
	int m_ram_size;

	int m_inh_bank;

	double m_x_calibration, m_y_calibration;

	device_a2bus_card_interface *m_slotdevice[8];

	void do_io(address_space &space, int offset);
	UINT8 read_floatingbus();
};

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define JOYSTICK_DELTA          80
#define JOYSTICK_SENSITIVITY    50
#define JOYSTICK_AUTOCENTER     80

WRITE_LINE_MEMBER(napple2_state::a2bus_irq_w)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, state);
}

WRITE_LINE_MEMBER(napple2_state::a2bus_nmi_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

// This code makes a ton of assumptions because we can guarantee a pre-IIe machine!
WRITE_LINE_MEMBER(napple2_state::a2bus_inh_w)
{
	if (state == ASSERT_LINE)
	{
		// assume no cards are pulling /INH
		m_inh_slot = -1;

		// scan the slots to figure out which card(s) are INHibiting stuff
		for (int i = 0; i <= 7; i++)
		{
			if (m_slotdevice[i])
			{
				// this driver only can inhibit from 0xd000-0xffff
				if ((m_slotdevice[i]->inh_start() == 0xd000) &&
					(m_slotdevice[i]->inh_end() == 0xffff))
				{
					if ((m_slotdevice[i]->inh_type() & INH_READ) == INH_READ)
					{
						if (m_inh_bank != 1)
						{
							m_upperbank->set_bank(1);
							m_inh_bank = 1;
						}
					}
					else
					{
						if (m_inh_bank != 0)
						{
							m_upperbank->set_bank(0);
							m_inh_bank = 0;
						}
					}

					m_inh_slot = i;
					break;
				}
			}
		}

		// if no slots are inhibiting, make sure ROM is fully switched in
		if ((m_inh_slot == -1) && (m_inh_bank != 0))
		{
			m_upperbank->set_bank(0);
			m_inh_bank = 0;
		}
	}
}

/***************************************************************************
    START/RESET
***************************************************************************/

void napple2_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);
	m_cassette_state = 0;
	m_cassette->output(-1.0f);
	m_upperbank->set_bank(0);
	m_inh_bank = 0;

	// precalculate joystick time constants
	m_x_calibration = attotime::from_usec(12).as_double();
	m_y_calibration = attotime::from_usec(13).as_double();

	// cache slot devices
	for (int i = 0; i <= 7; i++)
	{
		m_slotdevice[i] = m_a2bus->get_a2bus_card(i);
	}

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_cassette_state));
	save_item(NAME(m_joystick_x1_time));
	save_item(NAME(m_joystick_y1_time));
	save_item(NAME(m_joystick_x2_time));
	save_item(NAME(m_joystick_y2_time));
	save_item(NAME(m_lastchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_transchar));
	save_item(NAME(m_inh_slot));
	save_item(NAME(m_inh_bank));
	save_item(NAME(m_cnxx_slot));
	save_item(NAME(m_page2));
	save_item(NAME(m_an0));
	save_item(NAME(m_an1));
	save_item(NAME(m_an2));
	save_item(NAME(m_an3));
	save_item(NAME(m_anykeydown));

	// setup video pointers
	m_video->m_ram_ptr = m_ram_ptr;
	m_video->m_aux_ptr = m_ram_ptr;
	m_video->m_char_ptr = memregion("gfx1")->base();
	m_video->m_char_size = memregion("gfx1")->bytes();
}

void napple2_state::machine_reset()
{
	m_inh_slot = -1;
	m_cnxx_slot = -1;
	m_page2 = false;
	m_an0 = m_an1 = m_an2 = m_an3 = false;
	m_anykeydown = false;
}

/***************************************************************************
    VIDEO
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(napple2_state::apple2_interrupt)
{
	int scanline = param;

	if((scanline % 8) == 0)
		machine().first_screen()->update_partial(machine().first_screen()->vpos());

	// update the video system's shadow copy of the system config at the end of the frame
	if (scanline == 192)
	{
		m_video->m_sysconfig = m_sysconfig->read();

		// check reset
		if (m_resetdip) // if reset DIP is present, use it
		{
			if (m_resetdip->read() & 1)
			{       // CTRL-RESET
				if ((m_kbspecial->read() & 0x88) == 0x88)
				{
					m_maincpu->reset();
				}
			}
			else    // plain RESET
			{
				if (m_kbspecial->read() & 0x80)
				{
					m_maincpu->reset();
				}
			}
		}
		else    // no DIP, so always plain RESET
		{
			if (m_kbspecial->read() & 0x80)
			{
				m_maincpu->reset();
			}
		}
	}
}

PALETTE_INIT_MEMBER(napple2_state, apple2)
{
	m_video->palette_init_apple2(palette);
}

UINT32 napple2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// always update the flash timer here so it's smooth regardless of mode switches
	m_video->m_flash = ((machine().time() * 4).seconds() & 1) ? true : false;

	if (m_video->m_graphics)
	{
		if (m_video->m_hires)
		{
			if (m_video->m_mix)
			{
				m_video->hgr_update(screen, bitmap, cliprect, 0, 159);
				if (!strcmp(machine().system().name, "ivelultr"))
				{
					m_video->text_update_ultr(screen, bitmap, cliprect, 160, 191);
				}
				else
				{
					m_video->text_update_orig(screen, bitmap, cliprect, 160, 191);
				}
			}
			else
			{
				m_video->hgr_update(screen, bitmap, cliprect, 0, 191);
			}
		}
		else    // lo-res
		{
			if (m_video->m_mix)
			{
				m_video->lores_update(screen, bitmap, cliprect, 0, 159);
				if (!strcmp(machine().system().name, "ivelultr"))
				{
					m_video->text_update_ultr(screen, bitmap, cliprect, 160, 191);
				}
				else
				{
					m_video->text_update_orig(screen, bitmap, cliprect, 160, 191);
				}
			}
			else
			{
				m_video->lores_update(screen, bitmap, cliprect, 0, 191);
			}
		}
	}
	else
	{
		if (!strcmp(machine().system().name, "ivelultr"))
		{
			m_video->text_update_ultr(screen, bitmap, cliprect, 0, 191);
		}
		else
		{
			m_video->text_update_orig(screen, bitmap, cliprect, 0, 191);
		}
	}

	return 0;
}

/***************************************************************************
    I/O
***************************************************************************/
// most softswitches don't care about read vs write, so handle them here
void napple2_state::do_io(address_space &space, int offset)
{
	if(space.debugger_access())
	{
		return;
	}

	switch (offset)
	{
		case 0x20:
			m_cassette_state ^= 1;
			m_cassette->output(m_cassette_state ? 1.0f : -1.0f);
			break;

		case 0x30:
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x50:  // graphics mode
			m_video->m_graphics = true; break;

		case 0x51:  // text mode
			m_video->m_graphics = false; break;

		case 0x52:  // no mix
			m_video->m_mix = false; break;

		case 0x53:  // mixed mode
			m_video->m_mix = true; break;

		case 0x54:  // set page 1
			m_page2 = false;
			m_video->m_page2 = false;
			break;

		case 0x55:  // set page 2
			m_page2 = true;
			m_video->m_page2 = true;
			break;

		case 0x56: // select lo-res
			m_video->m_hires = false; break;

		case 0x57: // select hi-res
			m_video->m_hires = true; break;

		case 0x58: // AN0 off
			m_an0 = false; break;

		case 0x59: // AN0 on
			m_an0 = true; break;

		case 0x5a: // AN1 off
			m_an1 = false; break;

		case 0x5b: // AN1 on
			m_an1 = true; break;

		case 0x5c: // AN2 off
			m_an2 = false; break;

		case 0x5d: // AN2 on
			m_an2 = true; break;

		case 0x5e: // AN3 off
			m_an3 = false; break;

		case 0x5f: // AN3 on
			m_an3 = true; break;

		case 0x68:  // IIgs STATE register, which ProDOS touches
			break;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_joy1x->read();
			m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_joy1y->read();
			m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_joy2x->read();
			m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_joy2y->read();
			break;
	}
}

READ8_MEMBER(napple2_state::c000_r)
{
	switch (offset)
	{
		case 0x00:  // keyboard latch
			return m_transchar | m_strobe;

		case 0x10:  // reads any key down, clears strobe
			{
				UINT8 rv = m_transchar | (m_anykeydown ? 0x80 : 0x00);
				m_strobe = 0;
				return rv;
			}

		case 0x60: // cassette in
		case 0x68:
			return m_cassette->input() > 0.0 ? 0x80 : 0;

		case 0x61:  // button 0
		case 0x69:
			return (m_joybuttons->read() & 0x10) ? 0x80 : 0;

		case 0x62:  // button 1
		case 0x6a:
			return (m_joybuttons->read() & 0x20) ? 0x80 : 0;

		case 0x63:  // button 2
		case 0x6b:
			// check if SHIFT key mod configured
			if (m_sysconfig->read() & 0x04)
			{
				return ((m_joybuttons->read() & 0x40) || (m_kbspecial->read() & 0x06)) ? 0x80 : 0;
			}
			return (m_joybuttons->read() & 0x40) ? 0x80 : 0;

		case 0x64:  // joy 1 X axis
		case 0x6c:
			return (space.machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0;

		case 0x65:  // joy 1 Y axis
		case 0x6d:
			return (space.machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0;

		case 0x66: // joy 2 X axis
		case 0x6e:
			return (space.machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0;

		case 0x67: // joy 2 Y axis
		case 0x6f:
			return (space.machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0;

		default:
			do_io(space, offset);
			break;
	}

	return read_floatingbus();
}

WRITE8_MEMBER(napple2_state::c000_w)
{
	switch (offset)
	{
		case 0x10:  // clear keyboard latch
			m_strobe = 0;
			break;

		default:
			do_io(space, offset);
			break;
	}
}

READ8_MEMBER(napple2_state::c080_r)
{
	if(!space.debugger_access())
	{
		int slot;

		offset &= 0x7F;
		slot = offset / 0x10;

		if (m_slotdevice[slot] != NULL)
		{
			return m_slotdevice[slot]->read_c0nx(space, offset % 0x10);
		}
	}

	return read_floatingbus();
}

WRITE8_MEMBER(napple2_state::c080_w)
{
	int slot;

	offset &= 0x7F;
	slot = offset / 0x10;

	if (m_slotdevice[slot] != NULL)
	{
		m_slotdevice[slot]->write_c0nx(space, offset % 0x10, data);
	}
}

READ8_MEMBER(napple2_state::c100_r)
{
	int slotnum;

	slotnum = ((offset>>8) & 0xf) + 1;

	if (m_slotdevice[slotnum] != NULL)
	{
		if ((m_slotdevice[slotnum]->take_c800()) && (!space.debugger_access()))
		{
			m_cnxx_slot = slotnum;
		}

		return m_slotdevice[slotnum]->read_cnxx(space, offset&0xff);
	}

	return read_floatingbus();
}

WRITE8_MEMBER(napple2_state::c100_w)
{
	int slotnum;

	slotnum = ((offset>>8) & 0xf) + 1;

	if (m_slotdevice[slotnum] != NULL)
	{
		if ((m_slotdevice[slotnum]->take_c800()) && (!space.debugger_access()))
		{
			m_cnxx_slot = slotnum;
		}

		m_slotdevice[slotnum]->write_cnxx(space, offset&0xff, data);
	}
}

READ8_MEMBER(napple2_state::c800_r)
{
	if (offset == 0x7ff)
	{
		m_cnxx_slot = -1;
		return 0xff;
	}

	if ((m_cnxx_slot != -1) && (m_slotdevice[m_cnxx_slot] != NULL))
	{
		return m_slotdevice[m_cnxx_slot]->read_c800(space, offset&0xfff);
	}

	return read_floatingbus();
}

WRITE8_MEMBER(napple2_state::c800_w)
{
	if (offset == 0x7ff)
	{
		m_cnxx_slot = -1;
		return;
	}

	if ((m_cnxx_slot != -1) && (m_slotdevice[m_cnxx_slot] != NULL))
	{
		m_slotdevice[m_cnxx_slot]->write_c800(space, offset&0xfff, data);
	}
}

READ8_MEMBER(napple2_state::inh_r)
{
	if (m_inh_slot != -1)
	{
		return m_slotdevice[m_inh_slot]->read_inh_rom(space, offset + 0xd000);
	}

	assert(0);  // hitting inh_r with invalid m_inh_slot should not be possible
	return read_floatingbus();
}

WRITE8_MEMBER(napple2_state::inh_w)
{
	if (m_inh_slot != -1)
	{
		m_slotdevice[m_inh_slot]->write_inh_rom(space, offset + 0xd000, data);
	}
}

// floating bus code from old machine/apple2: needs to be reworked based on real beam position to enable e.g. Bob Bishop's screen splitter
UINT8 napple2_state::read_floatingbus()
{
	enum
	{
		// scanner types
		kScannerNone = 0, kScannerApple2, kScannerApple2e,

		// scanner constants
		kHBurstClock      =    53, // clock when Color Burst starts
		kHBurstClocks     =     4, // clocks per Color Burst duration
		kHClock0State     =  0x18, // H[543210] = 011000
		kHClocks          =    65, // clocks per horizontal scan (including HBL)
		kHPEClock         =    40, // clock when HPE (horizontal preset enable) goes low
		kHPresetClock     =    41, // clock when H state presets
		kHSyncClock       =    49, // clock when HSync starts
		kHSyncClocks      =     4, // clocks per HSync duration
		kNTSCScanLines    =   262, // total scan lines including VBL (NTSC)
		kNTSCVSyncLine    =   224, // line when VSync starts (NTSC)
		kPALScanLines     =   312, // total scan lines including VBL (PAL)
		kPALVSyncLine     =   264, // line when VSync starts (PAL)
		kVLine0State      = 0x100, // V[543210CBA] = 100000000
		kVPresetLine      =   256, // line when V state presets
		kVSyncLines       =     4, // lines per VSync duration
		kClocksPerVSync   = kHClocks * kNTSCScanLines // FIX: NTSC only?
	};

	// vars
	//
	int i, Hires, Mixed, Page2, _80Store, ScanLines, /* VSyncLine, ScanCycles,*/
		h_clock, h_state, h_0, h_1, h_2, h_3, h_4, h_5,
		v_line, v_state, v_A, v_B, v_C, v_0, v_1, v_2, v_3, v_4, /* v_5, */
		_hires, addend0, addend1, addend2, sum, address;

	// video scanner data
	//
	i = m_maincpu->total_cycles() % kClocksPerVSync; // cycles into this VSync

	// machine state switches
	//
	Hires    = m_video->m_hires ? 1 : 0;
	Mixed    = m_video->m_mix ? 1 : 0;
	Page2    = m_page2 ? 1 : 0;
	_80Store = 0;

	// calculate video parameters according to display standard
	//
	ScanLines  = 1 ? kNTSCScanLines : kPALScanLines; // FIX: NTSC only?
	// VSyncLine  = 1 ? kNTSCVSyncLine : kPALVSyncLine; // FIX: NTSC only?
	// ScanCycles = ScanLines * kHClocks;

	// calculate horizontal scanning state
	//
	h_clock = (i + kHPEClock) % kHClocks; // which horizontal scanning clock
	h_state = kHClock0State + h_clock; // H state bits
	if (h_clock >= kHPresetClock) // check for horizontal preset
	{
		h_state -= 1; // correct for state preset (two 0 states)
	}
	h_0 = (h_state >> 0) & 1; // get horizontal state bits
	h_1 = (h_state >> 1) & 1;
	h_2 = (h_state >> 2) & 1;
	h_3 = (h_state >> 3) & 1;
	h_4 = (h_state >> 4) & 1;
	h_5 = (h_state >> 5) & 1;

	// calculate vertical scanning state
	//
	v_line  = i / kHClocks; // which vertical scanning line
	v_state = kVLine0State + v_line; // V state bits
	if ((v_line >= kVPresetLine)) // check for previous vertical state preset
	{
		v_state -= ScanLines; // compensate for preset
	}
	v_A = (v_state >> 0) & 1; // get vertical state bits
	v_B = (v_state >> 1) & 1;
	v_C = (v_state >> 2) & 1;
	v_0 = (v_state >> 3) & 1;
	v_1 = (v_state >> 4) & 1;
	v_2 = (v_state >> 5) & 1;
	v_3 = (v_state >> 6) & 1;
	v_4 = (v_state >> 7) & 1;
	//v_5 = (v_state >> 8) & 1;

	// calculate scanning memory address
	//
	_hires = Hires;
	if (Hires && Mixed && (v_4 & v_2))
	{
		_hires = 0; // (address is in text memory)
	}

	addend0 = 0x68; // 1            1            0            1
	addend1 =              (h_5 << 5) | (h_4 << 4) | (h_3 << 3);
	addend2 = (v_4 << 6) | (v_3 << 5) | (v_4 << 4) | (v_3 << 3);
	sum     = (addend0 + addend1 + addend2) & (0x0F << 3);

	address = 0;
	address |= h_0 << 0; // a0
	address |= h_1 << 1; // a1
	address |= h_2 << 2; // a2
	address |= sum;      // a3 - aa6
	address |= v_0 << 7; // a7
	address |= v_1 << 8; // a8
	address |= v_2 << 9; // a9
	address |= ((_hires) ? v_A : (1 ^ (Page2 & (1 ^ _80Store)))) << 10; // a10
	address |= ((_hires) ? v_B : (Page2 & (1 ^ _80Store))) << 11; // a11
	if (_hires) // hires?
	{
		// Y: insert hires only address bits
		//
		address |= v_C << 12; // a12
		address |= (1 ^ (Page2 & (1 ^ _80Store))) << 13; // a13
		address |= (Page2 & (1 ^ _80Store)) << 14; // a14
	}
	else
	{
		// N: text, so no higher address bits unless Apple ][, not Apple //e
		//
		if ((1) && // Apple ][? // FIX: check for Apple ][? (FB is most useful in old games)
			(kHPEClock <= h_clock) && // Y: HBL?
			(h_clock <= (kHClocks - 1)))
		{
			address |= 1 << 12; // Y: a12 (add $1000 to address!)
		}
	}

	return m_ram_ptr[address % m_ram_size]; // FIX: this seems to work, but is it right!?
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

READ8_MEMBER(napple2_state::ram_r)
{
	if (offset < m_ram_size)
	{
		return m_ram_ptr[offset];
	}

	return 0xff;
}

WRITE8_MEMBER(napple2_state::ram_w)
{
	if (offset < m_ram_size)
	{
		m_ram_ptr[offset] = data;
	}
}

static ADDRESS_MAP_START( apple2_map, AS_PROGRAM, 8, napple2_state )
	AM_RANGE(0x0000, 0xbfff) AM_READWRITE(ram_r, ram_w)
	AM_RANGE(0xc000, 0xc07f) AM_READWRITE(c000_r, c000_w)
	AM_RANGE(0xc080, 0xc0ff) AM_READWRITE(c080_r, c080_w)
	AM_RANGE(0xc100, 0xc7ff) AM_READWRITE(c100_r, c100_w)
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(c800_r, c800_w)
	AM_RANGE(0xd000, 0xffff) AM_DEVICE(A2_UPPERBANK_TAG, address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( inhbank_map, AS_PROGRAM, 8, napple2_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM AM_REGION("maincpu", 0x1000) AM_WRITE(inh_w)
	AM_RANGE(0x3000, 0x5fff) AM_READWRITE(inh_r, inh_w)
ADDRESS_MAP_END

/***************************************************************************
    KEYBOARD
***************************************************************************/

READ_LINE_MEMBER(napple2_state::ay3600_shift_r)
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

READ_LINE_MEMBER(napple2_state::ay3600_control_r)
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

static const UINT8 a2_key_remap[0x32][4] =
{
/*    norm shft ctrl both */
	{ 0x33,0x23,0x33,0x23 },    /* 3 #     00     */
	{ 0x34,0x24,0x34,0x24 },    /* 4 $     01     */
	{ 0x35,0x25,0x35,0x25 },    /* 5 %     02     */
	{ 0x36,'&', 0x35,'&'  },    /* 6 &     03     */
	{ 0x37,0x27,0x37,0x27 },    /* 7 '     04     */
	{ 0x38,0x28,0x38,0x28 },    /* 8 (     05     */
	{ 0x39,0x29,0x39,0x29 },    /* 9 )     06     */
	{ 0x30,0x30,0x30,0x30 },    /* 0       07     */
	{ 0x3a,0x2a,0x3b,0x2a },    /* : *     08     */
	{ 0x2d,0x3d,0x2d,0x3d },    /* - =     09     */
	{ 0x51,0x51,0x11,0x11 },    /* q Q     0a     */
	{ 0x57,0x57,0x17,0x17 },    /* w W     0b     */
	{ 0x45,0x45,0x05,0x05 },    /* e E     0c     */
	{ 0x52,0x52,0x12,0x12 },    /* r R     0d     */
	{ 0x54,0x54,0x14,0x14 },    /* t T     0e     */
	{ 0x59,0x59,0x19,0x19 },    /* y Y     0f     */
	{ 0x55,0x55,0x15,0x15 },    /* u U     10     */
	{ 0x49,0x49,0x09,0x09 },    /* i I     11     */
	{ 0x4f,0x4f,0x0f,0x0f },    /* o O     12     */
	{ 0x50,0x40,0x10,0x40 },    /* p P     13     */
	{ 0x44,0x44,0x04,0x04 },    /* d D     14     */
	{ 0x46,0x46,0x06,0x06 },    /* f F     15     */
	{ 0x47,0x47,0x07,0x07 },    /* g G     16     */
	{ 0x48,0x48,0x08,0x08 },    /* h H     17     */
	{ 0x4a,0x4a,0x0a,0x0a },    /* j J     18     */
	{ 0x4b,0x4b,0x0b,0x0b },    /* k K     19     */
	{ 0x4c,0x4c,0x0c,0x0c },    /* l L     1a     */
	{ ';' ,0x2b,';' ,0x2b },    /* ; +     1b     */
	{ 0x08,0x08,0x08,0x08 },    /* Left    1c     */
	{ 0x15,0x15,0x15,0x15 },    /* Right   1d     */
	{ 0x5a,0x5a,0x1a,0x1a },    /* z Z     1e     */
	{ 0x58,0x58,0x18,0x18 },    /* x X     1f     */
	{ 0x43,0x43,0x03,0x03 },    /* c C     20     */
	{ 0x56,0x56,0x16,0x16 },    /* v V     21     */
	{ 0x42,0x42,0x02,0x02 },    /* b B     22     */
	{ 0x4e,0x5e,0x0e,0x5e },    /* n N     23     */
	{ 0x4d,0x4d,0x0d,0x0d },    /* m M     24     */
	{ 0x2c,0x3c,0x2c,0x3c },    /* , <     25     */
	{ 0x2e,0x3e,0x2e,0x3e },    /* . >     26     */
	{ 0x2f,0x3f,0x2f,0x3f },    /* / ?     27     */
	{ 0x53,0x53,0x13,0x13 },    /* s S     28     */
	{ 0x32,0x22,0x32,0x00 },    /* 2 "     29     */
	{ 0x31,0x21,0x31,0x31 },    /* 1 !     2a     */
	{ 0x1b,0x1b,0x1b,0x1b },    /* Escape  2b     */
	{ 0x41,0x41,0x01,0x01 },    /* a A     2c     */
	{ 0x20,0x20,0x20,0x20 },    /* Space   2d     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x2e unused    */
	{ 0x00,0x00,0x00,0x00 },    /* 0x2f unused    */
	{ 0x00,0x00,0x00,0x00 },    /* 0x30 unused    */
	{ 0x0d,0x0d,0x0d,0x0d },    /* Enter   31     */
};

WRITE_LINE_MEMBER(napple2_state::ay3600_data_ready_w)
{
	if (state == ASSERT_LINE)
	{
		int mod = 0;
		m_lastchar = m_ay3600->b_r();

		mod = (m_kbspecial->read() & 0x06) ? 0x01 : 0x00;
		mod |= (m_kbspecial->read() & 0x08) ? 0x02 : 0x00;

		m_transchar = a2_key_remap[m_lastchar&0x3f][mod];

		if (m_transchar != 0)
		{
			m_strobe = 0x80;
//          printf("new char = %04x (%02x)\n", m_lastchar&0x3f, m_transchar);
		}
	}
}

WRITE_LINE_MEMBER(napple2_state::ay3600_ako_w)
{
	m_anykeydown = (state == ASSERT_LINE) ? true : false;
}

TIMER_DEVICE_CALLBACK_MEMBER(napple2_state::ay3600_repeat)
{
	// is the key still down?
	if (m_anykeydown)
	{
		if (m_kbrepeat->read() & 1)
		{
			m_strobe = 0x80;
		}
	}
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( apple2_joystick )
	PORT_START("joystick_1_x")      /* Joystick 1 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P1 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_4_PAD)    PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_1_y")      /* Joystick 1 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P1 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_8_PAD)    PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_2_x")      /* Joystick 2 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P2 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_2_y")      /* Joystick 2 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P2 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_buttons")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(1)            PORT_CODE(KEYCODE_0_PAD)    PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_PLAYER(1)            PORT_CODE(KEYCODE_ENTER_PAD)PORT_CODE(JOYCODE_BUTTON2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(2)            PORT_CODE(JOYCODE_BUTTON1)
INPUT_PORTS_END

INPUT_PORTS_START( apple2_gameport )
	PORT_INCLUDE( apple2_joystick )
INPUT_PORTS_END

INPUT_PORTS_START( apple2_sysconfig )
	PORT_START("a2_config")
	PORT_CONFNAME(0x03, 0x00, "Composite monitor type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x01, "B&W")
	PORT_CONFSETTING(0x02, "Green")
	PORT_CONFSETTING(0x03, "Amber")

	PORT_CONFNAME(0x04, 0x04, "Shift key mod")  // default to installed
	PORT_CONFSETTING(0x00, "Not present")
	PORT_CONFSETTING(0x04, "Installed")
INPUT_PORTS_END

	/*
	  Apple II / II Plus key matrix (from "The Apple II Circuit Description")

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X0  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  0  | :*  |  -  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  |  Q  |  W  |  E  |  R  |  T  |  Y  |  U  |  I  |  O  |  P  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |  D  |  F  |  G  |  H  |  J  |  K  |  L  | ;+  |LEFT |RIGHT|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |  Z  |  X  |  C  |  V  |  B  |  N  |  M  | ,<  | .>  |  /? |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  |  S  |  2  |  1  | ESC |  A  |SPACE|     |     |     |ENTER|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

static INPUT_PORTS_START( apple2_common )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)  PORT_CHAR('0')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('@')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2 )
	PORT_INCLUDE(apple2_common)

	PORT_START("keyb_repeat")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REPT")         PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')

	/* other devices */
	PORT_INCLUDE( apple2_gameport )

	PORT_INCLUDE(apple2_sysconfig)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2p )
	PORT_INCLUDE( apple2 )

	PORT_START("reset_dip")
	PORT_DIPNAME( 0x01, 0x01, "Reset" )
	PORT_DIPSETTING( 0x01, "CTRL-RESET" )
	PORT_DIPSETTING( 0x00, "RESET" )
INPUT_PORTS_END

static SLOT_INTERFACE_START(apple2_slot0_cards)
	SLOT_INTERFACE("lang", A2BUS_RAMCARD16K)      /* Apple II RAM Language Card */
	SLOT_INTERFACE("ssram", A2BUS_RAMCARD128K)    /* Saturn Systems 128K extended language card */
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(apple2_cards)
	SLOT_INTERFACE("diskii", A2BUS_DISKII)  /* Disk II Controller Card */
	SLOT_INTERFACE("diskiing", A2BUS_DISKIING)  /* Disk II Controller Card, cycle-accurate version */
	SLOT_INTERFACE("mockingboard", A2BUS_MOCKINGBOARD)  /* Sweet Micro Systems Mockingboard */
	SLOT_INTERFACE("phasor", A2BUS_PHASOR)  /* Applied Engineering Phasor */
	SLOT_INTERFACE("cffa2", A2BUS_CFFA2)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware */
	SLOT_INTERFACE("cffa202", A2BUS_CFFA2_6502)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware */
	SLOT_INTERFACE("memexp", A2BUS_MEMEXP)  /* Apple II Memory Expansion Card */
	SLOT_INTERFACE("ramfactor", A2BUS_RAMFACTOR)    /* Applied Engineering RamFactor */
	SLOT_INTERFACE("thclock", A2BUS_THUNDERCLOCK)    /* ThunderWare ThunderClock Plus */
	SLOT_INTERFACE("softcard", A2BUS_SOFTCARD)  /* Microsoft SoftCard */
	SLOT_INTERFACE("videoterm", A2BUS_VIDEOTERM)    /* Videx VideoTerm */
	SLOT_INTERFACE("ssc", A2BUS_SSC)    /* Apple Super Serial Card */
	SLOT_INTERFACE("swyft", A2BUS_SWYFT)    /* IAI SwyftCard */
	SLOT_INTERFACE("themill", A2BUS_THEMILL)    /* Stellation Two The Mill (6809 card) */
	SLOT_INTERFACE("sam", A2BUS_SAM)    /* SAM Software Automated Mouth (8-bit DAC + speaker) */
	SLOT_INTERFACE("alfam2", A2BUS_ALFAM2)    /* ALF Apple Music II */
	SLOT_INTERFACE("echoii", A2BUS_ECHOII)    /* Street Electronics Echo II */
	SLOT_INTERFACE("ap16", A2BUS_IBSAP16)    /* IBS AP16 (German VideoTerm clone) */
	SLOT_INTERFACE("ap16alt", A2BUS_IBSAP16ALT)    /* IBS AP16 (German VideoTerm clone), alternate revision */
	SLOT_INTERFACE("vtc1", A2BUS_VTC1)    /* Unknown VideoTerm clone #1 */
	SLOT_INTERFACE("vtc2", A2BUS_VTC2)    /* Unknown VideoTerm clone #2 */
	SLOT_INTERFACE("arcbd", A2BUS_ARCADEBOARD)    /* Third Millenium Engineering Arcade Board */
	SLOT_INTERFACE("midi", A2BUS_MIDI)  /* Generic 6840+6850 MIDI board */
	SLOT_INTERFACE("zipdrive", A2BUS_ZIPDRIVE)  /* ZIP Technologies IDE card */
	SLOT_INTERFACE("echoiiplus", A2BUS_ECHOPLUS)    /* Street Electronics Echo Plus (Echo II + Mockingboard clone) */
	SLOT_INTERFACE("scsi", A2BUS_SCSI)  /* Apple II SCSI Card */
	SLOT_INTERFACE("applicard", A2BUS_APPLICARD)    /* PCPI Applicard */
	SLOT_INTERFACE("aesms", A2BUS_AESMS)    /* Applied Engineering Super Music Synthesizer */
	SLOT_INTERFACE("ultraterm", A2BUS_ULTRATERM)    /* Videx UltraTerm (original) */
	SLOT_INTERFACE("ultratermenh", A2BUS_ULTRATERMENH)    /* Videx UltraTerm (enhanced //e) */
	SLOT_INTERFACE("aevm80", A2BUS_VTC2)    /* Applied Engineering ViewMaster 80 */
	SLOT_INTERFACE("parallel", A2BUS_PIC)   /* Apple Parallel Interface Card */
	SLOT_INTERFACE("corvus", A2BUS_CORVUS)  /* Corvus flat-cable HDD interface (see notes in a2corvus.c) */
	SLOT_INTERFACE("mcms1", A2BUS_MCMS1)  /* Mountain Computer Music System, card 1 of 2 */
	SLOT_INTERFACE("mcms2", A2BUS_MCMS2)  /* Mountain Computer Music System, card 2 of 2.  must be in card 1's slot + 1! */
	SLOT_INTERFACE("dx1", A2BUS_DX1)    /* Decillonix DX-1 sampler card */
	SLOT_INTERFACE("tm2ho", A2BUS_TIMEMASTERHO) /* Applied Engineering TimeMaster II H.O. */
	SLOT_INTERFACE("mouse", A2BUS_MOUSE)    /* Apple II Mouse Card */
	SLOT_INTERFACE("ezcgi", A2BUS_EZCGI)    /* E-Z Color Graphics Interface */
	SLOT_INTERFACE("ezcgi9938", A2BUS_EZCGI_9938)   /* E-Z Color Graphics Interface (TMS9938) */
	SLOT_INTERFACE("ezcgi9958", A2BUS_EZCGI_9958)   /* E-Z Color Graphics Interface (TMS9958) */
//  SLOT_INTERFACE("magicmusician", A2BUS_MAGICMUSICIAN)    /* Magic Musician Card */
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( apple2_common, napple2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(A2_CPU_TAG, M6502, 1021800)     /* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", napple2_state, apple2_interrupt, "screen", 0, 1)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_DEVICE_ADD(A2_VIDEO_TAG, APPLE2_VIDEO, XTAL_14_31818MHz)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192)
	MCFG_SCREEN_UPDATE_DRIVER(napple2_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(napple2_state, apple2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(A2_SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* /INH banking */
	MCFG_DEVICE_ADD(A2_UPPERBANK_TAG, ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(inhbank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x3000)

	/* keyboard controller */
	MCFG_DEVICE_ADD(A2_KBDC_TAG, AY3600, 0)
	MCFG_AY3600_MATRIX_X0(IOPORT("X0"))
	MCFG_AY3600_MATRIX_X1(IOPORT("X1"))
	MCFG_AY3600_MATRIX_X2(IOPORT("X2"))
	MCFG_AY3600_MATRIX_X3(IOPORT("X3"))
	MCFG_AY3600_MATRIX_X4(IOPORT("X4"))
	MCFG_AY3600_MATRIX_X5(IOPORT("X5"))
	MCFG_AY3600_MATRIX_X6(IOPORT("X6"))
	MCFG_AY3600_MATRIX_X7(IOPORT("X7"))
	MCFG_AY3600_MATRIX_X8(IOPORT("X8"))
	MCFG_AY3600_SHIFT_CB(READLINE(napple2_state, ay3600_shift_r))
	MCFG_AY3600_CONTROL_CB(READLINE(napple2_state, ay3600_control_r))
	MCFG_AY3600_DATA_READY_CB(WRITELINE(napple2_state, ay3600_data_ready_w))
	MCFG_AY3600_AKO_CB(WRITELINE(napple2_state, ay3600_ako_w))

	/* repeat timer.  15 Hz from page 90 of "The Apple II Circuit Description */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("repttmr", napple2_state, ay3600_repeat, attotime::from_hz(15))

	/* slot devices */
	MCFG_DEVICE_ADD(A2_BUS_TAG, A2BUS, 0)
	MCFG_A2BUS_CPU(A2_CPU_TAG)
	MCFG_A2BUS_OUT_IRQ_CB(WRITELINE(napple2_state, a2bus_irq_w))
	MCFG_A2BUS_OUT_NMI_CB(WRITELINE(napple2_state, a2bus_nmi_w))
	MCFG_A2BUS_OUT_INH_CB(WRITELINE(napple2_state, a2bus_inh_w))
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl0", apple2_slot0_cards, "lang")
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl1", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl2", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl3", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl4", apple2_cards, "mockingboard")
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl5", apple2_cards, NULL)
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl6", apple2_cards, "diskiing")
	MCFG_A2BUS_SLOT_ADD(A2_BUS_TAG, "sl7", apple2_cards, NULL)

	MCFG_SOFTWARE_LIST_ADD("flop525_list","apple2")

	MCFG_CASSETTE_ADD(A2_CASSETTE_TAG)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2, apple2_common )
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K")
	MCFG_RAM_EXTRA_OPTIONS("4K,8K,12K,16K,20K,24K,32K,36K,48K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2p, apple2_common )
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K")
	MCFG_RAM_EXTRA_OPTIONS("16K,32K,48K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( space84, apple2p )
MACHINE_CONFIG_END

#if 0
static MACHINE_CONFIG_DERIVED( laba2p, apple2p )
	MCFG_MACHINE_START_OVERRIDE(napple2_state,laba2p)

	MCFG_A2BUS_SLOT_REMOVE("sl0")
	MCFG_A2BUS_SLOT_REMOVE("sl3")
	MCFG_A2BUS_SLOT_REMOVE("sl6")

//  MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl3", A2BUS_LAB_80COL, NULL)
	MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_IWM_FDC, NULL)

MACHINE_CONFIG_END
#endif

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(apple2) /* the classic, non-autoboot apple2 with integer basic in rom. optional card with autoboot and applesoft basic was possible but isn't yet supported */
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "a2.chr", 0x0000, 0x0800, BAD_DUMP CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659)) /* current dump is 341-0036 which is the appleII+ character generator, not the original appleII one, whose rom number is not yet known! */

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD_OPTIONAL ( "341-0016-00.d0", 0x1000, 0x0800, CRC(4234e88a) SHA1(c9a81d704dc2f0c3416c20f9c4ab71fedda937ed)) /* 341-0016: Programmer's Aid #1 D0 */

	ROM_LOAD ( "341-0001-00.e0", 0x2000, 0x0800, CRC(c0a4ad3b) SHA1(bf32195efcb34b694c893c2d342321ec3a24b98f)) /* Needs verification. From eBay: Label: S7925E // C48077 // 3410001-00 // (C)APPLE78 E0 */
	ROM_LOAD ( "341-0002-00.e8", 0x2800, 0x0800, CRC(a99c2cf6) SHA1(9767d92d04fc65c626223f25564cca31f5248980)) /* Needs verification. From eBay: Label: S7916E // C48078 // 3410002-00 // (C)APPLE78 E8 */
	ROM_LOAD ( "341-0003-00.f0", 0x3000, 0x0800, CRC(62230d38) SHA1(f268022da555e4c809ca1ae9e5d2f00b388ff61c)) /* Needs verification. From eBay: Label: S7908E // C48709 // 3410003 // CAPPLE78 F0 */
	ROM_LOAD ( "341-0004-00.f8", 0x3800, 0x0800, CRC(020a86d0) SHA1(52a18bd578a4694420009cad7a7a5779a8c00226))
	ROM_END

ROM_START(apple2p) /* the autoboot apple2+ with applesoft (microsoft-written) basic in rom; optional card with monitor and integer basic was possible but isn't yet supported */
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "341-0036.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x4000, "maincpu", ROMREGION_LE)
	ROM_LOAD ( "341-0011.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "341-0015.f0", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "341-0020-00.f8", 0x3800, 0x0800, CRC(079589c4) SHA1(a28852ff997b4790e53d8d0352112c4b1a395098)) /* 341-0020-00: Autostart Monitor/Applesoft Basic $f800; Was sometimes mounted on Language card; Label(from Apple Language Card - Front.jpg): S 8115 // C68018 // 341-0020-00 */
ROM_END

ROM_START(elppa)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "elppa.chr", 0x0000, 0x0800, BAD_DUMP CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659)) // Taken from 341-0036.chr used in apple2p

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "elppa.d0", 0x1000, 0x0800, CRC(ce5b0e7e) SHA1(2c1a0aa023ae6deb2bddb8937345ee354028aeef))
	ROM_LOAD ( "elppa.d8", 0x1800, 0x0800, CRC(bd409bad) SHA1(5145d238042938efbb9b71e0a4ef9a980b0e38de))
	ROM_LOAD ( "elppa.e0", 0x2000, 0x0800, CRC(4c997c88) SHA1(70b639d8cbafcd5367d2f9dfd6890e5d1c6890f0))
	ROM_LOAD ( "elppa.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "elppa.f0", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "elppa.f8", 0x3800, 0x0800, CRC(62c0c761) SHA1(19f28544fd5021a2d72e6015b3183c462c0e86f8))
ROM_END

ROM_START(prav82)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "pravetz82.chr", 0x0000, 0x0800, BAD_DUMP CRC(8c55c984) SHA1(5a5a202000576b88b4ae2e180dd2d1b9b337b594)) // Taken from Agat computer

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "pravetz82.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "pravetz82.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "pravetz82.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "pravetz82.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "pravetz82.f0", 0x3000, 0x0800, CRC(e26d9d35) SHA1(ce6e42e6c9a6c98e92522af7a6090cd04c56c778))
	ROM_LOAD ( "pravetz82.f8", 0x3800, 0x0800, CRC(57547818) SHA1(db30bedec98305e31a14acb9e2a92be1c4853807))
ROM_END

ROM_START(prav8m)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "pravetz8m.chr", 0x0000, 0x0800, BAD_DUMP CRC(8c55c984) SHA1(5a5a202000576b88b4ae2e180dd2d1b9b337b594)) // Taken from Agat computer
	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "pravetz8m.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "pravetz8m.d8", 0x1800, 0x0800, CRC(654b6f7b) SHA1(f7b1457b48fe6974c4de7e976df3a8fca6b7b661))
	ROM_LOAD ( "pravetz8m.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "pravetz8m.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "pravetz8m.f0", 0x3000, 0x0800, CRC(e26d9d35) SHA1(ce6e42e6c9a6c98e92522af7a6090cd04c56c778))
	ROM_LOAD ( "pravetz8m.f8", 0x3800, 0x0800, CRC(5bab0a46) SHA1(f6c0817ce37d2e2c43f482c339acaede0a73359b))
ROM_END

ROM_START(uniap2pt)
	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "unitron.chr", 0x0000, 0x1000, CRC(7fdd1af6) SHA1(2f4f90d90f2f3a8c1fbea304e1072780fb22e698))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "unitron_pt.d0", 0x1000, 0x1000, CRC(311beae6) SHA1(f6379aba9ac982850edc314c93a393844a3349ef))
	ROM_LOAD ( "unitron.e0"   , 0x2000, 0x1000, CRC(0d494efd) SHA1(a2fd1223a3ca0cfee24a6afe66ea3c4c144dd98e))
	ROM_LOAD ( "unitron.f0"   , 0x3000, 0x1000, CRC(8e047c4a) SHA1(78c57c0e00dfce7fdec9437fe2b4c25def447e5d))
ROM_END

ROM_START(uniap2en)
	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "unitron.chr", 0x0000, 0x1000, CRC(7fdd1af6) SHA1(2f4f90d90f2f3a8c1fbea304e1072780fb22e698))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "unitron_en.d0", 0x1000, 0x1000, CRC(24d73c7b) SHA1(d17a15868dc875c67061c95ec53a6b2699d3a425))
	ROM_LOAD ( "unitron.e0"   , 0x2000, 0x1000, CRC(0d494efd) SHA1(a2fd1223a3ca0cfee24a6afe66ea3c4c144dd98e))
	ROM_LOAD ( "unitron.f0"   , 0x3000, 0x1000, CRC(8e047c4a) SHA1(78c57c0e00dfce7fdec9437fe2b4c25def447e5d))
ROM_END

/*
    J-Plus ROM numbers confirmed by:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Computers/Apple%20II/Apple%20II%20j-plus/Photos/Apple%20II%20j-plus%20-%20Motherboard.jpg
*/

ROM_START(apple2jp)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "a2jp.chr", 0x0000, 0x0800, CRC(487104b5) SHA1(0a382be58db5215c4a3de53b19a72fab660d5da2))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "341-0011.d0", 0x1000, 0x0800, BAD_DUMP CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8", 0x1800, 0x0800, BAD_DUMP CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0", 0x2000, 0x0800, BAD_DUMP CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8", 0x2800, 0x0800, BAD_DUMP CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "341-0015.f0", 0x3000, 0x0800, BAD_DUMP CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "341-0047.f8", 0x3800, 0x0800, CRC(6ea8379b) SHA1(00a75ae3b58e1917ad640249366f654608589cf4))
ROM_END

ROM_START(maxxi)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "maxxi.chr", 0x0000, 0x0800, BAD_DUMP CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659)) // Taken from 341-0036.chr used in apple2p

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "maxxi.d0", 0x1000, 0x1000, CRC(7831f025) SHA1(0eb4161e5223c0dde2d140fcbace80d292ff9dc6))
	ROM_LOAD ( "maxxi.e0", 0x2000, 0x1000, CRC(0d494efd) SHA1(a2fd1223a3ca0cfee24a6afe66ea3c4c144dd98e))
	ROM_LOAD ( "maxxi.f0", 0x3000, 0x1000, CRC(34e4d01b) SHA1(44853b2d59ddd234db76c1a0d529180fb1e008ef))

	ROM_REGION(0x0800,"keyboard",0)
	ROM_LOAD ( "maxxi_teclado.rom", 0x0000, 0x0800, CRC(10c2d5b6) SHA1(226036d2f6f8fa5675303640ee1e5f0bab1135c6))
ROM_END

ROM_START(ace100)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "ace100.chr", 0x0000, 0x0800, BAD_DUMP CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659)) // copy of a2.chr - real Ace chr is undumped

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "ace100.rom", 0x1000, 0x3000, CRC(9d5ec94f) SHA1(8f2b3f2561788bebc7a805f620ec9e7ade973460))
ROM_END

ROM_START(space84)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "space 84 mobo chargen.bin", 0x0000, 0x2000, CRC(ceb98990) SHA1(8b2758da611bcfdd3d144edabc63ef1df2ca787b) )

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "341-0011.d0",  0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8",  0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0",  0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8",  0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD( "space84_f.bin", 0x3000, 0x1000, CRC(4e741069) SHA1(ca1f16da9fb40e966ee4a899964cd6a7e140ab50))
ROM_END

ROM_START(am64)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "gm-2716.bin",  0x0000, 0x0800, CRC(863e657f) SHA1(cc954204c503bc545ec0d08862483aaad83805d5) )

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "am64-27128.bin", 0x0000, 0x4000, CRC(f25cdc7b) SHA1(235e72b77695938a9df8781f5bea3cbbbe1f4c76) )

	ROM_REGION(0x2000, "spares", 0)
	// parallel card ROM
	ROM_LOAD( "ap-2716.bin",  0x0000, 0x0800, CRC(c6990f08) SHA1(e7daf63639234e46738a4d78a49287d11ccaf537) )
	// i8048 keyboard MCU ROM
	ROM_LOAD( "tk10.bin",     0x0800, 0x0800, CRC(a06c5b78) SHA1(27c5160b913e0f62120f384026d24b9f1acb6970) )
ROM_END

ROM_START( agat7 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "monitor7.rom", 0x3800, 0x0800, CRC(071fda0b) SHA1(6089d46b7addc4e2ae096b2cf81124681bd2b27a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "agat_pzu.bin", 0x3800, 0x0800, CRC(c605163d) SHA1(b30fd1b264a347a9de69bb9e3105483254994d06), ROM_BIOS(2))
	// Floppy controllers
	ROM_LOAD( "shugart7.rom", 0x4500, 0x0100, CRC(c6e4850c) SHA1(71626d3d2d4bbeeac2b77585b45a5566d20b8d34))
	ROM_LOAD( "teac.rom",     0x4500, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e))
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD( "agathe7.fnt", 0x0000, 0x0800, CRC(fcffb490) SHA1(0bda26ae7ad75f74da835c0cf6d9928f9508844c))
ROM_END

ROM_START( agat9 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "monitor9.rom", 0x3800, 0x0800, CRC(b90bb66a) SHA1(02217f0785913b41fc25eabcff70fa814799c69a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "monitor91.rom", 0x3800, 0x0800, CRC(89b10fc1) SHA1(7fe1ede32b5525255f82597ca9c3c2034c5996fa), ROM_BIOS(2))
	// Floppy controllers
	ROM_LOAD( "shugart9.rom", 0x4500, 0x0100, CRC(964a0ce2) SHA1(bf955189ebffe874c20ef649a3db8177dc16af61))
	ROM_LOAD( "teac.rom",     0x4500, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e))
	// Printer card
	ROM_LOAD( "cm6337.rom", 0x8000, 0x0100, CRC(73be16ec) SHA1(ead1abbef5b86f1def0b956147d5b267f0d544b5))
	ROM_LOAD( "cm6337p.rom", 0x8100, 0x0800, CRC(9120f11f) SHA1(78107653491e88d5ea12e07367c4c028771a4aca))
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD( "agathe9.fnt", 0x0000, 0x0800, CRC(8c55c984) SHA1(5a5a202000576b88b4ae2e180dd2d1b9b337b594))
ROM_END

ROM_START(ivelultr)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "ultra.chr", 0x0000, 0x1000,CRC(fed62c85) SHA1(479fb3f38a3f7332cef2e8c4856871afe8dc6017))
	ROM_LOAD( "ultra.chr", 0x1000, 0x1000,CRC(fed62c85) SHA1(479fb3f38a3f7332cef2e8c4856871afe8dc6017))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "ultra1.bin", 0x2000, 0x1000, CRC(8ab49c1c) SHA1(b41da28a40c3a22bc10a954a86716a1a2bae04a4))
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_LOAD( "ultra2.bin", 0x3000, 0x1000, CRC(1ac1e17e) SHA1(a5b8adec37da91970c303905b5e2c4d1b715ee4e))

	ROM_REGION(0x800, "kbmcu", 0)   // 6802 code for keyboard MCU (very unlike real Apples, will require some reverse-engineering)
	ROM_LOAD( "ultra4.bin", 0x0000, 0x0800, CRC(3dce51ac) SHA1(676b6e775d5159049cae5b6143398ec7b2bf437a) )
ROM_END

#if 0
ROM_START(laba2p) /* II Plus clone with on-board Disk II controller and Videx-compatible 80-column card, supposedly from lab equipment */
	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD( "char.u30",     0x0000, 0x1000, CRC(2dbaef88) SHA1(9834842796132a11facd57923326d6954bcb609f) )

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD( "maind0.u35",   0x1000, 0x1000, CRC(24d73c7b) SHA1(d17a15868dc875c67061c95ec53a6b2699d3a425) )
	ROM_LOAD( "maine0.u34",   0x2000, 0x2000, CRC(314462ca) SHA1(5a23616dca14e59b4aca8ff6cfa0d98592a78a79) )

	ROM_REGION(0x1000, "fw80col", 0)
	ROM_LOAD( "80cfw.u3",     0x0000, 0x1000, CRC(92d2b8b0) SHA1(5149483eb3e550ece1584e85fc821bb04d068dec) )    // firmware for on-board Videx

	ROM_REGION(0x1000, "cg80col", 0)
	ROM_LOAD( "80ccgv80.u25", 0x0000, 0x1000, CRC(6d5e2707) SHA1(c56f76e8a366fee7374eb09f4866435c692490b2) )    // character generator for on-board Videx

	ROM_REGION(0x800, "diskii", 0)
	ROM_LOAD( "diskfw.u7",    0x0000, 0x0800, CRC(9207ef4e) SHA1(5fcffa4c68b16a7ef2f62651d4c7470400e5bd35) )    // firmware for on-board Disk II

	ROM_REGION(0x800, "unknown", 0)
	ROM_LOAD( "unk.u5",       0x0000, 0x0800, CRC(240a1774) SHA1(e6aeb0702dc99d76fd8c5a642fdfbe9ab896acd4) )    // unknown ROM
ROM_END
#endif

/*    YEAR  NAME      PARENT    COMPAT    MACHINE      INPUT     INIT      COMPANY            FULLNAME */
COMP( 1977, apple2,   0,        0,        apple2,      apple2,  driver_device,   0,        "Apple Computer",    "Apple ][", MACHINE_SUPPORTS_SAVE )
COMP( 1979, apple2p,  apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Apple Computer",    "Apple ][+", MACHINE_SUPPORTS_SAVE )
COMP( 1980, apple2jp, apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Apple Computer",    "Apple ][ J-Plus", MACHINE_SUPPORTS_SAVE )
COMP( 198?, elppa,    apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Victor do Brasil",  "Elppa II+", MACHINE_SUPPORTS_SAVE )
COMP( 1982, maxxi,    apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Polymax",  "Maxxi", MACHINE_SUPPORTS_SAVE )
COMP( 1982, prav82,   apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Pravetz",           "Pravetz 82", MACHINE_SUPPORTS_SAVE )
COMP( 1982, ace100,   apple2,   0,        apple2,      apple2p, driver_device,  0,        "Franklin Computer", "Franklin Ace 100", MACHINE_SUPPORTS_SAVE )
COMP( 1982, uniap2en, apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Unitron Eletr??nica", "Unitron AP II (in English)", MACHINE_SUPPORTS_SAVE )
COMP( 1982, uniap2pt, apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Unitron Eletr??nica", "Unitron AP II (in Brazilian Portuguese)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, agat7,    apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Agat",              "Agat-7", MACHINE_NOT_WORKING) // disk controller ROM JSRs to $FCA8 which is a delay on apple II, illegal instruction crash here :(
// reverse font direction -\/
COMP( 1984, ivelultr, apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Ivasim",            "Ivel Ultra", MACHINE_SUPPORTS_SAVE )
COMP( 1984, agat9,    apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Agat",              "Agat-9", MACHINE_NOT_WORKING)
COMP( 1985, prav8m,   apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Pravetz",           "Pravetz 8M", MACHINE_SUPPORTS_SAVE )
COMP( 1985, space84,  apple2,   0,        space84,     apple2p, driver_device,  0,        "ComputerTechnik/IBS",  "Space 84",   MACHINE_NOT_WORKING )
COMP( 1985, am64,     apple2,   0,        space84,     apple2p, driver_device,  0,        "ASEM",              "AM 64", MACHINE_SUPPORTS_SAVE )
//COMP( 19??, laba2p,   apple2,   0,        laba2p,      apple2p, driver_device,  0,        "<unknown>",         "Lab equipment Apple II Plus clone", MACHINE_SUPPORTS_SAVE )
