// license:BSD-3-Clause
// copyright-holders:QUFB

#ifndef MAME_BUS_PICO_DORAEMON_H
#define MAME_BUS_PICO_DORAEMON_H

#pragma once

#include "pico_ps2.h"

class pico_doraemon_device : public device_t, public device_pico_ps2_slot_interface
{
public:
	pico_doraemon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	// device_pico_ps2_slot_interface overrides
	virtual uint8_t ps2_r(offs_t offset) override;
	virtual void ps2_w(offs_t offset, uint8_t data) override;

private:
	// TODO: Several commands are unemulated, which likely control
	// the "Bamboo Copter" and sounds played. It's currently unknown
	// how these affect gameplay.
	enum commands : uint8_t
	{
		UNK_00 = 0x00,
		UNK_04 = 0x04,
		COMMAND_START = 0x57,
		UNK_70 = 0x70,
		BUTTON_STATUS = 0x71,
		UNK_80 = 0x80,
		UNK_C0 = 0xC0,
	};

	required_ioport m_io_buttons;

	uint8_t m_i;
	uint8_t m_data;
	uint8_t m_prev_data;
	bool m_is_write_done;
};

DECLARE_DEVICE_TYPE(PICO_DORAEMON, pico_doraemon_device)

#endif // MAME_BUS_PICO_DORAEMON_H;
