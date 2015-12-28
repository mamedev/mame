// license:BSD-3-Clause
// copyright-holders:David Haywood, ???
/* PGM 2 hardware.

    Motherboard is bare bones stuff, and does not contain any ROMs.
    The IGS036 used by the games is an ARM based CPU, like IGS027A used on PGM1 it has internal ROM.
    Decryption should be correct in most cases, but the ARM mode code at the start of the external
    ROMs is a bit weird, with many BNV instructions rather than jumps.  Maybe the ARM is customized,
    the code has been 'NOPPED' out this way (BNV is Branch Never) or it's a different type of ARM?

     - Some of the THUMB code looks like THUMB2 code
      eg
        f004 BL (HI) 00004000
        e51f B #fffffa3e
        0434 LSL R4, R6, 16
        0000 LSL R0, R0, 0

        should be a 32-bit branch instruction with the 2nd dword used as data.


    We need to determine where VRAM etc. map in order to attempt tests on the PCBs.


    PGM2 Motherboard Components:

     IS61LV25616AL(SRAM)
     IGS037(GFX PROCESSOR)
     YMZ774-S(SOUND)
     R5F21256SN(extra MCU for protection and ICcard communication)
      - Appears to be referred to by the games as MPU

    Cartridges
     IGS036 (MAIN CPU) (differs per game, internal code)
     ROMs
     Custom program ROM module (KOV3 only)
      - on some games ROM socket contains Flash ROM + SRAM

     QFP100 chip (Xlinx CPLD)

     Single PCB versions of some of the titles were also available

    Only 5 Games were released for this platform, 3 of which are just updates / re-releases of older titles!
    The platform has since been superseded by PGM3 (HD system uses flash cards etc.)

    Oriental Legend 2
    The King of Fighters '98 - Ultimate Match - Hero  (NOT DUMPED)
    Knights of Valour 2 New Legend
    Dodonpachi Daioujou Tamashii
    Knights of Valour 3

    These were only released as single board PGM2 based hardware, seen for sale in Japan for around $250-$300

    Jigsaw World Arena
    Puzzle of Ocha / Ochainu No Pazuru

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/ymz770.h"
#include "machine/igs036crypt.h"

class pgm2_state : public driver_device
{
public:
	pgm2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	DECLARE_DRIVER_INIT(kov2nl);
	DECLARE_DRIVER_INIT(orleg2);
	DECLARE_DRIVER_INIT(ddpdojh);
	DECLARE_DRIVER_INIT(kov3);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_pgm2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_pgm2(screen_device &screen, bool state);
	required_device<cpu_device> m_maincpu;

	void pgm_create_dummy_internal_arm_region(int addr);
};

static ADDRESS_MAP_START( pgm2_map, AS_PROGRAM, 32, pgm2_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM //AM_REGION("user1", 0x00000) // internal ROM
	AM_RANGE(0x08000000, 0x087fffff) AM_ROM AM_REGION("user1", 0) // not 100% sure it maps here.
	AM_RANGE(0xffff0000, 0xffffffff) AM_RAM

ADDRESS_MAP_END

static INPUT_PORTS_START( pgm2 )
INPUT_PORTS_END

UINT32 pgm2_state::screen_update_pgm2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void pgm2_state::screen_eof_pgm2(screen_device &screen, bool state)
{
}

void pgm2_state::video_start()
{
}

void pgm2_state::machine_start()
{
}

void pgm2_state::machine_reset()
{
}

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

/* slightly odd decode, and ends up with multiple letters in a single 32x32 tile, can probably specify corners, or needs an address line swap before decode */
/* actually 7bpp? roms report fixed bits, but should be good dumps */
static const gfx_layout tiles32x32x8_layout =
{
	32,32,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 2*8, 1*8, 3*8, 4*8, 6*8, 5*8, 7*8, 8*8, 10*8, 9*8, 11*8, 12*8, 14*8, 13*8, 15*8,
		16*8, 18*8, 17*8, 19*8, 20*8, 22*8, 21*8, 23*8, 24*8, 26*8, 25*8, 27*8, 28*8, 30*8, 29*8, 31*8 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256,
		16*256, 17*256, 18*256, 19*256, 20*256, 21*256, 22*256, 23*256, 24*256, 25*256, 26*256, 27*256, 28*256, 29*256, 30*256, 31*256
	},
	256*32
};

#if 0
/* sprites aren't tile based, this is variable width 1bpp data, colour data is almost certainly in sprites b */
/* there don't seem to be any indexes into the colour data, probably provided by the program, or the colour data references the bitmaps (reverse of PGM) */
static const gfx_layout tiles32x8x1_layout =
{
	32,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};
#endif



static GFXDECODE_START( pgm2 )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "bgtile", 0, tiles32x32x8_layout, 0, 16 )
#if 0
	// not tile based
	GFXDECODE_ENTRY( "spritesa", 0, tiles32x8x1_layout, 0, 16 )
	GFXDECODE_ENTRY( "spritesb", 0, tiles32x8x1_layout, 0, 16 )
#endif
GFXDECODE_END


void pgm2_state::pgm_create_dummy_internal_arm_region(int addr)
{
	UINT16 *temp16 = (UINT16 *)memregion("maincpu")->base();
	int i;
	for (i=0;i<0x4000/2;i+=2)
	{
		temp16[i] = 0xFFFE;
		temp16[i+1] = 0xEAFF;

	}
	int base = 0;

	// just do a jump to 0x080003c9  because there is some valid thumb code there with the current hookup..
	// i'd expect valid non-thumb code at 0x08000000 tho?

	temp16[(base) / 2] = 0x0004; base += 2;
	temp16[(base) / 2] = 0xe59f; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;
	temp16[(base) / 2] = 0xe590; base += 2;
	temp16[(base) / 2] = 0xff10; base += 2;
	temp16[(base) / 2] = 0xe12f; base += 2;
	temp16[(base) / 2] = 0x0010; base += 2;
	temp16[(base) / 2] = 0x0000; base += 2;

	temp16[(base) / 2] = addr&0xffff; base += 2;
	temp16[(base) / 2] = (addr>>16)&0xffff; base += 2;
}



static MACHINE_CONFIG_START( pgm2, pgm2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM9, 20000000) // ?? ARM baesd CPU, has internal ROM.
	MCFG_CPU_PROGRAM_MAP(pgm2_map)
//  MCFG_DEVICE_DISABLE()



	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pgm2_state, screen_update_pgm2)
	MCFG_SCREEN_VBLANK_DRIVER(pgm2_state, screen_eof_pgm2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pgm2)
	MCFG_PALETTE_ADD("palette", 0x1000)


	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_YMZ770_ADD("ymz770", 16384000)  // Actually a YMZ774 on-board
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

ROM_START( orleg2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "xyj2_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "xyj2_v104cn.u7",          0x00000000, 0x0800000, CRC(7c24a4f5) SHA1(3cd9f9264ef2aad0869afdf096e88eb8d74b2570) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a_text.u4",            0x00000000, 0x0200000, CRC(fa444c32) SHA1(31e5e3efa92d52bf9ab97a0ece51e3b77f52ce8a) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "ig-a_bgl.u35",     0x00000000, 0x0800000, CRC(083a8315) SHA1(0dba25e132fbb12faa59ced648c27b881dc73478) )
	ROM_LOAD32_WORD( "ig-a_bgh.u36",     0x00000002, 0x0800000, CRC(e197221d) SHA1(5574b1e3da4b202db725be906dd868edc2fd4634) )

	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a_bml.u12",     0x00000000, 0x1000000, CRC(113a331c) SHA1(ee6b31bb2b052cc8799573de0d2f0a83f0ab4f6a) )
	ROM_LOAD32_WORD( "ig-a_bmh.u16",     0x00000002, 0x1000000, CRC(fbf411c8) SHA1(5089b5cc9bbf6496ef1367c6255e63e9ab895117) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "ig-a_cgl.u18",     0x00000000, 0x2000000, CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) )
	ROM_LOAD32_WORD( "ig-a_cgh.u26",     0x00000002, 0x2000000, CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) )

	ROM_REGION( 0x1000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "ig-a_sp.u2",              0x00000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) )
ROM_END

ROM_START( orleg2o )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "xyj2_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "xyj2_v103cn.u7",  0x000000, 0x800000, CRC(21c1fae8) SHA1(36eeb7a5e8dc8ee7c834f3ff1173c28cf6c2f1a3) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a_text.u4",            0x00000000, 0x0200000, CRC(fa444c32) SHA1(31e5e3efa92d52bf9ab97a0ece51e3b77f52ce8a) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "ig-a_bgl.u35",     0x00000000, 0x0800000, CRC(083a8315) SHA1(0dba25e132fbb12faa59ced648c27b881dc73478) )
	ROM_LOAD32_WORD( "ig-a_bgh.u36",     0x00000002, 0x0800000, CRC(e197221d) SHA1(5574b1e3da4b202db725be906dd868edc2fd4634) )

	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a_bml.u12",     0x00000000, 0x1000000, CRC(113a331c) SHA1(ee6b31bb2b052cc8799573de0d2f0a83f0ab4f6a) )
	ROM_LOAD32_WORD( "ig-a_bmh.u16",     0x00000002, 0x1000000, CRC(fbf411c8) SHA1(5089b5cc9bbf6496ef1367c6255e63e9ab895117) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "ig-a_cgl.u18",     0x00000000, 0x2000000, CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) )
	ROM_LOAD32_WORD( "ig-a_cgh.u26",     0x00000002, 0x2000000, CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) )

	ROM_REGION( 0x1000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "ig-a_sp.u2",              0x00000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) )
ROM_END

ROM_START( orleg2oa )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "xyj2_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "orleg2_xyj2_v101cn.u7",  0x000000, 0x800000, CRC(45805b53) SHA1(f2a8399c821b75fadc53e914f6f318707e70787c) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a_text.u4",            0x00000000, 0x0200000, CRC(fa444c32) SHA1(31e5e3efa92d52bf9ab97a0ece51e3b77f52ce8a) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "ig-a_bgl.u35",     0x00000000, 0x0800000, CRC(083a8315) SHA1(0dba25e132fbb12faa59ced648c27b881dc73478) )
	ROM_LOAD32_WORD( "ig-a_bgh.u36",     0x00000002, 0x0800000, CRC(e197221d) SHA1(5574b1e3da4b202db725be906dd868edc2fd4634) )

	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a_bml.u12",     0x00000000, 0x1000000, CRC(113a331c) SHA1(ee6b31bb2b052cc8799573de0d2f0a83f0ab4f6a) )
	ROM_LOAD32_WORD( "ig-a_bmh.u16",     0x00000002, 0x1000000, CRC(fbf411c8) SHA1(5089b5cc9bbf6496ef1367c6255e63e9ab895117) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "ig-a_cgl.u18",     0x00000000, 0x2000000, CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) )
	ROM_LOAD32_WORD( "ig-a_cgh.u26",     0x00000002, 0x2000000, CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) )

	ROM_REGION( 0x1000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "ig-a_sp.u2",              0x00000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) )
ROM_END

ROM_START( kov2nl )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "gsyx_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "gsyx_v302cn.u7",          0x00000000, 0x0800000, CRC(b19cf540) SHA1(25da5804bbfd7ef2cdf5cc5aabaa803d18b98929) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a3_text.u4",           0x00000000, 0x0200000, CRC(214530ff) SHA1(4231a02054b0345392a077042b95779fd45d6c22) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "ig-a3_bgl.u35",    0x00000000, 0x0800000, CRC(2d46b1f6) SHA1(ea8c805eda6292e86a642e9633d8fee7054d10b1) )
	ROM_LOAD32_WORD( "ig-a3_bgh.u36",    0x00000002, 0x0800000, CRC(df710c36) SHA1(f826c3f496c4f17b46d18af1d8e02cac7b7027ac) )

	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a3_bml.u12",    0x00000000, 0x1000000, CRC(0bf63836) SHA1(b8e4f1951f8074b475b795bd7840c5a375b6f5ef) )
	ROM_LOAD32_WORD( "ig-a3_bmh.u16",    0x00000002, 0x1000000, CRC(4a378542) SHA1(5d06a8a8796285a786ebb690c34610f923ef5570) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "ig-a3_cgl.u18",    0x00000000, 0x2000000, CRC(8d923e1f) SHA1(14371cf385dd8857017d3111cd4710f4291b1ae2) )
	ROM_LOAD32_WORD( "ig-a3_cgh.u26",    0x00000002, 0x2000000, CRC(5b6fbf3f) SHA1(d1f52e230b91ee6cde939d7c2b74da7fd6527e73) )

	ROM_REGION( 0x2000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "ig-a3_sp.u37",            0x00000000, 0x2000000, CRC(45cdf422) SHA1(8005d284bcee73cff37a147fcd1c3e9f039a7203) )
ROM_END

ROM_START( kov2nlo )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "gsyx_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "gsyx_v301cn.u7",  0x000000, 0x800000, CRC(c4595c2c) SHA1(09e379556ef76f81a63664f46d3f1415b315f384) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a3_text.u4",           0x00000000, 0x0200000, CRC(214530ff) SHA1(4231a02054b0345392a077042b95779fd45d6c22) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "ig-a3_bgl.u35",    0x00000000, 0x0800000, CRC(2d46b1f6) SHA1(ea8c805eda6292e86a642e9633d8fee7054d10b1) )
	ROM_LOAD32_WORD( "ig-a3_bgh.u36",    0x00000002, 0x0800000, CRC(df710c36) SHA1(f826c3f496c4f17b46d18af1d8e02cac7b7027ac) )

	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a3_bml.u12",    0x00000000, 0x1000000, CRC(0bf63836) SHA1(b8e4f1951f8074b475b795bd7840c5a375b6f5ef) )
	ROM_LOAD32_WORD( "ig-a3_bmh.u16",    0x00000002, 0x1000000, CRC(4a378542) SHA1(5d06a8a8796285a786ebb690c34610f923ef5570) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "ig-a3_cgl.u18",    0x00000000, 0x2000000, CRC(8d923e1f) SHA1(14371cf385dd8857017d3111cd4710f4291b1ae2) )
	ROM_LOAD32_WORD( "ig-a3_cgh.u26",    0x00000002, 0x2000000, CRC(5b6fbf3f) SHA1(d1f52e230b91ee6cde939d7c2b74da7fd6527e73) )

	ROM_REGION( 0x2000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "ig-a3_sp.u37",            0x00000000, 0x2000000, CRC(45cdf422) SHA1(8005d284bcee73cff37a147fcd1c3e9f039a7203) )
ROM_END

ROM_START( kov2nloa )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "gsyx_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "kov2nl_gsyx_v300tw.u7",  0x000000, 0x800000, CRC(08da7552) SHA1(303b97d7694405474c8133a259303ccb49db48b1) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a3_text.u4",           0x00000000, 0x0200000, CRC(214530ff) SHA1(4231a02054b0345392a077042b95779fd45d6c22) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "ig-a3_bgl.u35",    0x00000000, 0x0800000, CRC(2d46b1f6) SHA1(ea8c805eda6292e86a642e9633d8fee7054d10b1) )
	ROM_LOAD32_WORD( "ig-a3_bgh.u36",    0x00000002, 0x0800000, CRC(df710c36) SHA1(f826c3f496c4f17b46d18af1d8e02cac7b7027ac) )

	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a3_bml.u12",    0x00000000, 0x1000000, CRC(0bf63836) SHA1(b8e4f1951f8074b475b795bd7840c5a375b6f5ef) )
	ROM_LOAD32_WORD( "ig-a3_bmh.u16",    0x00000002, 0x1000000, CRC(4a378542) SHA1(5d06a8a8796285a786ebb690c34610f923ef5570) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "ig-a3_cgl.u18",    0x00000000, 0x2000000, CRC(8d923e1f) SHA1(14371cf385dd8857017d3111cd4710f4291b1ae2) )
	ROM_LOAD32_WORD( "ig-a3_cgh.u26",    0x00000002, 0x2000000, CRC(5b6fbf3f) SHA1(d1f52e230b91ee6cde939d7c2b74da7fd6527e73) )

	ROM_REGION( 0x2000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "ig-a3_sp.u37",            0x00000000, 0x2000000, CRC(45cdf422) SHA1(8005d284bcee73cff37a147fcd1c3e9f039a7203) )
ROM_END

ROM_START( ddpdojh )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "ddpdoj_igs036.rom",       0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 )
	ROM_LOAD( "ddpdoj_v201cn.u4",        0x00000000, 0x0200000, CRC(89e4b760) SHA1(9fad1309da31d12a413731b416a8bbfdb304ed9e) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ddpdoj_text.u1",          0x00000000, 0x0200000, CRC(f18141d1) SHA1(a16e0a76bc926a158bb92dfd35aca749c569ef50) )

	ROM_REGION( 0x2000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "ddpdoj_bgl.u23",   0x00000000, 0x1000000, CRC(ff65fdab) SHA1(abdd5ca43599a2daa722547a999119123dd9bb28) )
	ROM_LOAD32_WORD( "ddpdoj_bgh.u24",   0x00000002, 0x1000000, CRC(bb84d2a6) SHA1(a576a729831b5946287fa8f0d923016f43a9bedb) )

	ROM_REGION( 0x1000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ddpdoj_mapl0.u13", 0x00000000, 0x800000, CRC(bcfbb0fc) SHA1(9ec478eba9905913cf997bd9b46c70c1ad383630) )
	ROM_LOAD32_WORD( "ddpdoj_maph0.u15", 0x00000002, 0x800000, CRC(0cc75d4e) SHA1(6d1b5ef0fdebf1e84fa199b939ffa07b810b12c9) )

	ROM_REGION( 0x2000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "ddpdoj_spa0.u9",   0x00000000, 0x1000000, CRC(1232c1b4) SHA1(ecc1c549ae19d2f052a85fe4a993608aedf49a25) )
	ROM_LOAD32_WORD( "ddpdoj_spb0.u18",  0x00000002, 0x1000000, CRC(6a9e2cbf) SHA1(8e0a4ea90f5ef534820303d62f0873f8ac9f080e) )

	ROM_REGION( 0x1000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "ddpdoj_wave0.u12",        0x00000000, 0x1000000, CRC(2b71a324) SHA1(f69076cc561f40ca564d804bc7bd455066f8d77c) )
ROM_END


ROM_START( kov3 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "kov3_igs036.rom",         0x00000000, 0x0004000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* custom ROM module instead of regular ROMs, this might be incorrect - same module is used on newer gambling boards */

	// this was an attempt to read the ROM module directly and could be bad
	ROM_LOAD( "kov3_v102cn_direct.bin",         0x00000000, 0x0800000, CRC(2568cca4) SHA1(3f0e949bc0ae5d7ec0109f2748b30024dcd19ac4) )
	// this was read with a logic analyser after booting, you can't however replace the module directly with this because some kind of
	// additional check / communication with the module is done on startup resulting in the internal ROM refusing to boot it
	ROM_LOAD( "kov3_v102cn.bin",         0x00000000, 0x0800000, CRC(1fcedff3) SHA1(522538510c5f94e8b1f641250c25a2a58962ca42) )

	ROM_REGION( 0x200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "kov3_text.u1",            0x00000000, 0x0200000, CRC(198b52d6) SHA1(e4502abe7ba01053d16c02114f0c88a3f52f6f40) )

	ROM_REGION( 0x2000000, "bgtile", 0 )
	ROM_LOAD32_WORD( "kov3_bgl.u6",      0x00000000, 0x1000000, CRC(49a4c5bc) SHA1(26b7da91067bda196252520e9b4893361c2fc675) )
	ROM_LOAD32_WORD( "kov3_bgh.u7",      0x00000002, 0x1000000, CRC(adc1aff1) SHA1(b10490f0dbef9905cdb064168c529f0b5a2b28b8) )

	ROM_REGION( 0x4000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "kov3_mapl0.u15",   0x00000000, 0x2000000, CRC(9e569bf7) SHA1(03d26e000e9d8e744546be9649628d2130f2ec4c) )
	ROM_LOAD32_WORD( "kov3_maph0.u16",   0x00000002, 0x2000000, CRC(6f200ad8) SHA1(cd12c136d4f5d424bd7daeeacd5c4127beb3d565) )

	ROM_REGION( 0x8000000, "spritesb", 0 ) // sprite colour data
	ROM_LOAD32_WORD( "kov3_spa0.u17",    0x00000000, 0x4000000, CRC(3a1e58a9) SHA1(6ba251407c69ee62f7ea0baae91bc133acc70c6f) )
	ROM_LOAD32_WORD( "kov3_spb0.u10",    0x00000002, 0x4000000, CRC(90396065) SHA1(01bf9f69d77a792d5b39afbba70fbfa098e194f1) )

	ROM_REGION( 0x4000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770 */
	ROM_LOAD16_WORD_SWAP( "kov3_wave0.u13",              0x00000000, 0x4000000, CRC(aa639152) SHA1(2314c6bd05524525a31a2a4668a36a938b924ba4) )
ROM_END


static void iga_u16_decode(UINT16 *rom, int len, int ixor)
{
	int i;

	for (i = 1; i < len / 2; i+=2)
	{
		UINT16 x = ixor;

		if ( (i>>1) & 0x000001) x ^= 0x0010;
		if ( (i>>1) & 0x000002) x ^= 0x2004;
		if ( (i>>1) & 0x000004) x ^= 0x0801;
		if ( (i>>1) & 0x000008) x ^= 0x0300;
		if ( (i>>1) & 0x000010) x ^= 0x0080;
		if ( (i>>1) & 0x000020) x ^= 0x0020;
		if ( (i>>1) & 0x000040) x ^= 0x4008;
		if ( (i>>1) & 0x000080) x ^= 0x1002;
		if ( (i>>1) & 0x000100) x ^= 0x0400;
		if ( (i>>1) & 0x000200) x ^= 0x0040;
		if ( (i>>1) & 0x000400) x ^= 0x8000;

		rom[i] ^= x;
		rom[i] = BITSWAP16(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

static void iga_u12_decode(UINT16* rom, int len, int ixor)
{
	int i;

	for (i = 0; i < len / 2; i+=2)
	{
		UINT16 x = ixor;

		if ( (i>>1) & 0x000001) x ^= 0x9004;
		if ( (i>>1) & 0x000002) x ^= 0x0028;
		if ( (i>>1) & 0x000004) x ^= 0x0182;
		if ( (i>>1) & 0x000008) x ^= 0x0010;
		if ( (i>>1) & 0x000010) x ^= 0x2040;
		if ( (i>>1) & 0x000020) x ^= 0x0801;
		if ( (i>>1) & 0x000040) x ^= 0x0000;
		if ( (i>>1) & 0x000080) x ^= 0x0000;
		if ( (i>>1) & 0x000100) x ^= 0x4000;
		if ( (i>>1) & 0x000200) x ^= 0x0600;
		if ( (i>>1) & 0x000400) x ^= 0x0000;

		rom[i] ^= x;
		rom[i] = BITSWAP16(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

DRIVER_INIT_MEMBER(pgm2_state,orleg2)
{
	UINT16 *src = (UINT16 *)memregion("spritesa")->base();

	iga_u12_decode(src, 0x2000000, 0x4761);
	iga_u16_decode(src, 0x2000000, 0xc79f);

	igs036_decryptor decrypter(orleg2_key);
	decrypter.decrypter_rom(memregion("user1"));

	pgm_create_dummy_internal_arm_region(0x80003c9);
}

DRIVER_INIT_MEMBER(pgm2_state,kov2nl)
{
	UINT16 *src = (UINT16 *)memregion("spritesa")->base();

	iga_u12_decode(src, 0x2000000, 0xa193);
	iga_u16_decode(src, 0x2000000, 0xb780);

	igs036_decryptor decrypter(kov2_key);
	decrypter.decrypter_rom(memregion("user1"));

	pgm_create_dummy_internal_arm_region(0x8000069);
}

DRIVER_INIT_MEMBER(pgm2_state,ddpdojh)
{
	UINT16 *src = (UINT16 *)memregion("spritesa")->base();

	iga_u12_decode(src, 0x800000, 0x1e96);
	iga_u16_decode(src, 0x800000, 0x869c);

	igs036_decryptor decrypter(ddpdoj_key);
	decrypter.decrypter_rom(memregion("user1"));

	pgm_create_dummy_internal_arm_region(0x80003c9);
}

DRIVER_INIT_MEMBER(pgm2_state,kov3)
{
	UINT16 *src = (UINT16 *)memregion("spritesa")->base();

	iga_u12_decode(src, 0x2000000, 0x956d);
	iga_u16_decode(src, 0x2000000, 0x3d17);

	igs036_decryptor decrypter(kov3_key);
	decrypter.decrypter_rom(memregion("user1"));

	pgm_create_dummy_internal_arm_region(0x80003c9);
}


/* PGM2 */
GAME( 2007, orleg2,       0,         pgm2,    pgm2, pgm2_state,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V104, China)", MACHINE_IS_SKELETON )
GAME( 2007, orleg2o,      orleg2,    pgm2,    pgm2, pgm2_state,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V103, China)", MACHINE_IS_SKELETON )
GAME( 2007, orleg2oa,     orleg2,    pgm2,    pgm2, pgm2_state,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V101, China)", MACHINE_IS_SKELETON )
// should be earlier verisons too.

GAME( 2008, kov2nl,       0,         pgm2,    pgm2, pgm2_state,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V302, China)", MACHINE_IS_SKELETON )
GAME( 2008, kov2nlo,      kov2nl,    pgm2,    pgm2, pgm2_state,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V301, China)", MACHINE_IS_SKELETON )
GAME( 2008, kov2nloa,     kov2nl,    pgm2,    pgm2, pgm2_state,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V300, Taiwan)", MACHINE_IS_SKELETON )
// should be earlier verisons too.

GAME( 2010, ddpdojh,      0,    pgm2,    pgm2, pgm2_state,     ddpdojh,    ROT270, "IGS", "Dodonpachi Daioujou Tamashii (V201, China)", MACHINE_IS_SKELETON )
// should be earlier verisons too.

GAME( 2011, kov3,         0,    pgm2,    pgm2, pgm2_state,     kov3,       ROT0, "IGS", "Knights of Valour 3 (V102, China)", MACHINE_IS_SKELETON )
// should be earlier verisons too.

// The King of Fighters '98 - Ultimate Match - Hero
// Jigsaw World Arena
//Puzzle of Ocha / Ochainu No Pazuru
