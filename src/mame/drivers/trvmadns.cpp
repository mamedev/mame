// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

Trivia Madness

PCB Thunderhead, Inc. 1983


PC-1031-D    (CPU Board)
+---------------------------------------------+
|H6845P       CNA                   10MHz     |
|      BAT          CNB                       C
|       9                                     N
C       1               6       8             2
N  Z    2               1       9             8
5  8    8  U   U   U    1       1             |
0  0       7   6   5    6       0          VOL|
+---------------------------------------------+

A-1041-B    (ROM Board)
+-------------------------|-CN50-|------------+
|                                     AM9128  |
|                       HM6264                |
|                                             |
C PAL10L8  U13   U17    U21     U29     U35   |
N          U12   U16    U20     U28     U34   |
5          U11   U15    U19     U27     U33   |
0          U10   U14    U18     U26     U32   |
+---------------------------------------------+

CPU board:
  CPU: Z84000ABI Z80 cpu
Sound: AY-3-8910
  RAM: AMD AM9128-15PC (2048x8 Static RAM)
  OSC: 10.000MHz
Video: F6845P (or H6845P also labeled as HD46505RP) 40 pin dil (8 bit CRT Controller)
 Misc: RCA X (CDM6116E2) 24 pin dil (General-Purpose Static RAM - Multiplexed I/O)
 Roms: u7f lat green - type 2764
       u6f lat green - type 2764
       u5f lat la trivia - type 2764
  BAT: Battery to back AM9128 for game configuration?
 CN28: Edge connector 28 finger dual side for 56 connections, not JAMMA compatible
2 50-pin Ribon cable connectors (CNA + CNB make up 1 50-pin connection to the ROM board)


Rom board:
 Ram: HM6264P-15 (64 K SRAM ( 8-kword X 8-bit )
 Ram: 8437 (SY2128-3 2Kx8 SRAM 150NS)
Roms: row d-e sex a1       - type 27128
      row c-d sex a2       - type 27128
      row b-c sex a3       - type 27128
      row a sex a4         - type 27128
      row d-e soaps a1     - type 27128
      row c-d soaps a2     - type 27128
      row b-c soaps a3     - type 27128
      row d-e 2 str trk a1 - type 27256
      row c-d 2 str trk a2 - type 27256
      row d-e 2 rebel a1   - type 27256
      row c-d 2 rebel a2   - type 27256
      row triva madness    - type 27128
2 50-pin Ribon cable connectors

Google for F6845P shows this info from DatasheetArchive.com:

Part Number = F6845P
Description = Video Output Graphics Controller - CRT Controller
Manufacturer = Various
Micro Processor Family = 6800
Maximum Clock Frequency (Hz) = 1.0M
Vsup Nom.(V) Supply Voltage = 5.0
Package = DIP
Pins = 40
Military = N
Technology = NMOS


 driver by Pierpaolo Prazzoli
 thanks to David Haywood and Tomasz Slanina for their assistance

 TODO:
 - fix question banks (sometimes the game hangs)
 - fix palette
 - fix tilemap colors
 - remove hack for irq0 firing
 - add nvram

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


class trvmadns_state : public driver_device
{
public:
	trvmadns_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gfxram(*this, "gfxram"),
		m_tileram(*this, "tileram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_8(*this, "paletteram") { }

	void trvmadns(machine_config &config);

private:
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<uint8_t> m_gfxram;
	required_shared_ptr<uint8_t> m_tileram;
	int m_old_data;
	DECLARE_WRITE8_MEMBER(trvmadns_banking_w);
	DECLARE_WRITE8_MEMBER(trvmadns_gfxram_w);
	DECLARE_WRITE8_MEMBER(trvmadns_palette_w);
	DECLARE_WRITE8_MEMBER(w2);
	DECLARE_WRITE8_MEMBER(w3);
	DECLARE_WRITE8_MEMBER(trvmadns_tileram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_trvmadns(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;
	void cpu_map(address_map &map);
	void io_map(address_map &map);
};


WRITE8_MEMBER(trvmadns_state::trvmadns_banking_w)
{
	uint8_t *rom;
	int address = 0;

	if((data & 0xf0) == 0xa0)
	{
		/* FIXME: selects GFX char RAM bank here */
	}
	else if((data & 0xf0) == 0x80 || (data & 0xf0) == 0x90)
	{
		rom = memregion("user2")->base();

		switch(data & 0xf)
		{
			case 0x00: address = 0x6000; break;
			case 0x04: address = 0x4000; break;
			case 0x06: address = 0x2000; break;
			case 0x07: address = 0x0000; break;
			case 0x08: address = 0xe000; break;
			case 0x0c: address = 0xc000; break;
			case 0x0e: address = 0xa000; break;
			case 0x0f: address = 0x8000; break;
		}

		address |= (data & 0x10) ? 0x10000 : 0;

		membank("bank1")->set_base(&rom[address]);
		membank("bank2")->set_base(&rom[address + 0x1000]);
	}
	else
	{
			if(data != m_old_data)
			{
				m_old_data = data;
				logerror("port80 = %02X\n",data);
				//logerror("port80 = %02X\n",data);
			}

		rom = memregion("user1")->base();

		/*
		7
		6
		4
		0
		*/

		//switch(data & 0xf)
		switch(data & 7)
		{
			case 0x00: address = 0x6000; break;
			case 0x04: address = 0x0000; break;
			case 0x06: address = 0x2000; break;
			case 0x07: address = 0x4000; break;

		}
//24: 1st rom star trek
		address |= ((data & 0x60) >> 5) * 0x10000;

		//not sure about (((data & 7) ^ 7) * 0x2000)
		//address = (((data & 0x60) >> 5) * 0x10000) | (((data & 0x0f) ^ 7) * 0x2000);

		//address |= (data & 0x08) ? 0x8000 : 0;

//      logerror("add = %X\n",address);

		membank("bank1")->set_base(&rom[address]);
	}
}

WRITE8_MEMBER(trvmadns_state::trvmadns_gfxram_w)
{
	m_gfxram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset/16);
}

WRITE8_MEMBER(trvmadns_state::trvmadns_palette_w)
{
	int r,g,b,datax;
	m_generic_paletteram_8[offset] = data;
	offset>>=1;
	datax=m_generic_paletteram_8[offset*2+1]+256*m_generic_paletteram_8[offset*2];

	b = (((datax & 0x0007)>>0) | ((datax & 0x0200)>>6)) ^ 0xf;
	r = (((datax & 0x0038)>>3) | ((datax & 0x0400)>>7)) ^ 0xf;
	g = (((datax & 0x01c0)>>6) | ((datax & 0x0800)>>8)) ^ 0xf;

	m_palette->set_pen_color(offset, pal4bit(r), pal4bit(g), pal4bit(b));
}


WRITE8_MEMBER(trvmadns_state::w2)
{
/*  static int old = -1;
    if(data!=old)
        logerror("w2 = %02X\n",old=data);
*/
}

WRITE8_MEMBER(trvmadns_state::w3)
{
/*  static int old = -1;
    if(data!=old)
        logerror("w3 = %02X\n",old=data);
*/
}

WRITE8_MEMBER(trvmadns_state::trvmadns_tileram_w)
{
	if(offset==0)
	{
		if(m_maincpu->pcbase()==0x29e9)// || m_maincpu->pcbase()==0x1b3f) //29f5
		{
			m_maincpu->set_input_line(0, HOLD_LINE);
		}
//      else
//          logerror("%x \n", m_maincpu->pcbase());

	}

	m_tileram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}


void trvmadns_state::cpu_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).bankr("bank1");
	map(0x7000, 0x7fff).bankr("bank2");
	map(0x6000, 0x7fff).w(FUNC(trvmadns_state::trvmadns_gfxram_w)).share("gfxram");
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa7ff).ram().w(FUNC(trvmadns_state::trvmadns_tileram_w)).share("tileram");
	map(0xc000, 0xc01f).ram().w(FUNC(trvmadns_state::trvmadns_palette_w)).share("paletteram");
	map(0xe000, 0xe000).w(FUNC(trvmadns_state::w2));//NOP
	map(0xe004, 0xe004).w(FUNC(trvmadns_state::w3));//NOP
}

void trvmadns_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x02).portr("IN0");
	map(0x80, 0x80).w(FUNC(trvmadns_state::trvmadns_banking_w));
}

static INPUT_PORTS_START( trvmadns )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	0x200,
	2,
	{ 0, 1 },
	{ 6, 4, 2, 0, 14, 12, 10, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static GFXDECODE_START( gfx_trvmadns )
	GFXDECODE_ENTRY( nullptr, 0x6000, charlayout, 0, 4 ) // doesn't matter where we point this, all the tiles are decoded while the game runs
GFXDECODE_END

TILE_GET_INFO_MEMBER(trvmadns_state::get_bg_tile_info)
{
	int tile,attr,color,flag;

	attr = m_tileram[tile_index*2 + 0];
	tile = m_tileram[tile_index*2 + 1] + ((attr & 0x01) << 8);
	color = (attr & 0x18) >> 3;
	flag = TILE_FLIPXY((attr & 0x06) >> 1);

//  if((~attr & 0x20) || (~attr & 0x40))
//      flag |= TILE_FORCE_LAYER0;

	//0x20? tile transparent pen 1?
	//0x40? tile transparent pen 1?

	SET_TILE_INFO_MEMBER(0,tile,color,flag);

	tileinfo.category = (attr & 0x20)>>5;
}

void trvmadns_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(trvmadns_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

//  fg_tilemap->set_transparent_pen(1);

	m_gfxdecode->gfx(0)->set_source(m_gfxram);
}

uint32_t trvmadns_state::screen_update_trvmadns(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	bitmap.fill(0xd, cliprect);

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int attr = m_tileram[count*2+0];
			int tile = m_tileram[count*2+1] | ((attr & 0x01) << 8);
			int color = (attr & 0x18) >> 3;
			int flipx = attr & 4;
			int flipy = attr & 2;

			if(!(attr & 0x20))
				gfx->opaque(bitmap,cliprect,tile,color,flipx,flipy,(x*8),(y*8));
			count++;
		}
	}

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int attr = m_tileram[count*2+0];
			int tile = m_tileram[count*2+1] | ((attr & 0x01) << 8);
			int color = (attr & 0x18) >> 3;
			int flipx = attr & 4;
			int flipy = attr & 2;

			if(attr & 0x20)
				gfx->transpen(bitmap,cliprect,tile,color,flipx,flipy,(x*8),(y*8),1);
			count++;
		}
	}

	return 0;
}

void trvmadns_state::machine_reset()
{
	m_old_data = -1;
}

void trvmadns_state::trvmadns(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(10'000'000)/4); // Most likely 2.5MHz (less likely 5MHz (10MHz/2))
	m_maincpu->set_addrmap(AS_PROGRAM, &trvmadns_state::cpu_map);
	m_maincpu->set_addrmap(AS_IO, &trvmadns_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 31*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(trvmadns_state::screen_update_trvmadns));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_trvmadns);
	PALETTE(config, m_palette).set_entries(16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, "aysnd", XTAL(10'000'000)/2/4).add_route(ALL_OUTPUTS, "mono", 1.0); //?
}


ROM_START( trvmadns )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5f lat la trivia.u5", 0x0000, 0x2000, CRC(a8fb07ea) SHA1(dcf2cccd8b98087d30b3347e69b1bf8565f95ad6) )
	ROM_LOAD( "u6f lat green.u6",     0x2000, 0x2000, CRC(40f816f1) SHA1(a1a6a9af99edb1860bc4c8eb51859bbfbf91cae2) )
	ROM_LOAD( "u7f lat green.u7",     0x4000, 0x2000, CRC(3e45feb0) SHA1(5ffc18ab3f6ace844242d4be52b3946c1469944a) )

	ROM_REGION( 0x40000, "user1", ROMREGION_ERASEFF ) /* Question roms 1st set */
	ROM_LOAD( "row d-e 2 rebel a1.bin",   0x00000, 0x8000, CRC(92e6dcf8) SHA1(e8429fe60fadfc841ed0d69b4a815765e82723db) )
	ROM_LOAD( "row c-d 2 rebel a2.bin",   0x08000, 0x8000, CRC(b45429be) SHA1(9285f12bc0bceb0a91fc1f2f2941825b73bdb02c) )
	ROM_LOAD( "row d-e 2 str trk a1.bin", 0x10000, 0x8000, CRC(dc6fc7e1) SHA1(86ba730123bbb06d8290d68c042dd215bddf7629) )
	ROM_LOAD( "row c-d 2 str trk a2.bin", 0x18000, 0x8000, CRC(0133c462) SHA1(e6928880cb7916408579fa9d67ad3adc558de133) )
	ROM_LOAD( "row d-e soaps a1.bin",     0x20000, 0x4000, CRC(df451b8a) SHA1(23a5f953018e5401fe26eb638e13caacf6fa628f) )
	ROM_LOAD( "row c-d soaps a2.bin",     0x24000, 0x4000, CRC(13685dac) SHA1(f5f6103404c846decf32eae5d504e00e17629b03) )
	ROM_LOAD( "row b-c soaps a3.bin",     0x28000, 0x4000, CRC(b42f3294) SHA1(eac4e26ed48e7de80a60fbb2ee4e661619700d1d) )
	// 0x2c000 - 0x2ffff empty
	ROM_LOAD( "row d-e sex a1.bin",       0x30000, 0x4000, CRC(857c1332) SHA1(fdb08080143e170441b3db9e69b21ac9da10d499) )
	ROM_LOAD( "row c-d sex a2.bin",       0x34000, 0x4000, CRC(8fba2e07) SHA1(cbe7f9b973bd2a127cae736df39112b050ec98d8) )
	ROM_LOAD( "row b-c sex a3.bin",       0x38000, 0x4000, CRC(3fea2c2a) SHA1(fa403e14b057f0e6d607871adcaba85a6c77f1f9) )
	ROM_LOAD( "row a sex a4.bin",         0x3c000, 0x4000, CRC(2d179c7b) SHA1(153240f1fcc4f53b6840eafdd9ce0fb3e52ec1aa) )

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF ) /* Question roms 2nd set */
	ROM_LOAD( "trivia madness 81b9--6aa6.u35", 0x00000, 0x2000, CRC(5aec7cfa) SHA1(09e4eac78d975aef3af224b42b60499d759e7749) )
	ROM_CONTINUE(                              0x0e000, 0x2000 )
	// empty space, for 3 roms (each one max 0x8000 bytes long)
ROM_END

ROM_START( trvmadnsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5f lat la trivia.u5", 0x0000, 0x2000, CRC(a8fb07ea) SHA1(dcf2cccd8b98087d30b3347e69b1bf8565f95ad6) )
	ROM_LOAD( "u6f lat green.u6",     0x2000, 0x2000, CRC(40f816f1) SHA1(a1a6a9af99edb1860bc4c8eb51859bbfbf91cae2) )
	ROM_LOAD( "u7f lat green.u7",     0x4000, 0x2000, CRC(3e45feb0) SHA1(5ffc18ab3f6ace844242d4be52b3946c1469944a) )

	ROM_REGION( 0x40000, "user1", ROMREGION_ERASEFF ) /* Question roms 1st set */
	ROM_LOAD( "row d.e. ent b1 5fae--1ec8.u29",    0x00000, 0x4000, CRC(7ff56ea9) SHA1(c8e6e3b0ac4fc2ac566d041dee5422c6976d3b91) )
	ROM_LOAD( "row c.d. ent b2 8e46--6967.u28",    0x04000, 0x4000, CRC(fc6aa7f0) SHA1(e95a7bf6dc07d151abb46c0066208666d01b96a8) )
	ROM_LOAD( "row b.c. ent b3 bcf7--914c.u27",    0x08000, 0x4000, CRC(48d6f2f4) SHA1(59fe01a8474fb3c77a995cb7d55ea1dacbfb247a) )
	ROM_LOAD( "row a. ent b4 6429--71fb.u26",      0x0c000, 0x4000, CRC(eb72757e) SHA1(4614e845ec44c04a208afc9bff16067b25091ba3) )
	ROM_LOAD( "row d.e. t.v. b1 2313--edb8.u21",   0x10000, 0x4000, CRC(9841b455) SHA1(1281d9085a026617950d609cf3cb8c45d58b6aa3) )
	ROM_LOAD( "row c.d. t.v. b2 e38b--f95a.u20",   0x14000, 0x4000, CRC(7ded2e40) SHA1(1766f12f82f4692b0f73e4a65456c4ed7dbb56ee) )
	ROM_LOAD( "row b.c. t.v. b3 22c9--de46.u19",   0x18000, 0x4000, CRC(eeebbfa0) SHA1(d679c29a7868aa2214857d8381cdff4f7a7c116f) )
	ROM_LOAD( "row a. t.v. b4 2555--9e1e.u18",     0x1c000, 0x4000, CRC(f6837c47) SHA1(f3e49fe69ab84eba8ead04b7c2c1d0c9227517fd) )
	ROM_LOAD( "row d.e. sports b1 4714--1f76.u17", 0x20000, 0x4000, CRC(70a33fbd) SHA1(e4e725a86b85827599b5ba2fec56352e55c0f33d) )
	ROM_LOAD( "row c.d. sports b2 f9c1--1f79.u16", 0x24000, 0x4000, CRC(b582bd2b) SHA1(cb8123e971d3618573591a4cbd13c40531b70140) )
	ROM_LOAD( "row b.c. sports b3 0167--04c6.u15", 0x28000, 0x4000, CRC(c182b664) SHA1(2af3050cf375528bd27a09cce30832a678bb55db) )
	ROM_LOAD( "row a. sports b4 6392--bb9b.u14",   0x2c000, 0x4000, CRC(c333669a) SHA1(a2db88f716a529ab88ad01a165b3581b299f6283) )
	ROM_LOAD( "row d.e. travel b1 a257--b03b.u13", 0x30000, 0x4000, CRC(fc7711eb) SHA1(91e3ae7be16a498aef6f1594744043ad5efd4b26) )
	ROM_LOAD( "row c.d. travel b2 04b0--ed47.u12", 0x34000, 0x4000, CRC(62247db1) SHA1(3c24d0c77bd8560d3ec26603b5ba18ffb401f5d2) )
	ROM_LOAD( "row b.c. travel b3 bcbd--f516.u11", 0x38000, 0x4000, CRC(389a0f0f) SHA1(ef9cbb8ce921aadfea9932074899e8c08eea5d4e) )
	ROM_LOAD( "row a. travel b4 04bc--8208.u10",   0x3c000, 0x4000, CRC(eaa9c4d3) SHA1(49518a5baba42459b0a777d25874e0ef979a3847) )

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF ) /* Question roms 2nd set */
	ROM_LOAD( "trivia madness 81b9--6aa6.u35", 0x00000, 0x2000, CRC(5aec7cfa) SHA1(09e4eac78d975aef3af224b42b60499d759e7749) )
	ROM_CONTINUE(                              0x0e000, 0x2000 )
	// empty space, for 3 roms (each one max 0x8000 bytes long)
ROM_END

GAME( 1985, trvmadns,         0, trvmadns, trvmadns, trvmadns_state, empty_init, ROT0, "Thunderhead Inc.", "Trivia Madness - Series A Question set", MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING )
GAME( 1985, trvmadnsa, trvmadns, trvmadns, trvmadns, trvmadns_state, empty_init, ROT0, "Thunderhead Inc.", "Trivia Madness - Series B Question set", MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING )
