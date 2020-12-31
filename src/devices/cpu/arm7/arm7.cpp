// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7.c
 *   Portable CPU Emulator for 32-bit ARM v3/4/5/6
 *
 *   Copyright Steve Ellenoff, all rights reserved.
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
#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"
#include "arm7.h"
#include "arm7core.h"   //include arm7 core
#include "arm7help.h"

#define LOG_MMU             (1 << 0)
#define LOG_DSP             (1 << 1)
#define LOG_COPRO_READS     (1 << 2)
#define LOG_COPRO_WRITES    (1 << 3)
#define LOG_COPRO_UNKNOWN   (1 << 4)
#define LOG_COPRO_RESERVED  (1 << 5)
#define LOG_TLB             (1 << 6)

#define VERBOSE             (0) // (LOG_COPRO_READS | LOG_COPRO_WRITES | LOG_COPRO_UNKNOWN | LOG_COPRO_RESERVED)
#include "logmacro.h"

#define PRINT_HAPYFSH2      (0)
#define PRINT_CE_KERNEL		(0)

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
DEFINE_DEVICE_TYPE(SA1110,       sa1110_cpu_device,       "sa1110",       "Intel StrongARM SA-1110")
DEFINE_DEVICE_TYPE(IGS036,       igs036_cpu_device,       "igs036",       "IGS036")

arm7_cpu_device::arm7_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, ARM7, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_LITTLE)
{
}

arm7_cpu_device::arm7_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags, endianness_t endianness)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", endianness, 32, 32, 0)
	, m_prefetch_word0_shift(endianness == ENDIANNESS_LITTLE ? 0 : 16)
	, m_prefetch_word1_shift(endianness == ENDIANNESS_LITTLE ? 16 : 0)
	, m_endian(endianness)
	, m_archRev(archRev)
	, m_archFlags(archFlags)
	, m_vectorbase(0)
	, m_pc(0)
{
	memset(m_r, 0x00, sizeof(m_r));
	uint32_t arch = ARM9_COPRO_ID_ARCH_V4;
	if (m_archFlags & ARCHFLAG_T)
		arch = ARM9_COPRO_ID_ARCH_V4T;

	m_copro_id = ARM9_COPRO_ID_MFR_ARM | arch | ARM9_COPRO_ID_PART_GENERICARM7;

	// TODO[RH]: Default to 3-instruction prefetch for unknown ARM variants. Derived cores should set the appropriate value in their constructors.
	m_insn_prefetch_depth = 3;

	memset(m_insn_prefetch_buffer, 0, sizeof(uint32_t) * 3);
	memset(m_insn_prefetch_address, 0, sizeof(uint32_t) * 3);
	memset(m_insn_prefetch_valid, 0, sizeof(bool) * 3);
	m_insn_prefetch_count = 0;
	m_insn_prefetch_index = 0;
	m_tlb_log = 0;
	m_actual_log = 0;
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

	memset(ITCM, 0, 0x8000);
	memset(DTCM, 0, 0x4000);

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
	m_r[eCPSR] = val;
	if ((GET_CPSR & MODE_FLAG) != old_mode)
	{
		update_reg_ptr();
	}
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

	switch (lvl1_desc & 3)
	{
		case 0: // Unmapped
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
			return page_table_finish_translation(vaddr, COPRO_TLB_TYPE_SECTION, lvl1_desc, lvl1_desc, flags, lvl1_addr, lvl1_addr);

		case 3: // Fine Table
		{
			const uint32_t lvl2_addr = (lvl1_desc & COPRO_TLB_FPTB_ADDR_MASK) | ((vaddr & COPRO_TLB_VADDR_FSLTI_MASK) >> COPRO_TLB_VADDR_FSLTI_MASK_SHIFT);
			const uint32_t lvl2_desc = m_program->read_dword(lvl2_addr);

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
		return page_table_translate(vaddr, flags);
	}
}

void arm7_cpu_device::translate_insn_command(int ref, const std::vector<std::string> &params)
{
	translate_command(ref, params, TRANSLATE_FETCH);
}

void arm7_cpu_device::translate_data_command(int ref, const std::vector<std::string> &params)
{
	translate_command(ref, params, TRANSLATE_READ);
}

void arm7_cpu_device::translate_command(int ref, const std::vector<std::string> &params, int intention)
{
	uint64_t vaddr;

	if (!machine().debugger().commands().validate_number_parameter(params[0], vaddr)) return;

	vaddr &= 0xffffffff;

	offs_t paddr = (offs_t)vaddr;
	bool can_translate = memory_translate(AS_PROGRAM, intention, paddr);
	if (can_translate)
		machine().debugger().console().printf("%s vaddr %08x => phys %08x\n", intention == TRANSLATE_FETCH ? "instruction" : "data", (uint32_t)vaddr, paddr);
	else
		machine().debugger().console().printf("%s vaddr %08x => unmapped\n", intention == TRANSLATE_FETCH ? "instruction" : "data");
}

bool arm7_cpu_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	/* only applies to the program address space and only does something if the MMU's enabled */
	if (spacenum == AS_PROGRAM && (m_control & COPRO_CTRL_MMU_EN))
	{
		int intention_type = intention & TRANSLATE_TYPE_MASK;

		const int flags = (intention_type & TRANSLATE_FETCH) ? ARM7_TLB_ABORT_P : ARM7_TLB_ABORT_D;
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
		machine().debugger().console().register_command("translate_insn", CMDFLAG_NONE, 0, 1, 1, std::bind(&arm7_cpu_device::translate_insn_command, this, _1, _2));
		machine().debugger().console().register_command("translate_data", CMDFLAG_NONE, 0, 1, 1, std::bind(&arm7_cpu_device::translate_data_command, this, _1, _2));
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
	memset(m_r, 0, sizeof(m_r));
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
	memset(m_decoded_access_control, 0, sizeof(uint8_t) * 16);

	/* start up in SVC mode with interrupts disabled. */
	m_r[eCPSR] = I_MASK | F_MASK | 0x10;
	SwitchMode(eARM7_MODE_SVC);
	m_r[eR15] = 0 | m_vectorbase;

	m_impstate.cache_dirty = true;

	memset(m_dtlb_entries, 0, sizeof(tlb_entry) * ARRAY_LENGTH(m_dtlb_entries));
	memset(m_itlb_entries, 0, sizeof(tlb_entry) * ARRAY_LENGTH(m_itlb_entries));
	memset(m_dtlb_entry_index, 0, ARRAY_LENGTH(m_dtlb_entry_index));
	memset(m_itlb_entry_index, 0, ARRAY_LENGTH(m_itlb_entry_index));
}

void arm1176jzf_s_cpu_device::device_reset()
{
	arm7_cpu_device::device_reset();
	m_control = 0x00050078;
}

#define UNEXECUTED() \
	m_r[eR15] += 4; \
	m_icount +=2; /* Any unexecuted instruction only takes 1 cycle (page 193) */

void arm7_cpu_device::update_insn_prefetch(uint32_t curr_pc)
{
	curr_pc &= ~3;
	if (m_insn_prefetch_address[m_insn_prefetch_index] != curr_pc)
	{
		m_insn_prefetch_count = 0;
		m_insn_prefetch_index = 0;
	}

	if (m_insn_prefetch_count == m_insn_prefetch_depth)
		return;

	const uint32_t to_fetch = m_insn_prefetch_depth - m_insn_prefetch_count;
	const uint32_t start_index = (m_insn_prefetch_depth + (m_insn_prefetch_index - to_fetch)) % m_insn_prefetch_depth;
	//printf("need to prefetch %d instructions starting at index %d\n", to_fetch, start_index);

	uint32_t pc = curr_pc + m_insn_prefetch_count * 4;
	for (uint32_t i = 0; i < to_fetch; i++)
	{
		uint32_t index = (i + start_index) % m_insn_prefetch_depth;
		m_insn_prefetch_valid[index] = true;
		offs_t physical_pc = pc;
		if ((m_control & COPRO_CTRL_MMU_EN) && !translate_vaddr_to_paddr(physical_pc, ARM7_TLB_ABORT_P | ARM7_TLB_READ))
		{
			m_insn_prefetch_valid[index] = false;
			break;
		}
		uint32_t op = m_pr32(physical_pc);
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
	m_insn_prefetch_index = (m_insn_prefetch_index + 1) % m_insn_prefetch_depth;
	m_insn_prefetch_count--;
	return valid;
}

void arm7_cpu_device::add_ce_kernel_addr(offs_t addr, std::string value)
{
	m_ce_kernel_addrs[addr - 0xf0000000] = value;
	m_ce_kernel_addr_present[addr - 0xf0000000] = true;
}

void arm7_cpu_device::init_ce_kernel_addrs()
{
	memset(m_ce_kernel_addr_present, 0, 0x10400);

	add_ce_kernel_addr(0xf0010000, "SH_AFS_Unmount");
	add_ce_kernel_addr(0xf0010000, "SH_AFS_CloseHandle");
	add_ce_kernel_addr(0xf0010008, "SH_AFS_CreateDirectoryW");
	add_ce_kernel_addr(0xf001000c, "SH_AFS_RemoveDirectoryW");
	add_ce_kernel_addr(0xf0010010, "SH_AFS_GetFileAttributesW");
	add_ce_kernel_addr(0xf0010014, "SH_AFS_SetFileAttributesW");
	add_ce_kernel_addr(0xf0010018, "SH_AFS_CreateFileW");
	add_ce_kernel_addr(0xf001001c, "SH_AFS_DeleteFileW");
	add_ce_kernel_addr(0xf0010020, "SH_AFS_MoveFileW");
	add_ce_kernel_addr(0xf0010024, "SH_AFS_FindFirstFileW");
	add_ce_kernel_addr(0xf0010030, "SH_AFS_PrestoChangoFileName");
	add_ce_kernel_addr(0xf0010034, "SH_AFS_CloseAllFileHandles");
	add_ce_kernel_addr(0xf0010038, "SH_AFS_GetDiskFreeSpace");
	add_ce_kernel_addr(0xf001003c, "SH_AFS_NotifyMountedFS");
	add_ce_kernel_addr(0xf0010040, "SH_AFS_RegisterFileSystemFunction");
	add_ce_kernel_addr(0xf0010044, "SH_AFS_FindFirstChangeNotificationW");
	add_ce_kernel_addr(0xf000fffc, "SH_WIN32_NotSupported");
	add_ce_kernel_addr(0xf000fff8, "SH_WIN32_CreateAPISet");
	add_ce_kernel_addr(0xf000fff4, "SH_WIN32_VirtualAlloc");
	add_ce_kernel_addr(0xf000fff0, "SH_WIN32_VirtualFree");
	add_ce_kernel_addr(0xf000ffec, "SH_WIN32_VirtualProtect");
	add_ce_kernel_addr(0xf000ffe8, "SH_WIN32_VirtualQuery");
	add_ce_kernel_addr(0xf000ffe4, "SH_WIN32_VirtualCopy");
	add_ce_kernel_addr(0xf000ffe0, "SH_WIN32_LoadLibraryW");
	add_ce_kernel_addr(0xf000ffdc, "SH_WIN32_FreeLibrary");
	add_ce_kernel_addr(0xf000ffd8, "SH_WIN32_GetProcAddressW");
	add_ce_kernel_addr(0xf000ffd4, "SH_WIN32_ThreadAttachOrDetach");
	add_ce_kernel_addr(0xf000ffd0, "SH_WIN32_ThreadDetachAllDLLs");
	add_ce_kernel_addr(0xf000ffcc, "SH_WIN32_GetTickCount");
	add_ce_kernel_addr(0xf000ffc8, "SH_WIN32_OutputDebugStringW");
	add_ce_kernel_addr(0xf000ffc4, "SH_WIN32_TlsCall");
	add_ce_kernel_addr(0xf000ffc0, "SH_WIN32_GetSystemInfo");
	add_ce_kernel_addr(0xf000ffbc, "SH_WIN32_U_ropen");
	add_ce_kernel_addr(0xf000ffb8, "SH_WIN32_U_rread");
	add_ce_kernel_addr(0xf000ffb4, "SH_WIN32_U_rwrite");
	add_ce_kernel_addr(0xf000ffb0, "SH_WIN32_U_rlseek");
	add_ce_kernel_addr(0xf000ffac, "SH_WIN32_U_rclose");
	add_ce_kernel_addr(0xf000ffa8, "SH_WIN32_RegisterDbgZones");
	add_ce_kernel_addr(0xf000ffa4, "SH_WIN32_NKvDbgPrintfW");
	add_ce_kernel_addr(0xf000ffa0, "SH_WIN32_ProfileSyscall");
	add_ce_kernel_addr(0xf000ff9c, "SH_WIN32_FindResourceW");
	add_ce_kernel_addr(0xf000ff98, "SH_WIN32_LoadResource");
	add_ce_kernel_addr(0xf000ff94, "SH_WIN32_SizeofResource");
	add_ce_kernel_addr(0xf000ff90, "SH_WIN32_GetRealTime");
	add_ce_kernel_addr(0xf000ff8c, "SH_WIN32_SetRealTime");
	add_ce_kernel_addr(0xf000ff88, "SH_WIN32_ProcessDetachAllDLLs");
	add_ce_kernel_addr(0xf000ff84, "SH_WIN32_ExtractResource");
	add_ce_kernel_addr(0xf000ff80, "SH_WIN32_GetRomFileInfo");
	add_ce_kernel_addr(0xf000ff7c, "SH_WIN32_GetRomFileBytes");
	add_ce_kernel_addr(0xf000ff78, "SH_WIN32_CacheRangeFlush");
	add_ce_kernel_addr(0xf000ff74, "SH_WIN32_AddTrackedItem");
	add_ce_kernel_addr(0xf000ff70, "SH_WIN32_DeleteTrackedItem");
	add_ce_kernel_addr(0xf000ff6c, "SH_WIN32_PrintTrackedItem");
	add_ce_kernel_addr(0xf000ff68, "SH_WIN32_GetKPhys");
	add_ce_kernel_addr(0xf000ff64, "SH_WIN32_GiveKPhys");
	add_ce_kernel_addr(0xf000ff60, "SH_WIN32_SetExceptionHandler");
	add_ce_kernel_addr(0xf000ff5c, "SH_WIN32_RegisterTrackedItem");
	add_ce_kernel_addr(0xf000ff58, "SH_WIN32_FilterTrackedItem");
	add_ce_kernel_addr(0xf000ff54, "SH_WIN32_SetKernelAlarm");
	add_ce_kernel_addr(0xf000ff50, "SH_WIN32_RefreshKernelAlarm");
	add_ce_kernel_addr(0xf000ff4c, "SH_WIN32_CeGetRandomSeed");
	add_ce_kernel_addr(0xf000ff48, "SH_WIN32_CloseProcOE");
	add_ce_kernel_addr(0xf000ff44, "SH_WIN32_SetGwesOOMEvent");
	add_ce_kernel_addr(0xf000ff40, "SH_WIN32_StringCompress");
	add_ce_kernel_addr(0xf000ff3c, "SH_WIN32_StringDecompress");
	add_ce_kernel_addr(0xf000ff38, "SH_WIN32_BinaryCompress");
	add_ce_kernel_addr(0xf000ff34, "SH_WIN32_BinaryDecompress");
	add_ce_kernel_addr(0xf000ff30, "SH_WIN32_CreateEventW");
	add_ce_kernel_addr(0xf000ff2c, "SH_WIN32_CreateProcessW");
	add_ce_kernel_addr(0xf000ff28, "SH_WIN32_CreateThread");
	add_ce_kernel_addr(0xf000ff24, "SH_WIN32_InputDebugCharW");
	add_ce_kernel_addr(0xf000ff20, "SH_WIN32_TakeCritSec");
	add_ce_kernel_addr(0xf000ff1c, "SH_WIN32_LeaveCritSec");
	add_ce_kernel_addr(0xf000ff18, "SH_WIN32_WaitForMultipleObjects");
	add_ce_kernel_addr(0xf000ff14, "SH_WIN32_MapPtrToProcess");
	add_ce_kernel_addr(0xf000ff10, "SH_WIN32_MapPtrUnsecure");
	add_ce_kernel_addr(0xf000ff0c, "SH_WIN32_GetProcFromPtr");
	add_ce_kernel_addr(0xf000ff08, "SH_WIN32_IsBadPtr");
	add_ce_kernel_addr(0xf000ff04, "SH_WIN32_GetProcAddrBits");
	add_ce_kernel_addr(0xf000ff00, "SH_WIN32_GetFSHeapInfo");
	add_ce_kernel_addr(0xf000fefc, "SH_WIN32_OtherThreadsRunning");
	add_ce_kernel_addr(0xf000fef8, "SH_WIN32_KillAllOtherThreads");
	add_ce_kernel_addr(0xf000fef4, "SH_WIN32_GetOwnerProcess");
	add_ce_kernel_addr(0xf000fef0, "SH_WIN32_GetCallerProcess");
	add_ce_kernel_addr(0xf000feec, "SH_WIN32_GetIdleTime");
	add_ce_kernel_addr(0xf000fee8, "SH_WIN32_SetLowestScheduledPriority");
	add_ce_kernel_addr(0xf000fee4, "SH_WIN32_IsPrimaryThread");
	add_ce_kernel_addr(0xf000fee0, "SH_WIN32_SetProcPermissions");
	add_ce_kernel_addr(0xf000fedc, "SH_WIN32_GetCurrentPermissions");
	add_ce_kernel_addr(0xf000fed4, "SH_WIN32_SetDaylightTime");
	add_ce_kernel_addr(0xf000fed0, "SH_WIN32_SetTimeZoneBias");
	add_ce_kernel_addr(0xf000fecc, "SH_WIN32_SetCleanRebootFlag");
	add_ce_kernel_addr(0xf000fec8, "SH_WIN32_CreateCrit");
	add_ce_kernel_addr(0xf000fec4, "SH_WIN32_PowerOffSystem");
	add_ce_kernel_addr(0xf000fec0, "SH_WIN32_CreateMutexW");
	add_ce_kernel_addr(0xf000febc, "SH_WIN32_SetDbgZone");
	add_ce_kernel_addr(0xf000feb8, "SH_WIN32_Sleep");
	add_ce_kernel_addr(0xf000feb4, "SH_WIN32_TurnOnProfiling");
	add_ce_kernel_addr(0xf000feb0, "SH_WIN32_TurnOffProfiling");
	add_ce_kernel_addr(0xf000feac, "SH_WIN32_CeGetCurrentTrust");
	add_ce_kernel_addr(0xf000fea8, "SH_WIN32_CeGetCallerTrust");
	add_ce_kernel_addr(0xf000fea4, "SH_WIN32_NKTerminateThread");
	add_ce_kernel_addr(0xf000fea0, "SH_WIN32_SetLastError");
	add_ce_kernel_addr(0xf000fe9c, "SH_WIN32_GetLastError");
	add_ce_kernel_addr(0xf000fe98, "SH_WIN32_GetProcName");
	add_ce_kernel_addr(0xf000fe94, "SH_WIN32_ExitProcess");
	add_ce_kernel_addr(0xf000fe90, "SH_WIN32_CloseAllHandles");
	add_ce_kernel_addr(0xf000fe8c, "SH_WIN32_SetHandleOwner");
	add_ce_kernel_addr(0xf000fe88, "SH_WIN32_LoadDriver");
	add_ce_kernel_addr(0xf000fe84, "SH_WIN32_CreateFileMappingW");
	add_ce_kernel_addr(0xf000fe80, "SH_WIN32_UnmapViewOfFile");
	add_ce_kernel_addr(0xf000fe7c, "SH_WIN32_FlushViewOfFile");
	add_ce_kernel_addr(0xf000fe78, "SH_WIN32_CreateFileForMappingW");
	add_ce_kernel_addr(0xf000fe74, "SH_WIN32_KernelIoControl");
	add_ce_kernel_addr(0xf000fe70, "SH_WIN32_GetCallStackSnapshot");
	add_ce_kernel_addr(0xf000fe6c, "SH_WIN32_PPSHRestart");
	add_ce_kernel_addr(0xf000fe64, "SH_WIN32_UpdateNLSInfoEx");
	add_ce_kernel_addr(0xf000fe60, "SH_WIN32_ConnectDebugger");
	add_ce_kernel_addr(0xf000fe5c, "SH_WIN32_InterruptInitialize");
	add_ce_kernel_addr(0xf000fe58, "SH_WIN32_InterruptDone");
	add_ce_kernel_addr(0xf000fe54, "SH_WIN32_InterruptDisable");
	add_ce_kernel_addr(0xf000fe50, "SH_WIN32_SetKMode");
	add_ce_kernel_addr(0xf000fe4c, "SH_WIN32_SetPowerOffHandler");
	add_ce_kernel_addr(0xf000fe48, "SH_WIN32_SetGwesPowerHandler");
	add_ce_kernel_addr(0xf000fe44, "SH_WIN32_SetHardwareWatch");
	add_ce_kernel_addr(0xf000fe40, "SH_WIN32_QueryAPISetID");
	add_ce_kernel_addr(0xf000fe3c, "SH_WIN32_PerformCallBack");
	add_ce_kernel_addr(0xf000fe38, "SH_WIN32_RaiseException");
	add_ce_kernel_addr(0xf000fe34, "SH_WIN32_GetCallerProcessIndex");
	add_ce_kernel_addr(0xf000fe30, "SH_WIN32_WaitForDebugEvent");
	add_ce_kernel_addr(0xf000fe2c, "SH_WIN32_ContinueDebugEvent");
	add_ce_kernel_addr(0xf000fe28, "SH_WIN32_DebugNotify");
	add_ce_kernel_addr(0xf000fe24, "SH_WIN32_OpenProcess");
	add_ce_kernel_addr(0xf000fe20, "SH_WIN32_THCreateSnapshot");
	add_ce_kernel_addr(0xf000fe1c, "SH_WIN32_THGrow");
	add_ce_kernel_addr(0xf000fe18, "SH_WIN32_NotifyForceCleanboot");
	add_ce_kernel_addr(0xf000fe14, "SH_WIN32_DumpKCallProfile");
	add_ce_kernel_addr(0xf000fe10, "SH_WIN32_GetProcessVersion");
	add_ce_kernel_addr(0xf000fe0c, "SH_WIN32_GetModuleFileNameW");
	add_ce_kernel_addr(0xf000fe08, "SH_WIN32_QueryPerformanceCounter");
	add_ce_kernel_addr(0xf000fe04, "SH_WIN32_QueryPerformanceFrequency");
	add_ce_kernel_addr(0xf000fe00, "SH_WIN32_KernExtractIcons");
	add_ce_kernel_addr(0xf000fdfc, "SH_WIN32_ForcePageout");
	add_ce_kernel_addr(0xf000fdf8, "SH_WIN32_GetThreadTimes");
	add_ce_kernel_addr(0xf000fdf4, "SH_WIN32_GetModuleHandleW");
	add_ce_kernel_addr(0xf000fdf0, "SH_WIN32_SetWDevicePowerHandler");
	add_ce_kernel_addr(0xf000fdec, "SH_WIN32_SetStdioPathW");
	add_ce_kernel_addr(0xf000fde8, "SH_WIN32_GetStdioPathW");
	add_ce_kernel_addr(0xf000fde4, "SH_WIN32_ReadRegistryFromOEM");
	add_ce_kernel_addr(0xf000fde0, "SH_WIN32_WriteRegistryToOEM");
	add_ce_kernel_addr(0xf000fddc, "SH_WIN32_WriteDebugLED");
	add_ce_kernel_addr(0xf000fdd8, "SH_WIN32_LockPages");
	add_ce_kernel_addr(0xf000fdd4, "SH_WIN32_UnlockPages");
	add_ce_kernel_addr(0xf000fdd0, "SH_WIN32_VirtualSetAttributes");
	add_ce_kernel_addr(0xf000fdcc, "SH_WIN32_SetRAMMode");
	add_ce_kernel_addr(0xf000fdc8, "SH_WIN32_SetStoreQueueBase");
	add_ce_kernel_addr(0xf000fdc4, "SH_WIN32_FlushViewOfFileMaybe");
	add_ce_kernel_addr(0xf000fdc0, "SH_WIN32_GetProcAddressA");
	add_ce_kernel_addr(0xf000fdbc, "SH_WIN32_GetCommandLineW");
	add_ce_kernel_addr(0xf000fdb8, "SH_WIN32_DisableThreadLibraryCalls");
	add_ce_kernel_addr(0xf000fdb4, "SH_WIN32_CreateSemaphoreW");
	add_ce_kernel_addr(0xf000fdb0, "SH_WIN32_LoadLibraryExW");
	add_ce_kernel_addr(0xf000fdac, "SH_WIN32_PerformCallForward");
	add_ce_kernel_addr(0xf000fda8, "SH_WIN32_CeMapArgumentArray");
	add_ce_kernel_addr(0xf000fda4, "SH_WIN32_KillThreadIfNeeded");
	add_ce_kernel_addr(0xf000fda0, "SH_WIN32_GetProcessIndexFromID");
	add_ce_kernel_addr(0xf000fd9c, "SH_WIN32_RegisterGwesHandler");
	add_ce_kernel_addr(0xf000fd98, "SH_WIN32_GetProfileBaseAddress");
	add_ce_kernel_addr(0xf000fd94, "SH_WIN32_SetProfilePortAddress");
	add_ce_kernel_addr(0xf000fd90, "SH_WIN32_CeLogData");
	add_ce_kernel_addr(0xf000fd8c, "SH_WIN32_CeLogSetZones");
	add_ce_kernel_addr(0xf000fd88, "SH_WIN32_CeModuleJit");
	add_ce_kernel_addr(0xf000fd84, "SH_WIN32_CeSetExtendedPdata");
	add_ce_kernel_addr(0xf000fd80, "SH_WIN32_VerQueryValueW");
	add_ce_kernel_addr(0xf000fd7c, "SH_WIN32_GetFileVersionInfoSizeW");
	add_ce_kernel_addr(0xf000fd78, "SH_WIN32_GetFileVersionInfoW");
	add_ce_kernel_addr(0xf000fd74, "SH_WIN32_CreateLocaleView");
	add_ce_kernel_addr(0xf000fd70, "SH_WIN32_CeLogReSync");
	add_ce_kernel_addr(0xf000fd6c, "SH_WIN32_LoadIntChainHandler");
	add_ce_kernel_addr(0xf000fd68, "SH_WIN32_FreeIntChainHandler");
	add_ce_kernel_addr(0xf000fd64, "SH_WIN32_LoadKernelLibrary");
	add_ce_kernel_addr(0xf000fd60, "SH_WIN32_AllocPhysMem");
	add_ce_kernel_addr(0xf000fd5c, "SH_WIN32_FreePhysMem");
	add_ce_kernel_addr(0xf000fd58, "SH_WIN32_KernelLibIoControl");
	add_ce_kernel_addr(0xf000fd54, "SH_WIN32_OpenEventW");
	add_ce_kernel_addr(0xf000fd50, "SH_WIN32_SleepTillTick");
	add_ce_kernel_addr(0xf000fd4c, "SH_WIN32_DuplicateHandle");
	add_ce_kernel_addr(0xf000fd48, "SH_WIN32_CreateStaticMapping");
	add_ce_kernel_addr(0xf000fd44, "SH_WIN32_MapCallerPtr");
	add_ce_kernel_addr(0xf000fd40, "SH_WIN32_MapPtrToProcWithSize");
	add_ce_kernel_addr(0xf000fd3c, "SH_WIN32_LoadStringW");
	add_ce_kernel_addr(0xf000fd38, "SH_WIN32_QueryInstructionSet");
	add_ce_kernel_addr(0xf000fd34, "SH_WIN32_CeLogGetZones");
	add_ce_kernel_addr(0xf000fd30, "SH_WIN32_GetProcessIDFromIndex");
	add_ce_kernel_addr(0xf000fd2c, "SH_WIN32_IsProcessorFeaturePresent");
	add_ce_kernel_addr(0xf000fd28, "SH_WIN32_DecompressBinaryBlock");
	add_ce_kernel_addr(0xf000fd24, "SH_WIN32_PageOutModule");
	add_ce_kernel_addr(0xf000fd20, "SH_WIN32_InterruptMask");
	add_ce_kernel_addr(0xf000fd1c, "SH_WIN32_GetProcModList");
	add_ce_kernel_addr(0xf000fd18, "SH_WIN32_FreeModFromCurrProc");
	add_ce_kernel_addr(0xf000fd14, "SH_WIN32_CeVirtualSharedAlloc");
	add_ce_kernel_addr(0xf000fd10, "SH_WIN32_NKDeleteStaticMapping");
	add_ce_kernel_addr(0xf000fd0c, "SH_WIN32_CreateToken");
	add_ce_kernel_addr(0xf000fd08, "SH_WIN32_RevertToSelf");
	add_ce_kernel_addr(0xf000fd04, "SH_WIN32_CeImpersonateCurrProc");
	add_ce_kernel_addr(0xf000fd00, "SH_WIN32_CeDuplicateToken");
	add_ce_kernel_addr(0xf000fcfc, "SH_WIN32_ConnectHdstub");
	add_ce_kernel_addr(0xf000fcf8, "SH_WIN32_ConnectOsAxsT0");
	add_ce_kernel_addr(0xf000fcf4, "SH_WIN32_IsNamedEventSignaled");
	add_ce_kernel_addr(0xf000fcf0, "SH_WIN32_ConnectOsAxsT1");
	add_ce_kernel_addr(0xf000fcec, "SH_WIN32_DebugSetProcessKillOnExit");
	add_ce_kernel_addr(0xf000fce8, "SH_WIN32_CeGetProcessTrust");
	add_ce_kernel_addr(0xf000fce4, "SH_WIN32_CeOpenFileHandle");

	add_ce_kernel_addr(0xf000fbfc, "SH_CURTHREAD_ThreadCloseHandle");
	add_ce_kernel_addr(0xf000fbf8, "SH_CURTHREAD_SuspendThread");
	add_ce_kernel_addr(0xf000fbf4, "SH_CURTHREAD_ResumeThread");
	add_ce_kernel_addr(0xf000fbf0, "SH_CURTHREAD_SetThreadPriority");
	add_ce_kernel_addr(0xf000fbec, "SH_CURTHREAD_GetThreadPriority");
	add_ce_kernel_addr(0xf000fbe8, "SH_CURTHREAD_GetExitCodeThread");
	add_ce_kernel_addr(0xf000fbe4, "SH_CURTHREAD_GetThreadContext");
	add_ce_kernel_addr(0xf000fbe0, "SH_CURTHREAD_SetThreadContext");
	add_ce_kernel_addr(0xf000fbdc, "SH_CURTHREAD_TerminateThread");
	add_ce_kernel_addr(0xf000fbd8, "SH_CURTHREAD_CeGetThreadPriority");
	add_ce_kernel_addr(0xf000fbd4, "SH_CURTHREAD_CeSetThreadPriority");
	add_ce_kernel_addr(0xf000fbd0, "SH_CURTHREAD_CeGetThreadQuantum");
	add_ce_kernel_addr(0xf000fbcc, "SH_CURTHREAD_CeSetThreadQuantum");
	add_ce_kernel_addr(0xf000f800, "SH_CURPROC_ProcCloseHandle");
	add_ce_kernel_addr(0xf000f7f8, "SH_CURPROC_TerminateProcess");
	add_ce_kernel_addr(0xf000f7f4, "SH_CURPROC_GetExitCodeProcess");
	add_ce_kernel_addr(0xf000f7ec, "SH_CURPROC_FlushInstructionCache");
	add_ce_kernel_addr(0xf000f7e8, "SH_CURPROC_ReadProcessMemory");
	add_ce_kernel_addr(0xf000f7e4, "SH_CURPROC_WriteProcessMemory");
	add_ce_kernel_addr(0xf000f7e0, "SH_CURPROC_DebugActiveProcess");
	add_ce_kernel_addr(0xf000f7dc, "SH_CURPROC_GetModuleInformation");
	add_ce_kernel_addr(0xf000f7d8, "SH_CURPROC_CeSetProcessVersion");
	add_ce_kernel_addr(0xf000f7d4, "SH_CURPROC_DebugActiveProcessStop");
	add_ce_kernel_addr(0xf000f7d0, "SH_CURPROC_CeGetModuleInfo");
	add_ce_kernel_addr(0xf000f7cc, "SH_CURPROC_CheckRemoteDebuggerPresent");
	add_ce_kernel_addr(0xf000f000, "HT_EVENT_EventCloseHandle");
	add_ce_kernel_addr(0xf000eff8, "HT_EVENT_EventModify");
	add_ce_kernel_addr(0xf000eff4, "HT_EVENT_AddEventAccess");
	add_ce_kernel_addr(0xf000eff0, "HT_EVENT_EventGetData");
	add_ce_kernel_addr(0xf000efec, "HT_EVENT_EventSetData");
	add_ce_kernel_addr(0xf000ec00, "HT_MUTEX_MutexCloseHandle");
	add_ce_kernel_addr(0xf000ebf8, "HT_MUTEX_ReleaseMutex");
	add_ce_kernel_addr(0xf000e800, "HT_APISET_CloseAPISet");
	add_ce_kernel_addr(0xf000e7f8, "HT_APISET_RegisterAPISet");
	add_ce_kernel_addr(0xf000e7f4, "HT_APISET_CreateAPIHandle");
	add_ce_kernel_addr(0xf000e7f0, "HT_APISET_VerifyAPIHandle");
	add_ce_kernel_addr(0xf000e3fc, "HT_FILE_PSLGetStoreInfo");
	add_ce_kernel_addr(0xf000e3f8, "HT_FILE_PSLDismountStore");
	add_ce_kernel_addr(0xf000e3f8, "HT_FILE_ReadFile");
	add_ce_kernel_addr(0xf000e3f4, "HT_FILE_PSLFormatStore");
	add_ce_kernel_addr(0xf000e3f4, "HT_FILE_WriteFile");
	add_ce_kernel_addr(0xf000e3f0, "HT_FILE_GetFileSize");
	add_ce_kernel_addr(0xf000e3f0, "HT_FILE_PSLCreatePart");
	add_ce_kernel_addr(0xf000e3ec, "HT_FILE_PSLDeletePartition");
	add_ce_kernel_addr(0xf000e3ec, "HT_FILE_SetFilePointer");
	add_ce_kernel_addr(0xf000e3e8, "HT_FILE_GetFileInformationByHandle");
	add_ce_kernel_addr(0xf000e3e8, "HT_FILE_PSLOpenPartition");
	add_ce_kernel_addr(0xf000e3e4, "HT_FILE_FlushFileBuffers");
	add_ce_kernel_addr(0xf000e3e4, "HT_FILE_PSLMountPartition");
	add_ce_kernel_addr(0xf000e3e0, "HT_FILE_GetFileTime");
	add_ce_kernel_addr(0xf000e3e0, "HT_FILE_PSLDismountPartition");
	add_ce_kernel_addr(0xf000e3dc, "HT_FILE_PSLRenamePartition");
	add_ce_kernel_addr(0xf000e3dc, "HT_FILE_SetFileTime");
	add_ce_kernel_addr(0xf000e3d8, "HT_FILE_PSLSetPartitionAttributes");
	add_ce_kernel_addr(0xf000e3d8, "HT_FILE_SetEndOfFile");
	add_ce_kernel_addr(0xf000e3d4, "HT_FILE_DeviceIoControl");
	add_ce_kernel_addr(0xf000e3d0, "HT_FILE_PSLGetPartitionInfo");
	add_ce_kernel_addr(0xf000e3d0, "HT_FILE_ReadFileWithSeek");
	add_ce_kernel_addr(0xf000e3cc, "HT_FILE_PSLFormatPart");
	add_ce_kernel_addr(0xf000e3cc, "HT_FILE_WriteFileWithSeek");
	add_ce_kernel_addr(0xf000e3c8, "HT_FILE_PSLFindFirstPartition");
	add_ce_kernel_addr(0xf000e000, "HT_FIND_FindClose");
	//add_ce_kernel_addr(0xf000e000, "HT_FIND_PSLFindClosePartition");
	//add_ce_kernel_addr(0xf000e000, "HT_FIND_PSLFindCloseStore");
	add_ce_kernel_addr(0xf000dff8, "HT_FIND_FindNextFileW");
	//add_ce_kernel_addr(0xf000dff8, "HT_FIND_PSLFindNextPartition");
	//add_ce_kernel_addr(0xf000dff8, "HT_FIND_PSLFindNextStore");
	add_ce_kernel_addr(0xf000dbf8, "HT_DBFILE_CeSeekDatabaseEx");
	add_ce_kernel_addr(0xf000dbf4, "HT_DBFILE_CeDeleteRecord");
	add_ce_kernel_addr(0xf000dbf0, "HT_DBFILE_CeReadRecordPropsEx");
	add_ce_kernel_addr(0xf000dbec, "HT_DBFILE_CeWriteRecordProps");
	add_ce_kernel_addr(0xf000dbe8, "HT_DBFILE_CeGetDBInformationByHandle");
	add_ce_kernel_addr(0xf000d7f8, "HT_DBFIND_CeFindNextDatabaseEx");
	add_ce_kernel_addr(0xf000d400, "HT_SOCKET_AFDCloseSocket");
	add_ce_kernel_addr(0xf000d3f8, "HT_SOCKET_AFDAccept");
	add_ce_kernel_addr(0xf000d3f4, "HT_SOCKET_AFDBind");
	add_ce_kernel_addr(0xf000d3f0, "HT_SOCKET_AFDConnect");
	add_ce_kernel_addr(0xf000d3ec, "HT_SOCKET_AFDIoctl");
	add_ce_kernel_addr(0xf000d3e8, "HT_SOCKET_AFDListen");
	add_ce_kernel_addr(0xf000d3e4, "HT_SOCKET_AFDRecv");
	add_ce_kernel_addr(0xf000d3e0, "HT_SOCKET_AFDSend");
	add_ce_kernel_addr(0xf000d3dc, "HT_SOCKET_AFDShutdown");
	add_ce_kernel_addr(0xf000d3d8, "HT_SOCKET_AFDGetsockname");
	add_ce_kernel_addr(0xf000d3d4, "HT_SOCKET_AFDGetpeername");
	add_ce_kernel_addr(0xf000d3d0, "HT_SOCKET_AFDGetSockOpt");
	add_ce_kernel_addr(0xf000d3cc, "HT_SOCKET_AFDSetSockOpt");
	add_ce_kernel_addr(0xf000d3c8, "HT_SOCKET_AFDWakeup");
	add_ce_kernel_addr(0xf000d3c4, "HT_SOCKET_AFDGetOverlappedResult");
	add_ce_kernel_addr(0xf000d3c0, "HT_SOCKET_AFDEventSelect");
	add_ce_kernel_addr(0xf000d3bc, "HT_SOCKET_AFDEnumNetworkEvents");
	add_ce_kernel_addr(0xf000cc00, "HT_SEMAPHORE_SemaphoreCloseHandle");
	add_ce_kernel_addr(0xf000cbf8, "HT_SEMAPHORE_ReleaseSemaphore");
	add_ce_kernel_addr(0xf000c800, "HT_FSMAP_MapCloseHandle");
	add_ce_kernel_addr(0xf000c7f8, "HT_FSMAP_MapViewOfFile");
	add_ce_kernel_addr(0xf000c400, "HT_WNETENUM_WNetCloseEnum");
	add_ce_kernel_addr(0xf000c3f8, "HT_WNETENUM_WNetEnumResourceW");
	add_ce_kernel_addr(0xf000bff8, "SH_GDI_AddFontResourceW");
	add_ce_kernel_addr(0xf000bff4, "SH_GDI_BitBlt");
	add_ce_kernel_addr(0xf000bff0, "SH_GDI_CombineRgn");
	add_ce_kernel_addr(0xf000bfec, "SH_GDI_CreateCompatibleDC");
	add_ce_kernel_addr(0xf000bfe8, "SH_GDI_CreateDIBPatternBrushPt");
	add_ce_kernel_addr(0xF000BFE4, "SH_GDI_CreateDIBSection");
	add_ce_kernel_addr(0xf000bfe0, "SH_GDI_CreateFontIndirectW");
	add_ce_kernel_addr(0xf000bfdc, "SH_GDI_CreateRectRgnIndirect");
	add_ce_kernel_addr(0xf000bfd8, "SH_GDI_CreatePenIndirect");
	add_ce_kernel_addr(0xf000bfd4, "SH_GDI_CreateSolidBrush");
	add_ce_kernel_addr(0xf000bfd0, "SH_GDI_DeleteDC");
	add_ce_kernel_addr(0xf000bfcc, "SH_GDI_DeleteObject");
	add_ce_kernel_addr(0xf000bfc8, "SH_GDI_DrawEdge");
	add_ce_kernel_addr(0xf000bfc4, "SH_GDI_DrawFocusRect");
	add_ce_kernel_addr(0xf000bfc0, "SH_GDI_DrawTextW");
	add_ce_kernel_addr(0xf000bfbc, "SH_GDI_Ellipse");
	add_ce_kernel_addr(0xf000bfb8, "SH_GDI_EnumFontFamiliesW");
	add_ce_kernel_addr(0xF000BFB4, "SH_GDI_EnumFontsW");
	add_ce_kernel_addr(0xf000bfb0, "SH_GDI_ExcludeClipRect");
	add_ce_kernel_addr(0xf000bfac, "SH_GDI_ExtTextOutW");
	add_ce_kernel_addr(0xf000bfa8, "SH_GDI_FillRect");
	add_ce_kernel_addr(0xf000bfa0, "SH_GDI_GetBkColor");
	add_ce_kernel_addr(0xf000bf9c, "SH_GDI_GetBkMode");
	add_ce_kernel_addr(0xf000bf98, "SH_GDI_GetClipRgn");
	add_ce_kernel_addr(0xf000bf94, "SH_GDI_GetCurrentObject");
	add_ce_kernel_addr(0xf000bf90, "SH_GDI_GetDeviceCaps");
	add_ce_kernel_addr(0xf000bf8c, "SH_GDI_GetNearestColor");
	add_ce_kernel_addr(0xf000bf88, "SH_GDI_GetObjectW");
	add_ce_kernel_addr(0xf000bf84, "SH_GDI_GetObjectType");
	add_ce_kernel_addr(0xf000bf80, "SH_GDI_GetPixel");
	add_ce_kernel_addr(0xf000bf7c, "SH_GDI_GetRegionData");
	add_ce_kernel_addr(0xf000bf78, "SH_GDI_GetRgnBox");
	add_ce_kernel_addr(0xf000bf74, "SH_GDI_GetStockObject");
	add_ce_kernel_addr(0xf000bf70, "SH_GDI_PatBlt");
	add_ce_kernel_addr(0xf000bf6c, "SH_GDI_GetTextColor");
	add_ce_kernel_addr(0xf000bf68, "SH_GDI_GetTextExtentExPointW");
	add_ce_kernel_addr(0xf000bf64, "SH_GDI_GetTextFaceW");
	add_ce_kernel_addr(0xf000bf60, "SH_GDI_GetTextMetricsW");
	add_ce_kernel_addr(0xf000bf5c, "SH_GDI_MaskBlt");
	add_ce_kernel_addr(0xf000bf58, "SH_GDI_OffsetRgn");
	add_ce_kernel_addr(0xf000bf54, "SH_GDI_Polygon");
	add_ce_kernel_addr(0xf000bf50, "SH_GDI_Polyline");
	add_ce_kernel_addr(0xf000bf4c, "SH_GDI_PtInRegion");
	add_ce_kernel_addr(0xf000bf48, "SH_GDI_Rectangle");
	add_ce_kernel_addr(0xf000bf44, "SH_GDI_RectInRegion");
	add_ce_kernel_addr(0xf000bf40, "SH_GDI_RemoveFontResourceW");
	add_ce_kernel_addr(0xf000bf3c, "SH_GDI_RestoreDC");
	add_ce_kernel_addr(0xf000bf38, "SH_GDI_RoundRect");
	add_ce_kernel_addr(0xf000bf34, "SH_GDI_SaveDC");
	add_ce_kernel_addr(0xf000bf30, "SH_GDI_SelectClipRgn");
	add_ce_kernel_addr(0xf000bf2c, "SH_GDI_SelectObject");
	add_ce_kernel_addr(0xf000bf28, "SH_GDI_SetBkColor");
	add_ce_kernel_addr(0xf000bf24, "SH_GDI_SetBkMode");
	add_ce_kernel_addr(0xf000bf20, "SH_GDI_SetBrushOrgEx");
	add_ce_kernel_addr(0xf000bf1c, "SH_GDI_SetPixel");
	add_ce_kernel_addr(0xf000bf18, "SH_GDI_SetTextColor");
	add_ce_kernel_addr(0xf000bf14, "SH_GDI_StretchBlt");
	add_ce_kernel_addr(0xf000bf10, "SH_GDI_CreateBitmap");
	add_ce_kernel_addr(0xf000bf0c, "SH_GDI_CreateCompatibleBitmap");
	add_ce_kernel_addr(0xf000bf08, "SH_GDI_GetSysColorBrush");
	add_ce_kernel_addr(0xf000bf04, "SH_GDI_IntersectClipRect");
	add_ce_kernel_addr(0xf000bf00, "SH_GDI_GetClipBox");
	add_ce_kernel_addr(0xf000befc, "SH_GDI_CeRemoveFontResource");
	add_ce_kernel_addr(0xf000bef8, "SH_GDI_EnableEUDC");
	add_ce_kernel_addr(0xf000bef4, "SH_GDI_CloseEnhMetaFile");
	add_ce_kernel_addr(0xf000bef0, "SH_GDI_CreateEnhMetaFileW");
	add_ce_kernel_addr(0xf000beec, "SH_GDI_DeleteEnhMetaFile");
	add_ce_kernel_addr(0xf000bee8, "SH_GDI_PlayEnhMetaFile");
	add_ce_kernel_addr(0xf000bee4, "SH_GDI_CreatePalette");
	add_ce_kernel_addr(0xf000bee0, "SH_GDI_SelectPalette");
	add_ce_kernel_addr(0xf000bedc, "SH_GDI_RealizePalette");
	add_ce_kernel_addr(0xf000bed8, "SH_GDI_GetPaletteEntries");
	add_ce_kernel_addr(0xf000bed4, "SH_GDI_SetPaletteEntries");
	add_ce_kernel_addr(0xf000bed0, "SH_GDI_GetSystemPaletteEntries");
	add_ce_kernel_addr(0xf000becc, "SH_GDI_GetNearestPaletteIndex");
	add_ce_kernel_addr(0xf000bec8, "SH_GDI_CreatePen");
	add_ce_kernel_addr(0xf000bec4, "SH_GDI_StartDocW");
	add_ce_kernel_addr(0xf000bec0, "SH_GDI_EndDoc");
	add_ce_kernel_addr(0xf000bebc, "SH_GDI_StartPage");
	add_ce_kernel_addr(0xf000beb8, "SH_GDI_EndPage");
	add_ce_kernel_addr(0xf000beb4, "SH_GDI_AbortDoc");
	add_ce_kernel_addr(0xf000beb0, "SH_GDI_SetAbortProc");
	add_ce_kernel_addr(0xf000beac, "SH_GDI_CreateDCW");
	add_ce_kernel_addr(0xf000bea8, "SH_GDI_CreateRectRgn");
	add_ce_kernel_addr(0xf000bea4, "SH_GDI_FillRgn");
	add_ce_kernel_addr(0xf000bea0, "SH_GDI_SetROP2");
	add_ce_kernel_addr(0xf000be9c, "SH_GDI_SetRectRgn");
	add_ce_kernel_addr(0xf000be98, "SH_GDI_RectVisible");
	add_ce_kernel_addr(0xf000be94, "SH_GDI_CreatePatternBrush");
	add_ce_kernel_addr(0xf000be90, "SH_GDI_CreateBitmapFromPointer");
	add_ce_kernel_addr(0xf000be8c, "SH_GDI_SetViewportOrgEx");
	add_ce_kernel_addr(0xf000be88, "SH_GDI_TransparentImage");
	add_ce_kernel_addr(0xf000be84, "SH_GDI_SetObjectOwner");
	add_ce_kernel_addr(0xf000be80, "SH_GDI_TranslateCharsetInfo");
	add_ce_kernel_addr(0xf000be7c, "SH_GDI_ExtEscape");
	add_ce_kernel_addr(0xf000be78, "SH_GDI_SetWindowsHookExW_Trap");
	add_ce_kernel_addr(0xf000be74, "SH_GDI_UnhookWindowsHookEx_Trap");
	add_ce_kernel_addr(0xf000be70, "SH_GDI_GetForegroundInfo");
	add_ce_kernel_addr(0xf000be6c, "SH_GDI_CeGetUserNotificationPreferences");
	add_ce_kernel_addr(0xf000be68, "SH_GDI_CeSetUserNotificationEx");
	add_ce_kernel_addr(0xf000be64, "SH_GDI_CeClearUserNotification");
	add_ce_kernel_addr(0xf000be60, "SH_GDI_CeRunAppAtEvent");
	add_ce_kernel_addr(0xf000be5c, "SH_GDI_CeHandleAppNotifications");
	add_ce_kernel_addr(0xf000be58, "SH_GDI_CeGetUserNotificationHandles");
	add_ce_kernel_addr(0xf000be54, "SH_GDI_CeGetUserNotification");
	add_ce_kernel_addr(0xf000be50, "SH_GDI_CeEventHasOccurred");
	add_ce_kernel_addr(0xf000be4c, "SH_GDI_SetWindowRgn_Trap");
	add_ce_kernel_addr(0xf000be48, "SH_GDI_GetPrivateCallbacks_Trap");
	add_ce_kernel_addr(0xf000be44, "SH_GDI_GetWindowRgn_Trap");
	add_ce_kernel_addr(0xf000be40, "SH_GDI_CeRunAppAtTime");
	add_ce_kernel_addr(0xf000be3c, "SH_GDI_GetDesktopWindow_Trap");
	add_ce_kernel_addr(0xf000be38, "SH_GDI_InSendMessage_Trap");
	add_ce_kernel_addr(0xf000be34, "SH_GDI_GetQueueStatus_Trap");
	add_ce_kernel_addr(0xf000be30, "SH_GDI_AllKeys_Trap");
	add_ce_kernel_addr(0xf000be2c, "SH_GDI_LoadAnimatedCursor_Trap");
	add_ce_kernel_addr(0xf000be28, "SH_GDI_SendMessageTimeout");
	add_ce_kernel_addr(0xf000be24, "SH_GDI_SetProp_Trap");
	add_ce_kernel_addr(0xf000be20, "SH_GDI_GetProp_Trap");
	add_ce_kernel_addr(0xf000be1c, "SH_GDI_RemoveProp_Trap");
	add_ce_kernel_addr(0xf000be18, "SH_GDI_EnumPropsEx_Trap");
	add_ce_kernel_addr(0xf000be14, "SH_GDI_GetMessageQueueReadyTimeStamp");
	add_ce_kernel_addr(0xf000be10, "SH_GDI_RegisterTaskBarEx");
	add_ce_kernel_addr(0xf000be0c, "SH_GDI_RegisterDesktop");
	add_ce_kernel_addr(0xf000be08, "SH_GDI_GlobalAddAtomW");
	add_ce_kernel_addr(0xf000be04, "SH_GDI_GlobalDeleteAtom");
	add_ce_kernel_addr(0xf000be00, "SH_GDI_GlobalFindAtomW");
	add_ce_kernel_addr(0xf000bdfc, "SH_GDI_MonitorFromPoint_Trap");
	add_ce_kernel_addr(0xf000bdf8, "SH_GDI_MonitorFromRect_Trap");
	add_ce_kernel_addr(0xf000bdf4, "SH_GDI_MonitorFromWindow_Trap");
	add_ce_kernel_addr(0xf000bdf0, "SH_GDI_GetMonitorInfo_Trap");
	add_ce_kernel_addr(0xf000bdec, "SH_GDI_EnumDisplayMonitors_Trap");
	add_ce_kernel_addr(0xf000bde8, "SH_GDI_AccessibilitySoundSentryEvent_Trap");
	add_ce_kernel_addr(0xf000bde4, "SH_GDI_ChangeDisplaySettingsEx_Trap");
	add_ce_kernel_addr(0xf000bde0, "SH_GDI_InvalidateRgn_Trap");
	add_ce_kernel_addr(0xf000bddc, "SH_GDI_ValidateRgn_Trap");
	add_ce_kernel_addr(0xf000bdd8, "SH_GDI_ExtCreateRegion");
	add_ce_kernel_addr(0xf000bdd4, "SH_GDI_MoveToEx");
	add_ce_kernel_addr(0xf000bdd0, "SH_GDI_LineTo");
	add_ce_kernel_addr(0xf000bdcc, "SH_GDI_GetCurrentPositionEx");
	add_ce_kernel_addr(0xf000bdc8, "SH_GDI_SetTextAlign");
	add_ce_kernel_addr(0xf000bdc4, "SH_GDI_GetTextAlign");
	add_ce_kernel_addr(0xf000bdc0, "SH_GDI_GetCharWidth32");
	add_ce_kernel_addr(0xf000bdbc, "SH_GDI_GetDIBColorTable");
	add_ce_kernel_addr(0xf000bdb8, "SH_GDI_SetDIBColorTable");
	add_ce_kernel_addr(0xf000bdb4, "SH_GDI_StretchDIBits");
	add_ce_kernel_addr(0xf000bdb0, "SH_GDI_RedrawWindow_Trap");
	add_ce_kernel_addr(0xf000bdac, "SH_GDI_SetBitmapBits");
	add_ce_kernel_addr(0xf000bda8, "SH_GDI_SetDIBitsToDevice");
	add_ce_kernel_addr(0xf000bda4, "SH_GDI_GradientFill");
	add_ce_kernel_addr(0xf000bda0, "SH_GDI_InvertRect");
	add_ce_kernel_addr(0xf000bd9c, "SH_GDI_EnumDisplaySettings_Trap");
	add_ce_kernel_addr(0xf000bd98, "SH_GDI_EnumDisplayDevices_Trap");
	add_ce_kernel_addr(0xf000bd94, "SH_GDI_GetCharABCWidths");
	add_ce_kernel_addr(0xf000bd90, "SH_GDI_ShowStartupWindow_Trap");
	add_ce_kernel_addr(0xF000BD8C, "SH_GDI_GetGweApiSetTables");
	add_ce_kernel_addr(0xF000BD88, "SH_GDI_GetStretchBltMode");
	add_ce_kernel_addr(0xF000BD84, "SH_GDI_SetStretchBltMode");
	add_ce_kernel_addr(0xF000BD80, "SH_GDI_AlphaBlend");
	add_ce_kernel_addr(0xF000BD7C, "SH_GDI_GetIconInfo");
	add_ce_kernel_addr(0xF000BD78, "SH_GDI_EnumFontFamiliesExW");
	add_ce_kernel_addr(0xF000BD74, "SH_GDI_GetFontData");
	add_ce_kernel_addr(0xF000BD70, "SH_GDI_GetCharABCWidthsI");
	add_ce_kernel_addr(0xF000BD6C, "SH_GDI_GetOutlineTextMetricsW");
	add_ce_kernel_addr(0xF000BD68, "SH_GDI_SetLayout");
	add_ce_kernel_addr(0xF000BD64, "SH_GDI_GetLayout");
	add_ce_kernel_addr(0xF000BD60, "SH_GDI_SetTextCharacterExtra");
	add_ce_kernel_addr(0xF000BD5C, "SH_GDI_GetTextCharacterExtra");
	add_ce_kernel_addr(0xF000BD58, "SH_GDI_ImmGetKeyboardLayout");
	add_ce_kernel_addr(0xF000BD54, "SH_GDI_GetViewportOrgEx");
	add_ce_kernel_addr(0xF000BD50, "SH_GDI_GetViewportExtEx");
	add_ce_kernel_addr(0xF000BD4C, "SH_GDI_OffsetViewportOrgEx");
	add_ce_kernel_addr(0xF000BD48, "SH_GDI_GetROP2");
	add_ce_kernel_addr(0xF000BD44, "SH_GDI_SetWindowOrgEx");
	add_ce_kernel_addr(0xF000BD40, "SH_GDI_GetWindowOrgEx");
	add_ce_kernel_addr(0xF000BD3C, "SH_GDI_GetWindowExtEx");
	add_ce_kernel_addr(0xF000BD34, "SH_GDI_Gesture");
	add_ce_kernel_addr(0xF000BD30, "SH_GDI_GetWindowAutoGesture");
	add_ce_kernel_addr(0xF000BD2C, "SH_GDI_SetWindowAutoGesture");
	add_ce_kernel_addr(0xF000BD28, "SH_GDI_RegisterGesture");
	add_ce_kernel_addr(0xF000BD24, "SH_GDI_RegisterDefaultGestureHandler");
	add_ce_kernel_addr(0xF000BD20, "SH_GDI_SetLayeredWindowAttributes");
	add_ce_kernel_addr(0xF000BD1C, "SH_GDI_GetLayeredWindowAttributes");
	add_ce_kernel_addr(0xF000BD18, "SH_GDI_UpdateLayeredWindow");
	add_ce_kernel_addr(0xF000BD14, "SH_GDI_UpdateLayeredWindowIndirect");
	add_ce_kernel_addr(0xF000BD10, "SH_GDI_DrawThemePrimitive");
	add_ce_kernel_addr(0xF000BD0C, "SH_GDI_ThemePrimitiveExists");
	add_ce_kernel_addr(0xF000BD08, "SH_GDI_UpdateThemePrimitives");
	add_ce_kernel_addr(0xF000BD04, "SH_GDI_COREDLL_2872");


	add_ce_kernel_addr(0xf000bbf8, "SH_WMGR_RegisterClassWApiSetEntry_Trap");
	add_ce_kernel_addr(0xf000bbf4, "SH_WMGR_UnregisterClassW_Trap");
	add_ce_kernel_addr(0xf000bbf0, "SH_WMGR_CreateWindowExW_Trap");
	add_ce_kernel_addr(0xf000bbec, "SH_WMGR_PostMessageW");
	add_ce_kernel_addr(0xf000bbe8, "SH_WMGR_PostQuitMessage_Trap");
	add_ce_kernel_addr(0xf000bbe4, "SH_WMGR_SendMessageW");
	add_ce_kernel_addr(0xf000bbe0, "SH_WMGR_GetMessageW");
	add_ce_kernel_addr(0xf000bbdc, "SH_WMGR_TranslateMessage_Trap");
	add_ce_kernel_addr(0xf000bbd8, "SH_WMGR_DispatchMessageW");
	add_ce_kernel_addr(0xf000bbd4, "SH_WMGR_GetCapture_Trap");
	add_ce_kernel_addr(0xf000bbd0, "SH_WMGR_SetCapture_Trap");
	add_ce_kernel_addr(0xf000bbcc, "SH_WMGR_ReleaseCapture_Trap");
	add_ce_kernel_addr(0xf000bbc8, "SH_WMGR_SetWindowPos");
	add_ce_kernel_addr(0xf000bbc4, "SH_WMGR_GetWindowRect");
	add_ce_kernel_addr(0xf000bbc0, "SH_WMGR_GetClientRect");
	add_ce_kernel_addr(0xf000bbbc, "SH_WMGR_InvalidateRect");
	add_ce_kernel_addr(0xf000bbb8, "SH_WMGR_GetWindow");
	add_ce_kernel_addr(0xf000bbb4, "SH_WMGR_GetSystemMetrics");
	add_ce_kernel_addr(0xf000bbb0, "SH_WMGR_ImageList_GetDragImage");
	add_ce_kernel_addr(0xf000bbac, "SH_WMGR_ImageList_GetIconSize");
	add_ce_kernel_addr(0xf000bba8, "SH_WMGR_ImageList_SetIconSize");
	add_ce_kernel_addr(0xf000bba4, "SH_WMGR_ImageList_GetImageInfo");
	add_ce_kernel_addr(0xf000bba0, "SH_WMGR_ImageList_Merge");
	add_ce_kernel_addr(0xf000bb9c, "SH_WMGR_ShowCursor_Trap");
	add_ce_kernel_addr(0xf000bb98, "SH_WMGR_SetCursorPos_Trap");
	add_ce_kernel_addr(0xf000bb94, "SH_WMGR_ImageList_CopyDitherImage");
	add_ce_kernel_addr(0xf000bb90, "SH_WMGR_ImageList_DrawIndirect");
	add_ce_kernel_addr(0xf000bb8c, "SH_WMGR_ImageList_DragShowNolock");
	add_ce_kernel_addr(0xf000bb88, "SH_WMGR_WindowFromPoint_Trap");
	add_ce_kernel_addr(0xf000bb84, "SH_WMGR_ChildWindowFromPoint_Trap");
	add_ce_kernel_addr(0xf000bb80, "SH_WMGR_ClientToScreen");
	add_ce_kernel_addr(0xf000bb7c, "SH_WMGR_ScreenToClient_Trap");
	add_ce_kernel_addr(0xf000bb78, "SH_WMGR_SetWindowTextW_Trap");
	add_ce_kernel_addr(0xf000bb74, "SH_WMGR_GetWindowTextW_Trap");
	add_ce_kernel_addr(0xf000bb70, "SH_WMGR_SetWindowLongW");
	add_ce_kernel_addr(0xf000bb6c, "SH_WMGR_GetWindowLongW");
	add_ce_kernel_addr(0xf000bb68, "SH_WMGR_BeginPaint");
	add_ce_kernel_addr(0xf000bb64, "SH_WMGR_EndPaint");
	add_ce_kernel_addr(0xf000bb60, "SH_WMGR_GetDC");
	add_ce_kernel_addr(0xf000bb5c, "SH_WMGR_ReleaseDC");
	add_ce_kernel_addr(0xf000bb58, "SH_WMGR_DefWindowProcW");
	add_ce_kernel_addr(0xf000bb54, "SH_WMGR_GetClassLongW_Trap");
	add_ce_kernel_addr(0xf000bb50, "SH_WMGR_SetClassLongW_Trap");
	add_ce_kernel_addr(0xf000bb4c, "SH_WMGR_DestroyWindow_Trap");
	add_ce_kernel_addr(0xf000bb48, "SH_WMGR_ShowWindow_Trap");
	add_ce_kernel_addr(0xf000bb44, "SH_WMGR_UpdateWindow_Trap");
	add_ce_kernel_addr(0xf000bb40, "SH_WMGR_SetParent_Trap");
	add_ce_kernel_addr(0xf000bb3c, "SH_WMGR_GetParent");
	add_ce_kernel_addr(0xf000bb38, "SH_WMGR_MessageBoxW_Trap");
	add_ce_kernel_addr(0xf000bb34, "SH_WMGR_SetFocus_Trap");
	add_ce_kernel_addr(0xf000bb30, "SH_WMGR_GetFocus");
	add_ce_kernel_addr(0xf000bb2c, "SH_WMGR_GetActiveWindow_Trap");
	add_ce_kernel_addr(0xf000bb28, "SH_WMGR_GetWindowDC_Trap");
	add_ce_kernel_addr(0xf000bb24, "SH_WMGR_GetSysColor");
	add_ce_kernel_addr(0xf000bb20, "SH_WMGR_AdjustWindowRectEx_Trap");
	add_ce_kernel_addr(0xf000bb1c, "SH_WMGR_IsWindow");
	add_ce_kernel_addr(0xf000bb18, "SH_WMGR_CreatePopupMenu_Trap");
	add_ce_kernel_addr(0xf000bb14, "SH_WMGR_InsertMenuW_Trap");
	add_ce_kernel_addr(0xf000bb10, "SH_WMGR_AppendMenuW_Trap");
	add_ce_kernel_addr(0xf000bb0c, "SH_WMGR_RemoveMenu_Trap");
	add_ce_kernel_addr(0xf000bb08, "SH_WMGR_DestroyMenu_Trap");
	add_ce_kernel_addr(0xf000bb04, "SH_WMGR_TrackPopupMenuEx_Trap");
	add_ce_kernel_addr(0xf000bb00, "SH_WMGR_LoadMenuW_Trap");
	add_ce_kernel_addr(0xf000bafc, "SH_WMGR_EnableMenuItem_Trap");
	add_ce_kernel_addr(0xf000baf8, "SH_WMGR_MoveWindow_Trap");
	add_ce_kernel_addr(0xf000baf4, "SH_WMGR_GetUpdateRgn_Trap");
	add_ce_kernel_addr(0xf000baf0, "SH_WMGR_GetUpdateRect_Trap");
	add_ce_kernel_addr(0xf000baec, "SH_WMGR_BringWindowToTop_Trap");
	add_ce_kernel_addr(0xf000bae8, "SH_WMGR_GetWindowTextLengthW_Trap");
	add_ce_kernel_addr(0xf000bae4, "SH_WMGR_IsChild_Trap");
	add_ce_kernel_addr(0xf000bae0, "SH_WMGR_IsWindowVisible");
	add_ce_kernel_addr(0xf000badc, "SH_WMGR_ValidateRect_Trap");
	add_ce_kernel_addr(0xf000bad8, "SH_WMGR_LoadBitmapW_Trap");
	add_ce_kernel_addr(0xf000bad4, "SH_WMGR_CheckMenuItem_Trap");
	add_ce_kernel_addr(0xf000bad0, "SH_WMGR_CheckMenuRadioItem_Trap");
	add_ce_kernel_addr(0xf000bacc, "SH_WMGR_DeleteMenu");
	add_ce_kernel_addr(0xf000bac8, "SH_WMGR_LoadIconW_Trap");
	add_ce_kernel_addr(0xf000bac4, "SH_WMGR_DrawIconEx_Trap");
	add_ce_kernel_addr(0xf000bac0, "SH_WMGR_DestroyIcon_Trap");
	add_ce_kernel_addr(0xf000babc, "SH_WMGR_GetAsyncKeyState");
	add_ce_kernel_addr(0xf000bab8, "SH_WMGR_multitouch_event");
	add_ce_kernel_addr(0xf000bab4, "SH_WMGR_DialogBoxIndirectParamW");
	add_ce_kernel_addr(0xf000bab0, "SH_WMGR_EndDialog");
	add_ce_kernel_addr(0xf000baac, "SH_WMGR_GetDlgItem");
	add_ce_kernel_addr(0xf000baa8, "SH_WMGR_GetDlgCtrlID");
	add_ce_kernel_addr(0xf000baa4, "SH_WMGR_GetKeyState");
	add_ce_kernel_addr(0xf000baa0, "SH_WMGR_KeybdGetDeviceInfo");
	add_ce_kernel_addr(0xf000ba9c, "SH_WMGR_KeybdInitStates");
	add_ce_kernel_addr(0xf000ba98, "SH_WMGR_PostKeybdMessage");
	add_ce_kernel_addr(0xf000ba94, "SH_WMGR_KeybdVKeyToUnicode");
	add_ce_kernel_addr(0xf000ba90, "SH_WMGR_keybd_event");
	add_ce_kernel_addr(0xf000ba8c, "SH_WMGR_mouse_event");
	add_ce_kernel_addr(0xf000ba88, "SH_WMGR_SetScrollInfo");
	add_ce_kernel_addr(0xf000ba84, "SH_WMGR_SetScrollPos");
	add_ce_kernel_addr(0xf000ba80, "SH_WMGR_SetScrollRange");
	add_ce_kernel_addr(0xf000ba7c, "SH_WMGR_GetScrollInfo");
	add_ce_kernel_addr(0xf000ba78, "SH_WMGR_PeekMessageW");
	add_ce_kernel_addr(0xf000ba74, "SH_WMGR_MapVirtualKeyW");
	add_ce_kernel_addr(0xf000ba70, "SH_WMGR_GetMessageWNoWait");
	add_ce_kernel_addr(0xf000ba6c, "SH_WMGR_GetClassNameW");
	add_ce_kernel_addr(0xf000ba68, "SH_WMGR_MapWindowPoints");
	add_ce_kernel_addr(0xf000ba64, "SH_WMGR_LoadImageW");
	add_ce_kernel_addr(0xf000ba60, "SH_WMGR_GetForegroundWindow");
	add_ce_kernel_addr(0xf000ba5c, "SH_WMGR_SetForegroundWindow");
	add_ce_kernel_addr(0xf000ba58, "SH_WMGR_RegisterTaskBar");
	add_ce_kernel_addr(0xf000ba54, "SH_WMGR_SetActiveWindow");
	add_ce_kernel_addr(0xf000ba50, "SH_WMGR_CallWindowProcW_Trap");
	add_ce_kernel_addr(0xf000ba4c, "SH_WMGR_GetClassInfoW");
	add_ce_kernel_addr(0xf000ba48, "SH_WMGR_GetNextDlgTabItem");
	add_ce_kernel_addr(0xf000ba44, "SH_WMGR_CreateDialogIndirectParamW");
	add_ce_kernel_addr(0xf000ba40, "SH_WMGR_IsDialogMessageW");
	add_ce_kernel_addr(0xf000ba3c, "SH_WMGR_SetDlgItemInt");
	add_ce_kernel_addr(0xf000ba38, "SH_WMGR_GetDlgItemInt");
	add_ce_kernel_addr(0xf000ba34, "SH_WMGR_FindWindowW");
	add_ce_kernel_addr(0xf000ba30, "SH_WMGR_CreateCaret");
	add_ce_kernel_addr(0xf000ba2c, "SH_WMGR_DestroyCaret");
	add_ce_kernel_addr(0xf000ba28, "SH_WMGR_HideCaret");
	add_ce_kernel_addr(0xf000ba24, "SH_WMGR_ShowCaret");
	add_ce_kernel_addr(0xf000ba20, "SH_WMGR_SetCaretPos");
	add_ce_kernel_addr(0xf000ba1c, "SH_WMGR_GetCaretPos");
	add_ce_kernel_addr(0xf000ba18, "SH_WMGR_GetCursorPos_Trap");
	add_ce_kernel_addr(0xf000ba14, "SH_WMGR_ClipCursor");
	add_ce_kernel_addr(0xf000ba10, "SH_WMGR_GetClipCursor");
	add_ce_kernel_addr(0xf000ba0c, "SH_WMGR_GetCursor_Trap");
	add_ce_kernel_addr(0xf000ba08, "SH_WMGR_ExtractIconExW");
	add_ce_kernel_addr(0xf000ba04, "SH_WMGR_SetTimer_Trap");
	add_ce_kernel_addr(0xf000ba00, "SH_WMGR_KillTimer_Trap");
	add_ce_kernel_addr(0xf000b9fc, "SH_WMGR_GetNextDlgGroupItem");
	add_ce_kernel_addr(0xf000b9f8, "SH_WMGR_CheckRadioButton");
	add_ce_kernel_addr(0xf000b9f4, "SH_WMGR_EnableWindow");
	add_ce_kernel_addr(0xf000b9f0, "SH_WMGR_IsWindowEnabled");
	add_ce_kernel_addr(0xf000b9ec, "SH_WMGR_CreateMenu");
	add_ce_kernel_addr(0xf000b9e8, "SH_WMGR_GetSubMenu");
	add_ce_kernel_addr(0xf000b9e4, "SH_WMGR_DefDlgProcW");
	add_ce_kernel_addr(0xf000b9e0, "SH_WMGR_SendNotifyMessageW");
	add_ce_kernel_addr(0xf000b9dc, "SH_WMGR_PostThreadMessageW");
	add_ce_kernel_addr(0xf000b9d8, "SH_WMGR_TranslateAcceleratorW");
	add_ce_kernel_addr(0xf000b9d4, "SH_WMGR_GetKeyboardLayout");
	add_ce_kernel_addr(0xf000b9d0, "SH_WMGR_GetKeyboardLayoutList");
	add_ce_kernel_addr(0xf000b9cc, "SH_WMGR_GetKeyboardType");
	add_ce_kernel_addr(0xf000b9c8, "SH_WMGR_ImageList_Create");
	add_ce_kernel_addr(0xf000b9c4, "SH_WMGR_ImageList_Destroy");
	add_ce_kernel_addr(0xf000b9c0, "SH_WMGR_ImageList_GetImageCount");
	add_ce_kernel_addr(0xf000b9bc, "SH_WMGR_ImageList_Add");
	add_ce_kernel_addr(0xf000b9b8, "SH_WMGR_ImageList_ReplaceIcon");
	add_ce_kernel_addr(0xf000b9b4, "SH_WMGR_ImageList_SetBkColor");
	add_ce_kernel_addr(0xf000b9b0, "SH_WMGR_ImageList_GetBkColor");
	add_ce_kernel_addr(0xf000b9ac, "SH_WMGR_ImageList_SetOverlayImage");
	add_ce_kernel_addr(0xf000b9a8, "SH_WMGR_ImageList_Draw");
	add_ce_kernel_addr(0xf000b9a4, "SH_WMGR_ImageList_Replace");
	add_ce_kernel_addr(0xf000b9a0, "SH_WMGR_ImageList_AddMasked");
	add_ce_kernel_addr(0xf000b99c, "SH_WMGR_ImageList_DrawEx");
	add_ce_kernel_addr(0xf000b998, "SH_WMGR_ImageList_Remove");
	add_ce_kernel_addr(0xf000b994, "SH_WMGR_ImageList_GetIcon");
	add_ce_kernel_addr(0xf000b990, "SH_WMGR_ImageList_LoadImage");
	add_ce_kernel_addr(0xf000b98c, "SH_WMGR_ImageList_BeginDrag");
	add_ce_kernel_addr(0xf000b988, "SH_WMGR_ImageList_EndDrag");
	add_ce_kernel_addr(0xf000b984, "SH_WMGR_ImageList_DragEnter");
	add_ce_kernel_addr(0xf000b980, "SH_WMGR_ImageList_DragLeave");
	add_ce_kernel_addr(0xf000b97c, "SH_WMGR_ImageList_DragMove");
	add_ce_kernel_addr(0xf000b978, "SH_WMGR_ImageList_SetDragCursorImage");
	add_ce_kernel_addr(0xf000b974, "SH_WMGR_AudioUpdateFromRegistry");
	add_ce_kernel_addr(0xf000b970, "SH_WMGR_ScrollDC");
	add_ce_kernel_addr(0xf000b96c, "SH_WMGR_ScrollWindowEx");
	add_ce_kernel_addr(0xf000b968, "SH_WMGR_OpenClipboard");
	add_ce_kernel_addr(0xf000b964, "SH_WMGR_CloseClipboard");
	add_ce_kernel_addr(0xf000b960, "SH_WMGR_GetClipboardOwner");
	add_ce_kernel_addr(0xf000b95c, "SH_WMGR_SetClipboardData");
	add_ce_kernel_addr(0xf000b958, "SH_WMGR_GetClipboardDataGwe");
	add_ce_kernel_addr(0xf000b954, "SH_WMGR_RegisterClipboardFormatW");
	add_ce_kernel_addr(0xf000b950, "SH_WMGR_CountClipboardFormats");
	add_ce_kernel_addr(0xf000b94c, "SH_WMGR_EnumClipboardFormats");
	add_ce_kernel_addr(0xf000b948, "SH_WMGR_GetClipboardFormatNameW");
	add_ce_kernel_addr(0xf000b944, "SH_WMGR_EmptyClipboard");
	add_ce_kernel_addr(0xf000b940, "SH_WMGR_IsClipboardFormatAvailable");
	add_ce_kernel_addr(0xf000b93c, "SH_WMGR_GetPriorityClipboardFormat");
	add_ce_kernel_addr(0xf000b938, "SH_WMGR_GetOpenClipboardWindow");
	add_ce_kernel_addr(0xf000b934, "SH_WMGR_MessageBeep");
	add_ce_kernel_addr(0xf000b930, "SH_WMGR_SystemIdleTimerReset");
	add_ce_kernel_addr(0xf000b92c, "SH_WMGR_SystemIdleTimerUpdateMax");
	add_ce_kernel_addr(0xf000b928, "SH_WMGR_Unused182");
	add_ce_kernel_addr(0xf000b924, "SH_WMGR_SetKeyboardTarget");
	add_ce_kernel_addr(0xf000b920, "SH_WMGR_GetKeyboardTarget");
	add_ce_kernel_addr(0xf000b91c, "SH_WMGR_NotifyWinUserSystem");
	add_ce_kernel_addr(0xf000b918, "SH_WMGR_SetMenuItemInfoW");
	add_ce_kernel_addr(0xf000b914, "SH_WMGR_GetMenuItemInfoW");
	add_ce_kernel_addr(0xf000b910, "SH_WMGR_SetCaretBlinkTime");
	add_ce_kernel_addr(0xf000b90c, "SH_WMGR_GetCaretBlinkTime");
	add_ce_kernel_addr(0xf000b908, "SH_WMGR_GetMessagePos");
	add_ce_kernel_addr(0xf000b904, "SH_WMGR_QASetWindowsJournalHook");
	add_ce_kernel_addr(0xf000b900, "SH_WMGR_QAUnhookWindowsJournalHook");
	add_ce_kernel_addr(0xf000b8fc, "SH_WMGR_NLedGetDeviceInfo");
	add_ce_kernel_addr(0xf000b8f8, "SH_WMGR_NLedSetDevice");
	add_ce_kernel_addr(0xf000b8f4, "SH_WMGR_EnumWindows_Trap");
	add_ce_kernel_addr(0xf000b8f0, "SH_WMGR_RectangleAnimation");
	add_ce_kernel_addr(0xf000b8ec, "SH_WMGR_MapDialogRect");
	add_ce_kernel_addr(0xf000b8e8, "SH_WMGR_GetSystemPowerStatusEx");
	add_ce_kernel_addr(0xf000b8e4, "SH_WMGR_GetDialogBaseUnits");
	add_ce_kernel_addr(0xf000b8e0, "SH_WMGR_GetDoubleClickTime");
	add_ce_kernel_addr(0xf000b8dc, "SH_WMGR_GetWindowThreadProcessId");
	add_ce_kernel_addr(0xf000b8d8, "SH_WMGR_CreateIconIndirect");
	add_ce_kernel_addr(0xf000b8d4, "SH_WMGR_ShellModalEnd");
	add_ce_kernel_addr(0xf000b8d0, "SH_WMGR_TouchCalibrate");
	add_ce_kernel_addr(0xf000b8cc, "SH_WMGR_BatteryGetLifeTimeInfo");
	add_ce_kernel_addr(0xf000b8c8, "SH_WMGR_BatteryDrvrGetLevels");
	add_ce_kernel_addr(0xf000b8c4, "SH_WMGR_GwesPowerOffSystem");
	add_ce_kernel_addr(0xf000b8c0, "SH_WMGR_BatteryNotifyOfTimeChange");
	add_ce_kernel_addr(0xf000b8bc, "SH_WMGR_LoadCursorW_Trap");
	add_ce_kernel_addr(0xf000b8b8, "SH_WMGR_SetCursor");
	add_ce_kernel_addr(0xf000b8b4, "SH_WMGR_DestroyCursor_Trap");
	add_ce_kernel_addr(0xf000b8b0, "SH_WMGR_DisableCaretSystemWide");
	add_ce_kernel_addr(0xf000b8ac, "SH_WMGR_EnableCaretSystemWide");
	add_ce_kernel_addr(0xf000b8a8, "SH_WMGR_GetMouseMovePoints");
	add_ce_kernel_addr(0xf000b8a4, "SH_WMGR_BatteryDrvrSupportsChangeNotification");
	add_ce_kernel_addr(0xf000b8a0, "SH_WMGR_EnableHardwareKeyboard");
	add_ce_kernel_addr(0xf000b89c, "SH_WMGR_GetKeyboardStatus");
	add_ce_kernel_addr(0xf000b898, "SH_WMGR_RegisterSIPanel");
	add_ce_kernel_addr(0xf000b894, "SH_WMGR_GetAsyncShiftFlags");
	add_ce_kernel_addr(0xf000b890, "SH_WMGR_MsgWaitForMultipleObjectsEx");
	add_ce_kernel_addr(0xf000b88c, "SH_WMGR_SetAssociatedMenu");
	add_ce_kernel_addr(0xf000b888, "SH_WMGR_GetAssociatedMenu");
	add_ce_kernel_addr(0xf000b884, "SH_WMGR_DrawMenuBar");
	add_ce_kernel_addr(0xf000b880, "SH_WMGR_SetSysColors");
	add_ce_kernel_addr(0xf000b87c, "SH_WMGR_DrawFrameControl");
	add_ce_kernel_addr(0xf000b878, "SH_WMGR_CreateCursor_Trap");
	add_ce_kernel_addr(0xf000b874, "SH_WMGR_RegisterWindowMessageW");
	add_ce_kernel_addr(0xf000b870, "SH_WMGR_SystemParametersInfo_GWE_Trap");
	add_ce_kernel_addr(0xf000b86c, "SH_WMGR_SendInput");
	add_ce_kernel_addr(0xf000b868, "SH_WMGR_SendDlgItemMessageW");
	add_ce_kernel_addr(0xf000b864, "SH_WMGR_SetDlgItemTextW");
	add_ce_kernel_addr(0xf000b860, "SH_WMGR_GetDlgItemTextW");
	add_ce_kernel_addr(0xf000b85c, "SH_WMGR_GetMessageSource_Trap");
	add_ce_kernel_addr(0xf000b858, "SH_WMGR_RegisterHotKey_Trap");
	add_ce_kernel_addr(0xf000b854, "SH_WMGR_UnregisterHotKey_Trap");
	add_ce_kernel_addr(0xf000b850, "SH_WMGR_ImageList_Copy");
	add_ce_kernel_addr(0xf000b84c, "SH_WMGR_ImageList_Duplicate");
	add_ce_kernel_addr(0xf000b848, "SH_WMGR_ImageList_SetImageCount");
	add_ce_kernel_addr(0xf000b844, "SH_WMGR_UnregisterFunc1_Trap");
	add_ce_kernel_addr(0xf000b840, "SH_WMGR_ImmGetContextFromWindowGwe");
	add_ce_kernel_addr(0xf000b83c, "SH_WMGR_ImmAssociateContextWithWindowGwe");
	add_ce_kernel_addr(0xf000b838, "SH_WMGR_ImmSetHotKey");
	add_ce_kernel_addr(0xf000b834, "SH_WMGR_BeginDeferWindowPos_Trap");
	add_ce_kernel_addr(0xf000b830, "SH_WMGR_DeferWindowPos_Trap");
	add_ce_kernel_addr(0xf000b82c, "SH_WMGR_EndDeferWindowPos_Trap");
	add_ce_kernel_addr(0xf000b828, "SH_WMGR_ImmGetHotKey");
	add_ce_kernel_addr(0xf000b824, "SH_WMGR_GetDCEx_Trap");
	add_ce_kernel_addr(0xf000b820, "SH_WMGR_GwesPowerDown");
	add_ce_kernel_addr(0xf000b81c, "SH_WMGR_GwesPowerUp");
	add_ce_kernel_addr(0xf000b810, "SH_WMGR_LoadKeyboardLayoutW");
	add_ce_kernel_addr(0xf000b80c, "SH_WMGR_ActivateKeyboardLayout");
	add_ce_kernel_addr(0xf000b808, "SH_WMGR_GetSystemPowerStatusEx2");
	add_ce_kernel_addr(0xf000b804, "SH_WMGR_GetKeyboardLayoutNameW");
	add_ce_kernel_addr(0xf000b7fc, "SH_WNET_WNetAddConnection3W");
	add_ce_kernel_addr(0xf000b7f8, "SH_WNET_WNetCancelConnection2W");
	add_ce_kernel_addr(0xf000b7f4, "SH_WNET_WNetConnectionDialog1W");
	add_ce_kernel_addr(0xf000b7f0, "SH_WNET_WNetDisconnectDialog");
	add_ce_kernel_addr(0xf000b7ec, "SH_WNET_WNetDisconnectDialog1W");
	add_ce_kernel_addr(0xf000b7e8, "SH_WNET_WNetGetConnectionW");
	add_ce_kernel_addr(0xf000b7e4, "SH_WNET_WNetGetUniversalNameW");
	add_ce_kernel_addr(0xf000b7e0, "SH_WNET_WNetGetUserW");
	add_ce_kernel_addr(0xf000b7dc, "SH_WNET_WNetOpenEnumW");
	add_ce_kernel_addr(0xf000b3f8, "SH_COMM_AFDSocket");
	add_ce_kernel_addr(0xf000b3f4, "SH_COMM_AFDControl");
	add_ce_kernel_addr(0xf000b3f0, "SH_COMM_AFDEnumProtocolsW");
	add_ce_kernel_addr(0xf000b3ec, "SH_COMM_RasDial");
	add_ce_kernel_addr(0xf000b3e8, "SH_COMM_RasHangup");
	add_ce_kernel_addr(0xf000b3e4, "SH_COMM_AFDGetHostentByAttr");
	add_ce_kernel_addr(0xf000b3e0, "SH_COMM_AFDAddIPHostent");
	add_ce_kernel_addr(0xf000b3dc, "SH_COMM_RasIOControl");
	add_ce_kernel_addr(0xf000b3d8, "SH_COMM_AFDSelect");
	add_ce_kernel_addr(0xf000b3d4, "SH_COMM_RasEnumEntries");
	add_ce_kernel_addr(0xf000b3d0, "SH_COMM_RasGetEntryDialParams");
	add_ce_kernel_addr(0xf000b3cc, "SH_COMM_RasSetEntryDialParams");
	add_ce_kernel_addr(0xf000b3c8, "SH_COMM_RasGetEntryProperties");
	add_ce_kernel_addr(0xf000b3c4, "SH_COMM_RasSetEntryProperties");
	add_ce_kernel_addr(0xf000b3c0, "SH_COMM_RasValidateEntryName");
	add_ce_kernel_addr(0xf000b3bc, "SH_COMM_RasDeleteEntry");
	add_ce_kernel_addr(0xf000b3b8, "SH_COMM_RasRenameEntry");
	add_ce_kernel_addr(0xf000b3b4, "SH_COMM_AFDAddInterface");
	add_ce_kernel_addr(0xf000b3b0, "SH_COMM_RasEnumConnections");
	add_ce_kernel_addr(0xf000b3ac, "SH_COMM_RasGetConnectStatus");
	add_ce_kernel_addr(0xf000b3a8, "SH_COMM_RasGetEntryDevConfig");
	add_ce_kernel_addr(0xf000b3a4, "SH_COMM_RasSetEntryDevConfig");
	add_ce_kernel_addr(0xf000b3a0, "SH_COMM_NETbios");
	add_ce_kernel_addr(0xf000b398, "SH_COMM_PMInstallProvider");
	add_ce_kernel_addr(0xf000b394, "SH_COMM_PMEnumProtocols");
	add_ce_kernel_addr(0xf000b390, "SH_COMM_PMFindProvider");
	add_ce_kernel_addr(0xf000b38c, "SH_COMM_PMInstallNameSpace");
	add_ce_kernel_addr(0xf000b388, "SH_COMM_PMEnumNameSpaceProviders");
	add_ce_kernel_addr(0xf000b384, "SH_COMM_PMFindNameSpaces");
	add_ce_kernel_addr(0xf000b380, "SH_COMM_PMAddrConvert");
	add_ce_kernel_addr(0xf000aff8, "SH_FILESYS_APIS_CreateDirectoryW");
	add_ce_kernel_addr(0xf000aff4, "SH_FILESYS_APIS_RemoveDirectoryW");
	add_ce_kernel_addr(0xf000aff0, "SH_FILESYS_APIS_MoveFileW");
	add_ce_kernel_addr(0xf000afec, "SH_FILESYS_APIS_CopyFileW");
	add_ce_kernel_addr(0xf000afe8, "SH_FILESYS_APIS_DeleteFileW");
	add_ce_kernel_addr(0xf000afe4, "SH_FILESYS_APIS_GetFileAttributesW");
	add_ce_kernel_addr(0xf000afe0, "SH_FILESYS_APIS_FindFirstFileW");
	add_ce_kernel_addr(0xf000afdc, "SH_FILESYS_APIS_CreateFileW");
	add_ce_kernel_addr(0xf000afd8, "SH_FILESYS_APIS_CeRegisterFileSystemNotification");
	add_ce_kernel_addr(0xf000afd4, "SH_FILESYS_APIS_CeRegisterReplNotification");
	add_ce_kernel_addr(0xf000afd0, "SH_FILESYS_APIS_CeOidGetInfoEx2");
	add_ce_kernel_addr(0xf000afcc, "SH_FILESYS_APIS_CeFindFirstDatabaseEx");
	add_ce_kernel_addr(0xf000afc8, "SH_FILESYS_APIS_CeCreateDatabaseEx2");
	add_ce_kernel_addr(0xf000afc4, "SH_FILESYS_APIS_CeSetDatabaseInfoEx2");
	add_ce_kernel_addr(0xf000afc0, "SH_FILESYS_APIS_CeOpenDatabaseEx2");
	add_ce_kernel_addr(0xf000afbc, "SH_FILESYS_APIS_RegCloseKey");
	add_ce_kernel_addr(0xf000afb8, "SH_FILESYS_APIS_RegCreateKeyExW");
	add_ce_kernel_addr(0xf000afb4, "SH_FILESYS_APIS_RegDeleteKeyW");
	add_ce_kernel_addr(0xf000afb0, "SH_FILESYS_APIS_RegDeleteValueW");
	add_ce_kernel_addr(0xf000afac, "SH_FILESYS_APIS_RegEnumValueW");
	add_ce_kernel_addr(0xf000afa8, "SH_FILESYS_APIS_RegEnumKeyExW");
	add_ce_kernel_addr(0xf000afa4, "SH_FILESYS_APIS_RegOpenKeyExW");
	add_ce_kernel_addr(0xf000afa0, "SH_FILESYS_APIS_RegQueryInfoKeyW");
	add_ce_kernel_addr(0xf000af9c, "SH_FILESYS_APIS_RegQueryValueExW");
	add_ce_kernel_addr(0xf000af98, "SH_FILESYS_APIS_RegSetValueExW");
	add_ce_kernel_addr(0xf000af94, "SH_FILESYS_APIS_GetTempPathW");
	add_ce_kernel_addr(0xf000af90, "SH_FILESYS_APIS_CeDeleteDatabaseEx");
	add_ce_kernel_addr(0xf000af8c, "SH_FILESYS_APIS_CheckPassword");
	add_ce_kernel_addr(0xf000af88, "SH_FILESYS_APIS_SetPassword");
	add_ce_kernel_addr(0xf000af84, "SH_FILESYS_APIS_SetFileAttributesW");
	add_ce_kernel_addr(0xf000af80, "SH_FILESYS_APIS_GetStoreInformation");
	add_ce_kernel_addr(0xf000af7c, "SH_FILESYS_APIS_CeGetReplChangeMask");
	add_ce_kernel_addr(0xf000af78, "SH_FILESYS_APIS_CeSetReplChangeMask");
	add_ce_kernel_addr(0xf000af74, "SH_FILESYS_APIS_CeGetReplChangeBitsEx");
	add_ce_kernel_addr(0xf000af70, "SH_FILESYS_APIS_CeClearReplChangeBitsEx");
	add_ce_kernel_addr(0xf000af6c, "SH_FILESYS_APIS_CeGetReplOtherBitsEx");
	add_ce_kernel_addr(0xf000af68, "SH_FILESYS_APIS_CeSetReplOtherBitsEx");
	add_ce_kernel_addr(0xf000af64, "SH_FILESYS_APIS_GetSystemMemoryDivision");
	add_ce_kernel_addr(0xf000af60, "SH_FILESYS_APIS_SetSystemMemoryDivision");
	add_ce_kernel_addr(0xf000af5c, "SH_FILESYS_APIS_RegCopyFile");
	add_ce_kernel_addr(0xf000af58, "SH_FILESYS_APIS_CloseAllFileHandles");
	add_ce_kernel_addr(0xf000af54, "SH_FILESYS_APIS_DeleteAndRenameFile");
	add_ce_kernel_addr(0xf000af50, "SH_FILESYS_APIS_RegRestoreFile");
	add_ce_kernel_addr(0xf000af4c, "SH_FILESYS_APIS_RegisterAFSEx");
	add_ce_kernel_addr(0xf000af48, "SH_FILESYS_APIS_DeregisterAFS");
	add_ce_kernel_addr(0xf000af44, "SH_FILESYS_APIS_GetPasswordActive");
	add_ce_kernel_addr(0xf000af40, "SH_FILESYS_APIS_SetPasswordActive");
	add_ce_kernel_addr(0xf000af3c, "SH_FILESYS_APIS_RegFlushKey");
	add_ce_kernel_addr(0xf000af38, "SH_FILESYS_APIS_FileSystemPowerFunction");
	add_ce_kernel_addr(0xf000af34, "SH_FILESYS_APIS_CeSetReplChangeBitsEx");
	add_ce_kernel_addr(0xf000af30, "SH_FILESYS_APIS_RegisterAFSName");
	add_ce_kernel_addr(0xf000af2c, "SH_FILESYS_APIS_DeregisterAFSName");
	add_ce_kernel_addr(0xf000af28, "SH_FILESYS_APIS_GetDiskFreeSpaceExW");
	add_ce_kernel_addr(0xf000af24, "SH_FILESYS_APIS_IsSystemFile");
	add_ce_kernel_addr(0xf000af20, "SH_FILESYS_APIS_CeChangeDatabaseLCID");
	add_ce_kernel_addr(0xf000af1c, "SH_FILESYS_APIS_DumpFileSystemHeap");
	add_ce_kernel_addr(0xf000af18, "SH_FILESYS_APIS_CeMountDBVol");
	add_ce_kernel_addr(0xf000af14, "SH_FILESYS_APIS_CeEnumDBVolumes");
	add_ce_kernel_addr(0xf000af10, "SH_FILESYS_APIS_CeUnmountDBVol");
	add_ce_kernel_addr(0xf000af0c, "SH_FILESYS_APIS_CeFlushDBVol");
	add_ce_kernel_addr(0xf000af08, "SH_FILESYS_APIS_CeFreeNotification");
	add_ce_kernel_addr(0xf000af04, "SH_FILESYS_APIS_FindFirstFileExW");
	add_ce_kernel_addr(0xf000af00, "SH_FILESYS_APIS_RegSaveKey");
	add_ce_kernel_addr(0xf000aefc, "SH_FILESYS_APIS_RegReplaceKey");
	add_ce_kernel_addr(0xf000aef8, "SH_FILESYS_APIS_SignalStarted");
	add_ce_kernel_addr(0xf000aef4, "SH_FILESYS_APIS_SetCurrentUser");
	add_ce_kernel_addr(0xf000aef0, "SH_FILESYS_APIS_SetUserData");
	add_ce_kernel_addr(0xf000aeec, "SH_FILESYS_APIS_GetUserInformation");
	add_ce_kernel_addr(0xf000aee8, "SH_FILESYS_APIS_SetPasswordStatus");
	add_ce_kernel_addr(0xf000aee4, "SH_FILESYS_APIS_GetPasswordStatus");
	add_ce_kernel_addr(0xf000aee0, "SH_FILESYS_APIS_ReplOpenSync");
	add_ce_kernel_addr(0xf000aedc, "SH_FILESYS_APIS_ReplCheckpoint");
	add_ce_kernel_addr(0xf000aed8, "SH_FILESYS_APIS_ReplCloseSync");
	add_ce_kernel_addr(0xf000aed4, "SH_FILESYS_APIS_ReplGetSyncState");
	add_ce_kernel_addr(0xf000aed0, "SH_FILESYS_APIS_ReplChangeSyncSettings");
	add_ce_kernel_addr(0xf000aecc, "SH_FILESYS_APIS_ReplFindNextChange");
	add_ce_kernel_addr(0xf000aec8, "SH_FILESYS_APIS_ReplGetOidStatus");
	add_ce_kernel_addr(0xf000aec4, "SH_FILESYS_APIS_CreateMsgQueue");
	add_ce_kernel_addr(0xf000aec0, "SH_FILESYS_APIS_OpenMsgQueue");
	add_ce_kernel_addr(0xf000aebc, "SH_FILESYS_APIS_ReadMsgQueue");
	add_ce_kernel_addr(0xf000aeb8, "SH_FILESYS_APIS_WriteMsgQueue");
	add_ce_kernel_addr(0xf000aeb4, "SH_FILESYS_APIS_GetMsgQueueInfo");
	add_ce_kernel_addr(0xf000aeb0, "SH_FILESYS_APIS_CloseMsgQueue");
	add_ce_kernel_addr(0xf000aeac, "SH_FILESYS_APIS_CryptProtectData");
	add_ce_kernel_addr(0xf000aea8, "SH_FILESYS_APIS_CryptUnprotectData");
	add_ce_kernel_addr(0xf000aea4, "SH_FILESYS_APIS_GenRandom");
	add_ce_kernel_addr(0xf000aea0, "SH_FILESYS_APIS_FindFirstChangeNotificationW");
	add_ce_kernel_addr(0xf000ae9c, "SH_FILESYS_APIS_FindNextChangeNotification");
	add_ce_kernel_addr(0xf000ae98, "SH_FILESYS_APIS_FindCloseChangeNotification");
	add_ce_kernel_addr(0xf000ae94, "SH_FILESYS_APIS_CeGetFileNotificationInfo");
	add_ce_kernel_addr(0xf000abf4, "SH_SHELL_GetOpenFileNameW");
	add_ce_kernel_addr(0xf000abf0, "SH_SHELL_GetSaveFileNameW");
	add_ce_kernel_addr(0xf000abe8, "SH_SHELL_Shell_NotifyIcon");
	add_ce_kernel_addr(0xf000abe4, "SH_SHELL_SHAddToRecentDocs");
	add_ce_kernel_addr(0xf000abdc, "SH_SHELL_SHCreateExplorerInstance");
	add_ce_kernel_addr(0xf000ab94, "SH_SHELL_NotSystemParametersInfo_Trap");
	add_ce_kernel_addr(0xf000ab90, "SH_SHELL_SHGetAppKeyAssoc_Trap");
	add_ce_kernel_addr(0xf000ab8c, "SH_SHELL_SHSetAppKeyWndAssoc_Trap");
	add_ce_kernel_addr(0xf000ab78, "SH_SHELL_SHFileNotifyRemove_Trap");
	add_ce_kernel_addr(0xf000ab74, "SH_SHELL_SHFileNotifyFree_Trap");
	add_ce_kernel_addr(0xf000ab6c, "SH_SHELL_SHCloseApps_Trap");
	add_ce_kernel_addr(0xf000ab68, "SH_SHELL_SHSipPreference_Trap");
	add_ce_kernel_addr(0xf000ab5c, "SH_SHELL_SHSetNavBarText_Trap");
	add_ce_kernel_addr(0xf000ab58, "SH_SHELL_SHDoneButton_Trap");
	add_ce_kernel_addr(0xf000ab38, "SH_SHELL_SHChangeNotifyRegister_Trap");
	add_ce_kernel_addr(0xf000ab24, "SH_SHELL_SHNotificationAdd_Trap");
	add_ce_kernel_addr(0xf000ab20, "SH_SHELL_SHNotificationUpdate_Trap");
	add_ce_kernel_addr(0xf000ab1c, "SH_SHELL_SHNotificationRemove_Trap");
	add_ce_kernel_addr(0xf000ab18, "SH_SHELL_SHNotificationGetData_Trap");
	add_ce_kernel_addr(0xf000aadc, "SH_SHELL_SendChangeNotificationToWindow_Trap");
	add_ce_kernel_addr(0xf000a7f8, "SH_DEVMGR_APIS_RegisterDevice");
	add_ce_kernel_addr(0xf000a7f4, "SH_DEVMGR_APIS_DeregisterDevice");
	add_ce_kernel_addr(0xf000a7f0, "SH_DEVMGR_APIS_CloseAllDeviceHandles");
	add_ce_kernel_addr(0xf000a7ec, "SH_DEVMGR_APIS_CreateDeviceHandle");
	add_ce_kernel_addr(0xf000a7e8, "SH_DEVMGR_APIS_LoadFSD");
	add_ce_kernel_addr(0xf000a7e0, "SH_DEVMGR_APIS_DeactivateDevice");
	add_ce_kernel_addr(0xf000a7dc, "SH_DEVMGR_APIS_LoadFSDEx");
	add_ce_kernel_addr(0xf000a7d8, "SH_DEVMGR_APIS_GetDeviceByIndex");
	add_ce_kernel_addr(0xf000a7d4, "SH_DEVMGR_APIS_CeResyncFilesys");
	add_ce_kernel_addr(0xf000a7d0, "SH_DEVMGR_APIS_ActivateDeviceEx");
	add_ce_kernel_addr(0xf000a7cc, "SH_DEVMGR_APIS_RequestDeviceNotifications");
	add_ce_kernel_addr(0xf000a7c8, "SH_DEVMGR_APIS_StopDeviceNotifications");
	add_ce_kernel_addr(0xf000a7c4, "SH_DEVMGR_APIS__GetDevicePathFromPnp");
	add_ce_kernel_addr(0xf000a7c0, "SH_DEVMGR_APIS_ResourceCreateList");
	add_ce_kernel_addr(0xf000a7bc, "SH_DEVMGR_APIS_ResourceAdjust");
	add_ce_kernel_addr(0xf000a7b8, "SH_DEVMGR_APIS_GetSystemPowerState");
	add_ce_kernel_addr(0xf000a7b4, "SH_DEVMGR_APIS_SetSystemPowerState");
	add_ce_kernel_addr(0xf000a7b0, "SH_DEVMGR_APIS_SetPowerRequirement");
	add_ce_kernel_addr(0xf000a7ac, "SH_DEVMGR_APIS_ReleasePowerRequirement");
	add_ce_kernel_addr(0xf000a7a8, "SH_DEVMGR_APIS_RequestPowerNotifications");
	add_ce_kernel_addr(0xf000a7a4, "SH_DEVMGR_APIS_StopPowerNotifications");
	add_ce_kernel_addr(0xf000a79c, "SH_DEVMGR_APIS_DevicePowerNotify");
	add_ce_kernel_addr(0xf000a798, "SH_DEVMGR_APIS_RegisterPowerRelationship");
	add_ce_kernel_addr(0xf000a794, "SH_DEVMGR_APIS_ReleasePowerRelationship");
	add_ce_kernel_addr(0xf000a790, "SH_DEVMGR_APIS_SetDevicePower");
	add_ce_kernel_addr(0xf000a78c, "SH_DEVMGR_APIS_GetDevicePower");
	add_ce_kernel_addr(0xf000a788, "SH_DEVMGR_APIS_AdvertiseInterface");
	add_ce_kernel_addr(0xf000a3f8, "SH_TAPI_lineClose");
	add_ce_kernel_addr(0xf000a3f4, "SH_TAPI_lineConfigDialogEdit");
	add_ce_kernel_addr(0xf000a3f0, "SH_TAPI_lineDeallocateCall");
	add_ce_kernel_addr(0xf000a3ec, "SH_TAPI_lineDrop");
	add_ce_kernel_addr(0xf000a3e8, "SH_TAPI_lineGetDevCaps");
	add_ce_kernel_addr(0xf000a3e4, "SH_TAPI_lineGetDevConfig");
	add_ce_kernel_addr(0xf000a3e0, "SH_TAPI_lineGetTranslateCaps");
	add_ce_kernel_addr(0xf000a3dc, "SH_TAPI_TAPIlineInitialize");
	add_ce_kernel_addr(0xf000a3d8, "SH_TAPI_lineMakeCall");
	add_ce_kernel_addr(0xf000a3d4, "SH_TAPI_lineNegotiateAPIVersion");
	add_ce_kernel_addr(0xf000a3d0, "SH_TAPI_lineOpen");
	add_ce_kernel_addr(0xf000a3cc, "SH_TAPI_lineSetDevConfig");
	add_ce_kernel_addr(0xf000a3c8, "SH_TAPI_lineSetStatusMessages");
	add_ce_kernel_addr(0xf000a3c4, "SH_TAPI_TAPIlineShutdown");
	add_ce_kernel_addr(0xf000a3c0, "SH_TAPI_lineTranslateAddress");
	add_ce_kernel_addr(0xf000a3bc, "SH_TAPI_lineTranslateDialog");
	add_ce_kernel_addr(0xf000a3b8, "SH_TAPI_lineGetID");
	add_ce_kernel_addr(0xf000a3b4, "SH_TAPI_lineAddProvider");
	add_ce_kernel_addr(0xf000a3b0, "SH_TAPI_lineSetCurrentLocation");
	add_ce_kernel_addr(0xf000a3ac, "SH_TAPI_lineAccept");
	add_ce_kernel_addr(0xf000a3a8, "SH_TAPI_lineAddToConference");
	add_ce_kernel_addr(0xf000a3a4, "SH_TAPI_lineAnswer");
	add_ce_kernel_addr(0xf000a3a0, "SH_TAPI_lineBlindTransfer");
	add_ce_kernel_addr(0xf000a39c, "SH_TAPI_lineCompleteTransfer");
	add_ce_kernel_addr(0xf000a398, "SH_TAPI_lineDevSpecific");
	add_ce_kernel_addr(0xf000a394, "SH_TAPI_lineDial");
	add_ce_kernel_addr(0xf000a390, "SH_TAPI_lineForward");
	add_ce_kernel_addr(0xf000a38c, "SH_TAPI_lineGenerateDigits");
	add_ce_kernel_addr(0xf000a388, "SH_TAPI_lineGenerateTone");
	add_ce_kernel_addr(0xf000a384, "SH_TAPI_lineGetAddressCaps");
	add_ce_kernel_addr(0xf000a380, "SH_TAPI_lineGetAddressID");
	add_ce_kernel_addr(0xf000a37c, "SH_TAPI_lineGetAddressStatus");
	add_ce_kernel_addr(0xf000a378, "SH_TAPI_lineGetAppPriority");
	add_ce_kernel_addr(0xf000a374, "SH_TAPI_lineGetCallInfo");
	add_ce_kernel_addr(0xf000a370, "SH_TAPI_lineGetCallStatus");
	add_ce_kernel_addr(0xf000a36c, "SH_TAPI_lineGetConfRelatedCalls");
	add_ce_kernel_addr(0xf000a368, "SH_TAPI_lineGetIcon");
	add_ce_kernel_addr(0xf000a364, "SH_TAPI_lineGetLineDevStatus");
	add_ce_kernel_addr(0xf000a360, "SH_TAPI_lineGetMessage");
	add_ce_kernel_addr(0xf000a35c, "SH_TAPI_lineGetNewCalls");
	add_ce_kernel_addr(0xf000a358, "SH_TAPI_lineGetNumRings");
	add_ce_kernel_addr(0xf000a354, "SH_TAPI_lineGetProviderList");
	add_ce_kernel_addr(0xf000a350, "SH_TAPI_lineGetStatusMessages");
	add_ce_kernel_addr(0xf000a34c, "SH_TAPI_lineHandoff");
	add_ce_kernel_addr(0xf000a348, "SH_TAPI_lineHold");
	add_ce_kernel_addr(0xf000a344, "SH_TAPI_TAPIlineInitializeEx");
	add_ce_kernel_addr(0xf000a340, "SH_TAPI_lineMonitorDigits");
	add_ce_kernel_addr(0xf000a33c, "SH_TAPI_lineMonitorMedia");
	add_ce_kernel_addr(0xf000a338, "SH_TAPI_lineNegotiateExtVersion");
	add_ce_kernel_addr(0xf000a334, "SH_TAPI_linePickup");
	add_ce_kernel_addr(0xf000a330, "SH_TAPI_linePrepareAddToConference");
	add_ce_kernel_addr(0xf000a32c, "SH_TAPI_lineRedirect");
	add_ce_kernel_addr(0xf000a328, "SH_TAPI_lineReleaseUserUserInfo");
	add_ce_kernel_addr(0xf000a324, "SH_TAPI_lineRemoveFromConference");
	add_ce_kernel_addr(0xf000a320, "SH_TAPI_lineSendUserUserInfo");
	add_ce_kernel_addr(0xf000a31c, "SH_TAPI_lineSetAppPriority");
	add_ce_kernel_addr(0xf000a318, "SH_TAPI_lineSetCallParams");
	add_ce_kernel_addr(0xf000a314, "SH_TAPI_lineSetCallPrivilege");
	add_ce_kernel_addr(0xf000a310, "SH_TAPI_lineSetMediaMode");
	add_ce_kernel_addr(0xf000a30c, "SH_TAPI_lineSetNumRings");
	add_ce_kernel_addr(0xf000a308, "SH_TAPI_lineSetTerminal");
	add_ce_kernel_addr(0xf000a304, "SH_TAPI_lineSetTollList");
	add_ce_kernel_addr(0xf000a300, "SH_TAPI_lineSetupConference");
	add_ce_kernel_addr(0xf000a2fc, "SH_TAPI_lineSetupTransfer");
	add_ce_kernel_addr(0xf000a2f8, "SH_TAPI_lineSwapHold");
	add_ce_kernel_addr(0xf000a2f4, "SH_TAPI_lineUnhold");
	add_ce_kernel_addr(0xf000a2f0, "SH_TAPI_phoneClose");
	add_ce_kernel_addr(0xf000a2ec, "SH_TAPI_phoneConfigDialog");
	add_ce_kernel_addr(0xf000a2e8, "SH_TAPI_phoneDevSpecific");
	add_ce_kernel_addr(0xf000a2e4, "SH_TAPI_phoneGetDevCaps");
	add_ce_kernel_addr(0xf000a2e0, "SH_TAPI_phoneGetGain");
	add_ce_kernel_addr(0xf000a2dc, "SH_TAPI_phoneGetHookSwitch");
	add_ce_kernel_addr(0xf000a2d8, "SH_TAPI_phoneGetIcon");
	add_ce_kernel_addr(0xf000a2d4, "SH_TAPI_phoneGetID");
	add_ce_kernel_addr(0xf000a2d0, "SH_TAPI_phoneGetMessage");
	add_ce_kernel_addr(0xf000a2cc, "SH_TAPI_phoneGetRing");
	add_ce_kernel_addr(0xf000a2c8, "SH_TAPI_phoneGetStatus");
	add_ce_kernel_addr(0xf000a2c4, "SH_TAPI_phoneGetStatusMessages");
	add_ce_kernel_addr(0xf000a2c0, "SH_TAPI_phoneGetVolume");
	add_ce_kernel_addr(0xf000a2bc, "SH_TAPI_TAPIphoneInitializeEx");
	add_ce_kernel_addr(0xf000a2b8, "SH_TAPI_phoneNegotiateAPIVersion");
	add_ce_kernel_addr(0xf000a2b4, "SH_TAPI_phoneNegotiateExtVersion");
	add_ce_kernel_addr(0xf000a2b0, "SH_TAPI_phoneOpen");
	add_ce_kernel_addr(0xf000a2ac, "SH_TAPI_phoneSetGain");
	add_ce_kernel_addr(0xf000a2a8, "SH_TAPI_phoneSetHookSwitch");
	add_ce_kernel_addr(0xf000a2a4, "SH_TAPI_phoneSetRing");
	add_ce_kernel_addr(0xf000a2a0, "SH_TAPI_phoneSetStatusMessages");
	add_ce_kernel_addr(0xf000a29c, "SH_TAPI_phoneSetVolume");
	add_ce_kernel_addr(0xf000a298, "SH_TAPI_TAPIphoneShutdown");
	add_ce_kernel_addr(0xf0009ff8, "SH_PATCHER_PatchExe");
	add_ce_kernel_addr(0xf0009ff4, "SH_PATCHER_PatchDll");
	add_ce_kernel_addr(0xf0009ff0, "SH_PATCHER_FreeDllPatch");
	add_ce_kernel_addr(0xf00097f8, "SH_SERVICES_ActivateService");
	add_ce_kernel_addr(0xf00097f4, "SH_SERVICES_RegisterService");
	add_ce_kernel_addr(0xf00097f0, "SH_SERVICES_DeregisterService");
	add_ce_kernel_addr(0xf00097ec, "SH_SERVICES_CloseAllServiceHandles");
	add_ce_kernel_addr(0xf00097e8, "SH_SERVICES_CreateServiceHandle");
	add_ce_kernel_addr(0xf00097e4, "SH_SERVICES_GetServiceByIndex");
	add_ce_kernel_addr(0xf00097e0, "SH_SERVICES_ServiceIoControl");
	add_ce_kernel_addr(0xf00097dc, "SH_SERVICES_ServiceAddPort");
	add_ce_kernel_addr(0xf00097d8, "SH_SERVICES_ServiceUnbindPorts");
	add_ce_kernel_addr(0xf00097d4, "SH_SERVICES_EnumServices");
	add_ce_kernel_addr(0xf00097d0, "SH_SERVICES_GetServiceHandle");
	add_ce_kernel_addr(0xf00097cc, "SH_SERVICES_ServiceClosePort");
}

void arm7_cpu_device::print_ce_kernel_address(const offs_t addr)
{
	if (addr < 0x10400 && m_ce_kernel_addr_present[addr])
	{
		printf("Kernel Call: %s\n", m_ce_kernel_addrs[addr].c_str());
	}
	else if (addr == 0xFFFFCB80)
		printf("Kernel Call: InterlockedPopList\n");
	else if (addr == 0xFFFFCB98)
		printf("Kernel Call: InterlockedPushList\n");
	else if (addr == 0xFFFFCBAC)
		printf("Kernel Call: InterlockedCompareExchange\n");
	else if (addr == 0xFFFFCBC0)
		printf("Kernel Call: InterlockedExchangeAdd\n");
	else if (addr == 0xFFFFCBD4)
		printf("Kernel Call: InterlockedExchange\n");
}

void arm7_cpu_device::execute_run()
{
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
			offs_t raddr;

			pc = m_r[eR15];

			// "In Thumb state, bit [0] is undefined and must be ignored. Bits [31:1] contain the PC."
			raddr = pc & (~1);

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
			offs_t raddr;

			/* load 32 bit instruction */

			// "In ARM state, bits [1:0] of r15 are undefined and must be ignored. Bits [31:2] contain the PC."
			raddr = pc & (~3);

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
	switch (irqline) {
	case ARM7_IRQ_LINE: /* IRQ */
		m_pendingIrq = state ? true : false;
		break;

	case ARM7_FIRQ_LINE: /* FIRQ */
		m_pendingFiq = state ? true : false;
		break;

	case ARM7_ABORT_EXCEPTION:
		m_pendingAbtD = state ? true : false;
		break;
	case ARM7_ABORT_PREFETCH_EXCEPTION:
		m_pendingAbtP = state ? true : false;
		break;

	case ARM7_UNDEFINE_EXCEPTION:
		m_pendingUnd = state ? true : false;
		break;
	}

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
			LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback TLB Ops = %08x (%d) (%d), PC = %08x\n", data, op2, op3, m_r[eR15]);
			switch (op2)
			{
				case 0:
					switch (op3)
					{
						case 5:
							// Flush I
							for (uint32_t i = 0; i < ARRAY_LENGTH(m_itlb_entries); i++)
							{
								m_itlb_entries[i].valid = false;
							}
							break;
						case 6:
							// Flush D
							for (uint32_t i = 0; i < ARRAY_LENGTH(m_dtlb_entries); i++)
							{
								m_dtlb_entries[i].valid = false;
							}
							break;
						case 7:
							// Flush I+D
							for (uint32_t i = 0; i < ARRAY_LENGTH(m_dtlb_entries); i++)
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
					if (op3 == 6)
					{
						// Flush D single entry
						tlb_entry *entry = tlb_map_entry(m_r[op3], ARM7_TLB_ABORT_D);
						if (entry)
						{
							entry->valid = false;
						}
					}
					else
					{
						LOGMASKED(LOG_COPRO_WRITES, "arm7_rt_w_callback Unsupported TLB Op\n");
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
			result = *wp;
			result = (result >> (8 * (addr & 3))) | (result << (32 - (8 * (addr & 3))));
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
			result = *wp;
			result = (result >> (8 * (addr & 3))) | (result << (32 - (8 * (addr & 3))));
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
			result = m_program->read_dword(addr & ~3);
			result = (result >> (8 * (addr & 3))) | (result << (32 - (8 * (addr & 3))));
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
		result = m_program->read_dword(addr & ~3);
		result = (result >> (8 * (addr & 3))) | (result << (32 - (8 * (addr & 3))));
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
