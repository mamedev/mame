/****************************************************************************

    Paranoia
    Driver by Mariusz Wojcieszek

    Notes:
    - jamma interface is not emulated, hence the game is marked as 'not working'
    - rom mapping, memory maps and clocks for jamma interface cpus are probably not correct

Paranoia by Naxat Soft 1990

CPU Z84C00A85 (Z80A CPU)

Xtal : 18.000 Mhz

Ram : GM76C28A (Goldstar)

Ram : 2x W2416K-70 (Winbond)

Else :

Winbond WF19054

Sound : Nec D8085AHC + Nec D8155HC

This board has also :

HuC6260A (Hudson)
HuC6270  (Hudson)
HuC6280A (Hudson)
2x HSRM2564LM12
1x HSRM2564LM10

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "machine/pcecommn.h"
#include "video/vdc.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"


class paranoia_state : public driver_device
{
public:
	paranoia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static INPUT_PORTS_START( paranoia )
    PORT_START( "JOY" )
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* button I */
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* button II */
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* select */
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) /* run */
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

static ADDRESS_MAP_START( pce_mem , AS_PROGRAM, 8)
	AM_RANGE( 0x000000, 0x03FFFF) AM_ROM
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000) AM_BASE( &pce_user_ram )
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_READWRITE( vdc_0_r, vdc_0_w )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_READWRITE( vce_r, vce_w )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE( "c6280", c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_READWRITE( h6280_timer_r, h6280_timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE( pce_joystick_r, pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_READWRITE( h6280_irq_status_r, h6280_irq_status_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io , AS_IO, 8)
	AM_RANGE( 0x00, 0x03) AM_READWRITE( vdc_0_r, vdc_0_w )
ADDRESS_MAP_END

static WRITE8_HANDLER( paranoia_8085_d000_w )
{
	//logerror( "D000 (8085) write %02x\n", data );
}

static WRITE8_HANDLER( paranoia_8085_8155_w )
{
	switch( offset )
	{
	case 0: logerror( "8155 Command register write %x, timer command = %x, interrupt enable = %x, ports = %x\n", data, (data >> 6) & 3, (data >> 4) & 3, data & 0xf ); break;
	case 1: logerror( "8155 I/O Port A write %x\n", data ); break;
	case 2: logerror( "8155 I/O Port B write %x\n", data ); break;
	case 3: logerror( "8155 I/O Port C (or control) write %x\n", data ); break;
	case 4: logerror( "8155 Timer low 8 bits write %x\n", data ); break;
	case 5: logerror( "8155 Timer high 6 bits write %x, timer mode %x\n", data & 0x3f, (data >> 6) & 3); break;
	}
}

static ADDRESS_MAP_START(paranoia_8085_map, AS_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x7fff) AM_ROM
	AM_RANGE( 0x8000, 0x80ff) AM_RAM
	AM_RANGE( 0x8100, 0x8105) AM_WRITE( paranoia_8085_8155_w )
	AM_RANGE( 0xd000, 0xd000) AM_WRITE( paranoia_8085_d000_w )
	AM_RANGE( 0xe000, 0xe1ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(paranoia_8085_io_map, AS_IO, 8)
ADDRESS_MAP_END

static ADDRESS_MAP_START(paranoia_z80_map, AS_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x3fff) AM_ROM
	AM_RANGE( 0x6000, 0x67ff) AM_RAM
	AM_RANGE( 0x7000, 0x73ff) AM_RAM
ADDRESS_MAP_END

static READ8_HANDLER(paranoia_z80_io_01_r)
{
	return 0;
}

static READ8_HANDLER(paranoia_z80_io_02_r)
{
	return 0;
}

static WRITE8_HANDLER(paranoia_z80_io_17_w)
{
}

static WRITE8_HANDLER(paranoia_z80_io_37_w)
{
}

static ADDRESS_MAP_START(paranoia_z80_io_map, AS_IO, 8)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( paranoia_z80_io_01_r )
	AM_RANGE( 0x02, 0x02 ) AM_READ( paranoia_z80_io_02_r )
	AM_RANGE( 0x17, 0x17 ) AM_WRITE( paranoia_z80_io_17_w )
	AM_RANGE( 0x37, 0x37 ) AM_WRITE( paranoia_z80_io_37_w )
ADDRESS_MAP_END

static const c6280_interface c6280_config =
{
	"maincpu"
};

static MACHINE_CONFIG_START( paranoia, paranoia_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, PCE_MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(pce_mem)
	MCFG_CPU_IO_MAP(pce_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", pce_interrupt, "screen", 0, 1)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_CPU_ADD("sub", I8085A, 18000000/3)
	MCFG_CPU_PROGRAM_MAP(paranoia_8085_map)
	MCFG_CPU_IO_MAP(paranoia_8085_io_map)

	MCFG_CPU_ADD("sub2", Z80, 18000000/6)
	MCFG_CPU_PROGRAM_MAP(paranoia_z80_map)
	MCFG_CPU_IO_MAP(paranoia_z80_io_map)

    /* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK/2, VDC_WPF, 70, 70 + 512 + 32, VDC_LPF, 14, 14+242)
	MCFG_SCREEN_UPDATE( pce )

	/* MCFG_GFXDECODE( pce_gfxdecodeinfo ) */
	MCFG_PALETTE_LENGTH(1024)
	MCFG_PALETTE_INIT( vce )

	MCFG_VIDEO_START( pce )

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
	MCFG_SOUND_ADD("c6280", C6280, PCE_MAIN_CLOCK/6)
	MCFG_SOUND_CONFIG(c6280_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

MACHINE_CONFIG_END

ROM_START(paranoia)
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "5.201", 0x00000, 0x40000, CRC(9893e0e6) SHA1(b3097e7f163e4a067cf32f290e59657a8b5e271b) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD( "6.29", 0x0000, 0x8000, CRC(5517532e) SHA1(df8f1621abf1f0c65d86d406cd79d97ec233c378) )

	ROM_REGION( 0x20000, "sub2", 0 )
	ROM_LOAD( "1.319", 0x00000, 0x8000, CRC(ef9f85d8) SHA1(951239042b56cd256daf1965ead2949e2bddcd8b) )
	ROM_LOAD( "2.318", 0x08000, 0x8000, CRC(a35fccca) SHA1(d50e9044a97fe77f31e3198bb6759ba451359069) )
	ROM_LOAD( "3.317", 0x10000, 0x8000, CRC(e3e48ec1) SHA1(299820d0e4fb2fd947c7a52f1c49e2e4d0dd050a) )
	ROM_LOAD( "4.352", 0x18000, 0x8000, CRC(11297fed) SHA1(17a294e65ba1c4806307602dee4c7c627ad1fcfd) )
ROM_END

static DRIVER_INIT(paranoia)
{
	DRIVER_INIT_CALL(pce);
}

GAME( 1990, paranoia, 0, paranoia, paranoia, paranoia, ROT0, "Naxat Soft", "Paranoia", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
