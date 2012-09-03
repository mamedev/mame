/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __TMS9927__
#define __TMS9927__

#include "devlegcy.h"


class tms9927_device : public device_t
{
public:
	tms9927_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms9927_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~tms9927_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type TMS9927;

class crt5027_device : public tms9927_device
{
public:
	crt5027_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type CRT5027;

class crt5037_device : public tms9927_device
{
public:
	crt5037_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type CRT5037;

class crt5057_device : public tms9927_device
{
public:
	crt5057_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type CRT5057;



#define MCFG_TMS9927_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, TMS9927, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_TMS9927_RECONFIG(_tag, _clock, _config) \
	MCFG_DEVICE_MODIFY(_tag) \
	MCFG_DEVICE_CLOCK(_clock) \
	MCFG_DEVICE_CONFIG(_config)



/* interface */
typedef struct _tms9927_interface tms9927_interface;
struct _tms9927_interface
{
	const char *screen_tag;			/* screen we are acting on */
	int hpixels_per_column;			/* number of pixels per video memory address */
	const char *selfload_region;	/* name of the region with self-load data */
};

extern const tms9927_interface tms9927_null_interface;


/* basic read/write handlers */
WRITE8_DEVICE_HANDLER( tms9927_w );
READ8_DEVICE_HANDLER( tms9927_r );

/* other queries */
int tms9927_screen_reset(device_t *device);
int tms9927_upscroll_offset(device_t *device);
int tms9927_cursor_bounds(device_t *device, rectangle &bounds);



#endif
