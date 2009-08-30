/*

    United Amusement PC-Engine based hardware
    Driver by Mariusz Wojcieszek
    Thanks for Charles MacDonald for hardware docs

 Overview

 The system consists of a stock PC-Engine console, JAMMA interface board,
 and several cables to connect certain pins from the PCE backplane connector
 and pad connector to the interface board.

 History

 In 1989 United Amusement (a large operator of arcades in the US at that
 time) developed a JAMMA interface for the PC-Engine with NEC's blessing. NEC
 pulled funding for the project before mass production began, and it never
 took off.

 Driver notes:
 - game time is controlled using software loop - with current clock it takes lots
   of time until credit expires. Should Z80 clock be raised?
 - tone played by jamma if board is not emulated
*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"
#include "machine/pcecommn.h"
#include "video/vdc.h"

static UINT8 jamma_if_control_latch = 0;

static WRITE8_HANDLER( jamma_if_control_latch_w )
{
	UINT8 diff = data ^ jamma_if_control_latch;
	jamma_if_control_latch = data;

	sound_global_enable( space->machine, (data >> 7) & 1 );

	if ( diff & 0x40 )
	{
		cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_RESET, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	}

	// bit 3 - enable 752 Hz (D-3) square wave output

	logerror( "Writing control latch with %02X\n", data );
}

static READ8_HANDLER( jamma_if_control_latch_r )
{
	return jamma_if_control_latch & 0x08;
}

static READ8_HANDLER( jamma_if_read_dsw )
{
	UINT8 dsw_val;

	dsw_val = input_port_read(space->machine,  "DSW" );

	if ( BIT( offset, 7 ) == 0 )
	{
		dsw_val >>= 7;
	}
	else if ( BIT( offset, 6 ) == 0 )
	{
		dsw_val >>= 6;
	}
	else if ( BIT( offset, 5 ) == 0 )
	{
		dsw_val >>= 5;
	}
	else if ( BIT( offset, 4 ) == 0 )
	{
		dsw_val >>= 4;
	}
	else if ( BIT( offset, 3 ) == 0 )
	{
		dsw_val >>= 3;
	}
	else if ( BIT( offset, 2 ) == 0 )
	{
		dsw_val >>= 2;
	}
	else if ( BIT( offset, 1 ) == 0 )
	{
		dsw_val >>= 1;
	}
	else if ( BIT( offset, 0 ) == 0 )
	{
		dsw_val >>= 0;
	}

	return dsw_val & 1;
}

static UINT8 jamma_if_read_joystick( running_machine *machine )
{
	if ( jamma_if_control_latch & 0x10 )
	{
		return input_port_read(machine,  "JOY" );
	}
	else
	{
		return input_port_read(machine,  "JOY" ) | 0x08;
	}
}

static MACHINE_RESET( uapce )
{
	pce_set_joystick_readinputport_callback( jamma_if_read_joystick );
	jamma_if_control_latch = 0;
}

static ADDRESS_MAP_START( z80_map, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x07FF) AM_ROM
	AM_RANGE( 0x0800, 0x0FFF) AM_RAM
	AM_RANGE( 0x1000, 0x17FF) AM_WRITE( jamma_if_control_latch_w )
	AM_RANGE( 0x1800, 0x1FFF) AM_READ(  jamma_if_read_dsw )
	AM_RANGE( 0x2000, 0x27FF) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x2800, 0x2FFF) AM_READ(  jamma_if_control_latch_r )
ADDRESS_MAP_END


static INPUT_PORTS_START( uapce )
    PORT_START( "JOY" )
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* button I */
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* button II */
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* select */
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) /* run */
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START( "DSW" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x00, "Game timer (?)" )
	PORT_DIPSETTING(    0x00, "Game timer setting 1" )
	PORT_DIPSETTING(    0x08, "Game timer setting 2" )
	PORT_DIPSETTING(    0x10, "Game timer setting 3" )
	PORT_DIPSETTING(    0x18, "Game timer setting 4" )
	PORT_DIPSETTING(    0x20, "Game timer setting 5" )
	PORT_DIPSETTING(    0x28, "Game timer setting 6" )
	PORT_DIPSETTING(    0x30, "Game timer setting 7" )
	PORT_DIPSETTING(    0x38, "Game timer setting 8" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "COIN" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static ADDRESS_MAP_START( pce_mem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x000000, 0x09FFFF) AM_ROM
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000) AM_BASE( &pce_user_ram )
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_READWRITE( vdc_0_r, vdc_0_w )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_READWRITE( vce_r, vce_w )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE( "c6280", c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_READWRITE( h6280_timer_r, h6280_timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE( pce_joystick_r, pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_READWRITE( h6280_irq_status_r, h6280_irq_status_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io , ADDRESS_SPACE_IO, 8)
	AM_RANGE( 0x00, 0x03) AM_READWRITE( vdc_0_r, vdc_0_w )
ADDRESS_MAP_END

static const c6280_interface c6280_config =
{
	"maincpu"
};

static MACHINE_DRIVER_START( uapce )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", H6280, PCE_MAIN_CLOCK/3)
	MDRV_CPU_PROGRAM_MAP(pce_mem)
	MDRV_CPU_IO_MAP(pce_io)
	MDRV_CPU_VBLANK_INT_HACK(pce_interrupt, VDC_LPF)

	MDRV_CPU_ADD("sub", Z80, 1400000)
	MDRV_CPU_PROGRAM_MAP(z80_map)

	MDRV_QUANTUM_TIME(HZ(60))

	MDRV_MACHINE_RESET( uapce )

    /* video hardware */

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK/2, VDC_WPF, 70, 70 + 512 + 32, VDC_LPF, 14, 14+242)

	/* MDRV_GFXDECODE( pce_gfxdecodeinfo ) */
	MDRV_PALETTE_LENGTH(1024)
	MDRV_PALETTE_INIT( vce )

	MDRV_VIDEO_START( pce )
	MDRV_VIDEO_UPDATE( pce )

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
	MDRV_SOUND_ADD("c6280", C6280, PCE_MAIN_CLOCK/6)
	MDRV_SOUND_CONFIG(c6280_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.00)

MACHINE_DRIVER_END

ROM_START(blazlaz)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "ic1.bin", 0x000000, 0x020000, CRC(c86d44fe) SHA1(070512918e4d305db48bbda374eaff2d121909c5) )
	ROM_LOAD( "ic2.bin", 0x020000, 0x020000, CRC(fb813309) SHA1(60d1ea45717a04d776b6837377467e81431f9bc6) )
	ROM_LOAD( "ic3.bin", 0x080000, 0x020000, CRC(d30a2ecf) SHA1(8328b303e2ffeb694b472719e59044d41471725f) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

static DRIVER_INIT(uapce)
{
	DRIVER_INIT_CALL(pce);
}

GAME( 1989, blazlaz, 0, uapce, uapce, uapce, ROT0, "Hudson Soft", "Blazing Lazers", GAME_IMPERFECT_SOUND )
