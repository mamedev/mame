// license:BSD-2-Clause
// copyright-holders:Lubomir Rintel

#ifndef MAME_CPU_H8_H8_I2C_H
#define MAME_CPU_H8_H8_I2C_H

#pragma once

#include "h8.h"
#include "machine/i2chle.h"

class h8_i2c_device : public device_t {
public:
	h8_i2c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	h8_i2c_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: h8_i2c_device(mconfig, tag, owner, 0)
	{
	}

	// callback configuration
	auto scl_cb() { return m_scl_w.bind(); }
	auto sda_cb() { return m_sda_w.bind(); }
	auto sda_out_cb() { return m_sda_r.bind(); }

	void iccr_w(u8 data);
	u8 iccr_r();
	void icsr_w(u8 data);
	u8 icsr_r();
	void icdr_w(u8 data);
	u8 icdr_r();
	void icmr_sar_w(u8 data);
	u8 icmr_sar_r();

protected:
	devcb_write_line m_scl_w;
	devcb_write_line m_sda_w;
	devcb_read_line m_sda_r;

	u8 m_icsr_read;

	u8 m_iccr;
	u8 m_icsr;
	u8 m_icdr;
	u8 m_icmr;
	u8 m_sar;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(H8_I2C, h8_i2c_device)

#endif // MAME_CPU_H8_H8_I2C_H
