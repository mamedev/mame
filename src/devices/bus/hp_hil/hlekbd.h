// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_DEVICES_HP_HIL_HLEKBD_H
#define MAME_DEVICES_HP_HIL_HLEKBD_H

#pragma once

#include "hp_hil.h"
#include "machine/keyboard.h"


extern device_type const HP_IPC_HLE_KEYBOARD;


namespace bus { namespace hp_hil {
class hle_device_base
	: public device_t
	, public device_hp_hil_interface
	, protected device_matrix_keyboard_interface<15U>
{
public:

protected:
	// constructor/destructor
	hle_device_base(machine_config const &mconfig, device_type type, char const *name, char const *tag, device_t *owner, uint32_t clock, char const *shortname, char const *source);
	virtual ~hle_device_base() override;

	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

	// device_hp_hil_interface overrides
	virtual void hil_write(uint16_t data) override;

private:
	// device_serial_interface uses 10'000 range
	// device_matrix_keyboard_interface uses 20'000 range

	void transmit_byte(uint8_t byte);

	util::fifo<uint8_t, 8> m_fifo;
};


class hle_hp_ipc_device : public hle_device_base
{
public:
	hle_hp_ipc_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};

} } // namespace bus::hp_hil

#endif // MAME_DEVICES_HP_HIL_HLEKBD_H
