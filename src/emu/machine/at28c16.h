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

/* memory handlers */

extern READ8_HANDLER( at28c16_0_r );
extern READ8_HANDLER( at28c16_1_r );
extern READ8_HANDLER( at28c16_2_r );
extern READ8_HANDLER( at28c16_3_r );
extern WRITE8_HANDLER( at28c16_0_w );
extern WRITE8_HANDLER( at28c16_1_w );
extern WRITE8_HANDLER( at28c16_2_w );
extern WRITE8_HANDLER( at28c16_3_w );

#endif
