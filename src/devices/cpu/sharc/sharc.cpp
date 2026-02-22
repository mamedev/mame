// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* Analog Devices ADSP-2106x SHARC emulator v3.0

   Written by Ville Linde
*/

#include "emu.h"
#include "sharc.h"

#include "sharcdsm.h"
#include "sharcfe.h"
#include "sharcinternal.ipp"

#include "emuopts.h"

#include "endianness.h"

#include <algorithm>
#include <cstdio>

//#define VERBOSE 1
#include "logmacro.h"


#define DISABLE_FAST_REGISTERS      1



#define CACHE_SIZE                      (2 * 1024 * 1024)
#define COMPILE_BACKWARDS_BYTES         128
#define COMPILE_FORWARDS_BYTES          512
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/4) + (COMPILE_FORWARDS_BYTES/4))
#define COMPILE_MAX_SEQUENCE            64


enum
{
	SHARC_PC=1,     SHARC_PCSTK,    SHARC_MODE1,    SHARC_MODE2,
	SHARC_ASTAT,    SHARC_STKY,     SHARC_IRPTL,    SHARC_IMASK,
	SHARC_IMASKP,   SHARC_USTAT1,   SHARC_USTAT2,   SHARC_LCNTR,
	SHARC_R0,       SHARC_R1,       SHARC_R2,       SHARC_R3,
	SHARC_R4,       SHARC_R5,       SHARC_R6,       SHARC_R7,
	SHARC_R8,       SHARC_R9,       SHARC_R10,      SHARC_R11,
	SHARC_R12,      SHARC_R13,      SHARC_R14,      SHARC_R15,
	SHARC_SYSCON,   SHARC_SYSSTAT,  SHARC_MRF,      SHARC_MRB,
	SHARC_STSTKP,   SHARC_PCSTKP,   SHARC_LSTKP,    SHARC_CURLCNTR,
	SHARC_FADDR,    SHARC_DADDR,
	SHARC_I0,       SHARC_I1,       SHARC_I2,       SHARC_I3,
	SHARC_I4,       SHARC_I5,       SHARC_I6,       SHARC_I7,
	SHARC_I8,       SHARC_I9,       SHARC_I10,      SHARC_I11,
	SHARC_I12,      SHARC_I13,      SHARC_I14,      SHARC_I15,
	SHARC_M0,       SHARC_M1,       SHARC_M2,       SHARC_M3,
	SHARC_M4,       SHARC_M5,       SHARC_M6,       SHARC_M7,
	SHARC_M8,       SHARC_M9,       SHARC_M10,      SHARC_M11,
	SHARC_M12,      SHARC_M13,      SHARC_M14,      SHARC_M15,
	SHARC_L0,       SHARC_L1,       SHARC_L2,       SHARC_L3,
	SHARC_L4,       SHARC_L5,       SHARC_L6,       SHARC_L7,
	SHARC_L8,       SHARC_L9,       SHARC_L10,      SHARC_L11,
	SHARC_L12,      SHARC_L13,      SHARC_L14,      SHARC_L15,
	SHARC_B0,       SHARC_B1,       SHARC_B2,       SHARC_B3,
	SHARC_B4,       SHARC_B5,       SHARC_B6,       SHARC_B7,
	SHARC_B8,       SHARC_B9,       SHARC_B10,      SHARC_B11,
	SHARC_B12,      SHARC_B13,      SHARC_B14,      SHARC_B15
};

DEFINE_DEVICE_TYPE(ADSP21062, adsp21062_device, "adsp21062", "Analog Devices ADSP21062 \"SHARC\"")
DEFINE_DEVICE_TYPE(ADSP21060, adsp21060_device, "adsp21060", "Analog Devices ADSP21060 \"SHARC\"")

void adsp21062_device::pgm_2m(address_map &map)
{
	map(0x20000, 0x24fff).mirror(0x18000).rw(FUNC(adsp21062_device::pm_r<1>), FUNC(adsp21062_device::pm_w<1>));
	map(0x20000, 0x24fff).rw(FUNC(adsp21062_device::pm_r<0>), FUNC(adsp21062_device::pm_w<0>));
}

void adsp21062_device::pgm_4m(address_map &map)
{
	map(0x20000, 0x29fff).rw(FUNC(adsp21062_device::pm_r<0>), FUNC(adsp21062_device::pm_w<0>));
	map(0x30000, 0x39fff).rw(FUNC(adsp21062_device::pm_r<1>), FUNC(adsp21062_device::pm_w<1>));
}

void adsp21062_device::data_2m(address_map &map)
{
	map(0x00000, 0x000ff).rw(FUNC(adsp21062_device::iop_r), FUNC(adsp21062_device::iop_w));
	map(0x20000, 0x27fff).mirror(0x18000).ram().share(m_blocks[1]);
	map(0x20000, 0x27fff).ram().share(m_blocks[0]);
	map(0x40000, 0x4ffff).mirror(0x30000).rw(FUNC(adsp21062_device::dmw_r<1>), FUNC(adsp21062_device::dmw_w<1>));
	map(0x40000, 0x4ffff).rw(FUNC(adsp21062_device::dmw_r<0>), FUNC(adsp21062_device::dmw_w<0>));
}

void adsp21062_device::data_4m(address_map &map)
{
	map(0x00000, 0x000ff).rw(FUNC(adsp21062_device::iop_r), FUNC(adsp21062_device::iop_w));
	map(0x20000, 0x2ffff).ram().share(m_blocks[0]);
	map(0x30000, 0x3ffff).ram().share(m_blocks[1]);
	map(0x40000, 0x5ffff).rw(FUNC(adsp21062_device::dmw_r<0>), FUNC(adsp21062_device::dmw_w<0>));
	map(0x60000, 0x7ffff).rw(FUNC(adsp21062_device::dmw_r<1>), FUNC(adsp21062_device::dmw_w<1>));
}

adsp21062_device::adsp21062_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp21062_device(
			mconfig,
			ADSP21062,
			tag,
			owner,
			clock,
			address_map_constructor(FUNC(adsp21062_device::pgm_2m), this),
			address_map_constructor(FUNC(adsp21062_device::data_2m), this))
{
}

adsp21060_device::adsp21060_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adsp21062_device(
			mconfig,
			ADSP21060,
			tag,
			owner,
			clock,
			address_map_constructor(FUNC(adsp21060_device::pgm_4m), this),
			address_map_constructor(FUNC(adsp21060_device::data_4m), this))
{
}

adsp21062_device::adsp21062_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		address_map_constructor internal_pgm,
		address_map_constructor internal_data)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 64, 24, -3, internal_pgm)
	, m_data_config("data", ENDIANNESS_LITTLE, 32, 32, -2, internal_data)
	, m_boot_mode(BOOT_MODE_HOST)
	, m_cache(CACHE_SIZE + sizeof(sharc_internal_state))
	, m_entry(nullptr)
	, m_nocode(nullptr)
	, m_out_of_cycles(nullptr)
	, m_reset_cache(nullptr)
	, m_pm_read48(nullptr)
	, m_pm_write48(nullptr)
	, m_pm_read32(nullptr)
	, m_pm_write32(nullptr)
	, m_dm_read32(nullptr)
	, m_dm_write32(nullptr)
	, m_push_pc(nullptr)
	, m_pop_pc(nullptr)
	, m_push_loop(nullptr)
	, m_pop_loop(nullptr)
	, m_push_status(nullptr)
	, m_pop_status(nullptr)
	, m_loop_check(nullptr)
	, m_call_loop_check(nullptr)
	, m_swap_dag1_0_3(nullptr)
	, m_swap_dag1_4_7(nullptr)
	, m_swap_dag2_0_3(nullptr)
	, m_swap_dag2_4_7(nullptr)
	, m_swap_r0_7(nullptr)
	, m_swap_r8_15(nullptr)
	, m_blocks(*this, "block%u", 0U)
	, m_flag_pending_val{ 0, 0, 0, 0 }
	, m_write_stalled_pending_val{ false }
	, m_flag_pending{ false, false, false, false }
	, m_write_stalled_pending(false)
	, m_input_update_pending(false)
	, m_enable_drc(false)
{
	std::fill(std::begin(m_exception), std::end(m_exception), nullptr);
}

adsp21062_device::~adsp21062_device()
{
}

adsp21060_device::~adsp21060_device()
{
}

device_memory_interface::space_config_vector adsp21062_device::memory_space_config() const
{
	return space_config_vector{
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_DATA,    &m_data_config) };
}

std::unique_ptr<util::disasm_interface> adsp21062_device::create_disassembler()
{
	return std::make_unique<sharc_disassembler>();
}

void adsp21062_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SHARC_ASTAT:
			if (m_enable_drc)
				m_core->astat_drc.unpack(m_core->astat);
			break;
	}
}

void adsp21062_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SHARC_ASTAT:
			if (m_enable_drc)
			{
				uint32_t const flags_mask = FLG0 | FLG1 | FLG2 | FLG3;
				m_core->astat = (m_core->astat & flags_mask) | m_core->astat_drc.pack();
			}
			break;
	}
}


void adsp21062_device::enable_recompiler()
{
	if (started())
		throw emu_fatalerror("SHARC: enable_recompiler: changing mode after starting\n");
	m_enable_drc = allow_drc();
}


void adsp21062_device::CHANGE_PC(uint32_t newpc)
{
	m_core->pc = newpc;
	m_core->daddr = newpc;
	m_core->faddr = newpc+1;
	m_core->nfaddr = newpc+2;
}

void adsp21062_device::CHANGE_PC_DELAYED(uint32_t newpc)
{
	m_core->nfaddr = newpc;

	m_core->delay_slot1 = m_core->pc;
	m_core->delay_slot2 = m_core->daddr;
}

TIMER_CALLBACK_MEMBER(adsp21062_device::sharc_iop_delayed_write_callback)
{
	switch (m_core->iop_delayed_reg)
	{
		case 0x1c:
		{
			if (m_core->iop_delayed_data & 0x1)
			{
				sharc_dma_exec(6);
			}
			break;
		}

		case 0x1d:
		{
			if (m_core->iop_delayed_data & 0x1)
			{
				sharc_dma_exec(7);
			}
			break;
		}

		default:
			throw emu_fatalerror("SHARC: sharc_iop_delayed_write: unknown IOP register %02X\n", m_core->iop_delayed_reg);
	}

	m_core->delayed_iop_timer->adjust(attotime::never, 0);
}

void adsp21062_device::sharc_iop_delayed_w(uint32_t reg, uint32_t data, int cycles)
{
	m_core->iop_delayed_reg = reg;
	m_core->iop_delayed_data = data;

	m_core->delayed_iop_timer->adjust(cycles_to_attotime(cycles), 0);
}

// 0 012 0h 0l 1h
// 1 453 2h 2l 1l
// 2 678 3h 3l 4h
// 3 ab9 5h 5l 4l
// 4 cde 6h 6l 7h

template <unsigned N>
uint64_t adsp21062_device::pm_r(offs_t offset)
{
	offs_t slot = offset >> 12;
	offs_t base = (offset & 0xfff) + (slot >> 1) * (3<<12);
	if (slot & 1)
		return (uint64_t(m_blocks[N][base + 0x2000]) << 16) | (m_blocks[N][base + 0x1000] & 0xffff);
	else
		return (uint64_t(m_blocks[N][base         ]) << 16) | (m_blocks[N][base + 0x1000] >> 16);
}

template <unsigned N>
void adsp21062_device::pm_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	offs_t slot = offset >> 12;
	offs_t base = (offset & 0xfff) + (slot >> 1) * (3<<12);
	if (slot & 1)
	{
		if (ACCESSING_BITS_0_15)
			m_blocks[N][base + 0x1000] = (m_blocks[N][base + 0x1000] & 0xffff0000) | (data & 0xffff);
		m_blocks[N][base + 0x2000] = (m_blocks[N][base + 0x2000] & ~(mem_mask >> 16)) | ((data & mem_mask) >> 16);
	}
	else
	{
		m_blocks[N][base + 0x0000] = (m_blocks[N][base + 0x0000] & ~(mem_mask >> 16)) | ((data & mem_mask) >> 16);
		if (ACCESSING_BITS_0_15)
			m_blocks[N][base + 0x1000] = (m_blocks[N][base + 0x1000] & 0xffff) | ((data & 0xffff) << 16);
	}
}

template <unsigned N>
uint32_t adsp21062_device::dmw_r(offs_t offset)
{
	return util::little_endian_cast<uint16_t const>(&m_blocks[N][0])[offset];
}

template <unsigned N>
void adsp21062_device::dmw_w(offs_t offset, uint32_t data)
{
	util::little_endian_cast<uint16_t>(&m_blocks[N][0])[offset] = uint16_t(data);
}

/* IOP registers */
uint32_t adsp21062_device::iop_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00: return 0;    // System configuration

		case 0x37:      // DMA status
		{
			return m_core->dma_status;
		}
		default:
		if (!machine().side_effects_disabled())
			throw emu_fatalerror("sharc_iop_r: Unimplemented IOP reg %02X at %08X\n", offset, m_core->pc);

		return 0;
	}
}

void adsp21062_device::iop_w(offs_t offset, uint32_t data)
{
	switch (offset)
	{
		case 0x00: m_core->syscon = data; break;
		case 0x02: break;       // External Memory Wait State Configuration
		case 0x04: // External port DMA buffer 0
		{
			external_dma_write(m_core->extdma_shift, data);
			m_core->extdma_shift++;
			if (m_core->extdma_shift == 3)
				m_core->extdma_shift = 0;
			break;
		}

		case 0x08: break;       // Message Register 0
		case 0x09: break;       // Message Register 1
		case 0x0a: break;       // Message Register 2
		case 0x0b: break;       // Message Register 3
		case 0x0c: break;       // Message Register 4
		case 0x0d: break;       // Message Register 5
		case 0x0e: break;       // Message Register 6
		case 0x0f: break;       // Message Register 7

		case 0x14: // reserved??? written by Last Bronx
		case 0x17: break;

		// DMA 6
		case 0x1c:
		{
			m_core->dma[6].control = data;
			if (data & 0x1)
			{
				sharc_iop_delayed_w(0x1c, data, 1);
			}
			break;
		}

		case 0x20: break;

		case 0x40: m_core->dma[6].int_index = data; return;
		case 0x41: m_core->dma[6].int_modifier = data; return;
		case 0x42: m_core->dma[6].int_count = data; return;
		case 0x43: m_core->dma[6].chain_ptr = data; return;
		case 0x44: m_core->dma[6].gen_purpose = data; return;
		case 0x45: m_core->dma[6].ext_index = data; return;
		case 0x46: m_core->dma[6].ext_modifier = data; return;
		case 0x47: m_core->dma[6].ext_count = data; return;

		// DMA 7
		case 0x1d:
		{
			m_core->dma[7].control = data;
			if (data & 0x1)
			{
				sharc_iop_delayed_w(0x1d, data, 30);
			}
			break;
		}

		case 0x48: m_core->dma[7].int_index = data; return;
		case 0x49: m_core->dma[7].int_modifier = data; return;
		case 0x4a: m_core->dma[7].int_count = data; return;
		case 0x4b: m_core->dma[7].chain_ptr = data; return;
		case 0x4c: m_core->dma[7].gen_purpose = data; return;
		case 0x4d: m_core->dma[7].ext_index = data; return;
		case 0x4e: m_core->dma[7].ext_modifier = data; return;
		case 0x4f: m_core->dma[7].ext_count = data; return;

		default:
			throw emu_fatalerror("sharc_iop_w: Unimplemented IOP reg %02X, %08X at %08X\n", offset, data, m_core->pc);
	}
}


#include "sharcdma.hxx"
#include "sharcops.hxx"



void adsp21062_device::build_opcode_table()
{
	for (int i = 0; i < std::size(m_sharc_op); i++)
	{
		m_sharc_op[i] = &adsp21062_device::sharcop_unimplemented;

		const uint16_t op = i << 7;

		int j = 0;
		while (j < s_num_ops)
		{
			auto &opcode = s_sharc_opcode_table[j++];
			if ((opcode.op_mask & op) == opcode.op_bits)
			{
				m_sharc_op[i] = opcode.handler;
				break;
			}
		}
		while (j < s_num_ops)
		{
			auto &opcode = s_sharc_opcode_table[j++];
			if ((opcode.op_mask & op) == opcode.op_bits)
			{
				throw emu_fatalerror("build_opcode_table: table already filled! (i=%04X, j=%d)\n", i, j);
			}
		}
	}
}

/*****************************************************************************/

void adsp21062_device::external_iop_write(uint32_t address, uint32_t data)
{
	// host packing mode used to determine width of external host bus width (16/32)
	// if writing to external port DMA buffer, just write the data directly;
	// external_dma_write() handles packed data
	if (m_core->syscon & 0x10 && address != 0x04)
	{
		if ((m_core->iop_write_num++ & 1) == 0)
		{
			m_core->iop_data = data & 0xffff;
			return;
		}
		else
		{
			m_core->iop_data |= (data & 0xffff) << 16;
		}
	}
	else
	{
		m_core->iop_data = data;
	}

	if (address == 0x1c)
	{
		m_core->dma[6].control = m_core->iop_data;
	}
	else
	{
		LOG("SHARC IOP write %08X, %08X\n", address, m_core->iop_data);
		m_data.write_dword(address, m_core->iop_data);
	}
}

void adsp21062_device::external_dma_write(uint32_t address, uint64_t data)
{
	/*
	All addresses in the 17-bit index registers are offset by 0x0002 0000, the
	first internal RAM location, before they are used by the DMA controller.
	*/

	offs_t const index = (m_core->dma[6].int_index & 0x1ffff) | 0x20000;
	unsigned const mswf = BIT(m_core->dma[6].control, 8);
	unsigned const pmode = BIT(m_core->dma[6].control, 6, 2);
	unsigned const dtype = BIT(m_core->dma[6].control, 5);
	switch (pmode)
	{
		case 0:         // no packing
		{
			if (dtype)
				pm_write32(index, data);
			else
				dm_write32(index, data);

			m_core->dma[6].int_index += m_core->dma[6].int_modifier;
			break;
		}
		case 2:         // 16/48 packing
		{
			// FIXME: honour DTYPE
			unsigned const word = address % 3;
			unsigned const shift = (mswf ? (2 - word) : word) * 16;

			uint64_t r = pm_read48(index);
			r &= ~(uint64_t(0xffff) << shift);
			r |= (data & 0xffff) << shift;

			pm_write48(index, r);

			if (word == 2)
			{
				m_core->dma[6].int_index += m_core->dma[6].int_modifier;
			}
			break;
		}
		default:
		{
			throw emu_fatalerror("sharc_external_dma_write: unimplemented packing mode %d\n", pmode);
		}
	}
}

void adsp21062_device::device_start()
{
	assert(m_blocks[0].length() == m_blocks[1].length());
	assert(!(m_blocks[0].length() & (m_blocks[0].length() - 1)));

	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);

	if (!m_enable_drc)
	{
		m_heap_core = std::make_unique<sharc_internal_state>();
		m_core = m_heap_core.get();
		memset(m_core, 0, sizeof(sharc_internal_state));

		build_opcode_table();
	}
	else
	{
		m_cache.allocate_cache(mconfig().options().drc_rwx());
		m_core = m_cache.alloc_near<sharc_internal_state>();
		memset(m_core, 0, sizeof(sharc_internal_state));

		// init UML generator
		uint32_t umlflags = 0;
		m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, umlflags, 1, 24, 0);

		// add UML symbols
		m_drcuml->symbol_add(&m_core->pc, sizeof(m_core->pc), "pc");
		m_drcuml->symbol_add(&m_core->icount, sizeof(m_core->icount), "icount");

		for (int i = 0; 16 > i; ++i)
		{
			char buf[10];

			std::snprintf(buf, std::size(buf), "r%d", i);
			m_drcuml->symbol_add(&m_core->r[i], sizeof(m_core->r[i]), buf);

			auto &dag((i < 8) ? m_core->dag1 : m_core->dag2);
			std::snprintf(buf, std::size(buf), "dag_i%d", i);
			m_drcuml->symbol_add(&dag.i[i & 7], sizeof(dag.i[i & 0x07]), buf);
			std::snprintf(buf, std::size(buf), "dag_m%d", i);
			m_drcuml->symbol_add(&dag.m[i & 7], sizeof(dag.m[i & 0x07]), buf);
			std::snprintf(buf, std::size(buf), "dag_l%d", i);
			m_drcuml->symbol_add(&dag.l[i & 7], sizeof(dag.l[i & 0x07]), buf);
			std::snprintf(buf, std::size(buf), "dag_b%d", i);
			m_drcuml->symbol_add(&dag.b[i & 7], sizeof(dag.b[i & 0x07]), buf);
		}

		m_drcuml->symbol_add(&m_core->astat, sizeof(m_core->astat), "astat");
		m_drcuml->symbol_add(&m_core->mode1, sizeof(m_core->mode1), "mode1");
		m_drcuml->symbol_add(&m_core->mode2, sizeof(m_core->mode2), "mode2");
		m_drcuml->symbol_add(&m_core->lcntr, sizeof(m_core->lcntr), "lcntr");
		m_drcuml->symbol_add(&m_core->curlcntr, sizeof(m_core->curlcntr), "curlcntr");
		m_drcuml->symbol_add(&m_core->imask, sizeof(m_core->imask), "imask");
		m_drcuml->symbol_add(&m_core->imaskp, sizeof(m_core->imaskp), "imaskp");
		m_drcuml->symbol_add(&m_core->irptl, sizeof(m_core->irptl), "irptl");
		m_drcuml->symbol_add(&m_core->ustat1, sizeof(m_core->ustat1), "ustat1");
		m_drcuml->symbol_add(&m_core->ustat2, sizeof(m_core->ustat2), "ustat2");
		m_drcuml->symbol_add(&m_core->stky, sizeof(m_core->stky), "stky");

		m_drcuml->symbol_add(&m_core->astat_drc.az, sizeof(m_core->astat_drc.az), "astat_az");
		m_drcuml->symbol_add(&m_core->astat_drc.ac, sizeof(m_core->astat_drc.ac), "astat_ac");
		m_drcuml->symbol_add(&m_core->astat_drc.an, sizeof(m_core->astat_drc.an), "astat_an");
		m_drcuml->symbol_add(&m_core->astat_drc.av, sizeof(m_core->astat_drc.av), "astat_av");
		m_drcuml->symbol_add(&m_core->astat_drc.ai, sizeof(m_core->astat_drc.ai), "astat_ai");
		m_drcuml->symbol_add(&m_core->astat_drc.as, sizeof(m_core->astat_drc.as), "astat_as");
		m_drcuml->symbol_add(&m_core->astat_drc.mv, sizeof(m_core->astat_drc.mv), "astat_mv");
		m_drcuml->symbol_add(&m_core->astat_drc.mn, sizeof(m_core->astat_drc.mn), "astat_mn");
		m_drcuml->symbol_add(&m_core->astat_drc.mu, sizeof(m_core->astat_drc.mu), "astat_mu");
		m_drcuml->symbol_add(&m_core->astat_drc.mi, sizeof(m_core->astat_drc.mi), "astat_mi");
		m_drcuml->symbol_add(&m_core->astat_drc.sz, sizeof(m_core->astat_drc.sz), "astat_sz");
		m_drcuml->symbol_add(&m_core->astat_drc.sv, sizeof(m_core->astat_drc.sv), "astat_sv");
		m_drcuml->symbol_add(&m_core->astat_drc.ss, sizeof(m_core->astat_drc.ss), "astat_ss");

		m_drcuml->symbol_add(&m_core->arg0, sizeof(m_core->arg0), "arg0");
		m_drcuml->symbol_add(&m_core->arg1, sizeof(m_core->arg1), "arg1");
		m_drcuml->symbol_add(&m_core->arg2, sizeof(m_core->arg2), "arg2");
		m_drcuml->symbol_add(&m_core->arg3, sizeof(m_core->arg3), "arg3");

		m_drcuml->symbol_add(&m_core->dreg_temp, sizeof(m_core->dreg_temp), "dreg_temp");
		m_drcuml->symbol_add(&m_core->lstkp, sizeof(m_core->lstkp), "lstkp");
		m_drcuml->symbol_add(&m_core->px, sizeof(m_core->px), "px");

		m_drcfe = std::make_unique<sharc_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, COMPILE_MAX_SEQUENCE);

		for (int i = 0; i < 16; i++)
			m_regmap[i] = uml::mem(&m_core->r[i]);

		// I0-3 used by the DRC, rest can be assigned to fast registers
		if (!DISABLE_FAST_REGISTERS)
		{
			drcbe_info beinfo;
			m_drcuml->get_backend_info(beinfo);
			if (beinfo.direct_iregs > 4)
				m_regmap[0] = uml::I4;
			if (beinfo.direct_iregs > 5)
				m_regmap[1] = uml::I5;
			if (beinfo.direct_iregs > 6)
				m_regmap[2] = uml::I6;
			if (beinfo.direct_iregs > 7)
				m_regmap[3] = uml::I7;
		}

		generate_invariant();
		m_core->cache_dirty = 1;
	}

	m_core->delayed_iop_timer = timer_alloc(FUNC(adsp21062_device::sharc_iop_delayed_write_callback), this);

	for (auto & elem : m_core->dma_op)
	{
		elem.src = 0;
		elem.dst = 0;
		elem.chain_ptr = 0;
		elem.src_modifier = 0;
		elem.dst_modifier = 0;
		elem.src_count = 0;
		elem.dst_count = 0;
		elem.pmode = 0;
		elem.chained_direction = 0;
		elem.active = false;
		elem.timer = timer_alloc(FUNC(adsp21062_device::sharc_dma_callback), this);
	}

	for (int i=0; i < 16; i++)
	{
		m_core->r[i].r = 0;
		m_core->reg_alt[i].r = 0;
	}
	m_core->mrf = 0;
	m_core->mrb = 0;
	std::fill(std::begin(m_core->pcstack), std::end(m_core->pcstack), 0);
	std::fill(std::begin(m_core->lcstack), std::end(m_core->lcstack), 0);
	std::fill(std::begin(m_core->lastack), std::end(m_core->lastack), 0);
	m_core->pcstk = 0;
	m_core->laddr.addr = m_core->laddr.code = m_core->laddr.loop_type = 0;
	m_core->curlcntr = 0;
	m_core->lcntr = 0;
	for (int i=0; i < 8; i++)
	{
		m_core->dag1.i[i] = m_core->dag1.m[i] = m_core->dag1.b[i] = m_core->dag1.l[i] = 0;
		m_core->dag2.i[i] = m_core->dag2.m[i] = m_core->dag2.b[i] = m_core->dag2.l[i] = 0;
		m_core->dag1_alt.i[i] = m_core->dag1_alt.m[i] = m_core->dag1_alt.b[i] = m_core->dag1_alt.l[i] = 0;
		m_core->dag2_alt.i[i] = m_core->dag2_alt.m[i] = m_core->dag2_alt.b[i] = m_core->dag2_alt.l[i] = 0;
	}
	for (auto & elem : m_core->dma)
	{
		elem.control = 0;
		elem.int_index = 0;
		elem.int_modifier = 0;
		elem.int_count = 0;
		elem.chain_ptr = 0;
		elem.gen_purpose = 0;
		elem.ext_index = 0;
		elem.ext_modifier = 0;
		elem.ext_count = 0;
	}
	m_core->mode1 = 0;
	m_core->mode2 = 0;
	m_core->astat = 0;
	m_core->irptl = 0;
	m_core->imask = 0;
	m_core->imaskp = 0;
	m_core->ustat1 = 0;
	m_core->ustat2 = 0;
	m_core->flag[0] = m_core->flag[1] = m_core->flag[2] = m_core->flag[3] = 0;
	m_core->syscon = 0;
	m_core->sysstat = 0;
	for (auto & elem : m_core->status_stack)
	{
		elem.mode1 = 0;
		elem.astat = 0;
	}
	m_core->status_stkp = 0;
	m_core->px = 0;
	m_core->opcode = 0;
	m_core->irq_pending = 0;
	m_core->active_irq_num = 0;
	m_core->dma_status = 0;
	m_core->iop_delayed_reg = 0;
	m_core->iop_delayed_data = 0;
	m_core->delay_slot1 = 0;
	m_core->delay_slot2 = 0;
	m_core->systemreg_latency_cycles = 0;
	m_core->systemreg_latency_reg = 0;
	m_core->systemreg_latency_data = 0;
	m_core->systemreg_previous_data = 0;
	m_core->astat_old = 0;
	m_core->astat_old_old = 0;
	m_core->astat_old_old_old = 0;

	m_core->fp_const.k0_0 = 0.0F;
	m_core->fp_const.k0_5 = 0.5F;
	m_core->fp_const.k1_0 = 1.0F;
	m_core->fp_const.k2_0 = 2.0F;

	save_pointer(NAME(&m_core->r[0].r), std::size(m_core->r));
	save_pointer(NAME(&m_core->reg_alt[0].r), std::size(m_core->reg_alt));
	save_item(NAME(m_core->pc));
	save_item(NAME(m_core->mrf));
	save_item(NAME(m_core->mrb));

	save_item(NAME(m_core->pcstack));
	save_item(NAME(m_core->lcstack));
	save_item(NAME(m_core->lastack));
	save_item(NAME(m_core->lstkp));

	save_item(NAME(m_core->faddr));
	save_item(NAME(m_core->daddr));
	save_item(NAME(m_core->pcstkp));
	save_item(NAME(m_core->iop_write_num));
	save_item(NAME(m_core->iop_data));

	save_item(NAME(m_core->dag1.i));
	save_item(NAME(m_core->dag1.m));
	save_item(NAME(m_core->dag1.b));
	save_item(NAME(m_core->dag1.l));
	save_item(NAME(m_core->dag2.i));
	save_item(NAME(m_core->dag2.m));
	save_item(NAME(m_core->dag2.b));
	save_item(NAME(m_core->dag2.l));
	save_item(NAME(m_core->dag1_alt.i));
	save_item(NAME(m_core->dag1_alt.m));
	save_item(NAME(m_core->dag1_alt.b));
	save_item(NAME(m_core->dag1_alt.l));
	save_item(NAME(m_core->dag2_alt.i));
	save_item(NAME(m_core->dag2_alt.m));
	save_item(NAME(m_core->dag2_alt.b));
	save_item(NAME(m_core->dag2_alt.l));

	save_item(STRUCT_MEMBER(m_core->dma, control));
	save_item(STRUCT_MEMBER(m_core->dma, int_index));
	save_item(STRUCT_MEMBER(m_core->dma, int_modifier));
	save_item(STRUCT_MEMBER(m_core->dma, int_count));
	save_item(STRUCT_MEMBER(m_core->dma, chain_ptr));
	save_item(STRUCT_MEMBER(m_core->dma, gen_purpose));
	save_item(STRUCT_MEMBER(m_core->dma, ext_index));
	save_item(STRUCT_MEMBER(m_core->dma, ext_modifier));
	save_item(STRUCT_MEMBER(m_core->dma, ext_count));

	save_item(NAME(m_core->mode1));
	save_item(NAME(m_core->mode2));
	save_item(NAME(m_core->astat));
	save_item(NAME(m_core->stky));
	save_item(NAME(m_core->irptl));
	save_item(NAME(m_core->imask));
	save_item(NAME(m_core->imaskp));
	save_item(NAME(m_core->ustat1));
	save_item(NAME(m_core->ustat2));

	save_item(NAME(m_core->flag));

	save_item(NAME(m_core->syscon));
	save_item(NAME(m_core->sysstat));

	save_item(STRUCT_MEMBER(m_core->status_stack, mode1));
	save_item(STRUCT_MEMBER(m_core->status_stack, astat));
	save_item(NAME(m_core->status_stkp));

	save_item(NAME(m_core->px));

	save_item(NAME(m_core->opcode));

	save_item(NAME(m_core->nfaddr));

	save_item(NAME(m_core->idle));
	save_item(NAME(m_core->irq_pending));
	save_item(NAME(m_core->active_irq_num));

	save_item(STRUCT_MEMBER(m_core->dma_op, src));
	save_item(STRUCT_MEMBER(m_core->dma_op, dst));
	save_item(STRUCT_MEMBER(m_core->dma_op, chain_ptr));
	save_item(STRUCT_MEMBER(m_core->dma_op, src_modifier));
	save_item(STRUCT_MEMBER(m_core->dma_op, dst_modifier));
	save_item(STRUCT_MEMBER(m_core->dma_op, src_count));
	save_item(STRUCT_MEMBER(m_core->dma_op, dst_count));
	save_item(STRUCT_MEMBER(m_core->dma_op, pmode));
	save_item(STRUCT_MEMBER(m_core->dma_op, chained_direction));
	save_item(STRUCT_MEMBER(m_core->dma_op, active));
	save_item(STRUCT_MEMBER(m_core->dma_op, chained));

	save_item(NAME(m_core->dma_status));
	save_item(NAME(m_core->write_stalled));

	save_item(NAME(m_core->interrupt_active));

	save_item(NAME(m_core->iop_delayed_reg));
	save_item(NAME(m_core->iop_delayed_data));

	save_item(NAME(m_core->delay_slot1));
	save_item(NAME(m_core->delay_slot2));

	save_item(NAME(m_core->systemreg_latency_cycles));
	save_item(NAME(m_core->systemreg_latency_reg));
	save_item(NAME(m_core->systemreg_latency_data));
	save_item(NAME(m_core->systemreg_previous_data));

	save_item(NAME(m_core->astat_old));
	save_item(NAME(m_core->astat_old_old));
	save_item(NAME(m_core->astat_old_old_old));

	state_add( SHARC_PC,     "PC", m_core->pc).mask(0x00ffffff).formatstr("%06X");
	state_add( SHARC_PCSTK,  "PCSTK", m_core->pcstk).mask(0x00ffffff).formatstr("%06X");
	state_add( SHARC_PCSTKP, "PCSTKP", m_core->pcstkp).mask(0x1f).formatstr("%02X");
	state_add( SHARC_LSTKP,  "LSTKP", m_core->lstkp).mask(0x07).formatstr("%01X");
	state_add( SHARC_FADDR,  "FADDR", m_core->faddr).formatstr("%08X");
	state_add( SHARC_DADDR,  "DADDR", m_core->daddr).formatstr("%08X");
	state_add( SHARC_MODE1,  "MODE1", m_core->mode1).formatstr("%08X");
	state_add( SHARC_MODE2,  "MODE2", m_core->mode2).formatstr("%08X");
	state_add( SHARC_ASTAT,  "ASTAT", m_core->astat).formatstr("%08X").callimport().callexport();
	state_add( SHARC_IRPTL,  "IRPTL", m_core->irptl).formatstr("%08X");
	state_add( SHARC_IMASK,  "IMASK", m_core->imask).formatstr("%08X");
	state_add( SHARC_USTAT1, "USTAT1", m_core->ustat1).formatstr("%08X");
	state_add( SHARC_USTAT2, "USTAT2", m_core->ustat2).formatstr("%08X");
	state_add( SHARC_CURLCNTR, "CURLCNTR", m_core->curlcntr).formatstr("%08X");
	state_add( SHARC_STSTKP, "STSTKP", m_core->status_stkp).formatstr("%08X");

	char namebuf[8];
	for (int i = 0; 16 > i; ++i)
	{
		std::snprintf(namebuf, std::size(namebuf), "R%d", i);
		state_add(SHARC_R0 + i, namebuf, m_core->r[i].r).formatstr("%08X");
	}
	for (int i = 0; 16 > i; ++i)
	{
		std::snprintf(namebuf, std::size(namebuf), "I%d", i);
		auto &dag((i < 8) ? m_core->dag1 : m_core->dag2);
		state_add(SHARC_I0 + i, namebuf, dag.i[i]).formatstr("%08X");
	}
	for (int i = 0; 16 > i; ++i)
	{
		std::snprintf(namebuf, std::size(namebuf), "M%d", i);
		auto &dag((i < 8) ? m_core->dag1 : m_core->dag2);
		state_add(SHARC_M0 + i, namebuf, dag.m[i]).formatstr("%08X");
	}
	for (int i = 0; 16 > i; ++i)
	{
		std::snprintf(namebuf, std::size(namebuf), "L%d", i);
		auto &dag((i < 8) ? m_core->dag1 : m_core->dag2);
		state_add(SHARC_L0 + i, namebuf, dag.l[i]).formatstr("%08X");
	}
	for (int i = 0; 16 > i; ++i)
	{
		std::snprintf(namebuf, std::size(namebuf), "B%d", i);
		auto &dag((i < 8) ? m_core->dag1 : m_core->dag2);
		state_add(SHARC_B0 + i, namebuf, dag.b[i]).formatstr("%08X");
	}

	state_add( STATE_GENPC, "GENPC", m_core->pc).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_core->pc).noshow();

	set_icountptr(m_core->icount);
}

void adsp21062_device::device_reset()
{
	for (auto &block : m_blocks)
		std::fill(std::begin(block), std::end(block), 0);

	switch (m_boot_mode)
	{
		case BOOT_MODE_EPROM:
		{
			m_core->dma[6].int_index      = 0x20000;
			m_core->dma[6].int_modifier   = 1;
			m_core->dma[6].int_count      = 0x100;
			m_core->dma[6].ext_index      = 0x400000;
			m_core->dma[6].ext_modifier   = 1;
			m_core->dma[6].ext_count      = 0x600;
			m_core->dma[6].control        = 0x2a1;

			sharc_dma_exec(6);
			dma_op(6);

			m_core->dma_op[6].timer->adjust(attotime::never, 0);
			break;
		}

		case BOOT_MODE_HOST:
		{
			m_core->dma[6].int_index      = 0x20000;
			m_core->dma[6].int_modifier   = 1;
			m_core->dma[6].int_count      = 0x100;
			m_core->dma[6].ext_index      = 0x400000;
			m_core->dma[6].ext_modifier   = 1;
			m_core->dma[6].ext_count      = 0x600;
			m_core->dma[6].control        = 0xa1;
			break;
		}

		default:
			throw emu_fatalerror("SHARC: Unimplemented boot mode %d\n", m_boot_mode);
	}

	m_core->pc = 0x20004;
	m_core->extdma_shift = 0;
	m_core->daddr = m_core->pc + 1;
	m_core->faddr = m_core->daddr + 1;
	m_core->nfaddr = m_core->faddr+1;

	m_core->idle = 0;
	m_core->mode1 = 0x00000000;
	m_core->mode2 &= 0xf0000000;
	m_core->astat &= FLG0 | FLG1 | FLG2 | FLG3;
	m_core->stky = PCEM | SSEM | LSEM;
	m_core->irptl = 0x0000;
	m_core->imask = 0x0003;
	m_core->ustat1 = 0x0000;
	m_core->ustat2 = 0x0000;

	m_core->pcstkp = 0;
	m_core->lstkp = 0;
	m_core->pcstk = 0x00ffffff;
	m_core->curlcntr = 0xffffffff;
	m_core->lcntr = m_core->lcstack[0];
	m_core->laddr.unpack(0xffffffff);
	m_core->status_stkp = 0;
	m_core->interrupt_active = 0;

	m_core->syscon = 0x00000010;
	m_core->sysstat &= 0x00000ff0;
	m_core->iop_write_num = 0;
	m_core->iop_data = 0;

	if (m_enable_drc)
	{
		m_core->astat_drc.clear();

		m_core->cache_dirty = 1;
		m_drcuml->reset();
	}
}

void adsp21062_device::device_pre_save()
{
	assert(!m_input_update_pending);

	cpu_device::device_pre_save();

	if ((m_core->pcstkp > 0) && (m_core->pcstkp < 31))
		m_core->pcstack[m_core->pcstkp - 1] = m_core->pcstk;

	if ((m_core->lstkp > 0) && (m_core->lstkp < 7))
	{
		m_core->lcstack[m_core->lstkp - 1] = m_core->curlcntr;
		m_core->lastack[m_core->lstkp - 1] = m_core->laddr.pack();
	}

	if (m_core->lstkp < 6)
		m_core->lcstack[m_core->lstkp] = m_core->lcntr;

	if (m_enable_drc)
	{
		m_core->astat = (m_core->astat & (FLG0 | FLG1 | FLG2 | FLG3)) | m_core->astat_drc.pack();
		m_core->astat_old = m_core->astat_drc_copy.pack();
		m_core->astat_old_old = m_core->astat_delay_copy.pack();
	}
}

void adsp21062_device::device_post_load()
{
	cpu_device::device_post_load();

	for (auto &pcstk : m_core->pcstack)
		pcstk &= 0x00ffffff;

	m_core->pcstkp &= 0x1f;
	m_core->lstkp &= 0x07;

	if ((m_core->pcstkp > 0) && (m_core->pcstkp < 31))
		m_core->pcstk = m_core->pcstack[m_core->pcstkp - 1];
	else
		m_core->pcstk = 0x00ffffff;

	if ((m_core->lstkp > 0) && (m_core->lstkp < 7))
	{
		m_core->curlcntr = m_core->lcstack[m_core->lstkp - 1];
		m_core->laddr.unpack(m_core->lastack[m_core->lstkp - 1]);
	}
	else
	{
		m_core->curlcntr = 0xffffffff;
		m_core->laddr.unpack(0xffffffff);
	}

	if (m_core->lstkp < 6)
		m_core->lcntr = m_core->lcstack[m_core->lstkp];
	else
		m_core->lcntr = 0xffffffff;

	if (m_core->pcstkp > 0)
		m_core->stky &= ~PCEM;
	else
		m_core->stky |= PCEM;

	if (m_core->pcstkp >= 30)
		m_core->stky |= PCFL;
	else
		m_core->stky &= ~PCFL;

	if (m_core->lstkp > 0)
		m_core->stky &= ~LSEM;
	else
		m_core->stky |= LSEM;

	m_core->astat_drc.unpack(m_core->astat);
	m_core->astat_drc_copy.unpack(m_core->astat_old);
	m_core->astat_delay_copy.unpack(m_core->astat_old_old);
}


void adsp21062_device::execute_set_input(int irqline, int state)
{
	if (irqline >= 0 && irqline <= 2)
	{
		if (state == ASSERT_LINE)
		{
			m_core->irq_pending |= 1 << (8-irqline);
		}
		else
		{
			m_core->irq_pending &= ~(1 << (8-irqline));
		}
	}
}

void adsp21062_device::set_flag_input(int flag_num, int state)
{
	assert((flag_num >= 0) && (flag_num < 4));

	// Check if flag is set to input in MODE2 (bit == 0)
	if (BIT(m_core->mode2, flag_num + 15))
		throw emu_fatalerror("sharc_set_flag_input: flag %d is set output!\n", flag_num);

	state = state ? 1 : 0;
	device_execute_interface *const current = machine().scheduler().currently_executing();
	bool const not_executing = current && (current != static_cast<device_execute_interface *>(this));
	if (not_executing || m_flag_pending[flag_num])
	{
		if (m_flag_pending[flag_num] || (m_core->flag[flag_num] != state))
		{
			m_flag_pending_val[flag_num] = state;
			m_flag_pending[flag_num] = true;
			if (!m_input_update_pending)
			{
				m_input_update_pending = true;
				machine().scheduler().synchronize(timer_expired_delegate(FUNC(adsp21062_device::sharc_update_inputs), this));
			}
		}
	}
	else
	{
		m_core->flag[flag_num] = state ? 1 : 0;
	}
}

void adsp21062_device::write_stall(int state)
{
	bool const stall = state != 0;
	device_execute_interface *const current = machine().scheduler().currently_executing();
	bool const not_executing = current && (current != static_cast<device_execute_interface *>(this));
	if (m_enable_drc || not_executing || m_write_stalled_pending)
	{
		if (m_write_stalled_pending || (m_core->write_stalled != stall))
		{
			m_write_stalled_pending_val = stall;
			m_write_stalled_pending = true;
			if (!m_input_update_pending)
			{
				m_input_update_pending = true;
				machine().scheduler().synchronize(timer_expired_delegate(FUNC(adsp21062_device::sharc_update_inputs), this));
			}
		}
	}
	else
	{
		m_core->write_stalled = stall;
	}
}

TIMER_CALLBACK_MEMBER(adsp21062_device::sharc_update_inputs)
{
	m_input_update_pending = false;

	for (unsigned i = 0; 4 > i; ++i)
	{
		if (m_flag_pending[i])
		{
			m_core->flag[i] = m_flag_pending_val[i];
			m_flag_pending[i] = false;
		}
	}

	if (m_write_stalled_pending)
	{
		if (m_core->write_stalled != m_write_stalled_pending_val)
		{
			m_core->write_stalled = m_write_stalled_pending_val;
#if 0 // FIXME: this implementation breaks Thrill Drive on Hornet
			if (m_enable_drc)
			{
				if (m_core->write_stalled)
				{
					m_core->dma_op[6].timer->adjust(attotime::never, 0);
					m_core->dma_op[7].timer->adjust(attotime::never, 0);
				}
				else
				{
					if (m_core->dma_status & (1 << 6))
						m_core->dma_op[6].timer->adjust(cycles_to_attotime(m_core->dma_op[6].src_count / 4), 6);
					if (m_core->dma_status & (1 << 7))
						m_core->dma_op[7].timer->adjust(cycles_to_attotime(m_core->dma_op[7].src_count / 4), 7);
				}
			}
#endif
		}
		m_write_stalled_pending = false;
	}
}


void adsp21062_device::check_interrupts()
{
	if ((m_core->imask & m_core->irq_pending) && (m_core->mode1 & MODE1_IRPTEN) && !m_core->interrupt_active &&
		m_core->pc != m_core->delay_slot1 && m_core->pc != m_core->delay_slot2)
	{
		int which = 0;
		for (int i = 0; i < 32; i++)
		{
			if (BIT(m_core->irq_pending, i))
				break;
			which++;
		}

		PUSH_PC();
		if (m_core->idle)
			m_core->pcstk = m_core->pc + 1;
		else
			m_core->pcstk = m_core->daddr;

		m_core->irptl |= 1 << which;

		// TODO: timer and VIRPT interrupts also push the status stack
		if (which >= 6 && which <= 8)
			PUSH_STATUS_STACK();

		CHANGE_PC(0x20000 + (which * 0x4));

		/* TODO: alter IMASKP */

		m_core->active_irq_num = which;
		m_core->irq_pending &= ~(1 << which);

		m_core->interrupt_active = 1;
	}
}

void adsp21062_device::execute_run()
{
	if (m_enable_drc)
	{
		if (m_core->irq_pending != 0)
		{
			m_core->idle = 0;
		}
		execute_run_drc();
		return;
	}
	else
	{
		if (m_core->write_stalled)
			eat_cycles(m_core->icount);

		if (m_core->idle && m_core->irq_pending == 0)
		{
			debugger_wait_hook();

			int dma_count = m_core->icount;

			// run active DMAs even while idling
			while ((dma_count > 0) && (m_core->dma_status & ((1 << 6) | (1 << 7))))
			{
				if (!m_core->write_stalled)
				{
					dma_run_cycle(6);
					dma_run_cycle(7);
				}
				dma_count--;
			}

			m_core->icount = 0;
		}
		if (m_core->irq_pending != 0)
		{
			check_interrupts();
			m_core->idle = 0;
		}

		while (m_core->icount > 0 && !m_core->idle && !m_core->write_stalled)
		{
			m_core->pc = m_core->daddr;
			m_core->daddr = m_core->faddr;
			m_core->faddr = m_core->nfaddr;
			m_core->nfaddr++;

			m_core->astat_old_old_old = m_core->astat_old_old;
			m_core->astat_old_old = m_core->astat_old;
			m_core->astat_old = m_core->astat;

			debugger_instruction_hook(m_core->pc);

			m_core->opcode = m_program.read_qword(m_core->pc);

			// handle looping
			if (!(m_core->stky & LSEM) && (m_core->pc == m_core->laddr.addr))
			{
				switch (m_core->laddr.loop_type)
				{
				case 0:     // arithmetic condition-based
				{
					if ((m_core->pc - TOP_PC()) > 2)
					{
						m_core->astat = m_core->astat_old_old_old;
					}

					if (DO_CONDITION_CODE(m_core->laddr.code))
					{
						POP_LOOP();
						POP_PC();
					}
					else
					{
						CHANGE_PC(TOP_PC());
					}

					m_core->astat = m_core->astat_old;
					break;
				}
				case 1:     // counter-based, length 1
				{
					//throw emu_fatalerror("SHARC: counter-based loop, length 1 at %08X\n", m_pc);
					//break;
				}
				case 2:     // counter-based, length 2
				{
					//throw emu_fatalerror("SHARC: counter-based loop, length 2 at %08X\n", m_pc);
					//break;
				}
				case 3:     // counter-based, length >2
				{
					--m_core->curlcntr;
					if (m_core->curlcntr == 0)
					{
						POP_LOOP();
						POP_PC();
					}
					else
					{
						CHANGE_PC(TOP_PC());
					}
				}
				}
			}

			(this->*m_sharc_op[(m_core->opcode >> 39) & 0x1ff])();




			// System register latency effect
			if (m_core->systemreg_latency_cycles > 0)
			{
				--m_core->systemreg_latency_cycles;
				if (m_core->systemreg_latency_cycles <= 0)
				{
					systemreg_write_latency_effect();
				}
			}

			if (!m_core->write_stalled)
			{
				dma_run_cycle(6);
				dma_run_cycle(7);
			}

			--m_core->icount;
		};
	}
}
