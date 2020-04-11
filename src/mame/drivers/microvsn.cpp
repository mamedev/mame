// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
// thanks-to:Kevin Horton, Sean Riddle
/***************************************************************************

Milton Bradley MicroVision, handheld game console

Hardware notes:
- SCUS0488(Hughes HLCD0488) LCD, 16*16 screen
- piezo, 12 buttons under membrane + analog paddle(MB calls it the Control Knob)
- no CPU on console, it is on the cartridge

12 games were released, all of them have a TMS1100 MCU. The first couple of
games had an I8021 MCU at first, but Milton Bradley switched to TMS1100.

Since the microcontrollers were on the cartridges it was possible to have
different clocks on different games.
The Connect Four I8021 game is clocked at around 2MHz. The TMS1100 versions
of the games were clocked at around 500KHz, 550KHz, or 350KHz.

Each game had a screen- and keypad overlay attached to it, MAME external
artwork is recommended.

TODO:
- Finish support for i8021 based cartridges

****************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/tms1000/tms1100.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

#include "emupal.h"
#include "softlist.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


class microvision_state : public driver_device
{
public:
	microvision_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_dac( *this, "dac" ),
		m_i8021( *this, "i8021_cpu" ),
		m_tms1100( *this, "tms1100_cpu" ),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "COL%u", 0),
		m_paddle(*this, "PADDLE"),
		m_conf(*this, "CONF"),
		m_overlay_out(*this, "overlay")
	{ }

	void microvision(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(conf_changed) { apply_settings(); }

protected:
	static constexpr device_timer_id TIMER_PADDLE = 0;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void microvision_palette(palette_device &palette) const;

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	// i8021 interface
	DECLARE_WRITE8_MEMBER(i8021_p0_write);
	DECLARE_WRITE8_MEMBER(i8021_p1_write);
	DECLARE_WRITE8_MEMBER(i8021_p2_write);
	DECLARE_READ_LINE_MEMBER(i8021_t1_read);
	DECLARE_READ8_MEMBER(i8021_bus_read);

	// TMS1100 interface
	DECLARE_READ8_MEMBER(tms1100_read_k);
	DECLARE_WRITE16_MEMBER(tms1100_write_o);
	DECLARE_WRITE16_MEMBER(tms1100_write_r);
	u32 tms1100_decode_micro(offs_t offset);

	required_device<dac_byte_interface> m_dac;
	optional_device<i8021_device> m_i8021;
	optional_device<tms1100_cpu_device> m_tms1100;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<3> m_inputs;
	required_ioport m_paddle;
	required_ioport m_conf;
	output_finder<> m_overlay_out;

	// Timers
	emu_timer *m_paddle_timer;

	// i8021 variables
	uint8_t   m_p0;
	uint8_t   m_p2;
	uint8_t   m_t1;

	// tms1100 variables
	uint16_t  m_r;
	uint16_t  m_o;

	// generic variables
	void    update_lcd();
	void    lcd_write(uint8_t control, uint8_t data);

	void apply_settings(void);

	u8 m_pla_auto;
	u8 m_overlay_auto;
	u16 m_button_mask;
	bool m_paddle_auto;
	bool m_paddle_on;

	uint8_t   m_lcd_latch[8];
	uint8_t   m_lcd_holding_latch[8];
	uint8_t   m_lcd_latch_index;
	uint8_t   m_lcd[16][16];
	uint8_t   m_lcd_control_old;
};


void microvision_state::microvision_palette(palette_device &palette) const
{
	palette.set_pen_color( 15, 0x00, 0x00, 0x00 );
	palette.set_pen_color( 14, 0x11, 0x11, 0x11 );
	palette.set_pen_color( 13, 0x22, 0x22, 0x22 );
	palette.set_pen_color( 12, 0x33, 0x33, 0x33 );
	palette.set_pen_color( 11, 0x44, 0x44, 0x44 );
	palette.set_pen_color( 10, 0x55, 0x55, 0x55 );
	palette.set_pen_color(  9, 0x66, 0x66, 0x66 );
	palette.set_pen_color(  8, 0x77, 0x77, 0x77 );
	palette.set_pen_color(  7, 0x88, 0x88, 0x88 );
	palette.set_pen_color(  6, 0x99, 0x99, 0x99 );
	palette.set_pen_color(  5, 0xaa, 0xaa, 0xaa );
	palette.set_pen_color(  4, 0xbb, 0xbb, 0xbb );
	palette.set_pen_color(  3, 0xcc, 0xcc, 0xcc );
	palette.set_pen_color(  2, 0xdd, 0xdd, 0xdd );
	palette.set_pen_color(  1, 0xee, 0xee, 0xee );
	palette.set_pen_color(  0, 0xff, 0xff, 0xff );
}


void microvision_state::machine_start()
{
	m_paddle_timer = timer_alloc(TIMER_PADDLE);
	m_overlay_out.resolve();

	save_item(NAME(m_p0));
	save_item(NAME(m_p2));
	save_item(NAME(m_t1));
	save_item(NAME(m_r));
	save_item(NAME(m_o));
	save_item(NAME(m_lcd_latch));
	save_item(NAME(m_lcd_latch_index));
	save_item(NAME(m_lcd));
	save_item(NAME(m_lcd_control_old));
	save_item(NAME(m_lcd_holding_latch));
}


void microvision_state::machine_reset()
{
	apply_settings();
	std::fill(std::begin(m_lcd_latch), std::end(m_lcd_latch), 0);

	for (auto &elem : m_lcd)
		std::fill(std::begin(elem), std::end(elem), 0);

	m_o = 0;
	m_r = 0;
	m_p0 = 0;
	m_p2 = 0;
	m_t1 = 0;

	m_paddle_timer->adjust(attotime::never);
}


void microvision_state::update_lcd()
{
	uint16_t row = ( m_lcd_holding_latch[0] << 12 ) | ( m_lcd_holding_latch[1] << 8 ) | ( m_lcd_holding_latch[2] << 4 ) | m_lcd_holding_latch[3];
	uint16_t col = ( m_lcd_holding_latch[4] << 12 ) | ( m_lcd_holding_latch[5] << 8 ) | ( m_lcd_holding_latch[6] << 4 ) | m_lcd_holding_latch[7];

	LOG( "row = %04x, col = %04x\n", row, col );
	for ( int i = 0; i < 16; i++ )
	{
		uint16_t temp = row;

		for (auto & elem : m_lcd)
		{
			if ( ( temp & col ) & 0x8000 )
			{
				elem[i] = 15;
			}
			temp <<= 1;
		}
		col <<= 1;
	}
}


uint32_t microvision_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for ( uint8_t i = 0; i < 16; i++ )
	{
		for ( uint8_t j = 0; j < 16; j++ )
		{
			bitmap.pix16(i,j) = m_lcd [i] [j];
		}
	}

	return 0;
}


WRITE_LINE_MEMBER(microvision_state::screen_vblank)
{
	if ( state )
	{
		for (auto & elem : m_lcd)
		{
			for ( int j= 0; j < 16; j++ )
			{
				if ( elem[j] )
				{
					elem[j]--;
				}
			}
		}
		update_lcd();
	}
}


/*
control is signals LCD5 LCD4
  LCD5 = -Data Clk on 0488
  LCD4 = Latch pulse on 0488
  LCD3 = Data 0
  LCD2 = Data 1
  LCD1 = Data 2
  LCD0 = Data 3
data is signals LCD3 LCD2 LCD1 LCD0
*/
void microvision_state::lcd_write(uint8_t control, uint8_t data)
{
	// Latch pulse, when high, resets the %8 latch address counter
	if ( control & 0x01 ) {
		m_lcd_latch_index = 0;
	}

	// The addressed latches load when -Data Clk is low
	if ( ! ( control & 0x02 ) ) {
		m_lcd_latch[ m_lcd_latch_index & 0x07 ] = data & 0x0f;
	}

	// The latch address counter is incremented on rising edges of -Data Clk
	if ( ( ! ( m_lcd_control_old & 0x02 ) ) && ( control & 0x02 ) ) {
		// Check if Latch pule is low
		if ( ! ( control & 0x01 ) ) {
			m_lcd_latch_index++;
		}
	}

	// A parallel transfer of data from the addressed latches to the holding latches occurs
	// whenever Latch Pulse is high and -Data Clk is high
	if ( control == 3 ) {
		for ( int i = 0; i < 8; i++ ) {
			m_lcd_holding_latch[i] = m_lcd_latch[i];
		}
		update_lcd();
	}

	m_lcd_control_old = control;
}


void microvision_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch ( id )
	{
		case TIMER_PADDLE:
			m_t1 = 0;
			break;
	}
}


/*
 x--- ---- KEY3
 -x-- ---- KEY4
 --x- ---- KEY5
 ---x ---- KEY6
 ---- x---
 ---- -x-- KEY0
 ---- --x- KEY1
 ---- ---x KEY2
*/
WRITE8_MEMBER( microvision_state::i8021_p0_write )
{
	LOG( "p0_write: %02x\n", data );

	m_p0 = data;
}


/*
 x--- ---- LCD3
 -x-- ---- LCD2
 --x- ---- LCD1
 ---x ---- LCD0
 ---- --x- LCD5
 ---- ---x LCD4
*/
WRITE8_MEMBER( microvision_state::i8021_p1_write )
{
	LOG( "p1_write: %02x\n", data );

	lcd_write( data & 0x03, data >> 4 );
}


/*
---- xx-- CAP2 (paddle)
---- --x- SPKR1
---- ---x SPKR0
*/
WRITE8_MEMBER( microvision_state::i8021_p2_write )
{
	LOG( "p2_write: %02x\n", data );

	m_p2 = data;

	m_dac->write(m_p2 & 0x03);

	if ( m_p2 & 0x0c )
	{
		m_t1 = 1;
		// Stop paddle timer
		m_paddle_timer->adjust( attotime::never );
	}
	else
	{
		// Start paddle timer (min is 160uS, max is 678uS)
		uint8_t paddle = 255 - ioport("PADDLE")->read();
		m_paddle_timer->adjust( attotime::from_usec(160 + ( 518 * paddle ) / 255 ) );
	}
}


READ_LINE_MEMBER( microvision_state::i8021_t1_read )
{
	return m_t1;
}


READ8_MEMBER( microvision_state::i8021_bus_read )
{
	uint8_t data = m_p0;

	uint8_t col0 = ioport("COL0")->read();
	uint8_t col1 = ioport("COL1")->read();
	uint8_t col2 = ioport("COL2")->read();

	// Row scanning
	if ( ! ( m_p0 & 0x80 ) )
	{
		uint8_t t = ( ( col0 & 0x01 ) << 2 ) | ( ( col1 & 0x01 ) << 1 ) | ( col2 & 0x01 );

		data &= ( t ^ 0xFF );
	}
	if ( ! ( m_p0 & 0x40 ) )
	{
		uint8_t t = ( ( col0 & 0x02 ) << 1 ) | ( col1 & 0x02 ) | ( ( col2 & 0x02 ) >> 1 );

		data &= ( t ^ 0xFF );
	}
	if ( ! ( m_p0 & 0x20 ) )
	{
		uint8_t t = ( col0 & 0x04 ) | ( ( col1 & 0x04 ) >> 1 ) | ( ( col2 & 0x04 ) >> 2 );

		data &= ( t ^ 0xFF );
	}
	if ( ! ( m_p0 & 0x10 ) )
	{
		uint8_t t = ( ( col0 & 0x08 ) >> 1 ) | ( ( col1 & 0x08 ) >> 2 ) | ( ( col2 & 0x08 ) >> 3 );

		data &= ( t ^ 0xFF );
	}
	return data;
}


READ8_MEMBER( microvision_state::tms1100_read_k )
{
	uint8_t data = 0;

	LOG("read_k\n");

	// multiplexed inputs
	for (int i = 0; i < 3; i++)
		if (BIT(m_r, i + 8))
			data |= m_inputs[i]->read() & (m_button_mask >> (i * 4) & 0xf);

	// K8: paddle capacitor
	if (m_paddle_on)
	{
		u8 paddle = m_paddle_timer->enabled() ? 0 : BIT(m_r, 2);
		return paddle << 3 | data;
	}
	else
		return data;
}


WRITE16_MEMBER( microvision_state::tms1100_write_o )
{
	LOG("write_o: %04x\n", data);

	// O0-O3: LCD data
	m_o = data;

	lcd_write( ( m_r >> 6 ) & 0x03, m_o & 0x0f );
}



WRITE16_MEMBER( microvision_state::tms1100_write_r )
{
	LOG("write_r: %04x\n", data);

	// R2: charge paddle capacitor
	if (~m_r & data & 4 && m_paddle_on)
	{
		// range is ~360us to ~2663us (measured on 4952-79 REV B PCB)
		// note that the games don't use the whole range, so there's a deadzone around the edges
		float step = (2663 - 360) / 255.0;
		m_paddle_timer->adjust(attotime::from_usec(360 + m_paddle->read() * step));
	}

	// R0: speaker lead 2
	// R1: speaker lead 1 (GND on some carts)
	m_dac->write((BIT(data, 0) << 1) | BIT(data, 1));

	// R6: LCD latch pulse
	// R7: LCD data clock
	lcd_write((data >> 6) & 0x03, m_o & 0x0f);

	// R8-R10: input mux
	m_r = data;
}


static const u16 microvision_output_pla[2][0x20] =
{
	// default TMS1100 O output PLA
	// verified for: blckbstr, pinball
	{
		0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
		0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f,
		0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
		0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
	},

	// reversed bit order
	// verified for: bowling, vegasslt
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	}
};

u32 microvision_state::tms1100_decode_micro(offs_t offset)
{
	// default TMS1100 microinstructions PLA - this should work for all games
	// verified for: blckbstr, bowling, pinball, vegasslt
	static const u16 micro[0x80] =
	{
		0x1402, 0x0c30, 0xd002, 0x2404, 0x8019, 0x8038, 0x0416, 0x0415,
		0x0104, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1100, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x000a, 0x0404, 0x0408, 0x8004, 0xa019, 0xa038, 0x2004, 0x2000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x1580, 0x1580, 0x1580, 0x1580, 0x0c34, 0x0834, 0x0434, 0x1400,
		0x0108, 0x0108, 0x0108, 0x0108, 0x0108, 0x0108, 0x0108, 0x0108,
		0x0108, 0x0108, 0x0108, 0x0108, 0x0108, 0x0108, 0x0108, 0x0108,
		0x9080, 0x9080, 0x9080, 0x9080, 0x9080, 0x9080, 0x9080, 0x9080,
		0x9080, 0x9080, 0x9080, 0x9080, 0x9080, 0x9080, 0x9080, 0x9080,
		0x8068, 0x8068, 0x8068, 0x8068, 0x8068, 0x8068, 0x8068, 0x8068,
		0x8068, 0x8068, 0x8068, 0x8068, 0x8068, 0x8068, 0x8068, 0x8068,
		0x0136, 0x0136, 0x0136, 0x0136, 0x0136, 0x0136, 0x0136, 0x0136,
		0x0136, 0x0136, 0x0136, 0x0136, 0x0136, 0x0136, 0x0136, 0x0134
	};

	if (offset >= 0x80 || micro[offset] == 0)
		return 0x8fa3;
	else
		return micro[offset];
}

DEVICE_IMAGE_LOAD_MEMBER(microvision_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");

	if (size != 0x400 && size != 0x800)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid ROM file size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	// set default settings
	u32 clock = (size == 0x400) ? 2000000 : 500000;
	m_pla_auto = 0;
	m_overlay_auto = 0;
	m_paddle_auto = false;

	if (image.loaded_through_softlist())
	{
		u32 sclock = strtoul(image.get_feature("clock"), nullptr, 0);
		if (sclock != 0)
			clock = sclock;

		m_overlay_auto = strtoul(image.get_feature("overlay"), nullptr, 0);
		m_pla_auto = strtoul(image.get_feature("pla"), nullptr, 0) ? 1 : 0;
		m_paddle_auto = bool(strtoul(image.get_feature("paddle"), nullptr, 0) ? 1 : 0);
	}

	// detect MCU on file size
	if (size == 0x400)
	{
		// I8021 MCU
		memcpy(memregion("i8021_cpu")->base(), m_cart->get_rom_base(), size);
		m_i8021->set_clock(clock);
	}
	else
	{
		// TMS1100 MCU
		memcpy(memregion("tms1100_cpu")->base(), m_cart->get_rom_base(), size);
		m_tms1100->set_clock(clock);
	}

	return image_init_result::PASS;
}

void microvision_state::apply_settings()
{
	u8 conf = m_conf->read();

	u8 overlay = (conf & 1) ? m_overlay_auto : 0;
	m_overlay_out = overlay;

	// overlay physically restricts button panel
	switch (overlay)
	{
		default:
			m_button_mask = 0xfff;
	}

	u8 pla = ((conf & 0x18) == 0x10) ? m_pla_auto : (conf >> 3 & 1);
	m_tms1100->set_output_pla(microvision_output_pla[pla]);

	m_paddle_on = ((conf & 6) == 4) ? m_paddle_auto : bool(conf & 2);
}


static INPUT_PORTS_START( microvision )
	PORT_START("COL0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_3)

	PORT_START("COL1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_2)

	PORT_START("COL2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_1)

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0)

	PORT_START("CONF")
	PORT_CONFNAME( 0x01, 0x01, "Overlay" ) PORT_CHANGED_MEMBER(DEVICE_SELF, microvision_state, conf_changed, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "Auto" )
	PORT_CONFNAME( 0x06, 0x04, "Paddle Hardware" ) PORT_CHANGED_MEMBER(DEVICE_SELF, microvision_state, conf_changed, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( No ) ) // no circuitry on cartridge PCB
	PORT_CONFSETTING(    0x02, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x04, "Auto" )
	PORT_CONFNAME( 0x18, 0x10, "TMS1100 PLA" ) PORT_CHANGED_MEMBER(DEVICE_SELF, microvision_state, conf_changed, 0)
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x10, "Auto" )
INPUT_PORTS_END


void microvision_state::microvision(machine_config &config)
{
	/* basic machine hardware */
	I8021(config, m_i8021, 0);
	m_i8021->bus_out_cb().set(FUNC(microvision_state::i8021_p0_write));
	m_i8021->p1_out_cb().set(FUNC(microvision_state::i8021_p1_write));
	m_i8021->p2_out_cb().set(FUNC(microvision_state::i8021_p2_write));
	m_i8021->t1_in_cb().set(FUNC(microvision_state::i8021_t1_read));
	m_i8021->bus_in_cb().set(FUNC(microvision_state::i8021_bus_read));

	TMS1100(config, m_tms1100, 0);
	m_tms1100->set_output_pla(microvision_output_pla[0]);
	m_tms1100->set_decode_micro().set(FUNC(microvision_state::tms1100_decode_micro));
	m_tms1100->k().set(FUNC(microvision_state::tms1100_read_k));
	m_tms1100->o().set(FUNC(microvision_state::tms1100_write_o));
	m_tms1100->r().set(FUNC(microvision_state::tms1100_write_r));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(0);
	screen.set_screen_update(FUNC(microvision_state::screen_update));
	screen.screen_vblank().set(FUNC(microvision_state::screen_vblank));
	screen.set_size(16, 16);
	screen.set_visarea(0, 15, 0, 15);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(microvision_state::microvision_palette), 16);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "microvision_cart");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(microvision_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("microvision");
}


ROM_START( microvsn )
	// nothing here yet, ROM is on the cartridge
	ROM_REGION( 0x400, "i8021_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x800, "tms1100_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 867, "tms1100_cpu:mpla", ROMREGION_ERASE00 )
	ROM_REGION( 365, "tms1100_cpu:opla", ROMREGION_ERASE00 )
ROM_END


CONS( 1979, microvsn, 0, 0, microvision, microvision, microvision_state, empty_init, "Milton Bradley", "MicroVision", MACHINE_NOT_WORKING | MACHINE_REQUIRES_ARTWORK )
