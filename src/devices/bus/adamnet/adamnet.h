// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco ADAMnet bus emulation

**********************************************************************/

#pragma once

#ifndef __ADAMNET__
#define __ADAMNET__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define ADAMNET_TAG     "adamnet"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ADAMNET_BUS_ADD() \
	MCFG_DEVICE_ADD(ADAMNET_TAG, ADAMNET, 0)

#define MCFG_ADAMNET_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ADAMNET_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_adamnet_card_interface;

// ======================> adamnet_device

class adamnet_device : public device_t
{
public:
	// construction/destruction
	adamnet_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void add_device(device_t *target);

	DECLARE_READ_LINE_MEMBER( rxd_r );
	int rxd_r(device_t *device);
	DECLARE_WRITE_LINE_MEMBER( txd_w );
	void txd_w(device_t *device, int state);

	DECLARE_READ_LINE_MEMBER( reset_r );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;

private:
	class daisy_entry
	{
	public:
		daisy_entry(device_t *device);
		daisy_entry *next() const { return m_next; }

		daisy_entry *               m_next;         // next device
		device_t *                  m_device;       // associated device
		device_adamnet_card_interface * m_interface;    // associated device's daisy interface

		int m_txd;
	};

	simple_list<daisy_entry> m_device_list;

	int m_txd;
	int m_reset;
};


// ======================> adamnet_slot_device

class adamnet_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	adamnet_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;

protected:
	// configuration
	adamnet_device  *m_bus;
};


// ======================> device_adamnet_card_interface

class device_adamnet_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_adamnet_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_adamnet_card_interface();

	virtual void adamnet_reset_w(int state) = 0;

	adamnet_device  *m_bus;
};


// device type definitions
extern const device_type ADAMNET;
extern const device_type ADAMNET_SLOT;


SLOT_INTERFACE_EXTERN( adamnet_devices );



#endif
