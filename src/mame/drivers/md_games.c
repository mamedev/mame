/*--------------------------------------- NOTICE ------------------------------------

 this file is not a key component of MAME and the sets included here are not
 available in a normal build.

 The sets included in this driver file are intended solely to aid in the deveopment
 of the Genesis emulation in MAME.  The Megadrive / Genesis form the basis of a number
 of official Sega Arcade PCBs as well as a selection of bootlegs; the arcade titles
 however do not provide a sufficient test-bed to ensure high quality emulation.

 If changes are made to the Genesis emulation they should be regression tested against
 a full range of titles.  By including accsesible support for a range of sets such
 bug-fixing and testing becomes easier.

 Many console based arcade bootlegs are also encrypted in some way, so being able
 to reference a standard console version is useful.  Likewise the official Data East
 Arcade release of High Seas Havoc is currently unemulated due to encryption.

----------------------------------------- NOTICE ----------------------------------*/

/* sets with known issues

 --- readd this section

*/


#include "driver.h"
#include "megadriv.h"
#include "deprecat.h"

/****** --PROPER-- dumps ********/

/* NHL94, Cart manufactured by Electronics Art, Includes ROM, RAM, Battery etc. */
ROM_START( g_nh94 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "nhl94 hl9402", 0x000000, 0x100000, CRC(acecd225) SHA1(5a11c7e3c925a6e256d2000b292ad7aa530bda0f) )
	/*
	Rom Labeled

	NHL94 HL9402
	9347
	(C)ELECTRONIC ARTS 1993
	ALL RIGHTS RESERVED
	*/
ROM_END

/* James Pond 2 - Robocod, Cart manufactured By Electronic Arts, Electronic Arts (c)1991 on board */
ROM_START( g_jp2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pond ii robocod rob02", 0x000000, 0x080000,  CRC(c32b5d66) SHA1(20e70c2a8236915a6e4746b6ad1b603563aecf48) )
	/*
	Rom Labeled

	Pond II:Robocod ROB02
	(c)Electronic Arts 1991
	All Rights Reserved
	*/
ROM_END


/****** GAMES ********/



ROM_START( g_bloc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bloc.bin", 0x000000, 0x020000, CRC(5e2966f1) SHA1(f6620d3b712f3bd333d0bb355c08cf992af6e12d) )
ROM_END

ROM_START( g_col )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_col.bin", 0x000000, 0x020000, CRC(d783c244) SHA1(17ae2595e4d3fb705c9f8f66d5938deca3f95c4e) )
ROM_END

ROM_START( g_paca )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_paca.bin", 0x000000, 0x040000,  CRC(5df382f7) SHA1(55e9122e72eeb3439c62cc0de810ea5df74de893) )
ROM_END


ROM_START( g_lem )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lem.bin", 0x000000, 0x100000, CRC(68c70362) SHA1(61d468378c06a3d1044a8e11255294b46d0c094d) )
ROM_END

ROM_START( g_gsh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gsh.bin", 0x000000, 0x100000, CRC(b813cf0d) SHA1(7721e302555f16e3242ad4a47ed87a1de1882122) )
ROM_END

ROM_START( g_tf4 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tf4.bin", 0x000000, 0x100000, CRC(8d606480) SHA1(44be21dead3b55f21762a7c5cf640eec0a1769d9) )
ROM_END

ROM_START( g_will )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_will.bin", 0x000000, 0x100000, CRC(921ebd1c) SHA1(adb0a2edebb6f978c3217075a2f29003a8b025c6) )
ROM_END

ROM_START( g_tje )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tje.bin", 0x000000, 0x100000, CRC(7a588f4b) SHA1(85e8d0a4fac591b25b77c35680ac4175976f251b) )
ROM_END

ROM_START( g_rrsh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rrsh.bin", 0x000000, 0x0c0000, CRC(dea53d19) SHA1(8ff2b2a58b09c42eeb95f633bd8ea5227ac71358) )
ROM_END

ROM_START( g_ddpl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ddpl.bin", 0x000000, 0x040000, CRC(0053bfd6) SHA1(37bc8429a6259582b840e219a2cf412cea5fc873) )
ROM_END



ROM_START( g_soni )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soni.bin", 0x000000, 0x080000, CRC(afe05eee) SHA1(69e102855d4389c3fd1a8f3dc7d193f8eee5fe5b) )
ROM_END

ROM_START( g_son2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_son2.bin", 0x000000, 0x100000, CRC(7b905383) SHA1(8bca5dcef1af3e00098666fd892dc1c2a76333f9) )
ROM_END

ROM_START( g_son2a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_son2a.bin", 0x000000, 0x100000, CRC(f23ad4b3) SHA1(2af1003247aec262089c8df22d05e80d04a1b5e4) )
ROM_END

ROM_START( g_son3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_son3.bin", 0x000000, 0x200000, CRC(9bc192ce) SHA1(75e9c4705259d84112b3e697a6c00a0813d47d71) )
ROM_END

ROM_START( g_sons )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sons.bin", 0x000000, 0x100000, CRC(677206cb) SHA1(24bf6342b98c09775089c9f39cfb2f6fbe7806f7) )
ROM_END

ROM_START( g_wani )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wani.bin", 0x000000, 0x080000, CRC(56f0dbb2) SHA1(4cbe0ad0dbd70fd54432d6ae52d185c39c45260e) )
ROM_END

ROM_START( g_wild )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wild.bin", 0x000000, 0x080000, CRC(0c1a49e5) SHA1(68241498209c5e1f09ca335aee1b0f55ce19ff6e) )
ROM_END

ROM_START( g_cill )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cill.bin", 0x000000, 0x080000, CRC(ba4e9fd0) SHA1(4ac3687634a5acc55ac7f156c6de9749158713e4) )
ROM_END


ROM_START( g_jp3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jp3.bin", 0x000000, 0x200000, CRC(26f64b2a) SHA1(df7fca887e7988e24ab2d08b015c6db2902fe571) )
ROM_END

ROM_START( g_abat )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_abat.bin", 0x000000, 0x200000, CRC(0caaa4ac) SHA1(1da4b3fc92a6572e70d748ebad346aeb291f89f3) )
ROM_END

ROM_START( g_jim2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jim2.bin", 0x000000, 0x300000, CRC(d57f8ba7) SHA1(ef7cccfc5eafa32fc6acc71dd9b71693f64eac94) )
ROM_END

ROM_START( g_anim )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_anim.bin", 0x000000, 0x100000, CRC(86224d86) SHA1(7e9d70b5172b4ea9e1b3f5b6009325fa39315a7c) )
ROM_END

ROM_START( g_gley )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gley.bin", 0x000000, 0x100000, CRC(42cf9b5b) SHA1(529d88f96eb7082bfbc00be3f42a1b2e365c34b7) )
ROM_END

ROM_START( g_rist )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rist.bin", 0x000000, 0x200000, CRC(6511aa61) SHA1(1d15ff596dd4f3b2c1212a2e0c6e2b72f62c001e) )
ROM_END

ROM_START( g_rkni )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rkni.bin", 0x000000, 0x100000, CRC(a6efec47) SHA1(49634bb09c38fa03549577f977e6afb6cebaac48) )
ROM_END

ROM_START( g_srr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_srr.bin", 0x000000, 0x080000, CRC(543bed30) SHA1(0773ae487319b68695e9a6c3dcd0223695c02609) )
ROM_END

ROM_START( g_tazm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tazm.bin", 0x000000, 0x080000, CRC(0e901f45) SHA1(01875bb6484d44a844f3d3e1ae141864664b73b8) )
ROM_END

ROM_START( g_spl3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spl3.bin", 0x000000, 0x200000, CRC(00f05d07) SHA1(7f29f00ec724e20cb93907f1e33ac4af16879827) )
ROM_END

ROM_START( g_mk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mk.bin", 0x000000, 0x200000, CRC(33f19ab6) SHA1(2c4a0618cc93ef7be8329a82ca6d2d16f49b23e0) )
ROM_END

ROM_START( g_comx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_comx.bin", 0x000000, 0x200000, CRC(17da0354) SHA1(e8747eefdf61172be9da8787ba5be447ec73180f) )
ROM_END

ROM_START( g_coss )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_coss.bin", 0x000000, 0x100000, CRC(c593d31c) SHA1(95a3eb13e5d28db8c8ea5ff3e95b0d3e614def69) )
ROM_END

ROM_START( g_dino )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dino.bin", 0x000000, 0x100000, CRC(4608f53a) SHA1(49d4a654dd2f393e43a363ed171e73cd4c8ff4f4) )
ROM_END

ROM_START( g_jbok )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jbok.bin", 0x000000, 0x200000,  CRC(3fb6d92e) SHA1(306021e52501c9d51afd9c51acf384356c84ffbf) )
ROM_END

ROM_START( g_fant )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fant.bin", 0x000000, 0x080000,  CRC(fc43df2d) SHA1(e3ba172d93505f8e0c1763b600f68af11b68f261) )
ROM_END

ROM_START( g_dcap )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dcap.bin", 0x000000, 0x080000,  CRC(73dc0dd8) SHA1(9665f54a6149d71ea72db9c168755e62cb61649c) )
ROM_END

ROM_START( g_pirg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pirg.bin", 0x000000, 0x100000,  CRC(ed50e75c) SHA1(ea597dbefc8f804524606af3c1c4fe6ba55e86e9) )
ROM_END


ROM_START( g_gau4 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gau4.bin", 0x000000, 0x100000,  CRC(3bf46dce) SHA1(26c26ee2bb9571d51537d9328a5fd2a91b4e9dc1) )
ROM_END



ROM_START( g_hard )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hard.bin", 0x000000, 0x040000,  CRC(3225baaf) SHA1(e15a3e704900b9736bc274bcb49d4421f4a3605b) )
ROM_END

ROM_START( g_kawa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kawa.bin", 0x000000, 0x100000,  CRC(631cc8e9) SHA1(fa7e07bbab70a7b5c32a0f2713494b2c10ce8e1e) )
ROM_END

ROM_START( g_jpa2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jpa2.bin", 0x000000, 0x400000,  CRC(140a284c) SHA1(25a93bbcfbbe286a36f0b973adf86b0f0f3cfa3f) )
ROM_END

ROM_START( g_toy )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_toy.bin", 0x000000, 0x400000, CRC(829fe313) SHA1(49be571cd943fd594949c318a0bdbe6263fdd512) )
ROM_END

ROM_START( g_quac )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_quac.bin", 0x000000, 0x140000, CRC(1801098b) SHA1(7b1bd16fb817cc5e2af238cebe8521aaca8f6195) )
ROM_END



ROM_START( g_jimp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jimp.bin", 0x000000, 0x100000,  CRC(1cf3238b) SHA1(38adc1f792b06637e109d4b76fbfbf57623faf3b) )
ROM_END

ROM_START( g_vec2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vec2.bin", 0x000000, 0x300000,  CRC(c1a24088) SHA1(c5adca10408f055c0431e1ffc01d4fbab53ade01) )
ROM_END


ROM_START( g_wbug )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wbug.bin", 0x000000, 0x004000,  CRC(75582ddb) SHA1(bd913dcadba2588d4b9b1aa970038421cba0a1af) )
ROM_END

ROM_START( g_raid )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_raid.bin", 0x000000, 0x100000,  CRC(f839a811) SHA1(9d4011d650179c62acdba0ed58143aa77b97d0e9) )
ROM_END


ROM_START( g_str2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_str2.bin", 0x000000, 0x100000, CRC(42589b79) SHA1(8febb4aff32f40148f572d54e158f4b791736f55) )
ROM_END


ROM_START( g_gf2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gf2.bin", 0x000000, 0x100000,  CRC(d15f5c3c) SHA1(db1615fc239cb0ed9fdc792217964c33e1e700fc) )
ROM_END

ROM_START( g_mic )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mic.bin", 0x000000, 0x080000,  CRC(7ffbd1ad) SHA1(6c822ab42a37657aa2a4e70e44b201f8e8365a98) )
ROM_END

ROM_START( g_mica )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mica.bin", 0x000000, 0x080000,  CRC(e5cf560d) SHA1(bbcf8a40e7bfe09225fdc8fc3f22f8b8cc710d06) )
ROM_END

ROM_START( g_mic2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mic2.bin", 0x000000, 0x100000,  CRC(01c22a5d) SHA1(cb5fb33212592809639b37c2babd72a7953fa102) )
ROM_END

ROM_START( g_mic9 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mic9.bin", 0x000000, 0x100000,  CRC(23319d0d) SHA1(e8ff759679a0df2b3f9ece37ef686f248d3cf37b) )
ROM_END

ROM_START( g_micm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_micm.bin", 0x000000, 0x100000,  CRC(b3abb15e) SHA1(6d3df64ab8bb0b559f216adca62d1cdd74704a26) )
ROM_END

ROM_START( g_mimmx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mimmx.bin", 0x000000, 0x100000,  CRC(a1ad9f97) SHA1(e58572080a6a7918b60a65461f6432582ef1641c) )
ROM_END

ROM_START( g_mmax )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmax.bin", 0x000000, 0x100000,  CRC(55f13a00) SHA1(147364ce2de49a85bb64dd7f1075d9687d4fe89e) )
ROM_END

ROM_START( g_casv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_casv.bin", 0x000000, 0x100000,  CRC(fb1ea6df) SHA1(4809cf80ced70e77bc7479bb652a9d9fe22ce7e6) )
ROM_END


ROM_START( g_asol )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_asol.bin", 0x000000, 0x200000,  CRC(90fa1539) SHA1(8f6eb584ed9487b8504fbc21d86783f58e6c9cd6) )
ROM_END

ROM_START( g_dhed )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dhed.bin", 0x000000, 0x200000,  CRC(3dfeeb77) SHA1(e843decdff262791b1237f1545f5b17c56712d5f) )
ROM_END

ROM_START( g_f98 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f98.bin", 0x000000, 0x200000,  CRC(96947f57) SHA1(6613f13da5494aaaba3222ed5e730ec9ce3c09a7) )
ROM_END

ROM_START( g_cont )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cont.bin", 0x000000, 0x200000,  CRC(c579f45e) SHA1(68ea84146105bda91f6056932ff4fb42aa3eb4a7) )
ROM_END

ROM_START( g_dash )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dash.bin", 0x000000, 0x100000, CRC(dcb76fb7) SHA1(1d9a12b0df291a21f1e1070af2b13c1f15dbbe67) )
ROM_END

ROM_START( g_maui )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_maui.bin", 0x000000, 0x300000, CRC(b2dd857f) SHA1(cd3c50b7f9c2f97d7bb0042e4239a05066ae72e0) )
ROM_END

ROM_START( g_gcm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gcm.bin", 0x000000, 0x200000, CRC(14744883) SHA1(f2df9807fe2659e8f8e6ea43b2031a6abd980873) )
ROM_END

ROM_START( g_iss )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_iss.bin", 0x000000, 0x200000, CRC(9bb3b180) SHA1(ccc60352b43f8c3d536267dd05a8f2c0f3b73df6) )
ROM_END

ROM_START( g_boog )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_boog.bin", 0x000000, 0x300000, CRC(1a7a2bec) SHA1(bb1d23ca4c48d37bf3170d320cc30bfbc2acdfff) )
ROM_END

ROM_START( g_sor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sor.bin", 0x000000, 0x080000, CRC(4052e845) SHA1(731cdf182fe647e4977477ba4dd2e2b46b9b878a) )
ROM_END

ROM_START( g_sor2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sor2.bin", 0x000000, 0x200000, CRC(e01fa526) SHA1(8b656eec9692d88bbbb84787142aa732b44ce0be) )
ROM_END

ROM_START( g_sor3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sor3.bin", 0x000000, 0x300000, CRC(d5bb15d9) SHA1(40a33dd6f9dab0aff26c7525c9b8f342482c7af6) )
ROM_END

ROM_START( g_sgp2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sgp2.bin", 0x000000, 0x100000, CRC(eac8ded6) SHA1(1ee87744d86c4bdd4958cc70d77538351aa206e6) )
ROM_END


ROM_START( g_batf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_batf.bin", 0x000000, 0x300000, CRC(8b723d01) SHA1(6e6776b2c9d0f74e8497287b946ead7f630a35ce) )
ROM_END

ROM_START( g_boas )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_boas.bin", 0x000000, 0x300000, CRC(c4728225) SHA1(2944910c07c02eace98c17d78d07bef7859d386a) )
ROM_END

ROM_START( g_chk2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chk2.bin", 0x000000, 0x100000, CRC(408b1cdb) SHA1(f5dc902a3d722842a4fb536644102de600e7dca3) )
ROM_END

ROM_START( g_alad )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alad.bin", 0x000000, 0x200000, CRC(ed427ea9) SHA1(d21c085b8429edc2c5092cd74ef3c36d01bf987f) )
ROM_END

ROM_START( g_bean )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bean.bin", 0x000000, 0x100000, CRC(c7ca517f) SHA1(aa6b60103fa92bc95fcc824bf1675e411627c8d3) )
ROM_END

ROM_START( g_blee )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blee.bin", 0x000000, 0x200000, CRC(efe850e5) SHA1(01b45b9865282124253264c5f2e3d3338858ff92) )
ROM_END

ROM_START( g_eco2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_eco2.bin", 0x000000, 0x200000, CRC(ccb21f98) SHA1(82ec466a81b95942ad849c5e2f88781bef28acc8) )
ROM_END

ROM_START( g_exos )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_exos.bin", 0x000000, 0x100000, CRC(10ec03f3) SHA1(d958c3f2365162cb2ffa37fcea36695a1d4ab287) )
ROM_END

ROM_START( g_fatr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fatr.bin", 0x000000, 0x080000, CRC(e91aed05) SHA1(02634d919ec7d08f3c6833f229b5127dd52c9e8a) )
ROM_END


ROM_START( g_kgs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kgs.bin", 0x000000, 0x080000, CRC(21dbb69d) SHA1(cabd42d2edd333871269a7bc03a68f6765d254ce) )
ROM_END

ROM_START( g_kgsr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kgsr.bin", 0x000000, 0x100000, CRC(b8e7668a) SHA1(47cdd668998139c92305e1b5abf7e196901490d6) )
ROM_END

ROM_START( g_gtwi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gtwi.bin", 0x000000, 0x100000, CRC(7ae5e248) SHA1(ce565d7df0abee02879b85866de0e1a609729ad8) )
ROM_END

ROM_START( g_garg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_garg.bin", 0x000000, 0x300000, CRC(2d965364) SHA1(2b76764ca1e5a26e406e42c5f8dbd5b8df915522) )
ROM_END

ROM_START( g_gng )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gng.bin", 0x000000, 0x0a0000, CRC(4f2561d5) SHA1(aab6d20f01db51576b1d7cafab46e613dddf7f8a) )
ROM_END

ROM_START( g_daim )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_daim.bin", 0x000000, 0x0a0000, CRC(5659f379) SHA1(a08a5764dc8651d31ef72466028cfb87fb6dd166) )
ROM_END

ROM_START( g_hsh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hsh.bin", 0x000000, 0x100000, CRC(17be551c) SHA1(0dc1969098716ba332978b89356f62961417682b) )
ROM_END


ROM_START( g_lgal )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lgal.bin", 0x000000, 0x100000, CRC(679557bc) SHA1(3368af01da1f3f9e0ea7c80783170aeb08f5c24d) )
ROM_END

ROM_START( g_lion )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lion.bin", 0x000000, 0x300000, CRC(5696a5bc) SHA1(24cbe4e75ec10e8e0ffbdb400cf86ecb072d4da9) )
ROM_END

ROM_START( g_mazi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mazi.bin", 0x000000, 0x100000, CRC(1bd9fef1) SHA1(5cb87bb4efc8fa8754e687b62d0fd858e624997d) )
ROM_END

ROM_START( g_mick )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mick.bin", 0x000000, 0x100000, CRC(40f17bb3) SHA1(4fd2818888a3c265e148e9be76525654e76347e4) )
ROM_END

ROM_START( g_mman )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mman.bin", 0x000000, 0x200000, CRC(629e5963) SHA1(20779867821bab019f63ce42e3067ffafb4fe480) )
ROM_END

ROM_START( g_pano )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pano.bin", 0x000000, 0x280000, CRC(9e57d92e) SHA1(63d9b71cfe0b7aaa6913ae06c1f2547d6d2aae5c) )
ROM_END

ROM_START( g_poca )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_poca.bin", 0x000000, 0x400000, CRC(6ddd1c6d) SHA1(b57ae70f1d735c8052991053297a0c878f8a9417) )
ROM_END

ROM_START( g_pmon )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pmon.bin", 0x000000, 0x100000, CRC(fb599b86) SHA1(f088e8cdb90c037b4ce1c5ae600a75f4e4219c87) )
ROM_END


ROM_START( g_puls )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_puls.bin", 0x000000, 0x200000, CRC(138a104e) SHA1(5fd76a5f80e4684b5f9d445ddb893679985684e6) )
ROM_END

ROM_START( g_redz )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_redz.bin", 0x000000, 0x200000, CRC(56512ee5) SHA1(2e01a1ee8ff31b3eee0c422d30b529aedd7ffdc1) )
ROM_END

ROM_START( g_sock )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sock.bin", 0x000000, 0x100000, CRC(3c14e15a) SHA1(fee9eb272362bd6f36d552a9ebfd25f5f0db7d2f) )
ROM_END

ROM_START( g_skit )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_skit.bin", 0x000000, 0x200000, CRC(f785f9d7) SHA1(98be93964c14ebc91727b429dd7a7a563b4e2e9f) )
ROM_END

ROM_START( g_s3d )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s3d.bin", 0x000000, 0x400000, CRC(44a2ca44) SHA1(89957568386a5023d198ac2251ded9dfb2ab65e7) )
ROM_END

ROM_START( g_spo2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spo2.bin", 0x000000, 0x300000, CRC(bdad1cbc) SHA1(064a384f745eeffee3621d55a0278c133abdbc11) )
ROM_END

ROM_START( g_subt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_subt.bin", 0x000000, 0x200000, CRC(dc3c6c45) SHA1(70a5d4da311dd8a92492d01676bf9170fa4bd095) )
ROM_END

ROM_START( g_sfz )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sfz.bin", 0x000000, 0x100000, CRC(767780d7) SHA1(14dc8568205f3b1b89ce14b9412541ce4ae47f91) )
ROM_END

ROM_START( g_sho )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sho.bin", 0x000000, 0x080000, CRC(3877d107) SHA1(e58a8e6c472a34d9ecf3b450137df8a63ec9c791) )
ROM_END

ROM_START( g_smgp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smgp.bin", 0x000000, 0x080000, CRC(be91b28a) SHA1(1e49a449367f0ec7ba0331b7b0d074f796e48d58) )
ROM_END

ROM_START( g_stb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stb.bin", 0x000000, 0x080000, CRC(b13087ee) SHA1(21810b4a309a5b9a70965dd440e9aeed0b6ca4c5) )
ROM_END

ROM_START( g_tta )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tta.bin", 0x000000, 0x080000, CRC(a26d3ae0) SHA1(d8d159c7c5a365242f989cc3aad2352fb27e3af3) )
ROM_END

ROM_START( g_tyra )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tyra.bin", 0x000000, 0x100000,  CRC(a744921e) SHA1(99ae974fccebfbf6f5a4738e953ad55181144a99) )
ROM_END

ROM_START( g_vf2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vf2.bin", 0x000000, 0x400000, CRC(937380f3) SHA1(c283bf31b646489c2341f8325c52fb8b788a3702) )
ROM_END

ROM_START( g_wizl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wizl.bin", 0x000000, 0x100000, CRC(df036b62) SHA1(1cbd0f820aaf35dd6328a865ecc4b9eedc740807) )
ROM_END

ROM_START( g_ztks )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ztks.bin", 0x000000, 0x200000, CRC(423968df) SHA1(db110fa33c185a7da4c80e92dbe4c7f23ccec0d6) )
ROM_END

ROM_START( g_popu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_popu.bin", 0x000000, 0x080000, CRC(bd74b31e) SHA1(89907c4ba4fd9db4e8ef2271c0253bb0e4b6d52d) )
ROM_END

ROM_START( g_pino )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pino.bin", 0x000000, 0x300000, CRC(cd4128d8) SHA1(ad6098fe3642489d0281b1f168319a753e8bd02a) )
ROM_END

ROM_START( g_puyo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_puyo.bin", 0x000000, 0x080000, CRC(7f26614e) SHA1(02656b5707cf9452d4cf48378ffd3a95dc84e9c5) )
ROM_END

ROM_START( g_revs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_revs.bin", 0x000000, 0x080000, CRC(4d35ebe4) SHA1(7ed6f142884a08bb3c07a2e6e35405440343502f) )
ROM_END

ROM_START( g_seaq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_seaq.bin", 0x000000, 0x200000, CRC(25b05480) SHA1(ba34c2016684f1d3fb0854de4f8a92390e24bc52) )
ROM_END

ROM_START( g_tale )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tale.bin", 0x000000, 0x080000, CRC(f5c0c8d0) SHA1(9b6ab86fea23adb3cfba38b893278d856540c8b8) )
ROM_END

ROM_START( g_zool )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zool.bin", 0x000000, 0x100000, CRC(cb2939f1) SHA1(ee016f127b81f1ca25657e8fa47fac0c4ed15c97) )
ROM_END

ROM_START( g_aate )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aate.bin", 0x000000, 0x080000, CRC(e755dd51) SHA1(aadf22d36cd745fc35676c07f04df3de579c422c) )
ROM_END

ROM_START( g_balz )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_balz.bin", 0x000000, 0x200000, CRC(b362b705) SHA1(4825cb9245a701cc59900c57a7a5fa70edc160f0) )
ROM_END

ROM_START( g_bsht )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bsht.bin", 0x000000, 0x200000, CRC(f9f2bceb) SHA1(513005efd123539a905986130d15125085837559) )
ROM_END


ROM_START( g_bonk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bonk.bin", 0x000000, 0x100000, CRC(d1e66017) SHA1(938642252fdb1c5aedc785bce2ba383fc683c917) )
ROM_END


ROM_START( g_mars )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mars.bin", 0x000000, 0x200000, CRC(c76558df) SHA1(3e761bdbf4ab8cb43d4a8d99e22b7de896884819) )
ROM_END

ROM_START( g_fdma )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fdma.bin", 0x000000, 0x04000, CRC(549cebf4) SHA1(c44f0037a6856bd52a212b26ea416501e92f7645) )
ROM_END

ROM_START( g_3nin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_3nin.bin", 0x000000, 0x200000, CRC(e5a24999) SHA1(d634e3f04672b8a01c3a6e13f8722d5cbbc6b900) )
ROM_END

ROM_START( g_acro )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_acro.bin", 0x000000, 0x100000, CRC(a3a7a8b5) SHA1(438fec09e05337a063f986632b52c9d60dd03ba1) )
ROM_END

ROM_START( g_acr2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_acr2.bin", 0x000000, 0x200000, CRC(39eb74eb) SHA1(6284a8a6c39137493b1ccc01fcd115cdfb775750) )
ROM_END

ROM_START( g_haun )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_haun.bin", 0x000000, 0x200000, CRC(c9fc876d) SHA1(d6227ef00fdded9184676edf5dd2f6cae9b244a5) )
ROM_END

ROM_START( g_pugg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pugg.bin", 0x000000, 0x100000, CRC(70132168) SHA1(5f4d1acd1b8580e83952e1083cfa881b0ec5b9fd) )
ROM_END

ROM_START( g_sams )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sams.bin", 0x000000, 0x300000, CRC(5bb8b2d4) SHA1(504cc7a948f96ce1989f8e91ffb42acc6a956e47) )
ROM_END

ROM_START( g_mmf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmf.bin", 0x000000, 0x200000, CRC(2b8c8cce) SHA1(b73d01599d8201dc3bd98fdc9cc262e14dbc52b4) )
ROM_END

ROM_START( g_mfli )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mfli.bin", 0x000000, 0x100000, CRC(bef9a4f4) SHA1(f37518380d3e41598cbff16519ca15fcb751de57) )
ROM_END

ROM_START( g_skid )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_skid.bin", 0x000000, 0x200000, CRC(4a9c62f9) SHA1(957850ce688dcf9c5966f6712b4804f0a5b7457d) )
ROM_END

ROM_START( g_or20 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_or20.bin", 0x000000, 0x100000, CRC(e32e17e2) SHA1(d4a9f1992d69d849f7d4d770a1f4f665a5352c78) )
ROM_END

ROM_START( g_rwoo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rwoo.bin", 0x000000, 0x100000, CRC(d975e93c) SHA1(5664febf187d26274996c888778b0ff5268474f2) )
ROM_END

ROM_START( g_rolo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rolo.bin", 0x000000, 0x080000, CRC(306861a2) SHA1(8f27907df777124311b7415cd52775641276cf0d) )
ROM_END

ROM_START( g_lot )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lot.bin", 0x000000, 0x100000, CRC(a3cf6e9c) SHA1(b6944c40b7d7e7548942efb4a732ad20d4500c0c) )
ROM_END

ROM_START( g_lot2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lot2.bin", 0x000000, 0x100000, CRC(1d8ee010) SHA1(5a00e8aaae8d987ee0f18fc4604230265aea699e) )
ROM_END

ROM_START( g_stra )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stra.bin", 0x000000, 0x100000, CRC(1a58d5fe) SHA1(95aa250ea47d14d60da9f0fed5b1aad1ff2c1862) )
ROM_END

ROM_START( g_tinh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tinh.bin", 0x000000, 0x100000, CRC(d6724b84) SHA1(e5acf758e76c95017a6ad50ab0f6ae2db5c9e8bc) )
ROM_END

ROM_START( g_tg2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tg2.bin", 0x000000, 0x100000, CRC(bd3074d2) SHA1(e548e2ed4f69c32dd601a2b90bcf4eeb34d36c49) )
ROM_END

ROM_START( g_gdog )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gdog.bin", 0x000000, 0x080000, CRC(c4820a03) SHA1(fe47514e9aeecabaed954a65d7241079dfec3d9e) )
ROM_END

ROM_START( g_gaia )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gaia.bin", 0x000000, 0x100000, CRC(5d8bf68b) SHA1(f62e8be872dc116c4cc331c50ae63a63f013eb58) )
ROM_END

ROM_START( g_gods )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gods.bin", 0x000000, 0x100000, CRC(fd234ccd) SHA1(bfc84beba074c7dc58b0b4fcac73fffcf0c6b585) )
ROM_END

ROM_START( g_gyno )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gyno.bin", 0x000000, 0x080000, CRC(1b69241f) SHA1(9b99b109f5f1f221f08672a7c4752a94067b62b0) )
ROM_END


ROM_START( g_elem )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_elem.bin", 0x000000, 0x080000, CRC(390918c6) SHA1(fd47e4f8c2d6cc82860ae281faad50e3dee2d033) )
ROM_END


ROM_START( g_mano )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mano.bin", 0x000000, 0x100000, CRC(cae0e3a6) SHA1(36c66a1cbf44f4019b84538a6bf452c369239bc9) )
ROM_END


ROM_START( g_daze )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_daze.bin", 0x000000, 0x200000, CRC(b95e25c9) SHA1(240c0d9487d7659a4b2999e0394d552ab17bee8a) )
ROM_END


ROM_START( g_jwws )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jwws.bin", 0x000000, 0x080000, CRC(0aef5b1f) SHA1(040c810ccbe6310d369aa147471213d898ec2ad5) )
ROM_END


ROM_START( g_exmu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_exmu.bin", 0x000000, 0x100000, CRC(33b1979f) SHA1(20b282ff8220291d98853888cfe504692c04e654) )
ROM_END

ROM_START( g_busq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_busq.bin", 0x000000, 0x080000, CRC(28c4a006) SHA1(0457b0220c86f58d8249cbd5987f63c5439d460c) )
ROM_END

ROM_START( g_olan )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olan.bin", 0x000000, 0x100000, CRC(fe6f2350) SHA1(48b764d85c435ddebba39dc160fd70509b76d94e) )
ROM_END

ROM_START( g_shar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shar.bin", 0x000000, 0x080000, CRC(e5c9cbb0) SHA1(db4285e4ffb69aa9f1ca68c4103fbfd0843f7b86) )
ROM_END

ROM_START( g_pidw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pidw.bin", 0x000000, 0x200000, CRC(0c45b9f7) SHA1(a95eabc00ed9f4867f394c1e5e03afad0381843b) )
ROM_END

ROM_START( g_bbrb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bbrb.bin", 0x000000, 0x100000, CRC(13e7b519) SHA1(e98c7b232e213a71f79563cf0617caf0b3699cbf) )
ROM_END

ROM_START( g_bbbq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bbbq.bin", 0x000000, 0x100000, CRC(befb6fae) SHA1(f88a712ae085ac67f49cb0a8fa16a47e82e780cf) )
ROM_END

ROM_START( g_toki )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_toki.bin", 0x000000, 0x080000, CRC(7362c3f4) SHA1(77270a6ded838d0263284dcc075aa4b2b2aef234) )
ROM_END

ROM_START( g_mtur )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mtur.bin", 0x000000, 0x100000, CRC(fe898cc9) SHA1(180285dbfc1613489f1c20e9fd6c2b154dec7fe2) )
ROM_END

ROM_START( g_tanr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tanr.bin", 0x000000, 0x200000, CRC(d2d2d437) SHA1(2bf6965ee883a70b4f0842e9efa17c5e20b5cb47) )
ROM_END

ROM_START( g_rop )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rop.bin", 0x000000, 0x100000, CRC(41fcc497) SHA1(84851ce4527761b8d74ce581c19e15d0dd17f368) )
ROM_END

ROM_START( g_seni )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_seni.bin", 0x000000, 0x080000, CRC(04e3bcca) SHA1(b88833ba92e084d4433e2b22eeb67e71ed36cd5c) )
ROM_END

ROM_START( g_ps96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ps96.bin", 0x000000, 0x200000, CRC(14e3fb7b) SHA1(139b9cea418c7c932033ddff87bf10466828cbfa) )
ROM_END

ROM_START( g_toug )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_toug.bin", 0x000000, 0x400000, CRC(e19fbc93) SHA1(c7c533b25a50b9c1ccd4c9772bf50957f728c074) )
ROM_END

ROM_START( g_spir )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spir.bin", 0x000000, 0x100000, CRC(6634b130) SHA1(5f18db9c85df4eaf4647a0519c9dc966aee583fa) )
ROM_END

ROM_START( g_strk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_strk.bin", 0x000000, 0x200000, CRC(cc5d7ab2) SHA1(9917c35a263cc9bd922d55bf59d01bc2733b4e24) )
ROM_END

ROM_START( g_wcs2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wcs2.bin", 0x000000, 0x100000, CRC(c1dd1c8e) SHA1(f6ce0b826e028599942957729d72c7a8955c5e35) )
ROM_END

ROM_START( g_rrs2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rrs2.bin", 0x000000, 0x100000, CRC(7b29c209) SHA1(6fa6420a2abcf5c2c4620c19b1f2a831996af481) )
ROM_END

ROM_START( g_rrs3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rrs3.bin", 0x000000, 0x200000, CRC(15785956) SHA1(b2324833bf81223a9626a6060cafc203dd203468) )
ROM_END

ROM_START( g_zomb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zomb.bin", 0x000000, 0x100000, CRC(2bf3626f) SHA1(57090434ee3ffb060b33a1dcce91d1108bf60c47) )
ROM_END
ROM_START( g_yuyu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yuyu.bin", 0x000000, 0x300000, CRC(71ceac6f) SHA1(89a70d374a3f3e2dc5b430e6d444bea243193b74) )
ROM_END
ROM_START( g_devi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_devi.bin", 0x000000, 0x080000, CRC(d3f300ac) SHA1(bc6f346a162cf91a788a88e44daa246d3d31ee8b) )
ROM_END
ROM_START( g_alis )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alis.bin", 0x000000, 0x100000, CRC(d28d5c40) SHA1(69bf7a9ebcb851e0d571f93b26d785ca87621b01) )
ROM_END
ROM_START( g_arun )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arun.bin", 0x000000, 0x100000, CRC(0677c210) SHA1(8a6e868fe36f2e5ac01af2557d3798a892e34799) )
ROM_END
ROM_START( g_orur )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_orur.bin", 0x000000, 0x200000, CRC(ede636b9) SHA1(5d00ad7d9dccd9067c4bd716c2ab9c0a18930ae2) )
ROM_END



ROM_START( g_orun )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_orun.bin", 0x000000, 0x100000, CRC(fdd9a8d2) SHA1(95d2055ffd679ab19f0d4ca0af62a0d565bc258e) )
ROM_END
ROM_START( g_chee )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chee.bin", 0x000000, 0x200000, CRC(ff634b28) SHA1(12bb0d838c675b34cecc9041d3560fe35145ec39) )
ROM_END
ROM_START( g_nbah )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nbah.bin", 0x000000, 0x300000, CRC(176b0338) SHA1(bff36de7e0ca875b1fab84928f00999b48ff8f02) )
ROM_END
ROM_START( g_bhb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bhb.bin", 0x000000, 0x100000, CRC(95b0ea2b) SHA1(dca9d505302ce9ff1f98c4da95505139c7d3cafc) )
ROM_END
ROM_START( g_vect )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vect.bin", 0x000000, 0x200000, CRC(d38b3354) SHA1(57a64d08028b539dc236a693d383f2e1269a5dd4) )
ROM_END
ROM_START( g_stol )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stol.bin", 0x000000, 0x080000, CRC(39ab50a5) SHA1(1bf4b58d50fdc0fdc173ce3dcadcc5d9b58f0723) )
ROM_END


ROM_START( g_td2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_td2.bin", 0x000000, 0x100000, CRC(f9bdf8c5) SHA1(cb70e5de149521f20723413cd11c5e661ec63c3e) )
ROM_END
ROM_START( g_tout )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tout.bin", 0x000000, 0x080000, CRC(0c661369) SHA1(3e4b9881dde758ef7bb090b39d3556d9bc0d9f1e) )
ROM_END
ROM_START( g_sprk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sprk.bin", 0x000000, 0x100000, CRC(6bdb14ed) SHA1(d6e2f4aa87633aeea2ec1c05a6100b8905549095) )
ROM_END
ROM_START( g_garf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_garf.bin", 0x000000, 0x200000, CRC(f0ff078e) SHA1(9fff7dea16c4d0e6c9d6dbaade20c7048bb485ec) )
ROM_END
ROM_START( g_dfry )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dfry.bin", 0x000000, 0x080000, CRC(58037bc6) SHA1(bcbafa6c4ab0b16ddb4f316a1ef8c0eecd0cd990) )
ROM_END
ROM_START( g_drev )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drev.bin", 0x000000, 0x100000, CRC(841edbc0) SHA1(75854a732d4cf9a310c4359092cf5c2482df49a7) )
ROM_END

ROM_START( g_afam )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_afam.bin", 0x000000, 0x100000, CRC(71f58614) SHA1(1cd7ac493a448c7486eefde2300240c3675986a9) )
ROM_END
ROM_START( g_afav )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_afav.bin", 0x000000, 0x200000, CRC(b906b992) SHA1(b7f138e7658a0151ad154ddaed18aea10e114c46) )
ROM_END
ROM_START( g_abus )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_abus.bin", 0x000000, 0x080000, CRC(f3d65baa) SHA1(239636cc38a865359b2deeb5f8dc3fd68da41209) )
ROM_END
ROM_START( g_ali3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ali3.bin", 0x000000, 0x080000, CRC(b327fd1b) SHA1(26959752d0683298146c2a89599fa598d009651b) )
ROM_END
ROM_START( g_arcu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arcu.bin", 0x000000, 0x100000, CRC(bc4d9b20) SHA1(26feaafad8c464ce3fa96912c2a85c7596e5fa4e) )
ROM_END
ROM_START( g_arie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arie.bin", 0x000000, 0x080000, CRC(58e297df) SHA1(201105569535b7c8f11bd97b93cbee884c7845c4) )
ROM_END
ROM_START( g_arro )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arro.bin", 0x000000, 0x080000, CRC(4d89e66b) SHA1(916524d2b403a633108cf457eec13b4df7384d95) )
ROM_END
ROM_START( g_asgr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_asgr.bin", 0x000000, 0x200000, CRC(4735fee6) SHA1(e6353362ba261f1f8efb31e624250e74e1ca0da1) )
ROM_END
ROM_START( g_aspg )
	ROM_REGION( 0x400000, "main", 0 )/* 68000 Code */
	ROM_LOAD( "g_aspg.bin", 0x000000, 0x200000, CRC(4ff1d83f) SHA1(189c1e7dd280d0d621eb9e895831fb8109e3e3ab) )
ROM_END
ROM_START( g_awep )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_awep.bin", 0x000000, 0x200000, CRC(1f07577f) SHA1(a0fe802de7874c95c355ec12b29136651fe0af28) )
ROM_END

ROM_START( g_bob )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bob.bin", 0x000000, 0x100000, CRC(eaa2acb7) SHA1(84d63c848d68d844dbcf360040a87a0c93d67f74) )
ROM_END
ROM_START( g_blma )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blma.bin", 0x000000, 0x100000, CRC(c11e4ba1) SHA1(20c9b85f543ef8e66a97e6403cf486045f295d48) )
ROM_END
ROM_START( g_body )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_body.bin", 0x000000, 0x100000, CRC(3575a030) SHA1(3098f3c7ea9e25bf86219d7ed0795bf363338714) )
ROM_END
ROM_START( g_drac )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drac.bin", 0x000000, 0x100000, CRC(077084a6) SHA1(4ede0e75054655acab63f2a41b8c57e1cf137e58) )
ROM_END
ROM_START( g_brpw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_brpw.bin", 0x000000, 0x200000, CRC(98d502cd) SHA1(51b712e87e8f9cf7a93cfc78ec27d2e80b316ec3) )
ROM_END
ROM_START( g_buba )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_buba.bin", 0x000000, 0x100000, CRC(d45cb46f) SHA1(907c875794b8e3836e5811c1f28aa90cc2c8ffed) )
ROM_END
ROM_START( g_bubs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bubs.bin", 0x000000, 0x200000, CRC(3e30d365) SHA1(719140754763e5062947ef9e76ee748cfad38202) )
ROM_END
ROM_START( g_bub2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bub2.bin", 0x000000, 0x200000, CRC(f8beff56) SHA1(0cfb6c619798ba47f35069dea094fbc96f974ecb) )
ROM_END
ROM_START( g_bbny )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bbny.bin", 0x000000, 0x200000, CRC(365305a2) SHA1(50b18d9f9935a46854641c9cfc5b3d3b230edd5e) )
ROM_END
ROM_START( g_cano )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cano.bin", 0x000000, 0x180000, CRC(ad217654) SHA1(ced5f0967e30c3b4c4c2a81007a7db2910b1885d) )
ROM_END


ROM_START( g_cpoo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cpoo.bin", 0x000000, 0x100000, CRC(253512cf) SHA1(443b96c518120078fb33f3ff9586a2b7ebc141c7) )
ROM_END
ROM_START( g_chao )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chao.bin", 0x000000, 0x180000, CRC(bd9eecf4) SHA1(b72d36565e13ab04dc20c547e0dcee1f67bcdb42) )
ROM_END
ROM_START( g_chq2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chq2.bin", 0x000000, 0x080000, CRC(f39e4bf2) SHA1(47c8c173980749aca075b9b3278c0df89a21303f) )
ROM_END
ROM_START( g_cool )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cool.bin", 0x000000, 0x100000, CRC(f024c1a1) SHA1(9a214e0eab58ddb8e9d752e41fce2ce08e6c39a7) )
ROM_END
ROM_START( g_cybo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cybo.bin", 0x000000, 0x080000, CRC(ab0d1269) SHA1(6d0c72fa5e53d897390707eb4c6d3e86e6772215) )
ROM_END
ROM_START( g_dicv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dicv.bin", 0x000000, 0x200000, CRC(1312cf22) SHA1(2aac7a1cf92e51a14fd3e12a830e135fee255044) )
ROM_END
ROM_START( g_ejim )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ejim.bin", 0x000000, 0x300000, CRC(df3acf59) SHA1(a544211d1ebab1f096f6e72a0d724f74f9ddbce8) )
ROM_END
ROM_START( g_taz2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_taz2.bin", 0x000000, 0x200000, CRC(62009f8c) SHA1(7aa9c7b74541d414d81aa61d150803c3b3b1701b) )
ROM_END

ROM_START( g_fand )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fand.bin", 0x000000, 0x080000, CRC(46447e7a) SHA1(b320174d3b43f30b477818a27b4da30462a52003) )
ROM_END

ROM_START( g_fanda )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fanda.bin", 0x000000, 0x080000, CRC(86b2a235) SHA1(ddda3664e0e7e2999815cce50e3c02170a8fec52) )
ROM_END

ROM_START( g_fbak )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fbak.bin", 0x000000, 0x180000, CRC(23a9616d) SHA1(bce40031f6adab48670c8a2d73e42f3a3dcba97c) )
ROM_END
ROM_START( g_huni )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_huni.bin", 0x000000, 0x080000, CRC(1acbe608) SHA1(198e243cc21e4aea97e2c6aca376cb2ae70bc1c9) )
ROM_END
ROM_START( g_hell )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hell.bin", 0x000000, 0x080000, CRC(184018f9) SHA1(9a91bef8a07f709e1f79f8519da79eac34d1796d) )
ROM_END
ROM_START( g_hook )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hook.bin", 0x000000, 0x100000, CRC(2c48e712) SHA1(67031af6ec4b771bd8d69a44c9945562a063593e) )
ROM_END
ROM_START( g_huma )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_huma.bin", 0x000000, 0x100000, CRC(a0cf4366) SHA1(21ad6238edf0aa0563782ef17439b0c73a668059) )
ROM_END

ROM_START( g_jpra )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jpra.bin", 0x000000, 0x200000, CRC(98b4aa1b) SHA1(535c78d91f76302a882d69ff40b3d0f030a5b6ae) )
ROM_END
ROM_START( g_ksfh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ksfh.bin", 0x000000, 0x080000, CRC(56976261) SHA1(6aa026e394dba0e5584c4cf99ad1c166d91f3923) )
ROM_END
ROM_START( g_land )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_land.bin", 0x000000, 0x200000, CRC(fbbb5b97) SHA1(24345e29427b000b90df778965dd8834300a9dde) )
ROM_END
ROM_START( g_mhat )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mhat.bin", 0x000000, 0x080000, CRC(e43e853d) SHA1(1a741125a80ba9207c74a32e5b21dbb347c3c34a) )
ROM_END
ROM_START( g_mtla )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mtla.bin", 0x000000, 0x100000, CRC(04ef4899) SHA1(bcb77c10bc8f3322599269214e0f8dde32b01a5c) )
ROM_END
ROM_START( g_nutz )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nutz.bin", 0x000000, 0x100000, CRC(0786ea0b) SHA1(318ff3a44554b75260d5b9b9e7b81a3cfd07581a) )
ROM_END

ROM_START( g_ooze )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ooze.bin", 0x000000, 0x100000, CRC(1c0dd42f) SHA1(dafcda90285e71151072ba36134f94f1e9d76a23) )
ROM_END
ROM_START( g_ootw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ootw.bin", 0x000000, 0x100000, CRC(2da36e01) SHA1(f00f96fb1b346d5b13c6bc8bc586477fed743800) )
ROM_END
ROM_START( g_page )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_page.bin", 0x000000, 0x200000, CRC(75a96d4e) SHA1(46d0495638e618bb9ac7fb07f46f8f0d61a8cedc) )
ROM_END
ROM_START( g_2040 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2040.bin", 0x000000, 0x200000, CRC(fb36e1f3) SHA1(260b9f8fbd681bb1fc445538726836f0d423b4cc) )
ROM_END
ROM_START( g_ppin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ppin.bin", 0x000000, 0x180000, CRC(d704784b) SHA1(699db175ab8a6abdd941b8a3d81cf203e01053f5) )
ROM_END
ROM_START( g_ranx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ranx.bin", 0x000000, 0x100000, CRC(55915915) SHA1(2611c818ecb53edc7d561fce1a349acad6abff4a) )
ROM_END

ROM_START( g_shi3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shi3.bin", 0x000000, 0x100000, CRC(5381506f) SHA1(1e07d7998e3048fcfba4238ae96496460e91b3a5) )
ROM_END
ROM_START( g_krew )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_krew.bin", 0x000000, 0x200000, CRC(c2e05acb) SHA1(2a6f6ea7d2fc1f3a396269f9455011ef95266ffc) )
ROM_END
ROM_START( g_sks3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sks3.bin", 0x000000, 0x400000, CRC(63522553) SHA1(cfbf98c36c776677290a872547ac47c53d2761d6) )
ROM_END
ROM_START( g_skik )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_skik.bin", 0x000000, 0x080000, CRC(f43793ff) SHA1(4e270b13a399d78d919157e50ab11f4645aa6d32) )
ROM_END
ROM_START( g_sylv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sylv.bin", 0x000000, 0x200000, CRC(89fc54ce) SHA1(bf50d0afe82966907671a46060d27b7b5d92a752) )
ROM_END
ROM_START( g_term )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_term.bin", 0x000000, 0x100000, CRC(31a629be) SHA1(2a1894e7f40b9001961f7bf1c70672351aa525f9) )
ROM_END
ROM_START( g_tje2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tje2.bin", 0x000000, 0x200000, CRC(aa021bdd) SHA1(0ea0da09183eb01d030515beeb40c1427c6e1f07) )
ROM_END
ROM_START( g_wolv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wolv.bin", 0x000000, 0x200000, CRC(d2437bb7) SHA1(261f8ed75586ca7cdec176eb81550458bf1ff437) )
ROM_END
ROM_START( g_uqix )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_uqix.bin", 0x000000, 0x040000, CRC(d83369d9) SHA1(400edf467e20f8f43b7f7c8f18f5f46ed54eac86) )
ROM_END

ROM_START( g_real )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_real.bin", 0x000000, 0x200000, CRC(fdc80bfc) SHA1(fa579a7e47ee4a1bdb080bbdec1eef480a85293e) )
ROM_END
ROM_START( g_abz2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_abz2.bin", 0x000000, 0x100000, CRC(9377f1b5) SHA1(f29b148c052f50a89e3b877ce027a0d5aaa387d1) )
ROM_END
ROM_START( g_abu2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_abu2.bin", 0x000000, 0x080000, CRC(ccafe00e) SHA1(523e4abdf19794a167a347b7eeca79907416e084) )
ROM_END
ROM_START( g_aqua )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aqua.bin", 0x000000, 0x080000, CRC(400f4ba7) SHA1(3bbd0853099f655cd33b52d32811f8ccb64b0418) )
ROM_END
ROM_START( g_suj2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_suj2.bin", 0x000000, 0x200000, CRC(321bb6bd) SHA1(b13f13ccc1a21dacd295f30c66695bf97bbeff8d) ) // mode 13
ROM_END
ROM_START( g_batr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_batr.bin", 0x000000, 0x100000,  CRC(4a3225c0) SHA1(b173d388485461b9f8b27d299a014d226aef7aa1) )
ROM_END

ROM_START( g_btoa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_btoa.bin", 0x000000, 0x080000, CRC(d10e103a) SHA1(5ef3c29b6bdd04d24552ab200d0530f647afdb08) ) // stuck
ROM_END
ROM_START( g_che2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_che2.bin", 0x000000, 0x100000, CRC(b97b735d) SHA1(3a0a01338320a4326a365447dc8983727738a21e) )
ROM_END
ROM_START( g_che )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_che.bin", 0x000000, 0x100000, CRC(250e3ec3) SHA1(0e1d659d2b6bae32a25365c6592abf64bde94fe2) )
ROM_END
ROM_START( g_chuk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chuk.bin", 0x000000, 0x100000, CRC(f8ac454a) SHA1(f6177b4c9ac48325c53fa26531cdd9bbc673dda3) )
ROM_END
ROM_START( g_clay )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_clay.bin", 0x000000, 0x200000, CRC(b12c1bc1) SHA1(9cd15c84f4ee85ad8f3a512a0fed000724251758) ) // mode 33
ROM_END
ROM_START( g_crue )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crue.bin", 0x000000, 0x080000, CRC(4b195fc0) SHA1(f0c62f1beb4126d1d1d1b634d11fcd81f2723704) )
ROM_END
ROM_START( g_daff )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_daff.bin", 0x000000, 0x200000,  CRC(1fdc66b0) SHA1(67cc61b724d4ccb94dc2b59cc8ea1b0eb9a8cf4e) )
ROM_END
ROM_START( g_davi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_davi.bin", 0x000000, 0x100000, CRC(894686f0) SHA1(735cf7c84869bfa795114f5eff835a74252a4adc) )
ROM_END

ROM_START( g_desd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_desd.bin", 0x000000, 0x100000, CRC(c287343d) SHA1(afc95f7ce66e30abbe10f8d5cd6b791407c7a0bc) )
ROM_END
ROM_START( g_dstr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dstr.bin", 0x000000, 0x100000, CRC(67a9860b) SHA1(d7e7d8c358eb845b84fb08f904cc0b95d0a4053d) )
ROM_END
ROM_START( g_djby )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_djby.bin", 0x000000, 0x080000, CRC(dc9f02db) SHA1(981f0eeffa28210cc55702413305244aaf36d71c) )
ROM_END
ROM_START( g_dtro )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dtro.bin", 0x000000, 0x200000, CRC(11194414) SHA1(aff5e0651fecbc07d718ed3cea225717ce18a7aa) )
ROM_END
ROM_START( g_ecco )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ecco.bin", 0x000000, 0x100000, CRC(45547390) SHA1(2cad130f3118c189d39fd1d46c5c31a5060ce894) )
ROM_END
ROM_START( g_e_sw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_e-sw.bin", 0x000000, 0x200000, CRC(f50be478) SHA1(9400a2ee865f11d68c766059318d6fe69987d89b) )
ROM_END
ROM_START( g_etch )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_etch.bin", 0x000000, 0x300000, CRC(48f1a42e) SHA1(5e978217c10b679a42d7d2966a4ccb77e1715962) )
ROM_END
ROM_START( g_f1ce )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1ce.bin", 0x000000, 0x200000, CRC(74cee0a7) SHA1(c50f66c7a220c58ae29a4610faa9ea6e39a54dfe) )
ROM_END

ROM_START( g_flic )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_flic.bin", 0x000000, 0x020000, CRC(4291c8ab) SHA1(83d8bbf0a9b38c42a0bf492d105cc3abe9644a96) )
ROM_END
ROM_START( g_gax3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gax3.bin", 0x000000, 0x100000, CRC(c7862ea3) SHA1(cd9ecc1df4e01d69af9bebcf45bbd944f1b17f9f) )
ROM_END
ROM_START( g_gran )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gran.bin", 0x000000, 0x080000, CRC(e89d1e66) SHA1(4eb8bbdb9ee8adcefaa202281cb88c19970437f7) )
ROM_END
ROM_START( g_hurr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hurr.bin", 0x000000, 0x200000, CRC(deccc874) SHA1(87ea7e663a0573f02440f9c5661b1df91b4d3ffb) )
ROM_END
ROM_START( g_izzy )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_izzy.bin", 0x000000, 0x200000, CRC(77b416e4) SHA1(0f05e0c333d6ee58a254ee420a70d6020488ac54) )
ROM_END
ROM_START( g_bond )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bond.bin", 0x000000, 0x080000, CRC(291a3e4b) SHA1(e880bfad98d0e6f9b73db63ea85fb0a103b0e1e7) )
ROM_END
ROM_START( g_jp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jp.bin",  0x000000, 0x080000, CRC(d0e7b466) SHA1(b4e1c945c3ccea2e76b296d6694c0931a1ec1310) )
ROM_END
ROM_START( g_jstr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jstr.bin", 0x000000, 0x200000, CRC(a5d29735) SHA1(7a7568e39341b1bb218280ee05c2b37c273317b5) )
ROM_END


ROM_START( g_kidc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kidc.bin", 0x000000, 0x100000, CRC(ce36e6cc) SHA1(28b904000b2863b6760531807760b571f1a5fc1d) )
ROM_END
ROM_START( g_lawn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lawn.bin", 0x000000, 0x100000, CRC(a7cacd59) SHA1(9e19ac92bc06954985dd97a9a7f55ef87e8c7364) )
ROM_END
ROM_START( g_lem2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lem2.bin", 0x000000, 0x200000, CRC(de59a3a3) SHA1(1de84f5c9b25f6af4c2c3e18bb710b9572fc0a10) )
ROM_END
ROM_START( g_lost )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lost.bin", 0x000000, 0x100000, CRC(7ba49edb) SHA1(f00464c111b57c8b23698760cbd377c3c8cfe712) )
ROM_END
ROM_START( g_marv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_marv.bin", 0x000000, 0x100000, CRC(cd7eeeb7) SHA1(29f290c80f992542e73c9ea95190403cb262b6ad) )
ROM_END
ROM_START( g_megp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_megp.bin", 0x000000, 0x040000, CRC(6240f579) SHA1(1822905930f5f3627e9f9109760205e617295fda) )
ROM_END
ROM_START( g_mmpm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmpm.bin", 0x000000, 0x200000, CRC(aa941cbc) SHA1(866d7ccc204e45d188594dde99c2ea836912a136) )
ROM_END
ROM_START( g_mk2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mk2.bin", 0x000000, 0x300000, CRC(a9e013d8) SHA1(af6d2db16f2b76940ff5a9738f1e00c4e7ea485e) )
ROM_END

ROM_START( g_mk3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mk3.bin", 0x000000, 0x400000, CRC(dd638af6) SHA1(55cdcba77f7fcd9994e748524d40c98089344160) )
ROM_END
ROM_START( g_nh98 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nh98.bin", 0x000000, 0x200000, CRC(7b64cd98) SHA1(6771e9b660bde010cf28656cafb70f69249a3591) )
ROM_END
ROM_START( g_otti )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_otti.bin", 0x000000, 0x100000, CRC(41ac8003) SHA1(4bb7cfc41ea7eaaed79ec402ec0d551c1e5c5bb6) )
ROM_END
ROM_START( g_pst3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pst3.bin", 0x000000, 0x0c0000, CRC(c6b42b0f) SHA1(59d4914e652672fd1e453c76b8250d17e8ca154e) )
ROM_END
ROM_START( g_pink )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pink.bin", 0x000000, 0x100000, CRC(b5804771) SHA1(fc63d2a2a723f60f2141bc50bd94acef74ba9ab3) )
ROM_END
ROM_START( g_pitf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pitf.bin", 0x000000, 0x200000, CRC(f917e34f) SHA1(f2067e7d974f03bf922286241119dac7c0ecabff) )
ROM_END
ROM_START( g_prin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_prin.bin", 0x000000, 0x100000, CRC(13c181a4) SHA1(30080c7a8617ba3aaf67587970f32cd846234611) )
ROM_END
ROM_START( g_radr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_radr.bin", 0x000000, 0x100000, CRC(2e6eec7e) SHA1(51019abbb8a4fc246eacd5f553628b74394fab66) )
ROM_END

ROM_START( g_sks2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sks2.bin", 0x000000, 0x340000, CRC(2ac1e7c6) SHA1(6cd0537a3aee0e012bb86d5837ddff9342595004) )
ROM_END
ROM_START( g_s_sa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s-sa.bin", 0x000000, 0x300000, CRC(512ade32) SHA1(563b45254c168aa0de0bba8fadd75a2d1e8e094b) )
ROM_END
ROM_START( g_spl2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spl2.bin", 0x000000, 0x100000, CRC(2d1766e9) SHA1(59ec19ec442989d2738c055b9290661661d13f8f) )
ROM_END
ROM_START( g_stri )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stri.bin", 0x000000, 0x100000, CRC(b9d099a4) SHA1(26fe42d13a01c8789bbad722ebac05b8a829eb37) )
ROM_END
ROM_START( g_turt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_turt.bin", 0x000000, 0x100000,  CRC(679c41de) SHA1(f440dfa689f65e782a150c1686ab90d7e5cc6355) )
ROM_END
ROM_START( g_tick )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tick.bin", 0x000000, 0x200000, CRC(425132f0) SHA1(e0fe77f1d512a753938ce4c5c7c0badb5edfc407) )
ROM_END
ROM_START( g_tomj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tomj.bin", 0x000000, 0x100000, CRC(3044460c) SHA1(09dd23ab18dcaf4e21754992a898504188bd76f2) )
ROM_END
ROM_START( g_umk3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_umk3.bin", 0x000000, 0x400000, CRC(7290770d) SHA1(bf2da4a7ae7aa428b0b316581f65b280dc3ba356) )
ROM_END

ROM_START( g_uded )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_uded.bin", 0x000000, 0x100000, CRC(fb3ca1e6) SHA1(cb9f248bfd19b16ed8a11639a73a6e90fa7ee79e) )
ROM_END
ROM_START( g_ustr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ustr.bin", 0x000000, 0x200000, CRC(cf690a75) SHA1(897a8f6a08d6d7d6e5316f8047532b3e4603e705) )
ROM_END
ROM_START( g_muth )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_muth.bin", 0x000000, 0x200000, CRC(3529180f) SHA1(84e203c5226bc1913a485804e59c6418e939bd3d) )
ROM_END

ROM_START( g_sbea )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sbea.bin", 0x000000, 0x100000,  CRC(bd385c27) SHA1(2f6f5165d3aa40213c8020149d5e4b29f86dfba8) )
ROM_END

ROM_START( g_alex )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alex.bin", 0x000000, 0x040000,  CRC(47dba0ac) SHA1(bbb204c1eb5ca1b6d180c32722955b2c39d2c17b) )
ROM_END
ROM_START( g_btdd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_btdd.bin", 0x000000, 0x100000,  CRC(8239dd17) SHA1(5b79623e90806206e575b3f15499cab823065783) )
ROM_END
ROM_START( g_chak )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chak.bin", 0x000000, 0x100000,  CRC(046a48de) SHA1(7eae088f0b15e4bcd9a6a38849df5a20446be548) )
ROM_END
ROM_START( g_blav )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blav.bin", 0x000000, 0x100000,  CRC(74c65a49) SHA1(11f342ec4be17dcf7a4a6a80649b6b5ff19940a5) )
ROM_END
ROM_START( g_budo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_budo.bin", 0x000000, 0x080000,  CRC(acd9f5fc) SHA1(93bc8242106bc9b2e0a8a974a3f65b559dd2941d) )
ROM_END
ROM_START( g_clif )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_clif.bin", 0x000000, 0x100000,  CRC(9cbf44d3) SHA1(bed365b6b7a1ef96cbdf64b35ad42b54d7d3fb1c) )
ROM_END
ROM_START( g_col3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_col3.bin", 0x000000, 0x080000,  CRC(dc678f6d) SHA1(8e52a5d0adbff3b2a15f32e9299b4ffdf35f5541) )
ROM_END
ROM_START( g_cutt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cutt.bin", 0x000000, 0x200000,  CRC(ebabbc70) SHA1(1b1648e02bb2e915286c35f01476358a8401608f) )
ROM_END
ROM_START( g_dang )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dang.bin", 0x000000, 0x080000,  CRC(a2990031) SHA1(32b2b1de0947fb713787020679efacffd29ec04e) )
ROM_END
ROM_START( g_dar2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dar2.bin", 0x000000, 0x100000,  CRC(25dfe62a) SHA1(93593e4014e13db90fedc1903a402c6f7d885a2f) )
ROM_END
ROM_START( g_duel )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_duel.bin", 0x000000, 0x100000,  CRC(a9804dcc) SHA1(21390cc3036047f3de4a58c5f41f588079b0e56f) ) // size 13
ROM_END
ROM_START( g_demo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_demo.bin", 0x000000, 0x200000,  CRC(5ff71877) SHA1(40d71f6bd6cd44f8003bfaff8c953b0693ec1b01) )
ROM_END


ROM_START( g_dick )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dick.bin", 0x000000, 0x080000,  CRC(ef887533) SHA1(320e527847ebae79d2686c5a500c5100b080ff98) )
ROM_END
ROM_START( g_dora )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dora.bin", 0x000000, 0x080000,  CRC(eeed1130) SHA1(aa92c04afd6b1916fcc6c64285d8dffe0b2b895f) )
ROM_END
ROM_START( g_dn3d )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dn3d.bin", 0x000000, 0x400000,  CRC(6bd2accb) SHA1(a4663f2b96787a92db604a92491fa27e2b5ced9e) )
ROM_END
ROM_START( g_elvi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_elvi.bin", 0x000000, 0x100000, CRC(070a1ceb) SHA1(b53e901725fd6220d8e6d19ef8df42e89e0874db) )
ROM_END
ROM_START( g_faf2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_faf2.bin", 0x000000, 0x300000, CRC(1b1754cb) SHA1(52a269de38ed43ea5c6623906af6b64f01696ffb) )
ROM_END

ROM_START( g_faf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_faf.bin", 0x000000, 0x180000, CRC(98d49170) SHA1(48b6277fc73368d8e96ba407a66f908c3bd87c39) )
ROM_END
ROM_START( g_fzon )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fzon.bin", 0x000000, 0x080000,  CRC(731fa4a1) SHA1(e891e5c89fa13e3f9813d5a45feeed0cb7710acb) )
ROM_END
ROM_START( g_flin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_flin.bin", 0x000000, 0x080000,   CRC(7c982c59) SHA1(5541579ffaee1570da8bdd6b2c20da2e395065b0) )
ROM_END
ROM_START( g_genc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_genc.bin", 0x000000, 0x100000,  CRC(f1ecc4df) SHA1(aea1dfa67b0e583a3d367a67499948998cb92f56) )
ROM_END
ROM_START( g_glos )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_glos.bin", 0x000000, 0x100000,  CRC(131f36a6) SHA1(86af34198a8c67bd92fb03241d14861b2a9e270a) )
ROM_END

ROM_START( g_gbus )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gbus.bin", 0x000000, 0x080000,  CRC(792df93b) SHA1(6fceffee406679c0c8221a8b6cfad447695e99fb) ) // no worky
ROM_END
ROM_START( g_hoso )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hoso.bin", 0x000000, 0x200000,  CRC(dcffa327) SHA1(440835a94ea7c23fc0b0b86564020337a99514f1) )
ROM_END
ROM_START( g_home )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_home.bin", 0x000000, 0x080000,  CRC(aa0d4387) SHA1(c3794ef25d53cb7811dd0df73ba6fadd4cecb116) )
ROM_END
ROM_START( g_hom2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hom2.bin", 0x000000, 0x080000,  CRC(cbf87c14) SHA1(24232a572b7eabc3e0ed5f483042a7085bd77c48) )
ROM_END
ROM_START( g_immo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_immo.bin", 0x000000, 0x100000,  CRC(f653c508) SHA1(71fc189fe2ba5687e9d45b68830baed27194f627) )
ROM_END
ROM_START( g_icd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_icd.bin", 0x000000, 0x100000,  CRC(1f6e574a) SHA1(ca0115dd843e072815c4be86a7a491b26e3c4762) )
ROM_END
ROM_START( g_hulk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hulk.bin", 0x000000, 0x200000,  CRC(84a5a2dc) SHA1(41247e3166ec42b7cada33615da49e53d48fd809) ) // invalid vsize
ROM_END
ROM_START( g_junc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_junc.bin", 0x000000, 0x080000,  CRC(94cdce8d) SHA1(7981e5764458f5230184c7b2cf77469e1ed34270) )
ROM_END

ROM_START( g_jpar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jpar.bin", 0x000000, 0x200000,  CRC(7b31deef) SHA1(e7a1f49d362b5c7e13d9c7942d4a83fe003cfbd2) )
ROM_END
ROM_START( g_ligh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ligh.bin", 0x000000, 0x200000,  CRC(beb715dc) SHA1(df58fbcbede4b9659740b5505641d4cc7dd1b7f8) )
ROM_END
ROM_START( g_fran )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fran.bin", 0x000000, 0x200000,  CRC(48993dc3) SHA1(2dd34478495a2988fe5839ef7281499f08bf7294) )
ROM_END
ROM_START( g_mfpl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mfpl.bin", 0x000000, 0x100000,  CRC(38174f40) SHA1(cf3086b664312d03c749f5439f1bdc6785f035cc) )
ROM_END
ROM_START( g_onsl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_onsl.bin", 0x000000, 0x080000,   CRC(9f19d6df) SHA1(dc542ddfa878f2aed3a7fcedc4b0f8d503eb5d70) )
ROM_END
ROM_START( g_peng )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_peng.bin", 0x000000, 0x100000, CRC(d1e2324b) SHA1(c88c30d9e1fb6fb3a8aadde047158a3683bb6b1a) )
ROM_END

ROM_START( g_rain )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rain.bin", 0x000000, 0x080000,  CRC(c74dcb35) SHA1(e35fcc88e37d1691cd2572967e1ae193fcd303eb) )
ROM_END
ROM_START( g_rens )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rens.bin", 0x000000, 0x100000,  CRC(d9503ba5) SHA1(29788359cc3311107e82f275868e908314d3d426) )
ROM_END
ROM_START( g_robt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robt.bin", 0x000000, 0x200000,  CRC(bbad77a4) SHA1(b67b708006145406f2a8dacb237517ededec359a) )
ROM_END
ROM_START( g_sitd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sitd.bin", 0x000000, 0x100000,  CRC(4d2785bc) SHA1(4e10c90199d6edd2030a4ba1c42c7c166bf309ec) )
ROM_END
ROM_START( g_btnm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_btnm.bin", 0x000000, 0x100000,  CRC(24d7507c) SHA1(fb95b7fdf12dcf62883dabf65d2bf8ffa83786fc) )
ROM_END
ROM_START( g_smf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smf.bin", 0x000000, 0x100000,  CRC(88b30eff) SHA1(0da7e621e05dc9160122d728e1fca645ff11e670) )
ROM_END
ROM_START( g_smf2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smf2.bin", 0x000000, 0x100000,  CRC(b28bdd69) SHA1(b4368369e1d5b9a60bc565fe09a9c5fff6b79fd4) )
ROM_END
ROM_START( g_snkn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_snkn.bin", 0x000000, 0x200000,  CRC(0658f691) SHA1(88d6499d874dcb5721ff58d76fe1b9af811192e3) )
ROM_END

ROM_START( g_s_mc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s_mc.bin", 0x000000, 0x200000,  CRC(8fa0b6e6) SHA1(43624536cbebd65232abe4af042c0fa4b0d9e3b7) )
ROM_END
ROM_START( g_sgat )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sgat.bin", 0x000000, 0x200000, CRC(e587069e) SHA1(d0843442059c89b11db02615670632fda2b2ee85) )
ROM_END
ROM_START( g_tfh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tfh.bin", 0x000000, 0x100000,  CRC(c8bb0257) SHA1(1d36bd69e356f276c582fee247af2f71af1f3bf4) )
ROM_END
ROM_START( g_tutf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tutf.bin", 0x000000, 0x200000,  CRC(95b5484d) SHA1(1a27be1e7f8f47eb539b874eaa48586fe2dab9c0) )
ROM_END
ROM_START( g_tf3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tf3.bin", 0x000000, 0x080000,  CRC(1b3f399a) SHA1(9eadee76eb0509d5a0f16372fc9eac7a883e5f2d) )
ROM_END
ROM_START( g_ttaa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ttaa.bin", 0x000000, 0x100000,  CRC(2f9faa1d) SHA1(d64736a69fca430fc6a84a60335add0c765feb71) )
ROM_END
ROM_START( g_tter )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tter.bin", 0x000000, 0x040000,  CRC(aabb349f) SHA1(d0dc2acdc17a1e1da25828f7d07d4ba9e3c9bd78) )
ROM_END
ROM_START( g_unis )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_unis.bin", 0x000000, 0x100000,  CRC(352ebd49) SHA1(9ac1416641ba0e6632369ffe69599b08fc3c225f) )
ROM_END

ROM_START( g_vrtr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vrtr.bin", 0x000000, 0x200000,  CRC(2f35516e) SHA1(e4c5271ef2034532841fde323f59d728365d7f6a) )
ROM_END
ROM_START( g_wayn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wayn.bin", 0x000000, 0x100000,  CRC(d2cf6ebe) SHA1(2b55be87cd53514b261828fd264108fbad1312cd) )
ROM_END
ROM_START( g_weap )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_weap.bin", 0x000000, 0x300000,  CRC(b9895365) SHA1(04128e3859d6eca6e044f456f8b0d06b63e3fb0c) )
ROM_END
ROM_START( g_wmar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wmar.bin", 0x000000, 0x400000,  CRC(a5d023f9) SHA1(e49ad2f9119b0788bbbb7258908d605c032989b4) )
ROM_END
ROM_START( g_xme2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xme2.bin", 0x000000, 0x200000,  CRC(710bc628) SHA1(61409e6cf6065ab67d8952b891d8edcf47777193) )
ROM_END
ROM_START( g_yogi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yogi.bin", 0x000000, 0x100000,  CRC(204f97d8) SHA1(9f235fd4ac2612fb398ecbe423f40c901be8f564) )
ROM_END

ROM_START( g_soff )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soff.bin", 0x000000, 0x080000, CRC(8f2fdada) SHA1(89f264afba7aa8764b301d46cc8c51f74c23919e) )
ROM_END
ROM_START( g_ddr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ddr.bin", 0x000000, 0x080000,  CRC(054f5d53) SHA1(f054fb2c16a78ce5f5ce20a36bf0f2634f169969) )
ROM_END
ROM_START( g_ddr2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ddr2.bin", 0x000000, 0x080000,  CRC(a8bfdbd6) SHA1(68dc151ada307ed0ed34f98873e0be5f65f1b573) )
ROM_END
ROM_START( g_ddr3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ddr3.bin", 0x000000, 0x100000,  CRC(b36ab75c) SHA1(663dfeebd21409942bcc446633b9b9f0dd238aa8) )
ROM_END
ROM_START( g_ddrv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ddrv.bin", 0x000000, 0x300000,  CRC(27e59e35) SHA1(53402b7c43cd20bf4cbaf40d1ba5062e3823f4bd) )
ROM_END

ROM_START( g_mb8p )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mb8p.bin", 0x000000, 0x100000,  CRC(d41c0d81) SHA1(6cc338e314533d7e4724715ef0d156f5c3f873f3) )
ROM_END

ROM_START( g_fido )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fido.bin", 0x000000, 0x100000,  CRC(c6d4a240) SHA1(fa69728de541321a5d55fd2c11ce8222d7daac45) )
ROM_END


ROM_START( g_tnnb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tnnb.bin", 0x000000, 0x100000,  CRC(c83ffa1b) SHA1(f76acb6d5da07377685d42daf1ce4ca53be5d6b9) )
ROM_END

ROM_START( g_tnno )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tnno.bin", 0x000000, 0x200000,  CRC(5c523c0b) SHA1(0ca72f28e88675066c466246977143599240b09f) )
ROM_END

ROM_START( g_sscc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sscc.bin", 0x000000, 0x100000,  CRC(0a4f48c3) SHA1(d5ef2b50cf1a22e07401f6a14c7946df66d8b605) )
ROM_END


ROM_START( g_xper )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xper.bin", 0x000000, 0x400000,  CRC(57e8abfd) SHA1(be1df5a63f4363d11d340db9a37ba54db5f5ea38) )
ROM_END


ROM_START( g_batj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_batj.bin", 0x000000, 0x100000,  CRC(caa044a1) SHA1(e780c949571e427cf1444e1e35efe33fc9500c81) )
ROM_END
ROM_START( g_bat )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bat.bin", 0x000000, 0x080000,  CRC(017410ae) SHA1(ffa7ddd8ecf436daecf3628d52dfae6b522ed829) )
ROM_END
ROM_START( g_bsqu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bsqu.bin", 0x000000, 0x080000,  CRC(0feaa8bf) SHA1(f003f7af0f7edccc317c944b88e57f4c9b66935a) )
ROM_END
ROM_START( g_crkd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crkd.bin", 0x000000, 0x080000,  CRC(b9ce9051) SHA1(949cd961a99e9c35388cf6a4db5e866102ed27a2) )
ROM_END
ROM_START( g_dune )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dune.bin", 0x000000, 0x100000,  CRC(4dea40ba) SHA1(0f7c1c130cb39abc97f57545933e1ef6c481783d) )
ROM_END
ROM_START( g_earn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_earn.bin", 0x000000, 0x100000,  CRC(a243816d) SHA1(cb5a2a928e2c2016f915e07e1d148672563183f0) )
ROM_END
ROM_START( g_f117 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f117.bin", 0x000000, 0x200000,  CRC(1bf67a07) SHA1(906c7ac3ea3c460e9093d5082c68e35f1bb1bb0e) )
ROM_END
ROM_START( g_frog )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_frog.bin", 0x000000, 0x080000,  CRC(ea2e48c0) SHA1(c0ccfec43ea859ab1e83293a38cfb302c0191719) ) // no pal!
ROM_END
ROM_START( g_gloc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gloc.bin", 0x000000, 0x100000,  CRC(f2af886e) SHA1(81bb08c4080ca9a8af65597d1c7b11ce902c8d9e) )
ROM_END


ROM_START( g_gax2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gax2.bin", 0x000000, 0x080000,  CRC(725e0a18) SHA1(31f12a21af018cdf88b3f2170af5389b84fba7e7) )
ROM_END
ROM_START( g_gshi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gshi.bin", 0x000000, 0x100000,  CRC(da1440c9) SHA1(3820eac00b888b17c02f271fab5fd08af99d5ef9) )
ROM_END
ROM_START( g_herz )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_herz.bin", 0x000000, 0x080000,  CRC(a605b65b) SHA1(8f7262102c2b2334f0bc88ee6fd6b08797919176) )
ROM_END
ROM_START( g_last )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_last.bin", 0x000000, 0x100000,  CRC(15357dde) SHA1(8efa876894c8bfb0ea457e86173eb5b233861cd0) )
ROM_END
ROM_START( g_mush )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mush.bin", 0x000000, 0x080000,  CRC(58a7f7b4) SHA1(821eea5d357f26710a4e2430a2f349a80df5f2f6) )
ROM_END
ROM_START( g_mjmw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mjmw.bin", 0x000000, 0x080000,  CRC(11ce1f9e) SHA1(70d9b760c87196af364492512104fa18c9d69cce) )
ROM_END
ROM_START( g_mmpr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmpr.bin", 0x000000, 0x200000,  CRC(715158a9) SHA1(dbe0c63c9e659255b091760889787001e85016a9) )
ROM_END
ROM_START( g_mpac )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mpac.bin", 0x000000, 0x020000,  CRC(af041be6) SHA1(29fff97e19a00904846ad99baf6b9037b28df15f) )
ROM_END
ROM_START( g_ncir )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ncir.bin", 0x000000, 0x200000,  CRC(06da3217) SHA1(3c80ff5ba54abe4702a3bb7d812571d3dc50c00f) )
ROM_END

ROM_START( g_ogol )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ogol.bin", 0x000000, 0x080000,  CRC(339594b4) SHA1(f9febd976d98545dee35c10b69755908d6929fd4) )
ROM_END

ROM_START( g_pacm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pacm.bin", 0x000000, 0x040000,  CRC(74bba09b) SHA1(e21546f653607d95f2747305703ddd6bf118f90a) )
ROM_END
ROM_START( g_pdri )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pdri.bin", 0x000000, 0x100000,  CRC(8c00ad61) SHA1(6ffaaac637fb41a2f94912c382385a9cb17639a9) )
ROM_END
ROM_START( g_race )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_race.bin", 0x000000, 0x080000,  CRC(d737cf3d) SHA1(7961b37d364c8296d96f08a79577985113f72ba5) )
ROM_END
ROM_START( g_sbe2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sbe2.bin", 0x000000, 0x100000,  CRC(2dede3db) SHA1(260a497348313c22f2d1a626150b0915990720c2) ) // raster
ROM_END
ROM_START( g_shaq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shaq.bin", 0x000000, 0x300000,  CRC(499955f2) SHA1(1ad8cd54f8fe474da01d1baba98491411c4cedaf) )
ROM_END
ROM_START( g_bart )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bart.bin", 0x000000, 0x080000,  CRC(db70e8ca) SHA1(2d42143c83ec3b4167860520ee0a9030ef563333) )
ROM_END
ROM_START( g_sold )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sold.bin", 0x000000, 0x100000,  CRC(a77e4e9f) SHA1(c10545e20a147d4fbf228db4a2b6e309799708c3) ) // gitch
ROM_END
ROM_START( g_si91 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_si91.bin", 0x000000, 0x040000,  CRC(bb83b528) SHA1(d8046f1c703ea7c2d7f9f3f08702db7706f56cb4) )
ROM_END
ROM_START( g_s_ar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s_ar.bin", 0x000000, 0x100000,  CRC(4a4414ea) SHA1(978dabcc7d098edebc9d3f2fef04f27fd6aeab19) )
ROM_END


ROM_START( g_ssri )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ssri.bin", 0x000000, 0x080000,  CRC(ac30c297) SHA1(b40ea5b00f477d7b7448447f15b4c571f5e8ff0d) )
ROM_END
ROM_START( g_sstv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sstv.bin", 0x000000, 0x080000,  CRC(f22412b6) SHA1(0459f7c61f152fa0afa98d96ef9fbe4964641f34) )
ROM_END
ROM_START( g_targ )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_targ.bin", 0x000000, 0x080000,  CRC(cddf62d3) SHA1(e19f7a02f140882e0364f11cd096aec712e56f83) )
ROM_END
ROM_START( g_ter2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ter2.bin", 0x000000, 0x100000,  CRC(2f75e896) SHA1(6144fbb941c1bf0df285f6d13906432c23af2ba6) )
ROM_END
ROM_START( g_tf2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tf2.bin", 0x000000, 0x080000,  CRC(9b1561b3) SHA1(b81e7ebc4ceb6c1ae2975d27e0a78ba1e8546b5f) )
ROM_END
ROM_START( g_toys )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_toys.bin", 0x000000, 0x100000,  CRC(cbc9951b) SHA1(debc3a571c2c08a731758113550c040dfcda4782) )
ROM_END
ROM_START( g_true )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_true.bin", 0x000000, 0x200000,  CRC(18c09468) SHA1(d39174bed46ede85531b86df7ba49123ce2f8411) )
ROM_END

ROM_START( g_uwnh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_uwnh.bin", 0x000000, 0x200000,  CRC(ead69824) SHA1(9fd375cd212a132db24c40a8977c50d0f7b81524) )
ROM_END
ROM_START( g_view )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_view.bin", 0x000000, 0x200000,  CRC(59c71866) SHA1(f25f770464448da4e49eab3832100ba480c9844a) )
ROM_END
ROM_START( g_wagh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wagh.bin", 0x000000, 0x080000,  CRC(d68e9c00) SHA1(8c648318fdd54e2c75c44429a72c1c1a846e67cb) )
ROM_END
ROM_START( g_worm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_worm.bin", 0x000000, 0x200000,  CRC(b9a8b299) SHA1(1a15447a4a791c02b6ad0a609f788d39fe6c3aa6) )
ROM_END
ROM_START( g_xen2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xen2.bin", 0x000000, 0x080000,  CRC(59abe7f9) SHA1(055b554f7d61ed671eddc385bce950b2baa32249) )
ROM_END
ROM_START( g_zany )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zany.bin", 0x000000, 0x080000,  CRC(ed5d12ea) SHA1(4f9bea2d8f489bfbc963718a8dca212e033fb5a2) )
ROM_END
ROM_START( g_zoop )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zoop.bin", 0x000000, 0x080000,  CRC(a899befa) SHA1(e6669f16902caf35337de60027ed2013deed0d40) )
ROM_END

ROM_START( g_arta )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arta.bin", 0x000000, 0x020000,  CRC(f1b72cdd) SHA1(2c57e38592a206a1847e9e202341595832798587) )
ROM_END
ROM_START( g_asl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_asl.bin", 0x000000, 0x080000,  CRC(81a2c800) SHA1(332886d9fe7823092fed6530781269b60f24d792) )
ROM_END
ROM_START( g_arca )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arca.bin", 0x000000, 0x080000,  CRC(8aed2090) SHA1(ec29aec7848dbcea6678adb4b31deba0a6ecf1e2) )
ROM_END
ROM_START( g_aahh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aahh.bin", 0x000000, 0x200000,  CRC(065f6021) SHA1(fcc4b27aaaea0439fb256f40fb79c823d90a9d88) )
ROM_END
ROM_START( g_aero )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aero.bin", 0x000000, 0x100000,  CRC(cfaa9bce) SHA1(104ad3c6f5aaba08270faf427991d6a256c6e67c) )
ROM_END

ROM_START( g_awsp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_awsp.bin", 0x000000, 0x080000,  CRC(707017e5) SHA1(59b47ce071b38eb740a916c42b256af77e2e7a61) )
ROM_END
ROM_START( g_bttf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bttf.bin", 0x000000, 0x080000,  CRC(66a388c3) SHA1(a704a1538fb4392e4631e96dce11eeff99d9b04a) )
ROM_END
ROM_START( g_bar3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bar3.bin", 0x000000, 0x300000,  CRC(5d09236f) SHA1(b9b2b4a98a9d8f4c49aa1e5395e2279339517fdb) )
ROM_END

ROM_START( g_suj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_suj.bin", 0x000000, 0x100000,  CRC(63fbf497) SHA1(fc60a682412b4f7f851c5eb7f6ae68fcee3d2dd1) )
ROM_END
ROM_START( g_beav )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_beav.bin", 0x000000, 0x200000,  CRC(f5d7b948) SHA1(9abfbf8c5d07a11f090a151c2801caee251f1599) )
ROM_END
ROM_START( g_botb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_botb.bin", 0x000000, 0x100000,  CRC(c3d6a5d4) SHA1(1fafc2d289215d0f835fa6c74a8bf68f8d36bcc8) )
ROM_END

ROM_START( g_sail )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sail.bin", 0x000000, 0x200000,  CRC(5e246938) SHA1(7565b0b19fb830ded5e90399f552c14c2aacdeb8) )
ROM_END
ROM_START( g_bl96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bl96.bin", 0x000000, 0x100000,  CRC(fa3024af) SHA1(0b08788a0f8214c5d07b8e2293f0b954dd05bef5) )
ROM_END
ROM_START( g_buck )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_buck.bin", 0x000000, 0x100000,  CRC(44e3bfff) SHA1(89c39f00745f2a8798fe985ad8ce28411b977f9e) )
ROM_END
ROM_START( g_curs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_curs.bin", 0x000000, 0x080000, CRC(a4fbf9a9) SHA1(978780d9575022450d415591f32e1118c7fac5ad) )
ROM_END
ROM_START( g_hb95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hb95.bin", 0x000000, 0x300000, CRC(ed10bc9e) SHA1(6a63fba59add9ba8e1845cbfcf4722833893113f) )
ROM_END
ROM_START( g_ws98 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ws98.bin", 0x000000, 0x300000, CRC(05b1ab53) SHA1(0881077fd253d19d43ad45de6089d66e75d856b3) )
ROM_END

ROM_START( g_dark )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dark.bin", 0x000000, 0x080000,  CRC(0464aca4) SHA1(23e1ad9822338362113e55087d60fb9a1674bf8a) )
ROM_END
ROM_START( g_4081 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_4081.bin", 0x000000, 0x080000, CRC(7a33b0cb) SHA1(e9decc48451aba62d949a7710e466e1d041a2210) )
ROM_END
ROM_START( g_dmov )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dmov.bin", 0x000000, 0x100000, CRC(35cbd237) SHA1(8733d179292d4dc5c3513459539d96484b6d018f) )
ROM_END
ROM_START( g_drs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drs.bin", 0x000000, 0x200000, CRC(982242d3) SHA1(3534d17801bd5756b10a7f8d7d95f3a8d9b74844) )
ROM_END
ROM_START( g_hire )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hire.bin", 0x000000, 0x100000, CRC(39351146) SHA1(d006efbf1d811e018271745925fe00ca6d93f24f) )
ROM_END
ROM_START( g_dblc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dblc.bin", 0x000000, 0x040000, CRC(d98c623c) SHA1(e19905cfcf74185e56fa94ae292f78451c8f4e2e) )
ROM_END
ROM_START( g_elim )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_elim.bin", 0x000000, 0x100000,  CRC(48467542) SHA1(53b418bde065011e161e703dd7c175aa48a04fe5) )
ROM_END
ROM_START( g_eswa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_eswa.bin", 0x000000, 0x080000, CRC(e72f8a36) SHA1(5cb96061bd2b00c82f8d6b46ab9802e2b1820c86) )
ROM_END

ROM_START( g_ecs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ecs.bin", 0x000000, 0x080000, CRC(6a5cf104) SHA1(03e1a6c7fb8003e196c0f0bf787276a14daa313e) )
ROM_END
ROM_START( g_must )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_must.bin", 0x000000, 0x080000, CRC(eb7e36c3) SHA1(6daa07738b8f62659f2a3be01d3adc8557b879c5) )
ROM_END
ROM_START( g_gax )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gax.bin", 0x000000, 0x080000, CRC(665d7df9) SHA1(2ce17105ca916fbbe3ac9ae3a2086e66b07996dd) )
ROM_END
ROM_START( g_goof )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_goof.bin", 0x000000, 0x100000,  CRC(4e1cc833) SHA1(caaeebc269b3b68e2a279864c44d518976d67d8b) )
ROM_END
ROM_START( g_grin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_grin.bin", 0x000000, 0x100000, CRC(7e6bef15) SHA1(8f93445e2d0b1798f680dda26a3d31f8aee88f01) )
ROM_END
ROM_START( g_dodg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dodg.bin", 0x000000, 0x080000,  CRC(630f07c6) SHA1(ebdf20fd8aaeb3c7ec97302089b3330265118cf0) )
ROM_END
ROM_START( g_indl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_indl.bin", 0x000000, 0x100000, CRC(3599a3fd) SHA1(82758a8a47c4f1f0e990bd50b773b2c4300f616e) )
ROM_END
ROM_START( g_jcte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jcte.bin", 0x000000, 0x080000, CRC(ab2abc8e) SHA1(8d72ea31c87b1a229098407e9c59a46e65f996a2) )
ROM_END


ROM_START( g_jdre )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jdre.bin", 0x000000, 0x200000, CRC(ea342ed8) SHA1(6553b4eabcba9d2823cbffd08554408a9f1067c9) )
ROM_END
ROM_START( g_jltf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jltf.bin", 0x000000, 0x300000, CRC(2a60ebe9) SHA1(1be166689726b98fc5924028e736fc8007f958ef) )
ROM_END

ROM_START( g_taru )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_taru.bin", 0x000000, 0x080000, CRC(f11060a5) SHA1(208f1d83bb410f9cc5e5308942b83c0e84f64294) )
ROM_END
ROM_START( g_osom )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_osom.bin", 0x000000, 0x040000, CRC(2453350c) SHA1(bf82cb1c4d2144bd0e0d45629f346988cc4cc5b2) )
ROM_END
ROM_START( g_puy2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_puy2.bin", 0x000000, 0x200000, CRC(25b7b2aa) SHA1(d9c0edaadf66f215565a9dd4178313581fec811c) )
ROM_END
ROM_START( g_scob )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_scob.bin", 0x000000, 0x200000, CRC(7bb9dd9b) SHA1(ccc9542d964de5bdc1e9101620cae40dd98e7127) )
ROM_END
ROM_START( g_srun )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_srun.bin", 0x000000, 0x200000, CRC(fbb92909) SHA1(a06a281d39e845bff446a541b2ff48e1d93143c2) )
ROM_END

ROM_START( g_snow )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_snow.bin", 0x000000, 0x100000, CRC(11b56228) SHA1(27caf554f48d2e3c9c6745f32dbff231eca66744) )
ROM_END
ROM_START( g_spb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spb2.bin", 0x000000, 0x080000, CRC(9fc340a7) SHA1(4598f1714fd05e74ab758c09263f7948c8ca1883) )
ROM_END
ROM_START( g_s_as )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s_as.bin", 0x000000, 0x200000, CRC(11b5b590) SHA1(a38d5ad7d503999b7fea3ebf59f3dda9d667758b) )
ROM_END
ROM_START( g_s_kp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s_kp.bin", 0x000000, 0x080000, CRC(70ab775f) SHA1(250f7a7301a028450eef2f2a9dcec91f99ecccbd) )
ROM_END
ROM_START( g_semp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_semp.bin", 0x000000, 0x100000, CRC(d0e7a0b6) SHA1(45c0b9d85b4a8053c3f3432828626dae47022634) )
ROM_END
ROM_START( g_kout )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kout.bin", 0x000000, 0x100000, CRC(755d0b8a) SHA1(7ff88050ae9b2f12afe80432781ba25aa3a15e1c) )
ROM_END
ROM_START( g_sair )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sair.bin", 0x000000, 0x080000, CRC(fa451982) SHA1(d24a3d23c9f12eebfbd233fdaab91c4acc362962) )
ROM_END
ROM_START( g_sbat )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sbat.bin", 0x000000, 0x080000, CRC(99ca1bfb) SHA1(fd1cdde1448f20388ccb3f66498867573f8c6fc2) )
ROM_END

ROM_START( g_tprk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tprk.bin", 0x000000, 0x200000, CRC(289da2c5) SHA1(03c6504b5d797f10c7c361735d801902e4b00981) )
ROM_END
ROM_START( g_tint )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tint.bin", 0x000000, 0x200000, CRC(4243caf3) SHA1(54fb11f601be37418b5bba3e0762d8b87068177a) )
ROM_END
ROM_START( g_tsht )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsht.bin", 0x000000, 0x080000, CRC(becfc39b) SHA1(3fc9ffa49ece5e9cbba1f2a5ba1dfd068b86c65d) )
ROM_END
ROM_START( g_gomo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gomo.bin", 0x000000, 0x100000, CRC(c511e8d2) SHA1(4e1d77cc1bf081e42abfd1c489fbd0073f0236af) )
ROM_END
ROM_START( g_usoc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_usoc.bin", 0x000000, 0x100000, CRC(83db6e58) SHA1(91781d0561f84de0d304221bbc26f4035f62010f) )
ROM_END
ROM_START( g_wfrr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wfrr.bin", 0x000000, 0x200000, CRC(b69dc53e) SHA1(34e85015b8681ce15ad4777a60c81297ccf718b1) )
ROM_END
ROM_START( g_wfra )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wfra.bin", 0x000000, 0x300000, CRC(4ef5d411) SHA1(94be19287c64ab8e164ab1105085cb3548c6b179 ) )
ROM_END
ROM_START( g_wolf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wolf.bin", 0x000000, 0x100000, CRC(eb5b1cbf) SHA1(000d45769e7e9f72f665e160910ba9fb79eec43f) )
ROM_END

ROM_START( g_zoom )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zoom.bin", 0x000000, 0x040000, CRC(724d6965) SHA1(05efc300b3f68496200cb73a2a462a97f930a011) )
ROM_END
ROM_START( g_resq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_resq.bin", 0x000000, 0x100000, CRC(558e35e0) SHA1(f572443bd1e0cf956c87e6cd925412cb0f35045f) )
ROM_END
ROM_START( g_jely )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jely.bin", 0x000000, 0x100000, CRC(7cfadc16) SHA1(75be8d5b305e669848b4a5a48cdcfa43b951dc20) )
ROM_END
ROM_START( g_itch )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_itch.bin", 0x000000, 0x100000, CRC(81b7725d) SHA1(860d9bd7501d59da8304ab284b385afa4def13a0) )
ROM_END
ROM_START( g_2sam )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2sam.bin", 0x000000, 0x100000, CRC(78e92143) SHA1(72bfe0d4c56d9600a39fae00399fef157eb1fe6f) )
ROM_END

ROM_START( g_fcr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fcr.bin", 0x000000, 0x100000, CRC(42e27845) SHA1(779aba11bf77ca59e6ae981854805bf3fabe4b0e) )
ROM_END



ROM_START( g_pre2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pre2.bin", 0x000000, 0x100000, CRC(bdba113e) SHA1(0d482bae2922c81c8bc7500a62c396b038978114) )
ROM_END
ROM_START( g_pst2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pst2.bin", 0x000000, 0x0c0000, CRC(904fa047) SHA1(0711080e968490a6b8c5fafbb9db3e62ba597231) )
ROM_END
ROM_START( g_pst4 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pst4.bin", 0x000000, 0x300000, CRC(fe236442) SHA1(bc7ff6d6a8408f38562bc610f24645cad6c42629) )
ROM_END
ROM_START( g_pga3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pga3.bin", 0x000000, 0x200000, CRC(aeb3f65f) SHA1(702707efcbfe229f6e190f2b6c71b6f53ae9ec36) )
ROM_END
ROM_START( g_pga2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pga2.bin", 0x000000, 0x100000, CRC(e82b8606) SHA1(12d5236a4ff23c5b1e4f452b3abd3d48e6e55314) )
ROM_END
ROM_START( g_pga )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pga.bin", 0x000000, 0x080000, CRC(c1f0b4e1) SHA1(1b173a1b674a6d5bdcd539c4fe8fe984586c3a8a) )
ROM_END
ROM_START( g_pg96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pg96.bin", 0x000000, 0x200000, CRC(9698bbde) SHA1(abc2a8d773724cd8fb1aeae483f5ca72f47e77fa) )
ROM_END
ROM_START( g_pm97 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pm97.bin", 0x000000, 0x100000, CRC(fccbf69b) SHA1(e2df3b48170e1a7bde46af2adbf939803e267e13) )
ROM_END
ROM_START( g_pman )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pman.bin", 0x000000, 0x100000, CRC(303b889f) SHA1(2916e5ef628e077cde87be873e0ea2507ef5c844) )
ROM_END

ROM_START( g_prim )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_prim.bin", 0x000000, 0x300000, CRC(2884c6d1) SHA1(18fc82ae49948128f2347b8b6d7174c8a0c1bf5f) )
ROM_END
ROM_START( g_pwbl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pwbl.bin", 0x000000, 0x080000, CRC(7adf232f) SHA1(217d64da0b09b88dfa3fd91b7fa081ec7a8666e0) )
ROM_END
ROM_START( g_puni )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_puni.bin", 0x000000, 0x200000, CRC(695cd8b8) SHA1(4ef2675e728903925a7b865daa75ed66bbe24829) )
ROM_END
ROM_START( g_ram3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ram3.bin", 0x000000, 0x040000, CRC(4d47a647) SHA1(e337e618653830f84c77789e81724c5fc69888be) )
ROM_END
ROM_START( g_revx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_revx.bin", 0x000000, 0x400000, CRC(5fb0c5d4) SHA1(e1c8011ff878c7d99236ddd4008dd284e1c28333) )
ROM_END
ROM_START( g_rbls )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rbls.bin", 0x000000, 0x080000, CRC(ec6cd5f0) SHA1(927cdb93e688152f2ff9a89a1b22cd6517ada652) )
ROM_END
ROM_START( g_rrr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rrr.bin", 0x000000, 0x100000, CRC(6abab577) SHA1(9793572b6d64c85c1add0721c3be388ac18777d6) )
ROM_END
ROM_START( g_rth2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rth2.bin", 0x000000, 0x100000, CRC(3ace429b) SHA1(1b451389910e6b353c713f56137863ce2aff109d) )
ROM_END
ROM_START( g_rth3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rth3.bin", 0x000000, 0x180000, CRC(64fb13aa) SHA1(34c2b5df456a1f8ff35963ca737372e221e89ef6) )
ROM_END

ROM_START( g_snsm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_snsm.bin", 0x000000, 0x400000, CRC(2fb4eaba) SHA1(7252006bf41e41b92c6261c6722766a1352a6dca) ) // rast
ROM_END
ROM_START( g_sdan )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sdan.bin", 0x000000, 0x080000, CRC(ebe9ad10) SHA1(c5f096b08470a564a737140b71247748608c32b6) )
ROM_END
ROM_START( g_s2de )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s2de.bin", 0x000000, 0x100000,  CRC(ebe9e840) SHA1(7d6dc54b8943880a5fd26af10364eb943e9724c2) )
ROM_END
ROM_START( g_shf1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shf1.bin", 0x000000, 0x180000, CRC(e0594abe) SHA1(7cbb3ed31c982750d70a273b9561a9e1b2c04eea) )
ROM_END
ROM_START( g_shf2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shf2.bin", 0x000000, 0x200000, CRC(4815e075) SHA1(22defc2e8e6c1dbb20421b906796538725b3d893) )
ROM_END
ROM_START( g_sole )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sole.bin", 0x000000, 0x200000, CRC(a30ebdb1) SHA1(110a61671b83fe17fba768ab85b535ca1cc6d7ea) )
ROM_END
ROM_START( g_sspa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sspa.bin", 0x000000, 0x100000, CRC(e9960371) SHA1(8f372e3552e309d3462adeb700242b251f59def1) )
ROM_END
ROM_START( g_sf2c )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sf2c.bin", 0x000000, 0x300000, CRC(13fe08a1) SHA1(a5aad1d108046d9388e33247610dafb4c6516e0b) )
ROM_END
ROM_START( g_sbt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sbt.bin", 0x000000, 0x080000, CRC(b0b5e3c9) SHA1(4ce9aaaa9d3f98e1747af12ad488b6bdbde1afb4) )
ROM_END

ROM_START( g_2020 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2020.bin", 0x000000, 0x200000,  CRC(c17acee5) SHA1(a6ea6dcc33d60cf4d3be75b1cc867699811f8b3a) ) // long pause
ROM_END
ROM_START( g_f22i )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f22i.bin", 0x000000, 0x0c0000, CRC(dd19b2b3) SHA1(32a479ed7571c5f99b2181829afc517370e3051a) )
ROM_END
ROM_START( g_faer )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_faer.bin", 0x000000, 0x080000, CRC(963f4969) SHA1(5d9a448d741743c7e75e5fe462e0da0ea3d18a13) )
ROM_END
ROM_START( g_gain )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gain.bin", 0x000000, 0x080000, CRC(83e7b8ae) SHA1(3cc501086f794ac663aad14d5c5a75b648041151) )
ROM_END
ROM_START( g_0tol )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_0tol.bin", 0x000000, 0x200000, CRC(23f603f5) SHA1(d6d9733a619ba6be0dd76591d8dec621e4fdc17e) )
ROM_END
ROM_START( g_zwin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zwin.bin", 0x000000, 0x100000, CRC(89b744a3) SHA1(98335b97c5e21f7f8c5436427621836660b91075) )
ROM_END
ROM_START( g_yidy )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yidy.bin", 0x000000, 0x100000, CRC(4e384ef0) SHA1(7ffd31a8fd0f2111ef3dcce1b8493e6ea6e24deb) )
ROM_END
ROM_START( g_wher )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wher.bin", 0x000000, 0x200000, CRC(0f4d22ec) SHA1(5a563f441c3013ac4b3f9e08d5e5a6e05efc5de0) )
ROM_END
ROM_START( g_whip )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_whip.bin", 0x000000, 0x080000, CRC(7eb6b86b) SHA1(f05560b088565c1c32c0039dc7bf58caa5310680) )
ROM_END

ROM_START( g_ward )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ward.bin", 0x000000, 0x080000, CRC(1e369ae2) SHA1(23bd6421f0e3710350e12f9322e75160a699ace8) )
ROM_END
ROM_START( g_vbar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vbar.bin", 0x000000, 0x200000, CRC(8db9f378) SHA1(5675fdafb27bb3e23f7d9bf4e74d313e42b26c65) )
ROM_END
ROM_START( g_2cd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2cd.bin", 0x000000, 0x100000, CRC(721b5744) SHA1(e8c2faa0de6d370a889426b38538e75c264c4456) )
ROM_END
ROM_START( g_twih )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_twih.bin", 0x000000, 0x080000, CRC(a2ec8c67) SHA1(f884f2a41dd50f4c1a17c26da7f2d31093bb36b6) )
ROM_END
ROM_START( g_twic )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_twic.bin", 0x000000, 0x0a0000, CRC(2c708248) SHA1(c386c617703a3f5278d24b310c6bc15e3e180bdf) )
ROM_END
ROM_START( g_turr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_turr.bin", 0x000000, 0x080000, CRC(634d67a6) SHA1(5a471a276909dcc428cd66c51047aa8a142c76a8) )
ROM_END
ROM_START( g_trux )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_trux.bin", 0x000000, 0x080000, CRC(5bd0882d) SHA1(90039844478e7cb99951fdff1979c3bda04d080a) )
ROM_END
ROM_START( g_tp96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tp96.bin", 0x000000, 0x400000, CRC(f1748e91) SHA1(c0981b524d5e1c5368f9e74a4ce9c57d87fe323a) )
ROM_END
ROM_START( g_tpgo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tpgo.bin", 0x000000, 0x400000,  CRC(bbe69017) SHA1(007bee242384db1887c5831657470584ff77a163) )
ROM_END

ROM_START( g_todd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_todd.bin", 0x000000, 0x080000, CRC(652e8b7d) SHA1(e558e39e3e556d20c789bf2823af64e1a5c78784) )
ROM_END
ROM_START( g_tkil )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tkil.bin", 0x000000, 0x200000, CRC(4b5f52ac) SHA1(91b2dd5463261ca240c4977f58d9c8fd7e770624) )
ROM_END
ROM_START( g_tfox )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tfox.bin", 0x000000, 0x100000, CRC(5463f50f) SHA1(c1699ccabb89c2877dd616471e80d175434bffe3) )
ROM_END
ROM_START( g_tetr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tetr.bin", 0x000000, 0x040000, CRC(4ce90db0) SHA1(2f2b559c5855e34500e43fb5cc8aff04dd72eb56) )
ROM_END
ROM_START( g_synd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_synd.bin", 0x000000, 0x200000, CRC(95bbf87b) SHA1(87442ecc50df508d54d241cbd468b41c926b974d) )
ROM_END
ROM_START( g_pins )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pins.bin", 0x000000, 0x300000, CRC(abe9c415) SHA1(db6b2046e8b6373f141e8c5db68450f2db377dd8) )
ROM_END
ROM_START( g_pifg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pifg.bin", 0x000000, 0x100000, CRC(d48a8b02) SHA1(8cd0ffaf3c29b3aa1f9068cd89cb64f492066d74) )
ROM_END
ROM_START( g_pac2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pac2.bin", 0x000000, 0x200000, CRC(fe7a7ed1) SHA1(9ed27068b00345d04d9dd1052ba0606c172e0090) )
ROM_END
ROM_START( g_nbd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nbd.bin", 0x000000, 0x100000,  CRC(a8d828a0) SHA1(9cc3419ca7ecaf0d106aa896ffc0266c7145fff7) )
ROM_END

ROM_START( g_ko3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ko3.bin", 0x000000, 0x100000, CRC(bc37401a) SHA1(e449cfd4f9d59cf28b4842d465022a399964d0d6) )
ROM_END
ROM_START( g_kgk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kgk.bin", 0x000000, 0x100000, CRC(effc0fa6) SHA1(893b1bae5242f25494b6de64a861b1aa1dc6cf14) )
ROM_END
ROM_START( g_ksal )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ksal.bin", 0x000000, 0x080000, CRC(f516e7d9) SHA1(c2d8e2c569a2c275677ae85094e0dfad7fdf680e) )
ROM_END
ROM_START( g_lvsc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lvsc.bin", 0x000000, 0x080000, CRC(0e33fc75) SHA1(b70b5f884dd7b26ffe2d6d50625dd61fec8f2899) )
ROM_END
ROM_START( g_wbdt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wbdt.bin", 0x000000, 0x100000, CRC(70155b5b) SHA1(b1f8e741399fd2c28dfb1c3340af868d222b1c14) )
ROM_END
ROM_START( g_comc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_comc.bin", 0x000000, 0x100000, CRC(e439b101) SHA1(0c7ca93b412c8ab5753ae047de49a3e41271cc3b) )
ROM_END

ROM_START( g_688a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_688a.bin", 0x000000, 0x100000, CRC(f2c58bf7) SHA1(6795b9fc9a21167d94a0b4c9c38d4e11214e1ea7) )
ROM_END
ROM_START( g_adiv )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_adiv.bin", 0x000000, 0x080000, CRC(2041885e) SHA1(0ea311e3ed667d5d8b7ca4fad66f9d069f0bcb77) )
ROM_END
ROM_START( g_asto )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_asto.bin", 0x000000, 0x080000, CRC(f5ac8de5) SHA1(e4f8774c5f96db76a781c31476d06203ec16811a) )
ROM_END
ROM_START( g_abea )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_abea.bin", 0x000000, 0x080000, CRC(154d59bb) SHA1(38945360d824d2fb9535b4fd7f25b9aa9b32f019) )
ROM_END
ROM_START( g_agla )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_agla.bin", 0x000000, 0x100000, CRC(9952fa85) SHA1(996581f1b1f7887f3f103ed170ddd9e03ce8c74c) )
ROM_END
ROM_START( g_arch )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arch.bin", 0x000000, 0x080000, CRC(e389d7e7) SHA1(2bfbe4698f13ade720dbfe10cebf02fe51e5e6ef) )
ROM_END
ROM_START( g_aptg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aptg.bin", 0x000000, 0x080000, CRC(35b995ef) SHA1(76ab194beafcf9e9d5bc40a8e70e2a01d7e42a5b) )
ROM_END
ROM_START( g_aof )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aof.bin", 0x000000, 0x200000, CRC(c9a57e07) SHA1(7603f33a98994e8145c942e1ed28e6b072332324) )
ROM_END
ROM_START( g_akid )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_akid.bin", 0x000000, 0x080000, CRC(7cd8169e) SHA1(38115bc07f11885b4e4c19f92508f729ad6c8765) )
ROM_END
ROM_START( g_atpt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_atpt.bin", 0x000000, 0x200000, CRC(8c822884) SHA1(1ccd027cac63ee56b24a54a84706646d22d0b610) ) // backup
ROM_END

ROM_START( g_arug )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arug.bin", 0x000000, 0x200000, CRC(ac5bc26a) SHA1(b54754180f22d52fc56bab3aeb7a1edd64c13fef) )
ROM_END
ROM_START( g_bjak )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bjak.bin", 0x000000, 0x040000,CRC(f5c3c54f) SHA1(c8aa71c5632a5cc59da430ca3870cffb37fbd30f) ) // colour?
ROM_END
ROM_START( g_barb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_barb.bin", 0x000000, 0x100000, CRC(81c9662b) SHA1(490f4f7a45ca7f3e13069a498218ae8eaa563e85) )
ROM_END
ROM_START( g_bass )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bass.bin", 0x000000, 0x200000, CRC(cf1ff00a) SHA1(62ece2382d37930b84b62e600b1107723fbbc77f) )
ROM_END
ROM_START( g_basp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_basp.bin", 0x000000, 0x200000, CRC(9eddeb3d) SHA1(9af4f7138d261122dfbc88c8afcbc769ce923816) ) // raster
ROM_END
ROM_START( g_barn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_barn.bin", 0x000000, 0x100000, CRC(1efa9d53) SHA1(64ebe54459267efaa400d801fc80be2097a6c60f) )
ROM_END
ROM_START( g_btl2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_btl2.bin", 0x000000, 0x100000, CRC(312fa0f2) SHA1(fbcf7e899ff964f52ade160793d6cb22bf68375c) )
ROM_END
ROM_START( g_btlm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_btlm.bin", 0x000000, 0x080000, CRC(a76c4a29) SHA1(9886dd39cfd8fe505860fc4d0119aacec7484a4e) )
ROM_END
ROM_START( g_btms )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_btms.bin", 0x000000, 0x080000, CRC(fd2b35e3) SHA1(c27ad7070ec068cb2eb13a9e6bdfb3b70e55d4ad) )
ROM_END
ROM_START( g_btec )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_btec.bin", 0x000000, 0x200000, CRC(409e5d14) SHA1(ee22ae12053928b652f9b9f513499181b01c8429) )
ROM_END

ROM_START( g_bwre )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bwre.bin", 0x000000, 0x100000, CRC(0ca5bb64) SHA1(b8752043027a6fb9717543b02f42c99759dfd45e) )
ROM_END
ROM_START( g_bear )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bear.bin", 0x000000, 0x100000, CRC(1f86237b) SHA1(5fd62e60fd3dd19f78d63a93684881c7a81485d6)) // a mess!
ROM_END
ROM_START( g_bw95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bw95.bin", 0x000000, 0x200000, CRC(a582f45a) SHA1(2ae000f45474b3cdedd08eeca7f5e195959ba689) )
ROM_END
ROM_START( g_bwcf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bwcf.bin", 0x000000, 0x100000,CRC(3ed83362) SHA1(2bbb454900ac99172a2d72d1e6f96a96b8d6840b) )
ROM_END
ROM_START( g_bimi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bimi.bin", 0x000000, 0x080000, CRC(d4dc5188) SHA1(62a6a1780f4846dcfefeae11de35a18ec039f424) )
ROM_END
ROM_START( g_bnza )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bnza.bin", 0x000000, 0x080000,CRC(20d1ad4c) SHA1(31c589bc0d1605502cdd04069dc4877811e84e58) ) // raster
ROM_END
ROM_START( g_bwbw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bwbw.bin", 0x000000, 0x080000, CRC(ccf52828) SHA1(57599254461bf05ac0822c42e0bb223a68d4cd71) )
ROM_END
ROM_START( g_boxl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_boxl.bin", 0x000000, 0x100000, CRC(00f225ac) SHA1(4004af082d0e917d6b05c137ab5cff55e11c12a9) )
ROM_END
ROM_START( g_bh95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bh95.bin", 0x000000, 0x200000, CRC(f7775a09) SHA1(bcc9d8a737b5b6ccc5ddcd5906202508e4307f79) )
ROM_END
ROM_START( g_blcr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blcr.bin", 0x000000, 0x100000, CRC(408cf5c3) SHA1(032eb1a15c95540b392b52ee17eef7eee6ac436d) ) // z80 / sound
ROM_END

ROM_START( g_bvsb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bvsb.bin", 0x000000, 0x100000, CRC(d4e4b4e8) SHA1(6db65704e2132c2f6e0501e8481abde7f2c7078d) )
ROM_END
ROM_START( g_bvsl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bvsl.bin", 0x000000, 0x100000, CRC(e56023a0) SHA1(102652dcd218e3420ea9c4116231fa62f8fcd770) )
ROM_END
ROM_START( g_burf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_burf.bin", 0x000000, 0x080000, CRC(bdc8f02c) SHA1(28fcb1c9b5c72255443ab5bb950a52030baaf409) ) // raster
ROM_END
ROM_START( g_cada )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cada.bin", 0x000000, 0x080000, CRC(13bdf374) SHA1(5791cc9a1b118c58fc5209bf2e64156ffdb80134) )
ROM_END
ROM_START( g_caes )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_caes.bin", 0x000000, 0x080000, CRC(8fdaa9bb) SHA1(83c16e539ce332e1d33ab196b84abc1be4beb982) )
ROM_END
ROM_START( g_crjb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crjb.bin", 0x000000, 0x100000, CRC(9b1c96c0) SHA1(47579111ed8f9d4f4eb48ec700272bc73ee35295) )
ROM_END
ROM_START( g_c50 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_c50.bin", 0x000000, 0x100000, CRC(44f4fa05) SHA1(a68f3b9350a3a05850c17d157b56de88556cd26a) )
ROM_END
ROM_START( g_cgam )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cgam.bin", 0x000000, 0x080000, CRC(43b1b672) SHA1(0417ff05bb8bd696cfae8f795c09786665cb60ef) )
ROM_END
ROM_START( g_capa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_capa.bin", 0x000000, 0x100000, CRC(e0639ca2) SHA1(ae71b5744de7ca6aa00d72ccd7713146d87ca002) )
ROM_END
ROM_START( g_capp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_capp.bin", 0x000000, 0x080000, CRC(7672efa5) SHA1(b048f922db83802a4a78d1e8197a5ec52b73a89f) )
ROM_END

ROM_START( g_chik )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chik.bin", 0x000000, 0x100000, CRC(813a7d62) SHA1(3cbeb068751c39790116aa8f422dd6f333be42e0) )
ROM_END
ROM_START( g_clue )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_clue.bin", 0x000000, 0x080000, CRC(7753a296) SHA1(d1f9114f41a3d6237e24392629fea5fbeb3f0b87) )
ROM_END
ROM_START( g_corp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_corp.bin", 0x000000, 0x100000, CRC(a80d18aa) SHA1(520e01abe76120dbd680b9fc34eb1303c780b069) )
ROM_END
ROM_START( g_cuty )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cuty.bin", 0x000000, 0x080000, CRC(13795dca) SHA1(4e3f222c7333d39fc5452b37aebc14fa09ff8d5e) )
ROM_END
ROM_START( g_ddwe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ddwe.bin", 0x000000, 0x100000, CRC(af4a9cd7) SHA1(9135f7fda03ef7da92dfade9c0df75808214f693) )
ROM_END
ROM_START( g_dlnd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dlnd.bin", 0x000000, 0x080000, CRC(5fe351b8) SHA1(e55937e48a0c348ba3db731a8722dae7f39dede5) )
ROM_END
ROM_START( g_dduk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dduk.bin", 0x000000, 0x080000, CRC(39d01c8c) SHA1(1bd77ad31665f7bdda85d9dfb9f08c0338ec4da9) )
ROM_END
ROM_START( g_ecjr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ecjr.bin", 0x000000, 0x100000, CRC(6c6f4b89) SHA1(0b493ef23874f82606d4fd22c2380b289247aa9f) )
ROM_END
ROM_START( g_e_bt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_e_bt.bin", 0x000000, 0x200000, CRC(96d8440c) SHA1(8353eb5ccf4fe6676465eb2619b8ecb594191e0a) )
ROM_END
ROM_START( g_e_hn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_e_hn.bin", 0x000000, 0x200000, CRC(1d08828c) SHA1(cfd65e3ffb17e1718356ef8de7c527e2c9fd8940) ) // colours
ROM_END

ROM_START( g_e_sn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_e_sn.bin", 0x000000, 0x200000, CRC(61e9c309) SHA1(03f8c8805ebd4313c8a7d76b34121339bad33f89) ) // size 33
ROM_END
ROM_START( g_ehrd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ehrd.bin", 0x000000, 0x080000, CRC(4fef37c8) SHA1(eb4aca22f8b5837a0a0b10491c46714948b09844) )
ROM_END
ROM_START( g_f15s )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f15s.bin", 0x000000, 0x100000, CRC(412c4d60) SHA1(d5f0d3d3b9cb7557d85bd54465072555f60c25a1) )
ROM_END
ROM_START( g_feud )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_feud.bin", 0x000000, 0x080000, CRC(1aa628b0) SHA1(8277f86faf6968e277638ab6dfa410a5b3daf5b6) )
ROM_END
ROM_START( g_fatl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fatl.bin", 0x000000, 0x020000, CRC(5f0bd984) SHA1(c136cb3fae1914a8fe079eb28c4533af5d8774e2) )
ROM_END
ROM_START( g_frgp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_frgp.bin", 0x000000, 0x100000, CRC(f73f6bec) SHA1(bd08e2e0857c7b385c15d96e555f25d12457c917) ) // raster
ROM_END
ROM_START( g_fifa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fifa.bin", 0x000000, 0x200000, CRC(bddbb763) SHA1(1cbef8c4541311b84d7388365d12a93a1f712dc4) )
ROM_END
ROM_START( g_fi95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fi95.bin", 0x000000, 0x200000, CRC(b389d036) SHA1(586f9d0f218cf6bb3388a8610b44b6ebb9538fb5) )
ROM_END
ROM_START( g_fi96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fi96.bin", 0x000000, 0x200000, CRC(bad30ffa) SHA1(a7fcfe478b368d7d33bcbca65245f5faed9a1e07) )
ROM_END
ROM_START( g_fi97 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fi97.bin", 0x000000, 0x200000, CRC(a33d5803) SHA1(2d91cb1c50586723f877cb25a37b5ebcd70d8bcc) )
ROM_END

ROM_START( g_gfko )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gfko.bin", 0x000000, 0x100000, CRC(e1fdc787) SHA1(62a953cc6ea8535aa7f3f59b40cff0e285d4392a) )
ROM_END
ROM_START( g_gste )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gste.bin", 0x000000, 0x080000,  CRC(8c2670de) SHA1(4fd5ce8207c4568da07bb28c6ab38a23084c801a) )
ROM_END
ROM_START( g_grwl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_grwl.bin", 0x000000, 0x080000, CRC(f60ef143) SHA1(63a08057fe9fd489590f39c5c06709266f625ab0) )
ROM_END
ROM_START( g_hnov )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hnov.bin", 0x000000, 0x100000, CRC(f6b6a9d8) SHA1(f5bda60615c7436a40b5a31a726099583ed85413) )
ROM_END
ROM_START( g_hice )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hice.bin", 0x000000, 0x080000, CRC(85b23606) SHA1(d0eec70787362b415ffdcf09524e0e6cf0f9f910) )
ROM_END
ROM_START( g_jnpg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jnpg.bin", 0x000000, 0x100000, CRC(5545e909) SHA1(8d7edfe87da732ecd9820a6afbb9c5700cce43b2) )
ROM_END
ROM_START( g_ktm2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ktm2.bin", 0x000000, 0x200000, CRC(ee1638ac) SHA1(9fa2d329ea443b3e030206fd64d7faa7778d492d) )
ROM_END
ROM_START( g_ktm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ktm.bin", 0x000000, 0x100000, CRC(f390d406) SHA1(b88555b50b12f7d9470e5a9870882ca38100342c) )
ROM_END
ROM_START( g_klax )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_klax.bin", 0x000000, 0x040000, CRC(248cd09e) SHA1(b7c07baf74b945549e067405566eeaa6856dd6b1) )
ROM_END
ROM_START( g_len )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_len.bin", 0x000000, 0x200000, CRC(51d9a84a) SHA1(309b41c6d4159f4f07fe9a76aca4fc4ddf45de63) )
ROM_END

ROM_START( g_len2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_len2.bin", 0x000000, 0x200000, CRC(e5fdd28b) SHA1(823a59b1177665313a1114a622ee98a795141eec) )
ROM_END
ROM_START( g_mamr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mamr.bin", 0x000000, 0x200000, CRC(7f1dc0aa) SHA1(d28d79177cf25c8cbf54b28a7d357ac86b7820b5) ) // raster
ROM_END
ROM_START( g_mamo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mamo.bin", 0x000000, 0x080000, CRC(91354820) SHA1(28f38617911a99504542b30f70c0d9c81996ef65) ) // data error
ROM_END
ROM_START( g_mbmb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mbmb.bin", 0x000000, 0x100000, CRC(4bd6667d) SHA1(89495a8acbcb0ddcadfe5b7bada50f4d9efd0ddd) )
ROM_END
ROM_START( g_njte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_njte.bin", 0x000000, 0x300000, CRC(6e25ebf0) SHA1(0f5bb5d5352fe2ebe4b4051a1dd9b9fde4b505ab) )
ROM_END
ROM_START( g_nl98 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nl98.bin", 0x000000, 0x200000, CRC(23473a8a) SHA1(89b98867c7393371a364de58ba6955e0798fa10f) ) // raster
ROM_END
ROM_START( g_tnzs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tnzs.bin", 0x000000, 0x080000, CRC(1c77ad21) SHA1(07b75c12667c6c5620aa9143b4c5f35b76b15418) )
ROM_END
ROM_START( g_noes )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_noes.bin", 0x000000, 0x200000, CRC(44ee5f20) SHA1(d72302927b618165cd94ec2e072c8d8bbf85cbb9) )
ROM_END
ROM_START( g_norm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_norm.bin", 0x000000, 0x100000, CRC(b56a8220) SHA1(286d4f922fa9a95740e791c284d141a49983d871) )
ROM_END
ROM_START( g_papb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_papb.bin", 0x000000, 0x080000, CRC(0a44819b) SHA1(10faaf47e585423eec45285be4f5ce1015ae3386) )
ROM_END

ROM_START( g_pap2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pap2.bin", 0x000000, 0x100000, CRC(1de28bb1) SHA1(6421c26000bec21d0c808252c5d9ce75b75cb8a4) )
ROM_END
ROM_START( g_pop2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pop2.bin", 0x000000, 0x100000, CRC(ee988bd9) SHA1(556a79febff1fe4beb41e4e8a2629ff02b20c38f) )
ROM_END
ROM_START( g_quad )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_quad.bin", 0x000000, 0x080000, CRC(74736a80) SHA1(c7db4cd8b5dc8c0927824e9ddf1257591305d99a) ) // raster
ROM_END
ROM_START( g_risk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_risk.bin", 0x000000, 0x080000, CRC(80416d0d) SHA1(6f4bf5e1dc0de79e42934e5867641e1baadba0d9) )
ROM_END
ROM_START( g_rise )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rise.bin", 0x000000, 0x300000, CRC(5650780b) SHA1(646bf23a13a4c2ad249ca3cc58ff3cd3244e5dff) )
ROM_END
ROM_START( g_shov )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shov.bin", 0x000000, 0x020000, CRC(c51f40cb) SHA1(e4094c5a575f8d7325e7ec7425ecf022a6bf434e) )
ROM_END
ROM_START( g_side )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_side.bin", 0x000000, 0x100000, CRC(af9f275d) SHA1(2b30982d04628edc620d8d99f7dceb4ed87b41e3) )
ROM_END
ROM_START( g_std9 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_std9.bin", 0x000000, 0x100000, CRC(a771e1a4) SHA1(3918dcf48d99d075a6062d967a0f27fb56f89ad2) )
ROM_END
ROM_START( g_stng )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stng.bin", 0x000000, 0x200000,  CRC(ef840ef2) SHA1(fe72aff307182dc6970048e88eaa5f03348781f5) )
ROM_END
ROM_START( g_t2ar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_t2ar.bin", 0x000000, 0x100000, CRC(a1264f17) SHA1(85cc1cf3379d3ce23ca3c03d84fe6e2b3adc9c56) )
ROM_END
ROM_START( g_vali )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vali.bin", 0x000000, 0x100000, CRC(13bc5b72) SHA1(fed29e779aa0d75645a59608f9f3a13f39d43888) )
ROM_END
ROM_START( g_val3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_val3.bin", 0x000000, 0x100000, CRC(59a2a368) SHA1(bb051779d6c4c68a8a4571177990f7d190696b4a) )
ROM_END
ROM_START( g_vpin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vpin.bin", 0x000000, 0x100000, CRC(d63473aa) SHA1(cd066bb54e0a4c21821639728893462b0218597e) )
ROM_END
ROM_START( g_wimb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wimb.bin", 0x000000, 0x100000, CRC(f9142aee) SHA1(db9083fd257d7cb718a010bdc525980f254e2c66) )
ROM_END

ROM_START( g_nl97 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nl97.bin", 0x000000, 0x200000, CRC(7024843a) SHA1(1671451ab4ab6991e13db70671054c0f2c652a95) )
ROM_END
ROM_START( g_nl96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nl96.bin", 0x000000, 0x200000, CRC(49de0062) SHA1(5fca106c839d3dea11cbf6842d1d7650db06ca72) )
ROM_END
ROM_START( g_nl95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nl95.bin", 0x000000, 0x200000, CRC(66018abc) SHA1(f86bc9601751ac94119ab2f3ecce2029d5678f01) )
ROM_END
ROM_START( g_njam )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_njam.bin", 0x000000, 0x200000, CRC(eb8360e6) SHA1(55f2b26a932c69b2c7cb4f24f56b43f24f113a7c) )
ROM_END
ROM_START( g_nasc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nasc.bin", 0x000000, 0x100000, CRC(c4674adf) SHA1(fc55c83df4318a17b55418ce14619e42805e497f) ) // raster
ROM_END
ROM_START( g_nact )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nact.bin", 0x000000, 0x200000, CRC(99c348ba) SHA1(e2b5290d656219636e2422fcf93424ae602c4d29) )
ROM_END
ROM_START( g_na95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_na95.bin", 0x000000, 0x200000, CRC(aa7006d6) SHA1(34e2df219e09c24c95c588a37d2a2c5e15814d68) )
ROM_END
ROM_START( g_npbb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_npbb.bin", 0x000000, 0x100000, CRC(4565ce1f) SHA1(77a048094269fbf5cf9f0ad02ea781ba3bcbecf3) )
ROM_END
ROM_START( g_nba9 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nba9.bin", 0x000000, 0x200000, CRC(eea19bce) SHA1(99c91fe3a5401e84e7b3fd2218dcb3aeaf10db74) ) // resets
ROM_END
ROM_START( g_npbl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_npbl.bin", 0x000000, 0x100000, CRC(4416ce39) SHA1(003b01282aff14ccea9390551aba5ac4a9e53825) )
ROM_END

ROM_START( g_nbs9 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nbs9.bin", 0x000000, 0x200000, CRC(160b7090) SHA1(3134a3cb63115d2e16e63a76c2708cdaecab83e4) ) // resets
ROM_END
ROM_START( g_nccf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nccf.bin", 0x000000, 0x100000, CRC(081012f0) SHA1(227e3c650d01c35a80de8a3ef9b18f96c07ecd38) )
ROM_END
ROM_START( g_ncff )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ncff.bin", 0x000000, 0x180000, CRC(ed0c1303) SHA1(29021b8c3bbcc62606c692a3de90d4e7a71b6361) ) // bad gfx
ROM_END
ROM_START( g_nfl8 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nfl8.bin", 0x000000, 0x200000, CRC(f73ec54c) SHA1(27ea0133251e96c091b5a96024eb099cdca21e40) ) // no boot
ROM_END
ROM_START( g_nfl5 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nfl5.bin", 0x000000, 0x200000, CRC(b58e4a81) SHA1(327249456f96ccb3e9758c0162c5f3e3f389072f) ) // no boot
ROM_END
ROM_START( g_nfl4 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nfl4.bin", 0x000000, 0x200000, CRC(0d486ed5) SHA1(ad0150d0c0cabe1ab05e3d6ac4eb7aa4b38a467c) )
ROM_END
ROM_START( g_nfpt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nfpt.bin", 0x000000, 0x200000, CRC(5aa53cbc) SHA1(7d7a5a920ac30831556b58caac66f4a8dde1632a) ) // no boot
ROM_END
ROM_START( g_nqc6 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nqc6.bin", 0x000000, 0x400000, CRC(d5a37cab) SHA1(2948419532a6079404f05348bc4bbf2dd989622d) ) // no boot
ROM_END
ROM_START( g_nqc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nqc.bin", 0x000000, 0x300000, CRC(94542eaf) SHA1(60744af955df83278f119df3478baeebd735a26c) ) // no boot
ROM_END

ROM_START( g_nh95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nh95.bin", 0x000000, 0x200000, CRC(e8ee917e) SHA1(09e87b076aa4cd6f057a1d65bb50fd889b509b44) )
ROM_END
ROM_START( g_nh9e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nh9e.bin", 0x000000, 0x200000, CRC(e10a25c0) SHA1(5f2c8303099ce13fe1e5760b7ef598a2967bfa8d) )
ROM_END
ROM_START( g_nh96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nh96.bin", 0x000000, 0x200000, CRC(8135702c) SHA1(7204633fbb9966ac637e7966d02ba15c5acdee6b) )
ROM_END
ROM_START( g_nh97 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nh97.bin", 0x000000, 0x200000, CRC(f067c103) SHA1(8a90d7921ab8380c0abb0b5515a6b9f96ca6023c) )
ROM_END
ROM_START( g_nash )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nash.bin", 0x000000, 0x200000, CRC(e6c0218b) SHA1(f2069295f99b5d50444668cfa9d4216d988d5f46) )
ROM_END
ROM_START( g_nhlh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nhlh.bin", 0x000000, 0x080000, CRC(2641653f) SHA1(2337ad3c065f46edccf19007d7a5be80a97520d5) )
ROM_END
ROM_START( g_nhlp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nhlp.bin", 0x000000, 0x080000, CRC(f361d0bf) SHA1(2ab048fc7209df28b00ef47f2f686f5b7208466e) )
ROM_END
ROM_START( g_olsg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olsg.bin", 0x000000, 0x200000, CRC(9e470fb9) SHA1(88c8627d0b5bd5f4847d4b62f3681fbadc477829) )
ROM_END
ROM_START( g_olwg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olwg.bin", 0x000000, 0x200000, CRC(c5834437) SHA1(da5a2a0c1a0a6b7b29492f8845eca1167158ebea) )
ROM_END
ROM_START( g_patb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_patb.bin", 0x000000, 0x080000, CRC(3d9318a7) SHA1(0655ac1043c27c68d40ae089f1af2826613e5f80) )
ROM_END

ROM_START( g_pele )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pele.bin", 0x000000, 0x100000, CRC(5a8abe51) SHA1(c0a91e2d10b98174faa647fb732c335d7438abf7) )
ROM_END
ROM_START( g_pelw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pelw.bin", 0x000000, 0x200000, CRC(05a486e9) SHA1(96498065545ed122aa29471d762a2ae1362b2dea) )
ROM_END
ROM_START( g_pst )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pst.bin", 0x000000, 0x100000, CRC(9ef5bbd1) SHA1(222a66cdb8865a7f89e5a72418413888bb400176) )
ROM_END
ROM_START( g_pgae )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pgae.bin", 0x000000, 0x100000, CRC(8ca45acd) SHA1(640615be6891a8457d94bb81b0e8e1fa7c5119a8) )
ROM_END
ROM_START( g_rb3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rb3.bin", 0x000000, 0x080000, CRC(4840348c) SHA1(fe6d17d8670e046a90d126ee6a11149735a1aeb8) )
ROM_END
ROM_START( g_rb4 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rb4.bin", 0x000000, 0x100000, CRC(fecf9b94) SHA1(37aaac7eda046968f4bff766566ca15b473f9060) )
ROM_END
ROM_START( g_rb93 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rb93.bin", 0x000000, 0x100000, CRC(beafce84) SHA1(4db6cca411ba3d32ad43d572389612935a42e3b3) )
ROM_END
ROM_START( g_rb94 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rb94.bin", 0x000000, 0x200000, CRC(4eb4d5e4) SHA1(3e94f52d311fae140da870f55e3cb103d7923d44) )
ROM_END
ROM_START( g_rcvm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rcvm.bin", 0x000000, 0x100000, CRC(83699e34) SHA1(10b4be6f7e8046ec527ea487ce5f2678990e92c6) )
ROM_END
ROM_START( g_rw93 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rw93.bin", 0x000000, 0x200000, CRC(61f90a8a) SHA1(9b435c82b612e23cb512efaebf4d35b203339e44) )
ROM_END

ROM_START( g_swcr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_swcr.bin", 0x000000, 0x100000, CRC(68865f6f) SHA1(4264625be91c3d96edca9c5bfe6259d00ca8b737) )
ROM_END
ROM_START( g_shi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shi.bin", 0x000000, 0x100000, CRC(b870c2f7) SHA1(a5ce32a8e96d39424d4377c45befa6fd8389b941) ) // too fast (hv?)
ROM_END
ROM_START( g_srba )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_srba.bin", 0x000000, 0x080000, CRC(f04765ba) SHA1(0197df59951085dc7078c4ec66c75be84566530a) )
ROM_END
ROM_START( g_svol )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_svol.bin", 0x000000, 0x040000, CRC(a88fee44) SHA1(4fbf379fc806453c763980a3e6ee7f858f32ee3e) )
ROM_END
ROM_START( g_virr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virr.bin", 0x000000, 0x200000, CRC(7e1a324a) SHA1(ff969ae53120cc4e7cb1a8a7e47458f2eb8a2165) )
ROM_END
ROM_START( g_ws96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ws96.bin", 0x000000, 0x300000, CRC(04ee8272) SHA1(f91eeedbadd277904f821dfaae9e46f6078ff207) ) // invalid vsize
ROM_END
ROM_START( g_ws95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ws95.bin", 0x000000, 0x300000, CRC(25130077) SHA1(878e9fdbbc0b20b27f25d56e4087efbde1e8979a) ) // invalid vsize
ROM_END
ROM_START( g_wsb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wsb.bin", 0x000000, 0x200000, CRC(57c1d5ec) SHA1(e63cbd1f00eac2ccd2ee0290e7bf1bb47c1288e4) )
ROM_END
ROM_START( g_wts )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wts.bin", 0x000000, 0x080000, CRC(6e3edc7c) SHA1(0a0ac6d37d284ec29b9331776bbf6b78edcdbe81) ) // clone of euro club soccer
ROM_END
ROM_START( g_wfsw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wfsw.bin", 0x000000, 0x100000, CRC(b929d6c5) SHA1(a6df2e9887a3f33139e505b5ac739158b987069f) )
ROM_END

ROM_START( g_ys3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ys3.bin", 0x000000, 0x100000, CRC(ea27976e) SHA1(b3331b41a9a5e2c3f4fb3b64c5b005037a3b6fdd) )
ROM_END
ROM_START( g_ma95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma95.bin", 0x000000, 0x200000, CRC(db0be0c2) SHA1(41cde6211da87a8e61e2ffd42cef5de588f9b9fc) )
ROM_END
ROM_START( g_ma96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma96.bin", 0x000000, 0x200000, CRC(f126918b) SHA1(35a4241eed51f10de2e63c843f162ce5d92c70a2) )
ROM_END
ROM_START( g_ma97 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma97.bin", 0x000000, 0x200000, CRC(c4b4e112) SHA1(63544d2a0230be102f2558c03a74855fc712b865) )
ROM_END
ROM_START( g_ma98 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma98.bin", 0x000000, 0x200000, CRC(e051ea62) SHA1(761e0903798a8d0ad9e7ab72e6d2762fc9d366d2) )
ROM_END
ROM_START( g_ma94 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma94.bin", 0x000000, 0x200000, CRC(d14b811b) SHA1(856d68d3e8589df3452096434feef823684d11eb) )
ROM_END
ROM_START( g_ma93 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma93.bin", 0x000000, 0x100000,  CRC(ca323b3e) SHA1(59d2352ecb31bc1305128a9d8df894a3bfd684cf) )
ROM_END
ROM_START( g_ma92 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma92.bin", 0x000000, 0x080000, CRC(046e3945) SHA1(1a4c1dcc2de5018142a770f753ff42667b83e5be) )
ROM_END
ROM_START( g_ma3c )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma3c.bin", 0x000000, 0x100000, CRC(ca534b1a) SHA1(417acacb9f9ef90d08b3cfb81972a9d8b56f4293) )
ROM_END
ROM_START( g_ma )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ma.bin", 0x000000, 0x080000, CRC(90fb8818) SHA1(10682f1763711b281542fcd5e192e1633809dc75) )
ROM_END

ROM_START( g_jmof )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jmof.bin", 0x000000, 0x080000, CRC(8aa6a1dd) SHA1(64b03ad0b17c022c831d057b9d6087c3b719147a) )
ROM_END
ROM_START( g_jms2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jms2.bin", 0x000000, 0x100000, CRC(a45da893) SHA1(ce006ff3b9bcd71fef4591a63a80d887004abe77) )
ROM_END
ROM_START( g_jms )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jms.bin", 0x000000, 0x180000, CRC(ce0b1fe1) SHA1(3c97396cb4cc566013d06f055384105983582067) )
ROM_END
ROM_START( g_hb94 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hb94.bin", 0x000000, 0x200000, CRC(ea9c4878) SHA1(eceeecb3d520f9b350e41d0dd010abefbcfbbdab) ) // backup
ROM_END
ROM_START( g_hb3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hb3.bin", 0x000000, 0x200000, CRC(a4f2f011) SHA1(88812ae494288fbf3dda86ccc69161ad960a2e3b) ) // backup
ROM_END
ROM_START( g_hb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hb.bin", 0x000000, 0x100000, CRC(bd1b9a04) SHA1(42d42af36b4a69f0adb38aaa7fec32eb8c44c349) )
ROM_END

ROM_START( g_coak )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_coak.bin", 0x000000, 0x200000, CRC(67c309c6) SHA1(8ff5d7a7fcc47f030a3ea69f4534d9c892f58ce2) )
ROM_END
ROM_START( g_cf96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cf96.bin", 0x000000, 0x200000, CRC(b9075385) SHA1(3079bdc5f2d29dcf3798f899a3098736cdc2cd88) )
ROM_END
ROM_START( g_cf97 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cf97.bin", 0x000000, 0x200000, CRC(2ebb90a3) SHA1(9b93035ecdc2b6f0815281764ef647f2de039e7b) )
ROM_END
ROM_START( g_cfn )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cfn.bin", 0x000000, 0x200000, CRC(172c5dbb) SHA1(a3db8661e160e07b09bca03ba0d20ba4e80a4c59) )
ROM_END
ROM_START( g_cfn2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cfn2.bin", 0x000000, 0x200000, CRC(65b64413) SHA1(9609f9934a80dba183dab603ae07f445f02b919d) )
ROM_END
ROM_START( g_csla )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_csla.bin", 0x000000, 0x400000, CRC(96a42431) SHA1(0dbbe740b14077fe8648955f7e17965ea25f382a) ) // no boot
ROM_END
ROM_START( g_cfir )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cfir.bin", 0x000000, 0x080000, CRC(cc73f3a9) SHA1(cc681bb62483dfb3ee3ef976dff29cc80ad01820) )
ROM_END
ROM_START( g_cc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cc.bin", 0x000000, 0x200000, CRC(41858f6f) SHA1(bf2e8d122f4670865bedbc305ef991ee5f52d647) )
ROM_END
ROM_START( g_crys )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crys.bin", 0x000000, 0x100000, CRC(6cf7a4df) SHA1(dc99ed62da89745559f49b040d0365038140efd9) )
ROM_END
ROM_START( g_cybb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cybb.bin", 0x000000, 0x080000, CRC(76120e96) SHA1(4e459751ced8956326602c581b8b169f8e716545) )
ROM_END

ROM_START( g_ccop )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ccop.bin", 0x000000, 0x100000, CRC(01e719c8) SHA1(72470ea83064046a0b1dfba3047032c26d610df8) )
ROM_END
ROM_START( g_dcat )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dcat.bin", 0x000000, 0x100000, CRC(9177088c) SHA1(1543d9790f4311847fd4426b6f395b9c63365a9e) )
ROM_END
ROM_START( g_drbb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drbb.bin", 0x000000, 0x080000,  CRC(56164b00) SHA1(eeac902df31240c246afe2b97a5b25104dd9f0e5) )
ROM_END
ROM_START( g_drsc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drsc.bin", 0x000000, 0x080000, CRC(512b7599) SHA1(adcee1a54caa0db0aab0927b877d2be828964275) )
ROM_END
ROM_START( g_dcmd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dcmd.bin", 0x000000, 0x080000, CRC(4c4caad8) SHA1(58d447ecd6a6af9845cdd90ac3df0b5503535117) )
ROM_END
ROM_START( g_dc3d )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dc3d.bin", 0x000000, 0x180000, CRC(bd090c67) SHA1(4509c10ab263175c605111c72c6c63c57321046a) )
ROM_END
ROM_START( g_drib )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drib.bin", 0x000000, 0x200000, CRC(8352b1d0) SHA1(d97bbe009e9667709e04a1bed9f71cea7893a495) )
ROM_END
ROM_START( g_dbzj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dbzj.bin", 0x000000, 0x200000,  CRC(af8f3371) SHA1(dc6336dfbbe76c24ada735009d0d667ce27843f6) )
ROM_END
ROM_START( g_dbzf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dbzf.bin", 0x000000, 0x200000, CRC(f035c737) SHA1(5ff71986f4911b5dfd16598a5a3a9ba398c92c60) )
ROM_END
ROM_START( g_deye )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_deye.bin", 0x000000, 0x040000, CRC(81f0c3cf) SHA1(b5a2a3b0b65058614d24853c525505b709f00851) )
ROM_END

ROM_START( g_eaho )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_eaho.bin", 0x000000, 0x080000, CRC(9bfc279c) SHA1(118419e596bf07047a18d1828290019fbc4afc45) )
ROM_END
ROM_START( g_exil )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_exil.bin", 0x000000, 0x100000,  CRC(1b569dc2) SHA1(28377e9a68c2dcdfdb4133c2eb0b634aec552958) )
ROM_END
ROM_START( g_f1c )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1c.bin", 0x000000, 0x080000, CRC(5d30befb) SHA1(2efb8d46163e785a57421e726991328024ecd2a7) )
ROM_END
ROM_START( g_f1gp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1gp.bin", 0x000000, 0x100000, CRC(93be47cf) SHA1(e108342d53e1542d572d2e45524efbfe9d5dc964) )
ROM_END
ROM_START( g_f1h )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1h.bin", 0x000000, 0x100000, CRC(24f87987) SHA1(1a61809b4637ba48d193d29a5c25d72ab2c6d72d) )
ROM_END
ROM_START( g_f1sl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1sl.bin", 0x000000, 0x100000, CRC(8774bc79) SHA1(ad5456259890bcb32098f82f14a1d5355af83f7e) )
ROM_END
ROM_START( g_f1wc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1wc.bin", 0x000000, 0x100000, CRC(ccd73738) SHA1(8f8edd8e6846cbba1b46f8eb9015b195ccc4acf9) )
ROM_END
ROM_START( g_fas1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fas1.bin", 0x000000, 0x080000,  CRC(bb43f0de) SHA1(837a19b4b821076f1cd01d53d90b4553a0252340) ) // raster
ROM_END
ROM_START( g_fatm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fatm.bin", 0x000000, 0x0a0000, CRC(7867da3a) SHA1(9c2791e44f1ff236cbab742c2942defc7b378e2c) )
ROM_END
ROM_START( g_fmas )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fmas.bin", 0x000000, 0x080000, CRC(5f51983b) SHA1(2a860f06473e436041c2017342deb9125c1c7af5) )
ROM_END

ROM_START( g_fblo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fblo.bin", 0x000000, 0x080000, CRC(48ad505d) SHA1(610c3136e6ddccc41ab216affd07034fa46341a8) )
ROM_END
ROM_START( g_fpro )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fpro.bin", 0x000000, 0x080000, CRC(24408c73) SHA1(7f41afbf62d83424067f872a5dfe2c0f0ec40052) )
ROM_END
ROM_START( g_fshk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fshk.bin", 0x000000, 0x080000, CRC(9c175146) SHA1(020169eb2a4b3ad63fa2cbaa1927ab7c33b6add4) )
ROM_END
ROM_START( g_fore )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fore.bin", 0x000000, 0x300000, CRC(36248f90) SHA1(e1faa22d62652f7d7cf8c8b581f1df232f076f86) )
ROM_END
ROM_START( g_fw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fw.bin", 0x000000, 0x080000, CRC(95513985) SHA1(79aadecc1069f47a2a8b4a0a1d55712d4f9cb8ef) )
ROM_END
ROM_START( g_fung )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fung.bin", 0x000000, 0x100000, CRC(b5ae351d) SHA1(6085d3033edb32c468a63e0db95e5977e6853b48) )
ROM_END
ROM_START( g_geng )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_geng.bin", 0x000000, 0x100000, CRC(87a281ae) SHA1(4c1151413a261ad271543eeb64f512053f261a35) )
ROM_END
ROM_START( g_gws )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gws.bin", 0x000000, 0x100000,  CRC(8c5c93b8) SHA1(fe7601c92a67fb4deec28164c5fb0516ef0058c4) )
ROM_END
ROM_START( g_hydu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hydu.bin", 0x000000, 0x200000, CRC(f27c576a) SHA1(91560ddf53f50bb5b9f4b48be906ce27e9c05742) )
ROM_END
ROM_START( g_imgi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_imgi.bin", 0x000000, 0x200000, CRC(e04ffc2b) SHA1(1f1b410d17b39851785dee3eee332fff489db395) )
ROM_END

ROM_START( g_insx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_insx.bin", 0x000000, 0x080000, CRC(70626304) SHA1(4f4ecff167d5ada699b0b1d4b0e547b5ed9964d5) )
ROM_END
ROM_START( g_intr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_intr.bin", 0x000000, 0x080000, CRC(d97d1699) SHA1(8cea50f668fbfba7dd10244a001172c1e648c352) )
ROM_END

ROM_START( g_ishi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ishi.bin", 0x000000, 0x020000, CRC(b1de7d5e) SHA1(3bd159e323d86e69031bf1ee9febeb6f9bb078d4) )
ROM_END
ROM_START( g_jlcs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jlcs.bin", 0x000000, 0x080000, CRC(453c405e) SHA1(c28d8dac0f5582c3c2754342605764ee6e5b8af3) ) // euro soccer clone
ROM_END
ROM_START( g_jlp2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jlp2.bin", 0x000000, 0x200000, CRC(9fe71002) SHA1(924f1ae3d90bec7326a5531cd1d598cdeba30d36) )
ROM_END
ROM_START( g_jlpp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jlpp.bin", 0x000000, 0x100000, CRC(0abed379) SHA1(7cfd8c9119d0565ee9a7708dc46bb34dd3258e37) )
ROM_END
ROM_START( g_jlp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jlp.bin", 0x000000, 0x100000, CRC(2d5b7a11) SHA1(1e437182fab2980156b101a53623c3c2f27c3a6c) )
ROM_END
ROM_START( g_jlpf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jlpf.bin", 0x000000, 0x200000,  CRC(e35e25fb) SHA1(74e4a3ac4b93e25ace6ec8c3818e0df2390cffa2) )
ROM_END
ROM_START( g_jbdb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jbdb.bin", 0x000000, 0x080000, CRC(87bbcf2a) SHA1(617b61da9bceb6f4d8362d074dc1dad3f7304584) )
ROM_END

ROM_START( g_jamm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jamm.bin", 0x000000, 0x200000, CRC(d91b52b8) SHA1(2f43bfd04f563a67f4a2c8b8e36c541c19913a50) )
ROM_END
ROM_START( g_jano )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jano.bin", 0x000000, 0x100000, CRC(7011e8eb) SHA1(620ecac6c340e617bbde922bf742a9e6d7f8d786) )
ROM_END
ROM_START( g_jant )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jant.bin", 0x000000, 0x100000, CRC(8a1b19ad) SHA1(8e109961a89e1366c83808fb0bb33333f88a82b7) )
ROM_END
ROM_START( g_jeop )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jeop.bin", 0x000000, 0x080000, CRC(56cff3f1) SHA1(2a5c5ec5648b1d29768c808ab3c21118dc77fa27) )
ROM_END
ROM_START( g_jeod )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jeod.bin", 0x000000, 0x080000, CRC(25e2f9d2) SHA1(1817433ac701c407eb748e87f01a0bc8639e3ac2) )
ROM_END
ROM_START( g_jeos )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jeos.bin", 0x000000, 0x080000, CRC(13f924d2) SHA1(bc20e31c387ed091fe0288e86ea9b421063953bb) )
ROM_END
ROM_START( g_jgpf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jgpf.bin", 0x000000, 0x100000, CRC(e7f48d30) SHA1(fbcd0e7cb8dfc327b6d019afbdde9728f656957a) )
ROM_END
ROM_START( g_jewl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jewl.bin", 0x000000, 0x080000, CRC(cee98813) SHA1(9a6e4ca71546e798e1c98e78c4ab72aba46374c5) )
ROM_END
ROM_START( g_jmac )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jmac.bin", 0x000000, 0x100000, CRC(85bcc1c7) SHA1(d238ecdbc76affb0b92946a1ee984399b6e8fe27) )
ROM_END
ROM_START( g_jb11 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jb11.bin", 0x000000, 0x080000, CRC(4d3ddd7c) SHA1(4c3c6696157a3629f10aa60626f504cd64c36a58) )
ROM_END

ROM_START( g_kbox )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kbox.bin", 0x000000, 0x100000, CRC(9bdc230c) SHA1(c3f74c01e8124d5d08f817ff6e0c9416545b8302) )
ROM_END
ROM_START( g_kbou )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kbou.bin", 0x000000, 0x080000, CRC(aa68a92e) SHA1(32f90806f44a0bd1d65d84ceeb644681b9cee967) )
ROM_END
ROM_START( g_lbat )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lbat.bin", 0x000000, 0x080000, CRC(bbfaad77) SHA1(6ebef9c86779040bd2996564caa8631f4c41cf03) )
ROM_END
ROM_START( g_lhx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lhx.bin", 0x000000, 0x100000, CRC(70c3428d) SHA1(f23c6d0bc6daae11a3398d73961f621e508c9229) )
ROM_END
ROM_START( g_msbu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_msbu.bin", 0x000000, 0x100000, CRC(1a5d4412) SHA1(d7c075d98430f2864d9120d7c9f2efb92e8d350e) )
ROM_END
ROM_START( g_mhy )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mhy.bin", 0x000000, 0x080000, CRC(10bb359b) SHA1(5c8c600ca7468871b0ef301fd82a1909aa8b3b10) )
ROM_END
ROM_START( g_mmad )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmad.bin", 0x000000, 0x080000, CRC(79eba28a) SHA1(059e99fde8726a45a584007186913eb9a01f738e) )
ROM_END
ROM_START( g_mlem )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mlem.bin", 0x000000, 0x080000, CRC(f664eb6c) SHA1(34cb9c26030c5a0e66dc7de3302bb50633e4dbb6) )
ROM_END
ROM_START( g_mowe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mowe.bin", 0x000000, 0x080000, CRC(12ad6178) SHA1(574da95ab171b12546daa56dc28761c6bcc4a5fc) )
ROM_END
ROM_START( g_math )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_math.bin", 0x000000, 0x100000, CRC(d055a462) SHA1(119c2615db2607b8102ccb57d862ac6084e37c9d) )
ROM_END

ROM_START( g_mlo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mlo.bin", 0x000000, 0x100000, CRC(2148d56d) SHA1(e861551edabe55181ed1b7260169e953af903b5e) )
ROM_END
ROM_START( g_mloj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mloj.bin", 0x000000, 0x100000, CRC(a60d8619) SHA1(66e391c69dfbe7329654550ebf4464b4f426c5a0) )
ROM_END
ROM_START( g_mswi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mswi.bin", 0x000000, 0x100000, CRC(1ec66bf7) SHA1(6afc7c86dd9f03d24ca976437d1f70e742a7928d) )
ROM_END
ROM_START( g_mtrx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mtrx.bin", 0x000000, 0x080000, CRC(a0837741) SHA1(9e611f2a70fb2505a661d0906b535c484db99d0b) )
ROM_END
ROM_START( g_merc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_merc.bin", 0x000000, 0x100000, CRC(16113a72) SHA1(ab633913147325190ef13bd10f3ef5b06d3a9e28) )
ROM_END
ROM_START( g_mult )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mult.bin", 0x000000, 0x100000, CRC(30b512ee) SHA1(def06da570df4ff73df36591ef05cce6d409b950) )
ROM_END
ROM_START( g_midr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_midr.bin", 0x000000, 0x100000, CRC(187c6af6) SHA1(6cdd9083e2ff72cfb0099fd57a7f9eade9a74dda) )
ROM_END
ROM_START( g_m29 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_m29.bin", 0x000000, 0x100000, CRC(59ccabb2) SHA1(140245f688c42cb2f6bfd740785742286c71be65) )
ROM_END
ROM_START( g_mmg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmg.bin", 0x000000, 0x0c0000, CRC(f509145f) SHA1(52e7ade244d48bc282db003d87c408e96dcb3d85) )
ROM_END
ROM_START( g_mike )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mike.bin", 0x000000, 0x100000, CRC(6078b310) SHA1(e49b9eb91b2e951efe4509ae8a9a5a083afeb920) )
ROM_END


ROM_START( g_mlbb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mlbb.bin", 0x000000, 0x200000, CRC(14a8064d) SHA1(5a85c659db9dd7485ed1463a252f0941346aba24) )
ROM_END
ROM_START( g_mlbs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mlbs.bin", 0x000000, 0x100000, CRC(0deb79c2) SHA1(e223513d9bcecb49a6798720f3195dbd1c34681c) )
ROM_END
ROM_START( g_mono )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mono.bin", 0x000000, 0x080000, CRC(c10268da) SHA1(8ff34d2557270f91e2032cba100cd65eb51129ca) )
ROM_END
ROM_START( g_mahb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mahb.bin", 0x000000, 0x100000, CRC(b638b6a3) SHA1(caad1bc67fd3d7f7add0a230def4a0d5a7f196ef) )
ROM_END
ROM_START( g_mysd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mysd.bin", 0x000000, 0x080000, CRC(50fd5d93) SHA1(aec8aefa11233699eacefdf2cf17d62f68cbdd98) )
ROM_END
ROM_START( g_mysf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mysf.bin", 0x000000, 0x080000, CRC(b2f2a69b) SHA1(b8a602692e925d7e394afacd3269f2472d03635c) )
ROM_END
ROM_START( g_nhir )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nhir.bin", 0x000000, 0x200000, CRC(1233a229) SHA1(e7083aaa4a4f6539b10b054aa4ecdcee52f3f8cc) )
ROM_END
ROM_START( g_nobu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nobu.bin", 0x000000, 0x080000, CRC(b9bc07bc) SHA1(394c6b46a9e9b9177a135fd8fd43392eb1099666) )
ROM_END
ROM_START( g_opeu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_opeu.bin", 0x000000, 0x100000, CRC(e7cba1d8) SHA1(e2e484c6db2bb058d04288b4dfea0ed199108d24) )
ROM_END
ROM_START( g_pto )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pto.bin", 0x000000, 0x100000, CRC(d9d4c6e2) SHA1(41d1d1956104133388b4ea69792fccca8013524a) )
ROM_END

ROM_START( g_pbgl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pbgl.bin", 0x000000, 0x200000, CRC(95823c43) SHA1(c9971710651056b8211b9bbd68eb5d1569e39e1f) )
ROM_END
ROM_START( g_phel )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_phel.bin", 0x000000, 0x080000, CRC(11c79320) SHA1(c2e34b7eb411ac78bb480e30434808b2ae4989ea) )
ROM_END
ROM_START( g_path )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_path.bin", 0x000000, 0x100000, CRC(b41b77cf) SHA1(d371e339c5d85b69c34007dc514c1adb524dac2a) ) // raster
ROM_END
ROM_START( g_prho )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_prho.bin", 0x000000, 0x080000, CRC(9dcdc894) SHA1(40166e9c2b40f35ba7d7cf5d07865c824c323757) )
ROM_END
ROM_START( g_prqu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_prqu.bin", 0x000000, 0x100000, CRC(cc8b2b69) SHA1(a878d6149a3d5c4ccca7741af45659f753b32ed7) ) // raster
ROM_END
ROM_START( g_ramp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ramp.bin", 0x000000, 0x080000, CRC(9c4dd057) SHA1(9dd4ee99e2ccf8759446d355584a9cbf685a3d8b) )
ROM_END
ROM_START( g_rast )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rast.bin", 0x000000, 0x080000, CRC(c7ee8965) SHA1(adec2cb70344ffc720ecdfeb68712bb3d1d0fd9c) )
ROM_END
ROM_START( g_rsbt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rsbt.bin", 0x000000, 0x200000, CRC(7bb60c3f) SHA1(42e5ac265e3bf82c44dca70deb71b0aa08acf1a9) )
ROM_END
ROM_START( g_rmmw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rmmw.bin", 0x000000, 0x200000, CRC(85c956ef) SHA1(a435ac53589a29dbb655662c942daab425d3f6bd) )
ROM_END
ROM_START( g_robw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robw.bin", 0x000000, 0x100000, CRC(ef02d57b) SHA1(a70e83f25ff8ae8b0590920531558faa61738d0f) )
ROM_END

ROM_START( g_r3k2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_r3k2.bin", 0x000000, 0x100000, CRC(3d842478) SHA1(89fc2203b0369565124184690a5a5037ac9c54d7) )
ROM_END
ROM_START( g_r3k3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_r3k3.bin", 0x000000, 0x140000, CRC(7e41c8fe) SHA1(a47b0ac13ef8229fbfc89216c8ecb66d72f6f73a) )
ROM_END
ROM_START( g_sswo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sswo.bin", 0x000000, 0x080000, CRC(44f66bff) SHA1(53e376faed7fe20c5ffe78a568dd8d2cf3fe2d1a) )
ROM_END
ROM_START( g_sen )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sen.bin", 0x000000, 0x080000, CRC(f9b396b8) SHA1(2ee1ad4aac354b6d227b16976f0875f4625ff604) )
ROM_END
ROM_START( g_sbls )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sbls.bin", 0x000000, 0x080000, CRC(713d377b) SHA1(8485453264fd67ff8e4549a9db50b4f314a6fcb5) )
ROM_END
ROM_START( g_sdnk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sdnk.bin", 0x000000, 0x200000, CRC(cdf5678f) SHA1(7161a12c2a477cff8e29fa51403eea12c03180c7) )
ROM_END
ROM_START( g_sspo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sspo.bin", 0x000000, 0x0a0000, CRC(af9f9d9c) SHA1(9734c8a4e2beb6e6bb2d14ce0d332825537384e0)  )
ROM_END
ROM_START( g_sks1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sks1.bin", 0x000000, 0x280000, CRC(e01f6ed5) SHA1(a3084262f5af481df1a5c5ab03c4862551a53c91) )
ROM_END
ROM_START( g_scon )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_scon.bin", 0x000000, 0x180000, CRC(8e2bceaf) SHA1(108e00cdbcb339834b54fb11819591627fe4b351) )
ROM_END
ROM_START( g_sfli )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sfli.bin", 0x000000, 0x100000, CRC(1217dbea) SHA1(41722d8c0b4865ca8044fb53a818b13eded758fc) )
ROM_END

ROM_START( g_ssma )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ssma.bin", 0x000000, 0x080000, CRC(b1dedfad) SHA1(a9cb2295b9cd42475904cba19b983e075a1b6014) ) // raster
ROM_END
ROM_START( g_sumc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sumc.bin", 0x000000, 0x200000,  CRC(d7d53dc1) SHA1(28d18ba3d2ac3f8fece0bf652e87d491c565b8df) )
ROM_END
ROM_START( g_shyd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shyd.bin", 0x000000, 0x080000, CRC(1335ddaa) SHA1(5ff65139c7e10539dd5a12bdf56073504c998471) )
ROM_END
ROM_START( g_sl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sl.bin", 0x000000, 0x080000, CRC(ea13cb1d) SHA1(9e975ba4c396617ff9535d51d0929f6d592478fc) )
ROM_END
ROM_START( g_sl91 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sl91.bin", 0x000000, 0x080000, CRC(a948ab7e) SHA1(9d446471b8fe37b08dc7c4a99de22de9d56a1d16) )
ROM_END
ROM_START( g_smas )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smas.bin", 0x000000, 0x080000, CRC(088ba825) SHA1(662a87d679066aad0638b2ffb01c807d9f376121) )
ROM_END
ROM_START( g_supm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_supm.bin", 0x000000, 0x100000, CRC(543a5869) SHA1(ca14653589fd36e6394ec99f223ed9b18f70fd6a) )
ROM_END
ROM_START( g_swso )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_swso.bin", 0x000000, 0x080000, CRC(9cb8468f) SHA1(088b81c3bcda86b9803b7e3f8067beb21d1d2553) )
ROM_END
ROM_START( g_swve )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_swve.bin", 0x000000, 0x0a0000, CRC(ea1bc9ab) SHA1(c18fc75e0c5fa0e98c8664903e978ec4f73ef5d2) )
ROM_END
ROM_START( g_tusa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tusa.bin", 0x000000, 0x100000, CRC(a0caf97e) SHA1(8d8d833dfc88663408bd7cf9fb821608ad2bef3d) )
ROM_END

ROM_START( g_tcls )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tcls.bin", 0x000000, 0x100000, CRC(4e65e483) SHA1(686afbd7130fe8487d9126e690bf53800ae953ba) )
ROM_END
ROM_START( g_tcop )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tcop.bin", 0x000000, 0x080000, CRC(7459ad06) SHA1(739421dd98d8073030d43cf8f50e411acb82f7d6) )
ROM_END
ROM_START( g_tc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tc.bin", 0x000000, 0x080000, CRC(e889e218) SHA1(e78f42104c66f49e0492e65c64354baac599369e) ) // illegal dma
ROM_END
ROM_START( g_tsb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsb.bin", 0x000000, 0x100000, CRC(227a1178) SHA1(f2ecc7b32cef29f22eb4a21a22be122bd4bad212) ) // noboot
ROM_END
ROM_START( g_tsbw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsbw.bin", 0x000000, 0x100000, CRC(21f27d34) SHA1(8d34ffac312caeac853876415c74ab6fe63d8dc2) )
ROM_END
ROM_START( g_tbw2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tbw2.bin", 0x000000, 0x200000, CRC(0a0e67d8) SHA1(5fadb2a0e780ec868671b0e888fad5d7c203f59f) )
ROM_END
ROM_START( g_tbw3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tbw3.bin", 0x000000, 0x200000, CRC(aae4089f) SHA1(d5d1609cdf72d98f5e5daa47a9585ae7ca87a410) )
ROM_END
ROM_START( g_tsh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsh.bin", 0x000000, 0x100000, CRC(5f86ddc9) SHA1(4d4fd22d2fafd7e56790029be9b02e61995df11c) )
ROM_END
ROM_START( g_tsnb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsnb.bin", 0x000000, 0x100000, CRC(53913991) SHA1(ac7aa724d6464fbd8e3144a49f3821ff6e42f67a) ) // lockup
ROM_END
ROM_START( g_tw92 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tw92.bin", 0x000000, 0x040000, CRC(5e93c8b0) SHA1(ebab8f8b4f25aae44900d266c674621db2a831d9) )
ROM_END

ROM_START( g_tw93 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tw93.bin", 0x000000, 0x040000, CRC(caf8eb2c) SHA1(53e47c40ac550c334147482610b3465fe9f6a535) )
ROM_END
ROM_START( g_ttnk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ttnk.bin", 0x000000, 0x100000, CRC(1a406299) SHA1(e714e9faa9c1687a2dfcb0ada22a75c7a4ee01a6) )
ROM_END
ROM_START( g_tlbb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tlbb.bin", 0x000000, 0x080000, CRC(4fb50304) SHA1(dde1b2465f54f4f6f485c1ba063ff47c5d1baf27) )
ROM_END
ROM_START( g_tr95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tr95.bin", 0x000000, 0x200000, CRC(3f848a92) SHA1(0e73742113aa3f0aa5b010bb847569589cd3a5b0) )
ROM_END
ROM_START( g_trbb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_trbb.bin", 0x000000, 0x100000, CRC(24629c78) SHA1(0e3f7ebf8661cac9bd7a0c5af64f260e7b5f0a0b) )
ROM_END
ROM_START( g_totf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_totf.bin", 0x000000, 0x200000, CRC(8360b66a) SHA1(271b13e1c0697d17ad702e97297ed1ea09ddb53b) )
ROM_END
ROM_START( g_toxi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_toxi.bin", 0x000000, 0x080000, CRC(11fd46ce) SHA1(fd83f309b6d4261f0c98ba97a8627cfb5212093b) )
ROM_END
ROM_START( g_tanf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tanf.bin", 0x000000, 0x200000, CRC(015f2713) SHA1(820efb4a4d3d29036911d9077bb6c0a4ce7f36d4) )
ROM_END
ROM_START( g_uman )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_uman.bin", 0x000000, 0x080000, CRC(83b4d5fb) SHA1(7af1b66555c636664a73ca2091ef92ac800dc5d8) )
ROM_END
ROM_START( g_ur95 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ur95.bin", 0x000000, 0x200000, CRC(9920e7b7) SHA1(7e100cb56c30498c1fee3867ff3612567287b656) )
ROM_END

ROM_START( g_vapt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vapt.bin", 0x000000, 0x100000, CRC(c49e3a0a) SHA1(d8eb087eeda31f202e6a1a0c4de891ced162abc5) )
ROM_END
ROM_START( g_v5 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_v5.bin", 0x000000, 0x100000, CRC(ad9d0ec0) SHA1(ca344521cb5015d142bdbce0eb44cea050b8e86b) )
ROM_END
ROM_START( g_volf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_volf.bin", 0x000000, 0x040000, CRC(b0c5e3f7) SHA1(7de808400f64dd0e4f468049fff7d29212ea0215) )
ROM_END
ROM_START( g_wack )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wack.bin", 0x000000, 0x100000, CRC(8af4552d) SHA1(e331c57ce6a176ab9ff1461e9423514756c5558d) )
ROM_END
ROM_START( g_wloc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wloc.bin", 0x000000, 0x200000, CRC(0a46539b) SHA1(b64f3d0fa74ec93782b4c0441653d72b675e23a7) ) // bad gfx
	ROM_LOAD( "g_wloc.bin", 0x200000, 0x200000, CRC(0a46539b) SHA1(b64f3d0fa74ec93782b4c0441653d72b675e23a7) ) // bad gfx
ROM_END
ROM_START( g_wrom )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wrom.bin", 0x000000, 0x100000,  CRC(5be10c6a) SHA1(d75eb583e7ec83d6b8308f6dc7cdb31c62b4dbf9) )
ROM_END
ROM_START( g_wars )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wars.bin", 0x000000, 0x080000, CRC(4b680285) SHA1(9b13a85f39b3f4cc31f54077df29bbe812405a08) )
ROM_END
ROM_START( g_wgas )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wgas.bin", 0x000000, 0x200000, CRC(c2c13b81) SHA1(0b068f684e206139bcd592daba4613cbf634dd56) )
ROM_END
ROM_START( g_wfor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wfor.bin", 0x000000, 0x080000, CRC(c8d8efc3) SHA1(ab00e1caa8c86a37e7766b3102d02d433f77470e) )
ROM_END
ROM_START( g_winc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_winc.bin", 0x000000, 0x100000, CRC(f57c7068) SHA1(6c948c98bbf52b849ffa1920e127c74ae04d75d1) )
ROM_END

ROM_START( g_wb3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wb3.bin", 0x000000, 0x080000, CRC(c24bc5e4) SHA1(1fd3f77a2223ebeda547b81e49f3dfc9d0197439) )
ROM_END
ROM_START( g_wbmw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wbmw.bin", 0x000000, 0x0c0000, CRC(1592f5b0) SHA1(87a968f773c7e807e647c0737132457b06b78276) )
ROM_END
ROM_START( g_wcl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wcl.bin", 0x000000, 0x080000, CRC(53434bab) SHA1(db53b7c661392b8558d0eb1f1dd51c8a0a17fa4e) )
ROM_END
ROM_START( g_wcs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wcs.bin", 0x000000, 0x040000, CRC(bf272bcb) SHA1(9da376f266c98c93ae387bec541c513fec84a0e9) )
ROM_END
ROM_START( g_wc90 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wc90.bin", 0x000000, 0x040000, CRC(dd95f829) SHA1(c233313214418300a39afc446e8426cc11f99c6c) )
ROM_END
ROM_START( g_wc94 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wc94.bin", 0x000000, 0x100000, CRC(0171b47f) SHA1(af0e8fada3db7e746aef2c0070deb19602c6d32a) )
ROM_END
ROM_START( g_wwar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wwar.bin", 0x000000, 0x080000, CRC(2d162a85) SHA1(1eac51029fd7fb1da1c5546cbd959220244cf3e5) )
ROM_END

ROM_START( g_cwcs )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cwcs.bin", 0x000000, 0x100000, CRC(883e33db) SHA1(2c072aa60c8cec143f5626fde4b704fd13a5f845) )
ROM_END
ROM_START( g_cbwl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cbwl.bin", 0x000000, 0x080000, CRC(1bf92520) SHA1(ffb469106bc7f475cd89d717035a54cab149c818) )
ROM_END
ROM_START( g_cpam )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cpam.bin", 0x000000, 0x040000, CRC(b496de28) SHA1(6e3cc6e97d33890996dcbad9a88b1aef361ca2d9) )
ROM_END
ROM_START( g_cdor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cdor.bin", 0x000000, 0x0c0000, CRC(21283b14) SHA1(8cc2a28309b2dc61da68c77d8a6545af05660e8b) )
ROM_END
ROM_START( g_ckid )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ckid.bin", 0x000000, 0x100000,  CRC(50217c80) SHA1(237ff4041f3e8ce5047f06f695fb55dca51354b8) )
ROM_END
ROM_START( g_chi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chi.bin", 0x000000, 0x100000, CRC(9c3973a4) SHA1(d5e3a698b780c8f67def5f6df87b94d5419beefc) )
ROM_END
ROM_START( g_bglf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bglf.bin", 0x000000, 0x080000, CRC(4aa03e4e) SHA1(84db25ff7a0d2db6afddcfe4cdc7c26e20d0dd72) )
ROM_END
ROM_START( g_baha )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_baha.bin", 0x000000, 0x080000, CRC(b1e268da) SHA1(cee49b613298e060d938de523dfcbb27e790b5af) )
ROM_END
ROM_START( g_awld )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_awld.bin", 0x000000, 0x100000,  CRC(e9742041) SHA1(9d98d6817b3e3651837bb2692f7a2a60a608c055) )
ROM_END
ROM_START( g_airm )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_airm.bin", 0x000000, 0x100000, CRC(b3db0c71) SHA1(b4e5752453cb3e4b3f6cb4fd9ad6473b4b8addd4) )
ROM_END

ROM_START( g_airm2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_airm2.bin", 0x000000, 0x100000, CRC(4582817b) SHA1(6ed89fc3302023bc95c2bba0d45fe1d30e1c5d86) )
ROM_END
ROM_START( g_aoki )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aoki.bin", 0x000000, 0x140000, CRC(10be1d93) SHA1(0de0f798f636285da2b4d248f9894bf975b45304) )
ROM_END
ROM_START( g_advdai )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_advdai.bin", 0x000000, 0x100000,  CRC(e0d5e18a) SHA1(842430c0465d810576e75851023929fec873580d) )
ROM_END
ROM_START( g_blueal )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blueal.bin", 0x000000, 0x100000, CRC(7222ebb3) SHA1(0be0d4d3e192beb4106c0a95c1fb0aec952c3917) )
ROM_END
ROM_START( g_chav2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chav2.bin", 0x000000, 0x100000, CRC(5bc0dbb8) SHA1(5df09bfd8523166d5ad514eaeaf4f4dc76a8a06d) )
ROM_END
ROM_START( g_chibi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chibi.bin", 0x000000, 0x080000, CRC(91a144b8) SHA1(39d4da607d020bd20063c34b0a617a5e6cb7c5aa) )
ROM_END
ROM_START( g_crayon )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crayon.bin", 0x000000, 0x200000, CRC(97fc42d2) SHA1(46214898990ebc5e74413a53f7304ad466875fbe) )
ROM_END
ROM_START( g_konsen )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_konsen.bin", 0x000000, 0x100000, CRC(05cc7369) SHA1(9f205fc916df523ec84e0defbd0f1400a495cf8a) ) // general chaos
ROM_END
ROM_START( g_dahna )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dahna.bin", 0x000000, 0x100000, CRC(4602584f) SHA1(da0246a063e6f70933e251b9ae84587fe620d4f0) )
ROM_END
ROM_START( g_daikou )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_daikou.bin", 0x000000, 0x100000, CRC(5a652458) SHA1(a4552b23079b161da9ad47ac7cb9c4ecb3731967) )
ROM_END

ROM_START( g_discol )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_discol.bin", 0x000000, 0x100000, CRC(adfde883) SHA1(991e22d7f258cba94306d16d5dd37172a34da9d3) )
ROM_END
ROM_START( g_dslay )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dslay.bin", 0x000000, 0x200000, CRC(01bc1604) SHA1(f67c9139bbc93f171e274a5cd3fba66480cd8244) ) // noboot
ROM_END
ROM_START( g_dslay2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dslay2.bin", 0x000000, 0x200000, CRC(46924dc3) SHA1(79b6201301acb5d9e9c56dcf65a9bcf9d9a931ab) ) // noboot
ROM_END
ROM_START( g_draxos )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_draxos.bin", 0x000000, 0x100000, CRC(1ea07af2) SHA1(9652b8ed0e36ee17ef6c0b007ca3ad237c13a7a0) ) // ea clone..
ROM_END
ROM_START( g_eadoub )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_eadoub.bin", 0x000000, 0x100000, CRC(a0b54cbc) SHA1(499047852b5892fcdaca191a5aab19257d6a85a8) )
ROM_END
ROM_START( g_europa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_europa.bin", 0x000000, 0x100000, CRC(b0416c60) SHA1(d382f977ea3071f133a947ceb3528904e72f9bc9) )
ROM_END
ROM_START( g_exranz )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_exranz.bin", 0x000000, 0x100000, CRC(349bb68d) SHA1(0006f55e826148cff9e717b582a39a04adf100df) )
ROM_END
ROM_START( g_fushig )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fushig.bin", 0x000000, 0x100000,  CRC(4762062a) SHA1(bbcd26d5d1f1422051467aacaa835e1f38383f64) )
ROM_END
ROM_START( g_ftbhb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ftbhb.bin", 0x000000, 0x400000, CRC(863e0950) SHA1(9c978aaab10e16be59558561b07a0c610c74b43e) ) // noboot
ROM_END
ROM_START( g_gameno )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gameno.bin", 0x000000, 0x300000, CRC(cdad7e6b) SHA1(31c66bd13abf4ae8271c09ec5286a0ee0289dbbc) ) // BANKING?
ROM_END

ROM_START( g_gemfi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gemfi.bin", 0x000000, 0x100000, CRC(3d36135b) SHA1(98da5fdec3147edb75210ccf662601e502e23c31) )
ROM_END
ROM_START( g_gaunt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gaunt.bin", 0x000000, 0x100000, CRC(f9872055) SHA1(30208982dd1f50634943d894e7458a556127f8e4) )
ROM_END
ROM_START( g_ghwor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ghwor.bin", 0x000000, 0x200000, CRC(6e3621d5) SHA1(4ef7aec80003aab0e1d2260b7fc0d22d63d90038) )
ROM_END
ROM_START( g_hssoc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hssoc.bin", 0x000000, 0x080000, CRC(f49c3a86) SHA1(d865b01e58a269400de369fc1fbb3b3e84e1add0) )
ROM_END
ROM_START( g_hokuto )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hokuto.bin", 0x000000, 0x080000, CRC(1b6585e7) SHA1(7d489e1087a0816e2091261f6550f42a61474915) )
ROM_END
ROM_START( g_hybrid )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hybrid.bin", 0x000000, 0x200000,  CRC(a1f1cfe7) SHA1(ed592c78ef60d91a6c5723d11cd553d3798524e1) )
ROM_END
ROM_START( g_ichir )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ichir.bin", 0x000000, 0x200000, CRC(7bdec762) SHA1(2e43ea1870dd3352e3c153373507554d97d51edf) )
ROM_END
ROM_START( g_hyokk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hyokk.bin", 0x000000, 0x080000, CRC(72253bdb) SHA1(32a73559f84dbab5460bfd1e266acfc8f4391ae4) ) // no start?
ROM_END
ROM_START( g_janout )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_janout.bin", 0x000000, 0x100000, CRC(b5ef97c6) SHA1(4f307e0146e944fbbd4537f5cdc5da136204fc9b) )
ROM_END
ROM_START( g_juju )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_juju.bin", 0x000000, 0x080000, CRC(d09b1ef1) SHA1(1a94cde6392d385271797a6b21ad0eaad920a8da) )
ROM_END

ROM_START( g_juuo00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_juuo00.bin", 0x000000, 0x080000, CRC(1b7c96c0) SHA1(bf5782b6f25dcf2bd1ff987e2821441c77da3c5d) )
ROM_END
ROM_START( g_juuo01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_juuo01.bin", 0x000000, 0x080000, CRC(b2233e87) SHA1(d0713d6da20145fe9e13b56a103fce23bc6051d7) )
ROM_END
ROM_START( g_kishi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kishi.bin", 0x000000, 0x180000, CRC(22e1f04a) SHA1(2884f79b8f717fc9e244dac0fb441bdc44c68203) )
ROM_END
ROM_START( g_kyuuk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kyuuk.bin", 0x000000, 0x080000, CRC(de48dce3) SHA1(40e771ace8f89e40d3315be698aa68effd617c5c) )
ROM_END
ROM_START( g_langr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_langr.bin", 0x000000, 0x080000, CRC(b6ea5016) SHA1(cc67c5a3b91e706b495eb561a95a038fff72b5da) )
ROM_END
ROM_START( g_lngr2a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lngr2a.bin", 0x000000, 0x200000, CRC(7f891dfc) SHA1(4bbc2502784a61eedf45eca5303dc68062964ff4) )
ROM_END
ROM_START( g_lngr2c )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lngr2c.bin", 0x000000, 0x200000, CRC(4967c9f9) SHA1(167944091348c89ce43dfa4854f8a51ed7276dde) )
ROM_END
ROM_START( g_librty )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_librty.bin", 0x000000, 0x200000, CRC(2adb0364) SHA1(c0ddfc2149cd84fbb0c5860b98c3a16f6000b85e) )
ROM_END
ROM_START( g_m1tank )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_m1tank.bin", 0x000000, 0x080000, CRC(1e2f74cf) SHA1(65248727b0b52106007ec1193832f16545db5378) )
ROM_END
ROM_START( g_madou )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_madou.bin", 0x000000, 0x200000, CRC(dd82c401) SHA1(143456600e44f543796cf6ade77830115a8f2f99) )
ROM_END

ROM_START( g_mahcop )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mahcop.bin", 0x000000, 0x040000, CRC(1ccbc782) SHA1(7fd5866347067e6111739833595278e192d275fb) ) // crash(!)
ROM_END
ROM_START( g_maoure )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_maoure.bin", 0x000000, 0x080000, CRC(24a7f28c) SHA1(56253adf20d420233722d428170a646262be226f) ) // mystic fighter clone
ROM_END
ROM_START( g_maten )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_maten.bin", 0x000000, 0x100000, CRC(b804a105) SHA1(31bc44b019310e18174eb5e1a6d8e7d351103e4e) )
ROM_END
ROM_START( g_manser )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_manser.bin", 0x000000, 0x080000, CRC(08ece367) SHA1(d353e673bd332aa71370fa541e6fd26d918b5a2b) )
ROM_END
ROM_START( g_megaq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_megaq.bin", 0x000000, 0x100000, CRC(9d4b447a) SHA1(3f82b2f028345b6fc84801fa38d70475645d6aa2) )
ROM_END
ROM_START( g_mfang )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mfang.bin", 0x000000, 0x080000, CRC(a8df1c4c) SHA1(489e81d2ac81810d571829e8466374f242e81621) )
ROM_END
ROM_START( g_minnie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_minnie.bin", 0x000000, 0x200000, CRC(5aa0f3a0) SHA1(0341c231c268b61add922c5c3bf5ab2f9dfbf88f) )
ROM_END
ROM_START( g_minato )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_minato.bin", 0x000000, 0x100000, CRC(bd89fd09) SHA1(dea227a41a5ba28f8c8ea75cba12965bbc5ff8da) )
ROM_END
ROM_START( g_mpiano )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mpiano.bin", 0x000000, 0x080000, CRC(a719542e) SHA1(5141cb7cc03bc087c17cc663ea3cd889e6faa16c) )
ROM_END
ROM_START( g_monwr4 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_monwr4.bin", 0x000000, 0x200000, CRC(36a3aaa4) SHA1(46ba5e8775a2223fe5056f54555d9caa7d04f4e1) )
ROM_END

ROM_START( g_mlfoot )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mlfoot.bin", 0x000000, 0x100000, CRC(dce29c9d) SHA1(6f8638a1c56229ddcb71c9da9f652b49c2978f44) )
ROM_END
ROM_START( g_new3dg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_new3dg.bin", 0x000000, 0x180000, CRC(d2a9bf92) SHA1(d023f9fc5d7c7f2873a5bf79f6035111b78cdd5d) )
ROM_END
ROM_START( g_nikkan )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nikkan.bin", 0x000000, 0x080000, CRC(c3655a59) SHA1(d29f7e70feae3b4f1b423026b7a124514ee48dcf) )
ROM_END
ROM_START( g_nobbus )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nobbus.bin", 0x000000, 0x100000,  CRC(30bf8637) SHA1(1ed50cae48405ec8ad9257df3d445521e60636d8) )
ROM_END
ROM_START( g_nobzen )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nobzen.bin", 0x000000, 0x080000, CRC(1381b313) SHA1(a1f64afbf3bcdc85b61bce96454105128ca6746a) )
ROM_END
ROM_START( g_noblor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_noblor.bin", 0x000000, 0x200000, CRC(96c01fc6) SHA1(9246a29b25e276dc1f7edfae87229549cf87d92e) )
ROM_END
ROM_START( g_patlab )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_patlab.bin", 0x000000, 0x080000,  CRC(21a0e749) SHA1(5595422530e6891042a4a005d11b79af7f09fe9b) )
ROM_END
ROM_START( g_psyobl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_psyobl.bin", 0x000000, 0x0a0000, CRC(8ba7e6c5) SHA1(274d98e0c04caed378f1dd489dc3741b9ad13a88) )
ROM_END
ROM_START( g_ragna )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ragna.bin", 0x000000, 0x200000, CRC(6a3f5ae2) SHA1(e0df9f3cb64beb3ea921653eccdd45aca6abc0aa) ) // crusade japan version
ROM_END
ROM_START( g_ransei )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ransei.bin", 0x000000, 0x100000, CRC(a9a0083d) SHA1(7c431e26226b99d09d61c4dc64131b2d81ae870e) )
ROM_END

ROM_START( g_renthe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_renthe.bin", 0x000000, 0x100000, CRC(2e515f82) SHA1(a07e4d56c9842e1177df9016fb9478060dc4c1c6) )
ROM_END
ROM_START( g_robo3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robo3.bin", 0x000000, 0x080000, CRC(34fb7b27) SHA1(7860eb700d85801831ea14501b47bcaa1753c9fc) )
ROM_END
ROM_START( g_roybld )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_roybld.bin", 0x000000, 0x100000,  CRC(0e0107f1) SHA1(914f684f085927257020ffaa1ca536ec057e1603) )
ROM_END
ROM_START( g_sagia )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sagia.bin", 0x000000, 0x100000, CRC(f1e22f43) SHA1(0aa2632da5ec3c1db21b273bcb8630d84b7bb805) )
ROM_END
ROM_START( g_sango2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sango2.bin", 0x000000, 0x100000, CRC(437ba326) SHA1(d9a912caacaa476a890564f62b6ce9cc8f60496e) )
ROM_END
ROM_START( g_sango3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sango3.bin", 0x000000, 0x180000, CRC(a8de6aea) SHA1(c85d03a6bcfeccc66ea3fa3cbda006c83576815f) )
ROM_END
ROM_START( g_sangor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sangor.bin", 0x000000, 0x100000, CRC(0f56785a) SHA1(9b79f91060e68b9f6f48c8a5f38bc9873d03c4a9) )
ROM_END
ROM_START( g_shikin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shikin.bin", 0x000000, 0x080000, CRC(5ea0c97d) SHA1(34add3f084636d671b48e77a771329f23f8083cd) )
ROM_END
ROM_START( g_shiten )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shiten.bin", 0x000000, 0x080000, CRC(7e729693) SHA1(7bc3be0753b4ba8bbab2f5096e0efa0c0884dd98) ) // shadow blasters
ROM_END
ROM_START( g_shogi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shogi.bin", 0x000000, 0x040000, CRC(4148f816) SHA1(097fc6e7b7d9152f538d973a1837c09b711bf9d2) )
ROM_END

ROM_START( g_shura )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shura.bin", 0x000000, 0x100000, CRC(e19da6e5) SHA1(22b58751fe7320c53a49e7c2c3e72b0500192d58) )
ROM_END
ROM_START( g_slapf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_slapf.bin", 0x000000, 0x100000, CRC(d6695695) SHA1(3cb992c70b4a5880a3e5f89211f29fe90486c0e0) )
ROM_END
ROM_START( g_soldf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soldf.bin", 0x000000, 0x180000, CRC(a84d28a1) SHA1(619faee3d78532478aad405db335d00fb93e6850) )
ROM_END
ROM_START( g_sorckd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sorckd.bin", 0x000000, 0x100000, CRC(bb1fc9ce) SHA1(87759abb603f1f97c2e136682dc78eea545338ce) )
ROM_END
ROM_START( g_stalon )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stalon.bin", 0x000000, 0x080000, CRC(10e4ec63) SHA1(b4f0a13646c13911be5550103301af25827fcd0c) )
ROM_END
ROM_START( g_sthor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sthor.bin", 0x000000, 0x300000, CRC(1110b0db) SHA1(178ef742dad227d4128fa81dddb116bad0cabe1d) )
ROM_END
ROM_START( g_sthorj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sthorj.bin", 0x000000, 0x300000, CRC(4f39783c) SHA1(54296f5cf1917c568bb29b0086641c282b8884bd) )
ROM_END
ROM_START( g_supdai )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_supdai.bin", 0x000000, 0x080000,  CRC(d50a166f) SHA1(56d9366b50cea65b16ed621b9a5bf355ef89e6b5) )
ROM_END
ROM_START( g_surgin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_surgin.bin", 0x000000, 0x200000, CRC(65ac1d2b) SHA1(9f14cd11cfa499cdd58248de81db30f9308326e5) )
ROM_END
ROM_START( g_sydval )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sydval.bin", 0x000000, 0x080000, CRC(37dc0108) SHA1(36e010f16791816108d395fce39b39ab0a49268c) )
ROM_END

ROM_START( g_taiga )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_taiga.bin", 0x000000, 0x100000, CRC(09fbb30e) SHA1(67358048fc9bc93d8835af852bc437124e015a61) )
ROM_END
ROM_START( g_taikou )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_taikou.bin", 0x000000, 0x140000, CRC(f96fe15b) SHA1(a96d6492d4e89687d970bc010eb0b93ee2481a44) )
ROM_END
ROM_START( g_teitok )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_teitok.bin", 0x000000, 0x100000, CRC(9b08e4e4) SHA1(adc4c03636dbd8ca5449a8a66c6a7b7ef281893a) )
ROM_END
ROM_START( g_telmj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_telmj.bin", 0x000000, 0x040000, CRC(44817e92) SHA1(8edaec4944c4b9d876601ee5f8921247a5ffe057) )
ROM_END
ROM_START( g_telstd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_telstd.bin", 0x000000, 0x080000, CRC(54cf8c29) SHA1(2924ed0b4266edddbb981f97acb93bbdf90494e6) )
ROM_END
ROM_START( g_tpglf2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tpglf2.bin", 0x000000, 0x100000, CRC(b8ce98b3) SHA1(95a8918c98420fdfd59a4cb0be8fd5ec69d3593e) )
ROM_END
ROM_START( g_tpglf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tpglf.bin", 0x000000, 0x100000, CRC(62bad606) SHA1(1663289933b37526e4e07a6ee7fcd5e6cc2b489a) )
ROM_END
ROM_START( g_traysi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_traysi.bin", 0x000000, 0x100000, CRC(96184f4f) SHA1(ff0efe6da308919f843a7593e8af7fae82160b0b) ) // eng ver of..?
ROM_END
ROM_START( g_twintl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_twintl.bin", 0x000000, 0x100000, CRC(d757f924) SHA1(5377e96b4cb14038675c41d165f1d92ae067cd9b) )
ROM_END
ROM_START( g_uw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_uw.bin", 0x000000, 0x100000, CRC(4edaec59) SHA1(a76cf7dd06784cba15fa0c3be0ae92cba71ccade) )
ROM_END

ROM_START( g_verytx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_verytx.bin", 0x000000, 0x080000, CRC(bafc375f) SHA1(420780933e34da0f9b2a22b6bbb0739e363aab3a) )
ROM_END
ROM_START( g_vix357 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vix357.bin", 0x000000, 0x100000, CRC(3afa2d7b) SHA1(460037301df0d67947bd17eddb38a3011896cb43) )
ROM_END
ROM_START( g_waiala )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_waiala.bin", 0x000000, 0x180000, CRC(cbe2c1f6) SHA1(5d78c5cf3a514275a7df6d9fdbd708584b42e697) )
ROM_END
ROM_START( g_warps )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_warps.bin", 0x000000, 0x200000, CRC(143697ed) SHA1(a51564e9cf60fd5df6a7c00cdbbaa310cfa48d19) )
ROM_END
ROM_START( g_wrom2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wrom2.bin", 0x000000, 0x100000, CRC(cd8c472a) SHA1(fe4e3684212f1e695bdf4a4c41999fac773259f4) )
ROM_END
ROM_START( g_wboy5 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wboy5.bin", 0x000000, 0x0a0000, CRC(45a50f96) SHA1(1582f159e1969ff0541319a9bd7e6f7a53505d01) )
ROM_END
ROM_START( g_wonlib )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wonlib.bin", 0x000000, 0x080000, CRC(9350e754) SHA1(57b97d9ddecfa2e9a75c4a05cd2b7e821210155a) ) // noboot
ROM_END
ROM_START( g_wresbl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wresbl.bin", 0x000000, 0x080000, CRC(d563e07f) SHA1(bb6e6cd4f80ad69265b8c0d16b7581c629fd1770) )
ROM_END
ROM_START( g_xdaze )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xdaze.bin", 0x000000, 0x080000, CRC(ab22d002) SHA1(5c700cd69964645f80a852d308d0f0de22d88f83) )
ROM_END
ROM_START( g_xmen )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xmen.bin", 0x000000, 0x100000, CRC(f71b21b4) SHA1(2f2b5d018c98b78faf9ab6b172947f5cd65d5cf0) )
ROM_END
ROM_START( g_yuyuga )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yuyuga.bin", 0x000000, 0x200000, CRC(7dc98176) SHA1(274b86709b509852ca004ceaa244f3ae1d455c50) )
ROM_END
ROM_START( g_zanya )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zanya.bin", 0x000000, 0x080000, CRC(637fe8f3) SHA1(f1ea9a88233b76e3df32526be0b64fb653c13b7d) )
ROM_END

ROM_START( g_aresha )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aresha.bin", 0x000000, 0x080000,  CRC(3d45de4f) SHA1(d0d0aaad68978fd15c5857773141ae639de1abc7) )
ROM_END
ROM_START( g_asolde )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_asolde.bin", 0x000000, 0x200000, CRC(0496e06c) SHA1(fa141778bd6540775194d77318f27d2a934e1ac1) )
ROM_END
ROM_START( g_alisie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alisie.bin", 0x000000, 0x100000, CRC(28165bd1) SHA1(c8a3667631fdbd4d0073e42ada9f7199d09c0cfa) )
ROM_END
ROM_START( g_alisij )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alisij.bin", 0x000000, 0x100000, CRC(4d476722) SHA1(04cf02a0004f2eba16af781dd55b946176bb9dbf) )
ROM_END
ROM_START( g_adivej )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_adivej.bin", 0x000000, 0x080000,  CRC(9e2d5b91) SHA1(540460e95f6a82256ca2a16f347a7b6524f3053f) )
ROM_END
ROM_START( g_bdomen )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bdomen.bin", 0x000000, 0x080000, CRC(975693ce) SHA1(5378af243fb6f592f1e1cd17e3722e2c5e807e72) )
ROM_END
ROM_START( g_aofe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aofe.bin", 0x000000, 0x200000,  CRC(9970c422) SHA1(a58b1efbbdfa8c4ee6f3d06d474c3771ebe36ca4) )
ROM_END
ROM_START( g_bnza00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bnza00.bin", 0x000000, 0x080000, CRC(adf6476c) SHA1(ce46f2b53988f06e0bdfaad1b0dc3e9ca936e1e8) )
ROM_END
ROM_START( g_bnza01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bnza01.bin", 0x000000, 0x080000, CRC(c6aac589) SHA1(f7313ecaf7143873597e3a6ebf58d542d06c3ef3) )
ROM_END
ROM_START( g_astgru )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_astgru.bin", 0x000000, 0x200000, CRC(7f112cd8) SHA1(6d73f37fa31dbcc6a8e9e1590f552e084346088c) )
ROM_END

ROM_START( g_booge )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_booge.bin", 0x000000, 0x300000, CRC(dbc4340c) SHA1(4fd33eeaf1e804d005793a6e1185122fd7e9a751) )
ROM_END
ROM_START( g_cvane )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cvane.bin", 0x000000, 0x100000, CRC(4dd4e4a5) SHA1(61aabb1053f090fb6c13968c86170357c5df4eba) )
ROM_END
ROM_START( g_captha )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_captha.bin", 0x000000, 0x100000, CRC(76e6d20d) SHA1(6e0344aace03b703cb05a9a0e10c47ebe404a247) )
ROM_END
ROM_START( g_chouya )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chouya.bin", 0x000000, 0x200000, CRC(6d8c2206) SHA1(af2fd89dc7fb4ac0647c09edff462c7ae92dc771) )
ROM_END
ROM_START( g_chikij )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chikij.bin", 0x000000, 0x100000, CRC(06918c17) SHA1(77191eaee3775a425147ae7140ddf6ed3b6b41d2) )
ROM_END
ROM_START( g_comixe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_comixe.bin", 0x000000, 0x200000, CRC(1318e923) SHA1(9523cf8e485a3246027f5a02ecbcee3c5ba690f0) )
ROM_END
ROM_START( g_comixj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_comixj.bin", 0x000000, 0x200000, CRC(7a6027b8) SHA1(44f8c2a102971d0afcb0d9bd9081ccf51ff830a9) )
ROM_END
ROM_START( g_contrj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_contrj.bin", 0x000000, 0x200000, CRC(2ab26380) SHA1(0a9d263490497c85d7010979765c48f98d9927bd) )
ROM_END
ROM_START( g_coole )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_coole.bin", 0x000000, 0x100000, CRC(5f09fa41) SHA1(b6dc5d4c29b2161f7252828cf267117e726d8e82) )
ROM_END
ROM_START( g_crying )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crying.bin", 0x000000, 0x100000, CRC(4aba1d6a) SHA1(c45b6da77021d57df6a9cb511cc93a5bf83ecf1c) )
ROM_END

ROM_START( g_alade )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alade.bin", 0x000000, 0x200000, CRC(d1845e8f) SHA1(42debba01ba3555f61d1e9b445542a05d01451dd) )
ROM_END
ROM_START( g_aladj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aladj.bin", 0x000000, 0x200000, CRC(fb5aacf0) SHA1(43753dafd0b816c39aca87fc0788e598fb4bb4f3) )
ROM_END
ROM_START( g_ejim2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ejim2e.bin", 0x000000, 0x300000, CRC(af235fdf) SHA1(b8e93ea8b42c688a218b83797e4a18eda659f3e0) )
ROM_END
ROM_START( g_ejime )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ejime.bin", 0x000000, 0x300000, CRC(1c07b337) SHA1(d5dc11009e3a5cc2381dd2a75cb81ce2e7428342) )
ROM_END
ROM_START( g_dtusa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dtusa.bin", 0x000000, 0x100000, CRC(e2e21b72) SHA1(48b36852b31f74d33e00b33df04ae1e9e9b0cb1c) )
ROM_END
ROM_START( g_beane )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_beane.bin", 0x000000, 0x100000, CRC(70680706) SHA1(8cdaca024585aab557e9a09732a298e5112ee15b) )
ROM_END
ROM_START( g_dynab2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dynab2.bin", 0x000000, 0x200000, CRC(47e0a64e) SHA1(0d6c3d9eb0cb9a56ab91b7507b09473b078e773c) )
ROM_END
ROM_START( g_dheadj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dheadj.bin", 0x000000, 0x200000, CRC(d03cdb53) SHA1(02727a217e654f3cdf5a3fcd33b2f38d404a467d) )
ROM_END
ROM_START( g_taz2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_taz2e.bin", 0x000000, 0x200000, CRC(62100099) SHA1(1edd44d9f9d1d410b6a9ec37647a04d9b50b549e) )
ROM_END
ROM_START( g_f15e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f15e.bin", 0x000000, 0x100000, CRC(e98ee370) SHA1(190508c5fe65c47f289d85768f7d79b1a235f3d5) )
ROM_END

ROM_START( g_f22j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f22j.bin", 0x000000, 0x0c0000, CRC(fb55c785) SHA1(a6d0cf179d40ab7844d2a9f8a412a320a0639eec) )
ROM_END
ROM_START( g_f22ua )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f22ua.bin", 0x000000, 0x0c0000, CRC(31e9d1a5) SHA1(1100fbde1cb0534b3688ea8d8dc480d9f6e79548) )
ROM_END
ROM_START( g_f22u )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f22u.bin", 0x000000, 0x0c0000, CRC(9cf552c2) SHA1(a8fbd68686abcec9b765f47de7704ffbed4ec008) )
ROM_END
ROM_START( g_fever )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fever.bin", 0x000000, 0x200000, CRC(fac29677) SHA1(e3489b80a4b21049170fedee7630111773fe592c) )
ROM_END
ROM_START( g_fbckua )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fbckua.bin", 0x000000, 0x180000, CRC(33cd2b65) SHA1(225b405274d39541e07488fdd33de8f854624bf1) )
ROM_END
ROM_START( g_fbckj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fbckj.bin", 0x000000, 0x180000, CRC(b790e3b4) SHA1(5082180974a125b5f9c01c96410c0fdbfb707d2b) )
ROM_END
ROM_START( g_fbcke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fbcke.bin", 0x000000, 0x180000, CRC(6f311c83) SHA1(31372b2c056eacb747de0a706de3899d224f2c92) )
ROM_END
ROM_START( g_gamblr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gamblr.bin", 0x000000, 0x080000, CRC(05650b7a) SHA1(42814f8921f42c023c1fff433a2e9399aaff5d2e) )
ROM_END
ROM_START( g_garou )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_garou.bin", 0x000000, 0x180000, CRC(bf3e3fa4) SHA1(b9eb37dbc77d0c6919ad78e587fa4c3403e9e9af) )
ROM_END
ROM_START( g_garou2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_garou2.bin", 0x000000, 0x300000, CRC(2af4427f) SHA1(08eadaf6177d884b8e1fb66c5949850f10c5d77c) )
ROM_END

ROM_START( g_godse )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_godse.bin", 0x000000, 0x100000, CRC(6c415016) SHA1(404bc6e67cd4942615ccb7bd894d780278ec6da7) )
ROM_END
ROM_START( g_godsj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_godsj.bin", 0x000000, 0x100000, CRC(e4f50206) SHA1(804fd783c6fb7c226fbe4b227ed5c665d668ff57) )
ROM_END
ROM_START( g_gstenj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gstenj.bin", 0x000000, 0x080000,  CRC(30cf37d0) SHA1(f51c4332235a9545b86a7b121ba30644c33be098) )
ROM_END
ROM_START( g_gshe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gshe.bin", 0x000000, 0x100000, CRC(866ed9d0) SHA1(a7b265f49ec74f7febb8463c64535ceda15c8398) )
ROM_END
ROM_START( g_gshj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gshj.bin", 0x000000, 0x100000,  CRC(1cfd0383) SHA1(64ae3ef9d063d21b290849809902c221f6ab10d5) )
ROM_END
ROM_START( g_mickey )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mickey.bin", 0x000000, 0x080000, CRC(ce8333c6) SHA1(0679162757f375751a677fd05195c9248abc84f0) )
ROM_END
ROM_START( g_immorj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_immorj.bin", 0x000000, 0x200000, CRC(c99fad92) SHA1(f36d088c37bbdcf473615b23e1cb21c0b70d8f05) )
ROM_END
ROM_START( g_jmons2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jmons2.bin", 0x000000, 0x100000, CRC(f2363a4a) SHA1(3925623fafb79a7e8467e9b6fe70361c147cbfd3) )
ROM_END
ROM_START( g_madj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_madj.bin", 0x000000, 0x080000, CRC(0460611c) SHA1(c4cf3681d86861a823fa3e7ffe0cd451fbafcee6) )
ROM_END
ROM_START( g_jbooke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jbooke.bin", 0x000000, 0x200000,  CRC(b9709a99) SHA1(b96dcaf595a713eeec54257e8cee6306d7748baf) )
ROM_END

ROM_START( g_jstrj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jstrj.bin", 0x000000, 0x200000, CRC(ba7a870b) SHA1(e35c554c52ce31de837c0e73bf2014720657f4d5) )
ROM_END
ROM_START( g_kingcj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kingcj.bin", 0x000000, 0x100000, CRC(ffe7b3c7) SHA1(92bfb3548ebff18eedebe07751bf2170f95780d7) )
ROM_END
ROM_START( g_klaxj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_klaxj.bin", 0x000000, 0x040000, CRC(1afcc1da) SHA1(f084f7a3851161523ab7e9cffb2f729563c17643) )
ROM_END
ROM_START( g_kujaku )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kujaku.bin", 0x000000, 0x060000, CRC(affd56bc) SHA1(d48ed88269b4ea4c62a85f8607658f9ce566590c) )
ROM_END
ROM_START( g_ksfh00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ksfh00.bin", 0x000000, 0x080000, CRC(f764005e) SHA1(ab794df527e9fc5823cc5f08130dc856456980b0) )
ROM_END
ROM_START( g_kuuga )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kuuga.bin", 0x000000, 0x100000, CRC(83b6b6ba) SHA1(54d257ad1f941fdb39f6f7b2a3a168eb30ebd9ff) )
ROM_END
ROM_START( g_kyuuky )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kyuuky.bin", 0x000000, 0x0a0000, CRC(61276d21) SHA1(cbe207732c6ce5e5e5846e44847ce902315f2bc3) )
ROM_END
ROM_START( g_landsj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_landsj.bin", 0x000000, 0x200000, CRC(60d4cedb) SHA1(cdbc7cd9ceb181cad9e49b641ff717072546f0d9) )
ROM_END
ROM_START( g_landse )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_landse.bin", 0x000000, 0x200000, CRC(e3c65277) SHA1(9fbadc86319936855831ecd096d82d716b304215) )
ROM_END
ROM_START( g_landsg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_landsg.bin", 0x000000, 0x200000, CRC(10fedb8f) SHA1(b0b447158cabb562e6bf4c0b829ed938b34afb52) )
ROM_END

ROM_START( g_lem2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lem2e.bin", 0x000000, 0x200000, CRC(741eb624) SHA1(b56f9e78dee0186c8f8103c7d125e8b497eb0196) )
ROM_END
ROM_START( g_leme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_leme.bin", 0x000000, 0x100000, CRC(6a1a4579) SHA1(f4b86031e348edb4dcffaf969998f368955828ce) )
ROM_END
ROM_START( g_lem00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lem00.bin", 0x000000, 0x100000, CRC(f015c2ad) SHA1(83aebf600069cf053282e813d3ab4910f586706e) )
ROM_END
ROM_START( g_lenfe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lenfe.bin", 0x000000, 0x200000, CRC(ca2bf99d) SHA1(b9cc7ff3c6a50b2624358785bcadd1451e23993e) )
ROM_END
ROM_START( g_lenfj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lenfj.bin", 0x000000, 0x200000, CRC(f25f1e49) SHA1(14245fccf4d7e5d1dd4ad5f426507516e71e3a06) )
ROM_END
ROM_START( g_lighfr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lighfr.bin", 0x000000, 0x100000, CRC(c8f8c0e0) SHA1(ab5bd9ddbfc07d860f44b9c72d098aef2581d1d8) )
ROM_END
ROM_START( g_lordmo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lordmo.bin", 0x000000, 0x200000, CRC(238bf5db) SHA1(9b5c71d70de132c8ba6f2adfdeba43077f76ac3e) )
ROM_END
ROM_START( g_mmftbe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmftbe.bin", 0x000000, 0x200000, CRC(2307b905) SHA1(717e924db1ac7cfd099adb6031a08606fcb30219) )
ROM_END
ROM_START( g_marsue )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_marsue.bin", 0x000000, 0x200000, CRC(e09bbd70) SHA1(d2b8358ef261f8b5ad54a58e89f3999312d0cec9) )
ROM_END
ROM_START( g_mazij )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mazij.bin", 0x000000, 0x100000, CRC(45b3a34b) SHA1(bcaf2820f22d4a7bcf6324f288698192996875bd) )
ROM_END

ROM_START( g_mbmba )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mbmba.bin", 0x000000, 0x100000,  CRC(54ab3beb) SHA1(01f76e2f719bdae5f21ff0e5a1ac1262c2def279) )
ROM_END
ROM_START( g_mturre )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mturre.bin", 0x000000, 0x100000, CRC(b1d15d0f) SHA1(00ad2cf231bedbd373253b169e170e8b0db4c86a) )
ROM_END
ROM_START( g_mwlk00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mwlk00.bin", 0x000000, 0x080000, CRC(6a70791b) SHA1(8960bac2027cdeadb07e535a77597fb783e1433b) )
ROM_END
ROM_START( g_mmprme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmprme.bin", 0x000000, 0x200000, CRC(254a4972) SHA1(b3d105d9f7a8d2fd92015e1ac98d13a4094de5ef) )
ROM_END
ROM_START( g_mmpre )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmpre.bin", 0x000000, 0x200000, CRC(7f96e663) SHA1(7bad0db9a96eafa413562b6631487ca847b02466) )
ROM_END
ROM_START( g_mk3e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mk3e.bin", 0x000000, 0x400000, CRC(af6de3e8) SHA1(7f555d647972fee4e86b66e840848e91082f9c2d) )
ROM_END
ROM_START( g_mk00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mk00.bin", 0x000000, 0x200000, CRC(1aa3a207) SHA1(c098bf38ddd755ab7caa4612d025be2039009eb2) )
ROM_END
ROM_START( g_mushaj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mushaj.bin", 0x000000, 0x080000, CRC(8fde18ab) SHA1(71d363186b9ee023fd2ae1fb9f518c59bf7b8bee) )
ROM_END
ROM_START( g_nbahte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nbahte.bin", 0x000000, 0x300000, CRC(edb4d4aa) SHA1(4594ba338a07dd79639c11b5b96c7f1a6e283d0c) )
ROM_END
ROM_START( g_nbajj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nbajj.bin", 0x000000, 0x200000, CRC(a6c6305a) SHA1(2a88b2e1ecf115fa6246397d829448b755a5385e) )
ROM_END


ROM_START( g_nbaj00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nbaj00.bin", 0x000000, 0x200000, CRC(10fa248f) SHA1(99c5bc57fdea7f9df0cd8dec54160b162342344d) )
ROM_END
ROM_START( g_nbajt0 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nbajt0.bin", 0x000000, 0x300000, CRC(e9ffcb37) SHA1(ddbf09c5e6ed5d528ef5ec816129a332c685f103) )
ROM_END
ROM_START( g_nigel )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nigel.bin", 0x000000, 0x100000, CRC(6bc57b2c) SHA1(a64b3c8e8ad221cd02d47d551f3492515658ee1d) )
ROM_END
ROM_START( g_oozee )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_oozee.bin", 0x000000, 0x100000, CRC(e16b102c) SHA1(0e45b84e875aa4e27bf003cbc0f857f2e5659625) )
ROM_END
ROM_START( g_2019e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2019e.bin", 0x000000, 0x100000, CRC(5cb3536a) SHA1(f1a26bd827e9a674cf1dde74dc2c36f61c827c76) )
ROM_END
ROM_START( g_2019j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2019j.bin", 0x000000, 0x100000, CRC(0eac7440) SHA1(3b26b50f4194408cccd3fb9484ac83b97e94e67f) )
ROM_END
ROM_START( g_orunj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_orunj.bin", 0x000000, 0x100000, CRC(ee7d9f4a) SHA1(a94c96ffb53080b2c098be86e715f1dc727be07d) )
ROM_END
ROM_START( g_orunrj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_orunrj.bin", 0x000000, 0x200000, CRC(e164a09f) SHA1(4225162ed3cf51e16bfe800ae74a9b026f5dee56) )
ROM_END
ROM_START( g_pstr4j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pstr4j.bin", 0x000000, 0x300000, CRC(f0bfad42) SHA1(9d330d5e395b7caae11fae92f71d259b8391904b) )
ROM_END
ROM_START( g_pstr2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pstr2j.bin", 0x000000, 0x0c0000, CRC(bec8eb5a) SHA1(fc186c681e110723b4be5590f242c73d5004c8a7) )
ROM_END

ROM_START( g_pstr3j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pstr3j.bin", 0x000000, 0x0c0000, CRC(6c48c06f) SHA1(68b0f8e73dea5dca1b6ac8c0e12bc1d9761edf32) )
ROM_END
ROM_START( g_2040e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2040e.bin", 0x000000, 0x200000, CRC(b024882e) SHA1(6d588fac0fed0e2303082e29cde12d94719cca32) )
ROM_END
ROM_START( g_pitfe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pitfe.bin", 0x000000, 0x200000, CRC(c9198e19) SHA1(b7c5f6e5f422c7bb5ec3177328d963c9beace24f) )
ROM_END
ROM_START( g_pifia )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pifia.bin", 0x000000, 0x100000, CRC(1e0e0831) SHA1(19f81e5d0c4564e06fdccd7a7e67d98fa04360f4) )
ROM_END
ROM_START( g_pocae )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pocae.bin", 0x000000, 0x400000, CRC(165e7987) SHA1(5ffcdc3e01151837e707ae225a3a845a8b6d3394) )
ROM_END
ROM_START( g_popue )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_popue.bin", 0x000000, 0x080000, CRC(83d56f64) SHA1(fe388caefbad7c08b699ed2d0e8a62ff1d697a16) )
ROM_END
ROM_START( g_popuj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_popuj.bin", 0x000000, 0x080000, CRC(97c26818) SHA1(9abd1806c6e921a6df0f71361aee8024d5c3f049) )
ROM_END
ROM_START( g_pmonj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pmonj.bin", 0x000000, 0x100000, CRC(553289b3) SHA1(f0a48f25d87f7a6d17ff76f5e29ea7be2d430ce4) )
ROM_END
ROM_START( g_probot )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_probot.bin", 0x000000, 0x200000, CRC(bc597d48) SHA1(69d905751ffa6a921586767263e45ddeb73333c2) )
ROM_END
ROM_START( g_ppina )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ppina.bin", 0x000000, 0x180000, CRC(11e9c3f2) SHA1(2ea7befb7e52334111385667030ceb647157a397) )
ROM_END


ROM_START( g_puy200 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_puy200.bin", 0x000000, 0x200000,  CRC(51ad7797) SHA1(b9fd6e14f446a16927b54de0234bcd981a86488f) )
ROM_END
ROM_START( g_renste )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_renste.bin", 0x000000, 0x100000, CRC(c276c220) SHA1(34d520d8a835e386c26793f216b1dc8f78d8f67a) )
ROM_END
ROM_START( g_rrsh2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rrsh2j.bin", 0x000000, 0x100000, CRC(9a5723b6) SHA1(d89d7707cd4f30eef1fd2fd7e322d760ba8d6786) )
ROM_END
ROM_START( g_rrsh22 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rrsh22.bin", 0x000000, 0x100000, CRC(0876e992) SHA1(9afea43ed627671b50dd4a2abdd043b235414b91) )
ROM_END
ROM_START( g_runark )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_runark.bin", 0x000000, 0x080000, CRC(0894d8fb) SHA1(25a0322db3b82e3da6f1dab550b3e52f21be6b58) )
ROM_END
ROM_START( g_ryuuko )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ryuuko.bin", 0x000000, 0x200000, CRC(054cf5f6) SHA1(c4adc3fc6e1130f64440c7dc2673b239f1523626) ) // aof clone
ROM_END
ROM_START( g_samshe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_samshe.bin", 0x000000, 0x300000, CRC(c972014f) SHA1(86384123c49c05b6f90cffd7327c11882f01e9c0) )
ROM_END
ROM_START( g_samspi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_samspi.bin", 0x000000, 0x300000, CRC(0ea2ae36) SHA1(54a54996035b220ce72aa7e725f99860e3cd66a0) )
ROM_END
ROM_START( g_shin3e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shin3e.bin", 0x000000, 0x100000, CRC(0b6d3eb5) SHA1(23579c8f7e2396080b478b113aff36d2382395a3) )
ROM_END
ROM_START( g_sidepe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sidepe.bin", 0x000000, 0x100000, CRC(36e08145) SHA1(a19cd561397dfda55942cf4f9771b0e815f95f65) )
ROM_END

ROM_START( g_bart00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bart00.bin", 0x000000, 0x080000, CRC(c8620574) SHA1(3cf4447a0a883c78645c6faded28c51e0d8c0d63) )
ROM_END
ROM_START( g_soleif )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soleif.bin", 0x000000, 0x200000, CRC(08dc1ead) SHA1(7890074018f165eeb1281d81039fb07ccde7d197) )
ROM_END
ROM_START( g_soleig )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soleig.bin", 0x000000, 0x200000, CRC(332b9ecd) SHA1(65c8b7ab94b05812d009b4bebda3c49891a6bfbe) )
ROM_END
ROM_START( g_sonse )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sonse.bin", 0x000000, 0x100000, CRC(aea0786d) SHA1(f61a568314133b60de82ac162b5b52473adc9e1c) )
ROM_END
ROM_START( g_sonsj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sonsj.bin", 0x000000, 0x100000, CRC(acd08ce8) SHA1(43b2fdac9c747d6f6a629347c589599074408cd9) )
ROM_END
ROM_START( g_son200 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_son200.bin", 0x000000, 0x100000, CRC(24ab4c3a) SHA1(14dd06fc3aa19a59a818ea1f6de150c9061b14d4) )
ROM_END
ROM_START( g_son3e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_son3e.bin", 0x000000, 0x200000, CRC(6a632503) SHA1(2ff45bb056ede0f745e52f8d02c54b4ca724ca4c) )
ROM_END
ROM_START( g_son3j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_son3j.bin", 0x000000, 0x200000, CRC(f4951d1f) SHA1(7b98b21b7274233e962132bc22a7ccdf548c0ddb) )
ROM_END
ROM_START( g_soni00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soni00.bin", 0x000000, 0x080000, CRC(f9394e97) SHA1(6ddb7de1e17e7f6cdb88927bd906352030daa194) )
ROM_END
ROM_START( g_sorcer )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sorcer.bin", 0x000000, 0x080000, CRC(a143a8c5) SHA1(cbd7f0693a0d127138977da7cdf5a7f9440dfd43) )
ROM_END

ROM_START( g_sfbob )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sfbob.bin", 0x000000, 0x100000, CRC(e9310d3b) SHA1(2d0e4423e28d7175fc27a9c5b1cb86f1d5cedd3e) )
ROM_END
ROM_START( g_si90 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_si90.bin", 0x000000, 0x040000, CRC(22adbd66) SHA1(d15830fd1070960d1696c1a9d48c9f7db3aa89e4) )
ROM_END
ROM_START( g_sf2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sf2e.bin", 0x000000, 0x300000, CRC(56d41136) SHA1(2a406e2e4743de98785c85322f858abfb8221ae0) )
ROM_END
ROM_START( g_sf2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sf2j.bin", 0x000000, 0x300000, CRC(2e487ee3) SHA1(0d624f1a34014ead022dd8d5df1134a88eca69bb) )
ROM_END
ROM_START( g_sor2je )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sor2je.bin", 0x000000, 0x200000, CRC(42e3efdc) SHA1(a0d3a216278aef5564dcbed83df0dd59222812c8) )
ROM_END
ROM_START( g_sor3e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sor3e.bin", 0x000000, 0x300000, CRC(3b78135f) SHA1(5419f5eaf12201a662f03a79298a1b661005f73a) )
ROM_END
ROM_START( g_sor3ea )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sor3ea.bin", 0x000000, 0x300000, CRC(90ef991e) SHA1(8c0bc5b66703efbdeef2d6ede5745aad89bd8a44) )
ROM_END
ROM_START( g_sor00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sor00.bin", 0x000000, 0x080000, CRC(bff227c6) SHA1(3d74dbc81f3472a5bde45bf265e636a72a314667) )
ROM_END
ROM_START( g_strid2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_strid2.bin", 0x000000, 0x100000, CRC(e85e5270) SHA1(c048bf092745654bb60a437ef1543abfd407093c) )
ROM_END
ROM_START( g_suphq )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_suphq.bin", 0x000000, 0x080000, CRC(ab2c52b0) SHA1(0598e452e3d42b5e714bf5b32834000607483813) )
ROM_END


ROM_START( g_talmit )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_talmit.bin", 0x000000, 0x100000, CRC(05dc3ffc) SHA1(be80f96bee64bab159614d29f882442abef9de76) )
ROM_END
ROM_START( g_terme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_terme.bin", 0x000000, 0x100000, CRC(15f4d302) SHA1(c8ee275f2e30aaf6ad713c6cd915a4ede65328e0) )
ROM_END
ROM_START( g_tf2md )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tf2md.bin", 0x000000, 0x080000, CRC(e75ec3e0) SHA1(44b173b74225e5b562cdf3982926a051f05ed98e) )
ROM_END
ROM_START( g_tf4e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tf4e.bin", 0x000000, 0x100000, CRC(e7e3c05b) SHA1(ecbc2bfc4f3d8bbd46b398274ed2f5cc3db68454) )
ROM_END
ROM_START( g_tje00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tje00.bin", 0x000000, 0x100000, CRC(d1b36786) SHA1(7f82d8b57fff88bdca5d8aff85b01e231dc1239a) )
ROM_END
ROM_START( g_tje2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tje2e.bin", 0x000000, 0x200000, CRC(47b0a871) SHA1(ac05701d86ba8957adb8fe0b67d8b4bb51328d98) )
ROM_END
ROM_START( g_tje2g )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tje2g.bin", 0x000000, 0x200000, CRC(4081b9f2) SHA1(9d4e4b358147ab913c3fcff2811558eac7b8b466) )
ROM_END
ROM_START( g_tje2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tje2j.bin", 0x000000, 0x200000, CRC(e1b36850) SHA1(141af8fd1e5b4ba5118a384771b0d75f40af312f) )
ROM_END

ROM_START( g_toyste )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_toyste.bin", 0x000000, 0x400000, CRC(8e89a9f3) SHA1(6e7bb9b191389973922a5ab9978205bb9d2664cc) )
ROM_END
ROM_START( g_umk3e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_umk3e.bin", 0x000000, 0x400000, CRC(ecfb5cb4) SHA1(044bfdb3761df7c4d54a25898353fabcd3f604a3) )
ROM_END
ROM_START( g_vamkil )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vamkil.bin", 0x000000, 0x100000, CRC(91b57d2b) SHA1(3e709bd27577056abbbd6735021eaffd90caa140) )
ROM_END
ROM_START( g_wardj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wardj.bin", 0x000000, 0x080000, CRC(80f1035c) SHA1(28844399b73628a3507ca38e855d0afe24c59f4b) )
ROM_END
ROM_START( g_wwics )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wwics.bin", 0x000000, 0x100000, CRC(7d4450ad) SHA1(8441f0eba23c0b013ca3932914f9f8d364e61a01) )
ROM_END
ROM_START( g_wtics )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wtics.bin", 0x000000, 0x100000, CRC(d523b552) SHA1(213b6dfc129fe245f2ecd73ad91c772efd628462) )
ROM_END
ROM_START( g_wticsa )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wticsa.bin", 0x000000, 0x100000, CRC(ea19d4a4) SHA1(0b726481cd9333d26aa3fe53fa2f293c0c385509) )
ROM_END
ROM_START( g_whipj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_whipj.bin", 0x000000, 0x080000,  CRC(8084b4d1) SHA1(c7aac47bd0ecd6018ec1f1f4b42e53f27e318b15) )
ROM_END
ROM_START( g_winwor )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_winwor.bin", 0x000000, 0x080000,  CRC(210a2fcd) SHA1(91caad60355f6bd71949118bd30ea16d0f4c066e) )
ROM_END
ROM_START( g_wizlie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wizlie.bin", 0x000000, 0x100000, CRC(f09353b4) SHA1(2e7a1724a72e89a3f9a66720d9a7c6f293263798) )
ROM_END

ROM_START( g_zombe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zombe.bin", 0x000000, 0x100000, CRC(179a1aa2) SHA1(f0b447196d648b282050a52bdfdd9bf0d8f3d57e) )
ROM_END
ROM_START( g_zoole )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zoole.bin", 0x000000, 0x100000, CRC(1ee58b03) SHA1(cab14f63b7d00b35a11a3a7f60cf231199121dc8) ) //noboot
ROM_END
ROM_START( g_zoope )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zoope.bin", 0x000000, 0x080000, CRC(2fdac6ab) SHA1(9914956f974b71d24b65e82ae3c980d0eb23c2e5) )
ROM_END
ROM_START( g_zouzou )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zouzou.bin", 0x000000, 0x080000, CRC(1a761e67) SHA1(0cf2e2a7f00fb3d7a1a6424152693138ece586f1) )
ROM_END
ROM_START( g_ztkse )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ztkse.bin", 0x000000, 0x200000,  CRC(45ff0b4b) SHA1(11a501f5dbde889edcc14e366b67bb75b0c5bc13) )
ROM_END


ROM_START( g_007 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_007.bin", 0x000000, 0x080000, CRC(aeb4b262) SHA1(7e0de7011a60a1462dc48594f3caa956ff942281) )
ROM_END
ROM_START( g_abate )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_abate.bin", 0x000000, 0x200000,  CRC(355e8c39) SHA1(c08c48236c38263df8ea38a5820d16644bddb1a2) )
ROM_END
ROM_START( g_aerobl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aerobl.bin", 0x000000, 0x080000,   CRC(a00da987) SHA1(64d1964c3b6293f521427d3d83e4defe363cc129) )
ROM_END
ROM_START( g_acro2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_acro2e.bin", 0x000000, 0x200000,  CRC(a451f9a1) SHA1(529200d5cea7a5560debd42b547631e7cef38b8b) )
ROM_END
ROM_START( g_abrn2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_abrn2j.bin", 0x000000, 0x080000,  CRC(4ff37e66) SHA1(8f1e6a3e83094e5d0e237d6a1ffcc87171595ea5) )
ROM_END
ROM_START( g_alexkk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alexkk.bin", 0x000000, 0x040000,  CRC(5b0678fb) SHA1(aa9f686045fe83ee5f86640e5a711f80cad16b1e) )
ROM_END
ROM_START( g_alxkeb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alxkeb.bin", 0x000000, 0x040000,  CRC(778a0f00) SHA1(b7e96b33ab1715fe265ed0de81a26dde969698d5) )
ROM_END
ROM_START( g_alexke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alexke.bin", 0x000000, 0x040000,  CRC(c3a52529) SHA1(fd033ad9bd60d57adf52fa8cd0c1a5968d083f46) )
ROM_END
ROM_START( g_alexkj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_alexkj.bin", 0x000000, 0x040000,  CRC(8a5ed856) SHA1(0bd83099edd3938a0f127ed399cb01046e36ac32) )
ROM_END
ROM_START( g_ali300 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ali300.bin", 0x000000, 0x080000, CRC(a3b00d6e) SHA1(1f4b969592f98d2692cb06eca550da9c03062593) )
ROM_END

ROM_START( g_aatee )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aatee.bin", 0x000000, 0x080000,  CRC(224256c7) SHA1(70fa6185a4ebbbc9a6b2c7428c489ba5303859b0) )
ROM_END
ROM_START( g_anime )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_anime.bin", 0x000000, 0x100000,   CRC(92b6f255) SHA1(c474d13afb04bfdb291cfabe43ffc0931be42dbc) )
ROM_END
ROM_START( g_arcusj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arcusj.bin", 0x000000, 0x100000,  CRC(41c5fb4f) SHA1(8e1a9a9f9ba50bca0a377bd8f795b7c9c1275815) )
ROM_END
ROM_START( g_arrowj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arrowj.bin", 0x000000, 0x080000,   CRC(d49f8444) SHA1(5d2ca55704b7fe8d83fa7564fb1efc62834d3148) )
ROM_END
ROM_START( g_robokj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robokj.bin", 0x000000, 0x080000,  CRC(e833067e) SHA1(9bed099693c27a6575b394bdd150efb7cc53c5c6) )
ROM_END
ROM_START( g_arunre )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_arunre.bin", 0x000000, 0x100000,  CRC(b3c05418) SHA1(ab84274bc98a1f8f808bee3f41645884c95cc840) )
ROM_END
ROM_START( g_smgp2a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smgp2a.bin", 0x000000, 0x100000,  CRC(60af0f76) SHA1(373fb1744170a114ef99802db987bc9aae009032) )
ROM_END
ROM_START( g_bttfe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bttfe.bin", 0x000000, 0x080000,  CRC(2737f92e) SHA1(c808ee7f6f61c096ab73b68dd181e25fdcfde243) )
ROM_END
ROM_START( g_batmnj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_batmnj.bin", 0x000000, 0x080000,  CRC(d7b4febf) SHA1(7f96c72cc7bbdb4a7847d1058f5a751b2dc00ab5) )
ROM_END
ROM_START( g_beaswr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_beaswr.bin", 0x000000, 0x100000,  CRC(4646c694) SHA1(b504a09e3283716f0f9b0464ea9c20cd40110408) )
ROM_END

ROM_START( g_beave )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_beave.bin", 0x000000, 0x200000,  CRC(c7b6435e) SHA1(0d132afbc76589b95a0c617d39122f0715eab2c6) )
ROM_END
ROM_START( g_drace )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drace.bin", 0x000000, 0x100000,  CRC(9ba5a063) SHA1(d1e2bb4febf973e3510118d2ea71b4c6594480a9) )
ROM_END
ROM_START( g_bubbae )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bubbae.bin", 0x000000, 0x100000,  CRC(b467432e) SHA1(4039dfb41c08d17047d2acf90d0ab8bb7932cabd) )
ROM_END
ROM_START( g_budoe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_budoe.bin", 0x000000, 0x080000,  CRC(97add5bd) SHA1(004f3d6f333795315a072f3f0661ce4e5e91a4ae) )
ROM_END
ROM_START( g_burnfj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_burnfj.bin", 0x000000, 0x080000,  CRC(0c1deb47) SHA1(8849253262f545fbaf6140bfa5ca67a3caac9a80) )
ROM_END
ROM_START( g_chelno )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chelno.bin", 0x000000, 0x100000,   CRC(b2fe74d8) SHA1(ac1ae7b38f6498472bc4f072726a4ca069e41204) )
ROM_END
ROM_START( g_chk2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chk2e.bin", 0x000000, 0x100000,  CRC(1ade9488) SHA1(f57ed0e6201b706abd5e837d9043723a1b3b4de5) )
ROM_END
ROM_START( g_chk2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chk2j.bin", 0x000000, 0x100000,   CRC(bfd24be8) SHA1(e9217b089ade7f7b9566233e7bff66ba363ad6cb) )
ROM_END
ROM_START( g_chkrke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chkrke.bin", 0x000000, 0x100000,  CRC(7cd40bea) SHA1(832a18eb028630e31b5bacd05f9694f4a827268b) )
ROM_END
ROM_START( g_col00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_col00.bin", 0x000000, 0x020000,  CRC(03163d7a) SHA1(b262a4c2738a499f070777dbe05e2629d211a107) )
ROM_END


ROM_START( g_col3j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_col3j.bin", 0x000000, 0x080000, CRC(cd07462f) SHA1(2e850c2b737098b9926ac0fc9b8b2116fc5aa48a) )
ROM_END
ROM_START( g_crkde )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crkde.bin", 0x000000, 0x080000,  CRC(d012a47a) SHA1(7c146c24216bb333eaa3b08e358582e4465b145e) )
ROM_END
ROM_START( g_crkdj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crkdj.bin", 0x000000, 0x080000,  CRC(538aaa5d) SHA1(1082c89920699dbc6f6672c9b0519b3d0f626ba5) )
ROM_END
ROM_START( g_crudeb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crudeb.bin", 0x000000, 0x100000, CRC(affb4b00) SHA1(9122ef0e920133449266cf437a87a110e0343425) )
ROM_END
ROM_START( g_cruej )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cruej.bin", 0x000000, 0x080000,  CRC(514c53e2) SHA1(1ceb4370dacaa8e3ab4b8beb6ff008db269d0387) )
ROM_END
ROM_START( g_dinolj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dinolj.bin", 0x000000, 0x080000,  CRC(81f939de) SHA1(4bee752cd2205d60b9d3c7a58e0e90a4837522de) )
ROM_END
ROM_START( g_aladb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aladb.bin", 0x000000, 0x200000,  CRC(8c60ef73) SHA1(1f8d4f888b761a878dcc5ffe2dc7c6fef46db1ca) )
ROM_END
ROM_START( g_djboye )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_djboye.bin", 0x000000, 0x080000,  CRC(860e749a) SHA1(d2f111c240d0165a231c236e9ae6e62e73ca9caa) )
ROM_END
ROM_START( g_djboyj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_djboyj.bin", 0x000000, 0x080000,  CRC(202abaa8) SHA1(c66b2e00b8a9172a76f5e699730c7d88740b5dd7) )
ROM_END
ROM_START( g_ddukea )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ddukea.bin", 0x000000, 0x080000,  CRC(246f0bda) SHA1(965ec4e6a239a707160d0a67973bc6da8212b53d) )
ROM_END

ROM_START( g_ecco2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ecco2e.bin", 0x000000, 0x200000,  CRC(7b1bf89c) SHA1(cb7d44a40992cff6c31d685866814d6cb85add59) )
ROM_END
ROM_START( g_ecco2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ecco2j.bin", 0x000000, 0x200000,  CRC(062d439c) SHA1(0ca535d0e0d430c67e413ab904ef867516ce9fad) )
ROM_END
ROM_START( g_ecjr01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ecjr01.bin", 0x000000, 0x100000,  CRC(3c517975) SHA1(636d2fb5f865f916e4a9fe0ff1819fcbc61b4258) )
ROM_END
ROM_START( g_eccodj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_eccodj.bin", 0x000000, 0x100000,  CRC(6520304d) SHA1(1440fb5821ebb08048f73a0a71ac22e0cdbcf394) )
ROM_END
ROM_START( g_elvinj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_elvinj.bin", 0x000000, 0x100000,  CRC(6091c36e) SHA1(564ffd2a40b9bebe30587182a644050de474aa5a) )
ROM_END
ROM_START( g_elemj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_elemj.bin", 0x000000, 0x080000,  CRC(5f553e29) SHA1(6cbe2cd607aa9850306970a8a61cb9fa82445293) )
ROM_END
ROM_START( g_eswatj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_eswatj.bin", 0x000000, 0x080000,  CRC(87b636a2) SHA1(353f7d136a2c464ee976402b4620b0a42b8b7267) )
ROM_END
ROM_START( g_echmpe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_echmpe.bin", 0x000000, 0x300000,  CRC(b9512f5e) SHA1(578475a736bef8c76ba158110cebadf495c2f252) )
ROM_END
ROM_START( g_echmpj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_echmpj.bin", 0x000000, 0x300000, CRC(66aa3c64) SHA1(1b115e64e138ca045acfe64e2337fc68172be576) )
ROM_END
ROM_START( g_yuyub )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yuyub.bin", 0x000000, 0x300000,  CRC(fe3fb8ee) SHA1(e307363837626d47060f5347d0fccfc568a12b18) )
ROM_END

ROM_START( g_ys3j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ys3j.bin", 0x000000, 0x100000,  CRC(52da4e76) SHA1(28dc01d5dde8d569bae3fafce2af55ee9b836454) )
ROM_END
ROM_START( g_xzr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xzr.bin", 0x000000, 0x100000, CRC(880bf311) SHA1(14ef77cbf6c023365e168f54456d5486679292ef) ) // clone of exile
ROM_END
ROM_START( g_xmene )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_xmene.bin", 0x000000, 0x100000,  CRC(0b78ca97) SHA1(111d696b317ad5b57bc66e037a935d3f123d41c2) )
ROM_END
ROM_START( g_wfwaal )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wfwaal.bin", 0x000000, 0x040000,  CRC(719d6155) SHA1(7b76ddf26f11ad0e2da81f1cfe96e60a45fa7c33) )
ROM_END
ROM_START( g_wormp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wormp.bin", 0x000000, 0x200000,  CRC(1d191694) SHA1(23127e9b3a98eea13fb97bed2d8e206adb495d97) )
ROM_END
ROM_START( g_wille )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wille.bin", 0x000000, 0x100000,  CRC(121c6a49) SHA1(4b7aa8de517516edd9ee5288124f77238fc9ba6b) )
ROM_END
ROM_START( g_wheroj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wheroj.bin", 0x000000, 0x200000,  CRC(56e3ceff) SHA1(0c806b701069ae72b6cfd19b7b65a123192cff23) )
ROM_END
ROM_START( g_wcs00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wcs00.bin", 0x000000, 0x040000,  CRC(b01c3d70) SHA1(39bbab6430aad3fa9bf024c4b42387ba4ba3e488) )
ROM_END
ROM_START( g_wimbp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wimbp.bin", 0x000000, 0x100000,  CRC(9febc760) SHA1(402bdc507647d861ee7bb80599f528d3d5aeaf0f) )
ROM_END
ROM_START( g_wimbe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wimbe.bin", 0x000000, 0x100000,  CRC(b791a435) SHA1(6b7d8aa9d4b9d10c26dc079ab78e11766982cef2) )
ROM_END

ROM_START( g_virraj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virraj.bin", 0x000000, 0x200000,  CRC(53a293b5) SHA1(0ad38a3ab1cc99edac72184f8ae420e13df5cac6) )
ROM_END
ROM_START( g_virrae )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virrae.bin", 0x000000, 0x200000,   CRC(9624d4ef) SHA1(2c3812f8a010571e51269a33a989598787d27c2d) )
ROM_END
ROM_START( g_vectp )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vectp.bin", 0x000000, 0x200000,  CRC(a315c8aa) SHA1(54d611a0519d34c25ef6b9963543ece4afc23e19) )
ROM_END
ROM_START( g_vect2p )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vect2p.bin", 0x000000, 0x200000,  CRC(ada2b0ef) SHA1(65dc6261179e51d21c3c5fe0c0353befbcc95a2d) )
ROM_END
ROM_START( g_valsdj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_valsdj.bin", 0x000000, 0x080000,  CRC(1aef72ea) SHA1(4b4509936a75cb9b3b6fbf75b617be475cb99ffd) )
ROM_END
ROM_START( g_val3j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_val3j.bin", 0x000000, 0x100000,  CRC(4d49a166) SHA1(640c3d4f341c8a9b19e5deafae33e7742f109e2d) )
ROM_END
ROM_START( g_valj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_valj.bin", 0x000000, 0x100000,  CRC(24431625) SHA1(06208a19ca5f2b25bef7c972ec175d4aff235a77) )
ROM_END
ROM_START( g_uzuke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_uzuke.bin", 0x000000, 0x080000,  CRC(a7255ba8) SHA1(1d59864916be04640a117082c62453c09bcbf8b8) )
ROM_END

ROM_START( g_f1wlce )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1wlce.bin", 0x000000, 0x100000,  CRC(fbdd4520) SHA1(a8327157f6537f4cd5daaff648864e8e0bf945f1) )
ROM_END
ROM_START( g_f22b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f22b.bin", 0x000000, 0x0c0000,  CRC(d6a880a4) SHA1(f104e6fab831fb482b0426bae98f04a65a05f392) )
ROM_END
ROM_START( g_f117j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f117j.bin", 0x000000, 0x200000,  CRC(ea6e421a) SHA1(ded62d63676664c5b9e8a38e36ce1d2659d1c8d6) )
ROM_END
ROM_START( g_fante )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fante.bin", 0x000000, 0x080000,  CRC(d351b242) SHA1(ce0f0813878700e78b3819ff0db49bd297add09e) )
ROM_END
ROM_START( g_fant00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fant00.bin", 0x000000, 0x080000,  CRC(34e04627) SHA1(e1d84357eb8dfb972f22e80b8d26a0b66167f69e) )
ROM_END
ROM_START( g_ferias )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ferias.bin", 0x000000, 0x100000,  CRC(7b2e416d) SHA1(ec546d1c00c88554b5e9135f097cb1c388cfd68f) )
ROM_END
ROM_START( g_fi99r )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fi99r.bin", 0x000000, 0x200000,  CRC(c5c5a4b0) SHA1(2c8c1dc0aaa711e3ab3fe0d74b79184f33127350) )
ROM_END
ROM_START( g_fghmsj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fghmsj.bin", 0x000000, 0x080000,  CRC(39be80ec) SHA1(f944a8314d615ef58f0207e5760767fb417cbeb9) )
ROM_END
ROM_START( g_fshrke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fshrke.bin", 0x000000, 0x080000,  CRC(2351ce61) SHA1(54b9060699187bf32048c005a3379fda72c0fb96) )
ROM_END
ROM_START( g_fshrku )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fshrku.bin", 0x000000, 0x080000,  CRC(570b5024) SHA1(453ca331d15c47171c42312c14585541a3613802) )
ROM_END

ROM_START( g_flinte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_flinte.bin", 0x000000, 0x080000,  CRC(21845d61) SHA1(6350da9fa81a84a7a73d0238be135b1331147599) )
ROM_END
ROM_START( g_flintj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_flintj.bin", 0x000000, 0x080000,  CRC(920a3031) SHA1(a3dc66bb5b35916a188b2ef4b6b783f3c5ffc03c) )
ROM_END
ROM_START( g_fw00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fw00.bin", 0x000000, 0x080000,  CRC(d0ee6434) SHA1(8b9a37c206c332ef23dc71f09ec40e1a92b1f83a) ) // problems
ROM_END
ROM_START( g_ggrouj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ggrouj.bin", 0x000000, 0x080000,  CRC(8641a2ab) SHA1(a5017e44b5f470e0499f4a9b494385c567632864) )
ROM_END
ROM_START( g_gfkobe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gfkobe.bin", 0x000000, 0x100000, CRC(bd556381) SHA1(c93298ee3ad6164ce497bd49d0ab6638854acb79) )
ROM_END
ROM_START( g_gbus00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gbus00.bin", 0x000000, 0x080000, CRC(00419da3) SHA1(2a1589781fc4aca2c1ba97ec9ecf1acf563b7bfb) )
ROM_END
ROM_START( g_gng01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gng01.bin", 0x000000, 0x0a0000,  CRC(d31bd910) SHA1(3a3e9eb5caaf6cfb75e99ed2843d58b3dda7284f) )
ROM_END
ROM_START( g_gax00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gax00.bin", 0x000000, 0x080000,  CRC(e8182b90) SHA1(564e51f6b7fe5281f281d5fcb66767ab83ecf7b9) )
ROM_END
ROM_START( g_gran00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gran00.bin", 0x000000, 0x080000,   CRC(7f45719b) SHA1(6830d6ca1d7e2b220eb4431b42d33382e16e2791) )
ROM_END
ROM_START( g_ghwj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ghwj.bin", 0x000000, 0x200000,  CRC(7ef8b162) SHA1(1d5fe812df75e0dc1ff379c563058e078b839a09) )
ROM_END

ROM_START( g_gshsam )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gshsam.bin", 0x000000, 0x100000,  CRC(6f90b502) SHA1(a8d7f502d877b0ea3ff2431c94001e9d5dd84868) )
ROM_END
ROM_START( g_gynoge )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gynoge.bin", 0x000000, 0x080000,  CRC(03405102) SHA1(fada42bed3a97e1d1b0c6128ddb1f202a50cb1b0) )
ROM_END
ROM_START( g_helfij )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_helfij.bin", 0x000000, 0x080000,  CRC(8e5e13ba) SHA1(17bbf3da65757cad3c7ff82c094e54cc68fd2f73) )
ROM_END
ROM_START( g_herzoj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_herzoj.bin", 0x000000, 0x080000,  CRC(4cf676b3) SHA1(851be7abad64a4fb05e4c51ce26fbe5efe12ea42) )
ROM_END
ROM_START( g_hybrip )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hybrip.bin", 0x000000, 0x300000,  CRC(04f02687) SHA1(b6e72b69a22869f966c52e7d58d146bc48b5eb84) )
ROM_END
ROM_START( g_hdunkj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hdunkj.bin", 0x000000, 0x200000,  CRC(5baf53d7) SHA1(ef5c13926ee6eb32593b9e750ec342b98f48d1ef) )
ROM_END
ROM_START( g_indlce )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_indlce.bin", 0x000000, 0x100000,  CRC(eb8f4374) SHA1(70b5b4bc3f6d7ee7fbc77489bcfa4a96a831b88d) )
ROM_END
ROM_START( g_insxj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_insxj.bin", 0x000000, 0x080000,  CRC(9625c434) SHA1(204351b97aff579853cdba646b427ea299b1eddf) )
ROM_END
ROM_START( g_jlps00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jlps00.bin", 0x000000, 0x100000,   CRC(ec229156) SHA1(d3dcc24e50373234988061d3ef56c16d28e580ad) )
ROM_END
ROM_START( g_jp2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jp2j.bin", 0x000000, 0x080000,  CRC(98794702) SHA1(fa7d4e77fd98eb1fc9f8e1d66269bf86881c695d) )
ROM_END

ROM_START( g_jewlj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jewlj.bin", 0x000000, 0x080000,  CRC(2cf6926c) SHA1(52fbcf9902a4a3f70aeb9c3df31019e07e679ea8) )
ROM_END
ROM_START( g_jb11j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jb11j.bin", 0x000000, 0x080000,   CRC(8837e896) SHA1(769d7ed746ccbec0bcbe271f81767ffcccd36cea) )
ROM_END
ROM_START( g_jb1100 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jb1100.bin", 0x000000, 0x080000, CRC(22d77e6d) SHA1(977430962510867b16c113cff1436cb75c0485ac) )
ROM_END
ROM_START( g_jparke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jparke.bin", 0x000000, 0x200000,  CRC(448341f6) SHA1(2dca183ddb79805b6300d8fcca1163fce88dd9db) )
ROM_END
ROM_START( g_jparkj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jparkj.bin", 0x000000, 0x200000,  CRC(ec8e5783) SHA1(d5d0e2ed4c00435ec4a343583bb399b07927de21) )
ROM_END
ROM_START( g_kageki )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kageki.bin", 0x000000, 0x100000,  CRC(391866a1) SHA1(9424e8b759609004748dd6cd4779f917211264ae) )
ROM_END
ROM_START( g_kotme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kotme.bin", 0x000000, 0x100000,  CRC(7a94fd49) SHA1(46dc75f1e4d79fd159fcb4a256881375b63d9a2b) )
ROM_END
ROM_START( g_ksalmj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ksalmj.bin", 0x000000, 0x080000,   CRC(2cfc9f61) SHA1(62bea17b1e9152bde2355deb98347a802518d08d) )
ROM_END
ROM_START( g_lth2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lth2j.bin", 0x000000, 0x200000,  CRC(4bfe045c) SHA1(0c99f93ef90d6242b198c99a1e940be432ec861f) )
ROM_END
ROM_START( g_lhxj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lhxj.bin", 0x000000, 0x100000,  CRC(224ff103) SHA1(4e98a59fbd25db9fa9a05c84ecfa79154dd49d12) )
ROM_END

ROM_START( g_licrue )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_licrue.bin", 0x000000, 0x200000,  CRC(52c7252b) SHA1(4352ae7ba1316e4384c4632be80f2fe277443f51) )
ROM_END
ROM_START( g_licruj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_licruj.bin", 0x000000, 0x200000,   CRC(237076a4) SHA1(2b8c0931c33c143d41ef7cff6a8dbb9b8351d613) )
ROM_END
ROM_START( g_licruk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_licruk.bin", 0x000000, 0x200000,   CRC(6d0cbcb2) SHA1(0f018f95b4933bb5b3b3a91cee8b9a8ecb376942) )
ROM_END
ROM_START( g_marvj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_marvj.bin", 0x000000, 0x100000,  CRC(5d162d21) SHA1(2f7ef2f956d62373dcd5f3808e7501e3660c0658) )
ROM_END
ROM_START( g_momonj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_momonj.bin", 0x000000, 0x080000,  CRC(d51ee8c2) SHA1(ca62d376be5cc7b944bbe9b5f2610f8bb55a2fed) )
ROM_END
ROM_START( g_mazie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mazie.bin", 0x000000, 0x100000,  CRC(4b07a105) SHA1(40e48ce531ed013d5a4a6f689e70781df3e0095c) )
ROM_END
ROM_START( g_mcdtj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mcdtj.bin", 0x000000, 0x100000,  CRC(febcfd06) SHA1(aee7fd6a08ec22f42159b188f23406cbe3229f3d) )
ROM_END
ROM_START( g_micmaj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_micmaj.bin", 0x000000, 0x100000,   CRC(5c218c6a) SHA1(daf1ded2439c626c0fd227550be6563cd1b09612) )
ROM_END
ROM_START( g_mmanie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmanie.bin", 0x000000, 0x200000,  CRC(cb5a8b85) SHA1(6bcbf683a5d0e9f67b9ed8f79bc593fab594d84a) )
ROM_END
ROM_START( g_mmanij )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmanij.bin", 0x000000, 0x200000,  CRC(23180cf7) SHA1(afe3e1c9267e2a7f0bec5e3e951307ebea7f0b04) )
ROM_END

ROM_START( g_micm2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_micm2e.bin", 0x000000, 0x100000,  CRC(42bfb7eb) SHA1(ab29077a6a5c2ccc777b0bf22f4d5908401f4d47) )
ROM_END
ROM_START( g_midrej )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_midrej.bin", 0x000000, 0x100000,   CRC(8f3f6e4d) SHA1(b11db9fde0955c8aa0ae2d1a234d3295eda75d12) )
ROM_END
ROM_START( g_mig29e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mig29e.bin", 0x000000, 0x100000,  CRC(70b0a5d7) SHA1(5a2e7f8c752dfd8b301d2c044619c003a15894aa) )
ROM_END
ROM_START( g_mdpfua )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mdpfua.bin", 0x000000, 0x100000,  CRC(de50ca8e) SHA1(e0832fcd63fb164cac66c3df4b5dfb23eecbb0f6) )
ROM_END
ROM_START( g_mutlfj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mutlfj.bin", 0x000000, 0x100000,  CRC(2a97e6af) SHA1(ce68da5d70dddc0291b09b1e5790739ef6ac0748) )
ROM_END
ROM_START( g_mysd00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mysd00.bin", 0x000000, 0x080000,  CRC(f9ce1ab8) SHA1(fd82ea9badc892bfabacaad426f57ba04c6183b1) )
ROM_END
ROM_START( g_nfl94j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nfl94j.bin", 0x000000, 0x200000,  CRC(e490dc4a) SHA1(a530a93d124ccecfbf54f0534b6d3269026e5988) )
ROM_END
ROM_START( g_olgole )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olgole.bin", 0x000000, 0x080000,  CRC(924e57d3) SHA1(55702a7dee0cd2092a751b19c04d81694b0c0d0f) )
ROM_END
ROM_START( g_olgolj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olgolj.bin", 0x000000, 0x080000,   CRC(e9c925b8) SHA1(ce640bb979fcb285729b58de4da421c5493ef5e2) )
ROM_END
ROM_START( g_olgolu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olgolu.bin", 0x000000, 0x080000,  CRC(af639376) SHA1(089c4013e942fcb3e2e6645cf3e702cdc3f9fc36) )
ROM_END

ROM_START( g_olwge )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olwge.bin", 0x000000, 0x200000,  CRC(fa537a45) SHA1(84528efaf0729637167774d59a00694deadd5d6d) )
ROM_END
ROM_START( g_olwgj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_olwgj.bin", 0x000000, 0x200000,  CRC(654a4684) SHA1(aa2fc21eaa833640eaf882d66ca6ceb4f8adabcf) )
ROM_END
ROM_START( g_sswoj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sswoj.bin", 0x000000, 0x080000,  CRC(3960a00f) SHA1(e1e2bbbaf3e64c62fa695d1d1ee3496826951d13) )
ROM_END
ROM_START( g_samex3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_samex3.bin", 0x000000, 0x080000,  CRC(77bbd841) SHA1(b0d2552c5aae75dbe2d63600c0dbd64868c2f2c5) )
ROM_END
ROM_START( g_shbeaj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shbeaj.bin", 0x000000, 0x100000,  CRC(0cd09d31) SHA1(6e4bb7fb1a0f642d72cf58376bf2637e0ac6ef50) )
ROM_END
ROM_START( g_shrunj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shrunj.bin", 0x000000, 0x200000,  CRC(d32199f7) SHA1(e28bb51227abcc71aaf18d445d3651054247c662) )
ROM_END
ROM_START( g_shinch )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shinch.bin", 0x000000, 0x200000,  CRC(77b5b10b) SHA1(40265aae1b43f004434194038d32ca6b4841707d) ) // crusade clone
ROM_END
ROM_START( g_shdrkj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shdrkj.bin", 0x000000, 0x100000, CRC(496af51c) SHA1(c09ff5bcee1a29a48c65be4ad584708b85ca549b) )
ROM_END
ROM_START( g_shfrcj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shfrcj.bin", 0x000000, 0x180000,  CRC(9378fbcd) SHA1(ef8afbdc9af931d9da34d72efc8a76f0d5f4379d))
ROM_END
ROM_START( g_shfr2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shfr2j.bin", 0x000000, 0x200000,  CRC(0288f3e1) SHA1(8e1f1a510af4d43716d9ee34d47becf907dec147) )
ROM_END

ROM_START( g_shfr2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shfr2e.bin", 0x000000, 0x200000,   CRC(83cb46d1) SHA1(a2ba86f4d756f98886f8f9a56f20cbf8c3b2945e) )
ROM_END
ROM_START( g_shdrkb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shdrkb.bin", 0x000000, 0x100000,  CRC(3ee2bbc4) SHA1(08c6e884d48329c45d9f090aeea03efd4c1918c0) )
ROM_END
ROM_START( g_showd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_showd.bin", 0x000000, 0x200000,  CRC(0a22df04) SHA1(6f6ade812eb18e093a53773b8481c78f5a25631c) )
ROM_END
ROM_START( g_showd2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_showd2.bin", 0x000000, 0x200000,  CRC(48ee66cb) SHA1(8f713ef6035b616877ed9a09bac07913470c1f2f) )
ROM_END
ROM_START( g_shwd2a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shwd2a.bin", 0x000000, 0x200000,  CRC(d8c9ac6d) SHA1(d9c3b494086fef2fa59e031436756236f3cfdf22) )
ROM_END
ROM_START( g_sokoba )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sokoba.bin", 0x000000, 0x040000,  CRC(40f4aacc) SHA1(4f5341f3cd0b648c2116593b3f706b634a7e0128) )
ROM_END
ROM_START( g_soni2p )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soni2p.bin", 0x000000, 0x100000,  CRC(eea21b5c) SHA1(2f3228088b000260c2e00961efb0ed76629c84e9) )
ROM_END
ROM_START( g_sorkin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sorkin.bin", 0x000000, 0x100000,  CRC(944135ca) SHA1(16394aebece9d03f43505eab0827889e1c61857f) )
ROM_END
ROM_START( g_shar2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shar2j.bin", 0x000000, 0x080000,  CRC(edc0fb28) SHA1(5836fbe907610ff15286911457049933b7cdd49c) )
ROM_END
ROM_START( g_sparke )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sparke.bin", 0x000000, 0x100000,  CRC(d63e9f2d) SHA1(91057f22c5cea9bf08edf62862c56b939d570770) )
ROM_END

ROM_START( g_sparkj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sparkj.bin", 0x000000, 0x100000,  CRC(914ec662) SHA1(49e813e108751d502bfb242551b40121c71c5442) )
ROM_END
ROM_START( g_sbal2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sbal2e.bin", 0x000000, 0x080000,  CRC(056a6e03) SHA1(9c989b31de7de38bc488f825575b7f6c1db9dbee) )
ROM_END
ROM_START( g_shou2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shou2e.bin", 0x000000, 0x100000,  CRC(2559e03c) SHA1(e01940808006a346b8711a74fbfa173ec872624f) )
ROM_END
ROM_START( g_shou3j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shou3j.bin", 0x000000, 0x200000,  CRC(31b83d22) SHA1(1fcb8adfdb19cb772adabac14e78c560d4f2e718) )
ROM_END
ROM_START( g_spot2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spot2e.bin", 0x000000, 0x300000,  CRC(fbe254ea) SHA1(be1144c3d9d49dce2d5e1ab598ef0e2b730950b7) )
ROM_END
ROM_START( g_starcj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_starcj.bin", 0x000000, 0x080000,  CRC(2b75b52f) SHA1(cb099ecde141beffdfed6bb7f1d3dc6340da81d1) )
ROM_END
ROM_START( g_stng00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stng00.bin", 0x000000, 0x200000,  CRC(272153fb) SHA1(ca0cf81784262fe6c00502cb495ede7daf3685c0) )
ROM_END
ROM_START( g_strf00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_strf00.bin", 0x000000, 0x100000,  CRC(d550c928) SHA1(24252ca934be3f005436cdb85671413a9e6b0489) )
ROM_END
ROM_START( g_stalj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stalj.bin", 0x000000, 0x080000,  CRC(04f388e6) SHA1(93c51ec9fcc56d858642018905b3defc17442c26) )
ROM_END
ROM_START( g_slordj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_slordj.bin", 0x000000, 0x080000,  CRC(0b440fed) SHA1(fe06ea2d7fcccecce337a535ae683c31aae4a637) )
ROM_END

ROM_START( g_stridj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stridj.bin", 0x000000, 0x100000,  CRC(859173f2) SHA1(4198030057a1a0479b382fc2d69cfe32a523fa32) )
ROM_END
ROM_START( g_subte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_subte.bin", 0x000000, 0x200000,  CRC(e8ced28d) SHA1(23c6a0616f170f6616bc8214f3d45f1f293bba9f) )
ROM_END
ROM_START( g_subtj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_subtj.bin", 0x000000, 0x200000,  CRC(7638ea91) SHA1(e4e25a7a9a583be0dd8e7aa0f1ab3e96b2180bc6) )
ROM_END
ROM_START( g_ssride )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ssride.bin", 0x000000, 0x080000,   CRC(0ff33054) SHA1(4a4f2cf397ade091e83e07bb3ffc7aa5862aeedd) )
ROM_END
ROM_START( g_2020j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2020j.bin", 0x000000, 0x200000,   CRC(2bbee127) SHA1(3d00178aecd7611c7e4ad09ea07e8436de6a5375) )
ROM_END
ROM_START( g_sfze )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sfze.bin", 0x000000, 0x100000,  CRC(927975be) SHA1(ed4f8a98eed7838d29fa31a6f34d11f6d8887c7f) )
ROM_END
ROM_START( g_sho00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sho00.bin", 0x000000, 0x080000,   CRC(cb2201a3) SHA1(ecfd7b3bf4dcbee472ddf2f9cdbe968a05b814e0) )
ROM_END
ROM_START( g_shyde )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shyde.bin", 0x000000, 0x080000,  CRC(1fe2d90b) SHA1(6c0a4b72b90ecfe8c324691bc6e54243746043c1) )
ROM_END
ROM_START( g_shydj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shydj.bin", 0x000000, 0x080000,  CRC(599be386) SHA1(76bceb5915f3546c68c1fdb5fb0e205cd56a3fe6) )
ROM_END
ROM_START( g_smgp03 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smgp03.bin", 0x000000, 0x080000,   CRC(725018ee) SHA1(1947d41598daa3880ecb826303abae2accd1857f) )
ROM_END

ROM_START( g_smgp00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smgp00.bin", 0x000000, 0x080000,  CRC(90f9bab3) SHA1(631b72e27b394ae6b5a1188dfa980333fc675379) )
ROM_END
ROM_START( g_smgp01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_smgp01.bin", 0x000000, 0x080000,  CRC(b1823595) SHA1(ed6f80546a7847bf06cf4a62b34d1c3b989e4d3e) )
ROM_END
ROM_START( g_srealj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_srealj.bin", 0x000000, 0x080000,  CRC(4346e11a) SHA1(c86725780027ef9783cb7884c8770cc030b0cd0d) )
ROM_END
ROM_START( g_sshin2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sshin2.bin", 0x000000, 0x100000,  CRC(5b412816) SHA1(e3dbe326aa93320d405c561c306fc3954ab8ea7c) )
ROM_END
ROM_START( g_stb00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stb00.bin", 0x000000, 0x080000,  CRC(8bd77836) SHA1(f40bef7a5a2e414d87335e3e56a2c34fb4f83fca) )
ROM_END
ROM_START( g_svolua )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_svolua.bin", 0x000000, 0x040000,  CRC(85102799) SHA1(3f00e7f8de50e73346cba06dd44b6dd9461d5c9c) )
ROM_END
ROM_START( g_supmne )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_supmne.bin", 0x000000, 0x100000,  CRC(7db434ba) SHA1(c956730af44b737ee3d0c1e83c147f32e3504383) )
ROM_END
ROM_START( g_swsoj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_swsoj.bin", 0x000000, 0x080000,  CRC(58edb3f3) SHA1(9c17187e3eb0842c5300f69616a58baa86b769e7) )
ROM_END
ROM_START( g_swvej )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_swvej.bin", 0x000000, 0x0a0000,  CRC(e400dfc3) SHA1(697fb165051179a2bbca77c8cfd0c929e334f8c1) )
ROM_END

ROM_START( g_pachin )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pachin.bin", 0x000000, 0x100000,  CRC(9d137e7f) SHA1(84c3395c6dc123737d9da94e0392f3f30be59182) )
ROM_END
ROM_START( g_pageme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pageme.bin", 0x000000, 0x200000,  CRC(79a180e2) SHA1(c4cf7695127b7ec5e003b5e7e15fdbfb172d1fcb) )
ROM_END
ROM_START( g_pboyj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pboyj.bin", 0x000000, 0x080000,  CRC(e14250ae) SHA1(3384e238edaf8b2f6950cd1660864599e476d29f) )
ROM_END
ROM_START( g_pblbej )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pblbej.bin", 0x000000, 0x180000,  CRC(96ed2e5d) SHA1(dee8430f4f15b4d04e1e3c0cc9d7d7f55ca1ad7b) )
ROM_END
ROM_START( g_pste03 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pste03.bin", 0x000000, 0x100000,  CRC(aa8b19bc) SHA1(3f73dcabcdf0d781834fcc673533301b82f0e91b) )
ROM_END
ROM_START( g_pgat01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pgat01.bin", 0x000000, 0x080000,  CRC(0489ff8e) SHA1(73935bfbdf63d3400284a16e464286b7630964aa) )
ROM_END
ROM_START( g_pga2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pga2j.bin", 0x000000, 0x100000,  CRC(c05b7a4a) SHA1(a5896f2f019530929194a6d80828d18b859b9174) )
ROM_END
ROM_START( g_pga200 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pga200.bin", 0x000000, 0x100000,  CRC(16b2d816) SHA1(cab753b958b336dab731407bd46429de16c6919f) )
ROM_END
ROM_START( g_phst2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_phst2b.bin", 0x000000, 0x0c0000,  CRC(e6688b66) SHA1(ab62fac112cb4dd1f19cb973bd7952b161f6d100) )
ROM_END
ROM_START( g_phst3b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_phst3b.bin", 0x000000, 0x0c0000,  CRC(2e9b4c23) SHA1(59ccfc6b85b95666d0e2c85e3e08c847c4a7ad34) )
ROM_END


ROM_START( g_phelie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_phelie.bin", 0x000000, 0x080000,  CRC(13abc2b2) SHA1(5b95529dd491266648134f1808f4da55a2ab9296) )
ROM_END
ROM_START( g_phelij )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_phelij.bin", 0x000000, 0x080000,  CRC(94596174) SHA1(9834c9963debebb6ee80ffac4191ff260b85e511) )
ROM_END
ROM_START( g_puggse )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_puggse.bin", 0x000000, 0x100000,  CRC(5d5c9ade) SHA1(89a347f35c4d0d364c284f2f32f708d0378b8735) )
ROM_END
ROM_START( g_quac00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_quac00.bin", 0x000000, 0x080000,  CRC(88c8dd94) SHA1(3acacac6c20d9b5f9ddeacb82b82b3039c4e2485) )
ROM_END
ROM_START( g_quac01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_quac01.bin", 0x000000, 0x140000,  CRC(a5c946b3) SHA1(1b1421f57669e149ccced2d751a6de83ca941932) )
ROM_END
ROM_START( g_ram300 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ram300.bin", 0x000000, 0x040000,  CRC(2232f03d) SHA1(ece53156a43fe14597a44f36a4e65e13cfc6d9d5) )
ROM_END
ROM_START( g_ranxe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ranxe.bin", 0x000000, 0x100000,  CRC(b8c04804) SHA1(53908ee1c47f0d9d49e81fc295379552e2198948) )
ROM_END
ROM_START( g_rastj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rastj.bin", 0x000000, 0x080000,  CRC(ebacfb5a) SHA1(e7bd8367ae962af48a2e937bf6ced253f428b03d) )
ROM_END
ROM_START( g_rshi00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rshi00.bin", 0x000000, 0x080000,  CRC(5c7e5ea6) SHA1(d5807a44d2059aa4ff27ecb7bdc749fbb0382550) )
ROM_END
ROM_START( g_rshi01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rshi01.bin", 0x000000, 0x080000,  CRC(05f27994) SHA1(a88546e159bc882bf05eba66c09a3b21dee4154e) )
ROM_END


ROM_START( g_rshi02 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rshi02.bin", 0x000000, 0x080000,  CRC(fe91ab7e) SHA1(76af6ca97df08f2370f3a7fc2311781a0f147f4b) )
ROM_END
ROM_START( g_ristj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ristj.bin", 0x000000, 0x200000,  CRC(ce464f0e) SHA1(a4dd8dc0b673afe7372ba9c05c8d6090302dcf4e) )
ROM_END
ROM_START( g_roadbj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_roadbj.bin", 0x000000, 0x080000, CRC(a0015440) SHA1(7127e76ea33512d14cd78e6cac72ff8578454e1e) )
ROM_END
ROM_START( g_rnrre )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rnrre.bin", 0x000000, 0x100000,  CRC(bc5a0562) SHA1(c4d84425c007711f006a67d6577fd90f421605e5) )
ROM_END
ROM_START( g_rkadve )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rkadve.bin", 0x000000, 0x100000,  CRC(8eedfd51) SHA1(cb741f868b0eba17819a27e37cda96f33674c342) )
ROM_END
ROM_START( g_rkadvj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rkadvj.bin", 0x000000, 0x100000,  CRC(d1c8c1c5) SHA1(76d33c8549cd86cb1a4d17ac6eb8dd47c2f07d03) )
ROM_END
ROM_START( g_rkmnja )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rkmnja.bin", 0x000000, 0x200000,  CRC(4d87235e) SHA1(4dfcf5c07106f6db4d8b7e73a053bb46a21686c3) )
ROM_END
ROM_START( g_rthn2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rthn2e.bin", 0x000000, 0x100000,  CRC(c440f292) SHA1(e8ae09b2945eac555d6f0d39a1f768724b221357) )
ROM_END
ROM_START( g_rthn2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rthn2j.bin", 0x000000, 0x100000,  CRC(965b2628) SHA1(6e15ce792fbe01b918b197deea320f953aa989cc) )
ROM_END
ROM_START( g_ron98b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ron98b.bin", 0x000000, 0x200000, CRC(dd27c84e) SHA1(183ba7d8fe6bcddf0b7738c9ef2aa163725eb261)) // clone of iss
ROM_END

ROM_START( g_tfhxj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tfhxj.bin", 0x000000, 0x100000,  CRC(e9a54eed) SHA1(d365d22d9076e98966eb12fdb1d93c3c101f519e) )
ROM_END
ROM_START( g_tsbwlj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsbwlj.bin", 0x000000, 0x100000,  CRC(90c6e20c) SHA1(477880e7976ac0f7203fddabba4a6e8799aa604d) )
ROM_END
ROM_START( g_tsbwlu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsbwlu.bin", 0x000000, 0x100000,  CRC(bd5933ee) SHA1(529b8e86b97c326592540f5e427198a205c127d0) )
ROM_END
ROM_START( g_tsbw2j )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsbw2j.bin", 0x000000, 0x200000,  CRC(32fb633d) SHA1(cb9e8cc1651b719054f05e1e1a9e0fbbc3876ebd) )
ROM_END
ROM_START( g_tsnbaj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tsnbaj.bin", 0x000000, 0x100000,   CRC(79f33eb6) SHA1(07f160e6eb7e358f54e4fdabfc95bb5525c57fc9) )
ROM_END
ROM_START( g_tmntj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tmntj.bin", 0x000000, 0x100000,  CRC(1b003498) SHA1(f64556be092a13de8eaacc78dd630ac9d7bb75ee) )
ROM_END
ROM_START( g_tmnttj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tmnttj.bin", 0x000000, 0x200000,  CRC(8843f2c9) SHA1(5be86d03abdb49b824104e9bbf0ac80023e4908c) )
ROM_END
ROM_START( g_telerb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_telerb.bin", 0x000000, 0x040000,  CRC(1db99045) SHA1(917c0f72e21c98ee2ee367c4d42c992c886a3aa2) )
ROM_END
ROM_START( g_tfoxj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tfoxj.bin", 0x000000, 0x100000,  CRC(eca6cffa) SHA1(4e6dc58327bdbb5d0d885b3cecf89fe139b32532) )
ROM_END
ROM_START( g_tdom1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tdom1.bin", 0x000000, 0x100000,  CRC(7eba7a5c) SHA1(c9358fb57314be5792af5e97748f7b886a7194d2) )
ROM_END

ROM_START( g_tkille )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tkille.bin", 0x000000, 0x200000,  CRC(a4f48a1a) SHA1(5bc883edd092602aac162b42462442e462d3c881) )
ROM_END
ROM_START( g_ttabe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ttabe.bin", 0x000000, 0x080000,  CRC(d10fba51) SHA1(9e63b150cc2ec0ef141e68ffda862aa8db604441) )
ROM_END
ROM_START( g_toddj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_toddj.bin", 0x000000, 0x080000,  CRC(7ff5529f) SHA1(02cf05687a7f3f8177b2aff0a0cfbd5c490e557d) )
ROM_END
ROM_START( g_tajua )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tajua.bin", 0x000000, 0x100000, CRC(b9992e1c) SHA1(a785c642bdcce284e6a607bb68e3993a176f4361) )
ROM_END
ROM_START( g_turma )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_turma.bin", 0x000000, 0x100000,  CRC(f8288de1) SHA1(80fc2a6a6b8b943f781598094f3b5a5fe4f05ede) )
ROM_END

ROM_START( g_clascl )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_clascl.bin", 0x000000, 0x200000,  CRC(73f948b1) SHA1(80629bb91a1123ae832f6997f9f3c0e070ce81ca) )
ROM_END
ROM_START( g_gen6pk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gen6pk.bin", 0x000000, 0x300000, CRC(1a6f45dc) SHA1(aa276048f010e94020f0c139dcf5829fb9ea6b21) )
ROM_END
ROM_START( g_mg2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mg2e.bin", 0x000000, 0x200000, CRC(30d59f2f) SHA1(1cf425556de9352c1efe60b07e592b610e78cfbd) )
ROM_END
ROM_START( g_mg3e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mg3e.bin", 0x000000, 0x200000, CRC(b4247d98) SHA1(4329d7fa2c4b8e6ed0b4adfc64dca474d0ee5a51) )
ROM_END
ROM_START( g_mg6v1e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mg6v1e.bin", 0x000000, 0x300000, CRC(b66fb80d) SHA1(42f29050e071c3086be35452d917debf25648b23) )
ROM_END
ROM_START( g_mg6v2e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mg6v2e.bin", 0x000000, 0x300000, CRC(e8d10db9) SHA1(1fc777b946874216add624414ef6f5878bab202b) )
ROM_END
ROM_START( g_mg6v3e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mg6v3e.bin", 0x000000, 0x300000, CRC(fe3e7e4f) SHA1(cca818d624e95c2d07cfc1b22c44eb53e4bdcd02) )
ROM_END
ROM_START( g_mg10i1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mg10i1.bin", 0x000000, 0x400000, CRC(c19ae368) SHA1(791a4de13cfe2af4302aac969753d67dd76e89c8) )
ROM_END
ROM_START( g_mg1e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mg1e.bin", 0x000000, 0x100000, CRC(db753224) SHA1(076df34a01094ce0893f32600e24323567e2a23b) )
ROM_END
ROM_START( g_menace )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_menace.bin", 0x000000, 0x100000, CRC(936b85f7) SHA1(2357ed0b9da25e36f6a937c99ebc32729b8c10d2) )
ROM_END

ROM_START( g_segsp1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_segsp1.bin", 0x000000, 0x280000, CRC(07fedaf1) SHA1(ff03bb2aced48c82de0ddfb048b114ec84daf16a) )
ROM_END
ROM_START( g_stop5b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stop5b.bin", 0x000000, 0x200000,  CRC(61069069) SHA1(b13ba8e49fb9e8b41835e3d4af86ad25f27c804c) )
ROM_END
ROM_START( g_soncle )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soncle.bin", 0x000000, 0x300000, CRC(8c70b84e) SHA1(109e6d7b31d00fbdb9d4ecb304d4ea19e96c8607) )
ROM_END
ROM_START( g_sonclu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sonclu.bin", 0x000000, 0x300000,  CRC(95b5e8d7) SHA1(2196cbe754cfcd1d7bbbd0a8a45ae44de4deb2fb) )
ROM_END
ROM_START( g_sptgb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sptgb.bin", 0x000000, 0x200000, CRC(7e3ecabf) SHA1(41604a07b0ac7dff9e01e6829cf84ca911620729) )
ROM_END

ROM_START( g_adamb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_adamb.bin", 0x000000, 0x100000, CRC(2803a5ca) SHA1(34deb427637d0458ce229b4fe3e55ad7901a0a34) )
ROM_END
ROM_START( g_mmaxe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmaxe.bin", 0x000000, 0x100000, CRC(24f1a3bb) SHA1(e60eec1d39b32ce5cc2125cffd3016b4070a65c3) )
ROM_END
ROM_START( g_aateb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aateb.bin", 0x000000, 0x080000, CRC(3bbf700d) SHA1(f4a05902002273788572fcd49502db44e5dff963) )
ROM_END

ROM_START( g_atpte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_atpte.bin", 0x000000, 0x200000, CRC(1a3da8c5) SHA1(c5fe0fe967369e9d9e855fd3c7826c8f583c49e3) )
ROM_END
ROM_START( g_awsep )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_awsep.bin", 0x000000, 0x200000, CRC(0158dc53) SHA1(8728f7d18c75e132b98917b96ade5d42c0d4a0cb) )
ROM_END
ROM_START( g_bobb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bobb.bin", 0x000000, 0x100000, CRC(e3e8421e) SHA1(eb5e4221b13372f2155ac80e88eb1114a6e2ccc7) )
ROM_END
ROM_START( g_babyb1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_babyb1.bin", 0x000000, 0x100000, CRC(459b891c) SHA1(99983282e75e1cf01a47d00f0be15e20eaae0907) )
ROM_END
ROM_START( g_babyb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_babyb2.bin", 0x000000, 0x100000, CRC(b2e7cc49) SHA1(5cc32a2826b3cc5c581043fe6c481ffb753321dd) )
ROM_END
ROM_START( g_barbvb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_barbvb.bin", 0x000000, 0x100000, CRC(10e0ba69) SHA1(51fe102543e0419a9ccd0f3e016150fdb3666c24) )
ROM_END
ROM_START( g_bk2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bk2b.bin", 0x000000, 0x130000, CRC(0cf2acbe) SHA1(ee3bee676944bc5a3cab163c209825f678dc8204) )
ROM_END
ROM_START( g_bk3b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bk3b.bin", 0x000000, 0x300000, CRC(e7ff99db) SHA1(1d75372571ce970545f6ce63977cea9fa811e23f) )
ROM_END
ROM_START( g_batme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_batme.bin", 0x000000, 0x080000, CRC(61c60c35) SHA1(c7279c6d45e6533f9de14f65098c289b7534beb3) )
ROM_END
ROM_START( g_beavib )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_beavib.bin", 0x000000, 0x200000, CRC(81ed5335) SHA1(f9ed871867f543a8d94c47270cd530a5ca410664) ) // noboot
ROM_END

ROM_START( g_botbb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_botbb.bin", 0x000000, 0x100000, CRC(f842240b) SHA1(c7ffaa2af35d0365340075555b6a5c3f252d33b3) )
ROM_END
ROM_START( g_bzerot )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bzerot.bin", 0x000000, 0x200000, CRC(c61ed2ed) SHA1(663a1a6c7e25ba05232e22f102de36c1af2f48f8) )
ROM_END
ROM_START( g_biohzb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_biohzb.bin", 0x000000, 0x100000, CRC(dd10dd1a) SHA1(1445b0babb52d252bf822d8d2eec0eda05b63229) )
ROM_END
ROM_START( g_blam2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blam2b.bin", 0x000000, 0x100000, CRC(08f78c70) SHA1(973cd6b8c7d29c9d14ac45631b99fd4ae4ca5713) )
ROM_END
ROM_START( g_blocb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blocb2.bin", 0x000000, 0x400000, CRC(4385e366) SHA1(1a06213a3a26c9105fc3013a141c22d212045a0b) ) // noboot
ROM_END
ROM_START( g_bcounb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bcounb.bin", 0x000000, 0x100000,  CRC(b4ffb6ce) SHA1(a01971cdce4d98e770d9c083a5accb8cb5260112) )
ROM_END
ROM_START( g_blcrb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blcrb.bin", 0x000000, 0x100000, CRC(90f5c2b7) SHA1(0f4bec98af08027fc034b9801c28bb299a04be35) )
ROM_END
ROM_START( g_brutle )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_brutle.bin", 0x000000, 0x200000, CRC(7e9a8d32) SHA1(8667fa820e90911f12b682fcd1ac870b84b6b60b) )
ROM_END
ROM_START( g_bubbab )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_bubbab.bin", 0x000000, 0x100000,  CRC(a8731cb4) SHA1(3e10ed88ac29b084d3665579a79c23ff2921b832) )
ROM_END
ROM_START( g_burnfe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_burnfe.bin", 0x000000, 0x080000, CRC(776ff6ff) SHA1(a25930ee55a2d88838e3999fb5939d9392fd0efa) )
ROM_END



ROM_START( g_caeno )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_caeno.bin", 0x000000, 0x100000, CRC(69796e93) SHA1(4a5b9169262caf81fb7ae76fc3835107769f28a1) ) // no sound?
ROM_END
ROM_START( g_caeno2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_caeno2.bin", 0x000000, 0x100000, CRC(4f327b3a) SHA1(cdb2f47bde3ff412c7b1f560637f2ccec023980f) ) // warrior of rome 2
ROM_END
ROM_START( g_capamb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_capamb.bin", 0x000000, 0x100000, CRC(baac59c0) SHA1(af8271c0c99637413006bdd8c66a7b5412e0ebd7) )
ROM_END
ROM_START( g_capame )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_capame.bin", 0x000000, 0x100000, CRC(43225612) SHA1(66ec647175251d8c109c6c21440d415e13e14001) )
ROM_END
ROM_START( g_cappb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cappb.bin", 0x000000, 0x080000, CRC(bf2cbd3a) SHA1(750770bb43a2d1310b9011f7588cc91e0476826b) )
ROM_END
ROM_START( g_cvtngb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cvtngb.bin", 0x000000, 0x100000, CRC(84cd103a) SHA1(8c4004c0d8cbe211ffa3919fd609653221a0078b) )
ROM_END
ROM_START( g_cengb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cengb.bin", 0x000000, 0x100000, CRC(3fb045c2) SHA1(9e2e12e90b60e6ca28af6a5afb213cafe1868a95) )
ROM_END
ROM_START( g_chk2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chk2b.bin", 0x000000, 0x100000, CRC(d6a3b324) SHA1(12faedb14e8ce02461b3529cce81ac9d4e854110) )
ROM_END
ROM_START( g_chuck )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_chuck.bin", 0x000000, 0x060000, CRC(6360ee58) SHA1(191726e84fb80ccb992d7cf4188008e23f644432) )
ROM_END
ROM_START( g_clifhb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_clifhb.bin", 0x000000, 0x100000, CRC(628251fd) SHA1(af567a1ea2fe5ef080352f74d4b70a4e0a33f319) ) // crash
ROM_END


ROM_START( g_clifhe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_clifhe.bin", 0x000000, 0x100000, CRC(35bff1fd) SHA1(c453130b2789fa5682367f9c071206e06952b951) )
ROM_END
ROM_START( g_comacb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_comacb.bin", 0x000000, 0x080000, CRC(84560d5a) SHA1(0492bcd5341e768455bd353e786fac74837289af) )
ROM_END
ROM_START( g_comixb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_comixb.bin", 0x000000, 0x200000, CRC(2efcb6ee) SHA1(af73f6d0d9e54416496a39dbfafc610bf16b3c0c) )
ROM_END
ROM_START( g_congob )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_congob.bin", 0x000000,  0x0f7d36, CRC(13746716) SHA1(976d7610eb80691f50466f5102ccd49cc3a2b9f7) ) // odd size
ROM_END

ROM_START( g_spotb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_spotb.bin", 0x000000, 0x100000, CRC(0ebaa4a8) SHA1(9b42bb33186ddc759a469ed3e0ee12e5dee0b809) )
ROM_END
ROM_START( g_cyjusb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_cyjusb.bin", 0x000000, 0x080000, CRC(91daf11e) SHA1(27c2932b2e25b17fdf5f2ebd74ebbde4015118b6) )
ROM_END
ROM_START( g_daffyb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_daffyb.bin", 0x000000, 0x200000, CRC(0eaa4740) SHA1(7d869208846ce10ccf799c581cb09034c81aa3e5) )
ROM_END
ROM_START( g_daik2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_daik2.bin", 0x000000, 0x200000, CRC(e040f0da) SHA1(74f61092067d82127cae3306d5a66d3efe946bc3) )
ROM_END
ROM_START( g_dari2a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dari2a.bin", 0x000000, 0x100000, CRC(0140ab58) SHA1(5824936a1a8696d8363431f31293d4a1a1d0b69b) )
ROM_END
ROM_START( g_dashb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dashb.bin", 0x000000, 0x100000, CRC(adaffc3f) SHA1(4b04d7f6fc731275d9fcf3b7566674d4987c2154) )
ROM_END

ROM_START( g_dwctb1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dwctb1.bin", 0x000000, 0x200000, CRC(6f4183c0) SHA1(3c8527d1a01a4c7ab97b2664046f9c11283e3332) )
ROM_END
ROM_START( g_dwctb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dwctb2.bin", 0x000000, 0x200000, CRC(76f2bed8) SHA1(5b548c9794a8af0d9fba4d7abfb6ca96a7f01709) )
ROM_END
ROM_START( g_dwctb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dwctb.bin", 0x000000, 0x100000, CRC(7c6b0646) SHA1(bce5937d930e016bb14eca80c833071df4bdc0f8) )
ROM_END
ROM_START( g_dazeb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dazeb.bin", 0x000000, 0x200000, CRC(317c9491) SHA1(a63cdb54c31b1c1219467987a81e761ee92feee3) )
ROM_END
ROM_START( g_demomb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_demomb.bin", 0x000000, 0x200000, CRC(57ffad7a) SHA1(33a33bff7277c2aab45d0d843d13728ce2c62ab2) )
ROM_END

ROM_START( g_dominu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dominu.bin", 0x000000, 0x0c0000, CRC(30006ebd) SHA1(fde8160bf51575463e81cbd0b5d12fb5d49eb695) )
ROM_END
ROM_START( g_beanb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_beanb.bin", 0x000000, 0x100000, CRC(4d0e5273) SHA1(312f9a283bebc5d612a63afd2cf67eb923f4f074) )
ROM_END

ROM_START( g_drgble )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_drgble.bin", 0x000000, 0x200000, CRC(fdeed51d) SHA1(d6fb86d73e1abc7b7f1aecf77a52fa3f759aedb1) )
ROM_END
ROM_START( g_duneg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_duneg.bin", 0x000000, 0x100000, CRC(39790728) SHA1(55996cd262df518e92271bceee4d2a657cd7e02c) )
ROM_END
ROM_START( g_dunee )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dunee.bin", 0x000000, 0x100000, CRC(b58ae71d) SHA1(133cc86b43afe133fc9c9142b448340c17fa668e) )
ROM_END
ROM_START( g_dynabr )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dynabr.bin", 0x000000, 0x100000, CRC(360c1b20) SHA1(7e57c6fd5c20e356f9c967dbf168db53574eda84) )
ROM_END
ROM_START( g_dheadb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_dheadb.bin", 0x000000, 0x200000, CRC(5c25e934) SHA1(9889510234dd8771fd14c4022710028c5928d152) )
ROM_END
ROM_START( g_ecco2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ecco2b.bin", 0x000000, 0x200000, CRC(1d1470ea) SHA1(364523cd30615ce4a94793ebbc189c0db6adc38f) )
ROM_END

ROM_START( g_e_nhb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_e_nhb.bin", 0x000000, 0x200000, CRC(a427814a) SHA1(c890fc232ca46a9499bb4e3107a519ae2d8edb81) )
ROM_END
ROM_START( g_etchmb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_etchmb.bin", 0x000000, 0x300000, CRC(e0665f06) SHA1(c06f2d9ff29b6e6ec2dbec06eab2eac21e80e423) )
ROM_END
ROM_START( g_exosb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_exosb.bin", 0x000000, 0x100000, CRC(70edf964) SHA1(d165e0057a5d290ae02ff6cae9f99c2ebc8a3f7c) )
ROM_END


ROM_START( g_exrnzb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_exrnzb.bin", 0x000000, 0x100000, CRC(c642fdf4) SHA1(f1c883f3bc6343c8a8181261ead3a534520a447f) )
ROM_END
ROM_START( g_f1wceb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f1wceb.bin", 0x000000, 0x200000,  CRC(2269ed6b) SHA1(6b9984e92eee0044ac245ea534ed667425747e7b) )
ROM_END
ROM_START( g_f15b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f15b.bin", 0x000000, 0x100000,  CRC(fd4f5a01) SHA1(2ee364462f3206d220bbcebe50ca35e039967cf2) )
ROM_END
ROM_START( g_fatfue )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fatfue.bin", 0x000000, 0x180000, CRC(2e730a91) SHA1(e0a74238cd4592d900f88d033314c668d1522b72) )
ROM_END

ROM_START( g_fergpb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fergpb.bin", 0x000000, 0x100000, CRC(d48d02d4) SHA1(133e362ab58a309d5b77fa0544423591cc535ec9) )
ROM_END
ROM_START( g_funnge )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_funnge.bin", 0x000000, 0x100000, CRC(da4ab3cd) SHA1(3677dfe5450c0800d29cfff31f226389696bfb32) )
ROM_END
ROM_START( g_fut98 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_fut98.bin", 0x000000, 0x200000, CRC(5c015888) SHA1(44adb1cf6c1ce6e53314dc336168c5cf3313a739) )
ROM_END
ROM_START( g_glf200 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_glf200.bin", 0x000000, 0x100000, CRC(cae883c5) SHA1(d4143bf5f49b0d03f4b8fe270c2ecc23fa6627e0) )
ROM_END
ROM_START( g_gaun4a )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gaun4a.bin", 0x000000, 0x100000, CRC(f9d60510) SHA1(d28e22207121f0e2980dd409b4fb24f9fb8967ae) )
ROM_END



ROM_START( g_glocb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_glocb.bin", 0x000000, 0x100000,  CRC(175c7e63) SHA1(ccbedc9d3c05e212e4e2bea6824999a3c1fd2006) )
ROM_END
ROM_START( g_godsb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_godsb.bin", 0x000000, 0x100000, CRC(2c06bb64) SHA1(dd9c03eaf3160303775ca1bca048101614507203) )
ROM_END
ROM_START( g_gax2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_gax2b.bin", 0x000000, 0x080000, CRC(e62ea1bb) SHA1(97e49394321e97b8488e6cf57bade4dc20aec0f4) )
ROM_END
ROM_START( g_helfie )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_helfie.bin", 0x000000, 0x080000, CRC(cf30acec) SHA1(3cee325f4e8d12157b20c4ca7093bf806f5f9148) )
ROM_END

ROM_START( g_homeab )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_homeab.bin", 0x000000, 0x100000,  CRC(3a235fb9) SHA1(1ace5abd524c6ab5b434374c51f64f59dbd6ec7a) )
ROM_END
ROM_START( g_hdunkb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hdunkb.bin", 0x000000, 0x200000, CRC(db124bbb) SHA1(87a2d58614fab2799e2a4456a08436fc7acc745b) )
ROM_END
ROM_START( g_micdo )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_micdo.bin", 0x000000, 0x100000, CRC(cb9ee238) SHA1(c79935fdeb680a1a2e76db1aef2c6897e6ee8e4d) )
ROM_END
ROM_START( g_icdb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_icdb.bin", 0x000000, 0x100000, CRC(623a920f) SHA1(6c1db6d8ec7ace551b70ee60026e4a553d50f964) )
ROM_END

ROM_START( g_icftd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_icftd.bin", 0x000000, 0x080000, CRC(25afb4f7) SHA1(245816c9744552856fc2745253db7b37ce9d9251) )
ROM_END
ROM_START( g_jbondw )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jbondw.bin", 0x000000, 0x080000,  CRC(4e614548) SHA1(847de1168bf737ac7faa4f77eb0749738014ed87) )
ROM_END
ROM_START( g_jdrdb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jdrdb.bin", 0x000000, 0x200000, CRC(8d46f4da) SHA1(84bb0ee7c42612abfd41c5676e7ecb1b828ee42a) )
ROM_END
ROM_START( g_jdrdba )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jdrdba.bin", 0x000000, 0x200000, CRC(e649f784) SHA1(21210afaa9a53762108936e28380a9075b7c5c05) )
ROM_END
ROM_START( g_jstrkb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jstrkb.bin", 0x000000, 0x200000, CRC(0cd540d4) SHA1(b76be43101e72100b63c107d1710fa8f9ad4cfd6) )
ROM_END
ROM_START( g_junkb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_junkb.bin", 0x000000, 0x100000, CRC(23534949) SHA1(81a9b6d29878b4e9313a887b5349c12846b9d77f) )
ROM_END
ROM_START( g_jparkb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_jparkb.bin", 0x000000, 0x200000, CRC(cf890eed) SHA1(ac2097b5f2a30787d7ca1ed8ab4eac7f4be77f0f) )
ROM_END
ROM_START( g_kawab )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kawab.bin", 0x000000, 0x100000, CRC(55934d1b) SHA1(a66446d5c3d07ce211d7198faf4ea3ff6dbfa0b9) )
ROM_END
ROM_START( g_landsb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_landsb.bin", 0x000000, 0x200000, CRC(70483d03) SHA1(dfca19397479852584d4ac6fcbe27412f9bc1af0) )
ROM_END

ROM_START( g_landsf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_landsf.bin", 0x000000, 0x200000, CRC(5de7d917) SHA1(86db4b22b54e8583e35717927ad66b7535bf33b4) )
ROM_END
ROM_START( g_lng201 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lng201.bin", 0x000000, 0x200000, CRC(0caa0593) SHA1(4ba6591dfd85aa75ffe8dc21137a9291aa7f5603) )
ROM_END

ROM_START( g_lostvb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lostvb.bin", 0x000000, 0x100000, CRC(17bed25f) SHA1(375eaa9845692db4fdbd0b51985aa0892a8fe425) )
ROM_END
ROM_START( g_lostve )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lostve.bin", 0x000000, 0x100000, CRC(1f14efc6) SHA1(c977a21d287187c3931202b3501063d71fcaf714) )
ROM_END
ROM_START( g_lotu2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_lotu2b.bin", 0x000000, 0x100000, CRC(2997b7d4) SHA1(e108d612735dcc768bed68a58d6fd45d71de565d) )
ROM_END
ROM_START( g_mmfb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmfb.bin", 0x000000, 0x200000, CRC(0273e564) SHA1(42b871c3a91697e1d7b8c6f1eae9d2a8b07a0fca) )
ROM_END
ROM_START( g_mazib )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mazib.bin", 0x000000, 0x100000, CRC(36459b59) SHA1(3509fdf8bd0a589c216a5032eef0c09a51c7a578) )
ROM_END
ROM_START( g_mcdtlb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mcdtlb.bin", 0x000000, 0x100000, CRC(7bf477e8) SHA1(ddf496f9a95b2963fe50f3bcaef8e3b592e2fc64) )
ROM_END
ROM_START( g_mcdtle )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mcdtle.bin", 0x000000, 0x100000, CRC(6ab6a8da) SHA1(f6178018102df3c92d05a48ff5949db9416acd5c) )
ROM_END

ROM_START( g_mlom01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mlom01.bin", 0x000000, 0x100000, CRC(ab9fed30) SHA1(3794cb708b1f54675eb6fb272cfee01e6dbcecc1) )
ROM_END
ROM_START( g_mswive )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mswive.bin", 0x000000, 0x100000, CRC(78c2f046) SHA1(6396fb0f204c9f23d0af0b39d069ff0883e191aa) )
ROM_END
ROM_START( g_megme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_megme.bin", 0x000000, 0x200000, CRC(dcf6e8b2) SHA1(ea9ae2043c97db716a8d31ee90e581c3d75f4e3e) )
ROM_END
ROM_START( g_micmab )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_micmab.bin", 0x000000, 0x100000,  CRC(08c2af21) SHA1(433db3e145499ebad4f71aae66ee6726ef30d5db) )
ROM_END
ROM_START( g_mmanib )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mmanib.bin", 0x000000, 0x200000, CRC(7fc1bdf0) SHA1(cd80dcd53c9996744eb94ba8702f84d784e83f34) )
ROM_END
ROM_START( g_micm96 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_micm96.bin", 0x000000, 0x100000, CRC(3137b3c4) SHA1(7e59fc5800137cea3d4526ee549f7caef150dcaa) )
ROM_END
ROM_START( g_micmc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_micmc.bin", 0x000000, 0x080000,  CRC(54e4cff1) SHA1(0d4d023a1f9dc8b794bd60bf6e70465b712ffded) )
ROM_END
ROM_START( g_mima3b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mima3b.bin", 0x000000, 0x200000, CRC(6ef7104a) SHA1(d366d05644eb59a14baf3c2e7281c1584630c021) )
ROM_END
ROM_START( g_monob )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_monob.bin", 0x000000, 0x080000, CRC(dfbcc3fa) SHA1(ca808cc27e3acb7c663a8a229aed75cb2407366e) )
ROM_END
ROM_START( g_mahbb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mahbb.bin", 0x000000, 0x100000, CRC(7b852653) SHA1(f509c3a49cf18af44da3e4605db8790a1ab70a32) )
ROM_END

ROM_START( g_mahbe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_mahbe.bin", 0x000000, 0x100000, CRC(8ea4717b) SHA1(d79218ef96d2f1d9763dec2a9e6ad7c2907d024b) )
ROM_END
ROM_START( g_nba94b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nba94b.bin", 0x000000, 0x200000, CRC(6643a308) SHA1(e804ca0f4da505056f0813f00df9b139248f59af) )
ROM_END
ROM_START( g_nhl96e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nhl96e.bin", 0x000000, 0x200000, CRC(9821d0a3) SHA1(085fb8e6f0d2ff0f399de5c57eb13d9c9325dbae) )
ROM_END
ROM_START( g_nhlp00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nhlp00.bin", 0x000000, 0x080000, CRC(cbbf4262) SHA1(8efc1cacb079ea223966dda065c16c49e584cac2) )
ROM_END
ROM_START( g_nigwce )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_nigwce.bin", 0x000000, 0x100000, CRC(3fe3d63b) SHA1(81b6d231998c0dd90caa9325b9cc6e50c6e622bb) )
ROM_END
ROM_START( g_ncircb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ncircb.bin", 0x000000, 0x200000, CRC(31de5a94) SHA1(61b7d25e15011d379d07a8dad4f8c5bb5f75e29f) )
ROM_END
ROM_START( g_ngaidb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ngaidb.bin", 0x000000, 0x100000, CRC(0d7f74ba) SHA1(dced2e4eb53ae5d9e979b13f093f70d466634c5b) )
ROM_END
ROM_START( g_ottifb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ottifb.bin", 0x000000, 0x100000, CRC(c6e3dd23) SHA1(76d2606d12a94b10208324ad1ff3e295ef89f5f4) )
ROM_END
ROM_START( g_ootwb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ootwb.bin", 0x000000, 0x100000, CRC(3aad905a) SHA1(9698cc5acc0031e26d71bb074742295142dafdb9) )
ROM_END
ROM_START( g_outlnb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_outlnb.bin", 0x000000, 0x100000, CRC(c5ba7bbf) SHA1(3b59fb2bfd94de10fdeec1222f27284bb81217c3) )
ROM_END

ROM_START( g_2019b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_2019b.bin", 0x000000, 0x100000, CRC(d2ecddfa) SHA1(6aca01bd23b99ffbcbd135f7c1dcaa42b8f89e61) )
ROM_END
ROM_START( g_pagemb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pagemb.bin", 0x000000, 0x200000, CRC(29895e3d) SHA1(8a81ea7fca88fffff3fc1e932019e36e71be4978) )
ROM_END
ROM_START( g_pblbee )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pblbee.bin", 0x000000, 0x200000, CRC(6cfc7297) SHA1(53e8e8f11fb9950732409b770b7d9c8dabe85a19) )
ROM_END
ROM_START( g_pst201 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pst201.bin", 0x000000, 0x0c0000, CRC(0d07d0ef) SHA1(fcd032ded2235171f51db316ad1b7688fbbdafe4) )
ROM_END
ROM_START( g_pinkb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pinkb.bin", 0x000000, 0x100000, CRC(56087cff) SHA1(5f2631b1850875129efd98d501f6419eb12b9817) )
ROM_END
ROM_START( g_pinnoe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pinnoe.bin", 0x000000, 0x300000, CRC(28014bdc) SHA1(7b6c9fdd201341f5319be85b5042870a84e82ae0) )
ROM_END
ROM_START( g_pirdwu )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pirdwu.bin", 0x000000, 0x200000, CRC(0a62de34) SHA1(55496714f32eb7f57335201f90b3f437a1500c49) )
ROM_END
ROM_START( g_pgoldb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pgoldb.bin", 0x000000, 0x100000, CRC(0a525641) SHA1(6449aea7af69e6b91c91b654c9ea952a72f0dd7c) )
ROM_END

ROM_START( g_persb1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_persb1.bin", 0x000000, 0x040000, CRC(425e6a87) SHA1(224516e54a4bac00089e61c8e0a4794eac92d8df) )
ROM_END

ROM_START( g_persb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_persb2.bin", 0x000000, 0x100000, CRC(505314b6) SHA1(addd33ce2f9c022433be1c3ea803371e7f6b694b) )
ROM_END
ROM_START( g_perse )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_perse.bin", 0x000000, 0x100000, CRC(61de6fe0) SHA1(6e645b791e6e2b84a206dca6cf47e8f955e60a72) )
ROM_END
ROM_START( g_puggsb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_puggsb.bin", 0x000000, 0x100000, CRC(37fa4877) SHA1(d1b4e6528e90319d5bab47de98dd691070998df4) )
ROM_END
ROM_START( g_punise )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_punise.bin", 0x000000, 0x200000, CRC(729edd17) SHA1(d11ec72726498e4608eefc1ef2b8a3cadb025394) )
ROM_END
ROM_START( g_rrexe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rrexe.bin", 0x000000, 0x100000, CRC(d02d3282) SHA1(c8dc73b7cabf43813f9cf09370a4437af4f1573f) )
ROM_END
ROM_START( g_rbi4b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rbi4b.bin", 0x000000, 0x100000, CRC(f7420278) SHA1(d999e11813dca4fe0ac05f371ff7900806238a69) )
ROM_END
ROM_START( g_renstb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_renstb.bin", 0x000000, 0x100000, CRC(fcb86336) SHA1(85e2e41d31bde9acc98070014640c00ba8dba6f8) )
ROM_END
ROM_START( g_robtb1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robtb1.bin", 0x000000, 0x200000, CRC(ecebff29) SHA1(4d35cdf21f50c1cf73bacc9b8747a9eac1141e46) )
ROM_END
ROM_START( g_robtb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robtb2.bin", 0x000000, 0x200000, CRC(2487049f) SHA1(a1625783cf1b42d808ad4f56dde5ce1b1884218a) )
ROM_END
ROM_START( g_robte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robte.bin", 0x000000, 0x200000, CRC(85a93f8d) SHA1(21348f50ccb0031108ce3fb999026688e9bef4ed) )
ROM_END

ROM_START( g_robwrb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robwrb.bin", 0x000000, 0x100000, CRC(c67ddb14) SHA1(8cecca591f781a638b45f99f69bf2f18b7abb289) )
ROM_END
ROM_START( g_snsme )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_snsme.bin", 0x000000, 0x400000, CRC(08fa5a3f) SHA1(f67f324165abdf148f80aabb319375dc3a504e17) )
ROM_END
ROM_START( g_scrabb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_scrabb.bin", 0x000000, 0x100000, CRC(360b2610) SHA1(b2a06070eda30eb2ef745dcb2676dc25d39a07ed) )
ROM_END
ROM_START( g_seaqe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_seaqe.bin", 0x000000, 0x200000, CRC(0bb511bd) SHA1(e0324a23bc26b40f7e564ae864afb8ede1776f6e) )
ROM_END
ROM_START( g_sensib )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sensib.bin", 0x000000, 0x080000, CRC(ef52664d) SHA1(c12a644cf81886050a4ae108b17b3c742055f5c3) )
ROM_END
ROM_START( g_shan2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_shan2b.bin", 0x000000, 0x100000, CRC(5e33867b) SHA1(bd136fd6485b653456a8bf27837ce20139b28dcc) )
ROM_END
ROM_START( g_sfrcbt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sfrcbt.bin", 0x000000, 0x180000, CRC(ce67143a) SHA1(f026c9243431c9c2d4b0660e340158816a22b869) )
ROM_END
ROM_START( g_skrewe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_skrewe.bin", 0x000000, 0x200000, CRC(5f872737) SHA1(9752fbd8508492dae252ae749393281ed9527de0) )
ROM_END

ROM_START( g_soleib )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soleib.bin", 0x000000, 0x100000, CRC(43797455) SHA1(963f13f74d648c573ced103b2803d15d0a2742e8) )
ROM_END

ROM_START( g_soleis )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soleis.bin", 0x000000, 0x200000, CRC(9ed4c323) SHA1(95ae529cecb7dbca281df25c3c4e7cb8f48d936c) )
ROM_END
ROM_START( g_soncrk )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_soncrk.bin", 0x000000, 0x100000, CRC(7fada88d) SHA1(05f460a6ebbc1f86eb40b5f762083a59fb29f3e2) )
ROM_END
ROM_START( g_sonsb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sonsb.bin", 0x000000, 0x140000, CRC(b1524979) SHA1(b426457e5b440ed33ee1756bc6dad2bdcd0c0d9f) )
ROM_END
ROM_START( g_sork00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sork00.bin", 0x000000, 0x100000, CRC(cbe6c1ea) SHA1(57322c1714fd4e42e1a10d56bfd795bcbc3380d7) )
ROM_END
ROM_START( g_s_asb1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s_asb1.bin", 0x000000, 0x200000, CRC(b88a710d) SHA1(0bb3131108ed57c0cd12b646eb2b6dd73eb679f9) )
ROM_END
ROM_START( g_s_asb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_s_asb2.bin", 0x000000, 0x200000, CRC(b88a710d) SHA1(0bb3131108ed57c0cd12b646eb2b6dd73eb679f9) )
ROM_END
ROM_START( g_stds9e )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_stds9e.bin", 0x000000, 0x100000, CRC(d4b122f9) SHA1(efc96336ccd83c31ab48ab48fe1e262c3ebcf0be) )
ROM_END
ROM_START( g_sgatb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sgatb.bin", 0x000000, 0x200000, CRC(8dc8ab23) SHA1(95a0dab802a313281d74fe66b21c7877078c57e9) )
ROM_END
ROM_START( g_sempb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sempb.bin", 0x000000, 0x100000,  CRC(e5517b77) SHA1(00109e6de5b823525b3ba519222fecc4e229f6d1) )
ROM_END
ROM_START( g_sthorb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sthorb.bin", 0x000000, 0x1b0000, CRC(bfc11649) SHA1(ac1952f2f7cd4109561376f3097c4daec2ae64ae) )
ROM_END


ROM_START( g_sthorf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sthorf.bin", 0x000000, 0x300000, CRC(b97cca1c) SHA1(677e1fbf2f6f90dd5a016f3bd5f305547249205a) )
ROM_END
ROM_START( g_sthorg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sthorg.bin", 0x000000, 0x300000,  CRC(fa20d011) SHA1(a82ffb7c4bf4b0f89f42a9cdc6600bc5bac1c854) )
ROM_END
ROM_START( g_sthork )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sthork.bin", 0x000000, 0x300000, CRC(ee1603c5) SHA1(e0a43fb3d6da940b1fda449753bffae637a802cd) )
ROM_END
ROM_START( g_sthors )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sthors.bin", 0x000000, 0x300000, CRC(4631f941) SHA1(0fcc02355176e1c96043f4d827a3ff88d2d272df) )
ROM_END
ROM_START( g_sf2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sf2b.bin", 0x000000, 0x200000, CRC(a85491ae) SHA1(23e1e1b587a7d2d1a82599d82d01c9931ca7b4cf) )
ROM_END
ROM_START( g_strikb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_strikb.bin", 0x000000, 0x200000, CRC(c10b270e) SHA1(128395e635e948005e89c7f4a6cd5b209be1ffbc) )
ROM_END
ROM_START( g_subtb1 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_subtb1.bin", 0x000000, 0x140000, CRC(9c13d25c) SHA1(3766bfaf355cc239187255a142daddcd42d1fc58) )
ROM_END
ROM_START( g_subtb2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_subtb2.bin", 0x000000, 0x200000, CRC(3a1022d1) SHA1(57279fa2bc9baf07d701be44e51e42d7f1e0e2a2) )
ROM_END
ROM_START( g_sleage )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sleage.bin", 0x000000, 0x080000, CRC(55baec6e) SHA1(fefc7abc2f9fbcc7992b1420b62eb3eb3d5ad1bb) )
ROM_END
ROM_START( g_sshi2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sshi2b.bin", 0x000000, 0x100000, CRC(c47e8aea) SHA1(e1eca5faec785f4e1093b088421a818e2fab29c1) )
ROM_END

ROM_START( g_skida )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_skida.bin", 0x000000, 0x200000, CRC(a61a0f0c) SHA1(1ec5fb1e7c8adfaf147271bd40454826fffed13e) )
ROM_END
ROM_START( g_supmnb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_supmnb.bin", 0x000000, 0x100000, CRC(5cd0e1d4) SHA1(9d94f0e364b5170ac1f1f2098091c3fc89d07389) )
ROM_END
ROM_START( g_sylvb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sylvb.bin", 0x000000, 0x200000, CRC(9d9c786b) SHA1(d1ca731869b2ffd452bd4f2b40bb0afdc997936d) )
ROM_END
ROM_START( g_t2arcb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_t2arcb.bin", 0x000000, 0x100000, CRC(94255703) SHA1(87660389d70d73e5f0f68d672a0843712f7c4c85) )
ROM_END
ROM_START( g_tmhte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tmhte.bin", 0x000000, 0x100000, CRC(966d5286) SHA1(ed6c32cae0813cbcf590fad715fa045fbeab6d78) )
ROM_END
ROM_START( g_tmhtte )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tmhtte.bin", 0x000000, 0x200000, CRC(3cd2b7e6) SHA1(6a6c4ae9d944ad1d459d46ae40d3af09e60b5d7d) )
ROM_END
ROM_START( g_ttadae )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_ttadae.bin", 0x000000, 0x100000, CRC(1227b2b2) SHA1(2672018d9e005a9a3b5006fa8f61e08f2d1909aa) )
ROM_END
ROM_START( g_tpgola )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tpgola.bin", 0x000000, 0x400000, CRC(a89638a0) SHA1(cb7f4b9b89fbf6162d7d4182229c8ac473f91cf4) )
ROM_END
ROM_START( g_twisf )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_twisf.bin", 0x000000, 0x080000, CRC(6dd47554) SHA1(a6eac3e1b9df9d138fceeccbe34113015d3a676f) )
ROM_END

ROM_START( g_crudee )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_crudee.bin", 0x000000, 0x100000, CRC(b6d90a10) SHA1(babf6ec6dac62e7563c4fe9cb278179dc4343ea4) )
ROM_END

ROM_START( g_vectb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_vectb.bin", 0x000000, 0x200000, CRC(2084d3da) SHA1(e6c0854ff0f5a0b53760677743e9b901b3e5a4b7) )
ROM_END
ROM_START( g_viewpb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_viewpb.bin", 0x000000, 0x180000, CRC(f2e69ce7) SHA1(7c7a7812e8d1ab438907233ad03ec0b763a9b556) )
ROM_END

ROM_START( g_virrea )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_virrea.bin", 0x000000, 0x200000, CRC(5a943df9) SHA1(2c08ea556c79d48e88ff5202944c161ae1b41c63) )
ROM_END
ROM_START( g_wacrac )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wacrac.bin", 0x000000, 0x200000, CRC(1b173f09) SHA1(5f310699e017e0becb4f41e7f00cb86606dbc0ed) )
ROM_END
ROM_START( g_warlob )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_warlob.bin", 0x000000, 0x200000, CRC(c9b6edb3) SHA1(870ce90ae5a58838c1e0531abdc0442389189234) )
ROM_END
ROM_START( g_watrb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_watrb.bin", 0x000000, 0x200000, CRC(51c80498) SHA1(35e4186654a677a43d16861f9832199c5cb5e0ef) )
ROM_END
ROM_START( g_wwcse )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wwcse.bin", 0x000000, 0x100000, CRC(eef372e8) SHA1(97c1f36e375ac33fbc5724bbb4bb296d974b114f) )
ROM_END

ROM_START( g_waghe )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_waghe.bin", 0x000000, 0x080000, CRC(c0dce0e5) SHA1(97e35e8fbe1546b9cf075ab75c8f790edcd9db93) )
ROM_END
ROM_START( g_wchalb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wchalb.bin", 0x000000, 0x100000,  CRC(60d2a8c4) SHA1(61748b459900f26832da72d6b4c51f886994d7cf) )
ROM_END
ROM_START( g_wcs02 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wcs02.bin", 0x000000, 0x040000, CRC(bf84ede6) SHA1(ef8c106acad9c3b4a5db2cd0d311762491d28392) )
ROM_END
ROM_START( g_wcs2b )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wcs2b.bin", 0x000000, 0x100000, CRC(c1e21c1a) SHA1(d3f4f2f5e165738bde6c6011c3d68322c27d97ed) )
ROM_END
ROM_START( g_willb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_willb.bin", 0x000000, 0x100000,  CRC(577f680f) SHA1(c3d721dc3d6660156f28728d729a2c0d4bb23fdf) )
ROM_END
ROM_START( g_wwarb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wwarb.bin", 0x000000, 0x080000,  CRC(1cdee87b) SHA1(65c4815c6271bb7d526d84dac1bf177741e35364) )
ROM_END
ROM_START( g_yindyb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yindyb.bin", 0x000000, 0x180000,  CRC(ad6c2050) SHA1(63f5ac7dddb166fba76d74c58daf320568d6b016) )
ROM_END
ROM_START( g_yindcb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_yindcb.bin", 0x000000, 0x080000,  CRC(44f6be35) SHA1(1b1a2dc24e4decebc6f1717d387d560a1ae9332c) )
ROM_END
ROM_START( g_zany01 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zany01.bin", 0x000000, 0x080000,  CRC(74ed7607) SHA1(f1c3b211c91edfed9c96f422cd10633afa43fdf0) )
ROM_END
ROM_START( g_zwingj )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zwingj.bin", 0x000000, 0x100000,  CRC(7e203d2b) SHA1(fad499e21ac55a3b8513110dc1f6e3f6cdeca8dd) )
ROM_END

ROM_START( g_zombhb )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_zombhb.bin", 0x000000, 0x0fa0ef,  CRC(7bea6194) SHA1(56f1a0b616e178315bdb9f2e43ddc942dcf3780e) ) // strange size ?!
ROM_END


ROM_START( g_16ton )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_16ton.bin", 0x000000, 0x040000,  CRC(537f04b6) SHA1(fd978f7f643311de21de17ed7ed1e7c737b518ee) )
ROM_END
ROM_START( g_hymar )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hymar.bin", 0x000000, 0x040000,  CRC(83bb2799) SHA1(b2c5c78084b988de1873c89eedfa984124404c0c) )
ROM_END
ROM_START( g_labyd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_labyd.bin", 0x000000, 0x040000,  CRC(a6d7e02d) SHA1(513732ae1c5a40276959967bbf4775fce1c83a7e) )
ROM_END
ROM_START( g_padfi )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_padfi.bin", 0x000000, 0x040000,  CRC(3d8147e6) SHA1(a768f64f394d4185f99da300517521091b6f0de5) )
ROM_END
ROM_START( g_p2anne )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_p2anne.bin", 0x000000, 0x040000,   CRC(fafa5b6f) SHA1(cf93d2d5eafc72b9de7aaa6d35b81a813862d9b7) )
ROM_END
ROM_START( g_p2huey )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_p2huey.bin", 0x000000, 0x040000,  CRC(1a076f83) SHA1(abc5a7ac58aefc055ed8ca5fd0f7c802ab3e1ad5) )
ROM_END
ROM_START( g_p2kind )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_p2kind.bin", 0x000000, 0x040000,  CRC(c334f308) SHA1(f2bd41741add13e8551250cf6afeb746e8cea316) )
ROM_END
ROM_START( g_p2shil )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_p2shil.bin", 0x000000, 0x040000,  CRC(1f83beb2) SHA1(114f37a287b51664e52ba483f07911a660f9e7df) )
ROM_END
ROM_START( g_putter )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_putter.bin", 0x000000, 0x040000,  CRC(20f168a6) SHA1(345af8995cf9d88f7c4d8cfd81c8ccd45e6478fc) )
ROM_END
ROM_START( g_pymag )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pymag.bin", 0x000000, 0x040000,  CRC(306d839e) SHA1(e32382b419d63186dc4f5521045cae2145cb7975) )
ROM_END

ROM_START( g_pymag2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pymag2.bin", 0x000000, 0x040000,  CRC(c9ddac72) SHA1(b74feb0b7f5f6eaac72f0895b51a8f8421dcb768) )
ROM_END
ROM_START( g_pymag3 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pymag3.bin", 0x000000, 0x040000,  CRC(8329820a) SHA1(c386a1fc6cf5d0d6f0d2eba0b2ddda649143da67) )
ROM_END
ROM_START( g_pymags )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_pymags.bin", 0x000000, 0x040000,  CRC(153a3afa) SHA1(010a336bedc1022846f3498693cd6b5218a05b4c) )
ROM_END
ROM_START( g_serase )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_serase.bin", 0x000000, 0x040000,  CRC(62d8a0e7) SHA1(be70d4246be49c0301a1402bd93f28c58b558a8d) )
ROM_END



ROM_START( g_hercu ) // clone of g_dahna
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_hercu.bin", 0x000000, 0x100000, CRC(ff75d9d0) SHA1(6d89f589aeefaceda85a6f592d5d5b9062472b7a) )
ROM_END

ROM_START( g_f2000g ) // clone of g_fi97
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_f2000g.bin", 0x000000, 0x200000, CRC(647df504) SHA1(ed29312dbd9574514c06e5701e24c6474ed84898) )
ROM_END
ROM_START( g_sho3ja )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sho3ja.bin", 0x000000, 0x200000,  CRC(ed68373a) SHA1(a42fc3b2a4f4c2db2f598244c2b137862b8e79ad) )
ROM_END



ROM_START( radicav1 )
	ROM_REGION( 0xc00000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "radicav1.bin", 0x000000, 0x400000,  CRC(3b4c8438) SHA1(5ed9c053f9ebc8d4bf571d57e562cf347585d158) )
	ROM_RELOAD( 0x400000, 0x400000 ) // for bankswitch
	ROM_RELOAD( 0x800000, 0x400000 ) // allow bank to wrap
ROM_END

ROM_START( radicasf )
	ROM_REGION( 0xc00000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "radicasf.bin", 0x000000, 0x400000,  CRC(868afb44) SHA1(f4339e36272c18b1d49aa4095127ed18e0961df6) )
	ROM_RELOAD( 0x400000, 0x400000 ) // for bankswitch
	ROM_RELOAD( 0x800000, 0x400000 ) // allow bank to wrap
ROM_END

ROM_START( g_ssf2 )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Case, leave this area empty and let the INIT / banking take care of it */
	ROM_LOAD( "g_ssf2.bin", 0x400000, 0x500000,  CRC(165defbf) SHA1(9ce6e69db9d28386f7542dacd3e3ead28eacf2a4) )
ROM_END

ROM_START( g_ssf2e )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Case, leave this area empty and let the INIT / banking take care of it */
	ROM_LOAD( "g_ssf2e.bin", 0x400000, 0x500000, CRC(682c192f) SHA1(56768a0524fa13fcd76474c8bb89b995bb471847) )
ROM_END
ROM_START( g_ssf2j )
	ROM_REGION( 0x1400000, "main", 0 ) /* 68000 Code */
	/* Special Case, leave this area empty and let the INIT / banking take care of it */
	ROM_LOAD( "g_ssf2j.bin", 0x400000, 0x500000, CRC(d8eeb2bd) SHA1(24c8634f59a481118f8350125fa6e00d33e04c95) )
ROM_END



ROM_START( g_indy )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_indy.bin", 0x000000, 0x200000,  CRC(9a01974e) SHA1(d59bb77b83cf912e3cb8a7598cabcf87a273e429) )
ROM_END

ROM_START( g_princ2 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_princ2.bin", 0x000000, 0x200000,  CRC(3ab44d46) SHA1(ba63dc1e521bee68b4121b626061ebb203ac63c6) )
ROM_END


ROM_START( g_sanret )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_sanret.bin", 0x000000, 0x100000, CRC(3b5cc398) SHA1(7aaa0cc1dafaa14e6d62d6c1dbd462e69617bf7e) ) // clone?
ROM_END


ROM_START( g_megamd )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_megamd.bin", 0x000000, 0x040000,  CRC(76df2ae2) SHA1(dc5697f6baeeafe04250b39324464bb6dcdcac7f) )
ROM_END
ROM_START( g_aworg )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_aworg.bin", 0x000000, 0x040000,  CRC(069c27c1) SHA1(8316433112fb4b80bbbaf905f93bbfa95f5cb8b2) )
ROM_END
ROM_START( g_teddy )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_teddy.bin", 0x000000, 0x040000,  CRC(733d2eb3) SHA1(dc858342be31ab9491acfaebf1524ece2c6ef9a3) )
ROM_END
ROM_START( g_robobt )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_robobt.bin", 0x000000, 0x040000,  CRC(fdf23eff) SHA1(1e5cbfa3dfbab399a84f5f249c7f5dec9ecd0026) )
ROM_END
ROM_START( g_medalc )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_medalc.bin", 0x000000, 0x040000,  CRC(3ef4135d) SHA1(809bbb203dd2f2542e3b9987380e040631b788cc) )
ROM_END
ROM_START( g_riddle )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_riddle.bin", 0x000000, 0x040000,  CRC(fae3d720) SHA1(662ffc946978030125dc1274aeec5196bc9a721b) )
ROM_END
ROM_START( g_kisssh )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_kisssh.bin", 0x000000, 0x040000,  CRC(e487088c) SHA1(bc47dceea512c0195e51172ab3a2ff5cad03c9bd) )
ROM_END
ROM_START( g_rist00 )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_rist00.bin", 0x000000, 0x200000,  CRC(9700139b) SHA1(cf0215feddd38f19cd2d27bfa96dd4d742ba8bf7) )
ROM_END


ROM_START( g_svolx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_svolx.bin", 0x000000, 0x040000,  CRC(e2ee8ad2) SHA1(461260a04beeee86d7480bb0b3eccace064c6b87) )
ROM_END
ROM_START( g_tcupx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_tcupx.bin", 0x000000, 0x080000, CRC(88fdd060) SHA1(67e9261097e1ed9c8d7d11fd45a7c7e04ac1145f) )
ROM_END
ROM_START( g_wclbx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_wclbx.bin", 0x000000, 0x080000,   CRC(daca01c3) SHA1(c307a731763c7f858ef27058b4f46017868749d6) )
ROM_END
ROM_START( g_blc96x )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_blc96x.bin", 0x000000, 0x100000,  CRC(fe52f7e1) SHA1(3e1ef39e9008a4a55fd57b25948668c8c52ba9e3) )
ROM_END
ROM_START( g_astpgx )
	ROM_REGION( 0x400000, "main", 0 ) /* 68000 Code */
	ROM_LOAD( "g_astpgx.bin", 0x000000, 0x200000,  CRC(45c8b5b7) SHA1(0905252a58dc5ce0aaa06f1129516fefbda65ddb) )
ROM_END





/*********************************************************
  Generic Backup RAM handling for some games
*********************************************************/

WRITE16_HANDLER( megadriv_backupram_w )
{
	/* Write the backup RAM to the CPU1 region so that the DMA / Z80 etc. can see it
	   there is no ROM mapped here anyway */
	megadriv_backupram[offset]=data;
}

void init_backupram(int backupram_base, int length)
{
	UINT16 *ROM = (UINT16*)memory_region(Machine, "main");
	megadriv_backupram = ROM+(backupram_base/2);
	megadriv_backupram_length = length;
	//memory_install_read_handle(machine,(0, ADDRESS_SPACE_PROGRAM, backupram_base, backupram_base+megadriv_backupram_length, 0, 0, megadriv_backupram_r);
	memory_install_write16_handler(Machine, 0, ADDRESS_SPACE_PROGRAM, backupram_base, backupram_base+megadriv_backupram_length, 0, 0, megadriv_backupram_w);

}



DRIVER_INIT( g_hb95 )
{
	DRIVER_INIT_CALL(megadriv);
	init_backupram(0x300000,0x10000);
}

DRIVER_INIT( g_dino )
{
	DRIVER_INIT_CALL(megadrie);
	init_backupram(0x200000,0x2000);
}

DRIVER_INIT( g_buck )
{
	DRIVER_INIT_CALL(megadriv);
	init_backupram(0x200000,0x10000);
}


DRIVER_INIT( g_nh98 )
{
	DRIVER_INIT_CALL(megadriv);
	init_backupram(0x200000,0x10000);
}



DRIVER_INIT( g_wrom2 )
{
	DRIVER_INIT_CALL(megadriv);
	init_backupram(0x200000,0x2000);
}

DRIVER_INIT( g_caeno2 )
{
	DRIVER_INIT_CALL(megadrij);
	init_backupram(0x200000,0x2000);
}

DRIVER_INIT( megadriv_backup_0x200000_0x800 )
{
	DRIVER_INIT_CALL(megadriv);
	init_backupram(0x200000,0x800);
}

/* Standard Backup RAM (0x4000 8-bits, at 0x200000, no enable / disable) */

DRIVER_INIT( megadrie_backup_0x200000_0x4000 )
{
	DRIVER_INIT_CALL(megadrie);
	init_backupram(0x200000,0x4000);
}



DRIVER_INIT( megadriv_backup_0x200000_0x4000 )
{
	DRIVER_INIT_CALL(megadriv);
	init_backupram(0x200000,0x4000);
}

DRIVER_INIT( megadrij_backup_0x200000_0x4000 )
{
	DRIVER_INIT_CALL(megadrij);
	init_backupram(0x200000,0x4000);
}


DRIVER_INIT( megadriv_backup_0x200000_0x10000 )
{
	DRIVER_INIT_CALL(megadriv);
	init_backupram(0x200000,0x10000);
}

DRIVER_INIT( megadrie_backup_0x200000_0x10000 )
{
	DRIVER_INIT_CALL(megadrie);
	init_backupram(0x200000,0x10000);
}

DRIVER_INIT( megadrij_backup_0x200000_0x10000 )
{
	DRIVER_INIT_CALL(megadrij);
	init_backupram(0x200000,0x10000);
}


/***********************************************************
 Other Backup RAM cases
***********************************************************/

UINT16* sks3_rom_backup;
UINT16* sks3_ram_backup;
int sks3_backupram_enable;
UINT16 *sks3_backupram_location;


WRITE16_HANDLER( sks3_backupram_w )
{
	/* Write the backup RAM to the CPU1 region so that the DMA / Z80 etc. can see it
	   there is no ROM mapped here anyway */
	if (sks3_backupram_enable)
		megadriv_backupram[offset]=data;
	else
		printf("attempt to write backup ram when rom is banked\n");
}

WRITE16_HANDLER( sks3_backupram_enable_w )
{
	sks3_backupram_enable = data & 1;

	if (sks3_backupram_enable)
	{
		memcpy(sks3_backupram_location, megadriv_backupram, megadriv_backupram_length);
	}
	else
	{
		memcpy(sks3_rom_backup, megadriv_backupram, megadriv_backupram_length);

	}
}

void init_sks3_backupram(int backupram_base, int length)
{
	UINT16 *ROM = (UINT16*)memory_region(Machine, "main");
	megadriv_backupram_length = length;

	/* Create a copy of the ROM which maps at the same address as the backup RAM so
	   we can easily swap it back in */
	sks3_rom_backup = auto_malloc(megadriv_backupram_length);
	sks3_backupram_location = ROM+(backupram_base/2);
	memcpy(sks3_rom_backup, sks3_backupram_location, megadriv_backupram_length);

	/* Allocate the backup RAM */
	megadriv_backupram = auto_malloc(megadriv_backupram_length);
	memory_install_write16_handler(Machine, 0, ADDRESS_SPACE_PROGRAM, 0xA130f0, 0xa130f1, 0, 0, sks3_backupram_enable_w);
	memory_install_write16_handler(Machine, 0, ADDRESS_SPACE_PROGRAM, backupram_base, backupram_base+megadriv_backupram_length, 0, 0, sks3_backupram_w);

}



DRIVER_INIT( g_sks3 )
{
	DRIVER_INIT_CALL(megadriv);
	init_sks3_backupram(0x200000,0x400);
}

DRIVER_INIT( g_sks3e)
{
	DRIVER_INIT_CALL(megadrie);
	init_sks3_backupram(0x200000,0x400);
}

DRIVER_INIT( g_sks3j )
{
	DRIVER_INIT_CALL(megadrij);
	init_sks3_backupram(0x200000,0x400);
}

DRIVER_INIT( g_pst4 )
{
	DRIVER_INIT_CALL(megadriv);
	init_sks3_backupram(0x200000,0x4000);
}

DRIVER_INIT( g_pst4j )
{
	DRIVER_INIT_CALL(megadrij);
	init_sks3_backupram(0x200000,0x4000);
}

DRIVER_INIT( g_gameno )
{
	DRIVER_INIT_CALL(megadrij);
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x000000, 0x3fffff, 0, 0, SMH_BANK5);
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x000000, 0x3fffff, 0, 0, SMH_BANK5);

	memory_set_bankptr( 5, memory_region(Machine, "main" ) );
}




DRIVER_INIT( _32x )
{
	memory_set_bankptr( 2, memory_region( Machine, "user1" ) );
	memory_set_bankptr( 4, memory_region( Machine, "user1"  ) );
	DRIVER_INIT_CALL(megadriv);
}


WRITE16_HANDLER( megadriv_ssf2_bank_w )
{
	UINT8 *ROM = memory_region(Machine,"main");


	switch (offset<<1)
	{
		case 0x00: // write protect register
			break;
		case 0x02: /* 0x080000 - 0x0FFFFF */
			memcpy(ROM + 0x080000, ROM + 0x400000+((data&0xf)*0x080000), 0x080000);
			break;
		case 0x04: /* 0x100000 - 0x17FFFF */
			memcpy(ROM + 0x100000, ROM + 0x400000+((data&0xf)*0x080000), 0x080000);
			break;
		case 0x06: /* 0x180000 - 0x1FFFFF */
			memcpy(ROM + 0x180000, ROM + 0x400000+((data&0xf)*0x080000), 0x080000);
			break;
		case 0x08: /* 0x200000 - 0x27FFFF */
			memcpy(ROM + 0x200000, ROM + 0x400000+((data&0xf)*0x080000), 0x080000);
			break;
		case 0x0a: /* 0x280000 - 0x2FFFFF */
			memcpy(ROM + 0x280000, ROM + 0x400000+((data&0xf)*0x080000), 0x080000);
			break;
		case 0x0c: /* 0x300000 - 0x37FFFF */
			memcpy(ROM + 0x300000, ROM + 0x400000+((data&0xf)*0x080000), 0x080000);
			break;
		case 0x0e: /* 0x380000 - 0x3FFFFF */
			memcpy(ROM + 0x380000, ROM + 0x400000+((data&0xf)*0x080000), 0x080000);
			break;

	}

}

DRIVER_INIT( g_ssf2 )
{
	UINT8 *ROM = memory_region(Machine,"main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* copy 1st 4meg (it checksums this before writing bank regs)*/
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xA130F0, 0xA130FF, 0, 0, megadriv_ssf2_bank_w);
	DRIVER_INIT_CALL(megadriv);
}

DRIVER_INIT( g_ssf2e )
{
	UINT8 *ROM = memory_region(Machine,"main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* copy 1st 4meg (it checksums this before writing bank regs)*/
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xA130F0, 0xA130FF, 0, 0, megadriv_ssf2_bank_w);
	DRIVER_INIT_CALL(megadrie);
}

DRIVER_INIT( g_ssf2j )
{
	UINT8 *ROM = memory_region(Machine,"main");
	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* copy 1st 4meg (it checksums this before writing bank regs)*/
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xA130F0, 0xA130FF, 0, 0, megadriv_ssf2_bank_w);
	DRIVER_INIT_CALL(megadrij);
}


READ16_HANDLER( radicav1_bank_select )
{
	int bank = offset&0x3f;
	UINT8 *ROM = memory_region(Machine,"main");
	memcpy(ROM, ROM +  (bank*0x10000)+0x400000, 0x400000);

//	printf("bank %02x\n",offset);
	return 0;
}

DRIVER_INIT( radicav1 )
{
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xa13000, 0xa1307f, 0, 0, radicav1_bank_select );
	DRIVER_INIT_CALL(megadriv);
}

DRIVER_INIT( radicasf )
{
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0xa13000, 0xa1307f, 0, 0, radicav1_bank_select );
	DRIVER_INIT_CALL(megadrie);
}


UINT16 virrac_unk[0x010000];

READ16_HANDLER( virrac_status_r )
{
	return 1;
}

READ16_HANDLER( virrac_unk_r )
{
	return virrac_unk[offset];
}

WRITE16_HANDLER( virrac_unk_w )
{
	virrac_unk[offset] = data;
}

DRIVER_INIT( g_virrac )
{
	/* RAM shared with 3D DSP */
	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x300000, 0x31ffff, 0, 0, virrac_unk_r );
	memory_install_write16_handler(machine, 0,ADDRESS_SPACE_PROGRAM, 0x300000, 0x31ffff, 0, 0, virrac_unk_w );

	memory_install_read16_handler(machine,0, ADDRESS_SPACE_PROGRAM, 0x30fe02, 0x30fe03, 0, 0, virrac_status_r );

	DRIVER_INIT_CALL(megadriv);


}

/********************************* Sorted ********************************************/

// some serious raster abuse from the end of level 2 onwards
GAME( 1995, g_abat,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Adventures of Batman and Robin, The (U) [!]", 0 )
GAME( 1995, g_abate,  g_abat,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Adventures of Batman and Robin, The (E) [!]", 0 )

GAME( 1993, g_acro,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sunsoft / Iguana", "Aero the Acro-Bat (U) [c][!]", 0 )

GAME( 1994, g_acr2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sunsoft / Iguana", "Aero the Acro-Bat 2 (U) [!]", 0 )
GAME( 1994, g_acro2e, g_acr2,   megadpal,    megadriv,    megadrie, ROT0,   "Sunsoft / Iguana", "Aero the Acro-Bat 2 (E) [!]", 0 )

GAME( 199?, g_aero,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,   ROT0,   "Koei", "Aerobiz (U) [!]", 0 )
GAME( 199?, g_airm,   g_aero,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,   ROT0,   "Koei", "Air Management - Oozora ni Kakeru (J) [c][!]", 0 )

GAME( 1994, g_abz2,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,   ROT0,   "Koei", "Aerobiz Supersonic (U) [!]", 0 )
GAME( 1994, g_airm2,  g_abz2,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,   ROT0,   "Koei", "Air Management II - Kouku Ou wo Mezase (J) [!]", 0 )


GAME( 1993, g_alad,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Disney / Virgin", "Disney's Aladdin (U) [!]", 0 )
GAME( 1993, g_alade,  g_alad,   megadpal,    megadriv,    megadrie, ROT0,   "Disney / Virgin", "Disney's Aladdin (E) [!]", 0 )
GAME( 1993, g_aladj,  g_alad,   megadriv,    megadriv,    megadrij, ROT0,   "Disney / Virgin", "Disney's Aladdin (J) [!]", 0 )
GAME( 1993, g_aladb,  g_alad,   megadriv,    megadriv,    megadriv, ROT0,   "Disney / Virgin", "Disney's Aladdin (Beta) [!]", 0 )

GAME( 1989, g_alex,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Alex Kidd in the Enchanted Castle (U) [!]", 0 )
GAME( 1989, g_alxkeb, g_alex,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Alex Kidd in the Enchanted Castle (E) (MD Bundle) [!]", 0 )
GAME( 1989, g_alexke, g_alex,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Alex Kidd in the Enchanted Castle (E) [!]", 0 )
GAME( 1989, g_alexkj, g_alex,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Alex Kidd in the Enchanted Castle (J) [!]", 0 )
GAME( 1989, g_alexkk, g_alex,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Alex Kidd - Cheongongmaseong (K) [!]", 0 )


GAME( 1988, g_abea,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Altered Beast (UE) (REV02) [!]", 0 )
GAME( 1988, g_juuo00, g_abea,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Juu-Ou-Ki (J) (REV00) [c][!]", 0 )
GAME( 1988, g_juuo01, g_abea,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Juu-Ou-Ki (J) (REV01) [!]", 0 )


GAME( 1992, g_arun,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Data East", "Atomic Runner (U) [!]", 0 )
GAME( 1992, g_arunre, g_arun,   megadpal,    megadriv,    megadrie, ROT0,   "Data East", "Atomic Runner (E) [!]", 0 )
GAME( 1992, g_chelno, g_arun,   megadriv,    megadriv,    megadrij, ROT0,   "Data East", "Chelnov (J) [!]", 0 )

GAME( 1994, g_aof,    0,        megadriv,    megadri6,    megadriv, ROT0,   "SNK / Sega", "Art of Fighting (U) [!]", 0 )
GAME( 1994, g_aofe,   g_aof,    megadpal,    megadri6,    megadrie, ROT0,   "SNK / Sega", "Art of Fighting (E) [!]", 0 )
GAME( 1994, g_ryuuko, g_aof,    megadriv,    megadri6,    megadrij, ROT0,   "SNK / Sega", "Ryuuko no Ken (J) [!]", 0 )

GAME( 1995, g_boas,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Beyond Oasis (U) [!]", 0 )
GAME( 1995, g_sthorj, g_boas,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Story of Thor, The - A Successor of The Light (J) [!]",0 )
GAME( 1995, g_sthor,  g_boas,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Story of Thor, The - A Successor of The Light (E) [!]", 0 )
GAME( 1995, g_sthorf, g_boas,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Story of Thor, The - A Successor of The Light (F)", 0 )
GAME( 1995, g_sthorg, g_boas,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Story of Thor, The - A Successor of The Light (G)", 0 )
GAME( 1995, g_sthork, g_boas,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Story of Thor, The - A Successor of The Light (K)", 0 )
GAME( 1995, g_sthors, g_boas,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Story of Thor, The - A Successor of The Light (S)", 0 )
GAME( 1995, g_sthorb, g_boas,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Story of Thor, The - A Successor of The Light (Beta)", 0 )

GAME( 1994, g_bloc,    0,       megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Block Out (W) (REV01) [!]", 0 )

// corrupt texture is correct
GAME( 1994, g_bsht,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Domark / Acclaim", "Blood Shot (E) (M4) [!]", 0 )

GAME( 1994, g_bob,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "B.O.B (UE) (REV01) [!]", 0 )
GAME( 1994, g_sfbob,  g_bob,    megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts", "Space Funky B.O.B. (J) [!]", 0 )
GAME( 1994, g_bobb,   g_bob,    megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "B.O.B (Beta)", 0 )

// broken, doesn't like current HV implementation
GAME( 1994, g_drac,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Psygnosis / Sony", "Bram Stoker's Dracula (U) [!]", GAME_NOT_WORKING )
GAME( 1994, g_drace,  g_drac,   megadpal,    megadriv,    megadrie, ROT0,   "Psygnosis / Sony", "Bram Stoker's Dracula (E) [!]", GAME_NOT_WORKING )

GAME( 1900, g_capp,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Captain Planet and the Planeteers (E) [!]", 0 )
GAME( 1900, g_cappb,  g_capp,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Captain Planet and the Planeteers (Beta)", 0 )

GAME( 1990, g_cill,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sega", "Castle of Illusion Starring Mickey Mouse (UE) [!]", 0 )
GAME( 1990, g_mickey, g_cill,   megadriv,    megadriv,    megadrij, ROT0,   "Disney / Sega", "I Love Mickey Mouse - Fushigi no Oshiro Dai Bouken (J) [!]", 0 )

GAME( 1994, g_casv,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Konami", "Castlevania - Bloodlines (U) [!]", 0 )
GAME( 1994, g_cvane,  g_casv,   megadpal,    megadriv,    megadrie, ROT0,   "Konami", "Castlevania - The New Generation (E) [!]", 0 )
GAME( 1994, g_cvtngb, g_casv,   megadpal,    megadriv,    megadrie, ROT0,   "Konami", "Castlevania - The New Generation (Beta)", 0 )
GAME( 1994, g_vamkil, g_casv,   megadriv,    megadriv,    megadrij, ROT0,   "Konami", "Vampire Killer (J) [!]", 0 )


GAME( 1994, g_col,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Columns (W) (REV01) [!]", 0 )
GAME( 1994, g_col00,  g_col,    megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Columns (W) (REV00) [!]", 0 )

// col3j doesn't work well when set to euro / pal, but it's not meant to be run like that ;-)
GAME( 1994, g_col3,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Columns III - Revenge of Columns (U) [!]", 0 )
GAME( 1994, g_col3j,  g_col3,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Columns III - Taiketsu! Columns World (J) [!]", 0 )

// Comix Zone has a 1 line raster error
GAME( 1995, g_comx,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Sega", "Comix Zone (U) [!]", 0 )
GAME( 1995, g_comixe, g_comx,   megadpal,    megadri6,    megadrie, ROT0,   "Sega", "Comix Zone (E) (M4) [!]", 0 )
GAME( 1995, g_comixj, g_comx,   megadriv,    megadri6,    megadrij, ROT0,   "Sega", "Comix Zone (J) [!]", 0 )
GAME( 1995, g_comixb, g_comx,   megadriv,    megadri6,    megadriv, ROT0,   "Sega", "Comix Zone (Beta)", 0 )

// raster abuse ;-)
GAME( 1994, g_cont,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Konami", "Contra - Hard Corps (U) [!]", 0 )
GAME( 1994, g_contrj, g_cont,   megadriv,    megadri6,    megadrij, ROT0,   "Konami", "Contra - The Hard Corps (J) [!]", 0 )
GAME( 1994, g_probot, g_cont,   megadpal,    megadri6,    megadrie, ROT0,   "Konami", "Probotector (E) [!]", 0 )

GAME( 1991, g_cfir,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Kyugo", "CrossFire (U) [c][!]", 0 )
GAME( 1991, g_sair,   g_cfir,   megadriv,    megadriv,    megadrij, ROT0,   "Kyugo", "Super Airwolf (J) [!]", 0 )

GAME( 1992, g_crue,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Crue Ball (UE) [!]", 0 )
GAME( 1992, g_cruej,  g_crue,   megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts", "Crue Ball (J) [!]", 0 )
GAME( 1992, g_twisf,  g_crue,   megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Twisted Flipper (Beta)", 0 )

GAME( 1994, g_cc,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Crusader of Centy (U) [!]", 0 )
GAME( 1994, g_sole,   g_cc,     megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Soleil (E) [!]", 0 )
GAME( 1994, g_soleif, g_cc,     megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Soleil (F) [!]", 0 )
GAME( 1994, g_soleig, g_cc,     megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Soleil (G) [!]", 0 )
GAME( 1994, g_soleis, g_cc,     megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Soleil (S)", 0 )
GAME( 1994, g_soleib, g_cc,     megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Soleil (Beta)", 0 )
GAME( 1994, g_ragna,  g_cc,     megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,  ROT0,   "Sega", "Ragnacenty (J) [!]", 0 )
GAME( 1994, g_shinch, g_cc,     megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,  ROT0,   "Sega", "Shin Changsegi - Ragnacenty (K) [!]", 0 )

GAME( 1994, g_ccop,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Core / Virgin", "Cyber-Cop (U) [c][!]", 0 )
GAME( 1994, g_corp,   g_ccop,   megadpal,    megadriv,    megadrie, ROT0,   "Core / Virgin", "Corporation (E) [c][!]", 0 )

GAME( 1993, g_cybo,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "Cyborg Justice (W) [!]", 0 )
GAME( 1993, g_cyjusb, g_cybo,   megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "Cyborg Justice (Beta)", 0 )
GAME( 1993, g_robwrb, g_cybo,   megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "Robot Wreckage (Beta)", 0 )


// Requires NVRAM to be initalized to FF, Strange Goal priorities are probably part of the original
GAME( 1994, g_dino,   0,        megadriv,    megadriv,    g_dino,  ROT0,   "Dino Dini / Virgin", "Dino Dini's Soccer (E)", 0 )

GAME( 1992, g_dfry,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Technosoft / Tengen", "Dragon's Fury (UE) [!]", 0 )
GAME( 1992, g_dcmd,   g_dfry,   megadriv,    megadriv,    megadrij, ROT0,   "Technosoft", "Devil Crash MD (J) [!]", 0 )

GAME( 1992, g_devi,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sage's Creation", "Devilish - The Next Possession (U) [c][!]", 0 )
GAME( 1992, g_bdomen, g_devi,   megadriv,    megadriv,    megadrij, ROT0,   "Hot B", "Bad Omen (J) [c][!]", 0 )

GAME( 1994, g_dslay,  0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Sega / Falcom", "Dragon Slayer - The Legend of Heroes (J) [!]", 0 )

GAME( 1994, g_dslay2, 0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Sega / Falcom", "Dragon Slayer II - The Legend of Heroes (J) [!]", 0 )

GAME( 199?, g_ecco,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "ECCO The Dolphin (UE) [!]", 0 )
GAME( 199?, g_eccodj, g_ecco,   megadriv,    megadriv,    megadrij, ROT0,   "Sega / Novotrade", "ECCO The Dolphin (J) [!]", 0 )


GAME( 1994, g_eco2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "ECCO - The Tides of Time (U) [!]", 0 )
GAME( 1994, g_ecco2e, g_eco2,   megadpal,    megadriv,    megadrie, ROT0,   "Sega / Novotrade", "ECCO - The Tides of Time (E) [!]", 0 )
GAME( 1994, g_ecco2j, g_eco2,   megadriv,    megadriv,    megadrij, ROT0,   "Sega / Novotrade", "ECCO - The Tides of Time (J) [!]", 0 )
GAME( 1994, g_ecco2b, g_eco2,   megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "ECCO - The Tides of Time (Beta)", 0 )

// Revision 01 seems to have buggy screen clear?
GAME( 1995, g_ecjr,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "ECCO Jr. (U) (REV00) [!]", 0 )
GAME( 1995, g_ecjr01, g_ecjr,   megadriv,    megadriv,    megadriv, ROT0,   "Sega / Novotrade", "ECCO Jr. (U) (REV01) [!]", 0 )

// needs TAS writeback disabled
GAME( 1992, g_exmu,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Ex-Mutants (U) [!]", 0 )

GAME( 1991, g_f22i,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Edward Lemer", "F-22 Interceptor (U) (Jun 1992) [c][!]", 0 )
GAME( 1991, g_f22u,   g_f22i,   megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Edward Lemer", "F-22 Interceptor (U) (Sep 1991) [c][!]", 0 )
GAME( 1991, g_f22ua,  g_f22i,   megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Edward Lemer", "F-22 Interceptor (U) (Sep 1991) [a1][c][!]", 0 )
GAME( 1991, g_f22j,   g_f22i,   megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts / Edward Lemer", "F-22 Interceptor - Advanced Tactical Fighter (J) [!]", 0 )
GAME( 1991, g_f22b,   g_f22i,   megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Edward Lemer", "F-22 Interceptor (Beta) [c][!]", 0 )


GAME( 1993, g_faf,    0,        megadriv,    megadriv,    megadriv, ROT0,   "SNK / Takara", "Fatal Fury (U) [!]", 0 )
GAME( 1993, g_fatfue, g_faf,    megadpal,    megadriv,    megadrie, ROT0,   "SNK / Takara", "Fatal Fury (E)", 0 )
GAME( 1993, g_garou,  g_faf,    megadriv,    megadriv,    megadrij, ROT0,   "SNK / Takara", "Garou Densetsu - Shukumei no Tatakai (J) [!]", 0 )

GAME( 1994, g_faf2,   0,        megadriv,    megadri6,    megadriv, ROT0,   "SNK / Takara", "Fatal Fury 2 (U) [c][!]", 0 )
GAME( 1994, g_garou2, g_faf2,   megadriv,    megadri6,    megadrij, ROT0,   "SNK / Takara", "Garou Densetsu 2 - Arata-naru Tatakai (J) [c][!]", 0 )


// Fatal Rewind is very sensitive to timing / CPU speed, the clones aren't
GAME( 1991, g_fatr,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Psygnosis", "Fatal Rewind (UE) [!]", GAME_NOT_WORKING )
GAME( 1991, g_kgs,    g_fatr,   megadriv,    megadriv,    megadrij, ROT0,   "Psygnosis", "Killing Game Show, The (J)", GAME_NOT_WORKING )
GAME( 1993, g_kgsr,   g_fatr,   megadriv,    megadriv,    megadrij, ROT0,   "Psygnosis", "Killing Game Show, The - 1993 Remix", GAME_NOT_WORKING )

GAME( 1993, g_fbak,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Delphine / US Gold", "Flashback - The Quest for Identity (U) [!]", 0 )
GAME( 1993, g_fbckua, g_fbak,   megadriv,    megadriv,    megadriv, ROT0,   "Delphine / US Gold", "Flashback - The Quest for Identity (U) [a1][!]", 0 )
GAME( 1993, g_fbckj,  g_fbak,   megadriv,    megadriv,    megadrij, ROT0,   "Delphine / US Gold", "Flashback - The Quest for Identity (J) [!]", 0 )
GAME( 1993, g_fbcke,  g_fbak,   megadpal,    megadriv,    megadrie, ROT0,   "Delphine / US Gold", "Flashback - The Quest for Identity (E) [!]", 0 )

// Japanese version has a screen at the start, no 4 player support warning?
GAME( 1994, g_gau4,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Tengen / Atari", "Gauntlet 4 (UE) (Aug 1993) [!]", 0 )
GAME( 1994, g_gaun4a, g_gau4,   megadpal,    megadriv,    megadrie, ROT0,   "Tengen / Atari", "Gauntlet 4 (UE) (Sep 1993)", 0 )
GAME( 1994, g_gaunt,  g_gau4,   megadriv,    megadriv,    megadrij, ROT0,   "Tengen / Atari", "Gauntlet (J) [!]", 0 )

// needs TAS writeback disabled
GAME( 1995, g_garg,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Buena Vista Interactive", "Gargoyles (U) [!]", 0 )

// Dai Makaimura has a very sensitive sound program, if you overload it with IRQs it crashes
GAME( 1989, g_gng,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Capcom / Sega", "Ghouls 'N Ghosts (UE) (REV02) [!]", 0 )
GAME( 1989, g_gng01,  g_gng,    megadpal,    megadriv,    megadriv, ROT0,   "Capcom / Sega", "Ghouls 'N Ghosts (UE) (REV01) [!]", 0 )
GAME( 1989, g_daim,   g_gng,    megadriv,    megadriv,    megadrij, ROT0,   "Capcom / Sega", "Dai Makaimura (J) [!]", 0 )

GAME( 1994, g_gcm,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Disney / Capcom", "Great Circus Mystery, The - Starring Mickey & Minnie (U) [!]", 0 )
GAME( 1994, g_minnie, g_gcm,    megadriv,    megadriv,    megadrij, ROT0,   "Disney / Capcom", "Mickey Mouse - Minnie's Magical Adventure 2 (J) [!]", 0 )

GAME( 1993, g_gsh,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / Treasure", "Gunstar Heroes (U) [!]", 0 )
GAME( 1993, g_gshe,   g_gsh,    megadpal,    megadriv,    megadrie, ROT0,   "Sega / Treasure", "Gunstar Heroes (E) [!]", 0 )
GAME( 1993, g_gshj,   g_gsh,    megadriv,    megadriv,    megadrij, ROT0,   "Sega / Treasure", "Gunstar Heroes (J) [!]]", 0 )
GAME( 1993, g_gshsam, g_gsh,    megadriv,    megadriv,    megadrij, ROT0,   "Sega / Treasure", "Gunstar Heroes (Sample) (J) [!]", 0 )

GAME( 1990, g_hard,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Tengen / Atari", "Hard Drivin' (W) [!]", 0 )

// There is also an arcade version, but its encrypted / protected
GAME( 1993, g_hsh,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Data East", "High Seas Havoc (U) [!]", 0 )
GAME( 1994, g_captha, g_hsh,    megadpal,    megadriv,    megadrie, ROT0,   "Data East / Codemasters", "Capt'n Havoc (E) [!]", 0 )

// Ronaldinho 98 is a cheap glitchy hack of International Superstar Soccer.  ISS has single line raster error after scoring a goal
GAME( 1996, g_iss,    0,        megadpal,    megadri6,    megadrie, ROT0,   "Konami / Factor 5", "International Superstar Soccer Deluxe (E) [!]", 0 )
GAME( 1998, g_ron98b, g_iss,    megadriv,    megadri6,    megadriv, ROT0,   "bootleg", "Ronaldinho 98 (B) [c][!]", 0 )

GAME( 1993, g_bond,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Domark", "James Bond 007 - The Duel (UE) [!]", 0 )
GAME( 1993, g_jbondw, g_bond,   megadriv,    megadriv,    megadriv, ROT0,   "Domark", "James Bond 007 - The Duel (W)", 0 )
GAME( 1993, g_007,    g_bond,   megadriv,    megadriv,    megadrij, ROT0,   "Domark", "007 Shitou (J) [!]", 0 )

GAME( 1991, g_jp,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Millenium", "James Pond - Underwater Agent (UE) [!]", 0 )

GAME( 1991, g_jp2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Millenium", "James Pond II - Codename RoboCod (UE) [!]", 0 )
GAME( 1991, g_jp2j,   g_jp2,    megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts / Millenium", "James Pond II - Codename RoboCod (J) [!]", 0 )

GAME( 1992, g_aqua,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Vectordean / Millenium", "Aquatic Games - Starring James Pond, The (UE) [!]", 0 )

GAME( 1993, g_jp3,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Vectordean / Millenium", "James Pond 3", 0 )

GAME( 1995, g_jdre,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Acclaim / Probe", "Judge Dredd - The Movie (W) [!]", 0 )
GAME( 1995, g_jdrdb,  g_jdre,   megadriv,    megadriv,    megadriv, ROT0,   "Acclaim / Probe", "Judge Dredd - The Movie (Beta)", 0 )
GAME( 1995, g_jdrdba, g_jdre,   megadriv,    megadriv,    megadriv, ROT0,   "Acclaim / Probe", "Judge Dredd - The Movie (Beta) [a1]", 0 )

GAME( 1994, g_kidc,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Kid Chameleon (UE) [!]", 0 )
GAME( 1994, g_ckid,   g_kidc,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Chameleon Kid (J) [!]", 0 )

GAME( 1994, g_land,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Landstalker - The Treasures of King Nole (U) [!]", 0 )
GAME( 1994, g_landse, g_land,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Landstalker - The Treasures of King Nole (E) [!]", 0 )
GAME( 1994, g_landsg, g_land,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Landstalker - The Treasures of King Nole (G) [!]", 0 )
GAME( 1994, g_landsf, g_land,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Landstalker - The Treasures of King Nole (F)", 0 )
GAME( 1994, g_landsj, g_land,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,  ROT0,   "Sega", "Landstalker - Koutei no Zaihou (J) [!]", 0 )
GAME( 1994, g_landsb, g_land,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Landstalker - The Treasures of King Nole (Beta)", 0 )


GAME( 1993, g_or20,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sims", "OutRun 2019 (U) [!]", 0 )
GAME( 1993, g_2019e,  g_or20,   megadpal,    megadriv,    megadrie, ROT0,   "Sims", "OutRun 2019 (E) [!]", 0 )
GAME( 1993, g_2019j,  g_or20,   megadriv,    megadriv,    megadrij, ROT0,   "Sims", "OutRun 2019 (J) [!]", 0 )
GAME( 1993, g_2019b,  g_or20,   megadriv,    megadriv,    megadriv, ROT0,   "Sims", "OutRun 2019 (Beta)", 0 )
GAME( 1992, g_junkb,  g_or20,   megadriv,    megadriv,    megadriv, ROT0,   "Sims", "Junkers High (Beta)", 0 )

// The US version has broken Graphics, I'm sure it worked at one point
GAME( 199?, g_orur,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / Data East", "OutRunners (U) [!]", GAME_NOT_WORKING )
GAME( 199?, g_orunrj, g_orur,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "OutRunners (J) [!]", 0 )

// broken raster effect (status bar cut in half)
GAME( 1992, g_lem,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Psygnosis / Sunsoft", "Lemmings (JU) (REV01) [!]", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, g_lem00,  g_lem,    megadriv,    megadriv,    megadriv, ROT0,   "Psygnosis / Sunsoft", "Lemmings (JU) (REV00) [!]", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, g_leme,   g_lem,    megadpal,    megadriv,    megadrie, ROT0,   "Psygnosis / Sunsoft", "Lemmings (E) [!]", GAME_IMPERFECT_GRAPHICS )

// Sensitive to timing of Vblank / IRQ pending lines
GAME( 1993, g_mazi,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Mazin Saga (U) [!]", 0 )
GAME( 1993, g_mazie,  g_mazi,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mazin Wars (E) [!]", 0 )
GAME( 1993, g_mazij,  g_mazi,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Mazin Saga (J) [!]", 0 )
GAME( 1993, g_mazib,  g_mazi,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Mazin Saga (Beta)", 0 )

GAME( 1993, g_mtla,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / Treasure / McDonalds", "McDonald's Treasure Land Adventure (U) [!]", 0 )
GAME( 1993, g_mcdtj,  g_mtla,   megadriv,    megadriv,    megadrij, ROT0,   "Sega / Treasure / McDonalds", "McDonald's Treasure Land Adventure (J) [!]", 0 )
GAME( 1993, g_mcdtle, g_mtla,   megadpal,    megadriv,    megadrie, ROT0,   "Sega / Treasure / McDonalds", "McDonald's Treasure Land Adventure (E)", 0 )
GAME( 1993, g_mcdtlb, g_mtla,   megadriv,    megadriv,    megadriv, ROT0,   "Sega / Treasure / McDonalds", "McDonald's Treasure Land Adventure (Beta)", 0 )

GAME( 1994, g_mtur,    0,       megadriv,    megadriv,    megadriv, ROT0,   "Data East / Factor 5", "Mega Turrican (U) [!]", 0 )
GAME( 1994, g_mturre,  g_mtur,  megadpal,    megadriv,    megadrie, ROT0,   "Data East / Factor 5", "Mega Turrican (E) [!]", 0 )

GAME( 1994, g_mick,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Mick & Mack as the Global Gladiators (U) [!]", 0 )
GAME( 1994, g_micmaj, g_mick,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mick & Mack as the Global Gladiators (E) [!]", 0 )
GAME( 1994, g_micmab, g_mick,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Mick & Mack as the Global Gladiators (Beta)", 0 )


GAME( 1994, g_mman,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sony", "Mickey Mania - Timeless Adventures of Mickey Mouse (U) [!]", 0 )
GAME( 1994, g_mmanie, g_mman,   megadpal,    megadriv,    megadrie, ROT0,   "Disney / Sony", "Mickey Mania - Timeless Adventures of Mickey Mouse (E) [!]", 0 )
GAME( 1994, g_mmanij, g_mman,   megadriv,    megadriv,    megadrij, ROT0,   "Disney / Sony", "Mickey Mania - Timeless Adventures of Mickey Mouse (J) [!]", 0 )
GAME( 1994, g_mmanib, g_mman,   megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sony", "Mickey Mania - Timeless Adventures of Mickey Mouse (Beta)", 0 )

GAME( 1993, g_mic,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Codemasters", "Micro Machines (UE) [c][!]", 0 ) // black title screen bg
GAME( 1993, g_mica,   g_mic,    megadriv,    megadriv,    megadriv, ROT0,   "Codemasters", "Micro Machines (UE) [a1][c][!]", 0 ) // blue title screen bg
GAME( 1993, g_micmc,  g_mic,    megadriv,    megadriv,    megadriv, ROT0,   "Codemasters", "Micro Machines (C)", 0 )

GAME( 1994, g_mic2,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Codemasters", "Micro Machines 2 - Turbo Tournament (E) [c][!]", 0 )
GAME( 1994, g_micm2e, g_mic2,   megadpal,    megadriv,    megadrie, ROT0,   "Codemasters", "Micro Machines 2 - Turbo Tournament (E) (J-Cart) [c][!]", 0 )

GAME( 1995, g_mic9,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Codemasters", "Micro Machines - Turbo Tournament '96 (V1.1) (E) (J-Cart) [c][!]", 0 )
GAME( 1995, g_micm96, g_mic9,   megadpal,    megadriv,    megadrie, ROT0,   "Codemasters", "Micro Machines - Turbo Tournament '96 (V1.1) (E)", 0 )

GAME( 1996, g_micm,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Codemasters", "Micro Machines Military - It's a Blast! (E) (J-Cart) [c][!]", 0 )
GAME( 1996, g_mimmx,  g_micm,   megadpal,    megadriv,    megadrie, ROT0,   "Codemasters", "Micro Machines Military - It's a Blast! (E) [x]", 0 )


GAME( 1992, g_ootw,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Delphine / Virgin", "Out of this World (U) [!]", 0 )
GAME( 1992, g_awld,   g_ootw,   megadpal,    megadriv,    megadrie, ROT0,   "Delphine / Virgin", "Another World (E) [!]", 0 )
GAME( 1992, g_ootwb,  g_ootw,   megadriv,    megadriv,    megadriv, ROT0,   "Delphine / Virgin", "Out of this World (Beta)", 0 )


GAME( 1994, g_page,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Probe", "Pagemaster, The (U) [!]", 0 )
GAME( 1994, g_pageme, g_page,   megadpal,    megadriv,    megadrie, ROT0,   "Probe", "Pagemaster, The (E) [!]", 0 )
GAME( 1994, g_pagemb, g_page,   megadpal,    megadriv,    megadrie, ROT0,   "Probe", "Pagemaster, The (Beta)", 0 )

GAME( 1989, g_pst2,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Phantasy Star II (UE) (REV02) [!]", 0 )
GAME( 1989, g_pst201, g_pst2,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Phantasy Star II (UE) (REV01)", 0 )
GAME( 1989, g_pstr2j, g_pst2,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,  ROT0,   "Sega", "Phantasy Star II (J) [!]", 0 )
GAME( 1989, g_phst2b, g_pst2,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Phantasy Star II (B) (REV01) [!]", 0 )

GAME( 1991, g_pst3,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Phantasy Star III - Generations of Doom (UE) [!]", 0 )
GAME( 1991, g_pstr3j, g_pst3,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,  ROT0,   "Sega", "Phantasy Star III - Toki no Keishousha (J) [!]", 0 )
GAME( 1991, g_phst3b, g_pst3,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Phantasy Star III - Generations of Doom (B) [!]", 0 )

GAME( 1994, g_pst4,   0,        megadriv,    megadriv,    g_pst4,   ROT0,   "Sega", "Phantasy Star - The End of the Millenium (U) [!]", 0 )
GAME( 1994, g_pstr4j, g_pst4,   megadriv,    megadriv,    g_pst4j,  ROT0,   "Sega", "Phantasy Star - The End of the Millenium (J) [!]", 0 )

GAME( 1993, g_prin,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Domark", "Prince of Persia (U) [!]", 0 )
GAME( 1993, g_perse,  g_prin,   megadpal,    megadriv,    megadrie, ROT0,   "Domark", "Prince of Persia (E)", 0 )
GAME( 1993, g_persb1, g_prin,   megadriv,    megadriv,    megadriv, ROT0,   "Domark", "Prince of Persia (Beta 1)", 0 )
GAME( 1993, g_persb2, g_prin,   megadriv,    megadriv,    megadriv, ROT0,   "Domark", "Prince of Persia (Beta 2)", 0 )

GAME( 1991, g_quac,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sega", "Quack Shot Starring Donald Duck (W) (REV01) [!]", 0 )
GAME( 1994, g_quac00, g_quac,   megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sega", "Quack Shot Starring Donald Duck (W) (REV00) [!]", 0 )
GAME( 1994, g_quac01, g_quac,   megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sega", "Quack Shot Starring Donald Duck (W) (REV01) [a1][c][!]", 0 )

GAME( 1993, g_race,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Tengen / Atari", "Race Drivin' (U) [!]", 0 )

GAME( 1990, g_revs,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Revenge of Shinobi, The (W) (REV03) [!]", 0 )
GAME( 1990, g_rshi00, g_revs,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Revenge of Shinobi, The (W) (REV00) [!]", 0 )
GAME( 1990, g_rshi01, g_revs,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Revenge of Shinobi, The (W) (REV01) [!]", 0 )
GAME( 1990, g_rshi02, g_revs,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Revenge of Shinobi, The (W) (REV02) [!]", 0 )

GAME( 1994, g_sams,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Sega", "Samurai Shodown (U) [!]", 0 )
GAME( 1994, g_samshe, g_sams,   megadpal,    megadri6,    megadrie, ROT0,   "Sega", "Samurai Shodown (E) [c][!]", 0 )
GAME( 1994, g_samspi, g_sams,   megadriv,    megadri6,    megadrij, ROT0,   "Sega", "Samurai Spirits (J) [!]", 0 )


GAME( 1991, g_sitd,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining in the Darkness (UE) [!]", 0 )
GAME( 1991, g_shdrkj, g_sitd,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000 , ROT0,   "Sega", "Shining and the Darkness (J) [!]", 0 )
GAME( 1991, g_shdrkb, g_sitd,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining in the Darkness (B) [!]", 0 )

GAME( 1993, g_shf1,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining Force (U) [!]", 0 )
GAME( 1993, g_shfrcj, g_shf1,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining Force - The Legacy of Great Intention (J) [!]", 0 )
GAME( 1993, g_sfrcbt, g_shf1,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining Force (Beta)", 0 )

GAME( 1994, g_shf2,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining Force II (U) [!]", 0 )
GAME( 1994, g_shfr2e, g_shf2,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining Force II (E) [!]", 0 )
GAME( 1994, g_shfr2j, g_shf2,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000,  ROT0,   "Sega", "Shining Force II - Inishie no Fuuin (J) [!]", 0 )


GAME( 1993, g_shi3,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Shinobi III - Return of the Ninja Master (U) [!]", 0 )
GAME( 1993, g_shin3e, g_shi3,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Shinobi III - Return of the Ninja Master (E) [c][!]", 0 )
GAME( 1993, g_sshin2, g_shi3,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Super Shinobi II, The (J) [!]", 0 )
GAME( 1993, g_sshi2b, g_shi3,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Super Shinobi II, The (Beta)", 0 )

GAME( 1995, g_skid,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Codemasters / Acid", "Super Skidmarks (E) (J-Cart) [!]", 0 )
GAME( 1995, g_skida,  g_skid,   megadpal,    megadriv,    megadrie, ROT0,   "Codemasters / Acid", "Super Skidmarks (E) [a1]", 0 )

GAME( 1993, g_ranx,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Sega / Gau", "Ranger-X (U) [!]", 0 )
GAME( 1993, g_ranxe,  g_ranx,   megadpal,    megadri6,    megadrie, ROT0,   "Sega / Gau", "Ranger-X (E) [!]", 0 )
GAME( 1993, g_exranz, g_ranx,   megadriv,    megadri6,    megadrij, ROT0,   "Sega / Gau", "Ex-Ranza (J) [!]", 0 )
GAME( 1993, g_exrnzb, g_ranx,   megadriv,    megadri6,    megadrij, ROT0,   "Sega / Gau", "Ex-Ranza (Beta)", 0 )


// the road rash series are sensitive to timing
GAME( 1991, g_rrsh,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Road Rash (UE) [c][!]", 0 )

GAME( 1992, g_rrs2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Road Rash II (UE) (REV00) [!]", 0 )
GAME( 1992, g_rrsh22, g_rrs2,   megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Road Rash II (UE) (REV02) [c][!]", 0 )
GAME( 1992, g_rrsh2j, g_rrs2,   megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts", "Road Rash II (J) [!]", 0 )

GAME( 1995, g_rrs3,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Road Rash 3 (UE) [!]", 0 )

GAME( 1993, g_robt,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Virgin", "Robocop Versus The Terminator (U) [!]", 0 )
GAME( 1993, g_robte,  g_robt,   megadpal,    megadriv,    megadrie, ROT0,   "Virgin", "Robocop Versus The Terminator (E)", 0 )
GAME( 1993, g_robtb1, g_robt,   megadriv,    megadriv,    megadriv, ROT0,   "Virgin", "Robocop Versus The Terminator (Beta 1)", 0 )
GAME( 1993, g_robtb2, g_robt,   megadriv,    megadriv,    megadriv, ROT0,   "Virgin", "Robocop Versus The Terminator (Beta 2)", 0 )

GAME( 1993, g_robo3,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Acclaim", "Robocop 3 (UE) [!]", 0 )

GAME( 1992, g_rolo,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts / Vectordean", "Rolo to the Rescue (UE) [!]", 0 )
GAME( 1992, g_zouzou, g_rolo,   megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts / Vectordean", "Zou! Zou! Zou! Rescue Daisenryaku (J) [!]", 0 )


// The region check fails on the beta, why?
GAME( 1994, g_seni,   0,        megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Sensible Software", "Sensible Soccer - International Edition (E) (M4) [!]", 0 )
GAME( 1993, g_sen,    g_seni,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Sensible Software", "Sensible Soccer (E) (M4) [!]", 0 )
GAME( 1993, g_sensib, g_seni,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Sensible Software", "Sensible Soccer (Beta)", GAME_NOT_WORKING ) // region check fails?

GAME( 1990, g_smgp,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Super Monaco Grand Prix (W) (M2) (REV02) [!]", 0 )
GAME( 1990, g_smgp00, g_smgp,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Super Monaco Grand Prix (W) (M2) (REV00) [!]", 0 )
GAME( 1990, g_smgp01, g_smgp,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Super Monaco Grand Prix (W) (M2) (REV01) [!]", 0 )
GAME( 1990, g_smgp03, g_smgp,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Super Monaco Grand Prix (U) (M2) (REV03) [!]", 0 )

GAME( 1991, g_soni,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic The Hedgehog (W) (REV01) [!]", 0 )
GAME( 1991, g_soni00, g_soni,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic The Hedgehog (W) (REV00) [!]", 0 )

GAME( 1992, g_son2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic The Hedgehog 2 (W) (REV01) [!]", 0 )
GAME( 1992, g_son2a,  g_son2,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic The Hedgehog 2 (W) (REVSC02)", 0 )
GAME( 1992, g_son200, g_son2,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic The Hedgehog 2 (W) (REV00) [!]", 0 )
GAME( 1992, g_soni2p, g_son2,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic The Hedgehog 2 (Prototype) [!]", 0 )

GAME( 1994, g_son3,   0,        megadriv,    megadriv,    g_sks3,   ROT0,   "Sega", "Sonic The Hedgehog 3 (U) [!]", 0 )
GAME( 1994, g_son3e,  g_son3,   megadpal,    megadriv,    g_sks3e,  ROT0,   "Sega", "Sonic The Hedgehog 3 (E) [!]", 0 )
GAME( 1994, g_son3j,  g_son3,   megadriv,    megadriv,    g_sks3j,  ROT0,   "Sega", "Sonic The Hedgehog 3 (J) [!]", 0 )

GAME( 1996, g_s3d,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic 3D Blast (UE) [!]", 0 )

// you could plug any other cart into Sonic+Knuckles, you got a random generated bonus stage based on that cart, for Sonic 2 and Sonic 3 you got extended games
GAME( 1994, g_snkn,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic and Knuckles (W) [!]", 0 )
GAME( 1994, g_sks1,   g_snkn,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic and Knuckles & Sonic 1 (W) [!]", 0 ) // aka sonic special stages
GAME( 1994, g_sks2,   g_snkn,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic and Knuckles & Sonic 2 (W) [!]", 0 )
GAME( 1994, g_sks3,   g_snkn,   megadriv,    megadriv,    g_sks3,   ROT0,   "Sega", "Sonic and Knuckles & Sonic 3 (W) [!]", 0 )

// Sonic Spinball can be quite glitchy at times with the collisions, but the original was like this.
GAME( 1993, g_sons,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic Spinball (U) [!]", 0 )
GAME( 1993, g_sspa,   g_sons,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic Spinball (U) (alt music) [!]", 0 )
GAME( 1993, g_sonse,  g_sons,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Sonic Spinball (E) [!]", 0 )
GAME( 1993, g_sonsj,  g_sons,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Sonic Spinball (J) [!]", 0 )
GAME( 1993, g_sonsb,  g_sons,   megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic Spinball (Beta)", 0 )

GAME( 1993, g_sock,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Vic Tokai", "Socket (U) [!]", 0 )
GAME( 1993, g_tdom1,  g_sock,   megadriv,    megadriv,    megadrij, ROT0,   "Vic Tokai", "Time Dominator 1st (J) [!]", 0 )

GAME( 1995, g_s_as,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Acclaim", "Spider-Man - The Animated Series (W) [!]", 0 )
GAME( 1995, g_s_asb1, g_s_as,   megadriv,    megadriv,    megadriv, ROT0,   "Acclaim", "Spider-Man - The Animated Series (Beta 1)", 0 )
GAME( 1995, g_s_asb2, g_s_as,   megadriv,    megadriv,    megadriv, ROT0,   "Acclaim", "Spider-Man - The Animated Series (Beta 2)", 0 )

GAME( 199?, g_spo2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Virgin / Acclaim / Eurocom", "Spot Goes to Hollywood (U) (REV01) [!]", 0 )
GAME( 199?, g_spot2e, g_spo2,   megadpal,    megadriv,    megadrie, ROT0,   "Virgin / Acclaim / Eurocom", "Spot Goes to Hollywood (E) [!]", 0 )

GAME( 1994, g_semp,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Hot B / Acclaim", "Steel Empire, The (UE) [!]", 0 )
GAME( 1994, g_kout,   g_semp,   megadriv,    megadriv,    megadrij, ROT0,   "Hot B", "Koutetsu Teikoku (J) [!]", 0 )
GAME( 1994, g_sempb,  g_semp,   megadriv,    megadriv,    megadriv, ROT0,   "Hot B / Acclaim", "Steel Empire, The (Beta)", 0 ) // Empire of Steel

GAME( 1991, g_sor,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Streets of Rage (W) (REV01) [!]", 0 )
GAME( 1991, g_sor00,  g_sor,    megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Streets of Rage (W) (REV00) [!]", 0 )

GAME( 1992, g_sor2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Streets of Rage 2 (U) [!]", 0 )
GAME( 1992, g_sor2je, g_sor2,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Streets of Rage 2 (JE) [!]", 0 )
GAME( 1992, g_bk2b,   g_sor2,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Bare Knuckle II (Beta)", 0 )

GAME( 1994, g_sor3,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Sega", "Streets of Rage 3 (U) [!]", 0 )
GAME( 1994, g_sor3e,  g_sor3,   megadpal,    megadri6,    megadrie, ROT0,   "Sega", "Streets of Rage 3 (E) (Apr 1994) [!]", 0 )
GAME( 1994, g_sor3ea, g_sor3,   megadpal,    megadri6,    megadrie, ROT0,   "Sega", "Streets of Rage 3 (E) (May 1994) [!]", 0 )
GAME( 1994, g_bar3,   g_sor3,   megadriv,    megadri6,    megadrij, ROT0,   "Sega", "Bare Knuckle III (J) [!]", 0 )
GAME( 1994, g_bk3b,   g_sor3,   megadriv,    megadri6,    megadrij, ROT0,   "Sega", "Bare Knuckle III (Beta)", 0 )

// this makes use of the hardware sprite collision
GAME( 1994, g_str2,    0,       megadriv,    megadriv,    megadriv, ROT0,   "Tiertex / Capcom / US Gold", "Journey From Darkness - Strider Returns (U) [c][!]", 0 )
GAME( 1994, g_strid2,  g_str2,  megadpal,    megadriv,    megadrie, ROT0,   "Tiertex / Capcom / US Gold", "Strider II (E) [c][!]", 0 )

// Beta version 1  crashes the Z80, and MAME ...
GAME( 1993, g_subt,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Zyrinx", "Sub-Terrania (U) [!]", 0 )
GAME( 1993, g_subte,  g_subt,   megadpal,    megadriv,    megadrie, ROT0,   "Zyrinx", "Sub-Terrania (E) [!]", 0 )
GAME( 1993, g_subtj,  g_subt,   megadriv,    megadriv,    megadrij, ROT0,   "Zyrinx", "Sub-Terrania (J) [!]", 0 )
GAME( 1993, g_subtb1, g_subt,   megadriv,    megadriv,    megadriv, ROT0,   "Zyrinx", "Sub-Terrania (Beta 1)", GAME_NOT_WORKING ) // this crashes (z80 crash)
GAME( 1993, g_subtb2, g_subt,   megadriv,    megadriv,    megadriv, ROT0,   "Zyrinx", "Sub-Terrania (Beta 2)", 0 )

// Custom banking, this should be set to use the 6-button Pad, once it's emulated ;-)
GAME( 1994, g_ssf2,   0,        megadriv,    megadri6,    g_ssf2,   ROT0,   "Capcom", "Super Street Fighter II - The New Challengers (U) [c][!]", 0 )
GAME( 1994, g_ssf2e,  g_ssf2,   megadpal,    megadri6,    g_ssf2e,  ROT0,   "Capcom", "Super Street Fighter II - The New Challengers (E) [c][!]", 0 )
GAME( 1994, g_ssf2j,  g_ssf2,   megadriv,    megadri6,    g_ssf2j,  ROT0,   "Capcom", "Super Street Fighter II - The New Challengers (J) [c][!]", 0 )

GAME( 1992, g_lighfr, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Tecnosoft", "Lightening Force - Quest for the Darkstar (U) [c][!]", 0 )
GAME( 1992, g_tf4,    g_lighfr, megadriv,    megadriv,    megadrij, ROT0,   "Tecnosoft", "Thunder Force IV (J) [!]", 0 )
GAME( 1992, g_tf4e,   g_lighfr, megadpal,    megadriv,    megadrie, ROT0,   "Tecnosoft", "Thunder Force IV (E) [c][!]", 0 )

GAME( 1991, g_tje00,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / JVP", "Toejam & Earl (U) (REV00) [!]", 0 )
GAME( 1991, g_tje,    g_tje00,  megadriv,    megadriv,    megadrij, ROT0,   "Sega / JVP", "Toejam & Earl (J) (REV02) [!]", 0 )

GAME( 1993, g_tje2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / JVP", "Toejam & Earl in Panic on Funkotron (U) [!]", 0 )
GAME( 1993, g_tje2e,  g_tje2,   megadpal,    megadriv,    megadrie, ROT0,   "Sega / JVP", "Toejam & Earl in Panic on Funkotron (E) [!]", 0 )
GAME( 1993, g_tje2g,  g_tje2,   megadpal,    megadriv,    megadrie, ROT0,   "Sega / JVP", "Toejam & Earl in Panic on Funkotron (G) [!]", 0 )
GAME( 1993, g_tje2j,  g_tje2,   megadriv,    megadriv,    megadrij, ROT0,   "Sega / JVP", "Toejam & Earl in Panic on Funkotron (J) [!]", 0 )

// very fussy about irq6 pending flag
GAME( 1994, g_tyra,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Tyrants - Fight Through Time (U) [!]", 0 )
GAME( 1994, g_mlo,    g_tyra,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Lo Mania (E) (REV00) [c][!]", 0 )
GAME( 1994, g_mlom01, g_tyra,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Lo Mania (E) (REV01)", 0 )
GAME( 1994, g_mloj,   g_tyra,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Mega Lo Mania (J) [!]", 0 )

GAME( 1995, g_vect,   0,        megadriv,   megadriv,     megadriv, ROT0,   "Sega", "Vectorman (UE) [!]", 0 )
GAME( 1995, g_vectp,  g_vect,   megadriv,   megadriv,     megadriv, ROT0,   "Sega", "Vectorman (UE) (Prototype) [!]", 0 )
GAME( 1995, g_vectb,  g_vect,   megadriv,   megadriv,     megadriv, ROT0,   "Sega", "Vectorman (Beta)", 0 )

GAME( 1996, g_vec2,   0,        megadriv,   megadriv,     megadriv, ROT0,   "Sega", "Vectorman 2 (U) [!]", 0 )
GAME( 1996, g_vect2p, g_vec2,   megadriv,   megadriv,     megadriv, ROT0,   "Sega", "Vectorman 2 (U) (Prototype) [!]", 0 )

GAME( 1992, g_wani,   0,        megadriv,    megadriv,    megadrij, ROT0,   "Kaneko", "Wani Wani World (J) [c][!]", 0 )

// broken, reason unknown, DMAs over all of VRAM
GAME( 1994, g_wloc,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Trimark / Acclaim", "Warlock (UE) [!]", 0 )
GAME( 1994, g_warlob, g_wloc,   megadpal,    megadriv,    megadrie, ROT0,   "Trimark / Acclaim", "Warlock (Beta)", 0 )

GAME( 1991, g_wrom,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Micronet", "Warrior of Rome (U) [!]", 0 )
GAME( 1991, g_caeno,  g_wrom,   megadriv,    megadriv,    megadrij, ROT0,   "Micronet", "Caesar no Yabo (J)", 0 )

GAME( 1992, g_wrom2,  0,        megadriv,    megadriv,    g_wrom2,  ROT0,   "Micronet", "Warrior of Rome II (U) [c][!]", 0 )
GAME( 1992, g_caeno2, g_wrom2,  megadriv,    megadriv,    g_caeno2, ROT0,   "Micronet", "Caesar no Yabo II (J)", 0 )

GAME( 1991, g_winwor, 0,        megadriv,    megadriv,    megadriv, ROT0,   "NCS / Dreamworks", "Wings of Wor (U) [!]", 0 )
GAME( 1991, g_gynoge, g_winwor, megadpal,    megadriv,    megadrie, ROT0,   "NCS", "Gynoug (E) [!]", 0 )
GAME( 1991, g_gyno,   g_winwor, megadriv,    megadriv,    megadrij, ROT0,   "NCS", "Gynoug (J) [!]", 0 )

GAME( 1994, g_wcs,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "World Championship Soccer (JU) (REV03) [!]", 0 )
GAME( 1994, g_wcs02,  g_wcs,    megadriv,    megadriv,    megadriv, ROT0,   "Sega", "World Championship Soccer (JU) (REV02)", 0 )
GAME( 1994, g_wcs00,  g_wcs,    megadriv,    megadriv,    megadriv, ROT0,   "Sega", "World Championship Soccer (JU) (REV00) [!]", 0 )
GAME( 1994, g_wc90,   g_wcs,    megadpal,    megadriv,    megadrie, ROT0,   "Sega", "World Cup Italia 90 (E) [!]", 0 )

GAME( 1994, g_wild,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Bullet-Proof Software", "Wild Snake (U) (Prototype) [!]", 0 )

// I don't know where the Japanese title comes from, box art?
GAME( 1992, g_will,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sega", "World of Illusion Starring Mickey Mouse & Donald Duck (U) [!]", 0 )
GAME( 1992, g_wille,  g_will,   megadpal,    megadriv,    megadrie, ROT0,   "Disney / Sega", "World of Illusion Starring Mickey Mouse & Donald Duck (E) [!]", 0 )
GAME( 1992, g_willb,  g_will,   megadriv,    megadriv,    megadriv, ROT0,   "Disney / Sega", "World of Illusion Starring Mickey Mouse & Donald Duck (Beta)", 0 )
GAME( 1992, g_micdo,  g_will,   megadriv,    megadriv,    megadrij, ROT0,   "Disney / Sega", "I Love Mickey & Donald - Fushigi na Magic Box (J)", 0 )

GAME( 1996, g_xper,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Sega", "X-perts (U) [!]", 0 )

// The only games to use Sega's SVP chip
GAME( 199?, g_virr,   0,        megdsvp,   megdsvp,     megadsvp, ROT0,   "Sega", "Virtua Racing (U) [!]", 0 )
GAME( 199?, g_virraj, g_virr,   megdsvp,   megdsvp,     megadsvp, ROT0,   "Sega", "Virtua Racing (J) [!]", GAME_NOT_WORKING )
GAME( 199?, g_virrae, g_virr,   megdsvp,   megdsvp,     megadsvp, ROT0,   "Sega", "Virtua Racing (E) [!]", GAME_NOT_WORKING )
GAME( 199?, g_virrea, g_virr,   megdsvp,   megdsvp,     megadsvp, ROT0,   "Sega", "Virtua Racing (E) [a1]", GAME_NOT_WORKING )

/********************************* Official MultiGame Packs **************************/

GAME( 1991, g_discol, 0,        megadpal,    megadriv,    megadrie, ROT0,   "Disney / Sega", "Disney Collection - Castle of Illusion & Quack Shot (E) [!]", 0 )

GAME( 199?, g_clascl, 0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Classic Collection (UE) [!]", 0 )

GAME( 199?, g_gen6pk, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Genesis 6-Pak (U) [!]", 0 )

GAME( 199?, g_mg1e,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Games I (E) [!]", 0 )
GAME( 199?, g_mg2e,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Games 2 (E) [!]", 0 )
GAME( 199?, g_mg3e,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Games 3 (E) [!]", 0 )

GAME( 199?, g_mg6v1e, 0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Games 6 (Vol 1) (E) [!]", 0 )
GAME( 199?, g_mg6v2e, 0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Games 6 (Vol 2) (E) [!]", 0 )
GAME( 199?, g_mg6v3e, 0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Games 6 (Vol 3) (E) [!]", 0 )

GAME( 199?, g_mg10i1, 0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Mega Games 10 in 1 (EB) [!]", 0 )

GAME( 199?, g_segsp1, 0,        megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Sega Sports 1 (Wimbledon, Ult.Soccer, Super Monaco) (E) [!].", 0 )

GAME( 199?, g_stop5b, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sega Top 5 (B) [!]", 0 )

GAME( 199?, g_sonclu, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sonic Classics (Compilation) (UE) (REV01) [!]", 0 )
GAME( 199?, g_soncle, g_sonclu, megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Sonic Classics (Compilation) (E) (REV00) [!]", 0 )

// it contains 3 games, but no menu??
GAME( 199?, g_sptgb,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Sport Games (B) [c][!]", GAME_NOT_WORKING )

// needs light gun emulation
GAME( 199?, g_menace, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Menacer 6-in-1 Game Pack (UE) [!]", GAME_NOT_WORKING )

/********************************* Radica System Games *******************************/
/* The Radica is/was a TV game system. An all-in-1 battery operated Megadrive unit
  with built in games ROM.

  Apparently the FM reproduction wasn't quite 100% correct on it, but it's impossible
  to emulate the differences without extensive testing
*/

GAME( 2004, radicav1,   0,        megadriv,    megadriv,    radicav1, ROT0,   "Radica Games / Sega", "Radica: Volume 1 (Sonic the Hedgehog, Altered Beast, Golden Axe, Dr. Robotnik's Mean Bean Machine, Kid Chameleon, Flicky) (US)", 0 )
GAME( 2004, radicasf,   0,        megadpal,    megadri6,    radicasf, ROT0,   "Radica Games / Capcom", "Radica: Street Fighter Pack (Street Fighter 2' Special Champion Edition, Ghouls and Ghosts) (EURO)", 0 )


/********************************* Sega Channel / Seganet Games **********************/

// fussy about 'unused' bits in vdp control register
GAME( 1994, g_gameno, 0,        megadriv,    megadriv,    g_gameno, ROT0,   "Sega", "Game no Kanzume Otokuyou (J) [!]", 0 )

// check years etc.
GAME( 1994, g_ddpl,   0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Doki Doki Penguin Land MD (SN) (J) [!]", 0 )
GAME( 1994, g_16ton,  0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "16 Ton (SN) (J) [!]", 0 )
GAME( 1994, g_hymar,  0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Hyper Marbles (SN) (J) [!]", 0 )
GAME( 1994, g_labyd,  0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Labyrinth of Death (SN) (J) [!]", 0 )
GAME( 1994, g_padfi,  0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Paddle Fighter (SN) (J) [!]", 0 )
GAME( 1994, g_p2anne, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Phantasy Star II - Anne's Adventure (SN) (J) [!]", 0 )
GAME( 1994, g_p2huey, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Phantasy Star II - Huey's Adventure (SN) (J) [!]", 0 )
GAME( 1994, g_p2kind, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Phantasy Star II - Kinds's Adventure (SN) (J) [!]", 0 )
GAME( 1994, g_p2shil, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Phantasy Star II - Shilka's Adventure (SN) (J) [!]", 0 )
GAME( 1994, g_putter, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Putter Golf (SN) (J) [!]", 0 )
GAME( 1994, g_pymag,  0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Pyramid Magic (SN) (J) [!]", 0 )
GAME( 1994, g_pymag2, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Pyramid Magic II (SN) (J) [!]", 0 )
GAME( 1994, g_pymag3, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Pyramid Magic III (SN) (J) [!]", 0 )
GAME( 1994, g_pymags, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Pyramid Magic Special (SN) (J) [!]", 0 )
GAME( 1994, g_serase, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Sonic Eraser (SN) (J) [!]", 0 )
GAME( 1900, g_megamd, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "MegaMind (SN) (J)", 0 )
GAME( 1900, g_aworg,  0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Aworg (SN) (J)", 0 )
GAME( 1900, g_teddy,  0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Teddy Boy Blues (SN) (J)", 0 )
GAME( 1900, g_robobt, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Robot Battler (SN) (J)", 0 )
GAME( 1900, g_medalc, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Medal City (SN) (J)", 0 )
GAME( 1900, g_riddle, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Riddle Wired (SN) (J)", 0 )
GAME( 1900, g_kisssh, 0,        megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Kiss Shot (SN) (J)", 0 )



/********************************* Unlicensed Games **********************************/


/********************************* Not Fully Sorted ****************************************/
GAME( 1900, g_2020,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Baseball 2020 (UE) [!]", 0 )
GAME( 1900, g_2020j,  g_2020,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Super Baseball 2020 (J) [!]", 0 )

GAME( 1900, g_aate,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Andre Agassi Tennis (U) (REV00) [!]", 0 )
GAME( 1900, g_aatee,  g_aate,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Andre Agassi Tennis (E) (REV01) [c][!]", 0 )
GAME( 1900, g_aateb,  g_aate,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Andre Agassi Tennis (Beta)", 0 )

GAME( 1900, g_afam,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Addams Family, The (UE) [!]", 0 )
GAME( 1900, g_adamb,  g_afam,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Addams Family, The (Beta)", 0 )

GAME( 1900, g_mmax,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Adventures of Mighty Max, The (U) [!]", 0 )
GAME( 1900, g_mmaxe,  g_mmax,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Adventures of Mighty Max, The (E)", 0 )

GAME( 1900, g_abus,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Air Buster (U) [c][!]", 0 )
GAME( 1900, g_aerobl, g_abus,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Aero Blasters (J) [!]", 0 )

GAME( 1900, g_abu2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "After Burner II (UE) [!]", 0 )
GAME( 1900, g_abrn2j, g_abu2,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "After Burner II (J) [!]", 0 )

GAME( 1900, g_ali3,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Alien 3 (UE) (REV01) [!]", 0 )
GAME( 1900, g_ali300, g_ali3,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Alien 3 (UE) (REV00) [!]", 0 )

GAME( 1900, g_arcu,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Arcus Odyssey (U) [!]", 0 )
GAME( 1900, g_arcusj, g_arcu,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Arcus Odyssey (J) [!]", 0 )

GAME( 1900, g_asol,   0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Alien Soldier (J) [!]", 0 )
GAME( 1900, g_asolde, g_asol,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Alien Soldier (E) [!]", 0 )

GAME( 1900, g_alis,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Alisia Dragoon (U) [!]", 0 )
GAME( 1900, g_alisie, g_alis,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Alisia Dragoon (E) [!]", 0 )
GAME( 1900, g_alisij, g_alis,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Alisia Dragoon (J) [!]", 0 )

GAME( 1900, g_anim,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Animaniacs (U) [!]", 0 )
GAME( 1900, g_anime,  g_anim,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Animaniacs (E) [!]", 0 )

GAME( 1900, g_asgr,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Asterix and the Great Rescue (E) (M5) [c][!]", 0 )
GAME( 1900, g_astgru, g_asgr,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Asterix and the Great Rescue (U) [!]", 0 )

GAME( 1900, g_aspg,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Asterix and the Power of The Gods (E) (M4) [!]", 0 )
GAME( 1900, g_astpgx, g_aspg,   megadriv,    megadriv,    megadrie, ROT0,   "Unsorted", "Asterix and the Power of The Gods (E) (M5) [x] (Beta?)", 0 )


GAME( 1900, g_akid,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Atomic Robo-Kid (U) [c][!]", 0 )
GAME( 1900, g_robokj, g_akid,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Atomic Robo-Kid (J) [c][!]", 0 )

GAME( 1900, g_awep,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Awesome Possum Kicks Dr. Machino's Butt! (U) [!]", 0 )
GAME( 1900, g_awsep,  g_awep,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Awesome Possum Kicks Dr. Machino's Butt! (Beta)", 0 )

GAME( 1900, g_beav,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Beavis and Butt-head (U) [!]", 0 )
GAME( 1900, g_beave,  g_beav,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Beavis and Butt-head (E) [!]", 0 )
GAME( 1900, g_beavib, g_beav,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Beavis and Butt-head (Beta)", 0 )

GAME( 1900, g_botb,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Best of the Best - Championship Karate (U) [c][!]", 0 )
GAME( 1900, g_botbb,  g_botb,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Best of the Best - Championship Karate (Beta)", 0 )
GAME( 1900, g_kbox,   g_botb,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Kick Boxing, The (J) [!]", 0 )


GAME( 1900, g_buba,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bubba N Stix (U) [!]", 0 )
GAME( 1900, g_bubbae, g_buba,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Bubba N Stix (E) [c][!]", 0 )
GAME( 1900, g_bubbab, g_buba,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bubba N Stix (Beta)", 0 )

GAME( 1900, g_budo,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Budokan - The Martial Spirit (U) [c][!]", 0 )
GAME( 1900, g_budoe,  g_budo,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Budokan - The Martial Spirit (E) [!]", 0 )

GAME( 1900, g_bhb,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bio-Hazard Battle (UE) [!]", 0 )
GAME( 1900, g_biohzb, g_bhb,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bio-Hazard Battle (Beta)", 0 )
GAME( 1900, g_crying, g_bhb,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Crying - Asia Seimei Sensou (J) [!]", 0 )

GAME( 1900, g_body,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Body Count (E) (M5) [!]", 0 )
GAME( 1900, g_bcounb, g_body,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Body Count (Beta) (M5)", 0 )

GAME( 1900, g_bnza,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bonanza Bros. (U) [!]", 0 )
GAME( 1900, g_bnza00, g_bnza,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bonanza Bros. (JE) (REV00) [!]", 0 )
GAME( 1900, g_bnza01, g_bnza,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bonanza Bros. (JE) (REV01) [!]", 0 )

GAME( 1900, g_boog,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Boogerman (U) [!]", 0 )
GAME( 1900, g_booge,  g_boog,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Boogerman (E) [!]", 0 )

GAME( 1900, g_boxl,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Boxing Legends of the Ring (U) [!]", 0 )
GAME( 1900, g_chav2,  g_boxl,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chavez II (U) [!]", 0 )

GAME( 1900, g_brpw,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Brutal - Paws of Fury (U) [!]", 0 )
GAME( 1900, g_brutle, g_brpw,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Brutal - Paws of Fury (E)", 0 )

GAME( 1900, g_burf,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Burning Force (U) [!]", 0 )
GAME( 1900, g_burnfj, g_burf,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Burning Force (J) [!]", 0 )
GAME( 1900, g_burnfe, g_burf,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Burning Force (E)", 0 )

GAME( 1900, g_clif,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cliffhanger (U) [c][!]", 0 )
GAME( 1900, g_clifhe, g_clif,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Cliffhanger (E)", 0 )
GAME( 1900, g_clifhb, g_clif,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cliffhanger (Beta)", 0 )

GAME( 1991, g_dahna,  0,        megadriv,    megadriv,    megadrij, ROT0,   "IGS", "Dahna - Megami Tanjou (J) [!]", 0 )
GAME( 1900, g_hercu,  g_dahna,  megadriv,    megadriv,    megadriv, ROT0,   "Unknown (Unlicensed)", "Hercules (Unl) [!]", 0 )

GAME( 1900, g_bean,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dr. Robotnik's Mean Bean Machine (U) [!]", 0 )
GAME( 1900, g_beane,  g_bean,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Dr. Robotnik's Mean Bean Machine (E) [!]", 0 )
GAME( 1900, g_beanb,  g_bean,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dr. Robotnik's Mean Bean Machine (Beta)", 0 )

GAME( 1900, g_dash,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dashin' Desperadoes (U) [!]", 0 )
GAME( 1900, g_dashb,  g_dash,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dashin' Desperadoes (Beta)", 0 )

GAME( 1900, g_blee,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Dragon - The Bruce Lee Story (U) [!]", 0 )
GAME( 1900, g_drgble, g_blee,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Dragon - The Bruce Lee Story (E)", 0 )

GAME( 1900, g_dduk,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dynamite Duke (W) [!]", 0 )
GAME( 1900, g_ddukea, g_dduk,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dynamite Duke (W) [a1][!]", 0 )

GAME( 1900, g_dhed,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dynamite Headdy (UE) [c][!]", 0 )
GAME( 1900, g_dheadj, g_dhed,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dynamite Headdy (J) [c][!]", 0 )
GAME( 1900, g_dheadb, g_dhed,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dynamite Headdy (Beta)", 0 )

GAME( 1900, g_ejim,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Earthworm Jim (U) [!]", 0 )
GAME( 1900, g_ejime,  g_ejim,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Earthworm Jim (E) [!]", 0 )

GAME( 1900, g_jim2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Earthworm Jim 2 (U) [!]", 0 )
GAME( 1900, g_ejim2e, g_jim2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Earthworm Jim 2 (E) [!]", 0 )

GAME( 1900, g_etch,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Eternal Champions (U) [!]", 0 )
GAME( 1900, g_echmpe, g_etch,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Eternal Champions (E) [!]", 0 )
GAME( 1900, g_echmpj, g_etch,   megadriv,    megadri6,    megadrij, ROT0,   "Unsorted", "Eternal Champions (J) [c][!]", 0 )
GAME( 1900, g_etchmb, g_etch,   megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Eternal Champions (Beta)", 0 )

GAME( 1900, g_exil,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Exile (U) [!]", 0 )
GAME( 1900, g_xzr,    g_exil,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "XZR (J) [!]", 0 )

GAME( 1900, g_exos,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Exo-Squad (U) [!]", 0 )
GAME( 1900, g_exosb,  g_exos,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Exo-Squad (Beta)", 0 )

GAME( 1900, g_fand,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Codemasters", "Fantastic Dizzy (UE) (M5) [c][!]", 0 )
GAME( 1900, g_fanda,  g_fand,   megadriv,    megadriv,    megadriv, ROT0,   "Codemasters", "Fantastic Dizzy (UE) (M5) [a1]", 0 )

GAME( 1900, g_fi95,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Electronic Arts", "FIFA Soccer 95 (UE) [!]", 0 )
GAME( 1900, g_fut98,  g_fi95,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unknown (Unlicensed)", "Futbol Argentino 98 - Pasion de Multitudes (Unl)", 0 )

GAME( 1900, g_fi96,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Electronic Arts", "FIFA Soccer 96 (UE) (M6) [!]", 0 )
GAME( 1900, g_fi99r,  g_fi96,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unknown (Unlicensed)", "FIFA Soccer 99 (R) [!]", 0 )

GAME( 1996, g_fi97,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Electronic Arts", "FIFA Soccer 97 Gold Edition (UE) (M6) [!]", 0 ) // backupram not in header
GAME( 2000, g_f2000g, g_fi97,   megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unknown (Unlicensed)", "FIFA Soccer 2000 Gold Edition (Unl) (M6) [!]", 0 )

GAME( 1900, g_fshk,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fire Shark (U) [!]", 0 )
GAME( 1900, g_fshrke, g_fshk,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Fire Shark (E) [!]", 0 )
GAME( 1900, g_fshrku, g_fshk,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fire Shark (U) [a1][!]", 0 )
GAME( 1900, g_samex3, g_fshk,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Same! Same! Same! (J) [!]", 0 )

GAME( 1900, g_gf2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Galaxy Force II (W) (REV01) [!]", 0 )
GAME( 1900, g_glf200, g_gf2,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Galaxy Force II (W) (REV00)", 0 )

GAME( 1900, g_genc,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "General Chaos (UE) [!]", 0 )
GAME( 1900, g_konsen, g_genc,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dai Konsen (J) [!]", 0 )

GAME( 1900, g_gbus,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ghostbusters (W) (REV01) [!]", 0 )
GAME( 1900, g_gbus00, g_gbus,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ghostbusters (W) (REV00) [!]", 0 )

GAME( 1900, g_gods,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Gods (U) [!]", 0 )
GAME( 1900, g_godse,  g_gods,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Gods (E) [!]", 0 )
GAME( 1900, g_godsj,  g_gods,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Gods (J) [!]", 0 )
GAME( 1900, g_godsb,  g_gods,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Gods (Beta)", 0 )

GAME( 1900, g_gax,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Golden Axe (W) (REV01) [!]", 0 )
GAME( 1900, g_gax00,  g_gax,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Golden Axe (W) (REV00) [!]", 0 )

GAME( 1900, g_gax2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Golden Axe II (W) [!]", 0 )
GAME( 1900, g_gax2b,  g_gax2,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Golden Axe II (Beta)", 0 )

GAME( 1900, g_gax3,   0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Golden Axe III (J) [!]", 0 )

GAME( 1900, g_gran,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Granada (JU) (REV01) [!]", 0 )
GAME( 1900, g_gran00, g_gran,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Granada (JU) (REV00) [!]", 0 )


GAME( 1900, g_grin,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "GRIND Stormer (U) [!]", 0 )
GAME( 1900, g_v5,     g_grin,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "V-Five (J) [!]", 0 )

GAME( 1900, g_hoso,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Head-On Soccer (U) (M5) [!]", 0 )
GAME( 1900, g_fever,  g_hoso,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Fever Pitch Soccer (E) (M5) [!]", 0 )

GAME( 1900, g_herz,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Herzog Zwei (UE) [!]", 0 )
GAME( 1900, g_herzoj, g_herz,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Herzog Zwei (J) [!]", 0 )

GAME( 1995, g_indy,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Factor 5", "Indiana Jones' Greatest Adventure (Prototype)", 0 )

GAME( 1900, g_jpar,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jurassic Park (U) [!]", 0 )
GAME( 1900, g_jparke, g_jpar,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Jurassic Park (E) [!]", 0 )
GAME( 1900, g_jparkj, g_jpar,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Jurassic Park (J) [!]", 0 )
GAME( 1900, g_jparkb, g_jpar,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jurassic Park (Beta)", 0 )

GAME( 1900, g_jltf,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Justice League Task Force (W) [!]", 0 )

GAME( 1900, g_kawa,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Kawasaki Superbike Challenge (UE) [!]", 0 )
GAME( 1900, g_kawab,  g_kawa,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Kawasaki Superbike Challenge (Beta)", 0 )

GAME( 1900, g_ksfh,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Krusty's Super Funhouse (UE) (REV01) [!]", 0 )
GAME( 1900, g_ksfh00, g_ksfh,   megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Krusty's Super Funhouse (UE) (REV00) [!]", 0 )

GAME( 1900, g_lbat,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Last Battle (UE) [!]", 0 )
GAME( 1900, g_hokuto, g_lbat,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Hokuto no Ken (J) [!]", 0 )

GAME( 1900, g_len,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lethal Enforcers (U) [!]", 0 )
GAME( 1900, g_lenfe,  g_len,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Lethal Enforcers (E) [!]", 0 )
GAME( 1900, g_lenfj,  g_len,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Lethal Enforcers (J) [!]", 0 )

GAME( 1900, g_len2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lethal Enforcers II - Gun Fighters (U) [!]", 0 )
GAME( 1900, g_lth2j,  g_len2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Lethal Enforcers II - Gun Fighters (E) [!]", 0 )

GAME( 1900, g_ligh,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Light Crusader (U) [!]", 0 )
GAME( 1900, g_licrue, g_ligh,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Unsorted", "Light Crusader (E) (M4) [!]", 0 )
GAME( 1900, g_licruj, g_ligh,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Light Crusader (J) [!]", 0 )
GAME( 1900, g_licruk, g_ligh,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Light Crusader (K) [!]", 0 )

GAME( 1900, g_lost,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Lost Vikings, The (U) [!]", 0 )
GAME( 1900, g_lostve, g_lost,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Lost Vikings, The (E)", 0 )
GAME( 1900, g_lostvb, g_lost,   megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Lost Vikings, The (Beta)", 0 )

GAME( 1900, g_mmf,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Marko's Magic Football (U) [!]", 0 )
GAME( 1900, g_mmftbe, g_mmf,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Marko's Magic Football (E) (M4) [!]", 0 )
GAME( 1900, g_mmfb,   g_mmf,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Marko's Magic Football (Beta) (M4)", 0 )

GAME( 1900, g_mk,     0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Mortal Kombat (UE) (REV01) [c][!]", 0 )
GAME( 1900, g_mk00,   g_mk,     megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Mortal Kombat (W) (REV00) [!]", 0 )

GAME( 1900, g_mk2,    0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Mortal Kombat II (W) [!]", 0 )

GAME( 1900, g_mk3,    0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Mortal Kombat 3 (U) [!]", 0 )
GAME( 1900, g_mk3e,   g_mk3,    megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Mortal Kombat 3 (E) [!]", 0 )

GAME( 1900, g_umk3,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Ultimate Mortal Kombat 3 (U) [!]", 0 )
GAME( 1900, g_umk3e,  g_umk3,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Ultimate Mortal Kombat 3 (E) [!]", 0 )

GAME( 1900, g_marv,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Marvel Land (U) [!]", 0 )
GAME( 1900, g_marvj,  g_marv,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Marvel Land (J) [!]", 0 )
GAME( 1900, g_talmit, g_marv,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Talmit's Adventure (E) [!]", 0 )

GAME( 1900, g_mjmw,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Michael Jackson's Moonwalker (W) (REV01) [!]", 0 )
GAME( 1900, g_mwlk00, g_mjmw,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Michael Jackson's Moonwalker (W) (REV00) [!]", 0 )

GAME( 1900, g_mono,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Monopoly (U) [c][!]", 0 )
GAME( 1900, g_monob,  g_mono,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Monopoly (Beta)", 0 )

GAME( 1900, g_mush,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "M.U.S.H.A (U) [!]", 0 )
GAME( 1900, g_mushaj, g_mush,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Musha Aleste - Full Metal Fighter Ellinor (J) [!]", 0 )

GAME( 1900, g_mysf,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mystical Fighter (U) [c][!]", 0 )
GAME( 1900, g_maoure, g_mysf,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Maou Renjishi (J) [c][!]", 0 )

GAME( 1900, g_nh94,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000,   ROT0,   "Electronic Arts", "NHL 94 (UE) [!]", 0 )

GAME( 1900, g_ogol,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Olympic Gold - Barcelona 92 (U) (M8) [c][!]", 0 )
GAME( 1900, g_olgole, g_ogol,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Olympic Gold - Barcelona 92 (E) (M8) [c][!]", 0 )
GAME( 1900, g_olgolj, g_ogol,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Olympic Gold - Barcelona 92 (J) (M8) [c][!]", 0 )
GAME( 1900, g_olgolu, g_ogol,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Olympic Gold - Barcelona 92 (U) (M8) [a1][c][!]", 0 )

GAME( 1900, g_otti,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Ottifants, The (E) (M5) [!]", 0 )
GAME( 1900, g_ottifb, g_otti,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Ottifants, The (Beta)", 0 )


GAME( 1900, g_pga,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "PGA Tour Golf (UE) (REV02) [!]", 0 )
GAME( 1900, g_pgat01, g_pga,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "PGA Tour Golf (UE) (REV01) [!]", 0 )

GAME( 1900, g_pga2,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "PGA Tour Golf II (UE) (REV01) [!]", 0 )
GAME( 1900, g_pga2j,  g_pga2,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "PGA Tour Golf II (J) [!]", 0 )
GAME( 1900, g_pga200, g_pga2,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "PGA Tour Golf II (UE) (REV00) [c][!]", 0 )

GAME( 1900, g_pga3,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "PGA Tour Golf III (UE) [!]", 0 )

GAME( 1900, g_pg96,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "PGA Tour 96 (UE) [!]", 0 )

GAME( 1900, g_phel,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Phelios (U) [c][!]", 0 )
GAME( 1900, g_phelie, g_phel,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Phelios (E) [c][!]", 0 )
GAME( 1900, g_phelij, g_phel,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Phelios (J) [c][!]", 0 )

GAME( 1900, g_pirg,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pirates! Gold (U) [!]", 0 )
GAME( 1900, g_pgoldb, g_pirg,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pirates! Gold (Beta)", 0 )

GAME( 1900, g_popu,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Populous (U) [!]", 0 )
GAME( 1900, g_popue,  g_popu,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Populous (E) [!]", 0 )
GAME( 1900, g_popuj,  g_popu,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Populous (J) [!]", 0 )

GAME( 1900, g_pmon,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Power Monger (UE) [!]", 0 )
GAME( 1900, g_pmonj,  g_pmon,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Power Monger (J) [!]", 0 )

GAME( 1900, g_ppin,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Psycho Pinball (E) (Oct 1994) (M5) [c][!]", 0 )
GAME( 1900, g_ppina,  g_ppin,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Psycho Pinball (E) (Sep 1994) (M5) [c][!]", 0 )

GAME( 1900, g_pugg,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Puggsy (U) [!]", 0 )
GAME( 1900, g_puggse, g_pugg,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Puggsy (E) [!]", 0 )
GAME( 1900, g_puggsb, g_pugg,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Puggsy (Beta)", 0 )

GAME( 1900, g_puni,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Punisher, The (U) [!]", 0 )
GAME( 1900, g_punise, g_puni,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Punisher, The (E)", 0 )

GAME( 1900, g_puy2,   0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Puyo Puyo Tsu (J) (REV01) [!]", 0 )
GAME( 1900, g_puy200, g_puy2,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Puyo Puyo Tsu (J) (REV00) [!]", 0 )

GAME( 1900, g_rist,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ristar (UE) [!]", 0 )
GAME( 1900, g_rist00, g_rist,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ristar (UE) (REV00)", 0 )
GAME( 1900, g_ristj,  g_rist,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Ristar - The Shooting Star (J) [!]", 0 )

GAME( 1900, g_r3k2,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Koei", "Romance of the Three Kingdoms II (U) [!]", 0 )
GAME( 1900, g_sango2, g_r3k2,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Koei", "Sangokushi II (J) [!]", 0 )

GAME( 1900, g_r3k3,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Koei", "Romance of the Three Kingdoms III (U) [!]", 0 )
GAME( 1900, g_sango3, g_r3k3,   megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Koei", "Sangokushi III (J) [!]", 0 )

GAME( 1900, g_sagia,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sagaia (U) [c][!]", 0 )
GAME( 1900, g_dar2,   g_sagia,  megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Darius II (J) [c][!]", 0 )
GAME( 1900, g_dari2a, g_sagia,  megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Darius II (J) [a1]", 0 )

GAME( 1900, g_sswo,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Saint Sword (U) [c][!]", 0 )
GAME( 1900, g_sswoj,  g_sswo,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Saint Sword (J) [c][!]", 0 )

// doesn't need 6 buttons, but they can be used..
GAME( 1900, g_snsm,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Saturday Night Slam Masters (U) [!]", 0 )
GAME( 1900, g_snsme,  g_snsm,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Saturday Night Slam Masters (E)", 0 )

GAME( 1900, g_shaq,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Shaq Fu (UE) [!]", 0 )

GAME( 1900, g_sbls,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Shadow Blasters (U) [c][!]", 0 )
GAME( 1900, g_shiten, g_sbls,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Shiten Myooh (J) [c][!]", 0 )

GAME( 1900, g_soldf,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Soldiers of Fortune (U) [c][!]", 0 )
GAME( 1900, g_chao,   g_soldf,  megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Chaos Engine, The (E) [!]", 0 )
GAME( 1900, g_cengb,  g_soldf,  megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chaos Engine, The (Beta)", 0 )

GAME( 1900, g_spl2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Namco", "Splatterhouse 2 (U) [c][!]", 0 )
GAME( 1900, g_shou2e, g_spl2,   megadpal,    megadriv,    megadrie, ROT0,   "Namco", "Splatterhouse 2 (E) [c][!]", 0 )

GAME( 1900, g_spl3,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Namco", "Splatterhouse 3 (U) [c][!]", 0 )
GAME( 1900, g_shou3j, g_spl3,   megadriv,    megadriv,    megadrij, ROT0,   "Namco", "Splatterhouse Part 3 (J) [c][!]", 0 )
GAME( 1900, g_sho3ja, g_spl3,   megadriv,    megadriv,    megadrij, ROT0,   "Namco", "Splatterhouse Part 3 (J) [a1]", 0 )

GAME( 1900, g_sf2c,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Street Fighter II' - Special Champion Edition (U) [!]", 0 )
GAME( 1900, g_sf2e,   g_sf2c,   megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Street Fighter II' - Special Champion Edition (E) [!]", 0 )
GAME( 1900, g_sf2j,   g_sf2c,   megadriv,    megadri6,    megadrij, ROT0,   "Unsorted", "Street Fighter II' Plus - Champion Edition (J) [!]", 0 )
GAME( 1900, g_sf2b,   g_sf2c,   megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Street Fighter II' Turbo (Beta)", 0 )

GAME( 1900, g_stri,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Strider (UE) [!]", 0 )
GAME( 1900, g_stridj, g_stri,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Strider Hiryuu (J) [!]", 0 )

GAME( 1900, g_supm,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Superman (U) [!]", 0 )
GAME( 1900, g_supmne, g_supm,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Superman (E) [!]", 0 )
GAME( 1900, g_supmnb, g_supm,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Superman (Beta)", 0 )

GAME( 1900, g_shyd,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Super Hydlide (U) [!]", 0 )
GAME( 1900, g_shyde,  g_shyd,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Unsorted", "Super Hydlide (E) [!]", 0 )
GAME( 1900, g_shydj,  g_shyd,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Super Hydlide (J) [c][!]", 0 )

GAME( 1900, g_sydval, 0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Syd of Valis (U) [!]", 0 )
GAME( 1900, g_valsdj, g_sydval, megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Valis SD (J) [!]", 0 )

GAME( 1900, g_sylv,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sylvester & Tweety in Cagey Capers (UE) [!]", 0 )
GAME( 1900, g_sylvb,  g_sylv,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sylvester & Tweety in Cagey Capers (Beta)", 0 )

GAME( 1900, g_targ,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Target Earth (U) [!]", 0 )
GAME( 1900, g_asl,    g_targ,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Assault Suits Leynos (J) [!]", 0 )

GAME( 1900, g_turt,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Teenage Mutant Ninja Turtles - The Hyperstone Heist (U) [!]", 0 )
GAME( 1900, g_tmntj,  g_turt,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Teenage Mutant Ninja Turtles - Return of the Shredder (J) [!]", 0 )
GAME( 1900, g_tmhte,  g_turt,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Teenage Mutant Hero Turtles - The Hyperstone Heist (E)", 0 )

GAME( 1900, g_tutf,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Teenage Mutant Ninja Turtles - Tournament Fighters (U) [c][!]", 0 )
GAME( 1900, g_tmnttj, g_tutf,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Teenage Mutant Ninja Turtles - Tournament Fighters (J) [!]", 0 )
GAME( 1900, g_tmhtte, g_tutf,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Teenage Mutant Hero Turtles - Tournament Fighters (E)", 0 )

GAME( 1900, g_tta,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tiny Toon Adventures - Buster's Hidden Treasure (U) [!]", 0 )
GAME( 1900, g_ttabe,  g_tta,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Tiny Toon Adventures - Buster's Hidden Treasure (E) [!]", 0 )

GAME( 1900, g_ttaa,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tiny Toon Adventures - Acme All-Stars (U) [!]", 0 )
GAME( 1900, g_ttadae, g_ttaa,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Tiny Toon Adventures - Acme All-Stars (E)", 0 )

GAME( 1900, g_toki,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Toki - Going Ape Spit (UE) [c][!]", 0 )
GAME( 1900, g_juju,   g_toki,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "JuJu Densetsu (J) [!]", 0 )

GAME( 1900, g_toy,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Toy Story (U) [!]", 0 )
GAME( 1900, g_toyste, g_toy,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Toy Story (E) [!]", 0 )

GAME( 1900, g_traysi, 0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Traysia (U) [!]", 0 )
GAME( 1900, g_minato, g_traysi, megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Minato no Toreijia (J) [!]", 0 )

GAME( 1900, g_2cd,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Two Crude Dudes (U) [!]", 0 )
GAME( 1900, g_crudee, g_2cd,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Two Crude Dudes (E)", 0 )
GAME( 1900, g_crudeb, g_2cd,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Crude Buster (J) [!]", 0 )

GAME( 1900, g_uqix,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Taito", "Ultimate Qix (U) [!]", 0 )
GAME( 1900, g_volf,   g_uqix,   megadriv,    megadriv,    megadrij, ROT0,   "Taito", "Volfied (J) [!]", 0 )

GAME( 1900, g_vf2,    0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Virtua Fighter 2 (UE) [!]", 0 )

// should this be 6-button? it doesn't detect it correctly..
GAME( 1900, g_vrtr,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "VR Troopers (UE) [!]", 0 )

GAME( 1900, g_weap,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Weaponlord (U) [!]", 0 )

GAME( 1900, g_wwcse,  0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Where in the World is Carmen Sandiego (E) (M5)", 0 )
GAME( 1900, g_wwics,  g_wwcse,  megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Where in the World is Carmen Sandiego (B) (M2) [!]", 0 )

GAME( 1900, g_wtics,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Where in Time is Carmen Sandiego (B) [!]", 0 )
GAME( 1900, g_wticsa, g_wtics,  megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Where in Time is Carmen Sandiego (UE) (M5) [!]", 0 )

GAME( 1900, g_wars,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Warsong (U) [!]", 0 )
GAME( 1900, g_langr,  g_wars,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Langrisser (J) [!]", 0 )

GAME( 1900, g_lngr2a, 0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Langrisser II (J) (REV00) [!]", 0 )
GAME( 1900, g_lngr2c, g_lngr2a, megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Langrisser II (J) (REV02) [!]", 0 )
GAME( 1900, g_lng201, g_lngr2a, megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Langrisser II (J) (REV01)", 0 )

GAME( 1900, g_wimb,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wimbledon Championship Tennis (U) [!]", 0 )
GAME( 1900, g_wimbp,  g_wimb,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wimbledon Championship Tennis (U) (Prototype) [!]", 0 )
GAME( 1900, g_wimbe,  g_wimb,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Wimbledon Championship Tennis (E) [!]", 0 )

GAME( 1900, g_wwar,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wrestle War (JE) [c][!]", 0 )
GAME( 1900, g_wwarb,  g_wwar,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wrestle War (Beta)", 0 )

GAME( 1900, g_wmar,   0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "WWF Wrestlemania Arcade (U) [!]", 0 )
GAME( 1900, g_wfwaal, g_wmar,   megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "WWF Wrestlemania Arcade (Alpha) [!]", 0 )

GAME( 1900, g_xmen,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "X-Men (U) [!]", 0 )
GAME( 1900, g_xmene,  g_xmen,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "X-Men (E) [!]", 0 )

GAME( 1900, g_xme2,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "X-Men 2 - Clone Wars (UE) [!]", 0 )

GAME( 1900, g_yuyu,   0,        megadriv,    megadri6,    megadrij, ROT0,   "Unsorted", "Yuu Yuu Hakusho - Makyou Toitsusen (J) [!]", 0 )
GAME( 1900, g_yuyub,  g_yuyu,   megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Yuu Yuu Hakusho - Sunset Fighters (B) [!]", 0 )

GAME( 1900, g_ys3,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Ys III - Wanderers from Ys (U) [!]", 0 )
GAME( 1900, g_ys3j,   g_ys3,    megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Ys III - Wanderers from Ys (J) [!]", 0 )

GAME( 1900, g_zany,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zany Golf (UE) (REV00) [c][!]", 0 )
GAME( 1900, g_zany01, g_zany,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zany Golf (UE) (REV01))", 0 )

GAME( 1900, g_zwin,   0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Zero Wing (E) [c][!]", 0 )
GAME( 1900, g_zwingj, g_zwin,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Zero Wing (J)", 0 )

GAME( 1900, g_zomb,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zombies Ate My Neighbors (U) [c][!]", 0 )
GAME( 1900, g_zombe,  g_zomb,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Zombies (E) [!]", 0 )

GAME( 1900, g_zool,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zool (U) [!]", 0 )
GAME( 1900, g_zoole,  g_zool,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Zool (E) [!]", 0 )



/********************************* Unsorted Koei ****************************************/
GAME( 199?, g_uw,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Koei", "Uncharted Waters (U) [!]", 0 )
GAME( 1900, g_daikou, g_uw,     megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Koei", "Daikoukai Jidai (J) [c][!]", 0 )

GAME( 1900, g_uwnh,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Koei", "Uncharted Waters - New Horizons (U) [!]", 0 )
GAME( 1900, g_daik2,  g_uwnh,   megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Koei", "Daikoukai Jidai II (J)", 0 )

GAME( 1900, g_gemfi,  0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Koei", "GemFire (U) [!]", 0 )
GAME( 1900, g_roybld, g_gemfi,  megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Koei", "Royal Blood (J) [!]", 0 )

GAME( 1900, g_nobu,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Koei", "Nobunaga's Ambition (U) [!]", 0 )
GAME( 1900, g_nobzen, g_nobu,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Koei", "Nobunaga no Yabou - Zenkokuban (J) [!]", 0 )

GAME( 1900, g_pto,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Koei", "P.T.O. Pacific Theater of Operations (U) [!]", 0 )
GAME( 1900, g_teitok, g_pto,    megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Koei", "Teitoku no Ketsudan (J) [c][!]", 0 )


GAME( 1900, g_opeu,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Koei", "Operation Europe - Path to Victory 1939-1945 (U) [!]", 0 )
GAME( 1900, g_europa, g_opeu,   megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Koei", "Europa Sensen (J) [!]", 0 )

// english
GAME( 1993, g_librty, 0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Koei", "Liberty or Death (U) [!]", 0 )
GAME( 1900, g_geng,   0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Koei", "Genghis Khan II - Clan of the Gray Wolf (U) [!]", 0 )

// japanese
GAME( 1900, g_nobbus, 0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Koei", "Nobunaga no Yabou - Bushou Fuuunsoku (J) [!]", 0 )
GAME( 1900, g_noblor, 0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Koei", "Nobunaga no Yabou Haouden - Lord of Darkness (J) [!]", 0 )
GAME( 1900, g_taikou, 0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Koei", "Taikou Risshiden (J) [!]", 0 )
GAME( 1900, g_aoki,   0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Koei", "Aoki Ookami to Shiroki Meshika - Genchou Hishi (J) [!]", 0 )



/********************************* Not At All Sorted ****************************************/

GAME( 1900, g_gley,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Gley Lancer (J)", 0 )
GAME( 1900, g_srr,     0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Snake Rattle 'n' Roll (E) [c][!]", 0 )
GAME( 1900, g_tazm,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Taz-Mania (W) [!]", 0 )
GAME( 1900, g_coss,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cosmic Spacehead (E) (M4) [c][!]", 0 )
GAME( 1900, g_paca,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pac-Attack (U) [!]", 0 )

GAME( 1900, g_jbok,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jungle Book, The (U) [!]", 0 )
GAME( 1900, g_jbooke,  g_jbok,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Jungle Book, The (E) [!]", 0 )

GAME( 1900, g_fant,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fantasia (JU) (REV01) [!]", 0 )
GAME( 1900, g_fante,   g_fant,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Fantasia (E) [!]", 0 )
GAME( 1900, g_fant00,  g_fant,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fantasia (U) (REV00) [!]", 0 )


GAME( 1900, g_dcap,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Decap Attack (UE) [!]", 0 )
GAME( 1998, g_jpa2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jurassic Park 2 - The Lost World (U) [!]", 0 )
GAME( 1900, g_jimp,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jim Power - The Arcade Game (Beta)", 0 )
GAME( 1900, g_wbug,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Charles' Window Bug Example", 0 )
GAME( 1900, g_raid,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Raiden Trad (JU) [!]", 0 )


GAME( 1900, g_f98,     0,        megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Unsorted", "FIFA Soccer 98 - Road to the World Cup (E) (M5) [!]", 0 )

GAME( 1900, g_maui,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Donald in Maui Mallard (E) [!]", 0 )

GAME( 1900, g_sgp2,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Ayrton Senna's Super Monaco GP II (U) [!]", 0 )
GAME( 1900, g_smgp2a,  g_sgp2,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Unsorted", "Ayrton Senna's Super Monaco GP II (JE) [!]", 0 )

GAME( 1900, g_batf,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Batman Forever (W) [!]", 0 )

GAME( 1900, g_gtwi,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Gadget Twins, The (U) [!]", 0 )
GAME( 1900, g_lgal,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Legend of Galahad, The (UE) [!]", 0 )
GAME( 1900, g_lion,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lion King, The (W) [!]", 0 )


GAME( 1900, g_pano,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Panorama Cotton (J) [c][!]", 0 )
GAME( 1900, g_puls,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Pulseman (J) [c][!]", 0 )
GAME( 1900, g_redz,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Red Zone (UE) [!]", 0 )
GAME( 1900, g_skit,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Skitchin' (UE) [!]", 0 )

GAME( 1900, g_sfze,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Super Fantasy Zone (E) [!]", 0 )
GAME( 1900, g_sfz,     g_sfze,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Super Fantasy Zone (J) [!]", 0 )

GAME( 1900, g_sho,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Hang-On (W) (REV01) [!]", 0 )
GAME( 1900, g_sho00,   g_sho,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Hang-On (W) (REV00) [!]", 0 )


GAME( 1900, g_stb,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Thunder Blade (W) (REV01) [!]", 0 )
GAME( 1900, g_stb00,   g_stb,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Thunder Blade (W) (REV00) [!]", 0 )



GAME( 1900, g_pino,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pinocchio (U) [!]", 0 )
GAME( 1900, g_pinnoe,  g_pino,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Pinocchio (E)", 0 )

GAME( 1900, g_puyo,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Puyo Puyo (J) [!]", 0 )

GAME( 1900, g_seaq,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "SeaQuest DSV (U) [!]", 0 )
GAME( 1900, g_seaqe,   g_seaq,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "SeaQuest DSV (E)", 0 )


GAME( 1900, g_tale,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tale Spin (UE) [!]", 0 )

GAME( 1900, g_balz,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ballz (UE) [!]", 0 )
GAME( 1900, g_bonk,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bonkers (UE) [!]", 0 )

GAME( 1900, g_mars,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Marsupilami (U) (M5) [!]", 0 )
GAME( 1900, g_marsue,  g_mars,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Marsupilami (E) (M5) [!]", 0 )

GAME( 1900, g_fdma,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Flavio's DMA Test (PD)", 0 )

GAME( 1900, g_3nin,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "3 Ninjas Kick Back (U) [!]", 0 )
GAME( 1900, g_haun,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Haunting Starring Polterguy (UE) [!]", 0 )
GAME( 1900, g_mfli,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Misadventures of Flink, The (E) [!]", 0 )

GAME( 1900, g_rwoo,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Risky Woods (UE) [!]", 0 )
GAME( 1900, g_draxos,  g_rwoo,   megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts", "Draxos (J) [!]", 0 )

GAME( 1900, g_lot,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lotus Turbo Challenge (UE) [!]", 0 )

GAME( 1900, g_lot2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lotus II RECS (UE) [!]", 0 )
GAME( 1900, g_lotu2b,  g_lot2,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lotus II RECS (Beta)", 0 )


GAME( 1900, g_stra,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Street Racer (E) [c][!]", 0 )
GAME( 1900, g_tinh,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tinhead (U) [!]", 0 )
GAME( 1900, g_tg2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Top Gear 2 (U) [!]", 0 )
GAME( 1900, g_gdog,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Greendog - The Beached Surfer Dude (UE) [!]", 0 )
GAME( 1900, g_gaia,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Gaiares (JU) [!]", 0 )

GAME( 1900, g_elem,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Elemental Master (U) [!]", 0 )
GAME( 1900, g_elemj,   g_elem,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Elemental Master (J) [!]", 0 )


GAME( 1900, g_mano,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Man Overboard! - S.S. Lucifer (E) [c][!]", 0 )

GAME( 1900, g_daze,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Daze Before Christmas, The (E) [!]", 0 )
GAME( 1900, g_dazeb,   g_daze,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Daze Before Christmas, The (Beta)", 0 )


GAME( 1900, g_jwws,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Jimmy White's Whirlwind Snooker (E) [c][!]", 0 )


GAME( 1900, g_busq,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bubble and Squeak (U) [!].zip", 0 )

GAME( 1900, g_olan,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Outlander (U) [!]", 0 )
GAME( 1900, g_outlnb,  g_olan,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Outlander (Beta)", 0 )


GAME( 1900, g_pidw,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pirates of Dark Water, The (U) [!]", 0 )
GAME( 1900, g_pirdwu,  g_pidw,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pirates of Dark Water, The (UE)", 0 )

GAME( 1900, g_bbrb,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Beauty and the Beast - Roar of the Beast (U) [!]", 0 )
GAME( 1900, g_bbbq,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Beauty and the Beast - Belle's Quest (U) [!]", 0 )
GAME( 1900, g_tanr,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Tanto R (J) [!]", 0 )
GAME( 1900, g_rop,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rings of Power (UE) [!]", 0 )
GAME( 1900, g_ps96,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Pete Sampras Tennis 96 (E) [c][!]", 0 )
GAME( 1900, g_toug,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Toughman Contest (UE) [!]", 0 )
GAME( 1900, g_spir,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Spirou (E) [!]", 0 )

GAME( 1900, g_strk,    0,        megadriv,    megadriv,    megadrie_backup_0x200000_0x10000, ROT0,   "Unsorted", "Striker (E) (M5) [!]", 0 )
GAME( 1900, g_strikb,  g_strk,   megadriv,    megadriv,    megadrie_backup_0x200000_0x10000, ROT0,   "Unsorted", "Striker (Beta)", 0 )






GAME( 1900, g_chee,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Cheese Cat-Astrophe Starring Speedy Gonzales (E) (M4) [!]", 0 )


GAME( 1900, g_td2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Test Drive II - The Duel (U) [!]", 0 )
GAME( 1900, g_tout,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Turbo Outrun (JE) [c][!]", 0 )
GAME( 1900, g_garf,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Garfield - Caught in the Act (UE) [!]", 0 )
GAME( 1900, g_drev,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dragon's Revenge (UE) [!]", 0 )

GAME( 1900, g_afav,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Addams Family Values (E) (M3) [!]", 0 )
GAME( 1900, g_arie,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ariel - The Little Mermaid (UE) [!]", 0 )

GAME( 1900, g_arro,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Arrow Flash (UE) [!]", 0 )
GAME( 1900, g_arrowj,  g_arro,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Arrow Flash (J) [!]", 0 )

GAME( 1900, g_blma,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Blaster Master 2 (U) [c][!]", 0 )
GAME( 1900, g_blam2b,  g_blma,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Blaster Master 2 (Beta)", 0 )

GAME( 1900, g_bubs,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bubsy in Claws Encounters of the Furred Kind (UE) [!]", 0 )
GAME( 1900, g_bub2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bubsy II (UE) [!]", 0 )
GAME( 1900, g_bbny,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bugs Bunny in Double Trouble (U) [!]", 0 )
GAME( 1900, g_cano,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Cannon Fodder (E) [!]", 0 )

GAME( 1900, g_cpoo,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Championship Pool (U) [!]", 0 )

GAME( 1900, g_chq2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chase HQ II (U) [!]", 0 )
GAME( 1900, g_suphq,   g_chq2,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Super HQ (J) [!]", 0 )

GAME( 1900, g_cool,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cool Spot (U) [!]", 0 )
GAME( 1900, g_coole,   g_cool,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Cool Spot (E) [!]", 0 )
GAME( 1900, g_spotb,   g_cool,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cool Spot (Beta)", 0 )


GAME( 1900, g_dicv,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dick Vitale's Awesome Baby! College Hoops (U) [!]", 0 )

GAME( 1900, g_taz2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Escape From Mars Starring Taz (U) [!]", 0 )
GAME( 1900, g_taz2e,   g_taz2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Escape From Mars Starring Taz (E) [!]", 0 )


GAME( 1900, g_huni,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Heavy Unit (J) [!]", 0 )
GAME( 1900, g_hook,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Hook (U) [c][!]", 0 )
GAME( 1900, g_huma,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Humans, The (U) [!]", 0 )
GAME( 1900, g_jpra,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jurassic Park - Rampage Edition (UE) [!]", 0 )
GAME( 1900, g_mhat,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Magical Hat no Buttobi Turbo! Daibouken (J) [!]", 0 )
GAME( 1900, g_nutz,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Mr. Nutz (E) [!]", 0 )

GAME( 1900, g_ooze,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ooze, The (JU) [!]", 0 )
GAME( 1900, g_oozee,   g_ooze,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Ooze, The (E) [!]", 0 )


GAME( 1900, g_2040,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Phantom 2040 (U) [!]", 0 )
GAME( 1900, g_2040e,   g_2040,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Phantom 2040 (E) [!]", 0 )

GAME( 1900, g_krew,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Skeleton Krew (U) [!]", 0 )
GAME( 1900, g_skrewe,  g_krew,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Skeleton Krew (E)", 0 )

GAME( 1900, g_skik,    0,        megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Unsorted", "Super Kick Off (E) [c][!]", 0 )


GAME( 1900, g_term,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Terminator, The (U) [!]", 0 )
GAME( 1900, g_terme,   g_term,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Terminator, The (E) [c][!]", 0 )

GAME( 1900, g_wolv,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wolverine Adamantium Rage (UE) [!]", 0 )

GAME( 1900, g_real,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "AAAHH!!! Real Monsters (UE) [!]", 0 )
GAME( 1900, g_suj2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Barkley Shut Up and Jam! 2 (U) [!]", 0 )
GAME( 1900, g_batr,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Batman Returns (W) [!]", 0 )
GAME( 1900, g_btoa,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Battletoads (W) [!]", 0 )
GAME( 1900, g_che2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chester Cheetah 2 - Wild Wild Quest (U) [!]", 0 )
GAME( 1900, g_che,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chester Cheetah (U) [c][!]", 0 )

GAME( 1900, g_clay,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Clay Fighter (U) [!]", 0 )

GAME( 1900, g_daff,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Daffy Duck in Hollywood (E) (M5) [!]", 0 )
GAME( 1900, g_daffyb,  g_daff,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Daffy Duck in Hollywood (Beta)", 0 )

GAME( 1900, g_davi,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Davis Cup World Tour Tennis (UE) [!]", 0 )
GAME( 1900, g_desd,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Desert Demolition (UE) [!]", 0 )
GAME( 1900, g_dstr,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Desert Strike - Return to the Gulf (UE) [!]", 0 )
GAME( 1900, g_dtro,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Doom Troopers - The Mutant Chronicles (U) [c][!]", 0 )
GAME( 1900, g_e_sw,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "ESPN SpeedWorld (U) [c][!]", 0 )


GAME( 1900, g_f1ce,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "F1 World Championship Edition (E) [!]", 0 )
GAME( 1900, g_f1wceb,  g_f1ce,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "F1 World Championship Edition (Beta)", 0 )

GAME( 1900, g_flic,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Flicky (UE) [!]", 0 )
GAME( 1900, g_hurr,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Hurricanes (E) [!]", 0 )
GAME( 1900, g_izzy,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Izzy's Quest for the Olympic Rings (UE) [!]", 0 )

GAME( 1900, g_jstr,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jungle Strike (UE) [!]", 0 )
GAME( 1900, g_jstrj,   g_jstr,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Jungle Strike (J) [!]", 0 )
GAME( 1900, g_jstrkb,  g_jstr,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jungle Strike (Beta)", 0 )

GAME( 1900, g_lawn,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lawnmower Man, The (UE) [!]", 0 )

GAME( 1900, g_lem2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Lemmings 2 - The Tribes (U) [!]", 0 )
GAME( 1900, g_lem2e,   g_lem2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Lemmings 2 - The Tribes (E) [!]", 0 )

GAME( 1900, g_megp,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Megapanel (J) [c][!]", 0 )

GAME( 1900, g_mmpr,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mighty Morphin Power Rangers (U) [!]", 0 )
GAME( 1900, g_mmpre,   g_mmpr,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Mighty Morphin Power Rangers (E) [!]", 0 )

GAME( 1900, g_mmpm,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mighty Morphin Power Rangers - The Movie (U) [!]", 0 )
GAME( 1900, g_mmprme,  g_mmpm,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Mighty Morphin Power Rangers - The Movie (E) [!]", 0 )

GAME( 1900, g_nh98,    0,        megadriv,    megadriv,    g_nh98, ROT0,   "Unsorted", "NHL 98 (U) [c][!]", 0 )



GAME( 1900, g_s_sa,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Spider-Man and Venom - Separation Anxiety (U) [!]", 0 )

GAME( 1900, g_tick,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tick, The (U) [!]", 0 )

GAME( 1900, g_uded,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Undead Line (J) [!]", 0 )
GAME( 1900, g_ustr,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Urban Strike (UE) [!]", 0 )
GAME( 1900, g_muth,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mutant League Hockey (UE) [!]", 0 )


GAME( 1900, g_btdd,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Battletoads and Double Dragon (U) [c][!]", 0 )
GAME( 1900, g_chak,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chakan - The Forever Man (UE) [!]", 0 )
GAME( 1900, g_cutt,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cutthroat Island (UE) [!]", 0 )
GAME( 1900, g_dang,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dangerous Seed (J) [!]", 0 )
GAME( 1900, g_blav,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Blades of Vengeance (UE) [!]", 0 )


GAME( 1900, g_duel,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Death Duel (U) [!]", 0 )
GAME( 1900, g_demo,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Demolition Man (UE) [!]", 0 )
GAME( 1900, g_dick,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dick Tracy (W) [c][!]", 0 )
GAME( 1900, g_dora,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Doraemon - Yume Dorobouto 7 Nin No Gozansu (J) [!]", 0 )
GAME( 1900, g_dn3d,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Duke Nukem 3D (B) [!]", 0 )

GAME( 1900, g_elvi,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "El Viento (U) [!]", 0 )
GAME( 1900, g_elvinj,  g_elvi,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "El Viento (J) [!]", 0 )

GAME( 1900, g_fzon,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Final Zone (JU) [!]", 0 )

GAME( 1900, g_glos,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Generations Lost (UE) [!]", 0 )

GAME( 1900, g_home,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Home Alone (U) [!]", 0 )
GAME( 1900, g_homeab,  g_home,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Home Alone (Beta)", 0 )

GAME( 1900, g_hom2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Home Alone 2 - Lost in New York (U) [!]", 0 )

GAME( 1900, g_immo,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Immortal, The (UE) [!]", 0 )
GAME( 1900, g_immorj,  g_immo,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Immortal, The (J) [!]", 0 )

GAME( 1900, g_hulk,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Incredible Hulk, The (UE) [!]", 0 )
GAME( 1900, g_junc,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Junction (JU) [!]", 0 )

GAME( 1900, g_icd,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Incredible Crash Dummies, The (UE) [c][!]", 0 )
GAME( 1900, g_icdb,    g_icd,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Incredible Crash Dummies, The (Beta)", 0 )

GAME( 1900, g_fran,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mary Shelley's Frankenstein (U) [!]", 0 )
GAME( 1900, g_onsl,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Onslaught (U) [c][!]", 0 )
GAME( 1900, g_peng,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Pengo (J) [!]", 0 )
GAME( 1900, g_rain,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Rainbow Islands - The Story of Bubble Bobble 2 (J) [c][!]", 0 )

GAME( 1900, g_btnm,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Simpsons, The - Bart's Nightmare (UE) (REV02) [!]", 0 )
GAME( 1900, g_smf,     0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Smurfs, The (E) (M5) [!]", 0 )
GAME( 1900, g_smf2,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Smurfs 2, The (E) (M4) [!]", 0 )
GAME( 1900, g_s_mc,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Spider-Man and Venom - Maximum Carnage (W) [!]", 0 )

GAME( 1900, g_sgat,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Stargate (UE) [!]", 0 )
GAME( 1900, g_sgatb,   g_sgat,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Stargate (Beta)", 0 )


GAME( 1900, g_tfh,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Task Force Harrier EX (U) [!]", 0 )
GAME( 1900, g_tfhxj,   g_tfh,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Task Force Harrier EX (J) [!]", 0 )

GAME( 1900, g_tf3,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Thunder Force III (JU) [!]", 0 )


GAME( 1900, g_tter,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Trampoline Terror! (U) [!]", 0 )
GAME( 1900, g_unis,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Universal Soldier (U) [!]", 0 )


GAME( 1900, g_wayn,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wayne's World (U) [c][!]", 0 )
GAME( 1900, g_yogi,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Yogi Bear's Cartoon Capers (E) [!]", 0 )

GAME( 1900, g_soff,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Off Road (U) [!]", 0 )
GAME( 1900, g_ddr,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Double Dragon (U) [!]", 0 )
GAME( 1900, g_ddr2,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Double Dragon 2 - The Revenge (J) [!]", 0 )
GAME( 1900, g_ddr3,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Double Dragon 3 - The Rosetta Stone (UE) [!]", 0 )
GAME( 1900, g_ddrv,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Double Dragon V - The Shadow Falls (U) [!]", 0 )

GAME( 1900, g_fido,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fido Dido (Prototype)", 0 )

GAME( 1900, g_tnnb,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "TNN Bass Tournament of Champions (U) (REV01) [!]", 0 )
GAME( 1900, g_tnno,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "TNN Outdoors Bass Tournament '96 (U) [!]", 0 )
GAME( 1900, g_sscc,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sesame Street Counting Cafe (U) [!]", 0 )

GAME( 1900, g_batj,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Batman - Revenge of the Joker (U) [!]", 0 )
GAME( 1900, g_bsqu,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Battle Squadron (UE) [!]", 0 )
GAME( 1900, g_earn,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Earnest Evans (U) [!]", 0 )

GAME( 1900, g_f117,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "F-117 Night Storm (UE) [!]", 0 )
GAME( 1900, g_f117j,   g_f117,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "F-117 Stealth - Operation Night Storm (J) [!]", 0 )


GAME( 1900, g_frog,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Frogger (U) [!]", 0 )

GAME( 1900, g_gloc,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "G-LOC Air Battle (W) [c][!]", 0 )
GAME( 1900, g_glocb,   g_gloc,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "G-LOC Air Battle (Beta)", 0 )

GAME( 1900, g_gshi,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Gunship (E) [c][!]", 0 )

GAME( 1900, g_last,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Last Action Hero (U) [!]", 0 )

GAME( 1900, g_mpac,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ms. Pac-Man (U) [!]", 0 )

GAME( 1900, g_ncir,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Nightmare Circus (B) [!]", 0 )
GAME( 1900, g_ncircb,  g_ncir,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Nightmare Circus (Beta)", 0 )


GAME( 1900, g_pacm,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pac-Mania (U) [!]", 0 )

GAME( 1900, g_pdri,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Power Drive (E) (M5) [!]", 0 )
GAME( 1900, g_sbe2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Shadow of the Beast 2 (UE) [!]", 0 )


GAME( 1900, g_si91,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Space Invaders 91 (U) [c][!]", 0 )
GAME( 1900, g_si90,    g_si91,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Space Invaders 90 (J) [c][!]", 0 )

GAME( 1900, g_s_ar,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Spider-Man and X-Men - Arcade's Revenge (UE) [!]", 0 )
GAME( 1900, g_sstv,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Smash TV (UE) [!]", 0 )

GAME( 1900, g_sold,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sol-Deace (U) [!]", 0 )

GAME( 1900, g_ter2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Terminator 2 - Judgment Day (UE) [!]", 0 )

GAME( 1900, g_tf2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Thunder Force II (U) [!]", 0 )
GAME( 1900, g_tf2md,   g_tf2,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Thunder Force II MD (J) [!]", 0 )


GAME( 1900, g_toys,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Toys (U) [!]", 0 )
GAME( 1900, g_true,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "True Lies (W) [!]", 0 )

GAME( 1900, g_view,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Viewpoint (U) [!]", 0 )
GAME( 1900, g_viewpb,  g_view,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Viewpoint (Beta)", 0 )



GAME( 1900, g_worm,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Worms (E) [!]", 0 )
GAME( 1900, g_wormp,   g_worm,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Worms (E) (Prototype) [c][!]", 0 )


GAME( 1900, g_xen2,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Xenon 2 - Megablast (E) [c][!]", 0 )

GAME( 1900, g_zoop,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zoop (U) [!]", 0 )
GAME( 1900, g_zoope,   g_zoop,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Zoop (E) [!]", 0 )

GAME( 1900, g_arta,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Art Alive! (W) [!]", 0 )
GAME( 1900, g_arca,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Arcade Classics (U) [!]", 0 )
GAME( 1900, g_aahh,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Aah! Harimanada (J) [!]", 0 ) // sumo wrestling
GAME( 1900, g_awsp,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "AWS Pro Moves Soccer (U) [!]", 0 )


GAME( 1900, g_suj,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Barkley Shut Up and Jam! (UE) [!]", 0 )
GAME( 1900, g_sail,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Bishoujo Senshi Sailor Moon (J) [!]", 0 )

GAME( 1900, g_bl96,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Brian Lara Cricket 96 (E) [c][!]", 0 )
GAME( 1900, g_blc96x,  g_bl96,   megadriv,    megadriv,    megadrie, ROT0,   "Unsorted", "Brian Lara Cricket 96 (E) [a1][x]", 0 ) // is it good?


GAME( 1900, g_buck,    0,        megadriv,    megadriv,    g_buck, ROT0,   "Unsorted", "Buck Rogers - Countdown to Doomsday (UE) [!]", 0 )
GAME( 1900, g_curs,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Curse (J) [!]", 0 )
GAME( 1900, g_hb95,    0,        megadriv,    megadriv,    g_hb95, ROT0,   "Unsorted", "HardBall '95 (U) [!]", 0 )
GAME( 1900, g_ws98,    0,        megadriv,    megadriv,    g_hb95, ROT0,   "Unsorted", "World Series Baseball '98 (U) [!]", 0 )

GAME( 1900, g_dark,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dark Castle (UE) [!]", 0 )
GAME( 1900, g_4081,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Darwin 4081 (J) [!]", 0 )

GAME( 1900, g_drs,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Death and Return of Superman, The (U) [!]", 0 )
GAME( 1900, g_hire,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dinosaurs for Hire (U) [!]", 0 )
GAME( 1900, g_dblc,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Double Clutch (E) [c][!]", 0 )
GAME( 1900, g_elim,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Eliminate Down (J) [!]", 0 )

GAME( 1900, g_must,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Fire Mustang (J) [c][!]", 0 )

GAME( 1900, g_goof,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Goofy's Hysterical History Tour (U) [!]", 0 )

GAME( 1900, g_dodg,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Honoo no Toukyuuji Dodge Danpei (J) [!]", 0 )

GAME( 1900, g_jcte,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega / System Sacom", "Jennifer Capriati Tennis (U) [!]", 0 )
GAME( 1900, g_gste,    g_jcte,   megadpal,    megadriv,    megadrie, ROT0,   "Sega / System Sacom", "Grand Slam Tennis (E) [!]", 0 )
GAME( 1900, g_gstenj,  g_jcte,   megadriv,    megadriv,    megadrij, ROT0,   "Sega / System Sacom", "Grand Slam Tennis (J) [!]", 0 )

GAME( 1900, g_taru,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Magical Taruruuto-Kun (J) [!]", 0 )

GAME( 1900, g_osom,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Osomatsu-kun - Hachamecha Gekijou (J) [c][!]", 0 )
GAME( 1900, g_scob,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Scooby Doo Mystery (U) [!]", 0 )

GAME( 1900, g_srun,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Shadowrun (U) [!]", 0 )
GAME( 1900, g_shrunj,  g_srun,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Shadowrun (J) [!]", 0 )


GAME( 1900, g_snow,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Snow Bros. - Nick & Tom (J) [c][!]", 0 )
GAME( 1900, g_s_kp,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Spider-Man vs The Kingpin (W) [!]", 0 )

GAME( 1900, g_sbat,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Battleship (U) [!]", 0 )
GAME( 1900, g_tprk,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Theme Park (UE) [!]", 0 )
GAME( 1900, g_tint,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Tintin Au Tibet (E) (M6) [!]", 0 )

GAME( 1900, g_gomo,    0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Uchu Senkan Gomora (J) [c][!]", 0 ) // aka bioship paladin
GAME( 1900, g_usoc,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Ultimate Soccer (E) [!]", 0 )
GAME( 1900, g_wfrr,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "WWF Royal Rumble (W) [!]", 0 )
GAME( 1900, g_wfra,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "WWF RAW (W) [!]", 0 )
GAME( 1900, g_wolf,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wolf Child (U) [c][!]", 0 )

GAME( 1900, g_zoom,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zoom! (JU) [!]", 0 )
GAME( 1900, g_resq,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Resq (Beta)", 0 )
GAME( 1900, g_jely,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jelly Boy (Beta)", 0 )
GAME( 1900, g_itch,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Itchy and Scratchy Game, The (Beta)", 0 )
GAME( 1900, g_2sam,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Second Samurai, The (E)", 0 )
GAME( 1900, g_fcr,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fun Car Rally (Unl)", 0 )

GAME( 1900, g_pre2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Predator 2 (UE) [!]", 0 )

GAME( 1900, g_pm97,     0,        megadpal,    megadriv,    megadrie_backup_0x200000_0x10000, ROT0,   "Unsorted", "Premier Manager 97 (E) [!]", 0 )

GAME( 1900, g_pman,     0,        megadpal,    megadriv,    megadrie_backup_0x200000_0x10000, ROT0,   "Unsorted", "Premier Manager (E) [!]", 0 )

GAME( 1900, g_prim,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Primal Rage (UE) [!]", 0 )

GAME( 1900, g_pwbl,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Powerball (U) [c][!]", 0 )
GAME( 1900, g_wresbl,   g_pwbl,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Wrestleball (J) [c][!]", 0 )


GAME( 1900, g_ram3,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rambo III (W) (REV01) [!]", 0 )
GAME( 1900, g_ram300,   g_ram3,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Rambo III (W) (REV00) [!]", 0 )


GAME( 1900, g_revx,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Revolution X (UE) [!]", 0 )



GAME( 1900, g_sdan,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Shadow Dancer - The Secret of Shinobi (W) [c][!]", 0 )


GAME( 1900, g_sbt,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Battle Tank - War in the Gulf (U) [!]", 0 )


GAME( 1900, g_faer,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Faery Tale Adventure, The (UE) [!]", 0 )

GAME( 1900, g_gain,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Gain Ground (UE) [c][!]", 0 )
GAME( 1900, g_ggrouj,   g_gain,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Gain Ground (J) [!]", 0 )


GAME( 1900, g_0tol,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zero Tolerance (UE) [!]", 0 )

GAME( 1900, g_whip,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Whip Rush 2222 AD (U) [!]", 0 )
GAME( 1900, g_whipj,    g_whip,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Whip Rush 2222 AD (J) [!]", 0 )


GAME( 1900, g_vbar,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Virtual Bart (W) [!]", 0 )

GAME( 1900, g_twih,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Twin Hawk (JE) [!]", 0 )

GAME( 1900, g_twic,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Twin Cobra (U) [!]", 0 )
GAME( 1900, g_kyuuky,   g_twic,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Kyuukyou Tiger (J) [!]", 0 )


GAME( 1900, g_turr,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Turrican (U) [c][!]", 0 )
GAME( 1900, g_trux,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Truxton (W) [!]", 0 )
GAME( 1900, g_tp96,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Triple Play '96 (U) [c][!]", 0 ) // special backupram

GAME( 1900, g_tpgo,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Triple Play Gold (U) [c][!]", 0 ) // special backupram
GAME( 1900, g_tpgola,   g_tpgo,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Triple Play Gold (U) [a1]", 0 ) // special backupram

GAME( 1900, g_tkil,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Time Killers (U) [!]", 0 )
GAME( 1900, g_tkille,   g_tkil,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Time Killers (E) [!]", 0 )



GAME( 1900, g_tetr,     0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Tetris (J) [!]", 0 )
GAME( 1900, g_synd,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Syndicate (UE) [!]", 0 )

GAME( 1900, g_pins,     0,        megadriv,    megadri6,    megadrij, ROT0,   "Unsorted", "Power Instinct (J) [!]", 0 )
GAME( 1900, g_pac2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pac-Man 2 - The New Adventures (U) [!]", 0 )
GAME( 1900, g_nbd,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Ninja Burai Densetsu (J) [!]", 0 )
GAME( 1900, g_ko3,      0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Kick Off 3 - European Challenge (E) [!]", 0 )


GAME( 1900, g_kgk,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ka-Ge-Ki - Fists of Steel (U) [!]", 0 )
GAME( 1900, g_kageki,   g_kgk,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Ka-Ge-Ki (J) [!]", 0 )



GAME( 1900, g_wbdt,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "We're Back! - A Dinosaur's Tale (U) [!]", 0 )
GAME( 1900, g_comc,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Combat Cars (UE) [!]", 0 )

GAME( 1900, g_688a,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "688 Attack Sub (UE) [!]", 0 )

GAME( 1900, g_adiv,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Air Diver (U) [!]", 0 )
GAME( 1900, g_adivej,   g_adiv,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Air Diver (J) [!]", 0 )


GAME( 1900, g_asto,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Alien Storm (W) [!]", 0 )
GAME( 1900, g_agla,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "American Gladiators (U) [!]", 0 )
GAME( 1900, g_arch,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Arch Rivals (UE) [!]", 0 )
GAME( 1900, g_aptg,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Arnold Palmer Tournament Golf (UE) [!]", 0 )

GAME( 1900, g_atpt,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "ATP Tour Championship Tennis (U) [!]", 0 )
GAME( 1900, g_atpte,    g_atpt,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "ATP Tour Championship Tennis (E)", 0 )

GAME( 1900, g_arug,     0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Australian Rugby League (E) [!]", 0 )
GAME( 1900, g_bjak,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ball Jacks (JE) [c][!]", 0 )
GAME( 1900, g_barb,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Barbie Super Model (U) [!]", 0 )
GAME( 1900, g_bass,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bass Masters Classics (U) [!]", 0 )
GAME( 1900, g_basp,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bass Masters Classic Pro Edition (U) [!]", 0 )
GAME( 1900, g_barn,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Barney's Hide & Seek Game (U) [!]", 0 )
GAME( 1900, g_btl2,     0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Battle Mania - Dai Gin Jou (J) [c][!]", 0 )

GAME( 1900, g_tsht,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Trouble Shooter (U) [!]", 0 )
GAME( 1900, g_btlm,    g_tsht,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Battle Mania (J) [!]", 0 )


GAME( 1900, g_btms,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Battlemaster (U) [c][!]", 0 )
GAME( 1900, g_btec,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Battletech (U) [!]", 0 )

GAME( 1900, g_bear,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Berenstain Bears', The - Camping Adventure (U) [!]", 0 )
GAME( 1900, g_bw95,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bill Walsh College Football '95 (U) [!]", 0 )
GAME( 1900, g_bwcf,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bill Walsh College Football (UE) [c][!]", 0 )
GAME( 1900, g_bimi,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Bimini Run (U) (REV02) [!]", 0 )

GAME( 1900, g_cbwl,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Championship Bowling (U) [!]", 0 )
GAME( 1900, g_bwbw,     g_cbwl,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Boogie Woogie Bowling (J) [!]", 0 )


GAME( 1900, g_bh95,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Brett Hull Hockey '95 (U) [!]", 0 )


GAME( 1990, g_lvsc,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Lakers vs Celtics and the NBA Playoffs (UE) [!]", 0 )

GAME( 1993, g_bvsb,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Bulls vs Blazers and the NBA Playoffs (UE) [!]", 0 )
GAME( 1993, g_npbb,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "NBA Playoff - Bulls vs Blazers (J) [!]", 0 )

GAME( 1991, g_bvsl,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Bulls vs Lakers and the NBA Playoffs (UE) [!]", 0 )
GAME( 1993, g_npbl,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "NBA Pro Basketball - Bulls vs Lakers (J) [!]", 0 )

GAME( 1900, g_cada,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cadash (JU) [c][!]", 0 )
GAME( 1900, g_caes,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Caesars Palace (U) [!]", 0 )
GAME( 1900, g_crjb,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Cal Ripken Jr. Baseball (U) [c][!]", 0 )
GAME( 1900, g_c50,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Caliber Fifty (U) [!]", 0 )
GAME( 1900, g_cgam,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "California Games (UE) [!]", 0 )


GAME( 1900, g_capa,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Captain America and the Avengers (U) [!]", 0 )
GAME( 1900, g_capamb,   g_capa,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Captain America and the Avengers (Beta)", 0 )
GAME( 1900, g_capame,   g_capa,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Captain America and the Avengers (E)", 0 )


GAME( 1900, g_chik,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chiki Chiki Boys (UE) [!]", 0 )
GAME( 1900, g_chikij,   g_chik,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Chiki Chiki Boys (J) [!]", 0 )


GAME( 1900, g_clue,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Clue (U) [!]", 0 )
GAME( 1900, g_cuty,     0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Cuty Suzuki no Ringside Angel (J) [!]", 0 )
GAME( 1900, g_ddwe,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "D&D - Warriors of the Eternal Sun (UE) [!]", 0 )

GAME( 1900, g_dlnd,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dino Land (U) [!]", 0 )
GAME( 1900, g_dinolj,   g_dlnd,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dino Land (J) [!]", 0 )


GAME( 1900, g_e_bt,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "ESPN Baseball Tonight (U) [!]", 0 )

GAME( 1900, g_e_hn,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "ESPN National Hockey Night (U) [c][!]", 0 )
GAME( 1900, g_e_nhb,    g_e_hn,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "ESPN National Hockey Night (Beta)", 0 )

GAME( 1900, g_e_sn,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "ESPN Sunday Night NFL (U) [!]", 0 )

GAME( 1900, g_ehrd,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Evander Holyfield's Real Deal Boxing (JU) [!]", 0 )

GAME( 1900, g_feud,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Family Feud (U) [!]", 0 )
GAME( 1900, g_fatl,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fatal Labyrinth (JU) [!]", 0 )
GAME( 1993, g_fifa,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "FIFA International Soccer (UE) (M4) [!]", 0 )




GAME( 1900, g_grwl,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Growl (U) [!]", 0 )
GAME( 1900, g_runark,   g_grwl,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Runark (J) [!]", 0 )


GAME( 1900, g_hnov,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Heavy Nova (U) [!]", 0 )
GAME( 1900, g_hice,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Hit The Ice (U) [c][!]", 0 )
GAME( 1900, g_jnpg,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x800, ROT0,   "Unsorted", "Jack Nicklaus' Power Challenge Golf (UE) [!]", 0 )

GAME( 1900, g_ktm,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "King of the Monsters (U) [!]", 0 )
GAME( 1900, g_kotme,    g_ktm,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "King of the Monsters (E) [!]", 0 )

GAME( 1900, g_klax,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Klax (UE) [!]", 0 )
GAME( 1900, g_klaxj,    g_klax,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Klax (J) [c][!]", 0 )

GAME( 1900, g_ktm2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "King of the Monsters 2 (U) [!]", 0 )

GAME( 1900, g_mamr,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mario Andretti Racing (UE) [!]", 0 )

GAME( 1900, g_mamo,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Master of Monsters (U) [!]", 0 )
GAME( 1900, g_momonj,   g_mamo,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Master of Monsters (J) [!]", 0 )

GAME( 1900, g_mb8p,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Factor 5", "Mega Bomberman - 8 Player Demo (Unl)", 0 )


GAME( 1900, g_tnzs,     0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "New Zealand Story, The (J) [!]", 0 )
GAME( 1900, g_noes,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "No Escape (U) [!]", 0 )
GAME( 1900, g_norm,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Normy's Beach Babe-O-Rama (UE) [!]", 0 )

GAME( 1900, g_papb,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Paperboy (UE) [!]", 0 )
GAME( 1900, g_pboyj,    g_papb,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Paperboy (J) [!]", 0 )

GAME( 1900, g_pap2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Paperboy II (U) [!]", 0 )
GAME( 1900, g_pop2,     0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Populous II - Two Tribes (E) [!]", 0 )

GAME( 1900, g_quad,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Quad Challenge (U) [c][!]", 0 )
GAME( 1900, g_mtrx,     g_quad,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "MegaTraX (J) [c][!]", 0 )

GAME( 1900, g_risk,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Risk (U) [!]", 0 )

GAME( 1900, g_rise,     0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Rise of the Robots (E) [!]", 0 )

GAME( 1900, g_shov,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Shove It! - The Warehouse Game (U) [!]", 0 )
GAME( 1900, g_sokoba,   g_shov,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Sokoban (J) [!]", 0 )


GAME( 1900, g_std9,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Star Trek - Deep Space 9 - Crossroads of Time (U) [!]", 0 )
GAME( 1900, g_stds9e,   g_std9,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Star Trek - Deep Space 9 - Crossroads of Time (E)", 0 )


GAME( 1900, g_stng,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Star Trek - The Next Generation (U) (REV01) [!]", 0 )
GAME( 1900, g_stng00,   g_stng,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Star Trek - The Next Generation (U) (REV00) [!]", 0 )

GAME( 1900, g_t2ar,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "T2 - The Arcade Game (UE) (REV01) [!]", 0 )
GAME( 1900, g_t2arcb,   g_t2ar,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "T2 - The Arcade Game (Beta)", 0 )


GAME( 1900, g_vali,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Valis (U) [!]", 0 )
GAME( 1900, g_valj,     g_vali,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Valis (J) [!]", 0 )

GAME( 1900, g_val3,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Valis III (U) [!]", 0 )
GAME( 1900, g_val3j,    g_val3,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Valis III (J) [!]", 0 )

GAME( 1900, g_vpin,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Virtual Pinball (UE) [!]", 0 )

GAME( 1900, g_nl98,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NBA Live 98 (U) [!]", 0 ) // backupram not in header
GAME( 1900, g_nl97,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NBA Live 97 (UE) [!]", 0 )
GAME( 1900, g_nl96,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NBA Live 96 (UE) [!]", 0 )
GAME( 1900, g_nl95,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NBA Live 95 (UE) [!]", 0 )

GAME( 1900, g_nasc,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NBA All-Star Challenge (UE) [!]", 0 )

GAME( 1994, g_nact,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Sega Sports", "NBA Action (U) [!]", 0 )
GAME( 1900, g_na95,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Sega Sports", "NBA Action '95 (UE) [!]", 0 )




GAME( 1900, g_nbs9,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NBA Showdown 94 (UE) [!]", 0 )
GAME( 1900, g_nba94b,   g_nbs9,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NBA Showdown 94 (Beta)", 0 )
GAME( 1900, g_nba9,     g_nbs9,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "NBA Pro Basketball '94 (J) [!]", 0 )


GAME( 1900, g_nccf,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NCAA College Football (U) [!]", 0 )
GAME( 1900, g_ncff,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NCAA Final Four College Basketball (U) [!]", 0 )

GAME( 1997, g_nfl8,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Sega Sports", "NFL 98 (U) [!]", 0 )
GAME( 1994, g_nfl5,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Sega Sports", "NFL '95 (U) [!]", 0 )

GAME( 1900, g_nfl4,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NFL Football '94 Starring Joe Montana (U) [!]", 0 ) // backupram not in header
GAME( 1900, g_nfl94j,   g_nfl4,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "NFL Football '94 Starring Joe Montana (J) [!]", 0 )

GAME( 1995, g_nfpt,     0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Sega Sports", "NFL Prime Time (U) [!]", 0 )

GAME( 1900, g_nqc6,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NFL Quarterback Club 96 (UE) [!]", GAME_NOT_WORKING ) // eeprom?
GAME( 1900, g_nqc,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NFL Quarterback Club (W) [!]", GAME_NOT_WORKING ) // eeprom?

GAME( 1900, g_nh95,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "NHL 95 (UE) [!]", 0 )
GAME( 1900, g_nh9e,      g_nh95,   megadpal,    megadriv,    megadrie_backup_0x200000_0x10000, ROT0,   "Unsorted", "NHL 95 Elitserien (E) [c][!]", 0 )

GAME( 1900, g_nh96,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "NHL 96 (UE) [!]", 0 ) // backupram not in header
GAME( 1900, g_nhl96e,    g_nh96,   megadpal,    megadriv,    megadrie_backup_0x200000_0x4000, ROT0,   "Unsorted", "NHL 96 Elitserien (E)", 0 )

GAME( 1900, g_nh97,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "NHL 97 (UE) [!]", 0 )

GAME( 1900, g_nash,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NHL All-Star Hockey '95 (U) [!]", 0 )

GAME( 1900, g_nhlp,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NHLPA Hockey '93 (UE) (REV01) [!]", 0 )
GAME( 1900, g_nhlp00,    g_nhlp,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NHLPA Hockey '93 (UE) (REV00)", 0 )


GAME( 1900, g_olsg,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Olympic Summer Games Atlanta 96 (UE) [!]", 0 )

GAME( 1990, g_patb,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Sega", "Pat Riley Basketball (U) [!]", 0 )
GAME( 1900, g_srba,      g_patb,   megadpal,    megadriv,    megadrie, ROT0,   "Sega", "Super Real Basketball (E) [c][!]", 0 )
GAME( 1900, g_srealj,    g_patb,   megadriv,    megadriv,    megadrij, ROT0,   "Sega", "Super Real Basketball (J) [c][!]", 0 )

GAME( 1900, g_pele,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Accolate", "Pele! (UE) [!]", 0 )
GAME( 1900, g_pelw,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Sport Accolate", "Pele's World Tournament Soccer (UE) [!]", 0 )

GAME( 1900, g_pgae,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "PGA European Tour (UE) [!]", 0 )
GAME( 1900, g_rb3,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "RBI Baseball 3 (U) [c][!]", 0 )

GAME( 1900, g_rb4,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "RBI Baseball 4 (U) [!]", 0 )
GAME( 1900, g_rbi4b,     g_rb4,    megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "RBI Baseball 4 (Beta)", 0 )

GAME( 1900, g_rb93,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "RBI Baseball 93 (U) [!]", 0 )
GAME( 1900, g_rb94,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "RBI Baseball 94 (UE) [!]", 0 )
GAME( 1900, g_rcvm,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Roger Clemens' MVP Baseball (U) [!]", 0 )


GAME( 1900, g_rw93,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rugby World Cup 1995 (UE) (M3) [!]", 0 )
GAME( 1900, g_swcr,      0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Shane Warne Cricket (E) [c][!]", 0 )
GAME( 1900, g_shi,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super High Impact (UE) [!]", 0 )




GAME( 1900, g_svol,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Volleyball (U) [c][!]", 0 )
GAME( 1900, g_svolua,    g_svol,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Super Volleyball (U) [a1][!]", 0 )
GAME( 1900, g_svolx,     g_svol,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Super Volleyball (J) [x]", 0 ) // pirate version?



GAME( 1900, g_ws96,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Series Baseball '96 (U) [!]", 0 ) // special backup?
GAME( 1900, g_ws95,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Series Baseball '95 (U) [!]", 0 )
GAME( 1900, g_wsb,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Series Baseball (U) [!]", 0 )

// gfx flicker
GAME( 1900, g_wts,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Trophy Soccer (U) [!]", GAME_IMPERFECT_GRAPHICS )
GAME( 1900, g_ecs,       g_wts,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "European Club Soccer (E) [!]", GAME_IMPERFECT_GRAPHICS )
GAME( 1900, g_jlcs,      g_wts,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "J. League Champion Soccer (J) [!]", GAME_IMPERFECT_GRAPHICS )


GAME( 1900, g_wfsw,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "WWF Super Wrestlemania (UE) [!]", 0 )

GAME( 1900, g_ma94,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "John Madden NFL 94 (UE) [c][!]", 0 )
GAME( 1900, g_ma95,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Madden NFL 95 (UE) [!]", 0 )
GAME( 1900, g_ma96,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Madden NFL 96 (UE) [!]", 0 )
GAME( 1900, g_ma97,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "Madden NFL 97 (UE) [!]", 0 )
GAME( 1900, g_ma98,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "Madden NFL 98 (U) [c][!]", 0 ) // backupram not in header


GAME( 1900, g_ma93,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "John Madden Football '93 (UE) [c][!]", 0 )


GAME( 1900, g_ma92,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "John Madden Football '92 (UE) [!]", 0 )
GAME( 1900, g_ma3c,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "John Madden Football '93 - Championship Edition (U) [!]", 0 )

GAME( 1900, g_ma,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "John Madden Football (UE) [!]", 0 )
GAME( 1900, g_madj,      g_ma,     megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "John Madden Football (J) [!]", 0 )

GAME( 1900, g_jmof,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Joe Montana Football (JU) [!]", 0 )

GAME( 1900, g_jms2,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Joe Montana Sports Talk Football 2 (W) (REV01) [!]", 0 )
GAME( 1900, g_jmons2,    g_jms2,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Joe Montana Sports Talk Football 2 (W) (REV00) [!]", 0 )

GAME( 1900, g_jms,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Joe Montana Sports Talk Football (UE) [!]", 0 )

GAME( 1900, g_hb94,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "HardBall '94 (UE) [!]", 0 )
GAME( 1900, g_hb3,       0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "HardBall III (U) [!]", 0 ) // backup ram not in header
GAME( 1900, g_hb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Accolate / Ballistic", "HardBall! (U) [!]", 0 )

GAME( 1900, g_coak,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Coach K College Basketball (U) [!]", 0 )
GAME( 1900, g_cf96,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "College Football USA 96 (U) [!]", 0 ) // backup ram not in header..
GAME( 1900, g_cf97,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "College Football USA 97 - The Road to New Orleans (U) [!]", 0 )

GAME( 1994, g_cfn,       0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Sega Sports", "College Football's National Championship (U) [!]", 0 )

GAME( 1995, g_cfn2,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Sega Sports", "College Football's National Championship II (U) [!]", 0 )

GAME( 1900, g_csla,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "College Slam (U) [!]", GAME_NOT_WORKING ) // eeprom
GAME( 1900, g_crys,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Crystal's Pony Tale (U) [!]", 0 )
GAME( 1900, g_cybb,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "CyberBall (W) [!]", 0 )

GAME( 1900, g_dcat,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "David Crane's Amazing Tennis (U) [!]", 0 )
GAME( 1900, g_drbb,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "David Robinson Basketball (J) [!]", 0 )
GAME( 1900, g_drsc,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "David Robinson's Supreme Court (U) [!]", 0 )
GAME( 1900, g_dc3d,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Devil's Course 3-D Golf (J) [!]", 0 )
GAME( 1900, g_drib,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Double Dribble - The Playoff Edition (U) [!]", 0 )

GAME( 1900, g_dbzj,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dragon Ball Z - Buyuu Retsuden (J) [!]", 0 )
GAME( 1900, g_dbzf,      g_dbzj,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Dragon Ball Z - L'Appel du Destin (F) [!]", 0 )

GAME( 1900, g_deye,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dragon's Eye - Shanghai 3 (J) [!]", 0 )

// really clones?
GAME( 1900, g_nhlh,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NHL Hockey (U) [!]", 0 )
GAME( 1900, g_eaho,      g_nhlh,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "EA Hockey (E) [!]", 0 )
GAME( 1900, g_prho,      g_nhlh,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Pro Hockey (J) [!]", 0 )

GAME( 1900, g_f1c,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "F1 Circus MD (J) [!]", 0 )
GAME( 1900, g_f1gp,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "F1 Grand Prix - Nakajima Satoru (J) [!]", 0 )
GAME( 1900, g_f1h,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "F1 Hero MD (J) [c][!]", 0 )
GAME( 1900, g_f1sl,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "F1 Super License - Nakajima Satoru (J) [!]", 0 )
GAME( 1900, g_fas1,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Fastest 1 (J) [!]", 0 )


GAME( 1900, g_fpro,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Fire Pro Wrestling Gaiden (J) [c][!]", 0 )
GAME( 1900, g_fore,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Foreman For Real (W) [!]", 0 )

GAME( 1900, g_fung,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fun-N-Games (U) [c][!]", 0 )
GAME( 1900, g_funnge,    g_fung,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Fun-N-Games (E)", 0 )


GAME( 1900, g_gws,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Great Waldo Search, The (U) [!]", 0 )
GAME( 1900, g_imgi,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "IMG International Tour Tennis (UE) [!]", 0 )


GAME( 1900, g_intr,      0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "International Rugby (E) [c][!]", 0 )
GAME( 1900, g_ishi,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ishido - The Way of the Stones (U) [c][!]", 0 )

GAME( 1900, g_jlp2,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "J. League Pro Striker 2 (J) [!]", 0 )
GAME( 1900, g_jlpp,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "J. League Pro Striker - Perfect Edition (J) [c][!]", 0 )

GAME( 1900, g_jlpf,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "J. League Pro Striker Final Stage (J) [c][!]", 0 )

GAME( 1900, g_jbdb,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "James Buster Douglas Knock Out Boxing (UE) [c][!]", 0 )
GAME( 1900, g_fblo,      g_jbdb,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Final Blow (J) [c][!]", 0 )


GAME( 1900, g_jamm,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jammit (U) [!]", 0 )

GAME( 1900, g_jano,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Janou Touryumon (J) [!]", 0 )
GAME( 1900, g_janout,    g_jano,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Janou Touryumon (J) [a1][!]", 0 )

GAME( 1900, g_jant,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Jantei Monogatari (J) [!]", 0 )
GAME( 1900, g_jeop,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jeopardy! (U) [c][!]", 0 )
GAME( 1900, g_jeod,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jeopardy! Deluxe (U) [c][!]", 0 )
GAME( 1900, g_jeos,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jeopardy! Sports Edition (U) [!]", 0 )
GAME( 1900, g_jgpf,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jerry Glanville's Pigskin Footbrawl (U) [!]", 0 )
GAME( 1900, g_jmac,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Joe & Mac Caveman Ninja (U) [!]", 0 )



GAME( 1900, g_kbou,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "King's Bounty (UE) [!]", 0 )

GAME( 1900, g_msbu,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Magic School Bus, The (U) [!]", 0 )
GAME( 1900, g_mhy,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Mamono Hunter Yohko - Makai Kara no Tenkosei (J) [!]", 0 )
GAME( 1900, g_mmad,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Marble Madness (UE) [!]", 0 )
GAME( 1900, g_mlem,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mario Lemieux Hockey (UE) [!]", 0 )
GAME( 1900, g_mowe,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Master of Weapon (J) [!]", 0 )
GAME( 1900, g_math,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Math Blaster - Episode 1 (U) [!]", 0 )

GAME( 1900, g_mswive,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Mega SWIV (E)", 0 )
GAME( 1900, g_mswi,      g_mswive, megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Mega SWIV (E) [p1][!]", 0 )


GAME( 1900, g_merc,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mercs (W) [!]", 0 )
GAME( 1900, g_mult,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mickey's Ultimate Challenge (U) [!]", 0 )
GAME( 1900, g_mmg,       0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Might and Magic - Gates to Another World (UE) (REV01) [!]", 0 )
GAME( 1900, g_mlbb,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "MLBPA Baseball (U) [!]", 0 )

GAME( 1900, g_mlbs,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "MLBPA Sports Talk Baseball (U) [!]", 0 )

GAME( 1900, g_mahb,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Muhammad Ali Heavyweight Boxing (U) [c][!]", 0 )
GAME( 1900, g_mahbb,     g_mahb,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Muhammad Ali Heavyweight Boxing (Beta)", 0 )
GAME( 1900, g_mahbe,     g_mahb,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Muhammad Ali Heavyweight Boxing (E)", 0 )


GAME( 1900, g_nhir,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Newman-Haas IndyCar Racing (W) [!]", 0 )

GAME( 1900, g_dmov,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Kaneko", "Deadly Moves (U) [!]", 0 )
GAME( 1900, g_path,    g_dmov,   megadriv,    megadriv,    megadrij, ROT0,   "Kaneko", "Power Athlete (J) [!]", 0 )

GAME( 1900, g_prqu,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pro Quarterback (U) [c][!]", 0 )
GAME( 1900, g_ramp,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rampart (U) [!]", 0 )
GAME( 1900, g_rsbt,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Richard Scarry's Busytown (U) [!]", 0 )

/* these need eeprom emulation */
GAME( 1900, g_rmmw,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Rockman Megaworld (J) [!]", 0 )
GAME( 1900, g_rkmnja,    g_rmmw,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Rockman Megaworld (J) [a1][!]", 0 )
GAME( 1900, g_megme,     g_rmmw,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Megaman - The Wily Wars (E)", 0 )



GAME( 1900, g_robw,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rocky and Bullwinkle, The Adventures of (U) [!]", 0 )





GAME( 1900, g_sdnk,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Slam Dunk - Shikyou Gekitotsu! (J) [!]", 0 )

GAME( 1900, g_sspo,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Slaughter Sport (U) [c][!]", 0 )
GAME( 1900, g_fatm,      g_sspo,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Fatman (J) [c][!]", 0 )

GAME( 1900, g_scon,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Star Control (U) [c][!]", 0 )
GAME( 1900, g_ssma,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Street Smart (JU) [!]", 0 )

GAME( 1900, g_sumc,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Summer Challenge (U) [!]", 0 )

GAME( 1900, g_sleage,    0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Super League (E)", 0 )
GAME( 1900, g_sl,        g_sleage, megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Super League (J) [!]", 0 )


GAME( 1900, g_sl91,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Super League 91 (J) [!]", 0 )
GAME( 1900, g_smas,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Super Masters Golf (J) [!]", 0 )

GAME( 1900, g_swso,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sword of Sodan (UE) [!]", 0 )
GAME( 1900, g_swsoj,     g_swso,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Sword of Sodan (J) [!]", 0 )

GAME( 1900, g_swve,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sword of Vermilion (UE) [!]", 0 )
GAME( 1900, g_swvej,     g_swve,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sword of Vermilion (J) [c][!]", 0 )

GAME( 1900, g_tusa,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Team USA Basketball (UE) [c][!]", 0 )
GAME( 1900, g_dtusa,     g_tusa,   megadriv,    megadriv,    megadrij, ROT0,   "Electronic Arts", "Dream Team USA (J) [!]", 0 )


GAME( 1900, g_tcls,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Techno Clash (UE) [!]", 0 )

GAME( 1900, g_tcop,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Technocop (U) [!]", 0 )

GAME( 1900, g_tcupx,     0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Tecmo Cup (J) [x]", 0 ) // original? (no hacked title)
GAME( 1900, g_tc,        g_tcupx,  megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Tecmo Cup (J) [p1][!]", 0 )

GAME( 1900, g_tsb,       0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tecmo Super Baseball (U) [!]", 0 )

GAME( 1900, g_tsbw,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tecmo Super Bowl (U) (Oct 1993) [!]", 0 )
GAME( 1900, g_tsbwlj,    g_tsbw,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tecmo Super Bowl (J) [!]", 0 )
GAME( 1900, g_tsbwlu,    g_tsbw,   megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tecmo Super Bowl (U) (Sep 1993) [!]", 0 )


GAME( 1900, g_tbw2,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "Tecmo Super Bowl II SE (U) [!]", 0 )
GAME( 1900, g_tsbw2j,    g_tbw2,   megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Tecmo Super Bowl II SE (J) [!]", 0 )

GAME( 1900, g_tbw3,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "Tecmo Super Bowl III Final Edition (U) [!]", 0 )

GAME( 1900, g_tsh,       0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tecmo Super Hockey (U) [!]", 0 )

GAME( 1900, g_tsnb,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tecmo Super NBA Basketball (U) [!]", 0 )
GAME( 1900, g_tsnbaj,    g_tsnb,   megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tecmo Super NBA Basketball (J) [!]", 0 )

GAME( 1900, g_tw93,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tecmo World Cup '93 (U) [!]", 0 )
GAME( 1900, g_tw92,      g_tw93,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Tecmo World Cup '92 (J) [!]", 0 )

GAME( 1900, g_ttnk,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Thomas the Tank Engine & Friends (U) [c][!]", 0 )
GAME( 1900, g_tlbb,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tommy Lasorda Baseball (U) [!]", 0 )
GAME( 1900, g_tr95,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "Tony La Russa Baseball 95 (U) [!]", 0 )
GAME( 1900, g_trbb,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tony La Russa Baseball (U) [!]", 0 )
GAME( 1900, g_totf,      0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Total Football (E) [!]", 0 )
GAME( 1900, g_toxi,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Toxic Crusaders (U) [!]", 0 )
GAME( 1900, g_tanf,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Troy Aikman NFL Football (U) [!]", 0 )
GAME( 1900, g_uman,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Ultraman (J) [!]", 0 )
GAME( 1900, g_ur95,      0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x800, ROT0,   "Unsorted", "Unnecessary Roughness 95 (U) [!]", 0 )

GAME( 1900, g_vapt,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Vapor Trail (U) [!]", 0 )
GAME( 1900, g_kuuga,  g_vapt,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Kuuga - Operation Vapor Trail (J) [!]", 0 )


GAME( 1900, g_wack,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wacky Worlds (U) [!]", 0 )
GAME( 1900, g_wgas,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wayne Gretzsky NHLPA All-Stars (UE) [!]", 0 )
GAME( 1900, g_wfor,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wheel of Fortune (U) [!]", 0 )

GAME( 1900, g_winc,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Winter Challenge (UE) [!]", 0 )
GAME( 1900, g_wchalb,    g_winc,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Winter Challenge (Beta)", 0 )

GAME( 1900, g_wb3,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wonder Boy III - Monster Lair (JE) [c][!]", 0 )

GAME( 1900, g_wbmw,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wonder Boy in Monster World (UE) [!]", 0 )
GAME( 1900, g_turma,     g_wbmw,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Turma da Monica na Terra Dos Monstros (B) [!]", 0 )


GAME( 1900, g_wcl,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Class Leaderboard Golf (U) [c][!]", 0 )
GAME( 1900, g_wclbx,     g_wcl,    megadriv,    megadriv,    megadrie, ROT0,   "Unsorted", "World Class Leaderboard Golf (E) [x]", 0 )


GAME( 1900, g_wc94,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Cup USA 94 (UE) [!]", 0 )





GAME( 1900, g_cwcs,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Champions World Class Soccer (W) (M4) [!]", 0 )
GAME( 1900, g_cpam,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Championship Pro-Am (U) [c][!]", 0 )
GAME( 1900, g_cdor,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Electronic Arts", "Centurion - Defender of Rome (UE) [!]", 0 )

// golf game
GAME( 1900, g_chi,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chi Chi's Pro Challenge (U) [!]", 0 )
GAME( 1900, g_bglf,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Battle Golfer Yui (J) [c][!]", 0 )

GAME( 1900, g_baha,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Bahamut Senki (J) [c][!]", 0 )
GAME( 1900, g_advdai,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Advanced Daisenryaku (J) (REV01) [!]", 0 )
GAME( 1900, g_blueal,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Blue Almanac (J) [!]", 0 )
GAME( 1900, g_chibi,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Chibi Maruko-Chan - Wakuwaku Shopping (J) [c][!]", 0 )
GAME( 1900, g_crayon,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Crayon Shin-Chan - Arashi o Yobu Sono Ko (J) [!]", 0 )



GAME( 1900, g_eadoub,      0,        megadpal,    megadriv,    megadrie, ROT0,   "Electronic Arts", "EA Sports Double Header (E) [!]", 0 )
GAME( 1900, g_fushig,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Namco", "Fushigi no Umi no Nadia (J) [c][!]", 0 )
GAME( 1900, g_ftbhb,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Frank Thomas Big Hurt Baseball (UE) [!]", 0 )

GAME( 1900, g_ghwor,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Greatest Heavyweights of the Ring (U) [!]", 0 )
GAME( 1900, g_ghwj,        g_ghwor,  megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Greatest Heavyweights of the Ring (J) [c][!]", 0 )


GAME( 1900, g_hssoc,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "High School Soccer - Kunio Kun (J) [!]", 0 )
GAME( 1900, g_ichir,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Ichidant R (J) [!]", 0 )
GAME( 1900, g_hyokk,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Hyokkori Hyoutanjima (J) [c][!]", 0 ) // inputs??

GAME( 1900, g_kishi,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Kishi Densetsu (J) [!]", 0 )
GAME( 1900, g_kyuuk,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Kyuukai Douchuuki (J) [c][!]", 0 )


GAME( 1900, g_m1tank,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "M-1 Abrams Battle Tank (UE) (REV01) [!]", 0 )
GAME( 1900, g_madou,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Madou Monogatari I (J) [!]", 0 )


GAME( 1900, g_mahcop,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Mahjong Cop Ryuu - Shiro Ookami no Yabou (J) [!]", 0 )

GAME( 1900, g_maten,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Maten no Soumetsu / Maten Densetsu (J) [!]", 0 )

GAME( 1900, g_manser,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Mega Anser (J) (REV01) [!]", 0 )
GAME( 1900, g_megaq,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Mega Q - The Party Quiz Game (J) [!]", 0 )
GAME( 1900, g_mfang,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Metal Fangs (J) [c][!]", 0 )
GAME( 1900, g_mpiano,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Miracle Piano Teaching System (U) [!]", 0 )
GAME( 1900, g_monwr4,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Monster World IV (J) [!]", 0 )


GAME( 1900, g_new3dg,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "New 3D Golf Simulation Harukanaru Augusta (J) [!]", 0 )
GAME( 1900, g_nikkan,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Nikkan Sports Pro Yakyuu Van (J) [!]", 0 )
GAME( 1900, g_patlab,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Patlabor (J) [c][!]", 0 )
GAME( 1900, g_psyobl,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Psy-O-Blade Moving Adventure (J) [!]", 0 )
GAME( 1900, g_ransei,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Ransei no Hasha (J) [!]", 0 )


GAME( 1900, g_renthe,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Rent A Hero (J) [!]", 0 )

GAME( 1900, g_sangor,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sangokushi Retsuden - Ransei no Eiyuu Tachi (J) [!]", 0 )
GAME( 1900, g_sanret,      g_sangor, megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sangokushi Retsuden (Ch)", 0 )

GAME( 1900, g_shikin,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Shi Kin Joh (J) [!]", 0 )
GAME( 1900, g_shogi,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Shogi no Hoshi (J) [!]", 0 )


GAME( 1900, g_shura,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Shura no Mon (J) [!]", 0 )
GAME( 1900, g_slapf,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Slap Fight (J) [c][!]", 0 )

GAME( 1900, g_supdai,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Super Daisenryaku (J) (REV00) [!]", 0 )
GAME( 1900, g_surgin,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Surging Aura (J) [!]", 0 )

GAME( 1900, g_taiga,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Taiga Drama Taiheiki (J) [!]", 0 )
GAME( 1900, g_telmj,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Tel Tel Mahjong (J) [c][!]", 0 )
GAME( 1900, g_telstd,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Tel Tel Stadium (J) [!]", 0 )
GAME( 1900, g_tpglf2,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Top Pro Golf 2 (J) [!]", 0 )
GAME( 1900, g_tpglf,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Top Pro Golf (J) [!]", 0 )
GAME( 1900, g_twintl,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Twinkle Tale (J) [!]", 0 )

GAME( 1900, g_verytx,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Verytex (J) [c][!]", 0 )
GAME( 1900, g_vix357,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Vixen 357 (J) [!]", 0 )

// golf game
GAME( 1900, g_waiala,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Waialae no Kiseki (J) [!]", 0 )


GAME( 1900, g_warps,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Accolade", "Warpspeed (U) [!]", 0 )

GAME( 1900, g_wboy5,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Wonder Boy V - Monster World III (J) [!]", 0 )
GAME( 1900, g_wonlib,      0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Wonder Library (J) [!]", GAME_NOT_WORKING ) // what is this? megadrive, mega cd?, accesses extra regs, extra ram etc.

GAME( 1900, g_xdaze,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "X Dazedly Ray (J) [!]", 0 )

GAME( 1900, g_yuyuga,      0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Yuu Yuu Hakusho Gaiden (J) [!]", 0 )
GAME( 1991, g_zanya,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Wolfteam", "Zan - Yasha Enbukyoku / Zan Yasha Enbuden (J) [!]", 0 )


GAME( 1900, g_aresha,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "A Ressha de Gyoukou MD (J) [!]", 0 )


GAME( 1900, g_chouya,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Chou Yakyuu Miracle Nine (J) [!]", 0 )

GAME( 1900, g_dynab2,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dyna Brothers 2 (J) [!]", 0 )

GAME( 1900, g_f15s,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "F-15 Strike Eagle II (U) [!]", 0 )
GAME( 1900, g_f15e,     g_f15s,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "F-15 Strike Eagle II (E) [!]", 0 )
GAME( 1900, g_f15b,     g_f15s,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "F-15 Strike Eagle II (Beta)", 0 )

GAME( 1900, g_gamblr,       0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Gambler Jiko Chuushinha - Katayama Masayuki no Mahjong Doujou (J) [!]", 0 )





GAME( 1900, g_kingcj,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "King Colossus (J) [!]", 0 )



GAME( 1900, g_lordmo,       0,        megadriv,    megadri6,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Lord Monarch (J) [!]", 0 )

GAME( 1900, g_mbmb,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mega Bomberman (UE) [!]", 0 )
GAME( 1900, g_mbmba,    g_mbmb,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mega Bomberman (UE) [a1][!]", 0 )




GAME( 1900, g_njam,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NBA Jam (UE) (REV01) [!]", 0 )
GAME( 1900, g_nbaj00,  g_njam,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NBA Jam (UE) (REV00) [!]", 0 )
GAME( 1900, g_nbajj,   g_njam,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "NBA Jam (J) [!]", 0 )

GAME( 1900, g_njte,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NBA Jam Tournament Edition (W) (REV01) [!]", 0 )
GAME( 1900, g_nbajt0,  g_njte,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NBA Jam Tournament Edition (W) (REV00) [!]", 0 )

GAME( 1900, g_nbah,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "NBA Hang Time (U) [!]", 0 )
GAME( 1900, g_nbahte,  g_nbah,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "NBA Hang Time (E) [!]", 0 )

GAME( 1900, g_nigel,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Nigel Mansell's World Championship (U) [!]", 0 )
GAME( 1900, g_nigwce,  g_nigel,  megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Nigel Mansell's World Championship (E)", 0 )

GAME( 1900, g_orun,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "OutRun (W) [!]", 0 )
GAME( 1900, g_orunj,   g_orun,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "OutRun (J) [!]", 0 )


GAME( 1900, g_pitf,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pitfall - The Mayan Adventure (U) [!]", 0 )
GAME( 1900, g_pitfe,   g_pitf,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Pitfall - The Mayan Adventure (E) [!]", 0 )

GAME( 1900, g_pifg,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pit Fighter (W) (Oct 1991) [!]", 0 )
GAME( 1900, g_pifia,   g_pifg,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pit Fighter (UE) (Jun 1991) [!]", 0 )

GAME( 1900, g_poca,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pocahontas (U) [!]", 0 )
GAME( 1900, g_pocae,   g_poca,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Pocahontas (E) [!]", 0 )


GAME( 1900, g_rens,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ren and Stimpy Show, The - Stimpy's Invention (U) [!]", 0 )
GAME( 1900, g_renstb,  g_rens,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ren and Stimpy Show, The - Stimpy's Invention (Beta))", 0 )
GAME( 1900, g_renste,  g_rens,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Ren and Stimpy Show, The - Stimpy's Invention (E) [!]", 0 )

GAME( 1900, g_side,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Side Pocket (JU) [!]", 0 )
GAME( 1900, g_sidepe,  g_side,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Side Pocket (E) [!]", 0 )

GAME( 1900, g_mfpl,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Minnesota Fats Pool Legend (U) [!]", 0 )


GAME( 1900, g_bart,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Simpsons, The - Bart vs The Space Mutants (UE) (REV01) [!]", 0 )
GAME( 1900, g_bart00,  g_bart,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Simpsons, The - Bart vs The Space Mutants (UE) (REV00) [!]", 0 )

GAME( 1900, g_sorcer,        0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sorcerian (J) [!]", 0 )



GAME( 1900, g_ward,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wardner (U) [!]", 0 )
GAME( 1900, g_wardj,    g_ward,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Wardner no Mori Special (J) [!]", 0 )



GAME( 1900, g_wizl,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wiz 'n' Liz - The Frantic Wabbit Wescue (U) [!]", 0 )
GAME( 1900, g_wizlie,  g_wizl,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Wiz 'n' Liz - The Frantic Wabbit Wescue (E) [!]", 0 )

GAME( 1900, g_ztks,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zero the Kamikaze Squirrel (U) [!]", 0 )
GAME( 1900, g_ztkse,   g_ztks,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Zero the Kamikaze Squirrel (E) [!]", 0 )

GAME( 1900, g_bttf,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Back to the Future Part III (U) [!]", 0 )
GAME( 1900, g_bttfe,   g_bttf,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Back to the Future Part III (E) [!]", 0 )

GAME( 1900, g_bwre,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Beast Wrestler (U) [!]", 0 )
GAME( 1900, g_beaswr,  g_bwre,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Beast Warriors (J) [!]", 0 )

GAME( 1900, g_chuk,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chuck Rock (U) [c][!]", 0 )
GAME( 1900, g_chkrke,  g_chuk,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Chuck Rock (E) [c][!]", 0 )

GAME( 1900, g_chk2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chuck II - Son of Chuck (U) [!]", 0 )
GAME( 1900, g_chk2b,   g_chk2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Chuck II - Son of Chuck (Beta)", 0 )
GAME( 1900, g_chk2e,   g_chk2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Chuck II - Son of Chuck (E) [c][!]", 0 )
GAME( 1900, g_chk2j,   g_chk2,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Chuck II - Son of Chuck (J) [!]", 0 )


GAME( 1900, g_crkd,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Crack Down (U) [!]", 0 )
GAME( 1900, g_crkde,   g_crkd,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Crack Down (E) [!]", 0 )
GAME( 1900, g_crkdj,   g_crkd,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Crack Down (J) [!]", 0 )

GAME( 1900, g_djby,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "DJ Boy (U) [!]", 0 )
GAME( 1900, g_djboye,  g_djby,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "DJ Boy (E) [!]", 0 )
GAME( 1900, g_djboyj,  g_djby,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "DJ Boy (J) [!]", 0 )



GAME( 1900, g_eswa,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "ESWAT Cyber Police - City Under Siege (U) [!]", 0 )
GAME( 1900, g_eswatj,  g_eswa,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "ESWAT Cyber Police (J) [!]", 0 )


GAME( 1900, g_wher,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Heroes (U) [!]", 0 )
GAME( 1900, g_wheroj,   g_wher,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "World Heroes (J) [!]", 0 )


GAME( 1900, g_uzuke,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Uzu Keobukseon (K) [!]", 0 )


GAME( 1900, g_f1wc,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "F1 World Championship (U) [!]", 0 )
GAME( 1900, g_f1wlce,    g_f1wc,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "F1 World Championship (E) [!]", 0 )


GAME( 1900, g_ferias,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ferias Frustradas do Pica-Pau (B) [!]", 0 )

GAME( 1900, g_fmas,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Fighting Masters (U) [!]", 0 )
GAME( 1900, g_fghmsj,  g_fmas,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Fighting Masters (J) [!]", 0 )

GAME( 1900, g_flin,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Flintstones, The (U) [c][!]", 0 )
GAME( 1900, g_flinte,  g_flin,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Flintstones, The (E) [c][!]", 0 )
GAME( 1900, g_flintj,  g_flin,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Flintstones, The (J) [c][!]", 0 )

GAME( 1900, g_fw,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Forgotten Worlds (W) (REV01) [!]", 0 )
GAME( 1900, g_fw00,    g_fw,     megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Forgotten Worlds (W) (REV00) [!]", 0 )


GAME( 1900, g_gfko,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "George Foreman's KO Boxing (U) [!]", 0 )
GAME( 1900, g_gfkobe,   g_gfko,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "George Foreman's KO Boxing (E) [!]", 0 )


GAME( 1900, g_hell,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Hellfire (U) [!]", 0 )
GAME( 1900, g_helfij,  g_hell,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Hellfire (J) [!]", 0 )
GAME( 1900, g_helfie,  g_hell,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Hellfire (E)", 0 )

GAME( 1900, g_hybrid,    0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Hybrid Front, The (J) [!]", 0 )
GAME( 1900, g_hybrip,    g_hybrid, megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Hybrid Front, The (J) (Prototype) [!]", 0 )

GAME( 1900, g_hydu,      0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Hyper Dunk - The Playoff Edition (E) [!]", 0 )
GAME( 1900, g_hdunkb,    g_hydu,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Hyper Dunk - The Playoff Edition (Beta)", 0 )
GAME( 1900, g_hdunkj,    g_hydu,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Hyper Dunk - The Playoff Edition (J) [!]", 0 )

GAME( 1900, g_indl,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Indiana Jones and the Last Crusade (U) [c][!]", 0 )
GAME( 1900, g_indlce,    g_indl,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Indiana Jones and the Last Crusade (E) [c][!]", 0 )

GAME( 1900, g_insx,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Insector X (U) [!]", 0 )
GAME( 1900, g_insxj,     g_insx,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Insector X (J) [!]", 0 )

GAME( 1900, g_jlp,       0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "J. League Pro Striker (J) (REV03) [c][!]", 0 )
GAME( 1900, g_jlps00,    g_jlp,    megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "J. League Pro Striker (J) (REV00) [c][!]", 0 )


GAME( 1900, g_jewl,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jewel Master (UE) [!]", 0 )
GAME( 1900, g_jewlj,     g_jewl,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Jewel Master (J) [c][!]", 0 )

GAME( 1900, g_jb11,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jordan vs Bird - Super One-on-One (UE) (REV01) [!]", 0 )
GAME( 1900, g_jb11j,     g_jb11,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Jordan vs Bird - Super One-on-One (J) [!]", 0 )
GAME( 1900, g_jb1100,    g_jb11,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Jordan vs Bird - Super One-on-One (UE) (REV00) [!]", 0 )

GAME( 1900, g_ksal,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "King Salmon - The Big Catch (U) [c][!]", 0 )
GAME( 1900, g_ksalmj,    g_ksal,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "King Salmon - The Big Catch (J) [c][!]", 0 )

GAME( 1900, g_lhx,       0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "LHX Attack Chopper (UE) [!]", 0 )
GAME( 1900, g_lhxj,      g_lhx,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "LHX Attack Chopper (J) [!]", 0 )

GAME( 1900, g_midr,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Midnight Resistance (U) [!]", 0 )
GAME( 1900, g_midrej,    g_midr,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Midnight Resistance (J) [!]", 0 )

GAME( 1900, g_m29,       0,        megadriv,    megadri6,    megadriv, ROT0,   "Unsorted", "Mig-29 Fighter Pilot (U) [!]", 0 )
GAME( 1900, g_mig29e,    g_m29,    megadpal,    megadri6,    megadrie, ROT0,   "Unsorted", "Mig-29 Fighter Pilot (E) [!]", 0 )

GAME( 1900, g_mike,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mike Ditka Power Football (U) [!]", 0 )
GAME( 1900, g_mdpfua,    g_mike,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mike Ditka Power Football (U) [a1][!]", 0 )

GAME( 1900, g_mlfoot,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mutant League Football (UE) [!]", 0 )
GAME( 1900, g_mutlfj,    g_mlfoot, megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Mutant League Football (J) [!]", 0 )

GAME( 1900, g_mysd,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mystic Defender (W) (REV01) [!]", 0 )
GAME( 1900, g_mysd00,    g_mysd,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Mystic Defender (W) (REV00) [!]", 0 )
GAME( 1900, g_kujaku,    g_mysd,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Kujaku Ou 2 (J) [!]", 0 )



GAME( 1900, g_olwg,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Olympic Winter Games - Lillehammer 94 (U) [c][!]", 0 )
GAME( 1900, g_olwge,   g_olwg,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Olympic Winter Games - Lillehammer 94 (E) [c][!]", 0 )
GAME( 1900, g_olwgj,   g_olwg,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Olympic Winter Games - Lillehammer 94 (J) [c][!]", 0 )

GAME( 1900, g_sbea,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Shadow of the Beast (UE) [!]", 0 )
GAME( 1900, g_shbeaj,  g_sbea,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Shadow of the Beast (J) [!]", 0 )

GAME( 1900, g_showd,   0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Show do Milhao (B) [!]", 0 )

GAME( 1900, g_showd2,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Show do Milhao Volume 2 (B) [!]", 0 )
GAME( 1900, g_shwd2a,  g_showd2, megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Show do Milhao Volume 2 (B) [a1][!]", 0 )

GAME( 1900, g_sorckd,  0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sorcerer's Kingdom (U) (REV01) [!]", 0 )
GAME( 1900, g_sork00,  g_sorckd, megadriv,    megadriv,    megadriv_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sorcerer's Kingdom (U) (REV00)", 0 )
GAME( 1900, g_sorkin,  g_sorckd, megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Sorcer Kingdom (J) [c][!]", 0 )


GAME( 1900, g_shar,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Space Harrier II (UE) [!]", 0 )
GAME( 1900, g_shar2j,  g_shar,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Space Harrier II (J) [!]", 0 )

GAME( 1900, g_sprk,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sparkster (U) [!]", 0 )
GAME( 1900, g_sparke,  g_sprk,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Sparkster (E) [!]", 0 )
GAME( 1900, g_sparkj,  g_sprk,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Sparkster (J) [!]", 0 )


GAME( 1900, g_starcj,  0,        megadriv,    megadriv,    megadrij_backup_0x200000_0x4000, ROT0,   "Unsorted", "Star Cruiser (J) [!]", 0 )

GAME( 1900, g_spb2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Speed Ball 2 - Brutal Deluxe (U) [c][!]", 0 )
GAME( 1900, g_sbal2e,  g_spb2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Speed Ball 2 - Brutal Deluxe (E) [!]", 0 )

GAME( 1900, g_stalon,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Steel Talons (UE) [!]", 0 )
GAME( 1900, g_stalj,   g_stalon, megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Steel Talons (J) [!]", 0 )

GAME( 1900, g_stol,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Stormlord (U) [!]", 0 )
GAME( 1900, g_slordj,  g_stol,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Stormlord (J) [!]", 0 )

GAME( 1900, g_sfli,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Starflight (UE) (REV01) [!]", 0 )
GAME( 1900, g_strf00,  g_sfli,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Starflight (UE) (REV00) [!]", 0 )

GAME( 1900, g_ssri,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sunset Riders (U) [!]", 0 )
GAME( 1900, g_ssride,  g_ssri,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Sunset Riders (E) [!]", 0 )


GAME( 1900, g_pachin,        0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Pachinko Canyon (J) [!]", 0 )


GAME( 1900, g_pst,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pete Sampras Tennis (UE) (REV00) (J-Cart) [c][!]", 0 )
GAME( 1900, g_pste03,   g_pst,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Pete Sampras Tennis (E) (REV03) (J-Cart) [!]", 0 )

GAME( 1900, g_rast,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rastan Saga II (U) [!]", 0 )
GAME( 1900, g_rastj,    g_rast,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Rastan Saga II (J) [!]", 0 )

GAME( 1900, g_rbls,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Road Blasters (U) [!]", 0 )
GAME( 1900, g_roadbj,   g_rbls,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Road Blasters (J) [!]", 0 )

GAME( 1900, g_rrr,      0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rock n' Roll Racing (U) [!]", 0 )
GAME( 1900, g_rnrre,    g_rrr,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Rock n' Roll Racing (E) [!]", 0 )

GAME( 1900, g_rkni,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rocket Knight Adventures (U) [!]", 0 )
GAME( 1900, g_rkadve,  g_rkni,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Rocket Knight Adventures (E) [!]", 0 )
GAME( 1900, g_rkadvj,  g_rkni,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Rocket Knight Adventures (J) [!]", 0 )


GAME( 1900, g_rth2,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rolling Thunder 2 (U) [c][!]", 0 )
GAME( 1900, g_rthn2e,   g_rth2,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Rolling Thunder 2 (E) [c][!]", 0 )
GAME( 1900, g_rthn2j,   g_rth2,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Rolling Thunder 2 (J) [c][!]", 0 )

GAME( 1900, g_rth3,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Rolling Thunder 3 (U) [c][!]", 0 )





GAME( 1900, g_telerb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Telebradesco Residencia (B) [!]", 0 )

GAME( 1900, g_tfox,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Thunder Fox (U) [c][!]", 0 )
GAME( 1900, g_tfoxj,    g_tfox,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Thunder Fox (J) [c][!]", 0 )


GAME( 1900, g_todd,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Todd's Adventures in Slime World (U) [!]", 0 )
GAME( 1900, g_toddj,    g_todd,   megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Todd's Adventures in Slime World (J) [!]", 0 )

GAME( 1900, g_tomj,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tom and Jerry - Frantic Antics (U) (1994) [!]", 0 )
GAME( 1900, g_tajua,   g_tomj,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Tom and Jerry - Frantic Antics (U) (1993) [!]", 0 )



GAME( 1900, g_babyb2,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Baby's Day Out (Beta 2)", 0 )
GAME( 1900, g_babyb1,        g_babyb2, megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Baby's Day Out (Beta 1)", 0 )


GAME( 1900, g_barbvb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Barbie Vacation Adventure (Beta)", 0 )

GAME( 1900, g_bat,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Batman (U) [!]", 0 )
GAME( 1900, g_batmnj,  g_bat,    megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Batman (J) [!]", 0 )
GAME( 1900, g_batme,   g_bat,    megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Batman (E)", 0 )

GAME( 1900, g_bzerot,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Beyond Zero Tolerance (Beta)", 0 )
GAME( 1900, g_blocb2,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Blockbuster World Video Game Championship II (U)", GAME_NOT_WORKING ) // eeprom



GAME( 1900, g_chuck,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Chuck's Excellent Art Tool Animator (U)", 0 )



GAME( 1900, g_comacb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Combat Aces (Beta)", 0 )
GAME( 1900, g_congob,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Congo - The Game (Beta)", 0 )


GAME( 1900, g_dwctb1,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Davis Cup World Tour Tennis 2 (Beta 1)", 0 )
GAME( 1900, g_dwctb2,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Davis Cup World Tour Tennis 2 (Beta 2)", 0 )
GAME( 1900, g_dwctb,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Davis Cup World Tour Tennis (Beta)", 0 )
GAME( 1900, g_demomb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Demolition Man (Beta)", 0 )
GAME( 1900, g_dominu,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dominus (Beta)", 0 )

GAME( 1900, g_dune,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Dune - The Battle for Arrakis (U) [!]", 0 )
GAME( 1900, g_duneg,   g_dune,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Dune - Der Kampf um den Wuestenplaneten (G)", 0 )
GAME( 1900, g_dunee,   g_dune,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Dune - The Battle for Arrakis (E)", 0 )


GAME( 1900, g_dynabr,        0,        megadriv,    megadriv,    megadrij, ROT0,   "Unsorted", "Dyna Brothers (J) (REV01)", 0 )






GAME( 1900, g_frgp,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ferrari Grand Prix Challenge (U) [!]", 0 )
GAME( 1900, g_fergpb,   g_frgp,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ferrari Grand Prix Challenge (Beta)", 0 )




GAME( 1900, g_icftd,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "It Came From The Desert (Beta)", 0 )






GAME( 1900, g_mima3b,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Might and Magic III - Isles of Terra (Beta)", 0 )




GAME( 1900, g_ngaidb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Ninja Gaiden (Beta)", 0 )

GAME( 1900, g_pbgl,    0,        megadriv,    megadriv,    megadriv_backup_0x200000_0x10000, ROT0,   "Unsorted", "Pebble Beach Golf Links (U) [!]", 0 )
GAME( 1900, g_pblbee,  g_pbgl,   megadpal,    megadriv,    megadrie_backup_0x200000_0x10000, ROT0,   "Unsorted", "Pebble Beach Golf Links (E)", 0 )
GAME( 1900, g_pblbej,  g_pbgl,   megadriv,    megadriv,    megadrij_backup_0x200000_0x10000, ROT0,   "Unsorted", "Pebble Beach no Hatou (J) [!]", 0 )

GAME( 1900, g_pink,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pink Goes to Hollywood (U) [!]", 0 )
GAME( 1900, g_pinkb,   g_pink,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Pink Goes to Hollywood (Beta)", 0 )



GAME( 1900, g_radr,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Radical Rex (U) [!]", 0 )
GAME( 1900, g_rrexe,   g_radr,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Radical Rex (E)", 0 )



GAME( 1900, g_scrabb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Scrabble (Beta)", 0 )

GAME( 1900, g_s2de,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Shanghai 2 - Dragon's Eye (U) [!]", 0 )
GAME( 1900, g_shan2b,   g_s2de,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Shanghai 2 - Dragon's Eye (Beta)", 0 )

GAME( 1900, g_soncrk,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Sonic and Crackers (Beta)", 0 )




GAME( 1900, g_wacrac,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Wacky Races (Beta)", 0 )

GAME( 1900, g_watrb,         0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Waterworld (Beta)", 0 )


GAME( 1900, g_wagh,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Williams Arcade's Greatest Hits (U) [!]", 0 )
GAME( 1900, g_waghe,   g_wagh,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Williams Arcade's Greatest Hits (E)", 0 )


GAME( 1900, g_wcs2,    0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Championship Soccer II (U) [!]", 0 )
GAME( 1900, g_wcs2b,   g_wcs2,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "World Championship Soccer II (Beta)", 0 )

GAME( 1900, g_yidy,     0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Young Indiana Jones - Instrument of Chaos (U) [!]", 0 )
GAME( 1900, g_yindyb,   g_yidy,   megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Young Indiana Jones - Instrument of Chaos (Beta)", 0 )

GAME( 1900, g_yindcb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Young Indiana Jones Chronicles (Beta)", 0 )

GAME( 1900, g_zombhb,        0,        megadriv,    megadriv,    megadriv, ROT0,   "Unsorted", "Zombie High (U) (Prototype)", 0 )

GAME( 1900, g_blcr,     0,        megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Brian Lara Cricket (E) [c][!]", 0 ) // sound is sensitive to timing
GAME( 1900, g_blcrb,    g_blcr,   megadpal,    megadriv,    megadrie, ROT0,   "Unsorted", "Brian Lara Cricket (Beta)", 0 )
GAME( 1900, g_princ2,  0,        megadriv,    megadriv,    megadriv, ROT0,   "Psygnosis", "Prince of Persia 2 (Prototype)", 0 )



/*

I should add a rom header check to look for the right values for these, even if i hardcode them.

ShimaPong notes:


Failed standard back up RAM

    * Story of Thor / Beyond Oasis (g_boas, g_sthorj, g_sthorf, g_sthorg, g_sthork, g_sthors, g_sthorb)
    * Evander Holyfield's Real Deal Boxing (g_ehrd) : Fails to boot with black screen
    * Honoo no Toukyuuji Dodge Danpei (g_dodg) : Fails to boot with black screen after SEGA logo
    * Ransei no Hasha (g_ransei) : You can't choose "SAVE" icon in-game
    * Tecmo Super Bowl (g_tsbw, g_tsbwlj, g_tsbwlu) : Return the menu when you start playing the game


Unconfirmed but the game has back up RAM
    * Ninja Burai Densetsu (g_nbd) : I don't have...
    * Power Monger (g_pmon, g_pmonj) : I have not played until a save point...
    * Wonder Boy V - Monster World III (g_wboy5) : I don't have...



*/



