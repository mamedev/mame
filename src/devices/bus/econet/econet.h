// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn Computers Econet local area network emulation

**********************************************************************/

#pragma once

#ifndef __ECONET__
#define __ECONET__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ECONET_TAG          "econet"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ECONET_ADD() \
	MCFG_DEVICE_ADD(ECONET_TAG, ECONET, 0)

#define MCFG_ECONET_SLOT_ADD(_tag, _num, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ECONET_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	econet_slot_device::static_set_slot(*device, _num);


#define MCFG_ECONET_CLK_CALLBACK(_write) \
	devcb = &econet_device::set_clk_wr_callback(*device, DEVCB_##_write);

#define MCFG_ECONET_DATA_CALLBACK(_write) \
	devcb = &econet_device::set_data_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> econet_device

class device_econet_interface;

class econet_device : public device_t
{
public:
	// construction/destruction
	econet_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_clk_wr_callback(device_t &device, _Object object) { return downcast<econet_device &>(device).m_write_clk.set_callback(object); }
	template<class _Object> static devcb_base &set_data_wr_callback(device_t &device, _Object object) { return downcast<econet_device &>(device).m_write_data.set_callback(object); }

	void add_device(device_t *target, int address);

	// writes for host (driver_device)
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( data_w );

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

	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;

	class daisy_entry
	{
	public:
		daisy_entry(device_t *device);
		daisy_entry *next() const { return m_next; }

		daisy_entry *               m_next;         // next device
		device_t *                  m_device;       // associated device
		device_econet_interface *   m_interface;    // associated device's daisy interface

		int m_line[SIGNAL_COUNT];
	};

	simple_list<daisy_entry> m_device_list;

private:
	devcb_write_line   m_write_clk;
	devcb_write_line   m_write_data;

	inline void set_signal(device_t *device, int signal, int state);
	inline int get_signal(int signal);

	int m_line[SIGNAL_COUNT];
};


// ======================> econet_slot_device

class econet_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	econet_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void static_set_slot(device_t &device, int address);

private:
	// configuration
	UINT8 m_address;
	econet_device  *m_econet;
};


// ======================> device_econet_interface

class device_econet_interface : public device_slot_card_interface
{
	friend class econet_device;

public:
	// construction/destruction
	device_econet_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_econet_interface() { }

	device_econet_interface *next() const { return m_next; }
	device_econet_interface *m_next;

	virtual void econet_clk(int state) = 0;
	virtual void econet_data(int state) = 0;

	econet_device  *m_econet;
	UINT8 m_address;
};


// device type definition
extern const device_type ECONET;
extern const device_type ECONET_SLOT;


SLOT_INTERFACE_EXTERN( econet_devices );



#endif
