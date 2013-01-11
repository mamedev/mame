/**********************************************************************

    Hudson/NEC HuC6260 interface and definitions

**********************************************************************/


#ifndef __HUC6260_H_
#define __HUC6260_H_

#include "emu.h"


#define HUC6260_PALETTE_SIZE    1024

/* Screen timing stuff */
#define HUC6260_WPF         1365    /* width of a line in frame including blanking areas */
#define HUC6260_LPF         263     /* max number of lines in a single frame */


PALETTE_INIT( huc6260 );


#define MCFG_HUC6260_ADD( _tag, clock, _intrf ) \
	MCFG_DEVICE_ADD( _tag, HUC6260, clock )     \
	MCFG_DEVICE_CONFIG( _intrf )


struct huc6260_interface
{
	/* Tag for the screen we will be drawing on */
	const char *screen_tag;

	/* Callback function to retrieve pixel data */
	devcb_read16                    get_next_pixel_data;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16                    get_time_til_next_event;

	/* Callback function which gets called when vsync changes */
	devcb_write_line                vsync_changed;

	/* Callback function which gets called when hsync changes */
	devcb_write_line                hsync_changed;
};


class huc6260_device :  public device_t,
						public huc6260_interface
{
public:
	// construction/destruction
	huc6260_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	screen_device *m_screen;
	int     m_last_h;
	int     m_last_v;
	int     m_height;

	/* callbacks */
	devcb_resolved_read16       m_get_next_pixel_data;
	devcb_resolved_read16       m_get_time_til_next_event;
	devcb_resolved_write_line   m_hsync_changed;
	devcb_resolved_write_line   m_vsync_changed;

	UINT16  m_palette[512];
	UINT16  m_address;
	UINT8   m_greyscales;       /* Should the HuC6260 output grey or color graphics */
	UINT8   m_blur;             /* Should the edges of graphics be blurred/Select screen height 0=262, 1=263 */
	UINT8   m_pixels_per_clock; /* Number of pixels to output per colour clock */
	UINT16  m_pixel_data;
	UINT8   m_pixel_clock;

	emu_timer   *m_timer;
	bitmap_ind16    *m_bmp;
};


extern const device_type HUC6260;


#endif
