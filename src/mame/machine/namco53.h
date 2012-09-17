#ifndef NAMCO53_H
#define NAMCO53_H

#include "devlegcy.h"


struct namco_53xx_interface
{
	devcb_read8		k;			/* read handlers for K port */
	devcb_read8 	in[4];		/* read handlers for ports A-D */
	devcb_write8	p;			/* write handler for P port */
};


#define MCFG_NAMCO_53XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_53XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


void namco_53xx_read_request(device_t *device);
DECLARE_READ8_DEVICE_HANDLER( namco_53xx_read );


class namco_53xx_device : public device_t
{
public:
	namco_53xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_53xx_device() { global_free(m_token); }

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

extern const device_type NAMCO_53XX;



#endif	/* NAMCO53_H */
