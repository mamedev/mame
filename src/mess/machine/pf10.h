/**********************************************************************

    EPSON PF-10

    Battery operated portable 3.5" floppy drive

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __PF10_H__
#define __PF10_H__

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/upd765.h"
#include "machine/epson_sio.h"
#include "imagedev/flopdrv.h"
#include "formats/mfi_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/d88_dsk.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_pf10_device : public device_t,
                          public device_epson_sio_interface
{
public:
	// construction/destruction
	epson_pf10_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "epson_pf10"; }
	virtual void device_start();

	// device_epson_sio_interface overrides
	virtual int rx_r();
	virtual int pin_r();
	virtual void tx_w(int level);
	virtual void pout_w(int level);

private:
	required_device<cpu_device> m_cpu;
	required_device<upd765a_device> m_fdc;
	required_device<epson_sio_device> m_sio;

	floppy_image_device *m_floppy;
};


// device type definition
extern const device_type EPSON_PF10;


#endif // __PF10_H__
