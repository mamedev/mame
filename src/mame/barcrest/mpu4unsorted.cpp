// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    These are MPU4 based machines that are still in need of correct placement.

    Some of them appear to be official Barcrest machines, and will likely end up
    being moved to the mod2/mod4 files based on hardware types.

    Official machines / licensed machines can typically be identified by the
    standard Barcrest 'characteriser' protection sequence being written.

    Other machines in this files are unofficial machines, maybe one-off units
    that will require individual attention.
*/

#include "emu.h"
#include "mpu4.h"


namespace {

class mpu4unsorted_state : public mpu4_state
{
public:

	mpu4unsorted_state(const machine_config& mconfig, device_type type, const char* tag) :
		mpu4_state(mconfig, type, tag)
	{
	}

	void init_m4aao();
	void init_m4test();
};

void mpu4unsorted_state::init_m4test()
{
	init_m4default();
	m_overcurrent_detect = true;
}

#include "m4aao.lh"


ROM_START( m4tst2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut2.p1",  0xe000, 0x2000,  CRC(f7fb6575) SHA1(f7961cbd0801b9561d8cd2d23081043d733e1902))
ROM_END

ROM_START( m4clr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "meter-zero.p1",  0x8000, 0x8000,  CRC(e74297e5) SHA1(49a2cc85eda14199975ec37a794b685c839d3ab9))
ROM_END

ROM_START( m4rltst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rtv.p1", 0x08000, 0x08000, CRC(7b78f3f2) SHA1(07ef8e6a08fd70ee48e4463672a1230ecc669532) )
ROM_END


ROM_START( m4addrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dal12.bin", 0x0000, 0x010000, CRC(4affa79a) SHA1(68bceab42b3616641a34a64a83306175ffc1ce32) )
ROM_END


ROM_START( m4amhiwy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dah20", 0x0000, 0x010000, CRC(e3f92f00) SHA1(122c8a429a1f75dac80b90c4f218bd311813daf5) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdr6_1.snd", 0x000000, 0x080000, CRC(63ad952d) SHA1(acc0ac3898fcc281e2d7ba19ada52d727885fe06) )
	ROM_LOAD( "sdr6_2.snd", 0x080000, 0x080000, CRC(48d2ace5) SHA1(ada0180cc60266c0a6d981a019d66bbedbced21a) )
ROM_END


ROM_START( m4blkwhd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbw11.bin", 0x0000, 0x010000, CRC(337aaa2c) SHA1(26b12ea3ada9668293c6b44d62458590e5b4ac8f) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bwsnd.bin", 0x0000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
ROM_END





ROM_START( m4blkcat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbl14.bin", 0x0000, 0x010000, CRC(c5db9532) SHA1(309b5122b4a1cb33bbccfb97faf4fa996d29432e) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "dblcsnd.bin", 0x0000, 0x080000, CRC(c90fa8ad) SHA1(a98f03d4b6f5892333279bff7537d4d6d887da62) )

	ROM_REGION( 0x200000, "msm6376_alt", 0 ) // bad dump of some sound rom?
	ROM_LOAD( "sdbl_1.snd", 0x0000, 0x18008e, CRC(e36f71ae) SHA1(ebb643cfa02d28550f2bef135ceefc902baf0df6) )
ROM_END


ROM_START( m4bluedm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbd10.bin", 0x0000, 0x010000, CRC(b75e319d) SHA1(8b81e852e318cfde1f5ff2123e1ef7076b208253) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bdsnd.bin", 0x0000, 0x080000, CRC(8ac4aae6) SHA1(70dba43b398010a8bd0d82cf91553d3f5e0921f0) )
ROM_END



ROM_START( m4casmul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casinomultiplay.bin", 0x0000, 0x010000, CRC(2ebd1800) SHA1(d15e2593d17d8db9c6946af3366cf429ad291f76) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "casinomultiplaysnd.bin", 0x0000, 0x080000, CRC(be293e95) SHA1(bf0d419c898920a7546b542d8b205e25004ef04f) )
ROM_END

ROM_START( m4oldtmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dot11.bin",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e))

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdot01.bin", 0x0000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
ROM_END

ROM_START( m4casot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casrom.bin",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e) ) // == old timer   (aka b&wrom.bin)

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cassound.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) ) // ( aka b&wsound.bin )
ROM_END

ROM_START( m4jpmcla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jcv2.epr",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e) ) // == old timer

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sjcv2.snd", 0x0000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
ROM_END


ROM_START( m4ceptr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dce10.bin", 0x0000, 0x010000, CRC(c94d41ef) SHA1(58fdff2de8dd3ead3980f6f34362183d084ce917) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cepsnd.p1", 0x000000, 0x080000, CRC(3a91784a) SHA1(7297ccec3264aa9f1e7b3a2841f5f8a1e4ca6c54) )
	ROM_LOAD( "cepsnd.p2", 0x080000, 0x080000, CRC(a82f0096) SHA1(45b6b5a2ae06b45add9cdbb9f5e6f834687b4902) )
ROM_END





ROM_START( m4crzjk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crjok2.04.bin", 0x0000, 0x010000, CRC(838336d6) SHA1(6f36de20c930cbbff479af2667c11152c6adb43e) )
ROM_END




#define M4DRAC_EXTRAS \
	ROM_REGION( 0x200000, "msm6376", 0 ) \
	ROM_LOAD( "drasnd.p1", 0x000000, 0x080000, CRC(54c3821c) SHA1(1fcc62e2b127dd7f1d5d27a3afdf56dc27f122f8) ) \
	ROM_LOAD( "drasnd.p2", 0x080000, 0x080000, CRC(9096d2bc) SHA1(1b4c530b7b0fde869980d519255e2585c5461e13) ) \
	ROM_LOAD( "drasnd.p3", 0x100000, 0x080000, CRC(a07f412b) SHA1(cca8f5cfe620ece45ca40bf801f0643cd76547e9) ) \
	ROM_LOAD( "drasnd.p4", 0x180000, 0x080000, CRC(018ed789) SHA1(64202da2c542f5ef208faeb04945eb1a758d4746) )

ROM_START( m4drac )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dra21.bin", 0x0000, 0x020000, CRC(23be387e) SHA1(08a78f4b8ddef46069d1c75113300b21e52338c1) )
	M4DRAC_EXTRAS
ROM_END

ROM_START( m4draca )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dra24.bin", 0x0000, 0x020000, CRC(3db112ae) SHA1(b5303e2a65476931d4769327ca62afd0f6a9eda7) )
	M4DRAC_EXTRAS
ROM_END

ROM_START( m4dracb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dra27.p1", 0x0000, 0x020000, CRC(8a095175) SHA1(41006e298f1688499ce6820ec28196c7578684b9) )
	M4DRAC_EXTRAS
ROM_END

ROM_START( m4exgam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "czep30.bin", 0x0000, 0x010000, CRC(4614e6f6) SHA1(5602a68e9b47394cb31bbcd49a9920e19af6242f) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sczep.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END




ROM_START( m4frtgm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruit.bin", 0x0000, 0x010000, CRC(dbe44316) SHA1(15cd49dd2e6166f7a7668663f7fea802d6cbb12f) )

	ROM_REGION( 0x800000, "msm6376", 0 ) // this isn't OKI, or is corrupt (bad size)
	ROM_LOAD( "fruitsnd.bin", 0x0000, 0x010000, CRC(86547dc7) SHA1(4bf64f22e84c0ee82d961b0ba64932b8bf6a521f) ) // matches 'Replay' on SC1 hardware, probably just belongs there.. or this is eurocoin with different sound hw here?
ROM_END



ROM_START( m4gldgat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgg22.bin", 0x0000, 0x010000, CRC(ef8498df) SHA1(6bf164ef18445e83e4510a000bc924cbe916ad99) )
ROM_END

ROM_START( m4gldjok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgj12.bin", 0x0000, 0x010000, CRC(93ee0c35) SHA1(5ae67b14f7f3d8528fa106519a8a27437c997a70) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdgj.snd", 0x0000, 0x080000, CRC(b6cd118b) SHA1(51c5d694ed0dfde8d3fd682f2471d83eec236736) )
ROM_END



ROM_START( m4gnsmk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgu16", 0x0000, 0x010000, CRC(6aa23345) SHA1(45e129ec95b1a796f334bedd08469f2ab47a18f8) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "sdgu01.s1", 0x000000, 0x080000, CRC(bfb284a2) SHA1(860b98d54a3180fbb00b7b03feae049fb4cf9d7f) )
	ROM_LOAD( "sdgu01.s2", 0x080000, 0x080000, CRC(1a46ba28) SHA1(d7154e5f92be8631207620eb313b28990c6a1c7f) )
	ROM_LOAD( "sdgu01.s3", 0x100000, 0x080000, CRC(88bffcf4) SHA1(1da853193f6a22889edff5aafd9440c676a82ea6) )
	ROM_LOAD( "sdgu01.s4", 0x180000, 0x080000, CRC(a6160bef) SHA1(807f7d470728a479a55c782fca3df1eacd0b594c) )
ROM_END

ROM_START( m4blkbuld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgu16", 0x0000, 0x010000, CRC(6aa23345) SHA1(45e129ec95b1a796f334bedd08469f2ab47a18f8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dbbsnd.p1", 0x000000, 0x080000, CRC(a913ad0d) SHA1(5f39b661912da903ce8d6658b7848081b191ea56) )
	ROM_LOAD( "dbbsnd.p2", 0x080000, 0x080000, CRC(6a22b39f) SHA1(0e0dbeac4310e03490b665fff514392481ad265f) )
ROM_END


ROM_START( m4hpyjok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dhj12", 0x0000, 0x010000, CRC(982439d7) SHA1(8d27fcecf7a6a7fd774678580074f945675758f4) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dhjsnd", 0x0000, 0x080000, CRC(8ac4aae6) SHA1(70dba43b398010a8bd0d82cf91553d3f5e0921f0) )
ROM_END


ROM_START( m4holdtm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dht10.hex", 0x0000, 0x010000, CRC(217d382b) SHA1(a27dd107c554d4787967633dff998d3962ee0ea5) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sun01.hex", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END


ROM_START( m4jok300 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cjo", 0x0000, 0x020000, CRC(386e99db) SHA1(5bb0b513ef63ffaedd98b8e9e7206658fe784fda) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASEFF )
	// missing?
ROM_END

ROM_START( m4jokmil )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cjm03.epr", 0x0000, 0x020000, CRC(e5e4986e) SHA1(149b950a739ad308f7759927c344de8193ce67c5) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASEFF )
	// missing?
ROM_END


ROM_START( m4joljokh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jollyjokerhungarian.bin", 0x0000, 0x010000, CRC(85b6a406) SHA1(e277f9d3b62faead04d65efbc06de7f4a50ae38d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "jollyjokerhungariansnd.bin", 0x0000, 0x080000, CRC(93460383) SHA1(2b179a1dde09ebdfe8c84641899df7be87d443e5) )
ROM_END





ROM_START( m4luck7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dl716.bin", 0x0000, 0x010000, CRC(141b23a9) SHA1(3bfb82ea0ee4104bd8739b545aba617f84bef770) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dl7snd.bin", 0x0000, 0x080000, CRC(c90fa8ad) SHA1(a98f03d4b6f5892333279bff7537d4d6d887da62) )
ROM_END

ROM_START( m4luckdv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cld_16.bin", 0x0000, 0x010000, CRC(89f63938) SHA1(8d3a5628e2c0bf39784afe2f00a007d40ea35423) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cld_snd1.snd", 0x000000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
	ROM_LOAD( "cld_snd2.snd", 0x080000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END

ROM_START( m4luckdvd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dld13", 0x0000, 0x010000, CRC(b8ceb29b) SHA1(84b6ebad300214610635fb8141d18de2b7065435) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdld01.snd", 0x000000, 0x080000, CRC(9b035fa6) SHA1(51b7e5bc3abdf4f1beba2347146a91a2b3f4de35) )
ROM_END


#define M4LUCKWB_EXTRAS \
	ROM_REGION( 0x100000, "msm6376", 0 ) /* these are all different sound roms... */  \
	ROM_LOAD( "lwbs3.bin", 0x0000, 0x07dc89, CRC(ee102376) SHA1(3fed581a4654acf285dd430fbfbac33cd67411b8) ) \
	ROM_LOAD( "lwbs7.bin", 0x0000, 0x080000, CRC(5d4177c7) SHA1(e13f145885bb719b0021ae4ce289261a3eaa2e18) ) \
	ROM_LOAD( "lwbs8.bin", 0x0000, 0x080000, CRC(187cdf5b) SHA1(87ec189af27c95f278a7531ec13df53a08889af8) ) \
	ROM_LOAD( "lwbs9.bin", 0x0000, 0x080000, CRC(2e02b617) SHA1(2502a1d2cff155a7fc5148e23a4723d4d60e9d42) )

ROM_START( m4luckwb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb10.bin", 0x0000, 0x010000, CRC(6d43a14e) SHA1(267aba1a01bfd5f0eaa7683d041d5fcb2d301934) )
	M4LUCKWB_EXTRAS
ROM_END

ROM_START( m4luckwba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb15.bin", 0x0000, 0x010000, CRC(b5af8cb2) SHA1(474975b83803627ad3ac4217d8cecb2d2db16fec) )
	M4LUCKWB_EXTRAS
ROM_END

ROM_START( m4luckwbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb21.bin", 0x0000, 0x010000, CRC(6c570733) SHA1(7488318ca9689371e4f80be0a0fddd8ad141733e) )
	M4LUCKWB_EXTRAS
ROM_END

ROM_START( m4luckwbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb22.bin", 0x0000, 0x010000, CRC(05b952a7) SHA1(952e328b280a18c1ffe253b6a56f2b5e893b1b72) )
	M4LUCKWB_EXTRAS
ROM_END

ROM_START( m4luckwbd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb27.bin", 0x0000, 0x010000, CRC(9d6b6637) SHA1(65bad12cd08de128ca31c9488e32e3cebfb8eedb) )
	M4LUCKWB_EXTRAS
ROM_END

ROM_START( m4luckwbe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb6.bin", 0x0000, 0x010000, CRC(8e7d4594) SHA1(4824a9a4628585a170c41e00f7b3fcb8a2330c02) )
	M4LUCKWB_EXTRAS
ROM_END

ROM_START( m4luckwbf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb7.bin", 0x0000, 0x010000, CRC(8e651705) SHA1(bd4d09d586d14759a17d4d7d4016c427f3eef015) )
	M4LUCKWB_EXTRAS
ROM_END


ROM_START( m4maglin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dma21.bin", 0x0000, 0x010000, CRC(836a25e6) SHA1(5f83bb8a2c77dd3b02724c076d6b37d2c1c93b93) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mlsound1.p1", 0x000000, 0x080000, CRC(ff8749ff) SHA1(509b53f09cdfe5ee865e60ab42fd578586ac53ea) )
	ROM_LOAD( "mlsound2.p2", 0x080000, 0x080000, CRC(c8165b6c) SHA1(7c5059ee8630da31fc3ad50d84a4730297757d46) )
ROM_END

ROM_START( m4magrep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dmr13.bin", 0x0000, 0x010000, CRC(c3015da3) SHA1(23cd505eedf666c012e4064a5fcf5a983f098e83) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mrdsound.bin", 0x000000, 0x080000, CRC(9b035fa6) SHA1(51b7e5bc3abdf4f1beba2347146a91a2b3f4de35) )
ROM_END



ROM_START( m4nile )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gjn08.p1", 0x0000, 0x020000, CRC(2bafac0c) SHA1(363d08f798b5bea409510b1a9415098a69f19ee0) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "gjnsnd.p1", 0x000000, 0x080000, CRC(1d839591) SHA1(2e4ba74f96e7c0592b85409a3f50ec81e00e064c) )
	ROM_LOAD( "gjnsnd.p2", 0x080000, 0x080000, CRC(e2829c42) SHA1(2139c1625ad163cce99a522c2cf02ee47a8f9007) )
	ROM_LOAD( "gjnsnd.p3", 0x100000, 0x080000, CRC(db084eb4) SHA1(9b46a3cb16974942b0edd25b1b080d30fc60c3df) )
	ROM_LOAD( "gjnsnd.p4", 0x180000, 0x080000, CRC(da785b0a) SHA1(63358ab197eb1de8e489a9fd6ffbc2039efc9536) )
ROM_END




ROM_START( m4ordmnd )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rab01.p1", 0x0000, 0x020000, CRC(99964fe7) SHA1(3745d09e7a4f417c8e85270d3ffec3e37ee1344d) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "odsnd1.bin", 0x000000, 0x080000, CRC(d746bae4) SHA1(293e1dc9edf88a183cc23dbb4576cefbc8f9d028) )
	ROM_LOAD( "odsnd2.bin", 0x080000, 0x080000, CRC(84ace1f4) SHA1(9cc70e59e9d26006870ea1cc522de33e71b71692) )
	ROM_LOAD( "odsnd3.bin", 0x100000, 0x080000, CRC(b1b12def) SHA1(d8debf8cfb3af2157d5d1571927588dc1c8d07b6) )
ROM_END




ROM_START( m4prem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dpm14.bin", 0x0000, 0x010000, CRC(de344759) SHA1(d3e7514da83bbf1eba63661fb0675a6230af93cd) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dpms.bin", 0x0000, 0x080000, CRC(93fd4253) SHA1(69feda7ffc56defd515c9cd1ce204af3d9731a3f) )
ROM_END


ROM_START( m4rdht )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drh12", 0x0000, 0x010000, CRC(b26cd308) SHA1(4e29f6cce773232a1c43cd2fb3ce9b844c446bb8) ) // aka gdjb

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "drh_1.snd", 0x0000, 0x080000, CRC(f652cd0c) SHA1(9ce986bc12bcf22a57e065329e82671d19cc96d7) ) // aka gn.snd
ROM_END


ROM_START( m4rwb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drw14.bin", 0x0000, 0x010000, CRC(22c30ebe) SHA1(479f66732aac56dae60c80d11f05c084865f9389) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "rwb_1.snd", 0x000000, 0x080000, CRC(e0a6def5) SHA1(e3867b83e588fd6a9039b8d45186480a9d0433ea) )
	ROM_LOAD( "rwb_2.snd", 0x080000, 0x080000, CRC(54a2b2fd) SHA1(25875ff873bf22df510e7a4c56c336fbabcbdedb) )
ROM_END


ROM_START( m4ringfr )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rof03s.p1", 0x0000, 0x020000, CRC(4b4703fe) SHA1(853ce1f5932e09af2b5f3b5314709f13aa35cf19) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// missing?
ROM_END


ROM_START( m4roadrn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dro19", 0x0000, 0x010000, CRC(8b591766) SHA1(df156390b427e31cdda64826a6c1d2457c915f25) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dro_1.snd", 0x000000, 0x080000, CRC(895cfe63) SHA1(02134e149cef3526bbdb6cb93ef3efa283b9d6a2) )
	ROM_LOAD( "dro_2.snd", 0x080000, 0x080000, CRC(1d5c8d4f) SHA1(15c18ae7286807cdc0feb825b958eae808445690) )
ROM_END


ROM_START( m4royjwl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rj.bin", 0x0000, 0x020000, CRC(3ffbe4a8) SHA1(47a0309cc9ff315ad9f64e6855863409443e94e2) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "rj_sound1.bin", 0x000000, 0x080000, CRC(443c4901) SHA1(7b3c6737b47dfe04c072f0e157d83c09340c3f9b) )
	ROM_LOAD( "rj_sound2.bin", 0x080000, 0x080000, CRC(9456523e) SHA1(ea1b6bf16b7d1015c188ad83760336d9851de391) )
ROM_END


ROM_START( m4salsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsa15.epr", 0x0000, 0x010000, CRC(22b60b0b) SHA1(4ad184d1557bfd01650684ea9d8ad794fded65f7) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dsa_1@97c2.snd", 0x0000, 0x080000, CRC(0281a6dd) SHA1(a35a8cd0da32c51f77856ea3eeff7c58fd032333) )
ROM_END


ROM_START( m4showtm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsh13.bin", 0x0000, 0x010000, CRC(4ce40ff1) SHA1(f145d6c8e926ca4368d43dacda0fa38615988d84) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdsh01s1.snd", 0x0000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
ROM_END


ROM_START( m4steptm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dst11.bin", 0x0000, 0x010000, CRC(3960f210) SHA1(c7c4fe74cb9a53eaa9114a84240de3bce4ffe75e) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "sdun01.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END




#define M4TECHNO_EXTRAS \
	ROM_REGION( 0x080000, "msm6376", 0 ) \
	ROM_LOAD( "techno.bin", 0x0000, 0x080000, CRC(3e80f8bd) SHA1(2e3a195b49448da11cc0c089a8a9b462894c766b) )

ROM_START( m4techno )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dte13.bin", 0x0000, 0x010000, CRC(cf661d06) SHA1(316b2c42e7253a03b2c12b713821045d9f95a8a7) )
	M4TECHNO_EXTRAS
ROM_END

ROM_START( m4technoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dte13hack.bin", 0x0000, 0x010000, CRC(8b8eafe3) SHA1(93a7714eb4c749b7b19f4f844cf88da9443b0bb7) )
	M4TECHNO_EXTRAS
ROM_END


ROM_START( m4toma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtk23.bin", 0x0000, 0x010000, CRC(ffba2b96) SHA1(c7635023ac5181e661e808c6b44ac1add58f4f56) )
ROM_END


ROM_START( m4topdk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtd26pj.bin", 0x0000, 0x010000, CRC(1f84d995) SHA1(7412632cf79008b980e48f14aea89c3f8d742ed2) )
ROM_END




#define M4TOPTIM_EXTRAS \
	ROM_REGION( 0x080000, "msm6376", 0 ) \
	ROM_LOAD( "toptimer-snd.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )

ROM_START( m4toptim )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "toptimer.bin", 0x0000, 0x010000, CRC(74804012) SHA1(0d9460ba6b1d359d358483c4e8bfd5518f364518) )
	M4TOPTIM_EXTRAS
ROM_END


ROM_START( m4toptima )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtt2-1.bin", 0x0000, 0x010000, CRC(f9c84a34) SHA1(ad654442f580d6a49658f0e4e39bacbd9d0d0018) )
	M4TOPTIM_EXTRAS
ROM_END




#define M4TBPLAY_EXTRAS \
	ROM_REGION( 0x100000, "msm6376", 0 ) \
	ROM_LOAD( "dtps10_1", 0x000000, 0x080000, CRC(d1d2c981) SHA1(6a4940248b0bc8df0a9de0d60e98cfebf1962504) ) \
	ROM_LOAD( "dtps20_1", 0x080000, 0x080000, CRC(f77c4f39) SHA1(dc0e056f4d8c00824b3e672a02da64613bbf204e) )

ROM_START( m4tbplay )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtp13", 0x0000, 0x010000, CRC(de424bc3) SHA1(c82dd56a0b3ccea78325cd90ed8e72ed68a1af77) )
	M4TBPLAY_EXTRAS
ROM_END

ROM_START( m4tbplaya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rmtp4b", 0x0000, 0x010000, CRC(33a1764e) SHA1(7475f460dee015a2cd78fc3e0d1d14fd96fdbb9c) )
	M4TBPLAY_EXTRAS
ROM_END

ROM_START( m4tbplayb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trmyid", 0x0000, 0x010000, CRC(e7af5944) SHA1(64559c97375a3536f7929d7f4d8d19c30527a3ec) )
	M4TBPLAY_EXTRAS
ROM_END


ROM_START( m4twintm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d2t11.bin", 0x0000, 0x010000, CRC(6a76ac6f) SHA1(824912ff1fc3155d11d32b597be53481532fdf5e) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "sdun01.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END


ROM_START( m4twist )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twist_again_mk29-6", 0x8000, 0x008000, CRC(cb331bee) SHA1(a88099a3f35caf02925f1a3f548fbf65c11e3ec9) )
ROM_END

ROM_START( m4twista )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twistagain-98-mkii.bin", 0x8000, 0x008000, CRC(1cbc7b58) SHA1(eda998a64272fe6796243c2db48ef988b9668c35) )
ROM_END

ROM_START( m4twistb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twistagain-mki-27.bin", 0x8000, 0x008000, CRC(357f7072) SHA1(8a23509fff79a83a819b27eff8de8db08c679e3f) )
ROM_END

ROM_START( m4univ )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dun20", 0x0000, 0x010000, CRC(6a845d4d) SHA1(82bfc3f3a0ede76a4d482efc71b0390610db7acf) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "sdun01.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END


ROM_START( m4vegastg )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "vs.p1", 0x0000, 0x020000, CRC(4099d572) SHA1(91a7c1575013e61c754b2c2cb841e7687b76d7f9) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "vssound.bin", 0x0000, 0x16ee37, CRC(456da6be) SHA1(f0e293f0a383878b581326f869c2e49bec61d0c5) )
ROM_END


ROM_START( m4vivalvd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dlv11.bin", 0x0000, 0x010000, CRC(a890184c) SHA1(26d9952bf2eb4b55d21cdb934ffc73ff1a1cfbac) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "vegssnd.bin", 0x0000, 0x080000, CRC(93fd4253) SHA1(69feda7ffc56defd515c9cd1ce204af3d9731a3f) )
ROM_END


ROM_START( m4wildtm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wildtimer.bin", 0x0000, 0x010000, CRC(5bd54924) SHA1(23fcf13c52ee7b9b39f30f999a9102171fffd642) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "wildtimer-snd.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END


/* Vifico MPU4 games.
   Escalera Tobogan use the "Barcrest Sampled Sound" game PCB:

  BARCREST SAMPLED SOUND
 _________________________
 | ·                      |
 | ·                      |
 | ·                      |
 | ·                      |_____________
 | ·           _________   _________  |_|
 |             SN74LS139N  |_A880440| |_|
 |             _____________________  |_|
 |             | ST EF68B21P        | |_|
 |             |____________________| |_|
 |   _______         _______________  |_|
 |  |  OKI  |       | PROG EPROM    | |_|
 |  | M6376 |       |_______________| |_|
 |  |_______|         ______________  |_|
 |                   | ST EF68B40P  | |_|
 | _______________   |______________| |_|
 | |  SOUND 2     |   __   ___________|_|
 | |______________|  | |  |
 | _______________   | |  |
 | |  SOUND 1     |  | |<-PAL16L8D
 | |______________|  |_|  |
 |________________________|

Just one different byte between the three "Escalera y Tobogan" sets, at address 00001401:
 1268: 0xF4
 1269: 0xF5
 1270: 0xF6
May be the game serial number hard-encoded on the EPROM?
*/
ROM_START( m4esctbg )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ma-15_b-1925-94_esc_1.6a_n-1270.rom1", 0x0000, 0x10000, CRC(6fa2a0ef) SHA1(3b60b545e417a45e61e3babbe27758a053ced926) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "escsnd_0.2_p1.rom2", 0x000000, 0x080000, CRC(2f6517bc) SHA1(b39a4fa17d3e373b7a89663668529d752e595641) )
	ROM_LOAD( "escsnd_0.2_p2.rom3", 0x080000, 0x080000, CRC(3b0b9fed) SHA1(5a03be7f3a7f40252cfec5f719a845d175e3995c) )

	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "m578.chr", 0x0000, 0x0048, NO_DUMP )

	ROM_REGION( 0x104, "pld", 0 )
	ROM_LOAD( "pal16l8d-2pc.ic7", 0x000, 0x104, CRC(e8e7ccde) SHA1(b1ece0d51003c794f00655a8c52e5f7fd843b4c5) )
ROM_END

ROM_START( m4esctbga )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ma-15_b-1925-94_esc_1.6a_n-1269.rom1", 0x0000, 0x10000, CRC(8c3f1cf3) SHA1(0e7961bacc4ba701efbbd1ee99b2a72422f96b07) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "escsnd_0.2_p1.rom2", 0x000000, 0x080000, CRC(2f6517bc) SHA1(b39a4fa17d3e373b7a89663668529d752e595641) )
	ROM_LOAD( "escsnd_0.2_p2.rom3", 0x080000, 0x080000, CRC(3b0b9fed) SHA1(5a03be7f3a7f40252cfec5f719a845d175e3995c) )

	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "m578.chr", 0x0000, 0x0048, NO_DUMP )

	ROM_REGION( 0x104, "pld", 0 )
	ROM_LOAD( "pal16l8d-2pc.ic7", 0x000, 0x104, CRC(e8e7ccde) SHA1(b1ece0d51003c794f00655a8c52e5f7fd843b4c5) )
ROM_END

ROM_START( m4esctbgb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ma-15_b-1925-94_esc_1.6a_n-1268.rom1", 0x0000, 0x10000, CRC(d2b47707) SHA1(65096835d94242a5c07b266b8561a9e0d9f95e36) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "escsnd_0.2_p1.rom2", 0x000000, 0x080000, CRC(2f6517bc) SHA1(b39a4fa17d3e373b7a89663668529d752e595641) )
	ROM_LOAD( "escsnd_0.2_p2.rom3", 0x080000, 0x080000, CRC(3b0b9fed) SHA1(5a03be7f3a7f40252cfec5f719a845d175e3995c) )

	ROM_REGION( 0x48, "chr", 0 )
	ROM_LOAD( "m578.chr", 0x0000, 0x0048, NO_DUMP )

	ROM_REGION( 0x104, "pld", 0 )
	ROM_LOAD( "pal16l8d-2pc.ic7", 0x000, 0x104, CRC(e8e7ccde) SHA1(b1ece0d51003c794f00655a8c52e5f7fd843b4c5) )
ROM_END


void mpu4unsorted_state::init_m4aao()
{
	//Derived from Against_All_Odds_(Eurotek)_[C01_800_15jp].gam
	init_m4default();
	use_m4_hopper_duart_a();
	use_m4_standard_reels();
	//PCKEY =9
	//STKEY =0
	//JPKEY =0
	//JPSET =0
	//DIP1_0=true
	//DIP1_1=true
	//DIP1_2=true
	//DIP1_3=true
	//DIP1_4=true
	//DIP1_5=false
	//DIP1_6=false
	//DIP1_7=false
	//DIP2_0=false
	//DIP2_1=false
	//DIP2_2=false
	//DIP2_3=false
	//DIP2_4=false
	//DIP2_5=false
	//DIP2_6=false
	//DIP2_7=false
	//Sound barcrest1
	//Standard
	//Volume 0 Stereo= 0
	//Sample rate 16000
	//Front door code 255 Cash door code 255
}

ROM_START( m4aao )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "aao2_8.bin", 0x0000, 0x010000, CRC(94ce4016) SHA1(2aecb6dbe798b7bbfb3d27f4d115b6611c7d990f) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "aaosnd.bin", 0x0000, 0x080000, CRC(7bf30b96) SHA1(f0086ae239b1d973018a3ea04e816a87f8f20bad) )
ROM_END


ROM_START( m4bandgd )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bog.bin", 0x0000, 0x020000, CRC(21186fb9) SHA1(3d536098c7541cbdf02d68a18a38cae71155d7ff) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "bandsofgoldsnd.bin", 0x0000, 0x080000, CRC(95c6235f) SHA1(a13afa048b73fabfad229b5c2f8ef5ee9948d9fb) )
ROM_END


ROM_START( m4bigben )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "b_bv2_7.bin", 0x0000, 0x010000, CRC(9f3a7638) SHA1(b7169dc26a6e136d6daaf8d012f4c3d017e99e4a) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbena )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "b_bv2_9.bin", 0x0000, 0x010000, CRC(86a745ee) SHA1(2347e8e38c743ea4d00faee6a56bb77e05c9c94d) ) // aka bb2_9.bin

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbenb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bb1_9p.bin", 0x0000, 0x010000, CRC(c76c5a09) SHA1(b0e3b38998428f535841ab5373d57cb0d5b21ed3) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END


ROM_START( m4bigbend )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bb_2_1.bin", 0x0000, 0x010000, CRC(d3511805) SHA1(c86756998d36e729874c71a5d6442785069c57e9) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbene )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bbs_2_9p.bin", 0x0000, 0x010000, CRC(0107608d) SHA1(9e5def90e77f65c366aea2a9ac24d5f17c4d0ae8) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END


ROM_START( m4boltbl )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bfb.bin", 0x8000, 0x008000, CRC(63058a6b) SHA1(ebccc647a937c36ffc6c7cfc01389f04f829999c) )
ROM_END

ROM_START( m4boltbla )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bfb1.1.bin", 0x8000, 0x008000, CRC(7a91122d) SHA1(28229e86feb4411978e556f7f7bd85bfd996b8aa) )
ROM_END

ROM_START( m4boltblb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bfb9 5p cash.bin", 0x8000, 0x008000, CRC(792bff34) SHA1(6996e87f22df6bac7bbe9908534b7e0480f03ede) )
ROM_END

ROM_START( m4boltblc )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bolt-gilwern.bin", 0x8000, 0x008000, CRC(74e2c821) SHA1(1dcdc58585d1dcfc93e2aeb3df0cd41705cde196) )
ROM_END

ROM_START( m4dblchn )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "doublechance.bin", 0x0000, 0x010000, CRC(6feeeb7d) SHA1(40fe67d854fbf48959e08fdb5743e14d340c16e7) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "doublechancesnd.bin", 0x0000, 0x080000, CRC(3e80f8bd) SHA1(2e3a195b49448da11cc0c089a8a9b462894c766b) )
ROM_END


ROM_START( m4kqclub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kingsque.p1", 0x8000, 0x008000, CRC(6501e501) SHA1(e289a9418c640415967fafda43f20877b38e3671) )
ROM_END

ROM_START( m4snookr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "snooker.ts2", 0x8000, 0x004000, CRC(a6906eb3) SHA1(43b91e88f909b758f880d83df4f889f15aa17eb3) )
	ROM_LOAD( "snooker.ts1", 0xc000, 0x004000, CRC(3e3072dd) SHA1(9ea8b270044b48767a2e6c19e8ed257d5491c1d0) )
ROM_END


ROM_START( m4stakex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stakex.bin", 0x0000, 0x010000, CRC(098c7117) SHA1(27f04cfb88ef870fc30afd055cf32ffe448275ea) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "stakexsnd.bin", 0x0000, 0x080000, CRC(baf17991) SHA1(282e0ac9d18299e9f7a0fecaf9edf0cb4205ef0e) )
ROM_END

ROM_START( m4stakexa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stakex2.bin", 0x0000, 0x010000, CRC(77ae3f63) SHA1(c5f1cfd5bffcf3156f584757de57ef6530214511) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "stakexsnd.bin", 0x0000, 0x080000, CRC(baf17991) SHA1(282e0ac9d18299e9f7a0fecaf9edf0cb4205ef0e) )
ROM_END


ROM_START( m4stand2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stand 2 del 8.bin", 0x08000, 0x08000, CRC(a9a5edc7) SHA1(035d3f3b3373cec475753f1b0de2f4db48d6d288) )
ROM_END


ROM_START( m4bigban )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "big04.p1", 0x0000, 0x020000, CRC(f7ead9c6) SHA1(46c10abb892cb6d427ad508aae96752c14b4cb83) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// Missing?
ROM_END

ROM_START( m4crzcsn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "crz03.bin", 0x0000, 0x020000, CRC(48610c4f) SHA1(a62ac8b3ee704ee4e98f9d56bfc723d4cbb25b54) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// Missing?
ROM_END

ROM_START( m4crzcav )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gcv05.p1", 0x0000, 0x020000, CRC(b9ba46f6) SHA1(78b745d85b36444c39747982987088a772b20a7e) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// Missing?
ROM_END

ROM_START( m4dragon )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dgl01.p1", 0x0000, 0x020000, CRC(d7d39c9b) SHA1(5350c9db549edee30815516b1ce74a018390ff3d) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// Missing?
ROM_END

ROM_START( m4hilonv )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hnc02.p1", 0x0000, 0x020000, CRC(33a8022b) SHA1(5168b8f32630aa2cb56f30c941695f1728e4fb7a) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// Missing?
ROM_END

ROM_START( m4octo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "oct03.p1", 0x0000, 0x020000, CRC(8df66e94) SHA1(e1ab93982846d83becae36b5814ebbd515b9078e) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// Missing?
ROM_END

ROM_START( m4sctagt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gse3_0.p1", 0x0000, 0x010000, CRC(eff705ff) SHA1(6bf96872ef4bcc8f8041c5384d892f072c72be2b) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// Missing?
ROM_END



ROM_START( m4barcrz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "barcrazy.bin", 0x0000, 0x010000, CRC(917ad749) SHA1(cb0a3f6737b8f183d2efb0a3f8adbf86d40a38ff) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "barcrazysnd.bin", 0x0000, 0x080000, CRC(0e155193) SHA1(7583e9f3e3624f82f2329565bdcbdaa5a5b03ee0) )
ROM_END

ROM_START( m4bonzbn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bingo-bonanza_v1.bin", 0x0000, 0x010000, CRC(3d137ddf) SHA1(1ce23db111448e44a166554dd8853dc379e787da) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bingo-bonanzasnd1.bin", 0x000000, 0x080000, CRC(e0eb2a92) SHA1(cbc0b3bba7857d87535d1c2a7459aed60709734a) )
	ROM_LOAD( "bingo-bonanzasnd2.bin", 0x080000, 0x080000, CRC(7db27b28) SHA1(98c5fa4bf8c7f67fae90a1ca98b74057f5ed9b6b) )
ROM_END

ROM_START( m4dnj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d.n.j 1-02", 0x0000, 0x010000, CRC(5750843d) SHA1(b87923e84071ea4a1af7566a7f413f8e30e208e9) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) // should this set have an OKI?
ROM_END

ROM_START( m4dnja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d.n.j 1-03", 0x0000, 0x010000, CRC(7b805255) SHA1(f62765bfa66e2422ac0a71ebaff27f1ccd470fe2) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) // should this set have an OKI?
ROM_END

ROM_START( m4dnjb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d.n.j 1-06", 0x0000, 0x010000, CRC(aab770c7) SHA1(f24fff8346915017bc43fef9fac356a067676d86) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) // should this set have an OKI?
ROM_END


ROM_START( m4matdr )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "matador.bin", 0x0000, 0x020000, CRC(367788a4) SHA1(3c9b077a64f993cb60107558efdfcbee0fe5c958) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// missing
ROM_END




#define M4SBX_EXTRAS \
	ROM_REGION( 0x40000, "upd", 0 ) /* not oki at least... */ \
	ROM_LOAD( "sbsnd", 0x0000, 0x040000, CRC(27fd9fe6) SHA1(856fdc95a833affde0ada7041c68a4b6b729b715) )

ROM_START( m4sbx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbx-2.1-cash.bin", 0x8000, 0x008000, CRC(2dca703e) SHA1(aef398f4ed38ba34f28009058c9486a570f64e0f) )
	M4SBX_EXTRAS
ROM_END

ROM_START( m4sbxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b_sbx23.bin", 0x8000, 0x008000, CRC(8188e94f) SHA1(dfbfc549d12c8f7c7db6c12ba766c28f1cf0873f) )
	M4SBX_EXTRAS
ROM_END

ROM_START( m4sbxb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s bears v1-4 20p po.bin", 0x8000, 0x008000, CRC(03486714) SHA1(91c237956bbec58cc08a3e92543488d8e2daa673) )
	M4SBX_EXTRAS
ROM_END

ROM_START( m4sbxc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s bears v2-4 10p 8.bin", 0x8000, 0x008000, CRC(9b94f8d0) SHA1(9808386def14c8a058730e90135a4d6506e6ed3d) )
	M4SBX_EXTRAS
ROM_END

ROM_START( m4sbxd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s bears v2-4 20p po.bin", 0x8000, 0x008000, CRC(ad8f8d9d) SHA1(abd808f95b587a84e8b3aad1af9fe1cb613c9821) )
	M4SBX_EXTRAS
ROM_END

ROM_START( m4sbxe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superbea.10p", 0x8000, 0x008000, CRC(70020466) SHA1(473c9feb9ce0024b870612af19ec8a47a7798506) )
	M4SBX_EXTRAS
ROM_END


ROM_START( m4bclimb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bc8pv4.bin", 0x8000, 0x008000, CRC(229a7607) SHA1(b20b2c9f9d19ccd6146affdf519fa4bc0322c971) )

	ROM_REGION( 0x40000, "upd", 0 ) // not oki at least...
	ROM_LOAD( "sbsnd", 0x0000, 0x040000, CRC(27fd9fe6) SHA1(856fdc95a833affde0ada7041c68a4b6b729b715) )
ROM_END

ROM_START( m4captb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_bear21.rom", 0x8000, 0x008000, CRC(2e9a42e9) SHA1(0c3f33311f1543daf2ff5c0443dc8c000d49c26d) )

	ROM_REGION( 0x40000, "upd", ROMREGION_ERASE00 ) // not oki at least...
//  ROM_LOAD( "sbsnd", 0x0000, 0x040000, CRC(27fd9fe6) SHA1(856fdc95a833affde0ada7041c68a4b6b729b715) )
ROM_END

#define M4JUNGJ_EXTRAS \
	ROM_REGION( 0x40000, "upd", ROMREGION_ERASE00 ) \
	/* missing? */
ROM_START( m4jungj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jj2410p.bin", 0x8000, 0x008000, CRC(490838c6) SHA1(a1e9963df9a429ae594592312e977f22f96c6073) )
	M4JUNGJ_EXTRAS
ROM_END

ROM_START( m4jungja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jj2420p.bin", 0x8000, 0x008000, CRC(39329ccf) SHA1(6b79e4fc553bad935ec9989ad5ef3e186e720633) )
	M4JUNGJ_EXTRAS
ROM_END

ROM_START( m4jungjb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jjv2_4p.bin", 0x8000, 0x008000, CRC(125a8138) SHA1(18c62df5b331bd09d6dcda6280351e94b7b816fd) )
	M4JUNGJ_EXTRAS
ROM_END

ROM_START( m4jungjc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jjv4.bin", 0x8000, 0x008000, CRC(bf583156) SHA1(084c5ed3d96c92f265ad08cc7aed7fe6092217a5) )
	M4JUNGJ_EXTRAS
ROM_END


#define M4FSX_EXTRAS \
	ROM_REGION( 0x40000, "upd", ROMREGION_ERASE00 ) \
	/* missing? */

ROM_START( m4fsx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("funspotx.10p", 0x8000, 0x008000, CRC(55199f36) SHA1(7af376781e381582b06972725a2022cc28ba60b3) )
	M4FSX_EXTRAS
ROM_END

ROM_START( m4fsxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "funspotx.20p", 0x8000, 0x008000, CRC(08d1eb6e) SHA1(7c7c02d9c34696d75490df8596ffe64fba93dcc4) )
	M4FSX_EXTRAS
ROM_END

ROM_START( m4fsxb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b_fsv1.bin", 0x8000, 0x008000, CRC(b077f944) SHA1(97d96594b8d2d7232bad087cc55912dec02d7484) )
	M4FSX_EXTRAS
ROM_END

/*
Coinworld data

Error Number    Cause of alarm        Comments
11              1 GBP coin in         These alarms go off when a coin is jammed in the mech, or if the Mars anti-strimming alarm is activated.
12              50p coin in           The machine will lock up for a short amount of time, whilst sounding as alarm tone.
13              20p coin in           Error 15 can be caused by having DIL switch 6 in the wrong position for your coin mech loom.
14              10p coin in
15               5p coin in
16              2 GBP coin in
21              Reel 1 alarm          The faulty reel will flash. Nothing more will happen until the machine is reset
22              Reel 2 alarm
23              Reel 3 alarm
42              Ram Cleared           The RAM is cleared when the machine is turned on for the first time, or when the price of play is changed. The alarm
                                      clears after a short time
51             Checksum error         The machine will lock up completely if the eprom has failed, or if the security chip is missing or has failed
54             Security chip fail
61             Cash in meter failure  The machine will not run if the cash in, or cash out meters are not connected properly.
62             Cash out meter failure
71             Datapack error         If the machine is in protocol mode, and a datapack is not connected, then the machine alarms. It will reset after a
                                      time, and have another go at transmitting the data
72             Sound card fail        If the sound card is missing, or the wrong sound eprom is fitted, the machine alarms on power on. The machine will then
                                      operate in silence.
99             Payout tubes empty     If one of the tubes runs dry, the machine will attempt to compensate by paying from the other tube. If this runs dry
                                      as well, the machine will lock up, requiring a refill before games can continue. The alarm tone is a softer, more friendly one.
*/

#define M4CCOP_EXTRAS \
	ROM_REGION( 0x100000, "alt1msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "cash-copssnd1-de.bin", 0x000000, 0x080000, CRC(cd03f7f7) SHA1(4c09a86bcdf9a9eb224b19b932b75c9db3784fad) ) \
	ROM_LOAD( "cash-copssnd2-de.bin", 0x080000, 0x080000, CRC(107816a2) SHA1(f5d4a0390b85a665a3536da4689ec91b1a2da3ae) ) \
	ROM_REGION( 0x100000, "alt2msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "cash-copssnd1.bin", 0x000000, 0x080000, CRC(776a303d) SHA1(a5a282674674f25bc6ca169eeebee7309239871f) ) \
	ROM_LOAD( "cash-copssnd2.bin", 0x080000, 0x080000, CRC(107816a2) SHA1(f5d4a0390b85a665a3536da4689ec91b1a2da3ae) ) \
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 ) \
	ROM_LOAD( "cashcops.p1", 0x000000, 0x080000, CRC(9a59a3a1) SHA1(72cfc99b22ec5fb89714c6d2d66760d86dc19f2f) ) \
	ROM_LOAD( "cashcops.p2", 0x080000, 0x080000, CRC(deb3e755) SHA1(01f92881c451919be549a1c58afa1fa4630bf171) )

ROM_START( m4ccop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashcop9.bin", 0x0000, 0x010000, CRC(5f993207) SHA1(ab0614e6a1355d275158b1a32f65086e40c2f890) )
	M4CCOP_EXTRAS
ROM_END

ROM_START( m4ccopa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash-cops_v4-de.bin", 0x0000, 0x010000, CRC(df3da824) SHA1(c275a33e4a89f1b9ecbae80cb7b62007b29b9fd2) )
	M4CCOP_EXTRAS
ROM_END

ROM_START( m4ccopb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashcop8.bin", 0x0000, 0x010000, CRC(165603df) SHA1(d301696a340ed136a43c5753c8bf73283a925fd7) )
	M4CCOP_EXTRAS
ROM_END

ROM_START( m4ccc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ccc12.bin", 0x8000, 0x008000, CRC(570cc766) SHA1(036c95ff6428ab38cceb0537dcc990be78fb331a) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "criss cross crazy sound,27c2001", 0x0000, 0x040000, CRC(1994c509) SHA1(2bbe91a43aa9953b7776faf67e81e30a4f7b7cb2) )
ROM_END


ROM_START( m4treel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trgv1.1s", 0x0000, 0x010000, CRC(a9c76b08) SHA1(a5b3bc980eb58e346cb02d8ca43401f304e5b6de) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4treela )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "trgv1.1b", 0x0000, 0x020000, CRC(7eaebef6) SHA1(5ab86329041e7df09cc2e3ce8d5afd44d88c246c) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
ROM_END




ROM_START( m4remag )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "remagv2", 0x0000, 0x010000, CRC(80d9c1c2) SHA1(c77d443d92084c324ef75575acca66ffbd9beef3) )
ROM_END

ROM_START( m4rmg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rmgicdd", 0x0000, 0x010000, CRC(bd64be0d) SHA1(772b80619c7d514a7a253f35137896d6a73bf4c6) )
ROM_END

ROM_START( m4wnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wnudge.bin", 0x8000, 0x008000, CRC(1d935575) SHA1(c4177c41473c0fb511e0ee035961f55ad43be14d) )
ROM_END


ROM_START( m4riotrp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drt10.bin", 0x0000, 0x010000, CRC(a1badb8a) SHA1(871786ea4e65ecbf61c9a776100321253922d11e) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "dblcsnd.bin", 0x0000, 0x080000, CRC(c90fa8ad) SHA1(a98f03d4b6f5892333279bff7537d4d6d887da62) )
ROM_END


#define M4SURF_EXTRAS \
	ROM_REGION( 0x200000, "msm6376", 0 ) \
	ROM_LOAD( "s_surf.sn1", 0x000000, 0x080000, CRC(f20a7d69) SHA1(7887230613b497dc71a60125dd1e265ebbc8eb23) ) \
	ROM_LOAD( "s_surf.sn2", 0x080000, 0x080000, CRC(6c4a9074) SHA1(3b993120156677de893e5dc1e0c5d6e0285c5570) )

ROM_START( m4surf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "s_surfin._pound5", 0x0000, 0x020000, CRC(5f800636) SHA1(5b1789890eea44e5275e13f360876374d862935f) )
	M4SURF_EXTRAS
ROM_END

ROM_START( m4surfa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "s_surfin.upd", 0x0000, 0x020000, CRC(d0bef9cd) SHA1(9d53bfe8d928b190202bf747c0d7bb4cc0ae0efd) )
	M4SURF_EXTRAS
ROM_END

ROM_START( m4surfb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "s_surfin._pound15", 0x0000, 0x020000, CRC(eabce7fd) SHA1(4bb2bbcc7d2917eca72385a21ab85d2d94a882ec) )
	M4SURF_EXTRAS
ROM_END


ROM_START( m4wife )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "moy_wife.p1", 0x0000, 0x020000, CRC(293d35a6) SHA1(980a28ca5e9ec3ca2e1a5b34f658b622dca4cf50) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	// missing?
ROM_END

#define M4BLKGD_EXTRAS \
	ROM_REGION( 0x200000, "msm6376", 0 ) \
	ROM_LOAD( "blackgoldsnd1.bin", 0x000000, 0x080000, CRC(d251b59e) SHA1(960b81b87f0fb5000028c863892a273362cb897f) ) \
	ROM_LOAD( "blackgoldsnd2.bin", 0x080000, 0x080000, CRC(87cbcd1e) SHA1(a6cd186af7c5682e216f549b77735b9bf1b985ae) ) \
	ROM_LOAD( "blackgoldsnd3.bin", 0x100000, 0x080000, CRC(258f7b83) SHA1(a6df577d98ade8c5c5ff68ef891667e65e83ac17) )
ROM_START( m4blkgd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "blackgoldprg.bin", 0x0000, 0x080000, CRC(a04736b2) SHA1(9e060cc79e7922b38115f1412ed76f8c76deb917) )
	M4BLKGD_EXTRAS
ROM_END

//Early rom banks empty? May need different loading
ROM_START( m4blkgda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "blackgoldversion2.4.bin", 0x0000, 0x040000, CRC(fad4e360) SHA1(23c6a13e8d1ca307b0ef22edffed536675985aca) )
	M4BLKGD_EXTRAS
ROM_END

#define M4ZILL_EXTRAS \
	ROM_REGION( 0x200000, "msm6376", 0 ) \
	ROM_LOAD( "zillsnd.bin", 0x0000, 0x080000, CRC(171ed677) SHA1(25d63f4d9c64f13bec4feffa265c5b0c5f6be4ec) )

ROM_START( m4zill )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "zillprgh.bin", 0x0000, 0x080000, CRC(6f831f6d) SHA1(6ab6d7f1752d27bc216bc11533b90178ce188715) )
	M4ZILL_EXTRAS
ROM_END

ROM_START( m4zilla )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "zillprog.bin", 0x0000, 0x080000, CRC(0f730bab) SHA1(3ea82c8f7d62c70897a5c132273820c9f192cd72) )
	M4ZILL_EXTRAS
ROM_END


#define M4HSTR_EXTRAS \
	ROM_REGION( 0x200000, "altmsm6376", 0 ) \
	ROM_LOAD( "happystreak.p1", 0x0000, 0x080000, CRC(b1f328ff) SHA1(2bc6605965cb5743a2f8b813d68cf1646a4bcac1) ) \
	ROM_REGION( 0x200000, "msm6376", 0 ) \
	ROM_LOAD( "happystreaksnd.p1", 0x0000, 0x080000, CRC(76cda195) SHA1(21a985cd6cf1f63f4aa799563099a0527a7c0ea2) ) \
	ROM_LOAD( "happystreaksnd.p2", 0x080000, 0x080000, CRC(f3b4c763) SHA1(7fd6230c13b66a16daad9d45935c7803a5a4c35c) )

ROM_START( m4hstr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "h_s_v1_2.bin", 0x0000, 0x010000, CRC(ef3d3461) SHA1(aa5b1934ab1c6739f36ac7b55d3fda2c640fe4f4) )
	M4HSTR_EXTRAS
ROM_END

ROM_START( m4hstra )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hs2_5.bin", 0x0000, 0x010000, CRC(f669a4c9) SHA1(46813ba7104c97eaa851b50019af9b80046d03b3) )
	M4HSTR_EXTRAS
ROM_END

ROM_START( m4hstrb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hs2_5p.bin", 0x0000, 0x010000, CRC(71c981aa) SHA1(5effe7487e7216078127d3dc4a0a7ad02ad84390) )
	M4HSTR_EXTRAS
ROM_END


ROM_START( m4hstrcs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chs3_6.bin", 0x0000, 0x010000, CRC(d097ae0c) SHA1(bd78c14e7f057f173859bcb1db5e6a142d0c4062) )
	M4HSTR_EXTRAS
ROM_END

ROM_START( m4hstrcsa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chs3_6p.bin", 0x0000, 0x010000, CRC(57378b6f) SHA1(cf1cf528b9790c1013d87ccf63dcbf59f365067f) )
	M4HSTR_EXTRAS
ROM_END

ROM_START( m4hstrcsb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chs3_6pk.bin", 0x0000, 0x010000, CRC(f95f1afe) SHA1(fffa409e8c7148a840d5dedf490fd9f6975e9476) )
	M4HSTR_EXTRAS
ROM_END

ROM_START( m4hstrcsc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chs3_6k.bin", 0x0000, 0x010000, CRC(7eff3f9d) SHA1(31dedb0d9476633e8eb947a687c7b8a94b0e182c) )
	M4HSTR_EXTRAS
ROM_END

ROM_START( m4hstrcsd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chs_4_2.bin", 0x0000, 0x010000, CRC(ec148b65) SHA1(2d6252ce68719281f5597955227a1f662743f006) )
	M4HSTR_EXTRAS
ROM_END


#define M4DDB_EXTRAS \
	ROM_REGION( 0x200000, "msm6376", 0 ) \
	ROM_LOAD( "ddbsound1", 0x000000, 0x080000, CRC(47c87bd5) SHA1(c1578ae553c38e93235cea2142cb139170de2a7e) ) \
	ROM_LOAD( "ddbsound2", 0x080000, 0x080000, CRC(9c733ab1) SHA1(a83c3ebe99703bb016370a8caf76bdeaff5f2f40) )
ROM_START( m4ddb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ddb3_1.bin", 0x0000, 0x010000, CRC(3b2da727) SHA1(8a677be3b82464d1bf1e97d22adad3b27374079f) )
	M4DDB_EXTRAS
ROM_END

ROM_START( m4ddba )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ddb3_1p.bin", 0x0000, 0x010000, CRC(bc8d8244) SHA1(9b8e0706b3add42e5e4a8b6c6a2f80a333a2f49e) )
	M4DDB_EXTRAS
ROM_END


ROM_START( m4hapfrt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf1_1.bin", 0x0000, 0x010000, CRC(6c16cb05) SHA1(421b164c8410629956177355e505859757c97a6b) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4hapfrta )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf1_1p.bin", 0x0000, 0x010000, CRC(ebb6ee66) SHA1(1f9b67260e5becd013d95358cc89acb1099d655d) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4hapfrtb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf1_4pk.bin", 0x0000, 0x010000, CRC(0944b3c6) SHA1(00cdb75dda4f8984f77806047ad79fe9a1a8760a) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4sunday )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sunday_sport_v11", 0x0000, 0x010000, CRC(14147d59) SHA1(03b14f4f83a545b3252702267ac012b3be76013d) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4jp777 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jpot71", 0x0000, 0x010000, CRC(f4564a05) SHA1(97d21e2268e5d99e6e51cb12c45e09445cff1f50) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4booze )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "boozecruise10_v10.bin", 0x0000, 0x010000, CRC(b37f752b) SHA1(166f7d17694689bd9d51d859c13ddafa1c6e5e7f) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4cbing )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cherrybingoprg.bin", 0x0000, 0x010000, CRC(00c1d4f3) SHA1(626df7f2f597ed13c32ce0fa8846f2e27ca68eae) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) // not oki!
	ROM_LOAD( "cherrybingosnd.p1", 0x000000, 0x100000, CRC(11bed9f9) SHA1(63ed45122dda8e412bb1eaeb967d8a0f925d4bde) )
	ROM_LOAD( "cherrybingosnd.p2", 0x100000, 0x100000, CRC(b2a7ec28) SHA1(307f19ffb46f4a2e8e93923ddb666e50de43a00e) )
ROM_END


ROM_START( m4nod )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nod.bin", 0x0000, 0x010000, CRC(bc738af5) SHA1(8df436139554ccfb48c4db0a32e3333dbf3c4f46) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) //region was called "upd" but machine is mod4oki? Which one is correct?
	ROM_LOAD( "nodsnd.bin", 0x0000, 0x080000, CRC(2134494a) SHA1(3b665bf79567a71195b20e76c50b02707d15b78d) )
ROM_END


ROM_START( m4aliz )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "70000000.bin", 0x0000, 0x040000, CRC(56f64dd9) SHA1(11f990c9a6864a969dc9a4146e1ac2c963e3eb9b) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "alizsnd.hi", 0x0000, 0x080000, CRC(c7bd937a) SHA1(cc4d85a3d4cdf57fa96c812a4cd78b599c7052ff) )
	ROM_LOAD( "alizsnd.lo", 0x080000, 0x04e15e, CRC(111cc111) SHA1(413efedbc9e85240df833c10d680b0e907da10b3) )

	ROM_REGION( 0x200000, "misc", ROMREGION_ERASE00 ) // i think this is just the sound roms as intelhex
	ROM_LOAD( "71000000.hi", 0x0000, 0x0bbe9c, CRC(867058c1) SHA1(bd980cb0bb3075854cc2e9b829c31f3742f4f1c2) )
	ROM_LOAD( "71000000.lo", 0x0000, 0x134084, CRC(53046751) SHA1(b8f9eca933315b497732c895f4311f62103344fc) )
ROM_END


ROM_START( m4c2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ci2-0601.bin", 0x0000, 0x010000, CRC(84cc8aca) SHA1(1471e3ad9c9ba957b6cc99c204fe588cc55fbc50) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4coney )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "70000060.bin", 0x0000, 0x010000, CRC(fda208e4) SHA1(b1a243b2681faa03add4ab6e4df98814f9c52fc5) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4goldnn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "goldenyears10.bin", 0x0000, 0x020000, CRC(1074bac6) SHA1(967ee64f267a80017fc95bbc6c5a38354e9cab65) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "tgyosnd.p1", 0x000000, 0x080000, CRC(bda49b46) SHA1(fac143003641824bf0db4ac6841292e509fa00da) )
	ROM_LOAD( "tgyosnd.p2", 0x080000, 0x080000, CRC(43d28a0a) SHA1(5863e493e84641e4fabcd69e6402e3bcca87dde2) )
	ROM_LOAD( "tgyosnd.p3", 0x100000, 0x080000, CRC(b5b9eb68) SHA1(8d5a0a687dd7096da8dfd2a59c6fe96f4b1949f9) )
ROM_END


ROM_START( m4mgpn )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "mgp15.p1", 0x0000, 0x010000, CRC(ec76233f) SHA1(aa8595c639c83026d7fe5c3a161f8b08ff9a8b46) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mgpsnd.p1", 0x000000, 0x080000, CRC(d5f0b845) SHA1(6d97d0d4d07407bb0a51e1d62da95c664418a9e9) )
	ROM_LOAD( "mgpsnd.p2", 0x080000, 0x080000, CRC(cefeea06) SHA1(45142ca1bab898dc6f3c32e382ee9157132810a6) )
	ROM_LOAD( "mgpsnd.p3", 0x100000, 0x080000, CRC(be4b3bd0) SHA1(f14c08dc770a24db8bbd00a65d3edf6ee9895ca3) )
	ROM_LOAD( "mgpsnd.p4", 0x180000, 0x080000, CRC(d74b4b03) SHA1(a35c99040a72485a6c2d4a4fdfc203634f6a9ad0) )
ROM_END


ROM_START( m4spotln )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gsp01.p1", 0x0000, 0x020000, CRC(54c56a07) SHA1(27f21872a7ffe0c497983fa5bbb59e967bf48974) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4vivan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "vlv.bin", 0x0000, 0x010000, CRC(f20c4858) SHA1(94bf19cfa79a1f5347ab61a80cbbce06942187a2) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "vlvsound1.bin", 0x0000, 0x080000, CRC(ce4da47a) SHA1(7407f8053ee482db4d8d0732fdd7229aa531b405) )
	ROM_LOAD( "vlvsound2.bin", 0x0000, 0x080000, CRC(571c00d1) SHA1(5e7be40d3caae88dc3a580415f8ab796f6efd67f) )
ROM_END





ROM_START( m4funh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "funhouse.bin", 0x00000, 0x10000, CRC(4e342025) SHA1(288125ff5e3da7249d89dfcc3cd0915f791f7d43) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) // no idea if it uses an OKI
ROM_END



ROM_START( m4eaw51 ) \
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "everyones a winner v2-5p", 0x08000, 0x008000, CRC(eb8f2fc5) SHA1(0d3614bd5ff561d17bef0d1e620f2f812b8fed5b))
ROM_END

} // anonymous namespace

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)


/* Barcrest */
GAME( 198?, m4tst2,   0,          mod2_no_bacta,       mpu4,    mpu4unsorted_state, init_m4test,  ROT0,   "Barcrest","MPU4 Unit Test (Program 2)",MACHINE_MECHANICAL )
GAME( 198?, m4clr,    0,          mod2_no_bacta,       mpu4,    mpu4unsorted_state, init_m4test,  ROT0,   "Barcrest","MPU4 Meter Clear ROM",MACHINE_MECHANICAL )
GAME( 198?, m4rltst,  0,          mod2_no_bacta,       mpu4,    mpu4unsorted_state, init_m4test,  ROT0,   "Barcrest","MPU4 Reel Test (3.0)",MACHINE_MECHANICAL )

// barcrest, to split

GAME(199?, m4ringfr,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::tentendia_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_big_lextender,ROT0, "Barcrest","Ring Of Fire (Barcrest) (German) (MPU4) (ROF 0.3)",GAME_FLAGS )

GAME(199?, m4royjwl,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::jewelcrown_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_big_lextender,  ROT0,   "Barcrest","Royal Jewels (Barcrest) (German) (MPU4) (GRJ 1.4)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4maglin,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_six_alt, ROT0,   "Barcrest","Magic Liner (Barcrest) (Dutch) (MPU4) (DMA 2.1)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4bluedm,  0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::bluediamond_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_six_alt, ROT0,   "Barcrest","Blue Diamond (Barcrest) (Dutch) (MPU4) (DBD 1.0)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4amhiwy,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m462_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_five_rev, ROT0,   "Barcrest","American Highway (Barcrest) (Dutch) (MPU4) (DAH 2.0)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4addrd,   0,          mod2_alt_cheatchr_pal<mpu4_characteriser_pal::m470_characteriser_prot>,   mpu4,    mpu4unsorted_state, init_m4default_five_rev, ROT0,   "Barcrest","Adders & Ladders (Barcrest) (Dutch) (MPU4) (DAL 1.2)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4rdht,    0,          mod4oki_7reel_cheatchr_pal<mpu4_characteriser_pal::redheat_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_seven, ROT0,   "Barcrest","Red Heat (Barcrest) (Dutch) (MPU4) (DRH 1.2)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4rwb,     0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::redwhite_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_six_alt, ROT0,   "Barcrest","Red White & Blue (Barcrest) (Dutch) (MPU4) (DRW 1.4)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4salsa,   0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::salsa_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_six_alt, ROT0,   "Barcrest","Salsa (Barcrest) (Dutch) (MPU4) (DSA 1.5)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4techno,  0,          mod4oki_7reel_cheatchr_pal<mpu4_characteriser_pal::techno_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_seven, ROT0,   "Barcrest","Techno Reel (Barcrest) (Dutch) (MPU4) (DTE 1.3, set 1)",GAME_FLAGS )
GAME(199?, m4technoa, m4techno,   mod4oki_7reel_cheatchr_pal<mpu4_characteriser_pal::techno_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_seven, ROT0,   "Barcrest","Techno Reel (Barcrest) (Dutch) (MPU4) (DTE 1.3, set 2)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4twintm,  0,          mod4oki_7reel_cheatchr_pal<mpu4_characteriser_pal::m533_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_seven, ROT0,   "Barcrest","Twin Timer (Barcrest) (MPU4) (D2T 1.1)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4gldgat,  0,          mod2_7reel_cheatchr_pal<mpu4_characteriser_pal::m450_characteriser_prot>,   mpu4,    mpu4unsorted_state, init_m4default_seven, ROT0,   "Barcrest","Golden Gate (Barcrest) (Dutch) (MPU4) (DGG 2.2)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4holdtm,  0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Hold Timer (Barcrest) (Dutch) (MPU4) (DHT 1.0)",GAME_FLAGS )

// Tube Sense ALM (this seems like an exported version of one of the Dutch games?)
GAME(199?, m4exgam,   0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Extra Game (Fairplay - Barcrest) (MPU4) (CEG 2.0)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4toma,    0,          mod2_7reel_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>,   mpu4,    mpu4unsorted_state, init_m4default_seven, ROT0,   "Barcrest","Tomahawk (Barcrest) (Dutch) (MPU4) (DTK 2.3)",GAME_FLAGS )

// GEEN TUBES, confirmed oki
GAME(199?, m4toptim,  0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Top Timer (Barcrest) (Dutch) (MPU4) (DTT 1.8, set 1)",GAME_FLAGS )
GAME(199?, m4toptima, m4toptim,   mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Top Timer (Barcrest) (Dutch) (MPU4) (DTT 1.8, set 2)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4univ,    0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Universe (Barcrest) (Dutch) (MPU4) (DUN 2.0)",GAME_FLAGS )

// Sample EPROM Alm
GAME(199?, m4frtgm,   0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m400_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Fruit Game (Barcrest) (MPU4) (FRU 2.0)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4roadrn,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Road Runner (Barcrest) (Dutch) (MPU4) (DRO 1.9)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4showtm,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::andybt_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Show Timer (Barcrest) (Dutch) (MPU4) (DSH 1.3)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4steptm,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::phr_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Step Timer (Barcrest) (Dutch) (MPU4) (DST 1.1)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4wildtm,  0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::wildtime_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4altreels, ROT0,   "Barcrest","Wild Timer (Barcrest) (Dutch) (MPU4) (DWT 1.3)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4magrep,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::turboplay_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Magic Replay (Barcrest) (Dutch) (MPU4) (DMR 1.3)",GAME_FLAGS )

// GEEN TUBES, but German?
GAME(199?, m4nile,    0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::actclba_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big_lextender,ROT0,"Barcrest","Nile Jewels (Barcrest) (German) (MPU4) (GJN 0.8)",GAME_FLAGS ) // DM1 SW ALM

// yes, the ingame display is 'Millenium' not 'Millennium'.  There are also strings from The Crystal Maze in the ROM, probably used as a base project?
GAME(199?, m4jokmil,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big_lextender,ROT0,"Barcrest","Jokers Millenium 300 (Barcrest) (German) (MPU4) (DJO 0.1, set 1)",GAME_FLAGS ) // DM1 SW ALM
GAME(199?, m4jok300,  m4jokmil,   mod4oki_cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big_lextender,ROT0,"Barcrest","Jokers Millenium 300 (Barcrest) (German) (MPU4) (DJO 0.1, set 2)",GAME_FLAGS ) // DM1 SW ALM

GAME(199?, m4drac,    0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::bankrollerclub_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big_lextender,ROT0,"Barcrest","Dracula (Barcrest - Nova) (German) (MPU4) (DRA 2.1)",GAME_FLAGS ) // DM1 SW ALM
GAME(199?, m4draca,   m4drac,     mod4oki_cheatchr_pal<mpu4_characteriser_pal::bankrollerclub_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big_lextender,ROT0,"Barcrest","Dracula (Barcrest - Nova) (German) (MPU4) (DRA 2.4)",GAME_FLAGS ) // DM1 SW ALM
GAME(199?, m4dracb,   m4drac,     mod4oki_cheatchr_pal<mpu4_characteriser_pal::bankrollerclub_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big_lextender,ROT0,"Barcrest","Dracula (Barcrest - Nova) (German) (MPU4) (DRA 2.7)",GAME_FLAGS ) // DM1 SW ALM


GAME(199?, m4vegastg, 0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big,ROT0,"Barcrest","Vegas Strip (Barcrest) (German) (MPU4)",GAME_FLAGS ) // 1 DM SW ALM

GAME(199?, m4luckdv,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Lucky Devil (Barcrest) (Czech) (MPU4)",GAME_FLAGS ) // AUX2 locked

GAME(199?, m4luckdvd, 0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::salsa_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Lucky Devil (Barcrest) (Dutch) (MPU4) (DLD 1.3)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4hpyjok,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::redheat_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Happy Joker (Barcrest) (Dutch) (MPU4) (DHJ 1.2)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4ceptr,   0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::salsa_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Ceptor (Barcrest) (Dutch) (MPU4) (DCE 1.0)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4gnsmk,   0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Gun Smoke (Barcrest) (Dutch) (MPU4) (DGU 1.6)",GAME_FLAGS )
GAME(199?, m4blkbuld, m4gnsmk,    mod4oki_cheatchr_pal<mpu4_characteriser_pal::age_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Gun Smoke (Barcrest) (Dutch) (MPU4) (DGU 1.6) (alt sound roms)",GAME_FLAGS ) // was marked 'Black Bull' but is GunSmoke  not sure either set of sound roms is right

// GEEN TUBES
GAME(199?, m4blkwhd,  0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::blackwhite_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_six, ROT0,   "Barcrest","Black & White (Barcrest) (Dutch) (MPU4) (DBW 1.1)",GAME_FLAGS ) // Reel Error

// GEEN TUBES, these 3 sets are identical, just with different sound ROMs, probably hacks?
GAME(199?, m4oldtmr,  0,          mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m470_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_six,  ROT0,   "Barcrest","Old Timer (Barcrest) (Dutch) (MPU4) (DOT 1.1)",GAME_FLAGS )
GAME(199?, m4casot,   m4oldtmr,   mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m470_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_six,  ROT0,   "Barcrest","Old Timer (Barcrest) (Dutch, alt 'Black and White' sound roms) (DOT 1.1)",GAME_FLAGS ) // uses the same program???
GAME(199?, m4jpmcla,  m4oldtmr,   mod4oki_alt_cheatchr_pal<mpu4_characteriser_pal::m470_characteriser_prot>,mpu4,    mpu4unsorted_state, init_m4default_six,  ROT0,   "Barcrest","Old Timer (Barcrest) (Dutch, alt 'JPM Classic' sound roms) (DOT 1.1)",GAME_FLAGS ) // uses the same program???

// GEEN TUBES
GAME(199?, m4tbplay,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::turboplay_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Turbo Play (Barcrest) (Dutch) (MPU4) (DTP 1.3)",GAME_FLAGS )
// NO METERS
GAME(199?, m4tbplaya, m4tbplay,   mod4oki_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Turbo Play (Barcrest) (MPU4) (CTP 0.4)",GAME_FLAGS )
GAME(199?, m4tbplayb, m4tbplay,   mod4oki_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Turbo Play (Barcrest) (MPU4) (ZTP 0.7)",GAME_FLAGS )
// NO METERS, non-standard protection
GAME(199?, m4remag,   m4tbplay,   mod2_bootleg_fixedret<0x19>,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "<unknown>","Turbo Play (Barcrest) (bootleg) (MPU4) (ZTP 0.7)",GAME_FLAGS )
GAME(199?, m4rmg,     m4tbplay,   mod2_bootleg_fixedret<0x6a>,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "<unknown>","Turbo Play (Barcrest) (bootleg) (MPU4) (CTP 0.4)",GAME_FLAGS )



// bwb/nova?
GAME(199?, m4ordmnd,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::actclba_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_big,ROT0,"Barcrest","Oriental Diamonds (Barcrest) (German) (MPU4) (RAB 0.1)",GAME_FLAGS )


// GEEN TUBES (even in test mode)
GAME(199?, m4topdk,   0,          mod2_cheatchr_pal<mpu4_characteriser_pal::intcep_characteriser_prot>,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Top Deck (Barcrest) (Dutch) (MPU4) (DT 2.6)",GAME_FLAGS )

// GEEN TUBES, runs open door
GAME(199?, m4bigban,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m4dtri98_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big, ROT0,   "Nova","Big Bandit (Nova) (German) (MPU4) (BIG 0.4)",GAME_FLAGS ) // DM1 SW ALM

// KEINE TOKENROEHR, runs open door
GAME(199?, m4crzcsn,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::mag7s_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big, ROT0,   "Nova","Crazy Casino (Nova) (German) (MPU4) (CRZ 0.3)",GAME_FLAGS )

// only runs with door open or gets stuck on initializing?
GAME(199?, m4crzcav,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::bdash_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_big, ROT0,   "Nova","Crazy Cavern (Nova) (MPU4) (GCV 0.5)",GAME_FLAGS )

// can coin up, but start doesn't work? runs open door
GAME(199?, m4dragon,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m683_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big, ROT0,   "Nova","Dragon (Nova) (MPU4) (DGL 0.1)",GAME_FLAGS )

// KEINE TOKENROEHR, runs open door
GAME(199?, m4hilonv,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::bagtel_characteriser_prot>,    mpu4_invcoin,    mpu4unsorted_state, init_m4default_big, ROT0,   "Nova","Hi Lo Casino (Nova) (MPU4) (HNC 0.2)",GAME_FLAGS )

// only runs with door open or gets stuck on initializing?
GAME(199?, m4octo,    0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::fruitfall_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_big, ROT0,   "Nova","Octopus (Nova) (German) (MPU4) (OCT 0.3)",GAME_FLAGS )

// NETWORK FAIL ALARM (reel comms?)
GAME(1994, m4esctbg,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m578_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default,     ROT0, "Vifico", "Escalera Tobogan (MPU4) (ESC1, set 1)", GAME_FLAGS )
GAME(1994, m4esctbga, m4esctbg,   mod4oki_cheatchr_pal<mpu4_characteriser_pal::m578_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default,     ROT0, "Vifico", "Escalera Tobogan (MPU4) (ESC1, set 2)", GAME_FLAGS )
GAME(1994, m4esctbgb, m4esctbg,   mod4oki_cheatchr_pal<mpu4_characteriser_pal::m578_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default,     ROT0, "Vifico", "Escalera Tobogan (MPU4) (ESC1, set 3)", GAME_FLAGS )

// boots but will give HOPPER JAM after a credit
GAME(199?, m4gldjok,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::goljok_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Golden Joker (Barcrest) (Dutch) (MPU4) (DGJ 1.2)",GAME_FLAGS )

// similar to m4gldjok but can't coin up
GAME(199?, m4blkcat,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::blkcat_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Black Cat (Barcrest) (Dutch) (MPU4) (DBL 1.4)",GAME_FLAGS )

// runs, coins don't work, Dutch?
GAME(199?, m4riotrp,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::blkcat_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Rio Tropico (Barcrest) (Dutch) (MPU4) (DRT 1.0)",GAME_FLAGS )

// expects the following response sequence, but check code is standard
// was the CHR replaced with something else that just happens to give this seuqence, or is this valid somehow?
// runs, coins don't work
// fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc 6c dc bc 7c fc fc fc fc fc fc fc fc fc 00 04 0c 1c 3c 7c fc fc fc fc fc fc fc fc d4 ac 5c bc 7c fc fc fc fc fc fc fc fc fc fc fc fc 00
GAME(199?, m4luck7,   0,          mod4oki_cheatchr,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Lucky 7 (Barcrest) (Dutch) (MPU4)",GAME_FLAGS )

// gives a DMD?? message if you attempt to coin it up, is there a mussing Dot Matrix Display ROM of some kind?
GAME(199?, m4joljokh, 0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::salsa_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Jolly Joker (Barcrest) (Hungarian) (MPU4) (HJJ 1.4)",GAME_FLAGS )


// Others

GAME(199?, m4sctagt,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Nova","Secret Agent (Nova) (MPU4)",GAME_FLAGS ) // AUX2 LOCKED


// works.  This is not a Barcrest game, but has a standard CHR check after you coin up for the first time, expecting the m4lv sequence back.  Reports ILLEGAL SOFTWARE if it fails
GAME(199?, m4bandgd,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Eurogames","Bands Of Gold (Eurogames) (MPU4)",GAME_FLAGS )

// ERROR CODE 2. as with m4bandgd this isn't a Barcrest, but does the standard CHR check and shows ILLEGAL SOFTWARE if check fails, assume it is also by Eurogames
GAME(199?, m4matdr,   0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Eurogames","Matador (Eurogames) (MPU4)",GAME_FLAGS )

// works, not standard protection, but cheatchr works
GAME(199?, m4bigben,  0,          mod4oki_cheatchr,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 1)",GAME_FLAGS )
GAME(199?, m4bigbena, m4bigben,   mod4oki_cheatchr,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 2)",GAME_FLAGS )
GAME(199?, m4bigbenb, m4bigben,   mod4oki_cheatchr,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 3)",GAME_FLAGS )
GAME(199?, m4bigbend, m4bigben,   mod4oki_cheatchr,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 4)",GAME_FLAGS )
GAME(199?, m4bigbene, m4bigben,   mod4oki_cheatchr,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 5)",GAME_FLAGS )

GAME(199?, m4kqclub,  0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Newby","Kings & Queens Club (Newby) (MPU4)",GAME_FLAGS )

GAME(199?, m4snookr,  0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Eurocoin","Snooker (Eurocoin) (MPU4)",GAME_FLAGS ) // works?

GAME(199?, m4stakex,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 1)",GAME_FLAGS ) // has issues with coins in 'separate bank' (default) mode, reel issues
GAME(199?, m4stakexa, m4stakex,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 2)",GAME_FLAGS ) // like above, but doesn't default to separate bank?

GAME(199?, m4boltbl,  0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 1)",GAME_FLAGS ) // Reel 1 Fault
GAME(199?, m4boltbla, m4boltbl,   mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 2)",GAME_FLAGS )
GAME(199?, m4boltblb, m4boltbl,   mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 3)",GAME_FLAGS )
GAME(199?, m4boltblc, m4boltbl,   mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 4)",GAME_FLAGS )

GAME(199?, m4stand2,  0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "DJE","Stand To Deliver (DJE) (MPU4)",GAME_FLAGS ) // Reel 1 Fault

GAME(199?, m4dblchn,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "DJE","Double Chance (DJE) (MPU4)",GAME_FLAGS ) // Reels spin forever

// other issues, only plays an 'alarm' sound when there's money to payout? wrong sound ROM or something else?
GAME(2001, m4casmul,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "DJE","Casino Multiplay (MPU4)",GAME_FLAGS ) // unprotected, copyright year / manufacturer found in ROM

 // has D.J.E 1999 copyright in ROM
GAMEL(1999, m4aao,    0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4aao,     ROT0,   "DJE / Eurotek","Against All Odds (Eurotek) (MPU4)",GAME_FLAGS, layout_m4aao )

/* Unknown stuff that looks like it might be MPU4, but needs further verification, some could be bad */

// PAL FAIL
GAME(199?, m4barcrz,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "<unknown>","Bar Crazy (unknown) (MPU4?)",GAME_FLAGS )

// gives an error
GAME(199?, m4bonzbn,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "<unknown>","Bingo Bonanza (unknown) (MPU4?)",GAME_FLAGS )


/* *if* these are MPU4 they have a different sound system at least - The copyright strings in them are 'AET' tho (Ace?) - Could be related to the Crystal stuff? */
GAME(199?, m4sbx,     0,          mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4sbxa,    m4sbx,      mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4sbxb,    m4sbx,      mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 3)",GAME_FLAGS )
GAME(199?, m4sbxc,    m4sbx,      mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 4)",GAME_FLAGS )
GAME(199?, m4sbxd,    m4sbx,      mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 5)",GAME_FLAGS )
GAME(199?, m4sbxe,    m4sbx,      mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 6)",GAME_FLAGS )

GAME(199?, m4bclimb,  0,          mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Bear Climber (MPU4?)",GAME_FLAGS )

GAME(199?, m4captb,   0,          mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Captain Bear (MPU4?)",GAME_FLAGS )

GAME(199?, m4jungj,   0,          mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4jungja,  m4jungj,    mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4jungjb,  m4jungj,    mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 3)",GAME_FLAGS )
GAME(199?, m4jungjc,  m4jungj,    mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 4)",GAME_FLAGS )

GAME(199?, m4fsx,     0,          mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Fun Spot X (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4fsxa,    m4fsx,      mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Fun Spot X (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4fsxb,    m4fsx,      mod4oki,   mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "AET/Coinworld","Fun Spot X (MPU4?) (set 3)",GAME_FLAGS )

// Error 42 then 52, 54
GAME(199?, m4ccop,    0,          mod4oki,    mpu4_cw, mpu4unsorted_state, init_m4default, ROT0, "Coinworld","Cash Cops (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4ccopa,   m4ccop,     mod4oki,    mpu4_cw, mpu4unsorted_state, init_m4default, ROT0, "Coinworld","Cash Cops (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4ccopb,   m4ccop,     mod4oki,    mpu4_cw, mpu4unsorted_state, init_m4default, ROT0, "Coinworld","Cash Cops (MPU4?) (set 3)",GAME_FLAGS )

// Error 72
GAME(199?, m4ccc,     0,          mod4oki,    mpu4_cw, mpu4unsorted_state, init_m4default, ROT0, "Coinworld","Criss Cross Crazy (Coinworld) (MPU4?)",GAME_FLAGS )

// PAL ERROR
GAME(199?, m4treel,   0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "<unknown>","Turbo Reels (unknown) (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4treela,  m4treel,    mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "<unknown>","Turbo Reels (unknown) (MPU4?) (set 2)",GAME_FLAGS )

// works
GAME(199?, m4surf,    0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Gemini","Super Surfin' (Gemini) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4surfa,   m4surf,     mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Gemini","Super Surfin' (Gemini) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4surfb,   m4surf,     mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Gemini","Super Surfin' (Gemini) (MPU4) (set 3)",GAME_FLAGS )

// works
GAME(199?, m4wife,    0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Gemini","Money Or Yer Wife (Gemini) (MPU4)",GAME_FLAGS )

// works, error unless you set %
GAME(199?, m4blkgd,   0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Gemini","Black Gold (Gemini) (MPU4) (set 1)",GAME_FLAGS )
// doesn't work, might be bad dump
GAME(199?, m4blkgda,  m4blkgd,    mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Gemini","Black Gold (Gemini) (MPU4) (set 2)",GAME_FLAGS )

// runs, non-reel game?
GAME(199?, m4zill,    0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Pure Leisure","Zillionare's Challenge (Pure Leisure) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4zilla,   m4zill,     mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Pure Leisure","Zillionare's Challenge (Pure Leisure) (MPU4) (set 2)",GAME_FLAGS )

// runs but corrupt VFD
GAME(199?, m4hstr,    0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Happy Streak (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4hstra,   m4hstr,     mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Happy Streak (Coinworld) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4hstrb,   m4hstr,     mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Happy Streak (Coinworld) (MPU4) (set 3)",GAME_FLAGS )

// runs but corrupt VFD
GAME(199?, m4hstrcs,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4hstrcsa, m4hstrcs,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4hstrcsb, m4hstrcs,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 3)",GAME_FLAGS )
GAME(199?, m4hstrcsc, m4hstrcs,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 4)",GAME_FLAGS )
GAME(199?, m4hstrcsd, m4hstrcs,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 5)",GAME_FLAGS )

// Error 1.8
GAME(199?, m4ddb,     0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Ding Dong Bells (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4ddba,    m4ddb,      mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Ding Dong Bells (Coinworld) (MPU4) (set 2)",GAME_FLAGS )

// runs but corrupt VFD
GAME(199?, m4hapfrt,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Happy Fruits (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4hapfrta, m4hapfrt,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Happy Fruits (Coinworld) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4hapfrtb, m4hapfrt,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Coinworld","Happy Fruits (Coinworld) (MPU4) (set 3)",GAME_FLAGS )

// backwards VFD
GAME(199?, m4sunday,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Pcp","Sunday Sport (Pcp) (MPU4)",GAME_FLAGS )

// HOPPER FAULT
GAME(199?, m4jp777,   0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Cotswold Microsystems","Jackpot 777 (Cotswold Microsystems) (MPU4)",GAME_FLAGS ) /* Hopper Fault */

// HOPPER FAULT
GAME(199?, m4dnj,     0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Cotswold Microsystems","Double Nudge (Cotswold Microsystems) (MPU4) (set 1)",GAME_FLAGS ) /* Hopper Fault */
GAME(199?, m4dnja,    m4dnj,      mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Cotswold Microsystems","Double Nudge (Cotswold Microsystems) (MPU4) (set 2)",GAME_FLAGS ) /* Hopper Fault */
GAME(199?, m4dnjb,    m4dnj,      mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Cotswold Microsystems","Double Nudge (Cotswold Microsystems) (MPU4) (set 3)",GAME_FLAGS ) /* Hopper Fault */

// PIC CHECK, backwards VFD
GAME(199?, m4booze,   0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Extreme","Booze Cruise (Extreme) (MPU4)",GAME_FLAGS )

 // custom sound system
GAME(199?, m4cbing,   0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Redpoint Systems","Cherry Bingo (Redpoint Systems) (MPU4)",GAME_FLAGS )

// just spins wheels badly
GAME( 199?, m4nod,    0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, 0,      "Eurotech",   "Nod And A Wink (Eurotech) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE) // this has valid strings in it BEFORE the bfm decode, but decodes to valid code, does it use some funky mapping, or did they just fill unused space with valid looking data?

// LINKUP SUCHE
GAME( 199?, m4aliz,   0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default_big, 0,      "Qps",  "AlizBaz (Qps) (German) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// SHELF RESONSE
GAME( 199?, m4coney,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, 0,      "Qps",   "Coney Island (Qps) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// very similar to m4c2 behavior below, but no protection?
GAME( 199?, m4crzjk,  0,          mod2_alt,       mpu4_invcoin,    mpu4unsorted_state, init_m4default, 0,      "Nova?", "Crazy Jokers (Nova?) (MPU4)",GAME_FLAGS )

// not standard protection, but cheatchr passes it, code crashes after a short time?
GAME( 199?, m4c2,     0,          mod4oki_alt_cheatchr,    mpu4_invcoin,    mpu4unsorted_state, init_m4default, 0,      "Nova?", "Circus Circus 2 (Nova?) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE) // COIN   ALM

// regular barcrest structure, keine tube (hopper issue)
GAME( 199?, m4vivan,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::premier_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, 0,      "Nova",  "Viva Las Vegas (Nova) (MPU4) (GLV 1.2)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// GEEN TUBES
GAME(199?, m4vivalvd, 0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::premier_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "Barcrest","Viva Las Vegas (Barcrest) (Dutch) (MPU4) (DLV 1.1)",GAME_FLAGS )

// GEEN TUBES
GAME(199?, m4prem,    0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::premier_characteriser_prot> ,mpu4,    mpu4unsorted_state, init_m4default_six_alt, ROT0,   "Barcrest","Premier (Barcrest) (Dutch) (MPU4) (DPM 1.4)",GAME_FLAGS )

// runs
GAME( 199?, m4spotln, 0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::viva_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_big, 0,      "Barcrest / Nova",  "Spotlight (Nova) (German) (MPU4) (GSP 0.1)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// runs
GAME( 199?, m4goldnn, 0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::alf_characteriser_prot>,    mpu4,    mpu4unsorted_state, init_m4default_big, 0,      "Nova",  "Golden Years (Nova) (German) (MPU4) (TGY 0.1)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// doesn't boot at all? (checking AUX ports?)
GAME( 199?, m4mgpn,   0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, 0,      "Nova",  "Monaco Grand Prix (Nova) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// no protection?
GAME(198?, m4funh,    0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default, 0,      "<unknown>",      "Fun House (unknown) (MPU4)", GAME_FLAGS ) // TUNE ALARM  (was in the SC1 Fun House set)


// REEL 1 FAULT
// Not the same as Barcrest Everyone's A Winner? has "(C) J.A. Brown 1991" in the ROM
GAME(1991, m4eaw51,   0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0, "J.A. Brown", "Everyone's A Winner (J.A. Brown) (MPU4) (EAW 5.1)", GAME_FLAGS )

// REEL 1 FAULT
// has "(C) J.A. Brown 1993" in the ROM
GAME(1993, m4twist,   0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "J.A. Brown","Twist Again (J.A. Brown) (MPU4) (TA 9.6, set 1)",GAME_FLAGS ) //   REEL 1 FAULT
GAME(1993, m4twista,  m4twist,    mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "J.A. Brown","Twist Again (J.A. Brown) (MPU4) (TA 9.6, set 2)",GAME_FLAGS ) // TA 9.6  REEL 1 FAULT
GAME(1993, m4twistb,  m4twist,    mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "J.A. Brown","Twist Again (J.A. Brown) (MPU4) (TA 9.6, set 3)",GAME_FLAGS ) // TA 9.6  REEL 1 FAULT

// REEL 1 FAULT
// has "(C) J. Brown 1988" in the ROM (assume same J.A. Brown as above)
GAME(1988, m4wnud,    0,          mod2,       mpu4,    mpu4unsorted_state, init_m4default, ROT0,   "J.A. Brown","unknown MPU4 'W Nudge' (J.A. Brown) (MPU4)",GAME_FLAGS )



// corrupt VFD (many XX), doesn't use standard Barcrest protection, maybe Coinworld?
// reads a jump offset from 0x800 (where protection would usually map)
GAME(199?, m4luckwb,  0,          mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "<unknown>","Lucky Wild Boar (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4luckwba, m4luckwb,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "<unknown>","Lucky Wild Boar (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4luckwbb, m4luckwb,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "<unknown>","Lucky Wild Boar (MPU4) (set 3)",GAME_FLAGS )
GAME(199?, m4luckwbc, m4luckwb,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "<unknown>","Lucky Wild Boar (MPU4) (set 4)",GAME_FLAGS )
GAME(199?, m4luckwbd, m4luckwb,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "<unknown>","Lucky Wild Boar (MPU4) (set 5)",GAME_FLAGS )
GAME(199?, m4luckwbe, m4luckwb,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "<unknown>","Lucky Wild Boar (MPU4) (set 6)",GAME_FLAGS )
GAME(199?, m4luckwbf, m4luckwb,   mod4oki,    mpu4,    mpu4unsorted_state, init_m4default,  ROT0,   "<unknown>","Lucky Wild Boar (MPU4) (set 7)",GAME_FLAGS )

