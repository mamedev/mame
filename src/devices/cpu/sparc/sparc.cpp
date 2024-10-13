// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  sparc.cpp - Emulation for the SPARCv7/v8 line of
//              processors.
//
//  Notes:
//      - The CPU core implementation has been simplified
//        somewhat compared to the spec. In particular, bus
//        holding on read/write accesses is disabled, as there
//        is currently no use made of it, and it is unlikely to
//        ever be.
//
//  To-Do:
//      - Test: SPARCv8 ops are untested
//      - Extended-precision FPU support
//      - Coprocessor support
//      - Finish SPARClite peripherals
//
//================================================================

#include "emu.h"
#include "sparc.h"
#include "sparcdefs.h"

#include "softfloat3/source/include/softfloat.h"

#define LOG_BIU_CTRL            (1U << 1)
#define LOG_LOCK_CTRL           (1U << 2)
#define LOG_LOCK_CTRL_SAVE      (1U << 3)
#define LOG_CACHE_STATUS        (1U << 4)
#define LOG_RESTORE_LOCK_CTRL   (1U << 5)
#define LOG_SYSTEM_CTRL         (1U << 6)
#define LOG_SAME_PAGE_MASK      (1U << 7)
#define LOG_ADDR_RANGE          (1U << 8)
#define LOG_ADDR_MASK           (1U << 9)
#define LOG_WAIT_STATE          (1U << 10)
#define LOG_TIMER               (1U << 11)
#define LOG_TIMER_PRELOAD       (1U << 12)
#define LOG_UNMAPPED            (1U << 13)
#define LOG_ICACHE_LOCK         (1U << 14)
#define LOG_ICACHE_TAG          (1U << 15)
#define LOG_ICACHE_DATA         (1U << 16)
#define LOG_DCACHE_LOCK         (1U << 17)
#define LOG_DCACHE_TAG          (1U << 18)
#define LOG_DCACHE_DATA         (1U << 19)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SPARCV7, sparcv7_device, "sparcv7", "Sun SPARC v7")
DEFINE_DEVICE_TYPE(SPARCV8, sparcv8_device, "sparcv8", "Sun SPARC v8")
DEFINE_DEVICE_TYPE(MB86930, mb86930_device, "mb86930", "Fujitsu MB86930 'SPARClite'")

#if LOG_FCODES
#include "ss1fcode.ipp"
#endif

namespace
{
const sparc_disassembler::asi_desc_map::value_type mb86930_asi_desc[] =
{
	{ 0x01, { nullptr, "Control Registers"          } },
	{ 0x02, { nullptr, "Instruction Cache Lock"     } },
	{ 0x03, { nullptr, "Data Cache Lock"            } },
	{ 0x08, { nullptr, "User Instruction"           } },
	{ 0x09, { nullptr, "Supervisor Instruction"     } },
	{ 0x0a, { nullptr, "User Data"                  } },
	{ 0x0b, { nullptr, "Supervisor Data"            } },
	{ 0x0c, { nullptr, "Instruction Cache Tag RAM"  } },
	{ 0x0d, { nullptr, "Instruction Cache Data RAM" } },
	{ 0x0e, { nullptr, "Data Cache Tag RAM"         } },
	{ 0x0f, { nullptr, "Data Cache Data RAM"        } }
};
} // anonymous namespace

const char *const sparc_base_device::DEFAULT_ASI_NAMES[16] =
{
	"asi0",  "asi1",  "asi2",  "asi3",  "asi4",  "asi5",  "asi6",  "asi7",
	"asi8",  "asi9",  "asi10", "asi11", "asi12", "asi13", "asi14", "asi15"
};

//-------------------------------------------------
//  sparc_base_device - constructor
//-------------------------------------------------

sparc_base_device::sparc_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_mmu(*this, finder_base::DUMMY_TAG)
{
	m_debugger_config = address_space_config("debug", ENDIANNESS_BIG, 32, 32);

	if (type != MB86930)
	{
		for (int i = 0; i < 0x10; i++)
		{
			m_asi_config[i] = address_space_config(DEFAULT_ASI_NAMES[i], ENDIANNESS_BIG, 32, 32);
		}
	}
}


//-------------------------------------------------
//  sparcv7_device - constructors
//-------------------------------------------------

sparcv7_device::sparcv7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sparcv7_device(mconfig, SPARCV7, tag, owner, clock)
{
}

sparcv7_device::sparcv7_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sparc_base_device(mconfig, type, tag, owner, clock)
{
}


//-------------------------------------------------
//  sparcv8_device - constructor
//-------------------------------------------------

sparcv8_device::sparcv8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sparcv8_device(mconfig, SPARCV8, tag, owner, clock)
{
}


sparcv8_device::sparcv8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sparc_base_device(mconfig, type, tag, owner, clock)
{
}


//-------------------------------------------------
//  mb86930_device - constructor
//-------------------------------------------------

mb86930_device::mb86930_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sparcv8_device(mconfig, MB86930, tag, owner, clock)
	, m_cs_r(*this, 0)
	, m_cs_w(*this)
{
	m_asi_config[0x00] = address_space_config("debugger",       ENDIANNESS_BIG, 32, 32);
	m_asi_config[0x01] = address_space_config("system_control", ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::control_map), this));
	m_asi_config[0x02] = address_space_config("icache_lock",    ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::icache_lock_map), this));
	m_asi_config[0x03] = address_space_config("dcache_lock",    ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::dcache_lock_map), this));
	m_asi_config[0x04] = address_space_config("asi4",           ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<4>), this));
	m_asi_config[0x05] = address_space_config("asi5",           ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<5>), this));
	m_asi_config[0x06] = address_space_config("asi6",           ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<6>), this));
	m_asi_config[0x07] = address_space_config("asi7",           ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<7>), this));
	m_asi_config[0x08] = address_space_config("user_insn",      ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<8>), this));
	m_asi_config[0x09] = address_space_config("super_insn",     ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<9>), this));
	m_asi_config[0x0a] = address_space_config("user_data",      ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<10>), this));
	m_asi_config[0x0b] = address_space_config("super_data",     ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::mmu_map<11>), this));
	m_asi_config[0x0c] = address_space_config("icache_tag",     ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::icache_tag_map), this));
	m_asi_config[0x0d] = address_space_config("icache_data",    ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::icache_data_map), this));
	m_asi_config[0x0e] = address_space_config("dcache_tag",     ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::dcache_tag_map), this));
	m_asi_config[0x0f] = address_space_config("dcache_data",    ENDIANNESS_BIG, 32, 32, 0, address_map_constructor(FUNC(mb86930_device::dcache_data_map), this));

	add_asi_desc([](sparc_disassembler *dasm) { dasm->add_asi_desc(mb86930_asi_desc); });
}


void sparc_base_device::device_start()
{
#if LOG_FCODES
	m_ss1_fcode_table.clear();
	FILE* input = fopen("names.txt", "rb");

	if (input != NULL)
	{
		fseek(input, 0, SEEK_END);
		size_t filesize = ftell(input);
		fseek(input, 0, SEEK_SET);

		uint8_t *buf = new uint8_t[filesize];
		fread(buf, 1, filesize, input);
		fclose(input);

		size_t pos = 0;
		while (pos < filesize)
		{
			// eat newlines
			while (pos < filesize && (buf[pos] == 0x0d || buf[pos] == 0x0a))
				pos++;

			if (pos >= filesize)
				break;

			// get opcode
			uint16_t opcode = 0;
			for (int shift = 12; shift >= 0 && pos < filesize; shift -= 4)
			{
				uint8_t digit = buf[pos];
				if (digit >= 'a' && digit <= 'z')
				{
					digit &= ~0x20;
				}

				if (digit >= '0' && digit <= '9')
				{
					opcode |= (digit - 0x30) << shift;
				}
				else if (digit >= 'A' && digit <= 'F')
				{
					opcode |= ((digit - 0x41) + 10) << shift;
				}
				pos++;
			}

			if (pos >= filesize)
				break;

			// skip " : "
			pos += 3;

			if (pos >= filesize)
				break;

			// read description up to the first space
			std::string description;
			while (buf[pos] != ' ' && pos < filesize)
			{
				description += char(buf[pos]);
				pos++;
			}

			if (pos >= filesize)
				break;

			// skip everything else up to the trailing semicolon
			while (buf[pos] != ';' && pos < filesize)
				pos++;

			if (pos >= filesize)
				break;

			if (buf[pos] == ';')
				pos++;

			m_ss1_fcode_table[opcode] = description;
		}
		delete [] buf;
	}
	m_log_fcodes = false;
#endif

	NWINDOWS = 7;
	PSR = 0;
	m_ver = 0;
	m_impl = 0;

	m_bp_reset_in = false;
	m_bp_fpu_present = true;
	m_bp_cp_present = false;
	m_pb_error = false;
	m_pb_block_ldst_byte = false;
	m_pb_block_ldst_word = false;
	m_bp_irl = 0;
	m_irq_state = 0;

	for (int i = 0; i < 0x20; i++)
	{
		if (i > 0 && i < 0x10)
		{
			continue;
		}
		space(i).specific(m_asi[i]);
	}

	std::fill_n(m_dbgregs, std::size(m_dbgregs), 0);

	std::fill_n(m_illegal_instruction_asr, std::size(m_illegal_instruction_asr), false);
	std::fill_n(m_privileged_asr, std::size(m_privileged_asr), true);
	m_privileged_asr[0] = false;

	std::fill_n(m_alu_setcc, std::size(m_alu_setcc), false);
	m_alu_setcc[OP3_ADDCC] = true;
	m_alu_setcc[OP3_ANDCC] = true;
	m_alu_setcc[OP3_ORCC] = true;
	m_alu_setcc[OP3_XORCC] = true;
	m_alu_setcc[OP3_SUBCC] = true;
	m_alu_setcc[OP3_ANDNCC] = true;
	m_alu_setcc[OP3_ORNCC] = true;
	m_alu_setcc[OP3_XNORCC] = true;
	m_alu_setcc[OP3_ADDXCC] = true;
	m_alu_setcc[OP3_SUBXCC] = true;
	m_alu_setcc[OP3_TADDCC] = true;
	m_alu_setcc[OP3_TSUBCC] = true;
	m_alu_setcc[OP3_TADDCCTV] = true;
	m_alu_setcc[OP3_TSUBCCTV] = true;
	m_alu_setcc[OP3_MULSCC] = true;

	std::fill_n(m_alu_op3_assigned, std::size(m_alu_op3_assigned), false);
	m_alu_op3_assigned[OP3_ADD] = true;
	m_alu_op3_assigned[OP3_AND] = true;
	m_alu_op3_assigned[OP3_OR] = true;
	m_alu_op3_assigned[OP3_XOR] = true;
	m_alu_op3_assigned[OP3_SUB] = true;
	m_alu_op3_assigned[OP3_ANDN] = true;
	m_alu_op3_assigned[OP3_ORN] = true;
	m_alu_op3_assigned[OP3_XNOR] = true;
	m_alu_op3_assigned[OP3_ADDX] = true;
	m_alu_op3_assigned[OP3_SUBX] = true;
	m_alu_op3_assigned[OP3_ADDCC] = true;
	m_alu_op3_assigned[OP3_ANDCC] = true;
	m_alu_op3_assigned[OP3_ORCC] = true;
	m_alu_op3_assigned[OP3_XORCC] = true;
	m_alu_op3_assigned[OP3_SUBCC] = true;
	m_alu_op3_assigned[OP3_ANDNCC] = true;
	m_alu_op3_assigned[OP3_ORNCC] = true;
	m_alu_op3_assigned[OP3_XNORCC] = true;
	m_alu_op3_assigned[OP3_ADDXCC] = true;
	m_alu_op3_assigned[OP3_SUBXCC] = true;
	m_alu_op3_assigned[OP3_TADDCC] = true;
	m_alu_op3_assigned[OP3_TSUBCC] = true;
	m_alu_op3_assigned[OP3_TADDCCTV] = true;
	m_alu_op3_assigned[OP3_TSUBCCTV] = true;
	m_alu_op3_assigned[OP3_MULSCC] = true;
	m_alu_op3_assigned[OP3_SLL] = true;
	m_alu_op3_assigned[OP3_SRL] = true;
	m_alu_op3_assigned[OP3_SRA] = true;
	m_alu_op3_assigned[OP3_RDASR] = true;
	m_alu_op3_assigned[OP3_RDPSR] = true;
	m_alu_op3_assigned[OP3_RDWIM] = true;
	m_alu_op3_assigned[OP3_RDTBR] = true;
	m_alu_op3_assigned[OP3_WRASR] = true;
	m_alu_op3_assigned[OP3_WRPSR] = true;
	m_alu_op3_assigned[OP3_WRWIM] = true;
	m_alu_op3_assigned[OP3_WRTBR] = true;
	m_alu_op3_assigned[OP3_FPOP1] = true;
	m_alu_op3_assigned[OP3_FPOP2] = true;
	m_alu_op3_assigned[OP3_CPOP1] = true;
	m_alu_op3_assigned[OP3_CPOP2] = true;
	m_alu_op3_assigned[OP3_JMPL] = true;
	m_alu_op3_assigned[OP3_RETT] = true;
	m_alu_op3_assigned[OP3_TICC] = true;
	m_alu_op3_assigned[OP3_IFLUSH] = true;
	m_alu_op3_assigned[OP3_SAVE] = true;
	m_alu_op3_assigned[OP3_RESTORE] = true;

	std::fill_n(m_ldst_op3_assigned, std::size(m_ldst_op3_assigned), false);
	m_ldst_op3_assigned[OP3_LD] = true;
	m_ldst_op3_assigned[OP3_LDUB] = true;
	m_ldst_op3_assigned[OP3_LDUH] = true;
	m_ldst_op3_assigned[OP3_LDD] = true;
	m_ldst_op3_assigned[OP3_ST] = true;
	m_ldst_op3_assigned[OP3_STB] = true;
	m_ldst_op3_assigned[OP3_STH] = true;
	m_ldst_op3_assigned[OP3_STD] = true;
	m_ldst_op3_assigned[OP3_LDSB] = true;
	m_ldst_op3_assigned[OP3_LDSH] = true;
	m_ldst_op3_assigned[OP3_LDSTUB] = true;
	m_ldst_op3_assigned[OP3_LDA] = true;
	m_ldst_op3_assigned[OP3_LDUBA] = true;
	m_ldst_op3_assigned[OP3_LDUHA] = true;
	m_ldst_op3_assigned[OP3_LDDA] = true;
	m_ldst_op3_assigned[OP3_STA] = true;
	m_ldst_op3_assigned[OP3_STBA] = true;
	m_ldst_op3_assigned[OP3_STHA] = true;
	m_ldst_op3_assigned[OP3_STDA] = true;
	m_ldst_op3_assigned[OP3_LDSBA] = true;
	m_ldst_op3_assigned[OP3_LDSHA] = true;
	m_ldst_op3_assigned[OP3_LDSTUBA] = true;
	m_ldst_op3_assigned[OP3_LDFPR] = true;
	m_ldst_op3_assigned[OP3_LDFSR] = true;
	m_ldst_op3_assigned[OP3_LDDFPR] = true;
	m_ldst_op3_assigned[OP3_STFPR] = true;
	m_ldst_op3_assigned[OP3_STFSR] = true;
	m_ldst_op3_assigned[OP3_STDFQ] = true;
	m_ldst_op3_assigned[OP3_STDFPR] = true;

	// register our state for the debugger
	state_add(STATE_GENPC,      "GENPC",    m_pc).noshow();
	state_add(STATE_GENPCBASE,  "CURPC",    m_pc).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS", m_psr).callimport().callexport().formatstr("%6s").noshow();
	state_add(SPARC_PC,         "PC",       m_pc).formatstr("%08X");
	state_add(SPARC_NPC,        "nPC",      m_npc).formatstr("%08X");
	state_add(SPARC_PSR,        "PSR",      m_psr).formatstr("%08X");
	state_add(SPARC_WIM,        "WIM",      m_wim).formatstr("%08X");
	state_add(SPARC_TBR,        "TBR",      m_tbr).formatstr("%08X");
	state_add(SPARC_Y,          "Y",        m_y).formatstr("%08X");
	state_add(SPARC_ANNUL,      "ANNUL",    m_no_annul).formatstr("%01u");
	state_add(SPARC_ICC,        "icc",      m_icc).formatstr("%4s");
	state_add(SPARC_CWP,        "CWP",      m_cwp).formatstr("%2d");

	for (int i = 0; i < 8; i++)
		state_add(SPARC_G0 + i, util::string_format("g%d", i).c_str(), m_r[i]).formatstr("%08X");

	for (int i = 0; i < 8; i++)
		state_add(SPARC_O0 + i, util::string_format("o%d", i).c_str(), m_dbgregs[i]).formatstr("%08X");

	for (int i = 0; i < 8; i++)
		state_add(SPARC_L0 + i, util::string_format("l%d", i).c_str(), m_dbgregs[8+i]).formatstr("%08X");

	for (int i = 0; i < 8; i++)
		state_add(SPARC_I0 + i, util::string_format("i%d", i).c_str(), m_dbgregs[16+i]).formatstr("%08X");

	state_add(SPARC_EC,     "EC",       m_ec).formatstr("%1u");
	state_add(SPARC_EF,     "EF",       m_ef).formatstr("%1u");
	state_add(SPARC_ET,     "ET",       m_et).formatstr("%1u");
	state_add(SPARC_PIL,    "PIL",      m_pil).formatstr("%2d");
	state_add(SPARC_S,      "S",        m_s).formatstr("%1u");
	state_add(SPARC_PS,     "PS",       m_ps).formatstr("%1u");
	state_add(SPARC_FSR,    "FSR",      m_fsr).formatstr("%08X");

	for (int i = 0; i < 32; i++)
		state_add(SPARC_F0 + i, util::string_format("f%d", i).c_str(), m_fpr[i]);

	for (int i = 0; i < 136; i++)
		state_add(SPARC_R0 + i, util::string_format("r%d", i).c_str(), m_r[i]).formatstr("%08X");

	save_item(NAME(m_r));
	save_item(NAME(m_fpr));
	save_item(NAME(m_fsr));
	save_item(NAME(m_ftt));
	save_item(NAME(m_pc));
	save_item(NAME(m_npc));
	save_item(NAME(m_psr));
	save_item(NAME(m_wim));
	save_item(NAME(m_tbr));
	save_item(NAME(m_y));
	save_item(NAME(m_bp_reset_in));
	save_item(NAME(m_bp_irl));
	save_item(NAME(m_bp_fpu_present));
	save_item(NAME(m_bp_cp_present));
	save_item(NAME(m_pb_error));
	save_item(NAME(m_pb_block_ldst_byte));
	save_item(NAME(m_pb_block_ldst_word));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_trap));
	save_item(NAME(m_tt));
	save_item(NAME(m_ticc_trap_type));
	save_item(NAME(m_interrupt_level));
	save_item(NAME(m_privileged_instruction));
	save_item(NAME(m_illegal_instruction));
	save_item(NAME(m_mem_address_not_aligned));
	save_item(NAME(m_fp_disabled));
	save_item(NAME(m_cp_disabled));
	save_item(NAME(m_fp_exception));
	save_item(NAME(m_fp_exception_pending));
	save_item(NAME(m_fpr_pending));
	save_item(NAME(m_pending_fpr));
	save_item(NAME(m_cp_exception));
	save_item(NAME(m_instruction_access_exception));
	save_item(NAME(m_data_access_exception));
	save_item(NAME(m_trap_instruction));
	save_item(NAME(m_window_underflow));
	save_item(NAME(m_window_overflow));
	save_item(NAME(m_tag_overflow));
	save_item(NAME(m_reset_mode));
	save_item(NAME(m_reset_trap));
	save_item(NAME(m_execute_mode));
	save_item(NAME(m_error_mode));
	save_item(NAME(m_fpu_sequence_err));
	save_item(NAME(m_cp_sequence_err));
	save_item(NAME(m_impl));
	save_item(NAME(m_ver));
	save_item(NAME(m_icc));
	save_item(NAME(m_ec));
	save_item(NAME(m_ef));
	save_item(NAME(m_pil));
	save_item(NAME(m_s));
	save_item(NAME(m_ps));
	save_item(NAME(m_et));
	save_item(NAME(m_cwp));
	save_item(NAME(m_alu_op3_assigned));
	save_item(NAME(m_ldst_op3_assigned));
	save_item(NAME(m_alu_setcc));
	save_item(NAME(m_nwindows));
	save_item(NAME(m_privileged_asr));
	save_item(NAME(m_illegal_instruction_asr));
	save_item(NAME(m_mae));
	save_item(NAME(m_no_annul));
	save_item(NAME(m_hold_bus));
	save_item(NAME(m_insn_space));
	save_item(NAME(m_data_space));

#if LOG_FCODES
	save_item(NAME(m_ss1_next_pc));
	save_item(NAME(m_ss1_next_opcode));
	save_item(NAME(m_ss1_next_handler_base));
	save_item(NAME(m_ss1_next_entry_point));
	save_item(NAME(m_ss1_next_stack));
	save_item(NAME(m_log_fcodes));
#endif

	// set our instruction counter
	set_icountptr(m_icount);
}


void sparc_base_device::device_resolve_objects()
{
	if (m_mmu.found())
		m_mmu->set_host(this);
}

void sparc_base_device::device_reset()
{
	m_trap = false;
	m_tt = 0;
	m_ticc_trap_type = 0;
	m_interrupt_level = 0;
	m_privileged_instruction = false;
	m_illegal_instruction = false;
	m_mem_address_not_aligned = false;
	m_fp_disabled = false;
	m_cp_disabled = false;
	m_fp_exception = false;
	m_fp_exception_pending = false;
	m_fpr_pending = 0;
	m_pending_fpr = std::size(m_fpr);
	m_cp_exception = false;
	m_instruction_access_exception = false;
	m_data_access_exception = false;
	m_trap_instruction = false;
	m_window_underflow = false;
	m_window_overflow = false;
	m_tag_overflow = false;
	m_reset_mode = true;
	m_reset_trap = false;
	m_execute_mode = false;
	m_error_mode = false;
	m_fpu_sequence_err = 0;
	m_cp_sequence_err = 0;

	m_bp_irl = 0;
	m_irq_state = 0;

	MAE = false;
	HOLD_BUS = false;
	m_no_annul = true;

	PC = 0;
	nPC = 4;
	std::fill_n(m_r, std::size(m_r), 0);
	std::fill_n(m_fpr, std::size(m_fpr), 0);

	WIM = 0;
	TBR = 0;
	Y = 0;

	PSR = (PSR & PSR_ZERO_MASK) | (PSR_S_MASK | PSR_PS_MASK);
	m_s = true;
	m_ps = true;
	m_data_space = 11;
	m_pil = 0;
	m_et = false;
	m_icc = 0;
	m_ec = false;
	m_ef = false;
	m_cwp = 0;

	for (int i = 0; i < 8; i++)
	{
		m_regs[i] = m_r + i;
	}
	update_gpr_pointers();

#if LOG_FCODES
	m_ss1_next_pc = ~0;
	m_ss1_next_opcode = ~0;
	m_ss1_next_handler_base = ~0;
	m_ss1_next_entry_point = ~0;
	m_ss1_next_stack = ~0;
#endif
}

void sparcv8_device::device_start()
{
	sparc_base_device::device_start();

	save_item(NAME(m_unimplemented_FLUSH));
	save_item(NAME(m_r_register_access_error));
	save_item(NAME(m_instruction_access_error));
	save_item(NAME(m_data_access_error));
	save_item(NAME(m_data_store_error));
	save_item(NAME(m_division_by_zero));

	m_alu_setcc[OP3_UMULCC] = true;
	m_alu_setcc[OP3_SMULCC] = true;
	m_alu_setcc[OP3_UDIVCC] = true;
	m_alu_setcc[OP3_SDIVCC] = true;

	m_alu_op3_assigned[OP3_UMUL] = true;
	m_alu_op3_assigned[OP3_SMUL] = true;
	m_alu_op3_assigned[OP3_UDIV] = true;
	m_alu_op3_assigned[OP3_SDIV] = true;
	m_alu_op3_assigned[OP3_UMULCC] = true;
	m_alu_op3_assigned[OP3_SMULCC] = true;
	m_alu_op3_assigned[OP3_UDIVCC] = true;
	m_alu_op3_assigned[OP3_SDIVCC] = true;

	m_ldst_op3_assigned[OP3_SWAP] = true;
	m_ldst_op3_assigned[OP3_SWAPA] = true;
	m_ldst_op3_assigned[OP3_LDCPR] = true;
	m_ldst_op3_assigned[OP3_LDCSR] = true;
	m_ldst_op3_assigned[OP3_LDDCPR] = true;
	m_ldst_op3_assigned[OP3_STCPR] = true;
	m_ldst_op3_assigned[OP3_STCSR] = true;
	m_ldst_op3_assigned[OP3_STDCQ] = true;
	m_ldst_op3_assigned[OP3_STDCPR] = true;
}

void sparcv8_device::device_reset()
{
	sparc_base_device::device_reset();

	m_unimplemented_FLUSH = 0;
	m_r_register_access_error = 0;
	m_instruction_access_error = 0;
	m_data_access_error = 0;
	m_data_store_error = 0;
	m_division_by_zero = 0;
}


//-------------------------------------------------
//  mb86930_device - initializers
//-------------------------------------------------

void mb86930_device::device_start()
{
	sparcv8_device::device_start();

	NWINDOWS = 8;
	PSR |= 2 << PSR_VER_SHIFT;
	m_ver = 2;

	m_bp_cp_present = false;
	m_bp_fpu_present = false;

	m_alu_setcc[OP3_DIVSCC] = true;

	m_alu_op3_assigned[OP3_DIVSCC] = true;
	m_alu_op3_assigned[OP3_SCAN] = true;

	save_item(NAME(m_ssctrl));
	save_item(NAME(m_spmr));
	save_item(NAME(m_spmr_mask));
	save_item(NAME(m_arsr));
	save_item(NAME(m_amr));
	save_item(NAME(m_wssr));
	save_item(NAME(m_last_masked_addr));

	std::fill_n(&m_arsr[0], 6, 0);
	std::fill_n(&m_amr[0], 6, 0);
}

void mb86930_device::device_reset()
{
	sparcv8_device::device_reset();

	m_ssctrl = 0x08;
	m_spmr = 0;
	m_spmr_mask = ~0ULL;
	m_last_masked_addr = 0ULL;

	std::fill_n(&m_wssr[0], 3, 0);
	m_wssr[0] = 0x1ffd << 6;

	m_arsr[0] = (9 << 23);
	m_amr[0] = (0x1f << 1);

	update_addr_masks();
	update_wait_states();
}


//-------------------------------------------------
//  mb86930_device - internal maps
//-------------------------------------------------

void mb86930_device::control_map(address_map &map)
{
	map(0x00000000, 0x00000003).rw(FUNC(mb86930_device::biu_ctrl_r), FUNC(mb86930_device::biu_ctrl_w));
	map(0x00000004, 0x00000007).rw(FUNC(mb86930_device::lock_ctrl_r), FUNC(mb86930_device::lock_ctrl_w));
	map(0x00000008, 0x0000000b).rw(FUNC(mb86930_device::lock_ctrl_save_r), FUNC(mb86930_device::lock_ctrl_save_w));
	map(0x0000000c, 0x0000000f).rw(FUNC(mb86930_device::cache_status_r), FUNC(mb86930_device::cache_status_w));
	map(0x00000010, 0x00000013).rw(FUNC(mb86930_device::restore_lock_ctrl_r), FUNC(mb86930_device::restore_lock_ctrl_w));
	map(0x00000080, 0x00000083).rw(FUNC(mb86930_device::system_support_ctrl_r), FUNC(mb86930_device::system_support_ctrl_w));
	map(0x00000120, 0x00000123).rw(FUNC(mb86930_device::same_page_mask_r), FUNC(mb86930_device::same_page_mask_w));
	map(0x00000124, 0x00000137).rw(FUNC(mb86930_device::addr_range_spec_r), FUNC(mb86930_device::addr_range_spec_w));
	map(0x00000140, 0x00000157).rw(FUNC(mb86930_device::addr_mask_r), FUNC(mb86930_device::addr_mask_w));
	map(0x00000160, 0x0000016b).rw(FUNC(mb86930_device::wait_specifier_r), FUNC(mb86930_device::wait_specifier_w));
	map(0x00000174, 0x00000177).rw(FUNC(mb86930_device::timer_r), FUNC(mb86930_device::timer_w));
	map(0x00000178, 0x0000017b).rw(FUNC(mb86930_device::timer_preload_r), FUNC(mb86930_device::timer_preload_w));
}

void mb86930_device::icache_lock_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mb86930_device::icache_lock_r), FUNC(mb86930_device::icache_lock_w));
}

void mb86930_device::dcache_lock_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mb86930_device::dcache_lock_r), FUNC(mb86930_device::dcache_lock_w));
}

void mb86930_device::icache_tag_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mb86930_device::icache_tag_r), FUNC(mb86930_device::icache_tag_w));
}

void mb86930_device::icache_data_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mb86930_device::icache_data_r), FUNC(mb86930_device::icache_data_w));
}

void mb86930_device::dcache_tag_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mb86930_device::dcache_tag_r), FUNC(mb86930_device::dcache_tag_w));
}

void mb86930_device::dcache_data_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mb86930_device::dcache_data_r), FUNC(mb86930_device::dcache_data_w));
}


//-------------------------------------------------
//  mb86930_device - cache control (TODO)
//-------------------------------------------------

uint32_t mb86930_device::icache_lock_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_ICACHE_LOCK, "%s: icache_lock_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::icache_lock_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_ICACHE_LOCK, "%s: icache_lock_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::dcache_lock_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_DCACHE_LOCK, "%s: dcache_lock_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::dcache_lock_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_DCACHE_LOCK, "%s: dcache_lock_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::icache_tag_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_ICACHE_TAG, "%s: icache_tag_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::icache_tag_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_ICACHE_TAG, "%s: icache_tag_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::icache_data_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_ICACHE_DATA, "%s: icache_data_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::icache_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_ICACHE_DATA, "%s: icache_data_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::dcache_tag_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_DCACHE_TAG, "%s: dcache_tag_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::dcache_tag_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_DCACHE_TAG, "%s: dcache_tag_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::dcache_data_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_DCACHE_DATA, "%s: dcache_data_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::dcache_data_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_DCACHE_DATA, "%s: dcache_data_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


//-------------------------------------------------
//  mb86930_device - MMU map and handlers
//-------------------------------------------------

template <uint8_t Asi>
void mb86930_device::mmu_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(mb86930_device::mmu_r<Asi>), FUNC(mb86930_device::mmu_w<Asi>));
}

template <uint8_t Asi>
uint32_t mb86930_device::mmu_r(offs_t offset, uint32_t mem_mask)
{
	const uint64_t full_addr = ((uint64_t)Asi << 30) | offset;

	const uint64_t masked_addr = full_addr & m_spmr_mask;
	const bool is_same_page = (masked_addr == m_last_masked_addr);
	m_last_masked_addr = masked_addr;

	for (int cs = 0; cs < 6; cs++)
	{
		if ((full_addr & m_full_masks[cs]) == m_full_ranges[cs])
		{
			m_icount -= is_same_page ? m_same_page_waits[cs] : m_other_page_waits[cs];
			return m_cs_r[cs](offset, mem_mask);
		}
	}
	LOGMASKED(LOG_UNMAPPED, "%s: mmu_r, unmapped access: ASI %d, address %08x & %08x\n", machine().describe_context(), Asi, offset << 2, mem_mask);
	return 0;
}

template <uint8_t Asi> void mb86930_device::mmu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint64_t full_addr = ((uint64_t)Asi << 30) | offset;

	const uint64_t masked_addr = full_addr & m_spmr_mask;
	const bool is_same_page = (masked_addr == m_last_masked_addr);
	m_last_masked_addr = masked_addr;

	for (int cs = 0; cs < 6; cs++)
	{
		if ((full_addr & m_full_masks[cs]) == m_full_ranges[cs])
		{
			m_icount -= is_same_page ? m_same_page_waits[cs] : m_other_page_waits[cs];
			m_cs_w[cs](offset, data, mem_mask);
			return;
		}
	}
	LOGMASKED(LOG_UNMAPPED, "%s: mmu_w, unmapped access: ASI %d, address %08x = %08x & %08x\n", machine().describe_context(), Asi, offset << 2, data, mem_mask);
}


//-------------------------------------------------
//  mb86930_device - system control handlers
//-------------------------------------------------

void mb86930_device::update_addr_masks()
{
	for (int i = 0; i < 6; i++)
	{
		m_full_masks[i] = ~(((uint64_t)m_amr[i] << 7) | 0xff);
		m_full_ranges[i] = ((uint64_t)m_arsr[i] << 7) & m_full_masks[i];
	}
}

void mb86930_device::update_wait_states()
{
	for (int i = 0; i < 6; i++)
	{
		const uint8_t shift = (i & 1) ? 19 : 6;
		const bool enable = bool(BIT(m_wssr[i >> 1], shift + 2));
		const bool single = bool(BIT(m_wssr[i >> 1], shift + 1));
		//const bool override = bool(BIT(m_wssr[i >> 1], shift + 0));

		if (BIT(m_ssctrl, 3) && !single && enable)
		{
			const uint8_t count1 = ((m_wssr[i >> 1] >> (shift + 8)) & 0x1f) + 1;
			const uint8_t count2 = ((m_wssr[i >> 1] >> (shift + 3)) & 0x1f) + 1;
			m_other_page_waits[i] = count1;
			m_same_page_waits[i] = BIT(m_ssctrl, 5) ? count2 : count1;
		}
		else
		{
			m_other_page_waits[i] = 0;
			m_same_page_waits[i] = 0;
		}
	}
}

uint32_t mb86930_device::biu_ctrl_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_BIU_CTRL, "%s: biu_ctrl_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::biu_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_BIU_CTRL, "%s: biu_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::lock_ctrl_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_LOCK_CTRL, "%s: lock_ctrl_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::lock_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_LOCK_CTRL, "%s: lock_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::lock_ctrl_save_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_LOCK_CTRL_SAVE, "%s: lock_ctrl_save_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::lock_ctrl_save_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_LOCK_CTRL_SAVE, "%s: lock_ctrl_save_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::cache_status_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_CACHE_STATUS, "%s: cache_status_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::cache_status_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CACHE_STATUS, "%s: cache_status_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::restore_lock_ctrl_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_RESTORE_LOCK_CTRL, "%s: restore_lock_ctrl_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::restore_lock_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_RESTORE_LOCK_CTRL, "%s: restore_lock_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::system_support_ctrl_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = m_ssctrl;
	LOGMASKED(LOG_SYSTEM_CTRL, "%s: system_ctrl_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::system_support_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SYSTEM_CTRL, "%s: system_ctrl_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ssctrl);
	update_wait_states();
}


uint32_t mb86930_device::same_page_mask_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = m_spmr;
	LOGMASKED(LOG_SAME_PAGE_MASK, "%s: same_page_mask_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::same_page_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SAME_PAGE_MASK, "%s: same_page_mask_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_spmr);
	m_spmr_mask = ~(((uint64_t)m_spmr << 7) | 0xff);
}


uint32_t mb86930_device::addr_range_spec_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = m_arsr[offset + 1];
	LOGMASKED(LOG_ADDR_RANGE, "%s: addr_range_spec_r[%d]: %08x & %08x\n", machine().describe_context(), offset + 1, data, mem_mask);
	return data;
}

void mb86930_device::addr_range_spec_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_ADDR_RANGE, "%s: addr_range_spec_w[%d]: %08x & %08x\n", machine().describe_context(), offset + 1, data, mem_mask);
	COMBINE_DATA(&m_arsr[offset + 1]);
	update_addr_masks();
}


uint32_t mb86930_device::addr_mask_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = m_amr[offset];
	LOGMASKED(LOG_ADDR_MASK, "%s: addr_mask_r[%d]: %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	return data;
}

void mb86930_device::addr_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_ADDR_MASK, "%s: addr_mask_w[%d]: %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	COMBINE_DATA(&m_amr[offset]);
	update_addr_masks();
}


uint32_t mb86930_device::wait_specifier_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = m_wssr[offset];
	LOGMASKED(LOG_WAIT_STATE, "%s: wait_state_r[%d]: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::wait_specifier_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_WAIT_STATE, "%s: wait_state_w[%d]: %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	COMBINE_DATA(&m_wssr[offset]);
	update_wait_states();
}


uint32_t mb86930_device::timer_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_TIMER, "%s: timer_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::timer_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TIMER, "%s: timer_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


uint32_t mb86930_device::timer_preload_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0;
	LOGMASKED(LOG_TIMER_PRELOAD, "%s: timer_preload_r: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void mb86930_device::timer_preload_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TIMER_PRELOAD, "%s: timer_preload_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


//-------------------------------------------------
//  device_post_load - update register pointers
//  after loading a savestate
//-------------------------------------------------

void sparc_base_device::device_post_load()
{
	update_gpr_pointers();
}

void mb86930_device::device_post_load()
{
	sparcv8_device::device_post_load();
	update_addr_masks();
	update_wait_states();
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector sparc_base_device::memory_space_config() const
{
	space_config_vector config_vector;
	config_vector.push_back(std::make_pair(0, &m_debugger_config));
	for (int i = 0; i < 0x10; i++)
	{
		config_vector.push_back(std::make_pair(0x10 + i, &m_asi_config[i]));
	}
	return config_vector;
}

inline uint32_t sparc_base_device::read_word(const uint8_t asi, const uint32_t address, const uint32_t mem_mask)
{
	assert(asi < 0x10); // We do not currently support ASIs outside the range used by actual Sun machines.
	return m_asi[asi | 0x10].read_dword(address, mem_mask);
}

inline void sparc_base_device::write_word(const uint8_t asi, const uint32_t address, const uint32_t data, const uint32_t mem_mask)
{
	assert(asi < 0x10); // We do not currently support ASIs outside the range used by actual Sun machines.
	return m_asi[asi | 0x10].write_dword(address, data, mem_mask);
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void sparc_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		case SPARC_ANNUL:
			str = string_format("%01u", m_no_annul ? 0 : 1);
			break;
		case SPARC_CWP:
			str = string_format("%2u", PSR & PSR_CWP_MASK);
			break;
		case SPARC_EC:
			str = string_format("%01u", PSR & PSR_EC_MASK ? 1 : 0);
			break;
		case SPARC_EF:
			str = string_format("%01u", PSR & PSR_EF_MASK ? 1 : 0);
			break;
		case SPARC_ET:
			str = string_format("%01u", PSR & PSR_ET_MASK ? 1 : 0);
			break;
		case SPARC_PS:
			str = string_format("%01u", PSR & PSR_PS_MASK ? 1 : 0);
			break;
		case SPARC_S:
			str = string_format("%01u", PSR & PSR_S_MASK ? 1 : 0);
			break;
		case SPARC_PIL:
			str = string_format("%02u", (PSR & PSR_PIL_MASK) >> PSR_PIL_SHIFT);
			break;
		case SPARC_ICC:
			str = string_format("%c%c%c%c", ICC_N_SET ? 'n' : ' ', ICC_Z_SET ? 'z' : ' ', ICC_V_SET ? 'v' : ' ', ICC_C_SET ? 'c' : ' ');
			break;
		case SPARC_O0:  case SPARC_O1:  case SPARC_O2:  case SPARC_O3:  case SPARC_O4:  case SPARC_O5:  case SPARC_O6:  case SPARC_O7:
			str = string_format("%08X", m_dbgregs[entry.index() - SPARC_O0]);
			break;
		case SPARC_L0:  case SPARC_L1:  case SPARC_L2:  case SPARC_L3:  case SPARC_L4:  case SPARC_L5:  case SPARC_L6:  case SPARC_L7:
			str = string_format("%08X", m_dbgregs[8 + (entry.index() - SPARC_L0)]);
			break;
		case SPARC_I0:  case SPARC_I1:  case SPARC_I2:  case SPARC_I3:  case SPARC_I4:  case SPARC_I5:  case SPARC_I6:  case SPARC_I7:
			str = string_format("%08X", m_dbgregs[16 + (entry.index() - SPARC_I0)]);
			break;
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> sparc_base_device::create_disassembler()
{
	auto dasm = std::make_unique<sparc_disassembler>(static_cast<sparc_disassembler::config const *>(this), sparc_disassembler::v7);
	if (m_asi_desc_adder)
		m_asi_desc_adder(dasm.get());
	return std::move(dasm);
}

std::unique_ptr<util::disasm_interface> sparcv8_device::create_disassembler()
{
	auto dasm = std::make_unique<sparc_disassembler>(static_cast<sparc_disassembler::config const *>(this), sparc_disassembler::v8);
	if (m_asi_desc_adder)
		m_asi_desc_adder(dasm.get());
	return std::move(dasm);
}

std::unique_ptr<util::disasm_interface> mb86930_device::create_disassembler()
{
	auto dasm = std::make_unique<sparc_disassembler>(static_cast<sparc_disassembler::config const *>(this), sparc_disassembler::sparclite);
	if (m_asi_desc_adder)
		m_asi_desc_adder(dasm.get());
	return std::move(dasm);
}

//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t sparc_base_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t sparc_base_device::execute_max_cycles() const noexcept
{
	return 4;
}


//-------------------------------------------------
//  execute_set_input - set the state of an input
//  line during execution
//-------------------------------------------------

void sparc_base_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case SPARC_IRQ1:
		case SPARC_IRQ2:
		case SPARC_IRQ3:
		case SPARC_IRQ4:
		case SPARC_IRQ5:
		case SPARC_IRQ6:
		case SPARC_IRQ7:
		case SPARC_IRQ8:
		case SPARC_IRQ9:
		case SPARC_IRQ10:
		case SPARC_IRQ11:
		case SPARC_IRQ12:
		case SPARC_IRQ13:
		case SPARC_IRQ14:
		case SPARC_NMI:
		{
			int index = (inputnum - SPARC_IRQ1) + 1;
			if (state)
			{
				m_irq_state |= 1 << index;
			}
			else
			{
				m_irq_state &= ~(1 << index);
			}

			for(index = 15; index > 0; index--)
			{
				if (m_irq_state & (1 << index))
				{
					break;
				}
			}

			m_bp_irl = index;
			break;
		}

		case SPARC_MAE:
			m_mae = (state != 0) ? 1 : 0;
			break;

		case SPARC_RESET:
			m_bp_reset_in = (state != 0) ? 1 : 0;
			break;
	}
}


//-------------------------------------------------
//  execute_add - execute an add-type opcode
//-------------------------------------------------

void sparc_base_device::execute_add(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 173, "Appendix C - ISP Descriptions - Add Instructions" (SPARCv8.pdf, pg. 170)

	uint32_t rs1 = RS1REG;
	uint32_t operand2 = USEIMM ? SIMM13 : RS2REG;

	uint32_t result = 0;
	if (ADD || ADDCC)
		result = rs1 + operand2;
	else if (ADDX || ADDXCC)
		result = rs1 + operand2 + ICC_C;

	if (RDBITS)
		RDREG = result;

	if (ADDCC || ADDXCC)
	{
		CLEAR_ICC;
		PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
		PSR |= ((BIT31(rs1) && BIT31(operand2) && !BIT31(result)) ||
				(!BIT31(rs1) && !BIT31(operand2) && BIT31(result))) ? PSR_V_MASK : 0;
		PSR |= ((BIT31(rs1) && BIT31(operand2)) ||
				(!BIT31(result) && (BIT31(rs1) || BIT31(operand2)))) ? PSR_C_MASK : 0;
	}

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_taddcc - execute a tagged add-type
//  opcode
//-------------------------------------------------

void sparc_base_device::execute_taddcc(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 173, "Appendix C - ISP Descriptions - Tagged Add Instructions" (SPARCv8.pdf, pg. 170)

	uint32_t rs1 = RS1REG;
	uint32_t operand2 = USEIMM ? SIMM13 : RS2REG;

	uint32_t result = rs1 + operand2;

	bool temp_v = (BIT31(rs1) && BIT31(operand2) && !BIT31(result)) ||
					(!BIT31(rs1) && !BIT31(operand2) && BIT31(result)) ||
					((rs1 & 3) != 0 || (operand2 & 3) != 0) ? true : false;

	if (TADDCCTV && temp_v)
	{
		m_trap = 1;
		m_tag_overflow = true;
		return;
	}

	CLEAR_ICC;
	PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
	PSR |= (result == 0) ? PSR_Z_MASK : 0;
	PSR |= temp_v ? PSR_V_MASK : 0;
	PSR |= ((BIT31(rs1) && BIT31(operand2)) ||
			(!BIT31(result) && (BIT31(rs1) || BIT31(operand2)))) ? PSR_C_MASK : 0;

	if (RDBITS)
		RDREG = result;

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_sub - execute a subtraction-type
//  opcode
//-------------------------------------------------

void sparc_base_device::execute_sub(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 174, "Appendix C - ISP Descriptions - Subtract Instructions" (SPARCv8.pdf, pg. 171)

	uint32_t rs1 = RS1REG;
	uint32_t operand2 = USEIMM ? SIMM13 : RS2REG;

	uint32_t result = 0;
	if (SUB || SUBCC)
		result = rs1 - operand2;
	else if (SUBX || SUBXCC)
		result = rs1 - operand2 - ICC_C;

	if (RDBITS)
		RDREG = result;

	if (SUBCC || SUBXCC)
	{
		CLEAR_ICC;
		PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
		PSR |= ((BIT31(rs1) && !BIT31(operand2) && !BIT31(result)) ||
				(!BIT31(rs1) && BIT31(operand2) && BIT31(result))) ? PSR_V_MASK : 0;
		PSR |= ((!BIT31(rs1) && BIT31(operand2)) ||
				(BIT31(result) && (!BIT31(rs1) || BIT31(operand2)))) ? PSR_C_MASK : 0;
	}

	PC = nPC;
	nPC = nPC + 4;
}


//--------------------------------------------------
//  execute_tsubcc - execute a tagged subtract-type
//  opcode
//--------------------------------------------------

void sparc_base_device::execute_tsubcc(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 174, "Appendix C - ISP Descriptions - Tagged Subtract Instructions" (SPARCv8.pdf, pg. 171)

	uint32_t rs1 = RS1REG;
	uint32_t operand2 = USEIMM ? SIMM13 : RS2REG;

	uint32_t result = rs1 - operand2;

	bool temp_v = (BIT31(rs1) && !BIT31(operand2) && !BIT31(result)) ||
					(!BIT31(rs1) && BIT31(operand2) && BIT31(result)) ||
					((rs1 & 3) != 0 || (operand2 & 3) != 0) ? true : false;

	if (TSUBCCTV && temp_v)
	{
		m_trap = 1;
		m_tag_overflow = 1;
		return;
	}

	CLEAR_ICC;
	PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
	PSR |= (result == 0) ? PSR_Z_MASK : 0;
	PSR |= temp_v ? PSR_V_MASK : 0;
	PSR |= ((!BIT31(rs1) && BIT31(operand2)) ||
			(BIT31(result) && (!BIT31(rs1) || BIT31(operand2)))) ? PSR_C_MASK : 0;

	if (RDBITS)
		RDREG = result;

	PC = nPC;
	nPC = nPC + 4;
}


// The SPARC Instruction Manual: Version 8, page 172, "Appendix C - ISP Descriptions - Logical Instructions" (SPARCv8.pdf, pg. 169)

template <sparc_base_device::set_cc SETCC>
void sparc_base_device::execute_and(const uint32_t op)
{
	const uint32_t result = RS1REG & (USEIMM ? SIMM13 : RS2REG);
	if (RDBITS) RDREG = result;
	if (SETCC)
	{
		CLEAR_ICC;
		if (result & 0x80000000)
			PSR |= PSR_N_MASK;
		else if (!result)
			PSR |= PSR_Z_MASK;
	}

	PC = nPC;
	nPC = nPC + 4;
}

template <sparc_base_device::set_cc SETCC>
void sparc_base_device::execute_or(const uint32_t op)
{
	const uint32_t result = RS1REG | (USEIMM ? SIMM13 : RS2REG);
	if (RDBITS) RDREG = result;
	if (SETCC)
	{
		CLEAR_ICC;
		if (result & 0x80000000)
			PSR |= PSR_N_MASK;
		else if (!result)
			PSR |= PSR_Z_MASK;
	}

	PC = nPC;
	nPC = nPC + 4;
}

template <sparc_base_device::set_cc SETCC>
void sparc_base_device::execute_xor(const uint32_t op)
{
	const uint32_t result = RS1REG ^ (USEIMM ? SIMM13 : RS2REG);
	if (RDBITS) RDREG = result;
	if (SETCC)
	{
		CLEAR_ICC;
		if (result & 0x80000000)
			PSR |= PSR_N_MASK;
		else if (!result)
			PSR |= PSR_Z_MASK;
	}

	PC = nPC;
	nPC = nPC + 4;
}

template <sparc_base_device::set_cc SETCC>
void sparc_base_device::execute_andn(const uint32_t op)
{
	const uint32_t result = RS1REG & ~(USEIMM ? SIMM13 : RS2REG);
	if (RDBITS) RDREG = result;
	if (SETCC)
	{
		CLEAR_ICC;
		if (result & 0x80000000)
			PSR |= PSR_N_MASK;
		else if (!result)
			PSR |= PSR_Z_MASK;
	}

	PC = nPC;
	nPC = nPC + 4;
}

template <sparc_base_device::set_cc SETCC>
void sparc_base_device::execute_orn(const uint32_t op)
{
	const uint32_t result = RS1REG | ~(USEIMM ? SIMM13 : RS2REG);
	if (RDBITS) RDREG = result;
	if (SETCC)
	{
		CLEAR_ICC;
		if (result & 0x80000000)
			PSR |= PSR_N_MASK;
		else if (!result)
			PSR |= PSR_Z_MASK;
	}

	PC = nPC;
	nPC = nPC + 4;
}

template <sparc_base_device::set_cc SETCC>
void sparc_base_device::execute_xnor(const uint32_t op)
{
	const uint32_t result = RS1REG ^ ~(USEIMM ? SIMM13 : RS2REG);
	if (RDBITS) RDREG = result;
	if (SETCC)
	{
		CLEAR_ICC;
		if (result & 0x80000000)
			PSR |= PSR_N_MASK;
		else if (!result)
			PSR |= PSR_Z_MASK;
	}

	PC = nPC;
	nPC = nPC + 4;
}

//-------------------------------------------------
//  execute_shift - execute a shift-type opcode,
//  sll/srl/sra
//-------------------------------------------------

void sparc_base_device::execute_shift(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 172, "Appendix C - ISP Descriptions - Shift Instructions" (SPARCv8.pdf, pg. 169)

	uint32_t shift_count = USEIMM ? (SIMM13 & 31) : (RS2REG & 31);

	if (RDBITS)
	{
		if (SLL)
			RDREG = RS1REG << shift_count;
		else if (SRL)
			RDREG = uint32_t(RS1REG) >> shift_count;
		else if (SRA)
			RDREG = int32_t(RS1REG) >> shift_count;
	}

	PC = nPC;
	nPC = nPC + 4;
}


//--------------------------------------------------
//  execute_mulscc - execute a multiply step opcode
//--------------------------------------------------

void sparc_base_device::execute_mulscc(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 175, "Appendix C - ISP Descriptions - Multiply Step Instruction" (SPARCv8.pdf, pg. 172)

	uint32_t operand1 = ((ICC_N != ICC_V) ? 0x80000000 : 0) | (RS1REG >> 1);

	uint32_t operand2 = (Y & 1) ? (USEIMM ? SIMM13 : RS2REG) : 0;

	uint32_t result = operand1 + operand2;
	Y = ((RS1REG & 1) ? 0x80000000 : 0) | (Y >> 1);

	if (RDBITS)
		RDREG = result;

	CLEAR_ICC;
	PSR |= (BIT31(result)) ? PSR_N_MASK : 0;
	PSR |= (result == 0) ? PSR_Z_MASK : 0;
	PSR |= ((BIT31(operand1) && BIT31(operand2) && !BIT31(result)) ||
			(!BIT31(operand1) && !BIT31(operand2) && BIT31(result))) ? PSR_V_MASK : 0;
	PSR |= ((BIT31(operand1) && BIT31(operand2)) ||
			(!BIT31(result) && (BIT31(operand1) || BIT31(operand2)))) ? PSR_C_MASK : 0;

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_rdsr - execute a status register read
//  opcode
//-------------------------------------------------

void sparc_base_device::execute_rdsr(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 182, "Appendix C - ISP Descriptions - Read State Register Instructions" (SPARCv8.pdf, pg. 179)

	if (RDASR && RS1 == 15 && RD == 0)
	{
		// Store Barrier instruction - not implemented, as stores are always consistent in the context of MAME
		PC = nPC;
		nPC = nPC + 4;
		return;
	}

	if (((RDPSR || RDWIM || RDTBR) || (RDASR && m_privileged_asr[RS1])) && IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if (m_illegal_instruction_asr[RS1])
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	if (RDBITS)
	{
		if (RDASR)
		{
			if (RS1 == 0)
			{
				RDREG = Y;
			}
		}
		else if (RDPSR)
		{
			RDREG = PSR;
		}
		else if (RDWIM)
			RDREG = WIM;
		else if (RDTBR)
			RDREG = TBR;
	}

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_wrsr - execute a status register write
//  opcode
//-------------------------------------------------

void sparc_base_device::execute_wrsr(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 183, "Appendix C - ISP Descriptions - Write State Register Instructions" (SPARCv8.pdf, pg. 180)

	uint32_t operand2 = USEIMM ? SIMM13 : RS2REG;

	uint32_t result = RS1REG ^ operand2;

	if (WRASR && RD == 0)
	{
		Y = result;
		PC = nPC;
		nPC = nPC + 4;
	}
	else if (WRASR)
	{
		if (m_privileged_asr[RD] && IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
			return;
		}
		else if (m_illegal_instruction_asr[RD])
		{
			m_trap = 1;
			m_illegal_instruction = 1;
			return;
		}
		else
		{
			// SPARCv8
			PC = nPC;
			nPC = nPC + 4;
		}
	}
	else if (WRPSR)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
			return;
		}
		else if ((result & 31) >= NWINDOWS)
		{
			m_trap = 1;
			m_illegal_instruction = 1;
			return;
		}

		PSR = (PSR & PSR_ZERO_MASK) | (result & ~PSR_ZERO_MASK);
		update_gpr_pointers();

		m_et = PSR & PSR_ET_MASK;
		m_pil = (PSR & PSR_PIL_MASK) >> PSR_PIL_SHIFT;
		m_s = PSR & PSR_S_MASK;
		if (m_s)
		{
			m_data_space = 11;
		}
		else
		{
			m_data_space = 10;
		}

		if (m_et && (m_bp_irl == 15 || m_bp_irl > m_pil) && m_interrupt_level == 0)
		{
			m_trap = 1;
			m_interrupt_level = m_bp_irl;
		}

		PC = nPC;
		nPC = nPC + 4;
	}
	else if (WRWIM)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
			return;
		}

		WIM = result & 0x7f;
		PC = nPC;
		nPC = nPC + 4;
	}
	else if (WRTBR)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
			return;
		}

		TBR = result & 0xfffff000;
		PC = nPC;
		nPC = nPC + 4;
	}
}


//-------------------------------------------------
//  execute_rett - execute a return-from-trap
//  opcode
//-------------------------------------------------

void sparc_base_device::execute_rett(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 181, "Appendix C - ISP Descriptions - Return from Trap Instructions" (SPARCv8.pdf, pg. 178)

	uint8_t new_cwp = ((PSR & PSR_CWP_MASK) + 1) % NWINDOWS;
	uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
	if (m_et)
	{
		m_trap = 1;
		if (IS_USER)
		{
			m_privileged_instruction = 1;
		}
		else
		{
			m_illegal_instruction = 1;
		}
		return;
	}
	else if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		m_tt = 3;
		m_execute_mode = 0;
		m_error_mode = 1;
		return;
	}
	else if ((WIM & (1 << new_cwp)) != 0)
	{
		m_trap = 1;
		m_window_underflow = 1;
		m_tt = 6;
		m_execute_mode = 0;
		m_error_mode = 1;
		return;
	}
	else if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		m_tt = 7;
		m_execute_mode = 0;
		m_error_mode = 1;
		return;
	}

	PSR |= PSR_ET_MASK;
	m_et = true;
	PC = nPC;
	nPC = address;

	PSR &= ~PSR_CWP_MASK;
	PSR |= new_cwp;

	if (PSR & PSR_PS_MASK)
	{
		PSR |= PSR_S_MASK;
		m_s = true;
		m_data_space = 11;
	}
	else
	{
		PSR &= ~PSR_S_MASK;
		m_s = false;
		m_data_space = 10;
	}

	update_gpr_pointers();

	if (m_et && (m_bp_irl == 15 || m_bp_irl > m_pil) && m_interrupt_level == 0)
	{
		m_trap = 1;
		m_interrupt_level = m_bp_irl;
	}
}



//-------------------------------------------------
// execute_saverestore - execute a save or restore
// opcode
//-------------------------------------------------

void sparc_base_device::execute_saverestore(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 177, "Appendix C - ISP Descriptions - SAVE and RESTORE Instructions" (SPARCv8.pdf, pg. 174)

	uint32_t rs1 = RS1REG;
	uint32_t operand2 = USEIMM ? SIMM13 : RS2REG;

	uint32_t result = 0;
	if (SAVE)
	{
		uint8_t new_cwp = (((PSR & PSR_CWP_MASK) + NWINDOWS) - 1) % NWINDOWS;
		if ((WIM & (1 << new_cwp)) != 0)
		{
			m_trap = 1;
			m_window_overflow = 1;
			return;
		}

		result = rs1 + operand2;
		PSR &= ~PSR_CWP_MASK;
		PSR |= new_cwp;
	}
	else if (RESTORE)
	{
		uint8_t new_cwp = ((PSR & PSR_CWP_MASK) + 1) % NWINDOWS;
		if ((WIM & (1 << new_cwp)) != 0)
		{
			m_trap = 1;
			m_window_underflow = 1;
			return;
		}

		result = rs1 + operand2;
		PSR &= ~PSR_CWP_MASK;
		PSR |= new_cwp;
	}

	update_gpr_pointers();

	if (RDBITS)
		RDREG = result;

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_jmpl - execute a jump and link opcode
//-------------------------------------------------

void sparc_base_device::execute_jmpl(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 180, "Appendix C - ISP Descriptions - SAVE and RESTORE Instructions" (SPARCv8.pdf, pg. 177)

	uint32_t jump_address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (jump_address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
	}
	else
	{
		if (RDBITS)
			RDREG = PC;
		PC = nPC;
		nPC = jump_address;
	}
}


//-------------------------------------------------
//  execute_group2 - execute an opcode in group 2,
//  mostly ALU ops
//-------------------------------------------------

inline void sparc_base_device::execute_group2(uint32_t op)
{
	switch (OP3)
	{
		case OP3_ADD:
		case OP3_ADDX:
		case OP3_ADDCC:
		case OP3_ADDXCC:
			execute_add(op);
			break;

		case OP3_SUB:
		case OP3_SUBX:
		case OP3_SUBCC:
		case OP3_SUBXCC:
			execute_sub(op);
			break;

		case OP3_TADDCC:
		case OP3_TADDCCTV:
			execute_taddcc(op);
			break;

		case OP3_TSUBCC:
		case OP3_TSUBCCTV:
			execute_tsubcc(op);
			break;

		case OP3_AND:
			execute_and<NOCC>(op);
			break;
		case OP3_OR:
			execute_or<NOCC>(op);
			break;
		case OP3_XOR:
			execute_xor<NOCC>(op);
			break;
		case OP3_ANDN:
			execute_andn<NOCC>(op);
			break;
		case OP3_ORN:
			execute_orn<NOCC>(op);
			break;
		case OP3_XNOR:
			execute_xnor<NOCC>(op);
			break;
		case OP3_ANDCC:
			execute_and<USECC>(op);
			break;
		case OP3_ORCC:
			execute_or<USECC>(op);
			break;
		case OP3_XORCC:
			execute_xor<USECC>(op);
			break;
		case OP3_ANDNCC:
			execute_andn<USECC>(op);
			break;
		case OP3_ORNCC:
			execute_orn<USECC>(op);
			break;
		case OP3_XNORCC:
			execute_xnor<USECC>(op);
			break;

		case OP3_MULSCC:
			execute_mulscc(op);
			break;

		case OP3_SLL:
		case OP3_SRL:
		case OP3_SRA:
			execute_shift(op);
			break;

		case OP3_RDASR:
		case OP3_RDPSR:
		case OP3_RDWIM:
		case OP3_RDTBR:
			execute_rdsr(op);
			break;

		case OP3_WRASR:
		case OP3_WRPSR:
		case OP3_WRWIM:
		case OP3_WRTBR:
			execute_wrsr(op);
			break;

		case OP3_FPOP1:
		case OP3_FPOP2:
			if (!(PSR & PSR_EF_MASK) || !m_bp_fpu_present)
			{
				m_trap = 1;
				m_fp_disabled = 1;
			}
			complete_fp_execution(op);
			return;

		case OP3_JMPL:
			execute_jmpl(op);
			break;

		case OP3_RETT:
			execute_rett(op);
			break;

		case OP3_TICC:
			execute_ticc(op);
			break;

		case OP3_IFLUSH:
			// Ignored
			PC = nPC;
			nPC = nPC + 4;
			break;

		case OP3_SAVE:
		case OP3_RESTORE:
			execute_saverestore(op);
			break;

		default:
			if (!execute_extra_group2(op))
			{
				logerror("illegal instruction at %08x: %08x\n", PC, op);
				m_trap = 1;
				m_illegal_instruction = 1;
			}
			break;
	}
}


//-------------------------------------------------
//  update_extra_group2 - execute a group2
//  instruction belonging to a newer SPARC version
//  than v7, or which can be overridden from v7
//-------------------------------------------------

bool sparcv7_device::execute_extra_group2(uint32_t op)
{
	switch (OP3)
	{
		default:
			return false;
	}
}

bool sparcv8_device::execute_extra_group2(uint32_t op)
{
	switch (OP3)
	{
		case OP3_UMUL:
		case OP3_SMUL:
		case OP3_UMULCC:
		case OP3_SMULCC:
			execute_mul(op);
			return true;

		case OP3_UDIV:
		case OP3_SDIV:
		case OP3_UDIVCC:
		case OP3_SDIVCC:
			execute_div(op);
			return true;

		case OP3_CPOP1:
		case OP3_CPOP2:
			logerror("cpop @ %08x: %08x\n", PC, op);
			m_trap = 1;
			m_cp_disabled = 1;
			return true;

		default:
			return false;
	}
}

void mb86930_device::execute_divscc(uint32_t op)
{
	fatalerror("Not yet supported: divscc at %08x\n", PC);
	/*const bool n_flag = (PSR & PSR_N_MASK);
	const bool v_flag = (PSR & PSR_V_MASK);
	const bool true_sign = n_flag ^ v_flag;
	const uint64_t dividend = ((uint64_t)m_y << 32) | RS1REG;
	const uint32_t remainder = m_y << 1;
	const uint32_t divisor = (USEIMM ? SIMM13 : RS2REG);*/
}

void mb86930_device::execute_scan(uint32_t op)
{
	fatalerror("Not yet supported: scan at %08x\n", PC);
}

bool mb86930_device::execute_extra_group2(uint32_t op)
{
	switch (OP3)
	{
		case OP3_DIVSCC:
			execute_divscc(op);
			return true;

		case OP3_SCAN:
			execute_scan(op);
			return true;

		case OP3_UMUL:
		case OP3_SMUL:
		case OP3_UMULCC:
		case OP3_SMULCC:
			return sparcv8_device::execute_extra_group2(op);

		default:
			return false;
	}
}

//-------------------------------------------------
//  update_gpr_pointers - cache pointers to
//  the registers in our current window
//-------------------------------------------------

void sparc_base_device::update_gpr_pointers()
{
	int cwp = PSR & PSR_CWP_MASK;
	for (int i = 0; i < 8; i++)
	{
		m_regs[ 8 + i] = &m_r[8 + (( 0 + cwp * 16 + i) % (NWINDOWS * 16))];
		m_regs[16 + i] = &m_r[8 + (( 8 + cwp * 16 + i) % (NWINDOWS * 16))];
		m_regs[24 + i] = &m_r[8 + ((16 + cwp * 16 + i) % (NWINDOWS * 16))];
	}
}


//-------------------------------------------------
//  execute_store - execute a store-type opcode
//-------------------------------------------------

void sparc_base_device::execute_store(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 165, "Appendix C - ISP Descriptions - Store Instructions" (SPARCv8.pdf, pg. 162)

	if (IS_USER && (STDA || STA || STHA || STBA || STDFQ || STDCQ))
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if ((USEIMM && (STDA || STA || STHA || STBA)) || ((STD || STDA) && (RD & 1)))
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	uint32_t address = 0;
	uint8_t addr_space = 0;
	if (STD || ST || STH || STB || STF || STDF || STFSR || STDFQ || STCSR || STC || STDC || STDCQ)
	{
		address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
		addr_space = m_data_space;
	}
	else if (STDA || STA || STHA || STBA)
	{
		address = RS1REG + RS2REG;
		addr_space = ASI;
	}
	if ((STF || STDF || STFSR || STDFQ) && (!(PSR & PSR_EF_MASK) || !m_bp_fpu_present))
	{
		m_trap = 1;
		m_fp_disabled = 1;
		return;
	}
	if ((STC || STDC || STCSR || STDCQ) && (!(PSR & PSR_EC_MASK) || !m_bp_cp_present))
	{
		m_trap = 1;
		m_cp_disabled = 1;
		return;
	}

	if ((STH || STHA) && ((address & 1) != 0))
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}
	else if ((ST || STA || STF || STFSR || STC || STCSR) && ((address & 3) != 0))
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}
	else if ((STD || STDA || STDF || STDFQ || STDC || STDCQ) && ((address & 7) != 0))
	{
		m_mem_address_not_aligned = 1;
		m_trap = 1;
		return;
	}

	if (STDFQ)
	{
		// assume no floating-point queue for now
		return;
	}
	if (STDCQ)
	{
		// assume no coprocessor queue for now
		m_trap = 1;
		m_cp_exception = 1;
		// { possibly additional implementation-dependent actions }
		return;
	}

	uint32_t data0 = 0;
	uint32_t data1 = 0;
	//uint8_t byte_mask;
	if (STF)
	{
		//byte_mask = 15;
		if (get_fpr32(data0, RD))
		{
			return;
		}
	}
	else if (STC)
	{
		//byte_mask = 15;
		data0 = 0;
	}
	else if (STDF)
	{
		//byte_mask = 15;
		if (get_fpr32(data0, RD & 0x1e))
		{
			return;
		}
		if (get_fpr32(data1, RD | 1))
		{
			return;
		}
	}
	else if (STDC)
	{
		//byte_mask = 15;
		data0 = 0;
	}
	else if (STD || STDA)
	{
		//byte_mask = 15;
		data0 = REG(RD & 0x1e);
		data1 = REG(RD | 1);
	}
	else if (STDFQ)
	{
		//byte_mask = 15;
		data0 = 0;
		data1 = 0;
	}
	else if (STDCQ)
	{
		//byte_mask = 15;
		data0 = 0;
		data1 = 0;
	}
	else if (STFSR)
	{
		// while ((FSR.qne = 1) and (trap = 0)) (
		//     wait for pending floating-point instructions to complete
		// )
		// next;
		//byte_mask = 15;
		data0 = FSR;
	}
	else if (STCSR)
	{
		// { implementation-dependent actions }
		//byte_mask = 15;
		data0 = 0;
	}
	else if (ST || STA)
	{
		//byte_mask = 15;
		data0 = REG(RD);
	}
	else if (STH || STHA)
	{
		if ((address & 3) == 0)
		{
			//byte_mask = 12;
			data0 = REG(RD) << 16;
		}
		else if ((address & 3) == 2)
		{
			//byte_mask = 3;
			data0 = REG(RD);
		}
	}
	else if (STB || STBA)
	{
		if ((address & 3) == 0)
		{
			//byte_mask = 8;
			data0 = REG(RD) << 24;
		}
		else if ((address & 3) == 1)
		{
			//byte_mask = 4;
			data0 = REG(RD) << 16;
		}
		else if ((address & 3) == 2)
		{
			//byte_mask = 2;
			data0 = REG(RD) << 8;
		}
		else if ((address & 3) == 3)
		{
			//byte_mask = 1;
			data0 = REG(RD);
		}
	}

	static const uint32_t mask16[4] = { 0xffff0000, 0x00000000, 0x0000ffff, 0x00000000 };
	static const uint32_t mask8[4] = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff };
	write_word(addr_space, address, data0, (ST || STA || STD || STDA || STF || STDF || STDFQ || STFSR || STC || STDC || STDCQ || STCSR) ? 0xffffffff : ((STH || STHA) ? mask16[address & 2] : mask8[address & 3]));
	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (STD || STDA || STDF || STDC || STDFQ || STDCQ)
	{
		write_word(addr_space, address + 4, data1);
		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
			return;
		}
	}

	PC = nPC;
	nPC = nPC + 4;
}

// The SPARC Instruction Manual: Version 8, page 163, "Appendix C - ISP Descriptions - C.9. Instruction Defintions - Load Instructions" (SPARCv8.pdf, pg. 160)

inline void sparc_base_device::execute_ldd(uint32_t op)
{
	if (RD & 1)
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (address & 7)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	const uint32_t data = read_word(m_data_space, address);

	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
		RDREG = data;

	const uint32_t word1 = read_word(m_data_space, address + 4);
	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	REG(RD | 1) = word1;

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ld(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	const uint32_t data = read_word(m_data_space, address);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
		RDREG = data;

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldsh(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (address & 1)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	static const uint32_t mask16[4] = { 0xffff0000, 0x00000000, 0x0000ffff, 0x00000000 };
	const uint32_t data = read_word(m_data_space, address, mask16[address & 2]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		if ((address & 3) == 0) RDREG = (int32_t)data >> 16;
		else if ((address & 3) == 2) RDREG = ((int32_t)data << 16) >> 16;
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_lduh(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (address & 1)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	static const uint32_t mask16[4] = { 0xffff0000, 0x00000000, 0x0000ffff, 0x00000000 };
	const uint32_t data = read_word(m_data_space, address, mask16[address & 2]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		if ((address & 3) == 0) RDREG = data >> 16;
		else if ((address & 3) == 2) RDREG = data & 0xffff;
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldsb(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	static const uint32_t mask8[4] = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff };
	const uint32_t data = read_word(m_data_space, address, mask8[address & 3]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		if ((address & 3) == 0) RDREG = (int32_t)data >> 24;
		else if ((address & 3) == 1) RDREG = ((int32_t)data << 8) >> 24;
		else if ((address & 3) == 2) RDREG = ((int32_t)data << 16) >> 24;
		else if ((address & 3) == 3) RDREG = ((int32_t)data << 24) >> 24;
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldub(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	static const uint32_t mask8[4] = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff };
	const uint32_t byte_idx = address & 3;
	const uint32_t data = read_word(m_data_space, address, mask8[byte_idx]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		static const int shifts[4] = { 24, 16, 8, 0 };
		RDREG = (uint8_t)(data >> shifts[byte_idx]);
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_lddfpr(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (!(PSR & PSR_EF_MASK) || m_bp_fpu_present == 0)
	{
		m_trap = 1;
		m_fp_disabled = 1;
		return;
	}

	if (address & 7)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	const uint32_t data = read_word(m_data_space, address);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (m_pending_fpr == (RD & 0x1e))
	{
		return;
	}

	const uint32_t word1 = read_word(m_data_space, address + 4);
	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (m_pending_fpr == (RD | 1))
	{
		return;
	}

	FREG(RD & 0x1e) = data;
	FREG(RD | 1) = word1;

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldfpr(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (!(PSR & PSR_EF_MASK) || m_bp_fpu_present == 0)
	{
		m_trap = 1;
		m_fp_disabled = 1;
		return;
	}

	if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	const uint32_t data = read_word(m_data_space, address);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (m_pending_fpr == RD)
	{
		return;
	}

	FDREG = data;

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldfsr(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (!(PSR & PSR_EF_MASK) || m_bp_fpu_present == 0)
	{
		m_trap = 1;
		m_fp_disabled = 1;
		return;
	}

	if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	const uint32_t data = read_word(m_data_space, address);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	FSR = (data & ~FSR_RESV_MASK) | FSR_VER;

	switch (FSR & FSR_RD_MASK)
	{
	case FSR_RD_NEAR: softfloat_roundingMode = softfloat_round_near_even; break;
	case FSR_RD_ZERO: softfloat_roundingMode = softfloat_round_minMag; break;
	case FSR_RD_UP:   softfloat_roundingMode = softfloat_round_max; break;
	case FSR_RD_DOWN: softfloat_roundingMode = softfloat_round_min; break;
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_lddcpr(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (!(PSR & PSR_EC_MASK) || !m_bp_cp_present)
	{
		m_trap = 1;
		m_cp_disabled = 1;
		return;
	}

	if (address & 7)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	if (m_cp_sequence_err)
	{
		m_trap = 1;
		m_cp_exception = 1;
		// possibly additional implementation-dependent actions
		return;
	}

	read_word(m_data_space, address);
	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	// implementation-dependent actions

	read_word(m_data_space, address + 4);
	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	// implementation-dependent actions

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldcpr(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (!(PSR & PSR_EC_MASK) || !m_bp_cp_present)
	{
		m_trap = 1;
		m_cp_disabled = 1;
		return;
	}

	if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	if (m_cp_sequence_err)
	{
		m_trap = 1;
		m_cp_exception = 1;
		// possibly additional implementation-dependent actions
		return;
	}

	read_word(m_data_space, address);

	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	// implementation-dependent actions

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldcsr(uint32_t op)
{
	const uint32_t address = RS1REG + (USEIMM ? SIMM13 : RS2REG);

	if (!(PSR & PSR_EC_MASK) || !m_bp_cp_present)
	{
		m_trap = 1;
		m_cp_disabled = 1;
		return;
	}

	if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	if (m_cp_sequence_err)
	{
		m_trap = 1;
		m_cp_exception = 1;
		// possibly additional implementation-dependent actions
		return;
	}

	read_word(m_data_space, address);

	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	// implementation-dependent actions

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldda(uint32_t op)
{
	if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if (USEIMM || (RD & 1))
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	const uint32_t address = RS1REG + RS2REG;
	const uint32_t addr_space = ASI;

	if (address & 7)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	const uint32_t data = read_word(addr_space, address);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
		RDREG = data;

	uint32_t word1 = read_word(addr_space, address + 4);
	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	REG(RD | 1) = word1;

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_lda(uint32_t op)
{
	if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if (USEIMM)
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	const uint32_t address = RS1REG + RS2REG;

	if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	const uint32_t data = read_word(ASI, address);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
		RDREG = data;

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldsha(uint32_t op)
{
	if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if (USEIMM)
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	const uint32_t address = RS1REG + RS2REG;

	if (address & 1)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	static const uint32_t mask16[4] = { 0xffff0000, 0x00000000, 0x0000ffff, 0x00000000 };
	const uint32_t data = read_word(ASI, address, mask16[address & 2]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		if ((address & 3) == 0) RDREG = (int32_t)data >> 16;
		else if ((address & 3) == 2) RDREG = ((int32_t)data << 16) >> 16;
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_lduha(uint32_t op)
{
	if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if (USEIMM)
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	const uint32_t address = RS1REG + RS2REG;
	if (address & 1)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	static const uint32_t mask16[4] = { 0xffff0000, 0x00000000, 0x0000ffff, 0x00000000 };
	const uint32_t data = read_word(ASI, address, mask16[address & 2]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		if ((address & 3) == 0) RDREG = data >> 16;
		else if ((address & 3) == 2) RDREG = data & 0xffff;
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_ldsba(uint32_t op)
{
	if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if (USEIMM)
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	static const uint32_t mask8[4] = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff };
	const uint32_t address = RS1REG + RS2REG;
	const uint32_t data = read_word(ASI, address, mask8[address & 3]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		if ((address & 3) == 0) RDREG = (int32_t)data >> 24;
		else if ((address & 3) == 1) RDREG = ((int32_t)data << 8) >> 24;
		else if ((address & 3) == 2) RDREG = ((int32_t)data << 16) >> 24;
		else if ((address & 3) == 3) RDREG = ((int32_t)data << 24) >> 24;
	}

	PC = nPC;
	nPC = nPC + 4;
}

inline void sparc_base_device::execute_lduba(uint32_t op)
{
	if (IS_USER)
	{
		m_trap = 1;
		m_privileged_instruction = 1;
		return;
	}
	else if (USEIMM)
	{
		m_trap = 1;
		m_illegal_instruction = 1;
		return;
	}

	static const uint32_t mask8[4] = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff };
	const uint32_t address = RS1REG + RS2REG;
	const uint32_t data = read_word(ASI, address, mask8[address & 3]);

	if (m_mae)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
	{
		if ((address & 3) == 0) RDREG = data >> 24;
		else if ((address & 3) == 1) RDREG = (data >> 16) & 0xff;
		else if ((address & 3) == 2) RDREG = (data >>  8) & 0xff;
		else if ((address & 3) == 3) RDREG = data & 0xff;
	}

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_ldstub - execute an atomic load-store
//  instruction
//-------------------------------------------------

void sparc_base_device::execute_ldstub(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 169, "Appendix C - ISP Descriptions - Atomic Load-Store Unsigned Byte Instructions" (SPARCv8.pdf, pg. 166)

	uint32_t address = 0;
	uint8_t addr_space = 0;
	if (LDSTUB)
	{
		address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
		addr_space = (IS_USER ? 10 : 11);
	}
	else if (LDSTUBA)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
			return;
		}
		else if (USEIMM)
		{
			m_trap = 1;
			m_illegal_instruction = 1;
			return;
		}
		else
		{
			address = RS1REG + RS2REG;
			addr_space = ASI;
		}
	}

	//while (m_pb_block_ldst_byte || m_pb_block_ldst_word)
	//{
		// { wait for lock(s) to be lifted }
		// { an implementation actually need only block when another LDSTUB or SWAP
		//   is pending on the same byte in memory as the one addressed by this LDSTUB }
	//}

	m_pb_block_ldst_byte = 1;

	static const uint32_t mask8[4] = { 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff };
	uint32_t data = read_word(addr_space, address, mask8[address & 3]);

	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	write_word(addr_space, address, 0xffffffff, mask8[address & 3]);

	m_pb_block_ldst_byte = 0;

	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	uint32_t word;
	if ((address & 3) == 0)
	{
		word = (data >> 24) & 0xff;
	}
	else if ((address & 3) == 1)
	{
		word = (data >> 16) & 0xff;
	}
	else if ((address & 3) == 2)
	{
		word = (data >> 8) & 0xff;
	}
	else // if ((address & 3) == 3)
	{
		word = data & 0xff;
	}
	if (RDBITS)
		RDREG = word;

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_group3 - execute an opcode in group 3
//  (load/store)
//-------------------------------------------------

inline void sparc_base_device::execute_group3(uint32_t op)
{
	static const int ldst_cycles[64] = {
		1, 1, 1, 2, 2, 2, 2, 3,
		0, 1, 1, 0, 0, 3, 0, 0,
		1, 1, 1, 2, 2, 2, 2, 3,
		0, 1, 1, 0, 0, 3, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	switch (OP3)
	{
		case OP3_LD:
			execute_ld(op);
			break;
		case OP3_LDUB:
			execute_ldub(op);
			break;
		case OP3_LDUH:
			execute_lduh(op);
			break;
		case OP3_LDD:
			execute_ldd(op);
			break;
		case OP3_LDSB:
			execute_ldsb(op);
			break;
		case OP3_LDSH:
			execute_ldsh(op);
			break;
		case OP3_LDA:
			execute_lda(op);
			break;
		case OP3_LDUBA:
			execute_lduba(op);
			break;
		case OP3_LDUHA:
			execute_lduha(op);
			break;
		case OP3_LDDA:
			execute_ldda(op);
			break;
		case OP3_LDSBA:
			execute_ldsba(op);
			break;
		case OP3_LDSHA:
			execute_ldsha(op);
			break;
		case OP3_LDFPR:
			execute_ldfpr(op);
			break;
		case OP3_LDFSR:
			execute_ldfsr(op);
			break;
		case OP3_LDDFPR:
			execute_lddfpr(op);
			break;
		case OP3_LDCPR:
			execute_ldcpr(op);
			break;
		case OP3_LDCSR:
			execute_ldcsr(op);
			break;
		case OP3_LDDCPR:
			execute_lddcpr(op);
			break;

		case OP3_ST:
		case OP3_STB:
		case OP3_STH:
		case OP3_STD:
		case OP3_STA:
		case OP3_STBA:
		case OP3_STHA:
		case OP3_STDA:
		case OP3_STFPR:
		case OP3_STFSR:
		case OP3_STDFQ:
		case OP3_STDFPR:
		case OP3_STCPR:
		case OP3_STCSR:
		case OP3_STDCQ:
		case OP3_STDCPR:
			execute_store(op);
			break;

		case OP3_LDSTUB:
		case OP3_LDSTUBA:
			execute_ldstub(op);
			break;

		default:
			if (!execute_extra_group3(op))
			{
				logerror("illegal instruction at %08x: %08x\n", PC, op);
				m_trap = 1;
				m_illegal_instruction = 1;
			}
			break;
	}

	if (MAE)
		m_icount--;
	else
		m_icount -= ldst_cycles[OP3];
}


//-------------------------------------------------
//  update_extra_group3 - execute a group3
//  instruction belonging to a newer SPARC version
//  than v7
//-------------------------------------------------

bool sparcv7_device::execute_extra_group3(uint32_t op)
{
	return false;
}

bool sparcv8_device::execute_extra_group3(uint32_t op)
{
	switch (OP3)
	{
		case OP3_SWAP:
		case OP3_SWAPA:
			execute_swap(op);
			return true;

		default:
			return false;
	}
}


//-------------------------------------------------
//  evaluate_fp_condition - evaluate a given fp
//  condition code
//-------------------------------------------------

bool sparc_base_device::evaluate_fp_condition(uint32_t op)
{
	// COND     & 8
	// 0        8
	// fbn      fba
	// fbne     fbe
	// fblg     fbue
	// fbul     fbge
	// fbl      fbuge
	// fbug     fble
	// fbg      fbule
	// fbu      fbo

	static const uint32_t EQ_BIT = (1 << (FSR_FCC_EQ >> FSR_FCC_SHIFT));
	static const uint32_t LT_BIT = (1 << (FSR_FCC_LT >> FSR_FCC_SHIFT));
	static const uint32_t GT_BIT = (1 << (FSR_FCC_GT >> FSR_FCC_SHIFT));
	static const uint32_t UO_BIT = (1 << (FSR_FCC_UO >> FSR_FCC_SHIFT));
	const uint32_t fcc_bit = 1 << ((m_fsr & FSR_FCC_MASK) >> FSR_FCC_SHIFT);

	switch(COND)
	{
		case 0:     return false;
		case 1:     return fcc_bit & (LT_BIT | GT_BIT | UO_BIT);
		case 2:     return fcc_bit & (LT_BIT | GT_BIT);
		case 3:     return fcc_bit & (LT_BIT | UO_BIT);
		case 4:     return fcc_bit & (LT_BIT);
		case 5:     return fcc_bit & (GT_BIT | UO_BIT);
		case 6:     return fcc_bit & (GT_BIT);
		case 7:     return fcc_bit & (UO_BIT);

		case 8:     return true;
		case 9:     return fcc_bit & (EQ_BIT);
		case 10:    return fcc_bit & (EQ_BIT | UO_BIT);
		case 11:    return fcc_bit & (EQ_BIT | GT_BIT);
		case 12:    return fcc_bit & (EQ_BIT | GT_BIT | UO_BIT);
		case 13:    return fcc_bit & (EQ_BIT | LT_BIT);
		case 14:    return fcc_bit & (EQ_BIT | LT_BIT | UO_BIT);
		case 15:    return fcc_bit & (EQ_BIT | LT_BIT | GT_BIT);
	}

	return false;
}


//-------------------------------------------------
//  execute_fbfcc - execute an fp branch opcode
//-------------------------------------------------

void sparc_base_device::execute_fbfcc(uint32_t op)
{
	bool branch_taken = evaluate_fp_condition(op);
	uint32_t pc = PC;
	PC = nPC;
	if (branch_taken)
	{
		nPC = pc + DISP22;
		if (COND == COND_BA && ANNUL)
			m_no_annul = false;
	}
	else
	{
		nPC = nPC + 4;
		if (ANNUL)
			m_no_annul = false;
	}
}


//-------------------------------------------------
//  evaluate_condition - evaluate a given integer
//  condition code
//-------------------------------------------------

bool sparc_base_device::evaluate_condition(uint32_t op)
{
	// COND     & 8
	// 0        8
	// bn       ba
	// bz       bne
	// ble      bg
	// bl       bge
	// bleu     bgu
	// bcs      bcc
	// bneg     bpos
	// bvs      bvc

	switch(COND)
	{
		case 0:     return false;
		case 1:     return ICC_Z_SET;
		case 2:     return ICC_Z_SET || (ICC_N != ICC_Z);
		case 3:     return (ICC_N != ICC_V);
		case 4:     return ICC_C_SET || ICC_Z_SET;
		case 5:     return ICC_C_SET;
		case 6:     return ICC_N_SET;
		case 7:     return ICC_V_SET;

		case 8:     return true;
		case 9:     return ICC_Z_CLEAR;
		case 10:    return ICC_Z_CLEAR && ICC_N_CLEAR;
		case 11:    return (ICC_N == ICC_V);
		case 12:    return ICC_C_CLEAR && ICC_Z_CLEAR;
		case 13:    return ICC_C_CLEAR;
		case 14:    return ICC_N_CLEAR;
		case 15:    return ICC_V_CLEAR;
	}

	return false;
}


//-------------------------------------------------
//  execute_bicc - execute a branch opcode
//-------------------------------------------------

void sparc_base_device::execute_bicc(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 178, "Appendix C - ISP Descriptions - Branch on Integer Condition Instructions" (SPARCv8.pdf, pg. 175)

	bool branch_taken = evaluate_condition(op);
	uint32_t pc = PC;
	PC = nPC;
	if (branch_taken)
	{
		nPC = pc + DISP22;
		if (COND == COND_BA && ANNUL)
			m_no_annul = false;
	}
	else
	{
		nPC = nPC + 4;
		if (ANNUL)
			m_no_annul = false;
	}
}


//-------------------------------------------------
//  execute_ticc - execute a conditional trap
//-------------------------------------------------

void sparc_base_device::execute_ticc(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 182, "Appendix C - ISP Descriptions - Trap on Integer Condition Instructions" (SPARCv8.pdf, pg. 179)

	bool trap_eval_icc = evaluate_condition(op);

	uint8_t trap_number = RS1REG + (USEIMM ? SIMM7 : RS2REG);

	if (COND)
	{
		if (trap_eval_icc)
		{
			m_trap = 1;
			m_trap_instruction = 1;
			m_ticc_trap_type = trap_number & 0x7f;
		}
		else
		{
			PC = nPC;
			nPC = nPC + 4;
		}
	}
}



//-------------------------------------------------
//  select_trap - prioritize traps and perform any
//  additional functions from taking them
//-------------------------------------------------

void sparc_base_device::select_trap()
{
	if (!m_trap)
		return;

	if (m_reset_trap)
	{
		m_trap = 0;
		return;
	}
	else if (!m_et)
	{
		m_execute_mode = 0;
		m_error_mode = 1;
		m_icount = 0;
	}
	else
	{
		update_tt();
	}

	TBR = (TBR & 0xfffff000) | (m_tt << 4);
	m_trap = 0;
	m_instruction_access_exception = 0;
	m_illegal_instruction = 0;
	m_privileged_instruction = 0;
	m_fp_disabled = 0;
	m_fp_exception = 0;
	m_cp_disabled = 0;
	m_window_overflow = 0;
	m_window_underflow = 0;
	m_mem_address_not_aligned = 0;
	m_cp_exception = 0;
	m_data_access_exception = 0;
	m_tag_overflow = 0;
	m_trap_instruction = 0;
	m_interrupt_level = 0;
	m_mae = 0;
}


//-------------------------------------------------
//  update_tt - determine TT register contents
//  based on trap priority
//-------------------------------------------------

void sparcv7_device::update_tt()
{
	if (m_instruction_access_exception)
		m_tt = 0x01;
	else if (m_privileged_instruction)
		m_tt = 0x03;
	else if (m_illegal_instruction)
		m_tt = 0x02;
	else if (m_fp_disabled)
		m_tt = 0x04;
	else if (m_cp_disabled)
		m_tt = 0x24;
	else if (m_window_overflow)
		m_tt = 0x05;
	else if (m_window_underflow)
		m_tt = 0x06;
	else if (m_mem_address_not_aligned)
		m_tt = 0x07;
	else if (m_fp_exception)
		m_tt = 0x08;
	else if (m_cp_exception)
		m_tt = 0x28;
	else if (m_data_access_exception)
		m_tt = 0x09;
	else if (m_tag_overflow)
		m_tt = 0x0a;
	else if (m_trap_instruction)
		m_tt = 0x80 | m_ticc_trap_type;
	else if (m_interrupt_level > 0)
		m_tt = 0x10 | m_interrupt_level;
}


void sparcv8_device::update_tt()
{
	if (m_data_store_error)
		m_tt = 0x2b;
	else if (m_instruction_access_error)
		m_tt = 0x21;
	else if (m_r_register_access_error)
		m_tt = 0x20;
	else if (m_instruction_access_exception)
		m_tt = 0x01;
	else if (m_privileged_instruction)
		m_tt = 0x03;
	else if (m_illegal_instruction)
		m_tt = 0x02;
	else if (m_fp_disabled)
		m_tt = 0x04;
	else if (m_cp_disabled)
		m_tt = 0x24;
	else if (m_unimplemented_FLUSH)
		m_tt = 0x25;
	else if (m_window_overflow)
		m_tt = 0x05;
	else if (m_window_underflow)
		m_tt = 0x06;
	else if (m_mem_address_not_aligned)
		m_tt = 0x07;
	else if (m_fp_exception)
		m_tt = 0x08;
	else if (m_cp_exception)
		m_tt = 0x28;
	else if (m_data_access_error)
		m_tt = 0x29;
	else if (m_data_access_exception)
		m_tt = 0x09;
	else if (m_tag_overflow)
		m_tt = 0x0a;
	else if (m_division_by_zero)
		m_tt = 0x2a;
	else if (m_trap_instruction)
		m_tt = 0x80 | m_ticc_trap_type;
	else if (m_interrupt_level > 0)
		m_tt = 0x10 | m_interrupt_level;

	m_unimplemented_FLUSH = 0;
	m_r_register_access_error = 0;
	m_instruction_access_error = 0;
	m_data_access_error = 0;
	m_data_store_error = 0;
	m_division_by_zero = 0;
}


//-------------------------------------------------
//  execute_trap - prioritize and invoke traps
//  that have been flagged by the previous
//  instructions, if any.
//-------------------------------------------------

void sparc_base_device::execute_trap()
{
	// The SPARC Instruction Manual: Version 8, page 161, "Appendix C - C.8. Traps" (SPARCv8.pdf, pg. 158)

	if (!m_trap)
	{
		return;
	}

	select_trap();

	if (!m_error_mode)
	{
		PSR &= ~PSR_ET_MASK;
		m_et = false;

		if (IS_USER)
			PSR &= ~PSR_PS_MASK;
		else
			PSR |= PSR_PS_MASK;

		PSR |= PSR_S_MASK;
		m_s = true;
		m_data_space = 11;

		int cwp = PSR & PSR_CWP_MASK;
		int new_cwp = ((cwp + NWINDOWS) - 1) % NWINDOWS;

		PSR &= ~PSR_CWP_MASK;
		PSR |= new_cwp;

		update_gpr_pointers();

		if (m_no_annul)
		{
			REG(17) = PC;
			REG(18) = nPC;
		}
		else
		{
			REG(17) = nPC;
			REG(18) = nPC + 4;
			m_no_annul = true;
		}

		if (!m_reset_trap)
		{
			PC = TBR;
			nPC = TBR + 4;
		}
		else
		{
			PC = 0;
			nPC = 4;
			m_reset_trap = 0;
		}
	}
}


//-------------------------------------------------
//  dispatch_instruction - executes a
//  single fetched instruction.
//-------------------------------------------------

// The SPARC Instruction Manual: Version 8, page 159, "Appendix C - ISP Descriptions - C.6. Instruction Dispatch" (SPARCv8.pdf, pg. 156)

inline void sparc_base_device::dispatch_instruction(uint32_t op)
{
	const uint8_t op_type = OP;
	switch (op_type)
	{
	case OP_TYPE0:  // Bicc, SETHI, FBfcc
		switch (OP2)
		{
		case OP2_UNIMP: // unimp
			logerror("unimp @ %x\n", PC);
			break;
		case OP2_BICC: // branch on integer condition codes
			execute_bicc(op);
			break;
		case OP2_SETHI: // sethi or nop
			if (RDBITS) RDREG = op << 10;
			PC = nPC;
			nPC = nPC + 4;
			break;
		case OP2_FBFCC: // branch on floating-point condition codes
			if (!(PSR & PSR_EF_MASK) || !m_bp_fpu_present)
			{
				m_trap = 1;
				m_fp_disabled = 1;
				return;
			}
			execute_fbfcc(op);
			break;
		default:
			if (!dispatch_extra_instruction(op))
			{
				logerror("illegal instruction at %08x: %08x\n", PC, op);
				m_trap = 1;
				m_illegal_instruction = 1;
			}
			return;
		}
		break;

	case OP_CALL:
	{
		uint32_t pc = PC;
		uint32_t callpc = PC + DISP30;
		PC = nPC;
		nPC = callpc;

		REG(15) = pc;
		break;
	}

	case OP_ALU:
		execute_group2(op);
		break;

	case OP_LDST:
		execute_group3(op);
		break;
	}
}

bool sparcv7_device::dispatch_extra_instruction(uint32_t op)
{
	return false;
}

bool sparcv8_device::dispatch_extra_instruction(uint32_t op)
{
	const uint8_t op_type = OP;
	switch (op_type)
	{
	case OP_TYPE0:  // Bicc, SETHI, FBfcc
		switch (OP2)
		{
		case OP2_CBCCC: // branch on coprocessor condition codes, SPARCv8
			if (!(PSR & PSR_EC_MASK) || !m_bp_cp_present)
			{
				logerror("cbccc @ %08x: %08x\n", PC, op);
				m_trap = 1;
				m_cp_disabled = 1;
				return true;
			}
			return true;
		default:
			return false;
		}
	default:
		return false;
	}
}

void sparc_base_device::check_fdiv_zero_exception()
{
	m_fsr |= FSR_CEXC_DZC;
	if (m_fsr & FSR_TEM_DZM)
	{
		m_fsr = (m_fsr & ~FSR_FTT_MASK) | FSR_FTT_IEEE;
		m_fp_exception_pending = true;
		return;
	}
	m_fsr |= FSR_AEXC_DZA;
}

bool sparc_base_device::check_fp_exceptions()
{
	if (softfloat_exceptionFlags & softfloat_flag_inexact)
		m_fsr |= FSR_CEXC_NXC;
	if (softfloat_exceptionFlags & softfloat_flag_underflow)
		m_fsr |= FSR_CEXC_UFC;
	if (softfloat_exceptionFlags & softfloat_flag_overflow)
		m_fsr |= FSR_CEXC_OFC;
	if (softfloat_exceptionFlags & softfloat_flag_invalid)
		m_fsr |= FSR_CEXC_NVC;

	// accrue disabled exceptions
	const uint32_t cexc = m_fsr & FSR_CEXC_MASK;
	const uint32_t tem = (m_fsr & FSR_TEM_MASK) >> FSR_TEM_SHIFT;
	m_fsr |= (~tem & cexc) << FSR_AEXC_SHIFT;

	// check if exception is enabled
	if (tem & cexc)
	{
		m_fsr = (m_fsr & ~FSR_FTT_MASK) | FSR_FTT_IEEE;
		m_fp_exception = 1;
		return true;
	}
	return false;
}

bool sparc_base_device::get_fpr32(uint32_t &data, const uint32_t rd)
{
	if (m_pending_fpr == rd)
	{
		return true;
	}
	data = FREG(rd);
	return false;
}

bool sparc_base_device::get_fpr64(uint64_t &data, const uint32_t rd)
{
	if (m_pending_fpr == rd || m_pending_fpr == (rd | 1))
	{
		return true;
	}
	data = (uint64_t)FREG(rd) << 32;
	data |= FREG(rd | 1);
	return false;
}

bool sparc_base_device::set_fpr32(const uint32_t rd, const uint32_t data)
{
	m_pending_fpr = rd;
	if (softfloat_exceptionFlags && check_fp_exceptions())
	{
		m_fp_exception_pending = true;
		m_fpr[rd] = 0;
		return true;
	}

	m_fpr[rd] = data;
	return false;
}

bool sparc_base_device::set_fpr64(const uint32_t rd, const uint64_t data)
{
	m_pending_fpr = rd;
	if (softfloat_exceptionFlags && check_fp_exceptions())
	{
		m_fp_exception_pending = true;
		m_fpr[rd] = 0;
		m_fpr[rd + 1] = 0;
		return true;
	}

	m_fpr[rd]     = (uint32_t)(data >> 32);
	m_fpr[rd + 1] = (uint32_t)data;
	return false;
}

//-------------------------------------------------
//  complete_fp_execution - completes execution
//  of a floating-point operation
//-------------------------------------------------

void sparc_base_device::complete_fp_execution(uint32_t op)
{
	softfloat_exceptionFlags = 0;

	uint32_t rs1 = 0;
	uint32_t rs2 = 0;
	uint64_t rd1 = 0;
	uint64_t rd2 = 0;

	const uint32_t fpop = (op >> 5) & 0x1ff;
	switch (fpop)
	{
	case FPOP_FMOVS:
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 4;
		set_fpr32(RD, rs2);
		break;
	case FPOP_FNEGS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 4;
		const float32_t fs2 = float32_t{ rs2 };
		set_fpr32(RD, f32_mul(fs2, i32_to_f32(-1)).v);
		break;
	}
	case FPOP_FABSS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 4;
		const float32_t fs2 = float32_t{ rs2 };
		if (f32_lt(fs2, float32_t{0}))
		{
			set_fpr32(RD, f32_mul(fs2, i32_to_f32(-1)).v);
		}
		else
		{
			set_fpr32(RD, rs2);
		}
		break;
	}
	case FPOP_FSQRTS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 62;
		const float32_t fs2 = float32_t{ rs2 };
		set_fpr32(RD, f32_sqrt(fs2).v);
		break;
	}
	case FPOP_FSQRTD:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 120;
		const float64_t fs2 = float64_t{ rd2 };
		set_fpr64(RD_D, f64_sqrt(fs2).v);
		break;
	}
	case FPOP_FADDS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs1, RS1) || get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 8;
		const float32_t fs1 = float32_t{ rs1 };
		const float32_t fs2 = float32_t{ rs2 };
		set_fpr32(RD, f32_add(fs1, fs2).v);
		break;
	}
	case FPOP_FADDD:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr64(rd1, RS1_D) || get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 8;
		const float64_t fs1 = float64_t{ rd1 };
		const float64_t fs2 = float64_t{ rd2 };
		set_fpr64(RD_D, f64_add(fs1, fs2).v);
		break;
	}
	case FPOP_FSUBS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs1, RS1) || get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 8;
		const float32_t fs1 = float32_t{ rs1 };
		const float32_t fs2 = float32_t{ rs2 };
		set_fpr32(RD, f32_sub(fs1, fs2).v);
		break;
	}
	case FPOP_FSUBD:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr64(rd1, RS1_D) || get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 8;
		const float64_t fs1 = float64_t{ rd1 };
		const float64_t fs2 = float64_t{ rd2 };
		set_fpr64(RD_D, f64_sub(fs1, fs2).v);
		break;
	}
	case FPOP_FMULS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs1, RS1) || get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 8;
		const float32_t fs1 = float32_t{ rs1 };
		const float32_t fs2 = float32_t{ rs2 };
		set_fpr32(RD, f32_mul(fs1, fs2).v);
		break;
	}
	case FPOP_FMULD:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr64(rd1, RS1_D) || get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 14;
		const float64_t fs1 = float64_t{ rd1 };
		const float64_t fs2 = float64_t{ rd2 };
		set_fpr64(RD_D, f64_mul(fs1, fs2).v);
		break;
	}
	case FPOP_FDIVS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs1, RS1) || get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 40;
		if (rs2 == 0)
		{
			check_fdiv_zero_exception();
			m_pending_fpr = RD;
			break;
		}
		const float32_t fs1 = float32_t{ rs1 };
		const float32_t fs2 = float32_t{ rs2 };
		set_fpr32(RD, f32_div(fs1, fs2).v);
		break;
	}
	case FPOP_FDIVD:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr64(rd1, RS1_D) || get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 68;
		if (rd2 == 0)
		{
			check_fdiv_zero_exception();
			m_pending_fpr = RD;
			break;
		}
		const float64_t fs1 = float64_t{ rd1 };
		const float64_t fs2 = float64_t{ rd2 };
		set_fpr64(RD_D, f64_div(fs1, fs2).v);
		break;
	}
	case FPOP_FITOS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 16;
		set_fpr32(RD, i32_to_f32(int32_t(rs2)).v);
		break;
	}
	case FPOP_FDTOS:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 8;
		const float64_t fs2 = float64_t{ rd2 };
		set_fpr32(RD, f64_to_f32(fs2).v);
		break;
	}
	case FPOP_FITOD:
	{
		if (RD_D == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 8;
		set_fpr64(RD_D, i32_to_f64(int32_t(rs2)).v);
		break;
	}
	case FPOP_FSTOD:
	{
		if (RD_D == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 8;
		const float32_t fs = float32_t{ rs2 };
		set_fpr64(RD_D, f32_to_f64(fs).v);
		break;
	}
	case FPOP_FSTOI:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 16; // Guessed based on FITOS
		const float32_t fs2 = float32_t{ rs2 };
		set_fpr32(RD, f32_to_i32(fs2, softfloat_roundingMode, true));
		break;
	}
	case FPOP_FDTOI:
	{
		if (RD == m_pending_fpr) return;
		if (get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 8; // Guessed based on FITOD
		const float64_t fs2 = float64_t{ rd2 };
		set_fpr32(RD, f64_to_i32(fs2, softfloat_roundingMode, true));
		break;
	}
	case FPOP_FCMPS:
	{
		if (get_fpr32(rs1, RS1) || get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 4;
		const float32_t fs1 = float32_t{ rs1 };
		const float32_t fs2 = float32_t{ rs2 };
		bool equal = f32_eq(fs1, fs2);
		if (softfloat_exceptionFlags & softfloat_flag_invalid)
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_UO;
		else if (equal)
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_EQ;
		else if (f32_lt(fs1, fs2))
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_LT;
		else
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_GT;
		break;
	}
	case FPOP_FCMPD:
	{
		if (get_fpr64(rd1, RS1_D) || get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 4;
		const float64_t fs1 = float64_t{ rd1 };
		const float64_t fs2 = float64_t{ rd2 };
		bool equal = f64_eq(fs1, fs2);
		if (softfloat_exceptionFlags & softfloat_flag_invalid)
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_UO;
		else if (equal)
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_EQ;
		else if (f64_lt(fs1, fs2))
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_LT;
		else
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_GT;
		break;
	}
	case FPOP_FCMPES:
	{
		if (get_fpr32(rs1, RS1) || get_fpr32(rs2, RS2)) return;
		m_fpr_pending = 4;
		const float32_t fs1 = float32_t{ rs1 };
		const float32_t fs2 = float32_t{ rs2 };
		bool equal = f32_eq(fs1, fs2);
		if (softfloat_exceptionFlags & softfloat_flag_invalid)
		{
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_UO;
			m_fsr |= FSR_CEXC_NVC;
			if (m_fsr & FSR_TEM_NVM)
			{
				m_fsr = (m_fsr & ~FSR_FTT_MASK) | FSR_FTT_IEEE;
				m_fp_exception = 1;
				break;
			}
			m_fsr |= FSR_AEXC_NVA;
		}
		else if (equal)
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_EQ;
		else if (f32_lt(fs1, fs2))
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_LT;
		else
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_GT;
		break;
	}
	case FPOP_FCMPED:
	{
		if (get_fpr64(rd1, RS1_D) || get_fpr64(rd2, RS2_D)) return;
		m_fpr_pending = 4;
		const float64_t fs1 = float64_t{ rd1 };
		const float64_t fs2 = float64_t{ rd2 };
		bool equal = f64_eq(fs1, fs2);
		if (softfloat_exceptionFlags & softfloat_flag_invalid)
		{
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_UO;
			m_fsr |= FSR_CEXC_NVC;
			if (m_fsr & FSR_TEM_NVM)
			{
				m_fsr = (m_fsr & ~FSR_FTT_MASK) | FSR_FTT_IEEE;
				m_fp_exception = 1;
				break;
			}
			m_fsr |= FSR_AEXC_NVA;
		}
		else if (equal)
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_EQ;
		else if (f64_lt(fs1, fs2))
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_LT;
		else
			m_fsr = (m_fsr & ~FSR_FCC_MASK) | FSR_FCC_GT;
		break;
	}
	case FPOP_FSQRTX:
	case FPOP_FADDX:
	case FPOP_FSUBX:
	case FPOP_FMULX:
	case FPOP_FDIVX:
	case FPOP_FXTOI:
	case FPOP_FXTOS:
	case FPOP_FXTOD:
	case FPOP_FITOX:
	case FPOP_FSTOX:
	case FPOP_FDTOX:
	case FPOP_FCMPX:
	case FPOP_FCMPEX:
	default:
		m_fsr = (m_fsr & ~FSR_FTT_MASK) | FSR_FTT_UNIMP;
		m_fp_exception = 1;
		break;
	}

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_swap - execute a swap instruction
//-------------------------------------------------

void sparcv8_device::execute_swap(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 169, "Appendix C - ISP Descriptions - Atomic Load-Store Unsigned Byte Instructions" (SPARCv8.pdf, pg. 166)

	uint32_t address = 0;
	uint8_t addr_space = 0;
	if (SWAP)
	{
		address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
		addr_space = (IS_USER ? 10 : 11);
	}
	else if (SWAPA)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
			return;
		}
		else if (USEIMM)
		{
			m_trap = 1;
			m_illegal_instruction = 1;
			return;
		}
		else
		{
			address = RS1REG + RS2REG;
			addr_space = ASI;
		}
	}

	if (address & 3)
	{
		m_trap = 1;
		m_mem_address_not_aligned = 1;
		return;
	}

	uint32_t word = 0;
	uint32_t data = RDREG;

	//while (m_pb_block_ldst_byte || m_pb_block_ldst_word)
	//{
		// { wait for lock(s) to be lifted }
		// { an implementation actually need only block when another SWAP is pending on
		//   the same word in memory as the one addressed by this SWAP, or a LDSTUB is
		//   pending on any byte of the word in memory addressed by this SWAP }
	//}

	m_pb_block_ldst_word = 1;

	word = read_word(addr_space, address);

	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	write_word(addr_space, address, data);

	m_pb_block_ldst_word = 0;

	if (MAE)
	{
		m_trap = 1;
		m_data_access_exception = 1;
		return;
	}

	if (RDBITS)
		RDREG = word;

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_mul - execute a multiply opcode
//-------------------------------------------------

void sparcv8_device::execute_mul(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 175, "Appendix C - ISP Descriptions - Multiply Instructions" (SPARCv8.pdf, pg. 172)

	uint32_t operand2 = (USEIMM ? SIMM13 : RS2REG);

	uint32_t result = 0;
	if (UMUL || UMULCC)
	{
		uint64_t dresult = (uint64_t)RS1REG * (uint64_t)operand2;
		Y = (uint32_t)(dresult >> 32);
		result = (uint32_t)dresult;
	}
	else if (SMUL || SMULCC)
	{
		int64_t dresult = (int64_t)(int32_t)RS1REG * (int64_t)(int32_t)operand2;
		Y = (uint32_t)(dresult >> 32);
		result = (uint32_t)dresult;
	}

	if (RDBITS)
	{
		RDREG = result;
	}
	if (UMULCC || SMULCC)
	{
		CLEAR_ICC;
		PSR |= BIT31(result) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
	}

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_div - execute a divide opcode
//-------------------------------------------------

void sparcv8_device::execute_div(uint32_t op)
{
	// The SPARC Instruction Manual: Version 8, page 176, "Appendix C - ISP Descriptions - Multiply Instructions" (SPARCv8.pdf, pg. 173)

	uint32_t operand2 = (USEIMM ? SIMM13 : RS2REG);

	if (operand2 == 0)
	{
		m_trap = 1;
		m_division_by_zero = 1;
		return;
	}
	else
	{
		uint32_t result = 0;
		bool temp_v = false;
		int64_t temp_64bit = 0;
		if (UDIV || UDIVCC)
		{
			temp_64bit = int64_t(uint64_t((uint64_t(Y) << 32) | uint64_t(RS1REG)) / operand2);

			result = uint32_t(temp_64bit);

			temp_v = ((temp_64bit & 0xffffffff00000000ULL) == 0) ? false : true;
		}
		else if (SDIV || SDIVCC)
		{
			temp_64bit = int64_t(int64_t((uint64_t(Y) << 32) | uint64_t(RS1REG)) / operand2);

			result = uint32_t(temp_64bit);

			uint64_t shifted = uint64_t(temp_64bit) >> 31;
			temp_v = (shifted == 0 || shifted == 0x1ffffffffULL) ? false : true;
		}

		if (temp_v)
		{
			if (UDIV || UDIVCC)
			{
				result = 0xffffffff;
			}
			else if (SDIV || SDIVCC)
			{
				if (temp_64bit > 0)
					result = 0x7fffffff;
				else
					result = 0x80000000;
			}
		}

		if (RDBITS)
			RDREG = result;

		if (UDIVCC || SDIVCC)
		{
			CLEAR_ICC;
			PSR |= BIT31(result) ? PSR_N_MASK : 0;
			PSR |= (result == 0) ? PSR_Z_MASK : 0;
			PSR |= temp_v ? PSR_V_MASK : 0;
		}
	}

	PC = nPC;
	nPC = nPC + 4;
}


//-------------------------------------------------
//  execute_step - perform one step in execute
//  mode (versus error or reset modes)
//-------------------------------------------------

inline void sparc_base_device::execute_step()
{
	// The SPARC Instruction Manual: Version 8, page 156, "Appendix C - ISP Descriptions - C.5. Processor States and Instruction Dispatch" (SPARCv8.pdf, pg. 153)

	// write-state-register delay not yet implemented

	const uint32_t op = read_word(8 + (IS_SUPERVISOR ? 1 : 0), PC);

#if LOG_FCODES
	//if (m_log_fcodes)
	{
		log_fcodes();
	}
#endif

	if (m_no_annul)
	{
		if (MAE)
		{
			m_trap = 1;
			m_instruction_access_exception = 1;
			return;
		}
		dispatch_instruction(op);

		if (m_fpr_pending)
		{
			m_fpr_pending--;
			if (!m_fpr_pending)
			{
				m_pending_fpr = std::size(m_fpr);
				if (m_fp_exception_pending)
				{
					m_fp_exception_pending = false;
					m_trap = 1;
					m_fp_exception = 1;
					return;
				}
			}
		}
	}
	else
	{
		m_no_annul = true;
		PC = nPC;
		nPC = nPC + 4;
	}
}


//-------------------------------------------------
//  reset_step - step one cycle in reset mode
//-------------------------------------------------

void sparc_base_device::reset_step()
{
	// The SPARC Instruction Manual: Version 8, page 156, "Appendix C - ISP Descriptions - C.5. Processor States and Instruction Dispatch" (SPARCv8.pdf, pg. 153)

	m_reset_mode = 0;
	m_execute_mode = 1;
	m_trap = 1;
	m_reset_trap = 1;
}


//-------------------------------------------------
//  error_step - step one cycle in error mode
//-------------------------------------------------

void sparc_base_device::error_step()
{
	// The SPARC Instruction Manual: Version 8, page 157, "Appendix C - ISP Descriptions - C.5. Processor States and Instruction Dispatch" (SPARCv8.pdf, pg. 154)

	// waiting for SPARC_RESET
	m_icount = 0;
}

template <bool CHECK_DEBUG, sparc_base_device::running_mode MODE>
void sparc_base_device::run_loop()
{
	do
	{
		/*if (HOLD_BUS)
		{
		    m_icount--;
		    continue;
		}*/

		if (CHECK_DEBUG)
			debugger_instruction_hook(PC);

		if (MODE == MODE_RESET)
		{
			reset_step();
		}
		else if (MODE == MODE_ERROR)
		{
			error_step();
		}
		else if (MODE == MODE_EXECUTE)
		{
			execute_step();
		}

		if (CHECK_DEBUG)
		{
			for (int i = 0; i < 8; i++)
			{
				m_dbgregs[i]        = *m_regs[8 + i];
				m_dbgregs[8 + i]    = *m_regs[16 + i];
				m_dbgregs[16 + i]   = *m_regs[24 + i];
			}
		}
		--m_icount;
	} while (m_icount > 0 && !m_trap);
}

//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void sparc_base_device::execute_run()
{
	bool debug = machine().debug_flags & DEBUG_FLAG_ENABLED;

	if (m_bp_reset_in)
	{
		if (m_error_mode)
		{
			m_pb_error = 0;
		}

		m_execute_mode = 0;
		m_error_mode = 0;
		m_reset_mode = 1;
		m_icount = 0;
		return;
	}
	else if (m_et && (m_bp_irl == 15 || m_bp_irl > m_pil))
	{
		m_trap = 1;
		m_interrupt_level = m_bp_irl;
	}

	do
	{
		if (m_trap)
		{
			execute_trap();
		}

		if (debug)
		{
			if (m_reset_mode)
				run_loop<true, MODE_RESET>();
			else if (m_error_mode)
				run_loop<true, MODE_ERROR>();
			else
				run_loop<true, MODE_EXECUTE>();
		}
		else
		{
			if (m_reset_mode)
				run_loop<false, MODE_RESET>();
			else if (m_error_mode)
				run_loop<false, MODE_ERROR>();
			else
				run_loop<false, MODE_EXECUTE>();
		}
	} while (m_icount > 0);
}


//-------------------------------------------------
//  get_reg_r - get integer register value for
//  disassembler
//-------------------------------------------------

uint64_t sparc_base_device::get_reg_r(unsigned index) const
{
	return REG(index & 31);
}


//-------------------------------------------------
//  get_reg_pc - get program counter value for
//  disassembler
//-------------------------------------------------

uint64_t sparc_base_device::get_translated_pc() const
{
	// FIXME: how do we apply translation to the address so it's in the same space the disassembler sees?
	return m_pc;
}


//-------------------------------------------------
//  get_icc - get integer condition codes for
//  disassembler
//-------------------------------------------------

uint8_t sparc_base_device::get_icc() const
{
	return (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT;
}


//-------------------------------------------------
//  get_icc - get extended integer condition codes
//  for disassembler
//-------------------------------------------------

uint8_t sparc_base_device::get_xcc() const
{
	// not present before SPARCv9
	return 0;
}


//-------------------------------------------------
//  get_icc - get extended integer condition codes
//  for disassembler
//-------------------------------------------------

uint8_t sparc_base_device::get_fcc(unsigned index) const
{
	// only one fcc instance before SPARCv9
	return (m_fsr >> 10) & 3;
}
