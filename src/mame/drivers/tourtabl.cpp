// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

  Atari Tournament Table driver

  Hardware is identical to the VCS2600 except for an extra 6532 chip.

***************************************************************************/

#include "emu.h"
#include "machine/6532riot.h"
#include "cpu/m6502/m6502.h"
#include "sound/tiaintf.h"
#include "video/tia.h"


class tourtabl_state : public driver_device
{
public:
	tourtabl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;

	DECLARE_WRITE8_MEMBER(tourtabl_led_w);
	DECLARE_READ16_MEMBER(tourtabl_read_input_port);
	DECLARE_READ8_MEMBER(tourtabl_get_databus_contents);
	DECLARE_WRITE8_MEMBER(watchdog_w);
};


#define MASTER_CLOCK    XTAL_3_579545MHz


WRITE8_MEMBER(tourtabl_state::tourtabl_led_w)
{
	set_led_status(machine(), 0, data & 0x40); /* start 1 */
	set_led_status(machine(), 1, data & 0x20); /* start 2 */
	set_led_status(machine(), 2, data & 0x10); /* start 4 */
	set_led_status(machine(), 3, data & 0x80); /* select game */

	coin_lockout_global_w(machine(), !(data & 0x80));
}


READ16_MEMBER(tourtabl_state::tourtabl_read_input_port)
{
	static const char *const tianames[] = { "PADDLE4", "PADDLE3", "PADDLE2", "PADDLE1", "TIA_IN4", "TIA_IN5" };

	return ioport(tianames[offset])->read();
}

READ8_MEMBER(tourtabl_state::tourtabl_get_databus_contents)
{
	return offset;
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, tourtabl_state )
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0100) AM_DEVREADWRITE("tia_video", tia_video_device, read, write)
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x0100) AM_RAM
	AM_RANGE(0x0280, 0x029f) AM_DEVREADWRITE("riot1", riot6532_device, read, write)
	AM_RANGE(0x0400, 0x047f) AM_RAM
	AM_RANGE(0x0500, 0x051f) AM_DEVREADWRITE("riot2", riot6532_device, read, write)
	AM_RANGE(0x0800, 0x1fff) AM_ROM
	AM_RANGE(0xe800, 0xffff) AM_ROM
ADDRESS_MAP_END


WRITE8_MEMBER(tourtabl_state::watchdog_w)
{
	machine().watchdog_reset();
}

static INPUT_PORTS_START( tourtabl )

	PORT_START("PADDLE4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(4)

	PORT_START("PADDLE3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("PADDLE2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TIA_IN4")   /* TIA INPT4 */
	PORT_DIPNAME( 0x80, 0x80, "Breakout Replay" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("TIA_IN5")   /* TIA INPT5 */
	PORT_DIPNAME( 0x80, 0x80, "Game Length" )
	PORT_DIPSETTING(    0x00, "11 points (3 balls)" )
	PORT_DIPSETTING(    0x80, "15 points (5 balls)" )

	PORT_START("RIOT0_SWA") /* RIOT #0 SWCHA */
	PORT_DIPNAME( 0x0F, 0x0E, "Replay Level" )
	PORT_DIPSETTING(    0x0B, "200 points" )
	PORT_DIPSETTING(    0x0C, "250 points" )
	PORT_DIPSETTING(    0x0D, "300 points" )
	PORT_DIPSETTING(    0x0E, "400 points" )
	PORT_DIPSETTING(    0x0F, "450 points" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("RIOT0_SWB") /* RIOT #0 SWCHB */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Game Select") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("RIOT1_SWA")/* RIOT #1 SWCHA */
	PORT_DIPNAME( 0x0F, 0x07, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x00, "Mode A" )
	PORT_DIPSETTING(    0x01, "Mode B" )
	PORT_DIPSETTING(    0x02, "Mode C" )
	PORT_DIPSETTING(    0x03, "Mode D" )
	PORT_DIPSETTING(    0x04, "Mode E" )
	PORT_DIPSETTING(    0x05, "Mode F" )
	PORT_DIPSETTING(    0x06, "Mode G" )
	PORT_DIPSETTING(    0x07, "Mode H" )
	PORT_DIPSETTING(    0x08, "Mode I" )
	PORT_DIPSETTING(    0x09, "Mode J" )
	PORT_DIPSETTING(    0x0A, "Mode K" )
	PORT_DIPSETTING(    0x0B, "Mode L" )
	PORT_DIPSETTING(    0x0C, "Mode M" )
	PORT_DIPSETTING(    0x0D, "Mode N" )
	PORT_DIPSETTING(    0x0E, "Mode O" )
	PORT_DIPSETTING(    0x0F, "Mode P" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x10, DEF_STR( French ) )
	PORT_DIPSETTING(    0x20, DEF_STR( German ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Spanish ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("RIOT1_SWB") /* RIOT #1 SWCHB */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )

INPUT_PORTS_END


static MACHINE_CONFIG_START( tourtabl, tourtabl_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK / 3)    /* actually M6507 */
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_DEVICE_ADD("riot1", RIOT6532, MASTER_CLOCK / 3)
	MCFG_RIOT6532_IN_PA_CB(IOPORT("RIOT0_SWA"))
	MCFG_RIOT6532_IN_PB_CB(IOPORT("RIOT0_SWB"))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(tourtabl_state, watchdog_w))

	MCFG_DEVICE_ADD("riot2", RIOT6532, MASTER_CLOCK / 3)
	MCFG_RIOT6532_IN_PA_CB(IOPORT("RIOT1_SWA"))
	MCFG_RIOT6532_IN_PB_CB(IOPORT("RIOT1_SWB"))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(tourtabl_state, tourtabl_led_w))

	/* video hardware */
	MCFG_DEVICE_ADD("tia_video", TIA_NTSC_VIDEO, 0)
	MCFG_TIA_READ_INPUT_PORT_CB(READ16(tourtabl_state, tourtabl_read_input_port))
	MCFG_TIA_DATABUS_CONTENTS_CB(READ8(tourtabl_state, tourtabl_get_databus_contents))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( MASTER_CLOCK, 228, 34, 34 + 160, 262, 46, 46 + 200 )
	MCFG_SCREEN_UPDATE_DEVICE("tia_video", tia_video_device, screen_update)
	MCFG_SCREEN_PALETTE("tia_video:palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_TIA_ADD("tia", MASTER_CLOCK/114)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( tourtabl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "030751.ab2", 0x0800, 0x0800, CRC(4479a6f7) SHA1(bf3fd859614533a592f831e3539ea0a9d1964c82) )
	ROM_RELOAD(             0xE800, 0x0800 )
	ROM_LOAD( "030752.ab3", 0x1000, 0x0800, CRC(c92c49dc) SHA1(cafcf13e1b1087b477a667d1e785f5e2be187b0d) )
	ROM_RELOAD(             0xF000, 0x0800 )
	ROM_LOAD( "030753.ab4", 0x1800, 0x0800, CRC(3978b269) SHA1(4fa05c655bb74711eb99428f36df838ec70da699) )
	ROM_RELOAD(             0xF800, 0x0800 )
ROM_END


ROM_START( tourtab2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "030929.ab2", 0x0800, 0x0800, CRC(fcdfafa2) SHA1(f35ab83366a334a110fbba0cef09f4db950dbb68) )
	ROM_RELOAD(             0xE800, 0x0800 )
	ROM_LOAD( "030752.ab3", 0x1000, 0x0800, CRC(c92c49dc) SHA1(cafcf13e1b1087b477a667d1e785f5e2be187b0d) )
	ROM_RELOAD(             0xF000, 0x0800 )
	ROM_LOAD( "030753.ab4", 0x1800, 0x0800, CRC(3978b269) SHA1(4fa05c655bb74711eb99428f36df838ec70da699) )
	ROM_RELOAD(             0xF800, 0x0800 )
ROM_END


GAME( 1978, tourtabl, 0,        tourtabl, tourtabl, driver_device, 0, ROT0, "Atari", "Tournament Table (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1978, tourtab2, tourtabl, tourtabl, tourtabl, driver_device, 0, ROT0, "Atari", "Tournament Table (set 2)", MACHINE_SUPPORTS_SAVE )
