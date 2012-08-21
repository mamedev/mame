/**********************************************************************

    COMX-35 Disk Controller Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMX_FD__
#define __COMX_FD__


#include "emu.h"
#include "formats/basicdsk.h"
#include "formats/comx35_dsk.h"
#include "imagedev/flopdrv.h"
#include "machine/comxexp.h"
#include "machine/wd17xx.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_fd_device

class comx_fd_device : public device_t,
					   public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_fd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "comx_fd"; }

	// device_comx_expansion_card_interface overrides
	virtual void comx_ds_w(int state);
	virtual void comx_q_w(int state);
	virtual UINT8 comx_mrd_r(offs_t offset, int *extrom);
	virtual UINT8 comx_io_r(offs_t offset);
	virtual void comx_io_w(offs_t offset, UINT8 data);

private:
	inline void update_ef4();

	// internal state
	required_device<device_t> m_fdc;
	required_device<device_t> m_floppy0;
	required_device<device_t> m_floppy1;

	// floppy state
	int m_ds;				// device select
	UINT8 *m_rom;
	int m_q;				// FDC register select
	int m_addr;				// FDC address
	int m_intrq;			// interrupt request
	int m_drq;				// data request
	int m_disb;				// data request disable
};


// device type definition
extern const device_type COMX_FD;


#endif
