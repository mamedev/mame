// license:BSD-3-Clause
// copyright-holders:Curt Coder

#ifndef MAME_COMMODORE_CLIPPER_PRN_H
#define MAME_COMMODORE_CLIPPER_PRN_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/i8255.h"

class clipper_prn_device : public device_t
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	clipper_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto ack_handler() { return m_ack_cb.bind(); }

	void data_w(uint8_t data) { m_data = data; }
	void strobe_w(int state) { m_ppi->pc4_w(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t p2_r();
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	uint8_t data_r();
	void pc_w(uint8_t data);

	required_device<i8039_device> m_mcu;
	required_device<i8255_device> m_ppi;
	devcb_write_line m_ack_cb;
	memory_share_creator<uint8_t> m_ram;
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_data;
	uint8_t m_pc;
};

DECLARE_DEVICE_TYPE(CLIPPER_PRN, clipper_prn_device)

#endif // MAME_COMMODORE_CLIPPER_PRN_H
