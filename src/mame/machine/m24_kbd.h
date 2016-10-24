// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef M24KBD_H_
#define M24KBD_H_

#include "emu.h"
#include "cpu/mcs48/mcs48.h"

#define MCFG_M24_KEYBOARD_OUT_DATA_HANDLER(_devcb) \
	devcb = &m24_keyboard_device::set_out_data_handler(*device, DEVCB_##_devcb);

class m24_keyboard_device :  public device_t
{
public:
	m24_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_out_data_handler(device_t &device, _Object object) { return downcast<m24_keyboard_device &>(device).m_out_data.set_callback(object); }

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void device_start() override;
	void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void bus_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t t0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void clock_w(int state);
	void data_w(int state);
private:
	required_ioport_array<16> m_rows;
	required_ioport m_mousebtn;
	uint8_t m_p1;
	bool m_keypress, m_kbcdata;
	devcb_write_line m_out_data;
	required_device<cpu_device> m_mcu;
	emu_timer *m_reset_timer;
};

extern const device_type M24_KEYBOARD;

#endif /* M24KBD_H_ */
