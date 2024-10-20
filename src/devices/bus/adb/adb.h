// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// ADB - Apple Desktop Bus
//
// The serial desktop device bus from before USB was cool.
//
// Single data wire + poweron line, open collector

#ifndef MAME_BUS_ADB_ADB_H
#define MAME_BUS_ADB_ADB_H

#pragma once

class adb_device;
class adb_slot_card_interface;

class adb_connector: public device_t, public device_single_card_slot_interface<adb_slot_card_interface>
{
public:
	template <typename T>
	adb_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false)
		: adb_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}

	adb_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~adb_connector() = default;

	adb_device *get_device();

protected:
	virtual void device_start() override ATTR_COLD;
};

class adb_slot_card_interface : public device_interface
{
	friend class adb_connector;

public:
	adb_slot_card_interface(const machine_config &mconfig, device_t &device, const char *adb_tag);

private:
	required_device<adb_device> m_adb;
};

class adb_device: public device_t
{
public:
	virtual void adb_w(int state) = 0;
	auto adb_r() { return m_adb_cb.bind(); }
	auto poweron_r() { return m_poweron_cb.bind(); }

	static void default_devices(device_slot_interface &device);

protected:
	adb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line m_adb_cb;

private:
	devcb_write_line m_poweron_cb;
	bool m_adb_istate, m_adb_ostate;
};

DECLARE_DEVICE_TYPE(ADB_CONNECTOR, adb_connector)

#endif
