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
	centronics_printer_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_strobe ) override;
	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { if (state) m_data |= 0x01; else m_data &= ~0x01; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { if (state) m_data |= 0x02; else m_data &= ~0x02; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { if (state) m_data |= 0x04; else m_data &= ~0x04; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { if (state) m_data |= 0x08; else m_data &= ~0x08; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { if (state) m_data |= 0x10; else m_data &= ~0x10; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { if (state) m_data |= 0x20; else m_data &= ~0x20; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { if (state) m_data |= 0x40; else m_data &= ~0x40; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) override { if (state) m_data |= 0x80; else m_data &= ~0x80; }
	virtual DECLARE_WRITE_LINE_MEMBER( input_init ) override;

	DECLARE_WRITE_LINE_MEMBER( printer_online );

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
	UINT8 m_data;
	int m_busy;

	required_device<printer_image_device> m_printer;
};

// device type definition
extern const device_type CENTRONICS_PRINTER;

#endif
