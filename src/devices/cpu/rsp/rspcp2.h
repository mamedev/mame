// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,Tyler J. Stachecki
/***************************************************************************

    rspcp2.h

    Interface file for Reality Signal Processor (RSP) vector extensions.

***************************************************************************/

#pragma once

#ifndef __RSPCP2_H__
#define __RSPCP2_H__

#include "cpu/drcuml.h"
#include "rsp.h"
#include "rspdiv.h"

#define SIMD_OFF        (1)

#if (defined(__SSE2__) || defined(__SSE3__) || defined(__SSSE3__) || defined(__SSE4_1__) || defined(__SSE4_2__))
#define SSE_AVAILABLE   (1)
#else
#define SSE_AVAILABLE   (0)
#endif

#if (SSE_AVAILABLE || defined(_MSC_VER)) && defined(PTR64) && !SIMD_OFF
#define USE_SIMD    (1)
#else
#define USE_SIMD    (0)
#endif

#if USE_SIMD

#ifdef _MSC_VER
#define __align16 __declspec(align(16))
#else
#define __align16 __attribute__((aligned(16)))
#endif

#if (defined(__SSE4_2__) || defined(_MSC_VER))
#include <nmmintrin.h>
#elif (defined(__SSE4_1__) || defined(_MSC_VER))
#include <smmintrin.h>
#elif (defined(__SSSE3__) || defined(_MSC_VER))
#include <tmmintrin.h>
#elif (defined(__SSE3__ ) || defined(_MSC_VER))
#include <pmmintrin.h>
#else
#include <emmintrin.h>
#endif

typedef __m128i rsp_vec_t;
#endif

union VECTOR_REG
{
	UINT64 d[2];
	UINT32 l[4];
	UINT16 s[8];
	UINT8 b[16];
#if USE_SIMD
	rsp_vec_t v;
#endif
};

union ACCUMULATOR_REG
{
	UINT64 q;
	UINT32 l[2];
	UINT16 w[4];
};

struct compiler_state;

class rsp_cop2
{
	friend class rsp_device;

public:
	rsp_cop2(rsp_device &rsp, running_machine &machine);

protected:
	virtual void init();
	virtual void start();

	virtual int generate_cop2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }
	virtual int generate_lwc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }
	virtual int generate_swc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }

	virtual void state_string_export(const int index, std::string &str) const;

public:
	virtual ~rsp_cop2();

	virtual void lbv() { }
	virtual void lsv() { }
	virtual void llv() { }
	virtual void ldv() { }
	virtual void lqv() { }
	virtual void lrv() { }
	virtual void lpv() { }
	virtual void luv() { }
	virtual void lhv() { }
	virtual void lfv() { }
	virtual void lwv() { }
	virtual void ltv() { }
	virtual void sbv() { }
	virtual void ssv() { }
	virtual void slv() { }
	virtual void sdv() { }
	virtual void sqv() { }
	virtual void srv() { }
	virtual void spv() { }
	virtual void suv() { }
	virtual void shv() { }
	virtual void sfv() { }
	virtual void swv() { }
	virtual void stv() { }
	virtual void vmulf() { }
	virtual void vmulu() { }
	virtual void vmudl() { }
	virtual void vmudm() { }
	virtual void vmudn() { }
	virtual void vmudh() { }
	virtual void vmacf() { }
	virtual void vmacu() { }
	virtual void vmadl() { }
	virtual void vmadm() { }
	virtual void vmadn() { }
	virtual void vmadh() { }
	virtual void vadd() { }
	virtual void vsub() { }
	virtual void vabs() { }
	virtual void vaddc() { }
	virtual void vsubc() { }
	virtual void vaddb() { }
	virtual void vsaw() { }
	virtual void vlt() { }
	virtual void veq() { }
	virtual void vne() { }
	virtual void vge() { }
	virtual void vcl() { }
	virtual void vch() { }
	virtual void vcr() { }
	virtual void vmrg() { }
	virtual void vand() { }
	virtual void vnand() { }
	virtual void vor() { }
	virtual void vnor() { }
	virtual void vxor() { }
	virtual void vnxor() { }
	virtual void vrcp() { }
	virtual void vrcpl() { }
	virtual void vrcph() { }
	virtual void vmov() { }
	virtual void vrsql() { }
	virtual void vrsqh() { }
	virtual void vrsq() { }
	virtual void mfc2();
	virtual void cfc2();
	virtual void mtc2();
	virtual void ctc2();

	virtual void    handle_cop2(UINT32 op);

	void            log_instruction_execution();
	virtual void    cfunc_unimplemented_opcode() { }

	void            dump(UINT32 op);
	void            dump_dmem();

protected:
	virtual int     generate_vector_opcode(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }

	UINT16          SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive);

	// Data that needs to be stored close to the generated DRC code
	struct internal_rspcop2_state
	{
		UINT32      op;
	};

	internal_rspcop2_state *m_rspcop2_state;
	rsp_device&     m_rsp;
	running_machine& m_machine;
	UINT32          m_vres[8];          /* used for temporary vector results */

#if USE_SIMD
	__align16 VECTOR_REG      m_v[32];
#else
	VECTOR_REG      m_v[32];
#endif
	ACCUMULATOR_REG m_accum[8];
	UINT16          m_vflag[6][8];

	INT32           m_reciprocal_res;
	UINT32          m_reciprocal_high;
	INT32           m_dp_allowed;

#if USE_SIMD
	enum rsp_flags_t {
		RSP_VCO = 0,
		RSP_VCC = 1,
		RSP_VCE = 2
	};

	enum rsp_acc_t {
		RSP_ACC_LO = 16,
		RSP_ACC_MD = 8,
		RSP_ACC_HI = 0,
	};

	enum rsp_mem_request_type {
		RSP_MEM_REQUEST_NONE,
		RSP_MEM_REQUEST_INT_MEM,
		RSP_MEM_REQUEST_VECTOR,
		RSP_MEM_REQUEST_FOURTH,
		RSP_MEM_REQUEST_HALF,
		RSP_MEM_REQUEST_PACK,
		RSP_MEM_REQUEST_QUAD,
		RSP_MEM_REQUEST_REST,
		RSP_MEM_REQUEST_UPACK
	};

	union aligned_rsp_1vect_t {
		rsp_vec_t __align;
		UINT16 s[8];
	};

	union aligned_rsp_2vect_t {
		rsp_vec_t __align[2];
		UINT16 s[16];
	};

	union aligned_rsp_3vect_t {
		rsp_vec_t __align[3];
		UINT16 s[24];
	};

	__align16 aligned_rsp_1vect_t m_vdqm;
	__align16 aligned_rsp_2vect_t m_flags[3];
	__align16 aligned_rsp_3vect_t m_acc;
	UINT32 m_dp_flag;

	typedef struct
	{
		rsp_vec_t dummy_for_alignment;
		const UINT16 logic_mask[2][8];
		const UINT16 vrsq_mask_table[8][8];
		const UINT16 shuffle_keys[16][8];
		const UINT16 sll_b2l_keys[16][8];
		const UINT16 sll_l2b_keys[16][8];
		const UINT16 srl_b2l_keys[16][8];
		const UINT16 ror_b2l_keys[16][8];
		const UINT16 rol_l2b_keys[16][8];
		const UINT16 ror_l2b_keys[16][8];
		const UINT16 qr_lut[16][8];
		const UINT16 bdls_lut[4][4];
		const UINT16 word_reverse[8];
	} vec_helpers_t;

	static const vec_helpers_t m_vec_helpers;

	rsp_vec_t vec_load_and_shuffle_operand(const UINT16* src, UINT32 element);
	static inline UINT32 sign_extend_6(INT32 i) {
		return ((i << (32 - 7)) >> (32 - 7)) & 0xfff;
	}
	static inline rsp_vec_t vec_load_unshuffled_operand(const void* src)
	{
		return _mm_load_si128((rsp_vec_t*) src);
	}
	static inline void vec_write_operand(UINT16* dest, rsp_vec_t src)
	{
		_mm_store_si128((rsp_vec_t*) dest, src);
	}
	static inline rsp_vec_t read_acc_lo(const UINT16* acc)
	{
		return vec_load_unshuffled_operand(acc + sizeof(rsp_vec_t));
	}
	static inline rsp_vec_t read_acc_mid(const UINT16* acc)
	{
		return vec_load_unshuffled_operand(acc + (sizeof(rsp_vec_t) >> 1));
	}
	static inline rsp_vec_t read_acc_hi(const void* acc)
	{
		return vec_load_unshuffled_operand(acc);
	}
	static inline rsp_vec_t read_vcc_lo(const UINT16 *vcc)
	{
		return vec_load_unshuffled_operand(vcc + (sizeof(rsp_vec_t) >> 1));
	}
	static inline rsp_vec_t read_vcc_hi(const UINT16 *vcc)
	{
		return vec_load_unshuffled_operand(vcc);
	}
	static inline rsp_vec_t read_vco_lo(const UINT16 *vco)
	{
		return vec_load_unshuffled_operand(vco + (sizeof(rsp_vec_t) >> 1));
	}
	static inline rsp_vec_t read_vco_hi(const UINT16 *vco)
	{
		return vec_load_unshuffled_operand(vco);
	}
	static inline rsp_vec_t read_vce(const UINT16 *vce)
	{
		return vec_load_unshuffled_operand(vce + (sizeof(rsp_vec_t) >> 1));
	}
	static inline void write_acc_lo(UINT16 *acc, rsp_vec_t acc_lo)
	{
		return vec_write_operand(acc + sizeof(rsp_vec_t), acc_lo);
	}
	static inline void write_acc_mid(UINT16 *acc, rsp_vec_t acc_mid)
	{
		return vec_write_operand(acc + (sizeof(rsp_vec_t) >> 1), acc_mid);
	}
	static inline void write_acc_hi(UINT16 *acc, rsp_vec_t acc_hi)
	{
		return vec_write_operand(acc, acc_hi);
	}
	static inline void write_vcc_lo(UINT16 *vcc, rsp_vec_t vcc_lo)
	{
		return vec_write_operand(vcc + (sizeof(rsp_vec_t) >> 1), vcc_lo);
	}
	static inline void write_vcc_hi(UINT16 *vcc, rsp_vec_t vcc_hi)
	{
		return vec_write_operand(vcc, vcc_hi);
	}
	static inline void write_vco_lo(UINT16 *vcc, rsp_vec_t vco_lo)
	{
		return vec_write_operand(vcc + (sizeof(rsp_vec_t) >> 1), vco_lo);
	}
	static inline void write_vco_hi(UINT16 *vcc, rsp_vec_t vco_hi)
	{
		return vec_write_operand(vcc, vco_hi);
	}
	static inline void write_vce(UINT16 *vce, rsp_vec_t vce_r)
	{
		return vec_write_operand(vce + (sizeof(rsp_vec_t) >> 1), vce_r);
	}

	static inline INT16 get_flags(const UINT16 *flags)
	{
		return _mm_movemask_epi8(_mm_packs_epi16(_mm_load_si128((rsp_vec_t*) (flags + (sizeof(rsp_vec_t) >> 1))), _mm_load_si128((rsp_vec_t*) flags)));
	}

	static inline rsp_vec_t vec_zero()
	{
		return _mm_setzero_si128();
	}

	void vec_load_group1(UINT32 addr, UINT32 element, UINT16* regp, rsp_vec_t reg, rsp_vec_t dqm);
	void vec_load_group2(UINT32 addr, UINT32 element, UINT16* regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type);
	void vec_load_group4(UINT32 addr, UINT32 element, UINT16* regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type);
	void vec_store_group1(UINT32 addr, UINT32 element, UINT16* regp, rsp_vec_t reg, rsp_vec_t dqm);
	void vec_store_group2(UINT32 addr, UINT32 element, UINT16* regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type);
	void vec_store_group4(UINT32 addr, UINT32 element, UINT16* regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type);

#include "clamp.h"
#include "vabs.h"
#include "vadd.h"
#include "vaddc.h"
#include "vand.h"
#include "vch.h"
#include "vcmp.h"
#include "vcl.h"
#include "vcr.h"
#include "vdivh.h"
#include "vmac.h"
#include "vmov.h"
#include "vmrg.h"
#include "vmul.h"
#include "vmulh.h"
#include "vmull.h"
#include "vmulm.h"
#include "vmuln.h"
#include "vor.h"
#include "vrcpsq.h"
#include "vrsq.h"
#include "vsub.h"
#include "vsubc.h"
#include "vxor.h"
#include "vldst.h"
#endif

private:
	void            handle_lwc2(UINT32 op);
	void            handle_swc2(UINT32 op);
	void            handle_vector_ops(UINT32 op);

	UINT32          m_div_in;
	UINT32          m_div_out;
};

#endif /* __RSPCP2_H__ */
