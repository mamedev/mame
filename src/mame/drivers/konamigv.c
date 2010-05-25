/***************************************************************************

  Konami GV System (aka "Baby Phoenix") - Arcade PSX Hardware
  ===========================================================
  Driver by R. Belmont & smf


Known Dumps
-----------

Game       Description                  Mother Board   Code       Version       Date   Time

pbball96   Powerful Pro Baseball '96    GV999          GV017   JAPAN 1.03   96.05.27  18:00
hyperath   Hyper Athlete                ZV610          GV021   JAPAN 1.00   96.06.09  19:00
susume     Susume! Taisen Puzzle-Dama   ZV610          GV027   JAPAN 1.20   96.03.04  12:00
btchamp    Beat the Champ               GV999          GV053   UAA01        ?
kdeadeye   Dead Eye                     GV999          GV054   UA01         ?
weddingr   Wedding Rhapsody             ?              GX624   JAA          97.05.29   9:12
tokimosh   Tokimeki Memorial Oshiete    ?              GE755   JAA          97.08.06  11:52
           Your Heart
tokimosp   Tokimeki Memorial Oshiete    ?              GE756   JAB          97.09.27   9:10
           Your Heart Seal version PLUS
nagano98   Winter Olypmics in Nagano 98 GV999          GX720   EAA01 1.03   98.01.08  10:45
simpbowl   Simpsons Bowling             ?              GQ829   UAA          ?

PCB Layouts
-----------

ZV610 PWB301331
|---------------------------------------|
|   000180       056602      LM324   CN8|
|CN2                                    |
|                                       |
|      999A01.7E                     CN6|
|                         CXD2922BQ     |
|      10E                KM416V256BLT-7|
|                                       |
|J     12E                              |
|A CXD2923AR     058239                 |
|M                                      |
|M                     CXD8530BQ        |
|A   D482445LGW-A70            93CF96-2 |
|               CXD8514Q               S|
|    D482445LGW-A70                    C|
|                      67.7376MHz      S|
|         53.693175MHz                 I|
|                                 32MHz |
|    93C46   KM48V514BJ-6  KM48V514BJ-6 |
|            KM48V514BJ-6  KM48V514BJ-6 |
|    CN5      CN3                001231 |
|---------------------------------------|

GV999 PWB301949A
|---------------------------------------|
|                056602      LM324   CN8|
|CN2                                    |
|TEST_SW                                |
|      999A01.7E                     CN6|
|MC44200                  CXD2925Q      |
|      9E                 TC51V4260BJ-80|
|                                       |
|J     12E                              |
|A               058239                 |
|M  53.693175MHz                        |
|M                     CXD8530CQ        |
|A                             93CF96-2 |
|      CXD8561Q                        S|
|              KM4132G271Q-12          C|
|                      67.7376MHz      S|
|         53.693175MHz                 I|
|                                 32MHz |
|    93C46   KM48V514BJ-6  KM48V514BJ-6 |
|            KM48V514BJ-6  KM48V514BJ-6 |
|    CN5      CN3                001231 |
|---------------------------------------|

Notes:
      - Simpsons Bowling uses an unknown board revision with 4 x 16M FLASHROMs & a trackball.

      - 000180 is used for driving the RGB output. It's a very thin piece of very brittle ceramic
        containing a circuit, a LM1203 chip, some smt transistors/caps/resistors etc (let's just say
        placing this thing on the edge of the PCB wasn't a good design choice!)
        On GV999, it has been replaced by three transistors and a MC44200.

      - 056602 seems to be some sort of A/D converter (another ceramic thing containing caps/resistors/transistors and a chip)

      - CXD2922 and CXD2925 are SPU's.

      - The BIOS on ZV610 and GV999 is identical. It's a 4M MASK ROM, compatible with 27C040.

      - The CD contains one MODE 1 data track and several Redbook audio tracks which are streamed to the speaker via CN8.

      - The ZV and GV PCB's are virtually identical aside from some minor component shuffling and the RGB output mechanism.
        However note that the GPU revision is different between the two boards and so are some of the other Sony IC's.

      - CN8 used to connect redbook audio output from CD drive to PCB.

      - CN6 used to connect power to CD drive.

      - CN2 used for extra speaker connection for stereo output.

      - CN3, CN5 used for connecting 3rd and 4th player controls.

      - 001231, 058239 are PALCE16V8H PALs.

      - 10E, 12E are unpopulated positions for 16M TSOP56 FLASHROMs (10E is 9E on GV999).

      - If the CD is swapped to another GV game, the game will boot but will stop with an error '25C MBAD' (the EEPROM is 25C)
        So the games can not be swapped by simply exchanging CDs because the EEPROM will not re-init itself if the CDROM is swapped.
        This appears to be some form of mild protection to stop operators swapping CD's.
        However it is possible to swap games to another PCB by exchanging the CD _AND_ the EEPROM from another PCB which belongs
        to that same game. It won't work with a blank EEPROM or a different games' EEPROM.
*/

#include "emu.h"
#include "cdrom.h"
#include "cpu/mips/psx.h"
#include "includes/psx.h"
#include "machine/eeprom.h"
#include "machine/intelfsh.h"
#include "machine/am53cf96.h"
#include "sound/psx.h"
#include "sound/cdda.h"

/* static variables */

static UINT8 sector_buffer[ 4096 ];
static UINT32 flash_address;

static UINT16 trackball_prev[ 2 ];
static UINT32 trackball_data[ 2 ];
static UINT16 btc_trackball_prev[ 4 ];
static UINT32 btc_trackball_data[ 4 ];

/* EEPROM handlers */

static WRITE32_DEVICE_HANDLER( eeprom_w )
{
	eeprom_write_bit(device, (data&0x01) ? 1 : 0);
	eeprom_set_clock_line(device, (data&0x04) ? ASSERT_LINE : CLEAR_LINE);
	eeprom_set_cs_line(device, (data&0x02) ? CLEAR_LINE : ASSERT_LINE);
}

static WRITE32_HANDLER( mb89371_w )
{
}

static READ32_HANDLER( mb89371_r )
{
	return 0xffffffff;
}

static ADDRESS_MAP_START( konamigv_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM	AM_SHARE("share1") AM_BASE(&g_p_n_psxram) AM_SIZE(&g_n_psxramsize) /* ram */
	AM_RANGE(0x1f000000, 0x1f00001f) AM_READWRITE(am53cf96_r, am53cf96_w)
	AM_RANGE(0x1f100000, 0x1f100003) AM_READ_PORT("P1")
	AM_RANGE(0x1f100004, 0x1f100007) AM_READ_PORT("P2")
	AM_RANGE(0x1f100008, 0x1f10000b) AM_READ_PORT("P3_P4")
	AM_RANGE(0x1f180000, 0x1f180003) AM_DEVWRITE("eeprom", eeprom_w)
	AM_RANGE(0x1f680000, 0x1f68001f) AM_READWRITE(mb89371_r, mb89371_w)
	AM_RANGE(0x1f780000, 0x1f780003) AM_WRITENOP /* watchdog? */
	AM_RANGE(0x1f800000, 0x1f8003ff) AM_RAM /* scratchpad */
	AM_RANGE(0x1f801000, 0x1f801007) AM_WRITENOP
	AM_RANGE(0x1f801008, 0x1f80100b) AM_RAM /* ?? */
	AM_RANGE(0x1f80100c, 0x1f80102f) AM_WRITENOP
	AM_RANGE(0x1f801010, 0x1f801013) AM_READNOP
	AM_RANGE(0x1f801014, 0x1f801017) AM_DEVREAD("spu", psx_spu_delay_r)
	AM_RANGE(0x1f801040, 0x1f80105f) AM_READWRITE(psx_sio_r, psx_sio_w)
	AM_RANGE(0x1f801060, 0x1f80106f) AM_WRITENOP
	AM_RANGE(0x1f801070, 0x1f801077) AM_READWRITE(psx_irq_r, psx_irq_w)
	AM_RANGE(0x1f801080, 0x1f8010ff) AM_READWRITE(psx_dma_r, psx_dma_w)
	AM_RANGE(0x1f801100, 0x1f80112f) AM_READWRITE(psx_counter_r, psx_counter_w)
	AM_RANGE(0x1f801810, 0x1f801817) AM_READWRITE(psx_gpu_r, psx_gpu_w)
	AM_RANGE(0x1f801820, 0x1f801827) AM_READWRITE(psx_mdec_r, psx_mdec_w)
	AM_RANGE(0x1f801c00, 0x1f801dff) AM_DEVREADWRITE("spu", psx_spu_r, psx_spu_w)
	AM_RANGE(0x1f802020, 0x1f802033) AM_RAM /* ?? */
	AM_RANGE(0x1f802040, 0x1f802043) AM_WRITENOP
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x801fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa01fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END

/* SCSI */

static void scsi_dma_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( sector_buffer ) / 4 )
		{
			n_this = sizeof( sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		if( n_this < 2048 / 4 )
		{
			/* non-READ commands */
			am53cf96_read_data( n_this * 4, sector_buffer );
		}
		else
		{
			/* assume normal 2048 byte data for now */
			am53cf96_read_data( 2048, sector_buffer );
			n_this = 2048 / 4;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			g_p_n_psxram[ n_address / 4 ] =
				( sector_buffer[ i + 0 ] << 0 ) |
				( sector_buffer[ i + 1 ] << 8 ) |
				( sector_buffer[ i + 2 ] << 16 ) |
				( sector_buffer[ i + 3 ] << 24 );
			n_address += 4;
			i += 4;
			n_this--;
		}
	}
}

static void scsi_dma_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( sector_buffer ) / 4 )
		{
			n_this = sizeof( sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			sector_buffer[ i + 0 ] = ( g_p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
			sector_buffer[ i + 1 ] = ( g_p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
			sector_buffer[ i + 2 ] = ( g_p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
			sector_buffer[ i + 3 ] = ( g_p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			n_address += 4;
			i += 4;
			n_this--;
		}

		am53cf96_write_data( n_this * 4, sector_buffer );
	}
}

static void scsi_irq(running_machine *machine)
{
	psx_irq_set(machine, 0x400);
}

static const SCSIConfigTable dev_table =
{
	1, /* 1 SCSI device */
	{
		{ SCSI_ID_4, "cdrom", SCSI_DEVICE_CDROM } /* SCSI ID 4, using CHD 0, and it's a CD-ROM */
	}
};

static const struct AM53CF96interface scsi_intf =
{
	&dev_table,		/* SCSI device table */
	&scsi_irq,		/* command completion IRQ */
};

static void konamigv_exit(running_machine *machine)
{
	am53cf96_exit(&scsi_intf);
}

static DRIVER_INIT( konamigv )
{
	psx_driver_init(machine);

	/* init the scsi controller and hook up it's DMA */
	am53cf96_init(machine, &scsi_intf);
	add_exit_callback(machine, konamigv_exit);
	psx_dma_install_read_handler(5, scsi_dma_read);
	psx_dma_install_write_handler(5, scsi_dma_write);
}

static MACHINE_START( konamigv )
{
	state_save_register_global_array(machine, sector_buffer);
	state_save_register_global(machine, flash_address);
	state_save_register_global_array(machine, trackball_prev);
	state_save_register_global_array(machine, trackball_data);
	state_save_register_global_array(machine, btc_trackball_prev);
	state_save_register_global_array(machine, btc_trackball_data);
}

static MACHINE_RESET( konamigv )
{
	psx_machine_init(machine);

	/* also hook up CDDA audio to the CD-ROM drive */
	cdda_set_cdrom(devtag_get_device(machine, "cdda"), am53cf96_get_device(SCSI_ID_4));
}

static void spu_irq(running_device *device, UINT32 data)
{
	psx_irq_set(device->machine, data);
}

static const psx_spu_interface konamigv_psxspu_interface =
{
	&g_p_n_psxram,
	spu_irq,
	psx_dma_install_read_handler,
	psx_dma_install_write_handler
};

static MACHINE_DRIVER_START( konamigv )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",  PSXCPU, XTAL_67_7376MHz )
	MDRV_CPU_PROGRAM_MAP( konamigv_map)
	MDRV_CPU_VBLANK_INT("screen", psx_vblank)

	MDRV_MACHINE_START( konamigv )
	MDRV_MACHINE_RESET( konamigv )

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE( 60 )
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC( 0 ))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE( 1024, 512 )
	MDRV_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )

	MDRV_PALETTE_LENGTH( 65536 )

	MDRV_PALETTE_INIT( psx )
	MDRV_VIDEO_START( psx_type2 )
	MDRV_VIDEO_UPDATE( psx )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD( "spu", PSXSPU, 0 )
	MDRV_SOUND_CONFIG( konamigv_psxspu_interface )
	MDRV_SOUND_ROUTE( 0, "lspeaker", 0.75 )
	MDRV_SOUND_ROUTE( 1, "rspeaker", 0.75 )

	MDRV_SOUND_ADD( "cdda", CDDA, 0 )
	MDRV_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MDRV_SOUND_ROUTE( 1, "rspeaker", 1.0 )
MACHINE_DRIVER_END

static INPUT_PORTS_START( konamigv )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00001000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE( "eeprom", eeprom_read_bit )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Simpsons Bowling */

static NVRAM_HANDLER( simpbowl )
{
	nvram_handler_intelflash( machine, 0, file, read_or_write );
	nvram_handler_intelflash( machine, 1, file, read_or_write );
	nvram_handler_intelflash( machine, 2, file, read_or_write );
	nvram_handler_intelflash( machine, 3, file, read_or_write );
}

static READ32_HANDLER( flash_r )
{
	int reg = offset*2;
	//int shift = 0;

	if (mem_mask == 0xffff0000)
	{
		reg++;
		//shift = 16;
	}

	if (reg == 4)	// set odd address
	{
		flash_address |= 1;
	}

	if (reg == 0)
	{
		int chip = (flash_address >= 0x200000) ? 2 : 0;
		int ret;

		ret = intelflash_read(chip, flash_address & 0x1fffff) & 0xff;
		ret |= intelflash_read(chip+1, flash_address & 0x1fffff)<<8;
		flash_address++;

		return ret;
	}
	return 0;
}

static WRITE32_HANDLER( flash_w )
{
	int reg = offset*2;
	int chip;

	if (mem_mask == 0xffff0000)
	{
		reg++;
		data>>= 16;
	}

	switch (reg)
	{
		case 0:
			chip = (flash_address >= 0x200000) ? 2 : 0;
			intelflash_write(chip, flash_address & 0x1fffff, data&0xff);
			intelflash_write(chip+1, flash_address & 0x1fffff, (data>>8)&0xff);
			break;

		case 1:
			flash_address = 0;
			flash_address |= (data<<1);
			break;
		case 2:
			flash_address &= 0xff00ff;
			flash_address |= (data<<8);
			break;
		case 3:
			flash_address &= 0x00ffff;
			flash_address |= (data<<15);
			break;
	}
}

static READ32_HANDLER( trackball_r )
{
	if( offset == 0 && mem_mask == 0x0000ffff )
	{
		int axis;
		UINT16 diff;
		UINT16 value;
		static const char *const axisnames[] = { "TRACK0_X", "TRACK0_Y" };

		for( axis = 0; axis < 2; axis++ )
		{
			value = input_port_read(space->machine, axisnames[axis]);
			diff = value - trackball_prev[ axis ];
			trackball_prev[ axis ] = value;
			trackball_data[ axis ] = ( ( diff & 0xf00 ) << 16 ) | ( ( diff & 0xff ) << 8 );
		}
	}
	return trackball_data[ offset ];
}

static READ32_HANDLER( unknown_r )
{
	return 0xffffffff;
}

static DRIVER_INIT( simpbowl )
{
	intelflash_init( machine, 0, FLASH_FUJITSU_29F016A, NULL );
	intelflash_init( machine, 1, FLASH_FUJITSU_29F016A, NULL );
	intelflash_init( machine, 2, FLASH_FUJITSU_29F016A, NULL );
	intelflash_init( machine, 3, FLASH_FUJITSU_29F016A, NULL );

	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f680080, 0x1f68008f, 0, 0, flash_r, flash_w );
	memory_install_read32_handler     ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f6800c0, 0x1f6800c7, 0, 0, trackball_r );
	memory_install_read32_handler     ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f6800c8, 0x1f6800cb, 0, 0, unknown_r ); /* ?? */

	DRIVER_INIT_CALL(konamigv);
}

static MACHINE_DRIVER_START( simpbowl )
	MDRV_IMPORT_FROM( konamigv )
	MDRV_NVRAM_HANDLER( simpbowl )
MACHINE_DRIVER_END

static INPUT_PORTS_START( simpbowl )
	PORT_INCLUDE( konamigv )

	PORT_START("TRACK0_X")
	PORT_BIT( 0xfff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xfff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

INPUT_PORTS_END

/* Beat the Champ */

static READ32_HANDLER( btcflash_r )
{
	if (mem_mask == 0x0000ffff)
	{
		return intelflash_read(0, offset*2);
	}
	else if (mem_mask == 0xffff0000)
	{
		return intelflash_read(0, (offset*2)+1)<<16;
	}

	return 0;
}

static WRITE32_HANDLER( btcflash_w )
{
	if (mem_mask == 0x0000ffff)
	{
		intelflash_write(0, offset*2, data&0xffff);
	}
	else if (mem_mask == 0xffff0000)
	{
		intelflash_write(0, (offset*2)+1, (data>>16)&0xffff);
	}
}

static READ32_HANDLER( btc_trackball_r )
{
//  mame_printf_debug( "r %08x %08x %08x\n", cpu_get_pc(space->cpu), offset, mem_mask );

	if( offset == 1 && mem_mask == 0xffff0000 )
	{
		int axis;
		UINT16 diff;
		UINT16 value;
		static const char *const axisnames[] = { "TRACK0_X", "TRACK0_Y", "TRACK1_X", "TRACK1_Y" };

		for( axis = 0; axis < 4; axis++ )
		{
			value = input_port_read(space->machine, axisnames[axis]);
			diff = value - btc_trackball_prev[ axis ];
			btc_trackball_prev[ axis ] = value;
			btc_trackball_data[ axis ] = ( ( diff & 0xf00 ) << 16 ) | ( ( diff & 0xff ) << 8 );
		}
	}
	return btc_trackball_data[ offset ] | ( btc_trackball_data[ offset + 2 ] >> 8 );
}

static WRITE32_HANDLER( btc_trackball_w )
{
//  mame_printf_debug( "w %08x %08x %08x %08x\n", cpu_get_pc(space->cpu), offset, data, mem_mask );
}

static NVRAM_HANDLER( btchamp )
{
	nvram_handler_intelflash( machine, 0, file, read_or_write );
}

static DRIVER_INIT( btchamp )
{
	intelflash_init( machine, 0, FLASH_SHARP_LH28F400, NULL );

	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f680080, 0x1f68008f, 0, 0, btc_trackball_r, btc_trackball_w );
	memory_nop_write                  ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f6800e0, 0x1f6800e3, 0, 0 );
	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f380000, 0x1f3fffff, 0, 0, btcflash_r, btcflash_w );

	DRIVER_INIT_CALL(konamigv);
}

static MACHINE_DRIVER_START( btchamp )
	MDRV_IMPORT_FROM( konamigv )
	MDRV_NVRAM_HANDLER( btchamp )
MACHINE_DRIVER_END

static INPUT_PORTS_START( btchamp )
	PORT_INCLUDE( konamigv )

	PORT_START("TRACK0_X")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("TRACK1_X")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("TRACK1_Y")
	PORT_BIT( 0x7ff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)
INPUT_PORTS_END

/* Tokimeki Memorial games - have a mouse and printer and who knows what else */

static READ32_HANDLER( tokimeki_serial_r )
{
	// bits checked: 0x80 and 0x20 for periodic status (800b6968 and 800b69e0 in tokimosh)
	// 0x08 for reading the serial device (8005e624)

	return 0xffffffff;
}

static WRITE32_HANDLER( tokimeki_serial_w )
{
	/*
        serial EEPROM-like device here: when mem_mask == 0x000000ff only,

        0x40 = chip enable
        0x20 = clock
        0x10 = data

        tokimosh sends 6 bits: 110100 then reads 8 bits.
        readback is bit 3 (0x08) of serial_r
        This happens starting around 8005e580.
    */

}

static DRIVER_INIT( tokimosh )
{
	memory_install_read32_handler ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f680080, 0x1f680083, 0, 0, tokimeki_serial_r );
	memory_install_write32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f680090, 0x1f680093, 0, 0, tokimeki_serial_w );

	DRIVER_INIT_CALL(konamigv);
}

/*
Dead Eye

Top board:
    PWB402610
    Xilinx XC3020A
    Xilinx 1718DPC
    74F244N (2 of these)
    LVT245SS (2 of theses)

CD:
    P/N 002715
    054
    UA
    A01
*/

static WRITE32_HANDLER( kdeadeye_0_w )
{
}

static DRIVER_INIT( kdeadeye )
{
	intelflash_init( machine, 0, FLASH_SHARP_LH28F400, NULL );

	memory_install_read_port  ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f680080, 0x1f680083, 0, 0, "GUNX1" );
	memory_install_read_port  ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f680090, 0x1f680093, 0, 0, "GUNY1" );
	memory_install_read_port  ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f6800a0, 0x1f6800a3, 0, 0, "GUNX2" );
	memory_install_read_port  ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f6800b0, 0x1f6800b3, 0, 0, "GUNY2" );
	memory_install_read_port  ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f6800c0, 0x1f6800c3, 0, 0, "BUTTONS" );
	memory_install_write32_handler    ( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f6800e0, 0x1f6800e3, 0, 0, kdeadeye_0_w );
	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f380000, 0x1f3fffff, 0, 0, btcflash_r, btcflash_w );

	DRIVER_INIT_CALL(konamigv);
}

static MACHINE_DRIVER_START( kdeadeye )
	MDRV_IMPORT_FROM( konamigv )
	MDRV_NVRAM_HANDLER( btchamp )
MACHINE_DRIVER_END

static INPUT_PORTS_START( kdeadeye )
	PORT_INCLUDE( konamigv )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0000007f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0000007f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GUNX1")
	PORT_BIT( 0xffff, 0x0100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x004c, 0x01bb ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 1 )

	PORT_START("GUNY1")
	PORT_BIT( 0xffff, 0x0077, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 1 )

	PORT_START("GUNX2")
	PORT_BIT( 0xffff, 0x0100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x004c, 0x01bb ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 2 )

	PORT_START("GUNY2")
	PORT_BIT( 0xffff, 0x0077, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 2 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

#define GV_BIOS	\
	ROM_REGION32_LE( 0x080000, "user1", 0 )	\
	ROM_LOAD( "999a01.7e",   0x0000000, 0x080000, CRC(ad498d2d) SHA1(02a82a2fe1fba0404517c3602324bfa64e23e478) )

ROM_START( konamigv )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", ROMREGION_ERASE00 ) /* default eeprom */
ROM_END

ROM_START( susume )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "susume.25c",   0x000000, 0x000080, CRC(52f17df7) SHA1(b8ad7787b0692713439d7d9bebfa0c801c806006) )
	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "gv027j1", 0, SHA1(ad474c60ee68202324a31cc106f2054dc465f4b7) )
ROM_END

ROM_START( hyperath )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "hyperath.25c", 0x000000, 0x000080, CRC(20a8c435) SHA1(a0f203a999757fba68b391c525ac4b9684a57ba9) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "hyperath", 0, SHA1(8638dcb27d53a30ea66c09349f6ff0b78910bf79) )
ROM_END

ROM_START( pbball96 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "pbball96.25c", 0x000000, 0x000080, CRC(405a7fc9) SHA1(e2d978f49748ba3c4a425188abcd3d272ec23907) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "pbball96", 0, SHA1(6d47624e86f97c0b8ea4ebb84cc5446aa4f11fcf) )
ROM_END

ROM_START( weddingr )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "weddingr.25c", 0x000000, 0x000080, CRC(b90509a0) SHA1(41510a0ceded81dcb26a70eba97636d38d3742c3) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "weddingr", 0, SHA1(798686b410cb43b60d6ae91507d034db2db1b185) )
ROM_END

ROM_START( simpbowl )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "simpbowl.25c", 0x000000, 0x000080, CRC(2c61050c) SHA1(16ae7f81cbe841c429c5c7326cf83e87db1782bf) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "simpbowl", 0, SHA1(476f100b6c420343e16a18575bbedf1f15fbd274) )
ROM_END

ROM_START( btchamp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "btchmp.25c", 0x000000, 0x000080, CRC(6d02ea54) SHA1(d3babf481fd89db3aec17f589d0d3d999a2aa6e1) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "btchamp", 0, SHA1(c9c858e9034826e1a12c3c003dd068a49a3577e1) )
ROM_END

ROM_START( kdeadeye )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "kdeadeye.25c", 0x000000, 0x000080, CRC(3935d2df) SHA1(cbb855c475269077803c380dbc3621e522efe51e) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "kdeadeye", 0, SHA1(55a86ad568069f3799125b47253d579793276fab) )
ROM_END

ROM_START( nagano98 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "nagano98.25c",  0x000000, 0x000080, CRC(b64b7451) SHA1(a77a37e0cc580934d1e7e05d523bae0acd2c1480) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "nagano98", 0, SHA1(efe09a23dfbb957574b8989b5672af1ab5e27640) )
ROM_END

ROM_START( tokimosh )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
        ROM_LOAD( "tokimosh.25c", 0x000000, 0x000080, CRC(e57b833f) SHA1(f18a0974a6be69dc179706643aab837ff61c2738) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "755jaa01", 0, SHA1(f7f1545658b430f60edccf448b2832dab9984b19) )
ROM_END

ROM_START( tokimosp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) /* default eeprom */
	ROM_LOAD( "tokimosp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "756jab01", 0, SHA1(ef76fc27e43f7e4cff16bf88458e7ee327c11ca3) )
ROM_END

/* BIOS placeholder */
GAME( 1995, konamigv, 0, konamigv, konamigv, konamigv, ROT0, "Konami", "Baby Phoenix/GV System", GAME_IS_BIOS_ROOT )

GAME( 1996, pbball96, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Powerful Baseball '96 (GV017 Japan 1.03)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, hyperath, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Hyper Athlete (GV021 Japan 1.00)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, susume,   konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Susume! Taisen Puzzle-Dama (GV027 Japan 1.20)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, btchamp,  konamigv, btchamp,  btchamp,  btchamp,  ROT0, "Konami", "Beat the Champ (GV053 UAA01)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, kdeadeye, konamigv, kdeadeye, kdeadeye, kdeadeye, ROT0, "Konami", "Dead Eye (GV054 UA01)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, weddingr, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Wedding Rhapsody (GX624 JAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, tokimosh, konamigv, konamigv, konamigv, tokimosh, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart (GE755 JAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1997, tokimosp, konamigv, konamigv, konamigv, tokimosh, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal version PLUS (GE756 JAB)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1998, nagano98, konamigv, konamigv, konamigv, konamigv, ROT0, "Konami", "Nagano Winter Olympics '98 (GX720 EAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 2000, simpbowl, konamigv, simpbowl, simpbowl, simpbowl, ROT0, "Konami", "Simpsons Bowling (GQ829 UAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
