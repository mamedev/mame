/* Cubo CD32 (Original hardware by Commodore, additional hardware and games by CD Express, Milan, Italy)

   Preliminary driver by Mariusz Wojcieszek
   CD-ROM controller by Ernesto Corvi

   This is basically a CD32 (Amiga 68020, AGA based games system) hacked up to run arcade games.
   see http://ninjaw.ifrance.com/cd32/cubo/ for a brief overview.

   Several of the games have Audio tracks, therefore the CRC / SHA1 information you get when
   reading your own CDs may not match those in the driver.  There is currently no 100% accurate
   way to rip the audio data with full sub-track and offset information.

   CD32 Hardware Specs (from Wikipedia, http://en.wikipedia.org/wiki/Amiga_CD32)
    * Main Processor: Motorola 68EC020 at 14.3 MHz
    * System Memory: 2 MB Chip RAM
    * 1 MB ROM with Kickstart ROM 3.1 and integrated cdfs.filesystem
    * 1KB of FlashROM for game saves
    * Graphics/Chipset: AGA Chipset
    * Akiko chip, which handles CD-ROM and can do Chunky to Planar conversion
    * Proprietary (MKE) CD-ROM drive at 2x speed
    * Expansion socket for MPEG cartridge, as well as 3rd party devices such as the SX-1 and SX32 expansion packs.
    * 4 8-bit audio channels (2 for left, 2 for right)
    * Gamepad, Serial port, 2 Gameports, Interfaces for keyboard


   ToDo:
      Inputs


   Known Games:
   Title                | rev. | year
   ----------------------------------------------
   Candy Puzzle         |  1.0 | 1995
   Harem Challenge      |      | 1995
   Laser Quiz           |      | 1995
   Laser Quiz 2 "Italy" |  1.0 | 1995
   Laser Strixx         |      | 1995
   Magic Premium        |  1.1 | 1996
   Laser Quiz France    |  1.0 | 1995
   Odeon Twister        |      | 199x
   Odeon Twister 2      |202.19| 1999


*/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/cdda.h"
#include "includes/amiga.h"
#include "includes/cubocd32.h"
#include "machine/6526cia.h"

static void handle_cd32_joystick_cia(UINT8 pra, UINT8 dra);

static WRITE32_HANDLER( aga_overlay_w )
{
	if (ACCESSING_BITS_16_23)
	{
		data = (data >> 16) & 1;

		/* switch banks as appropriate */
		memory_set_bank(space->machine, 1, data & 1);

		/* swap the write handlers between ROM and bank 1 based on the bit */
		if ((data & 1) == 0)
			/* overlay disabled, map RAM on 0x000000 */
			memory_install_write32_handler(space, 0x000000, 0x1fffff, 0, 0, (write32_space_func)SMH_BANK(1));
		else
			/* overlay enabled, map Amiga system ROM on 0x000000 */
			memory_install_write32_handler(space, 0x000000, 0x1fffff, 0, 0, (write32_space_func)SMH_UNMAP);
	}
}

/*************************************
 *
 *  CIA-A port A access:
 *
 *  PA7 = game port 1, pin 6 (fire)
 *  PA6 = game port 0, pin 6 (fire)
 *  PA5 = /RDY (disk ready)
 *  PA4 = /TK0 (disk track 00)
 *  PA3 = /WPRO (disk write protect)
 *  PA2 = /CHNG (disk change)
 *  PA1 = /LED (LED, 0=bright / audio filter control)
 *  PA0 = MUTE
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( cd32_cia_0_porta_w )
{
	/* bit 1 = cd audio mute */
	sound_set_output_gain(devtag_get_device(device->machine, "cdda"), 0, ( data & 1 ) ? 0.0 : 1.0 );

	/* bit 2 = Power Led on Amiga */
	set_led_status(0, (data & 2) ? 0 : 1);

	handle_cd32_joystick_cia(data, cia_r(device, 2));
}

/*************************************
 *
 *  CIA-A port B access:
 *
 *  PB7 = parallel data 7
 *  PB6 = parallel data 6
 *  PB5 = parallel data 5
 *  PB4 = parallel data 4
 *  PB3 = parallel data 3
 *  PB2 = parallel data 2
 *  PB1 = parallel data 1
 *  PB0 = parallel data 0
 *
 *************************************/

static READ8_DEVICE_HANDLER( cd32_cia_0_portb_r )
{
	/* parallel port */
	logerror("%s:CIA0_portb_r\n", cpuexec_describe_context(device->machine));
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( cd32_cia_0_portb_w )
{
	/* parallel port */
	logerror("%s:CIA0_portb_w(%02x)\n", cpuexec_describe_context(device->machine), data);
}

static ADDRESS_MAP_START( cd32_map, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_RAMBANK(1) AM_BASE(&amiga_chip_ram32) AM_SIZE(&amiga_chip_ram_size)
	AM_RANGE(0x800000, 0x800003) AM_READ_PORT("DIPSW1")
	AM_RANGE(0xb80000, 0xb8003f) AM_READWRITE(amiga_akiko32_r, amiga_akiko32_w)
	AM_RANGE(0xbfa000, 0xbfa003) AM_WRITE(aga_overlay_w)
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE16(amiga_cia_r, amiga_cia_w, 0xffffffff)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE16(amiga_custom_r, amiga_custom_w, 0xffffffff) AM_BASE((UINT32**)&amiga_custom_regs)
	AM_RANGE(0xe00000, 0xe7ffff) AM_ROM AM_REGION("user1", 0x80000)	/* CD32 Extended ROM */
	AM_RANGE(0xa00000, 0xf7ffff) AM_NOP
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("user1", 0x0)		/* Kickstart */
ADDRESS_MAP_END

//int cubo_input_port_val = 0;
//int cubo_input_select = 0;
static UINT16 potgo_value = 0;
static int cd32_shifter[2];

static void cubocd32_potgo_w(running_machine *machine, UINT16 data)
{
	int i;

	potgo_value = potgo_value & 0x5500;
	potgo_value |= data & 0xaa00;

    for (i = 0; i < 8; i += 2)
	{
		UINT16 dir = 0x0200 << i;
		if (data & dir)
		{
			UINT16 d = 0x0100 << i;
			potgo_value &= ~d;
			potgo_value |= data & d;
		}
    }
    for (i = 0; i < 2; i++)
	{
	    UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
	    UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */
	    if ((potgo_value & p5dir) && (potgo_value & p5dat))
		cd32_shifter[i] = 8;
    }

}

static void handle_cd32_joystick_cia(UINT8 pra, UINT8 dra)
{
    static int oldstate[2];
    int i;

    for (i = 0; i < 2; i++)
	{
		UINT8 but = 0x40 << i;
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */
		if (!(potgo_value & p5dir) || !(potgo_value & p5dat))
		{
			if ((dra & but) && (pra & but) != oldstate[i])
			{
				if (!(pra & but))
				{
					cd32_shifter[i]--;
					if (cd32_shifter[i] < 0)
						cd32_shifter[i] = 0;
				}
			}
		}
		oldstate[i] = pra & but;
    }
}

static UINT16 handle_joystick_potgor (running_machine *machine, UINT16 potgor)
{
    int i;

    for (i = 0; i < 2; i++)
	{
		UINT16 p9dir = 0x0800 << (i * 4); /* output enable P9 */
		UINT16 p9dat = 0x0400 << (i * 4); /* data P9 */
		UINT16 p5dir = 0x0200 << (i * 4); /* output enable P5 */
		UINT16 p5dat = 0x0100 << (i * 4); /* data P5 */

	    /* p5 is floating in input-mode */
	    potgor &= ~p5dat;
	    potgor |= potgo_value & p5dat;
	    if (!(potgo_value & p9dir))
			potgor |= p9dat;
	    /* P5 output and 1 -> shift register is kept reset (Blue button) */
	    if ((potgo_value & p5dir) && (potgo_value & p5dat))
			cd32_shifter[i] = 8;
	    /* shift at 1 == return one, >1 = return button states */
	    if (cd32_shifter[i] == 0)
			potgor &= ~p9dat; /* shift at zero == return zero */
		if (i == 0)
			if (cd32_shifter[i] >= 2 && (input_port_read(machine, "IN0") & (1 << (cd32_shifter[i] - 2))))
				potgor &= ~p9dat;
    }
    return potgor;
}

static CUSTOM_INPUT(cubo_input)
{
	return handle_joystick_potgor(field->port->machine, potgo_value) >> 10;
}


static INPUT_PORTS_START( cd32 )
	PORT_START("CIA0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("CIA0PORTB")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("JOY0DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(amiga_joystick_convert, "P1JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY1DAT")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(amiga_joystick_convert, "P2JOY")
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("POTGO")
	PORT_BIT( 0x4400, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(cubo_input, 0)
	PORT_BIT( 0xaaff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1 1" )
	PORT_DIPSETTING(    0x01, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x02, 0x02, "DSW1 2" )
	PORT_DIPSETTING(    0x02, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 3" )
	PORT_DIPSETTING(    0x04, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x08, 0x08, "DSW1 4" )
	PORT_DIPSETTING(    0x08, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x10, 0x10, "DSW1 5" )
	PORT_DIPSETTING(    0x10, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_DIPNAME( 0x20, 0x20, "DSW1 6" )
	PORT_DIPSETTING(    0x20, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x80, "DSW1 8" )
	PORT_DIPSETTING(    0x80, "Reset" )
	PORT_DIPSETTING(    0x00, "Set" )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const cia6526_interface cia_0_intf =
{
	DEVCB_LINE(amiga_cia_0_irq),									/* irq_func */
	DEVCB_NULL,	/* pc_func */
	0,													/* tod_clock */
	{
		{ DEVCB_INPUT_PORT("CIA0PORTA"), DEVCB_HANDLER(cd32_cia_0_porta_w) },		/* port A */
		{ DEVCB_HANDLER(cd32_cia_0_portb_r), DEVCB_HANDLER(cd32_cia_0_portb_w) }		/* port B */
	}
};

static const cia6526_interface cia_1_intf =
{
	DEVCB_LINE(amiga_cia_1_irq),									/* irq_func */
	DEVCB_NULL,	/* pc_func */
	0,													/* tod_clock */
	{
		{ DEVCB_NULL, DEVCB_NULL },									/* port A */
		{ DEVCB_NULL, DEVCB_NULL }									/* port B */
	}
};

static MACHINE_DRIVER_START( cd32 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, AMIGA_68EC020_PAL_CLOCK) /* 14.3 Mhz */
	MDRV_CPU_PROGRAM_MAP(cd32_map)

	MDRV_MACHINE_RESET(amiga)
	MDRV_NVRAM_HANDLER(cd32)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.997)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512*2, 312)
	MDRV_SCREEN_VISIBLE_AREA((129-8)*2, (449+8-1)*2, 44-8, 300+8-1)

	MDRV_PALETTE_LENGTH(4096)
	MDRV_PALETTE_INIT(amiga_aga)

	MDRV_VIDEO_START(amiga_aga)
	MDRV_VIDEO_UPDATE(amiga_aga)

	/* sound hardware */
    MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

    MDRV_SOUND_ADD("amiga", AMIGA, 3579545)
    MDRV_SOUND_ROUTE(0, "lspeaker", 0.25)
    MDRV_SOUND_ROUTE(1, "rspeaker", 0.25)
    MDRV_SOUND_ROUTE(2, "rspeaker", 0.25)
    MDRV_SOUND_ROUTE(3, "lspeaker", 0.25)

    MDRV_SOUND_ADD( "cdda", CDDA, 0 )
	MDRV_SOUND_ROUTE( 0, "lspeaker", 0.50 )
	MDRV_SOUND_ROUTE( 1, "rspeaker", 0.50 )

	/* cia */
	MDRV_CIA8520_ADD("cia_0", AMIGA_68EC020_PAL_CLOCK / 10, cia_0_intf)
	MDRV_CIA8520_ADD("cia_1", AMIGA_68EC020_PAL_CLOCK / 10, cia_1_intf)
MACHINE_DRIVER_END



#define ROM_LOAD16_WORD_BIOS(bios,name,offset,length,hash)     ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1))

#define CD32_BIOS \
	ROM_REGION32_BE(0x100000, "user1", 0 ) \
	ROM_SYSTEM_BIOS(0, "cd32", "Kickstart v3.1 rev 40.60 with CD32 Extended-ROM" ) \
	ROM_LOAD16_WORD_BIOS(0, "391640-03.u6a", 0x000000, 0x100000, CRC(d3837ae4) SHA1(06807db3181637455f4d46582d9972afec8956d9) ) \


ROM_START( cd32 )
	CD32_BIOS
ROM_END

ROM_START( cndypuzl )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "cndypuzl", 0, SHA1(5f41ed3521b3e05d233ac1245b78cb0b118b2b90) )
ROM_END

ROM_START( haremchl )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "haremchl", 0, SHA1(abbab347c0d7c5eef0465d0eee770754a452e874) )
ROM_END

ROM_START( lsrquiz )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lsrquiz", 0, SHA1(41fb6cd0c9d36bd77e9c3db69d36801edc791e96) )
ROM_END

ROM_START( lsrquiz2 )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lsrquiz2", 0, SHA1(78e261df1c548fa492e6cf37a9469640bb8816bf) )
ROM_END

ROM_START( mgprem11 )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "mgprem11", 0, SHA1(7808db33d5949f6c86d12b32bc388c12377e7038) )
ROM_END

ROM_START( lasstixx )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lasstixx", 0, SHA1(b8f6138e1f1840c193e786c56dab03c512f3e21f) )
ROM_END

ROM_START( mgnumber )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "magicnumber", 0, SHA1(60e1fadc42694742d19cc0ac2b6e99e9e33faa3d) )
ROM_END

/***************************************************************************************************/

static DRIVER_INIT( cd32 )
{
	static const amiga_machine_interface cubocd32_intf =
	{
		AGA_CHIP_RAM_MASK,
		NULL, NULL, cubocd32_potgo_w,
		NULL, NULL, NULL,
		NULL, NULL,
		NULL,
		FLAGS_AGA_CHIPSET
	};

	/* configure our Amiga setup */
	amiga_machine_config(machine, &cubocd32_intf);

	/* set up memory */
	memory_configure_bank(machine, 1, 0, 1, amiga_chip_ram32, 0);
	memory_configure_bank(machine, 1, 1, 1, memory_region(machine, "user1"), 0);

	/* intialize akiko */
	amiga_akiko_init(machine);
}

/***************************************************************************************************/

/* BIOS */
GAME( 1993, cd32, 0, cd32, cd32, cd32,   ROT0, "Commodore", "Amiga CD32 Bios", GAME_IS_BIOS_ROOT )

GAME( 1995, cndypuzl, cd32, cd32, cd32, cd32,	   ROT0, "CD Express", "Candy Puzzle (v1.0)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, haremchl, cd32, cd32, cd32, cd32,	   ROT0, "CD Express", "Harem Challenge", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, lsrquiz,  cd32, cd32, cd32, cd32,	   ROT0, "CD Express", "Laser Quiz", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, lsrquiz2, cd32, cd32, cd32, cd32,	   ROT0, "CD Express", "Laser Quiz '2' Italy (v1.0)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, mgprem11, cd32, cd32, cd32, cd32,	   ROT0, "CD Express", "Magic Premium (v1.1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, lasstixx, cd32, cd32, cd32, cd32,	   ROT0, "CD Express", "Laser Strixx", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, mgnumber, cd32, cd32, cd32, cd32,	   ROT0, "CD Express", "Magic Number", GAME_NOT_WORKING|GAME_NO_SOUND )
