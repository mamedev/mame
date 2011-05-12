/*******************************************************************************************

Super Pool 99 (c) 1998 Electronic Projects

driver by David Haywood and Angelo Salese

A rip-off of other famous gambling games (Namely C.M.C. and Funworld games)

Notes:
-At start-up a Test Hardware with RAM NG msg pops up.Do a soft reset and keep pressed start
 and service 1 buttons until the RAM init msg appears.
-There's a "(c) 1993 Hi-Tech software" string in the program roms,this is actually the Z80 C
 compiler used for doing this game.
-On the 0.31 version program rom,starting from 0xacf6 the following string is present:
 "EP V31 PIPPO BELLISSIMA TI AMO" (translated: "beautiful Pippo I love you",the beautiful word
 is actually used as a female gender adverb). While the pippo name is a common joke for naming
 printfs variables for newbie programmers,I'll let others interpret what it means the rest...
-vcarn: tries to write a nop at 0x744, if it succeeds it glitches the text when you win. This means that
 the ROM can be written only at the first 0x100 bytes on this HW.

TODO:
-spool99: EEPROM barely hooked up,enough to let this to boot but it doesn't save settings at the
 moment;
-spool99: An "input BAD" msg pops up at start-up,probably because there are inputs not yet hooked up.
-spool99: Visible area might be wrong (384x240),but this doesn't even have a cross-hatch test,so I
 need a snapshot from the original thing...

============================================================================================

Year    1998
Manufacturer Electronic Projects
Revision N. 0.36

CPU

1x Z84C00BC6 (main)
1x U6295 (equivalent to OKI6295)(sound)
1x LM358 (sound)
1x TDA2003 (sound)
1x ispLSI1032E-70LJ (not dumped)
1x oscillator 14.318180
1x green resonator 1000

ROMs

1x M27C512 (main)
1x W27E040 (gfx)
1x W27E02 (sound)
1x ST93C46 (SOIC8) (not dumped)

Note

1x JAMMA edge connector
1x trimmer (volume)
1x pushbutton
1x red led
1x battery

--- 2nd pcb

Year    1998
Manufacturer Electronic Projects
Revision N. 0.31

CPU

1x Z84C00BC6 (main)
1x U6295 (equivalent to OKI6295)(sound)
1x LM358 (sound)
1x TDA2003 (sound)
1x ispLSI1032E-70LJ (not dumped)
1x oscillator 14.318180
1x green resonator 1000

ROMs

1x NM27C010Q (main)
1x 27C040 (gfx)
1x 27C020 (sound)
1x ST93C46 (SOIC8) (not dumped)

Note

1x JAMMA edge connector
1x trimmer (volume)
1x pushbutton
1x red led
1x battery

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "machine/eeprom.h"

class spool99_state : public driver_device
{
public:
	spool99_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_main;
	tilemap_t *m_sc0_tilemap;
	UINT8 *m_cram;
	UINT8 *m_vram;
};

static TILE_GET_INFO( get_spool99_tile_info )
{
	spool99_state *state = machine.driver_data<spool99_state>();
	int code = ((state->m_vram[tile_index*2+1]<<8) | (state->m_vram[tile_index*2+0]));
	int color = state->m_cram[tile_index*2+0];

	SET_TILE_INFO(
			0,
			code & 0x3fff,
			color & 0x1f,
			0);
}

static VIDEO_START(spool99)
{
	spool99_state *state = machine.driver_data<spool99_state>();

	state->m_sc0_tilemap = tilemap_create(machine, get_spool99_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
}

static SCREEN_UPDATE(spool99)
{
	spool99_state *state = screen->machine().driver_data<spool99_state>();

	tilemap_draw(bitmap,cliprect,state->m_sc0_tilemap,0,0);
	return 0;
}

static WRITE8_HANDLER( spool99_vram_w )
{
	spool99_state *state = space->machine().driver_data<spool99_state>();

	state->m_vram[offset] = data;
	tilemap_mark_tile_dirty(state->m_sc0_tilemap,offset/2);
}

static WRITE8_HANDLER( spool99_cram_w )
{
	spool99_state *state = space->machine().driver_data<spool99_state>();

	state->m_cram[offset] = data;
	tilemap_mark_tile_dirty(state->m_sc0_tilemap,offset/2);
}



static READ8_HANDLER( spool99_io_r )
{
	UINT8 *ROM = space->machine().region("maincpu")->base();

//  if(!(io_switch))
	{
		switch(offset+0xaf00)
		{
			case 0xafd8: return input_port_read(space->machine(),"COIN1");
//          case 0xafd9: return 1;
			case 0xafda: return input_port_read(space->machine(),"COIN2");
			case 0xafdb: return 1;
			case 0xafdc: return input_port_read(space->machine(),"SERVICE1");//attract mode
			case 0xafdd: return input_port_read(space->machine(),"HOLD3");
			case 0xafde: return input_port_read(space->machine(),"HOLD4");
			case 0xafdf: return input_port_read(space->machine(),"HOLD2");
			case 0xafe0: return input_port_read(space->machine(),"HOLD1");
			case 0xafe1: return input_port_read(space->machine(),"HOLD5");
			case 0xafe2: return input_port_read(space->machine(),"START");
			case 0xafe3: return input_port_read(space->machine(),"BET");//system 2
			case 0xafe4: return input_port_read(space->machine(),"SERVICE2");//attract mode
//          case 0xafe5: return 1;
//          case 0xafe6: return 1;
			case 0xafe7: return space->machine().device<eeprom_device>("eeprom")->read_bit();
			case 0xaff8: return space->machine().device<okim6295_device>("oki")->read(*space,0);
		}
	}
//  printf("%04x %d\n",offset+0xaf00,io_switch);

	return ROM[0xaf00+offset];
}


static WRITE8_DEVICE_HANDLER( eeprom_resetline_w )
{
	// reset line asserted: reset.
	eeprom_device *eeprom = downcast<eeprom_device *>(device);
	eeprom->set_cs_line((data & 0x01) ? CLEAR_LINE : ASSERT_LINE );
}

static WRITE8_DEVICE_HANDLER( eeprom_clockline_w )
{
	// clock line asserted: write latch or select next bit to read
	eeprom_device *eeprom = downcast<eeprom_device *>(device);
	eeprom->set_clock_line((data & 0x01) ? ASSERT_LINE : CLEAR_LINE );
}

static WRITE8_DEVICE_HANDLER( eeprom_dataline_w )
{
	// latch the bit
	eeprom_device *eeprom = downcast<eeprom_device *>(device);
	eeprom->write_bit(data & 0x01);
}

static ADDRESS_MAP_START( spool99_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_BASE_MEMBER(spool99_state,m_main)
	AM_RANGE(0x0100, 0xaeff) AM_ROM AM_REGION("maincpu", 0x100) AM_WRITENOP
	AM_RANGE(0xaf00, 0xafff) AM_READ(spool99_io_r)
	AM_RANGE(0xafed, 0xafed) AM_DEVWRITE("eeprom", eeprom_resetline_w )
	AM_RANGE(0xafee, 0xafee) AM_DEVWRITE("eeprom", eeprom_clockline_w )
	AM_RANGE(0xafef, 0xafef) AM_DEVWRITE("eeprom", eeprom_dataline_w )
	AM_RANGE(0xaff8, 0xaff8) AM_DEVWRITE_MODERN("oki", okim6295_device, write)

	AM_RANGE(0xb000, 0xb3ff) AM_RAM_WRITE(paletteram_xxxxBBBBGGGGRRRR_le_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0xb800, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(spool99_vram_w) AM_BASE_MEMBER(spool99_state,m_vram)
	AM_RANGE(0xf000, 0xffff) AM_RAM_WRITE(spool99_cram_w) AM_BASE_MEMBER(spool99_state,m_cram)
ADDRESS_MAP_END

static READ8_HANDLER( vcarn_io_r )
{
	UINT8 *ROM = space->machine().region("maincpu")->base();

//  if(!(io_switch))
	{
		switch(offset+0xa700)
		{
			case 0xa720: return input_port_read(space->machine(),"SERVICE1");//attract mode
			case 0xa722: return input_port_read(space->machine(),"COIN1");
			case 0xa723: return input_port_read(space->machine(),"COIN2");
			case 0xa724: return input_port_read(space->machine(),"SERVICE2");//attract mode
			case 0xa725: return input_port_read(space->machine(),"HOLD3");
			case 0xa726: return input_port_read(space->machine(),"HOLD4");
			case 0xa727: return input_port_read(space->machine(),"HOLD2");
			case 0xa780: return space->machine().device<okim6295_device>("oki")->read(*space,0);
			case 0xa7a0: return input_port_read(space->machine(),"HOLD1");
			case 0xa7a1: return input_port_read(space->machine(),"HOLD5");
			case 0xa7a2: return input_port_read(space->machine(),"START");
			case 0xa7a3: return input_port_read(space->machine(),"BET");//system 2

			case 0xa7a7: return space->machine().device<eeprom_device>("eeprom")->read_bit();

		}
	}
//  printf("%04x\n",offset+0xa700);

	return ROM[0xa700+offset];
}

static ADDRESS_MAP_START( vcarn_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_BASE_MEMBER(spool99_state,m_main)
	AM_RANGE(0x0100, 0xa6ff) AM_ROM AM_REGION("maincpu", 0x100) AM_WRITENOP
	AM_RANGE(0xa700, 0xa7ff) AM_READ(vcarn_io_r)
	AM_RANGE(0xa745, 0xa745) AM_DEVWRITE("eeprom", eeprom_resetline_w )
	AM_RANGE(0xa746, 0xa746) AM_DEVWRITE("eeprom", eeprom_clockline_w )
	AM_RANGE(0xa747, 0xa747) AM_DEVWRITE("eeprom", eeprom_dataline_w )
	AM_RANGE(0xa780, 0xa780) AM_DEVWRITE_MODERN("oki", okim6295_device, write)

	AM_RANGE(0xa800, 0xabff) AM_RAM_WRITE(paletteram_xxxxBBBBGGGGRRRR_le_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0xb000, 0xdfff) AM_RAM
//  AM_RANGE(0xdf00, 0xdfff) AM_READWRITE(vcarn_io_r,vcarn_io_w) AM_BASE(&vcarn_io)
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(spool99_vram_w) AM_BASE_MEMBER(spool99_state,m_vram)
	AM_RANGE(0xf000, 0xffff) AM_RAM_WRITE(spool99_cram_w) AM_BASE_MEMBER(spool99_state,m_cram)
ADDRESS_MAP_END



static const gfx_layout spool99_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 3*8,2*8,1*8,0*8 },
	{ 0,1,2,3,4,5,6,7 },
	{0*32,1*32,2*32,3*32, 4*32,5*32,6*32,7*32 },
	8*32
};

static GFXDECODE_START( spool99 )
	GFXDECODE_ENTRY( "gfx", 0, spool99_layout,   0x00, 0x20  )
GFXDECODE_END



static INPUT_PORTS_START( spool99 )
	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("HOLD1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("HOLD2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("HOLD3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("HOLD4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("HOLD5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static MACHINE_CONFIG_START( spool99, spool99_state )

	MCFG_CPU_ADD("maincpu", Z80, 24000000/8)
	MCFG_CPU_PROGRAM_MAP(spool99_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_GFXDECODE(spool99)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(7*8, 55*8-1, 1*8, 31*8-1) //384x240,raw guess
	MCFG_SCREEN_UPDATE(spool99)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_VIDEO_START(spool99)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( vcarn, spool99 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(vcarn_map)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 31*8-1) //512x240, raw guess

MACHINE_CONFIG_END


ROM_START( spool99 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code
	ROM_LOAD( "v.36.u2", 0x00000, 0x10000, CRC(29527f38) SHA1(bf302f4c6eb53ea55fe1ace7bc9bc7a68ad269e6) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u32.bin", 0x00000, 0x40000, CRC(1b7aa54c) SHA1(87fc4da8d2a85bc3ce00d8f0f03fef0027e8454a) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "u15.bin", 0x000000, 0x80000, CRC(707f062f) SHA1(e237a03192d7ce79509418fd8811ecad14890739) )
ROM_END

ROM_START( spool99a )
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code
	ROM_LOAD( "u2.bin", 0x00000, 0x10000, CRC(488dd1bf) SHA1(7289b639fa56722d1f60d8c4bda566d726f8e00b) ) // first half empty!
	ROM_CONTINUE( 0x00000, 0x10000) // 0x0000 - 0xafff used

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "u32.bin", 0x00000, 0x40000, CRC(1b7aa54c) SHA1(87fc4da8d2a85bc3ce00d8f0f03fef0027e8454a) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "u15.bin", 0x000000, 0x80000, CRC(707f062f) SHA1(e237a03192d7ce79509418fd8811ecad14890739) )
ROM_END

ROM_START( vcarn )
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code
	ROM_LOAD( "3.u2", 0x00000, 0x10000, CRC(e7c33032) SHA1(e769c83b6d2b48e347ad6112b4379f6e16bcc6e0) ) // first half empty!
	ROM_CONTINUE( 0x00000, 0x10000) // 0x0000 - 0xafff used

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "1.u32", 0x00000, 0x80000, CRC(8a0aa6b5) SHA1(dc39cb26607fabdcb3e74a60943cf88456172d09) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "2.u15", 0x000000, 0x80000, CRC(a647f378) SHA1(4c8a49afe8bd63d7e30242fb016fc76b38859ea8) )
ROM_END


static DRIVER_INIT( spool99 )
{
	spool99_state *state = machine.driver_data<spool99_state>();

	UINT8 *ROM = machine.region("maincpu")->base();
//  vram = auto_alloc_array(machine, UINT8, 0x2000);
	memcpy(state->m_main, ROM, 0x100);
}



GAME( 1998, spool99,    0,        spool99,    spool99,    spool99, ROT0,  "Electronic Projects", "Super Pool 99 (Version 0.36)", 0 )
GAME( 1998, spool99a,   spool99,  spool99,    spool99,    spool99, ROT0,  "Electronic Projects", "Super Pool 99 (Version 0.31)", 0 )
GAME( 1998, vcarn,      0,        vcarn,      spool99,    spool99, ROT0,  "Electronic Projects", "Video Carnival 1999 / Super Royal Card (Version 0.11)", 0 ) //MAME screen says '98, PCB screen says '99?
