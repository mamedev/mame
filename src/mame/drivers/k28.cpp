// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  Tiger Electronics K28: Talking Learning Computer (model 7-230/7-231)
  * PCB marked PB-123 WIZARD, TIGER
  * Intel P8021 MCU with 1KB internal ROM
  * MM5445N VFD driver, 9-digit alphanumeric display same as snmath
  * 2*TMS6100 (32KB VSM)
  * SC-01-A speech chip

  3 models exist:
  - 7-230: darkblue case, toy-ish looks
  - 7-231: gray case, hardware is the same
  - 7-232: this one is completely different hw --> driver tispeak.cpp

  TODO:
  - external module support (no dumps yet)
  - make MM5445N a device

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"
#include "machine/tms6100.h"
#include "sound/votrax.h"
#include "speaker.h"

#include "k28.lh"


class k28_state : public driver_device
{
public:
	k28_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms6100(*this, "tms6100"),
		m_speech(*this, "speech"),
		m_onbutton_timer(*this, "on_button"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	void k28(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_on);

private:
	// devices
	required_device<i8021_device> m_maincpu;
	required_device<tms6100_device> m_tms6100;
	required_device<votrax_sc01_device> m_speech;
	required_device<timer_device> m_onbutton_timer;
	required_ioport_array<7> m_inp_matrix;
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;

	// display common
	int m_display_wait;             // led/lamp off-delay in milliseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(u32 digits, u32 mask);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

	bool m_power_on;
	u8 m_inp_mux;
	u8 m_phoneme;
	int m_speech_strobe;
	int m_vfd_data_enable;
	int m_vfd_data_in;
	int m_vfd_clock;
	u64 m_vfd_shiftreg;
	u64 m_vfd_shiftreg_out;
	int m_vfd_shiftcount;

	DECLARE_WRITE8_MEMBER(mcu_p0_w);
	DECLARE_READ8_MEMBER(mcu_p1_r);
	DECLARE_READ8_MEMBER(mcu_p2_r);
	DECLARE_WRITE8_MEMBER(mcu_p2_w);
	DECLARE_WRITE_LINE_MEMBER(mcu_prog_w);

	void power_off();

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset/power

void k28_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();
	m_out_a.resolve();
	m_out_digit.resolve();

	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_power_on = false;
	m_inp_mux = 0;
	m_phoneme = 0x3f;
	m_speech_strobe = 0;
	m_vfd_data_enable = 0;
	m_vfd_data_in = 0;
	m_vfd_clock = 0;
	m_vfd_shiftreg = 0;
	m_vfd_shiftreg_out = 0;
	m_vfd_shiftcount = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_power_on));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_phoneme));
	save_item(NAME(m_speech_strobe));
	save_item(NAME(m_vfd_data_enable));
	save_item(NAME(m_vfd_data_in));
	save_item(NAME(m_vfd_clock));
	save_item(NAME(m_vfd_shiftreg));
	save_item(NAME(m_vfd_shiftreg_out));
	save_item(NAME(m_vfd_shiftcount));
}

void k28_state::machine_reset()
{
	m_power_on = true;
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	// the game relies on reading the on-button as pressed when it's turned on
	m_onbutton_timer->adjust(attotime::from_msec(250));
}

INPUT_CHANGED_MEMBER(k28_state::power_on)
{
	if (newval && !m_power_on)
		machine_reset();
}

void k28_state::power_off()
{
	m_power_on = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void k28_state::display_update()
{
	for (int y = 0; y < m_display_maxy; y++)
	{
		u32 active_state = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_power_on && m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			u32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state |= (ds << x);

			// output to y.x, or y.a when always-on
			if (x != m_display_maxx)
				m_out_x[y][x] = ds;
			else
				m_out_a[y] = ds;
		}

		// output to digity
		if (m_display_segmask[y] != 0)
			m_out_digit[y] = active_state & m_display_segmask[y];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(k28_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void k28_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void k28_state::set_display_segmask(u32 digits, u32 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void k28_state::display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	display_update();
}



/***************************************************************************

  I/O, Address Map(s)

***************************************************************************/

WRITE8_MEMBER(k28_state::mcu_p0_w)
{
	// d0,d1: phoneme high bits
	// d0-d2: input mux high bits
	m_inp_mux = (m_inp_mux & 0xf) | (~data << 4 & 0x70);
	m_phoneme = (m_phoneme & 0xf) | (data << 4 & 0x30);

	// d3: SC-01 strobe, latch phoneme on rising edge
	int strobe = data >> 3 & 1;
	if (strobe && !m_speech_strobe)
		m_speech->write(m_phoneme);
	m_speech_strobe = strobe;

	// d5: VFD driver data enable
	m_vfd_data_enable = ~data >> 5 & 1;
	if (m_vfd_data_enable)
		m_vfd_shiftreg = (m_vfd_shiftreg & ~u64(1)) | m_vfd_data_in;

	// d4: VSM chip enable
	// d6: VSM M0
	// d7: VSM M1
	m_tms6100->cs_w(~data >> 4 & 1);
	m_tms6100->m0_w(data >> 6 & 1);
	m_tms6100->m1_w(data >> 7 & 1);
	m_tms6100->clk_w(1);
	m_tms6100->clk_w(0);
}

READ8_MEMBER(k28_state::mcu_p1_r)
{
	u8 data = 0;

	// multiplexed inputs (active low)
	for (int i = 0; i < 7; i++)
		if (m_inp_mux >> i & 1)
		{
			data |= m_inp_matrix[i]->read();

			// force press on-button at boot
			if (i == 5 && m_onbutton_timer->enabled())
				data |= 1;
		}

	return data ^ 0xff;
}

READ8_MEMBER(k28_state::mcu_p2_r)
{
	// d3: VSM data
	return (m_tms6100->data_line_r()) ? 8 : 0;
}

WRITE8_MEMBER(k28_state::mcu_p2_w)
{
	// d0: VFD driver serial data
	m_vfd_data_in = data & 1;
	if (m_vfd_data_enable)
		m_vfd_shiftreg = (m_vfd_shiftreg & ~u64(1)) | m_vfd_data_in;

	// d0-d3: VSM data, input mux and SC-01 phoneme lower nibble
	m_tms6100->add_w(space, 0, data);
	m_inp_mux = (m_inp_mux & ~0xf) | (~data & 0xf);
	m_phoneme = (m_phoneme & ~0xf) | (data & 0xf);
}

WRITE_LINE_MEMBER(k28_state::mcu_prog_w)
{
	// 8021 PROG: clock VFD driver
	bool rise = state == 1 && !m_vfd_clock;
	m_vfd_clock = state;

	// on rising edge
	if (rise)
	{
		// leading 1 triggers shift start
		if (m_vfd_shiftcount == 0 && ~m_vfd_shiftreg & 1)
			return;

		// output shiftreg on 35th clock
		if (m_vfd_shiftcount == 35)
		{
			m_vfd_shiftcount = 0;

			// output 0-15: digit segment data
			u16 seg_data = (u16)(m_vfd_shiftreg >> 19);
			seg_data = bitswap<16>(seg_data,0,1,13,9,10,12,14,8,3,4,5,2,15,11,6,7);

			// output 16-24: digit select
			u16 digit_sel = (u16)(m_vfd_shiftreg >> 10) & 0x1ff;
			set_display_segmask(0x1ff, 0x3fff);
			display_matrix(16, 9, seg_data, digit_sel);

			// output 25: power-off request on falling edge
			if (~m_vfd_shiftreg & m_vfd_shiftreg_out & 0x200)
				power_off();
			m_vfd_shiftreg_out = m_vfd_shiftreg;
		}
		else
		{
			m_vfd_shiftreg <<= 1;
			m_vfd_shiftcount++;
		}
	}
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( k28 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Yes/True")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("No/False")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Select")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("Scroll")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("<")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Erase/Clear")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("Menu")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME(">")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter/Start")

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Prompt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Say It Again(Repeat)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, k28_state, power_on, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("=")

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void k28_state::k28(machine_config &config)
{
	/* basic machine hardware */
	I8021(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->bus_out_cb().set(FUNC(k28_state::mcu_p0_w));
	m_maincpu->p1_in_cb().set(FUNC(k28_state::mcu_p1_r));
	m_maincpu->p2_in_cb().set(FUNC(k28_state::mcu_p2_r));
	m_maincpu->p2_out_cb().set(FUNC(k28_state::mcu_p2_w));
	m_maincpu->prog_out_cb().set(FUNC(k28_state::mcu_prog_w));
	m_maincpu->t1_in_cb().set("speech", FUNC(votrax_sc01_device::request)); // SC-01 A/R pin

	TMS6100(config, m_tms6100, 3.579545_MHz_XTAL); // CLK tied to 8021 ALE pin

	TIMER(config, "on_button").configure_generic(timer_device::expired_delegate());
	TIMER(config, "display_decay").configure_periodic(FUNC(k28_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_k28);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	VOTRAX_SC01(config, "speech", 760000).add_route(ALL_OUTPUTS, "mono", 0.5); // measured 760kHz on its RC pin
}



/***************************************************************************

  ROM Defs, Game driver(s)

***************************************************************************/

ROM_START( k28 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "p8021", 0x0000, 0x0400, CRC(15536d20) SHA1(fac98ce652340ffb2d00952697c3a9ce75393fa4) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff? = space reserved for cartridge
	ROM_LOAD( "cm62050.vsm", 0x0000, 0x4000, CRC(6afb8645) SHA1(e22435568ed11c6516a3b4008131f99cd4e47aa9) )
	ROM_LOAD( "cm62051.vsm", 0x4000, 0x4000, CRC(0fa61baa) SHA1(831be669423ba60c7f85a896b4b09a1295478bd9) )
ROM_END



//    YEAR  NAME  PARENT CMP MACHINE  INPUT  CLASS      INIT        COMPANY              FULLNAME                                        FLAGS
COMP( 1981, k28,  0,      0, k28,     k28,   k28_state, empty_init, "Tiger Electronics", "K28: Talking Learning Computer (model 7-230)", MACHINE_SUPPORTS_SAVE )
