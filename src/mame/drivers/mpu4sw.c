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
INPUT_PORTS_EXTERN( mpu4jackpot8tkn );
extern DRIVER_INIT( m4default );
extern DRIVER_INIT( m4default_bigbank );
extern DRIVER_INIT( m_grtecp );

#define GAME_FLAGS (GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK)

static DRIVER_INIT( m4debug )
{
	// many original barcrest / bwb sets have identification info around here
	// this helps with sorting
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	int size = machine.root_device().memregion( "maincpu" )->bytes();
	
	for (int j=0;j<size;j+=0x10000)
	{
		if (size>0x10000) printf("\nblock 0x%06x:\n",j);
		printf("\ncopyright string:\n");
		for (int i = 0xffe0; i<0xfff0; i++)
		{
			printf("%c", src[j+i]);
		}
		printf("\n\nidentification string:\n");
		for (int i = 0xff28; i<0xff30; i++)
		{
			printf("%c", src[j+i]);
		}
		printf("\n");
	}
}

static DRIVER_INIT( m4_showstring )
{
	DRIVER_INIT_CALL( m4default );
	DRIVER_INIT_CALL( m4debug );
}

static DRIVER_INIT( m4_showstring_big )
{
	DRIVER_INIT_CALL( m4default_bigbank );
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
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring,ROT0,company,title,GAME_FLAGS ) \


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
GAME(199?, m4andycpdut,		m4andycp	,mod4oki	,mpu4				, mpu4_state,m4_showstring			,ROT0,   "Barcrest","Andy Capp (Barcrest) [DAC 1.3, Dutch] (MPU4)",			GAME_FLAGS|GAME_NO_SOUND )


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
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring,ROT0,company,title,GAME_FLAGS ) \

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
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring,ROT0,company,title,GAME_FLAGS ) \

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
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring,ROT0,company,title,GAME_FLAGS ) \

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
	GAME(year, setname, parent ,mod4oki	,grtecp , mpu4_state,m_grtecpss ,ROT0,company,title,GAME_FLAGS ) \

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
	GAME(year, setname, parent ,mod2	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \

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


#define M4DENMEN_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "densnd1.hex", 0x000000, 0x080000, CRC(468a8ec7) SHA1(ec450cd86fda09bc94caf913e9ee7900cfeaa0f2) ) \
	ROM_LOAD( "densnd2.hex", 0x080000, 0x080000, CRC(1c20a490) SHA1(62eddc469e4b93ea1f82070600fce628dc526f54) ) \

#define M4DENMEN_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( 0x10000, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4DENMEN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4DENMEN_SET( 199?, m4denmen,		0,			"dens.p1",						0x0000, 0x010000, CRC(d3687138) SHA1(611985a9116ea14992b34a84ed31693f88d99797), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DEN 1.2)" )
M4DENMEN_SET( 199?, m4denmendnd,	m4denmen,	"dend.p1",						0x0000, 0x010000, CRC(176cd283) SHA1(f72c69b346f926a6e11b685ab9a6a2783b836450), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DEN 1.2D)" )
M4DENMEN_SET( 199?, m4denmendnb,	m4denmen,	"denb.p1",						0x0000, 0x010000, CRC(b0164796) SHA1(61ff7e7ea2c27742177d851a4eb9a041d95b37d7), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DEN 1.2B)" )
M4DENMEN_SET( 199?, m4denmendnc,	m4denmen,	"denc.p1",						0x0000, 0x010000, CRC(549e17bc) SHA1(78271e11d4c8e742acce9087f194a1db8fc8c3eb), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DEN 1.2C)" )
M4DENMEN_SET( 199?, m4denmendnk,	m4denmen,	"denk.p1",						0x0000, 0x010000, CRC(8983cbe0) SHA1(159dcbc3f5d24b6be03ae9c3c2af58993bebd38c), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DEN 1.2K)" )
M4DENMEN_SET( 199?, m4denmendny,	m4denmen,	"deny.p1",						0x0000, 0x010000, CRC(83ebd9f6) SHA1(f59e9d34295df8200f85a51d725437954acf9bdc), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DEN 1.2Y)" )

M4DENMEN_SET( 199?, m4denmend5,		m4denmen,	"dm5s.p1",						0x0000, 0x010000, CRC(49672daa) SHA1(92e327b59b532e58b8c2a4e507f56c2ae069420c), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1))" )
M4DENMEN_SET( 199?, m4denmend5d,	m4denmen,	"dm5d.p1",						0x0000, 0x010000, CRC(0c6250d5) SHA1(56b316df56d6448137332044bfe1081401eef3e8), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1D)" )
M4DENMEN_SET( 199?, m4denmend5ad,	m4denmen,	"dm5ad.p1",						0x0000, 0x010000, CRC(f01125cc) SHA1(faa80bfb107db127b2f9c4c7d23ec495775d2162), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1AD)" )
M4DENMEN_SET( 199?, m4denmend5b,	m4denmen,	"dm5b.p1",						0x0000, 0x010000, CRC(2c6dae4c) SHA1(281e4ba31a60fb5600790f21095e697db80736b7), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1B)" )
M4DENMEN_SET( 199?, m4denmend5bd,	m4denmen,	"dm5bd.p1",						0x0000, 0x010000, CRC(a65c534d) SHA1(e5c38a9a06e20878cb820e5a12545405d699ff9d), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1BD)" )
M4DENMEN_SET( 199?, m4denmend5k,	m4denmen,	"dm5k.p1",						0x0000, 0x010000, CRC(581572d6) SHA1(ac7303ea828846e770f8f1c7c818369d4b006495), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1K)" )
M4DENMEN_SET( 199?, m4denmend5kd,	m4denmen,	"dm5dk.p1",						0x0000, 0x010000, CRC(848412a1) SHA1(bb385e2abdc2651b4a7ea9d30108dfa8adab0aea), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1KD)" )
M4DENMEN_SET( 199?, m4denmend5y,	m4denmen,	"dm5y.p1",						0x0000, 0x010000, CRC(e6b9a800) SHA1(543ef65352a98676d66f6a5d3d7f568e10aac084), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1Y)" )
M4DENMEN_SET( 199?, m4denmend5yd,	m4denmen,	"dm5dy.p1",						0x0000, 0x010000, CRC(0c091457) SHA1(930b87211b8df5846fa857744aafae2f2985e578), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM5 0.1YD)" )

M4DENMEN_SET( 199?, m4denmend8,		m4denmen,	"dm8s.p1",						0x0000, 0x010000, CRC(27484793) SHA1(872ad9bdbad793aa3bb4b8d227627f901a04d70e), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM8 0.1)" )
M4DENMEN_SET( 199?, m4denmend8d,	m4denmen,	"dm8d.p1",						0x0000, 0x010000, CRC(23258932) SHA1(03b929bd86c429a7806f75639569534bfe7634a8), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM8 0.1D)" )
M4DENMEN_SET( 199?, m4denmend8c,	m4denmen,	"dm8c.p1",						0x0000, 0x010000, CRC(f5bd6c61) SHA1(ec443a284dae480c944f437426c28481a61c8ebb), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM8 0.1C)" )
M4DENMEN_SET( 199?, m4denmend8k,	m4denmen,	"dm8k.p1",						0x0000, 0x010000, CRC(9b3c3827) SHA1(2f584cfbbf38435377785dd654fe7b97c78e731a), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM8 0.1K)" )
M4DENMEN_SET( 199?, m4denmend8y,	m4denmen,	"dm8y.p1",						0x0000, 0x010000, CRC(ebfcb926) SHA1(c6a623de9163e3f49ee7e5dbb8df867a90d0d0a9), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM8 0.1Y)" )
M4DENMEN_SET( 199?, m4denmend8yd,	m4denmen,	"dm8dy.p1",						0x0000, 0x010000, CRC(3c5ef7c8) SHA1(ac102525900f34c53082d37fb1bd14db9ce928fe), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DM8 0.1YD)" )

M4DENMEN_SET( 199?, m4denmendt,		m4denmen,	"dmts.p1",						0x0000, 0x010000, CRC(1a2776e3) SHA1(4d5029a5abafb3945d533ca5ca23b32c036fbb31), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1)" )
M4DENMEN_SET( 199?, m4denmendtd,	m4denmen,	"dmtd.p1",						0x0000, 0x010000, CRC(9b38fa46) SHA1(ce6509349c82a651336753a3062c1cf2390d0b9a), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1D)" )
M4DENMEN_SET( 199?, m4denmendtad,	m4denmen,	"dmtad.p1",						0x0000, 0x010000, CRC(2edab31e) SHA1(c1cb258aba42e6ae33df731504d23162118054be), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1AD)" )
M4DENMEN_SET( 199?, m4denmendtb,	m4denmen,	"dmtb.p1",						0x0000, 0x010000, CRC(c40fe8a4) SHA1(e182b0b1b975947da3b0a94afd17cdf166d7a8ac), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1B)" )
M4DENMEN_SET( 199?, m4denmendtbd,	m4denmen,	"dmtbd.p1",						0x0000, 0x010000, CRC(d9140665) SHA1(cba8fc1c285c9192a6ea80b3f0c958781a818489), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1BD)" )
M4DENMEN_SET( 199?, m4denmendtk,	m4denmen,	"dmtk.p1",						0x0000, 0x010000, CRC(b64b6b3f) SHA1(f39b2143b811375564ec82030a7d34057f79b3f7), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1K)" )
M4DENMEN_SET( 199?, m4denmendtkd,	m4denmen,	"dmtdk.p1",						0x0000, 0x010000, CRC(b6211765) SHA1(3a2c5b1ef27113221ce7b61562f06589bcfa9072), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1KD)" )
M4DENMEN_SET( 199?, m4denmendty,	m4denmen,	"dmty.p1",						0x0000, 0x010000, CRC(dbfa78a5) SHA1(edd9a1f286f3aa56a919e9e0c0013e9940d139ac), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1Y)" )
M4DENMEN_SET( 199?, m4denmendtyd,	m4denmen,	"dmtdy.p1",						0x0000, 0x010000, CRC(66064a45) SHA1(3f64212b85320fba66afd40c0bb0cd58a5a616bf), "Barcrest","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1YD)" )
// "(C)1993 BARCREST"  and "DMT 0.1" (but hack? similar to other DMT sets, but with extra code inserted in places etc. different chr check)
M4DENMEN_SET( 199?, m4denmen_h1,	m4denmen,	"dtm205",						0x0000, 0x010000, CRC(af76a460) SHA1(325021a92042c87e804bc17d6a7ccfda8bf865b8), "hack","Dennis The Menace (Barcrest) (MPU4) (DMT 0.1, hack?)" )
// "DAFFY   DUCK"  and "V1   0.1" (display message R.E.O instead of Barcrest)
M4DENMEN_SET( 199?, m4denmen_h2,	m4denmen,	"den20.10",						0x0000, 0x010000, CRC(e002932d) SHA1(0a9b31c138a79695e1c1c29eee40c5a741275da6), "hack","Dennis The Menace (Barcrest) (MPU4) (V1 0.1, hack, set 1)" )
M4DENMEN_SET( 199?, m4denmen_h3,	m4denmen,	"denm2010",						0x0000, 0x010000, CRC(dbed5e48) SHA1(f374f01aeefca7cc19fc46c93e2ca7a10606b183), "hack","Dennis The Menace (Barcrest) (MPU4) (V1 0.1, hack, set 2)" )


#define M4CRMAZE_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "crmsnd.p1", 0x000000, 0x080000, CRC(e05cdf96) SHA1(c85c7b31b775e3cc2d7f943eb02ff5ebae6c6080) ) \
	ROM_LOAD( "crmsnd.p2", 0x080000, 0x080000, CRC(11da0781) SHA1(cd63834bf5d5034c2473372bfcc4930c300333f7) ) \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) /* which sets are meant to use this? */ \
	ROM_LOAD( "cmazep1.bin", 0x000000, 0x080000, CRC(3d94a320) SHA1(a9b4e89ce36dbc2ef584b3adffffa00b7ae7e245) ) \

#define M4CRMAZE_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CRMAZE_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4jackpot8tkn , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \

// "(C)1993 BARCREST"  and "CRM 3.0"
M4CRMAZE_SET( 1993, m4crmaze,		0,			"crms.p1",				0x0000, 0x020000, CRC(b289c54b) SHA1(eb74bb559e2be2737fc311d044b9ce87014616f3), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0)" )
M4CRMAZE_SET( 1993, m4crmaze__h,	m4crmaze,	"crmd.p1",				0x0000, 0x020000, CRC(1232a809) SHA1(483b96b3b3ea50cbf5c3823c3ba20369b88bd459), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0D)" )
M4CRMAZE_SET( 1993, m4crmaze__d,	m4crmaze,	"crmad.p1",				0x0000, 0x020000, CRC(ed30e66e) SHA1(25c09637f6efaf8e24f758405fb55d6cfc7f4782), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0AD)" )
M4CRMAZE_SET( 1993, m4crmaze__e,	m4crmaze,	"crmb.p1",				0x0000, 0x020000, CRC(6f29a37f) SHA1(598541e2dbf05b3f2a70279276407cd93734731e), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0B)" )
M4CRMAZE_SET( 1993, m4crmaze__f,	m4crmaze,	"crmbd.p1",				0x0000, 0x020000, CRC(602a48ab) SHA1(3f1bf2b3294d15013e89d906865f065476202e54), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0BD)" )
M4CRMAZE_SET( 1993, m4crmaze__g,	m4crmaze,	"crmc.p1",				0x0000, 0x020000, CRC(58631e6d) SHA1(cffecd4c4ca46aa0ccfbaf7592d58da0428cf143), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0C)" )
M4CRMAZE_SET( 1993, m4crmaze__k,	m4crmaze,	"crmk.p1",				0x0000, 0x020000, CRC(25ee0b29) SHA1(addadf351a26e235a7fca573145a501aa6c0b53c), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0K)" )
M4CRMAZE_SET( 1993, m4crmaze__i,	m4crmaze,	"crmdk.p1",				0x0000, 0x020000, CRC(2aede0fd) SHA1(1731c901149c196d8f6a8bf3c2eec4f9a42126ad), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0KD)" )
M4CRMAZE_SET( 1993, m4crmaze__l,	m4crmaze,	"crmy.p1",				0x0000, 0x020000, CRC(a20d2bd7) SHA1(b05a0e2ab2b90a86873976c26a8299cb703fd6eb), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0Y)" )
M4CRMAZE_SET( 1993, m4crmaze__j,	m4crmaze,	"crmdy.p1",				0x0000, 0x020000, CRC(ad0ec003) SHA1(2d8a7467c3a79d60100f1290abe06410aaefaa49), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 3.0YD)" )
// "(C)1993 BARCREST"  and "CRM 2.3"
M4CRMAZE_SET( 1993, m4crmaze__c,	m4crmaze,	"cmaze8",				0x0000, 0x020000, CRC(f2f81306) SHA1(725bfbdc53cf66c08b440c2b8d45547aa426d9c7), "Barcrest","Crystal Maze (Barcrest) (MPU4) (CRM 2.3)" )
// no copyright string, and "CRM 3.0"
M4CRMAZE_SET( 1993, m4crmaze__m,	m4crmaze,	"crystalmaze15.bin",	0x0000, 0x020000, CRC(492440a4) SHA1(2d5fe812f1d815620f7e72333d44946b66f5c867), "hack?","Crystal Maze (Barcrest) (MPU4) (CRM 3.0, hack?)" ) // bad chr
// roms below are a smaller size, have they been hacked to not use banking, or are they bad, they're all Bwb versions but all have the Barcrest at the end blanked out rather than replaced
// no copyright string, and "CRC 0.7"
M4CRMAZE_SET( 199?, m4crmaze__n,	m4crmaze,	"cmaz5.10",				0x0000, 0x010000, CRC(13a64c64) SHA1(3a7c4173f99fdf1a4b5d5b627022b18eb66837ce), "Bwb / hack?","Crystal Maze (Bwb / Barcrest) (MPU4) (CRC 0.7, hack?)" ) // bad chr
// no copyright string, and "CRC 1.3"
M4CRMAZE_SET( 199?, m4crmaze__p,	m4crmaze,	"cmaz510",				0x0000, 0x010000, CRC(0a1d39ac) SHA1(37888bbea427e115c29253deb85ed851ff6bdfd4), "Bwb / hack?","Crystal Maze (Bwb / Barcrest) (MPU4) (CRC 1.3, hack?)" ) // bad chr
// no copyright string, and "CR5 1.0"
M4CRMAZE_SET( 199?, m4crmaze__o,	m4crmaze,	"cmaz5.5",				0x0000, 0x010000, CRC(1f110757) SHA1(a60bac78176dab70d68bfb2b6a44debf499c96e3), "Bwb / hack?","Crystal Maze (Bwb / Barcrest) (MPU4) (CR5 1.0, hack?)" ) // bad chr
// no copyright string, and "CR5 2.0"
M4CRMAZE_SET( 199?, m4crmaze__q,	m4crmaze,	"cmaz55",				0x0000, 0x010000, CRC(2c2540ce) SHA1(12163109e05fe8675bc2dbcad95f598bebec8ba3), "Bwb / hack?","Crystal Maze (Bwb / Barcrest) (MPU4) (CR5 2.0, hack?, set 1)" ) // bad chr
M4CRMAZE_SET( 199?, m4crmaze__r,	m4crmaze,	"cmaz55v2",				0x0000, 0x010000, CRC(9a3515d6) SHA1(5edd2c67152d353a48ad2f28b685fae1e1e7fff7), "Bwb / hack?","Crystal Maze (Bwb / Barcrest) (MPU4) (CR5 2.0, hack?, set 2)" ) // bad chr
// no copyright string, and "CR8 1.2"
M4CRMAZE_SET( 199?, m4crmaze__s,	m4crmaze,	"cmaz58t",				0x0000, 0x010000, CRC(81a2c48a) SHA1(3ea25a2863f1350054f41cb169282c592565dbcd), "Bwb / hack?","Crystal Maze (Bwb / Barcrest) (MPU4) (CR8 1.2, hack?)" ) // bad chr


// these were in the Crystal Maze set, but are Cash Machine

#define M4CASHMN_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "cmasnd.p1", 0x000000, 0x080000, CRC(1e7e13b8) SHA1(2db5c3789ad1b9bdb59e058562bd8be181ba0259) ) \
	ROM_LOAD( "cmasnd.p2", 0x080000, 0x080000, CRC(cce703a8) SHA1(97487f3df0724d3ee01f6f4deae126aec6d2dd68) ) \

#define M4CASHMN_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CASHMN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4jackpot8tkn , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


ROM_START( m4cashmn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cma07s.p1", 0x0000, 0x020000, CRC(e9c1d9f2) SHA1(f2df4ae650ec2b62d15bbaa562d638476bf926e7) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "camc2010", 0x0000, 0x020000, CRC(82e459ab) SHA1(62e1906007f6bba99e3e8badc3472070e8ae84f8) )
	ROM_LOAD( "cma07ad.p1", 0x0000, 0x020000, CRC(411889fd) SHA1(5855b584315867ecc5df6d37f4a664b8331ecde8) )
	ROM_LOAD( "cma07b.p1", 0x0000, 0x020000, CRC(ab889a33) SHA1(0f3ed0e4b8131585bcb4af47674fb1b65c37503d) )
	ROM_LOAD( "cma07bd.p1", 0x0000, 0x020000, CRC(cc022738) SHA1(5968d1b6db55008cbd3c83651214c61c28fd4c5c) )
	ROM_LOAD( "cma07c.p1", 0x0000, 0x020000, CRC(9cc22721) SHA1(ee4e9860641c8bf7db024a5bf9469265a6383e0a) )
	ROM_LOAD( "cma07d.p1", 0x0000, 0x020000, CRC(d6939145) SHA1(45b6f7f80c7a2f4377d9bf8e184fb791f4ed0a2d) )
	ROM_LOAD( "cma07dk.p1", 0x0000, 0x020000, CRC(86c58f6e) SHA1(fce50f86a641d27d0f5e5ecbac84822ccc9c177b) )
	ROM_LOAD( "cma07dr.p1", 0x0000, 0x020000, CRC(35ca345f) SHA1(ddbb926988028bef13ebaa949d3ee92599770003) )
	ROM_LOAD( "cma07dy.p1", 0x0000, 0x020000, CRC(0126af90) SHA1(0f303451fd8ca8c0cc50a31297f0d2729cfc2d7b) )
	ROM_LOAD( "cma07k.p1", 0x0000, 0x020000, CRC(e14f3265) SHA1(7b5dc581fe8679559356fdca9644985da7d299cb) )
	ROM_LOAD( "cma07r.p1", 0x0000, 0x020000, CRC(52408954) SHA1(623f840d94cc3cf2d2d648eb2be644d48350b169) )
	ROM_LOAD( "cma07y.p1", 0x0000, 0x020000, CRC(66ac129b) SHA1(97f8c0c1f46444d4a492bc3dd3689df038000640) )
	ROM_LOAD( "cma08ad.p1", 0x0000, 0x020000, CRC(fce2f785) SHA1(fc508e3d1036319894985600cb0142f13536078c) )
	ROM_LOAD( "cma08b.p1", 0x0000, 0x020000, CRC(df7526de) SHA1(71456496fc31ae11ffa7c543b6444adba735aeb9) )
	ROM_LOAD( "cma08bd.p1", 0x0000, 0x020000, CRC(71f85940) SHA1(439c54f35f4f6161a683d2c3d2bb6ce81b4190bf) )
	ROM_LOAD( "cma08c.p1", 0x0000, 0x020000, CRC(e83f9bcc) SHA1(e20297ba5238b59c3872776b01e6a89a51a7aea7) )
	ROM_LOAD( "cma08d.p1", 0x0000, 0x020000, CRC(a26e2da8) SHA1(928dfe399a7ae278dadd1e930bd370022f5113c4) )
	ROM_LOAD( "cma08dk.p1", 0x0000, 0x020000, CRC(3b3ff116) SHA1(f60f0f9d996398a0f1c5b7d2a411613c42149e65) )
	ROM_LOAD( "cma08dr.p1", 0x0000, 0x020000, CRC(88304a27) SHA1(9b86a49edca078dd68abab4c3e8655d3b4e79d47) )
	ROM_LOAD( "cma08dy.p1", 0x0000, 0x020000, CRC(bcdcd1e8) SHA1(a7a4ab2313198c3bc0536526bd83179fd9170e66) )
	ROM_LOAD( "cma08k.p1", 0x0000, 0x020000, CRC(95b28e88) SHA1(282a782900a0ddf60c66aa6a69e6871bb42c647a) )
	ROM_LOAD( "cma08r.p1", 0x0000, 0x020000, CRC(26bd35b9) SHA1(74d07da26932bf48fe4b79b39ff76956b0993f3b) )
	ROM_LOAD( "cma08s.p1", 0x0000, 0x020000, CRC(d0154d3c) SHA1(773f211092c51fb4ca1ef6a5a0cbdb15f842aca8) )
	ROM_LOAD( "cma08y.p1", 0x0000, 0x020000, CRC(1251ae76) SHA1(600ce195be615796b887bb56bebb6c4322709632) )
	ROM_LOAD( "cmh06ad.p1", 0x0000, 0x020000, CRC(ea2f6866) SHA1(afae312a488d7d83576c17eb2627a84637d88f18) )
	ROM_LOAD( "cmh06b.p1", 0x0000, 0x020000, CRC(2d4d9667) SHA1(896ed70962c8904646df7159c3717399d0ceb022) )
	ROM_LOAD( "cmh06bd.p1", 0x0000, 0x020000, CRC(6735c6a3) SHA1(4bce480c57473a9b0787a87a462c76e146a10157) )
	ROM_LOAD( "cmh06c.p1", 0x0000, 0x020000, CRC(1a072b75) SHA1(89d4aed011391b2f12b48c0344136d83175ff2f0) )
	ROM_LOAD( "cmh06d.p1", 0x0000, 0x020000, CRC(50569d11) SHA1(bdf7e984766bbe90bafbf0b367690ca65a8612d2) )
	ROM_LOAD( "cmh06dk.p1", 0x0000, 0x020000, CRC(2df26ef5) SHA1(c716b73396d0af1f69f5812bace06341d368859f) )
	ROM_LOAD( "cmh06dr.p1", 0x0000, 0x020000, CRC(9efdd5c4) SHA1(b9e02fe91e766aff41ca19879ab29e53bdee537e) )
	ROM_LOAD( "cmh06dy.p1", 0x0000, 0x020000, CRC(aa114e0b) SHA1(8bc9b94e488a98b8a8008f9a35b6c078cc5c8f3f) )
	ROM_LOAD( "cmh06k.p1", 0x0000, 0x020000, CRC(678a3e31) SHA1(2351b5167eec2a0d23c9938014de6f6ee07f13ff) )
	ROM_LOAD( "cmh06r.p1", 0x0000, 0x020000, CRC(d4858500) SHA1(489fd55ac6c93b94bfb9297fd71b5d74bf95a97f) )
	ROM_LOAD( "cmh06s.p1", 0x0000, 0x020000, CRC(9d3b4260) SHA1(7c4740585d17be3da3a0ea6e7fc68f89538013fb) )
	ROM_LOAD( "cmh06y.p1", 0x0000, 0x020000, CRC(e0691ecf) SHA1(978fa00736967dd09d48ce5c847698b39a058ab5) )
	ROM_LOAD( "cmh07ad.p1", 0x0000, 0x020000, CRC(4f354391) SHA1(687eccc312cd69f8bb70e35837f0b7ce74392936) )
	ROM_LOAD( "cmh07b.p1", 0x0000, 0x020000, CRC(27fb6e7b) SHA1(c1558e4a0e2c28a825c2c5bb4089143cf919b67c) )
	ROM_LOAD( "cmh07bd.p1", 0x0000, 0x020000, CRC(c22fed54) SHA1(5b6df1ed8518f9ba3e02b17c189c01ad1d0acbbb) )
	ROM_LOAD( "cmh07c.p1", 0x0000, 0x020000, CRC(10b1d369) SHA1(9933a2a7933df941ee93e16682e91dcc90abb627) )
	ROM_LOAD( "cmh07d.p1", 0x0000, 0x020000, CRC(5ae0650d) SHA1(da6917aa186daf59f35124c7cdc9d039d365c4c2) )
	ROM_LOAD( "cmh07dk.p1", 0x0000, 0x020000, CRC(88e84502) SHA1(2ab86be51b3dde0b2cb05e3af5f43aad3d8a76df) )
	ROM_LOAD( "cmh07dr.p1", 0x0000, 0x020000, CRC(3be7fe33) SHA1(074243cdfd37ba36e18e00610f45473e46ddc728) )
	ROM_LOAD( "cmh07dy.p1", 0x0000, 0x020000, CRC(0f0b65fc) SHA1(68d775bb4af9595ac87c33c2663b272640eea69e) )
	ROM_LOAD( "cmh07k.p1", 0x0000, 0x020000, CRC(6d3cc62d) SHA1(85f76fd8513c20683d486de7a1509cadfb6ecaa9) )
	ROM_LOAD( "cmh07r.p1", 0x0000, 0x020000, CRC(de337d1c) SHA1(dd07727fb183833eced5c0c2dc284d571baacd25) )
	ROM_LOAD( "cmh07s.p1", 0x0000, 0x020000, CRC(0367f4cf) SHA1(8b24a9009ff17d517b34e078ebbdc17465df139d) )
	ROM_LOAD( "cmh07y.p1", 0x0000, 0x020000, CRC(eadfe6d3) SHA1(80541aba612b8ebba7ab159c61e6492b9c06feda) )

	M4CASHMN_EXTRA_ROMS
ROM_END

GAME(199?, m4cashmn	,0			,mod4oki	,mpu4jackpot8tkn	, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Cash Machine (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

// "(C)1993 BARCREST"  and "CMH 0.6"
M4CASHMN_SET( 199?, m4cashmn__za,	m4cashmn,	"cma15g",				0x0000, 0x020000, CRC(f30b3ef2) SHA1(c8fb4d883d12a477a703d8cb0842994675aaf879), "Barcrest","Cash Machine (Barcrest) (MPU4) (CMH 0.6Y)" )
// no copyright string, and "CMA 0.7"
M4CASHMN_SET( 199?, m4cashmn__zb,	m4cashmn,	"cma15t",				0x0000, 0x020000, CRC(a4ed66a4) SHA1(0e98859c4d7dbccdea9396c3fea9f345b2f08db6), "Barcrest","Cash Machine (Barcrest) (MPU4) (CMA 0.7C, hack?)" )



#define M4TOPTEN_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "tops1.hex", 0x000000, 0x080000, CRC(70f16892) SHA1(e6448831d3ce7fa251b40023bc7d5d6dee9d6793) ) \
	ROM_LOAD( "tops2.hex", 0x080000, 0x080000, CRC(5fc888b0) SHA1(8d50ee4f36bd36aed5d0e7a77f76bd6caffc6376) ) \

#define M4TOPTEN_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TOPTEN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \



M4TOPTEN_SET( 199?, m4topten,		0,			"tts04s.p1",	0x0000, 0x020000, CRC(5e53f04f) SHA1(d49377966ed787cc3571eadff8c4c16fac74434c), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4)" )
M4TOPTEN_SET( 199?, m4topten__ak,	m4topten,	"tts04ad.p1",	0x0000, 0x020000, CRC(cdcc3d18) SHA1(4e9ccb8bfbe5b86731a24631cc60819919bb3ce8), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4AD)" )
M4TOPTEN_SET( 199?, m4topten__al,	m4topten,	"tts04b.p1",	0x0000, 0x020000, CRC(d0280881) SHA1(c2e416a224a7ed4cd9010a8e10b0aa5e808fbbb9), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4B)" )
M4TOPTEN_SET( 199?, m4topten__am,	m4topten,	"tts04bd.p1",	0x0000, 0x020000, CRC(40d693dd) SHA1(fecbf86d6b533dd0721497cc689ab978c75d67e5), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4BD)" )
M4TOPTEN_SET( 199?, m4topten__an,	m4topten,	"tts04c.p1",	0x0000, 0x020000, CRC(e762b593) SHA1(7bcd65b747d12801430e783ead01c746fee3f371), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4C)" )
M4TOPTEN_SET( 199?, m4topten__ao,	m4topten,	"tts04d.p1",	0x0000, 0x020000, CRC(ad3303f7) SHA1(5df231e7d20bf21da56ce912b736fc570707a10f), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4D)" )
M4TOPTEN_SET( 199?, m4topten__ap,	m4topten,	"tts04dh.p1",	0x0000, 0x020000, CRC(8e3ac39e) SHA1(27ed795953247075089c1df8e577aa61ae64f59e), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4DH)" )
M4TOPTEN_SET( 199?, m4topten__aq,	m4topten,	"tts04dk.p1",	0x0000, 0x020000, CRC(0a113b8b) SHA1(1282ad75537040fa84620f0871050762546b5a28), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4KD)" )
M4TOPTEN_SET( 199?, m4topten__ar,	m4topten,	"tts04dr.p1",	0x0000, 0x020000, CRC(b91e80ba) SHA1(b7e082ea29d8558967564d057dfd4a48d0a997cc), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4RD)" )
M4TOPTEN_SET( 199?, m4topten__as,	m4topten,	"tts04dy.p1",	0x0000, 0x020000, CRC(8df21b75) SHA1(2de6bc76bae324e149fb9003eb8327f4a2db269b), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4YD)" )
M4TOPTEN_SET( 199?, m4topten__at,	m4topten,	"tts04h.p1",	0x0000, 0x020000, CRC(1ec458c2) SHA1(49a8e39a6506c8c5af3ad9eac47871d828611338), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4H)" )
M4TOPTEN_SET( 199?, m4topten__au,	m4topten,	"tts04k.p1",	0x0000, 0x020000, CRC(9aefa0d7) SHA1(f90be825c58ac6e443822f7c8f5da74dcf18c652), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4K)" )
M4TOPTEN_SET( 199?, m4topten__av,	m4topten,	"tts04r.p1",	0x0000, 0x020000, CRC(29e01be6) SHA1(59ee4baf1f48dbd703e94c7a8e45d841f196ec54), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4R)" )
M4TOPTEN_SET( 199?, m4topten__aw,	m4topten,	"tts04y.p1",	0x0000, 0x020000, CRC(1d0c8029) SHA1(9ddc7a3d92715bfd4b24470f3d5ba2d9047be967), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.4Y)" )

M4TOPTEN_SET( 199?, m4topten__6,	m4topten,	"tts02ad.p1",	0x0000, 0x020000, CRC(afba21a4) SHA1(6394014f5d46df96d6c7cd840fec996a6d5ffee5), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2AD)" )
M4TOPTEN_SET( 199?, m4topten__7,	m4topten,	"tts02b.p1",	0x0000, 0x020000, CRC(ef4e080d) SHA1(a82940e58537d0c40f97c43aec470d68e9b344e8), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2B)" )
M4TOPTEN_SET( 199?, m4topten__8,	m4topten,	"tts02bd.p1",	0x0000, 0x020000, CRC(22a08f61) SHA1(5a28d4f3cf89368a1cfa0cf5df1a9050f27f7e05), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2BD)" )
M4TOPTEN_SET( 199?, m4topten__9,	m4topten,	"tts02c.p1",	0x0000, 0x020000, CRC(d804b51f) SHA1(b6f18c7855f11978c408de3ec799859d8a534c93), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2C)" )
M4TOPTEN_SET( 199?, m4topten__aa,	m4topten,	"tts02d.p1",	0x0000, 0x020000, CRC(9255037b) SHA1(f470f5d089a598bc5da6329caa38f87974bd2984), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2D)" )
M4TOPTEN_SET( 199?, m4topten__ab,	m4topten,	"tts02dh.p1",	0x0000, 0x020000, CRC(ec4cdf22) SHA1(d3af8a72ce2740461c4528328b04084924e12832), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2DH)" )
M4TOPTEN_SET( 199?, m4topten__ac,	m4topten,	"tts02dk.p1",	0x0000, 0x020000, CRC(68672737) SHA1(79e5d15a62a6fcb90c3949e3676276b49167e353), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2KD)" )
M4TOPTEN_SET( 199?, m4topten__ad,	m4topten,	"tts02dr.p1",	0x0000, 0x020000, CRC(db689c06) SHA1(6c333c1fce6ccd0a34f11c11e3b826488e3ea663), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2RD)" )
M4TOPTEN_SET( 199?, m4topten__ae,	m4topten,	"tts02dy.p1",	0x0000, 0x020000, CRC(ef8407c9) SHA1(ecd3704d995d797d2c4a8c3aa729850ae4ccde56), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2YD)" )
M4TOPTEN_SET( 199?, m4topten__af,	m4topten,	"tts02h.p1",	0x0000, 0x020000, CRC(21a2584e) SHA1(b098abf763af86ac691ccd1dcc3e0a4d92b0073d), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2H)" )
M4TOPTEN_SET( 199?, m4topten__ag,	m4topten,	"tts02k.p1",	0x0000, 0x020000, CRC(a589a05b) SHA1(30e1b3a7baddd7a69be7a9a01ee4a84937eaedbd), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2K)" )
M4TOPTEN_SET( 199?, m4topten__ah,	m4topten,	"tts02r.p1",	0x0000, 0x020000, CRC(16861b6a) SHA1(8275099a3abfa388e9568be5465abe0e31db320d), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2R)" )
M4TOPTEN_SET( 199?, m4topten__ai,	m4topten,	"tts02s.p1",	0x0000, 0x020000, CRC(3cd87ce5) SHA1(b6214953e5b9f655b413b008d61624acbc39d419), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2)" )
M4TOPTEN_SET( 199?, m4topten__aj,	m4topten,	"tts02y.p1",	0x0000, 0x020000, CRC(226a80a5) SHA1(23b25dba28af1225a75e6f7c428c8576df4e8cb9), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2Y)" )

M4TOPTEN_SET( 199?, m4topten__e,	m4topten,	"tth10ad.p1",	0x0000, 0x020000, CRC(cd15be85) SHA1(fc2aeabbb8524a7225322c17c4f1a32d2a17bcd0), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0AD)" )
M4TOPTEN_SET( 199?, m4topten__f,	m4topten,	"tth10b.p1",	0x0000, 0x020000, CRC(52d17d33) SHA1(0da24e685b1a7c8ee8899a17a87a55c3d7916608), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0B)" )
M4TOPTEN_SET( 199?, m4topten__g,	m4topten,	"tth10bd.p1",	0x0000, 0x020000, CRC(400f1040) SHA1(4485791bb962d8f13bcc87a10dccf1fd096f31d5), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0BD)" )
M4TOPTEN_SET( 199?, m4topten__h,	m4topten,	"tth10c.p1",	0x0000, 0x020000, CRC(659bc021) SHA1(12e99ef185ac75ecab0604659085fee344e2d798), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0C)" )
M4TOPTEN_SET( 199?, m4topten__i,	m4topten,	"tth10d.p1",	0x0000, 0x020000, CRC(2fca7645) SHA1(84237e9d72180f39f273540e1df711f9cb282afd), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0D)" )
M4TOPTEN_SET( 199?, m4topten__j,	m4topten,	"tth10dh.p1",	0x0000, 0x020000, CRC(29cf8d8c) SHA1(b5fe04e5dd1417cda8665d2593809ae1b49bf4be), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0DH)" )
M4TOPTEN_SET( 199?, m4topten__k,	m4topten,	"tth10dk.p1",	0x0000, 0x020000, CRC(0ac8b816) SHA1(d8b7d4e467b48b4998cbc7bf02a7adb8307d1594), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0KD)" )
M4TOPTEN_SET( 199?, m4topten__l,	m4topten,	"tth10dr.p1",	0x0000, 0x020000, CRC(b9c70327) SHA1(5c4ba29b11f317677bea0b588975e1ce8f9c66f2), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0RD)" )
M4TOPTEN_SET( 199?, m4topten__m,	m4topten,	"tth10dy.p1",	0x0000, 0x020000, CRC(8d2b98e8) SHA1(3dc25a4d90407b3b38a1717d4a67d5ef5f815417), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0YD)" )
M4TOPTEN_SET( 199?, m4topten__n,	m4topten,	"tth10h.p1",	0x0000, 0x020000, CRC(3b11e0ff) SHA1(9aba4f17235876aeab71fcc46a4f15e58c87aa42), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0H)" )
M4TOPTEN_SET( 199?, m4topten__o,	m4topten,	"tth10k.p1",	0x0000, 0x020000, CRC(1816d565) SHA1(78d65f8cd5dfba8bfca732295fab12d408a4a8a0), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0K)" )
M4TOPTEN_SET( 199?, m4topten__p,	m4topten,	"tth10r.p1",	0x0000, 0x020000, CRC(ab196e54) SHA1(d55b8cb3acd961e4fd8ace7590be7ce0ae5babfc), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0R)" )
M4TOPTEN_SET( 199?, m4topten__q,	m4topten,	"tth10s.p1",	0x0000, 0x020000, CRC(046f5357) SHA1(ddcf7ff7d113b2bbf2106095c9166a678b00ad06), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0)" )
M4TOPTEN_SET( 199?, m4topten__r,	m4topten,	"tth10y.p1",	0x0000, 0x020000, CRC(9ff5f59b) SHA1(a51f5f9bc90bfe89efd2cb39f32a626831c22056), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0Y)" )

M4TOPTEN_SET( 199?, m4topten__ax,	m4topten,	"tth11s.p1",	0x0000, 0x020000, CRC(c46b2866) SHA1(26d9ee1f25e6a0f708a48ce91c7e9ed9ad3bee7a), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.1)" )

M4TOPTEN_SET( 199?, m4topten__s,	m4topten,	"tth12ad.p1",	0x0000, 0x020000, CRC(08e54740) SHA1(c86e36eb16d6031017a9a309ae0ae627c855b75a), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2AD)" )
M4TOPTEN_SET( 199?, m4topten__t,	m4topten,	"tth12b.p1",	0x0000, 0x020000, CRC(dc787847) SHA1(0729350c65f4363b04aedbae214aca9f54b22b36), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2B)" )
M4TOPTEN_SET( 199?, m4topten__u,	m4topten,	"tth12bd.p1",	0x0000, 0x020000, CRC(85ffe985) SHA1(bf0eba40b0f74f77b7625fbbe6fa0382f089fd9d), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2BD)" )
M4TOPTEN_SET( 199?, m4topten__v,	m4topten,	"tth12c.p1",	0x0000, 0x020000, CRC(eb32c555) SHA1(aa5c36d1c6d3f5f198d38705742a0a6a44b745bb), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2C)" )
M4TOPTEN_SET( 199?, m4topten__w,	m4topten,	"tth12d.p1",	0x0000, 0x020000, CRC(a1637331) SHA1(b5787e5f099375db689f8815493d1b6c9de5ee1e), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2D)" )
M4TOPTEN_SET( 199?, m4topten__x,	m4topten,	"tth12dh.p1",	0x0000, 0x020000, CRC(ec3f7449) SHA1(1db5e83734342c4c431614001fe06a8d8632242b), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2DH)" )
M4TOPTEN_SET( 199?, m4topten__y,	m4topten,	"tth12dk.p1",	0x0000, 0x020000, CRC(cf3841d3) SHA1(e2629b9f06c39b2c7e1b321ab262e61d64c706b1), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2KD)" )
M4TOPTEN_SET( 199?, m4topten__z,	m4topten,	"tth12dr.p1",	0x0000, 0x020000, CRC(7c37fae2) SHA1(e16eb1297ec725a879c3339d17ae2d8029646375), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2RD)" )
M4TOPTEN_SET( 199?, m4topten__0,	m4topten,	"tth12dy.p1",	0x0000, 0x020000, CRC(48db612d) SHA1(6ab67832ad61c4b0192b8e3a282238981a730aba), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2YD)" )
M4TOPTEN_SET( 199?, m4topten__1,	m4topten,	"tth12h.p1",	0x0000, 0x020000, CRC(b5b8e58b) SHA1(d1dd2ce68c657089267f78622ab5ecc34d7c306e), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2H)" )
M4TOPTEN_SET( 199?, m4topten__2,	m4topten,	"tth12k.p1",	0x0000, 0x020000, CRC(96bfd011) SHA1(38a1473d61eaec3d9d2bbb6486a8cb1d81d03b9f), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2K)" )
M4TOPTEN_SET( 199?, m4topten__3,	m4topten,	"tth12r.p1",	0x0000, 0x020000, CRC(25b06b20) SHA1(21b1566424fd68ab90a0ba11cb2b61fc6131256c), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2R)" )
M4TOPTEN_SET( 199?, m4topten__4,	m4topten,	"tth12s.p1",	0x0000, 0x020000, CRC(d204097c) SHA1(d06c0e1c8d3da373772723c580977aefdd7224b3), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2)" )
M4TOPTEN_SET( 199?, m4topten__5,	m4topten,	"tth12y.p1",	0x0000, 0x020000, CRC(115cf0ef) SHA1(b13122a69d16200a587c8cb6328fe7cd89897261), "Barcrest","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.2Y)" )
// no copyright string
M4TOPTEN_SET( 199?, m4topten__a,	m4topten,	"topt15g",		0x0000, 0x020000, CRC(4bd34f0b) SHA1(e513582ec0579e2d030a82735284ca62ee8fedf9), "hack?","Top Tenner (Barcrest) (type 1) (MPU4) (TTH 1.0, hack?)" )
M4TOPTEN_SET( 199?, m4topten__b,	m4topten,	"topt15t",		0x0000, 0x020000, CRC(5c0f6549) SHA1(c20c0beccf23d49633127da37536f81186861c28), "hack?","Top Tenner (Barcrest) (type 1) (MPU4) (TTS 0.2, hack?)" )


#define M4TOOT_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tenoutten.chr", 0x0000, 0x000048, CRC(f94b32a4) SHA1(7d7fdf7612dab684b549c6fc99f11185056b8c3e) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "tocsnd.p1", 0x000000, 0x080000, CRC(b9527b0e) SHA1(4dc5f6794c3e63c8faced34e166dcc748ffb4941) ) \
	ROM_LOAD( "tocsnd.p2", 0x080000, 0x080000, CRC(f684a488) SHA1(7c93cda3d3b55d9818625f696798c7c2cde79fa8) ) \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) \
	ROM_LOAD( "totsnd.p1", 0x0000, 0x080000, CRC(684e9eb1) SHA1(8af28de879ae41efa07dfb07ecbd6c72201749a7) ) \

#define M4TOOT_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TOOT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


ROM_START( m4toot )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "toc03c.p1", 0x0000, 0x020000, CRC(752ffa3f) SHA1(6cbe521ff85173159b6d34cc3e29a4192cd66394) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "toc03ad.p1", 0x0000, 0x020000, CRC(f67e53c1) SHA1(07a50fb649c5085a33f0a1a9b3d65b0b61a3f152) )
	ROM_LOAD( "toc03b.p1", 0x0000, 0x020000, CRC(4265472d) SHA1(01d5eb4e0a30abd1efed45658dcd8455494aabc4) )
	ROM_LOAD( "toc03bd.p1", 0x0000, 0x020000, CRC(7b64fd04) SHA1(377af32317d8356f06b19de553a13e8558993c34) )
	ROM_LOAD( "toc03d.p1", 0x0000, 0x020000, CRC(61f6b566) SHA1(2abd8092fe387c474ed10885a23ac242fa1462fa) )
	ROM_LOAD( "toc03dk.p1", 0x0000, 0x020000, CRC(31a35552) SHA1(3765fe6209d7b33afa1805ba56376e83e825165f) )
	ROM_LOAD( "toc03dr.p1", 0x0000, 0x020000, CRC(82acee63) SHA1(5a95425d6175c6496d745011d0ca3d744a027579) )
	ROM_LOAD( "toc03dy.p1", 0x0000, 0x020000, CRC(b64075ac) SHA1(ffdf8e45c2eab593570e15efd5161b67de5e4ecf) )
	ROM_LOAD( "toc03k.p1", 0x0000, 0x020000, CRC(08a2ef7b) SHA1(a4181db6280c7cc37b54baaf9cce1e61f61f3274) )
	ROM_LOAD( "toc03r.p1", 0x0000, 0x020000, CRC(bbad544a) SHA1(5fb31e5641a9e85147f5b61c5aba5a1ee7470f9c) )
	ROM_LOAD( "toc03s.p1", 0x0000, 0x020000, CRC(30feff92) SHA1(14397768ebd7469b4d1cff22ca9727f63608a98a) )
	ROM_LOAD( "toc03y.p1", 0x0000, 0x020000, CRC(8f41cf85) SHA1(315b359d6d1a9f6ad939be1fc5e4d8f21f998fb8) )
	ROM_LOAD( "toc04ad.p1", 0x0000, 0x020000, CRC(59075e2e) SHA1(a3ad5c642fb9cebcce2fb6c1e65514f2414948e0) )
	ROM_LOAD( "toc04b.p1", 0x0000, 0x020000, CRC(b2d54721) SHA1(4c72d434c0f4f37b9a3f08a760c7fe3851717059) )
	ROM_LOAD( "toc04bd.p1", 0x0000, 0x020000, CRC(d41df0eb) SHA1(e5a04c728a2893073ff8b5f6efd7cffd433a2985) )
	ROM_LOAD( "toc04c.p1", 0x0000, 0x020000, CRC(859ffa33) SHA1(05b7bd3b87a0ebcc78de751766cfcdc4276035ac) )
	ROM_LOAD( "toc04d.p1", 0x0000, 0x020000, CRC(9146b56a) SHA1(04bcd265d83e3554aef2de05aab9c3869bb966ea) )
	ROM_LOAD( "toc04dk.p1", 0x0000, 0x020000, CRC(9eda58bd) SHA1(5e38f87a162d1cb37e74850af6a00ae81619ecbe) )
	ROM_LOAD( "toc04dr.p1", 0x0000, 0x020000, CRC(2dd5e38c) SHA1(a1b0e8d48e164ab91b277a7efedf5b9fc73fc266) )
	ROM_LOAD( "toc04dy.p1", 0x0000, 0x020000, CRC(19397843) SHA1(e7e3299d8e46c79d3cd0ea7fd639a1d649a806df) )
	ROM_LOAD( "toc04k.p1", 0x0000, 0x020000, CRC(f812ef77) SHA1(d465b771efed27a9f616052d3fcabdbeb7c2d151) )
	ROM_LOAD( "toc04r.p1", 0x0000, 0x020000, CRC(4b1d5446) SHA1(f4bac0c8257add41295679b3541d2064d8c772c2) )
	ROM_LOAD( "toc04s.p1", 0x0000, 0x020000, CRC(295e6fff) SHA1(a21d991f00f144e12de60b891e3e2e5dd7d08d71) )
	ROM_LOAD( "toc04y.p1", 0x0000, 0x020000, CRC(7ff1cf89) SHA1(d4ab56b2b5b05643cd077b8d596b6cddf8a25134) )
	ROM_LOAD( "tot05ad.p1", 0x0000, 0x020000, CRC(fce00fcc) SHA1(7d10c0b83d782a9e603522ed039089866d931474) )
	ROM_LOAD( "tot05b.p1", 0x0000, 0x020000, CRC(594d551c) SHA1(3fcec5f41cfbea497aa53af4570664265774d1aa) )
	ROM_LOAD( "tot05bd.p1", 0x0000, 0x020000, CRC(71faa109) SHA1(8bec4a03e2dc43656c910652cf10d1afdf0bab33) )
	ROM_LOAD( "tot05c.p1", 0x0000, 0x020000, CRC(6e07e80e) SHA1(087a51da5578c326d7d9716c61b454be5e091761) )
	ROM_LOAD( "tot05d.p1", 0x0000, 0x020000, CRC(24565e6a) SHA1(0f2c19e54e5a78ae10e786d49ebcd7c16e41850c) )
	ROM_LOAD( "tot05dk.p1", 0x0000, 0x020000, CRC(3b3d095f) SHA1(326574adaa285a479abb5ae7515ef7d6bdd64126) )
	ROM_LOAD( "tot05dr.p1", 0x0000, 0x020000, CRC(8832b26e) SHA1(d5414560245bbb2c070f7a035e5e3416617c2cf3) )
	ROM_LOAD( "tot05dy.p1", 0x0000, 0x020000, CRC(bcde29a1) SHA1(cf8af40faf81a2a3141d3addd6b28c917beeda49) )
	ROM_LOAD( "tot05k.p1", 0x0000, 0x020000, CRC(138afd4a) SHA1(bc53d71d926da7aca74c79f45c47610e62e347b6) )
	ROM_LOAD( "tot05r.p1", 0x0000, 0x020000, CRC(a085467b) SHA1(de2deda7635c9565db0f69aa6f375216ed36b7bb) )
	ROM_LOAD( "tot05s.p1", 0x0000, 0x020000, CRC(7dd1cfa8) SHA1(3bd0eeb621cc81ac462a6981e081837985f6635b) )
	ROM_LOAD( "tot05y.p1", 0x0000, 0x020000, CRC(9469ddb4) SHA1(553812e3ece921d31c585b6a412c00ea5095b1b0) )
	ROM_LOAD( "tot06ad.p1", 0x0000, 0x020000, CRC(ebe50569) SHA1(1906f1a8d47cc9ee3fa703ad57b180f8a4cdcf89) )
	ROM_LOAD( "tot06b.p1", 0x0000, 0x020000, CRC(4d5a8ebe) SHA1(d30da9ce729fed7ad42b30d522b2b6d65a462b84) )
	ROM_LOAD( "tot06bd.p1", 0x0000, 0x020000, CRC(66ffabac) SHA1(25822e42a58173b8f51dcbbd98d041b261e675e4) )
	ROM_LOAD( "tot06c.p1", 0x0000, 0x020000, CRC(7a1033ac) SHA1(9d3c2f521574f405e1da81b605581bc4b6f011a4) )
	ROM_LOAD( "tot06d.p1", 0x0000, 0x020000, CRC(304185c8) SHA1(f7ef4fce3ce9a455f39766ae97ebfbf93418e019) )
	ROM_LOAD( "tot06dk.p1", 0x0000, 0x020000, CRC(2c3803fa) SHA1(68c83ccdd1f776376608918d9a1257fe64ce3a9b) )
	ROM_LOAD( "tot06dr.p1", 0x0000, 0x020000, CRC(9f37b8cb) SHA1(3eb2f843dc4f87b7bfe16da1d133750ad7075a71) )
	ROM_LOAD( "tot06dy.p1", 0x0000, 0x020000, CRC(abdb2304) SHA1(1c29f4176306f472323388f8c34b102930fb9f5f) )
	ROM_LOAD( "tot06k.p1", 0x0000, 0x020000, CRC(079d26e8) SHA1(60e3d02d62f6fde6bdf4b9e77702549d493ccf09) )
	ROM_LOAD( "tot06r.p1", 0x0000, 0x020000, CRC(b4929dd9) SHA1(fa3d99b8f6344c9511ecc864d4fff4629b105b5f) )
	ROM_LOAD( "tot06s.p1", 0x0000, 0x020000, CRC(c6140fea) SHA1(c2257dd84bf97b71580e8b873fc745dfa456ddd9) )
	ROM_LOAD( "tot06y.p1", 0x0000, 0x020000, CRC(807e0616) SHA1(2a3f89239a7fa43dfde90dd7ad929747e888074b) )
	ROM_LOAD( "tten2010", 0x0000, 0x020000, CRC(28373e9a) SHA1(496df7b511b950b5affe9d65c80037f3ecddc5f8) )

	M4TOOT_EXTRA_ROMS
ROM_END

GAME(199?, m4toot	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Ten Out Of Ten (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

M4TOOT_SET( 199?, m4toot__za,	m4topten,	"tot15g",		0x0000, 0x020000, CRC(1f9508ad) SHA1(33089ea05f6adecf8f4004aa1e4d626969b6ac3a), "hack?","Ten Out Of Ten (Barcrest) (MPU4) (TOC 0.3, hack?)" )
M4TOOT_SET( 199?, m4toot__zb,	m4topten,	"tot15t",		0x0000, 0x020000, CRC(1ce7f467) SHA1(cf47a126500680107a2f31743c3fff8290b595b8), "hack?","Ten Out Of Ten (Barcrest) (MPU4) (TOT 0.4, hack?)" )

