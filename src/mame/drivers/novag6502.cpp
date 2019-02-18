// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

    Novag generic 6502 based chess computer driver

    NOTE: MAME doesn't include a generalized implementation for boardpieces yet,
    greatly affecting user playability of emulated electronic board games.
    As workaround for the chess games, use an external chess GUI on the side,
    such as Arena(in editmode).

    TODO:
    - cforte ACIA?
    - verify supercon IRQ and beeper frequency
    - sforte irq active time (21.5us is too long)
    - sforte/sexpert led handling is correct?
    - printer port

*******************************************************************************

Super Constellation Chess Computer (model 844):
- UMC UM6502C @ 4 MHz (8MHz XTAL), 600Hz? IRQ(source unknown?)
- 2*2KB RAM TC5516APL-2 battery-backed, 2*32KB ROM custom label
- TTL, buzzer, 24 LEDs, 8*8 chessboard buttons
- external ports for clock and printer, not emulated here


*******************************************************************************

Constellation Forte:
- R65C02P4 @ 5MHz (10MHz XTAL)
- 2*2KB RAM(NEC D449C-3), 2*32KB ROM(27C256)
- HLCD0538P, 10-digit 7seg LCD display
- TTL, 18 LEDs, 8*8 chessboard buttons

When it was first added to MAME as skeleton driver in mmodular.c, cforteb romset
was assumed to be Super Forte B, but it definitely isn't. I/O is similar to supercon.


*******************************************************************************

Super Expert (model 878/887/902):
- 65C02 @ 5MHz or 6MHz (10MHz or 12MHz XTAL)
- 8KB RAM battery-backed, 3*32KB ROM
- HD44780 LCD controller (16x1)
- beeper(32KHz/32), IRQ(32KHz/128) via MC14060
- optional R65C51P2 ACIA @ 1.8432MHz, for IBM PC interface (only works in version C?)
- printer port, magnetic sensors, 8*8 chessboard leds

I/O via TTL, hardware design was very awkward.
Super Forte is very similar, just a cheaper plastic case and chessboard buttons
instead of magnet sensors.

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "video/hlcd0538.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_cforte.lh" // clickable
#include "novag_sexpert.lh" // clickable
#include "novag_sforte.lh" // clickable
#include "novag_supercon.lh" // clickable


class novag6502_state : public novagbase_state
{
public:
	novag6502_state(const machine_config &mconfig, device_type type, const char *tag) :
		novagbase_state(mconfig, type, tag),
		m_hlcd0538(*this, "hlcd0538"),
		m_rombank(*this, "rombank")
	{ }

	void supercon(machine_config &config);
	void cforte(machine_config &config);
	void sforte_map(address_map &map);
	void sexpert(machine_config &config);
	void sforte(machine_config &config);

	void init_sexpert();

	DECLARE_INPUT_CHANGED_MEMBER(sexpert_cpu_freq);

private:
	optional_device<hlcd0538_device> m_hlcd0538;
	optional_memory_bank m_rombank;

	// Super Constellation
	DECLARE_WRITE8_MEMBER(supercon_mux_w);
	DECLARE_WRITE8_MEMBER(supercon_control_w);
	DECLARE_READ8_MEMBER(supercon_input1_r);
	DECLARE_READ8_MEMBER(supercon_input2_r);
	void supercon_map(address_map &map);

	// Constellation Forte
	void cforte_prepare_display();
	DECLARE_WRITE64_MEMBER(cforte_lcd_output_w);
	DECLARE_WRITE8_MEMBER(cforte_mux_w);
	DECLARE_WRITE8_MEMBER(cforte_control_w);
	void cforte_map(address_map &map);

	// Super Expert
	DECLARE_WRITE8_MEMBER(sexpert_leds_w);
	DECLARE_WRITE8_MEMBER(sexpert_mux_w);
	DECLARE_WRITE8_MEMBER(sexpert_lcd_control_w);
	DECLARE_WRITE8_MEMBER(sexpert_lcd_data_w);
	DECLARE_READ8_MEMBER(sexpert_input1_r);
	DECLARE_READ8_MEMBER(sexpert_input2_r);
	DECLARE_MACHINE_RESET(sexpert);
	void sexpert_map(address_map &map);
	void sexpert_set_cpu_freq();

	// Super Forte
	DECLARE_WRITE8_MEMBER(sforte_lcd_control_w);
	DECLARE_WRITE8_MEMBER(sforte_lcd_data_w);
};


// machine start/reset

void novagbase_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();
	m_out_a.resolve();
	m_out_digit.resolve();

	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_inp_mux = 0;
	m_led_select = 0;
	m_led_data = 0;
	m_lcd_control = 0;
	m_lcd_data = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_data));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_data));
}

void novagbase_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void novagbase_state::display_update()
{
	for (int y = 0; y < m_display_maxy; y++)
	{
		u32 active_state = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
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

TIMER_DEVICE_CALLBACK_MEMBER(novagbase_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void novagbase_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void novagbase_state::set_display_segmask(u32 digits, u32 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void novagbase_state::display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// LCD

void novagbase_state::novag_lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(novagbase_state::novag_lcd_pixel_update)
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 8)
	{
		// internal: (8+8)*1, external: 1*16
		bitmap.pix16(1 + y, 1 + line*8*6 + pos*6 + x) = state ? 1 : 2;
	}
}


// generic input handlers

u16 novagbase_state::read_inputs(int columns)
{
	u16 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}



// Devices, I/O

/******************************************************************************
    Super Constellation
******************************************************************************/

// TTL

WRITE8_MEMBER(novag6502_state::supercon_mux_w)
{
	// d0-d7: input mux, led data
	m_inp_mux = m_led_data = data;
	display_matrix(8, 3, m_led_data, m_led_select);
}

WRITE8_MEMBER(novag6502_state::supercon_control_w)
{
	// d0-d3: ?
	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	display_matrix(8, 3, m_led_data, m_led_select);

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}

READ8_MEMBER(novag6502_state::supercon_input1_r)
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~read_inputs(8) & 0xff;
}

READ8_MEMBER(novag6502_state::supercon_input2_r)
{
	// d0-d5: ?
	// d6,d7: multiplexed inputs (side panel)
	return (read_inputs(8) >> 2 & 0xc0) ^ 0xff;
}



/******************************************************************************
    Constellation Forte
******************************************************************************/

// TTL/generic

void novag6502_state::cforte_prepare_display()
{
	// 3 led rows
	display_matrix(8, 3, m_led_data, m_led_select, false);

	// lcd panel (mostly handled in cforte_lcd_output_w)
	set_display_segmask(0x3ff0, 0xff);
	set_display_size(8, 3+13);
	display_update();
}

WRITE64_MEMBER(novag6502_state::cforte_lcd_output_w)
{
	// 4 rows used
	u32 rowdata[4];
	for (int i = 0; i < 4; i++)
		rowdata[i] = (data >> i & 1) ? u32(data >> 8) : 0;

	// 2 segments per row
	for (int dig = 0; dig < 13; dig++)
	{
		m_display_state[dig+3] = 0;
		for (int i = 0; i < 4; i++)
			m_display_state[dig+3] |= ((rowdata[i] >> (2*dig) & 3) << (2*i));

		m_display_state[dig+3] = bitswap<8>(m_display_state[dig+3],7,2,0,4,6,5,3,1);
	}

	cforte_prepare_display();
}

WRITE8_MEMBER(novag6502_state::cforte_mux_w)
{
	// d0-d7: input mux, led data
	m_inp_mux = m_led_data = data;
	cforte_prepare_display();
}

WRITE8_MEMBER(novag6502_state::cforte_control_w)
{
	// d0: HLCD0538 data in
	// d1: HLCD0538 clk
	// d2: HLCD0538 lcd
	m_hlcd0538->write_data(data & 1);
	m_hlcd0538->write_clk(data >> 1 & 1);
	m_hlcd0538->write_lcd(data >> 2 & 1);

	// d3: unused?

	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	cforte_prepare_display();

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}



/******************************************************************************
    Super Expert
******************************************************************************/

// TTL/generic

WRITE8_MEMBER(novag6502_state::sexpert_lcd_control_w)
{
	// d0: HD44780 RS
	// d1: HD44780 R/W
	// d2: HD44780 E
	if (m_lcd_control & ~data & 4 && ~data & 2)
		m_lcd->write(m_lcd_control & 1, m_lcd_data);
	m_lcd_control = data & 7;
}

WRITE8_MEMBER(novag6502_state::sexpert_lcd_data_w)
{
	// d0-d7: HD44780 data
	m_lcd_data = data;
}

WRITE8_MEMBER(novag6502_state::sexpert_leds_w)
{
	// d0-d7: chessboard leds
	m_led_data = data;
}

WRITE8_MEMBER(novag6502_state::sexpert_mux_w)
{
	// d0: rom bankswitch
	m_rombank->set_entry(data & 1);

	// d3: enable beeper
	m_beeper->set_state(data >> 3 & 1);

	// d4-d7: 74145 to input mux/led select
	m_inp_mux = 1 << (data >> 4 & 0xf) & 0xff;
	display_matrix(8, 8, m_led_data, m_inp_mux);
	m_led_data = 0; // ?
}

READ8_MEMBER(novag6502_state::sexpert_input1_r)
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~read_inputs(8) & 0xff;
}

READ8_MEMBER(novag6502_state::sexpert_input2_r)
{
	// d0-d2: printer port
	// d5-d7: multiplexed inputs (side panel)
	return ~read_inputs(8) >> 3 & 0xe0;
}

void novag6502_state::sexpert_set_cpu_freq()
{
	// machines were released with either 5MHz or 6MHz CPU
	m_maincpu->set_unscaled_clock((ioport("FAKE")->read() & 1) ? (12_MHz_XTAL/2) : (10_MHz_XTAL/2));
}

MACHINE_RESET_MEMBER(novag6502_state, sexpert)
{
	novagbase_state::machine_reset();

	sexpert_set_cpu_freq();
	m_rombank->set_entry(0);
}

void novag6502_state::init_sexpert()
{
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x8000, 0x8000);
}



/******************************************************************************
    Super Forte
******************************************************************************/

WRITE8_MEMBER(novag6502_state::sforte_lcd_control_w)
{
	// d3: rom bankswitch
	m_rombank->set_entry(data >> 3 & 1);

	// assume same as sexpert
	sexpert_lcd_control_w(space, 0, data);
}

WRITE8_MEMBER(novag6502_state::sforte_lcd_data_w)
{
	// d0-d2: input mux/led select
	m_inp_mux = 1 << (data & 7);

	// if lcd is disabled, misc control
	if (~m_lcd_control & 4)
	{
		// d5,d6: led data, but not both at same time?
		if ((data & 0x60) != 0x60)
			display_matrix(2, 8, data >> 5 & 3, m_inp_mux);

		// d7: enable beeper
		m_beeper->set_state(data >> 7 & 1);
	}

	// assume same as sexpert
	sexpert_lcd_data_w(space, 0, data);
}



/******************************************************************************
    Address Maps
******************************************************************************/

// Super Constellation / Constellation Forte

void novag6502_state::supercon_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x1c00, 0x1c00).nopw(); // printer/clock?
	map(0x1d00, 0x1d00).nopw(); // printer/clock?
	map(0x1e00, 0x1e00).rw(FUNC(novag6502_state::supercon_input2_r), FUNC(novag6502_state::supercon_mux_w));
	map(0x1f00, 0x1f00).rw(FUNC(novag6502_state::supercon_input1_r), FUNC(novag6502_state::supercon_control_w));
	map(0x2000, 0xffff).rom();
}

void novag6502_state::cforte_map(address_map &map)
{
	supercon_map(map);
	map(0x1e00, 0x1e00).rw(FUNC(novag6502_state::supercon_input2_r), FUNC(novag6502_state::cforte_mux_w));
	map(0x1f00, 0x1f00).rw(FUNC(novag6502_state::supercon_input1_r), FUNC(novag6502_state::cforte_control_w));
}


// Super Expert / Super Forte

void novag6502_state::sforte_map(address_map &map)
{
	map(0x0000, 0x1fef).ram().share("nvram"); // 8KB RAM, but RAM CE pin is deactivated on $1ff0-$1fff
	map(0x1ff0, 0x1ff0).r(FUNC(novag6502_state::sexpert_input1_r));
	map(0x1ff1, 0x1ff1).r(FUNC(novag6502_state::sexpert_input2_r));
	map(0x1ff2, 0x1ff2).nopw(); // printer
	map(0x1ff3, 0x1ff3).nopw(); // printer
	map(0x1ff6, 0x1ff6).w(FUNC(novag6502_state::sforte_lcd_control_w));
	map(0x1ff7, 0x1ff7).w(FUNC(novag6502_state::sforte_lcd_data_w));
	map(0x1ffc, 0x1fff).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x2000, 0x7fff).rom();
	map(0x8000, 0xffff).bankr("rombank");
}

void novag6502_state::sexpert_map(address_map &map)
{
	sforte_map(map);
	map(0x1ff4, 0x1ff4).w(FUNC(novag6502_state::sexpert_leds_w));
	map(0x1ff5, 0x1ff5).w(FUNC(novag6502_state::sexpert_mux_w));
	map(0x1ff6, 0x1ff6).w(FUNC(novag6502_state::sexpert_lcd_control_w));
	map(0x1ff7, 0x1ff7).w(FUNC(novag6502_state::sexpert_lcd_data_w));
}



/******************************************************************************
    Input Ports
******************************************************************************/

INPUT_PORTS_START( novag_cb_buttons )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
INPUT_PORTS_END

INPUT_PORTS_START( novag_cb_magnets )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_NAME("Board Sensor")
INPUT_PORTS_END


static INPUT_PORTS_START( supercon )
	PORT_INCLUDE( novag_cb_buttons )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Multi Move / Player/Player / King")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Best Move/Random / Training Level / Queen")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Knight")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Rook")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Form Size")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back")
INPUT_PORTS_END


static INPUT_PORTS_START( cforte )
	PORT_INCLUDE( novag_cb_buttons )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Player/Player / Gambit/Large / King")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify/Set Up / Pro-Op")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Random/Tour/Normal / Training Level / Queen")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color / Time Control / Priority")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Flip Display / Clear Board / Clear Book")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Infinite / Knight")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves / Print Evaluations / Print Book")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Interface / Rook")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trace Forward / Auto Play / No/End")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint / Next Best / Yes/Start")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go / ->")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back / Restore / <-")
INPUT_PORTS_END


static INPUT_PORTS_START( sexy_shared )
	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Go")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Take Back / Analyze Games")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("->")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Level")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Flip Display / Time Control")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("<-")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Hint / Next Best")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Priority / Tournament Book / Pawn")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Yes/Start / Start of Game")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Trace Forward / AutoPlay")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pro-Op / Restore Game / Rook")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("No/End / End of Game")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Clear Board / Delete Pro-Op")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Best Move/Random / Review / Knight")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Print Book / Store Game")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Change Color")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Sound / Info / Bishop")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Print Moves / Print Evaluations")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Verify/Set Up / Pro-Op Book/Both Books")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Solve Mate / Infinite / Queen")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Print List / Acc. Time")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("New Game")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Player/Player / Gambit Book / King")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Print Board / Interface")

	PORT_START("FAKE")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, novag6502_state, sexpert_cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "5MHz" )
	PORT_CONFSETTING(    0x01, "6MHz" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(novag6502_state::sexpert_cpu_freq)
{
	sexpert_set_cpu_freq();
}

static INPUT_PORTS_START( sexpert )
	PORT_INCLUDE( novag_cb_magnets )
	PORT_INCLUDE( sexy_shared )
INPUT_PORTS_END

static INPUT_PORTS_START( sforte )
	PORT_INCLUDE( novag_cb_buttons )
	PORT_INCLUDE( sexy_shared )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

MACHINE_CONFIG_START(novag6502_state::supercon)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6502, 8_MHz_XTAL/2)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(novag6502_state, irq0_line_hold, 600) // guessed
	MCFG_DEVICE_PROGRAM_MAP(supercon_map)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TIMER(config, "display_decay").configure_periodic(FUNC(novag6502_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_supercon);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1024); // guessed
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(novag6502_state::cforte)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", R65C02, 10_MHz_XTAL/2)
	MCFG_DEVICE_PROGRAM_MAP(cforte_map)

	const attotime irq_period = attotime::from_hz(32.768_kHz_XTAL/128); // 256Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(novag6502_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_usec(11)); // active for 11us
	TIMER(config, "irq_off").configure_periodic(FUNC(novag6502_state::irq_off<M6502_IRQ_LINE>), irq_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* video hardware */
	HLCD0538(config, m_hlcd0538, 0);
	m_hlcd0538->write_cols_callback().set(FUNC(novag6502_state::cforte_lcd_output_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(novag6502_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_cforte);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(novag6502_state::sexpert)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M65C02, 10_MHz_XTAL/2) // or 12_MHz_XTAL/2
	MCFG_DEVICE_PROGRAM_MAP(sexpert_map)

	const attotime irq_period = attotime::from_hz(32.768_kHz_XTAL/128); // 256Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(novag6502_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(21500)); // active for 21.5us
	TIMER(config, "irq_off").configure_periodic(FUNC(novag6502_state::irq_off<M6502_IRQ_LINE>), irq_period);

	mos6551_device &acia(MOS6551(config, "acia", 0)); // R65C51P2 - RTS to CTS, DCD to GND
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.irq_handler().set_inputline("maincpu", m65c02_device::NMI_LINE);
	acia.rts_handler().set("acia", FUNC(mos6551_device::write_cts));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	rs232.dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_MACHINE_RESET_OVERRIDE(novag6502_state, sexpert)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(6*16+1, 10)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16, 0, 10-1)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	PALETTE(config, "palette", FUNC(novag6502_state::novag_lcd_palette), 3);

	HD44780(config, m_lcd, 0);
	m_lcd->set_lcd_size(2, 8);
	m_lcd->set_pixel_update_cb(FUNC(novag6502_state::novag_lcd_pixel_update), this);

	TIMER(config, "display_decay").configure_periodic(FUNC(novag6502_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_sexpert);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(novag6502_state::sforte)
	sexpert(config);

	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(sforte_map)

	m_irq_on->set_start_delay(m_irq_on->period() - attotime::from_usec(11)); // active for ?us (assume same as cforte)

	config.set_default_layout(layout_novag_sforte);
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( supercon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_8441", 0x0000, 0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0) ) // label obscured by Q.C. sticker
	ROM_LOAD("novag_8442", 0x8000, 0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe) )
ROM_END


ROM_START( cfortea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("a_903_l", 0x0000, 0x8000, CRC(01e7e306) SHA1(6a2b982bb0f412a63f5d3603958dd863e38669d9) ) // NEC D27C256AD-12
	ROM_LOAD("a_903_h", 0x8000, 0x8000, CRC(c5a5573f) SHA1(7e11eb2f3d96bc41386a14a19635427a386ec1ec) ) // "
ROM_END

ROM_START( cforteb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("forte_b_l.bin", 0x0000, 0x8000, CRC(e3d194a1) SHA1(80457580d7c57e07895fd14bfdaf14b30952afca) )
	ROM_LOAD("forte_b_h.bin", 0x8000, 0x8000, CRC(dd824be8) SHA1(cd8666b6b525887f9fc48a730b71ceabcf07f3b9) )
ROM_END


ROM_START( sfortea )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfalo.u3", 0x0000, 0x8000, CRC(86e0230a) SHA1(0d6e18a17e636b8c7292c8f331349d361892d1a8) )
	ROM_LOAD("sfahi.u1", 0x8000, 0x8000, CRC(81c02746) SHA1(0bf68b68ade5a3263bead88da0a8965fc71483c1) )
	ROM_LOAD("sfabook.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sfortea1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfa_lo.u3", 0x0000, 0x8000, CRC(78734bfd) SHA1(b6d8e9efccee6f6d0b0cd257a82162bf8ccec719) )
	ROM_LOAD("sfa_hi_1.u1", 0x8000, 0x8000, CRC(e5e84580) SHA1(bae55c3da7b720bf6ccfb450e383c53cebd5e9ef) )
	ROM_LOAD("sfa_hi_0.u2", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sforteb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("forte_b_lo.u3", 0x0000, 0x8000, CRC(48bfe5d6) SHA1(323642686b6d2fb8db2b7d50c6cd431058078ce1) )
	ROM_LOAD("forte_b_hi1.u1", 0x8000, 0x8000, CRC(9778ca2c) SHA1(d8b88b9768a1a9171c68cbb0892b817d68d78351) )
	ROM_LOAD("forte_b_hi0.u2", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sfortec )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfl_c_111.u3", 0x0000, 0x8000, CRC(f040cf30) SHA1(1fc1220b8ed67cdffa3866d230ce001721cf684f) ) // Toshiba TC57256AD-12
	ROM_LOAD("sfh_c_111.u1", 0x8000, 0x8000, CRC(0f926b32) SHA1(9c7270ecb3f41dd9172a9a7928e6e04e64b2a340) ) // NEC D27C256AD-12
	ROM_LOAD("h0_c_c26.u2", 0x10000, 0x8000, CRC(c6a1419a) SHA1(017a0ffa9aa59438c879624a7ddea2071d1524b8) ) // Toshiba TC57256AD-12
ROM_END


ROM_START( sexperta )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_b15.u3", 0x0000, 0x8000, CRC(6cc9527c) SHA1(29bab809399f2863a88a9c41535ecec0a4fd65ea) )
	ROM_LOAD("se_hi1_b15.u1", 0x8000, 0x8000, CRC(6e57f0c0) SHA1(ea44769a6f54721fd4543366bda932e86e497d43) )
	ROM_LOAD("sef_hi0_a23.u2", 0x10000, 0x8000, CRC(7d4e1528) SHA1(53c7d458a5571afae402f00ae3d0f5066634b068) )
ROM_END

ROM_START( sexpertb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_619.u3", 0x0000, 0x8000, CRC(92002eb6) SHA1(ed8ca16701e00b48fa55c856fa4a8c6613079c02) )
	ROM_LOAD("se_hi1_619.u1", 0x8000, 0x8000, CRC(814b4420) SHA1(c553e6a8c048dcc1cf48d410111a86e06b99d356) )
	ROM_LOAD("sef_hi0_605.u2", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sexpertc )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_v3.6.u3", 0x0000, 0x8000, CRC(5a29105e) SHA1(be37bb29b530dbba847a5e8d27d81b36525e47f7) )
	ROM_LOAD("se_hi1.u1", 0x8000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa) )
	ROM_LOAD("se_hi0.u2", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) )
ROM_END

ROM_START( sexpertc1 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("se_lo_v1.2.u3", 0x0000, 0x8000, CRC(43ed7a9e) SHA1(273c485e5be6b107b6c5c448003ba7686d4a6d06) )
	ROM_LOAD("se_hi1.u1", 0x8000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa) )
	ROM_LOAD("se_hi0.u2", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) )
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT     CLASS            INIT          COMPANY  FULLNAME               FLAGS
CONS( 1984, supercon,  0,        0,      supercon, supercon, novag6502_state, empty_init,   "Novag", "Super Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1986, cfortea,   0,        0,      cforte,   cforte,   novag6502_state, empty_init,   "Novag", "Constellation Forte (version A)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1986, cforteb,   cfortea,  0,      cforte,   cforte,   novag6502_state, empty_init,   "Novag", "Constellation Forte (version B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1987, sfortea,   0,        0,      sforte,   sforte,   novag6502_state, init_sexpert, "Novag", "Super Forte (version A, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1987, sfortea1,  sfortea,  0,      sforte,   sforte,   novag6502_state, init_sexpert, "Novag", "Super Forte (version A, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1988, sforteb,   sfortea,  0,      sforte,   sforte,   novag6502_state, init_sexpert, "Novag", "Super Forte (version B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, sfortec,   sfortea,  0,      sforte,   sforte,   novag6502_state, init_sexpert, "Novag", "Super Forte (version C)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1987, sexperta,  0,        0,      sexpert,  sexpert,  novag6502_state, init_sexpert, "Novag", "Super Expert (version A)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1988, sexpertb,  sexperta, 0,      sexpert,  sexpert,  novag6502_state, init_sexpert, "Novag", "Super Expert (version B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, sexpertc,  sexperta, 0,      sexpert,  sexpert,  novag6502_state, init_sexpert, "Novag", "Super Expert (version C, V3.6)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, sexpertc1, sexperta, 0,      sexpert,  sexpert,  novag6502_state, init_sexpert, "Novag", "Super Expert (version C, V1.2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
