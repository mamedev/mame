// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    z80daisy.h

    Z80/180 daisy chaining support functions.

***************************************************************************/

#ifndef MAME_MACHINE_Z80DAISY_H
#define MAME_MACHINE_Z80DAISY_H

#pragma once


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these constants are returned from the irq_state function
const uint8_t Z80_DAISY_INT = 0x01;       // interrupt request mask
const uint8_t Z80_DAISY_IEO = 0x02;       // interrupt disable mask (IEO)


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

	// interface-level overrides
	virtual void interface_post_start() override;
	virtual void interface_post_reset() override;

	// required operation overrides
	virtual int z80daisy_irq_state() = 0;
	virtual int z80daisy_irq_ack() = 0;
	virtual void z80daisy_irq_reti() = 0;

	// instruction decoding
	void z80daisy_decode(uint8_t opcode);

private:
	device_z80daisy_interface *m_daisy_next;    // next device in the chain
	uint8_t m_last_opcode;
};



// ======================> z80_daisy_chain_interface

class z80_daisy_chain_interface : public device_interface
{
public:
	// construction/destruction
	z80_daisy_chain_interface(const machine_config &mconfig, device_t &device);
	virtual ~z80_daisy_chain_interface();

	// configuration helpers
	void set_daisy_config(const z80_daisy_config *config) { m_daisy_config = config; }

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
	device_z80daisy_interface *daisy_get_irq_device();
	void daisy_call_reti_device();

private:
	const z80_daisy_config *m_daisy_config;
	device_z80daisy_interface *m_chain;     // head of the daisy chain
};


#endif // MAME_MACHINE_Z80DAISY_H
