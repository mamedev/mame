// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef NAMCO06_H
#define NAMCO06_H


struct namco_06xx_config
{
	const char *nmicpu;
	const char *chip0;
	const char *chip1;
	const char *chip2;
	const char *chip3;
};


#define MCFG_NAMCO_06XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_06XX, _clock)

#define MCFG_NAMCO_06XX_MAINCPU(_tag) \
	namco_06xx_device::set_maincpu(*device, "^" _tag);

#define MCFG_NAMCO_06XX_READ_0_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_0_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_1_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_1_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_2_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_2_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_3_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_3_callback(*device, DEVCB_##_devcb);


#define MCFG_NAMCO_06XX_READ_REQUEST_0_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_request_0_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_REQUEST_1_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_request_1_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_REQUEST_2_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_request_2_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_READ_REQUEST_3_CB(_devcb) \
	devcb = &namco_06xx_device::set_read_request_3_callback(*device, DEVCB_##_devcb);


#define MCFG_NAMCO_06XX_WRITE_0_CB(_devcb) \
	devcb = &namco_06xx_device::set_write_0_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_WRITE_1_CB(_devcb) \
	devcb = &namco_06xx_device::set_write_1_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_WRITE_2_CB(_devcb) \
	devcb = &namco_06xx_device::set_write_2_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_06XX_WRITE_3_CB(_devcb) \
	devcb = &namco_06xx_device::set_write_3_callback(*device, DEVCB_##_devcb);


/* device get info callback */
class namco_06xx_device : public device_t
{
public:
	namco_06xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_maincpu(device_t &device, const char *tag) { downcast<namco_06xx_device &>(device).m_nmicpu.set_tag(tag); }

	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_read_0.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_read_1.set_callback(object); }
	template<class _Object> static devcb_base &set_read_2_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_read_2.set_callback(object); }
	template<class _Object> static devcb_base &set_read_3_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_read_3.set_callback(object); }

	template<class _Object> static devcb_base &set_read_request_0_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_readreq_0.set_callback(object); }
	template<class _Object> static devcb_base &set_read_request_1_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_readreq_1.set_callback(object); }
	template<class _Object> static devcb_base &set_read_request_2_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_readreq_2.set_callback(object); }
	template<class _Object> static devcb_base &set_read_request_3_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_readreq_3.set_callback(object); }


	template<class _Object> static devcb_base &set_write_0_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_write_0.set_callback(object); }
	template<class _Object> static devcb_base &set_write_1_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_write_1.set_callback(object); }
	template<class _Object> static devcb_base &set_write_2_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_write_2.set_callback(object); }
	template<class _Object> static devcb_base &set_write_3_callback(device_t &device, _Object object) { return downcast<namco_06xx_device &>(device).m_write_3.set_callback(object); }

	uint8_t data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:

	void nmi_generate(void *ptr, int32_t param);

	// internal state
	uint8_t m_control;
	emu_timer *m_nmi_timer;

	required_device<cpu_device> m_nmicpu;

	devcb_read8 m_read_0;
	devcb_read8 m_read_1;
	devcb_read8 m_read_2;
	devcb_read8 m_read_3;

	devcb_write_line m_readreq_0;
	devcb_write_line m_readreq_1;
	devcb_write_line m_readreq_2;
	devcb_write_line m_readreq_3;

	devcb_write8 m_write_0;
	devcb_write8 m_write_1;
	devcb_write8 m_write_2;
	devcb_write8 m_write_3;
};

extern const device_type NAMCO_06XX;



#endif
