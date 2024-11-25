// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NAMCO_NAMCO65_H
#define MAME_NAMCO_NAMCO65_H

#pragma once

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
	void mcu_map(address_map &map) ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<hd63705z0_device> m_mcu;

	devcb_read8        m_in_pb_cb;
	devcb_read8        m_in_pc_cb;
	devcb_read8        m_in_ph_cb;
	devcb_read8        m_in_pdsw_cb;

	devcb_read8::array<8> m_port_analog_in_cb;
	devcb_read8::array<4> m_port_dial_in_cb;

	devcb_read8        m_dp_in;
	devcb_write8       m_dp_out;

	uint8_t namcos2_mcu_port_d_r();
	void namcos2_mcu_port_d_w(uint8_t data);

	uint8_t namcos2_mcu_analog_port_r();
	void namcos2_mcu_analog_port_w(uint8_t data);

	uint8_t namcos2_mcu_analog_ctrl_r();
	void namcos2_mcu_analog_ctrl_w(uint8_t data);

	uint8_t dpram_byte_r(offs_t offset);
	void dpram_byte_w(offs_t offset, uint8_t data);

	uint8_t mcub_r() { return m_in_pb_cb(); }
	uint8_t mcuc_r() { return m_in_pc_cb(); }
	uint8_t mcuh_r() { return m_in_ph_cb(); }
	uint8_t mcudsw_r() { return m_in_pdsw_cb(); }

	uint8_t mcudi0_r() { return m_port_dial_in_cb[0](); }
	uint8_t mcudi1_r() { return m_port_dial_in_cb[1](); }
	uint8_t mcudi2_r() { return m_port_dial_in_cb[2](); }
	uint8_t mcudi3_r() { return m_port_dial_in_cb[3](); }

	int m_mcu_analog_ctrl;
	int m_mcu_analog_data;
	int m_mcu_analog_complete;
};

#endif // MAME_NAMCO_NAMCO65_H
