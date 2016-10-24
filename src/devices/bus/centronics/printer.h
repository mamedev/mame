// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __CENTRONICS_PRINTER_H__
#define __CENTRONICS_PRINTER_H__

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

	void printer_online(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:

	enum
	{
		TIMER_ACK,
		TIMER_BUSY
	};

	int m_strobe;
	uint8_t m_data;
	int m_busy;

	required_device<printer_image_device> m_printer;
};

// device type definition
extern const device_type CENTRONICS_PRINTER;

#endif
