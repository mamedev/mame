// license:BSD-3-Clause
// copyright-holders:David Haywood, Vas Crabb
#ifndef MAME_SHARED_TAITO68705_H
#define MAME_SHARED_TAITO68705_H

#pragma once

#include "cpu/m6805/m68705.h"


DECLARE_DEVICE_TYPE(TAITO68705_MCU,       taito68705_mcu_device)
DECLARE_DEVICE_TYPE(TAITO68705_MCU_TIGER, taito68705_mcu_tiger_device)
DECLARE_DEVICE_TYPE(ARKANOID_68705P3,     arkanoid_68705p3_device)
DECLARE_DEVICE_TYPE(ARKANOID_68705P5,     arkanoid_68705p5_device)


class taito68705_mcu_device_base : public device_t
{
public:
	// host interface
	u8 data_r();
	void data_w(u8 data);
	void reset_w(int state);
	int host_semaphore_r() { return m_host_flag ? 1 : 0; }
	int mcu_semaphore_r() { return m_mcu_flag ? 1 : 0; }

protected:
	taito68705_mcu_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);

	auto semaphore_cb() { return m_semaphore_cb.bind(); }

	// MCU callbacks
	void mcu_pa_w(u8 data);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	bool host_flag() const { return m_host_flag; }
	bool mcu_flag() const { return m_mcu_flag; }
	u8 pa_value() const { return m_pa_output & (m_latch_driven ? m_host_latch : 0xff); }
	void latch_control(u8 data, u8 &value, unsigned host_bit, unsigned mcu_bit);

	required_device<m68705p_device> m_mcu;

private:
	devcb_write_line    m_semaphore_cb;

	bool    m_latch_driven;
	bool    m_reset_input;
	bool    m_host_flag;
	bool    m_mcu_flag;
	u8      m_host_latch;
	u8      m_mcu_latch;
	u8      m_pa_output;
};


class taito68705_mcu_device : public taito68705_mcu_device_base
{
public:
	template <unsigned N> auto aux_out_cb() { return m_aux_out_cb[N].bind(); }
	auto aux_strobe_cb() { return m_aux_strobe_cb.bind(); }

	taito68705_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	taito68705_mcu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual u8 mcu_portc_r();
	void mcu_portb_w(offs_t offset, u8 data, u8 mem_mask = ~0);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	devcb_write_line::array<6>  m_aux_out_cb;
	devcb_write8                m_aux_strobe_cb;

	u8  m_pb_output;
};


class taito68705_mcu_tiger_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_tiger_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u8 mcu_portc_r() override;
};


class arkanoid_mcu_device_base : public taito68705_mcu_device_base
{
public:
	using taito68705_mcu_device_base::semaphore_cb;
	auto portb_r_cb() { return m_portb_r_cb.bind(); }

protected:
	arkanoid_mcu_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);

	// MCU callbacks
	u8 mcu_pb_r();
	u8 mcu_pc_r();
	void mcu_pc_w(u8 data);

	virtual void device_start() override ATTR_COLD;

	devcb_read8 m_portb_r_cb;

	u8  m_pc_output;
};


class arkanoid_68705p3_device : public arkanoid_mcu_device_base
{
public:
	arkanoid_68705p3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class arkanoid_68705p5_device : public arkanoid_mcu_device_base
{
public:
	arkanoid_68705p5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

#endif // MAME_SHARED_TAITO68705_H
