#ifndef __NAMCOIO_H__
#define __NAMCOIO_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct namcoio_interface
{
	devcb_read8 in[4];
	devcb_write8 out[2];

	device_t *device;
};

class namcoio_device : public device_t
{
public:
	namcoio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~namcoio_device() { global_free(m_token); }

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

extern const device_type NAMCO56XX;

#define NAMCO58XX NAMCO56XX
#define NAMCO59XX NAMCO56XX

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_NAMCO56XX_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO56XX, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_NAMCO58XX_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO58XX, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_NAMCO59XX_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NAMCO59XX, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

DECLARE_READ8_DEVICE_HANDLER( namcoio_r );
DECLARE_WRITE8_DEVICE_HANDLER( namcoio_w );

WRITE_LINE_DEVICE_HANDLER( namcoio_set_reset_line );
READ_LINE_DEVICE_HANDLER( namcoio_read_reset_line );


/* these must be used in the single drivers, inside a timer */
void namco_customio_56xx_run(device_t *device);
void namco_customio_58xx_run(device_t *device);
void namco_customio_59xx_run(device_t *device);


#endif  /* __NAMCOIO_H__ */
