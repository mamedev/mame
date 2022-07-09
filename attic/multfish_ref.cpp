// license:BSD-3-Clause
// copyright-holders:David Haywood

/* The following sets are known to exist based on official documentation, but have not been dumped. */
/* no other official sets are known to exist apart from these and the ones in multfish.c */

#if 0

ROM_START( czmon_10 ) // 081027 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_l_081027.rom", 0x00000, 0x40000, SHA1(11a1523bc0ce5cf43534b34201f59784283693f0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_loto.001", 0x000000, 0x80000, SHA1(bf953dc53ec85f4841fe7ada7e480520b3bce1d7) )
	ROM_LOAD( "crazymonkey_loto.002", 0x100000, 0x80000, SHA1(57db2212b690a8a92034ba4993526c34cbf48c09) )
	ROM_LOAD( "crazymonkey_loto.003", 0x200000, 0x80000, SHA1(4551494b883f8076931bb22fd0541b193039dfdc) )
	ROM_LOAD( "crazymonkey_loto.004", 0x300000, 0x80000, SHA1(b33d5007b649661a3d811139c2b40d036863343d) )
	ROM_LOAD( "crazymonkey_m.005",    0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006",    0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007",    0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008",    0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( czmon_11 ) // 081113 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_l_081113.rom", 0x00000, 0x40000, SHA1(7196c301691b47a572cefc090888db550f10998c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_loto.001", 0x000000, 0x80000, SHA1(bf953dc53ec85f4841fe7ada7e480520b3bce1d7) )
	ROM_LOAD( "crazymonkey_loto.002", 0x100000, 0x80000, SHA1(57db2212b690a8a92034ba4993526c34cbf48c09) )
	ROM_LOAD( "crazymonkey_loto.003", 0x200000, 0x80000, SHA1(4551494b883f8076931bb22fd0541b193039dfdc) )
	ROM_LOAD( "crazymonkey_loto.004", 0x300000, 0x80000, SHA1(b33d5007b649661a3d811139c2b40d036863343d) )
	ROM_LOAD( "crazymonkey_m.005",    0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006",    0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007",    0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008",    0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END


ROM_START( czmon_14 ) // 100311 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_l_100311.rom", 0x00000, 0x40000, CRC(8a766a31) SHA1(2dc50aabf2b027a578d433714023290ad320ea00) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_loto.001", 0x000000, 0x80000, SHA1(bf953dc53ec85f4841fe7ada7e480520b3bce1d7) )
	ROM_LOAD( "crazymonkey_loto.002", 0x100000, 0x80000, SHA1(57db2212b690a8a92034ba4993526c34cbf48c09) )
	ROM_LOAD( "crazymonkey_loto.003", 0x200000, 0x80000, SHA1(4551494b883f8076931bb22fd0541b193039dfdc) )
	ROM_LOAD( "crazymonkey_loto.004", 0x300000, 0x80000, SHA1(b33d5007b649661a3d811139c2b40d036863343d) )
	ROM_LOAD( "crazymonkey_m.005",    0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006",    0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007",    0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008",    0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END


ROM_START( fcockt_13 ) // 081124 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_l_081124.rom", 0x00000, 0x40000, SHA1(896252194f32842f784463668e6416cbfe9687a0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_loto.001", 0x000000, 0x80000, SHA1(bb2b518dc166836f7cedd4ec443b50687e8927e1) ) /* Only this set is listed as official hashes */
	ROM_LOAD( "fruitcocktail_loto.002", 0x100000, 0x80000, SHA1(2621e2644ebec3959c49905c54eb20a83d5a7bd6) )
	ROM_LOAD( "fruitcocktail_loto.003", 0x200000, 0x80000, SHA1(ffe11deef6b3b86b4b78e2e4d96c30f820e77971) )
	ROM_LOAD( "fruitcocktail_loto.004", 0x300000, 0x80000, SHA1(fa88d113721ce7c0b3418614cd6bb974c20df644) )
	ROM_LOAD( "fruitcocktail_m.005",    0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006",    0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007",    0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008",    0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END


ROM_START( lhaunt_9 ) // 081208 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_l_081208.rom", 0x00000, 0x40000, SHA1(4962bfc9c3aadd45fdb30bb159aaaed463e4d06b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_loto.001",   0x000000, 0x80000, SHA1(961f832654f2cdb844e36a1a9034b87b5e3750f5) )
	ROM_LOAD( "luckyhaunter_loto.002",   0x100000, 0x80000, SHA1(20ac980f4f8b502773845d2e1350b960ea707d83) )
	ROM_LOAD( "luckyhaunter_loto.003",   0x200000, 0x80000, SHA1(e4cf08104e7717d9706105ad52ade6bf9a782d76) )
	ROM_LOAD( "luckyhaunter_loto.004",   0x300000, 0x80000, SHA1(ceb4a5e9912f5d98483cb75e871c925dffbb8e72) )
	ROM_LOAD( "luckyhaunter_m.005",      0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006",      0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007",      0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008",      0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END


ROM_START( garage_8 ) // 081229 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_l_081229.rom", 0x00000, 0x40000, SHA1(6bc22aeb6d8d5ffbc556d9056a25e6506bb8f118) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_loto.001", 0x000000, 0x80000, SHA1(50de46c4ae28f70c96da03391446cca0cb91f43b) )
	ROM_LOAD( "garage_loto.002", 0x100000, 0x80000, SHA1(df5c1684f1a29f77a3fa79b35a9a0b9371c1b8a3) )
	ROM_LOAD( "garage_loto.003", 0x200000, 0x80000, SHA1(47e81aa034c1ac717b1715c521cbaa8ff4d336c5) )
	ROM_LOAD( "garage_loto.004", 0x300000, 0x80000, SHA1(71dbcd71ee5bddeda77a012c243981e0960e0c9e) )
	ROM_LOAD( "garage_m.005",    0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006",    0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007",    0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008",    0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( rclimb_6 ) // 090217 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc_l_090217.rom", 0x00000, 0x40000, SHA1(587be46d846fa7288227179bacedcc1ad5c2cd67) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber_loto.001", 0x000000, 0x80000, SHA1(50fd1548e9f6736c5bb34d75ebd36e233e8773c2) )
	ROM_LOAD( "rockclimber_loto.002", 0x100000, 0x80000, SHA1(50b4807becf3386ce7f4492f71f833973bf764d0) )
	ROM_LOAD( "rockclimber_loto.003", 0x200000, 0x80000, SHA1(ab3401f624fa6b5ef2fe0dcdd0dc94b7a0eabece) )
	ROM_LOAD( "rockclimber_loto.004", 0x300000, 0x80000, SHA1(e34b17e323542b368f8613cf2bc42a0c3b98fd29) )
	ROM_LOAD( "rockclimber_m.005",    0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006",    0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007",    0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008",    0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( resdnt_4 ) // 090129 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs_l_090129.rom", 0x00000, 0x40000, SHA1(5728b019241359d83abc117157ebf62a52457917) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "resident_loto.001", 0x000000, 0x80000, SHA1(acd8b424cab982e471c7d3a56ccd6e1720fd8ceb) )
	ROM_LOAD( "resident_loto.002", 0x100000, 0x80000, SHA1(83b9cf3a28e93e31d3a5cff01e5d0b9356e112cf) )
	ROM_LOAD( "resident_loto.003", 0x200000, 0x80000, SHA1(30ccd372f1a5ad9a600099cf1ac31d9b235f88b9) )
	ROM_LOAD( "resident_loto.004", 0x300000, 0x80000, SHA1(acfec89793a591d32a90bb7ba82514d97b2652f8) )
	ROM_LOAD( "resident_m.005",    0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006",    0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007",    0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008",    0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_7 ) // 100311 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs_l_100311.rom", 0x00000, 0x40000, CRC(9969562e) SHA1(08052c1e9f3415ac005e5f67411e15d0c8f7450e) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "resident_loto.001", 0x000000, 0x80000, SHA1(acd8b424cab982e471c7d3a56ccd6e1720fd8ceb) )
	ROM_LOAD( "resident_loto.002", 0x100000, 0x80000, SHA1(83b9cf3a28e93e31d3a5cff01e5d0b9356e112cf) )
	ROM_LOAD( "resident_loto.003", 0x200000, 0x80000, SHA1(30ccd372f1a5ad9a600099cf1ac31d9b235f88b9) )
	ROM_LOAD( "resident_loto.004", 0x300000, 0x80000, SHA1(acfec89793a591d32a90bb7ba82514d97b2652f8) )
	ROM_LOAD( "resident_m.005",    0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006",    0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007",    0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008",    0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END


ROM_START( gnome_6 ) // 090604 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gn_l_090604.rom", 0x00000, 0x40000, SHA1(5c736c974011980b343cf131b54f00aede5ef0ef) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome_loto.001", 0x000000, 0x80000, CRC(15f75190) SHA1(85587a008889b5e34f5f79ceb1abfcd9a6c53cec) )
	ROM_LOAD( "gnome_loto.002", 0x100000, 0x80000, CRC(26f9af6a) SHA1(131b26e035b4cfd9d36ab8a7f2957e77170a529d) )
	ROM_LOAD( "gnome_loto.003", 0x200000, 0x80000, CRC(7d388bd5) SHA1(2f2eadc44f35033d61dbab390a4dbfec23f31c85) )
	ROM_LOAD( "gnome_loto.004", 0x300000, 0x80000, CRC(7bad4ac5) SHA1(2cfac6462b666b4bb0d546932b6784a80cf8d0d4) )
	ROM_LOAD( "gnome_loto.005", 0x080000, 0x80000, CRC(f86a7d02) SHA1(1e7da8ac89eb8b1d2c293d2cfead7a52524fc674) )
	ROM_LOAD( "gnome_loto.006", 0x180000, 0x80000, CRC(d66f1ab8) SHA1(27b612ab42008f8673a0508a1b813c63a0e2ba4c) )
	ROM_LOAD( "gnome_loto.007", 0x280000, 0x80000, CRC(99ae985c) SHA1(f0fe5a0dbc289a93246a825f32a726cf62ccb9aa) )
	ROM_LOAD( "gnome_loto.008", 0x380000, 0x80000, CRC(4dc3f777) SHA1(3352170877c59daff63c056dfca00915f87b5795) )
ROM_END



//GAME( 2003, czmon_10,   czmon_13,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,            ROT0, "Igrosoft", "Crazy Monkey (081027 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */
//GAME( 2003, czmon_11,   czmon_13,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,            ROT0, "Igrosoft", "Crazy Monkey (081113 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */
//GAME( 2003, czmon_14,   czmon_13,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,            ROT0, "Igrosoft", "Crazy Monkey (100311 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */

//GAME( 2003, fcockt_13,   fcockt_8,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,           ROT0, "Igrosoft", "Fruit Cocktail (081124 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */

//GAME( 2003, lhaunt_9,    lhaunt_6,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,           ROT0, "Igrosoft", "Lucky Haunter (081208 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */

//GAME( 2004, garage_8,    garage_5,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,           ROT0, "Igrosoft", "Garage (081229 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */

//GAME( 2004, rclimb_6,    rclimb_3,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,           ROT0, "Igrosoft", "Rock Climber (090217 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */

//GAME( 2004, resdnt_4,    resdnt_6,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,           ROT0, "Igrosoft", "Resident (090129 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */
//GAME( 2004, resdnt_7,    resdnt_6,   igrosoft_gamble, igrosoft_gamble, driver_device,  empty_init,           ROT0, "Igrosoft", "Resident (100311 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */

//GAME( 2007, gnome_6,     gnome_9,    igrosoft_gamble, igrosoft_gamble, igrosoft_gamble_state, init_gnomel,   ROT0, "Igrosoft", "Gnome (090604 Lottery)", MACHINE_SUPPORTS_SAVE ) /* Lottery */

#endif
