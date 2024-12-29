// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppccom.c

    Common PowerPC definitions and functions

***************************************************************************/

#include "emu.h"
#include "ppccom.h"
#include "ppcfe.h"
#include "ppc_dasm.h"

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_SPU              (0)
#define PRINTF_DECREMENTER      (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DOUBLE_SIGN     (0x8000000000000000U)
#define DOUBLE_EXP      (0x7ff0000000000000U)
#define DOUBLE_FRAC     (0x000fffffffffffffU)
#define DOUBLE_ZERO     (0)



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

/* lookup table for FP modes */
static const uint8_t fpmode_source[4] =
{
	uml::ROUND_ROUND,
	uml::ROUND_TRUNC,
	uml::ROUND_CEIL,
	uml::ROUND_FLOOR
};

/* flag lookup table for SZ */
static const uint8_t sz_cr_table_source[32] =
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
static const uint8_t cmp_cr_table_source[32] =
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
static const uint8_t cmpl_cr_table_source[32] =
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
static const uint8_t fcmp_cr_table_source[32] =
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


DEFINE_DEVICE_TYPE(PPC601,    ppc601_device,    "ppc601",     "IBM PowerPC 601")
DEFINE_DEVICE_TYPE(PPC602,    ppc602_device,    "ppc602",     "IBM PowerPC 602")
DEFINE_DEVICE_TYPE(PPC603,    ppc603_device,    "ppc603",     "IBM PowerPC 603")
DEFINE_DEVICE_TYPE(PPC603E,   ppc603e_device,   "ppc603e",    "IBM PowerPC 603E")
DEFINE_DEVICE_TYPE(PPC603R,   ppc603r_device,   "ppc603r",    "IBM PowerPC 603R")
DEFINE_DEVICE_TYPE(PPC604,    ppc604_device,    "ppc604",     "IBM PowerPC 604")
DEFINE_DEVICE_TYPE(MPC8240,   mpc8240_device,   "mpc8240",    "IBM PowerPC MPC8240")
DEFINE_DEVICE_TYPE(PPC403GA,  ppc403ga_device,  "ppc403ga",   "IBM PowerPC 403GA")
DEFINE_DEVICE_TYPE(PPC403GCX, ppc403gcx_device, "ppc403gcx",  "IBM PowerPC 403GCX")
DEFINE_DEVICE_TYPE(PPC405GP,  ppc405gp_device,  "ppc405gp",   "IBM PowerPC 405GP")
DEFINE_DEVICE_TYPE(PPC740,    ppc740_device,    "ppc740",     "IBM PowerPC 740")
DEFINE_DEVICE_TYPE(PPC750,    ppc750_device,    "ppc750",     "IBM PowerPC 750")


ppc_device::ppc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int address_bits, int data_bits, powerpc_flavor flavor, uint32_t cap, uint32_t tb_divisor, address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, device_vtlb_interface(mconfig, *this, AS_PROGRAM)
	, m_program_config("program", ENDIANNESS_BIG, data_bits, address_bits, 0, internal_map)
	, c_bus_frequency(0)
	, c_serial_clock(0)
	, m_core(nullptr)
	, m_bus_freq_multiplier(1)
	, m_flavor(flavor)
	, m_cap(cap)
	, m_tb_divisor(tb_divisor)
	, m_spu(*this)
	, m_dcr_read_func(*this)
	, m_dcr_write_func(*this)
	, m_dcstore_cb(*this)
	, m_ext_dma_read_cb(*this)
	, m_ext_dma_write_cb(*this)
	, m_cache(CACHE_SIZE + sizeof(internal_ppc_state))
	, m_drcuml(nullptr)
	, m_drcfe(nullptr)
	, m_drcoptions(0)
	, m_dasm(powerpc_disassembler())
{
	m_program_config.m_logaddr_width = 32;
	m_program_config.m_page_shift = POWERPC_MIN_PAGE_SHIFT;

	// configure the virtual TLB
	set_vtlb_dynamic_entries(POWERPC_TLB_ENTRIES);
	if (m_cap & PPCCAP_603_MMU)
		set_vtlb_fixed_entries(PPC603_FIXED_TLB_ENTRIES);
}

ppc_device::~ppc_device()
{
}

//ppc403_device::ppc403_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
//  : ppc_device(mconfig, PPC403, "PPC403", tag, owner, clock, "ppc403", 32?, 64?)
//{
//}
//
//ppc405_device::ppc405_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
//  : ppc_device(mconfig, PPC405, "PPC405", tag, owner, clock, "ppc405", 32?, 64?)
//{
//}

ppc603_device::ppc603_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC603, tag, owner, clock, 32, 64, PPC_MODEL_603, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, address_map_constructor())
{
}

ppc603e_device::ppc603e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC603E, tag, owner, clock, 32, 64, PPC_MODEL_603E, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, address_map_constructor())
{
}

ppc603r_device::ppc603r_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC603R, tag, owner, clock, 32, 64, PPC_MODEL_603R, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, address_map_constructor())
{
}

ppc602_device::ppc602_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC602, tag, owner, clock, 32, 64, PPC_MODEL_602, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, address_map_constructor())
{
}

mpc8240_device::mpc8240_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, MPC8240, tag, owner, clock, 32, 64, PPC_MODEL_MPC8240, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4/* unknown */, address_map_constructor())
{
}

ppc601_device::ppc601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC601, tag, owner, clock, 32, 64, PPC_MODEL_601, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_MFIOC | PPCCAP_601BAT, 0/* no TB */, address_map_constructor())
{
}

std::unique_ptr<util::disasm_interface> ppc601_device::create_disassembler()
{
	// 601 has both POWER and PowerPC instructions
	return std::make_unique<powerpc_disassembler>((powerpc_disassembler::implementation)(powerpc_disassembler::I_POWER|powerpc_disassembler::I_POWERPC));
}

ppc604_device::ppc604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC604, tag, owner, clock, 32, 64, PPC_MODEL_604, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_604_MMU, 4, address_map_constructor())
{
}

ppc740_device::ppc740_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC740, tag, owner, clock, 32, 64, PPC_MODEL_740, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_604_MMU | PPCCAP_750_TLB , 4, address_map_constructor())
{
}

ppc750_device::ppc750_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc_device(mconfig, PPC750, tag, owner, clock, 32, 64, PPC_MODEL_750, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_604_MMU | PPCCAP_750_TLB, 4, address_map_constructor())
{
}

void ppc4xx_device::internal_ppc4xx(address_map &map)
{
	map(0x40000000, 0x4000000f).rw(FUNC(ppc4xx_device::ppc4xx_spu_r), FUNC(ppc4xx_device::ppc4xx_spu_w));
}

ppc4xx_device::ppc4xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, powerpc_flavor flavor, uint32_t cap, uint32_t tb_divisor)
	: ppc_device(mconfig, type, tag, owner, clock, 31, 32, flavor, cap, tb_divisor, address_map_constructor(FUNC(ppc4xx_device::internal_ppc4xx), this))
{
}

ppc403ga_device::ppc403ga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc4xx_device(mconfig, PPC403GA, tag, owner, clock, PPC_MODEL_403GA, PPCCAP_4XX, 1)
{
}

ppc403gcx_device::ppc403gcx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc4xx_device(mconfig, PPC403GCX, tag, owner, clock, PPC_MODEL_403GCX, PPCCAP_4XX, 1)
{
}

ppc405gp_device::ppc405gp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ppc4xx_device(mconfig, PPC405GP, tag, owner, clock, PPC_MODEL_405GP, PPCCAP_4XX | PPCCAP_VEA, 1)
{
}

device_memory_interface::space_config_vector ppc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    page_access_allowed - return true if we are
    allowed to access memory based on the type
    of access and the protection bits
-------------------------------------------------*/

static inline bool page_access_allowed(int transtype, uint8_t key, uint8_t protbits)
{
	if (key == 0)
		return (transtype == device_memory_interface::TR_WRITE) ? (protbits != 3) : true;
	else
		return (transtype == device_memory_interface::TR_WRITE) ? (protbits == 2) : (protbits != 0);
}


/*-------------------------------------------------
    get_cr - return the current CR value
-------------------------------------------------*/

inline uint32_t ppc_device::get_cr()
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

inline void ppc_device::set_cr(uint32_t value)
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

inline uint32_t ppc_device::get_xer()
{
	return m_core->spr[SPR_XER] | (m_core->xerso << 31);
}


/*-------------------------------------------------
    set_xer - set the current XER value
-------------------------------------------------*/

inline void ppc_device::set_xer(uint32_t value)
{
	m_core->spr[SPR_XER] = value & ~XER_SO;
	m_core->xerso = value >> 31;
}


/*-------------------------------------------------
    get_timebase - return the current timebase
    value
-------------------------------------------------*/

inline uint64_t ppc_device::get_timebase()
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

inline void ppc_device::set_timebase(uint64_t newtb)
{
	m_tb_zero_cycles = total_cycles() - newtb * m_tb_divisor;
}


/*-------------------------------------------------
    get_decremeter - return the current
    decrementer value
-------------------------------------------------*/

inline uint32_t ppc_device::get_decrementer()
{
	int64_t cycles_until_zero = m_dec_zero_cycles - total_cycles();
	cycles_until_zero = std::max<int64_t>(cycles_until_zero, 0);

	if (!m_tb_divisor)
	{
		return 0;
	}

	return cycles_until_zero / m_tb_divisor;
}


/*-------------------------------------------------
    set_decrementer - set the decremeter
-------------------------------------------------*/

inline void ppc_device::set_decrementer(uint32_t newdec)
{
	uint64_t cycles_until_done = ((uint64_t)newdec + 1) * m_tb_divisor;
	uint32_t curdec = get_decrementer();

	if (!m_tb_divisor)
	{
		return;
	}

	if (PRINTF_DECREMENTER)
	{
		uint64_t total = total_cycles();
		osd_printf_debug("set_decrementer: olddec=%08X newdec=%08X divisor=%d totalcyc=%016X timer=%016X\n",
				curdec, newdec, m_tb_divisor,
				total, cycles_until_done);
	}

	m_dec_zero_cycles = total_cycles() + cycles_until_done;
	m_decrementer_int_timer->adjust(cycles_to_attotime(cycles_until_done));

	if ((int32_t)curdec >= 0 && (int32_t)newdec < 0)
		m_core->irq_pending |= 0x02;
}


#if 0
/*-------------------------------------------------
    is_nan_double - is a double value a NaN
-------------------------------------------------*/

static inline int is_nan_double(double x)
{
	uint64_t xi = *(uint64_t*)&x;
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
	uint64_t xi = *(uint64_t*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & 0x0008000000000000U) == 0x0008000000000000U) );
}


#if 0
/*-------------------------------------------------
    is_snan_double - is a double value a
    signaling NaN
-------------------------------------------------*/

static inline int is_snan_double(double x)
{
	uint64_t xi = *(uint64_t*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) &&
			((xi & 0x0008000000000000U) == DOUBLE_ZERO) );
}
#endif


/*-------------------------------------------------
    is_infinity_double - is a double value
    infinity
-------------------------------------------------*/

static inline int is_infinity_double(double x)
{
	uint64_t xi = *(uint64_t*)&x;
	return( ((xi & DOUBLE_EXP) == DOUBLE_EXP) &&
			((xi & DOUBLE_FRAC) == DOUBLE_ZERO) );
}


/*-------------------------------------------------
    is_normalized_double - is a double value
    normalized
-------------------------------------------------*/

static inline int is_normalized_double(double x)
{
	uint64_t exp;
	uint64_t xi = *(uint64_t*)&x;
	exp = (xi & DOUBLE_EXP) >> 52;

	return (exp >= 1) && (exp <= 2046);
}


/*-------------------------------------------------
    is_denormalized_double - is a double value
    denormalized
-------------------------------------------------*/

static inline int is_denormalized_double(double x)
{
	uint64_t xi = *(uint64_t*)&x;
	return( ((xi & DOUBLE_EXP) == 0) &&
			((xi & DOUBLE_FRAC) != DOUBLE_ZERO) );
}


/*-------------------------------------------------
    sign_double - return sign of a double value
-------------------------------------------------*/

static inline int sign_double(double x)
{
	uint64_t xi = *(uint64_t*)&x;
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

	m_spu.clear();
	m_pit_reload = 0;
	m_irqstate = 0;
	memset(m_buffered_dma_rate, 0, sizeof(m_buffered_dma_rate));
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

	m_serial_clock = 0;

	m_cache_line_size = 32;
	m_cpu_clock = clock();
	m_program = &space(AS_PROGRAM);
	if(m_cap & PPCCAP_4XX)
	{
		m_program->cache(m_cache32);
		m_pr32 = [this](offs_t address) -> u32 { return m_cache32.read_dword(address); };
		m_prptr = [this](offs_t address) -> const void * { return m_cache32.read_ptr(address); };
	}
	else
	{
		m_program->cache(m_cache64);
		m_pr32 = [this](offs_t address) -> u32 { return m_cache64.read_dword(address); };
		if(space_config()->m_endianness != ENDIANNESS_NATIVE)
			m_prptr = [this](offs_t address) -> const void * {
				const u32 *ptr = static_cast<u32 *>(m_cache64.read_ptr(address & ~7));
				if(!(address & 4))
					ptr++;
				return ptr;
			};
		else
			m_prptr = [this](offs_t address) -> const void * {
				const u32 *ptr = static_cast<u32 *>(m_cache64.read_ptr(address & ~7));
				if(address & 4)
					ptr++;
				return ptr;
			};
	}
	m_system_clock = c_bus_frequency != 0 ? c_bus_frequency : clock();
	m_dcr_read_func.set(nullptr);
	m_dcr_write_func.set(nullptr);

	m_tb_divisor = (m_tb_divisor * clock() + m_system_clock / 2 - 1) / m_system_clock;

	m_serial_clock = c_serial_clock != 0 ? c_serial_clock : 3'686'400; // TODO: get rid of this hard-coded magic number
	if (m_serial_clock > m_system_clock / 2)
		fatalerror("%s: PPC: serial clock (%d) must not be more than half of the system clock (%d)\n", tag(), m_serial_clock, m_system_clock);

	/* allocate a timer for the compare interrupt */
	if ((m_cap & PPCCAP_OEA) && (m_tb_divisor))
		m_decrementer_int_timer = timer_alloc(FUNC(ppc_device::decrementer_int_callback), this);

	/* and for the 4XX interrupts if needed */
	if (m_cap & PPCCAP_4XX)
	{
		m_fit_timer = timer_alloc(FUNC(ppc_device::ppc4xx_fit_callback), this);
		m_pit_timer = timer_alloc(FUNC(ppc_device::ppc4xx_pit_callback), this);
		m_spu.timer = timer_alloc(FUNC(ppc_device::ppc4xx_spu_callback), this);
	}

	if (m_cap & PPCCAP_4XX)
	{
		m_buffered_dma_timer[0] = timer_alloc(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this);
		m_buffered_dma_timer[1] = timer_alloc(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this);
		m_buffered_dma_timer[2] = timer_alloc(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this);
		m_buffered_dma_timer[3] = timer_alloc(FUNC(ppc_device::ppc4xx_buffered_dma_callback), this);

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

	for (int regnum = 0; regnum < 16; regnum++)
		state_add(PPC_SR0 + regnum, string_format("SR%d", regnum).c_str(), m_core->sr[regnum]).formatstr("%08X");

	for (int regnum = 0; regnum < 32; regnum++)
		state_add(PPC_R0 + regnum, string_format("R%d", regnum).c_str(), m_core->r[regnum]).formatstr("%08X");

	for (int regnum = 0; regnum < 32; regnum++)
		state_add(PPC_F0 + regnum, string_format("F%d", regnum).c_str(), m_core->f[regnum]).formatstr("%12s");
	state_add(PPC_FPSCR, "FPSCR", m_core->fpscr).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_core->pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_core->pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).noshow().formatstr("%1s");

	set_icountptr(m_core->icount);

	uint32_t flags = 0;
	/* initialize the UML generator */
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, flags, 8, 32, 2);

	/* add symbols for our stuff */
	m_drcuml->symbol_add(&m_core->pc, sizeof(m_core->pc), "pc");
	m_drcuml->symbol_add(&m_core->icount, sizeof(m_core->icount), "icount");
	for (int regnum = 0; regnum < 32; regnum++)
	{
		char buf[10];
		snprintf(buf, 10, "r%d", regnum);
		m_drcuml->symbol_add(&m_core->r[regnum], sizeof(m_core->r[regnum]), buf);
		snprintf(buf, 10, "fpr%d", regnum);
		m_drcuml->symbol_add(&m_core->f[regnum], sizeof(m_core->f[regnum]), buf);
	}
	for (int regnum = 0; regnum < 8; regnum++)
	{
		char buf[10];
		snprintf(buf, 10, "cr%d", regnum);
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
	m_drcfe = std::make_unique<frontend>(*this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);

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


		if (beinfo.direct_fregs > 3)
			m_fdregmap[0] = uml::F3;
		if (beinfo.direct_fregs > 4)
			m_fdregmap[1] = uml::F4;
		if (beinfo.direct_fregs > 5)
			m_fdregmap[2] = uml::F5;
		if (beinfo.direct_fregs > 6)
			m_fdregmap[3] = uml::F6;
		if (beinfo.direct_fregs > 7)
			m_fdregmap[30] = uml::F7;
		if (beinfo.direct_fregs > 8)
			m_fdregmap[31] = uml::F8;
	}

	/* mark the cache dirty so it is updated on next execute */
	m_cache_dirty = true;
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
			m_debugger_temp = (uint32_t)get_timebase();
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
			set_timebase((get_timebase() & ~u64(0x00ffffff00000000U)) | m_debugger_temp);
			break;

		case PPC_TBH:
			set_timebase((get_timebase() & ~u64(0x00000000ffffffffU)) | ((uint64_t)(m_debugger_temp & 0x00ffffff) << 32));
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
			str = string_format("%12f", m_core->f[0]);
			break;

		case PPC_F1:
			str = string_format("%12f", m_core->f[1]);
			break;

		case PPC_F2:
			str = string_format("%12f", m_core->f[2]);
			break;

		case PPC_F3:
			str = string_format("%12f", m_core->f[3]);
			break;

		case PPC_F4:
			str = string_format("%12f", m_core->f[4]);
			break;

		case PPC_F5:
			str = string_format("%12f", m_core->f[5]);
			break;

		case PPC_F6:
			str = string_format("%12f", m_core->f[6]);
			break;

		case PPC_F7:
			str = string_format("%12f", m_core->f[7]);
			break;

		case PPC_F8:
			str = string_format("%12f", m_core->f[8]);
			break;

		case PPC_F9:
			str = string_format("%12f", m_core->f[9]);
			break;

		case PPC_F10:
			str = string_format("%12f", m_core->f[10]);
			break;

		case PPC_F11:
			str = string_format("%12f", m_core->f[11]);
			break;

		case PPC_F12:
			str = string_format("%12f", m_core->f[12]);
			break;

		case PPC_F13:
			str = string_format("%12f", m_core->f[13]);
			break;

		case PPC_F14:
			str = string_format("%12f", m_core->f[14]);
			break;

		case PPC_F15:
			str = string_format("%12f", m_core->f[15]);
			break;

		case PPC_F16:
			str = string_format("%12f", m_core->f[16]);
			break;

		case PPC_F17:
			str = string_format("%12f", m_core->f[17]);
			break;

		case PPC_F18:
			str = string_format("%12f", m_core->f[18]);
			break;

		case PPC_F19:
			str = string_format("%12f", m_core->f[19]);
			break;

		case PPC_F20:
			str = string_format("%12f", m_core->f[20]);
			break;

		case PPC_F21:
			str = string_format("%12f", m_core->f[21]);
			break;

		case PPC_F22:
			str = string_format("%12f", m_core->f[22]);
			break;

		case PPC_F23:
			str = string_format("%12f", m_core->f[23]);
			break;

		case PPC_F24:
			str = string_format("%12f", m_core->f[24]);
			break;

		case PPC_F25:
			str = string_format("%12f", m_core->f[25]);
			break;

		case PPC_F26:
			str = string_format("%12f", m_core->f[26]);
			break;

		case PPC_F27:
			str = string_format("%12f", m_core->f[27]);
			break;

		case PPC_F28:
			str = string_format("%12f", m_core->f[28]);
			break;

		case PPC_F29:
			str = string_format("%12f", m_core->f[29]);
			break;

		case PPC_F30:
			str = string_format("%12f", m_core->f[30]);
			break;

		case PPC_F31:
			str = string_format("%12f", m_core->f[31]);
			break;
	}
}


/*-------------------------------------------------
    ppccom_exit - common cleanup/exit
-------------------------------------------------*/

void ppc_device::device_stop()
{
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
			decrementer_int_callback(0);
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
	{
		m_core->spr[SPR602_ESASRR] = 0;
		m_core->spr[SPR603_HID0] = 1;
	}

	/* time base starts here */
	m_tb_zero_cycles = total_cycles();

	/* clear interrupts */
	m_core->irq_pending = 0;

	/* flush the TLB */
	if (m_cap & PPCCAP_603_MMU)
	{
		for (int tlbindex = 0; tlbindex < PPC603_FIXED_TLB_ENTRIES; tlbindex++)
		{
			vtlb_load(tlbindex, 0, 0, 0);
		}
	}

	/* Mark the cache dirty */
	m_core->mode = 0;
	m_cache_dirty = true;
}


/*-------------------------------------------------
    ppccom_dasm - handle disassembly for a
    CPU
-------------------------------------------------*/

std::unique_ptr<util::disasm_interface> ppc_device::create_disassembler()
{
	return std::make_unique<powerpc_disassembler>(powerpc_disassembler::I_POWERPC);
}


/*-------------------------------------------------
    ppccom_dcstore_callback - call the dcstore
    callback if installed
-------------------------------------------------*/

void ppc_device::ppccom_dcstore_callback()
{
	if (!m_dcstore_cb.isnull())
	{
		m_dcstore_cb(m_core->param0, 0);
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

uint32_t ppc_device::ppccom_translate_address_internal(int intention, bool debug, offs_t &address)
{
	int transpriv = ((intention & TR_USER) == 0);   // 1 for supervisor, 0 for user
	int transtype = intention & TR_TYPE;
	offs_t hash, hashbase, hashmask;
	int batbase, batnum, hashnum;
	uint32_t segreg;

	/* 4xx case: "TLB" really just caches writes and checks compare registers */
	if (m_cap & PPCCAP_4XX)
	{
		/* we don't support the MMU of the 403GCX */
		if (m_flavor == PPC_MODEL_403GCX && (m_core->msr & MSROEA_DR))
			fatalerror("MMU enabled but not supported!\n");

		/* only check if PE is enabled */
		if (transtype == TR_WRITE && (m_core->msr & MSR4XX_PE))
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
	if ((transtype == TR_FETCH && (m_core->msr & MSROEA_IR) == 0) || (transtype != TR_FETCH && (m_core->msr & MSROEA_DR) == 0))
		return 0x001;

	/* first scan the appropriate BAT */
	if (m_cap & PPCCAP_601BAT)
	{
		for (batnum = 0; batnum < 4; batnum++)
		{
			uint32_t upper = m_core->spr[SPROEA_IBAT0U + 2*batnum + 0];
			uint32_t lower = m_core->spr[SPROEA_IBAT0U + 2*batnum + 1];
			int privbit = ((intention & TR_USER) == 0) ? 3 : 2;

//            printf("bat %d upper = %08x privbit %d\n", batnum, upper, privbit);

			// is this pair valid?
			if (lower & 0x40)
			{
				uint32_t mask = ((lower & 0x3f) << 17) ^ 0xfffe0000;
				uint32_t addrout;
				uint32_t key = (upper >> privbit) & 1;

				/* check for a hit against this bucket */
				if ((address & mask) == (upper & mask))
				{
					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, key, upper & 3))
					{
						return DSISR_PROTECTED | ((transtype == TR_WRITE) ? DSISR_STORE : 0);
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
		batbase = (transtype == TR_FETCH) ? SPROEA_IBAT0U : SPROEA_DBAT0U;

		for (batnum = 0; batnum < 4; batnum++)
		{
			uint32_t upper = m_core->spr[batbase + 2*batnum + 0];

			/* check user/supervisor valid bit */
			if ((upper >> transpriv) & 0x01)
			{
				uint32_t mask = (~upper << 15) & 0xfffe0000;

				/* check for a hit against this bucket */
				if ((address & mask) == (upper & mask))
				{
					uint32_t lower = m_core->spr[batbase + 2*batnum + 1];

					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, 1, lower & 3))
					{
						return DSISR_PROTECTED | ((transtype == TR_WRITE) ? DSISR_STORE : 0);
					}

					/* otherwise we're good */
					address = (lower & mask) | (address & ~mask);
					return 0x001;
				}
			}
		}
	}

#if 1
	/* 602-specific Protection-Only mode */
	if (m_flavor == PPC_MODEL_602 && m_core->spr[SPR603_HID0] & 0x00000080)
	{
		// TODO
		return 0x001;
	}
#endif

	/* look up the segment register */
	segreg = m_core->sr[address >> 28];
	if (transtype == TR_FETCH && (segreg & 0x10000000))
		return DSISR_PROTECTED | ((transtype == TR_WRITE) ? DSISR_STORE : 0);

	/* check for memory-forced I/O */
	if (m_cap & PPCCAP_MFIOC)
	{
		if ((transtype != TR_FETCH) && ((segreg & 0x87f00000) == 0x87f00000))
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
		uint32_t entry = vtlb_table()[address >> 12];
		m_core->mmu603_cmp = 0x80000000 | ((segreg & 0xffffff) << 7) | (0 << 6) | ((address >> 22) & 0x3f);
		m_core->mmu603_hash[0] = hashbase | ((hash << 6) & hashmask);
		m_core->mmu603_hash[1] = hashbase | ((~hash << 6) & hashmask);
		if ((entry & (FLAG_FIXED | FLAG_VALID)) == (FLAG_FIXED | FLAG_VALID))
		{
			address = (entry & 0xfffff000) | (address & 0x00000fff);
			return 0x001;
		}
		return DSISR_NOT_FOUND | ((transtype == TR_WRITE) ? DSISR_STORE : 0);
	}

	/* loop twice over hashes */
	for (hashnum = 0; hashnum < 2; hashnum++)
	{
		offs_t ptegaddr = hashbase | ((hash << 6) & hashmask);
		uint32_t *ptegptr = (uint32_t *)m_program->get_read_ptr(ptegaddr);

		/* should only have valid memory here, but make sure */
		if (ptegptr != nullptr)
		{
			uint32_t targetupper = 0x80000000 | ((segreg & 0xffffff) << 7) | (hashnum << 6) | ((address >> 22) & 0x3f);
			int ptenum;

			/* scan PTEs */
			for (ptenum = 0; ptenum < 8; ptenum++)
				if (ptegptr[BYTE_XOR_BE(ptenum * 2)] == targetupper)
				{
					uint32_t pteglower = ptegptr[BYTE_XOR_BE(ptenum * 2 + 1)];

					/* verify protection; if we fail, return false and indicate a protection violation */
					if (!page_access_allowed(transtype, (segreg >> (29 + transpriv)) & 1, pteglower & 3))
						return DSISR_PROTECTED | ((transtype == TR_WRITE) ? DSISR_STORE : 0);

					/* update page table bits */
					if (!debug)
					{
						pteglower |= 0x100;
						if (transtype == TR_WRITE)
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
	return DSISR_NOT_FOUND | ((transtype == TR_WRITE) ? DSISR_STORE : 0);
}


/*-------------------------------------------------
    ppccom_translate_address - translate an address
    from logical to physical
-------------------------------------------------*/

bool ppc_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);

	/* only applies to the program address space */
	if (spacenum != AS_PROGRAM)
		return true;

	/* translation is successful if the internal routine returns 0 or 1 */
	return (ppccom_translate_address_internal(intention, true, address) <= 1);
}


/*-------------------------------------------------
    ppccom_tlb_fill - handle a missing TLB entry
-------------------------------------------------*/

void ppc_device::ppccom_tlb_fill()
{
	offs_t address = m_core->param0;
	if(ppccom_translate_address_internal(m_core->param1, false, address) > 1)
		return;
	vtlb_fill(m_core->param0, address, m_core->param1);
}


/*-------------------------------------------------
    ppccom_tlb_flush - flush the entire TLB,
    including fixed entries
-------------------------------------------------*/

void ppc_device::ppccom_tlb_flush()
{
	vtlb_flush_dynamic();
}



/***************************************************************************
    OPCODE HANDLING
***************************************************************************/

/*-------------------------------------------------
    ppccom_get_dsisr - gets the DSISR value for a
    failing TLB lookup's data access exception.
-------------------------------------------------*/

void ppc_device::ppccom_get_dsisr()
{
	int intent = 0;

	if (m_core->param1 & 1)
	{
		intent = TR_WRITE;
	}
	else
	{
		intent = TR_READ;
	}

	m_core->param1 = ppccom_translate_address_internal(intent, false, m_core->param0);
}

/*-------------------------------------------------
    ppccom_execute_tlbie - execute a TLBIE
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_tlbie()
{
	vtlb_flush_address(m_core->param0);
}


/*-------------------------------------------------
    ppccom_execute_tlbia - execute a TLBIA
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_tlbia()
{
	vtlb_flush_dynamic();
}


/*-------------------------------------------------
    ppccom_execute_tlbl - execute a TLBLD/TLBLI
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_tlbl()
{
	uint32_t address = m_core->param0;
	int isitlb = m_core->param1;
	vtlb_entry flags;
	int entrynum;

	if (m_flavor == PPC_MODEL_602) // TODO
		return;

	/* determine entry number; we use machine().rand() for associativity */
	entrynum = ((address >> 12) & 0x1f) | (machine().rand() & 0x20) | (isitlb ? 0x40 : 0);

	/* determine the flags */
	flags = FLAG_VALID | READ_ALLOWED | FETCH_ALLOWED;
	if (m_core->spr[SPR603_RPA] & 0x80)
		flags |= WRITE_ALLOWED;
	if (isitlb)
		flags |= FETCH_ALLOWED;

	/* load the entry */
	vtlb_load(entrynum, 1, address, (m_core->spr[SPR603_RPA] & 0xfffff000) | flags);
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

	/* handle 602 SPRs */
	if (m_flavor == PPC_MODEL_602)
	{ // TODO: Which are read/write only?
		switch (m_core->param0)
		{
			case SPR602_TCR:
			case SPR602_IBR:
			case SPR602_ESASRR:
			case SPR602_SEBR:
			case SPR602_SER:
			case SPR602_SP:
			case SPR602_LT:
				m_core->param1 = m_core->spr[m_core->param0];
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

	/* handle 602 SPRs */
	if (m_flavor == PPC_MODEL_602)
	{
		switch (m_core->param0)
		{ // TODO: Which are read/write only?
			case SPR602_TCR:
			case SPR602_IBR:
			case SPR602_ESASRR:
			case SPR602_SEBR:
			case SPR602_SER:
			case SPR602_SP:
			case SPR602_LT:
				m_core->spr[m_core->param0] = m_core->param1;
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
				set_timebase((get_timebase() & ~u64(0xffffffff00000000U)) | m_core->param1);
				return;
			case SPR603_TBU_W:
				set_timebase((get_timebase() & ~u64(0x00000000ffffffffU)) | ((uint64_t)m_core->param1 << 32));
				return;
		}
	}

	/* handle 4XX SPRs */
	if (m_cap & PPCCAP_4XX)
	{
		uint32_t oldval = m_core->spr[m_core->param0];
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
					ppc4xx_fit_callback(false);
				if ((oldval ^ m_core->spr[SPR4XX_TCR]) & PPC4XX_TCR_PIE)
					ppc4xx_pit_callback(false);
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
				ppc4xx_pit_callback(false);
				return;

			/* timebase */
			case SPR4XX_TBLO:
				set_timebase((get_timebase() & ~u64(0x00ffffff00000000U)) | m_core->param1);
				return;
			case SPR4XX_TBHI:
				set_timebase((get_timebase() & ~u64(0x00000000ffffffffU)) | ((uint64_t)(m_core->param1 & 0x00ffffff) << 32));
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
		if (m_core->param0 < std::size(m_dcr))
			m_core->param1 = m_dcr[m_core->param0];
		else
			m_core->param1 = 0;
	} else {
		m_core->param1 = m_dcr_read_func(m_core->param0);
	}
}


/*-------------------------------------------------
    ppccom_execute_mtdcr - execute an MTDCR
    instruction
-------------------------------------------------*/

void ppc_device::ppccom_execute_mtdcr()
{
	uint8_t oldval;

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
		if (m_core->param0 < std::size(m_dcr))
			m_dcr[m_core->param0] = m_core->param1;
	} else {
		m_dcr_write_func(m_core->param0,m_core->param1);
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
	uint32_t fprf;
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
	uint64_t cycles_until_next;

	/* set the decrementer IRQ state */
	m_core->irq_pending |= 0x02;

	/* advance by another full rev */
	m_dec_zero_cycles += (uint64_t)m_tb_divisor << 32;
	cycles_until_next = m_dec_zero_cycles - total_cycles();
	m_decrementer_int_timer->adjust(cycles_to_attotime(cycles_until_next));
}

/*-------------------------------------------------
    ppc_set_dcstore_callback - installs a callback
    for detecting datacache stores with dcbst
-------------------------------------------------*/

void ppc_device::ppc_set_dcstore_callback(write32sm_delegate callback)
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

void ppc_device::ppc4xx_set_irq_line(uint32_t bitmask, int state)
{
	uint32_t oldstate = m_irqstate;
	uint32_t levelmask;

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
		m_core->irq_pending = true;
	if ((m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_PIE) && (m_core->spr[SPR4XX_TSR] & PPC4XX_TSR_PIS))
		m_core->irq_pending = true;
}


/*-------------------------------------------------
    ppc4xx_get_irq_line - PowerPC 4XX-specific
    IRQ line state getter
-------------------------------------------------*/

int ppc_device::ppc4xx_get_irq_line(uint32_t bitmask)
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

bool ppc_device::ppc4xx_dma_decrement_count(int dmachan)
{
	uint32_t *dmaregs = &m_dcr[8 * dmachan];

	/* decrement the counter */
	dmaregs[DCR4XX_DMACT0]--;

	/* if non-zero, we keep going */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) != 0)
		return false;

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

		int64_t numdata = dmaregs[DCR4XX_DMACT0];
		if (numdata == 0)
			numdata = 65536;

		int64_t time = (numdata * 1000000) / m_buffered_dma_rate[dmachan];

		m_buffered_dma_timer[dmachan]->adjust(attotime::from_usec(time), dmachan);
	}
	else
	{
		/* set the complete bit and handle interrupts */
		m_dcr[DCR4XX_DMASR] |= 1 << (31 - dmachan);
	//  m_dcr[DCR4XX_DMASR] |= 1 << (27 - dmachan);
		ppc4xx_dma_update_irq_states();

		m_buffered_dma_timer[dmachan]->adjust(attotime::never, false);
	}
	return true;
}


/*-------------------------------------------------
    buffered_dma_callback - callback that fires
    when buffered DMA transfer is ready
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ppc_device::ppc4xx_buffered_dma_callback )
{
	int dmachan = param;

	static const uint8_t dma_transfer_width[4] = { 1, 2, 4, 16 };
	uint32_t *dmaregs = &m_dcr[8 * dmachan];
	int32_t destinc;
	uint8_t width;

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
				uint8_t data = 0;
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
				uint16_t data = 0;
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
				uint32_t data = 0;
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
				uint8_t data = m_program->read_byte(dmaregs[DCR4XX_DMADA0]);
				if (!m_ext_dma_write_cb[dmachan].isnull())
					(m_ext_dma_write_cb[dmachan])(1, data);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;

			/* word transfer */
			case 2:
			do
			{
				uint16_t data = m_program->read_word(dmaregs[DCR4XX_DMADA0]);
				if (!m_ext_dma_write_cb[dmachan].isnull())
					(m_ext_dma_write_cb[dmachan])(2, data);
				dmaregs[DCR4XX_DMADA0] += destinc;
			} while (!ppc4xx_dma_decrement_count(dmachan));
			break;

			/* dword transfer */
			case 4:
			do
			{
				uint32_t data = m_program->read_dword(dmaregs[DCR4XX_DMADA0]);
				if (!m_ext_dma_write_cb[dmachan].isnull())
					(m_ext_dma_write_cb[dmachan])(4, data);
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

bool ppc_device::ppc4xx_dma_fetch_transmit_byte(int dmachan, uint8_t *byte)
{
	uint32_t *dmaregs = &m_dcr[8 * dmachan];

	/* if the channel is not enabled, fail */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return false;

	/* if no transfers remaining, fail */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) == 0)
		return false;

	/* fetch the data */
	*byte = m_program->read_byte(dmaregs[DCR4XX_DMADA0]++);
	ppc4xx_dma_decrement_count(dmachan);
	return true;
}


/*-------------------------------------------------
    ppc4xx_dma_handle_receive_byte - receive a byte
    transmitted by a peripheral
-------------------------------------------------*/

bool ppc_device::ppc4xx_dma_handle_receive_byte(int dmachan, uint8_t byte)
{
	uint32_t *dmaregs = &m_dcr[8 * dmachan];

	/* if the channel is not enabled, fail */
	if (!(dmaregs[DCR4XX_DMACR0] & PPC4XX_DMACR_CE))
		return false;

	/* if no transfers remaining, fail */
	if ((dmaregs[DCR4XX_DMACT0] & 0xffff) == 0)
		return false;

	/* store the data */
	m_program->write_byte(dmaregs[DCR4XX_DMADA0]++, byte);
	ppc4xx_dma_decrement_count(dmachan);
	return true;
}


/*-------------------------------------------------
    ppc4xx_dma_execute - execute a DMA operation
    if one is pending
-------------------------------------------------*/

void ppc_device::ppc4xx_dma_exec(int dmachan)
{
	static const uint8_t dma_transfer_width[4] = { 1, 2, 4, 16 };
	uint32_t *dmaregs = &m_dcr[8 * dmachan];
	int32_t destinc, srcinc;
	uint8_t width;

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

				int64_t numdata = dmaregs[DCR4XX_DMACT0];
				if (numdata == 0)
					numdata = 65536;

				int64_t time;
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
		uint32_t timebase = get_timebase();
		uint32_t interval = 0x200 << (4 * ((m_core->spr[SPR4XX_TCR] & PPC4XX_TCR_FP_MASK) >> 24));
		uint32_t target = (timebase + interval) & ~(interval - 1);
		m_fit_timer->adjust(cycles_to_attotime((target + 1 - timebase) / m_tb_divisor), true);
	}

	/* otherwise, turn ourself off */
	else
		m_fit_timer->adjust(attotime::never, false);
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
		uint32_t timebase = get_timebase();
		uint32_t interval = m_pit_reload;
		uint32_t target = timebase + interval;
		m_pit_timer->adjust(cycles_to_attotime((target + 1 - timebase) / m_tb_divisor), true);
	}

	/* otherwise, turn ourself off */
	else
		m_pit_timer->adjust(attotime::never, false);
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

void ppc_device::ppc4xx_spu_rx_data(uint8_t data)
{
	uint32_t new_rxin;

	/* fail if we are going to overflow */
	new_rxin = (m_spu.rxin + 1) % std::size(m_spu.rxbuffer);
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
	uint8_t enabled = (m_spu.regs[SPU4XX_RX_COMMAND] | m_spu.regs[SPU4XX_TX_COMMAND]) & 0x80;

	/* if we're enabled, reset at the current baud rate */
	if (enabled)
	{
		attotime clockperiod = attotime::from_hz((m_dcr[DCR4XX_IOCR] & 0x02) ? m_serial_clock : m_system_clock);
		int divisor = ((m_spu.regs[SPU4XX_BAUD_DIVISOR_H] * 256 + m_spu.regs[SPU4XX_BAUD_DIVISOR_L]) & 0xfff) + 1;
		int bpc = 7 + ((m_spu.regs[SPU4XX_CONTROL] & 8) >> 3) + 1 + (m_spu.regs[SPU4XX_CONTROL] & 1);
		attotime charperiod = clockperiod * (divisor * 16 * bpc);
		m_spu.timer->adjust(charperiod, 0, charperiod);
		if (PRINTF_SPU)
			printf("ppc4xx_spu_timer_reset: baud rate = %.0f\n", charperiod.as_hz() * bpc);
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
				(m_spu.tx_cb)(m_spu.txbuf);

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
			uint8_t rxbyte;

			/* consume the byte and advance the out pointer */
			rxbyte = m_spu.rxbuffer[m_spu.rxout];
			m_spu.rxout = (m_spu.rxout + 1) % std::size(m_spu.rxbuffer);

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

uint8_t ppc4xx_device::ppc4xx_spu_r(offs_t offset)
{
	uint8_t result = 0xff;

	switch (offset)
	{
		case SPU4XX_BUFFER:
			result = m_spu.rxbuf;
			m_spu.regs[SPU4XX_LINE_STATUS] &= ~0x80;
			break;

		default:
			if (offset < std::size(m_spu.regs))
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

void ppc4xx_device::ppc4xx_spu_w(offs_t offset, uint8_t data)
{
	uint8_t oldstate, newstate;

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
			if (offset < std::size(m_spu.regs))
				m_spu.regs[offset] = data;
			break;
	}
}



/*-------------------------------------------------
    ppc4xx_spu_set_tx_handler - PowerPC 4XX-
    specific TX handler configuration
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_spu_set_tx_handler(write8smo_delegate callback)
{
	m_spu.tx_cb = callback;
}


/*-------------------------------------------------
    ppc4xx_spu_receive_byte - PowerPC 4XX-
    specific serial byte receive
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_spu_receive_byte(uint8_t byteval)
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

void ppc4xx_device::ppc4xx_set_dma_write_handler(int channel, write32sm_delegate callback, int rate)
{
	m_ext_dma_write_cb[channel] = callback;
	m_buffered_dma_rate[channel] = rate;
}

/*-------------------------------------------------
    ppc4xx_set_dcr_read_handler
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_set_dcr_read_handler(read32sm_delegate dcr_read_func)
{
	m_dcr_read_func = dcr_read_func;

}

/*-------------------------------------------------
    ppc4xx_set_dcr_write_handler
-------------------------------------------------*/

void ppc4xx_device::ppc4xx_set_dcr_write_handler(write32sm_delegate dcr_write_func)
{
	m_dcr_write_func = dcr_write_func;
}
