// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Fujitsu MB89363 Parallel Communication Interface
    (this acts as a trampoline to 2x i8255 chips)

***************************************************************************/

#ifndef MAME_MACHINE_MB89363B_H
#define MAME_MACHINE_MB89363B_H

#pragma once

#include "machine/i8255.h"


DECLARE_DEVICE_TYPE(MB89363B, mb89363b_device)

class mb89363b_device :  public device_t
{
public:
	// construction/destruction
	mb89363b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	auto in_pa() { return m_in_a_pa_cb.bind(); }
	auto in_pb() { return m_in_a_pb_cb.bind(); }
	auto in_pc() { return m_in_a_pc_cb.bind(); }
	auto out_pa() { return m_out_a_pa_cb.bind(); }
	auto out_pb() { return m_out_a_pb_cb.bind(); }
	auto out_pc() { return m_out_a_pc_cb.bind(); }

	auto in_pd() { return m_in_b_pa_cb.bind(); }
	auto in_pe() { return m_in_b_pb_cb.bind(); }
	auto in_pf() { return m_in_b_pc_cb.bind(); }
	auto out_pd() { return m_out_b_pa_cb.bind(); }
	auto out_pe() { return m_out_b_pb_cb.bind(); }
	auto out_pf() { return m_out_b_pc_cb.bind(); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t i8255_a_port_a_r(offs_t offset);
	uint8_t i8255_a_port_b_r(offs_t offset);
	uint8_t i8255_a_port_c_r(offs_t offset);
	void i8255_a_port_a_w(offs_t offset, uint8_t data);
	void i8255_a_port_b_w(offs_t offset, uint8_t data);
	void i8255_a_port_c_w(offs_t offset, uint8_t data);
	uint8_t i8255_b_port_a_r(offs_t offset);
	uint8_t i8255_b_port_b_r(offs_t offset);
	uint8_t i8255_b_port_c_r(offs_t offset);
	void i8255_b_port_a_w(offs_t offset, uint8_t data);
	void i8255_b_port_b_w(offs_t offset, uint8_t data);
	void i8255_b_port_c_w(offs_t offset, uint8_t data);

	required_device<i8255_device> m_i8255_a;
	required_device<i8255_device> m_i8255_b;

	devcb_read8        m_in_a_pa_cb;
	devcb_read8        m_in_a_pb_cb;
	devcb_read8        m_in_a_pc_cb;

	devcb_write8       m_out_a_pa_cb;
	devcb_write8       m_out_a_pb_cb;
	devcb_write8       m_out_a_pc_cb;

	devcb_read8        m_in_b_pa_cb;
	devcb_read8        m_in_b_pb_cb;
	devcb_read8        m_in_b_pc_cb;

	devcb_write8       m_out_b_pa_cb;
	devcb_write8       m_out_b_pb_cb;
	devcb_write8       m_out_b_pc_cb;
};

#endif // MAME_MACHINE_MB89363B_H
