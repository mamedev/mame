#ifndef __TAITOSND_H__
#define __TAITOSND_H__

#include "devlegcy.h"
#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct tc0140syt_interface
{
	const char         *master;
	const char         *slave;
};

class tc0140syt_device : public device_t
{
public:
	tc0140syt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0140syt_device() { global_free(m_token); }

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

extern const device_type TC0140SYT;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TC0140SYT_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0140SYT, 0) \
	MCFG_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/* MASTER (8bit bus) control functions */
DECLARE_WRITE8_DEVICE_HANDLER( tc0140syt_port_w );
DECLARE_WRITE8_DEVICE_HANDLER( tc0140syt_comm_w );
DECLARE_READ8_DEVICE_HANDLER( tc0140syt_comm_r );


/* SLAVE (8bit bus) control functions ONLY */
DECLARE_WRITE8_DEVICE_HANDLER( tc0140syt_slave_port_w );
DECLARE_READ8_DEVICE_HANDLER( tc0140syt_slave_comm_r );
DECLARE_WRITE8_DEVICE_HANDLER( tc0140syt_slave_comm_w );


#endif /*__TAITOSND_H__*/
