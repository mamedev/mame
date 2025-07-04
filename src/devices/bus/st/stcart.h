// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Atari ST cartridges

#ifndef MAME_BUS_ST_STCART_H
#define MAME_BUS_ST_STCART_H

#pragma once

class device_stcart_interface;

class stcart_connector: public device_t, public device_single_card_slot_interface<device_stcart_interface>
{
public:
	template <typename T>
	stcart_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: stcart_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	stcart_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_space_installer &space);

protected:
	virtual void device_start() override ATTR_COLD;
};

class device_stcart_interface: public device_interface
{
public:
	device_stcart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_stcart_interface();

	virtual void map(address_space_installer &space) = 0;

protected:

};

DECLARE_DEVICE_TYPE(STCART_CONNECTOR, stcart_connector)

void stcart_intf(device_slot_interface &device);

#endif // MAME_BUS_ST_STCART_H
