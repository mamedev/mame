/*************************************************************************

    drivers/advision.c

    Driver for the Entex Adventure Vision

**************************************************************************/

/*

    TODO:

    - Turtles music is monotonous
    - convert to discrete sound
    - screen pincushion distortion

*/


#include "emu.h"
#include "includes/advision.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/cop400/cop400.h"
#include "imagedev/cartslot.h"
#include "sound/dac.h"

/* Memory Maps */

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, advision_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROMBANK("bank1")
	AM_RANGE(0x0400, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, advision_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(ext_ram_r, ext_ram_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(controller_r, bankswitch_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(av_control_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(vsync_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, advision_state )
	AM_RANGE(COP400_PORT_L, COP400_PORT_L) AM_READ(sound_cmd_r)
	AM_RANGE(COP400_PORT_G, COP400_PORT_G) AM_WRITE(sound_g_w)
	AM_RANGE(COP400_PORT_D, COP400_PORT_D) AM_WRITE(sound_d_w)
	AM_RANGE(COP400_PORT_SIO, COP400_PORT_SIO) AM_NOP
	AM_RANGE(COP400_PORT_SK, COP400_PORT_SK) AM_NOP
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( advision )
    PORT_START("joystick")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_PLAYER(1)
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_PLAYER(1)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )       PORT_PLAYER(1)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_PLAYER(1)
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
INPUT_PORTS_END

/* Machine Driver */

static COP400_INTERFACE( advision_cop411_interface )
{
	COP400_CKI_DIVISOR_4,
	COP400_CKO_RAM_POWER_SUPPLY, // ??? or not connected
	COP400_MICROBUS_DISABLED
};

static MACHINE_CONFIG_START( advision, advision_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8048_TAG, I8048, XTAL_11MHz)
	MCFG_CPU_PROGRAM_MAP(program_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_CPU_ADD(COP411_TAG, COP411, 52631*16) // COP411L-KCN/N
	MCFG_CPU_CONFIG(advision_cop411_interface)
	MCFG_CPU_IO_MAP(sound_io_map)

    /* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(4*15)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(advision_state, screen_update)
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("advision_cart")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","advision")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( advision )
	ROM_REGION( 0x1000, I8048_TAG, 0 )
	ROM_CART_LOAD( "cart", 0x0000, 0x1000, ROM_NOMIRROR | ROM_FULLSIZE )

	ROM_REGION( 0x400, "bios", 0 )
    ROM_LOAD( "avbios.u5", 0x000, 0x400, CRC(279e33d1) SHA1(bf7b0663e9125c9bfb950232eab627d9dbda8460) )

	ROM_REGION( 0x200, COP411_TAG, 0 )
	ROM_LOAD( "avsound.u8", 0x000, 0x200, CRC(81e95975) SHA1(8b6f8c30dd3e9d8e43f1ea20fba2361b383790eb) )
ROM_END

/* Game Driver */

/*    YEAR  NAME        PARENT  COMPAT  MACHINE   INPUT     INIT        COMPANY                 FULLNAME            FLAGS */
CONS( 1982, advision,	0,		0,		advision, advision, driver_device,	0,			"Entex Industries Inc",	"Adventure Vision", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
