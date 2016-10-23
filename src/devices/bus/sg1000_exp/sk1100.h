// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sega SK-1100 keyboard emulation

**********************************************************************/

#pragma once

#ifndef __SEGA_SK1100__
#define __SEGA_SK1100__


#include "emu.h"
#include "sg1000exp.h"
#include "formats/sc3000_bit.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "machine/i8255.h"


#define UPD9255_0_TAG   "upd9255_0" // "upd9255_1" is being used by the SF-7000 driver


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sega_sk1100_device

class sega_sk1100_device : public device_t,
							public device_sg1000_expansion_slot_interface
{
public:
	// construction/destruction
	sega_sk1100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	uint8_t ppi_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ppi_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ppi_pc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_sg1000_expansion_slot_interface overrides
	virtual uint8_t peripheral_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void peripheral_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual bool is_readable(uint8_t offset) override;

private:
	required_device<cassette_image_device> m_cassette;
	required_device<i8255_device> m_ppi;
	required_ioport_array<8> m_pa;
	required_ioport_array<8> m_pb;

	/* keyboard state */
	uint8_t m_keylatch;
};


// device type definition
extern const device_type SEGA_SK1100;


#endif
