/*
 * ATMEL AT28C16
 *
 * 16K ( 2K x 8 ) Parallel EEPROM
 *
 */

#if !defined( AT28C16_H )
#define AT28C16_H ( 1 )

#define MAX_AT28C16_CHIPS ( 4 )

extern void at28c16_init( int chip, UINT8 *data, UINT8 *id );
extern void at28c16_a9_12v( int chip, int a9_12v );

/* nvram handlers */

NVRAM_HANDLER( at28c16_0 );
NVRAM_HANDLER( at28c16_1 );
NVRAM_HANDLER( at28c16_2 );
NVRAM_HANDLER( at28c16_3 );

/* 16bit memory handlers */

extern READ16_HANDLER( at28c16_16msb_0_r );
extern READ16_HANDLER( at28c16_16msb_1_r );
extern READ16_HANDLER( at28c16_16msb_2_r );
extern READ16_HANDLER( at28c16_16msb_3_r );
extern WRITE16_HANDLER( at28c16_16msb_0_w );
extern WRITE16_HANDLER( at28c16_16msb_1_w );
extern WRITE16_HANDLER( at28c16_16msb_2_w );
extern WRITE16_HANDLER( at28c16_16msb_3_w );

/* 32bit memory handlers */

extern READ32_HANDLER( at28c16_32le_0_r );
extern READ32_HANDLER( at28c16_32le_1_r );
extern READ32_HANDLER( at28c16_32le_2_r );
extern READ32_HANDLER( at28c16_32le_3_r );
extern WRITE32_HANDLER( at28c16_32le_0_w );
extern WRITE32_HANDLER( at28c16_32le_1_w );
extern WRITE32_HANDLER( at28c16_32le_2_w );
extern WRITE32_HANDLER( at28c16_32le_3_w );

extern READ32_HANDLER( at28c16_32le_16lsb_0_r );
extern READ32_HANDLER( at28c16_32le_16lsb_1_r );
extern READ32_HANDLER( at28c16_32le_16lsb_2_r );
extern READ32_HANDLER( at28c16_32le_16lsb_3_r );
extern WRITE32_HANDLER( at28c16_32le_16lsb_0_w );
extern WRITE32_HANDLER( at28c16_32le_16lsb_1_w );
extern WRITE32_HANDLER( at28c16_32le_16lsb_2_w );
extern WRITE32_HANDLER( at28c16_32le_16lsb_3_w );

#endif
