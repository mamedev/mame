// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio 128KB RAM card emulation

**********************************************************************/

#ifndef MAME_BUS_POFO_RAM_H
#define MAME_BUS_POFO_RAM_H

#pragma once

#include "ccm.h"
#include "fileio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> portfolio_ram_card_device

class portfolio_ram_card_device :  public device_t,
							  public device_portfolio_memory_card_slot_interface,
							  public device_nvram_interface
{
public:
	// construction/destruction
	portfolio_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override { }
	virtual void nvram_read(emu_file &file) override { if (m_nvram != nullptr) { file.read(m_nvram, m_nvram.bytes()); } }
	virtual void nvram_write(emu_file &file) override { if (m_nvram != nullptr) { file.write(m_nvram, m_nvram.bytes()); } }

	// device_portfolio_memory_card_slot_interface overrides
	virtual bool cdet() override { return 0; }

	virtual uint8_t nrdi_r(offs_t offset) override;
	virtual void nwri_w(offs_t offset, uint8_t data) override;

	memory_share_creator<uint8_t> m_nvram;
};


// device type definition
DECLARE_DEVICE_TYPE(PORTFOLIO_RAM_CARD, portfolio_ram_card_device)

#endif // MAME_BUS_POFO_RAM_H
