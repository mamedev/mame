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
	sega_sk1100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_sg1000_expansion_slot_interface overrides
	virtual DECLARE_READ8_MEMBER(peripheral_r) override;
	virtual DECLARE_WRITE8_MEMBER(peripheral_w) override;
	virtual bool is_readable(UINT8 offset) override;

private:
	required_device<cassette_image_device> m_cassette;
	required_device<i8255_device> m_ppi;
	required_ioport_array<8> m_pa;
	required_ioport_array<8> m_pb;

	/* keyboard state */
	UINT8 m_keylatch;
};


// device type definition
extern const device_type SEGA_SK1100;


#endif
