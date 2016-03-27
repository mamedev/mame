// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Kung-Fu Master / Spartan X (Tecfri bootleg)
single PCB with 2x Z80
similar looking to the '1942p' and 'spyhuntpr' PCBs

*/

#include "emu.h"
#include "cpu/z80/z80.h"


class spartanxtec_state : public driver_device
{
public:
	spartanxtec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_palette(*this, "palette")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_spartanxtec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(spartanxtec);

	required_device<palette_device> m_palette;


};


void spartanxtec_state::video_start()
{
}


UINT32 spartanxtec_state::screen_update_spartanxtec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( spartanxtec_map, AS_PROGRAM, 8, spartanxtec_state )
ADDRESS_MAP_END


static INPUT_PORTS_START( spartanxtec )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7, 128, 129, 130, 131, 132, 133, 134, 135 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8  },
	32*8
};


static GFXDECODE_START( news )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END



void spartanxtec_state::machine_start()
{
}

void spartanxtec_state::machine_reset()
{
}

PALETTE_INIT_MEMBER(spartanxtec_state, spartanxtec)
{
//	const UINT8 *color_prom = memregion("cprom")->base();

}



static MACHINE_CONFIG_START( spartanxtec, spartanxtec_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(spartanxtec_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spartanxtec_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(spartanxtec_state, screen_update_spartanxtec)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(spartanxtec_state,spartanxtec)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", news)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



ROM_START( spartanxtec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x04000, CRC(d5d6cddf) SHA1(baaec83be455bf2267d51ea2a2c1fcda22f27bd5) )
	ROM_LOAD( "2.bin", 0x04000, 0x04000, CRC(2803bb72) SHA1(d0f93c61f3f08fb866e2a4617a7824e72f61c97f) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "3.bin", 0x00000, 0x01000, CRC(9a18af94) SHA1(1644295aa0c837dced5934360e41d77e0a93ccd1) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "4.bin", 0x00000, 0x02000, CRC(b55672ef) SHA1(7bd556a76e130be1262aa7db09df84c6463ce9ef) )
	ROM_LOAD( "5.bin", 0x02000, 0x02000, CRC(8a3d2978) SHA1(e50ba8d63e894c6a555d92c3144682be68f111b0) )
	ROM_LOAD( "6.bin", 0x04000, 0x02000, CRC(b1570b6b) SHA1(380a692309690e6ff6b57fda657192fff95167e0) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "7.bin", 0x00000, 0x08000, CRC(aa897e30) SHA1(90b3b316800be106d3baa6783ca894703f369d4e) )
	ROM_LOAD( "8.bin", 0x08000, 0x08000, CRC(98a1803b) SHA1(3edfc45c289f850b07a0231ce0b792cbec6fb245) )
	ROM_LOAD( "9.bin", 0x10000, 0x08000, CRC(e3bf0d73) SHA1(4562422c07399e240081792b96b9018d1e7dd97b) )

	ROM_REGION( 0x600, "cprom", 0 )
	// first half of all of these is empty
	ROM_LOAD( "4_MCM7643_82s137.BIN", 0x0000, 0x0200, CRC(548a0ab1) SHA1(e414b61feba73bcc1a53e17c848aceea3b8100e7) ) ROM_CONTINUE(0x0000,0x0200)
	ROM_LOAD( "5_MCM7643_82s137.BIN", 0x0200, 0x0200, CRC(a678480e) SHA1(515fa2b09c666a46dc145313eda3c465afff4451) ) ROM_CONTINUE(0x0200,0x0200)
	ROM_LOAD( "6_MCM7643_82s137.BIN", 0x0400, 0x0200, CRC(5a707f85) SHA1(35932daf453787780550464b78465581e1ef35e1) ) ROM_CONTINUE(0x0400,0x0200)

	ROM_REGION( 0x18000, "timing", 0 ) // i think
	ROM_LOAD( "7_82s147.BIN", 0x0000, 0x0200, CRC(54a9e294) SHA1(d44d21ab8141bdfe697fd303cdc1b5c4177909bc) )

	ROM_REGION( 0x18000, "unkprom", 0 ) // just linear increasing value
	ROM_LOAD( "1_tbp24s10_82s129.BIN", 0x0000, 0x0100, CRC(b6135ee0) SHA1(248a978987cff86c2bbad10ef332f63a6abd5bee) )
	ROM_LOAD( "2_tbp24s10_82s129.BIN", 0x0000, 0x0100, CRC(b6135ee0) SHA1(248a978987cff86c2bbad10ef332f63a6abd5bee) )
ROM_END



GAME( 1987, spartanxtec,  kungfum,    spartanxtec, spartanxtec, driver_device,  0, ROT0, "bootleg (Tecfri)", "Spartan X (Tecfri hardware bootleg)", MACHINE_NOT_WORKING )

