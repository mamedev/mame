/**********************************************************************

    DEC VT Terminal video emulation
    [ DC012 and DC011 emulation ]

    01/05/2009 Initial implementation [Miodrag Milanovic]

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __VT_VIDEO__
#define __VT_VIDEO__

#include "emu.h"
#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class vt100_video_device : public device_t
{
public:
	vt100_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vt100_video_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type VT100_VIDEO;


#define MCFG_VT100_VIDEO_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, VT100_VIDEO, 0) \
	MCFG_DEVICE_CONFIG(_intrf)
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct vt_video_interface
{
	const char *screen_tag;		/* screen we are acting on */
	const char *char_rom_region_tag; /* character rom region */

	/* this gets called for every memory read */
	devcb_read8			in_ram_func;
	devcb_write8		clear_video_interrupt;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/
/* register access */
DECLARE_READ8_DEVICE_HANDLER  ( vt_video_lba7_r );
DECLARE_WRITE8_DEVICE_HANDLER ( vt_video_dc012_w );
DECLARE_WRITE8_DEVICE_HANDLER ( vt_video_dc011_w );
DECLARE_WRITE8_DEVICE_HANDLER ( vt_video_brightness_w );


/* screen update */
void vt_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);
void rainbow_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);

#endif
