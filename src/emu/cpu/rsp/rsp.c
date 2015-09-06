// license:BSD-3-Clause
// copyright-holders:Ville Linde, Ryan Holtz
/*
    Nintendo/SGI Reality Signal Processor (RSP) emulator

    Written by Ville Linde
*/

#include "emu.h"
#include "debugger.h"
#include "rsp.h"
#include "rspfe.h"
#include "rspcp2.h"
#include "rspcp2d.h"


const device_type RSP = &device_creator<rsp_device>;


#define LOG_INSTRUCTION_EXECUTION       0
#define SAVE_DISASM                     0
#define SAVE_DMEM                       0
#define RSP_TEST_SYNC                   0

#define PRINT_VECREG(x)     osd_printf_debug("V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X\n", (x), \
							(UINT16)VREG_S((x),0), (UINT16)VREG_S((x),1), \
							(UINT16)VREG_S((x),2), (UINT16)VREG_S((x),3), \
							(UINT16)VREG_S((x),4), (UINT16)VREG_S((x),5), \
							(UINT16)VREG_S((x),6), (UINT16)VREG_S((x),7))

#define PRINT_ACCUM(x)     osd_printf_debug("A%d: %08X|%08X\n", (x), \
							(UINT32)( ( ACCUM(x) >> 32 ) & 0x00000000ffffffff ),    \
							(UINT32)(   ACCUM(x)         & 0x00000000ffffffff ))

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);


#define SIMM16      ((INT32)(INT16)(op))
#define UIMM16      ((UINT16)(op))
#define UIMM26      (op & 0x03ffffff)

#define RSVAL           (m_rsp_state->r[RSREG])
#define RTVAL           (m_rsp_state->r[RTREG])
#define RDVAL           (m_rsp_state->r[RDREG])

#define JUMP_ABS(addr)          { m_nextpc = 0x04001000 | (((addr) << 2) & 0xfff); }
#define JUMP_ABS_L(addr,l)      { m_nextpc = 0x04001000 | (((addr) << 2) & 0xfff); m_rsp_state->r[l] = m_rsp_state->pc + 4; }
#define JUMP_REL(offset)        { m_nextpc = 0x04001000 | ((m_rsp_state->pc + ((offset) << 2)) & 0xfff); }
#define JUMP_REL_L(offset,l)    { m_nextpc = 0x04001000 | ((m_rsp_state->pc + ((offset) << 2)) & 0xfff); m_rsp_state->r[l] = m_rsp_state->pc + 4; }
#define JUMP_PC(addr)           { m_nextpc = 0x04001000 | ((addr) & 0xfff); }
#define JUMP_PC_L(addr,l)       { m_nextpc = 0x04001000 | ((addr) & 0xfff); m_rsp_state->r[l] = m_rsp_state->pc + 4; }
#define LINK(l)                 { m_rsp_state->r[l] = m_rsp_state->pc + 4; }

#define CARRY_FLAG(x)           (m_vflag[CARRY][x & 7] != 0 ? 0xffff : 0)
#define COMPARE_FLAG(x)         (m_vflag[COMPARE][x & 7] != 0 ? 0xffff : 0)
#define CLIP1_FLAG(x)           (m_vflag[CLIP1][x & 7] != 0 ? 0xffff : 0)
#define ZERO_FLAG(x)            (m_vflag[ZERO][x & 7] != 0 ? 0xffff : 0)
#define CLIP2_FLAG(x)           (m_vflag[CLIP2][x & 7] != 0 ? 0xffff : 0)

#define CLEAR_CARRY_FLAGS()     { memset(m_vflag[CARRY], 0, 16); }
#define CLEAR_COMPARE_FLAGS()   { memset(m_vflag[COMPARE], 0, 16); }
#define CLEAR_CLIP1_FLAGS()     { memset(m_vflag[CLIP1], 0, 16); }
#define CLEAR_ZERO_FLAGS()      { memset(m_vflag[ZERO], 0, 16); }
#define CLEAR_CLIP2_FLAGS()     { memset(m_vflag[CLIP2], 0, 16); }

#define SET_CARRY_FLAG(x)       { m_vflag[CARRY][x & 7] = 0xffff; }
#define SET_COMPARE_FLAG(x)     { m_vflag[COMPARE][x & 7] = 0xffff; }
#define SET_CLIP1_FLAG(x)       { m_vflag[CLIP1][x & 7] = 0xffff; }
#define SET_ZERO_FLAG(x)        { m_vflag[ZERO][x & 7] = 0xffff; }
#define SET_CLIP2_FLAG(x)       { m_vflag[CLIP2][x & 7] = 0xffff; }

#define CLEAR_CARRY_FLAG(x)     { m_vflag[CARRY][x & 7] = 0; }
#define CLEAR_COMPARE_FLAG(x)   { m_vflag[COMPARE][x & 7] = 0; }
#define CLEAR_CLIP1_FLAG(x)     { m_vflag[CLIP1][x & 7] = 0; }
#define CLEAR_ZERO_FLAG(x)      { m_vflag[ZERO][x & 7] = 0; }
#define CLEAR_CLIP2_FLAG(x)     { m_vflag[CLIP2][x & 7] = 0; }

#define ROPCODE(pc)     m_program->read_dword(pc)


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


rsp_device::rsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, RSP, "RSP", tag, owner, clock, "rsp", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32)
	, m_cache(CACHE_SIZE + sizeof(internal_rsp_state))
	, m_drcuml(NULL)
//  , m_drcuml(*this, m_cache, 0, 8, 32, 2)
	, m_drcfe(NULL)
	, m_drcoptions(0)
	, m_cache_dirty(TRUE)
	, m_numcycles(0)
	, m_format(NULL)
	, m_arg2(0)
	, m_arg3(0)
	, m_entry(NULL)
	, m_nocode(NULL)
	, m_out_of_cycles(NULL)
	, m_read8(NULL)
	, m_write8(NULL)
	, m_read16(NULL)
	, m_write16(NULL)
	, m_read32(NULL)
	, m_write32(NULL)
	, m_rsp_state(NULL)
	, m_exec_output(NULL)
	, m_sr(0)
	, m_step_count(0)
	, m_ppc(0)
	, m_nextpc(0)
	, m_dmem32(NULL)
	, m_dmem16(NULL)
	, m_dmem8(NULL)
	, m_imem32(NULL)
	, m_imem16(NULL)
	, m_imem8(NULL)
	, m_debugger_temp(0)
	, m_dp_reg_r_func(*this)
	, m_dp_reg_w_func(*this)
	, m_sp_reg_r_func(*this)
	, m_sp_reg_w_func(*this)
	, m_sp_set_status_func(*this)
{
}

offs_t rsp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( rsp );
	return CPU_DISASSEMBLE_NAME( rsp )(this, buffer, pc, oprom, opram, options);
}

void rsp_device::rsp_add_imem(UINT32 *base)
{
	m_imem32 = base;
	m_imem16 = (UINT16*)base;
	m_imem8 = (UINT8*)base;
}

void rsp_device::rsp_add_dmem(UINT32 *base)
{
	m_dmem32 = base;
	m_dmem16 = (UINT16*)base;
	m_dmem8 = (UINT8*)base;
}

UINT8 rsp_device::DM_READ8(UINT32 address)
{
	UINT8 ret = m_dmem8[BYTE4_XOR_BE(address & 0xfff)];
	//printf("R8:%08x=%02x\n", address, ret);
	return ret;
}

UINT16 rsp_device::DM_READ16(UINT32 address)
{
	UINT16 ret;
	address &= 0xfff;
	ret = m_dmem8[BYTE4_XOR_BE(address)] << 8;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 1)];
	//printf("R16:%08x=%04x\n", address, ret);
	return ret;
}

UINT32 rsp_device::DM_READ32(UINT32 address)
{
	UINT32 ret;
	address &= 0xfff;
	ret = m_dmem8[BYTE4_XOR_BE(address)] << 24;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 1)] << 16;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 2)] << 8;
	ret |= m_dmem8[BYTE4_XOR_BE(address + 3)];
	//printf("R32:%08x=%08x\n", address, ret);
	return ret;
}

void rsp_device::DM_WRITE8(UINT32 address, UINT8 data)
{
	address &= 0xfff;
	m_dmem8[BYTE4_XOR_BE(address)] = data;
	//printf("W8:%08x=%02x\n", address, data);
}

void rsp_device::DM_WRITE16(UINT32 address, UINT16 data)
{
	address &= 0xfff;
	m_dmem8[BYTE4_XOR_BE(address)] = data >> 8;
	m_dmem8[BYTE4_XOR_BE(address + 1)] = data & 0xff;
	//printf("W16:%08x=%04x\n", address, data);
}

void rsp_device::DM_WRITE32(UINT32 address, UINT32 data)
{
	address &= 0xfff;
	m_dmem8[BYTE4_XOR_BE(address)] = data >> 24;
	m_dmem8[BYTE4_XOR_BE(address + 1)] = (data >> 16) & 0xff;
	m_dmem8[BYTE4_XOR_BE(address + 2)] = (data >> 8) & 0xff;
	m_dmem8[BYTE4_XOR_BE(address + 3)] = data & 0xff;
	//printf("W32:%08x=%08x\n", address, data);
}

UINT8 rsp_device::READ8(UINT32 address)
{
	UINT8 ret;
	address &= 0xfff;
	ret = m_program->read_byte(address);
	//printf("R8:%08x=%02x\n", address, ret);
	return ret;
}

UINT16 rsp_device::READ16(UINT32 address)
{
	UINT16 ret;
	address &= 0xfff;

	ret = (m_program->read_byte(address) << 8) | (m_program->read_byte(address + 1) & 0xff);

	//printf("R16:%08x=%04x\n", address, ret);
	return ret;
}

UINT32 rsp_device::READ32(UINT32 address)
{
	UINT32 ret;
	address &= 0xfff;

	ret =   (m_program->read_byte(address) << 24) |
			(m_program->read_byte(address + 1) << 16) |
			(m_program->read_byte(address + 2) << 8) |
			(m_program->read_byte(address + 3) << 0);

	//printf("R32:%08x=%08x\n", address, ret);
	return ret;
}

void rsp_device::WRITE8(UINT32 address, UINT8 data)
{
	address &= 0xfff;
	m_program->write_byte(address, data);
	//printf("W8:%08x=%02x\n", address, data);
}

void rsp_device::WRITE16(UINT32 address, UINT16 data)
{
	address &= 0xfff;

	m_program->write_byte(address, data >> 8);
	m_program->write_byte(address + 1, data & 0xff);
	//printf("W16:%08x=%04x\n", address, data);
}

void rsp_device::WRITE32(UINT32 address, UINT32 data)
{
	address &= 0xfff;

	m_program->write_byte(address, data >> 24);
	m_program->write_byte(address + 1, (data >> 16) & 0xff);
	m_program->write_byte(address + 2, (data >> 8) & 0xff);
	m_program->write_byte(address + 3, data & 0xff);
	//printf("W32:%08x=%08x\n", address, data);
}

/*****************************************************************************/

UINT32 rsp_device::get_cop0_reg(int reg)
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

void rsp_device::set_cop0_reg(int reg, UINT32 data)
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

void rsp_device::unimplemented_opcode(UINT32 op)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		char string[200];
		rsp_dasm_one(string, m_ppc, op);
		osd_printf_debug("%08X: %s\n", m_ppc, string);
	}

#if SAVE_DISASM
	{
		char string[200];
		int i;
		FILE *dasm;
		dasm = fopen("rsp_disasm.txt", "wt");

		for (i=0; i < 0x1000; i+=4)
		{
			UINT32 opcode = ROPCODE(0x04001000 + i);
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
			fputc(READ8(rsp, 0x04000000 + i), dmem);
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
	m_isdrc = (mconfig().options().drc() && !mconfig().m_force_no_drc) ? true : false;
	m_rsp_state = (internal_rsp_state *)m_cache.alloc_near(sizeof(internal_rsp_state));

	if (LOG_INSTRUCTION_EXECUTION)
		m_exec_output = fopen("rsp_execute.txt", "wt");

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	resolve_cb();

	if (m_isdrc)
	{
		m_cop2 = auto_alloc(machine(), rsp_cop2_drc(*this, machine()));
	}
	else
	{
		m_cop2 = auto_alloc(machine(), rsp_cop2(*this, machine()));
	}
	m_cop2->init();
	m_cop2->start();

	// RSP registers should power on to a random state
	for(int regIdx = 0; regIdx < 32; regIdx++ )
	{
		m_rsp_state->r[regIdx] = 0;
	}

	m_sr = RSP_STATUS_HALT;
	m_step_count = 0;

	/* initialize the UML generator */
	UINT32 drc_flags = 0;
	m_drcuml = auto_alloc(machine(), drcuml_state(*this, m_cache, drc_flags, 8, 32, 2));

	/* add symbols for our stuff */
	m_drcuml->symbol_add(&m_rsp_state->pc, sizeof(m_rsp_state->pc), "pc");
	m_drcuml->symbol_add(&m_rsp_state->icount, sizeof(m_rsp_state->icount), "icount");
	for (int regnum = 0; regnum < 32; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		m_drcuml->symbol_add(&m_rsp_state->r[regnum], sizeof(m_rsp_state->r[regnum]), buf);
	}
	m_drcuml->symbol_add(&m_rsp_state->arg0, sizeof(m_rsp_state->arg0), "arg0");
	m_drcuml->symbol_add(&m_rsp_state->arg1, sizeof(m_rsp_state->arg1), "arg1");
	m_drcuml->symbol_add(&m_arg2, sizeof(m_arg2), "arg2");
	m_drcuml->symbol_add(&m_arg3, sizeof(m_arg3), "arg3");
	m_drcuml->symbol_add(&m_numcycles, sizeof(m_numcycles), "numcycles");

	/* initialize the front-end helper */
	m_drcfe = auto_alloc(machine(), rsp_frontend(*this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE));

	/* compute the register parameters */
	for (int regnum = 0; regnum < 32; regnum++)
	{
		m_regmap[regnum] = (regnum == 0) ? uml::parameter(0) : uml::parameter::make_memory(&m_rsp_state->r[regnum]);
	}

	/* mark the cache dirty so it is updated on next execute */
	m_cache_dirty = TRUE;

	state_add( RSP_PC,      "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add( RSP_R0,      "R0", m_rsp_state->r[0]).formatstr("%08X");
	state_add( RSP_R1,      "R1", m_rsp_state->r[1]).formatstr("%08X");
	state_add( RSP_R2,      "R2", m_rsp_state->r[2]).formatstr("%08X");
	state_add( RSP_R3,      "R3", m_rsp_state->r[3]).formatstr("%08X");
	state_add( RSP_R4,      "R4", m_rsp_state->r[4]).formatstr("%08X");
	state_add( RSP_R5,      "R5", m_rsp_state->r[5]).formatstr("%08X");
	state_add( RSP_R6,      "R6", m_rsp_state->r[6]).formatstr("%08X");
	state_add( RSP_R7,      "R7", m_rsp_state->r[7]).formatstr("%08X");
	state_add( RSP_R8,      "R8", m_rsp_state->r[8]).formatstr("%08X");
	state_add( RSP_R9,      "R9", m_rsp_state->r[9]).formatstr("%08X");
	state_add( RSP_R10,     "R10", m_rsp_state->r[10]).formatstr("%08X");
	state_add( RSP_R11,     "R11", m_rsp_state->r[11]).formatstr("%08X");
	state_add( RSP_R12,     "R12", m_rsp_state->r[12]).formatstr("%08X");
	state_add( RSP_R13,     "R13", m_rsp_state->r[13]).formatstr("%08X");
	state_add( RSP_R14,     "R14", m_rsp_state->r[14]).formatstr("%08X");
	state_add( RSP_R15,     "R15", m_rsp_state->r[15]).formatstr("%08X");
	state_add( RSP_R16,     "R16", m_rsp_state->r[16]).formatstr("%08X");
	state_add( RSP_R17,     "R17", m_rsp_state->r[17]).formatstr("%08X");
	state_add( RSP_R18,     "R18", m_rsp_state->r[18]).formatstr("%08X");
	state_add( RSP_R19,     "R19", m_rsp_state->r[19]).formatstr("%08X");
	state_add( RSP_R20,     "R20", m_rsp_state->r[20]).formatstr("%08X");
	state_add( RSP_R21,     "R21", m_rsp_state->r[21]).formatstr("%08X");
	state_add( RSP_R22,     "R22", m_rsp_state->r[22]).formatstr("%08X");
	state_add( RSP_R23,     "R23", m_rsp_state->r[23]).formatstr("%08X");
	state_add( RSP_R24,     "R24", m_rsp_state->r[24]).formatstr("%08X");
	state_add( RSP_R25,     "R25", m_rsp_state->r[25]).formatstr("%08X");
	state_add( RSP_R26,     "R26", m_rsp_state->r[26]).formatstr("%08X");
	state_add( RSP_R27,     "R27", m_rsp_state->r[27]).formatstr("%08X");
	state_add( RSP_R28,     "R28", m_rsp_state->r[28]).formatstr("%08X");
	state_add( RSP_R29,     "R29", m_rsp_state->r[29]).formatstr("%08X");
	state_add( RSP_R30,     "R30", m_rsp_state->r[30]).formatstr("%08X");
	state_add( RSP_R31,     "R31", m_rsp_state->r[31]).formatstr("%08X");
	state_add( RSP_SR,      "SR",  m_sr).formatstr("%08X");
	state_add( RSP_NEXTPC,  "NPC", m_debugger_temp).callimport().callexport().formatstr("%08X");
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

	state_add( STATE_GENPC, "GENPC", m_debugger_temp).callimport().callexport().noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%1s").noshow();
	state_add( STATE_GENSP, "GENSP", m_rsp_state->r[31]).noshow();
	state_add( STATE_GENPCBASE, "GENPCBASE", m_debugger_temp).callimport().callexport().noshow();

	m_icountptr = &m_rsp_state->icount;
}

void rsp_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case RSP_PC:
			m_rsp_state->pc = m_debugger_temp;
			break;

		case STATE_GENPCBASE:
			m_ppc = m_debugger_temp;
			break;

		case RSP_NEXTPC:
			m_nextpc = m_debugger_temp;
			break;
	}
}


void rsp_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
		case RSP_PC:
			m_debugger_temp = m_rsp_state->pc | 0x04000000;
			break;

		case STATE_GENPCBASE:
			m_debugger_temp = m_ppc | 0x04000000;
			break;

		case RSP_NEXTPC:
			m_debugger_temp = m_nextpc | 0x04000000;
			break;
	}
}

void rsp_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	const int index = entry.index();
	if (index >= RSP_V0 && index <= RSP_V31)
	{
		m_cop2->state_string_export(index, str);
	}
	else if (index == STATE_GENFLAGS)
	{
		strprintf(str, "%s", "");
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
			UINT32 opcode = ROPCODE(0x04001000 + i);
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
#if 0
		dmem = fopen("rsp_dmem.txt", "wt");

		for (i=0; i < 0x1000; i+=4)
		{
			fprintf(dmem, "%08X: %08X\n", 0x04000000 + i, READ32(0x04000000 + i));
		}
		fclose(dmem);
#endif
		dmem = fopen("rsp_dmem.bin", "wb");

		for (i=0; i < 0x1000; i++)
		{
			fputc(READ8(0x04000000 + i), dmem);
		}
		fclose(dmem);
	}
#endif

	if (m_exec_output)
		fclose(m_exec_output);
	m_exec_output = NULL;

	/* clean up the DRC */
	if (m_drcuml)
	{
		auto_free(machine(), m_drcuml);
	}
	if (m_drcfe)
	{
		auto_free(machine(), m_drcfe);
	}

	if (m_cop2)
	{
		auto_free(machine(), m_cop2);
	}
}

void rsp_device::device_reset()
{
	m_nextpc = ~0;
}

void rsp_device::execute_run()
{
	if (m_isdrc)
	{
		execute_run_drc();
		return;
	}

	m_rsp_state->pc = 0x4001000 | (m_rsp_state->pc & 0xfff);

	if( m_sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
	{
		m_rsp_state->icount = MIN(m_rsp_state->icount, 0);
	}

	while (m_rsp_state->icount > 0)
	{
		m_ppc = m_rsp_state->pc;
		debugger_instruction_hook(this, m_rsp_state->pc);

		UINT32 op = ROPCODE(m_rsp_state->pc);
		if (m_nextpc != ~0)
		{
			m_rsp_state->pc = m_nextpc;
			m_nextpc = ~0;
		}
		else
		{
			m_rsp_state->pc += 4;
		}

		switch (op >> 26)
		{
			case 0x00:  /* SPECIAL */
			{
				switch (op & 0x3f)
				{
					case 0x00:  /* SLL */       if (RDREG) RDVAL = (UINT32)RTVAL << SHIFT; break;
					case 0x02:  /* SRL */       if (RDREG) RDVAL = (UINT32)RTVAL >> SHIFT; break;
					case 0x03:  /* SRA */       if (RDREG) RDVAL = (INT32)RTVAL >> SHIFT; break;
					case 0x04:  /* SLLV */      if (RDREG) RDVAL = (UINT32)RTVAL << (RSVAL & 0x1f); break;
					case 0x06:  /* SRLV */      if (RDREG) RDVAL = (UINT32)RTVAL >> (RSVAL & 0x1f); break;
					case 0x07:  /* SRAV */      if (RDREG) RDVAL = (INT32)RTVAL >> (RSVAL & 0x1f); break;
					case 0x08:  /* JR */        JUMP_PC(RSVAL); break;
					case 0x09:  /* JALR */      JUMP_PC_L(RSVAL, RDREG); break;
					case 0x0d:  /* BREAK */
					{
						m_sp_set_status_func(0, 0x3, 0xffffffff);
						m_rsp_state->icount = MIN(m_rsp_state->icount, 1);
						break;
					}
					case 0x20:  /* ADD */       if (RDREG) RDVAL = (INT32)(RSVAL + RTVAL); break;
					case 0x21:  /* ADDU */      if (RDREG) RDVAL = (INT32)(RSVAL + RTVAL); break;
					case 0x22:  /* SUB */       if (RDREG) RDVAL = (INT32)(RSVAL - RTVAL); break;
					case 0x23:  /* SUBU */      if (RDREG) RDVAL = (INT32)(RSVAL - RTVAL); break;
					case 0x24:  /* AND */       if (RDREG) RDVAL = RSVAL & RTVAL; break;
					case 0x25:  /* OR */        if (RDREG) RDVAL = RSVAL | RTVAL; break;
					case 0x26:  /* XOR */       if (RDREG) RDVAL = RSVAL ^ RTVAL; break;
					case 0x27:  /* NOR */       if (RDREG) RDVAL = ~(RSVAL | RTVAL); break;
					case 0x2a:  /* SLT */       if (RDREG) RDVAL = (INT32)RSVAL < (INT32)RTVAL; break;
					case 0x2b:  /* SLTU */      if (RDREG) RDVAL = (UINT32)RSVAL < (UINT32)RTVAL; break;
					default:    unimplemented_opcode(op); break;
				}
				break;
			}

			case 0x01:  /* REGIMM */
			{
				switch (RTREG)
				{
					case 0x00:  /* BLTZ */      if ((INT32)(RSVAL) < 0) JUMP_REL(SIMM16); break;
					case 0x01:  /* BGEZ */      if ((INT32)(RSVAL) >= 0) JUMP_REL(SIMM16); break;
					case 0x10:  /* BLTZAL */    if ((INT32)(RSVAL) < 0) JUMP_REL_L(SIMM16, 31); break;
					case 0x11:  /* BGEZAL */    if ((INT32)(RSVAL) >= 0) JUMP_REL_L(SIMM16, 31); break;
					default:    unimplemented_opcode(op); break;
				}
				break;
			}

			case 0x02:  /* J */         JUMP_ABS(UIMM26); break;
			case 0x03:  /* JAL */       JUMP_ABS_L(UIMM26, 31); break;
			case 0x04:  /* BEQ */       if (RSVAL == RTVAL) JUMP_REL(SIMM16); break;
			case 0x05:  /* BNE */       if (RSVAL != RTVAL) JUMP_REL(SIMM16); break;
			case 0x06:  /* BLEZ */      if ((INT32)RSVAL <= 0) JUMP_REL(SIMM16); break;
			case 0x07:  /* BGTZ */      if ((INT32)RSVAL > 0) JUMP_REL(SIMM16); break;
			case 0x08:  /* ADDI */      if (RTREG) RTVAL = (INT32)(RSVAL + SIMM16); break;
			case 0x09:  /* ADDIU */     if (RTREG) RTVAL = (INT32)(RSVAL + SIMM16); break;
			case 0x0a:  /* SLTI */      if (RTREG) RTVAL = (INT32)(RSVAL) < ((INT32)SIMM16); break;
			case 0x0b:  /* SLTIU */     if (RTREG) RTVAL = (UINT32)(RSVAL) < (UINT32)((INT32)SIMM16); break;
			case 0x0c:  /* ANDI */      if (RTREG) RTVAL = RSVAL & UIMM16; break;
			case 0x0d:  /* ORI */       if (RTREG) RTVAL = RSVAL | UIMM16; break;
			case 0x0e:  /* XORI */      if (RTREG) RTVAL = RSVAL ^ UIMM16; break;
			case 0x0f:  /* LUI */       if (RTREG) RTVAL = UIMM16 << 16; break;

			case 0x10:  /* COP0 */
			{
				switch ((op >> 21) & 0x1f)
				{
					case 0x00:  /* MFC0 */      if (RTREG) RTVAL = get_cop0_reg(RDREG); break;
					case 0x04:  /* MTC0 */      set_cop0_reg(RDREG, RTVAL); break;
					default:    unimplemented_opcode(op); break;
				}
				break;
			}

			case 0x12:  /* COP2 */
			{
				m_cop2->handle_cop2(op);
				break;
			}

			case 0x20:  /* LB */        if (RTREG) RTVAL = (INT32)(INT8)READ8(RSVAL + SIMM16); break;
			case 0x21:  /* LH */        if (RTREG) RTVAL = (INT32)(INT16)READ16(RSVAL + SIMM16); break;
			case 0x23:  /* LW */        if (RTREG) RTVAL = READ32(RSVAL + SIMM16); break;
			case 0x24:  /* LBU */       if (RTREG) RTVAL = (UINT8)READ8(RSVAL + SIMM16); break;
			case 0x25:  /* LHU */       if (RTREG) RTVAL = (UINT16)READ16(RSVAL + SIMM16); break;
			case 0x28:  /* SB */        WRITE8(RSVAL + SIMM16, RTVAL); break;
			case 0x29:  /* SH */        WRITE16(RSVAL + SIMM16, RTVAL); break;
			case 0x2b:  /* SW */        WRITE32(RSVAL + SIMM16, RTVAL); break;
			case 0x32:  /* LWC2 */      m_cop2->handle_lwc2(op); break;
			case 0x3a:  /* SWC2 */      m_cop2->handle_swc2(op); break;

			default:
			{
				unimplemented_opcode(op);
				break;
			}
		}

		if (LOG_INSTRUCTION_EXECUTION)
		{
			int i, l;
			static UINT32 prev_regs[32];
			char string[200];
			rsp_dasm_one(string, m_ppc, op);

			fprintf(m_exec_output, "%08X: %s", m_ppc, string);

			l = strlen(string);
			if (l < 36)
			{
				for (i=l; i < 36; i++)
				{
					fprintf(m_exec_output, " ");
				}
			}

			fprintf(m_exec_output, "| ");

			for (i=0; i < 32; i++)
			{
				if (m_rsp_state->r[i] != prev_regs[i])
				{
					fprintf(m_exec_output, "R%d: %08X ", i, m_rsp_state->r[i]);
				}
				prev_regs[i] = m_rsp_state->r[i];
			}

			m_cop2->log_instruction_execution();

			fprintf(m_exec_output, "\n");

		}

		--m_rsp_state->icount;

		if( m_sr & RSP_STATUS_SSTEP )
		{
			if( m_step_count )
			{
				m_step_count--;
			}
			else
			{
				m_sr |= RSP_STATUS_BROKE;
			}
		}

		if( m_sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
		{
			m_rsp_state->icount = MIN(m_rsp_state->icount, 0);
		}
		/*m_cop2->dump(op);
		if (((op >> 26) & 0x3f) == 0x3a)
		{
		    m_cop2->dump_dmem();
		}*/
	}
}
