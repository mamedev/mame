// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    taitoio.h

    Implementation of various Taito custom I/O ICs

**************************************************************************/

#ifndef __TAITOIO_H__
#define __TAITOIO_H__

#include "machine/watchdog.h"


class tc0220ioc_device : public device_t
{
public:
	tc0220ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0220ioc_device() {}

	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_7_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_7_cb.set_callback(object); }

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portreg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	// internal state
	uint8_t      m_regs[8];
	uint8_t      m_port;

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_read8 m_read_7_cb;
};

extern const device_type TC0220IOC;

class tc0510nio_device : public device_t
{
public:
	tc0510nio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0510nio_device() {}

	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_7_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_7_cb.set_callback(object); }

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t halfword_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void halfword_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t halfword_wordswap_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void halfword_wordswap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	// internal state
	uint8_t   m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_read8 m_read_7_cb;
};

extern const device_type TC0510NIO;

class tc0640fio_device : public device_t
{
public:
	tc0640fio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0640fio_device() {}

	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_7_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_7_cb.set_callback(object); }


	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t halfword_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void halfword_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t halfword_byteswap_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void halfword_byteswap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	private:
	// internal state
	uint8_t   m_regs[8];

	required_device<watchdog_timer_device> m_watchdog;

	devcb_read8 m_read_0_cb;
	devcb_read8 m_read_1_cb;
	devcb_read8 m_read_2_cb;
	devcb_read8 m_read_3_cb;
	devcb_read8 m_read_7_cb;
};

extern const device_type TC0640FIO;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TC0220IOC_READ_0_CB(_devcb) \
	devcb = &tc0220ioc_device::set_read_0_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_1_CB(_devcb) \
	devcb = &tc0220ioc_device::set_read_1_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_2_CB(_devcb) \
	devcb = &tc0220ioc_device::set_read_2_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_3_CB(_devcb) \
	devcb = &tc0220ioc_device::set_read_3_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0220IOC_READ_7_CB(_devcb) \
	devcb = &tc0220ioc_device::set_read_7_callback(*device, DEVCB_##_devcb);


#define MCFG_TC0510NIO_READ_0_CB(_devcb) \
	devcb = &tc0510nio_device::set_read_0_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_1_CB(_devcb) \
	devcb = &tc0510nio_device::set_read_1_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_2_CB(_devcb) \
	devcb = &tc0510nio_device::set_read_2_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_3_CB(_devcb) \
	devcb = &tc0510nio_device::set_read_3_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0510NIO_READ_7_CB(_devcb) \
	devcb = &tc0510nio_device::set_read_7_callback(*device, DEVCB_##_devcb);


#define MCFG_TC0640FIO_READ_0_CB(_devcb) \
	devcb = &tc0640fio_device::set_read_0_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_1_CB(_devcb) \
	devcb = &tc0640fio_device::set_read_1_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_2_CB(_devcb) \
	devcb = &tc0640fio_device::set_read_2_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_3_CB(_devcb) \
	devcb = &tc0640fio_device::set_read_3_callback(*device, DEVCB_##_devcb);

#define MCFG_TC0640FIO_READ_7_CB(_devcb) \
	devcb = &tc0640fio_device::set_read_7_callback(*device, DEVCB_##_devcb);


#endif  /* __TAITOIO_H__ */
