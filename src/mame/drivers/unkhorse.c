/***************************************************************************

  unknown Japanese horse gambling game
  probably early 80s, manufacturer unknown

  from a broken PCB, labeled EFI TG-007
  8085A CPU + 8155 (for I/O and sound)
  8KB RAM mainly for bitmap video, and 512x4 RAM for color map

TODO:
- identify game!
- improve I/O:
  * more buttons/sensors?
  * horse_output_w bits
  * 6-pos dipswitch on the pcb, only 4 are known at the moment
- confirm colors and sound pitch

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "sound/dac.h"


class horse_state : public driver_device
{
public:
	horse_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_video_ram;
	UINT8 *m_color_ram;
	UINT8 m_output;
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT( horse )
{
	// palette is simply 3bpp
	for (int i = 0; i < 8; i++)
		palette_set_color_rgb(machine, i, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
}

static SCREEN_UPDATE_IND16( horse )
{
	horse_state *state = screen.machine().driver_data<horse_state>();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			UINT8 data = state->m_video_ram[y << 5 | x];
			UINT8 color = state->m_color_ram[(y << 3 & 0x780) | x] >> 4;

			for (int i = 0; i < 8; i++)
				bitmap.pix16(y, x << 3 | i) = (data >> i & 1) ? color : 0;
		}
	}

	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

static ADDRESS_MAP_START( horse_map, AS_PROGRAM, 8, horse_state )
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0x4000, 0x40ff) AM_DEVREADWRITE("i8155", i8155_device, memory_r, memory_w)
	AM_RANGE(0x6000, 0x7fff) AM_RAM AM_BASE(m_video_ram)
	AM_RANGE(0x8000, 0x879f) AM_RAM AM_BASE(m_color_ram) AM_MIRROR(0x0860)

ADDRESS_MAP_END

static ADDRESS_MAP_START( horse_io_map, AS_IO, 8, horse_state )
	AM_RANGE(0x40, 0x47) AM_DEVREADWRITE("i8155", i8155_device, io_r, io_w)
ADDRESS_MAP_END


static READ8_DEVICE_HANDLER(horse_input_r)
{
	horse_state *state = device->machine().driver_data<horse_state>();

	switch (state->m_output >> 6 & 3)
	{
		case 0: return input_port_read(device->machine(), "IN0");
		case 1: return input_port_read(device->machine(), "IN1");
		case 2: return input_port_read(device->machine(), "IN2");
		default: break;
	}

	return 0xff;
}

static WRITE8_DEVICE_HANDLER(horse_output_w)
{
	horse_state *state = device->machine().driver_data<horse_state>();
	state->m_output = data;

	// d4: payout related
	// d6-d7: input mux
	// other bits: ?
}

static WRITE_LINE_DEVICE_HANDLER(horse_timer_out)
{
	dac_signed_w(device->machine().device("dac"), 0, state ? 0x7f : 0);
}

static I8155_INTERFACE(i8155_intf)
{
	// port A input, port B output, port C output (but unused)
	DEVCB_HANDLER(horse_input_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(horse_output_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(horse_timer_out)
};


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( horse )
	PORT_START("IN0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING( 0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING( 0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING( 0x07, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING( 0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "UNK04" )				PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK05" )				PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Maximum Bet" )		PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static INTERRUPT_GEN( horse_interrupt )
{
	device_set_input_line(device, I8085_RST75_LINE, ASSERT_LINE);
	device_set_input_line(device, I8085_RST75_LINE, CLEAR_LINE);
}

static MACHINE_CONFIG_START( horse, horse_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_12MHz / 2)
	MCFG_CPU_PROGRAM_MAP(horse_map)
	MCFG_CPU_IO_MAP(horse_io_map)
	MCFG_CPU_VBLANK_INT("screen", horse_interrupt)

	MCFG_I8155_ADD("i8155", XTAL_12MHz / 2, i8155_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MCFG_SCREEN_UPDATE_STATIC(horse)

	MCFG_PALETTE_LENGTH(8)
	MCFG_PALETTE_INIT(horse)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( unkhorse )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "h1.bin", 0x0000, 0x0800, CRC(2b6fc963) SHA1(bdbf71bd0994231517ecf8188ea19cc7d42e5333) )
	ROM_LOAD( "h2.bin", 0x0800, 0x0800, CRC(b9653e78) SHA1(fd1369c734a0fec8def5b7819b14e4c0d1361896) )
	ROM_LOAD( "h3.bin", 0x1000, 0x0800, CRC(77ce7149) SHA1(e6a38f9eb676f84ec0e4c28e27d0aa959b97301f) )
	ROM_LOAD( "h4.bin", 0x1800, 0x0800, CRC(7e77d95d) SHA1(0e0b7acd622806b4eee3c691f05a04bd2989dbea) )
	ROM_LOAD( "h5.bin", 0x2000, 0x0800, CRC(e2c6abdb) SHA1(555f759897904e577a0a38abaaa7636e974192da) )
	ROM_LOAD( "h6.bin", 0x2800, 0x0800, CRC(8b179039) SHA1(079bec1ead7e04e29e552e3b48bec740a869751d) )
	ROM_LOAD( "h7.bin", 0x3000, 0x0800, CRC(db21fc82) SHA1(38cf58c4d33da3e919d058abb482566c8f70d276) )
ROM_END


GAME( 19??, unkhorse, 0, horse, horse, 0, ROT270, "<unknown>", "unknown Japanese horse gambling game", 0 )
