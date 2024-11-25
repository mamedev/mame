// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 2031 Single Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C2031_H
#define MAME_BUS_IEEE488_C2031_H

#pragma once

#include "ieee488.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/floppy.h"
#include "machine/64h156.h"
#include "machine/6522via.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c2031_device

class c2031_device :  public device_t,
						public device_ieee488_interface
{
public:
	// construction/destruction
	c2031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_ieee488_interface overrides
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ifc(int state) override;

private:
	inline int get_device_number();

	void via0_irq_w(int state);
	uint8_t via0_pa_r();
	void via0_pa_w(uint8_t data);
	uint8_t via0_pb_r();
	void via0_pb_w(uint8_t data);
	void via1_irq_w(int state);
	uint8_t via1_pb_r();
	void via1_pb_w(uint8_t data);
	void byte_w(int state);

	void c2031_mem(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<c64h156_device> m_ga;
	required_device<floppy_image_device> m_floppy;
	required_ioport m_address;
	output_finder<2> m_leds;

	// IEEE-488 bus
	int m_nrfd_out;             // not ready for data
	int m_ndac_out;             // not data accepted
	int m_atna;                 // attention acknowledge
	int m_ifc;

	// interrupts
	int m_via0_irq;             // VIA #0 interrupt request
	int m_via1_irq;             // VIA #1 interrupt request
};


// device type definition
DECLARE_DEVICE_TYPE(C2031, c2031_device)


#endif // MAME_BUS_IEEE488_C2031_H
