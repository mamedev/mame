// license:BSD-3-Clause
// copyright-holders:Philip Bennett
#ifndef MAME_MACHINE_NAMCO62_H
#define MAME_MACHINE_NAMCO62_H

#include "cpu/mb88xx/mb88xx.h"

#define MCFG_NAMCO_62XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_62XX, _clock)

#define MCFG_NAMCO_62XX_INPUT_0_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_callback<0>(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_INPUT_1_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_callback<1>(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_INPUT_2_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_callback<2>(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_INPUT_3_CB(_devcb) \
	devcb = &namco_62xx_device::set_input_callback<3>(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_OUTPUT_0_CB(_devcb) \
	devcb = &namco_62xx_device::set_output_callback<0>(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_62XX_OUTPUT_1_CB(_devcb) \
	devcb = &namco_62xx_device::set_output_callback<1>(*device, DEVCB_##_devcb);


class namco_62xx_device : public device_t
{
public:
	namco_62xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N, class Object> static devcb_base &set_input_callback(device_t &device, Object &&cb) { return downcast<namco_62xx_device &>(device).m_in[N].set_callback(std::forward<Object>(cb)); }

	template <unsigned N, class Object> static devcb_base &set_output_callback(device_t &device, Object &&cb) { return downcast<namco_62xx_device &>(device).m_out[N].set_callback(std::forward<Object>(cb)); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	devcb_read8 m_in[4];
	devcb_write8 m_out[2];
};

DECLARE_DEVICE_TYPE(NAMCO_62XX, namco_62xx_device)

#endif // MAME_MACHINE_NAMCO62_H
