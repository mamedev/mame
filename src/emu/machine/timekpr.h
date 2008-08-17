/*
 * STmicroelectronics TIMEKEEPER SRAM
 *
 * Supports: MK48T08, M48T02 & M48T58
 *
 */

#if !defined( TIMEKPR_H )
#define TIMEKPR_H ( 1 )

extern void timekeeper_init( running_machine *machine, int chip, int type, UINT8 *data );

#define MAX_TIMEKEEPER_CHIPS ( 1 )

#define TIMEKEEPER_M48T02 ( 1 )
#define TIMEKEEPER_M48T35 ( 2 )
#define TIMEKEEPER_M48T58 ( 3 )
#define TIMEKEEPER_MK48T08 ( 4 )

/* nvram handlers */

extern NVRAM_HANDLER( timekeeper_0 );

/* memory handlers */

extern READ8_HANDLER( timekeeper_0_r );
extern WRITE8_HANDLER( timekeeper_0_w );

#endif
