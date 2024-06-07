// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_CPU_Z80_Z80N_H
#define MAME_CPU_Z80_Z80N_H

#pragma once

#include "z80.h"

class z80n_device : public z80_device
{
public:
	// construction/destruction
	z80n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto in_nextreg_cb() { return m_in_nextreg_cb.bind(); }
	auto out_nextreg_cb() { return m_out_nextreg_cb.bind(); }
	auto out_retn_seen_cb() { return m_out_retn_seen_cb.bind(); }

	bool nmi_stackless_r() { return m_stackless; }
	void nmi_stackless_w(bool data) { m_stackless = data; }

	void nmi() { m_nmi_pending = true; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void do_op() override;

	devcb_write8 m_out_retn_seen_cb;
	devcb_read8 m_in_nextreg_cb;
	devcb_write8 m_out_nextreg_cb;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	bool m_stackless;

};

DECLARE_DEVICE_TYPE(Z80N, z80n_device)

#endif // MAME_CPU_Z80_Z80N_H
