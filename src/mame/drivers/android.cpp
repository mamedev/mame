// license:BSD-3-Clause
// copyright-holders:David Haywood

// Note, another VERY different version of this also exists, see https://www.youtube.com/watch?v=5rtqZqMBACI (uploaded by Chris Hardy)
// it is unclear if that version is on the same hardware as this

#include "emu.h"
#include "cpu/z80/z80.h"



class androidp_state : public driver_device
{
public:
	androidp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_androidp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};



void androidp_state::video_start()
{
}

UINT32 androidp_state::screen_update_androidp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



static ADDRESS_MAP_START( androidp_map, AS_PROGRAM, 8, androidp_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM 
ADDRESS_MAP_END


static INPUT_PORTS_START( androidp )
INPUT_PORTS_END



static const gfx_layout sprite16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4,0  ,12,8,  20,16,  28,24,  36,32,  44,40,  52, 48,  60, 56},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	64*16
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

static GFXDECODE_START( androidp )
	GFXDECODE_ENTRY( "sprites", 0, sprite16x16_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END



void androidp_state::machine_start()
{
}

void androidp_state::machine_reset()
{
}

static MACHINE_CONFIG_START( androidp, androidp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(androidp_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", androidp_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(androidp_state, screen_update_androidp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", androidp)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



ROM_START( androidp )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "MITSUBISHI_A01.toppcb.m5l27256k.k1.BIN", 0x00000, 0x08000, CRC(25ab85eb) SHA1(e1fab149c83ff880b119258206d5818f3db641c5) )
	ROM_LOAD( "MITSUBISHI_A02.toppcb.m5l27256k.J1.BIN", 0x08000, 0x08000, CRC(e41426be) SHA1(e7e06ef3ff5160bb7d870e148ba2799da52cf24c) )
	ROM_LOAD( "MITSUBISHI_A03.toppcb.m5l27256k.G1.BIN", 0x10000, 0x08000, CRC(6cf5f48a) SHA1(b9b4e5e7bace0e8d98fbc9f4ad91bc56ef42099e) )

	ROM_REGION( 0x18000, "soundcpu", 0 )
	ROM_LOAD( "MITSUBISHI_A04.toppcb.m5l27256k.N6.BIN", 0x00000, 0x08000, CRC(13c38fe4) SHA1(34a35fa057159a5c83892a88b8c908faa39d5cb3) )	

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD16_BYTE( "MITSUBISHI_A06.botpcb.m5l27512k.9E.BIN", 0x00000, 0x10000, CRC(5e42984e) SHA1(2a928960c740dfb94589e011cce093bed2fd7685) )
	ROM_LOAD16_BYTE( "MITSUBISHI_A07.botpcb.m5l27512k.9B.BIN", 0x00001, 0x10000, CRC(611ff400) SHA1(1a9aed33d0e3f063811f92b9fee3ecbff0e965bf) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "MITSUBISHI_A05.toppcb.m5l27512k.F5.BIN", 0x00000, 0x10000, CRC(4c72a930) SHA1(f1542844391b55fe43293eef7ce48c09b7aca75a) )

	// + 2 undumped PLDs
ROM_END


GAME( 198?, androidp, 0, androidp, androidp, driver_device, 0, ROT90, "Nasco",         "Android (early build?)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
