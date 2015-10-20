// license:BSD-3-Clause
// copyright-holders:David Haywood
/* IGS Gambling games using IGS036 CPU
   pgm2.c also uses this CPU

<Chao Ji Da Heng 2>
cjdh2.zip
---------------------------------------------------
CPU: IGS036
GFX: IGS036
SND: 6295
==============================================
a IGS game use IGS036 chip
IGS036 could be a upgraded version of IGS027A
but with GFX processor integrated

I don't know the CPU core (should be ARM based due to fail test)
the chip has internal rom built-in
the FLASH(u33, EV29LV160AB-90PCR) is external rom and encrypted
if the external rom is decrypted then we can
try to trojan the internal rom
here we offer several revisions of the same game to see
if anyone could find any clue, these 4 revision can
be programmed and running on a same PCB.
===================================================
filename                        |
---------------------------------------------------
cjdh2_s215cn.u33                | PRG (Ver S215CN)
cjdh2_s311cn.rom                | PRG (Ver S311CN)
cjdh2_s311cna.rom               | PRG (Ver S311CNA)
cjdh2_s311cnb.rom               | PRG (Ver S311CNB)
cjdh2_cg1.u8                    | GFX ROM
cjdh2_cg2.u24                   | GFX ROM
cjdh2_sp.u20                    | SND ROM
DSC00257.JPG                    | PCB Photo
---------------------------------------------------

(dump by XingXing)


<Super Dou Di Zhu Special>
cjddzsp.zip
---------------------------------------------------
CPU: IGS036
GFX: IGS036
SND: TT5665
----------------------------------------------------------------------------
Filename            TYPE        CRC32           SHA1
cjddzsp_s122cn.u27  GFX     797e5ba3    784fae513ac8cfd1143f0d0ce0936f74e2e64e48
cjddzsp_s122cn.u28  GFX     d0441a6b    e1c948f94472398aa5887963cf8e87be28dd66e0
cjddzsp_s122cn.u30  SND     e0e02a57    96074a5226dd24d0bc150adff7324b5349cb5dc2
cjddzsp_s122cn.u18  PRG     4a42aad6    96805e5bfbd50686177fe50020229ea8787ade17
----------------------------------------------------------------------------
check more info and photo from cjdh2.zip!!!

(dump by XingXing)

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/igs036crypt.h"


class igs_m036_state : public driver_device
{
public:
	igs_m036_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	UINT32 screen_update_igs_m036(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DRIVER_INIT(igs_m036);

	DECLARE_DRIVER_INIT(cjdh2);
	DECLARE_DRIVER_INIT(cjddzsp);
	DECLARE_DRIVER_INIT(igsm312);

	required_device<cpu_device> m_maincpu;

	void pgm_create_dummy_internal_arm_region(void);

};




UINT32 igs_m036_state::screen_update_igs_m036(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( igs_m036_map, AS_PROGRAM, 32, igs_m036_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM /* Internal ROM */
	AM_RANGE(0x08000000, 0x081fffff) AM_ROM AM_REGION("user1", 0) // not 100% sure it maps here.

ADDRESS_MAP_END

static INPUT_PORTS_START( igs_m036 )
INPUT_PORTS_END



ROM_START( cjdh2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cn.u33",  0x000000, 0x200000, CRC(a6fb72f0) SHA1(1d9583eafaea21d5ec078b7f2e3dc426571a9550) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2a )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cna.u33", 0x000000, 0x200000, CRC(0bc6bc1b) SHA1(c891a7051cda1fd250d9380d7f33b47c375db74d) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2b )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cnb.u33", 0x000000, 0x200000, CRC(ddcf50bd) SHA1(39a3ed728be5894a2fec5cf0858f6f40be5ccae1) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2c )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s215cn.u33",  0x000000, 0x200000, CRC(ebe35131) SHA1(1f167e70a80b39e0658fd97c249982a0aa622683) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END


ROM_START( cjddzsp )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "cjddzsp_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjddzsp_s122cn.u18",  0x000000, 0x200000, CRC(4a42aad6) SHA1(96805e5bfbd50686177fe50020229ea8787ade17) )

	ROM_REGION( 0x800100, "oki", 0 ) // TT5665 samples
	ROM_LOAD( "cjddzsp_s122cn.u27", 0x000000, 0x800100, CRC(797e5ba3) SHA1(784fae513ac8cfd1143f0d0ce0936f74e2e64e48))

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjddzsp_s122cn.u28",  0x000000, 0x800100, CRC(d0441a6b) SHA1(e1c948f94472398aa5887963cf8e87be28dd66e0) )
	ROM_LOAD( "cjddzsp_s122cn.u30",  0x800100, 0x800100, CRC(e0e02a57) SHA1(96074a5226dd24d0bc150adff7324b5349cb5dc2) )
ROM_END

ROM_START( igsm312 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "igsunk_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "m312cn.rom", 0x000000, 0x200000, CRC(5069c310) SHA1(d53a2e8acddfbb7afc27c68c0b3167419a3ec3e6) )

	ROM_REGION( 0x800100, "oki", ROMREGION_ERASE00 ) // TT5665 samples
	/* missing */
	ROM_REGION( 0x800100*2, "gfx", ROMREGION_ERASE00 )
	/* missing */
ROM_END


void igs_m036_state::pgm_create_dummy_internal_arm_region(void)
{
	UINT16 *temp16 = (UINT16 *)memregion("maincpu")->base();
	int i;
	for (i=0;i<0x4000/2;i+=2)
	{
		temp16[i] = 0xFFFE;
		temp16[i+1] = 0xEAFF;

	}
	int base = 0;

	// jump straight to where we've mapped the external rom for testing (should really  set up a fake stack etc. too)


	temp16[(base) / 2] = 0x0004; base += 2;
	temp16[(base) / 2] = 0xe59f; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;
	temp16[(base) / 2] = 0xe590; base += 2;
	temp16[(base) / 2] = 0xff10; base += 2;
	temp16[(base) / 2] = 0xe12f; base += 2;
	temp16[(base) / 2] = 0x0010; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;

#if 0
	temp16[(base) / 2] = 0x03c9; base += 2;
	temp16[(base) / 2] = 0x0800; base += 2;
#else
	temp16[(base) / 2] = 0x0000; base += 2;
	temp16[(base) / 2] = 0x0800; base += 2;
#endif

}



#define IGS036_CPU ARM7

static MACHINE_CONFIG_START( igs_m036, igs_m036_state )
	MCFG_CPU_ADD("maincpu",IGS036_CPU, 20000000)

	MCFG_CPU_PROGRAM_MAP(igs_m036_map)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs_m036_state, screen_update_igs_m036)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	/* sound hardware (OKI) */
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( igs_m036_tt, igs_m036_state )
	MCFG_CPU_ADD("maincpu",IGS036_CPU, 20000000)

	MCFG_CPU_PROGRAM_MAP(igs_m036_map)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(igs_m036_state, screen_update_igs_m036)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	/* sound hardware (TT5665) */
MACHINE_CONFIG_END



DRIVER_INIT_MEMBER(igs_m036_state,igs_m036)
{
	pgm_create_dummy_internal_arm_region();
}

DRIVER_INIT_MEMBER(igs_m036_state, cjdh2)
{
	DRIVER_INIT_CALL(igs_m036);

	igs036_decryptor decrypter(cjdh2_key);
	decrypter.decrypter_rom(memregion("user1"));
}

DRIVER_INIT_MEMBER(igs_m036_state, cjddzsp)
{
	DRIVER_INIT_CALL(igs_m036);

	igs036_decryptor decrypter(cjddzsp_key);
	decrypter.decrypter_rom(memregion("user1"));
}

DRIVER_INIT_MEMBER(igs_m036_state, igsm312)
{
	DRIVER_INIT_CALL(igs_m036);

	igs036_decryptor decrypter(m312cn_key);
	decrypter.decrypter_rom(memregion("user1"));
}

/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 200?,  cjdh2,      0,     igs_m036, igs_m036, igs_m036_state, cjdh2,        ROT0, "IGS", "Chao Ji Da Heng 2 (V311CN)", MACHINE_IS_SKELETON )
GAME( 200?,  cjdh2a,     cjdh2, igs_m036, igs_m036, igs_m036_state, cjdh2,        ROT0, "IGS", "Chao Ji Da Heng 2 (V311CNA)", MACHINE_IS_SKELETON )
GAME( 200?,  cjdh2b,     cjdh2, igs_m036, igs_m036, igs_m036_state, cjdh2,        ROT0, "IGS", "Chao Ji Da Heng 2 (V311CNB)", MACHINE_IS_SKELETON )
GAME( 200?,  cjdh2c,     cjdh2, igs_m036, igs_m036, igs_m036_state, cjdh2,        ROT0, "IGS", "Chao Ji Da Heng 2 (V215CN)", MACHINE_IS_SKELETON )

GAME( 200?,  cjddzsp,    0,     igs_m036_tt, igs_m036, igs_m036_state, cjddzsp,   ROT0, "IGS", "Super Dou Di Zhu Special (V122CN)", MACHINE_IS_SKELETON )

GAME( 200?,  igsm312,    0,     igs_m036_tt, igs_m036, igs_m036_state, igsm312,   ROT0, "IGS", "unknown 'IGS 6POKER2' game (V312CN)", MACHINE_IS_SKELETON ) // there's very little code and no gfx roms, might be a 'set/clear' chip for a gambling game.
