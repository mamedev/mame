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
    - move other Novag sets here when applicable

-------------------------------------------------------------------------------

Super Constellation Chess Computer (model 844):
- UMC UM6502C @ 4 MHz (8MHz XTAL), 600Hz IRQ(source unknown?)
- 2*2KB RAM TC5516APL-2 battery-backed, 2*32KB ROM custom label
- TTL, buzzer, 24 LEDs, 8*8 chessboard buttons
- external ports for clock and printer, not emulated here

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "sound/beep.h"
#include "video/hd44780.h"

// internal artwork
#include "supercon.lh" // clickable


class novag6502_state : public driver_device
{
public:
	novag6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper"),
		m_lcd(*this, "hd44780"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<beep_device> m_beeper;
	optional_device<hd44780_device> m_lcd;
	optional_ioport_array<8> m_inp_matrix;

	// misc common
	uint16_t m_inp_mux;                   // multiplexed keypad mask
	uint16_t m_led_select;
	uint16_t m_led_data;

	uint16_t read_inputs(int columns);

	// display common
	int m_display_wait;                   // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                   // display matrix number of rows
	int m_display_maxx;                   // display matrix number of columns (max 31 for now)

	uint32_t m_display_state[0x20];       // display matrix rows data (last bit is used for always-on)
	uint16_t m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	uint32_t m_display_cache[0x20];       // (internal use)
	uint8_t m_display_decay[0x20][0x20];  // (internal use)
	
	uint8_t m_lcd_control;

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(uint32_t digits, uint32_t mask);
	void display_matrix(int maxx, int maxy, uint32_t setx, uint32_t sety, bool update = true);

	// Super Constellation	
	DECLARE_WRITE8_MEMBER(supercon_1e_w);
	DECLARE_WRITE8_MEMBER(supercon_1f_w);
	DECLARE_READ8_MEMBER(supercon_1e_r);
	DECLARE_READ8_MEMBER(supercon_1f_r);

	// Super Expert
	DECLARE_WRITE8_MEMBER(sexpert_leds_w);
	DECLARE_WRITE8_MEMBER(sexpert_mux_w);
	DECLARE_WRITE8_MEMBER(sexpert_lcd_control_w);
	DECLARE_WRITE8_MEMBER(sexpert_lcd_data_w);
	DECLARE_READ8_MEMBER(sexpert_lcd_data_r);
	DECLARE_READ8_MEMBER(sexpert_input1_r);
	DECLARE_READ8_MEMBER(sexpert_input2_r);
	DECLARE_PALETTE_INIT(sexpert);
	HD44780_PIXEL_UPDATE(sexpert_pixel_update);
	DECLARE_MACHINE_RESET(sexpert);
	DECLARE_DRIVER_INIT(sexpert);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void novag6502_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_inp_mux = 0;
	m_led_select = 0;
	m_led_data = 0;
	m_lcd_control = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	/* save_item(NAME(m_display_cache)); */ // don't save!
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_data));
	save_item(NAME(m_lcd_control));
}

void novag6502_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void novag6502_state::display_update()
{
	uint32_t active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			uint32_t ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_display_segmask[y] != 0)
				output().set_digit_value(y, active_state[y] & m_display_segmask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x <= m_display_maxx; x++)
			{
				int state = active_state[y] >> x & 1;
				char buf1[0x10]; // lampyx
				char buf2[0x10]; // y.x

				if (x == m_display_maxx)
				{
					// always-on if selected
					sprintf(buf1, "lamp%da", y);
					sprintf(buf2, "%d.a", y);
				}
				else
				{
					sprintf(buf1, "lamp%d", y * mul + x);
					sprintf(buf2, "%d.%d", y, x);
				}
				output().set_value(buf1, state);
				output().set_value(buf2, state);
			}
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(novag6502_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void novag6502_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void novag6502_state::set_display_segmask(uint32_t digits, uint32_t mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void novag6502_state::display_matrix(int maxx, int maxy, uint32_t setx, uint32_t sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	uint32_t mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// generic input handlers

uint16_t novag6502_state::read_inputs(int columns)
{
	uint16_t ret = 0;

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

WRITE8_MEMBER(novag6502_state::supercon_1e_w)
{
	// d0-d7: input mux, led data
	m_inp_mux = m_led_data = data;
	display_matrix(8, 3, m_led_data, m_led_select);
}

WRITE8_MEMBER(novag6502_state::supercon_1f_w)
{
	// d0-d3: ?
	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	display_matrix(8, 3, m_led_data, m_led_select);

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}

READ8_MEMBER(novag6502_state::supercon_1e_r)
{
	// d0-d5: ?
	// d6,d7: multiplexed inputs (side panel)
	return (read_inputs(8) >> 2 & 0xc0) ^ 0xff;
}

READ8_MEMBER(novag6502_state::supercon_1f_r)
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~read_inputs(8) & 0xff;
}



/******************************************************************************
    Super Expert
******************************************************************************/

// LCD

PALETTE_INIT_MEMBER(novag6502_state, sexpert)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(novag6502_state::sexpert_pixel_update)
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

WRITE8_MEMBER(novag6502_state::sexpert_lcd_control_w)
{
	// d0: HD44780 RS
	// d1: HD44780 R/W
	// d2: HD44780 E
	m_lcd_control = data;
}

WRITE8_MEMBER(novag6502_state::sexpert_lcd_data_w)
{
	if (m_lcd_control & 4 && ~m_lcd_control & 2)
		m_lcd->write(space, m_lcd_control & 1, data);
}

READ8_MEMBER(novag6502_state::sexpert_lcd_data_r)
{
	// unused?
	return 0;
}

// TTL/generic

WRITE8_MEMBER(novag6502_state::sexpert_leds_w)
{
	// d0-d7: chessboard leds
	m_led_data = data;
}

WRITE8_MEMBER(novag6502_state::sexpert_mux_w)
{
	// d0: rom bankswitch
	membank("bank1")->set_entry(data & 1);

	// d3: enable beeper
	m_beeper->set_state(data >> 3 & 1);
	
	// d4-d7: 74145 to input mux/led select
	m_inp_mux = 1 << (data >> 4 & 0xf) & 0xff;
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
	return (read_inputs(8) >> 3 & 0xc0) ^ 0xff;
}

MACHINE_RESET_MEMBER(novag6502_state, sexpert)
{
	membank("bank1")->set_entry(0);
	novag6502_state::machine_reset();
}

DRIVER_INIT_MEMBER(novag6502_state, sexpert)
{
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x8000, 0x8000);
}



/******************************************************************************
    Address Maps
******************************************************************************/

// Super Constellation

static ADDRESS_MAP_START( supercon_mem, AS_PROGRAM, 8, novag6502_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x1c00, 0x1c00) AM_WRITENOP // printer/clock?
	AM_RANGE(0x1d00, 0x1d00) AM_WRITENOP // printer/clock?
	AM_RANGE(0x1e00, 0x1e00) AM_READWRITE(supercon_1e_r, supercon_1e_w)
	AM_RANGE(0x1f00, 0x1f00) AM_READWRITE(supercon_1f_r, supercon_1f_w)
	AM_RANGE(0x2000, 0xffff) AM_ROM
ADDRESS_MAP_END


// Super Expert

static ADDRESS_MAP_START( sexpert_mem, AS_PROGRAM, 8, novag6502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fef) AM_RAM // 8KB RAM, but RAM CE pin is deactivated on $1ff0-$1fff
	AM_RANGE(0x1ff0, 0x1ff0) AM_READ(sexpert_input1_r)
	AM_RANGE(0x1ff1, 0x1ff1) AM_READ(sexpert_input2_r)
	AM_RANGE(0x1ff2, 0x1ff2) AM_WRITENOP // printer port
	AM_RANGE(0x1ff3, 0x1ff3) AM_WRITENOP // printer port
	AM_RANGE(0x1ff4, 0x1ff4) AM_WRITE(sexpert_leds_w)
	AM_RANGE(0x1ff5, 0x1ff5) AM_WRITE(sexpert_mux_w)
	AM_RANGE(0x1ff6, 0x1ff6) AM_WRITE(sexpert_lcd_control_w)
	AM_RANGE(0x1ff7, 0x1ff7) AM_READWRITE(sexpert_lcd_data_r, sexpert_lcd_data_w)
	AM_RANGE(0x2000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( cb_buttons )
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

static INPUT_PORTS_START( cb_magnets )
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
	PORT_INCLUDE( cb_buttons )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("King / Multi Move / Player/Player")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Queen / Best Move/Random / Training Level")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Bishop / Sound / Depth Search")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Knight / Solve Mate")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Rook / Print Board")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Form Size")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Pion / Print List / Acc. Time")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back")
INPUT_PORTS_END


static INPUT_PORTS_START( sexpert )
	PORT_INCLUDE( cb_magnets )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( supercon, novag6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_8MHz/2)
	MCFG_CPU_PERIODIC_INT_DRIVER(novag6502_state, irq0_line_hold, 600) // guessed
	MCFG_CPU_PROGRAM_MAP(supercon_mem)

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", novag6502_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_supercon)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 1200) // guessed
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sexpert, novag6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65C02, XTAL_10MHz/2)
	MCFG_CPU_PERIODIC_INT_DRIVER(novag6502_state, irq0_line_hold, XTAL_32_768kHz/128)
	MCFG_CPU_PROGRAM_MAP(sexpert_mem)
	
	MCFG_MACHINE_RESET_OVERRIDE(novag6502_state, sexpert)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(6*16+1, 10)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16, 0, 10-1)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(novag6502_state, sexpert)


	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 8)
	MCFG_HD44780_PIXEL_UPDATE_CB(novag6502_state, sexpert_pixel_update)

	//MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", novag6502_state, display_decay_tick, attotime::from_msec(1))
	//MCFG_DEFAULT_LAYOUT(layout_sexpert)



	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, XTAL_32_768kHz/32)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( supercon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_8441", 0x0000, 0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0) ) // label obscured by Q.C. sticker
	ROM_LOAD("novag_8442", 0x8000, 0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe) )
ROM_END


ROM_START( sfortea )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfalo.bin", 0x0000, 0x8000, CRC(86e0230a) SHA1(0d6e18a17e636b8c7292c8f331349d361892d1a8) )
	ROM_LOAD("sfahi.bin", 0x8000, 0x8000, CRC(81c02746) SHA1(0bf68b68ade5a3263bead88da0a8965fc71483c1) )
	ROM_LOAD("sfabook.bin", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957) )
ROM_END

ROM_START( sforteb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("forte_b.lo", 0x0000, 0x8000, CRC(48bfe5d6) SHA1(323642686b6d2fb8db2b7d50c6cd431058078ce1) )
	ROM_LOAD("forte_b.hi1", 0x8000, 0x8000, CRC(9778ca2c) SHA1(d8b88b9768a1a9171c68cbb0892b817d68d78351) )
	ROM_LOAD("forte_b.hi0", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sforteba )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("forte b_l.bin", 0x0000, 0x8000, CRC(e3d194a1) SHA1(80457580d7c57e07895fd14bfdaf14b30952afca) )
	ROM_LOAD("forte b_h.bin", 0x8000, 0x8000, CRC(dd824be8) SHA1(cd8666b6b525887f9fc48a730b71ceabcf07f3b9) )
	ROM_LOAD("forte_b.hi0", 0x10000, 0x8000, BAD_DUMP CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END

ROM_START( sfortec )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("sfclow.bin", 0x0000, 0x8000, CRC(f040cf30) SHA1(1fc1220b8ed67cdffa3866d230ce001721cf684f) )
	ROM_LOAD("sfchi.bin", 0x8000, 0x8000, CRC(0f926b32) SHA1(9c7270ecb3f41dd9172a9a7928e6e04e64b2a340) )
	ROM_LOAD("sfcbook.bin", 0x10000, 0x8000, CRC(c6a1419a) SHA1(017a0ffa9aa59438c879624a7ddea2071d1524b8) )
ROM_END


ROM_START( sexpertc )
	ROM_REGION( 0x18000, "maincpu", 0 ) // Version 3.6 of firmware
	ROM_LOAD("seclow.bin", 0x0000, 0x8000, CRC(5a29105e) SHA1(be37bb29b530dbba847a5e8d27d81b36525e47f7) )
	ROM_LOAD("sechi.bin", 0x8000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa) )
	ROM_LOAD("secbook.bin", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af) )
ROM_END

ROM_START( sexpertb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD("seb69u3.bin", 0x0000, 0x8000, CRC(92002eb6) SHA1(ed8ca16701e00b48fa55c856fa4a8c6613079c02) )
	ROM_LOAD("seb69u1.bin", 0x8000, 0x8000, CRC(814b4420) SHA1(c553e6a8c048dcc1cf48d410111a86e06b99d356) )
	ROM_LOAD("seb605u2.bin", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6) )
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1984, supercon, 0,      0,      supercon, supercon, driver_device, 0, "Novag", "Super Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1987, sfortea,  0,       0,      sexpert,   sexpert, novag6502_state,  sexpert,       "Novag",                     "Novag Super Forte Chess Computer (version A)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1988, sforteb,  sfortea, 0,      sexpert,   sexpert, novag6502_state,  sexpert,       "Novag",                     "Novag Super Forte Chess Computer (version B)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1988, sforteba, sfortea, 0,      sexpert,   sexpert, novag6502_state,  sexpert,       "Novag",                     "Novag Super Forte Chess Computer (version B, alt)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1989, sfortec,  sfortea, 0,      sexpert,   sexpert, novag6502_state,  sexpert,       "Novag",                     "Novag Super Forte Chess Computer (version C)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

CONS( 1989, sexpertc, 0, 0,      sexpert,   sexpert, novag6502_state,  sexpert,       "Novag",                     "Novag Super Expert C Chess Computer", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1988, sexpertb, sexpertc, 0,      sexpert,   sexpert, novag6502_state,  sexpert,       "Novag",                     "Novag Super Expert B Chess Computer", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
