// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_MSX_AUDIO_KB_H
#define MAME_BUS_MSX_CART_MSX_AUDIO_KB_H

#pragma once

DECLARE_DEVICE_TYPE(MSX_AUDIO_KBDC_PORT, msx_audio_kbdc_port_device)

class msx_audio_kb_port_interface : public device_slot_card_interface
{
public:

	virtual DECLARE_READ8_MEMBER(read) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write) { }

protected:
	// construction/destruction
	using device_slot_card_interface::device_slot_card_interface;
};


class msx_audio_kbdc_port_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	msx_audio_kbdc_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: msx_audio_kbdc_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	msx_audio_kbdc_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// Physical connection simply consists of 8 input and 8 output lines split across 2 connectors
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);


protected:
	msx_audio_kb_port_interface *m_keyboard;
};


void msx_audio_keyboards(device_slot_interface &device);


#endif // MAME_BUS_MSX_CART_MSX_AUDIO_KB_H
