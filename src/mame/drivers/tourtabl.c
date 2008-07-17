/***************************************************************************

  Atari Tournament Table driver

  Hardware is identical to the VCS2600 except for an extra 6532 chip.

***************************************************************************/

#include "driver.h"
#include "machine/6532riot.h"
#include "cpu/m6502/m6502.h"
#include "sound/tiaintf.h"
#include "video/tia.h"


#define MASTER_CLOCK	3579575


static UINT8* r6532_0_ram;
static UINT8* r6532_1_ram;



static WRITE8_HANDLER( tourtabl_led_w )
{
	set_led_status(0, data & 0x40); /* start 1 */
	set_led_status(1, data & 0x20); /* start 2 */
	set_led_status(2, data & 0x10); /* start 4 */
	set_led_status(3, data & 0x80); /* select game */

	coin_lockout_global_w(!(data & 0x80));
}


static WRITE8_HANDLER( r6532_0_ram_w )
{
	r6532_0_ram[offset] = data;
}
static WRITE8_HANDLER( r6532_1_ram_w )
{
	r6532_1_ram[offset] = data;
}


static READ8_HANDLER( r6532_0_ram_r )
{
	return r6532_0_ram[offset];
}
static READ8_HANDLER( r6532_1_ram_r )
{
	return r6532_1_ram[offset];
}


static READ16_HANDLER( tourtabl_read_input_port )
{
	static const char *tianames[] = { "PADDLE4", "PADDLE3", "PADDLE2", "PADDLE1", "TIA_IN4", "TIA_IN5" };

	return input_port_read(machine, tianames[offset]);
}

static READ8_HANDLER( tourtabl_get_databus_contents )
{
	return offset;
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007F) AM_READ(tia_r)
	AM_RANGE(0x0080, 0x00FF) AM_READ(r6532_0_ram_r)
	AM_RANGE(0x0100, 0x017F) AM_READ(tia_r)
	AM_RANGE(0x0180, 0x01FF) AM_READ(r6532_0_ram_r)
	AM_RANGE(0x0280, 0x029F) AM_READ(r6532_0_r)
	AM_RANGE(0x0400, 0x047F) AM_READ(r6532_1_ram_r)
	AM_RANGE(0x0500, 0x051F) AM_READ(r6532_1_r)
	AM_RANGE(0x0800, 0x1FFF) AM_READ(SMH_ROM)
	AM_RANGE(0xE800, 0xFFFF) AM_READ(SMH_ROM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007F) AM_WRITE(tia_w)
	AM_RANGE(0x0080, 0x00FF) AM_WRITE(r6532_0_ram_w) AM_BASE(&r6532_0_ram)
	AM_RANGE(0x0100, 0x017F) AM_WRITE(tia_w)
	AM_RANGE(0x0180, 0x01FF) AM_WRITE(r6532_0_ram_w)
	AM_RANGE(0x0280, 0x029F) AM_WRITE(r6532_0_w)
	AM_RANGE(0x0400, 0x047F) AM_WRITE(r6532_1_ram_w) AM_BASE(&r6532_1_ram)
	AM_RANGE(0x0500, 0x051F) AM_WRITE(r6532_1_w)
	AM_RANGE(0x0800, 0x1FFF) AM_WRITE(SMH_ROM)
	AM_RANGE(0xE800, 0xFFFF) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END


static const struct riot6532_interface r6532_interface_0 =
{
	input_port_6_r,
	input_port_7_r,
	NULL,
	watchdog_reset_w,
	NULL
};


static const struct riot6532_interface r6532_interface_1 =
{
	input_port_8_r,
	input_port_9_r,
	NULL,
	tourtabl_led_w,
	NULL
};


static const struct tia_interface tourtabl_tia_interface =
{
	tourtabl_read_input_port,
	tourtabl_get_databus_contents,
	NULL
};


static MACHINE_START( tourtabl )
{
	r6532_config(machine, 0, &r6532_interface_0);
	r6532_config(machine, 1, &r6532_interface_1);
	r6532_reset(machine, 0);
	r6532_reset(machine, 1);

	tia_init( machine, &tourtabl_tia_interface );
}


static INPUT_PORTS_START( tourtabl )

	PORT_START_TAG("PADDLE4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(4)

	PORT_START_TAG("PADDLE3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(3)

	PORT_START_TAG("PADDLE2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)

	PORT_START_TAG("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)

	PORT_START_TAG("TIA_IN4")	/* TIA INPT4 */
	PORT_DIPNAME( 0x80, 0x80, "Breakout Replay" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START_TAG("TIA_IN5")	/* TIA INPT5 */
	PORT_DIPNAME( 0x80, 0x80, "Game Length" )
	PORT_DIPSETTING(    0x00, "11 points (3 balls)" )
	PORT_DIPSETTING(    0x80, "15 points (5 balls)" )

	PORT_START_TAG("RIOT0_SWA")	/* RIOT #0 SWCHA */
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

	PORT_START_TAG("RIOT0_SWB")	/* RIOT #0 SWCHB */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Game Select") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("RIOT1_SWA")/* RIOT #1 SWCHA */
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

	PORT_START_TAG("RIOT1_SWB")	/* RIOT #1 SWCHB */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )

INPUT_PORTS_END


static MACHINE_DRIVER_START( tourtabl )
	/* basic machine hardware */
	MDRV_CPU_ADD(M6502, MASTER_CLOCK / 3)	/* actually M6507 */
	MDRV_CPU_PROGRAM_MAP(readmem, writemem)

	MDRV_MACHINE_START(tourtabl)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS( MASTER_CLOCK, 228, 34, 34 + 160, 262, 46, 46 + 200 )

	MDRV_PALETTE_LENGTH(TIA_PALETTE_LENGTH)
	MDRV_PALETTE_INIT(tia_NTSC)

	MDRV_VIDEO_START(tia)
	MDRV_VIDEO_UPDATE(tia)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(TIA, MASTER_CLOCK/114)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( tourtabl )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "030751.ab2", 0x0800, 0x0800, CRC(4479a6f7) SHA1(bf3fd859614533a592f831e3539ea0a9d1964c82) )
	ROM_RELOAD(             0xE800, 0x0800 )
	ROM_LOAD( "030752.ab3", 0x1000, 0x0800, CRC(c92c49dc) SHA1(cafcf13e1b1087b477a667d1e785f5e2be187b0d) )
	ROM_RELOAD(             0xF000, 0x0800 )
	ROM_LOAD( "030753.ab4", 0x1800, 0x0800, CRC(3978b269) SHA1(4fa05c655bb74711eb99428f36df838ec70da699) )
	ROM_RELOAD(             0xF800, 0x0800 )
ROM_END


ROM_START( tourtab2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "030929.ab2", 0x0800, 0x0800, CRC(fcdfafa2) SHA1(f35ab83366a334a110fbba0cef09f4db950dbb68) )
	ROM_RELOAD(             0xE800, 0x0800 )
	ROM_LOAD( "030752.ab3", 0x1000, 0x0800, CRC(c92c49dc) SHA1(cafcf13e1b1087b477a667d1e785f5e2be187b0d) )
	ROM_RELOAD(             0xF000, 0x0800 )
	ROM_LOAD( "030753.ab4", 0x1800, 0x0800, CRC(3978b269) SHA1(4fa05c655bb74711eb99428f36df838ec70da699) )
	ROM_RELOAD(             0xF800, 0x0800 )
ROM_END


GAME( 1978, tourtabl, 0,        tourtabl, tourtabl, 0, ROT0, "Atari", "Tournament Table (set 1)", 0 )
GAME( 1978, tourtab2, tourtabl, tourtabl, tourtabl, 0, ROT0, "Atari", "Tournament Table (set 2)", 0 )
