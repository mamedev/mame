/*************************************************************************

    taitoio.h

    Implementation of various Taito custom I/O ICs

**************************************************************************/

#ifndef __TAITOIO_H__
#define __TAITOIO_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct tc0220ioc_interface
{
	devcb_read8 read_0;
	devcb_read8 read_1;
	devcb_read8 read_2;
	devcb_read8 read_3;
	devcb_read8 read_7;
};


struct tc0510nio_interface
{
	devcb_read8 read_0;
	devcb_read8 read_1;
	devcb_read8 read_2;
	devcb_read8 read_3;
	devcb_read8 read_7;
};


struct tc0640fio_interface
{
	devcb_read8 read_0;
	devcb_read8 read_1;
	devcb_read8 read_2;
	devcb_read8 read_3;
	devcb_read8 read_7;
};

class tc0220ioc_device : public device_t
{
public:
	tc0220ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0220ioc_device() { global_free(m_token); }

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

extern const device_type TC0220IOC;

class tc0510nio_device : public device_t
{
public:
	tc0510nio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0510nio_device() { global_free(m_token); }

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

extern const device_type TC0510NIO;

class tc0640fio_device : public device_t
{
public:
	tc0640fio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0640fio_device() { global_free(m_token); }

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

extern const device_type TC0640FIO;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TC0220IOC_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0220IOC, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0510NIO_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0510NIO, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0640FIO_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0640FIO, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/** TC0220IOC **/
DECLARE_READ8_DEVICE_HANDLER( tc0220ioc_r );
DECLARE_WRITE8_DEVICE_HANDLER( tc0220ioc_w );
DECLARE_READ8_DEVICE_HANDLER( tc0220ioc_port_r );
DECLARE_WRITE8_DEVICE_HANDLER( tc0220ioc_port_w );
DECLARE_READ8_DEVICE_HANDLER( tc0220ioc_portreg_r );
DECLARE_WRITE8_DEVICE_HANDLER( tc0220ioc_portreg_w );


/** TC0510NIO **/
DECLARE_READ8_DEVICE_HANDLER( tc0510nio_r );
DECLARE_WRITE8_DEVICE_HANDLER( tc0510nio_w );
DECLARE_READ16_DEVICE_HANDLER( tc0510nio_halfword_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0510nio_halfword_w );
DECLARE_READ16_DEVICE_HANDLER( tc0510nio_halfword_wordswap_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0510nio_halfword_wordswap_w );


/** TC0640FIO**/
DECLARE_READ8_DEVICE_HANDLER( tc0640fio_r );
DECLARE_WRITE8_DEVICE_HANDLER( tc0640fio_w );
DECLARE_READ16_DEVICE_HANDLER( tc0640fio_halfword_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0640fio_halfword_w );
DECLARE_READ16_DEVICE_HANDLER( tc0640fio_halfword_byteswap_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0640fio_halfword_byteswap_w );


#endif	/* __TAITOIO_H__ */
