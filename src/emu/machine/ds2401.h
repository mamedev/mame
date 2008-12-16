/*
 * DS2401
 *
 * Dallas Semiconductor
 * Silicon Serial Number
 *
 */

#if !defined( DS2401_H )
#define DS2401_H ( 1 )

#define DS2401_MAXCHIP ( 3 )

extern void ds2401_init( running_machine *machine, int which, const UINT8 *data );
extern void ds2401_write( running_machine *machine, int which, int data );
extern int ds2401_read( running_machine *machine, int which );

#endif
