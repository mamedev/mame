// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD SwiftLink RS-232 cartridge emulation

**********************************************************************/

#pragma once

#ifndef __SWIFTLINK__
#define __SWIFTLINK__


#include "exp.h"
#include "machine/mos6551.h"




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_swiftlink_cartridge_device

class c64_swiftlink_cartridge_device : public device_t,
										public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_swiftlink_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_device<mos6551_device> m_acia;
	required_ioport m_io_cs;
	required_ioport m_io_irq;

	enum
	{
		D700 = 0,
		DE00,
		DF00
	};

	enum
	{
		IRQ = 0,
		NMI
	};

	int m_cs;
	int m_irq;
};


// device type definition
extern const device_type C64_SWIFTLINK;


#endif
