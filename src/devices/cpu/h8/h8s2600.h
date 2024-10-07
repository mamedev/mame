// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2600.h

    H8S-2600 base cpu emulation

    Adds the multiply-and-accumulate register and related instructions


***************************************************************************/

#ifndef MAME_CPU_H8_H8S2600_H
#define MAME_CPU_H8_H8S2600_H

#pragma once

#include "h8s2000.h"

class h8s2600_device : public h8s2000_device {
protected:
	h8s2600_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	bool m_mac_saturating;

#define O(o) void o ## _full(); void o ## _partial()
	O(clrmac);
	O(ldmac_r32l_mach); O(ldmac_r32l_macl);
	O(mac_r32ph_r32pl);
	O(stmac_mach_r32l); O(stmac_macl_r32l);
#undef O
};

#endif // MAME_CPU_H8_H8S2600_H
