// license:BSD-3-Clause
// copyright-holders:R. Belmont, Sergey Svishchev
/***************************************************************************

    agat.cpp

    Driver for Agat series of Soviet Apple II non-clones

    These are similar to Apple II (same bus architecture, keyboard
    and floppy interface), but native video controllers are different.

    agat7 supports Apple (40col, HGR and DHR) video modes with add-on
    card; agat9 has built-in support for 40col and HGR.  Palette in
    Apple modes is different and pixel stretching is not done.

    To do (common):
    - video: use palette rom
    - video: text modes use 7x8 character cell and 224x256 raster
    - video: vertical raster splits (used at least by Rapira)
    - what is the state of devices at init and reset?
    - ignore debugger reads -- use side_effects_disabled()
    - softlists

    To do (agat7):
    - hw variant: 16 colors
    - hw variant: 256-char chargen
    - hw variant: 64K and 128K onboard memory
    - "500hz" interrupt breakage
    - what does write to C009 do? (basedos7.nib)

    To do (agat9):
    - accurate lc emulation (what do writes do? etc.)
    - 840k floppy: support writing, motor off, the rest of .aim format
    - memory expansion boards, a2bus_inh_w
    - hw variant: video status readback ("Moscow")
    - hw variant: agat9a model (via http://agatcomp.ru/Images/case.shtml)
    - mouse via parallel port
    - does cassette output work?
    - what do floating bus reads do?
    - why prodos does not boot?
    - why spriteos does not see printer card?

    Slot devices -- 1st party:
    - agat7: apple2 video card -- decimal 3.089.121 -- http://agatcomp.ru/Images/new_j121.shtml
    - agat7: Serial-parallel card -- decimal 3.089.106
    - agat9: Printer card -- decimal 3.089.174

    Slot devices -- 3rd party:
    - agat7: Mymrin's color expansion (p. 254-255, port C112)
    - IEEE-488, etc. via via http://agatcomp.ru/Reading/period/1s1995_35-usn1.shtml
    - HDC, etc. via http://agatcomp.ru/Hard/losthard.shtml
    - network cards -- via http://agatcomp.ru/Images/new_net.shtml
    - sound cards (5-voice synth card, MIDI i/o card) via http://agatcomp.ru/Images/new_sound.shtml
    - Nippel Mouse -- http://agatcomp.ru/Images/new_mouse.shtml
    - Nippel ADC (digital oscilloscope) -- via http://agatcomp.ru/Images/new_IO.shtml
    - Nippel Co-processor (R65C02 clone + dual-ported RAM) -- via http://agatcomp.ru/Images/new_IO.shtml
    - Sprite Card-93 (fd1793)
    - Sprite Programmable I/O (8035 + 8251 + 8253 + 8255) -- via http://agatcomp.ru/Images/new_IO.shtml
    - Sprite Communication Extension (8251 + 8253 + mc146818) -- via http://agatcomp.ru/Images/new_misc.shtml

    References for Agat-7:
    - http://agatcomp.ru/Reading/docs_txt.shtml (first set of links) (1987)
    - http://agatcomp.ru/Reading/docs_shtat.shtml (1989) -- in particular,
      http://agatcomp.ru/Reading/docs/TO4_5-Hi.djvu

    References for Agat-9:
    - http://agatcomp.ru/Reading/docs_shtat.shtml#l2

************************************************************************/

#include "emu.h"
#include "agatkeyb.h"
#include "agat7.h"
#include "agat9.h"

#include "bus/a2bus/a2diskiing.h"
#include "bus/a2bus/agat7langcard.h"
#include "bus/a2bus/agat7ports.h"
#include "bus/a2bus/agat7ram.h"
#include "bus/a2bus/agat840k_hle.h"
#include "bus/a2bus/agat_fdc.h"
#include "bus/a2bus/nippelclock.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/r65c02.h"
#include "imagedev/cassette.h"
#include "machine/bankdev.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"


namespace {

#define A7_CPU_TAG "maincpu"
#define A7_SPEAKER_TAG "speaker"
#define A7_CASSETTE_TAG "tape"
#define A7_UPPERBANK_TAG "inhbank"
#define A7_VIDEO_TAG "a7video"
#define A9_VIDEO_TAG "a9video"


class agat_base_state : public driver_device
{
public:
	agat_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, A7_CPU_TAG)
		, m_ram(*this, RAM_TAG)
		, m_a2bus(*this, "a2bus")
		, m_joy1x(*this, "joystick_1_x")
		, m_joy1y(*this, "joystick_1_y")
		, m_joy2x(*this, "joystick_2_x")
		, m_joy2y(*this, "joystick_2_y")
		, m_joybuttons(*this, "joystick_buttons")
		, m_speaker(*this, A7_SPEAKER_TAG)
		, m_cassette(*this, A7_CASSETTE_TAG)
		, m_upperbank(*this, A7_UPPERBANK_TAG)
	{ }

	INTERRUPT_GEN_MEMBER(agat_vblank);

	uint8_t c080_r(offs_t offset);
	void c080_w(offs_t offset, uint8_t data);
	uint8_t c100_r(offs_t offset);
	void c100_w(offs_t offset, uint8_t data);
	uint8_t c800_r(offs_t offset);
	void c800_w(offs_t offset, uint8_t data);
	uint8_t inh_r(offs_t offset);
	void inh_w(offs_t offset, uint8_t data);
	void a2bus_irq_w(int state);
	void a2bus_nmi_w(int state);
	void a2bus_inh_w(int state);

	uint8_t agat7_membank_r(offs_t offset);
	void agat7_membank_w(offs_t offset, uint8_t data);
	uint8_t agat7_ram_r(offs_t offset);
	void agat7_ram_w(offs_t offset, uint8_t data);
	uint8_t keyb_strobe_r();
	void keyb_strobe_w(uint8_t data);
	uint8_t cassette_toggle_r();
	void cassette_toggle_w(uint8_t data);
	uint8_t speaker_toggle_r();
	void speaker_toggle_w(uint8_t data);
	uint8_t interrupts_on_r();
	void interrupts_on_w(uint8_t data);
	uint8_t interrupts_off_r();
	void interrupts_off_w(uint8_t data);
	uint8_t flags_r(offs_t offset);
	uint8_t controller_strobe_r();
	void controller_strobe_w(uint8_t data);

	void kbd_put(u8 data);
	void kbd_meta(int state);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<a2bus_device> m_a2bus;
	required_ioport m_joy1x, m_joy1y, m_joy2x, m_joy2y, m_joybuttons;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<address_map_bank_device> m_upperbank;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

protected:
	int m_speaker_state;
	int m_cassette_state;

	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;

	uint8_t m_transchar, m_strobe;
	bool m_meta;

	int m_inh_slot;
	int m_cnxx_slot;

	uint8_t *m_ram_ptr;
	int m_ram_size;

	int m_inh_bank;

	double m_x_calibration, m_y_calibration;

	device_a2bus_card_interface *m_slotdevice[8];

	bool m_agat_interrupts;
	int m_agat7_membank;
	int m_agat7_ram_slot;

	uint8_t read_floatingbus();
};

class agat7_state : public agat_base_state
{
public:
	agat7_state(const machine_config &mconfig, device_type type, const char *tag)
		: agat_base_state(mconfig, type, tag)
		, m_video(*this, A7_VIDEO_TAG)
	{ }

	void agat7(machine_config &config);

	optional_device<agat7video_device> m_video;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_irq);
	uint8_t keyb_data_r();

	void agat7_map(address_map &map) ATTR_COLD;
	void inhbank_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
};

class agat9_state : public agat_base_state
{
public:
	agat9_state(const machine_config &mconfig, device_type type, const char *tag)
		: agat_base_state(mconfig, type, tag)
		, m_video(*this, A9_VIDEO_TAG)
	{ }

	void agat9(machine_config &config);

	optional_device<agat9video_device> m_video;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_irq);
	uint8_t keyb_data_r();

	void agat9_map(address_map &map) ATTR_COLD;
	void inhbank_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t c090_r(offs_t offset);
	void c090_w(offs_t offset, uint8_t data);
	uint8_t c200_r(offs_t offset);
	void c200_w(offs_t offset, uint8_t data);
	void apple_w(offs_t offset, uint8_t data);

	uint8_t agat9_membank_r(offs_t offset);
	void agat9_membank_w(offs_t offset, uint8_t data);
	uint8_t agat9_upperbank_r(offs_t offset);
	void agat9_upperbank_w(offs_t offset, uint8_t data);

private:
	int m_agat9_membank[16]{}; // 8 physical banks, but ram chip has 16 locations
	int m_agat9_upperbank = 0;
	int m_agat9_lcbank = 0;
	bool m_agat9_prewrite = false;
	bool m_apple = false;
};

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define JOYSTICK_DELTA          80
#define JOYSTICK_SENSITIVITY    50
#define JOYSTICK_AUTOCENTER     80

void agat_base_state::a2bus_irq_w(int state)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, state);
}

void agat_base_state::a2bus_nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

// This code makes a ton of assumptions because we can guarantee a pre-IIe machine!
void agat_base_state::a2bus_inh_w(int state)
{
	if (state == ASSERT_LINE)
	{
		// assume no cards are pulling /INH
		m_inh_slot = -1;
		m_agat7_ram_slot = -1;

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
					// break;
				}

				if ((m_slotdevice[i]->inh_start() == 0x8000) &&
					(m_slotdevice[i]->inh_end() == 0xbfff))
				{
					if ((m_slotdevice[i]->inh_type() & INH_READ) == INH_READ)
					{
						m_agat7_ram_slot = i;
					}
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

void agat_base_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);
	m_cassette_state = 0;
	m_cassette->output(-1.0f);
	m_inh_bank = 0;
	m_transchar = 0;
	m_strobe = 0;
	m_meta = false;

	// precalculate joystick time constants
	m_x_calibration = attotime::from_usec(12).as_double();
	m_y_calibration = attotime::from_usec(13).as_double();

	// cache slot devices
	for (int i = 0; i <= 7; i++)
	{
		m_slotdevice[i] = m_a2bus->get_a2bus_card(i);
	}

	m_inh_slot = -1;
	m_cnxx_slot = -1;
	m_agat7_ram_slot = -1;

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_cassette_state));
	save_item(NAME(m_joystick_x1_time));
	save_item(NAME(m_joystick_y1_time));
	save_item(NAME(m_joystick_x2_time));
	save_item(NAME(m_joystick_y2_time));
	save_item(NAME(m_strobe));
	save_item(NAME(m_transchar));
	save_item(NAME(m_inh_slot));
	save_item(NAME(m_inh_bank));
	save_item(NAME(m_cnxx_slot));
}

void agat7_state::machine_start()
{
	agat_base_state::machine_start();

	m_upperbank->set_bank(0);
}

void agat9_state::machine_start()
{
	agat_base_state::machine_start();
	logerror("start apple %d\n", m_apple);

	m_upperbank->set_bank(1);
	m_agat9_upperbank = 0;
	m_agat9_lcbank = 0;
	m_agat9_prewrite = false;
	// reset only on power on
	m_apple = false;
}


void agat_base_state::machine_reset()
{
	m_agat_interrupts = false;
}

void agat7_state::machine_reset()
{
	agat_base_state::machine_reset();

	m_agat7_membank = 0;
	m_upperbank->set_bank(0);
}

void agat9_state::machine_reset()
{
	agat_base_state::machine_reset();
	logerror("reset apple %d\n", m_apple);

	if (!m_apple)
	{
		m_agat9_membank[0] = 0x00;
		m_agat9_membank[1] = 0x11;
		m_agat9_membank[2] = 0x22;
		m_agat9_membank[3] = 0x33;
		m_agat9_membank[4] = 0x44;
		m_agat9_membank[5] = 0x55;
		m_agat9_membank[6] = 0x66;
		m_agat9_membank[7] = 0x77;

		membank("bank0")->set_base(m_ram_ptr + 0x0000);
		membank("bank1")->set_base(m_ram_ptr + 0x2000);
		membank("bank2")->set_base(m_ram_ptr + 0x4000);
		membank("bank3")->set_base(m_ram_ptr + 0x6000);
		membank("bank4")->set_base(m_ram_ptr + 0x8000);
		membank("bank5")->set_base(m_ram_ptr + 0xa000);
		membank("bank6")->set_base(m_ram_ptr + 0xc000);
		membank("bank6a")->set_base(m_ram_ptr + 0xc000);
		membank("bank7")->set_base(m_ram_ptr + 0xe000);
		membank("bank7a")->set_base(m_ram_ptr + 0xe000);

		m_upperbank->set_bank(1);
	}
}

/***************************************************************************
    I/O
***************************************************************************/

void agat_base_state::kbd_put(u8 data)
{
	if (!m_strobe)
	{
		m_transchar = data;
		m_strobe = 0x80;
	}
}

void agat_base_state::kbd_meta(int state)
{
	m_meta = state;
}

uint8_t agat7_state::keyb_data_r()
{
	return m_strobe ? (m_transchar | m_strobe) : 0;
}

uint8_t agat9_state::keyb_data_r()
{
	return m_transchar | m_strobe;
}

uint8_t agat_base_state::keyb_strobe_r()
{
	// reads any key down, clears strobe
	uint8_t rv = m_strobe;
	if (!machine().side_effects_disabled())
		m_strobe = 0;
	return rv;
}

void agat_base_state::keyb_strobe_w(uint8_t data)
{
	// clear keyboard latch
	m_strobe = 0;
}

uint8_t agat_base_state::cassette_toggle_r()
{
	if (!machine().side_effects_disabled())
		cassette_toggle_w(0);
	return read_floatingbus();
}

void agat_base_state::cassette_toggle_w(uint8_t data)
{
	m_cassette_state ^= 1;
	m_cassette->output(m_cassette_state ? 1.0f : -1.0f);
}

uint8_t agat_base_state::speaker_toggle_r()
{
	if (!machine().side_effects_disabled())
		speaker_toggle_w(0);
	return read_floatingbus();
}

void agat_base_state::speaker_toggle_w(uint8_t data)
{
	m_speaker_state ^= 1;
	m_speaker->level_w(m_speaker_state);
}

uint8_t agat_base_state::interrupts_on_r()
{
	if (!machine().side_effects_disabled())
		interrupts_on_w(0);
	return read_floatingbus();
}

void agat_base_state::interrupts_on_w(uint8_t data)
{
	m_agat_interrupts = true;
}

uint8_t agat_base_state::interrupts_off_r()
{
	if (!machine().side_effects_disabled())
		interrupts_off_w(0);
	return read_floatingbus();
}

void agat_base_state::interrupts_off_w(uint8_t data)
{
	m_agat_interrupts = false;
}

uint8_t agat_base_state::flags_r(offs_t offset)
{
	switch (offset)
	{
	case 0: // cassette in
		return m_cassette->input() > 0.0 ? 0x80 : 0;

	case 1: // button 0
		return (m_joybuttons->read() & 0x10) ? 0x80 : 0;

	case 2: // button 1
		return (m_joybuttons->read() & 0x20) ? 0x80 : 0;

	case 3: // meta key
		return m_meta ? 0 : 0x80;

	case 4: // joy 1 X axis
		return (machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0;

	case 5: // joy 1 Y axis
		return (machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0;

	case 6: // joy 2 X axis
		return (machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0;

	case 7: // joy 2 Y axis
		return (machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0;
	}

	// this is never reached
	return 0;
}

uint8_t agat_base_state::controller_strobe_r()
{
	if (!machine().side_effects_disabled())
		controller_strobe_w(0);
	return read_floatingbus();
}

void agat_base_state::controller_strobe_w(uint8_t data)
{
	// 555 monostable one-shot timers; a running timer cannot be restarted
	if (machine().time().as_double() >= m_joystick_x1_time)
	{
		m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_joy1x->read();
	}
	if (machine().time().as_double() >= m_joystick_y1_time)
	{
		m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_joy1y->read();
	}
	if (machine().time().as_double() >= m_joystick_x2_time)
	{
		m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_joy2x->read();
	}
	if (machine().time().as_double() >= m_joystick_y2_time)
	{
		m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_joy2y->read();
	}

}

uint8_t agat_base_state::c080_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		int slot;

		offset &= 0x7F;
		slot = offset / 0x10;

		if (m_slotdevice[slot] != nullptr)
		{
			u8 data = m_slotdevice[slot]->read_c0nx(offset % 0x10);
			logerror("%s: c080_r %04X (slot %d) == %02X\n", machine().describe_context(), offset + 0xc080, slot, data);
			return data;
		}

	}

	return read_floatingbus();
}

void agat_base_state::c080_w(offs_t offset, uint8_t data)
{
	int slot;

	offset &= 0x7F;
	slot = offset / 0x10;

	logerror("%s: c080_w %04X (slot %d) <- %02X\n", machine().describe_context(), offset + 0xc080, slot, data);

	if (m_slotdevice[slot] != nullptr)
	{
		m_slotdevice[slot]->write_c0nx(offset % 0x10, data);
	}
}

uint8_t agat_base_state::c100_r(offs_t offset)
{
	int slotnum;

	slotnum = ((offset >> 8) & 0xf) + 1;

	if (m_slotdevice[slotnum] != nullptr)
	{
//      u8 data;

		if ((m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
		}

//      logerror("%s: c100_r %04X (slot %d) == %02X\n", machine().describe_context(), offset+0xc100, slotnum, data);

		return m_slotdevice[slotnum]->read_cnxx(offset & 0xff);
	}

	return read_floatingbus();
}

void agat_base_state::c100_w(offs_t offset, uint8_t data)
{
	int slotnum;

	slotnum = ((offset >> 8) & 0xf) + 1;

	logerror("%s: c100_w %04X (slot %d) <- %02X\n", machine().describe_context(), offset + 0xc100, slotnum, data);

	if (m_slotdevice[slotnum] != nullptr)
	{
		if ((m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
		}

		m_slotdevice[slotnum]->write_cnxx(offset & 0xff, data);
	}
}

uint8_t agat9_state::c090_r(offs_t offset)
{
	return c080_r(offset + 0x10);
}

void agat9_state::c090_w(offs_t offset, uint8_t data)
{
	c080_w(offset + 0x10, data);
}

uint8_t agat9_state::c200_r(offs_t offset)
{
	return c100_r(offset + 0x0100);
}

void agat9_state::c200_w(offs_t offset, uint8_t data)
{
	c100_w(offset + 0x0100, data);
}

uint8_t agat_base_state::c800_r(offs_t offset)
{
	if (offset == 0x7ff)
	{
		if (!machine().side_effects_disabled())
		{
			m_cnxx_slot = -1;
		}

		return 0xff;
	}

	if ((m_cnxx_slot != -1) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		return m_slotdevice[m_cnxx_slot]->read_c800(offset & 0xfff);
	}

	return read_floatingbus();
}

void agat_base_state::c800_w(offs_t offset, uint8_t data)
{
	if (offset == 0x7ff)
	{
		if (!machine().side_effects_disabled())
		{
			m_cnxx_slot = -1;
		}

		return;
	}

	if ((m_cnxx_slot != -1) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		m_slotdevice[m_cnxx_slot]->write_c800(offset & 0xfff, data);
	}
}

void agat9_state::apple_w(offs_t offset, uint8_t data)
{
	logerror("%s: c0f0_w %04X <- %02X (apple mode set)\n", machine().describe_context(), offset + 0xc0f0, data);

	m_apple = true;
	m_upperbank->set_bank(5); // FIXME: get current bank
}

uint8_t agat_base_state::inh_r(offs_t offset)
{
	if (m_inh_slot != -1)
	{
		return m_slotdevice[m_inh_slot]->read_inh_rom(offset + 0xd000);
	}

	assert(0); // hitting inh_r with invalid m_inh_slot should not be possible
	return read_floatingbus();
}

void agat_base_state::inh_w(offs_t offset, uint8_t data)
{
	if (m_inh_slot != -1)
	{
		m_slotdevice[m_inh_slot]->write_inh_rom(offset + 0xd000, data);
	}
}

uint8_t agat_base_state::read_floatingbus()
{
	return 0xff;
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

/* onboard memory banking on Agat-7 */

uint8_t agat_base_state::agat7_membank_r(offs_t offset)
{
	logerror("%s: c0f0_r %04X == %02X\n", machine().describe_context(), offset + 0xc0f0, 0xff);

	if (!machine().side_effects_disabled())
	{
		m_agat7_membank = offset;
	}

	return 0xff;
}

void agat_base_state::agat7_membank_w(offs_t offset, uint8_t data)
{
	logerror("%s: c0f0_w %04X <- %02X\n", machine().describe_context(), offset + 0xc0f0, data);

	m_agat7_membank = offset;
}

uint8_t agat_base_state::agat7_ram_r(offs_t offset)
{
	if (offset < 32768)
	{
		if (offset < m_ram_size)
		{
			return m_ram_ptr[offset];
		}
	}
	else
	{
		if (m_agat7_ram_slot != -1)
			return m_slotdevice[m_agat7_ram_slot]->read_inh_rom(offset + 0x8000);
		if (offset < m_ram_size)
		{
			if (m_agat7_membank == 0)
				return m_ram_ptr[offset];
			else if (m_agat7_membank == 1)
				return m_ram_ptr[offset + 16384];
		}
	}

	return 0xff;
}

void agat_base_state::agat7_ram_w(offs_t offset, uint8_t data)
{
	if (offset < 32768)
	{
		if (offset < m_ram_size)
		{
			m_ram_ptr[offset] = data;
		}
	}
	else
	{
		if (m_agat7_ram_slot != -1)
			m_slotdevice[m_agat7_ram_slot]->write_inh_rom(offset + 0x8000, data);
		else if (offset < m_ram_size)
		{
			if (m_agat7_membank == 0)
				m_ram_ptr[offset] = data;
			else if (m_agat7_membank == 1)
				m_ram_ptr[offset + 16384] = data;
		}
	}
}

/*
 * onboard memory banking on Agat-9
 *
 * native mode: writes do the bankswitch, reads return current mapping mode
 * apple2 mode: writes ignored ??, reads do language card emulation
 * and return current mapping mode
 */

uint8_t agat9_state::agat9_upperbank_r(offs_t offset)
{
	if (machine().side_effects_disabled())
	{
		return read_floatingbus();
	}

	if (!m_apple)
	{
		logerror("%s: c080_r %04X == %02X\n", machine().describe_context(), 0xc080 + offset, m_agat9_upperbank);

		return m_agat9_upperbank;
	}

	int rc;

#if 0
	if (BIT(offset, 2)) // select physical memory banks for LC emulation
	{
		int newbank = bitswap<8>(offset,7,6,5,4,3,1,0,2) & 14;
		rc = newbank == m_agat9_lcbank ? (0xc0 + m_agat9_upperbank) : 0xc0;
		m_agat9_lcbank = newbank;

		membank("bank6")->set_base(m_ram_ptr + (newbank * 8192) + (BIT(m_agat9_upperbank, 3) * 4096));
		membank("bank7")->set_base(m_ram_ptr + (newbank + 1) * 8192);

		logerror("%s: c080_r %04X == %02X: select new map %d (mode %d d000 %s)\n", machine().describe_context(), offset + 0xc080,
			newbank, 0xc0 + m_agat9_upperbank, m_agat9_upperbank & 8 ? "upper" : "lower");
	}
	else // select mapping modes
#endif
	{
		m_agat9_upperbank = BIT(offset, 0) ? offset : (offset ^ 2);
		rc = 0xc0 + m_agat9_upperbank;

		// taken from ramcard16k.cpp
		if (BIT(offset, 0))
		{
			if (m_agat9_prewrite == false)
			{
				m_agat9_prewrite = true;
				m_upperbank->set_bank(4 + ((offset + 1) & 3)); // writes disabled
			}
			else
			{
				m_upperbank->set_bank(4 + (offset & 3));
			}
		}
		else
		{
			m_agat9_prewrite = false;
			m_upperbank->set_bank(4 + (offset & 3));
		}

		membank("bank6a")->set_base(m_ram_ptr + (m_agat9_membank[14] & 15) * 8192 + BIT(m_agat9_upperbank, 3) * 4096);

		logerror("%s: c080_r %04X == %02X: select new mode %d (map %d d000 %s)\n", machine().describe_context(), offset + 0xc080,
			rc, offset & 3, m_agat9_lcbank, m_agat9_upperbank & 8 ? "upper" : "lower");
	}

	return rc;
}

void agat9_state::agat9_upperbank_w(offs_t offset, uint8_t data)
{
	logerror("%s: c080_w %04X <- %02X: select new mode %d (d000 %s)\n", machine().describe_context(), offset + 0xc080, data,
		offset & 3, offset & 8 ? "upper" : "lower");

	if (m_apple)
	{
		m_agat9_prewrite = false;
	}
	else
	{
		m_upperbank->set_bank(offset & 3);
		m_agat9_upperbank = (offset & 3) ? (offset & 0xb) : (2 | (offset & 0xb));
		m_agat9_upperbank |= 0x80; // ikp2-19176.aim passes

		membank("bank6")->set_base(m_ram_ptr + (m_agat9_membank[6] & 15) * 8192 + BIT(m_agat9_upperbank, 3) * 4096);
	}
}

uint8_t agat9_state::agat9_membank_r(offs_t offset)
{
	u8 data = 0;

	data = m_agat9_membank[(offset >> 4)];

	logerror("%s: c100_r %04X == %02X\n", machine().describe_context(), offset + 0xc100, data);

	return data;
}

void agat9_state::agat9_membank_w(offs_t offset, uint8_t data)
{
	logerror("%s: c100_w %04X <- %02X: bank %d va %04X = page %02d pa %05X (apple %d)\n", machine().describe_context(), offset + 0xc100, data,
		(offset >> 4), 0x2000 * (offset >> 4), (offset & 15), (offset & 15) * 8192, m_apple);

	m_agat9_membank[(offset >> 4)] = offset;

	if (m_apple)
		return;

	int newbank = (offset & 15) * 8192;

	switch (offset >> 4)
	{
	case 0:
		membank("bank0")->set_base(m_ram_ptr + newbank);
		break;

	case 1:
		membank("bank1")->set_base(m_ram_ptr + newbank);
		break;

	case 2:
		membank("bank2")->set_base(m_ram_ptr + newbank);
		break;

	case 3:
		membank("bank3")->set_base(m_ram_ptr + newbank);
		break;

	case 4:
		membank("bank4")->set_base(m_ram_ptr + newbank);
		break;

	case 5:
		membank("bank5")->set_base(m_ram_ptr + newbank);
		break;

	case 6:
		membank("bank6")->set_base(m_ram_ptr + newbank + (BIT(m_agat9_upperbank, 3) * 4096));
		break;

	case 7:
		membank("bank7")->set_base(m_ram_ptr + newbank);
		break;

	case 14:
		membank("bank6a")->set_base(m_ram_ptr + newbank + (BIT(m_agat9_upperbank, 3) * 4096));
		break;

	case 15:
		membank("bank7a")->set_base(m_ram_ptr + newbank);
		break;

	default:
		logerror("membank %d UNIMP\n", (offset >> 4));
		break;
	}
}


/*
 * agat7: onboard RAM is at least 32K, up to 128K.
 * first 32K of onboard RAM are always mapped at 0.
 * standard add-on RAM cards hold 32K (possibly up to 128K?)
 * and are supported only on motherboards with 32K onboard.
 * all extra RAM (onboard or addon) is accessible via 16K window at 0x8000.
 */
void agat7_state::agat7_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).rw(FUNC(agat7_state::agat7_ram_r), FUNC(agat7_state::agat7_ram_w));
	map(0xc000, 0xc000).mirror(0xf).r(FUNC(agat7_state::keyb_data_r)).nopw();
	map(0xc010, 0xc010).mirror(0xf).rw(FUNC(agat7_state::keyb_strobe_r), FUNC(agat7_state::keyb_strobe_w));
	map(0xc020, 0xc020).mirror(0xf).rw(FUNC(agat7_state::cassette_toggle_r), FUNC(agat7_state::cassette_toggle_w));
	map(0xc030, 0xc030).mirror(0xf).rw(FUNC(agat7_state::speaker_toggle_r), FUNC(agat7_state::speaker_toggle_w));
	map(0xc040, 0xc040).mirror(0xf).rw(FUNC(agat7_state::interrupts_on_r), FUNC(agat7_state::interrupts_on_w));
	map(0xc050, 0xc050).mirror(0xf).rw(FUNC(agat7_state::interrupts_off_r), FUNC(agat7_state::interrupts_off_w));
	map(0xc060, 0xc067).mirror(0x8).r(FUNC(agat7_state::flags_r)).nopw();
	map(0xc070, 0xc070).mirror(0xf).rw(FUNC(agat7_state::controller_strobe_r), FUNC(agat7_state::controller_strobe_w));
	map(0xc080, 0xc0ef).rw(FUNC(agat7_state::c080_r), FUNC(agat7_state::c080_w));
	map(0xc0f0, 0xc0ff).rw(FUNC(agat7_state::agat7_membank_r), FUNC(agat7_state::agat7_membank_w));
	map(0xc100, 0xc6ff).rw(FUNC(agat7_state::c100_r), FUNC(agat7_state::c100_w));
	map(0xc700, 0xc7ff).rw(A7_VIDEO_TAG, FUNC(agat7video_device::read), FUNC(agat7video_device::write));
	map(0xc800, 0xcfff).rw(FUNC(agat7_state::c800_r), FUNC(agat7_state::c800_w));
	map(0xd000, 0xffff).m(m_upperbank, FUNC(address_map_bank_device::amap8));
}

void agat7_state::inhbank_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0x1000).w(FUNC(agat7_state::inh_w));
	map(0x3000, 0x5fff).rw(FUNC(agat7_state::inh_r), FUNC(agat7_state::inh_w));
}

/*
 * agat9: onboard RAM is always 128K.  language card emulation is built in.
 * standard add-on RAM cards hold 128K, up to 4 cards are supported.
 * both onboard and add-on RAM is split into 16K banks.
 */
void agat9_state::agat9_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).bankrw("bank0");
	map(0x2000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x5fff).bankrw("bank2");
	map(0x6000, 0x7fff).bankrw("bank3");
	map(0x8000, 0x9fff).bankrw("bank4");
	map(0xa000, 0xbfff).bankrw("bank5");
	map(0xc000, 0xc000).mirror(0xf).r(FUNC(agat9_state::keyb_data_r)).nopw();
	map(0xc010, 0xc010).mirror(0xf).rw(FUNC(agat9_state::keyb_strobe_r), FUNC(agat9_state::keyb_strobe_w));
	map(0xc020, 0xc020).mirror(0xf).rw(FUNC(agat9_state::interrupts_off_r), FUNC(agat9_state::interrupts_off_w));
	map(0xc030, 0xc030).mirror(0xf).rw(FUNC(agat9_state::speaker_toggle_r), FUNC(agat9_state::speaker_toggle_w));
	map(0xc040, 0xc040).mirror(0xf).rw(FUNC(agat9_state::interrupts_on_r), FUNC(agat9_state::interrupts_on_w));
	map(0xc050, 0xc05f).rw(A9_VIDEO_TAG, FUNC(agat9video_device::apple_read), FUNC(agat9video_device::apple_write));
	map(0xc060, 0xc067).mirror(0x8).r(FUNC(agat9_state::flags_r)).nopw();
	map(0xc070, 0xc070).mirror(0xf).rw(FUNC(agat9_state::controller_strobe_r), FUNC(agat9_state::controller_strobe_w));
	map(0xc080, 0xc08f).rw(FUNC(agat9_state::agat9_upperbank_r), FUNC(agat9_state::agat9_upperbank_w));
	map(0xc090, 0xc0ef).rw(FUNC(agat9_state::c090_r), FUNC(agat9_state::c090_w));
	map(0xc0f0, 0xc0ff).w(FUNC(agat9_state::apple_w));
	map(0xc100, 0xc1ff).rw(FUNC(agat9_state::agat9_membank_r), FUNC(agat9_state::agat9_membank_w));
	map(0xc200, 0xc6ff).rw(FUNC(agat9_state::c200_r), FUNC(agat9_state::c200_w));
	map(0xc700, 0xc7ff).rw(A9_VIDEO_TAG, FUNC(agat9video_device::read), FUNC(agat9video_device::write));
	map(0xc800, 0xcfff).rw(FUNC(agat9_state::c800_r), FUNC(agat9_state::c800_w));
	map(0xd000, 0xffff).m(m_upperbank, FUNC(address_map_bank_device::amap8));
}

void agat9_state::inhbank_map(address_map &map)
{
	// native mode

	// map 0 -- C080/C088
	map(0x00000, 0x00fff).bankr("bank6").nopw();
	map(0x01000, 0x02fff).bankr("bank7").nopw();

	// map 1 -- C081/C089
	map(0x10000, 0x10fff).rom().region("maincpu", 0x1000).bankw("bank6");
	map(0x11000, 0x12fff).rom().region("maincpu", 0x2000).bankw("bank7");

	// map 2 -- C082/C08A -- only in apple mode (same as map 0 in agat mode)
	map(0x20000, 0x20fff).bankr("bank6").nopw();
	map(0x21000, 0x22fff).bankr("bank7").nopw();

	// map 3 -- C083/C08B
	map(0x30000, 0x30fff).bankrw("bank6");
	map(0x31000, 0x32fff).bankrw("bank7");

	// apple mode -- ROM is simulated by onboard RAM (logical banks 14 and 15)
	// language card emulation can use onboard or add-on card memory

	// map 0 -- C080/C088 -- rc C2/CA -- bank_r mapped to LC area
	map(0x40000, 0x40fff).bankr("bank6a").nopw();
	map(0x41000, 0x42fff).bankr("bank7a").nopw();

	// map 1 -- C081/C089 -- rc C1/C9 -- bank_r mapped to apple ROM area, bank_w to LC area
	map(0x50000, 0x50fff).bankr("bank6").bankw("bank6a");
	map(0x51000, 0x52fff).bankr("bank7").bankw("bank7a");

	// map 2 -- C082/C08A -- rc C0/C8 -- bank_r mapped to apple ROM area
	map(0x60000, 0x60fff).bankr("bank6").nopw();
	map(0x61000, 0x62fff).bankr("bank7").nopw();

	// map 3 -- C083/C08B -- rc C3/CB -- banks mapped to LC area
	map(0x70000, 0x70fff).bankrw("bank6a");
	map(0x71000, 0x72fff).bankrw("bank7a");
}

/***************************************************************************
    KEYBOARD
***************************************************************************/

INTERRUPT_GEN_MEMBER(agat_base_state::agat_vblank)
{
	if (m_agat_interrupts)
	{
		m_maincpu->pulse_input_line(M6502_NMI_LINE, attotime::zero);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(agat7_state::timer_irq)
{
	if (m_agat_interrupts)
	{
		switch (param & 0x3f)
		{
		case 0:
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;

		case 0x20:
			m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
			break;
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(agat9_state::timer_irq)
{
	if (m_agat_interrupts)
	{
		switch (param & 0xf)
		{
		case 0:
			m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
			break;

		case 8:
			m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
			break;
		}
	}
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( agat7_joystick )
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(1)            PORT_CODE(KEYCODE_0_PAD)    PORT_CODE(JOYCODE_BUTTON1) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_PLAYER(1)            PORT_CODE(KEYCODE_ENTER_PAD)PORT_CODE(JOYCODE_BUTTON2) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(2)            PORT_CODE(JOYCODE_BUTTON1)
INPUT_PORTS_END

static INPUT_PORTS_START(agat7)
	/* other devices */
	PORT_INCLUDE(agat7_joystick)
INPUT_PORTS_END

static void agat7_cards(device_slot_interface &device)
{
	device.option_add("a7lang", A2BUS_AGAT7LANGCARD); // Agat-7 RAM Language Card -- decimal 3.089.119
	device.option_add("a7ram", A2BUS_AGAT7RAM); // Agat-7 32K RAM Card -- decimal 3.089.119-01, KR565RU6D chips
	device.option_add("a7fdc", A2BUS_AGAT7_FDC); // Disk II clone -- decimal 3.089.105
	device.option_add("a9fdchle", A2BUS_AGAT840K_HLE); // 840K floppy controller -- decimal 7.104.351 or 3.089.023?
	device.option_add("a9fdc", A2BUS_AGAT_FDC); // 840K floppy controller LLE
	device.option_add("a7ports", A2BUS_AGAT7_PORTS); // Serial-parallel card -- decimal 3.089.106
}

static void agat9_cards(device_slot_interface &device)
{
//  device.option_add("a9ram", A2BUS_AGAT9RAM); // Agat-9 128K RAM Card -- decimal 3.089.170
	device.option_add("diskiing", A2BUS_DISKIING);  /* Disk II Controller Card */
	device.option_add("a9fdc140", A2BUS_AGAT9_FDC); // Disk II clone -- decimal 3.089.173 (reworked for agat9)
	device.option_add("a9fdchle", A2BUS_AGAT840K_HLE); // 840K floppy controller -- decimal 7.104.351 or 3.089.023?
	device.option_add("a9fdc", A2BUS_AGAT_FDC); // 840K floppy controller LLE
	device.option_add("a7ports", A2BUS_AGAT7_PORTS); // Serial-parallel card -- decimal 3.089.106
	device.option_add("nclock", A2BUS_NIPPELCLOCK); // Nippel Clock (mc146818)
}

void agat7_state::agat7(machine_config &config)
{
	M6502(config, m_maincpu, XTAL(14'300'000) / 14);
	m_maincpu->set_addrmap(AS_PROGRAM, &agat7_state::agat7_map);
	m_maincpu->set_vblank_int(A7_VIDEO_TAG ":a7screen", FUNC(agat_base_state::agat_vblank));

	TIMER(config, "scantimer").configure_scanline(FUNC(agat7_state::timer_irq), A7_VIDEO_TAG ":a7screen", 0, 1);

	AGAT7VIDEO(config, m_video, RAM_TAG, "gfx1");

	RAM(config, m_ram).set_default_size("32K").set_default_value(0);//.set_extra_options("64K,128K");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* /INH banking */
	ADDRESS_MAP_BANK(config, m_upperbank).set_map(&agat7_state::inhbank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	agat_keyboard_device &keyboard(AGAT_KEYBOARD(config, "keyboard", 0));
	keyboard.out_callback().set(FUNC(agat_base_state::kbd_put));
	keyboard.out_meta_callback().set(FUNC(agat_base_state::kbd_meta));
	keyboard.out_reset_callback().set([this](bool state) { m_maincpu->reset(); });

	/*
	 * slot 0 is reserved for SECAM encoder or Apple II compat card.
	 * slot 1 always holds the CPU card.
	 * most of the software expects a7lang in slot 2 and a7ram in slot 6.
	 */
	A2BUS(config, m_a2bus, 0);
	m_a2bus->set_space(m_maincpu, AS_PROGRAM);
	m_a2bus->irq_w().set(FUNC(agat_base_state::a2bus_irq_w));
	m_a2bus->nmi_w().set(FUNC(agat_base_state::a2bus_nmi_w));
	m_a2bus->inh_w().set(FUNC(agat_base_state::a2bus_inh_w));
	m_a2bus->dma_w().set_inputline(m_maincpu, INPUT_LINE_HALT);
	A2BUS_SLOT(config, "sl2", m_a2bus, agat7_cards, "a7lang");
	A2BUS_SLOT(config, "sl3", m_a2bus, agat7_cards, "a7fdc");
	A2BUS_SLOT(config, "sl4", m_a2bus, agat7_cards, "a7ports");
	A2BUS_SLOT(config, "sl5", m_a2bus, agat7_cards, nullptr);
	A2BUS_SLOT(config, "sl6", m_a2bus, agat7_cards, "a7ram");

	CASSETTE(config,m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void agat9_state::agat9(machine_config &config)
{
	R65C02(config, m_maincpu, XTAL(14'300'000) / 14);
	m_maincpu->set_addrmap(AS_PROGRAM, &agat9_state::agat9_map);
	m_maincpu->set_vblank_int(A9_VIDEO_TAG ":a9screen", FUNC(agat_base_state::agat_vblank));

	TIMER(config, "scantimer").configure_scanline(FUNC(agat9_state::timer_irq), A9_VIDEO_TAG ":a9screen", 0, 1);

	AGAT9VIDEO(config, m_video, RAM_TAG, "gfx1");

	RAM(config, m_ram).set_default_size("128K").set_default_value(0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* /INH banking */
	ADDRESS_MAP_BANK(config, m_upperbank).set_map(&agat9_state::inhbank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x10000);

	agat_keyboard_device &keyboard(AGAT_KEYBOARD(config, "keyboard", 0));
	keyboard.out_callback().set(FUNC(agat_base_state::kbd_put));
	keyboard.out_meta_callback().set(FUNC(agat_base_state::kbd_meta));
	keyboard.out_reset_callback().set([this](bool state) { m_maincpu->reset(); });

	A2BUS(config, m_a2bus, 0);
	m_a2bus->set_space(m_maincpu, AS_PROGRAM);
	m_a2bus->irq_w().set(FUNC(agat_base_state::a2bus_irq_w));
	m_a2bus->nmi_w().set(FUNC(agat_base_state::a2bus_nmi_w));
	m_a2bus->inh_w().set(FUNC(agat_base_state::a2bus_inh_w));
	m_a2bus->dma_w().set_inputline(m_maincpu, INPUT_LINE_HALT);
	// slot 0 does not exist
	A2BUS_SLOT(config, "sl1", m_a2bus, agat9_cards, nullptr);
	A2BUS_SLOT(config, "sl2", m_a2bus, agat9_cards, nullptr); // a9ram
	A2BUS_SLOT(config, "sl3", m_a2bus, agat9_cards, nullptr); // printer->mouse
	A2BUS_SLOT(config, "sl4", m_a2bus, agat9_cards, nullptr); // printer
	A2BUS_SLOT(config, "sl5", m_a2bus, agat9_cards, "a9fdc");
	A2BUS_SLOT(config, "sl6", m_a2bus, agat9_cards, "a9fdc140");

	CASSETTE(config,m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( agat7 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("v1")

	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" ) // original?
	ROMX_LOAD("monitor7.rom", 0x3800, 0x0800, CRC(071fda0b) SHA1(6089d46b7addc4e2ae096b2cf81124681bd2b27a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" ) // modded by author of agatcomp.ru
	ROMX_LOAD("agat_pzu.bin", 0x3800, 0x0800, CRC(c605163d) SHA1(b30fd1b264a347a9de69bb9e3105483254994d06), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "debug", "Debug" )  // written by author of agatcomp.ru
	ROMX_LOAD("debug-sysmon7.bin", 0x3800, 0x0800, CRC(d26f18a4) SHA1(2862c13a82e2f4dfc757aa2eeab11fe71c570c12), ROM_BIOS(2))

	// 140KB floppy controller
	ROM_LOAD( "shugart7.rom", 0x4500, 0x0100, CRC(c6e4850c) SHA1(71626d3d2d4bbeeac2b77585b45a5566d20b8d34))
	// 840KB floppy controller
	ROM_LOAD( "teac.rom",     0x4500, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e))

	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD( "agathe7.fnt", 0x0000, 0x0800, CRC(fcffb490) SHA1(0bda26ae7ad75f74da835c0cf6d9928f9508844c))
ROM_END

ROM_START( agat9 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD("monitor9.rom", 0x3000, 0x0800, CRC(b90bb66a) SHA1(02217f0785913b41fc25eabcff70fa814799c69a), ROM_BIOS(0))
	ROMX_LOAD("monitor9.rom", 0x3800, 0x0800, CRC(b90bb66a) SHA1(02217f0785913b41fc25eabcff70fa814799c69a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD("monitor91.rom", 0x3000, 0x0800, CRC(89b10fc1) SHA1(7fe1ede32b5525255f82597ca9c3c2034c5996fa), ROM_BIOS(1))
	ROMX_LOAD("monitor91.rom", 0x3800, 0x0800, CRC(89b10fc1) SHA1(7fe1ede32b5525255f82597ca9c3c2034c5996fa), ROM_BIOS(1))

	// Floppy controllers
	ROM_LOAD( "shugart9.rom", 0x4500, 0x0100, CRC(964a0ce2) SHA1(bf955189ebffe874c20ef649a3db8177dc16af61))
	ROM_LOAD( "teac.rom",     0x4500, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e))

	// Printer card
	ROM_LOAD( "cm6337.rom", 0x8000, 0x0100, CRC(73be16ec) SHA1(ead1abbef5b86f1def0b956147d5b267f0d544b5))
	ROM_LOAD( "cm6337p.rom", 0x8100, 0x0800, CRC(9120f11f) SHA1(78107653491e88d5ea12e07367c4c028771a4aca))

	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD( "agathe9.fnt", 0x0000, 0x0800, CRC(8c55c984) SHA1(5a5a202000576b88b4ae2e180dd2d1b9b337b594))
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1983, agat7, apple2, 0,      agat7,   agat7, agat7_state, empty_init, "Agat",  "Agat-7", MACHINE_IMPERFECT_GRAPHICS)
COMP( 1988, agat9, apple2, 0,      agat9,   agat7, agat9_state, empty_init, "Agat",  "Agat-9", MACHINE_NOT_WORKING)
