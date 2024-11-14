// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Logitech bus mouse interface emulation

**********************************************************************/

#ifndef MAME_MACHINE_BUSMOUSE_H
#define MAME_MACHINE_BUSMOUSE_H

#pragma once

class bus_mouse_device : public device_t
{
public:
	// construction/destruction
	bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto extint_callback() { return m_write_extint.bind(); }

	DECLARE_INPUT_CHANGED_MEMBER(mouse_x_changed);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_y_changed);

	void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t ppi_a_r();
	uint8_t ppi_c_r();
	void ppi_c_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	emu_timer *m_irq_timer;
	bool irq, irq_disabled;
	int irq_line;

	devcb_write_line m_write_extint;

	required_ioport m_buttons;

	uint8_t m_pa;
	int8_t m_x, m_y;
};

DECLARE_DEVICE_TYPE(BUS_MOUSE, bus_mouse_device)

#endif // MAME_MACHINE_BUSMOUSE_H
