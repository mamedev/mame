// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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
#include "cpu/tms1000/tms1100.h"
#include "sound/dac.h"
#include "rendlay.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

#define LOG 0


class microvision_state : public driver_device
{
public:
	microvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_dac( *this, "dac" ),
		m_i8021( *this, "maincpu1" ),
		m_tms1100( *this, "maincpu2" ),
		m_cart(*this, "cartslot")
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_PALETTE_INIT(microvision);
	DECLARE_MACHINE_START(microvision);
	DECLARE_MACHINE_RESET(microvision);

	void screen_vblank(screen_device &screen, bool state);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( microvsn_cart );

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

	// enums
	enum cpu_type
	{
		CPU_TYPE_I8021,
		CPU_TYPE_TMS1100
	};

	enum pcb_type
	{
		PCB_TYPE_4952_REV_A,
		PCB_TYPE_4952_9_REV_B,
		PCB_TYPE_4971_REV_C,
		PCB_TYPE_7924952D02,
		PCB_TYPE_UNKNOWN
	};

	enum rc_type
	{
		RC_TYPE_100PF_21_0K,
		RC_TYPE_100PF_23_2K,
		RC_TYPE_100PF_39_4K,
		RC_TYPE_UNKNOWN
	};

	cpu_type    m_cpu_type;
	pcb_type    m_pcb_type;
	rc_type     m_rc_type;

protected:
	required_device<dac_device> m_dac;
	required_device<cpu_device> m_i8021;
	required_device<cpu_device> m_tms1100;
	required_device<generic_slot_device> m_cart;

	// Timers
	static const device_timer_id TIMER_PADDLE = 0;
	emu_timer *m_paddle_timer;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// i8021 variables
	UINT8   m_p0;
	UINT8   m_p2;
	UINT8   m_t1;

	// tms1100 variables
	UINT16  m_r;
	UINT16  m_o;

	// generic variables
	void    update_lcd();
	void    lcd_write(UINT8 control, UINT8 data);
	void    speaker_write(UINT8 speaker);
	bool    m_pla;

	UINT8   m_lcd_latch[8];
	UINT8   m_lcd_holding_latch[8];
	UINT8   m_lcd_latch_index;
	UINT8   m_lcd[16][16];
	UINT8   m_lcd_control_old;
};


PALETTE_INIT_MEMBER(microvision_state,microvision)
{
	palette.set_pen_color( 15, 0x00, 0x00, 0x00 );
	palette.set_pen_color( 14, 0x11, 0x11, 0x11 );
	palette.set_pen_color( 13, 0x22, 0x22, 0x22 );
	palette.set_pen_color( 12, 0x33, 0x33, 0x33 );
	palette.set_pen_color( 11, 0x44, 0x44, 0x44 );
	palette.set_pen_color( 10, 0x55, 0x55, 0x55 );
	palette.set_pen_color( 9, 0x66, 0x66, 0x66 );
	palette.set_pen_color( 8, 0x77, 0x77, 0x77 );
	palette.set_pen_color( 7, 0x88, 0x88, 0x88 );
	palette.set_pen_color( 6, 0x99, 0x99, 0x99 );
	palette.set_pen_color( 5, 0xaa, 0xaa, 0xaa );
	palette.set_pen_color( 4, 0xbb, 0xbb, 0xbb );
	palette.set_pen_color( 3, 0xcc, 0xcc, 0xcc );
	palette.set_pen_color( 2, 0xdd, 0xdd, 0xdd );
	palette.set_pen_color( 1, 0xee, 0xee, 0xee );
	palette.set_pen_color( 0, 0xff, 0xff, 0xff );
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
	save_item(NAME(m_pla));
	save_item(NAME(m_lcd_holding_latch));
}


MACHINE_RESET_MEMBER(microvision_state, microvision)
{
	for(auto & elem : m_lcd_latch)
	{
		elem = 0;
	}

	for(auto & elem : m_lcd)
	{
		for ( int j = 0; j < 16; j++ )
		{
			elem[j] = 0;
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

			switch ( m_rc_type )
			{
				case RC_TYPE_100PF_21_0K:
					static_set_clock( m_tms1100, 550000 );
					break;

				case RC_TYPE_100PF_23_2K:
				case RC_TYPE_UNKNOWN:   // Default to most occurring setting
					static_set_clock( m_tms1100, 500000 );
					break;

				case RC_TYPE_100PF_39_4K:
					static_set_clock( m_tms1100, 300000 );
					break;
			}
			break;
	}
}


void microvision_state::update_lcd()
{
	UINT16 row = ( m_lcd_holding_latch[0] << 12 ) | ( m_lcd_holding_latch[1] << 8 ) | ( m_lcd_holding_latch[2] << 4 ) | m_lcd_holding_latch[3];
	UINT16 col = ( m_lcd_holding_latch[4] << 12 ) | ( m_lcd_holding_latch[5] << 8 ) | ( m_lcd_holding_latch[6] << 4 ) | m_lcd_holding_latch[7];

	if (LOG) logerror("row = %04x, col = %04x\n", row, col );
	for ( int i = 0; i < 16; i++ )
	{
		UINT16 temp = row;

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


UINT32 microvision_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for ( UINT8 i = 0; i < 16; i++ )
	{
		for ( UINT8 j = 0; j < 16; j++ )
		{
			bitmap.pix16(i,j) = m_lcd [i] [j];
		}
	}

	return 0;
}


void microvision_state::screen_vblank(screen_device &screen, bool state)
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
void microvision_state::lcd_write(UINT8 control, UINT8 data)
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


static const UINT16 microvision_output_pla_0[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E,
	0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static const UINT16 microvision_output_pla_1[0x20] =
{
	/* O output PLA configuration currently unknown */
	/* Reversed bit order */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


DEVICE_IMAGE_LOAD_MEMBER(microvision_state, microvsn_cart)
{
	UINT8 *rom1 = memregion("maincpu1")->base();
	UINT8 *rom2 = memregion("maincpu2")->base();
	UINT32 file_size = m_cart->common_get_size("rom");
	m_pla = 0;

	if ( file_size != 1024 && file_size != 2048 )
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid rom file size");
		return IMAGE_INIT_FAIL;
	}

	/* Read cartridge */
	if (image.software_entry() == nullptr)
	{
		if (image.fread(rom1, file_size) != file_size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file");
			return IMAGE_INIT_FAIL;
		}
	}
	else
	{
		// Copy rom contents
		memcpy(rom1, image.get_software_region("rom"), file_size);

		// Get PLA type
		const char *pla = image.get_feature("pla");

		if (pla)
			m_pla = 1;

		tms1100_cpu_device::set_output_pla(m_tms1100, m_pla ? microvision_output_pla_1 : microvision_output_pla_0);

		// Set default setting for PCB type and RC type
		m_pcb_type = microvision_state::PCB_TYPE_UNKNOWN;
		m_rc_type = microvision_state::RC_TYPE_UNKNOWN;

		// Detect settings for PCB type
		const char *pcb = image.get_feature("pcb");

		if (pcb)
		{
			static const struct { const char *pcb_name; microvision_state::pcb_type pcbtype; } pcb_types[] =
				{
					{ "4952 REV-A", microvision_state::PCB_TYPE_4952_REV_A },
					{ "4952-79 REV-B", microvision_state::PCB_TYPE_4952_9_REV_B },
					{ "4971-REV-C", microvision_state::PCB_TYPE_4971_REV_C },
					{ "7924952D02", microvision_state::PCB_TYPE_7924952D02 }
				};

			for (int i = 0; i < ARRAY_LENGTH(pcb_types) && m_pcb_type == microvision_state::PCB_TYPE_UNKNOWN; i++)
			{
				if (!core_stricmp(pcb, pcb_types[i].pcb_name))
				{
					m_pcb_type = pcb_types[i].pcbtype;
				}
			}
		}

		// Detect settings for RC types
		const char *rc = image.get_feature("rc");

		if (rc)
		{
			static const struct { const char *rc_name; microvision_state::rc_type rctype; } rc_types[] =
				{
					{ "100pf/21.0K", microvision_state::RC_TYPE_100PF_21_0K },
					{ "100pf/23.2K", microvision_state::RC_TYPE_100PF_23_2K },
					{ "100pf/39.4K", microvision_state::RC_TYPE_100PF_39_4K }
				};

			for (int i = 0; i < ARRAY_LENGTH(rc_types) && m_rc_type == microvision_state::RC_TYPE_UNKNOWN; i++)
			{
				if (!core_stricmp(rc, rc_types[i].rc_name))
				{
					m_rc_type = rc_types[i].rctype;
				}
			}
		}
	}

	// Mirror rom data to maincpu2 region
	memcpy(rom2, rom1, file_size);

	// Based on file size select cpu:
	// - 1024 -> I8021
	// - 2048 -> TI TMS1100

	switch (file_size)
	{
		case 1024:
			m_cpu_type = microvision_state::CPU_TYPE_I8021;
			break;

		case 2048:
			m_cpu_type = microvision_state::CPU_TYPE_TMS1100;
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


static MACHINE_CONFIG_START( microvision, microvision_state )
	MCFG_CPU_ADD("maincpu1", I8021, 2000000)    // approximately
	MCFG_CPU_IO_MAP( microvision_8021_io )
	MCFG_CPU_ADD("maincpu2", TMS1100, 500000)   // most games seem to be running at approximately this speed
	MCFG_TMS1XXX_OUTPUT_PLA( microvision_output_pla_0 )
	MCFG_TMS1XXX_READ_K_CB( READ8( microvision_state, tms1100_read_k ) )
	MCFG_TMS1XXX_WRITE_O_CB( WRITE16( microvision_state, tms1100_write_o ) )
	MCFG_TMS1XXX_WRITE_R_CB( WRITE16( microvision_state, tms1100_write_r ) )

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
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(microvision_state,microvision)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "microvision_cart")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(microvision_state, microvsn_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","microvision")
MACHINE_CONFIG_END


ROM_START( microvsn )
	ROM_REGION( 0x800, "maincpu1", ROMREGION_ERASE00 )
	ROM_REGION( 0x800, "maincpu2", ROMREGION_ERASE00 )
	ROM_REGION( 867, "maincpu2:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // verified for: pinball, blockbuster, bowling

	ROM_REGION( 365, "maincpu2:opla", ROMREGION_ERASE00 )
ROM_END


CONS( 1979, microvsn, 0, 0, microvision, microvision, driver_device, 0, "Milton Bradley", "MicroVision", MACHINE_NOT_WORKING )
