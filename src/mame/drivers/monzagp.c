// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Monza GP - Olympia


Upper board (MGP_02):
1x Intel P8035L (HMOS Single Component 8BIT Microcontroller - no internal ROM - 64x8 RAM)(main)
1x National DP8350N (Video Output Graphics Controller - CRT Controller)(main)
1x oscillator 10535

6x 2708 (10,11,12,13,14,15)
4x 2114 (4e,7e,6f,7f)

3x 10x2 legs connectors with flat cable to lower board
1x 8x2 dip switches


Lower board (MGP_01):
9x 2708 (1,2,3,4,5,6,7,8,9)(position 2 is empty, maybe my PCB is missing an EPROM)
5x MMI63S140N (1,3,4,5,6)
2x DM74S287N (2,7)

3x 10x2 legs connectors with flat cable to upper board
1x 6 legs connector (power supply)
1x 20x2 legs connector for flat cable
1x trimmer (TIME?????)

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"


class monzagp_state : public driver_device
{
public:
	monzagp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_coordx;
	int m_coordy;
	UINT8 *m_vram;
	int m_screenw;
	int m_bank;

	DECLARE_READ8_MEMBER(rng_r);
	DECLARE_WRITE8_MEMBER(port_w);
	DECLARE_WRITE8_MEMBER(port0_w);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_WRITE8_MEMBER(port3_w);
	virtual void video_start();
	DECLARE_PALETTE_INIT(monzagp);
	UINT32 screen_update_monzagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



PALETTE_INIT_MEMBER(monzagp_state, monzagp)
{
}

void monzagp_state::video_start()
{
	m_screenw = 80;
	m_vram = auto_alloc_array(machine(), UINT8, 0x10000);
}

UINT32 monzagp_state::screen_update_monzagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;

	if(machine().input().code_pressed_once(KEYCODE_Z))
		m_bank--;

	if(machine().input().code_pressed_once(KEYCODE_X))
		m_bank++;

	if(machine().input().code_pressed_once(KEYCODE_Q))
	{
		m_screenw--;
		printf("%x\n",m_screenw);
	}

	if(machine().input().code_pressed_once(KEYCODE_W))
	{
		m_screenw++;
		printf("%x\n",m_screenw);
	}

	if(machine().input().code_pressed_once(KEYCODE_A))
	{
		FILE * p=fopen("vram.bin","wb");
		fwrite(&m_vram[0],1,0x10000,p);
		fclose(p);
	}

	bitmap.fill(0, cliprect);
	for(y=0;y<256;y++)
	{
		for(x=0;x<256;x++)
		{
			m_gfxdecode->gfx(m_bank&1)->transpen(bitmap,cliprect,
				m_vram[y*m_screenw+x],
				//(m_vram[y*m_screenw+x]&0x3f)+(m_bank>>1)*64,
				0,
				0, 0,
				x*8,y*8,
				0);

		}
	}

	return 0;
}

static ADDRESS_MAP_START( monzagp_map, AS_PROGRAM, 8, monzagp_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

READ8_MEMBER(monzagp_state::rng_r)
{
	return machine().rand();
}

WRITE8_MEMBER(monzagp_state::port_w)
{
	m_coordx=offset;//-0xc0;
	//m_vram[m_coordy*m_screenw+m_coordx]=data;
	//if(output==0xfe)
	{
	//  if(data>='A' && data <='Z')
	//      printf("%.2x %.2x %c %c\n",m_coordy, offset,data, znaki[data-'A']);
		//m_vram[m_coordy*m_screenw+m_coordx]=data;
		m_vram[(m_coordx*256+m_coordy)&0x7ff]=data;
	}
}
/*

10 da U T
0f f2 F F
0e ea E E
0d e2 M L
12 ca U T
11 e2 H H
0f da G G
0e f2 I I
0d ea S R
13 e6 U T
11 de I I
10 f6 D D
0f ee E E
0e e6 S R
0c de C C
14 d0 Z
10 d8 O N
0f f0 I I
0e e8 P O
0d e0 C C
14 d6 T S
13 ee U T
12 e6 I I
10 de D D
0f f6 E E
0e ee S R
0d e6 C C
14 d8 Z
11 e0 O N
0f d8 I I
0e f0 P O
0d e8 C C
*/




WRITE8_MEMBER(monzagp_state::port0_w)
{
//  printf("P0 %x = %x\n",space.device().safe_pc(),data);
}

WRITE8_MEMBER(monzagp_state::port1_w)
{
//  printf("P1 %x = %x\n",space.device().safe_pc(),data);
}

WRITE8_MEMBER(monzagp_state::port2_w)
{
//  printf("P2 %x = %x\n",space.device().safe_pc(),data);
	m_coordy=data;
}

#if 0
WRITE8_MEMBER(monzagp_state::port3_w)
{
	output=data;
}
#endif

/*


#define  I8039_p0   0x100
#define  I8039_p1   0x101
#define  I8039_p2   0x102
#define  I8039_p4   0x104
#define  I8039_p5   0x105
#define  I8039_p6   0x106
#define  I8039_p7   0x107
#define  I8039_t0   0x110
#define  I8039_t1   0x111
#define  I8039_bus  0x120
*/

static ADDRESS_MAP_START( monzagp_io, AS_IO, 8, monzagp_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(rng_r,port_w)
	AM_RANGE(0x100, 0x100) AM_WRITE(port0_w)
	AM_RANGE(0x101, 0x101) AM_WRITE(port1_w)
	AM_RANGE(0x102, 0x102) AM_WRITE(port2_w)
	AM_RANGE(0x104, 0x104) AM_READ(rng_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( monzagp )
	PORT_START("IN0")
INPUT_PORTS_END

static const gfx_layout tile_layout1 =
{
	8,8,
	RGN_FRAC(1,1),
	1, /* 2 bit per pixel */
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8*2
};

static const gfx_layout tile_layout2 =
{
	8,8,
	RGN_FRAC(1,1),
	1, /* 2 bit per pixel */
	{ 8*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8*2
};

static GFXDECODE_START( monzagp )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tile_layout1,   0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tile_layout2,   0, 8 )
GFXDECODE_END

static MACHINE_CONFIG_START( monzagp, monzagp_state )
	MCFG_CPU_ADD("maincpu", I8035, 12000000/32) /* 400KHz ??? - Main board Crystal is 12MHz */
	MCFG_CPU_PROGRAM_MAP(monzagp_map)
	MCFG_CPU_IO_MAP(monzagp_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", monzagp_state, irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(monzagp_state, screen_update_monzagp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(monzagp_state, monzagp)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", monzagp)
MACHINE_CONFIG_END

ROM_START( monzagp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "12.6a",       0x0000, 0x0400, CRC(35715718) SHA1(aa64cedf1f5898b109f643975722cf15a1c752ba) )
	ROM_LOAD( "13.7a",       0x0400, 0x0400, CRC(4e16bb68) SHA1(fb1d311a40145b3dccbd3d003a683c12898f43ff) )
	ROM_LOAD( "14.8a",       0x0800, 0x0400, CRC(16a1b36c) SHA1(6bc6bac37febb7c0fe18dc9b0a4e3a71ad1faafd) )
	ROM_LOAD( "15.9a",       0x0c00, 0x0400, CRC(ee6d9cc6) SHA1(0aa9efe812c1d4865fee2bbb1764a135dd642790) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "11.8d",       0x0000, 0x0400, CRC(47607a83) SHA1(91ce272c4af3e1994db71d2239b68879dd279347) )
	ROM_LOAD( "10.7d",       0x0400, 0x0400, CRC(d2bedd67) SHA1(9b75731d2701f5b9ce0446141c5cd55b05671ec1) )
	ROM_LOAD( "9.10j",       0x0800, 0x0400, CRC(474ab63f) SHA1(6ba623d1768ed92b39e8f76c2f2eed7874955f1b) )
	ROM_LOAD( "6.4f",        0x0c00, 0x0400, CRC(934d2070) SHA1(e742bfcb910e8747780d32ca66efd7e343190fb4) )
	ROM_LOAD( "7.3f",        0x1000, 0x0400, CRC(08f29f60) SHA1(9ca454e848aa986ff9eccaead3fec5076df2e4d3) )
	ROM_LOAD( "8.1f",        0x1400, 0x0400, CRC(99ce2753) SHA1(f4540700ea909ba1be34ac2c33dafd8ec67a2bb7) )

	ROM_REGION( 0x10000, "unk1", 0 )
	ROM_LOAD( "1.9c",        0x0000, 0x0400, CRC(005d5fed) SHA1(145a860751ef7d99129b7242aacac7a4e1e14a51) )
	ROM_LOAD( "2.13f",       0x0400, 0x0400, BAD_DUMP CRC(1943122f) SHA1(3d343314fcb594560b4a280e795c8cea4a3200c9) ) /* missing, so use rom from below. Not confirmed to 100% same */
	ROM_LOAD( "3.12f",       0x0800, 0x0400, CRC(e5591074) SHA1(ac756ee605d932d7c1c3eddbe2b9c6f78dad6ce8) )
	ROM_LOAD( "4.10f",       0x0c00, 0x0400, CRC(a426a371) SHA1(d6023bebf6924d1820e631ee53896100e5b256a5) )
	ROM_LOAD( "5.9f",        0x1000, 0x0400, CRC(5abd1ef6) SHA1(1bc79225c1be2821930fdb8e821a70c7ac8683ab) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "63s140.1",    0x0000, 0x0100, CRC(5123c83e) SHA1(d8ff06af421d3dae65bc9b0a081ed56249ef61ab) )
	ROM_LOAD( "74s287.2",    0x0100, 0x0100, CRC(a5488f72) SHA1(e7cd61a5577e2935b1ffa9dc17ca9da9b1196668) )
	ROM_LOAD( "63s140.3",    0x0200, 0x0100, CRC(eebbe52a) SHA1(14af033871cad4e35c391bce4435e7cf1ba146f7) )
	ROM_LOAD( "63s140.4",    0x0300, 0x0100, CRC(b89961a3) SHA1(99070a12e66764d21fd38ce4318ee0929daea465) )
	ROM_LOAD( "63s140.5",    0x0400, 0x0100, CRC(82c92620) SHA1(51d65156ebb592ff9e6375da7aa279325482fd5f) )
	ROM_LOAD( "63s140.6",    0x0500, 0x0100, CRC(8274f838) SHA1(c3518c668bda98759b1b1d4690062ced6c639efe) )
	ROM_LOAD( "74s287.7",    0x0600, 0x0100, CRC(3248ba56) SHA1(d449f4be8df1b4189afca55a4cf0cc2e19eb4dd4) )
ROM_END

// bootleg hardware seems identical, just bad quality pcb
ROM_START( monzagpb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m12c.6a",     0x0000, 0x0400, CRC(35715718) SHA1(aa64cedf1f5898b109f643975722cf15a1c752ba) )
	ROM_LOAD( "m13c.7a",     0x0400, 0x0400, CRC(4e16bb68) SHA1(fb1d311a40145b3dccbd3d003a683c12898f43ff) )
	ROM_LOAD( "m14.8a",      0x0800, 0x0400, CRC(16a1b36c) SHA1(6bc6bac37febb7c0fe18dc9b0a4e3a71ad1faafd) )
	ROM_LOAD( "m15bi.9a",    0x0c00, 0x0400, CRC(ee6d9cc6) SHA1(0aa9efe812c1d4865fee2bbb1764a135dd642790) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "m11.8d",      0x0000, 0x0400, CRC(5b4a7ffa) SHA1(50fa073437febe516065cd83fbaf85b596c4f3c8) ) /* differs from above */
	ROM_LOAD( "m10.7d",      0x0400, 0x0400, CRC(19db00af) SHA1(c73da9c2fdbdb1b52a7354ba169af43b26fcb4cc) ) /* differs from above */
	ROM_LOAD( "m9.10j",      0x0800, 0x0400, CRC(474ab63f) SHA1(6ba623d1768ed92b39e8f76c2f2eed7874955f1b) )
	ROM_LOAD( "m6.4f",       0x0c00, 0x0400, CRC(934d2070) SHA1(e742bfcb910e8747780d32ca66efd7e343190fb4) )
	ROM_LOAD( "m7.3f",       0x1000, 0x0400, CRC(08f29f60) SHA1(9ca454e848aa986ff9eccaead3fec5076df2e4d3) )
	ROM_LOAD( "m8.1f",       0x1400, 0x0400, CRC(99ce2753) SHA1(f4540700ea909ba1be34ac2c33dafd8ec67a2bb7) )

	ROM_REGION( 0x10000, "unk1", 0 )
	ROM_LOAD( "m1.9c",       0x0000, 0x0400, CRC(005d5fed) SHA1(145a860751ef7d99129b7242aacac7a4e1e14a51) )
	ROM_LOAD( "m2.13f",      0x0400, 0x0400, CRC(1943122f) SHA1(3d343314fcb594560b4a280e795c8cea4a3200c9) )
	ROM_LOAD( "m3.12f",      0x0800, 0x0400, CRC(e5591074) SHA1(ac756ee605d932d7c1c3eddbe2b9c6f78dad6ce8) )
	ROM_LOAD( "m4.10f",      0x0c00, 0x0400, CRC(a426a371) SHA1(d6023bebf6924d1820e631ee53896100e5b256a5) )
	ROM_LOAD( "m5.9f",       0x1000, 0x0400, CRC(5abd1ef6) SHA1(1bc79225c1be2821930fdb8e821a70c7ac8683ab) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "6300.1",      0x0000, 0x0100, CRC(5123c83e) SHA1(d8ff06af421d3dae65bc9b0a081ed56249ef61ab) )
	ROM_LOAD( "6300.2",      0x0100, 0x0100, CRC(8274f838) SHA1(c3518c668bda98759b1b1d4690062ced6c639efe) ) /* differs from above */
	ROM_LOAD( "6300.3",      0x0200, 0x0100, CRC(eebbe52a) SHA1(14af033871cad4e35c391bce4435e7cf1ba146f7) )
	ROM_LOAD( "6300.4",      0x0300, 0x0100, CRC(b89961a3) SHA1(99070a12e66764d21fd38ce4318ee0929daea465) )
	ROM_LOAD( "6300.5",      0x0400, 0x0100, CRC(82c92620) SHA1(51d65156ebb592ff9e6375da7aa279325482fd5f) )
	ROM_LOAD( "6300.6",      0x0500, 0x0100, CRC(8274f838) SHA1(c3518c668bda98759b1b1d4690062ced6c639efe) )
	ROM_LOAD( "6300.7",      0x0600, 0x0100, CRC(3248ba56) SHA1(d449f4be8df1b4189afca55a4cf0cc2e19eb4dd4) )
ROM_END


GAME( 1981, monzagp,  0,       monzagp, monzagp, driver_device, 0, ROT270, "Olympia", "Monza GP", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1981, monzagpb, monzagp, monzagp, monzagp, driver_device, 0, ROT270, "bootleg", "Monza GP (bootleg)", GAME_NOT_WORKING|GAME_NO_SOUND )
