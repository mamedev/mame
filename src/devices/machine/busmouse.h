// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Logitech bus mouse interface emulation

**********************************************************************/

#ifndef MAME_MACHINE_BUSMOUSE_H
#define MAME_MACHINE_BUSMOUSE_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BUS_MOUSE_EXTINT_CALLBACK(_write) \
	devcb = &bus_mouse_device::set_extint_wr_callback(*device, DEVCB_##_write);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bus_mouse_device

class bus_mouse_device : public device_t
{
public:
	// construction/destruction
	bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_extint_wr_callback(device_t &device, Object &&cb) { return downcast<bus_mouse_device &>(device).m_write_extint.set_callback(std::forward<Object>(cb)); }

	DECLARE_INPUT_CHANGED_MEMBER(mouse_x_changed);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_y_changed);

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_READ8_MEMBER(ppi_a_r);
	DECLARE_READ8_MEMBER(ppi_c_r);
	DECLARE_WRITE8_MEMBER(ppi_c_w);

private:
	emu_timer *m_irq_timer;
	bool irq, irq_disabled;
	int irq_line;

	devcb_write_line m_write_extint;

	required_ioport m_buttons;

	uint8_t m_pa;
	int8_t m_x, m_y;
};


// device type definition
DECLARE_DEVICE_TYPE(BUS_MOUSE, bus_mouse_device)


#endif // MAME_MACHINE_BUSMOUSE_H
