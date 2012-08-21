/*****************************************************************************
 *
 * audio/ted7360.h
 *
 *
 ****************************************************************************/

#ifndef __TED7360_H__
#define __TED7360_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*ted7360_dma_read)(running_machine &machine, int);
typedef int (*ted7360_dma_read_rom)(running_machine &machine, int);
typedef void (*ted7360_irq) (running_machine &, int);
typedef UINT8 (*ted7360_key_cb) (running_machine &, int);

typedef enum
{
	TED7360_NTSC,
	TED7360_PAL
} ted_type;

typedef struct _ted7360_interface ted7360_interface;
struct _ted7360_interface
{
	const char             *screen;

	ted_type               type;

	ted7360_dma_read       dma_read;
	ted7360_dma_read_rom   dma_read_rom;

	ted7360_irq            irq;

	ted7360_key_cb         keyb_cb;
};

/***************************************************************************
    CONSTANTS
***************************************************************************/


#define TED7360NTSC_VRETRACERATE 60
#define TED7360PAL_VRETRACERATE 50
#define TED7360_HRETRACERATE 15625

/* the following values depend on the VIC clock,
 * but to achieve TV-frequency the clock must have a fix frequency */
#define TED7360_HSIZE	320
#define TED7360_VSIZE	200

/* of course you clock select an other clock, but for accurate */
/* video timing (these are used in c16/c116/plus4) */
#define TED7360NTSC_CLOCK	(14318180/4)
#define TED7360PAL_CLOCK	(17734470/5)

/* pal 50 Hz vertical screen refresh, screen consists of 312 lines
 * ntsc 60 Hz vertical screen refresh, screen consists of 262 lines */
#define TED7360NTSC_LINES 261
#define TED7360PAL_LINES 312

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

DECLARE_LEGACY_SOUND_DEVICE(TED7360, ted7360);

#define MCFG_TED7360_ADD(_tag, _interface) \
	MCFG_SOUND_ADD(_tag, TED7360, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/*----------- defined in audio/ted7360.c -----------*/

WRITE8_DEVICE_HANDLER( ted7360_port_w );
READ8_DEVICE_HANDLER( ted7360_port_r );

WRITE_LINE_DEVICE_HANDLER( ted7360_rom_switch_w );
READ_LINE_DEVICE_HANDLER( ted7360_rom_switch_r );

void ted7360_frame_interrupt_gen(device_t *device);
void ted7360_raster_interrupt_gen(device_t *device);
UINT32 ted7360_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);


#endif /* __TED7360_H__ */
