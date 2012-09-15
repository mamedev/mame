#ifndef NAMCO62_H
#define NAMCO62_H

#include "devlegcy.h"


struct namco_62xx_interface
{
	devcb_read8 	in[4];		/* read handlers for ports A-D */
	devcb_write8	out[2];		/* write handlers for ports A-B */
};


#define MCFG_NAMCO_62XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_62XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


READ8_DEVICE_HANDLER( namco_62xx_read );
WRITE8_DEVICE_HANDLER( namco_62xx_write );


class namco_62xx_device : public device_t
{
public:
	namco_62xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_62xx_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
	// internal state
	void *m_token;
};

extern const device_type NAMCO_62XX;



#endif	/* NAMCO62_H */
