#ifndef NAMCO52_H
#define NAMCO52_H

#include "devlegcy.h"
#include "sound/discrete.h"
#include "devcb.h"


struct namco_52xx_interface
{
	const char *	discrete;	/* name of the discrete sound device */
	int				firstnode;	/* index of the first node */
	attoseconds_t	extclock;	/* external clock period */
	devcb_read8		romread;	/* ROM read handler */
	devcb_read8 	si;			/* SI (pin 6) read handler */
};


#define MCFG_NAMCO_52XX_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO_52XX, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


DECLARE_WRITE8_DEVICE_HANDLER( namco_52xx_write );


class namco_52xx_device : public device_t
{
public:
	namco_52xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_52xx_device() { global_free(m_token); }

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

extern const device_type NAMCO_52XX;



/* discrete nodes */
#define NAMCO_52XX_P_DATA(base)		(base)


#endif	/* NAMCO52_H */
