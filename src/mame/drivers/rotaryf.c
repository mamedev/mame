/* Rotary Fighter, 01/1979, Kasco (Kansai Seiki Seisakusho Co.)
 board KIV-101 CPU: xtal(??mhz), i8085A, 40 pin IC(i8255?), 6*ROM, 1*RAM, DIP(8 switches), ..
 board KIV-101 CRT: 2*RAM, lots of 74xx TTL

driver by Barry Rodewald
 based on Initial work by David Haywood

 todo:

 sound
 verify game speed if possible (related to # of interrupts)

*/

#include "emu.h"
#include "cpu/i8085/i8085.h"


class rotaryf_state : public driver_device
{
public:
	rotaryf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT8 *m_videoram;
	size_t m_videoram_size;

	required_device<cpu_device> m_maincpu;
};






/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( rotaryf_interrupt )
{
	rotaryf_state *state = timer.machine().driver_data<rotaryf_state>();
	int scanline = param;

	if (scanline == 256)
		device_set_input_line(state->m_maincpu, I8085_RST55_LINE, HOLD_LINE);
	else if((scanline % 64) == 0)
	{
		device_set_input_line(state->m_maincpu, I8085_RST75_LINE, ASSERT_LINE);
		device_set_input_line(state->m_maincpu, I8085_RST75_LINE, CLEAR_LINE);
	}
}



/*************************************
 *
 *  Video system
 *
 *************************************/

static SCREEN_UPDATE( rotaryf )
{
	rotaryf_state *state = screen->machine().driver_data<rotaryf_state>();
	offs_t offs;

	for (offs = 0; offs < state->m_videoram_size; offs++)
	{
		int i;

		UINT8 x = offs << 3;
		int y = offs >> 5;
		UINT8 data = state->m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x01) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			data = data >> 1;
			x = x + 1;
		}
	}

	return 0;
}


static ADDRESS_MAP_START( rotaryf_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x17ff) AM_MIRROR(0x4000) AM_ROM
//  AM_RANGE(0x6ffb, 0x6ffb) AM_READ(random_r) ??
//  AM_RANGE(0x6ffd, 0x6ffd) AM_READ(random_r) ??
//  AM_RANGE(0x6fff, 0x6fff) AM_READ(random_r) ??
	AM_RANGE(0x7000, 0x73ff) AM_RAM // clears to 1ff ?
	AM_RANGE(0x8000, 0x9fff) AM_MIRROR(0x4000) AM_RAM AM_BASE_MEMBER(rotaryf_state, m_videoram) AM_SIZE_MEMBER(rotaryf_state, m_videoram_size)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rotaryf_io_map, AS_IO, 8 )
//  AM_RANGE(0x00, 0x00) AM_READ_PORT("UNK")
	AM_RANGE(0x21, 0x21) AM_READ_PORT("COIN")
	AM_RANGE(0x26, 0x26) AM_READ_PORT("DSW")
	AM_RANGE(0x29, 0x29) AM_READ_PORT("INPUTS")
ADDRESS_MAP_END


static INPUT_PORTS_START( rotaryf )
	PORT_START("UNK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x81, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x81, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
//  PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
//  PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
//  PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("COCKTAIL")		/* Dummy port for cocktail mode */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static MACHINE_CONFIG_START( rotaryf, rotaryf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A,4000000) /* ?? MHz */
	MCFG_CPU_PROGRAM_MAP(rotaryf_map)
	MCFG_CPU_IO_MAP(rotaryf_io_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", rotaryf_interrupt, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(32*8, 262)		/* vert size is a guess, taken from mw8080bw */
	MCFG_SCREEN_VISIBLE_AREA(1*8, 30*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE(rotaryf)

MACHINE_CONFIG_END


ROM_START( rotaryf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "krf-1.bin", 0x0000, 0x0400, CRC(f7b2d3e6) SHA1(be7afc1a14be60cb895fc4180167353c7156fc4c) )
	ROM_LOAD( "krf-2.bin", 0x0400, 0x0400, CRC(be9f047a) SHA1(e5dd2b5b4fda7f178e7f1137592ba49fbc9cc82e) )
	ROM_LOAD( "krf-3.bin", 0x0800, 0x0400, CRC(c7629eb6) SHA1(03aae964783ce4b1de77737e83fd2094483fbda4) )
	ROM_LOAD( "krf-4.bin", 0x0c00, 0x0400, CRC(b4703093) SHA1(9239d6da818049bc98a631c3bf5b962b5df5b2ea) )
	ROM_LOAD( "krf-5.bin", 0x1000, 0x0400, CRC(ae233f07) SHA1(a7bbd2ee4477ee041d170e2fc4e94c99c3b564fc) )
	ROM_LOAD( "krf-6.bin", 0x1400, 0x0400, CRC(e28b3713) SHA1(428f73891125f80c722357f1029b18fa9416bcfd) )
ROM_END


GAME( 1979, rotaryf, 0, rotaryf, rotaryf, 0, ROT270, "Kasco", "Rotary Fighter", GAME_NO_SOUND )
