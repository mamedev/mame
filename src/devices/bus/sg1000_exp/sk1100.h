// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sega SK-1100 keyboard emulation

**********************************************************************/

#ifndef MAME_BUS_SG1000_EXP_SK1100_H
#define MAME_BUS_SG1000_EXP_SK1100_H

#pragma once


#include "sg1000exp.h"
#include "sk1100prn.h"
#include "formats/sc3000_bit.h"
#include "imagedev/cassette.h"
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

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_sg1000_expansion_slot_interface overrides
	virtual DECLARE_READ8_MEMBER(peripheral_r) override;
	virtual DECLARE_WRITE8_MEMBER(peripheral_w) override;
	virtual bool is_readable(uint8_t offset) override;

private:
	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );

	required_device<cassette_image_device> m_cassette;
	required_device<i8255_device> m_ppi;
	required_device<sk1100_printer_port_device> m_printer_port;
	required_ioport_array<8> m_pa;
	required_ioport_array<8> m_pb;

	/* keyboard state */
	uint8_t m_keylatch;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_SK1100, sega_sk1100_device)


#endif // MAME_BUS_SG1000_EXP_SK1100_H
