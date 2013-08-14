#ifndef __TAITOSND_H__
#define __TAITOSND_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TC0140SYT_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0140SYT, 0) \
	MCFG_DEVICE_CONFIG(_interface)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct tc0140syt_interface
{
	const char         *master;
	const char         *slave;
};


// ======================> tc0140syt_device

class tc0140syt_device : public device_t
{
public:
	tc0140syt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0140syt_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

public:
	// MASTER (4-bit bus) control functions
	DECLARE_WRITE8_MEMBER( tc0140syt_port_w );
	DECLARE_WRITE8_MEMBER( tc0140syt_comm_w );
	DECLARE_READ8_MEMBER( tc0140syt_comm_r );

	// SLAVE (4-bit bus) control functions ONLY
	DECLARE_WRITE8_MEMBER( tc0140syt_slave_port_w );
	DECLARE_READ8_MEMBER( tc0140syt_slave_comm_r );
	DECLARE_WRITE8_MEMBER( tc0140syt_slave_comm_w );

private:
	void update_nmi();

private:
	UINT8     m_slavedata[4];  /* Data on master->slave port (4 nibbles) */
	UINT8     m_masterdata[4]; /* Data on slave->master port (4 nibbles) */
	UINT8     m_mainmode;      /* Access mode on master cpu side */
	UINT8     m_submode;       /* Access mode on slave cpu side */
	UINT8     m_status;        /* Status data */
	UINT8     m_nmi_enabled;   /* 1 if slave cpu has nmi's enabled */

	device_t *m_mastercpu;     /* this is the maincpu */
	device_t *m_slavecpu;      /* this is the audiocpu */
};

extern const device_type TC0140SYT;


#endif /*__TAITOSND_H__*/
