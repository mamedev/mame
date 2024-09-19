// license:BSD-3-Clause
// copyright-holders:Robbbert
/*************************************************************************************

PINBALL
Micropin : Pentacup
First version used a 6800, but a later revision used a 8085A.

An unusual design. All the guides and the flippers are made of wireform. The 4 bumpers
are raised metal silver studs. There's no conventional rollovers, instead there are
sensors under the board. The bumpers and slings still propel the ball, so it's presumed
that the studs and wireforms bend a little, allowing a solenoid to pulse the ball back.
The schematic doesn't include the switch, lamp, or solenoid matrix, or any info about
them at all. From a video, it seems that on the playboard there are 27 lamps, 15 solenoids,
and 21 sensors. There's no shooter or lane, the ball being injected from between the flippers.

Status:
- Rev.1: Working. At start of ball, hold X and tap Z, so that the player light becomes steady,
          then release both keys. This enables sensors.
- Rev.2: Starts up then does nothing

Rev.2:
- No manuals or schematics available.
- Uses a different layout, not coded.
- Picture of display panel shows 52x 7seg digits, and 10x round LEDs.

ToDo:
- Rev 1: Mechanical sounds, outputs (need better schematic), electronic volume control.
- Rev.2: Mechanical sounds, outputs, inputs, displays, sound (no info available)

**************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/i8085/i8085.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "speaker.h"

#include "micropin.lh"

namespace {

class micropin_state : public genpin_class
{
public:
	micropin_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_digits(*this, "digit%d", 0U)
		, m_leds(*this, "led%d", 0U)
	{ }

	void pentacup2(machine_config &config);

protected:
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
	u8 m_led_time[10]{}; // size must match m_leds
	void mr_common();
	void ms_common();
	output_finder<76> m_digits;
	output_finder<10> m_leds;
};

class pent6800_state : public micropin_state
{
public:
	pent6800_state(const machine_config &mconfig, device_type type, const char *tag)
		: micropin_state(mconfig, type, tag)
		, m_v1cpu(*this, "v1cpu")
		, m_pia51(*this, "pia51")
		, m_beep(*this, "beeper")
	{ }

	void pent6800(machine_config &config);

private:
	u8 pia51_r(offs_t offset);
	void pia51_w(offs_t offset, u8 data);
	u8 p51b_r();
	void sol_w(u8 data);
	void p50ca2_w(int state);
	void p51ca2_w(int state);
	void sw_w(u8 data);
	void lamp_w(u8 data);
	void p50a_w(u8 data);
	void p50b_w(u8 data);
	void p51a_w(u8 data);
	void p51b_w(u8 data) { };  // volume control
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_row = 0U;
	u8 m_counter = 0U;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<m6800_cpu_device> m_v1cpu;
	required_device<pia6821_device> m_pia51;
	required_device<beep_device> m_beep;
};

class pent8085_state : public micropin_state
{
public:
	pent8085_state(const machine_config &mconfig, device_type type, const char *tag)
		: micropin_state(mconfig, type, tag)
		, m_v2cpu(*this, "v2cpu")
	{ }

	void pent8085(machine_config &config);

private:
	void clock_w(int state);
	void disp_w(offs_t, u8);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<i8085a_cpu_device> m_v2cpu;
};

void pent6800_state::mem_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x01ff).ram().share("nvram"); // 4x 6561 RAM
	map(0x4000, 0x4005).w(FUNC(pent6800_state::sw_w));
	map(0x4000, 0x4000).portr("X1");
	map(0x4001, 0x4001).portr("X2");
	map(0x4002, 0x4002).portr("X3");
	map(0x4003, 0x4003).portr("X4");
	map(0x4004, 0x4004).portr("X5");
	map(0x5000, 0x5003).rw("pia50", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x5100, 0x5103).rw(FUNC(pent6800_state::pia51_r), FUNC(pent6800_state::pia51_w));
	map(0x5200, 0x5200).w(FUNC(pent6800_state::sol_w));
	map(0x5202, 0x5202).w(FUNC(pent6800_state::lamp_w));
	map(0x5203, 0x5203).nopw();
	map(0x6400, 0x7fff).rom().region("v1cpu", 0);
}

void pent8085_state::mem_map(address_map &map)
{
	map.global_mask(0x3fff);
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
}

void pent8085_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).w(FUNC(pent8085_state::disp_w));
	map(0x00, 0x00).portr("X0");
	map(0x01, 0x01).portr("X1");
	map(0x02, 0x02).portr("X2");
	map(0x03, 0x03).portr("X3");
	map(0x04, 0x04).portr("X4");
	map(0x05, 0x05).portr("X5");
}

static INPUT_PORTS_START( pent6800 )
	PORT_START("X0")
	// bits 0-3 are masked off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Vol+")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Vol-")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Tilt Alarm")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Tilt 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Ball Enable")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Tilt 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt 3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Tilt 4")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP48")
INPUT_PORTS_END

// guess
static INPUT_PORTS_START( pent8085 )
	PORT_START("X0")
	PORT_DIPNAME( 0x80, 0x80, "SW 01")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 02")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 03")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 04")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 05")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 06")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "SW 07")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x01, "SW 08") // Machine won't boot with this on
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("X1")
	PORT_DIPNAME( 0x80, 0x80, "SW 09")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "SW 10")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "SW 11")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "SW 12")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "SW 13")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "SW 14")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "SW 15")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x01, "SW 16")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP01")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP02")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP03")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP04")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP05")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP06")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP07")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP08")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP16")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP21")
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM at boot")
	PORT_DIPSETTING(    0x20, DEF_STR(On))
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP24")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP32")
INPUT_PORTS_END

u8  pent6800_state::pia51_r(offs_t offset)
{
	return m_pia51->read(offset) ^ 0xff;
}

void pent6800_state::pia51_w(offs_t offset, u8 data)
{
	m_pia51->write(offset, data ^ 0xff);
}

// lamps and disp strobes
void pent6800_state::lamp_w(u8 data)
{
	m_row = data & 15;
	m_counter = 0;
	// lamps
}

// solenoids
void pent6800_state::sol_w(u8 data)
{
}

// offs 0,5 = solenoids; else lamps
void pent6800_state::sw_w(u8 data)
{
}

void pent6800_state::p50a_w(u8 data)
{
	m_counter++;
	if (m_counter == 1)
	{
		static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
		m_digits[m_row] = patterns[data&15];
		m_digits[m_row+20] = patterns[data>>4];
	}
}

void pent6800_state::p50b_w(u8 data)
{
	m_counter++;
	if (m_counter == 2)
	{
		static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
		m_digits[m_row+40] = patterns[data&15];
		m_digits[m_row+60] = patterns[data>>4];
	}
}

// round LEDs on score panel
void pent6800_state::p50ca2_w(int state)
{
	if ((!state) && (m_row < 8))
	{
		m_led_time[m_row] = 12;
		m_leds[m_row] = 0; // turn on
	}
}

// sound & volume
// Sound consists of a 16-resistor chain controlling the frequency of a NE555.
// We use a beeper to make the tones.
// There's an electronic volume control which is not emulated. The volume level
//  is controlled by the Vol+ and Vol- buttons.
void pent6800_state::p51a_w(u8 data)
{
	static uint16_t frequency[16] = { 387, 435, 488, 517, 581, 652, 691, 775, 870, 977, 1035, 1161, 1304, 1381, 1550, 1740 };
	m_beep->set_clock(frequency[data & 15]);
}

// Sound on/off
void pent6800_state::p51ca2_w(int state)
{
	m_beep->set_state(state);
}

u8 pent6800_state::p51b_r()
{
	return ioport("X0")->read();
}

TIMER_DEVICE_CALLBACK_MEMBER( micropin_state::timer_a )
{
	// turn off round leds that aren't being refreshed
	for (u8 i = 0; i < 8; i++)
	{
		if (m_led_time[i])
		{
			m_led_time[i]--;
			if (m_led_time[i] == 0)
			{
				m_leds[i] = 1; // turn off
			}
		}
	}
}

// This is completely wrong - it has to handle all outputs, not just digits
void pent8085_state::disp_w(offs_t offset, u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
	m_digits[offset] = patterns[BIT(~data, 0, 4)];
	m_digits[offset+20] = patterns[BIT(~data, 4, 4)];
}

void pent8085_state::clock_w(int state)
{
	if (state)
		m_v2cpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
	else
		m_v2cpu->set_input_line(I8085_RST65_LINE, HOLD_LINE);
}

void micropin_state::ms_common()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_leds.resolve();
	//m_io_outputs.resolve();

	save_item(NAME(m_led_time));
}

void pent6800_state::machine_start()
{
	ms_common();
	save_item(NAME(m_row));
	save_item(NAME(m_counter));
}

void micropin_state::mr_common()
{
	genpin_class::machine_reset();
	//for (u8 i = 0; i < m_io_outputs.size(); i++)
		//m_io_outputs[i] = 0;

	for (u8 i = 0; i < std::size(m_led_time); i++)
		m_led_time[i] = 5;
}

void pent6800_state::machine_reset()
{
	mr_common();
	m_row = 0;
}

void pent8085_state::machine_start()
{
	ms_common();
}

void pent8085_state::machine_reset()
{
	mr_common();
}


void pent6800_state::pent6800(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_v1cpu, XTAL(2'000'000) / 2);
	m_v1cpu->set_addrmap(AS_PROGRAM, &pent6800_state::mem_map);
	m_v1cpu->set_periodic_int(FUNC(pent6800_state::irq0_line_hold), attotime::from_hz(500));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_micropin);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 387).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	pia6821_device &pia50(PIA6821(config, "pia50"));
	pia50.writepa_handler().set(FUNC(pent6800_state::p50a_w));
	pia50.writepb_handler().set(FUNC(pent6800_state::p50b_w));
	pia50.ca2_handler().set(FUNC(pent6800_state::p50ca2_w));

	PIA6821(config, m_pia51);
	m_pia51->writepa_handler().set(FUNC(pent6800_state::p51a_w));
	m_pia51->readpb_handler().set(FUNC(pent6800_state::p51b_r));
	m_pia51->writepb_handler().set(FUNC(pent6800_state::p51b_w));
	m_pia51->ca2_handler().set(FUNC(pent6800_state::p51ca2_w));
	//m_pia51->cb2_handler().set(FUNC(pent6800_state::p51cb2_w)); // issue NMI which in turn causes reboot

	TIMER(config, "timer_a").configure_periodic(FUNC(pent6800_state::timer_a), attotime::from_hz(100));
}

void pent8085_state::pent8085(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_v2cpu, 2000000);
	m_v2cpu->set_addrmap(AS_PROGRAM, &pent8085_state::mem_map);
	m_v2cpu->set_addrmap(AS_IO, &pent8085_state::io_map);

	clock_device &cpoint_clock(CLOCK(config, "cpoint_clock", 50)); // crosspoint detector
	cpoint_clock.signal_handler().set(FUNC(pent8085_state::clock_w));

	config.set_default_layout(layout_micropin); // wrong layout
//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Sound */
	genpin_audio(config);
	TIMER(config, "timer_a").configure_periodic(FUNC(pent8085_state::timer_a), attotime::from_hz(100));
}

/*-------------------------------------------------------------------
/ Pentacup
/-------------------------------------------------------------------*/
ROM_START(pentacup)
	ROM_REGION(0x1c00, "v1cpu", 0)
	ROM_LOAD("ic2.bin", 0x0000, 0x0400, CRC(fa468a0f) SHA1(e9c8028bcd5b87d24f4588516536767a869c38ff))
	ROM_LOAD("ic3.bin", 0x0400, 0x0400, CRC(7bfdaec8) SHA1(f2037c0e2d4acf0477351ecafc9f0826e9d64d76))
	ROM_LOAD("ic4.bin", 0x0800, 0x0400, CRC(5e0fcb1f) SHA1(e529539c6eb1e174a799ad6abfce9e31870ff8af))
	ROM_LOAD("ic5.bin", 0x0c00, 0x0400, CRC(a26c6e0b) SHA1(21c4c306fbc2da52887e309b1c83a1ea69501c1f))
	ROM_LOAD("ic6.bin", 0x1000, 0x0400, CRC(4715ac34) SHA1(b6d8c20c487db8d7275e36f5793666cc591a6691))
	ROM_LOAD("ic7.bin", 0x1400, 0x0400, CRC(c58d13c0) SHA1(014958bc69ff326392a5a7782703af0980e6e170))
	ROM_LOAD("ic8.bin", 0x1800, 0x0400, CRC(9f67bc65) SHA1(504008d4c7c23a14fdf247c9e6fc00e95d907d7b))
ROM_END

ROM_START(pentacup2)
	ROM_REGION(0x2000, "v2cpu", 0)
	ROM_LOAD("micro_1.bin", 0x0000, 0x0800, CRC(62d04111) SHA1(f0ce705c06a43a81293d8610394ce7c4143148e9))
	ROM_LOAD("micro_2.bin", 0x0800, 0x0800, CRC(832e4223) SHA1(1409b0c7de35012b9d0eba9bb73b52aecc93c0f2))
	ROM_LOAD("micro_3.bin", 0x1000, 0x0800, CRC(9d5d04d1) SHA1(1af32c418b73ee457f06ee9a8362cfec75e61f30))
	ROM_LOAD("micro_4.bin", 0x1800, 0x0800, CRC(358ffd6a) SHA1(f5299e39d991bf882f827a62a1d9bb18e46dbcfc))
	// 2 undumped proms DMA-01, DMA-02
ROM_END

ROM_START(pentacupt)
	ROM_REGION(0x10000, "v2cpu", 0)
	ROM_LOAD("microt_1.bin", 0x0000, 0x0800, CRC(690646eb) SHA1(86253b61ac9554ee5bdcdf9c0a2302fc393b9ada))
	ROM_LOAD("microt_2.bin", 0x0800, 0x0800, CRC(51d09098) SHA1(4efe3a05ad60f0fc52aa5402e660f34b99855b59))
	ROM_LOAD("microt_3.bin", 0x1000, 0x0800, CRC(cefb0966) SHA1(836491745417fc0d5f88c01a9c69a5c322d194be))
	ROM_LOAD("microt_4.bin", 0x1800, 0x0800, CRC(6f691929) SHA1(a18352312706e0f0af14a33fac31c3f5f7156ba8))
ROM_END

} // anonymous namespace

GAME(1978,  pentacup,  0,         pent6800,  pent6800, pent6800_state, empty_init, ROT0, "Micropin", "Pentacup (rev. 1)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  pentacup2, pentacup,  pent8085,  pent8085, pent8085_state, empty_init, ROT0, "Micropin", "Pentacup (rev. 2)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  pentacupt, pentacup,  pent8085,  pent8085, pent8085_state, empty_init, ROT0, "Micropin", "Pentacup (rev. T)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
