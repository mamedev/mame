/***************************************************************************

    namcops2.c

    Namco System 246 / System 256 games (Sony PS2 based)

    PS2 baseboard includes:
    * R5900 "Emotion Engine" - MIPS III with 128-bit integer regs & SIMD
    * R3000 IOP - Stock R3000 with cache, not like the PSXCPU
    * VU0 - can operate either as in-line R5900 coprocessor or run independently
    * VU1 - runs independently only, has special DMA path to GS
    * GS - Graphics Synthesizer.  Nothing fancy, draws flat, Gouraud, and/or
      textured tris with no, bilinear, or trilinear filtering very quickly.
    * SPU2 - Sound Processing Unit.  Almost literally 2 PS1 SPUs stuck together,
      with 2 MB of wave RAM, a 48 kHz sample rate (vs. 44.1 on PS1), and 2
      stereo DMADACs.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/mips/r3000.h"


class namcops2_state : public driver_device
{
public:
	namcops2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
};


void namcops2_state::video_start()
{
}

bool namcops2_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START(ps2_map, AS_PROGRAM, 32, namcops2_state)
	AM_RANGE(0x00000000, 0x01ffffff) AM_RAM	// 32 MB RAM in consumer PS2s, do these have more?
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( system246 )
INPUT_PORTS_END

static const mips3_config r5000_config =
{
	16384,				/* code cache size - probably wrong */
	16384				/* data cache size */
};

static MACHINE_CONFIG_START( system246, namcops2_state )
	MCFG_CPU_ADD("maincpu", R5000LE, 294000000)	// actually R5900 @ 294 MHz
	MCFG_CPU_PROGRAM_MAP(ps2_map)
	MCFG_CPU_CONFIG(r5000_config)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_PALETTE_LENGTH(65536)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system256, system246 )
MACHINE_CONFIG_END

#define SYSTEM246_BIOS	\
        ROM_LOAD( "r27v1602f.7d", 0x000000, 0x200000, CRC(2b2e41a2) SHA1(f0a74bbcaf801f3fd0b7002ebd0118564aae3528) )

#define SYSTEM256_BIOS	\
        ROM_LOAD( "r27v1602f.8g", 0x000000, 0x200000, CRC(b2a8eeb6) SHA1(bc4fb4e1e53adbd92385f1726bd69663ff870f1e) )

ROM_START( sys246 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	DISK_REGION("dvd")
ROM_END

ROM_START( sys256 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	DISK_REGION("dvd")
ROM_END

ROM_START( dragchrn )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "dc001vera.ic002", 0x000000, 0x800000, CRC(923351f0) SHA1(b34c46836af8fa7ab164156a70120da38fa1c31f) )
        ROM_LOAD( "dc001vera_spr.ic002", 0x800000, 0x040000, CRC(1f42dca9) SHA1(10f75649653b4cfa53c25f6c08308e404ed7b0f2) )

	DISK_REGION("dvd")
ROM_END

ROM_START( fghtjam )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "jam1vera.ic002", 0x000000, 0x800000, CRC(61cf3746) SHA1(165195a773bac717b5701647bca4073d86906f4e) )
        ROM_LOAD( "jam1vera_spr.ic002", 0x800000, 0x040000, CRC(5ff79918) SHA1(60146cddc3474cd4c5b51d13cf116dce1664a759) )

	DISK_REGION("dvd")
ROM_END

ROM_START( kinniku )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "kn1vera.ic002", 0x000000, 0x800000, CRC(17aac6c3) SHA1(dddf37e88385f01bba27496d03f053fdc33882e2) )
        ROM_LOAD( "kn1vera_spr.ic002", 0x800000, 0x040000, CRC(a601f981) SHA1(39485ab3c10f3d58a2c9651cca82a73617b2fe52) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "kn1-b", 0, SHA1(2f0f9ebe74cdafe3713890221532b4d1dc18c74f) )
ROM_END

ROM_START( netchu02 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "npy1verb.ic002", 0x000000, 0x800000, CRC(43c0f334) SHA1(5a7f6d607ae012b8477ff32cdfd091b765264499) )
        ROM_LOAD( "npy1verb_spr.ic002", 0x800000, 0x040000, CRC(6a3374f0) SHA1(0c0845edc0ac0e9871e65caade8b4157614b81eb) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "npy1cd0b", 0, SHA1(514adcd2d4205873b3d144a05c033822344798e3) )
ROM_END

ROM_START( soulclb2 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "sc23vera.ic002", 0x000000, 0x800000, CRC(5c537182) SHA1(ff4213db24b1200b494e6c3bd3eb7b75789e4032) )
        ROM_LOAD( "sc23vera_spr.ic002", 0x800000, 0x040000, CRC(8f548cbc) SHA1(81b844dc5873bb397cd4cd5aca101d7486d60385) )
	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc21-dvd0d", 0, SHA1(9a7b1ea836adc9d78481928a3067530e0f8d74a6) )
ROM_END

ROM_START( soulcl2a )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "sc22vera.ic002", 0x000000, 0x800000, CRC(2a1031b4) SHA1(81ad0b9273734758da917c62910906f06e774bd6) )
        ROM_LOAD( "sc22vera_spr.ic002", 0x800000, 0x040000, CRC(6dd152e4) SHA1(1eb23b2c65f12b39fecf34d6b21916165441ebe4) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc21-dvd0d", 0, SHA1(9a7b1ea836adc9d78481928a3067530e0f8d74a6) )
ROM_END

ROM_START( soulcl2b )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "sc21vera.ic002", 0x000000, 0x800000, CRC(7e92ceeb) SHA1(0c8d9337476c04f30ed86c7a77996f81733c1953) )
        ROM_LOAD( "sc21vera_spr.ic002", 0x800000, 0x040000, CRC(f5502fdf) SHA1(064196982d855bd41bafe97db5ff5694b933016a) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc21-dvd0d", 0, SHA1(9a7b1ea836adc9d78481928a3067530e0f8d74a6) )
ROM_END

ROM_START( soulclb3 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "sc31001-na-a.ic002", 0x000000, 0x800000, CRC(ddbe9774) SHA1(6bb2d31cb669336345b5508bcca56936ea97c04a) )
        ROM_LOAD( "sc31001-na-a_spr.ic002", 0x800000, 0x040000, CRC(18c6f56d) SHA1(13bc6a3688985c0cd9900b063824a4af691a1b31) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc31001-na-dvd0-b", 0, SHA1(b46ee35083f8fcc091ce562951c55fbdbb929e4b) )
ROM_END

ROM_START( sukuinuf )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "in2vera.ic002", 0x000000, 0x800000, CRC(bba7a744) SHA1(c1c6857317d0d6648898e9b51d4c693b83e49f16) )
        ROM_LOAD( "in2vera_spr.ic002", 0x800000, 0x040000, CRC(c43fed95) SHA1(b6001dc8ff34198400a7bf3e41e5ab73823685b0) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "hm-in2", 0, SHA1(4e2d95798a2bcc6f93bc82c364379a3936d68986) )
ROM_END

ROM_START( taiko9 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "tk91001-na-a.ic002", 0x000000, 0x800000, CRC(db4efc9a) SHA1(a24f10c726f5bc7313559a515d5c4c34cd129c97) )
        ROM_LOAD( "tk91001-na-a_spr.ic002", 0x800000, 0x040000, CRC(99ece8c0) SHA1(871b1c76ccc0311da04b81c59240e65117cbc9f4) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tk9100-1-na-dvd0-a", 0, SHA1(6bd40b2c19f30a81689601c3dd46b6dac6d0a2f1) )
ROM_END

ROM_START( taiko10 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "t101001-na-a.ic002", 0x000000, 0x800000, CRC(fa7f4c4d) SHA1(4f6b24243f2c2fdffadc7acaa3a6fb668e497606) )
        ROM_LOAD( "t101001-na-a_spr.ic002", 0x800000, 0x040000, CRC(0a2926c4) SHA1(fb3d23545b5f9a649c4a14b6424c606139723bd5) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tk10100-1-na-dvd0-a", 0, SHA1(9aef4a6b64295a6684d56334904b4c92a20abe15) )
ROM_END

ROM_START( tekken4 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "tef3verc.ic002", 0x000000, 0x800000, CRC(8a41290c) SHA1(2c674e3203c7b5302430b1c1115fcf591a0dcbf2) )
        ROM_LOAD( "tef3verc_spr.ic002", 0x800000, 0x040000, CRC(af248bf7) SHA1(b99193fcdad683c0bbd684f37dfea5c5412b398e) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tef1dvd0", 0, SHA1(f39aa37156245f622a6e19e8a0e081418e247b36) )
ROM_END

ROM_START( tekken4a )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "tef2vera.ic002", 0x000000, 0x800000, CRC(6dbbde96) SHA1(101711f36fe428f3fdb5de88cb03efccebc6e68d) )
        ROM_LOAD( "tef2vera_spr.ic002", 0x800000, 0x040000, CRC(a95fd114) SHA1(669229d47d49a511ab77a6f9b8c8541c00d478cf) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tef1dvd0", 0, SHA1(f39aa37156245f622a6e19e8a0e081418e247b36) )
ROM_END

ROM_START( tekken4b )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "tef1vera.bin", 0x000000, 0x800000, CRC(154c615b) SHA1(3823daa6dd5e8d9699f8d832d7ca690559b84e96) )
        ROM_LOAD( "tef1vera.spr", 0x800000, 0x040000, CRC(64e12053) SHA1(04383cf928b4fd82290d7cccc7b23104fbf2c2f2) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tef1dvd0", 0, SHA1(f39aa37156245f622a6e19e8a0e081418e247b36) )
ROM_END

ROM_START( tekken51 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "te51verb.ic002", 0x000000, 0x800000, CRC(b4031e38) SHA1(72ee2aea4032e9b03a735b1b6c7574233f0c7711) )
        ROM_LOAD( "te51verb_spr.ic002", 0x800000, 0x040000, CRC(683bad0d) SHA1(ef10accbdc82143c31d29e2b8b812a209b341b1b) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "te51-dvd0", 0, SHA1(2a0ac3723725572c1810b0ef4bcfa7aa114062f8) )
ROM_END

ROM_START( zgundm )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "zga1vera.ic002", 0x000000, 0x800000, CRC(b9e0fcdc) SHA1(ed7329351e951b5a2aed893e55311018547b852b) )
        ROM_LOAD( "zga1vera_spr.ic002", 0x800000, 0x040000, CRC(8e4c715b) SHA1(a2218051f54d5ce4cdd21ef021b9acf7a384b766) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "zga1dvd0", 0, SHA1(7930e5a65f6079851438669dfb1f0e5f9e11329a) )
ROM_END

ROM_START( timecrs3 )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "tst1vera.ic002", 0x000000, 0x800000, CRC(2c7ede91) SHA1(b3d3547f5aac402da2fe76ef51dca3841a982a5e) )
        ROM_LOAD( "tst1vera_spr.ic002", 0x800000, 0x040000, CRC(ee9c8132) SHA1(fb00e102389e2163d2c7efcfefd4f680f0b4d4e8) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tst1dvd0", 0, SHA1(f8a447d9a4224282516bea590f5217c751bdc4ae) )
ROM_END

ROM_START( zgundmdx )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
        ROM_LOAD( "zdx1vera.ic002", 0x000000, 0x800000, CRC(ffcb6f3b) SHA1(57cae327a0af3f6a77291d6cda948d1349a43c00) )
        ROM_LOAD( "zdx1vera_spr.ic002", 0x800000, 0x040000, CRC(16446b28) SHA1(65bdcf216917beec7a36ff640e16aa5cf413c5e4) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "zdx1dvd0", 0, SHA1(fa21626f771106e2441c4515a0e5dff478187ccd) )
ROM_END

ROM_START( gundzaft )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sed1vera.ic002", 0x000000, 0x800000, CRC(db52309d) SHA1(3e325dfa68dadcc2f9abd9d338e47ffa511e73f8) )
	ROM_LOAD( "sed1vera_spr.ic002", 0x800000, 0x040000, CRC(12641e0e) SHA1(64b7655f95a2e5e41b5a89998f2b858dab05ae75) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sed1dvd0", 0, SHA1(0e6db61d94f66a4ddd7d4a3013983a838d256c5d) )
ROM_END

ROM_START( rrvac )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "rrv3vera.ic002", 0x000000, 0x800000, CRC(dd20c4a2) SHA1(07bddaac958ac62d9fc29671fc83bd1e3b27f4b8) )
	ROM_LOAD( "rrv3vera_spr.ic002", 0x800000, 0x040000, CRC(712e0e9a) SHA1(d396aaf918036ff7f909a84daefe8f651fdf9b05) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "rrv1-a", 0, SHA1(77bb70407511cbb12ab999410e797dcaf0779229) )
ROM_END

ROM_START( scptour )
	ROM_REGION(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "scp1vera.ic002", 0x000000, 0x800000, CRC(4743a999) SHA1(97ae15d75dd9b80411d101b97dd215e31de56390) )
	ROM_LOAD( "scp1vera_spr.ic002", 0x800000, 0x040000, CRC(b7094978) SHA1(1e4903cd5f594c13dad2fd74666ba35c62550044) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "scp1cd0", 0, SHA1(19fa70ba22787704c40f0a8f27bc841218bbc99b) )
ROM_END

GAME(2001, sys246,          0, system246, system246, 0, ROT0, "Namco", "System 246 BIOS", GAME_IS_SKELETON|GAME_IS_BIOS_ROOT)
GAME(2001, rrvac,      sys246, system246, system246, 0, ROT0, "Namco", "Ridge Racer V Arcade Battle (RRV3 Ver. A)", GAME_IS_SKELETON)
GAME(2002, dragchrn,   sys246, system246, system246, 0, ROT0, "Namco", "Dragon Chronicles (DC001 Ver. A)", GAME_IS_SKELETON)
GAME(2002, netchu02,   sys246, system246, system246, 0, ROT0, "Namco", "Netchuu Pro Yakyuu 2002 (NPY1 Ver. A)", GAME_IS_SKELETON)
GAME(2002, scptour,    sys246, system246, system246, 0, ROT0, "Namco", "Smash Court Pro Tournament (SCP1)", GAME_IS_SKELETON)
GAME(2002, soulclb2,   sys246, system246, system246, 0, ROT0, "Namco", "Soul Calibur II (SC23 Ver. A)", GAME_IS_SKELETON)
GAME(2002, soulcl2a, soulclb2, system246, system246, 0, ROT0, "Namco", "Soul Calibur II (SC22 Ver. A)", GAME_IS_SKELETON)
GAME(2002, soulcl2b, soulclb2, system246, system246, 0, ROT0, "Namco", "Soul Calibur II (SC21 Ver. A)", GAME_IS_SKELETON)
GAME(2002, tekken4,    sys246, system246, system246, 0, ROT0, "Namco", "Tekken 4 (TEF3 Ver. C)", GAME_IS_SKELETON)
GAME(2002, tekken4a,  tekken4, system246, system246, 0, ROT0, "Namco", "Tekken 4 (TEF2 Ver. A)", GAME_IS_SKELETON)
GAME(2002, tekken4b,  tekken4, system246, system246, 0, ROT0, "Namco", "Tekken 4 (TEF1 Ver. A)", GAME_IS_SKELETON)
GAME(2003, timecrs3,   sys246, system246, system246, 0, ROT0, "Namco", "Time Crisis 3 (TST1)", GAME_IS_SKELETON)
GAME(2003, zgundm,     sys246, system246, system246, 0, ROT0, "Capcom / Banpresto", "Mobile Suit Z-Gundam: A.E.U.G. vs Titans (ZGA1 Ver. A)", GAME_IS_SKELETON)
GAME(2004, fghtjam,    sys246, system246, system246, 0, ROT0, "Capcom / Namco", "Capcom Fighting Jam (JAM1 Ver. A)", GAME_IS_SKELETON)
GAME(2004, sukuinuf,   sys246, system246, system246, 0, ROT0, "Namco", "Quiz and Variety Suku Suku Inufuku 2 (IN2 Ver. A)", GAME_IS_SKELETON)
GAME(2004, zgundmdx,   sys246, system246, system246, 0, ROT0, "Capcom / Banpresto", "Mobile Suit Z-Gundam: A.E.U.G. vs Titans DX (ZDX1 Ver. A)", GAME_IS_SKELETON)
GAME(2005, gundzaft,   sys246, system246, system246, 0, ROT0, "Capcom / Banpresto", "Gundam Seed: Federation vs. Z.A.F.T. (SED1 Ver. A)", GAME_IS_SKELETON)
GAME(2005, soulclb3,   sys246, system246, system246, 0, ROT0, "Namco", "Soul Calibur III (SC31001-NA-A)", GAME_IS_SKELETON)

GAME(2004, sys256,          0, system256, system246, 0, ROT0, "Namco", "System 256 BIOS", GAME_IS_SKELETON|GAME_IS_BIOS_ROOT)
GAME(2005, tekken51,   sys256, system256, system246, 0, ROT0, "Namco", "Tekken 5.1 (TE51 Ver. B)", GAME_IS_SKELETON)
GAME(2006, kinniku,    sys256, system256, system246, 0, ROT0, "Namco", "Kinnikuman Muscle Grand Prix (KN1 Ver. A)", GAME_IS_SKELETON)
GAME(2006, taiko9,     sys256, system256, system246, 0, ROT0, "Namco", "Taiko No Tatsujin 9 (TK91001-NA-A)", GAME_IS_SKELETON)
GAME(2007, taiko10,    sys256, system256, system246, 0, ROT0, "Namco", "Taiko No Tatsujin 10 (T101001-NA-A)", GAME_IS_SKELETON)
