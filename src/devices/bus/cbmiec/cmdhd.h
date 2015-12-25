// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD HD disk drive emulation

**********************************************************************/

#pragma once

#ifndef __CMD_HD__
#define __CMD_HD__

#include "emu.h"
#include "cbmiec.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/harddriv.h"
#include "machine/6522via.h"
#include "machine/i8255.h"
#include "bus/scsi/scsihd.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CMD_HD_TAG          "cmdhd"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cmd_hd_device

class cmd_hd_device :  public device_t,
						public device_cbm_iec_interface
{
public:
	// construction/destruction
	cmd_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE8_MEMBER( led_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_cbm_iec_interface overrides
	void cbm_iec_srq(int state) override;
	void cbm_iec_atn(int state) override;
	void cbm_iec_data(int state) override;
	void cbm_iec_reset(int state) override;

	required_device<cpu_device> m_maincpu;
	required_device<SCSI_PORT_DEVICE> m_scsibus;
};


// device type definition
extern const device_type CMD_HD;



#endif
