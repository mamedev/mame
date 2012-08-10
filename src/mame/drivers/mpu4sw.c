/* mpu4 sets which have been split, and appear to do something useful */

/* many sets have the BARCREST or BWG string at FFE0 replaced with other things
  such at 'FATHER CHRISTMAS' are these hacked / bootlegs requiring special hw?
  
  sets with 'D' in the ident code are Datapak sets, and do not play without a datapak connected
  sets with 'Y' seem to require the % key to be set

  */

#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4yam );
MACHINE_CONFIG_EXTERN( mod4oki );
MACHINE_CONFIG_EXTERN( mod2 );
INPUT_PORTS_EXTERN( mpu4 );
INPUT_PORTS_EXTERN( grtecp );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn );
INPUT_PORTS_EXTERN( mpu4jackpot8per );

#define GAME_FLAGS (GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK)

DRIVER_INIT_MEMBER(mpu4_state,m4debug)
{
	// many original barcrest / bwb sets have identification info around here
	// this helps with sorting
	UINT8 *src = machine().root_device().memregion( "maincpu" )->base();
	int size = machine().root_device().memregion( "maincpu" )->bytes();
	
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

DRIVER_INIT_MEMBER(mpu4_state,m4_showstring)
{
	DRIVER_INIT_CALL(m4default);
	DRIVER_INIT_CALL(m4debug);
}

DRIVER_INIT_MEMBER(mpu4_state,m4_showstring_big)
{
	DRIVER_INIT_CALL(m4default_bigbank);
	DRIVER_INIT_CALL(m4debug);
}



DRIVER_INIT_MEMBER(mpu4_state,m_grtecpss)
{
	DRIVER_INIT_CALL(m_grtecp);
	DRIVER_INIT_CALL(m4debug);
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


M4CASHMN_SET( 199?, m4cashmn,		0,			"cma07s.p1",			0x0000, 0x020000, CRC(e9c1d9f2) SHA1(f2df4ae650ec2b62d15bbaa562d638476bf926e7), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 1)" )
M4CASHMN_SET( 199?, m4cashmn__a,	m4cashmn,	"camc2010",				0x0000, 0x020000, CRC(82e459ab) SHA1(62e1906007f6bba99e3e8badc3472070e8ae84f8), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 2)" )
M4CASHMN_SET( 199?, m4cashmn__b,	m4cashmn,	"cma07ad.p1",			0x0000, 0x020000, CRC(411889fd) SHA1(5855b584315867ecc5df6d37f4a664b8331ecde8), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 3)" )
M4CASHMN_SET( 199?, m4cashmn__c,	m4cashmn,	"cma07b.p1",			0x0000, 0x020000, CRC(ab889a33) SHA1(0f3ed0e4b8131585bcb4af47674fb1b65c37503d), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 4)" )
M4CASHMN_SET( 199?, m4cashmn__d,	m4cashmn,	"cma07bd.p1",			0x0000, 0x020000, CRC(cc022738) SHA1(5968d1b6db55008cbd3c83651214c61c28fd4c5c), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 5)" )
M4CASHMN_SET( 199?, m4cashmn__e,	m4cashmn,	"cma07c.p1",			0x0000, 0x020000, CRC(9cc22721) SHA1(ee4e9860641c8bf7db024a5bf9469265a6383e0a), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 6)" )
M4CASHMN_SET( 199?, m4cashmn__f,	m4cashmn,	"cma07d.p1",			0x0000, 0x020000, CRC(d6939145) SHA1(45b6f7f80c7a2f4377d9bf8e184fb791f4ed0a2d), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 7)" )
M4CASHMN_SET( 199?, m4cashmn__g,	m4cashmn,	"cma07dk.p1",			0x0000, 0x020000, CRC(86c58f6e) SHA1(fce50f86a641d27d0f5e5ecbac84822ccc9c177b), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 8)" )
M4CASHMN_SET( 199?, m4cashmn__h,	m4cashmn,	"cma07dr.p1",			0x0000, 0x020000, CRC(35ca345f) SHA1(ddbb926988028bef13ebaa949d3ee92599770003), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 9)" )
M4CASHMN_SET( 199?, m4cashmn__i,	m4cashmn,	"cma07dy.p1",			0x0000, 0x020000, CRC(0126af90) SHA1(0f303451fd8ca8c0cc50a31297f0d2729cfc2d7b), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 10)" )
M4CASHMN_SET( 199?, m4cashmn__j,	m4cashmn,	"cma07k.p1",			0x0000, 0x020000, CRC(e14f3265) SHA1(7b5dc581fe8679559356fdca9644985da7d299cb), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 11)" )
M4CASHMN_SET( 199?, m4cashmn__k,	m4cashmn,	"cma07r.p1",			0x0000, 0x020000, CRC(52408954) SHA1(623f840d94cc3cf2d2d648eb2be644d48350b169), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 12)" )
M4CASHMN_SET( 199?, m4cashmn__l,	m4cashmn,	"cma07y.p1",			0x0000, 0x020000, CRC(66ac129b) SHA1(97f8c0c1f46444d4a492bc3dd3689df038000640), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 13)" )
M4CASHMN_SET( 199?, m4cashmn__m,	m4cashmn,	"cma08ad.p1",			0x0000, 0x020000, CRC(fce2f785) SHA1(fc508e3d1036319894985600cb0142f13536078c), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 14)" )
M4CASHMN_SET( 199?, m4cashmn__n,	m4cashmn,	"cma08b.p1",			0x0000, 0x020000, CRC(df7526de) SHA1(71456496fc31ae11ffa7c543b6444adba735aeb9), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 15)" )
M4CASHMN_SET( 199?, m4cashmn__o,	m4cashmn,	"cma08bd.p1",			0x0000, 0x020000, CRC(71f85940) SHA1(439c54f35f4f6161a683d2c3d2bb6ce81b4190bf), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 16)" )
M4CASHMN_SET( 199?, m4cashmn__p,	m4cashmn,	"cma08c.p1",			0x0000, 0x020000, CRC(e83f9bcc) SHA1(e20297ba5238b59c3872776b01e6a89a51a7aea7), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 17)" )
M4CASHMN_SET( 199?, m4cashmn__q,	m4cashmn,	"cma08d.p1",			0x0000, 0x020000, CRC(a26e2da8) SHA1(928dfe399a7ae278dadd1e930bd370022f5113c4), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 18)" )
M4CASHMN_SET( 199?, m4cashmn__r,	m4cashmn,	"cma08dk.p1",			0x0000, 0x020000, CRC(3b3ff116) SHA1(f60f0f9d996398a0f1c5b7d2a411613c42149e65), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 19)" )
M4CASHMN_SET( 199?, m4cashmn__s,	m4cashmn,	"cma08dr.p1",			0x0000, 0x020000, CRC(88304a27) SHA1(9b86a49edca078dd68abab4c3e8655d3b4e79d47), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 20)" )
M4CASHMN_SET( 199?, m4cashmn__t,	m4cashmn,	"cma08dy.p1",			0x0000, 0x020000, CRC(bcdcd1e8) SHA1(a7a4ab2313198c3bc0536526bd83179fd9170e66), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 21)" )
M4CASHMN_SET( 199?, m4cashmn__u,	m4cashmn,	"cma08k.p1",			0x0000, 0x020000, CRC(95b28e88) SHA1(282a782900a0ddf60c66aa6a69e6871bb42c647a), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 22)" )
M4CASHMN_SET( 199?, m4cashmn__v,	m4cashmn,	"cma08r.p1",			0x0000, 0x020000, CRC(26bd35b9) SHA1(74d07da26932bf48fe4b79b39ff76956b0993f3b), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 23)" )
M4CASHMN_SET( 199?, m4cashmn__w,	m4cashmn,	"cma08s.p1",			0x0000, 0x020000, CRC(d0154d3c) SHA1(773f211092c51fb4ca1ef6a5a0cbdb15f842aca8), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 24)" )
M4CASHMN_SET( 199?, m4cashmn__x,	m4cashmn,	"cma08y.p1",			0x0000, 0x020000, CRC(1251ae76) SHA1(600ce195be615796b887bb56bebb6c4322709632), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 25)" )
M4CASHMN_SET( 199?, m4cashmn__y,	m4cashmn,	"cmh06ad.p1",			0x0000, 0x020000, CRC(ea2f6866) SHA1(afae312a488d7d83576c17eb2627a84637d88f18), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 26)" )
M4CASHMN_SET( 199?, m4cashmn__z,	m4cashmn,	"cmh06b.p1",			0x0000, 0x020000, CRC(2d4d9667) SHA1(896ed70962c8904646df7159c3717399d0ceb022), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 27)" )
M4CASHMN_SET( 199?, m4cashmn__0,	m4cashmn,	"cmh06bd.p1",			0x0000, 0x020000, CRC(6735c6a3) SHA1(4bce480c57473a9b0787a87a462c76e146a10157), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 28)" )
M4CASHMN_SET( 199?, m4cashmn__1,	m4cashmn,	"cmh06c.p1",			0x0000, 0x020000, CRC(1a072b75) SHA1(89d4aed011391b2f12b48c0344136d83175ff2f0), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 29)" )
M4CASHMN_SET( 199?, m4cashmn__2,	m4cashmn,	"cmh06d.p1",			0x0000, 0x020000, CRC(50569d11) SHA1(bdf7e984766bbe90bafbf0b367690ca65a8612d2), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 30)" )
M4CASHMN_SET( 199?, m4cashmn__3,	m4cashmn,	"cmh06dk.p1",			0x0000, 0x020000, CRC(2df26ef5) SHA1(c716b73396d0af1f69f5812bace06341d368859f), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 31)" )
M4CASHMN_SET( 199?, m4cashmn__4,	m4cashmn,	"cmh06dr.p1",			0x0000, 0x020000, CRC(9efdd5c4) SHA1(b9e02fe91e766aff41ca19879ab29e53bdee537e), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 32)" )
M4CASHMN_SET( 199?, m4cashmn__5,	m4cashmn,	"cmh06dy.p1",			0x0000, 0x020000, CRC(aa114e0b) SHA1(8bc9b94e488a98b8a8008f9a35b6c078cc5c8f3f), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 33)" )
M4CASHMN_SET( 199?, m4cashmn__6,	m4cashmn,	"cmh06k.p1",			0x0000, 0x020000, CRC(678a3e31) SHA1(2351b5167eec2a0d23c9938014de6f6ee07f13ff), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 34)" )
M4CASHMN_SET( 199?, m4cashmn__7,	m4cashmn,	"cmh06r.p1",			0x0000, 0x020000, CRC(d4858500) SHA1(489fd55ac6c93b94bfb9297fd71b5d74bf95a97f), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 35)" )
M4CASHMN_SET( 199?, m4cashmn__8,	m4cashmn,	"cmh06s.p1",			0x0000, 0x020000, CRC(9d3b4260) SHA1(7c4740585d17be3da3a0ea6e7fc68f89538013fb), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 36)" )
M4CASHMN_SET( 199?, m4cashmn__9,	m4cashmn,	"cmh06y.p1",			0x0000, 0x020000, CRC(e0691ecf) SHA1(978fa00736967dd09d48ce5c847698b39a058ab5), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 37)" )
M4CASHMN_SET( 199?, m4cashmn__aa,	m4cashmn,	"cmh07ad.p1",			0x0000, 0x020000, CRC(4f354391) SHA1(687eccc312cd69f8bb70e35837f0b7ce74392936), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 38)" )
M4CASHMN_SET( 199?, m4cashmn__ab,	m4cashmn,	"cmh07b.p1",			0x0000, 0x020000, CRC(27fb6e7b) SHA1(c1558e4a0e2c28a825c2c5bb4089143cf919b67c), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 39)" )
M4CASHMN_SET( 199?, m4cashmn__ac,	m4cashmn,	"cmh07bd.p1",			0x0000, 0x020000, CRC(c22fed54) SHA1(5b6df1ed8518f9ba3e02b17c189c01ad1d0acbbb), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 40)" )
M4CASHMN_SET( 199?, m4cashmn__ad,	m4cashmn,	"cmh07c.p1",			0x0000, 0x020000, CRC(10b1d369) SHA1(9933a2a7933df941ee93e16682e91dcc90abb627), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 41)" )
M4CASHMN_SET( 199?, m4cashmn__ae,	m4cashmn,	"cmh07d.p1",			0x0000, 0x020000, CRC(5ae0650d) SHA1(da6917aa186daf59f35124c7cdc9d039d365c4c2), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 42)" )
M4CASHMN_SET( 199?, m4cashmn__af,	m4cashmn,	"cmh07dk.p1",			0x0000, 0x020000, CRC(88e84502) SHA1(2ab86be51b3dde0b2cb05e3af5f43aad3d8a76df), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 43)" )
M4CASHMN_SET( 199?, m4cashmn__ag,	m4cashmn,	"cmh07dr.p1",			0x0000, 0x020000, CRC(3be7fe33) SHA1(074243cdfd37ba36e18e00610f45473e46ddc728), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 44)" )
M4CASHMN_SET( 199?, m4cashmn__ah,	m4cashmn,	"cmh07dy.p1",			0x0000, 0x020000, CRC(0f0b65fc) SHA1(68d775bb4af9595ac87c33c2663b272640eea69e), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 45)" )
M4CASHMN_SET( 199?, m4cashmn__ai,	m4cashmn,	"cmh07k.p1",			0x0000, 0x020000, CRC(6d3cc62d) SHA1(85f76fd8513c20683d486de7a1509cadfb6ecaa9), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 46)" )
M4CASHMN_SET( 199?, m4cashmn__aj,	m4cashmn,	"cmh07r.p1",			0x0000, 0x020000, CRC(de337d1c) SHA1(dd07727fb183833eced5c0c2dc284d571baacd25), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 47)" )
M4CASHMN_SET( 199?, m4cashmn__ak,	m4cashmn,	"cmh07s.p1",			0x0000, 0x020000, CRC(0367f4cf) SHA1(8b24a9009ff17d517b34e078ebbdc17465df139d), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 48)" )
M4CASHMN_SET( 199?, m4cashmn__al,	m4cashmn,	"cmh07y.p1",			0x0000, 0x020000, CRC(eadfe6d3) SHA1(80541aba612b8ebba7ab159c61e6492b9c06feda), "Barcrest","Cash Machine (Barcrest) (MPU4) (set 49)" )
// "(C)1993 BARCREST"  and "CMH 0.6"
M4CASHMN_SET( 199?, m4cashmn__za,	m4cashmn,	"cma15g",				0x0000, 0x020000, CRC(f30b3ef2) SHA1(c8fb4d883d12a477a703d8cb0842994675aaf879), "hack?","Cash Machine (Barcrest) (MPU4) (CMH 0.6Y, hack?)" )
// no copyright string, and "CMA 0.7"
M4CASHMN_SET( 199?, m4cashmn__zb,	m4cashmn,	"cma15t",				0x0000, 0x020000, CRC(a4ed66a4) SHA1(0e98859c4d7dbccdea9396c3fea9f345b2f08db6), "hack?","Cash Machine (Barcrest) (MPU4) (CMA 0.7C, hack?)" )



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


M4TOOT_SET( 199?, m4toot,		0,		"toc03c.p1",	0x0000, 0x020000, CRC(752ffa3f) SHA1(6cbe521ff85173159b6d34cc3e29a4192cd66394), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 1)" )
M4TOOT_SET( 199?, m4toot__a,	m4toot,	"toc03ad.p1",	0x0000, 0x020000, CRC(f67e53c1) SHA1(07a50fb649c5085a33f0a1a9b3d65b0b61a3f152), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 2)" )
M4TOOT_SET( 199?, m4toot__b,	m4toot,	"toc03b.p1",	0x0000, 0x020000, CRC(4265472d) SHA1(01d5eb4e0a30abd1efed45658dcd8455494aabc4), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 3)" )
M4TOOT_SET( 199?, m4toot__c,	m4toot,	"toc03bd.p1",	0x0000, 0x020000, CRC(7b64fd04) SHA1(377af32317d8356f06b19de553a13e8558993c34), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 4)" )
M4TOOT_SET( 199?, m4toot__d,	m4toot,	"toc03d.p1",	0x0000, 0x020000, CRC(61f6b566) SHA1(2abd8092fe387c474ed10885a23ac242fa1462fa), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 5)" )
M4TOOT_SET( 199?, m4toot__e,	m4toot,	"toc03dk.p1",	0x0000, 0x020000, CRC(31a35552) SHA1(3765fe6209d7b33afa1805ba56376e83e825165f), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 6)" )
M4TOOT_SET( 199?, m4toot__f,	m4toot,	"toc03dr.p1",	0x0000, 0x020000, CRC(82acee63) SHA1(5a95425d6175c6496d745011d0ca3d744a027579), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 7)" )
M4TOOT_SET( 199?, m4toot__g,	m4toot,	"toc03dy.p1",	0x0000, 0x020000, CRC(b64075ac) SHA1(ffdf8e45c2eab593570e15efd5161b67de5e4ecf), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 8)" )
M4TOOT_SET( 199?, m4toot__h,	m4toot,	"toc03k.p1",	0x0000, 0x020000, CRC(08a2ef7b) SHA1(a4181db6280c7cc37b54baaf9cce1e61f61f3274), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 9)" )
M4TOOT_SET( 199?, m4toot__i,	m4toot,	"toc03r.p1",	0x0000, 0x020000, CRC(bbad544a) SHA1(5fb31e5641a9e85147f5b61c5aba5a1ee7470f9c), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 10)" )
M4TOOT_SET( 199?, m4toot__j,	m4toot,	"toc03s.p1",	0x0000, 0x020000, CRC(30feff92) SHA1(14397768ebd7469b4d1cff22ca9727f63608a98a), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 11)" )
M4TOOT_SET( 199?, m4toot__k,	m4toot,	"toc03y.p1",	0x0000, 0x020000, CRC(8f41cf85) SHA1(315b359d6d1a9f6ad939be1fc5e4d8f21f998fb8), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 12)" )
M4TOOT_SET( 199?, m4toot__l,	m4toot,	"toc04ad.p1",	0x0000, 0x020000, CRC(59075e2e) SHA1(a3ad5c642fb9cebcce2fb6c1e65514f2414948e0), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 13)" )
M4TOOT_SET( 199?, m4toot__m,	m4toot,	"toc04b.p1",	0x0000, 0x020000, CRC(b2d54721) SHA1(4c72d434c0f4f37b9a3f08a760c7fe3851717059), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 14)" )
M4TOOT_SET( 199?, m4toot__n,	m4toot,	"toc04bd.p1",	0x0000, 0x020000, CRC(d41df0eb) SHA1(e5a04c728a2893073ff8b5f6efd7cffd433a2985), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 15)" )
M4TOOT_SET( 199?, m4toot__o,	m4toot,	"toc04c.p1",	0x0000, 0x020000, CRC(859ffa33) SHA1(05b7bd3b87a0ebcc78de751766cfcdc4276035ac), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 16)" )
M4TOOT_SET( 199?, m4toot__p,	m4toot,	"toc04d.p1",	0x0000, 0x020000, CRC(9146b56a) SHA1(04bcd265d83e3554aef2de05aab9c3869bb966ea), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 17)" )
M4TOOT_SET( 199?, m4toot__q,	m4toot,	"toc04dk.p1",	0x0000, 0x020000, CRC(9eda58bd) SHA1(5e38f87a162d1cb37e74850af6a00ae81619ecbe), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 18)" )
M4TOOT_SET( 199?, m4toot__r,	m4toot,	"toc04dr.p1",	0x0000, 0x020000, CRC(2dd5e38c) SHA1(a1b0e8d48e164ab91b277a7efedf5b9fc73fc266), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 19)" )
M4TOOT_SET( 199?, m4toot__s,	m4toot,	"toc04dy.p1",	0x0000, 0x020000, CRC(19397843) SHA1(e7e3299d8e46c79d3cd0ea7fd639a1d649a806df), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 20)" )
M4TOOT_SET( 199?, m4toot__t,	m4toot,	"toc04k.p1",	0x0000, 0x020000, CRC(f812ef77) SHA1(d465b771efed27a9f616052d3fcabdbeb7c2d151), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 21)" )
M4TOOT_SET( 199?, m4toot__u,	m4toot,	"toc04r.p1",	0x0000, 0x020000, CRC(4b1d5446) SHA1(f4bac0c8257add41295679b3541d2064d8c772c2), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 22)" )
M4TOOT_SET( 199?, m4toot__v,	m4toot,	"toc04s.p1",	0x0000, 0x020000, CRC(295e6fff) SHA1(a21d991f00f144e12de60b891e3e2e5dd7d08d71), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 23)" )
M4TOOT_SET( 199?, m4toot__w,	m4toot,	"toc04y.p1",	0x0000, 0x020000, CRC(7ff1cf89) SHA1(d4ab56b2b5b05643cd077b8d596b6cddf8a25134), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 24)" )
M4TOOT_SET( 199?, m4toot__x,	m4toot,	"tot05ad.p1",	0x0000, 0x020000, CRC(fce00fcc) SHA1(7d10c0b83d782a9e603522ed039089866d931474), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 25)" )
M4TOOT_SET( 199?, m4toot__y,	m4toot,	"tot05b.p1",	0x0000, 0x020000, CRC(594d551c) SHA1(3fcec5f41cfbea497aa53af4570664265774d1aa), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 26)" )
M4TOOT_SET( 199?, m4toot__z,	m4toot,	"tot05bd.p1",	0x0000, 0x020000, CRC(71faa109) SHA1(8bec4a03e2dc43656c910652cf10d1afdf0bab33), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 27)" )
M4TOOT_SET( 199?, m4toot__0,	m4toot,	"tot05c.p1",	0x0000, 0x020000, CRC(6e07e80e) SHA1(087a51da5578c326d7d9716c61b454be5e091761), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 28)" )
M4TOOT_SET( 199?, m4toot__1,	m4toot,	"tot05d.p1",	0x0000, 0x020000, CRC(24565e6a) SHA1(0f2c19e54e5a78ae10e786d49ebcd7c16e41850c), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 29)" )
M4TOOT_SET( 199?, m4toot__2,	m4toot,	"tot05dk.p1",	0x0000, 0x020000, CRC(3b3d095f) SHA1(326574adaa285a479abb5ae7515ef7d6bdd64126), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 30)" )
M4TOOT_SET( 199?, m4toot__3,	m4toot,	"tot05dr.p1",	0x0000, 0x020000, CRC(8832b26e) SHA1(d5414560245bbb2c070f7a035e5e3416617c2cf3), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 31)" )
M4TOOT_SET( 199?, m4toot__4,	m4toot,	"tot05dy.p1",	0x0000, 0x020000, CRC(bcde29a1) SHA1(cf8af40faf81a2a3141d3addd6b28c917beeda49), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 32)" )
M4TOOT_SET( 199?, m4toot__5,	m4toot,	"tot05k.p1",	0x0000, 0x020000, CRC(138afd4a) SHA1(bc53d71d926da7aca74c79f45c47610e62e347b6), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 33)" )
M4TOOT_SET( 199?, m4toot__6,	m4toot,	"tot05r.p1",	0x0000, 0x020000, CRC(a085467b) SHA1(de2deda7635c9565db0f69aa6f375216ed36b7bb), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 34)" )
M4TOOT_SET( 199?, m4toot__7,	m4toot,	"tot05s.p1",	0x0000, 0x020000, CRC(7dd1cfa8) SHA1(3bd0eeb621cc81ac462a6981e081837985f6635b), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 35)" )
M4TOOT_SET( 199?, m4toot__8,	m4toot,	"tot05y.p1",	0x0000, 0x020000, CRC(9469ddb4) SHA1(553812e3ece921d31c585b6a412c00ea5095b1b0), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 36)" )
M4TOOT_SET( 199?, m4toot__9,	m4toot,	"tot06ad.p1",	0x0000, 0x020000, CRC(ebe50569) SHA1(1906f1a8d47cc9ee3fa703ad57b180f8a4cdcf89), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 37)" )
M4TOOT_SET( 199?, m4toot__aa,	m4toot,	"tot06b.p1",	0x0000, 0x020000, CRC(4d5a8ebe) SHA1(d30da9ce729fed7ad42b30d522b2b6d65a462b84), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 38)" )
M4TOOT_SET( 199?, m4toot__ab,	m4toot,	"tot06bd.p1",	0x0000, 0x020000, CRC(66ffabac) SHA1(25822e42a58173b8f51dcbbd98d041b261e675e4), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 39)" )
M4TOOT_SET( 199?, m4toot__ac,	m4toot,	"tot06c.p1",	0x0000, 0x020000, CRC(7a1033ac) SHA1(9d3c2f521574f405e1da81b605581bc4b6f011a4), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 40)" )
M4TOOT_SET( 199?, m4toot__ad,	m4toot,	"tot06d.p1",	0x0000, 0x020000, CRC(304185c8) SHA1(f7ef4fce3ce9a455f39766ae97ebfbf93418e019), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 41)" )
M4TOOT_SET( 199?, m4toot__ae,	m4toot,	"tot06dk.p1",	0x0000, 0x020000, CRC(2c3803fa) SHA1(68c83ccdd1f776376608918d9a1257fe64ce3a9b), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 42)" )
M4TOOT_SET( 199?, m4toot__af,	m4toot,	"tot06dr.p1",	0x0000, 0x020000, CRC(9f37b8cb) SHA1(3eb2f843dc4f87b7bfe16da1d133750ad7075a71), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 43)" )
M4TOOT_SET( 199?, m4toot__ag,	m4toot,	"tot06dy.p1",	0x0000, 0x020000, CRC(abdb2304) SHA1(1c29f4176306f472323388f8c34b102930fb9f5f), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 44)" )
M4TOOT_SET( 199?, m4toot__ah,	m4toot,	"tot06k.p1",	0x0000, 0x020000, CRC(079d26e8) SHA1(60e3d02d62f6fde6bdf4b9e77702549d493ccf09), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 45)" )
M4TOOT_SET( 199?, m4toot__ai,	m4toot,	"tot06r.p1",	0x0000, 0x020000, CRC(b4929dd9) SHA1(fa3d99b8f6344c9511ecc864d4fff4629b105b5f), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 46)" )
M4TOOT_SET( 199?, m4toot__aj,	m4toot,	"tot06s.p1",	0x0000, 0x020000, CRC(c6140fea) SHA1(c2257dd84bf97b71580e8b873fc745dfa456ddd9), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 47)" )
M4TOOT_SET( 199?, m4toot__ak,	m4toot,	"tot06y.p1",	0x0000, 0x020000, CRC(807e0616) SHA1(2a3f89239a7fa43dfde90dd7ad929747e888074b), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 48)" )
M4TOOT_SET( 199?, m4toot__al,	m4toot,	"tten2010",		0x0000, 0x020000, CRC(28373e9a) SHA1(496df7b511b950b5affe9d65c80037f3ecddc5f8), "Barcrest","Ten Out Of Ten (Barcrest) (MPU4) (set 49)" )
M4TOOT_SET( 199?, m4toot__za,	m4toot,	"tot15g",		0x0000, 0x020000, CRC(1f9508ad) SHA1(33089ea05f6adecf8f4004aa1e4d626969b6ac3a), "hack?","Ten Out Of Ten (Barcrest) (MPU4) (TOC 0.3, hack?)" )
M4TOOT_SET( 199?, m4toot__zb,	m4toot,	"tot15t",		0x0000, 0x020000, CRC(1ce7f467) SHA1(cf47a126500680107a2f31743c3fff8290b595b8), "hack?","Ten Out Of Ten (Barcrest) (MPU4) (TOT 0.4, hack?)" )

#define M4EAW_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "m683.chr", 0x0000, 0x000048, CRC(cbe68b44) SHA1(60efc69eba86531f51230dee17efdbbf8917f907) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "er4snd.p1", 0x000000, 0x080000, CRC(32fd0836) SHA1(ea68252b690fe1d6070209cbcfb65fe20926c6ce) ) \
	ROM_LOAD( "er4snd.p2", 0x080000, 0x080000, CRC(1df9c24f) SHA1(f0d31b1bec6f3a9791f7fabe57b45687df900efa) ) \

#define M4EAW_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4EAW_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4EAW_SET( 199?, m4eaw,		0,		"er4s.p1",					0x0000, 0x010000, CRC(163fc987) SHA1(8e1768ed2fbddbd5e00652ff40614de3978c9567), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 1)" )
M4EAW_SET( 199?, m4eaw__a,	m4eaw,	"cet03ad.p1",				0x0000, 0x010000, CRC(33afe7a5) SHA1(5d3bdb74c6babd49e88915282ad81c184bd7aa68), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 2)" )
M4EAW_SET( 199?, m4eaw__b,	m4eaw,	"cet03b.p1",				0x0000, 0x010000, CRC(7674e2a5) SHA1(188e683eac91f64fe563b0f09f2b934e709c47fb), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 3)" )
M4EAW_SET( 199?, m4eaw__c,	m4eaw,	"cet03bd.p1",				0x0000, 0x010000, CRC(406843a2) SHA1(7d4bf6cd3c5be0f6df687b0ba97b3b88fd377170), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 4)" )
M4EAW_SET( 199?, m4eaw__d,	m4eaw,	"cet03d.p1",				0x0000, 0x010000, CRC(2c03d5b6) SHA1(e79fd15b6a05168eb08dcd2b5f7e00d015618a22), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 5)" )
M4EAW_SET( 199?, m4eaw__e,	m4eaw,	"cet03dk.p1",				0x0000, 0x010000, CRC(7a81b524) SHA1(71856b90379af946fbc9263f596a16e1701f3564), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 6)" )
M4EAW_SET( 199?, m4eaw__f,	m4eaw,	"cet03dr.p1",				0x0000, 0x010000, CRC(63a5622a) SHA1(1e3cf5487623b850598d21c0bb5ef8a0b9dffd4f), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 7)" )
M4EAW_SET( 199?, m4eaw__g,	m4eaw,	"cet03dy.p1",				0x0000, 0x010000, CRC(fece4ac4) SHA1(badf4f94d565958fc9f42a443f53ec9624925ee1), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 8)" )
M4EAW_SET( 199?, m4eaw__h,	m4eaw,	"cet03k.p1",				0x0000, 0x010000, CRC(f6531a43) SHA1(75ec5c8fc0012fee144daab7761f3717c17fa22d), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 9)" )
M4EAW_SET( 199?, m4eaw__i,	m4eaw,	"cet03r.p1",				0x0000, 0x010000, CRC(fec4a6c0) SHA1(89fac7e4df77f526d0e357f1874b73be932548ce), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 10)" )
M4EAW_SET( 199?, m4eaw__j,	m4eaw,	"cet03s.p1",				0x0000, 0x010000, CRC(bec3ea51) SHA1(740a73da105d8329dc9ceaa5e8c25b305124e2dd), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 11)" )
M4EAW_SET( 199?, m4eaw__k,	m4eaw,	"cet03y.p1",				0x0000, 0x010000, CRC(63af8e2e) SHA1(97b9dd02bf8a72ca0be7c1a9cb753fbd55644497), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 12)" )
M4EAW_SET( 199?, m4eaw__l,	m4eaw,	"ceu02ad.p1",				0x0000, 0x010000, CRC(5805182c) SHA1(c15ef2e05061fd89944b039f007d92bc4bdf66d5), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 13)" )
M4EAW_SET( 199?, m4eaw__m,	m4eaw,	"ceu02b.p1",				0x0000, 0x010000, CRC(cbf62a02) SHA1(20fb16ac4602d4e386e5dc01e1b7e83c459f614d), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 14)" )
M4EAW_SET( 199?, m4eaw__n,	m4eaw,	"ceu02bd.p1",				0x0000, 0x010000, CRC(6e197566) SHA1(16f44ca77bc02c7eb186c3684b4e837da0d73553), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 15)" )
M4EAW_SET( 199?, m4eaw__o,	m4eaw,	"ceu02d.p1",				0x0000, 0x010000, CRC(470cab31) SHA1(f42045f25022cc5e4b07a687f55f7698435b550e), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 16)" )
M4EAW_SET( 199?, m4eaw__p,	m4eaw,	"ceu02dk.p1",				0x0000, 0x010000, CRC(bea24ff7) SHA1(1bf8464136732ee8433c73747e854be3c991a2fe), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 17)" )
M4EAW_SET( 199?, m4eaw__q,	m4eaw,	"ceu02dr.p1",				0x0000, 0x010000, CRC(3e2e4183) SHA1(e52bca2913509f26af9ca4a93ab2a2bbf74d1ac9), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 18)" )
M4EAW_SET( 199?, m4eaw__r,	m4eaw,	"ceu02dy.p1",				0x0000, 0x010000, CRC(a345696d) SHA1(a189eb6a6a6a83fe0d490f4a7c8e9c4c52aa91f7), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 19)" )
M4EAW_SET( 199?, m4eaw__s,	m4eaw,	"ceu02k.p1",				0x0000, 0x010000, CRC(0e0a1ba9) SHA1(e1ee2595a3fd4fe874f50dc027f6c931636aadcc), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 20)" )
M4EAW_SET( 199?, m4eaw__t,	m4eaw,	"ceu02r.p1",				0x0000, 0x010000, CRC(1a882a6a) SHA1(c966be957e7a78c33a28afd79ba60c69a6de42b8), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 21)" )
M4EAW_SET( 199?, m4eaw__u,	m4eaw,	"ceu02s.p1",				0x0000, 0x010000, CRC(d52099e6) SHA1(10f1acb948fa7c4b547f801ddb5e15111992ca91), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 22)" )
M4EAW_SET( 199?, m4eaw__v,	m4eaw,	"ceu02y.p1",				0x0000, 0x010000, CRC(87e30284) SHA1(4c598a33b73cfe6338c0f51408f2a6c1abfa978b), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 23)" )
M4EAW_SET( 199?, m4eaw__w,	m4eaw,	"enn01ad.p1",				0x0000, 0x010000, CRC(913ba1d6) SHA1(1167ccce2f0b528ec8eba140b1f9c8358fa19f54), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 24)" )
M4EAW_SET( 199?, m4eaw__x,	m4eaw,	"enn01b.p1",				0x0000, 0x010000, CRC(76cf750c) SHA1(7f3ede643c5b92d9e313c4450a0d4ef3bd9eefd3), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 25)" )
M4EAW_SET( 199?, m4eaw__y,	m4eaw,	"enn01bd.p1",				0x0000, 0x010000, CRC(c6c29211) SHA1(a49759c4c00633405a338eeb89fcb00f7503990c), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 26)" )
M4EAW_SET( 199?, m4eaw__z,	m4eaw,	"enn01c.p1",				0x0000, 0x010000, CRC(1ea8f766) SHA1(3f08da014727b50e0375b8470a37c75042b089c6), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 27)" )
M4EAW_SET( 199?, m4eaw__0,	m4eaw,	"enn01d.p1",				0x0000, 0x010000, CRC(3691905e) SHA1(131b3384c2399b214fb70670c9945be4afdb470e), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 28)" )
M4EAW_SET( 199?, m4eaw__1,	m4eaw,	"enn01dk.p1",				0x0000, 0x010000, CRC(1abb5196) SHA1(952451f637d890a51a2567b5b02826f7647e5deb), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 29)" )
M4EAW_SET( 199?, m4eaw__2,	m4eaw,	"enn01dr.p1",				0x0000, 0x010000, CRC(e50fb399) SHA1(5d4d5a933efe7e122e4d0cecab9b7e6f01398a8f), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 30)" )
M4EAW_SET( 199?, m4eaw__3,	m4eaw,	"enn01dy.p1",				0x0000, 0x010000, CRC(be3e5901) SHA1(ea3f366724135682da7cddad3c82e5f4c434f4a9), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 31)" )
M4EAW_SET( 199?, m4eaw__4,	m4eaw,	"enn01k.p1",				0x0000, 0x010000, CRC(273d7b10) SHA1(5577355c918407e548266a16b225e8a4f58c921c), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 32)" )
M4EAW_SET( 199?, m4eaw__5,	m4eaw,	"enn01r.p1",				0x0000, 0x010000, CRC(aee3f31e) SHA1(72676bc6b3bc287bf3bd3e7719848b40aa1b3627), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 33)" )
M4EAW_SET( 199?, m4eaw__6,	m4eaw,	"enn01s.p1",				0x0000, 0x010000, CRC(d0ba447d) SHA1(744d5448c5318287e58994b684e116ac1a236f05), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 34)" )
M4EAW_SET( 199?, m4eaw__7,	m4eaw,	"enn01y.p1",				0x0000, 0x010000, CRC(91a73867) SHA1(5197fcd5bf3dc036095b8291d7b23776995d84d1), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 35)" )
M4EAW_SET( 199?, m4eaw__8,	m4eaw,	"eon01ad.p1",				0x0000, 0x010000, CRC(998b0e8d) SHA1(f2d0c43073d76d662c3a997b1fd081016e4c7a7d), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 36)" )
M4EAW_SET( 199?, m4eaw__9,	m4eaw,	"eon01b.p1",				0x0000, 0x010000, CRC(66f281db) SHA1(b9bd37c53ab7c8838ec87062c8b9da39779b9fa9), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 37)" )
M4EAW_SET( 199?, m4eaw__aa,	m4eaw,	"eon01bd.p1",				0x0000, 0x010000, CRC(66a378ca) SHA1(6639f36df67af8bdd381ad3e16e0adc78a4552f4), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 38)" )
M4EAW_SET( 199?, m4eaw__ab,	m4eaw,	"eon01c.p1",				0x0000, 0x010000, CRC(d02e2c30) SHA1(34f1be5f49d50f468bffc425fa2e9d0f8afcf70b), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 39)" )
M4EAW_SET( 199?, m4eaw__ac,	m4eaw,	"eon01d.p1",				0x0000, 0x010000, CRC(a2752f68) SHA1(3a47ced5259c6f690b03c3a884f1d25bd68e0d3f), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 40)" )
M4EAW_SET( 199?, m4eaw__ad,	m4eaw,	"eon01dk.p1",				0x0000, 0x010000, CRC(efd60656) SHA1(9383a7b183266f75edd3ae519e8dfff858f015c4), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 41)" )
M4EAW_SET( 199?, m4eaw__ae,	m4eaw,	"eon01dr.p1",				0x0000, 0x010000, CRC(1062e459) SHA1(de334cdecde0dd414e016e11a54720dee903393c), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 42)" )
M4EAW_SET( 199?, m4eaw__af,	m4eaw,	"eon01dy.p1",				0x0000, 0x010000, CRC(d5a39761) SHA1(9b69f9e45d87f53196e5d4fd595300beb573ff49), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 43)" )
M4EAW_SET( 199?, m4eaw__ag,	m4eaw,	"eon01k.p1",				0x0000, 0x010000, CRC(1d34dea7) SHA1(546db8247d0c78501fe4ec818d614e8f451b0076), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 44)" )
M4EAW_SET( 199?, m4eaw__ah,	m4eaw,	"eon01r.p1",				0x0000, 0x010000, CRC(7c70a508) SHA1(2c5835f36ef4c215ff9f6f6cc350f0916b397b7b), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 45)" )
M4EAW_SET( 199?, m4eaw__ai,	m4eaw,	"eon01s.p1",				0x0000, 0x010000, CRC(e2e9ce10) SHA1(41a08b17285d6591b4a5cb6b1b6cc40ee7d35f01), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 46)" )
M4EAW_SET( 199?, m4eaw__aj,	m4eaw,	"eon01y.p1",				0x0000, 0x010000, CRC(ddc4f7d1) SHA1(bbc21ba153541df1507e01d4a25a1a669c8eab62), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 47)" )
M4EAW_SET( 199?, m4eaw__ak,	m4eaw,	"er2ad.p1",					0x0000, 0x010000, CRC(4e5fcc8b) SHA1(8176ca01ad49f39e1337a085cf3a1fd33803c517), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 48)" )
M4EAW_SET( 199?, m4eaw__al,	m4eaw,	"er2b.p1",					0x0000, 0x010000, CRC(999c6510) SHA1(bc70b88183df84ea0e18e1017ab9d74545ce7588), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 49)" )
M4EAW_SET( 199?, m4eaw__am,	m4eaw,	"er2bd.p1",					0x0000, 0x010000, CRC(3f50573a) SHA1(46527b08d751372df09d61fd67054600b6e933f3), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 50)" )
M4EAW_SET( 199?, m4eaw__an,	m4eaw,	"er2d.p1",					0x0000, 0x010000, CRC(6c625759) SHA1(65de484632317b7bd1372f20e7cbdedc85a90ea4), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 51)" )
M4EAW_SET( 199?, m4eaw__ao,	m4eaw,	"er2dk.p1",					0x0000, 0x010000, CRC(e1e1ab0b) SHA1(353863e2ef1e778b7fce035ae725053fb95c300e), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 52)" )
M4EAW_SET( 199?, m4eaw__ap,	m4eaw,	"er2dr.p1",					0x0000, 0x010000, CRC(0d2e1d3f) SHA1(f75f6cf9e0ce6ccf36df83e18f03fc1485242c88), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 53)" )
M4EAW_SET( 199?, m4eaw__aq,	m4eaw,	"er2dy.p1",					0x0000, 0x010000, CRC(f20c4b31) SHA1(744ce6065b3bea3a0c128a4848282cbca2bc8056), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 54)" )
M4EAW_SET( 199?, m4eaw__ar,	m4eaw,	"er2k.p1",					0x0000, 0x010000, CRC(2c3661bb) SHA1(5f5a6b47dacdb2184d3ac9646da616283743fcbf), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 55)" )
M4EAW_SET( 199?, m4eaw__as,	m4eaw,	"er2r.p1",					0x0000, 0x010000, CRC(cb636e43) SHA1(44df3adc1d5af4c1930596f34f41884e7731be62), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 56)" )
M4EAW_SET( 199?, m4eaw__at,	m4eaw,	"er2s.p1",					0x0000, 0x010000, CRC(bfee8157) SHA1(3ce5a2ec16f06c753a054a9f645efbcd26f411ab), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 57)" )
M4EAW_SET( 199?, m4eaw__au,	m4eaw,	"er2y.p1",					0x0000, 0x010000, CRC(91369b00) SHA1(7427fcf9e350bc9a3883577de5ee4a4ab5ff63b0), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 58)" )
M4EAW_SET( 199?, m4eaw__av,	m4eaw,	"er4ad.p1",					0x0000, 0x010000, CRC(93fff89d) SHA1(3f90168efa5ecaf7707ef357616638a9d5ab746f), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 59)" )
M4EAW_SET( 199?, m4eaw__aw,	m4eaw,	"er4b.p1",					0x0000, 0x010000, CRC(cb39fda7) SHA1(4a31d2ff53942a658992a5e13c2b617da5fb03ce), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 60)" )
M4EAW_SET( 199?, m4eaw__ax,	m4eaw,	"er4bd.p1",					0x0000, 0x010000, CRC(a5e395d7) SHA1(3f134a2ce3788ac84a6de096306c651e6b2d6a4a), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 61)" )
M4EAW_SET( 199?, m4eaw__ay,	m4eaw,	"er4d.p1",					0x0000, 0x010000, CRC(33612923) SHA1(1129ced207aaf46045f20a1ef1a37af8ec537bb0), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 62)" )
M4EAW_SET( 199?, m4eaw__az,	m4eaw,	"er4dk.p1",					0x0000, 0x010000, CRC(df41d570) SHA1(4a2db04ee51bb811ac3aee5b2c3c1f1a2201f7ec), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 63)" )
M4EAW_SET( 199?, m4eaw__a0,	m4eaw,	"er4dy.p1",					0x0000, 0x010000, CRC(7df882e6) SHA1(1246220a5ac8a4454a7f3a359a5a00319395095d), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 64)" )
M4EAW_SET( 199?, m4eaw__a1,	m4eaw,	"er4k.p1",					0x0000, 0x010000, CRC(9803cc0d) SHA1(1516c3836919a7a2cc32711a9bf2d3bf3d6b82c0), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 65)" )
M4EAW_SET( 199?, m4eaw__a2,	m4eaw,	"er4y.p1",					0x0000, 0x010000, CRC(d8dece2d) SHA1(8482092434e1e94e6648e402c8b518c2f0fcc28e), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 66)" )
M4EAW_SET( 199?, m4eaw__a3,	m4eaw,	"er8ad.p1",					0x0000, 0x010000, CRC(ba059e06) SHA1(f6bb9092c9d18bccde111f8e20e79b8b4e6d8593), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 67)" )
M4EAW_SET( 199?, m4eaw__a4,	m4eaw,	"er8b.p1",					0x0000, 0x010000, CRC(27c7f954) SHA1(93305d1d4a5781de56f1e54801e25b29b6713ef0), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 68)" )
M4EAW_SET( 199?, m4eaw__a5,	m4eaw,	"er8c.p1",					0x0000, 0x010000, CRC(cee94fb3) SHA1(01ec098016b6946c3fbf96b2071076316bbd5795), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 69)" )
M4EAW_SET( 199?, m4eaw__a6,	m4eaw,	"er8dk.p1",					0x0000, 0x010000, CRC(789c5e1d) SHA1(5f5b686a770f4ab0cfa8e8ae21b3805ef6102516), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 70)" )
M4EAW_SET( 199?, m4eaw__a7,	m4eaw,	"er8dy.p1",					0x0000, 0x010000, CRC(4adf568b) SHA1(dd21b547211566ad5cb018a0205d887b7f860bc9), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 71)" )
M4EAW_SET( 199?, m4eaw__a8,	m4eaw,	"er8k.p1",					0x0000, 0x010000, CRC(c76140e4) SHA1(6c097fdd018eb594a84ceb7712a45201490ca370), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 72)" )
M4EAW_SET( 199?, m4eaw__a9,	m4eaw,	"er8s.p1",					0x0000, 0x010000, CRC(5d36bbc6) SHA1(4d0cd8e939f22d919671dc97c3d97bf6191e738f), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 73)" )
M4EAW_SET( 199?, m4eaw__ba,	m4eaw,	"er8y.p1",					0x0000, 0x010000, CRC(8a1aa409) SHA1(a7ae62e1038e52a111de3004e2160838e0d102d0), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 74)" )
M4EAW_SET( 199?, m4eaw__bb,	m4eaw,	"ertad.p1",					0x0000, 0x010000, CRC(75798f2d) SHA1(68939c187d841aa046a4f7dd8f39e8387969460c), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 75)" )
M4EAW_SET( 199?, m4eaw__bc,	m4eaw,	"ertb.p1",					0x0000, 0x010000, CRC(c6407839) SHA1(79d73d79b389682586fdf7c9c25d8e2ea5943bb6), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 76)" )
M4EAW_SET( 199?, m4eaw__bd,	m4eaw,	"ertbd.p1",					0x0000, 0x010000, CRC(4365e267) SHA1(b1853c3ddb707cb114e6bb2d780b142b80f099b6), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 77)" )
M4EAW_SET( 199?, m4eaw__be,	m4eaw,	"ertd.p1",					0x0000, 0x010000, CRC(2fabc730) SHA1(8a43afd6048006e906892d35bb0cfaa127fc0415), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 78)" )
M4EAW_SET( 199?, m4eaw__bf,	m4eaw,	"ertdk.p1",					0x0000, 0x010000, CRC(21264f37) SHA1(9819cf120e81525f18096152a555859a4f48f8ad), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 79)" )
M4EAW_SET( 199?, m4eaw__bg,	m4eaw,	"ertdr.p1",					0x0000, 0x010000, CRC(1b644f23) SHA1(94c5a307126cada90eeb45439aaab82a30228ffa), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 80)" )
M4EAW_SET( 199?, m4eaw__bh,	m4eaw,	"ertdy.p1",					0x0000, 0x010000, CRC(5a7c77fa) SHA1(37c212db131b682fd8d293a8cf8efad2e80a8a18), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 81)" )
M4EAW_SET( 199?, m4eaw__bi,	m4eaw,	"ertk.p1",					0x0000, 0x010000, CRC(19959bd3) SHA1(617f7079b39b0ef41ebb0b5f89053d723a28824d), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 82)" )
M4EAW_SET( 199?, m4eaw__bj,	m4eaw,	"ertr.p1",					0x0000, 0x010000, CRC(3264f04a) SHA1(88d1f6857f3a0acd89db1563fd5f24582b578765), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 83)" )
M4EAW_SET( 199?, m4eaw__bk,	m4eaw,	"erts.p1",					0x0000, 0x010000, CRC(185b47bb) SHA1(377cb42878572a3e94dd6be6fb106ecacb3c5059), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 84)" )
M4EAW_SET( 199?, m4eaw__bl,	m4eaw,	"erty.p1",					0x0000, 0x010000, CRC(38adc77e) SHA1(7a925e2aa946fdcf38df454ec733da1ce9bdc495), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 85)" )
M4EAW_SET( 199?, m4eaw__bm,	m4eaw,	"eun01ad.p1",				0x0000, 0x010000, CRC(0148eb57) SHA1(7ebf73402ffe68cfb045a906ed039407bd173b88), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 86)" )
M4EAW_SET( 199?, m4eaw__bn,	m4eaw,	"eun01b.p1",				0x0000, 0x010000, CRC(ad152cda) SHA1(ca5c72a54e14f8b44fddfbc5c38c4e149432f593), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 87)" )
M4EAW_SET( 199?, m4eaw__bo,	m4eaw,	"eun01bd.p1",				0x0000, 0x010000, CRC(6b0abd7c) SHA1(a6f74096bfffa082a441c094b5acadd5929ac36a), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 88)" )
M4EAW_SET( 199?, m4eaw__bp,	m4eaw,	"eun01c.p1",				0x0000, 0x010000, CRC(a65fcf8b) SHA1(13a5bc1f2918a4f3590a1cdc34b439a874934ee7), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 89)" )
M4EAW_SET( 199?, m4eaw__bq,	m4eaw,	"eun01d.p1",				0x0000, 0x010000, CRC(400af364) SHA1(ca67e98624d50717763e9965c45f0beecb07d2f9), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 90)" )
M4EAW_SET( 199?, m4eaw__br,	m4eaw,	"eun01dk.p1",				0x0000, 0x010000, CRC(c11914fa) SHA1(884fee07227f46dde056eb8e082bb821eeab99cb), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 91)" )
M4EAW_SET( 199?, m4eaw__bs,	m4eaw,	"eun01dr.p1",				0x0000, 0x010000, CRC(0eb075d4) SHA1(3977f03f6aac765c556618616919a5e10660b35d), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 92)" )
M4EAW_SET( 199?, m4eaw__bt,	m4eaw,	"eun01dy.p1",				0x0000, 0x010000, CRC(93db5d3a) SHA1(ddd209b22ed396d3329b9522649db6dda64958b7), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 93)" )
M4EAW_SET( 199?, m4eaw__bu,	m4eaw,	"eun01k.p1",				0x0000, 0x010000, CRC(9fca43fd) SHA1(f7626f122dedb217002888971100d8a34910b48d), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 94)" )
M4EAW_SET( 199?, m4eaw__bv,	m4eaw,	"eun01r.p1",				0x0000, 0x010000, CRC(15b8eb9e) SHA1(e4babaf526e6dd45bb4b7f7441a08cfbec12c661), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 95)" )
M4EAW_SET( 199?, m4eaw__bw,	m4eaw,	"eun01s.p1",				0x0000, 0x010000, CRC(d0b49fc6) SHA1(4062d9763010d42666660e383e52818d572b61b9), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 96)" )
M4EAW_SET( 199?, m4eaw__bx,	m4eaw,	"eun01y.p1",				0x0000, 0x010000, CRC(88d3c370) SHA1(6c3839a9c89ae67f80ab932ec70ebaf1240de9bb), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 97)" )
M4EAW_SET( 199?, m4eaw__bz,	m4eaw,	"everyones a winner v2-5p",	0x0000, 0x008000, CRC(eb8f2fc5) SHA1(0d3614bd5ff561d17bef0d1e620f2f812b8fed5b), "Barcrest","Everyone's A Winner (Barcrest) (MPU4) (set 99)" )


#define M4WTA_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "wn5s.chr", 0x0000, 0x000048, CRC(b90e5068) SHA1(14c57dcd7242104eb48a9be36192170b97bc5110) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "winsnd.p1", 0x000000, 0x080000, CRC(a913ad0d) SHA1(5f39b661912da903ce8d6658b7848081b191ea56) ) \
	ROM_LOAD( "winsnd.p2", 0x080000, 0x080000, CRC(6a22b39f) SHA1(0e0dbeac4310e03490b665fff514392481ad265f) ) \

#define M4WTA_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4WTA_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4WTA_SET( 199?, m4wta,		0,		"wta55",									0x0000, 0x010000, CRC(df3e66cd) SHA1(68e769816cb1a71dea8a3ccf4636414c45c01646), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 1)" )
M4WTA_SET( 199?, m4wta__a,	m4wta,	"pwnk.p1",									0x0000, 0x020000, CRC(7c6e21e3) SHA1(d6aeb5948e0800050193575a3b5c06c11f46eed8), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 2)" )
M4WTA_SET( 199?, m4wta__b,	m4wta,	"windy.p1",									0x0000, 0x010000, CRC(d8b78c2d) SHA1(d8c2a2ac30a9b876acfbe99e3c540ba0e82cde33), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 3)" )
M4WTA_SET( 199?, m4wta__d,	m4wta,	"wins.p1",									0x0000, 0x010000, CRC(d79d1e5b) SHA1(722657423a605d6d272d61e4e00b4055ed05f98d), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 5)" )
M4WTA_SET( 199?, m4wta__e,	m4wta,	"winy.p1",									0x0000, 0x010000, CRC(5ff8ed08) SHA1(9567db64e8ebf25ecb22236598cc88a3106f0e36), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 6)" )
M4WTA_SET( 199?, m4wta__f,	m4wta,	"wn5ad.p1",									0x0000, 0x010000, CRC(0eb0845d) SHA1(57a2ca27672119e71af3b990cedcf52dd89e24cc), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 7)" )
M4WTA_SET( 199?, m4wta__g,	m4wta,	"wn5b.p1",									0x0000, 0x010000, CRC(82cefba2) SHA1(07753a5f0d455422f33495a6f050c8e16a92e087), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 8)" )
M4WTA_SET( 199?, m4wta__h,	m4wta,	"wn5bd.p1",									0x0000, 0x010000, CRC(19d25b26) SHA1(91459c87e95d9800c5f77fd0c7f72f8a1488dc37), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 9)" )
M4WTA_SET( 199?, m4wta__i,	m4wta,	"wn5d.p1",									0x0000, 0x010000, CRC(8a3d6bed) SHA1(a20f24cd5216976913c0405f54883d6080986867), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 10)" )
M4WTA_SET( 199?, m4wta__j,	m4wta,	"wn5dk.p1",									0x0000, 0x010000, CRC(1dfcb2bc) SHA1(b1a73a7758c3126f7b13156835c91a4900cbe6e0), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 11)" )
M4WTA_SET( 199?, m4wta__k,	m4wta,	"wn5dy.p1",									0x0000, 0x010000, CRC(d45e1db0) SHA1(2524c4b60a89ea0ca15cf999fbd1f8d9029dfbb6), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 12)" )
M4WTA_SET( 199?, m4wta__l,	m4wta,	"wn5k.p1",									0x0000, 0x010000, CRC(71c34cb4) SHA1(e1b96dd30d8ab680128d76886691d06fcd2d48c0), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 13)" )
M4WTA_SET( 199?, m4wta__m,	m4wta,	"wn5s.p1",									0x0000, 0x010000, CRC(f6e925c1) SHA1(963f06462c73300757aad2371df4ebe28afca521), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 14)" )
M4WTA_SET( 199?, m4wta__n,	m4wta,	"wn5y.p1",									0x0000, 0x010000, CRC(7155f8b5) SHA1(f55f88fd7b0144cb7b64640d529b179dd056f5ec), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 15)" )
M4WTA_SET( 199?, m4wta__o,	m4wta,	"wn8b.p1",									0x0000, 0x010000, CRC(7e84f99c) SHA1(bef41b3e7906bdaadfa5741e9ae40028f4fd360f), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 16)" )
M4WTA_SET( 199?, m4wta__p,	m4wta,	"wn8c.p1",									0x0000, 0x010000, CRC(471ba65a) SHA1(6ede860bcf323ee75dd7f75a51e5d1166ee72abc), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 17)" )
M4WTA_SET( 199?, m4wta__q,	m4wta,	"wn8d.p1",									0x0000, 0x010000, CRC(eb2bd01e) SHA1(df74f8eb8fa411bab20ab522fd7c511a1370fe90), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 18)" )
M4WTA_SET( 199?, m4wta__r,	m4wta,	"wn8dk.p1",									0x0000, 0x010000, CRC(ec20a0bc) SHA1(61b615165a6e77cd85e1fa6aeb955307ec48d1b6), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 19)" )
M4WTA_SET( 199?, m4wta__s,	m4wta,	"wn8dy.p1",									0x0000, 0x010000, CRC(d2a1513c) SHA1(e4d2ad88846cbb6b393d3615bf10e1dea01de219), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 20)" )
M4WTA_SET( 199?, m4wta__t,	m4wta,	"wn8k.p1",									0x0000, 0x010000, CRC(3e15c690) SHA1(2fc1cca91ac5cc9abeac112e4d60e8fd57b07b94), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 21)" )
M4WTA_SET( 199?, m4wta__u,	m4wta,	"wn8s.p1",									0x0000, 0x010000, CRC(5c5a0f31) SHA1(301e595141dd6eb9250d71e591780e15a7d36423), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 22)" )
M4WTA_SET( 199?, m4wta__v,	m4wta,	"wn8y.p1",									0x0000, 0x010000, CRC(993cee6a) SHA1(26b2d5d3aa3465f90fe74960f183b8580ea2fbb1), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 23)" )
M4WTA_SET( 199?, m4wta__w,	m4wta,	"wnta2010",									0x0000, 0x010000, CRC(5b08faf8) SHA1(f4657041562044e17febfe77ad1f849545dcdaec), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 24)" )
M4WTA_SET( 199?, m4wta__x,	m4wta,	"wntad.p1",									0x0000, 0x010000, CRC(8502766e) SHA1(2a47c8f8ce8711b30962c5e8ef9093bdd3543551), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 25)" )
M4WTA_SET( 199?, m4wta__y,	m4wta,	"wntb.p1",									0x0000, 0x010000, CRC(1e3159f0) SHA1(ab9d0e9e6731b40c66c358d98c6481f31d9a0b0c), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 26)" )
M4WTA_SET( 199?, m4wta__z,	m4wta,	"wntbd.p1",									0x0000, 0x010000, CRC(91cc8978) SHA1(570ad4092bb148106fb2600f1e22b6cb6f57002a), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 27)" )
M4WTA_SET( 199?, m4wta__0,	m4wta,	"wntd.p1",									0x0000, 0x010000, CRC(ad68d804) SHA1(f301d0d267dd0020903f06b67ee6494b71258c68), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 28)" )
M4WTA_SET( 199?, m4wta__1,	m4wta,	"wntdk.p1",									0x0000, 0x010000, CRC(3a6b65b8) SHA1(1da0448e53a45fa249c14b5655cd0dc957ebb646), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 29)" )
M4WTA_SET( 199?, m4wta__2,	m4wta,	"wntdy.p1",									0x0000, 0x010000, CRC(2420634f) SHA1(5c6e891c34a6e2b3a6acb3856c1554145bb24d0d), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 30)" )
M4WTA_SET( 199?, m4wta__3,	m4wta,	"wntk.p1",									0x0000, 0x010000, CRC(3d8d07c7) SHA1(4659e2459d956bbcf5ef2a605527317ccdafcccb), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 31)" )
M4WTA_SET( 199?, m4wta__4,	m4wta,	"wnts.p1",									0x0000, 0x010000, CRC(3a9b0878) SHA1(85e86cca1a3a079746cd4401767ba1d9fc31a938), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 32)" )
M4WTA_SET( 199?, m4wta__5,	m4wta,	"wnty.p1",									0x0000, 0x010000, CRC(edaa5ae7) SHA1(d24b9f37d75f13f16718374e48e6c003b0b3333f), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 33)" )
M4WTA_SET( 199?, m4wta__6,	m4wta,	"wta20p10.bin",								0x0000, 0x010000, CRC(c7f235b8) SHA1(a25f6f755140d70b0392985839b1729640cf5d5d), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 34)" )
M4WTA_SET( 199?, m4wta__7,	m4wta,	"wta510l",									0x0000, 0x010000, CRC(9ce140ae) SHA1(01d53a5da0161ac4ecc861309f645d6eb47b4af5), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 35)" )
M4WTA_SET( 199?, m4wta__8,	m4wta,	"wta58tl",									0x0000, 0x010000, CRC(7275e865) SHA1(d5550646a062609cfc052fab81c533ca69171875), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 36)" )
M4WTA_SET( 199?, m4wta__9,	m4wta,	"wta_5p_4c.bin",							0x0000, 0x010000, CRC(54c51976) SHA1(70cae1f931615b993ac6a9e7ce2e529ad6d27da8), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 37)" )
M4WTA_SET( 199?, m4wta__aa,	m4wta,	"wtall20a",									0x0000, 0x010000, CRC(b53c951e) SHA1(24f96d16852a4fbaf49fbdf29a26d15877f07b18), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 38)" )
M4WTA_SET( 199?, m4wta__ab,	m4wta,	"wt_05__4.1_1",								0x0000, 0x010000, CRC(5e05485e) SHA1(062f16ca92518f746f5410a2b9b551542e1a68e3), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 39)" )
M4WTA_SET( 199?, m4wta__ac,	m4wta,	"wt_05__5.3_1",								0x0000, 0x010000, CRC(8a289bbd) SHA1(8ae0858716ed6aa02f6b4f93fd367c7cee85d13a), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 40)" )
M4WTA_SET( 199?, m4wta__ad,	m4wta,	"wta5.10",									0x0000, 0x010000, CRC(c1ae8e9a) SHA1(66c0b200202386a10b96b7141517a52921266950), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 41)" )
M4WTA_SET( 199?, m4wta__ae,	m4wta,	"wta5.4",									0x0000, 0x010000, CRC(00c64637) SHA1(54214edb107b28852a1bd3e095787bf9241e4fe3), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 42)" )
M4WTA_SET( 199?, m4wta__af,	m4wta,	"wta5.5n",									0x0000, 0x010000, CRC(85eed9b5) SHA1(6a11ff6a031b788524d23018e3af44767176246a), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 43)" )
M4WTA_SET( 199?, m4wta__ag,	m4wta,	"wta5.8t",									0x0000, 0x010000, CRC(548122ab) SHA1(c611084e8a08d5556e458daf9cc721c0e5ba1948), "Barcrest","Winner Takes All (Barcrest) (MPU4) (set 44)" )


#define M4GOODTM_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "gtrsnd.p1", 0x000000, 0x080000, CRC(23317580) SHA1(c0c5244ddcf976211e2a5e5a0b1dbc6faaec22b4) ) \
	ROM_LOAD( "gtrsnd.p2", 0x080000, 0x080000, CRC(866ce0d2) SHA1(46e800c7364a6d291c6af87b30c680c530100e74) ) \

#define M4GOODTM_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4GOODTM_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4GOODTM_SET( 199?, m4goodtm,		0,			"gtr20d.p1",	0x0000, 0x020000, CRC(a19eaef1) SHA1(5e9f9cffd841b9d4f21175e3dcec7436d016bb19), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 1)" )
M4GOODTM_SET( 199?, m4goodtm__a,	m4goodtm,	"gta01ad.p1",	0x0000, 0x020000, CRC(2b556e66) SHA1(50a042fdb53294f74ab23a41a8a850dd14ad580d), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 2)" )
M4GOODTM_SET( 199?, m4goodtm__b,	m4goodtm,	"gta01b.p1",	0x0000, 0x020000, CRC(67dc4342) SHA1(bade42f329b4ab19e5802d8ac8b139486b05ac5a), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 3)" )
M4GOODTM_SET( 199?, m4goodtm__c,	m4goodtm,	"gta01bd.p1",	0x0000, 0x020000, CRC(6192c630) SHA1(f18c5042dc45add52b3bd0c28ad1574a85a9f3c4), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 4)" )
M4GOODTM_SET( 199?, m4goodtm__d,	m4goodtm,	"gta01d.p1",	0x0000, 0x020000, CRC(eac6ed87) SHA1(14e424b6a3232d751e5b800395b2962f997afb74), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 5)" )
M4GOODTM_SET( 199?, m4goodtm__e,	m4goodtm,	"gta01dh.p1",	0x0000, 0x020000, CRC(4201347b) SHA1(f0c086f15baa0f458f64a6d4ff7da297d9b53c8f), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 6)" )
M4GOODTM_SET( 199?, m4goodtm__f,	m4goodtm,	"gta01dk.p1",	0x0000, 0x020000, CRC(985ad557) SHA1(404a42f03d733ca6d89ec558ba6b5d815e8ce339), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 7)" )
M4GOODTM_SET( 199?, m4goodtm__g,	m4goodtm,	"gta01dr.p1",	0x0000, 0x020000, CRC(02b40bee) SHA1(a5bc063b0eac1689b13e9b523844aee105086c6b), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 8)" )
M4GOODTM_SET( 199?, m4goodtm__h,	m4goodtm,	"gta01dy.p1",	0x0000, 0x020000, CRC(acb64e98) SHA1(93d7252fe15e6a3db760b0218b4610cb5acbaca3), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 9)" )
M4GOODTM_SET( 199?, m4goodtm__i,	m4goodtm,	"gta01h.p1",	0x0000, 0x020000, CRC(444fb109) SHA1(ad04dbc67dade0012ce61718854f7656abd9d342), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 10)" )
M4GOODTM_SET( 199?, m4goodtm__j,	m4goodtm,	"gta01k.p1",	0x0000, 0x020000, CRC(9e145025) SHA1(d26b4aaee4d08d7470e862dc0e5a80c914025991), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 11)" )
M4GOODTM_SET( 199?, m4goodtm__k,	m4goodtm,	"gta01r.p1",	0x0000, 0x020000, CRC(04fa8e9c) SHA1(9ac94b59dcf8e4e123dd0f1f422d23698f1c8c38), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 12)" )
M4GOODTM_SET( 199?, m4goodtm__l,	m4goodtm,	"gta01s.p1",	0x0000, 0x020000, CRC(4340d9f6) SHA1(e9ccd419318bc3a3aba35a0104a98d1756b41731), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 13)" )
M4GOODTM_SET( 199?, m4goodtm__m,	m4goodtm,	"gta01y.p1",	0x0000, 0x020000, CRC(aaf8cbea) SHA1(a027a0a243538a6b417b3859db90adc64eeb4d31), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 14)" )
M4GOODTM_SET( 199?, m4goodtm__n,	m4goodtm,	"gtk02k.p1",	0x0000, 0x020000, CRC(a1665c5d) SHA1(056dcd9370df56129a65267fb70bbfac498f5a97), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 15)" )
M4GOODTM_SET( 199?, m4goodtm__o,	m4goodtm,	"gtr10ad.p1",	0x0000, 0x020000, CRC(b776f5dd) SHA1(9b6bf6b4d02e432ef411a9d2501c0dc7c5b551b1), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 16)" )
M4GOODTM_SET( 199?, m4goodtm__p,	m4goodtm,	"gtr10b.p1",	0x0000, 0x020000, CRC(3a0aee1d) SHA1(d7adf7118943ac946082b6c3687e10773f25608e), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 17)" )
M4GOODTM_SET( 199?, m4goodtm__q,	m4goodtm,	"gtr10bd.p1",	0x0000, 0x020000, CRC(fdb15d8b) SHA1(2542765c382cd84be1a1e7d6654e4668b1fc7fea), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 18)" )
M4GOODTM_SET( 199?, m4goodtm__r,	m4goodtm,	"gtr10d.p1",	0x0000, 0x020000, CRC(19991c56) SHA1(bed33027d8bc5dacc88d940ff3505be8186f5324), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 19)" )
M4GOODTM_SET( 199?, m4goodtm__s,	m4goodtm,	"gtr10dh.p1",	0x0000, 0x020000, CRC(80aa56fd) SHA1(f01edf442eee7dd78fbc934deb361d7e90c2025a), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 20)" )
M4GOODTM_SET( 199?, m4goodtm__t,	m4goodtm,	"gtr10dk.p1",	0x0000, 0x020000, CRC(04794eec) SHA1(3e2b95ed2092ad1c0242f8a6cb540de0c6f995a3), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 21)" )
M4GOODTM_SET( 199?, m4goodtm__u,	m4goodtm,	"gtr10dr.p1",	0x0000, 0x020000, CRC(9e979055) SHA1(b95111c4c986c4e9ceca9771ccb97fdeb67ffb02), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 22)" )
M4GOODTM_SET( 199?, m4goodtm__v,	m4goodtm,	"gtr10dy.p1",	0x0000, 0x020000, CRC(3095d523) SHA1(eed9cf874f13fe9576e8ba0da8296546311fff01), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 23)" )
M4GOODTM_SET( 199?, m4goodtm__w,	m4goodtm,	"gtr10h.p1",	0x0000, 0x020000, CRC(4711e56b) SHA1(3717b418758039286174594563281c21e4752eb5), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 24)" )
M4GOODTM_SET( 199?, m4goodtm__x,	m4goodtm,	"gtr10k.p1",	0x0000, 0x020000, CRC(c3c2fd7a) SHA1(d6e91fed1276c660cfe8e92c6902951803947a56), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 25)" )
M4GOODTM_SET( 199?, m4goodtm__y,	m4goodtm,	"gtr10r.p1",	0x0000, 0x020000, CRC(592c23c3) SHA1(17e7a22c9b80e8669cb46f691d56251bbf0717d0), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 26)" )
M4GOODTM_SET( 199?, m4goodtm__z,	m4goodtm,	"gtr10s.p1",	0x0000, 0x020000, CRC(f43fd459) SHA1(7247048088bd39cf8b20d96b1c08be48b005ac86), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 27)" )
M4GOODTM_SET( 199?, m4goodtm__0,	m4goodtm,	"gtr10y.p1",	0x0000, 0x020000, CRC(f72e66b5) SHA1(14e09a94e1cde77e87573ad7e0f438485f637dfc), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 28)" )
M4GOODTM_SET( 199?, m4goodtm__1,	m4goodtm,	"gtr11s",		0x0000, 0x020000, CRC(ff4bd1fb) SHA1(959a7975209e2d17c5b3e4adc72bd52bd3005035), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 29)" )
M4GOODTM_SET( 199?, m4goodtm__2,	m4goodtm,	"gtr15g",		0x0000, 0x020000, CRC(9da85042) SHA1(3148e654380f1bcca93c01a282f1c409e4f2d393), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 30)" )
M4GOODTM_SET( 199?, m4goodtm__3,	m4goodtm,	"gtr15t",		0x0000, 0x020000, CRC(581bcc5b) SHA1(cf80bba2b2e44c886b9bf6ae6ab1c83e2fbd7888), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 31)" )
M4GOODTM_SET( 199?, m4goodtm__4,	m4goodtm,	"gtr20ad.p1",	0x0000, 0x020000, CRC(38cf724f) SHA1(e58c02e5ca4ff0ecab41fe4597aa652ff8cc604f), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 32)" )
M4GOODTM_SET( 199?, m4goodtm__5,	m4goodtm,	"gtr20b.p1",	0x0000, 0x020000, CRC(820d5cba) SHA1(d297500c3a5388d7c9203fcb15778079e8671329), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 33)" )
M4GOODTM_SET( 199?, m4goodtm__6,	m4goodtm,	"gtr20bd.p1",	0x0000, 0x020000, CRC(7208da19) SHA1(c28b50eb91204a7494664b082ce57a910fdb29fd), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 34)" )
M4GOODTM_SET( 199?, m4goodtm__7,	m4goodtm,	"gtr20dh.p1",	0x0000, 0x020000, CRC(0f13d16f) SHA1(3a159982a3c5231d577848ca1cde17e8d84660a1), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 35)" )
M4GOODTM_SET( 199?, m4goodtm__8,	m4goodtm,	"gtr20dk.p1",	0x0000, 0x020000, CRC(8bc0c97e) SHA1(3b3eb7e44aa3f9ce19f6f612ea6bd24cc78630ff), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 36)" )
M4GOODTM_SET( 199?, m4goodtm__9,	m4goodtm,	"gtr20dr.p1",	0x0000, 0x020000, CRC(112e17c7) SHA1(519a62d61a0aa8dc2433369bd74f62f61a85994c), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 37)" )
M4GOODTM_SET( 199?, m4goodtm__aa,	m4goodtm,	"gtr20dy.p1",	0x0000, 0x020000, CRC(bf2c52b1) SHA1(c8f97b56e9782b23c0ca8795ae2663c89660225e), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 38)" )
M4GOODTM_SET( 199?, m4goodtm__ab,	m4goodtm,	"gtr20h.p1",	0x0000, 0x020000, CRC(ff1657cc) SHA1(07dfbc8c2b2d8fb7001ef51d80ef46562111d3ac), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 39)" )
M4GOODTM_SET( 199?, m4goodtm__ac,	m4goodtm,	"gtr20k.p1",	0x0000, 0x020000, CRC(7bc54fdd) SHA1(287c1a403429bae1cc3a4ba9dfe9b510777a64b1), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 40)" )
M4GOODTM_SET( 199?, m4goodtm__ad,	m4goodtm,	"gtr20r.p1",	0x0000, 0x020000, CRC(e12b9164) SHA1(a54993c4f9ffc08c03905e6f50e499eba13db0d6), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 41)" )
M4GOODTM_SET( 199?, m4goodtm__ae,	m4goodtm,	"gtr20s.p1",	0x0000, 0x020000, CRC(91d2632d) SHA1(b8a7ef106a16e0526626cd69e82d07616d5c07d9), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 42)" )
M4GOODTM_SET( 199?, m4goodtm__af,	m4goodtm,	"gtr20y.p1",	0x0000, 0x020000, CRC(4f29d412) SHA1(c6a72e6fa7daaa6d8622936d10ae745814f4a8b7), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 43)" )
M4GOODTM_SET( 199?, m4goodtm__ag,	m4goodtm,	"gts01ad.p1",	0x0000, 0x020000, CRC(b415d3f3) SHA1(a22d4cbce9b66049c000806b565c86cd3d91fd82), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 44)" )
M4GOODTM_SET( 199?, m4goodtm__ah,	m4goodtm,	"gts01b.p1",	0x0000, 0x020000, CRC(a05ea188) SHA1(5b6acadaf8b0d18b9c9daaf4aea45202cad13355), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 45)" )
M4GOODTM_SET( 199?, m4goodtm__ai,	m4goodtm,	"gts01bd.p1",	0x0000, 0x020000, CRC(fed27ba5) SHA1(14f6fd7c3e964c3e6547a4bf5fa5d7cd802f715d), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 46)" )
M4GOODTM_SET( 199?, m4goodtm__aj,	m4goodtm,	"gts01d.p1",	0x0000, 0x020000, CRC(2d440f4d) SHA1(09881d8aede3a2011010022ee0e03ba8f02ea907), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 47)" )
M4GOODTM_SET( 199?, m4goodtm__ak,	m4goodtm,	"gts01dh.p1",	0x0000, 0x020000, CRC(dd4189ee) SHA1(f68da5f688ffa08625c92fd139bef263c43f674a), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 48)" )
M4GOODTM_SET( 199?, m4goodtm__al,	m4goodtm,	"gts01dk.p1",	0x0000, 0x020000, CRC(071a68c2) SHA1(ea2052a389df09e0d5614106cc20ee0eccd339dd), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 49)" )
M4GOODTM_SET( 199?, m4goodtm__am,	m4goodtm,	"gts01dr.p1",	0x0000, 0x020000, CRC(9df4b67b) SHA1(608de0bb5280ac20c8c47eb22f8a575ad87a4fdd), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 50)" )
M4GOODTM_SET( 199?, m4goodtm__an,	m4goodtm,	"gts01dy.p1",	0x0000, 0x020000, CRC(33f6f30d) SHA1(aae905d0d278c38497513eb131a7cdaab4d22a85), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 51)" )
M4GOODTM_SET( 199?, m4goodtm__ao,	m4goodtm,	"gts01h.p1",	0x0000, 0x020000, CRC(83cd53c3) SHA1(a4233e22d0bf53b3ddc4c951894a653e7951f5ad), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 52)" )
M4GOODTM_SET( 199?, m4goodtm__ap,	m4goodtm,	"gts01k.p1",	0x0000, 0x020000, CRC(5996b2ef) SHA1(874d69a4adc31667ea177b3a0350b77bb735ea8d), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 53)" )
M4GOODTM_SET( 199?, m4goodtm__aq,	m4goodtm,	"gts01r.p1",	0x0000, 0x020000, CRC(c3786c56) SHA1(c9659df6302046c466e6447b8b427628c88b773d), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 54)" )
M4GOODTM_SET( 199?, m4goodtm__ar,	m4goodtm,	"gts01s.p1",	0x0000, 0x020000, CRC(b3819e1f) SHA1(fb44500e06b8a6b09e6b707ee8c0cfe7844870a2), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 55)" )
M4GOODTM_SET( 199?, m4goodtm__as,	m4goodtm,	"gts01y.p1",	0x0000, 0x020000, CRC(6d7a2920) SHA1(7d31087e3645e05baf6b0100966d4773a6d023cd), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 56)" )
M4GOODTM_SET( 199?, m4goodtm__at,	m4goodtm,	"gts02s.t",		0x0000, 0x020000, CRC(e4f5ebcc) SHA1(3e070628375db980583c3b38e2676d73fbeaae68), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 57)" )
M4GOODTM_SET( 199?, m4goodtm__au,	m4goodtm,	"gts10ad.p1",	0x0000, 0x020000, CRC(b754490c) SHA1(26811ae53b3ee8ae0a381604109f0f77f096e2c6), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 58)" )
M4GOODTM_SET( 199?, m4goodtm__av,	m4goodtm,	"gts10b.p1",	0x0000, 0x020000, CRC(21410553) SHA1(2060269ab6dc9375b6e7101b3944305fcd4b6d12), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 59)" )
M4GOODTM_SET( 199?, m4goodtm__aw,	m4goodtm,	"gts10bd.p1",	0x0000, 0x020000, CRC(fd93e15a) SHA1(b9574f34f1e7a92cc75d6aed8914f94ad661bba3), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 60)" )
M4GOODTM_SET( 199?, m4goodtm__ax,	m4goodtm,	"gts10d.p1",	0x0000, 0x020000, CRC(ac5bab96) SHA1(2b4ab1f096a5d63d5688228debb25b392b94a297), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 61)" )
M4GOODTM_SET( 199?, m4goodtm__ay,	m4goodtm,	"gts10dh.p1",	0x0000, 0x020000, CRC(de001311) SHA1(a096937e1f2fd2f9046c0ea2363805112da76c95), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 62)" )
M4GOODTM_SET( 199?, m4goodtm__az,	m4goodtm,	"gts10dk.p1",	0x0000, 0x020000, CRC(045bf23d) SHA1(dd5567d4c07fba7ff1cc257db287e1a82d9b930a), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 63)" )
M4GOODTM_SET( 199?, m4goodtm__a0,	m4goodtm,	"gts10dr.p1",	0x0000, 0x020000, CRC(9eb52c84) SHA1(385b09e424863f3778605c6e768c9b1068eae66e), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 64)" )
M4GOODTM_SET( 199?, m4goodtm__a1,	m4goodtm,	"gts10dy.p1",	0x0000, 0x020000, CRC(30b769f2) SHA1(7cf1e66b992faf48b1fed39a005469e51d471a0b), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 65)" )
M4GOODTM_SET( 199?, m4goodtm__a2,	m4goodtm,	"gts10h.p1",	0x0000, 0x020000, CRC(02d2f718) SHA1(9f46ef4cc7d08c42cf617b471e599ca21b4cd72f), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 66)" )
M4GOODTM_SET( 199?, m4goodtm__a3,	m4goodtm,	"gts10k.p1",	0x0000, 0x020000, CRC(d8891634) SHA1(0019d2b3dd1c59d37d9c13e912907a55f2a9ca5f), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 67)" )
M4GOODTM_SET( 199?, m4goodtm__a4,	m4goodtm,	"gts10r.p1",	0x0000, 0x020000, CRC(4267c88d) SHA1(22047782c384caeb9cf2de69dcdd05f42ee137ad), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 68)" )
M4GOODTM_SET( 199?, m4goodtm__a5,	m4goodtm,	"gts10s.p1",	0x0000, 0x020000, CRC(2851ba23) SHA1(7597a2df22aa0e2670be2d5bb2407ea1feace3a0), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 69)" )
M4GOODTM_SET( 199?, m4goodtm__a6,	m4goodtm,	"gts10y.p1",	0x0000, 0x020000, CRC(ec658dfb) SHA1(a9d8ba1b66811ccd336892160d47e5d42eb83a23), "Barcrest","Let The Good Times Roll (Barcrest) (MPU4) (set 70)" )


#define M4JPGEM_EXTRA_ROMS \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) \
	ROM_LOAD( "cg4snd.p1", 0x000000, 0x080000, CRC(e4addde8) SHA1(6b84de51cc5195d551e0787ff92bfa4371ab27a3) ) \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "jagsnd.p1", 0x080000, 0x080000, CRC(7488f7a7) SHA1(d581e9d6b5052ee8fee353a83e9d9031443d060a) ) \


#define M4JPGEM_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JPGEM_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \

M4JPGEM_SET( 199?, m4jpgem,		0,			"cg4ad.p1",		0x0000, 0x010000, CRC(417c98c1) SHA1(2ce23e27742c418d5ebaa0f4f0597e29955ea57d), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 1)" )
M4JPGEM_SET( 199?, m4jpgem__a,	m4jpgem,	"cg4b.p1",		0x0000, 0x010000, CRC(c57cca63) SHA1(80a440912362d55cac6bc77b6ff6d6672af378c6), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 2)" )
M4JPGEM_SET( 199?, m4jpgem__b,	m4jpgem,	"cg4bd.p1",		0x0000, 0x010000, CRC(7604ea50) SHA1(3d6eee763bd21119ab52a2388229da076caf78a4), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 3)" )
M4JPGEM_SET( 199?, m4jpgem__c,	m4jpgem,	"cg4d.p1",		0x0000, 0x010000, CRC(87ea1087) SHA1(47f7c17fa3611745c881669ff50559e4b4386fd9), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 4)" )
M4JPGEM_SET( 199?, m4jpgem__d,	m4jpgem,	"cg4dk.p1",		0x0000, 0x010000, CRC(230284fb) SHA1(39ab2abdd8d3af4818e4e3738529f020055ba659), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 5)" )
M4JPGEM_SET( 199?, m4jpgem__e,	m4jpgem,	"cg4dy.p1",		0x0000, 0x010000, CRC(7d02342d) SHA1(097c9c9dc84bd00f1ddd64b1f9564f0cf7a9023f), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 6)" )
M4JPGEM_SET( 199?, m4jpgem__f,	m4jpgem,	"cg4k.p1",		0x0000, 0x010000, CRC(ba4ef5a8) SHA1(1673985aee634aa5c8129cc1239ce08fb9f5da2c), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 7)" )
M4JPGEM_SET( 199?, m4jpgem__g,	m4jpgem,	"cg4s.p1",		0x0000, 0x010000, CRC(f25eba0b) SHA1(250189b7fb8aa82a8696c3a0099eb13ec74eeb10), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 8)" )
M4JPGEM_SET( 199?, m4jpgem__h,	m4jpgem,	"cg4y.p1",		0x0000, 0x010000, CRC(237098d3) SHA1(9f54ed0d9ce37f3b4e6dca136fe4a12ba79c89f9), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 9)" )
M4JPGEM_SET( 199?, m4jpgem__i,	m4jpgem,	"cgt03ad.p1",	0x0000, 0x010000, CRC(88842c4a) SHA1(c86987b44f04cf28a6f68300e4345f635455d4bf), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 10)" )
M4JPGEM_SET( 199?, m4jpgem__j,	m4jpgem,	"cgt03b.p1",	0x0000, 0x010000, CRC(99634ce1) SHA1(9fe867b0619070f563fb72b4415e4a9263c808e7), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 11)" )
M4JPGEM_SET( 199?, m4jpgem__k,	m4jpgem,	"cgt03bd.p1",	0x0000, 0x010000, CRC(be984100) SHA1(dfa7d97f02dc988b7743a1f57ab08c406f712559), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 12)" )
M4JPGEM_SET( 199?, m4jpgem__l,	m4jpgem,	"cgt03d.p1",	0x0000, 0x010000, CRC(aba3a305) SHA1(9a0203f830a0a8c6013eb5824bd48373c589dcb5), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 13)" )
M4JPGEM_SET( 199?, m4jpgem__m,	m4jpgem,	"cgt03dk.p1",	0x0000, 0x010000, CRC(be9292b0) SHA1(0d7944ac647c8fd92530389d61f5c1eec0d2c8d1), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 14)" )
M4JPGEM_SET( 199?, m4jpgem__n,	m4jpgem,	"cgt03dr.p1",	0x0000, 0x010000, CRC(935fa628) SHA1(5dd93fd27d2e15606ba22bada1ecff85c4f4a8c3), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 15)" )
M4JPGEM_SET( 199?, m4jpgem__o,	m4jpgem,	"cgt03dy.p1",	0x0000, 0x010000, CRC(b83879b0) SHA1(b0664e1bd97b76b73c96a9e0d20d1a15707863ff), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 16)" )
M4JPGEM_SET( 199?, m4jpgem__p,	m4jpgem,	"cgt03k.p1",	0x0000, 0x010000, CRC(451a8f66) SHA1(e218db61fdaca6824abebe59ec7f8d0f595e2cfa), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 17)" )
M4JPGEM_SET( 199?, m4jpgem__q,	m4jpgem,	"cgt03r.p1",	0x0000, 0x010000, CRC(85dd3733) SHA1(10b8c4d147d4b534ce31394d5ba69806b83a297e), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 18)" )
M4JPGEM_SET( 199?, m4jpgem__r,	m4jpgem,	"cgt03s.p1",	0x0000, 0x010000, CRC(b516cbcd) SHA1(c04d32818f9f8772b2a945cf40075ce7844b936e), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 19)" )
M4JPGEM_SET( 199?, m4jpgem__s,	m4jpgem,	"cgt03y.p1",	0x0000, 0x010000, CRC(57937087) SHA1(489bcbe5598020c24357f4c7b4e9096bc6332aa3), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 20)" )
M4JPGEM_SET( 199?, m4jpgem__t,	m4jpgem,	"cgts.p1",		0x0000, 0x010000, CRC(2a6f4489) SHA1(e410dd49cca50b3c051815a1b4be4bf2dc55f1af), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 21)" )
M4JPGEM_SET( 199?, m4jpgem__u,	m4jpgem,	"cgu02ad.p1",	0x0000, 0x010000, CRC(eee268a6) SHA1(ebc0d1e14ff27c5497b7c4e90e6fafa58916c83b), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 22)" )
M4JPGEM_SET( 199?, m4jpgem__v,	m4jpgem,	"cgu02b.p1",	0x0000, 0x010000, CRC(7d05d069) SHA1(2a94b121528bf39939f5a8b36318c0073171997d), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 23)" )
M4JPGEM_SET( 199?, m4jpgem__w,	m4jpgem,	"cgu02bd.p1",	0x0000, 0x010000, CRC(d8fe05ec) SHA1(7e2de5c6ece6779d09daf23f3ab4b61817fad103), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 24)" )
M4JPGEM_SET( 199?, m4jpgem__x,	m4jpgem,	"cgu02d.p1",	0x0000, 0x010000, CRC(daaf1fe1) SHA1(f2606c454e191166d217c5f5c82e91794977384b), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 25)" )
M4JPGEM_SET( 199?, m4jpgem__y,	m4jpgem,	"cgu02dk.p1",	0x0000, 0x010000, CRC(0487c66b) SHA1(3be30181590e5f5d2181bc76da5fd49fe9796006), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 26)" )
M4JPGEM_SET( 199?, m4jpgem__z,	m4jpgem,	"cgu02dr.p1",	0x0000, 0x010000, CRC(68655b09) SHA1(df40c058172d960f4f9393343cf9271fc52c58c8), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 27)" )
M4JPGEM_SET( 199?, m4jpgem__0,	m4jpgem,	"cgu02dy.p1",	0x0000, 0x010000, CRC(5f1709d1) SHA1(36ae3cd57e5db956b8ef362043d5c63aea0da06a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 28)" )
M4JPGEM_SET( 199?, m4jpgem__1,	m4jpgem,	"cgu02k.p1",	0x0000, 0x010000, CRC(90058f14) SHA1(0e73410253e422ff2d4182b034624ab8dd996cb8), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 29)" )
M4JPGEM_SET( 199?, m4jpgem__2,	m4jpgem,	"cgu02r.p1",	0x0000, 0x010000, CRC(8f1d071b) SHA1(caa05465a12ca7ab6df0dce458caefb40dad818a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 30)" )
M4JPGEM_SET( 199?, m4jpgem__3,	m4jpgem,	"cgu02s.p1",	0x0000, 0x010000, CRC(1cff0517) SHA1(162651a1af6273ea49490d0809a30ee9b13c728e), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 31)" )
M4JPGEM_SET( 199?, m4jpgem__4,	m4jpgem,	"cgu02y.p1",	0x0000, 0x010000, CRC(a2468782) SHA1(5f9161cffc6d9ffe8c30c41434ab012c16a48dfd), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 32)" )
M4JPGEM_SET( 199?, m4jpgem__5,	m4jpgem,	"jagb.p1",		0x0000, 0x010000, CRC(75b9a4b6) SHA1(ecade0921cd535ee7f1b67767fa7d5ab3cd45b2c), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 33)" )
M4JPGEM_SET( 199?, m4jpgem__6,	m4jpgem,	"jagd.p1",		0x0000, 0x010000, CRC(c7546004) SHA1(31bdbd6b681a3a2b13f380f2807691c0b0fec83e), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 34)" )
M4JPGEM_SET( 199?, m4jpgem__7,	m4jpgem,	"jagdk.p1",		0x0000, 0x010000, CRC(313f7a1f) SHA1(358a33878ca70f2bcdb1d5d79c39e357586ebe8b), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 35)" )
M4JPGEM_SET( 199?, m4jpgem__8,	m4jpgem,	"jagdy.p1",		0x0000, 0x010000, CRC(d105a41e) SHA1(365e382683362c815461801753fb03e2f084de65), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 36)" )
M4JPGEM_SET( 199?, m4jpgem__9,	m4jpgem,	"jags.p1",		0x0000, 0x010000, CRC(dd93f084) SHA1(5cb25b3beb6d7a7b83227a6bb8382cfbcc285887), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 37)" )
M4JPGEM_SET( 199?, m4jpgem__aa,	m4jpgem,	"jagy.p1",		0x0000, 0x010000, CRC(08d510ca) SHA1(b79c9fe8dc17152f3e8c601c27515beff1d67219), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 38)" )
M4JPGEM_SET( 199?, m4jpgem__ab,	m4jpgem,	"jg3ad.p1",		0x0000, 0x010000, CRC(501bb879) SHA1(a97519042b4a4ed03efbcad9f11f279184dec847), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 39)" )
M4JPGEM_SET( 199?, m4jpgem__ac,	m4jpgem,	"jg3b.p1",		0x0000, 0x010000, CRC(e568ae84) SHA1(9126f9b45633e7eb44626aa0ab40784c62870c8a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 40)" )
M4JPGEM_SET( 199?, m4jpgem__ad,	m4jpgem,	"jg3bd.p1",		0x0000, 0x010000, CRC(435d5d28) SHA1(1ea48323f48edc20ce1c28e4e7080e0824e73d3c), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 41)" )
M4JPGEM_SET( 199?, m4jpgem__ae,	m4jpgem,	"jg3d.p1",		0x0000, 0x010000, CRC(774f9d41) SHA1(c99e9a46b1216f430007b5ebbf942899b5e691f9), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 42)" )
M4JPGEM_SET( 199?, m4jpgem__af,	m4jpgem,	"jg3dk.p1",		0x0000, 0x010000, CRC(c422e514) SHA1(172b25bf75a529b555e328cef77a3340609d818b), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 43)" )
M4JPGEM_SET( 199?, m4jpgem__ag,	m4jpgem,	"jg3dy.p1",		0x0000, 0x010000, CRC(5d1c886f) SHA1(b49ab97ba6cdc810e7baa520ffad25f54c0d8412), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 44)" )
M4JPGEM_SET( 199?, m4jpgem__ah,	m4jpgem,	"jg3k.p1",		0x0000, 0x010000, CRC(8e7985ae) SHA1(8c8de22aab2508b2317d5edde779d54ebe67ac92), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 45)" )
M4JPGEM_SET( 199?, m4jpgem__ai,	m4jpgem,	"jg3s.p1",		0x0000, 0x010000, CRC(91945adc) SHA1(d80321fc4c2e67461d69df2164e3e290caa905bc), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 46)" )
M4JPGEM_SET( 199?, m4jpgem__aj,	m4jpgem,	"jg3y.p1",		0x0000, 0x010000, CRC(bf96ad55) SHA1(48d828398f32c3ddfafeb84cfd777f8e668df1b3), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 47)" )
M4JPGEM_SET( 199?, m4jpgem__ak,	m4jpgem,	"jg8b.p1",		0x0000, 0x010000, CRC(f2e3d009) SHA1(90c85f9a300d157d560b08ccabfe79f826780d74), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 48)" )
M4JPGEM_SET( 199?, m4jpgem__al,	m4jpgem,	"jg8c.p1",		0x0000, 0x010000, CRC(cc24cf15) SHA1(0c4c28633f33c78570f5da17c64c2e90bf3d5cd0), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 49)" )
M4JPGEM_SET( 199?, m4jpgem__am,	m4jpgem,	"jg8d.p1",		0x0000, 0x010000, CRC(58eff94c) SHA1(9acde535ad808789233876dd8076c03a8d56a9e7), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 50)" )
M4JPGEM_SET( 199?, m4jpgem__an,	m4jpgem,	"jg8db.p1",		0x0000, 0x010000, CRC(3006a36a) SHA1(37297cae02c1fd5308ba9935537b35c565374a07), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 51)" )
M4JPGEM_SET( 199?, m4jpgem__ao,	m4jpgem,	"jg8dk.p1",		0x0000, 0x010000, CRC(199401d7) SHA1(33eef070e437386c7ad0d834b40353047f1a6a6f), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 52)" )
M4JPGEM_SET( 199?, m4jpgem__ap,	m4jpgem,	"jg8dy.p1",		0x0000, 0x010000, CRC(ead58bed) SHA1(cd0e151c843f5268edddb2f82555201deccac65a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 53)" )
M4JPGEM_SET( 199?, m4jpgem__aq,	m4jpgem,	"jg8k.p1",		0x0000, 0x010000, CRC(f5b14363) SHA1(f0ace838cc0d0c262006bb514eff75903d92d679), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 54)" )
M4JPGEM_SET( 199?, m4jpgem__ar,	m4jpgem,	"jg8s.p1",		0x0000, 0x010000, CRC(8cdd650a) SHA1(c4cb87513f0d7986e158b3c5ab1f034c8ba933a9), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 55)" )
M4JPGEM_SET( 199?, m4jpgem__as,	m4jpgem,	"jgtad.p1",		0x0000, 0x010000, CRC(90e10b6c) SHA1(548c7537829ca9395cac460ccf76e0d566898e44), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 56)" )
M4JPGEM_SET( 199?, m4jpgem__at,	m4jpgem,	"jgtb.p1",		0x0000, 0x010000, CRC(5f343a43) SHA1(033824e93b1fcd2f7c5f27a573a728747ef7b21a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 57)" )
M4JPGEM_SET( 199?, m4jpgem__au,	m4jpgem,	"jgtbd.p1",		0x0000, 0x010000, CRC(50ba2771) SHA1(f487ed2eeff0369e3fa718de68e3ba4912fd7576), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 58)" )
M4JPGEM_SET( 199?, m4jpgem__av,	m4jpgem,	"jgtd.p1",		0x0000, 0x010000, CRC(2625da4a) SHA1(b1f9d22a46bf20283c5735fce5768d9cef299f59), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 59)" )
M4JPGEM_SET( 199?, m4jpgem__aw,	m4jpgem,	"jgtdk.p1",		0x0000, 0x010000, CRC(94220901) SHA1(f62c9a59bb419e98f7de358f7fee072b08aab3f5), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 60)" )
M4JPGEM_SET( 199?, m4jpgem__ax,	m4jpgem,	"jgtdr.p1",		0x0000, 0x010000, CRC(5011d1e3) SHA1(85e5d28d26449a951704698a4419cd2c0f7dd9c4), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 61)" )
M4JPGEM_SET( 199?, m4jpgem__ay,	m4jpgem,	"jgtdy.p1",		0x0000, 0x010000, CRC(6397e38c) SHA1(ed0c165a5ab27524374c540fd9bdcfd41ce8096c), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 62)" )
M4JPGEM_SET( 199?, m4jpgem__az,	m4jpgem,	"jgtk.p1",		0x0000, 0x010000, CRC(cb30d644) SHA1(751fc5c7ae07e64c07a3e89e74ace09dd2b99a02), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 63)" )
M4JPGEM_SET( 199?, m4jpgem__a0,	m4jpgem,	"jgtr.p1",		0x0000, 0x010000, CRC(6224a93d) SHA1(6d36b64c2eaddf122a6a7e798b5efb44ec2e5b45), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 64)" )
M4JPGEM_SET( 199?, m4jpgem__a1,	m4jpgem,	"jgts.p1",		0x0000, 0x010000, CRC(0e3810a7) SHA1(cf840bd84eba65d9dec2d6821a48112b6f2f9bca), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 65)" )
M4JPGEM_SET( 199?, m4jpgem__a2,	m4jpgem,	"jgty.p1",		0x0000, 0x010000, CRC(84830d1f) SHA1(a4184a5bd08393c35f22bc05315377bff74f666c), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 66)" )
M4JPGEM_SET( 199?, m4jpgem__a3,	m4jpgem,	"jgu02ad.p1",	0x0000, 0x010000, CRC(ccec7d40) SHA1(75cc1a0dfda9592e35c24c030e04a768871a9e41), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 67)" )
M4JPGEM_SET( 199?, m4jpgem__a4,	m4jpgem,	"jgu02b.p1",	0x0000, 0x010000, CRC(daf0ebe3) SHA1(2e73f7b8171c0be7d06bf6da22e0395d5241b043), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 68)" )
M4JPGEM_SET( 199?, m4jpgem__a5,	m4jpgem,	"jgu02bd.p1",	0x0000, 0x010000, CRC(faf0100a) SHA1(c97b8eadfd473650ec497c7caa98e8efc59ecb6f), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 69)" )
M4JPGEM_SET( 199?, m4jpgem__a6,	m4jpgem,	"jgu02d.p1",	0x0000, 0x010000, CRC(b46e3f66) SHA1(1ede9d794effbc8cc9f097a06b7df4023d3d47ba), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 70)" )
M4JPGEM_SET( 199?, m4jpgem__a7,	m4jpgem,	"jgu02dk.p1",	0x0000, 0x010000, CRC(cdbf0041) SHA1(fb90d4f8112e169dab16f78fdea9d1b5306e05d6), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 71)" )
M4JPGEM_SET( 199?, m4jpgem__a8,	m4jpgem,	"jgu02dr.p1",	0x0000, 0x010000, CRC(41f1f723) SHA1(96623358e6dc450dbdc769d176703917f67e767a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 72)" )
M4JPGEM_SET( 199?, m4jpgem__a9,	m4jpgem,	"jgu02dy.p1",	0x0000, 0x010000, CRC(2023388d) SHA1(c9f1abaa12c78ac61304966b46044b82ea2ea3ff), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 73)" )
M4JPGEM_SET( 199?, m4jpgem__ba,	m4jpgem,	"jgu02k.p1",	0x0000, 0x010000, CRC(615029e8) SHA1(aecba0fad8c74fef9a4d04e95df961432ac999b7), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 74)" )
M4JPGEM_SET( 199?, m4jpgem__bb,	m4jpgem,	"jgu02r.p1",	0x0000, 0x010000, CRC(4bc55daa) SHA1(996f23bd66a4ef6ad8f77a28dc6ee67d9a293248), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 75)" )
M4JPGEM_SET( 199?, m4jpgem__bc,	m4jpgem,	"jgu02s.p1",	0x0000, 0x010000, CRC(f8abd287) SHA1(906d2817f73ea21cf830b0bd9a1938d344cc0341), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 76)" )
M4JPGEM_SET( 199?, m4jpgem__bd,	m4jpgem,	"jgu02y.p1",	0x0000, 0x010000, CRC(9b4325a8) SHA1(3d7c54691ed4d596acacec97e452a66b324957db), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 77)" )
M4JPGEM_SET( 199?, m4jpgem__be,	m4jpgem,	"rrh01ad.p1",	0x0000, 0x010000, CRC(d4f21930) SHA1(cba034b42a3587c0e173bc06d80142d7e494c849), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 78)" )
M4JPGEM_SET( 199?, m4jpgem__bf,	m4jpgem,	"rrh01b.p1",	0x0000, 0x010000, CRC(c85f9099) SHA1(f3c8f79c2e0cc58024202564761f4935f5d241b1), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 79)" )
M4JPGEM_SET( 199?, m4jpgem__bg,	m4jpgem,	"rrh01bd.p1",	0x0000, 0x010000, CRC(e2ee747a) SHA1(7f6cb93e3cbe4a2dd97d1ad15d17fa4f2f0a4b12), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 80)" )
M4JPGEM_SET( 199?, m4jpgem__bh,	m4jpgem,	"rrh01c.p1",	0x0000, 0x010000, CRC(00dfced2) SHA1(c497cb9835dca0d67f5ec6b6b1321a7b92612c9a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 81)" )
M4JPGEM_SET( 199?, m4jpgem__bi,	m4jpgem,	"rrh01d.p1",	0x0000, 0x010000, CRC(aef7ddbd) SHA1(8db2f1dbc11af7ef4357a90a77838c01588f8108), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 82)" )
M4JPGEM_SET( 199?, m4jpgem__bj,	m4jpgem,	"rrh01dk.p1",	0x0000, 0x010000, CRC(56206f0a) SHA1(3bdb5824ab6d748eb83b56f08cf2d1074a94b38a), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 83)" )
M4JPGEM_SET( 199?, m4jpgem__bk,	m4jpgem,	"rrh01dr.p1",	0x0000, 0x010000, CRC(8f622573) SHA1(9bbfcb2f3cf5f6edd2c6222b25c4971a59d2c235), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 84)" )
M4JPGEM_SET( 199?, m4jpgem__bl,	m4jpgem,	"rrh01dy.p1",	0x0000, 0x010000, CRC(d89136fd) SHA1(40fd0978bc76d81bfb5dc2f1e4a0c1c95b7c4e00), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 85)" )
M4JPGEM_SET( 199?, m4jpgem__bm,	m4jpgem,	"rrh01k.p1",	0x0000, 0x010000, CRC(da4a08c9) SHA1(3a86c0a543a7192680663b465ddfd1fa338cfec5), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 86)" )
M4JPGEM_SET( 199?, m4jpgem__bn,	m4jpgem,	"rrh01r.p1",	0x0000, 0x010000, CRC(fb45c547) SHA1(8d9c35c47c0f03c9dc6727fc5f952d64e25336f7), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 87)" )
M4JPGEM_SET( 199?, m4jpgem__bo,	m4jpgem,	"rrh01s.p1",	0x0000, 0x010000, CRC(dea2f376) SHA1(92f43c75950553d9b76af8179192d106de95fc03), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 88)" )
M4JPGEM_SET( 199?, m4jpgem__bp,	m4jpgem,	"rrh01y.p1",	0x0000, 0x010000, CRC(27014453) SHA1(febc118fcb8f048806237b38958c02d02b9f2874), "Barcrest","Jackpot Gems (Barcrest) (MPU4) (set 89)" )

#define M4JPGEMC_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing? */ \


#define M4JPGEMC_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JPGEMC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4JPGEMC_SET( 199?, m4jpgemc,		0,			"gtc01ad.p1",	0x0000, 0x010000, CRC(e4f61afd) SHA1(36e007275cce0565c50b150dba4c8df272cd4c2e), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 1)" )
M4JPGEMC_SET( 199?, m4jpgemc__a,	m4jpgemc,	"gtc01b.p1",	0x0000, 0x010000, CRC(e4e27c71) SHA1(b46da3f00134d3a2f17ceb35529adb598c75ee4e), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 2)" )
M4JPGEMC_SET( 199?, m4jpgemc__b,	m4jpgemc,	"gtc01bd.p1",	0x0000, 0x010000, CRC(d2ea77b7) SHA1(4f66fa8d692f26ffa92ae3aff4f43257fc573e93), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 3)" )
M4JPGEMC_SET( 199?, m4jpgemc__c,	m4jpgemc,	"gtc01c.p1",	0x0000, 0x010000, CRC(21c4c4f7) SHA1(f8a2de8453c095db80ff19018a72b15b949bace9), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 4)" )
M4JPGEMC_SET( 199?, m4jpgemc__d,	m4jpgemc,	"gtc01d.p1",	0x0000, 0x010000, CRC(b6a3d2c3) SHA1(48cbd3cf14b8f8e9ecbee1f351e781506ca6c17f), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 5)" )
M4JPGEMC_SET( 199?, m4jpgemc__e,	m4jpgemc,	"gtc01dk.p1",	0x0000, 0x010000, CRC(70cf4790) SHA1(b6aac10fd9ad3aafa277e6de58db3f1a28501529), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 6)" )
M4JPGEMC_SET( 199?, m4jpgemc__f,	m4jpgemc,	"gtc01dr.p1",	0x0000, 0x010000, CRC(4cb11885) SHA1(288da6617868f7d082fc72f50c13671fdaf9442a), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 7)" )
M4JPGEMC_SET( 199?, m4jpgemc__g,	m4jpgemc,	"gtc01dy.p1",	0x0000, 0x010000, CRC(713dec4a) SHA1(3cb1e3f5299a5145addaa677022e7d9a164072d9), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 8)" )
M4JPGEMC_SET( 199?, m4jpgemc__h,	m4jpgemc,	"gtc01k.p1",	0x0000, 0x010000, CRC(fb5102ec) SHA1(36c9c50c8266707542b00cfc55f57ec454401f70), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 9)" )
M4JPGEMC_SET( 199?, m4jpgemc__i,	m4jpgemc,	"gtc01r.p1",	0x0000, 0x010000, CRC(e311ca39) SHA1(602aee41400793f46f47ac9c8a9e6ce7f2d5f203), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 10)" )
M4JPGEMC_SET( 199?, m4jpgemc__j,	m4jpgemc,	"gtc01s.p1",	0x0000, 0x010000, CRC(af33337b) SHA1(97d28e224b73baa9d6d7b0c309385f57b6dd5d9b), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 11)" )
M4JPGEMC_SET( 199?, m4jpgemc__k,	m4jpgemc,	"gtc01y.p1",	0x0000, 0x010000, CRC(59e8557a) SHA1(8493b160427c21bbb2834c01b39f8a6a8b221bb3), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 12)" )
M4JPGEMC_SET( 199?, m4jpgemc__l,	m4jpgemc,	"hge01ad.p1",	0x0000, 0x010000, CRC(bb201074) SHA1(eb954d165c2d96f952439277d255e3ec3326ada3), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 13)" )
M4JPGEMC_SET( 199?, m4jpgemc__m,	m4jpgemc,	"hge01b.p1",	0x0000, 0x010000, CRC(d7ad2482) SHA1(ed90c4531608e66b14eb1079e85ea59573adf451), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 14)" )
M4JPGEMC_SET( 199?, m4jpgemc__n,	m4jpgemc,	"hge01bd.p1",	0x0000, 0x010000, CRC(3ea0f524) SHA1(1967e5ec14c41c4140c7c39b07085f740c2d1f01), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 15)" )
M4JPGEMC_SET( 199?, m4jpgemc__o,	m4jpgemc,	"hge01c.p1",	0x0000, 0x010000, CRC(498de7bf) SHA1(32dc31852fa69f7d2dd47bbcef695fcf5337f01f), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 16)" )
M4JPGEMC_SET( 199?, m4jpgemc__p,	m4jpgemc,	"hge01d.p1",	0x0000, 0x010000, CRC(be6bb0dd) SHA1(3a0550608c8738b92b48b7a12fb43fb82f52cdd7), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 17)" )
M4JPGEMC_SET( 199?, m4jpgemc__q,	m4jpgemc,	"hge01dk.p1",	0x0000, 0x010000, CRC(80904843) SHA1(8030def4c0e80ac8f28452662487dbfc21a761ee), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 18)" )
M4JPGEMC_SET( 199?, m4jpgemc__r,	m4jpgemc,	"hge01dr.p1",	0x0000, 0x010000, CRC(d89b36b7) SHA1(22c334f1aa314ff288c65eb01ad0415db8e05b15), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 19)" )
M4JPGEMC_SET( 199?, m4jpgemc__s,	m4jpgemc,	"hge01dy.p1",	0x0000, 0x010000, CRC(18ca3ae3) SHA1(ebb434a060564d3a1bc51876257729650e2903a6), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 20)" )
M4JPGEMC_SET( 199?, m4jpgemc__t,	m4jpgemc,	"hge01k.p1",	0x0000, 0x010000, CRC(4161f733) SHA1(b551bb278666790f0c293c76d5c3fabf8f4d368e), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 21)" )
M4JPGEMC_SET( 199?, m4jpgemc__u,	m4jpgemc,	"hge01r.p1",	0x0000, 0x010000, CRC(6dc8dc70) SHA1(e96fc4284ece65f76d5e9bd06c4a002de65bf4da), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 22)" )
M4JPGEMC_SET( 199?, m4jpgemc__v,	m4jpgemc,	"hge01s.p1",	0x0000, 0x010000, CRC(b79f8c42) SHA1(7d8b3352fbd9a80b86f5a8b22833d6f5c4b9854b), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 23)" )
M4JPGEMC_SET( 199?, m4jpgemc__w,	m4jpgemc,	"hge01y.p1",	0x0000, 0x010000, CRC(a96db093) SHA1(17520306112cee6f082829811e1f8c432c6aa354), "Barcrest","Jackpot Gems Classic (Barcrest) (MPU4) (set 24)" )


#define M4JOLGEM_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "jolly1.hex", 0x000000, 0x080000, CRC(de0edae5) SHA1(e3e21e28ae5e838bd6eacc7cf7b20204d7b0327d) ) \
	ROM_LOAD( "jolly2.hex", 0x080000, 0x080000, CRC(08ae81a2) SHA1(6459a694cd820f1a55b636f7c5c77674d3fe4bdb) ) \


#define M4JOLGEM_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4JOLGEM_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \

M4JOLGEM_SET( 199?, m4jolgem,		0,			"gem07s.p1",	0x0000, 0x020000, CRC(945ad0d2) SHA1(d636bc41a4f887d24affc0f5b644c5d5351cf0df), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 1)" )
M4JOLGEM_SET( 199?, m4jolgem__a,	m4jolgem,	"gem05s",		0x0000, 0x020000, CRC(b7ceafc2) SHA1(b66d846da5ff20df912d31695eaef146dbbe759e), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 2)" )
M4JOLGEM_SET( 199?, m4jolgem__b,	m4jolgem,	"gem06ad.p1",	0x0000, 0x020000, CRC(a3270974) SHA1(59992779415ff20b8589843510099b77c9b157fd), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 3)" )
M4JOLGEM_SET( 199?, m4jolgem__c,	m4jolgem,	"gem06b.p1",	0x0000, 0x020000, CRC(188ea295) SHA1(b8a2bdede4478a582f041fd3ff84b5563feaedd3), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 4)" )
M4JOLGEM_SET( 199?, m4jolgem__d,	m4jolgem,	"gem06bd.p1",	0x0000, 0x020000, CRC(4d1b6a6f) SHA1(6cb733b3f8e011e1d12a9aee25577e2bee7deb1a), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 5)" )
M4JOLGEM_SET( 199?, m4jolgem__e,	m4jolgem,	"gem06d.p1",	0x0000, 0x020000, CRC(b68ce7e3) SHA1(ec4400f7c2bb79204fdec1d061801afdd4de70f2), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 6)" )
M4JOLGEM_SET( 199?, m4jolgem__f,	m4jolgem,	"gem06dh.p1",	0x0000, 0x020000, CRC(0dae55fa) SHA1(3f8a23e4efc0c059852801882865baa6654a4eb3), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 7)" )
M4JOLGEM_SET( 199?, m4jolgem__g,	m4jolgem,	"gem06dr.p1",	0x0000, 0x020000, CRC(d7f5b4d6) SHA1(18ff01d9f56d772863698bc72d8e9ec61a9ac9d8), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 8)" )
M4JOLGEM_SET( 199?, m4jolgem__h,	m4jolgem,	"gem06dy.p1",	0x0000, 0x020000, CRC(19771aa3) SHA1(e785196e55353952000d805d23502bc220b2c747), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 9)" )
M4JOLGEM_SET( 199?, m4jolgem__i,	m4jolgem,	"gem06h.p1",	0x0000, 0x020000, CRC(583b9d00) SHA1(6cd43f74c9d4d9d9a4b995277313049a353e6d80), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 10)" )
M4JOLGEM_SET( 199?, m4jolgem__j,	m4jolgem,	"gem06r.p1",	0x0000, 0x020000, CRC(82607c2c) SHA1(a30366773305aacbba96f0c211a67448dd8a2702), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 11)" )
M4JOLGEM_SET( 199?, m4jolgem__k,	m4jolgem,	"gem06s.p1",	0x0000, 0x020000, CRC(e0d82632) SHA1(35a51394a68311d03800db671fbd634bae087e86), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 12)" )
M4JOLGEM_SET( 199?, m4jolgem__l,	m4jolgem,	"gem06y.p1",	0x0000, 0x020000, CRC(4ce2d259) SHA1(bca3e5f79048965bc5c0e80565bbb5ebeefeac87), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 13)" )
M4JOLGEM_SET( 199?, m4jolgem__m,	m4jolgem,	"gem07ad.p1",	0x0000, 0x020000, CRC(21496739) SHA1(05771636542275ecae1cd45bc248ed104a422f03), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 14)" )
M4JOLGEM_SET( 199?, m4jolgem__n,	m4jolgem,	"gem07b.p1",	0x0000, 0x020000, CRC(70ca3435) SHA1(ff631f9adea268c1160646bacca976c069751ba8), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 15)" )
M4JOLGEM_SET( 199?, m4jolgem__o,	m4jolgem,	"gem07bd.p1",	0x0000, 0x020000, CRC(cf750422) SHA1(01c275c90f33afe4cd54c1b9c4963b6c5e66596b), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 16)" )
M4JOLGEM_SET( 199?, m4jolgem__p,	m4jolgem,	"gem07d.p1",	0x0000, 0x020000, CRC(dec87143) SHA1(080483f5500bedac6dfbd252d2ac42e23c1b5ac5), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 17)" )
M4JOLGEM_SET( 199?, m4jolgem__q,	m4jolgem,	"gem07dh.p1",	0x0000, 0x020000, CRC(8fc03bb7) SHA1(dc240714d59f9055ce9c098f7cd60f06a92a0a28), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 18)" )
M4JOLGEM_SET( 199?, m4jolgem__r,	m4jolgem,	"gem07dr.p1",	0x0000, 0x020000, CRC(559bda9b) SHA1(91ec97aab73717eb401672ca4e4b58a87a71f099), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 19)" )
M4JOLGEM_SET( 199?, m4jolgem__s,	m4jolgem,	"gem07dy.p1",	0x0000, 0x020000, CRC(9b1974ee) SHA1(7d6b7ee79ac4401d7e65aa2240ea01cad26eb881), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 20)" )
M4JOLGEM_SET( 199?, m4jolgem__t,	m4jolgem,	"gem07h.p1",	0x0000, 0x020000, CRC(307f0ba0) SHA1(518acd033599b188e972e51753ac610623038aca), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 21)" )
M4JOLGEM_SET( 199?, m4jolgem__u,	m4jolgem,	"gem07r.p1",	0x0000, 0x020000, CRC(ea24ea8c) SHA1(8d3598e44d219f1d66b63bf3ca4062eb84ecbe60), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 22)" )
M4JOLGEM_SET( 199?, m4jolgem__v,	m4jolgem,	"gem07y.p1",	0x0000, 0x020000, CRC(24a644f9) SHA1(b97b83ce0ebf315529efe6d5be051ad71f2e648a), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 23)" )
M4JOLGEM_SET( 199?, m4jolgem__w,	m4jolgem,	"gms04ad.p1",	0x0000, 0x020000, CRC(afd91cf8) SHA1(43f2549d81d9a414c5e2e049fe62a6939ba48943), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 24)" )
M4JOLGEM_SET( 199?, m4jolgem__x,	m4jolgem,	"gms04b.p1",	0x0000, 0x020000, CRC(a11cd9fd) SHA1(16b23c8091da2ada9823f204e7cd5f02b68d37c3), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 25)" )
M4JOLGEM_SET( 199?, m4jolgem__y,	m4jolgem,	"gms04bd.p1",	0x0000, 0x020000, CRC(41e57fe3) SHA1(77322a0988422df6feb5f6871758fbdcf410dae5), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 26)" )
M4JOLGEM_SET( 199?, m4jolgem__z,	m4jolgem,	"gms04d.p1",	0x0000, 0x020000, CRC(0f1e9c8b) SHA1(39661143619230ae64c70ee6d6e6f553e47691a1), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 27)" )
M4JOLGEM_SET( 199?, m4jolgem__0,	m4jolgem,	"gms04dh.p1",	0x0000, 0x020000, CRC(01504076) SHA1(93e44465d74abdbdf5f651e0e8b96ea5b05c1597), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 28)" )
M4JOLGEM_SET( 199?, m4jolgem__1,	m4jolgem,	"gms04dk.p1",	0x0000, 0x020000, CRC(68041a6b) SHA1(03a246ff17001fd937d5556ff8da7165cb95b67c), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 29)" )
M4JOLGEM_SET( 199?, m4jolgem__2,	m4jolgem,	"gms04dr.p1",	0x0000, 0x020000, CRC(db0ba15a) SHA1(85880bf152309b4e81b066a62ace827b899236cd), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 30)" )
M4JOLGEM_SET( 199?, m4jolgem__3,	m4jolgem,	"gms04dy.p1",	0x0000, 0x020000, CRC(15890f2f) SHA1(760806e506d1ff406fab2e50f75428c9b9762804), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 31)" )
M4JOLGEM_SET( 199?, m4jolgem__4,	m4jolgem,	"gms04h.p1",	0x0000, 0x020000, CRC(e1a9e668) SHA1(8b732c52dc221934f57a369e8a20ac2df1aa562c), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 32)" )
M4JOLGEM_SET( 199?, m4jolgem__5,	m4jolgem,	"gms04k.p1",	0x0000, 0x020000, CRC(88fdbc75) SHA1(b1842e2f29e2f3d81c6679a407827601955f02f4), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 33)" )
M4JOLGEM_SET( 199?, m4jolgem__6,	m4jolgem,	"gms04r.p1",	0x0000, 0x020000, CRC(3bf20744) SHA1(9b8d1273545abbf5a7e8e0735e4f23ce024505b9), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 34)" )
M4JOLGEM_SET( 199?, m4jolgem__7,	m4jolgem,	"gms04s.p1",	0x0000, 0x020000, CRC(93f25eef) SHA1(d4bfb2787df10dd09281d33341fbf666850ac23d), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 35)" )
M4JOLGEM_SET( 199?, m4jolgem__8,	m4jolgem,	"gms04y.p1",	0x0000, 0x020000, CRC(f570a931) SHA1(dd50e4626feb9a3e6f0868fa9030204c737f1567), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 36)" )
M4JOLGEM_SET( 199?, m4jolgem__9,	m4jolgem,	"gms05ad.p1",	0x0000, 0x020000, CRC(77d4829c) SHA1(45a240cc2aa3829160854274c928d20655966087), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 37)" )
M4JOLGEM_SET( 199?, m4jolgem__aa,	m4jolgem,	"gms05b.p1",	0x0000, 0x020000, CRC(b929f905) SHA1(028d74687f5c6443d84e84a03666278b9df8e657), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 38)" )
M4JOLGEM_SET( 199?, m4jolgem__ab,	m4jolgem,	"gms05bd.p1",	0x0000, 0x020000, CRC(99e8e187) SHA1(cbb8d3403e6d21fce45b05d1889d17d0355857b4), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 39)" )
M4JOLGEM_SET( 199?, m4jolgem__ac,	m4jolgem,	"gms05d.p1",	0x0000, 0x020000, CRC(172bbc73) SHA1(932907d70b18d25fa912fe895602b0adc52b8e6c), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 40)" )
M4JOLGEM_SET( 199?, m4jolgem__ad,	m4jolgem,	"gms05dh.p1",	0x0000, 0x020000, CRC(d95dde12) SHA1(e509f65b8f0575193002dd0906b1751cae9352f7), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 41)" )
M4JOLGEM_SET( 199?, m4jolgem__ae,	m4jolgem,	"gms05dk.p1",	0x0000, 0x020000, CRC(b009840f) SHA1(5c8f7c081e577642fc7738f7d6fc816155ffc99b), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 42)" )
M4JOLGEM_SET( 199?, m4jolgem__af,	m4jolgem,	"gms05dr.p1",	0x0000, 0x020000, CRC(03063f3e) SHA1(83995208d6c392eacee794c75075296d87e07b13), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 43)" )
M4JOLGEM_SET( 199?, m4jolgem__ag,	m4jolgem,	"gms05dy.p1",	0x0000, 0x020000, CRC(cd84914b) SHA1(8470ba93c518893771ee45190bce5f6f93df7b68), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 44)" )
M4JOLGEM_SET( 199?, m4jolgem__ah,	m4jolgem,	"gms05h.p1",	0x0000, 0x020000, CRC(f99cc690) SHA1(c3cf8aaf8376e9d4eff37afdd818802f9ec4fe64), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 45)" )
M4JOLGEM_SET( 199?, m4jolgem__ai,	m4jolgem,	"gms05k.p1",	0x0000, 0x020000, CRC(90c89c8d) SHA1(4b8e23c9a6bd85563be041d8e95175dc4c39b8e7), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 46)" )
M4JOLGEM_SET( 199?, m4jolgem__aj,	m4jolgem,	"gms05r.p1",	0x0000, 0x020000, CRC(23c727bc) SHA1(0187a14a789f018e6161f2ae160f82c87d03b6a8), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 47)" )
M4JOLGEM_SET( 199?, m4jolgem__ak,	m4jolgem,	"gms05s.p1",	0x0000, 0x020000, CRC(1b830e70) SHA1(eb053a629bd7854759d14acd9793f7eb545fc008), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 48)" )
M4JOLGEM_SET( 199?, m4jolgem__al,	m4jolgem,	"gms05y.p1",	0x0000, 0x020000, CRC(ed4589c9) SHA1(899622c22d29c0ef05bf562176943c9749e236a2), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 49)" )
M4JOLGEM_SET( 199?, m4jolgem__am,	m4jolgem,	"jgem15g",		0x0000, 0x020000, CRC(8288eb80) SHA1(bfb1004b49914b6ae1b0608c9e5c61efe4635ba3), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 50)" )
M4JOLGEM_SET( 199?, m4jolgem__an,	m4jolgem,	"jgem15t",		0x0000, 0x020000, CRC(248f0b95) SHA1(89257c583d6540ffc0fa39f7cb31a87ec8f1f45b), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 51)" )
M4JOLGEM_SET( 199?, m4jolgem__ao,	m4jolgem,	"jjem0",		0x0000, 0x020000, CRC(9b54a881) SHA1(3b1bfacf8fe295c771c558154fe2fca70f049df0), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 52)" )
M4JOLGEM_SET( 199?, m4jolgem__ap,	m4jolgem,	"jgs_xa_x.1_0",	0x0000, 0x020000, CRC(7ac16252) SHA1(b01b2333e1e99f9404a7e0ac80e5e8ee834ec39d), "Barcrest","Jolly Gems (Barcrest) (MPU4) (set 53)" )


#define M4HITTOP_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "hi4snd.p1", 0x000000, 0x080000, CRC(066f262b) SHA1(fd48da486592740c68ee497396602199101711a6) ) \
	ROM_LOAD( "hi4snd.p2", 0x080000, 0x080000, CRC(0ee89f6c) SHA1(7088149000efd1dcdf37aa9b88f7c6491184da24) ) \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) \
	ROM_LOAD( "httsnd.p1",   0x000000, 0x080000, CRC(1cfb12d2) SHA1(d909c7ee8ea10587a9a9251af943b0151d2c4a16) ) \


#define M4HITTOP_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4HITTOP_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4HITTOP_SET( 199?, m4hittop,	0,			"hi4s.p1",		0x0000, 0x010000, CRC(3a04ee7a) SHA1(d23e9da2c22f6983a855bc519597ea9cea84f2dd), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 1)" )
M4HITTOP_SET( 199?, m4hittop__a,	m4hittop,	"chuad.p1",		0x0000, 0x010000, CRC(01d3b86c) SHA1(27af0e76661495d5b91ee6a53507f9a5d4e5ab85), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 2)" )
M4HITTOP_SET( 199?, m4hittop__b,	m4hittop,	"chub.p1",		0x0000, 0x010000, CRC(17ff4ed4) SHA1(f193a00a46c82d4989af18055f9f69d93df79ec6), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 3)" )
M4HITTOP_SET( 199?, m4hittop__c,	m4hittop,	"chubd.p1",		0x0000, 0x010000, CRC(3e7a6b1b) SHA1(8939a0cac8578ff5e1d1ab2b3a64b3809793c44a), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 4)" )
M4HITTOP_SET( 199?, m4hittop__d,	m4hittop,	"chud.p1",		0x0000, 0x010000, CRC(26875ed3) SHA1(06dbf594e2c5202ee624f4202f634281a89a3870), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 5)" )
M4HITTOP_SET( 199?, m4hittop__e,	m4hittop,	"chudk.p1",		0x0000, 0x010000, CRC(10c1f6c3) SHA1(e6ff6ea40f35cfd9ed7643e69eca62775f20b3a2), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 6)" )
M4HITTOP_SET( 199?, m4hittop__f,	m4hittop,	"chudy.p1",		0x0000, 0x010000, CRC(65302d8c) SHA1(de340cc182212b576cae46669492d0d760d2f288), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 7)" )
M4HITTOP_SET( 199?, m4hittop__g,	m4hittop,	"chuk.p1",		0x0000, 0x010000, CRC(7f333a2c) SHA1(73719997c200ec5291ceaa12f8667979a731212e), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 8)" )
M4HITTOP_SET( 199?, m4hittop__h,	m4hittop,	"chur.p1",		0x0000, 0x010000, CRC(dbb89a00) SHA1(70b2f2c78011b8b470aa58153d524f920d553b28), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 9)" )
M4HITTOP_SET( 199?, m4hittop__i,	m4hittop,	"chus.p1",		0x0000, 0x010000, CRC(8a39816e) SHA1(3869f7ae0c9b681cfb07e2f6c1a94fc81fa13fe3), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 10)" )
M4HITTOP_SET( 199?, m4hittop__j,	m4hittop,	"chuy.p1",		0x0000, 0x010000, CRC(e0902d74) SHA1(a34db63f1354853ad5a1026e4402ccd2e564c7d7), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 11)" )
M4HITTOP_SET( 199?, m4hittop__k,	m4hittop,	"hi4ad.p1",		0x0000, 0x010000, CRC(eeb958f3) SHA1(ee7f7615df2141ad5183288101949b74c4543de9), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 12)" )
M4HITTOP_SET( 199?, m4hittop__l,	m4hittop,	"hi4b.p1",		0x0000, 0x010000, CRC(68af264b) SHA1(e7f75b5294cc7541f9397c492c171c79b7a21a36), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 13)" )
M4HITTOP_SET( 199?, m4hittop__m,	m4hittop,	"hi4bd.p1",		0x0000, 0x010000, CRC(d72cd485) SHA1(d0d38cbb518c824d4a8107e1711f85120c39bc4c), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 14)" )
M4HITTOP_SET( 199?, m4hittop__n,	m4hittop,	"hi4d.p1",		0x0000, 0x010000, CRC(59d7364c) SHA1(1e665b178b8bf7314ca5b5ea97dc185491fc2930), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 15)" )
M4HITTOP_SET( 199?, m4hittop__o,	m4hittop,	"hi4dk.p1",		0x0000, 0x010000, CRC(76d4bb70) SHA1(7365f072a7e3e8141e15fbf56c3355bc6310895f), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 16)" )
M4HITTOP_SET( 199?, m4hittop__p,	m4hittop,	"hi4dy.p1",		0x0000, 0x010000, CRC(b1ddf7fe) SHA1(a334619b5dfc7a44e9082cc37cb5187413adb29f), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 17)" )
M4HITTOP_SET( 199?, m4hittop__q,	m4hittop,	"hi4k.p1",		0x0000, 0x010000, CRC(99cb8bc9) SHA1(106bf6e327643c49024f9422d6b87f5b157b452f), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 18)" )
M4HITTOP_SET( 199?, m4hittop__r,	m4hittop,	"hi4y.p1",		0x0000, 0x010000, CRC(c60e01e6) SHA1(c4a7ea44c36c78401cab3ef87d7e02add0b48ab5), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 19)" )
M4HITTOP_SET( 199?, m4hittop__s,	m4hittop,	"hit04ad.p1",	0x0000, 0x010000, CRC(cc9d10fa) SHA1(b7ce14fecfd8142fa7127c23f152c749dae74701), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 20)" )
M4HITTOP_SET( 199?, m4hittop__t,	m4hittop,	"hit04b.p1",	0x0000, 0x010000, CRC(da511063) SHA1(3f4fb8518cb2057ec4c2bb13fd3e61ee73bfa457), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 21)" )
M4HITTOP_SET( 199?, m4hittop__u,	m4hittop,	"hit04bd.p1",	0x0000, 0x010000, CRC(40a84b97) SHA1(416f78c19e08f405a3b36f886f69e7b88e5aa90a), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 22)" )
M4HITTOP_SET( 199?, m4hittop__v,	m4hittop,	"hit04d.p1",	0x0000, 0x010000, CRC(89607e84) SHA1(280209ca3030383547cc91eee2f71a810768353f), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 23)" )
M4HITTOP_SET( 199?, m4hittop__w,	m4hittop,	"hit04dk.p1",	0x0000, 0x010000, CRC(b89e606f) SHA1(9096126b719ecd92185d9c1d50d13c9339d09583), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 24)" )
M4HITTOP_SET( 199?, m4hittop__x,	m4hittop,	"hit04dr.p1",	0x0000, 0x010000, CRC(1b4d8099) SHA1(505e0948a78b5d57f0986896ab900d25a20d7877), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 25)" )
M4HITTOP_SET( 199?, m4hittop__y,	m4hittop,	"hit04dy.p1",	0x0000, 0x010000, CRC(25f54881) SHA1(9f4ae52295df5810cbe6c18cae66877bec006a28), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 26)" )
M4HITTOP_SET( 199?, m4hittop__z,	m4hittop,	"hit04k.p1",	0x0000, 0x010000, CRC(5ef3f78d) SHA1(e72727b3dc7793c36f182b3e7d363741254c0be7), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 27)" )
M4HITTOP_SET( 199?, m4hittop__0,	m4hittop,	"hit04r.p1",	0x0000, 0x010000, CRC(d87a9f60) SHA1(614224b80afaa6e407f9b40b45b8aecdf999e13a), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 28)" )
M4HITTOP_SET( 199?, m4hittop__1,	m4hittop,	"hit04s.p1",	0x0000, 0x010000, CRC(05376f9f) SHA1(e59bdd6541669b150bb68eb97ea316c3fe451778), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 29)" )
M4HITTOP_SET( 199?, m4hittop__2,	m4hittop,	"hit04y.p1",	0x0000, 0x010000, CRC(c398df63) SHA1(5e93cb95da37b1593d030e99e97996252ad6cda1), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 30)" )
M4HITTOP_SET( 199?, m4hittop__3,	m4hittop,	"ht201ad.p1",	0x0000, 0x010000, CRC(b0f3873b) SHA1(6e7d1b20dff4b81ebd171d6d92c95e46817bdf90), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 31)" )
M4HITTOP_SET( 199?, m4hittop__4,	m4hittop,	"ht201b.p1",	0x0000, 0x010000, CRC(9dbe41fc) SHA1(ce5ed2707ab63057a2f66a1098e3752acaa72dac), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 32)" )
M4HITTOP_SET( 199?, m4hittop__5,	m4hittop,	"ht201bd.p1",	0x0000, 0x010000, CRC(be23c8b6) SHA1(0d4ab2d3c7ac063ec1ce10b2af28c8770d8bd818), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 33)" )
M4HITTOP_SET( 199?, m4hittop__6,	m4hittop,	"ht201d.p1",	0x0000, 0x010000, CRC(25b9fcd7) SHA1(8bebbf0b621a704ed9811e67eab003f4ddebcde2), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 34)" )
M4HITTOP_SET( 199?, m4hittop__7,	m4hittop,	"ht201dk.p1",	0x0000, 0x010000, CRC(1c77872e) SHA1(a728811efee6f40779b01a6d60f2d0167e204a09), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 35)" )
M4HITTOP_SET( 199?, m4hittop__8,	m4hittop,	"ht201dr.p1",	0x0000, 0x010000, CRC(9836d075) SHA1(b015e9706c5ec7b03133eda70fe0322c24969d7e), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 36)" )
M4HITTOP_SET( 199?, m4hittop__9,	m4hittop,	"ht201dy.p1",	0x0000, 0x010000, CRC(811cafc0) SHA1(e31ad353ee8ce4ea059d6a469baaa14357b738c9), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 37)" )
M4HITTOP_SET( 199?, m4hittop__aa,	m4hittop,	"ht201k.p1",	0x0000, 0x010000, CRC(191ca612) SHA1(a2a80b64cc04aa590046413f1474340cd3a5b03a), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 38)" )
M4HITTOP_SET( 199?, m4hittop__ab,	m4hittop,	"ht201r.p1",	0x0000, 0x010000, CRC(154643be) SHA1(280ae761c434bbed84317d85aef2ad4a78c61d1d), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 39)" )
M4HITTOP_SET( 199?, m4hittop__ac,	m4hittop,	"ht201s.p1",	0x0000, 0x010000, CRC(37b20464) SHA1(e87b0a2023416fa7b63201e19850319723eb6c10), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 40)" )
M4HITTOP_SET( 199?, m4hittop__ad,	m4hittop,	"ht201y.p1",	0x0000, 0x010000, CRC(84778efc) SHA1(bdc43973913d0e8be0e16ee89da01b1bcdc2da6f), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 41)" )
M4HITTOP_SET( 199?, m4hittop__ae,	m4hittop,	"ht501ad.p1",	0x0000, 0x010000, CRC(7bf00848) SHA1(700d90218d0bd31860dc905c00d0afbf3a1e8704), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 42)" )
M4HITTOP_SET( 199?, m4hittop__af,	m4hittop,	"ht501b.p1",	0x0000, 0x010000, CRC(c06dd046) SHA1(a47c62fc299842e66694f34844b43a55d6f20c3d), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 43)" )
M4HITTOP_SET( 199?, m4hittop__ag,	m4hittop,	"ht501bd.p1",	0x0000, 0x010000, CRC(d4a3843f) SHA1(cc66ebaa334bab86b9bcb1623316c31318e84d2a), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 44)" )
M4HITTOP_SET( 199?, m4hittop__ah,	m4hittop,	"ht501d.p1",	0x0000, 0x010000, CRC(67d3c040) SHA1(beec134c53715544080327319b5d6231b625fbb4), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 45)" )
M4HITTOP_SET( 199?, m4hittop__ai,	m4hittop,	"ht501dk.p1",	0x0000, 0x010000, CRC(feec7950) SHA1(7bcd8d0166847f72871a78e4b287c72e1a06d26e), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 46)" )
M4HITTOP_SET( 199?, m4hittop__aj,	m4hittop,	"ht501dr.p1",	0x0000, 0x010000, CRC(c73b60b1) SHA1(9f14957eb0eec7e833b6bb5b162286d94c8f03c4), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 47)" )
M4HITTOP_SET( 199?, m4hittop__ak,	m4hittop,	"ht501dy.p1",	0x0000, 0x010000, CRC(756c7ae9) SHA1(42a731e472f073845b98d7fcc47fe70f57181ce6), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 48)" )
M4HITTOP_SET( 199?, m4hittop__al,	m4hittop,	"ht501k.p1",	0x0000, 0x010000, CRC(93269e53) SHA1(7e40ac4e9f4b26755867353fdccadf0f976402b4), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 49)" )
M4HITTOP_SET( 199?, m4hittop__am,	m4hittop,	"ht501r.p1",	0x0000, 0x010000, CRC(9aec0493) SHA1(6b0b7e5f4a988ff4d2bc123978adc09195eb4232), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 50)" )
M4HITTOP_SET( 199?, m4hittop__an,	m4hittop,	"ht501s.p1",	0x0000, 0x010000, CRC(ac440a2b) SHA1(f3f3d0c9c8dcb41509307c970f0776ebcfffdeb0), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 51)" )
M4HITTOP_SET( 199?, m4hittop__ao,	m4hittop,	"ht501y.p1",	0x0000, 0x010000, CRC(a7f8ece6) SHA1(f4472c040c9255eaef5b1109c3bec44f4978b600), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 52)" )
M4HITTOP_SET( 199?, m4hittop__ap,	m4hittop,	"httad.p1",		0x0000, 0x010000, CRC(e5a3df45) SHA1(70bebb33cbe466c379f278347d0b47862b1d01a8), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 53)" )
M4HITTOP_SET( 199?, m4hittop__aq,	m4hittop,	"httb.p1",		0x0000, 0x010000, CRC(5c921ff2) SHA1(a9184e4e3916c1ab92761d0e33b42cce4a58e7b1), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 54)" )
M4HITTOP_SET( 199?, m4hittop__ar,	m4hittop,	"httbd.p1",		0x0000, 0x010000, CRC(9d19fac9) SHA1(17072ac5b49cd947bf397dfbe9b6b0bd269dd1b4), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 55)" )
M4HITTOP_SET( 199?, m4hittop__as,	m4hittop,	"httd.p1",		0x0000, 0x010000, CRC(5e5bacb9) SHA1(d673010cdf2fb9352fc510409deade42b5508b29), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 56)" )
M4HITTOP_SET( 199?, m4hittop__at,	m4hittop,	"httdk.p1",		0x0000, 0x010000, CRC(17b1db87) SHA1(196163f68c82c4600ecacee52ee8044739568fbf), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 57)" )
M4HITTOP_SET( 199?, m4hittop__au,	m4hittop,	"httdy.p1",		0x0000, 0x010000, CRC(428af7bf) SHA1(954a512105d1a5998d4ffcbf21be0c9d9a65bbeb), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 58)" )
M4HITTOP_SET( 199?, m4hittop__av,	m4hittop,	"httk.p1",		0x0000, 0x010000, CRC(581dd34a) SHA1(00cad1860f5edf056b8f9397ca46165593be4755), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 59)" )
M4HITTOP_SET( 199?, m4hittop__aw,	m4hittop,	"htts.p1",		0x0000, 0x010000, CRC(6c794eb2) SHA1(347a7c74b1fd7631fbcd398bf5e7c36af088109e), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 60)" )
M4HITTOP_SET( 199?, m4hittop__ax,	m4hittop,	"htty.p1",		0x0000, 0x010000, CRC(c9b402b2) SHA1(2165c1892fc1f0b9b0c39127f322f15c9e1912b1), "Barcrest","Hit The Top (Barcrest) (MPU4) (set 61)" )


#define M4NNWW_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "m574.chr", 0x0000, 0x000048, CRC(cc4b7911) SHA1(9f8a96a1f8b0f9b33b852e93483ce5c684703349) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "cn4snd.p1", 0x0000, 0x080000, CRC(720011ce) SHA1(fa9108463131ea7e64525e080ac0eff2f6708db8) ) \

#define M4NNWW_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4NNWW_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4NNWW_SET( 199?, m4nnww,		0,		"nn5bd.p1",		0x0000, 0x010000, CRC(56cc9559) SHA1(53e109a579e422932dd25c52cf2beca51d3a53e3), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 1)" )
M4NNWW_SET( 199?, m4nnww__a,	m4nnww,	"cf301s",		0x0000, 0x010000, CRC(1d8abf59) SHA1(81e47797baddd777fbbb1b1e044df1bfe3d49cb2), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 2)" )
M4NNWW_SET( 199?, m4nnww__b,	m4nnww,	"cni01ad.p1",	0x0000, 0x010000, CRC(788e47b1) SHA1(6d07500a38b54e1a9038e35d82fdb4a0f22d23ba), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 3)" )
M4NNWW_SET( 199?, m4nnww__c,	m4nnww,	"cni01b.p1",	0x0000, 0x010000, CRC(33512643) SHA1(865ed3b68fe3b737833734513b5045c5db97791e), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 4)" )
M4NNWW_SET( 199?, m4nnww__d,	m4nnww,	"cni01bd.p1",	0x0000, 0x010000, CRC(8a00d73b) SHA1(702579ea1bc586aacd5cba889919f3e86ea05771), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 5)" )
M4NNWW_SET( 199?, m4nnww__e,	m4nnww,	"cni01c.p1",	0x0000, 0x010000, CRC(b836ee44) SHA1(832914461492f120894ec7e63f6aa1ad00b89b41), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 6)" )
M4NNWW_SET( 199?, m4nnww__f,	m4nnww,	"cni01d.p1",	0x0000, 0x010000, CRC(94fbe9cb) SHA1(7daabf1cd315f8d18796ba34f8c2ec271cc1e396), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 7)" )
M4NNWW_SET( 199?, m4nnww__g,	m4nnww,	"cni01dk.p1",	0x0000, 0x010000, CRC(708fbcca) SHA1(7e97d8adf660099873a94d1915c79f110614cb11), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 8)" )
M4NNWW_SET( 199?, m4nnww__h,	m4nnww,	"cni01dr.p1",	0x0000, 0x010000, CRC(5b7ed753) SHA1(8072ac849dc61e50963ae6730fa32823bd038c77), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 9)" )
M4NNWW_SET( 199?, m4nnww__i,	m4nnww,	"cni01dy.p1",	0x0000, 0x010000, CRC(fcf6da8b) SHA1(95d86af30035884211ed26ccb5db9aae12ac7bf2), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 10)" )
M4NNWW_SET( 199?, m4nnww__j,	m4nnww,	"cni01k.p1",	0x0000, 0x010000, CRC(f7c90833) SHA1(3b3b44e61f24e9fb45f465fd9c381fe81b6851a0), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 11)" )
M4NNWW_SET( 199?, m4nnww__k,	m4nnww,	"cni01r.p1",	0x0000, 0x010000, CRC(c611b1eb) SHA1(524ee18da8a086d15277d9fb0ea383ee3d49d47a), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 12)" )
M4NNWW_SET( 199?, m4nnww__l,	m4nnww,	"cni01s.p1",	0x0000, 0x010000, CRC(5ed6a396) SHA1(299767467b56d1aa93602f98cc387e7ff18bda9d), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 13)" )
M4NNWW_SET( 199?, m4nnww__m,	m4nnww,	"cni01y.p1",	0x0000, 0x010000, CRC(d3612bf2) SHA1(40a8ff08a38c4411946a67f380891945d166d199), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 14)" )
M4NNWW_SET( 199?, m4nnww__n,	m4nnww,	"cnuad.p1",		0x0000, 0x010000, CRC(f4b28628) SHA1(7323525a44477e2a3f89562f6094ed7bb47a16cc), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 15)" )
M4NNWW_SET( 199?, m4nnww__o,	m4nnww,	"cnub.p1",		0x0000, 0x010000, CRC(735260a3) SHA1(e08fff6314d7cb4e396107366fdc16dcbf7f5d67), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 16)" )
M4NNWW_SET( 199?, m4nnww__p,	m4nnww,	"cnubd.p1",		0x0000, 0x010000, CRC(fbe1ee39) SHA1(21bdaa6f9af686b4e44958ee09a131d0e12c2c53), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 17)" )
M4NNWW_SET( 199?, m4nnww__q,	m4nnww,	"cnud.p1",		0x0000, 0x010000, CRC(d3a0eff1) SHA1(2b18c3e14a43d072ae5702bc77fcac65dbd8305c), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 18)" )
M4NNWW_SET( 199?, m4nnww__r,	m4nnww,	"cnudk.p1",		0x0000, 0x010000, CRC(a7b506e8) SHA1(40d712076b434a339dfa60b937eec91038568312), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 19)" )
M4NNWW_SET( 199?, m4nnww__s,	m4nnww,	"cnudr.p1",		0x0000, 0x010000, CRC(e163caea) SHA1(273a13567e5cb7fd071dfc9c8a9bc923e25d7679), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 20)" )
M4NNWW_SET( 199?, m4nnww__t,	m4nnww,	"cnudy.p1",		0x0000, 0x010000, CRC(7c08e204) SHA1(34c906f3a284fde0c997232738a51b709a0dca93), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 21)" )
M4NNWW_SET( 199?, m4nnww__u,	m4nnww,	"cnuk.p1",		0x0000, 0x010000, CRC(b9c08873) SHA1(9c5a754a7b57c8ab4334afdcbe30884a7181ac48), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 22)" )
M4NNWW_SET( 199?, m4nnww__v,	m4nnww,	"cnur.p1",		0x0000, 0x010000, CRC(729d89ea) SHA1(c98a89dd8f85dde7ab005bcb7eba1fcc31162e08), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 23)" )
M4NNWW_SET( 199?, m4nnww__w,	m4nnww,	"cnus.p1",		0x0000, 0x010000, CRC(6afee8e1) SHA1(35464eef29a5a66b8efea890987ff120ca5b7409), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 24)" )
M4NNWW_SET( 199?, m4nnww__x,	m4nnww,	"cnuy.p1",		0x0000, 0x010000, CRC(eff6a104) SHA1(021baf5fe88defca05627a85501622d86e846233), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 25)" )
M4NNWW_SET( 199?, m4nnww__y,	m4nnww,	"nn3xad.p1",	0x0000, 0x010000, CRC(8ccfceb8) SHA1(762ab26826d3d2a4dd7999a71724389344e9dafb), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 26)" )
M4NNWW_SET( 199?, m4nnww__z,	m4nnww,	"nn3xb.p1",		0x0000, 0x010000, CRC(9b0dd473) SHA1(9975dafea8c7d6ccfc9f826adb1a0d3d0ed9740a), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 27)" )
M4NNWW_SET( 199?, m4nnww__0,	m4nnww,	"nn3xbd.p1",	0x0000, 0x010000, CRC(21bf4a89) SHA1(200c9ccc4bc2a93fcd0f68bb00ad4391bdeecda1), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 28)" )
M4NNWW_SET( 199?, m4nnww__1,	m4nnww,	"nn3xd.p1",		0x0000, 0x010000, CRC(11e22c45) SHA1(6da31eea7b25612d99cc79f6f9579622f105c862), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 29)" )
M4NNWW_SET( 199?, m4nnww__2,	m4nnww,	"nn3xdk.p1",	0x0000, 0x010000, CRC(0f4642c6) SHA1(53a0b8bc102c2b1c0db71887470b70852b09a4e9), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 30)" )
M4NNWW_SET( 199?, m4nnww__3,	m4nnww,	"nn3xdy.p1",	0x0000, 0x010000, CRC(ba3c1cf0) SHA1(ab94227018c3f9173e6a648749d455afd1ed36ce), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 31)" )
M4NNWW_SET( 199?, m4nnww__4,	m4nnww,	"nn3xk.p1",		0x0000, 0x010000, CRC(ec3a9831) SHA1(0b3ba86faf39cf3a1e42cb1c31fd2c50c24d65dc), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 32)" )
M4NNWW_SET( 199?, m4nnww__5,	m4nnww,	"nn3xr.p1",		0x0000, 0x010000, CRC(6416481c) SHA1(b06ed4964d9cbf403905504ac68abdab53131476), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 33)" )
M4NNWW_SET( 199?, m4nnww__6,	m4nnww,	"nn3xrd.p1",	0x0000, 0x010000, CRC(0fd3f9b9) SHA1(99115b217cfc54b52469ffc77e7a7592907c53ea), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 34)" )
M4NNWW_SET( 199?, m4nnww__7,	m4nnww,	"nn3xs.p1",		0x0000, 0x010000, CRC(13d02d21) SHA1(8e4dac8e60538884d3f3a92fc1bb9f41276be4c8), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 35)" )
M4NNWW_SET( 199?, m4nnww__8,	m4nnww,	"nn3xy.p1",		0x0000, 0x010000, CRC(8a5d0f4b) SHA1(ef727e7ee8bb20d1b201927186a1a4f83e1e7497), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 36)" )
M4NNWW_SET( 199?, m4nnww__9,	m4nnww,	"nn4ad.p1",		0x0000, 0x010000, CRC(827b832f) SHA1(4448ccb03282b9d39c6a00d02cea4d8ce2225b0e), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 37)" )
M4NNWW_SET( 199?, m4nnww__aa,	m4nnww,	"nn4b.p1",		0x0000, 0x010000, CRC(65e16330) SHA1(cfd18693155b4b7c5692064a2f693eb198d02749), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 38)" )
M4NNWW_SET( 199?, m4nnww__ab,	m4nnww,	"nn4bd.p1",		0x0000, 0x010000, CRC(b467ee65) SHA1(79030aa06ca8fd9c8becff62d56628939e9b5075), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 39)" )
M4NNWW_SET( 199?, m4nnww__ac,	m4nnww,	"nn4d.p1",		0x0000, 0x010000, CRC(548dacb9) SHA1(55949910374fae419ba015b70780e3e9e269caa0), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 40)" )
M4NNWW_SET( 199?, m4nnww__ad,	m4nnww,	"nn4dk.p1",		0x0000, 0x010000, CRC(9053aa15) SHA1(99d1e6d8776434a4ec69a565d673b45402467b8d), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 41)" )
M4NNWW_SET( 199?, m4nnww__ae,	m4nnww,	"nn4dy.p1",		0x0000, 0x010000, CRC(5fcd5a18) SHA1(b1b3283a303114ca1daab89cea44211ece7188ef), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 42)" )
M4NNWW_SET( 199?, m4nnww__af,	m4nnww,	"nn4k.p1",		0x0000, 0x010000, CRC(09a808c0) SHA1(c74c3acb2c1f52fd1e83003fb1a022f80f55e0b8), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 43)" )
M4NNWW_SET( 199?, m4nnww__ag,	m4nnww,	"nn4s.p1",		0x0000, 0x010000, CRC(ec4f01ee) SHA1(443da7ed359a3e208417f7bca0dc52a09594a927), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 44)" )
M4NNWW_SET( 199?, m4nnww__ah,	m4nnww,	"nn4y.p1",		0x0000, 0x010000, CRC(a1eff941) SHA1(369ec89b82f97c3d8266d41e5eb27be7770bdca4), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 45)" )
M4NNWW_SET( 199?, m4nnww__ai,	m4nnww,	"nn5ad.p1",		0x0000, 0x010000, CRC(22537184) SHA1(aef542a34e2b14a5db624e42d1cd2682de237b52), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 46)" )
M4NNWW_SET( 199?, m4nnww__aj,	m4nnww,	"nn5b.p1",		0x0000, 0x010000, CRC(e2a99408) SHA1(a0868a38c290a84926089c60d1b5555706485bff), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 47)" )
M4NNWW_SET( 199?, m4nnww__ak,	m4nnww,	"nn5d.p1",		0x0000, 0x010000, CRC(ef1a21b6) SHA1(ba763b06583af1273e384b878fbacc68f88714dc), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 48)" )
M4NNWW_SET( 199?, m4nnww__al,	m4nnww,	"nn5dk.p1",		0x0000, 0x010000, CRC(74c48e28) SHA1(db6be2275b6122845c662dd5f12266b66e888221), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 49)" )
M4NNWW_SET( 199?, m4nnww__am,	m4nnww,	"nn5dr.p1",		0x0000, 0x010000, CRC(f52c9f87) SHA1(e8b1037c9ed5d9452abccb6b07bae46b45c4705e), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 50)" )
M4NNWW_SET( 199?, m4nnww__an,	m4nnww,	"nn5dy.p1",		0x0000, 0x010000, CRC(6847b769) SHA1(1b4d42774c72a3c7b40551c7181413ea1fca0b88), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 51)" )
M4NNWW_SET( 199?, m4nnww__ao,	m4nnww,	"nn5k.p1",		0x0000, 0x010000, CRC(ceab49d9) SHA1(633e7bab6a30176dbcea2bd3e7bab0f7833409ba), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 52)" )
M4NNWW_SET( 199?, m4nnww__ap,	m4nnww,	"nn5r.p1",		0x0000, 0x010000, CRC(144523cd) SHA1(d12586ccea659ecb75af944d87ddd480da917eaf), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 53)" )
M4NNWW_SET( 199?, m4nnww__aq,	m4nnww,	"nn5s.p1",		0x0000, 0x010000, CRC(459e5663) SHA1(66ae821e5202d6d3ba05be44d0c1f26da60a3a32), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 54)" )
M4NNWW_SET( 199?, m4nnww__ar,	m4nnww,	"nn5y.p1",		0x0000, 0x010000, CRC(892e0b23) SHA1(ff3f550e20e71e868d52b60740f743a7d2d6c645), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 55)" )
M4NNWW_SET( 199?, m4nnww__as,	m4nnww,	"nnu40x.bin",	0x0000, 0x010000, CRC(63e3d7df) SHA1(1a5a00185ec5150f5b05765f06297d7884540aaf), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 56)" )
M4NNWW_SET( 199?, m4nnww__at,	m4nnww,	"nnus.p1",		0x0000, 0x010000, CRC(3e3a829e) SHA1(5aa3a56e007bad4dacdc3c993c87569e4250eecd), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 57)" )
M4NNWW_SET( 199?, m4nnww__au,	m4nnww,	"nnux.p1",		0x0000, 0x010000, CRC(38806ebf) SHA1(a897a33e3260de1b284b01a65d1da7cbe05d51f8), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 58)" )
M4NNWW_SET( 199?, m4nnww__av,	m4nnww,	"nnuxb.p1",		0x0000, 0x010000, CRC(c4dba8df) SHA1(0f8516cc9b2f0be9d1c936667974cd8116018dad), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 59)" )
M4NNWW_SET( 199?, m4nnww__aw,	m4nnww,	"nnuxc.p1",		0x0000, 0x010000, CRC(797e0c4d) SHA1(211b0a804643731275d0075461f8d94985fde1db), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 60)" )
M4NNWW_SET( 199?, m4nnww__ax,	m4nnww,	"nnwink.hex",	0x0000, 0x010000, CRC(f77bd6c4) SHA1(1631040fbfe3fc37c2cbd3145857c31d16b92bde), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 61)" )
M4NNWW_SET( 199?, m4nnww__ay,	m4nnww,	"nnww2010",		0x0000, 0x010000, CRC(67b1c7b5) SHA1(495e25bc2051ab78e473cd0c36e0c1825c06db14), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 62)" )
M4NNWW_SET( 199?, m4nnww__az,	m4nnww,	"wink2010",		0x0000, 0x010000, CRC(056a2ffa) SHA1(9da96d70ff850b6672ae7009253e179fa7159db4), "Barcrest","Nudge Nudge Wink Wink (Barcrest) (MPU4) (set 63)" )


#define M4RFYM_EXTRA_ROMS \
	ROM_REGION( 0x10, "fakechr", 0 ) \
	ROM_LOAD( "rft20.10_chr", 0x0000, 0x000010, CRC(87cdda8d) SHA1(3d17d42d5eaf5dcc04b856ec07689cab6183b12d) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "runsnd.p1", 0x000000, 0x080000, CRC(a37a3a6d) SHA1(b82c7e90508795a53b91d7dab7938abf07e8ab4c) ) \
	ROM_LOAD( "runsnd.p2", 0x080000, 0x080000, CRC(1c03046f) SHA1(5235b2f60f12cbee11fb5e54e1858a11a755f460) ) \

#define M4RFYM_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RFYM_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4RFYM_SET( 199?, m4rfym,		0,		"rund.p1",		0x0000, 0x010000, CRC(2be2a66d) SHA1(a66d74ccf1783912673cfcb6c1ae7fbb6d70ca0e), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 1)" )
M4RFYM_SET( 199?, m4rfym__a,	m4rfym,	"ap1ad.p1",		0x0000, 0x010000, CRC(d1adbf80) SHA1(08801f38b8ba5034fd83b53b6cfff864104525b4), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 2)" )
M4RFYM_SET( 199?, m4rfym__b,	m4rfym,	"ap1b.p1",		0x0000, 0x010000, CRC(4939f186) SHA1(389d46d603e75d3aaeeca990f4e1143c61f1565f), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 3)" )
M4RFYM_SET( 199?, m4rfym__c,	m4rfym,	"ap1bd.p1",		0x0000, 0x010000, CRC(08a33b2c) SHA1(ef38e9cd0c9bc8393530e36060c803d1250c46a6), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 4)" )
M4RFYM_SET( 199?, m4rfym__d,	m4rfym,	"ap1d.p1",		0x0000, 0x010000, CRC(edef44fe) SHA1(4907804c1bebc1f13aa3eb9dad0e9189de8e9601), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 5)" )
M4RFYM_SET( 199?, m4rfym__e,	m4rfym,	"ap1dk.p1",		0x0000, 0x010000, CRC(873a402c) SHA1(1315a4ad18544ca5d65526ea0f620cac528e4cad), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 6)" )
M4RFYM_SET( 199?, m4rfym__f,	m4rfym,	"ap1dy.p1",		0x0000, 0x010000, CRC(e8436c00) SHA1(1c2f171e55c3519d63d6c4dd0d56df4e1daad6af), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 7)" )
M4RFYM_SET( 199?, m4rfym__g,	m4rfym,	"ap1k.p1",		0x0000, 0x010000, CRC(9afeb1e7) SHA1(5fc5d73a2c976d227a0598fb1dd802c6336415d1), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 8)" )
M4RFYM_SET( 199?, m4rfym__h,	m4rfym,	"ap1s.p1",		0x0000, 0x010000, CRC(7474509c) SHA1(c87e20f10806ec87fd33f97b43b8378d304f7d67), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 9)" )
M4RFYM_SET( 199?, m4rfym__i,	m4rfym,	"ap1y.p1",		0x0000, 0x010000, CRC(152bf7cb) SHA1(8dd8b621f9dac430c293b29ca03814fc21a148b9), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 10)" )
M4RFYM_SET( 199?, m4rfym__j,	m4rfym,	"ap502ad.p1",	0x0000, 0x010000, CRC(ab059e57) SHA1(45ba91989b0fd1a44628f696b78eae2a349e3e4a), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 11)" )
M4RFYM_SET( 199?, m4rfym__k,	m4rfym,	"ap502b.p1",	0x0000, 0x010000, CRC(9ed27a6e) SHA1(2d655305a178e4ebe43f3d429dfec5a2ef6b9873), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 12)" )
M4RFYM_SET( 199?, m4rfym__l,	m4rfym,	"ap502bd.p1",	0x0000, 0x010000, CRC(48e83fcd) SHA1(3e2de0416722df5004f00baae2d3f6846ff596e5), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 13)" )
M4RFYM_SET( 199?, m4rfym__m,	m4rfym,	"ap502d.p1",	0x0000, 0x010000, CRC(d0560301) SHA1(c35e97391c588f6567eeb253eb9de59bec9e1724), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 14)" )
M4RFYM_SET( 199?, m4rfym__n,	m4rfym,	"ap502dk.p1",	0x0000, 0x010000, CRC(82aa8d80) SHA1(e42d10537dcc5aaae59472681b215b0eb0821c25), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 15)" )
M4RFYM_SET( 199?, m4rfym__o,	m4rfym,	"ap502dr.p1",	0x0000, 0x010000, CRC(1cfb3102) SHA1(b1d3a533de0ff93e15f7c039e75af0ef6c8eec57), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 16)" )
M4RFYM_SET( 199?, m4rfym__p,	m4rfym,	"ap502dy.p1",	0x0000, 0x010000, CRC(819019ec) SHA1(36d2093a7a592850533d4206e0c9dd28cdc17568), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 17)" )
M4RFYM_SET( 199?, m4rfym__q,	m4rfym,	"ap502k.p1",	0x0000, 0x010000, CRC(5064a894) SHA1(3e67358fe5ed9bfac05f621d7e72e5be7aae67df), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 18)" )
M4RFYM_SET( 199?, m4rfym__r,	m4rfym,	"ap502r.p1",	0x0000, 0x010000, CRC(2503c7da) SHA1(2478bab8b19ab68ff01be8fae2e86e47894b3d7c), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 19)" )
M4RFYM_SET( 199?, m4rfym__s,	m4rfym,	"ap502s.p1",	0x0000, 0x010000, CRC(8502a09a) SHA1(e635552b7f0c7b2e142d7f4d0f1fd93edac6132d), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 20)" )
M4RFYM_SET( 199?, m4rfym__t,	m4rfym,	"ap502y.p1",	0x0000, 0x010000, CRC(b868ef34) SHA1(a773503afd2f59b71e0b9a7e202d3e7120ec88ff), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 21)" )
M4RFYM_SET( 199?, m4rfym__u,	m4rfym,	"aprad.p1",		0x0000, 0x010000, CRC(936f59ac) SHA1(325708d965d56a9a7482dbeaa089ca871d5c01b5), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 22)" )
M4RFYM_SET( 199?, m4rfym__v,	m4rfym,	"aprb.p1",		0x0000, 0x010000, CRC(72ad662a) SHA1(11f1695e05ecf34a58f8df3ffbc72ab2dd7d02c9), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 23)" )
M4RFYM_SET( 199?, m4rfym__w,	m4rfym,	"aprbd.p1",		0x0000, 0x010000, CRC(13af990d) SHA1(604d2173e3d6d25252b30b5bf386b53470c35581), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 24)" )
M4RFYM_SET( 199?, m4rfym__x,	m4rfym,	"aprc.p1",		0x0000, 0x010000, CRC(fd3ece9a) SHA1(e11d1d258a415865f7477cdfddcd47e9bdb1c9b5), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 25)" )
M4RFYM_SET( 199?, m4rfym__y,	m4rfym,	"aprd.p1",		0x0000, 0x010000, CRC(8c19b732) SHA1(e7aeea41cf649fe2a28414ddedacdf72f56d32fe), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 26)" )
M4RFYM_SET( 199?, m4rfym__z,	m4rfym,	"aprdk.p1",		0x0000, 0x010000, CRC(58a41fcd) SHA1(e8c92dfb5c9662c90d363b5b7a7e0a4b4894d4cb), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 27)" )
M4RFYM_SET( 199?, m4rfym__0,	m4rfym,	"aprdy.p1",		0x0000, 0x010000, CRC(9496cfad) SHA1(cb24779db99d283f1df86864886f21ad333cb98b), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 28)" )
M4RFYM_SET( 199?, m4rfym__1,	m4rfym,	"aprk.p1",		0x0000, 0x010000, CRC(7277ef07) SHA1(dc509d125f8d377d4b2cb011d32be5bdba1daa17), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 29)" )
M4RFYM_SET( 199?, m4rfym__2,	m4rfym,	"aprs.p1",		0x0000, 0x010000, CRC(a114a96a) SHA1(b0a9091cac86750329513a0927dd39b76995b2f2), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 30)" )
M4RFYM_SET( 199?, m4rfym__3,	m4rfym,	"apry.p1",		0x0000, 0x010000, CRC(bf2120bc) SHA1(473374a9510dd53e39b94bfcf1369e13647239e6), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 31)" )
M4RFYM_SET( 199?, m4rfym__4,	m4rfym,	"rfym20",		0x0000, 0x010000, CRC(5e1d70e2) SHA1(2da1b8033a77d367c4b5c3d83a0e5def4e5e5d78), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 32)" )
M4RFYM_SET( 199?, m4rfym__5,	m4rfym,	"rfym2010",		0x0000, 0x010000, CRC(ec440e7e) SHA1(21f8d4708b5d779dcefcc1e921a5efe17dd6f8c7), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 33)" )
M4RFYM_SET( 199?, m4rfym__6,	m4rfym,	"rfym510l",		0x0000, 0x010000, CRC(24af47f3) SHA1(3d1ec9b013f3f7b497cfb62b42fbb2fa914b24b6), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 34)" )
M4RFYM_SET( 199?, m4rfym__7,	m4rfym,	"rfym55",		0x0000, 0x010000, CRC(b7d638d8) SHA1(6064ceffd94ff149d8bcb117fd823de52030ac64), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 35)" )
M4RFYM_SET( 199?, m4rfym__8,	m4rfym,	"ru5ad.p1",		0x0000, 0x010000, CRC(1c3e1f39) SHA1(a45cdaaa875e52cf5cd5adf986c98f4a22a14785), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 36)" )
M4RFYM_SET( 199?, m4rfym__9,	m4rfym,	"ru5b.p1",		0x0000, 0x010000, CRC(41e44d37) SHA1(8eb409b96864fb0f7c3bf5c66a20a63c8cbc68af), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 37)" )
M4RFYM_SET( 199?, m4rfym__aa,	m4rfym,	"ru5bd.p1",		0x0000, 0x010000, CRC(8d4db415) SHA1(b023a13f89b7e5c2f72fd213179f723621871faf), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 38)" )
M4RFYM_SET( 199?, m4rfym__ab,	m4rfym,	"ru5d.p1",		0x0000, 0x010000, CRC(fcb70a63) SHA1(df81c3c26c066c1326b20b9e0dda2863ee9635a6), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 39)" )
M4RFYM_SET( 199?, m4rfym__ac,	m4rfym,	"ru5dk.p1",		0x0000, 0x010000, CRC(b4d83863) SHA1(02aebf94773d0a9454119b4ad663b6d8475fc8d3), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 40)" )
M4RFYM_SET( 199?, m4rfym__ad,	m4rfym,	"ru5dy.p1",		0x0000, 0x010000, CRC(66375af5) SHA1(0a6d10357c163e5e27e7436f8190070e36e3ef90), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 41)" )
M4RFYM_SET( 199?, m4rfym__ae,	m4rfym,	"ru5k.p1",		0x0000, 0x010000, CRC(7871c141) SHA1(e1e9d2972c87d2835b1e5a62502160cb4abb7736), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 42)" )
M4RFYM_SET( 199?, m4rfym__af,	m4rfym,	"ru5s.p1",		0x0000, 0x010000, CRC(41795ea3) SHA1(6bfb6da6c0f7e762d628ce8a9dcdcbc3c0326ca6), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 43)" )
M4RFYM_SET( 199?, m4rfym__ag,	m4rfym,	"ru5y.p1",		0x0000, 0x010000, CRC(ee217541) SHA1(68474c2e430d95ded2856183b9a02be917d092d6), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 44)" )
M4RFYM_SET( 199?, m4rfym__ah,	m4rfym,	"ru8c.p1",		0x0000, 0x010000, CRC(93290724) SHA1(37b17b08f77b308289d4392900576dc66a0377eb), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 45)" )
M4RFYM_SET( 199?, m4rfym__ai,	m4rfym,	"ru8d.p1",		0x0000, 0x010000, CRC(3e7d6ebb) SHA1(a836a52aef9fe4a9021835e99109b7fefb4ead76), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 46)" )
M4RFYM_SET( 199?, m4rfym__aj,	m4rfym,	"ru8dk.p1",		0x0000, 0x010000, CRC(b2983dc1) SHA1(412bf4a643c807371fa465fb5f9a85bc3e46623d), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 47)" )
M4RFYM_SET( 199?, m4rfym__ak,	m4rfym,	"ru8dy.p1",		0x0000, 0x010000, CRC(7d06cdcc) SHA1(d68f6ee59eb7689df30412288db4e9ee6c4bf178), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 48)" )
M4RFYM_SET( 199?, m4rfym__al,	m4rfym,	"ru8k.p1",		0x0000, 0x010000, CRC(42f6226e) SHA1(c4bac8efd9c17f96dd9d973e9f64c85ceeacb36b), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 49)" )
M4RFYM_SET( 199?, m4rfym__am,	m4rfym,	"ru8s.p1",		0x0000, 0x010000, CRC(d6ce5891) SHA1(c130e7bf614c67767c9af6f38e3cd41ce63d11ef), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 50)" )
M4RFYM_SET( 199?, m4rfym__an,	m4rfym,	"ru8y.p1",		0x0000, 0x010000, CRC(f1fc1e75) SHA1(f6f1008349505ee0c494fcdde27db2a15147b6cb), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 51)" )
M4RFYM_SET( 199?, m4rfym__ao,	m4rfym,	"runc.p1",		0x0000, 0x010000, CRC(09f53ddf) SHA1(f46be95bfacac751102a5f4d4a0917a5e51a653e), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 52)" )
M4RFYM_SET( 199?, m4rfym__ap,	m4rfym,	"rundy.p1",		0x0000, 0x010000, CRC(a6f69a24) SHA1(8370287dcc890fcb7529d3d4c7a3c2e2e688f6a8), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 53)" )
M4RFYM_SET( 199?, m4rfym__aq,	m4rfym,	"runk.p1",		0x0000, 0x010000, CRC(a2828b82) SHA1(0ae371a441df679fd9c699771ae9f58ce960d4a1), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 54)" )
M4RFYM_SET( 199?, m4rfym__ar,	m4rfym,	"runs.p1",		0x0000, 0x010000, CRC(e20f5a06) SHA1(f0f71f8870db7003fce96f1dfe09804cf17c3ab3), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 55)" )
M4RFYM_SET( 199?, m4rfym__as,	m4rfym,	"runy.p1",		0x0000, 0x010000, CRC(0e311ab4) SHA1(c98540c07e9cc23ec70ecfbcb2f4d66f2c716fc3), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 56)" )
M4RFYM_SET( 199?, m4rfym__at,	m4rfym,	"rutad.p1",		0x0000, 0x010000, CRC(f27090c9) SHA1(28b7bb8046f67a3f8b90069de845b0b791b57078), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 57)" )
M4RFYM_SET( 199?, m4rfym__au,	m4rfym,	"rutb.p1",		0x0000, 0x010000, CRC(cb7a74bf) SHA1(24274c7e3b40642d698f5c3a9a10cfeb23faaf1b), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 58)" )
M4RFYM_SET( 199?, m4rfym__av,	m4rfym,	"rutbd.p1",		0x0000, 0x010000, CRC(19aba8f2) SHA1(cb726130837149c25adb5d87718b72259cb63a63), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 59)" )
M4RFYM_SET( 199?, m4rfym__aw,	m4rfym,	"rutd.p1",		0x0000, 0x010000, CRC(16a872bd) SHA1(47ad5eb9b473805e2eb86e0d4d9ef4b2e6e3c926), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 60)" )
M4RFYM_SET( 199?, m4rfym__ax,	m4rfym,	"rutdk.p1",		0x0000, 0x010000, CRC(a8259673) SHA1(443081395ea0c1b0a07e6cd4b17670b3e01bb50f), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 61)" )
M4RFYM_SET( 199?, m4rfym__ay,	m4rfym,	"rutdy.p1",		0x0000, 0x010000, CRC(6b799f68) SHA1(87482236f1116983e80a7f190710524d3809cd3a), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 62)" )
M4RFYM_SET( 199?, m4rfym__az,	m4rfym,	"rutk.p1",		0x0000, 0x010000, CRC(20962e5e) SHA1(0be43050d403750b67c796a007b503e132014f4c), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 63)" )
M4RFYM_SET( 199?, m4rfym__a0,	m4rfym,	"ruts.p1",		0x0000, 0x010000, CRC(efaf4e03) SHA1(da19d6e28a6727eb9afb69c23fd5685f0dbcc31a), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 64)" )
M4RFYM_SET( 199?, m4rfym__a1,	m4rfym,	"ruty.p1",		0x0000, 0x010000, CRC(abb708c5) SHA1(6fe3b52a0ba484576fc83ed35aefeda01d275aec), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 65)" )
M4RFYM_SET( 199?, m4rfym__a2,	m4rfym,	"rfym20.10",	0x0000, 0x010000, CRC(947d00d2) SHA1(2c99da689541de247e35ac39eadfe070ac3196b5), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 66)" )
M4RFYM_SET( 199?, m4rfym__a3,	m4rfym,	"rfym5.10",		0x0000, 0x010000, CRC(c2ce2cc2) SHA1(d5633e01f669ee8772ed77befa90180c6aa0111c), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 67)" )
M4RFYM_SET( 199?, m4rfym__a4,	m4rfym,	"rfym5.4",		0x0000, 0x010000, CRC(fe613006) SHA1(898b90893bfcb121575952c22c16570a27948bce), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 68)" )
M4RFYM_SET( 199?, m4rfym__a5,	m4rfym,	"rfym5.8t",		0x0000, 0x010000, CRC(c600718a) SHA1(168fa558f1b5b91fb805d483f3f4351ac80f90ff), "Barcrest","Run For Your Money (Barcrest) (MPU4) (set 69)" )


#define M4READY_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "rgosnd.p1", 0x000000, 0x080000, CRC(d9345794) SHA1(4ed060fe61b3530e88ba9afea1fb69efed47c955) ) \
	ROM_LOAD( "rgosnd.p2", 0x080000, 0x080000, CRC(4656f94e) SHA1(2f276ced34a43bb7fc69304f519b913d699c3450) ) \

#define M4READY_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4READY_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4READY_SET( 199?, m4ready,		0,			"rgob.p1",		0x0000, 0x010000, CRC(43ac7b73) SHA1(994d6256432543e1353521359f8faaea671a7bea), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 1)" )
M4READY_SET( 199?, m4ready__a,	m4ready,	"cgo11ad.p1",	0x0000, 0x010000, CRC(9f8bbdaf) SHA1(210cdc9ce493edbf55d43a3127b10931e3ce2fee), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 2)" )
M4READY_SET( 199?, m4ready__b,	m4ready,	"cgo11b.p1",	0x0000, 0x010000, CRC(2ea96acb) SHA1(ffcf1fcb2b769b29b53b00c9ce80af061cc21b9d), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 3)" )
M4READY_SET( 199?, m4ready__c,	m4ready,	"cgo11bd.p1",	0x0000, 0x010000, CRC(4cabc589) SHA1(2b0b91f4ac6ebd18edb7a913b8079acc9f026e7d), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 4)" )
M4READY_SET( 199?, m4ready__d,	m4ready,	"cgo11c.p1",	0x0000, 0x010000, CRC(76d36b80) SHA1(2699982fed3c2116ff0187d24059f59d3b6c1cae), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 5)" )
M4READY_SET( 199?, m4ready__e,	m4ready,	"cgo11d.p1",	0x0000, 0x010000, CRC(63516954) SHA1(abefafe43e3386a5c916e55503bcb623d74840e1), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 6)" )
M4READY_SET( 199?, m4ready__f,	m4ready,	"cgo11dk.p1",	0x0000, 0x010000, CRC(84f112ef) SHA1(85fa44c7b25aeb83fa2c199abafe099a8ae92bf8), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 7)" )
M4READY_SET( 199?, m4ready__g,	m4ready,	"cgo11dr.p1",	0x0000, 0x010000, CRC(07d13cf6) SHA1(11685efebf9c7091191654fec1f2ac6ad3d05ce1), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 8)" )
M4READY_SET( 199?, m4ready__h,	m4ready,	"cgo11dy.p1",	0x0000, 0x010000, CRC(13c5b934) SHA1(3212ba2534726c8fca9a70325acff3f6e85dd1f7), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 9)" )
M4READY_SET( 199?, m4ready__i,	m4ready,	"cgo11k.p1",	0x0000, 0x010000, CRC(4f46e7f6) SHA1(9485edbcbb3a81b1a335a7c420aa676af8b14050), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 10)" )
M4READY_SET( 199?, m4ready__j,	m4ready,	"cgo11r.p1",	0x0000, 0x010000, CRC(f44dd36f) SHA1(6623daaa237e97b9d63815393562fe8abdb8d732), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 11)" )
M4READY_SET( 199?, m4ready__k,	m4ready,	"cgo11s.p1",	0x0000, 0x010000, CRC(a6b9ddd4) SHA1(b06d5d19b165b82c76b29f7925e0936aeccedb8c), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 12)" )
M4READY_SET( 199?, m4ready__l,	m4ready,	"cgo11y.p1",	0x0000, 0x010000, CRC(d91653f6) SHA1(6445958cd07088fbf08c37a8b5540e3eb561d021), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 13)" )
M4READY_SET( 199?, m4ready__m,	m4ready,	"drr02ad.p1",	0x0000, 0x010000, CRC(5acc5189) SHA1(abf66b90f4a64c3fb9ac4bf16f3bba2758f54482), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 14)" )
M4READY_SET( 199?, m4ready__n,	m4ready,	"drr02b.p1",	0x0000, 0x010000, CRC(729e13c9) SHA1(dcefdd44592464616570101a5e05db31289fc66c), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 15)" )
M4READY_SET( 199?, m4ready__o,	m4ready,	"drr02bd.p1",	0x0000, 0x010000, CRC(70c5b183) SHA1(b1431d0c2c48941d1ff6d6115c8d1ab026d71f63), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 16)" )
M4READY_SET( 199?, m4ready__p,	m4ready,	"drr02c.p1",	0x0000, 0x010000, CRC(258acbf9) SHA1(ced9dbef9162ddadb4838ad430d50aa14574e97d), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 17)" )
M4READY_SET( 199?, m4ready__q,	m4ready,	"drr02d.p1",	0x0000, 0x010000, CRC(60940b5a) SHA1(a4d293944e0e65f99dea9391d9d7e1066aa7b83d), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 18)" )
M4READY_SET( 199?, m4ready__r,	m4ready,	"drr02dk.p1",	0x0000, 0x010000, CRC(0335775e) SHA1(4d943c3e522f5c42ddd2104c316f75eec90f494f), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 19)" )
M4READY_SET( 199?, m4ready__s,	m4ready,	"drr02dr.p1",	0x0000, 0x010000, CRC(c05eef66) SHA1(ac5966ea0ff036d9c9179df6bc7aabd149f41d6c), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 20)" )
M4READY_SET( 199?, m4ready__t,	m4ready,	"drr02dy.p1",	0x0000, 0x010000, CRC(6a700473) SHA1(4025a99aa9e87a80875d150e965650d339d2a143), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 21)" )
M4READY_SET( 199?, m4ready__u,	m4ready,	"drr02k.p1",	0x0000, 0x010000, CRC(525e370e) SHA1(9849399643731beb31b7163b7eebd8774caf9289), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 22)" )
M4READY_SET( 199?, m4ready__v,	m4ready,	"drr02r.p1",	0x0000, 0x010000, CRC(352613a0) SHA1(052e7770d55dd379d1bf3501e46d973bc4fc48d8), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 23)" )
M4READY_SET( 199?, m4ready__w,	m4ready,	"drr02s.p1",	0x0000, 0x010000, CRC(67b03b7f) SHA1(61e09db8b7622e6e094c4e585dbcfea724155829), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 24)" )
M4READY_SET( 199?, m4ready__x,	m4ready,	"drr02y.p1",	0x0000, 0x010000, CRC(009c7ece) SHA1(48463d7d0e521d51bad83ac5ddaaffabc68bf610), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 25)" )
M4READY_SET( 199?, m4ready__y,	m4ready,	"hjj.hex",		0x0000, 0x010000, CRC(48ab2375) SHA1(4d9360a89e97a6bb7bdb099940d73f425eadd63d), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 26)" )
M4READY_SET( 199?, m4ready__z,	m4ready,	"hjj02ad.p1",	0x0000, 0x010000, CRC(9f787e01) SHA1(d6cae1c1ae15b74285076e87c7fd8105f6a114ae), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 27)" )
M4READY_SET( 199?, m4ready__0,	m4ready,	"hjj02b.p1",	0x0000, 0x010000, CRC(778ec121) SHA1(98454562da1da56d57ce3e6279805207671d7337), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 28)" )
M4READY_SET( 199?, m4ready__1,	m4ready,	"hjj02bd.p1",	0x0000, 0x010000, CRC(7e8dbab0) SHA1(5b40536503b2d62792f874535367f5658acf8d2e), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 29)" )
M4READY_SET( 199?, m4ready__2,	m4ready,	"hjj02c.p1",	0x0000, 0x010000, CRC(fbb149fc) SHA1(6a8305a3ef4a1818a12dab3d380e79b7e642a904), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 30)" )
M4READY_SET( 199?, m4ready__3,	m4ready,	"hjj02d.p1",	0x0000, 0x010000, CRC(9e657e28) SHA1(3eaf9f8a0511f4533e9b47105d4417c71248fab2), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 31)" )
M4READY_SET( 199?, m4ready__4,	m4ready,	"hjj02dk.p1",	0x0000, 0x010000, CRC(2e1bab77) SHA1(76a2784bc183c6d79a845bb7306eae687ced82a0), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 32)" )
M4READY_SET( 199?, m4ready__5,	m4ready,	"hjj02dr.p1",	0x0000, 0x010000, CRC(9d26064f) SHA1(6596b3a671ab8e38b8357023f8994948ef1c1f0f), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 33)" )
M4READY_SET( 199?, m4ready__6,	m4ready,	"hjj02dy.p1",	0x0000, 0x010000, CRC(0ab388b6) SHA1(cc8f157a8a91e3fb8bd1fbdd35989d72c8684c50), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 34)" )
M4READY_SET( 199?, m4ready__7,	m4ready,	"hjj02k.p1",	0x0000, 0x010000, CRC(c224c58a) SHA1(5f9b5ff92e2f1b0438380d635b255ec8b4fc080f), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 35)" )
M4READY_SET( 199?, m4ready__8,	m4ready,	"hjj02r.p1",	0x0000, 0x010000, CRC(32fefefe) SHA1(f58e228a1496b0858903c2d850c8453835b6f24b), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 36)" )
M4READY_SET( 199?, m4ready__9,	m4ready,	"hjj02s.p1",	0x0000, 0x010000, CRC(39de9801) SHA1(c29e883c45ed6b272d65c7922b1871199a424244), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 37)" )
M4READY_SET( 199?, m4ready__aa,	m4ready,	"hjj02y.p1",	0x0000, 0x010000, CRC(0178cc91) SHA1(d618ff2eb0a1992b88f3b5427ffc54d34bf8c124), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 38)" )
M4READY_SET( 199?, m4ready__ab,	m4ready,	"ppl02ad.p1",	0x0000, 0x010000, CRC(d8b3be27) SHA1(95d1d979b439303817670fd686b5df324feb618f), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 39)" )
M4READY_SET( 199?, m4ready__ac,	m4ready,	"ppl02b.p1",	0x0000, 0x010000, CRC(dbd56cf1) SHA1(968c1d09626e493c51d7637e19a7f092047b283f), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 40)" )
M4READY_SET( 199?, m4ready__ad,	m4ready,	"ppl02bd.p1",	0x0000, 0x010000, CRC(eeafd36d) SHA1(68c314e937d24a59ca305facc409218c63bef24e), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 41)" )
M4READY_SET( 199?, m4ready__ae,	m4ready,	"ppl02c.p1",	0x0000, 0x010000, CRC(ad6f5c6d) SHA1(91f3d7bad3cdb7014ff3caa1631e6567cb95f47e), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 42)" )
M4READY_SET( 199?, m4ready__af,	m4ready,	"ppl02d.p1",	0x0000, 0x010000, CRC(041f0fd8) SHA1(8e32c88f7b0a541a9460926a2fec0318a7239279), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 43)" )
M4READY_SET( 199?, m4ready__ag,	m4ready,	"ppl02dk.p1",	0x0000, 0x010000, CRC(632ecd46) SHA1(9e90279db99aa22923a79e309b053b35b70c9f8e), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 44)" )
M4READY_SET( 199?, m4ready__ah,	m4ready,	"ppl02dy.p1",	0x0000, 0x010000, CRC(4fb07726) SHA1(f234018ea18511217d176023b489254cf5a5a15e), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 45)" )
M4READY_SET( 199?, m4ready__ai,	m4ready,	"ppl02k.p1",	0x0000, 0x010000, CRC(7fcc03d7) SHA1(359743edec7ca54bd9a780f81ac25d314ada2d7e), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 46)" )
M4READY_SET( 199?, m4ready__aj,	m4ready,	"ppl02r.p1",	0x0000, 0x010000, CRC(e35580e3) SHA1(397bd2ce068aa45f3c55a3ddd97ae3e09391a7da), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 47)" )
M4READY_SET( 199?, m4ready__ak,	m4ready,	"ppl02s.p1",	0x0000, 0x010000, CRC(40c1d256) SHA1(abd55dcc06b49d54976743c610ad3de21278ac2d), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 48)" )
M4READY_SET( 199?, m4ready__al,	m4ready,	"ppl02y.p1",	0x0000, 0x010000, CRC(7802f70c) SHA1(c96758c02bebff4b85436a93ae012c80c6cb2963), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 49)" )
M4READY_SET( 199?, m4ready__am,	m4ready,	"rgoad.p1",		0x0000, 0x010000, CRC(d4ed739c) SHA1(6a7d5f63eaf59f08a8f870aba8523e2dc59d20cd), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 50)" )
M4READY_SET( 199?, m4ready__an,	m4ready,	"rgobd.p1",		0x0000, 0x010000, CRC(0505340c) SHA1(e61b007dc50beb22bf3efa2c3cfab595880d3248), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 51)" )
M4READY_SET( 199?, m4ready__ao,	m4ready,	"rgod.p1",		0x0000, 0x010000, CRC(f3898077) SHA1(4d2f32b4c3f01a0b54966dd0558dcadcf89fd229), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 52)" )
M4READY_SET( 199?, m4ready__ap,	m4ready,	"rgodk.p1",		0x0000, 0x010000, CRC(9a9b61c7) SHA1(756ed419451d1e070809303467789e01949dea2b), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 53)" )
M4READY_SET( 199?, m4ready__aq,	m4ready,	"rgody.p1",		0x0000, 0x010000, CRC(a0eef0f0) SHA1(781de603d19eab0ee771b10374f53c149432c877), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 54)" )
M4READY_SET( 199?, m4ready__ar,	m4ready,	"rgok.p1",		0x0000, 0x010000, CRC(00413e8f) SHA1(580efbdf3ba092978648d83b6d21b5a4966d57e3), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 55)" )
M4READY_SET( 199?, m4ready__as,	m4ready,	"rgos.p1",		0x0000, 0x010000, CRC(d00d3540) SHA1(0fd6a08477d05d1c129038c8de47de68a28c0a56), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 56)" )
M4READY_SET( 199?, m4ready__at,	m4ready,	"rgoy.p1",		0x0000, 0x010000, CRC(cfdfce82) SHA1(68464381f658f08efb3f790eea1e7dd61086f936), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 57)" )
M4READY_SET( 199?, m4ready__au,	m4ready,	"rgt10ad.p1",	0x0000, 0x010000, CRC(22f65e05) SHA1(d488e4c65059b3b7e8e88e39a05e0cc9eae2d836), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 58)" )
M4READY_SET( 199?, m4ready__av,	m4ready,	"rgt10b.p1",	0x0000, 0x010000, CRC(5d86b45a) SHA1(a7553848a1e4304acaf72f9d293123ca2af629f0), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 59)" )
M4READY_SET( 199?, m4ready__aw,	m4ready,	"rgt10bd.p1",	0x0000, 0x010000, CRC(6280594e) SHA1(d2b666aaac8ebe94bfab1c4404d0e42bd6c8b176), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 60)" )
M4READY_SET( 199?, m4ready__ax,	m4ready,	"rgt10c.p1",	0x0000, 0x010000, CRC(91f28aa6) SHA1(42c21d11df3145a0919f7aef53a5621b2beca353), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 61)" )
M4READY_SET( 199?, m4ready__ay,	m4ready,	"rgt10d.p1",	0x0000, 0x010000, CRC(abd4a67e) SHA1(183746cd2cd661587854a80ad5455074fcf143cc), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 62)" )
M4READY_SET( 199?, m4ready__az,	m4ready,	"rgt10dk.p1",	0x0000, 0x010000, CRC(63ee9525) SHA1(e1fa5348672d05149e6ab26f31af047e38192f2c), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 63)" )
M4READY_SET( 199?, m4ready__a0,	m4ready,	"rgt10dr.p1",	0x0000, 0x010000, CRC(481ffebc) SHA1(3608faa929b703d2e45ea37b4f7051d5bb37f073), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 64)" )
M4READY_SET( 199?, m4ready__a1,	m4ready,	"rgt10dy.p1",	0x0000, 0x010000, CRC(fe2498e9) SHA1(bd81c775daf860c2484af88a8b11b75df00ccaaa), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 65)" )
M4READY_SET( 199?, m4ready__a2,	m4ready,	"rgt10k.p1",	0x0000, 0x010000, CRC(78d6eff1) SHA1(d3172ffc9ef3a4f60680081d993e1487e4229625), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 66)" )
M4READY_SET( 199?, m4ready__a3,	m4ready,	"rgt10r.p1",	0x0000, 0x010000, CRC(1992945e) SHA1(716b62ca8edc5523fd83355e650982b50b4f9458), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 67)" )
M4READY_SET( 199?, m4ready__a4,	m4ready,	"rgt10s.p1",	0x0000, 0x010000, CRC(dd289204) SHA1(431f73cb45d248c672c50dc8fbc579209e41207d), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 68)" )
M4READY_SET( 199?, m4ready__a5,	m4ready,	"rgt10y.p1",	0x0000, 0x010000, CRC(8dab3aca) SHA1(0fe8f87a17acd8df0b7b75b852b58eb1e273eb27), "Barcrest","Ready Steady Go (Barcrest) (type 2) (MPU4) (set 69)" )


#define M4MAG7S_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "mag7.chr", 0x0000, 0x000048, CRC(4835e911) SHA1(7171cdabf6cf76e09ea55b41f0f8a98b94032486) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "ma7snd1.bin", 0x000000, 0x080000, CRC(f0e31329) SHA1(60b94c3223c8863fe801b93f65ff65e94f3dec83) ) \
	ROM_LOAD( "ma7snd2.bin", 0x080000, 0x080000, CRC(12110d16) SHA1(fa93a263d1e3fa8b0b2f618f52e5145330f4315d) ) \

#define M4MAG7S_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MAG7S_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4jackpot8per , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4MAG7S_SET( 199?, m4mag7s,		0,			"mas12y.p1",		0x0000, 0x020000, CRC(5f012d8e) SHA1(069b493285df9ac3639c43349245a77890333dcc), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 1)" )
M4MAG7S_SET( 199?, m4mag7s__a,	m4mag7s,	"ma714s",			0x0000, 0x020000, CRC(9c1d4f97) SHA1(7875f044f992b313f4dfaae2e7b604baf16387a3), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 2)" )
M4MAG7S_SET( 199?, m4mag7s__b,	m4mag7s,	"ma715ad.p1",		0x0000, 0x020000, CRC(f807cc3f) SHA1(d402a1bf6b9a69d26b0806da83d6a943760aa6ed), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 3)" )
M4MAG7S_SET( 199?, m4mag7s__c,	m4mag7s,	"ma715b.p1",		0x0000, 0x020000, CRC(bedfd8b5) SHA1(a2bcd42e7163779aa7bb74ddc4a44d07f1179994), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 4)" )
M4MAG7S_SET( 199?, m4mag7s__d,	m4mag7s,	"ma715bd.p1",		0x0000, 0x020000, CRC(b2c06469) SHA1(00ed3998c24c7f03fc270e3025d3815163b6ff38), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 5)" )
M4MAG7S_SET( 199?, m4mag7s__e,	m4mag7s,	"ma715d.p1",		0x0000, 0x020000, CRC(9d4c2afe) SHA1(2833f9de808b2630ca67405098c3c502d597b6ca), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 6)" )
M4MAG7S_SET( 199?, m4mag7s__f,	m4mag7s,	"ma715dh.p1",		0x0000, 0x020000, CRC(9615f2ba) SHA1(ac6bc67bc740b6efd3050f9f5bc553acc25d9be6), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 7)" )
M4MAG7S_SET( 199?, m4mag7s__g,	m4mag7s,	"ma715dk.p1",		0x0000, 0x020000, CRC(4b08770e) SHA1(e748b800804cd2505d10e5e7168a3dffaf007c7c), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 8)" )
M4MAG7S_SET( 199?, m4mag7s__h,	m4mag7s,	"ma715dr.p1",		0x0000, 0x020000, CRC(d1e6a9b7) SHA1(d5c67cbc07931831159c801e0f4abfdc477980b8), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 9)" )
M4MAG7S_SET( 199?, m4mag7s__i,	m4mag7s,	"ma715dy.p1",		0x0000, 0x020000, CRC(7fe4ecc1) SHA1(ce4abc96d8d50683bee31c9bce7835dd76065a9b), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 10)" )
M4MAG7S_SET( 199?, m4mag7s__j,	m4mag7s,	"ma715h.p1",		0x0000, 0x020000, CRC(9a0a4e66) SHA1(ef8acb1f2deda724ff7b2850b1ce7a09f88ac011), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 11)" )
M4MAG7S_SET( 199?, m4mag7s__k,	m4mag7s,	"ma715k.p1",		0x0000, 0x020000, CRC(4717cbd2) SHA1(b88b17dda9e16ecbcce55bdb99c23d9e8267b7b9), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 12)" )
M4MAG7S_SET( 199?, m4mag7s__l,	m4mag7s,	"ma715r.p1",		0x0000, 0x020000, CRC(ddf9156b) SHA1(b8fb10944d910cd72a3268a80e8cd1b07dbee8f3), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 13)" )
M4MAG7S_SET( 199?, m4mag7s__m,	m4mag7s,	"ma715s.p1",		0x0000, 0x020000, CRC(6518c171) SHA1(df884c875f3cfb8e12fb35550dd1f5b331e4b204), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 14)" )
M4MAG7S_SET( 199?, m4mag7s__n,	m4mag7s,	"ma715y.p1",		0x0000, 0x020000, CRC(73fb501d) SHA1(2449da89e811ebf27970a8a9336107f85d876229), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 15)" )
M4MAG7S_SET( 199?, m4mag7s__o,	m4mag7s,	"ma716ad.p1",		0x0000, 0x020000, CRC(7632ce64) SHA1(2308d777110aa7636c4c0fc08be23d1732ba3b69), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 16)" )
M4MAG7S_SET( 199?, m4mag7s__p,	m4mag7s,	"ma716b.p1",		0x0000, 0x020000, CRC(cc8e289a) SHA1(7f5bac0374ff0eb0395f8d3c18ddad82c3e9c51d), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 17)" )
M4MAG7S_SET( 199?, m4mag7s__q,	m4mag7s,	"ma716bd.p1",		0x0000, 0x020000, CRC(3cf56632) SHA1(43be834b4d2d3a7fb9893e966ee909971319a10e), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 18)" )
M4MAG7S_SET( 199?, m4mag7s__r,	m4mag7s,	"ma716d.p1",		0x0000, 0x020000, CRC(ef1ddad1) SHA1(67f865856ba497e712a93ae3d2d308fe7e17fc01), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 19)" )
M4MAG7S_SET( 199?, m4mag7s__s,	m4mag7s,	"ma716dh.p1",		0x0000, 0x020000, CRC(1820f0e1) SHA1(4592cb213002d28450ac3da748a77501483a6816), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 20)" )
M4MAG7S_SET( 199?, m4mag7s__t,	m4mag7s,	"ma716dk.p1",		0x0000, 0x020000, CRC(c53d7555) SHA1(4d9ab16b7d28cf99fb8cbe3dc84295c634f7460e), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 21)" )
M4MAG7S_SET( 199?, m4mag7s__u,	m4mag7s,	"ma716dr.p1",		0x0000, 0x020000, CRC(5fd3abec) SHA1(a0187c62ed1949400c3e1d82071b2d500192bc90), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 22)" )
M4MAG7S_SET( 199?, m4mag7s__v,	m4mag7s,	"ma716dy.p1",		0x0000, 0x020000, CRC(f1d1ee9a) SHA1(d67d47891235624b4606244b275fbfee56517c77), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 23)" )
M4MAG7S_SET( 199?, m4mag7s__w,	m4mag7s,	"ma716h.p1",		0x0000, 0x020000, CRC(e85bbe49) SHA1(83ff53b1f4d2c14d313905b3194139089b1f0363), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 24)" )
M4MAG7S_SET( 199?, m4mag7s__x,	m4mag7s,	"ma716k.p1",		0x0000, 0x020000, CRC(35463bfd) SHA1(c288189f5ca7f00d1ae66521f77976275da39cc1), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 25)" )
M4MAG7S_SET( 199?, m4mag7s__y,	m4mag7s,	"ma716r.p1",		0x0000, 0x020000, CRC(afa8e544) SHA1(83f7d80ec359ba832caa9814e9a9fa04174ae596), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 26)" )
M4MAG7S_SET( 199?, m4mag7s__z,	m4mag7s,	"ma716s.p1",		0x0000, 0x020000, CRC(30fd2e9f) SHA1(9ed06ee736a09b36f48fb3b69be03b39861b0ea5), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 27)" )
M4MAG7S_SET( 199?, m4mag7s__0,	m4mag7s,	"ma716y.p1",		0x0000, 0x020000, CRC(01aaa032) SHA1(9b27b5b90cbf89e537110964837507cff4573094), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 28)" )
M4MAG7S_SET( 199?, m4mag7s__1,	m4mag7s,	"mag715g",			0x0000, 0x020000, CRC(5a28a94b) SHA1(16621ac2294dd7f0b9e58125ba800331e879f39e), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 29)" )
M4MAG7S_SET( 199?, m4mag7s__2,	m4mag7s,	"mag715t",			0x0000, 0x020000, CRC(cdea8e84) SHA1(ad38be8bb5e1247c53c7b48ec6dceebe3e757c9c), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 30)" )
M4MAG7S_SET( 199?, m4mag7s__3,	m4mag7s,	"mag7s1-6_15.bin",	0x0000, 0x020000, CRC(2a4d7328) SHA1(78b6b358e7ff3efd086512550e7690f59ee4b225), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 31)" )
M4MAG7S_SET( 199?, m4mag7s__4,	m4mag7s,	"mas12ad.p1",		0x0000, 0x020000, CRC(46c6522a) SHA1(e8bfa00afb0e07c524023a92c715f191ad26e759), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 32)" )
M4MAG7S_SET( 199?, m4mag7s__5,	m4mag7s,	"mas12b.p1",		0x0000, 0x020000, CRC(9225a526) SHA1(7c9139c45ddfbcbbabf59c28f17d19ce3b8f7c05), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 33)" )
M4MAG7S_SET( 199?, m4mag7s__6,	m4mag7s,	"mas12bd.p1",		0x0000, 0x020000, CRC(cbdcfcef) SHA1(11eb24b04535e8fc922bc802ab2f8af5e70881d5), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 34)" )
M4MAG7S_SET( 199?, m4mag7s__7,	m4mag7s,	"mas12c.p1",		0x0000, 0x020000, CRC(a56f1834) SHA1(9a3bcca0ff62100a36b1de06c621108910da51d8), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 35)" )
M4MAG7S_SET( 199?, m4mag7s__8,	m4mag7s,	"mas12d.p1",		0x0000, 0x020000, CRC(ef3eae50) SHA1(0eb26a6ffc963a84bfb11934bd9c1fecfec15f2a), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 36)" )
M4MAG7S_SET( 199?, m4mag7s__9,	m4mag7s,	"mas12dh.p1",		0x0000, 0x020000, CRC(0530acac) SHA1(43d73351b5002afbc0e14c49056c7cac17c93854), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 37)" )
M4MAG7S_SET( 199?, m4mag7s__aa,	m4mag7s,	"mas12dk.p1",		0x0000, 0x020000, CRC(811b54b9) SHA1(c54b50b64856615eb362e0de889b7d758866ee77), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 38)" )
M4MAG7S_SET( 199?, m4mag7s__ab,	m4mag7s,	"mas12dr.p1",		0x0000, 0x020000, CRC(3214ef88) SHA1(8f0ab310363882c92fb4d58197ee093822e9611c), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 39)" )
M4MAG7S_SET( 199?, m4mag7s__ac,	m4mag7s,	"mas12dy.p1",		0x0000, 0x020000, CRC(06f87447) SHA1(9df82a89c9846eb51a3719870b91dd675a97cbe6), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 40)" )
M4MAG7S_SET( 199?, m4mag7s__ad,	m4mag7s,	"mas12h.p1",		0x0000, 0x020000, CRC(5cc9f565) SHA1(833c77ebd895e884430193d14edf0882b5bc5a75), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 41)" )
M4MAG7S_SET( 199?, m4mag7s__ae,	m4mag7s,	"mas12k.p1",		0x0000, 0x020000, CRC(d8e20d70) SHA1(a411a06b89ff21ad7f55196f80dd2766a99bfc21), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 42)" )
M4MAG7S_SET( 199?, m4mag7s__af,	m4mag7s,	"mas12r.p1",		0x0000, 0x020000, CRC(6bedb641) SHA1(39b5a8dfb581ae6ed209c6f2a12c06b38ad861a3), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 43)" )
M4MAG7S_SET( 199?, m4mag7s__ag,	m4mag7s,	"mas12s.p1",		0x0000, 0x020000, CRC(0a94e574) SHA1(e4516638fb7f783e79cfcdbbef1188965351eae2), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 44)" )
M4MAG7S_SET( 199?, m4mag7s__ah,	m4mag7s,	"mas13ad.p1",		0x0000, 0x020000, CRC(f39f7e0e) SHA1(39453c09c2e8b84fce9e53fbe86244f2381e2ac5), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 45)" )
M4MAG7S_SET( 199?, m4mag7s__ai,	m4mag7s,	"mas13b.p1",		0x0000, 0x020000, CRC(71863b1f) SHA1(6214d485509f827dd7bbcd00855d6caaec13ba09), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 46)" )
M4MAG7S_SET( 199?, m4mag7s__aj,	m4mag7s,	"mas13bd.p1",		0x0000, 0x020000, CRC(7e85d0cb) SHA1(598338482bbe0e903983a4061b8f772cb24e4961), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 47)" )
M4MAG7S_SET( 199?, m4mag7s__ak,	m4mag7s,	"mas13c.p1",		0x0000, 0x020000, CRC(46cc860d) SHA1(cd6dda5a822ac451cb9d9dcc21fa961a4bd26a71), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 48)" )
M4MAG7S_SET( 199?, m4mag7s__al,	m4mag7s,	"mas13d.p1",		0x0000, 0x020000, CRC(0c9d3069) SHA1(160b242d01fcf752d4bce35bb1b1b81a44b6afa0), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 49)" )
M4MAG7S_SET( 199?, m4mag7s__am,	m4mag7s,	"mas13dh.p1",		0x0000, 0x020000, CRC(b0698088) SHA1(3b7999cb85661a63914d9eacdb3e0d223b245a3d), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 50)" )
M4MAG7S_SET( 199?, m4mag7s__an,	m4mag7s,	"mas13dk.p1",		0x0000, 0x020000, CRC(3442789d) SHA1(3ae7b8d18e7fdc5104ed43b35743094e25e2819e), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 51)" )
M4MAG7S_SET( 199?, m4mag7s__ao,	m4mag7s,	"mas13dr.p1",		0x0000, 0x020000, CRC(874dc3ac) SHA1(cd20aa0b794748d7b853d1e5c8bee49bec03efc8), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 52)" )
M4MAG7S_SET( 199?, m4mag7s__ap,	m4mag7s,	"mas13dy.p1",		0x0000, 0x020000, CRC(b3a15863) SHA1(d57365c3a1291ccf5019be1bb6b6245168c2bcb8), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 53)" )
M4MAG7S_SET( 199?, m4mag7s__aq,	m4mag7s,	"mas13h.p1",		0x0000, 0x020000, CRC(bf6a6b5c) SHA1(a89632a6f71d9a480193924e9e3d678bc06b6f3e), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 54)" )
M4MAG7S_SET( 199?, m4mag7s__ar,	m4mag7s,	"mas13k.p1",		0x0000, 0x020000, CRC(3b419349) SHA1(d72f6493a6f7631b44f27e9cde499bdd192b5e0a), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 55)" )
M4MAG7S_SET( 199?, m4mag7s__as,	m4mag7s,	"mas13r.p1",		0x0000, 0x020000, CRC(884e2878) SHA1(c4f070019116543f4683c088d915709c040145ff), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 56)" )
M4MAG7S_SET( 199?, m4mag7s__at,	m4mag7s,	"mas13s.p1",		0x0000, 0x020000, CRC(80ca53c0) SHA1(19e67a259fca2fca3990f032d7825d67309d47d3), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 57)" )
M4MAG7S_SET( 199?, m4mag7s__au,	m4mag7s,	"mas13y.p1",		0x0000, 0x020000, CRC(bca2b3b7) SHA1(ff48b91578230bc77529cb59fbcb7e3bd77b946d), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 58)" )
M4MAG7S_SET( 199?, m4mag7s__av,	m4mag7s,	"mas10w.p1",		0x0000, 0x020000, CRC(e2fcc14a) SHA1(69b37a2d130b34636d5ff8e646d2be6d7e8b19f9), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 59)" )
M4MAG7S_SET( 199?, m4mag7s__aw,	m4mag7s,	"m7_sj_dc.2r1",		0x0000, 0x020000, CRC(0eefd40c) SHA1(2c30bc42d23c7cfb0a382b47f7ed865865341e2f), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 60)" )
M4MAG7S_SET( 199?, m4mag7s__ax,	m4mag7s,	"m7_sja_c.2r1",		0x0000, 0x020000, CRC(3933772c) SHA1(5e9d12a9f58ce5129634b8a4c8c0f083031df295), "Barcrest","Magnificent 7s (Barcrest) (MPU4) (set 61)" )


#define M4MAKMNT_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "mamsnd.p1", 0x000000, 0x080000, CRC(8dc408e3) SHA1(48a9ffc5cf4fd04ed1320619ca915bbfa2406750) ) \
	ROM_LOAD( "mamsnd.p2", 0x080000, 0x080000, CRC(6034e17a) SHA1(11e044c87b5fc6461b0c6cfac5c419daee930d7b) ) \

#define M4MAKMNT_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4MAKMNT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4MAKMNT_SET( 199?, m4makmnt,		0,			"mams.p1",		0x0000, 0x020000, CRC(af08e1e6) SHA1(c7e87d351f67592084d758ee53ba4d354bb28866), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 1)" )
M4MAKMNT_SET( 199?, m4makmnt__a,	m4makmnt,	"mam04ad.p1",	0x0000, 0x020000, CRC(9b750bc7) SHA1(10a86f0a0d18ce0be502a9d36282f6b5eef0ece5), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 2)" )
M4MAKMNT_SET( 199?, m4makmnt__b,	m4makmnt,	"mam04b.p1",	0x0000, 0x020000, CRC(8f5cefa9) SHA1(fc0dfb67794d090ef15facd0f2b60e1d505b295f), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 3)" )
M4MAKMNT_SET( 199?, m4makmnt__c,	m4makmnt,	"mam04bd.p1",	0x0000, 0x020000, CRC(166fa502) SHA1(345ad131d8757445c549312350507a3a804ca20e), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 4)" )
M4MAKMNT_SET( 199?, m4makmnt__d,	m4makmnt,	"mam04c.p1",	0x0000, 0x020000, CRC(b81652bb) SHA1(3e8544ac4b3e2d3845a1608dc821608e2d87088f), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 5)" )
M4MAKMNT_SET( 199?, m4makmnt__e,	m4makmnt,	"mam04d.p1",	0x0000, 0x020000, CRC(f247e4df) SHA1(de4afd3058292e3aea1abd718da16b06691b5623), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 6)" )
M4MAKMNT_SET( 199?, m4makmnt__f,	m4makmnt,	"mam04dk.p1",	0x0000, 0x020000, CRC(5ca80d54) SHA1(a82e9ebeb83cddf63f8fef129da4168734abcd48), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 7)" )
M4MAKMNT_SET( 199?, m4makmnt__g,	m4makmnt,	"mam04dr.p1",	0x0000, 0x020000, CRC(efa7b665) SHA1(4cb48215d69ee75efa2eccb6edf1526d497dba9c), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 8)" )
M4MAKMNT_SET( 199?, m4makmnt__h,	m4makmnt,	"mam04dy.p1",	0x0000, 0x020000, CRC(db4b2daa) SHA1(ba2fce6d7b6aa95a11f0054720e577db7415f1d2), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 9)" )
M4MAKMNT_SET( 199?, m4makmnt__i,	m4makmnt,	"mam04k.p1",	0x0000, 0x020000, CRC(c59b47ff) SHA1(8022c6c988532d3f98edf5c59c97403ab2d0fb90), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 10)" )
M4MAKMNT_SET( 199?, m4makmnt__j,	m4makmnt,	"mam04r.p1",	0x0000, 0x020000, CRC(7694fcce) SHA1(a67c76551240129914cc071543db96962f3b198f), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 11)" )
M4MAKMNT_SET( 199?, m4makmnt__k,	m4makmnt,	"mam04s.p1",	0x0000, 0x020000, CRC(08eac690) SHA1(e35793da266bd9dd8a018ba9773f368e36ce501d), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 12)" )
M4MAKMNT_SET( 199?, m4makmnt__l,	m4makmnt,	"mam04y.p1",	0x0000, 0x020000, CRC(42786701) SHA1(6efb0cbf630cd1b87715e692f76e368e3fba0856), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 13)" )
M4MAKMNT_SET( 199?, m4makmnt__m,	m4makmnt,	"mam15g",		0x0000, 0x020000, CRC(d3fd61f9) SHA1(1c738f818ea84f4bfca3c62fd9c34ce5e983b10a), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 14)" )
M4MAKMNT_SET( 199?, m4makmnt__n,	m4makmnt,	"mam15t",		0x0000, 0x020000, CRC(a5975cbe) SHA1(eb6dd70c79c6b051190055d71ec8421080e5ba39), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 15)" )
M4MAKMNT_SET( 199?, m4makmnt__o,	m4makmnt,	"mamad.p1",		0x0000, 0x020000, CRC(82f63f55) SHA1(2cbc514a49e826505580a57f17ee696bdf9bf436), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 16)" )
M4MAKMNT_SET( 199?, m4makmnt__p,	m4makmnt,	"mamb.p1",		0x0000, 0x020000, CRC(233c2e35) SHA1(823fc5736469c7e1d1da72bec8d64aabb277f9ab), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 17)" )
M4MAKMNT_SET( 199?, m4makmnt__q,	m4makmnt,	"mambd.p1",		0x0000, 0x020000, CRC(0fec9190) SHA1(6cea986853efc042c6325f31f790f80ae2993308), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 18)" )
M4MAKMNT_SET( 199?, m4makmnt__r,	m4makmnt,	"mamc.p1",		0x0000, 0x020000, CRC(14769327) SHA1(435406a46a60666bbe6ebc9392b21d5b0404cffd), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 19)" )
M4MAKMNT_SET( 199?, m4makmnt__s,	m4makmnt,	"mamd.p1",		0x0000, 0x020000, CRC(5e272543) SHA1(c974b21ba488568e12d47f51f21d3b94d40255f3), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 20)" )
M4MAKMNT_SET( 199?, m4makmnt__t,	m4makmnt,	"mamdk.p1",		0x0000, 0x020000, CRC(452b39c6) SHA1(8b065391abd01c7b9d6c5beb95cf17a52c8ebe1a), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 21)" )
M4MAKMNT_SET( 199?, m4makmnt__u,	m4makmnt,	"mamdy.p1",		0x0000, 0x020000, CRC(c2c81938) SHA1(de95d1e28af33dc96343cc615a945c5dfe3f04a7), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 22)" )
M4MAKMNT_SET( 199?, m4makmnt__v,	m4makmnt,	"mamk.p1",		0x0000, 0x020000, CRC(69fb8663) SHA1(741b6e7b97d0c9b243e8e318ed169f92f8fbd5e9), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 23)" )
M4MAKMNT_SET( 199?, m4makmnt__w,	m4makmnt,	"mamr.p1",		0x0000, 0x020000, CRC(daf43d52) SHA1(066cf554178d4e4fdff10c1e93567618f711d196), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 24)" )
M4MAKMNT_SET( 199?, m4makmnt__x,	m4makmnt,	"mamy.p1",		0x0000, 0x020000, CRC(ee18a69d) SHA1(d3cf2c5ed4ec00be4d68c895ca3973da1acccb79), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 25)" )
M4MAKMNT_SET( 199?, m4makmnt__y,	m4makmnt,	"mint2010",		0x0000, 0x020000, CRC(e60d10b9) SHA1(3abe7a7f33a73827ed6585d92fe53d4058c87baf), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 26)" )
M4MAKMNT_SET( 199?, m4makmnt__z,	m4makmnt,	"mmg04ad.p1",	0x0000, 0x020000, CRC(9cbf9691) SHA1(a68f2e9e0ec03dc47017c221a3e780e5cc992a15), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 27)" )
M4MAKMNT_SET( 199?, m4makmnt__0,	m4makmnt,	"mmg04b.p1",	0x0000, 0x020000, CRC(2507742d) SHA1(3bdd6f43da4d4c923bedcc1eba5cb1e1b92ee473), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 28)" )
M4MAKMNT_SET( 199?, m4makmnt__1,	m4makmnt,	"mmg04bd.p1",	0x0000, 0x020000, CRC(11a53854) SHA1(e52ee92f39645380864e86c694a345c028cb42cf), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 29)" )
M4MAKMNT_SET( 199?, m4makmnt__2,	m4makmnt,	"mmg04c.p1",	0x0000, 0x020000, CRC(124dc93f) SHA1(f58a4d6830ece84467a0b247ba66132d659d1383), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 30)" )
M4MAKMNT_SET( 199?, m4makmnt__3,	m4makmnt,	"mmg04d.p1",	0x0000, 0x020000, CRC(581c7f5b) SHA1(224a9e4f6078d38606c7b08a15df05fbcb31a209), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 31)" )
M4MAKMNT_SET( 199?, m4makmnt__4,	m4makmnt,	"mmg04dk.p1",	0x0000, 0x020000, CRC(5b629002) SHA1(e08f55e157973518837d1045fe714221d5ba812d), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 32)" )
M4MAKMNT_SET( 199?, m4makmnt__5,	m4makmnt,	"mmg04dr.p1",	0x0000, 0x020000, CRC(e86d2b33) SHA1(594c4fdba38f2463467e4906981738dafc15369d), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 33)" )
M4MAKMNT_SET( 199?, m4makmnt__6,	m4makmnt,	"mmg04dy.p1",	0x0000, 0x020000, CRC(dc81b0fc) SHA1(3154899d80375413936d2c08edaaf2e97d490c5b), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 34)" )
M4MAKMNT_SET( 199?, m4makmnt__7,	m4makmnt,	"mmg04k.p1",	0x0000, 0x020000, CRC(6fc0dc7b) SHA1(4ad8ef1e666e4796d3a09ba4bda9e48c60266380), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 35)" )
M4MAKMNT_SET( 199?, m4makmnt__8,	m4makmnt,	"mmg04r.p1",	0x0000, 0x020000, CRC(dccf674a) SHA1(a8b2ebeec8587e0655349baca700afc552c3e62d), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 36)" )
M4MAKMNT_SET( 199?, m4makmnt__9,	m4makmnt,	"mmg04s.p1",	0x0000, 0x020000, CRC(1c46683e) SHA1(d08589bb32056ea599e6ffbbb795f46f8eff0782), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 37)" )
M4MAKMNT_SET( 199?, m4makmnt__aa,	m4makmnt,	"mmg04y.p1",	0x0000, 0x020000, CRC(e823fc85) SHA1(10414bbac8ac3bdbf11bc4092370c499fb9db650), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 38)" )
M4MAKMNT_SET( 199?, m4makmnt__ab,	m4makmnt,	"mmg05ad.p1",	0x0000, 0x020000, CRC(d1e70066) SHA1(9635da90808628ae84c6073fef9622a8f37bd069), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 39)" )
M4MAKMNT_SET( 199?, m4makmnt__ac,	m4makmnt,	"mmg05b.p1",	0x0000, 0x020000, CRC(cc0335ff) SHA1(9ca52a49cc48cbfa7c394e0c22cc5075ad1096a1), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 40)" )
M4MAKMNT_SET( 199?, m4makmnt__ad,	m4makmnt,	"mmg05bd.p1",	0x0000, 0x020000, CRC(5cfdaea3) SHA1(f9c5c4b4021ace3e17818c4d4462fe9c87a64d70), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 41)" )
M4MAKMNT_SET( 199?, m4makmnt__ae,	m4makmnt,	"mmg05c.p1",	0x0000, 0x020000, CRC(fb4988ed) SHA1(a6ec8dac9e9a5eda67314286c27a8ce177663030), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 42)" )
M4MAKMNT_SET( 199?, m4makmnt__af,	m4makmnt,	"mmg05d.p1",	0x0000, 0x020000, CRC(b1183e89) SHA1(0f93a811e29ecf40de4338660c30b75f1d565c63), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 43)" )
M4MAKMNT_SET( 199?, m4makmnt__ag,	m4makmnt,	"mmg05dk.p1",	0x0000, 0x020000, CRC(163a06f5) SHA1(cd5fa8d7a2edfdadcfc2d96158bbdfdc74c76068), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 44)" )
M4MAKMNT_SET( 199?, m4makmnt__ah,	m4makmnt,	"mmg05dr.p1",	0x0000, 0x020000, CRC(a535bdc4) SHA1(0d2314f5cb72949c57e759b8f29955c762d9d2fc), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 45)" )
M4MAKMNT_SET( 199?, m4makmnt__ai,	m4makmnt,	"mmg05dy.p1",	0x0000, 0x020000, CRC(91d9260b) SHA1(4442dea0682b737c0053ec4f1109114cdeb3d422), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 46)" )
M4MAKMNT_SET( 199?, m4makmnt__aj,	m4makmnt,	"mmg05k.p1",	0x0000, 0x020000, CRC(86c49da9) SHA1(bf29075a87574009f9ad8fd36e2d3a84c50e6b26), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 47)" )
M4MAKMNT_SET( 199?, m4makmnt__ak,	m4makmnt,	"mmg05r.p1",	0x0000, 0x020000, CRC(35cb2698) SHA1(6371313a179559240ddb55976546ecc8d511e104), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 48)" )
M4MAKMNT_SET( 199?, m4makmnt__al,	m4makmnt,	"mmg05s.p1",	0x0000, 0x020000, CRC(771c17c8) SHA1(d9e595ae020c48769fcbf3de718b6986b6fd8bc5), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 49)" )
M4MAKMNT_SET( 199?, m4makmnt__am,	m4makmnt,	"mmg05y.p1",	0x0000, 0x020000, CRC(0127bd57) SHA1(3d1b59fda52f09fd8af59177b3f5c614b453ac25), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 50)" )
M4MAKMNT_SET( 199?, m4makmnt__an,	m4makmnt,	"ma_x6__5.3_1", 0x0000, 0x010000, CRC(2fe3c309) SHA1(5dba65b29ea5492a78866863629d89f9a8588959), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 51)" )
M4MAKMNT_SET( 199?, m4makmnt__ao,	m4makmnt,	"ma_x6__c.3_1", 0x0000, 0x010000, CRC(e9259a4d) SHA1(9a8e9590403f507f83197a898af5d543bda81b2b), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 52)" )
M4MAKMNT_SET( 199?, m4makmnt__ap,	m4makmnt,	"ma_x6_d5.3_1", 0x0000, 0x010000, CRC(a93dba0d) SHA1(6fabe994ac6c9ea4ce2bae99df699fa100098926), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 53)" )
M4MAKMNT_SET( 199?, m4makmnt__aq,	m4makmnt,	"ma_x6_dc.3_1", 0x0000, 0x010000, CRC(805d75c2) SHA1(b2433556b72f89887c1e404c80d85c940535e8af), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 54)" )
M4MAKMNT_SET( 199?, m4makmnt__ar,	m4makmnt,	"ma_x6a_5.3_1", 0x0000, 0x010000, CRC(79f673de) SHA1(805ea08f5ed016d25ec23dbc3952aad4873a1cde), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 55)" )
M4MAKMNT_SET( 199?, m4makmnt__as,	m4makmnt,	"ma_x6a_c.3_1", 0x0000, 0x010000, CRC(43ede82a) SHA1(d6ec3dd170c56e90018568480bca72cd8390aa2d), "Barcrest","Make A Mint (Barcrest) (MPU4) (set 56)" )


#define M4VIVAES_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "viva.chr", 0x0000, 0x000048, CRC(4662e1fb) SHA1(54074bcc67adedb3dc6df80bdc60e0222f934156) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "vivasnd1.bin", 0x000000, 0x080000, CRC(e7975c75) SHA1(407c3bcff29f4b6599de2c35d96f62c72a897bd1) ) \
	ROM_LOAD( "vivasnd2.bin", 0x080000, 0x080000, CRC(9f22f32d) SHA1(af64f6bde0b825d474c42c56f6e2253b28d4f90f) ) \


#define M4VIVAES_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4VIVAES_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4VIVAES_SET( 199?, m4vivaes,		0,			"5p5vivaespana6-0.bin",	0x0000, 0x010000, CRC(adf02a7b) SHA1(2c61e175b920a67098503eb4d80b07b828c9f91d), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 1)" )
M4VIVAES_SET( 199?, m4vivaes__a,	m4vivaes,	"ep8ad.p1",				0x0000, 0x010000, CRC(1591cc9b) SHA1(b7574b71955d7780f3f127670e458befad951383), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 2)" )
M4VIVAES_SET( 199?, m4vivaes__b,	m4vivaes,	"ep8b.p1",				0x0000, 0x010000, CRC(33b085b3) SHA1(5fc22ee8ae2d597392c82b09a830893bb04e1014), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 3)" )
M4VIVAES_SET( 199?, m4vivaes__c,	m4vivaes,	"ep8bd.p1",				0x0000, 0x010000, CRC(d1eedaac) SHA1(9773fbb9b15dbbe313d76b0746698fbc12e26dd2), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 4)" )
M4VIVAES_SET( 199?, m4vivaes__d,	m4vivaes,	"ep8c.p1",				0x0000, 0x010000, CRC(d2a8aaf5) SHA1(7aabe3e0522877700453068c30c74cbe2c058e9a), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 5)" )
M4VIVAES_SET( 199?, m4vivaes__e,	m4vivaes,	"ep8d.p1",				0x0000, 0x010000, CRC(06f87010) SHA1(636707d4077bee0ea2f221904fa0e187ea4a1e31), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 6)" )
M4VIVAES_SET( 199?, m4vivaes__f,	m4vivaes,	"ep8dk.p1",				0x0000, 0x010000, CRC(e87b56da) SHA1(f3de0ab0badc9bd14505822c63f110b9b2521d55), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 7)" )
M4VIVAES_SET( 199?, m4vivaes__g,	m4vivaes,	"ep8dy.p1",				0x0000, 0x010000, CRC(d20ec7ed) SHA1(dffd4fcaf360b2b9f4b7241fe80bb6ee983b6d57), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 8)" )
M4VIVAES_SET( 199?, m4vivaes__h,	m4vivaes,	"ep8k.p1",				0x0000, 0x010000, CRC(0a2509c5) SHA1(d0fd30953cbc36363a6d4941b4a0805f9663aebb), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 9)" )
M4VIVAES_SET( 199?, m4vivaes__i,	m4vivaes,	"ep8s.p1",				0x0000, 0x010000, CRC(51537f2d) SHA1(a837a525cd7da724f338c47e716be175c37070b0), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 10)" )
M4VIVAES_SET( 199?, m4vivaes__j,	m4vivaes,	"ep8y.p1",				0x0000, 0x010000, CRC(4cc454e4) SHA1(a08ec2a4a17600eba86300dcb6b150b1b5a7fc74), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 11)" )
M4VIVAES_SET( 199?, m4vivaes__k,	m4vivaes,	"espc.p1",				0x0000, 0x010000, CRC(9534d0d0) SHA1(8e4a1081821d472eb4d9aa01e38b6956a1388d28), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 12)" )
M4VIVAES_SET( 199?, m4vivaes__l,	m4vivaes,	"espd.p1",				0x0000, 0x010000, CRC(012fbc14) SHA1(5e4a1cd7989f804ac52c7cbf46d7f9c1d7200336), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 13)" )
M4VIVAES_SET( 199?, m4vivaes__m,	m4vivaes,	"espdy.p1",				0x0000, 0x010000, CRC(90efbb8e) SHA1(a7338c5d71719b86f524f35d7edd176f41383f15), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 14)" )
M4VIVAES_SET( 199?, m4vivaes__n,	m4vivaes,	"espk.p1",				0x0000, 0x010000, CRC(775a56d6) SHA1(b0e47b56315948a7162ae00c3f5197fbb7b81ec5), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 15)" )
M4VIVAES_SET( 199?, m4vivaes__o,	m4vivaes,	"esps.p1",				0x0000, 0x010000, CRC(0c83b014) SHA1(e7cc513b66534b4fec89170d7b739c99a1ba3831), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 16)" )
M4VIVAES_SET( 199?, m4vivaes__p,	m4vivaes,	"espy.p1",				0x0000, 0x010000, CRC(020aa8bb) SHA1(497dae13fe9f9eba624db907e9f4a5bef1584a64), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 17)" )
M4VIVAES_SET( 199?, m4vivaes__q,	m4vivaes,	"ve5ad.p1",				0x0000, 0x010000, CRC(c545d5f0) SHA1(6ad168d2c1f2da2fff85fe0e21a3191cba8f5838), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 18)" )
M4VIVAES_SET( 199?, m4vivaes__r,	m4vivaes,	"ve5b.p1",				0x0000, 0x010000, CRC(ed02fa94) SHA1(9980b2f78ea8f40715e77fd8fafe883739ac1165), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 19)" )
M4VIVAES_SET( 199?, m4vivaes__s,	m4vivaes,	"ve5bd.p1",				0x0000, 0x010000, CRC(fce73b5c) SHA1(35e635ade9b4a7a992c568e317190d12576f78c9), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 20)" )
M4VIVAES_SET( 199?, m4vivaes__t,	m4vivaes,	"ve5d.p1",				0x0000, 0x010000, CRC(e739556d) SHA1(0816aa256cf8ac253ff37999595e981e90874d39), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 21)" )
M4VIVAES_SET( 199?, m4vivaes__u,	m4vivaes,	"ve5dk.p1",				0x0000, 0x010000, CRC(64f174d0) SHA1(f51b28607715931a9d4c1c14fc71b4f8bb8e56fb), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 22)" )
M4VIVAES_SET( 199?, m4vivaes__v,	m4vivaes,	"ve5dy.p1",				0x0000, 0x010000, CRC(fe6339c6) SHA1(82f14d80e96b65eeea08f1029ffaebf2e505091e), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 23)" )
M4VIVAES_SET( 199?, m4vivaes__w,	m4vivaes,	"ve5k.p1",				0x0000, 0x010000, CRC(05428018) SHA1(b6884a1bfd2cf8268258d3d9a8d2c482ba92e5af), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 24)" )
M4VIVAES_SET( 199?, m4vivaes__x,	m4vivaes,	"ve5s.p1",				0x0000, 0x010000, CRC(65df6cf1) SHA1(26eadbad30b93df6dfd37f984be2dec77f1d6442), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 25)" )
M4VIVAES_SET( 199?, m4vivaes__y,	m4vivaes,	"ve5y.p1",				0x0000, 0x010000, CRC(2fe06579) SHA1(9e11b371edd8fab78e9594ed864f8eb487112150), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 26)" )
M4VIVAES_SET( 199?, m4vivaes__z,	m4vivaes,	"vesp05_11",			0x0000, 0x010000, CRC(32100a2e) SHA1(bb7324267708a0c0850fb77885df9868954d86cd), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 27)" )
M4VIVAES_SET( 199?, m4vivaes__0,	m4vivaes,	"vesp10_11",			0x0000, 0x010000, CRC(2a1dfcb2) SHA1(7d4ef072c41779554a2b8046688957585821e356), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 28)" )
M4VIVAES_SET( 199?, m4vivaes__1,	m4vivaes,	"vesp20_11",			0x0000, 0x010000, CRC(06233420) SHA1(06101dbe871617ae6ff098e070316ec98a15b704), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 29)" )
M4VIVAES_SET( 199?, m4vivaes__2,	m4vivaes,	"vetad.p1",				0x0000, 0x010000, CRC(fb9564dc) SHA1(9782d04eaec7d9c19138abf4f2dd3daa6c745c2a), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 30)" )
M4VIVAES_SET( 199?, m4vivaes__3,	m4vivaes,	"vetb.p1",				0x0000, 0x010000, CRC(2a8d7beb) SHA1(e503bdc388c2ab7551cc84dd9e45b85bd2420ef8), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 31)" )
M4VIVAES_SET( 199?, m4vivaes__4,	m4vivaes,	"vetbd.p1",				0x0000, 0x010000, CRC(ebaffb7d) SHA1(b54a581927fc28ce14ab9efe6fe62e074831a42a), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 32)" )
M4VIVAES_SET( 199?, m4vivaes__5,	m4vivaes,	"vetd.p1",				0x0000, 0x010000, CRC(365dff45) SHA1(6ce756f1d6133e05c46e8e7b7ad554f9f512b722), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 33)" )
M4VIVAES_SET( 199?, m4vivaes__6,	m4vivaes,	"vetdk.p1",				0x0000, 0x010000, CRC(5fb1ba90) SHA1(57a7f225d7bd8ed78c2ebf5d363e06b7694efc5f), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 34)" )
M4VIVAES_SET( 199?, m4vivaes__7,	m4vivaes,	"vetdy.p1",				0x0000, 0x010000, CRC(100261cb) SHA1(f834c5b848059673b9e9824854e6600dae6c4499), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 35)" )
M4VIVAES_SET( 199?, m4vivaes__8,	m4vivaes,	"vetk.p1",				0x0000, 0x010000, CRC(db48f34b) SHA1(013d84b27c4ea6d7b538011c22a3cd573f1d12cc), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 36)" )
M4VIVAES_SET( 199?, m4vivaes__9,	m4vivaes,	"vets.p1",				0x0000, 0x010000, CRC(d7e00f9d) SHA1(df2d85ff9eae7adf662b7d8a9c6f874ec8c07183), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 37)" )
M4VIVAES_SET( 199?, m4vivaes__aa,	m4vivaes,	"vety.p1",				0x0000, 0x010000, CRC(ba3b19c7) SHA1(6e9ee238ec6a272ef16ebfba0dc49bc076e741de), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 38)" )
M4VIVAES_SET( 199?, m4vivaes__ab,	m4vivaes,	"viva206",				0x0000, 0x010000, CRC(76ab9a5d) SHA1(455699cbc05f744eafe58881a8fb120b24cfe5c8), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 39)" )
M4VIVAES_SET( 199?, m4vivaes__ac,	m4vivaes,	"ve_05a__.3_1",			0x0000, 0x010000, CRC(92e0e121) SHA1(f32c8f1c8008794283bd32f9440e0a580f77b5b3), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 40)" )
M4VIVAES_SET( 199?, m4vivaes__ad,	m4vivaes,	"ve_10a__.5_1",			0x0000, 0x010000, CRC(afdc0a2f) SHA1(ab8fec2c48db07c0aba31930893fe7211b306468), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 41)" )
M4VIVAES_SET( 199?, m4vivaes__ae,	m4vivaes,	"vei05___.4_1",			0x0000, 0x010000, CRC(687a511b) SHA1(362e1d5557b6b7d551c9b9c5ef70d7944b44a3ce), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 42)" )
M4VIVAES_SET( 199?, m4vivaes__af,	m4vivaes,	"vei10___.4_1",			0x0000, 0x010000, CRC(b9e2471f) SHA1(3fa561466332ed14e233d97bf9170ec08a019bd0), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 43)" )
M4VIVAES_SET( 199?, m4vivaes__ag,	m4vivaes,	"vesp5.8c",				0x0000, 0x010000, CRC(266d42cf) SHA1(b1e583652d6184db2a5f03cb7ae3f694627591c8), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 44)" )
M4VIVAES_SET( 199?, m4vivaes__ah,	m4vivaes,	"vesp5.8t",				0x0000, 0x010000, CRC(bf8c9dfa) SHA1(69f28d3ce04efdb89db688dbc2341d19c27c5ba8), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 45)" )
M4VIVAES_SET( 199?, m4vivaes__ai,	m4vivaes,	"vesp510l",				0x0000, 0x010000, CRC(15c33530) SHA1(888625c383e52825c06cbf1e7022cd8b02bf549c), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 46)" )
M4VIVAES_SET( 199?, m4vivaes__aj,	m4vivaes,	"vesp55",				0x0000, 0x010000, CRC(9cc395ef) SHA1(d62cb55664246e3fada3d971ee317eef51739018), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 47)" )
M4VIVAES_SET( 199?, m4vivaes__ak,	m4vivaes,	"vesp58c",				0x0000, 0x010000, CRC(d8cc868d) SHA1(0b9fa8b61998badbd870827e32af4937548b583e), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 48)" )
M4VIVAES_SET( 199?, m4vivaes__al,	m4vivaes,	"vesp_10.4",			0x0000, 0x010000, CRC(95e95339) SHA1(59633b7c01da25237342bce7e989259bf723ba6f), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 49)" )
M4VIVAES_SET( 199?, m4vivaes__am,	m4vivaes,	"vesp_10.8",			0x0000, 0x010000, CRC(8054766d) SHA1(8e7fd6f8cd74d2760e2923af32813ca93fbf98e6), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 50)" )
M4VIVAES_SET( 199?, m4vivaes__an,	m4vivaes,	"vesp_20_.8",			0x0000, 0x010000, CRC(35f90f05) SHA1(0013ff32c809603efdad782306140bd7086be965), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 51)" )
M4VIVAES_SET( 199?, m4vivaes__ao,	m4vivaes,	"vesp_5.4",				0x0000, 0x010000, CRC(3b6762ce) SHA1(9dc53dce453a7b124ea2b65a590aff6c7d05831f), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 52)" )
M4VIVAES_SET( 199?, m4vivaes__ap,	m4vivaes,	"vesp_5.8",				0x0000, 0x010000, CRC(63abf642) SHA1(6b585147a771e4bd445b525aafc25293845f660b), "Barcrest","Viva Espana (Barcrest) (MPU4) (set 53)" )


#define M4POTBLK_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "pbsnd1.hex", 0x000000, 0x080000, CRC(72a3331d) SHA1(b7475ba0ad86a6277e3d4f7b4311a98f3fc29802) ) \
	ROM_LOAD( "pbsnd2.hex", 0x080000, 0x080000, CRC(c2460eec) SHA1(7c62fbc69ffaa788bf3839e37a75a812a7b8caef) ) \


#define M4POTBLK_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4POTBLK_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4POTBLK_SET( 199?, m4potblk,		0,			"pbg16s.p1",	0x0000, 0x020000, CRC(36a1c679) SHA1(bf2eb5c2a07e61b7a2c0d8402b0e0583adfa22dc), "Barcrest","Pot Black (Barcrest) (MPU4) (set 1)" )
M4POTBLK_SET( 199?, m4potblk__a,	m4potblk,	"pb15g",		0x0000, 0x020000, CRC(650a54be) SHA1(80a5bb95857c911c1972f8be5bf794637cb02323), "Barcrest","Pot Black (Barcrest) (MPU4) (set 2)" )
M4POTBLK_SET( 199?, m4potblk__b,	m4potblk,	"pb15t",		0x0000, 0x020000, CRC(98628744) SHA1(1a0df7036c36f3b87d5a239e1c9edfd7c74d2ae8), "Barcrest","Pot Black (Barcrest) (MPU4) (set 3)" )
M4POTBLK_SET( 199?, m4potblk__c,	m4potblk,	"pbg14s.p1",	0x0000, 0x020000, CRC(c9316c92) SHA1(d9248069c4702d4ce780ab82bdb783ba5aea034b), "Barcrest","Pot Black (Barcrest) (MPU4) (set 4)" )
M4POTBLK_SET( 199?, m4potblk__d,	m4potblk,	"pbg15ad.p1",	0x0000, 0x020000, CRC(ded4ba89) SHA1(f8b4727987bef1e74894df4e7549d3c28ba4de98), "Barcrest","Pot Black (Barcrest) (MPU4) (set 5)" )
M4POTBLK_SET( 199?, m4potblk__e,	m4potblk,	"pbg15b.p1",	0x0000, 0x020000, CRC(5a6570be) SHA1(f44e4511cc0c0f410104f9a36ae51b3972bd4522), "Barcrest","Pot Black (Barcrest) (MPU4) (set 6)" )
M4POTBLK_SET( 199?, m4potblk__f,	m4potblk,	"pbg15bd.p1",	0x0000, 0x020000, CRC(941312df) SHA1(51ec4052cfaa245873146d0ecb8834be5cc22db2), "Barcrest","Pot Black (Barcrest) (MPU4) (set 7)" )
M4POTBLK_SET( 199?, m4potblk__g,	m4potblk,	"pbg15d.p1",	0x0000, 0x020000, CRC(79f682f5) SHA1(ee6f31009b8a5354db930d6f228a2969dbebb9ad), "Barcrest","Pot Black (Barcrest) (MPU4) (set 8)" )
M4POTBLK_SET( 199?, m4potblk__h,	m4potblk,	"pbg15dh.p1",	0x0000, 0x020000, CRC(e90819a9) SHA1(f3d423e56205f6b18892fe8771aa853f7185336a), "Barcrest","Pot Black (Barcrest) (MPU4) (set 9)" )
M4POTBLK_SET( 199?, m4potblk__i,	m4potblk,	"pbg15dk.p1",	0x0000, 0x020000, CRC(6ddb01b8) SHA1(aeefd2145f328f0f7af87b16f9bc2324d134b7e1), "Barcrest","Pot Black (Barcrest) (MPU4) (set 10)" )
M4POTBLK_SET( 199?, m4potblk__j,	m4potblk,	"pbg15dr.p1",	0x0000, 0x020000, CRC(f735df01) SHA1(f669cd719cdf9fa170babc652be164bd7c580344), "Barcrest","Pot Black (Barcrest) (MPU4) (set 11)" )
M4POTBLK_SET( 199?, m4potblk__k,	m4potblk,	"pbg15dy.p1",	0x0000, 0x020000, CRC(59379a77) SHA1(91b56aef53de7c554924ebab56faa3e8655dcbfd), "Barcrest","Pot Black (Barcrest) (MPU4) (set 12)" )
M4POTBLK_SET( 199?, m4potblk__l,	m4potblk,	"pbg15h.p1",	0x0000, 0x020000, CRC(277e7bc8) SHA1(9f89a048fcf268883002bb0dcb14854949ebec46), "Barcrest","Pot Black (Barcrest) (MPU4) (set 13)" )
M4POTBLK_SET( 199?, m4potblk__m,	m4potblk,	"pbg15k.p1",	0x0000, 0x020000, CRC(a3ad63d9) SHA1(0e65ff6ae02bd42cb1b3c9249ac85dc13b4eb8ad), "Barcrest","Pot Black (Barcrest) (MPU4) (set 14)" )
M4POTBLK_SET( 199?, m4potblk__n,	m4potblk,	"pbg15r.p1",	0x0000, 0x020000, CRC(3943bd60) SHA1(cfcd1e2b76f592e3bc5b8ed33af66ad183e829e1), "Barcrest","Pot Black (Barcrest) (MPU4) (set 15)" )
M4POTBLK_SET( 199?, m4potblk__o,	m4potblk,	"pbg15s.p1",	0x0000, 0x020000, CRC(f31c9a6a) SHA1(f2c7dceaabbe0689227f2c59d063ac20403eae1d), "Barcrest","Pot Black (Barcrest) (MPU4) (set 16)" )
M4POTBLK_SET( 199?, m4potblk__p,	m4potblk,	"pbg15y.p1",	0x0000, 0x020000, CRC(9741f816) SHA1(52482fed48574582ed48424666284695fe661880), "Barcrest","Pot Black (Barcrest) (MPU4) (set 17)" )
M4POTBLK_SET( 199?, m4potblk__q,	m4potblk,	"pbg16ad.p1",	0x0000, 0x020000, CRC(919e90ba) SHA1(b32459b394595a5c3d238c6eec47c7d4d34fcdf8), "Barcrest","Pot Black (Barcrest) (MPU4) (set 18)" )
M4POTBLK_SET( 199?, m4potblk__r,	m4potblk,	"pbg16b.p1",	0x0000, 0x020000, CRC(db445653) SHA1(145af560641f9becb6d98c2f157f94b0fdd6459c), "Barcrest","Pot Black (Barcrest) (MPU4) (set 19)" )
M4POTBLK_SET( 199?, m4potblk__s,	m4potblk,	"pbg16bd.p1",	0x0000, 0x020000, CRC(db5938ec) SHA1(323f190c62f23b8092274dd17f07198f53abc828), "Barcrest","Pot Black (Barcrest) (MPU4) (set 20)" )
M4POTBLK_SET( 199?, m4potblk__t,	m4potblk,	"pbg16d.p1",	0x0000, 0x020000, CRC(f8d7a418) SHA1(81a2d020ec03574b041b9be0b8ed96386804f4af), "Barcrest","Pot Black (Barcrest) (MPU4) (set 21)" )
M4POTBLK_SET( 199?, m4potblk__u,	m4potblk,	"pbg16dh.p1",	0x0000, 0x020000, CRC(a642339a) SHA1(104b405923b71d69bc996dced4dc284d41397c5f), "Barcrest","Pot Black (Barcrest) (MPU4) (set 22)" )
M4POTBLK_SET( 199?, m4potblk__v,	m4potblk,	"pbg16dk.p1",	0x0000, 0x020000, CRC(22912b8b) SHA1(cdbaaf0509fb6115182c8ac79002e1d983bcc765), "Barcrest","Pot Black (Barcrest) (MPU4) (set 23)" )
M4POTBLK_SET( 199?, m4potblk__w,	m4potblk,	"pbg16dr.p1",	0x0000, 0x020000, CRC(b87ff532) SHA1(d64d4733523d4afe5b4e3ca5f2c33ee8d4ab1c2b), "Barcrest","Pot Black (Barcrest) (MPU4) (set 24)" )
M4POTBLK_SET( 199?, m4potblk__x,	m4potblk,	"pbg16dy.p1",	0x0000, 0x020000, CRC(167db044) SHA1(291982c6af2486724be32b8f41ee66d699afad22), "Barcrest","Pot Black (Barcrest) (MPU4) (set 25)" )
M4POTBLK_SET( 199?, m4potblk__y,	m4potblk,	"pbg16h.p1",	0x0000, 0x020000, CRC(a65f5d25) SHA1(6ea4bc92ecf849653c1abbb5c1e1d97d0e58a373), "Barcrest","Pot Black (Barcrest) (MPU4) (set 26)" )
M4POTBLK_SET( 199?, m4potblk__z,	m4potblk,	"pbg16k.p1",	0x0000, 0x020000, CRC(228c4534) SHA1(e4a4e7ec059e4da568c507d8f1f006c04e1c13c4), "Barcrest","Pot Black (Barcrest) (MPU4) (set 27)" )
M4POTBLK_SET( 199?, m4potblk__0,	m4potblk,	"pbg16r.p1",	0x0000, 0x020000, CRC(b8629b8d) SHA1(08fc4498b45f2e939c4c465d4d979aa4532b5ce4), "Barcrest","Pot Black (Barcrest) (MPU4) (set 28)" )
M4POTBLK_SET( 199?, m4potblk__1,	m4potblk,	"pbg16y.p1",	0x0000, 0x020000, CRC(1660defb) SHA1(9f6112759b029e71056a92137f890910b6afb708), "Barcrest","Pot Black (Barcrest) (MPU4) (set 29)" )
M4POTBLK_SET( 199?, m4potblk__2,	m4potblk,	"pbs04ad.p1",	0x0000, 0x020000, CRC(c4fedb7d) SHA1(1ce1e524bd11e775c0f8498849c1d9ca41fcf912), "Barcrest","Pot Black (Barcrest) (MPU4) (set 30)" )
M4POTBLK_SET( 199?, m4potblk__3,	m4potblk,	"pbs04b.p1",	0x0000, 0x020000, CRC(f1fcaf2d) SHA1(786513cbfdde2f37c6ab3649781bc7c70bbdfb61), "Barcrest","Pot Black (Barcrest) (MPU4) (set 31)" )
M4POTBLK_SET( 199?, m4potblk__4,	m4potblk,	"pbs04bd.p1",	0x0000, 0x020000, CRC(49e475b8) SHA1(95548a22c67bd8df6df05503b6318b15bd84bb7a), "Barcrest","Pot Black (Barcrest) (MPU4) (set 32)" )
M4POTBLK_SET( 199?, m4potblk__5,	m4potblk,	"pbs04c.p1",	0x0000, 0x020000, CRC(c6b6123f) SHA1(ccd286dc09606d6cbf45835701dc83ca361c8dc0), "Barcrest","Pot Black (Barcrest) (MPU4) (set 33)" )
M4POTBLK_SET( 199?, m4potblk__6,	m4potblk,	"pbs04dh.p1",	0x0000, 0x020000, CRC(34ff7ece) SHA1(a659aee7b093c00e920428f78e96e4246ad469ba), "Barcrest","Pot Black (Barcrest) (MPU4) (set 34)" )
M4POTBLK_SET( 199?, m4potblk__7,	m4potblk,	"pbs04dk.p1",	0x0000, 0x020000, CRC(0323ddee) SHA1(74ac57542d6bf32a105682daf661546d18e1eab4), "Barcrest","Pot Black (Barcrest) (MPU4) (set 35)" )
M4POTBLK_SET( 199?, m4potblk__8,	m4potblk,	"pbs04dr.p1",	0x0000, 0x020000, CRC(b02c66df) SHA1(7f2b24d747349a7a17b5f90dbada2a6c9f620b1d), "Barcrest","Pot Black (Barcrest) (MPU4) (set 36)" )
M4POTBLK_SET( 199?, m4potblk__9,	m4potblk,	"pbs04dy.p1",	0x0000, 0x020000, CRC(84c0fd10) SHA1(586a41908165211b6d6386f12170aeda1ff3fbe9), "Barcrest","Pot Black (Barcrest) (MPU4) (set 37)" )
M4POTBLK_SET( 199?, m4potblk__aa,	m4potblk,	"pbs04h.p1",	0x0000, 0x020000, CRC(8ce7a45b) SHA1(1eab7d504b4a8f6158aa5878dcc62c9531169a85), "Barcrest","Pot Black (Barcrest) (MPU4) (set 38)" )
M4POTBLK_SET( 199?, m4potblk__ab,	m4potblk,	"pbs04k.p1",	0x0000, 0x020000, CRC(bb3b077b) SHA1(584fd9f1578c61e1a1c30068c42b16716c5d490f), "Barcrest","Pot Black (Barcrest) (MPU4) (set 39)" )
M4POTBLK_SET( 199?, m4potblk__ac,	m4potblk,	"pbs04r.p1",	0x0000, 0x020000, CRC(0834bc4a) SHA1(0064b1ec9db506c4dd14ed7ffeffa08bebc117b1), "Barcrest","Pot Black (Barcrest) (MPU4) (set 40)" )
M4POTBLK_SET( 199?, m4potblk__ad,	m4potblk,	"pbs04s.p1",	0x0000, 0x020000, CRC(b4a7eaac) SHA1(295b793802a6145758861142133ced98f2258119), "Barcrest","Pot Black (Barcrest) (MPU4) (set 41)" )
M4POTBLK_SET( 199?, m4potblk__ae,	m4potblk,	"pbs04y.p1",	0x0000, 0x020000, CRC(3cd82785) SHA1(fb2cb5acfc60d0896da9c22b7a9370e7c0271cf7), "Barcrest","Pot Black (Barcrest) (MPU4) (set 42)" )
M4POTBLK_SET( 199?, m4potblk__af,	m4potblk,	"pbs06ad.p1",	0x0000, 0x020000, CRC(6344d6c7) SHA1(7c01149d9f21a15b1067a42d3f8def2868f15181), "Barcrest","Pot Black (Barcrest) (MPU4) (set 43)" )
M4POTBLK_SET( 199?, m4potblk__ag,	m4potblk,	"pbs06b.p1",	0x0000, 0x020000, CRC(2056d268) SHA1(ac978d59ff3cead2678d56579e404eb7494ab957), "Barcrest","Pot Black (Barcrest) (MPU4) (set 44)" )
M4POTBLK_SET( 199?, m4potblk__ah,	m4potblk,	"pbs06bd.p1",	0x0000, 0x020000, CRC(ee5e7802) SHA1(568b6b2e6d58ee766a74badb60118dc0899b8b68), "Barcrest","Pot Black (Barcrest) (MPU4) (set 45)" )
M4POTBLK_SET( 199?, m4potblk__ai,	m4potblk,	"pbs06c.p1",	0x0000, 0x020000, CRC(171c6f7a) SHA1(e0c7455b64105cdd41ab24ef4cec7b044732faf6), "Barcrest","Pot Black (Barcrest) (MPU4) (set 46)" )
M4POTBLK_SET( 199?, m4potblk__aj,	m4potblk,	"pbs06d.p1",	0x0000, 0x020000, CRC(03c52023) SHA1(534fbee8e19217002c428cc2d9a6693b8bccf974), "Barcrest","Pot Black (Barcrest) (MPU4) (set 47)" )
M4POTBLK_SET( 199?, m4potblk__ak,	m4potblk,	"pbs06dh.p1",	0x0000, 0x020000, CRC(93457374) SHA1(40f4ed7260f234b69084676448705b53ff4700e4), "Barcrest","Pot Black (Barcrest) (MPU4) (set 48)" )
M4POTBLK_SET( 199?, m4potblk__al,	m4potblk,	"pbs06dk.p1",	0x0000, 0x020000, CRC(a499d054) SHA1(6c740d6e765c5ac3690814f71cb340a67f0bb113), "Barcrest","Pot Black (Barcrest) (MPU4) (set 49)" )
M4POTBLK_SET( 199?, m4potblk__am,	m4potblk,	"pbs06dr.p1",	0x0000, 0x020000, CRC(17966b65) SHA1(2a785268954388dba259df162298316e0d187ceb), "Barcrest","Pot Black (Barcrest) (MPU4) (set 50)" )
M4POTBLK_SET( 199?, m4potblk__an,	m4potblk,	"pbs06dy.p1",	0x0000, 0x020000, CRC(237af0aa) SHA1(c6f1cf33506517eac98d449c54be33d1f220241c), "Barcrest","Pot Black (Barcrest) (MPU4) (set 51)" )
M4POTBLK_SET( 199?, m4potblk__ao,	m4potblk,	"pbs06h.p1",	0x0000, 0x020000, CRC(5d4dd91e) SHA1(a715dde45ce7f3c6e62cb08eb5eaacb918803280), "Barcrest","Pot Black (Barcrest) (MPU4) (set 52)" )
M4POTBLK_SET( 199?, m4potblk__ap,	m4potblk,	"pbs06k.p1",	0x0000, 0x020000, CRC(6a917a3e) SHA1(b8e6fb7ea83c5a363fdd756a5479a51d15cb246d), "Barcrest","Pot Black (Barcrest) (MPU4) (set 53)" )
M4POTBLK_SET( 199?, m4potblk__aq,	m4potblk,	"pbs06r.p1",	0x0000, 0x020000, CRC(d99ec10f) SHA1(62ffc2772495fd165b2ad9f76a54154f51464394), "Barcrest","Pot Black (Barcrest) (MPU4) (set 54)" )
M4POTBLK_SET( 199?, m4potblk__ar,	m4potblk,	"pbs06s.p1",	0x0000, 0x020000, CRC(d2b42b29) SHA1(a077605b1f9f3082a03882b4f5b360a530a97135), "Barcrest","Pot Black (Barcrest) (MPU4) (set 55)" )
M4POTBLK_SET( 199?, m4potblk__as,	m4potblk,	"pbs06y.p1",	0x0000, 0x020000, CRC(ed725ac0) SHA1(4c2c38e1c2ce7e15c409e06b6f21410f04b70348), "Barcrest","Pot Black (Barcrest) (MPU4) (set 56)" )
M4POTBLK_SET( 199?, m4potblk__at,	m4potblk,	"po_x6__5.1_1",	0x0000, 0x020000, CRC(1fe40fd1) SHA1(5e16ff5b1019d83c1f40d63f89c16030dae0ab11), "Barcrest","Pot Black (Barcrest) (MPU4) (set 57)" )
M4POTBLK_SET( 199?, m4potblk__au,	m4potblk,	"po_x6__t.1_1", 0x0000, 0x020000, CRC(c9314f6e) SHA1(4f9226883f9e1963c568eea327775688fb966431), "Barcrest","Pot Black (Barcrest) (MPU4) (set 58)" )
M4POTBLK_SET( 199?, m4potblk__av,	m4potblk,	"po_x6_d5.1_1", 0x0000, 0x020000, CRC(404d5a99) SHA1(7a846df3b7f9f0108d84e4a4c2d199e5971b6375), "Barcrest","Pot Black (Barcrest) (MPU4) (set 59)" )
M4POTBLK_SET( 199?, m4potblk__aw,	m4potblk,	"po_x6_dt.1_1", 0x0000, 0x020000, CRC(7213fd77) SHA1(07482cb54d4f03aad62c54d66322f7101f6c8dcf), "Barcrest","Pot Black (Barcrest) (MPU4) (set 60)" )
M4POTBLK_SET( 199?, m4potblk__ax,	m4potblk,	"po_x6a_t.1_1", 0x0000, 0x020000, CRC(1b47a76a) SHA1(3587e4c0b50e359529e132376af3cd239194db31), "Barcrest","Pot Black (Barcrest) (MPU4) (set 61)" )



#define M4PLACBT_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "pyb.chr", 0x0000, 0x000048, CRC(663e9d8e) SHA1(08e898967d41fbc582c9bfdebb461ad51269089d) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "pybsnd.p1", 0x000000, 0x080000, CRC(3a91784a) SHA1(7297ccec3264aa9f1e7b3a2841f5f8a1e4ca6c54) ) \
	ROM_LOAD( "pybsnd.p2", 0x080000, 0x080000, CRC(a82f0096) SHA1(45b6b5a2ae06b45add9cdbb9f5e6f834687b4902) ) \

#define M4PLACBT_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4PLACBT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \



M4PLACBT_SET( 199?, m4placbt,		0,			"pyb07s.p1",	0x0000, 0x020000, CRC(ad02705a) SHA1(027bcbbd828e4fd23831af9554d582857e6784e1), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 1)" )
M4PLACBT_SET( 199?, m4placbt__a,	m4placbt,	"pyb06ad.p1",	0x0000, 0x020000, CRC(e08b6176) SHA1(ccfb43ee033b4ed36e8656bcb4ba62230dde8466), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 2)" )
M4PLACBT_SET( 199?, m4placbt__b,	m4placbt,	"pyb06b.p1",	0x0000, 0x020000, CRC(b6486055) SHA1(e0926720aba1e9d1327c32db29220d91050ea338), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 3)" )
M4PLACBT_SET( 199?, m4placbt__c,	m4placbt,	"pyb06bd.p1",	0x0000, 0x020000, CRC(6d91cfb3) SHA1(82d6ed6d6b2022d0945ec0fb8012fa4fef7029e0), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 4)" )
M4PLACBT_SET( 199?, m4placbt__d,	m4placbt,	"pyb06c.p1",	0x0000, 0x020000, CRC(8102dd47) SHA1(7b834d896e104b1f42069d6fa0bce75b5c15b899), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 5)" )
M4PLACBT_SET( 199?, m4placbt__e,	m4placbt,	"pyb06d.p1",	0x0000, 0x020000, CRC(cb536b23) SHA1(874eaaa434a0212edefa05a440e7e5f826d1f92e), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 6)" )
M4PLACBT_SET( 199?, m4placbt__f,	m4placbt,	"pyb06dk.p1",	0x0000, 0x020000, CRC(275667e5) SHA1(4ac40eaa03462c1a70f0366f589bc2a59972827b), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 7)" )
M4PLACBT_SET( 199?, m4placbt__g,	m4placbt,	"pyb06dr.p1",	0x0000, 0x020000, CRC(9459dcd4) SHA1(ae8b559205c6cb3f2e3b1131cd99ff7ce037c573), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 8)" )
M4PLACBT_SET( 199?, m4placbt__h,	m4placbt,	"pyb06dy.p1",	0x0000, 0x020000, CRC(a0b5471b) SHA1(95400779bb1d3fdeada5d8fca4fd66a89d3e13a2), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 9)" )
M4PLACBT_SET( 199?, m4placbt__i,	m4placbt,	"pyb06k.p1",	0x0000, 0x020000, CRC(fc8fc803) SHA1(81fa3104075b56f51c35d944fa3652aa8cce988c), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 10)" )
M4PLACBT_SET( 199?, m4placbt__j,	m4placbt,	"pyb06r.p1",	0x0000, 0x020000, CRC(4f807332) SHA1(e3852ac9811d780ac87f375acaf5ec1026071b2e), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 11)" )
M4PLACBT_SET( 199?, m4placbt__k,	m4placbt,	"pyb06s.p1",	0x0000, 0x020000, CRC(acd9d628) SHA1(93d8f0ffa3b9ebdd9fef39b2bc49bb85b2fac00f), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 12)" )
M4PLACBT_SET( 199?, m4placbt__l,	m4placbt,	"pyb06y.p1",	0x0000, 0x020000, CRC(7b6ce8fd) SHA1(096fb2e8a4ac5f723810766bc4245d403814a20f), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 13)" )
M4PLACBT_SET( 199?, m4placbt__m,	m4placbt,	"pyb07ad.p1",	0x0000, 0x020000, CRC(427a7489) SHA1(fb0a24da5ef7a948152e8180968aaaebbd85afa0), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 14)" )
M4PLACBT_SET( 199?, m4placbt__n,	m4placbt,	"pyb07b.p1",	0x0000, 0x020000, CRC(35cdf803) SHA1(94953da72c2ee8792f53bf677483ffed15d4709c), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 15)" )
M4PLACBT_SET( 199?, m4placbt__o,	m4placbt,	"pyb07bd.p1",	0x0000, 0x020000, CRC(cf60da4c) SHA1(8667308f750e944894f68f20b70d42244b751e22), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 16)" )
M4PLACBT_SET( 199?, m4placbt__p,	m4placbt,	"pyb07c.p1",	0x0000, 0x020000, CRC(02874511) SHA1(b5acdcfb7d901faa1271eb16e5d36d0a484d97cb), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 17)" )
M4PLACBT_SET( 199?, m4placbt__q,	m4placbt,	"pyb07d.p1",	0x0000, 0x020000, CRC(48d6f375) SHA1(1891b6f8f4599d94280bcb68e9d0e9259351e2b8), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 18)" )
M4PLACBT_SET( 199?, m4placbt__r,	m4placbt,	"pyb07dk.p1",	0x0000, 0x020000, CRC(85a7721a) SHA1(e3af55577b4ad4ae48e95b576e336cb019f3ecd0), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 19)" )
M4PLACBT_SET( 199?, m4placbt__s,	m4placbt,	"pyb07dr.p1",	0x0000, 0x020000, CRC(36a8c92b) SHA1(a1091dea9ffe53c9ba1495f7e0d2aebe92d9bb64), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 20)" )
M4PLACBT_SET( 199?, m4placbt__t,	m4placbt,	"pyb07dy.p1",	0x0000, 0x020000, CRC(024452e4) SHA1(a92c887ab467be6bcccaec1cd5dcc1304eddba19), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 21)" )
M4PLACBT_SET( 199?, m4placbt__u,	m4placbt,	"pyb07k.p1",	0x0000, 0x020000, CRC(7f0a5055) SHA1(5620d8a3333f2f56ea24bcecf1a791e6ee0f43d9), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 22)" )
M4PLACBT_SET( 199?, m4placbt__v,	m4placbt,	"pyb07r.p1",	0x0000, 0x020000, CRC(cc05eb64) SHA1(1329decc84de231e8c1929f057233b25cc8b5942), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 23)" )
M4PLACBT_SET( 199?, m4placbt__w,	m4placbt,	"pyb07y.p1",	0x0000, 0x020000, CRC(f8e970ab) SHA1(66a54a9c2750ea1aa4ce562aad74c98775865ed6), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 24)" )
M4PLACBT_SET( 199?, m4placbt__x,	m4placbt,	"pyb10h",		0x0000, 0x020000, CRC(69be6185) SHA1(f697350912505cd857acae2733ad8b48e67cab6b), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 25)" )
M4PLACBT_SET( 199?, m4placbt__y,	m4placbt,	"pyb15g",		0x0000, 0x020000, CRC(369fd852) SHA1(4c532a59451352aa54a1e47d12f04403d2e9c8cb), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 26)" )
M4PLACBT_SET( 199?, m4placbt__z,	m4placbt,	"pyb15t",		0x0000, 0x020000, CRC(c38d7b04) SHA1(5785344084498cab4ce2734b3d8c0dc8f0cbed5a), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 27)" )
M4PLACBT_SET( 199?, m4placbt__0,	m4placbt,	"pyh04s",		0x0000, 0x020000, CRC(c824b937) SHA1(9bc0a1e75540520ef3448dc7a3c95c81f93abe78), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 28)" )
M4PLACBT_SET( 199?, m4placbt__1,	m4placbt,	"pyh05ad.p1",	0x0000, 0x020000, CRC(948d1ad6) SHA1(66c580f0ef9035de5f50600db51d63336a8d3fbb), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 29)" )
M4PLACBT_SET( 199?, m4placbt__2,	m4placbt,	"pyh05b.p1",	0x0000, 0x020000, CRC(26c5fca4) SHA1(8166a195eb1aa7df99cc27e7cd6207a9192d14b9), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 30)" )
M4PLACBT_SET( 199?, m4placbt__3,	m4placbt,	"pyh05bd.p1",	0x0000, 0x020000, CRC(1997b413) SHA1(cf701534243b302b83908bdf359050b325bd037a), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 31)" )
M4PLACBT_SET( 199?, m4placbt__4,	m4placbt,	"pyh05c.p1",	0x0000, 0x020000, CRC(118f41b6) SHA1(29b74182d24d8d301a7aa899f4c1dd2b1a4eb84c), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 32)" )
M4PLACBT_SET( 199?, m4placbt__5,	m4placbt,	"pyh05d.p1",	0x0000, 0x020000, CRC(5bdef7d2) SHA1(c00a602ea6f0a53d53b13c2e4921aabd9d10b0f3), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 33)" )
M4PLACBT_SET( 199?, m4placbt__6,	m4placbt,	"pyh05dk.p1",	0x0000, 0x020000, CRC(53501c45) SHA1(fa1161a0fe6916cd84370886688517e9905561d2), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 34)" )
M4PLACBT_SET( 199?, m4placbt__7,	m4placbt,	"pyh05dr.p1",	0x0000, 0x020000, CRC(e05fa774) SHA1(44d53a0480b382f01ab42ca5b0521a612f672433), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 35)" )
M4PLACBT_SET( 199?, m4placbt__8,	m4placbt,	"pyh05dy.p1",	0x0000, 0x020000, CRC(d4b33cbb) SHA1(e9681c025b3b661a26b89ef1fa6bbcffb6c2e233), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 36)" )
M4PLACBT_SET( 199?, m4placbt__9,	m4placbt,	"pyh05k.p1",	0x0000, 0x020000, CRC(6c0254f2) SHA1(df14735ec9bc77fc35f52094598bb3fbb015944f), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 37)" )
M4PLACBT_SET( 199?, m4placbt__aa,	m4placbt,	"pyh05r.p1",	0x0000, 0x020000, CRC(df0defc3) SHA1(d259f6eb770130671d06e221d467df658ba0b29b), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 38)" )
M4PLACBT_SET( 199?, m4placbt__ab,	m4placbt,	"pyh05s.p1",	0x0000, 0x020000, CRC(3c544ad9) SHA1(50780424382fd4ccd023a784e43bb60b8f862456), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 39)" )
M4PLACBT_SET( 199?, m4placbt__ac,	m4placbt,	"pyh05y.p1",	0x0000, 0x020000, CRC(ebe1740c) SHA1(424707d023fd026cf43a687ed02d4ee4398b299f), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 40)" )
M4PLACBT_SET( 199?, m4placbt__ad,	m4placbt,	"pyh06ad.p1",	0x0000, 0x020000, CRC(7ae70380) SHA1(c0e2b67ade2275a903359b6df7f55e44ef78828f), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 41)" )
M4PLACBT_SET( 199?, m4placbt__ae,	m4placbt,	"pyh06b.p1",	0x0000, 0x020000, CRC(dc588857) SHA1(dd2d2ffa87c61b200aa82337beea7d2205f1176c), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 42)" )
M4PLACBT_SET( 199?, m4placbt__af,	m4placbt,	"pyh06bd.p1",	0x0000, 0x020000, CRC(f7fdad45) SHA1(78334007b0a414fd3d2b8ec1645d1f04e711eb77), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 43)" )
M4PLACBT_SET( 199?, m4placbt__ag,	m4placbt,	"pyh06c.p1",	0x0000, 0x020000, CRC(eb123545) SHA1(410400c219fb15f3267c2f94737fa9d2785318a0), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 44)" )
M4PLACBT_SET( 199?, m4placbt__ah,	m4placbt,	"pyh06d.p1",	0x0000, 0x020000, CRC(a1438321) SHA1(53f7e0156b137bea91264fe662642083ca9f5f9c), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 45)" )
M4PLACBT_SET( 199?, m4placbt__ai,	m4placbt,	"pyh06dk.p1",	0x0000, 0x020000, CRC(bd3a0513) SHA1(98dfd874dcbee5a3c16a0ebf42f944bf0ba50672), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 46)" )
M4PLACBT_SET( 199?, m4placbt__aj,	m4placbt,	"pyh06dr.p1",	0x0000, 0x020000, CRC(0e35be22) SHA1(8ff910601bfec1b47e50ba92cdb80c1bdfe287ec), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 47)" )
M4PLACBT_SET( 199?, m4placbt__ak,	m4placbt,	"pyh06dy.p1",	0x0000, 0x020000, CRC(3ad925ed) SHA1(af584333e443127071b874bafea0c8667da33f6c), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 48)" )
M4PLACBT_SET( 199?, m4placbt__al,	m4placbt,	"pyh06k.p1",	0x0000, 0x020000, CRC(969f2001) SHA1(e934aa7e95e91f155ee82e8eaac5c949b35e024b), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 49)" )
M4PLACBT_SET( 199?, m4placbt__am,	m4placbt,	"pyh06r.p1",	0x0000, 0x020000, CRC(25909b30) SHA1(8a44e59a46ffec3badb27ee62e7e9bb0adff62a4), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 50)" )
M4PLACBT_SET( 199?, m4placbt__an,	m4placbt,	"pyh06s.p1",	0x0000, 0x020000, CRC(10b75ddf) SHA1(d093ac51c64642400d2cf24a713dc7adb4a6a9d0), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 51)" )
M4PLACBT_SET( 199?, m4placbt__ao,	m4placbt,	"pyh06y.p1",	0x0000, 0x020000, CRC(117c00ff) SHA1(ace5d8c4f4e0647c89608db2c2ad35f241be3672), "Barcrest","Place Your Bets (Barcrest) (MPU4) (set 52)" )



#define M4C9_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "du91.chr", 0x0000, 0x000048, CRC(9724122d) SHA1(a41687eec84cad453c1a2a89317078f48ca0895f) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "c9s.hex", 0x0000, 0x080000, CRC(ae952e15) SHA1(a9eed61c3d65ded5e1faa67362f181393cb6339a) ) \

#define M4C9_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4C9_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4C9_SET( 199?, m4c9,		0,		"c9211.p1",		0x0000, 0x010000, CRC(44e5cc87) SHA1(36fca9493d36ee6988d02da1b4c575278c43748c), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 1)" )
M4C9_SET( 199?, m4c9__a,	m4c9,	"c915.hex",		0x0000, 0x010000, CRC(dabfa3f3) SHA1(f507c78e61cba74e9b776bebaf0cc4fa40b6de95), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 2)" )
M4C9_SET( 199?, m4c9__b,	m4c9,	"c9210dk.p1",	0x0000, 0x010000, CRC(169a3ce4) SHA1(74d5d533c145908d17bb3e6ac6fea6e3c826ef1e), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 3)" )
M4C9_SET( 199?, m4c9__c,	m4c9,	"c9510ad.p1",	0x0000, 0x010000, CRC(e1a6a573) SHA1(d653d8dce8d8df4151e2fcd8b93964e326bfbe7f), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 4)" )
M4C9_SET( 199?, m4c9__d,	m4c9,	"c9510b.p1",	0x0000, 0x010000, CRC(80c1d5bb) SHA1(5928f58f7963710e4ec9043aae4f656d98888e5b), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 5)" )
M4C9_SET( 199?, m4c9__e,	m4c9,	"c9510bd.p1",	0x0000, 0x010000, CRC(0aadc7d5) SHA1(143d937ef7b17d86d2e41065bb8f851b548ac8a3), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 6)" )
M4C9_SET( 199?, m4c9__f,	m4c9,	"c9510d.p1",	0x0000, 0x010000, CRC(e669989f) SHA1(a9ee5e1d309585f21882681a06f064f6ed03951f), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 7)" )
M4C9_SET( 199?, m4c9__g,	m4c9,	"c9510dk.p1",	0x0000, 0x010000, CRC(43be243e) SHA1(3974051fe47a192c135eceb2a7966e6a41b01a3d), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 8)" )
M4C9_SET( 199?, m4c9__h,	m4c9,	"c9510dr.p1",	0x0000, 0x010000, CRC(8edf7aa6) SHA1(ac15a8c1d0e24cc99452b560b68a664e16e8d82f), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 9)" )
M4C9_SET( 199?, m4c9__i,	m4c9,	"c9510dy.p1",	0x0000, 0x010000, CRC(b0ffae04) SHA1(81921a45a06c38a5391ed3edec57da74b220a181), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 10)" )
M4C9_SET( 199?, m4c9__j,	m4c9,	"c9510k.p1",	0x0000, 0x010000, CRC(665b330a) SHA1(75fe5fbe6f3b11a21092f6d18f7f50980c92febe), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 11)" )
M4C9_SET( 199?, m4c9__k,	m4c9,	"c9510r.p1",	0x0000, 0x010000, CRC(a9f25224) SHA1(3fe4091b27a2d789a8c5d00cb4fc00289639588f), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 12)" )
M4C9_SET( 199?, m4c9__l,	m4c9,	"c9510s.p1",	0x0000, 0x010000, CRC(dc70433e) SHA1(86f158909fea49baf4239821ccf092d8ef1027b7), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 13)" )
M4C9_SET( 199?, m4c9__m,	m4c9,	"c9510y.p1",	0x0000, 0x010000, CRC(3a93bc6a) SHA1(2832b48b6391746dbcea3484715dd6a169c081af), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 14)" )
M4C9_SET( 199?, m4c9__n,	m4c9,	"clnv.p1",		0x0000, 0x010000, CRC(486097d8) SHA1(33e9eab0fb1c750160a8cb2b75eca73145d6956e), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 15)" )
M4C9_SET( 199?, m4c9__o,	m4c9,	"c9211ad.p1",	0x0000, 0x010000, CRC(dcabab11) SHA1(d73f33da37decfc403975a844916b49d527ee8f8), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 16)" )
M4C9_SET( 199?, m4c9__p,	m4c9,	"c9211b.p1",	0x0000, 0x010000, CRC(2f10f98b) SHA1(4add53d98f31f4a8bedb621906e91e92622d2c95), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 17)" )
M4C9_SET( 199?, m4c9__q,	m4c9,	"c9211bd.p1",	0x0000, 0x010000, CRC(6dc2add7) SHA1(26a2b9cd629132d7ba48c9ea3476c574006ad4af), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 18)" )
M4C9_SET( 199?, m4c9__r,	m4c9,	"c9211c.p1",	0x0000, 0x010000, CRC(760ee71b) SHA1(ed124fc56a59c06b6ba8d250af5dbfd6154e55c3), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 19)" )
M4C9_SET( 199?, m4c9__s,	m4c9,	"c9211d.p1",	0x0000, 0x010000, CRC(1dd0166f) SHA1(d8f11fc2cd2efe0f6436ffbb31dd6b5c16bbe3ec), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 20)" )
M4C9_SET( 199?, m4c9__t,	m4c9,	"c9211dk.p1",	0x0000, 0x010000, CRC(0ee51f5c) SHA1(c773bf537f92a25b6d2f362d5ea1307eec8f1663), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 21)" )
M4C9_SET( 199?, m4c9__u,	m4c9,	"c9211dr.p1",	0x0000, 0x010000, CRC(35c30093) SHA1(c3b56e468cad9ef0f80983b9c05daa3f38c80a2c), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 22)" )
M4C9_SET( 199?, m4c9__v,	m4c9,	"c9211dy.p1",	0x0000, 0x010000, CRC(a8a8287d) SHA1(5e0de3b864251491d243984c5499650dafd8bb56), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 23)" )
M4C9_SET( 199?, m4c9__w,	m4c9,	"c9211k.p1",	0x0000, 0x010000, CRC(4f9b6b6d) SHA1(5722c0698c3915eb380b24468539dccad6978218), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 24)" )
M4C9_SET( 199?, m4c9__x,	m4c9,	"c9211r.p1",	0x0000, 0x010000, CRC(43f8b759) SHA1(cb0f731f1584e4d23602d276c085b31be6966bb1), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 25)" )
M4C9_SET( 199?, m4c9__y,	m4c9,	"c9211y.p1",	0x0000, 0x010000, CRC(de939fb7) SHA1(a305bdf247f498f86cd681fba7d0593a668067c7), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 26)" )
M4C9_SET( 199?, m4c9__z,	m4c9,	"ct202ad.p1",	0x0000, 0x010000, CRC(c8484dfd) SHA1(778fc30597b942fd75f5230ef3193b9f599abd03), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 27)" )
M4C9_SET( 199?, m4c9__0,	m4c9,	"ct202b.p1",	0x0000, 0x010000, CRC(b7c611aa) SHA1(d7d4e7d4d06e7198424206b8259ca66cc06062bb), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 28)" )
M4C9_SET( 199?, m4c9__1,	m4c9,	"ct202bd.p1",	0x0000, 0x010000, CRC(fe5420b7) SHA1(f443f1669b4f263b678526e2890671ad4e5848be), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 29)" )
M4C9_SET( 199?, m4c9__2,	m4c9,	"ct202c.p1",	0x0000, 0x010000, CRC(a0997fbb) SHA1(52d6172d6b737a65d24d6750847ccf2797eb54d4), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 30)" )
M4C9_SET( 199?, m4c9__3,	m4c9,	"ct202d.p1",	0x0000, 0x010000, CRC(5811f1a2) SHA1(87614b915aa697869739026bf45f53574123c6f2), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 31)" )
M4C9_SET( 199?, m4c9__4,	m4c9,	"ct202dk.p1",	0x0000, 0x010000, CRC(58857027) SHA1(bcb37032237c7542bfde915de815eb93b5def43e), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 32)" )
M4C9_SET( 199?, m4c9__5,	m4c9,	"ct202dr.p1",	0x0000, 0x010000, CRC(b2769912) SHA1(b3030c07a07774462e956201b5843e366df39c47), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 33)" )
M4C9_SET( 199?, m4c9__6,	m4c9,	"ct202dy.p1",	0x0000, 0x010000, CRC(47aa690b) SHA1(a3fd71dae7a94402641048b5e986f13347bc28ac), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 34)" )
M4C9_SET( 199?, m4c9__7,	m4c9,	"ct202k.p1",	0x0000, 0x010000, CRC(990cf3cd) SHA1(13d29f3111d193e8cca45d8319f8657066b2ac8a), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 35)" )
M4C9_SET( 199?, m4c9__8,	m4c9,	"ct202r.p1",	0x0000, 0x010000, CRC(0da3e958) SHA1(37760de8134e9298212ddebaebe79a08016da7e9), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 36)" )
M4C9_SET( 199?, m4c9__9,	m4c9,	"ct202s.p1",	0x0000, 0x010000, CRC(19214c6e) SHA1(93c8c40fd7b3a8873715e7bee88a09a995b44b28), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 37)" )
M4C9_SET( 199?, m4c9__aa,	m4c9,	"ct202y.p1",	0x0000, 0x010000, CRC(79362dcc) SHA1(80782ddb98f896101fa89f77ce76aa6f63391645), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 38)" )
M4C9_SET( 199?, m4c9__ab,	m4c9,	"ct302ad.p1",	0x0000, 0x010000, CRC(2f29a7e9) SHA1(059f73b6a9c2a1d8f9b8bbef9050c61c2d4f13bb), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 39)" )
M4C9_SET( 199?, m4c9__ac,	m4c9,	"ct302b.p1",	0x0000, 0x010000, CRC(1e677623) SHA1(c6ee2686f853626e390f28a611e8861cc8f935b0), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 40)" )
M4C9_SET( 199?, m4c9__ad,	m4c9,	"ct302bd.p1",	0x0000, 0x010000, CRC(70e60d8f) SHA1(139d92aa978df03af7b7913b6d4e56b211e9ddba), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 41)" )
M4C9_SET( 199?, m4c9__ae,	m4c9,	"ct302c.p1",	0x0000, 0x010000, CRC(6a157909) SHA1(cdb8a9a61bcb5e817305cfcde8ad5c3bd74b1cee), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 42)" )
M4C9_SET( 199?, m4c9__af,	m4c9,	"ct302d.p1",	0x0000, 0x010000, CRC(2ca799c7) SHA1(675647f6e1811f2ef1c79a1a49cbb1aaace66444), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 43)" )
M4C9_SET( 199?, m4c9__ag,	m4c9,	"ct302dk.p1",	0x0000, 0x010000, CRC(603baa69) SHA1(da00850aa2439a203f6d903d43c8657a3ce3327b), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 44)" )
M4C9_SET( 199?, m4c9__ah,	m4c9,	"ct302dr.p1",	0x0000, 0x010000, CRC(9b7b28d1) SHA1(6c365a508a87977aeccb13f0e842d882af5a8192), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 45)" )
M4C9_SET( 199?, m4c9__ai,	m4c9,	"ct302dy.p1",	0x0000, 0x010000, CRC(8da792a5) SHA1(38e4aff15b5de00090bf93834f7e215c450f26aa), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 46)" )
M4C9_SET( 199?, m4c9__aj,	m4c9,	"ct302k.p1",	0x0000, 0x010000, CRC(cfb85369) SHA1(c5726477aeea5a70e8eef74e57732fe85abea737), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 47)" )
M4C9_SET( 199?, m4c9__ak,	m4c9,	"ct302r.p1",	0x0000, 0x010000, CRC(10c64611) SHA1(d85df4ca0fc13ddab219a5602019e54471b83aaf), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 48)" )
M4C9_SET( 199?, m4c9__al,	m4c9,	"ct302y.p1",	0x0000, 0x010000, CRC(46514a44) SHA1(71e698c88488a67e94c322cb393f637c7e35d633), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 49)" )
M4C9_SET( 199?, m4c9__am,	m4c9,	"ct502ad.p1",	0x0000, 0x010000, CRC(ff0ec7a7) SHA1(80ddc21a0df33aaa1c76ed5f57598494a1c36c5a), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 50)" )
M4C9_SET( 199?, m4c9__an,	m4c9,	"ct502b.p1",	0x0000, 0x010000, CRC(2585dc82) SHA1(10ee12ecc6dfc09f9f9993b2fce837b0989c19ee), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 51)" )
M4C9_SET( 199?, m4c9__ao,	m4c9,	"ct502bd.p1",	0x0000, 0x010000, CRC(0d80572d) SHA1(6dfb48438accef039e2de12962ad826eaa3caee4), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 52)" )
M4C9_SET( 199?, m4c9__ap,	m4c9,	"ct502c.p1",	0x0000, 0x010000, CRC(713d24df) SHA1(bac23bdaecbfceeaffe67b5eb6a84210e43d6a90), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 53)" )
M4C9_SET( 199?, m4c9__aq,	m4c9,	"ct502d.p1",	0x0000, 0x010000, CRC(e3ca4d87) SHA1(ab6854e0f546b4100690cda2584f162b27b9ba86), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 54)" )
M4C9_SET( 199?, m4c9__ar,	m4c9,	"ct502dk.p1",	0x0000, 0x010000, CRC(1c39ef10) SHA1(1d5e027c171757072801f9078bd829d4d732c21e), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 55)" )
M4C9_SET( 199?, m4c9__as,	m4c9,	"ct502dr.p1",	0x0000, 0x010000, CRC(0e8781d2) SHA1(656b737a79885447ca7f30aca7a1123846408cd1), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 56)" )
M4C9_SET( 199?, m4c9__at,	m4c9,	"ct502dy.p1",	0x0000, 0x010000, CRC(54d27491) SHA1(ba4474f98474da828ebc7bf9db52ead05df0cdfc), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 57)" )
M4C9_SET( 199?, m4c9__au,	m4c9,	"ct502k.p1",	0x0000, 0x010000, CRC(f53ee613) SHA1(678f59b923054e6d91ea1bd91515b6522f192a8c), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 58)" )
M4C9_SET( 199?, m4c9__av,	m4c9,	"ct502r.p1",	0x0000, 0x010000, CRC(b678557d) SHA1(fbf3c367d40d2f914906eb7cd7e95713bfe7fc30), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 59)" )
M4C9_SET( 199?, m4c9__aw,	m4c9,	"ct502s.p1",	0x0000, 0x010000, CRC(cb02b9e7) SHA1(786c64abd0b9c5dc23b1508a2527e87e385acfa9), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 60)" )
M4C9_SET( 199?, m4c9__ax,	m4c9,	"ct502y.p1",	0x0000, 0x010000, CRC(f4cc4dc9) SHA1(d23757467830dfbdeed2a52a0c7e31276124d24d), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 61)" )
M4C9_SET( 199?, m4c9__ay,	m4c9,	"c9o02__1.1",	0x0000, 0x010000, CRC(109f7040) SHA1(3fe9da13d9746e1cdaf6dcd539e4af624d2cec71), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 62)" )
M4C9_SET( 199?, m4c9__az,	m4c9,	"c9o05__1.1",	0x0000, 0x010000, CRC(2c821aa8) SHA1(33fba7dea0f66e7b0251971864d5a2923f96f8cd), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 63)" )
M4C9_SET( 199?, m4c9__a0,	m4c9,	"c9o10__1.1",	0x0000, 0x010000, CRC(c5063185) SHA1(ca98038ccd85ebc370cacce8583ddbc1f759558d), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 64)" )
M4C9_SET( 199?, m4c9__a1,	m4c9,	"c9o10d_1.1",	0x0000, 0x010000, CRC(6b20b16d) SHA1(15079fc5f14f545c291d357a795e6b41ca1d5a47), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 65)" )
M4C9_SET( 199?, m4c9__a2,	m4c9,	"c9o20__1.1",	0x0000, 0x010000, CRC(e05fa532) SHA1(63d070416a4e6979302901bb33e20c994cb3723e), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 66)" )
M4C9_SET( 199?, m4c9__a3,	m4c9,	"c9o20d_1.1",	0x0000, 0x010000, CRC(047b2d83) SHA1(b83f8fe6477226ef3e75f406020ea4f8b3d55c32), "Barcrest","Cloud Nine (Barcrest) (MPU4) (set 67)" )


#define M4TUTFRT_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) \
	ROM_LOAD( "tftsnd02.p1", 0x000000, 0x080000, CRC(9789e60f) SHA1(7299eb4b6bb2fc90e8a36859102aad5daf66b163) ) \
	ROM_LOAD( "tftsnd02.p2", 0x080000, 0x080000, CRC(0bdc1dc9) SHA1(909af8ff4d0e3e36e280e9553a73bb1dfdb62144) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "tfsnd1.hex", 0x000000, 0x080000, CRC(a5b623fa) SHA1(eb4d84a7d3977ddea42c4995dddaabace73e6f8a) ) \
	ROM_LOAD( "tfsnd2.hex", 0x080000, 0x080000, CRC(1275e528) SHA1(0c3c901cb2be1e84dba123677205108cf0388343) ) \

#define M4TUTFRT_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TUTFRT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4TUTFRT_SET( 199?, m4tutfrt,		0,			"tft04s.p1",			0x0000, 0x010000, CRC(c20c3589) SHA1(55d1bc5d5f4ae14acafb36bd640faaf4ffccc6eb), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 1)" )
M4TUTFRT_SET( 199?, m4tutfrt__a,	m4tutfrt,	"ctuad.p1",				0x0000, 0x010000, CRC(0ec1661b) SHA1(162ddc30c341fd8eda8ce57a60edf06b4e39a24f), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 2)" )
M4TUTFRT_SET( 199?, m4tutfrt__b,	m4tutfrt,	"ctub.p1",				0x0000, 0x010000, CRC(f4289621) SHA1(a4078552146c88c05845cbdcd551e4564840fea4), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 3)" )
M4TUTFRT_SET( 199?, m4tutfrt__c,	m4tutfrt,	"ctubd.p1",				0x0000, 0x010000, CRC(38dd0b51) SHA1(04df9511f366cc575a1a06d3a5d60ec0245f64a7), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 4)" )
M4TUTFRT_SET( 199?, m4tutfrt__d,	m4tutfrt,	"ctud.p1",				0x0000, 0x010000, CRC(6033fae5) SHA1(f5bdd1821344d4546eea8caa52d76e3bd509810e), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 5)" )
M4TUTFRT_SET( 199?, m4tutfrt__e,	m4tutfrt,	"ctudk.p1",				0x0000, 0x010000, CRC(36dd1e41) SHA1(ad5ad7cae12634149d38e286e6873b81bda52871), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 6)" )
M4TUTFRT_SET( 199?, m4tutfrt__f,	m4tutfrt,	"ctudy.p1",				0x0000, 0x010000, CRC(58c02db6) SHA1(faf85caeaa0678b5771d801cf3d9645d7767767c), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 7)" )
M4TUTFRT_SET( 199?, m4tutfrt__g,	m4tutfrt,	"ctuk.p1",				0x0000, 0x010000, CRC(4c247447) SHA1(f5aebb4a75632c9a74dca1f3e9559399c89ac679), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 8)" )
M4TUTFRT_SET( 199?, m4tutfrt__h,	m4tutfrt,	"ctur.p1",				0x0000, 0x010000, CRC(705a2b52) SHA1(40b0738146d073f93877a15f63830ff3e07814c1), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 9)" )
M4TUTFRT_SET( 199?, m4tutfrt__i,	m4tutfrt,	"ctus.p1",				0x0000, 0x010000, CRC(1b282170) SHA1(e3082aed6e96587de56c5593d32d0129c47fe667), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 10)" )
M4TUTFRT_SET( 199?, m4tutfrt__j,	m4tutfrt,	"ctuy.p1",				0x0000, 0x010000, CRC(ed3103bc) SHA1(eefb72728e026fad3dd031665510ee0aba23e14b), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 11)" )
M4TUTFRT_SET( 199?, m4tutfrt__k,	m4tutfrt,	"f1u01ad.p1",			0x0000, 0x010000, CRC(7573d8cf) SHA1(fe1553ca8f588554fdd495dc2f048e50e00590bb), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 12)" )
M4TUTFRT_SET( 199?, m4tutfrt__l,	m4tutfrt,	"f1u01b.p1",			0x0000, 0x010000, CRC(158d1a3a) SHA1(da80334e9982f778a908a6fe89a593863e7c763e), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 13)" )
M4TUTFRT_SET( 199?, m4tutfrt__m,	m4tutfrt,	"f1u01bd.p1",			0x0000, 0x010000, CRC(9844e568) SHA1(a580176338cdeed5fb4d1744b537bde1f499293e), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 14)" )
M4TUTFRT_SET( 199?, m4tutfrt__n,	m4tutfrt,	"f1u01c.p1",			0x0000, 0x010000, CRC(4709bd66) SHA1(c15f64767315ea0434a57b9e494a9e8090f1e05a), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 15)" )
M4TUTFRT_SET( 199?, m4tutfrt__o,	m4tutfrt,	"f1u01d.p1",			0x0000, 0x010000, CRC(3a3c6745) SHA1(f270bccb4bdedb5cfaf0130da6e480dc31889682), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 16)" )
M4TUTFRT_SET( 199?, m4tutfrt__p,	m4tutfrt,	"f1u01dk.p1",			0x0000, 0x010000, CRC(4fa79f23) SHA1(ce9a0815d96a94d564edf5a775af94ea10070ff5), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 17)" )
M4TUTFRT_SET( 199?, m4tutfrt__q,	m4tutfrt,	"f1u01dr.p1",			0x0000, 0x010000, CRC(6fcc4d76) SHA1(27d8fdd5965ba565cb5b6113b7cba5e820650419), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 18)" )
M4TUTFRT_SET( 199?, m4tutfrt__r,	m4tutfrt,	"f1u01dy.p1",			0x0000, 0x010000, CRC(cdd43fc2) SHA1(6f4da20de3040675592b4338a1d72654800c20eb), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 19)" )
M4TUTFRT_SET( 199?, m4tutfrt__s,	m4tutfrt,	"f1u01k.p1",			0x0000, 0x010000, CRC(7e9c3110) SHA1(56ab6e5362ce8795c65d0cf11742e3ddb6d8b8a3), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 20)" )
M4TUTFRT_SET( 199?, m4tutfrt__t,	m4tutfrt,	"f1u01r.p1",			0x0000, 0x010000, CRC(0e6b2132) SHA1(8757713677e2eb0400c69d3cdde6506662e0ef0b), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 21)" )
M4TUTFRT_SET( 199?, m4tutfrt__u,	m4tutfrt,	"f1u01s.p1",			0x0000, 0x010000, CRC(d69668d2) SHA1(86ea656a3a4d4e6701c70b5e730ae8402cd70342), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 22)" )
M4TUTFRT_SET( 199?, m4tutfrt__v,	m4tutfrt,	"f1u01y.p1",			0x0000, 0x010000, CRC(33e7d5fd) SHA1(96f53fbb228e98ce3a848b2c72bdb8876c9de160), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 23)" )
M4TUTFRT_SET( 199?, m4tutfrt__w,	m4tutfrt,	"f3u01ad.p1",			0x0000, 0x010000, CRC(acb1bfb3) SHA1(8aa22c45d98ecec324fa031b46689496f9a2842c), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 24)" )
M4TUTFRT_SET( 199?, m4tutfrt__x,	m4tutfrt,	"f3u01b.p1",			0x0000, 0x010000, CRC(a0d14e25) SHA1(16f2444334608702748a3b0b2556ac1a7760615a), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 25)" )
M4TUTFRT_SET( 199?, m4tutfrt__y,	m4tutfrt,	"f3u01bd.p1",			0x0000, 0x010000, CRC(9aadd2f9) SHA1(4dbff4f6fd4d02778733eb846a354177f0e204a5), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 26)" )
M4TUTFRT_SET( 199?, m4tutfrt__z,	m4tutfrt,	"f3u01c.p1",			0x0000, 0x010000, CRC(a3ad34d5) SHA1(e8c435f80b4fd3f7af16f341e107a85a33f1fe1c), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 27)" )
M4TUTFRT_SET( 199?, m4tutfrt__0,	m4tutfrt,	"f3u01d.p1",			0x0000, 0x010000, CRC(c6790301) SHA1(fb0b619e75e1227f4d293b613e80d8d653517eec), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 28)" )
M4TUTFRT_SET( 199?, m4tutfrt__1,	m4tutfrt,	"f3u01dk.p1",			0x0000, 0x010000, CRC(ee0554fe) SHA1(12cd26d6205fec35590fd23682c578f06466eb01), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 29)" )
M4TUTFRT_SET( 199?, m4tutfrt__2,	m4tutfrt,	"f3u01dr.p1",			0x0000, 0x010000, CRC(32d761eb) SHA1(aa1098629d2a1c98c606a71a7cf0ae97f381aebe), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 30)" )
M4TUTFRT_SET( 199?, m4tutfrt__3,	m4tutfrt,	"f3u01dy.p1",			0x0000, 0x010000, CRC(3ad66969) SHA1(4c79edc52095cfa1fae8215caaaaf434cd38199d), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 31)" )
M4TUTFRT_SET( 199?, m4tutfrt__4,	m4tutfrt,	"f3u01k.p1",			0x0000, 0x010000, CRC(2b6c0f0f) SHA1(64e50adc6656225c9cdaaee64ae59cafcd1623ee), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 32)" )
M4TUTFRT_SET( 199?, m4tutfrt__5,	m4tutfrt,	"f3u01r.p1",			0x0000, 0x010000, CRC(93cb1bfb) SHA1(e29439caed4a2f4512e50ff158427b61b5a9c4a9), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 33)" )
M4TUTFRT_SET( 199?, m4tutfrt__6,	m4tutfrt,	"f3u01s.p1",			0x0000, 0x010000, CRC(dce2e5be) SHA1(3c218cdb939d5b7cc650c820737ae3ac653435ce), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 34)" )
M4TUTFRT_SET( 199?, m4tutfrt__7,	m4tutfrt,	"f3u01y.p1",			0x0000, 0x010000, CRC(9aae0ca2) SHA1(83192225d886848ee0320973fb9dbd85cf9045b8), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 35)" )
M4TUTFRT_SET( 199?, m4tutfrt__8,	m4tutfrt,	"tf4ad.p1",				0x0000, 0x010000, CRC(6ddc90a9) SHA1(76dd22c5e65fc46360123e200016d11a8946d2f3), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 36)" )
M4TUTFRT_SET( 199?, m4tutfrt__9,	m4tutfrt,	"tf4b.p1",				0x0000, 0x010000, CRC(c3a70eac) SHA1(ea5a39e33af96e84ce0ea184850d5f580dbf19ce), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 37)" )
M4TUTFRT_SET( 199?, m4tutfrt__aa,	m4tutfrt,	"tf4bd.p1",				0x0000, 0x010000, CRC(54ae2498) SHA1(54a63a0de794eb2ce321f79b09a56485d9e77715), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 38)" )
M4TUTFRT_SET( 199?, m4tutfrt__ab,	m4tutfrt,	"tf4d.p1",				0x0000, 0x010000, CRC(d8ff9045) SHA1(ae7307212614c6f1b4e3d72d3a1ae68ca1d0b470), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 39)" )
M4TUTFRT_SET( 199?, m4tutfrt__ac,	m4tutfrt,	"tf4dk.p1",				0x0000, 0x010000, CRC(a2e3b67f) SHA1(dea9958caba08b5cdec6eec9e4c17038ecb0ca55), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 40)" )
M4TUTFRT_SET( 199?, m4tutfrt__ad,	m4tutfrt,	"tf4dy.p1",				0x0000, 0x010000, CRC(ff4f26c4) SHA1(21ef226bf92deeab15c9368d707bf75b7104e7c3), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 41)" )
M4TUTFRT_SET( 199?, m4tutfrt__ae,	m4tutfrt,	"tf4k.p1",				0x0000, 0x010000, CRC(1a4eb247) SHA1(f6b4c85dd8b155b672bd96ea7ee6630df773c6ca), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 42)" )
M4TUTFRT_SET( 199?, m4tutfrt__af,	m4tutfrt,	"tf4s.p1",				0x0000, 0x010000, CRC(2d298c58) SHA1(568c2babdb002da871df7a36d16e4f7810cac265), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 43)" )
M4TUTFRT_SET( 199?, m4tutfrt__ag,	m4tutfrt,	"tf4y.p1",				0x0000, 0x010000, CRC(06cd8b06) SHA1(92205e9edd42f80de67d5d6652de8ea80bc60af7), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 44)" )
M4TUTFRT_SET( 199?, m4tutfrt__ai,	m4tutfrt,	"tft04ad.p1",			0x0000, 0x010000, CRC(2994aa14) SHA1(af0e618f24cdedd14e3a347701313360d9fc73d1), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 46)" )
M4TUTFRT_SET( 199?, m4tutfrt__aj,	m4tutfrt,	"tft04b.p1",			0x0000, 0x010000, CRC(e95eab06) SHA1(70e85e38493ac1fd30a79582bab45af5227d835a), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 47)" )
M4TUTFRT_SET( 199?, m4tutfrt__ak,	m4tutfrt,	"tft04bd.p1",			0x0000, 0x010000, CRC(060d3572) SHA1(e78b6248d3aef6cd08f4b30e0b00bd4cf254e630), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 48)" )
M4TUTFRT_SET( 199?, m4tutfrt__al,	m4tutfrt,	"tft04c.p1",			0x0000, 0x010000, CRC(3499fe77) SHA1(3f82ca6d856bddf82581790c46abf725963335a0), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 49)" )
M4TUTFRT_SET( 199?, m4tutfrt__am,	m4tutfrt,	"tft04d.p1",			0x0000, 0x010000, CRC(10626059) SHA1(c7b2fd2b65946fe82950ff506a56bd08b7c2ef71), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 50)" )
M4TUTFRT_SET( 199?, m4tutfrt__an,	m4tutfrt,	"tft04dk.p1",			0x0000, 0x010000, CRC(40700fe2) SHA1(1f121adae094c2d11a66b5e8ae4b026e85fc7f73), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 51)" )
M4TUTFRT_SET( 199?, m4tutfrt__ao,	m4tutfrt,	"tft04dr.p1",			0x0000, 0x010000, CRC(feeb4417) SHA1(e2f2c55c48067ad67188ff5a75caa08d8726cb77), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 52)" )
M4TUTFRT_SET( 199?, m4tutfrt__ap,	m4tutfrt,	"tft04dy.p1",			0x0000, 0x010000, CRC(63806cf9) SHA1(850c707c65b8dba6b6914389d573a8b7b7b12cdb), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 53)" )
M4TUTFRT_SET( 199?, m4tutfrt__aq,	m4tutfrt,	"tft04k.p1",			0x0000, 0x010000, CRC(ffbf53e1) SHA1(a003bb5d94b43d6ae9b45c599cccb0006bd8a89a), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 54)" )
M4TUTFRT_SET( 199?, m4tutfrt__ar,	m4tutfrt,	"tft04r.p1",			0x0000, 0x010000, CRC(cbf79555) SHA1(0aacb3f28984637919294a18f40858e8f46a18b3), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 55)" )
M4TUTFRT_SET( 199?, m4tutfrt__as,	m4tutfrt,	"tft04y.p1",			0x0000, 0x010000, CRC(569cbdbb) SHA1(8a978dfba876e5a2e12226f5fe55c29b5f079fad), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 56)" )
M4TUTFRT_SET( 199?, m4tutfrt__at,	m4tutfrt,	"tut25.bin",			0x0000, 0x010000, CRC(c98fb5bb) SHA1(1a3bc343a38b5978a919b454e9a2e806dce7a78a), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 57)" )
M4TUTFRT_SET( 199?, m4tutfrt__au,	m4tutfrt,	"tut25patched.bin",		0x0000, 0x010000, CRC(b4443cf5) SHA1(e79ec52730146f1591140555b814cbd20b5dfe78), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 58)" )
M4TUTFRT_SET( 199?, m4tutfrt__av,	m4tutfrt,	"tu_05___.1a3",			0x0000, 0x010000, CRC(97acc82d) SHA1(be53e60cb8a33b91a7f5556715ab4befe7170dd2), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 59)" )
M4TUTFRT_SET( 199?, m4tutfrt__aw,	m4tutfrt,	"tu_05_d_.1a3",			0x0000, 0x010000, CRC(33bb3018) SHA1(2c2f49c31919682ac03e61a665ce15d835e22467), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 60)" )
M4TUTFRT_SET( 199?, m4tutfrt__ax,	m4tutfrt,	"tu_10___.1a3",			0x0000, 0x010000, CRC(7878827f) SHA1(ac692ae50e63e632d45e7240c2520df83d2baaf5), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 61)" )
M4TUTFRT_SET( 199?, m4tutfrt__ay,	m4tutfrt,	"tu_20___.1a3",			0x0000, 0x010000, CRC(cada1c42) SHA1(6a4048da89a0bffeebfd21549c2d9812cc275bd5), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 62)" )
M4TUTFRT_SET( 199?, m4tutfrt__az,	m4tutfrt,	"tu_20_b_.1a3",			0x0000, 0x010000, CRC(a8f1bc11) SHA1(03596171540e6490133f374cca69f4fd0359952e), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 63)" )
M4TUTFRT_SET( 199?, m4tutfrt__a0,	m4tutfrt,	"tu_20_d_.1a3",			0x0000, 0x010000, CRC(6ecde477) SHA1(694296eb226c59069800d6936c9dee2623105db0), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 64)" )
M4TUTFRT_SET( 199?, m4tutfrt__a1,	m4tutfrt,	"tu_20_k_.1a3",			0x0000, 0x010000, CRC(0ce64424) SHA1(7415c9de9982aa7f15f71ef791cbd8ad5a9331d3), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 65)" )
M4TUTFRT_SET( 199?, m4tutfrt__a2,	m4tutfrt,	"tu_20bg_.1a3",			0x0000, 0x010000, CRC(31a6196d) SHA1(1113737dd3b209afda14ec273d923e2057ea7d99), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 66)" )
M4TUTFRT_SET( 199?, m4tutfrt__a3,	m4tutfrt,	"tuf20__1.0",			0x0000, 0x010000, CRC(ddadbcb6) SHA1(2d2934ec73d979de45d0998f8975361d33358dd3), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 67)" )
M4TUTFRT_SET( 199?, m4tutfrt__a4,	m4tutfrt,	"tuf20ad1.0",			0x0000, 0x010000, CRC(5a74ead3) SHA1(3216c8d0c67aaeb18f791a6e1f3f6e30145d6beb), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 68)" )
M4TUTFRT_SET( 199?, m4tutfrt__a5,	m4tutfrt,	"tui05___.1a3",			0x0000, 0x010000, CRC(42e3d400) SHA1(4cf914141dfc1f88704403b467176da77369da06), "Barcrest","Tutti Fruity (Barcrest) (MPU4) (set 69)" )


#define M4CASHAT_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "cas1.hex", 0x000000, 0x080000, CRC(4711c483) SHA1(af1ceb317b7bb1c2d0c3f7a99049679c356e1860) ) \
	ROM_LOAD( "cas2.hex", 0x080000, 0x080000, CRC(26ec235c) SHA1(51de955e5def47b82ac8891d09dc0b0e5e19c01d) ) \

#define M4CASHAT_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CASHAT_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4CASHAT_SET( 199?, m4cashat,		0,			"csa12y.p1",		0x0000, 0x020000, CRC(0374584a) SHA1(446e1d122d5b38e4ee11d98a4235d7198d98b541), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 1)" )
M4CASHAT_SET( 199?, m4cashat__a,	m4cashat,	"caa22ad.p1",		0x0000, 0x020000, CRC(b6274874) SHA1(7c2dc0f3e8e7bb76f3b90300141b320fa0ca39ac), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 2)" )
M4CASHAT_SET( 199?, m4cashat__b,	m4cashat,	"caa22b.p1",		0x0000, 0x020000, CRC(e7f6f5e5) SHA1(fc16b50ae00525a3c84c0cbf7b418898cc5db1bc), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 3)" )
M4CASHAT_SET( 199?, m4cashat__c,	m4cashat,	"caa22bd.p1",		0x0000, 0x020000, CRC(581b2b6f) SHA1(55f910c7646d5e7d3be6ffd5b4ec0f04fb98b82e), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 4)" )
M4CASHAT_SET( 199?, m4cashat__d,	m4cashat,	"caa22d.p1",		0x0000, 0x020000, CRC(cc494044) SHA1(13ff215f41833aa133fe9d120792c834d1e0752b), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 5)" )
M4CASHAT_SET( 199?, m4cashat__e,	m4cashat,	"caa22dh.p1",		0x0000, 0x020000, CRC(18ae14fa) SHA1(20a8f197075ec153ac116b9a85e3591d9d4d045d), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 6)" )
M4CASHAT_SET( 199?, m4cashat__f,	m4cashat,	"caa22dk.p1",		0x0000, 0x020000, CRC(71fa4ee7) SHA1(ddf2cee47f93cc5794d64922658d5892993c8d2f), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 7)" )
M4CASHAT_SET( 199?, m4cashat__g,	m4cashat,	"caa22dr.p1",		0x0000, 0x020000, CRC(c2f5f5d6) SHA1(aebedb84ae388a1f0c558d36893d1341c1959594), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 8)" )
M4CASHAT_SET( 199?, m4cashat__h,	m4cashat,	"caa22dy.p1",		0x0000, 0x020000, CRC(3b3de6b1) SHA1(d72ce7851969466063c6d7952787691a7c44c9dd), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 9)" )
M4CASHAT_SET( 199?, m4cashat__i,	m4cashat,	"caa22h.p1",		0x0000, 0x020000, CRC(a743ca70) SHA1(e4b5ee02524873c2ccb66b4bfca39464c23eb43e), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 10)" )
M4CASHAT_SET( 199?, m4cashat__j,	m4cashat,	"caa22k.p1",		0x0000, 0x020000, CRC(ce17906d) SHA1(18a302132e683b00509982c09c6e3b00ae1201a0), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 11)" )
M4CASHAT_SET( 199?, m4cashat__k,	m4cashat,	"caa22r.p1",		0x0000, 0x020000, CRC(7d182b5c) SHA1(801d1b032e94cc45302a9f84ba7f9ce2b74f6449), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 12)" )
M4CASHAT_SET( 199?, m4cashat__l,	m4cashat,	"caa22s.p1",		0x0000, 0x020000, CRC(e7edf653) SHA1(f2bdf45cc18ad4b45b47d2b2b4641460fcdfa963), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 13)" )
M4CASHAT_SET( 199?, m4cashat__m,	m4cashat,	"caa22y.p1",		0x0000, 0x020000, CRC(84d0383b) SHA1(791666ce17fd65067df446a3320efd22bce23925), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 14)" )
M4CASHAT_SET( 199?, m4cashat__n,	m4cashat,	"caa23ad.p1",		0x0000, 0x020000, CRC(a8641c35) SHA1(18dad4634e27e4f0b791c331b9efcf5e1d56d3bb), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 15)" )
M4CASHAT_SET( 199?, m4cashat__o,	m4cashat,	"caa23b.p1",		0x0000, 0x020000, CRC(a867c129) SHA1(9b0b577938ae0500a8b80211710ed5c0b2a597fa), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 16)" )
M4CASHAT_SET( 199?, m4cashat__p,	m4cashat,	"caa23bd.p1",		0x0000, 0x020000, CRC(46587f2e) SHA1(b14ed6b810ba3039824a0d13c5b75fedd40803b3), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 17)" )
M4CASHAT_SET( 199?, m4cashat__q,	m4cashat,	"caa23d.p1",		0x0000, 0x020000, CRC(83d87488) SHA1(1e13a47de4837e42650c6a4a13a838eb68d0beae), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 18)" )
M4CASHAT_SET( 199?, m4cashat__r,	m4cashat,	"caa23dh.p1",		0x0000, 0x020000, CRC(06ed40bb) SHA1(68f2923c4ecd91231cc66a4be7c797d7b2a46ae0), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 19)" )
M4CASHAT_SET( 199?, m4cashat__s,	m4cashat,	"caa23dk.p1",		0x0000, 0x020000, CRC(6fb91aa6) SHA1(5966be8aa9d5348bbdcb85b21acceaabc3c02602), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 20)" )
M4CASHAT_SET( 199?, m4cashat__t,	m4cashat,	"caa23dr.p1",		0x0000, 0x020000, CRC(dcb6a197) SHA1(a18af78ba604b53a2af1e9b8dfdc6858964f631d), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 21)" )
M4CASHAT_SET( 199?, m4cashat__u,	m4cashat,	"caa23dy.p1",		0x0000, 0x020000, CRC(257eb2f0) SHA1(0a5f9743afb5dd7392425951580532ea5f8f17f1), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 22)" )
M4CASHAT_SET( 199?, m4cashat__v,	m4cashat,	"caa23h.p1",		0x0000, 0x020000, CRC(e8d2febc) SHA1(14fe5e1699fef74145f2f6fff61e75fe3e3a0b3b), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 23)" )
M4CASHAT_SET( 199?, m4cashat__w,	m4cashat,	"caa23k.p1",		0x0000, 0x020000, CRC(8186a4a1) SHA1(0d8f59df0fb5a1044f6fb7d81f50f9c9b94add9b), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 24)" )
M4CASHAT_SET( 199?, m4cashat__x,	m4cashat,	"caa23r.p1",		0x0000, 0x020000, CRC(32891f90) SHA1(c832c2610606bc5a3beeff8f85c31af496b14427), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 25)" )
M4CASHAT_SET( 199?, m4cashat__y,	m4cashat,	"caa23s.p1",		0x0000, 0x020000, CRC(26a49cdd) SHA1(ee28a22eeb8c4e8ddf041122505f9846d6b6d7d6), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 26)" )
M4CASHAT_SET( 199?, m4cashat__z,	m4cashat,	"caa23y.p1",		0x0000, 0x020000, CRC(cb410cf7) SHA1(31d34a766939a9b2a23be00c2ffd658d854b3ab4), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 27)" )
M4CASHAT_SET( 199?, m4cashat__0,	m4cashat,	"casattack8.bin",	0x0000, 0x020000, CRC(e29ea247) SHA1(ad00ea3bfd2eab51b20fd786cb1ce84de0d98173), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 28)" )
M4CASHAT_SET( 199?, m4cashat__1,	m4cashat,	"catt15g",			0x0000, 0x020000, CRC(3f7a8863) SHA1(df8ed393aeb3a5ec3fd5bdc01c9dbbb630e6d254), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 29)" )
M4CASHAT_SET( 199?, m4cashat__2,	m4cashat,	"catt15t",			0x0000, 0x020000, CRC(c6760c3a) SHA1(b7f4a3af52faf7e430e5b4ec75e2dc97e3f07dc0), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 30)" )
M4CASHAT_SET( 199?, m4cashat__3,	m4cashat,	"csa11ad.p1",		0x0000, 0x020000, CRC(7c1daa59) SHA1(9c0479094ba2f985803e58360b738b0baa2e410a), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 31)" )
M4CASHAT_SET( 199?, m4cashat__4,	m4cashat,	"csa11b.p1",		0x0000, 0x020000, CRC(c740daba) SHA1(afa5bdf9f6aacb3a5126aa828e4d0d2518efe663), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 32)" )
M4CASHAT_SET( 199?, m4cashat__5,	m4cashat,	"csa11bd.p1",		0x0000, 0x020000, CRC(55fccfd1) SHA1(b7c748573e5fb32a6be5e069e7f165a11c62b7d5), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 33)" )
M4CASHAT_SET( 199?, m4cashat__6,	m4cashat,	"csa11d.p1",		0x0000, 0x020000, CRC(ecff6f1b) SHA1(3f37d8e20d5663e376c9dc5876a910a320edcf7d), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 34)" )
M4CASHAT_SET( 199?, m4cashat__7,	m4cashat,	"csa11dh.p1",		0x0000, 0x020000, CRC(bbc0acca) SHA1(80d95505041fd4c869f9d835d0527070f2f582d9), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 35)" )
M4CASHAT_SET( 199?, m4cashat__8,	m4cashat,	"csa11dk.p1",		0x0000, 0x020000, CRC(1549f044) SHA1(22bc130106a23d0e9c354b4aa97d7b7fd8776082), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 36)" )
M4CASHAT_SET( 199?, m4cashat__9,	m4cashat,	"csa11dr.p1",		0x0000, 0x020000, CRC(cf121168) SHA1(aa52b528ac565684399dc58aeb56691457727035), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 37)" )
M4CASHAT_SET( 199?, m4cashat__aa,	m4cashat,	"csa11dy.p1",		0x0000, 0x020000, CRC(36da020f) SHA1(ca202c7127450d905e4717776e1f1d32fa89279b), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 38)" )
M4CASHAT_SET( 199?, m4cashat__ab,	m4cashat,	"csa11h.p1",		0x0000, 0x020000, CRC(297cb9a1) SHA1(63460eed75242fc7c27ee1fc9da28221e7bb21b1), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 39)" )
M4CASHAT_SET( 199?, m4cashat__ac,	m4cashat,	"csa11k.p1",		0x0000, 0x020000, CRC(87f5e52f) SHA1(30b7f8c17198045bba30aaabbe74b3c1dc7d0320), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 40)" )
M4CASHAT_SET( 199?, m4cashat__ad,	m4cashat,	"csa11r.p1",		0x0000, 0x020000, CRC(5dae0403) SHA1(6f2238f0fe0797bf0926044bb251fed6f97dbed6), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 41)" )
M4CASHAT_SET( 199?, m4cashat__ae,	m4cashat,	"csa11s.p1",		0x0000, 0x020000, CRC(bef7a119) SHA1(88fc2003a7adda928e2e0fb78db32c7ffcbda924), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 42)" )
M4CASHAT_SET( 199?, m4cashat__af,	m4cashat,	"csa11y.p1",		0x0000, 0x020000, CRC(a4661764) SHA1(740be82275358b8e3dcec5982b18a083d043d99d), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 43)" )
M4CASHAT_SET( 199?, m4cashat__ag,	m4cashat,	"csa12ad.p1",		0x0000, 0x020000, CRC(b15c5c64) SHA1(7a8c7b929ecaf0e14d9a5d6cdea303f5e3fc1dec), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 44)" )
M4CASHAT_SET( 199?, m4cashat__ah,	m4cashat,	"csa12b.p1",		0x0000, 0x020000, CRC(60529594) SHA1(a5e70b55b8df6a94c963b970c3a4398b64b0286b), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 45)" )
M4CASHAT_SET( 199?, m4cashat__ai,	m4cashat,	"csa12bd.p1",		0x0000, 0x020000, CRC(98bd39ec) SHA1(ebc5a2690f1453adae0f8faee0159a01df91dd6e), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 46)" )
M4CASHAT_SET( 199?, m4cashat__aj,	m4cashat,	"csa12d.p1",		0x0000, 0x020000, CRC(4bed2035) SHA1(d5438d372222c4258ffb6487ba64eed9ce190133), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 47)" )
M4CASHAT_SET( 199?, m4cashat__ak,	m4cashat,	"csa12dh.p1",		0x0000, 0x020000, CRC(76815af7) SHA1(6edd4a866a1b038a51cfcc9ed8cef48886b393fb), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 48)" )
M4CASHAT_SET( 199?, m4cashat__al,	m4cashat,	"csa12dk.p1",		0x0000, 0x020000, CRC(d8080679) SHA1(92babec65fbcee37ff8136a5c4b1e5f4ecd2f5a6), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 49)" )
M4CASHAT_SET( 199?, m4cashat__am,	m4cashat,	"csa12dr.p1",		0x0000, 0x020000, CRC(0253e755) SHA1(742174137549147ff23fbc9ba1b835cbeaffa602), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 50)" )
M4CASHAT_SET( 199?, m4cashat__an,	m4cashat,	"csa12dy.p1",		0x0000, 0x020000, CRC(fb9bf432) SHA1(5f519871cc50cf9f49ec652d620267cd11ab155b), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 51)" )
M4CASHAT_SET( 199?, m4cashat__ao,	m4cashat,	"csa12h.p1",		0x0000, 0x020000, CRC(8e6ef68f) SHA1(b6ac0993938bb065f02498a71628cf532085b347), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 52)" )
M4CASHAT_SET( 199?, m4cashat__ap,	m4cashat,	"csa12k.p1",		0x0000, 0x020000, CRC(20e7aa01) SHA1(093786b0992c1d9ce5e2d2cfad1eaf1d8e6dc733), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 53)" )
M4CASHAT_SET( 199?, m4cashat__aq,	m4cashat,	"csa12r.p1",		0x0000, 0x020000, CRC(fabc4b2d) SHA1(3710b7b4bf56e46c60a60fcae82342bf201e38dc), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 54)" )
M4CASHAT_SET( 199?, m4cashat__ar,	m4cashat,	"csa12s.p1",		0x0000, 0x020000, CRC(61c8af36) SHA1(d81a4056b573194a8627a3618f805d379140ff6a), "Barcrest","Cash Attack (Barcrest) (MPU4) (set 55)" )


#define M4RHR_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) \
	ROM_LOAD( "rhrsnd1.hex", 0x0000, 0x080000, CRC(3e80f8bd) SHA1(2e3a195b49448da11cc0c089a8a9b462894c766b) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "redhotroll10.bin", 0x0000, 0x080000, CRC(64513503) SHA1(4233492f3f6e7ad8459f1ab733727910d3b4bcf8) ) \

#define M4RHR_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHR_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4RHR_SET( 199?, m4rhr,		0,		"rhr15.hex",		0x0000, 0x010000, CRC(895ebbda) SHA1(f2117e743a30f3c9fc6af7fd7843bc333699db9d), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 1)" )
M4RHR_SET( 199?, m4rhr__a,	m4rhr,	"cr4ad.p1",			0x0000, 0x010000, CRC(b99b3d14) SHA1(2ff68b33881e9b3c2db48c335ccbad783013084a), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 2)" )
M4RHR_SET( 199?, m4rhr__b,	m4rhr,	"cr4b.p1",			0x0000, 0x010000, CRC(ae2691b8) SHA1(360c5c3d94bf85cf5ead114dd570ea6c61082aa9), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 3)" )
M4RHR_SET( 199?, m4rhr__c,	m4rhr,	"cr4bd.p1",			0x0000, 0x010000, CRC(9ba444bf) SHA1(adebf23827a5ac5e3a6d56e3352e0d3f3dc809c0), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 4)" )
M4RHR_SET( 199?, m4rhr__d,	m4rhr,	"cr4d.p1",			0x0000, 0x010000, CRC(ad9fe2a6) SHA1(e490c5c949559cc222d8491989196b10373ff043), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 5)" )
M4RHR_SET( 199?, m4rhr__e,	m4rhr,	"cr4dk.p1",			0x0000, 0x010000, CRC(200486b4) SHA1(3916e131801c44985668ccd57dc3e812268f9417), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 6)" )
M4RHR_SET( 199?, m4rhr__f,	m4rhr,	"cr4dy.p1",			0x0000, 0x010000, CRC(5b5ebe79) SHA1(6c72271258e6b951f2d6c815cfef5032e23cf7bc), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 7)" )
M4RHR_SET( 199?, m4rhr__g,	m4rhr,	"cr4k.p1",			0x0000, 0x010000, CRC(2cc956e8) SHA1(37fad3d3b9460763ba4d8f569ee71778f9907853), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 8)" )
M4RHR_SET( 199?, m4rhr__h,	m4rhr,	"cr4s.p1",			0x0000, 0x010000, CRC(836c3e49) SHA1(34dde2fd4fe82ab4a9e16dcf7915705f7b8a007f), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 9)" )
M4RHR_SET( 199?, m4rhr__i,	m4rhr,	"cr4y.p1",			0x0000, 0x010000, CRC(5a3588e8) SHA1(b25156f38fb67dc1f1e36a50af0a9b93882572d0), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 10)" )
M4RHR_SET( 199?, m4rhr__j,	m4rhr,	"crt03ad.p1",		0x0000, 0x010000, CRC(5b779273) SHA1(b9a278cc6b4af622af35f7d4fdacdca54c94a47f), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 11)" )
M4RHR_SET( 199?, m4rhr__k,	m4rhr,	"crt03b.p1",		0x0000, 0x010000, CRC(da5b3fa3) SHA1(66c570a193665ae0df4542112547fa6f5f9b7b79), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 12)" )
M4RHR_SET( 199?, m4rhr__l,	m4rhr,	"crt03bd.p1",		0x0000, 0x010000, CRC(6d6bff39) SHA1(08f4235bb2cadcc49c13991fe3e2c806c0be801d), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 13)" )
M4RHR_SET( 199?, m4rhr__m,	m4rhr,	"crt03c.p1",		0x0000, 0x010000, CRC(a5b38945) SHA1(31351667d471c107ade58e97fe5657632d91be80), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 14)" )
M4RHR_SET( 199?, m4rhr__n,	m4rhr,	"crt03d.p1",		0x0000, 0x010000, CRC(7f39cf9d) SHA1(6f8a1660a253cf7f49ba589b3847ca3dc5a9b4ee), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 15)" )
M4RHR_SET( 199?, m4rhr__o,	m4rhr,	"crt03dk.p1",		0x0000, 0x010000, CRC(32933785) SHA1(0ae9b8823ed8c914da0a64913afdf3c348142804), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 16)" )
M4RHR_SET( 199?, m4rhr__p,	m4rhr,	"crt03dr.p1",		0x0000, 0x010000, CRC(2381792a) SHA1(514b9e580d156ec3cfeb460d0895143368e9a360), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 17)" )
M4RHR_SET( 199?, m4rhr__q,	m4rhr,	"crt03dy.p1",		0x0000, 0x010000, CRC(3439dc85) SHA1(092dcd36e2ea43ecf62cfc1bf1498ea7777213dc), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 18)" )
M4RHR_SET( 199?, m4rhr__r,	m4rhr,	"crt03k.p1",		0x0000, 0x010000, CRC(0b841ae9) SHA1(5a78381122a3b718e3f212f30f76dc61e2e3ac5e), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 19)" )
M4RHR_SET( 199?, m4rhr__s,	m4rhr,	"crt03r.p1",		0x0000, 0x010000, CRC(2a8bd767) SHA1(a9547ef37da9494bd4ffe5fbb68eca67fe63c3ba), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 20)" )
M4RHR_SET( 199?, m4rhr__t,	m4rhr,	"crt03s.p1",		0x0000, 0x010000, CRC(2b4c24d2) SHA1(94b19b0e8090dbbde2c67d5949f19d4050972fb1), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 21)" )
M4RHR_SET( 199?, m4rhr__u,	m4rhr,	"crt03y.p1",		0x0000, 0x010000, CRC(40c3a105) SHA1(7ad988f71a3523ad2b19fa7d6cdf74d4328fb3e1), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 22)" )
M4RHR_SET( 199?, m4rhr__v,	m4rhr,	"cruad.p1",			0x0000, 0x010000, CRC(3a680f14) SHA1(cd3c2bf77b148ee4f4ce76b2c1bc142491117890), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 23)" )
M4RHR_SET( 199?, m4rhr__w,	m4rhr,	"crub.p1",			0x0000, 0x010000, CRC(4cee9020) SHA1(b919ba28294c39b49e4fcfa54a75e852f9c873ed), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 24)" )
M4RHR_SET( 199?, m4rhr__x,	m4rhr,	"crubd.p1",			0x0000, 0x010000, CRC(7184b193) SHA1(392cb5887ec988e3aa1cba2491885103da1e503a), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 25)" )
M4RHR_SET( 199?, m4rhr__y,	m4rhr,	"crud.p1",			0x0000, 0x010000, CRC(2528047f) SHA1(0b07470ff756b003c03fd4a7ff3c1d5f79e8307f), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 26)" )
M4RHR_SET( 199?, m4rhr__z,	m4rhr,	"crudk.p1",			0x0000, 0x010000, CRC(73465d95) SHA1(3eddaee64a681727743b23fd0bec0285ed59a5ef), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 27)" )
M4RHR_SET( 199?, m4rhr__0,	m4rhr,	"crudy.p1",			0x0000, 0x010000, CRC(e08696f9) SHA1(37c97bb22ae0d09657d7d589f76adfbe6fb642e0), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 28)" )
M4RHR_SET( 199?, m4rhr__1,	m4rhr,	"cruk.p1",			0x0000, 0x010000, CRC(168627f0) SHA1(c6c21f8442ff88736d3fd25860d815beb5a6b845), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 29)" )
M4RHR_SET( 199?, m4rhr__2,	m4rhr,	"crus.p1",			0x0000, 0x010000, CRC(bf2ff034) SHA1(7ee7ef30da4283dbb2b1b040fdd3313cb2e1b7e5), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 30)" )
M4RHR_SET( 199?, m4rhr__3,	m4rhr,	"cruy.p1",			0x0000, 0x010000, CRC(edf1346b) SHA1(c250178991885a922f676424e70c637e11089efb), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 31)" )
M4RHR_SET( 199?, m4rhr__4,	m4rhr,	"redhot8.bin",		0x0000, 0x010000, CRC(1dc62d7b) SHA1(640a5b29314a7dc67db271cce06c23c676d77eee), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 32)" )
M4RHR_SET( 199?, m4rhr__5,	m4rhr,	"rhr03.r",			0x0000, 0x010000, CRC(98d81b1e) SHA1(17ab0dced53be9755aada7954aff2dc2a6973190), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 33)" )
M4RHR_SET( 199?, m4rhr__6,	m4rhr,	"rhr10",			0x0000, 0x010000, CRC(2a18a033) SHA1(add907c5ab155c28142dcee57825059715afd80d), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 34)" )
M4RHR_SET( 199?, m4rhr__7,	m4rhr,	"rhr2015",			0x0000, 0x010000, CRC(dbfd3b95) SHA1(4fc7ae32f7d76be3d3d07d627391884bd4d6de09), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 35)" )
M4RHR_SET( 199?, m4rhr__8,	m4rhr,	"rhr2515",			0x0000, 0x010000, CRC(e4554c23) SHA1(6d977beb282fd638de3457e467e842ce79b5be7c), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 36)" )
M4RHR_SET( 199?, m4rhr__9,	m4rhr,	"rhr2pprg.bin",		0x0000, 0x010000, CRC(f97047b2) SHA1(d3ed8c93e405f9e7448b3924ff9aa84223b76046), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 37)" )
M4RHR_SET( 199?, m4rhr__aa,	m4rhr,	"rhrb.p1",			0x0000, 0x010000, CRC(876fbe46) SHA1(1c7faf68ddef2ccbb8e3cd2cf5c709a7a4f4daef), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 38)" )
M4RHR_SET( 199?, m4rhr__ab,	m4rhr,	"rhrbd.p1",			0x0000, 0x010000, CRC(f0fa0c7b) SHA1(96bfce8ea54e392a36cb8d82a032438bff992f07), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 39)" )
M4RHR_SET( 199?, m4rhr__ac,	m4rhr,	"rhrc.p1",			0x0000, 0x010000, CRC(76a0e556) SHA1(1a9bae286ca40d8e72022645d006a219f113e31a), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 40)" )
M4RHR_SET( 199?, m4rhr__ad,	m4rhr,	"rhrd.p1",			0x0000, 0x010000, CRC(58a5dd6f) SHA1(3646b8cb3d49e8c530e321daad052f27cdf4bb3d), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 41)" )
M4RHR_SET( 199?, m4rhr__ae,	m4rhr,	"rhrk.p1",			0x0000, 0x010000, CRC(2212cebb) SHA1(224e7e243b17f3ca90a6daa529984e9a879ff266), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 42)" )
M4RHR_SET( 199?, m4rhr__af,	m4rhr,	"rhrs.p1",			0x0000, 0x010000, CRC(a0e5d5b6) SHA1(c730e6319bbea6f035fb3e249991983783ef5743), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 43)" )
M4RHR_SET( 199?, m4rhr__ag,	m4rhr,	"rhtad.p1",			0x0000, 0x010000, CRC(ae3a31a0) SHA1(7e1f05a21cf5b3d2aceba755136c567b5d6ecfcd), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 44)" )
M4RHR_SET( 199?, m4rhr__ah,	m4rhr,	"rhtb.p1",			0x0000, 0x010000, CRC(7ceb13c8) SHA1(f0f22149bd0fb12ef06c4c3ecba605df33f52c51), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 45)" )
M4RHR_SET( 199?, m4rhr__ai,	m4rhr,	"rhtbd.p1",			0x0000, 0x010000, CRC(e4b290fc) SHA1(bf16d06429d67936118264f6c4f1ae637753d5db), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 46)" )
M4RHR_SET( 199?, m4rhr__aj,	m4rhr,	"rhtd.p1",			0x0000, 0x010000, CRC(a08d508c) SHA1(10efbfb4fc4820313b410ec73f9c32ed048e2228), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 47)" )
M4RHR_SET( 199?, m4rhr__ak,	m4rhr,	"rhtdk.p1",			0x0000, 0x010000, CRC(6495681a) SHA1(afd3451402e19c4c4bb8507447d6771323219e80), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 48)" )
M4RHR_SET( 199?, m4rhr__al,	m4rhr,	"rhtdr.p1",			0x0000, 0x010000, CRC(df9e5c83) SHA1(88586852c0773de4ee1b4c627eabf3de27e5c2a1), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 49)" )
M4RHR_SET( 199?, m4rhr__am,	m4rhr,	"rhtdy.p1",			0x0000, 0x010000, CRC(42f5746d) SHA1(964bd8801b44de9ea45c43b290b1cd6284e97578), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 50)" )
M4RHR_SET( 199?, m4rhr__an,	m4rhr,	"rhtk.p1",			0x0000, 0x010000, CRC(c3bfb174) SHA1(2579bf17252988de17a1367546ae187420f95cc5), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 51)" )
M4RHR_SET( 199?, m4rhr__ao,	m4rhr,	"rhtr.p1",			0x0000, 0x010000, CRC(f53f4876) SHA1(feda495361d384c662554d445a95191a2c52a56a), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 52)" )
M4RHR_SET( 199?, m4rhr__ap,	m4rhr,	"rhts.p1",			0x0000, 0x010000, CRC(fecb7076) SHA1(43086c6bfd878d0ca1ec8d45285d3e941a62ac8e), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 53)" )
M4RHR_SET( 199?, m4rhr__aq,	m4rhr,	"rhty.p1",			0x0000, 0x010000, CRC(68546098) SHA1(57981c06efcb44915d8c2d4b6e1cba377c4a8590), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 54)" )
M4RHR_SET( 199?, m4rhr__ar,	m4rhr,	"rhuad.p1",			0x0000, 0x010000, CRC(2093126b) SHA1(942994793697cec730c461c87b24a1429e46cc02), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 55)" )
M4RHR_SET( 199?, m4rhr__as,	m4rhr,	"rhub.p1",			0x0000, 0x010000, CRC(2be41a3a) SHA1(a50c7b5b93a619e541be480646517e278da8e579), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 56)" )
M4RHR_SET( 199?, m4rhr__at,	m4rhr,	"rhubd.p1",			0x0000, 0x010000, CRC(168f7f21) SHA1(9c9e09673bdadd146883a06a8db3c0ee4b304eab), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 57)" )
M4RHR_SET( 199?, m4rhr__au,	m4rhr,	"rhud.p1",			0x0000, 0x010000, CRC(71932d29) SHA1(e92af5cced251eea2e31c4c1968e77087c64b824), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 58)" )
M4RHR_SET( 199?, m4rhr__av,	m4rhr,	"rhudk.p1",			0x0000, 0x010000, CRC(8de54a5d) SHA1(a275d8c67d38c09f19ffa41e97fbcbea3d297aa4), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 59)" )
M4RHR_SET( 199?, m4rhr__aw,	m4rhr,	"rhudr.p1",			0x0000, 0x010000, CRC(ba01ac84) SHA1(d03b3b321abd220f619724e99cc396c38418f2d3), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 60)" )
M4RHR_SET( 199?, m4rhr__ax,	m4rhr,	"rhudy.p1",			0x0000, 0x010000, CRC(692bf4eb) SHA1(136f36073f236b48442a20e06aa51a978135f1b3), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 61)" )
M4RHR_SET( 199?, m4rhr__ay,	m4rhr,	"rhuk.p1",			0x0000, 0x010000, CRC(9e4e1e91) SHA1(f671858c41dc0e55189e9a86fff1846938b5c2e5), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 62)" )
M4RHR_SET( 199?, m4rhr__az,	m4rhr,	"rhur.p1",			0x0000, 0x010000, CRC(6e9425e5) SHA1(1e2827f3469af15e8d390d9af839c7b474ea95a7), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 63)" )
M4RHR_SET( 199?, m4rhr__a0,	m4rhr,	"rhus.p1",			0x0000, 0x010000, CRC(31e776fc) SHA1(e51799e9db5a08cbfb0b6c5466a0a085c3d91db4), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 64)" )
M4RHR_SET( 199?, m4rhr__a1,	m4rhr,	"rhuy.p1",			0x0000, 0x010000, CRC(5d12178a) SHA1(18525828fac1931bb8e11f96b79db143ed533771), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 65)" )
M4RHR_SET( 199?, m4rhr__a2,	m4rhr,	"cr__x__x.5_0",		0x0000, 0x010000, CRC(278fe91e) SHA1(dcfed3a7796d1ee365e535115b66c7d6cbe0ab74), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 66)" )
M4RHR_SET( 199?, m4rhr__a3,	m4rhr,	"cr__x_dx.2_0",		0x0000, 0x010000, CRC(73fb120c) SHA1(4c0f39253dee9b528763a9cb609dec31e8529713), "Barcrest","Red Hot Roll (Barcrest) (MPU4) (set 67)" )


#define M4UUAW_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "uuasnd.p1", 0x00000, 0x080000, CRC(be1a1131) SHA1(b7f50d8db6b7d134757e0746e7d9faf9fd3a2c7e) ) \
	ROM_LOAD( "uuasnd.p2", 0x080000, 0x080000, CRC(c8492b3a) SHA1(d390e37f4a62869079bb38395a2e86a5ad24392f) ) \

#define M4UUAW_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4UUAW_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \



M4UUAW_SET( 199?, m4uuaw,		0,		"uua21h.p1",	0x0000, 0x020000, CRC(199e6dae) SHA1(ecd95ba2c2255afbaa8df96d625a8bfc97e4d3bc), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 1)" )
M4UUAW_SET( 199?, m4uuaw__a,	m4uuaw,	"upa15g",		0x0000, 0x020000, CRC(d20b8b92) SHA1(6fcddc781c204dfd34de2c4e4ce0ec35fb3ec4e0), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 2)" )
M4UUAW_SET( 199?, m4uuaw__b,	m4uuaw,	"upa15t",		0x0000, 0x020000, CRC(85e3e82a) SHA1(e90183fab082f159d76ea14da794d52ee6ab8200), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 3)" )
M4UUAW_SET( 199?, m4uuaw__c,	m4uuaw,	"ups21ad.p1",	0x0000, 0x020000, CRC(c19fa891) SHA1(c2772ec20a65ce999d901e8c873ec687113b18d4), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 4)" )
M4UUAW_SET( 199?, m4uuaw__d,	m4uuaw,	"ups21b.p1",	0x0000, 0x020000, CRC(01320407) SHA1(a3273c59733e42013c3448b2a5c7c575ec0182b9), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 5)" )
M4UUAW_SET( 199?, m4uuaw__e,	m4uuaw,	"ups21bd.p1",	0x0000, 0x020000, CRC(2fa3cb8a) SHA1(8df994ce93fc6f0df27a6ee73676d9ee73593091), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 6)" )
M4UUAW_SET( 199?, m4uuaw__f,	m4uuaw,	"ups21d.p1",	0x0000, 0x020000, CRC(2a8db1a6) SHA1(873ab3757920c9153c1542748a74b36ce5e190c2), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 7)" )
M4UUAW_SET( 199?, m4uuaw__g,	m4uuaw,	"ups21dh.p1",	0x0000, 0x020000, CRC(7bcfbb46) SHA1(b93dfa71e3ec0ea96eaf2db4cd382b0a2852a1ff), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 8)" )
M4UUAW_SET( 199?, m4uuaw__h,	m4uuaw,	"ups21dk.p1",	0x0000, 0x020000, CRC(0642ae02) SHA1(8898341f8dc4f4c8c45ce6d04a01bd919cb0548a), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 9)" )
M4UUAW_SET( 199?, m4uuaw__i,	m4uuaw,	"ups21dr.p1",	0x0000, 0x020000, CRC(b54d1533) SHA1(1f9220342dcab675b04895f69a7ca75579ba729f), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 10)" )
M4UUAW_SET( 199?, m4uuaw__j,	m4uuaw,	"ups21dy.p1",	0x0000, 0x020000, CRC(4c850654) SHA1(21f386060301adef646fe469c1fcfb002d3e3424), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 11)" )
M4UUAW_SET( 199?, m4uuaw__k,	m4uuaw,	"ups21h.p1",	0x0000, 0x020000, CRC(555e74cb) SHA1(14246b54839eb334576a119d7c87901f3b2f25ad), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 12)" )
M4UUAW_SET( 199?, m4uuaw__l,	m4uuaw,	"ups21k.p1",	0x0000, 0x020000, CRC(28d3618f) SHA1(186337119e4b663dadc129533ce8a913013390a9), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 13)" )
M4UUAW_SET( 199?, m4uuaw__m,	m4uuaw,	"ups21r.p1",	0x0000, 0x020000, CRC(9bdcdabe) SHA1(db0bb90705abec92a220a3dbe0ea69266d5e0558), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 14)" )
M4UUAW_SET( 199?, m4uuaw__n,	m4uuaw,	"ups21s.p1",	0x0000, 0x020000, CRC(c4a8a542) SHA1(61063d55c6017cf17d704df576cb62da5bd75820), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 15)" )
M4UUAW_SET( 199?, m4uuaw__o,	m4uuaw,	"ups21y.p1",	0x0000, 0x020000, CRC(6214c9d9) SHA1(d25fecc9798e342207d358a54efad1908c0e2247), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 16)" )
M4UUAW_SET( 199?, m4uuaw__p,	m4uuaw,	"ups22ad.p1",	0x0000, 0x020000, CRC(ee0f53a6) SHA1(eabe58efa82015eb2266a793853e8ade546d6da1), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 17)" )
M4UUAW_SET( 199?, m4uuaw__q,	m4uuaw,	"ups22b.p1",	0x0000, 0x020000, CRC(e7dbf5ae) SHA1(0fbbc3da1af8b60993a7f6082bd5e96da21cd0b8), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 18)" )
M4UUAW_SET( 199?, m4uuaw__r,	m4uuaw,	"ups22bd.p1",	0x0000, 0x020000, CRC(003330bd) SHA1(42ad6ddfd7639909151dcee5e40e82a23074fd59), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 19)" )
M4UUAW_SET( 199?, m4uuaw__s,	m4uuaw,	"ups22d.p1",	0x0000, 0x020000, CRC(cc64400f) SHA1(0ff7858c637fbb43a7cd1313bbf046177e4b7761), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 20)" )
M4UUAW_SET( 199?, m4uuaw__t,	m4uuaw,	"ups22dh.p1",	0x0000, 0x020000, CRC(545f4071) SHA1(3947499d78d31fb0b269a63a518790b503a97685), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 21)" )
M4UUAW_SET( 199?, m4uuaw__u,	m4uuaw,	"ups22dk.p1",	0x0000, 0x020000, CRC(29d25535) SHA1(7f053741d12cce467dd437ea998064e13d1ca52b), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 22)" )
M4UUAW_SET( 199?, m4uuaw__v,	m4uuaw,	"ups22dr.p1",	0x0000, 0x020000, CRC(9addee04) SHA1(45c15536c8846da825a994a667b6e46598c1642e), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 23)" )
M4UUAW_SET( 199?, m4uuaw__w,	m4uuaw,	"ups22dy.p1",	0x0000, 0x020000, CRC(6315fd63) SHA1(9a5fcab51d4e94b96669149285dda28cd41020b8), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 24)" )
M4UUAW_SET( 199?, m4uuaw__x,	m4uuaw,	"ups22h.p1",	0x0000, 0x020000, CRC(b3b78562) SHA1(3e75fa20156faa3d38c2b5ac824bffe47e72b7bc), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 25)" )
M4UUAW_SET( 199?, m4uuaw__y,	m4uuaw,	"ups22k.p1",	0x0000, 0x020000, CRC(ce3a9026) SHA1(80977176c5bae809a564f4fc0e3d6370f91f829b), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 26)" )
M4UUAW_SET( 199?, m4uuaw__z,	m4uuaw,	"ups22r.p1",	0x0000, 0x020000, CRC(7d352b17) SHA1(d2d1d016a587be318e9018eb1953e68fe83620df), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 27)" )
M4UUAW_SET( 199?, m4uuaw__0,	m4uuaw,	"ups22s.p1",	0x0000, 0x020000, CRC(ac990aa9) SHA1(396c9eded9c18ab2bcb0f4066a890f6e239830f1), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 28)" )
M4UUAW_SET( 199?, m4uuaw__1,	m4uuaw,	"ups22y.p1",	0x0000, 0x020000, CRC(84fd3870) SHA1(8d294ae1a92d1e99c4c3f17a2d77fe1d994b2c33), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 29)" )
M4UUAW_SET( 199?, m4uuaw__2,	m4uuaw,	"uua21ad.p1",	0x0000, 0x020000, CRC(2a18c292) SHA1(5853cb069eb5caa23372e5dedd33868103125780), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 30)" )
M4UUAW_SET( 199?, m4uuaw__3,	m4uuaw,	"uua21b.p1",	0x0000, 0x020000, CRC(d71cc3db) SHA1(7d783110341237769165a08fd86f597225f8d90c), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 31)" )
M4UUAW_SET( 199?, m4uuaw__4,	m4uuaw,	"uua21bd.p1",	0x0000, 0x020000, CRC(f04323be) SHA1(194c996d4c8e2fed2bcb02e29423e68b53900a1f), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 32)" )
M4UUAW_SET( 199?, m4uuaw__5,	m4uuaw,	"uua21d.p1",	0x0000, 0x020000, CRC(664da8c3) SHA1(c93b0b5e796fcde0850e2b3054f1db0417f4e9ed), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 33)" )
M4UUAW_SET( 199?, m4uuaw__6,	m4uuaw,	"uua21dh.p1",	0x0000, 0x020000, CRC(3ec18dcb) SHA1(fb5fe8ba0b59a21401cf091f43ad9f2a4df3447c), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 34)" )
M4UUAW_SET( 199?, m4uuaw__7,	m4uuaw,	"uua21dk.p1",	0x0000, 0x020000, CRC(84919e1c) SHA1(c7315a3d1985180ec5ae1f4e5c7f0c99c1e0bac4), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 35)" )
M4UUAW_SET( 199?, m4uuaw__8,	m4uuaw,	"uua21dr.p1",	0x0000, 0x020000, CRC(434c988f) SHA1(eb6126048df5bc4ff98d9838a3bb6cf24a9ab895), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 36)" )
M4UUAW_SET( 199?, m4uuaw__9,	m4uuaw,	"uua21dy.p1",	0x0000, 0x020000, CRC(098b30d9) SHA1(48daa77f3aafdcd52d7291cdda533e8a9428de0e), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 37)" )
M4UUAW_SET( 199?, m4uuaw__aa,	m4uuaw,	"uua21k.p1",	0x0000, 0x020000, CRC(a3ce7e79) SHA1(8670d2cb7281ccabc15c5288a3e0dd99cfc1ae36), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 38)" )
M4UUAW_SET( 199?, m4uuaw__ab,	m4uuaw,	"uua21r.p1",	0x0000, 0x020000, CRC(641378ea) SHA1(de0282af6a17c7fc16c7eca10e81ffb208675779), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 39)" )
M4UUAW_SET( 199?, m4uuaw__ac,	m4uuaw,	"uua21s.p1",	0x0000, 0x020000, CRC(27c46fcc) SHA1(68a03fcce5d8155d6c0115d813c17217c4120375), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 40)" )
M4UUAW_SET( 199?, m4uuaw__ad,	m4uuaw,	"uua21y.p1",	0x0000, 0x020000, CRC(2ed4d0bc) SHA1(ffb0585e729b389855d24015e1ef7582eab88d3e), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 41)" )
M4UUAW_SET( 199?, m4uuaw__ae,	m4uuaw,	"uua22ad.p1",	0x0000, 0x020000, CRC(b2ace4d5) SHA1(da02abe111fea3fbfb9495e9b447139cd67a61e0), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 42)" )
M4UUAW_SET( 199?, m4uuaw__af,	m4uuaw,	"uua22b.p1",	0x0000, 0x020000, CRC(71a6374a) SHA1(c14ed22fceb83b5ac72021322c9b8bb3d5afeffb), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 43)" )
M4UUAW_SET( 199?, m4uuaw__ag,	m4uuaw,	"uua22bd.p1",	0x0000, 0x020000, CRC(68f705f9) SHA1(678ba97241f3dede96239265eed418d4717637a6), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 44)" )
M4UUAW_SET( 199?, m4uuaw__ah,	m4uuaw,	"uua22d.p1",	0x0000, 0x020000, CRC(c0f75c52) SHA1(a4e1e496b0cbb24767f017fbe228fbb8ab2bb907), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 45)" )
M4UUAW_SET( 199?, m4uuaw__ai,	m4uuaw,	"uua22dh.p1",	0x0000, 0x020000, CRC(a675ab8c) SHA1(37fb437b95f9ff50fe41ebce825e3dd1b361925e), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 46)" )
M4UUAW_SET( 199?, m4uuaw__aj,	m4uuaw,	"uua22dk.p1",	0x0000, 0x020000, CRC(1c25b85b) SHA1(b42cdae2e2c00644376eb5f0c5b7567d3811b162), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 47)" )
M4UUAW_SET( 199?, m4uuaw__ak,	m4uuaw,	"uua22dr.p1",	0x0000, 0x020000, CRC(dbf8bec8) SHA1(b3ac5ed5b8cbc0457e5dfadefcab563e3197b045), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 48)" )
M4UUAW_SET( 199?, m4uuaw__al,	m4uuaw,	"uua22dy.p1",	0x0000, 0x020000, CRC(913f169e) SHA1(9f82f5d868a9be046ced838f8b53730140ed50b2), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 49)" )
M4UUAW_SET( 199?, m4uuaw__am,	m4uuaw,	"uua22h.p1",	0x0000, 0x020000, CRC(bf24993f) SHA1(618d6d2f2b762d61eb58087a3597ffb709658631), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 50)" )
M4UUAW_SET( 199?, m4uuaw__an,	m4uuaw,	"uua22k.p1",	0x0000, 0x020000, CRC(05748ae8) SHA1(d9aeee26c8471bb6ee58a4a838e5c9930da92725), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 51)" )
M4UUAW_SET( 199?, m4uuaw__ao,	m4uuaw,	"uua22r.p1",	0x0000, 0x020000, CRC(c2a98c7b) SHA1(115f7c7a4b9eab5f3270f43a2db7a320dfc4e223), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 52)" )
M4UUAW_SET( 199?, m4uuaw__ap,	m4uuaw,	"uua22s.p1",	0x0000, 0x020000, CRC(65f57c0c) SHA1(7b2526cdd1ec973a91bc7ade116e16e03b32596a), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 53)" )
M4UUAW_SET( 199?, m4uuaw__aq,	m4uuaw,	"uua22y.p1",	0x0000, 0x020000, CRC(886e242d) SHA1(4b49b70fc2635fcf7b538b35b42a358cf4dd60b3), "Barcrest","Up Up and Away (Barcrest) (MPU4) (set 54)" )


#define M4RICHFM_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "rfamouss1.hex", 0x000000, 0x080000, CRC(b237c8b8) SHA1(b2322d68fe57cca0ed49b01ae0d3a0e93a623eac) ) \
	ROM_LOAD( "rfamouss2.hex", 0x080000, 0x080000, CRC(12c295d5) SHA1(0758354cfb5242b4ce3f5f25c3458d91f4b4a1ec) ) \

#define M4RICHFM_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RICHFM_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4RICHFM_SET( 199?, m4richfm,		0,			"rfts.p1",		0x0000, 0x010000, CRC(2a747164) SHA1(a4c8e160f09ebea4fca6dd32ff020d3f1a4f1a1c), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 1)" )
M4RICHFM_SET( 199?, m4richfm__a,	m4richfm,	"rafc.p1",		0x0000, 0x010000, CRC(d92f602f) SHA1(c93131138deb4018d499b9b45c07d4517c5072b7), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 2)" )
M4RICHFM_SET( 199?, m4richfm__b,	m4richfm,	"rafd.p1",		0x0000, 0x010000, CRC(b0e9f470) SHA1(cad080a5d7f24968524fe10f6c43b088f35d7364), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 3)" )
M4RICHFM_SET( 199?, m4richfm__c,	m4richfm,	"rafs.p1",		0x0000, 0x010000, CRC(f312b2e3) SHA1(8bf2cb7b73cfc320143d05d25e28c15fb4f26045), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 4)" )
M4RICHFM_SET( 199?, m4richfm__d,	m4richfm,	"rafy.p1",		0x0000, 0x010000, CRC(a8812d45) SHA1(c0b89833f87ed90eb3e9c3299fcea362d501ed90), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 5)" )
M4RICHFM_SET( 199?, m4richfm__e,	m4richfm,	"rchfam8",		0x0000, 0x004000, CRC(55f16698) SHA1(9853b17bbb81371192a564376be7b3074908dbca), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 6)" )
M4RICHFM_SET( 199?, m4richfm__f,	m4richfm,	"rf5ad.p1",		0x0000, 0x010000, CRC(cd280292) SHA1(605d89608e106979229a00701a2e5b578df50d60), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 7)" )
M4RICHFM_SET( 199?, m4richfm__g,	m4richfm,	"rf5b.p1",		0x0000, 0x010000, CRC(e1edf753) SHA1(677f0397ec57422241f4669be610cffd33a9b44a), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 8)" )
M4RICHFM_SET( 199?, m4richfm__h,	m4richfm,	"rf5bd.p1",		0x0000, 0x010000, CRC(2d698365) SHA1(7f91cee0d34550aba9ac0f4ee398df4de6fd6f7e), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 9)" )
M4RICHFM_SET( 199?, m4richfm__i,	m4richfm,	"rf5d.p1",		0x0000, 0x010000, CRC(034cab0b) SHA1(79eaeb84377dbb8e6bda1dd2ae29a1f79656b9e4), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 10)" )
M4RICHFM_SET( 199?, m4richfm__j,	m4richfm,	"rf5dk.p1",		0x0000, 0x010000, CRC(14fc0f13) SHA1(a2b294da18c3f5bc9c81eb3f3af5ab5ca58c9cad), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 11)" )
M4RICHFM_SET( 199?, m4richfm__k,	m4richfm,	"rf5dy.p1",		0x0000, 0x010000, CRC(a2664c64) SHA1(2256b6e0d6472faa901348cb5be849ad012f1d16), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 12)" )
M4RICHFM_SET( 199?, m4richfm__l,	m4richfm,	"rf5k.p1",		0x0000, 0x010000, CRC(d8787b25) SHA1(885ac7ddd3de4cb475539d02aefbf38fed7c1f2c), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 13)" )
M4RICHFM_SET( 199?, m4richfm__m,	m4richfm,	"rf5s.p1",		0x0000, 0x010000, CRC(8d1ed193) SHA1(a4ca973dac8a8fd550bf7e57a8cdc627c28da4b8), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 14)" )
M4RICHFM_SET( 199?, m4richfm__n,	m4richfm,	"rf5y.p1",		0x0000, 0x010000, CRC(ad288548) SHA1(a7222ab5bffe8e5e0844f8e6f13e09afe74b08a8), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 15)" )
M4RICHFM_SET( 199?, m4richfm__o,	m4richfm,	"rf8b.p1",		0x0000, 0x010000, CRC(105c24e1) SHA1(cb417976a74441bf2ca888198b57fed81d758c15), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 16)" )
M4RICHFM_SET( 199?, m4richfm__p,	m4richfm,	"rf8c.p1",		0x0000, 0x010000, CRC(8924a706) SHA1(abb1a1f6cdeb15884dfa63fc04882f794453d4ec), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 17)" )
M4RICHFM_SET( 199?, m4richfm__q,	m4richfm,	"rft20.10",		0x0000, 0x010000, CRC(41e6ef75) SHA1(d836fdea5a89b845687d2ff929365bd81737c760), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 18)" )
M4RICHFM_SET( 199?, m4richfm__r,	m4richfm,	"rftad.p1",		0x0000, 0x010000, CRC(8553386f) SHA1(ad834d52e51c7f375a370dc6d8586668921a9795), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 19)" )
M4RICHFM_SET( 199?, m4richfm__s,	m4richfm,	"rftb.p1",		0x0000, 0x010000, CRC(0189cc2f) SHA1(62ccc85c50c56aa2e0bcbb42b5c24d402f00d366), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 20)" )
M4RICHFM_SET( 199?, m4richfm__t,	m4richfm,	"rftbd.p1",		0x0000, 0x010000, CRC(08351e03) SHA1(d08d38d46793828b147ccde8121fbb9bf422cd60), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 21)" )
M4RICHFM_SET( 199?, m4richfm__u,	m4richfm,	"rftd.p1",		0x0000, 0x010000, CRC(689f02ed) SHA1(1a30aac5454b0c477a698e9c573fe313bc1fe858), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 22)" )
M4RICHFM_SET( 199?, m4richfm__v,	m4richfm,	"rftdk.p1",		0x0000, 0x010000, CRC(098b88f5) SHA1(4559b561380055c429a5b4741326f64ad89d8481), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 23)" )
M4RICHFM_SET( 199?, m4richfm__w,	m4richfm,	"rftdy.p1",		0x0000, 0x010000, CRC(26b912f8) SHA1(1719d63b4a25293199b0729235beb5b93c484490), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 24)" )
M4RICHFM_SET( 199?, m4richfm__x,	m4richfm,	"rftk.p1",		0x0000, 0x010000, CRC(6a48bd98) SHA1(2f17194869ca008f7a2eb622bd3725bc91950a17), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 25)" )
M4RICHFM_SET( 199?, m4richfm__y,	m4richfm,	"rfty.p1",		0x0000, 0x010000, CRC(723fe46e) SHA1(51bb8aff358d527483eaf1b1e20606d94a937dc6), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 26)" )
M4RICHFM_SET( 199?, m4richfm__z,	m4richfm,	"rich2010",		0x0000, 0x010000, CRC(baecbdbc) SHA1(5fffecf3c91e832d3cfc13dbf5e6b74fc3d6a146), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 27)" )
M4RICHFM_SET( 199?, m4richfm__0,	m4richfm,	"r&f5.10",		0x0000, 0x010000, CRC(45d493d0) SHA1(9a549821a005fa65c2eb8b35c5f15659bd897519), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 28)" )
M4RICHFM_SET( 199?, m4richfm__1,	m4richfm,	"r&f5.4",		0x0000, 0x010000, CRC(0441d833) SHA1(361910fd64bc7291f6200fe354c468d16e7d6c80), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 29)" )
M4RICHFM_SET( 199?, m4richfm__2,	m4richfm,	"r&f5.8t",		0x0000, 0x010000, CRC(525e2520) SHA1(84b2ff86d6a54ebb3bcf0138930b2619a8733161), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 30)" )
M4RICHFM_SET( 199?, m4richfm__3,	m4richfm,	"r&f55",		0x0000, 0x010000, CRC(6095a72b) SHA1(af25f7c2fb5241064ea995d35fe4fd2f242e3750), "Barcrest","Rich & Famous (Barcrest) (MPU4) (set 31)" )


#define M4NNWWC_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing, maybe the same as NNWW */ \

#define M4NNWWC_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4NNWWC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \

M4NNWWC_SET( 199?, m4nnwwc,		0,			"cn302c.p1",		0x0000, 0x010000, CRC(fd9de050) SHA1(14c80deba1396aa5be0a1d02964ecd4b946f2ee8), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 1)" )
M4NNWWC_SET( 199?, m4nnwwc__a,	m4nnwwc,	"cf302ad.p1",		0x0000, 0x010000, CRC(6c6aa0cd) SHA1(5a58a19c35b0b195f3b4e7a21f57ca61d45ec1fb), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 2)" )
M4NNWWC_SET( 199?, m4nnwwc__b,	m4nnwwc,	"cf302b.p1",		0x0000, 0x010000, CRC(9ca07939) SHA1(6eb0a5675bb803a11c4c874dc0516d94c48194b7), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 3)" )
M4NNWWC_SET( 199?, m4nnwwc__c,	m4nnwwc,	"cf302bd.p1",		0x0000, 0x010000, CRC(8ba33b7d) SHA1(ebfb62a390de512dc1482cfb9ab64196cbcc5831), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 4)" )
M4NNWWC_SET( 199?, m4nnwwc__d,	m4nnwwc,	"cf302c.p1",		0x0000, 0x010000, CRC(26be2dc4) SHA1(157ca96ebd36f2fbfb501945d0351cc3be38f3b7), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 5)" )
M4NNWWC_SET( 199?, m4nnwwc__e,	m4nnwwc,	"cf302d.p1",		0x0000, 0x010000, CRC(b52d5b47) SHA1(1583963b0bac1288bd20ed0550ad793be0980b03), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 6)" )
M4NNWWC_SET( 199?, m4nnwwc__f,	m4nnwwc,	"cf302dk.p1",		0x0000, 0x010000, CRC(c3d4c74d) SHA1(9a34c1f2fabb20da17988f63c9190ec4dd0b65fb), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 7)" )
M4NNWWC_SET( 199?, m4nnwwc__g,	m4nnwwc,	"cf302dr.p1",		0x0000, 0x010000, CRC(0b25e6b9) SHA1(5fd42abbe985dbdcfe09da50673551330dd26175), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 8)" )
M4NNWWC_SET( 199?, m4nnwwc__h,	m4nnwwc,	"cf302dy.p1",		0x0000, 0x010000, CRC(420b47c1) SHA1(0cb1a843cec3ace21d806fe98212250201a72f12), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 9)" )
M4NNWWC_SET( 199?, m4nnwwc__i,	m4nnwwc,	"cf302k.p1",		0x0000, 0x010000, CRC(07ca4c45) SHA1(8f6ee3c17527b05a6652845019919d490cc00c64), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 10)" )
M4NNWWC_SET( 199?, m4nnwwc__j,	m4nnwwc,	"cf302r.p1",		0x0000, 0x010000, CRC(e09f43bd) SHA1(65dcdf8d223936c4415ddc3f734b83367d6b8db7), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 11)" )
M4NNWWC_SET( 199?, m4nnwwc__k,	m4nnwwc,	"cf302s.p1",		0x0000, 0x010000, CRC(7a3e8ead) SHA1(590dc78b98f9928d6fa87ef661234f88dccfdff8), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 12)" )
M4NNWWC_SET( 199?, m4nnwwc__l,	m4nnwwc,	"cf302y.p1",		0x0000, 0x010000, CRC(c1063a32) SHA1(e1c8fc463b1a1db87110f272a8727435f9d9b97a), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 13)" )
M4NNWWC_SET( 199?, m4nnwwc__m,	m4nnwwc,	"ch302ad.p1",		0x0000, 0x010000, CRC(20405f4e) SHA1(7f87c881f428f704c98b0f4be459980062ccd29a), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 14)" )
M4NNWWC_SET( 199?, m4nnwwc__n,	m4nnwwc,	"ch302b.p1",		0x0000, 0x010000, CRC(cf7543ac) SHA1(2fe810741bfc18f800ad8028724218557d93a830), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 15)" )
M4NNWWC_SET( 199?, m4nnwwc__o,	m4nnwwc,	"ch302bd.p1",		0x0000, 0x010000, CRC(4c3e5664) SHA1(87a1f2133cad624683dac89f1da85d70b018f846), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 16)" )
M4NNWWC_SET( 199?, m4nnwwc__p,	m4nnwwc,	"ch302c.p1",		0x0000, 0x010000, CRC(dcde4d0a) SHA1(d1535f8754d2c0f8183c2c9db97edafdcdfed82e), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 17)" )
M4NNWWC_SET( 199?, m4nnwwc__q,	m4nnwwc,	"ch302d.p1",		0x0000, 0x010000, CRC(e1a02108) SHA1(fa8271a1246a3ae1289bb314494743cfec31f4e2), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 18)" )
M4NNWWC_SET( 199?, m4nnwwc__r,	m4nnwwc,	"ch302dk.p1",		0x0000, 0x010000, CRC(a3f636af) SHA1(c3de325ef5baa3cccd4c9997e615e87521b9e537), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 19)" )
M4NNWWC_SET( 199?, m4nnwwc__s,	m4nnwwc,	"ch302dr.p1",		0x0000, 0x010000, CRC(0620b0c0) SHA1(31aabba5f5b096254908221f884b5088a5a6e883), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 20)" )
M4NNWWC_SET( 199?, m4nnwwc__t,	m4nnwwc,	"ch302dy.p1",		0x0000, 0x010000, CRC(9b4b982e) SHA1(c7c9c501eb1c936ffb8bc2fe1fe9258e92b1d548), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 21)" )
M4NNWWC_SET( 199?, m4nnwwc__u,	m4nnwwc,	"ch302k.p1",		0x0000, 0x010000, CRC(908d8b10) SHA1(a80a5ce1a83d05f1e68e66d14bacc424bc833aa7), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 22)" )
M4NNWWC_SET( 199?, m4nnwwc__v,	m4nnwwc,	"ch302r.p1",		0x0000, 0x010000, CRC(c31c4c28) SHA1(e94c7588211044dae7c5ac587e6232b0ace2fc7b), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 23)" )
M4NNWWC_SET( 199?, m4nnwwc__w,	m4nnwwc,	"ch302s.p1",		0x0000, 0x010000, CRC(e7d0ceb2) SHA1(b75d58136b9e1e4bfde86730ef4e95bc98494813), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 24)" )
M4NNWWC_SET( 199?, m4nnwwc__x,	m4nnwwc,	"ch302y.p1",		0x0000, 0x010000, CRC(5e7764c6) SHA1(05a61a57ac906cbea1d72fffd1c8ea707852b895), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 25)" )
M4NNWWC_SET( 199?, m4nnwwc__y,	m4nnwwc,	"cn302ad.p1",		0x0000, 0x010000, CRC(7a6acd9b) SHA1(9a1f0ed19d66428c6b541ce1c8e169d9b4be3ef1), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 26)" )
M4NNWWC_SET( 199?, m4nnwwc__z,	m4nnwwc,	"cn302b.p1",		0x0000, 0x010000, CRC(b69cb520) SHA1(7313f2740960ca86ecea8609fe8fd58d84a3248c), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 27)" )
M4NNWWC_SET( 199?, m4nnwwc__0,	m4nnwwc,	"cn302bd.p1",		0x0000, 0x010000, CRC(ab828a0b) SHA1(53fa6dad9bdae1d46479596c98cf2c3f4454bb95), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 28)" )
M4NNWWC_SET( 199?, m4nnwwc__1,	m4nnwwc,	"cn302d.p1",		0x0000, 0x010000, CRC(8c6ac365) SHA1(a32b104968aaa4da060072a241a4c54fbdf3c404), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 29)" )
M4NNWWC_SET( 199?, m4nnwwc__2,	m4nnwwc,	"cn302dk.p1",		0x0000, 0x010000, CRC(24cbab96) SHA1(77fe3b21fc9470653bada31c700ce926d55ce82e), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 30)" )
M4NNWWC_SET( 199?, m4nnwwc__3,	m4nnwwc,	"cn302dr.p1",		0x0000, 0x010000, CRC(09069f0e) SHA1(68b2a34644ee1fca3ce5191e2f25aa808b85fb09), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 31)" )
M4NNWWC_SET( 199?, m4nnwwc__4,	m4nnwwc,	"cn302dy.p1",		0x0000, 0x010000, CRC(946db7e0) SHA1(fe29c1da478e3f1a53ad55c661ddcc7003679304), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 32)" )
M4NNWWC_SET( 199?, m4nnwwc__5,	m4nnwwc,	"cn302k.p1",		0x0000, 0x010000, CRC(7a3202f1) SHA1(2dd5e8195120b1efc3eb51214cf054432fc50aed), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 33)" )
M4NNWWC_SET( 199?, m4nnwwc__6,	m4nnwwc,	"cn302r.p1",		0x0000, 0x010000, CRC(e7cf9e1e) SHA1(66a1e54fc928c09d16f7ac1c002685eee841315f), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 34)" )
M4NNWWC_SET( 199?, m4nnwwc__7,	m4nnwwc,	"cn302s.p1",		0x0000, 0x010000, CRC(87703a1a) SHA1(6582ffa42a61b60e92e456a794c4c219a9901a1c), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 35)" )
M4NNWWC_SET( 199?, m4nnwwc__8,	m4nnwwc,	"cn302y.p1",		0x0000, 0x010000, CRC(7aa4b6f0) SHA1(2c185a9a7c8a4957fb5901305883661c41cb0cb4), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 36)" )
M4NNWWC_SET( 199?, m4nnwwc__9,	m4nnwwc,	"cnc03s.p1",		0x0000, 0x010000, CRC(57a03b29) SHA1(52cc8eb3f02c4a812de06ceec0588ca930e07876), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 37)" )
M4NNWWC_SET( 199?, m4nnwwc__aa,	m4nnwwc,	"cl__x__x.2_0",		0x0000, 0x010000, CRC(c3de4791) SHA1(220d32b961b6710d508c0c7e6b2d8e4d292746f4), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 38)" )
M4NNWWC_SET( 199?, m4nnwwc__ab,	m4nnwwc,	"cl__x_dx.2_0",		0x0000, 0x010000, CRC(c79833f8) SHA1(b3519b55f6f2a4f081b69483ac0b8860aa8190d9), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 39)" )
M4NNWWC_SET( 199?, m4nnwwc__ac,	m4nnwwc,	"cl__xa_x.2_0",		0x0000, 0x010000, CRC(4c3021a1) SHA1(7e7258808dd1693adb956a5e6b076f925eb0a026), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 40)" )
M4NNWWC_SET( 199?, m4nnwwc__ad,	m4nnwwc,	"cl__xb_x.2_0",		0x0000, 0x010000, CRC(75a5add7) SHA1(6802ec81b4ebcde9ed014a0440fdc50211a8a350), "Barcrest","Nudge Nudge Wink Wink Classic (Barcrest) (MPU4) (set 41)" )



#define M4RHRC_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	/* missing, or the same as m4rhr? */

#define M4RHRC_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4RHRC_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \



M4RHRC_SET( 199?, m4rhrc,		0,		"cld03ad.p1",		0x0000, 0x010000, CRC(821fde63) SHA1(61f77eeb01331e735cc8c736526d09371e6bdf56), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 1)" )
M4RHRC_SET( 199?, m4rhrc__a,	m4rhrc,	"cld03b.p1",		0x0000, 0x010000, CRC(c67a2e82) SHA1(b76110c73d5bd0290fdd31d8300914f63a56c25e), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 2)" )
M4RHRC_SET( 199?, m4rhrc__b,	m4rhrc,	"cld03bd.p1",		0x0000, 0x010000, CRC(0995fd93) SHA1(c3cc84f78adc54f4698280bf7d0831bb54c3fc3f), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 3)" )
M4RHRC_SET( 199?, m4rhrc__c,	m4rhrc,	"cld03c.p1",		0x0000, 0x010000, CRC(6e7b319f) SHA1(3da4feb72cb9d4ee24a8e0568f8d9c80a71caf9b), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 4)" )
M4RHRC_SET( 199?, m4rhrc__d,	m4rhrc,	"cld03d.p1",		0x0000, 0x010000, CRC(dc46afb0) SHA1(c461ac2ef3fcffac96536b1b1c26abe052edf35c), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 5)" )
M4RHRC_SET( 199?, m4rhrc__e,	m4rhrc,	"cld03dk.p1",		0x0000, 0x010000, CRC(f0b6b60f) SHA1(9addae6af20986c92c3ce71ce9756a6f3db5ebff), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 6)" )
M4RHRC_SET( 199?, m4rhrc__f,	m4rhrc,	"cld03dr.p1",		0x0000, 0x010000, CRC(703ab87b) SHA1(089597927f94bdacc4226900a944cbec85fe2286), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 7)" )
M4RHRC_SET( 199?, m4rhrc__g,	m4rhrc,	"cld03dy.p1",		0x0000, 0x010000, CRC(ed519095) SHA1(ac174166bf2cc6ab81f9782f1be4a9fbe226f34d), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 8)" )
M4RHRC_SET( 199?, m4rhrc__h,	m4rhrc,	"cld03k.p1",		0x0000, 0x010000, CRC(3bad05a9) SHA1(1b00ac52f6c87b5c79088b6fc3e6d00f57876ebc), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 9)" )
M4RHRC_SET( 199?, m4rhrc__i,	m4rhrc,	"cld03r.p1",		0x0000, 0x010000, CRC(2de70bdc) SHA1(d8d0170ca71fde4c79d0b465d09d4bb31acf40cf), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 10)" )
M4RHRC_SET( 199?, m4rhrc__j,	m4rhrc,	"cld03s.p1",		0x0000, 0x010000, CRC(03f8a6bf) SHA1(29ee59fd60d89fca0f236be8b4c12c885db032e7), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 11)" )
M4RHRC_SET( 199?, m4rhrc__k,	m4rhrc,	"cld03y.p1",		0x0000, 0x010000, CRC(b08c2332) SHA1(1cdf7fc0e95a50766df2d1cd51cb803b922c30c8), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 12)" )
M4RHRC_SET( 199?, m4rhrc__l,	m4rhrc,	"hhn03ad.p1",		0x0000, 0x010000, CRC(e7da568e) SHA1(00f9eecd06131bc5770a6ab650b3548f5b7a8c15), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 13)" )
M4RHRC_SET( 199?, m4rhrc__m,	m4rhrc,	"hhn03b.p1",		0x0000, 0x010000, CRC(406e47cd) SHA1(193aed33ac62eb04d89cf63beb33e8e4e28e286e), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 14)" )
M4RHRC_SET( 199?, m4rhrc__n,	m4rhrc,	"hhn03bd.p1",		0x0000, 0x010000, CRC(66aed369) SHA1(6c3151790292a277a1d44a1fceae985e52014749), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 15)" )
M4RHRC_SET( 199?, m4rhrc__o,	m4rhrc,	"hhn03c.p1",		0x0000, 0x010000, CRC(452e623c) SHA1(9350d7e30d8fc2b0f37528a7d0ce6797bab6f504), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 16)" )
M4RHRC_SET( 199?, m4rhrc__p,	m4rhrc,	"hhn03d.p1",		0x0000, 0x010000, CRC(e9ce4ee5) SHA1(45fe3832cc37e8ecbc5101b8b7b94f6243504e3f), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 17)" )
M4RHRC_SET( 199?, m4rhrc__q,	m4rhrc,	"hhn03dk.p1",		0x0000, 0x010000, CRC(2d750f34) SHA1(1672d5a8b4a338cac87281e1329f111f468dc611), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 18)" )
M4RHRC_SET( 199?, m4rhrc__r,	m4rhrc,	"hhn03dr.p1",		0x0000, 0x010000, CRC(88a3895b) SHA1(3e2dcf6728712620724774c16a5d84dbec9c5ab3), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 19)" )
M4RHRC_SET( 199?, m4rhrc__s,	m4rhrc,	"hhn03dy.p1",		0x0000, 0x010000, CRC(15c8a1b5) SHA1(5a2f28f290fa087b5010f778d4ad8d6c63a3d13e), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 20)" )
M4RHRC_SET( 199?, m4rhrc__t,	m4rhrc,	"hhn03k.p1",		0x0000, 0x010000, CRC(95450230) SHA1(3c1c239e84a89ef6acd44ac9c81d33021ac6b0e3), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 21)" )
M4RHRC_SET( 199?, m4rhrc__u,	m4rhrc,	"hhn03r.p1",		0x0000, 0x010000, CRC(d96d6825) SHA1(89c3f5494d97326369f10c982842310592456874), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 22)" )
M4RHRC_SET( 199?, m4rhrc__v,	m4rhrc,	"hhn03s.p1",		0x0000, 0x010000, CRC(b531ae78) SHA1(87d043541c23b88b8ec4067c67be77812095faaa), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 23)" )
M4RHRC_SET( 199?, m4rhrc__w,	m4rhrc,	"hhn03y.p1",		0x0000, 0x010000, CRC(440640cb) SHA1(de6b6edcdc99aaa0122ecd24a9a7437e6b44aad2), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 24)" )
M4RHRC_SET( 199?, m4rhrc__x,	m4rhrc,	"rrd03ad.p1",		0x0000, 0x010000, CRC(6f49d7d1) SHA1(2195a3ad4836e8ffd2e7e6a90e94319d5a5a0ce8), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 25)" )
M4RHRC_SET( 199?, m4rhrc__y,	m4rhrc,	"rrd03b.p1",		0x0000, 0x010000, CRC(e8447a3d) SHA1(8bf5936782e0fbec25a8ef892b8df04b6543bc74), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 26)" )
M4RHRC_SET( 199?, m4rhrc__z,	m4rhrc,	"rrd03bd.p1",		0x0000, 0x010000, CRC(52cf0357) SHA1(ab4668df6d5ad9614410aede7ad4e030283b78ca), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 27)" )
M4RHRC_SET( 199?, m4rhrc__0,	m4rhrc,	"rrd03c.p1",		0x0000, 0x010000, CRC(b03e7b76) SHA1(0b2779b584f8fa0e25e2a5248ecb8fb88aa53413), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 28)" )
M4RHRC_SET( 199?, m4rhrc__1,	m4rhrc,	"rrd03d.p1",		0x0000, 0x010000, CRC(44740c79) SHA1(ab1efb2090ef62795c17a685c7acb45820eb1a9d), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 29)" )
M4RHRC_SET( 199?, m4rhrc__2,	m4rhrc,	"rrd03dk.p1",		0x0000, 0x010000, CRC(78f18187) SHA1(33764416c6e5cccd6ae5fdc5c0d679e1ef451785), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 30)" )
M4RHRC_SET( 199?, m4rhrc__3,	m4rhrc,	"rrd03dr.p1",		0x0000, 0x010000, CRC(039c2869) SHA1(2eb887b36d86295d0e6aacc74d0a6223d32baa5a), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 31)" )
M4RHRC_SET( 199?, m4rhrc__4,	m4rhrc,	"rrd03dy.p1",		0x0000, 0x010000, CRC(b60b6e51) SHA1(eb6ed1de44d7c982ac8aa0621d4c1ed8e41db5de), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 32)" )
M4RHRC_SET( 199?, m4rhrc__5,	m4rhrc,	"rrd03k.p1",		0x0000, 0x010000, CRC(31adc6d6) SHA1(ea68d0d13978bf6cfa7fb9aa1cf91ddfd6258a3a), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 33)" )
M4RHRC_SET( 199?, m4rhrc__6,	m4rhrc,	"rrd03r.p1",		0x0000, 0x010000, CRC(11c61483) SHA1(66cd30096bca2f4356acaaa15179c00301c8bc3a), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 34)" )
M4RHRC_SET( 199?, m4rhrc__7,	m4rhrc,	"rrd03s.p1",		0x0000, 0x010000, CRC(e59b79dd) SHA1(32e515bdc861a4d548caedd56a1825c91a318a34), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 35)" )
M4RHRC_SET( 199?, m4rhrc__8,	m4rhrc,	"rrd03y.p1",		0x0000, 0x010000, CRC(66fff07a) SHA1(586279533d6d85abf7e97124c9c5342a6a1b0496), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 36)" )
M4RHRC_SET( 199?, m4rhrc__9,	m4rhrc,	"cr__x__x.5_0",		0x0000, 0x010000, CRC(278fe91e) SHA1(dcfed3a7796d1ee365e535115b66c7d6cbe0ab74), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 37)" )
M4RHRC_SET( 199?, m4rhrc__aa,	m4rhrc,	"cr__x_dx.5_0",		0x0000, 0x010000, CRC(4bcf5c02) SHA1(603935880c87f86e7bc765c176266c1c08a6114f), "Barcrest","Red Hot Roll Classic (Barcrest) (MPU4) (set 38)" )

#define M4VIZ_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "vizs.chr", 0x0000, 0x000048, CRC(2365ca6b) SHA1(b964420eb1df4065b2a6f3f934135d435b52af2b) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "vizsnd.p1", 0x000000, 0x080000, CRC(2b39a622) SHA1(77916650fe19f18025e10fb25de704f7bb733295) ) \
	ROM_LOAD( "vizsnd.p2", 0x080000, 0x080000, CRC(e309bede) SHA1(a4615436fcfd5f31293f887b8bc972f0d2d6b0cb) ) \

#define M4VIZ_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4VIZ_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \



M4VIZ_SET( 199?, m4viz,		0,		"viz208c",		0x0000, 0x010000, CRC(00a65029) SHA1(8dfb68d1a9f4cd00f239ed87a1d330ccb655c35b), "Barcrest","Viz (Barcrest) (MPU4) (set 1)" )
M4VIZ_SET( 199?, m4viz__a,	m4viz,	"viz20_101",	0x0000, 0x010000, CRC(0847b812) SHA1(6de9e9dad272932a22ebe457ac50da1126d931ea), "Barcrest","Viz (Barcrest) (MPU4) (set 2)" )
M4VIZ_SET( 199?, m4viz__b,	m4viz,	"viz20pv2",		0x0000, 0x010000, CRC(7e56ff95) SHA1(83679b64881adbe547b43255374de061859e17ef), "Barcrest","Viz (Barcrest) (MPU4) (set 3)" )
M4VIZ_SET( 199?, m4viz__c,	m4viz,	"viz58c",		0x0000, 0x010000, CRC(95b8918b) SHA1(4ad4ff9098e98c2076e7058493c181da705acb52), "Barcrest","Viz (Barcrest) (MPU4) (set 4)" )
M4VIZ_SET( 199?, m4viz__d,	m4viz,	"vizb.p1",		0x0000, 0x010000, CRC(afdc6306) SHA1(4d35703267b3742dd7008c00ec525689c56bf227), "Barcrest","Viz (Barcrest) (MPU4) (set 5)" )
M4VIZ_SET( 199?, m4viz__e,	m4viz,	"vizc.p1",		0x0000, 0x010000, CRC(876c30fc) SHA1(f126496e87d7e84ca39d2921bf9f2be0fa2c7586), "Barcrest","Viz (Barcrest) (MPU4) (set 6)" )
M4VIZ_SET( 199?, m4viz__f,	m4viz,	"vizd.p1",		0x0000, 0x010000, CRC(46bee8cd) SHA1(4094651fd8954ca2f5cfc2bba4fc51d865c86098), "Barcrest","Viz (Barcrest) (MPU4) (set 7)" )
M4VIZ_SET( 199?, m4viz__g,	m4viz,	"vizdk.p1",		0x0000, 0x010000, CRC(24476360) SHA1(b5141a40f8c1ed3b3fbaf43ae539ae2f1aedbcca), "Barcrest","Viz (Barcrest) (MPU4) (set 8)" )
M4VIZ_SET( 199?, m4viz__h,	m4viz,	"vizdy.p1",		0x0000, 0x010000, CRC(88807a1f) SHA1(dc1539a5e69b5f0b3f68ccf7360ff4f240f6b7c7), "Barcrest","Viz (Barcrest) (MPU4) (set 9)" )
M4VIZ_SET( 199?, m4viz__i,	m4viz,	"vizk.p1",		0x0000, 0x010000, CRC(6647f592) SHA1(2ce7222bd9e173480ddc901f84859ca3ad7aded1), "Barcrest","Viz (Barcrest) (MPU4) (set 10)" )
M4VIZ_SET( 199?, m4viz__j,	m4viz,	"vizs.p1",		0x0000, 0x010000, CRC(86b487dc) SHA1(62215752e1da1ca923e6b9e410c8445577be34dd), "Barcrest","Viz (Barcrest) (MPU4) (set 11)" )
M4VIZ_SET( 199?, m4viz__k,	m4viz,	"vizy.p1",		0x0000, 0x010000, CRC(0e12112d) SHA1(4a34832dd95246e80e616affe3eab3c8794ca769), "Barcrest","Viz (Barcrest) (MPU4) (set 12)" )
M4VIZ_SET( 199?, m4viz__l,	m4viz,	"vizzzvkn",		0x0000, 0x010000, CRC(cf5c41f5) SHA1(c9b7de0e73141833e5f8d23f0cb641b1c6094178), "Barcrest","Viz (Barcrest) (MPU4) (set 13)" )
M4VIZ_SET( 199?, m4viz__m,	m4viz,	"vi_05a__.1_1",	0x0000, 0x010000, CRC(56e0ea7a) SHA1(cbe979cdfceb2c1c7be5adaf8163b96bebbc4bb6), "Barcrest","Viz (Barcrest) (MPU4) (set 14)" )
M4VIZ_SET( 199?, m4viz__n,	m4viz,	"vi_05s__.1_1",	0x0000, 0x010000, CRC(c6896e33) SHA1(7db1a5e08f1a307aac0818424fab274cd8141474), "Barcrest","Viz (Barcrest) (MPU4) (set 15)" )
M4VIZ_SET( 199?, m4viz__o,	m4viz,	"vi_05sb_.1_1",	0x0000, 0x010000, CRC(12fecbdf) SHA1(c0137aac536ec17c3b2ffa405f8400308f759590), "Barcrest","Viz (Barcrest) (MPU4) (set 16)" )
M4VIZ_SET( 199?, m4viz__p,	m4viz,	"vi_05sd_.1_1",	0x0000, 0x010000, CRC(9241fd92) SHA1(f3e58273089ee9b828e431a043802d4ec3948a64), "Barcrest","Viz (Barcrest) (MPU4) (set 17)" )
M4VIZ_SET( 199?, m4viz__q,	m4viz,	"vi_10a__.1_1",	0x0000, 0x010000, CRC(e7c4e4d9) SHA1(9ac3bd60e6000e36cd2229284c48e009ea22cfdb), "Barcrest","Viz (Barcrest) (MPU4) (set 18)" )
M4VIZ_SET( 199?, m4viz__r,	m4viz,	"vi_10s__.1_1",	0x0000, 0x010000, CRC(039a4620) SHA1(097335ba8846c8c8b28bf85f836ba76d22bc763d), "Barcrest","Viz (Barcrest) (MPU4) (set 19)" )
M4VIZ_SET( 199?, m4viz__s,	m4viz,	"vi_10sb_.1_1",	0x0000, 0x010000, CRC(4b7e6686) SHA1(97985f1ecd3a8e77f07a91c5171810e6aff13f4c), "Barcrest","Viz (Barcrest) (MPU4) (set 20)" )
M4VIZ_SET( 199?, m4viz__t,	m4viz,	"vi_10sd_.1_1",	0x0000, 0x010000, CRC(84da6fca) SHA1(8a42855b161619a56435da52dd24e8e60fb56bd8), "Barcrest","Viz (Barcrest) (MPU4) (set 21)" )
M4VIZ_SET( 199?, m4viz__u,	m4viz,	"vii05___.1_1",	0x0000, 0x010000, CRC(22a10f78) SHA1(83411b77e5de441b0f5fa02f2b1dbc40755f41cb), "Barcrest","Viz (Barcrest) (MPU4) (set 22)" )
M4VIZ_SET( 199?, m4viz__v,	m4viz,	"vii10___.1_1",	0x0000, 0x010000, CRC(92e11e00) SHA1(2ebae74a39434269333ea0067163e9607926646d), "Barcrest","Viz (Barcrest) (MPU4) (set 23)" )
M4VIZ_SET( 199?, m4viz__w,	m4viz,	"viz_20_.8",	0x0000, 0x010000, CRC(b4fbc43b) SHA1(4cce5e3a0c32a402b81dfd16e66d12e98704c4d2), "Barcrest","Viz (Barcrest) (MPU4) (set 24)" )



#define M4TAKEPK_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "m441.chr", 0x0000, 0x000048, CRC(3ec3a5fa) SHA1(ea8c831da9944506393dd5d5f380a084fd6543b6) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "tapsnd1.hex", 0x000000, 0x080000, CRC(8dc408e3) SHA1(48a9ffc5cf4fd04ed1320619ca915bbfa2406750) ) \
	ROM_LOAD( "tapsnd2.hex", 0x080000, 0x080000, CRC(6034e17a) SHA1(11e044c87b5fc6461b0c6cfac5c419daee930d7b) ) \
	ROM_LOAD( "typkp2", 0x080000, 0x080000, CRC(753d9bc1) SHA1(c27c8b7cfba7ad67685f637ee3f68a3edb7986e7) ) /* alt copy of tapsnd2 */ \

#define M4TAKEPK_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TAKEPK_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4TAKEPK_SET( 199?, m4takepk,		0,			"tapy.p1",		0x0000, 0x020000, CRC(f21f6dc8) SHA1(d421bee2564d3aaa389c35601adc23ad3fda5aa0), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 1)" )
M4TAKEPK_SET( 199?, m4takepk__a,	m4takepk,	"tapad.p1",		0x0000, 0x020000, CRC(162448c4) SHA1(1f77d053fb5dfddeba1248e9e2a05536ab1bc66a), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 2)" )
M4TAKEPK_SET( 199?, m4takepk__b,	m4takepk,	"tapb.p1",		0x0000, 0x020000, CRC(3f3be560) SHA1(a60d66c5de33747d19ae43bbc15da104cc3e7390), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 3)" )
M4TAKEPK_SET( 199?, m4takepk__c,	m4takepk,	"tapbd.p1",		0x0000, 0x020000, CRC(9b3ee601) SHA1(95f11641d9e3ca0bcfa11a17fb0971b1e2598c7b), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 4)" )
M4TAKEPK_SET( 199?, m4takepk__d,	m4takepk,	"tapc.p1",		0x0000, 0x020000, CRC(99bf563d) SHA1(23672e4519911033f607566b27217c75b8b8651e), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 5)" )
M4TAKEPK_SET( 199?, m4takepk__e,	m4takepk,	"tapd.p1",		0x0000, 0x020000, CRC(4220ee16) SHA1(6c677b24f40e481bb3e61fc3bccccde39b088f8d), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 6)" )
M4TAKEPK_SET( 199?, m4takepk__f,	m4takepk,	"tapdk.p1",		0x0000, 0x020000, CRC(d1f94e57) SHA1(b1dde10ce45c668b5ba5a05462d41716779206a6), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 7)" )
M4TAKEPK_SET( 199?, m4takepk__g,	m4takepk,	"tapdy.p1",		0x0000, 0x020000, CRC(561a6ea9) SHA1(736af1fed00a4df540b8f83da677583dee950b50), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 8)" )
M4TAKEPK_SET( 199?, m4takepk__h,	m4takepk,	"tapk.p1",		0x0000, 0x020000, CRC(75fc4d36) SHA1(18a0e33af69c32a69416612f639a7c8601010177), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 9)" )
M4TAKEPK_SET( 199?, m4takepk__i,	m4takepk,	"tapr.p1",		0x0000, 0x020000, CRC(c6f3f607) SHA1(cee0b59a45ebb50d51bc26b2b5c37fe7ed299bc7), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 10)" )
M4TAKEPK_SET( 199?, m4takepk__j,	m4takepk,	"taps.p1",		0x0000, 0x020000, CRC(01956f25) SHA1(895cd30023b689b61d5ced0cf477f555faf786af), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 11)" )
M4TAKEPK_SET( 199?, m4takepk__k,	m4takepk,	"tphad.p1",		0x0000, 0x020000, CRC(51a2f147) SHA1(442d4adc92c6a9215c7655c2c4b955f974420a26), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 12)" )
M4TAKEPK_SET( 199?, m4takepk__l,	m4takepk,	"tphb.p1",		0x0000, 0x020000, CRC(391214a6) SHA1(21f6f29ff1a60b7a646d49bef2c29008f2dc501c), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 13)" )
M4TAKEPK_SET( 199?, m4takepk__m,	m4takepk,	"tphbd.p1",		0x0000, 0x020000, CRC(dcb85f82) SHA1(eb83c8d829d88e940c11f6d0d1c4607a4aa9cba2), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 14)" )
M4TAKEPK_SET( 199?, m4takepk__n,	m4takepk,	"tphd.p1",		0x0000, 0x020000, CRC(44091fd0) SHA1(a05814bc27da6a452802a253060efd9d04956d45), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 15)" )
M4TAKEPK_SET( 199?, m4takepk__o,	m4takepk,	"tphdk.p1",		0x0000, 0x020000, CRC(967ff7d4) SHA1(403865583aabc9983b8c6fa4f0c06ba53696df3b), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 16)" )
M4TAKEPK_SET( 199?, m4takepk__p,	m4takepk,	"tphdy.p1",		0x0000, 0x020000, CRC(119cd72a) SHA1(326f5a548234ec5cd780f22c96b9151222684ed0), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 17)" )
M4TAKEPK_SET( 199?, m4takepk__q,	m4takepk,	"tphk.p1",		0x0000, 0x020000, CRC(73d5bcf0) SHA1(22c620c7d6fc8bc51bda4dd3ae5ec3e38056ce82), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 18)" )
M4TAKEPK_SET( 199?, m4takepk__r,	m4takepk,	"tphs.p1",		0x0000, 0x020000, CRC(e9231738) SHA1(066b1ffd02238783931452b7a1dff05c293a6abe), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 19)" )
M4TAKEPK_SET( 199?, m4takepk__s,	m4takepk,	"tphy.p1",		0x0000, 0x020000, CRC(f4369c0e) SHA1(d8c1fc2ede48673a1e8efaf004e3d76b62594de1), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 20)" )
M4TAKEPK_SET( 199?, m4takepk__t,	m4takepk,	"typ15f",		0x0000, 0x020000, CRC(65c44b06) SHA1(629e7ac4149c66fc1dc33a103e1a4ff5aaecdcfd), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 21)" )
M4TAKEPK_SET( 199?, m4takepk__u,	m4takepk,	"typ15r",		0x0000, 0x020000, CRC(8138c70b) SHA1(aafc805a8a56cf1722ebe0f3eba0a47f15c9049a), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 22)" )
M4TAKEPK_SET( 199?, m4takepk__v,	m4takepk,	"typ510",		0x0000, 0x010000, CRC(ebf0c71c) SHA1(6c759144aecce83f82ded8aae7c61ecec2d92fb3), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 23)" )
M4TAKEPK_SET( 199?, m4takepk__w,	m4takepk,	"typ510s",		0x0000, 0x010000, CRC(4cc6032d) SHA1(e6eaff56e39555393156aa2e56bf1c17e548bdc9), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 24)" )
M4TAKEPK_SET( 199?, m4takepk__x,	m4takepk,	"typ55",		0x0000, 0x010000, CRC(6837344f) SHA1(4d5c6ea005d0916f27a7f445b37ce9252549c61f), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 25)" )
M4TAKEPK_SET( 199?, m4takepk__y,	m4takepk,	"typ55s",		0x0000, 0x010000, CRC(05dc9b07) SHA1(9fc2c7575a704ca1252bb5c6a638e28b0324f2a6), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 26)" )
M4TAKEPK_SET( 199?, m4takepk__z,	m4takepk,	"typ58s",		0x0000, 0x010000, CRC(56e26a42) SHA1(7add260212d3fbc8b356b58e85df8cafbef151e3), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 27)" )
M4TAKEPK_SET( 199?, m4takepk__0,	m4takepk,	"typ58t",		0x0000, 0x010000, CRC(3fbbbbc8) SHA1(9f097cbce3710a51c19ef7961f91ee6e77fc843f), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 28)" )
M4TAKEPK_SET( 199?, m4takepk__1,	m4takepk,	"typ5p10p.bin",	0x0000, 0x010000, CRC(45ddeaf4) SHA1(6db822aac402cb6772718015420c14875e74b13d), "Barcrest","Take Your Pick (Barcrest) (MPU4) (set 29)" )


#define M4OVERMN_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "otts.chr", 0x0000, 0x000048, CRC(2abec763) SHA1(307399724a994a5d0914a5d7e0931a5d94439a37) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "otnsnd.p1", 0x0000, 0x080000, CRC(d4f7ed82) SHA1(16e80bf0956f39a9e8e23384615a07594419db59) ) \

#define M4OVERMN_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4OVERMN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4OVERMN_SET( 199?, m4overmn,		0,			"otts.p1",	0x0000, 0x010000, CRC(6daf58a4) SHA1(e505a18b67dec54446e6d94a5d1c3bba13099619), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 1)" )
M4OVERMN_SET( 199?, m4overmn__a,	m4overmn,	"ot8b.p1",	0x0000, 0x010000, CRC(243c7f7c) SHA1(24b9d2cce1af75811d1e625ac8df5b58356776dc), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 2)" )
M4OVERMN_SET( 199?, m4overmn__b,	m4overmn,	"ot8c.p1",	0x0000, 0x010000, CRC(af5bb77b) SHA1(6a9eeb803fdaa03970b3a3a0738e804027aedd20), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 3)" )
M4OVERMN_SET( 199?, m4overmn__c,	m4overmn,	"ot8dk.p1",	0x0000, 0x010000, CRC(0798d12c) SHA1(068a676d4ccaf2964a3f6f6673199f8d62c45452), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 4)" )
M4OVERMN_SET( 199?, m4overmn__d,	m4overmn,	"ot8dy.p1",	0x0000, 0x010000, CRC(0904a38d) SHA1(0a0668ae384fe371abf2597ab66a56dd79a90c03), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 5)" )
M4OVERMN_SET( 199?, m4overmn__e,	m4overmn,	"ot8k.p1",	0x0000, 0x010000, CRC(8d83f697) SHA1(2fff475d44f1535c85988b195c3610201ece21ae), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 6)" )
M4OVERMN_SET( 199?, m4overmn__f,	m4overmn,	"ot8s.p1",	0x0000, 0x010000, CRC(db1bacdb) SHA1(fc2257eedec532094f3c229bcf215a0fde430d2b), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 7)" )
M4OVERMN_SET( 199?, m4overmn__g,	m4overmn,	"ot8y.p1",	0x0000, 0x010000, CRC(6e1508fb) SHA1(6a45a394e48f758456dc6cf17a5b134ca6887421), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 8)" )
M4OVERMN_SET( 199?, m4overmn__h,	m4overmn,	"otnb.p1",	0x0000, 0x010000, CRC(047aae70) SHA1(bf620b60f1107fff07a28944dec66fd71aab65c0), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 9)" )
M4OVERMN_SET( 199?, m4overmn__i,	m4overmn,	"otnc.p1",	0x0000, 0x010000, CRC(536e7640) SHA1(3a079bed9217c857efb8d435c7efacca69cfcabc), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 10)" )
M4OVERMN_SET( 199?, m4overmn__j,	m4overmn,	"otnd.p1",	0x0000, 0x010000, CRC(c6c15fc2) SHA1(99cb3fd2eaea636313085e0a6a9aff9c027cb187), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 11)" )
M4OVERMN_SET( 199?, m4overmn__k,	m4overmn,	"otndy.p1",	0x0000, 0x010000, CRC(6b22206e) SHA1(0714ddc445d530e1ff2055cd5e5d8b31704733d9), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 12)" )
M4OVERMN_SET( 199?, m4overmn__l,	m4overmn,	"otnk.p1",	0x0000, 0x010000, CRC(992cc40d) SHA1(6059ecaf91390e2a2ea80d3da5e44156273892ad), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 13)" )
M4OVERMN_SET( 199?, m4overmn__m,	m4overmn,	"otns.p1",	0x0000, 0x010000, CRC(7e03f295) SHA1(f874ddf8de8037aa251a8c3fb7c183e6dfb93dfa), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 14)" )
M4OVERMN_SET( 199?, m4overmn__n,	m4overmn,	"otny.p1",	0x0000, 0x010000, CRC(67cba8fa) SHA1(234cc5b4a0b60d33b2f4c00d082beee59236a126), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 15)" )
M4OVERMN_SET( 199?, m4overmn__o,	m4overmn,	"ottad.p1",	0x0000, 0x010000, CRC(682b01a3) SHA1(cb71fd56ad6d4fc67894bf86c54c49a7e45aae15), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 16)" )
M4OVERMN_SET( 199?, m4overmn__p,	m4overmn,	"ottb.p1",	0x0000, 0x010000, CRC(541c2d54) SHA1(3b42e9dcb468cb9bbf2092a4e7eabeb172dc90d0), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 17)" )
M4OVERMN_SET( 199?, m4overmn__q,	m4overmn,	"ottbd.p1",	0x0000, 0x010000, CRC(5e376ce9) SHA1(0628461395ebd233ca7b0513ea272ddd83c5accd), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 18)" )
M4OVERMN_SET( 199?, m4overmn__r,	m4overmn,	"ottd.p1",	0x0000, 0x010000, CRC(9b013a2b) SHA1(734cc79bc9452c86434bd085463ab512b5421dae), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 19)" )
M4OVERMN_SET( 199?, m4overmn__s,	m4overmn,	"ottdk.p1",	0x0000, 0x010000, CRC(c205194f) SHA1(bdfdffe09fd995c8ded80cc3042d2ce1eebad8bc), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 20)" )
M4OVERMN_SET( 199?, m4overmn__t,	m4overmn,	"ottdr.p1",	0x0000, 0x010000, CRC(f2b9bf4c) SHA1(5f1c5c347930b75473dcd83cf2ad5870ba26e289), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 21)" )
M4OVERMN_SET( 199?, m4overmn__u,	m4overmn,	"ottdy.p1",	0x0000, 0x010000, CRC(936b70e2) SHA1(6d434f399dc851621b703a1bf93bc71bed78d867), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 22)" )
M4OVERMN_SET( 199?, m4overmn__v,	m4overmn,	"ottk.p1",	0x0000, 0x010000, CRC(68c984d3) SHA1(b1cf87630ab093629eaa8d199dfcfd6343d9c31d), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 23)" )
M4OVERMN_SET( 199?, m4overmn__w,	m4overmn,	"ottr.p1",	0x0000, 0x010000, CRC(ceb322d1) SHA1(a62bd1f947fc15f1d42dae8e933d2fcb672bcce4), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 24)" )
M4OVERMN_SET( 199?, m4overmn__x,	m4overmn,	"otty.p1",	0x0000, 0x010000, CRC(974af7ff) SHA1(e0aecb91c1fc476a9258d6d57ba5ca8f249141b0), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 25)" )
M4OVERMN_SET( 199?, m4overmn__y,	m4overmn,	"otuad.p1",	0x0000, 0x010000, CRC(2576654b) SHA1(7fae2bd057d96af4c50fd84a5261ae750ba34033), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 26)" )
M4OVERMN_SET( 199?, m4overmn__z,	m4overmn,	"otub.p1",	0x0000, 0x010000, CRC(1463877d) SHA1(ea41e048aead52aabc1b8a2a224ef87b9011c163), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 27)" )
M4OVERMN_SET( 199?, m4overmn__0,	m4overmn,	"otubd.p1",	0x0000, 0x010000, CRC(8ac2d17b) SHA1(09f21f1233d82fd02830b6ece6a773402393a447), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 28)" )
M4OVERMN_SET( 199?, m4overmn__1,	m4overmn,	"otud.p1",	0x0000, 0x010000, CRC(8f1632c2) SHA1(729f2182c40f98e9b2fb9996d14c11d2334ba15f), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 29)" )
M4OVERMN_SET( 199?, m4overmn__2,	m4overmn,	"otudk.p1",	0x0000, 0x010000, CRC(2edbbe5d) SHA1(f6b8b625bf2d021524595ef1f69e730e78f42aa8), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 30)" )
M4OVERMN_SET( 199?, m4overmn__3,	m4overmn,	"otudr.p1",	0x0000, 0x010000, CRC(f799f424) SHA1(f0d1a72088dd3cd6f9ccaa1bf0e9a28f656194e0), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 31)" )
M4OVERMN_SET( 199?, m4overmn__4,	m4overmn,	"otudy.p1",	0x0000, 0x010000, CRC(562da6fd) SHA1(899f971124969c52a018634b2b2f2dd7cb634195), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 32)" )
M4OVERMN_SET( 199?, m4overmn__5,	m4overmn,	"otuk.p1",	0x0000, 0x010000, CRC(cbb66497) SHA1(ade033fb3d226bfcb3cdf3e3612fb65cfc22b030), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 33)" )
M4OVERMN_SET( 199?, m4overmn__6,	m4overmn,	"otur.p1",	0x0000, 0x010000, CRC(d05a8c2f) SHA1(754e2351431aa7bf6dea3a1498581da0c4283c1e), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 34)" )
M4OVERMN_SET( 199?, m4overmn__7,	m4overmn,	"otus.p1",	0x0000, 0x010000, CRC(5f2b8d0b) SHA1(1e3ac59fa0b108549c265eeba027591bce5122f3), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 35)" )
M4OVERMN_SET( 199?, m4overmn__8,	m4overmn,	"otuy.p1",	0x0000, 0x010000, CRC(fc65136d) SHA1(048f81de92a1db4e4e4e9aa7a87228805d57b263), "Barcrest","Over The Moon (Barcrest) (MPU4) (set 36)" )


#define M4VIVALV_EXTRA_ROMS \
	ROM_REGION( 0x080000, "msm6376", 0 ) \
	ROM_LOAD( "vivalasvegas4.bin", 0x0000, 0x080000, CRC(76971425) SHA1(0974a9dce51cc3dd4e26cec11a948c9c8021fde4) ) \

#define M4VIVALV_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4VIVALV_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \



M4VIVALV_SET( 199?, m4vivalv,		0,			"5p5vivalasvegas6.bin",			0x0000, 0x010000, CRC(4d365b57) SHA1(69ff75ccc91f1f7b867a0914d350d1649834a48e), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 1)" )
M4VIVALV_SET( 199?, m4vivalv__a,	m4vivalv,	"v.las vegas 6 10p 10m.bin",	0x0000, 0x010000, CRC(f09d5a2c) SHA1(6f9df58767e88a1ca7fc7dd17c618d30ab97067d), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 2)" )
M4VIVALV_SET( 199?, m4vivalv__b,	m4vivalv,	"vegas15g",						0x0000, 0x020000, CRC(5b804e4d) SHA1(5967b9f4e28e5a5e5e7005a29ecb30fc661800e0), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 3)" )
M4VIVALV_SET( 199?, m4vivalv__c,	m4vivalv,	"vegas15t",						0x0000, 0x020000, CRC(9d15f44f) SHA1(3a3f2034de9ba56cb2cb18b4898f2829a2019d4e), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 4)" )
M4VIVALV_SET( 199?, m4vivalv__d,	m4vivalv,	"viv55",						0x0000, 0x010000, CRC(4d365b57) SHA1(69ff75ccc91f1f7b867a0914d350d1649834a48e), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 5)" )
M4VIVALV_SET( 199?, m4vivalv__e,	m4vivalv,	"viva05_11",					0x0000, 0x010000, CRC(1e6ea483) SHA1(e6a53eb1bf3b8e661287c0d57fc6ab5ed41755a3), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 6)" )
M4VIVALV_SET( 199?, m4vivalv__f,	m4vivalv,	"viva10_11",					0x0000, 0x010000, CRC(246a39b7) SHA1(c0f5c21374e43b42df5df0ada0967a34ecefbdb4), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 7)" )
M4VIVALV_SET( 199?, m4vivalv__g,	m4vivalv,	"viva20_1.1",					0x0000, 0x010000, CRC(80ea2429) SHA1(e5d258967340fe85dd5baf6ba16f82ce83307b68), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 8)" )
M4VIVALV_SET( 199?, m4vivalv__h,	m4vivalv,	"viva20_11",					0x0000, 0x010000, CRC(51b93018) SHA1(fc13179e3e1939839c3b90d7600a7eb301ea03da), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 9)" )
M4VIVALV_SET( 199?, m4vivalv__i,	m4vivalv,	"viva58c",						0x0000, 0x010000, CRC(719d0802) SHA1(ba6bd5fbf49f0ada383cb2e8faa037b78f6af587), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 10)" )
M4VIVALV_SET( 199?, m4vivalv__j,	m4vivalv,	"viva_05_.4",					0x0000, 0x010000, CRC(b094914f) SHA1(8217b4bb7a8d55fb8e86018ffc520a63f41a79b8), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 11)" )
M4VIVALV_SET( 199?, m4vivalv__k,	m4vivalv,	"viva_05_.8",					0x0000, 0x010000, CRC(c5c09c10) SHA1(47890d0ba1c2ca53231ac148a02f046452dce1b4), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 12)" )
M4VIVALV_SET( 199?, m4vivalv__l,	m4vivalv,	"viva_10_.4",					0x0000, 0x010000, CRC(b1d5e820) SHA1(68012216d7e82168c7468d1e54c527c15d268917), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 13)" )
M4VIVALV_SET( 199?, m4vivalv__m,	m4vivalv,	"viva_10_.8",					0x0000, 0x010000, CRC(f392c81c) SHA1(cb3320b688b315dbc226f45b78490fed439ee9a2), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 14)" )
M4VIVALV_SET( 199?, m4vivalv__n,	m4vivalv,	"viva_20_.4",					0x0000, 0x010000, CRC(e1efc846) SHA1(a4bf7f5c4febe5a71a09e23876387328e1bba87b), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 15)" )
M4VIVALV_SET( 199?, m4vivalv__o,	m4vivalv,	"viva_20_.8",					0x0000, 0x010000, CRC(f538a1fc) SHA1(d0dbd22a1cb4b7ec5bfa304ba544806e01150662), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 16)" )
M4VIVALV_SET( 199?, m4vivalv__p,	m4vivalv,	"vlv208ac",						0x0000, 0x010000, CRC(416535ee) SHA1(f2b0177fecd5076d9d89c819fe9402fc944c8d77), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 17)" )
M4VIVALV_SET( 199?, m4vivalv__q,	m4vivalv,	"vlvad.p1",						0x0000, 0x010000, CRC(88262812) SHA1(f0a31d510c1b06af122df493585c04a49177f06d), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 18)" )
M4VIVALV_SET( 199?, m4vivalv__r,	m4vivalv,	"vlvb.p1",						0x0000, 0x010000, CRC(c4caec15) SHA1(d88c6e081a6bbdd80f773713b038293cabdeee8c), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 19)" )
M4VIVALV_SET( 199?, m4vivalv__s,	m4vivalv,	"vlvc.p1",						0x0000, 0x010000, CRC(4d651ba4) SHA1(7746656f0a9f8af8e265568f7479edef9a2247d9), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 20)" )
M4VIVALV_SET( 199?, m4vivalv__t,	m4vivalv,	"vlvd.p1",						0x0000, 0x010000, CRC(cce926c7) SHA1(8e3a0cef0cbee66d264da5d6dfc7ec2fbdcd9584), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 21)" )
M4VIVALV_SET( 199?, m4vivalv__u,	m4vivalv,	"vlvdy.p1",						0x0000, 0x010000, CRC(6e17cbc8) SHA1(5c69eda0ff6a01d9d0d434ff7ce1ac1e67b16362), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 22)" )
M4VIVALV_SET( 199?, m4vivalv__v,	m4vivalv,	"vlvk.p1",						0x0000, 0x010000, CRC(b5f2157e) SHA1(574f3e2890ac5479790ea92760c6500d37e6637d), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 23)" )
M4VIVALV_SET( 199?, m4vivalv__w,	m4vivalv,	"vlvs.p1",						0x0000, 0x010000, CRC(b7fb3e19) SHA1(c6cc4175f8c100fc37e6e7014b0744054b4e547a), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 24)" )
M4VIVALV_SET( 199?, m4vivalv__x,	m4vivalv,	"vlvy.p1",						0x0000, 0x010000, CRC(3211caf3) SHA1(3634ef11099c2f4938529bb262cc2556ad96a675), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 25)" )
M4VIVALV_SET( 199?, m4vivalv__y,	m4vivalv,	"vv_05___.3_3",					0x0000, 0x010000, CRC(bb8361f6) SHA1(d5f651a66be1cab3662798751a290a65c29bba64), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 26)" )
M4VIVALV_SET( 199?, m4vivalv__z,	m4vivalv,	"vv_05_b_.3_3",					0x0000, 0x010000, CRC(12079321) SHA1(5b5dd55080c04393a45d3ef9c63b6fef5de9b7cd), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 27)" )
M4VIVALV_SET( 199?, m4vivalv__0,	m4vivalv,	"vv_05_d_.3_3",					0x0000, 0x010000, CRC(b758df52) SHA1(f4d47a93fa1b1deb84654bb2272767093f3463c2), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 28)" )
M4VIVALV_SET( 199?, m4vivalv__1,	m4vivalv,	"vv_05_k_.3_3",					0x0000, 0x010000, CRC(9875c59c) SHA1(c31a7fc5df8af9d931353bc095a59befe808434b), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 29)" )
M4VIVALV_SET( 199?, m4vivalv__2,	m4vivalv,	"vv_05a__.3_3",					0x0000, 0x010000, CRC(0f416e47) SHA1(54338fbef5f227c440c04448b51e8f0ec04a4cc7), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 30)" )
M4VIVALV_SET( 199?, m4vivalv__3,	m4vivalv,	"vv_10___.3_3",					0x0000, 0x010000, CRC(dc8db002) SHA1(305547b4f0b1e1bde9354e5ed9f18f99c6829cab), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 31)" )
M4VIVALV_SET( 199?, m4vivalv__4,	m4vivalv,	"vv_10_b_.3_3",					0x0000, 0x010000, CRC(e1c4b292) SHA1(4516c7d918935862824e206626a5a24f936ec514), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 32)" )
M4VIVALV_SET( 199?, m4vivalv__5,	m4vivalv,	"vv_10_d_.3_3",					0x0000, 0x010000, CRC(e9dda1ee) SHA1(6363b5b26be22cb1f5aac71e98c5e5a5064839f4), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 33)" )
M4VIVALV_SET( 199?, m4vivalv__6,	m4vivalv,	"vv_10_k_.3_3",					0x0000, 0x010000, CRC(70fc4c56) SHA1(02cbaadd3575ef0d9dc192aabbe39a735893a662), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 34)" )
M4VIVALV_SET( 199?, m4vivalv__7,	m4vivalv,	"vv_10a__.3_3",					0x0000, 0x010000, CRC(c908d65a) SHA1(5af180e697c22c27380e275d76708103e298cf41), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 35)" )
M4VIVALV_SET( 199?, m4vivalv__8,	m4vivalv,	"vvi05___.3_3",					0x0000, 0x010000, CRC(a5829d5c) SHA1(4cd1a2185579898db7be75f8c3f565043f0691b6), "Barcrest","Viva Las Vegas (Barcrest) (MPU4) (set 36)" )




#define M4LUXOR_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "luxor.chr", 0x0000, 0x000048, CRC(21676e79) SHA1(b8f69e9aa35be1491655c0c52df277619892bdd8) ) \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) \
	ROM_LOAD( "luxorsnd1.hex", 0x000000, 0x080000, CRC(428daceb) SHA1(eec2b7efded3d0e0eea7faa5759a65a021465b13) ) \
	ROM_LOAD( "luxorsnd2.hex", 0x080000, 0x080000, CRC(860178e6) SHA1(705b1b0ad62a1b594bb123aec3c2b571a6500ce8) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "luxor-snd1.bin", 0x000000, 0x080000, CRC(d09394e9) SHA1(d3cbdbaf048d829271a6c2846b16ceee7775d767) ) \
	ROM_LOAD( "luxor-snd2.bin", 0x080000, 0x080000, CRC(bc720cb9) SHA1(a83c25ecec602ba047dd21de2beec6cd7ac76cbe) ) \

#define M4LUXOR_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4LUXOR_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \



M4LUXOR_SET( 199?, m4luxor,		0,			"luxor.rom",		0x0000, 0x010000, CRC(55277510) SHA1(9a866c36a398df52c54b554cd8085078c1f1954b), "Barcrest","Luxor (Barcrest) (MPU4) (set 1)" )
M4LUXOR_SET( 199?, m4luxor__a,	m4luxor,	"lux05_101",		0x0000, 0x010000, CRC(8f4dc4f4) SHA1(c9743a1b79b377313504296a060dff3f413a7a9d), "Barcrest","Luxor (Barcrest) (MPU4) (set 2)" )
M4LUXOR_SET( 199?, m4luxor__b,	m4luxor,	"lux10_101",		0x0000, 0x010000, CRC(8965c7be) SHA1(ca05803bc7d7a96e25dc0b025c2683b4679789fb), "Barcrest","Luxor (Barcrest) (MPU4) (set 3)" )
M4LUXOR_SET( 199?, m4luxor__c,	m4luxor,	"lux208c",			0x0000, 0x010000, CRC(f57bae67) SHA1(3a2523a2121948480381f49e26e870b10d541304), "Barcrest","Luxor (Barcrest) (MPU4) (set 4)" )
M4LUXOR_SET( 199?, m4luxor__d,	m4luxor,	"lux55",			0x0000, 0x010000, CRC(997419ab) SHA1(c616a5d6cb347963e7e5c5b88912c248bae184ca), "Barcrest","Luxor (Barcrest) (MPU4) (set 5)" )
M4LUXOR_SET( 199?, m4luxor__e,	m4luxor,	"lux58c",			0x0000, 0x010000, CRC(da408721) SHA1(971413620d1f304a026d3adc68f6ac5c1d104e20), "Barcrest","Luxor (Barcrest) (MPU4) (set 6)" )
M4LUXOR_SET( 199?, m4luxor__f,	m4luxor,	"luxc.p1",			0x0000, 0x010000, CRC(47d1c4dc) SHA1(0856fac4a7ec14dc1df24446e1355ed05bb5f1c1), "Barcrest","Luxor (Barcrest) (MPU4) (set 7)" )
M4LUXOR_SET( 199?, m4luxor__g,	m4luxor,	"luxd.p1",			0x0000, 0x010000, CRC(8f949379) SHA1(4f0a94d06b8e7036acaae5c0c42c91481837d3a1), "Barcrest","Luxor (Barcrest) (MPU4) (set 8)" )
M4LUXOR_SET( 199?, m4luxor__h,	m4luxor,	"luxk.p1",			0x0000, 0x010000, CRC(bd5eaf2d) SHA1(f9a3f3139d6b7ff4fcec805e0ca6e8ab1c3c10dd), "Barcrest","Luxor (Barcrest) (MPU4) (set 9)" )
M4LUXOR_SET( 199?, m4luxor__i,	m4luxor,	"luxor_std.bin",	0x0000, 0x010000, CRC(2c565bf7) SHA1(61612abbda037b63e2cda7746be8cf64b4563d43), "Barcrest","Luxor (Barcrest) (MPU4) (set 10)" )
M4LUXOR_SET( 199?, m4luxor__j,	m4luxor,	"luxs.p1",			0x0000, 0x010000, CRC(78d6f05a) SHA1(53de98b9248c67c83f255d33d5963bebb757d0af), "Barcrest","Luxor (Barcrest) (MPU4) (set 11)" )
M4LUXOR_SET( 199?, m4luxor__k,	m4luxor,	"lux_05_4",			0x0000, 0x010000, CRC(335503ec) SHA1(dd03096aa98e4cac9fade6e77f9f8a8ad9a64287), "Barcrest","Luxor (Barcrest) (MPU4) (set 12)" )
M4LUXOR_SET( 199?, m4luxor__l,	m4luxor,	"lux_05_8",			0x0000, 0x010000, CRC(43a15814) SHA1(694c8c6ee695bb746391f5269f540c995fc18002), "Barcrest","Luxor (Barcrest) (MPU4) (set 13)" )
M4LUXOR_SET( 199?, m4luxor__m,	m4luxor,	"lux_10_4",			0x0000, 0x010000, CRC(122461d9) SHA1(a347c834b27a00abc1864a1e00316a491d04d84b), "Barcrest","Luxor (Barcrest) (MPU4) (set 14)" )
M4LUXOR_SET( 199?, m4luxor__n,	m4luxor,	"lux_10_8",			0x0000, 0x010000, CRC(544208e7) SHA1(85e2ff663b7500ee6bb0a900ee5ef48f7bf1934a), "Barcrest","Luxor (Barcrest) (MPU4) (set 15)" )
M4LUXOR_SET( 199?, m4luxor__o,	m4luxor,	"lux_20.4",			0x0000, 0x010000, CRC(50b3e5cc) SHA1(ff08095c01d8eeff320b5a04fe9f7e1888690cf8), "Barcrest","Luxor (Barcrest) (MPU4) (set 16)" )
M4LUXOR_SET( 199?, m4luxor__p,	m4luxor,	"lux_20_8",			0x0000, 0x010000, CRC(6c9a7152) SHA1(e38e8452e0d3f5b0e8ac51da272ab9f2e57e1d89), "Barcrest","Luxor (Barcrest) (MPU4) (set 17)" )
M4LUXOR_SET( 199?, m4luxor__q,	m4luxor,	"lx_05a__.1o1",		0x0000, 0x010000, CRC(7b81f1b9) SHA1(412a8961571f279d70c05ef26c565b4b2a588060), "Barcrest","Luxor (Barcrest) (MPU4) (set 18)" )
M4LUXOR_SET( 199?, m4luxor__r,	m4luxor,	"lx_05s__.1o1",		0x0000, 0x010000, CRC(2bf86940) SHA1(cf96a7a12db84fc028766da55ca06d2350f9d08f), "Barcrest","Luxor (Barcrest) (MPU4) (set 19)" )
M4LUXOR_SET( 199?, m4luxor__s,	m4luxor,	"lx_05sb_.1o1",		0x0000, 0x010000, CRC(e210c1b6) SHA1(023b1e0b36c4d146af5e958be72575590588b3fd), "Barcrest","Luxor (Barcrest) (MPU4) (set 20)" )
M4LUXOR_SET( 199?, m4luxor__t,	m4luxor,	"lx_05sd_.1o1",		0x0000, 0x010000, CRC(8727963a) SHA1(4585c0e3fb14f54684ff199be9010ed7b5cb97c3), "Barcrest","Luxor (Barcrest) (MPU4) (set 21)" )
M4LUXOR_SET( 199?, m4luxor__u,	m4luxor,	"lx_10a__.1o1",		0x0000, 0x010000, CRC(ce8e6c05) SHA1(b48bc01d1a069881e9b9db1a4959c7b57e80f28a), "Barcrest","Luxor (Barcrest) (MPU4) (set 22)" )
M4LUXOR_SET( 199?, m4luxor__v,	m4luxor,	"lx_10s__.1o1",		0x0000, 0x010000, CRC(9f0f5b6b) SHA1(9f67500d62921dd680bd864856206306adc3f2f6), "Barcrest","Luxor (Barcrest) (MPU4) (set 23)" )
M4LUXOR_SET( 199?, m4luxor__w,	m4luxor,	"lx_10sb_.1o1",		0x0000, 0x010000, CRC(bd020920) SHA1(a6b5c11c82344afc1cdd350b9f31d1257be72615), "Barcrest","Luxor (Barcrest) (MPU4) (set 24)" )
M4LUXOR_SET( 199?, m4luxor__x,	m4luxor,	"lx_10sd_.1o1",		0x0000, 0x010000, CRC(cc59d370) SHA1(a428d93c005b629e86810c85ea91630a354e170b), "Barcrest","Luxor (Barcrest) (MPU4) (set 25)" )
M4LUXOR_SET( 199?, m4luxor__y,	m4luxor,	"lxi05a__.1o1",		0x0000, 0x010000, CRC(7a5fe065) SHA1(c44b41d01175c10051ae4cd1453be3411842825e), "Barcrest","Luxor (Barcrest) (MPU4) (set 26)" )
M4LUXOR_SET( 199?, m4luxor__z,	m4luxor,	"lxi10a__.1o1",		0x0000, 0x010000, CRC(17989464) SHA1(67aa9cc01d89ed4caeb33885f53dcaee762ccb6d), "Barcrest","Luxor (Barcrest) (MPU4) (set 27)" )


#define M4HIJINX_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "hijinx1.hex", 0x000000, 0x080000, CRC(8d5afedb) SHA1(6bf6dadddf8dd3672e3d05167ab9a0793c269176) ) \
	ROM_LOAD( "hijinx2.hex", 0x080000, 0x080000, CRC(696c8a92) SHA1(d54a1020fea80bacb678bc8bd6b7d4d0854af603) ) \

#define M4HIJINX_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4HIJINX_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4HIJINX_SET( 199?, m4hijinx,		0,			"jnx10y.p1",	0x0000, 0x020000, CRC(792b3bae) SHA1(d30aecce42953f1ec49753cc2d1df00ad9bd088f), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 1)" )
M4HIJINX_SET( 199?, m4hijinx__a,	m4hijinx,	"hij15g",		0x0000, 0x020000, CRC(73271cca) SHA1(8177e10e30386464a7e5a33dc3c02adbf4c93101), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 2)" )
M4HIJINX_SET( 199?, m4hijinx__b,	m4hijinx,	"hij15t",		0x0000, 0x020000, CRC(c7d54c64) SHA1(d3537c8412b583f2812f87ab68ac8855e9cdbd2f), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 3)" )
M4HIJINX_SET( 199?, m4hijinx__c,	m4hijinx,	"jns02ad.p1",	0x0000, 0x020000, CRC(436a6632) SHA1(25ab01b8785cf6f9d2316b15d2d2887e898358de), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 4)" )
M4HIJINX_SET( 199?, m4hijinx__d,	m4hijinx,	"jns02b.p1",	0x0000, 0x020000, CRC(171b8941) SHA1(8383f0ce3c21b187f031302b4d930e13c131f862), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 5)" )
M4HIJINX_SET( 199?, m4hijinx__e,	m4hijinx,	"jns02bd.p1",	0x0000, 0x020000, CRC(6a8b03ba) SHA1(ecc6826474a96a7d2e5cee851d672b954906bfbe), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 6)" )
M4HIJINX_SET( 199?, m4hijinx__f,	m4hijinx,	"jns02d.p1",	0x0000, 0x020000, CRC(3ca43ce0) SHA1(d369cc103ce1c506e85cdce87ac7836c99b03df6), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 7)" )
M4HIJINX_SET( 199?, m4hijinx__g,	m4hijinx,	"jns02dh.p1",	0x0000, 0x020000, CRC(84b760a1) SHA1(2312a8177c474d5bd402e5d8039e37430a7c37ae), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 8)" )
M4HIJINX_SET( 199?, m4hijinx__h,	m4hijinx,	"jns02dk.p1",	0x0000, 0x020000, CRC(3ee77376) SHA1(b19aea9254b9315c805e74a7d20dc626254a4834), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 9)" )
M4HIJINX_SET( 199?, m4hijinx__i,	m4hijinx,	"jns02dr.p1",	0x0000, 0x020000, CRC(f065dd03) SHA1(b7f3352b0807f30d4eebfca1c276bd4a53ede632), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 10)" )
M4HIJINX_SET( 199?, m4hijinx__j,	m4hijinx,	"jns02dy.p1",	0x0000, 0x020000, CRC(09adce64) SHA1(09a671ef097dbd4233aca241976c23375b4af789), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 11)" )
M4HIJINX_SET( 199?, m4hijinx__k,	m4hijinx,	"jns02h.p1",	0x0000, 0x020000, CRC(f927ea5a) SHA1(c1af9f2b20421c66d2141b85a61cd51e8cb6a67b), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 12)" )
M4HIJINX_SET( 199?, m4hijinx__l,	m4hijinx,	"jns02k.p1",	0x0000, 0x020000, CRC(4377f98d) SHA1(0e7b4e655acec07fea221697179045f06d4f3c48), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 13)" )
M4HIJINX_SET( 199?, m4hijinx__m,	m4hijinx,	"jns02r.p1",	0x0000, 0x020000, CRC(8df557f8) SHA1(899e73b265065f09eac785e45e06fd755a618c21), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 14)" )
M4HIJINX_SET( 199?, m4hijinx__n,	m4hijinx,	"jns02s.p1",	0x0000, 0x020000, CRC(42df2639) SHA1(8ed6addfc85cfeab4c5f03c24a692a9c392a8bc2), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 15)" )
M4HIJINX_SET( 199?, m4hijinx__o,	m4hijinx,	"jns02y.p1",	0x0000, 0x020000, CRC(743d449f) SHA1(739e41b28ee53465b40138cdccf0fcd1782c8b45), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 16)" )
M4HIJINX_SET( 199?, m4hijinx__p,	m4hijinx,	"jns03ad.p1",	0x0000, 0x020000, CRC(9d3e4a13) SHA1(5780eaeb148d64f7d2769207f7973c02a5e5a3de), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 17)" )
M4HIJINX_SET( 199?, m4hijinx__q,	m4hijinx,	"jns03b.p1",	0x0000, 0x020000, CRC(07eddb71) SHA1(05f5bbcf6c7e69407163c9dc76c6c587f0cdf10e), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 18)" )
M4HIJINX_SET( 199?, m4hijinx__r,	m4hijinx,	"jns03bd.p1",	0x0000, 0x020000, CRC(b4df2f9b) SHA1(1d6025bfb119007c7b17214577cf69f978fa2830), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 19)" )
M4HIJINX_SET( 199?, m4hijinx__s,	m4hijinx,	"jns03d.p1",	0x0000, 0x020000, CRC(2c526ed0) SHA1(99f7b144ca6a3924c2b5b5f38ac09cec12f20486), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 20)" )
M4HIJINX_SET( 199?, m4hijinx__t,	m4hijinx,	"jns03dh.p1",	0x0000, 0x020000, CRC(5ae34c80) SHA1(7fd217c4c251c2506fd41f4c6173ffb12a680ba8), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 21)" )
M4HIJINX_SET( 199?, m4hijinx__u,	m4hijinx,	"jns03dk.p1",	0x0000, 0x020000, CRC(e0b35f57) SHA1(06a322e2ad37bf4f3c7959fdcb213f95c414a16d), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 22)" )
M4HIJINX_SET( 199?, m4hijinx__v,	m4hijinx,	"jns03dr.p1",	0x0000, 0x020000, CRC(2e31f122) SHA1(f00b9d47314b270a8e222a46cb1493387bebbd84), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 23)" )
M4HIJINX_SET( 199?, m4hijinx__w,	m4hijinx,	"jns03dy.p1",	0x0000, 0x020000, CRC(d7f9e245) SHA1(1605efd18a9234e72a13c4df362e1007fc443a23), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 24)" )
M4HIJINX_SET( 199?, m4hijinx__x,	m4hijinx,	"jns03h.p1",	0x0000, 0x020000, CRC(e9d1b86a) SHA1(4a982370bfb788ef53ae9479a72dbca6c7f9ec00), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 25)" )
M4HIJINX_SET( 199?, m4hijinx__y,	m4hijinx,	"jns03k.p1",	0x0000, 0x020000, CRC(5381abbd) SHA1(8967579f31ef9a0ec868af1cb64d9fa049314f94), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 26)" )
M4HIJINX_SET( 199?, m4hijinx__z,	m4hijinx,	"jns03r.p1",	0x0000, 0x020000, CRC(9d0305c8) SHA1(525b36b43a09c27f807dcbdc8bff89af82bdffc0), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 27)" )
M4HIJINX_SET( 199?, m4hijinx__0,	m4hijinx,	"jns03s.p1",	0x0000, 0x020000, CRC(ef9f3d18) SHA1(cc2239a4dca1c092025216a16bb39fe9126bd6f8), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 28)" )
M4HIJINX_SET( 199?, m4hijinx__1,	m4hijinx,	"jns03y.p1",	0x0000, 0x020000, CRC(64cb16af) SHA1(f4bc3557f84cd0054c475e0354f0562f63df3146), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 29)" )
M4HIJINX_SET( 199?, m4hijinx__2,	m4hijinx,	"jnx05d.p1",	0x0000, 0x020000, CRC(27d9db28) SHA1(df333d884735a94d3c4d460deb05e50fcfce8896), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 30)" )
M4HIJINX_SET( 199?, m4hijinx__3,	m4hijinx,	"jnx05dh.p1",	0x0000, 0x020000, CRC(d7dc5d8b) SHA1(6b94ae9169b522a80455cbe42378427d2f1019f3), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 31)" )
M4HIJINX_SET( 199?, m4hijinx__4,	m4hijinx,	"jnx05dy.p1",	0x0000, 0x020000, CRC(f44fafc0) SHA1(996411b41071cc3c5c84f00106f9685b27e95352), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 32)" )
M4HIJINX_SET( 199?, m4hijinx__5,	m4hijinx,	"jnx05h.p1",	0x0000, 0x020000, CRC(4cd3511c) SHA1(2f283f50d2313eb9da49e4efbdb9a098f90c0afb), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 33)" )
M4HIJINX_SET( 199?, m4hijinx__6,	m4hijinx,	"jnx05y.p1",	0x0000, 0x020000, CRC(6f40a357) SHA1(7d019f925d7920df23782b1e6e742bc467e7767b), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 34)" )
M4HIJINX_SET( 199?, m4hijinx__7,	m4hijinx,	"jnx10d.p1",	0x0000, 0x020000, CRC(31b243d1) SHA1(029dd8ecbe83b63ca799b6507262193ef56d4b36), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 35)" )
M4HIJINX_SET( 199?, m4hijinx__8,	m4hijinx,	"jnx10dh.p1",	0x0000, 0x020000, CRC(ffc421b0) SHA1(28439c5d2ec371edae5e7b84e1da96d468bb8556), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 36)" )
M4HIJINX_SET( 199?, m4hijinx__9,	m4hijinx,	"jnx10dy.p1",	0x0000, 0x020000, CRC(dc57d3fb) SHA1(e622485f37f638a69e0c5dbb06faa519b10eafc9), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 37)" )
M4HIJINX_SET( 199?, m4hijinx__aa,	m4hijinx,	"jnx10h.p1",	0x0000, 0x020000, CRC(5ab8c9e5) SHA1(f12094bd95369288a76dac9d1e62810fa478cfd6), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 38)" )
M4HIJINX_SET( 199?, m4hijinx__ab,	m4hijinx,	"jnx10s.p1",	0x0000, 0x020000, CRC(a291147e) SHA1(818172bab2fad210a937d91e0be4ddf165f1cf99), "Barcrest","Hi Jinx (Barcrest) (MPU4) (set 39)" )




#define M4CASHLN_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "altmsm6376", 0 ) \
	ROM_LOAD( "cash lines 0.1 snd 1 9c3f.bin", 0x0000, 0x080000, CRC(1746f091) SHA1(d57fcaec3e3b0344671f2c984974bfdac50ec3d7) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "cls1.hex", 0x000000, 0x080000, CRC(d6b5d862) SHA1(eab2ef2999229db7182896267cd83742b2390237) ) \
	ROM_LOAD( "cls2.hex", 0x080000, 0x080000, CRC(e42e674b) SHA1(1cda06425f3d4797ee0c4ff7426970150e5af4b6) ) \

#define M4CASHLN_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4CASHLN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4CASHLN_SET( 199?, m4cashln,		0,			"cls04s.p1",	0x0000, 0x020000, CRC(c8b7f355) SHA1(437324bf499ba49ecbb3854f5f787da5f575f7f5), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 1)" )
M4CASHLN_SET( 199?, m4cashln__a,	m4cashln,	"cl15g",		0x0000, 0x020000, CRC(fdd5765d) SHA1(fee8ddc9b93934a5582d6730cfa26246191c22ff), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 2)" )
M4CASHLN_SET( 199?, m4cashln__b,	m4cashln,	"cl15t",		0x0000, 0x020000, CRC(56bb9f21) SHA1(2876ac79283ea5cbee45e9ac6d5d140ea7db8e95), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 3)" )
M4CASHLN_SET( 199?, m4cashln__c,	m4cashln,	"cli10s",		0x0000, 0x020000, CRC(9aca737d) SHA1(6669c8b7a192b1c67caad62aad528b08737f7e73), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 4)" )
M4CASHLN_SET( 199?, m4cashln__d,	m4cashln,	"cli11ad.p1",	0x0000, 0x020000, CRC(ab47d9b3) SHA1(e501bb61de76ffe6618be03a59614b36e71e031b), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 5)" )
M4CASHLN_SET( 199?, m4cashln__e,	m4cashln,	"cli11b.p1",	0x0000, 0x020000, CRC(4adcaa58) SHA1(027dda275f32e49e6c0f9ff4e7d42472bdd3174e), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 6)" )
M4CASHLN_SET( 199?, m4cashln__f,	m4cashln,	"cli11bd.p1",	0x0000, 0x020000, CRC(e18071e5) SHA1(e219be249b5992808745a4c668686dfad77e2837), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 7)" )
M4CASHLN_SET( 199?, m4cashln__g,	m4cashln,	"cli11d.p1",	0x0000, 0x020000, CRC(c7c6049d) SHA1(6ad1caed1272ff4e6902843bf2420020ca2d0cb4), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 8)" )
M4CASHLN_SET( 199?, m4cashln__h,	m4cashln,	"cli11dh.p1",	0x0000, 0x020000, CRC(c21383ae) SHA1(01a840f160aacbb2b253ab6ba0a915f7055cc381), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 9)" )
M4CASHLN_SET( 199?, m4cashln__i,	m4cashln,	"cli11dk.p1",	0x0000, 0x020000, CRC(18486282) SHA1(5910a68539f5343661f8e55105c767067633ef1c), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 10)" )
M4CASHLN_SET( 199?, m4cashln__j,	m4cashln,	"cli11dr.p1",	0x0000, 0x020000, CRC(82a6bc3b) SHA1(c12b46234bebe408374a7fc3e18bf79942434d95), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 11)" )
M4CASHLN_SET( 199?, m4cashln__k,	m4cashln,	"cli11dy.p1",	0x0000, 0x020000, CRC(2ca4f94d) SHA1(4d0345e3c819d8dd48c559f01c118430d5ccdea2), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 12)" )
M4CASHLN_SET( 199?, m4cashln__l,	m4cashln,	"cli11h.p1",	0x0000, 0x020000, CRC(694f5813) SHA1(3acf84d6e5060e7054097aa2601c9c2833a2d524), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 13)" )
M4CASHLN_SET( 199?, m4cashln__m,	m4cashln,	"cli11k.p1",	0x0000, 0x020000, CRC(b314b93f) SHA1(e94472e1d1e345be84b277088dc852ed75117344), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 14)" )
M4CASHLN_SET( 199?, m4cashln__n,	m4cashln,	"cli11r.p1",	0x0000, 0x020000, CRC(29fa6786) SHA1(8bd7a7b0cf84615a126ebed494954ed2b7bc0ec4), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 15)" )
M4CASHLN_SET( 199?, m4cashln__o,	m4cashln,	"cli11s.p1",	0x0000, 0x020000, CRC(e5ad1734) SHA1(69fb5c81ae04c98920d84f829be14983168196e5), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 16)" )
M4CASHLN_SET( 199?, m4cashln__p,	m4cashln,	"cli11y.p1",	0x0000, 0x020000, CRC(87f822f0) SHA1(701a8c88be972b8363490e92e98f37acd493ef07), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 17)" )
M4CASHLN_SET( 199?, m4cashln__q,	m4cashln,	"cli12ad.p1",	0x0000, 0x020000, CRC(d6c03a42) SHA1(203ea61def95c2ea0a9ea1f808056122d87993ff), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 18)" )
M4CASHLN_SET( 199?, m4cashln__r,	m4cashln,	"cli12b.p1",	0x0000, 0x020000, CRC(9d4245da) SHA1(604b1e68d271784b982cde81e3675298662df1bc), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 19)" )
M4CASHLN_SET( 199?, m4cashln__s,	m4cashln,	"cli12bd.p1",	0x0000, 0x020000, CRC(9c079214) SHA1(1ee6362d876dd5aaa1699186ee905b48755a80de), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 20)" )
M4CASHLN_SET( 199?, m4cashln__t,	m4cashln,	"cli12d.p1",	0x0000, 0x020000, CRC(1058eb1f) SHA1(2c4305679c829cbf6c34d2627a1e8e366c5be5c1), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 21)" )
M4CASHLN_SET( 199?, m4cashln__u,	m4cashln,	"cli12dh.p1",	0x0000, 0x020000, CRC(bf94605f) SHA1(e2ae87041791ba92d8db69c7a3b40b62c97cf941), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 22)" )
M4CASHLN_SET( 199?, m4cashln__v,	m4cashln,	"cli12dk.p1",	0x0000, 0x020000, CRC(65cf8173) SHA1(f7ed6227b3e57d50e327f3ff72b9aafafa51da63), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 23)" )
M4CASHLN_SET( 199?, m4cashln__w,	m4cashln,	"cli12dr.p1",	0x0000, 0x020000, CRC(ff215fca) SHA1(40cab82f6a7f5229b365236b4758e73b2a3dce1e), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 24)" )
M4CASHLN_SET( 199?, m4cashln__x,	m4cashln,	"cli12dy.p1",	0x0000, 0x020000, CRC(51231abc) SHA1(ae80491fb1496f6fccbd46f424fe1d45640da8e2), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 25)" )
M4CASHLN_SET( 199?, m4cashln__y,	m4cashln,	"cli12h.p1",	0x0000, 0x020000, CRC(bed1b791) SHA1(d017f62a099475786fa15c7a185574301b80dbdd), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 26)" )
M4CASHLN_SET( 199?, m4cashln__z,	m4cashln,	"cli12k.p1",	0x0000, 0x020000, CRC(648a56bd) SHA1(25ad92789b12512b8c65cc912080f62abfe101a6), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 27)" )
M4CASHLN_SET( 199?, m4cashln__0,	m4cashln,	"cli12r.p1",	0x0000, 0x020000, CRC(fe648804) SHA1(27e74aea209b90dc3d8cf46474e271d68d9af7e2), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 28)" )
M4CASHLN_SET( 199?, m4cashln__1,	m4cashln,	"cli12s.p1",	0x0000, 0x020000, CRC(ef5aa578) SHA1(c4c288a297f5f6cd0712c396237aa3bf1363e188), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 29)" )
M4CASHLN_SET( 199?, m4cashln__2,	m4cashln,	"cli12y.p1",	0x0000, 0x020000, CRC(5066cd72) SHA1(03cef55b4fff8fb6edd804fdbc4076db6b234614), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 30)" )
M4CASHLN_SET( 199?, m4cashln__3,	m4cashln,	"cls03ad.p1",	0x0000, 0x020000, CRC(c68249cf) SHA1(d2d16ce76a5b144827a11f7fa471c7ea558c1ce0), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 31)" )
M4CASHLN_SET( 199?, m4cashln__4,	m4cashln,	"cls03b.p1",	0x0000, 0x020000, CRC(db667c56) SHA1(8a8f2374d0d02307206071718376400d5a52dc6c), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 32)" )
M4CASHLN_SET( 199?, m4cashln__5,	m4cashln,	"cls03bd.p1",	0x0000, 0x020000, CRC(4b98e70a) SHA1(04e0732dd8c0283dd928da9caafd78f509a4d479), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 33)" )
M4CASHLN_SET( 199?, m4cashln__6,	m4cashln,	"cls03c.p1",	0x0000, 0x020000, CRC(ec2cc144) SHA1(bc2879e4487ad5638e2818f4c8c5b23ab660cecc), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 34)" )
M4CASHLN_SET( 199?, m4cashln__7,	m4cashln,	"cls03d.p1",	0x0000, 0x020000, CRC(a67d7720) SHA1(556128a8c464a5d1de0ebb0de8ce87a7ea0813d3), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 35)" )
M4CASHLN_SET( 199?, m4cashln__8,	m4cashln,	"cls03dh.p1",	0x0000, 0x020000, CRC(22587ac6) SHA1(04cacccfebdc01046e048b2c98a05f3f97fcd6f3), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 36)" )
M4CASHLN_SET( 199?, m4cashln__9,	m4cashln,	"cls03dk.p1",	0x0000, 0x020000, CRC(015f4f5c) SHA1(fd0e622217d42f52cfba78989e8cf843cb08a17d), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 37)" )
M4CASHLN_SET( 199?, m4cashln__aa,	m4cashln,	"cls03dr.p1",	0x0000, 0x020000, CRC(b250f46d) SHA1(02f04778ce6fb674ba9760b0fa9828b76a58239a), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 38)" )
M4CASHLN_SET( 199?, m4cashln__ab,	m4cashln,	"cls03dy.p1",	0x0000, 0x020000, CRC(86bc6fa2) SHA1(40a2bb1148989b5895b0f58417f404f7b035b472), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 39)" )
M4CASHLN_SET( 199?, m4cashln__ac,	m4cashln,	"cls03h.p1",	0x0000, 0x020000, CRC(b2a6e19a) SHA1(668527a7939ab70deb03b1db8a4a2629fb332815), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 40)" )
M4CASHLN_SET( 199?, m4cashln__ad,	m4cashln,	"cls03k.p1",	0x0000, 0x020000, CRC(91a1d400) SHA1(9e4ccd4f4119471d66a22b14a22312149faf28c0), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 41)" )
M4CASHLN_SET( 199?, m4cashln__ae,	m4cashln,	"cls03r.p1",	0x0000, 0x020000, CRC(22ae6f31) SHA1(b08ec5eb5bae377b829a14065b54ec1dbbc55677), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 42)" )
M4CASHLN_SET( 199?, m4cashln__af,	m4cashln,	"cls03s.p1",	0x0000, 0x020000, CRC(cb9a86b2) SHA1(2b4aee61c0070d295ba81ffa5739ceb8e05dc0e8), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 43)" )
M4CASHLN_SET( 199?, m4cashln__ag,	m4cashln,	"cls03y.p1",	0x0000, 0x020000, CRC(1642f4fe) SHA1(b4e3fa360fdc1908cafe61d833c5097cee965404), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 44)" )
M4CASHLN_SET( 199?, m4cashln__ah,	m4cashln,	"cls04ad.p1",	0x0000, 0x020000, CRC(6eb174ed) SHA1(dcf4e91ca16e3d5644429289470329102bc96f83), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 45)" )
M4CASHLN_SET( 199?, m4cashln__ai,	m4cashln,	"cls04b.p1",	0x0000, 0x020000, CRC(73554174) SHA1(f74ba0a6212c58306d3e4f2e467e860bd4b2b294), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 46)" )
M4CASHLN_SET( 199?, m4cashln__aj,	m4cashln,	"cls04bd.p1",	0x0000, 0x020000, CRC(e3abda28) SHA1(65b7883b52a8105f3df12659f4d9467f503ac1c6), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 47)" )
M4CASHLN_SET( 199?, m4cashln__ak,	m4cashln,	"cls04c.p1",	0x0000, 0x020000, CRC(441ffc66) SHA1(d56f8e47a17de84d63c580bd1100dd53cc90071c), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 48)" )
M4CASHLN_SET( 199?, m4cashln__al,	m4cashln,	"cls04d.p1",	0x0000, 0x020000, CRC(0e4e4a02) SHA1(94cf99d072731bd82902d6237d226e7872f1aa69), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 49)" )
M4CASHLN_SET( 199?, m4cashln__am,	m4cashln,	"cls04dh.p1",	0x0000, 0x020000, CRC(8a6b47e4) SHA1(ba7b0810375b650fa4c503dc9d3504a6297ca2b5), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 50)" )
M4CASHLN_SET( 199?, m4cashln__an,	m4cashln,	"cls04dk.p1",	0x0000, 0x020000, CRC(a96c727e) SHA1(4c9dfc007f69231cd31f166ba0eea87ecbfe4abc), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 51)" )
M4CASHLN_SET( 199?, m4cashln__ao,	m4cashln,	"cls04dr.p1",	0x0000, 0x020000, CRC(1a63c94f) SHA1(83ea53bfae911bbc1a2659b0a20ba2218849fdf0), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 52)" )
M4CASHLN_SET( 199?, m4cashln__ap,	m4cashln,	"cls04dy.p1",	0x0000, 0x020000, CRC(2e8f5280) SHA1(453a9012174bd6715abd74c7be25e882f8209ee3), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 53)" )
M4CASHLN_SET( 199?, m4cashln__aq,	m4cashln,	"cls04h.p1",	0x0000, 0x020000, CRC(1a95dcb8) SHA1(0926a6381e3426a18587d05430ad3fbe3ef9a0be), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 54)" )
M4CASHLN_SET( 199?, m4cashln__ar,	m4cashln,	"cls04k.p1",	0x0000, 0x020000, CRC(3992e922) SHA1(9c39d2740ef51004ea59f859ec3e008c6586c00d), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 55)" )
M4CASHLN_SET( 199?, m4cashln__as,	m4cashln,	"cls04r.p1",	0x0000, 0x020000, CRC(8a9d5213) SHA1(dce36cdf790415bb37735a09226d73e322f7510a), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 56)" )
M4CASHLN_SET( 199?, m4cashln__at,	m4cashln,	"cls04y.p1",	0x0000, 0x020000, CRC(be71c9dc) SHA1(d2c2374685e953a028726ba48329196aa0a9f098), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 57)" )
M4CASHLN_SET( 199?, m4cashln__au,	m4cashln,	"ncc10ad.p1",	0x0000, 0x020000, CRC(d4be9280) SHA1(cbae1aad2dc8d88df9869755063a7a1097995417), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 58)" )
M4CASHLN_SET( 199?, m4cashln__av,	m4cashln,	"ncc10b.p1",	0x0000, 0x020000, CRC(42abdedf) SHA1(4c428288ba9bb426ae011127896634f5995f8a6c), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 59)" )
M4CASHLN_SET( 199?, m4cashln__aw,	m4cashln,	"ncc10bd.p1",	0x0000, 0x020000, CRC(9e793ad6) SHA1(7c26c6cf2b23e2b73887c2eea1e5f7566127ca95), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 60)" )
M4CASHLN_SET( 199?, m4cashln__ax,	m4cashln,	"ncc10d.p1",	0x0000, 0x020000, CRC(cfb1701a) SHA1(20c47f0d30c4e4da37ce38fc097e841fb3ee89ed), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 61)" )
M4CASHLN_SET( 199?, m4cashln__ay,	m4cashln,	"ncc10dh.p1",	0x0000, 0x020000, CRC(bdeac89d) SHA1(5ddde5cee5c95714e66e10a061e76603a993f446), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 62)" )
M4CASHLN_SET( 199?, m4cashln__az,	m4cashln,	"ncc10dk.p1",	0x0000, 0x020000, CRC(67b129b1) SHA1(d7c0a107295ae3d11abe4138a1c76fd779777bec), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 63)" )
M4CASHLN_SET( 199?, m4cashln__a0,	m4cashln,	"ncc10dr.p1",	0x0000, 0x020000, CRC(fd5ff708) SHA1(1bc5b96a46130370fc16b43804721eb392ec585f), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 64)" )
M4CASHLN_SET( 199?, m4cashln__a1,	m4cashln,	"ncc10dy.p1",	0x0000, 0x020000, CRC(535db27e) SHA1(16b8d87fc22e040952988823d2c677130c17e137), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 65)" )
M4CASHLN_SET( 199?, m4cashln__a2,	m4cashln,	"ncc10h.p1",	0x0000, 0x020000, CRC(61382c94) SHA1(98e4fcfedbe16773be0abbd6691ccd9f19a33684), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 66)" )
M4CASHLN_SET( 199?, m4cashln__a3,	m4cashln,	"ncc10k.p1",	0x0000, 0x020000, CRC(bb63cdb8) SHA1(d4737ab4492bcdc0ffa3eb1cb0456f4f32e4482a), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 67)" )
M4CASHLN_SET( 199?, m4cashln__a4,	m4cashln,	"ncc10r.p1",	0x0000, 0x020000, CRC(218d1301) SHA1(4bbdb69c2ac6ee13b50b1de8a17b019ce46004ba), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 68)" )
M4CASHLN_SET( 199?, m4cashln__a5,	m4cashln,	"ncc10s.p1",	0x0000, 0x020000, CRC(2a18dc72) SHA1(f2434805212719db22ce163f2b338b25ca275c94), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 69)" )
M4CASHLN_SET( 199?, m4cashln__a6,	m4cashln,	"ncc10y.p1",	0x0000, 0x020000, CRC(8f8f5677) SHA1(188d5d9c274147367bde644e33e162d0541cdca2), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 70)" )
M4CASHLN_SET( 199?, m4cashln__a7,	m4cashln,	"ncl11ad.p1",	0x0000, 0x020000, CRC(73ba0558) SHA1(d86b2469d7cbe5f9271f8e8f3c6ee1659d72b3c6), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 71)" )
M4CASHLN_SET( 199?, m4cashln__a8,	m4cashln,	"ncl11b.p1",	0x0000, 0x020000, CRC(f6e8ee46) SHA1(6d0da96b6cba4482c254fef6ba44a4e80fc0fda4), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 72)" )
M4CASHLN_SET( 199?, m4cashln__a9,	m4cashln,	"ncl11bd.p1",	0x0000, 0x020000, CRC(397dad0e) SHA1(ad764f376b34f01578fc2ce78351f89642601252), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 73)" )
M4CASHLN_SET( 199?, m4cashln__ba,	m4cashln,	"ncl11d.p1",	0x0000, 0x020000, CRC(7bf24083) SHA1(eec5d8b279b751a432ed8a2635ba6ecb519ef985), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 74)" )
M4CASHLN_SET( 199?, m4cashln__bb,	m4cashln,	"ncl11dh.p1",	0x0000, 0x020000, CRC(1aee5f45) SHA1(1de8bc30925f7bc8c3363d680a07434875aee1d1), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 75)" )
M4CASHLN_SET( 199?, m4cashln__bc,	m4cashln,	"ncl11dk.p1",	0x0000, 0x020000, CRC(c0b5be69) SHA1(b7c808da29f021a056eb268b1910f60cb570a0a7), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 76)" )
M4CASHLN_SET( 199?, m4cashln__bd,	m4cashln,	"ncl11dr.p1",	0x0000, 0x020000, CRC(5a5b60d0) SHA1(634533386174cc77825c4def1c4524c20d665b30), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 77)" )
M4CASHLN_SET( 199?, m4cashln__be,	m4cashln,	"ncl11dy.p1",	0x0000, 0x020000, CRC(f45925a6) SHA1(49b7b949898d886431529f76a25cf4d2af3130f6), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 78)" )
M4CASHLN_SET( 199?, m4cashln__bf,	m4cashln,	"ncl11h.p1",	0x0000, 0x020000, CRC(d57b1c0d) SHA1(cfb0e78ecba28a5ebe6e272190c473023544452b), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 79)" )
M4CASHLN_SET( 199?, m4cashln__bg,	m4cashln,	"ncl11k.p1",	0x0000, 0x020000, CRC(0f20fd21) SHA1(25c3a7fe77819613de7bf4218759b096a05e5605), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 80)" )
M4CASHLN_SET( 199?, m4cashln__bh,	m4cashln,	"ncl11r.p1",	0x0000, 0x020000, CRC(95ce2398) SHA1(72aa7ec767a81c568b442f63c3cf916925b22a4a), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 81)" )
M4CASHLN_SET( 199?, m4cashln__bi,	m4cashln,	"ncl11s.p1",	0x0000, 0x020000, CRC(06ae30c0) SHA1(eb7fde45e0a0aa08f3c788f581b48adc8ee86a79), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 82)" )
M4CASHLN_SET( 199?, m4cashln__bj,	m4cashln,	"ncl11y.p1",	0x0000, 0x020000, CRC(3bcc66ee) SHA1(795ecf1e34ae44d7aea70512124b66b0bed3e875), "Barcrest","Cash Lines (Barcrest) (MPU4) (set 83)" )



#define M4LUCKLV_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "llvsnd.p1", 0x000000, 0x080000, CRC(36be26f3) SHA1(b1c66d3ebebd7eb18266bf6b30c4a4db7acdf10d) ) \
	ROM_LOAD( "llvsnd.p2", 0x080000, 0x080000, CRC(d5c2bb99) SHA1(e2096b8a33e89218d44200e87d1962790120a96c) ) \

#define M4LUCKLV_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4LUCKLV_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4LUCKLV_SET( 199?, m4lucklv,		0,			"llvs.p1",	0x0000, 0x010000, CRC(30727bc9) SHA1(c32112d0181f629540b31ce9959834111dbf7e0e), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 1)" )
M4LUCKLV_SET( 199?, m4lucklv__a,	m4lucklv,	"ll3ad.p1",	0x0000, 0x010000, CRC(e79e7f98) SHA1(7b3a22978f2f5a0b6062f0330fef15ce0e91c010), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 2)" )
M4LUCKLV_SET( 199?, m4lucklv__b,	m4lucklv,	"ll3b.p1",	0x0000, 0x010000, CRC(bcbbe728) SHA1(4930419e0e524a91687386e8a2fce2150cd8a172), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 3)" )
M4LUCKLV_SET( 199?, m4lucklv__c,	m4lucklv,	"ll3bd.p1",	0x0000, 0x010000, CRC(aa4e9e1e) SHA1(ebacd42049916a32d1738813d544e34507dd650a), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 4)" )
M4LUCKLV_SET( 199?, m4lucklv__d,	m4lucklv,	"ll3d.p1",	0x0000, 0x010000, CRC(c968fe37) SHA1(8a1b79928a86b6ba5449f0a454e5115fd37e3d55), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 5)" )
M4LUCKLV_SET( 199?, m4lucklv__e,	m4lucklv,	"ll3dk.p1",	0x0000, 0x010000, CRC(c03baaef) SHA1(f3c4394d0c9929d439551aa61784a51f67e2dc3a), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 6)" )
M4LUCKLV_SET( 199?, m4lucklv__f,	m4lucklv,	"ll3dy.p1",	0x0000, 0x010000, CRC(e73fa943) SHA1(3b2a372028ffa5200510e976cd4e8eba8e6c0612), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 7)" )
M4LUCKLV_SET( 199?, m4lucklv__g,	m4lucklv,	"ll3k.p1",	0x0000, 0x010000, CRC(2519ede2) SHA1(d9e4b57824cddc04d1166cee16c309a77ff510d6), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 8)" )
M4LUCKLV_SET( 199?, m4lucklv__h,	m4lucklv,	"ll3s.p1",	0x0000, 0x010000, CRC(fdda2e78) SHA1(f68b274b6af7b44347b8f684f6e8a9342d222590), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 9)" )
M4LUCKLV_SET( 199?, m4lucklv__i,	m4lucklv,	"ll3y.p1",	0x0000, 0x010000, CRC(195023d4) SHA1(a4fabaa44fa76c4e77fb40bb89c65dadeab5927c), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 10)" )
M4LUCKLV_SET( 199?, m4lucklv__j,	m4lucklv,	"ll8ad.p1",	0x0000, 0x010000, CRC(08f307ef) SHA1(1a613d8d2cbadffe1a97a7e2c3eafc709e6895c7), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 11)" )
M4LUCKLV_SET( 199?, m4lucklv__k,	m4lucklv,	"ll8b.p1",	0x0000, 0x010000, CRC(7dd12d38) SHA1(4466bca407500741e74dfec07361d030c1736c18), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 12)" )
M4LUCKLV_SET( 199?, m4lucklv__l,	m4lucklv,	"ll8bd.p1",	0x0000, 0x010000, CRC(3eef6aa5) SHA1(e01b95ca085a326207ef2ada1d5f31745b9c15c9), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 13)" )
M4LUCKLV_SET( 199?, m4lucklv__m,	m4lucklv,	"ll8c.p1",	0x0000, 0x010000, CRC(a2de47ff) SHA1(bc4179749474fec1963314dddac69f7f1cec1ce3), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 14)" )
M4LUCKLV_SET( 199?, m4lucklv__n,	m4lucklv,	"ll8d.p1",	0x0000, 0x010000, CRC(b6385af0) SHA1(e32251b4c7c0db4957f87a9eb3f6c07efd66142c), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 15)" )
M4LUCKLV_SET( 199?, m4lucklv__o,	m4lucklv,	"ll8dk.p1",	0x0000, 0x010000, CRC(f325c2af) SHA1(25a0818776c0cf4edcd2678fe457e21bf10dfe61), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 16)" )
M4LUCKLV_SET( 199?, m4lucklv__p,	m4lucklv,	"ll8dy.p1",	0x0000, 0x010000, CRC(10b33cc3) SHA1(fe0a41b98b1f3dde82365c30236586737c5fdb9d), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 17)" )
M4LUCKLV_SET( 199?, m4lucklv__q,	m4lucklv,	"ll8k.p1",	0x0000, 0x010000, CRC(d7c284be) SHA1(c33dbfa15ead02a581b4a874a6981e6094537fb4), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 18)" )
M4LUCKLV_SET( 199?, m4lucklv__r,	m4lucklv,	"ll8s.p1",	0x0000, 0x010000, CRC(1448c6fe) SHA1(211627a0f397658a8241c5e4e138a1a609beaabe), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 19)" )
M4LUCKLV_SET( 199?, m4lucklv__s,	m4lucklv,	"ll8y.p1",	0x0000, 0x010000, CRC(eb8b4a88) SHA1(f0de0d50de848e7fea35ebe754f20eaa1c02f295), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 20)" )
M4LUCKLV_SET( 199?, m4lucklv__t,	m4lucklv,	"lltad.p1",	0x0000, 0x010000, CRC(35e43f52) SHA1(4ca2c43c59f5c5c3f53527bbf8128f0e92a3ca5c), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 21)" )
M4LUCKLV_SET( 199?, m4lucklv__u,	m4lucklv,	"lltb.p1",	0x0000, 0x010000, CRC(ddc19b4e) SHA1(b44a56e5d543d895e37986d19649d7afa8d3d238), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 22)" )
M4LUCKLV_SET( 199?, m4lucklv__v,	m4lucklv,	"lltbd.p1",	0x0000, 0x010000, CRC(fe25aa83) SHA1(68ccae76938af64e0c5df0e56333fd6814734779), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 23)" )
M4LUCKLV_SET( 199?, m4lucklv__w,	m4lucklv,	"lltd.p1",	0x0000, 0x010000, CRC(200930a6) SHA1(5dc054231402b43ebb980d190cde20c3b89b0f13), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 24)" )
M4LUCKLV_SET( 199?, m4lucklv__x,	m4lucklv,	"lltdk.p1",	0x0000, 0x010000, CRC(02969e73) SHA1(a4344e198d8aba3ba85325de7eb650afdd1a491c), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 25)" )
M4LUCKLV_SET( 199?, m4lucklv__y,	m4lucklv,	"lltdr.p1",	0x0000, 0x010000, CRC(c8af8031) SHA1(1c683e9320a528d214afa7284cce8325fbd29cdf), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 26)" )
M4LUCKLV_SET( 199?, m4lucklv__z,	m4lucklv,	"lltdy.p1",	0x0000, 0x010000, CRC(55c4a8df) SHA1(a7cc7fa2aa893a3f134d6d1c04a16ecbcad99a40), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 27)" )
M4LUCKLV_SET( 199?, m4lucklv__0,	m4lucklv,	"lltk.p1",	0x0000, 0x010000, CRC(88c7f5e5) SHA1(c30651c6daa6c82157879e72a74ee5e03f9ea64f), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 28)" )
M4LUCKLV_SET( 199?, m4lucklv__1,	m4lucklv,	"lltr.p1",	0x0000, 0x010000, CRC(189c8fc7) SHA1(c06bedd4477ead2c071374d17fc0ef617ef6e924), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 29)" )
M4LUCKLV_SET( 199?, m4lucklv__2,	m4lucklv,	"llts.p1",	0x0000, 0x010000, CRC(3ccc9e2f) SHA1(15c90bbc135ddaa05768bde6970bda15f1b69d44), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 30)" )
M4LUCKLV_SET( 199?, m4lucklv__3,	m4lucklv,	"llty.p1",	0x0000, 0x010000, CRC(85f7a729) SHA1(b3a58071bb6dae36354280a8f0de1faaa190899f), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 31)" )
M4LUCKLV_SET( 199?, m4lucklv__4,	m4lucklv,	"lluad.p1",	0x0000, 0x010000, CRC(7a2acea9) SHA1(dfb5763664143dec4927e2a8f4088f04ca85c458), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 32)" )
M4LUCKLV_SET( 199?, m4lucklv__5,	m4lucklv,	"llub.p1",	0x0000, 0x010000, CRC(d3683b6b) SHA1(e67e996f5f2ed55fbc47b7f1a0e225125baf2a99), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 33)" )
M4LUCKLV_SET( 199?, m4lucklv__6,	m4lucklv,	"llubd.p1",	0x0000, 0x010000, CRC(4c36a3e3) SHA1(afbe3c4d60dd92e7bc34915f3d15cadd46da6738), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 34)" )
M4LUCKLV_SET( 199?, m4lucklv__7,	m4lucklv,	"llud.p1",	0x0000, 0x010000, CRC(f82d26a3) SHA1(4c3dffbdebd6e3f372f95633cec985c7595a1b6c), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 35)" )
M4LUCKLV_SET( 199?, m4lucklv__8,	m4lucklv,	"lludk.p1",	0x0000, 0x010000, CRC(1bf8f2fe) SHA1(d9f04fc91dd2da95844d78d928b01d7bcf20326e), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 36)" )
M4LUCKLV_SET( 199?, m4lucklv__9,	m4lucklv,	"lludr.p1",	0x0000, 0x010000, CRC(2d780bfc) SHA1(f787daebcc0706f018ad0ce1388ee08f1bbfa07c), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 37)" )
M4LUCKLV_SET( 199?, m4lucklv__aa,	m4lucklv,	"lludy.p1",	0x0000, 0x010000, CRC(6d042cfe) SHA1(3812f99d45cff5f147a7965f3cb8c96eda9a2e8d), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 38)" )
M4LUCKLV_SET( 199?, m4lucklv__ab,	m4lucklv,	"lluk.p1",	0x0000, 0x010000, CRC(476cd76c) SHA1(1a122bfbe3d95b2ce9ecf18bc6d4f85d1ea09bc7), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 39)" )
M4LUCKLV_SET( 199?, m4lucklv__ac,	m4lucklv,	"llur.p1",	0x0000, 0x010000, CRC(89a1a999) SHA1(e21007e2db0f69f0753becbf499c54cb29f46a39), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 40)" )
M4LUCKLV_SET( 199?, m4lucklv__ad,	m4lucklv,	"llus.p1",	0x0000, 0x010000, CRC(07745135) SHA1(ec602d01910ac52d20ff9c54914a5261f538233b), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 41)" )
M4LUCKLV_SET( 199?, m4lucklv__ae,	m4lucklv,	"lluy.p1",	0x0000, 0x010000, CRC(1cfc18d6) SHA1(eb34e5a43cee1d4b64443f1fe2b1d12ccf35b847), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 42)" )
M4LUCKLV_SET( 199?, m4lucklv__af,	m4lucklv,	"llvb.p1",	0x0000, 0x010000, CRC(72e27d9b) SHA1(2355c48c4ac8bc94dcea74b04e68be8a461f09a3), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 43)" )
M4LUCKLV_SET( 199?, m4lucklv__ag,	m4lucklv,	"llvc.p1",	0x0000, 0x010000, CRC(00ac2db0) SHA1(a8ec3a2862abf4eb56734d15cd9b7db0333c98f2), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 44)" )
M4LUCKLV_SET( 199?, m4lucklv__ah,	m4lucklv,	"llvd.p1",	0x0000, 0x010000, CRC(fdb4dcc5) SHA1(77fa51dcf1895b83182e7ee4137ed68c06d2593b), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 45)" )
M4LUCKLV_SET( 199?, m4lucklv__ai,	m4lucklv,	"llvr.p1",	0x0000, 0x010000, CRC(a806c43f) SHA1(8c36d50c911f956a0458f942cc8e313104f00005), "Barcrest","Lucky Las Vegas (Barcrest) (MPU4) (set 46)" )




#define M4LUCKST_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "lssnd1.bin", 0x000000, 0x080000, CRC(401686bc) SHA1(ab62e6e097b0af2f68ae7f8f686f00cede5ec3aa) ) \
	ROM_LOAD( "lssnd2.bin", 0x080000, 0x080000, CRC(d9e0c0db) SHA1(3eba5b19ca98d23a94edf2be27ccefaa0e526a56) ) \

#define M4LUCKST_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4LUCKST_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \


M4LUCKST_SET( 199?, m4luckst,		0,			"lss06s.p1",			0x0000, 0x020000, CRC(b6a69478) SHA1(6b05b7f9af94a83adfdff328d4132f72a1dfb19f), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 1)" )
M4LUCKST_SET( 199?, m4luckst__a,	m4luckst,	"ls15g",				0x0000, 0x020000, CRC(b942ac91) SHA1(e77b2acd07cac9b747731f9e0637112fc6bf94c7), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 2)" )
M4LUCKST_SET( 199?, m4luckst__b,	m4luckst,	"ls15t",				0x0000, 0x020000, CRC(20447a20) SHA1(ca2ba566317ca87afcc2501e551c1326b9712526), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 3)" )
M4LUCKST_SET( 199?, m4luckst__c,	m4luckst,	"lss06ad.p1",			0x0000, 0x020000, CRC(9a512ec1) SHA1(c1eb4d0f5c915f392411d0ff2c931eb22a41b3a8), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 4)" )
M4LUCKST_SET( 199?, m4luckst__d,	m4luckst,	"lss06b.p1",			0x0000, 0x020000, CRC(f2e1cb20) SHA1(7867085e0d1166419360bf1b6f39e3ec31bf2c7f), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 5)" )
M4LUCKST_SET( 199?, m4luckst__e,	m4luckst,	"lss06bd.p1",			0x0000, 0x020000, CRC(174b8004) SHA1(9cce3124797b80886ab744885444f7a69711f6f1), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 6)" )
M4LUCKST_SET( 199?, m4luckst__f,	m4luckst,	"lss06c.p1",			0x0000, 0x020000, CRC(c5ab7632) SHA1(de6fea314be38e6a779008f81cac0825ed6eda54), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 7)" )
M4LUCKST_SET( 199?, m4luckst__g,	m4luckst,	"lss06d.p1",			0x0000, 0x020000, CRC(8ffac056) SHA1(896dbbfe6265c77e10d6f36859ba92c13b748517), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 8)" )
M4LUCKST_SET( 199?, m4luckst__h,	m4luckst,	"lss06dh.p1",			0x0000, 0x020000, CRC(d9a7d047) SHA1(e85e48104ca76534bcb8f38a76a67eabfdbe8ba0), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 9)" )
M4LUCKST_SET( 199?, m4luckst__i,	m4luckst,	"lss06dk.p1",			0x0000, 0x020000, CRC(5d8c2852) SHA1(ab3959defb9c129a455177857da894942065231c), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 10)" )
M4LUCKST_SET( 199?, m4luckst__j,	m4luckst,	"lss06dr.p1",			0x0000, 0x020000, CRC(ee839363) SHA1(ff418c507a8ef2f28ebe7493801bf09afbd6a48a), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 11)" )
M4LUCKST_SET( 199?, m4luckst__k,	m4luckst,	"lss06dy.p1",			0x0000, 0x020000, CRC(da6f08ac) SHA1(597ff1d3623dda6ffb3b8f7b8f5ef0683d433079), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 12)" )
M4LUCKST_SET( 199?, m4luckst__l,	m4luckst,	"lss06h.p1",			0x0000, 0x020000, CRC(3c0d9b63) SHA1(57a0408908c521a2a44c3a825b3e4480dff4f778), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 13)" )
M4LUCKST_SET( 199?, m4luckst__m,	m4luckst,	"lss06k.p1",			0x0000, 0x020000, CRC(b8266376) SHA1(7d84dce05224c8882ed103796a054b50e2390234), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 14)" )
M4LUCKST_SET( 199?, m4luckst__n,	m4luckst,	"lss06r.p1",			0x0000, 0x020000, CRC(0b29d847) SHA1(3412bff6f38ab12b9d5e30f1ed3e327ad58dc470), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 15)" )
M4LUCKST_SET( 199?, m4luckst__o,	m4luckst,	"lss06s.p1",			0x0000, 0x020000, CRC(b6a69478) SHA1(6b05b7f9af94a83adfdff328d4132f72a1dfb19f), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 16)" )
M4LUCKST_SET( 199?, m4luckst__p,	m4luckst,	"lss06y.p1",			0x0000, 0x020000, CRC(3fc54388) SHA1(f57667cc0263efe05d0f0538fae2f4f8adc0c405), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 17)" )
M4LUCKST_SET( 199?, m4luckst__q,	m4luckst,	"lss07ad.p1",			0x0000, 0x020000, CRC(c4e113c0) SHA1(e5e81c08c2487ee8802ff4374b3affdff3e70003), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 18)" )
M4LUCKST_SET( 199?, m4luckst__r,	m4luckst,	"lss07b.p1",			0x0000, 0x020000, CRC(eec5db67) SHA1(c7b76b50524b256ec42adc33c99f933790d9d578), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 19)" )
M4LUCKST_SET( 199?, m4luckst__s,	m4luckst,	"lss07bd.p1",			0x0000, 0x020000, CRC(49fbbd05) SHA1(4edebdf607ae996e0f4a2df02f181e75b784e1ac), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 20)" )
M4LUCKST_SET( 199?, m4luckst__t,	m4luckst,	"lss07c.p1",			0x0000, 0x020000, CRC(d98f6675) SHA1(0ab59dad98a879a5010e84ba9f2feefe55b08cf5), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 21)" )
M4LUCKST_SET( 199?, m4luckst__u,	m4luckst,	"lss07d.p1",			0x0000, 0x020000, CRC(93ded011) SHA1(f9588096aa8d14b62ab4d7984988f567955aeb9f), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 22)" )
M4LUCKST_SET( 199?, m4luckst__v,	m4luckst,	"lss07dh.p1",			0x0000, 0x020000, CRC(8717ed46) SHA1(67725fb72f1182544479e554f1efea5f167647f5), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 23)" )
M4LUCKST_SET( 199?, m4luckst__w,	m4luckst,	"lss07dk.p1",			0x0000, 0x020000, CRC(033c1553) SHA1(fa07bc86f0a6cfd91c982d0f44fa43c1e54195e0), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 24)" )
M4LUCKST_SET( 199?, m4luckst__x,	m4luckst,	"lss07dr.p1",			0x0000, 0x020000, CRC(b033ae62) SHA1(8c58e4b881c73c9c7450f81026f982b5cfb56197), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 25)" )
M4LUCKST_SET( 199?, m4luckst__y,	m4luckst,	"lss07dy.p1",			0x0000, 0x020000, CRC(84df35ad) SHA1(eb9e5c0a4fcb85bc6a81862ff4852de380ef0a5a), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 26)" )
M4LUCKST_SET( 199?, m4luckst__z,	m4luckst,	"lss07h.p1",			0x0000, 0x020000, CRC(20298b24) SHA1(004d6d04d9e8223e7b97dbd39abc66e80d6ea0eb), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 27)" )
M4LUCKST_SET( 199?, m4luckst__0,	m4luckst,	"lss07k.p1",			0x0000, 0x020000, CRC(a4027331) SHA1(64320519f22ebac3fbb7d93e2e70e999e8e7cd29), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 28)" )
M4LUCKST_SET( 199?, m4luckst__1,	m4luckst,	"lss07r.p1",			0x0000, 0x020000, CRC(170dc800) SHA1(bebd8b41b756c7ca5ddc60647e6031ac4cf874db), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 29)" )
M4LUCKST_SET( 199?, m4luckst__2,	m4luckst,	"lss07s.p1",			0x0000, 0x020000, CRC(f4546d1a) SHA1(fed65704693e11087825b1dfda4df28ee6d2d3be), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 30)" )
M4LUCKST_SET( 199?, m4luckst__3,	m4luckst,	"lss07y.p1",			0x0000, 0x020000, CRC(23e153cf) SHA1(75c5c13ad735aba1bf62d26eb37f9aaa5804fdd2), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 31)" )
M4LUCKST_SET( 199?, m4luckst__4,	m4luckst,	"lst09ad.p1",			0x0000, 0x020000, CRC(2fc94fb9) SHA1(cda6ce0a9d9e124326e29b57bc553b517408f1c6), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 32)" )
M4LUCKST_SET( 199?, m4luckst__5,	m4luckst,	"lst09b.p1",			0x0000, 0x020000, CRC(c5b8927f) SHA1(15d227be23404b9393cb3d3625fd054eaf0da3a4), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 33)" )
M4LUCKST_SET( 199?, m4luckst__6,	m4luckst,	"lst09bd.p1",			0x0000, 0x020000, CRC(650ee7ef) SHA1(2422e4f9857376a029f6b4df46b5b81c73b2640a), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 34)" )
M4LUCKST_SET( 199?, m4luckst__7,	m4luckst,	"lst09d.p1",			0x0000, 0x020000, CRC(48a23cba) SHA1(04ce2a07ed89152da69e5d5309390060fbe30e8d), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 35)" )
M4LUCKST_SET( 199?, m4luckst__8,	m4luckst,	"lst09dh.p1",			0x0000, 0x020000, CRC(469d15a4) SHA1(6729b73da48f1bc2f169932a95bba5d0a7795905), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 36)" )
M4LUCKST_SET( 199?, m4luckst__9,	m4luckst,	"lst09dk.p1",			0x0000, 0x020000, CRC(9cc6f488) SHA1(ded41be5841bd9717bfe0526b89cfcfd98a1e757), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 37)" )
M4LUCKST_SET( 199?, m4luckst__aa,	m4luckst,	"lst09dr.p1",			0x0000, 0x020000, CRC(06282a31) SHA1(97c1d3aab87ec2a8602f840ce8b58056c28dc34c), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 38)" )
M4LUCKST_SET( 199?, m4luckst__ab,	m4luckst,	"lst09dy.p1",			0x0000, 0x020000, CRC(a82a6f47) SHA1(0694ca9ff9882547f1ca8cfad4c9c824f6da799f), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 39)" )
M4LUCKST_SET( 199?, m4luckst__ac,	m4luckst,	"lst09h.p1",			0x0000, 0x020000, CRC(e62b6034) SHA1(37ae4b6250795087475ea8e51b49e225572118b4), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 40)" )
M4LUCKST_SET( 199?, m4luckst__ad,	m4luckst,	"lst09k.p1",			0x0000, 0x020000, CRC(3c708118) SHA1(61ab1751dec08f7407fd3360cf129736d7d5ec16), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 41)" )
M4LUCKST_SET( 199?, m4luckst__ae,	m4luckst,	"lst09r.p1",			0x0000, 0x020000, CRC(a69e5fa1) SHA1(10ca0a2135c4d6832693bf97812f4eb5f1380efd), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 42)" )
M4LUCKST_SET( 199?, m4luckst__af,	m4luckst,	"lst09s.p1",			0x0000, 0x020000, CRC(285d0255) SHA1(89f74ad19525b636d6e1ea36308b659389f68245), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 43)" )
M4LUCKST_SET( 199?, m4luckst__ag,	m4luckst,	"lst09y.p1",			0x0000, 0x020000, CRC(089c1ad7) SHA1(bcfacbeaf1845d91f18c6f57bf166c82469cb460), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 44)" )
M4LUCKST_SET( 199?, m4luckst__ah,	m4luckst,	"lst10ad.p1",			0x0000, 0x020000, CRC(41e8dd7d) SHA1(2b794554f57dfb5b6fe9f64b9d3e6b73ec306056), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 45)" )
M4LUCKST_SET( 199?, m4luckst__ai,	m4luckst,	"lst10b.p1",			0x0000, 0x020000, CRC(afd0d023) SHA1(7ee5c004d9d0cacf2d55457f9846fd638b584482), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 46)" )
M4LUCKST_SET( 199?, m4luckst__aj,	m4luckst,	"lst10bd.p1",			0x0000, 0x020000, CRC(0b2f752b) SHA1(655f50bbe163fca4386b356e7db0b019eba5d94f), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 47)" )
M4LUCKST_SET( 199?, m4luckst__ak,	m4luckst,	"lst10d.p1",			0x0000, 0x020000, CRC(22ca7ee6) SHA1(487da48e02f11aef5a2c42efe54c5e9f43a7915d), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 48)" )
M4LUCKST_SET( 199?, m4luckst__al,	m4luckst,	"lst10dh.p1",			0x0000, 0x020000, CRC(28bc8760) SHA1(f442853ac0f6cedbdb4b928bfee82e6f9d0e3824), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 49)" )
M4LUCKST_SET( 199?, m4luckst__am,	m4luckst,	"lst10dk.p1",			0x0000, 0x020000, CRC(f2e7664c) SHA1(bc869ec0e533143bdcef52f06fcf34c5eb30d928), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 50)" )
M4LUCKST_SET( 199?, m4luckst__an,	m4luckst,	"lst10dr.p1",			0x0000, 0x020000, CRC(6809b8f5) SHA1(626859168f33ca3e2ce133c645a32a73a811ac75), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 51)" )
M4LUCKST_SET( 199?, m4luckst__ao,	m4luckst,	"lst10dy.p1",			0x0000, 0x020000, CRC(c60bfd83) SHA1(276d65498c8d1da56765c451b8c9bf2b4af096a6), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 52)" )
M4LUCKST_SET( 199?, m4luckst__ap,	m4luckst,	"lst10h.p1",			0x0000, 0x020000, CRC(8c432268) SHA1(c8707c587ab2cc1450bd47069206767d2d930b29), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 53)" )
M4LUCKST_SET( 199?, m4luckst__aq,	m4luckst,	"lst10k.p1",			0x0000, 0x020000, CRC(5618c344) SHA1(61565a9814fd1b4e77944e73d8f18f8545685928), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 54)" )
M4LUCKST_SET( 199?, m4luckst__ar,	m4luckst,	"lst10r.p1",			0x0000, 0x020000, CRC(ccf61dfd) SHA1(01772dd0b252b41b5cba024a717d5898e74971f8), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 55)" )
M4LUCKST_SET( 199?, m4luckst__as,	m4luckst,	"lst10s.p1",			0x0000, 0x020000, CRC(0e1ad810) SHA1(bd439f2857ebbde2b7941c411ac7edf7c66af7eb), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 56)" )
M4LUCKST_SET( 199?, m4luckst__at,	m4luckst,	"lst10y.p1",			0x0000, 0x020000, CRC(62f4588b) SHA1(ee7c06e2cc79f7d18d45b0a1793e5279a46ffcc0), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 57)" )
M4LUCKST_SET( 199?, m4luckst__au,	m4luckst,	"lstrikegame10-8t.bin",	0x0000, 0x020000, CRC(709c2dbf) SHA1(bba8d7af9502911ffa1c086b993484ab78ad38ac), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 58)" )
M4LUCKST_SET( 199?, m4luckst__av,	m4luckst,	"ls55",					0x0000, 0x020000, CRC(823e805b) SHA1(17f09fd53188950a8d98ac04cd94785947b52b01), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 59)" )
M4LUCKST_SET( 199?, m4luckst__aw,	m4luckst,	"ls__xa_x.1_1",			0x0000, 0x020000, CRC(a9642503) SHA1(2765c4d8943678446c516918035d7a888a812aae), "Barcrest","Lucky Strike (Barcrest) (MPU4) (set 60)" )


#define M4TENTEN_EXTRA_ROMS \
	ROM_REGION( 0x080000, "msm6376", 0 ) \
	ROM_LOAD( "tttsnd01.p1", 0x0000, 0x080000, CRC(5518474c) SHA1(0b7e98e33f62d80882f2b0b4af0c9056f1ffb78d) ) \

#define M4TENTEN_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4TENTEN_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \


M4TENTEN_SET( 199?, m4tenten,		0,			"t2002s.p1",	0x0000, 0x010000, CRC(6cd9fa10) SHA1(8efe36e3fc5b709fa4363194634686d62b5d6609), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 1)" )
M4TENTEN_SET( 199?, m4tenten__a,	m4tenten,	"n2503ad.p1",	0x0000, 0x010000, CRC(c84150e6) SHA1(8f143c26c6026a413bdd65ca148d78dead1d2474), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 2)" )
M4TENTEN_SET( 199?, m4tenten__b,	m4tenten,	"n2503b.p1",	0x0000, 0x010000, CRC(dd74fb57) SHA1(402f632f48cf1153cb8c22879a7482c82c8fecfe), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 3)" )
M4TENTEN_SET( 199?, m4tenten__c,	m4tenten,	"n2503bd.p1",	0x0000, 0x010000, CRC(62542d80) SHA1(90ebfb92891a7aaa4814b733c0e0df06bb292a4f), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 4)" )
M4TENTEN_SET( 199?, m4tenten__d,	m4tenten,	"n2503d.p1",	0x0000, 0x010000, CRC(b8c64fa4) SHA1(f53d9bdf97cc021399c8598a051ad5bcb7a611b5), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 5)" )
M4TENTEN_SET( 199?, m4tenten__e,	m4tenten,	"n2503dk.p1",	0x0000, 0x010000, CRC(28715d57) SHA1(1ab648a7f5dde5575e5ab5653823e4b88580677f), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 6)" )
M4TENTEN_SET( 199?, m4tenten__f,	m4tenten,	"n2503dr.p1",	0x0000, 0x010000, CRC(59932d11) SHA1(9895392ce48569e23816646124933192b6f720e3), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 7)" )
M4TENTEN_SET( 199?, m4tenten__g,	m4tenten,	"n2503dy.p1",	0x0000, 0x010000, CRC(287b8ee7) SHA1(10e428c73911e01ba53dd4321eb2c5a58e35441e), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 8)" )
M4TENTEN_SET( 199?, m4tenten__h,	m4tenten,	"n2503k.p1",	0x0000, 0x010000, CRC(f240b5ba) SHA1(fca353213447e50e29f1cd2c0d3895c437ae9336), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 9)" )
M4TENTEN_SET( 199?, m4tenten__i,	m4tenten,	"n2503r.p1",	0x0000, 0x010000, CRC(afaacb9d) SHA1(1bf3d648b82a8160ff1a38d92d2bb3ce1b41125e), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 10)" )
M4TENTEN_SET( 199?, m4tenten__j,	m4tenten,	"n2503s.p1",	0x0000, 0x010000, CRC(0d9e912c) SHA1(67d888fde242e81a1808f36e1e81c0c5724e99a7), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 11)" )
M4TENTEN_SET( 199?, m4tenten__k,	m4tenten,	"n2503y.p1",	0x0000, 0x010000, CRC(b365cff0) SHA1(65ab9d624bec1efc0590cec739541887ca20fc6f), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 12)" )
M4TENTEN_SET( 199?, m4tenten__l,	m4tenten,	"t2002ad.p1",	0x0000, 0x010000, CRC(f7903f0d) SHA1(8a10ff31ddaad817a31d39ea9a66d4453d8767bb), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 13)" )
M4TENTEN_SET( 199?, m4tenten__m,	m4tenten,	"t2002b.p1",	0x0000, 0x010000, CRC(90e150ea) SHA1(abc8456de42cef605e1f0e80a97b25cd51d90707), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 14)" )
M4TENTEN_SET( 199?, m4tenten__n,	m4tenten,	"t2002bd.p1",	0x0000, 0x010000, CRC(110c0d5c) SHA1(a263c678ebd58c95f33b198be544b726ac506449), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 15)" )
M4TENTEN_SET( 199?, m4tenten__o,	m4tenten,	"t2002d.p1",	0x0000, 0x010000, CRC(65a9bb19) SHA1(9a472eaded63ecab260cda03fb9e96bc6db5a99e), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 16)" )
M4TENTEN_SET( 199?, m4tenten__p,	m4tenten,	"t2002dk.p1",	0x0000, 0x010000, CRC(7cbbca26) SHA1(238717c37fbfab737805f46225ac740fed9a9501), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 17)" )
M4TENTEN_SET( 199?, m4tenten__q,	m4tenten,	"t2002dr.p1",	0x0000, 0x010000, CRC(b43b946d) SHA1(9239adb301eef818fe46ce7f3ce8b01a47031165), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 18)" )
M4TENTEN_SET( 199?, m4tenten__r,	m4tenten,	"t2002dy.p1",	0x0000, 0x010000, CRC(883e6777) SHA1(d5a5f0624ccf373922ef4b41519c6c95a2367b2a), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 19)" )
M4TENTEN_SET( 199?, m4tenten__s,	m4tenten,	"t2002k.p1",	0x0000, 0x010000, CRC(34628c9d) SHA1(3547dac8fd73abcba0f088c3f2db02ef8a40cf4e), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 20)" )
M4TENTEN_SET( 199?, m4tenten__t,	m4tenten,	"t2002r.p1",	0x0000, 0x010000, CRC(9c639380) SHA1(c044b7fc53daa01d93c701a3d3c786ab6a883e76), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 21)" )
M4TENTEN_SET( 199?, m4tenten__u,	m4tenten,	"t2002y.p1",	0x0000, 0x010000, CRC(dcf2ac8c) SHA1(e745534f45148ff33e6b1e503108a39e27d8bdae), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 22)" )
M4TENTEN_SET( 199?, m4tenten__v,	m4tenten,	"t2504ad.p1",	0x0000, 0x010000, CRC(f964cf74) SHA1(a72bd32f3785506e8f57107f2b8e42d64cbe267b), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 23)" )
M4TENTEN_SET( 199?, m4tenten__w,	m4tenten,	"t2504b.p1",	0x0000, 0x010000, CRC(f6cc485b) SHA1(6a95bf4c0cf35ccb79cb2a9e6bef208d6b61f4c7), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 24)" )
M4TENTEN_SET( 199?, m4tenten__x,	m4tenten,	"t2504bd.p1",	0x0000, 0x010000, CRC(f4ce2ee9) SHA1(8d0dc72f288810118702d00dc08f4e748c4da9e6), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 25)" )
M4TENTEN_SET( 199?, m4tenten__y,	m4tenten,	"t2504d.p1",	0x0000, 0x010000, CRC(44975553) SHA1(29116ab2d8bcbd7e73c2fa63381e93df7f9bdad9), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 26)" )
M4TENTEN_SET( 199?, m4tenten__z,	m4tenten,	"t2504dk.p1",	0x0000, 0x010000, CRC(3794f312) SHA1(1c15c9dd9c2631ea7fce070b45fd960cad053fcb), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 27)" )
M4TENTEN_SET( 199?, m4tenten__0,	m4tenten,	"t2504dr.p1",	0x0000, 0x010000, CRC(96c76a75) SHA1(28cf5e111b98e8f78ba5a5ea1ad59fc92657cd08), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 28)" )
M4TENTEN_SET( 199?, m4tenten__1,	m4tenten,	"t2504dy.p1",	0x0000, 0x010000, CRC(af52e603) SHA1(8c234f0964018eb21efff7a9e820c355d4465401), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 29)" )
M4TENTEN_SET( 199?, m4tenten__2,	m4tenten,	"t2504k.p1",	0x0000, 0x010000, CRC(c489942d) SHA1(047509ede2cb5a155274698423a1caa32ac0baeb), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 30)" )
M4TENTEN_SET( 199?, m4tenten__3,	m4tenten,	"t2504r.p1",	0x0000, 0x010000, CRC(da847c46) SHA1(b5897fe0266ee3eb5f364570f29ffca3de8c4c7f), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 31)" )
M4TENTEN_SET( 199?, m4tenten__4,	m4tenten,	"t2504s.p1",	0x0000, 0x010000, CRC(a7618248) SHA1(e28d34a7a4b2eb3f6192a04cc8849475f021d392), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 32)" )
M4TENTEN_SET( 199?, m4tenten__5,	m4tenten,	"t2504y.p1",	0x0000, 0x010000, CRC(e311f030) SHA1(ce21932013f1fdab1b9bbe8eb61d4fe067ccc62f), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 33)" )
M4TENTEN_SET( 199?, m4tenten__6,	m4tenten,	"t2t01ad.p1",	0x0000, 0x010000, CRC(debb50d9) SHA1(18b099111d81cf847155b81faba8be06cdfb3d54), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 34)" )
M4TENTEN_SET( 199?, m4tenten__7,	m4tenten,	"t2t01b.p1",	0x0000, 0x010000, CRC(4d8b3369) SHA1(e2221c712e8b031f39531fed61895af2214c23bc), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 35)" )
M4TENTEN_SET( 199?, m4tenten__8,	m4tenten,	"t2t01bd.p1",	0x0000, 0x010000, CRC(e7f852d2) SHA1(7aff9dc05e8a5db91abea61860f85793a73ad8db), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 36)" )
M4TENTEN_SET( 199?, m4tenten__9,	m4tenten,	"t2t01d.p1",	0x0000, 0x010000, CRC(6aba0e0d) SHA1(7feeb665974663109dc3a2afacd2b7b8c1c048d6), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 37)" )
M4TENTEN_SET( 199?, m4tenten__aa,	m4tenten,	"t2t01dk.p1",	0x0000, 0x010000, CRC(1bd1d573) SHA1(95b0b6eab8b71d7b43e1c50f77fb2d7e4c00d0ac), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 38)" )
M4TENTEN_SET( 199?, m4tenten__ab,	m4tenten,	"t2t01dr.p1",	0x0000, 0x010000, CRC(45978b39) SHA1(c40ea3a013a2593064cafb3933f7f3b15e1034f3), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 39)" )
M4TENTEN_SET( 199?, m4tenten__ac,	m4tenten,	"t2t01dy.p1",	0x0000, 0x010000, CRC(205c3c2e) SHA1(53d6829ac650fc7637f8743ac6cdb3b02dd0cc86), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 40)" )
M4TENTEN_SET( 199?, m4tenten__ad,	m4tenten,	"t2t01k.p1",	0x0000, 0x010000, CRC(488b2ce4) SHA1(3b47ea15f318b49744e869d0485e00fbd2cd74b2), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 41)" )
M4TENTEN_SET( 199?, m4tenten__ae,	m4tenten,	"t2t01r.p1",	0x0000, 0x010000, CRC(a8f71c79) SHA1(0b9d468819bfd2f7e2ec7b5f11f052531a13d703), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 42)" )
M4TENTEN_SET( 199?, m4tenten__af,	m4tenten,	"t2t01s.p1",	0x0000, 0x010000, CRC(75b421e3) SHA1(d5de7485180baf9d8458a895edbfd65310fed2cc), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 43)" )
M4TENTEN_SET( 199?, m4tenten__ag,	m4tenten,	"t2t01y.p1",	0x0000, 0x010000, CRC(0c3e29c2) SHA1(a0163587193145a0a173d1571ad9076c9f03d3ad), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 44)" )
M4TENTEN_SET( 199?, m4tenten__ah,	m4tenten,	"t3t01ad.p1",	0x0000, 0x010000, CRC(7075b69b) SHA1(3ac28d0542c287de9c2cabfed2da0ac0cc4a24cb), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 45)" )
M4TENTEN_SET( 199?, m4tenten__ai,	m4tenten,	"t3t01b.p1",	0x0000, 0x010000, CRC(7f7d4170) SHA1(3925a0e1f620777356ed6d6c67d6d432daeb9ade), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 46)" )
M4TENTEN_SET( 199?, m4tenten__aj,	m4tenten,	"t3t01bd.p1",	0x0000, 0x010000, CRC(90d5dbcb) SHA1(2c348f81fe070d0825c1824f05bc4c640da9ce24), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 47)" )
M4TENTEN_SET( 199?, m4tenten__ak,	m4tenten,	"t3t01d.p1",	0x0000, 0x010000, CRC(8a35aa83) SHA1(4156abc1eb649baf875e9b628f9b865974959c96), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 48)" )
M4TENTEN_SET( 199?, m4tenten__al,	m4tenten,	"t3t01dk.p1",	0x0000, 0x010000, CRC(b7d70c87) SHA1(0d0c3d73b37641151ca8e5aea02cbd90a6c93560), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 49)" )
M4TENTEN_SET( 199?, m4tenten__am,	m4tenten,	"t3t01dr.p1",	0x0000, 0x010000, CRC(f2dc9f57) SHA1(ba6a3d48c832d833df543f21d8aad98099894cff), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 50)" )
M4TENTEN_SET( 199?, m4tenten__an,	m4tenten,	"t3t01dy.p1",	0x0000, 0x010000, CRC(94bb082d) SHA1(236b81d7e10afd4dc3facfad30d9ab0a72ea21e3), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 51)" )
M4TENTEN_SET( 199?, m4tenten__ao,	m4tenten,	"t3t01k.p1",	0x0000, 0x010000, CRC(b7bd2547) SHA1(6154b30a587d52dadd1fa137e805762e096f02cb), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 52)" )
M4TENTEN_SET( 199?, m4tenten__ap,	m4tenten,	"t3t01r.p1",	0x0000, 0x010000, CRC(d4401ee1) SHA1(c270a0f191870136278ff8e47d529458bbc049d0), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 53)" )
M4TENTEN_SET( 199?, m4tenten__aq,	m4tenten,	"t3t01s.p1",	0x0000, 0x010000, CRC(eae20667) SHA1(3ea054516e36ac5ca521a68bba16e299a1926c90), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 54)" )
M4TENTEN_SET( 199?, m4tenten__ar,	m4tenten,	"t3t01y.p1",	0x0000, 0x010000, CRC(d2a6c8cd) SHA1(b30064145cebc2a39ae63009529cd8422dc98373), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 55)" )
M4TENTEN_SET( 199?, m4tenten__as,	m4tenten,	"tst01ad.p1",	0x0000, 0x010000, CRC(ca4be612) SHA1(70673e29a99ea0c70ff386c6c5fed49eabeea0e4), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 56)" )
M4TENTEN_SET( 199?, m4tenten__at,	m4tenten,	"tst01b.p1",	0x0000, 0x010000, CRC(7dd06165) SHA1(6853436a1b0c60283707932961bbf5cce7e185c0), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 57)" )
M4TENTEN_SET( 199?, m4tenten__au,	m4tenten,	"tst01bd.p1",	0x0000, 0x010000, CRC(38f4c0a2) SHA1(2cc62837c37618198dae7195e848c06ad1b96b06), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 58)" )
M4TENTEN_SET( 199?, m4tenten__av,	m4tenten,	"tst01d.p1",	0x0000, 0x010000, CRC(346adbf7) SHA1(5500822ea6838435ce28e084e2b13533e9467e85), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 59)" )
M4TENTEN_SET( 199?, m4tenten__aw,	m4tenten,	"tst01dk.p1",	0x0000, 0x010000, CRC(2e8f8b14) SHA1(932b7c32d623dc8c2127959a4a0642d10ec22e71), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 60)" )
M4TENTEN_SET( 199?, m4tenten__ax,	m4tenten,	"tst01dr.p1",	0x0000, 0x010000, CRC(191a1272) SHA1(8baae132505c469082af538b233f3fa3e7332d50), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 61)" )
M4TENTEN_SET( 199?, m4tenten__ay,	m4tenten,	"tst01dy.p1",	0x0000, 0x010000, CRC(ef87da08) SHA1(9358e4a02def10c98fddbb21d81b62f83500aa69), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 62)" )
M4TENTEN_SET( 199?, m4tenten__az,	m4tenten,	"tst01k.p1",	0x0000, 0x010000, CRC(1f097f64) SHA1(d752b688b4e0520393bc4bef0c618a7c68c2e323), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 63)" )
M4TENTEN_SET( 199?, m4tenten__a0,	m4tenten,	"tst01r.p1",	0x0000, 0x010000, CRC(ccc4ecb5) SHA1(0629c7951b56f44b8cb48d9aa66fb7c71ae275ec), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 64)" )
M4TENTEN_SET( 199?, m4tenten__a1,	m4tenten,	"tst01s.p1",	0x0000, 0x010000, CRC(c4bb2a12) SHA1(1d8c134facfa72d8438676c96e530f93c41f1266), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 65)" )
M4TENTEN_SET( 199?, m4tenten__a2,	m4tenten,	"tst01y.p1",	0x0000, 0x010000, CRC(e3ba4b94) SHA1(a7b13c172e5177711ddb81ef1ea77e27e14bf470), "Barcrest","10 X 10 (Barcrest) (MPU4) (set 66)" )


#define M4ANDYFH_EXTRA_ROMS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "afhsnd1.bin", 0x000000, 0x080000, CRC(ce0b1890) SHA1(224d05f936a1b6f84ad682c282c557e87ad8931f) ) \
	ROM_LOAD( "afhsnd2.bin", 0x080000, 0x080000, CRC(8a4dda7b) SHA1(ee77295609ff646212faa207e56acb2440d859b8) ) \

#define M4ANDYFH_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4ANDYFH_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring ,ROT0,company,title,GAME_FLAGS ) \




M4ANDYFH_SET( 199?, m4andyfh,		0,			"afhs.p1",		0x0000, 0x010000, CRC(722660ef) SHA1(e1700f4dc6d14da8e8d8402466057cfd126e067b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 1)" )
M4ANDYFH_SET( 199?, m4andyfh__a,	m4andyfh,	"af3ad.p1",		0x0000, 0x010000, CRC(ef141eca) SHA1(1ba03db9c05f5d60c5e1e0729eb124f6c5c3acf5), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 2)" )
M4ANDYFH_SET( 199?, m4andyfh__b,	m4andyfh,	"af3b.p1",		0x0000, 0x010000, CRC(78889d06) SHA1(5ea4c8010b7fd3e2e41d378b69a7cfda27aba99f), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 3)" )
M4ANDYFH_SET( 199?, m4andyfh__c,	m4andyfh,	"af3bd.p1",		0x0000, 0x010000, CRC(d9087380) SHA1(1a7f203b722583927eb6f99a493e564100321fe6), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 4)" )
M4ANDYFH_SET( 199?, m4andyfh__d,	m4andyfh,	"af3d.p1",		0x0000, 0x010000, CRC(ffe8a5f9) SHA1(b8632489ff015aa50e2c062f39096bb49e39ffe5), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 5)" )
M4ANDYFH_SET( 199?, m4andyfh__e,	m4andyfh,	"af3dk.p1",		0x0000, 0x010000, CRC(4fc4a031) SHA1(c5c68027231988a88610931f395ae08d8e60f962), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 6)" )
M4ANDYFH_SET( 199?, m4andyfh__f,	m4andyfh,	"af3dy.p1",		0x0000, 0x010000, CRC(f59c1a50) SHA1(55054a49b7bbf4a27ec808727cfbf3ce9bdfce40), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 7)" )
M4ANDYFH_SET( 199?, m4andyfh__g,	m4andyfh,	"af3k.p1",		0x0000, 0x010000, CRC(ddf5edfb) SHA1(ce69c70b1cdcebfa29e1613cb619617a961a649b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 8)" )
M4ANDYFH_SET( 199?, m4andyfh__h,	m4andyfh,	"af3s.p1",		0x0000, 0x010000, CRC(e9860d9a) SHA1(f1d1323e2329613748602559b6458a19963c091a), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 9)" )
M4ANDYFH_SET( 199?, m4andyfh__i,	m4andyfh,	"af3y.p1",		0x0000, 0x010000, CRC(1895bbe1) SHA1(c084f77004c9086bd75add665b25b0b3e114a91f), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 10)" )
M4ANDYFH_SET( 199?, m4andyfh__j,	m4andyfh,	"af8b.p1",		0x0000, 0x010000, CRC(fa8d002e) SHA1(cad754268706a1c942ce3751aa5a51720a104899), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 11)" )
M4ANDYFH_SET( 199?, m4andyfh__k,	m4andyfh,	"af8bd.p1",		0x0000, 0x010000, CRC(f64ce609) SHA1(d36a868647a954fd7974613510aabc6fc18035ee), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 12)" )
M4ANDYFH_SET( 199?, m4andyfh__l,	m4andyfh,	"af8c.p1",		0x0000, 0x010000, CRC(6dff4569) SHA1(10809f81924a72b21129158043c023ad6809cced), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 13)" )
M4ANDYFH_SET( 199?, m4andyfh__m,	m4andyfh,	"af8k.p1",		0x0000, 0x010000, CRC(0f6cb2a4) SHA1(320954e216e48ef2882f6d4feb7e29c106d49b79), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 14)" )
M4ANDYFH_SET( 199?, m4andyfh__n,	m4andyfh,	"af8s.p1",		0x0000, 0x010000, CRC(1b06be8e) SHA1(c1b67b23c6e2abca68fb242e24b61333bde688fa), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 15)" )
M4ANDYFH_SET( 199?, m4andyfh__o,	m4andyfh,	"afhb.p1",		0x0000, 0x010000, CRC(899945a4) SHA1(ed4a8c9b35e3aa08ea762740a713352560490443), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 16)" )
M4ANDYFH_SET( 199?, m4andyfh__p,	m4andyfh,	"afhc.p1",		0x0000, 0x010000, CRC(eff4016e) SHA1(4497ae5033aa4c3b3af8e2f6821dadb3f0683c82), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 17)" )
M4ANDYFH_SET( 199?, m4andyfh__q,	m4andyfh,	"afhd.p1",		0x0000, 0x010000, CRC(9f673d80) SHA1(ae2658d817d4d07f2d9f7948f0660f51626d07ac), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 18)" )
M4ANDYFH_SET( 199?, m4andyfh__r,	m4andyfh,	"afhr.p1",		0x0000, 0x010000, CRC(232bc900) SHA1(831368184be51b13db30468d519e395a9af7570e), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 19)" )
M4ANDYFH_SET( 199?, m4andyfh__s,	m4andyfh,	"aftad.p1",		0x0000, 0x010000, CRC(72b75e4a) SHA1(fb25a4a455589c51ec7bf1e77faee7f9809eea2c), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 20)" )
M4ANDYFH_SET( 199?, m4andyfh__t,	m4andyfh,	"aftb.p1",		0x0000, 0x010000, CRC(be3cd9ec) SHA1(135f5b575ff1921d08985251b6cd326db4de4f3e), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 21)" )
M4ANDYFH_SET( 199?, m4andyfh__u,	m4andyfh,	"aftbd.p1",		0x0000, 0x010000, CRC(d7fd4c6d) SHA1(3eea5f025f25194fd1d4b4cf0643445b11694c7b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 22)" )
M4ANDYFH_SET( 199?, m4andyfh__v,	m4andyfh,	"aftd.p1",		0x0000, 0x010000, CRC(4e5294c9) SHA1(94ef873a01e0635d4249f9c24b806ed3c08575fb), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 23)" )
M4ANDYFH_SET( 199?, m4andyfh__w,	m4andyfh,	"aftdk.p1",		0x0000, 0x010000, CRC(51e6eb8a) SHA1(32291d8f8ddd4002a0c149b97ac120f128f8347b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 24)" )
M4ANDYFH_SET( 199?, m4andyfh__x,	m4andyfh,	"aftdr.p1",		0x0000, 0x010000, CRC(b8b922d2) SHA1(ec90c7c5fb72a912b0918dc87f8feea49077f7ef), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 25)" )
M4ANDYFH_SET( 199?, m4andyfh__y,	m4andyfh,	"aftdy.p1",		0x0000, 0x010000, CRC(3e9fc7a6) SHA1(65d181398b3e574b26b060001e9477d5ee40bcc0), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 26)" )
M4ANDYFH_SET( 199?, m4andyfh__z,	m4andyfh,	"aftk.p1",		0x0000, 0x010000, CRC(8d87a910) SHA1(a315210a5c9d880621937412ff6d8d42ac658db2), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 27)" )
M4ANDYFH_SET( 199?, m4andyfh__0,	m4andyfh,	"aftr.p1",		0x0000, 0x010000, CRC(1be08c33) SHA1(21cd201cae159a1a3ee17f9661bc6db5e5a0ad48), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 28)" )
M4ANDYFH_SET( 199?, m4andyfh__1,	m4andyfh,	"afts.p1",		0x0000, 0x010000, CRC(7f059eec) SHA1(06de497bbae7391bbb09241204dfdd59ecc36569), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 29)" )
M4ANDYFH_SET( 199?, m4andyfh__2,	m4andyfh,	"afty.p1",		0x0000, 0x010000, CRC(d14f2670) SHA1(a6d21e855fbb90e80c8b8c4af02280343edcb3e8), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 30)" )
M4ANDYFH_SET( 199?, m4andyfh__3,	m4andyfh,	"afuad.p1",		0x0000, 0x010000, CRC(0f14e261) SHA1(080a5667127e14b6959ff1508f028fd849c27c24), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 31)" )
M4ANDYFH_SET( 199?, m4andyfh__4,	m4andyfh,	"afub.p1",		0x0000, 0x010000, CRC(99c6a4cc) SHA1(36fe83a32aeab413c19bc253edeadf4bc0f73615), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 32)" )
M4ANDYFH_SET( 199?, m4andyfh__5,	m4andyfh,	"afubd.p1",		0x0000, 0x010000, CRC(c38d376a) SHA1(843d0dcc0909ea7cd93f6ba707e784b160cb4984), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 33)" )
M4ANDYFH_SET( 199?, m4andyfh__6,	m4andyfh,	"afud.p1",		0x0000, 0x010000, CRC(640e0f24) SHA1(47879027c98557e7087b4fdf40161dbecc0f5c45), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 34)" )
M4ANDYFH_SET( 199?, m4andyfh__7,	m4andyfh,	"afudk.p1",		0x0000, 0x010000, CRC(eeebf560) SHA1(518931322d1ba7d7fe51140a21b38dd3a90f308a), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 35)" )
M4ANDYFH_SET( 199?, m4andyfh__8,	m4andyfh,	"afudr.p1",		0x0000, 0x010000, CRC(5f1e67ee) SHA1(80f13720256e00f924cdb2ad8f9101d131addcfb), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 36)" )
M4ANDYFH_SET( 199?, m4andyfh__9,	m4andyfh,	"afudy.p1",		0x0000, 0x010000, CRC(c2754f00) SHA1(4012231cb4a2eb0e0010f90d173295aa3c1fd6a5), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 37)" )
M4ANDYFH_SET( 199?, m4andyfh__aa,	m4andyfh,	"afuk.p1",		0x0000, 0x010000, CRC(f58fcf3c) SHA1(3731eab62e447a833b7decde842eda6a36cfadef), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 38)" )
M4ANDYFH_SET( 199?, m4andyfh__ab,	m4andyfh,	"afur.p1",		0x0000, 0x010000, CRC(0369ab49) SHA1(53acb382fada789c976b1dd124014778cfe518bc), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 39)" )
M4ANDYFH_SET( 199?, m4andyfh__ac,	m4andyfh,	"afus.p1",		0x0000, 0x010000, CRC(efbde76c) SHA1(abad98f2affb46e449a50f5a43729160b275294b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 40)" )
M4ANDYFH_SET( 199?, m4andyfh__ad,	m4andyfh,	"afuy.p1",		0x0000, 0x010000, CRC(9e0283a7) SHA1(63c0e3f26132a6bd6d8d3a8a3d0ab46e52fb2c09), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 41)" )
M4ANDYFH_SET( 199?, m4andyfh__ae,	m4andyfh,	"ca4ad.p1",		0x0000, 0x010000, CRC(bb311861) SHA1(9606b536c2775997935049caddb79170a98211b4), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 42)" )
M4ANDYFH_SET( 199?, m4andyfh__af,	m4andyfh,	"ca4b.p1",		0x0000, 0x010000, CRC(71e8f0f9) SHA1(663f536f4b3de20c2dcae52d22f3be9be19b0a4d), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 43)" )
M4ANDYFH_SET( 199?, m4andyfh__ag,	m4andyfh,	"ca4bd.p1",		0x0000, 0x010000, CRC(808b93e6) SHA1(08667db3f43f8550d7b96b53b92a53029a8d5d29), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 44)" )
M4ANDYFH_SET( 199?, m4andyfh__ah,	m4andyfh,	"ca4d.p1",		0x0000, 0x010000, CRC(9e3f10f1) SHA1(2386b960c8de6ca97bc96b69c11f8eb01188e27a), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 45)" )
M4ANDYFH_SET( 199?, m4andyfh__ai,	m4andyfh,	"ca4dk.p1",		0x0000, 0x010000, CRC(23cabc1a) SHA1(a72e14d7616da0b7cc394e466fe6df4c85eea986), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 46)" )
M4ANDYFH_SET( 199?, m4andyfh__aj,	m4andyfh,	"ca4dy.p1",		0x0000, 0x010000, CRC(caa2e461) SHA1(5dd3c609d1cc4bc43a6d00e98a71e927287c41fd), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 47)" )
M4ANDYFH_SET( 199?, m4andyfh__ak,	m4andyfh,	"ca4k.p1",		0x0000, 0x010000, CRC(78b0a533) SHA1(fa0fc59562be59d0aadba923281086a9de8e8934), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 48)" )
M4ANDYFH_SET( 199?, m4andyfh__al,	m4andyfh,	"ca4s.p1",		0x0000, 0x010000, CRC(ece1bca7) SHA1(84a168e0d36f7c4f56fc3a7579fe335cc1e5a5ba), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 49)" )
M4ANDYFH_SET( 199?, m4andyfh__am,	m4andyfh,	"ca4y.p1",		0x0000, 0x010000, CRC(5c1886f2) SHA1(4d6131989a04db993b7ade74d1950077d52cbc23), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 50)" )
M4ANDYFH_SET( 199?, m4andyfh__an,	m4andyfh,	"catad.p1",		0x0000, 0x010000, CRC(b2c5a227) SHA1(0c4253dddef07476778adf10b7afc8415ac2b170), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 51)" )
M4ANDYFH_SET( 199?, m4andyfh__ao,	m4andyfh,	"catb.p1",		0x0000, 0x010000, CRC(34007275) SHA1(102a30e4a83eec9ed144158cc4896c91f4eadd1b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 52)" )
M4ANDYFH_SET( 199?, m4andyfh__ap,	m4andyfh,	"catbd.p1",		0x0000, 0x010000, CRC(9c38229a) SHA1(1c24eff59e22d354e07f9a0655b35029a89a60ef), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 53)" )
M4ANDYFH_SET( 199?, m4andyfh__aq,	m4andyfh,	"catd.p1",		0x0000, 0x010000, CRC(ce909947) SHA1(a62cf731cc1309ff9f41777463e9ccc90313c401), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 54)" )
M4ANDYFH_SET( 199?, m4andyfh__ar,	m4andyfh,	"catdk.p1",		0x0000, 0x010000, CRC(c85a53ea) SHA1(1b7245b08bc0ad7ddd7ed4498ba4ac910f1df1d1), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 55)" )
M4ANDYFH_SET( 199?, m4andyfh__as,	m4andyfh,	"catdy.p1",		0x0000, 0x010000, CRC(08aa7a9c) SHA1(b8552ad4d9f1ea3c536ba7313ded56f9d47930d3), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 56)" )
M4ANDYFH_SET( 199?, m4andyfh__at,	m4andyfh,	"catk.p1",		0x0000, 0x010000, CRC(843a09b2) SHA1(56e845bcf940d80277b53df8fe847e5e862a05c9), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 57)" )
M4ANDYFH_SET( 199?, m4andyfh__au,	m4andyfh,	"cats.p1",		0x0000, 0x010000, CRC(e4cb7300) SHA1(fe9daaa587f1796227ad9ccb49869f2288b6d708), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 58)" )
M4ANDYFH_SET( 199?, m4andyfh__av,	m4andyfh,	"caty.p1",		0x0000, 0x010000, CRC(1a5c2413) SHA1(096695d99cc5dcf9d677b7821af5018751e21a89), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 59)" )
M4ANDYFH_SET( 199?, m4andyfh__aw,	m4andyfh,	"cauad.p1",		0x0000, 0x010000, CRC(a530d8ce) SHA1(9c919dfe85d947545e35c52b070dcb19ad3660ea), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 60)" )
M4ANDYFH_SET( 199?, m4andyfh__ax,	m4andyfh,	"caub.p1",		0x0000, 0x010000, CRC(4c6f35b7) SHA1(3300d3a8cdeda7183d066ff8fec2bdfbcd816f9b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 61)" )
M4ANDYFH_SET( 199?, m4andyfh__ay,	m4andyfh,	"caubd.p1",		0x0000, 0x010000, CRC(932cb584) SHA1(28dc617a242f01030d3bfdd855b2237b99dfb080), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 62)" )
M4ANDYFH_SET( 199?, m4andyfh__az,	m4andyfh,	"caud.p1",		0x0000, 0x010000, CRC(3b74131e) SHA1(569478d5e6f9c5db38db347d5c60be6aab8b1689), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 63)" )
M4ANDYFH_SET( 199?, m4andyfh__a0,	m4andyfh,	"caudk.p1",		0x0000, 0x010000, CRC(9d2ca094) SHA1(343304a09ae0aa36039b4b90f90ac7206f6b020b), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 64)" )
M4ANDYFH_SET( 199?, m4andyfh__a1,	m4andyfh,	"caudy.p1",		0x0000, 0x010000, CRC(b8e09c8e) SHA1(b9fc39f754dadfaf359f7df6e51a18e948eda574), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 65)" )
M4ANDYFH_SET( 199?, m4andyfh__a2,	m4andyfh,	"cauk.p1",		0x0000, 0x010000, CRC(38c7b3b0) SHA1(d5ee172e37e65911a4010abe7baae3e32131208d), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 66)" )
M4ANDYFH_SET( 199?, m4andyfh__a3,	m4andyfh,	"caus.p1",		0x0000, 0x010000, CRC(88e263a4) SHA1(2b8bc3d9aab344ca756b4829c4593db74200779e), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 67)" )
M4ANDYFH_SET( 199?, m4andyfh__a4,	m4andyfh,	"cauy.p1",		0x0000, 0x010000, CRC(b04ab546) SHA1(5f9d3a24fb0091406e45cdad7f22fad4bda27bff), "Barcrest","Andy's Full House (Barcrest) (MPU4) (set 68)" )


#define M4BDASH_EXTRA_ROMS \
	ROM_REGION( 0x48, "fakechr", 0 ) \
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) ) \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "blds1.hex", 0x000000, 0x080000, CRC(9cc07f5f) SHA1(e25295eb304624ed77d98d7e974363214c2c2cd1) ) \
	ROM_LOAD( "blds2.hex", 0x080000, 0x080000, CRC(949bee73) SHA1(9ea2001a4d91236708dc948b4e1cac9978095945) ) \

#define M4BDASH_SET(year, setname,parent,name,offset,length,hash,company,title) \
	ROM_START( setname ) \
		ROM_REGION( length, "maincpu", 0 ) \
		ROM_LOAD( name, offset, length, hash ) \
		M4BDASH_EXTRA_ROMS \
	ROM_END \
	GAME(year, setname, parent ,mod4oki	,mpu4 , mpu4_state,m4_showstring_big ,ROT0,company,title,GAME_FLAGS ) \



M4BDASH_SET( 199?, m4bdash,		0,			"bls01s.p1",	0x0000, 0x020000, CRC(4e4f403b) SHA1(f040568af530cf0ff060199f98b00e476191da22), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 1)" )
M4BDASH_SET( 199?, m4bdash__a,	m4bdash,	"bdvarg.bin",	0x0000, 0x020000, CRC(99d579e7) SHA1(afc47144e0a8d464d8547b1ad14b0a3a1c15c027), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 2)" )
M4BDASH_SET( 199?, m4bdash__b,	m4bdash,	"bld06s",		0x0000, 0x020000, CRC(0bc580b8) SHA1(432ac5aec08bd9d36cc4a0b257c17d6e22015bae), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 3)" )
M4BDASH_SET( 199?, m4bdash__c,	m4bdash,	"bld07ad.p1",	0x0000, 0x020000, CRC(56438185) SHA1(f78789042a1ac61b7dd333120b9fef76a2805cc7), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 4)" )
M4BDASH_SET( 199?, m4bdash__d,	m4bdash,	"bld07b.p1",	0x0000, 0x020000, CRC(4b24ec01) SHA1(80763e2832d9ef9c49f8729fbc93843865422d47), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 5)" )
M4BDASH_SET( 199?, m4bdash__e,	m4bdash,	"bld07bd.p1",	0x0000, 0x020000, CRC(db592f40) SHA1(bca6b78ea13ccab1f49d6b6078071739cb418778), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 6)" )
M4BDASH_SET( 199?, m4bdash__f,	m4bdash,	"bld07c.p1",	0x0000, 0x020000, CRC(7c6e5113) SHA1(814fb61aa64eecfa8b6d8e9c39a7b3b3287247dd), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 7)" )
M4BDASH_SET( 199?, m4bdash__g,	m4bdash,	"bld07d.p1",	0x0000, 0x020000, CRC(363fe777) SHA1(181b10e828b1308725cbe185c655c7deb7899cbe), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 8)" )
M4BDASH_SET( 199?, m4bdash__h,	m4bdash,	"bld07dh.p1",	0x0000, 0x020000, CRC(b299b28c) SHA1(3784cc7e33cd31c6ef5fd7fbc336b1b024a13993), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 9)" )
M4BDASH_SET( 199?, m4bdash__i,	m4bdash,	"bld07dk.p1",	0x0000, 0x020000, CRC(919e8716) SHA1(4be8b30a3db436fab8dfc9a131f2ca2b16ba6f7d), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 10)" )
M4BDASH_SET( 199?, m4bdash__j,	m4bdash,	"bld07dr.p1",	0x0000, 0x020000, CRC(22913c27) SHA1(33ed5a70b30d16fd607ec56a5ab085b55778c483), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 11)" )
M4BDASH_SET( 199?, m4bdash__k,	m4bdash,	"bld07dy.p1",	0x0000, 0x020000, CRC(167da7e8) SHA1(1cb81ad595ad5c7b70aed4f48ce9f6ae34d92089), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 12)" )
M4BDASH_SET( 199?, m4bdash__l,	m4bdash,	"bld07h.p1",	0x0000, 0x020000, CRC(22e471cd) SHA1(3e6e0a052761e1ed108475687dead185eef10119), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 13)" )
M4BDASH_SET( 199?, m4bdash__m,	m4bdash,	"bld07k.p1",	0x0000, 0x020000, CRC(01e34457) SHA1(5e9d8cb558222340df42904365ad90288ca5cdf2), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 14)" )
M4BDASH_SET( 199?, m4bdash__n,	m4bdash,	"bld07r.p1",	0x0000, 0x020000, CRC(b2ecff66) SHA1(9d8bca3e137a654d786b9257ce1206c7118ac6e0), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 15)" )
M4BDASH_SET( 199?, m4bdash__o,	m4bdash,	"bld07s.p1",	0x0000, 0x020000, CRC(b9c61540) SHA1(d6752d90a431cde17c7915746f645ef3157eeffe), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 16)" )
M4BDASH_SET( 199?, m4bdash__p,	m4bdash,	"bld07y.p1",	0x0000, 0x020000, CRC(860064a9) SHA1(8e13df769bde73bc5af3fa8010b39502e269f63f), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 17)" )
M4BDASH_SET( 199?, m4bdash__q,	m4bdash,	"bld10ad.p1",	0x0000, 0x020000, CRC(04b2781e) SHA1(828426d6191974050e3ccbfbc826d5474dc18312), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 18)" )
M4BDASH_SET( 199?, m4bdash__r,	m4bdash,	"bld10b.p1",	0x0000, 0x020000, CRC(160a6c83) SHA1(e2421fbc166b9e64a2b10afbfd12ebc724077248), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 19)" )
M4BDASH_SET( 199?, m4bdash__s,	m4bdash,	"bld10bd.p1",	0x0000, 0x020000, CRC(89a8d6db) SHA1(c93b8d7c57a970649204078f3428fc766ada32f7), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 20)" )
M4BDASH_SET( 199?, m4bdash__t,	m4bdash,	"bld10c.p1",	0x0000, 0x020000, CRC(2140d191) SHA1(151aba51cf0909f8bc3d252ef49a2ae2e96adf32), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 21)" )
M4BDASH_SET( 199?, m4bdash__u,	m4bdash,	"bld10d.p1",	0x0000, 0x020000, CRC(6b1167f5) SHA1(2aeb0fa0964867d90412bfd895da664d9be8a339), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 22)" )
M4BDASH_SET( 199?, m4bdash__v,	m4bdash,	"bld10dh.p1",	0x0000, 0x020000, CRC(e0684b17) SHA1(eb2832a3344aa9dfdc10c845faf3ae67171c40e9), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 23)" )
M4BDASH_SET( 199?, m4bdash__w,	m4bdash,	"bld10dk.p1",	0x0000, 0x020000, CRC(c36f7e8d) SHA1(1ee7bcca0cfdd27cd23328f60aa5230325db6366), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 24)" )
M4BDASH_SET( 199?, m4bdash__x,	m4bdash,	"bld10dr.p1",	0x0000, 0x020000, CRC(7060c5bc) SHA1(e409684b5f2494c1e580e4c9db001e89ef63ea1a), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 25)" )
M4BDASH_SET( 199?, m4bdash__y,	m4bdash,	"bld10dy.p1",	0x0000, 0x020000, CRC(448c5e73) SHA1(38193dbe23266d29344439d75e020b8236d34037), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 26)" )
M4BDASH_SET( 199?, m4bdash__z,	m4bdash,	"bld10h.p1",	0x0000, 0x020000, CRC(7fcaf14f) SHA1(e093bbaea83f7b2683b968b70d821ec42addab92), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 27)" )
M4BDASH_SET( 199?, m4bdash__0,	m4bdash,	"bld10k.p1",	0x0000, 0x020000, CRC(5ccdc4d5) SHA1(b7be6f027092106ef5e33ff988a153050f48943b), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 28)" )
M4BDASH_SET( 199?, m4bdash__1,	m4bdash,	"bld10r.p1",	0x0000, 0x020000, CRC(efc27fe4) SHA1(1ed3c5c92505b7fdf4993a9c8b119eff5e9a6f94), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 29)" )
M4BDASH_SET( 199?, m4bdash__2,	m4bdash,	"bld10s.p1",	0x0000, 0x020000, CRC(c59c186b) SHA1(83f16e15a215fe1cf3c07fac7268b00c55e0ff5b), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 30)" )
M4BDASH_SET( 199?, m4bdash__3,	m4bdash,	"bld10y.p1",	0x0000, 0x020000, CRC(db2ee42b) SHA1(b6a4bb4f78c14428a7bd2286b8fda51acb0c9e10), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 31)" )
M4BDASH_SET( 199?, m4bdash__4,	m4bdash,	"bls01ad.p1",	0x0000, 0x020000, CRC(2425cab4) SHA1(9df08f9dffc0ac5fe5994ad086e6b8eb8d03baa9), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 32)" )
M4BDASH_SET( 199?, m4bdash__5,	m4bdash,	"bls01b.p1",	0x0000, 0x020000, CRC(64d1e31d) SHA1(e30d199bd1d60ceabef27cfa81605eb3b307f68e), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 33)" )
M4BDASH_SET( 199?, m4bdash__6,	m4bdash,	"bls01bd.p1",	0x0000, 0x020000, CRC(a93f6471) SHA1(d895c7825c713626be57dd9eef2dbfc5f591825b), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 34)" )
M4BDASH_SET( 199?, m4bdash__7,	m4bdash,	"bls01c.p1",	0x0000, 0x020000, CRC(539b5e0f) SHA1(46abc7e8dd286fbb313233b6d2fabb73b1c4c519), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 35)" )
M4BDASH_SET( 199?, m4bdash__8,	m4bdash,	"bls01d.p1",	0x0000, 0x020000, CRC(19cae86b) SHA1(de811aed861bf4d834051d6048fb0028f6c68a7c), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 36)" )
M4BDASH_SET( 199?, m4bdash__9,	m4bdash,	"bls01dh.p1",	0x0000, 0x020000, CRC(c0fff9bd) SHA1(5b10547157a96f6221a41118ee29639ffc1bcdab), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 37)" )
M4BDASH_SET( 199?, m4bdash__aa,	m4bdash,	"bls01dk.p1",	0x0000, 0x020000, CRC(e3f8cc27) SHA1(4ffb6ebb792b7edbf88463ed0fb80743aab5c517), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 38)" )
M4BDASH_SET( 199?, m4bdash__ab,	m4bdash,	"bls01dr.p1",	0x0000, 0x020000, CRC(50f77716) SHA1(a20b709f65e2722e4fb622e851b4fb9c7d5820ca), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 39)" )
M4BDASH_SET( 199?, m4bdash__ac,	m4bdash,	"bls01dy.p1",	0x0000, 0x020000, CRC(641becd9) SHA1(419486ed63bd8d657796f786e73d44bd213b2916), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 40)" )
M4BDASH_SET( 199?, m4bdash__ad,	m4bdash,	"bls01h.p1",	0x0000, 0x020000, CRC(0d117ed1) SHA1(8b0a4f24b067907377072e5b5645ac8b44bf1d2e), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 41)" )
M4BDASH_SET( 199?, m4bdash__ae,	m4bdash,	"bls01k.p1",	0x0000, 0x020000, CRC(2e164b4b) SHA1(37d1c45db0002c7e8f16ede87cfe62cfbdbf39e8), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 42)" )
M4BDASH_SET( 199?, m4bdash__af,	m4bdash,	"bls01r.p1",	0x0000, 0x020000, CRC(9d19f07a) SHA1(bb1e2d8f6e1fd75d6c9a15448fc29b21d8f14bf7), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 43)" )
M4BDASH_SET( 199?, m4bdash__ag,	m4bdash,	"bls01y.p1",	0x0000, 0x020000, CRC(a9f56bb5) SHA1(771ea854bdc71af0ce09952be53671629babfa9b), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 44)" )
M4BDASH_SET( 199?, m4bdash__ah,	m4bdash,	"bls02ad.p1",	0x0000, 0x020000, CRC(f4b6828b) SHA1(8ca39a9dc29b40a097489e34ababaf70eb58c326), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 45)" )
M4BDASH_SET( 199?, m4bdash__ai,	m4bdash,	"bls02b.p1",	0x0000, 0x020000, CRC(d75f9bdb) SHA1(2018e1ebe4f00782be649544bc8d56d923d6c198), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 46)" )
M4BDASH_SET( 199?, m4bdash__aj,	m4bdash,	"bls02bd.p1",	0x0000, 0x020000, CRC(79ac2c4e) SHA1(83d6828272438ae0d687b40368701cbccac32d9f), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 47)" )
M4BDASH_SET( 199?, m4bdash__ak,	m4bdash,	"bls02c.p1",	0x0000, 0x020000, CRC(e01526c9) SHA1(1bc17c22e0741f6dbf1c3ad2e154ce8acc1e1788), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 48)" )
M4BDASH_SET( 199?, m4bdash__al,	m4bdash,	"bls02d.p1",	0x0000, 0x020000, CRC(aa4490ad) SHA1(963482e4977309babf35732be0b7046c543808d3), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 49)" )
M4BDASH_SET( 199?, m4bdash__am,	m4bdash,	"bls02dh.p1",	0x0000, 0x020000, CRC(106cb182) SHA1(1556392744646d8852cc82975dd94df250d54bfa), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 50)" )
M4BDASH_SET( 199?, m4bdash__an,	m4bdash,	"bls02dk.p1",	0x0000, 0x020000, CRC(336b8418) SHA1(c42a29c7599c2d8025b8e25fcda0034100a43835), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 51)" )
M4BDASH_SET( 199?, m4bdash__ao,	m4bdash,	"bls02dr.p1",	0x0000, 0x020000, CRC(80643f29) SHA1(e9128f78e7051274dbaab28051a1f23ed2426c3c), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 52)" )
M4BDASH_SET( 199?, m4bdash__ap,	m4bdash,	"bls02dy.p1",	0x0000, 0x020000, CRC(b488a4e6) SHA1(7a9083cfa9032cb0e6e28c22dde8da9a1bebadcf), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 53)" )
M4BDASH_SET( 199?, m4bdash__aq,	m4bdash,	"bls02h.p1",	0x0000, 0x020000, CRC(be9f0617) SHA1(fcc491ae5cb5312f47726c4b9ffda99317171bab), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 54)" )
M4BDASH_SET( 199?, m4bdash__ar,	m4bdash,	"bls02k.p1",	0x0000, 0x020000, CRC(9d98338d) SHA1(0e43896ae8361894c9060ec7a74dd23c6e2bed56), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 55)" )
M4BDASH_SET( 199?, m4bdash__as,	m4bdash,	"bls02r.p1",	0x0000, 0x020000, CRC(2e9788bc) SHA1(586a30b3485e0ceb8b9b389e103fdbab78115446), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 56)" )
M4BDASH_SET( 199?, m4bdash__at,	m4bdash,	"bls02s.p1",	0x0000, 0x020000, CRC(b8e435d5) SHA1(500c30d687d3e029f22de2bf132c12349c1575b4), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 57)" )
M4BDASH_SET( 199?, m4bdash__au,	m4bdash,	"bls02y.p1",	0x0000, 0x020000, CRC(1a7b1373) SHA1(dde4754d92f0fde495ab826294a650ac81fd586e), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 58)" )
M4BDASH_SET( 199?, m4bdash__av,	m4bdash,	"bold15g",		0x0000, 0x020000, CRC(fa400d34) SHA1(2faeb9b880fb4980aa0d96b4b962c879498445f2), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 59)" )
M4BDASH_SET( 199?, m4bdash__aw,	m4bdash,	"bold15t",		0x0000, 0x020000, CRC(f3f331ae) SHA1(d999c8571549d8d26b7b861299d77c7282aef700), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 60)" )
M4BDASH_SET( 199?, m4bdash__ax,	m4bdash,	"bo__x__x.2_0",	0x0000, 0x020000, CRC(7e54982f) SHA1(c5187d2f6a5b202af5fd6326d52451d3b3f48f33), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 61)" )
M4BDASH_SET( 199?, m4bdash__ay,	m4bdash,	"bo__x__x.2_1",	0x0000, 0x020000, CRC(3e48d8ad) SHA1(73d69712993819d012c2ab2a8a36b7ebad419144), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 62)" )
M4BDASH_SET( 199?, m4bdash__az,	m4bdash,	"bo__x_dx.2_0",	0x0000, 0x020000, CRC(d0d9e7b1) SHA1(31e858991fc1dfe9c1a8bd7955096617ebe0a4ce), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 63)" )
M4BDASH_SET( 199?, m4bdash__a0,	m4bdash,	"bo__x_dx.2_1",	0x0000, 0x020000, CRC(b6e146c4) SHA1(8bda363f16bd258d5c6ba1b20cecc0a76e0965f7), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 64)" )
M4BDASH_SET( 199?, m4bdash__a1,	m4bdash,	"bo__xa_x.2_0",	0x0000, 0x020000, CRC(e7054491) SHA1(7d102b1071d90ff29ea4a9418478b17b93c08059), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 65)" )
M4BDASH_SET( 199?, m4bdash__a2,	m4bdash,	"bo__xa_x.2_1",	0x0000, 0x020000, CRC(813de5e4) SHA1(498923261e49b20666a930593fcf25ccfc9a9d79), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 66)" )
M4BDASH_SET( 199?, m4bdash__a3,	m4bdash,	"bo__xb_x.2_0",	0x0000, 0x020000, CRC(adc2ecc7) SHA1(75e4216ff022c1ae0642913c9aaa7e241b806fcd), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 67)" )
M4BDASH_SET( 199?, m4bdash__a4,	m4bdash,	"bo__xb_x.2_1",	0x0000, 0x020000, CRC(cbfa4db2) SHA1(d1ed60f876b4f056f478cfc23b08a7789379e143), "Barcrest","Boulder Dash (Barcrest) (MPU4) (set 68)" )


