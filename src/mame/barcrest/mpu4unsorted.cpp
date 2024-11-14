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
};



#include "m4aao.lh"


ROM_START( m4casmul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casinomultiplay.bin", 0x0000, 0x010000, CRC(2ebd1800) SHA1(d15e2593d17d8db9c6946af3366cf429ad291f76) )

	ROM_REGION( 0x100000, "okicard:msm6376", 0 )
	ROM_LOAD( "casinomultiplaysnd.bin", 0x0000, 0x080000, CRC(be293e95) SHA1(bf0d419c898920a7546b542d8b205e25004ef04f) )
ROM_END



ROM_START( m4crzjk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crjok2.04.bin", 0x0000, 0x010000, CRC(838336d6) SHA1(6f36de20c930cbbff479af2667c11152c6adb43e) )
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



//Derived from Against_All_Odds_(Eurotek)_[C01_800_15jp].gam
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

ROM_START( m4aao )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "aao2_8.bin", 0x0000, 0x010000, CRC(94ce4016) SHA1(2aecb6dbe798b7bbfb3d27f4d115b6611c7d990f) )

	ROM_REGION( 0x080000, "okicard:msm6376", 0 )
	ROM_LOAD( "aaosnd.bin", 0x0000, 0x080000, CRC(7bf30b96) SHA1(f0086ae239b1d973018a3ea04e816a87f8f20bad) )
ROM_END


ROM_START( m4bandgd )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bog.bin", 0x0000, 0x020000, CRC(21186fb9) SHA1(3d536098c7541cbdf02d68a18a38cae71155d7ff) )

	ROM_REGION( 0x080000, "okicard:msm6376", 0 )
	ROM_LOAD( "bandsofgoldsnd.bin", 0x0000, 0x080000, CRC(95c6235f) SHA1(a13afa048b73fabfad229b5c2f8ef5ee9948d9fb) )
ROM_END


ROM_START( m4bigben )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "b_bv2_7.bin", 0x0000, 0x010000, CRC(9f3a7638) SHA1(b7169dc26a6e136d6daaf8d012f4c3d017e99e4a) )

	ROM_REGION( 0x100000, "okicard:msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbena )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "b_bv2_9.bin", 0x0000, 0x010000, CRC(86a745ee) SHA1(2347e8e38c743ea4d00faee6a56bb77e05c9c94d) ) // aka bb2_9.bin

	ROM_REGION( 0x100000, "okicard:msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbenb )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bb1_9p.bin", 0x0000, 0x010000, CRC(c76c5a09) SHA1(b0e3b38998428f535841ab5373d57cb0d5b21ed3) )

	ROM_REGION( 0x100000, "okicard:msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END


ROM_START( m4bigbend )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bb_2_1.bin", 0x0000, 0x010000, CRC(d3511805) SHA1(c86756998d36e729874c71a5d6442785069c57e9) )

	ROM_REGION( 0x100000, "okicard:msm6376", 0 )
	ROM_LOAD( "big-bensnd1.bin", 0x000000, 0x080000, CRC(e41c3ec1) SHA1(a0c09f51229afcd14f09bb9080d4f3bb198b2050) )
	ROM_LOAD( "big-bensnd2.bin", 0x080000, 0x080000, CRC(ed71dbe1) SHA1(e67ca3c178caacb99118bacfcd7612e699f40455) )
ROM_END

ROM_START( m4bigbene )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "bbs_2_9p.bin", 0x0000, 0x010000, CRC(0107608d) SHA1(9e5def90e77f65c366aea2a9ac24d5f17c4d0ae8) )

	ROM_REGION( 0x100000, "okicard:msm6376", 0 )
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

	ROM_REGION( 0x080000, "okicard:msm6376", 0 )
	ROM_LOAD( "generic_redhotroll_sound1.bin", 0x0000, 0x080000, CRC(3e80f8bd) SHA1(2e3a195b49448da11cc0c089a8a9b462894c766b) )
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

	ROM_REGION( 0x080000, "okicard:msm6376", 0 )
	ROM_LOAD( "stakexsnd.bin", 0x0000, 0x080000, CRC(baf17991) SHA1(282e0ac9d18299e9f7a0fecaf9edf0cb4205ef0e) )
ROM_END

ROM_START( m4stakexa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stakex2.bin", 0x0000, 0x010000, CRC(77ae3f63) SHA1(c5f1cfd5bffcf3156f584757de57ef6530214511) )

	ROM_REGION( 0x080000, "okicard:msm6376", 0 )
	ROM_LOAD( "stakexsnd.bin", 0x0000, 0x080000, CRC(baf17991) SHA1(282e0ac9d18299e9f7a0fecaf9edf0cb4205ef0e) )
ROM_END


ROM_START( m4stand2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stand 2 del 8.bin", 0x08000, 0x08000, CRC(a9a5edc7) SHA1(035d3f3b3373cec475753f1b0de2f4db48d6d288) )
ROM_END







ROM_START( m4barcrz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "barcrazy.bin", 0x0000, 0x010000, CRC(917ad749) SHA1(cb0a3f6737b8f183d2efb0a3f8adbf86d40a38ff) )

	ROM_REGION( 0x080000, "okicard:msm6376", 0 )
	ROM_LOAD( "barcrazysnd.bin", 0x0000, 0x080000, CRC(0e155193) SHA1(7583e9f3e3624f82f2329565bdcbdaa5a5b03ee0) )
ROM_END

ROM_START( m4bonzbn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bingo-bonanza_v1.bin", 0x0000, 0x010000, CRC(3d137ddf) SHA1(1ce23db111448e44a166554dd8853dc379e787da) )

	ROM_REGION( 0x100000, "okicard:msm6376", 0 )
	ROM_LOAD( "bingo-bonanzasnd1.bin", 0x000000, 0x080000, CRC(e0eb2a92) SHA1(cbc0b3bba7857d87535d1c2a7459aed60709734a) )
	ROM_LOAD( "bingo-bonanzasnd2.bin", 0x080000, 0x080000, CRC(7db27b28) SHA1(98c5fa4bf8c7f67fae90a1ca98b74057f5ed9b6b) )
ROM_END

ROM_START( m4dnj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d.n.j 1-02", 0x0000, 0x010000, CRC(5750843d) SHA1(b87923e84071ea4a1af7566a7f413f8e30e208e9) )
	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 ) // should this set have an OKI?
ROM_END

ROM_START( m4dnja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d.n.j 1-03", 0x0000, 0x010000, CRC(7b805255) SHA1(f62765bfa66e2422ac0a71ebaff27f1ccd470fe2) )
	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 ) // should this set have an OKI?
ROM_END

ROM_START( m4dnjb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d.n.j 1-06", 0x0000, 0x010000, CRC(aab770c7) SHA1(f24fff8346915017bc43fef9fac356a067676d86) )
	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 ) // should this set have an OKI?
ROM_END


ROM_START( m4matdr )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "matador.bin", 0x0000, 0x020000, CRC(367788a4) SHA1(3c9b077a64f993cb60107558efdfcbee0fe5c958) )

	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 )
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
	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 ) \
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

	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "criss cross crazy sound,27c2001", 0x0000, 0x040000, CRC(1994c509) SHA1(2bbe91a43aa9953b7776faf67e81e30a4f7b7cb2) )
ROM_END


ROM_START( m4treel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trgv1.1s", 0x0000, 0x010000, CRC(a9c76b08) SHA1(a5b3bc980eb58e346cb02d8ca43401f304e5b6de) )
	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4treela )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "trgv1.1b", 0x0000, 0x020000, CRC(7eaebef6) SHA1(5ab86329041e7df09cc2e3ce8d5afd44d88c246c) )
	ROM_REGION( 0x100000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END





ROM_START( m4wnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wnudge.bin", 0x8000, 0x008000, CRC(1d935575) SHA1(c4177c41473c0fb511e0ee035961f55ad43be14d) )
ROM_END



#define M4SURF_EXTRAS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
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

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// missing?
ROM_END

#define M4BLKGD_EXTRAS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
	ROM_LOAD( "blackgoldsnd1.bin", 0x000000, 0x080000, CRC(d251b59e) SHA1(960b81b87f0fb5000028c863892a273362cb897f) ) \
	ROM_LOAD( "blackgoldsnd2.bin", 0x080000, 0x080000, CRC(87cbcd1e) SHA1(a6cd186af7c5682e216f549b77735b9bf1b985ae) ) \
	ROM_LOAD( "blackgoldsnd3.bin", 0x100000, 0x080000, CRC(258f7b83) SHA1(a6df577d98ade8c5c5ff68ef891667e65e83ac17) )
ROM_START( m4blkgd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "blackgoldprg.bin", 0x0000, 0x080000, CRC(a04736b2) SHA1(9e060cc79e7922b38115f1412ed76f8c76deb917) )
	M4BLKGD_EXTRAS
ROM_END

ROM_START( m4blkgda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "blackgoldversion2.4.bin", 0x0000, 0x020000, CRC(fad4e360) SHA1(23c6a13e8d1ca307b0ef22edffed536675985aca) )
	ROM_CONTINUE( 0x0000, 0x020000 ) //Early rom banks empty
	M4BLKGD_EXTRAS
ROM_END

#define M4ZILL_EXTRAS \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
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
	ROM_REGION( 0x200000, "okicard:altmsm6376", 0 ) \
	ROM_LOAD( "happystreak.p1", 0x0000, 0x080000, CRC(b1f328ff) SHA1(2bc6605965cb5743a2f8b813d68cf1646a4bcac1) ) \
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
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
	ROM_REGION( 0x200000, "okicard:msm6376", 0 ) \
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
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4hapfrta )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf1_1p.bin", 0x0000, 0x010000, CRC(ebb6ee66) SHA1(1f9b67260e5becd013d95358cc89acb1099d655d) )
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4hapfrtb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf1_4pk.bin", 0x0000, 0x010000, CRC(0944b3c6) SHA1(00cdb75dda4f8984f77806047ad79fe9a1a8760a) )
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4sunday )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sunday_sport_v11", 0x0000, 0x010000, CRC(14147d59) SHA1(03b14f4f83a545b3252702267ac012b3be76013d) )
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4jp777 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jpot71", 0x0000, 0x010000, CRC(f4564a05) SHA1(97d21e2268e5d99e6e51cb12c45e09445cff1f50) )
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4booze )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "boozecruise10_v10.bin", 0x0000, 0x010000, CRC(b37f752b) SHA1(166f7d17694689bd9d51d859c13ddafa1c6e5e7f) )
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4aliz )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "70000000.bin", 0x0000, 0x040000, CRC(56f64dd9) SHA1(11f990c9a6864a969dc9a4146e1ac2c963e3eb9b) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "alizsnd.hi", 0x0000, 0x080000, CRC(c7bd937a) SHA1(cc4d85a3d4cdf57fa96c812a4cd78b599c7052ff) )
	ROM_LOAD( "alizsnd.lo", 0x080000, 0x04e15e, CRC(111cc111) SHA1(413efedbc9e85240df833c10d680b0e907da10b3) )

	ROM_REGION( 0x200000, "misc", ROMREGION_ERASE00 ) // i think this is just the sound roms as intelhex
	ROM_LOAD( "71000000.hi", 0x0000, 0x0bbe9c, CRC(867058c1) SHA1(bd980cb0bb3075854cc2e9b829c31f3742f4f1c2) )
	ROM_LOAD( "71000000.lo", 0x0000, 0x134084, CRC(53046751) SHA1(b8f9eca933315b497732c895f4311f62103344fc) )
ROM_END


ROM_START( m4c2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ci2-0601.bin", 0x0000, 0x010000, CRC(84cc8aca) SHA1(1471e3ad9c9ba957b6cc99c204fe588cc55fbc50) )
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4coney )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "70000060.bin", 0x0000, 0x010000, CRC(fda208e4) SHA1(b1a243b2681faa03add4ab6e4df98814f9c52fc5) )
	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
ROM_END



ROM_START( m4eaw51 )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "everyones a winner v2-5p", 0x08000, 0x008000, CRC(eb8f2fc5) SHA1(0d3614bd5ff561d17bef0d1e620f2f812b8fed5b))
ROM_END

} // anonymous namespace

using namespace mpu4_traits;

#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK|MACHINE_MECHANICAL)

// works.  This is not a Barcrest game, but has a standard CHR check after you coin up for the first time, expecting the m4lv sequence back.  Reports ILLEGAL SOFTWARE if it fails
GAME(199?, m4bandgd,  0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Eurogames","Bands Of Gold (Eurogames) (MPU4)",GAME_FLAGS )

// ERROR CODE 2. as with m4bandgd this isn't a Barcrest, but does the standard CHR check and shows ILLEGAL SOFTWARE if check fails, assume it is also by Eurogames
GAME(199?, m4matdr,   0,          mod4oki_cheatchr_pal<mpu4_characteriser_pal::m4lv_characteriser_prot>(R6, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Eurogames","Matador (Eurogames) (MPU4)",GAME_FLAGS )

// works, not standard protection, but cheatchr works
GAME(199?, m4bigben,  0,          mod4oki_cheatchr(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 1)",GAME_FLAGS )
GAME(199?, m4bigbena, m4bigben,   mod4oki_cheatchr(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 2)",GAME_FLAGS )
GAME(199?, m4bigbenb, m4bigben,   mod4oki_cheatchr(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 3)",GAME_FLAGS )
GAME(199?, m4bigbend, m4bigben,   mod4oki_cheatchr(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 4)",GAME_FLAGS )
GAME(199?, m4bigbene, m4bigben,   mod4oki_cheatchr(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 5)",GAME_FLAGS )

GAME(199?, m4kqclub,  0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Newby","Kings & Queens Club (Newby) (MPU4)",GAME_FLAGS )

GAME(199?, m4snookr,  0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Eurocoin","Snooker (Eurocoin) (MPU4)",GAME_FLAGS ) // works?

GAME(199?, m4stakex,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 1)",GAME_FLAGS ) // has issues with coins in 'separate bank' (default) mode, reel issues
GAME(199?, m4stakexa, m4stakex,   mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 2)",GAME_FLAGS ) // like above, but doesn't default to separate bank?

GAME(199?, m4boltbl,  0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 1)",GAME_FLAGS ) // Reel 1 Fault
GAME(199?, m4boltbla, m4boltbl,   mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 2)",GAME_FLAGS )
GAME(199?, m4boltblb, m4boltbl,   mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 3)",GAME_FLAGS )
GAME(199?, m4boltblc, m4boltbl,   mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 4)",GAME_FLAGS )

GAME(199?, m4stand2,  0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "DJE","Stand To Deliver (DJE) (MPU4)",GAME_FLAGS ) // Reel 1 Fault

GAME(199?, m4dblchn,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "DJE","Double Chance (DJE) (MPU4)",GAME_FLAGS ) // Reels spin forever

// other issues, only plays an 'alarm' sound when there's money to payout? wrong sound ROM or something else?
GAME(2001, m4casmul,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "DJE","Casino Multiplay (MPU4)",GAME_FLAGS ) // unprotected, copyright year / manufacturer found in ROM

 // has D.J.E 1999 copyright in ROM
GAMEL(1999, m4aao,    0,          mod4oki(R4, RT1, HDA),    mpu4,    mpu4unsorted_state, init_m4,     ROT0,   "DJE / Eurotek","Against All Odds (Eurotek) (MPU4)",GAME_FLAGS, layout_m4aao )

/* Unknown stuff that looks like it might be MPU4, but needs further verification, some could be bad */

// PAL FAIL
GAME(199?, m4barcrz,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "<unknown>","Bar Crazy (unknown) (MPU4?)",GAME_FLAGS )

// gives an error
GAME(199?, m4bonzbn,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "<unknown>","Bingo Bonanza (unknown) (MPU4?)",GAME_FLAGS )


/* *if* these are MPU4 they have a different sound system at least - The copyright strings in them are 'AET' tho (Ace?) - Could be related to the Crystal stuff? */
GAME(199?, m4sbx,     0,          mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4sbxa,    m4sbx,      mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4sbxb,    m4sbx,      mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 3)",GAME_FLAGS )
GAME(199?, m4sbxc,    m4sbx,      mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 4)",GAME_FLAGS )
GAME(199?, m4sbxd,    m4sbx,      mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 5)",GAME_FLAGS )
GAME(199?, m4sbxe,    m4sbx,      mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Super Bear X (MPU4?) (set 6)",GAME_FLAGS )

GAME(199?, m4bclimb,  0,          mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Bear Climber (MPU4?)",GAME_FLAGS )

GAME(199?, m4captb,   0,          mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Captain Bear (MPU4?)",GAME_FLAGS )

GAME(199?, m4jungj,   0,          mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4jungja,  m4jungj,    mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4jungjb,  m4jungj,    mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 3)",GAME_FLAGS )
GAME(199?, m4jungjc,  m4jungj,    mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Jungle Japes (MPU4?) (set 4)",GAME_FLAGS )

GAME(199?, m4fsx,     0,          mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Fun Spot X (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4fsxa,    m4fsx,      mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Fun Spot X (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4fsxb,    m4fsx,      mod4oki(R4, RT1),   mpu4,    mpu4unsorted_state, init_m4,  ROT0,   "AET/Coinworld","Fun Spot X (MPU4?) (set 3)",GAME_FLAGS )

// Error 42 then 52, 54
GAME(199?, m4ccop,    0,          mod4oki(R4, RT1),    mpu4_cw, mpu4unsorted_state, init_m4, ROT0, "Coinworld","Cash Cops (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4ccopa,   m4ccop,     mod4oki(R4, RT1),    mpu4_cw, mpu4unsorted_state, init_m4, ROT0, "Coinworld","Cash Cops (MPU4?) (set 2)",GAME_FLAGS )
GAME(199?, m4ccopb,   m4ccop,     mod4oki(R4, RT1),    mpu4_cw, mpu4unsorted_state, init_m4, ROT0, "Coinworld","Cash Cops (MPU4?) (set 3)",GAME_FLAGS )

// Error 72
GAME(199?, m4ccc,     0,          mod4oki(R4, RT1),    mpu4_cw, mpu4unsorted_state, init_m4, ROT0, "Coinworld","Criss Cross Crazy (Coinworld) (MPU4?)",GAME_FLAGS )

// PAL ERROR
GAME(199?, m4treel,   0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "<unknown>","Turbo Reels (unknown) (MPU4?) (set 1)",GAME_FLAGS )
GAME(199?, m4treela,  m4treel,    mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "<unknown>","Turbo Reels (unknown) (MPU4?) (set 2)",GAME_FLAGS )

// works
GAME(199?, m4surf,    0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Gemini","Super Surfin' (Gemini) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4surfa,   m4surf,     mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Gemini","Super Surfin' (Gemini) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4surfb,   m4surf,     mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Gemini","Super Surfin' (Gemini) (MPU4) (set 3)",GAME_FLAGS )

// works
GAME(199?, m4wife,    0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Gemini","Money Or Yer Wife (Gemini) (MPU4)",GAME_FLAGS )

// works, error unless you set %
GAME(199?, m4blkgd,   0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Gemini","Black Gold (Gemini) (MPU4) (set 1)",GAME_FLAGS )
// doesn't work, might be bad dump
GAME(199?, m4blkgda,  m4blkgd,    mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Gemini","Black Gold (Gemini) (MPU4) (set 2)",GAME_FLAGS )

// runs, non-reel game?
GAME(199?, m4zill,    0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Pure Leisure","Zillionare's Challenge (Pure Leisure) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4zilla,   m4zill,     mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Pure Leisure","Zillionare's Challenge (Pure Leisure) (MPU4) (set 2)",GAME_FLAGS )

// ALARM S4
GAME(199?, m4hstr,    0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Happy Streak (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4hstra,   m4hstr,     mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Happy Streak (Coinworld) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4hstrb,   m4hstr,     mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Happy Streak (Coinworld) (MPU4) (set 3)",GAME_FLAGS )

// "S4" error
GAME(199?, m4hstrcs,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4hstrcsa, m4hstrcs,   mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4hstrcsb, m4hstrcs,   mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 3)",GAME_FLAGS )
GAME(199?, m4hstrcsc, m4hstrcs,   mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 4)",GAME_FLAGS )
GAME(199?, m4hstrcsd, m4hstrcs,   mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4) (set 5)",GAME_FLAGS )

// Error 1.8
GAME(199?, m4ddb,     0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Ding Dong Bells (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4ddba,    m4ddb,      mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Ding Dong Bells (Coinworld) (MPU4) (set 2)",GAME_FLAGS )

// "S4" error
GAME(199?, m4hapfrt,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Happy Fruits (Coinworld) (MPU4) (set 1)",GAME_FLAGS )
GAME(199?, m4hapfrta, m4hapfrt,   mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Happy Fruits (Coinworld) (MPU4) (set 2)",GAME_FLAGS )
GAME(199?, m4hapfrtb, m4hapfrt,   mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Coinworld","Happy Fruits (Coinworld) (MPU4) (set 3)",GAME_FLAGS )

// backwards VFD
GAME(199?, m4sunday,  0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Pcp","Sunday Sport (Pcp) (MPU4)",GAME_FLAGS )

// HOPPER FAULT
GAME(199?, m4jp777,   0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Cotswold Microsystems","Jackpot 777 (Cotswold Microsystems) (MPU4)",GAME_FLAGS ) /* Hopper Fault */

// HOPPER FAULT
GAME(199?, m4dnj,     0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Cotswold Microsystems","Double Nudge (Cotswold Microsystems) (MPU4) (set 1)",GAME_FLAGS ) /* Hopper Fault */
GAME(199?, m4dnja,    m4dnj,      mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Cotswold Microsystems","Double Nudge (Cotswold Microsystems) (MPU4) (set 2)",GAME_FLAGS ) /* Hopper Fault */
GAME(199?, m4dnjb,    m4dnj,      mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Cotswold Microsystems","Double Nudge (Cotswold Microsystems) (MPU4) (set 3)",GAME_FLAGS ) /* Hopper Fault */

// PIC CHECK, backwards VFD
GAME(199?, m4booze,   0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, ROT0,   "Extreme","Booze Cruise (Extreme) (MPU4)",GAME_FLAGS )

// LINKUP SUCHE
GAME( 199?, m4aliz,   0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4big, 0,      "Qps",  "AlizBaz (Qps) (German) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// SHELF RESONSE
GAME( 199?, m4coney,  0,          mod4oki(R4, RT1),    mpu4,    mpu4unsorted_state, init_m4, 0,      "Qps",   "Coney Island (Qps) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE)

// very similar to m4c2 behavior below, but no protection?
GAME( 199?, m4crzjk,  0,          mod2(R4, RT2),       mpu4_invcoin,    mpu4unsorted_state, init_m4, 0,      "Nova?", "Crazy Jokers (Nova?) (MPU4)",GAME_FLAGS )

// not standard protection, but cheatchr passes it, code crashes after a short time?
GAME( 199?, m4c2,     0,          mod4oki_cheatchr(R4, RT2),    mpu4_invcoin,    mpu4unsorted_state, init_m4, 0,      "Nova?", "Circus Circus 2 (Nova?) (MPU4)",GAME_FLAGS|MACHINE_MECHANICAL|MACHINE_SUPPORTS_SAVE) // COIN   ALM


// REEL 1 FAULT
// Not the same as Barcrest Everyone's A Winner? has "(C) J.A. Brown 1991" in the ROM
GAME(1991, m4eaw51,   0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0, "J.A. Brown", "Everyone's A Winner (J.A. Brown) (MPU4) (EAW 5.1)", GAME_FLAGS )

// REEL 1 FAULT
// has "(C) J.A. Brown 1993" in the ROM
GAME(1993, m4twist,   0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "J.A. Brown","Twist Again (J.A. Brown) (MPU4) (TA 9.6, set 1)",GAME_FLAGS ) //   REEL 1 FAULT
GAME(1993, m4twista,  m4twist,    mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "J.A. Brown","Twist Again (J.A. Brown) (MPU4) (TA 9.6, set 2)",GAME_FLAGS ) // TA 9.6  REEL 1 FAULT
GAME(1993, m4twistb,  m4twist,    mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "J.A. Brown","Twist Again (J.A. Brown) (MPU4) (TA 9.6, set 3)",GAME_FLAGS ) // TA 9.6  REEL 1 FAULT

// REEL 1 FAULT
// has "(C) J. Brown 1988" in the ROM (assume same J.A. Brown as above)
GAME(1988, m4wnud,    0,          mod2(R4, RT1),       mpu4,    mpu4unsorted_state, init_m4, ROT0,   "J.A. Brown","unknown MPU4 'W Nudge' (J.A. Brown) (MPU4)",GAME_FLAGS )
