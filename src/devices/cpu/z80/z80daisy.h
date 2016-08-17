// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    z80daisy.h

    Z80/180 daisy chaining support functions.

***************************************************************************/

#pragma once

#ifndef __Z80DAISY_H__
#define __Z80DAISY_H__



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these constants are returned from the irq_state function
const UINT8 Z80_DAISY_INT = 0x01;       // interrupt request mask
const UINT8 Z80_DAISY_IEO = 0x02;       // interrupt disable mask (IEO)



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// configure devices
#define MCFG_Z80_DAISY_CHAIN(_config) \
	z80_daisy_chain_interface::static_set_daisy_config(*device, _config);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80_daisy_config

struct z80_daisy_config
{
	const char *    devname;                    // name of the device
};



// ======================> device_z80daisy_interface

class device_z80daisy_interface : public device_interface
{
	friend class z80_daisy_chain_interface;

public:
	// construction/destruction
	device_z80daisy_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_z80daisy_interface();

	// required operation overrides
	virtual int z80daisy_irq_state() = 0;
	virtual int z80daisy_irq_ack() = 0;
	virtual void z80daisy_irq_reti() = 0;

private:
	device_z80daisy_interface *m_daisy_next;    // next device in the chain
};



// ======================> z80_daisy_chain_interface

class z80_daisy_chain_interface : public device_interface
{
public:
	// construction/destruction
	z80_daisy_chain_interface(const machine_config &mconfig, device_t &device);
	virtual ~z80_daisy_chain_interface();

	// configuration helpers
	static void static_set_daisy_config(device_t &device, const z80_daisy_config *config);

	// getters
	bool daisy_chain_present() const { return (m_chain != nullptr); }
	std::string daisy_show_chain() const;

protected:
	// interface-level overrides
	virtual void interface_post_start() override;
	virtual void interface_post_reset() override;

	// initialization
	void daisy_init(const z80_daisy_config *daisy);

	// callbacks
	int daisy_update_irq_state();
	int daisy_call_ack_device();
	void daisy_call_reti_device();

private:
	const z80_daisy_config *m_daisy_config;
	device_z80daisy_interface *m_chain;     // head of the daisy chain
};


#endif
