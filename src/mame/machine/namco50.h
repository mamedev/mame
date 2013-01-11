#ifndef NAMCO50_H
#define NAMCO50_H

#include "devlegcy.h"


#define MCFG_NAMCO_50XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_50XX, _clock) \


DECLARE_READ8_DEVICE_HANDLER( namco_50xx_read );
void namco_50xx_read_request(device_t *device);
DECLARE_WRITE8_DEVICE_HANDLER( namco_50xx_write );


/* device get info callback */
class namco_50xx_device : public device_t
{
public:
	namco_50xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_50xx_device() { global_free(m_token); }

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

extern const device_type NAMCO_50XX;



#endif  /* NAMCO50_H */
