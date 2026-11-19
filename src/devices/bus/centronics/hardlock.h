// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_CENTRONICS_HARDLOCK_H
#define MAME_BUS_CENTRONICS_HARDLOCK_H

#pragma once

#include "ctronics.h"
#include "machine/eepromser.h"

#include <array>

class hardlock_device : public device_t, public device_centronics_peripheral_interface
{
public:
	hardlock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_keys(u16 module, u16 seed1, u16 seed2, u16 seed3) ATTR_COLD;
	void set_eeprom_data(const std::array<u16, 64> &data) { m_default_eeprom = data; }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_centronics_peripheral_interface implementation
	virtual void input_data0(int state) override;
	virtual void input_data1(int state) override;
	virtual void input_data2(int state) override;
	virtual void input_data3(int state) override;
	virtual void input_data4(int state) override;
	virtual void input_data5(int state) override;
	virtual void input_data6(int state) override;
	virtual void input_data7(int state) override;

private:
	void data_w(u8 bit, int state);
	TIMER_CALLBACK_MEMBER(update_data);
	void process_data(u8 data);
	bool select_module(u8 data);
	void reset_cipher();
	void clock_cipher(u8 data);
	void update_output();
	void eeprom_do_w(int state);

	required_device<eeprom_serial_93cxx_device> m_eeprom;
	std::array<u16, 64> m_default_eeprom;
	u16 m_module;
	u16 m_seed1;
	u16 m_seed2;
	u16 m_seed3;

	u8 m_data;
	u8 m_previous;
	bool m_pending;
	u32 m_token;
	s8 m_sync_clocks;
	u8 m_address_count;
	std::array<u8, 9> m_address;
	bool m_selected;
	bool m_eeprom_mode;
	bool m_eeprom_do;
	u8 m_index;
	u8 m_history;
	u16 m_shift;
};

DECLARE_DEVICE_TYPE(HARDLOCK, hardlock_device)

#endif // MAME_BUS_CENTRONICS_HARDLOCK_H
