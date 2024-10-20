// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2eext80col.h

    Apple IIe Extended 80 Column Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2EEXT809COL_H
#define MAME_BUS_A2BUS_A2EEXT809COL_H

#pragma once

#include "a2eauxslot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2eaux_ext80col_device:
	public device_t,
	public device_a2eauxslot_card_interface
{
public:
	// construction/destruction
	a2eaux_ext80col_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2eaux_ext80col_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u8 read_auxram(uint16_t offset) override;
	virtual void write_auxram(uint16_t offset, u8 data) override;
	virtual u8 *get_vram_ptr() override;
	virtual u8 *get_auxbank_ptr() override;
	virtual u16 get_auxbank_mask() override;
	virtual bool allow_dhr() override { return true; }

private:
	u8 m_ram[64*1024];
};

// device type definition
DECLARE_DEVICE_TYPE(A2EAUX_EXT80COL, a2eaux_ext80col_device)

#endif // MAME_BUS_A2BUS_A2EEXT809COL_H
