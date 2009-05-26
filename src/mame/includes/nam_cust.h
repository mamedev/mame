/* Namco Custom Chip ROM loading Macros */


/* the 50XX is a player score controller, used for protection */

/* the 51XX is an I/O controller */
#define ROM_REGION_NAMCO_51XX( region ) \
	ROM_REGION( 0x400, region, 0 ) /* 1k for the 51xx  */ \
	ROM_LOAD( "51xx.bin",     0x0000, 0x0400, CRC(c2f57ef8) SHA1(50de79e0d6a76bda95ffb02fcce369a79e6abfec) ) \

/* the 52XX is a sample player */

/* the 53XX is an I/O controller */
#define ROM_REGION_NAMCO_53XX( region ) \
	ROM_REGION( 0x400, region, 0 ) /* 1k for the 53xx  */ \
	ROM_LOAD( "53xx.bin",     0x0000, 0x0400, CRC(b326fecb) SHA1(758d8583d658e4f1df93184009d86c3eb8713899) ) \

/* the 54XX is an explosion sound generator */

