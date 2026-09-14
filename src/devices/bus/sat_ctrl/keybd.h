// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Fabio Priuli
/**********************************************************************

    Sega Saturn Keyboard emulation

**********************************************************************/

#ifndef MAME_BUS_SAT_CTRL_KEYBD_H
#define MAME_BUS_SAT_CTRL_KEYBD_H

#pragma once

#include "ctrl.h"
#include "machine/keyboard.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saturn_keybd_device

class saturn_keybd_device : public device_t,
							public device_saturn_control_port_interface,
							protected device_matrix_keyboard_interface<18U>
{
public:
	// construction/destruction
	saturn_keybd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_saturn_control_port_interface overrides
	virtual uint8_t read_ctrl(uint8_t offset) override;
	virtual uint8_t read_status() override { return 0xf1; }
	virtual uint8_t read_id(int idx) override { return m_ctrl_id; }

	// device_matrix_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;

private:
	uint8_t m_status;
	uint8_t m_data;

	uint16_t get_game_key();
};

// device type definition
DECLARE_DEVICE_TYPE(SATURN_KEYBD, saturn_keybd_device)

#endif // MAME_BUS_SAT_CTRL_KEYBD_H
