/***************************************************************************

	Umi de Poker (c) 1997 World Station Co.,LTD

	preliminary driver by Angelo Salese

	TMP68HC000-16 + z80 + YM3812 + OKI6295

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"


class umipoker_state : public driver_device
{
public:
	umipoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vram;
};

static VIDEO_START( umipoker )
{

}

static SCREEN_UPDATE( umipoker )
{
	umipoker_state *state = screen->machine().driver_data<umipoker_state>();
	int x,y;
	int count;
	const gfx_element *gfx = screen->machine().gfx[0];

	count = 0/2;

	for(y=0;y<32;y++)
	{
		for(x=0;x<64;x++)
		{
			int tile = state->m_vram[count*2+0];
			int attr = state->m_vram[count*2+1];
			int color = (attr & 0x1f);

			drawgfx_opaque(bitmap,cliprect,gfx,tile,color,0,0,(x*8),(y*8));

			count++;
		}
	}

	count = 0x3000/2;

	for(y=0;y<32;y++)
	{
		for(x=0;x<64;x++)
		{
			int tile = state->m_vram[count*2+0];
			int attr = state->m_vram[count*2+1];
			int color = (attr & 0x1f);

			drawgfx_transpen(bitmap,cliprect,gfx,tile,color,0,0,(x*8),(y*8),0);

			count++;
		}
	}

	return 0;
}

static READ8_HANDLER( z80_rom_readback_r )
{
	UINT8 *ROM = space->machine().region("audiocpu")->base();

	return ROM[offset];
}

static ADDRESS_MAP_START( umipoker_map, AS_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x400000, 0x403fff) AM_RAM
	AM_RANGE(0x600000, 0x6007ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0x800000, 0x807fff) AM_RAM AM_BASE_MEMBER(umipoker_state, m_vram)
	AM_RANGE(0xc00000, 0xc0ffff) AM_READ8(z80_rom_readback_r,0x00ff)
	AM_RANGE(0xc1f000, 0xc1ffff) AM_RAM // ODD addresses only? probably z80 shared RAM
	// 0xe000xx I/O
ADDRESS_MAP_END



static INPUT_PORTS_START( umipoker )

INPUT_PORTS_END

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4)  },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( umipoker )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,     0, 0x40)
GFXDECODE_END

static MACHINE_START( umipoker )
{
	//umipoker_state *state = machine.driver_data<_umipoker_state>();

}

static MACHINE_RESET( umipoker )
{
	//umipoker_state *state = machine.driver_data<_umipoker_state>();
}

static MACHINE_CONFIG_START( umipoker, umipoker_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,16000000) // TMP68HC000-16
	MCFG_CPU_PROGRAM_MAP(umipoker_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_MACHINE_START(umipoker)
	MCFG_MACHINE_RESET(umipoker)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(umipoker)

	MCFG_GFXDECODE(umipoker)

	MCFG_PALETTE_LENGTH(0x400)

	MCFG_VIDEO_START(umipoker)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( umipoker )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sp0.u61",      0x000000, 0x020000, CRC(866eaa02) SHA1(445afdfe010aad1102219a0dbd3a363a22294b4c) )
	ROM_LOAD16_BYTE( "sp1.u60",      0x000001, 0x020000, CRC(8db08696) SHA1(2854d511a8fd30b023e2a2a00b25413f88205d82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sz.u8",        0x000000, 0x010000, CRC(d874ba1a) SHA1(13c06f3b67694d5d5194023c4f7b75aea8b57129) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sg0.u42",      0x000000, 0x020000, CRC(876f1f4f) SHA1(eca4c397be57812f2c34791736fee7c43925d927) )
	ROM_LOAD( "sg1.u41",      0x020000, 0x020000, CRC(7fcbfb17) SHA1(be2f308a8e8f0941c54125950702ddfbd8538733) )
	ROM_LOAD( "sg2.u40",      0x040000, 0x020000, CRC(eb31649b) SHA1(c0741d85537827e2396e81a1aa3005871dffad78) )
	ROM_LOAD( "sg3.u39",      0x060000, 0x020000, CRC(ebd5f96d) SHA1(968c107ee17f1e92ffc2835e13803347881862f1) )

	ROM_REGION( 0x40000, "oki", 0 )
    ROM_LOAD( "sm.u17",       0x000000, 0x040000, CRC(99503aed) SHA1(011404fad01b3ced708a94143908be3e1d0194d3) ) // first half 1-filled
    ROM_CONTINUE(               0x000000, 0x040000 )
ROM_END

ROM_START( sayukipk )
	ROM_REGION( 0x40000, "maincpu", 0 )
    ROM_LOAD16_BYTE( "slp0-spq.u61", 0x000000, 0x020000, CRC(7fc0f201) SHA1(969170d68278e212dd459744373ed9e704976e45) )
    ROM_LOAD16_BYTE( "slp1-spq.u60", 0x000001, 0x020000, CRC(c8e3547c) SHA1(18bb380a64ed36f45a377b86cbbac892efe879bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
    ROM_LOAD( "slz.u8",       0x000000, 0x010000, CRC(4f32ba1c) SHA1(8f1f8c0995bcd05d19120dd3b64b135908caf759) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
    ROM_LOAD( "slg0.u42",     0x000000, 0x020000, CRC(49ba7ffd) SHA1(3bbb7656eafbd8c91c9054fca056c8fc3002ed13) )
    ROM_LOAD( "slg1.u41",     0x020000, 0x020000, CRC(59b5f399) SHA1(2b999cebcc53b3b8fd38e3034a12434d82b6fad3) )
    ROM_LOAD( "slg2.u40",     0x040000, 0x020000, CRC(fe6cd717) SHA1(65e59d88a30efd0cec642cda54e2bc38196f0231) )
    ROM_LOAD( "slg3.u39",     0x060000, 0x020000, CRC(e99b2906) SHA1(77884d2dae2e7f7cf27103aa8bbd0eaa39628993) )

	ROM_REGION( 0x40000, "oki", 0 )
    ROM_LOAD( "slm.u17",      0x000000, 0x040000, CRC(b50eb70b) SHA1(342fcb307844f4d0a02a85b2c61e73b5e8bacd44) ) // first half 1-filled
    ROM_CONTINUE(               0x000000, 0x040000 )
ROM_END


GAME( 1997, umipoker,  0,   umipoker,  umipoker,  0, ROT0, "World Station Co.,LTD", "Umi de Poker", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1997, sayukipk,  0,   umipoker,  umipoker,  0, ROT0, "World Station Co.,LTD", "Slot Poker Saiyuki", GAME_NOT_WORKING | GAME_NO_SOUND )
