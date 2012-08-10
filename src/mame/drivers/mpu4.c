/* these are the MPU4 set listings / set specific code, for hardware emulation see mpu4hw.c */

/* todo: driver inits (basic hw reel, protection configs etc.) should probably be moved here
         once the actual code for them is cleaned up and can be put into neater structures
		 like bfm_sc4
		 
		 due to the vast number of sets here this might be further split up by manufacturer
		 
*/

#include "emu.h"
#include "includes/mpu4.h"

MACHINE_CONFIG_EXTERN( mod4oki );
MACHINE_CONFIG_EXTERN( mod4yam );
MACHINE_CONFIG_EXTERN( mpu4crys );
MACHINE_CONFIG_EXTERN( bwboki );
MACHINE_CONFIG_EXTERN( mod2 );

INPUT_PORTS_EXTERN( mpu4 );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn );
INPUT_PORTS_EXTERN( mpu4jackpot8per );
INPUT_PORTS_EXTERN( grtecp );

ROM_START( m4tst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut4.p1",  0xC000, 0x4000,  CRC(086dc325) SHA1(923caeb61347ac9d3e6bcec45998ddf04b2c8ffd))
ROM_END

ROM_START( m4tst2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ut2.p1",  0xE000, 0x2000,  CRC(f7fb6575) SHA1(f7961cbd0801b9561d8cd2d23081043d733e1902))
ROM_END

ROM_START( m4clr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "meter-zero.p1",  0x8000, 0x8000,  CRC(e74297e5) SHA1(49a2cc85eda14199975ec37a794b685c839d3ab9))
ROM_END



ROM_START( m4blsbys )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD("bbprog.bin",  0x00000, 0x20000,  CRC(c262cfda) SHA1(f004895e0dd3f8420683927915554e19e41bd20b))
	ROM_RELOAD(0x40000,0x20000)

	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "bf_20a__.3_1", 0x0000, 0x020000, CRC(fca7764f) SHA1(a88378247b6710d6122c515c31c39c5cd9678ce2) )
	ROM_LOAD( "bf_20a__.6_1", 0x0000, 0x020000, CRC(0822931a) SHA1(8d53321832ee56ed5ad851928ad7705e1ad059ee) )
	ROM_LOAD( "bf_20s__.3_1", 0x0000, 0x020000, CRC(029a3a0b) SHA1(25952cafbc351ec6d5fc65dd2acddadeb48fb649) )
	ROM_LOAD( "bf_20s__.6_1", 0x0000, 0x020000, CRC(af7462cb) SHA1(188935ebb574e0b09f9e0e5f094eb99ed7df5075) )
	ROM_LOAD( "bf_20sb_.3_1", 0x0000, 0x020000, CRC(d2ff1a12) SHA1(d425985a8d109c9e3729618a995eeac1f3bf643c) )
	ROM_LOAD( "bf_20sb_.6_1", 0x0000, 0x020000, CRC(7548bdf1) SHA1(a366c10f35d85b1f3c103787dd5b2e6600ecf6c8) )
	ROM_LOAD( "bs_05___.3v1", 0x0000, 0x020000, CRC(26e8eb95) SHA1(7d8dbca127e1867714cbeb9d699b2173de724eb2) )
	ROM_LOAD( "bs_05__c.3v1", 0x0000, 0x020000, CRC(12e51237) SHA1(68235cf5f36862a26d5d44464041dabb01b9f95c) )
	ROM_LOAD( "bs_05_b_.3v1", 0x0000, 0x020000, CRC(a8ec1731) SHA1(5acd86b1018301df5c3caa388bbbee0c0daacb40) )
	ROM_LOAD( "bs_05_d_.3v1", 0x0000, 0x020000, CRC(5def4142) SHA1(e0019979f65e240002455d5aa41e1492dc517740) )
	ROM_LOAD( "bs_05_k_.3v1", 0x0000, 0x020000, CRC(d023bad9) SHA1(54c995c5166f018c41dd7007163f07f054f564e9) )
	ROM_LOAD( "bs_05_kc.3v1", 0x0000, 0x020000, CRC(0b8911da) SHA1(1426944c01edbf6edb664f961fe75befd3e81e65) )
	ROM_LOAD( "bs_05a__.2_1", 0x0000, 0x020000, CRC(88af47c9) SHA1(e5dfd572b47f2cff6e6ef68bd9016a3e5b2273b2) )
	ROM_LOAD( "bs_05a__.3v1", 0x0000, 0x00e561, CRC(a06db3b3) SHA1(ebdc56a2810612b495d35225b640d0cd42cdd9f3) )
	ROM_LOAD( "bs_05a_c.3v1", 0x0000, 0x020000, CRC(737999c4) SHA1(957fd06f7937b85436ef7011d47c559729d17d51) )
	ROM_LOAD( "bs_05b__.3v1", 0x0000, 0x020000, CRC(ea1b09e7) SHA1(9310e3d63b77c4bc2d0d428a630c8aafb500de74) )
	ROM_LOAD( "bs_05b_c.3v1", 0x0000, 0x020000, CRC(3ec0ce9f) SHA1(09472d392350da1593f084a2591125b0d71aca29) )
	ROM_LOAD( "bs_20__c.1_1", 0x0000, 0x020000, CRC(328dc0a6) SHA1(7bea0bac121f2c521d891267fb891479549350aa) )
	ROM_LOAD( "bs_20a__.6_1", 0x0000, 0x020000, CRC(9968e21f) SHA1(8615c8b7f0ae55f0b77f30c84a74d1fba0450a73) )
	ROM_LOAD( "bs_20a__.7_1", 0x0000, 0x020000, CRC(c48bc5d6) SHA1(5aa44d85c1a33ea1aa27e2eaa987ce4fe570b713) )
	ROM_LOAD( "bs_20a_c.1_1", 0x0000, 0x020000, CRC(b9483173) SHA1(a81ef9cb42cd090861b2e0a28a63906cf61c7534) )
	ROM_LOAD( "bs_20a_p.4_1", 0x0000, 0x020000, CRC(fb7ec0aa) SHA1(a0b681a8eacde06825c7e4fcf5b7ef8f64723d96) )
	ROM_LOAD( "bs_20a_s.6_1", 0x0000, 0x020000, CRC(72c5c16e) SHA1(18bdb0f9aff13587d95d871b4124d5a1cc07af04) )
	ROM_LOAD( "bs_20s__.6_1", 0x0000, 0x020000, CRC(1862df89) SHA1(b18f15f2098dfc488a6bdb9a7adff4446268f0d3) )
	ROM_LOAD( "bs_20s_s.6_1", 0x0000, 0x020000, CRC(f3cffcf8) SHA1(bda99c269e85baf64f66788f868879d21a896c5a) )
	ROM_LOAD( "bs_20sb_.6_1", 0x0000, 0x020000, CRC(3278bdcb) SHA1(c694d485c79be983924410ce00d364d233a054c0) )
	ROM_LOAD( "bs_20sbc.1_1", 0x0000, 0x020000, CRC(3a55b728) SHA1(948b353ec92aae1cb801e5e2d2385cd2316f0838) )
	ROM_LOAD( "bs_20sbs.6_1", 0x0000, 0x020000, CRC(d9d59eba) SHA1(91f608dc33541297797f937546fa5759b26af511) )
	ROM_LOAD( "bs_20sd_.6_1", 0x0000, 0x020000, CRC(df087fd9) SHA1(35c85ffe0bef5847a3a72bb33f196f4708f03b28) )
	ROM_LOAD( "bs_20sdc.1_1", 0x0000, 0x020000, CRC(1439c63e) SHA1(fdac1199e98bbecd944431338d2215d434a0c004) )
	ROM_LOAD( "bs_20sds.6_1", 0x0000, 0x020000, CRC(34a55ca8) SHA1(393d0164699a91fcb4eb68ca5246a0744e245873) )
	ROM_LOAD( "bs_25__c.2_1", 0x0000, 0x020000, CRC(d41de6c6) SHA1(43c059ef673de9cc1ef800f3da68b3a6fd54e8f7) )
	ROM_LOAD( "bs_25_bc.2_1", 0x0000, 0x020000, CRC(b692305d) SHA1(4ed96655a4fdfd97fa5783648f8cdea5af0c565e) )
	ROM_LOAD( "bs_25_dc.2_1", 0x0000, 0x020000, CRC(bbe844a5) SHA1(bd1d5d8601b0c36a2f4abaae04a49626cbef23b6) )
	ROM_LOAD( "bs_25a_c.2_1", 0x0000, 0x020000, CRC(0b64cc29) SHA1(43b958321ad5a04aae4629929844643dfcf17819) )
	ROM_LOAD( "bs_x3a__.2v1", 0x0000, 0x020000, CRC(99471e88) SHA1(e566cd1368e7234ec546b05528f2fcf345e03697) )
	ROM_LOAD( "bs_x3s__.2v1", 0x0000, 0x020000, CRC(84249d95) SHA1(3962ee9fde49b25c3485c7bfaecd63c400d8502d) )
	ROM_LOAD( "bs_x6a__.2v1", 0x0000, 0x020000, CRC(d03ef955) SHA1(03fa4b3b37b71fb61439200d5dd65dab846abc2c) )
	ROM_LOAD( "bs_x6s__.2v1", 0x0000, 0x020000, CRC(61d782b5) SHA1(70ca3875ff023fc091a6fe6e002fad662dbd639f) )
	ROM_LOAD( "bsix3___.2v1", 0x0000, 0x020000, CRC(4e7451fa) SHA1(b1417f948c7e80f506b90d6608f6dd79739389bf) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "bbsnd.p1",  0x000000, 0x080000,  CRC(715c9e95) SHA1(6a0c9c63e56cfc21bf77cf29c1b844b8e0844c1e) )
	ROM_LOAD( "bbsnd.p2",  0x080000, 0x080000,  CRC(594a87f8) SHA1(edfef7d08fab41fb5814c92930f08a565371eae1) )
ROM_END






ROM_START( m4actbnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "acts.p1", 0x0000, 0x010000, CRC(49a9007c) SHA1(b205270e53264c3d8cb009a5780cacba1ce2e2a8) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "actb.p1", 0x0000, 0x010000, CRC(1429708e) SHA1(8b3ecb443e5920ccec80695a142cb1eb9596b1c1) )
	ROM_LOAD( "actbd.p1", 0x0000, 0x010000, CRC(727d7bb6) SHA1(765a9944ee27b175ba1f45bf82dcf7ef0defd076) )
	ROM_LOAD( "actc.p1", 0x0000, 0x010000, CRC(0a2498e5) SHA1(97f1e35426156c8eece6f76f3ecffa85714ade5b) )
	ROM_LOAD( "actd.p1", 0x0000, 0x010000, CRC(c89f6957) SHA1(3389e933e1ba7c1e32d43de572f24e2612e0ee99) )
	ROM_LOAD( "actdk.p1", 0x0000, 0x010000, CRC(1342ac16) SHA1(659618dc7c0c2d370e6c418620d2f399c1725aaf) )
	ROM_LOAD( "actdy.p1", 0x0000, 0x010000, CRC(045b2c9b) SHA1(4136dbb417e41b601394575a0f13ecb3bc89aecb) )
	ROM_LOAD( "actk.p1", 0x0000, 0x010000, CRC(fa555503) SHA1(8bf2b0b28453eedf6541f3471e8c6ffaba04de9d) )
	ROM_LOAD( "acty.p1", 0x0000, 0x010000, CRC(0ab8ff54) SHA1(7dc16ef1bbed2a5b2da3bf9eb4bbaf87b176954f) )
	ROM_LOAD( "actad.p1", 0x0000, 0x010000, CRC(a8dfdf77) SHA1(92e9f0f3837e466c0c6d98b890234d80318ef236) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "actsnd.p1", 0x000000, 0x080000, CRC(34777fea) SHA1(be784e73586719219ae5c1a3841f0e44edb6b497) )
	ROM_LOAD( "actsnd.p2", 0x080000, 0x080000, CRC(2e832d40) SHA1(622b2c9694714446dbf67beb67d03af97d14ece7) )
ROM_END




ROM_START( m4addrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dal12.bin", 0x0000, 0x010000, CRC(4affa79a) SHA1(68bceab42b3616641a34a64a83306175ffc1ce32) )
ROM_END

ROM_START( m4addrc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "add05_101", 0x0000, 0x010000, CRC(4b3fb104) SHA1(9dba619019a476ce317122a3553965b279c684ba) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "add10_101", 0x0000, 0x010000, CRC(af8f8b4e) SHA1(712c33ed0f425dc10b79780b0cfce0ac5768e2d5) )
	ROM_LOAD( "add20_101", 0x0000, 0x010000, CRC(361b7173) SHA1(dea2b1b0f5910e2fd3f45d220554f0e712dedada) )
	ROM_LOAD( "add55", 0x0000, 0x010000, CRC(48c5bc73) SHA1(18c9f70bad6141cca95b6bbcb4fc621e71f87700) )
	ROM_LOAD( "alddr20", 0x0000, 0x010000, CRC(19cf4437) SHA1(b528823c476bebd1a9a6c720a4144294743693d2) )
	ROM_LOAD( "classic adders & ladders (labelled - nudge w 5p 10p 1p10).bin", 0x0000, 0x010000, CRC(ac948903) SHA1(e07023efd7722a661a2bbf93c0a168af70ad6c20) )
	ROM_LOAD( "classic adders & ladders (labelled connect 4 v1-9 (27512) 15-3-90)", 0x0000, 0x010000, CRC(843ed53d) SHA1(b1dff249df37800744e3fc9c32be20a62bd130a1) )

	ROM_REGION( 0x10000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ad_05s___183.bin", 0x0000, 0x010000, CRC(8d9e0f5d) SHA1(fecc844908876e161d0134ce3cc098e79e74e0b1) )
	ROM_LOAD( "adders classic.bin", 0x0000, 0x010000, CRC(6bc1d2aa) SHA1(cf17e697ff0cfba999f6511f24051dbc3d0384ef) )
	ROM_LOAD( "addl_10_.4", 0x0000, 0x010000, CRC(c2d11126) SHA1(0eafe9dc30013ed5817ac303a4eea5ea82d62715) )
	ROM_LOAD( "addl_10_.8", 0x0000, 0x010000, CRC(9fc82c47) SHA1(0f56afc33f09fe22afc5ec74aeb496c32f9e623c) )
	ROM_LOAD( "addl_20_.8", 0x0000, 0x010000, CRC(43c98f46) SHA1(0ca4a093b38fc04639e3f4bb742a8923b90d2ed1) )
	ROM_LOAD( "al10", 0x0000, 0x010000, CRC(3c3c82b6) SHA1(cc5ffdd0837c9af31d5737a70430a01d1989cdcc) )
	ROM_LOAD( "alad58c", 0x0000, 0x010000, CRC(df9c46b8) SHA1(439ea1ce17aa89e19cedb78465b4388b72c8c5ed) )
	ROM_LOAD( "nik56c", 0x0000, 0x010000, CRC(05fa11d1) SHA1(01d3d0c504489f1513a0c3aa26e910c9604f5366) )

ROM_END

ROM_START( m4addrcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "adcd.p1", 0x0000, 0x010000, CRC(47e41c9a) SHA1(546aaaa5765b3bc91eeb9bf5a979ed68a2e72da8) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "adcf.p1", 0x0000, 0x010000, CRC(1dbbc990) SHA1(fb9439b43089e3135a719ab94b24dd65561d17cf) )
	ROM_LOAD( "adcl.p1", 0x0000, 0x010000, CRC(89299196) SHA1(9a92b250b47b11536f8708429d69c95111ecdb98) )
	ROM_LOAD( "adcs.p1", 0x0000, 0x010000, CRC(7247de78) SHA1(e390b4e912d7bc8c1ca5e42bf2e2753d4c2b4d17) )
	ROM_LOAD( "adrscfm", 0x0000, 0x010000, CRC(6c95881a) SHA1(db658bd722c54fc84734105f1a9b0028b23179fb) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "aal.chr", 0x0000, 0x000048, CRC(bb48409f) SHA1(adefde520104b8c3815260ee136460ddf3e9e4b2) )
ROM_END





ROM_START( m4amhiwy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dah20", 0x0000, 0x010000, CRC(e3f92f00) SHA1(122c8a429a1f75dac80b90c4f218bd311813daf5) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdr6_1.snd", 0x000000, 0x080000, CRC(63ad952d) SHA1(acc0ac3898fcc281e2d7ba19ada52d727885fe06) )
	ROM_LOAD( "sdr6_2.snd", 0x080000, 0x080000, CRC(48d2ace5) SHA1(ada0180cc60266c0a6d981a019d66bbedbced21a) )
ROM_END

ROM_START( m4andybt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "abt18d.p1", 0x0000, 0x020000, CRC(77874578) SHA1(455964614b67af14f5baa5883e1076e986de9e9c) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "abt18f.p1", 0x0000, 0x020000, CRC(cdd756af) SHA1(b1bb851ad2a2ba631e13509a476fe60cb8a24e69) )
	ROM_LOAD( "abt18s.p1", 0x0000, 0x020000, CRC(625263e4) SHA1(23fa0547164cc1f9b7c6cd26e06b0d779bf0329d) )
	ROM_LOAD( "abt1.5",	   0x0000, 0x020000, CRC(05303209) SHA1(6a9eba19e7138ede122ec04c062556763b80f6c0) )//earlier revision

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "abt18s.chr", 0x0000, 0x000048, CRC(68007536) SHA1(72f7a76a1ba1c8ac94de425892780ffe78269513) )


	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "abtsnd.p1", 0x000000, 0x080000, CRC(0ba1e73a) SHA1(dde70b1bf973b023c45afb8d3191325514b96e47) )
	ROM_LOAD( "abtsnd.p2", 0x080000, 0x080000, CRC(dcfa85f2) SHA1(30e8467841309a4840824ec89f82044489c94ac5) )
ROM_END





ROM_START( m4bnkrol )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cbr05s.p1", 0x0000, 0x020000, CRC(a8b53a0d) SHA1(661ab61aa8f427b92fdee02539f19e5dd2243da7) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "br301d.p1", 0x0000, 0x020000, CRC(b9334e2d) SHA1(263808eb5ea3f9987eb7579b43329cb27e109921) )
	ROM_LOAD( "br301f.p1", 0x0000, 0x020000, CRC(c4be5b69) SHA1(9b08d5c0c5aebeef9f0767f5bd456cc6b05ea317) )
	ROM_LOAD( "br301s.p1", 0x0000, 0x020000, CRC(1e117651) SHA1(c06d3f14e55be83c89c8132cf219d46acc42991c) )
	ROM_LOAD( "cbr05d.p1", 0x0000, 0x020000, CRC(44cefec0) SHA1(7034c5acd44ccd3cd985ba4945c004c070a599a4) )
	ROM_LOAD( "cbr05f.p1", 0x0000, 0x020000, CRC(3943eb84) SHA1(76a00db6a0c6655c3a7942550c788822bacd73e5) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cbrsnd.p1", 0x000000, 0x080000, CRC(3524418a) SHA1(85cf286d9cf97cc9009c0283d632fef2a19f5de2) )
	ROM_LOAD( "cbrsnd.p2", 0x080000, 0x080000, CRC(a53796a3) SHA1(f094f40cc93ea445922a9c5412aa355b7d21b1f4) )
ROM_END

ROM_START( m4btclok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "beattheclock.hex", 0x6000, 0x00a000, CRC(a0d4e463) SHA1(45d1df08bfd70caf63b14d2ccc56038ed85e23d0) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "btc.chr", 0x0000, 0x000048, CRC(c77e5215) SHA1(c9e26ed593840cbc47dad893ea4df476f1d69ecd) )
ROM_END




ROM_START( m4blkwhd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dbw11.bin", 0x0000, 0x010000, CRC(337aaa2c) SHA1(26b12ea3ada9668293c6b44d62458590e5b4ac8f) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "bwsnd.bin", 0x0000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
ROM_END



ROM_START( m4blkbul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cbb08.epr", 0x0000, 0x010000, CRC(09376df6) SHA1(ba3b101accb6bbfbf75b9d22621dbda4efcb7769) )
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


ROM_START( m4brktak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "breakntake p1.bin", 0xc000, 0x004000, CRC(a3457409) SHA1(ceaca37f20a055b18a24ee99e43991df95e9b520) )
	ROM_LOAD( "breakntake p2.bin", 0x8000, 0x004000, CRC(7465cc6f) SHA1(f984e41c310bc58d7a668ec9f31c238fbf5de9c6) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "b-t v1-0 p1", 0xc000, 0x004000, CRC(a3457409) SHA1(ceaca37f20a055b18a24ee99e43991df95e9b520) )
	ROM_LOAD( "b-t v1-0 p2", 0x8000, 0x004000, CRC(7465cc6f) SHA1(f984e41c310bc58d7a668ec9f31c238fbf5de9c6) )
ROM_END


ROM_START( m4brook )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "brkl10.epr", 0x0000, 0x010000, CRC(857255b3) SHA1(cfd77918a19b2532a02b8bb3fa8e2716db31fb0e) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "brkl_snd.epr", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END



ROM_START( m4bucks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bufd.p1", 0x0000, 0x010000, CRC(02c575d3) SHA1(92dc7a0c298e4d2d19bf754a5c82cc15e4e6456c) )
	ROM_LOAD( "bufs.p1", 0x0000, 0x010000, CRC(e394ae40) SHA1(911077053c47cebba1bed9d359cd38bd676a46f1) )
ROM_END

ROM_START( m4calamab )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "bc302d.p1", 0x0000, 0x020000, CRC(36b87f8e) SHA1(6e3cbfa52d9ec52fe009d3331dda3781f7f7783a) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "bc302f.p1", 0x0000, 0x020000, CRC(4b356aca) SHA1(81ce1585f529f1717ec56ace0a4902ae901593ae) )
	ROM_LOAD( "bc302s.p1", 0x0000, 0x020000, CRC(b349bd2d) SHA1(9b026bece40584c4f53c30f3dacc91942c871a9f) )
	ROM_LOAD( "calamari.cl", 0x0000, 0x020000, CRC(bb5e81ac) SHA1(b27f71321978712d2950d58715d18fd5523d6b06) )
ROM_END

ROM_START( m4calama )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cac03s.p1", 0x0000, 0x020000, CRC(edc97795) SHA1(58fb91809c7f475fbceacfc1c3bda41b86dff54b) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ca301d.p1", 0x0000, 0x020000, CRC(9a220126) SHA1(d5b12955bb336f8233ed3f892e23a14ba755a511) )
	ROM_LOAD( "ca301f.p1", 0x0000, 0x020000, CRC(e7af1462) SHA1(72659ef85c3b7916e10b4dbc09ad62638e7ab7e1) )
	ROM_LOAD( "ca301s.p1", 0x0000, 0x020000, CRC(95beecf1) SHA1(70f72abc0d4280618033b61f9dbe5b90b455c2b1) )
	ROM_LOAD( "cac03d.p1", 0x0000, 0x020000, CRC(14436ec7) SHA1(eb654ef5cef94e24296512acb6134440a5f8d17e) )
	ROM_LOAD( "cac03f.p1", 0x0000, 0x020000, CRC(69ce7b83) SHA1(c1f2dea6fe7983f5cefbf58ad63bce5ae8d7f7a5) )

	ROM_REGION( 0x20000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "bca04.p1", 0x0000, 0x020000, CRC(3f97fe65) SHA1(6bc2c7e60658f39701974426ab652e8dd96b1913) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "m407.chr", 0x0000, 0x000048, CRC(fa693a0d) SHA1(601afba4a6efe8334ecc2cadfee99273a9818c1c) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cala1.hex", 0x0000, 0x080000, CRC(c9768f65) SHA1(a8f2946fdba640033da0e21d4e18293b3fc004bf) )
	ROM_LOAD( "cala2.hex", 0x0000, 0x080000, CRC(56bd2950) SHA1(b109c726514c3ee04c1bbdf5f518f60dfd0375a8) )
ROM_END

ROM_START( m4calicl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ca2s.p1", 0x0000, 0x010000, CRC(fad153fd) SHA1(bd1f1a5c73624df45d01cb4853d87e998e434d7a) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ca2d.p1", 0x0000, 0x010000, CRC(75eb8c6f) SHA1(1bb923d06dcfa24eaf9533c083f68f4bd840834f) )
	ROM_LOAD( "ca2f.p1", 0x0000, 0x010000, CRC(6c53cf29) SHA1(2e58453891ab4faa17ef58a81c5f3c0618d046a5) )
	ROM_LOAD( "cald.p1", 0x0000, 0x010000, CRC(296fdeeb) SHA1(7782c0c7d8f44e2c0d48cc24c13015241e47b9ec) )
	ROM_LOAD( "cals.p1", 0x0000, 0x010000, CRC(28a1c5fe) SHA1(e8474df609ea7f3517780b54d6f493987aad3650) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ca2s.chr", 0x0000, 0x000048, CRC(97618d38) SHA1(7958e99684d50b9bdb56c97f7fcfe161f0824578) )
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

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "m470.chr", 0x0000, 0x000048, CRC(10d302d4) SHA1(5858e550470a25dcd64efe004c79e6e9783bce07) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdot01.bin", 0x0000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
ROM_END

ROM_START( m4casot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casrom.bin",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e) ) // == old timer   (aka b&wrom.bin)

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "caschar.chr", 0x0000, 0x000048, CRC(10d302d4) SHA1(5858e550470a25dcd64efe004c79e6e9783bce07) ) // ( aka b&wchrt.chr )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cassound.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) ) // ( aka b&wsound.bin )
ROM_END



ROM_START( m4jpmcla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jcv2.epr",  0x00000, 0x10000,  CRC(da095666) SHA1(bc7654dc9da1f830a43f925db8079f27e18bb61e) ) // == old timer

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "jcchr.chr", 0x0000, 0x000048, CRC(e370e271) SHA1(2b712dd3590c31356e8b0b62ffc64ff8ce444f73) )

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


ROM_START( m4chasei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci2c.p1", 0x0000, 0x010000, CRC(fc49a2e1) SHA1(f4f02e168cd9bf0245c2b7340fe151da66f09c5c) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ch20p8pn.rom", 0x0000, 0x010000, CRC(712bd2e7) SHA1(0e83fa077f42a051aaa07a7e13196955b0ac840d) )
	ROM_LOAD( "chin2010l", 0x0000, 0x010000, CRC(7fe97181) SHA1(1ccf65ff108bdaa46efcb3f831fccc953297b9ac) )
	ROM_LOAD( "ci2k.p1", 0x0000, 0x010000, CRC(8d715b8a) SHA1(5dd6f8d3d6710b0741df37af8792d942f41062d2) )
	ROM_LOAD( "ci2s.p1", 0x0000, 0x010000, CRC(8175e1e3) SHA1(9a4b0a0288508e7900ceac8bc3b245ac1f898b19) )
	ROM_LOAD( "ci2y.p1", 0x0000, 0x010000, CRC(80410946) SHA1(60a4f73eb9a35e5c246d8ef7b25bcf25b28bf8ed) )

	ROM_LOAD( "chase invaders 8.bin", 0x0000, 0x010000, CRC(0bf6a8a0) SHA1(cea5ea40d71484a455615e14f6708b1bc06bbbe8) ) // bad prg (no vectors?)


	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "chaseinvaders.chr", 0x0000, 0x000048, CRC(d7703dcd) SHA1(16fd998d1b44f35c10e5486882aa7f2d018dc82b) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cha.s1", 0x000000, 0x080000, CRC(8200b6bc) SHA1(bcc4ffbddcdcc1dd994fe29e9b24e83272f59442) )
	ROM_LOAD( "cha.s2", 0x080000, 0x080000, CRC(542863fa) SHA1(501d66b2badb5036bb5dd8bac3cdb681f630a982) )
ROM_END



ROM_START( m4c9c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cncs.p1", 0x0000, 0x010000, CRC(10f15e2a) SHA1(c17ab13764d74302246984245485cb7692913b44) )
ROM_END






ROM_START( m4clbshf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "csss.p1", 0x0000, 0x010000, CRC(32dd9b96) SHA1(93831858b2f0ada8e4a0aa2fae59d12c53287df1) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "club_shuffle.chr", 0x0000, 0x000048, CRC(97618d38) SHA1(7958e99684d50b9bdb56c97f7fcfe161f0824578) )
ROM_END

ROM_START( m4clbtro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr2f.p1", 0x0000, 0x010000, CRC(fbdcd06f) SHA1(27ccdc83e60a62227d33d8cf3d516fc43908ab99) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "tr2d.p1", 0x0000, 0x010000, CRC(0cc23f89) SHA1(a66c8c28073f53381c43e3e597f15f81c5c61479) )
	ROM_LOAD( "tr2s.p1", 0x0000, 0x010000, CRC(6d43375c) SHA1(5be1dc85374c6a1235e0b137b46ebd7a2d7d922a) )
	ROM_LOAD( "tro20.bin", 0x0000, 0x010000, CRC(5e86c3fc) SHA1(ce2419991559839a8875060c1afe0f030190010a) )
	ROM_LOAD( "trod.p1", 0x0000, 0x010000, CRC(60c84612) SHA1(84dc8b34e41436331832c1a32ddac0fce269488a) )
	ROM_LOAD( "tros.p1", 0x0000, 0x010000, CRC(5e86c3fc) SHA1(ce2419991559839a8875060c1afe0f030190010a) )
ROM_END

ROM_START( m4clbveg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clad.p1", 0x0000, 0x010000, CRC(4fa45cce) SHA1(58a5d6cc8608eb1aa453429e26eacea589afa524) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "claf.p1", 0x0000, 0x010000, CRC(79b83184) SHA1(7319a405b2b0b274e03f5cd1465436f8548065e4) )
	ROM_LOAD( "clal.p1", 0x0000, 0x010000, CRC(db0bb5a2) SHA1(2735e02642fb92bb824e3b1f415a1a3ef13a856d) )
	ROM_LOAD( "clas.p1", 0x0000, 0x010000, CRC(6aad03f0) SHA1(2f611cc6f020e334dc4b87d2d907727ba15ff7ff) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "cvegas.chr", 0x0000, 0x000048, CRC(a6c341b0) SHA1(c8c838c9bb1ced52889504b9cea8d88f1e7fa79f) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cvegass1.hex", 0x0000, 0x080000, CRC(13a8c857) SHA1(c66e10bca1ad54f467b9c5eacd502c54397c09b2) )
	ROM_LOAD( "cvegass2.hex", 0x0000, 0x080000, CRC(88b37145) SHA1(1c6c9ad2010e1688d3370d1f2a5ae83dc683b500) )
ROM_END

ROM_START( m4clbx )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "clx12s.p1", 0x0000, 0x020000, CRC(6798c153) SHA1(e621e341a0fed1cb35637edb0769ae1cca72a663) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "clx12d.p1", 0x0000, 0x020000, CRC(43e797ba) SHA1(fb2fc843176fe50c1039214d48815d6e9871ae27) )
	ROM_LOAD( "clx12f.p1", 0x0000, 0x020000, CRC(3e6a82fe) SHA1(01ef9a15a3cf9b1191c573b36fb5758e79c3adc1) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cxs1.hex", 0x000000, 0x080000, CRC(4ce005f1) SHA1(ee0f59a9c7e0222dd63fa63ccff8f194abd01ddb) )
	ROM_LOAD( "cxs2.hex", 0x080000, 0x080000, CRC(495e0730) SHA1(7ba8150fbcf974ac494a82fd373ff02185543e35) )
ROM_END



ROM_START( m4coscas )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cc30s.p1", 0x0000, 0x020000, CRC(e308100a) SHA1(14cb07895d17237768877dd62ba7c3fc8e5b2630) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cosm15g", 0x0000, 0x020000, CRC(edd01d55) SHA1(49246fa1e12ceb3297f35616cdc1cf62472a379f) )
	ROM_LOAD( "cosmiccasinos15.bin", 0x0000, 0x020000, CRC(ddba1241) SHA1(7ca2928ae2ab4e323b60bb661b60681f89cc5663) )

	ROM_REGION( 0x20000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cc_sj___.3s1", 0x0000, 0x020000, CRC(52c312b0) SHA1(bd5381d58b1acb7adf6857c142eae4a253081fbd) )
	ROM_LOAD( "cc_sj__c.3r1", 0x0000, 0x020000, CRC(44b940a6) SHA1(7e621873fcf6460f654e35cc74552e86b6253ddb) )
	ROM_LOAD( "cc_sj__c.7_1", 0x0000, 0x020000, CRC(ee9e6126) SHA1(fab6fd04004acebf291544720ba06cea79d5a054) )
	ROM_LOAD( "cc_sj_b_.3s1", 0x0000, 0x020000, CRC(019f0a71) SHA1(7a97f4e89c16e25f8e7502bba37f49c8496fbb47) )
	ROM_LOAD( "cc_sj_bc.3r1", 0x0000, 0x020000, CRC(de9bb8e1) SHA1(7974b03974531eb4b5ed865b8eeb9649c1346df4) )
	ROM_LOAD( "cc_sj_bc.7_1", 0x0000, 0x020000, CRC(afe1aac6) SHA1(fc9c69e45db6a85c45ef8d32d048e5726d7da655) )
	ROM_LOAD( "cc_sj_d_.3s1", 0x0000, 0x020000, CRC(215e12f3) SHA1(68ed9923c6fd51e9305afac9d271c7b3ce38b12f) )
	ROM_LOAD( "cc_sj_dc.3r1", 0x0000, 0x020000, CRC(00e357c3) SHA1(02bf7427899d2e536442b87d41c140ebd787a580) )
	ROM_LOAD( "cc_sj_dc.7_1", 0x0000, 0x020000, CRC(330d68a2) SHA1(12410af5f37b26f29f5cd23606ab0e128675095a) )
	ROM_LOAD( "cc_sj_k_.3s1", 0x0000, 0x020000, CRC(9161912d) SHA1(d11109f4bdc1c60f4cf477e1f26556800a83abdb) )
	ROM_LOAD( "cc_sj_kc.3r1", 0x0000, 0x020000, CRC(b0dcd41d) SHA1(6b50a5e401bf854186331673dcc0c3fc5de2991b) )
	ROM_LOAD( "cc_sja__.3s1", 0x0000, 0x020000, CRC(1682b1d3) SHA1(24baaf789eca150f0f6fd9c510e245aa7b88cc4c) )
	ROM_LOAD( "cc_sja_c.3r1", 0x0000, 0x020000, CRC(373ff4e3) SHA1(55b7ab247863eb3c025e84782c8cab7734343077) )
	ROM_LOAD( "cc_sja_c.7_1", 0x0000, 0x020000, CRC(e956898e) SHA1(f51682651520551d481360bf86eba510cd758441) )
	ROM_LOAD( "cc_sjb__.3s1", 0x0000, 0x020000, CRC(5c451985) SHA1(517f634d31f7190ca6685c1037fb66a8b87effba) )
	ROM_LOAD( "cc_sjb_c.7_1", 0x0000, 0x020000, CRC(109e9ae9) SHA1(00f381beb33cae58fc3429d3501efa4a9d9f0035) )
	ROM_LOAD( "cc_sjbgc.3r1", 0x0000, 0x020000, CRC(2de82f88) SHA1(5c8029d43282a014e82b4f975616ed2bbc0e5641) )
	ROM_LOAD( "cc_sjbtc.3r1", 0x0000, 0x020000, CRC(976c2858) SHA1(a70a8fe51d1b9d903d099e89a40481ea6af13683) )
	ROM_LOAD( "cc_sjwb_.3s1", 0x0000, 0x020000, CRC(e2df8167) SHA1(c312b30402dd93c6d4a32932677430c9c996fd36) )
	ROM_LOAD( "cc_sjwbc.3r1", 0x0000, 0x020000, CRC(a33a59a6) SHA1(a74ffd647e8390d89df475cc3f5205462c9d93d7) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cc___snd.1_1", 0x000000, 0x080000, CRC(d858f238) SHA1(92a3dfacde8bfa8705e91fab5bb627f9b34ad2dc) )
	ROM_LOAD( "cc___snd.1_2", 0x080000, 0x080000, CRC(bab1bd8e) SHA1(c703d0e24c0a522ebf79895049e85f5471f7d7e9) )
ROM_END



ROM_START( m4crzjk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crjok2.04.bin", 0x0000, 0x010000, CRC(838336d6) SHA1(6f36de20c930cbbff479af2667c11152c6adb43e) )
ROM_END

ROM_START( m4crzjwl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cj11bin", 0x0000, 0x020000, CRC(208fda73) SHA1(8b15c197693ea7749bc961fe4e5e36b317f9f6f8) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cjew_v1-0k", 0x0000, 0x020000, CRC(4434902d) SHA1(31c7be1235cdfd00099d1e09644a0f76fc7a26f7) )
	ROM_LOAD( "cjexlow", 0x0000, 0x020000, CRC(07c227c1) SHA1(286341ed44ef7cd08ca411f2b3e6936b5e83a5f3) )
	ROM_LOAD( "cjgerman", 0x0000, 0x020000, CRC(b090e690) SHA1(bdbe4041085c995761306280c15f782ea3bdc110) )
	ROM_LOAD( "cjj54.bin", 0x0000, 0x020000, CRC(16dc92e7) SHA1(b791535054d5864c7053243408a54accfa014bd1) )
	ROM_LOAD( "gcn11", 0x0000, 0x020000, CRC(51493500) SHA1(901e60c1a7e9e628d723e199579fc82cf2e433e6) )
	ROM_LOAD( "gcn111", 0x0000, 0x020000, CRC(b1152ce6) SHA1(1d236bad57ad38b11215efe44008bb8e4014939e) )
	ROM_LOAD( "gjv4", 0x0000, 0x020000, CRC(df63105d) SHA1(56e28adef9ec8921da7ab8045859e834731196c5) )
	ROM_LOAD( "gjv5", 0x0000, 0x020000, CRC(e4f0bab2) SHA1(1a13d97ff2c4fbae39327f2a5a8b110f2617857e) )
	// Crown Jewels Deluxe?
	ROM_LOAD( "cjg.p1", 0x0000, 0x020000, CRC(1f4743bf) SHA1(f9a0da2ed9cad5e6685c8a6d1d09e5d4bbcfacec) )


	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cjsound1.bin", 0x000000, 0x080000, CRC(b023f6b9) SHA1(04c362c6511442d3ab775a5ff2051bfe26d5e624) )
	ROM_LOAD( "cjsound2.bin", 0x080000, 0x080000, CRC(02563a43) SHA1(dfcee4e0fdf81c726c8e13278e7950459bcaab18) )
	ROM_LOAD( "cjsound3.bin", 0x100000, 0x080000, CRC(e722e438) SHA1(070f3772920fa64d5214843c313b27a5b2a4c105) )

	ROM_LOAD( "cjew_sound1", 0x000000, 0x080000, CRC(a2f20c95) SHA1(874b22850732514a26448cee8e0b68f8d042a7c7) )
	ROM_LOAD( "cjew_sound2", 0x080000, 0x080000, CRC(3dcb7c38) SHA1(3c0e91f4d2ea9e6b25a01702c6f6fdc7cc2e0b65) )
ROM_END

ROM_START( m4crjwl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cjcf.p1", 0x0000, 0x010000, CRC(7feccc74) SHA1(4d1c7c6d2085492ee4205a7383ad7dc1de4e8d60) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cjcd.p1", 0x0000, 0x010000, CRC(cb83f226) SHA1(f09996436b3db3c8f0fe237884d9125be2b7855e) )
	ROM_LOAD( "cjcs.p1", 0x0000, 0x010000, CRC(1054e02d) SHA1(067705f20862f6cfc4334c74e0fab1a1016d427c) )
	ROM_LOAD( "cjn02.p1", 0x0000, 0x010000, CRC(a3d50e20) SHA1(15698e74a37d5f95a5634d48ae2a9a5d19faa2b6) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? or in above set? */
ROM_END

ROM_START( m4crjwl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cj214f.p1", 0x0000, 0x010000, CRC(7ee4d30c) SHA1(2bf702bc925c473f7e9eaeb5b3ae0b00e124161a) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cj214d.p1", 0x0000, 0x010000, CRC(359e2a73) SHA1(c85eeebafca14e6f975953f5daf2772a62693051) )
	ROM_LOAD( "cj214s.hex", 0x0000, 0x010000, CRC(296aa885) SHA1(045b02848b37e8a04d950d54301dc6888d6178ad) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "chr.chr", 0x0000, 0x000048, CRC(c5812913) SHA1(d167b1f512c183cf01a1f4e1c1588ea0ae21331b) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cjcs1.hex", 0x000000, 0x080000, CRC(2ac3ba9f) SHA1(3332f29f81918c34aeec3da6f7d001dc9922840d) )
	ROM_LOAD( "cjcs2.hex", 0x080000, 0x080000, CRC(89838a9d) SHA1(502243cc0a14e63882b537f05c4cc0eb852e4a0c) )
ROM_END

ROM_START( m4crdome )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cd212k.p1", 0x0000, 0x020000, CRC(673b10a1) SHA1(996ade8193f448970beea2c5b81d9f27c05f162f) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cd212c.p1", 0x0000, 0x020000, CRC(1ab605e5) SHA1(03327b2fac9d3d2891dc5950aa89ac4947c7b444) )
	ROM_LOAD( "cd212ad.p1", 0x0000, 0x020000, CRC(c76cab39) SHA1(abbe5d629929ff89b499cd4d0e15e9fa13fc33de) )
	ROM_LOAD( "cd212b.p1", 0x0000, 0x020000, CRC(2dfcb8f7) SHA1(ba711fb20556c447f4bb3a11fc1cc6a3599bfd6d) )
	ROM_LOAD( "cd212bd.p1", 0x0000, 0x020000, CRC(4a7605fc) SHA1(2a8a327d6dce8f7be1938d5a3854594adf5c092c) )
	ROM_LOAD( "cd212d.p1", 0x0000, 0x020000, CRC(50e7b381) SHA1(58141910fbbc2624da737a2ba273b79a6da695d3) )
	ROM_LOAD( "cd212dk.p1", 0x0000, 0x020000, CRC(00b1adaa) SHA1(ba089bf86fcc8817ae756ed5609aaf876d1ee5a5) )
	ROM_LOAD( "cd212dr.p1", 0x0000, 0x020000, CRC(b3be169b) SHA1(eb4699fdce371d94feec410c640bd49bfdccba98) )
	ROM_LOAD( "cd212dy.p1", 0x0000, 0x020000, CRC(87528d54) SHA1(974fa2c29af43c903add28dca0ea3b04f612d2f7) )
	ROM_LOAD( "cd212r.p1", 0x0000, 0x020000, CRC(d434ab90) SHA1(d42258bd965e8a028a418681a1307234c9b1c450) )
	ROM_LOAD( "cd212s.p1", 0x0000, 0x020000, CRC(f7d9d5e3) SHA1(1378e28c0a2c59a42a440502f20cc011625f43b5) )
	ROM_LOAD( "cd212y.p1", 0x0000, 0x020000, CRC(e0d8305f) SHA1(ddf1125eba0e470f6ae811fe050d4000300cfd0c) )
	ROM_LOAD( "cdom15r", 0x0000, 0x020000, CRC(28f9ee8e) SHA1(e3484933dd0b8ddc2eeefc4dc95ce5379565e750) )
	ROM_LOAD( "cdome10", 0x0000, 0x020000, CRC(945c9277) SHA1(6afee54b332152f6767781a040799d865999b292) )
	ROM_LOAD( "cdome8ac", 0x0000, 0x020000, CRC(0553bfe6) SHA1(77abfa556f04dca1be52fbed357807e6ada10458) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "tri98.chr", 0x0000, 0x000048, CRC(8a4532a8) SHA1(c128fd513bbcba68a1c75a11e09a54ba1d23d6f4) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cd2snd.p1", 0x000000, 0x080000, CRC(65a2dc92) SHA1(2c55a858ab17325189bed1974daf708c380541de) )
	ROM_LOAD( "cd2snd.p2", 0x080000, 0x080000, CRC(b1bb4678) SHA1(8e8ab0a8d1b3e70dcb56d071193fdb5f34af7d14) )
ROM_END



ROM_START( m4dbl9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d9s_6.bin", 0x0000, 0x010000, CRC(6029d46a) SHA1(0823f29f17562675a6f250429e46655c0b2e8f2c) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "du91.0", 0x0000, 0x010000, CRC(6207753d) SHA1(b19bcb60707b73f37e9bd8177d0b15847af0213f) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "du91.chr", 0x0000, 0x000048, CRC(9724122d) SHA1(a41687eec84cad453c1a2a89317078f48ca0895f) )
ROM_END

ROM_START( m4dbldmn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cdd01.p1", 0x0000, 0x020000, CRC(e35dffde) SHA1(0bfc977f25f25785f20b510c44d2d3d79e23af8b) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cdd05d.p1", 0x0000, 0x020000, CRC(fc1c5e90) SHA1(c756d2ac725168af5396c8ef7550db9087a50937) )
	ROM_LOAD( "cdd05f.p1", 0x0000, 0x020000, CRC(81914bd4) SHA1(cf286810ad6732ca1d706e70f4c2958d28cc979c) )
	ROM_LOAD( "cdd05s.p1", 0x0000, 0x020000, CRC(fc14771f) SHA1(f418af9fed331560195a694f20ef2fea27ed04b0) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cddsnd.p1", 0x000000, 0x080000, CRC(e1833e31) SHA1(1486e5afab347d6dee1543a55d1193b7db3c89d7) )
	ROM_LOAD( "cddsnd.p2", 0x080000, 0x080000, CRC(fd33ed2a) SHA1(f68ffadde40f88e7954d4a98bcd7ff023841b55b) )
ROM_END



ROM_START( m4drac )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dra21.bin", 0x0000, 0x020000, CRC(23be387e) SHA1(08a78f4b8ddef46069d1c75113300b21e52338c1) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "dra24.bin", 0x0000, 0x020000, CRC(3db112ae) SHA1(b5303e2a65476931d4769327ca62afd0f6a9eda7) )
	ROM_LOAD( "dra27.bin", 0x0000, 0x020000, CRC(8a095175) SHA1(41006e298f1688499ce6820ec28196c7578684b9) )
	ROM_LOAD( "dra27.p1", 0x0000, 0x020000, CRC(8a095175) SHA1(41006e298f1688499ce6820ec28196c7578684b9) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "drasnd.p1", 0x000000, 0x080000, CRC(54c3821c) SHA1(1fcc62e2b127dd7f1d5d27a3afdf56dc27f122f8) )
	ROM_LOAD( "drasnd.p2", 0x080000, 0x080000, CRC(9096d2bc) SHA1(1b4c530b7b0fde869980d519255e2585c5461e13) )
	ROM_LOAD( "drasnd.p3", 0x100000, 0x080000, CRC(a07f412b) SHA1(cca8f5cfe620ece45ca40bf801f0643cd76547e9) )
	ROM_LOAD( "drasnd.p4", 0x180000, 0x080000, CRC(018ed789) SHA1(64202da2c542f5ef208faeb04945eb1a758d4746) )
ROM_END






ROM_START( m4exgam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "czep30.bin", 0x0000, 0x010000, CRC(4614e6f6) SHA1(5602a68e9b47394cb31bbcd49a9920e19af6242f) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ceg.chr", 0x0000, 0x000048, CRC(f694224e) SHA1(936ab5e349fa59accbb37959cce9519fd97f3978) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sczep.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END

ROM_START( m4fastfw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fastf206", 0x0000, 0x010000, CRC(a830b121) SHA1(0bf813ee75bd8e109e6688b91bd0983d341a6695) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ffo05__1.0", 0x0000, 0x010000, CRC(8b683969) SHA1(7469b551e4d6f65550d54ee39b2bac07cf3dbd4b) )
	ROM_LOAD( "ffo10__1.0", 0x0000, 0x010000, CRC(294288fd) SHA1(87d25f6333b6862fcc57a550b5cc7c0bc64e72cd) )
	ROM_LOAD( "ffo10d_1.0", 0x0000, 0x010000, CRC(8d96f3d4) SHA1(2070a335cfa3f9de1bd9e9094d91cce81b91347d) )
	ROM_LOAD( "ffo20__1.0", 0x0000, 0x010000, CRC(9528291e) SHA1(61c0eb8ce955f708e8a68a28f253706267e28254) )
	ROM_LOAD( "ffo20d_1.0", 0x0000, 0x010000, CRC(5bae35fe) SHA1(7e4d61ed97ddd170bd1424f34d0327093668da3f) )
	ROM_LOAD( "ffo20dy1.0", 0x0000, 0x010000, CRC(37167d46) SHA1(94b87697615f81b746ce3bcc64fc893f865e00dc) )
ROM_END




ROM_START( m4fortcb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cfod.p1", 0x0000, 0x010000, CRC(9d0e2b63) SHA1(cce871d2bbe486793de5de9fadfbddf67c382e5c) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cfof.p1", 0x0000, 0x010000, CRC(010b3c1f) SHA1(b44c22c21d22603b277138eabf803e6d46ad4aae) )
	ROM_LOAD( "cfos.p1", 0x0000, 0x010000, CRC(f3b47df4) SHA1(3ad674864ba3a24283af14caaf2c999d4fde11fc) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "cfosnd.p1", 0x000000, 0x080000, CRC(74bbf913) SHA1(52ddc89ab34b11ede2c0e9b9b27e119b0c1eb2d9) )
	ROM_LOAD( "cfosnd.p2", 0x080000, 0x080000, CRC(1b2bb79a) SHA1(5f19ea000f34bb404ed6c8ea5ec7b809ccb1ae36) )
ROM_END


ROM_START( m4frtlt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pfr03i.p1", 0x0000, 0x010000, CRC(bf13d4b5) SHA1(4de2b5c55a0022c97804233bc5c6b4fc8ee05c24) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "pfr03o.p1", 0x0000, 0x010000, CRC(89a918e2) SHA1(afc2f80ea7539b68dc6bfd040e5717b607d22284) )
	ROM_LOAD( "pfr03s.p1", 0x0000, 0x010000, CRC(0ea80adb) SHA1(948a23fe8ccf6f423957a478a57bb875cc7b2cc2) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "pfrsnd.p1", 0x0000, 0x080000, CRC(71d1af20) SHA1(d87d61c561acbe9cb3dec18d8decf5e970efa272) )
ROM_END



ROM_START( m4frtgm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruit.bin", 0x0000, 0x010000, CRC(dbe44316) SHA1(15cd49dd2e6166f7a7668663f7fea802d6cbb12f) )

	ROM_REGION( 0x800000, "msm6376", 0 ) /* this isn't OKI, or is corrupt (bad size) */
	ROM_LOAD( "fruitsnd.bin", 0x0000, 0x010000, CRC(86547dc7) SHA1(4bf64f22e84c0ee82d961b0ba64932b8bf6a521f) ) // matches 'Replay' on SC1 hardware, probably just belongs there.. or this is eurocoin with different sound hw here?
ROM_END






ROM_START( m4gb006 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "006s.p1", 0x0000, 0x010000, CRC(6e750ab9) SHA1(2e1f08df7991efe450633e0bcec201e6fa7fdbaa) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "006d.p1", 0x0000, 0x010000, CRC(7e0a4282) SHA1(8fd0cbdd9cf3ac74b7b202ce7615392c1a746906) )
	ROM_LOAD( "006y.p1", 0x0000, 0x010000, CRC(2947f4ed) SHA1(7d212bcef36e2bd792ded3e1e1638218e76da119) )
	ROM_LOAD( "bond20_11", 0x0000, 0x010000, CRC(8d810cb1) SHA1(065d8df33472a3476dd6cf21a684db9d7c8ba829) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "006s.chr", 0x0000, 0x000048, CRC(ee3d06eb) SHA1(570a715e71d4184e4df02b7e5b68fee70e03aeb0) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "006snd.p1", 0x000000, 0x080000, CRC(44afef7d) SHA1(d8a4b6dc04e0f337db6d3b5322d066ae5f5bda41) )
	ROM_LOAD( "006snd.p2", 0x080000, 0x080000, CRC(5f3c7cf8) SHA1(500f8fb07ef344d44c062f8d01878df1c917bcfc) )
ROM_END

ROM_START( m4gbust )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ghostbusters 2p.bin", 0x0000, 0x010000, CRC(abb288c4) SHA1(2012e027711996a552ab59674ae3bce1bf14f44b) )

	ROM_REGION( 0x10000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "gb_02___.2n3", 0x0000, 0x010000, CRC(973b3538) SHA1(31df04d9f35cbde4d5e395256927f146d1613178) )
	ROM_LOAD( "gb_02___.3a3", 0x0000, 0x010000, CRC(2b9d94b6) SHA1(ca433240f9e926cdf5240209589951e6018a496a) )
	ROM_LOAD( "gb_02___.3n3", 0x0000, 0x010000, CRC(99514ddd) SHA1(432d484525867c6ad68cd93a4bfded4dba36cf56) )
	ROM_LOAD( "gb_02___.3s3", 0x0000, 0x010000, CRC(2634aa5f) SHA1(58ab973940138bdfd2690867e2ac3eb52bffb633) )
	ROM_LOAD( "gb_05___.4a3", 0x0000, 0x010000, CRC(8be6949e) SHA1(9731a1cb0d17c3cec2bec263cd6348f05662d917) )
	ROM_LOAD( "gb_05___.4n3", 0x0000, 0x010000, CRC(621b25f0) SHA1(bf699068284def8bad9143c5841f667f2cb6f20f) )
	ROM_LOAD( "gb_05___.4s3", 0x0000, 0x010000, CRC(e2227701) SHA1(271682c7bf6e0f6f49f6d6b138aa19b6ef6bc626) )
	ROM_LOAD( "gb_05_d_.4a3", 0x0000, 0x010000, CRC(a1b2b32f) SHA1(c1504b3768920f90dbd441b9d50db9676528ca97) )
	ROM_LOAD( "gb_10___.2a3", 0x0000, 0x010000, CRC(a5c692f3) SHA1(8305c88ab8b80b407f4723df25135c25a4c0794f) )
	ROM_LOAD( "gb_10___.2n3", 0x0000, 0x010000, CRC(de18c441) SHA1(5a7055fcd755c1ac58e1b94af243801f169f29f5) )
	ROM_LOAD( "gb_10___.3s3", 0x0000, 0x010000, CRC(427e043b) SHA1(2f64c11a04306692ac5eb9919892f7226156dce0) )
	ROM_LOAD( "gb_10_b_.3s3", 0x0000, 0x010000, CRC(091afb66) SHA1(ac32d7be1e1f4f1453e37017966990a481506024) )
	ROM_LOAD( "gb_10_d_.2a3", 0x0000, 0x010000, CRC(f1446bf5) SHA1(4011d60e13045476741c5a02c64dabbe6a1ae2d6) )
	ROM_LOAD( "gb_10_d_.2n3", 0x0000, 0x010000, CRC(cac5057d) SHA1(afcc21dbd07515ed134675b7dbfb53c048a465b0) )
	ROM_LOAD( "gb_10_d_.3s3", 0x0000, 0x010000, CRC(776736de) SHA1(4f80d9ffdf4468801cf830e9774b6028f7684864) )
	ROM_LOAD( "gb_20___.2n3", 0x0000, 0x010000, CRC(27fc2ee1) SHA1(2e6a042f7117b4594b2601ae166ee0db72c70ed5) )
	ROM_LOAD( "gb_20___.3s3", 0x0000, 0x010000, CRC(4a86d879) SHA1(72e92b6482fdeb4dca36d9426a712ac24d60f7f7) )
	ROM_LOAD( "gb_20_b_.2a3", 0x0000, 0x010000, CRC(4dd7d38f) SHA1(8a71c27189ec3089c016a8292db68f7cdc91b083) )
	ROM_LOAD( "gb_20_b_.2n3", 0x0000, 0x010000, CRC(28cbb217) SHA1(a74978ff5e1511a33f543006b3f8ad30a77ea462) )
	ROM_LOAD( "gb_20_b_.3s3", 0x0000, 0x010000, CRC(1a7cc3cf) SHA1(0d5764d35489bde284965c197b217a06f26a3e3b) )
	ROM_LOAD( "gb_20_d_.2a3", 0x0000, 0x010000, CRC(70f40688) SHA1(ed14f8f460825ffa087394ef5984ae064e02f7b6) )
	ROM_LOAD( "gb_20_d_.2n3", 0x0000, 0x010000, CRC(431c2965) SHA1(eb24e560d5c4bf419465fc760621a4fa853fff95) )
	ROM_LOAD( "gb_20_d_.3s3", 0x0000, 0x010000, CRC(4fc69155) SHA1(09a0f2122893d9fd90204a74c8862e01386503a4) )

ROM_END



ROM_START( m4gldgat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dgg22.bin", 0x0000, 0x010000, CRC(ef8498df) SHA1(6bf164ef18445e83e4510a000bc924cbe916ad99) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "m450.chr", 0x0000, 0x000048, CRC(fb7b2a45) SHA1(b6d5537bde9c05a3e79221a5577b8ae77bace9e6) )
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

ROM_START( m4hirise )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hirs.p1", 0x0000, 0x010000, CRC(a38f771e) SHA1(c1502200671389a1fe6dcb9c043d22583d5991dc) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "hiix.p1", 0x0000, 0x010000, CRC(c68c816c) SHA1(2ec89d83f3b658700433fc165358290ce58eba64) )
	ROM_LOAD( "hirs20dd", 0x0000, 0x010000, CRC(89941670) SHA1(28859adfa79dce53c348c63b46f6f5a068f2b2de) )
	ROM_LOAD( "hirx.p1", 0x0000, 0x010000, CRC(4280a16b) SHA1(c9179ec17404a6f084679ad5f04e53a50f00af98) )
	ROM_LOAD( "hirxc.p1", 0x0000, 0x010000, CRC(1ad1d942) SHA1(91d02212606e22b280be9640433e013bc50e5ea8) )
	ROM_LOAD( "hrise206", 0x0000, 0x010000, CRC(58b4bbdd) SHA1(0b76d27147fbadba97328eb9d2dc81cff9d576e0) )
ROM_END





ROM_START( m4holdtm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dht10.hex", 0x0000, 0x010000, CRC(217d382b) SHA1(a27dd107c554d4787967633dff998d3962ee0ea5) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ht01.chr", 0x0000, 0x000048, CRC(0fc2bb52) SHA1(0d0e47938f6e00166e7352732ddfb7c610f44db2) )
	ROM_LOAD( "m400.chr", 0x0000, 0x000048, CRC(8f00f720) SHA1(ea59fa2a3b016a7ae83be3caf863de87ce7aeffa) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sun01.hex", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END

ROM_START( m4hypclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hpcd.p1", 0x0000, 0x010000, CRC(7fac8944) SHA1(32f0f16ef6c4b99fe70464341a1ce226f6221122) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "hpcf.p1", 0x0000, 0x010000, CRC(2931a558) SHA1(2f7fe541edc502738dd6603435deaef1cb26a1e2) )
	ROM_LOAD( "hpcfd.p1", 0x0000, 0x010000, CRC(b127e577) SHA1(da034086bb92934f73d1a2be776f91462274479d) )
	ROM_LOAD( "hpcs.p1", 0x0000, 0x010000, CRC(55601e10) SHA1(78c3f13cd122e86ff8b7750b375c26e56c6b27c6) )
ROM_END






ROM_START( m4jok300 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cjo", 0x0000, 0x020000, CRC(386e99db) SHA1(5bb0b513ef63ffaedd98b8e9e7206658fe784fda) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASEFF )
	/* missing? */
ROM_END

ROM_START( m4jokmil )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cjm03.epr", 0x0000, 0x020000, CRC(e5e4986e) SHA1(149b950a739ad308f7759927c344de8193ce67c5) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASEFF )
	/* missing? */
ROM_END




ROM_START( m4joljok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jlyjk16l.bin", 0x0000, 0x010000, CRC(a0af938f) SHA1(484e075c3b9199d0d9e20185c6fa0be560845029) )
ROM_END


ROM_START( m4joljokd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "djj15.bin", 0x0000, 0x010000, CRC(155cb134) SHA1(c1026effeceba131df9681afd91ccd6fb43b738a) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdld.bin", 0x0000, 0x080000, CRC(9b035fa6) SHA1(51b7e5bc3abdf4f1beba2347146a91a2b3f4de35) )
ROM_END

ROM_START( m4joljokh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jollyjokerhungarian.bin", 0x0000, 0x010000, CRC(85b6a406) SHA1(e277f9d3b62faead04d65efbc06de7f4a50ae38d) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "jollyjokerhungariansnd.bin", 0x0000, 0x080000, CRC(93460383) SHA1(2b179a1dde09ebdfe8c84641899df7be87d443e5) )
ROM_END


ROM_START( m4joltav )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tavs.p1", 0x0000, 0x010000, CRC(12bdf083) SHA1(c1b73bfd05ae6128d1760083383805fdaf328003) )

	ROM_REGION( 0x20000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "jto20__1.1", 0x0000, 0x010000, CRC(4790c4ec) SHA1(2caab4ccc91158f6b76817e76c1d092ef1a79cd9) )
	ROM_LOAD( "jto20d_1.1", 0x0000, 0x010000, CRC(dff09dfc) SHA1(c13f31f7d96075f7c94ae5e79fc1f9b8ce7e4c80) )
ROM_END



ROM_START( m4lastrp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "uvsk.p1", 0x0000, 0x020000, CRC(7827bfa9) SHA1(720d9793e97f2e11c1c9b18e3b4fa6ec7e29250a) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "lvs", 0x0000, 0x020000, CRC(dcb0dc80) SHA1(6045b332eb4af09f6e0a669ea0b78ef4ac389ac2) )
	ROM_LOAD( "vsgs", 0x0000, 0x020000, CRC(126365e3) SHA1(1e648b7a8cb1ff49e43e2fdc30f482b2b73ed6d7) )
	ROM_LOAD( "vspa20st", 0x0000, 0x010000, CRC(267388eb) SHA1(2621724ebdd5031fc513692ff90989bf3b6115d1) )
	ROM_LOAD( "vstr2010", 0x0000, 0x020000, CRC(126365e3) SHA1(1e648b7a8cb1ff49e43e2fdc30f482b2b73ed6d7) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "lasv.chr", 0x0000, 0x000048, CRC(49ec2385) SHA1(1204c532897acc953867691124fc0b700c7aed47) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "uvssnd.p1", 0x000000, 0x080000, CRC(04a47007) SHA1(cfe1f4aa9d29c784b2034c2daa09b8bd7181562e) )
	ROM_LOAD( "uvssnd.p2", 0x080000, 0x080000, CRC(3b35d824) SHA1(e4007d5d13898ed0f91cd270c75b5df8cc62e003) )
ROM_END




ROM_START( m4lineup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lineup5p1.bin", 0xc000, 0x004000, CRC(9ba9edbd) SHA1(385e01816b5631b6896e85343ae96b3c36f9647a) )
	ROM_LOAD( "lineup5p2.bin", 0x8000, 0x004000, CRC(e9e4dfb0) SHA1(46a0efa84770036366c7a6a33ef1d42c7b2b782b) )
	ROM_LOAD( "lineup5p3.bin", 0x6000, 0x002000, CRC(86623376) SHA1(e29442bfcd401361287852b87673368322e946b5) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "lu2_10p1.bin", 0x0000, 0x004000, CRC(2fb89062) SHA1(55e86de8fd0d36cca9aab8ad5aae7b4f5a62b940) )
	ROM_LOAD( "lu2_10p2.bin", 0x0000, 0x004000, CRC(9d820af2) SHA1(63d27df91f80e47eb8c9685fcd2c3eff902a2ef8) )
	ROM_LOAD( "lu2_10p3.bin", 0x0000, 0x002000, CRC(8c8a210c) SHA1(2599d979f1a62e9ef6acc70d0ad5c9b4a65d712a) )
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

ROM_START( m4lucksc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "clu14d.p1", 0x0000, 0x020000, CRC(7a64199f) SHA1(62c7c8a4475a8005a1f969550d0717c9cc44bada) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "clu14f.p1", 0x0000, 0x020000, CRC(07e90cdb) SHA1(5d4bf7f6f84f2890a0119de898f01e3e99bfbb7f) )
	ROM_LOAD( "clu14s.p1", 0x0000, 0x020000, CRC(5f66d7cc) SHA1(bd8a832739d7aef4d04b89a94dd2886e89a6e0c2) )
	ROM_LOAD( "gls06d.p1", 0x0000, 0x020000, CRC(2f7f8a9a) SHA1(04243f190597e3d3bdd258b8146b71e9c7cd90c7) )
	ROM_LOAD( "gls06f.p1", 0x0000, 0x020000, CRC(52f29fde) SHA1(97df15c89540d8bbb15abb86f6a2e9d0e022c9df) )
	ROM_LOAD( "gls06s.p1", 0x0000, 0x020000, CRC(975adb8d) SHA1(b92d1ad93e51f55111921060939359471c2e5384) )
	ROM_LOAD( "gs301d.p1", 0x0000, 0x020000, CRC(e0807af7) SHA1(1740d4d56ad71407a4d2bb13b43c9d5f31caf638) )
	ROM_LOAD( "gs301f.p1", 0x0000, 0x020000, CRC(9d0d6fb3) SHA1(9206299604190deace09136ca2eebb9ad2792815) )
	ROM_LOAD( "gs301s.p1", 0x0000, 0x020000, CRC(53314dc6) SHA1(28b5d2a03b8f6221b80f10c46985fa906cc9be32) )
	ROM_LOAD( "ls301d.p1", 0x0000, 0x020000, CRC(39fb0ddf) SHA1(3a6934892585bde6a99f1d2e2fd95677cf37fcfe) )
	ROM_LOAD( "ls301f.p1", 0x0000, 0x020000, CRC(4476189b) SHA1(b94c6abbbf37ae28869b1f9c882de8fa56b2c676) )
	ROM_LOAD( "ls301s.p1", 0x0000, 0x020000, CRC(7e9e97f1) SHA1(43760792b529db8acb497d38ad3951abdebcf76b) )

	ROM_REGION( 0x20000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "lsc_.1_1", 0x0000, 0x020000, CRC(79ce3db0) SHA1(409e9d3b08284dee3af696fb7c839c0ca35eddee) )


	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "clu14s.chr", 0x0000, 0x000048, CRC(be933239) SHA1(52dbcbbcbfe25b6f8c186ce9af67b533c8da9a88) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "clusnd.p1", 0x000000, 0x080000, CRC(9c1042ba) SHA1(e4630bbcb3fe2f7d133275892eaf58c12402c610) )
	ROM_LOAD( "clusnd.p2", 0x080000, 0x080000, CRC(b4b28b80) SHA1(a40b6801740d64e54c5c1738d69737ab9f4cf950) )
ROM_END

ROM_START( m4luckwb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lwb10.bin", 0x0000, 0x010000, CRC(6d43a14e) SHA1(267aba1a01bfd5f0eaa7683d041d5fcb2d301934) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "lwb15.bin", 0x0000, 0x010000, CRC(b5af8cb2) SHA1(474975b83803627ad3ac4217d8cecb2d2db16fec) )
	ROM_LOAD( "lwb21.bin", 0x0000, 0x010000, CRC(6c570733) SHA1(7488318ca9689371e4f80be0a0fddd8ad141733e) )
	ROM_LOAD( "lwb22.bin", 0x0000, 0x010000, CRC(05b952a7) SHA1(952e328b280a18c1ffe253b6a56f2b5e893b1b72) )
	ROM_LOAD( "lwb27.bin", 0x0000, 0x010000, CRC(9d6b6637) SHA1(65bad12cd08de128ca31c9488e32e3cebfb8eedb) )
	ROM_LOAD( "lwb6.bin", 0x0000, 0x010000, CRC(8e7d4594) SHA1(4824a9a4628585a170c41e00f7b3fcb8a2330c02) )
	ROM_LOAD( "lwb7.bin", 0x0000, 0x010000, CRC(8e651705) SHA1(bd4d09d586d14759a17d4d7d4016c427f3eef015) )

	ROM_REGION( 0x100000, "msm6376", 0 ) // these are all different sound roms...
	ROM_LOAD( "lwbs3.bin", 0x0000, 0x07dc89, CRC(ee102376) SHA1(3fed581a4654acf285dd430fbfbac33cd67411b8) )
	ROM_LOAD( "lwbs7.bin", 0x0000, 0x080000, CRC(5d4177c7) SHA1(e13f145885bb719b0021ae4ce289261a3eaa2e18) )
	ROM_LOAD( "lwbs8.bin", 0x0000, 0x080000, CRC(187cdf5b) SHA1(87ec189af27c95f278a7531ec13df53a08889af8) )
	ROM_LOAD( "lwbs9.bin", 0x0000, 0x080000, CRC(2e02b617) SHA1(2502a1d2cff155a7fc5148e23a4723d4d60e9d42) )
ROM_END

ROM_START( m4madhse )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mh502y.p1", 0x0000, 0x010000, CRC(3ec1955a) SHA1(6939e6f5d749249825c41df8e05957450eaf1007) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "madc.p1", 0x0000, 0x010000, CRC(96da2d58) SHA1(23686a4dc5adaac81ba173f8fa0ea5ff8ac26260) )
	ROM_LOAD( "mhty.p1", 0x0000, 0x010000, CRC(e86e4542) SHA1(fb1b1d319c443daa1184eac4f6b0668ff3c6a1c5) )
	ROM_LOAD( "mads.p1", 0x0000, 0x010000, CRC(d4ea7f14) SHA1(808c64b65542c6bfd8336feec025e947c8c904ee) )
	ROM_LOAD( "md8c.p1", 0x0000, 0x010000, CRC(3d9c9cf7) SHA1(3f663fe9bd61d163d58bb4c51bea59121678aa76) )
	ROM_LOAD( "md8d.p1", 0x0000, 0x010000, CRC(6d150df3) SHA1(e93fda497696b06ad854b3b06e2b61737cef3fc1) )
	ROM_LOAD( "md8dy.p1", 0x0000, 0x010000, CRC(af01407f) SHA1(46359220e34dede4cc5e8c11699d01460dd9a469) )
	ROM_LOAD( "md8k.p1", 0x0000, 0x010000, CRC(c713f706) SHA1(65480171d3d69a670b9ce2c566425998134ad502) )
	ROM_LOAD( "md8s.p1", 0x0000, 0x010000, CRC(0d8a1a3e) SHA1(4df8b1c1834bbffb4798d9ed5135b6cb29b08e73) )
	ROM_LOAD( "md8y.p1", 0x0000, 0x010000, CRC(15fc9590) SHA1(a01bd5cea5873c8175262f668e330b2975a03eb1) )
	ROM_LOAD( "mh502ad.p1", 0x0000, 0x010000, CRC(55714741) SHA1(287ed4c0b070537e3cf9bf3a47bdf205e34b7ea8) )
	ROM_LOAD( "mh502b.p1", 0x0000, 0x010000, CRC(7d6a3e3a) SHA1(5ac43616bde8079d430c7a8f78884770396cc9e9) )
	ROM_LOAD( "mh502bd.p1", 0x0000, 0x010000, CRC(63bd7096) SHA1(ded53d603f7d1ed65914d9483923c74424964b59) )
	ROM_LOAD( "mh502d.p1", 0x0000, 0x010000, CRC(f618a54e) SHA1(947e155bf52a37edf38346b21a9e7171d12e07a1) )
	ROM_LOAD( "mh502dk.p1", 0x0000, 0x010000, CRC(968c9881) SHA1(30952b3811c13ebf6801df150ce24602a37414bb) )
	ROM_LOAD( "mh502dr.p1", 0x0000, 0x010000, CRC(430c2bee) SHA1(37fc62b1fda1de6b52d2a47b4015f6a57503bf2f) )
	ROM_LOAD( "mh502dy.p1", 0x0000, 0x010000, CRC(de670300) SHA1(32e17baed9c971e879974489ee9da375a0dbf735) )
	ROM_LOAD( "mh502k.p1", 0x0000, 0x010000, CRC(3aa341ec) SHA1(8012ee1d3f67fbdd5682ee07ac77dbb482b027ca) )
	ROM_LOAD( "mh502r.p1", 0x0000, 0x010000, CRC(a3aabdb4) SHA1(74e53a315b10264c23b8b00d6a4d5f99d3f204a3) )
	ROM_LOAD( "mh502s.p1", 0x0000, 0x010000, CRC(063cc07b) SHA1(0b43a5cf6094bd8c99e4395f31ff073389dd56ce) )
	ROM_LOAD( "mhtad.p1", 0x0000, 0x010000, CRC(edfe01be) SHA1(5d738acc0a39906f085c1bc55caf683d6b6a4f6c) )
	ROM_LOAD( "mhtb.p1", 0x0000, 0x010000, CRC(272a3c62) SHA1(c6d71295d11350a0b778382a276b8bdf88faede9) )
	ROM_LOAD( "mhtbd.p1", 0x0000, 0x010000, CRC(f73a3808) SHA1(5f74eb64a9b12b9c2c1141b30e64325b8a5beece) )
	ROM_LOAD( "mhtd.p1", 0x0000, 0x010000, CRC(60fcc377) SHA1(e7ca02e2f966a72449a6a5c239160c8f3e46d249) )
	ROM_LOAD( "mhtdk.p1", 0x0000, 0x010000, CRC(ceafb47e) SHA1(32b7e23229524cc79ca24ee00368a9b4c76a35c7) )
	ROM_LOAD( "mhtdy.p1", 0x0000, 0x010000, CRC(36748788) SHA1(04a541f1a6b94dca2bff16d50674f968e896bea7) )
	ROM_LOAD( "mhtk.p1", 0x0000, 0x010000, CRC(1ebfb014) SHA1(493bf3ca37f2e49c5f00d7b8f6122e42f7b71f73) )
	ROM_LOAD( "mhts.p1", 0x0000, 0x010000, CRC(751b4574) SHA1(a04820f48e0df936813ca984c77da08d703e6474) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "m574.chr", 0x0000, 0x000048, CRC(cc4b7911) SHA1(9f8a96a1f8b0f9b33b852e93483ce5c684703349) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "madh1.bin", 0x000000, 0x080000, CRC(2b2af5dd) SHA1(eec0808bf724a055ece3c964d8a43cc5f837a3bd) )
	ROM_LOAD( "madh2.bin", 0x080000, 0x080000, CRC(487d8e1d) SHA1(89e01a153d17564eba112d882b686c91b6c3aecc) )
ROM_END


ROM_START( m4magdrg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dmd10.bin", 0x0000, 0x010000, CRC(9cc4f2f8) SHA1(46a90ffa18d35ad2b06542f91120c02bc34f0c40) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "mdsnd.bin", 0x000000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
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









ROM_START( m4nhtt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nht01b.p1", 0x0000, 0x010000, CRC(8201a051) SHA1(a87550c0cdc0b14a30e8814bfef939eb5cf414f8) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "nht01ad.p1", 0x0000, 0x010000, CRC(a5c6ce9a) SHA1(f21dcc1a70fa45637f236aede9c6fa2e962af8f5) )
	ROM_LOAD( "nht01bd.p1", 0x0000, 0x010000, CRC(21c50c56) SHA1(66c7dfa15447a2519cad58daebe0832c4c2f6f5e) )
	ROM_LOAD( "nht01d.p1", 0x0000, 0x010000, CRC(4d0868a0) SHA1(1f70273928582b87693f046e10e22c19d6bcf87e) )
	ROM_LOAD( "nht01dk.p1", 0x0000, 0x010000, CRC(0826d154) SHA1(154d4bd702ffc13603adc4a72aa1f18e486aec30) )
	ROM_LOAD( "nht01dr.p1", 0x0000, 0x010000, CRC(77862195) SHA1(69f4a50507cf608e0a060231aaf09fc5690787ad) )
	ROM_LOAD( "nht01dy.p1", 0x0000, 0x010000, CRC(954df9ba) SHA1(61cfb2c68921576549d40ba6776877322e4dd338) )
	ROM_LOAD( "nht01k.p1", 0x0000, 0x010000, CRC(c9ab03b3) SHA1(acc26a54bfe6e26fc2c8ac58268ee9347bc4ddb9) )
	ROM_LOAD( "nht01r.p1", 0x0000, 0x010000, CRC(f5ec653e) SHA1(aed9320ab164dd0f2b3dfaee3aacde5ba62e31ef) )
	ROM_LOAD( "nht01s.p1", 0x0000, 0x010000, CRC(a4a44ddf) SHA1(e64953f3cd2559a8ebdacb2b0c12c84fd5c4b836) )
	ROM_LOAD( "nht01y.p1", 0x0000, 0x010000, CRC(54c02b5d) SHA1(75b3056d714ee232325f8a1058bef46d902d0b64) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "nhtsnd01.p1", 0x0000, 0x080000, CRC(2d1d93c6) SHA1(80a8d131bafdb74d20d1ca5cbe2219ee4df0b675) )
ROM_END

ROM_START( m4nick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nick_21.bin", 0x0000, 0x010000, CRC(77a0ba57) SHA1(d56bdd52f81d707b5138f00e68b10130cac1225f) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "nilc.p1", 0x0000, 0x010000, CRC(8e612b50) SHA1(a33142ca3988e449ae94978946ed0f171c52c5fa) )
	ROM_LOAD( "nilx.p1", 0x0000, 0x010000, CRC(210be67b) SHA1(b4f7b955ffe6a991f06334cb7eb2aebcf5fe11b3) )
	ROM_LOAD( "nilxb.p1", 0x0000, 0x010000, CRC(41fde39d) SHA1(fbb179d942a1ffb9c84925402179c7c7fd0a7692) )
	ROM_LOAD( "nilxc.p1", 0x0000, 0x010000, CRC(38cabacf) SHA1(67aafaecc93a348dcdf7beaf6c93c16101fccb55) )
	ROM_LOAD( "nilxk.p1", 0x0000, 0x010000, CRC(ceb04af2) SHA1(1cd65356fba532b4c34e03b418708b982e8e0828) )
ROM_END


ROM_START( m4nspot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nits.p1", 0x0000, 0x010000, CRC(47c965e6) SHA1(41a337a9a367c4e704a60e32d56b262d03f97b59) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ns2d.p1", 0x0000, 0x010000, CRC(5e66b7e0) SHA1(e82044e3c1e5cf3a2baf1fde7b7ab8b6e221d360) )
	ROM_LOAD( "ns2s.p1", 0x0000, 0x010000, CRC(ba0f5a81) SHA1(7015176d4528636cb8a753249c824c37941e8eae) )
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




ROM_START( m4nudshf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nus6", 0x0000, 0x010000, CRC(017c5354) SHA1(07491e4b03ab62ad923f8479300c1af4633e3e8c) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "nuss.bin", 0x0000, 0x010000, CRC(d3b860ee) SHA1(d5d1262c715e4684748b0cae708eeed31b1dc50f) )
	ROM_LOAD( "nusx.p1", 0x0000, 0x010000, CRC(87caab84) SHA1(e2492ad0d25ded4d760c4cbe05e9b51ca1a10544) )
	ROM_LOAD( "nusxc.p1", 0x0000, 0x010000, CRC(e2557b45) SHA1(a9d1514d4fe3897f6fcef22a5039d6bdff8126ff) )
ROM_END



ROM_START( m4ordmnd )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rab01.p1", 0x0000, 0x020000, CRC(99964fe7) SHA1(3745d09e7a4f417c8e85270d3ffec3e37ee1344d) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "odsnd1.bin", 0x000000, 0x080000, CRC(d746bae4) SHA1(293e1dc9edf88a183cc23dbb4576cefbc8f9d028) )
	ROM_LOAD( "odsnd2.bin", 0x080000, 0x080000, CRC(84ace1f4) SHA1(9cc70e59e9d26006870ea1cc522de33e71b71692) )
	ROM_LOAD( "odsnd3.bin", 0x100000, 0x080000, CRC(b1b12def) SHA1(d8debf8cfb3af2157d5d1571927588dc1c8d07b6) )
ROM_END


ROM_START( m4ptblkc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "potblackcasinoprg.bin", 0x0000, 0x020000, CRC(29190084) SHA1(c7a778331369c0fac796ef3e306e12c98605f365) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "potblackcasinosnd.p1", 0x000000, 0x080000, CRC(72a3331d) SHA1(b7475ba0ad86a6277e3d4f7b4311a98f3fc29802) )
	ROM_LOAD( "potblackcasinosnd.p2", 0x080000, 0x080000, CRC(c2460eec) SHA1(7c62fbc69ffaa788bf3839e37a75a812a7b8caef) )
ROM_END





ROM_START( m4prem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dpm14.bin", 0x0000, 0x010000, CRC(de344759) SHA1(d3e7514da83bbf1eba63661fb0675a6230af93cd) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dpms.bin", 0x0000, 0x080000, CRC(93fd4253) SHA1(69feda7ffc56defd515c9cd1ce204af3d9731a3f) )
ROM_END


ROM_START( m4przfrt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pfr03s.p1", 0x0000, 0x010000, CRC(0ea80adb) SHA1(948a23fe8ccf6f423957a478a57bb875cc7b2cc2) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "pfr03ad.p1", 0x0000, 0x010000, CRC(860cbd1b) SHA1(a3a3c0c3c5aff9b469ae82cf514937973b752421) )
	ROM_LOAD( "pfr03b.p1", 0x0000, 0x010000, CRC(2a7ba02c) SHA1(178fbf0301d263b32f9a8ac00e79731d074576d9) )
	ROM_LOAD( "pfr03bd.p1", 0x0000, 0x010000, CRC(dfff487c) SHA1(bf4bbb17241595ceb2c373c2bbd72fcecddedfd2) )
	ROM_LOAD( "pfr03d.p1", 0x0000, 0x010000, CRC(f5b1c305) SHA1(74c8c4a2f32af250cc44994425166fc8b6177610) )
	ROM_LOAD( "pfr03dk.p1", 0x0000, 0x010000, CRC(dc59bba1) SHA1(3ddb259dcda34efe0e8303dc39a3c082665709e9) )
	ROM_LOAD( "pfr03dr.p1", 0x0000, 0x010000, CRC(047fee03) SHA1(c0beb050062c175190dc5326ee21e9af6268bf81) )
	ROM_LOAD( "pfr03dy.p1", 0x0000, 0x010000, CRC(052c60fb) SHA1(bb3ac21ce7f9d41fbb5d62311c5f4a977d417adb) )
	ROM_LOAD( "pfr03k.p1", 0x0000, 0x010000, CRC(d8c2527c) SHA1(0c5925f3eb2e48038a2995367865daf687ee5db6) )
	ROM_LOAD( "pfr03r.p1", 0x0000, 0x010000, CRC(0a1aa1c9) SHA1(c4dcd3550e99a908fe4a9db34f3d8b685af57e30) )
	ROM_LOAD( "pfr03y.p1", 0x0000, 0x010000, CRC(9e230f5d) SHA1(f89c01b930af9cf952fa79baa0deab6503577a90) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "pfrsnd.p1", 0x0000, 0x080000, CRC(71d1af20) SHA1(d87d61c561acbe9cb3dec18d8decf5e970efa272) )
ROM_END




ROM_START( m4przlux )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plxs.p1", 0x0000, 0x010000, CRC(0aea0339) SHA1(28da52924fe2bf00799ef466143103e08399f5f5) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "plxad.p1", 0x0000, 0x010000, CRC(e52ddf4f) SHA1(ec3f198fb6658cadd45046ef7586f9178f95d814) )
	ROM_LOAD( "plxb.p1", 0x0000, 0x010000, CRC(03b0f7bd) SHA1(0ce1cec1afa0a2efee3bc55a2b9cdf8fec7d3ebc) )
	ROM_LOAD( "plxd.p1", 0x0000, 0x010000, CRC(46ae371e) SHA1(a164d0336ed6bf7d25f406e28a01bbec86f4b723) )
	ROM_LOAD( "plxdy.p1", 0x0000, 0x010000, CRC(40fbaad9) SHA1(d3da773d49941e87d008313e309c4dbe7c9bade2) )
	ROM_LOAD( "plxk.p1", 0x0000, 0x010000, CRC(8a15d3bc) SHA1(4536a52101f79ff352b446d130f7a15a0e4cb7df) )
	ROM_LOAD( "plxy.p1", 0x0000, 0x010000, CRC(0e5a2c5c) SHA1(7c64f9ad3aac30b4140be12ec451d17fd3b83b7a) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "plxsnd.p1", 0x000000, 0x080000, CRC(0e682b6f) SHA1(459a7ca216c47af58c03c15d6ef1f9aa7489eba0) )
	ROM_LOAD( "plxsnd.p2", 0x080000, 0x080000, CRC(3ef95a7f) SHA1(9c918769fbf0e687f27e431d934e2327df9ed3bb) )
ROM_END



ROM_START( m4przmns )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spmy.p1", 0x0000, 0x010000, CRC(2b27b2a0) SHA1(07950616da39e39d19452859390d3eaad89ea377) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sm8ad.p1", 0x0000, 0x010000, CRC(6272ae09) SHA1(96130f62646424dd9f2f34f2858a2635ec615f03) )
	ROM_LOAD( "sm8b.p1", 0x0000, 0x010000, CRC(25d95c1b) SHA1(7aa448d1fb383d1b89e71bbc63a554eaa5e06141) )
	ROM_LOAD( "sm8bd.p1", 0x0000, 0x010000, CRC(bf58108f) SHA1(a0dfc2447a014f4a9b1abad3f954ee9c58251289) )
	ROM_LOAD( "sm8d.p1", 0x0000, 0x010000, CRC(b4524fb3) SHA1(11d2542a43e61cee6cce0e621fe80f9aa6811ec2) )
	ROM_LOAD( "sm8dk.p1", 0x0000, 0x010000, CRC(f077ac65) SHA1(9baa5d2fd9833838d48c202a57aaa98783130dbc) )
	ROM_LOAD( "sm8dy.p1", 0x0000, 0x010000, CRC(2df61788) SHA1(003d6e172cee41cf9704dc285c2a0b39ee247ea8) )
	ROM_LOAD( "sm8k.p1", 0x0000, 0x010000, CRC(8d02ca2b) SHA1(b5defdc50fee9e9f1379571b638702c0779fd450) )
	ROM_LOAD( "sm8s.p1", 0x0000, 0x010000, CRC(be159855) SHA1(277884b5417857fa661b09d3e41bef2b22b89f6c) )
	ROM_LOAD( "sm8y.p1", 0x0000, 0x010000, CRC(51e76e1d) SHA1(3045ab447871c7369c5ed53da75326e64d6e57d9) )
	ROM_LOAD( "spmb.p1", 0x0000, 0x010000, CRC(752dd1c6) SHA1(e180c959bc3fb8bce9da22ed6e74fa03e4562a74) )
	ROM_LOAD( "spmd.p1", 0x0000, 0x010000, CRC(34172b4f) SHA1(8594d3863e3de3e6300cd5f4588545bf82c89e00) )
	ROM_LOAD( "spmdy.p1", 0x0000, 0x010000, CRC(1abed85e) SHA1(0b2d7e0127c30f6704a7f64a2955ecf3e8010206) )
	ROM_LOAD( "spmk.p1", 0x0000, 0x010000, CRC(ba2f467a) SHA1(327ebad946b028f387e04e9db9f882320995d175) )
	ROM_LOAD( "spms.p1", 0x0000, 0x010000, CRC(7d684358) SHA1(b07b13d6827e5ea4127eb763f4233a3d35ea99e6) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "s&fpmsnd.p1", 0x000000, 0x080000, CRC(e5bfc522) SHA1(38c8430f539d38a51a3d7fb846b625ae2080e930) )
	ROM_LOAD( "s&fpmsnd.p2", 0x080000, 0x080000, CRC(e14803ab) SHA1(41d501f61f202df2dbd2ac13c40a32fff6afc861) )
ROM_END




ROM_START( m4przrf )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pr8ad.p1", 0x0000, 0x020000, CRC(ebada7c9) SHA1(4a1e2f746116c23f87b53d25bd8b11322962306f) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "pr8b.p1", 0x0000, 0x020000, CRC(4a6448b6) SHA1(061dbc1603fff0cb60e02acdf21881047b2b7d43) )
	ROM_LOAD( "pr8bd.p1", 0x0000, 0x020000, CRC(66b7090c) SHA1(774f5b1403109ccc7ac1bc188f30e8b3a5025aad) )
	ROM_LOAD( "pr8d.p1", 0x0000, 0x020000, CRC(377f43c0) SHA1(14e29f1832afc47f06752d7da11cc2cb40fcb368) )
	ROM_LOAD( "pr8dj.p1", 0x0000, 0x020000, CRC(42629fdf) SHA1(79148956b0b2da42400fe3cc0a61955c77a6bf32) )
	ROM_LOAD( "pr8dk.p1", 0x0000, 0x020000, CRC(2c70a15a) SHA1(9dacf5eca4d7e41b09ee53ffc532a2928b1f60b4) )
	ROM_LOAD( "pr8dy.p1", 0x0000, 0x020000, CRC(ab9381a4) SHA1(90c3a048ad5c1e19007b6e089750a9e4b299d2a3) )
	ROM_LOAD( "pr8j.p1", 0x0000, 0x020000, CRC(6eb1de65) SHA1(b9e13173191e9a45fab29936b303a914e372918f) )
	ROM_LOAD( "pr8k.p1", 0x0000, 0x020000, CRC(00a3e0e0) SHA1(c0671052de5cdd7f169ca50590b9c4f0f10cb678) )
	ROM_LOAD( "pr8s.p1", 0x0000, 0x020000, CRC(bbbdd4f4) SHA1(72c2a8b3404384b524f49fc2d6507e2d8dab85cb) )
	ROM_LOAD( "pr8y.p1", 0x0000, 0x020000, CRC(8740c01e) SHA1(c75f4ad724e735a2ffabc9f7cce96dcb341eaf4a) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4przrfm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prub.p1", 0x0000, 0x010000, CRC(748f220f) SHA1(5d729057d521fa656375610e424cfd4088f6ea02) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "prud.p1", 0x0000, 0x010000, CRC(426bf7c1) SHA1(998b7968d4ed2fb0d1fcaf13929c76670100d9df) )
	ROM_LOAD( "prudy.p1", 0x0000, 0x010000, CRC(e9f76ebd) SHA1(8f1151e123e73ac40fdb6f071960d1ed3e72692a) )
	ROM_LOAD( "pruk.p1", 0x0000, 0x010000, CRC(b995d098) SHA1(22107fbbc8c4e026fc34159114cdbfcd130f814e) )
	ROM_LOAD( "prus.p1", 0x0000, 0x010000, CRC(d6c22253) SHA1(f9a25dd1c6f16849a6eb1febdc2da16080cc6838) )
	ROM_LOAD( "pruy.p1", 0x0000, 0x010000, CRC(fcd8add4) SHA1(14e922daf24d981a3a65463bf64213722d8ba758) )
	ROM_LOAD( "rm8b.p1", 0x0000, 0x010000, CRC(181da11e) SHA1(c06a9626a541a56d707f9b80806714020cefa7b2) )
	ROM_LOAD( "rm8bd.p1", 0x0000, 0x010000, CRC(b3d983b5) SHA1(7881c31617855983981f93190afddb0aa880ce0a) )
	ROM_LOAD( "rm8d.p1", 0x0000, 0x010000, CRC(94377ab0) SHA1(2c43dfd11eeca53faae661d7af4a986fdbb6d7e9) )
	ROM_LOAD( "rm8dj.p1", 0x0000, 0x010000, CRC(601b8f3b) SHA1(3cc130adb5e78e9a5380b27a219a022201293988) )
	ROM_LOAD( "rm8dk.p1", 0x0000, 0x010000, CRC(8b281018) SHA1(5d1f68662b206f9c9948d32fdcda98d99a53987b) )
	ROM_LOAD( "rm8dy.p1", 0x0000, 0x010000, CRC(bac738e3) SHA1(21bd359cfeaf1e33268cecef08d8c7d23d89360c) )
	ROM_LOAD( "rm8j.p1", 0x0000, 0x010000, CRC(b825b8fd) SHA1(6fa58784018fd7be6528e60d8642803cca55c15d) )
	ROM_LOAD( "rm8k.p1", 0x0000, 0x010000, CRC(3f559f9e) SHA1(f70c127490859a3b4c405fd0efd18168dd3b0728) )
	ROM_LOAD( "rm8s.p1", 0x0000, 0x010000, CRC(9ab83f24) SHA1(bdc72a9d6f22244a2be86b035fac84433705ce78) )
	ROM_LOAD( "rm8y.p1", 0x0000, 0x010000, CRC(47a3873e) SHA1(51baf82a7a4dee10b1a2f7862030f960912d8d7c) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* missing */
ROM_END










ROM_START( m4rdht )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drh12", 0x0000, 0x010000, CRC(b26cd308) SHA1(4e29f6cce773232a1c43cd2fb3ce9b844c446bb8) ) // aka gdjb

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "drh_1.snd", 0x0000, 0x080000, CRC(f652cd0c) SHA1(9ce986bc12bcf22a57e065329e82671d19cc96d7) ) // aka gn.snd
ROM_END






ROM_START( m4rhrcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhrc.hex", 0x0000, 0x010000, CRC(e4b89d53) SHA1(fc222d56cdba2891048726d6e6ecd8a4028ba8ba) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "rh2d.p1", 0x0000, 0x010000, CRC(b55a01c3) SHA1(8c94c2ca509ac7631528df78e82fb39b5f579c45) )
	ROM_LOAD( "rh2f.p1", 0x0000, 0x010000, CRC(83466c89) SHA1(790d626e361bfec1265edc6f6ce51f098eb774ba) )
	ROM_LOAD( "rh2s.p1", 0x0000, 0x010000, CRC(aa15e8a8) SHA1(243e7562a4cf938527afebbd99581acea1ab4134) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "m462.chr", 0x0000, 0x000048, CRC(ab59f1aa) SHA1(04a7deac039bc9bc15ec07b8a4ba3bce6f4d5103) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "rhrcs1.hex", 0x000000, 0x080000, CRC(7e265003) SHA1(3800ddfbdde07bf0af5db5cbe05a85425297fa4a) )
	ROM_LOAD( "rhrcs2.hex", 0x080000, 0x080000, CRC(39843d40) SHA1(7c8efcce4ed4ed53e681680bb33869f14f662609) )
ROM_END


ROM_START( m4rwb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drw14.bin", 0x0000, 0x010000, CRC(22c30ebe) SHA1(479f66732aac56dae60c80d11f05c084865f9389) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "rwb_1.snd", 0x000000, 0x080000, CRC(e0a6def5) SHA1(e3867b83e588fd6a9039b8d45186480a9d0433ea) )
	ROM_LOAD( "rwb_2.snd", 0x080000, 0x080000, CRC(54a2b2fd) SHA1(25875ff873bf22df510e7a4c56c336fbabcbdedb) )
ROM_END




ROM_START( m4rmtp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crmtb14.epr", 0x0000, 0x010000, CRC(79e1746c) SHA1(794317f3aba7b1a7994cde89d81abc2b687d0821) ) // aka starplay.epr

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "r4iha202.bin", 0x0000, 0x010000, CRC(b1588632) SHA1(ad21bbc5e99fd6b511e6881e8b20dcad177b937f) )
	ROM_LOAD( "r4iha203.bin", 0x0000, 0x010000, CRC(7f31cb76) SHA1(9a2a595afb9ff1b3165638d247ab98475ae0bfcd) )
	ROM_LOAD( "r4iha204.bin", 0x0000, 0x010000, CRC(1cc3a32d) SHA1(b6ed012a6d743ba2416e25e7c49ce9985bbacbd7) )
	ROM_LOAD( "r4iha205.bin", 0x0000, 0x010000, CRC(1a238632) SHA1(a15ca5801d41985387bc65579b6d6ee2ef7d8eee) )
	ROM_LOAD( "r4iua202.bin", 0x0000, 0x010000, CRC(c96d630a) SHA1(90ed759602aa3a052434b3f604ec26ec9e204e68) )
	ROM_LOAD( "r4iua203.bin", 0x0000, 0x010000, CRC(550fdfec) SHA1(d57eaba6690cbff2302559e9cea9e5d0f79cf9f9) )
	ROM_LOAD( "r4iua204.bin", 0x0000, 0x010000, CRC(cd8d166f) SHA1(4d78726df35914444be26ac9e1e3e1949b6a3d99) )
	ROM_LOAD( "r4iua205.bin", 0x0000, 0x010000, CRC(46df24f3) SHA1(31000815a90e47e744091bbf0fe9e96baac8d7e3) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ctp.chr", 0x0000, 0x000048, CRC(ead61793) SHA1(f38a38601a67804111b8f8cf0a05d35ed79b7ed1) ) // aka starpl.chr

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "rm.s3", 0x0000, 0x080000, CRC(250e64f2) SHA1(627c4dc5cdc7d0a7cb6f74991ae91b71a2f4dbc6) )
	ROM_LOAD( "scrmtb.snd", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) ) // aka starplay.snd
ROM_END

ROM_START( m4magtbo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crmtb14.epr", 0x0000, 0x010000, CRC(79e1746c) SHA1(794317f3aba7b1a7994cde89d81abc2b687d0821) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ctp.chr", 0x0000, 0x000048, CRC(ead61793) SHA1(f38a38601a67804111b8f8cf0a05d35ed79b7ed1) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "scrmtb.snd", 0x000000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END




ROM_START( m4rmtpd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rdiua202.bin", 0x0000, 0x010000, CRC(faa875ea) SHA1(d8d206fed8965a26dd8ded38a3be018311ccf407) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "r2iha203.bin", 0x0000, 0x010000, CRC(1cea7710) SHA1(a250569800d3679f317a485ac7a31b4f4fa7db78) )
	ROM_LOAD( "r2iha204.bin", 0x0000, 0x010000, CRC(c82cd025) SHA1(f26f2bbd83d673c61bd2609914349b45c31f4a5d) )
	ROM_LOAD( "r2iha205.bin", 0x0000, 0x010000, CRC(e53da9a5) SHA1(5019f5bd89c230459629670b808c59888a0f1ee9) )
	ROM_LOAD( "r2iha206.bin", 0x0000, 0x010000, CRC(f89b73b3) SHA1(34a9a8053e881b8aad578ef58209c8ff888b30f7) )
	ROM_LOAD( "r2iua203.bin", 0x0000, 0x010000, CRC(9590a747) SHA1(9f1a1277bdcbe0f23abcf38850eae939997c2e00) )
	ROM_LOAD( "r2iua205.bin", 0x0000, 0x010000, CRC(2eefca1a) SHA1(cabc0c8a3dddc881aab899c5419663efff5412d3) )
	ROM_LOAD( "r2iua206.bin", 0x0000, 0x010000, CRC(c4a1a218) SHA1(8208468ae9ddde7d387f7194e1f7d44f6e7ca730) )
	ROM_LOAD( "r3iha224.bin", 0x0000, 0x010000, CRC(a2e161ac) SHA1(bd63c9726cdf037919c8655221bc6416cef322aa) )
	ROM_LOAD( "r3iha225.bin", 0x0000, 0x010000, CRC(f49a41e9) SHA1(6c29ba4bf76aaafa79ce68f58f6672baa47fe147) )
	ROM_LOAD( "r3iua224.bin", 0x0000, 0x010000, CRC(715b7de7) SHA1(013827680c389968f2f80f97c565716757d696b2) )
	ROM_LOAD( "r3iua225.bin", 0x0000, 0x010000, CRC(37086f91) SHA1(413b32a8e354467a30c71dce3d1cb76795ff813d) )
	ROM_LOAD( "r4iha201.bin", 0x0000, 0x010000, CRC(789cfca1) SHA1(31aa7bf9461cb6c4f692d605463fde1f604b1614) )
	ROM_LOAD( "r4iua201.bin", 0x0000, 0x010000, CRC(ce0e2553) SHA1(4c9df36a7b8950a273cefceb6ba6817d8b862c78) )
	ROM_LOAD( "rdiha202.bin", 0x0000, 0x010000, CRC(02e01481) SHA1(253c2c8e800a4e6d1008745101e2457d76ac57d4) )
	ROM_LOAD( "rdiha212.bin", 0x0000, 0x010000, CRC(90984ae9) SHA1(b25a12f0529af64315c461363c788c22e30d4016) )
	ROM_LOAD( "rdiha213.bin", 0x0000, 0x010000, CRC(e9fa4c97) SHA1(07c75418890231102cf336f2d3f0048fe4884862) )
	ROM_LOAD( "rdiha214.bin", 0x0000, 0x010000, CRC(42f3a5e0) SHA1(3cf26e55edf0dcde9510e50c4b781ba8b906f092) )
	ROM_LOAD( "rdiha215.bin", 0x0000, 0x010000, CRC(2b704591) SHA1(9b880f40d3b268c96af5dab179760994c5a074c9) )
	ROM_LOAD( "rdiha217.bin", 0x0000, 0x010000, CRC(6df58d97) SHA1(df8f419a1e3acc68a3755c49e258db5af9102598) )
	ROM_LOAD( "rdiha219.bin", 0x0000, 0x010000, CRC(66f3ffa6) SHA1(1b1daf4b02e400d943f2a917be0f4452be891aaf) )
	ROM_LOAD( "rdiha220.bin", 0x0000, 0x010000, CRC(2047c55b) SHA1(8f0e6608271634a6a0f06e76df93dddd404c93cd) )
	ROM_LOAD( "rdiha221.bin", 0x0000, 0x010000, CRC(6e87f591) SHA1(750e9f01c1a3143d7d97a5b9b11d09aed72ca928) )
	ROM_LOAD( "rdiha222.bin", 0x0000, 0x010000, CRC(d200f6ec) SHA1(07e0e270a2184f24373cbe0a8a5e44c3d215d9a2) )
	ROM_LOAD( "rdiha223.bin", 0x0000, 0x010000, CRC(042a5a96) SHA1(3bc2dfb89c6781eb9fb105e5f8ea1576d7b49ad3) )
	ROM_LOAD( "rdihb202.bin", 0x0000, 0x010000, CRC(136c31ec) SHA1(abb095bd4ec0a0879f49e668f1ea08df026262e7) )
	ROM_LOAD( "rdiua204.bin", 0x0000, 0x010000, CRC(a6110b45) SHA1(61d08250fa3b5d7eb7cdf63562d7a6cc9a27372c) )
	ROM_LOAD( "rdiua205.bin", 0x0000, 0x010000, CRC(9f20d810) SHA1(e2a576313fa49fc72001d5de67e93c08423e8dd8) )
	ROM_LOAD( "rdiua206.bin", 0x0000, 0x010000, CRC(2954e2c3) SHA1(e4c9f51748bc1296298f95ca817e852f9e0ca38b) )
	ROM_LOAD( "rdiua207.bin", 0x0000, 0x010000, CRC(58f334d1) SHA1(b91288731750445e4cfcf87fe6a9504723b59fa9) )
	ROM_LOAD( "rdiua208.bin", 0x0000, 0x010000, CRC(13e6d84d) SHA1(6f75a75dfd6922349f8d29c955c1849522f8656c) )
	ROM_LOAD( "rdiua209.bin", 0x0000, 0x010000, CRC(f41af938) SHA1(f2d4e23717f49961fe104971b3a0da9aabbf0e05) )
	ROM_LOAD( "rdiua212.bin", 0x0000, 0x010000, CRC(c56a6433) SHA1(7ff8943843c334a79fc3b40bb004abb3f2c2d079) )
	ROM_LOAD( "rdiua213.bin", 0x0000, 0x010000, CRC(cdd5f399) SHA1(a4359c5166fbcd4ea2bb6820bbfead6bc2b2a4ef) )
	ROM_LOAD( "rdiua214.bin", 0x0000, 0x010000, CRC(04fa9d21) SHA1(c337486ece94a7004420edca677e6688eef1ac9e) )
	ROM_LOAD( "rdiua215.bin", 0x0000, 0x010000, CRC(43d8ca5e) SHA1(e5e24ed24bd5c1135392c98910d2797e621ecbd5) )
	ROM_LOAD( "rdiua217.bin", 0x0000, 0x010000, CRC(3c58970e) SHA1(15b7368078750021202ee7b4886a6510fcc1ba0d) )
	ROM_LOAD( "rdiua219.bin", 0x0000, 0x010000, CRC(54f8fe63) SHA1(48d1b04dde6056b839ec84daa40a7d6871893b3e) )
	ROM_LOAD( "rdiua220.bin", 0x0000, 0x010000, CRC(768715f6) SHA1(5c4102b4d2400806dd0f5a6f3e48da4d290d5255) )
	ROM_LOAD( "rdiua222.bin", 0x0000, 0x010000, CRC(07413d93) SHA1(e752fe382d222eefd4fe975fa40559fedd579320) )
	ROM_LOAD( "rdiua223.bin", 0x0000, 0x010000, CRC(80185ebf) SHA1(ae885325f2c63f7dbb034f8e3f3882d0b36aff99) )
	ROM_LOAD( "rdiub202.bin", 0x0000, 0x010000, CRC(4a8d7cf7) SHA1(cb1525d89d3a411163bfe9e70c8b0d1aa6cefdf5) )
	ROM_LOAD( "rdmha210.bin", 0x0000, 0x010000, CRC(7061373e) SHA1(67da39d1de4f3877f12bd1fd5545046f9dabfde9) )
	ROM_LOAD( "rdmhb210.bin", 0x0000, 0x010000, CRC(12c71e8a) SHA1(9bb45e72f202d3af19988ebf30ea4c2248d387fc) )
	ROM_LOAD( "rdpka316.bin", 0x0000, 0x010000, CRC(5178175d) SHA1(a732a82226c34be0b7f84e9f9e4700bd72da1c19) )
	ROM_LOAD( "rdpka318.bin", 0x0000, 0x010000, CRC(2789179f) SHA1(8d4b1e75995ea5b64fac1a36a98506aacfd1800a) )
	ROM_LOAD( "rdpkb316.bin", 0x0000, 0x010000, CRC(09d4f4c5) SHA1(fbc2b0710ef048c221b007692e9a97b99f1edbc0) )
	ROM_LOAD( "rdpkb318.bin", 0x0000, 0x010000, CRC(428aa7f2) SHA1(f85d173c25d0ab9d8c3c4d87b4fc27c3342b3dec) )
	ROM_LOAD( "rduha511.bin", 0x0000, 0x010000, CRC(823e0323) SHA1(4137a05efe87851a9f9ffcd6519bb57398773095) )
	ROM_LOAD( "rduhb511.bin", 0x0000, 0x010000, CRC(2b65eb19) SHA1(b00543b74ad5262b85f66f5e8cfdaee351f62f23) )

	ROM_REGION( 0x10000, "gal", 0 )
	ROM_LOAD( "rmdxi", 0x0000, 0x000b57, CRC(c16021ec) SHA1(df77e410ea2edae1559e40a877e292f0d1969b0a) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )

ROM_END





ROM_START( m4reeltm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "real.bin", 0x0000, 0x010000, CRC(5bd54924) SHA1(23fcf13c52ee7b9b39f30f999a9102171fffd642) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "charter.chr", 0x0000, 0x000048, CRC(4ff4eda2) SHA1(092435e34d79775910316a7bed0f90c4f086e5c4) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )

ROM_END

ROM_START( m4ringfr )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rof03s.p1", 0x0000, 0x020000, CRC(4b4703fe) SHA1(853ce1f5932e09af2b5f3b5314709f13aa35cf19) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( m4rhog2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "2rh06c.p1", 0x0000, 0x020000, CRC(62c312bc) SHA1(6b02345c97b130deabad58a238ba9045161b5a80) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "2rh06ad.p1", 0x0000, 0x020000, CRC(f44040d1) SHA1(685bbfe5f975c7e5b3efee17e1833f6f51b223af) )
	ROM_LOAD( "2rh06b.p1", 0x0000, 0x020000, CRC(5589afae) SHA1(15c9c65089cc2754d644dabfd6f5a32a2a788219) )
	ROM_LOAD( "2rh06bd.p1", 0x0000, 0x020000, CRC(795aee14) SHA1(7703c8456aaa2e27f71a7edbfa74fb2d7434a762) )
	ROM_LOAD( "2rh06d.p1", 0x0000, 0x020000, CRC(2892a4d8) SHA1(2b592552d5d45349b2df0d769bd24649f4da5680) )
	ROM_LOAD( "2rh06dh.p1", 0x0000, 0x020000, CRC(b7b6be57) SHA1(0b02eafe58e23ac50f2ae74137550868115d6f6f) )
	ROM_LOAD( "2rh06dk.p1", 0x0000, 0x020000, CRC(339d4642) SHA1(55301e716c235aab7ee88cc28ec7426bd24328c0) )
	ROM_LOAD( "2rh06dr.p1", 0x0000, 0x020000, CRC(8092fd73) SHA1(52c68ee7d02f7256b434110c4df2e926b528af16) )
	ROM_LOAD( "2rh06dy.p1", 0x0000, 0x020000, CRC(b47e66bc) SHA1(01ae45693fd4b81c9090a29264dfa1db58837dde) )
	ROM_LOAD( "2rh06h.p1", 0x0000, 0x020000, CRC(9b65ffed) SHA1(65ab62fe772bd54793c45cc1105a189f21bb5d25) )
	ROM_LOAD( "2rh06k.p1", 0x0000, 0x020000, CRC(1f4e07f8) SHA1(35459640bc215c465b84df073505e8fd6077a332) )
	ROM_LOAD( "2rh06r.p1", 0x0000, 0x020000, CRC(ac41bcc9) SHA1(0de0c0976ef5c58084f02310495b246dc7c23e60) )
	ROM_LOAD( "2rh06s.p1", 0x0000, 0x020000, CRC(2ea10eed) SHA1(825bd6a53100b389f7d67ec49e4535c1de0ece74) )
	ROM_LOAD( "2rh06y.p1", 0x0000, 0x020000, CRC(98ad2706) SHA1(862a725bad97d28580dad102a71750465c7b0f5d) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "rh.chr", 0x0000, 0x000048, CRC(5522383a) SHA1(4413b1d68500f21f10e7cff6b2d3de7258b1b614) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "2rhsnd02.p1", 0x000000, 0x080000, CRC(0f4630dc) SHA1(7235e53c74e113230a683de33763023e95090d39) )
	ROM_LOAD( "2rhsnd02.p2", 0x080000, 0x080000, CRC(c2d0540a) SHA1(160080b350d41b95a0c129f9189222d79734e7d0) )
ROM_END


ROM_START( m4rhogc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhcf.p1", 0x0000, 0x010000, CRC(0b726e87) SHA1(12c334e7dd712b9e19e8241b1a8e278ff84110d4) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "rhcs.p1", 0x0000, 0x010000, CRC(d1541050) SHA1(ef1ee3b9319e2a357540cf0de902de439267c3e2) )
	ROM_LOAD( "rhcd.p1", 0x0000, 0x010000, CRC(7a7df536) SHA1(9c53e5c6a5f3a32de05a574e1c8dedc3e5be66eb) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "rhc.chr", 0x0000, 0x000048, CRC(6ceab6b0) SHA1(04f4238ea3fcf944c97bc11031e456b851ebe917) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "rhc.s1", 0x000000, 0x080000, CRC(8840737f) SHA1(eb4a4bedfdba1b33fa74b9c2000c0d09a4cca5d7) )
	ROM_LOAD( "rhc.s2", 0x080000, 0x080000, CRC(04eaa2da) SHA1(2c23bde76f6a9406b0cb30246ce8805b5181047f) )
ROM_END


ROM_START( m4roadrn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dro19", 0x0000, 0x010000, CRC(8b591766) SHA1(df156390b427e31cdda64826a6c1d2457c915f25) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dro_1.snd", 0x000000, 0x080000, CRC(895cfe63) SHA1(02134e149cef3526bbdb6cb93ef3efa283b9d6a2) )
	ROM_LOAD( "dro_2.snd", 0x080000, 0x080000, CRC(1d5c8d4f) SHA1(15c18ae7286807cdc0feb825b958eae808445690) )
ROM_END


ROM_START( m4rockmn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rok06c.p1", 0x0000, 0x020000, CRC(8e3a628f) SHA1(3bedb095af710f0b6376a5d99c072f7b3d3de0af) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "rok06ad.p1", 0x0000, 0x020000, CRC(9daa1e35) SHA1(11e7a503c289813cc2ea4507bf5255957e92bc12) )
	ROM_LOAD( "rok06b.p1", 0x0000, 0x020000, CRC(b970df9d) SHA1(4230c3130a52502fb0a8aabf60fd33e90a7fa266) )
	ROM_LOAD( "rok06bd.p1", 0x0000, 0x020000, CRC(10b0b0f0) SHA1(a2f485e7578648aadb8ddb04ea885f4315b9ab82) )
	ROM_LOAD( "rok06d.p1", 0x0000, 0x020000, CRC(c46bd4eb) SHA1(cd07aebaf398eb4bcbc647d0fccdab2561b7aa1c) )
	ROM_LOAD( "rok06dk.p1", 0x0000, 0x020000, CRC(5a7718a6) SHA1(7c6cbdb4066e7664f70a2b584a074652f8baef2c) )
	ROM_LOAD( "rok06dr.p1", 0x0000, 0x020000, CRC(e978a397) SHA1(b48512e1ae332d6e3b6432e9caf247222df5d0a4) )
	ROM_LOAD( "rok06dy.p1", 0x0000, 0x020000, CRC(dd943858) SHA1(ac8e53f72f98b217f190a9d3a9822a41b3028adb) )
	ROM_LOAD( "rok06k.p1", 0x0000, 0x020000, CRC(f3b777cb) SHA1(2d5b67c69458712c370801702a813f81640ce184) )
	ROM_LOAD( "rok06r.p1", 0x0000, 0x020000, CRC(40b8ccfa) SHA1(8868a0ca622e5331204b2431bf64e723a1a79222) )
	ROM_LOAD( "rok06s.p1", 0x0000, 0x020000, CRC(e8b89551) SHA1(753828fd8631588c7725ee4f013f3c78d23f7038) )
	ROM_LOAD( "rok06y.p1", 0x0000, 0x020000, CRC(74545735) SHA1(79ee259656fb71c24382c6670150e49a5b8bc62f) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "rmchr.chr", 0x0000, 0x000048, CRC(94147e3d) SHA1(92e311096331deb54752cc158ece86fbe8ad2063) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "rokmsnd1",  0x000000, 0x080000, CRC(b51e5d7d) SHA1(71f36f866583d592d029cba47901cbfd17631b06) )
	ROM_LOAD( "roksnd.p1", 0x000000, 0x080000, CRC(462a690e) SHA1(5a82f63a9d03c89c8fdb0ead1fc40e480aedd787) )
	ROM_LOAD( "roksnd.p2", 0x080000, 0x080000, CRC(37786d14) SHA1(d6dc2d3dbe54ca943092938500d72081153b5a34) )
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
	ROM_LOAD( "dsa_1#97c2.snd", 0x0000, 0x080000, CRC(0281a6dd) SHA1(a35a8cd0da32c51f77856ea3eeff7c58fd032333) )
ROM_END






ROM_START( m4showtm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dsh13.bin", 0x0000, 0x010000, CRC(4ce40ff1) SHA1(f145d6c8e926ca4368d43dacda0fa38615988d84) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdsh01s1.snd", 0x0000, 0x080000, CRC(f247ba83) SHA1(9b173503e63a4a861d1380b2ab1fe14af1a189bd) )
ROM_END




ROM_START( m4shodf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdfs.p1", 0x0000, 0x010000, CRC(5df9abdb) SHA1(0dce3a7ff4d2f11c370a3a2578c592910a9e7371) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sd8b.p1", 0x0000, 0x010000, CRC(79f7fea2) SHA1(5bfa695aef54c9621a91beac2e6c8a09d3b2974b) )
	ROM_LOAD( "sd8d.p1", 0x0000, 0x010000, CRC(060a1b37) SHA1(fb4fbc1164f97f13eb10edbd4e8a37502d716340) )
	ROM_LOAD( "sd8dk.p1", 0x0000, 0x010000, CRC(20982264) SHA1(178ce24ce21e865608133fe2ae281ba2adbdf1d4) )
	ROM_LOAD( "sd8dy.p1", 0x0000, 0x010000, CRC(3fb73b48) SHA1(328f827a92e6fb8ccfb3a82c52401b2d31e974bf) )
	ROM_LOAD( "sd8k.p1", 0x0000, 0x010000, CRC(0d8f2238) SHA1(55643a1f9fe136fb724b05efc0362b6295c9caf9) )
	ROM_LOAD( "sd8s.p1", 0x0000, 0x010000, CRC(59d696e4) SHA1(e51a9a0bc1348b44e77f85343463154ad680ef89) )
	ROM_LOAD( "sd8y.p1", 0x0000, 0x010000, CRC(f79c2e78) SHA1(f6c298b77a9c32378e3f219063daab17e551d083) )
	ROM_LOAD( "sdfb.p1", 0x0000, 0x010000, CRC(a15204bb) SHA1(c862822615e82e5f2f9f2f3cb7e31f804fd859be) )
	ROM_LOAD( "sdfd.p1", 0x0000, 0x010000, CRC(19913c83) SHA1(894da549e790b9062f36fdce90b8e8d284d513e6) )
	ROM_LOAD( "sdfdy.p1", 0x0000, 0x010000, CRC(df1325b1) SHA1(002780fcecf895d20a2a3c0c57fbe4dd675a1e42) )
	ROM_LOAD( "sdfk.p1", 0x0000, 0x010000, CRC(32def2fb) SHA1(45064f319cb5268745e8d5210ceed3a84a8e7f20) )
	ROM_LOAD( "sdfy.p1", 0x0000, 0x010000, CRC(dbb6aa80) SHA1(976f5811a0a578c7f2497ac654f7c416b6018a34) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sdfsnd.p1", 0x000000, 0x080000, CRC(a5829cec) SHA1(eb65c86125350a7f384f9033f6a217284b6ff3d1) )
	ROM_LOAD( "sdfsnd.p2", 0x080000, 0x080000, CRC(1e5d8407) SHA1(64ee6eba3fb7700a06b89a1e0489a0cd54bb89fd) )
ROM_END
















ROM_START( m4squid )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "squidsin.bin", 0x0000, 0x020000, CRC(be369b43) SHA1(e5c7b7a858b264db2f8f726396ddeb42004d7cb9) )

	ROM_REGION( 0x20000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sq__x_dx.2_0", 0x0000, 0x020000, CRC(2eb6c814) SHA1(070ad5cb36220daf98043f175cf67d4d584c3d01) )
	ROM_LOAD( "sq__xa_x.2_0", 0x0000, 0x020000, CRC(196a6b34) SHA1(a044ba73b4cf04657ddfcf787dedcb151507ef15) )
	ROM_LOAD( "sq__xb_x.2_0", 0x0000, 0x020000, CRC(53adc362) SHA1(3920f08299bf284ee9f102ce1505d9e9cdc1d1f0) )


	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "squidsnd.p1", 0x000000, 0x080000, CRC(44cebe30) SHA1(a93f64897b4ba333d044649f28fa5dd68d3d2e94) )
	ROM_LOAD( "squidsnd.p2", 0x080000, 0x080000, CRC(d2a1b073) SHA1(d4931f18d369e89492fe72a7a1c511c8d3c23a71) )
ROM_END










ROM_START( m4steptm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dst11.bin", 0x0000, 0x010000, CRC(3960f210) SHA1(c7c4fe74cb9a53eaa9114a84240de3bce4ffe75e) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "sdun01.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END









ROM_START( m4supbj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbj31.hex", 0x0000, 0x010000, CRC(f7fb2b99) SHA1(c860d3f95ee3fde02bf00b2e20eeee0ebaf01912) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sbjackv3", 0x0000, 0x002000, CRC(13b799d6) SHA1(510f9b60dbcdbd5ce115f103aeee414443ca1d96) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "sbj.chr", 0x0000, 0x000048, CRC(cc4b7911) SHA1(9f8a96a1f8b0f9b33b852e93483ce5c684703349) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sbjsnd1.hex", 0x000000, 0x080000, CRC(70388bec) SHA1(256fa01b57049d73e88b0bb270fccb555b12dfb7) )
	ROM_LOAD( "sbjsnd2.hex", 0x080000, 0x080000, CRC(1d588554) SHA1(48c092ce83d2f881fc217a3d566e896718ad6f24) )
ROM_END


ROM_START( m4supbjc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbj31.hex", 0x0000, 0x010000, CRC(f7fb2b99) SHA1(c860d3f95ee3fde02bf00b2e20eeee0ebaf01912) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sbjd.p1", 0x0000, 0x010000, CRC(555361f4) SHA1(f5327b811ab3421307dc59d209a216798cd54393) )
	ROM_LOAD( "sbjf.p1", 0x0000, 0x010000, CRC(7966deff) SHA1(5cdb6c80ef56b27878eb1fffd6fdf31060e56291) )
	ROM_LOAD( "sbjl.p1", 0x0000, 0x010000, CRC(fc47ed74) SHA1(f29b2caac8168410e534e2f224c98dd4bbb9a7f7) )
	ROM_LOAD( "sbjs.p1", 0x0000, 0x010000, CRC(f7fb2b99) SHA1(c860d3f95ee3fde02bf00b2e20eeee0ebaf01912) )
	ROM_LOAD( "superbjclub.bin", 0x0000, 0x010000, CRC(68d11d27) SHA1(a0303f845fb5f5b396a7be3ca17a9eaf1a7baef4) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "sbj.chr", 0x0000, 0x000048, CRC(cc4b7911) SHA1(9f8a96a1f8b0f9b33b852e93483ce5c684703349) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "sbjsnd1.hex", 0x000000, 0x080000, CRC(70388bec) SHA1(256fa01b57049d73e88b0bb270fccb555b12dfb7) )
	ROM_LOAD( "sbjsnd2.hex", 0x080000, 0x080000, CRC(1d588554) SHA1(48c092ce83d2f881fc217a3d566e896718ad6f24) )

	ROM_LOAD( "sbj.s1", 0x000000, 0x080000, CRC(9bcba966) SHA1(5ced282aca9d39ebf0828aa19357026d5298e955) )
	ROM_LOAD( "sbj.s2", 0x080000, 0x080000, CRC(1d588554) SHA1(48c092ce83d2f881fc217a3d566e896718ad6f24) )
ROM_END


ROM_START( m4supbf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbff.p1", 0x0000, 0x010000, CRC(f27feba0) SHA1(157bf28e2d5fc2fa58bed11b3285cf56ae18abb8) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sbfs.p1", 0x0000, 0x010000, CRC(c8c52d5e) SHA1(d53513b9faabc307623a7c2f5be0225fb812beeb) )
ROM_END


ROM_START( m4suphv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hyperviper.bin", 0x0000, 0x010000, CRC(8373f6a3) SHA1(79bff20ab80ffe11447595c6fe8e5ab90d432e17) )

	ROM_REGION( 0x10000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "hv_05___.3h3", 0x0000, 0x010000, CRC(13bfa891) SHA1(ffddd14a019d52029bf8d4f680d8d05413a9f0b7) )
	ROM_LOAD( "hv_05___.3o3", 0x0000, 0x010000, CRC(9ae86366) SHA1(614ae0ab184645c9f568796783f29a177eda3208) )
	ROM_LOAD( "hv_05___.4n3", 0x0000, 0x010000, CRC(f607f351) SHA1(d7b779b80fa964a27b106bd9d5ca3be16a11d5e9) )
	ROM_LOAD( "hv_05_d_.3h3", 0x0000, 0x010000, CRC(50c66ce8) SHA1(ef12525fc3ac82caf80326edaac81bb9fbc3245c) )
	ROM_LOAD( "hv_05_d_.3o3", 0x0000, 0x010000, CRC(87dfca0e) SHA1(3ab4105680acc46d3633a722f40ff1af0a520a7f) )
	ROM_LOAD( "hv_05_d_.4n3", 0x0000, 0x010000, CRC(f4d702d7) SHA1(268c7f6443c7ae587caf5b227fcd438530a06bcc) )
	ROM_LOAD( "hv_10___.3h3", 0x0000, 0x010000, CRC(627caac7) SHA1(4851ce2441850743ea68ecbf89bde3f4cd6c2b4c) )
	ROM_LOAD( "hv_10___.3o3", 0x0000, 0x010000, CRC(02e4d86a) SHA1(47aa83e8bcd85e8ba7fb972cdd1ead7fe21e0418) )
	ROM_LOAD( "hv_10_d_.3h3", 0x0000, 0x010000, CRC(15cfa26e) SHA1(6bc3feaba65d1797b9945f23a89e983f56b13f79) )
	ROM_LOAD( "hv_10_d_.3n3", 0x0000, 0x010000, CRC(b81f1d0a) SHA1(5fd293be2b75393069c9f5e099b4700ff930f081) )
	ROM_LOAD( "hv_10_d_.3o3", 0x0000, 0x010000, CRC(85f176b9) SHA1(30380d58bf2834829764cbdbdc7d950632e61e6d) )
	ROM_LOAD( "hvi05___.3h3", 0x0000, 0x010000, CRC(6959332e) SHA1(edaa5f86ad4389b0a3bc2e6679fe8f62520be3ae) )
	ROM_LOAD( "hvi05___.3o3", 0x0000, 0x010000, CRC(cdba80a5) SHA1(6c9fac7e5ee324b18922cc7a053495f1977bcb6d) )
	ROM_LOAD( "hvi05___.4n3", 0x0000, 0x010000, CRC(38a33c2b) SHA1(21004092b81e08146291fd3a025652f0edbe47dc) )
	ROM_LOAD( "hvi10___.3h3", 0x0000, 0x010000, CRC(6c1b4b89) SHA1(e8eb4e689d43c5b9e8354aa7375ca3ba12ed1160) )
	ROM_LOAD( "hvi10___.3n3", 0x0000, 0x010000, CRC(9d95cf8c) SHA1(26daf3975e1e3a605bc4392700c5470b52450d6e) )
ROM_END







ROM_START( m4take5 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "take5.bin", 0x0000, 0x020000, CRC(24beb7d6) SHA1(746beccaf57fd0c54c8cf8d742b8ef50563a40fd) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "tfive1.hex", 0x000000, 0x080000, CRC(70f16892) SHA1(e6448831d3ce7fa251b40023bc7d5d6dee9d6793) )
	ROM_LOAD( "tfive2.hex", 0x080000, 0x080000, CRC(5fc888b0) SHA1(8d50ee4f36bd36aed5d0e7a77f76bd6caffc6376) )
ROM_END





ROM_START( m4tpcl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ctp12s.p1", 0x0000, 0x020000, CRC(5f0bbd2a) SHA1(ba1fa09ea7b4713a99b2033bdbbf6b15f973dcca) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ctp13d.p1", 0x0000, 0x020000, CRC(a0f081b9) SHA1(794bba6ed86c3f332165c4b3224315256c939926) )
	ROM_LOAD( "ctp13f.p1", 0x0000, 0x020000, CRC(dd7d94fd) SHA1(127ef8159facf647dff37109bcbb94311a8343f1) )
	ROM_LOAD( "ctp13s.p1", 0x0000, 0x020000, CRC(f0a69f92) SHA1(cd34fb26ecbe6a6e8602a8549c5f331a525567df) )
	ROM_LOAD( "ntp02.p1", 0x0000, 0x020000, CRC(6063e27d) SHA1(c99599fbc7146d8fcf62432994098dd51250b17b) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ctp13s.chr", 0x0000, 0x000048, CRC(6b8772a9) SHA1(8b92686e675b00d2c2541dd7b8055c3145283bec) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "ctpsnd02.p1", 0x000000, 0x080000, CRC(6fdd5051) SHA1(3f713314b303d6e1f78e3ca050bed7a45f43d5b3) )
	ROM_LOAD( "ctpsnd02.p2", 0x080000, 0x080000, CRC(994bfb3a) SHA1(3cebfbbe77c4bbb5fb73e6d9b23f721b07c6435e) )
ROM_END


ROM_START( m4techno )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dte13.bin", 0x0000, 0x010000, CRC(cf661d06) SHA1(316b2c42e7253a03b2c12b713821045d9f95a8a7) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "dte13hack.bin", 0x0000, 0x010000, CRC(8b8eafe3) SHA1(93a7714eb4c749b7b19f4f844cf88da9443b0bb7) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "techno.bin", 0x0000, 0x080000, CRC(3e80f8bd) SHA1(2e3a195b49448da11cc0c089a8a9b462894c766b) )
ROM_END








ROM_START( m4toma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtk23.bin", 0x0000, 0x010000, CRC(ffba2b96) SHA1(c7635023ac5181e661e808c6b44ac1add58f4f56) )
ROM_END




ROM_START( m4topdk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtd26pj.bin", 0x0000, 0x010000, CRC(1f84d995) SHA1(7412632cf79008b980e48f14aea89c3f8d742ed2) )
ROM_END


ROM_START( m4toprn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "toprun_v1_1.bin", 0xc000, 0x004000, CRC(9b924324) SHA1(7b155467f30cc22f7cda301ae770fb2a889c9c66) )
	ROM_LOAD( "toprun_v1_2.bin", 0x8000, 0x004000, CRC(940fafa9) SHA1(2a8b669c51c8df50710bd8b552ab30a5d1a136ab) )
ROM_END






ROM_START( m4toptim )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "toptimer.bin", 0x0000, 0x010000, CRC(74804012) SHA1(0d9460ba6b1d359d358483c4e8bfd5518f364518) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "dtt2-1.bin", 0x0000, 0x010000, CRC(f9c84a34) SHA1(ad654442f580d6a49658f0e4e39bacbd9d0d0018) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ttimer.chr", 0x0000, 0x000048, CRC(f694224e) SHA1(936ab5e349fa59accbb37959cce9519fd97f3978) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "toptimer-snd.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END







ROM_START( m4tropcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tro20.bin", 0x0000, 0x010000, CRC(5e86c3fc) SHA1(ce2419991559839a8875060c1afe0f030190010a) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "tr2d.p1", 0x0000, 0x010000, CRC(0cc23f89) SHA1(a66c8c28073f53381c43e3e597f15f81c5c61479) )
	ROM_LOAD( "tr2f.p1", 0x0000, 0x010000, CRC(fbdcd06f) SHA1(27ccdc83e60a62227d33d8cf3d516fc43908ab99) )
	ROM_LOAD( "tr2s.p1", 0x0000, 0x010000, CRC(6d43375c) SHA1(5be1dc85374c6a1235e0b137b46ebd7a2d7d922a) )
	ROM_LOAD( "trod.p1", 0x0000, 0x010000, CRC(60c84612) SHA1(84dc8b34e41436331832c1a32ddac0fce269488a) )
	ROM_LOAD( "tros.p1", 0x0000, 0x010000, CRC(5e86c3fc) SHA1(ce2419991559839a8875060c1afe0f030190010a) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "tro20.chr", 0x0000, 0x000048, CRC(97618d38) SHA1(7958e99684d50b9bdb56c97f7fcfe161f0824578) )
ROM_END




ROM_START( m4tbplay )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dtp13", 0x0000, 0x010000, CRC(de424bc3) SHA1(c82dd56a0b3ccea78325cd90ed8e72ed68a1af77) )

	ROM_REGION( 0x10000, "altbwb", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "rmtp4b", 0x0000, 0x010000, CRC(33a1764e) SHA1(7475f460dee015a2cd78fc3e0d1d14fd96fdbb9c) )
	ROM_LOAD( "trmyid", 0x0000, 0x010000, CRC(e7af5944) SHA1(64559c97375a3536f7929d7f4d8d19c30527a3ec) )


	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "dtpchr.chr", 0x0000, 0x000048, CRC(7743df66) SHA1(69b1943837ccf8671861ac8ef690138b41de0e5b) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "dtps10_1", 0x000000, 0x080000, CRC(d1d2c981) SHA1(6a4940248b0bc8df0a9de0d60e98cfebf1962504) )
	ROM_LOAD( "dtps20_1", 0x080000, 0x080000, CRC(f77c4f39) SHA1(dc0e056f4d8c00824b3e672a02da64613bbf204e) )
ROM_END





ROM_START( m4tutcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f2u01ad.p1", 0x0000, 0x010000, CRC(65537552) SHA1(b0a761dcc6e0a9f01cfb934b570356ca67fdd099) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "f2u01b.p1", 0x0000, 0x010000, CRC(2cae37df) SHA1(5aed985476b7b747a99a4046b846ee4a359776af) )
	ROM_LOAD( "f2u01bd.p1", 0x0000, 0x010000, CRC(0dd91ccf) SHA1(bcdfc39025d02e7a51f69757238dfa44fe9d3655) )
	ROM_LOAD( "f2u01c.p1", 0x0000, 0x010000, CRC(6b6d9bb9) SHA1(140e9cbb8b484116e5fb9a7670d41fb0bcb37ec0) )
	ROM_LOAD( "f2u01d.p1", 0x0000, 0x010000, CRC(b477a20d) SHA1(51daf5e61a2ebcb3cb9884421b9e8f32df51ec07) )
	ROM_LOAD( "f2u01dk.p1", 0x0000, 0x010000, CRC(ccd14dd3) SHA1(c93ff69e0534e8190c10e0c819ed439d4e61a472) )
	ROM_LOAD( "f2u01dr.p1", 0x0000, 0x010000, CRC(d4918506) SHA1(2081ead45ff744cafcf3c4164c86acf609e54632) )
	ROM_LOAD( "f2u01dy.p1", 0x0000, 0x010000, CRC(24dd0a73) SHA1(a75129e414dd8cbe5f6f44e39b1d3dc3d7dfafb2) )
	ROM_LOAD( "f2u01k.p1", 0x0000, 0x010000, CRC(b9cec403) SHA1(90a1f49202ea9b79e2ab097cf95cf94088c52926) )
	ROM_LOAD( "f2u01r.p1", 0x0000, 0x010000, CRC(471e39d7) SHA1(874db6f2d04ed0b2c6756efba5fa1140d2fbfc58) )
	ROM_LOAD( "f2u01s.p1", 0x0000, 0x010000, CRC(25b68f22) SHA1(7f484dbc841e1e87d9f5e322cf497b6b68e4a096) )
	ROM_LOAD( "f2u01y.p1", 0x0000, 0x010000, CRC(5a583a6f) SHA1(0421d079de12a7379c13832108e8608c9a01f41d) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* missing? in above set? */
ROM_END






ROM_START( m4twintm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d2t11.bin", 0x0000, 0x010000, CRC(6a76ac6f) SHA1(824912ff1fc3155d11d32b597be53481532fdf5e) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "m533.chr", 0x0000, 0x000048, CRC(b1d7e29b) SHA1(e8ef07f85780e24b5f406478de50287b740379d9) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "sdun01.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END


ROM_START( m4twist )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twist_again_mk29-6", 0x8000, 0x008000, CRC(cb331bee) SHA1(a88099a3f35caf02925f1a3f548fbf65c11e3ec9) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "twistagain-98-mkii.bin", 0x0000, 0x008000, CRC(1cbc7b58) SHA1(eda998a64272fe6796243c2db48ef988b9668c35) )
	ROM_LOAD( "twistagain-mki-27.bin", 0x0000, 0x008000, CRC(357f7072) SHA1(8a23509fff79a83a819b27eff8de8db08c679e3f) )
ROM_END




ROM_START( m4univ )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dun20", 0x0000, 0x010000, CRC(6a845d4d) SHA1(82bfc3f3a0ede76a4d482efc71b0390610db7acf) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "dunchr.chr", 0x0000, 0x000048, CRC(f694224e) SHA1(936ab5e349fa59accbb37959cce9519fd97f3978) )

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







ROM_START( m4voodoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ddo32", 0x0000, 0x010000, CRC(260dfef1) SHA1(2b4918e40808963a86d289cd251740a9b0bed70a) )
ROM_END






ROM_START( m4przwo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pwo206ac", 0x0000, 0x010000, CRC(b9dd88e7) SHA1(4c60e7a28b538ff2483839fc66600037ccd99440) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "pwob.p1", 0x0000, 0x010000, CRC(9e9f65d7) SHA1(69d28a1e08d2bde1a9c4d55555478808546ad4f0) )
	ROM_LOAD( "pwod.p1", 0x0000, 0x010000, CRC(ae97b585) SHA1(d6b90d8b696a21f9fa6b06c63a329b1370edd224) )
	ROM_LOAD( "pwody.p1", 0x0000, 0x010000, CRC(3abfd1c9) SHA1(131811807396103641d73cd7cef1797a6cecb35b) )
	ROM_LOAD( "pwok.p1", 0x0000, 0x010000, CRC(b8631e11) SHA1(c01aff60dad14945c2b45992f0112c6fc0ae7c5a) )
	ROM_LOAD( "pwos.p1", 0x0000, 0x010000, CRC(6a87aa68) SHA1(3dc8c006de3adcada43c3581be0ff921081ecff0) )
	ROM_LOAD( "pwoy.p1", 0x0000, 0x010000, CRC(1ada4987) SHA1(05a0480f5a92faaedc8183d948c7e2d657bda2a4) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "pwos.chr", 0x0000, 0x000048, CRC(352b86c4) SHA1(59c26a1948ffd6ecea08d8ca8e62735ec9732c0f) )

	ROM_REGION( 0x100000, "msm6376", 0 )
	ROM_LOAD( "pwo.s1", 0x000000, 0x080000, CRC(1dbd8a33) SHA1(37bd71688475591232422eb0841e23aff58e3800) )
	ROM_LOAD( "pwo.s2", 0x080000, 0x080000, CRC(6c7badef) SHA1(416c36fe2b4253bf7944b3ba412561bd0d21cbe5) )
ROM_END





ROM_START( m4wildtm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wildtimer.bin", 0x0000, 0x010000, CRC(5bd54924) SHA1(23fcf13c52ee7b9b39f30f999a9102171fffd642) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "charter.chr", 0x0000, 0x000048, CRC(4ff4eda2) SHA1(092435e34d79775910316a7bed0f90c4f086e5c4) )

	ROM_REGION( 0x080000, "msm6376", 0 )
	ROM_LOAD( "wildtimer-snd.bin", 0x0000, 0x080000, CRC(50450909) SHA1(181659b0594ba8d196b7130c5999c91676a363c0) )
ROM_END



ROM_START( m4ch30 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ch301s", 0x0000, 0x010000, CRC(d31c9081) SHA1(21d1f4cc3de2343d830e3ee02e3a53abd12b6b9d) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* missing */
ROM_END












ROM_START( m4bigmt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigm1320", 0x0000, 0x010000, CRC(a5085347) SHA1(93a7f7656e53461270e04190ff538959d6c917c1) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "tb_20___.7_1", 0x0000, 0x010000, CRC(22fae0f0) SHA1(a875adccf96fbbff69f5fe76720514767cdcae66) )
	ROM_LOAD( "tb_20_b_.7_1", 0x0000, 0x010000, CRC(40d140a3) SHA1(fd4de8dd827db933481f671e4f10684c3b7a363a) )
	ROM_LOAD( "tb_20_d_.7_1", 0x0000, 0x010000, CRC(86ed18c5) SHA1(1699645532134bd830e3fc2c3ff4b67b5d67ba3e) )
	ROM_LOAD( "tb_20_k_.7_1", 0x0000, 0x010000, CRC(e4c6b896) SHA1(ccf656d14ad68edb0aea99d4d3621540b899cdb7) )
	ROM_LOAD( "tb______.1_2", 0x0000, 0x004000, CRC(be2a3989) SHA1(5ef857101335f90cd9f6147ec330ae281bb97b2b) )
	ROM_LOAD( "tbi20___.7_1", 0x0000, 0x010000, CRC(104f45cc) SHA1(c28c08b44c46db9d0eade1f60aeda120c3981a03) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bigmsnd", 0x0000, 0x07db60, CRC(876c53ae) SHA1(ea2511ec9ba4ff67879212c6e2ba908873130a4e) )
	ROM_LOAD( "tbmsnd.hex", 0x0000, 0x080000, CRC(e98da8de) SHA1(36668f2b82778f441224c94831f5b95efb9fa92b) )
ROM_END


ROM_START( m4bingbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bb_20a__.8_1", 0x0000, 0x010000, CRC(10f29ba3) SHA1(739b413f35676834ebafeb121c6059759586ec72) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "bb_20bg_.8_1", 0x0000, 0x010000, CRC(9969c9ce) SHA1(8c754335e7ff75bc46f02095c2c7d57df046db47) )
	ROM_LOAD( "bb_20bt_.8_1", 0x0000, 0x010000, CRC(3a06f518) SHA1(79a9b1e63517f6436ee4743dfc7aa63a9cf585b8) )
	ROM_LOAD( "bb_20s__.8_1", 0x0000, 0x010000, CRC(c5bd878e) SHA1(dc6048cf25b3a62b23761c338fe6398724b24159) )
	ROM_LOAD( "bb_20sb_.8_1", 0x0000, 0x010000, CRC(a79627dd) SHA1(c9a6262a0e52a4d1ff073465ab6b489626ae6a2a) )
	ROM_LOAD( "bb_20sd_.8_1", 0x0000, 0x010000, CRC(61aa7fbb) SHA1(0bf54a562d04c0b41fc35c5c84f529efa2ae9e10) )
	ROM_LOAD( "bb_20sk_.8_1", 0x0000, 0x010000, CRC(0381dfe8) SHA1(8d3d232c10f9ec683a82464ee901f0963d9ffcd6) )
	ROM_LOAD( "bbi20___.8_1", 0x0000, 0x010000, CRC(eae04eac) SHA1(bcc619868a065d92d09fa3b2411958b373b3edde) )
ROM_END


ROM_START( m4bingbs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bp_20a__.2_1", 0x0000, 0x010000, CRC(ca005003) SHA1(271ff0dbee529ca15c79c9aa1047efa8993ea073) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "bp_20bg_.2_1", 0x0000, 0x010000, CRC(1b59c32f) SHA1(0c7df33f921639bfedbddd969dcbcd62e38ff912) )
	ROM_LOAD( "bp_20bt_.2_1", 0x0000, 0x010000, CRC(bc7a1830) SHA1(3c16432562ebaef3f17e51feebb4c35d911e90b9) )
	ROM_LOAD( "bp_20s__.2_1", 0x0000, 0x010000, CRC(1f4f4c2e) SHA1(28f6361c7ee693635c718ea485687bf9fbf8532b) )
	ROM_LOAD( "bp_20sb_.2_1", 0x0000, 0x010000, CRC(7d64ec7d) SHA1(bc279010642edadcba19cdfad222a783e35039b9) )
	ROM_LOAD( "bp_20sd_.2_1", 0x0000, 0x010000, CRC(bb58b41b) SHA1(253859ab6d8033eeb94ad5c4586eadc8792bd436) )
	ROM_LOAD( "bp_20sk_.2_1", 0x0000, 0x010000, CRC(d9731448) SHA1(2f054a99c5d1570b6e0307542c50421408a610dc) )
ROM_END


ROM_START( m4bingcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bc_xe___.2_1", 0x0000, 0x010000, CRC(3abbc215) SHA1(b5e59b30c07c4ffef69c5729f1a28d7ee55636bd) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "bc_xe_b_.2_1", 0x0000, 0x010000, CRC(3e11c5c0) SHA1(2d9bc987fed040664f211bb9d13984b6cba5e25f) )
	ROM_LOAD( "bc_xe_d_.2_1", 0x0000, 0x010000, CRC(0a3a162f) SHA1(1b0857039b0442269b8fd4c063fc99b3b50cc312) )
ROM_END





ROM_START( m4cshenc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ca_sj__c.5_1", 0x0000, 0x020000, CRC(d9131b39) SHA1(4af89a7bc10de1406f401bede41e1bc452dbb159) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ca_sj_bc.5_1", 0x0000, 0x020000, CRC(30d1fb6d) SHA1(f845bef4ad7f2f48077eed74840916e87abb24b2) )
	ROM_LOAD( "ca_sj_dc.5_1", 0x0000, 0x020000, CRC(ac3ec716) SHA1(4ff8c26c46ec6e1321249b4d6d0c5194ed917f33) )
	ROM_LOAD( "ca_sja_c.5_1", 0x0000, 0x020000, CRC(c56a9d0b) SHA1(b0298c2e03097ab8ba5f99892e732ff1ab784c9b) )
	ROM_LOAD( "ca_sjb_c.5_1", 0x0000, 0x020000, CRC(8fad355d) SHA1(2ac16ad85ab8239a3e961abb06f9f71d17e5832a) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cesnd.p1", 0x000000, 0x080000, CRC(2a10dc1a) SHA1(f6803f6e1fee2b58fe4831f59ddc08ec02792823) )
	ROM_LOAD( "cesnd.p2", 0x080000, 0x080000, CRC(6f0b75c0) SHA1(33898d75a1e51b49950d7843069066d17c4736c5) )
ROM_END


ROM_START( m4czne )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "czone 6.bin", 0x0000, 0x010000, CRC(e5b2b64e) SHA1(b73a2aed7b04184bc7c5c3d0a11d44e624a47428) )
ROM_END

ROM_START( m4csoc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "chsoc8ac", 0x0000, 0x040000, CRC(8e0471ba) SHA1(3b7e6edbb3490e99af148c0cfe8d39c13c282880) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sg_sj___.1_0", 0x0000, 0x040000, CRC(f21cd1aa) SHA1(dc010a315a8d738ad9e5e384197499e08a8d5ef6) )
	ROM_LOAD( "sg_sj___.2_0", 0x0000, 0x040000, CRC(5513f2a3) SHA1(e9e59461a007be02beae6cd1610b8582d367c15e) )
	ROM_LOAD( "sg_sj_d_.2_0", 0x0000, 0x040000, CRC(b0058d0f) SHA1(635c4f729c27c5cb4356f62dcc13127043ea0e5c) )
	ROM_LOAD( "so_sj___.b_0", 0x0000, 0x040000, CRC(65d5bb2d) SHA1(61c6896d97ed79e2f31b37b9d8998980ceac4fc5) )
	ROM_LOAD( "so_sj_d_.b_0", 0x0000, 0x040000, CRC(5e79ba34) SHA1(cb8c689319b5f94ce0385b3b7846a49589358ccc) )
	ROM_LOAD( "so_sjs__.b_0", 0x0000, 0x040000, CRC(2f50675e) SHA1(baed8e3a455ec5bfa810e64dc4c66996d6746bbc) )
	ROM_LOAD( "so_vc___.c_0", 0x0000, 0x040000, CRC(d683b202) SHA1(95803008a50229bc85ed177b587fdf05cb152df3) )
	ROM_LOAD( "so_vc_d_.c_0", 0x0000, 0x040000, CRC(ed2fb31b) SHA1(de72d8abbb4a22125ed312e6ccfcab6b3e591ec2) )
	ROM_LOAD( "ch_socc", 0x0000, 0x040000, CRC(ea9af5bd) SHA1(99319995ee886196ddd540bf37960a4e5b9d4f34) )
	ROM_LOAD( "ch_socc.5", 0x0000, 0x040000, CRC(1b2ea78d) SHA1(209534ccd537c0ca9d02301830a52ebc29b93cb7) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ch_socc.s1", 0x000000, 0x080000, CRC(abaea3f3) SHA1(cf3b6e4ee99680726efd2a839b49b4d86e2bd270) )
	ROM_LOAD( "ch_socc.s2", 0x080000, 0x080000, CRC(2048f5b2) SHA1(b07addfd9d861b1d19d4db248e16c597cf79b159) )
	ROM_LOAD( "ch_socc.s3", 0x100000, 0x080000, CRC(064224b0) SHA1(99a8bacfd3a42f72e40b93d1f7eeea633c3cf366) )
ROM_END




ROM_START( m4cpycat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "co_20_bc.1_1", 0x0000, 0x010000, CRC(c9d3cdc1) SHA1(28265b0f95a8829efc4e346269a7af17a6abe345) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "co_20_dc.1_1", 0x0000, 0x010000, CRC(c6552f9a) SHA1(ae7ad183d2cd89bc9748dcbb3ea26832bed30009) )
	ROM_LOAD( "co_20_kc.1_1", 0x0000, 0x010000, CRC(b5260e35) SHA1(6cbf4ca426fd47b0db49e188a7a7fe72f6c99aef) )
	ROM_LOAD( "co_20a_c.1_1", 0x0000, 0x010000, CRC(486d42af) SHA1(327fca9604845ec37c2212413105f48a7b0e2836) )
	ROM_LOAD( "co_20b_c.1_1", 0x0000, 0x010000, CRC(90e0e19f) SHA1(c7e73faa4c3e853dbaa6b14303ab454a09eb36d7) )
	ROM_LOAD( "co_20bgc.1_1", 0x0000, 0x010000, CRC(f99d6ae2) SHA1(2106412caa7d3dfc262dc2b1d3e258bb33605912) )
	ROM_LOAD( "co_20bgp.4_1", 0x0000, 0x010000, CRC(fdc6753a) SHA1(4dd39fa995ee9fa7d153d64dd163d5482aa490d2) )
	ROM_LOAD( "co_20btc.1_1", 0x0000, 0x010000, CRC(3780d767) SHA1(4fb4354b02eed754ca1caef2b56eccc76524ae1e) )
	ROM_LOAD( "co_25_bc.1_1", 0x0000, 0x010000, CRC(13c82730) SHA1(992a6d04ed357548bb6cea6505316013048a2e57) )
	ROM_LOAD( "co_25_bp.2_1", 0x0000, 0x010000, CRC(a5983412) SHA1(7daa3028355fb0e85dce1629477a8efae625f86d) )
	ROM_LOAD( "co_25_bp.3_1", 0x0000, 0x010000, CRC(4060da30) SHA1(34a93e0550992e7510d1eaf2d5109da3c3fa2f75) )
	ROM_LOAD( "co_25_dc.1_1", 0x0000, 0x010000, CRC(a3f34f3f) SHA1(4c4842cc668b4c3abd9d2896cc50bbc3d9643b75) )
	ROM_LOAD( "co_25_dp.2_1", 0x0000, 0x010000, CRC(1f192638) SHA1(16a86242281281ca7b994dee06910d7f107c4743) )
	ROM_LOAD( "co_25_kc.1_1", 0x0000, 0x010000, CRC(6b2a7f2b) SHA1(5d558431a6b83214cc0dc33d999eeb72f3c53e85) )
	ROM_LOAD( "co_25_kp.2_1", 0x0000, 0x010000, CRC(73d3aaba) SHA1(5ef9118462bfdf93182ec539d3b80a72c09fa032) )
	ROM_LOAD( "co_25_kp.3_1", 0x0000, 0x010000, CRC(2e8fa3ee) SHA1(18f4e4eae7d7ac14486ed731bb67cab22d0b287d) )
	ROM_LOAD( "co_25a_c.1_1", 0x0000, 0x010000, CRC(e753196a) SHA1(79fc9b567dc946f81d40c3b215035cf2adcf94af) )
	ROM_LOAD( "co_25a_p.2_1", 0x0000, 0x010000, CRC(a058d7f7) SHA1(6e9116ce757503ef5b1822473a310513dd2973e3) )
	ROM_LOAD( "co_25a_p.3_1", 0x0000, 0x010000, CRC(ba172858) SHA1(d0a025c339f886802b7491448b6b2a4e1f5a3451) )
	ROM_LOAD( "co_25b_c.1_1", 0x0000, 0x010000, CRC(d82a9080) SHA1(9b9091f867f0b5d75f2bd5f9d62a4419073da357) )
	ROM_LOAD( "co_25b_p.2_1", 0x0000, 0x010000, CRC(48c8f7e6) SHA1(ff651e4c88b81bb816ab92e7f1d1fbd2c2920db1) )
	ROM_LOAD( "co_25bgc.1_1", 0x0000, 0x010000, CRC(947a5465) SHA1(c99c0e8ca515fbad8801e07e5936256cca8e7af1) )
	ROM_LOAD( "co_25bgp.2_1", 0x0000, 0x010000, CRC(84ab9c9d) SHA1(4c1043fb7ff6cd4e681f18e2dd0ddd29d5ce6d09) )
	ROM_LOAD( "co_25bgp.3_1", 0x0000, 0x010000, CRC(7fa4089e) SHA1(f73aff58c7a627f993e65527bb551c23640b22ed) )
	ROM_LOAD( "co_25btc.1_1", 0x0000, 0x010000, CRC(dbbae1b6) SHA1(2ee1d53872774d4b80f8c1a4e8a6ceb9e79ed6f5) )
	ROM_LOAD( "co_25btp.2_1", 0x0000, 0x010000, CRC(cea10ed8) SHA1(2b10193824afb50d561b2307a3189d14e2f5d47a) )
	ROM_LOAD( "co_30_bc.2_1", 0x0000, 0x010000, CRC(2d39f5fa) SHA1(20075621085765150a57233eccd61f16dbbae9b1) )
	ROM_LOAD( "co_30_bp.4_1", 0x0000, 0x010000, CRC(0826ffa7) SHA1(05a12d68acf69d8a582fc7fee91a282280380420) )
	ROM_LOAD( "co_30_dc.2_1", 0x0000, 0x010000, CRC(b028d639) SHA1(9393b82d41e7f8a7e2dba33545477ae13a8d6804) )
	ROM_LOAD( "co_30_dp.4_1", 0x0000, 0x010000, CRC(f40f09ca) SHA1(11f7af5bf78768759c3eba50ab1a906e81ce1100) )
	ROM_LOAD( "co_30_kp.4_1", 0x0000, 0x010000, CRC(97ab5c33) SHA1(dc6b9705de4731a5cbc35557ca26c80b20cc6518) )
	ROM_LOAD( "co_30a_c.2_1", 0x0000, 0x010000, CRC(eea1522d) SHA1(fdfe797b8cf2fa10f24e89c3047290ac63acebc7) )
	ROM_LOAD( "co_30b_c.2_1", 0x0000, 0x010000, CRC(61e873b0) SHA1(00037fde263fe9e9cb227ef2945e8b90feee0d6e) )
	ROM_LOAD( "co_30bdc.2_1", 0x0000, 0x010000, CRC(6f261bdc) SHA1(df7ab51c984b20665fdb327d17ca6ec32109ec2d) )
	ROM_LOAD( "co_30bgc.2_1", 0x0000, 0x010000, CRC(85cd1c27) SHA1(e0c250bf2848b6991cf33c07b43c2704ae906e47) )
	ROM_LOAD( "co_30btc.2_1", 0x0000, 0x010000, CRC(3a940326) SHA1(f1a0eca5ceccbf979ac7a2c51bfdc1de6f0aa40e) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "copycatsnd.bin", 0x0000, 0x080000, CRC(cd27a3ce) SHA1(d061fae0ef8584d2e349e91e53f41718128c61e2) )
ROM_END




ROM_START( m4cpfinl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cu_10___.5_1", 0x0000, 0x010000, CRC(47a85443) SHA1(d308b9a6dcb0200f72d5c5b380907d2d55f3e40d) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cu_10_b_.5_1", 0x0000, 0x010000, CRC(2583f410) SHA1(447a2316e3c3da6f835699602834f7ca5bafbdf9) )
	ROM_LOAD( "cu_10_d_.5_1", 0x0000, 0x010000, CRC(e3bfac76) SHA1(bf1bfa7a995dc4198de890d307718ddb2d9e0092) )
	ROM_LOAD( "cu_10_k_.5_1", 0x0000, 0x010000, CRC(81940c25) SHA1(7bb98b0f6355c6cea12c2ffb2751c4dc35042120) )
	ROM_LOAD( "cu_20___.5_1", 0x0000, 0x010000, CRC(69a38622) SHA1(65d83c9590bf200077046d564778e7fc7d146030) )
	ROM_LOAD( "cu_20_b_.5_1", 0x0000, 0x010000, CRC(0b882671) SHA1(0f3382385c9065eb275d59cb17045a592bdc11cc) )
	ROM_LOAD( "cu_20_d_.5_1", 0x0000, 0x010000, CRC(cdb47e17) SHA1(f491049cbd2f5a1f803a3f22ef12f4d41020a928) )
	ROM_LOAD( "cu_20_k_.5_1", 0x0000, 0x010000, CRC(af9fde44) SHA1(916295e999ea1017a6f24a73436d97cf89f827c8) )
	ROM_LOAD( "cui20___.5_1", 0x0000, 0x010000, CRC(5b16231e) SHA1(1bf37843c6e757d7ffe9932eab72b7f49a9ef107) )
	ROM_LOAD( "cui20_b_.5_1", 0x0000, 0x010000, CRC(393d834d) SHA1(12766cdc1c75ed0d967d7937b9fea2cfb3b6c2c3) )
	ROM_LOAD( "cui20_d_.5_1", 0x0000, 0x010000, CRC(ff01db2b) SHA1(bd4f826f235e0c0bf53584abc0dc1929ac157d99) )
	ROM_LOAD( "cui20_k_.5_1", 0x0000, 0x010000, CRC(9d2a7b78) SHA1(d7c26a47dcbb836650f3021733d09a426ff3a390) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "cupsnd_1.0_2", 0x000000, 0x080000, CRC(54384ce8) SHA1(ff78c4ea16722662a480bff1f85af7efe84b01e5) )
	ROM_LOAD( "cupsnd_1.0_3", 0x080000, 0x080000, CRC(24d3d848) SHA1(64287c3cbe2e9693954bc880d6edf2bc17b0ed65) )
ROM_END


ROM_START( m4danced )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dd_22bg_.2_1", 0x0000, 0x020000, CRC(f79525a1) SHA1(babfbf8beae423626057235bcad5eae18531160e) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "dd_22bg_.4_1", 0x0000, 0x020000, CRC(e50ffa46) SHA1(b42d806422f85573bcbe284b4192f393e3e57306) )
	ROM_LOAD( "dd_22bt_.4_1", 0x0000, 0x020000, CRC(11e910a2) SHA1(6e35ae37dbd12169ccd9cf5b32a3f08f9e3d1899) )
	ROM_LOAD( "dd_25__c.2_1", 0x0000, 0x020000, CRC(1b072e2b) SHA1(cd40d0b7ef29979d65bfabdffee9cf686c6d6f49) )
	ROM_LOAD( "dd_25_bc.2_1", 0x0000, 0x020000, CRC(41f544ec) SHA1(5676f3ca6abc71fdb9286a3b47cb789990298548) )
	ROM_LOAD( "dd_25_dc.2_1", 0x0000, 0x020000, CRC(28b4b2bc) SHA1(8d6294fa4c10fa26322130e6ac1d227b777e8028) )
	ROM_LOAD( "dd_25a_c.2_1", 0x0000, 0x020000, CRC(e243b150) SHA1(8828b73921d49fa9e61ac56325bdc35e60b06d1e) )
	ROM_LOAD( "dd_25b_c.2_1", 0x0000, 0x020000, CRC(4fb0abb1) SHA1(bb8099c90312eb4509b15f098636d6c8e1db5a1d) )
	ROM_LOAD( "dd_25bgc.2_1", 0x0000, 0x020000, CRC(7e8c62bf) SHA1(63515ec61ff0e8fb4d62e14059001e2a33906bd6) )
	ROM_LOAD( "dd_25btc.2_1", 0x0000, 0x020000, CRC(10f3d837) SHA1(71958987e481fba5c5a43495d0c2f823e21e234c) )
	ROM_LOAD( "dd_26a__.5_1", 0x0000, 0x020000, CRC(e854ceb3) SHA1(1bb2eea5d353f62558993f3845bd3d11611045d1) )
	ROM_LOAD( "dd_26b__.4_1", 0x0000, 0x009712, CRC(3f974b8a) SHA1(9563c204e290756494f1efae67d2cfafefe1163e) )
	ROM_LOAD( "dd_26bg_.4_1", 0x0000, 0x009712, CRC(0ac7f8e4) SHA1(a8ebdcd281b46b3546f58f726f4d57d4ed41777b) )
	ROM_LOAD( "dd_26bt_.4_1", 0x0000, 0x009715, CRC(aab6371c) SHA1(cb0eb4fb1630ff36c872d17cb2984cc43e691f7e) )
	ROM_LOAD( "dd_26s__.4_1", 0x0000, 0x020000, CRC(ba1e2ec8) SHA1(c4f5a46a684219bdf531234c3dc06fda4b7c7a3e) )
	ROM_LOAD( "dd_26sb_.4_1", 0x0000, 0x00970e, CRC(b0b04ab6) SHA1(52d47bd159a26c745c3c61daae1ed68099f5d297) )
	ROM_LOAD( "dd_32a__.3_1", 0x0000, 0x020000, CRC(b6f7a984) SHA1(70323926114855071981f3b8f00064b0149824d1) )
	ROM_LOAD( "dd_32b__.3_1", 0x0000, 0x020000, CRC(013c05d2) SHA1(a332ddb36c137cc111a629adee60d2d89c5f2d88) )
	ROM_LOAD( "dd_32bg_.3_1", 0x0000, 0x020000, CRC(a949d679) SHA1(a282c498b263c6a85323f42d41ac706f44047adf) )
	ROM_LOAD( "dd_32bt_.3_1", 0x0000, 0x020000, CRC(f7935bd2) SHA1(cfa65404fa091f445a6b2f30a359739087b7d57b) )
	ROM_LOAD( "dd_32s__.3_1", 0x0000, 0x020000, CRC(46fc506e) SHA1(87f2a201e39e78d33ce87b9be44ddf5d5e62106d) )
	ROM_LOAD( "dd_32sb_.3_1", 0x0000, 0x020000, CRC(9155deb5) SHA1(5b3ee18fd003e882f80a6c14d01215b4eeb8831e) )
	ROM_LOAD( "dd_32sd_.3_1", 0x0000, 0x0096b3, CRC(4135b5d9) SHA1(568563d3057717754a348b9a8870f41441a3c6a7) )
	ROM_LOAD( "dd_sja__.2_1", 0x0000, 0x020000, CRC(45db5106) SHA1(b12a5c2c3f61cc78a7ff040e1ffff82c225e6d9e) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "dd______.1_2", 0x0000, 0x080000, CRC(b9043a08) SHA1(5d87a30f23e8b5e3eaa0584d0d49efc08209882b) )
ROM_END


ROM_START( m4daytn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "da_78___.1_0", 0x0000, 0x040000, CRC(50beafdd) SHA1(0ef6dd4fc9c8cda596fd383e47b9c7976b5d15f0) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "da_78_d_.1_0", 0x0000, 0x040000, CRC(d55d3f9a) SHA1(a145379237947601f2ecb84138c113b71842cd34) )
	ROM_LOAD( "da_80___.1_0", 0x0000, 0x040000, CRC(60f31edd) SHA1(ee016e5001cc80f6796b3a00ceebf14b5fb38ae7) )
	ROM_LOAD( "da_80_d_.1_0", 0x0000, 0x040000, CRC(e5108e9a) SHA1(4d0d530803bd349f57a2a3216fe284e7490a0ae0) )
	ROM_LOAD( "da_82___.1_0", 0x0000, 0x040000, CRC(93a22c97) SHA1(bcb0089c8d2febedd84207851a2c7997e31f8a27) )
	ROM_LOAD( "da_82_d_.1_0", 0x0000, 0x040000, CRC(1641bcd0) SHA1(66e4bc49d6dc8dc1926f3c2faac21498df6082d1) )
	ROM_LOAD( "da_84___.1_0", 0x0000, 0x040000, CRC(5d207c08) SHA1(57d6d5947c611badb1573739995037beb8017774) )
	ROM_LOAD( "da_84_d_.1_0", 0x0000, 0x040000, CRC(d8c3ec4f) SHA1(f3571c36114809dd440cee5d406f3da704797b01) )
	ROM_LOAD( "da_86___.1_0", 0x0000, 0x040000, CRC(ae714e42) SHA1(17ec05b35c28968c4319a71b1485052e04c23c0a) )
	ROM_LOAD( "da_86_d_.1_0", 0x0000, 0x040000, CRC(2b92de05) SHA1(f59e66c486c51bc0e968fc90009884e6ac851b93) )
	ROM_LOAD( "da_88___.1_0", 0x0000, 0x040000, CRC(1b55db77) SHA1(147a36ea76ba30eb465df3e2ad5795d8e3f96d99) )
	ROM_LOAD( "da_88_d_.1_0", 0x0000, 0x040000, CRC(9eb64b30) SHA1(f527ef38f9965774f59b5d36f45f801c7d7ce714) )
	ROM_LOAD( "da_90___.1_0", 0x0000, 0x040000, CRC(a54d2e81) SHA1(65f08c83dcff2934938a7aa1b56e02ba18ba7898) )
	ROM_LOAD( "da_90_d_.1_0", 0x0000, 0x040000, CRC(20aebec6) SHA1(bcf4ca9fa5723fcae0ea661b7cfa005cd0046cb1) )
	ROM_LOAD( "da_92_d_.1_0", 0x0000, 0x040000, CRC(9e99647d) SHA1(34cf734808ffbfa9bc920ad1c93c0a9f7bbba791) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END



ROM_START( m4excal )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ex_05a__.6_1", 0x0000, 0x020000, CRC(317fa289) SHA1(8a0e83a764e2a04285367e0f7ebb814fedc81400) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ex_20a_6.6_1", 0x0000, 0x020000, CRC(284937c8) SHA1(3be8bf21ab0ff97f67ce170cee48cd08ea325571) )
	ROM_LOAD( "ex_20a_8.6_1", 0x0000, 0x020000, CRC(45fb3559) SHA1(9824d2e6bc8c2249de66aea2422b7a6efd0e37b3) )
	ROM_LOAD( "ex_20a_c.7_1", 0x0000, 0x020000, CRC(ab773d70) SHA1(3a5e3aa3978bf0bc9a4924d3225665b583c90abc) )
	ROM_LOAD( "ex_20atc.7_1", 0x0000, 0x020000, CRC(ec5c7bc7) SHA1(c57eb69044858b7ea43726d0f49e1c224aeb1377) )
	ROM_LOAD( "ex_20s_6.6_1", 0x0000, 0x020000, CRC(44176959) SHA1(fcaad1732176330b382f1690293ed43fa66de084) )
	ROM_LOAD( "ex_20s_8.6_1", 0x0000, 0x020000, CRC(29a56bc8) SHA1(7345b8bd4e3cf64ca222c59857ef7df9b1db45c1) )
	ROM_LOAD( "ex_20sb6.6_1", 0x0000, 0x020000, CRC(e387c4b9) SHA1(cc83de53a1ffe8d75acb9c2f2ceca9bbff516df7) )
	ROM_LOAD( "ex_20sb8.6_1", 0x0000, 0x020000, CRC(8e35c628) SHA1(000811c50189cae0f7327b4e7484eb64091a989c) )
	ROM_LOAD( "ex_20sd6.6_1", 0x0000, 0x020000, CRC(7adbd752) SHA1(d758e202ed75ce3a99f0666c405090107cb0feca) )
	ROM_LOAD( "ex_20sd8.6_1", 0x0000, 0x020000, CRC(1769d5c3) SHA1(3e5ff338364781e99685cde9e45707dd88d1da11) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "ex______.1_2", 0x000000, 0x080000, CRC(9305e516) SHA1(5a5c67f97761fe2e042ba31594d1881238d3227b) )
	ROM_LOAD( "ex______.1_3", 0x080000, 0x080000, CRC(29e3709a) SHA1(2e2f089aa2a938158930f235bf821685932d698b) )
	ROM_LOAD( "ex______.1_4", 0x100000, 0x080000, CRC(dd747003) SHA1(cf0a2936c897e3b833984c55f4825c358b723ab8) )
ROM_END



ROM_START( m4exotic )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "eo_49bg_.2_0", 0x0000, 0x020000, CRC(c3bf2286) SHA1(74090fd0a103a6c311d426f4aae8e7af8b1d3bc0) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "eo_49bm_.2_0", 0x0000, 0x020000, CRC(c748c4ca) SHA1(7d0d498f9edd792ed861c8bf9cf1bb03698d144d) )
	ROM_LOAD( "eo_49bmd.2_0", 0x0000, 0x020000, CRC(98436c04) SHA1(1db7c95f7a0297aa3da7f1ce27c790ffa1fa4ebe) )
	ROM_LOAD( "eo_49bmd.2g0", 0x0000, 0x020000, CRC(425a0152) SHA1(6a235c613f52c4b8985a589f89542eebd3574fde) )
	ROM_LOAD( "eo_s9bt_.2g0", 0x0000, 0x020000, CRC(c527d333) SHA1(083d7be95d73d259fe8ec1d87a3a41089a4c44df) )
	ROM_LOAD( "eo_sja__.2_0", 0x0000, 0x020000, CRC(5ca9557f) SHA1(5fa42c56c67b505272d358a54ebe911fdb0b905e) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( m4firice )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "fi_20__d.5_0", 0x0000, 0x040000, CRC(ab46574c) SHA1(d233b137f8f42b9b644b34a627fbcc5b662e8ae1) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "fi_20_bd.5_0", 0x0000, 0x040000, CRC(9b2bc052) SHA1(34b970659218fde097238b852dadedcb928f69fd) )
	ROM_LOAD( "fi_20_dd.5_0", 0x0000, 0x040000, CRC(2bbc9855) SHA1(84d51eeadc01ac74d630a05b933343f01f04b2af) )
	ROM_LOAD( "fi_20_kd.5_0", 0x0000, 0x040000, CRC(529b43b1) SHA1(80d4d928918fdc869b1693e21a5c25045e5c9449) )
	ROM_LOAD( "fi_20a_d.5_0", 0x0000, 0x040000, CRC(8d6ce79d) SHA1(05954ed4b34af73c065b6203a50da9af7d8373fe) )
	ROM_LOAD( "fi_20s_d.5_0", 0x0000, 0x040000, CRC(d0aa53af) SHA1(f71801344c17a759ec4eb8958377bbcf4b4cae65) )
	ROM_LOAD( "fi_sj___.e_0", 0x0000, 0x040000, CRC(7f12e37a) SHA1(fb09ff782f66972b8bdeff105c5f3d1f9f676809) )
	ROM_LOAD( "fi_sj_b_.e_0", 0x0000, 0x040000, CRC(5aef48d2) SHA1(73f410951a737f75f3e7c14e704eca9c26cfa750) )
	ROM_LOAD( "fi_sj_d_.e_0", 0x0000, 0x040000, CRC(61822af2) SHA1(8c721229a5ce9f491cbc638b8c5fa5c0c3032700) )
	ROM_LOAD( "fi_sj_k_.e_0", 0x0000, 0x040000, CRC(2b57036b) SHA1(60cec130770ff643af1148f16a3afe3b102e94e2) )
	ROM_LOAD( "fi_sja__.e_0", 0x0000, 0x040000, CRC(da5e0eff) SHA1(9f5ddce366786bdf898c9410be417c8028cebeb4) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "fire_ice.s1", 0x000000, 0x080000, CRC(74ee37c1) SHA1(bc9b419dd1fd1c66090f8946247e754c0267baa3) )
	ROM_LOAD( "fire_ice.s2", 0x080000, 0x080000, CRC(b86bafeb) SHA1(ee237f601b970dc5be8096a4018cb6a3edac500f) )
	ROM_LOAD( "fire_ice.s3", 0x100000, 0x080000, CRC(75f349b3) SHA1(1505bec7b69e1eabd679b70d95ae58fd264ca698) )
ROM_END



ROM_START( m4fourmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frmr5p26.bin", 0x8000, 0x008000, CRC(f0c5bd8a) SHA1(39026459008ed5b5bd3a10841799227fef70e5b5) )
ROM_END





ROM_START( m4harle )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hq_20a__.1_1", 0x0000, 0x010000, CRC(b8ae3025) SHA1(94a449eff103bf6ba1fc6e85b03061b9ce658ae0) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "hq_20bg_.1_1", 0x0000, 0x010000, CRC(31356248) SHA1(d8791b1c861ed4388660bbe78f2589db7f1e779e) )
	ROM_LOAD( "hq_20bg_.2a1", 0x0000, 0x010000, CRC(f50898af) SHA1(ba4470abb85b92d647ff8da48dba571cec5f594e) )
	ROM_LOAD( "hq_20bt_.1_1", 0x0000, 0x010000, CRC(925a5e9e) SHA1(e8a4f75d4f7e6825894e19a915db21ee21206b5b) )
	ROM_LOAD( "hq_20s__.1_1", 0x0000, 0x010000, CRC(6de12c08) SHA1(df279a07ccd448b9a6948ccb059aff6306ddcc99) )
	ROM_LOAD( "hq_20sb_.1_1", 0x0000, 0x010000, CRC(0fca8c5b) SHA1(f430f2d1ff71513426bb5cfed883797de6afd3a7) )
	ROM_LOAD( "hq_20sd_.1_1", 0x0000, 0x010000, CRC(c9f6d43d) SHA1(4eef4ec6bff20fdf5b20b266ddd1646045390d42) )
	ROM_LOAD( "hq_20sk_.1_1", 0x0000, 0x010000, CRC(abdd746e) SHA1(5bb7043a1a790e5c784c7a51fd3c023b3099439f) )
	ROM_LOAD( "hq_xea__.2f1", 0x0000, 0x010000, CRC(4ef7c19c) SHA1(bd8e5c69ab31c5a9c89a7f2020a61bde5e00b6b2) )
	ROM_LOAD( "hq_xea__.2s1", 0x0000, 0x010000, CRC(05d42107) SHA1(e1dc4e2f84ebadead994a09c53c2d9a1ed8e29aa) )
	ROM_LOAD( "hq_xes__.2f1", 0x0000, 0x010000, CRC(9bb8ddb1) SHA1(c8bba9bdfd7a0bbcbe89cd422d37909f7ebcaa82) )
	ROM_LOAD( "hq_xes__.2s1", 0x0000, 0x010000, CRC(d09b3d2a) SHA1(4de366ed5eae89f2bb39abc052d88cb59e846307) )
	ROM_LOAD( "hq_xesd_.2a1", 0x0000, 0x010000, CRC(b402aa45) SHA1(6c6eed7b172604112a2c00df9fd00476d07cc971) )
	ROM_LOAD( "hq_xesd_.2f1", 0x0000, 0x010000, CRC(3faf2584) SHA1(01b049b6ae44771f37b298dc525e16b6e9a182f2) )
	ROM_LOAD( "hq_xesd_.2s1", 0x0000, 0x010000, CRC(748cc51f) SHA1(d2ac7c237be40564809b006bf68d09560817f97d) )
	ROM_LOAD( "ph_20a__.2s1", 0x0000, 0x010000, CRC(559bf168) SHA1(3c6a47bba52481af3f987d284a2102a8ee2cc7e6) )
	ROM_LOAD( "ph_20bg_.1_1", 0x0000, 0x010000, CRC(4c96d2fa) SHA1(1ff8c8c5dc6a67ac187ba0b96cd93c786884aa3b) )
	ROM_LOAD( "ph_20bt_.1_1", 0x0000, 0x010000, CRC(eff9ee2c) SHA1(ae15ee4bbc3028580b3c6cbc8a078fbe3291283e) )
	ROM_LOAD( "ph_20s__.1_1", 0x0000, 0x010000, CRC(4cccba32) SHA1(6d2ec2324866555dfa3f3fd5b79ff3883e1b2ebe) )
	ROM_LOAD( "ph_20s__.2s1", 0x0000, 0x010000, CRC(80d4ed45) SHA1(c818f3e154cfa1efcd11efb71061aa5bfc2e668f) )
	ROM_LOAD( "ph_20sb_.1_1", 0x0000, 0x010000, CRC(2ee71a61) SHA1(0e91cc5b22899f9765cabb9416fcafed790951ae) )
	ROM_LOAD( "ph_20sb_.2f1", 0x0000, 0x010000, CRC(1f56841a) SHA1(dfd90d8af765bc981d0dcb4b0ffb5ce613bfcbed) )
	ROM_LOAD( "ph_20sd_.1_1", 0x0000, 0x010000, CRC(e8db4207) SHA1(bd806e5b04207b3121284c485a6ab8a385231504) )
	ROM_LOAD( "ph_20sd_.2s1", 0x0000, 0x010000, CRC(24c31570) SHA1(a6da77ebd80dc234add5da880e3472f2f1e2ca3d) )
	ROM_LOAD( "ph_20sk_.1_1", 0x0000, 0x010000, CRC(8af0e254) SHA1(b7d80ab84684bdabe169623864b9efd0d3881f2e) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "hq______.1_2", 0x000000, 0x080000, CRC(0c8c14be) SHA1(a1268e71ea43772a532b63327f24c64fabd7e715) )
	ROM_LOAD( "hq______.1_3", 0x080000, 0x080000, CRC(5a07e514) SHA1(6e589756c0fc4b0458ca856e918fa3b7cd396c39) )
ROM_END


ROM_START( m4hvhel )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "hh_20__d.2_0", 0x0000, 0x040000, CRC(801de788) SHA1(417b985714d8f0ebed93b65a3f865e03474ce9e5) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "hh_20a_d.2_0", 0x0000, 0x040000, CRC(ea4e7876) SHA1(5bf711c2bdff50fe745edefa0eebf719824d9e5b) )
	ROM_LOAD( "hh_20s_d.2_0", 0x0000, 0x040000, CRC(a519a441) SHA1(f3c19d316c82d1ebbcfdabb6d4eaa6cfa369d287) )
	ROM_LOAD( "hh_sj___", 0x0000, 0x040000, CRC(04577b99) SHA1(48689c3a96bc42ad64dc4d363dad38c967f0cdcc) )
	ROM_LOAD( "hh_sj___.f_0", 0x0000, 0x040000, CRC(8ab33720) SHA1(0c9283a20c3f008baa8ce027d1266e4ef49ca56b) )
	ROM_LOAD( "hh_sjs__.f_0", 0x0000, 0x040000, CRC(8854763d) SHA1(323bd76a014e52e3b12427998b0e2851463246c8) )
	ROM_LOAD( "hh_vc___.g_0", 0x0000, 0x040000, CRC(db338fb7) SHA1(e7e92293374721e7360493e9ef189991dad0a1ee) )
	ROM_LOAD( "hh_vc_d_.g_0", 0x0000, 0x040000, CRC(292468bd) SHA1(f9b19f57a49c1afd670c68b7acd85d4141adfce1) )
	ROM_LOAD( "h_hell._pound5", 0x0000, 0x040000, CRC(cd59c0d0) SHA1(8caad9043a277fa39a3ad2d5ec3388c121e7f697) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "hh___snd.1_1", 0x000000, 0x080000, CRC(afa7ba60) SHA1(25278046252e49364d4a51de79295b87baf6018e) )
	ROM_LOAD( "hh___snd.1_2", 0x080000, 0x080000, CRC(ec1ec822) SHA1(3fdee0526cb70f4951b7bbced74e32641ded9b7b) )
	ROM_LOAD( "hh___snd.1_3", 0x100000, 0x080000, CRC(d4119155) SHA1(b61c71e1ee0dbfc0bb9eff1a8c019cf11731ee11) )
ROM_END


ROM_START( m4holywd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hollywood 5p.bin", 0x0000, 0x010000, CRC(fb4ebb6e) SHA1(1ccfa81c173011ce70640097c85b532fd44f5a6e) )
ROM_END



ROM_START( m4indycr )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ic_sj___.1_0", 0x0000, 0x040000, CRC(4dea0d17) SHA1(4fa19896dbb5e8f21ac7e74efc56de5cadd5bf54) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ic___.g", 0x0000, 0x000657, CRC(25cc2b32) SHA1(a065276fe4cc1e55f366ac216b5bc8cf934da9dd) )
	ROM_LOAD( "ic___.p", 0x0000, 0x00058b, CRC(26388ec4) SHA1(e33183d0c3e75fe23ab6df01d561f5b65923d1db) )
	ROM_LOAD( "ic_sj___.2_0", 0x0000, 0x040000, CRC(6d0ddf54) SHA1(0985aa9fddb71a499d266c12893aabbab8755319) )
	ROM_LOAD( "ic_sj_b_.1_0", 0x0000, 0x040000, CRC(4bc0cb73) SHA1(d4c048ba9578add0104f0c529f20356c3502ea71) )
	ROM_LOAD( "ic_sj_d_.1_0", 0x0000, 0x040000, CRC(165ad977) SHA1(daa444e0d128859832094d3b07026483cd3466ce) )
	ROM_LOAD( "ic_sj_d_.2_0", 0x0000, 0x040000, CRC(36bd0b34) SHA1(306d6e6536a4137353f9b895e64c7e9a5c79561a) )
	ROM_LOAD( "ic_sj_k_.1_0", 0x0000, 0x040000, CRC(857fda64) SHA1(3eb230ea1adf9acb4cf83422c4bb1cde40756310) )
	ROM_LOAD( "ic_sjs__.1_0", 0x0000, 0x040000, CRC(6310b904) SHA1(0f2cd7ed83f77423bcfb2a71144fab2047dfea13) )
	ROM_LOAD( "icsj___.1_0", 0x0000, 0x01215c, CRC(a8217e6c) SHA1(df419dc92196f1b7cd3c473df05d3b1429aecb05) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "da______.1_1", 0x000000, 0x080000, CRC(99d36c12) SHA1(f8848a28b5546649d6a3f8599dbc4ca84bdac77c) )
	ROM_LOAD( "da______.1_2", 0x080000, 0x080000, CRC(32b40094) SHA1(f02c3b088d76116f817b536cf7cec5188b2f73bf) )
	ROM_LOAD( "da______.1_3", 0x100000, 0x080000, CRC(2df33d18) SHA1(40afa32d6c72c6a76e3e2e61db19a16003f4e176) )
	ROM_LOAD( "da______.1_4", 0x180000, 0x080000, CRC(8e254a3b) SHA1(bc3643ea5878bbde110ee6971c5149b3320bcffc) )
ROM_END


ROM_START( m4jakjok )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "jj_sj___.6_0", 0x0000, 0x040000, CRC(7bc45b0e) SHA1(f30fef8fccdac04859f1ff93198a497eff723020) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "jj_sj_k_.3_0", 0x0000, 0x040000, CRC(c33dd82f) SHA1(c1f3f6ca1c45503b7f71e897e5c27368f5efb439) )
	ROM_LOAD( "jj_sjs__.6_0", 0x0000, 0x040000, CRC(4bcac6f5) SHA1(7dc07a7a61a6ba044020d6c2496143168c103a70) )
	ROM_LOAD( "jj_vc___.7_0", 0x0000, 0x040000, CRC(4cdca8da) SHA1(ee7448b12380416a3bea2713ed5feca7473be8aa) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "jj_____.1_1", 0x000000, 0x080000, CRC(e759a958) SHA1(b107f4ef5a2805e56d4024940bfc632155de1eb1) )
	ROM_LOAD( "jj_____.1_2", 0x080000, 0x080000, CRC(aa215ff0) SHA1(4bf2c6f8153730cc3ca86f78ec14063ece7d8700) )
	ROM_LOAD( "jj_____.1_3", 0x100000, 0x080000, CRC(03c0ffc3) SHA1(2572f62362325df8b235b487d4a764218e7f1589) )
ROM_END

ROM_START( m4jakjoka )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "j_joker", 0x0000, 0x040000, CRC(4f0c7ab8) SHA1(af962863ee55f6c2752bbe8a997e3b2102e42431) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "j_joker.s1", 0x0000, 0x080000, CRC(4ad711d2) SHA1(500381ac2a5075acd606a131cd1b382342cc3a80) )
	ROM_LOAD( "j_joker.s2", 0x0000, 0x080000, CRC(840ba491) SHA1(f7f43d5d8e521a59fdccbd5f22935c525c3d43c2) )
	ROM_LOAD( "j_joker.s3", 0x0000, 0x080000, CRC(2ed74890) SHA1(a3d039b4c3c9dd792300eb045e542a212d4d50ae) )
ROM_END

ROM_START( m4jflash )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "jf_25__c.2_1", 0x0000, 0x020000, CRC(4d5f1a12) SHA1(c25b6d899b74231da505bde7b671be001bdcea5d) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "jf_25a_c.2_1", 0x0000, 0x020000, CRC(76722e15) SHA1(4bd107049ad98b848cdaba3a1318373bbd06ab9f) )
	ROM_LOAD( "jf_25b_c.2_1", 0x0000, 0x020000, CRC(35a927c6) SHA1(6776fe77ad8a85feecdedfad0eac89f9cb826fbf) )
	ROM_LOAD( "jf_25bdc.2_1", 0x0000, 0x020000, CRC(d372689e) SHA1(ecad53022c7786f387586484a3e679afbf0bac37) )
	ROM_LOAD( "jf_25bgc.2_1", 0x0000, 0x020000, CRC(421084bc) SHA1(b6f847468c20a3f85d9e77b633dc48adb43c970f) )
	ROM_LOAD( "jf_25btc.2_1", 0x0000, 0x020000, CRC(1c701822) SHA1(559b4d269a3af388aa86f28ccedd22505fcdb355) )
	ROM_LOAD( "jf_25d_c.2_1", 0x0000, 0x020000, CRC(9cde7bfc) SHA1(a0b840f00c487e963f4dd9f58e3abca7b0cea31b) )
	ROM_LOAD( "jf_25dkc.2_1", 0x0000, 0x020000, CRC(ef583053) SHA1(23b33f0e49d8efceb4a6690ac58da1ccf6576a1a) )
	ROM_LOAD( "jf_25k_c.2_1", 0x0000, 0x020000, CRC(d82701b8) SHA1(a784fa11877e05f2219f1452463ec3348c84e879) )
	ROM_LOAD( "jf_25sbc.2_1", 0x0000, 0x020000, CRC(c6bce1c6) SHA1(708002059d307a02fec32a3cdb6eff995a438631) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4kingq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ee_05a_4.2_1", 0x0000, 0x010000, CRC(8dd842b6) SHA1(1c1bcaae355ceee4d7b1572b0fa1a8b23a8afdbf) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ee_05a__.2_1", 0x0000, 0x010000, CRC(36aa5fb9) SHA1(b4aaf647713e33e79be7927e5eeef240d3beedf7) )
	ROM_LOAD( "ee_20a__.2_1", 0x0000, 0x010000, CRC(2c61341f) SHA1(76d68ae2a44087414be8be12b3824c62311721dd) )
	ROM_LOAD( "ee_20a_c.1_1", 0x0000, 0x010000, CRC(948140ac) SHA1(d43f1f2903ecd809dee191087fa075c638728a5b) )
	ROM_LOAD( "ee_20b__.2_1", 0x0000, 0x010000, CRC(2fc7c7c2) SHA1(3b8736a582009d7b1455769374342ff72026d2fa) )
	ROM_LOAD( "ee_20b_c.1_1", 0x0000, 0x010000, CRC(70d399ab) SHA1(ca2c593151f4f852c7cb66859a12e832e53cd31f) )
	ROM_LOAD( "ee_20bd_.2_1", 0x0000, 0x010000, CRC(239de2dd) SHA1(c8021ba5bfdc10f59fec27c364035225093328d8) )
	ROM_LOAD( "ee_20bdc.1_1", 0x0000, 0x010000, CRC(cbb8c57b) SHA1(ea165199213f95128aec95ae40799faa8c457dd3) )
	ROM_LOAD( "ee_20bg_.2_1", 0x0000, 0x010000, CRC(ddc4d832) SHA1(031f987e9fced1df4acc57eb4b60911d52e1dbf6) )
	ROM_LOAD( "ee_20bt_.2_1", 0x0000, 0x010000, CRC(6f278771) SHA1(4459c9490be14bcbc139eebe6542325c80937ff3) )
	ROM_LOAD( "ee_20s_c.1_1", 0x0000, 0x010000, CRC(a0c1e313) SHA1(8a088a33e51a31ff0abdb554aa4d8ce61eaf4b7d) )
	ROM_LOAD( "ee_20sb_.2_1", 0x0000, 0x010000, CRC(307ad157) SHA1(32b6187e907bfbdb87a9ad2d9ca5870b09de5e4a) )
	ROM_LOAD( "ee_25a_c.3_1", 0x0000, 0x010000, CRC(4dc25083) SHA1(b754b4003f73bd74d1670a36a70985ce5e48794d) )
	ROM_LOAD( "ee_25b_c.3_1", 0x0000, 0x010000, CRC(a6fe50ff) SHA1(011602d9624f232ba8484e57f5f33ff06091809f) )
	ROM_LOAD( "ee_25bdc.3_1", 0x0000, 0x010000, CRC(d0088a97) SHA1(aacc1a86bd4b321d0ee21d14147e1d135b3a5bae) )
	ROM_LOAD( "ee_25bgc.3_1", 0x0000, 0x010000, CRC(e4dcd86b) SHA1(b8f8ec317bf9f18e3d0ae9a9fd59349fee24530d) )
	ROM_LOAD( "ee_25btc.3_1", 0x0000, 0x010000, CRC(8f44347a) SHA1(09815a6e1d3a91cd2e69578bbcfef3203ddb33d6) )
	ROM_LOAD( "ee_25s_c.3_1", 0x0000, 0x010000, CRC(a6fe50ff) SHA1(011602d9624f232ba8484e57f5f33ff06091809f) )
	ROM_LOAD( "ee_25sbc.3_1", 0x0000, 0x010000, CRC(0f4bdd7c) SHA1(5c5cb3a9d6a96afc6e29149d2a8adf19aae0bc41) )
	ROM_LOAD( "eei20___.2_1", 0x0000, 0x010000, CRC(15f4b869) SHA1(5be6f660321cb47900dda986ef44eb5c1c324013) )
	ROM_LOAD( "knq2pprog.bin", 0x0000, 0x010000, CRC(23b22f79) SHA1(3d8b9cbffb9b427897548981ddacf724215336a4) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "kingsnqueenssnd.bin", 0x0000, 0x080000, CRC(31d722d4) SHA1(efb7079a1036cad8d9c08106f97c70a248b31898) )
	//ROM_LOAD( "knqsnd.bin", 0x0000, 0x080000, CRC(13012f48) SHA1(392b3bcf6f8e3e01082087637f9d378302d046c4) )
	ROM_LOAD( "ee______.1_2", 0x0000, 0x080000, CRC(13012f48) SHA1(392b3bcf6f8e3e01082087637f9d378302d046c4) )
ROM_END


ROM_START( m4kingqc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cn_20_b4.6_1", 0x0000, 0x010000, CRC(22d0b20c) SHA1(a7a4f60017cf62247339c9b23420d29845657895) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cn_20_bc.3_1", 0x0000, 0x010000, CRC(dfb0eb80) SHA1(ad973125681db0aae8ef1cf57b1c280e7f0e5803) )
	ROM_LOAD( "cn_20_dc.3_1", 0x0000, 0x010000, CRC(56e919ad) SHA1(c3c6f522574b287f7ed4dc4d1d8a32f68369dd5c) )
	ROM_LOAD( "cn_20a_c.2_1", 0x0000, 0x010000, CRC(0df15fd9) SHA1(e7c5e2277aac1c71d27710ea71d09d1005c5b8f9) )
	ROM_LOAD( "cn_20a_c.3_1", 0x0000, 0x010000, CRC(68e1dcef) SHA1(a0b15344b900226052633703e935c0ec0f718936) )
	ROM_LOAD( "cn_20a_c.5_1", 0x0000, 0x010000, CRC(4975d39e) SHA1(243b8af1a12c3538e826bfd5f6feb6927c1467b0) )
	ROM_LOAD( "cn_20b_c.2_1", 0x0000, 0x010000, CRC(66e074f3) SHA1(1f5381a41dd1402ee344e228635b35521e9377c8) )
	ROM_LOAD( "cn_20b_c.3_1", 0x0000, 0x010000, CRC(bcae86c9) SHA1(f03b136d82fe7b93350f0ca5dc36e78e98aecfa9) )
	ROM_LOAD( "cn_20b_c.5_1", 0x0000, 0x010000, CRC(6a19d734) SHA1(e0d5f5020e7997d3927b42336ab18757bd9f1ed0) )
	ROM_LOAD( "cn_20bg4.6_1", 0x0000, 0x010000, CRC(6d4158fe) SHA1(9c12264a415601d6f28f23c1e1f6a3d97fadddba) )
	ROM_LOAD( "cn_20bgc.3_1", 0x0000, 0x010000, CRC(3ecc1bf3) SHA1(fb191749f920aa4ac0d9809c6c59b695afdf6594) )
	ROM_LOAD( "cn_20bgc.5_1", 0x0000, 0x010000, CRC(24743f7e) SHA1(c90d95df2357bc00aba2bb21c0c77082b8c32463) )
	ROM_LOAD( "cn_20btc.2_1", 0x0000, 0x010000, CRC(b8f0ade0) SHA1(5b5344f799b27833f6456ae852eb5085afb3dbe5) )
	ROM_LOAD( "cn_20btc.3_1", 0x0000, 0x010000, CRC(b92f3787) SHA1(7efd815cf1a9a738ffae2c3ce19149f47d465c72) )
	ROM_LOAD( "cn_20btc.5_1", 0x0000, 0x010000, CRC(e2b8baf0) SHA1(23e966a6cc94c26903bfe943160a327529d7e21b) )
	ROM_LOAD( "cn_20s_c.2_1", 0x0000, 0x010000, CRC(66e074f3) SHA1(1f5381a41dd1402ee344e228635b35521e9377c8) )
	ROM_LOAD( "cn_20s_c.5_1", 0x0000, 0x010000, CRC(6a19d734) SHA1(e0d5f5020e7997d3927b42336ab18757bd9f1ed0) )
	ROM_LOAD( "cn_20sbc.5_1", 0x0000, 0x007e02, CRC(a20d7293) SHA1(2e83b73907b867aec20edacb9b49250f746faa3e) )
	ROM_LOAD( "cn_25_bc.2_1", 0x0000, 0x010000, CRC(3e2b2d7b) SHA1(9a68cf4902ca210e8fb52a35b4c507708c7f6d2a) )
	ROM_LOAD( "cn_25_dc.2_1", 0x0000, 0x010000, CRC(eb384ef6) SHA1(489c59d8e1e6296ec2b05fb0aa307c48f3486aa2) )
	ROM_LOAD( "cn_25_kc.2_1", 0x0000, 0x010000, CRC(bd11e742) SHA1(0c3b290e3010bc3f904f9087ee89efe63072b8c3) )
	ROM_LOAD( "cn_25a_c.2_1", 0x0000, 0x010000, CRC(1994efd5) SHA1(d7c3d692737138b30244d2d51eb535b88c87e401) )
	ROM_LOAD( "cn_25b_c.2_1", 0x0000, 0x010000, CRC(24255989) SHA1(017d0dc811b5c82d5b8785022169929c94f3f18a) )
	ROM_LOAD( "cn_25bdc.2_1", 0x0000, 0x010000, CRC(503ccd3c) SHA1(936b77b33373624e6bd80d168bcd48dc2ebcb2fe) )
	ROM_LOAD( "cn_25bgc.2a1", 0x0000, 0x010000, CRC(89e2130d) SHA1(22f97030e6f4cb94f62215a0c653d170ba3e0efd) )
	ROM_LOAD( "cn_25btc.2_1", 0x0000, 0x010000, CRC(876bc126) SHA1(debe36a082493cdeba26a0808f205a19e9e897d5) )
	ROM_LOAD( "cn_25s_c.1_1", 0x0000, 0x010000, CRC(84d1a32b) SHA1(f6e76a2bf1bd7b31eb360dea8b453d235c365e64) )
	ROM_LOAD( "cn_25sbc.1_1", 0x0000, 0x010000, CRC(e53b672c) SHA1(2aea2a243817857df31b6f7b767e380bd003fafa) )
	ROM_LOAD( "cn_30_dc.1_1", 0x0000, 0x010000, CRC(aeb21904) SHA1(32bd505e738b8826c6ab138f30831b7a53b700cf) )
	ROM_LOAD( "cn_30a_c.1_1", 0x0000, 0x010000, CRC(be7aed91) SHA1(7dac1281bbc9da8924657b13ec4aa86aa6ff9de4) )
	ROM_LOAD( "cn_30b_c.1_1", 0x0000, 0x010000, CRC(232c87ec) SHA1(2c2bf1c273ab88c0ab27a672d53cd6184a24a8d1) )
	ROM_LOAD( "cn_30bgc.1_1", 0x0000, 0x010000, CRC(40afaa86) SHA1(edb8f55abf66e3e1cc7e353c520a93fc42073585) )
	ROM_LOAD( "cn_30btc.1_1", 0x0000, 0x010000, CRC(1920cc67) SHA1(55a3ad78d68d635faff98390e2feeea29dd10664) )
	ROM_LOAD( "knq2pprog.bin", 0x0000, 0x010000, CRC(23b22f79) SHA1(3d8b9cbffb9b427897548981ddacf724215336a4) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "knqsnd.bin", 0x0000, 0x080000, CRC(13012f48) SHA1(392b3bcf6f8e3e01082087637f9d378302d046c4) )
	ROM_LOAD( "cn______.5_a", 0x0000, 0x080000, CRC(7f82f113) SHA1(98851f8820cb39b45d477151982c80fc91b15e56) )
ROM_END


ROM_START( m4lazy )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "lb_sj___.1_0", 0x0000, 0x020000, CRC(8628dcf1) SHA1(80cb9348e2704d0f72a44b4aa74b24fe03e279bc) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "lb_sj___.1_2", 0x0000, 0x020000, CRC(2b906f52) SHA1(802bcf6b3679e135308026752a55e55f00f21e85) )
	ROM_LOAD( "lb_sj_d_.1_2", 0x0000, 0x020000, CRC(a7691bad) SHA1(6cda3f3c18c13c04dbe0e4c1e4c817eedc34aa92) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4lvlcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ll__x__x.1_1", 0x0000, 0x010000, CRC(1ef1c5b4) SHA1(455c147f158f8a36a9add9b984abc22af78258cf) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ll__x__x.3_1", 0x0000, 0x010000, CRC(42b85ebc) SHA1(a352d8389674fcfd90dc4e8155e6f4a78c9ec70d) )
	ROM_LOAD( "ll__x_dx.3_1", 0x0000, 0x010000, CRC(7753c8f0) SHA1(9600fee08529f29716697c4630730f15ef8a457b) )
	ROM_LOAD( "ll__xa_x.3_1", 0x0000, 0x010000, CRC(79468e93) SHA1(4beaa6fe2ad095b4674473ab99a7216513923077) )
	ROM_LOAD( "ll__xb_x.3_1", 0x0000, 0x010000, CRC(73b2fb34) SHA1(c127bc0954f8d01e9d8365a4506dde6f17da33fd) )
	ROM_LOAD( "ll__xgdx.1_1", 0x0000, 0x010000, CRC(65824c4f) SHA1(a514e48ac0f9d4a8d7506bf6932aeee88ca17104) )
	ROM_LOAD( "ll__xgdx.3_1", 0x0000, 0x010000, CRC(ba5b951a) SHA1(9ee36d3d42ce68f5797208633be87ddbbe605cf1) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END



ROM_START( m4ln7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "l7_20a__.1_1", 0x0000, 0x010000, CRC(bfe82d2a) SHA1(4477d737a2326602a355758d8fc06220312fc085) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "l7_20s__.1_1", 0x0000, 0x010000, CRC(0037cd57) SHA1(b5882027269cf71878a73009bc3e40d9fcfac60d) )
	ROM_LOAD( "l7_20sb_.1_1", 0x0000, 0x010000, CRC(d53bcd66) SHA1(7f5c65d5ca3dbb8a0c38f169585fc78d512166af) )
	ROM_LOAD( "l7_20sd_.1_1", 0x0000, 0x010000, CRC(a82d04cb) SHA1(1abe9e22f6f526ab076163ce79cff841d1b38b0a) )
	ROM_LOAD( "l7_20sk_.1_1", 0x0000, 0x010000, CRC(7d2104fa) SHA1(a0a65042f4db8ea5184d41c68ffcf7608580d928) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "l7______.1_2", 0x000000, 0x080000, CRC(216209e3) SHA1(af274a7f27ba0e7ac03400e9919537ab36464e64) )
	ROM_LOAD( "l7______.1_3", 0x080000, 0x080000, CRC(e909c3ec) SHA1(68ce743729aaefd6c20ee447af40d99e0f4c072b) )
ROM_END




ROM_START( m4madmon )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "mm_20a__.7_1", 0x0000, 0x020000, CRC(7df66388) SHA1(4e5bcbcb2fb08b23989c83f11751400f666bbdc2) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "mm_20b__.7_1", 0x0000, 0x020000, CRC(7f592e44) SHA1(05e78347cd09d1e58f0a50a724e0563490ec5185) )
	ROM_LOAD( "mm_20bg_.7_1", 0x0000, 0x020000, CRC(2cd8dcc2) SHA1(c4a2a423a55c6b0668739429c24c69b25e3824cf) )
	ROM_LOAD( "mm_20bt_.7_1", 0x0000, 0x020000, CRC(6e929612) SHA1(6b1d06c3cfce440c4cefb8dd04e7a5da673d545a) )
	ROM_LOAD( "mm_20s__.7_1", 0x0000, 0x020000, CRC(c23b338c) SHA1(0d3bc801132e68564c2bc01b810e71362047ade1) )
	ROM_LOAD( "mm_20sb_.7_1", 0x0000, 0x020000, CRC(51be206b) SHA1(913defb24bdb7551acefefd9673f5663129edbec) )
	ROM_LOAD( "mm_25__c.3_1", 0x0000, 0x020000, CRC(e1879caf) SHA1(4d0a804a8d81aab5bb9dec611654325e6a3fb741) )
	ROM_LOAD( "mm_25_dc.3_1", 0x0000, 0x020000, CRC(9bc81854) SHA1(b4c42a3da0ab03a0f43846e0c3a4a0b5f3c7e65a) )
	ROM_LOAD( "mm_25_gc.3_1", 0x0000, 0x020000, CRC(8d2f1259) SHA1(239c2430bd4ce7b53615b00fac79fb7eceecabf1) )
	ROM_LOAD( "mm_25a_c.3_1", 0x0000, 0x020000, CRC(b45b88c2) SHA1(56ed8c83c68f410fcc5ac342abac6b1f4419cccd) )
	ROM_LOAD( "mm_25b_c.3_1", 0x0000, 0x020000, CRC(fc33804b) SHA1(f817a6dd691739fcf7d7d622da265f63f60503f6) )
	ROM_LOAD( "mm_25bdc.3_1", 0x0000, 0x020000, CRC(188666c0) SHA1(e8a61c327c73aac2a6b0dc674dee7bc2aa358b27) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4madmnc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cm_25__c.3_1", 0x0000, 0x020000, CRC(3d9ff5fe) SHA1(b918bb15251514f50a669216c7d00ecf23e64d1b) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cm_25_bc.3_1", 0x0000, 0x020000, CRC(65a7b870) SHA1(58b910d7e002164cbbe1aa32c5e17dfe7cfb507d) )
	ROM_LOAD( "cm_25_dc.3_1", 0x0000, 0x020000, CRC(fcae8cf3) SHA1(0f1e86e2f02be2e1870f0f70509bc4a2ada6d3a5) )
	ROM_LOAD( "cm_25_kc.3_1", 0x0000, 0x020000, CRC(f66cf97b) SHA1(9b6c4da3a9d64ed2581b04ca82a945dc295931bc) )
	ROM_LOAD( "cm_25a_c.3_1", 0x0000, 0x020000, CRC(94e21dc0) SHA1(0d35a467fdcd19909d5540c8a4461364bf7e17f3) )
	ROM_LOAD( "cm_25b_c.3_1", 0x0000, 0x020000, CRC(95a1eb43) SHA1(a44d5c8fadf547187834c4686f6001e6e7df83f7) )
	ROM_LOAD( "cm_25bgc.3_1", 0x0000, 0x020000, CRC(3241cdf5) SHA1(9ac4b415fff3ee3422dc6e4df7f626cddcab6c38) )
	ROM_LOAD( "cm_25btc.3_1", 0x0000, 0x020000, CRC(b8c0f623) SHA1(62d489d82955ac492f78be77a3f2558ad080b375) )
	ROM_LOAD( "cm_29_dc.4_1", 0x0000, 0x020000, CRC(72542e93) SHA1(f9aad1e290b345ebd93dee81a10ad3ebb61b8228) )
	ROM_LOAD( "cm_29a_c.4_1", 0x0000, 0x020000, CRC(aa7c11ce) SHA1(3ae11b5279f975c0f6f6462d9e23180b47cbe280) )
	ROM_LOAD( "cm_29b_c.4_1", 0x0000, 0x020000, CRC(b5d749d8) SHA1(91f318a193a7ad841ac9a7f2114385ad97555f4d) )
	ROM_LOAD( "cm_29bgc.4_1", 0x0000, 0x020000, CRC(82f06ed1) SHA1(3f982d8fb1f689cd774f407585a8dce5d8c031a7) )
	ROM_LOAD( "cm_29btc.4_1", 0x0000, 0x020000, CRC(dd96bbeb) SHA1(1cf461dc36a8086b3f438922e1675fbd929af771) )
	ROM_LOAD( "cm_39_dc.4_1", 0x0000, 0x020000, CRC(c81c575e) SHA1(8c1f5151412da267381434259ca2c2d307668a74) )
	ROM_LOAD( "cm_39a_c.4_1", 0x0000, 0x020000, CRC(10346803) SHA1(2d7b0ca2c30a24c4779c44872fc86d6ae269b51f) )
	ROM_LOAD( "cm_39a_c.5_1", 0x0000, 0x020000, CRC(a235932f) SHA1(b6875c1119c8ed3a77d7f358298c119afac17dea) )
	ROM_LOAD( "cm_39b_c.4_1", 0x0000, 0x020000, CRC(0f9f3015) SHA1(8a02b6a22a96c7e310a7428d36b306e8af618e9a) )
	ROM_LOAD( "cm_39bgc.4_1", 0x0000, 0x020000, CRC(38b8171c) SHA1(5e33189d6b2d72a045c98c2bb8492961ab3d978f) )
	ROM_LOAD( "cm_39btc.4_1", 0x0000, 0x020000, CRC(67dec226) SHA1(68bfe25ff693ea7b4469d2e7d1b6972cc421d1cc) )
	ROM_LOAD( "cm_49_dc.4_1", 0x0000, 0x020000, CRC(e0ab38ee) SHA1(a324a1bb9f709b72de6fdd4c896326ca0004fdef) )
	ROM_LOAD( "cm_49a_c.4_1", 0x0000, 0x020000, CRC(388307b3) SHA1(070d431171ce325f6935f5bc1fd8db3acf79b13c) )
	ROM_LOAD( "cm_49b_c.4_1", 0x0000, 0x020000, CRC(27285fa5) SHA1(5f08bc751d7a0875b1b879d7d96aa9803fb942c6) )
	ROM_LOAD( "cm_49bgc.4_1", 0x0000, 0x020000, CRC(100f78ac) SHA1(c5d30fa46508b00b163dbf05e572f8c23fdb6cc3) )
	ROM_LOAD( "cm_49btc.4_1", 0x0000, 0x020000, CRC(4f69ad96) SHA1(50948166fee3cd1e6f0e378076046ee305204d61) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END



ROM_START( m4mmm )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mu_sj___.3_0", 0x0000, 0x040000, CRC(abdf9d1c) SHA1(e8c6a056025b44e4ec995b42b2720e6366a97283) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "mu_sja__.3_0", 0x0000, 0x040000, CRC(3d2a9ea4) SHA1(f2ec904c8cef84affaad603edf26a864bd34be29) )
	ROM_LOAD( "mu_sjk__.3_0", 0x0000, 0x040000, CRC(34e4f8ba) SHA1(606d607faeb43190f5167aa3d10c55d9986b7e58) )
	ROM_LOAD( "mu_sjs__.3_0", 0x0000, 0x040000, CRC(26fb12b3) SHA1(d341181be75c87b44e4066653225911ce3460ed8) )
	ROM_LOAD( "mu_ssj__.2_0", 0x0000, 0x040000, CRC(935b6602) SHA1(d5fa5688895fe3c2ae3ad7dbbc35d9b12574c93d) )
	ROM_LOAD( "mu_ssja_.2_0", 0x0000, 0x040000, CRC(ff97814c) SHA1(8d9d74e6b0096cdc3226cfa91d7b653855600d5a) )
	ROM_LOAD( "mu_ssjb_.2_0", 0x0000, 0x040000, CRC(5728973a) SHA1(2cd9c866fcc33150fb8d456f741ac809e0bd2b15) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "mu___snd.1_1", 0x000000, 0x080000, CRC(570cf5f8) SHA1(48b3703bf385d037e4e870dfb671b75e9bab84a7) )
	ROM_LOAD( "mu___snd.1_2", 0x080000, 0x080000, CRC(6ec1910b) SHA1(4920fe0b7c7f4ddb14d56f50598aaf62e5867014) )
	ROM_LOAD( "mu___snd.1_3", 0x100000, 0x080000, CRC(8699378c) SHA1(55c3e310cfde8046e58bf21a8788e697c8275b8d) )
	ROM_LOAD( "mu___snd.1_4", 0x180000, 0x080000, CRC(54b193d8) SHA1(ab24624ce69d352a14f6bd3db127fa1c8c5f07db) )

ROM_END



ROM_START( m4oadrac )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dr__x__x.2_0", 0x0000, 0x020000, CRC(4ca65bd9) SHA1(deb0a7d3596647210061b69a10fc6cdfc066538e) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "dr__x__x.2_1", 0x0000, 0x020000, CRC(d91773af) SHA1(3d8dda0f409f55bce9c4d4e2a8377e43fe2f1f7d) )
	ROM_LOAD( "dr__x_dx.2_0", 0x0000, 0x020000, CRC(47f3ac5a) SHA1(e0413c55b897e96e32c3332dac041bc94da6dea3) )
	ROM_LOAD( "dr__x_dx.2_1", 0x0000, 0x020000, CRC(f8c36b67) SHA1(c765d7a5eb4d7cd74295da26a7c6f5341a1ca257) )
	ROM_LOAD( "dr__xa_x.2_0", 0x0000, 0x020000, CRC(702f0f7a) SHA1(8529c3eaa33cb972cc38067d176c7c8af0674147) )
	ROM_LOAD( "dr__xa_x.2_1", 0x0000, 0x020000, CRC(cf1fc847) SHA1(6b09c0de15a380da1783a387569d83328f5b29a0) )
	ROM_LOAD( "dr__xb_x.2_0", 0x0000, 0x020000, CRC(3ae8a72c) SHA1(a27faba69430b1d16abf62e0ef37182ab302bbbd) )
	ROM_LOAD( "dr__xb_x.2_1", 0x0000, 0x020000, CRC(85d86011) SHA1(81f8624908299aa37e75fc5d12059b3600212d35) )
	ROM_LOAD( "dri_xa_x.2_0", 0x0000, 0x020000, CRC(849d2a80) SHA1(c9ff0a5a543b62ca5b885f93a35b5f40e88db8c3) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "oad.chr", 0x0000, 0x000048, CRC(910b09db) SHA1(d54399660b1bf1a89712b25292ac99b740442e5c) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "oadsnd1.bin", 0x000000, 0x080000, CRC(b9a9b49b) SHA1(261e939da031768e2a2b5b171cbba55c87d1a758) )
	ROM_LOAD( "oadsnd2.bin", 0x080000, 0x080000, CRC(94e34646) SHA1(8787d6757e4ed86417aafac0e042091189974d3b) )
ROM_END

ROM_START( m4orland )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "or 05a v2-1(27c010)", 0x0000, 0x020000, CRC(a33c22ee) SHA1(3598a2940f05622405fdef16426f3f5f30dfef29) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "or_05a__.1_1", 0x0000, 0x020000, CRC(3e7fe3ac) SHA1(9f4c0d5b7ba10726376b0654c8ddbc62b62c9eed) )
	ROM_LOAD( "or_20a__.7_1", 0x0000, 0x020000, CRC(ae524299) SHA1(3bb2bfe1c0ca0660aca148d6f17b730b7bdc8183) )
	ROM_LOAD( "or_20b__.7_1", 0x0000, 0x020000, CRC(c8a30e2e) SHA1(8abc5437751faf61c12709c963a6819cb0b2b43f) )
	ROM_LOAD( "or_20bg_.7_1", 0x0000, 0x020000, CRC(552561e5) SHA1(c92c4b21182511e4880b90313a673045e20b01e8) )
	ROM_LOAD( "or_20bt_.7_1", 0x0000, 0x020000, CRC(7db2e12a) SHA1(c10989d5d6bbc8f87ad21364f6c64495dc4a3047) )
	ROM_LOAD( "or_20s__.7_1", 0x0000, 0x020000, CRC(9ce4b650) SHA1(26a3337526d398ce265d735cfbe6d0e69c1f5cab) )
	ROM_LOAD( "or_20sb_.7_1", 0x0000, 0x020000, CRC(90f59de7) SHA1(2bb6c0680c654265c8669a5f13346ae6afb72fb5) )
	ROM_LOAD( "or_20sd_.7_1", 0x0000, 0x020000, CRC(242c552c) SHA1(dce5f0d38c8c6c101337028f30c05a7eb629e703) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "orlandosnd.p1", 0x000000, 0x080000, CRC(2735649d) SHA1(7b27bf2d4091ab581d399679b03f538f449f180c) )
	ROM_LOAD( "orlandosnd.p2", 0x080000, 0x080000, CRC(0741e2ff) SHA1(c49a2809073dd058ba85ae14c888e19d3eb2b133) )
ROM_END


ROM_START( m4pzbing )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pb_20a__.4_1", 0x0000, 0x010000, CRC(52aa92e5) SHA1(3dc20e521677e829967e1d689c9905fb96aee639) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "pb_20ad_.4_1", 0x0000, 0x010000, CRC(f6bd6ad0) SHA1(092cb895c576ed2e995b62aba21851af6fb90959) )
	ROM_LOAD( "pb_20bg_.4_1", 0x0000, 0x010000, CRC(593e89f4) SHA1(4ed79c889370eb5de20b434cd83b2ee3fae31ed8) )
	ROM_LOAD( "pb_20bt_.4_1", 0x0000, 0x010000, CRC(fa51b522) SHA1(1fdebe63b871f700a664053251b02c8cf47fbb92) )
	ROM_LOAD( "pb_20s__.4_1", 0x0000, 0x010000, CRC(601f37d9) SHA1(0971c5c5321f5ce1508a0dd8abd989939224a779) )
	ROM_LOAD( "pb_20sb_.4_1", 0x0000, 0x010000, CRC(0234978a) SHA1(f24069883efb69de1024b6efbeb3d6a100ac5b9a) )
ROM_END


ROM_START( m4quidin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qi_20a__.3_1", 0x0000, 0x010000, CRC(88873c45) SHA1(70fa65402dbbe716a089497a8ccb06e0ba2aac6d) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "qi_20s__.3_1", 0x0000, 0x010000, CRC(3758dc38) SHA1(d22a379975e948d465e13233a796e0fb07e3c04f) )
	ROM_LOAD( "qi_20sb_.3_1", 0x0000, 0x010000, CRC(e254dc09) SHA1(ad5853c854f628de6203be8d6c3cbaa6a600e340) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "qi_____.1_2", 0x000000, 0x080000, CRC(216209e3) SHA1(af274a7f27ba0e7ac03400e9919537ab36464e64) )
	ROM_LOAD( "qi_____.1_3", 0x080000, 0x080000, CRC(e909c3ec) SHA1(68ce743729aaefd6c20ee447af40d99e0f4c072b) )
ROM_END


ROM_START( m4quidis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pq_20a__.3_1", 0x0000, 0x010000, CRC(7eb762a1) SHA1(4546a7bf43f8ab6eb9713348e3f919de7532eed2) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "pq_20s__.3_1", 0x0000, 0x010000, CRC(71360992) SHA1(0b64f27f0edfdebca41552181ff0f2b5491ec308) )
	ROM_LOAD( "pq_20sb_.3_1", 0x0000, 0x010000, CRC(a43a09a3) SHA1(46d83465d1026620af2f59dd4b638444ca834ad1) )
	ROM_LOAD( "pq_20sd_.3_1", 0x0000, 0x010000, CRC(d92cc00e) SHA1(bff2b5da08cc34040b1d4d750ea6a654f9b77959) )
	ROM_LOAD( "pq_20sk_.3_1", 0x0000, 0x010000, CRC(0c20c03f) SHA1(f802daa8ff2c159ba4831ed048e0ddd8469448da) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4rackem )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "re_sj___.2_0", 0x0000, 0x040000, CRC(e36d3f86) SHA1(a5f522c86482517b8dc735b1012f8f7668c2f18d) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "re_sj___.3_0", 0x0000, 0x040000, CRC(2f463d2f) SHA1(3410cc8a6d097a4edfcb4c57c237d1d514b507ba) )
	ROM_LOAD( "re_sj_d_.2_0", 0x0000, 0x040000, CRC(7a31658c) SHA1(4fade421b3a1a732a99f7cb6346279ad82f55362) )
	ROM_LOAD( "re_sjs__.2_0", 0x0000, 0x040000, CRC(d7c499c8) SHA1(73542f54322f5ffb87d16f5f66cc3a22c2849f20) )
	ROM_LOAD( "re_sjsw_.2_0", 0x0000, 0x040000, CRC(66355370) SHA1(d54aab7403e64a67edf2baeaf1321ee5c4aa553d) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "re_snd.p1", 0x000000, 0x080000, CRC(aea88892) SHA1(457dab5cddfb9762f7e0bd61187b8052aee71c28) )
	ROM_LOAD( "re_snd.p2", 0x080000, 0x080000, CRC(57394ec6) SHA1(cba7abebd3ab165e9531017168f51ada6cf35991) )
	ROM_LOAD( "re_snd.p3", 0x100000, 0x080000, CRC(5d5d5309) SHA1(402615633976410225a1ee50c454391dc69a68cb) )
ROM_END


ROM_START( m4rbgold )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rb_20a_p.2a1", 0x0000, 0x010000, CRC(d7e6e514) SHA1(25645b69e86335622df43113908ed88a21f27e30) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "rb_20a_p.2f1", 0x0000, 0x010000, CRC(62af6db6) SHA1(0dcb679c05f090f8dab7228009a700c31f0179d8) )
	ROM_LOAD( "rb_20sbp.2a1", 0x0000, 0x010000, CRC(ba4c2e74) SHA1(fcc325754f96e742998373c6c5c13a8509f48cd5) )
	ROM_LOAD( "rb_20sbp.2f1", 0x0000, 0x010000, CRC(0f05a6d6) SHA1(1ba7ccbb3d78196d4b98a0a1173bf41c9dd62c1f) )
	ROM_LOAD( "rb_20sbp.2s1", 0x0000, 0x010000, CRC(d8278b82) SHA1(8e924096e238359fd2a7f81198c8af1515dc8a19) )
	ROM_LOAD( "rb_xea__.2a1", 0x0000, 0x010000, CRC(1bc266ff) SHA1(fcdb63613dbcf0b15744317a69bd8a1e3cf92526) )
	ROM_LOAD( "rb_xea__.2f1", 0x0000, 0x010000, CRC(ae8bee5d) SHA1(bae49779a009a29b27c99f5e5a4b2aeca39b5626) )
	ROM_LOAD( "rb_xea__.2s1", 0x0000, 0x010000, CRC(79a9c309) SHA1(8ad87ec153ed8a89e612011679a172fc66bae711) )
	ROM_LOAD( "rb_xes__.2a1", 0x0000, 0x010000, CRC(14430dcc) SHA1(7e514a1857f6911a57aff7900af297412ef0d905) )
	ROM_LOAD( "rb_xes__.2f1", 0x0000, 0x010000, CRC(a10a856e) SHA1(a8536d576b3698ccc539c8ac1c9136222b1cf297) )
	ROM_LOAD( "rb_xes__.2s1", 0x0000, 0x010000, CRC(7628a83a) SHA1(da1833df2e88480dafd2410fb24f0fbffbdeb679) )
	ROM_LOAD( "rb_xesb_.2f1", 0x0000, 0x010000, CRC(c321253d) SHA1(0d59b0cf7118c7932e9d89eca823ce200c5030f5) )
	ROM_LOAD( "rb_xesb_.2s1", 0x0000, 0x010000, CRC(14030869) SHA1(283e7ca543a37a60f2d3a8c6d5473b591bb20e62) )
	ROM_LOAD( "rb_xesd_.2a1", 0x0000, 0x010000, CRC(b054f5f9) SHA1(fdcca5375ff8f26f6889c5556216ff0fdf2bce94) )
	ROM_LOAD( "rb_xesd_.2f1", 0x0000, 0x010000, CRC(051d7d5b) SHA1(b8216e505de00802e5e34d11eb3e18e0736fa772) )
	ROM_LOAD( "rb_xesd_.2s1", 0x0000, 0x010000, CRC(d23f500f) SHA1(fe8d2825c8fb24c3885013c046a15ddec5cb3a1f) )
	ROM_LOAD( "rbixe___.2a1", 0x0000, 0x010000, CRC(349fafdd) SHA1(da68e210c8c0a716c1ef62e7f404f6985903b00a) )
	ROM_LOAD( "rbixe___.2s1", 0x0000, 0x010000, CRC(56f40a2b) SHA1(0c6c035d2a3dbef70b1bc95fa38ed62a70770739) )
ROM_END


ROM_START( m4rhfev )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "rt_sj___.7_0", 0x0000, 0x040000, CRC(3dd895ef) SHA1(433ecc268956c94c51dbccefd006b72e0ad8567b) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "rt_sja__.7_0", 0x0000, 0x040000, CRC(0ab59402) SHA1(485b4d2efd8f99085ed6ce5b7e07ede001c982c4) )
	ROM_LOAD( "rt_sjs__.7_0", 0x0000, 0x040000, CRC(1a8feafb) SHA1(83151f63b7ebe1c538f9334e9c3d6889d0730144) )
	ROM_LOAD( "rt_vc___.1_0", 0x0000, 0x040000, CRC(2a8df147) SHA1(df0e7021e9d169575a1297f9851b5a64e20d1a40) )
	ROM_LOAD( "rt_vc_d_.1_0", 0x0000, 0x040000, CRC(7adef22b) SHA1(d6a584581745c0ce64f646ef0b49cb68343990d0) )
	ROM_LOAD( "rtv.p1", 0x0000, 0x008000, CRC(7b78f3f2) SHA1(07ef8e6a08fd70ee48e4463672a1230ecc669532) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "rf_____.1_1", 0x000000, 0x080000, CRC(ac8d539d) SHA1(8baf14bece50774f93ae9eaf3effabb6882d2c43) )
	ROM_LOAD( "rf_____.1_2", 0x080000, 0x080000, CRC(cc2fadd8) SHA1(681850e2e6164cf8af8e7501ac44f475cc07b742) )
	ROM_LOAD( "rf_____.1_3", 0x100000, 0x080000, CRC(165aaf9f) SHA1(815224fe94a77628cef1dd0d8a238edcb4813006) )
	ROM_LOAD( "rf_____.1_4", 0x180000, 0x080000, CRC(4f7e7b49) SHA1(f9d421eeab73e0c795a08cf166c8807e0b14ec82) )
ROM_END



ROM_START( m4rhs )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rh_sj___.4s1", 0x0000, 0x020000, CRC(be6179cd) SHA1(8aefffdffb25bc4dd7d083c7027be746181c2ff9) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "rh_sj__c.6_1", 0x0000, 0x020000, CRC(476f3cf2) SHA1(18ce990e28ca8565ade5eec9a62f0b243121af73) )
	ROM_LOAD( "rh_sj_b_.4s1", 0x0000, 0x020000, CRC(58a4480e) SHA1(f4ecfa1debbfa9dba75263bce2c9f66741c3466f) )
	ROM_LOAD( "rh_sj_bc.6_1", 0x0000, 0x020000, CRC(2e37a58c) SHA1(a48c96384aa81f98bfa980c93e93523ecef3d43c) )
	ROM_LOAD( "rh_sj_d_.4s1", 0x0000, 0x020000, CRC(8f1176db) SHA1(283ef0b9515eac342a02489118bd30016ba85399) )
	ROM_LOAD( "rh_sj_k_.4s1", 0x0000, 0x020000, CRC(3f2ef505) SHA1(28c3806bc48af21a2b7ea27d42ea9f6b4346f3b8) )
	ROM_LOAD( "rh_sja__.4s1", 0x0000, 0x020000, CRC(b8cdd5fb) SHA1(4e336dd3d61f4fdba731951c56e440766ea8efeb) )
	ROM_LOAD( "rh_sja_c.6_1", 0x0000, 0x020000, CRC(b7b790e5) SHA1(e2b34dc2f6ede4f4c22b11123dfaed46f2c5c45e) )
	ROM_LOAD( "rh_sjab_.4s1", 0x0000, 0x020000, CRC(c8468d4c) SHA1(6a9f8fe10949712ecacca3bfcd7d5ab4860682e2) )
	ROM_LOAD( "rh_sjad_.4s1", 0x0000, 0x020000, CRC(df4768f0) SHA1(74894b232b27e65058d59acf174172da86def95a) )
	ROM_LOAD( "rh_sjak_.4s1", 0x0000, 0x020000, CRC(6f78eb2e) SHA1(a9fec7a7ad9334c3d8760e1982ac00651858cee8) )
	ROM_LOAD( "rocky15g", 0x0000, 0x020000, CRC(05f4f333) SHA1(a1b917f6c91d751fb2433e46c4c60840b47eed9e) )
	ROM_LOAD( "rocky15t", 0x0000, 0x020000, CRC(3fbad6de) SHA1(e8d76b3878794c769187d92d2834018a84e764ac) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "rh.chr", 0x0000, 0x000048, CRC(6e246d8f) SHA1(c6b0dff1b918c578b76f020ff70a24ea48dbae2e) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "rhp2snd", 0x0000, 0x080000, CRC(18112293) SHA1(b2bf838849ad1a9931c294ccc291ba2f5c5f45e9) )

	ROM_LOAD( "rh___snd.1_1", 0x000000, 0x080000, CRC(ceebd8f4) SHA1(fe9f62034aae7d2ec097d80dc471a7fd27ddec8a) )
	ROM_LOAD( "rh___snd.1_2", 0x080000, 0x080000, CRC(1f24cfb6) SHA1(cf1dc9d2a1c1cfb8718c89e245e9bf375fef8bfd) )
	ROM_LOAD( "rh___snd.1_3", 0x100000, 0x080000, CRC(726958d8) SHA1(6373765b80971dd7ff5c8eaeee83966335db4d27) )
ROM_END



ROM_START( m4sinbd )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sd_20__c.1_1", 0x0000, 0x020000, CRC(28cd336e) SHA1(45bdf5403c04b7d3a3645b6b44ac3d12e6463a55) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sd_20a__.4_1", 0x0000, 0x020000, CRC(12b8f629) SHA1(c8540ecb217cf0615d7a8d080136926646ca8497) )
	ROM_LOAD( "sd_20a__.5_1", 0x0000, 0x020000, CRC(68a3d155) SHA1(5ae47f1ca860af30c77beebe3acf615958ed59e9) )
	ROM_LOAD( "sd_20a_c.1_1", 0x0000, 0x020000, CRC(5a5eef7c) SHA1(32d801cea0593e220ac67c2c17782fc144d02cc4) )
	ROM_LOAD( "sd_20ad_.5_1", 0x0000, 0x020000, CRC(dc9dd7b3) SHA1(e26c8a814cdb268026e31acb668f34160ea9ce7b) )
	ROM_LOAD( "sd_20s__.1v1", 0x0000, 0x020000, CRC(acb5aa9a) SHA1(4186058f83e241f06de842d0ae79f26a2a0e6cf0) )
	ROM_LOAD( "sd_20s__.4_1", 0x0000, 0x020000, CRC(bce21ced) SHA1(9e089fbca1dd39e356f38d0c301162affea9316c) )
	ROM_LOAD( "sd_20s__.5_1", 0x0000, 0x020000, CRC(939ecca5) SHA1(a9cd465fbe742b910602f861ae6ac7cedc7d9be9) )
	ROM_LOAD( "sd_20s_p.4_1", 0x0000, 0x020000, CRC(5ae716a9) SHA1(4c9ceab423a1480f8257fad2d62b65a17788a472) )
	ROM_LOAD( "sd_20s_s.5_1", 0x0000, 0x020000, CRC(e47c869a) SHA1(2735613aafe71e9b2a3aabb433fbd98a83a546c7) )
	ROM_LOAD( "sd_20sb_.4_1", 0x0000, 0x020000, CRC(811efef2) SHA1(280c6d6697735faca58317925c58b687c5988b87) )
	ROM_LOAD( "sd_20sb_.5_1", 0x0000, 0x020000, CRC(594920ec) SHA1(03ac253ef35b8fbccbfe4b2c5a7d906a3001b3ec) )
	ROM_LOAD( "sd_20sbc.1_1", 0x0000, 0x020000, CRC(3720cf0d) SHA1(8f0e63985badbff6ac39fcf956ebff3b0655c2b9) )
	ROM_LOAD( "sd_20sbp.4_1", 0x0000, 0x020000, CRC(671bf4b6) SHA1(ac928b47c1392e8a82404776068233e7d1dd5d28) )
	ROM_LOAD( "sd_20sbs.5_1", 0x0000, 0x020000, CRC(2eab6ad3) SHA1(66261328999a139389198e772a69dcd994792439) )
	ROM_LOAD( "sd_20sdc.1_1", 0x0000, 0x020000, CRC(fc386926) SHA1(6e64aeb82e62ded75e48f0faab932b63732bcf08) )
	ROM_LOAD( "sd_25__c.1_1", 0x0000, 0x020000, CRC(348e846d) SHA1(ca0fffd59076e6e60d37f677d0c7e7f182a41b9e) )
	ROM_LOAD( "sd_25a_c.1_1", 0x0000, 0x020000, CRC(f50c55e9) SHA1(df72b196fb21b6359282c06b960fa53117a95fee) )
	ROM_LOAD( "sd_25sbc.1_1", 0x0000, 0x020000, CRC(0773632a) SHA1(34294e32243b903a6c1c54b8718ca7cddbd3316e) )
	ROM_LOAD( "sd_25sdc.1_1", 0x0000, 0x020000, CRC(4917a542) SHA1(23de449d536c799032afab678b9001fa8541fb8a) )
	ROM_LOAD( "sdi20___.1v1", 0x0000, 0x020000, CRC(c50b3555) SHA1(c2d89126b9122f48ecce52d50cd7a03cc2bf1829) )
	ROM_LOAD( "sv_20a__.4_1", 0x0000, 0x020000, CRC(0e790ae8) SHA1(48f055f3f1f5d3392b7fba1c5c30624c1f230327) )
	ROM_LOAD( "sv_20sb_.4_1", 0x0000, 0x020000, CRC(e88c1c29) SHA1(49d180068e4ae9cb65a58e65b4e1ac4d6657ae1d) )
	ROM_LOAD( "svi20___.4_1", 0x0000, 0x020000, CRC(643037ed) SHA1(d4063faba3069625474dd761f9ad2dcf2f710a19) )
	ROM_LOAD( "sinbadbwb1_1game.bin", 0x0000, 0x020000, CRC(cfe152a7) SHA1(b8ecfa8b763d04515b65eb902c18dba7198191c3) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	//These were with the last listed set, though I have no reason to believe they aren't valid for all BwB Sinbad games
	ROM_LOAD( "sinbadbwb1_2snd.bin", 0x000000, 0x080000, CRC(2ee60ce6) SHA1(865860639e8471f97ace0beac2f4c7fddb8ca97c) )
	ROM_LOAD( "sinbadbwb1_3snd.bin", 0x000000, 0x080000, CRC(7701e5cc) SHA1(4f9ff91f2b6b15a9c08396b52fc8509ba476ed8d) )
ROM_END


ROM_START( m4sky )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sk_s____.3_1", 0x0000, 0x040000, CRC(749af008) SHA1(036514f2bcb84193cfa84313f0617f3196aea73e) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sk_sj___.5_0", 0x0000, 0x040000, CRC(45ae0423) SHA1(94d5b3d4aacb69a18ff3f45681eb5f7fba7657e8) )
	ROM_LOAD( "sk_sj_k_.5_0", 0x0000, 0x040000, CRC(e1bab980) SHA1(1c8b127809422ab0baf1875ca907f18269a0cc17) )
	ROM_LOAD( "sk_sja__.5_0", 0x0000, 0x040000, CRC(b2a16ef7) SHA1(9012dcc320e8af8fef53e0dc91d3bcd6cbafa5ee) )
	ROM_LOAD( "sk_sjs__.5_0", 0x0000, 0x040000, CRC(d176431f) SHA1(8ca90ef61486fc5a5b6527f913cd05b42ceabe3e) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4souls )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ss_06a__.4_1", 0x0000, 0x020000, CRC(00390a21) SHA1(d31d1307301fa4e8cf0ce3677e68a4c1723e4404) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ss_16a__.4_1", 0x0000, 0x020000, CRC(b9ab9612) SHA1(ad30916a0f2cc745741c99d23c23192ae4088daf) )
	ROM_LOAD( "ss_26a__.2_1", 0x0000, 0x020000, CRC(bf9acf05) SHA1(13698b453e975a1801631163d06468f07c181b48) )
	ROM_LOAD( "ss_26ad_.2_1", 0x0000, 0x020000, CRC(3e867df2) SHA1(0c7d57c05952cb4f737ee07f139a3593803d3d60) )
	ROM_LOAD( "ss_26sb_.2_1", 0x0000, 0x020000, CRC(2010749e) SHA1(4220b12db7870a2efd7a7ab573e4e08ff0643e70) )
	ROM_LOAD( "ss_26sd_.2_1", 0x0000, 0x020000, CRC(773db916) SHA1(a7d168db22d5adeb3eaf64786bbe744ce787ff68) )
	ROM_LOAD( "ss_26sk_.2_1", 0x0000, 0x020000, CRC(9ae1672e) SHA1(79bcd12fae38dd1b0035e956148ffeaee33b9c71) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "ss______.1_2", 0x000000, 0x080000, CRC(ddea9d75) SHA1(fe5f88d49434109d0f51425e790e179dc1a02767) )
	ROM_LOAD( "ss______.1_3", 0x080000, 0x080000, CRC(23d1e57a) SHA1(b17afdaa95522fd7ea6c12f513fa338e1fcb06f6) )
	ROM_LOAD( "ss______.1_4", 0x100000, 0x080000, CRC(0ba3046a) SHA1(ec21fa328669bc7a5baf1ce8b9ac05f38f98e360) )
ROM_END


ROM_START( m4specu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "speculator.bin", 0x8000, 0x008000, CRC(4035d20c) SHA1(4a534294c5c7332eacd09ca44f351d6a6850cc29) )
ROM_END




ROM_START( m4spinbt )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sn_37ad_.5_0", 0x0000, 0x040000, CRC(42d6faaa) SHA1(3789e85981b33ffae7c50ccca3278ae62974972d) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sn_37b__.5_0", 0x0000, 0x040000, CRC(3a259a6f) SHA1(1acabb9e725ae1374b87808c4b3d06a329c824d0) )
	ROM_LOAD( "sn_37bd_.5_0", 0x0000, 0x040000, CRC(f4e3f395) SHA1(545b2ea1cf4231ba1663bea0e6770976b0797cf3) )
	ROM_LOAD( "sn_37bg_.5_0", 0x0000, 0x040000, CRC(c3ad8312) SHA1(9463903fe65462e5e9b09a992dc4625dba8452f0) )
	ROM_LOAD( "sn_37bt_.5_0", 0x0000, 0x040000, CRC(9fd1df18) SHA1(4c647bc887cdaa2415d0308787c710c83ce1c0d3) )
	ROM_LOAD( "sn_s7a__.5_0", 0x0000, 0x040000, CRC(69d39933) SHA1(ec2a9fb7c4977532a7a431745eb2d82d4e282159) )
	ROM_LOAD( "sn_s7ad_.5_0", 0x0000, 0x040000, CRC(a715f0c9) SHA1(bec188edcd0580465859876f8bff2ff5a392a9e1) )
	ROM_LOAD( "sn_s7s__.5_0", 0x0000, 0x040000, CRC(13288cc5) SHA1(2c47dcd8b57d10e0768729ad91dba12521c0d98a) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4starst )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sr_20__d.3_0", 0x0000, 0x040000, CRC(98f6619b) SHA1(fc0a568e6695c9ad0fda7bc6703c752af26a7777) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sr_20_bd.3_0", 0x0000, 0x040000, CRC(ff8209de) SHA1(41a4c20c89b3a04612ad6298276472b888915c89) )
	ROM_LOAD( "sr_20_kd.3_0", 0x0000, 0x040000, CRC(4c9a53d5) SHA1(43ebf6c06db58de9c3934e2dbba0d8126f3e2dda) )
	ROM_LOAD( "sr_20a_d.3_0", 0x0000, 0x040000, CRC(e9eebb4c) SHA1(60d8010140d9debe8f12d7f810de223d9abd02a4) )
	ROM_LOAD( "sr_20s_d.3_0", 0x0000, 0x040000, CRC(725b50e6) SHA1(3efde346022e37b09df08b8188ac76dcdfac8a4e) )
	ROM_LOAD( "sr_sj___.5_0", 0x0000, 0x040000, CRC(7964bd86) SHA1(7078de5a61b52dedb776993643f7edd8a2c863c3) )
	ROM_LOAD( "sr_sj_b_.5_0", 0x0000, 0x040000, CRC(4ee1f95b) SHA1(1e3d52afd19a9489608d5446ef2118561c6411b0) )
	ROM_LOAD( "sr_sj_d_.5_0", 0x0000, 0x040000, CRC(b4d78711) SHA1(c864c944b3fa74aa1fed22afe656a37413b024ce) )
	ROM_LOAD( "sr_sj_k_.5_0", 0x0000, 0x040000, CRC(c7681e28) SHA1(a8c1c75df33c85301257147c97d6af8808dad0d2) )
	ROM_LOAD( "sr_sja__.5_0", 0x0000, 0x040000, CRC(aa86c4f2) SHA1(e90fd91f1d14b89714e3fb8236ac9e8a641e4c71) )
	ROM_LOAD( "sr_sjs__.5_0", 0x0000, 0x040000, CRC(89e405e4) SHA1(5aa9053e08c27570731f65502c7fb31f0ea0a678) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "starsnd3", 0x0000, 0x080000, CRC(96882952) SHA1(bf366670aec5cb545c56caac3c63855db03d8c14) )// (?)strange id number?

	/* missing? */
ROM_END


ROM_START( m4thestr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "thestreak.bin", 0x0000, 0x010000, CRC(cb79f9e5) SHA1(6cbdc5327e81b51f1060fd91efa3d061b9748b49) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ts_20_b4.3_1", 0x0000, 0x010000, CRC(17726c7c) SHA1(193b572b9f859f1018f1be398b35a5103622faf8) )
	ROM_LOAD( "ts_20_bc.3_1", 0x0000, 0x010000, CRC(b03b3f11) SHA1(9116ac608ab5574d5912550b988fc319d0a38444) )
	ROM_LOAD( "ts_20_d4.3_1", 0x0000, 0x010000, CRC(7bfae07a) SHA1(9414ad510ca9a183181a30d98858278c375c185d) )
	ROM_LOAD( "ts_20_dc.3_1", 0x0000, 0x010000, CRC(7196b317) SHA1(c124ed3d030b77870b7851b3da104f8fc5393a31) )
	ROM_LOAD( "ts_20a_4.3_1", 0x0000, 0x010000, CRC(921b8cc3) SHA1(74143888de21aba4374d016cb4c08ae59dfa59ef) )
	ROM_LOAD( "ts_20a_c.3_1", 0x0000, 0x010000, CRC(c8eb1dd9) SHA1(7b7520467cd32295e6324d350d1f2bed829555e0) )
	ROM_LOAD( "ts_20b_4.3_1", 0x0000, 0x010000, CRC(2221f704) SHA1(8459b658d3ad84bb86250518d0403970f881323d) )
	ROM_LOAD( "ts_20b_c.3_1", 0x0000, 0x010000, CRC(ecdf59a9) SHA1(7c2141e336ba3f1865bbf422aaa0b78cb1a27a4c) )
	ROM_LOAD( "ts_20bg4.3_1", 0x0000, 0x010000, CRC(b2a419ea) SHA1(bbc565ce8e79d39e1b1a7cd1685fa8c7ce00d7b9) )
	ROM_LOAD( "ts_20bgc.3_1", 0x0000, 0x010000, CRC(3b2d7b50) SHA1(a8560b0894783a398aaf0510493583b8cb826947) )
	ROM_LOAD( "ts_20bt4.3_1", 0x0000, 0x010000, CRC(d10a6c5a) SHA1(07fdf3797c87e35468ef859bb67753f11c5fbded) )
	ROM_LOAD( "ts_20btc.3_1", 0x0000, 0x010000, CRC(58830ee0) SHA1(b2798e0f8e03870c77892a32654263575e9aaafa) )
	ROM_LOAD( "ts_25_bc.3_1", 0x0000, 0x010000, CRC(43877de6) SHA1(b006ae97139c9bd66a32884b92fdbdf4f10db58a) )
	ROM_LOAD( "ts_25_dc.3_1", 0x0000, 0x010000, CRC(60ab675c) SHA1(763b66d7731489abdec84d2f8e3c186ad95c7349) )
	ROM_LOAD( "ts_25_kc.3_1", 0x0000, 0x010000, CRC(418cbb32) SHA1(e19a0c7fd88a82983ec33b99f3819a2a238c68a5) )
	ROM_LOAD( "ts_25a_c.3_1", 0x0000, 0x010000, CRC(448a705f) SHA1(35a7cc480c376eaef7439d5c96cec490aec9fc4b) )
	ROM_LOAD( "ts_25b_c.3_1", 0x0000, 0x010000, CRC(5b390ec2) SHA1(db7719ab8021e0b75e9419d2a05f3139fbab8e61) )
	ROM_LOAD( "ts_25bgc.3_1", 0x0000, 0x010000, CRC(612463fc) SHA1(085d8faf91c8ef6520ca971d249322f336464856) )
	ROM_LOAD( "ts_25btc.3_1", 0x0000, 0x010000, CRC(6d658e61) SHA1(619dbacad424e5db82c6ee19d1e3358c18cfe783) )
	ROM_LOAD( "ts_30a_c.1_1", 0x0000, 0x010000, CRC(8636e700) SHA1(f11c20da6c3bfe1842ea8f9eac8c831d49f42c32) )
	ROM_LOAD( "ts_30b_c.1_1", 0x0000, 0x010000, CRC(2577c8fc) SHA1(6d22bd1a93f423862f5466f99690eeced9090420) )
	ROM_LOAD( "ts_30bgc.1_1", 0x0000, 0x010000, CRC(2c582ba6) SHA1(dbcae0ef90105a7f6c720156711f73bb3c237b8a) )
	ROM_LOAD( "ts_39_dc.1_1", 0x0000, 0x010000, CRC(84d338b8) SHA1(847e6b7808b6d5d361414a4aaa5d5cf6a5863a70) )
	ROM_LOAD( "ts_39a_c.1_1", 0x0000, 0x010000, CRC(9ee56a3a) SHA1(365ec4c90abbe4b352bdd2d6aed5eec4cdaf35ff) )
	ROM_LOAD( "ts_39b_c.1_1", 0x0000, 0x010000, CRC(470cd6d1) SHA1(c9c3c9c23c596e79f1b6495d4706b1da6cbd1b2e) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "thestreaksnd.bin", 0x0000, 0x080000, CRC(fdbd0f88) SHA1(8d0eaa9aa8d505affeb8bd12d7cb13337aa2e2c2) )
ROM_END


ROM_START( m4sunclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sucxe__0.2", 0x0000, 0x010000, CRC(fd702a6f) SHA1(0f6d553fcb096ca4874bb971425dabfbe18db31d) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sucxed_0.2", 0x0000, 0x010000, CRC(70802bc3) SHA1(69b36f716cb608931f933cb58e47232b18064f9d) )
ROM_END



ROM_START( m4sunscl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sc_xe___.3_3", 0x0000, 0x010000, CRC(e3732cc6) SHA1(77f0368bb29ad00030f83af794a52df92fe97e5d) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sc_xe_d_.3_3", 0x0000, 0x010000, CRC(b8627c4a) SHA1(ad616d38773cbd82376b518aa15dc3d7027237c5) )
	ROM_LOAD( "sc_xef__.3_3", 0x0000, 0x010000, CRC(8e7e1100) SHA1(7648ea860a546081388a213845e27312730f46d9) )
ROM_END


ROM_START( m4supleg )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sl_sj.hex", 0x0000, 0x040000, CRC(254835f7) SHA1(2fafaa3da747edd27d393ad106008e898e465283) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sl_sjs.hex", 0x0000, 0x040000, CRC(98942cd3) SHA1(858fde0a350159d089c6a0e0cc2e2eed6ab2092c) )
	ROM_LOAD( "sl_vc.hex", 0x0000, 0x040000, CRC(1940d117) SHA1(ae7338483ac39e9e1973dde5eb837443512630dd) )
	ROM_LOAD( "sl_vcd.hex", 0x0000, 0x040000, CRC(7aab16d1) SHA1(6da4e0d9883a48937d00bfc5929b3557de51f60e) )
	ROM_LOAD( "sls.hex", 0x0000, 0x040000, CRC(5ad6dbb9) SHA1(ff6f9dcf14df22c7bb2b949fcd5c70f31d4c1928) )
	ROM_LOAD( "s_leag._pound5", 0x0000, 0x040000, CRC(4c6bd78e) SHA1(f67793a2a16adacc8d92b57050f02cffa50a1283) ) //Whitbread?
	ROM_REGION( 0x80000, "altsnd", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "s_leag.s1", 0x0000, 0x080000, CRC(e8d90896) SHA1(4c67507f18b5dc966e2df3685dc6c257f5053e61) ) //alt first sound rom, connected to whitbread set

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "sls1.hex", 0x000000, 0x080000, CRC(341e3d3e) SHA1(b42ec737e95e766b1b26d6225416ee0c5cad2663) )
	ROM_LOAD( "sls2.hex", 0x080000, 0x080000, CRC(f4ab6f0d) SHA1(4b59608ca16c9d158d4d1ac532e7fbe6ff0da959) )
	ROM_LOAD( "sls3.hex", 0x100000, 0x080000, CRC(dcba96a1) SHA1(d474c63b37cb18a0b3b1299b5cacadfd8cd5458b) )
ROM_END



ROM_START( m4supscr )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "multistakesoccer.bin", 0x0000, 0x040000, CRC(ce27b6a7) SHA1(f9038336137b0642da4d1520b5d71a047d8fbe12) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "sm_78___.6_0", 0x0000, 0x040000, CRC(e7022c44) SHA1(da3a5b9954f7e50dce73aeb9c46bd4631c8350d5) )
	ROM_LOAD( "sm_78_d_.6_0", 0x0000, 0x040000, CRC(4dbe6a87) SHA1(fe2ce1fca7105afbf459ee6558744f8fee417169) )
	ROM_LOAD( "sm_80___.6_0", 0x0000, 0x040000, CRC(7e9831ec) SHA1(d0e645ea1f6d34c507103550c248200313d033ef) )
	ROM_LOAD( "sm_80_d_.6_0", 0x0000, 0x040000, CRC(d424772f) SHA1(938eb7a35e59db704b0737d309b7989b35ed43e0) )
	ROM_LOAD( "sm_82___.6_0", 0x0000, 0x040000, CRC(019b4d42) SHA1(b27894f99dc2945fccb0de66f34eacc5b02a3463) )
	ROM_LOAD( "sm_82_d_.6_0", 0x0000, 0x040000, CRC(ab270b81) SHA1(6af522e78b9b263c96cb350cbc8585be206dcd0a) )
	ROM_LOAD( "sm_84___.6_0", 0x0000, 0x040000, CRC(809ec8b0) SHA1(965edf407a1fd7dfda2817b884ef65b3d57d51e5) )
	ROM_LOAD( "sm_84_d_.6_0", 0x0000, 0x040000, CRC(2a228e73) SHA1(04c8d17b2f2800dac9aca81e926e6e48eb02c5ac) )
	ROM_LOAD( "sm_86___.6_0", 0x0000, 0x040000, CRC(ff9db41e) SHA1(5996438e801534ebf5d9755b340fada67cadc942) )
	ROM_LOAD( "sm_86_d_.6_0", 0x0000, 0x040000, CRC(5521f2dd) SHA1(add1b70a6cddc4e176697660aeff331535d92898) )
	ROM_LOAD( "sm_88___.6_0", 0x0000, 0x040000, CRC(59e4c515) SHA1(5a76962dc6530d326bfe6ef6498c8f2ad481d6f1) )
	ROM_LOAD( "sm_88_d_.6_0", 0x0000, 0x040000, CRC(f35883d6) SHA1(1948e06e302cdb01fbd32d711374957e9e6bd64a) )
	ROM_LOAD( "sm_90___.6_0", 0x0000, 0x040000, CRC(cdc7c594) SHA1(acb829257472bc4420c141932b6f4c708ea04f1b) )
	ROM_LOAD( "sm_90_d_.6_0", 0x0000, 0x040000, CRC(677b8357) SHA1(4ec54f8b7d28c0459152309a58bd5c0db1a2f036) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4ssclas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs__x__x.6_0", 0x0000, 0x010000, CRC(3230284d) SHA1(bca3b4c43859ed424956c4119fa6a91a2e7d6eec) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "cs__x_dx.2_0", 0x0000, 0x010000, CRC(ea004a13) SHA1(db9a187b0672c69a6a149ec6d1025bd6da9beccd) )
	ROM_LOAD( "cs__x_dx.6_0", 0x0000, 0x010000, CRC(6dd2d11f) SHA1(8c7e60d3e5a0d4fccb024b5c0aa21fd2b9a5ada9) )
	ROM_LOAD( "cs__xa_x.6_0", 0x0000, 0x010000, CRC(6657e810) SHA1(0860076cf01c732f419483876991fb42a838622a) )
	ROM_LOAD( "cs__xb_x.5_0", 0x0000, 0x010000, CRC(a5f46ff5) SHA1(a068029f774bc6ed2e76acc2eb509bc6e2490945) )
	ROM_LOAD( "cs__xb_x.6_0", 0x0000, 0x010000, CRC(801d543c) SHA1(f0905947312fb2a526765d17cde01af5095ef923) )

	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "ssbwb.chr", 0x0000, 0x000048, CRC(910b09db) SHA1(d54399660b1bf1a89712b25292ac99b740442e5c) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "css_____.1_1", 0x0000, 0x080000, CRC(e738fa1e) SHA1(7a1125320e0d488729aec66e658d418b96228fd0) )
ROM_END


ROM_START( m4sure )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "su_xf___.3_1", 0x0000, 0x010000, CRC(f85dae5c) SHA1(4c761c355fb6651f1e0cb041342f8a2ff510dfd2) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "su_xf_b_.3_1", 0x0000, 0x010000, CRC(9a760e0f) SHA1(fdacdae0e2322daa004b2385616dd34626814d42) )
	ROM_LOAD( "su_xf_d_.3_1", 0x0000, 0x010000, CRC(5c4a5669) SHA1(55e1e853fdfdbb43e7b61b59ab642fb013a0db0e) )
	ROM_LOAD( "suixf___.3_1", 0x0000, 0x010000, CRC(cae80b60) SHA1(23545aaf1cc3a0c8868beafb56eccedbbb6099de) )
ROM_END

ROM_START( m4tic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt_20a__.2_1", 0x0000, 0x010000, CRC(b923ac0d) SHA1(1237962af43c2c3f4ed0ad5bed21f24decfeae02) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "tt_20a_c.1_1", 0x0000, 0x010000, CRC(18a68ea0) SHA1(37783121ff5540e264d89069101d991acb66b982) )
	ROM_LOAD( "tt_20b__.2_1", 0x0000, 0x010000, CRC(b5eb86ab) SHA1(99ddb80941c67bd271e22af17405457d32676484) )
	ROM_LOAD( "tt_20b_c.1_1", 0x0000, 0x010000, CRC(d35079ab) SHA1(d109af8ef6f4d26b505f63df10d5850ddc0c0b65) )
	ROM_LOAD( "tt_20bd_.2_1", 0x0000, 0x010000, CRC(0889a699) SHA1(c96d135b9248e9bab78af438b97e6cb854b2c771) )
	ROM_LOAD( "tt_20bdc.1_1", 0x0000, 0x010000, CRC(2a43efd4) SHA1(9f6e568ca95a5f4e1a4e82eda2d15dfa225e65ea) )
	ROM_LOAD( "tt_20bg_.2_1", 0x0000, 0x010000, CRC(128896c1) SHA1(af37645b88116cde57fcc42ed58d69bf9c11ff8a) )
	ROM_LOAD( "tt_20bt_.2_1", 0x0000, 0x010000, CRC(362046f9) SHA1(6cb5a986517158d63e7403891bb749eaccb63acb) )
	ROM_LOAD( "tt_20s__.2_1", 0x0000, 0x010000, CRC(53dfefe9) SHA1(0f9fc1d65ebd7e370de6001f594616b79b2aa57e) )
	ROM_LOAD( "tt_20s_c.1_1", 0x0000, 0x010000, CRC(65a38960) SHA1(48ffdda1c5c98742124418429c510de9f5b90270) )
	ROM_LOAD( "tt_20sb_.2_1", 0x0000, 0x010000, CRC(c9174384) SHA1(f694a6a7f78b8a062fd26371fa6758ec4252352a) )
	ROM_LOAD( "tt_20sk_.2_1", 0x0000, 0x010000, CRC(dca42636) SHA1(c6e9aaf402c2fc7eec6e9b07aa4c33312bc0af0e) )
	ROM_LOAD( "tt_25a_c.3_1", 0x0000, 0x010000, CRC(2e44c6db) SHA1(ffc96dafbcfae719c3971882e066971540fafe78) )
	ROM_LOAD( "tt_25b_c.3_1", 0x0000, 0x010000, CRC(d393edf0) SHA1(66f17a88018fee71f3e0c7996371c9b6832ef23a) )
	ROM_LOAD( "tt_25bdc.3_1", 0x0000, 0x010000, CRC(2ce71772) SHA1(a2f36d0d11826a7be7f8cc04f21a77facb4ce188) )
	ROM_LOAD( "tt_25bgc.3_1", 0x0000, 0x010000, CRC(2dbeb9c3) SHA1(8288a9d17932582c7536563e34e2150a85c7a822) )
	ROM_LOAD( "tt_25btc.3_1", 0x0000, 0x010000, CRC(d5702abf) SHA1(6115f39d70dfdf1a00bcfc5f0fe257dd1e0ff968) )
	ROM_LOAD( "tt_25s_c.3_1", 0x0000, 0x010000, CRC(d393edf0) SHA1(66f17a88018fee71f3e0c7996371c9b6832ef23a) )
	ROM_LOAD( "tt_25sbc.3_1", 0x0000, 0x010000, CRC(11c0152f) SHA1(d46b0a6774da35cf9d3a352b9fe7cb574880b210) )
	ROM_LOAD( "tti20___.2_1", 0x0000, 0x010000, CRC(91054bf6) SHA1(68cc6c9b47849149a574e3af97bd0e8255fc5c43) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4ticcla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ct_20_b4.7_1", 0x0000, 0x010000, CRC(48b9a162) SHA1(2d19a5d6379dc93a56c920b3cd61a0d1a8c6b303) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ct_20_bc.4_1", 0x0000, 0x010000, CRC(fb40b5ff) SHA1(723a07a2b6b08483aa75ecdd4fd9720a66201fc3) )
	ROM_LOAD( "ct_20_d4.7_1", 0x0000, 0x010000, CRC(3c7c862c) SHA1(a3577f29950e845a14ca68750d2ab6c56a395dba) )
	ROM_LOAD( "ct_20_dc.4_1", 0x0000, 0x010000, CRC(0f20a790) SHA1(02876178f0af64154d490cc048a7bc1c9a6f521b) )
	ROM_LOAD( "ct_20a_4.7_1", 0x0000, 0x010000, CRC(35318095) SHA1(888105a674c9ea8ccad33e24c05ef42936f5f4cf) )
	ROM_LOAD( "ct_20a_c.4_1", 0x0000, 0x010000, CRC(e409f49f) SHA1(8774015ec20ed9fe54e812013dfc12d408276c31) )
	ROM_LOAD( "ct_20b_c.4_1", 0x0000, 0x010000, CRC(864a59cf) SHA1(abd9b7a47c791ce4f91abbd3bf97bdcd9d8296ee) )
	ROM_LOAD( "ct_20bg4.7_1", 0x0000, 0x010000, CRC(7f200f42) SHA1(0ea6aa0de88982737d818c9dac9f2605cea7bc11) )
	ROM_LOAD( "ct_20bgc.4_1", 0x0000, 0x010000, CRC(215b8965) SHA1(883735066a1425b502e89d1234575294ac83746c) )
	ROM_LOAD( "ct_20bt4.7_1", 0x0000, 0x010000, CRC(7c7280a4) SHA1(3dbdc53a3474f4147427ed4fa8a161a3b364d43b) )
	ROM_LOAD( "ct_25_bc.3_1", 0x0000, 0x010000, CRC(9d6fb3b0) SHA1(a6278579d217b5544d9f0b942a7a344596153950) )
	ROM_LOAD( "ct_25_dc.2_1", 0x0000, 0x010000, CRC(b49af435) SHA1(e5f92f114931e554eb8eb5fe89f50298783d541c) )
	ROM_LOAD( "ct_25_dc.3_1", 0x0000, 0x010000, CRC(eb359c82) SHA1(c137768461b859d5277b08c8783b0c8625f9b1be) )
	ROM_LOAD( "ct_25_kc.2_1", 0x0000, 0x010000, CRC(43309e7b) SHA1(d8f6ecbea618da7f54309f2a6e93210c51b68b81) )
	ROM_LOAD( "ct_25a_c.2_1", 0x0000, 0x010000, CRC(717396ed) SHA1(6cdb0f99b40096178f6e85a0966182e704d1b99a) )
	ROM_LOAD( "ct_25a_c.3_1", 0x0000, 0x010000, CRC(28e0a15b) SHA1(b3678ba3d1f392665cc6ec9c24c2c506a41cd4fa) )
	ROM_LOAD( "ct_25b_c.2_1", 0x0000, 0x010000, CRC(b3d7e79c) SHA1(86c0b419c3ca054f8a2ed785cffeb03e6c5b69f2) )
	ROM_LOAD( "ct_25b_c.3_1", 0x0000, 0x010000, CRC(e0ba763d) SHA1(453d8a0dbe616c5a8c4313b918fcfe21fed473e0) )
	ROM_LOAD( "ct_25bgc.2_1", 0x0000, 0x010000, CRC(0869d04c) SHA1(0f0fd3982ac376c66d139655a50639f48bf740b4) )
	ROM_LOAD( "ct_25bgc.3_1", 0x0000, 0x010000, CRC(1e0ca1d1) SHA1(0b1023cdd5cd3db657cea53c85e31ed83c2e5524) )
	ROM_LOAD( "ct_25btc.2_1", 0x0000, 0x010000, CRC(032ec96d) SHA1(c5cef956bc0e3eb45cf128c8d0b4e1d6e5b01afe) )
	ROM_LOAD( "ct_25btc.3_1", 0x0000, 0x010000, CRC(f656897a) SHA1(92ad5c6ce2a696298bbfc8c1750825db4e3bc80b) )
	ROM_LOAD( "ct_30_dc.2_1", 0x0000, 0x010000, CRC(57fabdfb) SHA1(ad86621e4bc8141508c691e148a66e74fc070a88) )
	ROM_LOAD( "ct_30a_c.2_1", 0x0000, 0x010000, CRC(800c94c3) SHA1(c78497899ea9cf27e66f6e8526b95d51215053b2) )
	ROM_LOAD( "ct_30b_c.2_1", 0x0000, 0x010000, CRC(3036ef04) SHA1(de514a85d45d11a880ed147aebe211ffb5bee146) )
	ROM_LOAD( "ct_30bdc.2_1", 0x0000, 0x010000, CRC(9852c9d4) SHA1(37bb20d63fa70ea99e18a16a8f11c461a377a07a) )
	ROM_LOAD( "ct_30bgc.2_1", 0x0000, 0x010000, CRC(a1bc89b4) SHA1(4c82ce8fe78768443823e868f7cc49a06e7cc441) )
	ROM_LOAD( "ct_30btc.2_1", 0x0000, 0x010000, CRC(cde0d12e) SHA1(5427ad700311c30cc86eccc7f1ff36cf0da3b980) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "ct______.5_a", 0x0000, 0x080000, CRC(9a936f50) SHA1(f3f66d6093a939220d24aee985e210cdfd214db4) )
ROM_END


ROM_START( m4ticgld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tg_25a_c.3_1", 0x0000, 0x010000, CRC(44b2b6b0) SHA1(c2caadd68659bd474df534101e3bc13b15a43694) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "ct______.5_a", 0x0000, 0x080000, CRC(9a936f50) SHA1(f3f66d6093a939220d24aee985e210cdfd214db4) )
ROM_END


ROM_START( m4ticglc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tg_25a_c.3_1", 0x0000, 0x010000, CRC(44b2b6b0) SHA1(c2caadd68659bd474df534101e3bc13b15a43694) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "tg_30_dc.4_1", 0x0000, 0x010000, CRC(19c0fb1e) SHA1(955da095df56f28ace6839c9b6df5669f576730c) )
	ROM_LOAD( "tg_30a_c.4_1", 0x0000, 0x010000, CRC(3e4dcc70) SHA1(c4ad3a8633e19015d4d2b08a653119e9e4c5dcbb) )
	ROM_LOAD( "tg_30b_c.4_1", 0x0000, 0x010000, CRC(83d1517a) SHA1(38a9269dac53ca701e4b621d5e77696142f429cd) )
	ROM_LOAD( "tg_30bgc.4_1", 0x0000, 0x010000, CRC(a366c32d) SHA1(8d86778411ef07e06d99c12147a211d7620af9bf) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4topdog )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "td_20_b4.7_1", 0x0000, 0x010000, CRC(fe864f25) SHA1(b9f97aaf0425b4987b5bfa0b793e9226fdffe58f) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "td_20_bc.7_1", 0x0000, 0x010000, CRC(3af18a9f) SHA1(0db7427d934363d021265fcac811505867f20d47) )
	ROM_LOAD( "td_20_d4.7_1", 0x0000, 0x010000, CRC(35da9e2d) SHA1(a2d1efd7c9cbe4bb5ce7574c6bea2edf55f3e08f) )
	ROM_LOAD( "td_20_dc.7_1", 0x0000, 0x010000, CRC(b90dfbce) SHA1(b9eb9393fbd33725d372b3b6648c261cf0ae486f) )
	ROM_LOAD( "td_20_k4.7_1", 0x0000, 0x010000, CRC(44618034) SHA1(0fce08e279a16d94422155c695b9b5f124b657ea) )
	ROM_LOAD( "td_20_kc.7_1", 0x0000, 0x010000, CRC(8ec10cf7) SHA1(cdc479f7f41f2205285a9db6539dce83feef6af4) )
	ROM_LOAD( "td_20a_4.7_1", 0x0000, 0x010000, CRC(e7bcc879) SHA1(6c963d059867bdd506af1826fe038daa560a3623) )
	ROM_LOAD( "td_20a_c.7_1", 0x0000, 0x010000, CRC(ea229917) SHA1(3e42c1eca1a89b2d536498156beddddcba9899b2) )
	ROM_LOAD( "td_20b_4.7_1", 0x0000, 0x010000, CRC(79468269) SHA1(709f34a0ebea816cb268b5dc36c3d02939cd6224) )
	ROM_LOAD( "td_20b_c.7_1", 0x0000, 0x010000, CRC(1301d28b) SHA1(b0fc0c73dedd89bbdb5845ec9f91530959fabeb6) )
	ROM_LOAD( "td_20bg4.7_1", 0x0000, 0x010000, CRC(4cb61b04) SHA1(6bb56cd06240c1bbb73406fe132e302822dec0df) )
	ROM_LOAD( "td_20bgc.7_1", 0x0000, 0x010000, CRC(8ce831d0) SHA1(e58ca3b38e8dc7196c27cf00123a6e7122bd7f58) )
	ROM_LOAD( "td_20bt4.7_1", 0x0000, 0x010000, CRC(2cdd5be2) SHA1(bc1afe70268eb7e3cb8fe1a43d262201faec0613) )
	ROM_LOAD( "td_20btc.7_1", 0x0000, 0x010000, CRC(67e96c75) SHA1(da9dd06f5d4773fa8e3945cf89cfdde4c465acb9) )
	ROM_LOAD( "td_25_bc.8_1", 0x0000, 0x010000, CRC(ac324184) SHA1(d6743c8cbbe719b12f47792a07ec2e898630591b) )
	ROM_LOAD( "td_25_dc.8_1", 0x0000, 0x010000, CRC(6ea8077c) SHA1(672976af1fad0257be7a15b839ec261653704be8) )
	ROM_LOAD( "td_25_kc.8_1", 0x0000, 0x010000, CRC(e006de48) SHA1(2c09e04d2dc3ec369c4c01eb1ff1af57156d05c1) )
	ROM_LOAD( "td_25a_c.8_1", 0x0000, 0x010000, CRC(84e54ba8) SHA1(dd09094854463f4b7033773be77d4a2d7f06b650) )
	ROM_LOAD( "td_25b_c.8_1", 0x0000, 0x010000, CRC(314f4f03) SHA1(a7c399ddf453305d0dbe2a63e57427b261c48c2c) )
	ROM_LOAD( "td_25bgc.8_1", 0x0000, 0x010000, CRC(efc0899c) SHA1(0d0e5a006d260a1bfcde7966c06360386c949f29) )
	ROM_LOAD( "td_25bgp.2_1", 0x0000, 0x010000, CRC(f0894f48) SHA1(63056dd434d18bb9a052db25cc6ce29d0c3f9f82) )
	ROM_LOAD( "td_25btc.8_1", 0x0000, 0x010000, CRC(f5dec7d9) SHA1(ffb361745aebb3c7d6bf4925d95904e8ced13a35) )
	ROM_LOAD( "td_30a_c.1_1", 0x0000, 0x010000, CRC(f0986895) SHA1(65c24de42a3009959c9bb7f5b42536aa6fd70c2b) )
	ROM_LOAD( "td_30b_c.1_1", 0x0000, 0x010000, CRC(7683cf72) SHA1(4319954b833ef6b0d88b8d22c5e700a9df96dc65) )
	ROM_LOAD( "td_30bdc.1_1", 0x0000, 0x010000, CRC(f5a4481b) SHA1(75b32b0996315b8ce833fd695377716dbeb0b7e4) )
	ROM_LOAD( "td_30bgc.1_1", 0x0000, 0x010000, CRC(1ffe440f) SHA1(adc1909fbbfe7e63bb89b29878bda5a6df776a6a) )
	ROM_LOAD( "td_30btc.1_1", 0x0000, 0x010000, CRC(5109516c) SHA1(a4919465286be9e1f0e7970a91a89738f8fcad4e) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "topdogsnd.bin", 0x0000, 0x080000, CRC(a29047c6) SHA1(5956674e6b895bd46b99f4d04d5797b53ccc6668) )
ROM_END


ROM_START( m4trex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr_20a__.2_1", 0x0000, 0x010000, CRC(21150b8e) SHA1(1531bc6fdb8b787fed6f4f98c6463313c55efc3c) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "tr_20a_p.2_1", 0x0000, 0x010000, CRC(ec1b35bc) SHA1(944959c6d1f8e9b0bb33c659b7c515cb7585fed0) )
	ROM_LOAD( "tr_20ab_.2_1", 0x0000, 0x010000, CRC(f4190bbf) SHA1(45c20c5e56f0bc39e3af5817eb6d705caef14b40) )
	ROM_LOAD( "tr_20bg_.2_1", 0x0000, 0x010000, CRC(14d8d80c) SHA1(6db190550b18401a067eeb26890af39612fc3012) )
	ROM_LOAD( "tr_20bt_.2_1", 0x0000, 0x010000, CRC(3ad19bf3) SHA1(4808f8bbb963be30eace75228d4d1decde3282b0) )
	ROM_LOAD( "tr_20s__.2_1", 0x0000, 0x010000, CRC(2e9460bd) SHA1(8e2ac58cb1686aadad38356a4d47e9502ddbaa52) )
	ROM_LOAD( "tr_20s_p.2_1", 0x0000, 0x010000, CRC(e39a5e8f) SHA1(d1d507040fefa959d71856395639778e898272aa) )
	ROM_LOAD( "tr_20sb_.2_1", 0x0000, 0x010000, CRC(fb98608c) SHA1(48b4fbdc289552131b621c870d7af082b6d8916e) )
	ROM_LOAD( "tr_20sbp.2_1", 0x0000, 0x010000, CRC(36965ebe) SHA1(f2adce7cc97b30b0bf67fd5698b867603973e87a) )
	ROM_LOAD( "tr_20sd_.2_1", 0x0000, 0x010000, CRC(868ea921) SHA1(3055816747b4773ec67669403a81420fabbe327e) )
	ROM_LOAD( "tr_20sdp.2_1", 0x0000, 0x010000, CRC(4b809713) SHA1(d70581ff669d19cf5a91b1546f5c02f27aeda2e4) )
	ROM_LOAD( "tr_20sk_.2_1", 0x0000, 0x010000, CRC(5382a910) SHA1(c8b2811081ec31fecd1b435e775d29e2e6406111) )
	ROM_LOAD( "tr_20skp.2_1", 0x0000, 0x010000, CRC(9e8c9722) SHA1(7aee25966e6d2107f8a8f89acf6af62a73ff05c9) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "tr______.1_2", 0x000000, 0x080000, CRC(75687514) SHA1(dc8f5f1db7da164175c241187cf3f0db1dd71fc9) )
	ROM_LOAD( "tr______.1_3", 0x080000, 0x080000, CRC(1e30d4ed) SHA1(8cd916d28f5060d74a0d795f9b75ab597de1cd60) )
ROM_END







ROM_START( m4volcan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "vo_sj___.5_0", 0x0000, 0x040000, CRC(78096ebf) SHA1(96915bc2eca00fbd82fab8b3f62e697da118acdd) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "vo_sj_d_.5_0", 0x0000, 0x040000, CRC(87e0347d) SHA1(be5d5b90739fa8ac10f6504290aa58fcf147f323) )
	ROM_LOAD( "vo_sj_k_.5_0", 0x0000, 0x040000, CRC(8604d102) SHA1(34c7df0257ba02ace4a74ffd5b0eed11eea0c333) )
	ROM_LOAD( "vo_sja__.5_0", 0x0000, 0x040000, CRC(d73bade2) SHA1(7e02493ec0710f109ae45e523ef3d7d275aaefab) )
	ROM_LOAD( "vo_sjs__.5_0", 0x0000, 0x040000, CRC(51eff796) SHA1(d0efb1eb4be176906726a438fcffb50cf5ddd217) )
	ROM_LOAD( "vo_vc___.2_0", 0x0000, 0x040000, CRC(24a9e5d6) SHA1(dd4223c3b5c024eb9d56bb45426e327b49f78dde) )
	ROM_LOAD( "vo_vc_d_.2_0", 0x0000, 0x040000, CRC(7f7341b6) SHA1(23d46ca0eed1e942b2a0d33d6ada2434ded5819b) )
	ROM_LOAD( "volcano_bwb_2-0.bin", 0x0000, 0x040000, CRC(20688684) SHA1(fe533341417a3a0b16f485351cb635f4e7d823db) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "vo___snd.1_1", 0x000000, 0x080000, CRC(62bb0166) SHA1(17e5557cce4e7841cbcf5d67783fe78452aacc63) )
	ROM_LOAD( "vo___snd.1_2", 0x080000, 0x080000, CRC(1eded545) SHA1(0010833e42b33fb0fd621a1059e1cf9a123c3fbd) )
	ROM_LOAD( "vo___snd.1_3", 0x100000, 0x080000, CRC(915f4adf) SHA1(fac6644329ee6ef0026d65d8b94c971e01770d45) )
	ROM_LOAD( "vo___snd.1_4", 0x180000, 0x080000, CRC(fec0fbe9) SHA1(26f651c5558a80e88666403d01cf916c3a13d948) )
ROM_END


ROM_START( m4vdexpr )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "vd_sj___.2_0", 0x0000, 0x040000, CRC(03efd2a5) SHA1(4fc3695c24335aef11ba168f660fb519d8c9d473) )

	ROM_REGION( 0x40000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "vd_sj_d_.2_0", 0x0000, 0x040000, CRC(5073b98e) SHA1(66b020b8c096e78e1c9694f1cbc139e97314ab48) )
	ROM_LOAD( "vd_sja__.2_0", 0x0000, 0x040000, CRC(c53dbf48) SHA1(ceee2de3ea8cb511540d90b87bc67bec3309de35) )
	ROM_LOAD( "vd_sjs__.2_0", 0x0000, 0x040000, CRC(036157b3) SHA1(b575751006c3ee59bf0404fa0e177fee9ef9c5db) )
	ROM_LOAD( "vd_vc___.1_0", 0x0000, 0x040000, CRC(6326e14a) SHA1(3dbfbb1cfb60dc10c6972aa2fda89c8e3c3107ea) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "vd___snd.1_1", 0x000000, 0x080000, CRC(d66b43a9) SHA1(087cf1571a9afb8c1c7cac13640fa453b614fd53) )
	ROM_LOAD( "vd___snd.1_2", 0x080000, 0x080000, CRC(a501c887) SHA1(c56a05fd8196afb86e665fec3fe7d02b9bf94c1a) )
	ROM_LOAD( "vd___snd.1_3", 0x100000, 0x080000, CRC(70c6bd96) SHA1(ecdd4276ff72939433630e04bba5be3df569e17e) )
	ROM_LOAD( "vd___snd.1_4", 0x180000, 0x080000, CRC(a6753f41) SHA1(b4af3054b62c3f00f2b5a19b816507fc3a62bef4) )

ROM_END



ROM_START( m4xch )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ec_25b__.b_0", 0x0000, 0x020000, CRC(cec9e836) SHA1(460ec38566d7608e51b62f1ffebc18a395002ed4) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "ec_36bg_.bv0", 0x0000, 0x020000, CRC(c5d1523a) SHA1(813916008d7e7576e4594a6eb79a76c514470f31) )
	ROM_LOAD( "ec_36bgn.bv0", 0x0000, 0x020000, CRC(4be33ee1) SHA1(888009e09c59f30649eac3238e0b70dec258cb3c) )
	ROM_LOAD( "ec_39b__.b_0", 0x0000, 0x020000, CRC(e5d5961d) SHA1(ac3916cba91a13d1e0820982a1cefabd378647c9) )
	ROM_LOAD( "ec_39bg_.b_0", 0x0000, 0x020000, CRC(49b2be01) SHA1(dbf808d0949a9658d23e57e7eaaa520891a4f0e0) )
	ROM_LOAD( "ec_39bm_.b_0", 0x0000, 0x020000, CRC(29c86cf0) SHA1(384a475d34ad5d15b68d019c3771aba471f89b4d) )
	ROM_LOAD( "ec_49bd_.b_0", 0x0000, 0x020000, CRC(02854189) SHA1(beab97ccd9889cfdb24e1fb6fb373bf9fd114eab) )
	ROM_LOAD( "ec_49bmd.b_0", 0x0000, 0x020000, CRC(2a5578e3) SHA1(d8f100bc83721b6b0f365be7a962249af79d6162) )
	ROM_LOAD( "ec_s9bt_.b_0", 0x0000, 0x020000, CRC(7e0a27d1) SHA1(d4a23a6c358e38a1a66a06b82af85c844f684830) )
	ROM_LOAD( "ec_sja__.a_0", 0x0000, 0x020000, CRC(1f923f89) SHA1(84486287d55591c7e81c59a10e8cc722ec21e8f9) )
	ROM_LOAD( "ec_sja__.b_0", 0x0000, 0x020000, CRC(16d7b8bb) SHA1(be8ab98a64aa976e25cb302b68323c6781034f2b) )
	ROM_LOAD( "xchange.bin", 0x0000, 0x010000, CRC(c96cd014) SHA1(6e32d10c18b6b34dbcb21e75925a77e810ffe892) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "xchasnd.bin", 0x0000, 0x080000, CRC(32c44cd5) SHA1(baafb48e6f95ba152942d1e1c273ffb3c95afa82) )
ROM_END


ROM_START( m4xs )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "es_39b__.3_0", 0x0000, 0x020000, CRC(ba478372) SHA1(c13f9cc4261e91119aa694ec3ac81d94d9f32d22) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "es_39bg_.3_0", 0x0000, 0x020000, CRC(b689f14f) SHA1(0c3253e1f747a979f55d53fe637fc61cf50e01a3) )
	ROM_LOAD( "es_39bm_.3_0", 0x0000, 0x020000, CRC(934f5d1e) SHA1(1ffd462d561d4a16f2392cc90a139499b74a234a) )
	ROM_LOAD( "es_39bmd.3_0", 0x0000, 0x020000, CRC(cf663dcc) SHA1(620e65528687eb5fd6fd879e305e8b7da9b95253) )
	ROM_LOAD( "es_49bg_.3_0", 0x0000, 0x020000, CRC(b76f1d7d) SHA1(9b43a9e847db3d4024f978b6f996534b8d52368b) )
	ROM_LOAD( "es_49bmd.3_0", 0x0000, 0x020000, CRC(1150e499) SHA1(25d2c37e5287f73d2b11608c50f21072422850f0) )
	ROM_LOAD( "es_sja__.3_0", 0x0000, 0x020000, CRC(5909092d) SHA1(64df6ad5ba5ac74592b525af2f4cab8a092a5766) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( m4xtrm )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "et_39bg_.2_0", 0x0000, 0x020000, CRC(db1a3c3c) SHA1(081c934ebfc0a9dfa195bb20f51e025e53d9c4b9) )

	ROM_REGION( 0x20000, "altrevs", 0 ) /* alternate revisions - to be sorted / split into clones in the future */
	ROM_LOAD( "et_49bg_.2_0", 0x0000, 0x020000, CRC(f858d927) SHA1(e7ab84c8898a95075a41fb0249e4b103d60e7d85) )
	ROM_LOAD( "et_sja__.2_0", 0x0000, 0x020000, CRC(8ee2602b) SHA1(b9a779b900ac71ec842dd7eb1643f7a2f1cb6a38) )

	ROM_REGION( 0x180000, "msm6376", ROMREGION_ERASE00 )
	/* missing? */
ROM_END






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

ROM_START( m4bangin )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang.hex", 0x0000, 0x020000, CRC(e40f21d3) SHA1(62319967882f01bbd4d10bca52daffd2fe3ec03a) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END

ROM_START( m4bangina )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang1-5n.p1", 0x0000, 0x020000, CRC(dabb462a) SHA1(c02fa204bfab07d5edbc784ccaca50f119ce8d5a) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END

ROM_START( m4banginb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "bang1-5p.p1", 0x0000, 0x020000, CRC(84bb8da8) SHA1(ae601957a3cd0b7e4b176987a5592d0b7c9be19d) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "bang.chr", 0x0000, 0x000048, CRC(aacbab22) SHA1(1f394b8947486f319743c0703884ecd35214c433) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "bangs1.hex", 0x000000, 0x080000, CRC(aa460a90) SHA1(e5e80d8b14bd976ed104e376c2e6f995870b0d77) )
	ROM_LOAD( "bangs2.hex", 0x080000, 0x080000, CRC(518ebd38) SHA1(6eaaf0cb34dd430f16b88f9d1ed97d6fb59d00ea) )
	ROM_LOAD( "bangs3.hex", 0x100000, 0x080000, CRC(2da78a75) SHA1(95975bc76fc32d05bd998bb75dcafc6eef7661b3) )
ROM_END


ROM_START( m4wwc )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "wack1-9n.p1", 0x0000, 0x020000, CRC(7ba6fd92) SHA1(3a5c7f9b3ebd8593c76132b46163c9d1299e210e) )

	ROM_REGION( 0x020000, "altrevs", 0 )
	ROM_LOAD( "wack1-9p.p1", 0x0000, 0x020000, CRC(4046b5eb) SHA1(e1ec9158810387b41b574202e9f27e7b741ac81c) )
	ROM_LOAD( "wacky.hex", 0x0000, 0x020000, CRC(a94a06fd) SHA1(a5856b6903fdd35f9dca19b114ca56c106a308f2) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	/* 2 sets of sound roms, one contains an extra sample */
	ROM_LOAD( "wacky1.hex", 0x000000, 0x080000, CRC(379d7af6) SHA1(3b1988c1ab570c075572d0e9bf03fcb331ea4a2c) )
	/* rom 2? should it match? */
	ROM_LOAD( "wacky3.hex", 0x100000, 0x080000, CRC(c7def11a) SHA1(6aab2b7f7e4c852891ee09e91a8a085e9b28803f) )

	ROM_LOAD( "wacky1snd.bin", 0x000000, 0x080000, CRC(45d6869a) SHA1(c1294522d190d22852b5c6006c92911f9e89cfac) )
	ROM_LOAD( "wacky2snd.bin", 0x080000, 0x080000, CRC(18b5f8c8) SHA1(e4dc312eea777c2375ba8c2be2f3c2be71bea5c4) )
	ROM_LOAD( "wacky3snd.bin", 0x100000, 0x080000, CRC(0516acad) SHA1(cfecd089c7250cb19c9e4ca251591f820acefd88) )
ROM_END


ROM_START( m4screw )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "scre0-8n.p1", 0x0000, 0x020000, CRC(5e07b33a) SHA1(6e8835edb61bd0777751bfdfe66d729554a9d6eb) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
ROM_END

ROM_START( m4screwp )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "scre0-8p.p1", 0x0000, 0x020000, CRC(34a70a77) SHA1(f76de47f6919d380eb0d0eeffc0e5dda72345038) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
ROM_END

ROM_START( m4screwa )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "screwinaroundv0-7.bin", 0x0000, 0x020000, CRC(78a1e3ca) SHA1(a3d6e76a474a3a5cd74e4b527aa575f21825a7aa) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
ROM_END

ROM_START( m4screwb )
	ROM_REGION( 0x020000, "maincpu", 0 )
	ROM_LOAD( "screw0_5.p1", 0x000000, 0x020000, CRC(4c0e8300) SHA1(1fea75f3cb1a96c14bd0e56a95bafd22996d002d) )

	ROM_REGION( 0x180000, "msm6376", 0 )
	ROM_LOAD( "screwsnd.p1", 0x000000, 0x080000, CRC(6fe4888c) SHA1(b02b7f322d22080123e8b18326910031aa9d39b4) )
	ROM_LOAD( "screwsnd.p2", 0x080000, 0x080000, CRC(29e842ee) SHA1(3325b137361c69244fffaa0d0e39e60106eaa5f9) )
	ROM_LOAD( "screwsnd.p3", 0x100000, 0x080000, CRC(91ef193f) SHA1(a356642ae1093cf69486c434673531042ae27be7) )
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




ROM_START( m4sstrek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhr2pprgpatched.bin", 0x0000, 0x010000, CRC(a0b3439d) SHA1(0976537a5170bf4c4f595f7fa04243a68f14b2ae) )
ROM_END






ROM_START( m4bigban )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "big04.p1", 0x0000, 0x020000, CRC(f7ead9c6) SHA1(46c10abb892cb6d427ad508aae96752c14b4cb83) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( m4crzcsn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "crz03.bin", 0x0000, 0x020000, CRC(48610c4f) SHA1(a62ac8b3ee704ee4e98f9d56bfc723d4cbb25b54) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( m4crzcav )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gcv05.p1", 0x0000, 0x020000, CRC(b9ba46f6) SHA1(78b745d85b36444c39747982987088a772b20a7e) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( m4dragon )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dgl01.p1", 0x0000, 0x020000, CRC(d7d39c9b) SHA1(5350c9db549edee30815516b1ce74a018390ff3d) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( m4hilonv )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "hnc02.p1", 0x0000, 0x020000, CRC(33a8022b) SHA1(5168b8f32630aa2cb56f30c941695f1728e4fb7a) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( m4octo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "oct03.p1", 0x0000, 0x020000, CRC(8df66e94) SHA1(e1ab93982846d83becae36b5814ebbd515b9078e) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END

ROM_START( m4sctagt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gse3_0.p1", 0x0000, 0x010000, CRC(eff705ff) SHA1(6bf96872ef4bcc8f8041c5384d892f072c72be2b) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* Missing? */
ROM_END










ROM_START( m4cld02 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cru0_2.bin", 0x0000, 0x010000, CRC(e3c01944) SHA1(33a2b2c05686f53811349b2980e590fdc4b72756) )
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
	ROM_LOAD( "d.n.j 1-03", 0x0000, 0x010000, CRC(7b805255) SHA1(f62765bfa66e2422ac0a71ebaff27f1ccd470fe2) )
	ROM_LOAD( "d.n.j 1-06", 0x0000, 0x010000, CRC(aab770c7) SHA1(f24fff8346915017bc43fef9fac356a067676d86) )
ROM_END


ROM_START( m4matdr )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "matador.bin", 0x0000, 0x020000, CRC(367788a4) SHA1(3c9b077a64f993cb60107558efdfcbee0fe5c958) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* missing */
ROM_END



ROM_START( m4hslo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hot30", 0x0000, 0x010000, CRC(62f2c420) SHA1(5ae89a1b585738255e8d9ae153c3c63b4a2893e4) )
ROM_END

ROM_START( m4sbx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbx-2.1-cash.bin", 0x8000, 0x008000, CRC(2dca703e) SHA1(aef398f4ed38ba34f28009058c9486a570f64e0f) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "b_sbx23.bin", 0x8000, 0x008000, CRC(8188e94f) SHA1(dfbfc549d12c8f7c7db6c12ba766c28f1cf0873f) )
	ROM_LOAD( "s bears v1-4 20p po.bin", 0x8000, 0x008000, CRC(03486714) SHA1(91c237956bbec58cc08a3e92543488d8e2daa673) )
	ROM_LOAD( "s bears v2-4 10p 8.bin", 0x8000, 0x008000, CRC(9b94f8d0) SHA1(9808386def14c8a058730e90135a4d6506e6ed3d) )
	ROM_LOAD( "s bears v2-4 20p po.bin", 0x8000, 0x008000, CRC(ad8f8d9d) SHA1(abd808f95b587a84e8b3aad1af9fe1cb613c9821) )
	ROM_LOAD( "superbea.10p", 0x8000, 0x008000, CRC(70020466) SHA1(473c9feb9ce0024b870612af19ec8a47a7798506) )

	ROM_REGION( 0x40000, "upd", 0 ) // not oki at least...
	ROM_LOAD( "sbsnd", 0x0000, 0x040000, CRC(27fd9fe6) SHA1(856fdc95a833affde0ada7041c68a4b6b729b715) )
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

ROM_START( m4jungj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jj2410p.bin", 0x8000, 0x008000, CRC(490838c6) SHA1(a1e9963df9a429ae594592312e977f22f96c6073) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "jj2420p.bin", 0x8000, 0x008000, CRC(39329ccf) SHA1(6b79e4fc553bad935ec9989ad5ef3e186e720633) )
	ROM_LOAD( "jjv2_4p.bin", 0x8000, 0x008000, CRC(125a8138) SHA1(18c62df5b331bd09d6dcda6280351e94b7b816fd) )
	ROM_LOAD( "jjv4.bin", 0x8000, 0x008000, CRC(bf583156) SHA1(084c5ed3d96c92f265ad08cc7aed7fe6092217a5) )

	ROM_REGION( 0x40000, "upd", ROMREGION_ERASE00 ) // not oki at least...
ROM_END


ROM_START( m4fsx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "funspotx.10p", 0x8000, 0x008000, CRC(55199f36) SHA1(7af376781e381582b06972725a2022cc28ba60b3) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "funspotx.20p", 0x8000, 0x008000, CRC(08d1eb6e) SHA1(7c7c02d9c34696d75490df8596ffe64fba93dcc4) )
	ROM_LOAD( "b_fsv1.bin", 0x8000, 0x008000, CRC(b077f944) SHA1(97d96594b8d2d7232bad087cc55912dec02d7484) )

	ROM_REGION( 0x40000, "upd", ROMREGION_ERASE00 ) // not oki at least...
ROM_END


ROM_START( m4ccop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashcop9.bin", 0x0000, 0x010000, CRC(5f993207) SHA1(ab0614e6a1355d275158b1a32f65086e40c2f890) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "cash-cops_v4-de.bin", 0x0000, 0x010000, CRC(df3da824) SHA1(c275a33e4a89f1b9ecbae80cb7b62007b29b9fd2) )
	ROM_LOAD( "cashcop8.bin", 0x0000, 0x010000, CRC(165603df) SHA1(d301696a340ed136a43c5753c8bf73283a925fd7) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	// 3 different sets of samples(!)
	ROM_LOAD( "cash-copssnd1-de.bin", 0x000000, 0x080000, CRC(cd03f7f7) SHA1(4c09a86bcdf9a9eb224b19b932b75c9db3784fad) )
	ROM_LOAD( "cash-copssnd2-de.bin", 0x080000, 0x080000, CRC(107816a2) SHA1(f5d4a0390b85a665a3536da4689ec91b1a2da3ae) )

	ROM_LOAD( "cash-copssnd1.bin", 0x000000, 0x080000, CRC(776a303d) SHA1(a5a282674674f25bc6ca169eeebee7309239871f) )
	ROM_LOAD( "cash-copssnd2.bin", 0x080000, 0x080000, CRC(107816a2) SHA1(f5d4a0390b85a665a3536da4689ec91b1a2da3ae) )

	ROM_LOAD( "cashcops.p1", 0x000000, 0x080000, CRC(9a59a3a1) SHA1(72cfc99b22ec5fb89714c6d2d66760d86dc19f2f) )
	ROM_LOAD( "cashcops.p2", 0x080000, 0x080000, CRC(deb3e755) SHA1(01f92881c451919be549a1c58afa1fa4630bf171) )
ROM_END

ROM_START( m4ccc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ccc12.bin", 0x8000, 0x008000, CRC(570cc766) SHA1(036c95ff6428ab38cceb0537dcc990be78fb331a) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "criss cross crazy sound (27c2001)", 0x0000, 0x040000, CRC(1994c509) SHA1(2bbe91a43aa9953b7776faf67e81e30a4f7b7cb2) )
ROM_END


ROM_START( m4treel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trgv1.1s", 0x0000, 0x010000, CRC(a9c76b08) SHA1(a5b3bc980eb58e346cb02d8ca43401f304e5b6de) )

	ROM_REGION( 0x20000, "altrevs", 0 )
	ROM_LOAD( "trgv1.1b", 0x0000, 0x020000, CRC(7eaebef6) SHA1(5ab86329041e7df09cc2e3ce8d5afd44d88c246c) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
ROM_END



ROM_START( m4unkjok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joker 10p 3.bin", 0x0000, 0x010000, CRC(009823ac) SHA1(5ab25da5876c87a8d8701f84446bb3d377e4c1ca) )
	ROM_LOAD( "joker 10p 6.bin", 0x0000, 0x010000, CRC(f25f0704) SHA1(35298b49f79c5029277f4777fe88d5e4344c115f) )
	ROM_LOAD( "joker 20p 3 or 6.bin", 0x0000, 0x010000, CRC(cae4397e) SHA1(53b61fd41c97a6ed29ce6a7b555e061ecf2b0ae2) )
	ROM_LOAD( "joker new 20p 6 or 3.bin", 0x0000, 0x010000, CRC(b8d77b97) SHA1(54f69823bb3fd9c2cca014dc7c51913b2d6c8058) )
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

ROM_START( m4t266 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "t2 66.bin", 0x0000, 0x010000, CRC(5c99c6bb) SHA1(7b74e0e5207c00b31cb1859e0cc458c0412a1a07) )
ROM_END

ROM_START( m4brnze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bv25", 0x0000, 0x010000, CRC(5c66f460) SHA1(c7587a6e992549ad8814f77c65b33a17a3641431) )
	ROM_LOAD( "bv25v2", 0x0000, 0x010000, CRC(a675edb3) SHA1(a3c6ee6a0bfb301fed72b45ee8e363d77b8b8dbb) )
	ROM_LOAD( "bv55", 0x0000, 0x010000, CRC(93905bc9) SHA1(e8d3cd125dced43fc2cf23cbccc59110561d2a40) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	/* missing */
ROM_END

ROM_START( m4riotrp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drt10.bin", 0x0000, 0x010000, CRC(a1badb8a) SHA1(871786ea4e65ecbf61c9a776100321253922d11e) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "dblcsnd.bin", 0x0000, 0x080000, CRC(c90fa8ad) SHA1(a98f03d4b6f5892333279bff7537d4d6d887da62) )
ROM_END


















ROM_START( m4surf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "s_surfin._pound5", 0x0000, 0x020000, CRC(5f800636) SHA1(5b1789890eea44e5275e13f360876374d862935f) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "s_surfin.upd", 0x0000, 0x020000, CRC(d0bef9cd) SHA1(9d53bfe8d928b190202bf747c0d7bb4cc0ae0efd) )
	ROM_LOAD( "s_surfin._pound15", 0x0000, 0x020000, CRC(eabce7fd) SHA1(4bb2bbcc7d2917eca72385a21ab85d2d94a882ec) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "s_surf.sn1", 0x000000, 0x080000, CRC(f20a7d69) SHA1(7887230613b497dc71a60125dd1e265ebbc8eb23) )
	ROM_LOAD( "s_surf.sn2", 0x080000, 0x080000, CRC(6c4a9074) SHA1(3b993120156677de893e5dc1e0c5d6e0285c5570) )
ROM_END

ROM_START( m4wife )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "moy_wife.p1", 0x0000, 0x020000, CRC(293d35a6) SHA1(980a28ca5e9ec3ca2e1a5b34f658b622dca4cf50) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	// missing?
ROM_END



ROM_START( m4blkgd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "blackgoldprg.bin", 0x0000, 0x080000, CRC(a04736b2) SHA1(9e060cc79e7922b38115f1412ed76f8c76deb917) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "blackgoldversion2.4.bin", 0x0000, 0x040000, CRC(fad4e360) SHA1(23c6a13e8d1ca307b0ef22edffed536675985aca) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "blackgoldsnd1.bin", 0x000000, 0x080000, CRC(d251b59e) SHA1(960b81b87f0fb5000028c863892a273362cb897f) )
	ROM_LOAD( "blackgoldsnd2.bin", 0x080000, 0x080000, CRC(87cbcd1e) SHA1(a6cd186af7c5682e216f549b77735b9bf1b985ae) )
	ROM_LOAD( "blackgoldsnd3.bin", 0x100000, 0x080000, CRC(258f7b83) SHA1(a6df577d98ade8c5c5ff68ef891667e65e83ac17) )
ROM_END


ROM_START( m4excam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ex1_4.bin", 0x0000, 0x010000, CRC(34c4aee2) SHA1(c5487c5b0144ca188bc2e3926a0343fd4c9c565a) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "ex1_0d.bin", 0x0000, 0x010000, CRC(490c510e) SHA1(21a03d8e2dd4d2c7760acbff5705f925fe9f31be) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "mdmexcalibsnd.p1", 0x000000, 0x080000, CRC(8ea73366) SHA1(3ee45ad98e03177eeef97521df7b3d1945242076) )
	ROM_LOAD( "mdmexcalibsnd.p2", 0x080000, 0x080000, CRC(0fca6ca2) SHA1(2029d15e3b51069f5847ab3846bf6c064f0a3381) )
	ROM_LOAD( "mdmexcalibsnd.p3", 0x100000, 0x080000, CRC(43be816a) SHA1(a95f702ec1bb20f3e0f18984948963b56769f5ba) )
	ROM_LOAD( "mdmexcalibsnd.p4", 0x180000, 0x080000, CRC(ef8a718c) SHA1(093a5fff5bab61fc9276a7f9f3c5b728a50603b3) )
ROM_END


ROM_START( m4front )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ff2_1.bin", 0x0000, 0x010000, CRC(3519cba1) SHA1(d83a5370ee82e258024d20ffacec7050950b1326) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "ffsnd2_1.bin", 0x000000, 0x080000, CRC(9b3cdf12) SHA1(1a209985493f686dd37e91693361ecbf32096f66) )
	ROM_LOAD( "ffsnd2_2.bin", 0x080000, 0x080000, CRC(0fc33bdf) SHA1(6de715e33411050ee1d2a0f08bf1c9a8001ffb4f) )
ROM_END







ROM_START( m4safar )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "saf4_1.bin", 0x0000, 0x010000, CRC(ad726457) SHA1(4104be61d179024fae9fb9c631677b1ba56d3f00) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )

ROM_END

ROM_START( m4snowbl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sb_3.p2", 0x8000, 0x004000, CRC(c22fe04e) SHA1(233ee0795b0029389247a9550ef39af95f671870) )
	ROM_LOAD( "sb_3.p1", 0xc000, 0x004000, CRC(98fdcaba) SHA1(f4a74d5550dd9fc8bff35a583b3289e1bb0be9d5) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4zill )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "zillprgh.bin", 0x0000, 0x080000, CRC(6f831f6d) SHA1(6ab6d7f1752d27bc216bc11533b90178ce188715) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "zillprog.bin", 0x0000, 0x080000, CRC(0f730bab) SHA1(3ea82c8f7d62c70897a5c132273820c9f192cd72) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "zillsnd.bin", 0x0000, 0x080000, CRC(171ed677) SHA1(25d63f4d9c64f13bec4feffa265c5b0c5f6be4ec) )
ROM_END

ROM_START( m4hstr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "h_s_v1_2.bin", 0x0000, 0x010000, CRC(ef3d3461) SHA1(aa5b1934ab1c6739f36ac7b55d3fda2c640fe4f4) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "happystreak.p1", 0x0000, 0x080000, CRC(b1f328ff) SHA1(2bc6605965cb5743a2f8b813d68cf1646a4bcac1) )
	ROM_LOAD( "hs2_5.bin", 0x0000, 0x010000, CRC(f669a4c9) SHA1(46813ba7104c97eaa851b50019af9b80046d03b3) )
	ROM_LOAD( "hs2_5p.bin", 0x0000, 0x010000, CRC(71c981aa) SHA1(5effe7487e7216078127d3dc4a0a7ad02ad84390) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "happystreak.p1", 0x0000, 0x080000, CRC(b1f328ff) SHA1(2bc6605965cb5743a2f8b813d68cf1646a4bcac1) ) // alt sound rom

	ROM_LOAD( "happystreaksnd.p1", 0x0000, 0x080000, CRC(76cda195) SHA1(21a985cd6cf1f63f4aa799563099a0527a7c0ea2) )
	ROM_LOAD( "happystreaksnd.p2", 0x080000, 0x080000, CRC(f3b4c763) SHA1(7fd6230c13b66a16daad9d45935c7803a5a4c35c) )
ROM_END

ROM_START( m4hstrcs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chs3_6.bin", 0x0000, 0x010000, CRC(d097ae0c) SHA1(bd78c14e7f057f173859bcb1db5e6a142d0c4062) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "chs3_6p.bin", 0x0000, 0x010000, CRC(57378b6f) SHA1(cf1cf528b9790c1013d87ccf63dcbf59f365067f) )
	ROM_LOAD( "chs3_6pk.bin", 0x0000, 0x010000, CRC(f95f1afe) SHA1(fffa409e8c7148a840d5dedf490fd9f6975e9476) )
	ROM_LOAD( "chs3_6k.bin", 0x0000, 0x010000, CRC(7eff3f9d) SHA1(31dedb0d9476633e8eb947a687c7b8a94b0e182c) )
	ROM_LOAD( "chs_4_2.bin", 0x0000, 0x010000, CRC(ec148b65) SHA1(2d6252ce68719281f5597955227a1f662743f006) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	/* might use the same roms as the normal version here */
ROM_END


ROM_START( m4ddb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ddb3_1.bin", 0x0000, 0x010000, CRC(3b2da727) SHA1(8a677be3b82464d1bf1e97d22adad3b27374079f) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "ddb3_1p.bin", 0x0000, 0x010000, CRC(bc8d8244) SHA1(9b8e0706b3add42e5e4a8b6c6a2f80a333a2f49e) )

	ROM_REGION( 0x200000, "msm6376", 0 )
	ROM_LOAD( "ddbsound1", 0x000000, 0x080000, CRC(47c87bd5) SHA1(c1578ae553c38e93235cea2142cb139170de2a7e) )
	ROM_LOAD( "ddbsound2", 0x080000, 0x080000, CRC(9c733ab1) SHA1(a83c3ebe99703bb016370a8caf76bdeaff5f2f40) )
ROM_END

ROM_START( m4hapfrt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf1_1.bin", 0x0000, 0x010000, CRC(6c16cb05) SHA1(421b164c8410629956177355e505859757c97a6b) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "hf1_1p.bin", 0x0000, 0x010000, CRC(ebb6ee66) SHA1(1f9b67260e5becd013d95358cc89acb1099d655d) )
	ROM_LOAD( "hf1_4pk.bin", 0x0000, 0x010000, CRC(0944b3c6) SHA1(00cdb75dda4f8984f77806047ad79fe9a1a8760a) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END




ROM_START( m4ewshft )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "each_way_shifter(mdm)_v1-0.bin", 0x0000, 0x010000, CRC(506b6cf0) SHA1(870e356b9785e51c5be5d6bc6af9ea7640b51ee8) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "each_way_shifter-snd1.bin", 0x000000, 0x080000, CRC(b21f9b09) SHA1(69ac3ca2874fc3aebd34dd225a195ad1c0305d00) )
	ROM_LOAD( "each_way_shifter-snd2.bin", 0x080000, 0x080000, CRC(e3ce5ec5) SHA1(9c7eefa4042b1b1aca3d0fbefcad10db34992c43) )
ROM_END

ROM_START( m4jiggin )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jig2-1n.p1", 0x0000, 0x010000, CRC(9ea16d00) SHA1(4b4f1519eb6565ce76665595154c58cd0d0ab6fd) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "jig2-1p.p1", 0x0000, 0x010000, CRC(09e6e111) SHA1(800a1dbc64c6a631cf3e53bd5f17b5d56955c92e) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "jigsnd1.oki", 0x000000, 0x080000, CRC(581fa143) SHA1(e35186597fc7932d306080ecc82c55af4b769367) )
	ROM_LOAD( "jigsnd2.oki", 0x080000, 0x080000, CRC(34c6fc3a) SHA1(6bfe52a94d8bed5b30d9ed741db7816ddc712aa3) )
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
	ROM_REGION( 0x200000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "nodsnd.bin", 0x0000, 0x080000, CRC(2134494a) SHA1(3b665bf79567a71195b20e76c50b02707d15b78d) )
ROM_END


ROM_START( m4dcrls )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "70000116.bin", 0x0000, 0x040000, CRC(27e5ad77) SHA1(83cabd8b52efc6c0d5530b55683295208f64abb6) ) // dcr_std_340.bin

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "70000117.bin", 0x0000, 0x080000, CRC(4106758c) SHA1(3d2b12f1820a65f00fd70856b7765b6f35a8688e) )
	ROM_LOAD( "70000118.bin", 0x0000, 0x080000, CRC(3603f93c) SHA1(cb969568e0244b465f8b120faba3adb65fe001e6) )
	ROM_LOAD( "70000134.bin", 0x0000, 0x080000, CRC(a81e80e7) SHA1(852c0b3afe8c22b6e6afe585efb8fec7aeb2aecb) )
	ROM_LOAD( "70001115.bin", 0x0000, 0x040000, CRC(26432b07) SHA1(ef7303793252210f3fd07b12f5684b5d2cc828ab) ) // dcr_data_340_lv.bin
	ROM_LOAD( "70001116.bin", 0x0000, 0x040000, CRC(522191e8) SHA1(f80656290295b556d4b67c4458d8f856f8b937fb) )
	ROM_LOAD( "70001117.bin", 0x0000, 0x080000, CRC(7dcf4e36) SHA1(593089aec7efe8b14b953c5d7b0f552c0906730a) )
	ROM_LOAD( "70001118.bin", 0x0000, 0x080000, CRC(6732182c) SHA1(aa6620458d381fc37c226f996eab12840573cf80) )
	ROM_LOAD( "70001134.bin", 0x0000, 0x080000, CRC(a382156e) SHA1(db884dac04f556ecf49f7ecaba0bc3e51a4822f8) )
	ROM_LOAD( "70001172.bin", 0x0000, 0x080000, CRC(32040f0f) SHA1(9f0452bc33e292ce61650f60f2943a3cef0da050) )
	ROM_LOAD( "70001173.bin", 0x0000, 0x080000, CRC(0d5f138a) SHA1(e65832d01b11010a7c71230596e3fbc2c750d175) )
	ROM_LOAD( "dcr_data_340.bin", 0x0000, 0x010000, CRC(fc12e68f) SHA1(f07a42323651ef9aefac24c3b9296a98068c2dc2) )
	ROM_LOAD( "dcr_gala_hopper_340.bin", 0x0000, 0x040000, CRC(e8a19eda) SHA1(14b49d7c9b8ad7c3f8605b2a57740aab2b98d030) )
	ROM_LOAD( "dcr_gala_hopper_340_lv.bin", 0x0000, 0x040000, CRC(e0d08c0e) SHA1(7c6a4e30bacfcbd895e418d4ce66425ec4f118f9) )
	ROM_LOAD( "dcr_mecca_340.bin", 0x0000, 0x040000, CRC(f18ac60f) SHA1(ffdd8d096ebc062a36a8d22cf881d0fa95adc2db) )
	ROM_LOAD( "dcr_mecca_340_lv.bin", 0x0000, 0x040000, CRC(0eb204c1) SHA1(648f5b90776f99155fd54257aabecb8c9f90abec) )
	ROM_LOAD( "dcr_std_340_lv.bin", 0x0000, 0x040000, CRC(d9632301) SHA1(19ac680f00e085d94fc45f765c975f3da1ca1eb3) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "dcr_sounds.bin", 0x0000, 0x09664e, CRC(431cecbc) SHA1(b564ae8d083fef84328526192626a220e979d5ad) ) // intelhex
	ROM_LOAD( "71000110.bin", 0x0000, 0x080000, CRC(0373a197) SHA1(b32bf521e36b5a53170d3a6ec545ce8db3a5094d) )
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


ROM_START( m4bluesn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bluesboys.bin", 0x0000, 0x020000, CRC(c1395649) SHA1(3cd0eed1f966f5391fe5de496dc747385ebfb556) )
	ROM_RELOAD(0x20000,0x20000)

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "bluesboyz.bi2", 0x000000, 0x080000, CRC(7f19a61b) SHA1(dd8742d84df24e118bdbffb1efffad1c71eb2283) )
	ROM_LOAD( "bluesboyz.bi3", 0x080000, 0x080000, CRC(32363184) SHA1(8f3f53ce4d9f9b54c441263def9d8e23880507a1) )
	ROM_LOAD( "bluesboyz.bi4", 0x100000, 0x080000, CRC(aa94281d) SHA1(e15b7bf97b8e307ed465d9b8cb6e5de0044f6fb5) )
	ROM_LOAD( "bluesboyz.bi5", 0x180000, 0x080000, CRC(d8d7aa2e) SHA1(2d8b86fa63e6649d628c7e343d8f5c329c8f8ced) )
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

ROM_START( m4cfinln )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cfd_7_1.bin", 0x0000, 0x020000, CRC(e42ec2aa) SHA1(6495448c1d11ce0ab9ad794bc3a0981432e22945) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "altrevs", 0 )
	// 'Jackpot Version'
	ROM_LOAD( "cfd_d0.bin", 0x0000, 0x020000, CRC(179fcf13) SHA1(abd18ed28118ba0a62ab321a9d963105946d5eef) )
ROM_END


ROM_START( m4dcrazy )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "70000130.bin", 0x0000, 0x080000, CRC(9d700a27) SHA1(c73c7fc4233dace32fac90a6b46dba5c12979160) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "70000131.bin", 0x0000, 0x080000, CRC(a7fcfcc8) SHA1(cc7b164e79d86d68112ac86e1ab9e81885cfcabf) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4ftladn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "fidse___.5_0", 0x00000, 0x20000, CRC(62347bbf) SHA1(2b1cd5adda831a8c74c9484ee1b616259d3e3981) )
	ROM_RELOAD(0x20000,0x20000)

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



ROM_START( m4jungjk )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "jjsoft_v550_1346_e7a3_lv.bin", 0x0000, 0x040000, CRC(c5315a0c) SHA1(5fd2115e033e0310ded3cfb39f31dc31b4d6bb5a) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "70000102.bin", 0x0000, 0x040000, CRC(e5f03540) SHA1(9a14cb4eade9f6b1c6d6cf78306259dbc108f1a5) )
	ROM_LOAD( "jj.bin", 0x0000, 0x040000, CRC(9e15c1b6) SHA1(9d4f3707f2cc2f0e8eb9051181bf8b368be3cbcf) )
	ROM_LOAD( "jjlump_v400_19a3.bin", 0x0000, 0x040000, CRC(bc86c415) SHA1(6cd828578835dafe5d8d46810dc70d47abd4e8b2) )
	ROM_LOAD( "70000092.bin", 0x0000, 0x040000, CRC(6530bc6c) SHA1(27819e760c84fbb40f354e87910fb15b3058e2a8) )
	ROM_LOAD( "jungle.p1", 0x0000, 0x080000, CRC(ed0eb72c) SHA1(e32590cb3eb7d07fb210bee1be3c0ee01554cb47) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "71000080.lo.hex", 0x0000, 0x134084, CRC(f3866082) SHA1(f33f6d7e078d7072cc7c67672b3afa3e90e1f805) )
	ROM_LOAD( "71000080.hi.hex", 0x0000, 0x12680f, CRC(2a9db1df) SHA1(73823c3db5c68068dadf6d9b4c93b47c0cf13bd3) )
	ROM_LOAD( "71000080.p1", 0x000000, 0x080000, CRC(b39d5e03) SHA1(94c9208601ea230463b460f5b6ea668363d239f4) )
	ROM_LOAD( "71000080.p2", 0x080000, 0x080000, CRC(ad6da9af) SHA1(9ec8c8fd7b9bcd1d4c6ed93726fafe9a50a15894) )
ROM_END

ROM_START( m4clab )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "70000019.bin", 0x0000, 0x040000, CRC(23a12863) SHA1(4047cb8cbc03f96f2b8681b6276e100e8e9194a5) )

	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "70000020.bin", 0x0000, 0x040000, CRC(88af7368) SHA1(14dea4267a4365286eea1e02b9b44d4053618cbe) )
	ROM_LOAD( "70000052.bin", 0x0000, 0x040000, CRC(99e60d45) SHA1(ec28bdd4ffb9674c2e9f8ab72aac3cb6011e7d6f) )
	ROM_LOAD( "70000053.bin", 0x0000, 0x040000, CRC(3ddf8b29) SHA1(0087ccd3429c081a3121e97d649914ab2cf8caa6) )
	ROM_LOAD( "clab_v300_1028_c23a_lv_10p.bin", 0x0000, 0x040000, CRC(c2ca098a) SHA1(e2bf80366af925a4c880a3377b79d494760f286f) )
	ROM_LOAD( "clab_v300_1031_3fb7_nlv_10p.bin", 0x0000, 0x040000, CRC(1502527f) SHA1(8a3dd5600ad9a073ea6fb8412178daa575002e56) )
	ROM_LOAD( "clab_v300_1033_1175_nlv_20p.bin", 0x0000, 0x040000, CRC(ba26efd7) SHA1(bf9c0dbf6882ddc42680c7d23c9b858eb12e5646) )
	ROM_LOAD( "clab_v400_1249_0162_nlv.bin", 0x0000, 0x040000, CRC(df256c84) SHA1(f4d0fc5acd7d0ac770cae548744ce18dfb9ec67c) )
	ROM_LOAD( "clab_v410_1254_1ca2_lv.bin", 0x0000, 0x040000, CRC(6e2ee8a9) SHA1(e64a01c93b879c9ade441bd4f1ce381f6e65a655) )
	ROM_LOAD( "clab_v410_1254_6e11_nlv.bin", 0x0000, 0x040000, CRC(efe4fcb9) SHA1(8a2f02593c7fbea060c78a98abd82fd970661e05) )
	ROM_LOAD( "clabrom", 0x0000, 0x040000, CRC(d80ecff5) SHA1(2608e95b718ecd49d880fd9911cb97e6644a307d) )


	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "71000010.lo", 0x0000, 0x134084, CRC(c39bbae4) SHA1(eee333376612a96a4c344729a96cc60c217bfde3) )
	ROM_LOAD( "71000010.hi", 0x0000, 0x091c0b, CRC(d0d3cb4f) SHA1(eaacf9ed3a6b6dcda8e1a3edbc3a9a2a51ffcbd8) )
	ROM_LOAD( "clab_snd1_c8a6.bin", 0x0000, 0x080000, CRC(cf9de981) SHA1(e5c73e9b9db9ac512602c2dd586ca5cf65f98bc1) )
	ROM_LOAD( "clab_snd2_517a.bin", 0x080000, 0x080000, CRC(d4eb949e) SHA1(0ebbd1b5e3c86da94f35c69d9d60e36844cc4d7e) )
ROM_END



ROM_START( m4looplt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "70000500.bin", 0x0000, 0x080000, CRC(040699a5) SHA1(e1ebc23684c5bc1faaac7409d2179488c3022872) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "70000500a.bin", 0x0000, 0x080000, CRC(0668f52d) SHA1(6560309facf0022e3c14421b848f212b18be7550) )
	ROM_LOAD( "70000501.bin", 0x0000, 0x080000, CRC(e2fbbfcf) SHA1(fc060468bf5e732626af8c3d0d6fc119a529c330) )
	ROM_LOAD( "70000501a.bin", 0x0000, 0x080000, CRC(42bef934) SHA1(c332eb6566ef5f9ac56d1c3944635296c21b3193) )
	ROM_LOAD( "70000504.bin", 0x0000, 0x080000, CRC(15e2c1c3) SHA1(69257749f1909b7ecc9c94cc2a27a5d4e6608251) )
	ROM_LOAD( "70000505.bin", 0x0000, 0x080000, CRC(f28f59bd) SHA1(d5cdb0c020693c7922c5243f9d18054d47ed039d) )
	ROM_LOAD( "70000506.bin", 0x0000, 0x080000, CRC(a3d40e9a) SHA1(97ac40e814824450e6705bc3240fffd4d0015b46) )
	ROM_LOAD( "70000507.bin", 0x0000, 0x080000, CRC(756eefe4) SHA1(b253fbd94fdab5df32375a02d16d9ba333e8d71c) )
	ROM_LOAD( "70001500.bin", 0x0000, 0x080000, CRC(0b9761a4) SHA1(e7a5e4b90d2e60808a7797d124308973130c440d) )
	ROM_LOAD( "70001500a.bin", 0x0000, 0x080000, CRC(09f90d2c) SHA1(addfd0d20ef9cafba042aa05ee84db85f060b67a) )
	ROM_LOAD( "70001501.bin", 0x0000, 0x080000, CRC(6ce9f76c) SHA1(467701786f8de136c9780a4ef93be6bb932d235d) )
	ROM_LOAD( "70001501a.bin", 0x0000, 0x080000, CRC(ccacb197) SHA1(c7573f309e9c79b2999229c46f78fd0283c4a064) )
	ROM_LOAD( "70001504.bin", 0x0000, 0x080000, CRC(1a7339c2) SHA1(575477d8abe3765d9cd4345336d0f7fa3a69202a) )
	ROM_LOAD( "70001505.bin", 0x0000, 0x080000, CRC(7c9d111e) SHA1(8f98feb70cdcd77b5e7bb6a015c935403a53f428) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "71000500.bin", 0x0000, 0x080000, CRC(94fe58f4) SHA1(e07d8e6d4b1e660abc4fa08d703fc0e586f3570d) )
ROM_END


ROM_START( m4mgpn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mgp15.p1", 0x0000, 0x010000, CRC(ec76233f) SHA1(aa8595c639c83026d7fe5c3a161f8b08ff9a8b46) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mgpsnd.p1", 0x000000, 0x080000, CRC(d5f0b845) SHA1(6d97d0d4d07407bb0a51e1d62da95c664418a9e9) )
	ROM_LOAD( "mgpsnd.p2", 0x080000, 0x080000, CRC(cefeea06) SHA1(45142ca1bab898dc6f3c32e382ee9157132810a6) )
	ROM_LOAD( "mgpsnd.p3", 0x100000, 0x080000, CRC(be4b3bd0) SHA1(f14c08dc770a24db8bbd00a65d3edf6ee9895ca3) )
	ROM_LOAD( "mgpsnd.p4", 0x180000, 0x080000, CRC(d74b4b03) SHA1(a35c99040a72485a6c2d4a4fdfc203634f6a9ad0) )
ROM_END


ROM_START( m4olygn )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ogdsx___.8_0", 0x0000, 0x040000, CRC(b51a2538) SHA1(d54f37dc14c44ab66e6d6ba6e2df8bc9ed003054) )
	ROM_REGION( 0x40000, "altrevs", 0 )
	ROM_LOAD( "ogdsxe__.8_0", 0x0000, 0x040000, CRC(13aa70aa) SHA1(3878c181ec07e24060935bec96e5128e6e4baf31) )
	//ROM_LOAD( "state.rst", 0x0000, 0x0000d0, CRC(e43a9cc2) SHA1(478edb0211224ad2732bf9e99160cf72c991edb9) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4rhnote )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "70000120.bin", 0x0000, 0x040000, CRC(d1ce1e1c) SHA1(2fc2b041b4e9fcade4b2ce6a0bc709f4174e2d88) )
	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "70000121.bin", 0x0000, 0x040000, CRC(1e1a26c0) SHA1(8a80a94d280c82887a0f7da607988597df23e1fb) )
	ROM_LOAD( "70000125.bin", 0x0000, 0x080000, CRC(67a617a2) SHA1(3900c0cc3f8e4d52105096c1e21903cb83b8c1b7) )
	ROM_LOAD( "70000126.bin", 0x0000, 0x080000, CRC(68deffbe) SHA1(9b94776aa0416309204987ac9109a65ad3234f1b) )
	ROM_LOAD( "70000132.bin", 0x0000, 0x080000, CRC(50c06d0d) SHA1(8d629d77390b92c5e30104237245f92dc8f52a6c) )
	ROM_LOAD( "70000133.bin", 0x0000, 0x080000, CRC(fb198e1b) SHA1(6fb03680ad29ca750fe2e75f48a05f538ddac9b7) )
	ROM_LOAD( "70000135.bin", 0x0000, 0x080000, CRC(02531c21) SHA1(de9da10bc81ab02ba131da1a1733eda1948dc3cc) )
	ROM_LOAD( "70001122.bin", 0x0000, 0x040000, CRC(13171ffc) SHA1(e49a2080afd27c0de183da64baa2060020910155) )
	ROM_LOAD( "70001124.bin", 0x0000, 0x040000, CRC(8acb2d7d) SHA1(ffd4f0e1f80b41b6f54af31e5dcd41fe12e4ea0b) )
	ROM_LOAD( "70001125.bin", 0x0000, 0x080000, CRC(6b202a88) SHA1(63f7325c8dc373f771f02e5bf9ac0c0d33a906bd) )
	ROM_LOAD( "70001126.bin", 0x0000, 0x080000, CRC(0db90e12) SHA1(0b010ca878ecabb47c0a0eec0badd595b2bafbfb) )
	ROM_LOAD( "70001135.bin", 0x0000, 0x080000, CRC(a9ed9178) SHA1(446919e869a9cc20f469954504adf448474d702b) )
	ROM_LOAD( "70001150.bin", 0x0000, 0x040000, CRC(3c3f4e45) SHA1(114c18e0fa8de224992138b72bf789ace39dffa0) )
	ROM_LOAD( "70001151.bin", 0x0000, 0x040000, CRC(0cb1f440) SHA1(7ebdac6ea495d96c7713a284fdad4da0874de3f2) )
	ROM_LOAD( "70001153.bin", 0x0000, 0x040000, CRC(e8ba9b3a) SHA1(71af6dd77da419868391e01f565c24a70d55b396) ) // rhn_gala_hopper_120.bin
	ROM_LOAD( "70001160.bin", 0x0000, 0x040000, CRC(2d532681) SHA1(fb4321b6922cf35780adbdc5f030ef0df8d6cc9a) )
	ROM_LOAD( "70001161.bin", 0x0000, 0x040000, CRC(e9a49319) SHA1(001163ece7a405a27fd71fdeb97489db143749a7) )
	ROM_LOAD( "70001502.bin", 0x0000, 0x040000, CRC(d1b332f1) SHA1(07db228705b0bce47107cf5458986e830b988cee) )
	ROM_LOAD( "70001503.bin", 0x0000, 0x040000, CRC(2a44069a) SHA1(0a1581ba552e0e93d6bc3b7298014ea4b6793da1) )
	ROM_LOAD( "70001510.bin", 0x0000, 0x080000, CRC(87cb4cae) SHA1(49c97e0e79a8cd1417e9e07a13afe736d00ef3df) )
	ROM_LOAD( "rhn_data_110_lv.bin", 0x0000, 0x040000, CRC(1f74c472) SHA1(86a170ddb001f817e960e7c166399280ad620bf0) )
	ROM_LOAD( "rhn_gala_hopper_120_lv.bin", 0x0000, 0x040000, CRC(521b6402) SHA1(7d260c45fa339f5ca34f8e335875ad47bb093a04) )
	ROM_LOAD( "rhn_mecca_120.bin", 0x0000, 0x040000, CRC(f131e386) SHA1(73672e6e66400b953dda7f2254082eff73dbf058) )
	ROM_LOAD( "rhn_mecca_120_lv.bin", 0x0000, 0x040000, CRC(471e5263) SHA1(79c205e0d8e748aa72f9f3fadad248edf71f5ae0) )
	ROM_LOAD( "rhn_std_110.bin", 0x0000, 0x040000, CRC(439f27d2) SHA1(4ad01c4dc9bbab7520fb281198777aea56f600b0) )
	ROM_LOAD( "rhn_std_110_lv.bin", 0x0000, 0x040000, CRC(922b8196) SHA1(6fdbf301aaadacaeabf29ad11c67b22122954051) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "71000120.hex", 0x0000, 0x112961, CRC(5eb5245e) SHA1(449b02baf56e5798f656d9aee497b88d34f562cc) )
	ROM_LOAD( "rhnsnd.bin", 0x0000, 0x080000, CRC(e03eaa43) SHA1(69117021adc1a8968d50703336147a7344c62100) )
ROM_END



ROM_START( m4rhrock )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "rhr_v200_1625_da8c_nlv.bin", 0x0000, 0x040000, CRC(dd67f5b3) SHA1(19b7b57ef20a2ad7997cf748396b246fda87db70) )
	ROM_LOAD( "rhr_v300_1216_ce52_nlv.bin", 0x0000, 0x040000, CRC(86b0d683) SHA1(c6553bf65c055c4f911c215ba112eaa672357290) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 ) // intelhex, needs convertig
	ROM_LOAD( "71000200.hi.hex", 0x0000, 0x0ff0f8, CRC(998e28ea) SHA1(f54a69af16e05119df2697bc01e548ac51ed3e11) )
	ROM_LOAD( "71000200.lo.hex", 0x0000, 0x134084, CRC(ccd0b35f) SHA1(6d3ef65577a46c68f8628675d146f829c9a99659) )
ROM_END


ROM_START( m4rhwhl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "70001184.bin", 0x0000, 0x080000, CRC(8792d95b) SHA1(24b4f78728db7ee95d1fcd3ba38b49a20baaae6b) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "rhw_v100_1333_6d40_lv.bin", 0x0000, 0x080000, CRC(9ef7b655) SHA1(605822eaee44bebf554218ef7346192a6a84077e) )
	ROM_LOAD( "rhw_v310_0925_0773_lv_p.bin", 0x0000, 0x080000, CRC(11880908) SHA1(0165bacf73dd54959975b3f186e256fd8d690d34) )
	ROM_LOAD( "rhw_v310_0931_fa02_lv.bin", 0x0000, 0x080000, CRC(5642892e) SHA1(7a80edf9aefac9731751afa8250de07004c55e77) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "71000180.hi.hex", 0x0000, 0x04da98, CRC(0ffa11a5) SHA1(a3f8eb00b6771cb49965a717e27d0b544c6b2f4f) ) // as intelhex?
	ROM_LOAD( "71000180.lo.hex", 0x0000, 0x134084, CRC(6dfc7474) SHA1(806b4b8ca5fa868581b4bf33080b9c486ce71bb6) )// as intelhex?
	ROM_LOAD( "redhotwheelssnd.p1", 0x0000, 0x080000, CRC(7b274a71) SHA1(38ba69084819133253b41f2eb1d784104e5f10f7) )
	ROM_LOAD( "redhotwheelssnd.p2", 0x0000, 0x080000, CRC(e36e19e2) SHA1(204554622c9020479b095acd4fbab1f21f829137) )
ROM_END

ROM_START( m4rdeal )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "70000703.bin", 0x0000, 0x080000, CRC(11e51311) SHA1(71a4327fa01cd7e899d423adc34c732ed56118d8) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "70000704.bin", 0x0000, 0x080000, CRC(b161c08b) SHA1(bb914eb900aff0f6eeec33ff8a595a288306e073) )
	ROM_LOAD( "70000723.bin", 0x0000, 0x080000, CRC(bb166401) SHA1(1adf244e97d52cc5a5116a01d804caadd1034507) )
	ROM_LOAD( "70000724.bin", 0x0000, 0x080000, CRC(37df89e5) SHA1(6e7da02053be91f27257e3c5f952bbea9df3bc09) )
	ROM_LOAD( "70001622.bin", 0x0000, 0x080000, CRC(1a8fff86) SHA1(e0bdb4fa233acdf18d535821fb9fc2cf69ac4f5e) )
	ROM_LOAD( "70001623.bin", 0x0000, 0x080000, CRC(97a223f8) SHA1(d1a87391a8178cb47664968031a7d17d9082847b) )
	ROM_LOAD( "70001703.bin", 0x0000, 0x080000, CRC(1b89777e) SHA1(09b87d352d27c2847dec0a147dea7cc75f07ffa5) )
	ROM_LOAD( "70001704.bin", 0x0000, 0x080000, CRC(11f3834d) SHA1(8c56cf60b064e8d755c5e760fcf6f0284ef710f9) )
	ROM_LOAD( "70001744.bin", 0x0000, 0x080000, CRC(6de0d366) SHA1(ec42608f3b9c7cf8e6b81541767a8664478e7ab4) )
	ROM_LOAD( "70001745.bin", 0x0000, 0x080000, CRC(4c0e9cab) SHA1(6003d4553f053a253965ba786553b00b7e197069) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4shoknr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "snr_v300_1218_3019_lv.bin", 0x0000, 0x040000, CRC(bec80497) SHA1(08de5e29a063b01fb904a156170a3063633115ab) )
	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "snr_v300_1221_c8ff_nlv.bin", 0x0000, 0x040000, CRC(d191b361) SHA1(4146e509e77878a51e32de877768504b3c85e6f8) )
	ROM_LOAD( "snr_v200_1145_047f_lv.bin", 0x0000, 0x040000, CRC(73ef1e1a) SHA1(6ccaf64daa5acacfba4df576281bb5478f2fbd29) )
	ROM_LOAD( "snr_v200_1655_5a69_nlv.bin", 0x0000, 0x040000, CRC(50ba0c6b) SHA1(767fd59858fc55ae95f096f00c54bd619369a56c) )
	ROM_LOAD( "shock.p1", 0x0000, 0x080000, CRC(65fb2f47) SHA1(33b997843a705fc446f33beb127b672a282286c4) ) // check if it's just an overdump of above sets

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "snrsnd.p1", 0x000000, 0x080000, CRC(985c7c8c) SHA1(d2740ff6192c21af3a8a8a9a92b6fd604b40e9d1) )
	ROM_LOAD( "snrsnd.p2", 0x080000, 0x080000, CRC(6a3a57ce) SHA1(3aaa0a761e17a2a14196cb023b10a49b44ba1046) )
	ROM_LOAD( "shock.s2", 0x080000, 0x080000, CRC(10e9912f) SHA1(833d2b125bf30bdb8de71f6c9d8a9fe92701f741) ) // alt snd2
ROM_END


ROM_START( m4shkwav )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "swave_v210_1135_08dd_lv.bin", 0x0000, 0x040000, CRC(ca9d40a3) SHA1(65c9e4aa022eb6fe70d619f67638c37ad578ddbf) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "swave_v210_11376_0bb3_nlv.bin", 0x0000, 0x040000, CRC(3fcaf973) SHA1(28258c8c60e6b542e1789cd8a4cfd530d1ed6084) )
	ROM_LOAD( "swsplv.bin", 0x0000, 0x040000, CRC(1e33e93f) SHA1(3e87f8ed35da776e1968c9574c140cc3984ea8de) )
	ROM_LOAD( "sho1_0lv.bin", 0x0000, 0x080000, CRC(a76d8544) SHA1(8277a2ce311840b8405a087d3dc0bbf97054ad87) )
	ROM_LOAD( "swave_v300_1552_13ed_nlv.bin", 0x0000, 0x040000, CRC(b0e03f04) SHA1(fdd113af30fd9e87b171ecdf3be7e720366476b3) )
	ROM_LOAD( "swave_v300_1555_119d_lv.bin", 0x0000, 0x040000, CRC(45b786d4) SHA1(24fd4fdea684103334385ca329f384796b496e2c) )
	ROM_LOAD( "swsp_v300_1602_e1b2_nlv.bin", 0x0000, 0x040000, CRC(4ed74015) SHA1(0ab2167ba0ce6f1a1317c2087091187b9fa94c27) )
	ROM_LOAD( "swsp_v300_1606_ded8_lv.bin", 0x0000, 0x040000, CRC(bb80f9c5) SHA1(95f577c427204b83bec0128acfd89dda90938d1f) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "71000250.hi.hex", 0x0000, 0x0c852b, CRC(e3a857c7) SHA1(66619b7926ae7df970045fffd7e20763abfe14a4) )
	ROM_LOAD( "71000250.lo.hex", 0x0000, 0x134084, CRC(46758bc5) SHA1(18d02960580646b276e7a6aabdeb4ca449ec5ea0) )
	ROM_LOAD( "shocksnd.p1", 0x000000, 0x080000, CRC(54bf0ddb) SHA1(693b855367972b5a45e9d2d6152849ab2cde38a7) )
	ROM_LOAD( "shocksnd.p2", 0x080000, 0x080000, CRC(facebc55) SHA1(75367473646cfc735f4d1267e13a9c92ea19c4e3) )
ROM_END

ROM_START( m4sinbdn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sbds3___.a_1", 0x0000, 0x020000, CRC(9bff0e40) SHA1(f8a1263a58f828554e9df77ed0db78e627666fb5) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4sinbd2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sxdsx___.2_0", 0x0000, 0x040000, CRC(4e1f98b5) SHA1(3e16e7a0cdccc9eb1a1bb6f9a0332c4582483eee) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4sinbd3 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sdd__.3_0", 0x0000, 0x040000, CRC(100098c1) SHA1(b125855c49325972f620463e32fdf124222e27d2) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4sinbdd )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sdds3__l.1w0", 0x0000, 0x040000, CRC(feedb8cf) SHA1(620b5379164d4da1200d4807199c2dc78d7d89ee) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4sinbdj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sbds3__l.9_0", 0x0000, 0x040000, CRC(e425375a) SHA1(d2bdd8e768fc7764054eff574360f3cfb5f4f66d) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4sinbdl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sbds3__l.aw0", 0x0000, 0x040000, CRC(c484ef9d) SHA1(62f6644b83dd6abaf80809217edf6a8230a89268) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4sinbdw )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sbds3___.7w1", 0x0000, 0x020000, CRC(23bc9ce0) SHA1(f750de2b781bc902c65de7109e10a5fc2d4e1c61) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4spotln )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gsp01.p1", 0x0000, 0x020000, CRC(54c56a07) SHA1(27f21872a7ffe0c497983fa5bbb59e967bf48974) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4sdquid )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "70000352.bin", 0x0000, 0x040000, CRC(303d6177) SHA1(aadff8a81244bfd62d1cc088caf01496e1ff61db) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "70000353.bin", 0x0000, 0x040000, CRC(6e3a9dfc) SHA1(1d5d04140811e17267102c0618ffdaf70f71f717) )
	ROM_LOAD( "70000354.bin", 0x0000, 0x080000, CRC(eb938886) SHA1(e6e882f28230b51091b2543df17c65e491a94f94) )
	ROM_LOAD( "70000355.bin", 0x0000, 0x080000, CRC(c4b857b3) SHA1(6b58ee5d780551a665e737ea291b04ab641e8029) )
	ROM_LOAD( "70001352.bin", 0x0000, 0x040000, CRC(0356e899) SHA1(51666a0ec4d29165f64391efa2339ea82710fa4c) )
	ROM_LOAD( "70001353.bin", 0x0000, 0x040000, CRC(324b4a86) SHA1(fd4cd59e3519e23ab824c84546ab50b3803766bf) )
	ROM_LOAD( "70001354.bin", 0x0000, 0x080000, CRC(e9aee455) SHA1(f9babcb90fe88fd522e4be7d8b794c9c1cd4f780) )
	ROM_LOAD( "70001355.bin", 0x0000, 0x080000, CRC(ebc428c3) SHA1(c75738fc547c5b1783a6c5a6ebb06eea5730683d) )
	ROM_LOAD( "70001401.bin", 0x0000, 0x080000, CRC(f83b1551) SHA1(d32f84938edfc4c3d9763fb3eecf56ed9102d979) )
	ROM_LOAD( "70001411.bin", 0x0000, 0x080000, CRC(fa0c95ec) SHA1(1a792e7ed8fa092fdb34a7b31df316d6afca3e90) )
	ROM_LOAD( "70001451.bin", 0x0000, 0x080000, CRC(2d288982) SHA1(bf3e1e1e20eb2d1d9ca6d0a8b48ef9c57aeb30bd) )
	ROM_LOAD( "70001461.bin", 0x0000, 0x080000, CRC(73238425) SHA1(a744adabd8854c6b45820899f15ebb2c2a74dd4d) )
	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
ROM_END



ROM_START( m4tornad )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "torn_v110_1146_979d_lv.bin", 0x0000, 0x040000, CRC(3160bddd) SHA1(4f36b081c8f6859a3fe55e1f177a0406c2480987) )

	ROM_REGION( 0x80000, "altrevs", 0 )
	ROM_LOAD( "torn_v110_1153_955f_nlv.bin", 0x0000, 0x040000, CRC(c437040d) SHA1(50c5ba655989b7f6a2ee61af0ad007ce825f4364) )
	ROM_LOAD( "tornsp_v110_1148_95bd_nlv.bin", 0x0000, 0x040000, CRC(f0933eb6) SHA1(a726b02ae6298ecfb6a01f7ecb09bac50ca13114) )
	ROM_LOAD( "tornsp_v110_1151_9693_lv.bin", 0x0000, 0x040000, CRC(05c48766) SHA1(e20916cd67904601cd25b3c6f70030c5302d12a6) )
	ROM_LOAD( "torn_v200_1613_efe5_nlv.bin", 0x0000, 0x040000, CRC(32936ae5) SHA1(f7ec8b9c0e74c0e51ea4b9c8f450f64907ce3300) )
	ROM_LOAD( "torn_v200_1617_ece9_lv.bin", 0x0000, 0x040000, CRC(c7c4d335) SHA1(955303d446e78bfa1ceeb07ca62cb6e11e478592) )
	ROM_LOAD( "tornsp_v200_1623_eee3_nlv.bin", 0x0000, 0x040000, CRC(6b4f8baf) SHA1(fea21f43b3bbc1c969a7426ca956898e3680823f) )
	ROM_LOAD( "tornsp_v200_1626_ec93_lv.bin", 0x0000, 0x040000, CRC(9e18327f) SHA1(7682cd172903cd5c26873306e70394c154e66c30) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "71000300.hi.hex", 0x0000, 0x0be342, CRC(f9021a32) SHA1(4bd7d7306385ef37dd9cbb5085dbc104657abc0e) )
	ROM_LOAD( "71000300.lo.hex", 0x0000, 0x134084, CRC(af34658d) SHA1(63a6db1f5ed00fa6208c63e0a2211ba2afe0e9a1) )
	ROM_LOAD( "tornadosnd.p1", 0x0000, 0x080000, CRC(cac88f25) SHA1(6ccbf372d983a47a49caedb8a526fc7703b31ed4) )
	ROM_LOAD( "tornadosnd.p2", 0x080000, 0x080000, CRC(ef4f563d) SHA1(1268061edd93474296e3454e0a2e706b90c0621c) )
ROM_END

ROM_START( m4vivan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "vlv.bin", 0x0000, 0x010000, CRC(f20c4858) SHA1(94bf19cfa79a1f5347ab61a80cbbce06942187a2) )

	ROM_REGION( 0x200000, "msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "vlvsound1.bin", 0x0000, 0x080000, CRC(ce4da47a) SHA1(7407f8053ee482db4d8d0732fdd7229aa531b405) )
	ROM_LOAD( "vlvsound2.bin", 0x0000, 0x080000, CRC(571c00d1) SHA1(5e7be40d3caae88dc3a580415f8ab796f6efd67f) )
ROM_END





ROM_START( m4abra )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD( "nn_sj___.4_0", 0x0000, 0x040000, CRC(48437d29) SHA1(72a2e9337fc0a004c382931f3af856253c44ed61) )

	ROM_REGION( 0x040000, "altrevs", 0 )
	ROM_LOAD( "nn_sja__.4_0", 0x0000, 0x040000, CRC(766cd4ae) SHA1(4d630b967ede615d325f524c2e4c92c7e7a60886) )
	ROM_LOAD( "nn_sjb__.4_0", 0x0000, 0x040000, CRC(ca77a68a) SHA1(e753c065d299038bae4c451e647b9bcda36421d9) )
	ROM_LOAD( "nn_sjk__.4_0", 0x0000, 0x040000, CRC(19018556) SHA1(6df993939e70a24621d4e732d0670d64fac1cf56) )
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4wcnov )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "wcdsxh__.5_0", 0x0000, 0x080000, CRC(a82d11de) SHA1(ece14fd5f56da8cc788c53d5c1404275e9000b65) )
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4excaln )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "exdsx___.6_0", 0x0000, 0x080000, CRC(fcdc703c) SHA1(927870723106aebbb2b492ce9bfebe4aa25d0325) )

	ROM_REGION( 0x080000, "altrevs", 0 )
	ROM_LOAD( "exdsx_e_.6_0", 0x0000, 0x080000, CRC(f6421feb) SHA1(5b3cf7fa4bf9711097ed1c9d2d5689329d73193d) )
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4vfm )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD( "v_f_mon", 0x0000, 0x020000, CRC(e4add02c) SHA1(5ef1bdd532ef0801b96ceae941f3da789039811c) )
	ROM_REGION( 0x080000, "msm6376", ROMREGION_ERASE00 )
ROM_END


ROM_START( m4sunseta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b2512s.p1", 0x0000, 0x010000, CRC(8c509538) SHA1(eab6a1e44e77cb48cf490616facc74932acc93c5) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "sunsetb.chr", 0x0000, 0x000048, CRC(f166963b) SHA1(5cc6ada61036d8dbeca470e9548f9f5d2bd545a8) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4sunsetb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b2512y.p1", 0x0000, 0x010000, CRC(65fa2cd9) SHA1(d2ab1ae25d5425a0788f86535a20d3ebe4a9db2b) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "sunsetb.chr", 0x0000, 0x000048, CRC(f166963b) SHA1(5cc6ada61036d8dbeca470e9548f9f5d2bd545a8) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
ROM_END

ROM_START( m4sunsetc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sunboul-5p3.bin", 0x0000, 0x010000, CRC(5ccbf062) SHA1(cf587018511d1a06624d271f2fde4e40f16ec87c) )
	ROM_REGION( 0x48, "fakechr", 0 )
	ROM_LOAD( "sunsetb.chr", 0x0000, 0x000048, CRC(f166963b) SHA1(5cc6ada61036d8dbeca470e9548f9f5d2bd545a8) )
	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
ROM_END


/* Barcrest */
GAME( 198?, m4tst,        0, mod2    ,   mpu4, mpu4_state,       m4tst,   ROT0, "Barcrest","MPU4 Unit Test (Program 4)",GAME_MECHANICAL )
GAME( 198?, m4tst2,       0, mod2    ,   mpu4, mpu4_state,       m4tst2,  ROT0, "Barcrest","MPU4 Unit Test (Program 2)",GAME_MECHANICAL )
GAME( 198?, m4clr,        0, mod2    ,   mpu4, driver_device,       0,       ROT0, "Barcrest","MPU4 Meter Clear ROM",GAME_MECHANICAL )

#define GAME_FLAGS (GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK)

// GEEN TUBES
GAME(199?, m4topdk	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Top Deck (Barcrest) (Dutch) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4addrd	,m4addr		,mod2   	,mpu4				, mpu4_state,m4default_alt		,ROT0,   "Barcrest","Adders & Ladders (Barcrest) (DAL, Dutch) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4voodoo	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Voodoo 1000 (Barcrest) (Dutch) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )


// needs better reel (correct setup) handling to boot
GAME(199?, m4blkbul	,0			,mod2   	,mpu4				, mpu4_state,m4default_alt		,ROT0,   "Barcrest","Super Play (Black Bull?) (Barcrest) [XSP] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // XSP??
GAME(199?, m4calicl	,0			,mod2   	,mpu4				, mpu4_state,m4default_alt		,ROT0,   "Barcrest","California Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bucks	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Bucks Fizz Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4clbtro	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Club Tropicana (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4gldgat	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Golden Gate (Barcrest) [DGG, Dutch] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hirise	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","High Rise (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4nspot	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Night Spot Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4supbf	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Super Bucks Fizz Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4toma	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Tomahawk (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4tropcl	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Tropicana Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

// other issues
GAME(199?, m4casmul	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Casino Multiplay (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // crashes mame
GAME(199?, m4crzjk	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Crazy Jokers (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4fastfw	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Fast Forward (Barcrest - Bwb) [FFD 1.0] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

// Bad CHR Alarm
GAME(199?, m4btclok	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Beat The Clock (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4brktak	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Break & Take (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4nick	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Nickelodeon (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

/* I don't actually think all of these are Barcrest, some are mislabeled */
GAME(199?, m4actbnk	,0			,mod4oki	,mpu4jackpot8tkn	, mpu4_state,m4default			,ROT0,   "Barcrest","Action Bank (Barcrest) [ACT] (set 1) (MPU4)",						GAME_FLAGS ) // set jackpot key to 8GBP TOKEN
GAME(199?, m4addrc	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Adders & Ladders Classic (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4addrcc	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Adders & Ladders Classic Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4amhiwy	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","American Highway (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // crash mame
GAME(199?, m4andybt	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Andy's Big Time Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bnkrol	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Bank Roller Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4blkwhd	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Black & White (Barcrest) [Dutch] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

GAME(199?, m4gnsmk	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Gun Smoke (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4blkbuld,m4blkbul	,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Gun Smoke (Black Bull sound?) (Barcrest) [Dutch] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

GAME(199?, m4blkcat	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Black Cat (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bluedm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Blue Diamond (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4brook	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Brooklyn (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4calamab,m4calama	,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Calamari Club (Barcrest - Bwb) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4calama	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Calamari Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ceptr	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Ceptor (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4chasei	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Chase Invaders (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4c9c	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Cloud Nine Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4clbshf	,0			,mod4yam	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Club Shuffle (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4clbveg	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Club Vegas (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4clbx	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Club X (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4coscas	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Cosmic Casino (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4crzjwl	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Crown Jewels (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4crjwl	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Crown Jewels Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4crjwl2	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Crown Jewels Mk II Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4crdome	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Crystal Dome (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4dbl9	,0			,mod2		,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Double 9's (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4dbldmn	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Double Diamond Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4drac	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Dracula (Barcrest - Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4exgam	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Extra Game (Fairplay - Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4fortcb	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Fortune Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4frtlt	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Fruit & Loot (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4frtgm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Fruit Game (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4gb006	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Games Bond 006 (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4gbust	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Ghost Buster (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4gldjok	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Golden Joker (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // dutch?
GAME(199?, m4hpyjok	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Happy Joker (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4holdtm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Hold Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hypclb	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Hyper Viper Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jok300	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Jokers 300 (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jokmil	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Jokers Millennium (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4joljok	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Jolly Joker (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // mame crash
GAME(199?, m4joltav	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Jolly Taverner (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // mame crash
GAME(199?, m4joljokd,m4joljok	,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Jolly Joker (Barcrest) [Dutch] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4joljokh,m4joljok	,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Jolly Joker (Barcrest) [Hungarian] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4lastrp	,0			,mod4oki	,mpu4jackpot8tkn	, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Las Vegas Strip (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4luck7	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Lucky 7 (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4luckdv	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Lucky Devil (Barcrest) [Czech] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4luckdvd,m4luckdv	,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Lucky Devil (Barcrest) [Dutch] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4lucksc	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Lucky Strike Club (Barcrest) [MPU 4] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4luckwb	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Lucky Wild Boar (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4przlux	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Prize Luxor (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4madhse	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Mad House (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4magdrg	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Magic Dragon (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4maglin	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Magic Liner (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4magrep	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Magic Replay DeLuxe (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4nhtt	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","New Hit the Top (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4nile	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Nile Jewels (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

GAME(199?, m4nudshf	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Nudge Shuffle (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4oldtmr	,0			,mod4oki	,mpu4				, mpu4_state,m_oldtmr			,ROT0,   "Barcrest","Old Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4casot	,m4oldtmr	,mod4oki	,mpu4				, mpu4_state,m_oldtmr			,ROT0,   "Barcrest","Casino Old Timer (Black & White?) (Old Timer Sound hack?) (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // uses the same program???
GAME(199?, m4jpmcla	,m4oldtmr	,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","JPM Classic (Old Timer Sound hack?) (Barcrest) (MPU4)",							GAME_FLAGS|GAME_NO_SOUND ) // uses the same program???
GAME(199?, m4ordmnd	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Oriental Diamonds (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ptblkc	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Pot Black Casino (Bwb - Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // main cpu crashes?
GAME(199?, m4prem	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Premier (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4przfrt	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Prize Fruit & Loot (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4przmns	,0			,mod4oki   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Prize Money ShowCase (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4przrf	,0			,mod4oki   	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Prize Rich And Famous (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4przrfm	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Prize Run For Your Money (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4przwo	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Prize What's On (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4reeltm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Reel Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rdht	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Red Heat (Golden Nugget?) (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rhrcl	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Red Hot Roll Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rwb	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Red White & Blue (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rmtp	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Reel Magic Turbo Play / Star Play 300? (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rmtpd	,0			,mod4oki   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Reel Magic Turbo Play Deluxe (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ringfr	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Ring Of Fire (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rhog2	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Road Hog 2 - I'm Back (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rhogc	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Road Hog Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4roadrn	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Road Runner (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rockmn	,0			,mod4oki	,mpu4jackpot8tkn	, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Rocket Money (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4royjwl	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Royal Jewels (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4salsa	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Salsa (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4showtm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Show Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4shodf	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Showcase Duty Free (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(1999, m4squid	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Squids In (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4steptm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Step Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4supbj	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Super Blackjack (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4supbjc	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Super Blackjack Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4suphv	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Super Hyper Viper (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4take5	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Take 5 (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4tpcl	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Take Your Pick Club (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4techno	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Techno Reel (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4toprn	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Top Run (Barcrest) (Dutch) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4toptim	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Top Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4tbplay	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Turbo Play (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4tutcl	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Tutti Fruity Classic (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4twintm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Twin Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4twist	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Twist Again (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4univ	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Universe (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4vegastg,m4vegast	,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Barcrest","Vegas Strip (Barcrest) [German] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4vivalvd,m4vivalv	,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Viva Las Vegas (Barcrest) [Dutch] (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4wildtm	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Wild Timer (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ch30	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","unknown MPU4 'CH3 0.1' (Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

// might need samples, but run silent with none
GAME(199?, m4lineup	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Line Up (Bwb - Barcrest) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND ) // no sound with any system?


/* Bwb */

/* are all these really MPU4 hw? , check things like Daytona, doesn't boot at all. */
GAME(199?, m4bigmt	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","The Big Match (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bingbl	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Bingo Belle (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bingbs	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Bingo Belle Showcase (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bingcl	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Bingo Club (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4blsbys	,0			,bwboki		,mpu4				, mpu4_state,m_blsbys			,ROT0,   "Bwb","Blues Boys (Bwb) (MPU4)",			GAME_FLAGS )
GAME(199?, m4bluesn	,0			,bwboki		,mpu4				, mpu4_state,m_blsbys			,ROT0,	 "Nova","Blues Boys (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE) // German version, still has BWB strings, crashes during boot, but boots by chance the first time?
GAME(199?, m4cshenc	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Cash Encounters (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4czne	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Cash Zone (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4csoc	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Championship Soccer (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4cpycat	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Copy Cat (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4cpfinl	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Cup Final (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4danced	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Dancing Diamonds (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4daytn	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Daytona (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4excal	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Excalibur (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4exotic	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Exotic Fruits (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4firice	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Fire & Ice (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4fourmr	,0			,mod2		,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Four More (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND ) // no sound with either system?
GAME(199?, m4harle	,0			,mod4oki	,mpu4				, mpu4_state,m4default	        ,ROT0,   "Bwb","Harlequin (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hvhel	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Heaven & Hell (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4holywd	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Hollywood (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4indycr	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Indy Cars (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jakjok	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Jackpot Jokers (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jakjoka,m4jakjok	,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Jackpot Jokers (Bwb) (MPU4, alternate)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jflash	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Jumping Jack Flash (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4kingq	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Kings & Queens (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4kingqc	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Kings & Queens Classic (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4lazy	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Lazy Bones (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4lvlcl	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Lucky Las Vegas Classic (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ln7	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Lucky No7 (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4madmon	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Mad Money (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4madmnc	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Mad Money Classic (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4mmm	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Money Mummy Money (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4oadrac	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Ooh Aah Dracula (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4orland	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Orlando Magic (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4pzbing	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Prize Bingo (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4quidin	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Quids In (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4quidis	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Quids In Showcase (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rackem	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Rack Em Up (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rbgold	,0			,mod2   	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Rainbow Gold (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rhfev	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Red Hot Fever (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rhs	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Rocky Horror Show (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sinbd	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Sinbad (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sky	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Sky Sports (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4souls	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Soul Sister (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4specu	,0			,mod2	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Speculator Club (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND ) // no sound with either system
GAME(199?, m4spinbt	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Spin The Bottle (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4starst	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Stars & Stripes (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4thestr	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","The Streak (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sunclb	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Sun Club (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sunscl	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Sunset Club (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4supleg	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Super League (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4supscr	,0			,mod4oki    ,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Super Soccer (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ssclas	,0			,mod4oki    ,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Super Streak Classic (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sure	,0			,mod2   	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Sure Thing (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4tic	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Tic Tac Toe (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ticcla	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Tic Tac Toe Classic (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ticgld	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Tic Tac Toe Gold (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ticglc	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Tic Tac Toe Gold Classic (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4topdog	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Top Dog (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4trex	,0			,mod4oki	,mpu4				, mpu4_state,m4default			,ROT0,   "Bwb","Trex (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4volcan	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Volcano (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4vdexpr	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","Voodoo Express (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4xch	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","X-change (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4xs		,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","X-s (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4xtrm	,0			,mod4oki	,mpu4				, mpu4_state,m4default_bigbank	,ROT0,   "Bwb","X-treme (Bwb) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )





/* Nova - German licensed Barcrest / Bwb sets? */

GAME(199?, m4bigban	,0			,mod4oki    	,mpu4				, mpu4_state,m4default			,ROT0,   "Nova","Big Bandit (Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4crzcsn	,0			,mod4oki    	,mpu4				, mpu4_state,m4default			,ROT0,   "Nova","Crazy Casino (Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4crzcav	,0			,mod4oki    	,mpu4				, mpu4_state,m4default			,ROT0,   "Nova","Crazy Cavern (Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4dragon	,0			,mod4oki    	,mpu4				, mpu4_state,m4default			,ROT0,   "Nova","Dragon (Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hilonv	,0			,mod4oki    	,mpu4				, mpu4_state,m4default			,ROT0,   "Nova","Hi Lo Casino (Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4octo	,0			,mod4oki    	,mpu4				, mpu4_state,m4default			,ROT0,   "Nova","Octopus (Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sctagt	,0			,mod4oki    	,mpu4				, mpu4_state,m4default			,ROT0,   "Nova","Secret Agent (Nova) (MPU4)",						GAME_FLAGS|GAME_NO_SOUND )

/* Others */

GAME(199?, m4aao,     0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Eurotek","Against All Odds (Eurotek) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bandgd,  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Eurogames","Bands Of Gold (Eurogames) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bangin,  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 1)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bangina, m4bangin,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 2)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4banginb, m4bangin,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Bangin' Away (Global) (MPU4, set 3)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4wwc,	  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Wacky Weekend Club (Global) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4screw,	  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.8)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4screwp,  m4screw,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.8) (Protocol)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4screwa,  m4screw,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.7)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4screwb,  m4screw,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Screwin' Around (Global) (MPU4, v0.5)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4vfm,	  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Global","Value For Money (Global) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bigben,  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 1)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bigbena, m4bigben,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 2)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bigbenb, m4bigben,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 3)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bigbend, m4bigben,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bigbene, m4bigben,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Big Ben (Coinworld) (MPU4, set 5)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4kqclub,  0,		mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Newby","Kings & Queens Club (Newby) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4snookr,  0,		mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Eurocoin","Snooker (Eurocoin) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND ) // works?
GAME(199?, m4stakex,  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 1)",   GAME_FLAGS|GAME_NO_SOUND ) // can't coin, no sound
GAME(199?, m4stakexa, m4stakex,	mod4oki, mpu4, mpu4_state, m4default, ROT0,   "Leisurama","Stake X (Leisurama) (MPU4, set 2)",   GAME_FLAGS|GAME_NO_SOUND ) // works?
GAME(199?, m4sstrek,  0,		mod4yam    ,mpu4, mpu4_state, m4default, ROT0,   "bootleg","Super Streak (bootleg) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND ) // works?, no sound
GAME(199?, m4boltbl,  0,		mod2    ,mpu4, mpu4_state, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 1)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4boltbla, m4boltbl,	mod2    ,mpu4, mpu4_state, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 2)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4boltblb, m4boltbl,	mod2    ,mpu4, mpu4_state, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 3)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4boltblc, m4boltbl,	mod2    ,mpu4, mpu4_state, m4default, ROT0,   "DJE","Bolt From The Blue (DJE) (MPU4, set 4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4dblchn,  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "DJE","Double Chance (DJE) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4stand2,  0,		mod2    ,mpu4, mpu4_state, m4default, ROT0,   "DJE","Stand To Deliver (DJE) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )

/* Unknown stuff that looks like it might be MPU4, but needs further verification, some could be bad */

GAME(199?, m4barcrz	,  0,		mod4oki	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","Bar Crazy (unknown) (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bonzbn	,  0,		mod4oki	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","Bingo Bonanza (unknown) (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4cld02	,  0,		mod4oki	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","unknown MPU4 'CLD 0.2C' (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4dnj	,  0,		mod4oki	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","Double Nudge (unknown) (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4matdr	,  0,		mod4oki	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","Matador (unknown) (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hslo	,  0,		mod2	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","unknown MPU4 'HOT 3.0' (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4unkjok	,  0,		mod2	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","unknown MPU4 'Joker' (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4remag	,  0,		mod2	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","unknown MPU4 'ZTP 0.7' (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4rmg	,  0,		mod2	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","unknown MPU4 'CTP 0.4' (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4wnud	,  0,		mod2	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","unknown MPU4 'W Nudge' (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4t266	,  0,		mod2	,mpu4, mpu4_state, m4default, ROT0,   "<unknown>","unknown MPU4 'TTO 1.1' (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4brnze	,  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "<unknown>","Bronze Voyage (unknown) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4riotrp	,  0,		mod4oki, mpu4, mpu4_state, m4default, ROT0,   "<unknown>","Rio Tropico (unknown) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )

/* *if* these are MPU4 they have a different sound system at least - The copyright strings in them are 'AET' tho (Ace?) - Could be related to the Crystal stuff? */
GAME(199?, m4sbx	,  0,		mpu4crys	,mpu4, mpu4_state, m_frkstn, ROT0,   "AET/Coinworld","Super Bear X (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4bclimb	,  0,		mpu4crys	,mpu4, mpu4_state, m_frkstn, ROT0,   "AET/Coinworld","Bear Climber (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4captb	,  0,		mpu4crys	,mpu4, mpu4_state, m_frkstn, ROT0,   "AET/Coinworld","Captain Bear (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jungj	,  0,		mpu4crys	,mpu4, mpu4_state, m_frkstn, ROT0,   "AET/Coinworld","Jungle Japes (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4fsx	,  0,		mpu4crys	,mpu4, mpu4_state, m_frkstn, ROT0,   "AET/Coinworld","Fun Spot X (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ccop	,  0,		mod4oki		,mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Cash Cops (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ccc	,  0,		mod4oki		,mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Criss Cross Crazy (Coinworld) (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4treel	,  0,		mod2		,mpu4, mpu4_state, m4default, ROT0,   "Jpm","Turbo Reels (Jpm) (MPU4?)",   GAME_FLAGS|GAME_NO_SOUND )




GAME(199?, m4surf, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Gemini","Super Surfin' (Gemini) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4wife, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Gemini","Money Or Yer Wife (Gemini) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4blkgd, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Gemini","Black Gold (Gemini) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4excam, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Excalibur (Mdm) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4front, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Final Frontier (Mdm) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4safar, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Safari Club (Mdm) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4zill, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Pure Leisure","Zillionare's Challenge (Pure Leisure) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )

GAME(199?, m4snowbl, 0,			mod2    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Snowball Bingo (Mdm) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hstr, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Happy Streak (Coinworld) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hstrcs, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Casino Happy Streak (Coinworld) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4ddb,  0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Ding Dong Bells (Coinworld) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4hapfrt,  0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Coinworld","Happy Fruits (Coinworld) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )


GAME(199?, m4ewshft, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Mdm","Each Way Shifter (Mdm) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jiggin, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Global","Jiggin' In The Riggin' (Global) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sunday, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Pcp","Sunday Sport (Pcp) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4jp777, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Cotswold Microsystems","Jackpot 777 (Cotswold Microsystems) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4booze, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Extreme","Booze Cruise (Extreme) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4cbing, 0,			mod4oki    ,mpu4, mpu4_state, m4default, ROT0,   "Redpoint Systems","Cherry Bingo (Redpoint Systems) (MPU4)",   GAME_FLAGS|GAME_NO_SOUND ) // custom sound system


GAME( 199?, m4nod		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Eurotech",   "Nod And A Wink (Eurotech) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE) // this has valid strings in it BEFORE the bfm decode, but decodes to valid code, does it use some funky mapping, or did they just fill unused space with valid looking data?

GAME( 199?, m4dcrls		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Mazooma",   "Double Crazy Reels (Mazooma) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)

// not sure about several of the nova ones
GAME( 199?, m4aliz		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "AlizBaz (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4c2		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Circus Circus 2 (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4coney		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Coney Island (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4cfinln	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Cup Final (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4dcrazy	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "D' Crazy Reels (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4goldnn	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Golden Years (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4jungjk	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Jungle Jackpots (Mazooma - Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4clab		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Cash Lab (Mazooma - Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4looplt	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Loop The Loot (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4mgpn		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Monaco Grand Prix (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4olygn		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Olympic Gold (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4rhnote	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Red Hot Notes (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4rhrock	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Red Hot Rocks (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4rhwhl		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Red Hot Wheels (Mazooma - Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4rdeal		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Reel Deal (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4shoknr	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Shock 'n' Roll (Mazooma - Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4shkwav	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Shockwave (Mazooma - Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sinbdn	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Sinbad (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sinbd2	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Sinbad Deluxe 2 (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sinbd3	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Sinbad Deluxe 3 (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sinbdd	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Sinbad Deluxe [Wall Mount] (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sinbdj	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Sinbad [Jackpot Link] (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sinbdl	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Sinbad [Jackpot Link] [Wall Mount] (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sinbdw	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Sinbad [Wall Mount] (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4spotln	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Spotlight (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4sdquid	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Sundance Quid (Qps) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4tornad	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Qps",   "Tornado (Qps - Mazooma) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4vivan		, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Viva Las Vegas (Nova) (MPU4)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME( 199?, m4ftladn	, 0			,  mod4oki		, mpu4		, mpu4_state, m4default		, 0,		 "Nova",  "Find the Lady (Nova)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)

// these don't contain a valid vector in the first bank?
GAME(199?, m4abra	,0			,bwboki		,mpu4				, mpu4_state,m_blsbys			,ROT0,	 "Bwb","Abracadabra (Bwb) (MPU4?)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME(199?, m4wcnov	,0			,bwboki		,mpu4				, mpu4_state,m_blsbys			,ROT0,	 "Nova","World Cup (Nova) (MPU4?)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)
GAME(199?, m4excaln	,0			,bwboki		,mpu4				, mpu4_state,m_blsbys			,ROT0,	 "Nova","Excalibur (Nova) (MPU4?)", GAME_FLAGS|GAME_MECHANICAL|GAME_SUPPORTS_SAVE)



GAME(199?, m4sunseta	,m4sunset	,mod4oki   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (B25 1.2, set 1)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sunsetb	,m4sunset	,mod4oki   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (B25 1.2, set 2)",						GAME_FLAGS|GAME_NO_SOUND )
GAME(199?, m4sunsetc	,m4sunset	,mod4oki   	,mpu4				, mpu4_state,m4default			,ROT0,   "Barcrest","Sunset Boulevard (Barcrest) (MPU4) (OSB 0.2)",						GAME_FLAGS|GAME_NO_SOUND ) // might be a mod 2
