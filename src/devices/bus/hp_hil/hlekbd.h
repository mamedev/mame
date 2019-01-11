// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_DEVICES_HP_HIL_HLEKBD_H
#define MAME_DEVICES_HP_HIL_HLEKBD_H

#pragma once

#include "hp_hil.h"
#include "machine/keyboard.h"


namespace bus { namespace hp_hil {

class hle_device_base
	: public device_t
	, public device_hp_hil_interface
	, protected device_matrix_keyboard_interface<15U>
{
public:
	virtual ~hle_device_base() override;

	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

protected:
	// constructor/destructor
	hle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

	// device_hp_hil_interface overrides
	virtual bool hil_write(uint16_t *data) override;

private:
	void transmit_byte(uint8_t byte);

	util::fifo<uint8_t, 8> m_fifo;
};


class hle_hp_ipc_device : public hle_device_base
{
public:
	hle_hp_ipc_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};

class hle_hp_itf_device : public hle_device_base
{
public:
	hle_hp_itf_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};


} } // namespace bus::hp_hil


DECLARE_DEVICE_TYPE_NS(HP_IPC_HLE_KEYBOARD, bus::hp_hil, hle_hp_ipc_device);
DECLARE_DEVICE_TYPE_NS(HP_ITF_HLE_KEYBOARD, bus::hp_hil, hle_hp_itf_device);

#endif // MAME_DEVICES_HP_HIL_HLEKBD_H
