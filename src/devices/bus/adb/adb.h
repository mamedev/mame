// license:BSD-3-Clause
// copyright-holders: R. Belmont, Olivier Galibert

// ADB - Apple Desktop Bus
//
// The serial desktop device bus from before USB was cool.
//
// Single data wire + poweron line, open collector

#ifndef MAME_BUS_ADB_ADB_H
#define MAME_BUS_ADB_ADB_H

#pragma once

#include "emu.h"

class adb_device_interface;
class adb_slot_card_interface;

class adb_bus_device : public device_t
{
public:
	static constexpr unsigned MAX_DEVICES = 16;

	adb_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto out_adb_callback() { return m_adb_handler.bind(); }
	auto out_poweron_callback() { return m_poweron_handler.bind(); }

	template <typename T>
	void set_external_device(unsigned id, T &&tag)
	{
		assert(id < MAX_DEVICES);
		m_external_devices[id].set_tag(std::forward<T>(tag));
	}

	int adb_line_r() const { return m_adb_line; }
	int adb_poweron_r() const { return m_poweron_line; }
	int adb_anykeydown_r() const { return m_anykeydown_state; }
	void adb_host_line_w(int state);
	void poll_devices();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

private:
	struct adb_dev_t
	{
		adb_device_interface *m_dev;
		int m_adb_state;
		int m_poweron_state;
		int m_anykeydown_state;
	};

	optional_device_array<adb_device_interface, MAX_DEVICES> m_external_devices;

	devcb_write_line m_adb_handler;
	devcb_write_line m_poweron_handler;

	adb_dev_t m_dev[MAX_DEVICES];
	unsigned m_devcnt;

	int m_adb_line;
	int m_poweron_line;
	int m_anykeydown_state;
	int m_host_drive_state;
	bool m_updating_adb;
	bool m_adb_update_pending;

	void adb_device_line_w(unsigned refid, int state);
	void poweron_device_line_w(unsigned refid, int state);
	void anykeydown_device_w(unsigned refid, int state);
	void update_adb_line();
	void update_poweron_line();
	void update_anykeydown();

	friend class adb_device_interface;
};

class adb_connector : public device_t, public device_single_card_slot_interface<adb_slot_card_interface>
{
public:
	template <typename T>
	adb_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false)
		: adb_connector(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, fixed);
	}

	adb_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~adb_connector() = default;

	adb_device_interface *get_device();

protected:
	virtual void device_start() override ATTR_COLD;
};

class adb_slot_card_interface : public device_interface
{
	friend class adb_connector;

public:
	adb_slot_card_interface(const machine_config &mconfig, device_t &device, const char *adb_tag);

private:
	required_device<adb_device_interface> m_adb;
};

class adb_device_interface : public device_t
{
public:
	virtual void adb_w(int state) = 0;

protected:
	adb_device_interface(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void adb_poll_inputs()
	{
	}

	bool adb_is_connected() const { return m_bus != nullptr; }
	int adb_line_r() const;
	int adb_poweron_r() const;
	// ADB is open collector: CLEAR_LINE drives low and ASSERT_LINE releases the line.
	void adb_drive_w(int state);
	void adb_drive_low() { adb_drive_w(CLEAR_LINE); }
	void adb_release() { adb_drive_w(ASSERT_LINE); }
	void adb_poweron_w(int state);
	void adb_anykeydown_w(int state);

private:
	void connect_to_bus(adb_bus_device &bus, unsigned refid);

	adb_bus_device *m_bus;
	unsigned m_refid;

	friend class adb_bus_device;
};

DECLARE_DEVICE_TYPE(ADB_BUS, adb_bus_device)
DECLARE_DEVICE_TYPE(ADB_CONNECTOR, adb_connector)

#endif
