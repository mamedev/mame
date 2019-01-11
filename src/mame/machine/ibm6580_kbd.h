// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_MACHINE_IBM6580_KBD_H
#define MAME_MACHINE_IBM6580_KBD_H

#pragma once


#define MCFG_DW_KEYBOARD_OUT_DATA_HANDLER(_devcb) \
	devcb = &downcast<dw_keyboard_device &>(*device).set_out_data_handler(DEVCB_##_devcb);

#define MCFG_DW_KEYBOARD_OUT_CLOCK_HANDLER(_devcb) \
	devcb = &downcast<dw_keyboard_device &>(*device).set_out_clock_handler(DEVCB_##_devcb);

#define MCFG_DW_KEYBOARD_OUT_STROBE_HANDLER(_devcb) \
	devcb = &downcast<dw_keyboard_device &>(*device).set_out_strobe_handler(DEVCB_##_devcb);

class dw_keyboard_device :  public device_t
{
public:
	dw_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_out_data_handler(Object &&cb) { return m_out_data.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_clock_handler(Object &&cb) { return m_out_clock.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_strobe_handler(Object &&cb) { return m_out_strobe.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(reset_w);
	DECLARE_WRITE_LINE_MEMBER(ack_w);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	void device_start() override;
	void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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

	DECLARE_WRITE8_MEMBER(bus_w);
	DECLARE_READ8_MEMBER(bus_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_READ_LINE_MEMBER(t0_r);
	DECLARE_READ_LINE_MEMBER(t1_r);
};

DECLARE_DEVICE_TYPE(DW_KEYBOARD, dw_keyboard_device)

#endif // MAME_MACHINE_IBM6580_KBD_H
