/*************************************************************************

    taitoio.h

    Implementation of various Taito custom I/O ICs

**************************************************************************/

#ifndef __TAITOIO_H__
#define __TAITOIO_H__

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct tc0220ioc_interface
{
	devcb_read8 m_read_0;
	devcb_read8 m_read_1;
	devcb_read8 m_read_2;
	devcb_read8 m_read_3;
	devcb_read8 m_read_7;
};


struct tc0510nio_interface
{
	devcb_read8 m_read_0;
	devcb_read8 m_read_1;
	devcb_read8 m_read_2;
	devcb_read8 m_read_3;
	devcb_read8 m_read_7;
};


struct tc0640fio_interface
{
	devcb_read8 m_read_0;
	devcb_read8 m_read_1;
	devcb_read8 m_read_2;
	devcb_read8 m_read_3;
	devcb_read8 m_read_7;
};

class tc0220ioc_device : public device_t,
										public tc0220ioc_interface
{
public:
	tc0220ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0220ioc_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( port_r );
	DECLARE_WRITE8_MEMBER( port_w );
	DECLARE_READ8_MEMBER( portreg_r );
	DECLARE_WRITE8_MEMBER( portreg_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT8      m_regs[8];
	UINT8      m_port;

	devcb_resolved_read8    m_read_0_func;
	devcb_resolved_read8    m_read_1_func;
	devcb_resolved_read8    m_read_2_func;
	devcb_resolved_read8    m_read_3_func;
	devcb_resolved_read8    m_read_7_func;
};

extern const device_type TC0220IOC;

class tc0510nio_device : public device_t,
										public tc0510nio_interface
{
public:
	tc0510nio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0510nio_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( halfword_r );
	DECLARE_WRITE16_MEMBER( halfword_w );
	DECLARE_READ16_MEMBER( halfword_wordswap_r );
	DECLARE_WRITE16_MEMBER( halfword_wordswap_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT8   m_regs[8];

	devcb_resolved_read8    m_read_0_func;
	devcb_resolved_read8    m_read_1_func;
	devcb_resolved_read8    m_read_2_func;
	devcb_resolved_read8    m_read_3_func;
	devcb_resolved_read8    m_read_7_func;
};

extern const device_type TC0510NIO;

class tc0640fio_device : public device_t,
										public tc0640fio_interface
{
public:
	tc0640fio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0640fio_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( halfword_r );
	DECLARE_WRITE16_MEMBER( halfword_w );
	DECLARE_READ16_MEMBER( halfword_byteswap_r );
	DECLARE_WRITE16_MEMBER( halfword_byteswap_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	private:
	// internal state
	UINT8   m_regs[8];

	devcb_resolved_read8    m_read_0_func;
	devcb_resolved_read8    m_read_1_func;
	devcb_resolved_read8    m_read_2_func;
	devcb_resolved_read8    m_read_3_func;
	devcb_resolved_read8    m_read_7_func;
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


#endif  /* __TAITOIO_H__ */
