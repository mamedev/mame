/*
 * uPD4701
 *
 * Incremental Encoder Control
 *
 */

#if !defined( UPD4701_H )

#define UPD4701_MAXCHIP ( 1 )

extern void uPD4701_init( int chip );
extern void uPD4701_cs_w( int chip, int cs );
extern void uPD4701_xy_w( int chip, int xy );
extern void uPD4701_ul_w( int chip, int ul );
extern void uPD4701_resetx_w( int chip, int resetx );
extern void uPD4701_resety_w( int chip, int resety );
extern int uPD4701_d_r( int chip );
extern int uPD4701_cf_r( int chip );
extern int uPD4701_sf_r( int chip );

extern void uPD4701_x_add( int chip, int dx );
extern void uPD4701_y_add( int chip, int dy );
extern void uPD4701_switches_set( int chip, int switches );

#define UPD4701_H
#endif
