/* Astra Fruit Machines
 -- Unknown HW platform
 -- 68k based
 -- dumps are of an unknown quality
 -- These might all be the same system with different rom configurations, or different systems (unknown)

Platform also used by Lowen? (at least some of their sets use the same address line scheme)

 some Astra games require linked machines with a 'master' and shared percentage system.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"


class astrafr_state : public driver_device
{
public:
	astrafr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( astrafr_master_map, AS_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM // as_partyd set
ADDRESS_MAP_END

// probably identical, afaik they're linekd units..
static ADDRESS_MAP_START( astrafr_slave_map, AS_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM // as_partyd set
ADDRESS_MAP_END


static ADDRESS_MAP_START( astra_map, AS_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM // as_partyd set
ADDRESS_MAP_END


static INPUT_PORTS_START( astrafr )
INPUT_PORTS_END


static MACHINE_CONFIG_START( astrafr_dual, astrafr_state )

	MCFG_CPU_ADD("maincpu", M68020, 16000000) // probably 68340 like other systems of this era? definitely not plain 68k
	MCFG_CPU_PROGRAM_MAP(astrafr_master_map)

	MCFG_CPU_ADD("slavecpu", M68020, 16000000) // probably 68340 like other systems of this era? definitely not plain 68k
	MCFG_CPU_PROGRAM_MAP(astrafr_slave_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( astra_single, astrafr_state )

	MCFG_CPU_ADD("maincpu", M68020, 16000000) 
	MCFG_CPU_PROGRAM_MAP(astra_map)
MACHINE_CONFIG_END




/* are the ptM roms Master nad ptS roms Slave?
  or is as_partyd set actually the master, with the larger roms?
*/

ROM_START( as_party )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "ptm105.u1", 0x0000, 0x080000, CRC(383a0132) SHA1(7a8f06afa0b747c328b4137ae6dcbebac2165bb7) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "pts105.u1", 0x0000, 0x080000, CRC(0699143f) SHA1(486dff92c27ede8cd0d9395f6e4418ba0d056e90) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "pts105d.u1", 0x0000, 0x080000, CRC(0f628b8c) SHA1(c970db9274df6b9c0383ae6e0bcf9b24288a40cd) )
ROM_END

ROM_START( as_partya )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "ptm110.u1", 0x0000, 0x080000, CRC(dd649eaf) SHA1(7449818635b6542148335e4ede72c6682d07834b) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "pts110.u1", 0x0000, 0x080000, CRC(b2771618) SHA1(40b0b52969a2ee53d38907c210305ae6f9ed7436) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "pts110d.u1", 0x0000, 0x080000, CRC(bb8c89ab) SHA1(2702574177d56768a4cc03f0b2bb95b3510dfe48) )
	ROM_LOAD( "pts110g.u1", 0x0000, 0x080000, CRC(c9cc1d09) SHA1(5ae08d5413d8b8f6ba06c412163168b3e86a67e6) )
ROM_END

ROM_START( as_partyb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "ptm112.u1", 0x0000, 0x080000, CRC(ab4a2808) SHA1(4b7c4986d4d3cf65be925085d41b698bbf0b4374) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "pts112.u1", 0x0000, 0x080000, CRC(b89bca1d) SHA1(899bafefa02321f2225276de2eca2a303b64a2b2) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "pts112d.u1", 0x0000, 0x080000, CRC(b16055ae) SHA1(beb411fb06f72557fcdc8c8d42dae0751448855b) )
ROM_END

ROM_START( as_partyc )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "ptnm206.u1", 0x0000, 0x080000, CRC(46d841b9) SHA1(6c4ffffd685bfa3474f697a1d9c96a36882ce104) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "ptns206.u1", 0x0000, 0x080000, CRC(933ce1a6) SHA1(c75a43e9bcb95f6d1579f7f5d5ed67b80499b5e7) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "ptns206d.u1", 0x0000, 0x080000, CRC(9ac77e15) SHA1(a0ee7db100a0303a15f5fde2b44a5982f8252e85) )
ROM_END

ROM_START( as_partyd )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "ptmv4-01.bin", 0x0000, 0x100000, CRC(3fde519a) SHA1(4d6f19e95e488e05174b2b025310852780e8d916) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "ptsv4-01.bin", 0x0000, 0x100000, CRC(da51bdca) SHA1(675fd673085155ffd4211f2fb4c8e8252fe76072) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "pts4-01d.bin", 0x0000, 0x100000, CRC(f6bc31a6) SHA1(c5c05e40da7c097c3b56a2b48cbf1992128cab6b) )
	ROM_LOAD( "pts4-01g.bin", 0x0000, 0x100000, CRC(0440d541) SHA1(afcef2d6671d23d6635a54faa2f72217f03ab2de) )
ROM_END

ROM_START( as_letsp )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "lpmv9-04.bin", 0x0000, 0x080000, CRC(c93e4e63) SHA1(3a1d4f589d8cdbcb25e35769104f21ba0883498d) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "lpsv9-04.bin", 0x0000, 0x080000, CRC(e2c5558b) SHA1(a49f6a23523aac3c5c8198fffc2d319e4bc7cfdd) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "lpsv904d.bin", 0x0000, 0x080000, CRC(eb3eca38) SHA1(9655fe22112110f015a6bbe5c43dbda96d800912) )
ROM_END







ROM_START( as_topsl )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tsm103.u1", 0x0000, 0x080000, CRC(da37998f) SHA1(202dad521dc15187c40e807b117aa70737e126f4) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tss103.u1", 0x0000, 0x080000, CRC(f4be316a) SHA1(f96e76b81db17ce1b4b42a45b5b209d140048657) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "tss103d.u1", 0x0000, 0x080000, CRC(fd45aed9) SHA1(029b66154ef171449b8712a1ec6a367052ea8aa8) )
ROM_END

ROM_START( as_topsla )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tsm104.u1", 0x0000, 0x080000, CRC(f5af7008) SHA1(73d436c146b46886004b0cf3b1e64a5a58e0e769) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tss104.u1", 0x0000, 0x080000, CRC(a3ab1bce) SHA1(9c9b9ca6e62219b7e8e2631e56800a212613df0e) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "tss104d.u1", 0x0000, 0x080000, CRC(aa50847d) SHA1(78375a73c1f318aaab8c3ea9024d9fdaeb42c2fc) )
ROM_END

ROM_START( as_topslb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tsm201.u1", 0x0000, 0x080000, CRC(acd57fcf) SHA1(69819d6002a2ce0481977369d469fe597fd6618b) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tss201.u1", 0x0000, 0x080000, CRC(3ead0a9e) SHA1(cf2630c0d555ee7aa6dc9a33265feb26208321b8) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "tss201d.u1", 0x0000, 0x080000, CRC(3756952d) SHA1(23a31b95e96df9b1090bc49b13df49cd4e425025) )
	ROM_LOAD( "tss201g.u1", 0x0000, 0x080000, CRC(4516018f) SHA1(008be270e2a9276989e069b757705035668dc8c6) )
	ROM_LOAD( "tss201gt.u1", 0x0000, 0x080000, CRC(5662e78c) SHA1(af9aede67706b3c596a46e5947b768c719883ab2) )
	ROM_LOAD( "tss201st.u1", 0x0000, 0x080000, CRC(2dd9ec9d) SHA1(1ccdee0316f1d73b8cb5c94396c537023b77524e) )
	ROM_LOAD( "tss201t.u1", 0x0000, 0x080000, CRC(2422732e) SHA1(c81a8558a986bfaec3277dcdab64a0b0a9318fb2) )
ROM_END

ROM_START( as_topslc )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tsm203.u1", 0x0000, 0x080000, CRC(78e61d11) SHA1(888c76cd44fe1f556eddf0227df03bd0d6210bbf) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tss203.u1", 0x0000, 0x080000, CRC(5c55f57a) SHA1(ca6172788deb06d3faeb7186f7922bd4f4094430) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD( "tss203d.u1", 0x0000, 0x080000, CRC(55ae6ac9) SHA1(8d297fcadb63e1e9b0ac1e91ff10321b043f1280) )
ROM_END


ROM_START( as_topsld )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tsmv2-05.bin", 0x0000, 0x080000, CRC(d525422d) SHA1(45b39fb0f4c8c8e7159b8832eca6e80b2cb79ced) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD( "tssv2-05.bin", 0x0000, 0x080000, CRC(d67ffde8) SHA1(f4e3b84c0a6a23f2fe1482b97dc48fb44315101f) )
ROM_END


ROM_START( as_hc )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcm107.u1", 0x00000, 0x080000, CRC(6481cf82) SHA1(975ac90615aee6fbda56707e0cc0776d103e6681) )
	ROM_LOAD16_BYTE( "hcm107.u2", 0x00001, 0x080000, CRC(e5dbba3e) SHA1(545828ad4649665bd8b1125a0943108fc41ef44b) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcs107.u1", 0x00000, 0x080000, CRC(07c68bad) SHA1(32987bac887a02e9b073d16fb51251f7d9196e19) )
	ROM_LOAD16_BYTE( "hcs107.u2", 0x00001, 0x080000, CRC(856c7bbf) SHA1(cf20481f93ebd10c8907f3f909fb0f6f9504b046) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcs107d.u1", 0x0000, 0x080000, CRC(0e3d141e) SHA1(66640deb844d3b23d408dcb15c9fcd8076d4ab15) )
ROM_END

ROM_START( as_hca )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcm109.u1", 0x00000, 0x080000, CRC(25496265) SHA1(72cb61732949cf81db3fa45fc97087ae6f2e0fe2) )
	ROM_LOAD16_BYTE( "hcm109.u2", 0x00001, 0x080000, CRC(85959686) SHA1(866dc6567639a10ddc8628ab7651140c558ac2df) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcs109.u1", 0x00000, 0x080000, CRC(f28ecdfb) SHA1(afc7fc048a8a5950bd0209c3dbe71a5a81b3313a) )
	ROM_LOAD16_BYTE( "hcs109.u2", 0x00001, 0x080000, CRC(61d426f8) SHA1(818ae31de701dca318eeb75a98ba387ee619fc02) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcs109d.u1", 0x00000, 0x080000, CRC(fb755248) SHA1(5432691c46e35592f2975ae2d5c71d69043f43a5) )
ROM_END

ROM_START( as_hcb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcmv1-10.u1", 0x00000, 0x080000, CRC(25dc38e9) SHA1(727d6de290b9078f1c8552b04b1d848863874107) )
	ROM_LOAD16_BYTE( "hcmv1-10.u2", 0x00001, 0x080000, CRC(7f80cb29) SHA1(fdad25bfab6827486b114896fa5dba3d4a0b6ebe) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcsv1-10.u1", 0x00000, 0x080000, CRC(64e90540) SHA1(d0e31d2c9aa5399c07ade7e85fc79f584f3d83ba) )
	ROM_LOAD16_BYTE( "hcsv1-10.u2", 0x00001, 0x080000, CRC(18c61602) SHA1(d4cbd894b65e46914ea858a221e01e222310c7ca) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcsv110d.u1", 0x00000, 0x080000, CRC(6d129af3) SHA1(49397a77d2c7cab507772793138acb543d16cd53) )
ROM_END


ROM_START( as_hcc )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcmv1-11.u1", 0x00000, 0x080000, CRC(7c29dffc) SHA1(185233d02165066e5b845b2821f40f07850c242e) )
	ROM_LOAD16_BYTE( "hcmv1-11.u2", 0x00001, 0x080000, CRC(96c39b83) SHA1(c7bfcd8a33338579837e008cb5cd19b64d291133) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcsv1-11.u1", 0x00000, 0x080000, CRC(cdce1540) SHA1(bf27fbf7670154f6c2048a25754dd79627335314) )
	ROM_LOAD16_BYTE( "hcsv1-11.u2", 0x00001, 0x080000, CRC(4f2b0bfe) SHA1(928ecc985e7d1d23d4db564f6499dd7fea47407a) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcs1-11d.u1", 0x00000, 0x080000, CRC(c4358af3) SHA1(9c08eb2a5daf1d77a4c81cf339a8a0d10f3c30da) )
ROM_END

ROM_START( as_hcd )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcm909.u1", 0x00000, 0x080000, CRC(c3d10181) SHA1(532e71733578fec11f452968fd82f2e656f27fdc) )
	ROM_LOAD16_BYTE( "hcm909.u2", 0x00001, 0x080000, CRC(0a899237) SHA1(8be4937429732f1cddeaa6565a80a0b5430adaee) )

	ROM_REGION( 0x200000, "slavecpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcs909.u1", 0x00000, 0x080000, CRC(cc2553ac) SHA1(2f6f45727b5a160ddf2afeb3c481a92046f80a8f) )
	ROM_LOAD16_BYTE( "hcs909.u2", 0x00001, 0x080000, CRC(97f4a5ea) SHA1(f4e625aabfaaaea71f1feddb3f681cbb7ce1de7e) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "hcs909d.u1", 0x00000, 0x080000, CRC(c5decc1f) SHA1(2824377a479901262614068406c6e7898bc2d9f3) )
ROM_END




ROM_START( as_bigtm )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "bti003.u1", 0x00000, 0x080000, CRC(6751c225) SHA1(c4b69293c40927e81bd473f4b6487f0a05472503) )
	ROM_LOAD16_BYTE( "bti003.u2", 0x00001, 0x080000, CRC(88369abd) SHA1(2d4d06a6598f7b4a2ae1aef0a4c6f571c9de9a0b) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "bti003d.u1", 0x0000, 0x080000, CRC(6eaa5d96) SHA1(6e34c4a20ab140059d80f5f3d2e8a5a5091efd62) )
ROM_END


ROM_START( as_acp )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "acpv403.u1", 0x00000, 0x100000, CRC(b1af78be) SHA1(6cacd0e815cea236130d35ac742076d24d825d66) )
	ROM_LOAD16_BYTE( "acpv403.u2", 0x00001, 0x100000, CRC(ba6d3127) SHA1(f2511455d387ee588b106b9f2823e44b6e9328c9) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "403compak.u1", 0x0000, 0x100000, CRC(d3a37514) SHA1(fe58c3172272a119431cdebcb27b50b8c1d36877) )
	ROM_LOAD16_BYTE( "403conx.u1", 0x0000, 0x100000, CRC(6fbe1035) SHA1(7b03a6b1cc4c89e7bdf583befe18d98e5f4a159b) )
ROM_END

ROM_START( as_celeb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "cel100.u1", 0x00000, 0x080000, CRC(7eb2de99) SHA1(8d24ac812da727b60a043b875f409223c7bd589f) )
	ROM_LOAD16_BYTE( "cel100.u2", 0x00001, 0x080000, CRC(9f5512a2) SHA1(86ec5df6905fe07bff2176c134a48a196f78a772) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cel100d.u1", 0x0000, 0x080000, CRC(7749412a) SHA1(3014cc7c6da6aff95aff6bb7113928a9223c17c8) )
ROM_END


ROM_START( as_celeba )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "cel101.u1", 0x00000, 0x080000, CRC(884187de) SHA1(5a96e47e09607ec91af575c6c29972f6ddcea103) )
	ROM_LOAD16_BYTE( "cel101.u2", 0x00001, 0x080000, CRC(b50b7dfb) SHA1(63b243464eae47a0676b40f2fe65e43e8cea62d7) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cel101d.u1", 0x0000, 0x080000, CRC(81ba186d) SHA1(91c149be902cc028e3e5f8cd4e6c26201366f2a7) )
ROM_END

ROM_START( as_celebb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "cel201.u1", 0x00000, 0x080000, CRC(4b5d11ca) SHA1(b4cd631524a67a18515230ef07bb52dca115472c) )
	ROM_LOAD16_BYTE( "cel201.u2", 0x00001, 0x080000, CRC(42f81484) SHA1(305b04c8300c868551c852c6305588b2331155cc) )
//  ROM_LOAD16_BYTE( "cel201u2.bin", 0x00001, 0x080000, CRC(42f81484) SHA1(305b04c8300c868551c852c6305588b2331155cc) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cel2pnd.u1", 0x0000, 0x080000, CRC(58aa2eac) SHA1(bc38fef5187f5e487c6fdeea404f1eef78b82cb2) )
ROM_END


ROM_START( as_cshah )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "cash1-050lwn-e.u1", 0x00000, 0x100000, CRC(7297ff77) SHA1(419ca98b29e2ae0b8c056201af631320fffb768a) )
	ROM_LOAD16_BYTE( "cash1-050lwn-e.u2", 0x00001, 0x100000, CRC(4d39a9b0) SHA1(44a78c5068791703547cb1618f1b2ccdd5ddb015) )
ROM_END


ROM_START( as_srb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "srb004.u1", 0x0000, 0x080000, CRC(a0ce3300) SHA1(8db1959ad5a51bd395a9105fe74351d043760cbd) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb004d.u1", 0x0000, 0x080000, CRC(a935acb3) SHA1(3bf3ce56410ff9ef6b1b3fcd069c1eb7a1d76e7a) )
	ROM_LOAD( "srb004g.u1", 0x0000, 0x080000, CRC(3f77c830) SHA1(fb64d0046f0b319c6d8d0968d2233e2ab4523ce0) )
ROM_END

ROM_START( as_srba )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "srb100.u1", 0x0000, 0x080000, CRC(9b12d52e) SHA1(bc65b1c6ba8eeaa07ce8320bba655e4726b61b6c) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb100d.u1", 0x0000, 0x080000, CRC(92e94a9d) SHA1(974b3a8c5db445cc7f8a16d90874fd6c765f5145) )
ROM_END

ROM_START( as_srbb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "srb101.u1", 0x0000, 0x080000, CRC(fa4d87af) SHA1(e553c7f70ad5b5cc1b7ed999103b947df1a18203) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb101d.u1", 0x0000, 0x080000, CRC(f3b6181c) SHA1(a62cb5ed72e956d4d6eb983d3f57f7ab3ae2a169) )
ROM_END

ROM_START( as_srbc )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "srb201.u1", 0x0000, 0x080000, CRC(d74fcc7c) SHA1(636b2b847baa25cb9472cc0240852a9995b15e54) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb201d.u1", 0x0000, 0x080000, CRC(deb453cf) SHA1(bda66c1aa0cb39588b2960a3e0065d0a986d0ee0) )
	ROM_LOAD( "srb201g.u1", 0x0000, 0x080000, CRC(48f6374c) SHA1(9f92e1a2a76b6f27cd440decd7e392ee39992936) )
ROM_END

ROM_START( as_srbd )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "srb202.u1", 0x0000, 0x080000, CRC(5035b095) SHA1(4113ca5d9ff1d80dbc2593c21d2eeba12352d9fb) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb202d.u1", 0x0000, 0x080000, CRC(59ce2f26) SHA1(69eed1344ae3f8db0d7edcbf4987b71e5a1ff5ee) )
ROM_END

ROM_START( as_srbe )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "srb203.bin", 0x0000, 0x080000, CRC(cf72c049) SHA1(7d29412a6787312a336ca007a9c9490c8a955f1f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb203d.bin", 0x0000, 0x080000, CRC(c6895ffa) SHA1(457d5aa6c746e0d9a7b9529686f469354a0f94d1) )
ROM_END

void astra_addresslines( UINT16* src, size_t srcsize )
{
	UINT16 *dst = (UINT16*)malloc(srcsize);

	for (int block = 0; block < srcsize; block += 0x100000)
	{
		for (int x = 0; x<0x100000/2;x+=2)
		{
			dst[((block/2)+(x/2))^1] = src[(block/2)+x+1];
			dst[((block/2)+(x/2+0x100000/4))^1] = src[(block/2)+x];
		}
	}

	memcpy(src,dst, srcsize);
	free(dst);	
}


static DRIVER_INIT( astradec )
{
	astra_addresslines( (UINT16*)machine.region( "maincpu" )->base(), machine.region( "maincpu" )->bytes() );
}

static DRIVER_INIT( astradec_dual )
{
	astra_addresslines( (UINT16*)machine.region( "maincpu" )->base(), machine.region( "maincpu" )->bytes() );
	astra_addresslines( (UINT16*)machine.region( "slavecpu" )->base(), machine.region( "slavecpu" )->bytes() );
}


// Single games?
GAME( 200?, as_srb,    0			, astra_single,    astrafr,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V004)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_srba,   as_srb		, astra_single,    astrafr,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V100)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_srbb,   as_srb		, astra_single,    astrafr,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V101)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_srbc,   as_srb		, astra_single,    astrafr,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V201)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_srbd,   as_srb		, astra_single,    astrafr,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V202)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_srbe,   as_srb		, astra_single,    astrafr,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V203)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)


// Linked games (single rom per CPU with master/slave?)
GAME( 200?, as_party,    0			, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Party Time (Astra, V105)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_partya,   as_party	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Party Time (Astra, V110)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_partyb,   as_party	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Party Time (Astra, V112)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_partyc,   as_party	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Party Time (Astra, V206)" ,GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_partyd,   as_party	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Party Time (Astra, V401)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // significantly different set

GAME( 200?, as_letsp,    0			, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Let's Party (Astra, V904)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)

GAME( 200?, as_topsl,   0			, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Top Slot (Astra, V103)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_topsla,  as_topsl	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Top Slot (Astra, V104)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_topslb,  as_topsl	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Top Slot (Astra, V201)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_topslc,  as_topsl	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Top Slot (Astra, V203)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_topsld,  as_topsl	, astrafr_dual,    astrafr,    0, ROT0,  "Astra", "Top Slot (Astra, V205)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)


// Other HW? (has u1/u2 pairing)
GAME( 200?, as_bigtm,   0			, astra_single ,    astrafr,    astradec, ROT0,  "Astra", "Big Time (Astra, V003)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_acp,     0			, astra_single ,    astrafr,    astradec, ROT0,  "Astra", "Unknown Astra 'ACP' (Astra, V403)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL) // no sound data in here?
GAME( 200?, as_celeb,   0			, astra_single ,    astrafr,    astradec, ROT0,  "Astra", "Celebration (Astra, V100)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_celeba,  as_celeb	, astra_single ,    astrafr,    astradec, ROT0,  "Astra", "Celebration (Astra, V101)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_celebb,  as_celeb	, astra_single ,    astrafr,    astradec, ROT0,  "Astra", "Celebration (Astra, V201)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)


// u1/u2 pairing and Linked?

GAME( 200?, as_hc,     0			, astrafr_dual ,    astrafr,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V107)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_hca,    as_hc		, astrafr_dual ,    astrafr,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V109)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_hcb,    as_hc		, astrafr_dual ,    astrafr,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V110)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_hcc,    as_hc		, astrafr_dual ,    astrafr,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V111)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 200?, as_hcd,    as_hc		, astrafr_dual ,    astrafr,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V909)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)

// Non-Astra, same hw?

GAME( 200?, as_cshah,  0	        , astra_single ,    astrafr,    astradec, ROT0,  "Lowen", "Cash Ahoi (Lowen, V105)", GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
