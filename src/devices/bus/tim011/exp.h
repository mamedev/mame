// license:BSD-3-Clause
// copyright-holders:Marko Solajic, Miodrag Milanovic
/*********************************************************************

        TIM-011 Expansion Port emulation

**********************************************************************

    Pinout:

        TIM011 Expansion Connector
        40 pin male, on main board

                    *******
        VCC     1   * . . *    2 VCC
        GND     3   * . . *    4 GND
        /RD     5   * . . *    6 PHI
        /WR     7   * . . *    8 /RESET
        E       9   * . . *   10 /LIR
        /NMI   11   * . . *   12 /EXPSEL (E0-FF)
        /WAIT  13   * . . *   14 NC
        /INT0  15   * . . *   16 /HALT
        ST     17     . . *   18 NC
        A0     19     . . *   20 A1
        /TEND0 21     . . *   22 A2
        A3     23   * . . *   24 A4
        /DREQ0 25   * . . *   26 /IOE
        8MHZ   27   * . . *   28 RESET
        D7     29   * . . *   30 D6
        D5     31   * . . *   32 D3
        D4     33   * . . *   34 D2
        D1     35   * . . *   36 D0
        GND    37   * . . *   38 GND
        NC     39   * . . *   40 NC
                    *******

**********************************************************************/

#ifndef MAME_BUS_TIM011_EXP_H
#define MAME_BUS_TIM011_EXP_H

#pragma once

namespace bus::tim011 {

class device_exp_interface;

class exp_slot_device : public device_t, public device_single_card_slot_interface<device_exp_interface>
{
public:
	// construction/destruction
	template <typename T>
	exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: exp_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	// called from expansion device
	void int_w(int state) { m_int_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

	required_address_space m_io;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;
};

// ======================> device_exp_interface

class device_exp_interface : public device_interface
{
protected:
	// construction/destruction
	device_exp_interface(const machine_config &mconfig, device_t &device);

	exp_slot_device *m_slot;
};

} // namespace bus::tim011

DECLARE_DEVICE_TYPE_NS(TIM011_EXPANSION_SLOT, bus::tim011, exp_slot_device)

void tim011_exp_devices(device_slot_interface &device);

#endif // MAME_BUS_TIM011_EXP_H
