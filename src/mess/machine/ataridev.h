/*
    ataridev.h

*/

#ifndef _ATARIDEV_H
#define _ATARIDEV_H

#define ATARI_5200	0
#define ATARI_400	1
#define ATARI_800	2
#define ATARI_600XL 3
#define ATARI_800XL 4

/*----------- defined in machine/ataricrt.c -----------*/

MACHINE_START( a400 );
MACHINE_START( a800 );
MACHINE_START( a800xl );
MACHINE_START( a5200 );
MACHINE_START( xegs );

DEVICE_IMAGE_LOAD( a800_cart );
DEVICE_IMAGE_UNLOAD( a800_cart );

DEVICE_IMAGE_LOAD( a800_cart_right );
DEVICE_IMAGE_UNLOAD( a800_cart_right );

DEVICE_IMAGE_LOAD( a5200_cart );
DEVICE_IMAGE_UNLOAD( a5200_cart );

DEVICE_IMAGE_LOAD( xegs_cart );
DEVICE_IMAGE_UNLOAD( xegs_cart );


/*----------- defined in machine/atarifdc.c -----------*/
/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(ATARI_FDC, atari_fdc);

#define MCFG_ATARI_FDC_ADD(_tag)	\
	MCFG_DEVICE_ADD((_tag),  ATARI_FDC, 0)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
READ8_DEVICE_HANDLER( atari_serin_r );
WRITE8_DEVICE_HANDLER( atari_serout_w );
WRITE_LINE_DEVICE_HANDLER( atarifdc_pia_cb2_w );

#endif /* _ATARIDEV_H */

