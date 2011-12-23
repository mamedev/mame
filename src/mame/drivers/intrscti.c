/*

Intersecti

No names on the pcb and the only ascii text is in the sound rom "Intersecti". Has a daughterboard with "Serai-Padova-1" on it

Having disassembled the 1911 code roms, I'd guess that there is a rom in the epoxy block.

I've not had a chance to wire up the board yet, but it might be possible to write a trojan to display the rom(s) at $8000


*/

#include "emu.h"
#include "cpu/z80/z80.h"


class intrscti_state : public driver_device
{
public:
	intrscti_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_ram;
};


static READ8_HANDLER( unk_r )
{
	return space->machine().rand();
}

static ADDRESS_MAP_START( intrscti_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x7000, 0x77ff) AM_RAM AM_BASE_MEMBER(intrscti_state, m_ram) // video ram
	AM_RANGE(0x8000, 0x8fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ( unk_r )
	AM_RANGE(0x01, 0x01) AM_READ( unk_r )
ADDRESS_MAP_END


static INPUT_PORTS_START( intrscti )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( intrscti )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static VIDEO_START(intrscti)
{
}

static SCREEN_UPDATE(intrscti)
{
	intrscti_state *state = screen->machine().driver_data<intrscti_state>();
	int y,x;
	int count;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	count = 0;
	for (y=0;y<64;y++)
	{
		for (x=0;x<32;x++)
		{
			int dat;
			dat = state->m_ram[count];
			drawgfx_transpen(bitmap,cliprect,screen->machine().gfx[0],dat/*+0x100*/,0,0,0,x*8,y*8,0);
			count++;
		}
	}


	return 0;
}

static MACHINE_CONFIG_START( intrscti, intrscti_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)		 /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(intrscti_map)
	MCFG_CPU_IO_MAP(readport)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(256, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 512-1)
	MCFG_SCREEN_UPDATE(intrscti)

	MCFG_GFXDECODE(intrscti)
	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(intrscti)
MACHINE_CONFIG_END


ROM_START( intrscti )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1911_1.7g", 0x0000, 0x1000, CRC(8940e6ee) SHA1(50da11a6fab8f31c72c08dbf374268fff18a74e3) )
	ROM_LOAD( "1911_2.8g", 0x1000, 0x1000, CRC(a461031e) SHA1(338c8cd79b98c666edd204150dea65ce4b9ec288) )
	ROM_LOAD( "epoxy_block", 0x8000,0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "ok.13b", 0x00000, 0x800, CRC(cbfa3eba) SHA1(b5a81a4535e7883a3ff8fb4021ddd7dbfaf3c7ae) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "r.5c", 0x0000, 0x1000, CRC(092cc8f2) SHA1(f0c717128e0ac6adc032616a8cafaec88aa0fb90) )
	ROM_LOAD( "g.5b", 0x1000, 0x1000, CRC(2d7cf465) SHA1(70fcb5818f2dfe499bb52501403449660837557d) )
	ROM_LOAD( "b.5a", 0x2000, 0x1000, CRC(8951fb7e) SHA1(c423bf0536e3a09453814172e31b47f9c3c3324c) )
ROM_END

static DRIVER_INIT( intrscti )
{
	UINT8 *cpu = machine.region( "maincpu" )->base();
	int i;
	for (i=0x8000;i<0x8fff;i++)
	{
		cpu[i]=0xc9; // ret
	}
}

GAME( 19??, intrscti,  0,    intrscti, intrscti, intrscti, ROT0, "<unknown>", "Intersecti", GAME_IS_SKELETON )
