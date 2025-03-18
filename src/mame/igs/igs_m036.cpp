// license:BSD-3-Clause
// copyright-holders: David Haywood

/* IGS Gambling games using IGS036 CPU
   pgm2.cpp also uses this CPU

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

#include "igs036crypt.h"

#include "cpu/arm7/arm7.h"
#include "sound/okim6295.h"
#include "sound/tt5665.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class igs_m036_state : public driver_device
{
public:
	igs_m036_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	void igs_m036_tt(machine_config &config);
	void igs_m036(machine_config &config);

	void init_igs_m036();
	void init_cjdh2();
	void init_cjddzsp();
	void init_igsm312();

private:
	uint32_t screen_update_igs_m036(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;

	void pgm_create_dummy_internal_arm_region(void);

	void igs_m036_map(address_map &map) ATTR_COLD;
};




uint32_t igs_m036_state::screen_update_igs_m036(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void igs_m036_state::igs_m036_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom(); // Internal ROM
	map(0x08000000, 0x081fffff).rom().region("user1", 0); // not 100% sure it maps here.

}

static INPUT_PORTS_START( igs_m036 )
INPUT_PORTS_END



ROM_START( cjdh2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cn.u33",  0x000000, 0x200000, CRC(a6fb72f0) SHA1(1d9583eafaea21d5ec078b7f2e3dc426571a9550) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2a )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cna.u33", 0x000000, 0x200000, CRC(0bc6bc1b) SHA1(c891a7051cda1fd250d9380d7f33b47c375db74d) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2b )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s311cnb.u33", 0x000000, 0x200000, CRC(ddcf50bd) SHA1(39a3ed728be5894a2fec5cf0858f6f40be5ccae1) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END

ROM_START( cjdh2c )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "chaohen2_igs036", 0x00000, 0x4000, NO_DUMP )

	// there is also a square socketed chip like the one on Haunted House (igs_m027) probably in need of dumping

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjdh2_s215cn.u33",  0x000000, 0x200000, CRC(ebe35131) SHA1(1f167e70a80b39e0658fd97c249982a0aa622683) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "cjdh2_sp.u20", 0x000000, 0x200000, CRC(14a20112) SHA1(de49ecbc6ffd89e8d2e0a4cf1f4cba1a78810d42) )

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjdh2_cg1.u8",  0x000000, 0x800100, CRC(c14bf4b2) SHA1(32bdd7c498b75f3444bb6a6ccf0981d5dd46028c) )
	ROM_LOAD( "cjdh2_cg2.u24", 0x800100, 0x800100, CRC(f9c747c3) SHA1(c4ff67e9da1322536841b8a9e9d9cfea6d7ebc4a) )
ROM_END


ROM_START( cjddzsp )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cjddzsp_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "cjddzsp_s122cn.u18",  0x000000, 0x200000, CRC(4a42aad6) SHA1(96805e5bfbd50686177fe50020229ea8787ade17) )

	ROM_REGION( 0x800100, "tt5665", 0 ) // samples
	ROM_LOAD( "cjddzsp_s122cn.u27", 0x000000, 0x800100, CRC(797e5ba3) SHA1(784fae513ac8cfd1143f0d0ce0936f74e2e64e48))

	ROM_REGION( 0x800100*2, "gfx", 0 )
	ROM_LOAD( "cjddzsp_s122cn.u28",  0x000000, 0x800100, CRC(d0441a6b) SHA1(e1c948f94472398aa5887963cf8e87be28dd66e0) )
	ROM_LOAD( "cjddzsp_s122cn.u30",  0x800100, 0x800100, CRC(e0e02a57) SHA1(96074a5226dd24d0bc150adff7324b5349cb5dc2) )
ROM_END


/*******************************************************************
Long Hu Zheng Ba Te Bie Ban, IGS, 2009
Que Huang Zheng Ba, IGS, 2007
(Year not shown on title screens but date shown on error screen when program ROM removed)
Hardware Info by Guru
---------------------

PCB-0701-01-IU   (Que Huang Zheng Ba)
PCB-0799-02-IU-1 (Long Hu Zheng Ba Te Bie Ban)
|----|  |-------------------------------|  |------|
|    |--|             JAMMA             |--|      |
|                              ULN2004    TDA1519 |
|-| 7407                       ULN2004        VOL |
  |     LV245 LV245                  4.952MHz     |
|-|      LV245 LV245  EPM3032   |------|    UPC844|
|       20MHz                   |TT5665|       U43|
|1        |------|              |------| QS3257   |
|8        |IGS036|                       QS3257   |
|W        |      |                                |
|A        |------|                 U27        CON3|
|Y  7404     32.768kHz                            |
|   7404                           U28            |
|-| HC132     U18                                 |
  | SW4    16LV1024                U29            |
  |                     LV245                  JP8|
|-| BATT                           U30            |
|           1117-18       SW1                     |
|10   TYN408G             SW2      U31            |
|WAY        1084-33       SW3                     |
|                                              JP7|
|-------------------------------------------------|
Notes:
      IGS036 - Custom SOC in BGA package with internal ROM. SOC is possibly ARM-based. Clock input 20.000MHz
      TT5665 - Tontek Design Technology TT5665 8-Channel (or 4-Channel L/R Stereo) ADPCM Voice Synthesis LSI
               Clock input 4.952MHz, pin 25 (SS) high, S0 and S1 low. Game sound is very low quality.
     EPM3032 - Altera EPM3032 CPLD in PLCC44 package
               Sticker on Que Huang Zheng Ba: "IU U23"
               Sticker on Long Hu Zheng Ba Te Bie Ban: "IU-1 U23"
     TDA1519 - NXP TDA1519 6W Stereo Power Amplifier IC
      UPC844 - NEC uPC844 Quad Operational Amplifier
      QS3257 - IDT QS3257 Mux/Demux (=74LS257)
     ULN2004 - Texas Instruments ULN2004 Darlington Array
       LV245 - 74LV245A Octal Bus Transceiver
        7407 - 7407D Hex Buffer
        7404 - 7404D Hex Inverter
       HC132 - 74HC132D Quadruple 2-Input Positive-NAND Schmitt Trigger
    16LV1024 - Chiplus CS16LV10243GCR70 64kBx16-bit SRAM
         U43 - PS/2 Port (not populated on PCB-0799)
        CON3 - DB9 Port (not populated on PCB-0799)
         JP7 - 4-pin power connector
         JP8 - 10-pin connector (not populated, might be for JTAG programming)
        BATT - 3V Coin Battery
       SW1-3 - 8-position DIP Switch
         SW4 - SPDT Switch (clears high scores/NVRAM)
     1117-18 - 1.8V 800mA LDO Regulator
     1084-33 - 3.3V 5A LDO Regulator
     TYN408G - ST TYN408G SCR
         U18 - EV29LV160AB 2Mx8-bit/1Mx16-bit DIP42 3.3V Flash ROM. ROM data is 16-bit.
               The ROM is not pin compatible with 27C160 and must be dumped using a custom adapter.
               This ROM is the game-specific program code. There is common code inside the IGS036 SOC because
               if the program ROM is removed and booted the screen shows 'PROGRAM ROM ERROR'
                - Que Huang Zheng Ba: V100CN_U18.U18
                - Long Hu Zheng Ba Te Bie Ban: S101CN_U18.U18
     U27-U31 - EV29LV640MT 8Mx8-bit/4Mx16-bit DIP48 Flash ROM (U29 & U31 not populated). ROM data is 16-bit.
               U27 - Audio Samples for TT5665
               Other ROMs - Graphics

*******************************************************************/

ROM_START( lhtb ) // PCB-0799-02-IU-1, every ROM label starts with 龍虎特別版
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "cn1012_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "s-101cn.u18",  0x000000, 0x200000, CRC(1020f4b5) SHA1(953bb776a804738c624a1dca336e42beb10238f7) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "s101cn_u27.u27", 0x000000, 0x800000, CRC(8db377ae) SHA1(769c146104d8577b337d5aa8b388b9c8b726dd21) )

	ROM_REGION( 0x800000*2, "gfx", 0 )
	ROM_LOAD( "s101cn_u28.u28",  0x000000, 0x800000, CRC(75029040) SHA1(d1d0fd696a6c2819034b089e5d2e97b6d5ccc5f9) )
	// u29 not populated
	ROM_LOAD( "s101cn_u30.u30",  0x800000, 0x800000, CRC(4024bd96) SHA1(c3f1c616c9783560a5ea0f8727e39c20c64d46f2) )
	// u31 not populated, etched CG1-L on PCB
ROM_END

ROM_START( qhzb ) // PCB-0701-01-IU
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "qhzb_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v100cn_u18.u18", 0x000000, 0x200000, CRC(8c2733b7) SHA1(226768eead8d7754b2468384e5e691812d0ac96d) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "sp_u27.u27", 0x000000, 0x800000, CRC(624ff81a) SHA1(99f06f725b761418e88198493276982e367bd432) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x800000*2, "gfx", 0 )
	ROM_LOAD( "cg_u28.u28",  0x000000, 0x800000, CRC(04105f6f) SHA1(e917825c2304bba3362aecbfab73d2e2b340478e) ) // FIXED BITS (xxxxxxxx0000xxxx)
	// u29 not populated
	ROM_LOAD( "cg_u30.u30",  0x800000, 0x800000, CRC(37c7d442) SHA1(81682aa1ba6912919c85ee8f68c2e9821f4a3171) )
	// u31 not populated, etched CG1-L on PCB
ROM_END


ROM_START( lhzb3in1 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "lhzb3in1_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb3in1_v100cn.u17",  0x000000, 0x200000, CRC(03caaba4) SHA1(701b97d791e9329bad2ddc4d365748e65c430758) )

	ROM_REGION( 0x1000000, "tt5665", 0 ) // samples
	ROM_LOAD( "lhzb3in1_v100cn.u29", 0x000000, 0x800000, CRC(d8c160a9) SHA1(4b567571764db679a265ae075136128db495acdd) )
	ROM_LOAD( "lhzb3in1_v100cn.u28", 0x800000, 0x800000, CRC(68624630) SHA1(56e638d59c4533136f69db22f562b39120b516c1) ) // 11xxxxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROM_LOAD( "lhzb3in1_v100cn.u30",  0x0000000, 0x800000, CRC(fb4124d7) SHA1(324fe2ade17b0ee9833decf2cab9dd4654a04cec) )
	ROM_LOAD( "lhzb3in1_v100cn.u31",  0x0800000, 0x800000, CRC(4572ff90) SHA1(5d4a40ddec1505edc8a1e35130abd7f2c97b1094) ) // FIXED BITS (xxxxxxxx0000xxxx)
	ROM_LOAD( "lhzb3in1_v100cn.u32",  0x1000000, 0x800000, CRC(04fe8ca2) SHA1(039009dd535e1388236bd0fd699eeaf593ae5323) )
	ROM_LOAD( "lhzb3in1_v100cn.u33",  0x1800000, 0x800000, CRC(9afa55d1) SHA1(0a19e1c54b271b21fb9931e7c81a9e7d9e77295a) )
ROM_END


ROM_START( igsm312 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "igsunk_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "m312cn.rom", 0x000000, 0x200000, CRC(5069c310) SHA1(d53a2e8acddfbb7afc27c68c0b3167419a3ec3e6) )

	ROM_REGION( 0x800100, "tt5665", ROMREGION_ERASE00 ) // samples
	// missing

	ROM_REGION( 0x800100*2, "gfx", ROMREGION_ERASE00 )
	// missing
ROM_END


// PCB-0999-00-KO main with PCB-0634-02-IN riser board for GFX ROMs + PCB-0998-01-KO-C for I/O
ROM_START( mghammer )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036E ARM based MCU (1119 0T7643 IGS036E)
	ROM_LOAD( "mghammer_igs036e", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v_100jp.u26", 0x000000, 0x200000, CRC(d78a4dbb) SHA1(149e68c1294b31f4b039d2cd36d36f17873c247e) )

	ROM_REGION( 0x1000000, "tt5665", ROMREGION_ERASE00 ) // samples
	ROM_LOAD( "sp_u17.u17", 0x000000, 0x800000, CRC(dd9b43b6) SHA1(5fa3191a2ebb0ea7fb737e44b1f651987c6c1bbb) )
	ROM_LOAD( "sp_u18.u18", 0x800000, 0x800000, CRC(0998e0c6) SHA1(bfa2b42248dc52f83682bbdec7bb7aaa53b4ac29) )

	ROM_REGION( 0x4000400, "gfx", 0 )
	ROM_LOAD( "cg_v_100jp.u1",  0x0000000, 0x1000100, CRC(6800418e) SHA1(81eb23f8d5c6d4eeac6bf7b06897d88d744b9681) )
	ROM_LOAD( "cg_v_100jp.u2",  0x1000100, 0x1000100, CRC(decffe69) SHA1(6161208eb399a8f418244c19e0236b4db79d0300) )
	ROM_LOAD( "cg_v_100jp.u3",  0x2000200, 0x1000100, CRC(6b6df061) SHA1(ba1e020626ce50a7fc5859f41ec717b0b39229cb) )
	ROM_LOAD( "cg_v_100jp.u4",  0x3000300, 0x1000100, CRC(5813401d) SHA1(7675a691ce0f6b99fa15a7c0004a733121772c3c) )

	ROM_REGION( 0x04000, "iocpu", 0 )
	// Internal ROM of IGS036E ARM based MCU (1119 0T7643 IGS036E), on I/O board (yes, same IGS036 code as main)
	ROM_LOAD( "io_mghammer_igs036e", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "io", 0 ) // external ARM data / prg
	ROM_LOAD( "io_v_100jp.u1", 0x000000, 0x200000, CRC(cf6c7440) SHA1(53d7b8240b153045569cac72683670df45c6ed32) )
ROM_END


ROM_START( lhfy )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "lhfy_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-206cn.u11", 0x000000, 0x200000, CRC(45bd9c9f) SHA1(3a9b06bf9c66520136522d67ed12f800569580f5) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v-206cn.u19", 0x000000, 0x800000, CRC(f7990ed4) SHA1(e8a72bc0926911ba5c079b02dd324ac060e8c768) ) // same as lhzbgqb

	ROM_REGION( 0x10000000, "gfx", ROMREGION_ERASE00 )
	// 4x 64MB flash ROMs (U1, U2, U3, U4) mounted onto a custom SODIMM at CN1 with a sticker "CG V206CN"
	ROM_LOAD( "cg_v206cn.u1", 0x0000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v206cn.u2", 0x4000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v206cn.u3", 0x8000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v206cn.u4", 0xc000000, 0x4000000, NO_DUMP )
ROM_END


ROM_START( lhzbgqb )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "lhzbgqb_igs036", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-105cn.u11", 0x000000, 0x200000, CRC(cecbb560) SHA1(262d7df19b57e57aa50cc241ddc25090411d6b74) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v105cn_u19.u19", 0x000000, 0x800000, CRC(f7990ed4) SHA1(e8a72bc0926911ba5c079b02dd324ac060e8c768) ) // same as lhfy

	ROM_REGION( 0x10000000, "gfx", ROMREGION_ERASE00 )
	// 4x 64MB flash ROMs (U1, U2, U3, U4) mounted onto a custom SODIMM at CN1 with a sticker "CG V105CN"
	ROM_LOAD( "cg_v105cn.u1", 0x0000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v105cn.u2", 0x4000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v105cn.u3", 0x8000000, 0x4000000, NO_DUMP )
	ROM_LOAD( "cg_v105cn.u4", 0xc000000, 0x4000000, NO_DUMP )
ROM_END


// this PCB has IGS036 MCU, R5F21256SN MCU, TT5665, ALTERA EPM3032ALC44-10N (stickered IS U15)
// ROM labels actually are written "super 70's..."
ROM_START( super70s )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "f9_igs036.u28", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "r5f21256sn.u32", 0x0000, 0x8000, NO_DUMP )

	// this seems to be dumped half sized if compared to other dumps in the driver of this same kind of ROM
	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "super_70s_v-100us.u31", 0x000000, 0x100000, BAD_DUMP CRC(60020aa3) SHA1(f6be4f9588192ef1e57182e5a61228440e5cfa64) ) // EV29LV160, BADADDR    xxxxxxxxxxxxxxxxxx-x

	// this seems to be dumped half sized if compared to other dumps in the driver of this same kind of ROM
	ROM_REGION( 0x400000, "tt5665", 0 ) // samples
	ROM_LOAD( "super_70s_v100us_u27.u27", 0x000000, 0x400000, BAD_DUMP CRC(a57fbc1c) SHA1(c7b0c72e678cd4120f576283eca8d718c058994c) ) // EV29LV640, 11xxxxxxxxxxxxxxxxxxxx = 0x00

	// these seem to be dumped half sized if compared to other dumps in the driver of this same kind of ROM
	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "super_70s_v100us_u25.u25", 0x000000, 0x400000, BAD_DUMP CRC(41baefa5) SHA1(1817bf43b3f72df35d50ef1ceb151d77ecfb988b) ) // EV29LV640
	ROM_LOAD( "super_70s_v100us_u26.u26", 0x400000, 0x400000, BAD_DUMP CRC(39bb6c75) SHA1(bc52e51f1ad3588253cb42eb61baa11d0720c5a5) ) // EV29LV640
ROM_END


// 金花争霸 (Jīn Huā Zhēngbà)
ROM_START( jhzb )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "jhzb_igs036", 0x0000, 0x4000, NO_DUMP ) // stickered K7

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU)
	ROM_LOAD( "xa", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v113cn.u17", 0x000000, 0x200000, CRC(1c08099b) SHA1(125a5302935e8bef8d4a4d8a6db748110bcab692) ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "v113cn.u26", 0x000000, 0x200000, CRC(99fc5dcd) SHA1(e737299d770b0fc3606085a566ad9fbbaca15ff5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v113cn-cg0-h.u27", 0x000000, 0x800000, CRC(a8fb40e4) SHA1(074ab61158a0baa5811f7d33792bc71051faeb62) ) // FIXED BITS (xxxxxxxx0000xxxx)
	ROM_LOAD( "v113cn-cg0-l.u29", 0x800000, 0x800000, CRC(a5f1952c) SHA1(3ae52eb604974414b6b97ff1e0156c39aa05a0cb) )
ROM_END


// 吉祥如意 (Jíxiáng Rúyì)
ROM_START( jxry )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "jhzb_igs036", 0x0000, 0x4000, NO_DUMP ) // stickered A3

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) stickered IL U14
	ROM_LOAD( "xa", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-116cn.u20", 0x000000, 0x200000, CRC(a8cd3431) SHA1(b978b2e27fc106a02213b6aa42620fbf287a4d24) )

	ROM_REGION( 0x200000, "oki", 0 ) // samples
	ROM_LOAD( "sp.u18", 0x000000, 0x200000, CRC(725950e1) SHA1(7226d3e9c2cb149dae598479cddeb12446944a1e) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "cg1.u22", 0x000000, 0x800000, CRC(bfbedd8c) SHA1(f4d97e9633de6dd87f11eade5752881d61a28a38) ) // FIXED BITS (xxxxxxxx0000xxxx)
	ROM_LOAD( "cg2.u24", 0x800000, 0x800000, CRC(c40627e5) SHA1(795217425533c569bad868b16e5b31ba29c587fb) )
ROM_END


// 双龙抢珠特别版 (Shuāng Lóng Qiǎng Zhū Tèbié Bǎn)
ROM_START( slqzsp )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "jhzb_igs036", 0x0000, 0x4000, NO_DUMP ) // stickered F8

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU) stickered IU U23
	ROM_LOAD( "xa", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v104cn.u18", 0x000000, 0x200000, CRC(7819aa1a) SHA1(c63b375916d0009e065b6239bcc752a56d2ee127) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v104cn.u27", 0x000000, 0x800000, CRC(440617cc) SHA1(87019bde5f0cf2215b6e28434a0593d0b910cbed) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v104cn.u28", 0x000000, 0x800000, CRC(e102b951) SHA1(619cf2aa77996e1831d6ff2116d6ae8045b9066c) )
	ROM_LOAD( "v104cn.u30", 0x800000, 0x800000, CRC(eecf61c6) SHA1(59d330508482dcc0971c5255c6c70c83ba72323c) )
ROM_END


// 鲨鱼大亨 (Shāyú Dàhēng)
ROM_START( sydh ) // PCB-0802-03-JL
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "jhzb_igs036", 0x0000, 0x4000, NO_DUMP ) // stickered C3

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU)
	ROM_LOAD( "xa", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-104cn.u20", 0x000000, 0x200000, CRC(43219633) SHA1(0f709c700c661f20a3f47c6df5a35aff9eb05bc6) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "v-104cn.u31", 0x000000, 0x800000, CRC(794a4ebf) SHA1(b6ea2228fc943dc65514d113c36a1e9d69564e9a) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "v-104cn.u33", 0x000000, 0x800000, CRC(cd8a1633) SHA1(2f63167fe282fc648d9e22c6a32d593f41546ba3) )
	ROM_LOAD( "v-104cn.u34", 0x800000, 0x800000, CRC(cb33469a) SHA1(cf2c52ccc688880574452bc806b80519eeff0298) )
ROM_END


// 逍遥斗地主 (Xiāoyáo Dòu Dìzhǔ)
ROM_START( xyddz )
	ROM_REGION( 0x4000, "maincpu", 0 )
	// Internal ROM of IGS036 ARM based MCU
	ROM_LOAD( "jhzb_igs036", 0x0000, 0x4000, NO_DUMP ) // stickered K8

	ROM_REGION( 0x10000, "xa:mcu", 0 ) // MX10EXAQC (80C51 XA based MCU)
	ROM_LOAD( "xa", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "xyddz.u18", 0x000000, 0x200000, CRC(5c569b88) SHA1(760bb48bb4739ae6f34921b8ed499174a8960001) )

	ROM_REGION( 0x800000, "tt5665", 0 ) // samples
	ROM_LOAD( "m2401.u27", 0x000000, 0x800000, CRC(cfaa177a) SHA1(b1ba246a51c6ec451bcd04392460f7a7ccb75cda) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "m2403-cg0-l.u30", 0x000000, 0x800000, CRC(3065ee91) SHA1(9bb1a0a739c244513f7785305aff13c9314a5d16) )
	ROM_LOAD( "s2402-cg0-h.u28", 0x800000, 0x800000, CRC(b0f43dd4) SHA1(5d82fdf3bd5eef1791e7204a040ede06c6028187) )
ROM_END


void igs_m036_state::pgm_create_dummy_internal_arm_region(void)
{
	uint16_t *temp16 = (uint16_t *)memregion("maincpu")->base();
	for (int i = 0; i < 0x4000 / 2; i += 2)
	{
		temp16[i] = 0xfffe;
		temp16[i + 1] = 0xeaff;

	}
	int base = 0;

	// jump straight to where we've mapped the external ROM for testing (should really set up a fake stack etc. too)


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



void igs_m036_state::igs_m036(machine_config &config)
{
	IGS036(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m036_state::igs_m036_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(igs_m036_state::screen_update_igs_m036));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x200);

	SPEAKER(config, "speaker").front_center();
	OKIM6295(config, "oki", 1'000'000, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "speaker", 1.0); // clock and pin 7 not verified
}


void igs_m036_state::igs_m036_tt(machine_config &config)
{
	IGS036(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m036_state::igs_m036_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(igs_m036_state::screen_update_igs_m036));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x200);

	SPEAKER(config, "speaker").front_center();
	TT5665(config, "tt5665", 4.952_MHz_XTAL, tt5665_device::ss_state::SS_HIGH, 0).add_route(1, "speaker", 1.0);
}



void igs_m036_state::init_igs_m036()
{
	pgm_create_dummy_internal_arm_region();
}

void igs_m036_state::init_cjdh2()
{
	init_igs_m036();

	igs036_decryptor decrypter(cjdh2_key);
	decrypter.decrypter_rom((uint16_t*)memregion("user1")->base(), memregion("user1")->bytes(), 0);
}

void igs_m036_state::init_cjddzsp()
{
	init_igs_m036();

	igs036_decryptor decrypter(cjddzsp_key);
	decrypter.decrypter_rom((uint16_t*)memregion("user1")->base(), memregion("user1")->bytes(), 0);
}

void igs_m036_state::init_igsm312()
{
	init_igs_m036();

	igs036_decryptor decrypter(m312cn_key);
	decrypter.decrypter_rom((uint16_t*)memregion("user1")->base(), memregion("user1")->bytes(), 0);
}

} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 200?, cjdh2,    0,     igs_m036,    igs_m036, igs_m036_state, init_cjdh2,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V311CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjdh2a,   cjdh2, igs_m036,    igs_m036, igs_m036_state, init_cjdh2,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V311CNA)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjdh2b,   cjdh2, igs_m036,    igs_m036, igs_m036_state, init_cjdh2,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V311CNB)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjdh2c,   cjdh2, igs_m036,    igs_m036, igs_m036_state, init_cjdh2,    ROT0, "IGS",           "Chao Ji Da Heng 2 (V215CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, cjddzsp,  0,     igs_m036_tt, igs_m036, igs_m036_state, init_cjddzsp,  ROT0, "IGS",           "Super Dou Di Zhu Special (V122CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2007, qhzb,     0,     igs_m036_tt, igs_m036, igs_m036_state, init_cjddzsp,  ROT0, "IGS",           "Que Huang Zheng Ba (V100CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2009, lhtb,     0,     igs_m036_tt, igs_m036, igs_m036_state, init_cjddzsp,  ROT0, "IGS",           "Long Hu Tebie Ban (V101CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 龍虎特別版 - Lónghǔ tèbié bǎn

GAME( 200?, lhzb3in1, 0,     igs_m036_tt, igs_m036, igs_m036_state, init_cjddzsp,  ROT0, "IGS",           "Long Hu Zhengba San He Yi (V100CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 龙虎争霸三合一

GAME( 200?, igsm312,  0,     igs_m036_tt, igs_m036, igs_m036_state, init_igsm312,  ROT0, "IGS",           "unknown 'IGS 6POKER2' game (V312CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // there's very little code and no gfx ROMs, might be a 'set/clear' chip for a gambling game.

GAME( 200?, super70s, 0,     igs_m036_tt, igs_m036, igs_m036_state, init_igsm312,  ROT0, "IGS",           "Super 70's (V100US)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, jhzb,     0,     igs_m036,    igs_m036, igs_m036_state, init_igs_m036, ROT0, "IGS",           "Jin Hua Zhengba (V113CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, jxry,     0,     igs_m036,    igs_m036, igs_m036_state, init_igs_m036, ROT0, "IGS",           "Jixiang Ruyi (V116CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, slqzsp,   0,     igs_m036_tt, igs_m036, igs_m036_state, init_igs_m036, ROT0, "IGS",           "Shuang Long Qiang Zhu Tebie Ban (V104CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, sydh,     0,     igs_m036_tt, igs_m036, igs_m036_state, init_igs_m036, ROT0, "IGS",           "Shayu Daheng (V104CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, xyddz,    0,     igs_m036_tt, igs_m036, igs_m036_state, init_igs_m036, ROT0, "IGS",           "Xiaoyao Dou Dizhu", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2010, lhfy,     0,     igs_m036_tt, igs_m036, igs_m036_state, init_igsm312,  ROT0, "IGS",           "Long Hu Feng Yun Gao Qing Ban (V206CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2010, lhzbgqb,  0,     igs_m036_tt, igs_m036, igs_m036_state, init_igsm312,  ROT0, "IGS",           "Long Hu Zheng Ba Gao Qing Ban (V105CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2015, mghammer, 0,     igs_m036_tt, igs_m036, igs_m036_state, init_igsm312,  ROT0, "IGS / Enheart", "Medal Get Hammer (V100JP)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
