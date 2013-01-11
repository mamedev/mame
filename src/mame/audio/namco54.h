#ifndef NAMCO54_H
#define NAMCO54_H

#include "devlegcy.h"
#include "sound/discrete.h"


struct namco_54xx_config
{
	const char *discrete;   /* name of the discrete sound device */
	int         firstnode;  /* index of the first node */
};


#define MCFG_NAMCO_54XX_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, NAMCO_54XX, _clock) \
	MCFG_DEVICE_CONFIG(_config)


DECLARE_WRITE8_DEVICE_HANDLER( namco_54xx_write );


class namco_54xx_device : public device_t
{
public:
	namco_54xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namco_54xx_device() { global_free(m_token); }

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

extern const device_type NAMCO_54XX;



/* discrete nodes */
#define NAMCO_54XX_0_DATA(base)     (NODE_RELATIVE(base, 0))
#define NAMCO_54XX_1_DATA(base)     (NODE_RELATIVE(base, 1))
#define NAMCO_54XX_2_DATA(base)     (NODE_RELATIVE(base, 2))
#define NAMCO_54XX_P_DATA(base)     (NODE_RELATIVE(base, 3))


#endif  /* NAMCO54_H */
