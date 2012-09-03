#ifndef NAMCO51_H
#define NAMCO51_H

#include "devlegcy.h"


typedef struct _namco_51xx_interface namco_51xx_interface;
struct _namco_51xx_interface
{
	devcb_read8 	in[4];		/* read handlers for ports A-D */
	devcb_write8	out[2];		/* write handlers for ports A-B */
};


#define MCFG_NAMCO_51XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_51XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


READ8_DEVICE_HANDLER( namco_51xx_read );
WRITE8_DEVICE_HANDLER( namco_51xx_write );


class namco_51xx_device : public device_t
{
public:
	namco_51xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_51xx_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
	// internal state
	void *m_token;
};

extern const device_type NAMCO_51XX;



#endif	/* NAMCO51_H */
