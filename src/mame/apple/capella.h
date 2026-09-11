// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks
#ifndef MAME_APPLE_CAPELLA_H
#define MAME_APPLE_CAPELLA_H

#pragma once

#include "cpu/powerpc/ppc.h"

class capella_device : public device_t
{
public:
	// construction/destruction
	capella_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	virtual void map(address_map &map) ATTR_COLD;

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }

	// 68040-style interrupt priority level from the I/O controller (1-7, anything else = no interrupt)
	void translate_ipl_state_change(int ipl);
	// NMI (programmer's switch / Cuda), presented as IPL 7
	void nmi_w(int state);

	u64 ctrl_r(offs_t offset);
	void ctrl_w(offs_t offset, u64 data);

	u64 ctrl_b_r(offs_t offset);
	void ctrl_b_w(offs_t offset, u64 data);

	u64 ipl_lines_r(offs_t offset);

	u64 irq_ack_r(offs_t offset);
	void irq_ack_w(offs_t offset, u64 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_ipl();

	required_device<ppc_device> m_maincpu;

	u8 m_ctrl_reg;
	u8 m_ctrl_reg_b;
	u8 m_iosb_ipl;      // level requested by the I/O controller (0 = none)
	u8 m_nmi;           // NMI input state
	u8 m_ipl;           // level currently presented to the CPU (0 = none)
	bool m_irq_pending; // IPL change latched, CPU interrupt asserted
};

// device type definition
DECLARE_DEVICE_TYPE(CAPELLA, capella_device)

#endif // MAME_APPLE_CAPELLA_H
