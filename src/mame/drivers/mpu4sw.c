/* mpu4 sets which have been split, and appear to do something useful */

/* many sets have the BARCREST or BWG string at FFE0 replaced with other things
  such at 'FATHER CHRISTMAS' are these hacked / bootlegs requiring special hw?
  
  sets with 'D' in the ident code are Datapak sets, and do not play without a datapak connected
  sets with 'Y' seem to require the % key to be set

  */

#include "emu.h"

MACHINE_CONFIG_EXTERN( mod4oki );
MACHINE_CONFIG_EXTERN( mod2 );
INPUT_PORTS_EXTERN( mpu4 );
INPUT_PORTS_EXTERN( grtecp );
extern DRIVER_INIT( m4default );
extern DRIVER_INIT( m_grtecp );

#define GAME_FLAGS (GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK)

static DRIVER_INIT( m4debug )
{
	// many original barcrest / bwb sets have identification info around here
	// this helps with sorting
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	printf("\ncopyright string:\n");
	for (int i = 0xffe0; i<0xfff0; i++)
	{
		printf("%c", src[i]);
	}
	printf("\n\nidentification string:\n");
	for (int i = 0xff28; i<0xff30; i++)
	{
		printf("%c", src[i]);
	}
}

static DRIVER_INIT( m4_showstring )
{
	DRIVER_INIT_CALL( m4default );
	DRIVER_INIT_CALL( m4debug );

}

static DRIVER_INIT( m_grtecpss )
{
	DRIVER_INIT_CALL( m_grtecp );
	DRIVER_INIT_CALL( m4debug );
}

#define M4ANDYCP_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "ac.chr", 0x0000, 0x000048, CRC(87808826) SHA1(df0915a6f89295efcd10e6a06bfa3d3fe8fef160) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "acapp1.bin",  0x000000, 0x080000, CRC(bda61fc6) SHA1(b94f39fa92d3d0cb580eaafa0f58bd5cde947e3a) ) \
	ROM_LOAD( "andsnd.bin",  0x000000, 0x080000, CRC(7d568671) SHA1(3a0a6af3dc980f2ccff0b6ef85833eb2e352031a) ) \
	ROM_LOAD( "andsnd2.bin", 0x080000, 0x080000, CRC(98a586ee) SHA1(94b94d198725e8174e14873b99afa19217a1d4fa) ) \

#define M4ANDYCP_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4ANDYCP_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 ,m4_showstring,ROT0,company,title,GAME_FLAGS ) \


// "(C)1994  B.W.B."  and  "AC101.0"
M4ANDYCP_SET( 1994, m4andycp,			0,			"ac10.hex",			0x0000, 0x010000, CRC(0e250923) SHA1(9557315cca7a47c307e811d437ff424fe77a2843), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC10)" )
M4ANDYCP_SET( 1994, m4andycp10c,		m4andycp,	"aci10___.1_1",		0x0000, 0x010000, CRC(afa29daa) SHA1(33d161977b1e3512b550980aed48954ba7f0c5a2), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC10C)" )
M4ANDYCP_SET( 1994, m4andycp10d,		m4andycp,	"ac_10sd_.1_1",		0x0000, 0x010000, CRC(ec800208) SHA1(47734ae5a3184e4805a7620287fb5da7fe823929), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC10D)" ) // datapak
M4ANDYCP_SET( 1994, m4andycp10k,		m4andycp,	"ac_10a__.1_1",		0x0000, 0x010000, CRC(c8a1150b) SHA1(99ba283aeacd1c415d261e10b5b7fd43d3c25af8), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC10K)" )
M4ANDYCP_SET( 1994, m4andycp10yd,		m4andycp,	"ac_10sb_.1_1",		0x0000, 0x010000, CRC(f68f8f48) SHA1(a156d942e7ab7446290dcd8def6236e7436126b9), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC10YD)" ) // datapak
// "FATHER CHISTMAS" and  "AC101.0" (hack?)
M4ANDYCP_SET( 1994, m4andycp10_a,		m4andycp,	"acap_10_.8",		0x0000, 0x010000, CRC(614403a7) SHA1(b627c7c3c6f9a43a0cd9e064715aeee8834c717c), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC10, hack?)" ) // won't boot  'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycp10c_a,		m4andycp,	"acapp10p5.bin",	0x0000, 0x010000, CRC(de650e19) SHA1(c1b9cbad23a1eac9b3718f4f2457c97317f96be6), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC10C, hack?, set 1)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycp10c_b,		m4andycp,	"acp8ac",			0x0000, 0x010000, CRC(d51997b5) SHA1(fe08b5a3832eeaa80f674893342c3baea1608a91), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC10C, hack?, set 2)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycp10c_c,		m4andycp,	"acap10_11",		0x0000, 0x010000, CRC(c3a866e7) SHA1(4c18e5a26ad2885eb012fd3dd61aaf9cc7d3519a), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC10C, hack?, set 3)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycp10c_d,		m4andycp,	"acap_10_.4",		0x0000, 0x010000, CRC(fffe742d) SHA1(f2ca45391690dc31662e2d97a3ee34473effa258), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC10C, hack?, set 4)" ) // won't boot 'FATHER CHISTMAS'
// "(C)1994  B.W.B."  and "AC5 1.0"
M4ANDYCP_SET( 1994, m4andycpac,			m4andycp,	"ac_05s__.1_1",		0x0000, 0x010000, CRC(eab8aaca) SHA1(ccec86cf44f97a894192b2a6f900a93d26e84bf9), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC5)" )
M4ANDYCP_SET( 1994, m4andycpacd,		m4andycp,	"ac_05sd_.1_1",		0x0000, 0x010000, CRC(4c815831) SHA1(66c6a4fed60ecc5ff5c9202528797d044fde3e76), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 D)" ) // datapak
M4ANDYCP_SET( 1994, m4andycpack,		m4andycp,	"ac_05a__.1_1",		0x0000, 0x010000, CRC(880c2532) SHA1(a6a3c996c7507f0e2b8ae8e9fdfb7473263bd5cf), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 K)" )
M4ANDYCP_SET( 1994, m4andycpacyd,		m4andycp,	"ac_05sb_.1_1",		0x0000, 0x010000, CRC(dfd2571b) SHA1(98d93e30f4684fcbbc5ce4f356b8c9eeb20cbbdb), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 YD)" ) // datapak
M4ANDYCP_SET( 1994, m4andycpacc,		m4andycp,	"aci05___.1_1",		0x0000, 0x010000, CRC(e06174e8) SHA1(e984e45b99d4aef9b46c83590efadbdec9888b2d), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 C)" )
// "FATHER CHISTMAS" and "AC5 1.0" (hack?)
M4ANDYCP_SET( 1994, m4andycpac_a,		m4andycp,	"acap_05_.8",		0x0000, 0x010000, CRC(a17dd8de) SHA1(963d39fdca7c7b54f5ecf723c982eb30a426ebae), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC5, hack?)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycpacc_a,		m4andycp,	"acap_05_.4",		0x0000, 0x010000, CRC(ca00ee84) SHA1(f1fef3db3db5ca7f0eb72ccc1daba8446db02924), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 C, hack?, set 1)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycpacc_b,		m4andycp,	"ac056c",			0x0000, 0x010000, CRC(cdeaeb06) SHA1(5bfcfba614477f4df9f4b2e56e8448eb357c554a), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 C, hack?, set 2)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycpacc_c,		m4andycp,	"ac058c",			0x0000, 0x010000, CRC(15204ccc) SHA1(ade376193bc2d53dd4c824ee35fbcc16da31330a), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 C, hack?, set 3)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycpacc_d,		m4andycp,	"acap05_11",		0x0000, 0x010000, CRC(fb1533a0) SHA1(814e5dd9c4fe3baf4ea3b22c7e02e30b07bd27a1), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 C, hack?, set 4)" ) // won't boot 'FATHER CHISTMAS'
M4ANDYCP_SET( 1994, m4andycpacc_e,		m4andycp,	"acap55",			0x0000, 0x010000, CRC(8007c459) SHA1(b3b6213d89eb0d2cc2f7dab81e0f0f2fdd0f8776), "hack?",	"Andy Capp (Bwb / Barcrest) (MPU4) (AC5 C, hack?, set 5)" ) // won't boot 'FATHER CHISTMAS'
// "(C)1991 BARCREST"  and "AN8 0.1"
M4ANDYCP_SET( 1991, m4andycp8,			m4andycp,	"an8s.p1",			0x0000, 0x010000, CRC(14ac28da) SHA1(0b4a3f997e10573f2c4c44daac344f4be52363a0), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AM8)" )
M4ANDYCP_SET( 1991, m4andycp8d,			m4andycp,	"an8d.p1",			0x0000, 0x010000, CRC(ae01af1c) SHA1(7b2305480a318648a3cc6c3bc66f21ac327e25aa), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 D)" ) // datapak
M4ANDYCP_SET( 1991, m4andycp8ad,		m4andycp,	"an8ad.p1",			0x0000, 0x010000, CRC(d0f9da00) SHA1(fb380897fffc33d238b8fe7d47ff4d9d97960283), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 AD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycp8b,			m4andycp,	"an8b.p1",			0x0000, 0x010000, CRC(fc4001ae) SHA1(b0cd795235e6f500f0150097b8f760165c17ca27), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 B)" )
M4ANDYCP_SET( 1991, m4andycp8c,			m4andycp,	"an8c.p1",			0x0000, 0x010000, CRC(35a4403e) SHA1(33d3ca4e7bad25d064e0780c2104c395259c2a94), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 C)" )
M4ANDYCP_SET( 1991, m4andycp8k,			m4andycp,	"an8k.p1",			0x0000, 0x010000, CRC(296b4453) SHA1(060a6cea9a0be923e359dd69e34a6c25d631e4e5), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 K)" )
M4ANDYCP_SET( 1991, m4andycp8kd,		m4andycp,	"an8dk.p1",			0x0000, 0x010000, CRC(d43ad86d) SHA1(a71f1eb26e5f688db675b5c6bddda713e709a7af), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 KD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycp8y,			m4andycp,	"an8y.p1",			0x0000, 0x010000, CRC(44da57c9) SHA1(0f2776214068400a0e30b5642f42d72f58bbc29b), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 Y)" ) // need % key
M4ANDYCP_SET( 1991, m4andycp8yd,		m4andycp,	"an8dy.p1",			0x0000, 0x010000, CRC(6730e476) SHA1(d19f7d173ec18085ef904c8621e81305bd54a143), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AN8 YD)" ) // datapak
// "(C)1991 BARCREST"  and "AND 0.4"
M4ANDYCP_SET( 1991, m4andycpd,			m4andycp,	"ands.p1",			0x0000, 0x010000, CRC(120967eb) SHA1(f47846e5f1c6300518104341740e66610b9a9ab3), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AND)" )
M4ANDYCP_SET( 1991, m4andycpdd,			m4andycp,	"andd.p1",			0x0000, 0x010000, CRC(d48a42fb) SHA1(94e3b994b9425af9a7744d511ad3413a79e24f21), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AND D)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpdc,			m4andycp,	"andc.p1",			0x0000, 0x010000, CRC(31735e79) SHA1(7247efbfe41dce04dd494f07a8871f34d76eaacd), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AND C)" )
M4ANDYCP_SET( 1991, m4andycpdk,			m4andycp,	"andk.p1",			0x0000, 0x010000, CRC(08e6d20f) SHA1(f66207f69bf417e9380ecc8bd2ba73c6f3d55150), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AND K)" )
M4ANDYCP_SET( 1991, m4andycpdy,			m4andycp,	"andy.p1",			0x0000, 0x010000, CRC(b1124803) SHA1(0f3422e5f048d1748d2c912f2ea56f206fd101bb), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AND Y, set 1)" ) // needs % key
M4ANDYCP_SET( 1991, m4andycpdyd,		m4andycp,	"anddy.p1",			0x0000, 0x010000, CRC(7f24b95d) SHA1(0aa97ad653b24265d73577db61200e44abf11c50), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AND YD)" ) // datapak
// "(C)1991 BARCREST"  and "AND 0.2"
M4ANDYCP_SET( 1991, m4andycpdy_a,		m4andycp,	"acap20p",			0x0000, 0x010000, CRC(f0a9a4a4) SHA1(3c9a2e3d90ea91f92ae500856ad97c376edc1548), "Barcrest", "Andy Capp (Barcrest) (MPU4) (AND Y, set 2)" ) // needs % key
// "(C)1991 BARCREST"  and "C2T 0.2"
M4ANDYCP_SET( 1991, m4andycpc2,			m4andycp,	"c2t02s.p1",		0x0000, 0x010000, CRC(d004f962) SHA1(1f211fd62438cb7c5d5f4ce9ced29a0a7e64e80b), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T, set 1)" )
M4ANDYCP_SET( 1991, m4andycpc2d,		m4andycp,	"c2t02d.p1",		0x0000, 0x010000, CRC(ce5bbf2e) SHA1(dab2a1015713ceb8dce8b766fc2660207fcbb9f2), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T D)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc2ad,		m4andycp,	"c2t02ad.p1",		0x0000, 0x010000, CRC(38e36fe3) SHA1(01c007e21a6ac1a77bf314402d727c41b7a222ca), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T AD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc2b,		m4andycp,	"c2t02b.p1",		0x0000, 0x010000, CRC(f059a9dc) SHA1(0c5d5a4b108c85215b9d5f8c2263b66559cfa90a), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T B)" )
M4ANDYCP_SET( 1991, m4andycpc2bd,		m4andycp,	"c2t02bd.p1",		0x0000, 0x010000, CRC(0eff02a9) SHA1(f460f93098630ac2757a560deb2e741ae9631a54), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T BD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc2r,		m4andycp,	"c2t02r.p1",		0x0000, 0x010000, CRC(6ccaf958) SHA1(8878e16d2c01131d36f211b3a73e987409f54ef9), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T R)" )
M4ANDYCP_SET( 1991, m4andycpc2rd,		m4andycp,	"c2t02dr.p1",		0x0000, 0x010000, CRC(7daee156) SHA1(2ae03c39ca5704c112c9ec6acba46022f4dd9805), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T RD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc2k,		m4andycp,	"c2t02k.p1",		0x0000, 0x010000, CRC(077024e0) SHA1(80597f28891caa25506bb6bbc77a005623096ff9), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T K)" )
M4ANDYCP_SET( 1991, m4andycpc2kd,		m4andycp,	"c2t02dk.p1",		0x0000, 0x010000, CRC(dc8c078e) SHA1(9dcde48d17a39dbe10333632eacc1f0860e165de), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T KD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc2y,		m4andycp,	"c2t02y.p1",		0x0000, 0x010000, CRC(f1a1d1b6) SHA1(d9ceedee3b833be8de5b065e45a72ca180283528), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T Y)" )
M4ANDYCP_SET( 1991, m4andycpc2yd,		m4andycp,	"c2t02dy.p1",		0x0000, 0x010000, CRC(e0c5c9b8) SHA1(d067d4786ded041d8031808078eb2c0383937931), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T YD)" ) // datapak
// "(C)1991 BARCREST"  and "C2T 0.1"
M4ANDYCP_SET( 1991, m4andycpc2_a,		m4andycp,	"acap2010",			0x0000, 0x010000, CRC(1b8e712b) SHA1(6770869966290fe6e61b7bf1971ab7a15e601d69), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C2T, set 2)" )
// "(C)1991 BARCREST"  and "C5T 0.1"
M4ANDYCP_SET( 1991, m4andycpc5,			m4andycp,	"c5ts.p1",			0x0000, 0x010000, CRC(3ade4b1b) SHA1(c65d05e2493a0e2d6a4be58a42aac6cb7f9c01b5), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T)" )
M4ANDYCP_SET( 1991, m4andycpc5d,		m4andycp,	"c5td.p1",			0x0000, 0x010000, CRC(ab359cae) SHA1(f8ab817709e0eeb91a059cdef19df99c6286bf3f), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T D)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc5ad,		m4andycp,	"c5tad.p1",			0x0000, 0x010000, CRC(dab92a37) SHA1(30297a7e1a995b76d8f955fd8a40efc914874e29), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T AD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc5b,		m4andycp,	"c5tb.p1",			0x0000, 0x010000, CRC(1a747871) SHA1(61eb026c2d35feade5cfecf609e99cd0c6d0693e), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T B)" )
M4ANDYCP_SET( 1991, m4andycpc5bd,		m4andycp,	"c5tbd.p1",			0x0000, 0x010000, CRC(b0fb7c1c) SHA1(f5edf7685cc7015ac9791d35dde3fd284180660f), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T BD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc5k,		m4andycp,	"c5tk.p1",			0x0000, 0x010000, CRC(26a1d1f6) SHA1(c64763188dd0520c3f802863d36c84a476efef40), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T K)" )
M4ANDYCP_SET( 1991, m4andycpc5kd,		m4andycp,	"c5tdk.p1",			0x0000, 0x010000, CRC(295976d6) SHA1(a506097e94d290f5b66f61c9979b0ae4f211bb0c), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T KD)" ) // datapak
M4ANDYCP_SET( 1991, m4andycpc5y,		m4andycp,	"c5ty.p1",			0x0000, 0x010000, CRC(52953040) SHA1(65102c88e8766e07d268fe0267bc6731d8b3eeb3), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T Y)" ) // needs % key
M4ANDYCP_SET( 1991, m4andycpc5yd,		m4andycp,	"c5tdy.p1",			0x0000, 0x010000, CRC(d9b4dc81) SHA1(e7b7a5f9b1ad348444d5403df2bf16b829364d33), "Barcrest", "Andy Capp (Barcrest) (MPU4) (C5T YD)" ) // datapak
// "(C)1995  B.W.B." and "ACC52.0" (wrong game?)
M4ANDYCP_SET( 1995, m4andycpaccsd,		m4andycp,	"ac_05_d4.2_1",		0x0000, 0x010000, CRC(f672182a) SHA1(55a6691fa9878bc2becf1f080915c0cd939240dd), "Bwb",		"Andy Capp (Bwb / Barcrest) (MPU4) (ACC5)" ) // datapak (odd ident string)
// "95,S  ALIVE!!!" and "AND 0.3" (hack?)
M4ANDYCP_SET( 199?, m4andycp20,			m4andycp,	"acap_20_.4",		0x0000, 0x010000, CRC(29848eed) SHA1(4096ab2f58b3293c559ff69c6f0f4d6c5dee2fd2), "hack?",	"Andy Capp (Barcrest) (MPU4) (hack?, set 1)" ) // bad chr
M4ANDYCP_SET( 199?, m4andycp20_a,		m4andycp,	"acap_20_.8",		0x0000, 0x010000, CRC(3981ec67) SHA1(ad040a4c8690d4348bfe306309df5374251f2b3e), "hack?",	"Andy Capp (Barcrest) (MPU4) (hack?, set 2)" ) // bad chr
M4ANDYCP_SET( 199?, m4andycp20_b,		m4andycp,	"acap20_11",		0x0000, 0x010000, CRC(799fd89e) SHA1(679016fad8b012bf6b6c617b99fd0dbe71eff562), "hack?",	"Andy Capp (Barcrest) (MPU4) (hack?, set 3)" ) // bad chr

#define M4ANDYCP_DUT_ROMS \
	ROM_REGION( 0x200000, "msm6376", 0 ) \
	ROM_LOAD( "sdac_1.snd", 0x000000, 0x080000, CRC(5ce93532) SHA1(547f98740889e6fbafc5a0c517ff75de41f2acc7) ) \
	ROM_LOAD( "sdac_2.snd", 0x080000, 0x080000, CRC(22dacd4b) SHA1(ad2dc943d4e3ec54937acacb963da938da809614) ) \
	ROM_LOAD( "sjcv2.snd", 0x080000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) ) \

ROM_START( m4andycpdut )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dac13.bin", 0x0000, 0x010000, CRC(a0cdd5b3) SHA1(7b7bc40a9a9aed3569f491acad15c606fe243e9b) )
	M4ANDYCP_DUT_ROMS
ROM_END

// blank copyright  and "DAC 1.3" (6 reel game, not the same as the UK version?)
GAME(199?, m4andycpdut,		m4andycp	,mod4oki	,mpu4				,m4_showstring			,ROT0,   "Barcrest","Andy Capp (Barcrest) [DAC 1.3, Dutch] (MPU4)",			GAME_FLAGS|GAME_NO_SOUND )


#define M4ANDYFL_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "alf.chr", 0x0000, 0x000048, CRC(22f09b0d) SHA1(5a612e54e0bb5ea5c35f1a7b1d7bc3cdc34e3bdd) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "alfsnd0.1", 0x0000, 0x080000, CRC(6691bc25) SHA1(4dd67b8bbdc5d707814b756005075fcb4f0c8be4) ) \

#define M4ANDYFL_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4ANDYFL_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 ,m4_showstring,ROT0,company,title,GAME_FLAGS ) \

// "(C)1996  B.W.B."  and "AL4 2.1"
M4ANDYFL_SET( 1996, m4andyfl,		0,			 "andy loves flo 05a 4 2-1",0x0000, 0x010000, CRC(773d2c6f) SHA1(944be6fff70439077a9c0d858e76806e0317585c), "Bwb", "Andy Loves Flo (Bwb / Barcrest) (MPU4) (AL4 2.1KS)" )
// "(C)1996  B.W.B."  and "AL_ 2.4"
M4ANDYFL_SET( 1996, m4andyfl8bs,	m4andyfl,	 "al_05a__.2_1",			0x0000, 0x010000, CRC(d28849c8) SHA1(17e79f92cb3667de0be54fd4bae7f4c3a3a80aa5), "Bwb", "Andy Loves Flo (Bwb / Barcrest) (MPU4) (AL_ 2.4KS)" )
// "(C)1991 BARCREST"  and "AL3 0.1"
M4ANDYFL_SET( 1991, m4andyfl3,		m4andyfl,	 "al3s.p1",					0x0000, 0x010000, CRC(07d4d6c3) SHA1(d013cf49ed4b84e6149065c95d1cd00eca0d62b8), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1)" )
M4ANDYFL_SET( 1991, m4andyfl3d,		m4andyfl,	 "al3d.p1",					0x0000, 0x010000, CRC(621b5831) SHA1(589e5a94324a56704b1a05bafe16bf6d838dea6c), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1D)" )
M4ANDYFL_SET( 1991, m4andyfl3ad,	m4andyfl,	 "al3ad.p1",				0x0000, 0x010000, CRC(6d057dd3) SHA1(3febe5aea14852559de554c2e034c328393ae0fa), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1AD)" )
M4ANDYFL_SET( 1991, m4andyfl3b,		m4andyfl,	 "al3b.p1",					0x0000, 0x010000, CRC(4b967a4f) SHA1(1a6e24ecaa907a5bb6fa589dd0de473c7e4c6f6c), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1B)" )
M4ANDYFL_SET( 1991, m4andyfl3bd,	m4andyfl,	 "al3bd.p1",				0x0000, 0x010000, CRC(5b191099) SHA1(9049ff924123ee9309155730d53cb168bd8237bf), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1BD)" )
M4ANDYFL_SET( 1991, m4andyfl3k,		m4andyfl,	 "al3k.p1",					0x0000, 0x010000, CRC(f036b844) SHA1(62269e3ed0c6fa5df592883294efc74da856d897), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1K)" )
M4ANDYFL_SET( 1991, m4andyfl3kd,	m4andyfl,	 "al3dk.p1",				0x0000, 0x010000, CRC(5a77dcf2) SHA1(63e67ca1e112b56ea99b3c91952fa9b04518d6ae), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1KD)" )
M4ANDYFL_SET( 1991, m4andyfl3y,		m4andyfl,	 "al3y.p1",					0x0000, 0x010000, CRC(1cce9f53) SHA1(aaa8492ea28cc0134ae7d070e182a3f98e769c40), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1Y)" )
M4ANDYFL_SET( 1991, m4andyfl3yd,	m4andyfl,	 "al3dy.p1",				0x0000, 0x010000, CRC(c7bdd13e) SHA1(674cad23b7d6299918951de5dbbb33acf01dac66), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL3 0.1YD)" )
// "(C)1991 BARCREST"  and "AL8 0.1"
M4ANDYFL_SET( 1991, m4andyfl8,		m4andyfl,	 "al8s.p1",					0x0000, 0x010000, CRC(37e211f9) SHA1(8614e8081fdd370d6c3dd537ee6058a2247d4ae0), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1)" )
M4ANDYFL_SET( 1991, m4andyfl8d,		m4andyfl,	 "al8d.p1",					0x0000, 0x010000, CRC(c1cb8f01) SHA1(c267c208c23bb7816f5475b0c0db2d69c6b98970), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1D)" )
M4ANDYFL_SET( 1991, m4andyfl8ad,	m4andyfl,	 "al8ad.p1",				0x0000, 0x010000, CRC(90a72618) SHA1(2c11e98b446500da9b618c8a7a9d441cff916851), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1AD)" )
M4ANDYFL_SET( 1991, m4andyfl8b,		m4andyfl,	 "al8b.p1",					0x0000, 0x010000, CRC(3c0324e9) SHA1(5ddf33b06728de62d995cdbfc6bdc9e711661e38), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1B)" )
M4ANDYFL_SET( 1991, m4andyfl8bd,	m4andyfl,	 "al8bd.p1",				0x0000, 0x010000, CRC(a6bb4b52) SHA1(0735c45c3f02a3f17dfbe1f744a8685de97fdd8f), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1BD)" )
M4ANDYFL_SET( 1991, m4andyfl8c,		m4andyfl,	 "al8c.p1",					0x0000, 0x010000, CRC(154b0f79) SHA1(e178404674ace57c639c90a44e5f03803ec812d0), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1C)" )
M4ANDYFL_SET( 1991, m4andyfl8k,		m4andyfl,	 "al8k.p1",					0x0000, 0x010000, CRC(77d8f8b4) SHA1(c91fe5a543ba83b68fe3285da55d77f7b93131db), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1K)" )
M4ANDYFL_SET( 1991, m4andyfl8kd,	m4andyfl,	 "al8dk.p1",				0x0000, 0x010000, CRC(bf346ace) SHA1(fdf0e5550caaae9e63ac5ea571e290fec4c768af), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1KD)" )
M4ANDYFL_SET( 1991, m4andyfl8y,		m4andyfl,	 "al8y.p1",					0x0000, 0x010000, CRC(c77ee4c2) SHA1(fc5cb6aff5e5aeaf577cb0b9ed2e1ac06359089e), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (AL8 0.1Y)" )
// "(C)1991 BARCREST"  and "ALF 2.0"
M4ANDYFL_SET( 1991, m4andyflf,		m4andyfl,	 "alfs.p1",					0x0000, 0x010000, CRC(5c0e14f6) SHA1(ebce737afb71b27829d69ff203ff86a828df946a), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALF 2.0)" )
M4ANDYFL_SET( 1991, m4andyflfb,		m4andyfl,	 "alfb.p1",					0x0000, 0x010000, CRC(3133c954) SHA1(49bedc54c7d39b3cf40c19a0e56a8bea798aeba7), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALF 2.0B)" )
M4ANDYFL_SET( 1991, m4andyflfc,		m4andyfl,	 "alfc.p1",					0x0000, 0x010000, CRC(c0fc9244) SHA1(30c7929a95e67b6a10877087a337b34a726b0ec9), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALF 2.0C)" )
M4ANDYFL_SET( 1991, m4andyflfk,		m4andyfl,	 "alfk.p1",					0x0000, 0x010000, CRC(f9691e32) SHA1(9b72a9c78de8979568a720e5e1986734063defac), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALF 2.0K)" )
M4ANDYFL_SET( 1991, m4andyflfr,		m4andyfl,	 "alfr.p1",					0x0000, 0x010000, CRC(acc4860b) SHA1(cbae236c5e1bdbb294f99cd749067d41a24e8973), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALF 2.0R)" )
// "(C)1991 BARCREST"  and "ALT 0.4"
M4ANDYFL_SET( 1991, m4andyflt,		m4andyfl,	 "alt04s.p1",				0x0000, 0x010000, CRC(81cf27b3) SHA1(b04970a20a297032cf33dbe97fa22fb723587228), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4)" )
M4ANDYFL_SET( 1991, m4andyfltd,		m4andyfl,	 "alt04d.p1",				0x0000, 0x010000, CRC(c6b36e95) SHA1(5e4e8fd1a2f0411be1ab5c0bbae1f9cd8062f234), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4D)" )
M4ANDYFL_SET( 1991, m4andyfltad,	m4andyfl,	 "alt04ad.p1",				0x0000, 0x010000, CRC(b0a332c5) SHA1(b0bbd193d44543c0f8cfe8c51b7956de84b9af10), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4AD)" )
M4ANDYFL_SET( 1991, m4andyfltb,		m4andyfl,	 "alt04b.p1",				0x0000, 0x010000, CRC(9da0465d) SHA1(2f02f739bf9ef8fa4bcb163f1a881052fc3d483f), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4B)" )
M4ANDYFL_SET( 1991, m4andyfltbd,	m4andyfl,	 "alt04bd.p1",				0x0000, 0x010000, CRC(86bf5f8f) SHA1(c310ac44f85e5883a8d4ed369c4b68c0aebe2820), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4BD)" )
M4ANDYFL_SET( 1991, m4andyfltk,		m4andyfl,	 "alt04k.p1",				0x0000, 0x010000, CRC(a5818f6d) SHA1(bd69e9e4e05cedfc044ae91a12e84d73e19a50ac), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4K)" )
M4ANDYFL_SET( 1991, m4andyfltkd,	m4andyfl,	 "alt04dk.p1",				0x0000, 0x010000, CRC(596abc65) SHA1(c534852c6dd8e6574529cd1da665dc60147b71de), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4KD)" )
M4ANDYFL_SET( 1991, m4andyfltr,		m4andyfl,	 "alt04r.p1",				0x0000, 0x010000, CRC(a1d4caf8) SHA1(ced673abb0a3f6f72c441f26eabb473e0a1b2fd7), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4R)" )
M4ANDYFL_SET( 1991, m4andyfltrd,	m4andyfl,	 "alt04dr.p1",				0x0000, 0x010000, CRC(9237bdfc) SHA1(630bed3c4e17774f30a7fc26aa69c69054bffdd9), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4RD)" )
M4ANDYFL_SET( 1991, m4andyflty,		m4andyfl,	 "alt04y.p1",				0x0000, 0x010000, CRC(c63a5a57) SHA1(90bb47cb87dcdc875546be64d9cf9e8cf9e15f97), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4Y)" )
M4ANDYFL_SET( 1991, m4andyfltyd,	m4andyfl,	 "alt04dy.p1",				0x0000, 0x010000, CRC(f6750d3e) SHA1(1c87a7f574e9db45cbfe3e9bf4600a68cb6d5bd4), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALT 0.4YD)" )
// "(C)1991 BARCREST"  and "ALU 0.3"
M4ANDYFL_SET( 1991, m4andyflu,		m4andyfl,	 "alu03s.p1",				0x0000, 0x010000, CRC(87704898) SHA1(47fe7b835619e770a58c71796197a0d2810a8e9b), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3)" )
M4ANDYFL_SET( 1991, m4andyflud,		m4andyfl,	 "alu03d.p1",				0x0000, 0x010000, CRC(478e09e6) SHA1(8a6221ceb841c41b839a8d478736144343d565a1), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3D)" )
M4ANDYFL_SET( 1991, m4andyfluad,	m4andyfl,	 "alu03ad.p1",				0x0000, 0x010000, CRC(3e6f03b5) SHA1(0bc85442b614091a25529b7ae2fc7e907d8d82e8), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3AD)" )
M4ANDYFL_SET( 1991, m4andyflub,		m4andyfl,	 "alu03b.p1",				0x0000, 0x010000, CRC(92baccd8) SHA1(08e4544575a62991a6ae19ec4430a459074a1984), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3B)" )
M4ANDYFL_SET( 1991, m4andyflubd,	m4andyfl,	 "alu03bd.p1",				0x0000, 0x010000, CRC(08736eff) SHA1(bb0c3c2a54b8828ed7ce5576abcec7800d033c9b), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3BD)" )
M4ANDYFL_SET( 1991, m4andyfluk,		m4andyfl,	 "alu03k.p1",				0x0000, 0x010000, CRC(1e5fcd28) SHA1(0e520661a47d2e6c9f2f14a8c5cbf17bc15ffc9b), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3K)" )
M4ANDYFL_SET( 1991, m4andyflukd,	m4andyfl,	 "alu03dk.p1",				0x0000, 0x010000, CRC(fe3efc18) SHA1(3f08c19581672748f9bc34a7d85ff946320f89ae), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3KD)" )
M4ANDYFL_SET( 1991, m4andyflur,		m4andyfl,	 "alu03r.p1",				0x0000, 0x010000, CRC(123c111c) SHA1(641fb7a49e2160956178e3dc635ec33a377bfa7a), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3R)" )
M4ANDYFL_SET( 1991, m4andyflurd,	m4andyfl,	 "alu03dr.p1",				0x0000, 0x010000, CRC(5486a30c) SHA1(8417afd7a9a07d4e9ac65880906320c49c0bf230), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3RD)" )
M4ANDYFL_SET( 1991, m4andyfluy,		m4andyfl,	 "alu03y.p1",				0x0000, 0x010000, CRC(254e43c4) SHA1(963b4e46d88b64f8ebc0c42dee2bbcb0ae1d3bec), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3Y)" )
M4ANDYFL_SET( 1991, m4andyfluyd,	m4andyfl,	 "alu03dy.p1",				0x0000, 0x010000, CRC(686e4818) SHA1(cf2af851ff7f4ce8edb82b78c3841b8b8c09bd17), "Barcrest", "Andy Loves Flo (Barcrest) (MPU4) (ALU 0.3YD)" )

#define M4DTYFRE_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "df503s.chr", 0x0000, 0x000048, CRC(46c28f35) SHA1(e229b211180f9f7b30cd0bb9de162971d16b2d33) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "dutsnd.p1", 0x000000, 0x080000, CRC(a5829cec) SHA1(eb65c86125350a7f384f9033f6a217284b6ff3d1) ) \
	ROM_LOAD( "dutsnd.p2", 0x080000, 0x080000, CRC(1e5d8407) SHA1(64ee6eba3fb7700a06b89a1e0489a0cd54bb89fd) ) \

#define M4DTYFRE_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4DTYFRE_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 ,m4_showstring,ROT0,company,title,GAME_FLAGS ) \

// "(C)1993 BARCREST"  and "DUT 0.4"
M4DTYFRE_SET( 1993, m4dtyfre,		0,			"duts.p1",					0x0000, 0x010000, CRC(8c7d6567) SHA1(8e82c4168d4d455c7cb95a895c04f7ad327894ec), "Barcrest","Duty Free (Barcrest) (MPU4) (DUT 0.4)" )
M4DTYFRE_SET( 1993, m4dtyfreutb,	m4dtyfre,	"dutb.p1",					0x0000, 0x010000, CRC(479acab7) SHA1(645e876b2c59dd4c091b5f168dcfd2cfa7eda0a3), "Barcrest","Duty Free (Barcrest) (MPU4) (DUT 0.4B)" )
M4DTYFRE_SET( 1993, m4dtyfreutc,	m4dtyfre,	"dutc.p1",					0x0000, 0x010000, CRC(654858eb) SHA1(4e95d6f1b84360b747a04d34bfda4d8c8ee3ea3b), "Barcrest","Duty Free (Barcrest) (MPU4) (DUT 0.4C)" )
// "(C)1993 BARCREST"  and "DF5 0.3"
M4DTYFRE_SET( 1993, m4dtyfref5,		m4dtyfre,	"df503s.p1",				0x0000, 0x010000, CRC(d5e80ed5) SHA1(b2d601b2a0020f4adf80b1256d31c8cce432ecee), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3)" )
M4DTYFRE_SET( 1993, m4dtyfref5d,	m4dtyfre,	"df503d.p1",				0x0000, 0x010000, CRC(3eab581a) SHA1(e1f358081953feccf1f03d733f29e839d5f51fcb), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3D)" )
M4DTYFRE_SET( 1993, m4dtyfref5ad,	m4dtyfre,	"df503ad.p1",				0x0000, 0x010000, CRC(348e375f) SHA1(f9a7e84afb33ec8fad14521eb2ea5d5cdfa48005), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3AD)" )
M4DTYFRE_SET( 1993, m4dtyfref5b,	m4dtyfre,	"df503b.p1",				0x0000, 0x010000, CRC(5eef10a2) SHA1(938e9a04fe54ac24dd93e9a1388c1dcf485ac212), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3B)" )
M4DTYFRE_SET( 1993, m4dtyfref5bd,	m4dtyfre,	"df503bd.p1",				0x0000, 0x010000, CRC(94840089) SHA1(a48668cdc1d7edae425cc80f2ce0f884f8619242), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3BD)" )
M4DTYFRE_SET( 1993, m4dtyfref5k,	m4dtyfre,	"df503k.p1",				0x0000, 0x010000, CRC(bc51cc39) SHA1(0bb977c14e66ec48cd64b01a509d8f0cecdc7880), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3K)" )
M4DTYFRE_SET( 1993, m4dtyfref5kd,	m4dtyfre,	"df503dk.p1",				0x0000, 0x010000, CRC(85ede229) SHA1(6799567df8078b69f897c0c5d8a315c6e3ef79b5), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3KD)" )
M4DTYFRE_SET( 1993, m4dtyfref5r,	m4dtyfre,	"df503r.p1",				0x0000, 0x010000, CRC(6b1940e0) SHA1(e8d3683d1ef65d2e7e035e9aab98ab9136f89464), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3R)" )
M4DTYFRE_SET( 1993, m4dtyfref5rd,	m4dtyfre,	"df503dr.p1",				0x0000, 0x010000, CRC(42721aa6) SHA1(8a29a4433d641ea37bbe3bf99f9222e8261dd63f), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3RD)" )
M4DTYFRE_SET( 1993, m4dtyfref5y,	m4dtyfre,	"df503y.p1",				0x0000, 0x010000, CRC(118642d4) SHA1(af2c86f0120f38652dc3d1141c5339a32bf73e11), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3Y)" )
M4DTYFRE_SET( 1993, m4dtyfref5yd,	m4dtyfre,	"df503dy.p1",				0x0000, 0x010000, CRC(cfce461e) SHA1(5bbbe878e89b1d775048945e259b711ef60de9a1), "Barcrest","Duty Free (Barcrest) (MPU4) (DF5 0.3YD)" )
// "(C)1993 BARCREST"  and "DF8 0.1"
M4DTYFRE_SET( 1993, m4dtyfref8,		m4dtyfre,	"df8s.p1",					0x0000, 0x010000, CRC(00571ce4) SHA1(39f5ecec8ccdefb68a8b9d2ab1cd0be6acb0c1c7), "Barcrest","Duty Free (Barcrest) (MPU4) (DF8 0.1)" )
M4DTYFRE_SET( 1993, m4dtyfref8d,	m4dtyfre,	"df8d.p1",					0x0000, 0x010000, CRC(df3a0ed7) SHA1(97569499f65e768a059fc86bdbcbde31e1977c23), "Barcrest","Duty Free (Barcrest) (MPU4) (DF8 0.1D)" )
M4DTYFRE_SET( 1993, m4dtyfref8c,	m4dtyfre,	"df8c.p1",					0x0000, 0x010000, CRC(07a82d24) SHA1(548576ce7c8d661777122e0d86d8273933beff11), "Barcrest","Duty Free (Barcrest) (MPU4) (DF8 0.1C)" )
M4DTYFRE_SET( 1993, m4dtyfref8k,	m4dtyfre,	"df8k.p1",					0x0000, 0x010000, CRC(056ac122) SHA1(9a993c0a7322323512a26b147963591212a226ab), "Barcrest","Duty Free (Barcrest) (MPU4) (DF8 0.1K)" )
M4DTYFRE_SET( 1993, m4dtyfref8y,	m4dtyfre,	"df8y.p1",					0x0000, 0x010000, CRC(cb902ef4) SHA1(efd7cb0a002aa54131725759cb73387f281f15a9), "Barcrest","Duty Free (Barcrest) (MPU4) (DF8 0.1Y)" )
M4DTYFRE_SET( 1993, m4dtyfref8yd,	m4dtyfre,	"df8dy.p1",					0x0000, 0x010000, CRC(0f24e42d) SHA1(1049f50bc8e0a2f7b77d8e3cdc8883b6879e5cd9), "Barcrest","Duty Free (Barcrest) (MPU4) (DF8 0.1YD)" )
// "(C)1993 BARCREST"  and "DFT 0.1"
M4DTYFRE_SET( 1993, m4dtyfreft,		m4dtyfre,	"dfts.p1",					0x0000, 0x010000, CRC(d6585e76) SHA1(91538ff218d8dd7a0d6747daaa9921d3e4b3ec33), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1)" )
M4DTYFRE_SET( 1993, m4dtyfreftd,	m4dtyfre,	"dftd.p1",					0x0000, 0x010000, CRC(9ac1f31f) SHA1(541a761c8755d1d85cedbba306ff7330d284480f), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1D)" )
M4DTYFRE_SET( 1993, m4dtyfreftad,	m4dtyfre,	"dftad.p1",					0x0000, 0x010000, CRC(045cedc1) SHA1(0f833077dee2b942e17ce49b5f506d9754ed0bc1), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1AD)" )
M4DTYFRE_SET( 1993, m4dtyfreftb,	m4dtyfre,	"dftb.p1",					0x0000, 0x010000, CRC(93567c8b) SHA1(8dc7d662ae4a5dd58240e90144c0c9905afc04f1), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1B)" )
M4DTYFRE_SET( 1993, m4dtyfreftbd,	m4dtyfre,	"dftbd.p1",					0x0000, 0x010000, CRC(b5e5b19a) SHA1(8533865e8c63498e808fb9b1da86fe0ac2a7efdc), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1BD)" )
M4DTYFRE_SET( 1993, m4dtyfreftk,	m4dtyfre,	"dftk.p1",					0x0000, 0x010000, CRC(ad9bb027) SHA1(630e334fdffbdecc903f75b9447c2c7993cf2656), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1K)" )
M4DTYFRE_SET( 1993, m4dtyfreftkd,	m4dtyfre,	"dftdk.p1",					0x0000, 0x010000, CRC(dbb4bf41) SHA1(c20b102a53f4d4ccbdb83433a80c77aa444a982d), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1KD)" )
M4DTYFRE_SET( 1993, m4dtyfrefty,	m4dtyfre,	"dfty.p1",					0x0000, 0x010000, CRC(0dead807) SHA1(a704ec65b1d6f91b4950181a792bb082c81fe668), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1Y)" )
M4DTYFRE_SET( 1993, m4dtyfreftyd,	m4dtyfre,	"dftdy.p1",					0x0000, 0x010000, CRC(6b12a337) SHA1(57cfa667a2ae3bea36d82ef32429638dc36533ad), "Barcrest","Duty Free (Barcrest) (MPU4) (DFT 0.1YD)" )
// "(C)1993 BARCREST"  and "XD5 0.2"
M4DTYFRE_SET( 1993, m4dtyfrexd,		m4dtyfre,	"xd502s.p1",				0x0000, 0x010000, CRC(223117c7) SHA1(9c017c4165db7076c76c081404d27742fd1f62e7), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2)" )
M4DTYFRE_SET( 1993, m4dtyfrexdd,	m4dtyfre,	"xd502d.p1",				0x0000, 0x010000, CRC(7b44a085) SHA1(d7e4c25e0d42a32f72afdb17b66425e1127373fc), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2D)" )
M4DTYFRE_SET( 1993, m4dtyfrexdad,	m4dtyfre,	"xd502ad.p1",				0x0000, 0x010000, CRC(62700345) SHA1(9825a9a6161e217ba4682902ac25528287d4ecf3), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2AD)" )
M4DTYFRE_SET( 1993, m4dtyfrexdb,	m4dtyfre,	"xd502b.p1",				0x0000, 0x010000, CRC(40069386) SHA1(0d065c2b528b406468354be68bbafdcac05f779d), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2B)" )
M4DTYFRE_SET( 1993, m4dtyfrexdbd,	m4dtyfre,	"xd502bd.p1",				0x0000, 0x010000, CRC(2cdc9833) SHA1(d3fa76c0a9a0113fbb7a83a47e3f7a72aeb942aa), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2BD)" )
M4DTYFRE_SET( 1993, m4dtyfrexdc,	m4dtyfre,	"xd502c.p1",				0x0000, 0x010000, CRC(17124bb6) SHA1(4ab22cffe11e84ff08bf0f026b0ca6d9a0d32bed), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2C)" )
M4DTYFRE_SET( 1993, m4dtyfrexdk,	m4dtyfre,	"xd502k.p1",				0x0000, 0x010000, CRC(c9a3b787) SHA1(c7166c9e809a37037dfdc616df5fbd6b6ff8b2f8), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2K)" )
M4DTYFRE_SET( 1993, m4dtyfrexdkd,	m4dtyfre,	"xd502dk.p1",				0x0000, 0x010000, CRC(790aac05) SHA1(db697b9a87d0266fabd23e1b085234e36c816170), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2KD)" )
M4DTYFRE_SET( 1993, m4dtyfrexdr,	m4dtyfre,	"xd502r.p1",				0x0000, 0x010000, CRC(4ddbd944) SHA1(c3df807ead3a50c7be73b084f65771e4b9d1f2d0), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2R)" )
M4DTYFRE_SET( 1993, m4dtyfrexdrd,	m4dtyfre,	"xd502dr.p1",				0x0000, 0x010000, CRC(77a14f87) SHA1(651b58c0a9ec13441c9bf8d7bf0d7c736337f171), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2RD)" )
M4DTYFRE_SET( 1993, m4dtyfrexdy,	m4dtyfre,	"xd502y.p1",				0x0000, 0x010000, CRC(d0b0f1aa) SHA1(39560550083952cae568d4d634c04bf48b7baca6), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2Y)" )
M4DTYFRE_SET( 1993, m4dtyfrexdyd,	m4dtyfre,	"xd502dy.p1",				0x0000, 0x010000, CRC(eaca6769) SHA1(1d3d1264d849043f0adcf9a32520e5f80ae17b5f), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.2YD)" )
// "(C)1993 BARCREST"  and "XD5 0.1"
M4DTYFRE_SET( 1993, m4dtyfrexd_a,	m4dtyfre,	"xd5s.p1",					0x0000, 0x010000, CRC(235ba9d1) SHA1(3a58c986f63c9ee75e91c59455b0a02582b4301b), "Barcrest","Duty Free (Barcrest) (MPU4) (XD5 0.1)" )
// "(C)1993 BARCREST"  and "XFT 0.1"
M4DTYFRE_SET( 1993, m4dtyfrexf,		m4dtyfre,	"xft01s.p1",				0x0000, 0x010000, CRC(fc107ba0) SHA1(661f1ab0d0192f77c355d5570885940d71174592), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1)" )
M4DTYFRE_SET( 1993, m4dtyfrexfd,	m4dtyfre,	"xft01d.p1",				0x0000, 0x010000, CRC(88391d1c) SHA1(f1b1034b962a03efd7d2cbe6ac0cc7328871a180), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1D)" )
M4DTYFRE_SET( 1993, m4dtyfrexfad,	m4dtyfre,	"xft01ad.p1",				0x0000, 0x010000, CRC(7299da07) SHA1(eb1371ce52e24fbfcac8f45166ca56d8aee9d403), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1AD)" )
M4DTYFRE_SET( 1993, m4dtyfrexfb,	m4dtyfre,	"xft01b.p1",				0x0000, 0x010000, CRC(c24904c4) SHA1(1c1b94b499f7a50e04b1287ce95633a8b0a5c0ea), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1B)" )
M4DTYFRE_SET( 1993, m4dtyfrexfbd,	m4dtyfre,	"xft01bd.p1",				0x0000, 0x010000, CRC(e67a0e47) SHA1(8115a5ab8b508ff30b28fa8f5d33f598385ee115), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1BD)" )
M4DTYFRE_SET( 1993, m4dtyfrexfc,	m4dtyfre,	"xft01c.p1",				0x0000, 0x010000, CRC(ee915038) SHA1(a0239268eae757e8e7ee16d9acb5dc28e7820b4e), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1C)" )
M4DTYFRE_SET( 1993, m4dtyfrexfk,	m4dtyfre,	"xft01k.p1",				0x0000, 0x010000, CRC(fbdc88b2) SHA1(231b6b8ba92a794ec363c1b853921e28e6b34fec), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1K)" )
M4DTYFRE_SET( 1993, m4dtyfrexfkd,	m4dtyfre,	"xft01dk.p1",				0x0000, 0x010000, CRC(dfef8231) SHA1(7610a7bcdb91a39cf86ac926818d02f4d751f099), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1KD)" )
M4DTYFRE_SET( 1993, m4dtyfrexfr,	m4dtyfre,	"xft01r.p1",				0x0000, 0x010000, CRC(dd8b05e6) SHA1(64a5aaaa6e7fb162c23ad0e36d39923e986b0fb4), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1R)" )
M4DTYFRE_SET( 1993, m4dtyfrexfrd,	m4dtyfre,	"xft01dr.p1",				0x0000, 0x010000, CRC(213f7fe5) SHA1(7e9cad6df7f4a58a0b98dbac552bf545a53ebfcd), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1RD)" )
M4DTYFRE_SET( 1993, m4dtyfrexfy,	m4dtyfre,	"xft01y.p1",				0x0000, 0x010000, CRC(39e49e72) SHA1(459e0d81b6d0d2aa44aa6a7a00cbdec4d9536df0), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1Y)" )
M4DTYFRE_SET( 1993, m4dtyfrexfyd,	m4dtyfre,	"xft01dy.p1",				0x0000, 0x010000, CRC(25fc8e71) SHA1(54c4c8c2118b4758dedb15f0a11f918f2ee0fb7d), "Barcrest","Duty Free (Barcrest) (MPU4) (XFT 0.1YD)" )
// "(C)1996  B.W.B."  and various ident strings, none boot, bad chr
M4DTYFRE_SET( 1996, m4dtyfrebwb,	m4dtyfre,	"4df5.10",					0x0000, 0x010000, CRC(01c9e06f) SHA1(6d9d4a43f621c4a80259040875a1fe851459b662), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF10 4.3, set 1)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_a,	m4dtyfre,	"dfree510l",				0x0000, 0x010000, CRC(7cf877a9) SHA1(54a87391832a641bf5f7104968b919dbb2bfa1eb), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF10 4.3, set 2)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_b,	m4dtyfre,	"4df5.8t",					0x0000, 0x010000, CRC(e8abec56) SHA1(84f6abc5e8b46c55052d308266000085374b12af), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF8 4.2)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_c,	m4dtyfre,	"bwb duty free 5.8.bin",	0x0000, 0x010000, CRC(c67e7315) SHA1(a70183b0937c138c96fd1a0cd5bacff1acd0cbdb), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF8 2.2, set 1)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_d,	m4dtyfre,	"df5 (2).8t",				0x0000, 0x010000, CRC(eb4cf0ae) SHA1(45c4e143a3e358c4bdc0c10e38039cba48a9e6dc), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF8 2.2, set 2)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_e,	m4dtyfre,	"4df5.4",					0x0000, 0x010000, CRC(60e21664) SHA1(2a343f16ece19396ad41eeac8c94a23d8e648d4f), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF4 4.1)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_f,	m4dtyfre,	"df5.4",					0x0000, 0x010000, CRC(14de7ecb) SHA1(f7445b33b2febbf93fd0398ab310ac104e79443c), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF4 2.1)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_g,	m4dtyfre,	"df5 (2).4",				0x0000, 0x010000, CRC(50f8566c) SHA1(364d33de4b34d0052ffc98536468c0a13f847a2a), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DF4 1.1)" )
M4DTYFRE_SET( 1996, m4dtyfrebwb_h,	m4dtyfre,	"df5.10",					0x0000, 0x010000, CRC(96acf53f) SHA1(1297a9162dea474079d0ea63b2b1b8e7f649230a), "Bwb","Duty Free (Bwb / Barcrest) (MPU4) (DFC 2.3)" )
// "1997  COCO"  and "DF4  4.1" (hack?)
M4DTYFRE_SET( 199?, m4dtyfre_h1,	m4dtyfre,	"dfre55",					0x0000, 0x010000, CRC(01e7d367) SHA1(638b709e4bb997998ccc7c4ea8adc33cabf2fe36), "hack?","Duty Free (Bwb / Barcrest) (MPU4) (DF4 4.1, hack?)" ) // bad chr
// "HI BIG BOY"  and "DFT 0.1" (hack?)
M4DTYFRE_SET( 199?, m4dtyfre_h2,	m4dtyfre,	"duty2010",					0x0000, 0x010000, CRC(48617f20) SHA1(dd35eef2357af6f88be42bb81608696ed97522c5), "hack?","Duty Free (Barcrest) (MPU4) (DFT 0.1, hack?)" ) // bad chr

#define M4RHOG_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "rhm.chr", 0x0000, 0x000048, CRC(e8417c98) SHA1(460c43327b41c95b7d091c04dbc9ce7b2e4773f6) ) \
	ROM_LOAD( "rr6s.chr", 0x0000, 0x000048, CRC(ca08d53a) SHA1(b419c45f46ee352cbdb0b38a8c3fd33383b61f3a) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "rr6snd.p1", 0x000000, 0x080000, CRC(a5ec3f46) SHA1(2d6f1adbbd8ac931a99a7d3d9caa2a7a117ac3fa) ) \
	ROM_LOAD( "rr6snd.p2", 0x080000, 0x080000, CRC(e5b72ef2) SHA1(dcdfa162db8bf3f9610709b5a8f3b695f42b2371) )


#define M4RHOG_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHOG_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 ,m4_showstring,ROT0,company,title,GAME_FLAGS ) \

// "(C)1991 BARCREST"  and "RR6 1.2"
M4RHOG_SET( 1991, m4rhog,			0,			"rr6s.p1",					0x0000, 0x010000, CRC(f978ca0b) SHA1(11eeac41f4c77b38b33baefb16dab7de1268d161), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2)" )
M4RHOG_SET( 1991, m4rhogr6d,		m4rhog,		"rr6d.p1",					0x0000, 0x010000, CRC(b61115ea) SHA1(92b97cc8b71eb31e8377a59344faaf0d800d1bdc), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2D)" )
M4RHOG_SET( 1991, m4rhogr6ad,		m4rhog,		"rr6ad.p1",					0x0000, 0x010000, CRC(f328204d) SHA1(057f28e7eaaa372b901a76250fb7ebf4403348ad), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2AD)" )
M4RHOG_SET( 1991, m4rhogr6b,		m4rhog,		"rr6b.p1",					0x0000, 0x010000, CRC(ccacd58e) SHA1(64b67e54e5568378a18ba99017078fcd4e6bc749), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2B)" )
M4RHOG_SET( 1991, m4rhogr6c,		m4rhog,		"rr6c.p1",					0x0000, 0x010000, CRC(b5783c69) SHA1(38c122455bed904c9fd683be1a8508a69cbad03f), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2C)" )
M4RHOG_SET( 1991, m4rhogr6k,		m4rhog,		"rr6k.p1",					0x0000, 0x010000, CRC(121d29bf) SHA1(8a6dcf345012b2c499acd32c6bb76eb81ada6fa9), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2K)" )
M4RHOG_SET( 1991, m4rhogr6y,		m4rhog,		"rr6y.p1",					0x0000, 0x010000, CRC(56344b28) SHA1(7f6c740d0991a646393a47e2e85322a7c92bdd62), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2Y)" )
M4RHOG_SET( 1991, m4rhogr6yd,		m4rhog,		"rr6dy.p1",					0x0000, 0x010000, CRC(0e540e0d) SHA1(a783e73822e436669c8cc1504619990725306df1), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.2YD)" )
// "(C)1991 BARCREST"  and "RR6 1.1"
M4RHOG_SET( 1991, m4rhogr6y_a,		m4rhog,		"rdhogvkn",					0x0000, 0x010000, CRC(3db03ada) SHA1(9b26f466c1dc1d03edacf64cbe507e084edf5f90), "Barcrest","Road Hog (Barcrest) (MPU4) (RR6 1.1Y)" )
// "(C)1995  B.W.B."  and "RO_ 3.0"
M4RHOG_SET( 1995, m4rhogr3,			m4rhog,		"rh5p8.bin",				0x0000, 0x010000, CRC(35d56379) SHA1(ab70ef8151823c3157cf4cc4f9b29875c6ac81cc), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 3.0)" )
// "(C)1994  B.W.B."  and "RO_ 2.0"
M4RHOG_SET( 1994, m4rhogr2,			m4rhog,		"ro_05s__.2_1",				0x0000, 0x010000, CRC(dc18f70f) SHA1(da81b8279e4f58b1447f51beb446a6007eb39df9), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0)" )
M4RHOG_SET( 1994, m4rhogr2d,		m4rhog,		"ro_05sd_.2_1",				0x0000, 0x010000, CRC(f230ae7e) SHA1(5525ed33d115b01722186587de20013265ac19b2), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0D)" )
M4RHOG_SET( 1994, m4rhogr2c,		m4rhog,		"roi05___.2_1",				0x0000, 0x010000, CRC(85fbd24a) SHA1(653a3cf3e651d94611caacddbd0692111667424a), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0C)" )
M4RHOG_SET( 1994, m4rhogr2k,		m4rhog,		"ro_05a__.2_1",				0x0000, 0x010000, CRC(67450ed1) SHA1(84cab7bb2411eb47c1336159bd1941862da59db3), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0K)" )
M4RHOG_SET( 1994, m4rhogr2y,		m4rhog,		"ro_05sk_.2_1",				0x0000, 0x010000, CRC(3e1dfedd) SHA1(a750663c96060b858e194445bc1e677b49da85b8), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0Y)" )
M4RHOG_SET( 1994, m4rhogr2yd,		m4rhog,		"ro_05sb_.2_1",				0x0000, 0x010000, CRC(4a33cfcf) SHA1(ac5d4873df74b521018d5eeac96fd7003ee093e8), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0YD)" )
// "(C)1994  B.W.B."  and "RO_ 1.0"
M4RHOG_SET( 1994, m4rhogr1,			m4rhog,		"ro_10s__.1_1",				0x0000, 0x010000, CRC(d140597a) SHA1(0ddf898b5db2a1cbfda84e8a63e0be3de7582cbd), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0)" )
M4RHOG_SET( 1994, m4rhogr1d,		m4rhog,		"ro_10sd_.1_1",				0x0000, 0x010000, CRC(3f9152f3) SHA1(97e0c0461b8d4994515ac9e20d001dc7e74042ec), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0D)" )
M4RHOG_SET( 1994, m4rhogr1c,		m4rhog,		"roi10___.1_1",				0x0000, 0x010000, CRC(2f832f4b) SHA1(b9228e2585cff6d4d9df64048c77e0b9ad3e75d7), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0C)" )
M4RHOG_SET( 1994, m4rhogr1k,		m4rhog,		"ro_10a__.1_1",				0x0000, 0x010000, CRC(1772bce6) SHA1(c5d0cec8e5bcfcef5003325169522f1da066354b), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0K, set 1)" )
M4RHOG_SET( 1994, m4rhogr1y,		m4rhog,		"ro_10sk_.1_1",				0x0000, 0x010000, CRC(5d5118d1) SHA1(c4abc5ccdeb711b6ec2a2c82bb2f8da9d824fe4e), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0Y)" )
M4RHOG_SET( 1994, m4rhogr1yd,		m4rhog,		"ro_10sb_.1_1",				0x0000, 0x010000, CRC(34febd6f) SHA1(e1d5e178771714f9633dd9782c1f9d373a9ca5e1), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0YD)" )
M4RHOG_SET( 1994, m4rhogr1k_a,		m4rhog,		"rhog5p",					0x0000, 0x010000, CRC(49b11beb) SHA1(89c2320de4b3f2ff6ba28501f88147b659f1ee20), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0K, set 2, wrong version number?)" ) // clearly not the same version as above, more code...
// "HAVE A NICE DAY"  and "RO_ 2.0" (won't boot)
M4RHOG_SET( 1994, m4rhog_h1,		m4rhog,		"road hog 5p 6.bin",		0x0000, 0x010000, CRC(b365d1f0) SHA1(af3b4f5162af6c033039a1e004bc803175a4e996), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0, hack?, set 1)" )
M4RHOG_SET( 1994, m4rhog_h2,		m4rhog,		"rhog05_11",				0x0000, 0x010000, CRC(8e4b14aa) SHA1(8b67b34597c0d30b0b3cf2566536c02f880a74bc), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0, hack?, set 2)" )
M4RHOG_SET( 1994, m4rhog_h3,		m4rhog,		"rhog55",					0x0000, 0x010000, CRC(29395082) SHA1(538434b82e31f7e40770a9b882e54a16195ee998), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0, hack?, set 3)" )
M4RHOG_SET( 1994, m4rhog_h4,		m4rhog,		"rhog58c",					0x0000, 0x010000, CRC(e02b6da6) SHA1(7d329adcac594c98685dc5404f2b9e8f717cc47f), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0, hack?, set 4)" )
M4RHOG_SET( 1994, m4rhog_h5,		m4rhog,		"rh056c",					0x0000, 0x010000, CRC(073845e2) SHA1(5e6f3ccdfc346f95e5e7e955144332e727da1d9e), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0, hack?, set 5)" )
M4RHOG_SET( 1994, m4rhog_h6,		m4rhog,		"rhog_05_.4",				0x0000, 0x010000, CRC(a75a2bd4) SHA1(d21505d27792acf8fa20a7cdc830efbe8756fe81), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0, hack?, set 6)" )
M4RHOG_SET( 1994, m4rhog_h7,		m4rhog,		"rhog_05_.8",				0x0000, 0x010000, CRC(5476f9b4) SHA1(fbd038e8710a79ea697d5acb482bed2f307cefbb), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 2.0, hack?, set 7)" )
// "HAVE A NICE DAY"  and "RO_ 1.0" (won't boot)
M4RHOG_SET( 1994, m4rhog_h8,		m4rhog,		"rhog10_11",				0x0000, 0x010000, CRC(83575be7) SHA1(2cb549554028f2fdc32ecfa58b786de375b8fa35), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0, hack?, set 1)" )
M4RHOG_SET( 1994, m4rhog_h9,		m4rhog,		"rhog10c",					0x0000, 0x010000, CRC(308c6d4f) SHA1(f7f8063fe8dd4ef204f225d0aa5202732ead5fa0), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0, hack?, set 2)" )
M4RHOG_SET( 1994, m4rhog_h10,		m4rhog,		"rhog_10_.4",				0x0000, 0x010000, CRC(8efa581c) SHA1(03c25b674cfb02792edc9ef8a76b16af31d80aae), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0, hack?, set 3)" )
M4RHOG_SET( 1994, m4rhog_h11,		m4rhog,		"rhog_10_.8",				0x0000, 0x010000, CRC(84d1f95d) SHA1(33f10e0e1e5abe6011b05f32f55c7dd6d3298945), "hack?","Road Hog (Bwb / Barcrest) (MPU4) (RO_ 1.0, hack?, set 4)" )
// "(C)1991 BARCREST"  and "RR6 1.2" but won't boot, and we already have valid roms above, hacked?
M4RHOG_SET( 1991, m4rhog_h12,		m4rhog,		"rhog20c",					0x0000, 0x010000, CRC(74ec16f7) SHA1(995d75b3a4e88d8a34dc395b185f728c18e00a2b), "hack?","Road Hog (Barcrest) (MPU4) (RR6 1.2?, hack?)" )
M4RHOG_SET( 1991, m4rhog_h13,		m4rhog,		"rhog_20_.8",				0x0000, 0x010000, CRC(3a82e4bf) SHA1(6582951c2afe14502c37460381bf4c28ec02f3c9), "hack?","Road Hog (Barcrest) (MPU4) (RR6 1.2, hack?)" )
M4RHOG_SET( 1991, m4rhog_h14,		m4rhog,		"rhog_20_.4",				0x0000, 0x010000, CRC(15e28457) SHA1(2a758a727a6956e3029b2026cd189f6249677c6a), "hack?","Road Hog (Barcrest) (MPU4) (RR6 1.2C, hack?, set 1)" )
M4RHOG_SET( 1991, m4rhog_h15,		m4rhog,		"rhog20_11",				0x0000, 0x010000, CRC(63c80ee0) SHA1(22a3f11007acedd833af9e73e3038fb3542781fe), "hack?","Road Hog (Barcrest) (MPU4) (RR6 1.2C, hack?, set 2)" )
// "(C)1995  B.W.B."  and "ROC 2.0"  (bad, and possible wrong game, club version?)
M4RHOG_SET( 1995, m4rhog_roc,		m4rhog,		"roadhog5p4std.bin",		0x0000, 0x010000, BAD_DUMP CRC(0ff60341) SHA1(c12d5b160d9e47a6f1aa6f378c2a70186be6bdff), "Bwb","Road Hog (Bwb / Barcrest) (MPU4) (ROC 2.0, bad)" )
// "(C)1991 BARCREST"  and "RH8 0.1" (wrong game!)
M4RHOG_SET( 1991, m4rh8,			m4rhog,		"rh8c.p1",					0x0000, 0x010000, CRC(e36d7ca0) SHA1(73970761c5c7004669b02ba9f3a299f36f2d00e9), "Barcrest","unknown (Barcrest) (MPU4) (RH8 0.1C)" )


#define M4ANDYGE_EXTRA_ROMS \
	ROM_REGION( 0x1200, "plds", 0 ) /* PAL16V8 PLD, like others - CHR? Guess it should be here... */  \
	ROM_LOAD( "age.bin", 0x0000, 0x000117, CRC(901359e5) SHA1(7dbcd6023e7ce68f4aa7f191f572d74f21f978aa) ) \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "char.chr", 0x0000, 0x000048, CRC(053a5846) SHA1(c92de79e568c9f253bb71dbda2ca32b7b3b6661a) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "an2snd.p1",  0x000000, 0x080000,  CRC(5394e9ae) SHA1(86ccd8531fc87f34d3c5482ba7e5a2c06ea69491) ) \
	ROM_LOAD( "an2snd.p2",  0x080000, 0x080000,  CRC(109ace1f) SHA1(9f0e8065186beb61ed50fea834de2d91e68db953) )

#define M4ANDYGE_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4ANDYGE_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,grtecp ,m_grtecpss ,ROT0,company,title,GAME_FLAGS ) \

// "(C)1991 BARCREST"  and "AN2 0.3"
M4ANDYGE_SET( 1991, m4andyge,			0,			"an2s.p1",					0x0000, 0x010000, CRC(65399fa0) SHA1(ecefdf63e7aa477001fa530ed340e90e85252c3c), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (AN2 0.3, set 1)" ) // one of these is probably hacked
M4ANDYGE_SET( 1991, m4andygen2_a,		m4andyge,	"agesc20p",					0x0000, 0x010000, CRC(94fec0f3) SHA1(7678e01a4e0fcc4136f6d4a668c4d1dd9a8f1246), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (AN2 0.3, set 2)" ) // or has the wrong id strings
M4ANDYGE_SET( 1991, m4andygen2d,		m4andyge,	"an2d.p1",					0x0000, 0x010000, CRC(5651ed3d) SHA1(6a1fbff252bf266b03c4cb64294053f686a523d6), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (AN2 0.3D)" )
M4ANDYGE_SET( 1991, m4andygen2c,		m4andyge,	"an2c.p1",					0x0000, 0x010000, CRC(3e233c24) SHA1(4e8f0cb45851db509020afd47821893ab49448d7), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (AN2 0.3C)" )
M4ANDYGE_SET( 1991, m4andygen2k,		m4andyge,	"an2k.p1",					0x0000, 0x010000, CRC(c0886dff) SHA1(ef2b509fde05ef4ef055a09275afc9e153f50efc), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (AN2 0.3K)" )
M4ANDYGE_SET( 1991, m4andygen2y,		m4andyge,	"an2y.p1",					0x0000, 0x010000, CRC(a9cd1ed2) SHA1(052fc711efe633a2ece6bf24fabdc0b69b9355fd), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (AN2 0.3Y)" )
// "(C)1991 BARCREST"  and "A28 0.1"
M4ANDYGE_SET( 1991, m4andyge28,			m4andyge,	"a28s.p1",					0x0000, 0x010000, CRC(40529bad) SHA1(d22b0e8a8f4acec78dc05cde01d68b625008f3b0), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1)" )
M4ANDYGE_SET( 1991, m4andyge28d,		m4andyge,	"a28d.p1",					0x0000, 0x010000, CRC(e8eee34e) SHA1(c223a8c1fd2c609376bab9e780020523c4e76b08), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1D)" )
M4ANDYGE_SET( 1991, m4andyge28ad,		m4andyge,	"a28ad.p1",					0x0000, 0x010000, CRC(ecb0b180) SHA1(23d68e34e7a58fc6574e6c8524ce2e4e4cd25582), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1AD)" )
M4ANDYGE_SET( 1991, m4andyge28b,		m4andyge,	"a28b.p1",					0x0000, 0x010000, CRC(481c6c1c) SHA1(d8133d87e481f9c01c60324e918f706da6486c1b), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1B)" )
M4ANDYGE_SET( 1991, m4andyge28bd,		m4andyge,	"a28bd.p1",					0x0000, 0x010000, CRC(a59430b1) SHA1(000a00ba115408ab35fea74faa745220a9fcad68), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1BD)" )
M4ANDYGE_SET( 1991, m4andyge28c,		m4andyge,	"a28c.p1",					0x0000, 0x010000, CRC(e74533db) SHA1(f6f77dc61c08cdced0dca9133dfeeb5fdd4076f0), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1C)" )
M4ANDYGE_SET( 1991, m4andyge28k,		m4andyge,	"a28k.p1",					0x0000, 0x010000, CRC(c83b94fa) SHA1(8194b25bfcb8ba0323c63ee2f2b45f030aa1caeb), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1K)" )
M4ANDYGE_SET( 1991, m4andyge28kd,		m4andyge,	"a28dk.p1",					0x0000, 0x010000, CRC(115a2bc1) SHA1(31736f9583b4f110a6c838cecbd47acb7baa58c9), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1KD)" )
M4ANDYGE_SET( 1991, m4andyge28y,		m4andyge,	"a28y.p1",					0x0000, 0x010000, CRC(fb1c83b7) SHA1(76b40e1ea47732ae0f6e9557c2d0445421122ac8), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1Y)" )
M4ANDYGE_SET( 1991, m4andyge28yd,		m4andyge,	"a28dy.p1",					0x0000, 0x010000, CRC(05ef8b21) SHA1(762aaad6892511ba1f3266c1ed0a09850339cc63), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A28 0.1YD)" )
// "(C)1991 BARCREST"  and "A2T 0.1"
M4ANDYGE_SET( 1991, m4andyge2t,			m4andyge,	"a2ts.p1",					0x0000, 0x010000, CRC(d47c9c42) SHA1(5374cb5739a5c2ab2be32166c4819682f3266320), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1)" )
M4ANDYGE_SET( 1991, m4andyge2td,		m4andyge,	"a2td.p1",					0x0000, 0x010000, CRC(ad17a652) SHA1(86006c706768a9227a21eb8da25817f4efacaa39), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1D)" )
M4ANDYGE_SET( 1991, m4andyge2tad,		m4andyge,	"a2tad.p1",					0x0000, 0x010000, CRC(0e3971d7) SHA1(f8de4a932937923d585f816fc9bffbe9887011c1), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1AD)" )
M4ANDYGE_SET( 1991, m4andyge2tb,		m4andyge,	"a2tb.p1",					0x0000, 0x010000, CRC(d8c4bf4d) SHA1(06e082db39576f2da39866bdb8daab49e2b4108d), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1B)" )
M4ANDYGE_SET( 1991, m4andyge2tbd,		m4andyge,	"a2tbd.p1",					0x0000, 0x010000, CRC(ed048ad0) SHA1(a2ffae901171363ccb827c7bf6299f29b0347e3c), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1BD)" )
M4ANDYGE_SET( 1991, m4andyge2tk,		m4andyge,	"a2tk.p1",					0x0000, 0x010000, CRC(8ca6ce3d) SHA1(6c869eceea88109b23a2b850deda6c5a46ca5a48), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1K)" )
M4ANDYGE_SET( 1991, m4andyge2tkd,		m4andyge,	"a2tdk.p1",					0x0000, 0x010000, CRC(f11bd420) SHA1(0904ecf296474ee5283da26d8c728af438aac595), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1KD)" )
M4ANDYGE_SET( 1991, m4andyge2ty,		m4andyge,	"a2ty.p1",					0x0000, 0x010000, CRC(30c22b5d) SHA1(be87fcbfb13c34c3d0ee1f586e887c80ffa01245), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1Y)" )
M4ANDYGE_SET( 1991, m4andyge2tyd,		m4andyge,	"a2tdy.p1",					0x0000, 0x010000, CRC(0ffcb8d7) SHA1(b1d591eed982d2bc2e02b96e2561bbb372242480), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A2T 0.1YD)" )
// "(C)1991 BARCREST"  and "A5T 0.1"
M4ANDYGE_SET( 1991, m4andyge5t,			m4andyge,	"a5ts.p1",					0x0000, 0x010000, CRC(9ab99a1e) SHA1(605c5ee71aa0583f02e9ced604692814e33b741a), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1)" )
M4ANDYGE_SET( 1991, m4andyge5td,		m4andyge,	"a5td.p1",					0x0000, 0x010000, CRC(b3ebc357) SHA1(6d0718474f83f71151189c3175b687564c1d49b0), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1D)" )
M4ANDYGE_SET( 1991, m4andyge5tad,		m4andyge,	"a5tad.p1",					0x0000, 0x010000, CRC(df767538) SHA1(17ca5ea5b217fda448f61412cae82ae61447c5ad), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1AD)" )
M4ANDYGE_SET( 1991, m4andyge5tb,		m4andyge,	"a5tb.p1",					0x0000, 0x010000, CRC(e6f22d3f) SHA1(f6da8edc0b058ce316ccca306f930469ef6d016c), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1B)" )
M4ANDYGE_SET( 1991, m4andyge5tbd,		m4andyge,	"a5tbd.p1",					0x0000, 0x010000, CRC(24aa63c8) SHA1(838f1fff46c65dd56f25fd491f8aab3be826a845), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1BD)" )
M4ANDYGE_SET( 1991, m4andyge5tk,		m4andyge,	"a5tk.p1",					0x0000, 0x010000, CRC(c63209f8) SHA1(71968dd94431610ddef35bb4cf8dcba749470a26), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1K)" )
M4ANDYGE_SET( 1991, m4andyge5tkd,		m4andyge,	"a5tdk.p1",					0x0000, 0x010000, CRC(67472634) SHA1(aae14b9ea4125b94dd1a7325c000629258573499), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1KD)" )
M4ANDYGE_SET( 1991, m4andyge5ty,		m4andyge,	"a5ty.p1",					0x0000, 0x010000, CRC(86ef0bd8) SHA1(870b8165e206f84e59a3badfba441a567626f297), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1Y)" )
M4ANDYGE_SET( 1991, m4andyge5tyd,		m4andyge,	"a5tdy.p1",					0x0000, 0x010000, CRC(9f9c15c2) SHA1(0e6471c62450bd8468adde1a2d69c5b24c472bfc), "Barcrest","Andy's Great Escape (Barcrest) (MPU4) (A5T 0.1YD)" )
// "(C)1995  B.W.B."  and "AGC 2.0"
M4ANDYGE_SET( 1995, m4andygegc2,		m4andyge,	"ag_05__c.2_1",				0x0000, 0x010000, CRC(c38c11a3) SHA1(c2d81d99a842eac8dff3e0be57f37af9eb534ad1), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AGC 2.0)" )
M4ANDYGE_SET( 1995, m4andygegc2d,		m4andyge,	"ag_05_d4.2_1",				0x0000, 0x010000, CRC(29953aa1) SHA1(c1346ab7e651c35d704e5127c4d44d2086fd48e3), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AGC 2.0D)" )
// "(C)1994  B.W.B."  and "AG5 3.0"
M4ANDYGE_SET( 1994, m4andygeg5,			m4andyge,	"ag_05s__.3_1",				0x0000, 0x010000, CRC(c0e45872) SHA1(936ca3230cd36dd4ad2c74ea33ea469c482e5688), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0)" )
M4ANDYGE_SET( 1994, m4andygeg5d,		m4andyge,	"ag_05sd_.3_1",				0x0000, 0x010000, CRC(b7fced5c) SHA1(6b359b29019bf22b2ebdd96a69f919b18935a98c), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0D)" )
M4ANDYGE_SET( 1994, m4andygeg5a,		m4andyge,	"agesc5p",					0x0000, 0x010000, CRC(9de05e25) SHA1(b4d6aea5cffb14babd89cfa76575a68277bfaa4b), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0A)" )
M4ANDYGE_SET( 1994, m4andygeg5c,		m4andyge,	"agi05___.3_1",				0x0000, 0x010000, CRC(b061a468) SHA1(a1f1a8bd55eb7a684de270bace9464812172ed92), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0C)" )
M4ANDYGE_SET( 1994, m4andygeg5k,		m4andyge,	"ag_05a__.3_1",				0x0000, 0x010000, CRC(89f4281e) SHA1(3ada70d7c5ef523f1a4eddfc8f1967e4a6de190d), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0K)" )
M4ANDYGE_SET( 1994, m4andygeg5yd,		m4andyge,	"ag_05sb_.3_1",				0x0000, 0x010000, CRC(f5055b62) SHA1(b12a7d2a1143ce47e6a327831d5df21483d78b03), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0YD)" )
// "(C)1994  B.W.B."  and "AG__2.0"
M4ANDYGE_SET( 1994, m4andygeg_2,		m4andyge,	"ag_10s__.2_1",				0x0000, 0x010000, CRC(0dfeda46) SHA1(27e7548845f116537043e26002d8a5458275389d), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG__2.0)" )
M4ANDYGE_SET( 1994, m4andygeg_2d,		m4andyge,	"ag_10sd_.2_1",				0x0000, 0x010000, CRC(03ab435f) SHA1(3b04324c1ae839529d99255008874df3744769a4), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG__2.0D)" )
M4ANDYGE_SET( 1994, m4andygeg_2c,		m4andyge,	"agi10___.2_1",				0x0000, 0x010000, CRC(7c56a6ca) SHA1(adb567b8e1b6cc727bcfa694ade947f8c695f44a), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG__2.0C)" )
M4ANDYGE_SET( 1994, m4andygeg_2k,		m4andyge,	"ag_10a__.2_1",				0x0000, 0x010000, CRC(ca80d891) SHA1(17bf51fecc3cecbb1e0ef0550296c8bf81d3d879), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG__2.0K)" )
M4ANDYGE_SET( 1994, m4andygeg_2yd,		m4andyge,	"ag_10sb_.2_1",				0x0000, 0x010000, CRC(6f025416) SHA1(bb0167ba0a67dd1a03ec3e69e2050e2bf1d35244), "Bwb","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG__2.0YD)" )
// "(C)1994  B.W.B."  and "AG5 3.0"  (are these legit? they don't seem to care much about the chr)
M4ANDYGE_SET( 1994, m4andyge_hx1,		m4andyge,	"acappgreatescape5p4.bin",	0x0000, 0x010000, CRC(87733a0d) SHA1(6e2fc0f43eb48740b120af77302f1322a27e8a5a), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0CX, hack?, set 1)" )
M4ANDYGE_SET( 1994, m4andyge_hx2,		m4andyge,	"age55",					0x0000, 0x010000, CRC(481e942d) SHA1(23ac3c4f624ae73940baf515002a178d39ba32b0), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0CX, hack?, set 2)" )
M4ANDYGE_SET( 1994, m4andyge_hx3,		m4andyge,	"age58c",					0x0000, 0x010000, CRC(0b1e4a0e) SHA1(e2bcd590a358e48b26b056f83c7180da0e036024), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0CX, hack?, set 3)" )
M4ANDYGE_SET( 1994, m4andyge_hx4,		m4andyge,	"age05_101",				0x0000, 0x010000, CRC(70c1d1ab) SHA1(478891cadaeba76666af5c4f25531456ebbe789a), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG5 3.0CX, hack?, set 4)" )
// "               "  and "AG__2.0"
M4ANDYGE_SET( 1994, m4andyge_hx5,		m4andyge,	"age10_101",				0x0000, 0x010000, CRC(55e3a27e) SHA1(209166d052cc296f135225c77bb57abbef1a86ae), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (AG__2.0CX, hack?)" )
// "RICK LUVS BRIAN"  and "8V1 3.0"
M4ANDYGE_SET( 199?, m4andyge_h1,		m4andyge,	"age5p8p.bin",				0x0000, 0x010000, CRC(c3b40981) SHA1(da56e468ae67f1a231fea721235036c75c5efac3), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (8V1 3.0, hack?, set 1)" )
M4ANDYGE_SET( 199?, m4andyge_h2,		m4andyge,	"ages58c",					0x0000, 0x010000, CRC(af479dc9) SHA1(7e0e3b36289d689bbd0c022730d7aee62192f49f), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (8V1 3.0, hack?, set 2)" )
// "               "  and "8V1 0.3"
M4ANDYGE_SET( 199?, m4andyge_h3,		m4andyge,	"age_20_.8",				0x0000, 0x010000, CRC(b1f91b2a) SHA1(9340f87d6d186b3af0384ab546c3d3f487e797d4), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (8V1 0.3, hack?, set 1)" )
M4ANDYGE_SET( 199?, m4andyge_h4,		m4andyge,	"age20_101",				0x0000, 0x010000, CRC(7e3674f0) SHA1(351e353da24b63d2ef7cb09690b770b26505569a), "hack?","Andy's Great Escape (Bwb / Barcrest) (MPU4) (8V1 0.3, hack?, set 2)" )


#define M4ADDR_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
	ROM_END \
	GAME(year, setname, parent ,mod2	,mpu4 ,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \

// all the adders and ladders sets kill the cpu, end up jumping to the ram area after an RTI/RTS combo? are we saturating the CPU with too many interrupts or is there a bug?
// also the BWB versioning is.. illogical
// I think this is a mod2, but because it doesn't boot I haven't moved it to mpu4mod2sw.c yet

// "(C)1991 BARCREST"  and "A6L 0.1"
M4ADDR_SET( 1991, m4addr,		0,		"a6ls.p1",					0x0000, 0x010000, CRC(9f97f57b) SHA1(402d1518bb78fdc489b06c2aabc771e5ce151847), "Barcrest","Adders & Ladders (Barcrest) (MPU4) (A6L 0.1)" )
M4ADDR_SET( 1991, m4addr6ld,	m4addr,	"a6ld.p1",					0x0000, 0x010000, CRC(de555e12) SHA1(2233160f1c734c889c1c00dee202a928f18ad763), "Barcrest","Adders & Ladders (Barcrest) (MPU4) (A6L 0.1D)" )
M4ADDR_SET( 1991, m4addr6lc,	m4addr,	"a6lc.p1",					0x0000, 0x010000, CRC(1e75fe67) SHA1(4497b19d4c512c934d445b4acf607dc2dc080d44), "Barcrest","Adders & Ladders (Barcrest) (MPU4) (A6L 0.1C)" )
M4ADDR_SET( 1991, m4addr6lk,	m4addr,	"a6lk.p1",					0x0000, 0x010000, CRC(af5ae5c4) SHA1(20e40cf996c2c3b7b18ec104a374be1da193b94e), "Barcrest","Adders & Ladders (Barcrest) (MPU4) (A6L 0.1K)" )
M4ADDR_SET( 1991, m4addr6ly,	m4addr,	"adders ladders 20p 6.bin",	0x0000, 0x010000, CRC(62abeb34) SHA1(8069e6fde0673fdbc124a1a172dc988bb3205ff6), "Barcrest","Adders & Ladders (Barcrest) (MPU4) (A6L 0.1Y)" )
M4ADDR_SET( 1991, m4addr6lyd,	m4addr,	"a6ldy.p1",					0x0000, 0x010000, CRC(82f060a5) SHA1(2e8474e6c17def07e35448b5bf8d453cce0f292c), "Barcrest","Adders & Ladders (Barcrest) (MPU4) (A6L 0.1YD)" )
M4ADDR_SET( 1991, m4addr6lybd,	m4addr,	"a6lbdy.p1",				0x0000, 0x010000, CRC(28064099) SHA1(c916f73911974440d4c79ecb51b343aad78f115b), "Barcrest","Adders & Ladders (Barcrest) (MPU4) (A6L 0.1YBD)" )
// "(C)1994  B.W.B."  and "ADD 1.0" (actually version 10?)
M4ADDR_SET( 1994, m4addr10,		m4addr,	"ad_05___.1o3",				0x0000, 0x010000, CRC(8d9e0f5d) SHA1(fecc844908876e161d0134ce3cc098e79e74e0b1), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0, set 1)" )
M4ADDR_SET( 1994, m4addr10d,	m4addr,	"ad_05_d_.1o3",				0x0000, 0x010000, CRC(2d29040f) SHA1(ee2bdd5da1a7e4146419ffd8bad521a9c1b49aa2), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0D, set 1)" )
M4ADDR_SET( 1994, m4addr10c,	m4addr,	"adi05___.1o3",				0x0000, 0x010000, CRC(050764b1) SHA1(364c50e4887c9fdd7ff62e63a6be4513336b4814), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0C, set 1)" )
M4ADDR_SET( 1994, m4addr10yd,	m4addr,	"ad_05_b_.1o3",				0x0000, 0x010000, CRC(b10b194a) SHA1(4dc3f14ff3b903c49829f4a91136f9b03a5cb1ae), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0YD, set 1)" )
M4ADDR_SET( 1994, m4addr10_a,	m4addr,	"ad_10___.1o3",				0x0000, 0x010000, CRC(d587cb00) SHA1(6574c42402f13e5f9cb8f951e0f59b499b2d025d), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0, set 2)" )
M4ADDR_SET( 1994, m4addr10d_a,	m4addr,	"ad_10_d_.1o3",				0x0000, 0x010000, CRC(d7670d32) SHA1(09dfe2a7fe267f485efed234411efc92d9cce414), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0D, set 2)" )
M4ADDR_SET( 1994, m4addr10c_a,	m4addr,	"adi10___.1o3",				0x0000, 0x010000, CRC(005caaa1) SHA1(b4b421c045012b5fbeaca95fa09d087a9c5e6b5b), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0C, set 2)" )
M4ADDR_SET( 1994, m4addr10yd_a,	m4addr,	"ad_10_b_.1o3",				0x0000, 0x010000, CRC(e2b5c0db) SHA1(9e1716186bb049c61dddaef2465fb1e55d2d93fd), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0YD, set 2)" )
// "(C)1993  B.W.B."  and "ADD 3.0"
M4ADDR_SET( 1993, m4addr3,		m4addr,	"ad_05___.3q3",				0x0000, 0x010000, CRC(ec6ed7ce) SHA1(dfad04b5f6c4ff0fd784ad20471f1cf84586f2cd), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0, set 1)" )
M4ADDR_SET( 1993, m4addr3d,		m4addr,	"ad_05_d_.3q3",				0x0000, 0x010000, CRC(8d05fba9) SHA1(9c69d7eec7ce0d647d4f8b8b0a6b7e54daa7a79f), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0D, set 1)" )
M4ADDR_SET( 1993, m4addr3yd,	m4addr,	"ad_05_b_.3q3",				0x0000, 0x010000, CRC(d4c06db1) SHA1(dacb66b98f9d1d51eddc48b6946d517c277e588e), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0YD, set 1)" )
M4ADDR_SET( 1993, m4addr3_a,	m4addr,	"ad_20___.3a3",				0x0000, 0x010000, CRC(c2431657) SHA1(b2b7541207eb3c898f9cf3df520bff396213b78a), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0, set 2)" )
M4ADDR_SET( 1993, m4addr3d_a,	m4addr,	"ad_20_d_.3a3",				0x0000, 0x010000, CRC(62304025) SHA1(59b7815bf1b5337f46083cef186fedd078a4ad37), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0D, set 2)" )
M4ADDR_SET( 1993, m4addr3yd_a,	m4addr,	"ad_20_b_.3a3",				0x0000, 0x010000, CRC(19990a19) SHA1(ab1031513fb1e499da4a3001b5b26ff1e86cc628), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0YD, set 2)" )
M4ADDR_SET( 1993, m4addr3_b,	m4addr,	"ad_20___.3n3",				0x0000, 0x010000, CRC(883ff001) SHA1(50540270dba31820ad99a4a4034c69d4a58d87c5), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0, set 3)" )
M4ADDR_SET( 1993, m4addr3d_b,	m4addr,	"ad_20_d_.3n3",				0x0000, 0x010000, CRC(cf254a00) SHA1(1e430b652e4023e28b5648b8bea63e778c6dafc9), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0D, set 3)" )
M4ADDR_SET( 1993, m4addr3yd_b,	m4addr,	"ad_20_b_.3n3",				0x0000, 0x010000, CRC(65f9946f) SHA1(6bf6f315ed2dc6f603381d36dd408e951ace76bc), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0YD, set 3)" )
M4ADDR_SET( 1993, m4addr3_c,	m4addr,	"ad_20___.3s3",				0x0000, 0x010000, CRC(b1d54cb6) SHA1(35205975ccdaccd5bf3c1b7bf9a26c5ef30050b3), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0, set 4)" )
M4ADDR_SET( 1993, m4addr3d_c,	m4addr,	"ad_20_d_.3s3",				0x0000, 0x010000, CRC(89d2301b) SHA1(62ad1a9e008063eb16442b50af806f061669dba7), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0D, set 4)" )
M4ADDR_SET( 1993, m4addr3yd_c,	m4addr,	"ad_20_b_.3s3",				0x0000, 0x010000, CRC(86982248) SHA1(a6d876333777a29eb0504fa3636727ebcc104f0a), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0YD, set 4)" )
M4ADDR_SET( 1993, m4addr3_d,	m4addr,	"adl5pv2",					0x0000, 0x010000, CRC(09c39527) SHA1(16af3e552a7d6c6b802d2b1923523e9aa9de766a), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 3.0, set 5)" )
// "(C)1994  B.W.B."  and "ADD 5.0"
M4ADDR_SET( 1994, m4addr5,		m4addr,	"ad_05___.5a3",				0x0000, 0x010000, CRC(9821a988) SHA1(2be85a0b68e5e31401a5c753b40f3cf803589444), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0, set 1)" )
M4ADDR_SET( 1994, m4addr5d,		m4addr,	"ad_05_d_.5a3",				0x0000, 0x010000, CRC(b5be8114) SHA1(28dfe1d1cc1d9fc2bcc13fd6437602a6e8c90de2), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0D, set 1)" )
M4ADDR_SET( 1994, m4addr5c,		m4addr,	"adi05___.5a3",				0x0000, 0x010000, CRC(03777f8c) SHA1(9e3fddc2130600f343df0531bf3e636b82c2f108), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0C, set 1)" )
M4ADDR_SET( 1994, m4addr5yd,	m4addr,	"ad_05_b_.5a3",				0x0000, 0x010000, CRC(592cb1ae) SHA1(5696ecb3e9e6419f73087120b6a832fde606bacc), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0YD, set 1)" )
M4ADDR_SET( 1994, m4addr5_a,	m4addr,	"ad_05___.5n3",				0x0000, 0x010000, CRC(86ac3564) SHA1(1dd9cf39d2aee11a3e1bbc68460c12f10e62aeaf), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0, set 2)" )
M4ADDR_SET( 1994, m4addr5d_a,	m4addr,	"ad_05_d_.5n3",				0x0000, 0x010000, CRC(ca2653d5) SHA1(30cd35627be8fb4fff2f0d61a6ab43cf3e4c1742), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0D, set 2)" )
M4ADDR_SET( 1994, m4addr5c_a,	m4addr,	"adi05___.5n3",				0x0000, 0x010000, CRC(13003560) SHA1(aabad24748f9b1b09f1820bf1af932160e64fe3e), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0C, set 2)" )
M4ADDR_SET( 1994, m4addr5yd_a,	m4addr,	"ad_05_b_.5n3",				0x0000, 0x010000, CRC(cdc8ca39) SHA1(33fdeef8ab8908f6908120aedf501ec3e9d7d23e), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 5.0YD, set 2)" )
// "(C)1993  B.W.B."  and "ADD 4.0"
M4ADDR_SET( 1993, m4addr4,		m4addr,	"ad_05___.4s3",				0x0000, 0x010000, CRC(6d1a3c51) SHA1(0e4b985173c7c3bd5804573d99913d66a05d54fb), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 4.0, set 1)" )
M4ADDR_SET( 1993, m4addr4c,		m4addr,	"adi05___.4s3",				0x0000, 0x010000, CRC(a4343a89) SHA1(cef67bbe03e6f535b530fc099f1b9a8bc7a2f864), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 4.0C, set 1)" )
M4ADDR_SET( 1993, m4addr4d,		m4addr,	"ad_05_d_.4s3",				0x0000, 0x010000, CRC(e672baf0) SHA1(bae2e2fe9f51b3b8da20fcefb145f6d35fa2d604), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 4.0D, set 1)" )
M4ADDR_SET( 1993, m4addr4yd,	m4addr,	"ad_05_b_.4s3",				0x0000, 0x010000, CRC(6bd6fdb6) SHA1(7ee1e80da5833b3eaf4b23035690a09379781584), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 4.0YD, set 1)" )
// "(C)1994  B.W.B."  and "ADD 4.0"
M4ADDR_SET( 1994, m4addr4_a,	m4addr,	"ad_10___.4a3",				0x0000, 0x010000, CRC(9151dac3) SHA1(bf1c065a62e84a8073f8f9854981bedad60805be), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 4.0, set 2)" )
M4ADDR_SET( 1994, m4addr4c_a,	m4addr,	"adi10___.4a3",				0x0000, 0x010000, CRC(2d2aa3cc) SHA1(21a7690c3fb7d158f4b4e6da63663778246ac902), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 4.0C, set 2)" )
M4ADDR_SET( 1994, m4addr4c_b,	m4addr,	"adi10___.4n3",				0x0000, 0x010000, CRC(af9aad00) SHA1(09729e73f27d9ac5d6ac7171191ed76aeaac3e3d), "Bwb","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 4.0C, set 3)" )
// "BIG DIPPER"  and ADD 1.0
M4ADDR_SET( 1994, m4addr_h1,	m4addr,	"5p4addersladders.bin",		0x0000, 0x010000, CRC(03fc43da) SHA1(cf2fdb0d1ad702331ba004fd39072484b05e2b97), "hack?","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0C, hack?, set 1)" )
M4ADDR_SET( 1994, m4addr_h2,	m4addr,	"ad05.6c",					0x0000, 0x010000, CRC(0940e4aa) SHA1(e8e7f7249a18386af990999a4c06f001db7003c5), "hack?","Adders & Ladders (Bwb / Barcrest) (MPU4) (ADD 1.0C, hack?, set 2)" )

