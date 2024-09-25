// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_MSX_AUDIO_KB_H
#define MAME_BUS_MSX_CART_MSX_AUDIO_KB_H

#pragma once

DECLARE_DEVICE_TYPE(MSX_AUDIO_KBDC_PORT, msx_audio_kbdc_port_device)

class msx_audio_kb_port_interface : public device_interface
{
public:
	virtual uint8_t read() { return 0xff; }
	virtual void write(uint8_t data) { }

protected:
	// construction/destruction
	msx_audio_kb_port_interface(machine_config const &mconfig, device_t &device);
};


class msx_audio_kbdc_port_device : public device_t, public device_single_card_slot_interface<msx_audio_kb_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	msx_audio_kbdc_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: msx_audio_kbdc_port_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	msx_audio_kbdc_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// Physical connection simply consists of 8 input and 8 output lines split across 2 connectors
	void write(u8 data);
	u8 read();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	msx_audio_kb_port_interface *m_keyboard;
};


void msx_audio_keyboards(device_slot_interface &device);

#endif // MAME_BUS_MSX_CART_MSX_AUDIO_KB_H
