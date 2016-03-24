// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#ifndef NAMCO62_H
#define NAMCO62_H

#include "cpu/mb88xx/mb88xx.h"

#define MCFG_NAMCO_62XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_62XX, _clock)

#define MCFG_NAMCO_62XX_INPUT_0_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_0_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_INPUT_1_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_1_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_INPUT_2_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_2_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_INPUT_3_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_3_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_OUTPUT_0_CB(_devcb) \
	devcb = &namco_62xx_device::set_output_0_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_OUTPUT_1_CB(_devcb) \
	devcb = &namco_62xx_device::set_output_1_callback(*device, DEVCB_##_devcb);


class namco_62xx_device : public device_t
{
public:
	namco_62xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_input_0_callback(device_t &device, _Object object) { return downcast<namco_62xx_device &>(device).m_in_0.set_callback(object); }
	template<class _Object> static devcb_base &set_input_1_callback(device_t &device, _Object object) { return downcast<namco_62xx_device &>(device).m_in_1.set_callback(object); }
	template<class _Object> static devcb_base &set_input_2_callback(device_t &device, _Object object) { return downcast<namco_62xx_device &>(device).m_in_2.set_callback(object); }
	template<class _Object> static devcb_base &set_input_3_callback(device_t &device, _Object object) { return downcast<namco_62xx_device &>(device).m_in_3.set_callback(object); }

	template<class _Object> static devcb_base &set_output_0_callback(device_t &device, _Object object) { return downcast<namco_62xx_device &>(device).m_out_0.set_callback(object); }
	template<class _Object> static devcb_base &set_output_1_callback(device_t &device, _Object object) { return downcast<namco_62xx_device &>(device).m_out_1.set_callback(object); }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	devcb_read8 m_in_0;
	devcb_read8 m_in_1;
	devcb_read8 m_in_2;
	devcb_read8 m_in_3;
	devcb_write8 m_out_0;
	devcb_write8 m_out_1;
};

extern const device_type NAMCO_62XX;

#endif  /* NAMCO62_H */
