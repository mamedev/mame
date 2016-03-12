// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain FDC emulation

**********************************************************************/

#pragma once

#ifndef __NEWBRAIN_FDC__
#define __NEWBRAIN_FDC__

#include "emu.h"
#include "exp.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> newbrain_fdc_t

class newbrain_fdc_t :  public device_t,
						public device_newbrain_expansion_slot_interface
{
public:
	// construction/destruction
	newbrain_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER( fdc_int_w );
	DECLARE_WRITE8_MEMBER( fdc_auxiliary_w );
	DECLARE_READ8_MEMBER( fdc_control_r );
	DECLARE_WRITE8_MEMBER( io_dec_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_newbrain_expansion_slot_interface overrides
	virtual UINT8 mreq_r(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh) override;
	virtual void mreq_w(address_space &space, offs_t offset, UINT8 data, bool &romov, int &exrm, bool &raminh) override;
	virtual UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data, bool &prtov) override;
	virtual void iorq_w(address_space &space, offs_t offset, UINT8 data, bool &prtov) override;

private:
	required_device<z80_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<newbrain_expansion_slot_t> m_exp;

	void moton(int state);

	int m_paging;
	int m_ma16;
	int m_mpm;
	int m_fdc_att;
	int m_fdc_int;
	int m_pa15;
};


// device type definition
extern const device_type NEWBRAIN_FDC;



#endif
