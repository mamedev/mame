// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    taitoio.h

    Implementation of various Taito custom I/O ICs

**************************************************************************/

#ifndef __TAITOIO_H__
#define __TAITOIO_H__


class tc0220ioc_device : public device_t
{
public:
	tc0220ioc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~tc0220ioc_device() {}

	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_7_callback(device_t &device, _Object object) { return downcast<tc0220ioc_device &>(device).m_read_7_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( port_r );
	DECLARE_WRITE8_MEMBER( port_w );
	DECLARE_READ8_MEMBER( portreg_r );
	DECLARE_WRITE8_MEMBER( portreg_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT8      m_regs[8];
	UINT8      m_port;

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
	tc0510nio_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~tc0510nio_device() {}

	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_7_callback(device_t &device, _Object object) { return downcast<tc0510nio_device &>(device).m_read_7_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( halfword_r );
	DECLARE_WRITE16_MEMBER( halfword_w );
	DECLARE_READ16_MEMBER( halfword_wordswap_r );
	DECLARE_WRITE16_MEMBER( halfword_wordswap_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT8   m_regs[8];

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
	tc0640fio_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~tc0640fio_device() {}

	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_7_callback(device_t &device, _Object object) { return downcast<tc0640fio_device &>(device).m_read_7_cb.set_callback(object); }


	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( halfword_r );
	DECLARE_WRITE16_MEMBER( halfword_w );
	DECLARE_READ16_MEMBER( halfword_byteswap_r );
	DECLARE_WRITE16_MEMBER( halfword_byteswap_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	private:
	// internal state
	UINT8   m_regs[8];

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
