// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_NAMCO53_H
#define MAME_MACHINE_NAMCO53_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"

#define MCFG_NAMCO_53XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_53XX, _clock)

#define MCFG_NAMCO_53XX_K_CB(_devcb) \
	devcb = &downcast<namco_53xx_device &>(*device).set_k_port_callback(DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_0_CB(_devcb) \
	devcb = &downcast<namco_53xx_device &>(*device).set_input_callback<0>(DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_1_CB(_devcb) \
	devcb = &downcast<namco_53xx_device &>(*device).set_input_callback<1>(DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_2_CB(_devcb) \
	devcb = &downcast<namco_53xx_device &>(*device).set_input_callback<2>(DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_INPUT_3_CB(_devcb) \
	devcb = &downcast<namco_53xx_device &>(*device).set_input_callback<3>(DEVCB_##_devcb);

#define MCFG_NAMCO_53XX_P_CB(_devcb) \
	devcb = &downcast<namco_53xx_device &>(*device).set_p_port_callback(DEVCB_##_devcb);


class namco_53xx_device : public device_t
{
public:
	namco_53xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N, class Object> devcb_base &set_input_callback(Object &&cb) { return m_in[N].set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_k_port_callback(Object &&cb) { return m_k.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_p_port_callback(Object &&cb) { return m_p.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( K_r );
	DECLARE_READ8_MEMBER( R0_r );
	DECLARE_READ8_MEMBER( R1_r );
	DECLARE_READ8_MEMBER( R2_r );
	DECLARE_READ8_MEMBER( R3_r );
	DECLARE_WRITE8_MEMBER( O_w );
	DECLARE_WRITE8_MEMBER( P_w );

	DECLARE_WRITE_LINE_MEMBER(read_request);
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	TIMER_CALLBACK_MEMBER( irq_clear );
private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	uint8_t           m_portO;
	devcb_read8    m_k;
	devcb_read8    m_in[4];
	devcb_write8   m_p;

};

DECLARE_DEVICE_TYPE(NAMCO_53XX, namco_53xx_device)


#endif // MAME_MACHINE_NAMCO53_H
