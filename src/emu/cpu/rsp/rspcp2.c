// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspcp2.c

    Universal machine language-based Nintendo/SGI RSP COP2 emulator.
    Written by Harmony of the MESS team.

***************************************************************************/

#include "emu.h"
#include "rsp.h"
#include "rspdiv.h"
#include "rspcp2.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

using namespace uml;

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

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

#define VREG_B(reg, offset)     m_v[(reg)].b[(offset)^1]
#define VREG_S(reg, offset)     m_v[(reg)].s[(offset)]
#define VREG_L(reg, offset)     m_v[(reg)].l[(offset)]

#define R_VREG_B(reg, offset)       m_v[(reg)].b[(offset)^1]
#define R_VREG_S(reg, offset)       (INT16)m_v[(reg)].s[(offset)]
#define R_VREG_L(reg, offset)       m_v[(reg)].l[(offset)]

#define W_VREG_B(reg, offset, val)  (m_v[(reg)].b[(offset)^1] = val)
#define W_VREG_S(reg, offset, val)  (m_v[(reg)].s[(offset)] = val)
#define W_VREG_L(reg, offset, val)  (m_v[(reg)].l[(offset)] = val)

#define VEC_EL_2(x,z)               (vector_elements_2[(x)][(z)])

#define CARRY       0
#define COMPARE     1
#define CLIP1       2
#define ZERO        3
#define CLIP2       4

#define ACCUM(x)            m_accum[x].q
#define ACCUM_H(x)          (UINT16)m_accum[x].w[3]
#define ACCUM_M(x)          (UINT16)m_accum[x].w[2]
#define ACCUM_L(x)          (UINT16)m_accum[x].w[1]
#define ACCUM_LL(x)         (UINT16)m_accum[x].w[0]

#define SET_ACCUM_H(v, x)       m_accum[x].w[3] = v;
#define SET_ACCUM_M(v, x)       m_accum[x].w[2] = v;
#define SET_ACCUM_L(v, x)       m_accum[x].w[1] = v;
#define SET_ACCUM_LL(v, x)      m_accum[x].w[0] = v;

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

#define WRITEBACK_RESULT() { \
		VREG_S(VDREG, 0) = m_vres[0];   \
		VREG_S(VDREG, 1) = m_vres[1];   \
		VREG_S(VDREG, 2) = m_vres[2];   \
		VREG_S(VDREG, 3) = m_vres[3];   \
		VREG_S(VDREG, 4) = m_vres[4];   \
		VREG_S(VDREG, 5) = m_vres[5];   \
		VREG_S(VDREG, 6) = m_vres[6];   \
		VREG_S(VDREG, 7) = m_vres[7];   \
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

rsp_cop2::rsp_cop2(rsp_device &rsp, running_machine &machine)
	: m_rsp(rsp)
	, m_machine(machine)
	, m_reciprocal_res(0)
	, m_reciprocal_high(0)
	, m_dp_allowed(0)
{
	memset(m_vres, 0, sizeof(m_vres));
	memset(m_v, 0, sizeof(m_v));
	memset(m_vflag, 0, sizeof(m_vflag));
	memset(m_accum, 0, sizeof(m_accum));
}

rsp_cop2::~rsp_cop2()
{
}

void rsp_cop2::init()
{
	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();
}

void rsp_cop2::start()
{
	for(int regIdx = 0; regIdx < 32; regIdx++ )
	{
		m_v[regIdx].d[0] = 0;
		m_v[regIdx].d[1] = 0;
	}

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();
	m_reciprocal_res = 0;
	m_reciprocal_high = 0;

	// Accumulators do not power on to a random state
	for(int accumIdx = 0; accumIdx < 8; accumIdx++ )
	{
		m_accum[accumIdx].q = 0;
	}
}

void rsp_cop2::state_string_export(const int index, std::string &str)
{
	switch (index)
	{
		case RSP_V0:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 0, 0), (UINT16)VREG_S( 0, 1), (UINT16)VREG_S( 0, 2), (UINT16)VREG_S( 0, 3), (UINT16)VREG_S( 0, 4), (UINT16)VREG_S( 0, 5), (UINT16)VREG_S( 0, 6), (UINT16)VREG_S( 0, 7));
			break;
		case RSP_V1:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 1, 0), (UINT16)VREG_S( 1, 1), (UINT16)VREG_S( 1, 2), (UINT16)VREG_S( 1, 3), (UINT16)VREG_S( 1, 4), (UINT16)VREG_S( 1, 5), (UINT16)VREG_S( 1, 6), (UINT16)VREG_S( 1, 7));
			break;
		case RSP_V2:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 2, 0), (UINT16)VREG_S( 2, 1), (UINT16)VREG_S( 2, 2), (UINT16)VREG_S( 2, 3), (UINT16)VREG_S( 2, 4), (UINT16)VREG_S( 2, 5), (UINT16)VREG_S( 2, 6), (UINT16)VREG_S( 2, 7));
			break;
		case RSP_V3:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 3, 0), (UINT16)VREG_S( 3, 1), (UINT16)VREG_S( 3, 2), (UINT16)VREG_S( 3, 3), (UINT16)VREG_S( 3, 4), (UINT16)VREG_S( 3, 5), (UINT16)VREG_S( 3, 6), (UINT16)VREG_S( 3, 7));
			break;
		case RSP_V4:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 4, 0), (UINT16)VREG_S( 4, 1), (UINT16)VREG_S( 4, 2), (UINT16)VREG_S( 4, 3), (UINT16)VREG_S( 4, 4), (UINT16)VREG_S( 4, 5), (UINT16)VREG_S( 4, 6), (UINT16)VREG_S( 4, 7));
			break;
		case RSP_V5:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 5, 0), (UINT16)VREG_S( 5, 1), (UINT16)VREG_S( 5, 2), (UINT16)VREG_S( 5, 3), (UINT16)VREG_S( 5, 4), (UINT16)VREG_S( 5, 5), (UINT16)VREG_S( 5, 6), (UINT16)VREG_S( 5, 7));
			break;
		case RSP_V6:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 6, 0), (UINT16)VREG_S( 6, 1), (UINT16)VREG_S( 6, 2), (UINT16)VREG_S( 6, 3), (UINT16)VREG_S( 6, 4), (UINT16)VREG_S( 6, 5), (UINT16)VREG_S( 6, 6), (UINT16)VREG_S( 6, 7));
			break;
		case RSP_V7:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 7, 0), (UINT16)VREG_S( 7, 1), (UINT16)VREG_S( 7, 2), (UINT16)VREG_S( 7, 3), (UINT16)VREG_S( 7, 4), (UINT16)VREG_S( 7, 5), (UINT16)VREG_S( 7, 6), (UINT16)VREG_S( 7, 7));
			break;
		case RSP_V8:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 8, 0), (UINT16)VREG_S( 8, 1), (UINT16)VREG_S( 8, 2), (UINT16)VREG_S( 8, 3), (UINT16)VREG_S( 8, 4), (UINT16)VREG_S( 8, 5), (UINT16)VREG_S( 8, 6), (UINT16)VREG_S( 8, 7));
			break;
		case RSP_V9:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 9, 0), (UINT16)VREG_S( 9, 1), (UINT16)VREG_S( 9, 2), (UINT16)VREG_S( 9, 3), (UINT16)VREG_S( 9, 4), (UINT16)VREG_S( 9, 5), (UINT16)VREG_S( 9, 6), (UINT16)VREG_S( 9, 7));
			break;
		case RSP_V10:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(10, 0), (UINT16)VREG_S(10, 1), (UINT16)VREG_S(10, 2), (UINT16)VREG_S(10, 3), (UINT16)VREG_S(10, 4), (UINT16)VREG_S(10, 5), (UINT16)VREG_S(10, 6), (UINT16)VREG_S(10, 7));
			break;
		case RSP_V11:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(11, 0), (UINT16)VREG_S(11, 1), (UINT16)VREG_S(11, 2), (UINT16)VREG_S(11, 3), (UINT16)VREG_S(11, 4), (UINT16)VREG_S(11, 5), (UINT16)VREG_S(11, 6), (UINT16)VREG_S(11, 7));
			break;
		case RSP_V12:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(12, 0), (UINT16)VREG_S(12, 1), (UINT16)VREG_S(12, 2), (UINT16)VREG_S(12, 3), (UINT16)VREG_S(12, 4), (UINT16)VREG_S(12, 5), (UINT16)VREG_S(12, 6), (UINT16)VREG_S(12, 7));
			break;
		case RSP_V13:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(13, 0), (UINT16)VREG_S(13, 1), (UINT16)VREG_S(13, 2), (UINT16)VREG_S(13, 3), (UINT16)VREG_S(13, 4), (UINT16)VREG_S(13, 5), (UINT16)VREG_S(13, 6), (UINT16)VREG_S(13, 7));
			break;
		case RSP_V14:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(14, 0), (UINT16)VREG_S(14, 1), (UINT16)VREG_S(14, 2), (UINT16)VREG_S(14, 3), (UINT16)VREG_S(14, 4), (UINT16)VREG_S(14, 5), (UINT16)VREG_S(14, 6), (UINT16)VREG_S(14, 7));
			break;
		case RSP_V15:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(15, 0), (UINT16)VREG_S(15, 1), (UINT16)VREG_S(15, 2), (UINT16)VREG_S(15, 3), (UINT16)VREG_S(15, 4), (UINT16)VREG_S(15, 5), (UINT16)VREG_S(15, 6), (UINT16)VREG_S(15, 7));
			break;
		case RSP_V16:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(16, 0), (UINT16)VREG_S(16, 1), (UINT16)VREG_S(16, 2), (UINT16)VREG_S(16, 3), (UINT16)VREG_S(16, 4), (UINT16)VREG_S(16, 5), (UINT16)VREG_S(16, 6), (UINT16)VREG_S(16, 7));
			break;
		case RSP_V17:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(17, 0), (UINT16)VREG_S(17, 1), (UINT16)VREG_S(17, 2), (UINT16)VREG_S(17, 3), (UINT16)VREG_S(17, 4), (UINT16)VREG_S(17, 5), (UINT16)VREG_S(17, 6), (UINT16)VREG_S(17, 7));
			break;
		case RSP_V18:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(18, 0), (UINT16)VREG_S(18, 1), (UINT16)VREG_S(18, 2), (UINT16)VREG_S(18, 3), (UINT16)VREG_S(18, 4), (UINT16)VREG_S(18, 5), (UINT16)VREG_S(18, 6), (UINT16)VREG_S(18, 7));
			break;
		case RSP_V19:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(19, 0), (UINT16)VREG_S(19, 1), (UINT16)VREG_S(19, 2), (UINT16)VREG_S(19, 3), (UINT16)VREG_S(19, 4), (UINT16)VREG_S(19, 5), (UINT16)VREG_S(19, 6), (UINT16)VREG_S(19, 7));
			break;
		case RSP_V20:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(20, 0), (UINT16)VREG_S(20, 1), (UINT16)VREG_S(20, 2), (UINT16)VREG_S(20, 3), (UINT16)VREG_S(20, 4), (UINT16)VREG_S(20, 5), (UINT16)VREG_S(20, 6), (UINT16)VREG_S(20, 7));
			break;
		case RSP_V21:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(21, 0), (UINT16)VREG_S(21, 1), (UINT16)VREG_S(21, 2), (UINT16)VREG_S(21, 3), (UINT16)VREG_S(21, 4), (UINT16)VREG_S(21, 5), (UINT16)VREG_S(21, 6), (UINT16)VREG_S(21, 7));
			break;
		case RSP_V22:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(22, 0), (UINT16)VREG_S(22, 1), (UINT16)VREG_S(22, 2), (UINT16)VREG_S(22, 3), (UINT16)VREG_S(22, 4), (UINT16)VREG_S(22, 5), (UINT16)VREG_S(22, 6), (UINT16)VREG_S(22, 7));
			break;
		case RSP_V23:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(23, 0), (UINT16)VREG_S(23, 1), (UINT16)VREG_S(23, 2), (UINT16)VREG_S(23, 3), (UINT16)VREG_S(23, 4), (UINT16)VREG_S(23, 5), (UINT16)VREG_S(23, 6), (UINT16)VREG_S(23, 7));
			break;
		case RSP_V24:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(24, 0), (UINT16)VREG_S(24, 1), (UINT16)VREG_S(24, 2), (UINT16)VREG_S(24, 3), (UINT16)VREG_S(24, 4), (UINT16)VREG_S(24, 5), (UINT16)VREG_S(24, 6), (UINT16)VREG_S(24, 7));
			break;
		case RSP_V25:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(25, 0), (UINT16)VREG_S(25, 1), (UINT16)VREG_S(25, 2), (UINT16)VREG_S(25, 3), (UINT16)VREG_S(25, 4), (UINT16)VREG_S(25, 5), (UINT16)VREG_S(25, 6), (UINT16)VREG_S(25, 7));
			break;
		case RSP_V26:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(26, 0), (UINT16)VREG_S(26, 1), (UINT16)VREG_S(26, 2), (UINT16)VREG_S(26, 3), (UINT16)VREG_S(26, 4), (UINT16)VREG_S(26, 5), (UINT16)VREG_S(26, 6), (UINT16)VREG_S(26, 7));
			break;
		case RSP_V27:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(27, 0), (UINT16)VREG_S(27, 1), (UINT16)VREG_S(27, 2), (UINT16)VREG_S(27, 3), (UINT16)VREG_S(27, 4), (UINT16)VREG_S(27, 5), (UINT16)VREG_S(27, 6), (UINT16)VREG_S(27, 7));
			break;
		case RSP_V28:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(28, 0), (UINT16)VREG_S(28, 1), (UINT16)VREG_S(28, 2), (UINT16)VREG_S(28, 3), (UINT16)VREG_S(28, 4), (UINT16)VREG_S(28, 5), (UINT16)VREG_S(28, 6), (UINT16)VREG_S(28, 7));
			break;
		case RSP_V29:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(29, 0), (UINT16)VREG_S(29, 1), (UINT16)VREG_S(29, 2), (UINT16)VREG_S(29, 3), (UINT16)VREG_S(29, 4), (UINT16)VREG_S(29, 5), (UINT16)VREG_S(29, 6), (UINT16)VREG_S(29, 7));
			break;
		case RSP_V30:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(30, 0), (UINT16)VREG_S(30, 1), (UINT16)VREG_S(30, 2), (UINT16)VREG_S(30, 3), (UINT16)VREG_S(30, 4), (UINT16)VREG_S(30, 5), (UINT16)VREG_S(30, 6), (UINT16)VREG_S(30, 7));
			break;
		case RSP_V31:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(31, 0), (UINT16)VREG_S(31, 1), (UINT16)VREG_S(31, 2), (UINT16)VREG_S(31, 3), (UINT16)VREG_S(31, 4), (UINT16)VREG_S(31, 5), (UINT16)VREG_S(31, 6), (UINT16)VREG_S(31, 7));
			break;
	}
}

/***************************************************************************
    Vector Load Instructions
***************************************************************************/

void rsp_cop2::handle_lwc2(UINT32 op)
{
	int i, end;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* LBV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Load 1 byte to vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;
			VREG_B(dest, index) = m_rsp.READ8(ea);
			break;
		}
		case 0x01:      /* LSV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 2 bytes starting from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
			break;
		}
		case 0x02:      /* LLV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 4 bytes starting from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
			break;
		}
		case 0x03:      /* LDV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 8 bytes starting from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
			break;
		}
		case 0x04:      /* LQV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00100 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads up to 16 bytes starting from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));
			if (end > 16) end = 16;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
			break;
		}
		case 0x05:      /* LRV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00101 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from right side until 16-byte boundary

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			index = 16 - ((ea & 0xf) - index);
			end = 16;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
			break;
		}
		case 0x06:      /* LPV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00110 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the upper 8 bits of each element

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea + (((16-index) + i) & 0xf)) << 8;
			}
			break;
		}
		case 0x07:      /* LUV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00111 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of each element

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea + (((16-index) + i) & 0xf)) << 7;
			}
			break;
		}
		case 0x08:      /* LHV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of each element, with 2-byte stride

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea + (((16-index) + (i<<1)) & 0xf)) << 7;
			}
			break;
		}
		case 0x09:      /* LFV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of upper or lower quad, with 4-byte stride

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			// not sure what happens if 16-byte boundary is crossed...

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea) << 7;
				ea += 4;
			}
			break;
		}
		case 0x0a:      /* LWV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = (16 - index) + 16;

			for (i=(16 - index); i < end; i++)
			{
				VREG_B(dest, i & 0xf) = m_rsp.READ8(ea);
				ea += 4;
			}
			break;
		}
		case 0x0b:      /* LTV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads one element to maximum of 8 vectors, while incrementing element index

			// FIXME: has a small problem with odd indices

			int element;
			int vs = dest;
			int ve = dest + 8;
			if (ve > 32)
				ve = 32;

			element = 7 - (index >> 1);

			if (index & 1)  fatalerror("RSP: LTV: index = %d\n", index);

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			ea = ((ea + 8) & ~0xf) + (index & 1);
			for (i=vs; i < ve; i++)
			{
				element = ((8 - (index >> 1) + (i-vs)) << 1);
				VREG_B(i, (element & 0xf)) = m_rsp.READ8(ea);
				VREG_B(i, ((element + 1) & 0xf)) = m_rsp.READ8(ea + 1);

				ea += 2;
			}
			break;
		}

		default:
		{
			m_rsp.unimplemented_opcode(op);
			break;
		}
	}
}


/***************************************************************************
    Vector Store Instructions
***************************************************************************/

void rsp_cop2::handle_swc2(UINT32 op)
{
	int i, end;
	int eaoffset;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* SBV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 1 byte from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;
			m_rsp.WRITE8(ea, VREG_B(dest, index));
			break;
		}
		case 0x01:      /* SSV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 2 bytes starting from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i));
				ea++;
			}
			break;
		}
		case 0x02:      /* SLV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 4 bytes starting from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i));
				ea++;
			}
			break;
		}
		case 0x03:      /* SDV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 8 bytes starting from vector byte index

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i));
				ea++;
			}
			break;
		}
		case 0x04:      /* SQV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00100 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from vector byte index until 16-byte boundary

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i & 0xf));
				ea++;
			}
			break;
		}
		case 0x05:      /* SRV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00101 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from right side until 16-byte boundary

			int o;
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (ea & 0xf);
			o = (16 - (ea & 0xf)) & 0xf;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, ((i + o) & 0xf)));
				ea++;
			}
			break;
		}
		case 0x06:      /* SPV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00110 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores upper 8 bits of each element

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					m_rsp.WRITE8(ea, VREG_B(dest, ((i & 0xf) << 1)));
				}
				else
				{
					m_rsp.WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
				}
				ea++;
			}
			break;
		}
		case 0x07:      /* SUV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00111 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					m_rsp.WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
				}
				else
				{
					m_rsp.WRITE8(ea, VREG_B(dest, ((i & 0x7) << 1)));
				}
				ea++;
			}
			break;
		}
		case 0x08:      /* SHV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element, with 2-byte stride

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				UINT8 d = ((VREG_B(dest, ((index + (i << 1) + 0) & 0xf))) << 1) |
							((VREG_B(dest, ((index + (i << 1) + 1) & 0xf))) >> 7);

				m_rsp.WRITE8(ea, d);
				ea += 2;
			}
			break;
		}
		case 0x09:      /* SFV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of upper or lower quad, with 4-byte stride

			// FIXME: only works for index 0 and index 8

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				m_rsp.WRITE8(ea + (eaoffset & 0xf), VREG_S(dest, i) >> 7);
				eaoffset += 4;
			}
			break;
		}
		case 0x0a:      /* SWV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = index + 16;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
				eaoffset++;
			}
			break;
		}
		case 0x0b:      /* STV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores one element from maximum of 8 vectors, while incrementing element index

			int element;
			int vs = dest;
			int ve = dest + 8;
			if (ve > 32)
				ve = 32;

			element = 8 - (index >> 1);

			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = (ea & 0xf) + (element * 2);
			ea &= ~0xf;

			for (i=vs; i < ve; i++)
			{
				m_rsp.WRITE16(ea + (eaoffset & 0xf), VREG_S(i, element & 0x7));
				eaoffset += 2;
				element++;
			}
			break;
		}

		default:
		{
			m_rsp.unimplemented_opcode(op);
			break;
		}
	}
}

/***************************************************************************
    Vector Accumulator Helpers
***************************************************************************/

UINT16 rsp_cop2::SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else if (slice == 1)
				{
					return ACCUM_M(accum);
				}
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else
				{
					return ACCUM_M(accum);
				}
			}
		}
	}
	return 0;
}


/***************************************************************************
    Vector Opcodes
***************************************************************************/

void rsp_cop2::handle_vector_ops(UINT32 op)
{
	int i;

	// Opcode legend:
	//    E = VS2 element type
	//    S = VS1, Source vector 1
	//    T = VS2, Source vector 2
	//    D = Destination vector

	switch (op & 0x3f)
	{
		case 0x00:      /* VMULF */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000000 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer * 2

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				if (s1 == -32768 && s2 == -32768)
				{
					// overflow
					SET_ACCUM_H(0, i);
					SET_ACCUM_M(-32768, i);
					SET_ACCUM_L(-32768, i);
					m_vres[i] = 0x7fff;
				}
				else
				{
					INT64 r =  s1 * s2 * 2;
					r += 0x8000;    // rounding ?
					SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
					SET_ACCUM_M((INT16)(r >> 16), i);
					SET_ACCUM_L((UINT16)(r), i);
					m_vres[i] = ACCUM_M(i);
				}
			}
			WRITEBACK_RESULT();

			break;
		}

		case 0x01:      /* VMULU */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000001 |
			// ------------------------------------------------------
			//

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				INT64 r = s1 * s2 * 2;
				r += 0x8000;    // rounding ?

				SET_ACCUM_H((UINT16)(r >> 32), i);
				SET_ACCUM_M((UINT16)(r >> 16), i);
				SET_ACCUM_L((UINT16)(r), i);

				if (r < 0)
				{
					m_vres[i] = 0;
				}
				else if (((INT16)(ACCUM_H(i)) ^ (INT16)(ACCUM_M(i))) < 0)
				{
					m_vres[i] = -1;
				}
				else
				{
					m_vres[i] = ACCUM_M(i);
				}
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x04:      /* VMUDL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000100 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by unsigned fraction
			// Stores the higher 16 bits of the 32-bit result to accumulator
			// The low slice of accumulator is stored into destination element

			for (i=0; i < 8; i++)
			{
				UINT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				UINT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				UINT32 r = s1 * s2;

				SET_ACCUM_H(0, i);
				SET_ACCUM_M(0, i);
				SET_ACCUM_L((UINT16)(r >> 16), i);

				m_vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x05:      /* VMUDM */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000101 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by unsigned fraction
			// The result is stored into accumulator
			// The middle slice of accumulator is stored into destination element

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));   // not sign-extended
				INT32 r =  s1 * s2;

				SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
				SET_ACCUM_M((INT16)(r >> 16), i);
				SET_ACCUM_L((UINT16)(r), i);

				m_vres[i] = ACCUM_M(i);
			}
			WRITEBACK_RESULT();
			break;

		}

		case 0x06:      /* VMUDN */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000110 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by signed integer
			// The result is stored into accumulator
			// The low slice of accumulator is stored into destination element

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT16)VREG_S(VS1REG, i);     // not sign-extended
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 * s2;

				SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
				SET_ACCUM_M((INT16)(r >> 16), i);
				SET_ACCUM_L((UINT16)(r), i);

				m_vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x07:      /* VMUDH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000111 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer
			// The result is stored into highest 32 bits of accumulator, the low slice is zero
			// The highest 32 bits of accumulator is saturated into destination element

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 * s2;

				SET_ACCUM_H((INT16)(r >> 16), i);
				SET_ACCUM_M((UINT16)(r), i);
				SET_ACCUM_L(0, i);

				if (r < -32768) r = -32768;
				if (r >  32767) r = 32767;
				m_vres[i] = (INT16)(r);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x08:      /* VMACF */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001000 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer * 2
			// The result is added to accumulator

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 * s2;

				UINT64 q = (UINT64)(UINT16)ACCUM_LL(i);
				q |= (((UINT64)(UINT16)ACCUM_L(i)) << 16);
				q |= (((UINT64)(UINT16)ACCUM_M(i)) << 32);
				q |= (((UINT64)(UINT16)ACCUM_H(i)) << 48);

				q += (INT64)(r) << 17;

				SET_ACCUM_LL((UINT16)q, i);
				SET_ACCUM_L((UINT16)(q >> 16), i);
				SET_ACCUM_M((UINT16)(q >> 32), i);
				SET_ACCUM_H((UINT16)(q >> 48), i);

				m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x09:      /* VMACU */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001001 |
			// ------------------------------------------------------
			//

			for (i = 0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r1 = s1 * s2;
				UINT32 r2 = (UINT16)ACCUM_L(i) + ((UINT16)(r1) * 2);
				UINT32 r3 = (UINT16)ACCUM_M(i) + (UINT16)((r1 >> 16) * 2) + (UINT16)(r2 >> 16);

				SET_ACCUM_L((UINT16)(r2), i);
				SET_ACCUM_M((UINT16)(r3), i);
				SET_ACCUM_H(ACCUM_H(i) + (UINT16)(r3 >> 16) + (UINT16)(r1 >> 31), i);

				if ((INT16)ACCUM_H(i) < 0)
				{
					m_vres[i] = 0;
				}
				else
				{
					if (ACCUM_H(i) != 0)
					{
						m_vres[i] = 0xffff;
					}
					else
					{
						if ((INT16)ACCUM_M(i) < 0)
						{
							m_vres[i] = 0xffff;
						}
						else
						{
							m_vres[i] = ACCUM_M(i);
						}
					}
				}
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x0c:      /* VMADL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001100 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by unsigned fraction
			// Adds the higher 16 bits of the 32-bit result to accumulator
			// The low slice of accumulator is stored into destination element

			for (i = 0; i < 8; i++)
			{
				UINT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				UINT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				UINT32 r1 = s1 * s2;
				UINT32 r2 = (UINT16)ACCUM_L(i) + (r1 >> 16);
				UINT32 r3 = (UINT16)ACCUM_M(i) + (r2 >> 16);

				SET_ACCUM_L((UINT16)(r2), i);
				SET_ACCUM_M((UINT16)(r3), i);
				SET_ACCUM_H(ACCUM_H(i) + (INT16)(r3 >> 16), i);

				m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x0d:      /* VMADM */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by unsigned fraction
			// The result is added into accumulator
			// The middle slice of accumulator is stored into destination element

			for (i=0; i < 8; i++)
			{
				UINT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				UINT32 s2 = (UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));   // not sign-extended
				UINT32 r1 = s1 * s2;
				UINT32 r2 = (UINT16)ACCUM_L(i) + (UINT16)(r1);
				UINT32 r3 = (UINT16)ACCUM_M(i) + (r1 >> 16) + (r2 >> 16);

				SET_ACCUM_L((UINT16)(r2), i);
				SET_ACCUM_M((UINT16)(r3), i);
				SET_ACCUM_H(ACCUM_H(i) + (UINT16)(r3 >> 16), i);
				if ((INT32)(r1) < 0)
					SET_ACCUM_H(ACCUM_H(i) - 1, i);

				m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x0e:      /* VMADN */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by signed integer
			// The result is added into accumulator
			// The low slice of accumulator is stored into destination element

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT16)VREG_S(VS1REG, i);     // not sign-extended
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				UINT64 q = (UINT64)ACCUM_LL(i);
				q |= (((UINT64)ACCUM_L(i)) << 16);
				q |= (((UINT64)ACCUM_M(i)) << 32);
				q |= (((UINT64)ACCUM_H(i)) << 48);
				q += (INT64)(s1*s2) << 16;

				SET_ACCUM_LL((UINT16)q, i);
				SET_ACCUM_L((UINT16)(q >> 16), i);
				SET_ACCUM_M((UINT16)(q >> 32), i);
				SET_ACCUM_H((UINT16)(q >> 48), i);

				m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
			}
			WRITEBACK_RESULT();

			break;
		}

		case 0x0f:      /* VMADH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001111 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer
			// The result is added into highest 32 bits of accumulator, the low slice is zero
			// The highest 32 bits of accumulator is saturated into destination element

			for (i = 0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				INT32 accum = (UINT32)(UINT16)ACCUM_M(i);
				accum |= ((UINT32)((UINT16)ACCUM_H(i))) << 16;
				accum += s1 * s2;

				SET_ACCUM_H((UINT16)(accum >> 16), i);
				SET_ACCUM_M((UINT16)accum, i);

				m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
			}
			WRITEBACK_RESULT();

			break;
		}

		case 0x10:      /* VADD */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010000 |
			// ------------------------------------------------------
			//
			// Adds two vector registers and carry flag, the result is saturated to 32767

			// TODO: check VS2REG == VDREG

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 + s2 + (CARRY_FLAG(i) != 0 ? 1 : 0);

				SET_ACCUM_L((INT16)(r), i);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;
				m_vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x11:      /* VSUB */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010001 |
			// ------------------------------------------------------
			//
			// Subtracts two vector registers and carry flag, the result is saturated to -32768

			// TODO: check VS2REG == VDREG

			for (i = 0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 - s2 - (CARRY_FLAG(i) != 0 ? 1 : 0);

				SET_ACCUM_L((INT16)(r), i);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;

				m_vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x13:      /* VABS */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010011 |
			// ------------------------------------------------------
			//
			// Changes the sign of source register 2 if source register 1 is negative and stores
			// the result to destination register

			for (i=0; i < 8; i++)
			{
				INT16 s1 = (INT16)VREG_S(VS1REG, i);
				INT16 s2 = (INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

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
			break;
		}

		case 0x14:      /* VADDC */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010100 |
			// ------------------------------------------------------
			//
			// Adds two vector registers, the carry out is stored into carry register

			// TODO: check VS2REG = VDREG

			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				INT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 + s2;

				m_vres[i] = (INT16)(r);
				SET_ACCUM_L((INT16)(r), i);

				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
				}
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x15:      /* VSUBC */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010101 |
			// ------------------------------------------------------
			//
			// Subtracts two vector registers, the carry out is stored into carry register

			// TODO: check VS2REG = VDREG

			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				INT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 - s2;

				m_vres[i] = (INT16)(r);
				SET_ACCUM_L((UINT16)(r), i);

				if ((UINT16)(r) != 0)
				{
					SET_ZERO_FLAG(i);
				}
				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
				}
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x1d:      /* VSAW */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 011101 |
			// ------------------------------------------------------
			//
			// Stores high, middle or low slice of accumulator to destination vector

			switch (EL)
			{
				case 0x08:      // VSAWH
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_H(i);
					}
					break;
				}
				case 0x09:      // VSAWM
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_M(i);
					}
					break;
				}
				case 0x0a:      // VSAWL
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_L(i);
					}
					break;
				}
				default:    //fatalerror("RSP: VSAW: el = %d\n", EL);//???????
					printf("RSP: VSAW: el = %d\n", EL);//??? ???
					exit(0);
			}
			break;
		}

		case 0x20:      /* VLT */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100000 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are less than VS2
			// Moves the element in VS2 to destination vector

			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT16 s1, s2;
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));
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

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x21:      /* VEQ */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100001 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are equal with VS2
			// Moves the element in VS2 to destination vector

			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i = 0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

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
			break;
		}

		case 0x22:      /* VNE */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100010 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are not equal with VS2
			// Moves the element in VS2 to destination vector

			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i = 0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

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

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x23:      /* VGE */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100011 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are greater or equal with VS2
			// Moves the element in VS2 to destination vector

			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

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

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x24:      /* VCL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100100 |
			// ------------------------------------------------------
			//
			// Vector clip low

			for (i = 0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if (CARRY_FLAG(i) != 0)
				{
					if (ZERO_FLAG(i) != 0)
					{
						if (COMPARE_FLAG(i) != 0)
						{
							SET_ACCUM_L(-(UINT16)s2, i);
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
							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
							{
								SET_ACCUM_L(s1, i);
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{
								SET_ACCUM_L(-((UINT16)s2), i);
								SET_COMPARE_FLAG(i);
							}
						}
						else
						{
							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) != 0)
							{
								SET_ACCUM_L(s1, i);
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{
								SET_ACCUM_L(-((UINT16)s2), i);
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
						if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
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
			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP1_FLAGS();
			WRITEBACK_RESULT();
			break;
		}

		case 0x25:      /* VCH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100101 |
			// ------------------------------------------------------
			//
			// Vector clip high

			CLEAR_CARRY_FLAGS();
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP1_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP2_FLAGS();
			UINT32 vce = 0;

			for (i=0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if ((s1 ^ s2) < 0)
				{
					vce = (s1 + s2 == -1);
					SET_CARRY_FLAG(i);
					if (s2 < 0)
					{
						SET_CLIP2_FLAG(i);
					}

					if (s1 + s2 <= 0)
					{
						SET_COMPARE_FLAG(i);
						m_vres[i] = -((UINT16)s2);
					}
					else
					{
						m_vres[i] = s1;
					}

					if (s1 + s2 != 0)
					{
						if (s1 != ~s2)
						{
							SET_ZERO_FLAG(i);
						}
					}
				}
				else
				{
					vce = 0;
					if (s2 < 0)
					{
						SET_COMPARE_FLAG(i);
					}
					if (s1 - s2 >= 0)
					{
						SET_CLIP2_FLAG(i);
						m_vres[i] = s2;
					}
					else
					{
						m_vres[i] = s1;
					}

					if ((s1 - s2) != 0)
					{
						if (s1 != ~s2)
						{
							SET_ZERO_FLAG(i);
						}
					}
				}
				if (vce != 0)
				{
					SET_CLIP1_FLAG(i);
				}

				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x26:      /* VCR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100110 |
			// ------------------------------------------------------
			//
			// Vector clip reverse

			CLEAR_CARRY_FLAGS();
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP1_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if ((INT16)(s1 ^ s2) < 0)
				{
					if (s2 < 0)
					{
						SET_CLIP2_FLAG(i);
					}
					if ((s1 + s2) <= 0)
					{
						SET_ACCUM_L(~((UINT16)s2), i);
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
			break;
		}

		case 0x27:      /* VMRG */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100111 |
			// ------------------------------------------------------
			//
			// Merges two vectors according to compare flags

			for (i = 0; i < 8; i++)
			{
				if (COMPARE_FLAG(i) != 0)
				{
					m_vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					m_vres[i] = VREG_S(VS2REG, VEC_EL_2(EL, i));
				}

				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x28:      /* VAND */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
			// ------------------------------------------------------
			//
			// Bitwise AND of two vector registers

			for (i = 0; i < 8; i++)
			{
				m_vres[i] = VREG_S(VS1REG, i) & VREG_S(VS2REG, VEC_EL_2(EL, i));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x29:      /* VNAND */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101001 |
			// ------------------------------------------------------
			//
			// Bitwise NOT AND of two vector registers

			for (i = 0; i < 8; i++)
			{
				m_vres[i] = ~((VREG_S(VS1REG, i) & VREG_S(VS2REG, VEC_EL_2(EL, i))));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2a:      /* VOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101010 |
			// ------------------------------------------------------
			//
			// Bitwise OR of two vector registers

			for (i = 0; i < 8; i++)
			{
				m_vres[i] = VREG_S(VS1REG, i) | VREG_S(VS2REG, VEC_EL_2(EL, i));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2b:      /* VNOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101011 |
			// ------------------------------------------------------
			//
			// Bitwise NOT OR of two vector registers

			for (i=0; i < 8; i++)
			{
				m_vres[i] = ~((VREG_S(VS1REG, i) | VREG_S(VS2REG, VEC_EL_2(EL, i))));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2c:      /* VXOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101100 |
			// ------------------------------------------------------
			//
			// Bitwise XOR of two vector registers

			for (i=0; i < 8; i++)
			{
				m_vres[i] = VREG_S(VS1REG, i) ^ VREG_S(VS2REG, VEC_EL_2(EL, i));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}
		case 0x2d:      /* VNXOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
			// ------------------------------------------------------
			//
			// Bitwise NOT XOR of two vector registers

			for (i=0; i < 8; i++)
			{
				m_vres[i] = ~((VREG_S(VS1REG, i) ^ VREG_S(VS2REG, VEC_EL_2(EL, i))));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x30:      /* VRCP */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110000 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal
			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, EL & 7));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
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

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
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

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}


			break;
		}

		case 0x31:      /* VRCPL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110001 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal low part

			INT32 shifter = 0;

			INT32 rec = (INT16)VREG_S(VS2REG, EL & 7);
			INT32 datainput = rec;

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


			for (i = 0; i < 32; i++)
			{
				if (datainput & (1 << ((~i) & 0x1f)))
				{
					shifter = i;
					break;
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
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

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

			break;
		}

		case 0x32:      /* VRCPH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110010 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal high part

			m_reciprocal_high = (VREG_S(VS2REG, EL & 7)) << 16;
			m_dp_allowed = 1;

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

			VREG_S(VDREG, VS1REG & 7) = (INT16)(m_reciprocal_res >> 16);

			break;
		}

		case 0x33:      /* VMOV */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110011 |
			// ------------------------------------------------------
			//
			// Moves element from vector to destination vector

			VREG_S(VDREG, VS1REG & 7) = VREG_S(VS2REG, EL & 7);
			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}
			break;
		}

		case 0x34:      /* VRSQ */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110100 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root

			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, EL & 7));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
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

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
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

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

			break;
		}

		case 0x35:      /* VRSQL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110101 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root low part

			INT32 shifter = 0;
			INT32 rec = (INT16)VREG_S(VS2REG, EL & 7);
			INT32 datainput = rec;

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
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))
					{
						shifter = i;
						break;
					}
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
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

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

			break;
		}

		case 0x36:      /* VRSQH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110110 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root high part

			m_reciprocal_high = (VREG_S(VS2REG, EL & 7)) << 16;
			m_dp_allowed = 1;

			for (i=0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

			VREG_S(VDREG, VS1REG & 7) = (INT16)(m_reciprocal_res >> 16);    // store high part
			break;
		}

		case 0x37:      /* VNOP */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110111 |
			// ------------------------------------------------------
			//
			// Vector null instruction

			break;
		}

		default:    m_rsp.unimplemented_opcode(op); break;
	}
}

/***************************************************************************
    Vector Flag Reading/Writing
***************************************************************************/

void rsp_cop2::handle_cop2(UINT32 op)
{
	switch ((op >> 21) & 0x1f)
	{
		case 0x00: /* MFC2 */
		{
			// 31 25 20 15 10 6 0
			// ---------------------------------------------------
			// | 010010 | 00000 | TTTTT | DDDDD | IIII | 0000000 |
			// ---------------------------------------------------
			//
			int el = (op >> 7) & 0xf;
			UINT16 b1 = VREG_B(RDREG, (el+0) & 0xf);
			UINT16 b2 = VREG_B(RDREG, (el+1) & 0xf);
			if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
			break;
		}

		case 0x02: /* CFC2 */
		{
			// 31 25 20 15 10 0
			// ------------------------------------------------
			// | 010010 | 00010 | TTTTT | DDDDD | 00000000000 |
			// ------------------------------------------------
			//
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
						// Anciliary clipping flags
						RTVAL = ((CLIP1_FLAG(0) & 1) << 0) |
						((CLIP1_FLAG(1) & 1) << 1) |
						((CLIP1_FLAG(2) & 1) << 2) |
						((CLIP1_FLAG(3) & 1) << 3) |
						((CLIP1_FLAG(4) & 1) << 4) |
						((CLIP1_FLAG(5) & 1) << 5) |
						((CLIP1_FLAG(6) & 1) << 6) |
						((CLIP1_FLAG(7) & 1) << 7);
				}
			}
			break;
		}

		case 0x04: /* MTC2 */
		{
			// 31 25 20 15 10 6 0
			// ---------------------------------------------------
			// | 010010 | 00100 | TTTTT | DDDDD | IIII | 0000000 |
			// ---------------------------------------------------
			//
			int el = (op >> 7) & 0xf;
			W_VREG_B(RDREG, (el+0) & 0xf, (RTVAL >> 8) & 0xff);
			W_VREG_B(RDREG, (el+1) & 0xf, (RTVAL >> 0) & 0xff);
			break;
		}

		case 0x06: /* CTC2 */
		{
			// 31 25 20 15 10 0
			// ------------------------------------------------
			// | 010010 | 00110 | TTTTT | DDDDD | 00000000000 |
			// ------------------------------------------------
			//
			switch(RDREG)
			{
				case 0:
					CLEAR_CARRY_FLAGS();
					CLEAR_ZERO_FLAGS();
					if (RTVAL & (1 << 0)) { SET_CARRY_FLAG(0); }
					if (RTVAL & (1 << 1)) { SET_CARRY_FLAG(1); }
					if (RTVAL & (1 << 2)) { SET_CARRY_FLAG(2); }
					if (RTVAL & (1 << 3)) { SET_CARRY_FLAG(3); }
					if (RTVAL & (1 << 4)) { SET_CARRY_FLAG(4); }
					if (RTVAL & (1 << 5)) { SET_CARRY_FLAG(5); }
					if (RTVAL & (1 << 6)) { SET_CARRY_FLAG(6); }
					if (RTVAL & (1 << 7)) { SET_CARRY_FLAG(7); }
					if (RTVAL & (1 << 8)) { SET_ZERO_FLAG(0); }
					if (RTVAL & (1 << 9)) { SET_ZERO_FLAG(1); }
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
					if (RTVAL & (1 << 0)) { SET_COMPARE_FLAG(0); }
					if (RTVAL & (1 << 1)) { SET_COMPARE_FLAG(1); }
					if (RTVAL & (1 << 2)) { SET_COMPARE_FLAG(2); }
					if (RTVAL & (1 << 3)) { SET_COMPARE_FLAG(3); }
					if (RTVAL & (1 << 4)) { SET_COMPARE_FLAG(4); }
					if (RTVAL & (1 << 5)) { SET_COMPARE_FLAG(5); }
					if (RTVAL & (1 << 6)) { SET_COMPARE_FLAG(6); }
					if (RTVAL & (1 << 7)) { SET_COMPARE_FLAG(7); }
					if (RTVAL & (1 << 8)) { SET_CLIP2_FLAG(0); }
					if (RTVAL & (1 << 9)) { SET_CLIP2_FLAG(1); }
					if (RTVAL & (1 << 10)) { SET_CLIP2_FLAG(2); }
					if (RTVAL & (1 << 11)) { SET_CLIP2_FLAG(3); }
					if (RTVAL & (1 << 12)) { SET_CLIP2_FLAG(4); }
					if (RTVAL & (1 << 13)) { SET_CLIP2_FLAG(5); }
					if (RTVAL & (1 << 14)) { SET_CLIP2_FLAG(6); }
					if (RTVAL & (1 << 15)) { SET_CLIP2_FLAG(7); }
					break;

				case 2:
					CLEAR_CLIP1_FLAGS();
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
			break;
		}

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			handle_vector_ops(op);
			break;
		}

		default:
			m_rsp.unimplemented_opcode(op);
			break;
	}
}

inline void rsp_cop2::mfc2()
{
	UINT32 op = m_op;
	int el = (op >> 7) & 0xf;

	UINT16 b1 = VREG_B(VS1REG, (el+0) & 0xf);
	UINT16 b2 = VREG_B(VS1REG, (el+1) & 0xf);
	if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
}

inline void rsp_cop2::cfc2()
{
	UINT32 op = m_op;
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

inline void rsp_cop2::mtc2()
{
	UINT32 op = m_op;
	int el = (op >> 7) & 0xf;
	VREG_B(VS1REG, (el+0) & 0xf) = (RTVAL >> 8) & 0xff;
	VREG_B(VS1REG, (el+1) & 0xf) = (RTVAL >> 0) & 0xff;
}

inline void rsp_cop2::ctc2()
{
	UINT32 op = m_op;
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

void rsp_cop2::log_instruction_execution()
{
	static VECTOR_REG prev_vecs[32];

	for (int i = 0; i < 32; i++)
	{
		if (m_v[i].d[0] != prev_vecs[i].d[0] || m_v[i].d[1] != prev_vecs[i].d[1])
		{
			fprintf(m_rsp.m_exec_output, "V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X ", i,
			(UINT16)VREG_S(i,0), (UINT16)VREG_S(i,1), (UINT16)VREG_S(i,2), (UINT16)VREG_S(i,3), (UINT16)VREG_S(i,4), (UINT16)VREG_S(i,5), (UINT16)VREG_S(i,6), (UINT16)VREG_S(i,7));
		}
		prev_vecs[i].d[0] = m_v[i].d[0];
		prev_vecs[i].d[1] = m_v[i].d[1];
	}
}
