/*******************************************************************************************

	Popo Bear (c) 2000 BMC

	preliminary driver by Angelo Salese

	TODO:
	- sprites;
	- I/Os;
	- IRQ generation;
	- Port 0x620000 is quite a mystery, some silly protection?

============================================================================================
Popo Bear - BMC-A00211
(c) 2000 - Bao Ma Technology Co., LTD

|-----------------------------------------|
| DIP2 DIP4  UM3567(YM2413)               |J
| DIP1 DIP3                               |A
|           TA-A-901                      |M
| EN-A-701  EN-A-801  U6295(OKI)          |M
| EN-A-501  EN-A-601                      |A
| EN-A-301  EN-A-401                      |
|                                         |C
|                   AIA90610              |O
|                   BMC-68pin  AIA90423   |N
|                   plcc (68k) BMC-160pin |N
|                                         |E
|                                    OSC  |C
|                                 42.000  |T
|-----------------------------------------|

1 - BMC AIA90423 - 160-Pin ASIC, FGPA, Video?
1 - BMC AIA90610 - 68 Pin CPU (Likely 16 MHz, 68-lead plastic LCC 68000)
1 - UM3567 (YM2413) Sound
1 - U6295 (OKI6295) Sound
1 - 42.000MHz XTAL
4 - 8 Position DIP switches

JAMMA CONNECTOR
Component Side   A   B   Solder Side
           GND   1   1   GND
           GND   2   2   GND
           +5v   3   3   +5v
           +5v   4   4   +5v
                 5   5
          +12v   6   6   +12v
                 7   7
    Coin Meter   8   8
                 9   9
       Speaker  10   10  GND
                11   11
           Red  12   12  Green
          Blue  13   13  Syn
           GND  14   14
          Test  15   15
         Coin1  16   16  Coin2
      1P Start  17   17  2P Start
         1P Up  18   18  2P Up
       1P Down  19   19  2P Down
       1P Left  20   20  2P Left
      1P Right  21   21  2P Right
          1P A  22   22  2P A
          1P B  23   23  2P B
          1P C  24   24  2P C
                25   25
                26   26
           GND  27   27  GND
           GND  28   28  GND
*******************************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"

class popobear_state : public driver_device
{
public:
	popobear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16 *m_vram;
	UINT16 *m_spr;
	required_device<cpu_device> m_maincpu;
};

VIDEO_START(popobear)
{

}

static void draw_layer(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect, UINT8 layer_n)
{
	popobear_state *state = machine.driver_data<popobear_state>();
	UINT8* vram = (UINT8 *)state->m_vram;
	int count;

	count = (0xf0000+layer_n*0x4000);

	for(int y=0;y<32;y++)
	{
		for(int x=0;x<128;x++)
		{
			int tile,xtile,ytile;

			tile = vram[count+0]|(vram[count+1]<<8);
			xtile = tile & 0x7f;
			xtile *= 8;
			ytile = tile >> 7;
			ytile *= 1024*8;

			for(int yi=0;yi<8;yi++)
			{
				for(int xi=0;xi<8;xi+=2)
				{
					UINT8 color;

					color = (vram[(xi+yi*1024)+xtile+ytile] & 0xff);

					if(cliprect.contains(x*8+xi+1, y*8+yi) && color)
						bitmap.pix16(y*8+yi, x*8+xi+1) = machine.pens[color];

					color = (vram[(xi+1+yi*1024)+xtile+ytile] & 0xff);

					if(cliprect.contains(x*8+xi, y*8+yi) && color)
						bitmap.pix16(y*8+yi, x*8+xi) = machine.pens[color];
				}
			}

			count+=2;
		}
	}
}

SCREEN_UPDATE_IND16( popobear )
{
	//popobear_state *state = screen.machine().driver_data<popobear_state>();

	bitmap.fill(0, cliprect);

	if(1)
	{
		draw_layer(screen.machine(),bitmap,cliprect,3);
		draw_layer(screen.machine(),bitmap,cliprect,2);
		draw_layer(screen.machine(),bitmap,cliprect,1);
		draw_layer(screen.machine(),bitmap,cliprect,0);
	}

	if(0)
	{
		popobear_state *state = screen.machine().driver_data<popobear_state>();
	int x,y,count;
		UINT8* spr = (UINT8 *)state->m_spr;
		static int m_test_x,m_test_y,m_start_offs;

	if(screen.machine().input().code_pressed(KEYCODE_Z))
		m_test_x++;

	if(screen.machine().input().code_pressed(KEYCODE_X))
		m_test_x--;

	if(screen.machine().input().code_pressed(KEYCODE_A))
		m_test_y++;

	if(screen.machine().input().code_pressed(KEYCODE_S))
		m_test_y--;

	if(screen.machine().input().code_pressed(KEYCODE_Q))
		m_start_offs+=0x200;

	if(screen.machine().input().code_pressed(KEYCODE_W))
		m_start_offs-=0x200;

	if(screen.machine().input().code_pressed(KEYCODE_E))
		m_start_offs++;

	if(screen.machine().input().code_pressed(KEYCODE_R))
		m_start_offs--;

	popmessage("%d %d %04x",m_test_x,m_test_y,m_start_offs);

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	count = (m_start_offs);

	for(y=0;y<m_test_y;y++)
	{
		for(x=0;x<m_test_x;x+=2)
		{
			UINT32 color;

			color = (spr[count] & 0xff)>>0;

			if(cliprect.contains(x+1, y))
				bitmap.pix16(y, x+1) = screen.machine().pens[color+0x100];

			color = (spr[count+1] & 0xff)>>0;

			if(cliprect.contains(x, y))
				bitmap.pix16(y, x) = screen.machine().pens[color+0x100];

			count+=2;
		}
	}
	}

	return 0;
}

/* ??? */
static READ8_HANDLER( popo_620000_r )
{
	return 9;
}

static WRITE8_HANDLER( popobear_irq_ack_w )
{
	popobear_state *state = space->machine().driver_data<popobear_state>();
	int i;

	for(i=0;i<8;i++)
	{
		if(data & 1 << i)
			device_set_input_line(state->m_maincpu, i, CLEAR_LINE);
	}
}

static ADDRESS_MAP_START( popobear_mem, AS_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM
	AM_RANGE(0x280000, 0x2fffff) AM_RAM AM_BASE_MEMBER(popobear_state, m_spr) // unknown boundaries, 0x2ff800 contains a sprite list
	AM_RANGE(0x300000, 0x3fffff) AM_RAM AM_BASE_MEMBER(popobear_state, m_vram)

	/* Most if not all of these are vregs */
	AM_RANGE(0x480000, 0x480001) AM_NOP //AM_READ(popo_480001_r) AM_WRITE(popo_480001_w)
	AM_RANGE(0x480018, 0x480019) AM_NOP //AM_WRITE(popo_480018_w)
	AM_RANGE(0x48001a, 0x48001b) AM_NOP //AM_WRITE(popo_48001a_w)
	AM_RANGE(0x48001c, 0x48001d) AM_NOP //AM_READ(popo_48001c_r) AM_WRITE(popo_48001c_w)
	AM_RANGE(0x480020, 0x480021) AM_NOP //AM_READ(popo_480020_r) AM_WRITE(popo_480020_w)
	AM_RANGE(0x480028, 0x480029) AM_NOP //AM_WRITE(popo_480028_w)
	AM_RANGE(0x48002c, 0x48002d) AM_NOP //AM_WRITE(popo_48002c_w)
	AM_RANGE(0x480030, 0x480031) AM_WRITE8(popobear_irq_ack_w, 0x00ff)
	AM_RANGE(0x48003a, 0x48003b) AM_NOP //AM_READ(popo_48003a_r) AM_WRITE(popo_48003a_w)

	AM_RANGE(0x480400, 0x4807ff) AM_RAM AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x500000, 0x500001) AM_NOP //AM_READ(popo_500000_r) // watchdog?
	AM_RANGE(0x520000, 0x520001) AM_READ_PORT("DSW4")
	AM_RANGE(0x540000, 0x540001) AM_DEVREADWRITE8_MODERN("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x550000, 0x550003) AM_DEVWRITE8( "ymsnd", ym2413_w, 0x00ff )

//	AM_RANGE(0x600000, 0x600001) AM_WRITE(popo_600000_w)
	AM_RANGE(0x620000, 0x620001) AM_READ8(popo_620000_r,0xff00) //AM_WRITE(popo_620000_w)
	AM_RANGE(0x800000, 0x9fffff) AM_ROM AM_REGION("gfx1", 0)
	AM_RANGE(0xa00000, 0xbfffff) AM_ROM AM_REGION("gfx2", 0) // correct?
ADDRESS_MAP_END

static INPUT_PORTS_START( popobear )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, "Coin_A" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, "Freeplay" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hard ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Arrow" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW2:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DSW3:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW3:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW3:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW3:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW3:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW3:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW3:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW3:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSWA" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END



static TIMER_DEVICE_CALLBACK( popobear_irq )
{
	popobear_state *state = timer.machine().driver_data<popobear_state>();
	int scanline = param;

	/* TODO: one of those is a timer irq, tied with YM2413 */
	if(scanline == 240)
		device_set_input_line(state->m_maincpu, 2, ASSERT_LINE);

	if(scanline == 128)
		device_set_input_line(state->m_maincpu, 3, ASSERT_LINE);

	if(scanline == 32)
		device_set_input_line(state->m_maincpu, 5, ASSERT_LINE);
}

static MACHINE_CONFIG_START( popobear, popobear_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_42MHz/4)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_CPU_PROGRAM_MAP(popobear_mem)
	// levels 2,3,5 look interesting
	//MCFG_CPU_VBLANK_INT("screen",irq3_line_hold)
	MCFG_TIMER_ADD_SCANLINE("scantimer", popobear_irq, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_STATIC(popobear)

//	MCFG_GFXDECODE(popobear)

	MCFG_SCREEN_SIZE(128*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_PALETTE_LENGTH(256*2)

	MCFG_VIDEO_START(popobear)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_42MHz/10)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_42MHz/32, OKIM6295_PIN7_LOW)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( popobear )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "popobear_en-a-301_1.6.u3", 0x000001, 0x20000, CRC(b934adf6) SHA1(93431c7a19af812b549aad35cc1176a81805ffab) )
	ROM_LOAD16_BYTE( "popobear_en-a-401_1.6.u4", 0x000000, 0x20000, CRC(0568af9c) SHA1(920531dbc4bbde2d1db062bd5c48b97dd50b7185) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "popobear_en-a-501.u5", 0x000000, 0x100000, CRC(185901a9) SHA1(7ff82b5751645df53435eaa66edce589684cc5c7) )
	ROM_LOAD16_BYTE( "popobear_en-a-601.u6", 0x000001, 0x100000, CRC(84fa9f3f) SHA1(34dd7873f88b0dae5fb81fe84e82d2b6b49f7332) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "popobear_en-a-701.u7", 0x000000, 0x100000, CRC(45eba6d0) SHA1(0278602ed57ac45040619d590e6cc85e2cfeed31) )
	ROM_LOAD16_BYTE( "popobear_en-a-801.u8", 0x000001, 0x100000, CRC(2760f2e6) SHA1(58af59f486c9df930f7c124f89154f8f389a5bd7) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "popobear_ta-a-901.u9", 0x00000, 0x40000,  CRC(f1e94926) SHA1(f4d6f5b5811d90d0069f6efbb44d725ff0d07e1c) )
ROM_END

GAME( 2000, popobear,    0, popobear,    popobear,    0, ROT0,  "BMC", "PoPo Bear", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
