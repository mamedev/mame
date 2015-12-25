// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Disk Controller Card emulation

**********************************************************************/

#pragma once

#ifndef __COMX_FD__
#define __COMX_FD__

#include "emu.h"
#include "exp.h"
#include "formats/comx35_dsk.h"
#include "machine/wd_fdc.h"



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
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_comx_expansion_card_interface overrides
	virtual int comx_ef4_r() override;
	virtual void comx_q_w(int state) override;
	virtual UINT8 comx_mrd_r(address_space &space, offs_t offset, int *extrom) override;
	virtual UINT8 comx_io_r(address_space &space, offs_t offset) override;
	virtual void comx_io_w(address_space &space, offs_t offset, UINT8 data) override;

private:
	// internal state
	required_device<wd1770_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_rom;

	// floppy state
	int m_q;                // FDC register select
	int m_addr;             // FDC address
	int m_disb;             // data request disable
};


// device type definition
extern const device_type COMX_FD;


#endif
