// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8280 Dual 8" Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C8280_H
#define MAME_BUS_IEEE488_C8280_H

#pragma once

#include "ieee488.h"
#include "cpu/m6502/m6502.h"
#include "formats/c8280_dsk.h"
#include "imagedev/floppy.h"
#include "machine/mos6530n.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c8280_device

class c8280_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c8280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_ieee488_interface overrides
	void ieee488_atn(int state) override;
	void ieee488_ifc(int state) override;

private:
	inline void update_ieee_signals();

	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( riot1_pa_r );
	DECLARE_WRITE8_MEMBER( riot1_pa_w );
	DECLARE_READ8_MEMBER( riot1_pb_r );
	DECLARE_WRITE8_MEMBER( riot1_pb_w );
	DECLARE_READ8_MEMBER( fk5_r );
	DECLARE_WRITE8_MEMBER( fk5_w );

	void c8280_fdc_mem(address_map &map);
	void c8280_main_mem(address_map &map);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_fdccpu;
	required_device<mos6532_new_device> m_riot0;
	required_device<mos6532_new_device> m_riot1;
	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_ioport m_address;
	floppy_image_device *m_floppy;
	output_finder<4> m_leds;

	// IEEE-488 bus
	int m_rfdo;                         // not ready for data output
	int m_daco;                         // not data accepted output
	int m_atna;                         // attention acknowledge
	int m_ifc;

	uint8_t m_fk5;
};


// device type definition
DECLARE_DEVICE_TYPE(C8280, c8280_device)


#endif // MAME_BUS_IEEE488_C8280_H
