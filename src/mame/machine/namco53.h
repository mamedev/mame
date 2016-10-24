// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef NAMCO53_H
#define NAMCO53_H

#include "cpu/mb88xx/mb88xx.h"

#define MCFG_NAMCO_53XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_53XX, _clock)

#define MCFG_NAMCO_53XX_K_CB(_devcb) \
	devcb = &namco_53xx_device::set_k_port_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_0_CB(_devcb) \
	devcb = &namco_53xx_device::set_input_0_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_1_CB(_devcb) \
	devcb = &namco_53xx_device::set_input_1_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_2_CB(_devcb) \
	devcb = &namco_53xx_device::set_input_2_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_3_CB(_devcb) \
	devcb = &namco_53xx_device::set_input_3_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_P_CB(_devcb) \
	devcb = &namco_53xx_device::set_p_port_callback(*device, DEVCB_##_devcb);


class namco_53xx_device : public device_t
{
public:
	namco_53xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_input_0_callback(device_t &device, _Object object) { return downcast<namco_53xx_device &>(device).m_in_0.set_callback(object); }
	template<class _Object> static devcb_base &set_input_1_callback(device_t &device, _Object object) { return downcast<namco_53xx_device &>(device).m_in_1.set_callback(object); }
	template<class _Object> static devcb_base &set_input_2_callback(device_t &device, _Object object) { return downcast<namco_53xx_device &>(device).m_in_2.set_callback(object); }
	template<class _Object> static devcb_base &set_input_3_callback(device_t &device, _Object object) { return downcast<namco_53xx_device &>(device).m_in_3.set_callback(object); }

	template<class _Object> static devcb_base &set_k_port_callback(device_t &device, _Object object) { return downcast<namco_53xx_device &>(device).m_k.set_callback(object); }
	template<class _Object> static devcb_base &set_p_port_callback(device_t &device, _Object object) { return downcast<namco_53xx_device &>(device).m_p.set_callback(object); }

	uint8_t K_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t Rx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void O_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void P_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void read_request(int state);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	void irq_clear(void *ptr, int32_t param);
private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	uint8_t           m_portO;
	devcb_read8    m_k;
	devcb_read8    m_in_0;
	devcb_read8    m_in_1;
	devcb_read8    m_in_2;
	devcb_read8    m_in_3;
	devcb_write8   m_p;

};

extern const device_type NAMCO_53XX;



#endif  /* NAMCO53_H */
