// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Naberezny
/**********************************************************************

    SSE SoftBox emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_SOFTBOX_H
#define MAME_BUS_IEEE488_SOFTBOX_H

#pragma once

#include "ieee488.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/harddriv.h"
#include "machine/corvushd.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> softbox_device

class softbox_device :  public device_t,
						public device_ieee488_interface
{
public:
	// construction/destruction
	softbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset_after_children() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_ieee488_interface overrides
	virtual void ieee488_ifc(int state) override;

private:
	DECLARE_READ8_MEMBER( ppi0_pa_r );
	DECLARE_WRITE8_MEMBER( ppi0_pb_w );

	DECLARE_READ8_MEMBER( ppi1_pa_r );
	DECLARE_WRITE8_MEMBER( ppi1_pb_w );
	DECLARE_READ8_MEMBER( ppi1_pc_r );
	DECLARE_WRITE8_MEMBER( ppi1_pc_w );

	DECLARE_WRITE8_MEMBER( dbrg_w );

	void softbox_io(address_map &map);
	void softbox_mem(address_map &map);

	enum
	{
		LED_A = 0,
		LED_B,
		LED_READY
	};

	required_device<cpu_device> m_maincpu;
	required_device<com8116_device> m_dbrg;
	required_device<corvus_hdc_device> m_hdc;
	output_finder<3> m_leds;

	int m_ifc;  // Tracks previous state of IEEE-488 IFC line
};


// device type definition
DECLARE_DEVICE_TYPE(SOFTBOX, softbox_device)


#endif // MAME_BUS_IEEE488_SOFTBOX_H
