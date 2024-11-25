// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#ifndef MAME_SHARED_TAITOSND_H
#define MAME_SHARED_TAITOSND_H


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tc0140syt_device

class tc0140syt_device : public device_t
{
public:
	tc0140syt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto nmi_callback() { return m_nmi_cb.bind(); }
	auto reset_callback() { return m_reset_cb.bind(); }

	// MASTER (4-bit bus) control functions
	void master_port_w(u8 data);
	void master_comm_w(u8 data);
	u8 master_comm_r();

	// SLAVE (4-bit bus) control functions ONLY
	void slave_port_w(u8 data);
	u8 slave_comm_r();
	void slave_comm_w(u8 data);

protected:
	tc0140syt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_nmi();

	u8     m_slavedata[4];  /* Data on master->slave port (4 nibbles) */
	u8     m_masterdata[4]; /* Data on slave->master port (4 nibbles) */
	u8     m_mainmode;      /* Access mode on master cpu side */
	u8     m_submode;       /* Access mode on slave cpu side */
	u8     m_status;        /* Status data */
	u8     m_nmi_enabled;   /* 1 if slave cpu has nmi's enabled */

	devcb_write_line m_nmi_cb;
	devcb_write_line m_reset_cb;
};

// ======================> pc060ha_device

class pc060ha_device : public tc0140syt_device
{
public:
	pc060ha_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(TC0140SYT, tc0140syt_device)
DECLARE_DEVICE_TYPE(PC060HA, pc060ha_device)


#endif // MAME_SHARED_TAITOSND_H
