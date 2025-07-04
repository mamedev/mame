// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_BUS_SPECTRUM_AY_SLOT_H
#define MAME_BUS_SPECTRUM_AY_SLOT_H

#pragma once

#include "sound/ay8910.h"


class device_ay_slot_interface;

class ay_slot_device : public device_t
	, public device_single_card_slot_interface<device_ay_slot_interface>
	, public device_mixer_interface
{
	friend class device_ay_slot_interface;

public:
	template <typename T>
	ay_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: ay_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	ay_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~ay_slot_device();

	u8 data_r();
	void data_w(u8 data);
	void address_w(u8 data);

protected:
	ay_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

private:
	device_ay_slot_interface *m_dev;
};

class device_ay_slot_interface : public device_interface
{
	friend class ay_slot_device;

public:
	virtual ~device_ay_slot_interface();

	virtual u8 data_r() { return 0xff; };
	virtual void data_w(u8 data) {};
	virtual void address_w(u8 data) {};

protected:
	device_ay_slot_interface(const machine_config &mconfig, device_t &device);

	ay_slot_device *m_slot;
};

class spectrum_ay_device : public device_t, public device_ay_slot_interface
{
public:
	virtual u8 data_r() override;
	virtual void data_w(u8 data) override;
	virtual void address_w(u8 data) override;

protected:
	spectrum_ay_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<ay8910_device> m_ay0;
	optional_device<ay8910_device> m_ay1;

	u8 m_ay_selected;
};

DECLARE_DEVICE_TYPE(AY_SLOT, ay_slot_device)

void default_ay_slot_devices(device_slot_interface &device);

#endif // MAME_BUS_SPECTRUM_AY_SLOT_H
