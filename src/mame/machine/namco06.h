#ifndef NAMCO06_H
#define NAMCO06_H

#include "devlegcy.h"


typedef struct _namco_06xx_config namco_06xx_config;
struct _namco_06xx_config
{
	const char *nmicpu;
	const char *chip0;
	const char *chip1;
	const char *chip2;
	const char *chip3;
};


#define MCFG_NAMCO_06XX_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, NAMCO_06XX, _clock) \
	MCFG_DEVICE_CONFIG(_config)


READ8_DEVICE_HANDLER( namco_06xx_data_r );
WRITE8_DEVICE_HANDLER( namco_06xx_data_w );
READ8_DEVICE_HANDLER( namco_06xx_ctrl_r );
WRITE8_DEVICE_HANDLER( namco_06xx_ctrl_w );


/* device get info callback */
class namco_06xx_device : public device_t
{
public:
	namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_06xx_device() { global_free(m_token); }

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

extern const device_type NAMCO_06XX;



#endif
