// license:BSD-3-Clause
// copyright-holders:David Haywood
/*


*/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "audio/t5182.h"

class metlfrzr_state : public driver_device
{
public:
	metlfrzr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
		{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_metlfrzr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


void metlfrzr_state::video_start()
{
}

UINT32 metlfrzr_state::screen_update_metlfrzr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( metlfrzr_map, AS_PROGRAM, 8, metlfrzr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( metlfrzr )
INPUT_PORTS_END



void metlfrzr_state::machine_start()
{
}

void metlfrzr_state::machine_reset()
{
}


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 4, 8, 12 },
	{ 0, 1, 2, 3, 16, 17, 18, 19 }, 
//	{ 19, 18, 17, 16, 3, 2, 1, 0 }, // maybe display is flipped?
//	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	{ 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	32*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 4, 8, 12 },
//	{ 0, 1, 2, 3, 16, 17, 18, 19, 64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+16, 64*8+17, 64*8+18, 64*8+19 },
	{ 64*8+19, 64*8+18, 64*8+17, 64*8+16, 64*8+3, 64*8+2, 64*8+1, 64*8+0, 19, 18, 17, 16, 3, 2, 1, 0 },
//	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	{ 15*32,14*32,13*32,12*32,11*32,10*32,9*32,8*32, 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },
	128*8
};


static GFXDECODE_START( metlfrzr )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x16_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END

static MACHINE_CONFIG_START( metlfrzr, metlfrzr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_12MHz/2)
	MCFG_CPU_PROGRAM_MAP(metlfrzr_map)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", metlfrzr_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("t5182", T5182, 0)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", metlfrzr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(metlfrzr_state, screen_update_metlfrzr)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_14_31818MHz/4)    /* 3.579545 MHz */
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("t5182", t5182_device, ym2151_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)

MACHINE_CONFIG_END



ROM_START( metlfrzr )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.15j",   0x00000, 0x08000, CRC(f59b5fa2) SHA1(6033967dad5e64f45afbcb1b45c8eb79e0787afb) )
	ROM_LOAD( "2.14j",   0x10000, 0x10000, CRC(21ecc248) SHA1(2fccf7db73890faf7c489bfc43c88ded54d5052d) )

	ROM_REGION( 0x8000, "t5182_z80", 0 ) /* Toshiba T5182 external data ROM */
	ROM_LOAD( "3.4h", 0x0000, 0x8000, CRC(36f88e54) SHA1(5cbea56c7e547c353ae2f9256caaceb20e5e8503) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "10.5a",   0x00001, 0x10000, CRC(3313e74a) SHA1(8622dfb5c013173d5bb037254f4c23b1282404e1) )
	ROM_LOAD16_BYTE( "12.7a",   0x00000, 0x10000, CRC(6da5fda9) SHA1(9d7b0b26598f31da589fece3535a4d1405b03fc2) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "11.6a",   0x00001, 0x10000, CRC(fa6490b8) SHA1(9a4c1e09b9e8fb256fec0a5ed120fece8a12e1c8) )
	ROM_LOAD16_BYTE( "13.9a",   0x00000, 0x10000, CRC(a4f689ec) SHA1(e58bfede3fabf4cfca76c20aafb3e9fb604777c9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "14.13a",   0x00001, 0x10000, CRC(a9cd5225) SHA1(f3d5e29ee08fb563fdc1af3c64128f2cd2feb987) )
	ROM_LOAD16_BYTE( "16.11a",   0x00000, 0x10000, CRC(92f2cb49) SHA1(498021d94b0fde216207076491702af2324a2dcc) )
	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD16_BYTE( "15.12a",   0x00001, 0x10000, CRC(ce5c4c8b) SHA1(2351d66ba51e80097ce53bfd448ac24901844cda) )
	ROM_LOAD16_BYTE( "17.10a",   0x00000, 0x10000, CRC(3fec33f7) SHA1(af086ba30fc4521a0114da2824f5baa04d225a89) )


	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "n8s129a.7f",   0x000, 0x100, CRC(c849d60b) SHA1(0022fb71b3d777cadac7005e6156725df9bcaf90) )
	ROM_LOAD( "n82s135n.9c",  0x000, 0x100, CRC(7bbd52db) SHA1(b9bab5fb515579d0270aea8b992a16eeb878f242) )

	ROM_REGION( 0x20000, "plds", 0 )
	ROM_LOAD( "pld3.14h.bin",   0x000, 0x149, CRC(8183f7f0) SHA1(3cec53838120064374ecf4ebee048409c6f34081) )
	ROM_LOAD( "pld8.4d.bin",    0x000, 0x149, CRC(f1e35034) SHA1(527faddbf2ac905fa59ebda8ea327e6e6a7c1fb6) )
ROM_END
    




GAME( 1989, metlfrzr,  0,    metlfrzr, metlfrzr, driver_device,  0, ROT90, "Seibu", "Metal Freezer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
