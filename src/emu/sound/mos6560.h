/*****************************************************************************
 *
 * audio/vic6560.h
 *
 ****************************************************************************/

#ifndef __MOS6560_H__
#define __MOS6560_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT8 (*mos6560_lightpen_x_callback)(running_machine *machine);
typedef UINT8 (*mos6560_lightpen_y_callback)(running_machine *machine);
typedef UINT8 (*mos6560_lightpen_button_callback)(running_machine *machine);
typedef UINT8 (*mos6560_paddle_callback)(running_machine *machine);

typedef int (*mos6560_dma_read)(running_machine *machine, int);
typedef int (*mos6560_dma_read_color)(running_machine *machine, int);


typedef enum
{
	MOS6560_ATTACKUFO,        // this is a 6560VIC derivative, missing some of the features
	MOS6560,                  // this is the NTSC version
	MOS6561                   // this is the PAL version
} mos6560_type;

typedef struct _mos6560_interface mos6560_interface;
struct _mos6560_interface
{
	const char         *screen;

	mos6560_type type;

	mos6560_lightpen_x_callback        x_cb;
	mos6560_lightpen_y_callback        y_cb;
	mos6560_lightpen_button_callback   button_cb;

	mos6560_paddle_callback        paddle0_cb, paddle1_cb;

	mos6560_dma_read          dma_read;
	mos6560_dma_read_color    dma_read_color;
};

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MOS6560_VRETRACERATE 60
#define MOS6561_VRETRACERATE 50
#define MOS656X_HRETRACERATE 15625

#define MOS6560_MAME_XPOS  4		   /* xleft not displayed */
#define MOS6560_MAME_YPOS  10		   /* y up not displayed */
#define MOS6561_MAME_XPOS  20
#define MOS6561_MAME_YPOS  10
#define MOS6560_MAME_XSIZE	200
#define MOS6560_MAME_YSIZE	248
#define MOS6561_MAME_XSIZE	224
#define MOS6561_MAME_YSIZE	296
/* real values */

#define MOS6560_LINES 261
#define MOS6561_LINES 312

#define MOS6560_XSIZE	(4+201)		   /* 4 left not visible */
#define MOS6560_YSIZE	(10+251)	   /* 10 not visible */
/* cycles 65 */

#define MOS6561_XSIZE	(20+229)	   /* 20 left not visible */
#define MOS6561_YSIZE	(10+302)	   /* 10 not visible */
/* cycles 71 */


/* the following values depend on the VIC clock,
 * but to achieve TV-frequency the clock must have a fix frequency */
#define MOS6560_CLOCK	(14318181/14)
#define MOS6561_CLOCK	(4433618/4)


/***************************************************************************
    INFO PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( mos6560 );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MOS656X DEVICE_GET_INFO_NAME( mos6560 )
#define SOUND_MOS656X MOS656X

#define MDRV_MOS656X_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, SOUND, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(sound_config, type, SOUND_MOS656X) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    I/O PROTOTYPES
***************************************************************************/

WRITE8_DEVICE_HANDLER( mos6560_port_w );
READ8_DEVICE_HANDLER( mos6560_port_r );

void mos6560_raster_interrupt_gen( running_device *device );
UINT32 mos6560_video_update( running_device *device, bitmap_t *bitmap, const rectangle *cliprect );


#endif /* __MOS6560_H__ */
