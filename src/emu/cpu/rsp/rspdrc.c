/***************************************************************************

    rspdrc.c

    Universal machine language-based Nintendo/SGI RSP emulator.
    Written by Harmony of the MESS team.
    SIMD versions of vector multiplication opcodes provided by Marathon Man
      of the CEN64 team.

    Copyright the MESS team.
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Future improvements/changes:

    * Confer with Aaron Giles about adding a memory hash-based caching
      system and static recompilation for maximum overhead minimization

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "rsp.h"
#include "rspdiv.h"
#include "rspfe.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

using namespace uml;

CPU_DISASSEMBLE( rsp );

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC                       M0
#define MAPVAR_CYCLES                   M1

/* exit codes */
#define EXECUTE_OUT_OF_CYCLES           0
#define EXECUTE_MISSING_CODE            1
#define EXECUTE_UNMAPPED_CODE           2
#define EXECUTE_RESET_CACHE             3



/***************************************************************************
    MACROS
***************************************************************************/

#define R32(reg)                m_regmap[reg]

/***************************************************************************
    HELPFUL DEFINES
***************************************************************************/

#define VDREG                       ((op >> 6) & 0x1f)
#define VS1REG                      ((op >> 11) & 0x1f)
#define VS2REG                      ((op >> 16) & 0x1f)
#define EL                          ((op >> 21) & 0xf)

#define SIMD_EXTRACT16(reg, value, element) \
	switch((element) & 7) \
	{ \
		case 0: value = _mm_extract_epi16(reg, 0); break; \
		case 1: value = _mm_extract_epi16(reg, 1); break; \
		case 2: value = _mm_extract_epi16(reg, 2); break; \
		case 3: value = _mm_extract_epi16(reg, 3); break; \
		case 4: value = _mm_extract_epi16(reg, 4); break; \
		case 5: value = _mm_extract_epi16(reg, 5); break; \
		case 6: value = _mm_extract_epi16(reg, 6); break; \
		case 7: value = _mm_extract_epi16(reg, 7); break; \
	}


#define SIMD_INSERT16(reg, value, element) \
	switch((element) & 7) \
	{ \
		case 0: reg = _mm_insert_epi16(reg, value, 0); break; \
		case 1: reg = _mm_insert_epi16(reg, value, 1); break; \
		case 2: reg = _mm_insert_epi16(reg, value, 2); break; \
		case 3: reg = _mm_insert_epi16(reg, value, 3); break; \
		case 4: reg = _mm_insert_epi16(reg, value, 4); break; \
		case 5: reg = _mm_insert_epi16(reg, value, 5); break; \
		case 6: reg = _mm_insert_epi16(reg, value, 6); break; \
		case 7: reg = _mm_insert_epi16(reg, value, 7); break; \
	}


#define SIMD_EXTRACT16C(reg, value, element) value = _mm_extract_epi16(reg, element);
#define SIMD_INSERT16C(reg, value, element) reg = _mm_insert_epi16(reg, value, element);

#define VREG_B(reg, offset)         m_v[(reg)].b[(offset)^1]
#define W_VREG_S(reg, offset)       m_v[(reg)].s[(offset)]
#define VREG_S(reg, offset)         (INT16)m_v[(reg)].s[(offset)]

#define VEC_EL_2(x,z)               (vector_elements_2[(x)][(z)])

#define ACCUM(x)        m_accum[x].q

#define CARRY       0
#define COMPARE     1
#define CLIP1       2
#define ZERO        3
#define CLIP2       4


#if USE_SIMD
static void cfunc_mfc2_simd(void *param);
static void cfunc_cfc2_simd(void *param);
static void cfunc_mtc2_simd(void *param);
static void cfunc_ctc2_simd(void *param);
#endif

#if (!USE_SIMD || SIMUL_SIMD)
static void cfunc_mfc2_scalar(void *param);
static void cfunc_cfc2_scalar(void *param);
static void cfunc_mtc2_scalar(void *param);
static void cfunc_ctc2_scalar(void *param);
#endif


#if USE_SIMD
inline UINT16 rsp_device::VEC_ACCUM_H(int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_accum_h, out, x);
	return out;
}

inline UINT16 rsp_device::VEC_ACCUM_M(int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_accum_m, out, x);
	return out;
}

inline UINT16 rsp_device::VEC_ACCUM_L(int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_accum_l, out, x);
	return out;
}

inline UINT16 rsp_device::VEC_ACCUM_LL(int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_accum_ll, out, x);
	return out;
}

#define VEC_SET_ACCUM_H(v, x) SIMD_INSERT16(m_accum_h, v, x);
#define VEC_SET_ACCUM_M(v, x) SIMD_INSERT16(m_>accum_m, v, x);
#define VEC_SET_ACCUM_L(v, x) SIMD_INSERT16(m_accum_l, v, x);
#define VEC_SET_ACCUM_LL(v, x) SIMD_INSERT16(m_accum_ll, v, x);

#define VEC_GET_SCALAR_VS1(out, i) SIMD_EXTRACT16(m_xv[VS1REG], out, i);
#define VEC_GET_SCALAR_VS2(out, i) SIMD_EXTRACT16(m_xv[VS2REG], out, VEC_EL_2(EL, i));

inline UINT16 rsp_device::VEC_CARRY_FLAG(const int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_xvflag[CARRY], out, x);
	return out;
}

inline UINT16 rsp_device::VEC_COMPARE_FLAG(const int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_xvflag[COMPARE], out, x);
	return out;
}

inline UINT16 rsp_device::VEC_CLIP1_FLAG(const int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_xvflag[CLIP1], out, x);
	return out;
}

inline UINT16 rsp_device::VEC_ZERO_FLAG(const int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_xvflag[ZERO], out, x);
	return out;
}

inline UINT16 rsp_device::VEC_CLIP2_FLAG(const int x)
{
	UINT16 out;
	SIMD_EXTRACT16(m_xvflag[CLIP2], out, x);
	return out;
}

#define VEC_CLEAR_CARRY_FLAGS()     { m_xvflag[CARRY] = _mm_setzero_si128(); }
#define VEC_CLEAR_COMPARE_FLAGS()   { m_xvflag[COMPARE] = _mm_setzero_si128(); }
#define VEC_CLEAR_CLIP1_FLAGS()     { m_xvflag[CLIP1] = _mm_setzero_si128(); }
#define VEC_CLEAR_ZERO_FLAGS()      { m_xvflag[ZERO] = _mm_setzero_si128(); }
#define VEC_CLEAR_CLIP2_FLAGS()     { m_xvflag[CLIP2] = _mm_setzero_si128(); }

#define VEC_SET_CARRY_FLAG(x)       { SIMD_INSERT16(m_xvflag[CARRY], 0xffff, x); }
#define VEC_SET_COMPARE_FLAG(x)     { SIMD_INSERT16(m_xvflag[COMPARE], 0xffff, x); }
#define VEC_SET_CLIP1_FLAG(x)       { SIMD_INSERT16(m_xvflag[CLIP1], 0xffff, x); }
#define VEC_SET_ZERO_FLAG(x)        { SIMD_INSERT16(m_xvflag[ZERO], 0xffff, x); }
#define VEC_SET_CLIP2_FLAG(x)       { SIMD_INSERT16(m_xvflag[CLIP2], 0xffff, x); }

#define VEC_CLEAR_CARRY_FLAG(x)     { SIMD_INSERT16(m_xvflag[CARRY], 0, x); }
#define VEC_CLEAR_COMPARE_FLAG(x)   { SIMD_INSERT16(m_xvflag[COMPARE], 0, x); }
#define VEC_CLEAR_CLIP1_FLAG(x)     { SIMD_INSERT16(m_xvflag[CLIP1], 0, x); }
#define VEC_CLEAR_ZERO_FLAG(x)      { SIMD_INSERT16(m_xvflag[ZERO], 0, x); }
#define VEC_CLEAR_CLIP2_FLAG(x)     { SIMD_INSERT16(m_xvflag[CLIP2], 0, x); }

#endif

#define ACCUM_H(x)           (UINT16)m_accum[x].w[3]
#define ACCUM_M(x)           (UINT16)m_accum[x].w[2]
#define ACCUM_L(x)           (UINT16)m_accum[x].w[1]
#define ACCUM_LL(x)          (UINT16)m_accum[x].w[0]

#define SET_ACCUM_H(v, x)       m_accum[x].w[3] = v;
#define SET_ACCUM_M(v, x)       m_accum[x].w[2] = v;
#define SET_ACCUM_L(v, x)       m_accum[x].w[1] = v;
#define SET_ACCUM_LL(v, x)      m_accum[x].w[0] = v;

#define SCALAR_GET_VS1(out, i)  out = VREG_S(VS1REG, i)
#define SCALAR_GET_VS2(out, i)  out = VREG_S(VS2REG, VEC_EL_2(EL, i))

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


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

INLINE UINT32 epc(const opcode_desc *desc)
{
	return ((desc->flags & OPFLAG_IN_DELAY_SLOT) ? (desc->pc - 3) : desc->pc) | 0x1000;
}


/*-------------------------------------------------
    alloc_handle - allocate a handle if not
    already allocated
-------------------------------------------------*/

INLINE void alloc_handle(drcuml_state *drcuml, code_handle **handleptr, const char *name)
{
	if (*handleptr == NULL)
		*handleptr = drcuml->handle_alloc(name);
}


/*-------------------------------------------------
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

inline void rsp_device::load_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
		if (m_regmap[regnum].is_int_register())
			UML_MOV(block, ireg(m_regmap[regnum].ireg() - REG_I0), mem(&m_rsp_state->r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

inline void rsp_device::save_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
		if (m_regmap[regnum].is_int_register())
			UML_MOV(block, mem(&m_rsp_state->r[regnum]), ireg(m_regmap[regnum].ireg() - REG_I0));
}

/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

void rsp_device::rspdrc_add_imem(UINT32 *base)
{
	m_imem32 = base;
	m_imem16 = (UINT16*)base;
	m_imem8 = (UINT8*)base;
}

void rsp_device::rspdrc_add_dmem(UINT32 *base)
{
	m_dmem32 = base;
	m_dmem16 = (UINT16*)base;
	m_dmem8 = (UINT8*)base;
}

inline UINT8 rsp_device::DM_READ8(UINT32 address)
{
	UINT8 ret = m_dmem8[BYTE4_XOR_BE(address & 0xfff)];
	return ret;
}

inline void rsp_device::ccfunc_read8()
{
	m_rsp_state->arg0 = DM_READ8(m_rsp_state->arg0);
}

static void cfunc_read8(void *param)
{
	((rsp_device *)param)->ccfunc_read8();
}

inline UINT16 rsp_device::DM_READ16(UINT32 address)
{
	UINT16 ret;
	address &= 0xfff;
	ret = m_dmem8[BYTE4_XOR_BE(address)] << 8;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 1)];
	return ret;
}

inline void rsp_device::ccfunc_read16()
{
	m_rsp_state->arg0 = DM_READ16(m_rsp_state->arg0);
}

static void cfunc_read16(void *param)
{
	((rsp_device *)param)->ccfunc_read16();
}

inline UINT32 rsp_device::DM_READ32(UINT32 address)
{
	UINT32 ret;
	address &= 0xfff;
	ret = m_dmem8[BYTE4_XOR_BE(address)] << 24;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 1)] << 16;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 2)] << 8;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 3)];
	return ret;
}

inline void rsp_device::ccfunc_read32()
{
	m_rsp_state->arg0 = DM_READ32(m_rsp_state->arg0);
}

static void cfunc_read32(void *param)
{
	((rsp_device *)param)->ccfunc_read32();;
}

inline void rsp_device::DM_WRITE8(UINT32 address, UINT8 data)
{
	address &= 0xfff;
	m_dmem8[BYTE4_XOR_BE(address)] = data;
}

inline void rsp_device::ccfunc_write8()
{
	DM_WRITE8(m_rsp_state->arg0, m_rsp_state->arg1);
}

static void cfunc_write8(void *param)
{
	((rsp_device *)param)->ccfunc_write8();;
}

inline void rsp_device::DM_WRITE16(UINT32 address, UINT16 data)
{
	address &= 0xfff;
	m_dmem8[BYTE4_XOR_BE(address)] = data >> 8;
	m_dmem8[BYTE4_XOR_BE(address + 1)] = data & 0xff;
}

inline void rsp_device::ccfunc_write16()
{
	DM_WRITE16(m_rsp_state->arg0, m_rsp_state->arg1);
}

static void cfunc_write16(void *param)
{
	((rsp_device *)param)->ccfunc_write16();;
}

inline void rsp_device::DM_WRITE32(UINT32 address, UINT32 data)
{
	address &= 0xfff;
	m_dmem8[BYTE4_XOR_BE(address)] = data >> 24;
	m_dmem8[BYTE4_XOR_BE(address + 1)] = (data >> 16) & 0xff;
	m_dmem8[BYTE4_XOR_BE(address + 2)] = (data >> 8) & 0xff;
	m_dmem8[BYTE4_XOR_BE(address + 3)] = data & 0xff;
}

inline void rsp_device::ccfunc_write32()
{
	DM_WRITE32(m_rsp_state->arg0, m_rsp_state->arg1);
}

static void cfunc_write32(void *param)
{
	((rsp_device *)param)->ccfunc_write32();;
}

/*****************************************************************************/

/*-------------------------------------------------
    rspdrc_set_options - configure DRC options
-------------------------------------------------*/

void rsp_device::rspdrc_set_options(UINT32 options)
{
	if (!machine().options().drc()) return;
	m_drcoptions = options;
}


/*-------------------------------------------------
    cfunc_printf_debug - generic printf for
    debugging
-------------------------------------------------*/

#ifdef UNUSED_CODE
inline void rs_device::cfunc_printf_debug()
{
	switch(m_arg2)
	{
		case 0: // WRITE8
			printf("%04x:%02x\n", m_rsp_state->arg0 & 0xffff, (UINT8)m_rsp_state->arg1);
			break;
		case 1: // WRITE16
			printf("%04x:%04x\n", m_rsp_state->arg0 & 0xffff, (UINT16)m_rsp_state->arg1);
			break;
		case 2: // WRITE32
			printf("%04x:%08x\n", m_rsp_state->arg0 & 0xffff, m_rsp_state->arg1);
			break;
		case 3: // READ8
			printf("%04xr%02x\n", m_rsp_state->arg0 & 0xffff, (UINT8)m_rsp_state->arg1);
			break;
		case 4: // READ16
			printf("%04xr%04x\n", m_rsp_state->arg0 & 0xffff, (UINT16)m_rsp_state->arg1);
			break;
		case 5: // READ32
			printf("%04xr%08x\n", m_rsp_state->arg0 & 0xffff, m_rsp_state->arg1);
			break;
		case 6: // Checksum
			printf("Sum: %08x\n", m_rsp_state->arg0);
			break;
		case 7: // Checksum
			printf("Correct Sum: %08x\n", m_rsp_state->arg0);
			break;
		default: // ???
			printf("%08x %08x\n", m_rsp_state->arg0 & 0xffff, m_rsp_state->arg1);
			break;
	}
}

static void cfunc_printf_debug(void *param)
{
	((rsp_device *)param)->ccfunc_printf_debug();
}
#endif

inline void rsp_device::ccfunc_get_cop0_reg()
{
	int reg = m_rsp_state->arg0;
	int dest = m_rsp_state->arg1;

	if (reg >= 0 && reg < 8)
	{
		if(dest)
		{
			m_rsp_state->r[dest] = m_sp_reg_r_func(reg, 0xffffffff);
		}
	}
	else if (reg >= 8 && reg < 16)
	{
		if(dest)
		{
			m_rsp_state->r[dest] = m_dp_reg_r_func(reg - 8, 0xffffffff);
		}
	}
	else
	{
		fatalerror("RSP: cfunc_get_cop0_reg: %d\n", reg);
	}
}

static void cfunc_get_cop0_reg(void *param)
{
	((rsp_device *)param)->ccfunc_get_cop0_reg();
}

inline void rsp_device::ccfunc_set_cop0_reg()
{
	int reg = m_rsp_state->arg0;
	UINT32 data = m_rsp_state->arg1;

	if (reg >= 0 && reg < 8)
	{
		m_sp_reg_w_func(reg, data, 0xffffffff);
	}
	else if (reg >= 8 && reg < 16)
	{
		m_dp_reg_w_func(reg - 8, data, 0xffffffff);
	}
	else
	{
		fatalerror("RSP: set_cop0_reg: %d, %08X\n", reg, data);
	}
}

static void cfunc_set_cop0_reg(void *param)
{
	((rsp_device *)param)->ccfunc_set_cop0_reg();
}

inline void rsp_device::ccfunc_unimplemented_opcode()
{
	int op = m_rsp_state->arg0;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		char string[200];
		rsp_dasm_one(string, m_ppc, op);
		osd_printf_debug("%08X: %s\n", m_ppc, string);
	}

	fatalerror("RSP: unknown opcode %02X (%08X) at %08X\n", op >> 26, op, m_ppc);
}

static void cfunc_unimplemented_opcode(void *param)
{
	((rsp_device *)param)->ccfunc_unimplemented_opcode();
}

/*****************************************************************************/

/* Legacy.  Going forward, this will be transitioned into unrolled opcode decodes. */
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

#if USE_SIMD
static __m128i vec_himask;
static __m128i vec_lomask;
static __m128i vec_hibit;
static __m128i vec_lobit;
static __m128i vec_n32768;
static __m128i vec_32767;
static __m128i vec_flagmask;
static __m128i vec_shiftmask2;
static __m128i vec_shiftmask4;
static __m128i vec_flag_reverse;
static __m128i vec_neg1;
static __m128i vec_zero;
static __m128i vec_shuf[16];
static __m128i vec_shuf_inverse[16];
#endif

void rsp_device::rspcom_init()
{
#if USE_SIMD
	VEC_CLEAR_CARRY_FLAGS();
	VEC_CLEAR_COMPARE_FLAGS();
	VEC_CLEAR_CLIP1_FLAGS();
	VEC_CLEAR_ZERO_FLAGS();
	VEC_CLEAR_CLIP2_FLAGS();
#endif

#if (!USE_SIMD || SIMUL_SIMD)
	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();
#endif

#if USE_SIMD
	vec_shuf_inverse[ 0] = _mm_set_epi16(0x0f0e, 0x0d0c, 0x0b0a, 0x0908, 0x0706, 0x0504, 0x0302, 0x0100); // none
	vec_shuf_inverse[ 1] = _mm_set_epi16(0x0f0e, 0x0d0c, 0x0b0a, 0x0908, 0x0706, 0x0504, 0x0302, 0x0100); // ???
	vec_shuf_inverse[ 2] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0908, 0x0908, 0x0504, 0x0504, 0x0100, 0x0100); // 0q
	vec_shuf_inverse[ 3] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0b0a, 0x0b0a, 0x0706, 0x0706, 0x0302, 0x0302); // 1q
	vec_shuf_inverse[ 4] = _mm_set_epi16(0x0908, 0x0908, 0x0908, 0x0908, 0x0100, 0x0100, 0x0100, 0x0100); // 0h
	vec_shuf_inverse[ 5] = _mm_set_epi16(0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0302, 0x0302, 0x0302, 0x0302); // 1h
	vec_shuf_inverse[ 6] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0504, 0x0504, 0x0504, 0x0504); // 2h
	vec_shuf_inverse[ 7] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0706, 0x0706, 0x0706, 0x0706); // 3h
	vec_shuf_inverse[ 8] = _mm_set_epi16(0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100); // 0
	vec_shuf_inverse[ 9] = _mm_set_epi16(0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302); // 1
	vec_shuf_inverse[10] = _mm_set_epi16(0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504); // 2
	vec_shuf_inverse[11] = _mm_set_epi16(0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706); // 3
	vec_shuf_inverse[12] = _mm_set_epi16(0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908); // 4
	vec_shuf_inverse[13] = _mm_set_epi16(0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a); // 5
	vec_shuf_inverse[14] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c); // 6
	vec_shuf_inverse[15] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e); // 7

	vec_shuf[ 0] = _mm_set_epi16(0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e); // none
	vec_shuf[ 1] = _mm_set_epi16(0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e); // ???
	vec_shuf[ 2] = _mm_set_epi16(0x0302, 0x0302, 0x0706, 0x0706, 0x0b0a, 0x0b0a, 0x0f0e, 0x0f0e); // 0q
	vec_shuf[ 3] = _mm_set_epi16(0x0100, 0x0100, 0x0504, 0x0706, 0x0908, 0x0908, 0x0d0c, 0x0d0c); // 1q
	vec_shuf[ 4] = _mm_set_epi16(0x0706, 0x0706, 0x0706, 0x0706, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e); // 0q
	vec_shuf[ 5] = _mm_set_epi16(0x0504, 0x0504, 0x0504, 0x0504, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c); // 1q
	vec_shuf[ 6] = _mm_set_epi16(0x0302, 0x0302, 0x0302, 0x0302, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a); // 2q
	vec_shuf[ 7] = _mm_set_epi16(0x0100, 0x0100, 0x0100, 0x0100, 0x0908, 0x0908, 0x0908, 0x0908); // 3q
	vec_shuf[ 8] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e); // 0
	vec_shuf[ 9] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c); // 1
	vec_shuf[10] = _mm_set_epi16(0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a); // 2
	vec_shuf[11] = _mm_set_epi16(0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908); // 3
	vec_shuf[12] = _mm_set_epi16(0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706); // 4
	vec_shuf[13] = _mm_set_epi16(0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504); // 5
	vec_shuf[14] = _mm_set_epi16(0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302); // 6
	vec_shuf[15] = _mm_set_epi16(0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100); // 7
	m_accum_h = _mm_setzero_si128();
	m_accum_m = _mm_setzero_si128();
	m_accum_l = _mm_setzero_si128();
	m_accum_ll = _mm_setzero_si128();
	vec_neg1 = _mm_set_epi64x(0xffffffffffffffffL, 0xffffffffffffffffL);
	vec_zero = _mm_setzero_si128();
	vec_himask = _mm_set_epi64x(0xffff0000ffff0000L, 0xffff0000ffff0000L);
	vec_lomask = _mm_set_epi64x(0x0000ffff0000ffffL, 0x0000ffff0000ffffL);
	vec_hibit = _mm_set_epi64x(0x0001000000010000L, 0x0001000000010000L);
	vec_lobit = _mm_set_epi64x(0x0000000100000001L, 0x0000000100000001L);
	vec_32767 = _mm_set_epi64x(0x7fff7fff7fff7fffL, 0x7fff7fff7fff7fffL);
	vec_n32768 = _mm_set_epi64x(0x8000800080008000L, 0x8000800080008000L);
	vec_flagmask = _mm_set_epi64x(0x0001000100010001L, 0x0001000100010001L);
	vec_shiftmask2 = _mm_set_epi64x(0x0000000300000003L, 0x0000000300000003L);
	vec_shiftmask4 = _mm_set_epi64x(0x000000000000000fL, 0x000000000000000fL);
	vec_flag_reverse = _mm_set_epi16(0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e);
#endif
}


#if USE_SIMD
// LBV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00000 | IIII | Offset |
// --------------------------------------------------
//
// Load 1 byte to vector byte index

inline void rsp_device::ccfunc_rsp_lbv_simd()
{
	UINT32 op = m_rsp_state->arg0;

	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp_state->r[base] + offset : offset;

	UINT16 element;
	SIMD_EXTRACT16(m_xv[dest], element, (index >> 1));
	element &= 0xff00 >> ((1-(index & 1)) * 8);
	element |= DM_READ8(ea) << ((1-(index & 1)) * 8);
	SIMD_INSERT16(m_xv[dest], element, (index >> 1));
}

static void cfunc_rsp_lbv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lbv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)
inline void rsp_device::ccfunc_rsp_lbv_scalar()
{
	UINT32 op = m_rsp_state->arg0;

	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp_state->r[base] + offset : offset;
	VREG_B(dest, index) = DM_READ8(ea);
}

static void cfunc_rsp_lbv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lbv_scalar();
}
#endif

#if USE_SIMD
// LSV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00001 | IIII | Offset |
// --------------------------------------------------
//
// Loads 2 bytes starting from vector byte index

inline void rsp_device::ccfunc_rsp_lsv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xe;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 2) : (offset * 2);
	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		UINT16 element;
		SIMD_EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= DM_READ8(ea) << ((1 - (i & 1)) * 8);
		SIMD_INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_rsp_lsv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lsv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)
inline void rsp_device::ccfunc_rsp_lsv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xe;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 2) : (offset * 2);
	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = DM_READ8(ea);
		ea++;
	}
}

static void cfunc_rsp_lsv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lsv_scalar();
}
#endif

#if USE_SIMD
// LLV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00010 | IIII | Offset |
// --------------------------------------------------
//
// Loads 4 bytes starting from vector byte index

inline void rsp_device::ccfunc_rsp_llv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xc;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;

	for (int i = index; i < end; i++)
	{
		UINT16 element;
		SIMD_EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= DM_READ8(ea) << ((1 - (i & 1)) * 8);
		SIMD_INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_rsp_llv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_llv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_llv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xc;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;

	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = DM_READ8(ea);
		ea++;
	}
}

static void cfunc_rsp_llv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_llv_scalar();
}
#endif

#if USE_SIMD
// LDV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00011 | IIII | Offset |
// --------------------------------------------------
//
// Loads 8 bytes starting from vector byte index

inline void rsp_device::ccfunc_rsp_ldv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0x8;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;

	for (int i = index; i < end; i++)
	{
		UINT16 element;
		SIMD_EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= DM_READ8(ea) << ((1 - (i & 1)) * 8);
		SIMD_INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_rsp_ldv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_ldv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_ldv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0x8;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;

	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = DM_READ8(ea);
		ea++;
	}
}

static void cfunc_rsp_ldv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_ldv_scalar();
}
#endif

#if USE_SIMD
// LQV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00100 | IIII | Offset |
// --------------------------------------------------
//
// Loads up to 16 bytes starting from vector byte index

inline void rsp_device::ccfunc_rsp_lqv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = 16 - (ea & 0xf);
	if (end > 16) end = 16;

	for (int i = 0; i < end; i++)
	{
		UINT16 element;
		SIMD_EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= DM_READ8(ea) << ((1 - (i & 1)) * 8);
		SIMD_INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_rsp_lqv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lqv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_lqv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = 16 - (ea & 0xf);
	if (end > 16) end = 16;

	for (int i = 0; i < end; i++)
	{
		VREG_B(dest, i) = DM_READ8(ea);
		ea++;
	}
}

static void cfunc_rsp_lqv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lqv_scalar();
}
#endif

#if USE_SIMD
// LRV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00101 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from right side until 16-byte boundary

inline void rsp_device::ccfunc_rsp_lrv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	index = 16 - ((ea & 0xf) - index);
	ea &= ~0xf;

	for (int i = index; i < 16; i++)
	{
		UINT16 element;
		SIMD_EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1-(i & 1)) * 8);
		element |= DM_READ8(ea) << ((1-(i & 1)) * 8);
		SIMD_INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_rsp_lrv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lrv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_lrv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	index = 16 - ((ea & 0xf) - index);
	ea &= ~0xf;

	for (int i = index; i < 16; i++)
	{
		VREG_B(dest, i) = DM_READ8(ea);
		ea++;
	}
}

static void cfunc_rsp_lrv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lrv_scalar();
}
#endif

#if USE_SIMD
// LPV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00110 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the upper 8 bits of each element

inline void rsp_device::ccfunc_rsp_lpv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		SIMD_INSERT16(m_xv[dest], DM_READ8(ea + (((16-index) + i) & 0xf)) << 8, i);
	}
}

static void cfunc_rsp_lpv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lpv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_lpv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		W_VREG_S(dest, i) = DM_READ8(ea + (((16-index) + i) & 0xf)) << 8;
	}
}

static void cfunc_rsp_lpv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lpv_scalar();
}
#endif

#if USE_SIMD
// LUV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00111 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of each element

inline void rsp_device::ccfunc_rsp_luv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		SIMD_INSERT16(m_xv[dest], DM_READ8(ea + (((16-index) + i) & 0xf)) << 7, i);
	}
}

static void cfunc_rsp_luv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_luv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_luv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		W_VREG_S(dest, i) = DM_READ8(ea + (((16-index) + i) & 0xf)) << 7;
	}
}

static void cfunc_rsp_luv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_luv_scalar();
}
#endif

#if USE_SIMD
// LHV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01000 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of each element, with 2-byte stride

inline void rsp_device::ccfunc_rsp_lhv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	for (int i = 0; i < 8; i++)
	{
		SIMD_INSERT16(m_xv[dest], DM_READ8(ea + (((16-index) + (i<<1)) & 0xf)) << 7, i);
	}
}

static void cfunc_rsp_lhv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lhv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_lhv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	for (int i = 0; i < 8; i++)
	{
		W_VREG_S(dest, i) = DM_READ8(ea + (((16-index) + (i<<1)) & 0xf)) << 7;
	}
}

static void cfunc_rsp_lhv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lhv_scalar();
}
#endif

#if USE_SIMD
// LFV
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01001 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of upper or lower quad, with 4-byte stride

inline void rsp_device::ccfunc_rsp_lfv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	// not sure what happens if 16-byte boundary is crossed...

	int end = (index >> 1) + 4;

	for (int i = index >> 1; i < end; i++)
	{
		SIMD_INSERT16(m_xv[dest], DM_READ8(ea) << 7, i);
		ea += 4;
	}
}

static void cfunc_rsp_lfv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lfv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_lfv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	// not sure what happens if 16-byte boundary is crossed...

	int end = (index >> 1) + 4;

	for (int i = index >> 1; i < end; i++)
	{
		W_VREG_S(dest, i) = DM_READ8(ea) << 7;
		ea += 4;
	}
}

static void cfunc_rsp_lfv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lfv_scalar();
}
#endif

#if USE_SIMD
// LWV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01010 | IIII | Offset |
// --------------------------------------------------
//
// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
// after byte index 15

inline void rsp_device::ccfunc_rsp_lwv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = (16 - index) + 16;

	UINT8 val[16];
	for (int i = (16 - index); i < end; i++)
	{
		val[i & 0xf] = DM_READ8(ea);
		ea += 4;
	}

	m_xv[dest] = _mm_set_epi8(val[15], val[14], val[13], val[12], val[11], val[10], val[ 9], val[ 8],
									val[ 7], val[ 6], val[ 5], val[ 4], val[ 3], val[ 2], val[ 1], val[ 0]);
}

static void cfunc_rsp_lwv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lwv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_lwv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = (16 - index) + 16;

	for (int i = (16 - index); i < end; i++)
	{
		VREG_B(dest, i & 0xf) = DM_READ8(ea);
		ea += 4;
	}
}

static void cfunc_rsp_lwv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_lwv_scalar();
}
#endif

#if USE_SIMD
// LTV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01011 | IIII | Offset |
// --------------------------------------------------
//
// Loads one element to maximum of 8 vectors, while incrementing element index

inline void rsp_device::ccfunc_rsp_ltv_simd()
{
	UINT32 op = m_rsp_state->arg0;
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

	int element = 7 - (index >> 1);

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	ea = ((ea + 8) & ~0xf) + (index & 1);
	for (int i = vs; i < ve; i++)
	{
		element = (8 - (index >> 1) + (i - vs)) << 1;
		UINT16 value = (DM_READ8(ea) << 8) | DM_READ8(ea + 1);
		SIMD_INSERT16(m_xv[i], value, (element >> 1));
		ea += 2;
	}
}

static void cfunc_rsp_ltv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_ltv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_ltv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
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

	int element = 7 - (index >> 1);

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	ea = ((ea + 8) & ~0xf) + (index & 1);
	for (int i = vs; i < ve; i++)
	{
		element = (8 - (index >> 1) + (i - vs)) << 1;
		VREG_B(i, (element & 0xf)) = DM_READ8(ea);
		VREG_B(i, ((element + 1) & 0xf)) = DM_READ8(ea + 1);
		ea += 2;
	}
}

static void cfunc_rsp_ltv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_ltv_scalar();
}
#endif

#if USE_SIMD && SIMUL_SIMD
inline void rsp_device::ccfunc_backup_regs()
{
	memcpy(m_old_dmem, m_dmem8, sizeof(m_old_dmem));
	memcpy(m_old_r, m_r, sizeof(m_r));

	m_simd_reciprocal_res = m_reciprocal_res;
	m_simd_reciprocal_high = m_reciprocal_high;
	m_simd_dp_allowed = m_dp_allowed;

	m_reciprocal_res = m_old_reciprocal_res;
	m_reciprocal_high = m_old_reciprocal_high;
	m_dp_allowed = m_old_dp_allowed;
}

static void cfunc_backup_regs(void *param)
{
	((rsp_device *)param)->ccfunc_backup_regs();
}

inline void rsp_device::ccfunc_restore_regs()
{
	memcpy(m_scalar_r, m_r, sizeof(m_r));
	memcpy(m_r, m_old_r, sizeof(m_r));
	memcpy(m_scalar_dmem, m_dmem8, sizeof(m_scalar_dmem));
	memcpy(m_dmem8, m_old_dmem, sizeof(m_old_dmem));

	m_scalar_reciprocal_res = m_reciprocal_res;
	m_scalar_reciprocal_high = m_reciprocal_high;
	m_scalar_dp_allowed = m_dp_allowed;

	m_reciprocal_res = m_simd_reciprocal_res;
	m_reciprocal_high = m_simd_reciprocal_high;
	m_dp_allowed = m_simd_dp_allowed;
}

static void cfunc_restore_regs(void *param)
{
	((rsp_device *)param)->ccfunc_restore_regs();
}

inline void rsp_device::ccfunc_verify_regs()
{
	int op = m_rsp_state->arg0;
	if (VEC_ACCUM_H(0) != ACCUM_H(0)) fatalerror("ACCUM_H element 0 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(0), ACCUM_H(0), op);
	if (VEC_ACCUM_H(1) != ACCUM_H(1)) fatalerror("ACCUM_H element 1 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(1), ACCUM_H(1), op);
	if (VEC_ACCUM_H(2) != ACCUM_H(2)) fatalerror("ACCUM_H element 2 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(2), ACCUM_H(2), op);
	if (VEC_ACCUM_H(3) != ACCUM_H(3)) fatalerror("ACCUM_H element 3 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(3), ACCUM_H(3), op);
	if (VEC_ACCUM_H(4) != ACCUM_H(4)) fatalerror("ACCUM_H element 4 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(4), ACCUM_H(4), op);
	if (VEC_ACCUM_H(5) != ACCUM_H(5)) fatalerror("ACCUM_H element 5 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(5), ACCUM_H(5), op);
	if (VEC_ACCUM_H(6) != ACCUM_H(6)) fatalerror("ACCUM_H element 6 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(6), ACCUM_H(6), op);
	if (VEC_ACCUM_H(7) != ACCUM_H(7)) fatalerror("ACCUM_H element 7 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_H(7), ACCUM_H(7), op);
	if (VEC_ACCUM_M(0) != ACCUM_M(0)) fatalerror("ACCUM_M element 0 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(0), ACCUM_M(0), op);
	if (VEC_ACCUM_M(1) != ACCUM_M(1)) fatalerror("ACCUM_M element 1 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(1), ACCUM_M(1), op);
	if (VEC_ACCUM_M(2) != ACCUM_M(2)) fatalerror("ACCUM_M element 2 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(2), ACCUM_M(2), op);
	if (VEC_ACCUM_M(3) != ACCUM_M(3)) fatalerror("ACCUM_M element 3 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(3), ACCUM_M(3), op);
	if (VEC_ACCUM_M(4) != ACCUM_M(4)) fatalerror("ACCUM_M element 4 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(4), ACCUM_M(4), op);
	if (VEC_ACCUM_M(5) != ACCUM_M(5)) fatalerror("ACCUM_M element 5 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(5), ACCUM_M(5), op);
	if (VEC_ACCUM_M(6) != ACCUM_M(6)) fatalerror("ACCUM_M element 6 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(6), ACCUM_M(6), op);
	if (VEC_ACCUM_M(7) != ACCUM_M(7)) fatalerror("ACCUM_M element 7 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_M(7), ACCUM_M(7), op);
	if (VEC_ACCUM_L(0) != ACCUM_L(0)) fatalerror("ACCUM_L element 0 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(0), ACCUM_L(0), op);
	if (VEC_ACCUM_L(1) != ACCUM_L(1)) fatalerror("ACCUM_L element 1 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(1), ACCUM_L(1), op);
	if (VEC_ACCUM_L(2) != ACCUM_L(2)) fatalerror("ACCUM_L element 2 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(2), ACCUM_L(2), op);
	if (VEC_ACCUM_L(3) != ACCUM_L(3)) fatalerror("ACCUM_L element 3 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(3), ACCUM_L(3), op);
	if (VEC_ACCUM_L(4) != ACCUM_L(4)) fatalerror("ACCUM_L element 4 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(4), ACCUM_L(4), op);
	if (VEC_ACCUM_L(5) != ACCUM_L(5)) fatalerror("ACCUM_L element 5 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(5), ACCUM_L(5), op);
	if (VEC_ACCUM_L(6) != ACCUM_L(6)) fatalerror("ACCUM_L element 6 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(6), ACCUM_L(6), op);
	if (VEC_ACCUM_L(7) != ACCUM_L(7)) fatalerror("ACCUM_L element 7 mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", VEC_ACCUM_L(7), ACCUM_L(7), op);
	for (int i = 0; i < 32; i++)
	{
		if (m_rsp_state->r[i] != m_scalar_r[i]) fatalerror("r[%d] mismatch (SIMD %08x vs. Scalar %08x) after op: %08x\n", i, m_rsp_state->r[i], m_scalar_r[i], op);
		for (int el = 0; el < 8; el++)
		{
			UINT16 out;
			SIMD_EXTRACT16(m_xv[i], out, el);
			if ((UINT16)VREG_S(i, el) != out) fatalerror("Vector %d element %d mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", i, el, out, (UINT16)VREG_S(i, el), op);
		}
	}
	for (int i = 0; i < 4096; i++)
	{
		if (m_dmem8[i] != m_scalar_dmem[i]) fatalerror("dmem[%d] mismatch (SIMD %02x vs. Scalar %02x) after op: %08x\n", i, m_dmem8[i], m_scalar_dmem[i], op);
	}
	for (int i = 0; i < 5; i++)
	{
		for (int el = 0; el < 8; el++)
		{
			UINT16 out;
			SIMD_EXTRACT16(m_xvflag[i], out, el);
			if (m_vflag[i][el] != out) fatalerror("flag[%d][%d] mismatch (SIMD %04x vs. Scalar %04x) after op: %08x\n", i, el, out, m_vflag[i][el], op);
		}
	}
}

static void cfunc_verify_regs(void *param)
{
	((rsp_device *)param)->ccfunc_verify_regs();
}
#endif

#if USE_SIMD
int rsp_device::generate_lwc2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	//int loopdest;
	UINT32 op = desc->opptr.l[0];
	//int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	//int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* LBV */
			//UML_ADD(block, I0, R32(RSREG), offset);
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lbv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lbv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x01:      /* LSV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lsv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lsv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x02:      /* LLV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_llv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_llv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x03:      /* LDV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ldv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_ldv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x04:      /* LQV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lqv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lqv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x05:      /* LRV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lrv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lrv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x06:      /* LPV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lpv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lpv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x07:      /* LUV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_luv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_luv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x08:      /* LHV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lhv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lhv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x09:      /* LFV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lfv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lfv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x0a:      /* LWV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lwv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_lwv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x0b:      /* LTV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ltv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_ltv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		default:
			return FALSE;
	}
}

#else

int rsp_device::generate_lwc2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	//int loopdest;
	UINT32 op = desc->opptr.l[0];
	//int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	//int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* LBV */
			//UML_ADD(block, I0, R32(RSREG), offset);
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lbv_scalar, this);
			return TRUE;
		case 0x01:      /* LSV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lsv_scalar, this);
			return TRUE;
		case 0x02:      /* LLV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_llv_scalar, this);
			return TRUE;
		case 0x03:      /* LDV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ldv_scalar, this);
			return TRUE;
		case 0x04:      /* LQV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lqv_scalar, this);
			return TRUE;
		case 0x05:      /* LRV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lrv_scalar, this);
			return TRUE;
		case 0x06:      /* LPV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lpv_scalar, this);
			return TRUE;
		case 0x07:      /* LUV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_luv_scalar, this);
			return TRUE;
		case 0x08:      /* LHV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lhv_scalar, this);
			return TRUE;
		case 0x09:      /* LFV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lfv_scalar, this);
			return TRUE;
		case 0x0a:      /* LWV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lwv_scalar, this);
			return TRUE;
		case 0x0b:      /* LTV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ltv_scalar, this);
			return TRUE;

		default:
			return FALSE;
	}
}
#endif

#if USE_SIMD
// SBV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00000 | IIII | Offset |
// --------------------------------------------------
//
// Stores 1 byte from vector byte index

inline void rsp_device::ccfunc_rsp_sbv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + offset : offset;
	UINT16 value;
	SIMD_EXTRACT16(m_xv[dest], value, (index >> 1));
	value >>= (1-(index & 1)) * 8;
	DM_WRITE8(ea, (UINT8)value);
}

static void cfunc_rsp_sbv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sbv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_sbv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + offset : offset;
	DM_WRITE8(ea, VREG_B(dest, index));
}

static void cfunc_rsp_sbv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sbv_scalar();
}
#endif

#if USE_SIMD
// SSV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00001 | IIII | Offset |
// --------------------------------------------------
//
// Stores 2 bytes starting from vector byte index

inline void rsp_device::ccfunc_rsp_ssv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 2) : (offset * 2);

	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1 - (i & 1)) * 8;
		DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_rsp_ssv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_ssv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_ssv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 2) : (offset * 2);

	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		DM_WRITE8(ea, VREG_B(dest, i));
		ea++;
	}
}

static void cfunc_rsp_ssv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_ssv_scalar();
}
#endif

#if USE_SIMD
// SLV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00010 | IIII | Offset |
// --------------------------------------------------
//
// Stores 4 bytes starting from vector byte index

inline void rsp_device::ccfunc_rsp_slv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1 - (i & 1)) * 8;
		DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_rsp_slv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_slv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_slv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;
	for (int i = index; i < end; i++)
	{
		DM_WRITE8(ea, VREG_B(dest, i));
		ea++;
	}
}

static void cfunc_rsp_slv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_slv_scalar();
}
#endif

#if USE_SIMD
// SDV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00011 | IIII | Offset |
// --------------------------------------------------
//
// Stores 8 bytes starting from vector byte index

inline void rsp_device::ccfunc_rsp_sdv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0x8;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1 - (i & 1)) * 8;
		DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_rsp_sdv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sdv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_sdv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0x8;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;
	for (int i = index; i < end; i++)
	{
		DM_WRITE8(ea, VREG_B(dest, i));
		ea++;
	}
}

static void cfunc_rsp_sdv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sdv_scalar();
}
#endif

#if USE_SIMD
// SQV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00100 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from vector byte index until 16-byte boundary

inline void rsp_device::ccfunc_rsp_sqv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = index + (16 - (ea & 0xf));
	for (int i=index; i < end; i++)
	{
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1-(i & 1)) * 8;
		DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_rsp_sqv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sqv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_sqv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = index + (16 - (ea & 0xf));
	for (int i=index; i < end; i++)
	{
		DM_WRITE8(ea, VREG_B(dest, i & 0xf));
		ea++;
	}
}

static void cfunc_rsp_sqv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sqv_scalar();
}
#endif

#if USE_SIMD
// SRV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00101 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from right side until 16-byte boundary

inline void rsp_device::ccfunc_rsp_srv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = index + (ea & 0xf);
	int o = (16 - (ea & 0xf)) & 0xf;
	ea &= ~0xf;

	for (int i = index; i < end; i++)
	{
		UINT32 bi = (i + o) & 0xf;
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, (bi >> 1));
		value >>= (1-(bi & 1)) * 8;
		DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_rsp_srv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_srv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_srv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = index + (ea & 0xf);
	int o = (16 - (ea & 0xf)) & 0xf;
	ea &= ~0xf;

	for (int i = index; i < end; i++)
	{
		DM_WRITE8(ea, VREG_B(dest, ((i + o) & 0xf)));
		ea++;
	}
}

static void cfunc_rsp_srv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_srv_scalar();
}
#endif

#if USE_SIMD
// SPV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00110 | IIII | Offset |
// --------------------------------------------------
//
// Stores upper 8 bits of each element

inline void rsp_device::ccfunc_rsp_spv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			UINT16 value;
			SIMD_EXTRACT16(m_xv[dest], value, i);
			DM_WRITE8(ea, (UINT8)(value >> 8));
		}
		else
		{
			UINT16 value;
			SIMD_EXTRACT16(m_xv[dest], value, i);
			DM_WRITE8(ea, (UINT8)(value >> 7));
		}
		ea++;
	}
}

static void cfunc_rsp_spv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_spv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_spv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			DM_WRITE8(ea, VREG_B(dest, (i & 0xf) << 1));
		}
		else
		{
			DM_WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
		}
		ea++;
	}
}

static void cfunc_rsp_spv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_spv_scalar();
}
#endif

#if USE_SIMD
// SUV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00111 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of each element

inline void rsp_device::ccfunc_rsp_suv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			UINT16 value;
			SIMD_EXTRACT16(m_xv[dest], value, i);
			DM_WRITE8(ea, (UINT8)(value >> 7));
		}
		else
		{
			UINT16 value;
			SIMD_EXTRACT16(m_xv[dest], value, i);
			DM_WRITE8(ea, (UINT8)(value >> 8));
		}
		ea++;
	}
}

static void cfunc_rsp_suv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_suv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_suv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			DM_WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
		}
		else
		{
			DM_WRITE8(ea, VREG_B(dest, ((i & 0x7) << 1)));
		}
		ea++;
	}
}

static void cfunc_rsp_suv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_suv_scalar();
}
#endif

#if USE_SIMD
// SHV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01000 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of each element, with 2-byte stride

inline void rsp_device::ccfunc_rsp_shv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	for (int i=0; i < 8; i++)
	{
		int element = index + (i << 1);
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, element >> 1);
		DM_WRITE8(ea, (value >> 7) & 0x00ff);
		ea += 2;
	}
}

static void cfunc_rsp_shv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_shv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_shv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	for (int i=0; i < 8; i++)
	{
		int element = index + (i << 1);
		UINT8 d = (VREG_B(dest, (element & 0xf)) << 1) |
					(VREG_B(dest, ((element + 1) & 0xf)) >> 7);
		DM_WRITE8(ea, d);
		ea += 2;
	}
}

static void cfunc_rsp_shv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_shv_scalar();
}
#endif

#if USE_SIMD
// SFV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01001 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of upper or lower quad, with 4-byte stride

inline void rsp_device::ccfunc_rsp_sfv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = (index >> 1) + 4;

	for (int i = index>>1; i < end; i++)
	{
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, i);
		DM_WRITE8(ea + (eaoffset & 0xf), (value >> 7) & 0x00ff);
		eaoffset += 4;
	}
}

static void cfunc_rsp_sfv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sfv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_sfv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = (index >> 1) + 4;

	for (int i = index>>1; i < end; i++)
	{
		DM_WRITE8(ea + (eaoffset & 0xf), VREG_S(dest, i) >> 7);
		eaoffset += 4;
	}
}

static void cfunc_rsp_sfv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_sfv_scalar();
}
#endif

#if USE_SIMD
// SWV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01010 | IIII | Offset |
// --------------------------------------------------
//
// Stores the full 128-bit vector starting from vector byte index and wrapping to index 0
// after byte index 15

inline void rsp_device::ccfunc_rsp_swv_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = index + 16;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		SIMD_EXTRACT16(m_xv[dest], value, i >> 1);
		DM_WRITE8(ea + (eaoffset & 0xf), (value >> ((1-(i & 1)) * 8)) & 0xff);
		eaoffset++;
	}
}

static void cfunc_rsp_swv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_swv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_swv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = index + 16;
	for (int i = index; i < end; i++)
	{
		DM_WRITE8(ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
		eaoffset++;
	}
}

static void cfunc_rsp_swv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_swv_scalar();
}
#endif

#if USE_SIMD
// STV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01011 | IIII | Offset |
// --------------------------------------------------
//
// Stores one element from maximum of 8 vectors, while incrementing element index

inline void rsp_device::ccfunc_rsp_stv_simd()
{
	UINT32 op = m_rsp_state->arg0;
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

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = (ea & 0xf) + (element * 2);
	ea &= ~0xf;

	for (int i = vs; i < ve; i++)
	{
		UINT16 value;
		SIMD_EXTRACT16(m_xv[i], value, element);
		DM_WRITE16(ea + (eaoffset & 0xf), value);
		eaoffset += 2;
		element++;
	}
}

static void cfunc_rsp_stv_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_stv_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_stv_scalar()
{
	UINT32 op = m_rsp_state->arg0;
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

	UINT32 ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = (ea & 0xf) + (element * 2);
	ea &= ~0xf;

	for (int i = vs; i < ve; i++)
	{
		DM_WRITE16(ea + (eaoffset & 0xf), VREG_S(i, element & 0x7));
		eaoffset += 2;
		element++;
	}
}

static void cfunc_rsp_stv_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_stv_scalar();
}
#endif

#if USE_SIMD
int rsp_device::generate_swc2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
//  int loopdest;
	UINT32 op = desc->opptr.l[0];
	//int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	//int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* SBV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sbv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_sbv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x01:      /* SSV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ssv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_ssv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x02:      /* SLV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_slv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_slv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x03:      /* SDV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sdv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_sdv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x04:      /* SQV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sqv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_sqv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x05:      /* SRV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_srv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_srv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x06:      /* SPV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_spv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_spv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x07:      /* SUV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_suv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_suv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x08:      /* SHV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_shv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_shv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x09:      /* SFV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sfv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_sfv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x0a:      /* SWV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_swv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_swv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;
		case 0x0b:      /* STV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_stv_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_stv_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		default:
			unimplemented_opcode(op);
			return FALSE;
	}

	return TRUE;
}

#else

int rsp_device::generate_swc2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
//  int loopdest;
	UINT32 op = desc->opptr.l[0];
	//int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	//int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* SBV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sbv_scalar, this);
			return TRUE;
		case 0x01:      /* SSV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ssv_scalar, this);
			return TRUE;
		case 0x02:      /* SLV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_slv_scalar, this);
			return TRUE;
		case 0x03:      /* SDV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sdv_scalar, this);
			return TRUE;
		case 0x04:      /* SQV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sqv_scalar, this);
			return TRUE;
		case 0x05:      /* SRV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_srv_scalar, this);
			return TRUE;
		case 0x06:      /* SPV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_spv_scalar, this);
			return TRUE;
		case 0x07:      /* SUV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_suv_scalar, this);
			return TRUE;
		case 0x08:      /* SHV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_shv_scalar, this);
			return TRUE;
		case 0x09:      /* SFV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sfv_scalar, this);
			return TRUE;
		case 0x0a:      /* SWV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_swv_scalar, this);
			return TRUE;
		case 0x0b:      /* STV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_stv_scalar, this);
			return TRUE;

		default:
			unimplemented_opcode(op);
			return FALSE;
	}

	return TRUE;
}
#endif

#if USE_SIMD
inline UINT16 rsp_device::VEC_SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive)
{
	if ((INT16)VEC_ACCUM_H(accum) < 0)
	{
		if ((UINT16)(VEC_ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)VEC_ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
				if (slice == 0)
				{
					return VEC_ACCUM_L(accum);
				}
				else if (slice == 1)
				{
					return VEC_ACCUM_M(accum);
				}
			}
		}
	}
	else
	{
		if ((UINT16)(VEC_ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)VEC_ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
				if (slice == 0)
				{
					return VEC_ACCUM_L(accum);
				}
				else
				{
					return VEC_ACCUM_M(accum);
				}
			}
		}
	}
	return 0;
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)
inline UINT16 rsp_device::SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive)
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
#endif

inline UINT16 rsp_device::SATURATE_ACCUM1(int accum, UINT16 negative, UINT16 positive)
{
	// Return negative if H<0 && (H!=0xffff || M >= 0)
	// Return positive if H>0 || (H==0 && M<0)
	// Return medium slice if H==0xffff && M<0
	// Return medium slice if H==0 && M>=0
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
				return ACCUM_M(accum);
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
				return ACCUM_M(accum);
			}
		}
	}
	// never executed
	//return 0;
}

#if USE_SIMD
#define VEC_WRITEBACK_RESULT() { \
		SIMD_INSERT16(m_xv[VDREG], vres[0], 0); \
		SIMD_INSERT16(m_xv[VDREG], vres[1], 1); \
		SIMD_INSERT16(m_xv[VDREG], vres[2], 2); \
		SIMD_INSERT16(m_xv[VDREG], vres[3], 3); \
		SIMD_INSERT16(m_xv[VDREG], vres[4], 4); \
		SIMD_INSERT16(m_xv[VDREG], vres[5], 5); \
		SIMD_INSERT16(m_xv[VDREG], vres[6], 6); \
		SIMD_INSERT16(m_xv[VDREG], vres[7], 7); \
}
#endif

#define WRITEBACK_RESULT() { \
		W_VREG_S(VDREG, 0) = vres[0];   \
		W_VREG_S(VDREG, 1) = vres[1];   \
		W_VREG_S(VDREG, 2) = vres[2];   \
		W_VREG_S(VDREG, 3) = vres[3];   \
		W_VREG_S(VDREG, 4) = vres[4];   \
		W_VREG_S(VDREG, 5) = vres[5];   \
		W_VREG_S(VDREG, 6) = vres[6];   \
		W_VREG_S(VDREG, 7) = vres[7];   \
}

#if USE_SIMD
/* ============================================================================
* RSPPackLo32to16: Pack LSBs of 32-bit vectors to 16-bits without saturation.
* TODO: 5 SSE2 operations is kind of expensive just to truncate values?
* ========================================================================= */
INLINE __m128i RSPPackLo32to16(__m128i vectorLow, __m128i vectorHigh)
{
	vectorLow = _mm_slli_epi32(vectorLow, 16);
	vectorHigh = _mm_slli_epi32(vectorHigh, 16);
	vectorLow = _mm_srai_epi32(vectorLow, 16);
	vectorHigh = _mm_srai_epi32(vectorHigh, 16);
	return _mm_packs_epi32(vectorLow, vectorHigh);
}

/* ============================================================================
* RSPPackHi32to16: Pack MSBs of 32-bit vectors to 16-bits without saturation.
* ========================================================================= */
INLINE __m128i RSPPackHi32to16(__m128i vectorLow, __m128i vectorHigh)
{
	vectorLow = _mm_srai_epi32(vectorLow, 16);
	vectorHigh = _mm_srai_epi32(vectorHigh, 16);
	return _mm_packs_epi32(vectorLow, vectorHigh);
}

/* ============================================================================
* RSPSignExtend16to32: Sign-extend 16-bit slices to 32-bit slices.
* ========================================================================= */
INLINE void RSPSignExtend16to32(__m128i source, __m128i *vectorLow, __m128i *vectorHigh)
{
	__m128i vMask = _mm_srai_epi16(source, 15);
	*vectorHigh = _mm_unpackhi_epi16(source, vMask);
	*vectorLow = _mm_unpacklo_epi16(source, vMask);
}

/* ============================================================================
* RSPZeroExtend16to32: Zero-extend 16-bit slices to 32-bit slices.
* ========================================================================= */
INLINE void RSPZeroExtend16to32(__m128i source, __m128i *vectorLow, __m128i *vectorHigh)
{
	*vectorHigh = _mm_unpackhi_epi16(source, _mm_setzero_si128());
	*vectorLow = _mm_unpacklo_epi16(source, _mm_setzero_si128());
}

/* ============================================================================
* _mm_mullo_epi32: SSE2 lacks _mm_mullo_epi32, define it manually.
* TODO/WARNING/DISCLAIMER: Assumes one argument is positive.
* ========================================================================= */
INLINE __m128i _mm_mullo_epi32(__m128i a, __m128i b)
{
	__m128i a4 = _mm_srli_si128(a, 4);
	__m128i b4 = _mm_srli_si128(b, 4);
	__m128i ba = _mm_mul_epu32(b, a);
	__m128i b4a4 = _mm_mul_epu32(b4, a4);

	__m128i mask = _mm_setr_epi32(~0, 0, ~0, 0);
	__m128i baMask = _mm_and_si128(ba, mask);
	__m128i b4a4Mask = _mm_and_si128(b4a4, mask);
	__m128i b4a4MaskShift = _mm_slli_si128(b4a4Mask, 4);

	return _mm_or_si128(baMask, b4a4MaskShift);
}

/* ============================================================================
* RSPClampLowToVal: Clamps the low word of the accumulator.
* ========================================================================= */
INLINE __m128i RSPClampLowToVal(__m128i vaccLow, __m128i vaccMid, __m128i vaccHigh)
{
	__m128i setMask = _mm_cmpeq_epi16(_mm_setzero_si128(), _mm_setzero_si128());
	__m128i negCheck, useValMask, negVal, posVal;

	/* Compute some common values ahead of time. */
	negCheck = _mm_cmplt_epi16(vaccHigh, _mm_setzero_si128());

	/* If accmulator < 0, clamp to val if val != TMin. */
	useValMask = _mm_and_si128(vaccHigh, _mm_srai_epi16(vaccMid, 15));
	useValMask = _mm_cmpeq_epi16(useValMask, setMask);
	negVal = _mm_and_si128(useValMask, vaccLow);

	/* Otherwise, clamp to ~0 if any high bits are set. */
	useValMask = _mm_or_si128(vaccHigh, _mm_srai_epi16(vaccMid, 15));
	useValMask = _mm_cmpeq_epi16(useValMask, _mm_setzero_si128());
	posVal = _mm_and_si128(useValMask, vaccLow);

	negVal = _mm_and_si128(negCheck, negVal);
	posVal = _mm_andnot_si128(negCheck, posVal);
	return _mm_or_si128(negVal, posVal);
}
#endif

#if USE_SIMD
// VMULF
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000000 |
// ------------------------------------------------------
//
// Multiplies signed integer by signed integer * 2

inline void rsp_device::ccfunc_rsp_vmulf_simd()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		VEC_GET_SCALAR_VS1(w1, i);
		VEC_GET_SCALAR_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		if (s1 == -32768 && s2 == -32768)
		{
			// overflow
			VEC_SET_ACCUM_H(0, i);
			VEC_SET_ACCUM_M(-32768, i);
			VEC_SET_ACCUM_L(-32768, i);
			vres[i] = 0x7fff;
		}
		else
		{
			INT64 r =  s1 * s2 * 2;
			r += 0x8000;    // rounding ?
			VEC_SET_ACCUM_H((r < 0) ? 0xffff : 0, i);
			VEC_SET_ACCUM_M((INT16)(r >> 16), i);
			VEC_SET_ACCUM_L((UINT16)(r), i);
			vres[i] = VEC_ACCUM_M(i);
		}
	}
	VEC_WRITEBACK_RESULT();
}

static void cfunc_rsp_vmulf_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmulf_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmulf_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		if (s1 == -32768 && s2 == -32768)
		{
			// overflow
			SET_ACCUM_H(0, i);
			SET_ACCUM_M(-32768, i);
			SET_ACCUM_L(-32768, i);
			vres[i] = 0x7fff;
		}
		else
		{
			INT64 r =  s1 * s2 * 2;
			r += 0x8000;    // rounding ?
			SET_ACCUM_H((r < 0) ? 0xffff : 0, i);
			SET_ACCUM_M((INT16)(r >> 16), i);
			SET_ACCUM_L((UINT16)(r), i);
			vres[i] = ACCUM_M(i);
		}
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmulf_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmulf_scalar();
}
#endif

#if USE_SIMD
// VMULU
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000001 |
// ------------------------------------------------------
//

inline void rsp_device::ccfunc_rsp_vmulu_simd()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		VEC_GET_SCALAR_VS1(w1, i);
		VEC_GET_SCALAR_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT64 r = s1 * s2 * 2;
		r += 0x8000;    // rounding ?

		VEC_SET_ACCUM_H((UINT16)(r >> 32), i);
		VEC_SET_ACCUM_M((UINT16)(r >> 16), i);
		VEC_SET_ACCUM_L((UINT16)(r), i);

		if (r < 0)
		{
			vres[i] = 0;
		}
		else if (((INT16)(VEC_ACCUM_H(i)) ^ (INT16)(VEC_ACCUM_M(i))) < 0)
		{
			vres[i] = -1;
		}
		else
		{
			vres[i] = VEC_ACCUM_M(i);
		}
	}
	VEC_WRITEBACK_RESULT();
}

static void cfunc_rsp_vmulu_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmulu_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmulu_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT64 r = s1 * s2 * 2;
		r += 0x8000;    // rounding ?

		SET_ACCUM_H((UINT16)(r >> 32), i);
		SET_ACCUM_M((UINT16)(r >> 16), i);
		SET_ACCUM_L((UINT16)(r), i);

		if (r < 0)
		{
			vres[i] = 0;
		}
		else if (((INT16)(ACCUM_H(i)) ^ (INT16)(ACCUM_M(i))) < 0)
		{
			vres[i] = -1;
		}
		else
		{
			vres[i] = ACCUM_M(i);
		}
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmulu_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmulu_scalar();
}
#endif

#if USE_SIMD
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

inline void rsp_device::ccfunc_rsp_vmudl_simd()
{
	int op = m_rsp_state->arg0;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	__m128i unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	__m128i unpackHi = _mm_mulhi_epu16(vsReg, vtReg);
	__m128i loProduct = _mm_unpacklo_epi16(unpackLo, unpackHi);
	__m128i hiProduct = _mm_unpackhi_epi16(unpackLo, unpackHi);

	m_xv[VDREG] = m_accum_l = RSPPackHi32to16(loProduct, hiProduct);

	m_accum_m = _mm_setzero_si128();
	m_accum_h = _mm_setzero_si128();
}

static void cfunc_rsp_vmudl_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudl_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmudl_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		UINT32 s1 = (UINT32)(UINT16)w1;
		UINT32 s2 = (UINT32)(UINT16)w2;

		UINT32 r = s1 * s2;

		SET_ACCUM_H(0, i);
		SET_ACCUM_M(0, i);
		SET_ACCUM_L((UINT16)(r >> 16), i);

		vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmudl_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudl_scalar();
}
#endif

#if USE_SIMD
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

inline void rsp_device::ccfunc_rsp_vmudm_simd()
{
	int op = m_rsp_state->arg0;

	__m128i vsRegLo, vsRegHi, vtRegLo, vtRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	RSPSignExtend16to32(vsReg, &vsRegLo, &vsRegHi);
	RSPZeroExtend16to32(vtReg, &vtRegLo, &vtRegHi);

	/* Begin accumulating the products. */
	__m128i loProduct = _mm_mullo_epi32(vsRegLo, vtRegLo);
	__m128i hiProduct = _mm_mullo_epi32(vsRegHi, vtRegHi);
	m_accum_l = RSPPackLo32to16(loProduct, hiProduct);
	m_accum_m = m_xv[VDREG] = RSPPackHi32to16(loProduct, hiProduct);

	loProduct = _mm_cmplt_epi32(loProduct, _mm_setzero_si128());
	hiProduct = _mm_cmplt_epi32(hiProduct, _mm_setzero_si128());
	m_accum_h = _mm_packs_epi32(loProduct, hiProduct);
}

static void cfunc_rsp_vmudm_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudm_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmudm_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (UINT16)w2;

		INT32 r =  s1 * s2;

		SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
		SET_ACCUM_M((INT16)(r >> 16), i);
		SET_ACCUM_L((UINT16)r, i);

		vres[i] = ACCUM_M(i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmudm_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudm_scalar();
}
#endif

#if USE_SIMD
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

inline void rsp_device::ccfunc_rsp_vmudn_simd()
{
	int op = m_rsp_state->arg0;

	__m128i vsRegLo, vsRegHi, vtRegLo, vtRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	RSPZeroExtend16to32(vsReg, &vsRegLo, &vsRegHi);
	RSPSignExtend16to32(vtReg, &vtRegLo, &vtRegHi);

	/* Begin accumulating the products. */
	__m128i loProduct = _mm_mullo_epi32(vsRegLo, vtRegLo);
	__m128i hiProduct = _mm_mullo_epi32(vsRegHi, vtRegHi);
	m_xv[VDREG] = m_accum_l = RSPPackLo32to16(loProduct, hiProduct);
	m_accum_m = RSPPackHi32to16(loProduct, hiProduct);
	m_accum_h = _mm_cmplt_epi16(m_accum_m, _mm_setzero_si128());
}

static void cfunc_rsp_vmudn_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudn_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmudn_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8] = { 0 };
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (UINT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT32 r = s1 * s2;

		SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
		SET_ACCUM_M((INT16)(r >> 16), i);
		SET_ACCUM_L((UINT16)(r), i);

		vres[i] = (UINT16)(r);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmudn_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudn_scalar();
}
#endif

#if USE_SIMD
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

inline void rsp_device::ccfunc_rsp_vmudh_simd()
{
	int op = m_rsp_state->arg0;

	__m128i vaccLow, vaccHigh;
	__m128i unpackLo, unpackHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Multiply the sources, accumulate the product. */
	unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	unpackHi = _mm_mulhi_epi16(vsReg, vtReg);
	vaccHigh = _mm_unpackhi_epi16(unpackLo, unpackHi);
	vaccLow = _mm_unpacklo_epi16(unpackLo, unpackHi);

	/* Pack the accumulator and result back up. */
	m_xv[VDREG] = _mm_packs_epi32(vaccLow, vaccHigh);
	m_accum_l = _mm_setzero_si128();
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_rsp_vmudh_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudh_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmudh_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT32 r = s1 * s2;

		SET_ACCUM_H((INT16)(r >> 16), i);
		SET_ACCUM_M((UINT16)(r), i);
		SET_ACCUM_L(0, i);

		if (r < -32768) r = -32768;
		if (r >  32767) r = 32767;
		vres[i] = (INT16)(r);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmudh_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmudh_scalar();
}
#endif

#if USE_SIMD
// VMACF
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001000 |
// ------------------------------------------------------
//

inline void rsp_device::ccfunc_rsp_vmacf_simd()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		VEC_GET_SCALAR_VS1(w1, i);
		VEC_GET_SCALAR_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT32 r = s1 * s2;

		UINT64 q = (UINT64)(UINT16)VEC_ACCUM_LL(i);
		q |= (((UINT64)(UINT16)VEC_ACCUM_L(i)) << 16);
		q |= (((UINT64)(UINT16)VEC_ACCUM_M(i)) << 32);
		q |= (((UINT64)(UINT16)VEC_ACCUM_H(i)) << 48);

		q += (INT64)(r) << 17;
		VEC_SET_ACCUM_LL((UINT16)q, i);
		VEC_SET_ACCUM_L((UINT16)(q >> 16), i);
		VEC_SET_ACCUM_M((UINT16)(q >> 32), i);
		VEC_SET_ACCUM_H((UINT16)(q >> 48), i);

		vres[i] = VEC_SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
	}
	VEC_WRITEBACK_RESULT();
/*
    __m128i loProduct, hiProduct, unpackLo, unpackHi;
    __m128i vaccHigh;
    __m128i vdReg, vdRegLo, vdRegHi;

    __m128i vsReg = m_xv[VS1REG];
    __m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

    __m128i vaccLow = m_accum_l;

    // Unpack to obtain for 32-bit precision.
    RSPZeroExtend16to32(vaccLow, &vaccLow, &vaccHigh);

    // Begin accumulating the products.
    unpackLo = _mm_mullo_epi16(vsReg, vtReg);
    unpackHi = _mm_mulhi_epi16(vsReg, vtReg);
    loProduct = _mm_unpacklo_epi16(unpackLo, unpackHi);
    hiProduct = _mm_unpackhi_epi16(unpackLo, unpackHi);
    loProduct = _mm_slli_epi32(loProduct, 1);
    hiProduct = _mm_slli_epi32(hiProduct, 1);

    vdRegLo = _mm_srli_epi32(loProduct, 16);
    vdRegHi = _mm_srli_epi32(hiProduct, 16);
    vdRegLo = _mm_slli_epi32(vdRegLo, 16);
    vdRegHi = _mm_slli_epi32(vdRegHi, 16);
    vdRegLo = _mm_xor_si128(vdRegLo, loProduct);
    vdRegHi = _mm_xor_si128(vdRegHi, hiProduct);

    vaccLow = _mm_add_epi32(vaccLow, vdRegLo);
    vaccHigh = _mm_add_epi32(vaccHigh, vdRegHi);

    m_accum_l = vdReg = RSPPackLo32to16(vaccLow, vaccHigh);

    // Multiply the MSB of sources, accumulate the product.
    vdRegLo = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
    vdRegHi = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

    loProduct = _mm_srai_epi32(loProduct, 16);
    hiProduct = _mm_srai_epi32(hiProduct, 16);
    vaccLow = _mm_srai_epi32(vaccLow, 16);
    vaccHigh = _mm_srai_epi32(vaccHigh, 16);

    vaccLow = _mm_add_epi32(loProduct, vaccLow);
    vaccHigh = _mm_add_epi32(hiProduct, vaccHigh);
    vaccLow = _mm_add_epi32(vdRegLo, vaccLow);
    vaccHigh = _mm_add_epi32(vdRegHi, vaccHigh);

    // Clamp the accumulator and write it all out.
    m_xv[VDREG] = _mm_packs_epi32(vaccLow, vaccHigh);
    m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
    m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
*/
}

static void cfunc_rsp_vmacf_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmacf_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmacf_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

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

		vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmacf_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmacf_scalar();
}
#endif

#if USE_SIMD
// VMACU
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001001 |
// ------------------------------------------------------
//

inline void rsp_device::ccfunc_rsp_vmacu_simd()
{
	int op = m_rsp_state->arg0;

	__m128i loProduct, hiProduct, unpackLo, unpackHi;
	__m128i vaccHigh;
	__m128i vdReg, vdRegLo, vdRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	__m128i vaccLow = m_accum_l;

	/* Unpack to obtain for 32-bit precision. */
	RSPZeroExtend16to32(vaccLow, &vaccLow, &vaccHigh);

	/* Begin accumulating the products. */
	unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	unpackHi = _mm_mulhi_epi16(vsReg, vtReg);
	loProduct = _mm_unpacklo_epi16(unpackLo, unpackHi);
	hiProduct = _mm_unpackhi_epi16(unpackLo, unpackHi);
	loProduct = _mm_slli_epi32(loProduct, 1);
	hiProduct = _mm_slli_epi32(hiProduct, 1);

	vdRegLo = _mm_srli_epi32(loProduct, 16);
	vdRegHi = _mm_srli_epi32(hiProduct, 16);
	vdRegLo = _mm_slli_epi32(vdRegLo, 16);
	vdRegHi = _mm_slli_epi32(vdRegHi, 16);
	vdRegLo = _mm_xor_si128(vdRegLo, loProduct);
	vdRegHi = _mm_xor_si128(vdRegHi, hiProduct);

	vaccLow = _mm_add_epi32(vaccLow, vdRegLo);
	vaccHigh = _mm_add_epi32(vaccHigh, vdRegHi);

	m_accum_l = vdReg = RSPPackLo32to16(vaccLow, vaccHigh);

	/* Multiply the MSB of sources, accumulate the product. */
	vdRegLo = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
	vdRegHi = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

	loProduct = _mm_srai_epi32(loProduct, 16);
	hiProduct = _mm_srai_epi32(hiProduct, 16);
	vaccLow = _mm_srai_epi32(vaccLow, 16);
	vaccHigh = _mm_srai_epi32(vaccHigh, 16);

	vaccLow = _mm_add_epi32(loProduct, vaccLow);
	vaccHigh = _mm_add_epi32(hiProduct, vaccHigh);
	vaccLow = _mm_add_epi32(vdRegLo, vaccLow);
	vaccHigh = _mm_add_epi32(vdRegHi, vaccHigh);

	/* Clamp the accumulator and write it all out. */
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_rsp_vmacu_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmacu_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmacu_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT32 r1 = s1 * s2;
		UINT32 r2 = (UINT16)ACCUM_L(i) + ((UINT16)(r1) * 2);
		UINT32 r3 = (UINT16)ACCUM_M(i) + (UINT16)((r1 >> 16) * 2) + (UINT16)(r2 >> 16);

		SET_ACCUM_L((UINT16)(r2), i);
		SET_ACCUM_M((UINT16)(r3), i);
		SET_ACCUM_H(ACCUM_H(i) + (UINT16)(r3 >> 16) + (UINT16)(r1 >> 31), i);

		if ((INT16)ACCUM_H(i) < 0)
		{
			vres[i] = 0;
		}
		else
		{
			if (ACCUM_H(i) != 0)
			{
				vres[i] = (INT16)0xffff;
			}
			else
			{
				if ((INT16)ACCUM_M(i) < 0)
				{
					vres[i] = (INT16)0xffff;
				}
				else
				{
					vres[i] = ACCUM_M(i);
				}
			}
		}
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmacu_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmacu_scalar();
}
#endif

#if USE_SIMD
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

inline void rsp_device::ccfunc_rsp_vmadl_simd()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		VEC_GET_SCALAR_VS1(w1, i);
		VEC_GET_SCALAR_VS2(w2, i);
		UINT32 s1 = w1;
		UINT32 s2 = w2;

		UINT32 r1 = s1 * s2;
		UINT32 r2 = (UINT16)VEC_ACCUM_L(i) + (r1 >> 16);
		UINT32 r3 = (UINT16)VEC_ACCUM_M(i) + (r2 >> 16);

		VEC_SET_ACCUM_L((UINT16)r2, i);
		VEC_SET_ACCUM_M((UINT16)r3, i);
		VEC_SET_ACCUM_H(VEC_ACCUM_H(i) + (INT16)(r3 >> 16), i);

		vres[i] = VEC_SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	VEC_WRITEBACK_RESULT();

	/*__m128i vaccHigh;
	__m128i unpackHi, loProduct, hiProduct;
	__m128i vdReg, vdRegLo, vdRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	__m128i vaccLow = m_accum_l;

	// Unpack to obtain for 32-bit precision.
	RSPZeroExtend16to32(vaccLow, &vaccLow, &vaccHigh);

	// Begin accumulating the products.
	unpackHi = _mm_mulhi_epu16(vsReg, vtReg);
	loProduct = _mm_unpacklo_epi16(unpackHi, _mm_setzero_si128());
	hiProduct = _mm_unpackhi_epi16(unpackHi, _mm_setzero_si128());

	vaccLow = _mm_add_epi32(vaccLow, loProduct);
	vaccHigh = _mm_add_epi32(vaccHigh, hiProduct);
	m_accum_l = vdReg = RSPPackLo32to16(vaccLow, vaccHigh);

	// Finish accumulating whatever is left.
	vdRegLo = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
	vdRegHi = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

	vaccLow = _mm_srai_epi32(vaccLow, 16);
	vaccHigh = _mm_srai_epi32(vaccHigh, 16);
	vaccLow = _mm_add_epi32(vdRegLo, vaccLow);
	vaccHigh = _mm_add_epi32(vdRegHi, vaccHigh);

	// Clamp the accumulator and write it all out.
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
	m_xv[VDREG] = RSPClampLowToVal(vdReg, m_accum_m, m_accum_h);*/
}

static void cfunc_rsp_vmadl_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadl_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmadl_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		UINT32 s1 = w1;
		UINT32 s2 = w2;

		UINT32 r1 = s1 * s2;
		UINT32 r2 = (UINT16)ACCUM_L(i) + (r1 >> 16);
		UINT32 r3 = (UINT16)ACCUM_M(i) + (r2 >> 16);

		SET_ACCUM_L((UINT16)r2, i);
		SET_ACCUM_M((UINT16)r3, i);
		SET_ACCUM_H(ACCUM_H(i) + (INT16)(r3 >> 16), i);

		vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmadl_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadl_scalar();
}
#endif

#if USE_SIMD
// VMADM
//

inline void rsp_device::ccfunc_rsp_vmadm_simd()
{
	int op = m_rsp_state->arg0;

	__m128i vaccLow, vaccHigh, loProduct, hiProduct;
	__m128i vsRegLo, vsRegHi, vtRegLo, vtRegHi, vdRegLo, vdRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	RSPSignExtend16to32(vsReg, &vsRegLo, &vsRegHi);
	RSPZeroExtend16to32(vtReg, &vtRegLo, &vtRegHi);
	RSPZeroExtend16to32(m_accum_l, &vaccLow, &vaccHigh);

	/* Begin accumulating the products. */
	loProduct = _mm_mullo_epi32(vsRegLo, vtRegLo);
	hiProduct = _mm_mullo_epi32(vsRegHi, vtRegHi);

	vdRegLo = _mm_srli_epi32(loProduct, 16);
	vdRegHi = _mm_srli_epi32(hiProduct, 16);
	vdRegLo = _mm_slli_epi32(vdRegLo, 16);
	vdRegHi = _mm_slli_epi32(vdRegHi, 16);
	vdRegLo = _mm_xor_si128(vdRegLo, loProduct);
	vdRegHi = _mm_xor_si128(vdRegHi, hiProduct);
	vaccLow = _mm_add_epi32(vaccLow, vdRegLo);
	vaccHigh = _mm_add_epi32(vaccHigh, vdRegHi);

	m_accum_l = m_xv[VDREG] = RSPPackLo32to16(vaccLow, vaccHigh);

	/* Multiply the MSB of sources, accumulate the product. */
	vdRegLo = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
	vdRegHi = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

	loProduct = _mm_srai_epi32(loProduct, 16);
	hiProduct = _mm_srai_epi32(hiProduct, 16);
	vaccLow = _mm_srai_epi32(vaccLow, 16);
	vaccHigh = _mm_srai_epi32(vaccHigh, 16);

	vaccLow = _mm_add_epi32(loProduct, vaccLow);
	vaccHigh = _mm_add_epi32(hiProduct, vaccHigh);
	vaccLow = _mm_add_epi32(vdRegLo, vaccLow);
	vaccHigh = _mm_add_epi32(vdRegHi, vaccHigh);

	/* Clamp the accumulator and write it all out. */
	m_xv[VDREG] = _mm_packs_epi32(vaccLow, vaccHigh);
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_rsp_vmadm_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadm_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmadm_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		UINT32 s1 = (INT32)(INT16)w1;
		UINT32 s2 = (UINT16)w2;

		UINT32 r1 = s1 * s2;
		UINT32 r2 = (UINT16)ACCUM_L(i) + (UINT16)(r1);
		UINT32 r3 = (UINT16)ACCUM_M(i) + (r1 >> 16) + (r2 >> 16);

		SET_ACCUM_L((UINT16)r2, i);
		SET_ACCUM_M((UINT16)r3, i);
		SET_ACCUM_H((UINT16)ACCUM_H(i) + (UINT16)(r3 >> 16), i);
		if ((INT32)(r1) < 0)
		{
			SET_ACCUM_H((UINT16)ACCUM_H(i) - 1, i);
		}

		vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmadm_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadm_scalar();
}
#endif

#if USE_SIMD
// VMADN
//

inline void rsp_device::ccfunc_rsp_vmadn_simd()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		VEC_GET_SCALAR_VS1(w1, i);
		VEC_GET_SCALAR_VS2(w2, i);
		INT32 s1 = (UINT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		UINT64 q = (UINT64)VEC_ACCUM_LL(i);
		q |= (((UINT64)VEC_ACCUM_L(i)) << 16);
		q |= (((UINT64)VEC_ACCUM_M(i)) << 32);
		q |= (((UINT64)VEC_ACCUM_H(i)) << 48);
		q += (INT64)(s1*s2) << 16;

		VEC_SET_ACCUM_LL((UINT16)q, i);
		VEC_SET_ACCUM_L((UINT16)(q >> 16), i);
		VEC_SET_ACCUM_M((UINT16)(q >> 32), i);
		VEC_SET_ACCUM_H((UINT16)(q >> 48), i);

		vres[i] = VEC_SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	VEC_WRITEBACK_RESULT();
}
/*INLINE void cfunc_rsp_vmadn_simd(void *param)
{
    rsp_state *rsp = (rsp_state*)param;
    int op = m_rsp_state->arg0;

    __m128i vaccLow, vaccHigh, loProduct, hiProduct;
    __m128i vsRegLo, vsRegHi, vtRegLo, vtRegHi, vdRegLo, vdRegHi;

    __m128i vsReg = m_xv[VS1REG];
    __m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

    vaccLow = m_accum_l;

    RSPZeroExtend16to32(vsReg, &vsRegLo, &vsRegHi);
    RSPSignExtend16to32(vtReg, &vtRegLo, &vtRegHi);
    RSPZeroExtend16to32(vaccLow, &vaccLow, &vaccHigh);

    // Begin accumulating the products.
    loProduct = _mm_mullo_epi32(vsRegLo, vtRegLo);
    hiProduct = _mm_mullo_epi32(vsRegHi, vtRegHi);

    vdRegLo = _mm_srli_epi32(loProduct, 16);
    vdRegHi = _mm_srli_epi32(hiProduct, 16);
    vdRegLo = _mm_slli_epi32(vdRegLo, 16);
    vdRegHi = _mm_slli_epi32(vdRegHi, 16);
    vdRegLo = _mm_xor_si128(vdRegLo, loProduct);
    vdRegHi = _mm_xor_si128(vdRegHi, hiProduct);

    vaccLow = _mm_add_epi32(vaccLow, vdRegLo);
    vaccHigh = _mm_add_epi32(vaccHigh, vdRegHi);

    m_accum_l = RSPPackLo32to16(vaccLow, vaccHigh);

    // Multiply the MSB of sources, accumulate the product.
    vdRegLo = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
    vdRegHi = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

    loProduct = _mm_srai_epi32(loProduct, 16);
    hiProduct = _mm_srai_epi32(hiProduct, 16);
    vaccLow = _mm_srai_epi32(vaccLow, 16);
    vaccHigh = _mm_srai_epi32(vaccHigh, 16);

    vaccLow = _mm_add_epi32(loProduct, vaccLow);
    vaccHigh = _mm_add_epi32(hiProduct, vaccHigh);
    vaccLow = _mm_add_epi32(vdRegLo, vaccLow);
    vaccHigh = _mm_add_epi32(vdRegHi, vaccHigh);

    // Clamp the accumulator and write it all out.
    m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
    m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
    m_xv[VDREG] = RSPClampLowToVal(m_accum_l, m_accum_m, m_accum_h);
}*/

static void cfunc_rsp_vmadn_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadn_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmadn_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (UINT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		UINT64 q = (UINT64)ACCUM_LL(i);
		q |= (((UINT64)ACCUM_L(i)) << 16);
		q |= (((UINT64)ACCUM_M(i)) << 32);
		q |= (((UINT64)ACCUM_H(i)) << 48);
		q += (INT64)(s1*s2) << 16;

		SET_ACCUM_LL((UINT16)q, i);
		SET_ACCUM_L((UINT16)(q >> 16), i);
		SET_ACCUM_M((UINT16)(q >> 32), i);
		SET_ACCUM_H((UINT16)(q >> 48), i);

		vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmadn_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadn_scalar();
}
#endif

#if USE_SIMD
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

inline void rsp_device::ccfunc_rsp_vmadh_simd()
{
	int op = m_rsp_state->arg0;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	__m128i vaccLow = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
	__m128i vaccHigh = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

	/* Multiply the sources, accumulate the product. */
	__m128i unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	__m128i unpackHi = _mm_mulhi_epi16(vsReg, vtReg);
	__m128i loProduct = _mm_unpacklo_epi16(unpackLo, unpackHi);
	__m128i hiProduct = _mm_unpackhi_epi16(unpackLo, unpackHi);
	vaccLow = _mm_add_epi32(vaccLow, loProduct);
	vaccHigh = _mm_add_epi32(vaccHigh, hiProduct);

	/* Pack the accumulator and result back up. */
	m_xv[VDREG] = _mm_packs_epi32(vaccLow, vaccHigh);
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_rsp_vmadh_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadh_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmadh_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT32 accum = (UINT32)(UINT16)ACCUM_M(i);
		accum |= ((UINT32)((UINT16)ACCUM_H(i))) << 16;
		accum += s1*s2;

		SET_ACCUM_H((UINT16)(accum >> 16), i);
		SET_ACCUM_M((UINT16)accum, i);

		vres[i] = SATURATE_ACCUM1(i, 0x8000, 0x7fff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmadh_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmadh_scalar();
}
#endif

#if USE_SIMD
// VADD
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010000 |
// ------------------------------------------------------
//
// Adds two vector registers and carry flag, the result is saturated to 32767

inline void rsp_device::ccfunc_rsp_vadd_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuffled = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i carry = _mm_and_si128(m_xvflag[CARRY], vec_flagmask);
	m_accum_l = _mm_add_epi16(_mm_add_epi16(m_xv[VS1REG], shuffled), carry);

	__m128i addvec = _mm_adds_epi16(m_xv[VS1REG], shuffled);

	carry = _mm_and_si128(carry, _mm_xor_si128(_mm_cmpeq_epi16(addvec, vec_32767), vec_neg1));
	carry = _mm_and_si128(carry, _mm_xor_si128(_mm_cmpeq_epi16(addvec, vec_n32768), vec_neg1));

	m_xv[VDREG] = _mm_add_epi16(addvec, carry);

	m_xvflag[ZERO] = vec_zero;
	m_xvflag[CARRY] = vec_zero;
}

static void cfunc_rsp_vadd_simd(void *param)
{
	((rsp_Device *)param)->ccfunc_rsp_vadd_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vadd_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8] = { 0 };
	for (int i = 0; i < 8; i++)
	{
		INT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;
		INT32 r = s1 + s2 + (((CARRY_FLAG(i)) != 0) ? 1 : 0);

		SET_ACCUM_L((INT16)(r), i);

		if (r > 32767) r = 32767;
		if (r < -32768) r = -32768;
		vres[i] = (INT16)(r);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vadd_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vadd_scalar();
}
#endif

#if USE_SIMD
// VSUB
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010001 |
// ------------------------------------------------------
//
// Subtracts two vector registers and carry flag, the result is saturated to -32768
// TODO: check VS2REG == VDREG

inline void rsp_device::ccfunc_rsp_vsub_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuffled = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i carry = _mm_and_si128(m_xvflag[CARRY], vec_flagmask);
	__m128i unsat = _mm_sub_epi16(m_xv[VS1REG], shuffled);

	__m128i vs2neg = _mm_cmplt_epi16(shuffled, vec_zero);
	__m128i vs2pos = _mm_cmpeq_epi16(vs2neg, vec_zero);

	__m128i saturated = _mm_subs_epi16(m_xv[VS1REG], shuffled);
	__m128i carry_mask = _mm_cmpeq_epi16(unsat, saturated);
	carry_mask = _mm_and_si128(vs2neg, carry_mask);

	vs2neg = _mm_and_si128(carry_mask, carry);
	vs2pos = _mm_and_si128(vs2pos, carry);
	__m128i dest_carry = _mm_or_si128(vs2neg, vs2pos);
	m_xv[VDREG] = _mm_subs_epi16(saturated, dest_carry);

	m_accum_l = _mm_sub_epi16(unsat, carry);

	m_xvflag[ZERO] = _mm_setzero_si128();
	m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_rsp_vsub_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vsub_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vsub_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;
		INT32 r = s1 - s2 - (((CARRY_FLAG(i)) != 0) ? 1 : 0);

		SET_ACCUM_L((INT16)(r), i);

		if (r > 32767) r = 32767;
		if (r < -32768) r = -32768;

		vres[i] = (INT16)(r);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vsub_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vsub_scalar();
}
#endif

#if USE_SIMD
// VABS
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010011 |
// ------------------------------------------------------
//
// Changes the sign of source register 2 if source register 1 is negative and stores the result to destination register

inline void rsp_device::ccfunc_rsp_vabs_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf2 = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i negs2 = _mm_sub_epi16(_mm_setzero_si128(), shuf2);
	__m128i s2_n32768 = _mm_cmpeq_epi16(shuf2, vec_n32768);
	__m128i s1_lz = _mm_cmplt_epi16(m_xv[VS1REG], _mm_setzero_si128());

	__m128i result_gz = _mm_and_si128(shuf2, _mm_cmpgt_epi16(m_xv[VS1REG], _mm_setzero_si128()));
	__m128i result_n32768 = _mm_and_si128(s1_lz, _mm_and_si128(vec_32767, s2_n32768));
	__m128i result_negs2 = _mm_and_si128(s1_lz, _mm_and_si128(negs2, _mm_xor_si128(s2_n32768, vec_neg1)));
	m_xv[VDREG] = m_accum_l = _mm_or_si128(result_gz, _mm_or_si128(result_n32768, result_negs2));
}

static void cfunc_rsp_vabs_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vabs_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vabs_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);

		if (s1 < 0)
		{
			if (s2 == -32768)
			{
				vres[i] = 32767;
			}
			else
			{
				vres[i] = -s2;
			}
		}
		else if (s1 > 0)
		{
			vres[i] = s2;
		}
		else
		{
			vres[i] = 0;
		}

		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vabs_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vabs_scalar();
}
#endif

#if USE_SIMD
// VADDC
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010100 |
// ------------------------------------------------------
//
// Adds two vector registers, the carry out is stored into carry register
// TODO: check VS2REG = VDREG

inline void rsp_device::ccfunc_rsp_vaddc_simd()
{
	int op = m_rsp_state->arg0;

	VEC_CLEAR_ZERO_FLAGS();
	VEC_CLEAR_CARRY_FLAGS();

	__m128i shuf2 = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i vec7531 = _mm_and_si128(m_xv[VS1REG], vec_lomask);
	__m128i vec6420 = _mm_srli_epi32(m_xv[VS1REG], 16);
	__m128i shuf7531 = _mm_and_si128(shuf2, vec_lomask);
	__m128i shuf6420 = _mm_srli_epi32(shuf2, 16);
	__m128i sum7531 = _mm_add_epi32(vec7531, shuf7531);
	__m128i sum6420 = _mm_add_epi32(vec6420, shuf6420);

	__m128i over7531 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum7531, _mm_setzero_si128()), vec_neg1), vec_himask);
	__m128i over6420 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum6420, _mm_setzero_si128()), vec_neg1), vec_himask);

	sum7531 = _mm_and_si128(sum7531, vec_lomask);
	sum6420 = _mm_and_si128(sum6420, vec_lomask);

	m_xvflag[CARRY] = _mm_or_si128(over6420, _mm_srli_epi32(over7531, 16));
	m_accum_l = m_xv[VDREG] = _mm_or_si128(_mm_slli_epi32(sum6420, 16), sum7531);
}

static void cfunc_rsp_vaddc_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vaddc_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vaddc_scalar()
{
	int op = m_rsp_state->arg0;

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	INT16 vres[8] = { 0 };
	for (int i = 0; i < 8; i++)
	{
		INT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (UINT32)(UINT16)w1;
		INT32 s2 = (UINT32)(UINT16)w2;
		INT32 r = s1 + s2;

		vres[i] = (INT16)r;
		SET_ACCUM_L((INT16)r, i);

		if (r & 0xffff0000)
		{
			SET_CARRY_FLAG(i);
		}
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vaddc_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vaddc_scalar();
}
#endif

#if USE_SIMD
// VSUBC
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010101 |
// ------------------------------------------------------
//
// Subtracts two vector registers, the carry out is stored into carry register
// TODO: check VS2REG = VDREG

inline void rsp_device::ccfunc_rsp_vsubc_simd()
{
	int op = m_rsp_state->arg0;

	VEC_CLEAR_ZERO_FLAGS();
	VEC_CLEAR_CARRY_FLAGS();

	__m128i shuf2 = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i vec7531 = _mm_and_si128(m_xv[VS1REG], vec_lomask);
	__m128i vec6420 = _mm_srli_epi32(m_xv[VS1REG], 16);
	__m128i shuf7531 = _mm_and_si128(shuf2, vec_lomask);
	__m128i shuf6420 = _mm_srli_epi32(shuf2, 16);
	__m128i sum7531 = _mm_sub_epi32(vec7531, shuf7531);
	__m128i sum6420 = _mm_sub_epi32(vec6420, shuf6420);

	__m128i over7531 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum7531, _mm_setzero_si128()), vec_neg1), vec_himask);
	__m128i over6420 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum6420, _mm_setzero_si128()), vec_neg1), vec_himask);
	sum7531 = _mm_and_si128(sum7531, vec_lomask);
	sum6420 = _mm_and_si128(sum6420, vec_lomask);
	__m128i zero7531 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum7531, _mm_setzero_si128()), vec_neg1), vec_lomask);
	__m128i zero6420 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum6420, _mm_setzero_si128()), vec_neg1), vec_lomask);

	m_xvflag[CARRY] = _mm_or_si128(over6420, _mm_srli_epi32(over7531, 16));
	m_xvflag[ZERO] = _mm_or_si128(_mm_slli_epi32(zero6420, 16), zero7531);

	m_accum_l = m_xv[VDREG] = _mm_or_si128(_mm_slli_epi32(sum6420, 16), sum7531);
}

static void cfunc_rsp_vsubc_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vsubc_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vsubc_scalar()
{
	int op = m_rsp_state->arg0;


	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 w1, w2;
		SCALAR_GET_VS1(w1, i);
		SCALAR_GET_VS2(w2, i);
		INT32 s1 = (UINT32)(UINT16)w1;
		INT32 s2 = (UINT32)(UINT16)w2;
		INT32 r = s1 - s2;

		vres[i] = (INT16)(r);
		SET_ACCUM_L((UINT16)r, i);

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
}

static void cfunc_rsp_vsubc_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vsubc_scalar();
}
#endif

#if USE_SIMD
// VSAW
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 011101 |
// ------------------------------------------------------
//
// Stores high, middle or low slice of accumulator to destination vector

inline void rsp_device::ccfunc_rsp_vsaw_simd()
{
	int op = m_rsp_state->arg0;

	switch (EL)
	{
		case 0x08:      // VSAWH
		{
			m_xv[VDREG] = m_accum_h;
			break;
		}
		case 0x09:      // VSAWM
		{
			m_xv[VDREG] = m_accum_m;
			break;
		}
		case 0x0a:      // VSAWL
		{
			m_xv[VDREG] = m_accum_l;
			break;
		}
		default:    fatalerror("RSP: VSAW: el = %d\n", EL);
	}
}

static void cfunc_rsp_vsaw_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vsaw_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vsaw_scalar()
{
	int op = m_rsp_state->arg0;

	switch (EL)
	{
		case 0x08:      // VSAWH
		{
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(VDREG, i) = ACCUM_H(i);
			}
			break;
		}
		case 0x09:      // VSAWM
		{
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(VDREG, i) = ACCUM_M(i);
			}
			break;
		}
		case 0x0a:      // VSAWL
		{
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(VDREG, i) = ACCUM_L(i);
			}
			break;
		}
		default:    fatalerror("RSP: VSAW: el = %d\n", EL);
	}
}

static void cfunc_rsp_vsaw_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vsaw_scalar();
}
#endif

#if USE_SIMD
// VLT
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100000 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are less than VS2
// Moves the element in VS2 to destination vector

inline void rsp_device::ccfunc_rsp_vlt_simd()
{
	int op = m_rsp_state->arg0;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i zc_mask = _mm_and_si128(m_xvflag[ZERO], m_xvflag[CARRY]);
	__m128i lt_mask = _mm_cmplt_epi16(m_xv[VS1REG], shuf);
	__m128i eq_mask = _mm_and_si128(_mm_cmpeq_epi16(m_xv[VS1REG], shuf), zc_mask);

	m_xvflag[COMPARE] = _mm_or_si128(lt_mask, eq_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void void cfunc_rsp_vlt_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vlt_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vlt_scalar()
{
	int op = m_rsp_state->arg0;

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);

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
			vres[i] = s1;
		}
		else
		{
			vres[i] = s2;
		}

		SET_ACCUM_L(vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vlt_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vlt_scalar();
}
#endif

#if USE_SIMD
// VEQ
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100001 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are equal with VS2
// Moves the element in VS2 to destination vector

inline void rsp_device::ccfunc_rsp_veq_simd()
{
	int op = m_rsp_state->arg0;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i zero_mask = _mm_cmpeq_epi16(m_xvflag[ZERO], _mm_setzero_si128());
	__m128i eq_mask = _mm_cmpeq_epi16(m_xv[VS1REG], shuf);

	m_xvflag[COMPARE] = _mm_and_si128(zero_mask, eq_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_rsp_veq_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_veq_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_veq_scalar()
{
	int op = m_rsp_state->arg0;

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);

		if ((s1 == s2) && ZERO_FLAG(i) == 0)
		{
			SET_COMPARE_FLAG(i);
			vres[i] = s1;
		}
		else
		{
			vres[i] = s2;
		}

		SET_ACCUM_L(vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_rsp_veq_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_veq_scalar();
}
#endif

#if USE_SIMD
// VNE
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100010 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are not equal with VS2
// Moves the element in VS2 to destination vector

inline void rsp_device::ccfunc_rsp_vne_simd()
{
	int op = m_rsp_state->arg0;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i neq_mask = _mm_xor_si128(_mm_cmpeq_epi16(m_xv[VS1REG], shuf), vec_neg1);

	m_xvflag[COMPARE] = _mm_or_si128(m_xvflag[ZERO], neq_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_rsp_vne_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vne_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vne_scalar()
{
	int op = m_rsp_state->arg0;

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);

		if (s1 != s2 || ZERO_FLAG(i) != 0)
		{
			SET_COMPARE_FLAG(i);
			vres[i] = s1;
		}
		else
		{
			vres[i] = s2;
		}

		SET_ACCUM_L(vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vne_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vne_scalar();
}
#endif

#if USE_SIMD
// VGE
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100011 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are greater or equal with VS2
// Moves the element in VS2 to destination vector

inline void rsp_device::ccfunc_rsp_vge_simd()
{
	int op = m_rsp_state->arg0;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i zero_mask = _mm_cmpeq_epi16(m_xvflag[ZERO], _mm_setzero_si128());
	__m128i carry_mask = _mm_cmpeq_epi16(m_xvflag[CARRY], _mm_setzero_si128());
	__m128i flag_mask = _mm_or_si128(zero_mask, carry_mask);
	__m128i eq_mask = _mm_and_si128(_mm_cmpeq_epi16(m_xv[VS1REG], shuf), flag_mask);
	__m128i gt_mask = _mm_cmpgt_epi16(m_xv[VS1REG], shuf);
	m_xvflag[COMPARE] = _mm_or_si128(eq_mask, gt_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_rsp_vge_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vge_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vge_scalar()
{
	int op = m_rsp_state->arg0;

	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP2_FLAGS();

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		if ((s1 == s2 && (ZERO_FLAG(i) == 0 || CARRY_FLAG(i) == 0)) || s1 > s2)
		{
			SET_COMPARE_FLAG(i);
			vres[i] = s1;
		}
		else
		{
			vres[i] = s2;
		}

		SET_ACCUM_L(vres[i], i);
	}

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vge_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vge_scalar();
}
#endif

#if USE_SIMD
// VCL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100100 |
// ------------------------------------------------------
//
// Vector clip low

inline void rsp_device::ccfunc_rsp_vcl_simd()
{
	int op = m_rsp_state->arg0;
	INT16 vres[8];

	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		VEC_GET_SCALAR_VS1(s1, i);
		VEC_GET_SCALAR_VS2(s2, i);

		if (VEC_CARRY_FLAG(i) != 0)
		{
			if (VEC_ZERO_FLAG(i) != 0)
			{
				if (VEC_COMPARE_FLAG(i) != 0)
				{
					VEC_SET_ACCUM_L(-(UINT16)s2, i);
				}
				else
				{
					VEC_SET_ACCUM_L(s1, i);
				}
			}
			else//VEC_ZERO_FLAG(i)==0
			{
				if (VEC_CLIP1_FLAG(i) != 0)
				{
					if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
					{//proper fix for Harvest Moon 64, r4
						VEC_SET_ACCUM_L(s1, i);
						VEC_CLEAR_COMPARE_FLAG(i);
					}
					else
					{
						VEC_SET_ACCUM_L(-((UINT16)s2), i);
						VEC_SET_COMPARE_FLAG(i);
					}
				}
				else
				{
					if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) != 0)
					{
						VEC_SET_ACCUM_L(s1, i);
						VEC_CLEAR_COMPARE_FLAG(i);
					}
					else
					{
						VEC_SET_ACCUM_L(-((UINT16)s2), i);
						VEC_SET_COMPARE_FLAG(i);
					}
				}
			}
		}
		else//VEC_CARRY_FLAG(i)==0
		{
			if (VEC_ZERO_FLAG(i) != 0)
			{
				if (VEC_CLIP2_FLAG(i) != 0)
				{
					VEC_SET_ACCUM_L(s2, i);
				}
				else
				{
					VEC_SET_ACCUM_L(s1, i);
				}
			}
			else
			{
				if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
				{
					VEC_SET_ACCUM_L(s2, i);
					VEC_SET_CLIP2_FLAG(i);
				}
				else
				{
					VEC_SET_ACCUM_L(s1, i);
					VEC_CLEAR_CLIP2_FLAG(i);
				}
			}
		}
		vres[i] = VEC_ACCUM_L(i);
	}
	VEC_CLEAR_ZERO_FLAGS();
	VEC_CLEAR_CARRY_FLAGS();
	VEC_CLEAR_CLIP1_FLAGS();
	VEC_WRITEBACK_RESULT();
}

static void cfunc_rsp_vcl_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vcl_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vcl_scalar()
{
	int op = m_rsp_state->arg0;
	INT16 vres[8];

	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);

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
			else//ZERO_FLAG(i)==0
			{
				if (CLIP1_FLAG(i) != 0)
				{
					if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
					{//proper fix for Harvest Moon 64, r4
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
		else//CARRY_FLAG(i)==0
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
		vres[i] = ACCUM_L(i);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	CLEAR_CLIP1_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vcl_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vcl_scalar();
}
#endif

#if USE_SIMD
// VCH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100101 |
// ------------------------------------------------------
//
// Vector clip high

inline void rsp_device::ccfunc_rsp_vch_simd()
{
	int op = m_rsp_state->arg0;

	VEC_CLEAR_CARRY_FLAGS();
	VEC_CLEAR_COMPARE_FLAGS();
	VEC_CLEAR_CLIP1_FLAGS();
	VEC_CLEAR_ZERO_FLAGS();
	VEC_CLEAR_CLIP2_FLAGS();

#if 0
	// Compare flag
	// flag[1] bit [0- 7] set if (s1 ^ s2) < 0 && (s1 + s2) <= 0)
	// flag[1] bit [0- 7] set if (s1 ^ s2) >= 0 && (s2 < 0)

	// flag[1] bit [8-15] set if (s1 ^ s2) < 0 && (s2 < 0)
	// flag[1] bit [8-15] set if (s1 ^ s2) >= 0 && (s1 - s2) >= 0

	// Carry flag
	// flag[0] bit [0- 7] set if (s1 ^ s2) < 0

	// Zero flag
	// flag[0] bit [8-15] set if (s1 ^ s2) < 0  && (s1 + s2) != 0 && (s1 != ~s2)
	// flag[0] bit [8-15] set if (s1 ^ s2) >= 0 && (s1 - s2) != 0 && (s1 != ~s2)

	// flag[2] bit [0- 7] set if (s1 ^ s2) < 0 && (s1 + s2) == -1

	// accum set to -s2 if (s1 ^ s2) < 0 && (s1 + s2) <= 0)
	// accum set to -s2 if (s1 ^ s2) >= 0 && (s1 - s2) >= 0

	// accum set to s1 if (s1 ^ s2) < 0 && (s1 + s2) > 0)
	// accum set to s1 if (s1 ^ s2) >= 0 && (s1 - s2) < 0

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i s1_xor_s2 = _mm_xor_si128(m_xv[VS1REG], shuf);
	__m128i s1_plus_s2 = _mm_add_epi16(m_xv[VS1REG], shuf);
	__m128i s1_sub_s2 = _mm_sub_epi16(m_xv[VS1REG], shuf);
	__m128i s2_neg = _mm_xor_si128(shuf, vec_neg1);

	__m128i s2_lz = _mm_cmplt_epi16(shuf, _mm_setzero_si128());
	__m128i s1s2_xor_lz = _mm_cmplt_epi16(s1_xor_s2, _mm_setzero_si128());
	__m128i s1s2_xor_gez = _mm_xor_si128(s1s2_xor_lz, vec_neg1);
	__m128i s1s2_plus_nz = _mm_xor_si128(_mm_cmpeq_epi16(s1_plus_s2, _mm_setzero_si128()), vec_neg1);
	__m128i s1s2_plus_gz = _mm_cmpgt_epi16(s1_plus_s2, _mm_setzero_si128());
	__m128i s1s2_plus_lez = _mm_xor_si128(s1s2_plus_gz, vec_neg1);
	__m128i s1s2_plus_n1 = _mm_cmpeq_epi16(s1_plus_s2, vec_neg1);
	__m128i s1s2_sub_nz = _mm_xor_si128(_mm_cmpeq_epi16(s1_sub_s2, _mm_setzero_si128()), vec_neg1);
	__m128i s1s2_sub_lz = _mm_cmplt_epi16(s1_sub_s2, _mm_setzero_si128());
	__m128i s1s2_sub_gez = _mm_xor_si128(s1s2_sub_lz, vec_neg1);
	__m128i s1_nens2 = _mm_xor_si128(_mm_cmpeq_epi16(m_xv[VS1REG], s2_neg), vec_neg1);

	__m128i ext_mask = _mm_and_si128(_mm_and_si128(s1s2_xor_lz, s1s2_plus_n1), vec_flagmask);
	m_flag[2] |= _mm_extract_epi16(ext_mask, 0) << 0;
	m_flag[2] |= _mm_extract_epi16(ext_mask, 1) << 1;
	m_flag[2] |= _mm_extract_epi16(ext_mask, 2) << 2;
	m_flag[2] |= _mm_extract_epi16(ext_mask, 3) << 3;
	m_flag[2] |= _mm_extract_epi16(ext_mask, 4) << 4;
	m_flag[2] |= _mm_extract_epi16(ext_mask, 5) << 5;
	m_flag[2] |= _mm_extract_epi16(ext_mask, 6) << 6;
	m_flag[2] |= _mm_extract_epi16(ext_mask, 7) << 7;

	__m128i carry_mask = _mm_and_si128(s1s2_xor_lz, vec_flagmask);
	m_flag[0] |= _mm_extract_epi16(carry_mask, 0) << 0;
	m_flag[0] |= _mm_extract_epi16(carry_mask, 1) << 1;
	m_flag[0] |= _mm_extract_epi16(carry_mask, 2) << 2;
	m_flag[0] |= _mm_extract_epi16(carry_mask, 3) << 3;
	m_flag[0] |= _mm_extract_epi16(carry_mask, 4) << 4;
	m_flag[0] |= _mm_extract_epi16(carry_mask, 5) << 5;
	m_flag[0] |= _mm_extract_epi16(carry_mask, 6) << 6;
	m_flag[0] |= _mm_extract_epi16(carry_mask, 7) << 7;

	__m128i z0_mask = _mm_and_si128(_mm_and_si128(s1s2_xor_gez, s1s2_sub_nz), s1_nens2);
	__m128i z1_mask = _mm_and_si128(_mm_and_si128(s1s2_xor_lz, s1s2_plus_nz), s1_nens2);
	__m128i z_mask = _mm_and_si128(_mm_or_si128(z0_mask, z1_mask), vec_flagmask);
	z_mask = _mm_and_si128(_mm_or_si128(z_mask, _mm_srli_epi32(z_mask, 15)), vec_shiftmask2);
	z_mask = _mm_and_si128(_mm_or_si128(z_mask, _mm_srli_epi64(z_mask, 30)), vec_shiftmask4);
	z_mask = _mm_or_si128(z_mask, _mm_srli_si128(z_mask, 7));
	z_mask = _mm_or_si128(z_mask, _mm_srli_epi16(z_mask, 4));
	m_flag[0] |= (_mm_extract_epi16(z_mask, 0) << 8) & 0x00ff00;

	__m128i f0_mask = _mm_and_si128(_mm_or_si128(_mm_and_si128(s1s2_xor_gez, s2_lz),         _mm_and_si128(s1s2_xor_lz, s1s2_plus_lez)), vec_flagmask);
	__m128i f8_mask = _mm_and_si128(_mm_or_si128(_mm_and_si128(s1s2_xor_gez, s1s2_sub_gez),  _mm_and_si128(s1s2_xor_lz, s2_lz)), vec_flagmask);
	f0_mask = _mm_and_si128(f0_mask, vec_flagmask);
	f8_mask = _mm_and_si128(f8_mask, vec_flagmask);
	m_flag[1] |= _mm_extract_epi16(f0_mask, 0) << 0;
	m_flag[1] |= _mm_extract_epi16(f0_mask, 1) << 1;
	m_flag[1] |= _mm_extract_epi16(f0_mask, 2) << 2;
	m_flag[1] |= _mm_extract_epi16(f0_mask, 3) << 3;
	m_flag[1] |= _mm_extract_epi16(f0_mask, 4) << 4;
	m_flag[1] |= _mm_extract_epi16(f0_mask, 5) << 5;
	m_flag[1] |= _mm_extract_epi16(f0_mask, 6) << 6;
	m_flag[1] |= _mm_extract_epi16(f0_mask, 7) << 7;

	m_flag[1] |= _mm_extract_epi16(f8_mask, 0) << 8;
	m_flag[1] |= _mm_extract_epi16(f8_mask, 1) << 9;
	m_flag[1] |= _mm_extract_epi16(f8_mask, 2) << 10;
	m_flag[1] |= _mm_extract_epi16(f8_mask, 3) << 11;
	m_flag[1] |= _mm_extract_epi16(f8_mask, 4) << 12;
	m_flag[1] |= _mm_extract_epi16(f8_mask, 5) << 13;
	m_flag[1] |= _mm_extract_epi16(f8_mask, 6) << 14;
	m_flag[1] |= _mm_extract_epi16(f8_mask, 7) << 15;
#endif
	INT16 vres[8];
	UINT32 vce = 0;
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		VEC_GET_SCALAR_VS1(s1, i);
		VEC_GET_SCALAR_VS2(s2, i);

		if ((s1 ^ s2) < 0)
		{
			vce = (s1 + s2 == -1);
			VEC_SET_CARRY_FLAG(i);
			if (s2 < 0)
			{
				VEC_SET_CLIP2_FLAG(i);
			}

			if ((s1 + s2) <= 0)
			{
				VEC_SET_COMPARE_FLAG(i);
				vres[i] = -((UINT16)s2);
			}
			else
			{
				vres[i] = s1;
			}

			if ((s1 + s2) != 0 && s1 != ~s2)
			{
				VEC_SET_ZERO_FLAG(i);
			}
		}//sign
		else
		{
			vce = 0;
			if (s2 < 0)
			{
				VEC_SET_COMPARE_FLAG(i);
			}
			if ((s1 - s2) >= 0)
			{
				VEC_SET_CLIP2_FLAG(i);
				vres[i] = s2;
			}
			else
			{
				vres[i] = s1;
			}

			if ((s1 - s2) != 0 && s1 != ~s2)
			{
				VEC_SET_ZERO_FLAG(i);
			}
		}
		if (vce)
		{
			VEC_SET_CLIP1_FLAG(i);
		}
		VEC_SET_ACCUM_L(vres[i], i);
	}
	VEC_WRITEBACK_RESULT();
}

static void cfunc_rsp_vch_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vch_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vch_scalar()
{
	int op = m_rsp_state->arg0;

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();

	INT16 vres[8];
	UINT32 vce = 0;
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);

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
				vres[i] = -((UINT16)s2);
			}
			else
			{
				vres[i] = s1;
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
				vres[i] = s2;
			}
			else
			{
				vres[i] = s1;
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
		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vch_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vch_scalar();
}
#endif

#if USE_SIMD
// VCR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100110 |
// ------------------------------------------------------
//
// Vector clip reverse

inline void rsp_device::ccfunc_rsp_vcr_simd()
{
	int op = m_rsp_state->arg0;

	VEC_CLEAR_CARRY_FLAGS();
	VEC_CLEAR_COMPARE_FLAGS();
	VEC_CLEAR_CLIP1_FLAGS();
	VEC_CLEAR_ZERO_FLAGS();
	VEC_CLEAR_CLIP2_FLAGS();

#if 0
	// flag[1] bit [0- 7] set if (s1 ^ s2) < 0 && (s1 + s2) <= 0)
	// flag[1] bit [0- 7] set if (s1 ^ s2) >= 0 && (s2 < 0)

	// flag[1] bit [8-15] set if (s1 ^ s2) < 0 && (s2 < 0)
	// flag[1] bit [8-15] set if (s1 ^ s2) >= 0 && (s1 - s2) >= 0

	// accum set to ~s2 if (s1 ^ s2) < 0 && (s1 + s2) <= 0)
	// accum set to ~s2 if (s1 ^ s2) >= 0 && (s1 - s2) >= 0

	// accum set to s1 if (s1 ^ s2) < 0 && (s1 + s2) > 0)
	// accum set to s1 if (s1 ^ s2) >= 0 && (s1 - s2) < 0
	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i s1_xor_s2 = _mm_xor_si128(m_xv[VS1REG], shuf);
	__m128i s1_plus_s2 = _mm_add_epi16(m_xv[VS1REG], shuf);
	__m128i s1_sub_s2 = _mm_sub_epi16(m_xv[VS1REG], shuf);
	__m128i s2_neg = _mm_xor_si128(shuf, vec_neg1);

	__m128i s2_lz = _mm_cmplt_epi16(shuf, _mm_setzero_si128());
	__m128i s1s2_xor_lz = _mm_cmplt_epi16(s1_xor_s2, _mm_setzero_si128());
	__m128i s1s2_xor_gez = _mm_xor_si128(s1s2_xor_lz, vec_neg1);
	__m128i s1s2_plus_gz = _mm_cmpgt_epi16(s1_plus_s2, _mm_setzero_si128());
	__m128i s1s2_plus_lez = _mm_xor_si128(s1s2_plus_gz, vec_neg1);
	__m128i s1s2_sub_lz = _mm_cmplt_epi16(s1_sub_s2, _mm_setzero_si128());
	__m128i s1s2_sub_gez = _mm_xor_si128(s1s2_sub_lz, vec_neg1);

	__m128i s1_mask = _mm_or_si128(_mm_and_si128(s1s2_xor_gez, s1s2_sub_lz),   _mm_and_si128(s1s2_xor_lz, s1s2_plus_gz));
	__m128i s2_mask = _mm_or_si128(_mm_and_si128(s1s2_xor_gez, s1s2_sub_gez),  _mm_and_si128(s1s2_xor_lz, s1s2_plus_lez));
	m_accum_l = _mm_or_si128(_mm_and_si128(m_xv[VS1REG], s1_mask), _mm_and_si128(s2_neg, s2_mask));
	m_xv[VDREG] = m_accum_l;

	m_xvflag[COMPARE] = _mm_or_si128(_mm_and_si128(s1s2_xor_gez, s2_lz),         _mm_and_si128(s1s2_xor_lz, s1s2_plus_lez));
	m_xvflag[CLIP2] = _mm_or_si128(_mm_and_si128(s1s2_xor_gez, s1s2_sub_gez),  _mm_and_si128(s1s2_xor_lz, s2_lz));
#endif
	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		VEC_GET_SCALAR_VS1(s1, i);
		VEC_GET_SCALAR_VS2(s2, i);

		if ((INT16)(s1 ^ s2) < 0)
		{
			if (s2 < 0)
			{
				VEC_SET_CLIP2_FLAG(i);
			}
			if ((s1 + s2) <= 0)
			{
				VEC_SET_ACCUM_L(~((UINT16)s2), i);
				VEC_SET_COMPARE_FLAG(i);
			}
			else
			{
				VEC_SET_ACCUM_L(s1, i);
			}
		}
		else
		{
			if (s2 < 0)
			{
				VEC_SET_COMPARE_FLAG(i);
			}
			if ((s1 - s2) >= 0)
			{
				VEC_SET_ACCUM_L(s2, i);
				VEC_SET_CLIP2_FLAG(i);
			}
			else
			{
				VEC_SET_ACCUM_L(s1, i);
			}
		}

		vres[i] = VEC_ACCUM_L(i);
	}
	VEC_WRITEBACK_RESULT();
}

static void cfunc_rsp_vcr_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vcr_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vcr_scalar()
{
	int op = m_rsp_state->arg0;

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);

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

		vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vcr_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vcr_scalar();
}
#endif

#if USE_SIMD
// VMRG
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100111 |
// ------------------------------------------------------
//
// Merges two vectors according to compare flags

inline void rsp_device::ccfunc_rsp_vmrg_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i s2mask = _mm_cmpeq_epi16(m_xvflag[COMPARE], _mm_setzero_si128());
	__m128i s1mask = _mm_xor_si128(s2mask, vec_neg1);
	__m128i result = _mm_and_si128(m_xv[VS1REG], s1mask);
	m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, s2mask));
	m_accum_l = m_xv[VDREG];
}

static void cfunc_rsp_vmrg_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmrg_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmrg_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		if (COMPARE_FLAG(i) != 0)
		{
			vres[i] = s1;
		}
		else
		{
			vres[i] = s2;
		}

		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vmrg_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmrg_scalar();
}
#endif

#if USE_SIMD
// VAND
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
// ------------------------------------------------------
//
// Bitwise AND of two vector registers

inline void rsp_device::ccfunc_rsp_vand_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_xv[VDREG] = _mm_and_si128(m_xv[VS1REG], shuf);
	m_accum_l = m_xv[VDREG];
}

static void cfunc_rsp_vand_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vand_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vand_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		vres[i] = s1 & s2;
		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vand_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vand_scalar();
}
#endif

#if USE_SIMD
// VNAND
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101001 |
// ------------------------------------------------------
//
// Bitwise NOT AND of two vector registers

inline void rsp_device::ccfunc_rsp_vnand_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_xv[VDREG] = _mm_xor_si128(_mm_and_si128(m_xv[VS1REG], shuf), vec_neg1);
	m_accum_l = m_xv[VDREG];
}

static void cfunc_rsp_vnand_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vnand_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vnand_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		vres[i] = ~((s1 & s2));
		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vnand_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vnand_scalar();
}
#endif

#if USE_SIMD
// VOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101010 |
// ------------------------------------------------------
//
// Bitwise OR of two vector registers

inline void rsp_device::ccfunc_rsp_vor_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_xv[VDREG] = _mm_or_si128(m_xv[VS1REG], shuf);
	m_accum_l = m_xv[VDREG];
}

static void cfunc_rsp_vor_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vor_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vor_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		vres[i] = s1 | s2;
		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vor_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vor_scalar();
}
#endif

#if USE_SIMD
// VNOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101011 |
// ------------------------------------------------------
//
// Bitwise NOT OR of two vector registers

inline void rsp_device::ccfunc_rsp_vnor_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_xv[VDREG] = _mm_xor_si128(_mm_or_si128(m_xv[VS1REG], shuf), vec_neg1);
	m_accum_l = m_xv[VDREG];
}

static void cfunc_rsp_vnor_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vnor_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vnor_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		vres[i] = ~(s1 | s2);
		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vnor_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vnor_scalar();
}
#endif

#if USE_SIMD
// VXOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101100 |
// ------------------------------------------------------
//
// Bitwise XOR of two vector registers

inline void rsp_device::ccfunc_rsp_vxor_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_xv[VDREG] = _mm_xor_si128(m_xv[VS1REG], shuf);
	m_accum_l = m_xv[VDREG];
}

static void cfunc_rsp_vxor_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vxor_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vxor_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		vres[i] = s1 ^ s2;
		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vxor_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vxor_scalar();
}
#endif

#if USE_SIMD
// VNXOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
// ------------------------------------------------------
//
// Bitwise NOT XOR of two vector registers

inline void rsp_device::ccfunc_rsp_vnxor_simd()
{
	int op = m_rsp_state->arg0;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_xv[VDREG] = _mm_xor_si128(_mm_xor_si128(m_xv[VS1REG], shuf), vec_neg1);
	m_accum_l = m_xv[VDREG];
}

static void cfunc_rsp_vnxor_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vnxor_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vnxor_scalar()
{
	int op = m_rsp_state->arg0;

	INT16 vres[8];
	for (int i = 0; i < 8; i++)
	{
		UINT16 s1, s2;
		SCALAR_GET_VS1(s1, i);
		SCALAR_GET_VS2(s2, i);
		vres[i] = ~(s1 ^ s2);
		SET_ACCUM_L(vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_rsp_vnxor_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vnxor_scalar();
}
#endif

#if USE_SIMD
// VRCP
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110000 |
// ------------------------------------------------------
//
// Calculates reciprocal

inline void rsp_device::ccfunc_rsp_vrcp_simd()
{
	int op = m_rsp_state->arg0;

	INT32 shifter = 0;
	UINT16 urec;
	INT32 rec;
	SIMD_EXTRACT16(m_xv[VS2REG], urec, EL);
	rec = (INT16)urec;
	INT32 datainput = (rec < 0) ? (-rec) : rec;
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

	SIMD_INSERT16(m_xv[VDREG], (UINT16)rec, VS1REG);
	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
}

static void cfunc_rsp_vrcp_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrcp_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vrcp_scalar()
{
	int op = m_rsp_state->arg0;

	INT32 shifter = 0;
	INT32 rec = (INT16)(VREG_S(VS2REG, EL & 7));
	INT32 datainput = (rec < 0) ? (-rec) : rec;
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

	W_VREG_S(VDREG, VS1REG & 7) = (UINT16)rec;
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
	}
}

static void cfunc_rsp_vrcp_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrcp_scalar();
}
#endif

#if USE_SIMD
// VRCPL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110001 |
// ------------------------------------------------------
//
// Calculates reciprocal low part

inline void rsp_device::ccfunc_rsp_vrcpl_simd()
{
	int op = m_rsp_state->arg0;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	INT32 shifter = 0;

	UINT16 urec;
	SIMD_EXTRACT16(m_xv[VS2REG], urec, EL);
	INT32 rec = (urec | m_reciprocal_high);

	INT32 datainput = rec;

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
		if (m_dp_allowed)
		{
			shifter = 0;
		}
		else
		{
			shifter = 0x10;
		}
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

	SIMD_INSERT16(m_xv[VDREG], (UINT16)rec, VS1REG);

	for (int i = 0; i < 8; i++)
	{
		INT16 val;
		SIMD_EXTRACT16(m_xv[VS2REG], val, VEC_EL_2(EL, i));
		VEC_SET_ACCUM_L(val, i);
	}
}

static void cfunc_rsp_vrcpl_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrcpl_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vrcpl_scalar()
{
	int op = m_rsp_state->arg0;

	INT32 shifter = 0;
	INT32 rec = ((UINT16)(VREG_S(VS2REG, EL & 7)) | m_reciprocal_high);
	INT32 datainput = rec;

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
		if (m_dp_allowed)
		{
			shifter = 0;
		}
		else
		{
			shifter = 0x10;
		}
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

	W_VREG_S(VDREG, VS1REG & 7) = (UINT16)rec;

	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
	}
}

static void cfunc_rsp_vrcpl_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrcpl_scalar();
}
#endif

#if USE_SIMD
// VRCPH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110010 |
// ------------------------------------------------------
//
// Calculates reciprocal high part

inline void rsp_device::ccfunc_rsp_vrcph_simd()
{
	int op = m_rsp_state->arg0;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	UINT16 rcph;
	SIMD_EXTRACT16(m_xv[VS2REG], rcph, EL);
	m_reciprocal_high = rcph << 16;
	m_dp_allowed = 1;

	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	SIMD_INSERT16(m_xv[VDREG], (INT16)(m_reciprocal_res >> 16), VS1REG);
}

static void cfunc_rsp_vrcph_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrcph_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vrcph_scalar()
{
	int op = m_rsp_state->arg0;

	m_reciprocal_high = (VREG_S(VS2REG, EL & 7)) << 16;
	m_dp_allowed = 1;

	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
	}

	W_VREG_S(VDREG, VS1REG & 7) = (INT16)(m_reciprocal_res >> 16);
}

static void cfunc_rsp_vrcph_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrcph_scalar();
}
#endif

#if USE_SIMD
// VMOV
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110011 |
// ------------------------------------------------------
//
// Moves element from vector to destination vector

inline void rsp_device::ccfunc_rsp_vmov_simd()
{
	int op = m_rsp_state->arg0;

	INT16 val;
	SIMD_EXTRACT16(m_xv[VS2REG], val, EL);
	SIMD_INSERT16(m_xv[VDREG], val, VS1REG);
	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
}

static void cfunc_rsp_vmov_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmov_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vmov_scalar()
{
	int op = m_rsp_state->arg0;

	W_VREG_S(VDREG, VS1REG & 7) = VREG_S(VS2REG, EL & 7);
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
	}
}

static void cfunc_rsp_vmov_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vmov_scalar();
}
#endif

#if USE_SIMD
// VRSQL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110101 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root low part

inline void rsp_device::ccfunc_rsp_vrsql_simd()
{
	int op = m_rsp_state->arg0;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	INT32 shifter = 0;
	UINT16 val;
	SIMD_EXTRACT16(m_xv[VS2REG], val, EL);
	INT32 rec = m_reciprocal_high | val;
	INT32 datainput = rec;

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
		if (m_dp_allowed)
		{
			shifter = 0;
		}
		else
		{
			shifter = 0x10;
		}
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

	SIMD_INSERT16(m_xv[VDREG], (UINT16)rec, VS1REG);
	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
}

static void cfunc_rsp_vrsql_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrsql_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vrsql_scalar()
{
	int op = m_rsp_state->arg0;

	INT32 shifter = 0;
	INT32 rec = m_reciprocal_high | (UINT16)VREG_S(VS2REG, EL & 7);
	INT32 datainput = rec;

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
		if (m_dp_allowed)
		{
			shifter = 0;
		}
		else
		{
			shifter = 0x10;
		}
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

	W_VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
	}
}

static void cfunc_rsp_vrsql_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrsql_scalar();
}
#endif

#if USE_SIMD
// VRSQH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110110 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root high part

inline void rsp_device::ccfunc_rsp_vrsqh_simd()
{
	int op = m_rsp_state->arg0;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	UINT16 val;
	SIMD_EXTRACT16(m_xv[VS2REG], val, EL);
	m_reciprocal_high = val << 16;
	m_dp_allowed = 1;

	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	SIMD_INSERT16(m_xv[VDREG], (INT16)(m_reciprocal_res >> 16), VS1REG); // store high part
}

static void cfunc_rsp_vrsqh_simd(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrsqh_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)

inline void rsp_device::ccfunc_rsp_vrsqh_scalar()
{
	int op = m_rsp_state->arg0;

	m_reciprocal_high = (VREG_S(VS2REG, EL & 7)) << 16;
	m_dp_allowed = 1;

	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
	}

	W_VREG_S(VDREG, VS1REG & 7) = (INT16)(m_reciprocal_res >> 16);  // store high part
}

static void cfunc_rsp_vrsqh_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_rsp_vrsqh_scalar();
}
#endif


inline void rsp_device::ccfunc_sp_set_status_cb()
{
	m_sp_set_status_func(0, m_rsp_state->arg0, 0xffffffff);
}

void cfunc_sp_set_status_cb(void *param)
{
	((rsp_device *)param)->ccfunc_sp_set_status_cb();
}

void rsp_device::execute_run_drc()
{
	drcuml_state *drcuml = m_drcuml;
	int execute_result;

	/* reset the cache if dirty */
	if (m_cache_dirty)
		code_flush_cache();
	m_cache_dirty = FALSE;

	/* execute */
	do
	{
		if( m_sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
		{
			m_rsp_state->icount = MIN(m_rsp_state->icount, 0);
			break;
		}

		/* run as much as we can */
		execute_result = drcuml->execute(*m_entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			code_compile_block(m_rsp_state->pc);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", m_rsp_state->pc);
		}
		else if (execute_result == EXECUTE_RESET_CACHE)
		{
			code_flush_cache();
		}
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}

/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    rspdrc_flush_drc_cache - outward-facing
    accessor to code_flush_cache
-------------------------------------------------*/

void rsp_device::rspdrc_flush_drc_cache()
{
	if (!machine().options().drc()) return;
	m_cache_dirty = TRUE;
}

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

void rsp_device::code_flush_cache()
{
	/* empty the transient cache contents */
	m_drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_entry_point();
		static_generate_nocode_handler();
		static_generate_out_of_cycles();

		/* add subroutines for memory accesses */
		static_generate_memory_accessor(1, FALSE, "read8",       m_read8);
		static_generate_memory_accessor(1, TRUE,  "write8",      m_write8);
		static_generate_memory_accessor(2, FALSE, "read16",      m_read16);
		static_generate_memory_accessor(2, TRUE,  "write16",     m_write16);
		static_generate_memory_accessor(4, FALSE, "read32",      m_read32);
		static_generate_memory_accessor(4, TRUE,  "write32",     m_write32);
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unable to generate static RSP code\n");
	}
}


/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

void rsp_device::code_compile_block(offs_t pc)
{
	drcuml_state *drcuml = m_drcuml;
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	int override = FALSE;
	drcuml_block *block;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = m_drcfe->describe_code(pc);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			block = drcuml->begin_block(4096);

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != NULL; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				UINT32 nextpc;

				/* add a code log entry */
				if (RSP_LOG_UML)
					block->append_comment("-------------------------");                 // comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != NULL);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !drcuml->hash_exists(0, seqhead->pc))
					UML_HASH(block, 0, seqhead->pc);                                        // hash    mode,pc

				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = TRUE;
					UML_HASH(block, 0, seqhead->pc);                                        // hash    mode,pc
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc
					UML_HASHJMP(block, 0, seqhead->pc, *m_nocode);
																							// hashjmp <0>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (m_program->get_write_ptr(seqhead->physpc) != NULL)
					generate_checksum_block(block, &compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
					generate_sequence_instruction(block, &compiler, curdesc);

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
					nextpc = pc;

				/* otherwise we just go to the next instruction */
				else
					nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

				/* count off cycles and go there */
				generate_update_cycles(block, &compiler, nextpc, TRUE);            // <subtract cycles>

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				if (seqlast->next() == NULL || seqlast->next()->pc != nextpc)
					UML_HASHJMP(block, 0, nextpc, *m_nocode);          // hashjmp <mode>,nextpc,nocode
			}

			/* end the sequence */
			block->end();
			g_profiler.stop();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			code_flush_cache();
		}
	}
}

/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

inline void rsp_device::ccfunc_unimplemented()
{
	UINT32 opcode = m_rsp_state->arg0;
	fatalerror("PC=%08X: Unimplemented op %08X (%02X,%02X)\n", m_rsp_state->pc, opcode, opcode >> 26, opcode & 0x3f);
}

static void cfunc_unimplemented(void *param)
{
	((rsp_device *)param)->ccfunc_unimplemented();
}

/*-------------------------------------------------
    cfunc_fatalerror - a generic fatalerror call
-------------------------------------------------*/

#ifdef UNUSED_CODE
static void cfunc_fatalerror(void *param)
{
	fatalerror("fatalerror\n");
}
#endif


/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    ferate_entry_point - generate a
    static entry point
-------------------------------------------------*/

void rsp_device::static_generate_entry_point()
{
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(20);

	/* forward references */
	alloc_handle(drcuml, &m_nocode, "nocode");

	alloc_handle(drcuml, &m_entry, "entry");
	UML_HANDLE(block, *m_entry);                                       // handle  entry

	/* load fast integer registers */
	load_fast_iregs(block);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_rsp_state->pc), *m_nocode);                   // hashjmp <mode>,<pc>,nocode
	block->end();
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

void rsp_device::static_generate_nocode_handler()
{
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);                                      // handle  nocode
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_rsp_state->pc), I0);                                          // mov     [pc],i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE

	block->end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

void rsp_device::static_generate_out_of_cycles()
{
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);                               // handle  out_of_cycles
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_rsp_state->pc), I0);                                          // mov     <pc>,i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                 // exit    EXECUTE_OUT_OF_CYCLES

	block->end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void rsp_device::static_generate_memory_accessor(int size, int iswrite, const char *name, code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I1 */
	drcuml_state *drcuml = m_drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, &handleptr, name);
	UML_HANDLE(block, *handleptr);                                                  // handle  *handleptr

	// write:
	if (iswrite)
	{
		if (size == 1)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);              // mov     [arg0],i0 ; address
			UML_MOV(block, mem(&m_rsp_state->arg1), I1);              // mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write8, this);                            // callc   cfunc_write8
		}
		else if (size == 2)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);              // mov     [arg0],i0 ; address
			UML_MOV(block, mem(&m_rsp_state->arg1), I1);              // mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write16, this);                           // callc   cfunc_write16
		}
		else if (size == 4)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);              // mov     [arg0],i0 ; address
			UML_MOV(block, mem(&m_rsp_state->arg1), I1);              // mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write32, this);                           // callc   cfunc_write32
		}
	}
	else
	{
		if (size == 1)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);          // mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read8, this);                         // callc   cfunc_printf_debug
			UML_MOV(block, I0, mem(&m_rsp_state->arg0));          // mov     i0,[arg0],i0 ; result
		}
		else if (size == 2)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);          // mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read16, this);                        // callc   cfunc_read16
			UML_MOV(block, I0, mem(&m_rsp_state->arg0));          // mov     i0,[arg0],i0 ; result
		}
		else if (size == 4)
		{
			UML_MOV(block, mem(&m_rsp_state->arg0), I0);          // mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read32, this);                        // callc   cfunc_read32
			UML_MOV(block, I0, mem(&m_rsp_state->arg0));          // mov     i0,[arg0],i0 ; result
		}
	}
	UML_RET(block);

	block->end();
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/
void rsp_device::generate_update_cycles(drcuml_block *block, compiler_state *compiler, parameter param, int allow_exception)
{
	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&m_rsp_state->icount), mem(&m_rsp_state->icount), MAPVAR_CYCLES);        // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		UML_EXHc(block, COND_S, *m_out_of_cycles, param);
	}
	compiler->cycles = 0;
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void rsp_device::generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (RSP_LOG_UML)
	{
		block->append_comment("[Validation for %08X]", seqhead->pc | 0x1000);       // comment
	}
	/* loose verify or single instruction: just compare and fail */
	if (!(m_drcoptions & RSPDRC_STRICT_VERIFY) || seqhead->next() == NULL)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			UINT32 sum = seqhead->opptr.l[0];
			void *base = m_direct->read_decrypted_ptr(seqhead->physpc | 0x1000);
			UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);                         // load    i0,base,0,dword

			if (seqhead->delay.first() != NULL && seqhead->physpc != seqhead->delay.first()->physpc)
			{
				base = m_direct->read_decrypted_ptr(seqhead->delay.first()->physpc | 0x1000);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);                 // load    i1,base,dword
				UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1

				sum += seqhead->delay.first()->opptr.l[0];
			}

			UML_CMP(block, I0, sum);                                    // cmp     i0,opptr[0]
			UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));     // exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		UINT32 sum = 0;
		void *base = m_direct->read_decrypted_ptr(seqhead->physpc | 0x1000);
		UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);                             // load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = m_direct->read_decrypted_ptr(curdesc->physpc | 0x1000);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);                     // load    i1,base,dword
				UML_ADD(block, I0, I0, I1);                         // add     i0,i0,i1
				sum += curdesc->opptr.l[0];

				if (curdesc->delay.first() != NULL && (curdesc == seqlast || (curdesc->next() != NULL && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					base = m_direct->read_decrypted_ptr(curdesc->delay.first()->physpc | 0x1000);
					UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);                 // load    i1,base,dword
					UML_ADD(block, I0, I0, I1);                     // add     i0,i0,i1

					sum += curdesc->delay.first()->opptr.l[0];
				}
			}
		UML_CMP(block, I0, sum);                                            // cmp     i0,sum
		UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));         // exne    nocode,seqhead->pc
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void rsp_device::generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	offs_t expc;

	/* add an entry for the log */
	if (RSP_LOG_UML && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);                                             // mapvar  PC,expc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                             // mapvar  CYCLES,compiler->cycles

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&m_rsp_state->pc), desc->pc);                                // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);                                         // debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
#if 0
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&m_rsp_state->pc), desc->pc);                               // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);                             // exit EXECUTE_UNMAPPED_CODE
	}
#endif

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	/*else*/ if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
		{
			UML_MOV(block, mem(&m_rsp_state->pc), desc->pc);                            // mov     [pc],desc->pc
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented, this);                             // callc   cfunc_unimplemented
		}
	}
}

/*------------------------------------------------------------------
    generate_branch
------------------------------------------------------------------*/

void rsp_device::generate_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	compiler_state compiler_temp = *compiler;

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, &compiler_temp, desc->targetpc, TRUE);	// <subtract cycles>
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
			UML_JMP(block, desc->targetpc | 0x80000000);						// jmp     desc->targetpc
		else
			UML_HASHJMP(block, 0, desc->targetpc, *m_nocode);					// hashjmp <mode>,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(block, &compiler_temp, mem(&m_rsp_state->jmpdest), TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, mem(&m_rsp_state->jmpdest), *m_nocode);						// hashjmp <mode>,<rsreg>,nocode
	}
}

/*------------------------------------------------------------------
    generate_delay_slot_and_branch
------------------------------------------------------------------*/

void rsp_device::generate_delay_slot_and_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg)
{
	compiler_state compiler_temp = *compiler;
	UINT32 op = desc->opptr.l[0];

	/* fetch the target register if dynamic, in case it is modified by the delay slot */
	if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
	{
		UML_AND(block, mem(&m_rsp_state->jmpdest), R32(RSREG), 0x00000fff);
		UML_OR(block, mem(&m_rsp_state->jmpdest), mem(&m_rsp_state->jmpdest), 0x1000);
	}

	/* set the link if needed -- before the delay slot */
	if (linkreg != 0)
	{
		UML_MOV(block, R32(linkreg), (INT32)(desc->pc + 8));                    // mov    <linkreg>,desc->pc + 8
	}

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay.first() != NULL);
	generate_sequence_instruction(block, &compiler_temp, desc->delay.first());     // <next instruction>

	generate_branch(block, compiler, desc);

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles and account for skipped slots */
	compiler->cycles += desc->skipslots;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                             // mapvar  CYCLES,compiler->cycles
}


/*-------------------------------------------------
    generate_vector_opcode - generate code for a
    vector opcode
-------------------------------------------------*/

#if USE_SIMD

int rsp_device::generate_vector_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	// Opcode legend:
	//    E = VS2 element type
	//    S = VS1, Source vector 1
	//    T = VS2, Source vector 2
	//    D = Destination vector

	switch (op & 0x3f)
	{
		case 0x00:      /* VMULF */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulf_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmulf_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x01:      /* VMULU */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulu_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmulu_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x04:      /* VMUDL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudl_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmudl_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x05:      /* VMUDM */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudm_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmudm_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x06:      /* VMUDN */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudn_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmudn_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x07:      /* VMUDH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudh_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmudh_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x08:      /* VMACF */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacf_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmacf_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x09:      /* VMACU */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacu_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmacu_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x0c:      /* VMADL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadl_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmadl_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x0d:      /* VMADM */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadm_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmadm_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x0e:      /* VMADN */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadn_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmadn_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x0f:      /* VMADH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadh_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmadh_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x10:      /* VADD */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vadd_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vadd_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x11:      /* VSUB */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsub_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vsub_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x13:      /* VABS */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vabs_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vabs_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x14:      /* VADDC */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vaddc_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vaddc_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x15:      /* VSUBC */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsubc_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vsubc_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x1d:      /* VSAW */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsaw_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vsaw_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x20:      /* VLT */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vlt_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vlt_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x21:      /* VEQ */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_veq_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_veq_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x22:      /* VNE */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vne_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vne_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x23:      /* VGE */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vge_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vge_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x24:      /* VCL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcl_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vcl_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x25:      /* VCH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vch_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vch_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x26:      /* VCR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcr_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vcr_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x27:      /* VMRG */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmrg_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmrg_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x28:      /* VAND */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vand_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vand_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x29:      /* VNAND */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnand_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vnand_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x2a:      /* VOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vor_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vor_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x2b:      /* VNOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnor_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vnor_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x2c:      /* VXOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vxor_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vxor_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x2d:      /* VNXOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnxor_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vnxor_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x30:      /* VRCP */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcp_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vrcp_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x31:      /* VRCPL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcpl_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vrcpl_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x32:      /* VRCPH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcph_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vrcph_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x33:      /* VMOV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmov_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vmov_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x35:      /* VRSQL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsql_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vrsql_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		case 0x36:      /* VRSQH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsqh_simd, this);
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_rsp_vrsqh_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
			return TRUE;

		default:
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented_opcode, this);
			return FALSE;
	}
}

#else

int rsp_device::generate_vector_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	// Opcode legend:
	//    E = VS2 element type
	//    S = VS1, Source vector 1
	//    T = VS2, Source vector 2
	//    D = Destination vector

	switch (op & 0x3f)
	{
		case 0x00:      /* VMULF */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulf_scalar, this);
			return TRUE;

		case 0x01:      /* VMULU */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulu_scalar, this);
			return TRUE;

		case 0x04:      /* VMUDL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudl_scalar, this);
			return TRUE;

		case 0x05:      /* VMUDM */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudm_scalar, this);
			return TRUE;

		case 0x06:      /* VMUDN */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudn_scalar, this);
			return TRUE;

		case 0x07:      /* VMUDH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudh_scalar, this);
			return TRUE;

		case 0x08:      /* VMACF */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacf_scalar, this);
			return TRUE;

		case 0x09:      /* VMACU */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacu_scalar, this);
			return TRUE;

		case 0x0c:      /* VMADL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadl_scalar, this);
			return TRUE;

		case 0x0d:      /* VMADM */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadm_scalar, this);
			return TRUE;

		case 0x0e:      /* VMADN */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadn_scalar, this);
			return TRUE;

		case 0x0f:      /* VMADH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadh_scalar, this);
			return TRUE;

		case 0x10:      /* VADD */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vadd_scalar, this);
			return TRUE;

		case 0x11:      /* VSUB */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsub_scalar, this);
			return TRUE;

		case 0x13:      /* VABS */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vabs_scalar, this);
			return TRUE;

		case 0x14:      /* VADDC */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vaddc_scalar, this);
			return TRUE;

		case 0x15:      /* VSUBC */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsubc_scalar, this);
			return TRUE;

		case 0x1d:      /* VSAW */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsaw_scalar, this);
			return TRUE;

		case 0x20:      /* VLT */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vlt_scalar, this);
			return TRUE;

		case 0x21:      /* VEQ */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_veq_scalar, this);
			return TRUE;

		case 0x22:      /* VNE */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vne_scalar, this);
			return TRUE;

		case 0x23:      /* VGE */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vge_scalar, this);
			return TRUE;

		case 0x24:      /* VCL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcl_scalar, this);
			return TRUE;

		case 0x25:      /* VCH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vch_scalar, this);
			return TRUE;

		case 0x26:      /* VCR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcr_scalar, this);
			return TRUE;

		case 0x27:      /* VMRG */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmrg_scalar, this);
			return TRUE;

		case 0x28:      /* VAND */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vand_scalar, this);
			return TRUE;

		case 0x29:      /* VNAND */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnand_scalar, this);
			return TRUE;

		case 0x2a:      /* VOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vor_scalar, this);
			return TRUE;

		case 0x2b:      /* VNOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnor_scalar, this);
			return TRUE;

		case 0x2c:      /* VXOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vxor_scalar, this);
			return TRUE;

		case 0x2d:      /* VNXOR */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnxor_scalar, this);
			return TRUE;

		case 0x30:      /* VRCP */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcp_scalar, this);
			return TRUE;

		case 0x31:      /* VRCPL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcpl_scalar, this);
			return TRUE;

		case 0x32:      /* VRCPH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcph_scalar, this);
			return TRUE;

		case 0x33:      /* VMOV */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmov_scalar, this);
			return TRUE;

		case 0x35:      /* VRSQL */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsql_scalar, this);
			return TRUE;

		case 0x36:      /* VRSQH */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsqh_scalar, this);
			return TRUE;

		default:
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented_opcode, this);
			return FALSE;
	}
}
#endif

int rsp_device::generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op >> 26;
	code_label skip;

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:  /* SPECIAL - MIPS I */
			return generate_special(block, compiler, desc);

		case 0x01:  /* REGIMM - MIPS I */
			return generate_regimm(block, compiler, desc);

		/* ----- jumps and branches ----- */

		case 0x02:  /* J - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			return TRUE;

		case 0x03:  /* JAL - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 31);     // <next instruction + hashjmp>
			return TRUE;

		case 0x04:  /* BEQ - MIPS I */
			UML_CMP(block, R32(RSREG), R32(RTREG));                             // cmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_NE, skip = compiler->labelnum++);              // jmp    skip,NE
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return TRUE;

		case 0x05:  /* BNE - MIPS I */
			UML_CMP(block, R32(RSREG), R32(RTREG));                             // dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_E, skip = compiler->labelnum++);                       // jmp     skip,E
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return TRUE;

		case 0x06:  /* BLEZ - MIPS I */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);                              // dcmp    <rsreg>,0
				UML_JMPc(block, COND_G, skip = compiler->labelnum++);                   // jmp     skip,G
				generate_delay_slot_and_branch(block, compiler, desc, 0);  // <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			else
				generate_delay_slot_and_branch(block, compiler, desc, 0);  // <next instruction + hashjmp>
			return TRUE;

		case 0x07:  /* BGTZ - MIPS I */
			UML_CMP(block, R32(RSREG), 0);                                  // dcmp    <rsreg>,0
			UML_JMPc(block, COND_LE, skip = compiler->labelnum++);                  // jmp     skip,LE
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return TRUE;


		/* ----- immediate arithmetic ----- */

		case 0x0f:  /* LUI - MIPS I */
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), SIMMVAL << 16);                  // dmov    <rtreg>,SIMMVAL << 16
			return TRUE;

		case 0x08:  /* ADDI - MIPS I */
		case 0x09:  /* ADDIU - MIPS I */
			if (RTREG != 0)
			{
				UML_ADD(block, R32(RTREG), R32(RSREG), SIMMVAL);                // add     i0,<rsreg>,SIMMVAL,V
			}
			return TRUE;

		case 0x0a:  /* SLTI - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), SIMMVAL);                            // dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, COND_L, R32(RTREG));                                    // dset    <rtreg>,l
			}
			return TRUE;

		case 0x0b:  /* SLTIU - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), SIMMVAL);                            // dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, COND_B, R32(RTREG));                                    // dset    <rtreg>,b
			}
			return TRUE;


		case 0x0c:  /* ANDI - MIPS I */
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), R32(RSREG), UIMMVAL);                // dand    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0d:  /* ORI - MIPS I */
			if (RTREG != 0)
				UML_OR(block, R32(RTREG), R32(RSREG), UIMMVAL);             // dor     <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0e:  /* XORI - MIPS I */
			if (RTREG != 0)
				UML_XOR(block, R32(RTREG), R32(RSREG), UIMMVAL);                // dxor    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		/* ----- memory load operations ----- */

		case 0x20:  /* LB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read8);                                    // callh   read8
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), I0, SIZE_BYTE);                     // dsext   <rtreg>,i0,byte
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x21:  /* LH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read16);                               // callh   read16
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), I0, SIZE_WORD);                     // dsext   <rtreg>,i0,word
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x23:  /* LW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read32);                               // callh   read32
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), I0);
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x24:  /* LBU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read8);                                    // callh   read8
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), I0, 0xff);                   // dand    <rtreg>,i0,0xff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x25:  /* LHU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *m_read16);                               // callh   read16
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), I0, 0xffff);                 // dand    <rtreg>,i0,0xffff
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x32:  /* LWC2 - MIPS I */
			return generate_lwc2(block, compiler, desc);


		/* ----- memory store operations ----- */

		case 0x28:  /* SB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write8);                               // callh   write8
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x29:  /* SH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write16);                              // callh   write16
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x2b:  /* SW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);                        // add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));                                 // mov     i1,<rtreg>
			UML_CALLH(block, *m_write32);                              // callh   write32
			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x3a:  /* SWC2 - MIPS I */
			return generate_swc2(block, compiler, desc);
			//UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);     // mov     [arg0],desc->opptr.l
			//UML_CALLC(block, cfunc_swc2, this);                                        // callc   cfunc_mfc2
			//return TRUE;

		/* ----- coprocessor instructions ----- */

		case 0x10:  /* COP0 - MIPS I */
			return generate_cop0(block, compiler, desc);

		case 0x12:  /* COP2 - MIPS I */
			return generate_cop2(block, compiler, desc);
			//UML_EXH(block, m_exception[EXCEPTION_INVALIDOP], 0);// exh     invalidop,0
			//return TRUE;


		/* ----- unimplemented/illegal instructions ----- */

		//default:    /* ??? */       invalid_instruction(op);                                                break;
	}

	return FALSE;
}


/*-------------------------------------------------
    generate_special - compile opcodes in the
    'SPECIAL' group
-------------------------------------------------*/

int rsp_device::generate_special(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op & 63;
	//code_label skip;

	switch (opswitch)
	{
		/* ----- shift instructions ----- */

		case 0x00:  /* SLL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x02:  /* SRL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x03:  /* SRA - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x04:  /* SLLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		case 0x06:  /* SRLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		case 0x07:  /* SRAV - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		/* ----- basic arithmetic ----- */

		case 0x20:  /* ADD - MIPS I */
		case 0x21:  /* ADDU - MIPS I */
			if (RDREG != 0)
			{
				UML_ADD(block, R32(RDREG), R32(RSREG), R32(RTREG));
			}
			return TRUE;

		case 0x22:  /* SUB - MIPS I */
		case 0x23:  /* SUBU - MIPS I */
			if (RDREG != 0)
			{
				UML_SUB(block, R32(RDREG), R32(RSREG), R32(RTREG));
			}
			return TRUE;

		/* ----- basic logical ops ----- */

		case 0x24:  /* AND - MIPS I */
			if (RDREG != 0)
			{
				UML_AND(block, R32(RDREG), R32(RSREG), R32(RTREG));             // dand     <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x25:  /* OR - MIPS I */
			if (RDREG != 0)
			{
				UML_OR(block, R32(RDREG), R32(RSREG), R32(RTREG));                  // dor      <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x26:  /* XOR - MIPS I */
			if (RDREG != 0)
			{
				UML_XOR(block, R32(RDREG), R32(RSREG), R32(RTREG));             // dxor     <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x27:  /* NOR - MIPS I */
			if (RDREG != 0)
			{
				UML_OR(block, I0, R32(RSREG), R32(RTREG));                  // dor      i0,<rsreg>,<rtreg>
				UML_XOR(block, R32(RDREG), I0, (UINT64)~0);             // dxor     <rdreg>,i0,~0
			}
			return TRUE;


		/* ----- basic comparisons ----- */

		case 0x2a:  /* SLT - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));                         // dcmp    <rsreg>,<rtreg>
				UML_SETc(block, COND_L, R32(RDREG));                                    // dset    <rdreg>,l
			}
			return TRUE;

		case 0x2b:  /* SLTU - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));                         // dcmp    <rsreg>,<rtreg>
				UML_SETc(block, COND_B, R32(RDREG));                                    // dset    <rdreg>,b
			}
			return TRUE;


		/* ----- jumps and branches ----- */

		case 0x08:  /* JR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 0);      // <next instruction + hashjmp>
			return TRUE;

		case 0x09:  /* JALR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, RDREG);  // <next instruction + hashjmp>
			return TRUE;


		/* ----- system calls ----- */

		case 0x0d:  /* BREAK - MIPS I */
			UML_MOV(block, mem(&m_rsp_state->arg0), 3);                   // mov     [arg0],3
			UML_CALLC(block, cfunc_sp_set_status_cb, this);                      // callc   cfunc_sp_set_status_cb
			UML_MOV(block, mem(&m_rsp_state->icount), 0);                       // mov icount, #0
			UML_MOV(block, mem(&m_rsp_state->jmpdest), mem(&desc->targetpc));

			generate_branch(block, compiler, desc);

			UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);
			return TRUE;
	}
	return FALSE;
}



/*-------------------------------------------------
    generate_regimm - compile opcodes in the
    'REGIMM' group
-------------------------------------------------*/

int rsp_device::generate_regimm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RTREG;
	code_label skip;

	switch (opswitch)
	{
		case 0x00:  /* BLTZ */
		case 0x10:  /* BLTZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);                              // dcmp    <rsreg>,0
				UML_JMPc(block, COND_GE, skip = compiler->labelnum++);              // jmp     skip,GE
				generate_delay_slot_and_branch(block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			return TRUE;

		case 0x01:  /* BGEZ */
		case 0x11:  /* BGEZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);                              // dcmp    <rsreg>,0
				UML_JMPc(block, COND_L, skip = compiler->labelnum++);                   // jmp     skip,L
				generate_delay_slot_and_branch(block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);                                         // skip:
			}
			else
				generate_delay_slot_and_branch(block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
			return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------
    generate_cop2 - compile COP2 opcodes
-------------------------------------------------*/

int rsp_device::generate_cop2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RSREG;

	switch (opswitch)
	{
		case 0x00:  /* MFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);    // mov     [arg0],desc->opptr.l
#if USE_SIMD
			UML_CALLC(block, cfunc_mfc2_simd, this);                                      // callc   cfunc_ctc2
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_mfc2_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
#else
			UML_CALLC(block, cfunc_mfc2_scalar, this);
#endif
				//UML_SEXT(block, R32(RTREG), I0, DWORD);                      // dsext   <rtreg>,i0,dword
			}
			return TRUE;

		case 0x02:  /* CFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);    // mov     [arg0],desc->opptr.l
#if USE_SIMD
			UML_CALLC(block, cfunc_cfc2_simd, this);                                      // callc   cfunc_ctc2
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_cfc2_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
#else
			UML_CALLC(block, cfunc_cfc2_scalar, this);
#endif
				//UML_SEXT(block, R32(RTREG), I0, DWORD);                      // dsext   <rtreg>,i0,dword
			}
			return TRUE;

		case 0x04:  /* MTCz */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
#if USE_SIMD
			UML_CALLC(block, cfunc_mtc2_simd, this);                                      // callc   cfunc_ctc2
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_mtc2_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
#else
			UML_CALLC(block, cfunc_mtc2_scalar, this);
#endif
			return TRUE;

		case 0x06:  /* CTCz */
			UML_MOV(block, mem(&m_rsp_state->arg0), desc->opptr.l[0]);        // mov     [arg0],desc->opptr.l
#if USE_SIMD
			UML_CALLC(block, cfunc_ctc2_simd, this);                                      // callc   cfunc_ctc2
#if SIMUL_SIMD
			UML_CALLC(block, cfunc_backup_regs, this);
			UML_CALLC(block, cfunc_ctc2_scalar, this);
			UML_CALLC(block, cfunc_restore_regs, this);
			UML_CALLC(block, cfunc_verify_regs, this);
#endif
#else
			UML_CALLC(block, cfunc_ctc2_scalar, this);
#endif
			return TRUE;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			return generate_vector_opcode(block, compiler, desc);
	}
	return FALSE;
}

/*-------------------------------------------------
    generate_cop0 - compile COP0 opcodes
-------------------------------------------------*/

int rsp_device::generate_cop0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RSREG;

	switch (opswitch)
	{
		case 0x00:  /* MFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_rsp_state->arg0), RDREG);               // mov     [arg0],<rdreg>
				UML_MOV(block, mem(&m_rsp_state->arg1), RTREG);               // mov     [arg1],<rtreg>
				UML_CALLC(block, cfunc_get_cop0_reg, this);                          // callc   cfunc_get_cop0_reg
				if(RDREG == 2)
				{
					generate_update_cycles(block, compiler, mem(&m_rsp_state->pc), TRUE);
					UML_HASHJMP(block, 0, mem(&m_rsp_state->pc), *m_nocode);
				}
			}
			return TRUE;

		case 0x04:  /* MTCz */
			UML_MOV(block, mem(&m_rsp_state->arg0), RDREG);                   // mov     [arg0],<rdreg>
			UML_MOV(block, mem(&m_rsp_state->arg1), R32(RTREG));                  // mov     [arg1],rtreg
			UML_CALLC(block, cfunc_set_cop0_reg, this);                              // callc   cfunc_set_cop0_reg
			return TRUE;
	}

	return FALSE;
}

#if USE_SIMD
inline void rsp_device::ccfunc_mfc2_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int el = (op >> 7) & 0xf;

	UINT16 out;
	SIMD_EXTRACT16(m_xv[VS1REG], out, (el >> 1));
	out >>= (1 - (el & 1)) * 8;
	out &= 0x00ff;

	el++;

	UINT16 temp;
	SIMD_EXTRACT16(m_xv[VS1REG], temp, (el >> 1));
	temp >>= (1 - (el & 1)) * 8;
	temp &= 0x00ff;

	m_rsp_state->r[RTREG] = (INT32)(INT16)((out << 8) | temp);
}

static void cfunc_mfc2_simd(void *param)
{
	((rsp_device *)param)->ccfunc_mfc2_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)
inline void rsp_device::ccfunc_mfc2_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int el = (op >> 7) & 0xf;

	UINT16 b1 = VREG_B(VS1REG, (el+0) & 0xf);
	UINT16 b2 = VREG_B(VS1REG, (el+1) & 0xf);
	if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
}

static void cfunc_mfc2_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_mfc2_scalar();
}
#endif

#if USE_SIMD
inline void rsp_device::ccfunc_cfc2_simd()
{
	UINT32 op = m_rsp_state->arg0;
	if (RTREG)
	{
		switch(RDREG)
		{
			case 0:
				RTVAL = ((VEC_CARRY_FLAG(0) & 1) << 0) |
						((VEC_CARRY_FLAG(1) & 1) << 1) |
						((VEC_CARRY_FLAG(2) & 1) << 2) |
						((VEC_CARRY_FLAG(3) & 1) << 3) |
						((VEC_CARRY_FLAG(4) & 1) << 4) |
						((VEC_CARRY_FLAG(5) & 1) << 5) |
						((VEC_CARRY_FLAG(6) & 1) << 6) |
						((VEC_CARRY_FLAG(7) & 1) << 7) |
						((VEC_ZERO_FLAG(0) & 1) << 8) |
						((VEC_ZERO_FLAG(1) & 1) << 9) |
						((VEC_ZERO_FLAG(2) & 1) << 10) |
						((VEC_ZERO_FLAG(3) & 1) << 11) |
						((VEC_ZERO_FLAG(4) & 1) << 12) |
						((VEC_ZERO_FLAG(5) & 1) << 13) |
						((VEC_ZERO_FLAG(6) & 1) << 14) |
						((VEC_ZERO_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 1:
				RTVAL = ((VEC_COMPARE_FLAG(0) & 1) << 0) |
						((VEC_COMPARE_FLAG(1) & 1) << 1) |
						((VEC_COMPARE_FLAG(2) & 1) << 2) |
						((VEC_COMPARE_FLAG(3) & 1) << 3) |
						((VEC_COMPARE_FLAG(4) & 1) << 4) |
						((VEC_COMPARE_FLAG(5) & 1) << 5) |
						((VEC_COMPARE_FLAG(6) & 1) << 6) |
						((VEC_COMPARE_FLAG(7) & 1) << 7) |
						((VEC_CLIP2_FLAG(0) & 1) << 8) |
						((VEC_CLIP2_FLAG(1) & 1) << 9) |
						((VEC_CLIP2_FLAG(2) & 1) << 10) |
						((VEC_CLIP2_FLAG(3) & 1) << 11) |
						((VEC_CLIP2_FLAG(4) & 1) << 12) |
						((VEC_CLIP2_FLAG(5) & 1) << 13) |
						((VEC_CLIP2_FLAG(6) & 1) << 14) |
						((VEC_CLIP2_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 2:
				RTVAL = ((VEC_CLIP1_FLAG(0) & 1) << 0) |
						((VEC_CLIP1_FLAG(1) & 1) << 1) |
						((VEC_CLIP1_FLAG(2) & 1) << 2) |
						((VEC_CLIP1_FLAG(3) & 1) << 3) |
						((VEC_CLIP1_FLAG(4) & 1) << 4) |
						((VEC_CLIP1_FLAG(5) & 1) << 5) |
						((VEC_CLIP1_FLAG(6) & 1) << 6) |
						((VEC_CLIP1_FLAG(7) & 1) << 7);
				break;
		}
	}
}

static void cfunc_cfc2_simd(void *param)
{
	((rsp_device *)param)->ccfunc_cfc2_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)
inline void rsp_device::ccfunc_cfc2_scalar()
{
	UINT32 op = m_rsp_state->arg0;
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

static void cfunc_cfc2_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_cfc2_scalar();
}
#endif

#if USE_SIMD
inline void rsp_device::ccfunc_mtc2_simd()
{
	UINT32 op = m_rsp_state->arg0;
	int el = (op >> 7) & 0xf;
	SIMD_INSERT16(m_xv[VS1REG], RTVAL, el >> 1);
}

static void cfunc_mtc2_simd(void *param)
{
	((rsp_device *)param)->ccfunc_mtc2_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)
inline void rsp_device::ccfunc_mtc2_scalar()
{
	UINT32 op = m_rsp_state->arg0;
	int el = (op >> 7) & 0xf;
	VREG_B(VS1REG, (el+0) & 0xf) = (RTVAL >> 8) & 0xff;
	VREG_B(VS1REG, (el+1) & 0xf) = (RTVAL >> 0) & 0xff;
}

static void cfunc_mtc2_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_mtc2_scalar();
}
#endif

#if USE_SIMD
inline void rsp_device::ccfunc_ctc2_simd()
{
	UINT32 op = m_rsp_state->arg0;
	switch(RDREG)
	{
		case 0:
			VEC_CLEAR_CARRY_FLAGS();
			VEC_CLEAR_ZERO_FLAGS();
			m_vflag[0][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[0][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[0][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[0][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[0][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[0][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[0][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[0][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0))  { VEC_SET_CARRY_FLAG(0); }
			if (RTVAL & (1 << 1))  { VEC_SET_CARRY_FLAG(1); }
			if (RTVAL & (1 << 2))  { VEC_SET_CARRY_FLAG(2); }
			if (RTVAL & (1 << 3))  { VEC_SET_CARRY_FLAG(3); }
			if (RTVAL & (1 << 4))  { VEC_SET_CARRY_FLAG(4); }
			if (RTVAL & (1 << 5))  { VEC_SET_CARRY_FLAG(5); }
			if (RTVAL & (1 << 6))  { VEC_SET_CARRY_FLAG(6); }
			if (RTVAL & (1 << 7))  { VEC_SET_CARRY_FLAG(7); }
			m_vflag[3][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[3][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[3][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[3][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[3][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[3][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[3][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[3][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { VEC_SET_ZERO_FLAG(0); }
			if (RTVAL & (1 << 9))  { VEC_SET_ZERO_FLAG(1); }
			if (RTVAL & (1 << 10)) { VEC_SET_ZERO_FLAG(2); }
			if (RTVAL & (1 << 11)) { VEC_SET_ZERO_FLAG(3); }
			if (RTVAL & (1 << 12)) { VEC_SET_ZERO_FLAG(4); }
			if (RTVAL & (1 << 13)) { VEC_SET_ZERO_FLAG(5); }
			if (RTVAL & (1 << 14)) { VEC_SET_ZERO_FLAG(6); }
			if (RTVAL & (1 << 15)) { VEC_SET_ZERO_FLAG(7); }
			break;
		case 1:
			VEC_CLEAR_COMPARE_FLAGS();
			VEC_CLEAR_CLIP2_FLAGS();
			m_vflag[1][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[1][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[1][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[1][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[1][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[1][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[1][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[1][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { VEC_SET_COMPARE_FLAG(0); }
			if (RTVAL & (1 << 1)) { VEC_SET_COMPARE_FLAG(1); }
			if (RTVAL & (1 << 2)) { VEC_SET_COMPARE_FLAG(2); }
			if (RTVAL & (1 << 3)) { VEC_SET_COMPARE_FLAG(3); }
			if (RTVAL & (1 << 4)) { VEC_SET_COMPARE_FLAG(4); }
			if (RTVAL & (1 << 5)) { VEC_SET_COMPARE_FLAG(5); }
			if (RTVAL & (1 << 6)) { VEC_SET_COMPARE_FLAG(6); }
			if (RTVAL & (1 << 7)) { VEC_SET_COMPARE_FLAG(7); }
			m_vflag[4][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[4][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[4][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[4][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[4][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[4][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[4][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[4][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { VEC_SET_CLIP2_FLAG(0); }
			if (RTVAL & (1 << 9))  { VEC_SET_CLIP2_FLAG(1); }
			if (RTVAL & (1 << 10)) { VEC_SET_CLIP2_FLAG(2); }
			if (RTVAL & (1 << 11)) { VEC_SET_CLIP2_FLAG(3); }
			if (RTVAL & (1 << 12)) { VEC_SET_CLIP2_FLAG(4); }
			if (RTVAL & (1 << 13)) { VEC_SET_CLIP2_FLAG(5); }
			if (RTVAL & (1 << 14)) { VEC_SET_CLIP2_FLAG(6); }
			if (RTVAL & (1 << 15)) { VEC_SET_CLIP2_FLAG(7); }
			break;
		case 2:
			VEC_CLEAR_CLIP1_FLAGS();
			m_vflag[2][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[2][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[2][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[2][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[2][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[2][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[2][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[2][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { VEC_SET_CLIP1_FLAG(0); }
			if (RTVAL & (1 << 1)) { VEC_SET_CLIP1_FLAG(1); }
			if (RTVAL & (1 << 2)) { VEC_SET_CLIP1_FLAG(2); }
			if (RTVAL & (1 << 3)) { VEC_SET_CLIP1_FLAG(3); }
			if (RTVAL & (1 << 4)) { VEC_SET_CLIP1_FLAG(4); }
			if (RTVAL & (1 << 5)) { VEC_SET_CLIP1_FLAG(5); }
			if (RTVAL & (1 << 6)) { VEC_SET_CLIP1_FLAG(6); }
			if (RTVAL & (1 << 7)) { VEC_SET_CLIP1_FLAG(7); }
			break;
	}
}

static void cfunc_ctc2_simd(void *param)
{
	((rsp_device *)param)->ccfunc_ctc2_simd();
}
#endif

#if (!USE_SIMD || SIMUL_SIMD)
inline void rsp_device::ccfunc_ctc2_scalar()
{
	UINT32 op = m_rsp_state->arg0;
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

static void cfunc_ctc2_scalar(void *param)
{
	((rsp_device *)param)->ccfunc_ctc2_scalar();
}
#endif

/***************************************************************************
    CODE LOGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a RSP instruction
-------------------------------------------------*/

void rsp_device::log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op)
{
#if (RSP_LOG_UML)
	char buffer[100];
	rsp_dasm_one(buffer, pc, op);
	block->append_comment("%08X: %s", pc, buffer);                                  // comment
#endif
}
