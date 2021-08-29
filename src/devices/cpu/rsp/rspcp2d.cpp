// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspcp2d.c

    Universal machine language-based Nintendo/SGI RSP COP2 emulator.
    Written by Ryan Holtz

***************************************************************************/

#include "emu.h"
#include "rspcp2d.h"

#include "rsp_dasm.h"

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

#include "rspdefs.h"


/***************************************************************************
    Helpful Defines
***************************************************************************/

#define VDREG   ((op >> 6) & 0x1f)
#define VS1REG  ((op >> 11) & 0x1f)
#define VS2REG  ((op >> 16) & 0x1f)
#define EL      ((op >> 21) & 0xf)

#define RSVAL   (m_rsp.m_rsp_state->r[RSREG])
#define RTVAL   (m_rsp.m_rsp_state->r[RTREG])
#define RDVAL   (m_rsp.m_rsp_state->r[RDREG])

#define VREG_B(reg, offset)         m_v[(reg)].b[(offset)^1]
#define W_VREG_S(reg, offset)       m_v[(reg)].s[(offset)]
#define VREG_S(reg, offset)         (int16_t)m_v[(reg)].s[(offset)]

#define VEC_EL_2(x,z)               (vector_elements_2[(x)][(z)])

#define ACCUM(x)        m_accum[x].q

#define CARRY       0
#define COMPARE     1
#define CLIP1       2
#define ZERO        3
#define CLIP2       4

#define ACCUM_H(x)           (uint16_t)m_accum[x].w[3]
#define ACCUM_M(x)           (uint16_t)m_accum[x].w[2]
#define ACCUM_L(x)           (uint16_t)m_accum[x].w[1]
#define ACCUM_LL(x)          (uint16_t)m_accum[x].w[0]
#define ACCUM(x)             m_accum[x].q

#define SET_ACCUM_H(v, x)       m_accum[x].w[3] = v;
#define SET_ACCUM_M(v, x)       m_accum[x].w[2] = v;
#define SET_ACCUM_L(v, x)       m_accum[x].w[1] = v;
#define SET_ACCUM_LL(v, x)      m_accum[x].w[0] = v;
#define SET_ACCUM(v, x)         m_accum[x].q = v;

#define GET_VS1(out, i)         out = VREG_S(vs1reg, i)
#define GET_VS2(out, i)         out = VREG_S(vs2reg, VEC_EL_2(el, i))

#define CARRY_FLAG(x)          (m_vflag[CARRY][x & 7] != 0 ? 0xffff : 0)
#define COMPARE_FLAG(x)        (m_vflag[COMPARE][x & 7] != 0 ? 0xffff : 0)
#define CLIP1_FLAG(x)          (m_vflag[CLIP1][x & 7] != 0 ? 0xffff : 0)
#define ZERO_FLAG(x)           (m_vflag[ZERO][x & 7] != 0 ? 0xffff : 0)
#define CLIP2_FLAG(x)          (m_vflag[CLIP2][x & 7] != 0 ? 0xffff : 0)

#define CLEAR_CARRY_FLAGS()         { memset(m_vflag[CARRY], 0, 16); }
#define CLEAR_COMPARE_FLAGS()       { memset(m_vflag[COMPARE], 0, 16); }
#define CLEAR_CLIP1_FLAGS()         { memset(m_vflag[CLIP1], 0, 16); }
#define CLEAR_ZERO_FLAGS()          { memset(m_vflag[ZERO], 0, 16); }
#define CLEAR_CLIP2_FLAGS()         { memset(m_vflag[CLIP2], 0, 16); }

#define SET_CARRY_FLAG(x)           { m_vflag[CARRY][x & 7] = 0xffff; }
#define SET_COMPARE_FLAG(x)         { m_vflag[COMPARE][x & 7] = 0xffff; }
#define SET_CLIP1_FLAG(x)           { m_vflag[CLIP1][x & 7] = 0xffff; }
#define SET_ZERO_FLAG(x)            { m_vflag[ZERO][x & 7] = 0xffff; }
#define SET_CLIP2_FLAG(x)           { m_vflag[CLIP2][x & 7] = 0xffff; }

#define CLEAR_CARRY_FLAG(x)         { m_vflag[CARRY][x & 7] = 0; }
#define CLEAR_COMPARE_FLAG(x)       { m_vflag[COMPARE][x & 7] = 0; }
#define CLEAR_CLIP1_FLAG(x)         { m_vflag[CLIP1][x & 7] = 0; }
#define CLEAR_ZERO_FLAG(x)          { m_vflag[ZERO][x & 7] = 0; }
#define CLEAR_CLIP2_FLAG(x)         { m_vflag[CLIP2][x & 7] = 0; }

#define CACHE_VALUES() \
	const int op = m_rspcop2_state->op;    \
	const int vdreg = VDREG;    \
	const int vs1reg = VS1REG;  \
	const int vs2reg = VS2REG;  \
	const int el = EL;

#define WRITEBACK_RESULT() { \
		W_VREG_S(vdreg, 0) = m_vres[0];   \
		W_VREG_S(vdreg, 1) = m_vres[1];   \
		W_VREG_S(vdreg, 2) = m_vres[2];   \
		W_VREG_S(vdreg, 3) = m_vres[3];   \
		W_VREG_S(vdreg, 4) = m_vres[4];   \
		W_VREG_S(vdreg, 5) = m_vres[5];   \
		W_VREG_S(vdreg, 6) = m_vres[6];   \
		W_VREG_S(vdreg, 7) = m_vres[7];   \
}

static const int vector_elements_2[16][8] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     // none
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     // ???
	{ 0, 0, 2, 2, 4, 4, 6, 6 },     // 0q
	{ 1, 1, 3, 3, 5, 5, 7, 7 },     // 1q
	{ 0, 0, 0, 0, 4, 4, 4, 4 },     // 0h
	{ 1, 1, 1, 1, 5, 5, 5, 5 },     // 1h
	{ 2, 2, 2, 2, 6, 6, 6, 6 },     // 2h
	{ 3, 3, 3, 3, 7, 7, 7, 7 },     // 3h
	{ 0, 0, 0, 0, 0, 0, 0, 0 },     // 0
	{ 1, 1, 1, 1, 1, 1, 1, 1 },     // 1
	{ 2, 2, 2, 2, 2, 2, 2, 2 },     // 2
	{ 3, 3, 3, 3, 3, 3, 3, 3 },     // 3
	{ 4, 4, 4, 4, 4, 4, 4, 4 },     // 4
	{ 5, 5, 5, 5, 5, 5, 5, 5 },     // 5
	{ 6, 6, 6, 6, 6, 6, 6, 6 },     // 6
	{ 7, 7, 7, 7, 7, 7, 7, 7 },     // 7
};

void rsp_device::cop2_drc::cfunc_unimplemented_opcode()
{
	const uint32_t ppc = m_rsp.m_ppc;
	if ((m_machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		rsp_disassembler rspd;
		std::ostringstream stream;
		rspd.dasm_one(stream, ppc, m_rspcop2_state->op);
		const std::string stream_string = stream.str();
		osd_printf_debug("%08X: %s\n", ppc, stream_string);
	}
	fatalerror("RSP: unknown opcode %02X (%08X) at %08X\n", m_rspcop2_state->op >> 26, m_rspcop2_state->op, ppc);
}

void rsp_device::cop2_drc::state_string_export(const int index, std::string &str) const
{
	switch (index)
	{
		case RSP_V0:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 0, 0), (uint16_t)VREG_S( 0, 1), (uint16_t)VREG_S( 0, 2), (uint16_t)VREG_S( 0, 3), (uint16_t)VREG_S( 0, 4), (uint16_t)VREG_S( 0, 5), (uint16_t)VREG_S( 0, 6), (uint16_t)VREG_S( 0, 7));
			break;
		case RSP_V1:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 1, 0), (uint16_t)VREG_S( 1, 1), (uint16_t)VREG_S( 1, 2), (uint16_t)VREG_S( 1, 3), (uint16_t)VREG_S( 1, 4), (uint16_t)VREG_S( 1, 5), (uint16_t)VREG_S( 1, 6), (uint16_t)VREG_S( 1, 7));
			break;
		case RSP_V2:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 2, 0), (uint16_t)VREG_S( 2, 1), (uint16_t)VREG_S( 2, 2), (uint16_t)VREG_S( 2, 3), (uint16_t)VREG_S( 2, 4), (uint16_t)VREG_S( 2, 5), (uint16_t)VREG_S( 2, 6), (uint16_t)VREG_S( 2, 7));
			break;
		case RSP_V3:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 3, 0), (uint16_t)VREG_S( 3, 1), (uint16_t)VREG_S( 3, 2), (uint16_t)VREG_S( 3, 3), (uint16_t)VREG_S( 3, 4), (uint16_t)VREG_S( 3, 5), (uint16_t)VREG_S( 3, 6), (uint16_t)VREG_S( 3, 7));
			break;
		case RSP_V4:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 4, 0), (uint16_t)VREG_S( 4, 1), (uint16_t)VREG_S( 4, 2), (uint16_t)VREG_S( 4, 3), (uint16_t)VREG_S( 4, 4), (uint16_t)VREG_S( 4, 5), (uint16_t)VREG_S( 4, 6), (uint16_t)VREG_S( 4, 7));
			break;
		case RSP_V5:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 5, 0), (uint16_t)VREG_S( 5, 1), (uint16_t)VREG_S( 5, 2), (uint16_t)VREG_S( 5, 3), (uint16_t)VREG_S( 5, 4), (uint16_t)VREG_S( 5, 5), (uint16_t)VREG_S( 5, 6), (uint16_t)VREG_S( 5, 7));
			break;
		case RSP_V6:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 6, 0), (uint16_t)VREG_S( 6, 1), (uint16_t)VREG_S( 6, 2), (uint16_t)VREG_S( 6, 3), (uint16_t)VREG_S( 6, 4), (uint16_t)VREG_S( 6, 5), (uint16_t)VREG_S( 6, 6), (uint16_t)VREG_S( 6, 7));
			break;
		case RSP_V7:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 7, 0), (uint16_t)VREG_S( 7, 1), (uint16_t)VREG_S( 7, 2), (uint16_t)VREG_S( 7, 3), (uint16_t)VREG_S( 7, 4), (uint16_t)VREG_S( 7, 5), (uint16_t)VREG_S( 7, 6), (uint16_t)VREG_S( 7, 7));
			break;
		case RSP_V8:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 8, 0), (uint16_t)VREG_S( 8, 1), (uint16_t)VREG_S( 8, 2), (uint16_t)VREG_S( 8, 3), (uint16_t)VREG_S( 8, 4), (uint16_t)VREG_S( 8, 5), (uint16_t)VREG_S( 8, 6), (uint16_t)VREG_S( 8, 7));
			break;
		case RSP_V9:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S( 9, 0), (uint16_t)VREG_S( 9, 1), (uint16_t)VREG_S( 9, 2), (uint16_t)VREG_S( 9, 3), (uint16_t)VREG_S( 9, 4), (uint16_t)VREG_S( 9, 5), (uint16_t)VREG_S( 9, 6), (uint16_t)VREG_S( 9, 7));
			break;
		case RSP_V10:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(10, 0), (uint16_t)VREG_S(10, 1), (uint16_t)VREG_S(10, 2), (uint16_t)VREG_S(10, 3), (uint16_t)VREG_S(10, 4), (uint16_t)VREG_S(10, 5), (uint16_t)VREG_S(10, 6), (uint16_t)VREG_S(10, 7));
			break;
		case RSP_V11:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(11, 0), (uint16_t)VREG_S(11, 1), (uint16_t)VREG_S(11, 2), (uint16_t)VREG_S(11, 3), (uint16_t)VREG_S(11, 4), (uint16_t)VREG_S(11, 5), (uint16_t)VREG_S(11, 6), (uint16_t)VREG_S(11, 7));
			break;
		case RSP_V12:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(12, 0), (uint16_t)VREG_S(12, 1), (uint16_t)VREG_S(12, 2), (uint16_t)VREG_S(12, 3), (uint16_t)VREG_S(12, 4), (uint16_t)VREG_S(12, 5), (uint16_t)VREG_S(12, 6), (uint16_t)VREG_S(12, 7));
			break;
		case RSP_V13:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(13, 0), (uint16_t)VREG_S(13, 1), (uint16_t)VREG_S(13, 2), (uint16_t)VREG_S(13, 3), (uint16_t)VREG_S(13, 4), (uint16_t)VREG_S(13, 5), (uint16_t)VREG_S(13, 6), (uint16_t)VREG_S(13, 7));
			break;
		case RSP_V14:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(14, 0), (uint16_t)VREG_S(14, 1), (uint16_t)VREG_S(14, 2), (uint16_t)VREG_S(14, 3), (uint16_t)VREG_S(14, 4), (uint16_t)VREG_S(14, 5), (uint16_t)VREG_S(14, 6), (uint16_t)VREG_S(14, 7));
			break;
		case RSP_V15:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(15, 0), (uint16_t)VREG_S(15, 1), (uint16_t)VREG_S(15, 2), (uint16_t)VREG_S(15, 3), (uint16_t)VREG_S(15, 4), (uint16_t)VREG_S(15, 5), (uint16_t)VREG_S(15, 6), (uint16_t)VREG_S(15, 7));
			break;
		case RSP_V16:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(16, 0), (uint16_t)VREG_S(16, 1), (uint16_t)VREG_S(16, 2), (uint16_t)VREG_S(16, 3), (uint16_t)VREG_S(16, 4), (uint16_t)VREG_S(16, 5), (uint16_t)VREG_S(16, 6), (uint16_t)VREG_S(16, 7));
			break;
		case RSP_V17:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(17, 0), (uint16_t)VREG_S(17, 1), (uint16_t)VREG_S(17, 2), (uint16_t)VREG_S(17, 3), (uint16_t)VREG_S(17, 4), (uint16_t)VREG_S(17, 5), (uint16_t)VREG_S(17, 6), (uint16_t)VREG_S(17, 7));
			break;
		case RSP_V18:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(18, 0), (uint16_t)VREG_S(18, 1), (uint16_t)VREG_S(18, 2), (uint16_t)VREG_S(18, 3), (uint16_t)VREG_S(18, 4), (uint16_t)VREG_S(18, 5), (uint16_t)VREG_S(18, 6), (uint16_t)VREG_S(18, 7));
			break;
		case RSP_V19:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(19, 0), (uint16_t)VREG_S(19, 1), (uint16_t)VREG_S(19, 2), (uint16_t)VREG_S(19, 3), (uint16_t)VREG_S(19, 4), (uint16_t)VREG_S(19, 5), (uint16_t)VREG_S(19, 6), (uint16_t)VREG_S(19, 7));
			break;
		case RSP_V20:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(20, 0), (uint16_t)VREG_S(20, 1), (uint16_t)VREG_S(20, 2), (uint16_t)VREG_S(20, 3), (uint16_t)VREG_S(20, 4), (uint16_t)VREG_S(20, 5), (uint16_t)VREG_S(20, 6), (uint16_t)VREG_S(20, 7));
			break;
		case RSP_V21:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(21, 0), (uint16_t)VREG_S(21, 1), (uint16_t)VREG_S(21, 2), (uint16_t)VREG_S(21, 3), (uint16_t)VREG_S(21, 4), (uint16_t)VREG_S(21, 5), (uint16_t)VREG_S(21, 6), (uint16_t)VREG_S(21, 7));
			break;
		case RSP_V22:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(22, 0), (uint16_t)VREG_S(22, 1), (uint16_t)VREG_S(22, 2), (uint16_t)VREG_S(22, 3), (uint16_t)VREG_S(22, 4), (uint16_t)VREG_S(22, 5), (uint16_t)VREG_S(22, 6), (uint16_t)VREG_S(22, 7));
			break;
		case RSP_V23:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(23, 0), (uint16_t)VREG_S(23, 1), (uint16_t)VREG_S(23, 2), (uint16_t)VREG_S(23, 3), (uint16_t)VREG_S(23, 4), (uint16_t)VREG_S(23, 5), (uint16_t)VREG_S(23, 6), (uint16_t)VREG_S(23, 7));
			break;
		case RSP_V24:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(24, 0), (uint16_t)VREG_S(24, 1), (uint16_t)VREG_S(24, 2), (uint16_t)VREG_S(24, 3), (uint16_t)VREG_S(24, 4), (uint16_t)VREG_S(24, 5), (uint16_t)VREG_S(24, 6), (uint16_t)VREG_S(24, 7));
			break;
		case RSP_V25:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(25, 0), (uint16_t)VREG_S(25, 1), (uint16_t)VREG_S(25, 2), (uint16_t)VREG_S(25, 3), (uint16_t)VREG_S(25, 4), (uint16_t)VREG_S(25, 5), (uint16_t)VREG_S(25, 6), (uint16_t)VREG_S(25, 7));
			break;
		case RSP_V26:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(26, 0), (uint16_t)VREG_S(26, 1), (uint16_t)VREG_S(26, 2), (uint16_t)VREG_S(26, 3), (uint16_t)VREG_S(26, 4), (uint16_t)VREG_S(26, 5), (uint16_t)VREG_S(26, 6), (uint16_t)VREG_S(26, 7));
			break;
		case RSP_V27:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(27, 0), (uint16_t)VREG_S(27, 1), (uint16_t)VREG_S(27, 2), (uint16_t)VREG_S(27, 3), (uint16_t)VREG_S(27, 4), (uint16_t)VREG_S(27, 5), (uint16_t)VREG_S(27, 6), (uint16_t)VREG_S(27, 7));
			break;
		case RSP_V28:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(28, 0), (uint16_t)VREG_S(28, 1), (uint16_t)VREG_S(28, 2), (uint16_t)VREG_S(28, 3), (uint16_t)VREG_S(28, 4), (uint16_t)VREG_S(28, 5), (uint16_t)VREG_S(28, 6), (uint16_t)VREG_S(28, 7));
			break;
		case RSP_V29:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(29, 0), (uint16_t)VREG_S(29, 1), (uint16_t)VREG_S(29, 2), (uint16_t)VREG_S(29, 3), (uint16_t)VREG_S(29, 4), (uint16_t)VREG_S(29, 5), (uint16_t)VREG_S(29, 6), (uint16_t)VREG_S(29, 7));
			break;
		case RSP_V30:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(30, 0), (uint16_t)VREG_S(30, 1), (uint16_t)VREG_S(30, 2), (uint16_t)VREG_S(30, 3), (uint16_t)VREG_S(30, 4), (uint16_t)VREG_S(30, 5), (uint16_t)VREG_S(30, 6), (uint16_t)VREG_S(30, 7));
			break;
		case RSP_V31:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (uint16_t)VREG_S(31, 0), (uint16_t)VREG_S(31, 1), (uint16_t)VREG_S(31, 2), (uint16_t)VREG_S(31, 3), (uint16_t)VREG_S(31, 4), (uint16_t)VREG_S(31, 5), (uint16_t)VREG_S(31, 6), (uint16_t)VREG_S(31, 7));
			break;
	}
}


/***************************************************************************
    Vector Load Instructions
***************************************************************************/

// LBV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00000 | IIII | Offset |
// --------------------------------------------------
//
// Load 1 byte to vector byte index

void rsp_device::cop2_drc::lbv()
{
	uint32_t op = m_rspcop2_state->op;

	uint32_t ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;
	VREG_B(dest, index) = m_rsp.DM_READ8(ea);
}


// LSV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00001 | IIII | Offset |
// --------------------------------------------------
//
// Loads 2 bytes starting from vector byte index

void rsp_device::cop2_drc::lsv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);
	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = m_rsp.DM_READ8(ea);
		ea++;
	}
}


// LLV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00010 | IIII | Offset |
// --------------------------------------------------
//
// Loads 4 bytes starting from vector byte index

void rsp_device::cop2_drc::llv()
{
	uint32_t op = m_rspcop2_state->op;
	uint32_t ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;

	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = m_rsp.DM_READ8(ea);
		ea++;
	}
}


// LDV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00011 | IIII | Offset |
// --------------------------------------------------
//
// Loads 8 bytes starting from vector byte index

void rsp_device::cop2_drc::ldv()
{
	uint32_t op = m_rspcop2_state->op;
	uint32_t ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;

	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = m_rsp.DM_READ8(ea);
		ea++;
	}
}


// LQV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00100 | IIII | Offset |
// --------------------------------------------------
//
// Loads up to 16 bytes starting from vector byte index

void rsp_device::cop2_drc::lqv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = 16 - (ea & 0xf);
	if (end > 16) end = 16;

	for (int i = 0; i < end; i++)
	{
		VREG_B(dest, i) = m_rsp.DM_READ8(ea);
		ea++;
	}
}


// LRV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00101 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from right side until 16-byte boundary

void rsp_device::cop2_drc::lrv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	index = 16 - ((ea & 0xf) - index);
	ea &= ~0xf;

	for (int i = index; i < 16; i++)
	{
		VREG_B(dest, i) = m_rsp.DM_READ8(ea);
		ea++;
	}
}


// LPV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00110 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the upper 8 bits of each element

void rsp_device::cop2_drc::lpv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		W_VREG_S(dest, i) = m_rsp.DM_READ8(ea + (((16-index) + i) & 0xf)) << 8;
	}
}


// LUV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00111 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of each element

void rsp_device::cop2_drc::luv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		W_VREG_S(dest, i) = m_rsp.DM_READ8(ea + (((16-index) + i) & 0xf)) << 7;
	}
}


// LHV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01000 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of each element, with 2-byte stride

void rsp_device::cop2_drc::lhv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	for (int i = 0; i < 8; i++)
	{
		W_VREG_S(dest, i) = m_rsp.DM_READ8(ea + (((16-index) + (i<<1)) & 0xf)) << 7;
	}
}


// LFV
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01001 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of upper or lower quad, with 4-byte stride

void rsp_device::cop2_drc::lfv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	// not sure what happens if 16-byte boundary is crossed...

	int end = (index >> 1) + 4;

	for (int i = index >> 1; i < end; i++)
	{
		W_VREG_S(dest, i) = m_rsp.DM_READ8(ea) << 7;
		ea += 4;
	}
}


// LWV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01010 | IIII | Offset |
// --------------------------------------------------
//
// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
// after byte index 15

void rsp_device::cop2_drc::lwv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = (16 - index) + 16;

	for (int i = (16 - index); i < end; i++)
	{
		VREG_B(dest, i & 0xf) = m_rsp.DM_READ8(ea);
		ea += 4;
	}
}


// LTV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01011 | IIII | Offset |
// --------------------------------------------------
//
// Loads one element to maximum of 8 vectors, while incrementing element index

void rsp_device::cop2_drc::ltv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);

	// FIXME: has a small problem with odd indices

	int vs = dest;
	int ve = dest + 8;
	if (ve > 32)
	{
		ve = 32;
	}

	int element;

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	ea = ((ea + 8) & ~0xf) + (index & 1);
	for (int i = vs; i < ve; i++)
	{
		element = (8 - (index >> 1) + (i - vs)) << 1;
		VREG_B(i, (element & 0xf)) = m_rsp.DM_READ8(ea);
		VREG_B(i, ((element + 1) & 0xf)) = m_rsp.DM_READ8(ea + 1);
		ea += 2;
	}
}


bool rsp_device::cop2_drc::generate_lwc2(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* LBV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lbv, this);
			return true;

		case 0x01:      /* LSV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lsv, this);
			return true;

		case 0x02:      /* LLV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_llv, this);
			return true;

		case 0x03:      /* LDV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_ldv, this);
			return true;

		case 0x04:      /* LQV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lqv, this);
			return true;

		case 0x05:      /* LRV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lrv, this);
			return true;

		case 0x06:      /* LPV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lpv, this);
			return true;

		case 0x07:      /* LUV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_luv, this);
			return true;

		case 0x08:      /* LHV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lhv, this);
			return true;

		case 0x09:      /* LFV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lfv, this);
			return true;

		case 0x0a:      /* LWV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_lwv, this);
			return true;

		case 0x0b:      /* LTV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [m_rspcop2_state->op],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_ltv, this);
			return true;

		default:
			return false;
	}
}


/***************************************************************************
    Vector Store Instructions
***************************************************************************/

// SBV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00000 | IIII | Offset |
// --------------------------------------------------
//
// Stores 1 byte from vector byte index

void rsp_device::cop2_drc::sbv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;
	m_rsp.DM_WRITE8(ea, VREG_B(dest, index));
}


// SSV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00001 | IIII | Offset |
// --------------------------------------------------
//
// Stores 2 bytes starting from vector byte index

void rsp_device::cop2_drc::ssv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);

	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		m_rsp.DM_WRITE8(ea, VREG_B(dest, i));
		ea++;
	}
}


// SLV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00010 | IIII | Offset |
// --------------------------------------------------
//
// Stores 4 bytes starting from vector byte index

void rsp_device::cop2_drc::slv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;
	for (int i = index; i < end; i++)
	{
		m_rsp.DM_WRITE8(ea, VREG_B(dest, i));
		ea++;
	}
}


// SDV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00011 | IIII | Offset |
// --------------------------------------------------
//
// Stores 8 bytes starting from vector byte index

void rsp_device::cop2_drc::sdv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;
	for (int i = index; i < end; i++)
	{
		m_rsp.DM_WRITE8(ea, VREG_B(dest, i));
		ea++;
	}
}


// SQV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00100 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from vector byte index until 16-byte boundary

void rsp_device::cop2_drc::sqv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = index + (16 - (ea & 0xf));
	for (int i=index; i < end; i++)
	{
		m_rsp.DM_WRITE8(ea, VREG_B(dest, i & 0xf));
		ea++;
	}
}


// SRV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00101 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from right side until 16-byte boundary

void rsp_device::cop2_drc::srv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = index + (ea & 0xf);
	int o = (16 - (ea & 0xf)) & 0xf;
	ea &= ~0xf;

	for (int i = index; i < end; i++)
	{
		m_rsp.DM_WRITE8(ea, VREG_B(dest, ((i + o) & 0xf)));
		ea++;
	}
}


// SPV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00110 | IIII | Offset |
// --------------------------------------------------
//
// Stores upper 8 bits of each element

void rsp_device::cop2_drc::spv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			m_rsp.DM_WRITE8(ea, VREG_B(dest, (i & 0xf) << 1));
		}
		else
		{
			m_rsp.DM_WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
		}
		ea++;
	}
}


// SUV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00111 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of each element

void rsp_device::cop2_drc::suv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			m_rsp.DM_WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
		}
		else
		{
			m_rsp.DM_WRITE8(ea, VREG_B(dest, ((i & 0x7) << 1)));
		}
		ea++;
	}
}


// SHV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01000 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of each element, with 2-byte stride

void rsp_device::cop2_drc::shv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	for (int i=0; i < 8; i++)
	{
		int element = index + (i << 1);
		uint8_t d = (VREG_B(dest, (element & 0xf)) << 1) |
					(VREG_B(dest, ((element + 1) & 0xf)) >> 7);
		m_rsp.DM_WRITE8(ea, d);
		ea += 2;
	}
}


// SFV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01001 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of upper or lower quad, with 4-byte stride

void rsp_device::cop2_drc::sfv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = (index >> 1) + 4;

	for (int i = index>>1; i < end; i++)
	{
		m_rsp.DM_WRITE8(ea + (eaoffset & 0xf), VREG_S(dest, i) >> 7);
		eaoffset += 4;
	}
}


// SWV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01010 | IIII | Offset |
// --------------------------------------------------
//
// Stores the full 128-bit vector starting from vector byte index and wrapping to index 0
// after byte index 15

void rsp_device::cop2_drc::swv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = index + 16;
	for (int i = index; i < end; i++)
	{
		m_rsp.DM_WRITE8(ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
		eaoffset++;
	}
}


// STV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01011 | IIII | Offset |
// --------------------------------------------------
//
// Stores one element from maximum of 8 vectors, while incrementing element index

void rsp_device::cop2_drc::stv()
{
	uint32_t op = m_rspcop2_state->op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);

	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	int vs = dest;
	int ve = dest + 8;
	if (ve > 32)
	{
		ve = 32;
	}

	int element = 8 - (index >> 1);

	uint32_t ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = (ea & 0xf) + (element * 2);
	ea &= ~0xf;

	for (int i = vs; i < ve; i++)
	{
		m_rsp.DM_WRITE16(ea + (eaoffset & 0xf), VREG_S(i, element & 0x7));
		eaoffset += 2;
		element++;
	}
}

bool rsp_device::cop2_drc::generate_swc2(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* SBV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_sbv, this);
			return true;

		case 0x01:      /* SSV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_ssv, this);
			return true;

		case 0x02:      /* SLV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_slv, this);
			return true;

		case 0x03:      /* SDV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_sdv, this);
			return true;

		case 0x04:      /* SQV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_sqv, this);
			return true;

		case 0x05:      /* SRV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_srv, this);
			return true;

		case 0x06:      /* SPV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_spv, this);
			return true;

		case 0x07:      /* SUV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_suv, this);
			return true;

		case 0x08:      /* SHV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_shv, this);
			return true;

		case 0x09:      /* SFV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_sfv, this);
			return true;

		case 0x0a:      /* SWV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_swv, this);
			return true;

		case 0x0b:      /* STV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_stv, this);
			return true;

		default:
			m_rsp.unimplemented_opcode(op);
			return false;
	}

	return true;
}


/***************************************************************************
    Vector Opcodes
***************************************************************************/

// VMULF
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000000 |
// ------------------------------------------------------
//
// Multiplies signed integer by signed integer * 2

void rsp_device::cop2_drc::vmulf()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		if (s1 == -32768 && s2 == -32768)
		{
			// overflow
			ACCUM(i) = s64(0x0000800080000000U);
			m_vres[i] = 0x7fff;
		}
		else
		{
			ACCUM(i) = (int64_t)(s1 * s2 * 2 + 0x8000) << 16; // rounding?
			m_vres[i] = ACCUM_M(i);
		}
	}
	WRITEBACK_RESULT();
}


// VMULU
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000001 |
// ------------------------------------------------------
//

void rsp_device::cop2_drc::vmulu()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		int64_t r = s1 * s2 * 2 + 0x8000; // rounding?

		ACCUM(i) = r << 16;

		if (r < 0)
		{
			m_vres[i] = 0;
		}
		else if (((int16_t)(ACCUM_H(i)) ^ (int16_t)(ACCUM_M(i))) < 0)
		{
			m_vres[i] = -1;
		}
		else
		{
			m_vres[i] = ACCUM_M(i);
		}
	}
	WRITEBACK_RESULT();
}


// VMUDL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
// ------------------------------------------------------
//
// Multiplies signed integer by unsigned fraction
// The result is added into accumulator
// The middle slice of accumulator is stored into destination element

void rsp_device::cop2_drc::vmudl()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		uint32_t s1 = (uint32_t)(uint16_t)w1;
		uint32_t s2 = (uint32_t)(uint16_t)w2;

		ACCUM(i) = s1 * s2;

		m_vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
}


// VMUDM
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000101 |
// ------------------------------------------------------
//
// Multiplies signed integer by unsigned fraction
// The result is stored into accumulator
// The middle slice of accumulator is stored into destination element

void rsp_device::cop2_drc::vmudm()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (uint16_t)w2;

		ACCUM(i) = (int64_t)(s1 * s2) << 16;

		m_vres[i] = ACCUM_M(i);
	}
	WRITEBACK_RESULT();
}


// VMUDN
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000110 |
// ------------------------------------------------------
//
// Multiplies unsigned fraction by signed integer
// The result is stored into accumulator
// The low slice of accumulator is stored into destination element

void rsp_device::cop2_drc::vmudn()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (uint16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		int32_t r = s1 * s2;

		ACCUM(i) = (int64_t)(s1 * s2) << 16;

		m_vres[i] = (uint16_t)(r);
	}
	WRITEBACK_RESULT();
}


// VMUDH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000111 |
// ------------------------------------------------------
//
// Multiplies signed integer by signed integer
// The result is stored into highest 32 bits of accumulator, the low slice is zero
// The highest 32 bits of accumulator is saturated into destination element

void rsp_device::cop2_drc::vmudh()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		int32_t r = s1 * s2;

		ACCUM(i) = (int64_t)r << 32;

		if (r < -32768) r = -32768;
		if (r >  32767) r = 32767;
		m_vres[i] = (int16_t)(r);
	}
	WRITEBACK_RESULT();
}


// VMACF
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001000 |
// ------------------------------------------------------
//

void rsp_device::cop2_drc::vmacf()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		ACCUM(i) += (int64_t)(s1 * s2 * 2) << 16;

		m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
	}
	WRITEBACK_RESULT();
}


// VMACU
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001001 |
// ------------------------------------------------------
//

void rsp_device::cop2_drc::vmacu()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		ACCUM(i) += (int64_t)(s1 * s2 * 2) << 16;

		if ((int16_t)ACCUM_H(i) < 0)
		{
			m_vres[i] = 0;
		}
		else
		{
			if (ACCUM_H(i) != 0)
			{
				m_vres[i] = (int16_t)0xffff;
			}
			else
			{
				if ((int16_t)ACCUM_M(i) < 0)
				{
					m_vres[i] = (int16_t)0xffff;
				}
				else
				{
					m_vres[i] = ACCUM_M(i);
				}
			}
		}
	}
	WRITEBACK_RESULT();
}


// VMADL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001100 |
// ------------------------------------------------------
//
// Multiplies unsigned fraction by unsigned fraction
// Adds the higher 16 bits of the 32-bit result to accumulator
// The low slice of accumulator is stored into destination element

void rsp_device::cop2_drc::vmadl()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		uint32_t s1 = w1;
		uint32_t s2 = w2;

		ACCUM(i) += (s1 * s2) & 0xffff0000;

		m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	WRITEBACK_RESULT();
}


// VMADM
//

void rsp_device::cop2_drc::vmadm()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		uint32_t s1 = (int32_t)(int16_t)w1;
		uint32_t s2 = (uint16_t)w2;

		ACCUM(i) += (int64_t)(int32_t)(s1 * s2) << 16;

		m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
	}
	WRITEBACK_RESULT();
}


// VMADN
//

void rsp_device::cop2_drc::vmadn()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (uint16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		ACCUM(i) += (int64_t)(s1 * s2) << 16;

		m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	WRITEBACK_RESULT();
}


// VMADH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001111 |
// ------------------------------------------------------
//
// Multiplies signed integer by signed integer
// The result is added into highest 32 bits of accumulator, the low slice is zero
// The highest 32 bits of accumulator is saturated into destination element

void rsp_device::cop2_drc::vmadh()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		int16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;

		ACCUM(i) += (int64_t)(s1 * s2) << 32;

		m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
	}
	WRITEBACK_RESULT();
}


// VADD
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010000 |
// ------------------------------------------------------
//
// Adds two vector registers and carry flag, the result is saturated to 32767

void rsp_device::cop2_drc::vadd()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		int16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;
		int32_t r = s1 + s2 + (((CARRY_FLAG(i)) != 0) ? 1 : 0);

		SET_ACCUM_L((int16_t)(r), i);

		if (r > 32767) r = 32767;
		if (r < -32768) r = -32768;
		m_vres[i] = (int16_t)(r);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}


// VSUB
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010001 |
// ------------------------------------------------------
//
// Subtracts two vector registers and carry flag, the result is saturated to -32768
// TODO: check VS2REG == VDREG

void rsp_device::cop2_drc::vsub()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		int16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (int32_t)(int16_t)w1;
		int32_t s2 = (int32_t)(int16_t)w2;
		int32_t r = s1 - s2 - (((CARRY_FLAG(i)) != 0) ? 1 : 0);

		SET_ACCUM_L((int16_t)(r), i);

		if (r > 32767) r = 32767;
		if (r < -32768) r = -32768;

		m_vres[i] = (int16_t)(r);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}


// VABS
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010011 |
// ------------------------------------------------------
//
// Changes the sign of source register 2 if source register 1 is negative and stores the result to destination register

void rsp_device::cop2_drc::vabs()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if (s1 < 0)
		{
			if (s2 == -32768)
			{
				m_vres[i] = 32767;
			}
			else
			{
				m_vres[i] = -s2;
			}
		}
		else if (s1 > 0)
		{
			m_vres[i] = s2;
		}
		else
		{
			m_vres[i] = 0;
		}

		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VADDC
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010100 |
// ------------------------------------------------------
//
// Adds two vector registers, the carry out is stored into carry register
// TODO: check VS2REG = VDREG

void rsp_device::cop2_drc::vaddc()
{
	CACHE_VALUES();

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		int16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (uint32_t)(uint16_t)w1;
		int32_t s2 = (uint32_t)(uint16_t)w2;
		int32_t r = s1 + s2;

		m_vres[i] = (int16_t)(r);
		SET_ACCUM_L((int16_t)r, i);

		if (r & 0xffff0000)
		{
			SET_CARRY_FLAG(i);
		}
	}
	WRITEBACK_RESULT();
}


// VSUBC
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010101 |
// ------------------------------------------------------
//
// Subtracts two vector registers, the carry out is stored into carry register
// TODO: check VS2REG = VDREG

void rsp_device::cop2_drc::vsubc()
{
	CACHE_VALUES();

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		int16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		int32_t s1 = (uint32_t)(uint16_t)w1;
		int32_t s2 = (uint32_t)(uint16_t)w2;
		int32_t r = s1 - s2;

		m_vres[i] = (int16_t)(r);
		SET_ACCUM_L((uint16_t)r, i);

		if ((uint16_t)(r) != 0)
		{
			SET_ZERO_FLAG(i);
		}
		if (r & 0xffff0000)
		{
			SET_CARRY_FLAG(i);
		}
	}
	WRITEBACK_RESULT();
}


// VADDB
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010110 |
// ------------------------------------------------------
//
// Adds two vector registers bytewise with rounding

void rsp_device::cop2_drc::vaddb()
{
	CACHE_VALUES();
	const int round = (el == 0) ? 0 : (1 << (el - 1));

	for (int i = 0; i < 8; i++)
	{
		uint16_t w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);

		uint8_t hb1 = w1 >> 8;
		uint8_t lb1 = w1 & 0xff;
		uint8_t hb2 = w2 >> 8;
		uint8_t lb2 = w2 & 0xff;

		uint16_t hs = hb1 + hb2 + round;
		uint16_t ls = lb1 + lb2 + round;

		SET_ACCUM_L((hs << 8) | ls, i);

		hs >>= EL;
		if (hs > 255)
		{
			hs = 255;
		}

		ls >>= EL;
		if (ls > 255)
		{
			ls = 255;
		}

		m_vres[i] = 0; // VD writeback disabled on production hardware
		// m_vres[i] = (hs << 8) | ls;
	}
	WRITEBACK_RESULT();
}


// VSAW
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 011101 |
// ------------------------------------------------------
//
// Stores high, middle or low slice of accumulator to destination vector

void rsp_device::cop2_drc::vsaw()
{
	const int op = m_rspcop2_state->op;
	const int vdreg = VDREG;
	const int el = EL;

	switch (el)
	{
		case 0x08:      // VSAWH
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(vdreg, i) = ACCUM_H(i);
			}
			break;
		case 0x09:      // VSAWM
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(vdreg, i) = ACCUM_M(i);
			}
			break;
		case 0x0a:      // VSAWL
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(vdreg, i) = ACCUM_L(i);
			}
			break;
		default:        // Unsupported
		{
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(vdreg, i) = 0;
			}
		}
	}
}


// VLT
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100000 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are less than VS2
// Moves the element in VS2 to destination vector

void rsp_device::cop2_drc::vlt()
{
	CACHE_VALUES();

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if (s1 < s2)
		{
			SET_COMPARE_FLAG(i);
		}
		else if (s1 == s2)
		{
			if (ZERO_FLAG(i) != 0 && CARRY_FLAG(i) != 0)
			{
				SET_COMPARE_FLAG(i);
			}
		}

		if (COMPARE_FLAG(i) != 0)
		{
			m_vres[i] = s1;
		}
		else
		{
			m_vres[i] = s2;
		}

		SET_ACCUM_L(m_vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}


// VEQ
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100001 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are equal with VS2
// Moves the element in VS2 to destination vector

void rsp_device::cop2_drc::veq()
{
	CACHE_VALUES();

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if ((s1 == s2) && ZERO_FLAG(i) == 0)
		{
			SET_COMPARE_FLAG(i);
			m_vres[i] = s1;
		}
		else
		{
			m_vres[i] = s2;
		}

		SET_ACCUM_L(m_vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}


// VNE
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100010 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are not equal with VS2
// Moves the element in VS2 to destination vector

void rsp_device::cop2_drc::vne()
{
	CACHE_VALUES();

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if (s1 != s2 || ZERO_FLAG(i) != 0)
		{
			SET_COMPARE_FLAG(i);
			m_vres[i] = s1;
		}
		else
		{
			m_vres[i] = s2;
		}

		SET_ACCUM_L(m_vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}


// VGE
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100011 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are greater or equal with VS2
// Moves the element in VS2 to destination vector

void rsp_device::cop2_drc::vge()
{
	CACHE_VALUES();

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		if ((s1 == s2 && (ZERO_FLAG(i) == 0 || CARRY_FLAG(i) == 0)) || s1 > s2)
		{
			SET_COMPARE_FLAG(i);
			m_vres[i] = s1;
		}
		else
		{
			m_vres[i] = s2;
		}

		SET_ACCUM_L(m_vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}


// VCL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100100 |
// ------------------------------------------------------
//
// Vector clip low

void rsp_device::cop2_drc::vcl()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if (CARRY_FLAG(i) != 0)
		{
			if (ZERO_FLAG(i) != 0)
			{
				if (COMPARE_FLAG(i) != 0)
				{
					SET_ACCUM_L(-(uint16_t)s2, i);
				}
				else
				{
					SET_ACCUM_L(s1, i);
				}
			}
			else
			{
				if (CLIP1_FLAG(i) != 0)
				{
					if (((uint32_t)(uint16_t)(s1) + (uint32_t)(uint16_t)(s2)) > 0x10000)
					{
						SET_ACCUM_L(s1, i);
						CLEAR_COMPARE_FLAG(i);
					}
					else
					{
						SET_ACCUM_L(-((uint16_t)s2), i);
						SET_COMPARE_FLAG(i);
					}
				}
				else
				{
					if (((uint32_t)(uint16_t)(s1) + (uint32_t)(uint16_t)(s2)) != 0)
					{
						SET_ACCUM_L(s1, i);
						CLEAR_COMPARE_FLAG(i);
					}
					else
					{
						SET_ACCUM_L(-((uint16_t)s2), i);
						SET_COMPARE_FLAG(i);
					}
				}
			}
		}
		else
		{
			if (ZERO_FLAG(i) != 0)
			{
				if (CLIP2_FLAG(i) != 0)
				{
					SET_ACCUM_L(s2, i);
				}
				else
				{
					SET_ACCUM_L(s1, i);
				}
			}
			else
			{
				if (((int32_t)(uint16_t)s1 - (int32_t)(uint16_t)s2) >= 0)
				{
					SET_ACCUM_L(s2, i);
					SET_CLIP2_FLAG(i);
				}
				else
				{
					SET_ACCUM_L(s1, i);
					CLEAR_CLIP2_FLAG(i);
				}
			}
		}
		m_vres[i] = ACCUM_L(i);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	CLEAR_CLIP1_FLAGS();
	WRITEBACK_RESULT();
}


// VCH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100101 |
// ------------------------------------------------------
//
// Vector clip high

void rsp_device::cop2_drc::vch()
{
	CACHE_VALUES();

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();

	uint32_t vce;
	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if ((s1 ^ s2) < 0)
		{
			vce = (s1 + s2 == -1);
			SET_CARRY_FLAG(i);
			if (s2 < 0)
			{
				SET_CLIP2_FLAG(i);
			}

			if ((s1 + s2) <= 0)
			{
				SET_COMPARE_FLAG(i);
				m_vres[i] = -((uint16_t)s2);
			}
			else
			{
				m_vres[i] = s1;
			}

			if ((s1 + s2) != 0 && s1 != ~s2)
			{
				SET_ZERO_FLAG(i);
			}
		}//sign
		else
		{
			vce = 0;
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(i);
			}
			if ((s1 - s2) >= 0)
			{
				SET_CLIP2_FLAG(i);
				m_vres[i] = s2;
			}
			else
			{
				m_vres[i] = s1;
			}

			if ((s1 - s2) != 0 && s1 != ~s2)
			{
				SET_ZERO_FLAG(i);
			}
		}
		if (vce)
		{
			SET_CLIP1_FLAG(i);
		}
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VCR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100110 |
// ------------------------------------------------------
//
// Vector clip reverse

void rsp_device::cop2_drc::vcr()
{
	CACHE_VALUES();

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if ((int16_t)(s1 ^ s2) < 0)
		{
			if (s2 < 0)
			{
				SET_CLIP2_FLAG(i);
			}
			if ((s1 + s2) <= 0)
			{
				SET_ACCUM_L(~((uint16_t)s2), i);
				SET_COMPARE_FLAG(i);
			}
			else
			{
				SET_ACCUM_L(s1, i);
			}
		}
		else
		{
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(i);
			}
			if ((s1 - s2) >= 0)
			{
				SET_ACCUM_L(s2, i);
				SET_CLIP2_FLAG(i);
			}
			else
			{
				SET_ACCUM_L(s1, i);
			}
		}

		m_vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
}


// VMRG
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100111 |
// ------------------------------------------------------
//
// Merges two vectors according to compare flags

void rsp_device::cop2_drc::vmrg()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		int16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		if (COMPARE_FLAG(i) != 0)
		{
			m_vres[i] = s1;
		}
		else
		{
			m_vres[i] = s2;
		}

		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VAND
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
// ------------------------------------------------------
//
// Bitwise AND of two vector registers

void rsp_device::cop2_drc::vand()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		m_vres[i] = s1 & s2;
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VNAND
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101001 |
// ------------------------------------------------------
//
// Bitwise NOT AND of two vector registers

void rsp_device::cop2_drc::vnand()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		m_vres[i] = ~((s1 & s2));
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101010 |
// ------------------------------------------------------
//
// Bitwise OR of two vector registers

void rsp_device::cop2_drc::vor()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		m_vres[i] = s1 | s2;
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VNOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101011 |
// ------------------------------------------------------
//
// Bitwise NOT OR of two vector registers

void rsp_device::cop2_drc::vnor()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		m_vres[i] = ~(s1 | s2);
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VXOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101100 |
// ------------------------------------------------------
//
// Bitwise XOR of two vector registers

void rsp_device::cop2_drc::vxor()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		m_vres[i] = s1 ^ s2;
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VNXOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
// ------------------------------------------------------
//
// Bitwise NOT XOR of two vector registers

void rsp_device::cop2_drc::vnxor()
{
	CACHE_VALUES();

	for (int i = 0; i < 8; i++)
	{
		uint16_t s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);
		m_vres[i] = ~(s1 ^ s2);
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}


// VRCP
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110000 |
// ------------------------------------------------------
//
// Calculates reciprocal

void rsp_device::cop2_drc::vrcp()
{
	CACHE_VALUES();

	int32_t shifter = 0;
	int32_t rec = (int16_t)(VREG_S(vs2reg, el & 7));
	int32_t datainput = (rec < 0) ? (-rec) : rec;
	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		shifter = 0x10;
	}

	int32_t address = ((datainput << shifter) & 0x7fc00000) >> 22;
	int32_t fetchval = rsp_divtable[address];
	int32_t temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	m_reciprocal_res = rec;
	m_dp_allowed = 0;

	W_VREG_S(vdreg, vs1reg & 7) = (uint16_t)rec;
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(vs2reg, VEC_EL_2(el, i)), i);
	}
}


// VRCPL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110001 |
// ------------------------------------------------------
//
// Calculates reciprocal low part

void rsp_device::cop2_drc::vrcpl()
{
	CACHE_VALUES();

	int32_t shifter = 0;
	int32_t rec = (int16_t)VREG_S(vs2reg, el & 7);
	int32_t datainput = rec;

	if (m_dp_allowed)
	{
		rec = (rec & 0x0000ffff) | m_reciprocal_high;
		datainput = rec;

		if (rec < 0)
		{
			if (rec < -32768)
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
	}
	else if (datainput < 0)
	{
		datainput = -datainput;

		shifter = 0x10;
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}

	uint32_t address = (datainput << shifter) >> 22;
	int32_t fetchval = rsp_divtable[address & 0x1ff];
	int32_t temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
	temp ^= rec >> 31;

	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	m_reciprocal_res = rec;
	m_dp_allowed = 0;

	W_VREG_S(vdreg, vs1reg & 7) = (uint16_t)rec;

	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(vs2reg, VEC_EL_2(el, i)), i);
	}
}


// VRCPH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110010 |
// ------------------------------------------------------
//
// Calculates reciprocal high part

void rsp_device::cop2_drc::vrcph()
{
	CACHE_VALUES();

	m_reciprocal_high = (VREG_S(vs2reg, el & 7)) << 16;
	m_dp_allowed = 1;

	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(vs2reg, VEC_EL_2(el, i)), i);
	}

	W_VREG_S(vdreg, vs1reg & 7) = (int16_t)(m_reciprocal_res >> 16);
}


// VMOV
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110011 |
// ------------------------------------------------------
//
// Moves element from vector to destination vector

void rsp_device::cop2_drc::vmov()
{
	CACHE_VALUES();

	W_VREG_S(vdreg, vs1reg & 7) = VREG_S(vs2reg, el & 7);
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(vs2reg, VEC_EL_2(el, i)), i);
	}
}


// VRSQ
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110100 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root

void rsp_device::cop2_drc::vrsq()
{
	CACHE_VALUES();

	int32_t shifter = 0;
	int32_t rec = (int16_t)VREG_S(vs2reg, el & 7);
	int32_t datainput = (rec < 0) ? (-rec) : (rec);

	if (rec < 0)
	{
		if (rec < -32768)
		{
			datainput = ~datainput;
		}
		else
		{
			datainput = -datainput;
		}
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		shifter = 0;
	}

	int32_t address = ((datainput << shifter) & 0x7fc00000) >> 22;
	address = ((address | 0x200) & 0x3fe) | (shifter & 1);

	int32_t fetchval = rsp_divtable[address];
	int32_t temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	if (rec < 0)
	{
		if (m_dp_allowed)
		{
			if (rec < -32768)
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
		else
		{
			datainput = -datainput;
		}
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		shifter = 0;
	}

	address = ((datainput << shifter) & 0x7fc00000) >> 22;
	address = ((address | 0x200) & 0x3fe) | (shifter & 1);

	fetchval = rsp_divtable[address];
	temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0x0000;
	}
	rec = temp;

	W_VREG_S(vdreg, vs1reg & 7) = (uint16_t)rec;
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(vs2reg, VEC_EL_2(el, i)), i);
	}
}


// VRSQL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110101 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root low part

void rsp_device::cop2_drc::vrsql()
{
	CACHE_VALUES();

	int32_t shifter = 0;
	int32_t rec = (int16_t)VREG_S(vs2reg, el & 7);
	int32_t datainput = rec;

	if (m_dp_allowed)
	{
		rec = (rec & 0x0000ffff) | m_reciprocal_high;
		datainput = rec;

		if (rec < 0)
		{
			if (rec < -32768)
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
	}
	else if (datainput < 0)
	{
		datainput = -datainput;

		shifter = 0x10;
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}

	int32_t address = ((datainput << shifter) & 0x7fc00000) >> 22;
	address = ((address | 0x200) & 0x3fe) | (shifter & 1);

	int32_t fetchval = rsp_divtable[address];
	int32_t temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
	temp ^= rec >> 31;

	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	m_reciprocal_res = rec;
	m_dp_allowed = 0;

	W_VREG_S(vdreg, vs1reg & 7) = (uint16_t)(rec & 0xffff);
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(vs2reg, VEC_EL_2(el, i)), i);
	}
}


// VRSQH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110110 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root high part

void rsp_device::cop2_drc::vrsqh()
{
	CACHE_VALUES();

	m_reciprocal_high = (VREG_S(vs2reg, el & 7)) << 16;
	m_dp_allowed = 1;

	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(vs2reg, VEC_EL_2(el, i)), i);
	}

	W_VREG_S(vdreg, vs1reg & 7) = (int16_t)(m_reciprocal_res >> 16);  // store high part
}


/*-------------------------------------------------
    generate_vector_opcode - generate code for a
    vector opcode
-------------------------------------------------*/

bool rsp_device::cop2_drc::generate_vector_opcode(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	// Opcode legend:
	//    E = VS2 element type
	//    S = VS1, Source vector 1
	//    T = VS2, Source vector 2
	//    D = Destination vector

	switch (op & 0x3f)
	{
		case 0x00:      /* VMULF */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmulf, this);
			return true;

		case 0x01:      /* VMULU */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmulu, this);
			return true;

		case 0x04:      /* VMUDL */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmudl, this);
			return true;

		case 0x05:      /* VMUDM */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmudm, this);
			return true;

		case 0x06:      /* VMUDN */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmudn, this);
			return true;

		case 0x07:      /* VMUDH */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmudh, this);
			return true;

		case 0x08:      /* VMACF */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmacf, this);
			return true;

		case 0x09:      /* VMACU */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmacu, this);
			return true;

		case 0x0c:      /* VMADL */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmadl, this);
			return true;

		case 0x0d:      /* VMADM */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmadm, this);
			return true;

		case 0x0e:      /* VMADN */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmadn, this);
			return true;

		case 0x0f:      /* VMADH */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmadh, this);
			return true;

		case 0x10:      /* VADD */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vadd, this);
			return true;

		case 0x11:      /* VSUB */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vsub, this);
			return true;

		case 0x13:      /* VABS */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vabs, this);
			return true;

		case 0x14:      /* VADDC */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vaddc, this);
			return true;

		case 0x15:      /* VSUBC */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vsubc, this);
			return true;

		case 0x16:      /* VADDB */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vaddb, this);
			return true;

		case 0x17:      /* VSUBB (reserved, functionally identical to VADDB) */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vaddb, this);
			return true;

		case 0x18:      /* VACCB (reserved, functionally identical to VADDB) */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vaddb, this);
			return true;

		case 0x19:      /* VSUCB (reserved, functionally identical to VADDB) */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vaddb, this);
			return true;

		case 0x1d:      /* VSAW */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vsaw, this);
			return true;

		case 0x20:      /* VLT */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vlt, this);
			return true;

		case 0x21:      /* VEQ */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_veq, this);
			return true;

		case 0x22:      /* VNE */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vne, this);
			return true;

		case 0x23:      /* VGE */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vge, this);
			return true;

		case 0x24:      /* VCL */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vcl, this);
			return true;

		case 0x25:      /* VCH */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vch, this);
			return true;

		case 0x26:      /* VCR */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vcr, this);
			return true;

		case 0x27:      /* VMRG */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmrg, this);
			return true;

		case 0x28:      /* VAND */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vand, this);
			return true;

		case 0x29:      /* VNAND */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vnand, this);
			return true;

		case 0x2a:      /* VOR */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vor, this);
			return true;

		case 0x2b:      /* VNOR */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vnor, this);
			return true;

		case 0x2c:      /* VXOR */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vxor, this);
			return true;

		case 0x2d:      /* VNXOR */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vnxor, this);
			return true;

		case 0x30:      /* VRCP */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vrcp, this);
			return true;

		case 0x31:      /* VRCPL */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vrcpl, this);
			return true;

		case 0x32:      /* VRCPH */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vrcph, this);
			return true;

		case 0x33:      /* VMOV */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vmov, this);
			return true;

		case 0x34:      /* VRSQ */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);         // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vrsq, this);
			return true;

		case 0x35:      /* VRSQL */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vrsql, this);
			return true;

		case 0x36:      /* VRSQH */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_vrsqh, this);
			return true;

		case 0x37:      /* VNOP */
		case 0x3F:      /* VNULL */
			return true;

		default:
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::unimplemented_opcode, &m_rsp);
			return false;
	}
}


/***************************************************************************
    Vector Flag Reading/Writing
***************************************************************************/

void rsp_device::cop2_drc::mfc2()
{
	uint32_t op = m_rspcop2_state->op;
	int el = (op >> 7) & 0xf;

	uint16_t b1 = VREG_B(VS1REG, (el+0) & 0xf);
	uint16_t b2 = VREG_B(VS1REG, (el+1) & 0xf);
	if (RTREG) RTVAL = (int32_t)(int16_t)((b1 << 8) | (b2));
}

void rsp_device::cop2_drc::cfc2()
{
	uint32_t op = m_rspcop2_state->op;
	if (RTREG)
	{
		switch(RDREG)
		{
			case 0:
				RTVAL = ((CARRY_FLAG(0) & 1) << 0) |
						((CARRY_FLAG(1) & 1) << 1) |
						((CARRY_FLAG(2) & 1) << 2) |
						((CARRY_FLAG(3) & 1) << 3) |
						((CARRY_FLAG(4) & 1) << 4) |
						((CARRY_FLAG(5) & 1) << 5) |
						((CARRY_FLAG(6) & 1) << 6) |
						((CARRY_FLAG(7) & 1) << 7) |
						((ZERO_FLAG(0) & 1) << 8) |
						((ZERO_FLAG(1) & 1) << 9) |
						((ZERO_FLAG(2) & 1) << 10) |
						((ZERO_FLAG(3) & 1) << 11) |
						((ZERO_FLAG(4) & 1) << 12) |
						((ZERO_FLAG(5) & 1) << 13) |
						((ZERO_FLAG(6) & 1) << 14) |
						((ZERO_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 1:
				RTVAL = ((COMPARE_FLAG(0) & 1) << 0) |
						((COMPARE_FLAG(1) & 1) << 1) |
						((COMPARE_FLAG(2) & 1) << 2) |
						((COMPARE_FLAG(3) & 1) << 3) |
						((COMPARE_FLAG(4) & 1) << 4) |
						((COMPARE_FLAG(5) & 1) << 5) |
						((COMPARE_FLAG(6) & 1) << 6) |
						((COMPARE_FLAG(7) & 1) << 7) |
						((CLIP2_FLAG(0) & 1) << 8) |
						((CLIP2_FLAG(1) & 1) << 9) |
						((CLIP2_FLAG(2) & 1) << 10) |
						((CLIP2_FLAG(3) & 1) << 11) |
						((CLIP2_FLAG(4) & 1) << 12) |
						((CLIP2_FLAG(5) & 1) << 13) |
						((CLIP2_FLAG(6) & 1) << 14) |
						((CLIP2_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 2:
				RTVAL = ((CLIP1_FLAG(0) & 1) << 0) |
						((CLIP1_FLAG(1) & 1) << 1) |
						((CLIP1_FLAG(2) & 1) << 2) |
						((CLIP1_FLAG(3) & 1) << 3) |
						((CLIP1_FLAG(4) & 1) << 4) |
						((CLIP1_FLAG(5) & 1) << 5) |
						((CLIP1_FLAG(6) & 1) << 6) |
						((CLIP1_FLAG(7) & 1) << 7);
				break;
		}
	}
}


void rsp_device::cop2_drc::mtc2()
{
	uint32_t op = m_rspcop2_state->op;
	int el = (op >> 7) & 0xf;
	VREG_B(VS1REG, (el+0) & 0xf) = (RTVAL >> 8) & 0xff;
	VREG_B(VS1REG, (el+1) & 0xf) = (RTVAL >> 0) & 0xff;
}


void rsp_device::cop2_drc::ctc2()
{
	uint32_t op = m_rspcop2_state->op;
	switch(RDREG)
	{
		case 0:
			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			m_vflag[0][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[0][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[0][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[0][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[0][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[0][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[0][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[0][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0))  { SET_CARRY_FLAG(0); }
			if (RTVAL & (1 << 1))  { SET_CARRY_FLAG(1); }
			if (RTVAL & (1 << 2))  { SET_CARRY_FLAG(2); }
			if (RTVAL & (1 << 3))  { SET_CARRY_FLAG(3); }
			if (RTVAL & (1 << 4))  { SET_CARRY_FLAG(4); }
			if (RTVAL & (1 << 5))  { SET_CARRY_FLAG(5); }
			if (RTVAL & (1 << 6))  { SET_CARRY_FLAG(6); }
			if (RTVAL & (1 << 7))  { SET_CARRY_FLAG(7); }
			m_vflag[3][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[3][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[3][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[3][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[3][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[3][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[3][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[3][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { SET_ZERO_FLAG(0); }
			if (RTVAL & (1 << 9))  { SET_ZERO_FLAG(1); }
			if (RTVAL & (1 << 10)) { SET_ZERO_FLAG(2); }
			if (RTVAL & (1 << 11)) { SET_ZERO_FLAG(3); }
			if (RTVAL & (1 << 12)) { SET_ZERO_FLAG(4); }
			if (RTVAL & (1 << 13)) { SET_ZERO_FLAG(5); }
			if (RTVAL & (1 << 14)) { SET_ZERO_FLAG(6); }
			if (RTVAL & (1 << 15)) { SET_ZERO_FLAG(7); }
			break;
		case 1:
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();
			m_vflag[1][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[1][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[1][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[1][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[1][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[1][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[1][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[1][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { SET_COMPARE_FLAG(0); }
			if (RTVAL & (1 << 1)) { SET_COMPARE_FLAG(1); }
			if (RTVAL & (1 << 2)) { SET_COMPARE_FLAG(2); }
			if (RTVAL & (1 << 3)) { SET_COMPARE_FLAG(3); }
			if (RTVAL & (1 << 4)) { SET_COMPARE_FLAG(4); }
			if (RTVAL & (1 << 5)) { SET_COMPARE_FLAG(5); }
			if (RTVAL & (1 << 6)) { SET_COMPARE_FLAG(6); }
			if (RTVAL & (1 << 7)) { SET_COMPARE_FLAG(7); }
			m_vflag[4][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[4][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[4][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[4][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[4][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[4][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[4][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[4][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { SET_CLIP2_FLAG(0); }
			if (RTVAL & (1 << 9))  { SET_CLIP2_FLAG(1); }
			if (RTVAL & (1 << 10)) { SET_CLIP2_FLAG(2); }
			if (RTVAL & (1 << 11)) { SET_CLIP2_FLAG(3); }
			if (RTVAL & (1 << 12)) { SET_CLIP2_FLAG(4); }
			if (RTVAL & (1 << 13)) { SET_CLIP2_FLAG(5); }
			if (RTVAL & (1 << 14)) { SET_CLIP2_FLAG(6); }
			if (RTVAL & (1 << 15)) { SET_CLIP2_FLAG(7); }
			break;
		case 2:
			CLEAR_CLIP1_FLAGS();
			m_vflag[2][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[2][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[2][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[2][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[2][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[2][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[2][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[2][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { SET_CLIP1_FLAG(0); }
			if (RTVAL & (1 << 1)) { SET_CLIP1_FLAG(1); }
			if (RTVAL & (1 << 2)) { SET_CLIP1_FLAG(2); }
			if (RTVAL & (1 << 3)) { SET_CLIP1_FLAG(3); }
			if (RTVAL & (1 << 4)) { SET_CLIP1_FLAG(4); }
			if (RTVAL & (1 << 5)) { SET_CLIP1_FLAG(5); }
			if (RTVAL & (1 << 6)) { SET_CLIP1_FLAG(6); }
			if (RTVAL & (1 << 7)) { SET_CLIP1_FLAG(7); }
			break;
	}
}

/***************************************************************************
    COP2 Opcode Compilation
***************************************************************************/

bool rsp_device::cop2_drc::generate_cop2(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc)
{
	uint32_t op = desc->opptr.l[0];
	uint8_t opswitch = RSREG;

	switch (opswitch)
	{
		case 0x00:  /* MFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
				UML_CALLC(block, &cop2_drc::cfunc_mfc2, this);             // callc   mfc2
			}
			return true;

		case 0x02:  /* CFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
				UML_CALLC(block, &cop2_drc::cfunc_cfc2, this);             // callc   cfc2
			}
			return true;

		case 0x04:  /* MTCz */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_mtc2, this);             // callc   mtc2
			return true;

		case 0x06:  /* CTCz */
			UML_MOV(block, mem(&m_rspcop2_state->op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
			UML_CALLC(block, &cop2_drc::cfunc_ctc2, this);             // callc   ctc2
			return true;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			return generate_vector_opcode(block, compiler, desc);
	}
	return false;
}
