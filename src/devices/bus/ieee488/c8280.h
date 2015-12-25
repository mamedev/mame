// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8280 Dual 8" Disk Drive emulation

**********************************************************************/

#pragma once

#ifndef __C8280__
#define __C8280__

#include "emu.h"
#include "ieee488.h"
#include "cpu/m6502/m6502.h"
#include "formats/c8280_dsk.h"
#include "machine/mos6530n.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c8280_t

class c8280_t :  public device_t,
					public device_ieee488_interface
{
public:
	// construction/destruction
	c8280_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( riot1_pa_r );
	DECLARE_WRITE8_MEMBER( riot1_pa_w );
	DECLARE_READ8_MEMBER( riot1_pb_r );
	DECLARE_WRITE8_MEMBER( riot1_pb_w );
	DECLARE_READ8_MEMBER( fk5_r );
	DECLARE_WRITE8_MEMBER( fk5_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_ieee488_interface overrides
	void ieee488_atn(int state) override;
	void ieee488_ifc(int state) override;

private:
	inline void update_ieee_signals();

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_fdccpu;
	required_device<mos6532_t> m_riot0;
	required_device<mos6532_t> m_riot1;
	required_device<fd1797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_ioport m_address;
	floppy_image_device *m_floppy;

	// IEEE-488 bus
	int m_rfdo;                         // not ready for data output
	int m_daco;                         // not data accepted output
	int m_atna;                         // attention acknowledge
	int m_ifc;

	UINT8 m_fk5;
};


// device type definition
extern const device_type C8280;



#endif
