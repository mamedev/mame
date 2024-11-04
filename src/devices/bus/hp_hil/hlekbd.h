// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_DEVICES_HP_HIL_HLEKBD_H
#define MAME_DEVICES_HP_HIL_HLEKBD_H

#pragma once

#include "hp_hil.h"
#include "hlebase.h"
#include "machine/keyboard.h"


namespace bus::hp_hil {

class hle_hp_ipc_device
		: public hle_device_base
		, protected device_matrix_keyboard_interface<15U>
{
public:
	hle_hp_ipc_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual int hil_poll() override;
	virtual void hil_idd() override;
	virtual void hil_typematic(uint8_t command) override;

	required_ioport m_modifiers;

private:
	virtual void will_scan_row(u8 row) override;

	util::fifo<uint8_t, 8> m_fifo;
	void transmit_byte(uint8_t byte);
	attotime typematic_delay() const;
	attotime typematic_period() const;

	u16 m_last_modifiers;
	bool m_typematic;
	int m_typematic_rate;
};

class hle_hp_itf_device
		: public hle_device_base
		, protected device_matrix_keyboard_interface<15U>
{
public:
	hle_hp_itf_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual int hil_poll() override;
	virtual void hil_idd() override;
private:
	util::fifo<uint8_t, 8> m_fifo;
	void transmit_byte(uint8_t byte);
};

} // namespace bus::hp_hil


DECLARE_DEVICE_TYPE_NS(HP_IPC_HLE_KEYBOARD, bus::hp_hil, hle_hp_ipc_device);
DECLARE_DEVICE_TYPE_NS(HP_ITF_HLE_KEYBOARD, bus::hp_hil, hle_hp_itf_device);

#endif // MAME_DEVICES_HP_HIL_HLEKBD_H
