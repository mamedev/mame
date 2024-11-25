// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_SIEMENS_PCD_KBD_H
#define MAME_SIEMENS_PCD_KBD_H

#pragma once


class pcd_keyboard_device : public device_t
{
public:
	pcd_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_tx_handler() { return m_out_tx_handler.bind(); }

	void t0_w(int state);

	void pcd_keyboard_map(address_map &map) ATTR_COLD;
protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void device_start() override ATTR_COLD;

private:
	required_ioport_array<17> m_rows;
	uint8_t m_p1;
	bool m_t0;
	devcb_write_line m_out_tx_handler;

	uint8_t bus_r();
	uint8_t p1_r();
	void p1_w(uint8_t data);
	int t0_r();
};

DECLARE_DEVICE_TYPE(PCD_KEYBOARD, pcd_keyboard_device)

#endif // MAME_SIEMENS_PCD_KBD_H
