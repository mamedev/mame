// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppccom.c

    Common PowerPC definitions and functions

***************************************************************************/

#include "emu.h"
#include "ppccom.h"
#include "ppcfe.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_SPU              (0)
#define PRINTF_DECREMENTER      (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DOUBLE_SIGN     (U64(0x8000000000000000))
#define DOUBLE_EXP      (U64(0x7ff0000000000000))
#define DOUBLE_FRAC     (U64(0x000fffffffffffff))
#define DOUBLE_ZERO     (0)



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

/* lookup table for FP modes */
static const UINT8 fpmode_source[4] =
{
	uml::ROUND_ROUND,
	uml::ROUND_TRUNC,
	uml::ROUND_CEIL,
	uml::ROUND_FLOOR
};

/* flag lookup table for SZ */
static const UINT8 sz_cr_table_source[32] =
{
	/* ..... */ 0x4,
	/* ....C */ 0x4,
	/* ...V. */ 0x4,
	/* ...VC */ 0x4,
	/* ..Z.. */ 0x2,
	/* ..Z.C */ 0x2,
	/* ..ZV. */ 0x2,
	/* ..ZVC */ 0x2,
	/* .S... */ 0x8,
	/* .S..C */ 0x8,
	/* .S.V. */ 0x8,
	/* .S.VC */ 0x8,
	/* .SZ.. */ 0x2,
	/* .SZ.C */ 0x2,
	/* .SZV. */ 0x2,
	/* .SZVC */ 0x2,
	/* U.... */ 0x4,
	/* U...C */ 0x4,
	/* U..V. */ 0x4,
	/* U..VC */ 0x4,
	/* U.Z.. */ 0x2,
	/* U.Z.C */ 0x2,
	/* U.ZV. */ 0x2,
	/* U.ZVC */ 0x2,
	/* US... */ 0x8,
	/* US..C */ 0x8,
	/* US.V. */ 0x8,
	/* US.VC */ 0x8,
	/* USZ.. */ 0x2,
	/* USZ.C */ 0x2,
	/* USZV. */ 0x2,
	/* USZVC */ 0x2
};

/* flag lookup table for CMP */
static const UINT8 cmp_cr_table_source[32] =
{
	/* ..... */ 0x4,
	/* ....C */ 0x4,
	/* ...V. */ 0x8,
	/* ...VC */ 0x8,
	/* ..Z.. */ 0x2,
	/* ..Z.C */ 0x2,
	/* ..ZV. */ 0x2,
	/* ..ZVC */ 0x2,
	/* .S... */ 0x8,
	/* .S..C */ 0x8,
	/* .S.V. */ 0x4,
	/* .S.VC */ 0x4,
	/* .SZ.. */ 0x2,
	/* .SZ.C */ 0x2,
	/* .SZV. */ 0x2,
	/* .SZVC */ 0x2,
	/* U.... */ 0x4,
	/* U...C */ 0x4,
	/* U..V. */ 0x8,
	/* U..VC */ 0x8,
	/* U.Z.. */ 0x2,
	/* U.Z.C */ 0x2,
	/* U.ZV. */ 0x2,
	/* U.ZVC */ 0x2,
	/* US... */ 0x8,
	/* US..C */ 0x8,
	/* US.V. */ 0x4,
	/* US.VC */ 0x4,
	/* USZ.. */ 0x2,
	/* USZ.C */ 0x2,
	/* USZV. */ 0x2,
	/* USZVC */ 0x2
};

/* flag lookup table for CMPL */
static const UINT8 cmpl_cr_table_source[32] =
{
	/* ..... */ 0x4,
	/* ....C */ 0x8,
	/* ...V. */ 0x4,
	/* ...VC */ 0x8,
	/* ..Z.. */ 0x2,
	/* ..Z.C */ 0x2,
	/* ..ZV. */ 0x2,
	/* ..ZVC */ 0x2,
	/* .S... */ 0x4,
	/* .S..C */ 0x8,
	/* .S.V. */ 0x4,
	/* .S.VC */ 0x8,
	/* .SZ.. */ 0x2,
	/* .SZ.C */ 0x2,
	/* .SZV. */ 0x2,
	/* .SZVC */ 0x2,
	/* U.... */ 0x4,
	/* U...C */ 0x8,
	/* U..V. */ 0x4,
	/* U..VC */ 0x8,
	/* U.Z.. */ 0x2,
	/* U.Z.C */ 0x2,
	/* U.ZV. */ 0x2,
	/* U.ZVC */ 0x2,
	/* US... */ 0x4,
	/* US..C */ 0x8,
	/* US.V. */ 0x4,
	/* US.VC */ 0x8,
	/* USZ.. */ 0x2,
	/* USZ.C */ 0x2,
	/* USZV. */ 0x2,
	/* USZVC */ 0x2
};

/* flag lookup table for FCMP */
static const UINT8 fcmp_cr_table_source[32] =
{
	/* ..... */ 0x4,
	/* ....C */ 0x8,
	/* ...V. */ 0x4,
	/* ...VC */ 0x8,
	/* ..Z.. */ 0x2,
	/* ..Z.C */ 0xa,
	/* ..ZV. */ 0x2,
	/* ..ZVC */ 0xa,
	/* .S... */ 0x4,
	/* .S..C */ 0x8,
	/* .S.V. */ 0x4,
	/* .S.VC */ 0x8,
	/* .SZ.. */ 0x2,
	/* .SZ.C */ 0xa,
	/* .SZV. */ 0x2,
	/* .SZVC */ 0xa,
	/* U.... */ 0x5,
	/* U...C */ 0x9,
	/* U..V. */ 0x5,
	/* U..VC */ 0x9,
	/* U.Z.. */ 0x3,
	/* U.Z.C */ 0xb,
	/* U.ZV. */ 0x3,
	/* U.ZVC */ 0xb,
	/* US... */ 0x5,
	/* US..C */ 0x9,
	/* US.V. */ 0x5,
	/* US.VC */ 0x9,
	/* USZ.. */ 0x3,
	/* USZ.C */ 0xb,
	/* USZV. */ 0x3,
	/* USZVC */ 0xb
};


const device_type PPC601 = &device_creator<ppc601_device>;
const device_type PPC602 = &device_creator<ppc602_device>;
const device_type PPC603 = &device_creator<ppc603_device>;
const device_type PPC603E = &device_creator<ppc603e_device>;
const device_type PPC603R = &device_creator<ppc603r_device>;
const device_type PPC604 = &device_creator<ppc604_device>;
const device_type MPC8240 = &device_creator<mpc8240_device>;
const device_type PPC403GA = &device_creator<ppc403ga_device>;
const device_type PPC403GCX = &device_creator<ppc403gcx_device>;
const device_type PPC405GP = &device_creator<ppc405gp_device>;


ppc_device::ppc_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, int address_bits, int data_bits, powerpc_flavor flavor, UINT32 cap, UINT32 tb_divisor, address_map_constructor internal_map)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, data_bits, address_bits, 0, internal_map)
	, c_bus_frequency(0)
	, m_core(nullptr)
	, m_bus_freq_multiplier(1)
	, m_vtlb(nullptr)
	, m_flavor(flavor)
	, m_cap(cap)
	, m_tb_divisor(tb_divisor)
	, m_cache(CACHE_SIZE + sizeof(internal_ppc_state))
	, m_drcuml(nullptr)
	, m_drcfe(nullptr)
	, m_drcoptions(0)
{
	m_program_config.m_logaddr_width = 32;
	m_program_config.m_page_shift = POWERPC_MIN_PAGE_SHIFT;
}

//ppc403_device::ppc403_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
//  : ppc_device(mconfig, PPC403, "PPC403", tag, owner, clock, "ppc403", 32?, 64?)
//{
//}
//
//ppc405_device::ppc405_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
//  : ppc_device(mconfig, PPC405, "PPC405", tag, owner, clock, "ppc405", 32?, 64?)
//{
//}

ppc603_device::ppc603_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC603, "PowerPC 603", tag, owner, clock, "ppc603", 32, 64, PPC_MODEL_603, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, nullptr)
{
}

ppc603e_device::ppc603e_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC603E, "PowerPC 603e", tag, owner, clock, "ppc603e", 32, 64, PPC_MODEL_603E, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, nullptr)
{
}

ppc603r_device::ppc603r_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC603R, "PowerPC 603R", tag, owner, clock, "ppc603r", 32, 64, PPC_MODEL_603R, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, nullptr)
{
}

ppc602_device::ppc602_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC602, "PowerPC 602", tag, owner, clock, "ppc602", 32, 64, PPC_MODEL_602, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, nullptr)
{
}

mpc8240_device::mpc8240_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, MPC8240, "PowerPC MPC8240", tag, owner, clock, "mpc8240", 32, 64, PPC_MODEL_MPC8240, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4/* unknown */, nullptr)
{
}

ppc601_device::ppc601_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC601, "PowerPC 601", tag, owner, clock, "ppc601", 32, 64, PPC_MODEL_601, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_MFIOC | PPCCAP_601BAT, 0/* no TB */, nullptr)
{
}

ppc604_device::ppc604_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC604, "PowerPC 604", tag, owner, clock, "ppc604", 32, 64, PPC_MODEL_604, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_604_MMU, 4, nullptr)
{
}

static ADDRESS_MAP_START( internal_ppc4xx, AS_PROGRAM, 32, ppc4xx_device )
	AM_RANGE(0x40000000, 0x4000000f) AM_READWRITE8(ppc4xx_spu_r, ppc4xx_spu_w, 0xffffffff)
ADDRESS_MAP_END

ppc4xx_device::ppc4xx_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, powerpc_flavor flavor, UINT32 cap, UINT32 tb_divisor)
	: ppc_device(mconfig, type, name, tag, owner, clock, shortname, 31, 32, flavor, cap, tb_divisor, ADDRESS_MAP_NAME(internal_ppc4xx))
{
}

ppc403ga_device::ppc403ga_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, PPC403GA, "PowerPC 403GA", tag, owner, clock, "ppc403ga", PPC_MODEL_403GA, PPCCAP_4XX, 1)
{
}

ppc403gcx_device::ppc403gcx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, PPC403GCX, "PowerPC 403GCX", tag, owner, clock, "ppc403gcx", PPC_MODEL_403GCX, PPCCAP_4XX, 1)
{
}

ppc405gp_device::ppc405gp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, PPC405GP, "PowerPC 405GP", tag, owner, clock, "ppc405gp", PPC_MODEL_405GP, PPCCAP_4XX | PPCCAP_VEA, 1)
{
}


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    page_access_allowed - return true if we are
    allowed to access memory based on the type
    of access and the protection bits
-------------------------------------------------*/

static inline int page_access_allowed(int transtype, UINT8 key, UINT8 protbits)
{
	if (key == 0)
		return (transtype == TRANSLATE_WRITE) ? (protbits != 3) : TRUE;
	else
		return (transtype == TRANSLATE_WRITE) ? (protbits == 2) : (protbits != 0);
}


/*-------------------------------------------------
    get_cr - return the current CR value
-------------------------------------------------*/

inline UINT32 ppc_device::get_cr()
{
	return  ((m_core->cr[0] & 0x0f) << 28) |
			((m_core->cr[1] & 0x0f) << 24) |
			((m_core->cr[2] & 0x0f) << 20) |
			((m_core->cr[3] & 0x0f) << 16) |
			((m_core->cr[4] & 0x0f) << 12) |
			((m_core->cr[5] & 0x0f) << 8) |
			((m_core->cr[6] & 0x0f) << 4) |
			((m_core->cr[7] & 0x0f) << 0);
}


/*-------------------------------------------------
    set_cr - set the current CR value
-------------------------------------------------*/

inline void ppc_device::set_cr(UINT32 value)
{
	m_core->cr[0] = value >> 28;
	m_core->cr[1] = value >> 24;
	m_core->cr[2] = value >> 20;
	m_core->cr[3] = value >> 16;
	m_core->cr[4] = value >> 12;
	m_core->cr[5] = value >> 8;
	m_core->cr[6] = value >> 4;
	m_core->cr[7] = value >> 0;
}


/*-------------------------------------------------
    get_xer - return the current XER value
-------------------------------------------------*/

inline UINT32 ppc_device::get_xer()
{
	return m_core->spr[SPR_XER] | (m_core->xerso << 31);
}


/*-------------------------------------------------
    set_xer - set the current XER value
-------------------------------------------------*/

inline void ppc_device::set_xer(UINT32 value)
{
	m_core->spr[SPR_XER] = value & ~XER_SO;
	m_core->xerso = value >> 31;
}


/*-------------------------------------------------
    get_timebase - return the current timebase
    value
-------------------------------------------------*/

inline UINT64 ppc_device::get_timebase()
{
	if (!m_tb_divisor)
	{
		return (total_cycles() - m_tb_zero_cycles);
	}

	return (total_cycles() - m_tb_zero_cycles) / m_tb_divisor;
}


/*-------------------------------------------------
    set_timebase - set the timebase
-------------------------------------------------*/

inline void ppc_device::set_timebase(UINT64 newtb)
{
	m_tb_zero_cycles = total_cycles() - newtb * m_tb_divisor;
}


/*-------------------------------------------------
    get_decremeter - return the current
    decrementer value
-------------------------------------------------*/

inline UINT32 ppc_device::get_decrementer()
{
	INT64 cycles_until_zero = m_dec_zero_cycles - total_cycles();
	cycles_until_zero = MAX(cycles_until_zero, 0);

	if (!m_tb_divisor)
	{
		return 0;
	}

	return cycles_until_zero / m_tb_divisor;
}


/*-------------------------------------------------
    set_decrementer - set the decremeter
-------------------------------------------------*/

inline void ppc_device::set_decrementer(UINT32 newdec)
{
	UINT64 cycles_until_done = ((UINT64)newdec + 1) * m_tb_divisor;
	UINT32 curdec = get_decrementer();

	if (!m_tb_divisor)
	{
		return;
	}

	if (PRINTF_DECREMENTER)
	{
		UINT64 total = total_cycles();
		osd_printf_debug("set_decrementer: olddec=%08X newdec=%08X divisor=%d totalcyc=%08X%08X timer=%08X%08X\n",
				curdec, newdec, m_tb_divisor,
				(UINT32)(total >> 32), (UINT32)total, (UINT32)(cycles_until_done >> 32), (UINT32)cycles_until_done);
	}

	m_dec_zero_cycles = total_cycles() + cycles_until_done;
	m_decrementer_int_timer->adjust(cycles_to_attotime(cycles_until_done));

	if ((INT32)curdec >= 0 && (INT32)newdec < 0)
		m_core->irq_pending |= 0x02;
}


#if 0
/*-------------------------------------------------
    is_nan_double - is a double value a NaN
-------------------------------------------------*/

static inline int is_nan_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) );
}
#endif


/*-------------------------------------------------
    is_qnan_double - is a double value a
    quiet NaN
-------------------------------------------------*/

static inline int is_qnan_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & U64(0x0007fffffffffff)) == U64(0x000000000000000)) &&
			((xi & U64(0x000800000000000)) == U64(0x000800000000000)) );
}


#if 0
/*-------------------------------------------------
    is_snan_double - is a double value a
    signaling NaN
-------------------------------------------------*/

static inline int is_snan_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) &&
			((xi & U64(0x0008000000000000)) == DOUBLE_ZERO) );
}
#endif


/*-------------------------------------------------
    is_infinity_double - is a double value
    infinity
-------------------------------------------------*/

static inline int is_infinity_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) == DOUBLE_ZERO) );
}


/*-------------------------------------------------
    is_normalized_double - is a double value
    normalized
-------------------------------------------------*/

static inline int is_normalized_double(double x)
{
	UINT64 exp;
	UINT64 xi = *(UINT64*)&x;
	exp = (xi & DOUBLE_EXP) >> 52;

	return (exp >= 1) && (exp <= 2046);
}


/*-------------------------------------------------
    is_denormalized_double - is a double value
    denormalized
-------------------------------------------------*/

static inline int is_denormalized_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return( ((xi & DOUBLE_EXP) == 0) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) );
}


/*-------------------------------------------------
    sign_double - return sign of a double value
-------------------------------------------------*/

static inline int sign_double(double x)
{
	UINT64 xi = *(UINT64*)&x;
	return ((xi & DOUBLE_SIGN) != 0);
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

/*-------------------------------------------------
    device_start - initialize the powerpc_state
    structure based on the configured type
-------------------------------------------------*/

void ppc_device::device_start()
{
	/* allocate the core from the near cache */
	m_core = (internal_ppc_state *)m_cache.alloc_near(sizeof(internal_ppc_state));
	memset(m_core, 0, sizeof(internal_ppc_state));

	m_entry = nullptr;
	m_nocode = nullptr;
	m_out_of_cycles = nullptr;
	m_tlb_mismatch = nullptr;
	m_swap_tgpr = nullptr;
	memset(m_lsw, 0, sizeof(m_lsw));
	memset(m_stsw, 0, sizeof(m_stsw));
	memset(m_read8, 0, sizeof(m_read8));
	memset(m_write8, 0, sizeof(m_write8));
	memset(m_read16, 0, sizeof(m_read16));
	memset(m_read16mask, 0, sizeof(m_read16mask));
	memset(m_write16, 0, sizeof(m_write16));
	memset(m_write16mask, 0, sizeof(m_write16mask));
	memset(m_read32, 0, sizeof(m_read32));
	memset(m_read32align, 0, sizeof(m_read32align));
	memset(m_read32mask, 0, sizeof(m_read32mask));
	memset(m_write32, 0, sizeof(m_write32));
	memset(m_write32align, 0, sizeof(m_write32align));
	memset(m_write32mask, 0, sizeof(m_write32mask));
	memset(m_read64, 0, sizeof(m_read64));
	memset(m_read64mask, 0, sizeof(m_read64mask));
	memset(m_write64, 0, sizeof(m_write64));
	memset(m_write64mask, 0, sizeof(m_write64mask));
	memset(m_exception, 0, sizeof(m_exception));
	memset(m_exception_norecover, 0, sizeof(m_exception_norecover));

	/* initialize the implementation state tables */
	memcpy(m_fpmode, fpmode_source, sizeof(fpmode_source));
	memcpy(m_sz_cr_table, sz_cr_table_source, sizeof(sz_cr_table_source));
	memcpy(m_cmp_cr_table, cmp_cr_table_source, sizeof(cmp_cr_table_source));
	memcpy(m_cmpl_cr_table, cmpl_cr_table_source, sizeof(cmpl_cr_table_source));
	memcpy(m_fcmp_cr_table, fcmp_cr_table_source, sizeof(fcmp_cr_table_source));

	/* initialize based on the config */
	m_ppc_tb_base_icount = 0;
	m_ppc_dec_base_icount = 0;
	m_ppc_dec_trigger_cycle = 0;
	m_bus_freq_multiplier = 0;

	m_npc = 0;
	memset(m_dcr, 0, sizeof(m_dcr));

	m_lr = 0;
	m_ctr = 0;
	m_xer = 0;
	m_pvr = 0;
	m_srr0 = 0;
	m_srr1 = 0;
	m_srr2 = 0;
	m_srr3 = 0;
	m_hid0 = 0;
	m_hid1 = 0;
	m_hid2 = 0;
	m_sdr1 = 0;
	memset(m_sprg, 0, sizeof(m_sprg));

	m_dsisr = 0;
	m_dar = 0;
	m_ear = 0;
	m_dmiss = 0;
	m_dcmp = 0;
	m_hash1 = 0;
	m_hash2 = 0;
	m_imiss = 0;
	m_icmp = 0;
	m_rpa = 0;

	memset(m_ibat, 0, sizeof(m_ibat));
	memset(m_dbat, 0, sizeof(m_dbat));

	m_evpr = 0;
	m_exier = 0;
	m_exisr = 0;
	m_bear = 0;
	m_besr = 0;
	m_iocr = 0;
	memset(m_br, 0, sizeof(m_br));
	m_iabr = 0;
	m_esr = 0;
	m_iccr = 0;
	m_dccr = 0;
	m_pit = 0;
	m_pit_counter = 0;
	m_pit_int_enable = 0;
	m_tsr = 0;
	m_dbsr = 0;
	m_sgr = 0;
	m_pid = 0;
	m_pbl1 = 0;
	m_pbl2 = 0;
	m_pbu1 = 0;
	m_pbu2 = 0;
	m_fit_bit = 0;
	m_fit_int_enable = 0;
	m_wdt_bit = 0;
	m_wdt_int_enable = 0;
	m_dac1 = 0;
	m_dac2 = 0;
	m_iac1 = 0;
	m_iac2 = 0;

	memset(&m_spu_old, 0, sizeof(m_spu_old));
	memset(m_dma, 0, sizeof(m_dma));
	m_dmasr = 0;

	m_reserved = 0;
	m_reserved_address = 0;
	m_interrupt_pending = 0;
	m_tb = 0;
	m_dec = 0;
	m_dec_frac = 0;
	memset(m_fpr, 0, sizeof(m_fpr));
	m_lt = 0;
	m_sp = 0;
	m_tcr = 0;
	m_ibr = 0;
	m_esasrr = 0;
	m_sebr = 0;
	m_ser = 0;

	memset(&m_spu, 0, sizeof(m_spu));
	m_pit_reload = 0;
	m_irqstate = 0;
	memset(m_buffered_dma_rate, 0, sizeof(m_buffered_dma_rate));
	m_codexor = 0;
	m_system_clock = 0;
	m_cpu_clock = 0;
	m_tb_zero_cycles = 0;
	m_dec_zero_cycles = 0;

	m_arg1 = 0;
	m_fastram_select = 0;
	memset(m_fastram, 0, sizeof(m_fastram));
	m_hotspot_select = 0;
	memset(m_hotspot, 0, sizeof(m_hotspot));

	m_debugger_temp = 0;

	m_cache_line_size = 32;
	m_cpu_clock = clock();
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_system_clock = c_bus_frequency != 0 ? c_bus_frequency : clock();
	m_dcr_read_func = read32_delegate();
	m_dcr_write_func = write32_delegate();

	m_tb_divisor = (m_tb_divisor * clock() + m_system_clock / 2 - 1) / m_system_clock;
	m_codexor = 0;
	if (!(m_cap & PPCCAP_4XX) && space_config()->m_endianness != ENDIANNESS_NATIVE)
		m_codexor = 4;

	/* allocate the virtual TLB */
	m_vtlb = vtlb_alloc(this, AS_PROGRAM, (m_cap & PPCCAP_603_MMU) ? PPC603_FIXED_TLB_ENTRIES : 0, POWERPC_TLB_ENTRIES);

	/* allocate a timer for the compare interrupt */
	if ((m_cap & PPCCAP_OEA) && (m_tb_divisor))
		m_decrementer_int_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::decrementer_int_callback), this));

	/* and for the 4XX interrupts if needed */
	if (m_cap & PPCCAP_4XX)
	{
		m_fit_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::ppc4xx_fit_callback), this));
		m_pit_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::ppc4xx_pit_callback), this));
		m_spu.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::ppc4xx_spu_callback), this));
	}

	if (m_cap & PPCCAP_4XX)
	{
		m_buffered_dma_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this));
		m_buffered_dma_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this));
		m_buffered_dma_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this));
		m_buffered_dma_timer[3] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this));

		m_buffered_dma_rate[0] = 10000;
		m_buffered_dma_rate[1] = 10000;
		m_buffered_dma_rate[2] = 10000;
		m_buffered_dma_rate[3] = 10000;
	}

	/* register for save states */
	save_item(NAME(m_core->pc));
	save_item(NAME(m_core->r));
	save_item(NAME(m_core->f));
	save_item(NAME(m_core->cr));
	save_item(NAME(m_core->xerso));
	save_item(NAME(m_core->fpscr));
	save_item(NAME(m_core->msr));
	save_item(NAME(m_core->sr));
	save_item(NAME(m_core->spr));
	save_item(NAME(m_dcr));
	if (m_cap & PPCCAP_4XX)
	{
		save_item(NAME(m_spu.regs));
		save_item(NAME(m_spu.txbuf));
		save_item(NAME(m_spu.rxbuf));
		save_item(NAME(m_spu.rxbuffer));
		save_item(NAME(m_spu.rxin));
		save_item(NAME(m_spu.rxout));
		save_item(NAME(m_pit_reload));
		save_item(NAME(m_irqstate));
	}
	if (m_cap & PPCCAP_603_MMU)
	{
		save_item(NAME(m_core->mmu603_cmp));
		save_item(NAME(m_core->mmu603_hash));
		save_item(NAME(m_core->mmu603_r));
	}
	save_item(NAME(m_core->irq_pending));
	save_item(NAME(m_tb_zero_cycles));
	save_item(NAME(m_dec_zero_cycles));

	// Register debugger state
	state_add(PPC_PC,    "PC", m_core->pc).formatstr("%08X");
	state_add(PPC_MSR,   "MSR", m_core->msr).formatstr("%08X");
	state_add(PPC_CR,    "CR", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(PPC_LR,    "LR", m_core->spr[SPR_LR]).formatstr("%08X");
	state_add(PPC_CTR,   "CTR", m_core->spr[SPR_CTR]).formatstr("%08X");
	state_add(PPC_XER,   "XER", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(PPC_SRR0,  "SRR0", m_core->spr[SPROEA_SRR0]).formatstr("%08X");
	state_add(PPC_SRR1,  "SRR1", m_core->spr[SPROEA_SRR1]).formatstr("%08X");
	state_add(PPC_SPRG0, "SPRG0", m_core->spr[SPROEA_SPRG0]).formatstr("%08X");
	state_add(PPC_SPRG1, "SPRG1", m_core->spr[SPROEA_SPRG1]).formatstr("%08X");
	state_add(PPC_SPRG2, "SPRG2", m_core->spr[SPROEA_SPRG2]).formatstr("%08X");
	state_add(PPC_SPRG3, "SPRG3", m_core->spr[SPROEA_SPRG3]).formatstr("%08X");
	state_add(PPC_SDR1,  "SDR1", m_core->spr[SPROEA_SDR1]).formatstr("%08X");
	state_add(PPC_EXIER, "EXIER", m_dcr[DCR4XX_EXIER]).formatstr("%08X");
	state_add(PPC_EXISR, "EXISR", m_dcr[DCR4XX_EXISR]).formatstr("%08X");
	state_add(PPC_EVPR,  "EVPR", m_core->spr[SPR4XX_EVPR]).formatstr("%08X");
	state_add(PPC_IOCR,  "IOCR", m_dcr[DCR4XX_EXISR]).formatstr("%08X");
	state_add(PPC_TBH,   "TBH", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(PPC_TBL,   "TBL", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(PPC_DEC,   "DEC", m_debugger_temp).callimport().callexport().formatstr("%08X");

	state_add(PPC_SR0,   "SR0", m_core->sr[0]).formatstr("%08X");
	state_add(PPC_SR1,   "SR1", m_core->sr[1]).formatstr("%08X");
	state_add(PPC_SR2,   "SR2", m_core->sr[2]).formatstr("%08X");
	state_add(PPC_SR3,   "SR3", m_core->sr[3]).formatstr("%08X");
	state_add(PPC_SR4,   "SR4", m_core->sr[4]).formatstr("%08X");
	state_add(PPC_SR5,   "SR5", m_core->sr[5]).formatstr("%08X");
	state_add(PPC_SR6,   "SR6", m_core->sr[6]).formatstr("%08X");
	state_add(PPC_SR7,   "SR7", m_core->sr[7]).formatstr("%08X");
	state_add(PPC_SR8,   "SR8", m_core->sr[8]).formatstr("%08X");
	state_add(PPC_SR9,   "SR9", m_core->sr[9]).formatstr("%08X");
	state_add(PPC_SR10,  "SR10", m_core->sr[10]).formatstr("%08X");
	state_add(PPC_SR11,  "SR11", m_core->sr[11]).formatstr("%08X");
	state_add(PPC_SR12,  "SR12", m_core->sr[12]).formatstr("%08X");
	state_add(PPC_SR13,  "SR13", m_core->sr[13]).formatstr("%08X");
	state_add(PPC_SR14,  "SR14", m_core->sr[14]).formatstr("%08X");
	state_add(PPC_SR15,  "SR15", m_core->sr[15]).formatstr("%08X");

	state_add(PPC_R0,    "R0", m_core->r[0]).formatstr("%08X");
	state_add(PPC_R1,    "R1", m_core->r[1]).formatstr("%08X");
	state_add(PPC_R2,    "R2", m_core->r[2]).formatstr("%08X");
	state_add(PPC_R3,    "R3", m_core->r[3]).formatstr("%08X");
	state_add(PPC_R4,    "R4", m_core->r[4]).formatstr("%08X");
	state_add(PPC_R5,    "R5", m_core->r[5]).formatstr("%08X");
	state_add(PPC_R6,    "R6", m_core->r[6]).formatstr("%08X");
	state_add(PPC_R7,    "R7", m_core->r[7]).formatstr("%08X");
	state_add(PPC_R8,    "R8", m_core->r[8]).formatstr("%08X");
	state_add(PPC_R9,    "R9", m_core->r[9]).formatstr("%08X");
	state_add(PPC_R10,   "R10", m_core->r[10]).formatstr("%08X");
	state_add(PPC_R11,   "R11", m_core->r[11]).formatstr("%08X");
	state_add(PPC_R12,   "R12", m_core->r[12]).formatstr("%08X");
	state_add(PPC_R13,   "R13", m_core->r[13]).formatstr("%08X");
	state_add(PPC_R14,   "R14", m_core->r[14]).formatstr("%08X");
	state_add(PPC_R15,   "R15", m_core->r[15]).formatstr("%08X");
	state_add(PPC_R16,   "R16", m_core->r[16]).formatstr("%08X");
	state_add(PPC_R17,   "R17", m_core->r[17]).formatstr("%08X");
	state_add(PPC_R18,   "R18", m_core->r[18]).formatstr("%08X");
	state_add(PPC_R19,   "R19", m_core->r[19]).formatstr("%08X");
	state_add(PPC_R20,   "R20", m_core->r[20]).formatstr("%08X");
	state_add(PPC_R21,   "R21", m_core->r[21]).formatstr("%08X");
	state_add(PPC_R22,   "R22", m_core->r[22]).formatstr("%08X");
	state_add(PPC_R23,   "R23", m_core->r[23]).formatstr("%08X");
	state_add(PPC_R24,   "R24", m_core->r[24]).formatstr("%08X");
	state_add(PPC_R25,   "R25", m_core->r[25]).formatstr("%08X");
	state_add(PPC_R26,   "R26", m_core->r[26]).formatstr("%08X");
	state_add(PPC_R27,   "R27", m_core->r[27]).formatstr("%08X");
	state_add(PPC_R28,   "R28", m_core->r[28]).formatstr("%08X");
	state_add(PPC_R29,   "R29", m_core->r[29]).formatstr("%08X");
	state_add(PPC_R30,   "R30", m_core->r[30]).formatstr("%08X");
	state_add(PPC_R31,   "R31", m_core->r[31]).formatstr("%08X");

	state_add(PPC_F0,    "F0", m_core->f[0]).formatstr("%12s");
	state_add(PPC_F1,    "F1", m_core->f[1]).formatstr("%12s");
	state_add(PPC_F2,    "F2", m_core->f[2]).formatstr("%12s");
	state_add(PPC_F3,    "F3", m_core->f[3]).formatstr("%12s");
	state_add(PPC_F4,    "F4", m_core->f[4]).formatstr("%12s");
	state_add(PPC_F5,    "F5", m_core->f[5]).formatstr("%12s");
	state_add(PPC_F6,    "F6", m_core->f[6]).formatstr("%12s");
	state_add(PPC_F7,    "F7", m_core->f[7]).formatstr("%12s");
	state_add(PPC_F8,    "F8", m_core->f[8]).formatstr("%12s");
	state_add(PPC_F9,    "F9", m_core->f[9]).formatstr("%12s");
	state_add(PPC_F10,   "F10", m_core->f[10]).formatstr("%12s");
	state_add(PPC_F11,   "F11", m_core->f[11]).formatstr("%12s");
	state_add(PPC_F12,   "F12", m_core->f[12]).formatstr("%12s");
	state_add(PPC_F13,   "F13", m_core->f[13]).formatstr("%12s");
	state_add(PPC_F14,   "F14", m_core->f[14]).formatstr("%12s");
	state_add(PPC_F15,   "F15", m_core->f[15]).formatstr("%12s");
	state_add(PPC_F16,   "F16", m_core->f[16]).formatstr("%12s");
	state_add(PPC_F17,   "F17", m_core->f[17]).formatstr("%12s");
	state_add(PPC_F18,   "F18", m_core->f[18]).formatstr("%12s");
	state_add(PPC_F19,   "F19", m_core->f[19]).formatstr("%12s");
	state_add(PPC_F20,   "F20", m_core->f[20]).formatstr("%12s");
	state_add(PPC_F21,   "F21", m_core->f[21]).formatstr("%12s");
	state_add(PPC_F22,   "F22", m_core->f[22]).formatstr("%12s");
	state_add(PPC_F23,   "F23", m_core->f[23]).formatstr("%12s");
	state_add(PPC_F24,   "F24", m_core->f[24]).formatstr("%12s");
	state_add(PPC_F25,   "F25", m_core->f[25]).formatstr("%12s");
	state_add(PPC_F26,   "F26", m_core->f[26]).formatstr("%12s");
	state_add(PPC_F27,   "F27", m_core->f[27]).formatstr("%12s");
	state_add(PPC_F28,   "F28", m_core->f[28]).formatstr("%12s");
	state_add(PPC_F29,   "F29", m_core->f[29]).formatstr("%12s");
	state_add(PPC_F30,   "F30", m_core->f[30]).formatstr("%12s");
	state_add(PPC_F31,   "F31", m_core->f[31]).formatstr("%12s");
	state_add(PPC_FPSCR, "FPSCR", m_core->fpscr).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_core->pc).noshow();
	state_add(STATE_GENSP, "GENSP", m_core->r[31]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).noshow().formatstr("%1s");

	m_icountptr = &m_core->icount;

	UINT32 flags = 0;
	/* initialize the UML generator */
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, flags, 8, 32, 2);

	/* add symbols for our stuff */
	m_drcuml->symbol_add(&m_core->pc, sizeof(m_core->pc), "pc");
	m_drcuml->symbol_add(&m_core->icount, sizeof(m_core->icount), "icount");
	for (int regnum = 0; regnum < 32; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		m_drcuml->symbol_add(&m_core->r[regnum], sizeof(m_core->r[regnum]), buf);
		sprintf(buf, "fpr%d", regnum);
		m_drcuml->symbol_add(&m_core->f[regnum], sizeof(m_core->f[regnum]), buf);
	}
	for (int regnum = 0; regnum < 8; regnum++)
	{
		char buf[10];
		sprintf(buf, "cr%d", regnum);
		m_drcuml->symbol_add(&m_core->cr[regnum], sizeof(m_core->cr[regnum]), buf);
	}
	m_drcuml->symbol_add(&m_core->xerso, sizeof(m_core->xerso), "xerso");
	m_drcuml->symbol_add(&m_core->fpscr, sizeof(m_core->fpscr), "fpscr");
	m_drcuml->symbol_add(&m_core->msr, sizeof(m_core->msr), "msr");
	m_drcuml->symbol_add(&m_core->sr, sizeof(m_core->sr), "sr");
	m_drcuml->symbol_add(&m_core->spr[SPR_XER], sizeof(m_core->spr[SPR_XER]), "xer");
	m_drcuml->symbol_add(&m_core->spr[SPR_LR], sizeof(m_core->spr[SPR_LR]), "lr");
	m_drcuml->symbol_add(&m_core->spr[SPR_CTR], sizeof(m_core->spr[SPR_CTR]), "ctr");
	m_drcuml->symbol_add(&m_core->spr, sizeof(m_core->spr), "spr");
	m_drcuml->symbol_add(&m_dcr, sizeof(m_dcr), "dcr");
	m_drcuml->symbol_add(&m_core->param0, sizeof(m_core->param0), "param0");
	m_drcuml->symbol_add(&m_core->param1, sizeof(m_core->param1), "param1");
	m_drcuml->symbol_add(&m_core->irq_pending, sizeof(m_core->irq_pending), "irq_pending");
	m_drcuml->symbol_add(&m_core->mode, sizeof(m_core->mode), "mode");
	m_drcuml->symbol_add(&m_core->arg0, sizeof(m_core->arg0), "arg0");
	m_drcuml->symbol_add(&m_arg1, sizeof(m_arg1), "arg1");
	m_drcuml->symbol_add(&m_core->updateaddr, sizeof(m_core->updateaddr), "updateaddr");
	m_drcuml->symbol_add(&m_core->swcount, sizeof(m_core->swcount), "swcount");
	m_drcuml->symbol_add(&m_core->tempaddr, sizeof(m_core->tempaddr), "tempaddr");
	m_drcuml->symbol_add(&m_core->tempdata, sizeof(m_core->tempdata), "tempdata");
	m_drcuml->symbol_add(&m_core->fp0, sizeof(m_core->fp0), "fp0");
	m_drcuml->symbol_add(&m_fpmode, sizeof(m_fpmode), "fpmode");
	m_drcuml->symbol_add(&m_sz_cr_table, sizeof(m_sz_cr_table), "sz_cr_table");
	m_drcuml->symbol_add(&m_cmp_cr_table, sizeof(m_cmp_cr_table), "cmp_cr_table");
	m_drcuml->symbol_add(&m_cmpl_cr_table, sizeof(m_cmpl_cr_table), "cmpl_cr_table");
	m_drcuml->symbol_add(&m_fcmp_cr_table, sizeof(m_fcmp_cr_table), "fcmp_cr_table");

	/* initialize the front-end helper */
	m_drcfe = std::make_unique<ppc_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);

	/* compute the register parameters */
	for (int regnum = 0; regnum < 32; regnum++)
	{
		m_regmap[regnum] = uml::mem(&m_core->r[regnum]);
		m_fdregmap[regnum] = uml::mem(&m_core->f[regnum]);
	}

	/* if we have registers to spare, assign r0, r1, r2 to leftovers */
	if (!DISABLE_FAST_REGISTERS)
	{
		drcbe_info beinfo;
		m_drcuml->get_backend_info(beinfo);
		if (beinfo.direct_iregs > 5)
			m_regmap[0] = uml::I5;
		if (beinfo.direct_iregs > 6)
			m_regmap[1] = uml::I6;
		if (beinfo.direct_iregs > 7)
			m_regmap[2] = uml::I7;
	}

	/* mark the cache dirty so it is updated on next execute */
	m_cache_dirty = TRUE;
}

void ppc_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PPC_CR:
			m_debugger_temp = get_cr();
			break;

		case PPC_XER:
			m_debugger_temp = get_xer();
			break;

		case PPC_TBH:
			m_debugger_temp = get_timebase() >> 32;
			break;

		case PPC_TBL:
			m_debugger_temp = (UINT32)get_timebase();
			break;

		case PPC_DEC:
			m_debugger_temp = get_decrementer();
			break;
	}
}

void ppc_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PPC_CR:
			set_cr(m_debugger_temp);
			break;

		case PPC_XER:
			set_xer(m_debugger_temp);
			break;

		case PPC_TBL:
			set_timebase((get_timebase() & ~U64(0x00ffffff00000000)) | m_debugger_temp);
			break;

		case PPC_TBH:
			set_timebase((get_timebase() & ~U64(0x00000000ffffffff)) | ((UINT64)(m_debugger_temp & 0x00ffffff) << 32));
			break;

		case PPC_DEC:
			set_decrementer(m_debugger_temp);
			break;
	}
}


void ppc_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case PPC_F0:
			strprintf(str, "%12f", m_core->f[0]);
			break;

		case PPC_F1:
			strprintf(str, "%12f", m_core->f[1]);
			break;

		case PPC_F2:
			strprintf(str, "%12f", m_core->f[2]);
			break;

		case PPC_F3:
			strprintf(str, "%12f", m_core->f[3]);
			break;

		case PPC_F4:
			strprintf(str, "%12f", m_core->f[4]);
			break;

		case PPC_F5:
			strprintf(str, "%12f", m_core->f[5]);
			break;

		case PPC_F6:
			strprintf(str, "%12f", m_core->f[6]);
			break;

		case PPC_F7:
			strprintf(str, "%12f", m_core->f[7]);
			break;

		case PPC_F8:
			strprintf(str, "%12f", m_core->f[8]);
			break;

		case PPC_F9:
			strprintf(str, "%12f", m_core->f[9]);
			break;

		case PPC_F10:
			strprintf(str, "%12f", m_core->f[10]);
			break;

		case PPC_F11:
			strprintf(str, "%12f", m_core->f[11]);
			break;

		case PPC_F12:
			strprintf(str, "%12f", m_core->f[12]);
			break;

		case PPC_F13:
			strprintf(str, "%12f", m_core->f[13]);
			break;

		case PPC_F14:
			strprintf(str, "%12f", m_core->f[14]);
			break;

		case PPC_F15:
			strprintf(str, "%12f", m_core->f[15]);
			break;

		case PPC_F16:
			strprintf(str, "%12f", m_core->f[16]);
			break;

		case PPC_F17:
			strprintf(str, "%12f", m_core->f[17]);
			break;

		case PPC_F18:
			strprintf(str, "%12f", m_core->f[18]);
			break;

		case PPC_F19:
			strprintf(str, "%12f", m_core->f[19]);
			break;

		case PPC_F20:
			strprintf(str, "%12f", m_core->f[20]);
			break;

		case PPC_F21:
			strprintf(str, "%12f", m_core->f[21]);
			break;

		case PPC_F22:
			strprintf(str, "%12f", m_core->f[22]);
			break;

		case PPC_F23:
			strprintf(str, "%12f", m_core->f[23]);
			break;

		case PPC_F24:
			strprintf(str, "%12f", m_core->f[24]);
			break;

		case PPC_F25:
			strprintf(str, "%12f", m_core->f[25]);
			break;

		case PPC_F26:
			strprintf(str, "%12f", m_core->f[26]);
			break;

		case PPC_F27:
			strprintf(str, "%12f", m_core->f[27]);
			break;

		case PPC_F28:
			strprintf(str, "%12f", m_core->f[28]);
			break;

		case PPC_F29:
			strprintf(str, "%12f", m_core->f[29]);
			break;

		case PPC_F30:
			strprintf(str, "%12f", m_core->f[30]);
			break;

		case PPC_F31:
			strprintf(str, "%12f", m_core->f[31]);
			break;
	}
}


/*-------------------------------------------------
    ppccom_exit - common cleanup/exit
-------------------------------------------------*/

void ppc_device::device_stop()
{
	if (m_vtlb != nullptr)
		vtlb_free(m_vtlb);
	m_vtlb = nullptr;
}


/*-------------------------------------------------
    ppccom_reset - reset the state of all the
    registers
-------------------------------------------------*/

void ppc_device::device_reset()
{
	/* initialize the OEA state */
	if (m_cap & PPCCAP_OEA)
	{
		/* PC to the reset vector; MSR has IP set to start */
		m_core->pc = 0xfff00100;
		m_core->msr = MSROEA_IP;

		/* reset the decrementer */
		m_dec_zero_cycles = total_cycles();
		if (m_tb_divisor)
		{
			decrementer_int_callback(nullptr, 0);
		}
	}

	/* initialize the 4XX state */
	if (m_cap & PPCCAP_4XX)
	{
		/* PC to the last word; MSR to 0 */
		m_core->pc = 0xfffffffc;
		m_core->msr = 0;

		/* reset the SPU status */
		m_core->spr[SPR4XX_TCR] &= ~PPC4XX_TCR_WRC_MASK;
		m_spu.regs[SPU4XX_LINE_STATUS] = 0x06;
	}

	/* initialize the 602 HID0 register */
	if (m_flavor == PPC_MODEL_602)
		m_core->spr[SPR603_HID0] = 1;

	/* time base starts here */
	m_tb_zero_cycles = total_cycles();

	/* clear interrupts */
	m_core->irq_pending = 0;

	/* flush the TLB */
	vtlb_flush_dynamic(m_vtlb);
	if (m_cap & PPCCAP_603_MMU)
	{
		for (int tlbindex = 0; tlbindex < PPC603_FIXED_TLB_ENTRIES; tlbindex++)
		{
			vtlb_load(m_vtlb, tlbindex, 0, 0, 0);
		}
	}

	/* Mark the cache dirty */
	m_core->mode = 0;
	m_cache_dirty = TRUE;
}


/*-------------------------------------------------
    ppccom_dasm - handle disassembly for a
    CPU
-------------------------------------------------*/

offs_t ppc_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern offs_t ppc_dasm_one(char *buffer, UINT32 pc, UINT32 op);
	UINT32 op = *(UINT32 *)oprom;
	op = BIG_ENDIANIZE_INT32(op);
	return ppc_dasm_one(buffer, pc, op);
}


/*-------------------------------------------------
    ppccom_dcstore_callback - call the dcstore
    callback if installed
-------------------------------------------------*/

void ppc_device::ppccom_dcstore_callback()
{
	if (!m_dcstore_cb.isnull())
	{
		m_dcstore_cb(*m_program, m_core->param0, 0, 0xffffffff);
	}
}


/***************************************************************************
    TLB HANDLING
***************************************************************************/

/*-------------------------------------------------
    ppccom_translate_address_internal - translate
    an address from logical to physical; shared
    between external requests and internal TLB
    filling
-------------------------------------------------*/

UINT32 ppc_device::ppccom_translate_address_internal(int intention, offs_t &address)
{
	int transpriv = ((intention & TRANSLATE_USER_MASK) == 0);   // 1 for supervisor, 0 for user
	int transtype = intention & TRANSLATE_TYPE_MASK;
	offs_t hash, hashbase, hashmask;
	int batbase, batnum, hashnum;
	UINT32 segreg;

	/* 4xx case: "TLB" really just caches writes and checks compare registers */
	if (m_cap & PPCCAP_4XX)
	{
		/* we don't support the MMU of the 403GCX */
		if (m_flavor == PPC_MODEL_403GCX && (m_core->msr & MSROEA_DR))
			fatalerror("MMU enabled but not supported!\n");

		/* only check if PE is enabled */
		if (transtype == TRANSLATE_WRITE && (m_core->msr & MSR4XX_PE))
		{
			/* are we within one of the protection ranges? */
			int inrange1 = ((address >> 12) >= (m_core->spr[SPR4XX_PBL1] >> 12) && (address >> 12) < (m_core->spr[SPR4XX_PBU1] >> 12));
			int inrange2 = ((address >> 12) >= (m_core->spr[SPR4XX_PBL2] >> 12) && (address >> 12) < (m_core->spr[SPR4XX_PBU2] >> 12));

			/* if PX == 1, writes are only allowed OUTSIDE of the bounds */
			if (((m_core->msr & MSR4XX_PX) && (inrange1 || inrange2)) || (!(m_core->msr & MSR4XX_PX) && (!inrange1 && !inrange2)))
				return 0x002;
		}
		address &= 0x7fffffff;
		return 0x001;
	}

	/* only applies if we support the OEA */
	if (!(m_cap & PPCCAP_OEA))
		return 0x001;

	/* also no translation necessary if translation is disabled */
	if ((transtype == TRANSLATE_FETCH && (m_core->msr & MSROEA_IR) == 0) || (transtype != TRANSLATE_FETCH && (m_core->msr & MSROEA_DR) == 0))
		return 0x001;

	/* first scan the appropriate BAT */
	if (m_cap & PPCCAP_601BAT)
	{
		for (batnum = 0; batnum < 4; batnum++)
		{
			UINT32 upper = m_core->spr[SPROEA_IBAT0U + 2*batnum + 0];
			UINT32 lower = m_core->spr[SPROEA_IBAT0U + 2*batnum + 1];
			int privbit = ((intention & TRANSLATE_USER_MASK) == 0) ? 3 : 2;

//            printf("bat %d upper = %08x privbit %d\n", batnum, upper, privbit);

			// is this pair valid?
			if (lower & 0x40)
			{
				UINT32 mask = ((lower & 0x3f) << 17) ^ 0xfffe0000;
				UINT32 addrout;
				UINT32 key = (upper >> privbit) & 1;

				/* check for a hit against this bucket */
				if ((address & mask) == (upper & mask))
				{
					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, key, upper & 3))
					{
						return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
					}

					/* otherwise we're good */
					addrout = (lower & mask) | (address & ~mask);
					address = addrout; // top 9 bits from top 9 of PBN
					return 0x001;
				}
			}
		}
	}
	else
	{
		batbase = (transtype == TRANSLATE_FETCH) ? SPROEA_IBAT0U : SPROEA_DBAT0U;

		for (batnum = 0; batnum < 4; batnum++)
		{
			UINT32 upper = m_core->spr[batbase + 2*batnum + 0];

			/* check user/supervisor valid bit */
			if ((upper >> transpriv) & 0x01)
			{
				UINT32 mask = (~upper << 15) & 0xfffe0000;

				/* check for a hit against this bucket */
				if ((address & mask) == (upper & mask))
				{
					UINT32 lower = m_core->spr[batbase + 2*batnum + 1];

					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, 1, lower & 3))
					{
						return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
					}

					/* otherwise we're good */
					address = (lower & mask) | (address & ~mask);
					return 0x001;
				}
			}
		}
	}

	/* look up the segment register */
	segreg = m_core->sr[address >> 28];
	if (transtype == TRANSLATE_FETCH && (segreg & 0x10000000))
		return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);

	/* check for memory-forced I/O */
	if (m_cap & PPCCAP_MFIOC)
	{
		if ((transtype != TRANSLATE_FETCH) && ((segreg & 0x87f00000) == 0x87f00000))
		{
			address = ((segreg & 0xf)<<28) | (address & 0x0fffffff);
			return 1;
		}
		else if (segreg & 0x80000000)
		{
			fatalerror("PPC: Unhandled segment register %08x with T=1\n", segreg);
		}
	}

	/* get hash table information from SD1 */
	hashbase = m_core->spr[SPROEA_SDR1] & 0xffff0000;
	hashmask = ((m_core->spr[SPROEA_SDR1] & 0x1ff) << 16) | 0xffff;
	hash = (segreg & 0x7ffff) ^ ((address >> 12) & 0xffff);

	/* if we're simulating the 603 MMU, fill in the data and stop here */
	if (m_cap & PPCCAP_603_MMU)
	{
		UINT32 entry = vtlb_table(m_vtlb)[address >> 12];
		m_core->mmu603_cmp = 0x80000000 | ((segreg & 0xffffff) << 7) | (0 << 6) | ((address >> 22) & 0x3f);
		m_core->mmu603_hash[0] = hashbase | ((hash << 6) & hashmask);
		m_core->mmu603_hash[1] = hashbase | ((~hash << 6) & hashmask);
		if ((entry & (VTLB_FLAG_FIXED | VTLB_FLAG_VALID)) == (VTLB_FLAG_FIXED | VTLB_FLAG_VALID))
		{
			address = (entry & 0xfffff000) | (address & 0x00000fff);
			return 0x001;
		}
		return DSISR_NOT_FOUND | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
	}

	/* loop twice over hashes */
	for (hashnum = 0; hashnum < 2; hashnum++)
	{
		offs_t ptegaddr = hashbase | ((hash << 6) & hashmask);
		UINT32 *ptegptr = (UINT32 *)m_program->get_read_ptr(ptegaddr);

		/* should only have valid memory here, but make sure */
		if (ptegptr != nullptr)
		{
			UINT32 targetupper = 0x80000000 | ((segreg & 0xffffff) << 7) | (hashnum << 6) | ((address >> 22) & 0x3f);
			int ptenum;

			/* scan PTEs */
			for (ptenum = 0; ptenum < 8; ptenum++)
				if (ptegptr[BYTE_XOR_BE(ptenum * 2)] == targetupper)
				{
					UINT32 pteglower = ptegptr[BYTE_XOR_BE(ptenum * 2 + 1)];

					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, (segreg >> (29 + transpriv)) & 1, pteglower & 3))
						return DSISR_PROTECTED | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);

					/* update page table bits */
					if (!(intention & TRANSLATE_DEBUG_MASK))
					{
						pteglower |= 0x100;
						if (transtype == TRANSLATE_WRITE)
							pteglower |= 0x080;
						ptegptr[BYTE_XOR_BE(ptenum * 2 + 1)] = pteglower;
					}

					/* otherwise we're good */
					address = (pteglower & 0xfffff000) | (address & 0x00000fff);
					return (pteglower >> 7) & 1;
				}
		}

		/* invert the hash after the first round */
		hash = ~hash;
	}

	/* we failed to find any match: not found */
	return DSISR_NOT_FOUND | ((transtype == TRANSLATE_WRITE) ? DSISR_STORE : 0);
}


/*-------------------------------------------------
    ppccom_translate_address - translate an address
    from logical to physical
-------------------------------------------------*/

bool ppc_device::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	/* only applies to the program address space */
	if (spacenum != AS_PROGRAM)
		return TRUE;

	/* translation is successful if the internal routine returns 0 or 1 */
	return (ppccom_translate_address_internal(intention, address) <= 1);
}


/*-------------------------------------------------
    ppccom_tlb_fill - handle a missing TLB entry
-------------------------------------------------*/

void ppc_device::ppccom_tlb_fill()
{
	vtlb_fill(m_vtlb, m_core->param0, m_core->param1);
}


/*-------------------------------------------------
    ppccom_tlb_flush - flush the entire TLB,
    including fixed entries
-------------------------------------------------*/

void ppc_device::ppccom_tlb_flush()
{
	vtlb_flush_dynamic(m_vtlb);
}



/***************************************************************************
    OPCODE HANDLING
***************************************************************************/

/*-------------------------------------------------
    ppccom_execute_tlbie - execute a TLBIE
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_tlbie()
{
	vtlb_flush_address(m_vtlb, m_core->param0);
}


/*-------------------------------------------------
    ppccom_execute_tlbia - execute a TLBIA
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_tlbia()
{
	vtlb_flush_dynamic(m_vtlb);
}


/*-------------------------------------------------
    ppccom_execute_tlbl - execute a TLBLD/TLBLI
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_tlbl()
{
	UINT32 address = m_core->param0;
	int isitlb = m_core->param1;
	vtlb_entry flags;
	int entrynum;

	/* determine entry number; we use rand() for associativity */
	entrynum = ((address >> 12) & 0x1f) | (machine().rand() & 0x20) | (isitlb ? 0x40 : 0);

	/* determine the flags */
	flags = VTLB_FLAG_VALID | VTLB_READ_ALLOWED | VTLB_FETCH_ALLOWED;
	if (m_core->spr[SPR603_RPA] & 0x80)
		flags |= VTLB_WRITE_ALLOWED;
	if (isitlb)
		flags |= VTLB_FETCH_ALLOWED;

	/* load the entry */
	vtlb_load(m_vtlb, entrynum, 1, address, (m_core->spr[SPR603_RPA] & 0xfffff000) | flags);
}


/*-------------------------------------------------
    ppccom_execute_mftb - execute an MFTB
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_mftb()
{
	switch (m_core->param0)
	{
		/* user mode timebase read */
		case SPRVEA_TBL_R:
			m_core->param1 = get_timebase();
			break;
		case SPRVEA_TBU_R:
			m_core->param1 = get_timebase() >> 32;
			break;
	}
}


/*-------------------------------------------------
    ppccom_execute_mfspr - execute an MFSPR
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_mfspr()
{
	/* handle OEA SPRs */
	if (m_cap & PPCCAP_OEA)
	{
		switch (m_core->param0)
		{
			/* read-through no-ops */
			case SPROEA_DSISR:
			case SPROEA_DAR:
			case SPROEA_SDR1:
			case SPROEA_SRR0:
			case SPROEA_SRR1:
			case SPROEA_EAR:
			case SPROEA_IBAT0L:
			case SPROEA_IBAT0U:
			case SPROEA_IBAT1L:
			case SPROEA_IBAT1U:
			case SPROEA_IBAT2L:
			case SPROEA_IBAT2U:
			case SPROEA_IBAT3L:
			case SPROEA_IBAT3U:
			case SPROEA_DBAT0L:
			case SPROEA_DBAT0U:
			case SPROEA_DBAT1L:
			case SPROEA_DBAT1U:
			case SPROEA_DBAT2L:
			case SPROEA_DBAT2U:
			case SPROEA_DBAT3L:
			case SPROEA_DBAT3U:
			case SPROEA_DABR:
				m_core->param1 = m_core->spr[m_core->param0];
				return;

			/* decrementer */
			case SPROEA_DEC:
				m_core->param1 = get_decrementer();
				return;
		}
	}

	/* handle 603 SPRs */
	if (m_cap & PPCCAP_603_MMU)
	{
		switch (m_core->param0)
		{
			/* read-through no-ops */
			case SPR603_DMISS:
			case SPR603_DCMP:
			case SPR603_HASH1:
			case SPR603_HASH2:
			case SPR603_IMISS:
			case SPR603_ICMP:
			case SPR603_RPA:
			case SPR603_HID0:
			case SPR603_HID1:
			case SPR603_IABR:
			case SPR603_HID2:
				m_core->param1 = m_core->spr[m_core->param0];
				return;

			/* timebase */
			case SPR603_TBL_R:
				m_core->param1 = get_timebase();
				return;
			case SPR603_TBU_R:
				m_core->param1 = (get_timebase() >> 32) & 0xffffff;
				return;
		}
	}

	/* handle 4XX SPRs */
	if (m_cap & PPCCAP_4XX)
	{
		switch (m_core->param0)
		{
			/* read-through no-ops */
			case SPR4XX_EVPR:
			case SPR4XX_ESR:
			case SPR4XX_SRR0:
			case SPR4XX_SRR1:
			case SPR4XX_SRR2:
			case SPR4XX_SRR3:
			case SPR4XX_TCR:
			case SPR4XX_TSR:
			case SPR4XX_IAC1:
			case SPR4XX_IAC2:
			case SPR4XX_DAC1:
			case SPR4XX_DAC2:
			case SPR4XX_DCCR:
			case SPR4XX_ICCR:
			case SPR4XX_PBL1:
			case SPR4XX_PBU1:
			case SPR4XX_PBL2:
			case SPR4XX_PBU2:
				m_core->param1 = m_core->spr[m_core->param0];
				return;

			/* timebase */
			case SPR4XX_TBLO:
			case SPR4XX_TBLU:
				m_core->param1 = get_timebase();
				return;
			case SPR4XX_TBHI:
			case SPR4XX_TBHU:
				m_core->param1 = (get_timebase() >> 32) & 0xffffff;
				return;
		}
	}

	/* default handling */
	osd_printf_debug("SPR %03X read\n", m_core->param0);
	m_core->param1 = m_core->spr[m_core->param0];
}


/*-------------------------------------------------
    ppccom_execute_mtspr - execute an MTSPR
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_mtspr()
{
	/* handle OEA SPRs */
	if (m_cap & PPCCAP_OEA)
	{
		switch (m_core->param0)
		{
			/* write-through no-ops */
			case SPROEA_DSISR:
			case SPROEA_DAR:
			case SPROEA_SRR0:
			case SPROEA_SRR1:
			case SPROEA_EAR:
			case SPROEA_DABR:
				m_core->spr[m_core->param0] = m_core->param1;
				return;

			/* registers that affect the memory map */
			case SPROEA_SDR1:
			case SPROEA_IBAT0L:
			case SPROEA_IBAT0U:
			case SPROEA_IBAT1L:
			case SPROEA_IBAT1U:
			case SPROEA_IBAT2L:
			case SPROEA_IBAT2U:
			case SPROEA_IBAT3L:
			case SPROEA_IBAT3U:
			case SPROEA_DBAT0L:
			case SPROEA_DBAT0U:
			case SPROEA_DBAT1L:
			case SPROEA_DBAT1U:
			case SPROEA_DBAT2L:
			case SPROEA_DBAT2U:
			case SPROEA_DBAT3L:
			case SPROEA_DBAT3U:
				m_core->spr[m_core->param0] = m_core->param1;
				ppccom_tlb_flush();
				return;

			/* decrementer */
			case SPROEA_DEC:
				set_decrementer(m_core->param1);
				return;
		}
	}

	/* handle 603 SPRs */
	if (m_cap & PPCCAP_603_MMU)
	{
		switch (m_core->param0)
		{
			/* read-only */
			case SPR603_DMISS:
			case SPR603_DCMP:
			case SPR603_HASH1:
			case SPR603_HASH2:
			case SPR603_IMISS:
			case SPR603_ICMP:
				return;

			/* write-through no-ops */
			case SPR603_RPA:
			case SPR603_HID0:
			case SPR603_HID1:
			case SPR603_IABR:
			case SPR603_HID2:
				m_core->spr[m_core->param0] = m_core->param1;
				return;

			/* timebase */
			case SPR603_TBL_W:
				set_timebase((get_timebase() & ~U64(0xffffffff00000000)) | m_core->param1);
				return;
			case SPR603_TBU_W:
				set_timebase((get_timebase() & ~U64(0x00000000ffffffff)) | ((UINT64)m_core->param1 << 32));
				return;
		}
	}

	/* handle 4XX SPRs */
	if (m_cap & PPCCAP_4XX)
	{
		UINT32 oldval = m_core->spr[m_core->param0];
		switch (m_core->param0)
		{
			/* write-through no-ops */
			case SPR4XX_EVPR:
			case SPR4XX_ESR:
			case SPR4XX_DCCR:
			case SPR4XX_ICCR:
			case SPR4XX_SRR0:
			case SPR4XX_SRR1:
			case SPR4XX_SRR2:
			case SPR4XX_SRR3:
				m_core->spr[m_core->param0] = m_core->param1;
				return;

			/* registers that affect the memory map */
			case SPR4XX_PBL1:
			case SPR4XX_PBU1:
			case SPR4XX_PBL2:
			case SPR4XX_PBU2:
				m_core->spr[m_core->param0] = m_core->param1;
				ppccom_tlb_flush();
				return;

			/* timer control register */
			case SPR4XX_TCR:
				m_core->spr[SPR4XX_TCR] = m_core->param1 | (oldval & PPC4XX_TCR_WRC_MASK);
				if ((oldval ^ m_core->spr[SPR4XX_TCR]) & PPC4XX_TCR_FIE)
					ppc4xx_fit_callback(nullptr, FALSE);
				if ((oldval ^ m_core->spr[SPR4XX_TCR]) & PPC4XX_TCR_PIE)
					ppc4xx_pit_callback(nullptr, FALSE);
				return;

			/* timer status register */
			case SPR4XX_TSR:
				m_core->spr[SPR4XX_TSR] &= ~m_core->param1;
				ppc4xx_set_irq_line(0, 0);
				return;

			/* PIT */
			case SPR4XX_PIT:
				m_core->spr[SPR4XX_PIT] = m_core->param1;
				m_pit_reload = m_core->param1;
				ppc4xx_pit_callback(nullptr, FALSE);
				return;

			/* timebase */
			case SPR4XX_TBLO:
				set_timebase((get_timebase() & ~U64(0x00ffffff00000000)) | m_core->param1);
				return;
			case SPR4XX_TBHI:
				set_timebase((get_timebase() & ~U64(0x00000000ffffffff)) | ((UINT64)(m_core->param1 & 0x00ffffff) << 32));
				return;
		}
	}

	/* default handling */
	osd_printf_debug("SPR %03X write = %08X\n", m_core->param0, m_core->param1);
	m_core->spr[m_core->param0] = m_core->param1;
}


/*-------------------------------------------------
    ppccom_execute_mfdcr - execute an MFDCR
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_mfdcr()
{
	/* handle various DCRs */
	switch (m_core->param0)
	{
		/* read-through no-ops */
		case DCR4XX_BR0:
		case DCR4XX_BR1:
		case DCR4XX_BR2:
		case DCR4XX_BR3:
		case DCR4XX_BR4:
		case DCR4XX_BR5:
		case DCR4XX_BR6:
		case DCR4XX_BR7:
		case DCR4XX_BESR:
		case DCR4XX_DMASR:
		case DCR4XX_DMACT0:
		case DCR4XX_DMADA0:
		case DCR4XX_DMASA0:
		case DCR4XX_DMACC0:
		case DCR4XX_DMACR0:
		case DCR4XX_DMACT1:
		case DCR4XX_DMADA1:
		case DCR4XX_DMASA1:
		case DCR4XX_DMACC1:
		case DCR4XX_DMACR1:
		case DCR4XX_DMACT2:
		case DCR4XX_DMADA2:
		case DCR4XX_DMASA2:
		case DCR4XX_DMACC2:
		case DCR4XX_DMACR2:
		case DCR4XX_DMACT3:
		case DCR4XX_DMADA3:
		case DCR4XX_DMASA3:
		case DCR4XX_DMACC3:
		case DCR4XX_DMACR3:
		case DCR4XX_EXIER:
		case DCR4XX_EXISR:
		case DCR4XX_IOCR:
			m_core->param1 = m_dcr[m_core->param0];
			return;
	}

	/* default handling */
	if (m_dcr_read_func.isnull()) {
		osd_printf_debug("DCR %03X read\n", m_core->param0);
		if (m_core->param0 < ARRAY_LENGTH(m_dcr))
			m_core->param1 = m_dcr[m_core->param0];
		else
			m_core->param1 = 0;
	} else {
		m_core->param1 = m_dcr_read_func(*m_program,m_core->param0,0xffffffff);
	}
}


/*-------------------------------------------------
    ppccom_execute_mtdcr - execute an MTDCR
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_mtdcr()
{
	UINT8 oldval;

	/* handle various DCRs */
	switch (m_core->param0)
	{
		/* write-through no-ops */
		case DCR4XX_BR0:
		case DCR4XX_BR1:
		case DCR4XX_BR2:
		case DCR4XX_BR3:
		case DCR4XX_BR4:
		case DCR4XX_BR5:
		case DCR4XX_BR6:
		case DCR4XX_BR7:
		case DCR4XX_BESR:
		case DCR4XX_DMACT0:
		case DCR4XX_DMADA0:
		case DCR4XX_DMASA0:
		case DCR4XX_DMACC0:
		case DCR4XX_DMACT1:
		case DCR4XX_DMADA1:
		case DCR4XX_DMASA1:
		case DCR4XX_DMACC1:
		case DCR4XX_DMACT2:
		case DCR4XX_DMADA2:
		case DCR4XX_DMASA2:
		case DCR4XX_DMACC2:
		case DCR4XX_DMACT3:
		case DCR4XX_DMADA3:
		case DCR4XX_DMASA3:
		case DCR4XX_DMACC3:
			m_dcr[m_core->param0] = m_core->param1;
			return;

		/* DMA status */
		case DCR4XX_DMASR:
			m_dcr[DCR4XX_DMASR] &= ~(m_core->param1 & 0xfff80070);
			ppc4xx_dma_update_irq_states();
			return;

		/* interrupt enables */
		case DCR4XX_EXIER:
			m_dcr[DCR4XX_EXIER] = m_core->param1;
			ppc4xx_set_irq_line(0, 0);
			return;

		/* interrupt clear */
		case DCR4XX_EXISR:
			m_dcr[m_core->param0] &= ~m_core->param1;
			ppc4xx_set_irq_line(0, 0);
			return;

		/* DMA controls */
		case DCR4XX_DMACR0:
		case DCR4XX_DMACR1:
		case DCR4XX_DMACR2:
		case DCR4XX_DMACR3:
			m_dcr[m_core->param0] = m_core->param1;
			if (m_core->param1 & PPC4XX_DMACR_CE)
				ppc4xx_dma_exec((m_core->param0 - DCR4XX_DMACR0) / 8);
			ppc4xx_dma_update_irq_states();
			return;

		/* I/O control */
		case DCR4XX_IOCR:
			oldval = m_dcr[m_core->param0];
			m_dcr[m_core->param0] = m_core->param1;
			if ((oldval ^ m_core->param1) & 0x02)
				ppc4xx_spu_timer_reset();
			return;
	}

	/* default handling */
	if (m_dcr_write_func.isnull()) {
		osd_printf_debug("DCR %03X write = %08X\n", m_core->param0, m_core->param1);
		if (m_core->param0 < ARRAY_LENGTH(m_dcr))
			m_dcr[m_core->param0] = m_core->param1;
	} else {
		m_dcr_write_func(*m_program,m_core->param0,m_core->param1,0xffffffff);
	}
}



/***************************************************************************
    FLOATING POINT STATUS FLAGS HANDLING
***************************************************************************/

/*-------------------------------------------------
    ppccom_update_fprf - update the FPRF field
    of the FPSCR register
-------------------------------------------------*/

void ppc_device::ppccom_update_fprf()
{
	UINT32 fprf;
	double f = m_core->f[m_core->param0];

	if (is_qnan_double(f))
	{
		fprf = 0x11;
	}
	else if (is_infinity_double(f))
	{
		if (sign_double(f))     /* -Infinity */
			fprf = 0x09;
		else                    /* +Infinity */
			fprf = 0x05;
	}
	else if (is_normalized_double(f))
	{
		if (sign_double(f))     /* -Normalized */
			fprf = 0x08;
		else                    /* +Normalized */
			fprf = 0x04;
	}
	else if (is_denormalized_double(f))
	{
		if (sign_double(f))     /* -Denormalized */
			fprf = 0x18;
		else                    /* +Denormalized */
			fprf = 0x14;
	}
	else
	{
		if (sign_double(f))     /* -Zero */
			fprf = 0x12;
		else                    /* +Zero */
			fprf = 0x02;
	}

	m_core->fpscr &= ~0x0001f000;
	m_core->fpscr |= fprf << 12;
}


/***************************************************************************
    OEA HELPERS
***************************************************************************/

/*-------------------------------------------------
    decrementer_int_callback - callback that fires
    whenever a decrementer interrupt is generated
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ppc_device::decrementer_int_callback )
{
	UINT64 cycles_until_next;

	/* set the decrementer IRQ state */
	m_core->irq_pending |= 0x02;

	/* advance by another full rev */
	m_dec_zero_cycles += (UINT64)m_tb_divisor << 32;
	cycles_until_next = m_dec_zero_cycles - total_cycles();
	m_decrementer_int_timer->adjust(cycles_to_attotime(cycles_until_next));
}

/*-------------------------------------------------
    ppc_set_dcstore_callback - installs a callback
    for detecting datacache stores with dcbst
-------------------------------------------------*/

void ppc_device::ppc_set_dcstore_callback(write32_delegate callback)
{
	m_dcstore_cb = callback;
}


void ppc_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case PPC_IRQ:
			m_core->irq_pending = (m_core->irq_pending & ~1) | ((state != CLEAR_LINE) ? 1 : 0);
			break;
	}
}


void ppc4xx_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case PPC_IRQ_LINE_0:
			ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_EXT0, state);
			break;

		case PPC_IRQ_LINE_1:
			ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_EXT1, state);
			break;

		case PPC_IRQ_LINE_2:
			ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_EXT2, state);
			break;

		case PPC_IRQ_LINE_3:
			ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_EXT3, state);
			break;

		case PPC_IRQ_LINE_4:
			ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_EXT4, state);
			break;
	}
}


/***************************************************************************
    EMBEDDED 4XX HELPERS
***************************************************************************/

/*-------------------------------------------------
    ppc4xx_set_irq_line - PowerPC 4XX-specific
    IRQ line management
-------------------------------------------------*/

void ppc_device::ppc4xx_set_irq_line(UINT32 bitmask, int state)
{
	UINT32 oldstate = m_irqstate;
	UINT32 levelmask;

	/* set or clear the appropriate bit */
	if (state != CLEAR_LINE)
		m_irqstate |= bitmask;
	else
		m_irqstate &= ~bitmask;

	/* if the state changed to on, edge trigger the interrupt */
	if (((m_irqstate ^ oldstate) & bitmask) && (m_irqstate & bitmask))
		m_dcr[DCR4XX_EXISR] |= bitmask;

	/* pass through all level-triggered interrupts */
	levelmask = PPC4XX_IRQ_BIT_CRITICAL | PPC4XX_IRQ_BIT_SPUR | PPC4XX_IRQ_BIT_SPUT;
	levelmask |= PPC4XX_IRQ_BIT_JTAGR | PPC4XX_IRQ_BIT_JTAGT;
	levelmask |= PPC4XX_IRQ_BIT_DMA0 | PPC4XX_IRQ_BIT_DMA1 | PPC4XX_IRQ_BIT_DMA2 | PPC4XX_IRQ_BIT_DMA3;
	if (!(m_dcr[DCR4XX_IOCR] & 0x80000000)) levelmask |= PPC4XX_IRQ_BIT_EXT0;
	if (!(m_dcr[DCR4XX_IOCR] & 0x20000000)) levelmask |= PPC4XX_IRQ_BIT_EXT1;
	if (!(m_dcr[DCR4XX_IOCR] & 0x08000000)) levelmask |= PPC4XX_IRQ_BIT_EXT2;
	if (!(m_dcr[DCR4XX_IOCR] & 0x02000000)) levelmask |= PPC4XX_IRQ_BIT_EXT3;
	if (!(m_dcr[DCR4XX_IOCR] & 0x00800000)) levelmask |= PPC4XX_IRQ_BIT_EXT4;
	m_dcr[DCR4XX_EXISR] = (m_dcr[DCR4XX_EXISR] & ~levelmask) | (m_irqstate & levelmask);

	/* update the IRQ status */
	m_core->irq_pending = ((m_dcr[DCR4XX_EXISR] & m_dcr[DCR4XX_EXIER]) != 0);
	if ((m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_FIE) && (m_core->spr[SPR4XX_TSR] & PPC4XX_TSR_FIS))
		m_core->irq_pending = TRUE;
	if ((m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_PIE) && (m_core->spr[SPR4XX_TSR] & PPC4XX_TSR_PIS))
		m_core->irq_pending = TRUE;
}


/*-------------------------------------------------
    ppc4xx_get_irq_line - PowerPC 4XX-specific
    IRQ line state getter
-------------------------------------------------*/

int ppc_device::ppc4xx_get_irq_line(UINT32 bitmask)
{
	return (m_irqstate & bitmask) ? ASSERT_LINE : CLEAR_LINE;
}


/*-------------------------------------------------
    ppc4xx_dma_update_irq_states - update the IRQ
    state for each DMA channel
-------------------------------------------------*/

void ppc_device::ppc4xx_dma_update_irq_states()
{
	/* update the IRQ state for each DMA channel */
	for (int dmachan = 0; dmachan < 4; dmachan++)
	{
		bool irq_pending = false;

		// Channel interrupt enabled?
		if ((m_dcr[DCR4XX_DMACR0 + 8 * dmachan] & PPC4XX_DMACR_CIE))
		{
			// Terminal count and end-of-transfer status bits
			int bitmask = 0x11 << (27 - dmachan);

			// Chained transfer status bit
			switch (dmachan)
			{
				case 0:
					bitmask |= 0x00080000;
					break;

				case 1:
				case 2:
				case 3:
					bitmask |= 1 << (7 - dmachan);
					break;
			}

			irq_pending = (m_dcr[DCR4XX_DMASR] & bitmask) != 0;
		}

		ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_DMA(dmachan), irq_pending ? ASSERT_LINE : CLEAR_LINE);
	}
}


/*-------------------------------------------------
    ppc4xx_dma_decrement_count - decrement the
    count on a channel and interrupt if configured
    to do so
-------------------------------------------------*/

int ppc_device::ppc4xx_dma_decrement_count(int dmachan)
{
	UINT32 *dmaregs = &m_dcr[8 * dmachan];

	/* decrement the counter */
	dmaregs[DCR4XX_DMACT0]--;

	/* if non-zero, we keep going */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) != 0)
		return FALSE;

	// if chained mode
	if (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CH)
	{
		dmaregs[DCR4XX_DMADA0] = dmaregs[DCR4XX_DMASA0];
		dmaregs[DCR4XX_DMACT0] = dmaregs[DCR4XX_DMACC0];
		dmaregs[DCR4XX_DMACR0] &= ~PPC4XX_DMACR_CH;

		switch (dmachan)
		{
			case 0:
				m_dcr[DCR4XX_DMASR] |= 0x00080000;
				break;

			case 1:
			case 2:
			case 3:
				m_dcr[DCR4XX_DMASR] |= 1 << (7 - dmachan);
				break;
		}

		ppc4xx_dma_update_irq_states();

		INT64 numdata = dmaregs[DCR4XX_DMACT0];
		if (numdata == 0)
			numdata = 65536;

		INT64 time = (numdata * 1000000) / m_buffered_dma_rate[dmachan];

		m_buffered_dma_timer[dmachan]->adjust(attotime::from_usec(time), dmachan);
	}
	else
	{
		/* set the complete bit and handle interrupts */
		m_dcr[DCR4XX_DMASR] |= 1 << (31 - dmachan);
	//  m_dcr[DCR4XX_DMASR] |= 1 << (27 - dmachan);
		ppc4xx_dma_update_irq_states();

		m_buffered_dma_timer[dmachan]->adjust(attotime::never, FALSE);
	}
	return TRUE;
}


/*-------------------------------------------------
    buffered_dma_callback - callback that fires
    when buffered DMA transfer is ready
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ppc_device::ppc4xx_buffered_dma_callback )
{
	int dmachan = param;

	static const UINT8 dma_transfer_width[4] = { 1, 2, 4, 16 };
	UINT32 *dmaregs = &m_dcr[8 * dmachan];
	INT32 destinc;
	UINT8 width;

	width = dma_transfer_width[(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_PW_MASK) >> 26];
	destinc = (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_DAI) ? width : 0;

	if (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_TD)
	{
		/* peripheral to memory */

		switch (width)
		{
			/* byte transfer */
			case 1:
			do
			{
				UINT8 data = 0;
				if (!m_ext_dma_read_cb[dmachan].isnull())
					data = (m_ext_dma_read_cb[dmachan])(*m_program, 1, 0xffffffff);
				m_program->write_byte(dmaregs[DCR4XX_DMADA0], data);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;

			/* word transfer */
			case 2:
			do
			{
				UINT16 data = 0;
				if (!m_ext_dma_read_cb[dmachan].isnull())
					data = (m_ext_dma_read_cb[dmachan])(*m_program, 2, 0xffffffff);
				m_program->write_word(dmaregs[DCR4XX_DMADA0], data);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;

			/* dword transfer */
			case 4:
			do
			{
				UINT32 data = 0;
				if (!m_ext_dma_read_cb[dmachan].isnull())
					data = (m_ext_dma_read_cb[dmachan])(*m_program, 4, 0xffffffff);
				m_program->write_dword(dmaregs[DCR4XX_DMADA0], data);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;
		}
	}
	else
	{
		/* memory to peripheral */

		// data is read from destination address!
		switch (width)
		{
			/* byte transfer */
			case 1:
			do
			{
				UINT8 data = m_program->read_byte(dmaregs[DCR4XX_DMADA0]);
				if (!m_ext_dma_write_cb[dmachan].isnull())
					(m_ext_dma_write_cb[dmachan])(*m_program, 1, data, 0xffffffff);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;

			/* word transfer */
			case 2:
			do
			{
				UINT16 data = m_program->read_word(dmaregs[DCR4XX_DMADA0]);
				if (!m_ext_dma_write_cb[dmachan].isnull())
					(m_ext_dma_write_cb[dmachan])(*m_program, 2, data, 0xffffffff);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;

			/* dword transfer */
			case 4:
			do
			{
				UINT32 data = m_program->read_dword(dmaregs[DCR4XX_DMADA0]);
				if (!m_ext_dma_write_cb[dmachan].isnull())
					(m_ext_dma_write_cb[dmachan])(*m_program, 4, data, 0xffffffff);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;
		}
	}
}


/*-------------------------------------------------
    ppc4xx_dma_fetch_transmit_byte - fetch a byte
    to send to a peripheral
-------------------------------------------------*/

int ppc_device::ppc4xx_dma_fetch_transmit_byte(int dmachan, UINT8 *byte)
{
	UINT32 *dmaregs = &m_dcr[8 * dmachan];

	/* if the channel is not enabled, fail */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return FALSE;

	/* if no transfers remaining, fail */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) == 0)
		return FALSE;

	/* fetch the data */
	*byte = m_program->read_byte(dmaregs[DCR4XX_DMADA0]++);
	ppc4xx_dma_decrement_count(dmachan);
	return TRUE;
}


/*-------------------------------------------------
    ppc4xx_dma_handle_receive_byte - receive a byte
    transmitted by a peripheral
-------------------------------------------------*/

int ppc_device::ppc4xx_dma_handle_receive_byte(int dmachan, UINT8 byte)
{
	UINT32 *dmaregs = &m_dcr[8 * dmachan];

	/* if the channel is not enabled, fail */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return FALSE;

	/* if no transfers remaining, fail */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) == 0)
		return FALSE;

	/* store the data */
	m_program->write_byte(dmaregs[DCR4XX_DMADA0]++, byte);
	ppc4xx_dma_decrement_count(dmachan);
	return TRUE;
}


/*-------------------------------------------------
    ppc4xx_dma_execute - execute a DMA operation
    if one is pending
-------------------------------------------------*/

void ppc_device::ppc4xx_dma_exec(int dmachan)
{
	static const UINT8 dma_transfer_width[4] = { 1, 2, 4, 16 };
	UINT32 *dmaregs = &m_dcr[8 * dmachan];
	INT32 destinc, srcinc;
	UINT8 width;

	/* skip if not enabled */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return;

	/* check for unsupported features */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_TCE))
		fatalerror("ppc4xx_dma_exec: DMA_TCE == 0\n");

	/* transfer mode */
	switch ((dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_TM_MASK) >> 21)
	{
		/* buffered mode DMA */
		case 0:
			if (((dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_PL) >> 28) == 0)
			{
				/* buffered DMA with external peripheral */

				INT64 numdata = dmaregs[DCR4XX_DMACT0];
				if (numdata == 0)
					numdata = 65536;

				INT64 time;
				if (numdata > 100)
				{
					time = (numdata * 1000000) / m_buffered_dma_rate[dmachan];
				}
				else
				{
					time = 0;       // let very short transfers occur instantly
				}

				m_buffered_dma_timer[dmachan]->adjust(attotime::from_usec(time), dmachan);
			}
			else        /* buffered DMA with internal peripheral (SPU) */
			{
				/* nothing to do; this happens asynchronously and is driven by the SPU */
			}
			break;

		/* fly-by mode DMA */
		case 1:
			fatalerror("ppc4xx_dma_exec: fly-by DMA not implemented\n");

		/* software initiated memory-to-memory mode DMA */
		case 2:
			width = dma_transfer_width[(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_PW_MASK) >> 26];
			srcinc = (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_SAI) ? width : 0;
			destinc = (dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_DAI) ? width : 0;

			switch (width)
			{
				/* byte transfer */
				case 1:
					do
					{
						m_program->write_byte(dmaregs[DCR4XX_DMADA0], m_program->read_byte(dmaregs[DCR4XX_DMASA0]));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(dmachan));
					break;

				/* word transfer */
				case 2:
					do
					{
						m_program->write_word(dmaregs[DCR4XX_DMADA0], m_program->read_word(dmaregs[DCR4XX_DMASA0]));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(dmachan));
					break;

				/* dword transfer */
				case 4:
					do
					{
						m_program->write_dword(dmaregs[DCR4XX_DMADA0], m_program->read_dword(dmaregs[DCR4XX_DMASA0]));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(dmachan));
					break;

				/* 16-byte transfer */
				case 16:
					do
					{
						m_program->write_qword(dmaregs[DCR4XX_DMADA0], m_program->read_qword(dmaregs[DCR4XX_DMASA0]));
						m_program->write_qword(dmaregs[DCR4XX_DMADA0] + 8, m_program->read_qword(dmaregs[DCR4XX_DMASA0] + 8));
						dmaregs[DCR4XX_DMASA0] += srcinc;
						dmaregs[DCR4XX_DMADA0] += destinc;
					} while (!ppc4xx_dma_decrement_count(dmachan));
					break;
			}
			break;

		/* hardware initiated memory-to-memory mode DMA */
		case 3:
			fatalerror("ppc4xx_dma_exec: HW mem-to-mem DMA not implemented\n");
	}
}


/*-------------------------------------------------
    ppc4xx_fit_callback - FIT timer callback
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ppc_device::ppc4xx_fit_callback )
{
	/* if this is a real callback and we are enabled, signal an interrupt */
	if (param)
	{
		m_core->spr[SPR4XX_TSR] |= PPC4XX_TSR_FIS;
		ppc4xx_set_irq_line(0, 0);
	}

	/* update ourself for the next interval if we are enabled */
	if (m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_FIE)
	{
		UINT32 timebase = get_timebase();
		UINT32 interval = 0x200 << (4 * ((m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_FP_MASK) >> 24));
		UINT32 target = (timebase + interval) & ~(interval - 1);
		m_fit_timer->adjust(cycles_to_attotime((target + 1 - timebase) / m_tb_divisor), TRUE);
	}

	/* otherwise, turn ourself off */
	else
		m_fit_timer->adjust(attotime::never, FALSE);
}


/*-------------------------------------------------
    ppc4xx_pit_callback - PIT timer callback
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ppc_device::ppc4xx_pit_callback )
{
	/* if this is a real callback and we are enabled, signal an interrupt */
	if (param)
	{
		m_core->spr[SPR4XX_TSR] |= PPC4XX_TSR_PIS;
		ppc4xx_set_irq_line(0, 0);
	}

	/* update ourself for the next interval if we are enabled and we are either being
	   forced to update, or we are in auto-reload mode */
	if ((m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_PIE) && m_pit_reload != 0 && (!param || (m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_ARE)))
	{
		UINT32 timebase = get_timebase();
		UINT32 interval = m_pit_reload;
		UINT32 target = timebase + interval;
		m_pit_timer->adjust(cycles_to_attotime((target + 1 - timebase) / m_tb_divisor), TRUE);
	}

	/* otherwise, turn ourself off */
	else
		m_pit_timer->adjust(attotime::never, FALSE);
}


/*-------------------------------------------------
    ppc4xx_spu_update_irq_states - update the IRQ
    state for the SPU
-------------------------------------------------*/

void ppc_device::ppc4xx_spu_update_irq_states()
{
	/* check for receive buffer full interrupt */
	if ((m_spu.regs[SPU4XX_RX_COMMAND] & 0x60) == 0x20 && (m_spu.regs[SPU4XX_LINE_STATUS] & 0x80))
		ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_SPUR, ASSERT_LINE);

	/* check for receive error interrupt */
	else if ((m_spu.regs[SPU4XX_RX_COMMAND] & 0x10) && (m_spu.regs[SPU4XX_LINE_STATUS] & 0x78))
		ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_SPUR, ASSERT_LINE);

	/* clear otherwise */
	else
		ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_SPUR, CLEAR_LINE);

	/* check for transmit buffer empty interrupt */
	if ((m_spu.regs[SPU4XX_TX_COMMAND] & 0x60) == 0x20 && (m_spu.regs[SPU4XX_LINE_STATUS] & 0x04))
		ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_SPUT, ASSERT_LINE);

	/* check for shift register empty interrupt */
	else if ((m_spu.regs[SPU4XX_TX_COMMAND] & 0x10) && (m_spu.regs[SPU4XX_LINE_STATUS] & 0x02))
		ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_SPUT, ASSERT_LINE);

	/* clear otherwise */
	else
		ppc4xx_set_irq_line(PPC4XX_IRQ_BIT_SPUT, CLEAR_LINE);
}


/*-------------------------------------------------
    ppc4xx_spu_rx_data - serial port data receive
-------------------------------------------------*/

void ppc_device::ppc4xx_spu_rx_data(UINT8 data)
{
	UINT32 new_rxin;

	/* fail if we are going to overflow */
	new_rxin = (m_spu.rxin + 1) % ARRAY_LENGTH(m_spu.rxbuffer);
	if (new_rxin == m_spu.rxout)
		fatalerror("ppc4xx_spu_rx_data: buffer overrun!\n");

	/* store the data and accept the new in index */
	m_spu.rxbuffer[m_spu.rxin] = data;
	m_spu.rxin = new_rxin;
}


/*-------------------------------------------------
    ppc4xx_spu_timer_reset - reset and recompute
    the transmit/receive timer
-------------------------------------------------*/

void ppc_device::ppc4xx_spu_timer_reset()
{
	UINT8 enabled = (m_spu.regs[SPU4XX_RX_COMMAND] | m_spu.regs[SPU4XX_TX_COMMAND]) & 0x80;

	/* if we're enabled, reset at the current baud rate */
	if (enabled)
	{
		attotime clockperiod = attotime::from_hz((m_dcr[DCR4XX_IOCR] & 0x02) ? 3686400 : 33333333);
		int divisor = ((m_spu.regs[SPU4XX_BAUD_DIVISOR_H] * 256 + m_spu.regs[SPU4XX_BAUD_DIVISOR_L]) & 0xfff) + 1;
		int bpc = 7 + ((m_spu.regs[SPU4XX_CONTROL] & 8) >> 3) + 1 + (m_spu.regs[SPU4XX_CONTROL] & 1);
		attotime charperiod = clockperiod * (divisor * 16 * bpc);
		m_spu.timer->adjust(charperiod, 0, charperiod);
		if (PRINTF_SPU)
			printf("ppc4xx_spu_timer_reset: baud rate = %.0f\n", ATTOSECONDS_TO_HZ(charperiod.attoseconds()) * bpc);
	}

	/* otherwise, disable the timer */
	else
		m_spu.timer->adjust(attotime::never);
}


/*-------------------------------------------------
    ppc4xx_spu_callback - serial port send/receive
    timer
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ppc_device::ppc4xx_spu_callback )
{
	/* transmit enabled? */
	if (m_spu.regs[SPU4XX_TX_COMMAND] & 0x80)
	{
		int operation = (m_spu.regs[SPU4XX_TX_COMMAND] >> 5) & 3;

		/* if we have data to transmit, do it now */
		if (!(m_spu.regs[SPU4XX_LINE_STATUS] & 0x04))
		{
			/* if we have a transmit handler, send it that way */
			if (!m_spu.tx_cb.isnull())
				(m_spu.tx_cb)(*m_program, 0, m_spu.txbuf, 0xff);

			/* indicate that we have moved it to the shift register */
			m_spu.regs[SPU4XX_LINE_STATUS] |= 0x04;
			m_spu.regs[SPU4XX_LINE_STATUS] &= ~0x02;
		}

		/* otherwise, clear the shift register */
		else if (!(m_spu.regs[SPU4XX_LINE_STATUS] & 0x02))
			m_spu.regs[SPU4XX_LINE_STATUS] |= 0x02;

		/* handle DMA */
		if (operation >= 2 && ppc4xx_dma_fetch_transmit_byte(operation, &m_spu.txbuf))
			m_spu.regs[SPU4XX_LINE_STATUS] &= ~0x04;
	}

	/* receive enabled? */
	if (m_spu.regs[SPU4XX_RX_COMMAND] & 0x80)
		if (m_spu.rxout != m_spu.rxin)
		{
			int operation = (m_spu.regs[SPU4XX_RX_COMMAND] >> 5) & 3;
			UINT8 rxbyte;

			/* consume the byte and advance the out pointer */
			rxbyte = m_spu.rxbuffer[m_spu.rxout];
			m_spu.rxout = (m_spu.rxout + 1) % ARRAY_LENGTH(m_spu.rxbuffer);

			/* if we're not full, copy data to the buffer and update the line status */
			if (!(m_spu.regs[SPU4XX_LINE_STATUS] & 0x80))
			{
				m_spu.rxbuf = rxbyte;
				m_spu.regs[SPU4XX_LINE_STATUS] |= 0x80;
			}

			/* otherwise signal an overrun */
			else
			{
				m_spu.regs[SPU4XX_LINE_STATUS] |= 0x20;
				goto updateirq;
			}

			/* handle DMA */
			if (operation >= 2 && ppc4xx_dma_handle_receive_byte(operation, m_spu.rxbuf))
				m_spu.regs[SPU4XX_LINE_STATUS] &= ~0x80;
		}

	/* update the final IRQ states */
updateirq:
	ppc4xx_spu_update_irq_states();
}


/*-------------------------------------------------
    ppc4xx_spu_r - serial port read handler
-------------------------------------------------*/

READ8_MEMBER( ppc4xx_device::ppc4xx_spu_r )
{
	UINT8 result = 0xff;

	switch (offset)
	{
		case SPU4XX_BUFFER:
			result = m_spu.rxbuf;
			m_spu.regs[SPU4XX_LINE_STATUS] &= ~0x80;
			break;

		default:
			if (offset < ARRAY_LENGTH(m_spu.regs))
				result = m_spu.regs[offset];
			break;
	}
	if (PRINTF_SPU)
		printf("spu_r(%d) = %02X\n", offset, result);
	return result;
}


/*-------------------------------------------------
    ppc4xx_spu_w - serial port write handler
-------------------------------------------------*/

WRITE8_MEMBER( ppc4xx_device::ppc4xx_spu_w )
{
	UINT8 oldstate, newstate;

	if (PRINTF_SPU)
		printf("spu_w(%d) = %02X\n", offset, data);
	switch (offset)
	{
		/* clear error bits */
		case SPU4XX_LINE_STATUS:
			m_spu.regs[SPU4XX_LINE_STATUS] &= ~(data & 0xf8);
			ppc4xx_spu_update_irq_states();
			break;

		/* enable/disable the timer if one of these is enabled */
		case SPU4XX_RX_COMMAND:
		case SPU4XX_TX_COMMAND:
			oldstate = m_spu.regs[SPU4XX_RX_COMMAND] | m_spu.regs[SPU4XX_TX_COMMAND];
			m_spu.regs[offset] = data;
			newstate = m_spu.regs[SPU4XX_RX_COMMAND] | m_spu.regs[SPU4XX_TX_COMMAND];
			if ((oldstate ^ newstate) & 0x80)
				ppc4xx_spu_timer_reset();
			ppc4xx_spu_update_irq_states();
			break;

		/* if the divisor changes, we need to update the timer */
		case SPU4XX_BAUD_DIVISOR_H:
		case SPU4XX_BAUD_DIVISOR_L:
			if (data != m_spu.regs[offset])
			{
				m_spu.regs[offset] = data;
				ppc4xx_spu_timer_reset();
			}
			break;

		/* if the number of data bits or stop bits changes, we need to update the timer */
		case SPU4XX_CONTROL:
			oldstate = m_spu.regs[offset];
			m_spu.regs[offset] = data;
			if ((oldstate ^ data) & 0x09)
				ppc4xx_spu_timer_reset();
			break;

		case SPU4XX_BUFFER:
			/* write to the transmit buffer and mark it full */
			m_spu.txbuf = data;
			m_spu.regs[SPU4XX_LINE_STATUS] &= ~0x04;
			break;

		default:
			if (offset < ARRAY_LENGTH(m_spu.regs))
				m_spu.regs[offset] = data;
			break;
	}
}



/*-------------------------------------------------
    ppc4xx_spu_set_tx_handler - PowerPC 4XX-
    specific TX handler configuration
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_spu_set_tx_handler(write8_delegate callback)
{
	m_spu.tx_cb = callback;
}


/*-------------------------------------------------
    ppc4xx_spu_receive_byte - PowerPC 4XX-
    specific serial byte receive
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_spu_receive_byte(UINT8 byteval)
{
	ppc4xx_spu_rx_data(byteval);
}

/*-------------------------------------------------
    ppc4xx_set_dma_read_handler - PowerPC 4XX-
    specific external DMA read handler configuration
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_set_dma_read_handler(int channel, read32_delegate callback, int rate)
{
	m_ext_dma_read_cb[channel] = callback;
	m_buffered_dma_rate[channel] = rate;
}

/*-------------------------------------------------
    ppc4xx_set_dma_write_handler - PowerPC 4XX-
    specific external DMA write handler configuration
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_set_dma_write_handler(int channel, write32_delegate callback, int rate)
{
	m_ext_dma_write_cb[channel] = callback;
	m_buffered_dma_rate[channel] = rate;
}

/*-------------------------------------------------
    ppc4xx_set_dcr_read_handler
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_set_dcr_read_handler(read32_delegate dcr_read_func)
{
	m_dcr_read_func = dcr_read_func;

}

/*-------------------------------------------------
    ppc4xx_set_dcr_write_handler
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_set_dcr_write_handler(write32_delegate dcr_write_func)
{
	m_dcr_write_func = dcr_write_func;
}
