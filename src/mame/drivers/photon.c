/*
    Photon System

    Uses PK8000 emulation by Miodrag Milanovic
    Imported to MAME by Mariusz Wojcieszek

    Russian arcade system based on PK8000 home computer, created by unknown manufacturer
    in late 1980s or early 1990s.

    Following games were produced for this system:
    - Tetris
    - Python
    - Klad/Labyrinth

    Use joystick left and right in Klad/Labyrinth attract mode to select a game to play.
*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/pk8000.h"
#include "machine/i8255.h"
#include "sound/speaker.h"


class photon_state : public driver_device
{
public:
	photon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	DECLARE_WRITE8_MEMBER(pk8000_80_porta_w);
	DECLARE_READ8_MEMBER(pk8000_80_portb_r);
	DECLARE_WRITE8_MEMBER(pk8000_80_portc_w);
	DECLARE_READ8_MEMBER(pk8000_84_porta_r);
	DECLARE_WRITE8_MEMBER(pk8000_84_porta_w);
	DECLARE_WRITE8_MEMBER(pk8000_84_portc_w);
};


static void pk8000_set_bank(running_machine &machine,UINT8 data)
{
	UINT8 *rom = machine.root_device().memregion("maincpu")->base();
	UINT8 *ram = machine.root_device().memregion("maincpu")->base();
	UINT8 block1 = data & 3;
	UINT8 block2 = (data >> 2) & 3;
	UINT8 block3 = (data >> 4) & 3;
	UINT8 block4 = (data >> 6) & 3;

	switch(block1) {
		case 0:
				machine.root_device().membank("bank1")->set_base(rom + 0x10000);
				machine.root_device().membank("bank5")->set_base(ram);
				break;
		case 1: break;
		case 2: break;
		case 3:
				machine.root_device().membank("bank1")->set_base(ram);
				machine.root_device().membank("bank5")->set_base(ram);
				break;
	}

	switch(block2) {
		case 0:
				machine.root_device().membank("bank2")->set_base(rom + 0x14000);
				machine.root_device().membank("bank6")->set_base(ram + 0x4000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				machine.root_device().membank("bank2")->set_base(ram + 0x4000);
				machine.root_device().membank("bank6")->set_base(ram + 0x4000);
				break;
	}
	switch(block3) {
		case 0:
				machine.root_device().membank("bank3")->set_base(rom + 0x18000);
				machine.root_device().membank("bank7")->set_base(ram + 0x8000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				machine.root_device().membank("bank3")->set_base(ram + 0x8000);
				machine.root_device().membank("bank7")->set_base(ram + 0x8000);
				break;
	}
	switch(block4) {
		case 0:
				machine.root_device().membank("bank4")->set_base(rom + 0x1c000);
				machine.root_device().membank("bank8")->set_base(ram + 0xc000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				machine.root_device().membank("bank4")->set_base(ram + 0xc000);
				machine.root_device().membank("bank8")->set_base(ram + 0xc000);
				break;
	}
}
WRITE8_MEMBER(photon_state::pk8000_80_porta_w)
{
	pk8000_set_bank(machine(),data);
}

READ8_MEMBER(photon_state::pk8000_80_portb_r)
{
	return 0xff;
}

WRITE8_MEMBER(photon_state::pk8000_80_portc_w)
{
	speaker_level_w(machine().device("speaker"), BIT(data,7));
}

static I8255_INTERFACE( pk8000_ppi8255_interface_1 )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(photon_state,pk8000_80_porta_w),
	DEVCB_DRIVER_MEMBER(photon_state,pk8000_80_portb_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(photon_state,pk8000_80_portc_w)
};

READ8_MEMBER(photon_state::pk8000_84_porta_r)
{
	return pk8000_video_mode;
}

WRITE8_MEMBER(photon_state::pk8000_84_porta_w)
{
	pk8000_video_mode = data;
}

WRITE8_MEMBER(photon_state::pk8000_84_portc_w)
{
	pk8000_video_enable = BIT(data,4);
}
static I8255A_INTERFACE( pk8000_ppi8255_interface_2 )
{
	DEVCB_DRIVER_MEMBER(photon_state,pk8000_84_porta_r),
	DEVCB_DRIVER_MEMBER(photon_state,pk8000_84_porta_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(photon_state,pk8000_84_portc_w)
};

static ADDRESS_MAP_START(pk8000_mem, AS_PROGRAM, 8, photon_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bank1") AM_WRITE_BANK("bank5")
	AM_RANGE( 0x4000, 0x7fff ) AM_READ_BANK("bank2") AM_WRITE_BANK("bank6")
	AM_RANGE( 0x8000, 0xbfff ) AM_READ_BANK("bank3") AM_WRITE_BANK("bank7")
	AM_RANGE( 0xc000, 0xffff ) AM_READ_BANK("bank4") AM_WRITE_BANK("bank8")
ADDRESS_MAP_END

static ADDRESS_MAP_START( pk8000_io , AS_IO, 8, photon_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
	AM_RANGE(0x88, 0x88) AM_READWRITE_LEGACY(pk8000_video_color_r,pk8000_video_color_w)
	AM_RANGE(0x8c, 0x8c) AM_READ_PORT("JOY1")
	AM_RANGE(0x8d, 0x8d) AM_READ_PORT("JOY2")
	AM_RANGE(0x90, 0x90) AM_READWRITE_LEGACY(pk8000_text_start_r,pk8000_text_start_w)
	AM_RANGE(0x91, 0x91) AM_READWRITE_LEGACY(pk8000_chargen_start_r,pk8000_chargen_start_w)
	AM_RANGE(0x92, 0x92) AM_READWRITE_LEGACY(pk8000_video_start_r,pk8000_video_start_w)
	AM_RANGE(0x93, 0x93) AM_READWRITE_LEGACY(pk8000_color_start_r,pk8000_color_start_w)
	AM_RANGE(0xa0, 0xbf) AM_READWRITE_LEGACY(pk8000_color_r,pk8000_color_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( photon )
	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("JOY2")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INTERRUPT_GEN( pk8000_interrupt )
{
	device->execute().set_input_line(0, HOLD_LINE);
}

static IRQ_CALLBACK(pk8000_irq_callback)
{
	return 0xff;
}


static MACHINE_RESET(pk8000)
{
	pk8000_set_bank(machine,0);
	machine.device("maincpu")->execute().set_irq_acknowledge_callback(pk8000_irq_callback);
}

static VIDEO_START( photon )
{
}

static SCREEN_UPDATE_IND16( photon )
{
	return pk8000_video_update(screen, bitmap, cliprect, screen.machine().root_device().memregion("maincpu")->base());
}

static MACHINE_CONFIG_START( photon, photon_state )

    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu",I8080, 1780000)
    MCFG_CPU_PROGRAM_MAP(pk8000_mem)
    MCFG_CPU_IO_MAP(pk8000_io)
    MCFG_CPU_VBLANK_INT("screen", pk8000_interrupt)

    MCFG_MACHINE_RESET(pk8000)

    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(50)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
    MCFG_SCREEN_SIZE(256+32, 192+32)
    MCFG_SCREEN_VISIBLE_AREA(0, 256+32-1, 0, 192+32-1)
    MCFG_SCREEN_UPDATE_STATIC(photon)
    MCFG_PALETTE_LENGTH(16)
    MCFG_PALETTE_INIT(pk8000)

    MCFG_VIDEO_START(photon)

    MCFG_I8255_ADD( "ppi8255_1", pk8000_ppi8255_interface_1 )
    MCFG_I8255_ADD( "ppi8255_2", pk8000_ppi8255_interface_2 )

    /* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/*
    Dump was made using custom adaptor, hence it is marked as bad dump.
    The real machine has following roms:
    0000...07FFh - ROM1 (D41)
    0800...0FFFh - ROM2 (D42)
    1000...17FFh - ROM3 (D43)
    1800...1FFFh - not populated (D44)
    2000...27FFh - ROM5 (D45)
    2800...2FFFh - ROM6 (D46)
    3000...37FFh - ROM7 (D47)
    3800...3FFFh - ROM8 (D48)
*/
ROM_START( phtetris )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "foton_tetris.bin", 0x10000, 0x4000, BAD_DUMP CRC(a8af10bb) SHA1(5e2ea9a5d38399cbe156638eea73a3d25c442f77) )
ROM_END

/*
    Dump was made using custom adaptor, hence it is marked as bad dump.
    The real machine has following roms:

    0000...07FFh - ROM1 (D41)
    0800...0FFFh - ROM2 (D42)
    1000...17FFh - ROM3 (D43)
*/
ROM_START( phpython )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "foton_piton.bin", 0x10000, 0x1800, BAD_DUMP CRC(4eac925a) SHA1(26f9a18c7aed31b7daacdc003bafb60a5e6d6300) )
ROM_END


/*
    Dump was made using custom adaptor, hence it is marked as bad dump.
*/
ROM_START( phklad )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "klad.bin", 0x10000, 0x4000, BAD_DUMP CRC(49cc7d65) SHA1(d966cfc1d973a533df8044a71fad37f7177da554) )
ROM_END

GAME( 19??,  phtetris, 0,      photon, photon, driver_device, 0, ROT0, "<unknown>", "Tetris (Photon System)", 0 )
GAME( 1989?, phpython,  0,     photon, photon, driver_device, 0, ROT0, "<unknown>", "Python (Photon System)", 0 )
GAME( 19??,  phklad,   0,      photon, photon, driver_device, 0, ROT0, "<unknown>", "Klad / Labyrinth (Photon System)", 0 )
