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
#include "imagedev/floppy.h"
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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_newbrain_expansion_slot_interface overrides
	virtual uint8_t mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual void mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) override;
	virtual uint8_t iorq_r(offs_t offset, uint8_t data, bool &prtov) override;
	virtual void iorq_w(offs_t offset, uint8_t data, bool &prtov) override;

private:
	void fdc_int_w(int state);

	void fdc_auxiliary_w(uint8_t data);
	uint8_t fdc_control_r();
	void io_dec_w(uint8_t data);

	void newbrain_fdc_io(address_map &map) ATTR_COLD;
	void newbrain_fdc_mem(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
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
