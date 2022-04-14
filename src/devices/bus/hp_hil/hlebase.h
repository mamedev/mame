// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_DEVICES_HP_HIL_HLEBASE_H
#define MAME_DEVICES_HP_HIL_HLEBASE_H

#pragma once

#include "hp_hil.h"
#include "machine/keyboard.h"


namespace bus::hp_hil {

class hle_device_base
	: public device_t
	, public device_hp_hil_interface
{
public:
	virtual ~hle_device_base() override;
	void transmit_byte(uint8_t byte);
protected:
	// constructor/destructor
	hle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_hp_hil_interface overrides
	virtual bool hil_write(uint16_t *data) override;
	virtual void hil_idd() = 0;
	virtual void hil_typematic(uint8_t command) {}
	virtual int hil_poll() = 0;
private:

	util::fifo<uint8_t, 8> m_fifo;

	bool m_powerup;
	bool m_passthru;
};

} // namespace bus::hp_hil

#endif // MAME_DEVICES_HP_HIL_HLEBASE_H
