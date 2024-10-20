// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer ASCII Turbofile SRAM external storage device

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_TURBOFILE_H
#define MAME_BUS_NES_CTRL_TURBOFILE_H

#pragma once

#include "ctrl.h"
#include "machine/nvram.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_turbofile_device

class nes_turbofile_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_turbofile_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

	DECLARE_INPUT_CHANGED_MEMBER(lock_changed);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<nvram_device> m_nvram;
	required_ioport m_lock;

	std::unique_ptr<u8[]> m_ram;
	u16 m_addr;
	u8 m_bit;
	u8 m_latch;
	u8 m_locked;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TURBOFILE, nes_turbofile_device)

#endif // MAME_BUS_NES_CTRL_TURBOFILE_H
