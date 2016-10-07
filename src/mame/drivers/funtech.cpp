// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

program rom contains the following details

COMPANY:FUN TECH CORPORATION
PRODUCT-NAME:SUPER TWO IN ONE
PROJECTOR:TIEN YUAN CHIEN,NOMA
HARDWARE-DESIGNER:EN YU CHENG
SOFTWARE-DESIGNER:RANG CHANG LI,CHIH HNI HUANG,WEN CHANG LIN
PROGRAM-VERSION:1.0
PROGRAM-DATE:09/23/1993

8x8 tiles and 8x32 reels, likely going to be very similar to skylncr.cpp or goldstar.cpp (which are both very similar anyway)
palette addresses are the same as unkch in goldtar.cpp, but the io stuff is definitely different here

board has an M5255 for sound
and an unpopulated position for a YM2413 or UM3567

*/

#include "emu.h"
#include "cpu/z80/z80.h"



class fun_tech_corp_state : public driver_device
{
public:
	fun_tech_corp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_fgram;

	INTERRUPT_GEN_MEMBER(funtech_vblank_interrupt);

	DECLARE_READ8_MEMBER(funtech_unk_r);
//	DECLARE_WRITE8_MEMBER(funtech_unk_w);
	DECLARE_WRITE8_MEMBER(funtech_unk_00_w);
//	DECLARE_WRITE8_MEMBER(funtech_unk_01_w);
	DECLARE_WRITE8_MEMBER(funtech_unk_03_w);
	DECLARE_WRITE8_MEMBER(funtech_unk_11_w);
	DECLARE_WRITE8_MEMBER(funtech_unk_12_w);

	UINT8 m_unk03;

	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE8_MEMBER(fgram_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_funtech(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


TILE_GET_INFO_MEMBER(fun_tech_corp_state::get_fg_tile_info)
{
	int code = m_fgram[tile_index];
	int attr = m_fgram[tile_index+0x800];

	code |= (attr & 0x0f) << 8;

	// maybe
	if (m_unk03&1) code |= 0x1000;

	SET_TILE_INFO_MEMBER(0,
			code,
			attr>>4,
			0);
}


void fun_tech_corp_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(fun_tech_corp_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

WRITE8_MEMBER(fun_tech_corp_state::fgram_w)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset&0x7ff);
}


UINT32 fun_tech_corp_state::screen_update_funtech(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



INTERRUPT_GEN_MEMBER(fun_tech_corp_state::funtech_vblank_interrupt)
{
//	if (m_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}



static ADDRESS_MAP_START( funtech_map, AS_PROGRAM, 8, fun_tech_corp_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM

	AM_RANGE(0xc000, 0xc1ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xc800, 0xc9ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")

	AM_RANGE(0xd000, 0xd7ff) AM_ROM // maybe

	AM_RANGE(0xd800, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(fgram_w) AM_SHARE("fgram")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


READ8_MEMBER(fun_tech_corp_state::funtech_unk_r)
{
	return 0xff;
}

/*
WRITE8_MEMBER(fun_tech_corp_state::funtech_unk_w)
{
}
*/

WRITE8_MEMBER(fun_tech_corp_state::funtech_unk_00_w)
{
// lots of 00 / 80 writes
//	printf("funtech_unk_00_w %02x\n", data);
}

WRITE8_MEMBER(fun_tech_corp_state::funtech_unk_03_w)
{
//	printf("funtech_unk_03_w %02x\n", data);
	m_unk03 = data;
	m_fg_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(fun_tech_corp_state::funtech_unk_11_w)
{
//	printf("funtech_unk_11_w %02x\n", data);
}

WRITE8_MEMBER(fun_tech_corp_state::funtech_unk_12_w)
{
//	printf("funtech_unk_12_w %02x\n", data);
}


static ADDRESS_MAP_START( funtech_io_map, AS_IO, 8, fun_tech_corp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(funtech_unk_00_w)

//	AM_RANGE(0x01, 0x01) AM_WRITE(funtech_unk_01_w) // on startup
	AM_RANGE(0x03, 0x03) AM_WRITE(funtech_unk_03_w)

	AM_RANGE(0x04, 0x04) AM_READ(funtech_unk_r)
	AM_RANGE(0x05, 0x05) AM_READ(funtech_unk_r)
//	AM_RANGE(0x06, 0x06) AM_READ(funtech_unk_r)
//	AM_RANGE(0x07, 0x07) AM_READ(funtech_unk_r)

	AM_RANGE(0x10, 0x10) AM_READ(funtech_unk_r)

	AM_RANGE(0x11, 0x11) AM_WRITE(funtech_unk_11_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(funtech_unk_12_w)

ADDRESS_MAP_END

static INPUT_PORTS_START( funtech )
INPUT_PORTS_END


static const gfx_layout tiles8x32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4 ,5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64, 16*64, 17*64, 18*64, 19*64, 20*64, 21*64, 22*64, 23*64, 24*64, 25*64, 26*64, 27*64, 28*64, 29*64, 30*64, 31*64 },
	32*64
};


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


static GFXDECODE_START( funtech )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32_layout, 0x100, 16 )
GFXDECODE_END




void fun_tech_corp_state::machine_start()
{
}

void fun_tech_corp_state::machine_reset()
{
}

static MACHINE_CONFIG_START( funtech, fun_tech_corp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(funtech_map)
	MCFG_CPU_IO_MAP(funtech_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fun_tech_corp_state,  funtech_vblank_interrupt)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 8, 256-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fun_tech_corp_state, screen_update_funtech)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", funtech)
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



ROM_START( fts2in1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5.bin", 0x00000, 0x10000, CRC(ab19fd28) SHA1(a65ff732e0aaaec256cc63beff5f24419e691645) )

	ROM_REGION( 0x80000, "gfx1", 0 ) // crc printed on label matches half the data, even if chip was double size
	ROM_LOAD( "u18.bin", 0x00000, 0x80000, CRC(d1154aac) SHA1(dc03c4b7a4dfda2a30bfabaeb0ce053660961663) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "u29.bin", 0x00000, 0x20000, CRC(ed6a1e2f) SHA1(2c72e764c7c8091a8fa1dfc257a84d61e2da0e4b) )
	ROM_LOAD16_BYTE( "u30.bin", 0x00001, 0x20000, CRC(d572bddc) SHA1(06499aeb47085a02af9eb4987ed987f9a3a397f7) )
ROM_END

GAME( 1993, fts2in1,  0,    funtech, funtech, driver_device,  0, ROT0, "Fun Tech Corporation", "Super Two In One", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
