// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_CENTRONICS_LUAPRINTER_H
#define MAME_BUS_CENTRONICS_LUAPRINTER_H

#pragma once

#include "ctronics.h"
#include "imagedev/printer.h"
#include "luaprinter.h"

// ====================== centronics_luaprinter_device

class centronics_luaprinter_device : public device_t,
	public device_centronics_peripheral_interface, public device_luaprinter_interface
{
public:
	// construction/destruction
	centronics_luaprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	DECLARE_WRITE_LINE_MEMBER( printer_online );

	enum
	{
		TIMER_ACK,
		TIMER_BUSY
	};

	emu_timer *m_ack_timer;
	emu_timer *m_busy_timer;

	int m_strobe;
	uint8_t m_data;
	int m_busy;

	required_device<printer_image_device> m_printer;
	required_device<screen_device> m_screen;
	bitmap_rgb32 m_bitmap;

	const int xdpi=160;
	const int ydpi=72;
	const int PAPER_WIDTH = 8.5 * xdpi;  // 8.5 inches wide
	const int PAPER_HEIGHT = 11 * ydpi;  // 11  inches high
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
};

// device type definition
DECLARE_DEVICE_TYPE(CENTRONICS_LUAPRINTER, centronics_luaprinter_device)

#endif // MAME_BUS_CENTRONICS_LUAPRINTER_H
