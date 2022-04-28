// license:BSD-3-Clause
// copyright-holders:Ville Linde, Ryan Holtz
/*
    Nintendo/SGI Reality Signal Processor (RSP) emulator

    Written by Ville Linde
*/

#include "emu.h"
#include "rsp.h"

#include "rspdefs.h"
#include "rspdiv.h"

#include "rsp_dasm.h"

DEFINE_DEVICE_TYPE(RSP, rsp_device, "rsp", "Nintendo & SGI Reality Signal Processor RSP")


#define LOG_INSTRUCTION_EXECUTION       0
#define SAVE_DISASM                     0
#define SAVE_DMEM                       0
#define RSP_TEST_SYNC                   0

#define PRINT_VECREG(x)     osd_printf_debug("V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X\n", x, \
							m_v[x].w[0], m_v[x].w[1], m_v[x].w[2], m_v[x].w[3], \
							m_v[x].w[4], m_v[x].w[5], m_v[x].w[6], m_v[x].w[7])

#define PRINT_ACCUM(x)     osd_printf_debug("A%d: %08X|%08X\n", x, (uint32_t)(m_accum[x].q >> 32), (uint32_t)m_accum[x].q);


#define SIMM16      ((int32_t)(int16_t)(op))
#define UIMM16      ((uint16_t)(op))
#define UIMM26      (op & 0x03ffffff)

#define JUMP_ABS(addr)          { m_nextpc = (addr) << 2; }
#define JUMP_ABS_L(addr,l)      { m_nextpc = (addr) << 2; m_r[l] = m_pc + 4; }
#define JUMP_REL(offset)        { m_nextpc = m_pc + ((offset) << 2); }
#define JUMP_REL_L(offset,l)    { m_nextpc = m_pc + ((offset) << 2); m_r[l] = m_pc + 4; }
#define JUMP_PC(addr)           { m_nextpc = addr; }
#define JUMP_PC_L(addr,l)       { m_nextpc = addr; m_r[l] = m_pc + 4; }

#define ROPCODE(pc)             m_icache.read_dword(pc & 0xfff)

/***************************************************************************
    Helpful Vector Defines
***************************************************************************/

#define VDREG   ((op >> 6) & 0x1f)
#define VS1REG  ((op >> 11) & 0x1f)
#define VS2REG  ((op >> 16) & 0x1f)
#define EL      ((op >> 21) & 0xf)

#define VREG_B(reg, offset)     m_v[(reg)].b[(offset)^1]
#define W_VREG_B(reg, offset, val)  (m_v[(reg)].b[(offset)^1] = val)

#define VEC_EL_2(x,z)               (vector_elements_2[(x)][(z)])

#define CARRY       0
#define COMPARE     1
#define CLIP1       2
#define ZERO        3
#define CLIP2       4

#define SLICE_H             3
#define SLICE_M             2
#define SLICE_L             1
#define SLICE_LL            0

#define WRITEBACK_RESULT() memcpy(m_v[VDREG].s, vres, sizeof(uint16_t) * 8);

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

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define SINGLE_INSTRUCTION_MODE         (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES         128
#define COMPILE_FORWARDS_BYTES          512
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/4) + (COMPILE_FORWARDS_BYTES/4))
#define COMPILE_MAX_SEQUENCE            64

/* size of the execution code cache */
#define CACHE_SIZE                      (32 * 1024 * 1024)


rsp_device::rsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, RSP, tag, owner, clock)
	, m_imem_config("imem", ENDIANNESS_BIG, 32, 12)
	, m_dmem_config("dmem", ENDIANNESS_BIG, 32, 12)
	, m_exec_output(nullptr)
	, m_sr(0)
	, m_step_count(0)
	, m_ppc(0)
	, m_nextpc(0xffff)
	, m_debugger_temp(0)
	, m_pc_temp(0)
	, m_ppc_temp(0)
	, m_nextpc_temp(0xffff)
	, m_dp_reg_r_func(*this)
	, m_dp_reg_w_func(*this)
	, m_sp_reg_r_func(*this)
	, m_sp_reg_w_func(*this)
	, m_sp_set_status_func(*this)
{
}

rsp_device::~rsp_device()
{
}

device_memory_interface::space_config_vector rsp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_imem_config),
		std::make_pair(AS_DATA, &m_dmem_config)
	};
}

std::unique_ptr<util::disasm_interface> rsp_device::create_disassembler()
{
	return std::make_unique<rsp_disassembler>();
}

uint8_t rsp_device::read_dmem_byte(uint32_t address)
{
	return m_dcache.read_byte(address);
}

uint16_t rsp_device::read_dmem_word(uint32_t address)
{
	return m_dcache.read_word_unaligned(address);
}

uint32_t rsp_device::read_dmem_dword(uint32_t address)
{
	return m_dcache.read_dword_unaligned(address);
}

void rsp_device::write_dmem_byte(uint32_t address, uint8_t data)
{
	m_dcache.write_byte(address, data);
}

void rsp_device::write_dmem_word(uint32_t address, uint16_t data)
{
	m_dcache.write_word_unaligned(address, data);
}

void rsp_device::write_dmem_dword(uint32_t address, uint32_t data)
{
	m_dcache.write_dword_unaligned(address, data);
}

/*****************************************************************************/

uint32_t rsp_device::get_cop0_reg(int reg)
{
	reg &= 0xf;
	if (reg < 8)
	{
		return m_sp_reg_r_func(reg, 0xffffffff);
	}
	else if (reg >= 8 && reg < 16)
	{
		return m_dp_reg_r_func(reg - 8, 0xffffffff);
	}

	return 0;
}

void rsp_device::set_cop0_reg(int reg, uint32_t data)
{
	reg &= 0xf;
	if (reg < 8)
	{
		m_sp_reg_w_func(reg, data, 0xffffffff);
	}
	else if (reg >= 8 && reg < 16)
	{
		m_dp_reg_w_func(reg - 8, data, 0xffffffff);
	}
}

void rsp_device::unimplemented_opcode(uint32_t op)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		std::ostringstream string;
		rsp_disassembler rspd;
		rspd.dasm_one(string, m_ppc, op);
		osd_printf_debug("%08X: %s\n", m_ppc, string.str());
	}

#if SAVE_DISASM
	{
		char string[200];
		int i;
		FILE *dasm;
		dasm = fopen("rsp_disasm.txt", "wt");

		for (i=0; i < 0x1000; i+=4)
		{
			uint32_t opcode = ROPCODE(0x04001000 + i);
			rsp_dasm_one(string, 0x04001000 + i, opcode);
			fprintf(dasm, "%08X: %08X   %s\n", 0x04001000 + i, opcode, string);
		}
		fclose(dasm);
	}
#endif
#if SAVE_DMEM
	{
		int i;
		FILE *dmem;
		dmem = fopen("rsp_dmem.bin", "wb");

		for (i=0; i < 0x1000; i++)
		{
			fputc(read_dmem_byte(i), dmem);
		}
		fclose(dmem);
	}
#endif

	fatalerror("RSP: unknown opcode %02X (%08X) at %08X\n", op >> 26, op, m_ppc);
}

/*****************************************************************************/

void rsp_device::resolve_cb()
{
	m_dp_reg_r_func.resolve();
	m_dp_reg_w_func.resolve();
	m_sp_reg_r_func.resolve();
	m_sp_reg_w_func.resolve();
	m_sp_set_status_func.resolve();
}

void rsp_device::device_start()
{
	if (LOG_INSTRUCTION_EXECUTION)
		m_exec_output = fopen("rsp_execute.txt", "wt");

	space(AS_PROGRAM).cache(m_icache);
	space(AS_PROGRAM).specific(m_imem);
	space(AS_DATA).cache(m_dcache);
	space(AS_DATA).specific(m_dmem);
	resolve_cb();

	for (int regIdx = 0; regIdx < 32; regIdx++)
		m_r[regIdx] = 0;

	for(auto & elem : m_v)
	{
		elem.d[0] = 0;
		elem.d[1] = 0;
	}

	m_vcarry = 0;
	m_vcompare = 0;
	m_vclip1 = 0;
	m_vzero = 0;
	m_vclip2 = 0;

	m_reciprocal_res = 0;
	m_reciprocal_high = 0;
	m_ideduct = 0;
	m_scalar_busy = false;
	m_vector_busy = false;
	m_paired_busy = false;

	for(auto & elem : m_accum)
	{
		elem.q = 0;
	}

	m_pc = 0;
	m_nextpc = 0xffff;
	m_sr = RSP_STATUS_HALT;
	m_step_count = 0;

	state_add( RSP_PC,      "PC", m_pc).callimport().callexport().formatstr("%08X");
	state_add( RSP_R0,      "R0", m_r[0]).formatstr("%08X");
	state_add( RSP_R1,      "R1", m_r[1]).formatstr("%08X");
	state_add( RSP_R2,      "R2", m_r[2]).formatstr("%08X");
	state_add( RSP_R3,      "R3", m_r[3]).formatstr("%08X");
	state_add( RSP_R4,      "R4", m_r[4]).formatstr("%08X");
	state_add( RSP_R5,      "R5", m_r[5]).formatstr("%08X");
	state_add( RSP_R6,      "R6", m_r[6]).formatstr("%08X");
	state_add( RSP_R7,      "R7", m_r[7]).formatstr("%08X");
	state_add( RSP_R8,      "R8", m_r[8]).formatstr("%08X");
	state_add( RSP_R9,      "R9", m_r[9]).formatstr("%08X");
	state_add( RSP_R10,     "R10", m_r[10]).formatstr("%08X");
	state_add( RSP_R11,     "R11", m_r[11]).formatstr("%08X");
	state_add( RSP_R12,     "R12", m_r[12]).formatstr("%08X");
	state_add( RSP_R13,     "R13", m_r[13]).formatstr("%08X");
	state_add( RSP_R14,     "R14", m_r[14]).formatstr("%08X");
	state_add( RSP_R15,     "R15", m_r[15]).formatstr("%08X");
	state_add( RSP_R16,     "R16", m_r[16]).formatstr("%08X");
	state_add( RSP_R17,     "R17", m_r[17]).formatstr("%08X");
	state_add( RSP_R18,     "R18", m_r[18]).formatstr("%08X");
	state_add( RSP_R19,     "R19", m_r[19]).formatstr("%08X");
	state_add( RSP_R20,     "R20", m_r[20]).formatstr("%08X");
	state_add( RSP_R21,     "R21", m_r[21]).formatstr("%08X");
	state_add( RSP_R22,     "R22", m_r[22]).formatstr("%08X");
	state_add( RSP_R23,     "R23", m_r[23]).formatstr("%08X");
	state_add( RSP_R24,     "R24", m_r[24]).formatstr("%08X");
	state_add( RSP_R25,     "R25", m_r[25]).formatstr("%08X");
	state_add( RSP_R26,     "R26", m_r[26]).formatstr("%08X");
	state_add( RSP_R27,     "R27", m_r[27]).formatstr("%08X");
	state_add( RSP_R28,     "R28", m_r[28]).formatstr("%08X");
	state_add( RSP_R29,     "R29", m_r[29]).formatstr("%08X");
	state_add( RSP_R30,     "R30", m_r[30]).formatstr("%08X");
	state_add( RSP_R31,     "R31", m_r[31]).formatstr("%08X");
	state_add( RSP_SR,      "SR",  m_sr).formatstr("%08X");
	state_add( RSP_NEXTPC,  "NPC", m_nextpc).callimport().callexport().formatstr("%04X");
	state_add( RSP_STEPCNT, "STEP",  m_step_count).formatstr("%08X");

	state_add( RSP_V0,      "V0",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V1,      "V1",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V2,      "V2",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V3,      "V3",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V4,      "V4",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V5,      "V5",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V6,      "V6",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V7,      "V7",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V8,      "V8",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V9,      "V9",  m_debugger_temp).formatstr("%39s");
	state_add( RSP_V10,     "V10", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V11,     "V11", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V12,     "V12", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V13,     "V13", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V14,     "V14", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V15,     "V15", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V16,     "V16", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V17,     "V17", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V18,     "V18", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V19,     "V19", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V20,     "V20", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V21,     "V21", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V22,     "V22", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V23,     "V23", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V24,     "V24", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V25,     "V25", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V26,     "V26", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V27,     "V27", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V28,     "V28", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V29,     "V29", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V30,     "V30", m_debugger_temp).formatstr("%39s");
	state_add( RSP_V31,     "V31", m_debugger_temp).formatstr("%39s");

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_r[31]).formatstr("%1s").noshow();

	set_icountptr(m_icount);
}

void rsp_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case RSP_PC:
			m_pc = (uint16_t)m_pc_temp;
			break;

		case STATE_GENPCBASE:
			m_ppc = (uint16_t)m_ppc_temp;
			break;

		case RSP_NEXTPC:
			m_nextpc = (uint16_t)m_nextpc_temp;
			break;
	}
}


void rsp_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case RSP_PC:
			m_pc_temp = m_pc;
			break;

		case STATE_GENPCBASE:
			m_ppc_temp = m_ppc;
			break;

		case RSP_NEXTPC:
			m_nextpc_temp = m_nextpc;
			break;
	}
}

void rsp_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	const int index = entry.index();
	switch (index)
	{
		case RSP_V0:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 0].w[0], m_v[ 0].w[1], m_v[ 0].w[2], m_v[ 0].w[3], m_v[ 0].w[4], m_v[ 0].w[5], m_v[ 0].w[6], m_v[ 0].w[7]); break;
		case RSP_V1:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 1].w[0], m_v[ 1].w[1], m_v[ 1].w[2], m_v[ 1].w[3], m_v[ 1].w[4], m_v[ 1].w[5], m_v[ 1].w[6], m_v[ 1].w[7]); break;
		case RSP_V2:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 2].w[0], m_v[ 2].w[1], m_v[ 2].w[2], m_v[ 2].w[3], m_v[ 2].w[4], m_v[ 2].w[5], m_v[ 2].w[6], m_v[ 2].w[7]); break;
		case RSP_V3:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 3].w[0], m_v[ 3].w[1], m_v[ 3].w[2], m_v[ 3].w[3], m_v[ 3].w[4], m_v[ 3].w[5], m_v[ 3].w[6], m_v[ 3].w[7]); break;
		case RSP_V4:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 4].w[0], m_v[ 4].w[1], m_v[ 4].w[2], m_v[ 4].w[3], m_v[ 4].w[4], m_v[ 4].w[5], m_v[ 4].w[6], m_v[ 4].w[7]); break;
		case RSP_V5:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 5].w[0], m_v[ 5].w[1], m_v[ 5].w[2], m_v[ 5].w[3], m_v[ 5].w[4], m_v[ 5].w[5], m_v[ 5].w[6], m_v[ 5].w[7]); break;
		case RSP_V6:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 6].w[0], m_v[ 6].w[1], m_v[ 6].w[2], m_v[ 6].w[3], m_v[ 6].w[4], m_v[ 6].w[5], m_v[ 6].w[6], m_v[ 6].w[7]); break;
		case RSP_V7:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 7].w[0], m_v[ 7].w[1], m_v[ 7].w[2], m_v[ 7].w[3], m_v[ 7].w[4], m_v[ 7].w[5], m_v[ 7].w[6], m_v[ 7].w[7]); break;
		case RSP_V8:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 8].w[0], m_v[ 8].w[1], m_v[ 8].w[2], m_v[ 8].w[3], m_v[ 8].w[4], m_v[ 8].w[5], m_v[ 8].w[6], m_v[ 8].w[7]); break;
		case RSP_V9:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[ 9].w[0], m_v[ 9].w[1], m_v[ 9].w[2], m_v[ 9].w[3], m_v[ 9].w[4], m_v[ 9].w[5], m_v[ 9].w[6], m_v[ 9].w[7]); break;
		case RSP_V10:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[10].w[0], m_v[10].w[1], m_v[10].w[2], m_v[10].w[3], m_v[10].w[4], m_v[10].w[5], m_v[10].w[6], m_v[10].w[7]); break;
		case RSP_V11:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[11].w[0], m_v[11].w[1], m_v[11].w[2], m_v[11].w[3], m_v[11].w[4], m_v[11].w[5], m_v[11].w[6], m_v[11].w[7]); break;
		case RSP_V12:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[12].w[0], m_v[12].w[1], m_v[12].w[2], m_v[12].w[3], m_v[12].w[4], m_v[12].w[5], m_v[12].w[6], m_v[12].w[7]); break;
		case RSP_V13:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[13].w[0], m_v[13].w[1], m_v[13].w[2], m_v[13].w[3], m_v[13].w[4], m_v[13].w[5], m_v[13].w[6], m_v[13].w[7]); break;
		case RSP_V14:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[14].w[0], m_v[14].w[1], m_v[14].w[2], m_v[14].w[3], m_v[14].w[4], m_v[14].w[5], m_v[14].w[6], m_v[14].w[7]); break;
		case RSP_V15:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[15].w[0], m_v[15].w[1], m_v[15].w[2], m_v[15].w[3], m_v[15].w[4], m_v[15].w[5], m_v[15].w[6], m_v[15].w[7]); break;
		case RSP_V16:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[16].w[0], m_v[16].w[1], m_v[16].w[2], m_v[16].w[3], m_v[16].w[4], m_v[16].w[5], m_v[16].w[6], m_v[16].w[7]); break;
		case RSP_V17:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[17].w[0], m_v[17].w[1], m_v[17].w[2], m_v[17].w[3], m_v[17].w[4], m_v[17].w[5], m_v[17].w[6], m_v[17].w[7]); break;
		case RSP_V18:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[18].w[0], m_v[18].w[1], m_v[18].w[2], m_v[18].w[3], m_v[18].w[4], m_v[18].w[5], m_v[18].w[6], m_v[18].w[7]); break;
		case RSP_V19:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[19].w[0], m_v[19].w[1], m_v[19].w[2], m_v[19].w[3], m_v[19].w[4], m_v[19].w[5], m_v[19].w[6], m_v[19].w[7]); break;
		case RSP_V20:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[20].w[0], m_v[20].w[1], m_v[20].w[2], m_v[20].w[3], m_v[20].w[4], m_v[20].w[5], m_v[20].w[6], m_v[20].w[7]); break;
		case RSP_V21:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[21].w[0], m_v[21].w[1], m_v[21].w[2], m_v[21].w[3], m_v[21].w[4], m_v[21].w[5], m_v[21].w[6], m_v[21].w[7]); break;
		case RSP_V22:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[22].w[0], m_v[22].w[1], m_v[22].w[2], m_v[22].w[3], m_v[22].w[4], m_v[22].w[5], m_v[22].w[6], m_v[22].w[7]); break;
		case RSP_V23:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[23].w[0], m_v[23].w[1], m_v[23].w[2], m_v[23].w[3], m_v[23].w[4], m_v[23].w[5], m_v[23].w[6], m_v[23].w[7]); break;
		case RSP_V24:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[24].w[0], m_v[24].w[1], m_v[24].w[2], m_v[24].w[3], m_v[24].w[4], m_v[24].w[5], m_v[24].w[6], m_v[24].w[7]); break;
		case RSP_V25:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[25].w[0], m_v[25].w[1], m_v[25].w[2], m_v[25].w[3], m_v[25].w[4], m_v[25].w[5], m_v[25].w[6], m_v[25].w[7]); break;
		case RSP_V26:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[26].w[0], m_v[26].w[1], m_v[26].w[2], m_v[26].w[3], m_v[26].w[4], m_v[26].w[5], m_v[26].w[6], m_v[26].w[7]); break;
		case RSP_V27:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[27].w[0], m_v[27].w[1], m_v[27].w[2], m_v[27].w[3], m_v[27].w[4], m_v[27].w[5], m_v[27].w[6], m_v[27].w[7]); break;
		case RSP_V28:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[28].w[0], m_v[28].w[1], m_v[28].w[2], m_v[28].w[3], m_v[28].w[4], m_v[28].w[5], m_v[28].w[6], m_v[28].w[7]); break;
		case RSP_V29:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[29].w[0], m_v[29].w[1], m_v[29].w[2], m_v[29].w[3], m_v[29].w[4], m_v[29].w[5], m_v[29].w[6], m_v[29].w[7]); break;
		case RSP_V30:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[30].w[0], m_v[30].w[1], m_v[30].w[2], m_v[30].w[3], m_v[30].w[4], m_v[30].w[5], m_v[30].w[6], m_v[30].w[7]); break;
		case RSP_V31:
			str = string_format("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", m_v[31].w[0], m_v[31].w[1], m_v[31].w[2], m_v[31].w[3], m_v[31].w[4], m_v[31].w[5], m_v[31].w[6], m_v[31].w[7]); break;
		case STATE_GENFLAGS:
			str = "";
			break;
	}
}

void rsp_device::device_stop()
{
#if SAVE_DISASM
	{
		char string[200];
		int i;
		FILE *dasm;
		dasm = fopen("rsp_disasm.txt", "wt");

		for (i=0; i < 0x1000; i+=4)
		{
			uint32_t opcode = ROPCODE(0x04001000 + i);
			rsp_dasm_one(string, 0x04001000 + i, opcode);
			fprintf(dasm, "%08X: %08X   %s\n", 0x04001000 + i, opcode, string);
		}
		fclose(dasm);
	}
#endif
#if SAVE_DMEM
	{
		int i;
		FILE *dmem = fopen("rsp_dmem.bin", "wb");

		for (i=0; i < 0x1000; i++)
		{
			fputc(read_dmem_byte(i), dmem);
		}
		fclose(dmem);
	}
#endif

	if (m_exec_output)
		fclose(m_exec_output);
	m_exec_output = nullptr;
}

void rsp_device::device_reset()
{
	m_nextpc = 0xffff;
}

uint16_t rsp_device::SATURATE_ACCUM(int accum, int slice, uint16_t negative, uint16_t positive)
{
	if ((int16_t)m_accum[accum].w[SLICE_H] < 0)
	{
		if ((uint16_t)m_accum[accum].w[SLICE_H] != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((int16_t)m_accum[accum].w[SLICE_M] >= 0)
			{
				return negative;
			}
			else
			{
				if (slice == 0)
				{
					return m_accum[accum].w[SLICE_L];
				}
				else if (slice == 1)
				{
					return m_accum[accum].w[SLICE_M];
				}
			}
		}
	}
	else
	{
		if ((uint16_t)m_accum[accum].w[SLICE_H] != 0)
		{
			return positive;
		}
		else
		{
			if ((int16_t)m_accum[accum].w[SLICE_M] < 0)
			{
				return positive;
			}
			else
			{
				if (slice == 0)
				{
					return m_accum[accum].w[SLICE_L];
				}
				else
				{
					return m_accum[accum].w[SLICE_M];
				}
			}
		}
	}
	return 0;
}

void rsp_device::handle_vector_ops(uint32_t op)
{
	uint16_t vres[8];

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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				if (s1 == -32768 && s2 == -32768)
				{
					// overflow
					m_accum[i].w[SLICE_H] = 0;
					m_accum[i].w[SLICE_M] = -32768;
					m_accum[i].w[SLICE_L] = -32768;
					vres[i] = 0x7fff;
				}
				else
				{
					int64_t r =  s1 * s2 * 2;
					r += 0x8000;    // rounding ?
					m_accum[i].w[SLICE_H] = (r < 0) ? 0xffff : 0; // Sign-extend to 48-bit
					m_accum[i].w[SLICE_M] = (int16_t)(r >> 16);
					m_accum[i].w[SLICE_L] = (uint16_t)r;
					vres[i] = m_accum[i].w[SLICE_M];
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				int64_t r = s1 * s2 * 2;
				r += 0x8000;    // rounding ?

				m_accum[i].w[SLICE_H] = (uint16_t)(r >> 32);
				m_accum[i].w[SLICE_M] = (uint16_t)(r >> 16);
				m_accum[i].w[SLICE_L] = (uint16_t)r;

				if (r < 0)
				{
					vres[i] = 0;
				}
				else if (((int16_t)m_accum[i].w[SLICE_H] ^ (int16_t)m_accum[i].w[SLICE_M]) < 0)
				{
					vres[i] = -1;
				}
				else
				{
					vres[i] = m_accum[i].w[SLICE_M];
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

			for (int i = 0; i < 8; i++)
			{
				uint32_t s1 = m_v[VS1REG].w[i];
				uint32_t s2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];
				uint32_t r = s1 * s2;

				m_accum[i].w[SLICE_H] = 0;
				m_accum[i].w[SLICE_M] = 0;
				m_accum[i].w[SLICE_L] = (uint16_t)(r >> 16);

				vres[i] = m_accum[i].w[SLICE_L];
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];   // not sign-extended
				int32_t r =  s1 * s2;

				m_accum[i].w[SLICE_H] = (r < 0) ? 0xffff : 0;      // sign-extend to 48-bit
				m_accum[i].w[SLICE_M] = (int16_t)(r >> 16);
				m_accum[i].w[SLICE_L] = (uint16_t)r;

				vres[i] = m_accum[i].w[SLICE_M];
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].w[i];     // not sign-extended
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				int32_t r = s1 * s2;

				m_accum[i].w[SLICE_H] = (r < 0) ? 0xffff : 0;      // sign-extend to 48-bit
				m_accum[i].w[SLICE_M] = (int16_t)(r >> 16);
				m_accum[i].w[SLICE_L] = (uint16_t)(r);

				vres[i] = m_accum[i].w[SLICE_L];
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				int32_t r = s1 * s2;

				m_accum[i].w[SLICE_H] = (int16_t)(r >> 16);
				m_accum[i].w[SLICE_M] = (uint16_t)(r);
				m_accum[i].w[SLICE_L] = 0;

				if (r < -32768) r = -32768;
				if (r >  32767) r = 32767;
				vres[i] = (int16_t)(r);
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				int32_t r = s1 * s2;

				uint64_t q = (uint64_t)(uint16_t)m_accum[i].w[SLICE_LL];
				q |= (((uint64_t)(uint16_t)m_accum[i].w[SLICE_L]) << 16);
				q |= (((uint64_t)(uint16_t)m_accum[i].w[SLICE_M]) << 32);
				q |= (((uint64_t)(uint16_t)m_accum[i].w[SLICE_H]) << 48);

				q += (int64_t)(r) << 17;

				m_accum[i].w[SLICE_LL] = (uint16_t)q;
				m_accum[i].w[SLICE_L] = (uint16_t)(q >> 16);
				m_accum[i].w[SLICE_M] = (uint16_t)(q >> 32);
				m_accum[i].w[SLICE_H] = (uint16_t)(q >> 48);

				vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				int32_t r1 = s1 * s2;
				uint32_t r2 = m_accum[i].w[SLICE_L] + ((uint16_t)r1 * 2);
				uint32_t r3 = m_accum[i].w[SLICE_M] + (uint16_t)((r1 >> 16) * 2) + (uint16_t)(r2 >> 16);

				m_accum[i].w[SLICE_L] = (uint16_t)r2;
				m_accum[i].w[SLICE_M] = (uint16_t)r3;
				m_accum[i].w[SLICE_H] += (uint16_t)(r3 >> 16) + (uint16_t)(r1 >> 31);

				if ((int16_t)m_accum[i].w[SLICE_H] < 0)
				{
					vres[i] = 0;
				}
				else
				{
					if (m_accum[i].w[SLICE_H] != 0)
					{
						vres[i] = 0xffff;
					}
					else
					{
						if ((int16_t)m_accum[i].w[SLICE_M] < 0)
						{
							vres[i] = 0xffff;
						}
						else
						{
							vres[i] = m_accum[i].w[SLICE_M];
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

			for (int i = 0; i < 8; i++)
			{
				uint32_t s1 = m_v[VS1REG].w[i];
				uint32_t s2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];
				uint32_t r1 = s1 * s2;
				uint32_t r2 = m_accum[i].w[SLICE_L] + (r1 >> 16);
				uint32_t r3 = m_accum[i].w[SLICE_M] + (r2 >> 16);

				m_accum[i].w[SLICE_L] = (uint16_t)r2;
				m_accum[i].w[SLICE_M] = (uint16_t)r3;
				m_accum[i].w[SLICE_H] += (int16_t)(r3 >> 16);

				vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
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

			for (int i = 0; i < 8; i++)
			{
				uint32_t s1 = m_v[VS1REG].s[i];
				uint32_t s2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];   // not sign-extended
				uint32_t r1 = s1 * s2;
				uint32_t r2 = (uint16_t)m_accum[i].w[SLICE_L] + (uint16_t)(r1);
				uint32_t r3 = (uint16_t)m_accum[i].w[SLICE_M] + (r1 >> 16) + (r2 >> 16);

				m_accum[i].w[SLICE_L] = (uint16_t)r2;
				m_accum[i].w[SLICE_M] = (uint16_t)r3;
				m_accum[i].w[SLICE_H] += (uint16_t)(r3 >> 16);
				if ((int32_t)r1 < 0)
					m_accum[i].w[SLICE_H] -= 1;

				vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].w[i];     // not sign-extended
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				uint64_t q = (uint64_t)m_accum[i].w[SLICE_LL];
				q |= (((uint64_t)m_accum[i].w[SLICE_L]) << 16);
				q |= (((uint64_t)m_accum[i].w[SLICE_M]) << 32);
				q |= (((uint64_t)m_accum[i].w[SLICE_H]) << 48);
				q += (int64_t)(s1*s2) << 16;

				m_accum[i].w[SLICE_LL] = (uint16_t)q;
				m_accum[i].w[SLICE_L] = (uint16_t)(q >> 16);
				m_accum[i].w[SLICE_M] = (uint16_t)(q >> 32);
				m_accum[i].w[SLICE_H] = (uint16_t)(q >> 48);

				vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				int32_t accum = (uint32_t)(uint16_t)m_accum[i].w[SLICE_M];
				accum |= ((uint32_t)((uint16_t)m_accum[i].w[SLICE_H])) << 16;
				accum += s1 * s2;

				m_accum[i].w[SLICE_H] = (uint16_t)(accum >> 16);
				m_accum[i].w[SLICE_M] = (uint16_t)accum;

				vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				int32_t r = s1 + s2 + BIT(m_vcarry, i);

				m_accum[i].w[SLICE_L] = (int16_t)r;

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;
				vres[i] = (int16_t)(r);
			}
			m_vzero = 0;
			m_vcarry = 0;
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

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].s[i];
				int32_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				int32_t r = s1 - s2 - BIT(m_vcarry, i);

				m_accum[i].w[SLICE_L] = (int16_t)r;

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;

				vres[i] = (int16_t)(r);
			}
			m_vzero = 0;
			m_vcarry = 0;
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

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

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

				m_accum[i].w[SLICE_L] = vres[i];
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

			m_vzero = 0;
			m_vcarry = 0;

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].w[i];
				int32_t s2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];
				int32_t r = s1 + s2;

				vres[i] = (int16_t)r;
				m_accum[i].w[SLICE_L] = (int16_t)r;

				if (r & 0xffff0000)
				{
					m_vcarry |= 1 << i;
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

			m_vzero = 0;
			m_vcarry = 0;

			for (int i = 0; i < 8; i++)
			{
				int32_t s1 = m_v[VS1REG].w[i];
				int32_t s2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];
				int32_t r = s1 - s2;

				vres[i] = (int16_t)(r);
				m_accum[i].w[SLICE_L] = (uint16_t)r;

				if ((uint16_t)r != 0)
				{
					m_vzero |= 1 << i;
				}
				if (r & 0xffff0000)
				{
					m_vcarry |= 1 << i;
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
					for (int i = 0; i < 8; i++)
					{
						m_v[VDREG].w[i] = m_accum[i].w[SLICE_H];
					}
					break;
				}
				case 0x09:      // VSAWM
				{
					for (int i = 0; i < 8; i++)
					{
						m_v[VDREG].w[i] = m_accum[i].w[SLICE_M];
					}
					break;
				}
				case 0x0a:      // VSAWL
				{
					for (int i = 0; i < 8; i++)
					{
						m_v[VDREG].w[i] = m_accum[i].w[SLICE_L];
					}
					break;
				}
				default:
					printf("RSP: VSAW: el = %d\n", EL);
					break;
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

			m_vcompare = 0;
			m_vclip2 = 0;

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				if (s1 < s2)
				{
					m_vcompare |= 1 << i;
				}
				else if (s1 == s2)
				{
					if (BIT(m_vzero & m_vcarry, i))
					{
						m_vcompare |= 1 << i;
					}
				}

				if (BIT(m_vcompare, i))
				{
					vres[i] = s1;
				}
				else
				{
					vres[i] = s2;
				}

				m_accum[i].w[SLICE_L] = vres[i];
			}

			m_vzero = 0;
			m_vcarry = 0;
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

			m_vcompare = 0;
			m_vclip2 = 0;

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				if ((s1 == s2) && !BIT(m_vzero, i))
				{
					m_vcompare |= 1 << i;
					vres[i] = s1;
				}
				else
				{
					vres[i] = s2;
				}
				m_accum[i].w[SLICE_L] = vres[i];
			}

			m_vzero = 0;
			m_vcarry = 0;
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

			m_vcompare = 0;
			m_vclip2 = 0;

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				if (s1 != s2 || BIT(m_vzero, i))
				{
					m_vcompare |= 1 << i;
					vres[i] = s1;
				}
				else
				{
					vres[i] = s2;
				}

				m_accum[i].w[SLICE_L] = vres[i];
			}

			m_vzero = 0;
			m_vcarry = 0;
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

			m_vcompare = 0;
			m_vclip2 = 0;

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				if ((s1 == s2 && (!BIT(m_vzero, i) || !BIT(m_vcarry, i))) || s1 > s2)
				{
					m_vcompare |= 1 << i;
					vres[i] = s1;
				}
				else
				{
					vres[i] = s2;
				}

				m_accum[i].w[SLICE_L] = vres[i];
			}

			m_vzero = 0;
			m_vcarry = 0;
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

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				if (BIT(m_vcarry, i)) // vco_lo
				{
					if (BIT(m_vzero, i)) // vco_hi
					{
						if (BIT(m_vcompare, i)) // vcc_lo
						{
							m_accum[i].w[SLICE_L] = -(uint16_t)s2;
						}
						else
						{
							m_accum[i].w[SLICE_L] = s1;
						}
					}
					else
					{
						if (BIT(m_vclip1, i)) // vce
						{
							if (((uint32_t)(uint16_t)(s1) + (uint32_t)(uint16_t)(s2)) > 0x10000)
							{
								m_accum[i].w[SLICE_L] = s1;
								m_vcompare &= ~(1 << i);
							}
							else
							{
								m_accum[i].w[SLICE_L] = -(uint16_t)s2;
								m_vcompare |= 1 << i;
							}
						}
						else
						{
							if (((uint32_t)(uint16_t)(s1) + (uint32_t)(uint16_t)(s2)) != 0)
							{
								m_accum[i].w[SLICE_L] = s1;
								m_vcompare &= ~(1 << i);
							}
							else
							{
								m_accum[i].w[SLICE_L] = -(uint16_t)s2;
								m_vcompare |= 1 << i;
							}
						}
					}
				}
				else
				{
					if (BIT(m_vzero, i)) // vco_hi
					{
						if (BIT(m_vclip2, i)) // vcc_hi
						{
							m_accum[i].w[SLICE_L] = s2;
						}
						else
						{
							m_accum[i].w[SLICE_L] = s1;
						}
					}
					else
					{
						if (((int32_t)(uint16_t)s1 - (int32_t)(uint16_t)s2) >= 0)
						{
							m_accum[i].w[SLICE_L] = s2;
							m_vclip2 |= 1 << i;
						}
						else
						{
							m_accum[i].w[SLICE_L] = s1;
							m_vclip2 &= ~(1 << i);
						}
					}
				}

				vres[i] = m_accum[i].w[SLICE_L];
			}

			m_vzero = 0;
			m_vcarry = 0;
			m_vclip1 = 0;
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

			m_vcarry = 0;
			m_vcompare = 0;
			m_vclip1 = 0;
			m_vzero = 0;
			m_vclip2 = 0;
			uint32_t vce;

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				if ((s1 ^ s2) < 0)
				{
					vce = (s1 + s2 == -1);
					m_vcarry |= 1 << i;
					if (s2 < 0)
					{
						m_vclip2 |= 1 << i;
					}

					if (s1 + s2 <= 0)
					{
						m_vcompare |= 1 << i;
						vres[i] = -((uint16_t)s2);
					}
					else
					{
						vres[i] = s1;
					}

					if (s1 + s2 != 0)
					{
						if (s1 != ~s2)
						{
							m_vzero |= 1 << i;
						}
					}
				}
				else
				{
					vce = 0;
					if (s2 < 0)
					{
						m_vcompare |= 1 << i;
					}
					if (s1 - s2 >= 0)
					{
						m_vclip2 |= 1 << i;
						vres[i] = s2;
					}
					else
					{
						vres[i] = s1;
					}

					if ((s1 - s2) != 0)
					{
						if (s1 != ~s2)
						{
							m_vzero |= 1 << i;
						}
					}
				}

				if (vce != 0)
				{
					m_vclip1 |= 1 << i;
				}

				m_accum[i].w[SLICE_L] = vres[i];
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

			m_vcarry = 0;
			m_vcompare = 0;
			m_vclip1 = 0;
			m_vzero = 0;
			m_vclip2 = 0;

			for (int i = 0; i < 8; i++)
			{
				int16_t s1 = m_v[VS1REG].s[i];
				int16_t s2 = m_v[VS2REG].s[VEC_EL_2(EL, i)];

				if ((int16_t)(s1 ^ s2) < 0)
				{
					if (s2 < 0)
					{
						m_vclip2 |= 1 << i;
					}
					if ((s1 + s2) <= 0)
					{
						m_accum[i].w[SLICE_L] = ~(uint16_t)s2;
						m_vcompare |= 1 << i;
					}
					else
					{
						m_accum[i].w[SLICE_L] = s1;
					}
				}
				else
				{
					if (s2 < 0)
					{
						m_vcompare |= 1 << i;
					}
					if ((s1 - s2) >= 0)
					{
						m_accum[i].w[SLICE_L] = s2;
						m_vclip2 |= 1 << i;
					}
					else
					{
						m_accum[i].w[SLICE_L] = s1;
					}
				}

				vres[i] = m_accum[i].w[SLICE_L];
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

			for (int i = 0; i < 8; i++)
			{
				if (BIT(m_vcompare, i))
				{
					vres[i] = m_v[VS1REG].s[i];
				}
				else
				{
					vres[i] = m_v[VS2REG].s[VEC_EL_2(EL, i)];
				}

				m_accum[i].w[SLICE_L] = vres[i];
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

			for (int i = 0; i < 8; i++)
			{
				vres[i] = m_v[VS1REG].w[i] & m_v[VS2REG].w[VEC_EL_2(EL, i)];
				m_accum[i].w[SLICE_L] = vres[i];
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

			for (int i = 0; i < 8; i++)
			{
				vres[i] = ~(m_v[VS1REG].w[i] & m_v[VS2REG].w[VEC_EL_2(EL, i)]);
				m_accum[i].w[SLICE_L] = vres[i];
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

			for (int i = 0; i < 8; i++)
			{
				vres[i] = m_v[VS1REG].w[i] | m_v[VS2REG].w[VEC_EL_2(EL, i)];
				m_accum[i].w[SLICE_L] = vres[i];
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

			for (int i = 0; i < 8; i++)
			{
				vres[i] = ~(m_v[VS1REG].w[i] | m_v[VS2REG].w[VEC_EL_2(EL, i)]);
				m_accum[i].w[SLICE_L] = vres[i];
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

			for (int i = 0; i < 8; i++)
			{
				vres[i] = m_v[VS1REG].w[i] ^ m_v[VS2REG].w[VEC_EL_2(EL, i)];
				m_accum[i].w[SLICE_L] = vres[i];
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

			for (int i = 0; i < 8; i++)
			{
				vres[i] = ~(m_v[VS1REG].w[i] ^ m_v[VS2REG].w[VEC_EL_2(EL, i)]);
				m_accum[i].w[SLICE_L] = vres[i];
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x2e:      /* V056 (Reserved) */
		case 0x2f:      /* V057 (Reserved) */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
			// ------------------------------------------------------
			//
			// Reserved Opcode
			// Appears to simply store the unsigned 16-bit sum of vector elements into low accumulator slice.
			// Zeroes destination vector.

			for (int i = 0; i < 8; i++)
			{
				vres[i] = 0;
				uint16_t e1 = m_v[VS1REG].w[i];
				uint16_t e2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];
				m_accum[i].w[SLICE_L] = e1 + e2;
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

			int32_t shifter = 0;

			int32_t rec = m_v[VS2REG].s[EL & 7];
			int32_t datainput = (rec < 0) ? (-rec) : rec;
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

			int32_t address = ((datainput << shifter) & 0x7fc00000) >> 22;
			int32_t fetchval = rsp_divtable[address];
			int32_t temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
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

			m_v[VDREG].w[VS1REG & 7] = (uint16_t)(rec & 0xffff);

			for (int i = 0; i < 8; i++)
			{
				m_accum[i].w[SLICE_L] = m_v[VS2REG].w[VEC_EL_2(EL, i)];
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

			int32_t shifter = 0;

			int32_t rec = m_v[VS2REG].s[EL & 7];
			int32_t datainput = rec;

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


			for (int i = 0; i < 32; i++)
			{
				if (datainput & (1 << ((~i) & 0x1f)))
				{
					shifter = i;
					break;
				}
			}

			int32_t address = ((datainput << shifter) & 0x7fc00000) >> 22;
			int32_t fetchval = rsp_divtable[address];
			int32_t temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
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

			m_v[VDREG].w[VS1REG & 7] = (uint16_t)(rec & 0xffff);

			for (int i = 0; i < 8; i++)
			{
				m_accum[i].w[SLICE_L] = m_v[VS2REG].w[VEC_EL_2(EL, i)];
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

			m_reciprocal_high = m_v[VS2REG].w[EL & 7] << 16;
			m_dp_allowed = 1;

			for (int i = 0; i < 8; i++)
			{
				m_accum[i].w[SLICE_L] = m_v[VS2REG].w[VEC_EL_2(EL, i)];
			}

			m_v[VDREG].s[VS1REG & 7] = (int16_t)(m_reciprocal_res >> 16);
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

			m_v[VDREG].w[VS1REG & 7] = m_v[VS2REG].w[VEC_EL_2(EL, VS1REG & 7)];
			for (int i = 0; i < 8; i++)
			{
				m_accum[i].w[SLICE_L] = m_v[VS2REG].w[i];
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

			int32_t shifter = 0;

			int32_t rec = m_v[VS2REG].s[EL & 7];
			int32_t datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (int i = 0; i < 32; i++)
				{
					if (datainput & (1 << (~i & 0x1f)))
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

			int32_t address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			int32_t fetchval = rsp_divtable[address];
			int32_t temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
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

			m_v[VDREG].w[VS1REG & 7] = (uint16_t)(rec & 0xffff);

			for (int i = 0; i < 8; i++)
			{
				m_accum[i].w[SLICE_L] = m_v[VS2REG].w[VEC_EL_2(EL, i)];
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

			int32_t shifter = 0;
			int32_t rec = m_v[VS2REG].s[EL & 7];
			int32_t datainput = rec;

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
				for (int i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))
					{
						shifter = i;
						break;
					}
				}
			}

			int32_t address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			int32_t fetchval = rsp_divtable[address];
			int32_t temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
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

			m_v[VDREG].w[VS1REG & 7] = (uint16_t)(rec & 0xffff);

			for (int i = 0; i < 8; i++)
			{
				m_accum[i].w[SLICE_L] = m_v[VS2REG].w[VEC_EL_2(EL, i)];
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

			m_reciprocal_high = m_v[VS2REG].w[EL & 7] << 16;
			m_dp_allowed = 1;

			for (int i = 0; i < 8; i++)
			{
				m_accum[i].w[SLICE_L] = m_v[VS2REG].w[VEC_EL_2(EL, i)];
			}

			m_v[VDREG].s[VS1REG & 7] = (int16_t)(m_reciprocal_res >> 16);    // store high part
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

		case 0x3b:      /* V073 (Reserved) */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
			// ------------------------------------------------------
			//
			// Reserved Opcode
			// Appears to simply store the unsigned 16-bit sum of vector elements into low accumulator slice.
			// Zeroes destination vector.

			for (int i = 0; i < 8; i++)
			{
				vres[i] = 0;
				uint16_t e1 = m_v[VS1REG].w[i];
				uint16_t e2 = m_v[VS2REG].w[VEC_EL_2(EL, i)];
				m_accum[i].w[SLICE_L] = e1 + e2;
			}
			WRITEBACK_RESULT();
			break;
		}

		case 0x3f:      /* VNULL (Reserved) */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
			// ------------------------------------------------------
			//
			// Reserved Opcode
			// Appears to simply store the unsigned 16-bit sum of vector elements into low accumulator slice.
			// Zeroes destination vector.

			for (int i = 0; i < 8; i++)
			{
				vres[i] = m_v[VS1REG].w[i];
				m_accum[i].w[SLICE_L] = 0;
			}
			WRITEBACK_RESULT();
			break;
		}

		default:    unimplemented_opcode(op); break;
	}
}

void rsp_device::handle_cop2(uint32_t op)
{
	switch ((op >> 21) & 0x1f)
	{
		case 0x00: /* MFC2 */
		{
			// 31 25 20 15 10 6 0
			// ---------------------------------------------------
			// | 010010 | 00000 | TTTTT | DDDDD | IIII | 0000000 |
			// ---------------------------------------------------

			int el = (op >> 7) & 0xf;
			uint16_t b1 = VREG_B(RDREG, (el+0) & 0xf);
			uint16_t b2 = VREG_B(RDREG, (el+1) & 0xf);
			if (RTREG) m_r[RTREG] = (int32_t)(int16_t)((b1 << 8) | (b2));
			break;
		}

		case 0x02: /* CFC2 */
		{
			// 31 25 20 15 10 0
			// ------------------------------------------------
			// | 010010 | 00010 | TTTTT | DDDDD | 00000000000 |
			// ------------------------------------------------

			if (RTREG)
			{
				switch (RDREG)
				{
					case 0:
						m_r[RTREG] = (m_vzero << 8) | m_vcarry;
						if (m_r[RTREG] & 0x8000) m_r[RTREG] |= 0xffff0000;
						break;
					case 1:
						m_r[RTREG] = (m_vclip2 << 8) | m_vcompare;
						if (m_r[RTREG] & 0x8000) m_r[RTREG] |= 0xffff0000;
						break;
					case 2:
						// Anciliary clipping flags
						m_r[RTREG] = m_vclip1;
						break;
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

			int el = (op >> 7) & 0xf;
			W_VREG_B(RDREG, (el+0) & 0xf, (m_r[RTREG] >> 8) & 0xff);
			W_VREG_B(RDREG, (el+1) & 0xf, (m_r[RTREG] >> 0) & 0xff);
			break;
		}

		case 0x06: /* CTC2 */
		{
			// 31 25 20 15 10 0
			// ------------------------------------------------
			// | 010010 | 00110 | TTTTT | DDDDD | 00000000000 |
			// ------------------------------------------------

			switch (RDREG)
			{
				case 0:
					m_vcarry = (uint8_t)m_r[RTREG];
					m_vzero = (uint8_t)(m_r[RTREG] >> 8);
					break;

				case 1:
					m_vcompare = (uint8_t)m_r[RTREG];
					m_vclip2 = (uint8_t)(m_r[RTREG] >> 8);
					break;

				case 2:
					m_vclip1 = (uint8_t)m_r[RTREG];
					break;
			}
			break;
		}

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			handle_vector_ops(op);
			break;

		default:
			unimplemented_opcode(op);
			break;
	}
}

void rsp_device::handle_lwc2(uint32_t op)
{
	int base = (op >> 21) & 0x1f;
	int dest = (op >> 16) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	offset = (offset << 25) >> 25;

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

			uint32_t ea = (base) ? m_r[base] + offset : offset;
			VREG_B(dest, index) = read_dmem_byte(ea);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 2) : (offset * 2);

			for (int i = index; i < index + 2; i++)
			{
				VREG_B(dest, i) = read_dmem_byte(ea);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 4) : (offset * 4);

			for (int i = index; i < index + 4; i++)
			{
				VREG_B(dest, i) = read_dmem_byte(ea);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 8) : (offset * 8);

			for (int i = index; i < index + 8; i++)
			{
				VREG_B(dest, i) = read_dmem_byte(ea);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			int end = index + (16 - (ea & 0xf));
			if (end > 16) end = 16;

			for (int i = index; i < end; i++)
			{
				VREG_B(dest, i) = read_dmem_byte(ea);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			index = 16 - ((ea & 0xf) - index);
			ea &= ~0xf;

			for (int i = index; i < 16; i++)
			{
				VREG_B(dest, i) = read_dmem_byte(ea);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 8) : (offset * 8);

			for (int i = 0; i < 8; i++)
			{
				m_v[dest].w[i] = read_dmem_byte(ea + (((16-index) + i) & 0xf)) << 8;
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

			uint32_t ea = (base) ? m_r[base] + (offset * 8) : (offset * 8);

			for (int i = 0; i < 8; i++)
			{
				m_v[dest].w[i] = read_dmem_byte(ea + (((16-index) + i) & 0xf)) << 7;
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			for (int i = 0; i < 8; i++)
			{
				m_v[dest].w[i] = read_dmem_byte(ea + (((16-index) + (i<<1)) & 0xf)) << 7;
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			// NOTE: Not sure what happens if 16-byte boundary is crossed

			int end = (index >> 1) + 4;

			for (int i = index >> 1; i < end; i++)
			{
				m_v[dest].w[i] = read_dmem_byte(ea) << 7;
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
			// Intended instruction behavior:
			// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15
			//
			// Actual instruction behavior:
			// Loads the full 128-bit vector starting from vector byte index 0.
			//
			// Hardware testing has proven that the vector index is ignored when executing LWV.
			// By contrast, SWV will function as intended when provided an index.

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			for (int i = 0; i < 16; i++)
			{
				VREG_B(dest, i) = read_dmem_byte(ea);
				ea++;
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

			int32_t vs = (op >> 16) & 0x1f;
			int32_t ve = vs + 8;
			if (ve > 32)
				ve = 32;

			if (index & 1)  fatalerror("RSP: LTV: index = %d\n", index);

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);
			ea = ((ea + 8) & ~0xf) + (index & 1);

			for (int32_t i = vs; i < ve; i++)
			{
				int32_t element = ((8 - (index >> 1) + (i-vs)) << 1);
				VREG_B(i, (element & 0xf)) = read_dmem_byte(ea);
				VREG_B(i, ((element + 1) & 0xf)) = read_dmem_byte(ea + 1);

				ea += 2;
			}
			break;
		}

		default:
		{
			unimplemented_opcode(op);
			break;
		}
	}
}


/***************************************************************************
    Vector Store Instructions
***************************************************************************/

void rsp_device::handle_swc2(uint32_t op)
{
	int base = (op >> 21) & 0x1f;
	int dest = (op >> 16) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	offset = (offset << 25) >> 25;

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

			uint32_t ea = (base) ? m_r[base] + offset : offset;
			write_dmem_byte(ea, VREG_B(dest, index));
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

			uint32_t ea = (base) ? m_r[base] + (offset * 2) : (offset * 2);

			for (int i = index; i < index + 2; i++)
			{
				write_dmem_byte(ea, VREG_B(dest, i));
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

			uint32_t ea = (base) ? m_r[base] + (offset * 4) : (offset * 4);

			for (int i = index; i < index + 4; i++)
			{
				write_dmem_byte(ea, VREG_B(dest, i));
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

			uint32_t ea = (base) ? m_r[base] + (offset * 8) : (offset * 8);

			for (int i = index; i < index + 8; i++)
			{
				write_dmem_byte(ea, VREG_B(dest, i));
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);
			int end = index + (16 - (ea & 0xf));

			for (int i = index; i < end; i++)
			{
				write_dmem_byte(ea, VREG_B(dest, i & 0xf));
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			int end = index + (ea & 0xf);
			int o = (16 - (ea & 0xf)) & 0xf;
			ea &= ~0xf;

			for (int i = index; i < end; i++)
			{
				write_dmem_byte(ea, VREG_B(dest, ((i + o) & 0xf)));
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

			uint32_t ea = (base) ? m_r[base] + (offset * 8) : (offset * 8);

			for (int i = index; i < index + 8; i++)
			{
				if ((i & 0xf) < 8)
				{
					write_dmem_byte(ea, VREG_B(dest, ((i & 0xf) << 1)));
				}
				else
				{
					write_dmem_byte(ea, m_v[dest].s[i & 0x7] >> 7);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 8) : (offset * 8);

			for (int i = index; i < index + 8; i++)
			{
				if ((i & 0xf) < 8)
				{
					write_dmem_byte(ea, m_v[dest].s[i & 0x7] >> 7);
				}
				else
				{
					write_dmem_byte(ea, VREG_B(dest, ((i & 0x7) << 1)));
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			for (int i = 0; i < 8; i++)
			{
				uint8_t d = ((VREG_B(dest, ((index + (i << 1) + 0) & 0xf))) << 1) |
							((VREG_B(dest, ((index + (i << 1) + 1) & 0xf))) >> 7);

				write_dmem_byte(ea, d);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			int eaoffset = ea & 0xf;
			ea &= ~0xf;

			int end = (index >> 1) + 4;

			for (int i = index >> 1; i < end; i++)
			{
				write_dmem_byte(ea + (eaoffset & 0xf), m_v[dest].s[i] >> 7);
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

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			int eaoffset = ea & 0xf;
			ea &= ~0xf;

			for (int i = index; i < index + 16; i++)
			{
				write_dmem_byte(ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
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

			int32_t vs = (op >> 16) & 0x1f;
			int32_t ve = vs + 8;
			if (ve > 32)
				ve = 32;

			int32_t element = 8 - (index >> 1);

			uint32_t ea = (base) ? m_r[base] + (offset * 16) : (offset * 16);

			int32_t eaoffset = (ea & 0xf) + (element * 2);
			ea &= ~0xf;

			for (int32_t i = vs; i < ve; i++)
			{
				write_dmem_word(ea + (eaoffset & 0xf), m_v[i].w[element & 0x7]);
				eaoffset += 2;
				element++;
			}
			break;
		}

		default:
			unimplemented_opcode(op);
			break;
	}
}

void rsp_device::update_scalar_op_deduction()
{
	/*if (m_paired_busy)
	{
	    m_scalar_busy = false;
	    m_vector_busy = false;
	    m_paired_busy = false;
	    m_ideduct = 1;
	}
	else if (m_vector_busy)
	{
	    m_scalar_busy = true;
	    m_paired_busy = true;
	    m_ideduct = 0;
	}
	else if (m_scalar_busy)
	{
	    m_ideduct = 1;
	}
	else
	{
	    m_scalar_busy = true;
	    m_ideduct = 0;
	}*/
}

void rsp_device::update_vector_op_deduction()
{
	/*if (m_paired_busy)
	{
	    m_scalar_busy = false;
	    m_vector_busy = false;
	    m_paired_busy = false;
	    m_ideduct = 1;
	}
	else if (m_scalar_busy)
	{
	    m_vector_busy = true;
	    m_paired_busy = true;
	    m_ideduct = 0;
	}
	else if (m_vector_busy)
	{
	    m_ideduct = 1;
	}
	else
	{
	    m_vector_busy = true;
	    m_ideduct = 0;
	}*/
}

void rsp_device::execute_run()
{
	if (m_sr & (RSP_STATUS_HALT | RSP_STATUS_BROKE))
	{
		m_ideduct = 0;
		m_scalar_busy = false;
		m_vector_busy = false;
		m_paired_busy = false;
		m_icount = std::min(m_icount, 0);
	}

	while (m_icount > 0)
	{
		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

		uint32_t op = ROPCODE(m_pc);
		if (m_nextpc != 0xffff)
		{
			m_pc = m_nextpc;
			m_nextpc = 0xffff;
		}
		else
		{
			m_pc += 4;
		}

		switch (op >> 26)
		{
			case 0x00:  /* SPECIAL */
			{
				update_scalar_op_deduction();
				switch (op & 0x3f)
				{
					case 0x00:  /* SLL */       if (RDREG) m_r[RDREG] = m_r[RTREG] << SHIFT; break;
					case 0x02:  /* SRL */       if (RDREG) m_r[RDREG] = m_r[RTREG] >> SHIFT; break;
					case 0x03:  /* SRA */       if (RDREG) m_r[RDREG] = (int32_t)m_r[RTREG] >> SHIFT; break;
					case 0x04:  /* SLLV */      if (RDREG) m_r[RDREG] = m_r[RTREG] << (m_r[RSREG] & 0x1f); break;
					case 0x06:  /* SRLV */      if (RDREG) m_r[RDREG] = m_r[RTREG] >> (m_r[RSREG] & 0x1f); break;
					case 0x07:  /* SRAV */      if (RDREG) m_r[RDREG] = (int32_t)m_r[RTREG] >> (m_r[RSREG] & 0x1f); break;
					case 0x08:  /* JR */        JUMP_PC(m_r[RSREG]); break;
					case 0x09:  /* JALR */      JUMP_PC_L(m_r[RSREG], RDREG); break;
					case 0x0d:  /* BREAK */
					{
						m_ideduct = 1;
						m_scalar_busy = false;
						m_vector_busy = false;
						m_paired_busy = false;
						m_sp_set_status_func(0, 0x3, 0xffffffff);
						m_icount = std::min(m_icount, 1);
						break;
					}
					case 0x20:  /* ADD */       if (RDREG) m_r[RDREG] = (int32_t)(m_r[RSREG] + m_r[RTREG]); break;
					case 0x21:  /* ADDU */      if (RDREG) m_r[RDREG] = (int32_t)(m_r[RSREG] + m_r[RTREG]); break;
					case 0x22:  /* SUB */       if (RDREG) m_r[RDREG] = (int32_t)(m_r[RSREG] - m_r[RTREG]); break;
					case 0x23:  /* SUBU */      if (RDREG) m_r[RDREG] = (int32_t)(m_r[RSREG] - m_r[RTREG]); break;
					case 0x24:  /* AND */       if (RDREG) m_r[RDREG] = m_r[RSREG] & m_r[RTREG]; break;
					case 0x25:  /* OR */        if (RDREG) m_r[RDREG] = m_r[RSREG] | m_r[RTREG]; break;
					case 0x26:  /* XOR */       if (RDREG) m_r[RDREG] = m_r[RSREG] ^ m_r[RTREG]; break;
					case 0x27:  /* NOR */       if (RDREG) m_r[RDREG] = ~(m_r[RSREG] | m_r[RTREG]); break;
					case 0x2a:  /* SLT */       if (RDREG) m_r[RDREG] = (int32_t)m_r[RSREG] < (int32_t)m_r[RTREG]; break;
					case 0x2b:  /* SLTU */      if (RDREG) m_r[RDREG] = m_r[RSREG] < m_r[RTREG]; break;
					default:    unimplemented_opcode(op); break;
				}
				break;
			}

			case 0x01:  /* REGIMM */
			{
				update_scalar_op_deduction();
				switch (RTREG)
				{
					case 0x00:  /* BLTZ */      if ((int32_t)m_r[RSREG] < 0) JUMP_REL(SIMM16); break;
					case 0x01:  /* BGEZ */      if ((int32_t)m_r[RSREG] >= 0) JUMP_REL(SIMM16); break;
					case 0x10:  /* BLTZAL */    if ((int32_t)m_r[RSREG] < 0) JUMP_REL_L(SIMM16, 31); break;
					case 0x11:  /* BGEZAL */    if ((int32_t)m_r[RSREG] >= 0) JUMP_REL_L(SIMM16, 31); break;
					default:    unimplemented_opcode(op); break;
				}
				break;
			}

			case 0x02:  /* J */         update_scalar_op_deduction(); JUMP_ABS(UIMM26); break;
			case 0x03:  /* JAL */       update_scalar_op_deduction(); JUMP_ABS_L(UIMM26, 31); break;
			case 0x04:  /* BEQ */       update_scalar_op_deduction(); if (m_r[RSREG] == m_r[RTREG]) JUMP_REL(SIMM16); break;
			case 0x05:  /* BNE */       update_scalar_op_deduction(); if (m_r[RSREG] != m_r[RTREG]) JUMP_REL(SIMM16); break;
			case 0x06:  /* BLEZ */      update_scalar_op_deduction(); if ((int32_t)m_r[RSREG] <= 0) JUMP_REL(SIMM16); break;
			case 0x07:  /* BGTZ */      update_scalar_op_deduction(); if ((int32_t)m_r[RSREG] > 0) JUMP_REL(SIMM16); break;
			case 0x08:  /* ADDI */      update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = (int32_t)m_r[RSREG] + SIMM16; break;
			case 0x09:  /* ADDIU */     update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = (int32_t)m_r[RSREG] + SIMM16; break;
			case 0x0a:  /* SLTI */      update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = (int32_t)m_r[RSREG] < (int32_t)SIMM16; break;
			case 0x0b:  /* SLTIU */     update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = m_r[RSREG] < UIMM16; break;
			case 0x0c:  /* ANDI */      update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = m_r[RSREG] & UIMM16; break;
			case 0x0d:  /* ORI */       update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = m_r[RSREG] | UIMM16; break;
			case 0x0e:  /* XORI */      update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = m_r[RSREG] ^ UIMM16; break;
			case 0x0f:  /* LUI */       update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = UIMM16 << 16; break;

			case 0x10:  /* COP0 */
			{
				update_scalar_op_deduction();
				switch ((op >> 21) & 0x1f)
				{
					case 0x00:  /* MFC0 */      if (RTREG) m_r[RTREG] = get_cop0_reg(RDREG); break;
					case 0x04:  /* MTC0 */      set_cop0_reg(RDREG, m_r[RTREG]); break;
					default:    unimplemented_opcode(op); break;
				}
				break;
			}

			case 0x12:  /* COP2 */
			{
				update_vector_op_deduction();
				handle_cop2(op);
				break;
			}

			case 0x20:  /* LB */        update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = (int32_t)(int8_t)read_dmem_byte(m_r[RSREG] + SIMM16); break;
			case 0x21:  /* LH */        update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = (int32_t)(int16_t)read_dmem_word(m_r[RSREG] + SIMM16); break;
			case 0x23:  /* LW */        update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = read_dmem_dword(m_r[RSREG] + SIMM16); break;
			case 0x24:  /* LBU */       update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = read_dmem_byte(m_r[RSREG] + SIMM16); break;
			case 0x25:  /* LHU */       update_scalar_op_deduction(); if (RTREG) m_r[RTREG] = read_dmem_word(m_r[RSREG] + SIMM16); break;
			case 0x28:  /* SB */        update_scalar_op_deduction(); write_dmem_byte(m_r[RSREG] + SIMM16, m_r[RTREG]); break;
			case 0x29:  /* SH */        update_scalar_op_deduction(); write_dmem_word(m_r[RSREG] + SIMM16, m_r[RTREG]); break;
			case 0x2b:  /* SW */        update_scalar_op_deduction(); write_dmem_dword(m_r[RSREG] + SIMM16, m_r[RTREG]); break;
			case 0x32:  /* LWC2 */      update_scalar_op_deduction(); handle_lwc2(op); break;
			case 0x3a:  /* SWC2 */      update_scalar_op_deduction(); handle_swc2(op); break;

			default:
			{
				unimplemented_opcode(op);
				break;
			}
		}

		if (LOG_INSTRUCTION_EXECUTION)
		{
			static uint32_t prev_regs[32];
			static VECTOR_REG prev_vecs[32];

			rsp_disassembler rspd;
			std::ostringstream string;
			rspd.dasm_one(string, m_ppc, op);

			fprintf(m_exec_output, "%08X: %s", m_ppc, string.str().c_str());

			int l = string.str().size();
			if (l < 36)
			{
				for (int i = l; i < 36; i++)
				{
					fprintf(m_exec_output, " ");
				}
			}

			fprintf(m_exec_output, "| ");

			for (int i = 0; i < 32; i++)
			{
				if (m_r[i] != prev_regs[i])
				{
					fprintf(m_exec_output, "R%d: %08X ", i, m_r[i]);
				}
				prev_regs[i] = m_r[i];
			}

			for (int i = 0; i < 32; i++)
			{
				if (m_v[i].d[0] != prev_vecs[i].d[0] || m_v[i].d[1] != prev_vecs[i].d[1])
				{
					fprintf(m_exec_output, "V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X ", i,
					m_v[i].w[0], m_v[i].w[1], m_v[i].w[2], m_v[i].w[3], m_v[i].w[4], m_v[i].w[5], m_v[i].w[6], m_v[i].w[7]);
				}
				prev_vecs[i].d[0] = m_v[i].d[0];
				prev_vecs[i].d[1] = m_v[i].d[1];
			}

			fprintf(m_exec_output, "\n");

		}

		//m_icount -= m_ideduct;
		--m_icount;

		if (m_sr & RSP_STATUS_SSTEP)
		{
			if (m_step_count)
			{
				m_step_count--;
			}
			else
			{
				m_sr |= RSP_STATUS_BROKE;
			}
		}

		if (m_sr & (RSP_STATUS_HALT | RSP_STATUS_BROKE))
		{
			m_ideduct = 0;
			m_scalar_busy = false;
			m_vector_busy = false;
			m_paired_busy = false;
			m_icount = std::min(m_icount, 0);
		}
	}
}
