/* Namco Custom Chip ROM loading Macros */


/* the 50XX is a player score controller, used for protection */
#define ROM_REGION_NAMCO_50XX( region ) \
	ROM_REGION( 0x800, region, 0 ) /* 2k for the 50xx  */ \
	ROM_LOAD( "50xx.bin",     0x0000, 0x0800, CRC(a0acbaf7) SHA1(f03c79451e73b3a93c1591cdb27fedc9f130508d) ) \

/* the 51XX is an I/O controller */
#define ROM_REGION_NAMCO_51XX( region ) \
	ROM_REGION( 0x400, region, 0 ) /* 1k for the 51xx  */ \
	ROM_LOAD( "51xx.bin",     0x0000, 0x0400, CRC(c2f57ef8) SHA1(50de79e0d6a76bda95ffb02fcce369a79e6abfec) ) \

/* the 52XX is a sample player */
#define ROM_REGION_NAMCO_52XX( region ) \
	ROM_REGION( 0x400, region, 0 ) /* 1k for the 52xx  */ \
	ROM_LOAD( "52xx.bin",     0x0000, 0x0400, CRC(3257d11e) SHA1(4883b2fdbc99eb7b9906357fcc53915842c2c186) ) \

/* the 53XX is an I/O controller */
#define ROM_REGION_NAMCO_53XX( region ) \
	ROM_REGION( 0x400, region, 0 ) /* 1k for the 53xx  */ \
	ROM_LOAD( "53xx.bin",     0x0000, 0x0400, CRC(b326fecb) SHA1(758d8583d658e4f1df93184009d86c3eb8713899) ) \

/* the 54XX is an explosion sound generator */
#define ROM_REGION_NAMCO_54XX( region ) \
	ROM_REGION( 0x400, region, 0 ) /* 1k for the 54xx  */ \
	ROM_LOAD( "54xx.bin",     0x0000, 0x0400, CRC(ee7357e0) SHA1(01bdf984a49e8d0cc8761b2cc162fd6434d5afbe) ) \

