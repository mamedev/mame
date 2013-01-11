/***************************************************************************

    Milton Bradley MicroVision

    To Do:
    * Add support for the paddle control
    * Finish support for i8021 based cartridges

Since the microcontrollers were on the cartridges it was possible to have
different clocks on different games.
The Connect Four I8021 game is clocked at around 2MHz. The TMS1100 versions
of the games were clocked at around 500KHz, 550KHz, or 300KHz.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/dac.h"
#include "imagedev/cartslot.h"
#include "rendlay.h"


#define LOG 0

enum cpu_type
{
	CPU_TYPE_I8021,
	CPU_TYPE_TMS1100
};


class microvision_state : public driver_device
{
public:
	microvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_dac( *this, "dac" )
		, m_i8021( *this, "maincpu1" )
		, m_tms1100( *this, "maincpu2" )
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_PALETTE_INIT(microvision);
	DECLARE_MACHINE_START(microvision);
	DECLARE_MACHINE_RESET(microvision);

	void screen_vblank(screen_device &screen, bool state);

	// i8021 interface
	DECLARE_WRITE8_MEMBER(i8021_p0_write);
	DECLARE_WRITE8_MEMBER(i8021_p1_write);
	DECLARE_WRITE8_MEMBER(i8021_p2_write);
	DECLARE_READ8_MEMBER(i8021_t1_read);
	DECLARE_READ8_MEMBER(i8021_bus_read);

	// TMS1100 interface
	DECLARE_READ8_MEMBER(tms1100_read_k);
	DECLARE_WRITE16_MEMBER(tms1100_write_o);
	DECLARE_WRITE16_MEMBER(tms1100_write_r);

	cpu_type    m_cpu_type;

protected:
	required_device<dac_device> m_dac;
	required_device<cpu_device> m_i8021;
	required_device<cpu_device> m_tms1100;

	// Timers
	static const device_timer_id TIMER_PADDLE = 0;
	emu_timer *m_paddle_timer;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// i8021 variables
	UINT8   m_p0;
	UINT8   m_p2;
	UINT8   m_t1;

	// tms1100 variables
	UINT16  m_r;
	UINT16  m_o;

	// generic variables
	void    lcd_write(UINT8 control, UINT8 data);
	void    speaker_write(UINT8 speaker);

	UINT8   m_lcd_latch[8];
	UINT8   m_lcd_latch_index;
	UINT8   m_lcd[16][16];
	UINT8   m_lcd_control_old;
};


PALETTE_INIT_MEMBER(microvision_state,microvision)
{
	palette_set_color_rgb( machine(), 15, 0x00, 0x00, 0x00 );
	palette_set_color_rgb( machine(), 14, 0x11, 0x11, 0x11 );
	palette_set_color_rgb( machine(), 13, 0x22, 0x22, 0x22 );
	palette_set_color_rgb( machine(), 12, 0x33, 0x33, 0x33 );
	palette_set_color_rgb( machine(), 11, 0x44, 0x44, 0x44 );
	palette_set_color_rgb( machine(), 10, 0x55, 0x55, 0x55 );
	palette_set_color_rgb( machine(),  9, 0x66, 0x66, 0x66 );
	palette_set_color_rgb( machine(),  8, 0x77, 0x77, 0x77 );
	palette_set_color_rgb( machine(),  7, 0x88, 0x88, 0x88 );
	palette_set_color_rgb( machine(),  6, 0x99, 0x99, 0x99 );
	palette_set_color_rgb( machine(),  5, 0xaa, 0xaa, 0xaa );
	palette_set_color_rgb( machine(),  4, 0xbb, 0xbb, 0xbb );
	palette_set_color_rgb( machine(),  3, 0xcc, 0xcc, 0xcc );
	palette_set_color_rgb( machine(),  2, 0xdd, 0xdd, 0xdd );
	palette_set_color_rgb( machine(),  1, 0xee, 0xee, 0xee );
	palette_set_color_rgb( machine(),  0, 0xff, 0xff, 0xff );
}


MACHINE_START_MEMBER(microvision_state, microvision)
{
	m_paddle_timer = timer_alloc(TIMER_PADDLE);

	save_item(NAME(m_p0));
	save_item(NAME(m_p2));
	save_item(NAME(m_t1));
	save_item(NAME(m_r));
	save_item(NAME(m_o));
	save_item(NAME(m_lcd_latch));
	save_item(NAME(m_lcd_latch_index));
	save_item(NAME(m_lcd));
	save_item(NAME(m_lcd_control_old));
}


MACHINE_RESET_MEMBER(microvision_state, microvision)
{
	for( int i = 0; i < 8; i++ )
	{
		m_lcd_latch[i] = 0;
	}

	for( int i = 0; i < 16; i++ )
	{
		for ( int j = 0; j < 16; j++ )
		{
			m_lcd[i][j] = 0;
		}
	}

	m_o = 0;
	m_r = 0;
	m_p0 = 0;
	m_p2 = 0;
	m_t1 = 0;

	m_paddle_timer->adjust( attotime::never );

	switch ( m_cpu_type )
	{
		case CPU_TYPE_I8021:
			m_i8021->resume( SUSPEND_REASON_DISABLE );
			m_tms1100->suspend( SUSPEND_REASON_DISABLE, 0 );
			break;

		case CPU_TYPE_TMS1100:
			m_i8021->suspend( SUSPEND_REASON_DISABLE, 0 );
			m_tms1100->resume( SUSPEND_REASON_DISABLE );
			break;
	}
}


UINT32 microvision_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for ( int i = 0; i < 16; i++ )
	{
		for ( int j = 0; j < 16; j++ )
		{
			bitmap.pix16(i,j) = m_lcd[i][j];
		}
	}

	return 0;
}


void microvision_state::screen_vblank(screen_device &screen, bool state)
{
	if ( state )
	{
		for ( int i = 0; i < 16; i++ )
		{
			for ( int j= 0; j < 16; j++ )
			{
				if ( m_lcd[i][j] )
				{
					m_lcd[i][j]--;
				}
			}
		}
	}
}


/*
control is signals LCD5 LCD4
data is signals LCD3 LCD2 LCD1 LCD0
*/
void microvision_state::lcd_write(UINT8 control, UINT8 data)
{
	data &= 0xf;
	if ( ( control == 2 ) && ( m_lcd_control_old == 0 ) )
	{
		m_lcd_latch[ m_lcd_latch_index & 0x07 ] = data;
		m_lcd_latch_index++;
	}
	else if ( ( control == 3 ) && ( m_lcd_control_old == 2 ) )
	{
		m_lcd_latch_index = 0;

		UINT16 row = ( m_lcd_latch[0] << 12 ) | ( m_lcd_latch[1] << 8 ) | ( m_lcd_latch[2] << 4 ) | m_lcd_latch[3];
		UINT16 col = ( m_lcd_latch[4] << 12 ) | ( m_lcd_latch[5] << 8 ) | ( m_lcd_latch[6] << 4 ) | m_lcd_latch[7];

		if (LOG) logerror("row = %04x, col = %04x\n", row, col );
		for ( int i = 0; i < 16; i++ )
		{
			UINT16 temp = row;

			for ( int j = 0; j < 16; j++ )
			{
				if ( ( temp & col ) & 0x8000 )
				{
					m_lcd[j][i] = 15;
				}
				temp <<= 1;
			}
			col <<= 1;
		}
	}
	m_lcd_control_old = control;
}


/*
speaker is SPKR1 SPKR0
*/
void microvision_state::speaker_write(UINT8 speaker)
{
	const INT8 speaker_level[4] = { 0, 127, -128, 0 };

	m_dac->write_signed8( speaker_level[ speaker & 0x03 ] );
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
	if (LOG) logerror( "p0_write: %02x\n", data );

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
	if (LOG) logerror( "p1_write: %02x\n", data );

	lcd_write( data & 0x03, data >> 4 );
}


/*
---- xx-- CAP2 (paddle)
---- --x- SPKR1
---- ---x SPKR0
*/
WRITE8_MEMBER( microvision_state::i8021_p2_write )
{
	if (LOG) logerror( "p2_write: %02x\n", data );

	m_p2 = data;

	speaker_write( m_p2 & 0x03 );

	if ( m_p2 & 0x0c )
	{
		m_t1 = 1;
		// Stop paddle timer
		m_paddle_timer->adjust( attotime::never );
	}
	else
	{
		// Start paddle timer (min is 160uS, max is 678uS)
		UINT8 paddle = 255 - ioport("PADDLE")->read();
		m_paddle_timer->adjust( attotime::from_usec(160 + ( 518 * paddle ) / 255 ) );
	}
}


READ8_MEMBER( microvision_state::i8021_t1_read )
{
	return m_t1;
}


READ8_MEMBER( microvision_state::i8021_bus_read )
{
	UINT8 data = m_p0;

	UINT8 col0 = ioport("COL0")->read();
	UINT8 col1 = ioport("COL1")->read();
	UINT8 col2 = ioport("COL2")->read();

	// Row scanning
	if ( ! ( m_p0 & 0x80 ) )
	{
		UINT8 t = ( ( col0 & 0x01 ) << 2 ) | ( ( col1 & 0x01 ) << 1 ) | ( col2 & 0x01 );

		data &= ( t ^ 0xFF );
	}
	if ( ! ( m_p0 & 0x40 ) )
	{
		UINT8 t = ( ( col0 & 0x02 ) << 1 ) | ( col1 & 0x02 ) | ( ( col2 & 0x02 ) >> 1 );

		data &= ( t ^ 0xFF );
	}
	if ( ! ( m_p0 & 0x20 ) )
	{
		UINT8 t = ( col0 & 0x04 ) | ( ( col1 & 0x04 ) >> 1 ) | ( ( col2 & 0x04 ) >> 2 );

		data &= ( t ^ 0xFF );
	}
	if ( ! ( m_p0 & 0x10 ) )
	{
		UINT8 t = ( ( col0 & 0x08 ) >> 1 ) | ( ( col1 & 0x08 ) >> 2 ) | ( ( col2 & 0x08 ) >> 3 );

		data &= ( t ^ 0xFF );
	}
	return data;
}


READ8_MEMBER( microvision_state::tms1100_read_k )
{
	UINT8 data = 0;

	if (LOG) logerror("read_k\n");

	if ( m_r & 0x100 )
	{
		data |= ioport("COL0")->read();
	}
	if ( m_r & 0x200 )
	{
		data |= ioport("COL1")->read();
	}
	if ( m_r & 0x400 )
	{
		data |= ioport("COL2")->read();
	}
	return data;
}


WRITE16_MEMBER( microvision_state::tms1100_write_o )
{
	if (LOG) logerror("write_o: %04x\n", data);

	m_o = data;

	lcd_write( ( m_r >> 6 ) & 0x03, m_o & 0x0f );
}


/*
x-- ---- ---- KEY2
-x- ---- ---- KEY1
--x ---- ---- KEY0
--- x--- ---- LCD5
--- -x-- ---- LCD4
--- ---- --x- SPKR0
--- ---- ---x SPKR1
*/
WRITE16_MEMBER( microvision_state::tms1100_write_r )
{
	if (LOG) logerror("write_r: %04x\n", data);

	m_r = data;

	speaker_write( ( ( m_r & 0x01 ) << 1 ) | ( ( m_r & 0x02 ) >> 1 ) );
	lcd_write( ( m_r >> 6 ) & 0x03, m_o & 0x0f );
}


static DEVICE_IMAGE_LOAD(microvision_cart)
{
	microvision_state *state = image.device().machine().driver_data<microvision_state>();
	UINT8 *rom1 = state->memregion("maincpu1")->base();
	UINT8 *rom2 = state->memregion("maincpu2")->base();
	UINT32 file_size;

	if (image.software_entry() == NULL)
	{
		file_size = image.length();
	}
	else
	{
		file_size = image.get_software_region_length("rom");
	}

	if ( file_size != 1024 && file_size != 2048 )
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid rom file size");
		return IMAGE_INIT_FAIL;
	}

	/* Read cartridge */
	if (image.software_entry() == NULL)
	{
		if (image.fread( rom1, file_size) != file_size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file");
			return IMAGE_INIT_FAIL;
		}
	}
	else
	{
		memcpy(rom1, image.get_software_region("rom"), file_size);
	}
	memcpy( rom2, rom1, file_size );

	// Based on file size select cpu:
	// - 1024 -> I8021
	// - 2048 -> TI TMS1100

	switch ( file_size )
	{
		case 1024:
			state->m_cpu_type = CPU_TYPE_I8021;
			break;

		case 2048:
			state->m_cpu_type = CPU_TYPE_TMS1100;
			break;
	}
	return IMAGE_INIT_PASS;
}


static INPUT_PORTS_START( microvision )
	PORT_START("COL0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_CODE(KEYCODE_3) PORT_NAME("B01")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_CODE(KEYCODE_E) PORT_NAME("B04")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON7)  PORT_CODE(KEYCODE_D) PORT_NAME("B07")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_CODE(KEYCODE_C) PORT_NAME("B10")

	PORT_START("COL1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_CODE(KEYCODE_4) PORT_NAME("B02")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON5)  PORT_CODE(KEYCODE_R) PORT_NAME("B05")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON8)  PORT_CODE(KEYCODE_F) PORT_NAME("B08")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON11) PORT_CODE(KEYCODE_V) PORT_NAME("B11")

	PORT_START("COL2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_CODE(KEYCODE_5) PORT_NAME("B03")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6)  PORT_CODE(KEYCODE_T) PORT_NAME("B06")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON9)  PORT_CODE(KEYCODE_G) PORT_NAME("B09")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON12) PORT_CODE(KEYCODE_B) PORT_NAME("B12")

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(1) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255)
INPUT_PORTS_END


static ADDRESS_MAP_START( microvision_8021_io, AS_IO, 8, microvision_state )
	AM_RANGE( 0x00, 0xFF ) AM_WRITE( i8021_p0_write )
	AM_RANGE( MCS48_PORT_P0, MCS48_PORT_P0 ) AM_WRITE( i8021_p0_write )
	AM_RANGE( MCS48_PORT_P1, MCS48_PORT_P1 ) AM_WRITE( i8021_p1_write )
	AM_RANGE( MCS48_PORT_P2, MCS48_PORT_P2 ) AM_WRITE( i8021_p2_write )
	AM_RANGE( MCS48_PORT_T1, MCS48_PORT_T1 ) AM_READ( i8021_t1_read )
	AM_RANGE( MCS48_PORT_BUS, MCS48_PORT_BUS ) AM_READ( i8021_bus_read )
ADDRESS_MAP_END


static const tms0980_config microvision_tms0980_config =
{
	{
		/* O output PLA configuration currently unknown */
		0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E,
		0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F,
		0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00,
		0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00, 0xFF00
	},
	DEVCB_DRIVER_MEMBER(microvision_state, tms1100_read_k),
	DEVCB_DRIVER_MEMBER16(microvision_state, tms1100_write_o),
	DEVCB_DRIVER_MEMBER16(microvision_state, tms1100_write_r)
};


static MACHINE_CONFIG_START( microvision, microvision_state )
	MCFG_CPU_ADD("maincpu1", I8021, 2000000)    // approximately
	MCFG_CPU_IO_MAP( microvision_8021_io )
	MCFG_CPU_ADD("maincpu2", TMS1100, 500000)   // most games seem to be running at approximately this speed
	MCFG_CPU_CONFIG( microvision_tms0980_config )

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(microvision_state, microvision )
	MCFG_MACHINE_RESET_OVERRIDE(microvision_state, microvision )

	MCFG_SCREEN_UPDATE_DRIVER(microvision_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(microvision_state, screen_vblank)
	MCFG_SCREEN_SIZE(16, 16)
	MCFG_SCREEN_VISIBLE_AREA(0, 15, 0, 15)

	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT_OVERRIDE(microvision_state,microvision)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("microvision_cart")
	MCFG_CARTSLOT_LOAD(microvision_cart)
MACHINE_CONFIG_END


ROM_START( microvsn )
	ROM_REGION( 0x800, "maincpu1", ROMREGION_ERASE00 )
	ROM_REGION( 0x800, "maincpu2", ROMREGION_ERASE00 )
ROM_END


CONS( 1979, microvsn, 0, 0, microvision, microvision, driver_device, 0, "Milton Bradley", "MicroVision", GAME_NOT_WORKING )
