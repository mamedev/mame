// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

	Z80N

***************************************************************************/

#include "emu.h"
#include "z80n.h"
#include "z80ndasm.h"

#ifndef PCD
#define PCD     m_pc.d
#define PC      m_pc.w.l

#define A       m_af.b.h

#define BC      m_bc.w.l
#define B       m_bc.b.h

#define DE      m_de.w.l
#define D       m_de.b.h
#define E       m_de.b.l

#define HL      m_hl.w.l
#define H       m_hl.b.h
#define L       m_hl.b.l
#endif

DEFINE_DEVICE_TYPE(Z80N, z80n_device, "z80n", "Z80N")

std::unique_ptr<util::disasm_interface> z80n_device::create_disassembler()
{
	return std::make_unique<z80n_disassembler>();
}

z80n_device::z80n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, Z80N, tag, owner, clock)
	, m_out_retn_seen_cb(*this)
	, m_in_nextreg_cb(*this, 0)
	, m_out_nextreg_cb(*this)
{
}

void z80n_device::retn()
{
	m_out_retn_seen_cb(0);
	z80_device::retn();
	if (m_stackless)
	{
		m_pc.b.l = m_in_nextreg_cb(0xc2);
		m_pc.b.h = m_in_nextreg_cb(0xc3);
	}
}

void z80n_device::ed_23()
{
	A = (A << 4) | (A >> 4);
}

void z80n_device::ed_24()
{
	A = bitswap<8>(A, 0, 1, 2, 3, 4, 5, 6, 7);
}

void z80n_device::ed_27()
{
	const u8 a = A;
	and_a(arg());
	A = a;
}

void z80n_device::ed_28()
{
	DE <<= std::min(B & 31, 16);
}

void z80n_device::ed_29()
{
	const u16 fill = (DE & 0x8000) ? ~u16(0) : u16(0);
	DE = (DE >> std::min(B & 31, 16)) | (fill << (16 - std::min(B & 31, 16)));
}

void z80n_device::ed_2a()
{
	DE >>= std::min(B & 31, 16);
}

void z80n_device::ed_2b()
{
	DE = (DE >> std::min(B & 31, 16)) | (~u16(0) << (16 - std::min(B & 31, 16)));
}

void z80n_device::ed_2c()
{
	DE = (DE << (B & 15)) | (DE >> (16 - (B & 15)));
}

void z80n_device::ed_30()
{
	DE = D * E;
}

void z80n_device::ed_31()
{
	HL += A;
}

void z80n_device::ed_32()
{
	DE += A;
}

void z80n_device::ed_33()
{
	BC += A;
}

void z80n_device::ed_34()
{
	HL += arg16();
}

void z80n_device::ed_35()
{
	DE += arg16();
}

void z80n_device::ed_36()
{
	BC += arg16();
}

void z80n_device::ed_8a()
{
	PAIR tmp = {{0, 0, 0, 0}};
	tmp.b.h = arg();
	tmp.b.l = arg();
	push(tmp);
}

void z80n_device::ed_90()
{
	const u8 data = rm(HL);
	out(BC, data);
	HL++;
}

void z80n_device::ed_91()
{
	const u8 reg = arg();
	const u8 data = arg();
	m_out_nextreg_cb(reg, data);
}

void z80n_device::ed_92()
{
	const u8 reg = arg();
	m_out_nextreg_cb(reg, A);
}

void z80n_device::ed_93()
{
	if (0x07 != (H & 0x07))
		HL = HL + 0x100;
	else if (0xe0 != (L & 0xe0))
		HL = (HL & 0xf8ff) + 0x20;
	else
		HL = (HL & 0xf81f) + 0x800;
}

void z80n_device::ed_94()
{
	HL = 0x4000 + ((D & 0xc0) << 5) + ((D & 0x07) << 8) + ((D & 0x38) << 2) + (E >> 3);
}

void z80n_device::ed_95()
{
	A = 0x80 >> (E & 7);
}

void z80n_device::ed_98()
{
	PCD = (PCD & 0xc000) + (in(BC) << 6);
}

void z80n_device::ed_a4()
{
	u8 data = rm(HL);
	if (data != A)
		wm(DE, data);
	HL++;
	DE++;
	BC--;
}

void z80n_device::ed_a5()
{
	wm(DE, rm(HL));
	L++;
	D = inc(D);
}

void z80n_device::ed_ac()
{
	const u8 data = rm(HL);
	if (data != A)
		wm(DE, data);
	DE++;
	HL--;
	BC--;
}

void z80n_device::ed_b4()
{
	ed_a4();
	if (BC != 0)
		PC -= 2;
}

void z80n_device::ed_b7()
{
	const u8 data = rm((HL & 0xfff8) + (E & 7));
	if (data != A)
		wm(DE, data);
	DE++;
	BC--;
	if (BC != 0)
		PC -= 2;
}

void z80n_device::ed_bc()
{
	ed_ac();
	if (BC != 0)
		PC -= 2;
}

void z80n_device::take_nmi()
{
	if (m_stackless)
	{
		m_out_nextreg_cb(0xc2, m_pc.b.l);
		m_out_nextreg_cb(0xc3, m_pc.b.h);
	}
	z80_device::take_nmi();
}

void z80n_device::device_start()
{
	z80_device::device_start();
	save_item(NAME(m_stackless));
}

void z80n_device::device_reset()
{
	z80_device::device_reset();
	m_stackless = 0;
}
