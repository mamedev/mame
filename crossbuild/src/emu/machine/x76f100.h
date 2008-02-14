/*
 * x76f100.h
 *
 * Secure SerialFlash
 *
 */

#if !defined( X76F100_H )

#define X76F100_MAXCHIP ( 2 )

extern void x76f100_init( int chip, UINT8 *data );
extern void x76f100_cs_write( int chip, int cs );
extern void x76f100_rst_write( int chip, int rst );
extern void x76f100_scl_write( int chip, int scl );
extern void x76f100_sda_write( int chip, int sda );
extern int x76f100_sda_read( int chip );
extern NVRAM_HANDLER( x76f100_0 );
extern NVRAM_HANDLER( x76f100_1 );

#define X76F100_H
#endif
