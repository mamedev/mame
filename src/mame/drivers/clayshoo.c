/***************************************************************************

    Atari Clay Shoot hardware

    driver by Zsolt Vasvari

    Games supported:
        * Clay Shoot

    Known issues:
        * none at this time

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"


class clayshoo_state : public driver_device
{
public:
	clayshoo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *   m_videoram;
	size_t    m_videoram_size;

	/* misc */
	emu_timer *m_analog_timer_1, *m_analog_timer_2;
	UINT8 m_input_port_select;
	UINT8 m_analog_port_val;
};


/*************************************
 *
 *  Digital control handling functions
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( input_port_select_w )
{
	clayshoo_state *state = device->machine().driver_data<clayshoo_state>();
	state->m_input_port_select = data;
}


static UINT8 difficulty_input_port_r( running_machine &machine, int bit )
{
	UINT8 ret = 0;

	/* read fake port and remap the buttons to 2 bits */
	UINT8	raw = input_port_read(machine, "FAKE");

	if (raw & (1 << (bit + 1)))
		ret = 0x03;		/* expert */
	else if (raw & (1 << (bit + 2)))
		ret = 0x01;		/* pro */
	else
		ret = 0x00;		/* amateur otherwise */

	return ret;
}


static READ8_DEVICE_HANDLER( input_port_r )
{
	clayshoo_state *state = device->machine().driver_data<clayshoo_state>();
	UINT8 ret = 0;

	switch (state->m_input_port_select)
	{
	case 0x01:	ret = input_port_read(device->machine(), "IN0"); break;
	case 0x02:	ret = input_port_read(device->machine(), "IN1"); break;
	case 0x04:	ret = (input_port_read(device->machine(), "IN2") & 0xf0) | difficulty_input_port_r(device->machine(), 0) |
					  (difficulty_input_port_r(device->machine(), 3) << 2); break;
	case 0x08:	ret = input_port_read(device->machine(), "IN3"); break;
	case 0x10:
	case 0x20:	break;	/* these two are not really used */
	default: logerror("Unexpected port read: %02X\n", state->m_input_port_select);
	}
	return ret;
}



/*************************************
 *
 *  Analog control handling functions
 *
 *************************************/

static TIMER_CALLBACK( reset_analog_bit )
{
	clayshoo_state *state = machine.driver_data<clayshoo_state>();
	state->m_analog_port_val &= ~param;
}


static attotime compute_duration( device_t *device, int analog_pos )
{
	/* the 58 comes from the length of the loop used to
       read the analog position */
	return downcast<cpu_device *>(device)->cycles_to_attotime(58 * analog_pos);
}


static WRITE8_HANDLER( analog_reset_w )
{
	clayshoo_state *state = space->machine().driver_data<clayshoo_state>();

	/* reset the analog value, and start the two times that will fire
       off in a short period proportional to the position of the
       analog control and set the appropriate bit. */

	state->m_analog_port_val = 0xff;

	state->m_analog_timer_1->adjust(compute_duration(&space->device(), input_port_read(space->machine(), "AN1")), 0x02);
	state->m_analog_timer_2->adjust(compute_duration(&space->device(), input_port_read(space->machine(), "AN2")), 0x01);
}


static READ8_HANDLER( analog_r )
{
	clayshoo_state *state = space->machine().driver_data<clayshoo_state>();
	return state->m_analog_port_val;
}


static void create_analog_timers( running_machine &machine )
{
	clayshoo_state *state = machine.driver_data<clayshoo_state>();
	state->m_analog_timer_1 = machine.scheduler().timer_alloc(FUNC(reset_analog_bit));
	state->m_analog_timer_2 = machine.scheduler().timer_alloc(FUNC(reset_analog_bit));
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static const ppi8255_interface ppi8255_intf[2] =
{
	{
		DEVCB_NULL,					/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{
		DEVCB_NULL,					/* Port A read */
		DEVCB_HANDLER(input_port_r),/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_HANDLER(input_port_select_w),	/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* sound effects, Port C write */
	}
};


static MACHINE_START( clayshoo )
{
	clayshoo_state *state = machine.driver_data<clayshoo_state>();
	create_analog_timers(machine);

	/* register for state saving */
	state->save_item(NAME(state->m_input_port_select));
	state->save_item(NAME(state->m_analog_port_val));
}



/*************************************
 *
 *  Video hardware
 *
 *************************************/

static SCREEN_UPDATE_RGB32( clayshoo )
{
	clayshoo_state *state = screen.machine().driver_data<clayshoo_state>();
	offs_t offs;

	for (offs = 0; offs < state->m_videoram_size; offs++)
	{
		int i;
		UINT8 x = offs << 3;
		UINT8 y = ~(offs >> 5);
		UINT8 data = state->m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			bitmap.pix32(y, x) = pen;

			data = data << 1;
			x = x + 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, clayshoo_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x47ff) AM_ROM
	AM_RANGE(0x8000, 0x97ff) AM_RAM AM_BASE_SIZE(m_videoram, m_videoram_size)	/* 6k of video ram according to readme */
	AM_RANGE(0x9800, 0xa800) AM_WRITENOP	  /* not really mapped, but cleared */
	AM_RANGE(0xc800, 0xc800) AM_READWRITE_LEGACY(analog_r, analog_reset_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, clayshoo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE_LEGACY(watchdog_reset_w)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE_LEGACY("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE_LEGACY("ppi8255_1", ppi8255_r, ppi8255_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( clayshoo )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNKNOWN )		/* doesn't appear to be used */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )	/* not 100% positive */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		/* used */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x07, 0x01, "Time/Bonus 1P-2P" )
	PORT_DIPSETTING(    0x00, "60/6k-90/6k" )
	PORT_DIPSETTING(    0x01, "60/6k-120/8k" )
	PORT_DIPSETTING(    0x02, "90/9.5k-150/9.5k" )
	PORT_DIPSETTING(    0x03, "90/9.5k-190/11k" )
	PORT_DIPSETTING(    0x04, "60/8k-90/8k" )
	PORT_DIPSETTING(    0x05, "60/8k-120/10k" )
	PORT_DIPSETTING(    0x06, "90/11.5k-150/11.5k" )
	PORT_DIPSETTING(    0x07, "90/11.5k-190/13k" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* doesn't appear to be used */

	PORT_START("IN2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_SPECIAL )	/* amateur/expert/pro Player 2 */
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_SPECIAL )	/* amateur/expert/pro Player 1 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("AN1")  /* IN4 - Fake analog control.  Visible in $c800 bit 1 */
	PORT_BIT( 0x0f, 0x08, IPT_AD_STICK_Y ) PORT_MINMAX(0,0x0f) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("AN2")  /* IN5 - Fake analog control.  Visible in $c800 bit 0 */
	PORT_BIT( 0x0f, 0x08, IPT_AD_STICK_Y ) PORT_MINMAX(0,0x0f) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("FAKE")	/* IN6 - Fake.  Visible in IN2 bits 0-1 and 2-3 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_TOGGLE PORT_PLAYER(2) PORT_NAME("P2 Amateur Difficulty")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_TOGGLE PORT_PLAYER(2) PORT_NAME("P2 Expert Difficulty")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_TOGGLE PORT_PLAYER(2) PORT_NAME("P2 Pro Difficulty")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("P1 Amateur Difficulty")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("P1 Expert Difficulty")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("P2 Pro Difficulty")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_RESET( clayshoo )
{
	clayshoo_state *state = machine.driver_data<clayshoo_state>();

	state->m_input_port_select = 0;
	state->m_analog_port_val = 0;
}

static MACHINE_CONFIG_START( clayshoo, clayshoo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,5068000/4)		/* 5.068/4 Mhz (divider is a guess) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(clayshoo)
	MCFG_MACHINE_RESET(clayshoo)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 64, 255)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_STATIC(clayshoo)

	MCFG_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MCFG_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( clayshoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0",      0x0000, 0x0800, CRC(9df9d9e3) SHA1(8ce71a6faf5df9c8c3dbb92a443b62c0f376491c) )
	ROM_LOAD( "1",      0x0800, 0x0800, CRC(5134a631) SHA1(f0764a5161934564fd0416be26087cf812e0c422) )
	ROM_LOAD( "2",      0x1000, 0x0800, CRC(5b5a67f6) SHA1(c97b4d44e6dc5dd0c42e04ffceed8934975fe769) )
	ROM_LOAD( "3",      0x1800, 0x0800, CRC(7eda8e44) SHA1(2974f8b06653aee2ffd96ff402707acfc059bc91) )
	ROM_LOAD( "4",      0x4000, 0x0800, CRC(3da16196) SHA1(eb0c0cf0c8fc3db05ac0c469fb20fe92ae6f27ce) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, clayshoo, 0, clayshoo, clayshoo, 0, ROT0, "Allied Leisure", "Clay Shoot", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
