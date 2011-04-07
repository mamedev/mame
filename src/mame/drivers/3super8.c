/*

3 Super 8

this dump is bad

TODO:
needs merging with spoker.c

Produttore  ?Italy?
N.revisione
CPU

1x 24mhz osc
2x fpga
1x z840006
1x PIC16c65a-20/p
1x 6295 oki

ROMs

Note

4x 8 dipswitch
1x 4 dispwitch

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"


class _3super8_state : public driver_device
{
public:
	_3super8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_lovram;
	UINT8 *m_hivram;
};


static VIDEO_START(3super8)
{

}

static SCREEN_UPDATE(3super8)
{
	_3super8_state *state = screen->machine().driver_data<_3super8_state>();
	int count = 0x00000;
	int y,x;
	const gfx_element *gfx = screen->machine().gfx[0];

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = ((state->m_lovram[count])+(state->m_hivram[count]<<8)) & 0xfff;
			//int color = (state->m_colorram[x*2]<<8) | (state->m_colorram[(x*2)+1]);

			drawgfx_opaque(bitmap,cliprect,gfx,tile,3,0,0,(x*8),(y*8));

			count++;
		}
	}

	return 0;
}


static ADDRESS_MAP_START( super8_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xf3ff) AM_RAM AM_REGION("maincpu",0)
	AM_RANGE(0xf400, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( super8_io, AS_IO, 8 )
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split1_w ) AM_BASE_GENERIC( paletteram )
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split2_w ) AM_BASE_GENERIC( paletteram2 )

	AM_RANGE(0x5000, 0x5fff) AM_RAM AM_BASE_MEMBER(_3super8_state, m_lovram)
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_BASE_MEMBER(_3super8_state, m_hivram)
ADDRESS_MAP_END

static INPUT_PORTS_START( 3super8 )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+2*8, RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+2*8,RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+2*8 },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 4*8, 5*8, 8*8, 9*8,  12*8, 13*8,},
	16*8
};

static GFXDECODE_START( 3super8 )
	GFXDECODE_ENTRY( "gfx", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END



static MACHINE_CONFIG_START( 3super8, _3super8_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,24000000/4)		 /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(super8_map)
	MCFG_CPU_IO_MAP(super8_io)
//	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-0-1)
	MCFG_SCREEN_UPDATE(3super8)

	MCFG_GFXDECODE(3super8)
	MCFG_PALETTE_LENGTH(0x800)

	MCFG_VIDEO_START(3super8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



// all gfx / sound roms are bad.  they're definitely meant to have different data
//  in each half, and maybe even be twice the size.
//  in all cases the first half is missing (the sample table in the samples rom for example)
//1.bin                                           1ST AND 2ND HALF IDENTICAL
//2.bin                                           1ST AND 2ND HALF IDENTICAL
//3.bin                                           1ST AND 2ND HALF IDENTICAL
//sound.bin                                       1ST AND 2ND HALF IDENTICAL


ROM_START( 3super8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prgrom.bin", 0x00000, 0x20000, CRC(37c85dfe) SHA1(56bd2fb859b17dda1e675a385b6bcd6867ecceb0)  )

	ROM_REGION( 0x1000, "pic", 0 )
	ROM_LOAD( "pic16c65a-20-p", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0xc0000, "gfx", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000, BAD_DUMP CRC(d9d3e21e) SHA1(2f3f07ca427d9f56f0ff143d15d95cbf15255e33) )
	ROM_LOAD( "2.bin", 0x40000, 0x40000, BAD_DUMP CRC(fbb50ab1) SHA1(50a7ef9219c38d59117c510fe6d53fb3ba1fa456) )
	ROM_LOAD( "3.bin", 0x80000, 0x40000, BAD_DUMP CRC(545aa4e6) SHA1(3348d4b692900c9e9cd4a52b20922a84e596cd35) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sound.bin", 0x00000, 0x40000, BAD_DUMP CRC(230b31c3) SHA1(38c107325d3a4e9781912078b1317dc9ba3e1ced) )
ROM_END

static DRIVER_INIT( 3super8 )
{
	UINT8 *ROM = machine.region("maincpu")->base();
	int i;

	/* TODO: proper decryption scheme */
	for(i=0;i<0x10000;i++)
		ROM[i] ^= (ROM[i+0x10000] ^ 0xff);
}

GAME( 199?, 3super8,  0,    3super8, 3super8,  3super8, ROT0, "<unknown>", "3 Super 8 (Italy)", GAME_NOT_WORKING|GAME_NO_SOUND )
