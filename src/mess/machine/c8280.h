/**********************************************************************

    Commodore 8280 Dual 8" Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C8280__
#define __C8280__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6532riot.h"
#include "machine/ieee488.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c8280_device

class c8280_device :  public device_t,
					  public device_ieee488_interface
{
public:
    // construction/destruction
    c8280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( riot1_pa_r );
	DECLARE_WRITE8_MEMBER( riot1_pa_w );
	DECLARE_READ8_MEMBER( riot1_pb_r );
	DECLARE_WRITE8_MEMBER( riot1_pb_w );
	DECLARE_READ8_MEMBER( fk5_r );
	DECLARE_WRITE8_MEMBER( fk5_w );
	void fdc_intrq_w(bool state);
	void fdc_drq_w(bool state);

protected:
    // device-level overrides
    virtual void device_config_complete() { m_shortname = "c8280"; }
    virtual void device_start();
	virtual void device_reset();

	// device_ieee488_interface overrides
	void ieee488_atn(int state);
	void ieee488_ifc(int state);

private:
	inline void update_ieee_signals();

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_fdccpu;
	required_device<riot6532_device> m_riot0;
	required_device<riot6532_device> m_riot1;
	required_device<fd1797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;

	// IEEE-488 bus
	int m_rfdo;							// not ready for data output
	int m_daco;							// not data accepted output
	int m_atna;							// attention acknowledge

	UINT8 m_fk5;
};


// device type definition
extern const device_type C8280;



#endif
