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
	m24_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_data_handler(device_t &device, _Object object) { return downcast<m24_keyboard_device &>(device).m_out_data.set_callback(object); }

	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	void device_start();
	void device_reset();
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	DECLARE_WRITE8_MEMBER(bus_w);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_READ8_MEMBER(t0_r);
	DECLARE_READ8_MEMBER(t1_r);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(data_w);
private:
	required_ioport_array<16> m_rows;
	required_ioport m_mousebtn;
	UINT8 m_p1;
	bool m_keypress, m_kbcdata;
	devcb_write_line m_out_data;
	required_device<cpu_device> m_mcu;
	emu_timer *m_reset_timer;
};

extern const device_type M24_KEYBOARD;

#endif /* M24KBD_H_ */
