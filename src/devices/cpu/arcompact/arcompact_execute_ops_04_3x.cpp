// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LD<zz><.x><.aa><.di> a,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CCAA AAAA
// LD<zz><.x><.aa><.di> 0,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CC11 1110
// PREFETCH<.aa> [b,c]             0010 0bbb aa11 0000   0BBB CCCC CC11 1110    (prefetch is an alias)
//
// LD<zz><.x><.aa><.di> a,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 10AA AAAA (+ Limm)
// LD<zz><.x><.aa><.di> 0,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 1011 1110 (+ Limm)
// PREFETCH<.aa> [b,limm]          0010 0bbb aa11 0000   0BBB 1111 1011 1110 (+ Limm) (prefetch is an alias)
//
// LD<zz><.x><.di> a,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CCAA AAAA (+ Limm)
// LD<zz><.x><.di> 0,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CC11 1110 (+ Limm)
// PREFETCH [limm,c]               0010 0110 RR11 0000   0111 CCCC CC11 1110 (+ Limm) (prefetch is an alias)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_LDrr(uint32_t op, int dsize, int extend)
{
	// ZZ value of 0x0 with X of 1 is illegal
	// ZZ value of 0x3 is illegal

	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_limm(breg, creg);
	uint8_t areg = common32_get_areg(op);

	uint8_t X = extend;
	uint32_t s = m_regs[creg];
	int a = (op & 0x00c00000) >> 22;
	//int D = (op & 0x00008000) >> 15; // D isn't handled
	uint8_t Z = dsize;

	arcompact_handle_ld_helper(op, areg, breg, s, X, Z, a);

	return m_pc + size;
}
