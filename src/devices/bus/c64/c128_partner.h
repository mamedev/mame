// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Timeworks PARTNER 128 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_C128_PARTNER_H
#define MAME_BUS_C64_C128_PARTNER_H

#pragma once

#include "bus/c64/exp.h"
#include "bus/vcs_ctrl/ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c128_partner_cartridge_device

class c128_partner_cartridge_device : public device_t,
						public device_c64_expansion_card_interface
						//public device_vcs_control_port_interface
{
public:
	// construction/destruction
	c128_partner_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void nmi_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

	// device_vcs_control_port_interface overrides
	virtual void vcs_joy_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(update_joyb2);

private:
	memory_share_creator<uint8_t> m_ram;

	emu_timer *t_joyb2;
	int m_ram_a12_a7;
	int m_ls74_cd;
	int m_ls74_q1;
	int m_ls74_q2;
	int m_joyb2;
};


// device type definition
DECLARE_DEVICE_TYPE(C128_PARTNER, c128_partner_cartridge_device)


#endif // MAME_BUS_C64_C128_PARTNER_H
