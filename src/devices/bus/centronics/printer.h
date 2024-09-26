// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_CENTRONICS_PRINTER_H
#define MAME_BUS_CENTRONICS_PRINTER_H

#pragma once

#include "ctronics.h"
#include "imagedev/printer.h"

// ======================> centronics_printer_device

class centronics_printer_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_data |= 0x01; else m_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_data |= 0x02; else m_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_data |= 0x04; else m_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_data |= 0x08; else m_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_data |= 0x10; else m_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_data |= 0x20; else m_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_data |= 0x40; else m_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_data |= 0x80; else m_data &= ~0x80; }
	virtual void input_init(int state) override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

private:
	void printer_online(int state);

	TIMER_CALLBACK_MEMBER(ack_timer_tick);
	TIMER_CALLBACK_MEMBER(busy_timer_tick);

	emu_timer *m_ack_timer;
	emu_timer *m_busy_timer;

	int m_strobe;
	uint8_t m_data;
	int m_busy;

	required_device<printer_image_device> m_printer;
};

// device type definition
DECLARE_DEVICE_TYPE(CENTRONICS_PRINTER, centronics_printer_device)

#endif // MAME_BUS_CENTRONICS_PRINTER_H
