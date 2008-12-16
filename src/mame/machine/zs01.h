/*
 * zs01.h
 *
 * Secure SerialFlash
 *
 */

#if !defined( ZS01_H )

#define ZS01_MAXCHIP ( 2 )

typedef void (*zs01_write_handler)( running_machine *machine, int pin, int value );
typedef int (*zs01_read_handler)( running_machine *machine, int pin );

extern void zs01_init( running_machine *machine, int chip, UINT8 *data, zs01_write_handler write, zs01_read_handler read, UINT8 *ds2401 );
extern void zs01_cs_write( running_machine *machine, int chip, int cs );
extern void zs01_rst_write( running_machine *machine, int chip, int rst );
extern void zs01_scl_write( running_machine *machine, int chip, int scl );
extern void zs01_sda_write( running_machine *machine, int chip, int sda );
extern int zs01_sda_read( running_machine *machine, int chip );
extern NVRAM_HANDLER( zs01_0 );
extern NVRAM_HANDLER( zs01_1 );

#define ZS01_H
#endif
