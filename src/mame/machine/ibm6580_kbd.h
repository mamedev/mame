// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef DWKBD_H_
#define DWKBD_H_

#include "cpu/mcs48/mcs48.h"

#define MCFG_DW_KEYBOARD_OUT_DATA_HANDLER(_devcb) \
	devcb = &dw_keyboard_device::set_out_data_handler(*device, DEVCB_##_devcb);

#define MCFG_DW_KEYBOARD_OUT_CLOCK_HANDLER(_devcb) \
	devcb = &dw_keyboard_device::set_out_clock_handler(*device, DEVCB_##_devcb);

#define MCFG_DW_KEYBOARD_OUT_STROBE_HANDLER(_devcb) \
	devcb = &dw_keyboard_device::set_out_strobe_handler(*device, DEVCB_##_devcb);

class dw_keyboard_device :  public device_t
{
public:
	dw_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_out_data_handler(device_t &device, _Object object) { return downcast<dw_keyboard_device &>(device).m_out_data.set_callback(object); }
	template<class _Object> static devcb_base &set_out_clock_handler(device_t &device, _Object object) { return downcast<dw_keyboard_device &>(device).m_out_clock.set_callback(object); }
	template<class _Object> static devcb_base &set_out_strobe_handler(device_t &device, _Object object) { return downcast<dw_keyboard_device &>(device).m_out_strobe.set_callback(object); }

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void device_start() override;
	void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_WRITE8_MEMBER(bus_w);
	DECLARE_READ8_MEMBER(bus_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_READ8_MEMBER(t0_r);
	DECLARE_READ8_MEMBER(t1_r);
	DECLARE_WRITE_LINE_MEMBER(reset_w);
	DECLARE_WRITE_LINE_MEMBER(ack_w);

private:
	uint8_t m_dip, m_bus, m_p2;
	int m_drive, m_sense;
	bool m_keylatch, m_ack;
	emu_timer *m_reset_timer;

	required_ioport_array<12> m_kbd;
	devcb_write_line m_out_data;
	devcb_write_line m_out_clock;
	devcb_write_line m_out_strobe;
	required_device<cpu_device> m_mcu;
};

extern const device_type DW_KEYBOARD;

#endif /* DWKBD_H_ */
