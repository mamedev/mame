/*****************************************************************************
 *
 * video/vdc8563.h
 *
 * CBM Video Device Chip 8563
 *
 * peter.trauner@jk.uni-linz.ac.at, 2000
 *
 ****************************************************************************/

#ifndef __VDC8563_H__
#define __VDC8563_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _vdc8563_interface vdc8563_interface;
struct _vdc8563_interface
{
	const char         *screen;
	int                ram16konly;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class vdc8563_device : public device_t
{
public:
	vdc8563_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vdc8563_device() { global_free(m_token); }

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

extern const device_type VDC8563;


#define MCFG_VDC8563_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, VDC8563, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/*----------- defined in video/vdc8563.c -----------*/

void vdc8563_set_rastering(device_t *device, int on);
UINT32 vdc8563_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);

WRITE8_DEVICE_HANDLER( vdc8563_port_w );
READ8_DEVICE_HANDLER( vdc8563_port_r );


#endif /* __VDC8563_H__ */
