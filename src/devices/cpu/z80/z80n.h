// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

	Z80N

***************************************************************************/

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

	virtual void ed_23() override; // swapnib
	virtual void ed_24() override; // mirror a
	virtual void ed_27() override; // test *
	virtual void ed_28() override; // bsla de,b
	virtual void ed_29() override; // bsra de,b
	virtual void ed_2a() override; // bsrl de,b
	virtual void ed_2b() override; // bsrf de,b
	virtual void ed_2c() override; // brlc de,b
	virtual void ed_30() override; // mul d,e
	virtual void ed_31() override; // add hl,a
	virtual void ed_32() override; // add de,a
	virtual void ed_33() override; // add bc,a
	virtual void ed_34() override; // add hl,**
	virtual void ed_35() override; // add de,**
	virtual void ed_36() override; // add bc,**
	virtual void ed_8a() override; // push **
	virtual void ed_90() override; // outinb
	virtual void ed_91() override; // nextreg *,*
	virtual void ed_92() override; // nextreg *,a
	virtual void ed_93() override; // pixeldn
	virtual void ed_94() override; // pixelad
	virtual void ed_95() override; // setae
	virtual void ed_98() override; // jp (c)
	virtual void ed_a4() override; // ldix
	virtual void ed_a5() override; // ldws
	virtual void ed_ac() override; // lddx
	virtual void ed_b4() override; // ldirx
	virtual void ed_b7() override; // ldpirx
	virtual void ed_bc() override; // lddrx

	virtual void retn() override;
	virtual void take_nmi() override;

	devcb_write8 m_out_retn_seen_cb;
	devcb_read8 m_in_nextreg_cb;
	devcb_write8 m_out_nextreg_cb;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	bool m_stackless;

};

DECLARE_DEVICE_TYPE(Z80N, z80n_device)

#endif // MAME_CPU_Z80_Z80N_H
