// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2estd80col.c

    Apple IIe Standard 80 Column Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2ESTD80COL_H
#define MAME_BUS_A2BUS_A2ESTD80COL_H

#pragma once

#include "a2eauxslot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2eaux_std80col_device:
	public device_t,
	public device_a2eauxslot_card_interface
{
public:
	// construction/destruction
	a2eaux_std80col_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2eaux_std80col_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_auxram(uint16_t offset) override;
	virtual void write_auxram(uint16_t offset, uint8_t data) override;
	virtual uint8_t *get_vram_ptr() override;
	virtual uint8_t *get_auxbank_ptr() override;
	virtual bool allow_dhr() override { return false; }  // we don't allow DHR

private:
	uint8_t m_ram[2*1024];
};

// device type definition
DECLARE_DEVICE_TYPE(A2EAUX_STD80COL, a2eaux_std80col_device)

#endif // MAME_BUS_A2BUS_A2ESTD80COL_H
