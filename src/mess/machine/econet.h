/**********************************************************************

    Acorn Computers Econet local area network emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#define MCFG_ECONET_ADD(_config) \
	MCFG_DEVICE_ADD(ECONET_TAG, ECONET, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define ECONET_INTERFACE(_name) \
	const econet_interface (_name) =


#define MCFG_ECONET_SLOT_ADD(_tag, _num, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, ECONET_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	econet_slot_device::static_set_slot(*device, _num);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> econet_interface

struct econet_interface
{
	devcb_write_line    m_out_clk_cb;
	devcb_write_line    m_out_data_cb;
};


// ======================> econet_device

class device_econet_interface;

class econet_device : public device_t,
						public econet_interface
{
public:
	// construction/destruction
	econet_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void add_device(device_t *target, int address);

	// reads for both host and peripherals
	DECLARE_READ_LINE_MEMBER( clk_r );
	DECLARE_READ_LINE_MEMBER( data_r );

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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

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
	devcb_resolved_write_line   m_out_clk_func;
	devcb_resolved_write_line   m_out_data_func;

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
	econet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

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
	virtual ~device_econet_interface();

	device_econet_interface *next() const { return m_next; }
	device_econet_interface *m_next;

	// optional operation overrides
	virtual void econet_clk(int state) = 0;
	virtual void econet_data(int state) { };

	econet_device  *m_econet;
	UINT8 m_address;
};


// device type definition
extern const device_type ECONET;
extern const device_type ECONET_SLOT;



#endif
