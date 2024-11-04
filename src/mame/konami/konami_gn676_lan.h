// license:BSD-3-Clause
// copyright-holders:Ville Linde, windyfairy

#ifndef MAME_KONAMI_GN676_LAN_H
#define MAME_KONAMI_GN676_LAN_H

#pragma once

#include "machine/x76f041.h"

class konami_gn676_lan_device : public device_t
{
public:
	static constexpr feature_type unemulated_features() { return feature::LAN; }

	uint8_t lanc1_r(offs_t offset);
	void lanc1_w(offs_t offset, uint8_t data);
	uint8_t lanc2_r(offs_t offset);
	void lanc2_w(offs_t offset, uint8_t data);

	void reset_fpga_state(bool state);

protected:
	konami_gn676_lan_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	optional_device<x76f041_device> m_x76f041;

private:
	std::unique_ptr<uint8_t[]> m_lanc2_ram;

	uint8_t m_lanc2_reg[0x20];

	bool m_fpga_uploaded;
	bool m_fpga_waiting_firmware;
	bool m_fpga_receiving;
	bool m_fpga_is_stubbed;
	bool m_fpga_reset_state;

	uint32_t m_fpga_firmware_size;
	uint32_t m_fpga_firmware_crc;

	uint32_t m_lanc2_ram_r;
	uint32_t m_lanc2_ram_w;

	uint16_t m_network_buffer_max_size;
	uint8_t m_network_id;

	bool m_x76f041_enabled;
	bool m_x76f041_read_enabled;
	bool m_x76f041_rst_triggered;
};

class konami_gn676a_lan_device : public konami_gn676_lan_device
{
public:
	konami_gn676a_lan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class konami_gn676b_lan_device : public konami_gn676_lan_device
{
public:
	konami_gn676b_lan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(KONAMI_GN676A_LAN, konami_gn676a_lan_device)
DECLARE_DEVICE_TYPE(KONAMI_GN676B_LAN, konami_gn676b_lan_device)

#endif // MAME_KONAMI_GN676_LAN_H
