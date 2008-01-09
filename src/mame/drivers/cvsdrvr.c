/***************************************************************************

    Century CVS System

    This file contains all game specific overrides

****************************************************************************/


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

#define CVS_BIOS 																							\
	ROM_REGION( 0x8000, REGION_CPU3, 0 )																	\
	ROM_LOAD( "5b.bin",      0x0000, 0x0800, CRC(f055a624) SHA1(5dfe89d7271092e665cdd5cd59d15a2b70f92f43) )	\
																											\
	ROM_REGION( 0x0820, REGION_PROMS, 0 )																	\
    ROM_LOAD( "82s185.10h",  0x0000, 0x0800, CRC(c205bca6) SHA1(ec9bd220e75f7b067ede6139763ef8aca0fb7a29) )	\
	ROM_LOAD( "82s123.10k",  0x0800, 0x0020, CRC(b5221cec) SHA1(71d9830b33b1a8140b0fe1a2ba8024ba8e6e48e0) )	\

#define CVS_ROM_REGION_SPEECH(name, len, hash)	\
	ROM_REGION( 0x1000, CVS_REGION_SPEECH, 0 )	\
	ROM_LOAD( name, 0x0000, len, hash )

#define ROM_LOAD_STAGGERED(name, offs, hash)		\
	ROM_LOAD( name, 0x0000 + offs, 0x0400, hash )	\
	ROM_CONTINUE(   0x2000 + offs, 0x0400 )			\
	ROM_CONTINUE(   0x4000 + offs, 0x0400 )			\
	ROM_CONTINUE(   0x6000 + offs, 0x0400 )


ROM_START( cvs )
	ROM_REGION( 0x8000, REGION_CPU1, ROMREGION_ERASE00 )

	ROM_REGION( 0x2000, REGION_CPU2, ROMREGION_ERASE00 )

	ROM_REGION( 0x1800, REGION_GFX1, ROMREGION_ERASE00 )

	CVS_BIOS
ROM_END

ROM_START( huncholy )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "ho-gp1.bin", 0x0000, CRC(4f17cda7) SHA1(ae6fe495c723042c6e060d4ada50aaef1019d5eb) )
	ROM_LOAD_STAGGERED( "ho-gp2.bin", 0x0400, CRC(70fa52c7) SHA1(179813fdc204870d72c0bfa8cd5dbf277e1f67c4) )
	ROM_LOAD_STAGGERED( "ho-gp3.bin", 0x0800, CRC(931934b1) SHA1(08fe5ad3459862246e9ea845abab4e01e1dbd62d) )
	ROM_LOAD_STAGGERED( "ho-gp4.bin", 0x0c00, CRC(af5cd501) SHA1(9a79b173aa41a82faa9f19210d3e18bfa6c593fa) )
	ROM_LOAD_STAGGERED( "ho-gp5.bin", 0x1000, CRC(658e8974) SHA1(30d0ada1cce99a842bad8f5a58630bc1b7048b03) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ho-sdp1.bin", 0x0000, 0x1000, CRC(3efb3ffd) SHA1(be4807c8b4fe23f2247aa3b6ac02285bee1a0520) )

	CVS_ROM_REGION_SPEECH( "ho-sp1.bin", 0x1000, CRC(3fd39b1e) SHA1(f5d0b2cfaeda994762403f039a6f7933c5525234) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "ho-cp1.bin",  0x0000, 0x0800, CRC(c6c73d46) SHA1(63aba92f77105fedf46337b591b074020bec05d0) )
	ROM_LOAD( "ho-cp2.bin",  0x0800, 0x0800, CRC(e596371c) SHA1(93a0d0ccdf830ae72d070b03b7e2222f4a737ead) )
	ROM_LOAD( "ho-cp3.bin",  0x1000, 0x0800, CRC(11fae1cf) SHA1(5ceabfb1ff1a6f76d1649512f57d7151f5258ecb) )

	CVS_BIOS
ROM_END

ROM_START( darkwar )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "dw-gp1.bin", 0x0000, CRC(f10ccf24) SHA1(f694a9016fc935798e5342598e4fd60fbdbc2829) )
	ROM_LOAD_STAGGERED( "dw-gp2.bin", 0x0400, CRC(b77d0483) SHA1(47d126b9ceaf07267c9078a342a860295320b01c) )
	ROM_LOAD_STAGGERED( "dw-gp3.bin", 0x0800, CRC(c01c3281) SHA1(3c272f424f8a35d08b58f203718b579c1abbe63f) )
	ROM_LOAD_STAGGERED( "dw-gp4.bin", 0x0c00, CRC(0b0bffaf) SHA1(48db78d86dc249fb4d7d93b79b2ac269a0c6698e) )
	ROM_LOAD_STAGGERED( "dw-gp5.bin", 0x1000, CRC(7fdbcaff) SHA1(db80d0d8690105ca72df359c1dc1a43952709111) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "dw-sdp1.bin", 0x0000, 0x0800, CRC(b385b669) SHA1(79621d3fb3eb4ea6fa8a733faa6f21edeacae186) )

	CVS_ROM_REGION_SPEECH( "dw-sp1.bin", 0x1000, CRC(ce815074) SHA1(105f24fb776131b30e35488cca29954298559518) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "dw-cp1.bin", 0x0000, 0x0800, CRC(7a0f9f3e) SHA1(0aa787923fbb614f15016d99c03093a59a0bfb88) )
	ROM_LOAD( "dw-cp2.bin", 0x0800, 0x0800, CRC(232e5120) SHA1(76e4d6d17e8108306761604bd56d6269bfc431e1) )
	ROM_LOAD( "dw-cp3.bin", 0x1000, 0x0800, CRC(573e0a17) SHA1(9c7991eac625b287bafb6cf722ffb405a9627e09) )

	CVS_BIOS
ROM_END

ROM_START( 8ball )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "8b-gp1.bin", 0x0000, CRC(1b4fb37f) SHA1(df6dd2766a3b70eec0bde0ae1932b35abdab3735) )
	ROM_LOAD_STAGGERED( "8b-gp2.bin", 0x0400, CRC(f193cdb5) SHA1(54fd1a10c1b9da0f9c4d190f95acc11b3c6e7907) )
	ROM_LOAD_STAGGERED( "8b-gp3.bin", 0x0800, CRC(191989bf) SHA1(dea129a4ed06aac453ab1fbbfae14d8048ef270d) )
	ROM_LOAD_STAGGERED( "8b-gp4.bin", 0x0c00, CRC(9c64519e) SHA1(9a5cad7ccf8f1f289da9a6de0edd4e6d4f0b12fb) )
	ROM_LOAD_STAGGERED( "8b-gp5.bin", 0x1000, CRC(c50d0f9d) SHA1(31b6ea6282fec96d9d2fb74129a215d10f12cc9b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "8b-sdp1.bin", 0x0000, 0x1000, CRC(a571daf4) SHA1(0db5b95db9da27216bbfa8fff84491a7755f9f1a) )

	CVS_ROM_REGION_SPEECH( "8b-sp1.bin", 0x0800, CRC(1ee167f3) SHA1(40c876a60832456a27108252ba0b9963f9fe70b0) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "8b-cp1.bin", 0x0000, 0x0800, CRC(c1f68754) SHA1(481c8e3dc35300f779b7925fa8a54320688dac54) )
	ROM_LOAD( "8b-cp2.bin", 0x0800, 0x0800, CRC(6ec1d711) SHA1(768df8e621a7b110a963c93402ee01b1c9009286) )
	ROM_LOAD( "8b-cp3.bin", 0x1000, 0x0800, CRC(4a9afce4) SHA1(187e5106aa2d0bdebf6ec9f2b7c2c2f67d47d221) )

	CVS_BIOS
ROM_END

ROM_START( 8ball1 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "8a-gp1.bin", 0x0000, CRC(b5d3b763) SHA1(23a01bcbd536ba7f773934ea9dedc7dd9f698100) )
	ROM_LOAD_STAGGERED( "8a-gp2.bin", 0x0400, CRC(5e4aa61a) SHA1(aefa79b4c63d1ac5cb000f2c7c5d06e85d58a547) )
	ROM_LOAD_STAGGERED( "8a-gp3.bin", 0x0800, CRC(3dc272fe) SHA1(303184f7c3557be91d6b8e62a9685080444a78c5) )
	ROM_LOAD_STAGGERED( "8a-gp4.bin", 0x0c00, CRC(33afedbf) SHA1(857c743fd81fbd439c204b6adb251db68465cfc3) )
	ROM_LOAD_STAGGERED( "8a-gp5.bin", 0x1000, CRC(b8b3f373) SHA1(e808db4dcac6d8a454e20b561bb4f3a3bb9c6200) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "8b-sdp1.bin", 0x0000, 0x1000, CRC(a571daf4) SHA1(0db5b95db9da27216bbfa8fff84491a7755f9f1a) )

	CVS_ROM_REGION_SPEECH( "8b-sp1.bin", 0x0800, CRC(1ee167f3) SHA1(40c876a60832456a27108252ba0b9963f9fe70b0) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "8a-cp1.bin", 0x0000, 0x0800, CRC(d9b36c16) SHA1(dbb496102fa2344f19b5d9a3eecdb29c433e4c08) )
	ROM_LOAD( "8a-cp2.bin", 0x0800, 0x0800, CRC(6f66f0ff) SHA1(1e91474973356e97f89b4d9093565747a8331f50) )
	ROM_LOAD( "8a-cp3.bin", 0x1000, 0x0800, CRC(baee8b17) SHA1(9f86f1d5903aeead17cc75dac8a2b892bb375dad) )

	CVS_BIOS
ROM_END

ROM_START( hunchbak )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "hb-gp1.bin", 0x0000, CRC(af801d54) SHA1(68e31561e98f7e2caa337dd764941d08f075b559) )
	ROM_LOAD_STAGGERED( "hb-gp2.bin", 0x0400, CRC(b448cc8e) SHA1(ed94f662c0e08a3a0aca073fbec29ae1fbd0328e) )
	ROM_LOAD_STAGGERED( "hb-gp3.bin", 0x0800, CRC(57c6ea7b) SHA1(8c3ba01ab1917a8c24180ed1c0011dbfed36d406) )
	ROM_LOAD_STAGGERED( "hb-gp4.bin", 0x0c00, CRC(7f91287b) SHA1(9383d885c142417de73879905cbce272ba9514c7) )
	ROM_LOAD_STAGGERED( "hb-gp5.bin", 0x1000, CRC(1dd5755c) SHA1(b1e158d52bd9a238e3e32ed3024e495df2292dcb) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "hb-sdp1.bin", 0x0000, 0x1000, CRC(f9ba2854) SHA1(d041198e2e8b8c3e668bd1610310f8d25c5b1119) )

	CVS_ROM_REGION_SPEECH( "hb-sp1.bin", 0x0800, CRC(ed1cd201) SHA1(6cc3842dda1bfddc06ffb436c55d14276286bd67) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "hb-cp1.bin", 0x0000, 0x0800, CRC(f256b047) SHA1(02d79882bad37ffdd58ef478e2658a1369c32ebc) )
	ROM_LOAD( "hb-cp2.bin", 0x0800, 0x0800, CRC(b870c64f) SHA1(ce4f8de87568782ce02bba754edff85df7f5c393) )
	ROM_LOAD( "hb-cp3.bin", 0x1000, 0x0800, CRC(9a7dab88) SHA1(cd39a9d4f982a7f49c478db1408d7e07335f2ddc) )

	CVS_BIOS
ROM_END

ROM_START( hunchbka )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "1b.bin", 0x0000, CRC(c816860b) SHA1(1109639645496d4644564d21c816b8baf8c84cf7) )
	ROM_LOAD_STAGGERED( "2a.bin", 0x0400, CRC(cab1e524) SHA1(c3fd7ac9ce5893fd2602a15ad0f6e3267a4ca122) )
	ROM_LOAD_STAGGERED( "3a.bin", 0x0800, CRC(b2adcfeb) SHA1(3090e2c6b945857c1e48dea395015a05c6165cd9) )
	ROM_LOAD_STAGGERED( "4c.bin", 0x0c00, CRC(229a8b71) SHA1(ea3815eb69d4927da356eada0add8382735feb48) )
	ROM_LOAD_STAGGERED( "5a.bin", 0x1000, CRC(cb4f0313) SHA1(1ef63cbe62e7a54d45e0afbc398c9d9b601e6403) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "hb-sdp1.bin", 0x0000, 0x1000, CRC(f9ba2854) SHA1(d041198e2e8b8c3e668bd1610310f8d25c5b1119) )

	CVS_ROM_REGION_SPEECH( "hb-sp1.bin", 0x0800, CRC(ed1cd201) SHA1(6cc3842dda1bfddc06ffb436c55d14276286bd67) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "hb-cp1.bin", 0x0000, 0x0800, CRC(f256b047) SHA1(02d79882bad37ffdd58ef478e2658a1369c32ebc) )
	ROM_LOAD( "hb-cp2.bin", 0x0800, 0x0800, CRC(b870c64f) SHA1(ce4f8de87568782ce02bba754edff85df7f5c393) )
	ROM_LOAD( "hb-cp3.bin", 0x1000, 0x0800, CRC(9a7dab88) SHA1(cd39a9d4f982a7f49c478db1408d7e07335f2ddc) )

	CVS_BIOS
ROM_END

ROM_START( wallst )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "ws-gp1.bin", 0x0000, CRC(bdac81b6) SHA1(6ce865d8902e815742a9ecf10d6f9495f376dede) )
	ROM_LOAD_STAGGERED( "ws-gp2.bin", 0x0400, CRC(9ca67cdd) SHA1(575a4d8d037d2a3c07a8f49d93c7cf6781349ec1) )
	ROM_LOAD_STAGGERED( "ws-gp3.bin", 0x0800, CRC(c2f407f2) SHA1(8208064fd0138a6ccacf03275b8d28793245bfd9) )
	ROM_LOAD_STAGGERED( "ws-gp4.bin", 0x0c00, CRC(1e4b2fe1) SHA1(28eda70cc9cf619452729092e68734ab1a5dc7fb) )
	ROM_LOAD_STAGGERED( "ws-gp5.bin", 0x1000, CRC(eec7bfd0) SHA1(6485e9e2e1624118e38892e74f80431820fd9672) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ws-sdp1.bin", 0x0000, 0x1000, CRC(faed2ac0) SHA1(c2c48e24a560d918531e5c17fb109d68bdec850f) )

	CVS_ROM_REGION_SPEECH( "ws-sp1.bin",  0x0800, CRC(84b72637) SHA1(9c5834320f39545403839fb7088c37177a6c8861) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "ws-cp1.bin", 0x0000, 0x0800, CRC(5aca11df) SHA1(5ef815b5b09445515ff8b958c4ea29f1a221cee1) )
	ROM_LOAD( "ws-cp2.bin", 0x0800, 0x0800, CRC(ca530d85) SHA1(e5a78667c3583d06d8387848323b11e4a91091ec) )
	ROM_LOAD( "ws-cp3.bin", 0x1000, 0x0800, CRC(1e0225d6) SHA1(410795046c64c24de6711b167315308808b54291) )

	CVS_BIOS
ROM_END

ROM_START( dazzler )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "dz-gp1.bin", 0x0000, CRC(2c5d75de) SHA1(d121de662e95f2fc362e367cef57e5e70bafd197) )
	ROM_LOAD_STAGGERED( "dz-gp2.bin", 0x0400, CRC(d0db80d6) SHA1(ca57d3a1d516e0afd750a8f05ae51d4ddee60ca0) )
	ROM_LOAD_STAGGERED( "dz-gp3.bin", 0x0800, CRC(d5f07796) SHA1(110bb0e1613db3634513e8456770dd9d43ad7d34) )
	ROM_LOAD_STAGGERED( "dz-gp4.bin", 0x0c00, CRC(84e41a46) SHA1(a1c1fd9ecacf3357f5c7916cf05dc0b79e975137) )
	ROM_LOAD_STAGGERED( "dz-gp5.bin", 0x1000, CRC(2ae59c41) SHA1(a17e9535409e9e91c41f26a3543f44f20c1b07a5) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "dz-sdp1.bin", 0x0000, 0x1000, CRC(89847352) SHA1(54037a4d95958c4c3383467d7f4c2c9416b2eb4a) )

	CVS_ROM_REGION_SPEECH( "dz-sp1.bin", 0x0800, CRC(25da1fc1) SHA1(c14717ec3399ce7dc47a9d42c8ac8f585db770e9) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "dz-cp1.bin", 0x0000, 0x0800, CRC(0a8a9034) SHA1(9df3d4f387bd5ce3d3580ba678aeda1b65634ac2) )
	ROM_LOAD( "dz-cp2.bin", 0x0800, 0x0800, CRC(3868dd82) SHA1(844584c5a80fb8f1797b4aa4e22024e75726293d) )
	ROM_LOAD( "dz-cp3.bin", 0x1000, 0x0800, CRC(755d9ed2) SHA1(a7165a1d12a5a81d8bb941d8ad073e2097c90beb) )

	CVS_BIOS
ROM_END

ROM_START( radarzon )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "rd-gp1.bin", 0x0000, CRC(775786ba) SHA1(5ad0f4e774821a7ed73615118ea42132d3b5424b) )
	ROM_LOAD_STAGGERED( "rd-gp2.bin", 0x0400, CRC(9f6be426) SHA1(24b6cf3d826f3aec0e928881f259a5bc6229232b) )
	ROM_LOAD_STAGGERED( "rd-gp3.bin", 0x0800, CRC(61d11b29) SHA1(fe321c1c912b93bbb098d591e5c4ed0b5b72c88e) )
	ROM_LOAD_STAGGERED( "rd-gp4.bin", 0x0c00, CRC(2fbc778c) SHA1(e45ba08156cf03a1c4a1bdfb8569476d0eb05847) )
	ROM_LOAD_STAGGERED( "rd-gp5.bin", 0x1000, CRC(692a99d5) SHA1(122ae802914cb9a41713536f9030cd9377cf3468) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "rd-sdp1.bin", 0x0000, 0x0800, CRC(cd5aea6d) SHA1(f7545b87e71e3108c0dec24a4e91620d006e0602) )

	CVS_ROM_REGION_SPEECH( "rd-sp1.bin", 0x0800, CRC(43b17734) SHA1(59960f0c48ed24cedb4b4655f97f6f1fdac4445e) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_BIOS
ROM_END

ROM_START( radarzn1 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "r1-gp1.bin", 0x0000, CRC(7c73c21f) SHA1(1113025ea16cfcc500b9624a031f3d25290db163) )
	ROM_LOAD_STAGGERED( "r1-gp2.bin", 0x0400, CRC(dedbd2ce) SHA1(ef80bf1b4a9561ad7f54e795c78e72664abf0501) )
	ROM_LOAD_STAGGERED( "r1-gp3.bin", 0x0800, CRC(966a49e7) SHA1(6c769ac12fbfb65184131f1ab16240e422125c04) )
	ROM_LOAD_STAGGERED( "r1-gp4.bin", 0x0c00, CRC(f3175bee) SHA1(f4927eea856ae56b1854263666a48e2cfb3ab60d) )
	ROM_LOAD_STAGGERED( "r1-gp5.bin", 0x1000, CRC(7484927b) SHA1(89a67baa91075d2777f2ecd1667ed79175ad57ca) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "rd-sdp1.bin", 0x0000, 0x0800, CRC(cd5aea6d) SHA1(f7545b87e71e3108c0dec24a4e91620d006e0602) )

	CVS_ROM_REGION_SPEECH( "rd-sp1.bin", 0x0800, CRC(43b17734) SHA1(59960f0c48ed24cedb4b4655f97f6f1fdac4445e) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_BIOS
ROM_END

ROM_START( radarznt )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "rt-gp1.bin", 0x0000, CRC(43573974) SHA1(854fe7022e9bdd94bb119c014156e9ffdb6682fa) )
	ROM_LOAD_STAGGERED( "rt-gp2.bin", 0x0400, CRC(257a11ce) SHA1(ca7f9260d9879ebce202f83a41838cb6dc9a6480) )
	ROM_LOAD_STAGGERED( "rt-gp3.bin", 0x0800, CRC(e00f3552) SHA1(156765809e4016527039e3d5cc1c320cfce06834) )
	ROM_LOAD_STAGGERED( "rt-gp4.bin", 0x0c00, CRC(d1e824ac) SHA1(f996813f02d32ddcde7f394740bdb3444eacda76) )
	ROM_LOAD_STAGGERED( "rt-gp5.bin", 0x1000, CRC(bc770af8) SHA1(79599b5f2f4d692986862076be1d487b45783c00) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "rd-sdp1.bin", 0x0000, 0x0800, CRC(cd5aea6d) SHA1(f7545b87e71e3108c0dec24a4e91620d006e0602) )

	CVS_ROM_REGION_SPEECH( "rd-sp1.bin", 0x0800, CRC(43b17734) SHA1(59960f0c48ed24cedb4b4655f97f6f1fdac4445e) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_BIOS
ROM_END

ROM_START( outline )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "rt-gp1.bin", 0x0000, CRC(43573974) SHA1(854fe7022e9bdd94bb119c014156e9ffdb6682fa) )
	ROM_LOAD_STAGGERED( "rt-gp2.bin", 0x0400, CRC(257a11ce) SHA1(ca7f9260d9879ebce202f83a41838cb6dc9a6480) )
	ROM_LOAD_STAGGERED( "ot-gp3.bin", 0x0800, CRC(699489e1) SHA1(d4b21c294254ee0a451c29ac91028582a52f5ba3) )
	ROM_LOAD_STAGGERED( "ot-gp4.bin", 0x0c00, CRC(c94aca17) SHA1(ea4ab93c52fee37afc7033b4b2acddcdce308f6b) )
	ROM_LOAD_STAGGERED( "ot-gp5.bin", 0x1000, CRC(154712f4) SHA1(90f69e30e1c1d2348d6644406d83d2b2bcfe8171) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ot-sdp1.bin", 0x0000, 0x0800, CRC(739066a9) SHA1(7b3ba8a163d341931bc0385c298d2061fa75e644) )

	CVS_ROM_REGION_SPEECH( "ot-sp1.bin", 0x1000, CRC(fa21422a) SHA1(a75d13455c65e5a77db02fc87f0c112e329d0d6d) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_BIOS
ROM_END

ROM_START( goldbug )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "gb-gp1.bin", 0x0000, CRC(8deb7761) SHA1(35f27fb6b5e3f76ddaf2c074b3391931e679df6e) )
	ROM_LOAD_STAGGERED( "gb-gp2.bin", 0x0400, CRC(135036c1) SHA1(9868eae2486687772bf0bf71b82e461a882ae1ab) )
	ROM_LOAD_STAGGERED( "gb-gp3.bin", 0x0800, CRC(d48b1090) SHA1(b3cbfeb4fc2bf1bbe0befab793fcc5e7e6ff804c) )
	ROM_LOAD_STAGGERED( "gb-gp4.bin", 0x0c00, CRC(c8053205) SHA1(7f814b059f6b9c62e8a83c1753da5e8780b09411) )
	ROM_LOAD_STAGGERED( "gb-gp5.bin", 0x1000, CRC(eca17472) SHA1(25c4ca59b4c96a22bc42b41adbf3cc33373cf85e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "gb-sdp1.bin", 0x0000, 0x1000, CRC(c8a4b39d) SHA1(29fffaa12639f3b19db818ad374d09fbf9c7fb98) )

	CVS_ROM_REGION_SPEECH( "gb-sp1.bin", 0x0800, CRC(5d0205c3) SHA1(578937058d56e5c9fba8a2204ddbb59a6d23dec7) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "gb-cp1.bin", 0x0000, 0x0800, CRC(80e1ad5a) SHA1(0a577b0faffd9d6807c39175ce213f017a5cc7f8) )
	ROM_LOAD( "gb-cp2.bin", 0x0800, 0x0800, CRC(0a288b29) SHA1(0c6471a3517805a5c873857ff21ca94dfe91c24e) )
	ROM_LOAD( "gb-cp3.bin", 0x1000, 0x0800, CRC(e5bcf8cf) SHA1(7f53b8ee6f87e6c8761d2200e8194a7d16d8c7ac) )

	CVS_BIOS
ROM_END

ROM_START( diggerc )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "dig-gp1.bin", 0x0000, CRC(6a67662f) SHA1(70e9814259c1fbaf195004e1d8ce0ab1125d62d0) )
	ROM_LOAD_STAGGERED( "dig-gp2.bin", 0x0400, CRC(aa9f93b5) SHA1(e118ecb9160b7ba4a81b75b6cca56c0f01a9a3af) )
	ROM_LOAD_STAGGERED( "dig-gp3.bin", 0x0800, CRC(4aa4c87c) SHA1(bcbe291e2ca060ecc623702cba1f4189dfa9c105) )
	ROM_LOAD_STAGGERED( "dig-gp4.bin", 0x0c00, CRC(127e6520) SHA1(a4f1813a297616b7f864e235f40a432881fe252b) )
	ROM_LOAD_STAGGERED( "dig-gp5.bin", 0x1000, CRC(76786827) SHA1(43e33c47a42878a58d72051c1edeccf944db4a17) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "dig-sdp1.bin", 0x0000, 0x1000, CRC(f82e51f0) SHA1(52903c19cdf7754894cbae57a16533579737b3d5) )

	CVS_ROM_REGION_SPEECH( "dig-sp1.bin", 0x0800, CRC(db526ee1) SHA1(afe319e64350b0c54b72394294a6369c885fdb7f) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "dig-cp1.bin", 0x0000, 0x0800, CRC(ca30fb97) SHA1(148f3a6f20b1f256a73e7a1992262116d77cc0a8) )
	ROM_LOAD( "dig-cp2.bin", 0x0800, 0x0800, CRC(bed2334c) SHA1(c93902d01174e13fb9265194e5e44f67b38c5970) )
	ROM_LOAD( "dig-cp3.bin", 0x1000, 0x0800, CRC(46db9b65) SHA1(1c655b4611ab182b6e4a3cdd3ef930e0d4dad0d9) )

	CVS_BIOS
ROM_END

ROM_START( superbik )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "sb-gp1.bin", 0x0000, CRC(f0209700) SHA1(7843e8ebcbecb93814863ddd135f5acb0d481043) )
	ROM_LOAD_STAGGERED( "sb-gp2.bin", 0x0400, CRC(1956d687) SHA1(00e261c5b1e1414b45661310c47daeceb3d5f4bf) )
	ROM_LOAD_STAGGERED( "sb-gp3.bin", 0x0800, CRC(ceb27b75) SHA1(56fecc72746113a6611c18663d1b9e0e2daf57b4) )
	ROM_LOAD_STAGGERED( "sb-gp4.bin", 0x0c00, CRC(430b70b3) SHA1(207c4939331c1561d145cbee0538da072aa51f5b) )
	ROM_LOAD_STAGGERED( "sb-gp5.bin", 0x1000, CRC(013615a3) SHA1(1795a4dcc98255ad185503a99f48b7bacb5edc9d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "sb-sdp1.bin", 0x0000, 0x0800, CRC(e977c090) SHA1(24bd4165434c745c1514d49cc90bcb621fb3a0f8) )

	CVS_ROM_REGION_SPEECH( "sb-sp1.bin", 0x0800, CRC(0aeb9ccd) SHA1(e7123eed21e4e758bbe1cebfd5aad44a5de45c27) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "sb-cp1.bin", 0x0000, 0x0800, CRC(03ba7760) SHA1(4ed252e2c4ec7cea2199524f7c35a1dc7c44f8d8) )
	ROM_LOAD( "sb-cp2.bin", 0x0800, 0x0800, CRC(04de69f2) SHA1(3ef3b3c159d47230622b6cc45baad8737bd93a90) )
	ROM_LOAD( "sb-cp3.bin", 0x1000, 0x0800, CRC(bb7d0b9a) SHA1(94c72d6961204be9cab351ac854ac9c69b51e79a) )

	CVS_BIOS
ROM_END

ROM_START( hero )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "hr-gp1.bin", 0x0000, CRC(82f39788) SHA1(44217dc2312d10fceeb35adf3999cd6f240b60be) )
	ROM_LOAD_STAGGERED( "hr-gp2.bin", 0x0400, CRC(79607812) SHA1(eaab829a2f5bcb8ec92c3f4122cffae31a4a77cb) )
	ROM_LOAD_STAGGERED( "hr-gp3.bin", 0x0800, CRC(2902715c) SHA1(cf63f72681d1dcbdabdf7673ad8f61b5969e4bd1) )
	ROM_LOAD_STAGGERED( "hr-gp4.bin", 0x0c00, CRC(696d2f8e) SHA1(73dd57f0f84e37ae707a89e17253aa3dd0c8b48b) )
	ROM_LOAD_STAGGERED( "hr-gp5.bin", 0x1000, CRC(936a4ba6) SHA1(86cddcfafbd93dcdad3a1f26e280ceb96f779ab0) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "hr-sdp1.bin", 0x0000, 0x0800, CRC(c34ecf79) SHA1(07c96283410b1e7401140094db95800708cf310f) )

	CVS_ROM_REGION_SPEECH( "hr-sp1.bin", 0x0800, CRC(a5c33cb1) SHA1(447ffb193b0dc4985bae5d8c214a893afd08664b) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "hr-cp1.bin", 0x0000, 0x0800, CRC(2d201496) SHA1(f195aa1b231a0e1752c7da824a10321f0527f8c9) )
	ROM_LOAD( "hr-cp2.bin", 0x0800, 0x0800, CRC(21b61fe3) SHA1(31882003f0557ffc4ec38ae6ee07b5d294b4162c) )
	ROM_LOAD( "hr-cp3.bin", 0x1000, 0x0800, CRC(9c8e3f9e) SHA1(9d949a4d12b45da12b434677670b2b109568564a) )

	CVS_BIOS
ROM_END

ROM_START( logger )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "lg-gp1.bin", 0x0000, CRC(0022b9ed) SHA1(4b94d2663f802a8140e8eae1b66ee78fdfa654f5) )
	ROM_LOAD_STAGGERED( "lg-gp2.bin", 0x0400, CRC(23c5c8dc) SHA1(37fb6a62cb798d96de20078fe4a3af74a2be0e66) )
	ROM_LOAD_STAGGERED( "lg-gp3.bin", 0x0800, CRC(f9288f74) SHA1(8bb588194186fc0e0c2d61ed2746542c978ebb76) )
	ROM_LOAD_STAGGERED( "lg-gp4.bin", 0x0c00, CRC(e52ef7bf) SHA1(df5509b6847d6b9520a9d83b15083546898a981e) )
	ROM_LOAD_STAGGERED( "lg-gp5.bin", 0x1000, CRC(4ee04359) SHA1(a592d4b280ac0ad5f06d68a7809092548261f123) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "lg-sdp1.bin", 0x0000, 0x1000, CRC(5af8da17) SHA1(357f02cdf38c6659aca51fa0a8534542fc29623c) )

	CVS_ROM_REGION_SPEECH( "lg-sp1.bin", 0x0800, CRC(74f67815) SHA1(6a26a16c27a7e4d58b611e5127115005a60cff91) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "lg-cp1.bin", 0x0000, 0x0800, CRC(e4ede80e) SHA1(62f2bc78106a057b6a8420d40421908df609bf29) )
	ROM_LOAD( "lg-cp2.bin", 0x0800, 0x0800, CRC(d3de8e5b) SHA1(f95320e001869c42e51195d9cc11e4f2555e153f) )
	ROM_LOAD( "lg-cp3.bin", 0x1000, 0x0800, CRC(9b8d1031) SHA1(87ef12aeae80cc0f240dead651c6222848f8dccc) )

	CVS_BIOS
ROM_END

ROM_START( cosmos )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "cs-gp1.bin", 0x0000, CRC(7eb96ddf) SHA1(f7456ee1ace03ab98c4e8128d375464122c4df01) )
	ROM_LOAD_STAGGERED( "cs-gp2.bin", 0x0400, CRC(6975a8f7) SHA1(13192d4eedd843c0c1d7e5c54a3086f71b09fbcb) )
	ROM_LOAD_STAGGERED( "cs-gp3.bin", 0x0800, CRC(76904b13) SHA1(de219999e4a1b72142e71ea707b6250f4732ccb3) )
	ROM_LOAD_STAGGERED( "cs-gp4.bin", 0x0c00, CRC(bdc89719) SHA1(668267d0b05990ff83a9e38a62950d3d725a53b3) )
	ROM_LOAD_STAGGERED( "cs-gp5.bin", 0x1000, CRC(94be44ea) SHA1(e496ea79d177c6d2d79d59f7d45c86b547469c6f) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "cs-sdp1.bin", 0x0000, 0x0800, CRC(b385b669) SHA1(79621d3fb3eb4ea6fa8a733faa6f21edeacae186) )

	CVS_ROM_REGION_SPEECH( "cs-sp1.bin", 0x1000, CRC(3c7fe86d) SHA1(9ae0b63b231a7092820650a196cde60588bc6b58) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "cs-cp1.bin", 0x0000, 0x0800, CRC(6a48c898) SHA1(c27f7bcdb2fe042ec52d1b9b4b9a4e47c288862d) )
	ROM_LOAD( "cs-cp2.bin", 0x0800, 0x0800, CRC(db0dfd8c) SHA1(f2b0dd43f0e514fdae54e4066606187f45b98e38) )
	ROM_LOAD( "cs-cp3.bin", 0x1000, 0x0800, CRC(01eee875) SHA1(6c41d716b5795f085229d855518862fb85f395a4) )

	CVS_BIOS
ROM_END

ROM_START( heartatk )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "ha-gp1.bin", 0x0000, CRC(e8297c23) SHA1(e79ae7e99f904afe90b43a54df7b0e257d65ac0b) )
	ROM_LOAD_STAGGERED( "ha-gp2.bin", 0x0400, CRC(f7632afc) SHA1(ebfc6e12c8b5078e8c448aa25d9de9d39c0baa5e) )
	ROM_LOAD_STAGGERED( "ha-gp3.bin", 0x0800, CRC(a9ce3c6a) SHA1(86ddb27c1c132f3cf5ad4268ea9a458e0da23677) )
	ROM_LOAD_STAGGERED( "ha-gp4.bin", 0x0c00, CRC(090f30a9) SHA1(acd6b0c7358bf4664d0de668853076326e82fd04) )
	ROM_LOAD_STAGGERED( "ha-gp5.bin", 0x1000, CRC(163b3d2d) SHA1(275275b54533e0ce2df6d189619be05a99c68b6d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ha-sdp1.bin", 0x0000, 0x1000, CRC(b9c466a0) SHA1(f28c21a15cf6d52123ed7feac4eea2a42ea5e93d) )

	CVS_ROM_REGION_SPEECH( "ha-sp1.bin", 0x1000, CRC(fa21422a) SHA1(a75d13455c65e5a77db02fc87f0c112e329d0d6d) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "ha-cp1.bin", 0x0000, 0x0800, CRC(2d0f6d13) SHA1(55e45eaf1bf24a7a78a2f34ffc0d99a4c191d138) )
	ROM_LOAD( "ha-cp2.bin", 0x0800, 0x0800, CRC(7f5671bd) SHA1(7f4ae92a96c5a847c113f6f7e8d67d3e5ee0bcb0) )
	ROM_LOAD( "ha-cp3.bin", 0x1000, 0x0800, CRC(35b05ab4) SHA1(f336eb0c674c3d52e84be0f37b70953cce6112dc) )

	CVS_BIOS
ROM_END

ROM_START( spacefrt )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "sf-gp1.bin", 0x0000, CRC(1158fc3a) SHA1(c1f470324b6ec65c3061f78a6ff8620154f20c09) )
	ROM_LOAD_STAGGERED( "sf-gp2.bin", 0x0400, CRC(8b4e1582) SHA1(5b92082d67f32197c0c61ddd8e1e3feb742195f4) )
	ROM_LOAD_STAGGERED( "sf-gp3.bin", 0x0800, CRC(48f05102) SHA1(72d40cdd0bbc4cfeb6ddf550de0dafc61270d382) )
	ROM_LOAD_STAGGERED( "sf-gp4.bin", 0x0c00, CRC(c5b14631) SHA1(360bed649185a090f7c96adadd7f045ef574865a) )
	ROM_LOAD_STAGGERED( "sf-gp5.bin", 0x1000, CRC(d7eca1b6) SHA1(8444e61827f0153d04c4f9c08416e7ab753d6918) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "sf-sdp1.bin", 0x0000, 0x0800, CRC(339a327f) SHA1(940887cd4660e37537fd9b57aa1ec3a4717ea0cf) )

	CVS_ROM_REGION_SPEECH( "sf-sp1.bin", 0x1000, CRC(c5628d30) SHA1(d29a5852a1762cbd5f3eba29ae2bf49b3a26f894) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "sf-cp1.bin", 0x0000, 0x0800, CRC(da194a68) SHA1(4215267e91644cf1e1f32f898bc9562bfba711f3) )
	ROM_LOAD( "sf-cp2.bin", 0x0800, 0x0800, CRC(b96977c7) SHA1(8f0fab044f16787bce83562e2b22d962d0a2c209) )
	ROM_LOAD( "sf-cp3.bin", 0x1000, 0x0800, CRC(f5d67b9a) SHA1(a492b41c53b1f28ac5f70969e5f06afa948c1a7d) )

	CVS_BIOS
ROM_END

ROM_START( raiders )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_STAGGERED( "raid4-5a.bin", 0x0000, CRC(1a92a5aa) SHA1(7f6dbbc0ac5ee2ba3efb3a9120cbee89c659b712) )
	ROM_LOAD_STAGGERED( "raid4-4b.bin", 0x0400, CRC(da69e4b9) SHA1(8a2c4130a5db2cd7dbadb220440bb94ed4513bca) )
	ROM_LOAD_STAGGERED( "raid4-3b.bin", 0x0800, CRC(ca794f92) SHA1(f281d88ffc6b7a84f9bcabc06eed99b8ae045eec) )
	ROM_LOAD_STAGGERED( "raid4-2c.bin", 0x0c00, CRC(9de2c085) SHA1(be6157904afd0dc6ef8d97ec01a9557021ac8f3a) )
	ROM_LOAD_STAGGERED( "raid4-1a.bin", 0x1000, CRC(f4db83ed) SHA1(7d519dc628c93f153ccede85e3cf77c012430f38) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "raidr1-6.bin", 0x0000, 0x0800, CRC(6f827e49) SHA1(4fb272616b60fcd468ed4074b94125e30aa46fd3) )

	CVS_ROM_REGION_SPEECH( "raidr1-8.bin", 0x0800, CRC(b6b90d2e) SHA1(a966fa208b72aec358b7fb277e603e47b6984aa7) )

	ROM_REGION( 0x1800, REGION_GFX1, 0 )
	ROM_LOAD( "raid4-11.bin", 0x0000, 0x0800, CRC(5eb7143b) SHA1(a19e803c15593b37ae2e61789f6e16f319620a37) )
	ROM_LOAD( "raid4-10.bin", 0x0800, 0x0800, CRC(391948a4) SHA1(7e20ad4f7e5bf7ad5dcb08ba6475313e2b8b1f03) )
	ROM_LOAD( "raid4-9b.bin", 0x1000, 0x0800, CRC(fecfde80) SHA1(23ea63080b8292fb00a743743cdff1a7ad0a8c6d) )

	CVS_BIOS
ROM_END



/*************************************
 *
 *  Game specific initalization
 *
 *************************************/

static DRIVER_INIT( spacefrt )
{
	/* patch out 2nd character mode change */
	memory_region(REGION_CPU1)[0x0260] = 0xc0;
	memory_region(REGION_CPU1)[0x0261] = 0xc0;
}


static DRIVER_INIT( cosmos )
{
	/* patch out 2nd character mode change */
	memory_region(REGION_CPU1)[0x0357] = 0xc0;
	memory_region(REGION_CPU1)[0x0358] = 0xc0;
}


static DRIVER_INIT( goldbug )
{
	/* redirect calls to real memory bank */
	memory_region(REGION_CPU1)[0x4347] = 0x1e;
	memory_region(REGION_CPU1)[0x436a] = 0x1e;
}


static DRIVER_INIT( huncholy )
{
	/* patch out protection */
	memory_region(REGION_CPU1)[0x0082] = 0xc0;
	memory_region(REGION_CPU1)[0x0083] = 0xc0;
	memory_region(REGION_CPU1)[0x0084] = 0xc0;
	memory_region(REGION_CPU1)[0x00b7] = 0xc0;
	memory_region(REGION_CPU1)[0x00b8] = 0xc0;
	memory_region(REGION_CPU1)[0x00b9] = 0xc0;
	memory_region(REGION_CPU1)[0x00d9] = 0xc0;
	memory_region(REGION_CPU1)[0x00da] = 0xc0;
	memory_region(REGION_CPU1)[0x00db] = 0xc0;
	memory_region(REGION_CPU1)[0x4456] = 0xc0;
	memory_region(REGION_CPU1)[0x4457] = 0xc0;
	memory_region(REGION_CPU1)[0x4458] = 0xc0;
}


static DRIVER_INIT( hunchbka )
{
	UINT8 *ROM = memory_region(REGION_CPU1);

	offs_t offs;

	/* data lines D2 and D5 swapped */
	for (offs = 0; offs < 0x7400; offs++)
		ROM[offs] = BITSWAP8(ROM[offs],7,6,2,4,3,5,1,0);
}


static DRIVER_INIT( superbik )
{
	/* patch out protection */
	memory_region(REGION_CPU1)[0x0079] = 0xc0;
	memory_region(REGION_CPU1)[0x007a] = 0xc0;
	memory_region(REGION_CPU1)[0x007b] = 0xc0;
	memory_region(REGION_CPU1)[0x0081] = 0xc0;
	memory_region(REGION_CPU1)[0x0082] = 0xc0;
	memory_region(REGION_CPU1)[0x0083] = 0xc0;
	memory_region(REGION_CPU1)[0x00b6] = 0xc0;
	memory_region(REGION_CPU1)[0x00b7] = 0xc0;
	memory_region(REGION_CPU1)[0x00b8] = 0xc0;
	memory_region(REGION_CPU1)[0x0168] = 0xc0;
	memory_region(REGION_CPU1)[0x0169] = 0xc0;
	memory_region(REGION_CPU1)[0x016a] = 0xc0;

	/* and speed up the protection check */
	memory_region(REGION_CPU1)[0x0099] = 0xc0;
	memory_region(REGION_CPU1)[0x009a] = 0xc0;
	memory_region(REGION_CPU1)[0x009b] = 0xc0;
	memory_region(REGION_CPU1)[0x00bb] = 0xc0;
	memory_region(REGION_CPU1)[0x00bc] = 0xc0;
	memory_region(REGION_CPU1)[0x00bd] = 0xc0;
}


static DRIVER_INIT( hero )
{
	/* patch out protection */
	memory_region(REGION_CPU1)[0x0087] = 0xc0;
	memory_region(REGION_CPU1)[0x0088] = 0xc0;
	memory_region(REGION_CPU1)[0x0aa1] = 0xc0;
	memory_region(REGION_CPU1)[0x0aa2] = 0xc0;
	memory_region(REGION_CPU1)[0x0aa3] = 0xc0;
	memory_region(REGION_CPU1)[0x0aaf] = 0xc0;
	memory_region(REGION_CPU1)[0x0ab0] = 0xc0;
	memory_region(REGION_CPU1)[0x0ab1] = 0xc0;
	memory_region(REGION_CPU1)[0x0abd] = 0xc0;
	memory_region(REGION_CPU1)[0x0abe] = 0xc0;
	memory_region(REGION_CPU1)[0x0abf] = 0xc0;
	memory_region(REGION_CPU1)[0x4de0] = 0xc0;
	memory_region(REGION_CPU1)[0x4de1] = 0xc0;
	memory_region(REGION_CPU1)[0x4de2] = 0xc0;
}


static DRIVER_INIT( raiders )
{
	UINT8 *ROM = memory_region(REGION_CPU1);

	offs_t offs;

	/* data lines D1 and D5 swapped */
	for (offs = 0; offs < 0x7400; offs++)
		ROM[offs] = BITSWAP8(ROM[offs],7,1,5,4,3,2,6,0);

	/* patch out protection */
	memory_region(REGION_CPU1)[0x010a] = 0xc0;
	memory_region(REGION_CPU1)[0x010b] = 0xc0;
	memory_region(REGION_CPU1)[0x010c] = 0xc0;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

#define CVS_GAME(year, name, parent, init, company, desc) \
GAME( year, name, parent, cvs, cvs, init, ROT90, company, desc, GAME_NO_COCKTAIL )

GAME( 1981, cvs, 0, cvs, cvs, 0, ROT90, "Century Electronics", "CVS BIOS", GAME_IS_BIOS_ROOT )

/*        YEAR  NAME      PARENT    INIT */
CVS_GAME( 1981, cosmos,   cvs,      cosmos,  "Century Electronics", "Cosmos" )
CVS_GAME( 1981, darkwar,  cvs,      0,       "Century Electronics", "Dark Warrior" )
CVS_GAME( 1981, spacefrt, cvs,      spacefrt,"Century Electronics", "Space Fortress (CVS)" )
CVS_GAME( 1982, 8ball,    cvs,      0,       "Century Electronics", "Video Eight Ball" )
CVS_GAME( 1982, 8ball1,   8ball,    0,       "Century Electronics", "Video Eight Ball (Rev.1)" )
CVS_GAME( 1982, logger,   cvs,      0,       "Century Electronics", "Logger" )
CVS_GAME( 1982, dazzler,  cvs,      0,       "Century Electronics", "Dazzler" )
CVS_GAME( 1982, wallst,   cvs,      0,       "Century Electronics", "Wall Street" )
CVS_GAME( 1982, radarzon, cvs,      0,       "Century Electronics", "Radar Zone" )
CVS_GAME( 1982, radarzn1, radarzon, 0,       "Century Electronics", "Radar Zone (Rev.1)" )
CVS_GAME( 1982, radarznt, radarzon, 0,       "Century Electronics (Tuni Electro Service Inc)", "Radar Zone (Tuni)" )
CVS_GAME( 1982, outline,  radarzon, 0,       "Century Electronics", "Outline" )
CVS_GAME( 1982, goldbug,  cvs,      goldbug, "Century Electronics", "Gold Bug" )
CVS_GAME( 1982, diggerc,  cvs,      0,       "Century Electronics", "Digger (CVS)" )
CVS_GAME( 1983, heartatk, cvs,      0,       "Century Electronics", "Heart Attack" )
CVS_GAME( 1983, hunchbak, cvs,      0,       "Century Electronics", "Hunchback (set 1)" )
CVS_GAME( 1983, hunchbka, hunchbak, hunchbka,"Century Electronics", "Hunchback (set 2)" )
CVS_GAME( 1983, superbik, cvs,      superbik,"Century Electronics", "Superbike" )
CVS_GAME( 1983, raiders,  cvs,      raiders, "Century Electronics", "Raiders" )
CVS_GAME( 1983, hero,     cvs,      hero,    "Seatongrove Ltd",     "Hero" )
CVS_GAME( 1984, huncholy, cvs,      huncholy,"Seatongrove Ltd",     "Hunchback Olympic" )
