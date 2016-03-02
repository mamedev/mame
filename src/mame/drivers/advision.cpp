// license:BSD-3-Clause
// copyright-holders:Nathan Woods
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
#include "sound/dac.h"
#include "softlist.h"

/* Memory Maps */

READ8_MEMBER( advision_state::rom_r )
{
	offset += 0x400;
	return m_cart->read_rom(space, offset & 0xfff);
}

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, advision_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROMBANK("bank1")
	AM_RANGE(0x0400, 0x0fff) AM_READ(rom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, advision_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(ext_ram_r, ext_ram_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(controller_r, bankswitch_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(av_control_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(vsync_r)
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

static MACHINE_CONFIG_START( advision, advision_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8048_TAG, I8048, XTAL_11MHz)
	MCFG_CPU_PROGRAM_MAP(program_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_CPU_ADD(COP411_TAG, COP411, 52631*16) // COP411L-KCN/N
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_RAM_POWER_SUPPLY, false)
	MCFG_COP400_READ_L_CB(READ8(advision_state, sound_cmd_r))
	MCFG_COP400_WRITE_G_CB(WRITE8(advision_state, sound_g_w))
	MCFG_COP400_WRITE_D_CB(WRITE8(advision_state, sound_d_w))

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(4*15)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(advision_state, screen_update)
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(84, 235, 60, 142)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(advision_state, advision)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "advision_cart")
	MCFG_GENERIC_MANDATORY

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","advision")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( advision )
	ROM_REGION( 0x1000, I8048_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "b225__ins8048-11kdp_n.u5", 0x000, 0x400, CRC(279e33d1) SHA1(bf7b0663e9125c9bfb950232eab627d9dbda8460) ) // "<natsemi logo> /B225 \\ INS8048-11KDP/N"

	ROM_REGION( 0x200, COP411_TAG, 0 )
	ROM_LOAD( "b8223__cop411l-kcn_n.u8", 0x000, 0x200, CRC(81e95975) SHA1(8b6f8c30dd3e9d8e43f1ea20fba2361b383790eb) ) // "<natsemi logo> /B8223 \\ COP411L-KCN/N"
ROM_END

/* Game Driver */

/*    YEAR  NAME        PARENT  COMPAT  MACHINE   INPUT     INIT               COMPANY  FULLNAME            FLAGS */
CONS( 1982, advision,   0,      0,      advision, advision, driver_device,  0, "Entex", "Adventure Vision", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
