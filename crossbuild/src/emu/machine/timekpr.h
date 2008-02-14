/*
 * STmicroelectronics TIMEKEEPER SRAM
 *
 * Supports: MK48T08, M48T02 & M48T58
 *
 */

#if !defined( TIMEKPR_H )
#define TIMEKPR_H ( 1 )

extern void timekeeper_init( int chip, int type, UINT8 *data );

#define MAX_TIMEKEEPER_CHIPS ( 1 )

#define TIMEKEEPER_M48T58 ( 1 )
#define TIMEKEEPER_M48T02 ( 2 )
#define TIMEKEEPER_MK48T08 ( 3 )
#define TIMEKEEPER_MIDZEUS2 ( 4 )

/* nvram handlers */

extern NVRAM_HANDLER( timekeeper_0 );

/* 8bit memory handlers */

extern READ8_HANDLER( timekeeper_0_r );
extern WRITE8_HANDLER( timekeeper_0_w );

/* 16bit memory handlers */

extern READ16_HANDLER( timekeeper_0_msb16_r );
extern WRITE16_HANDLER( timekeeper_0_msb16_w );

/* 32bit memory handlers */

extern READ32_HANDLER( timekeeper_0_32be_r );
extern WRITE32_HANDLER( timekeeper_0_32be_w );
extern READ32_HANDLER( timekeeper_0_32le_lsb16_r );
extern WRITE32_HANDLER( timekeeper_0_32le_lsb16_w );

#endif
