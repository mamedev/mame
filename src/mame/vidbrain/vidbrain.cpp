// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    VideoBrain FamilyComputer

    http://www.atariprotos.com/othersystems/videobrain/videobrain.htm
    http://www.seanriddle.com/vbinfo.html
    http://www.seanriddle.com/videobrain.html
    http://www.google.com/patents/US4232374
    http://www.google.com/patents/US4177462
    http://www.orphanedgames.com/videobrain/
    http://www.datalytixllc.com/videobrain/
    http://blog.kevtris.org/blogfiles/videobrain/videobrain_unwrapped.txt

****************************************************************************/

/*

Keyboard:
    - Shift is a toggle. This prevents Natural Keyboard & Paste from being
      able to shift as needed. The shift state shows as a block on the intro
      screen.

Accessories:
    - Expander 1 (-acc expander1) provides the PLAYBACK and RECORD cassette
      decks and the PRINTER and MODEM RS-232 jacks.
    - Expander 2 is a 300 baud acoustic coupler on the MODEM jack. Timeshare
      can talk to a host directly with a null modem on the MODEM jack set to
      300 baud.

Using the system:
    - At startup, the intro screen gives the choices TEXT, COLOR, CLOCK, ALARM.
    - Press F1 for TEXT - this allows you to test the keyboard, but note that
      the Space key simply advances the cursor - it doesn't erase.
    - Press F2 for COLOR - this shows colour bars, presumably as a hardware test.
    - Press F3 for CLOCK
    - Press F4 for ALARM

*/

#include "emu.h"

#include "uv201.h"

#include "bus/vidbrain/accessory.h"
#include "bus/vidbrain/exp.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/rescap.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"

#include "softlist_dev.h"
#include "speaker.h"

#include <cmath>

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class vidbrain_state : public driver_device
{
public:
	vidbrain_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_smi(*this, "smi"),
		m_uv(*this, "uv"),
		m_dac(*this, "dac%u", 0U),
		m_exp(*this, "exp"),
		m_acc(*this, "acc"),
		m_res2(*this, "res2"),
		m_ram(*this, "ram", 0x400, ENDIANNESS_LITTLE),
		m_io(*this, "IO%02u", 0),
		m_uv201_31(*this, "UV201-31"),
		m_joy_r(*this, "JOY-R"),
		m_reset_key(*this, "RESET"),
		m_joy(*this, { "JOY1-X", "JOY1-Y", "JOY2-X", "JOY2-Y", "JOY3-X", "JOY3-Y", "JOY4-X", "JOY4-Y" })
	{ }

	void vidbrain(machine_config &config);
	void vidbrain_video(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);


protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void vidbrain_mem(address_map &map) ATTR_COLD;
	void vidbrain_io(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(joystick_tick);
	TIMER_CALLBACK_MEMBER(field_tick);

	void keyboard_w(uint8_t data);
	uint8_t keyboard_r();
	void sound_w(uint8_t data);

	void hblank_w(int state);
	void uv_int_w(int state);
	void acc_int_w(int state);
	void eject_w(int state);
	void update_ext_int();
	void update_555();
	void update_555_timer();
	uint8_t memory_read_byte(offs_t offset);

	void bus_wait(bool registers);
	uint8_t bus_read(uint8_t data);
	uint8_t uv_r(offs_t offset);
	void uv_w(offs_t offset, uint8_t data);
	uint8_t ext_r();
	void ext_w(uint8_t data);
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t cs1_r(offs_t offset);
	void cs1_w(offs_t offset, uint8_t data);
	uint8_t cs2_r(offs_t offset);
	void cs2_w(offs_t offset, uint8_t data);
	uint8_t res2_r(offs_t offset);
	uint8_t unmap_r(offs_t offset);
	void unmap_w(offs_t offset, uint8_t data);

	required_device<f8_cpu_device> m_maincpu;
	required_device<f3853_device> m_smi;
	required_device<uv201_device> m_uv;
	required_device_array<dac_bit_interface, 2> m_dac;
	required_device<videobrain_expansion_slot_device> m_exp;
	required_device<videobrain_accessory_port_device> m_acc;
	required_region_ptr<uint8_t> m_res2;
	memory_share_creator<uint8_t> m_ram;
	required_ioport_array<8> m_io;
	required_ioport m_uv201_31;
	required_ioport m_joy_r;
	required_ioport m_reset_key;
	required_ioport_array<8> m_joy;

	// keyboard state
	uint8_t m_keylatch = 0;
	bool m_joy_enable = true;
	bool m_hblank = false;
	bool m_555_out = false;
	bool m_555_timeout = false;
	double m_555_charge = 0;
	double m_555_conductance = 0;
	attotime m_555_time;
	bool m_uv_int = false;
	bool m_acc_int = false;
	uint8_t m_port1 = 0;

	// sound state
	int m_sound_clk = 0;

	// buffered bus state
	uint8_t m_bd = 0;

	// timers
	emu_timer *m_timer_ne555 = nullptr;
	emu_timer *m_timer_field[2];

	// reset state
	bool m_reset_pressed = false;
	bool m_reset = false;
	bool m_eject = false;
};



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  keyboard_w - keyboard column write
//-------------------------------------------------

void vidbrain_state::keyboard_w(uint8_t data)
{
	/*

	    bit     description

	    0       keyboard column 0, sound data 0
	    1       keyboard column 1, sound data 1
	    2       keyboard column 2
	    3       keyboard column 3
	    4       keyboard column 4
	    5       keyboard column 5
	    6       keyboard column 6
	    7       keyboard column 7

	*/

	LOG("Keyboard %02x\n", data);

	m_keylatch = data;

	update_555_timer();
}


//-------------------------------------------------
//  keyboard_r - keyboard row read
//-------------------------------------------------

uint8_t vidbrain_state::keyboard_r()
{
	/*

	    bit     description

	    0       keyboard row 0, joystick 1 fire
	    1       keyboard row 1, joystick 2 fire
	    2       keyboard row 2, joystick 3 fire
	    3       keyboard row 3, joystick 4 fire
	    4
	    5
	    6
	    7

	*/

	uint8_t data = m_joy_r->read();

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_keylatch, i)) data |= m_io[i]->read();
	}

	if (!m_uv->kbd_r()) data |= m_uv201_31->read();

	// port pins are wired-AND with the output latch
	data |= m_port1 & 0xf0;
	uint8_t const acc = m_acc->io_r();
	data |= (BIT(acc, 0) << 5) | (BIT(acc, 1) << 6);

	return data;
}


//-------------------------------------------------
//  sound_w - sound clock write
//-------------------------------------------------

void vidbrain_state::sound_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       sound clock
	    5       accessory jack pin 6
	    6       accessory jack pin 1
	    7       joystick enable

	*/

	LOG("Sound %02x\n", data);

	m_port1 = data;

	m_acc->io_w((BIT(data, 6) << 1) | BIT(data, 5));

	// sound clock
	int sound_clk = BIT(data, 4);

	if (!m_sound_clk && sound_clk)
	{
		m_dac[0]->write(BIT(m_keylatch, 0));
		m_dac[1]->write(BIT(m_keylatch, 1));
	}

	m_sound_clk = sound_clk;

	// joystick enable
	m_joy_enable = !BIT(data, 7);

	update_555();
	update_ext_int();
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vidbrain_mem )
//-------------------------------------------------

void vidbrain_state::vidbrain_mem(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).rom().region("res1", 0);
	map(0x0800, 0x08ff).mirror(0x2000).rw(FUNC(vidbrain_state::uv_r), FUNC(vidbrain_state::uv_w));
	map(0x0900, 0x0bff).mirror(0x2000).rw(FUNC(vidbrain_state::ext_r), FUNC(vidbrain_state::ext_w));
	map(0x0c00, 0x0fff).mirror(0x2000).rw(FUNC(vidbrain_state::ram_r), FUNC(vidbrain_state::ram_w));
	map(0x1000, 0x17ff).rw(FUNC(vidbrain_state::cs1_r), FUNC(vidbrain_state::cs1_w));
	map(0x1800, 0x1fff).rw(FUNC(vidbrain_state::cs2_r), FUNC(vidbrain_state::cs2_w));
	map(0x2000, 0x27ff).r(FUNC(vidbrain_state::res2_r));
	map(0x3000, 0x3fff).rw(FUNC(vidbrain_state::unmap_r), FUNC(vidbrain_state::unmap_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( vidbrain_io )
//-------------------------------------------------

void vidbrain_state::vidbrain_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(vidbrain_state::keyboard_w));
	map(0x01, 0x01).rw(FUNC(vidbrain_state::keyboard_r), FUNC(vidbrain_state::sound_w));
	map(0x0c, 0x0f).rw(m_smi, FUNC(f3853_device::read), FUNC(f3853_device::write));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  reset_changed - master control is also the
//  accessory jack reset line
//-------------------------------------------------

INPUT_CHANGED_MEMBER(vidbrain_state::reset_changed)
{
	m_acc->reset_w(newval || m_eject);
}


//-------------------------------------------------
//  eject_w - the cartridge eject button shares
//  the master control reset node
//-------------------------------------------------

void vidbrain_state::eject_w(int state)
{
	if (state)
	{
		m_eject = true;
		m_acc->reset_w(1);
	}
}


//-------------------------------------------------
//  field_tick - master control reset is
//  synchronized to the end of vertical blank
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(vidbrain_state::field_tick)
{
	bool const key = m_reset_key->read();
	bool const pressed = key || m_eject;
	bool const reset = pressed && !m_reset_pressed;

	if (m_eject)
	{
		m_eject = false;
		m_acc->reset_w(key);
	}

	m_reset_pressed = pressed;

	if (reset != m_reset)
	{
		m_reset = reset;

		m_maincpu->set_input_line(INPUT_LINE_RESET, reset ? ASSERT_LINE : CLEAR_LINE);
		m_uv->reset_w(reset);
	}
}


//-------------------------------------------------
//  INPUT_PORTS( vidbrain )
//-------------------------------------------------

static INPUT_PORTS_START( vidbrain )
	PORT_START("IO00")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"P ¢") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR(0x00a2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"; π") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(0x03c0)

	PORT_START("IO01")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('\'') PORT_CHAR('*')

	PORT_START("IO02")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Y ÷") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR(0x00f7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("IO03")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"T ×") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR(0x00d7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('/')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ERASE RESTART") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("IO04")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('9')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE RUN/STOP") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("IO05")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPECIAL ALARM") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("IO06")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NEXT CLOCK") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("IO07")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PREVIOUS COLOR") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("UV201-31")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('.')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK TEXT") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MASTER CONTROL") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vidbrain_state::reset_changed), 0)

	PORT_START("JOY1-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(1)

	PORT_START("JOY1-Y")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_Y ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(1)

	PORT_START("JOY2-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(2)

	PORT_START("JOY2-Y")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_Y ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(2)

	PORT_START("JOY3-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(3)

	PORT_START("JOY3-Y")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_Y ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(3)

	PORT_START("JOY4-X")
	PORT_BIT( 0xff, 50, IPT_AD_STICK_X ) PORT_MINMAX(0, 99) PORT_SENSITIVITY(25) PORT_PLAYER(4)

	PORT_START("JOY4-Y")
	PORT_BIT( 0xff, 70, IPT_AD_STICK_Y ) PORT_MINMAX(0, 139) PORT_SENSITIVITY(25) PORT_PLAYER(4)

	PORT_START("JOY-R")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

void vidbrain_state::hblank_w(int state)
{
	m_hblank = state;

	update_555();
}

void vidbrain_state::uv_int_w(int state)
{
	m_uv_int = state;

	update_ext_int();
}

void vidbrain_state::acc_int_w(int state)
{
	m_acc_int = state;

	update_ext_int();
}

void vidbrain_state::update_ext_int()
{
	// EXT INT is open collector, pulled low by the UV201, the accessory jack, or while joystick enable is low and the 555 has timed out
	bool const asserted = m_uv_int || m_acc_int || (!m_joy_enable && !m_555_out);

	m_smi->ext_int_w(asserted);
	m_uv->ext_int_w(!asserted);
}

void vidbrain_state::update_555()
{
	bool const trigger = m_hblank && m_joy_enable;

	if (trigger && !m_555_out)
	{
		m_555_out = true;
		m_555_timeout = false;
		m_555_charge = 0;
		m_555_time = machine().time();

		update_555_timer();
		update_ext_int();
	}
	else if (!trigger && m_555_out && m_555_timeout)
	{
		m_555_out = false;

		update_ext_int();
	}
}

void vidbrain_state::update_555_timer()
{
	if (!m_555_out || m_555_timeout)
		return;

	// integrate the timing capacitor voltage with the previously selected pots
	attotime const now = machine().time();

	m_555_charge = 1.0 - (1.0 - m_555_charge) * std::exp(-(now - m_555_time).as_double() * m_555_conductance / CAP_U(0.003));
	m_555_time = now;

	// selected pots charge the timing capacitor in parallel
	m_555_conductance = 0;

	for (int i = 0; i < 8; i++)
		if (BIT(m_keylatch, i))
			m_555_conductance += 1.0 / (RES_K(3.9) + RES_K(m_joy[i]->read()));

	if (m_555_charge >= 2.0 / 3.0)
		m_timer_ne555->adjust(attotime::zero);
	else if (m_555_conductance > 0)
		m_timer_ne555->adjust(attotime::from_double(std::log((1.0 - m_555_charge) * 3.0) * CAP_U(0.003) / m_555_conductance));
	else
		m_timer_ne555->adjust(attotime::never);
}

uint8_t vidbrain_state::memory_read_byte(offs_t offset)
{
	if (offset < 0x800)
		m_bd = m_res2[offset];
	else if (offset >= 0x1800)
		m_bd = m_exp->cs2_r(offset & 0x7ff);
	else if (offset >= 0x1000)
		m_bd = m_exp->cs1_r(offset & 0x7ff);
	else if (offset >= 0xc00)
		m_bd = m_ram[offset & 0x3ff];

	return m_bd;
}


//-------------------------------------------------
//  bus_wait - hold the CPU clock for a buffered
//  bus cycle
//-------------------------------------------------

void vidbrain_state::bus_wait(bool registers)
{
	if (machine().side_effects_disabled())
		return;

	attotime const now = machine().time();
	uint32_t const brclk = m_uv->clock();
	uint64_t const edge = now.as_ticks(brclk) + 1;
	attotime const end = attotime::from_ticks(edge + m_uv->bus_request(registers), brclk);

	m_maincpu->adjust_icount(-int(m_maincpu->attotime_to_clocks(end - now)) - 1);
}

uint8_t vidbrain_state::bus_read(uint8_t data)
{
	if (!machine().side_effects_disabled())
		m_bd = data;

	return data;
}

uint8_t vidbrain_state::uv_r(offs_t offset)
{
	bus_wait(true);
	return bus_read(m_uv->read(offset));
}

void vidbrain_state::uv_w(offs_t offset, uint8_t data)
{
	bus_wait(true);
	m_uv->write(offset, m_bd = data);
}

uint8_t vidbrain_state::ext_r()
{
	bus_wait(true);
	return m_bd;
}

void vidbrain_state::ext_w(uint8_t data)
{
	bus_wait(true);
	m_bd = data;
}

uint8_t vidbrain_state::ram_r(offs_t offset)
{
	bus_wait(false);
	return bus_read(m_ram[offset]);
}

void vidbrain_state::ram_w(offs_t offset, uint8_t data)
{
	bus_wait(false);
	m_ram[offset] = m_bd = data;
}

uint8_t vidbrain_state::cs1_r(offs_t offset)
{
	bus_wait(false);
	return bus_read(m_exp->cs1_r(offset));
}

void vidbrain_state::cs1_w(offs_t offset, uint8_t data)
{
	bus_wait(false);
	m_exp->cs1_w(offset, m_bd = data);
}

uint8_t vidbrain_state::cs2_r(offs_t offset)
{
	bus_wait(false);
	return bus_read(m_exp->cs2_r(offset));
}

void vidbrain_state::cs2_w(offs_t offset, uint8_t data)
{
	bus_wait(false);
	m_exp->cs2_w(offset, m_bd = data);
}

uint8_t vidbrain_state::res2_r(offs_t offset)
{
	bus_wait(false);
	return bus_read(m_res2[offset]);
}

uint8_t vidbrain_state::unmap_r(offs_t offset)
{
	bus_wait(false);
	return bus_read(m_exp->unmap_r(offset));
}

void vidbrain_state::unmap_w(offs_t offset, uint8_t data)
{
	bus_wait(false);
	m_exp->unmap_w(offset, m_bd = data);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(vidbrain_state::joystick_tick)
{
	// the output is held high while the trigger input is still low
	m_555_timeout = true;

	update_555();
}


//-------------------------------------------------
//  MACHINE_START( vidbrain )
//-------------------------------------------------

void vidbrain_state::machine_start()
{
	// allocate timers
	m_timer_ne555 = timer_alloc(FUNC(vidbrain_state::joystick_tick), this);
	m_timer_field[0] = timer_alloc(FUNC(vidbrain_state::field_tick), this);
	m_timer_field[1] = timer_alloc(FUNC(vidbrain_state::field_tick), this);

	// register for state saving
	save_item(NAME(m_keylatch));
	save_item(NAME(m_joy_enable));
	save_item(NAME(m_hblank));
	save_item(NAME(m_555_out));
	save_item(NAME(m_555_timeout));
	save_item(NAME(m_555_charge));
	save_item(NAME(m_555_conductance));
	save_item(NAME(m_555_time));
	save_item(NAME(m_uv_int));
	save_item(NAME(m_acc_int));
	save_item(NAME(m_port1));
	save_item(NAME(m_reset_pressed));
	save_item(NAME(m_reset));
	save_item(NAME(m_eject));
	save_item(NAME(m_sound_clk));
	save_item(NAME(m_bd));
}


void vidbrain_state::machine_reset()
{
	screen_device &screen = m_uv->screen();

	m_timer_field[0]->adjust(screen.time_until_pos(0), 0, screen.frame_period());
	m_timer_field[1]->adjust(screen.time_until_pos(263), 0, screen.frame_period());

	m_port1 = 0;
	m_joy_enable = true;
	m_acc->io_w(0);

	update_ext_int();
}


//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( vidbrain )
//-------------------------------------------------

void vidbrain_state::vidbrain(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &vidbrain_state::vidbrain_mem);
	m_maincpu->set_addrmap(AS_IO, &vidbrain_state::vidbrain_io);
	m_maincpu->int_cycle_callback().set(m_smi, FUNC(f3853_device::int_acknowledge));

	// video hardware
	UV201(config, m_uv, 14.318181_MHz_XTAL / 4);
	m_uv->set_screen("screen");
	m_uv->ext_int_wr_callback().set(FUNC(vidbrain_state::uv_int_w));
	m_uv->hblank_wr_callback().set(FUNC(vidbrain_state::hblank_w));
	m_uv->db_rd_callback().set(FUNC(vidbrain_state::memory_read_byte));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_screen_update(m_uv, FUNC(uv201_device::screen_update));
	screen.set_raw(14.318181_MHz_XTAL / 4, 228, 33, 222, 525, 0, 483);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	// U16 drives SOUND OUT through a 120K/56K/120K/120K ladder, AC coupled into the modulator by 0.01uF and 1M
	FILTER_RC(config, "sound_filter").set_rc(filter_rc_device::HIGHPASS, RES_M(1), 0, 0, CAP_U(0.01)).add_route(ALL_OUTPUTS, "speaker", 0.25);
	DAC_1BIT(config, m_dac[0]).add_route(ALL_OUTPUTS, "sound_filter", 0.2542);
	DAC_1BIT(config, m_dac[1]).add_route(ALL_OUTPUTS, "sound_filter", 0.4915);

	// devices
	F3853(config, m_smi, 4_MHz_XTAL / 2);
	m_smi->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);

	// cartridge
	VIDEOBRAIN_EXPANSION_SLOT(config, m_exp, vidbrain_expansion_cards, nullptr);
	m_exp->eject_wr_callback().set(FUNC(vidbrain_state::eject_w));

	// accessory jack
	VIDEOBRAIN_ACCESSORY_PORT(config, m_acc, videobrain_accessories, nullptr);
	m_acc->ext_int_callback().set(FUNC(vidbrain_state::acc_int_w));

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("vidbrain");

}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( vidbrain )
//-------------------------------------------------

ROM_START( vidbrain )
	ROM_REGION( 0x800, "res1", 0 )
	ROM_LOAD( "uvres_1n.d67", 0x000, 0x800, CRC(065fe7c2) SHA1(9776f9b18cd4d7142e58eff45ac5ee4bc1fa5a2a) )

	ROM_REGION( 0x800, "res2", 0 )
	ROM_LOAD( "resn2.e5", 0x000, 0x800, CRC(1d85d7be) SHA1(26c5a25d1289dedf107fa43aa8dfc14692fd9ee6) )
ROM_END

} // anonymous namespace



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                        FULLNAME                     FLAGS
COMP( 1978, vidbrain, 0,      0,      vidbrain, vidbrain, vidbrain_state, empty_init, "VideoBrain Computer Company", "VideoBrain FamilyComputer", MACHINE_SUPPORTS_SAVE )
