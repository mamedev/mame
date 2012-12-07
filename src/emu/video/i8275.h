/***************************************************************************

    INTEL 8275 Programmable CRT Controller implementation

    25-05-2008 Initial implementation [Miodrag Milanovic]

    Copyright MESS team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __I8275_VIDEO__
#define __I8275_VIDEO__

#include "devcb.h"

/***************************************************************************
    MACROS
***************************************************************************/

class i8275_device : public device_t
{
public:
	i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~i8275_device() { global_free(m_token); }

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

extern const device_type I8275;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*i8275_display_pixels_func)(device_t *device, int x, int y, UINT8 linecount, UINT8 charcode, UINT8 lineattr, UINT8 lten, UINT8 rvv, UINT8 vsp, UINT8 gpa, UINT8 hlgt);
#define I8275_DISPLAY_PIXELS(name)	void name(device_t *device, int x, int y, UINT8 linecount, UINT8 charcode, UINT8 lineattr, UINT8 lten, UINT8 rvv, UINT8 vsp, UINT8 gpa, UINT8 hlgt)

/* interface */
struct i8275_interface
{
	const char *screen_tag;		/* screen we are acting on */
	int width;					/* char width in pixels */
	int char_delay;				/* delay of display char */

	devcb_write_line out_drq_func;

	devcb_write_line out_irq_func;

	i8275_display_pixels_func display_pixels;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* register access */
DECLARE_READ8_DEVICE_HANDLER ( i8275_r );
DECLARE_WRITE8_DEVICE_HANDLER ( i8275_w );

/* updates the screen */
void i8275_update(device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

DECLARE_WRITE8_DEVICE_HANDLER( i8275_dack_w );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8275_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, I8275, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif
