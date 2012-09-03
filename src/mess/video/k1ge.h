
#ifndef __K2GE_H_
#define __K2GE_H_

#include "devcb.h"


#define K1GE_SCREEN_HEIGHT	199


class k1ge_device : public device_t
{
public:
	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	k1ge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~k1ge_device() { global_free(m_token); }

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

extern const device_type K1GE;

class k2ge_device : public k1ge_device
{
public:
	k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type K2GE;



#define MCFG_K1GE_ADD(_tag, _clock, _config ) \
	MCFG_DEVICE_ADD( _tag, K1GE, _clock ) \
	MCFG_DEVICE_CONFIG( _config )


#define MCFG_K2GE_ADD(_tag, _clock, _config ) \
	MCFG_DEVICE_ADD( _tag, K2GE, _clock ) \
	MCFG_DEVICE_CONFIG( _config )


typedef struct _k1ge_interface k1ge_interface;
struct _k1ge_interface
{
	const char		*screen_tag;		/* screen we are drawing on */
	const char		*vram_tag;			/* memory region we will use for video ram */
	devcb_write8	vblank_pin_w;		/* called back when VBlank pin may have changed */
	devcb_write8	hblank_pin_w;		/* called back when HBlank pin may have changed */
};


PALETTE_INIT( k1ge );
PALETTE_INIT( k2ge );

WRITE8_DEVICE_HANDLER( k1ge_w );
READ8_DEVICE_HANDLER( k1ge_r );

void k1ge_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect );

#endif

