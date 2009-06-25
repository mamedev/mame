/*******************************************************************************************

Jackpot Cards / Jackpot Pool (c) 1997 Electronic Projects

driver by David Haywood & Angelo Salese

Notes:
-There's a "(c) 1992 HI-TECH Software..Brisbane, QLD Australia" string in the program roms,
 this is actually the m68k C compiler used for doing this game.

TODO:
-Correct NVRAM emulation (and default eeprom too?);
-Doesn't accept coins? (might be related to the eeprom);
-Transparent pen issue on the text layer? (might be a btanb);
-Understand master-slave comms properly;

*******************************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/eeprom.h"
#include "rendlay.h"


static UINT16 *sc0_vram;
static UINT16 *sc1_vram;
static UINT16 *sc2_vram;
static UINT16 *sc3_vram;

static UINT16 *jackpool_io;

static VIDEO_START(jackpool)
{
}

static VIDEO_UPDATE(jackpool)
{
	const device_config *left_screen  = devtag_get_device(screen->machine, "lscreen");
	const device_config *right_screen = devtag_get_device(screen->machine, "rscreen");
	const gfx_element *gfx = screen->machine->gfx[0];
	int count;// = 0x00000/2;

	int y,x;

	if(screen == left_screen)
	{
		count = 0x0000/2;
		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				int tile = (sc1_vram[count] & 0x7fff);
				int attr = (sc1_vram[count+0x800] & 0x1f00)>>8;
				//int t_pen = (sc1_vram[count+0x800] & 0x2000);
				//int colour = tile>>12;
				drawgfx_opaque(bitmap,cliprect,gfx,tile,attr,0,0,x*8,y*8);

				count++;
			}
		}

		count = 0x0000/2;
		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				int tile = (sc0_vram[count] & 0x7fff);
				int attr = (sc0_vram[count+0x800] & 0x1f00)>>8;
				/*might just be sloppy coding,colors are enabled as 0x20-0x3f*/
				int t_pen = (sc0_vram[count+0x800] & 0x2000);
				//int colour = tile>>12;
				drawgfx(bitmap,gfx,tile,attr,0,0,x*8,y*8,cliprect,(t_pen) ? TRANSPARENCY_NONE : TRANSPARENCY_PEN,0);
				count++;
			}
		}
	}

	if(screen == right_screen)
	{
		count = 0x0000/2;
		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				int tile = (sc2_vram[count] & 0x7fff);
				int attr = (sc2_vram[count+0x800] & 0x1f00)>>8;
				//int t_pen = (sc1_vram[count+0x800] & 0x2000);
				//int colour = tile>>12;
				drawgfx_opaque(bitmap,cliprect,gfx,tile,attr,0,0,x*8,y*8);

				count++;
			}
		}

		count = 0x0000/2;
		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				int tile = (sc3_vram[count] & 0x7fff);
				int attr = (sc3_vram[count+0x800] & 0x1f00)>>8;
				/*might just be sloppy coding,colors are enabled as 0x20-0x3f*/
				int t_pen = (sc3_vram[count+0x800] & 0x2000);
				//int colour = tile>>12;
				drawgfx(bitmap,gfx,tile,attr,0,0,x*8,y*8,cliprect,(t_pen) ? TRANSPARENCY_NONE : TRANSPARENCY_PEN,0);
				count++;
			}
		}
	}

	return 0;
}

/*Communication ram*/
static READ16_HANDLER( jackpool_ff_r )
{
	return 0xffff;
}

static READ16_HANDLER( jackpool_io_r )
{
	switch(offset*2)
	{
		case 0x0: return input_port_read(space->machine,"COIN1");
		case 0x8: return input_port_read(space->machine,"SERVICE1");
		case 0xa: return input_port_read(space->machine,"SERVICE2");//probably not a button
		case 0xc: return input_port_read(space->machine,"PAYOUT");
		case 0xe: return input_port_read(space->machine,"START2");
		case 0x10: return input_port_read(space->machine,"HOLD3");
		case 0x12: return input_port_read(space->machine,"HOLD4");
		case 0x14: return input_port_read(space->machine,"HOLD2");
		case 0x16: return input_port_read(space->machine,"HOLD1");
		case 0x18: return input_port_read(space->machine,"HOLD5");
		case 0x1a: return input_port_read(space->machine,"START1");
		case 0x1c: return input_port_read(space->machine,"BET");
//      case 0x2c: eeprom 1 r?
//      case 0x2e: eeprom 2 r?
	}

//  printf("R %02x\n",offset*2);
	return jackpool_io[offset];
}

static WRITE16_HANDLER( jackpool_io_w )
{
//  printf("W %02x %02x\n",offset*2,data);
	COMBINE_DATA(&jackpool_io[offset]);

	#if 0
	if(offset*2 == 0x54)
	{
		printf("Write bit %02x\n",data);
		eeprom_write_bit(data & 1);
	}
	if(offset*2 == 0x52)
	{
		printf("Clock bit %02x\n",data);
		eeprom_set_clock_line((data & 1) ? ASSERT_LINE : CLEAR_LINE );
	}
	if(offset*2 == 0x50)
	{
		printf("chip select bit %02x\n",data);
		eeprom_set_cs_line((data & 1) ? CLEAR_LINE : ASSERT_LINE );
	}
	#endif
}

static ADDRESS_MAP_START( jackpool_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x120000, 0x1200ff) AM_RAM
	AM_RANGE(0x340000, 0x341fff) AM_RAM AM_BASE(&sc0_vram)
	AM_RANGE(0x342000, 0x343fff) AM_RAM AM_BASE(&sc1_vram)
	AM_RANGE(0x344000, 0x345fff) AM_RAM AM_BASE(&sc2_vram)
	AM_RANGE(0x346000, 0x347fff) AM_RAM AM_BASE(&sc3_vram)
	/* are the ones below used? */
	AM_RANGE(0x348000, 0x349fff) AM_RAM
	AM_RANGE(0x34a000, 0x34bfff) AM_RAM
	AM_RANGE(0x34c000, 0x34dfff) AM_RAM
	AM_RANGE(0x34e000, 0x34ffff) AM_RAM

	AM_RANGE(0x360000, 0x3603ff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x380000, 0x380061) AM_READWRITE(jackpool_io_r,jackpool_io_w) AM_BASE(&jackpool_io)//AM_READ(jackpool_io_r)

	AM_RANGE(0x800000, 0x80000f) AM_READ(jackpool_ff_r) AM_WRITENOP
	AM_RANGE(0xa00000, 0xa00001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
ADDRESS_MAP_END


static INPUT_PORTS_START( jackpool )
	PORT_START("COIN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("SERVICE1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("SERVICE2") //toggle this to change game to Jackpot Pool,with different gfxs for cards.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PAYOUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("START1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("START2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("BET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Bet / Cancel") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( jackpool )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,   0x000, 0x20  ) /* sprites */
GFXDECODE_END


/*irq 2 used for communication stuff.3 is just a rte*/
static INTERRUPT_GEN( jackpool_interrupt )
{
	cpu_set_input_line(device, 1, HOLD_LINE);
}


static MACHINE_DRIVER_START( jackpool )
	MDRV_CPU_ADD("maincpu", M68000, 12000000) // ?
	MDRV_CPU_PROGRAM_MAP(jackpool_mem)
	MDRV_CPU_VBLANK_INT("lscreen",jackpool_interrupt)  // ?

	MDRV_GFXDECODE(jackpool)
	MDRV_DEFAULT_LAYOUT(layout_dualhsxs)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_NVRAM_HANDLER(93C46)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(jackpool)
	MDRV_VIDEO_UPDATE(jackpool)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( jackpool )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jpc2", 0x00001, 0x20000,CRC(5aad51ff) SHA1(af504d15c356c241efb6410a5dad09494d693eca) )
	ROM_LOAD16_BYTE( "jpc3", 0x00000, 0x20000,CRC(249c7073) SHA1(e654232d5f454932a108591deacadc9da9fd8055) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "jpc1", 0x00000, 0x40000, CRC(0f1372a1) SHA1(cec8a9bfb03945af4e1e2d2b916b9ded52a8d0bd) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "jpc4", 0x00000, 0x40000,  CRC(b719f138) SHA1(82799cbccab4e39627e48855f6003917602b42c7) )
	ROM_LOAD( "jpc5", 0x40000, 0x40000,  CRC(09661ed9) SHA1(fb298252c95a9040441c12c9d0e9280843d56a0d) )
	ROM_LOAD( "jpc6", 0x80000, 0x40000,  CRC(c3117411) SHA1(8ed044beb1d6ab7ac48595f7d6bf879f1264454a) )
	ROM_LOAD( "jpc7", 0xc0000, 0x40000,  CRC(b1d40623) SHA1(fb76ae6b53474bd4bee19dbce9537da0f2b63ff4) )
ROM_END

GAME( 1997, jackpool, 0, jackpool, jackpool, 0, ROT0, "Electronic Projects", "Jackpot Cards / Jackpot Pool (Italy)",GAME_NOT_WORKING )
