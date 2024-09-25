// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Serial/Parallel Printer Card emulation

**********************************************************************/

#ifndef MAME_BUS_COMX35_PRINTER_H
#define MAME_BUS_COMX35_PRINTER_H

#pragma once

#include "exp.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_prn_device

class comx_prn_device : public device_t, public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_comx_expansion_card_interface overrides
	virtual uint8_t comx_mrd_r(offs_t offset, int *extrom) override;
	virtual uint8_t comx_io_r(offs_t offset) override;
	virtual void comx_io_w(offs_t offset, uint8_t data) override;

private:
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_buffer_device> m_cent_status_in;
	required_memory_region m_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(COMX_PRN, comx_prn_device)


#endif // MAME_BUS_COMX35_PRINTER_H
