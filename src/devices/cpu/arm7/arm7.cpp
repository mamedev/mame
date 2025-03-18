// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7.cpp
 *   Portable CPU Emulator for 32-bit ARM v3/4/5/6
 *
 *   Copyright Steve Ellenoff
 *   Thumb, DSP, and MMU support and many bugfixes by R. Belmont and Ryan Holtz.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:

    ** This is a plain vanilla implementation of an ARM7 cpu which incorporates my ARM7 core.
       It can be used as is, or used to demonstrate how to utilize the arm7 core to create a cpu
       that uses the core, since there are numerous different mcu packages that incorporate an arm7 core.

       See the notes in the arm7core.inc file itself regarding issues/limitations of the arm7 core.
    **

TODO:
- Cleanups
- Fix and finish the DRC code, or remove it entirely

*****************************************************************************/

#include "emu.h"
#include "arm7.h"

#include "arm7core.h"   //include arm7 core
#include "arm7help.h"

#include "debug/debugcon.h"
#include "debugger.h"

#include <cassert>

#define LOG_MMU             (1U << 1)
#define LOG_DSP             (1U << 2)
#define LOG_COPRO_READS     (1U << 3)
#define LOG_COPRO_WRITES    (1U << 4)
#define LOG_COPRO_UNKNOWN   (1U << 5)
#define LOG_COPRO_RESERVED  (1U << 6)
#define LOG_TLB             (1U << 7)
#define LOG_TLB_MISS        (1U << 8)
#define LOG_PREFETCH        (1U << 9)

#define VERBOSE             (0) // (LOG_COPRO_READS | LOG_COPRO_WRITES | LOG_COPRO_UNKNOWN | LOG_COPRO_RESERVED)
#include "logmacro.h"

#define PRINT_HAPYFSH2      (0)
#define PRINT_CE_KERNEL     (0)

/* prototypes of coprocessor functions */
void arm7_dt_r_callback(arm_state *arm, uint32_t insn, uint32_t *prn, uint32_t (*read32)(arm_state *arm, uint32_t addr));
void arm7_dt_w_callback(arm_state *arm, uint32_t insn, uint32_t *prn, void (*write32)(arm_state *arm, uint32_t addr, uint32_t data));

// holder for the co processor Data Transfer Read & Write Callback funcs
void (*arm7_coproc_dt_r_callback)(arm_state *arm, uint32_t insn, uint32_t *prn, uint32_t (*read32)(arm_state *arm, uint32_t addr));
void (*arm7_coproc_dt_w_callback)(arm_state *arm, uint32_t insn, uint32_t *prn, void (*write32)(arm_state *arm, uint32_t addr, uint32_t data));


DEFINE_DEVICE_TYPE(ARM7,         arm7_cpu_device,         "arm7_le",      "ARM7 (little)")
DEFINE_DEVICE_TYPE(ARM7_BE,      arm7_be_cpu_device,      "arm7_be",      "ARM7 (big)")
DEFINE_DEVICE_TYPE(ARM710A,      arm710a_cpu_device,      "arm710a",      "ARM710a")
DEFINE_DEVICE_TYPE(ARM710T,      arm710t_cpu_device,      "arm710t",      "ARM710T")
DEFINE_DEVICE_TYPE(ARM7500,      arm7500_cpu_device,      "arm7500",      "ARM7500")
DEFINE_DEVICE_TYPE(ARM9,         arm9_cpu_device,         "arm9",         "ARM9")
DEFINE_DEVICE_TYPE(ARM920T,      arm920t_cpu_device,      "arm920t",      "ARM920T")
DEFINE_DEVICE_TYPE(ARM946ES,     arm946es_cpu_device,     "arm946es",     "ARM946ES")
DEFINE_DEVICE_TYPE(ARM11,        arm11_cpu_device,        "arm11",        "ARM11")
DEFINE_DEVICE_TYPE(ARM1176JZF_S, arm1176jzf_s_cpu_device, "arm1176jzf_s", "ARM1176JZF-S")
DEFINE_DEVICE_TYPE(PXA250,       pxa250_cpu_device,       "pxa250",       "Intel XScale PXA250")
DEFINE_DEVICE_TYPE(PXA255,       pxa255_cpu_device,       "pxa255",       "Intel XScale PXA255")
DEFINE_DEVICE_TYPE(PXA270,       pxa270_cpu_device,       "pxa270",       "Intel XScale PXA270")
DEFINE_DEVICE_TYPE(SA1100,       sa1100_cpu_device,       "sa1100",       "Intel StrongARM SA-1100")
DEFINE_DEVICE_TYPE(SA1110,       sa1110_cpu_device,       "sa1110",       "Intel StrongARM SA-1110")
DEFINE_DEVICE_TYPE(IGS036,       igs036_cpu_device,       "igs036",       "IGS036")

arm7_cpu_device::arm7_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, ARM7, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_LITTLE)
{
}

arm7_cpu_device::arm7_cpu_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag, device_t *owner,
		uint32_t clock,
		uint8_t archRev,
		uint32_t archFlags,
		endianness_t endianness,
		address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", endianness, 32, 32, 0, internal_map)
	, m_prefetch_word0_shift(endianness == ENDIANNESS_LITTLE ? 0 : 16)
	, m_prefetch_word1_shift(endianness == ENDIANNESS_LITTLE ? 16 : 0)
	, m_endian(endianness)
	, m_archRev(archRev)
	, m_archFlags(archFlags)
	, m_vectorbase(0)
	, m_pc(0)
{
	std::fill(std::begin(m_r), std::end(m_r), 0);
	uint32_t arch = ARM9_COPRO_ID_ARCH_V4;
	if (m_archFlags & ARCHFLAG_T)
		arch = ARM9_COPRO_ID_ARCH_V4T;

	m_copro_id = ARM9_COPRO_ID_MFR_ARM | arch | ARM9_COPRO_ID_PART_GENERICARM7;

	// TODO[RH]: Default to 3-instruction prefetch for unknown ARM variants. Derived cores should set the appropriate value in their constructors.
	m_insn_prefetch_depth = 3;

	std::fill_n(&m_insn_prefetch_buffer[0], 3, 0);
	std::fill_n(&m_insn_prefetch_address[0], 3, 0);
	std::fill_n(&m_insn_prefetch_valid[0], 3, false);
	m_insn_prefetch_count = 0;
	m_insn_prefetch_index = 0;
	m_tlb_log = 0;
	m_actual_log = 0;
}

arm7_cpu_device::~arm7_cpu_device()
{
}


arm7_be_cpu_device::arm7_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, ARM7_BE, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_BIG)
{
}


arm710a_cpu_device::arm710a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, ARM710A, tag, owner, clock, 4, ARCHFLAG_MODE26, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_ARM
			   | ARM9_COPRO_ID_ARCH_V4
			   | ARM9_COPRO_ID_PART_ARM710;
}


arm710t_cpu_device::arm710t_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, ARM710T, tag, owner, clock, 4, ARCHFLAG_MODE26, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_ARM
			   | ARM9_COPRO_ID_PART_ARM710
			   | 0x00800000;
}


arm7500_cpu_device::arm7500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, ARM7500, tag, owner, clock, 4, ARCHFLAG_MODE26, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_ARM
			   | ARM9_COPRO_ID_ARCH_V4
			   | ARM9_COPRO_ID_PART_ARM710;
}


arm9_cpu_device::arm9_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm9_cpu_device(mconfig, ARM9, tag, owner, clock, 5, ARCHFLAG_T | ARCHFLAG_E, ENDIANNESS_LITTLE)
{
}


arm9_cpu_device::arm9_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags, endianness_t endianness)
	: arm7_cpu_device(mconfig, type, tag, owner, clock, archRev, archFlags, endianness)
{
	uint32_t arch = ARM9_COPRO_ID_ARCH_V4;
	switch (archRev)
	{
		case 4:
			if (archFlags & ARCHFLAG_T)
				arch = ARM9_COPRO_ID_ARCH_V4T;
			break;
		case 5:
			arch = ARM9_COPRO_ID_ARCH_V5;
			if (archFlags & ARCHFLAG_T)
			{
				arch = ARM9_COPRO_ID_ARCH_V5T;
				if (archFlags & ARCHFLAG_E)
				{
					arch = ARM9_COPRO_ID_ARCH_V5TE;
				}
			}
			break;
		default: break;
	}

	m_copro_id = ARM9_COPRO_ID_MFR_ARM | arch | (0x900 << 4);
}


arm920t_cpu_device::arm920t_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm9_cpu_device(mconfig, ARM920T, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_ARM
			   | ARM9_COPRO_ID_SPEC_REV1
			   | ARM9_COPRO_ID_ARCH_V4T
			   | ARM9_COPRO_ID_PART_ARM920
			   | 0; // Stepping
}

arm946es_cpu_device::arm946es_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: arm9_cpu_device(mconfig, type, tag, owner, clock, 5, ARCHFLAG_T | ARCHFLAG_E, ENDIANNESS_LITTLE),
	cp15_control(0x78)
{
	m_copro_id = ARM9_COPRO_ID_MFR_ARM
		| ARM9_COPRO_ID_ARCH_V5TE
		| ARM9_COPRO_ID_PART_ARM946
		| ARM9_COPRO_ID_STEP_ARM946_A0;

	std::fill_n(&ITCM[0], 0x8000, 0);
	std::fill_n(&DTCM[0], 0x4000, 0);

	cp15_itcm_base = 0xffffffff;
	cp15_itcm_size = 0;
	cp15_itcm_end = 0;
	cp15_dtcm_base = 0xffffffff;
	cp15_dtcm_size = 0;
	cp15_dtcm_end = 0;
	cp15_itcm_reg = cp15_dtcm_reg = 0;
}

arm946es_cpu_device::arm946es_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm946es_cpu_device(mconfig, ARM946ES, tag, owner, clock)
{
}

arm11_cpu_device::arm11_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm11_cpu_device(mconfig, ARM11, tag, owner, clock, 6, ARCHFLAG_T | ARCHFLAG_E | ARCHFLAG_K, ENDIANNESS_LITTLE)
{
}


arm11_cpu_device::arm11_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags, endianness_t endianness)
	: arm9_cpu_device(mconfig, type, tag, owner, clock, archRev, archFlags, endianness)
{
	uint32_t arch = ARM9_COPRO_ID_ARCH_V6;

	m_copro_id = ARM9_COPRO_ID_MFR_ARM | arch | (0xB00 << 4);
}

arm1176jzf_s_cpu_device::arm1176jzf_s_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm11_cpu_device(mconfig, ARM1176JZF_S, tag, owner, clock, 6, ARCHFLAG_T | ARCHFLAG_E | ARCHFLAG_K, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_ARM
			   | ARM9_COPRO_ID_SPEC_REV0
			   | ARM9_COPRO_ID_ARCH_CPUID
			   | ARM9_COPRO_ID_PART_ARM1176JZF_S
			   | ARM9_COPRO_ID_STEP_ARM1176JZF_S_R0P7;
}

// unknown configuration, but uses MPU not MMU, so closer to ARM946ES
igs036_cpu_device::igs036_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm946es_cpu_device(mconfig, IGS036, tag, owner, clock)
{
}

pxa250_cpu_device::pxa250_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, PXA250, tag, owner, clock, 5, ARCHFLAG_T | ARCHFLAG_E | ARCHFLAG_XSCALE, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_INTEL
			   | ARM9_COPRO_ID_ARCH_V5TE
			   | ARM9_COPRO_ID_PART_PXA250
			   | ARM9_COPRO_ID_STEP_PXA255_A0;
}

pxa255_cpu_device::pxa255_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, PXA255, tag, owner, clock, 5, ARCHFLAG_T | ARCHFLAG_E | ARCHFLAG_XSCALE, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_INTEL
			   | ARM9_COPRO_ID_ARCH_V5TE
			   | ARM9_COPRO_ID_PART_PXA255
			   | ARM9_COPRO_ID_STEP_PXA255_A0;
}

pxa270_cpu_device::pxa270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, PXA270, tag, owner, clock, 5, ARCHFLAG_T | ARCHFLAG_E | ARCHFLAG_XSCALE, ENDIANNESS_LITTLE)
{
	m_copro_id = ARM9_COPRO_ID_MFR_INTEL
			   | ARM9_COPRO_ID_ARCH_V5TE
			   | ARM9_COPRO_ID_PART_PXA270
			   | ARM9_COPRO_ID_STEP_PXA255_A0;
}

sa1100_cpu_device::sa1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, SA1100, tag, owner, clock, 4, ARCHFLAG_SA, ENDIANNESS_LITTLE)
	// has StrongARM, no Thumb, no Enhanced DSP
{
	m_copro_id = ARM9_COPRO_ID_MFR_DEC
			   | ARM9_COPRO_ID_ARCH_V4
			   | ARM9_COPRO_ID_PART_SA1100
			   | ARM9_COPRO_ID_STEP_SA1100_A;
}

sa1110_cpu_device::sa1110_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, SA1110, tag, owner, clock, 4, ARCHFLAG_SA, ENDIANNESS_LITTLE)
	// has StrongARM, no Thumb, no Enhanced DSP
{
	m_copro_id = ARM9_COPRO_ID_MFR_INTEL
			   | ARM9_COPRO_ID_ARCH_V4
			   | ARM9_COPRO_ID_PART_SA1110
			   | ARM9_COPRO_ID_STEP_SA1110_B4;
}

device_memory_interface::space_config_vector arm7_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void arm7_cpu_device::update_reg_ptr()
{
	m_reg_group = sRegisterTable[GET_MODE];
}

void arm7_cpu_device::set_cpsr(uint32_t val)
{
	uint8_t old_mode = GET_CPSR & MODE_FLAG;
	bool call_hook = false;
	if (m_archFlags & ARCHFLAG_MODE26)
	{
		if ((val & 0x10) != (m_r[eCPSR] & 0x10))
		{
			if (val & 0x10)
			{
				// 26 -> 32
				val = (val & 0x0FFFFF3F) | (m_r[eR15] & 0xF0000000) /* N Z C V */ | ((m_r[eR15] & 0x0C000000) >> (26 - 6)) /* I F */;
				m_r[eR15] = m_r[eR15] & 0x03FFFFFC;
			}
			else
			{
				// 32 -> 26
				m_r[eR15] = (m_r[eR15] & 0x03FFFFFC) /* PC */ | (val & 0xF0000000) /* N Z C V */ | ((val & 0x000000C0) << (26 - 6)) /* I F */ | (val & 0x00000003) /* M1 M0 */;
			}
			call_hook = true;
		}
		else
		{
			if (!(val & 0x10))
			{
				// mirror bits in pc
				m_r[eR15] = (m_r[eR15] & 0x03FFFFFF) | (val & 0xF0000000) /* N Z C V */ | ((val & 0x000000C0) << (26 - 6)) /* I F */;
			}
		}
	}
	else
	{
		val |= 0x10; // force valid mode
	}
	if ((val & T_MASK) != (m_r[eCPSR] & T_MASK))
		call_hook = true;
	m_r[eCPSR] = val;
	if ((GET_CPSR & MODE_FLAG) != old_mode)
	{
		if ((GET_CPSR & MODE_FLAG) == eARM7_MODE_USER || old_mode == eARM7_MODE_USER)
			call_hook = true;
		update_reg_ptr();
	}
	if (call_hook)
		debugger_privilege_hook();
}


/**************************************************************************
 * ARM TLB IMPLEMENTATION
 **************************************************************************/

enum
{
	TLB_COARSE = 0,
	TLB_FINE
};

enum
{
	FAULT_NONE = 0,
	FAULT_DOMAIN,
	FAULT_PERMISSION
};


// COARSE, desc_level1, vaddr
uint32_t arm7_cpu_device::get_lvl2_desc_from_page_table( uint32_t granularity, uint32_t first_desc, uint32_t vaddr )
{
	uint32_t desc_lvl2 = vaddr;

	switch( granularity )
	{
		case TLB_COARSE:
			desc_lvl2 = (first_desc & COPRO_TLB_CFLD_ADDR_MASK) | ((vaddr & COPRO_TLB_VADDR_CSLTI_MASK) >> COPRO_TLB_VADDR_CSLTI_MASK_SHIFT);
			if (m_tlb_log)
				LOGMASKED(LOG_TLB, "%s: get_lvl2_desc_from_page_table: coarse descriptor, lvl2 address is %08x\n", machine().describe_context(), desc_lvl2);
			break;
		case TLB_FINE:
			desc_lvl2 = (first_desc & COPRO_TLB_FPTB_ADDR_MASK) | ((vaddr & COPRO_TLB_VADDR_FSLTI_MASK) >> COPRO_TLB_VADDR_FSLTI_MASK_SHIFT);
			if (m_tlb_log)
				LOGMASKED(LOG_TLB, "%s: get_lvl2_desc_from_page_table: fine descriptor, lvl2 address is %08x\n", machine().describe_context(), desc_lvl2);
			break;
		default:
			// We shouldn't be here
			LOGMASKED(LOG_MMU, "ARM7: Attempting to get second-level TLB descriptor of invalid granularity (%d)\n", granularity);
			break;
	}

	return m_program->read_dword( desc_lvl2 );
}


int arm7_cpu_device::detect_fault(int desc_lvl1, int ap, int flags)
{
	switch (m_decoded_access_control[(desc_lvl1 >> 5) & 0xf])
	{
		case 0 : // "No access - Any access generates a domain fault"
		{
			return FAULT_DOMAIN;
		}
		case 1 : // "Client - Accesses are checked against the access permission bits in the section or page descriptor"
		{
			if ((ap & 3) == 3)
			{
				return FAULT_NONE;
			}
			else if (ap & 2)
			{
				if (((m_r[eCPSR] & MODE_FLAG) == eARM7_MODE_USER) && (flags & ARM7_TLB_WRITE))
				{
					return FAULT_PERMISSION;
				}
			}
			else if (ap & 1)
			{
				if ((m_r[eCPSR] & MODE_FLAG) == eARM7_MODE_USER)
				{
					return FAULT_PERMISSION;
				}
			}
			else
			{
				int s = (m_control & COPRO_CTRL_SYSTEM) ? 1 : 0;
				int r = (m_control & COPRO_CTRL_ROM) ? 1 : 0;
				if (s == 0)
				{
					if (r == 0) // "Any access generates a permission fault"
					{
						return FAULT_PERMISSION;
					}
					else // "Any write generates a permission fault"
					{
						if (flags & ARM7_TLB_WRITE)
						{
							return FAULT_PERMISSION;
						}
					}
				}
				else
				{
					if (r == 0) // "Only Supervisor read permitted"
					{
						if (((m_r[eCPSR] & MODE_FLAG) == eARM7_MODE_USER) || (flags & ARM7_TLB_WRITE))
						{
							return FAULT_PERMISSION;
						}
					}
					else // "Reserved" -> assume same behaviour as S=0/R=0 case
					{
						return FAULT_PERMISSION;
					}
				}
			}
		}
		break;
		case 2 : // "Reserved - Reserved. Currently behaves like the no access mode"
		{
			return FAULT_DOMAIN;
		}
		case 3 : // "Manager - Accesses are not checked against the access permission bits so a permission fault cannot be generated"
		{
			return FAULT_NONE;
		}
	}
	return FAULT_NONE;
}

arm7_cpu_device::tlb_entry *arm7_cpu_device::tlb_map_entry(const offs_t vaddr, const int flags)
{
	const uint32_t section = (vaddr >> (COPRO_TLB_VADDR_FLTI_MASK_SHIFT + 2)) & 0xFFF;
	tlb_entry *entries = (flags & ARM7_TLB_ABORT_D) ? m_dtlb_entries : m_itlb_entries;
	const uint32_t start = section << 1;
	uint32_t index = (flags & ARM7_TLB_ABORT_D) ? m_dtlb_entry_index[section] : m_itlb_entry_index[section];

	bool entry_found = false;

	for (uint32_t i = 0; i < 2; i++)
	{
		index = (index + 1) & 1;
		if (!entries[start + index].valid)
		{
			entry_found = true;
			break;
		}
	}

	if (!entry_found)
	{
		index = (index + 1) & 1;
	}

	if (flags & ARM7_TLB_ABORT_D)
		m_dtlb_entry_index[section] = index;
	else
		m_itlb_entry_index[section] = index;

	return &entries[start + index];
}

arm7_cpu_device::tlb_entry *arm7_cpu_device::tlb_probe(const offs_t vaddr, const int flags)
{
	const uint32_t section = (vaddr >> (COPRO_TLB_VADDR_FLTI_MASK_SHIFT + 2)) & 0xFFF;
	tlb_entry *entries = (flags & ARM7_TLB_ABORT_D) ? m_dtlb_entries : m_itlb_entries;
	const uint32_t start = section << 1;
	uint32_t index = (flags & ARM7_TLB_ABORT_D) ? m_dtlb_entry_index[section] : m_itlb_entry_index[section];

	if (m_tlb_log)
		LOGMASKED(LOG_TLB, "%s: tlb_probe: vaddr %08x, section %02x, start %02x, index %d\n", machine().describe_context(), vaddr, section, start, index);

	for (uint32_t i = 0; i < 2; i++)
	{
		uint32_t position = start + index;
		if (entries[position].valid)
		{
			switch (entries[position].type)
			{
			case COPRO_TLB_TYPE_SECTION:
				if (entries[position].table_bits == (vaddr & COPRO_TLB_STABLE_MASK))
					return &entries[position];
				break;
			case COPRO_TLB_TYPE_LARGE:
			case COPRO_TLB_TYPE_SMALL:
				if (entries[position].table_bits == (vaddr & COPRO_TLB_LSTABLE_MASK))
					return &entries[position];
				break;
			case COPRO_TLB_TYPE_TINY:
				if (entries[position].table_bits == (vaddr & COPRO_TLB_TTABLE_MASK))
					return &entries[position];
				break;
			}
		}
		if (m_tlb_log)
		{
			LOGMASKED(LOG_TLB, "%s: tlb_probe: skipped due to mismatch (valid %d, domain %02x, access %d, table_bits %08x, base_addr %08x, type %d\n",
				machine().describe_context(), entries[position].valid ? 1 : 0, entries[position].domain, entries[position].access,
				entries[position].table_bits, entries[position].base_addr, entries[position].type);
		}

		index = (index - 1) & 1;
	}

	return nullptr;
}

uint32_t arm7_cpu_device::get_fault_from_permissions(const uint8_t access, const uint8_t domain, const uint8_t type, int flags)
{
	const uint8_t domain_bits = m_decoded_access_control[domain];
	switch (domain_bits)
	{
		case COPRO_DOMAIN_NO_ACCESS:
			if (type == COPRO_TLB_TYPE_SECTION)
				return (domain << 4) | COPRO_FAULT_DOMAIN_SECTION;
			return (domain << 4) | COPRO_FAULT_DOMAIN_PAGE;
		case COPRO_DOMAIN_CLIENT:
		{
			const uint32_t mode = GET_CPSR & 0xF;
			switch (access)
			{
				case 0: // Check System/ROM bit
				{
					const uint32_t sr = (COPRO_CTRL >> COPRO_CTRL_SYSTEM_SHIFT) & 3;
					switch (sr)
					{
						case 0: // No Access
							if (type == COPRO_TLB_TYPE_SECTION)
								return (domain << 4) | COPRO_FAULT_PERM_SECTION;
							return (domain << 4) | COPRO_FAULT_PERM_PAGE;
						case 1: // No User Access, Read-Only System Access
							if (mode == 0 || (flags & ARM7_TLB_WRITE))
							{
								if (type == COPRO_TLB_TYPE_SECTION)
									return (domain << 4) | COPRO_FAULT_PERM_SECTION;
								return (domain << 4) | COPRO_FAULT_PERM_PAGE;
							}
							return COPRO_FAULT_NONE;
						case 2: // Read-Only Access
							if (flags & ARM7_TLB_WRITE)
							{
								if (type == COPRO_TLB_TYPE_SECTION)
									return (domain << 4) | COPRO_FAULT_PERM_SECTION;
								return (domain << 4) | COPRO_FAULT_PERM_PAGE;
							}
							return COPRO_FAULT_NONE;
						case 3: // Unpredictable Access
							LOGMASKED(LOG_MMU, "%s: get_fault_from_permissions: Unpredictable access permissions (AP bits are 0, SR bits are 3).", machine().describe_context());
							return COPRO_FAULT_NONE;
					}
					return COPRO_FAULT_NONE;
				}
				case 1: // No User Access
					if (mode != 0)
						return COPRO_FAULT_NONE;
					if (type == COPRO_TLB_TYPE_SECTION)
						return (domain << 4) | COPRO_FAULT_PERM_SECTION;
					return (domain << 4) | COPRO_FAULT_PERM_PAGE;
				case 2: // Read-Only User Access
					if (mode != 0 || (flags & ARM7_TLB_READ))
						return COPRO_FAULT_NONE;
					if (type == COPRO_TLB_TYPE_SECTION)
						return (domain << 4) | COPRO_FAULT_PERM_SECTION;
					return (domain << 4) | COPRO_FAULT_PERM_PAGE;
				case 3: // Full Access
					return COPRO_FAULT_NONE;
			}
			return COPRO_FAULT_NONE;
		}
		case COPRO_DOMAIN_RESV:
			LOGMASKED(LOG_MMU, "%s: get_fault_from_permissions: Domain type marked as Reserved.\n", machine().describe_context());
			return COPRO_FAULT_NONE;
		default:
			return COPRO_FAULT_NONE;
	}
}

uint32_t arm7_cpu_device::tlb_check_permissions(tlb_entry *entry, const int flags)
{
	return get_fault_from_permissions(entry->access, entry->domain, entry->type, flags);
}

offs_t arm7_cpu_device::tlb_translate(tlb_entry *entry, const offs_t vaddr)
{
	switch (entry->type)
	{
		case COPRO_TLB_TYPE_SECTION:
			return entry->base_addr | (vaddr & ~COPRO_TLB_SECTION_PAGE_MASK);
		case COPRO_TLB_TYPE_LARGE:
			return entry->base_addr | (vaddr & ~COPRO_TLB_LARGE_PAGE_MASK);
		case COPRO_TLB_TYPE_SMALL:
			return entry->base_addr | (vaddr & ~COPRO_TLB_SMALL_PAGE_MASK);
		case COPRO_TLB_TYPE_TINY:
			return entry->base_addr | (vaddr & ~COPRO_TLB_TINY_PAGE_MASK);
		default:
			return 0;
	}
}

bool arm7_cpu_device::page_table_finish_translation(offs_t &vaddr, const uint8_t type, const uint32_t lvl1, const uint32_t lvl2, const int flags, const uint32_t lvl1a, const uint32_t lvl2a)
{
	const uint8_t domain = (uint8_t)(lvl1 >> 5) & 0xF;
	uint8_t access = 0;
	uint32_t table_bits = 0;
	switch (type)
	{
		case COPRO_TLB_TYPE_SECTION:
			access = (uint8_t)((lvl2 >> 10) & 3);
			table_bits = vaddr & COPRO_TLB_STABLE_MASK;
			break;
		case COPRO_TLB_TYPE_LARGE:
		{
			const uint8_t subpage_shift = 4 + (uint8_t)((vaddr >> 13) & 6);
			access = (uint8_t)((lvl2 >> subpage_shift) & 3);
			table_bits = vaddr & COPRO_TLB_LSTABLE_MASK;
			break;
		}

		case COPRO_TLB_TYPE_SMALL:
		{
			const uint8_t subpage_shift = 4 + (uint8_t)((vaddr >> 9) & 6);
			access = (uint8_t)((lvl2 >> subpage_shift) & 3);
			table_bits = vaddr & COPRO_TLB_LSTABLE_MASK;
			break;
		}

		case COPRO_TLB_TYPE_TINY:
			access = (uint8_t)((lvl2 >> 4) & 3);
			table_bits = vaddr & COPRO_TLB_TTABLE_MASK;
			break;
	}

	const uint32_t access_result = get_fault_from_permissions(access, domain, type, flags);
	if (access_result != 0)
	{
		if (flags & ARM7_TLB_ABORT_P)
		{
			LOGMASKED(LOG_MMU, "ARM7: Page walk, Potential prefetch abort, vaddr = %08x, lvl1A = %08x, lvl1D = %08x, lvl2A = %08x, lvl2D = %08x\n", vaddr, lvl1a, lvl1, lvl2a, lvl2);
		}
		else if (flags & ARM7_TLB_ABORT_D)
		{
			LOGMASKED(LOG_MMU, "ARM7: Page walk, Data abort, vaddr = %08x, lvl1A = %08x, lvl1D = %08x, lvl2A = %08x, lvl2D = %08x\n", vaddr, lvl1a, lvl1, lvl2a, lvl2);
			LOGMASKED(LOG_MMU, "access: %d, domain: %d, type: %d\n", access, domain, type);
			m_faultStatus[0] = access_result;
			m_faultAddress = vaddr;
			m_pendingAbtD = true;
			update_irq_state();
		}
		return false;
	}

	static const uint32_t s_page_masks[4] = { COPRO_TLB_SECTION_PAGE_MASK, COPRO_TLB_LARGE_PAGE_MASK, COPRO_TLB_SMALL_PAGE_MASK, COPRO_TLB_TINY_PAGE_MASK };
	const uint32_t base_addr = lvl2 & s_page_masks[type];
	const uint32_t paddr = base_addr | (vaddr & ~s_page_masks[type]);

	if (flags)
	{
		tlb_entry *entry = tlb_map_entry(vaddr, flags);

		entry->valid = true;
		entry->domain = domain;
		entry->access = access;
		entry->table_bits = table_bits;
		entry->base_addr = base_addr;
		entry->type = type;
	}

	vaddr = paddr;
	return true;
}

bool arm7_cpu_device::page_table_translate(offs_t &vaddr, const int flags)
{
	const uint32_t lvl1_addr = m_tlb_base_mask | ((vaddr & COPRO_TLB_VADDR_FLTI_MASK) >> COPRO_TLB_VADDR_FLTI_MASK_SHIFT);
	const uint32_t lvl1_desc = m_program->read_dword(lvl1_addr);

	LOGMASKED(LOG_MMU, "ARM7: Translating page table entry for %08x, lvl1_addr %08x, lvl1_desc %08x\n", vaddr, lvl1_addr, lvl1_desc);

	switch (lvl1_desc & 3)
	{
		case 0: // Unmapped
			LOGMASKED(LOG_MMU, "ARM7: Translating page table entry for %08x, Unmapped, lvl1a %08x, lvl1d %08x\n", vaddr, lvl1_addr, lvl1_desc);
			if (flags & ARM7_TLB_ABORT_D)
			{
				LOGMASKED(LOG_MMU, "ARM7: Page Table Translation failed (D), PC %08x, lvl1 unmapped, vaddr = %08x, lvl1A = %08x, lvl1D = %08x\n", m_r[eR15], vaddr, lvl1_addr, lvl1_desc);
				m_faultStatus[0] = COPRO_FAULT_TRANSLATE_SECTION;
				m_faultAddress = vaddr;
				m_pendingAbtD = true;
				update_irq_state();
			}
			else if (flags & ARM7_TLB_ABORT_P)
			{
				LOGMASKED(LOG_MMU, "ARM7: Page Table Translation failed (P), PC %08x, lvl1 unmapped, vaddr = %08x, lvl1A = %08x, lvl1D = %08x\n", m_r[eR15], vaddr, lvl1_addr, lvl1_desc);
			}
			return false;

		case 1: // Coarse Table
		{
			const uint32_t lvl2_addr = (lvl1_desc & COPRO_TLB_CFLD_ADDR_MASK) | ((vaddr & COPRO_TLB_VADDR_CSLTI_MASK) >> COPRO_TLB_VADDR_CSLTI_MASK_SHIFT);
			const uint32_t lvl2_desc = m_program->read_dword(lvl2_addr);

			LOGMASKED(LOG_MMU, "ARM7: Translating page table entry for %08x, Coarse, lvl1a %08x, lvl1d %08x, lvl2a %08x, lvl2d %08x\n", vaddr, lvl1_addr, lvl1_desc, lvl2_addr, lvl2_desc);

			switch (lvl2_desc & 3)
			{
				case 0: // Unmapped
					if (flags & ARM7_TLB_ABORT_D)
					{
						LOGMASKED(LOG_MMU, "ARM7: Page Table Translation failed (D), coarse lvl2 unmapped, PC %08x, vaddr = %08x, lvl1A = %08x, lvl1D = %08x, lvl2A = %08x, lvl2D = %08x\n", m_r[eR15], vaddr, lvl1_addr, lvl1_desc, lvl2_addr, lvl2_desc);
						m_faultStatus[0] = ((lvl1_desc >> 1) & 0xF0) | COPRO_FAULT_TRANSLATE_PAGE;
						m_faultAddress = vaddr;
						m_pendingAbtD = true;
						update_irq_state();
					}
					else if (flags & ARM7_TLB_ABORT_P)
					{
						LOGMASKED(LOG_MMU, "ARM7: Page Table Translation failed (P), coarse lvl2 unmapped, PC %08x, vaddr = %08x, lvl1A = %08x, lvl1D = %08x, lvl2A = %08x, lvl2D = %08x\n", m_r[eR15], vaddr, lvl1_addr, lvl1_desc, lvl2_addr, lvl2_desc);
					}
					return false;

				case 1: // Large Page
					return page_table_finish_translation(vaddr, COPRO_TLB_TYPE_LARGE, lvl1_desc, lvl2_desc, flags, lvl1_addr, lvl2_addr);

				case 2: // Small Page
					return page_table_finish_translation(vaddr, COPRO_TLB_TYPE_SMALL, lvl1_desc, lvl2_desc, flags, lvl1_addr, lvl2_addr);

				case 3: // Tiny Page (invalid)
					LOGMASKED(LOG_MMU, "ARM7: Page Table Translation failed, tiny page present in coarse lvl2 table, PC %08x, vaddr = %08x, lvl1A = %08x, lvl1D = %08x, lvl2A = %08x, lvl2D = %08x\n", m_r[eR15], vaddr, lvl1_addr, lvl1_desc, lvl2_addr, lvl2_desc);
					return false;
			}
			return false;
		}

		case 2: // Section Descriptor
			LOGMASKED(LOG_MMU, "ARM7: Translating page table entry for %08x, Section, lvl1a %08x, lvl1d %08x\n", vaddr, lvl1_addr, lvl1_desc);
			return page_table_finish_translation(vaddr, COPRO_TLB_TYPE_SECTION, lvl1_desc, lvl1_desc, flags, lvl1_addr, lvl1_addr);

		case 3: // Fine Table
		{
			const uint32_t lvl2_addr = (lvl1_desc & COPRO_TLB_FPTB_ADDR_MASK) | ((vaddr & COPRO_TLB_VADDR_FSLTI_MASK) >> COPRO_TLB_VADDR_FSLTI_MASK_SHIFT);
			const uint32_t lvl2_desc = m_program->read_dword(lvl2_addr);

			LOGMASKED(LOG_MMU, "ARM7: Translating page table entry for %08x, Fine, lvl1a %08x, lvl1d %08x, lvl2a %08x, lvl2d %08x\n", vaddr, lvl1_addr, lvl1_desc, lvl2_addr, lvl2_desc);

			switch (lvl2_desc & 3)
			{
				case 0: // Unmapped
					if (flags & ARM7_TLB_ABORT_D)
					{
						LOGMASKED(LOG_MMU, "ARM7: Page Table Translation failed (D), fine lvl2 unmapped, PC %08x, vaddr = %08x, lvl1A = %08x, lvl1D = %08x, lvl2A = %08x, lvl2D = %08x\n", m_r[eR15], vaddr, lvl1_addr, lvl1_desc, lvl2_addr, lvl2_desc);
						m_faultStatus[0] = ((lvl1_desc >> 1) & 0xF0) | COPRO_FAULT_TRANSLATE_PAGE;
						m_faultAddress = vaddr;
						m_pendingAbtD = true;
						update_irq_state();
					}
					else if (flags & ARM7_TLB_ABORT_P)
					{
						LOGMASKED(LOG_MMU, "ARM7: Page Table Translation failed (P), fine lvl2 unmapped, PC %08x, vaddr = %08x, lvl1A = %08x, lvl1D = %08x, lvl2A = %08x, lvl2D = %08x\n", m_r[eR15], vaddr, lvl1_addr, lvl1_desc, lvl2_addr, lvl2_desc);
					}
					return false;

				case 1: // Large Page
					return page_table_finish_translation(vaddr, COPRO_TLB_TYPE_LARGE, lvl1_desc, lvl2_desc, flags, lvl1_addr, lvl2_addr);

				case 2: // Small Page
					return page_table_finish_translation(vaddr, COPRO_TLB_TYPE_SMALL, lvl1_desc, lvl2_desc, flags, lvl1_addr, lvl2_addr);

				case 3: // Tiny Page
					return page_table_finish_translation(vaddr, COPRO_TLB_TYPE_TINY, lvl1_desc, lvl2_desc, flags, lvl1_addr, lvl2_addr);
			}
			return false;
		}
	}

	return false;
}

bool arm7_cpu_device::translate_vaddr_to_paddr(offs_t &vaddr, const int flags)
{
	if (m_tlb_log)
		LOGMASKED(LOG_TLB, "%s: translate_vaddr_to_paddr: vaddr %08x, flags %08x\n", machine().describe_context(), vaddr, flags);

	if (vaddr < 0x2000000)
	{
		vaddr += m_pid_offset;
		if (m_tlb_log)
			LOGMASKED(LOG_TLB, "%s: translate_vaddr_to_paddr: vaddr < 32M, adding PID (%08x) = %08x\n", machine().describe_context(), m_pid_offset, vaddr);
	}

	tlb_entry *entry = tlb_probe(vaddr, flags);

	if (entry)
	{
		if (m_tlb_log)
		{
			LOGMASKED(LOG_TLB, "%s: translate_vaddr_to_paddr: found entry (domain %02x, access %d, table_bits %08x, base_addr %08x, type %d\n",
				machine().describe_context(), entry->domain, entry->access, entry->table_bits, entry->base_addr, entry->type);
		}

		const uint32_t access_result = tlb_check_permissions(entry, flags);
		if (access_result == 0)
		{
			vaddr = tlb_translate(entry, vaddr);
			return true;
		}
		else if (flags & ARM7_TLB_ABORT_P)
		{
			LOGMASKED(LOG_MMU, "ARM7: TLB, Potential prefetch abort, vaddr = %08x\n", vaddr);
		}
		else if (flags & ARM7_TLB_ABORT_D)
		{
			LOGMASKED(LOG_MMU, "ARM7: TLB, Data abort, vaddr = %08x\n", vaddr);
			m_faultStatus[0] = access_result;
			m_faultAddress = vaddr;
			m_pendingAbtD = true;
			update_irq_state();
		}
		return false;
	}
	else
	{
		if (m_tlb_log)
			LOGMASKED(LOG_MMU, "No TLB entry for %08x yet, running page_table_translate\n", vaddr);
		return page_table_translate(vaddr, flags);
	}
}

void arm7_cpu_device::translate_insn_command(const std::vector<std::string_view> &params)
{
	translate_command(params, TR_FETCH);
}

void arm7_cpu_device::translate_data_command(const std::vector<std::string_view> &params)
{
	translate_command(params, TR_READ);
}

void arm7_cpu_device::translate_command(const std::vector<std::string_view> &params, int intention)
{
	uint64_t vaddr;

	if (!machine().debugger().console().validate_number_parameter(params[0], vaddr)) return;

	vaddr &= 0xffffffff;

	offs_t paddr = (offs_t)vaddr;
	address_space *space = nullptr;
	bool can_translate = memory_translate(AS_PROGRAM, intention, paddr, space);
	if (can_translate)
		machine().debugger().console().printf("%s vaddr %08x => phys %08x\n", intention == TR_FETCH ? "instruction" : "data", (uint32_t)vaddr, paddr);
	else
		machine().debugger().console().printf("%s vaddr %08x => unmapped\n", intention == TR_FETCH ? "instruction" : "data");
}

bool arm7_cpu_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);
	/* only applies to the program address space and only does something if the MMU's enabled */
	if (spacenum == AS_PROGRAM && (m_control & COPRO_CTRL_MMU_EN))
	{
		const int flags = intention == TR_FETCH ? ARM7_TLB_ABORT_P : ARM7_TLB_ABORT_D;
		if (address < 0x2000000)
			address += m_pid_offset;

		tlb_entry *entry = tlb_probe(address, flags);

		if (entry)
		{
			const uint32_t access_result = tlb_check_permissions(entry, flags);
			if (access_result == 0)
			{
				address = tlb_translate(entry, address);
				return true;
			}
			return false;
		}
		else
		{
			return page_table_translate(address, 0);
		}
	}
	return true;
}


/* include the arm7 core */
#include "arm7core.hxx"

/***************************************************************************
 * CPU SPECIFIC IMPLEMENTATIONS
 **************************************************************************/

void arm7_cpu_device::postload()
{
	update_reg_ptr();
}

void arm7_cpu_device::device_start()
{
	init_ce_kernel_addrs();

	m_program = &space(AS_PROGRAM);

	if(m_program->endianness() == ENDIANNESS_LITTLE) {
		m_program->cache(m_cachele);
		m_pr32 = [this](offs_t address) -> u32 { return m_cachele.read_dword(address); };
		m_prptr = [this](offs_t address) -> const void * { return m_cachele.read_ptr(address); };
	} else {
		m_program->cache(m_cachebe);
		m_pr32 = [this](offs_t address) -> u32 { return m_cachebe.read_dword(address); };
		m_prptr = [this](offs_t address) -> const void * { return m_cachebe.read_ptr(address); };
	}

	save_item(NAME(m_insn_prefetch_depth));
	save_item(NAME(m_insn_prefetch_count));
	save_item(NAME(m_insn_prefetch_index));
	save_item(NAME(m_insn_prefetch_buffer));
	save_item(NAME(m_insn_prefetch_address));
	save_item(NAME(m_insn_prefetch_valid));
	save_item(NAME(m_tlb_log));
	save_item(NAME(m_actual_log));
	save_item(NAME(m_r));
	save_item(NAME(m_pendingIrq));
	save_item(NAME(m_pendingFiq));
	save_item(NAME(m_pendingAbtD));
	save_item(NAME(m_pendingAbtP));
	save_item(NAME(m_pendingUnd));
	save_item(NAME(m_pendingSwi));
	save_item(NAME(m_pending_interrupt));
	save_item(NAME(m_control));
	save_item(NAME(m_tlbBase));
	save_item(NAME(m_tlb_base_mask));
	save_item(NAME(m_faultStatus));
	save_item(NAME(m_faultAddress));
	save_item(NAME(m_fcsePID));
	save_item(NAME(m_pid_offset));
	save_item(NAME(m_domainAccessControl));
	save_item(NAME(m_decoded_access_control));
	save_item(STRUCT_MEMBER(m_dtlb_entries, valid));
	save_item(STRUCT_MEMBER(m_dtlb_entries, domain));
	save_item(STRUCT_MEMBER(m_dtlb_entries, access));
	save_item(STRUCT_MEMBER(m_dtlb_entries, table_bits));
	save_item(STRUCT_MEMBER(m_dtlb_entries, base_addr));
	save_item(STRUCT_MEMBER(m_dtlb_entries, type));
	save_item(STRUCT_MEMBER(m_itlb_entries, valid));
	save_item(STRUCT_MEMBER(m_itlb_entries, domain));
	save_item(STRUCT_MEMBER(m_itlb_entries, access));
	save_item(STRUCT_MEMBER(m_itlb_entries, table_bits));
	save_item(STRUCT_MEMBER(m_itlb_entries, base_addr));
	save_item(STRUCT_MEMBER(m_itlb_entries, type));
	save_item(NAME(m_dtlb_entry_index));
	save_item(NAME(m_itlb_entry_index));
	machine().save().register_postload(save_prepost_delegate(FUNC(arm7_cpu_device::postload), this));

	set_icountptr(m_icount);

	state_add( ARM7_PC,    "PC", m_pc).callexport().formatstr("%08X");
	state_add(STATE_GENPC, "GENPC", m_pc).callexport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).callexport().noshow();
	/* registers shared by all operating modes */
	state_add( ARM7_R0,    "R0",   m_r[ 0]).formatstr("%08X");
	state_add( ARM7_R1,    "R1",   m_r[ 1]).formatstr("%08X");
	state_add( ARM7_R2,    "R2",   m_r[ 2]).formatstr("%08X");
	state_add( ARM7_R3,    "R3",   m_r[ 3]).formatstr("%08X");
	state_add( ARM7_R4,    "R4",   m_r[ 4]).formatstr("%08X");
	state_add( ARM7_R5,    "R5",   m_r[ 5]).formatstr("%08X");
	state_add( ARM7_R6,    "R6",   m_r[ 6]).formatstr("%08X");
	state_add( ARM7_R7,    "R7",   m_r[ 7]).formatstr("%08X");
	state_add( ARM7_R8,    "R8",   m_r[ 8]).formatstr("%08X");
	state_add( ARM7_R9,    "R9",   m_r[ 9]).formatstr("%08X");
	state_add( ARM7_R10,   "R10",  m_r[10]).formatstr("%08X");
	state_add( ARM7_R11,   "R11",  m_r[11]).formatstr("%08X");
	state_add( ARM7_R12,   "R12",  m_r[12]).formatstr("%08X");
	state_add( ARM7_R13,   "R13",  m_r[13]).formatstr("%08X");
	state_add( ARM7_R14,   "R14",  m_r[14]).formatstr("%08X");
	state_add( ARM7_R15,   "R15",  m_r[15]).formatstr("%08X");
	/* Current Status Program Register */
	state_add( ARM7_CPSR,  "CPSR", m_r[eCPSR]).formatstr("%08X");
	/* FIRQ Mode Shadowed Registers */
	state_add( ARM7_FR8,   "FR8",  m_r[eR8_FIQ]  ).formatstr("%08X");
	state_add( ARM7_FR9,   "FR9",  m_r[eR9_FIQ]  ).formatstr("%08X");
	state_add( ARM7_FR10,  "FR10", m_r[eR10_FIQ] ).formatstr("%08X");
	state_add( ARM7_FR11,  "FR11", m_r[eR11_FIQ] ).formatstr("%08X");
	state_add( ARM7_FR12,  "FR12", m_r[eR12_FIQ] ).formatstr("%08X");
	state_add( ARM7_FR13,  "FR13", m_r[eR13_FIQ] ).formatstr("%08X");
	state_add( ARM7_FR14,  "FR14", m_r[eR14_FIQ] ).formatstr("%08X");
	state_add( ARM7_FSPSR, "FR16", m_r[eSPSR_FIQ]).formatstr("%08X");
	/* IRQ Mode Shadowed Registers */
	state_add( ARM7_IR13,  "IR13", m_r[eR13_IRQ] ).formatstr("%08X");
	state_add( ARM7_IR14,  "IR14", m_r[eR14_IRQ] ).formatstr("%08X");
	state_add( ARM7_ISPSR, "IR16", m_r[eSPSR_IRQ]).formatstr("%08X");
	/* Supervisor Mode Shadowed Registers */
	state_add( ARM7_SR13,  "SR13", m_r[eR13_SVC] ).formatstr("%08X");
	state_add( ARM7_SR14,  "SR14", m_r[eR14_SVC] ).formatstr("%08X");
	state_add( ARM7_SSPSR, "SR16", m_r[eSPSR_SVC]).formatstr("%08X");
	/* Abort Mode Shadowed Registers */
	state_add( ARM7_AR13,  "AR13", m_r[eR13_ABT] ).formatstr("%08X");
	state_add( ARM7_AR14,  "AR14", m_r[eR14_ABT] ).formatstr("%08X");
	state_add( ARM7_ASPSR, "AR16", m_r[eSPSR_ABT]).formatstr("%08X");
	/* Undefined Mode Shadowed Registers */
	state_add( ARM7_UR13,  "UR13", m_r[eR13_UND] ).formatstr("%08X");
	state_add( ARM7_UR14,  "UR14", m_r[eR14_UND] ).formatstr("%08X");
	state_add( ARM7_USPSR, "UR16", m_r[eSPSR_UND]).formatstr("%08X");
	state_add( ARM7_LOGTLB, "LOGTLB", m_actual_log).formatstr("%01X");

	state_add(STATE_GENFLAGS, "GENFLAGS", m_r[eCPSR]).formatstr("%13s").noshow();

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("translate_insn", CMDFLAG_NONE, 1, 1, std::bind(&arm7_cpu_device::translate_insn_command, this, _1));
		machine().debugger().console().register_command("translate_data", CMDFLAG_NONE, 1, 1, std::bind(&arm7_cpu_device::translate_data_command, this, _1));
	}
}


void arm946es_cpu_device::device_start()
{
	arm9_cpu_device::device_start();

	save_item(NAME(cp15_control));
	save_item(NAME(cp15_itcm_base));
	save_item(NAME(cp15_dtcm_base));
	save_item(NAME(cp15_itcm_size));
	save_item(NAME(cp15_dtcm_size));
	save_item(NAME(cp15_itcm_end));
	save_item(NAME(cp15_dtcm_end));
	save_item(NAME(cp15_itcm_reg));
	save_item(NAME(cp15_dtcm_reg));
	save_item(NAME(ITCM));
	save_item(NAME(DTCM));
}

void arm7_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case STATE_GENPCBASE:
			m_pc = GET_PC;
			break;
	}
}


void arm7_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c %s",
				(ARM7REG(eCPSR) & N_MASK) ? 'N' : '-',
				(ARM7REG(eCPSR) & Z_MASK) ? 'Z' : '-',
				(ARM7REG(eCPSR) & C_MASK) ? 'C' : '-',
				(ARM7REG(eCPSR) & V_MASK) ? 'V' : '-',
				(ARM7REG(eCPSR) & Q_MASK) ? 'Q' : '-',
				(ARM7REG(eCPSR) & I_MASK) ? 'I' : '-',
				(ARM7REG(eCPSR) & F_MASK) ? 'F' : '-',
				(ARM7REG(eCPSR) & T_MASK) ? 'T' : '-',
				GetModeText(ARM7REG(eCPSR)));
		break;
	}
}

void arm7_cpu_device::device_reset()
{
	std::fill(std::begin(m_r), std::end(m_r), 0);
	m_pendingIrq = false;
	m_pendingFiq = false;
	m_pendingAbtD = false;
	m_pendingAbtP = false;
	m_pendingUnd = false;
	m_pendingSwi = false;
	m_pending_interrupt = false;
	m_control = 0;
	m_tlbBase = 0;
	m_tlb_base_mask = 0;
	m_faultStatus[0] = 0;
	m_faultStatus[1] = 0;
	m_faultAddress = 0;
	m_fcsePID = 0;
	m_pid_offset = 0;
	m_domainAccessControl = 0;
	std::fill_n(&m_decoded_access_control[0], 16, 0);

	/* start up in SVC mode with interrupts disabled. */
	m_r[eCPSR] = I_MASK | F_MASK | 0x10;
	SwitchMode(eARM7_MODE_SVC);
	m_r[eR15] = 0 | m_vectorbase;

	m_impstate.cache_dirty = true;

	for (auto &entry : m_dtlb_entries)
	{
		entry.valid = false;
		entry.domain = 0;
		entry.access = 0;
		entry.table_bits = 0;
		entry.base_addr = 0;
		entry.type = 0;
	}
	for (auto &entry : m_itlb_entries)
	{
		entry.valid = false;
		entry.domain = 0;
		entry.access = 0;
		entry.table_bits = 0;
		entry.base_addr = 0;
		entry.type = 0;
	}
	std::fill(std::begin(m_dtlb_entry_index), std::end(m_dtlb_entry_index), 0);
	std::fill(std::begin(m_itlb_entry_index), std::end(m_itlb_entry_index), 0);
}

void arm1176jzf_s_cpu_device::device_reset()
{
	arm7_cpu_device::device_reset();
	m_control = 0x00050078;
}

void arm7_cpu_device::update_insn_prefetch(uint32_t curr_pc)
{
	curr_pc &= ~3;
	if (m_insn_prefetch_address[m_insn_prefetch_index] != curr_pc)
	{
		LOGMASKED(LOG_PREFETCH, "Prefetch addr %08x doesn't match curr_pc %08x, flushing prefetch buffer\n", m_insn_prefetch_address[m_insn_prefetch_index], curr_pc);
		m_insn_prefetch_count = 0;
		m_insn_prefetch_index = 0;
	}

	if (m_insn_prefetch_count == m_insn_prefetch_depth)
	{
		LOGMASKED(LOG_PREFETCH, "We have prefetched up to the max depth, bailing\n");
		return;
	}

	const uint32_t to_fetch = m_insn_prefetch_depth - m_insn_prefetch_count;
	const uint32_t start_index = (m_insn_prefetch_depth + (m_insn_prefetch_index - to_fetch)) % m_insn_prefetch_depth;
	//printf("need to prefetch %d instructions starting at index %d\n", to_fetch, start_index);

	LOGMASKED(LOG_PREFETCH, "Need to fetch %d entries starting from index %d\n", to_fetch, start_index);
	uint32_t pc = curr_pc + m_insn_prefetch_count * 4;
	for (uint32_t i = 0; i < to_fetch; i++)
	{
		uint32_t index = (i + start_index) % m_insn_prefetch_depth;
		LOGMASKED(LOG_PREFETCH, "About to get prefetch index %d from addr %08x\n", index, pc);
		m_insn_prefetch_valid[index] = true;
		offs_t physical_pc = pc;
		if ((m_control & COPRO_CTRL_MMU_EN) && !translate_vaddr_to_paddr(physical_pc, ARM7_TLB_ABORT_P | ARM7_TLB_READ))
		{
			LOGMASKED(LOG_PREFETCH, "Unable to fetch, bailing\n");
			m_insn_prefetch_valid[index] = false;
			break;
		}
		uint32_t op = m_pr32(physical_pc);
		LOGMASKED(LOG_PREFETCH, "Got op %08x\n", op);
		//printf("ipb[%d] <- %08x(%08x)\n", index, op, pc);
		m_insn_prefetch_buffer[index] = op;
		m_insn_prefetch_address[index] = pc;
		m_insn_prefetch_count++;
		pc += 4;
	}
}

bool arm7_cpu_device::insn_fetch_thumb(uint32_t pc, uint32_t &out_insn)
{
	if (pc & 2)
	{
		out_insn = (uint16_t)(m_insn_prefetch_buffer[m_insn_prefetch_index] >> m_prefetch_word1_shift);
		bool valid = m_insn_prefetch_valid[m_insn_prefetch_index];
		m_insn_prefetch_index = (m_insn_prefetch_index + 1) % m_insn_prefetch_depth;
		m_insn_prefetch_count--;
		return valid;
	}
	out_insn = (uint16_t)(m_insn_prefetch_buffer[m_insn_prefetch_index] >> m_prefetch_word0_shift);
	return m_insn_prefetch_valid[m_insn_prefetch_index];
}

bool arm7_cpu_device::insn_fetch_arm(uint32_t pc, uint32_t &out_insn)
{
	//printf("ipb[%d] = %08x\n", m_insn_prefetch_index, m_insn_prefetch_buffer[m_insn_prefetch_index]);
	out_insn = m_insn_prefetch_buffer[m_insn_prefetch_index];
	bool valid = m_insn_prefetch_valid[m_insn_prefetch_index];
	LOGMASKED(LOG_PREFETCH, "Fetched op %08x for PC %08x with %s entry from %08x\n", out_insn, pc, valid ? "valid" : "invalid", m_insn_prefetch_address[m_insn_prefetch_index]);
	m_insn_prefetch_index = (m_insn_prefetch_index + 1) % m_insn_prefetch_depth;
	m_insn_prefetch_count--;
	return valid;
}

void arm7_cpu_device::add_ce_kernel_addr(offs_t addr, std::string value)
{
	m_ce_kernel_addrs[addr - 0xf0000000] = value;
	m_ce_kernel_addr_present[addr - 0xf0000000] = true;
}

#include "cecalls.hxx"

void arm7_cpu_device::execute_run()
{
	auto const UNEXECUTED = [this] { m_r[eR15] += 4; m_icount += 2; }; // Any unexecuted instruction only takes 1 cycle (page 193)

	m_tlb_log = m_actual_log;

	uint32_t insn;

	do
	{
		uint32_t pc = GET_PC;

#if PRINT_HAPYFSH2
		if (pc == 0xC0047374)
		{
			char substr_buf[4096];
			uint16_t substr_idx = 0;
			bool zero_prepend = false;
			uint8_t digit_count = 0;
			uint32_t string_addr = (m_r[eR0] & 0x0fffffff) | 0x30000000;
			uint8_t charval = 0;
			uint32_t reg_idx = eR1;
			do
			{
				charval = m_program->read_byte(string_addr);
				string_addr++;
				if (charval >= 0x20 && charval < 0x7f)
				{
					if (charval == '%')
					{
						bool still_processing = true;
						uint8_t nextval = m_program->read_byte(string_addr);
						string_addr++;
						switch (nextval)
						{
						case 0:
							printf("%%");
							charval = 0;
							still_processing = false;
							break;
						case '%':
							printf("%%");
							still_processing = false;
							break;
						case '0':
							zero_prepend = true;
							break;
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							digit_count = nextval - '0';
							break;
						case 'd':
							printf("%d", (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx]);
							still_processing = false;
							reg_idx++;
							break;
						case 'l':
						case 'u':
							printf("%u", (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx]);
							still_processing = false;
							reg_idx++;
							break;
						case 'x':
						case 'p':
							printf("%x", (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx]);
							still_processing = false;
							reg_idx++;
							break;
						case 'o':
							printf("%o", (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx]);
							still_processing = false;
							reg_idx++;
							break;
						case 's':
						{
							uint32_t val = (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx];
							uint32_t substring_addr = (val & 0x0fffffff) | 0x30000000;
							reg_idx++;
							bool end_found = false;
							while (!end_found)
							{
								substr_buf[substr_idx] = m_program->read_byte(substring_addr);
								if (substr_buf[substr_idx] == 0)
								{
									end_found = true;
								}
								substring_addr++;
								substr_idx++;
							}
							substr_idx = 0;
							printf("%s", substr_buf);
							still_processing = false;
							break;
						}
						}
						while (still_processing)
						{
							uint8_t nextval2 = m_program->read_byte(string_addr);
							string_addr++;
							if (nextval2 == 0)
							{
								printf("%c%c", (char)charval, (char)nextval);
								charval = 0;
								break;
							}
							else if (nextval2 >= '1' && nextval2 <= '9')
							{
								digit_count = nextval2 - '0';
							}
							else if (nextval2 == 'd')
							{
								uint32_t val = (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx];
								switch (digit_count)
								{
								case 1: if (zero_prepend) { printf("%01d", val); } else { printf("%1d", val); } break;
								case 2: if (zero_prepend) { printf("%02d", val); } else { printf("%2d", val); } break;
								case 3: if (zero_prepend) { printf("%03d", val); } else { printf("%3d", val); } break;
								case 4: if (zero_prepend) { printf("%04d", val); } else { printf("%4d", val); } break;
								case 5: if (zero_prepend) { printf("%05d", val); } else { printf("%5d", val); } break;
								case 6: if (zero_prepend) { printf("%06d", val); } else { printf("%6d", val); } break;
								case 7: if (zero_prepend) { printf("%07d", val); } else { printf("%7d", val); } break;
								case 8: if (zero_prepend) { printf("%08d", val); } else { printf("%8d", val); } break;
								case 9: if (zero_prepend) { printf("%09d", val); } else { printf("%9d", val); } break;
								}
								reg_idx++;
								still_processing = false;
							}
							else if (nextval2 == 'u' || nextval2 == 'l')
							{
								uint32_t val = (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx];
								switch (digit_count)
								{
								case 1: if (zero_prepend) { printf("%01u", val); } else { printf("%1u", val); } break;
								case 2: if (zero_prepend) { printf("%02u", val); } else { printf("%2u", val); } break;
								case 3: if (zero_prepend) { printf("%03u", val); } else { printf("%3u", val); } break;
								case 4: if (zero_prepend) { printf("%04u", val); } else { printf("%4u", val); } break;
								case 5: if (zero_prepend) { printf("%05u", val); } else { printf("%5u", val); } break;
								case 6: if (zero_prepend) { printf("%06u", val); } else { printf("%6u", val); } break;
								case 7: if (zero_prepend) { printf("%07u", val); } else { printf("%7u", val); } break;
								case 8: if (zero_prepend) { printf("%08u", val); } else { printf("%8u", val); } break;
								case 9: if (zero_prepend) { printf("%09u", val); } else { printf("%9u", val); } break;
								}
								reg_idx++;
								still_processing = false;
							}
							else if (nextval2 == 'x')
							{
								uint32_t val = (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx];
								switch (digit_count)
								{
								case 1: if (zero_prepend) { printf("%01x", val); } else { printf("%1x", val); } break;
								case 2: if (zero_prepend) { printf("%02x", val); } else { printf("%2x", val); } break;
								case 3: if (zero_prepend) { printf("%03x", val); } else { printf("%3x", val); } break;
								case 4: if (zero_prepend) { printf("%04x", val); } else { printf("%4x", val); } break;
								case 5: if (zero_prepend) { printf("%05x", val); } else { printf("%5x", val); } break;
								case 6: if (zero_prepend) { printf("%06x", val); } else { printf("%6x", val); } break;
								case 7: if (zero_prepend) { printf("%07x", val); } else { printf("%7x", val); } break;
								case 8: if (zero_prepend) { printf("%08x", val); } else { printf("%8x", val); } break;
								case 9: if (zero_prepend) { printf("%09x", val); } else { printf("%9x", val); } break;
								}
								reg_idx++;
								still_processing = false;
							}
							else if (nextval2 == 'o')
							{
								uint32_t val = (reg_idx >= eR4) ? m_program->read_dword(((m_r[eR13_SVC] & 0x0fffffff) | 0x30000000) + (reg_idx - eR4)) : m_r[reg_idx];
								switch (digit_count)
								{
								case 1: if (zero_prepend) { printf("%01o", val); } else { printf("%1o", val); } break;
								case 2: if (zero_prepend) { printf("%02o", val); } else { printf("%2o", val); } break;
								case 3: if (zero_prepend) { printf("%03o", val); } else { printf("%3o", val); } break;
								case 4: if (zero_prepend) { printf("%04o", val); } else { printf("%4o", val); } break;
								case 5: if (zero_prepend) { printf("%05o", val); } else { printf("%5o", val); } break;
								case 6: if (zero_prepend) { printf("%06o", val); } else { printf("%6o", val); } break;
								case 7: if (zero_prepend) { printf("%07o", val); } else { printf("%7o", val); } break;
								case 8: if (zero_prepend) { printf("%08o", val); } else { printf("%8o", val); } break;
								case 9: if (zero_prepend) { printf("%09o", val); } else { printf("%9o", val); } break;
								}
								reg_idx++;
								still_processing = false;
							}
							else
							{
								printf("%c%c", (char)charval, (char)nextval);
								still_processing = false;
							}
						}
					}
					else
					{
						printf("%c", (char)charval);
					}
				}
			} while (charval != 0);
			printf("\n");
		}
#endif
		update_insn_prefetch(pc);

		m_tlb_log = 0;
		debugger_instruction_hook(pc);
		m_tlb_log = m_actual_log;

		/* handle Thumb instructions if active */
		if (T_IS_SET(m_r[eCPSR]))
		{
			pc = m_r[eR15];

			// "In Thumb state, bit [0] is undefined and must be ignored. Bits [31:1] contain the PC."
			offs_t const raddr = pc & ~uint32_t(1);

			if (!insn_fetch_thumb(raddr, insn))
			{
				m_pendingAbtP = true;
				update_irq_state();
				goto skip_exec;
			}
			(this->*thumb_handler[(insn & 0xffc0) >> 6])(pc, insn);
		}
		else
		{
			/* load 32 bit instruction */

			// "In ARM state, bits [1:0] of r15 are undefined and must be ignored. Bits [31:2] contain the PC."
			offs_t const raddr = pc & ~uint32_t(3);

			if (!insn_fetch_arm(raddr, insn))
			{
#if PRINT_CE_KERNEL
				if (raddr >= 0xf0000000)
				{
					print_ce_kernel_address(raddr - 0xf0000000);
				}
#endif
				m_pendingAbtP = true;
				update_irq_state();
				goto skip_exec;
			}

			int op_offset = 0;
			/* process condition codes for this instruction */
			if ((insn >> INSN_COND_SHIFT) != COND_AL)
			{
				switch (insn >> INSN_COND_SHIFT)
				{
					case COND_EQ:
						if (Z_IS_CLEAR(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_NE:
						if (Z_IS_SET(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_CS:
						if (C_IS_CLEAR(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_CC:
						if (C_IS_SET(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_MI:
						if (N_IS_CLEAR(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_PL:
						if (N_IS_SET(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_VS:
						if (V_IS_CLEAR(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_VC:
						if (V_IS_SET(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_HI:
						if (C_IS_CLEAR(m_r[eCPSR]) || Z_IS_SET(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_LS:
						if (C_IS_SET(m_r[eCPSR]) && Z_IS_CLEAR(m_r[eCPSR]))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_GE:
						if (!(m_r[eCPSR] & N_MASK) != !(m_r[eCPSR] & V_MASK)) /* Use x ^ (x >> ...) method */
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_LT:
						if (!(m_r[eCPSR] & N_MASK) == !(m_r[eCPSR] & V_MASK))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_GT:
						if (Z_IS_SET(m_r[eCPSR]) || (!(m_r[eCPSR] & N_MASK) != !(m_r[eCPSR] & V_MASK)))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_LE:
						if (Z_IS_CLEAR(m_r[eCPSR]) && (!(m_r[eCPSR] & N_MASK) == !(m_r[eCPSR] & V_MASK)))
							{ UNEXECUTED();  goto skip_exec; }
						break;
					case COND_NV:
						if (m_archRev < 5)
						  { UNEXECUTED();  goto skip_exec; }
						else
							op_offset = 0x10;
						break;
				}
			}
			/*******************************************************************/
			/* If we got here - condition satisfied, so decode the instruction */
			/*******************************************************************/
			(this->*ops_handler[((insn & 0xF000000) >> 24) + op_offset])(insn);
		}

skip_exec:

		arm7_check_irq_state();

		/* All instructions remove 3 cycles.. Others taking less / more will have adjusted this # prior to here */
		m_icount -= 3;
	} while (m_icount > 0);

	m_tlb_log = 0;
}


void arm7_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case ARM7_IRQ_LINE: // IRQ
		m_pendingIrq = state != 0;
		break;

	case ARM7_FIRQ_LINE: // FIQ
		m_pendingFiq = state != 0;
		break;

	case ARM7_ABORT_EXCEPTION:
		m_pendingAbtD = state != 0;
		break;
	case ARM7_ABORT_PREFETCH_EXCEPTION:
		m_pendingAbtP = state != 0;
		break;

	case ARM7_UNDEFINE_EXCEPTION:
		m_pendingUnd = state != 0;
		break;
	}

	update_irq_state();
	arm7_check_irq_state();
}


void arm7_cpu_device::set_irq(int state)
{
	assert((machine().scheduler().currently_executing() == static_cast<device_execute_interface *>(this)) || !machine().scheduler().currently_executing());
	m_pendingIrq = state != 0;
	update_irq_state();
	arm7_check_irq_state();
}


void arm7_cpu_device::set_fiq(int state)
{
	assert((machine().scheduler().currently_executing() == static_cast<device_execute_interface *>(this)) || !machine().scheduler().currently_executing());
	m_pendingFiq = state != 0;
	update_irq_state();
	arm7_check_irq_state();
}


std::unique_ptr<util::disasm_interface> arm7_cpu_device::create_disassembler()
{
	return std::make_unique<arm7_disassembler>(this);
}

bool arm7_cpu_device::get_t_flag() const
{
	return T_IS_SET(m_r[eCPSR]);
}


/* ARM system coprocessor support */

void arm7_cpu_device::arm7_do_callback(uint32_t data)
{
	m_pendingUnd = true;
	update_irq_state();
}

uint32_t arm7_cpu_device::arm7_rt_r_callback(offs_t offset)
{
	uint32_t opcode = offset;
	uint8_t cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
	uint8_t op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
	uint8_t op3 =    opcode & INSN_COPRO_OP3;
	uint8_t cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;
	uint32_t data = 0;

//    printf("cpnum %d cReg %d op2 %d op3 %d (%x)\n", cpnum, cReg, op2, op3, GET_REGISTER(arm, 15));

	// we only handle system copro here
	if (cpnum != 15)
	{
		if (m_archFlags & ARCHFLAG_XSCALE)
		{
			// handle XScale specific CP14
			if (cpnum == 14)
			{
				switch( cReg )
				{
					case 1: // clock counter
						data = (uint32_t)total_cycles();
						break;

					default:
						break;
				}
			}
			else
			{
				fatalerror("XScale: Unhandled coprocessor %d (archFlags %x)\n", cpnum, m_archFlags);
			}

			return data;
		}
		else
		{
			LOGMASKED(LOG_COPRO_UNKNOWN, "ARM7: Unhandled coprocessor %d (archFlags %x)\n", cpnum, m_archFlags);
			m_pendingUnd = true;
			update_irq_state();
			return 0;
		}
	}

	switch( cReg )
	{
		case 4:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			// RESERVED
			LOGMASKED(LOG_COPRO_RESERVED, "arm7_rt_r_callback CR%d, RESERVED\n", cReg);
			break;
		case 0:             // ID
			switch(op2)
			{
			case 0:
				data = m_copro_id;
				break;
			case 1: // cache type
				data = 0x0f0d2112;  // HACK: value expected by ARMWrestler (probably Nintendo DS ARM9's value)
				//data = (6 << 25) | (1 << 24) | (0x172 << 12) | (0x172 << 0); // ARM920T (S3C24xx)
				break;
			case 2: // TCM type
				data = 0;
				break;
			case 3: // TLB type
				data = 0;
				break;
			case 4: // MPU type
				data = 0;
				break;
			}
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, ID %02x (%02x) -> %08x (PC=%08x)\n",op2,m_archRev,data,GET_PC);
			break;
		case 1:             // Control
			data = COPRO_CTRL | 0x70;   // bits 4-6 always read back as "1" (bit 3 too in XScale)
			break;
		case 2:             // Translation Table Base
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, TLB Base, PC = %08x\n", m_r[eR15]);
			data = COPRO_TLB_BASE;
			break;
		case 3:             // Domain Access Control
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, Domain Access Control, PC = %08x\n", m_r[eR15]);
			data = COPRO_DOMAIN_ACCESS_CONTROL;
			break;
		case 5:             // Fault Status
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, Fault Status, PC = %08x, op3 %d, FSR0 = %08x, FSR1 = %08x\n", m_r[eR15], op3, COPRO_FAULT_STATUS_D, COPRO_FAULT_STATUS_P);
			switch (op3)
			{
				case 0: data = COPRO_FAULT_STATUS_D; break;
				case 1: data = COPRO_FAULT_STATUS_P; break;
			}
			break;
		case 6:             // Fault Address
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, Fault Address, PC = %08x, FAR = %08x\n", m_r[eR15], COPRO_FAULT_ADDRESS);
			data = COPRO_FAULT_ADDRESS;
			break;
		case 13:            // Read Process ID (PID)
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, Read PID, PC = %08x\n", m_r[eR15]);
			data = COPRO_FCSE_PID;
			break;
		case 14:            // Read Breakpoint
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, Read Breakpoint\n");
			break;
		case 15:            // Test, Clock, Idle
			LOGMASKED(LOG_COPRO_READS, "arm7_rt_r_callback, Test / Clock / Idle \n");
			break;
	}

	return data;
}

void arm7_cpu_device::arm7_rt_w_callback(offs_t offset, uint32_t data)
{
	uint32_t opcode = offset;
	uint8_t cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
	uint8_t op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
	uint8_t op3 =    opcode & INSN_COPRO_OP3;
	uint8_t cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;

	// handle XScale specific CP14 - just eat writes for now
	if (cpnum != 15)
	{
		if (cpnum == 14)
		{
			LOGMASKED(LOG_COPRO_UNKNOWN, "arm7_rt_w_callback: write %x to XScale CP14 reg %d\n", data, cReg);
			return;
		}
		else
		{
			LOGMASKED(LOG_COPRO_UNKNOWN, "ARM7: Unhandled coprocessor %d\n", cpnum);
			m_pendingUnd = true;
			update_irq_state();
			return;
		}
	}

	switch (cReg)
	{
		case 0:
		case 4:
		case 10:
		case 11:
		case 12:
			// RESERVED
			LOGMASKED(LOG_COPRO_RESERVED, "arm7_rt_w_callback CR%d, RESERVED = %08x\n", cReg, data);
			break;
		case 1:             // Control
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Control = %08x (%d) (%d)\n", data, op2, op3);
			LOGMASKED(LOG_COPRO_WRITES, "    MMU:%d, Address Fault:%d, Data Cache:%d, Write Buffer:%d\n",
					data & COPRO_CTRL_MMU_EN, ( data & COPRO_CTRL_ADDRFAULT_EN ) >> COPRO_CTRL_ADDRFAULT_EN_SHIFT,
					( data & COPRO_CTRL_DCACHE_EN ) >> COPRO_CTRL_DCACHE_EN_SHIFT,
					( data & COPRO_CTRL_WRITEBUF_EN ) >> COPRO_CTRL_WRITEBUF_EN_SHIFT);
			LOGMASKED(LOG_COPRO_WRITES, "    Endianness:%d, System:%d, ROM:%d, Instruction Cache:%d\n",
					( data & COPRO_CTRL_ENDIAN ) >> COPRO_CTRL_ENDIAN_SHIFT,
					( data & COPRO_CTRL_SYSTEM ) >> COPRO_CTRL_SYSTEM_SHIFT,
					( data & COPRO_CTRL_ROM ) >> COPRO_CTRL_ROM_SHIFT,
					( data & COPRO_CTRL_ICACHE_EN ) >> COPRO_CTRL_ICACHE_EN_SHIFT);
			LOGMASKED(LOG_COPRO_WRITES, "    Int Vector Adjust:%d\n", ( data & COPRO_CTRL_INTVEC_ADJUST ) >> COPRO_CTRL_INTVEC_ADJUST_SHIFT);
#if ARM7_MMU_ENABLE_HACK
			if (((data & COPRO_CTRL_MMU_EN) != 0) && ((COPRO_CTRL & COPRO_CTRL_MMU_EN) == 0))
			{
				m_mmu_enable_addr = R15;
			}
			if (((data & COPRO_CTRL_MMU_EN) == 0) && ((COPRO_CTRL & COPRO_CTRL_MMU_EN) != 0))
			{
				if (!translate_vaddr_to_paddr( R15, 0))
				{
					fatalerror("ARM7_MMU_ENABLE_HACK translate failed\n");
				}
			}
#endif
			COPRO_CTRL = data & COPRO_CTRL_MASK;
			break;
		case 2:             // Translation Table Base
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback TLB Base = %08x (%d) (%d), PC = %08x\n", data, op2, op3, m_r[eR15]);
			COPRO_TLB_BASE = data;
			m_tlb_base_mask = data & COPRO_TLB_BASE_MASK;
			break;
		case 3:             // Domain Access Control
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Domain Access Control = %08x (%d) (%d), PC = %08x\n", data, op2, op3, m_r[eR15]);
			COPRO_DOMAIN_ACCESS_CONTROL = data;
			for (int i = 0; i < 32; i += 2)
			{
				m_decoded_access_control[i >> 1] = (COPRO_DOMAIN_ACCESS_CONTROL >> i) & 3;
			}
			break;
		case 5:             // Fault Status
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Fault Status = %08x (%d) (%d), PC = %08x\n", data, op2, op3, m_r[eR15]);
			switch (op3)
			{
				case 0: COPRO_FAULT_STATUS_D = data; break;
				case 1: COPRO_FAULT_STATUS_P = data; break;
			}
			break;
		case 6:             // Fault Address
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Fault Address = %08x (%d) (%d), PC = %08x\n", data, op2, op3, m_r[eR15]);
			COPRO_FAULT_ADDRESS = data;
			break;
		case 7:             // Cache Operations
//            LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Cache Ops = %08x (%d) (%d)\n", data, op2, op3);
			break;
		case 8:             // TLB Operations
			LOGMASKED(LOG_COPRO_WRITES, "%s: arm7_rt_w_callback TLB Ops = %08x (%d) (%d), PC = %08x\n", machine().describe_context(), data, op2, op3, m_r[eR15]);
			switch (op2)
			{
				case 0:
					switch (op3)
					{
						case 5:
							// Flush I
							for (uint32_t i = 0; i < std::size(m_itlb_entries); i++)
							{
								m_itlb_entries[i].valid = false;
							}
							break;
						case 6:
							// Flush D
							for (uint32_t i = 0; i < std::size(m_dtlb_entries); i++)
							{
								m_dtlb_entries[i].valid = false;
							}
							break;
						case 7:
							// Flush I+D
							for (uint32_t i = 0; i < std::size(m_dtlb_entries); i++)
							{
								m_dtlb_entries[i].valid = false;
								m_itlb_entries[i].valid = false;
							}
							break;
						default:
							LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Unsupported TLB Op\n");
							break;
					}
					break;
				case 1:
					switch (op3)
					{
						case 5:
						{
							// Flush I single entry
							tlb_entry *entry = tlb_probe(data, ARM7_TLB_ABORT_P);
							if (entry)
							{
								LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback TLB Ops: Successfully flushed I entry for %08x\n", data);
								entry->valid = false;
							}
							break;
						}

						case 6:
						{
							// Flush D single entry
							tlb_entry *entry = tlb_probe(data, ARM7_TLB_ABORT_D);
							if (entry)
							{
								LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback TLB Ops: Successfully flushed D entry for %08x\n", data);
								entry->valid = false;
							}
							break;
						}
						case 7:
						{
							// Flush unified single entry
							tlb_entry *entry = tlb_probe(data, ARM7_TLB_ABORT_D);
							if (entry)
							{
								LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback TLB Ops: Successfully flushed D entry for %08x\n", data);
								entry->valid = false;
							}
							entry = tlb_probe(data, ARM7_TLB_ABORT_P);
							if (entry)
							{
								LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback TLB Ops: Successfully flushed I entry for %08x\n", data);
								entry->valid = false;
							}
							break;
						}
						default:
							LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Unsupported TLB Op\n");
							break;
					}
					break;
			}
			break;
		case 9:             // Read Buffer Operations
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Read Buffer Ops = %08x (%d) (%d), PC = %08x\n", data, op2, op3, m_r[eR15]);
			break;
		case 13:            // Write Process ID (PID)
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Write PID = %08x (%d) (%d), PC = %08x\n", data, op2, op3, m_r[eR15]);
			COPRO_FCSE_PID = data;
			m_pid_offset = (((COPRO_FCSE_PID >> 25) & 0x7F)) * 0x2000000;
			break;
		case 14:            // Write Breakpoint
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Write Breakpoint = %08x (%d) (%d)\n", data, op2, op3);
			break;
		case 15:            // Test, Clock, Idle
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Test / Clock / Idle = %08x (%d) (%d)\n", data, op2, op3);
			break;
	}
}

uint32_t arm946es_cpu_device::arm7_rt_r_callback(offs_t offset)
{
	uint32_t opcode = offset;
	uint8_t cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
	uint8_t op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
	uint8_t op3 =    opcode & INSN_COPRO_OP3;
	uint8_t cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;
	uint32_t data = 0;

	//printf("arm7946: read cpnum %d cReg %d op2 %d op3 %d (%x)\n", cpnum, cReg, op2, op3, opcode);
	if (cpnum == 15)
	{
		switch( cReg )
		{
			case 0:
				switch (op2)
				{
					case 0: // chip ID
						data = 0x41059461;
						break;

					case 1: // cache ID
						data = 0x0f0d2112;
						break;

					case 2: // TCM size
						data = (6 << 6) | (5 << 18);
						break;
				}
				break;

			case 1:
				return cp15_control;
				break;

			case 9:
				if (op3 == 1)
				{
					if (op2 == 0)
					{
						return cp15_dtcm_reg;
					}
					else
					{
						return cp15_itcm_reg;
					}
				}
				break;
		}
	}

	return data;
}

void arm946es_cpu_device::arm7_rt_w_callback(offs_t offset, uint32_t data)
{
	uint32_t opcode = offset;
	uint8_t cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
	uint8_t op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
	uint8_t op3 =    opcode & INSN_COPRO_OP3;
	uint8_t cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;

//  printf("arm7946: copro %d write %x to cReg %d op2 %d op3 %d (mask %08x)\n", cpnum, data, cReg, op2, op3, mem_mask);

	if (cpnum == 15)
	{
		switch (cReg)
		{
			case 1: // control
				cp15_control = data;
				RefreshDTCM();
				RefreshITCM();
				break;

			case 2: // Protection Unit cacheability bits
				break;

			case 3: // write bufferability bits for PU
				break;

			case 5: // protection unit region controls
				break;

			case 6: // protection unit region controls 2
				break;

			case 7: // cache commands
				break;

			case 9: // cache lockdown & TCM controls
				if (op3 == 1)
				{
					if (op2 == 0)
					{
						cp15_dtcm_reg = data;
						RefreshDTCM();
					}
					else if (op2 == 1)
					{
						cp15_itcm_reg = data;
						RefreshITCM();
					}
				}
				break;
		}
	}
}

void arm946es_cpu_device::RefreshDTCM()
{
	if (cp15_control & (1<<16))
	{
		cp15_dtcm_base = (cp15_dtcm_reg & ~0xfff);
		cp15_dtcm_size = 512 << ((cp15_dtcm_reg & 0x3f) >> 1);
		cp15_dtcm_end = cp15_dtcm_base + cp15_dtcm_size;
		//printf("DTCM enabled: base %08x size %x\n", cp15_dtcm_base, cp15_dtcm_size);
	}
	else
	{
		cp15_dtcm_base = 0xffffffff;
		cp15_dtcm_size = cp15_dtcm_end = 0;
	}
}

void arm946es_cpu_device::RefreshITCM()
{
	if (cp15_control & (1<<18))
	{
		cp15_itcm_base = 0; //(cp15_itcm_reg & ~0xfff);
		cp15_itcm_size = 512 << ((cp15_itcm_reg & 0x3f) >> 1);
		cp15_itcm_end = cp15_itcm_base + cp15_itcm_size;
		//printf("ITCM enabled: base %08x size %x\n", cp15_dtcm_base, cp15_dtcm_size);
	}
	else
	{
		cp15_itcm_base = 0xffffffff;
		cp15_itcm_size = cp15_itcm_end = 0;
	}
}

void arm946es_cpu_device::arm7_cpu_write32(uint32_t addr, uint32_t data)
{
	addr &= ~3;

	if ((addr >= cp15_itcm_base) && (addr <= cp15_itcm_end))
	{
		uint32_t *wp = (uint32_t *)&ITCM[addr&0x7fff];
		*wp = data;
		return;
	}
	else if ((addr >= cp15_dtcm_base) && (addr <= cp15_dtcm_end))
	{
		uint32_t *wp = (uint32_t *)&DTCM[addr&0x3fff];
		*wp = data;
		return;
	}

	m_program->write_dword(addr, data);
}


void arm946es_cpu_device::arm7_cpu_write16(uint32_t addr, uint16_t data)
{
	addr &= ~1;
	if ((addr >= cp15_itcm_base) && (addr <= cp15_itcm_end))
	{
		uint16_t *wp = (uint16_t *)&ITCM[addr&0x7fff];
		*wp = data;
		return;
	}
	else if ((addr >= cp15_dtcm_base) && (addr <= cp15_dtcm_end))
	{
		uint16_t *wp = (uint16_t *)&DTCM[addr&0x3fff];
		*wp = data;
		return;
	}

	m_program->write_word(addr, data);
}

void arm946es_cpu_device::arm7_cpu_write8(uint32_t addr, uint8_t data)
{
	if ((addr >= cp15_itcm_base) && (addr <= cp15_itcm_end))
	{
		ITCM[addr&0x7fff] = data;
		return;
	}
	else if ((addr >= cp15_dtcm_base) && (addr <= cp15_dtcm_end))
	{
		DTCM[addr&0x3fff] = data;
		return;
	}

	m_program->write_byte(addr, data);
}

uint32_t arm946es_cpu_device::arm7_cpu_read32(uint32_t addr)
{
	uint32_t result;

	if ((addr >= cp15_itcm_base) && (addr <= cp15_itcm_end))
	{
		if (addr & 3)
		{
			uint32_t *wp = (uint32_t *)&ITCM[(addr & ~3)&0x7fff];
			result = rotr_32(*wp, 8 * (addr & 3));
		}
		else
		{
			uint32_t *wp = (uint32_t *)&ITCM[addr&0x7fff];
			result = *wp;
		}
	}
	else if ((addr >= cp15_dtcm_base) && (addr <= cp15_dtcm_end))
	{
		if (addr & 3)
		{
			uint32_t *wp = (uint32_t *)&DTCM[(addr & ~3)&0x3fff];
			result = rotr_32(*wp, 8 * (addr & 3));
		}
		else
		{
			uint32_t *wp = (uint32_t *)&DTCM[addr&0x3fff];
			result = *wp;
		}
	}
	else
	{
		if (addr & 3)
		{
			result = rotr_32(m_program->read_dword(addr & ~3), 8 * (addr & 3));
		}
		else
		{
			result = m_program->read_dword(addr);
		}
	}
	return result;
}

uint32_t arm946es_cpu_device::arm7_cpu_read16(uint32_t addr)
{
	addr &= ~1;

	if ((addr >= cp15_itcm_base) && (addr <= cp15_itcm_end))
	{
		uint16_t *wp = (uint16_t *)&ITCM[addr & 0x7fff];
		return *wp;
	}
	else if ((addr >= cp15_dtcm_base) && (addr <= cp15_dtcm_end))
	{
		uint16_t *wp = (uint16_t *)&DTCM[addr &0x3fff];
		return *wp;
	}

	return m_program->read_word(addr);
}

uint8_t arm946es_cpu_device::arm7_cpu_read8(uint32_t addr)
{
	if ((addr >= cp15_itcm_base) && (addr <= cp15_itcm_end))
	{
		return ITCM[addr & 0x7fff];
	}
	else if ((addr >= cp15_dtcm_base) && (addr <= cp15_dtcm_end))
	{
		return DTCM[addr & 0x3fff];
	}

	// Handle through normal 8 bit handler (for 32 bit cpu)
	return m_program->read_byte(addr);
}

void arm7_cpu_device::arm7_dt_r_callback(uint32_t insn, uint32_t *prn)
{
	uint8_t cpn = (insn >> 8) & 0xF;
	if ((m_archFlags & ARCHFLAG_XSCALE) && (cpn == 0))
	{
		LOGMASKED(LOG_DSP, "arm7_dt_r_callback: DSP Coprocessor 0 (CP0) not yet emulated (PC %08x)\n", GET_PC);
	}
	else
	{
		m_pendingUnd = true;
		update_irq_state();
	}
}


void arm7_cpu_device::arm7_dt_w_callback(uint32_t insn, uint32_t *prn)
{
	uint8_t cpn = (insn >> 8) & 0xF;
	if ((m_archFlags & ARCHFLAG_XSCALE) && (cpn == 0))
	{
		LOGMASKED(LOG_DSP, "arm7_dt_w_callback: DSP Coprocessor 0 (CP0) not yet emulated (PC %08x)\n", GET_PC);
	}
	else
	{
		m_pendingUnd = true;
		update_irq_state();
	}
}

uint32_t arm1176jzf_s_cpu_device::arm7_rt_r_callback(offs_t offset)
{
	uint32_t opcode = offset;
	uint8_t crn = (opcode & INSN_COPRO_CREG) >> INSN_COPRO_CREG_SHIFT;
	uint8_t op1 = (opcode & INSN_COPRO_OP1) >> INSN_COPRO_OP1_SHIFT;
	uint8_t op2 = (opcode & INSN_COPRO_OP2)  >> INSN_COPRO_OP2_SHIFT;
	uint8_t crm =  opcode & INSN_COPRO_OP3;
	uint8_t cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;
	uint32_t data = 0;

//  printf("arm7946: copro %d write %x to cReg %d op2 %d op3 %d (mask %08x)\n", cpnum, data, cReg, op2, op3, mem_mask);

	if (cpnum == 15)
	{
		if(crn == 0 && op1 == 0 && crm == 0 && op2 == 0) data = 0x410FB767; //ARM1176JZF-S Main ID.
		if(crn == 1 && op1 == 0 && crm == 0 && op2 == 0) data = m_control;
	}

	return data;
}

void arm1176jzf_s_cpu_device::arm7_rt_w_callback(offs_t offset, uint32_t data)
{
	uint32_t opcode = offset;
	uint8_t crn = (opcode & INSN_COPRO_CREG) >> INSN_COPRO_CREG_SHIFT;
	uint8_t op1 = (opcode & INSN_COPRO_OP1) >> INSN_COPRO_OP1_SHIFT;
	uint8_t op2 = (opcode & INSN_COPRO_OP2)  >> INSN_COPRO_OP2_SHIFT;
	uint8_t crm =  opcode & INSN_COPRO_OP3;
	uint8_t cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;

//  printf("arm7946: copro %d write %x to cReg %d op2 %d op3 %d (mask %08x)\n", cpnum, data, cReg, op2, op3, mem_mask);

	if (cpnum == 15)
	{
		LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback: CP15 CRn %02x Op1 %02x CRm %02x Op2 %02x data %08x\n", crn, op1, crm, op2, data);
		if(crn == 1 && op1 == 0 && crm == 0 && op2 == 0) m_control = data;
	}
}

/***************************************************************************
 * Default Memory Handlers
 ***************************************************************************/
void arm7_cpu_device::arm7_cpu_write32(uint32_t addr, uint32_t data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!translate_vaddr_to_paddr( addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	addr &= ~3;
	m_program->write_dword(addr, data);
}


void arm7_cpu_device::arm7_cpu_write16(uint32_t addr, uint16_t data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!translate_vaddr_to_paddr( addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	addr &= ~1;
	m_program->write_word(addr, data);
}

void arm7_cpu_device::arm7_cpu_write8(uint32_t addr, uint8_t data)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!translate_vaddr_to_paddr( addr, ARM7_TLB_ABORT_D | ARM7_TLB_WRITE ))
		{
			return;
		}
	}

	m_program->write_byte(addr, data);
}

uint32_t arm7_cpu_device::arm7_cpu_read32(uint32_t addr)
{
	uint32_t result;

	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!translate_vaddr_to_paddr( addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	if (addr & 3)
	{
		result = rotr_32(m_program->read_dword(addr & ~3), 8 * (addr & 3));
	}
	else
	{
		result = m_program->read_dword(addr);
	}

	return result;
}

uint32_t arm7_cpu_device::arm7_cpu_read16(uint32_t addr)
{
	uint32_t result;

	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!translate_vaddr_to_paddr( addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	result = m_program->read_word(addr & ~1);

	if (addr & 1)
	{
		result = ((result >> 8) & 0xff) | ((result & 0xff) << 24);
	}

	return result;
}

uint8_t arm7_cpu_device::arm7_cpu_read8(uint32_t addr)
{
	if( COPRO_CTRL & COPRO_CTRL_MMU_EN )
	{
		if (!translate_vaddr_to_paddr( addr, ARM7_TLB_ABORT_D | ARM7_TLB_READ ))
		{
			return 0;
		}
	}

	// Handle through normal 8 bit handler (for 32 bit cpu)
	return m_program->read_byte(addr);
}

#include "arm7drc.hxx"
