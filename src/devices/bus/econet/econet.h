// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn Computers Econet local area network emulation

**********************************************************************/

#ifndef MAME_BUS_ECONET_ECONET_H
#define MAME_BUS_ECONET_ECONET_H

#pragma once

#include <vector>



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> econet_device

class device_econet_interface;

class econet_device : public device_t
{
public:
	// construction/destruction
	econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto clk_wr_callback() { return m_write_clk.bind(); }
	auto data_wr_callback() { return m_write_data.bind(); }

	void add_device(device_econet_interface &target, int address);

	// writes for host (driver_device)
	void host_clk_w(int state);
	void host_data_w(int state);

	// writes for peripherals (device_t)
	void clk_w(device_t *device, int state);
	void data_w(device_t *device, int state);

protected:
	enum
	{
		CLK = 0,
		DATA,
		SIGNAL_COUNT
	};

	class daisy_entry
	{
	public:
		daisy_entry(device_econet_interface &device);

		device_t &device() { return *m_device; }
		device_econet_interface &interface() { return *m_interface; }

		int m_line[SIGNAL_COUNT];

	private:
		device_t *                  m_device;       // associated device
		device_econet_interface *   m_interface;    // associated device's daisy interface
	};

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	std::vector<daisy_entry> m_device_list;

private:
	devcb_write_line   m_write_clk;
	devcb_write_line   m_write_data;

	void set_signal(device_t *device, int signal, int state);
	int get_signal(int signal);

	int m_line[SIGNAL_COUNT];
};


// ======================> econet_slot_device

class econet_slot_device : public device_t, public device_single_card_slot_interface<device_econet_interface>
{
public:
	// construction/destruction
	econet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T, typename U>
	econet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&econet_tag, U &&devs)
		: econet_slot_device(mconfig, tag, owner, 0U)
	{
		set_econet_tag(std::forward<T>(econet_tag));
		devs(*this);
	}

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// inline configuration
	template <typename T> void set_econet_tag(T &&tag) { m_econet.set_tag(std::forward<T>(tag)); }
	void set_slot(int address) { m_address = address; }

private:
	// configuration
	uint8_t m_address;
	required_device<econet_device> m_econet;
};


// ======================> device_econet_interface

class device_econet_interface : public device_interface
{
	friend class econet_device;

public:
	virtual void econet_clk(int state) = 0;
	virtual void econet_data(int state) = 0;

protected:
	// construction/destruction
	device_econet_interface(const machine_config &mconfig, device_t &device);

	econet_device  *m_econet;
	uint8_t m_address;
};


// device type definition
DECLARE_DEVICE_TYPE(ECONET,      econet_device)
DECLARE_DEVICE_TYPE(ECONET_SLOT, econet_slot_device)

void econet_devices(device_slot_interface &device);

#endif // MAME_BUS_ECONET_ECONET_H
