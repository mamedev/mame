/* Cubo CD32 (Original hardware by Commodore, additional hardware and games by CD Express, Milan, Italy)

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

   Everything - This is a skeleton driver.
      Add full AGA (68020 based systems, Amiga 1200 / CD32) support to Amiga driver
      Add CD Controller emulation for CD32.
      ... work from there


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
#include "sound/custom.h"
#include "includes/amiga.h"
#include "includes/cubocd32.h"
#include "machine/6526cia.h"

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
			memory_install_write32_handler(space, 0x000000, 0x1fffff, 0, 0, SMH_BANK1);
		else
			/* overlay enabled, map Amiga system ROM on 0x000000 */
			memory_install_write32_handler(space, 0x000000, 0x1fffff, 0, 0, SMH_UNMAP);
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

static UINT8 cd32_cia_0_porta_r(const device_config *device)
{
	return input_port_read(device->machine, "CIA0PORTA");
}

static void cd32_cia_0_porta_w(const device_config *device, UINT8 data)
{
	/* bit 1 = cd audio mute */
	sndti_set_output_gain(SOUND_CDDA, 0, 0, ( data & 1 ) ? 0.0 : 1.0 );

	/* bit 2 = Power Led on Amiga */
	set_led_status(0, (data & 2) ? 0 : 1);
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

static UINT8 cd32_cia_0_portb_r(const device_config *device)
{
	/* parallel port */
	logerror("%s:CIA0_portb_r\n", cpuexec_describe_context(device->machine));
	return 0xff;
}

static void cd32_cia_0_portb_w(const device_config *device, UINT8 data)
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
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xaaff, IP_ACTIVE_HIGH, IPT_UNUSED )

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
INPUT_PORTS_END

/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const custom_sound_interface amiga_custom_interface =
{
	amiga_sh_start
};



static const cia6526_interface cia_0_intf =
{
	amiga_cia_0_irq,									/* irq_func */
	0,													/* tod_clock */
	{
		{ cd32_cia_0_porta_r, cd32_cia_0_porta_w },		/* port A */
		{ cd32_cia_0_portb_r, cd32_cia_0_portb_w }		/* port B */
	}
};

static const cia6526_interface cia_1_intf =
{
	amiga_cia_1_irq,									/* irq_func */
	0,													/* tod_clock */
	{
		{ NULL, NULL },									/* port A */
		{ NULL, NULL }									/* port B */
	}
};

static MACHINE_DRIVER_START( cd32 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M68EC020, AMIGA_68EC020_PAL_CLOCK) /* 14.3 Mhz */
	MDRV_CPU_PROGRAM_MAP(cd32_map,0)

	MDRV_MACHINE_RESET(amiga)
	MDRV_NVRAM_HANDLER(cd32)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.997)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512*2, 312)
	MDRV_SCREEN_VISIBLE_AREA((129-8)*2, (449+8-1)*2, 44-8, 300+8-1)

	MDRV_PALETTE_LENGTH(4096)
	MDRV_PALETTE_INIT(amiga)

	MDRV_VIDEO_START(amiga)
	MDRV_VIDEO_UPDATE(amiga)

	/* sound hardware */
    MDRV_SPEAKER_STANDARD_STEREO("left", "right")

    MDRV_SOUND_ADD("amiga", CUSTOM, 3579545)
    MDRV_SOUND_CONFIG(amiga_custom_interface)
    MDRV_SOUND_ROUTE(0, "left", 0.25)
    MDRV_SOUND_ROUTE(1, "right", 0.25)
    MDRV_SOUND_ROUTE(2, "right", 0.25)
    MDRV_SOUND_ROUTE(3, "left", 0.25)

    MDRV_SOUND_ADD( "cdda", CDDA, 0 )
	MDRV_SOUND_ROUTE( 0, "left", 0.50 )
	MDRV_SOUND_ROUTE( 1, "right", 0.50 )

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
	DISK_IMAGE_READONLY( "cndypuzl", 0, SHA1(21093753a1875dc4fb97f23232ed3d8776b48c06) MD5(dcb6cdd7d81d5468c1290a3baf4265cb) )
ROM_END

ROM_START( haremchl )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "haremchl", 0, SHA1(4d5df2b64b376e8d0574100110f3471d3190765c) MD5(00adbd944c05747e9445446306f904be) )
ROM_END

ROM_START( lsrquiz )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lsrquiz", 0, SHA1(4250c94ab77504104005229b28f24cfabe7c9e48) MD5(12a94f573fe5d218db510166b86fdda5) )
ROM_END

ROM_START( lsrquiz2 )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lsrquiz2", 0, SHA1(ea92df0e53bf36bb86d99ad19fca21c6129e61d7) MD5(df63c32aca815f6c97889e08c10b77bc) )
ROM_END

ROM_START( mgprem11 )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "mgprem11", 0, SHA1(a8a32d10148ba968b57b8186fdf4d4cd378fb0d5) MD5(e0e4d00c6f981c19a1d20d5e7090b0db) )
ROM_END

ROM_START( lasstixx )
	CD32_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "lasstixx", 0, SHA1(29c2525d43a696da54648caffac9952cec85fd37) MD5(6242dd8a3c0b15ef9eafb930b7a7e87f) )
ROM_END

/***************************************************************************************************/

static DRIVER_INIT( cd32 )
{
	static const amiga_machine_interface cubocd32_intf =
	{
		AGA_CHIP_RAM_MASK,
		NULL, NULL, NULL,
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
