/*

   Aristocrat MK5 / MKV hardware
   possibly 'Acorn Archimedes on a chip' hardware

   Note: ARM250 mapping is not identical to

*/

#include "emu.h"
#include "cpu/arm/arm.h"
#include "sound/dac.h"
#include "includes/archimds.h"

static ADDRESS_MAP_START( aristmk5_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x01ffffff) AM_READWRITE(archimedes_memc_logical_r, archimedes_memc_logical_w)
	AM_RANGE(0x02000000, 0x02ffffff) AM_RAM AM_BASE(&archimedes_memc_physmem) /* physical RAM - 16 MB for now, should be 512k for the A310 */
	AM_RANGE(0x03000000, 0x033fffff) AM_READWRITE(archimedes_ioc_r, archimedes_ioc_w)
	AM_RANGE(0x03400000, 0x035fffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITE(archimedes_memc_page_w)
	AM_RANGE(0x03600000, 0x037fffff) AM_READWRITE(archimedes_memc_r, archimedes_memc_w)
	AM_RANGE(0x03800000, 0x039fffff) AM_READWRITE(archimedes_vidc_r, archimedes_vidc_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( aristmk5 )
INPUT_PORTS_END

static VIDEO_START(aristmk5)
{

}

static VIDEO_UPDATE(aristmk5)
{
	return 0;
}

static DRIVER_INIT( aristmk5 )
{
	archimedes_driver_init(machine);
}

static MACHINE_START( aristmk5 )
{
	archimedes_init(machine);

	// reset the DAC to centerline
	dac_signed_data_w(devtag_get_device(machine, "dac"), 0x80);
}

static MACHINE_RESET( aristmk5 )
{
	archimedes_reset(machine);
}

static MACHINE_DRIVER_START( aristmk5 )
	MDRV_CPU_ADD("maincpu", ARM, 10000000) // ?
	MDRV_CPU_PROGRAM_MAP(aristmk5_map)

	MDRV_MACHINE_RESET( aristmk5 )
	MDRV_MACHINE_START( aristmk5 )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(aristmk5)
	MDRV_VIDEO_UPDATE(aristmk5)

	MDRV_SPEAKER_STANDARD_MONO("aristmk5")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(0, "aristmk5", 1.00)
MACHINE_DRIVER_END

ROM_START( reelrock )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "reelrock.u7",  0x000000, 0x80000, CRC(b60af34f) SHA1(1143380b765db234b3871c0fe04736472fde7de4) )
	ROM_LOAD32_WORD( "reelrock.u11", 0x000002, 0x80000, CRC(57e341d0) SHA1(9b0d50763bb74ca5fe404c9cd526633721cf6677) )
	ROM_LOAD32_WORD( "reelrock.u8",  0x100000, 0x80000, CRC(57eec667) SHA1(5f3888d75f48b6148f451d7ebb7f99e1a0939f3c) )
	ROM_LOAD32_WORD( "reelrock.u12", 0x100002, 0x80000, CRC(4ac20679) SHA1(0ac732ffe6a33806e4a06e87ec875a3e1314e06b) )
ROM_END

ROM_START( indiandr )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "indiandr.u7",  0x000000, 0x80000, CRC(0c924a3e) SHA1(499b4ae601e53173e3ba5f400a40e5ae7bbaa043) )
	ROM_LOAD32_WORD( "indiandr.u11", 0x000002, 0x80000, CRC(e371dc0f) SHA1(a01ab7fb63a19c144f2c465ecdfc042695124bdf) )
	ROM_LOAD32_WORD( "indiandr.u8",  0x100000, 0x80000, CRC(1c6bfb47) SHA1(7f751cb499a6185a0ab64eeec511583ceeee6ee8) )
	ROM_LOAD32_WORD( "indiandr.u12", 0x100002, 0x80000, CRC(4bbe67f6) SHA1(928f88387da66697f1de54f086531f600f80a15e) )
ROM_END

ROM_START( dolphntr )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "0100424v.u7",  0x000000, 0x80000, CRC(657faef7) SHA1(09e1f9d461e855c10cf8b825ef83dd3e7db65b43) )
	ROM_LOAD32_WORD( "0100424v.u11", 0x000002, 0x80000, CRC(65aa46ec) SHA1(3ad4270efbc2e947097d94a3258a544d79a1d599) )
	ROM_LOAD32_WORD( "0100424v.u8",  0x100000, 0x80000, CRC(e77868ad) SHA1(3345da120075bc0da47bac0a4840790693382620) )
	ROM_LOAD32_WORD( "0100424v.u12", 0x100002, 0x80000, CRC(6abd9309) SHA1(c405a13f5bfe447c1ab20d92e140e4fb145920d4) )
ROM_END

ROM_START( dolphtra )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "0200424v.u7",  0x000000, 0x80000, CRC(5dd88306) SHA1(ee8ec7d123d057e8df9be0e8dadecea7dab7aafd) )
	ROM_LOAD32_WORD( "0200424v.u11", 0x000002, 0x80000, CRC(bcb732ea) SHA1(838300914846c6e740780e5a24b9db7304a8a88d) )
ROM_END

ROM_START( goldprmd )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "goldprmd.u7",  0x000000, 0x80000, CRC(2fbed80c) SHA1(fb0d97cb2be96da37c487fc3aef06c6120efdb46) )
	ROM_LOAD32_WORD( "goldprmd.u11", 0x000002, 0x80000, CRC(ec9c183c) SHA1(e405082ee779c4fee103fb7384469c9d6afbc95b) )
	ROM_LOAD32_WORD( "goldprmd.u8",  0x100000, 0x80000, CRC(3cd7d8e5) SHA1(ae83a7c335564c398330d43295997b8ca547c92d) )
	ROM_LOAD32_WORD( "goldprmd.u12", 0x100002, 0x80000, CRC(8bbf45d0) SHA1(f58f28e7cc4ac225197959566d81973b5aa0e836) )
ROM_END

ROM_START( qotn )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "0200439v.u7",  0x000000, 0x80000, CRC(d476a893) SHA1(186d6fb1830c33976f2d3c96e4f045ece885dc63) )
	ROM_LOAD32_WORD( "0200439v.u11", 0x000002, 0x80000, CRC(8b0d7205) SHA1(ffa03f1c9332a1a7443eb91b0ded56e7cd9e3cee) )
	ROM_LOAD32_WORD( "0200439v.u8",  0x100000, 0x80000, CRC(9b996ef1) SHA1(72489e9a0ee5c34f7cad3d121bcd08e09ef72360) )
	ROM_LOAD32_WORD( "0200439v.u12", 0x100002, 0x80000, CRC(2a0f7feb) SHA1(27c89dadf759e6c892121650758c44ec50990cb6) )
ROM_END

ROM_START( swthrt2v )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "swthrt2v.u7",  0x000000, 0x80000, CRC(f51b2faa) SHA1(dbcfdbee92af5f89a8a2611bbc687ee0cc907642) )
	ROM_LOAD32_WORD( "swthrt2v.u11", 0x000002, 0x80000, CRC(bd7ead91) SHA1(9f775428a4aa0b0a8ee17aed9be620edc2020c5e) )
ROM_END

ROM_START( enchfrst )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "0400122v.u7",  0x000000, 0x80000, CRC(b5829b27) SHA1(f6f84c8dc524dcee95e37b93ead9090903bdca4f) )
	ROM_LOAD32_WORD( "0400122v.u11", 0x000002, 0x80000, CRC(7a97adc8) SHA1(b52f7fdc7edf9ad92351154c01b8003c0576ed94) )
ROM_END

ROM_START( margmgc )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "margmgc.u7",  0x000000, 0x80000, CRC(eee7ebaf) SHA1(bad0c08578877f84325c07d51c6ed76c40b70720) )
	ROM_LOAD32_WORD( "margmgc.u11", 0x000002, 0x80000, CRC(4901a166) SHA1(8afe6f08b4ac5c17744dff73939c4bc93124fdf1) )
	ROM_LOAD32_WORD( "margmgc.u8",  0x100000, 0x80000, CRC(b0d78efe) SHA1(bc8b345290f4d31c6553f1e2700bc8324b4eeeac) )
	ROM_LOAD32_WORD( "margmgc.u12", 0x100002, 0x80000, CRC(90ff59a8) SHA1(c9e342db2b5e8c3f45efa8496bc369385046e920) )
	ROM_LOAD32_WORD( "margmgc.u9",  0x200000, 0x80000, CRC(1f0ca910) SHA1(be7a2f395eae09a29faf99ba34551fbc38f20fdb) )
	ROM_LOAD32_WORD( "margmgc.u13", 0x200002, 0x80000, CRC(3f702945) SHA1(a6c9a848d059c1e564fdc5a65bf8c9600853edfa) )
ROM_END

ROM_START( adonis )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "adonis.u7",  0x000000, 0x80000, CRC(ab386ab0) SHA1(56c5baea4272866a9fe18bdc371a49f155251f86) )
	ROM_LOAD32_WORD( "adonis.u11", 0x000002, 0x80000, CRC(ce8c8449) SHA1(9894f0286f27147dcc437e4406870fe695a6f61a) )
	ROM_LOAD32_WORD( "adonis.u8",  0x100000, 0x80000, CRC(99097a82) SHA1(a08214ab4781b06b46fc3be5c48288e373230ef4) )
	ROM_LOAD32_WORD( "adonis.u12", 0x100002, 0x80000, CRC(443a7b6d) SHA1(c19a1c50fb8774826a1e12adacba8bbfce320891) )
ROM_END

ROM_START( dmdtouch )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "dmdtouch.u7",  0x000000, 0x80000, CRC(71b19365) SHA1(5a8ba1806af544d33e9acbcbbc0555805b4074e6) )
	ROM_LOAD32_WORD( "dmdtouch.u11", 0x000002, 0x80000, CRC(3d836342) SHA1(b015a4ba998b39ed86cdb6247c9c7f1365641b59) )
	ROM_LOAD32_WORD( "dmdtouch.u8",  0x100000, 0x80000, CRC(971bbf63) SHA1(082f81115209c7089c76fb207248da3c347a080b) )
	ROM_LOAD32_WORD( "dmdtouch.u12", 0x100002, 0x80000, CRC(9e0d08e2) SHA1(38b10f7c37f1cefe9271549073dc0a4fed409aec) )
ROM_END

ROM_START( magicmsk )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "magicmsk.u7",  0x000000, 0x80000, CRC(17317eb9) SHA1(3ddb8d61f23461c3194af534928164550208bbee) )
	ROM_LOAD32_WORD( "magicmsk.u11", 0x000002, 0x80000, CRC(23aefb5a) SHA1(ba4488754794f75f53b9c81b74b6ccd992c64acc) )
	ROM_LOAD32_WORD( "magicmsk.u8",  0x100000, 0x80000, CRC(23aefb5a) SHA1(ba4488754794f75f53b9c81b74b6ccd992c64acc) )
	ROM_LOAD32_WORD( "magicmsk.u12", 0x100002, 0x80000, CRC(6829a7bf) SHA1(97eed83763d0ec5e753d6ad194e906b1307c4940) )
ROM_END

ROM_START( geishanz )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* ARM Code */
	ROM_LOAD32_WORD( "0101408.u7",  0x000000, 0x80000, CRC(ebdde248) SHA1(83f4f4deb5c6f5b33ae066d50e043a24cb0cbfe0) )
	ROM_LOAD32_WORD( "0101408.u11", 0x000002, 0x80000, CRC(2f9e7cd4) SHA1(e9498879c9ca66740856c00fda0416f5d9f7c823) )
	ROM_LOAD32_WORD( "0101408.u8",  0x100000, 0x80000, CRC(87e41b1b) SHA1(029687aeaed701e0f4b8da9d1d60a5a0a9445518) )
	ROM_LOAD32_WORD( "0101408.u12", 0x100002, 0x80000, CRC(255f2368) SHA1(eb955452e1ed8d9d4f30f3372d7321f01d3654d3) )
	ROM_LOAD32_WORD( "0101408.u9",  0x200000, 0x80000, CRC(5f161953) SHA1(d07353d006811813b94cb022857f49c4906fd87b) )
	ROM_LOAD32_WORD( "0101408.u13", 0x200002, 0x80000, CRC(5ef6323e) SHA1(82a720d814ca06c6d286c59bbf325d9a1034375a) )
ROM_END


GAME( 1995, swthrt2v, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Sweet Hearts II (C - 07/09/95, Venezuela version)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1995, enchfrst, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Enchanted Forest (E - 23/06/95, Local)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, dolphntr, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (B - 06/12/96, NSW/ACT, Rev 1.24.4.0)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1996, dolphtra, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (B - 06/12/96, NSW/ACT, Rev 3)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1997, goldprmd, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Golden Pyramids (B - 13-05-97, USA)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1997, qotn,     0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Queen of the Nile (B - 13-05-97, NSW/ACT)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1997, dmdtouch, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Diamond Touch (E - 30-06-97, Local)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, adonis,   0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Adonis (A - 25-05-98, NSW/ACT)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, reelrock, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Reelin-n-Rockin (A - 13/07/98, Local)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1998, indiandr, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Indian Dreaming (B - 15/12/98, Local)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, magicmsk, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Magic Mask (A - 09/05/2000, Export))", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, margmgc,  0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Margarita Magic (A - 07/07/2000, NSW/ACT)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2001, geishanz, 0, aristmk5, aristmk5, aristmk5, ROT0,  "Aristocrat", "Geisha (A - 05/03/01, New Zealand)", GAME_NOT_WORKING|GAME_NO_SOUND )
