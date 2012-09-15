/**********************************************************************

    Signetics 2636 video chip

**********************************************************************/

#ifndef __S2636_H__
#define __S2636_H__

#include "devlegcy.h"


#define S2636_IS_PIXEL_DRAWN(p)     (((p) & 0x08) ? TRUE : FALSE)
#define S2636_PIXEL_COLOR(p)        ((p) & 0x07)

/*************************************
 *
 *  Type definitions
 *
 *************************************/

struct s2636_interface
{
	const char *screen;
	int        work_ram_size;
	int        y_offset;
	int        x_offset;
	const char *sound;
};

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

class s2636_device : public device_t
{
public:
	s2636_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s2636_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type S2636;


#define MCFG_S2636_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, S2636, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/*************************************
 *
 *  Device I/O functions
 *
 *************************************/


/* returns a BITMAP_FORMAT_IND16 bitmap the size of the screen
   D0-D2 of each pixel is the pixel color
   D3 indicates whether the S2636 drew this pixel - 0 = not drawn, 1 = drawn */

bitmap_ind16 &s2636_update( device_t *device, const rectangle &cliprect );
WRITE8_DEVICE_HANDLER( s2636_work_ram_w );
READ8_DEVICE_HANDLER( s2636_work_ram_r );


#endif /* __S2636_H__ */
