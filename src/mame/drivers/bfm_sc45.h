/* Scorpion 4 + 5 driver releated includes */
/* mainly used for stuff which is currently shared between sc4 / 5 sets to avoid duplication */

#define sc_dnd_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	/* Regular */ \
	ROM_LOAD( "95008606.bin", 0x0000, 0x100000, CRC(c63c8663) SHA1(4718baf87340fe93ccfe642a1a0cdb9d9dcac57f) ) /* 0 (1907)  DOND SOUNDS 11 */ \
	ROM_LOAD( "95008607.bin", 0x0000, 0x100000, CRC(a615514f) SHA1(d4ff7d4fe6f1dd1d7b00cc504f90b2921aa5e8fb) ) /* 1 */ \
	\
	/*  Casino */ \
	ROM_LOAD( "95008631.bin", 0x0000, 0x100000, CRC(7208854a) SHA1(a02de60cfcbafe5da4f67596ab65237f5b5f41b7) ) /* 0 (1954) DOND SOUNDS 11 */ \
	\
	ROM_LOAD( "95008632.bin", 0x0000, 0x100000, CRC(fd81a317) SHA1(1e597fd58aab5a7a8321dc4daf138ee07b42c094) ) /* 0 (1945) DOND SOUNDS 11 */ \
	ROM_LOAD( "95008633.bin", 0x0000, 0x100000, CRC(a7a445d4) SHA1(dbb1938c33ce654c2d4aa3b6af8c210f5aad2ae3) ) /* 1 */ \
	\
	/*  Casino */ \
	ROM_LOAD( "95008661.bin", 0x0000, 0x100000, CRC(2d9ebcd5) SHA1(d824a227420cbe616aca6e2fd279af691ddfd87a) ) /* 0 (1945) DOND SOUNDS 12 */ \
	\
	ROM_LOAD( "95008680.bin", 0x0000, 0x100000, CRC(9bd439d1) SHA1(5e71d04e5697e92998bae28f7352ea7742cafe07) ) /* 0 (1964) DOND SOUNDS 11 */ \
	\
	ROM_LOAD( "95008698.bin", 0x0000, 0x100000, CRC(8eea7754) SHA1(7612c128d6c062bba3477d55aee3089e1255f61e) ) /* 0 (1964) DOND SOUNDS 12 */ \


#define sc_dndbe_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008624.bin", 0x0000, 0x100000, CRC(bf9620ea) SHA1(63f5a209da3d0117fcb579364a53b23d2b02cfe5) ) \
	ROM_LOAD( "95008625.bin", 0x0000, 0x100000, CRC(2e1a1db0) SHA1(41ebad0615d0ad3fea6f2c00e2bb170d5e417e4a) ) \

#define sc_dndbc_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95009100.bin", 0x0000, 0x100000, CRC(b06110c0) SHA1(84289721a8c71124cc4df79fc90d0ded8d43fd07) ) \
	ROM_LOAD( "95009101.bin", 0x0000, 0x100000, CRC(53b38d0a) SHA1(1da40cbee8a18713864e3a578ac49c2108585e44) ) \

#define sc_mowow_others \
	ROM_REGION( 0x5000, "pics", 0 ) \
	ROM_LOAD( "95890712.bin", 0x0000, 0x5000, CRC(ec6db00b) SHA1(d16a1527caa3c115e3326c897ce0fa66e3a0420d) ) \
	ROM_LOAD( "95890713.bin", 0x0000, 0x5000, CRC(f0bb40b7) SHA1(33c19dab3086cdeae4f503fbf3f3cc5f0dad98c4) ) \
	ROM_LOAD( "95890714.bin", 0x0000, 0x5000, CRC(33e16227) SHA1(87efc1a046ef6af0b72cc76a6ee393a4d1ddbce3) ) \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008550.bin", 0x0000, 0x100000, CRC(db6343bf) SHA1(e4d702020af67aa5be0560027706c1cbf34296fa) ) \
	ROM_LOAD( "95008551.bin", 0x0000, 0x100000, CRC(2d89a52a) SHA1(244101df7f6beae545f9b823750f908f532ac1e4) ) \
	ROM_LOAD( "95008850.bin", 0x0000, 0x0af41f, CRC(8ca16e09) SHA1(9b494ad6946c2c7bbfad6591e62fa699fd53b6dc) ) \
	ROM_LOAD( "95008869.bin", 0x0000, 0x0b9d9d, CRC(f3ef3bbb) SHA1(92f9835e96c4fc444a451e97b2b8a7b66e5794b7) ) \

#define sc_nunsm_others \
	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) \
	ROM_LOAD( "95008522.bin", 0x0000, 0x0f9907, CRC(df612d06) SHA1(cbca56230c4ad4c6411aa5c2e2ca2ae8152b5297) ) \

