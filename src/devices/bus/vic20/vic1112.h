// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1112 IEEE-488 Interface Cartridge emulation

    SYS 45065 to start

**********************************************************************/

#pragma once

#ifndef __VIC1112__
#define __VIC1112__

#include "emu.h"
#include "exp.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1112_device

class vic1112_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1112_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( via0_irq_w );
	DECLARE_READ8_MEMBER( via0_pb_r );
	DECLARE_WRITE8_MEMBER( via0_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_vic20_expansion_card_interface overrides
	virtual UINT8 vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<ieee488_device> m_bus;

	//UINT8 *m_rom;

	int m_via0_irq;
	int m_via1_irq;
};


// device type definition
extern const device_type VIC1112;



#endif
