/*****************************************************************************
 *
 * audio/vic6560.h
 *
 ****************************************************************************/

#ifndef __MOS6560_H__
#define __MOS6560_H__

#include "devlegcy.h"
#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef enum
{
	MOS6560_ATTACKUFO,        // this is a 6560VIC derivative, missing some of the features
	MOS6560,                  // this is the NTSC version
	MOS6561                   // this is the PAL version
} mos6560_type;

typedef struct _mos6560_interface mos6560_interface;
struct _mos6560_interface
{
	const char		*screen;

	mos6560_type	type;

	devcb_read8		x_cb;
	devcb_read8		y_cb;
	devcb_read8		button_cb;

	devcb_read8		paddle0_cb, paddle1_cb;

	devcb_read8		dma_read;
	devcb_read8		dma_read_color;
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

class mos6560_device : public device_t,
                                  public device_sound_interface
{
public:
	mos6560_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mos6560_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }

	UINT8 bus_r();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type MOS656X;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MOS656X_ADD(_tag, _interface) \
	MCFG_SOUND_ADD(_tag, MOS656X, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_MOS6560_ADD(_tag, _screen_tag, _clock, _config) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_REFRESH_RATE(MOS6560_VRETRACERATE) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) \
	MCFG_SCREEN_SIZE((MOS6560_XSIZE + 7) & ~7, MOS6560_YSIZE) \
	MCFG_SCREEN_VISIBLE_AREA(MOS6560_MAME_XPOS, MOS6560_MAME_XPOS + MOS6560_MAME_XSIZE - 1, MOS6560_MAME_YPOS, MOS6560_MAME_YPOS + MOS6560_MAME_YSIZE - 1) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, mos6560_device, screen_update) \
	MCFG_PALETTE_LENGTH(16) \
	MCFG_PALETTE_INIT(mos6560) \
	MCFG_SOUND_ADD(_tag, MOS656X, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_MOS6561_ADD(_tag, _screen_tag, _clock, _config) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_REFRESH_RATE(MOS6561_VRETRACERATE) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) \
	MCFG_SCREEN_SIZE((MOS6561_XSIZE + 7) & ~7, MOS6561_YSIZE) \
	MCFG_SCREEN_VISIBLE_AREA(MOS6561_MAME_XPOS, MOS6561_MAME_XPOS + MOS6561_MAME_XSIZE - 1, MOS6561_MAME_YPOS, MOS6561_MAME_YPOS + MOS6561_MAME_YSIZE - 1) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, mos6560_device, screen_update) \
	MCFG_PALETTE_LENGTH(16) \
	MCFG_PALETTE_INIT(mos6560) \
	MCFG_SOUND_ADD(_tag, MOS656X, _clock) \
	MCFG_DEVICE_CONFIG(_config)

/***************************************************************************
    I/O PROTOTYPES
***************************************************************************/

WRITE8_DEVICE_HANDLER( mos6560_port_w );
READ8_DEVICE_HANDLER( mos6560_port_r );

void mos6560_raster_interrupt_gen( device_t *device );
UINT32 mos6560_video_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect );

extern PALETTE_INIT( mos6560 );

#endif /* __MOS6560_H__ */
