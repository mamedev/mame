// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Taito Super Speed Race driver

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sspeedr.lh"
#include "includes/sspeedr.h"



PALETTE_INIT_MEMBER(sspeedr_state, sspeedr)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		int r = (i & 1) ? 0xb0 : 0x20;
		int g = (i & 2) ? 0xb0 : 0x20;
		int b = (i & 4) ? 0xb0 : 0x20;

		if (i & 8)
		{
			r += 0x4f;
			g += 0x4f;
			b += 0x4f;
		}

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


WRITE8_MEMBER(sspeedr_state::sspeedr_int_ack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


WRITE8_MEMBER(sspeedr_state::sspeedr_lamp_w)
{
	output().set_value("lampGO", (data >> 0) & 1);
	output().set_value("lampEP", (data >> 1) & 1);
	machine().bookkeeping().coin_counter_w(0, data & 8);
}


/* uses a 7447A, which is equivalent to an LS47/48 */
static const UINT8 ls48_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

WRITE8_MEMBER(sspeedr_state::sspeedr_time_w)
{
	data = data & 15;
	output().set_digit_value(0x18 + offset, ls48_map[data]);
	m_led_TIME[offset] = data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_score_w)
{
	char buf[20];
	sprintf(buf, "LED%02d", offset);
	data = ~data & 15;
	output().set_digit_value(offset, ls48_map[data]);
	m_led_SCORE[offset] = data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_sound_w)
{
	/* not implemented */
}


static ADDRESS_MAP_START( sspeedr_map, AS_PROGRAM, 8, sspeedr_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x21ff) AM_RAM
	AM_RANGE(0x7f00, 0x7f17) AM_WRITE(sspeedr_score_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sspeedr_io_map, AS_IO, 8, sspeedr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x00, 0x01) AM_WRITE(sspeedr_sound_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(sspeedr_lamp_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN2")
	AM_RANGE(0x04, 0x05) AM_WRITE(sspeedr_time_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(sspeedr_driver_horz_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(sspeedr_driver_pic_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(sspeedr_driver_horz_2_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(sspeedr_drones_horz_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(sspeedr_drones_horz_2_w)
	AM_RANGE(0x15, 0x15) AM_WRITE(sspeedr_drones_mask_w)
	AM_RANGE(0x16, 0x16) AM_WRITE(sspeedr_driver_vert_w)
	AM_RANGE(0x17, 0x18) AM_WRITE(sspeedr_track_vert_w)
	AM_RANGE(0x19, 0x19) AM_WRITE(sspeedr_track_horz_w)
	AM_RANGE(0x1a, 0x1a) AM_WRITE(sspeedr_track_horz_2_w)
	AM_RANGE(0x1b, 0x1b) AM_WRITE(sspeedr_track_ice_w)
	AM_RANGE(0x1c, 0x1e) AM_WRITE(sspeedr_drones_vert_w)
	AM_RANGE(0x1f, 0x1f) AM_WRITE(sspeedr_int_ack_w)
ADDRESS_MAP_END


static const ioport_value sspeedr_controller_table[] =
{
	0x3f, 0x3e, 0x3c, 0x3d, 0x39, 0x38, 0x3a, 0x3b,
	0x33, 0x32, 0x30, 0x31, 0x35, 0x34, 0x36, 0x37,
	0x27, 0x26, 0x24, 0x25, 0x21, 0x20, 0x22, 0x23,
	0x2b, 0x2a, 0x28, 0x29, 0x2d, 0x2c, 0x2e, 0x2f,
	0x0f, 0x0e, 0x0c, 0x0d, 0x09, 0x08, 0x0a, 0x0b,
	0x03, 0x02, 0x00, 0x01, 0x05, 0x04, 0x06, 0x07,
	0x17, 0x16, 0x14, 0x15, 0x11, 0x10, 0x12, 0x13,
	0x1b, 0x1a, 0x18, 0x19, 0x1d, 0x1c, 0x1e, 0x1f
};


static INPUT_PORTS_START( sspeedr )

	PORT_START("IN0")
	PORT_BIT( 0x3f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(64) PORT_REMAP_TABLE(sspeedr_controller_table) PORT_WRAPS PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("IN1")
	/* The gas pedal is adjusted physically so the encoder is at position 2 when the pedal is not pressed. */
	/* It also only uses half of the encoder. */
	PORT_BIT( 0x1f, 0x00, IPT_POSITIONAL_V ) PORT_POSITIONS(30) PORT_REMAP_TABLE(sspeedr_controller_table + 2) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x08, "Play Time" )              PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "60 seconds")
	PORT_DIPSETTING(    0x04, "70 seconds")
	PORT_DIPSETTING(    0x08, "80 seconds")
	PORT_DIPSETTING(    0x0c, "90 seconds")
	PORT_DIPNAME( 0x10, 0x00, "Extended Play" )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "20 seconds" )
	PORT_DIPSETTING(    0x10, "30 seconds" )
	PORT_DIPNAME( 0xe0, 0x20, DEF_STR( Service_Mode ) )  PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x20, "Play Mode" )
	PORT_DIPSETTING(    0xa0, "RAM/ROM Test" )
	PORT_DIPSETTING(    0xe0, "Accelerator Adjustment" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static const gfx_layout car_layout =
{
	32, 16,
	16,
	4,
	{ 0, 1, 2, 3 },
	{
		0x04, 0x04, 0x00, 0x00, 0x0c, 0x0c, 0x08, 0x08,
		0x14, 0x14, 0x10, 0x10, 0x1c, 0x1c, 0x18, 0x18,
		0x24, 0x24, 0x20, 0x20, 0x2c, 0x2c, 0x28, 0x28,
		0x34, 0x34, 0x30, 0x30, 0x3c, 0x3c, 0x38, 0x38
	},
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0
	},
	0x400
};


static GFXDECODE_START( sspeedr )
	GFXDECODE_ENTRY( "gfx1", 0, car_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, car_layout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( sspeedr, sspeedr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_19_968MHz/8)
	MCFG_CPU_PROGRAM_MAP(sspeedr_map)
	MCFG_CPU_IO_MAP(sspeedr_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sspeedr_state,  irq0_line_assert)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.39)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(16 * 1000000 / 15680))
	MCFG_SCREEN_SIZE(376, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 375, 0, 247)
	MCFG_SCREEN_UPDATE_DRIVER(sspeedr_state, screen_update_sspeedr)
	MCFG_SCREEN_VBLANK_DRIVER(sspeedr_state, screen_eof_sspeedr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sspeedr)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(sspeedr_state, sspeedr)

	/* sound hardware */
MACHINE_CONFIG_END


ROM_START( sspeedr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ssr0000.pgm", 0x0000, 0x0800, CRC(bfc7069a) SHA1(2f7aa3d3c7cfd804ba4b625c6a8338534a204855) )
	ROM_LOAD( "ssr0800.pgm", 0x0800, 0x0800, CRC(ec46b59a) SHA1(d5727efecb32ad3d034b885e4a57d7373368ca9e) )

	ROM_REGION( 0x0800, "gfx1", 0 ) /* driver */
	ROM_LOAD( "ssrm762a.f3", 0x0000, 0x0800, CRC(de4653a9) SHA1(a6bbffb7eb60581eee43c74d20ca00b50c9a6e07) )

	ROM_REGION( 0x0800, "gfx2", 0 ) /* drone */
	ROM_LOAD( "ssrm762b.j3", 0x0000, 0x0800, CRC(ef6a1cd6) SHA1(77c31f14783e5ba90849bdc930b099c8360aeba7) )

	ROM_REGION( 0x0800, "gfx3", 0 ) /* track */
	ROM_LOAD( "ssrm762c.l3", 0x0000, 0x0800, CRC(ebaad3ee) SHA1(54ac994b505d20c75cf07a4f68da12360ee00153) )
ROM_END


GAMEL( 1979, sspeedr, 0, sspeedr, sspeedr, driver_device, 0, ROT270, "Midway", "Super Speed Race", MACHINE_NO_SOUND, layout_sspeedr )
