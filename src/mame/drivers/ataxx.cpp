// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Paul Leaman
/***************************************************************************

    Leland Ataxx-era driver

    driver by Aaron Giles and Paul Leaman

    Games supported:
        * Ataxx
        * World Soccer Finals
        * Danny Sullivan's Indy Heat
        * Brute Force
        * Asylum (prototype)

****************************************************************************

    To enter service mode in Ataxx and Brute Force, press 1P start and
    then press the service switch (F2).

    For World Soccer Finals, press the 1P button B and then press the
    service switch.

    For Indy Heat, press the red turbo button (1P button 1) and then
    press the service switch.

***************************************************************************/


#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "cpu/z80/z80.h"
#include "includes/leland.h"


#define MASTER_CLOCK        XTAL_28_63636MHz
#define MCU_CLOCK           XTAL_16MHz



/*************************************
 *
 *  Master CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( master_map_program, AS_PROGRAM, 8, leland_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xdfff) AM_ROMBANK("bank2") AM_WRITE(ataxx_battery_ram_w) AM_SHARE("battery")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(ataxx_paletteram_and_misc_r, ataxx_paletteram_and_misc_w) AM_SHARE("palette")
ADDRESS_MAP_END


static ADDRESS_MAP_START( master_map_io, AS_IO, 8, leland_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x04) AM_DEVREAD("custom", leland_80186_sound_device, leland_80186_response_r)
	AM_RANGE(0x05, 0x05) AM_DEVWRITE("custom", leland_80186_sound_device, leland_80186_command_hi_w)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("custom", leland_80186_sound_device, leland_80186_command_lo_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVWRITE("custom", leland_80186_sound_device, ataxx_80186_control_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(ataxx_eeprom_r, ataxx_eeprom_w)
	AM_RANGE(0xd0, 0xef) AM_READWRITE(ataxx_mvram_port_r, ataxx_mvram_port_w)
	AM_RANGE(0xf0, 0xff) AM_READWRITE(ataxx_master_input_r, ataxx_master_output_w)
ADDRESS_MAP_END




/*************************************
 *
 *  Slave CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( slave_map_program, AS_PROGRAM, 8, leland_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x9fff) AM_ROMBANK("bank3")
	AM_RANGE(0xa000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xfffc, 0xfffd) AM_WRITE(leland_slave_video_addr_w)
	AM_RANGE(0xfffe, 0xfffe) AM_READ(leland_raster_r)
	AM_RANGE(0xffff, 0xffff) AM_WRITE(ataxx_slave_banksw_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( slave_map_io, AS_IO, 8, leland_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x60, 0x7f) AM_READWRITE(ataxx_svram_port_r, ataxx_svram_port_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* Helps document the input ports. */
#define IPT_SLAVEHALT   IPT_SPECIAL


static INPUT_PORTS_START( ataxx )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* huh? affects trackball movement */
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")       /* 0x00 - analog X */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")       /* 0x01 - analog Y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN2")       /* 0x02 - analog X */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN3")       /* 0x03 - analog Y */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( wsf )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_P2")     /* 0x0D */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P3_P4")     /* 0x0E */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("BUTTONS")   /* 0x0F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END


static INPUT_PORTS_START( indyheat )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")       /* Analog wheel 1 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN1")       /* Analog wheel 2 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN2")       /* Analog wheel 3 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_START("AN3")       /* Analog pedal 1 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("AN4")       /* Analog pedal 2 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("AN5")       /* Analog pedal 3 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("P1")        /* 0x0D */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")        /* 0x0E */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")        /* 0x0F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


static INPUT_PORTS_START( brutforc )
	PORT_START("IN0")       /* 0xF6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")       /* 0xF7 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLAVEHALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* 0x20 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")        /* 0x0E */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")        /* 0x0D */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")        /* 0x0F */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( ataxx, leland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("master", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(master_map_program)
	MCFG_CPU_IO_MAP(master_map_io)

	MCFG_CPU_ADD("slave", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(slave_map_program)
	MCFG_CPU_IO_MAP(slave_map_io)

	MCFG_CPU_ADD("audiocpu", I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(leland_80186_map_program)
	MCFG_CPU_IO_MAP(ataxx_80186_map_io)
	MCFG_80186_CHIP_SELECT_CB(DEVWRITE16("custom", leland_80186_sound_device, peripheral_ctrl))
	MCFG_80186_TMROUT0_HANDLER(DEVWRITELINE("custom", leland_80186_sound_device, i80186_tmr0_w))

	MCFG_MACHINE_START_OVERRIDE(leland_state,ataxx)
	MCFG_MACHINE_RESET_OVERRIDE(leland_state,ataxx)

	MCFG_EEPROM_SERIAL_93C56_ADD("eeprom")
	MCFG_EEPROM_SERIAL_ENABLE_STREAMING()

	MCFG_NVRAM_ADD_0FILL("battery")

	/* video hardware */
	MCFG_FRAGMENT_ADD(ataxx_video)

	/* sound hardware */
	MCFG_DEVICE_ADD("custom", ATAXX_80186, 0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( wsf, ataxx )
	MCFG_CPU_MODIFY("audiocpu")
	MCFG_80186_TMROUT1_HANDLER(DEVWRITELINE("custom", leland_80186_sound_device, i80186_tmr1_w))

	MCFG_DEVICE_REMOVE("custom")
	MCFG_DEVICE_ADD("custom", WSF_80186, 0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ataxx )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "e-302-31005-04.u38",   0x00000, 0x20000, CRC(e1cf6236) SHA1(fabf423a006b1db22273c6fffa03edc148d7d957) )
	ROM_RELOAD(                       0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "e-302-31003-01.u15", 0x20001, 0x20000, CRC(8bb3233b) SHA1(5131ad78bdf904cde36534e99efa5576fcea25c0) )
	ROM_LOAD16_BYTE( "e-302-31001-01.u1",  0x20000, 0x20000, CRC(728d75f2) SHA1(d9e8e742cc2d536bd62370c1e474c7036e4392bb) )
	ROM_LOAD16_BYTE( "e-302-31004-01.u16", 0x60001, 0x20000, CRC(f2bdff48) SHA1(f34eb16ea180effffd81d637acc3d96bffaf81c9) )
	ROM_RELOAD(                            0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "e-302-31002-01.u2",  0x60000, 0x20000, CRC(ca06a394) SHA1(0858908bd150dd7354536e10b2a386b45f17ac9f) )
	ROM_RELOAD(                            0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x00001, "user1", ROMREGION_ERASEFF ) /* X-ROM (data used by main processor) */
	/* Empty / not used */

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxx.bin", 0x0000, 0x0100, CRC(989cdb8c) SHA1(13b30a328e71a195960e98e50d1657a8b6860dcf) )
ROM_END


ROM_START( ataxxa )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "u38.u38",   0x00000, 0x20000, CRC(3378937d) SHA1(3c62da7e11b2860c7fe3a35c077cadcf4d0272ca) )
	ROM_RELOAD(            0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "e-302-31003-01.u15", 0x20001, 0x20000, CRC(8bb3233b) SHA1(5131ad78bdf904cde36534e99efa5576fcea25c0) )
	ROM_LOAD16_BYTE( "e-302-31001-01.u1",  0x20000, 0x20000, CRC(728d75f2) SHA1(d9e8e742cc2d536bd62370c1e474c7036e4392bb) )
	ROM_LOAD16_BYTE( "e-302-31004-01.u16", 0x60001, 0x20000, CRC(f2bdff48) SHA1(f34eb16ea180effffd81d637acc3d96bffaf81c9) )
	ROM_RELOAD(                            0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "e-302-31002-01.u2",  0x60000, 0x20000, CRC(ca06a394) SHA1(0858908bd150dd7354536e10b2a386b45f17ac9f) )
	ROM_RELOAD(                            0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x00001, "user1", ROMREGION_ERASEFF ) /* X-ROM (data used by main processor) */
	/* Empty / not used */

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxx.bin", 0x0000, 0x0100, CRC(989cdb8c) SHA1(13b30a328e71a195960e98e50d1657a8b6860dcf) )
ROM_END

ROM_START( ataxxe )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "euro_ataxx_u38_3079.bin", 0x00000, 0x20000, CRC(16aef3b7) SHA1(b2de1e3fd032ab8cc5ed995522f528f0b3283d8a) )
	ROM_RELOAD(                          0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "302-38003-01.u15", 0x20001, 0x20000, CRC(db266d3f) SHA1(31c9baf4548b23e1e1939069620a937ee98a7b09) )
	ROM_LOAD16_BYTE( "302-38001-01.u1",  0x20000, 0x20000, CRC(d6db2724) SHA1(d3c7b45b165eb7c9a6369863b273ecac5c31ca65) )
	ROM_LOAD16_BYTE( "302-38004-01.u16", 0x60001, 0x20000, CRC(2b127f56) SHA1(909fed387ad6bb1d83f9cee271e6dc851ac50525) )
	ROM_RELOAD(                          0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "302-38002-01.u2",  0x60000, 0x20000, CRC(1b63b882) SHA1(cb04e641fc173f787a0f48c98f5198db265c26d8) )
	ROM_RELOAD(                          0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x00001, "user1", ROMREGION_ERASEFF ) /* X-ROM (data used by main processor) */
	/* Empty / not used */

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxxe.bin", 0x0000, 0x0100, CRC(8df1dee1) SHA1(876c5d5d506c31fdf4c3e611a1869b50ceadc6fd) )
ROM_END


ROM_START( ataxxj )
	ROM_REGION( 0x30000, "master", 0 )
	ROM_LOAD( "ataxxj.u38", 0x00000, 0x20000, CRC(513fa7d4) SHA1(1aada72214c0165d76667935855bf996a5b3d55b) )
	ROM_RELOAD(             0x10000, 0x20000 )

	ROM_REGION( 0x60000, "slave", 0 )
	ROM_LOAD( "e-302-31012-01.u111",  0x00000, 0x20000, CRC(9a3297cc) SHA1(1dfa0bacd2f2b18d44bfc2d55c40291c1b142f8f) )
	ROM_LOAD( "e-302-31013-01.u112",  0x20000, 0x20000, CRC(7e7c3e2f) SHA1(a7e31e1f1b09414c40ab9ace5e9bffbdbaee8704) )
	ROM_LOAD( "e-302-31014-01.u113",  0x40000, 0x20000, CRC(8cf3e101) SHA1(672a3a0ca0f5334cf614bc49cbc1ae5ccea54cbe) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "302-38003-01.u15", 0x20001, 0x20000, CRC(db266d3f) SHA1(31c9baf4548b23e1e1939069620a937ee98a7b09) )
	ROM_LOAD16_BYTE( "302-38001-01.u1",  0x20000, 0x20000, CRC(d6db2724) SHA1(d3c7b45b165eb7c9a6369863b273ecac5c31ca65) )
	ROM_LOAD16_BYTE( "302-38004-01.u16", 0x60001, 0x20000, CRC(2b127f56) SHA1(909fed387ad6bb1d83f9cee271e6dc851ac50525) )
	ROM_RELOAD(                          0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "302-38002-01.u2",  0x60000, 0x20000, CRC(1b63b882) SHA1(cb04e641fc173f787a0f48c98f5198db265c26d8) )
	ROM_RELOAD(                          0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "e-302-31006-01.u98",   0x00000, 0x20000, CRC(59d0f2ae) SHA1(8da5dc006e192af98458227e79421b6a07ac1cdc) )
	ROM_LOAD( "e-302-31007-01.u99",   0x20000, 0x20000, CRC(6ab7db25) SHA1(25c2fa23b99ac4bab5a9b851c2087de44512a5c2) )
	ROM_LOAD( "e-302-31008-01.u100",  0x40000, 0x20000, CRC(2352849e) SHA1(f49394b6efb6a87d86516ec0a5ddd582f96f7e5d) )
	ROM_LOAD( "e-302-31009-01.u101",  0x60000, 0x20000, CRC(4c31e02b) SHA1(2d8dd97a2a737bafb44dced7ce3eef22d7d14cbe) )
	ROM_LOAD( "e-302-31010-01.u102",  0x80000, 0x20000, CRC(a951228c) SHA1(7ec5cf4d0aa3702be9236d155bea373a06c0be03) )
	ROM_LOAD( "e-302-31011-01.u103",  0xa0000, 0x20000, CRC(ed326164) SHA1(8706192f525ece200587cee7e7beb4a1975bf63e) )

	ROM_REGION( 0x00001, "user1", ROMREGION_ERASEFF ) /* X-ROM (data used by main processor) */
	/* Empty / not used */

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ataxxj.bin", 0x0000, 0x0100, CRC(8df1dee1) SHA1(876c5d5d506c31fdf4c3e611a1869b50ceadc6fd) )
ROM_END


ROM_START( wsf )
	ROM_REGION( 0x50000, "master", 0 )
	ROM_LOAD( "30022-03.u64",  0x00000, 0x20000, CRC(2e7faa96) SHA1(d43915a433133eca650fabece61a4a65642b39f6) )
	ROM_RELOAD(                0x10000, 0x20000 )
	ROM_LOAD( "30023-03.u65",  0x30000, 0x20000, CRC(7146328f) SHA1(390b98a2cd54a981eb4fafba700ff2fa1e379a32) )

	ROM_REGION( 0x100000, "slave", 0 )
	ROM_LOAD( "30001-01.151",  0x00000, 0x20000, CRC(31c63af5) SHA1(268093ade200241339b6f60a00123bbf73325e38) )
	ROM_LOAD( "30002-01.152",  0x20000, 0x20000, CRC(a53e88a6) SHA1(0b7748b70d6dd9fcc1a22646e8af20f3baa4aa40) )
	ROM_LOAD( "30003-01.153",  0x40000, 0x20000, CRC(12afad1d) SHA1(848549db714b46497176e42d6f2088ba3d6ab2f4) )
	ROM_LOAD( "30004-01.154",  0x60000, 0x20000, CRC(b8b3d59c) SHA1(9ba6e25bb5132c556557a0395ce1d982c0853426) )
	ROM_LOAD( "30005-01.155",  0x80000, 0x20000, CRC(505724b9) SHA1(f8a29e3e7f0a146f2daf67883de12533b2ed7341) )
	ROM_LOAD( "30006-01.156",  0xa0000, 0x20000, CRC(c86b5c4d) SHA1(f04d8fc1e8f872f406fcad69ff71ed695f42797a) )
	ROM_LOAD( "30007-01.157",  0xc0000, 0x20000, CRC(451321ae) SHA1(da82f0bba4341b087136afa17767b64389a0f8f4) )
	ROM_LOAD( "30008-01.158",  0xe0000, 0x20000, CRC(4d23836f) SHA1(7b5b9419774e7537e69017c4c44a0601b6e93714) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "30017-01.u3",  0x20001, 0x20000, CRC(39ec13c1) SHA1(4067da05cbaf205ab7cc14a3370220ad98b394cd) )
	ROM_LOAD16_BYTE( "30020-01.u6",  0x20000, 0x20000, CRC(532c02bf) SHA1(a2070d57f1ce2a68a064872ea7b77ba418187cfe) )
	ROM_LOAD16_BYTE( "30018-01.u4",  0x60001, 0x20000, CRC(1ec16735) SHA1(86766742b50edd25cfeef6f808d2733c484eca4e) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "30019-01.u5",  0x60000, 0x20000, CRC(2881f73b) SHA1(414d974018fb4518c46b913184b07add69251724) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "30011-02.145",  0x00000, 0x10000, CRC(6153569b) SHA1(b6a106c8b87a9a3f01eff3854d0c1f2c4a64fd94) )
	ROM_LOAD( "30012-02.146",  0x10000, 0x10000, CRC(52d65e21) SHA1(25f63aa29dc7e7673043e1f43e357a5232a1be9e) )
	ROM_LOAD( "30013-02.147",  0x20000, 0x10000, CRC(b3afda12) SHA1(52bf780c642f0092114aeb994e6571c034f198a0) )
	ROM_LOAD( "30014-02.148",  0x30000, 0x10000, CRC(624e6c64) SHA1(02240adcf4433543c8f7ad8904c34400f25409cc) )
	ROM_LOAD( "30015-01.149",  0x40000, 0x10000, CRC(5d9064f2) SHA1(7a68a379aa6a6cd0518e8a4107b2e646f5700c2b) )
	ROM_LOAD( "30016-01.150",  0x50000, 0x10000, CRC(d76389cd) SHA1(2b7e6cd662ffde177b110ad0ed2e42fe4ccf811f) )

	ROM_REGION( 0x20000, "user1", 0 ) /* X-ROM (data used by main processor) */
	ROM_LOAD( "30009-01.u68",  0x00000, 0x10000, CRC(f2fbfc15) SHA1(712cfa7b11135b1f568f38cc478ef5a3330d0608) )
	ROM_LOAD( "30010-01.u69",  0x10000, 0x10000, CRC(b4ed2d3b) SHA1(61c9d86b63cf000187a105c6eed967fecb2f3c1c) )

	ROM_REGION( 0x20000, "dac", 0 ) /* externally clocked DAC data */
	ROM_LOAD( "30021-01.u8",   0x00000, 0x20000, CRC(bb91dc10) SHA1(a7d8676867b5cfe1049040e593985af57ef04334) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-wsf.bin", 0x0000, 0x0100, CRC(5bd0633d) SHA1(4917a0b0be82dc1bd4cfdb5bfb509f0472f1014f) )
ROM_END


ROM_START( indyheat )
	ROM_REGION( 0x90000, "master", 0 )
	ROM_LOAD( "u64_27c.010",   0x00000, 0x20000, CRC(2b97a347) SHA1(958a774e9ea3678c0fdd2466e578df8267b4413e) )
	ROM_RELOAD(                0x10000, 0x20000 )
	ROM_LOAD( "u65_27c.010",   0x30000, 0x20000, CRC(71301d74) SHA1(bbabc71aa8d56f6984de573f0fb5d3fea35421a9) )
	ROM_LOAD( "u66_27c.010",   0x50000, 0x20000, CRC(c9612072) SHA1(d00bf703ce4ad0a344b3d8afcd1f45c3c82b54fe) )
	ROM_LOAD( "u67_27c.010",   0x70000, 0x20000, CRC(4c4b25e0) SHA1(f07d347cc844df2d824853af8dbfc557933e7765) )

	ROM_REGION( 0x160000, "slave", 0 )
	ROM_LOAD( "u151_27c.010",  0x00000, 0x20000, CRC(2622dfa4) SHA1(759e46540ad9f2ed540314b174c88f7365214051) )
	ROM_LOAD( "u152_27c.020",  0x20000, 0x20000, CRC(ad40e4e2) SHA1(58c3df82551199fb3f28c6459aedc2117caf520e) )
	ROM_CONTINUE(             0x120000, 0x20000 )
	ROM_LOAD( "u153_27c.020",  0x40000, 0x20000, CRC(1e3803f7) SHA1(e3862ed748cdd0dffdde8e1435c20c7388e698dd) )
	ROM_CONTINUE(             0x140000, 0x20000 )
	ROM_LOAD( "u154_27c.010",  0x60000, 0x20000, CRC(76d3c235) SHA1(48b46fe465c6db4dc46a64245a6c69b21b54ab6f) )
	ROM_LOAD( "u155_27c.010",  0x80000, 0x20000, CRC(d5d866b3) SHA1(2584e2299bdbc50c836ae86a1c4b7e68c65a49cd) )
	ROM_LOAD( "u156_27c.010",  0xa0000, 0x20000, CRC(7fe71842) SHA1(4ba09ccba29f9feef89ce61155e2508e800cdee8) )
	ROM_LOAD( "u157_27c.010",  0xc0000, 0x20000, CRC(a6462adc) SHA1(bdc744e3c836715874d40b9e32f509f288ce00fd) )
	ROM_LOAD( "u158_27c.010",  0xe0000, 0x20000, CRC(d6ef27a3) SHA1(37fcf772ce564a9300f9dd437b9015a2d25b46b5) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "u3_27c.010",  0x20001, 0x20000, CRC(97413818) SHA1(64caa14e05dd9ec43ce13f5c738df1f39f5fa75c) )
	ROM_LOAD16_BYTE( "u6_27c.010",  0x20000, 0x20000, CRC(15a89962) SHA1(52f66e1ccde0ef3fb7959a207cc967237e37833e) )
	ROM_LOAD16_BYTE( "u4_27c.010",  0x60001, 0x20000, CRC(fa7bfa04) SHA1(0174f5372117d15bf0ecd48b72c9cca4cf8bb75f) )
	ROM_RELOAD(                     0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "u5_27c.010",  0x60000, 0x20000, CRC(198285d4) SHA1(8f6b3cba2bc729f2e0623578b13720ead91333e4) )
	ROM_RELOAD(                     0xc0000, 0x20000 )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "u145_27c.010",  0x00000, 0x20000, CRC(612d4bf8) SHA1(035cc8723524e2c6aa79ffa7d7c1f6fb0a25cc51) )
	ROM_LOAD( "u146_27c.010",  0x20000, 0x20000, CRC(77a725f6) SHA1(9bb521ed7202387bbf2670f9b1ae3cbe5064ae03) )
	ROM_LOAD( "u147_27c.010",  0x40000, 0x20000, CRC(d6aac372) SHA1(49f5f5d6c2a82ea15905086a2f8e3ea061d37dfc) )
	ROM_LOAD( "u148_27c.010",  0x60000, 0x20000, CRC(5d19723e) SHA1(a6f09b92c95321962f62a17fc0ccdbfbf78b8b88) )
	ROM_LOAD( "u149_27c.010",  0x80000, 0x20000, CRC(29056791) SHA1(343452b883f139eb09da6b5f384aa680d3a2218c) )
	ROM_LOAD( "u150_27c.010",  0xa0000, 0x20000, CRC(cb73dd6a) SHA1(60aabedbab409acaf8ba4f2366125290825971a4) )

	ROM_REGION( 0x40000, "user1", 0 ) /* X-ROM (data used by main processor) */
	ROM_LOAD( "u68_27c.010",   0x00000, 0x10000, CRC(9e88efb3) SHA1(983bc22c9401b9d6c959dd211b6b7dfa1a6c14e2) )
	ROM_CONTINUE(              0x20000, 0x10000 )
	ROM_LOAD( "u69_27c.010",   0x10000, 0x10000, CRC(aa39fcb3) SHA1(0cb328d784cda3e0dff3a018f52f9b06bc5d46b8) )
	ROM_CONTINUE(              0x30000, 0x10000 )

	ROM_REGION( 0x40000, "dac", 0 ) /* externally clocked DAC data */
	ROM_LOAD( "u8_27c.010",  0x00000, 0x20000, CRC(9f16e5b6) SHA1(0ea814db7f647f39d11dcde793a17831fca3bddd) )
	ROM_LOAD( "u9_27c.010",  0x20000, 0x20000, CRC(0dc8f488) SHA1(2ff0f45f17b8a182afdaa5603e7a1af70e6336b7) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-indyheat.bin", 0x0000, 0x0100, CRC(896f7257) SHA1(bd1f116c2650576da73f0ca647a7f872c890dfe5) )
ROM_END


ROM_START( brutforc )
	ROM_REGION( 0x90000, "master", 0 )
	ROM_LOAD( "u64",   0x00000, 0x20000, CRC(008ae3b8) SHA1(bc9fdba761501efeaf665ac33ff1ad6935d70638) )
	ROM_RELOAD(                 0x10000, 0x20000 )
	ROM_LOAD( "u65",   0x30000, 0x20000, CRC(6036e3fa) SHA1(eba79e92f3de7afdd6e404cabb4b8cfad09cf50b) )
	ROM_LOAD( "u66",   0x50000, 0x20000, CRC(7ebf0795) SHA1(6b25ccac88ff61be3c461eb49908fbecf509434f) )
	ROM_LOAD( "u67",   0x70000, 0x20000, CRC(e3cbf8b4) SHA1(ceaefc454385ee1dfbfe2d211a72af0883967bc0) )

	ROM_REGION( 0x100000, "slave", 0 )
	ROM_LOAD( "u151",  0x00000, 0x20000, CRC(bd3b677b) SHA1(8ac32b9598a97d9910ac31948f166e9474df07fa) )
	ROM_LOAD( "u152",  0x20000, 0x20000, CRC(5f4434e7) SHA1(2b8eb2f6ede328c88b7977e3bea73d00dcaa8f6f) )
	ROM_LOAD( "u153",  0x40000, 0x20000, CRC(20f7df53) SHA1(6ea4600a9cffbc414f546fcd8c036faaa6d7fffd) )
	ROM_LOAD( "u154",  0x60000, 0x20000, CRC(69ce2329) SHA1(24819883631e987a201e7dea0684410e74b9d56d) )
	ROM_LOAD( "u155",  0x80000, 0x20000, CRC(33d92e25) SHA1(fe47da054e12f7e16631cb7cb0279ace717b945b) )
	ROM_LOAD( "u156",  0xa0000, 0x20000, CRC(de7eca8b) SHA1(a5d452c0cb52be16560ccd67d423bdf33d58ec58) )
	ROM_LOAD( "u157",  0xc0000, 0x20000, CRC(e42b3dba) SHA1(ed3707932507bcddd0191e36e2f5479b2ce2e642) )
	ROM_LOAD( "u158",  0xe0000, 0x20000, CRC(a0aa3220) SHA1(bd9bffa4fcf76e34a72a497d322c0430cbc7c81e) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "u3",  0x20001, 0x20000, CRC(9984906c) SHA1(66626ea32fb510a9bb1974e41806fee6a4afa1cf) )
	ROM_LOAD16_BYTE( "u6",  0x20000, 0x20000, CRC(c9c5a413) SHA1(5d4f8bc895b89267643b41ecad52b886fd88df97) )
	ROM_LOAD16_BYTE( "u4",  0x60001, 0x20000, CRC(ca8ab3a6) SHA1(2e7c7f50fbaed7e052a97ac7954b634bbc657226) )
	ROM_RELOAD(             0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "u5",  0x60000, 0x20000, CRC(cbdb914b) SHA1(813640fa291c1245d04a628ee62afc95d5c67a03) )
	ROM_RELOAD(             0xc0000, 0x20000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "u145",  0x000000, 0x40000, CRC(c3d20d24) SHA1(a75217b0d1887c64bf5570ff7a461c8cf47c5e85) )
	ROM_LOAD( "u146",  0x040000, 0x40000, CRC(43e9dd87) SHA1(0694803a5b33c074858770c7e4cd884402c263f8) )
	ROM_LOAD( "u147",  0x080000, 0x40000, CRC(fb855ce8) SHA1(839bca2d8e344d43fad8978b812c9246a89054a8) )
	ROM_LOAD( "u148",  0x0c0000, 0x40000, CRC(e4b54eae) SHA1(591ee8e0c1b7c2eb8d7834a42548d5b25c79bb26) )
	ROM_LOAD( "u149",  0x100000, 0x40000, CRC(cf48401c) SHA1(70ba8f2d5f81795c26c2a552c29c913c5d3bd784) )
	ROM_LOAD( "u150",  0x140000, 0x40000, CRC(ca9e1e33) SHA1(f9889042b536e1fb5521702bc807d5aa0e6a25d1) )

	ROM_REGION( 0x40000, "user1", 0 ) /* X-ROM (data used by main processor) */
	ROM_LOAD( "u68",   0x00000, 0x10000, CRC(77c8de62) SHA1(ae15f84b7bf3d6705edf9f41d8de7b6ecab2bcf9) )
	ROM_CONTINUE(      0x20000, 0x10000 )
	ROM_LOAD( "u69",   0x10000, 0x10000, CRC(113aa6d5) SHA1(d032a04338e12135ba410afd71cf9538e99eb109) )
	ROM_CONTINUE(      0x30000, 0x10000 )

	ROM_REGION( 0x80000, "dac", 0 ) /* externally clocked DAC data */
	ROM_LOAD( "u8",  0x00000, 0x20000, CRC(1e0ead72) SHA1(879d5ba244238af21f6a516494c504721570ec15) )
	ROM_LOAD( "u9",  0x20000, 0x20000, CRC(3195b305) SHA1(7c795a7973e0b8dbeb882777d4bee2accc46cea0) )
	ROM_LOAD( "u10", 0x40000, 0x20000, CRC(1dc5f375) SHA1(9dd389c30d87fcb02c6a15b67b4b6ea5b555a762) )
	ROM_LOAD( "u11", 0x60000, 0x20000, CRC(5ed4877f) SHA1(eab9e949b1afd1fa21d87af5abcb1a8dc9bcf0d8) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-brutforc.bin", 0x0000, 0x0100, CRC(508809af) SHA1(17352c0922631fca2ca2bbca4c50b3e0277caaf9) )
ROM_END


ROM_START( asylum )
	ROM_REGION( 0x90000, "master", 0 )
	ROM_LOAD( "asy-m0.64",   0x00000, 0x20000, CRC(f5ca36fd) SHA1(8c36ce3ca1c30ffb0a32ff7e9df61901c1ee6151) )
	ROM_RELOAD(              0x10000, 0x20000 )
	ROM_LOAD( "asy-m1.65",   0x30000, 0x20000, CRC(14d91d09) SHA1(ad227e6f5047f43c421773385f441c634af110e6) )
	ROM_LOAD( "asy-m2.66",   0x50000, 0x20000, CRC(a34a6ef9) SHA1(c90307024039a7809b7fafb019c9ad4636708a88) )
	ROM_LOAD( "asy-m3.67",   0x70000, 0x20000, CRC(9db4c2b1) SHA1(cfe78e2fe803c816ed2f79250bbbaf293cb5bf2a) )

	ROM_REGION( 0x1e0000, "slave", 0 )
	ROM_LOAD( "asy-sp0.151",  0x00000, 0x20000, CRC(5ad5e3b0) SHA1(0162b56f63c169825677323dfbbd3ea991a9d9bb) )
	ROM_LOAD( "asy-sp2.152",  0x20000, 0x20000, CRC(6d2997ec) SHA1(bf97dba0a4a700af0eb753daf598ec8e903dbc7c) )
	ROM_CONTINUE(            0x120000, 0x20000 )
	ROM_LOAD( "asy-sp4.153",  0x40000, 0x20000, CRC(7c61973c) SHA1(560ac49f92ddb25b975cbfb3ffc1464fe0c72e90) )
	ROM_CONTINUE(            0x140000, 0x20000 )
	ROM_LOAD( "asy-sp6.154",  0x60000, 0x20000, CRC(f0a4f9d3) SHA1(af7737803c909afad0d44f328adf14a9e7b3b108) )
	ROM_CONTINUE(            0x160000, 0x20000 )
	ROM_LOAD( "asy-sp8.155",  0x80000, 0x20000, CRC(2ad0640e) SHA1(6be547c297eb09187663bf3302b01c31d2990dac) )
	ROM_CONTINUE(            0x180000, 0x20000 )
	ROM_LOAD( "asy-spa.156",  0xa0000, 0x20000, CRC(9d584fb4) SHA1(fb331c63cb3f29ed6925acc1b1e41d63a242af37) )
	ROM_CONTINUE(            0x1a0000, 0x20000 )
	ROM_LOAD( "asy-spc.157",  0xc0000, 0x20000, CRC(8485e48c) SHA1(7381b55c96b1fce58e2f8914d603b35b397c881b) )
	ROM_CONTINUE(            0x1c0000, 0x20000 )
	ROM_LOAD( "asy-spe.158",  0xe0000, 0x20000, CRC(49d19520) SHA1(6f24221c976e9dacc1ce96dfc1d1e3df4e8a8255) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "asy-65.3",  0x20001, 0x20000, CRC(709bdc78) SHA1(ca235c2ab26fbb153ffe775a1a44b31695902d3f) )
	ROM_LOAD16_BYTE( "asy-65.6",  0x20000, 0x20000, CRC(d019fb2e) SHA1(9d16b0399f03067e7bf79043904a1045119937c6) )
	ROM_LOAD16_BYTE( "asy-65.4",  0x60001, 0x20000, CRC(1882c3b2) SHA1(71af49d1f59e257e5f8a0fc590d0533dda5bf82b) )
	ROM_RELOAD(                   0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "asy-65.5",  0x60000, 0x20000, CRC(5814b307) SHA1(6db97804d58941a5543424d8c4658cb3edab1e43) )
	ROM_RELOAD(                   0xc0000, 0x20000 )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "asy-chr0.145",  0x000000, 0x40000, CRC(4dbcae49) SHA1(0aa54daa099d6590a41df4a24a27bf6463b3e116) )
	ROM_LOAD( "asy-chr1.146",  0x040000, 0x40000, CRC(34e7762d) SHA1(2d63971effc237846481bed7d829fa924b4bea31) )
	ROM_LOAD( "asy-chr2.147",  0x080000, 0x40000, CRC(f9b0d375) SHA1(305172d8cdf390d9566c2c6f32d8da44b165022a) )
	ROM_LOAD( "asy-chr3.148",  0x0c0000, 0x40000, CRC(5efcae94) SHA1(dd7f903efd15e14c06e8d53cf7021f4323c127d1) )
	ROM_LOAD( "asy-chr4.149",  0x100000, 0x40000, CRC(dbc2b155) SHA1(ba0d90b5a6acc53ecd02317cb82b630451e9d0e9) )
	ROM_LOAD( "asy-chr5.150",  0x140000, 0x40000, CRC(9675e44f) SHA1(d2633d21fa9e798b8f96d96fdce5bb99a7dc5ba5) )

	ROM_REGION( 0x40000, "user1", 0 ) /* X-ROM (data used by main processor) */
	ROM_LOAD( "asy-m4.68",   0x00000, 0x10000, CRC(77c8de62) SHA1(ae15f84b7bf3d6705edf9f41d8de7b6ecab2bcf9) )
	ROM_CONTINUE(            0x20000, 0x10000 )
	ROM_LOAD( "asy-m5.69",   0x10000, 0x10000, CRC(bfc50d6c) SHA1(3239242358e8336354a9bd35f75f9057f079b298) )
	ROM_CONTINUE(            0x30000, 0x10000 )

	ROM_REGION( 0x80000, "dac", 0 ) /* externally clocked DAC data */
	ROM_LOAD( "asy-65.8",  0x00000, 0x20000, CRC(624ad02f) SHA1(ce2dd0d11ff39a8e04d1c27cdaca3f068e6fbcf2) )
	ROM_LOAD( "asy-65.9",  0x20000, 0x20000, CRC(c92ff376) SHA1(0189519101e3b0b464f0bd3af8352c002e45f937) )
	ROM_LOAD( "asy-65.10", 0x40000, 0x20000, CRC(744dbf25) SHA1(03ea3d6eef94005ec0fbbaf43b59e3063830452e) )
	ROM_LOAD( "asy-65.11", 0x60000, 0x20000, CRC(4b185d22) SHA1(d59a72d8c6532875f6e31939c5f846da64ba1bdd) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-asylum.bin", 0x0000, 0x0100, CRC(9a9a361b) SHA1(35daf1677ba18c09d2f9e33e75cf3f8d6a01e7c8) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(leland_state,ataxx)
{
	leland_rotate_memory("master");
	leland_rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0x00, 0x03, read8_delegate(FUNC(leland_state::ataxx_trackball_r),this));
}


DRIVER_INIT_MEMBER(leland_state,ataxxj)
{
	leland_rotate_memory("master");
	leland_rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0x00, 0x03, read8_delegate(FUNC(leland_state::ataxx_trackball_r),this));
}


DRIVER_INIT_MEMBER(leland_state,wsf)
{
	leland_rotate_memory("master");
	leland_rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P1_P2");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P3_P4");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "BUTTONS");
}


DRIVER_INIT_MEMBER(leland_state,indyheat)
{
	leland_rotate_memory("master");
	leland_rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_handler(0x00, 0x02, read8_delegate(FUNC(leland_state::indyheat_wheel_r),this));
	m_master->space(AS_IO).install_read_handler(0x08, 0x0b, read8_delegate(FUNC(leland_state::indyheat_analog_r),this));
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P1");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P2");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "P3");

	/* set up additional output ports */
	m_master->space(AS_IO).install_write_handler(0x08, 0x0b, write8_delegate(FUNC(leland_state::indyheat_analog_w),this));
}


DRIVER_INIT_MEMBER(leland_state,brutforc)
{
	leland_rotate_memory("master");
	leland_rotate_memory("slave");

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P2");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P1");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "P3");
}


DRIVER_INIT_MEMBER(leland_state,asylum)
{
	leland_rotate_memory("master");
	leland_rotate_memory("slave");

	/* asylum appears to have some extra RAM for the slave CPU */
	m_slave->space(AS_PROGRAM).install_ram(0xf000, 0xfffb);

	/* set up additional input ports */
	m_master->space(AS_IO).install_read_port(0x0d, 0x0d, "P2");
	m_master->space(AS_IO).install_read_port(0x0e, 0x0e, "P1");
	m_master->space(AS_IO).install_read_port(0x0f, 0x0f, "P3");
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1990, ataxx,    0,      ataxx,   ataxx,    leland_state, ataxx,    ROT0,   "Leland Corp.", "Ataxx (set 1)", 0 )
GAME( 1990, ataxxa,   ataxx,  ataxx,   ataxx,    leland_state, ataxx,    ROT0,   "Leland Corp.", "Ataxx (set 2)", 0 )
GAME( 1990, ataxxe,   ataxx,  ataxx,   ataxx,    leland_state, ataxx,    ROT0,   "Leland Corp.", "Ataxx (Europe)", 0 )
GAME( 1990, ataxxj,   ataxx,  ataxx,   ataxx,    leland_state, ataxxj,   ROT0,   "Leland Corp. (Capcom license)", "Ataxx (Japan)", 0 )
GAME( 1990, wsf,      0,      wsf,     wsf,      leland_state, wsf,      ROT0,   "Leland Corp.", "World Soccer Finals", 0 )
GAME( 1991, indyheat, 0,      wsf,     indyheat, leland_state, indyheat, ROT0,   "Leland Corp.", "Danny Sullivan's Indy Heat", 0 )
GAME( 1991, brutforc, 0,      wsf,     brutforc, leland_state, brutforc, ROT0,   "Leland Corp.", "Brute Force", 0 )
GAME( 1991, asylum,   0,      wsf,     brutforc, leland_state, asylum,   ROT270, "Leland Corp.", "Asylum (prototype)", 0 )
