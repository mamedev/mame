/* Tecmo System
 Driver by Farfetch & David Haywood

can't do anything with this, its protected and expects to read back 68k code :-(

T.Slanina 20040530 :
 - preliminary gfx decode,
 - Angel Eyes - patched interrupt level1 vector
 - EEPROM r/w
 - txt layer
 - added hacks to see more gfx (press Z or X)
 - palette (press X in angel eyes to see 'color bar chack'(!))
 - watchdog (?) simulation
*/


/*

Deroon DeroDero
(c)1996 Tecmo
Tecmo System Board

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

EPROMs:
t001upau.bin - Main program (even) (27c4001)
t002upal.bin - Main program (odd)  (27c4001)

t003uz1.bin - Sound program (27c2001)

Mask ROMs:
t101uah1.j66 - Graphics (23c16000 SOP)
t102ual1.j67 |
t103ubl1.j08 |
t104ucl1.j68 /

t201ubb1.w61 - Graphics (23c8000)
t202ubc1.w62 /

t301ubd1.w63 - Graphics (23c8000)

t401uya1.w16 - YMZ280B Samples (23c16000)

t501uad1.w01 - M6295 Samples (23c4001)

*/

/*

Touki Denshou -Angel Eyes-
(c)1996 Tecmo
Tecmo System Board

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

EPROMs:
aeprge-2.pal - Main program (even) (27c4001)
aeprgo-2.pau - Main program (odd)  (27c4001)

aesprg-2.z1 - Sound program (27c1001)

Mask ROMs:
ae100h.ah1 - Graphics (23c32000/16000 SOP)
ae100.al1  |
ae101h.bh1 |
ae101.bl1  |
ae102h.ch1 |
ae102.cl1  |
ae104.el1  |
ae105.fl1  |
ae106.gl1  /

ae200w74.ba1 - Graphics (23c16000)
ae201w75.bb1 |
ae202w76.bc1 /

ae300w36.bd1 - Graphics (23c4000)

ae400t23.ya1 - YMZ280B Samples (23c16000)
ae401t24.yb1 /

ae500w07.ad1 - M6295 Samples (23c4001)

*/

#include "driver.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "cpu/m68000/m68k.h"
#include "sound/okim6295.h"
#include "sound/262intf.h"
#include "sound/ymz280b.h"

static int gametype;

static tilemap *txt_tilemap;
static TILE_GET_INFO( get_tile_info )
{

	SET_TILE_INFO(
			0,
			videoram16[2*tile_index+1],
			videoram16[2*tile_index]&0xf,
			0);
}



static UINT16* protram;

static UINT8 device[0x10000];
static UINT32 device_read_ptr = 0;
static UINT32 device_write_ptr = 0;

enum DEV_STATUS
{
	DS_CMD,
	DS_WRITE,
	DS_WRITE_ACK,
	DS_READ,
	DS_READ_ACK
};

static UINT8 device_status = DS_CMD;

static READ16_HANDLER(reg_f80000_r)
{
	UINT16 dt;
	// 0 means ok, no errors. -1 means error
	if (device_status == DS_CMD)
		return 0;

	if (device_status == DS_WRITE_ACK)
	{
		// Notice, this is the maximum. I think the device lets 68k just writes 4/5 bytes,
		// they contain "LUNA". Then, it starts sending to the 68k a bunch of stuff, including
		// 68k code.
		if (device_write_ptr == 0x10000)
		{
//          logerror("DEVICE write finished\n");
			device_status = DS_READ_ACK;
			device_write_ptr = 0;
			device_read_ptr = 0;
		}
		else
			device_status = DS_WRITE;

		return 0;
	}

	if (device_status == DS_WRITE)
	{
		logerror("UNEXPECTED read DS_WRITE (write ptr %x)\n", device_write_ptr);
		return 0;
	}


	if (device_status == DS_READ_ACK)
	{
//      logerror("Read ACK\n");
		device_status = DS_READ;
		return 0;
	}

	dt = device[device_read_ptr];

//  logerror("DEVICE read %x: %x (at %x)\n", device_read_ptr, dt, cpunum_get_pc(0));

	device_read_ptr++;
	device_read_ptr &= 0xFFFF;

	device_status = DS_READ_ACK;

	return dt<<8;
}

// Write 0x13
// Read something (acknowledge? If -1, write -1 and restart)
// Write data
// Read value (!=1 is ok)

static READ16_HANDLER(reg_b80000_r)
{
	if (ACCESSING_MSB)
	{
		// Bit 7: 0 = ready to write
		// Bit 6: 0 = ready to read
		return 0;
	}

	return 0;
}

static WRITE16_HANDLER(reg_e80000_w)
{
	// Only LSB
	data >>= 8;

	if (device_status == DS_CMD)
	{
		switch (data)
		{
		case 0x13:
//          logerror("DEVICE mode WRITE (cmd 0x13)\n");
			device_status = DS_WRITE;
			device_write_ptr = 0;
			break;
		}

		return;
	}

	// @@@ Should skip the writes while in read mode?
	if (device_status == DS_READ || device_status == DS_READ_ACK)
	{
//      logerror("EEPROM write %x: %x\n", device_write_ptr, data);
		return;
	}

	device[device_write_ptr] = (UINT8)data;
	device_write_ptr++;
	device_status = DS_WRITE_ACK;

}

static READ16_HANDLER( eeprom_r )
{
	 return ((EEPROM_read_bit() & 0x01) << 11);
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x200000, 0x20ffff) AM_READ(MRA16_RAM)
	AM_RANGE(0x210000, 0x210001) AM_READ(MRA16_RAM)
	AM_RANGE(0x300000, 0x3013ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x400000, 0x4013ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x500000, 0x5013ff) AM_READ(MRA16_RAM)
	AM_RANGE(0x700000, 0x703fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x880000, 0x880001) AM_READ(input_port_0_word_r)
	AM_RANGE(0x880002, 0x880007) AM_READ(input_port_1_word_r) /* test */
	AM_RANGE(0x900000, 0x907fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x980000, 0x980fff) AM_READ(MRA16_RAM)
	AM_RANGE(0xb80000, 0xb80001) AM_READ(reg_b80000_r)
	AM_RANGE(0xd00000, 0xd80003) AM_READ(MRA16_RAM)
	AM_RANGE(0xd80000, 0xd80001) AM_READ(eeprom_r)
	AM_RANGE(0xf00000, 0xf00001) AM_READ(MRA16_RAM)
	AM_RANGE(0xf80000, 0xf80001) AM_READ(reg_f80000_r)

ADDRESS_MAP_END

static WRITE16_HANDLER( eeprom_w )
{
	if ( ACCESSING_MSB )
	{
		EEPROM_write_bit(data & 0x0800);
		EEPROM_set_cs_line((data & 0x0200) ? CLEAR_LINE : ASSERT_LINE );
		EEPROM_set_clock_line((data & 0x0400) ? CLEAR_LINE: ASSERT_LINE );
	}
}

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x200000, 0x20ffff) AM_WRITE(MWA16_RAM) AM_BASE(&protram)
	AM_RANGE(0x300000, 0x3013ff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x400000, 0x4013ff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x500000, 0x5013ff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x700000, 0x703fff) AM_WRITE(MWA16_RAM) AM_BASE(&videoram16)
	AM_RANGE(0x800000, 0x80ffff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x900000, 0x907fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x980000, 0x980fff) AM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE(&paletteram16)

	AM_RANGE(0x880000, 0x88002f) AM_WRITE(MWA16_RAM )
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(eeprom_w	)
	AM_RANGE(0xa80000, 0xa80005) AM_WRITE(MWA16_RAM	)
	AM_RANGE(0xb00000, 0xb00005) AM_WRITE(MWA16_RAM	)
	AM_RANGE(0xb80000, 0xb80005) AM_WRITE(MWA16_RAM	)
	AM_RANGE(0xc00000, 0xc00005) AM_WRITE(MWA16_RAM	)
	AM_RANGE(0xc80000, 0xc80005) AM_WRITE(MWA16_RAM	)
	AM_RANGE(0xe00000, 0xe00001) AM_WRITE(MWA16_RAM )
	AM_RANGE(0xe80000, 0xe80001) AM_WRITE(reg_e80000_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( deroon )
	PORT_START
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT(  0x0002, IP_ACTIVE_HIGH,IPT_UNKNOWN )

	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static const gfx_layout gfxlayout =
{
   8,8,
   RGN_FRAC(1,1),
   4,
   { 0,1,2,3 },
   { 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
   { 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8},
   8*8*4
};

static const gfx_layout gfxlayout2 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
	  8*8*4*1+0*4, 8*8*4*1+1*4, 8*8*4*1+2*4, 8*8*4*1+3*4, 8*8*4*1+4*4, 8*8*4*1+5*4,8*8*4*1+6*4, 8*8*4*1+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  8*8*4*2+0*32, 8*8*4*2+1*32, 8*8*4*2+2*32, 8*8*4*2+3*32, 8*8*4*2+4*32, 8*8*4*2+5*32, 8*8*4*2+6*32, 8*8*4*2+7*32 },
	128*8
};


static GFXDECODE_START( tecmosys )
	GFXDECODE_ENTRY( REGION_GFX2, 0, gfxlayout,   0x40*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, gfxlayout2,   0, 16 )
GFXDECODE_END



static WRITE8_HANDLER( deroon_bankswitch_w )
{
	memory_set_bankptr( 1, memory_region(REGION_CPU2) + ((data-2) & 0x0f) * 0x4000 + 0x10000 );
}

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK1)
	AM_RANGE(0xe000, 0xf7ff) AM_READ(MRA8_RAM)

ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xe000, 0xf7ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END



static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(YMF262_status_0_r)
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_r)
	//AM_RANGE(0x60, 0x60) AM_READ(YMZ280B_status_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(YMF262_register_A_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YMF262_data_A_0_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(YMF262_register_B_0_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(YMF262_data_B_0_w)

	AM_RANGE(0x10, 0x10) AM_WRITE(OKIM6295_data_0_w)
	AM_RANGE(0x20, 0x20) AM_NOP

	AM_RANGE(0x30, 0x30) AM_WRITE(deroon_bankswitch_w)

	//AM_RANGE(0x50, 0x50) AM_WRITE(to_main_cpu_latch_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(MWA8_NOP)

	AM_RANGE(0x60, 0x60) AM_WRITE(YMZ280B_register_0_w)
	AM_RANGE(0x61, 0x61) AM_WRITE(YMZ280B_data_0_w)
ADDRESS_MAP_END



static VIDEO_START(deroon)
{
	txt_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,8,8,32*2,32*2);
	tilemap_set_transparent_pen(txt_tilemap,0);
}




static VIDEO_UPDATE(deroon)
{



#if 0
/* simulate sound commands writes here ... to test OPL3 emulator */
	int j;
	char buf[64];
	static int command_data=0;

	if (input_code_pressed_once(KEYCODE_Q))
	{
		command_data++;
	}
	if (input_code_pressed_once(KEYCODE_A))
	{
		command_data--;
	}
	command_data &= 0xff;

	sprintf(buf,"keys: Q,A and C\ncommand code: %2x", command_data);
	ui_draw_text(buf,10,20);

	if (input_code_pressed_once(KEYCODE_C))
	{
		soundlatch_w(0,command_data);
		cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);
		popmessage("command write=%2x",command_data);
	}
#endif




	// bg color , to see text in deroon
	if(!gametype)
			palette_set_color(screen->machine,0x800,MAKE_RGB(0x80,0x80,0x80));
	else
			palette_set_color(screen->machine,0x800,MAKE_RGB(0x0,0x0,0x0));

	fillbitmap(bitmap,0x800,cliprect);

	tilemap_mark_all_tiles_dirty(txt_tilemap);
	tilemap_draw(bitmap,cliprect,txt_tilemap,0,0);


//hacks

	if(input_code_pressed_once(KEYCODE_Z))
	{
		if(!gametype)
			cpunum_set_reg(0, M68K_PC, 0x23ae8); /* deroon */
		else
		{
			UINT16 *ROM = (UINT16 *)memory_region(REGION_CPU1);
			ROM[0x3aaa/2] = 0x4e73; // rte (trap 0)
			cpunum_set_reg(0, M68K_PC, 0x182a0); /* angel eyes */
		}
	}

	if(input_code_pressed_once(KEYCODE_X))
	{
		if(gametype)
		{
			UINT16 *ROM = (UINT16 *)memory_region(REGION_CPU1);
			ROM[0x3aaa/2] = 0x4e73; // rte (trap 0)
			cpunum_set_reg(0, M68K_PC, 0x17d2a); /* angel eyes */
		}
	}
	return 0;
}

/*
>>> Richard wrote:
> Here's the sound info (I got it playing in M1, I
> didn't bother "porting" it since the main game doesn't
> even boot).
>
> memory map:
> 0000-7fff: fixed program ROM
> 8000-bfff: banked ROM
> e000-f7ff: work RAM
>
> I/O ports:

> 0-3: YMF262 OPL3
> 0x10: OKIM6295
> 0x30: bank select, in 0x4000 byte units based at the
> start of the ROM (so 2 = 0x8000).
> 0x40: latch from 68000
> 0x50: latch to 68000
> 0x60/0x61: YMZ280B
>
> IRQ from YMF262 goes to Z80 IRQ.
>
> NMI is asserted when the 68000 writes a command.
>
> Z80 clock appears to be 8 MHz (music slows down in
> "intense" sections if it's 4 MHz, and the crystals are
> all in the area of 16 MHz).
>
> The YMZ280B samples for both games may be misdumped,
> deroon has lots of "bad" noises but tkdensho only has
> a few.
*/


static void sound_irq(int irq)
{
	/* IRQ */
	cpunum_set_input_line(Machine, 1,0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YMF262interface ymf262_interface =
{
	sound_irq		/* irq */
};


static const struct YMZ280Binterface ymz280b_interface =
{
	REGION_SOUND1,
	0	/* irq */
};

static MACHINE_DRIVER_START( deroon )
	MDRV_CPU_ADD(M68000, 16000000/8) /* the /8 divider is here only for OPL3 testing */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT("main", irq1_line_hold)

	MDRV_CPU_ADD(Z80, 16000000/2 )	/* 8 MHz ??? */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(readport,writeport)

	MDRV_GFXDECODE(tecmosys)

	MDRV_NVRAM_HANDLER(93C46)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 42*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x800+1)

	MDRV_VIDEO_START(deroon)
	MDRV_VIDEO_UPDATE(deroon)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMF262, 14318180)
	MDRV_SOUND_CONFIG(ymf262_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
	MDRV_SOUND_ROUTE(2, "left", 1.0)
	MDRV_SOUND_ROUTE(3, "right", 1.0)

	MDRV_SOUND_ADD(OKIM6295, 14318180/2048*132)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)

	MDRV_SOUND_ADD(YMZ280B, 16900000)
	MDRV_SOUND_CONFIG(ymz280b_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.30)
	MDRV_SOUND_ROUTE(1, "right", 0.30)
MACHINE_DRIVER_END


ROM_START( deroon )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) // Main Program
	ROM_LOAD16_BYTE( "t001upau.bin", 0x00000, 0x80000, CRC(14b92c18) SHA1(b47b8c828222a3f7c0fe9271899bd38171d972fb) )
	ROM_LOAD16_BYTE( "t002upal.bin", 0x00001, 0x80000, CRC(0fb05c68) SHA1(5140592e15414770fb46d5ac9ba8f76e3d4ab323) )

	ROM_REGION( 0x048000, REGION_CPU2, 0 ) // Sound Porgram
	ROM_LOAD( "t003uz1.bin", 0x000000, 0x008000, CRC(8bdfafa0) SHA1(c0cf3eb7a65d967958fe2aace171859b0faf7753) )
	ROM_CONTINUE(            0x010000, 0x038000 ) /* banked part */

	ROM_REGION( 0x800000, REGION_GFX1, 0 ) // Graphics - mostly (maybe all?) not tile based
	ROM_LOAD( "t101uah1.j66", 0x000000, 0x200000, CRC(74baf845) SHA1(935d2954ba227a894542be492654a2750198e1bc) )
	ROM_LOAD( "t102ual1.j67", 0x200000, 0x200000, CRC(1a02c4a3) SHA1(5155eeaef009fc9a9f258e3e54ca2a7f78242df5) )
	ROM_LOAD( "t103ubl1.j08", 0x400000, 0x200000, CRC(75431ec5) SHA1(c03e724c15e1fe7a0a385332f849e9ac9d149887) )
	ROM_LOAD( "t104ucl1.j68", 0x600000, 0x200000, CRC(66eb611a) SHA1(64435d35677fea3c06fdb03c670f3f63ee481c02) )

	ROM_REGION( 0x100000, REGION_GFX2, ROMREGION_DISPOSE ) // 8x8 4bpp tiles
	ROM_LOAD( "t301ubd1.w63", 0x000000, 0x100000, CRC(8b026177) SHA1(3887856bdaec4d9d3669fe3bc958ef186fbe9adb) )

	ROM_REGION( 0x300000, REGION_GFX3, ROMREGION_DISPOSE ) // 16x16 4bpp tiles
	ROM_LOAD( "t201ubb1.w61", 0x000000, 0x100000, CRC(d5a087ac) SHA1(5098160ce7719d93e3edae05f6edd317d4c61f0d) )
	ROM_LOAD( "t202ubc1.w62", 0x100000, 0x100000, CRC(f051dae1) SHA1(f5677c07fe644b3838657370f0309fb09244c619) )


	ROM_REGION( 0x200000, REGION_SOUND1, 0 ) // YMZ280B Samples
	ROM_LOAD( "t401uya1.w16", 0x000000, 0x200000, CRC(92111992) SHA1(ae27e11ae76dec0b9892ad32e1a8bf6ab11f2e6c) )

	ROM_REGION( 0x080000, REGION_SOUND2, 0 ) // M6295 Samples
	ROM_LOAD( "t501uad1.w01", 0x000000, 0x080000, CRC(2fbcfe27) SHA1(f25c830322423f0959a36955edb563a6150f2142) )
ROM_END

ROM_START( tkdensho )
	ROM_REGION( 0x600000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "aeprge-2.pal", 0x00000, 0x80000, CRC(25e453d6) SHA1(9c84e2af42eff5cc9b14c1759d5bab42fa7bb663) )
	ROM_LOAD16_BYTE( "aeprgo-2.pau", 0x00001, 0x80000, CRC(22d59510) SHA1(5ade482d6ab9a22df2ee8337458c22cfa9045c73) )

	ROM_REGION( 0x038000, REGION_CPU2, 0 ) // Sound Porgram
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x008000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_CONTINUE(            0x010000, 0x018000 ) /* banked part */

	ROM_REGION( 0x1e00000, REGION_GFX1, 0 ) // Graphics - mostly (maybe all?) not tile based
	ROM_LOAD( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD( "ae100.al1",     0x0400000, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD( "ae101.bl1",     0x0c00000, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD( "ae102.cl1",     0x1200000, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD( "ae104.el1",     0x1400000, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD( "ae105.fl1",     0x1800000, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD( "ae106.gl1",     0x1c00000, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, REGION_GFX2, ROMREGION_DISPOSE ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x300000, REGION_GFX3, ROMREGION_DISPOSE ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )
	ROM_LOAD( "ae201w75.bb1",  0x100000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )
	ROM_LOAD( "ae202w76.bc1",  0x200000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x080000, REGION_SOUND2, 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x000000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

static TIMER_CALLBACK( reset_callback )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_RESET, PULSE_LINE);
}


static DRIVER_INIT( deroon )
{
	UINT16 *ROM = (UINT16 *)memory_region(REGION_CPU1);
	ROM[0x39C2/2] = 0x0001;
	ROM[0x0448/2] = 0x4E71;
	ROM[0x044A/2] = 0x4E71;
	ROM[0x04bc/2] = 0x0000;
	ROM[0x302c/2] = 0x60a4;
	timer_set(ATTOTIME_IN_SEC(2), NULL,0,reset_callback);
	gametype=0;
}

static DRIVER_INIT( tkdensho )
{
	UINT16 *ROM = (UINT16 *)memory_region(REGION_CPU1);
	ROM[0x222c/2] = 0x4E71;
	ROM[0x222c/2] = 0x4E71;

	/* interrupt vector */
	ROM[0x64/2] = 0x0000;
	ROM[0x66/2] = 0x22c4;

	/* protection ? */
	ROM[0x3a3c/2] = 0x4E71;
	ROM[0x3a84/2] = 0x4E71;

	ROM[0x1759a/2] = 0x4E71; //trap 0
	ROM[0x04822/2] = 0x4E71;
	ROM[0x04862/2] = 0x4E71;

	timer_set(ATTOTIME_IN_SEC(2), NULL,0,reset_callback);
	gametype=1;

}

GAME( 1996, deroon,      0, deroon, deroon, deroon,     ROT0, "Tecmo", "Deroon DeroDero", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1996, tkdensho,    0, deroon, deroon, tkdensho,   ROT0, "Tecmo", "Touki Denshou -Angel Eyes-", GAME_NOT_WORKING | GAME_NO_SOUND )

