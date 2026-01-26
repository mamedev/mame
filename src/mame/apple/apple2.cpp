// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple2.cpp - Apple II/II Plus and clones

    Next generation driver written in September/October 2014 by R. Belmont.
    Thanks to the original Apple II series driver's authors: Mike Balfour, Nathan Woods, and R. Belmont
    Special thanks to the Apple II Documentation Project/Antoine Vignau and Peter Ferrie.

II: original base model.  RAM sizes of 4, 8, 12, 16, 20, 24, 32, 36, and 48 KB possible.
    (Later motherboard revisions only support multiples of 16 KB.)
    8K of ROM at $E000-$FFFF, empty sockets for $D000-$D7FF and $D800-$DFFF.
    Programmer's Aid #1 was sold by Apple for $D000-$D7FF, some third-party ROMs
    were also available.

    Revision 0 (very rare) had only 4 hi-res colors (blue and orange were missing).
    Revision 0 boards also did not include a color killer in text mode, making text
    fringey on color TVs/monitors.

    ROM contains original non-autostart Monitor and Integer BASIC.  The Autostart
    ROM was later offered as an upgrade (A2M0027); it was sometimes mounted on the
    language card.

    To boot a floppy disk from the monitor, assuming the disk controller is in
    slot 6, input the command 6^P (where ^P stands for control-P).  The monitor
    command ^B will enter BASIC directly.

II Plus: RAM options reduced to 16/32/48 KB.
    ROM expanded to 12KB from $D000-$FFFF containing Applesoft BASIC and
    the Autostart Monitor.  Applesoft is a licensed version of Microsoft's
    6502 BASIC as also found in Commodore and many other computers.


    Users of both models often connected the SHIFT key to one of the game I/O
    inputs, typically joystick switch #2 (read from $C063), in order to inform
    properly written software that characters were intended to be upper/lower
    case.  A few word processors also demand that the CONTROL key be connected
    in place of another game I/O input.

    Both models commonly included a RAM "language card" in slot 0 which added 16K
    of RAM which could be banked into the $D000-$FFFF space to replace the ROMs.
    This allowed running Applesoft on a II and Integer BASIC on a II Plus.
    A II Plus with this card installed is often called a "64K Apple II"; this is
    the base configuration required to run ProDOS and some larger games.

************************************************************************/

#include "emu.h"

#include "apple2common.h"
#include "apple2video.h"

#include "bus/a2bus/a2bus.h"
#include "bus/a2bus/cards.h"
#include "bus/a2gameio/gameio.h"
#include "bus/a2kbd/a2kbd.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/74259.h"
#include "machine/ram.h"

#include "sound/spkrdev.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"


namespace {

#define A2_CPU_TAG "maincpu"
#define A2_SPEAKER_TAG "speaker"
#define A2_CASSETTE_TAG "tape"
#define A2_UPPERBANK_TAG "inhbank"
#define A2_VIDEO_TAG "a2video"

class apple2_state : public driver_device
{
public:
	apple2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, A2_CPU_TAG),
		m_screen(*this, "screen"),
		m_ram(*this, RAM_TAG),
		m_kbd(*this, "kbd"),
		m_video(*this, A2_VIDEO_TAG),
		m_a2common(*this, "a2common"),
		m_a2bus(*this, "a2bus"),
		m_gameio(*this, "gameio"),
		m_sysconfig(*this, "a2_config"),
		m_speaker(*this, A2_SPEAKER_TAG),
		m_cassette(*this, A2_CASSETTE_TAG),
		m_softlatch(*this, "softlatch"),
		m_upperbank(*this, A2_UPPERBANK_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram;
	required_device<a2kbd_connector_device> m_kbd;
	required_device<a2_video_device_composite> m_video;
	required_device<apple2_common_device> m_a2common;
	required_device<a2bus_device> m_a2bus;
	required_device<apple2_gameio_device> m_gameio;
	required_ioport m_sysconfig;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<addressable_latch_device> m_softlatch;
	memory_view m_upperbank;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);
	u8 keyb_data_r();
	u8 keyb_clr_strobe_r();
	void keyb_clr_strobe_w(u8 data);
	u8 cassette_toggle_r();
	void cassette_toggle_w(u8 data);
	u8 speaker_toggle_r();
	void speaker_toggle_w(u8 data);
	u8 utility_strobe_r();
	void utility_strobe_w(u8 data);
	u8 switches_r(offs_t offset);
	u8 flags_r(offs_t offset);
	u8 controller_strobe_r();
	void controller_strobe_w(u8 data);
	u8 c080_r(offs_t offset);
	void c080_w(offs_t offset, u8 data);
	u8 c100_r(offs_t offset);
	void c100_w(offs_t offset, u8 data);
	u8 c800_r(offs_t offset);
	void c800_w(offs_t offset, u8 data);
	u8 inh_r(offs_t offset);
	void inh_w(offs_t offset, u8 data);
	void a2bus_irq_w(int state);
	void a2bus_nmi_w(int state);
	void a2bus_inh_w(int state);
	void keyb_data_w(u8 data);
	void keyb_strobe_w(int state);
	void keyb_reset_w(int state);

	void apple2_common(machine_config &config);
	void apple2jp(machine_config &config);
	void apple2(machine_config &config);
	void uniap2(machine_config &config);
	void am64(machine_config &config);
	void am100(machine_config &config);
	void dodo(machine_config &config);
	void albert(machine_config &config);
	void ivelultr(machine_config &config);
	void apple2p(machine_config &config);
	void apple2pe(machine_config &config);
	void apple2_map(address_map &map) ATTR_COLD;

private:
	int m_speaker_state, m_cassette_state;

	double m_joystick_x1_time, m_joystick_y1_time, m_joystick_x2_time, m_joystick_y2_time;

	u16 m_lastchar, m_strobe;
	u8 m_transchar;
	bool m_anykeydown;

	int m_inh_slot;
	int m_cnxx_slot;

	u8 *m_ram_ptr;
	int m_ram_size;

	int m_inh_bank;

	bool m_reset_latch;

	double m_x_calibration, m_y_calibration;

	device_a2bus_card_interface *m_slotdevice[8];

	u8 read_floatingbus();

	offs_t dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);
};

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define JOYSTICK_DELTA          80
#define JOYSTICK_SENSITIVITY    50
#define JOYSTICK_AUTOCENTER     80

offs_t apple2_state::dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	return m_a2common->dasm_override(stream, pc, opcodes, params);
}

void apple2_state::a2bus_irq_w(int state)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, state);
}

void apple2_state::a2bus_nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

// This code makes a ton of assumptions because we can guarantee a pre-IIe machine!
void apple2_state::a2bus_inh_w(int state)
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
							m_upperbank.select(1);
							m_inh_bank = 1;
						}
					}
					else
					{
						if (m_inh_bank != 0)
						{
							m_upperbank.select(0);
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
			m_upperbank.select(0);
			m_inh_bank = 0;
		}
	}
}

/***************************************************************************
    START/RESET
***************************************************************************/

void apple2_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);
	m_cassette_state = 0;
	m_cassette->output(-1.0f);
	m_upperbank.select(0);
	m_inh_slot = -1;
	m_inh_bank = 0;
	m_anykeydown = false;
	m_transchar = 0;

	// precalculate joystick time constants
	m_x_calibration = attotime::from_nsec(10800).as_double();
	m_y_calibration = attotime::from_nsec(10800).as_double();

	// cache slot devices
	for (int i = 0; i <= 7; i++)
	{
		m_slotdevice[i] = m_a2bus->get_a2bus_card(i);
	}

	for (int adr = 0; adr < m_ram_size; adr += 2)
	{
		m_ram_ptr[adr] = 0;
		m_ram_ptr[adr+1] = 0xff;
	}

	m_joystick_x1_time = m_joystick_x2_time = m_joystick_y1_time = m_joystick_y2_time = 0;
	m_reset_latch = true;

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
	save_item(NAME(m_anykeydown));
	save_item(NAME(m_reset_latch));

	// setup video pointers
	m_video->set_ram_pointers(m_ram_ptr, m_ram_ptr);
	m_video->set_char_pointer(memregion("gfx1")->base(), memregion("gfx1")->bytes());
}

void apple2_state::machine_reset()
{
	// m_inh_slot is not reset here since bootable cards may want to override the monitor
	m_cnxx_slot = -1;
	m_strobe = 0;
	m_kbd->ack_w(0);
}

/***************************************************************************
    I/O
***************************************************************************/

u8 apple2_state::keyb_data_r()
{
	// keyboard latch
	return m_transchar | m_strobe;
}

u8 apple2_state::keyb_clr_strobe_r()
{
	// reads floating bus, clears strobe
	if (!machine().side_effects_disabled())
	{
		if (m_strobe)
			m_kbd->ack_w(0);
		m_strobe = 0;
	}
	return read_floatingbus();
}

void apple2_state::keyb_clr_strobe_w(u8 data)
{
	// clear keyboard latch
	if (m_strobe)
		m_kbd->ack_w(0);
	m_strobe = 0;
}

u8 apple2_state::cassette_toggle_r()
{
	if (!machine().side_effects_disabled())
		cassette_toggle_w(0);
	return read_floatingbus();
}

void apple2_state::cassette_toggle_w(u8 data)
{
	m_cassette_state ^= 1;
	m_cassette->output(m_cassette_state ? 1.0f : -1.0f);
}

u8 apple2_state::speaker_toggle_r()
{
	if (!machine().side_effects_disabled())
		speaker_toggle_w(0);
	return read_floatingbus();
}

void apple2_state::speaker_toggle_w(u8 data)
{
	m_speaker_state ^= 1;
	m_speaker->level_w(m_speaker_state);
}

u8 apple2_state::utility_strobe_r()
{
	if (!machine().side_effects_disabled())
		utility_strobe_w(0);
	return read_floatingbus();
}

void apple2_state::utility_strobe_w(u8 data)
{
	// low pulse on pin 5 of game I/O connector
	m_gameio->strobe_w(0);
	m_gameio->strobe_w(1);
}

u8 apple2_state::switches_r(offs_t offset)
{
	const u8 uFloatingBus = read_floatingbus(); // video side-effects latch after reading
	if (!machine().side_effects_disabled())
		m_softlatch->write_bit((offset & 0x0e) >> 1, offset & 0x01);
	return uFloatingBus;
}

u8 apple2_state::flags_r(offs_t offset)
{
	const u8 uFloatingBus7 = read_floatingbus() & 0x7f;

	// Y output of 74LS251 at H14 read as D7
	switch (offset)
	{
	case 0: // cassette in, inverted (accidentally read at $C068 by ProDOS to attempt IIgs STATE register)
		return (m_cassette->input() > 0.0 ? 0 : 0x80) | uFloatingBus7;

	case 1:  // button 0
		return (m_gameio->sw0_r() ? 0x80 : 0) | uFloatingBus7;

	case 2:  // button 1
		return (m_gameio->sw1_r() ? 0x80 : 0) | uFloatingBus7;

	case 3:  // button 2 (or SHIFT key, with SHIFT key mod)
		return (((m_sysconfig->read() & 0x04) ? m_kbd->shift_r() : m_gameio->sw2_r()) ? 0x80 : 0) | uFloatingBus7;

	case 4:  // joy 1 X axis
		if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
		return ((machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0) | uFloatingBus7;

	case 5:  // joy 1 Y axis
		if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
		return ((machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0) | uFloatingBus7;

	case 6: // joy 2 X axis
		if (m_sysconfig->read() & 0x08) return (m_kbd->shift_r() ? 0x80 : 0) | uFloatingBus7;
		if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
		return ((machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0) | uFloatingBus7;

	case 7: // joy 2 Y axis
		if (m_sysconfig->read() & 0x08) return (m_kbd->control_r() ? 0x80 : 0) | uFloatingBus7;
		if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
		return ((machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0) | uFloatingBus7;
	}

	// this is never reached
	return 0;
}

u8 apple2_state::controller_strobe_r()
{
	if (!machine().side_effects_disabled())
		controller_strobe_w(0);
	return read_floatingbus();
}

void apple2_state::controller_strobe_w(u8 data)
{
	// 558 monostable one-shot timers; a running timer cannot be restarted
	if (machine().time().as_double() >= m_joystick_x1_time)
	{
		m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl0_r();
	}
	if (machine().time().as_double() >= m_joystick_y1_time)
	{
		m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl1_r();
	}
	if (machine().time().as_double() >= m_joystick_x2_time)
	{
		m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl2_r();
	}
	if (machine().time().as_double() >= m_joystick_y2_time)
	{
		m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl3_r();
	}

}

u8 apple2_state::c080_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		int slot;

		offset &= 0x7F;
		slot = offset / 0x10;

		if (m_slotdevice[slot] != nullptr)
		{
			return m_slotdevice[slot]->read_c0nx(offset % 0x10);
		}
	}

	return read_floatingbus();
}

void apple2_state::c080_w(offs_t offset, u8 data)
{
	int slot;

	offset &= 0x7F;
	slot = offset / 0x10;

	if (m_slotdevice[slot] != nullptr)
	{
		m_slotdevice[slot]->write_c0nx(offset % 0x10, data);
	}
}

u8 apple2_state::c100_r(offs_t offset)
{
	int slotnum;

	slotnum = ((offset>>8) & 0xf) + 1;

	if (m_slotdevice[slotnum] != nullptr)
	{
		if ((m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
		}

		return m_slotdevice[slotnum]->read_cnxx(offset&0xff);
	}

	return read_floatingbus();
}

void apple2_state::c100_w(offs_t offset, u8 data)
{
	int slotnum;

	slotnum = ((offset>>8) & 0xf) + 1;

	if (m_slotdevice[slotnum] != nullptr)
	{
		if ((m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
		}

		m_slotdevice[slotnum]->write_cnxx(offset&0xff, data);
	}
}

u8 apple2_state::c800_r(offs_t offset)
{
	if (offset == 0x7ff)
	{
		u8 rv = 0xff;

		if ((m_cnxx_slot != -1) && (m_slotdevice[m_cnxx_slot] != nullptr))
		{
			rv = m_slotdevice[m_cnxx_slot]->read_c800(offset&0xfff);
		}

		if (!machine().side_effects_disabled())
		{
			m_cnxx_slot = -1;
		}

		return rv;
	}

	if ((m_cnxx_slot != -1) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		return m_slotdevice[m_cnxx_slot]->read_c800(offset&0xfff);
	}

	return read_floatingbus();
}

void apple2_state::c800_w(offs_t offset, u8 data)
{
	if ((m_cnxx_slot != -1) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		m_slotdevice[m_cnxx_slot]->write_c800(offset&0xfff, data);
	}

	if (offset == 0x7ff)
	{
		if (!machine().side_effects_disabled())
		{
			m_cnxx_slot = -1;
		}

		return;
	}
}

u8 apple2_state::inh_r(offs_t offset)
{
	if (m_inh_slot != -1)
	{
		return m_slotdevice[m_inh_slot]->read_inh_rom(offset + 0xd000);
	}

	assert(0);  // hitting inh_r with invalid m_inh_slot should not be possible
	return read_floatingbus();
}

void apple2_state::inh_w(offs_t offset, u8 data)
{
	if ((m_inh_slot != -1) && (m_slotdevice[m_inh_slot] != nullptr))
	{
		m_slotdevice[m_inh_slot]->write_inh_rom(offset + 0xd000, data);
	}
}

// floating bus code from old machine/apple2: now works reasonably well with French Touch and Deater "vapor lock" stuff
u8 apple2_state::read_floatingbus()
{
	enum
	{
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
	Hires    = (m_video->get_hires() && m_video->get_graphics()) ? 1 : 0;
	Mixed    = m_video->get_mix() ? 1 : 0;
	Page2    = m_video->get_page2() ? 1 : 0;
	_80Store = 0;

	// calculate video parameters according to display standard
	//
	ScanLines  = 1 ? kNTSCScanLines : kPALScanLines; // FIX: NTSC only?
	// VSyncLine  = 1 ? kNTSCVSyncLine : kPALVSyncLine; // FIX: NTSC only?
	// ScanCycles = ScanLines * kHClocks;

	// calculate horizontal scanning state
	h_clock = (i + 63) % kHClocks; // which horizontal scanning clock
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
	v_line  = (i / kHClocks) + 192; // which vertical scanning line
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
		if ((kHPEClock <= h_clock) && // Y: HBL?
			(h_clock <= (kHClocks - 1)))
		{
			address |= 1 << 12; // Y: a12 (add $1000 to address!)
		}
	}

	return m_ram_ptr[address % m_ram_size];
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

u8 apple2_state::ram_r(offs_t offset)
{
	if (offset < m_ram_size)
	{
		return m_ram_ptr[offset];
	}

	return 0xff;
}

void apple2_state::ram_w(offs_t offset, u8 data)
{
	if (offset < m_ram_size)
	{
		m_ram_ptr[offset] = data;
	}
}

void apple2_state::apple2_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(FUNC(apple2_state::ram_r), FUNC(apple2_state::ram_w));
	map(0xc000, 0xc000).mirror(0xf).r(FUNC(apple2_state::keyb_data_r)).nopw();
	map(0xc010, 0xc010).mirror(0xf).rw(FUNC(apple2_state::keyb_clr_strobe_r), FUNC(apple2_state::keyb_clr_strobe_w));
	map(0xc020, 0xc020).mirror(0xf).rw(FUNC(apple2_state::cassette_toggle_r), FUNC(apple2_state::cassette_toggle_w));
	map(0xc030, 0xc030).mirror(0xf).rw(FUNC(apple2_state::speaker_toggle_r), FUNC(apple2_state::speaker_toggle_w));
	map(0xc040, 0xc040).mirror(0xf).rw(FUNC(apple2_state::utility_strobe_r), FUNC(apple2_state::utility_strobe_w));
	map(0xc050, 0xc05f).r(FUNC(apple2_state::switches_r)).w(m_softlatch, FUNC(addressable_latch_device::write_a0));
	map(0xc060, 0xc067).mirror(0x8).r(FUNC(apple2_state::flags_r)).nopw(); // includes IIgs STATE register, which ProDOS touches
	map(0xc070, 0xc070).mirror(0xf).rw(FUNC(apple2_state::controller_strobe_r), FUNC(apple2_state::controller_strobe_w));
	map(0xc080, 0xc0ff).rw(FUNC(apple2_state::c080_r), FUNC(apple2_state::c080_w));
	map(0xc100, 0xc7ff).rw(FUNC(apple2_state::c100_r), FUNC(apple2_state::c100_w));
	map(0xc800, 0xcfff).rw(FUNC(apple2_state::c800_r), FUNC(apple2_state::c800_w));
	map(0xd000, 0xffff).view(m_upperbank);
	m_upperbank[0](0xd000, 0xffff).rom().region("maincpu", 0x1000).w(FUNC(apple2_state::inh_w));
	m_upperbank[1](0xd000, 0xffff).rw(FUNC(apple2_state::inh_r), FUNC(apple2_state::inh_w));
}

/***************************************************************************
    KEYBOARD
***************************************************************************/

void apple2_state::keyb_data_w(u8 data)
{
	m_transchar = data;
}

void apple2_state::keyb_strobe_w(int state)
{
	if (m_anykeydown != state)
	{
		m_anykeydown = state;
		if (state)
		{
			// if the user presses a valid key to start the driver from the info screen,
			// we will see that key.  ignore keys in the first 25,000 cycles (in my tests,
			// the unwanted key shows up at 17030 cycles)
			if (m_maincpu->total_cycles() < 25000)
			{
				return;
			}

			m_strobe = 0x80;
			m_kbd->ack_w(1);
		}
	}
}

void apple2_state::keyb_reset_w(int state)
{
	if (state && !m_reset_latch)
	{
		// allow cards to see reset
		m_a2bus->reset_bus();
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else if (!state && m_reset_latch)
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_reset_latch = state;
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

INPUT_PORTS_START( apple2_sysconfig )
	PORT_START("a2_config")
	PORT_CONFNAME(0x0c, 0x04, "Shift key mod")  // default to installed
	PORT_CONFSETTING(0x00, "Not connected")
	PORT_CONFSETTING(0x04, "SW2 = Shift")
	PORT_CONFSETTING(0x08, "PADDL2 = Shift, PADDL3 = Ctrl") // Zardax Word Processor expects this as default
INPUT_PORTS_END

static INPUT_PORTS_START( apple2 )
	PORT_INCLUDE(apple2_sysconfig)
INPUT_PORTS_END

void apple2_state::apple2_common(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1021800);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2_state::apple2_map);
	m_maincpu->set_dasm_override(FUNC(apple2_state::dasm_trampoline));

	config.set_maximum_quantum(attotime::from_hz(60));

	APPLE2_VIDEO_COMPOSITE(config, m_video, XTAL(14'318'181)).set_screen(m_screen);
	APPLE2_COMMON(config, m_a2common, XTAL(14'318'181));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::II, true, true>)));
	m_screen->set_palette(m_video);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, A2_SPEAKER_TAG).add_route(ALL_OUTPUTS, "mono", 0.4);

	/* soft switches */
	F9334(config, m_softlatch); // F14 (labeled 74LS259 on some boards and in the Apple ][ Reference Manual)
	m_softlatch->q_out_cb<0>().set(m_video, FUNC(a2_video_device::txt_w));
	m_softlatch->q_out_cb<1>().set(m_video, FUNC(a2_video_device::mix_w));
	m_softlatch->q_out_cb<2>().set(m_video, FUNC(a2_video_device::scr_w));
	m_softlatch->q_out_cb<3>().set(m_video, FUNC(a2_video_device::res_w));
	m_softlatch->q_out_cb<4>().set(m_gameio, FUNC(apple2_gameio_device::an0_w));
	m_softlatch->q_out_cb<5>().set(m_gameio, FUNC(apple2_gameio_device::an1_w));
	m_softlatch->q_out_cb<6>().set(m_gameio, FUNC(apple2_gameio_device::an2_w));
	m_softlatch->q_out_cb<6>().append(m_video, FUNC(a2_video_device::an2_w));
	m_softlatch->q_out_cb<7>().set(m_gameio, FUNC(apple2_gameio_device::an3_w));
	m_softlatch->q_out_cb<7>().append(m_kbd, FUNC(a2kbd_connector_device::an3_w));

	APPLE2_GAMEIO(config, m_gameio, m_screen, apple2_gameio_device::iiandplus_options, nullptr);

	A2KBD_CONNECTOR(config, m_kbd, &a2kbd_connector_device::default_options, "nkbd");
	m_kbd->b_callback().set(FUNC(apple2_state::keyb_data_w));
	m_kbd->strobe_callback().set(FUNC(apple2_state::keyb_strobe_w));
	m_kbd->reset_callback().set(FUNC(apple2_state::keyb_reset_w));

	/* slot devices */
	A2BUS(config, m_a2bus, 0);
	m_a2bus->set_space(m_maincpu, AS_PROGRAM);
	m_a2bus->irq_w().set(FUNC(apple2_state::a2bus_irq_w));
	m_a2bus->nmi_w().set(FUNC(apple2_state::a2bus_nmi_w));
	m_a2bus->inh_w().set(FUNC(apple2_state::a2bus_inh_w));
	m_a2bus->dma_w().set_inputline(m_maincpu, INPUT_LINE_HALT);
	A2BUS_SLOT(config, "sl0", XTAL(14'318'181) / 2, m_a2bus, apple2_slot0_cards, "lang");
	A2BUS_SLOT(config, "sl1", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl2", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl3", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl4", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, "mockingboard");
	A2BUS_SLOT(config, "sl5", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl6", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, "diskiing");
	A2BUS_SLOT(config, "sl7", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);

	/* Set up the softlists: clean cracks priority, originals second, others last */
	SOFTWARE_LIST(config, "flop_a2_clean").set_original("apple2_flop_clcracked");
	SOFTWARE_LIST(config, "flop_a2_orig").set_compatible("apple2_flop_orig").set_filter("A2");
	SOFTWARE_LIST(config, "flop_a2_misc").set_compatible("apple2_flop_misc");
	SOFTWARE_LIST(config, "cass_list").set_original("apple2_cass");
	//MCFG_SOFTWARE_LIST_ADD("cass_list", "apple2_cass")

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("apple2_cass");
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void apple2_state::apple2(machine_config &config)
{
	apple2_common(config);
	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("48K").set_extra_options("4K,8K,12K,16K,20K,24K,32K,36K,48K").set_default_value(0x00);
}

void apple2_state::apple2p(machine_config &config)
{
	apple2_common(config);
	subdevice<software_list_device>("flop_a2_orig")->set_filter("A2P"); // Filter list to compatible disks for this machine.
	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("48K").set_extra_options("16K,32K,48K").set_default_value(0x00);
}

void apple2_state::apple2pe(machine_config &config)
{
	apple2p(config);
	m_kbd->set_default_option("videnh2");
}

void apple2_state::uniap2(machine_config &config)
{
	apple2p(config);
	m_kbd->set_default_option("uniap2ti");
}

void apple2_state::am64(machine_config &config)
{
	apple2p(config);
	m_kbd->set_default_option("tk10");
}

void apple2_state::am100(machine_config &config)
{
	apple2p(config);
	m_kbd->set_default_option("am100");
}

void apple2_state::apple2jp(machine_config &config)
{
	apple2p(config);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::II_J_PLUS, true, true>)));
}

void apple2_state::dodo(machine_config &config)
{
	apple2p(config);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::II, true, false>)));
}

void apple2_state::albert(machine_config &config)
{
	apple2p(config);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IVEL_ULTRA, false, true>)));
}

void apple2_state::ivelultr(machine_config &config)
{
	apple2p(config);
	m_screen->set_screen_update(m_video, NAME((&a2_video_device::screen_update<a2_video_device::model::IVEL_ULTRA, true, false>)));

	m_kbd->set_default_option("ivelultr");
	m_kbd->mode_callback().set(m_video, FUNC(a2_video_device::an2_w)); // not actually AN2 here
}

#if 0
void apple2_state::laba2p(machine_config &config)
{
	apple2p(config);
	MCFG_MACHINE_START_OVERRIDE(apple2_state,laba2p)

	config.device_remove("sl0");
	config.device_remove("sl3");
	config.device_remove("sl6");

//  A2BUS_LAB_80COL("sl3", A2BUS_LAB_80COL).set_onboard(m_a2bus);
	A2BUS_IWM_FDC("sl6", A2BUS_IWM_FDC).set_onboard(m_a2bus);
}
#endif

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(apple2) /* the classic, non-autoboot apple2 with integer basic in rom. optional card with autoboot and applesoft basic was possible but isn't yet supported */
	ROM_REGION(0x0800,"gfx1",0)
	// This is a GI RO-3-2513 on Rev. 0 Apple ][s, as per http://www.solivant.com/php/eview.php?album=appleII&filen=11 which shows serial #97
	// However, the presence of the lo-res patterns means it's a customized-mask variant, and not the same as the Apple I's 2513 that truly is stock.
	ROM_LOAD ( "a2.chr", 0x0000, 0x0800, BAD_DUMP CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659)) /* current dump is 341-0036 which is the appleII+ character generator, not the original appleII one, whose rom number is not yet known! */

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD_OPTIONAL ( "341-0016-00.d0", 0x1000, 0x0800, CRC(4234e88a) SHA1(c9a81d704dc2f0c3416c20f9c4ab71fedda937ed)) /* 341-0016: Programmer's Aid #1 D0 */

	ROM_LOAD ( "341-0001-00.e0", 0x2000, 0x0800, CRC(c0a4ad3b) SHA1(bf32195efcb34b694c893c2d342321ec3a24b98f)) /* Needs verification. From eBay: Label: S7925E // C48077 // 3410001-00 // (C)APPLE78 E0 */
	ROM_LOAD ( "341-0002-00.e8", 0x2800, 0x0800, CRC(a99c2cf6) SHA1(9767d92d04fc65c626223f25564cca31f5248980)) /* Needs verification. From eBay: Label: S7916E // C48078 // 3410002-00 // (C)APPLE78 E8 */
	ROM_LOAD ( "341-0003-00.f0", 0x3000, 0x0800, CRC(62230d38) SHA1(f268022da555e4c809ca1ae9e5d2f00b388ff61c)) /* Needs verification. From eBay: Label: S7908E // C48709 // 3410003 // CAPPLE78 F0 */
	ROM_SYSTEM_BIOS(0, "default", "Original Monitor")
	ROMX_LOAD ( "341-0004-00.f8", 0x3800, 0x0800, CRC(020a86d0) SHA1(52a18bd578a4694420009cad7a7a5779a8c00226), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "autostart", "Autostart Monitor")
	ROMX_LOAD ( "341-0020-00.f8", 0x3800, 0x0800, CRC(079589c4) SHA1(a28852ff997b4790e53d8d0352112c4b1a395098), ROM_BIOS(1)) /* 341-0020-00: Autostart Monitor/Applesoft Basic $f800; Label(from Apple Language Card - Front.jpg): S 8115 // C68018 // 341-0020-00 */
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
	ROM_LOAD ( "341-0020-00.f8", 0x3800, 0x0800, CRC(079589c4) SHA1(a28852ff997b4790e53d8d0352112c4b1a395098)) /* 341-0020-00: Autostart Monitor/Applesoft Basic $f800; Label(from Apple Language Card - Front.jpg): S 8115 // C68018 // 341-0020-00 */
ROM_END

ROM_START(apple2pe)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "videx lower case chip rom.bin", 0x0000, 0x0800, CRC(00f68076) SHA1(447874fe0850c8add3fd5b13fa98f6648fe6f999))

	ROM_REGION(0x4000, "maincpu", ROMREGION_LE)
	ROM_LOAD ( "341-0011.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "341-0015.f0", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "monitor.bin", 0x3800, 0x0800, CRC(65d726d3) SHA1(2186b371b96fc279338fe67c5e351438d86f4f7f)) // Autostart ROM patched as described on p. A-4 of Enhancer ][ manual
ROM_END

ROM_START(ace1000)
	ROM_REGION(0x1000,"gfx1",0)
	// TBD: second half doesn't look like a normal character set
	ROM_LOAD ( "1331409.u7", 0x0000, 0x1000, CRC(744c06e1) SHA1(e1c11495ee538f658d2918bfece8c4629f60fa13))

	ROM_REGION(0x4000, "maincpu", ROMREGION_LE)
	ROM_LOAD ( "1331401.g2", 0x3800, 0x0800, CRC(047d6fed) SHA1(83d225dc3f1a7bd6901cc24cd02287402022469f))
	ROM_LOAD ( "1331402.g3", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "1331403.g5", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "1331404.g7", 0x2000, 0x0800, CRC(1bee5169) SHA1(0cd57b4a2a79e0fc7f35619edc1be00952947b82))
	ROM_LOAD ( "1331405.g8", 0x1800, 0x0800, CRC(2a63f50a) SHA1(7cf424e7adbc84a4aa6f11d0132bf494bbb6a247))
	ROM_LOAD ( "1331406.g10", 0x1000, 0x0800, CRC(bfdd3cc6) SHA1(20067d27eb3b5bb03e7b560e44383e0926e39cbb))

	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD ( "348.bin", 0x000, 0x800, NO_DUMP) // EPROM for custom Key Tronic keyboard with INS8035(?) MCU (exact size unclear)
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
	ROM_LOAD("pravetz82_chr.bin", 0x000000, 0x000800, CRC(c5d6bbc2) SHA1(b2074675d1890b5d1f0a14ed1758665f190ea3c7))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "pravetz82.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "pravetz82.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "pravetz82.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "pravetz82.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "pravetz82.f0", 0x3000, 0x0800, CRC(e26d9d35) SHA1(ce6e42e6c9a6c98e92522af7a6090cd04c56c778))
	ROM_LOAD ( "pravetz82.f8", 0x3800, 0x0800, CRC(57547818) SHA1(db30bedec98305e31a14acb9e2a92be1c4853807))
ROM_END

ROM_START(prav8m)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD("pravetz8m_chr.bin", 0x000000, 0x002000, CRC(72244022) SHA1(4db7544e049bc7aeab4b4da2f8ef9fbeb3ceff24))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "pravetz8m.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "pravetz8m.d8", 0x1800, 0x0800, CRC(654b6f7b) SHA1(f7b1457b48fe6974c4de7e976df3a8fca6b7b661))
	ROM_LOAD ( "pravetz8m.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "pravetz8m.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "pravetz8m.f0", 0x3000, 0x0800, CRC(e26d9d35) SHA1(ce6e42e6c9a6c98e92522af7a6090cd04c56c778))
	ROM_LOAD ( "pravetz8m.f8", 0x3800, 0x0800, CRC(5bab0a46) SHA1(f6c0817ce37d2e2c43f482c339acaede0a73359b))
ROM_END

ROM_START(craft2p)
	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD( "gc.bin",       0x000000, 0x001000, CRC(93e4a754) SHA1(25f5f5fd1cbd763d43362e80de3acc5b34a25963) ) // both halves identical

	ROM_REGION(0x4000,"maincpu",0)
	// the d0 and e0 ROMs match the Unitron English ones, only f0 differs
	ROM_LOAD ( "unitron_en.d0", 0x1000, 0x1000, CRC(24d73c7b) SHA1(d17a15868dc875c67061c95ec53a6b2699d3a425))
	ROM_LOAD ( "unitron.e0"   , 0x2000, 0x1000, CRC(0d494efd) SHA1(a2fd1223a3ca0cfee24a6afe66ea3c4c144dd98e))
	ROM_LOAD ( "craftii-roms-f0-f7.bin", 0x3000, 0x1000, CRC(3f9dea08) SHA1(0e23bc884b8108675267d30b85b770066bdd94c9) )
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

ROM_START(microeng)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "microengenho_6c.bin", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "microengenho_d0_d8.bin", 0x1000, 0x1000, CRC(834eabf4) SHA1(9a2385c6df16e5f5d15b79da17d21bf0f99dbd08))
	ROM_LOAD ( "microengenho_e0_e8.bin", 0x2000, 0x1000, CRC(0d494efd) SHA1(a2fd1223a3ca0cfee24a6afe66ea3c4c144dd98e))
	ROM_LOAD ( "microengenho_f0_f8.bin", 0x3000, 0x1000, CRC(588717cf) SHA1(e2a867c4a390d65e5ea181a4f933abb9992e4a63))
ROM_END

/*
    J-Plus ROM numbers confirmed by:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Computers/Apple%20II/Apple%20II%20j-plus/Photos/Apple%20II%20j-plus%20-%20Motherboard.jpg

*/

ROM_START(apple2jp)
	ROM_REGION(0x0800,"gfx1",0)
	// probably a custom-mask variant of the Signetics 2513N or equivalent
	ROM_LOAD ( "a2jp.chr", 0x0000, 0x0800, CRC(487104b5) SHA1(0a382be58db5215c4a3de53b19a72fab660d5da2))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "341-0011.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "341-0015.f0", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
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
	// Both 4K halves identical; PCB photos suggest this is an overdumped 2732 (not 2716, despite silkscreen)
	ROM_LOAD( "space 84 mobo chargen.bin", 0x0000, 0x2000, CRC(ceb98990) SHA1(8b2758da611bcfdd3d144edabc63ef1df2ca787b) )

	ROM_REGION(0x4000,"maincpu",0)
	// IBS SPACE 84 actually has 24K of firmware in three 2764 EPROMs.
	// Unlike some extant 8K dumps, the original ROMs definitely contain more than just hacked copies of Applesoft BASIC and the autostart monitor.
	ROM_LOAD ( "341-0011.d0",  0x1000, 0x0800, BAD_DUMP CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8",  0x1800, 0x0800, BAD_DUMP CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0",  0x2000, 0x0800, BAD_DUMP CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8",  0x2800, 0x0800, BAD_DUMP CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD( "space84_f.bin", 0x3000, 0x1000, BAD_DUMP CRC(6acfe2a2) SHA1(07b19cd96101634543a3690b9f0d650e4d4d5675)) // single-bit corruption patched at $F010, $F810, $FCF0, $FD10

	ROM_REGION(0x1d6, "plds", 0)
	ROM_LOAD( "80.f2", 0x000, 0x0eb, NO_DUMP) // N82S15x?
	ROM_LOAD( "81.k3", 0x0eb, 0x0eb, NO_DUMP) // N82S15x?

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD( "d8748.bin", 0x000, 0x400, NO_DUMP)
ROM_END

ROM_START(am64)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "gm-2716.bin",  0x0000, 0x0800, CRC(863e657f) SHA1(cc954204c503bc545ec0d08862483aaad83805d5) )

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "am64-27128.bin", 0x0000, 0x4000, CRC(f25cdc7b) SHA1(235e72b77695938a9df8781f5bea3cbbbe1f4c76) )

	ROM_REGION(0x2000, "spares", 0)
	// parallel card ROM
	ROM_LOAD( "ap-2716.bin",  0x0000, 0x0800, CRC(c6990f08) SHA1(e7daf63639234e46738a4d78a49287d11ccaf537) )
ROM_END

ROM_START(ivelultr)
	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD( "ultra.chr", 0x0000, 0x1000,CRC(fed62c85) SHA1(479fb3f38a3f7332cef2e8c4856871afe8dc6017))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "ultra1.bin", 0x2000, 0x1000, CRC(8ab49c1c) SHA1(b41da28a40c3a22bc10a954a86716a1a2bae04a4))
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_LOAD( "ultra2.bin", 0x3000, 0x1000, CRC(1ac1e17e) SHA1(a5b8adec37da91970c303905b5e2c4d1b715ee4e))
ROM_END

ROM_START(laser2c)
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "g1.bin",       0x000000, 0x001000, BAD_DUMP CRC(7ad15cc4) SHA1(88c60ec0b008eccdbece09d18fe905380ddc070f) ) // both halves identical (TBD: is this really a bad dump?)

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "g2.bin",       0x000000, 0x001000, CRC(f1d92f9c) SHA1(a54d55201f04af4c24bf94450d2cd1fa87c2c259) ) // probably video timing related

	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD( "laser.bin",    0x001000, 0x002000, CRC(8b975094) SHA1(eea53530b4a3777afa00d2979abedf84fac62e08) )
	ROM_LOAD( "mon.bin",      0x003000, 0x001000, CRC(978c083f) SHA1(14e87cb717780b19db75c313004ba4d6ef20bc26) )
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

ROM_START( basis108 )
	ROM_REGION(0x4000, "maincpu", 0) // all roms overdumped
	ROM_LOAD( "d0.d83",   0x1000, 0x0800, CRC(bb4ac440) SHA1(7901203845adab588850ae35f81e4ee2a2248686) )
	ROM_IGNORE( 0x0800 )
	ROM_LOAD( "d8.d70",   0x1800, 0x0800, CRC(3e8cdbcd) SHA1(b2a418818e4130859afd6c08b5695328a3edd2c5) )
	ROM_IGNORE( 0x0800 )
	ROM_LOAD( "e0.d56",   0x2000, 0x0800, CRC(0575ba28) SHA1(938884eb3ebd0870f99df33ee7a03e93cd625ab4) )
	ROM_IGNORE( 0x0800 )
	ROM_LOAD( "e8.d40",   0x2800, 0x0800, CRC(fc7229f6) SHA1(380ffcf0dba008f0bc43a483931e98034b1d0d52) )
	ROM_IGNORE( 0x0800 )
	ROM_LOAD( "f0.d39",   0x3000, 0x0800, CRC(bae4b24d) SHA1(b5ffc9b3552b13b2f577a42196addae71289203d) )
	ROM_IGNORE( 0x0800 )
	ROM_LOAD( "f8.d25",   0x3800, 0x0800, CRC(f84efac5) SHA1(66b7eadfdb938cda0de01dbeab1b74aa88bd096c) )
	ROM_IGNORE( 0x0800 )

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD( "cg.d29",   0x0000, 0x1000, CRC(120de575) SHA1(e6e4e357b3834a143df9e5834abfb4a9139457d4) )

	ROM_REGION(0x1000, "cg80col", 0)
	ROM_LOAD( "dispcard_cg.bin",         0x0000, 0x1000, CRC(cf84811c) SHA1(135f4f35607dd74941f0a3cae813227bf8a8a020) )

	ROM_REGION(0x1000, "fw80col", 0)
	ROM_LOAD( "dispcard_ctrl_17.43.bin", 0x0000, 0x0800, CRC(bf04eda4) SHA1(86047c0ec6b06d647b95304d7f95d3d116f60e4a) )

	ROM_REGION(0x800, "diskii", 0)
	ROM_LOAD( "fdccard_fdc4_slot6.bin",  0x0000, 0x0800, CRC(2bd452bb) SHA1(10ba81d34117ef713c546d748bf0e1a8c04d1ae3) )

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD( "m5l8048-068p.ic1", 0x000, 0x400, NO_DUMP ) // Cherry keyboard with unique connector
ROM_END

// The bit1 and bit2 of each byte swap positions.
// 原机器ROM每个字节的第1位和第2位互换了位置
ROM_START(hkc8800a)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "341-0036.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x4000, "maincpu", ROMREGION_LE)
	ROM_LOAD ( "hkc8800a_c0.bin", 0x0000, 0x0800, CRC(8dceea26) SHA1(57623fd9ddef05cb56e8f0bcf0baa8902ebba2bb))
	ROM_LOAD ( "hkc8800a_c8.bin", 0x0800, 0x0800, CRC(a337c7b5) SHA1(bc3f021a85124785b78dd781fcabc66bc5645515))
	ROM_LOAD ( "341-0011.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "341-0015.f0", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "hkc8800a_f8.bin", 0x3800, 0x0800, CRC(f2287c5f) SHA1(0b6c2d6df11a0aa8c5737831758d9668fce11887))
ROM_END

ROM_START(albert)
	ROM_REGION(0x2000,"gfx1",0)
	// TBD: second half doesn't look like a normal character set
	ROM_LOAD( "albert 95-6005_rom_2732.bin", 0x0000, 0x1000, CRC(30df7410) SHA1(cb884efb12992e8a0140fdf6368b0268b6c0df8c) )

	ROM_REGION( 0x1000, "c000", 0 )
	ROM_LOAD( "albert_95-6004_rom_2732.bin", 0x0000, 0x1000, CRC(6d9a435f) SHA1(ce1da16659922daff5bc0065ff45b00d271108f9) ) // onboard I/O slots

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD( "albert_main_rom_27128.bin", 0x0000, 0x4000, CRC(ccf5696b) SHA1(59504a51d91486289330266e851f2ea1719766c1) )

	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD( "keyboard.bin", 0x000, 0x800, NO_DUMP ) // detached serial keyboard with custom layout; microcontroller type unknown
ROM_END

ROM_START(am100)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD( "nfl-asem-am100-u43.bin", 0x0000, 0x0800, CRC(863e657f) SHA1(cc954204c503bc545ec0d08862483aaad83805d5) )

	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("nfl-asem-am100-u24.bin", 0x0000, 0x4000, CRC(2fb0c717) SHA1(cb4f754d3e1aec9603faebc308a4a63466242e43) )
ROM_END

ROM_START(dodo)
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD( "gtac_2_charrom_um2316_a5.bin", 0x0000, 0x0800, CRC(a2dfcfeb) SHA1(adea922f950667d3b24297d2f64de697c28d6c17) )

	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD( "dodo2764.bin", 0x2000, 0x1000, CRC(4b761f87) SHA1(2e1741db8134c4c715ecae480f5bda51d58ae296) )
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_LOAD( "dodo2732.bin", 0x3000, 0x1000, CRC(405cdb0c) SHA1(3ed133eb94ee33194c668c4ee3f67885dd489d13) )
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS          INIT        COMPANY                FULLNAME
COMP( 1977, apple2,   0,      0,      apple2,   apple2, apple2_state, empty_init, "Apple Computer",      "Apple ][", MACHINE_SUPPORTS_SAVE )
COMP( 1979, apple2p,  apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Apple Computer",      "Apple ][+", MACHINE_SUPPORTS_SAVE )
COMP( 1982, apple2pe, apple2, 0,      apple2pe, apple2, apple2_state, empty_init, "hack (Videx)",        "Apple ][+ (with Enhancer ][ lowercase display mod)", MACHINE_SUPPORTS_SAVE )
COMP( 1980, apple2jp, apple2, 0,      apple2jp, apple2, apple2_state, empty_init, "Apple Computer",      "Apple ][ J-Plus", MACHINE_SUPPORTS_SAVE )
COMP( 198?, elppa,    apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Victor do Brasil",    "Elppa II+", MACHINE_SUPPORTS_SAVE )
COMP( 1982, microeng, apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Spectrum Eletronica (SCOPUS)", "Micro Engenho", MACHINE_SUPPORTS_SAVE )
COMP( 1982, maxxi,    apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Polymax",             "Maxxi", MACHINE_SUPPORTS_SAVE )
COMP( 1982, prav82,   apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Pravetz",             "Pravetz 82", MACHINE_SUPPORTS_SAVE )
COMP( 1982, ace100,   apple2, 0,      apple2,   apple2, apple2_state, empty_init, "Franklin Computer",   "Franklin ACE 100", MACHINE_SUPPORTS_SAVE )
COMP( 1982, ace1000,  apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Franklin Computer",   "Franklin ACE 1000", MACHINE_SUPPORTS_SAVE )
COMP( 1982, uniap2en, apple2, 0,      uniap2,   apple2, apple2_state, empty_init, "Unitron Eletronica",  "Unitron AP II (in English)", MACHINE_SUPPORTS_SAVE )
COMP( 1982, uniap2pt, apple2, 0,      uniap2,   apple2, apple2_state, empty_init, "Unitron Eletronica",  "Unitron AP II (in Brazilian Portuguese)", MACHINE_SUPPORTS_SAVE )
//COMP( 1984, uniap2ti, apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Unitron Eletronica",  "Unitron AP II+ (Teclado Inteligente)", MACHINE_SUPPORTS_SAVE )
COMP( 1982, craft2p,  apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Craft",               "Craft II+", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
// reverse font direction + wider character cell -\/
COMP( 1984, ivelultr, apple2, 0,      ivelultr, apple2, apple2_state, empty_init, "Ivasim",              "Ivel Ultra", MACHINE_SUPPORTS_SAVE )
COMP( 1985, prav8m,   apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "Pravetz",             "Pravetz 8M", MACHINE_SUPPORTS_SAVE )
COMP( 1985, space84,  apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "ComputerTechnik/IBS", "Space 84",   MACHINE_SUPPORTS_SAVE )
COMP( 1985, am64,     apple2, 0,      am64,     apple2, apple2_state, empty_init, "ASEM",                "AM 64", MACHINE_SUPPORTS_SAVE )
//COMP( 19??, laba2p,   apple2, 0,      laba2p,   apple2, apple2_state, empty_init, "<unknown>",           "Lab equipment Apple II Plus clone", MACHINE_SUPPORTS_SAVE )
COMP( 1985, laser2c,  apple2, 0,      dodo,     apple2, apple2_state, empty_init, "Milmar",              "Laser //c", MACHINE_SUPPORTS_SAVE )
COMP( 1982, basis108, apple2, 0,      apple2,   apple2, apple2_state, empty_init, "Basis Microcomputer GmbH", "Basis 108", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1984, hkc8800a, apple2, 0,      apple2p,  apple2, apple2_state, empty_init, "China HKC",           "HKC 8800A", MACHINE_SUPPORTS_SAVE )
COMP( 1984, albert,   apple2, 0,      albert,   apple2, apple2_state, empty_init, "Albert Computers, Inc.", "Albert", MACHINE_SUPPORTS_SAVE )
COMP( 198?, am100,    apple2, 0,      am100,    apple2, apple2_state, empty_init, "ASEM S.p.A.",         "AM100",     MACHINE_SUPPORTS_SAVE )
COMP( 198?, dodo,     apple2, 0,      dodo,     apple2, apple2_state, empty_init, "GTAC",                "Do-Do",     MACHINE_SUPPORTS_SAVE )
