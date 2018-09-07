// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_NAMCO65_H
#define MAME_MACHINE_NAMCO65_H

#pragma once

#include "machine/bankdev.h"
#include "cpu/m6805/m6805.h"

DECLARE_DEVICE_TYPE(NAMCOC65, namcoc65_device)


class namcoc65_device : public device_t
{
public:
	// construction/destruction
	namcoc65_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto in_pb_callback() { return m_in_pb_cb.bind(); }
	auto in_pc_callback() { return m_in_pc_cb.bind(); }
	auto in_ph_callback() { return m_in_ph_cb.bind(); }
	auto in_pdsw_callback() { return m_in_pdsw_cb.bind(); }

	auto an0_in_cb() { return m_port_analog_in_cb[0].bind(); }
	auto an1_in_cb() { return m_port_analog_in_cb[1].bind(); }
	auto an2_in_cb() { return m_port_analog_in_cb[2].bind(); }
	auto an3_in_cb() { return m_port_analog_in_cb[3].bind(); }
	auto an4_in_cb() { return m_port_analog_in_cb[4].bind(); }
	auto an5_in_cb() { return m_port_analog_in_cb[5].bind(); }
	auto an6_in_cb() { return m_port_analog_in_cb[6].bind(); }
	auto an7_in_cb() { return m_port_analog_in_cb[7].bind(); }

	auto di0_in_cb() { return m_port_dial_in_cb[0].bind(); }
	auto di1_in_cb() { return m_port_dial_in_cb[1].bind(); }
	auto di2_in_cb() { return m_port_dial_in_cb[2].bind(); }
	auto di3_in_cb() { return m_port_dial_in_cb[3].bind(); }

	auto dp_in_callback() { return m_dp_in.bind(); }
	auto dp_out_callback() { return m_dp_out.bind(); }

	void ext_interrupt(int state) { m_mcu->set_input_line(0, state); }
	void ext_reset(int state) { m_mcu->set_input_line(INPUT_LINE_RESET, state); }

protected:
	void mcu_map(address_map &map);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_reset() override;

private:
	required_device<hd63705_device> m_mcu;

	devcb_read8        m_in_pb_cb;
	devcb_read8        m_in_pc_cb;
	devcb_read8        m_in_ph_cb;
	devcb_read8        m_in_pdsw_cb;

	devcb_read8   m_port_analog_in_cb[8];
	devcb_read8   m_port_dial_in_cb[4];

	devcb_read8        m_dp_in;
	devcb_write8       m_dp_out;

	DECLARE_READ8_MEMBER(namcos2_mcu_port_d_r);
	DECLARE_WRITE8_MEMBER(namcos2_mcu_port_d_w);

	DECLARE_READ8_MEMBER(namcos2_mcu_analog_port_r );
	DECLARE_WRITE8_MEMBER(namcos2_mcu_analog_port_w);

	DECLARE_READ8_MEMBER(namcos2_mcu_analog_ctrl_r);
	DECLARE_WRITE8_MEMBER(namcos2_mcu_analog_ctrl_w);

	DECLARE_READ8_MEMBER(dpram_byte_r);
	DECLARE_WRITE8_MEMBER(dpram_byte_w);

	DECLARE_READ8_MEMBER(mcub_r) { return m_in_pb_cb(); }
	DECLARE_READ8_MEMBER(mcuc_r) { return m_in_pc_cb(); }
	DECLARE_READ8_MEMBER(mcuh_r) { return m_in_ph_cb(); }
	DECLARE_READ8_MEMBER(mcudsw_r) { return m_in_pdsw_cb(); }

	DECLARE_READ8_MEMBER(mcudi0_r) { return m_port_dial_in_cb[0](); }
	DECLARE_READ8_MEMBER(mcudi1_r) { return m_port_dial_in_cb[1](); }
	DECLARE_READ8_MEMBER(mcudi2_r) { return m_port_dial_in_cb[2](); }
	DECLARE_READ8_MEMBER(mcudi3_r) { return m_port_dial_in_cb[3](); }

	int m_mcu_analog_ctrl;
	int m_mcu_analog_data;
	int m_mcu_analog_complete;
};

#endif // MAME_MACHINE_NAMCO65_H
