// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#ifndef __TAITOSND_H__
#define __TAITOSND_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TC0140SYT_MASTER_CPU(_tag) \
	tc0140syt_device::set_master_tag(*device, "^" _tag);

#define MCFG_TC0140SYT_SLAVE_CPU(_tag) \
	tc0140syt_device::set_slave_tag(*device, "^" _tag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tc0140syt_device

class tc0140syt_device : public device_t
{
public:
	tc0140syt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0140syt_device() { }

	static void set_master_tag(device_t &device, const char *tag) { downcast<tc0140syt_device &>(device).m_mastercpu.set_tag(tag); }
	static void set_slave_tag(device_t &device, const char *tag)  { downcast<tc0140syt_device &>(device).m_slavecpu.set_tag(tag); }

	// MASTER (4-bit bus) control functions
	void master_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void master_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t master_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// SLAVE (4-bit bus) control functions ONLY
	void slave_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t slave_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void slave_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void update_nmi();

	uint8_t     m_slavedata[4];  /* Data on master->slave port (4 nibbles) */
	uint8_t     m_masterdata[4]; /* Data on slave->master port (4 nibbles) */
	uint8_t     m_mainmode;      /* Access mode on master cpu side */
	uint8_t     m_submode;       /* Access mode on slave cpu side */
	uint8_t     m_status;        /* Status data */
	uint8_t     m_nmi_enabled;   /* 1 if slave cpu has nmi's enabled */

	required_device<cpu_device> m_mastercpu;     /* this is the maincpu */
	required_device<cpu_device> m_slavecpu;      /* this is the audiocpu */
};

extern const device_type TC0140SYT;


#endif /*__TAITOSND_H__*/
