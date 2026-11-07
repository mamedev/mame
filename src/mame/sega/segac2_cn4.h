// license:BSD-3-Clause
// copyright-holders:Damian Rogers
#ifndef MAME_SEGA_SEGAC2_CN4_H
#define MAME_SEGA_SEGAC2_CN4_H

#pragma once


class device_segac2_cn4_interface;


class segac2_cn4_slot_device : public device_t, public device_single_card_slot_interface<device_segac2_cn4_interface>
{
public:
	template <typename T>
	segac2_cn4_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: segac2_cn4_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	segac2_cn4_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_segac2_cn4_interface *m_card;
};


class device_segac2_cn4_interface : public device_interface
{
public:
	virtual u8 read(offs_t offset) = 0;
	virtual void write(offs_t offset, u8 data) = 0;

protected:
	device_segac2_cn4_interface(machine_config const &mconfig, device_t &device);
};


DECLARE_DEVICE_TYPE(SEGAC2_CN4_SLOT, segac2_cn4_slot_device)

#endif // MAME_SEGA_SEGAC2_CN4_H
