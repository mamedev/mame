// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef PCD_KBD_H_
#define PCD_KBD_H_

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/speaker.h"

#define MCFG_PCD_KEYBOARD_OUT_TX_HANDLER(_devcb) \
	devcb = &pcd_keyboard_device::set_out_tx_handler(*device, DEVCB_##_devcb);

class pcd_keyboard_device :  public device_t
{
public:
	pcd_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_out_tx_handler(device_t &device, _Object object) { return downcast<pcd_keyboard_device &>(device).m_out_tx_handler.set_callback(object); }

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void device_start() override;

	uint8_t bus_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t t0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void t0_w(int state);
private:
	required_ioport_array<17> m_rows;
	uint8_t m_p1;
	bool m_t0;
	devcb_write_line m_out_tx_handler;
};

extern const device_type PCD_KEYBOARD;

#endif /* PCD_KBD_H_ */
