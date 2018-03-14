// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain FDC emulation

**********************************************************************/

#ifndef MAME_BUS_NEWBRAIN_FDC_H
#define MAME_BUS_NEWBRAIN_FDC_H

#pragma once

#include "exp.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> newbrain_fdc_device

class newbrain_fdc_device :  public device_t, public device_newbrain_expansion_slot_interface
{
public:
	// construction/destruction
	newbrain_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_newbrain_expansion_slot_interface overrides
	virtual uint8_t mreq_r(address_space &space, offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual void mreq_w(address_space &space, offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual uint8_t iorq_r(address_space &space, offs_t offset, uint8_t data, bool &prtov) override;
	virtual void iorq_w(address_space &space, offs_t offset, uint8_t data, bool &prtov) override;

private:
	DECLARE_WRITE_LINE_MEMBER( fdc_int_w );

	DECLARE_WRITE8_MEMBER( fdc_auxiliary_w );
	DECLARE_READ8_MEMBER( fdc_control_r );
	DECLARE_WRITE8_MEMBER( io_dec_w );

	void newbrain_fdc_io(address_map &map);
	void newbrain_fdc_mem(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<newbrain_expansion_slot_device> m_exp;

	void moton(int state);

	int m_paging;
	int m_ma16;
	int m_mpm;
	int m_fdc_att;
	int m_fdc_int;
	int m_pa15;
};


// device type definition
DECLARE_DEVICE_TYPE(NEWBRAIN_FDC, newbrain_fdc_device)

#endif // MAME_BUS_NEWBRAIN_FDC_H
