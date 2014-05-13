/***************************************************************************

    rsp.h

    Interface file for the universal machine language-based
    Reality Signal Processor (RSP) emulator.

    Copyright the MESS team
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __RSP_H__
#define __RSP_H__

#define USE_SIMD        (0)
#define SIMUL_SIMD      (0)

#if USE_SIMD
#include <tmmintrin.h>
#endif

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	RSP_PC = 1,
	RSP_R0,
	RSP_R1,
	RSP_R2,
	RSP_R3,
	RSP_R4,
	RSP_R5,
	RSP_R6,
	RSP_R7,
	RSP_R8,
	RSP_R9,
	RSP_R10,
	RSP_R11,
	RSP_R12,
	RSP_R13,
	RSP_R14,
	RSP_R15,
	RSP_R16,
	RSP_R17,
	RSP_R18,
	RSP_R19,
	RSP_R20,
	RSP_R21,
	RSP_R22,
	RSP_R23,
	RSP_R24,
	RSP_R25,
	RSP_R26,
	RSP_R27,
	RSP_R28,
	RSP_R29,
	RSP_R30,
	RSP_R31,
	RSP_SR,
	RSP_NEXTPC,
	RSP_STEPCNT,
	RSP_V0,  RSP_V1,  RSP_V2,  RSP_V3,  RSP_V4,  RSP_V5,  RSP_V6,  RSP_V7,
	RSP_V8,  RSP_V9,  RSP_V10, RSP_V11, RSP_V12, RSP_V13, RSP_V14, RSP_V15,
	RSP_V16, RSP_V17, RSP_V18, RSP_V19, RSP_V20, RSP_V21, RSP_V22, RSP_V23,
	RSP_V24, RSP_V25, RSP_V26, RSP_V27, RSP_V28, RSP_V29, RSP_V30, RSP_V31
};

/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

void rspdrc_flush_drc_cache(device_t *device);
void rspdrc_set_options(device_t *device, UINT32 options);
void rspdrc_add_dmem(device_t *device, UINT32 *base);
void rspdrc_add_imem(device_t *device, UINT32 *base);

/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define REG_LO          32
#define REG_HI          33

#define RSREG           ((op >> 21) & 31)
#define RTREG           ((op >> 16) & 31)
#define RDREG           ((op >> 11) & 31)
#define SHIFT           ((op >> 6) & 31)

#define RSVAL           (rsp->r[RSREG])
#define RTVAL           (rsp->r[RTREG])
#define RDVAL           (rsp->r[RDREG])

#define FRREG           ((op >> 21) & 31)
#define FTREG           ((op >> 16) & 31)
#define FSREG           ((op >> 11) & 31)
#define FDREG           ((op >> 6) & 31)

#define IS_SINGLE(o)    (((o) & (1 << 21)) == 0)
#define IS_DOUBLE(o)    (((o) & (1 << 21)) != 0)
#define IS_FLOAT(o)     (((o) & (1 << 23)) == 0)
#define IS_INTEGRAL(o)  (((o) & (1 << 23)) != 0)

#define SIMMVAL         ((INT16)op)
#define UIMMVAL         ((UINT16)op)
#define LIMMVAL         (op & 0x03ffffff)

#define RSP_STATUS_HALT          0x0001
#define RSP_STATUS_BROKE         0x0002
#define RSP_STATUS_DMABUSY       0x0004
#define RSP_STATUS_DMAFULL       0x0008
#define RSP_STATUS_IOFULL        0x0010
#define RSP_STATUS_SSTEP         0x0020
#define RSP_STATUS_INTR_BREAK    0x0040
#define RSP_STATUS_SIGNAL0       0x0080
#define RSP_STATUS_SIGNAL1       0x0100
#define RSP_STATUS_SIGNAL2       0x0200
#define RSP_STATUS_SIGNAL3       0x0400
#define RSP_STATUS_SIGNAL4       0x0800
#define RSP_STATUS_SIGNAL5       0x1000
#define RSP_STATUS_SIGNAL6       0x2000
#define RSP_STATUS_SIGNAL7       0x4000

#define RSPDRC_STRICT_VERIFY    0x0001          /* verify all instructions */

union VECTOR_REG
{
	UINT64 d[2];
	UINT32 l[4];
	INT16 s[8];
	UINT8 b[16];
};

union ACCUMULATOR_REG
{
	INT64 q;
	INT32 l[2];
	INT16 w[4];
};

#define MCFG_RSP_DP_REG_R_CB(_devcb) \
	devcb = &rsp_cpu_device::static_set_dp_reg_r_callback(*device, DEVCB2_##_devcb);
 
#define MCFG_RSP_DP_REG_W_CB(_devcb) \
	devcb = &rsp_cpu_device::static_set_dp_reg_w_callback(*device, DEVCB2_##_devcb);
 
#define MCFG_RSP_SP_REG_R_CB(_devcb) \
	devcb = &rsp_cpu_device::static_set_sp_reg_r_callback(*device, DEVCB2_##_devcb);
 
#define MCFG_RSP_SP_REG_W_CB(_devcb) \
	devcb = &rsp_cpu_device::static_set_sp_reg_w_callback(*device, DEVCB2_##_devcb);

#define MCFG_RSP_SP_SET_STATUS_CB(_devcb) \
	devcb = &rsp_cpu_device::static_set_status_callback(*device, DEVCB2_##_devcb);

class rsp_cpu_device : public legacy_cpu_device
{
protected:
	// construction/destruction
	rsp_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, cpu_get_info_func info);
	
public:
	void resolve_cb();
	template<class _Object> static devcb2_base &static_set_dp_reg_r_callback(device_t &device, _Object object) { return downcast<rsp_cpu_device &>(device).dp_reg_r_func.set_callback(object); }
	template<class _Object> static devcb2_base &static_set_dp_reg_w_callback(device_t &device, _Object object) { return downcast<rsp_cpu_device &>(device).dp_reg_w_func.set_callback(object); }
	template<class _Object> static devcb2_base &static_set_sp_reg_r_callback(device_t &device, _Object object) { return downcast<rsp_cpu_device &>(device).sp_reg_r_func.set_callback(object); }
	template<class _Object> static devcb2_base &static_set_sp_reg_w_callback(device_t &device, _Object object) { return downcast<rsp_cpu_device &>(device).sp_reg_w_func.set_callback(object); }
	template<class _Object> static devcb2_base &static_set_status_callback(device_t &device, _Object object) { return downcast<rsp_cpu_device &>(device).sp_set_status_func.set_callback(object); }
	

	devcb2_read32 dp_reg_r_func;
	devcb2_write32 dp_reg_w_func;
	devcb2_read32 sp_reg_r_func;
	devcb2_write32 sp_reg_w_func;
	devcb2_write32 sp_set_status_func;	
};


struct rspimp_state;
struct rsp_state
{
	FILE *exec_output;

	UINT32 pc;
	UINT32 r[35];
	VECTOR_REG v[32];
	UINT16 vflag[6][8];

#if SIMUL_SIMD
	UINT32 old_r[35];
	UINT8 old_dmem[4096];

	UINT32 scalar_r[35];
	UINT8 scalar_dmem[4096];

	INT32 old_reciprocal_res;
	UINT32 old_reciprocal_high;
	INT32 old_dp_allowed;

	INT32 scalar_reciprocal_res;
	UINT32 scalar_reciprocal_high;
	INT32 scalar_dp_allowed;

	INT32 simd_reciprocal_res;
	UINT32 simd_reciprocal_high;
	INT32 simd_dp_allowed;
#endif

#if USE_SIMD
	// Mirror of v[] for now, to be used in parallel as
	// more vector ops are transitioned over
	__m128i xv[32];
	__m128i xvflag[6];
#endif
	UINT32 sr;
	UINT32 step_count;

	ACCUMULATOR_REG accum[8];
#if USE_SIMD
	__m128i accum_h;
	__m128i accum_m;
	__m128i accum_l;
	__m128i accum_ll;
#endif
	INT32 reciprocal_res;
	UINT32 reciprocal_high;
	INT32 dp_allowed;

	UINT32 ppc;
	UINT32 nextpc;

	device_irq_acknowledge_delegate irq_callback;
	rsp_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	int icount;

	UINT32 *dmem32;
	UINT16 *dmem16;
	UINT8 *dmem8;

	UINT32 *imem32;
	UINT16 *imem16;
	UINT8 *imem8;

	rspimp_state* impstate;
};

CPU_GET_INFO( rsp_int );

class rsp_int_device : public rsp_cpu_device
{
public:
	rsp_int_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type RSP_INT;

CPU_GET_INFO( rsp_drc );

class rsp_drc_device : public rsp_cpu_device
{
public:
	rsp_drc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type RSP_DRC;

extern const device_type RSP;

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

#endif /* __RSP_H__ */
