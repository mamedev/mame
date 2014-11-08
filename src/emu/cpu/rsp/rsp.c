/*
    Nintendo/SGI Reality Signal Processor (RSP) emulator

    Written by Ville Linde
*/

#include "emu.h"
#include "debugger.h"
#include "rsp.h"
#include "rspdiv.h"
#include "rspfe.h"


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

#define JUMP_ABS(addr)          { m_nextpc = 0x04001000 | (((addr) << 2) & 0xfff); }
#define JUMP_ABS_L(addr,l)      { m_nextpc = 0x04001000 | (((addr) << 2) & 0xfff); m_rsp_state->r[l] = m_rsp_state->pc + 4; }
#define JUMP_REL(offset)        { m_nextpc = 0x04001000 | ((m_rsp_state->pc + ((offset) << 2)) & 0xfff); }
#define JUMP_REL_L(offset,l)    { m_nextpc = 0x04001000 | ((m_rsp_state->pc + ((offset) << 2)) & 0xfff); m_rsp_state->r[l] = m_rsp_state->pc + 4; }
#define JUMP_PC(addr)           { m_nextpc = 0x04001000 | ((addr) & 0xfff); }
#define JUMP_PC_L(addr,l)       { m_nextpc = 0x04001000 | ((addr) & 0xfff); m_rsp_state->r[l] = m_rsp_state->pc + 4; }
#define LINK(l)                 { m_rsp_state->r[l] = m_rsp_state->pc + 4; }

#define VREG_B(reg, offset)     m_v[(reg)].b[(offset)^1]
#define VREG_S(reg, offset)     m_v[(reg)].s[(offset)]
#define VREG_L(reg, offset)     m_v[(reg)].l[(offset)]

#define R_VREG_B(reg, offset)       m_v[(reg)].b[(offset)^1]
#define R_VREG_S(reg, offset)       (INT16)m_v[(reg)].s[(offset)]
#define R_VREG_L(reg, offset)       m_v[(reg)].l[(offset)]

#define W_VREG_B(reg, offset, val)  (m_v[(reg)].b[(offset)^1] = val)
#define W_VREG_S(reg, offset, val)  (m_v[(reg)].s[(offset)] = val)
#define W_VREG_L(reg, offset, val)  (m_v[(reg)].l[(offset)] = val)

#define VEC_EL_2(x,z)           (vector_elements[(x)][(z)])

#define ACCUM(x)        m_accum[((x))].q
#define ACCUM_H(x)      m_accum[((x))].w[3]
#define ACCUM_M(x)      m_accum[((x))].w[2]
#define ACCUM_L(x)      m_accum[((x))].w[1]

#define CARRY       0
#define COMPARE     1
#define CLIP1       2
#define ZERO        3
#define CLIP2       4

#define CARRY_FLAG(x)           (m_vflag[CARRY][x & 7] != 0 ? 0xffff : 0)
#define COMPARE_FLAG(x)         (m_vflag[COMPARE][x & 7] != 0 ? 0xffff : 0)
#define CLIP1_FLAG(x)           (m_vflag[CLIP1][x & 7] != 0 ? 0xffff : 0)
#define ZERO_FLAG(x)            (m_vflag[ZERO][x & 7] != 0 ? 0xffff : 0)
#define CLIP2_FLAG(x)           (m_vflag[CLIP2][x & 7] != 0 ? 0xffff : 0)

#define CLEAR_CARRY_FLAGS()     { memset(m_vflag[0], 0, 16); }
#define CLEAR_COMPARE_FLAGS()   { memset(m_vflag[1], 0, 16); }
#define CLEAR_CLIP1_FLAGS()     { memset(m_vflag[2], 0, 16); }
#define CLEAR_ZERO_FLAGS()      { memset(m_vflag[3], 0, 16); }
#define CLEAR_CLIP2_FLAGS()     { memset(m_vflag[4], 0, 16); }

#define SET_CARRY_FLAG(x)       { m_vflag[0][x & 7] = 0xffff; }
#define SET_COMPARE_FLAG(x)     { m_vflag[1][x & 7] = 0xffff; }
#define SET_CLIP1_FLAG(x)       { m_vflag[2][x & 7] = 0xffff; }
#define SET_ZERO_FLAG(x)        { m_vflag[3][x & 7] = 0xffff; }
#define SET_CLIP2_FLAG(x)       { m_vflag[4][x & 7] = 0xffff; }

#define CLEAR_CARRY_FLAG(x)     { m_vflag[0][x & 7] = 0; }
#define CLEAR_COMPARE_FLAG(x)   { m_vflag[1][x & 7] = 0; }
#define CLEAR_CLIP1_FLAG(x)     { m_vflag[2][x & 7] = 0; }
#define CLEAR_ZERO_FLAG(x)      { m_vflag[3][x & 7] = 0; }
#define CLEAR_CLIP2_FLAG(x)     { m_vflag[4][x & 7] = 0; }

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
#if SIMUL_SIMD
	, m_old_reciprocal_res(0)
	, m_old_reciprocal_high(0)
	, m_old_dp_allowed(0)
	, m_scalar_reciprocal_res(0)
	, m_scalar_reciprocal_high(0)
	, m_scalar_dp_allowed(0)
	, m_simd_reciprocal_res(0)
	, m_simd_reciprocal_high(0)
	, m_simd_dp_allowed(0)
#endif
	, m_sr(0)
	, m_step_count(0)
#if USE_SIMD
	, m_accum_h(0)
	, m_accum_m(0)
	, m_accum_l(0)
	, m_accum_ll(0)
#endif
	, m_reciprocal_res(0)
	, m_reciprocal_high(0)
	, m_dp_allowed(0)
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
	m_isdrc = mconfig.options().drc() ? true : false;
	memset(m_vres, 0, sizeof(m_vres));
	memset(m_v, 0, sizeof(m_v));
	memset(m_vflag, 0, sizeof(m_vflag));
#if SIMUL_SIMD
	memset(m_old_r, 0, sizeof(m_old_r));
	memset(m_old_dmem, 0, sizeof(m_old_dmem));
	memset(m_scalar_r, 0, sizeof(m_scalar_r));
	memset(m_scalar_dmem, 0, sizeof(m_scalar_dmem));
#endif
#if USE_SIMD
	memset(m_xv, 0, sizeof(m_xv));
	memset(m_xvflag, 0, sizeof(m_xvflag));
#endif
	memset(m_accum, 0, sizeof(m_accum));
}

offs_t rsp_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( rsp );
	return CPU_DISASSEMBLE_NAME( rsp )(this, buffer, pc, oprom, opram, options);
}

inline UINT8 rsp_device::READ8(UINT32 address)
{
	UINT8 ret;
	address = 0x04000000 | (address & 0xfff);
	ret = m_program->read_byte(address);
	//printf("%04xr%02x\n", address & 0x0000ffff, ret);
	return ret;
}

inline UINT16 rsp_device::READ16(UINT32 address)
{
	UINT16 ret;
	address = 0x04000000 | (address & 0xfff);

	if(address & 1)
	{
		ret = ((m_program->read_byte(address + 0) & 0xff) << 8) | (m_program->read_byte(address + 1) & 0xff);
	}
	else
	{
		ret = m_program->read_word(address);
	}

	//printf("%04xr%04x\n", address & 0x0000ffff, ret);

	return ret;
}

inline UINT32 rsp_device::READ32(UINT32 address)
{
	UINT32 ret;
	address = 0x04000000 | (address & 0xfff);

	if(address & 3)
	{
		ret =  ((m_program->read_byte(address + 0) & 0xff) << 24) |
				((m_program->read_byte(address + 1) & 0xff) << 16) |
				((m_program->read_byte(address + 2) & 0xff) << 8) |
				((m_program->read_byte(address + 3) & 0xff) << 0);
	}
	else
	{
		ret = m_program->read_dword(address);
	}

	//printf("%04xr%08x\n", address & 0x0000ffff, ret);
	return ret;
}

void rsp_device::WRITE8(UINT32 address, UINT8 data)
{
	address = 0x04000000 | (address & 0xfff);
	//printf("%04x:%02x\n", address & 0x0000ffff, data);
	m_program->write_byte(address, data);
}

void rsp_device::WRITE16(UINT32 address, UINT16 data)
{
	address = 0x04000000 | (address & 0xfff);
	//printf("%04x:%04x\n", address & 0x0000ffff, data);

	if(address & 1)
	{
		m_program->write_byte(address + 0, (data >> 8) & 0xff);
		m_program->write_byte(address + 1, (data >> 0) & 0xff);
		return;
	}

	m_program->write_word(address, data);
}

void rsp_device::WRITE32(UINT32 address, UINT32 data)
{
	address = 0x04000000 | (address & 0xfff);
	//printf("%04x:%08x\n", address & 0x0000ffff, data);

	if(address & 3)
	{
		m_program->write_byte(address + 0, (data >> 24) & 0xff);
		m_program->write_byte(address + 1, (data >> 16) & 0xff);
		m_program->write_byte(address + 2, (data >> 8) & 0xff);
		m_program->write_byte(address + 3, (data >> 0) & 0xff);
		return;
	}

	m_program->write_dword(address, data);
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

static const int vector_elements[16][8] =
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
	m_rsp_state = (internal_rsp_state *)m_cache.alloc_near(sizeof(internal_rsp_state));

	if (LOG_INSTRUCTION_EXECUTION)
		m_exec_output = fopen("rsp_execute.txt", "wt");

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	resolve_cb();

	// Inaccurate.  RSP registers power on to a random state...
	for(int regIdx = 0; regIdx < 32; regIdx++ )
	{
		m_rsp_state->r[regIdx] = 0;
		m_v[regIdx].d[0] = 0;
		m_v[regIdx].d[1] = 0;
	}
	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();
	//m_square_root_res = 0;
	//m_square_root_high = 0;
	m_reciprocal_res = 0;
	m_reciprocal_high = 0;

	// ...except for the accumulators.
	// We're not calling machine.rand() because initializing something with machine.rand()
	//   makes me retch uncontrollably.
	for(int accumIdx = 0; accumIdx < 8; accumIdx++ )
	{
		m_accum[accumIdx].q = 0;
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

	/*
	drcbe_info beinfo;
	m_drcuml->get_backend_info(beinfo);
	if (beinfo.direct_iregs > 2)
	{
	    m_regmap[30] = I2;
	}
	if (beinfo.direct_iregs > 3)
	{
	    m_regmap[31] = I3;
	}
	if (beinfo.direct_iregs > 4)
	{
	    m_regmap[2] = I4;
	}
	if (beinfo.direct_iregs > 5)
	{
	    m_regmap[3] = I5;
	}
	if (beinfo.direct_iregs > 6)
	{
	    m_regmap[4] = I6;
	}
	*/

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

void rsp_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%s","");
			break;

#if USE_SIMD
		case RSP_V0:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 0], 7), (UINT16)_mm_extract_epi16(m_xv[ 0], 6), (UINT16)_mm_extract_epi16(m_xv[ 0], 5), (UINT16)_mm_extract_epi16(m_xv[ 0], 4), (UINT16)_mm_extract_epi16(m_xv[ 0], 3), (UINT16)_mm_extract_epi16(m_xv[ 0], 2), (UINT16)_mm_extract_epi16(m_xv[ 0], 1), (UINT16)_mm_extract_epi16(m_xv[ 0], 0));
			break;
		case RSP_V1:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 1], 7), (UINT16)_mm_extract_epi16(m_xv[ 1], 6), (UINT16)_mm_extract_epi16(m_xv[ 1], 5), (UINT16)_mm_extract_epi16(m_xv[ 1], 4), (UINT16)_mm_extract_epi16(m_xv[ 1], 3), (UINT16)_mm_extract_epi16(m_xv[ 1], 2), (UINT16)_mm_extract_epi16(m_xv[ 1], 1), (UINT16)_mm_extract_epi16(m_xv[ 1], 0));
			break;
		case RSP_V2:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 2], 7), (UINT16)_mm_extract_epi16(m_xv[ 2], 6), (UINT16)_mm_extract_epi16(m_xv[ 2], 5), (UINT16)_mm_extract_epi16(m_xv[ 2], 4), (UINT16)_mm_extract_epi16(m_xv[ 2], 3), (UINT16)_mm_extract_epi16(m_xv[ 2], 2), (UINT16)_mm_extract_epi16(m_xv[ 2], 1), (UINT16)_mm_extract_epi16(m_xv[ 2], 0));
			break;
		case RSP_V3:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 3], 7), (UINT16)_mm_extract_epi16(m_xv[ 3], 6), (UINT16)_mm_extract_epi16(m_xv[ 3], 5), (UINT16)_mm_extract_epi16(m_xv[ 3], 4), (UINT16)_mm_extract_epi16(m_xv[ 3], 3), (UINT16)_mm_extract_epi16(m_xv[ 3], 2), (UINT16)_mm_extract_epi16(m_xv[ 3], 1), (UINT16)_mm_extract_epi16(m_xv[ 3], 0));
			break;
		case RSP_V4:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 4], 7), (UINT16)_mm_extract_epi16(m_xv[ 4], 6), (UINT16)_mm_extract_epi16(m_xv[ 4], 5), (UINT16)_mm_extract_epi16(m_xv[ 4], 4), (UINT16)_mm_extract_epi16(m_xv[ 4], 3), (UINT16)_mm_extract_epi16(m_xv[ 4], 2), (UINT16)_mm_extract_epi16(m_xv[ 4], 1), (UINT16)_mm_extract_epi16(m_xv[ 4], 0));
			break;
		case RSP_V5:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 5], 7), (UINT16)_mm_extract_epi16(m_xv[ 5], 6), (UINT16)_mm_extract_epi16(m_xv[ 5], 5), (UINT16)_mm_extract_epi16(m_xv[ 5], 4), (UINT16)_mm_extract_epi16(m_xv[ 5], 3), (UINT16)_mm_extract_epi16(m_xv[ 5], 2), (UINT16)_mm_extract_epi16(m_xv[ 5], 1), (UINT16)_mm_extract_epi16(m_xv[ 5], 0));
			break;
		case RSP_V6:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 6], 7), (UINT16)_mm_extract_epi16(m_xv[ 6], 6), (UINT16)_mm_extract_epi16(m_xv[ 6], 5), (UINT16)_mm_extract_epi16(m_xv[ 6], 4), (UINT16)_mm_extract_epi16(m_xv[ 6], 3), (UINT16)_mm_extract_epi16(m_xv[ 6], 2), (UINT16)_mm_extract_epi16(m_xv[ 6], 1), (UINT16)_mm_extract_epi16(m_xv[ 6], 0));
			break;
		case RSP_V7:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 7], 7), (UINT16)_mm_extract_epi16(m_xv[ 7], 6), (UINT16)_mm_extract_epi16(m_xv[ 7], 5), (UINT16)_mm_extract_epi16(m_xv[ 7], 4), (UINT16)_mm_extract_epi16(m_xv[ 7], 3), (UINT16)_mm_extract_epi16(m_xv[ 7], 2), (UINT16)_mm_extract_epi16(m_xv[ 7], 1), (UINT16)_mm_extract_epi16(m_xv[ 7], 0));
			break;
		case RSP_V8:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 8], 7), (UINT16)_mm_extract_epi16(m_xv[ 8], 6), (UINT16)_mm_extract_epi16(m_xv[ 8], 5), (UINT16)_mm_extract_epi16(m_xv[ 8], 4), (UINT16)_mm_extract_epi16(m_xv[ 8], 3), (UINT16)_mm_extract_epi16(m_xv[ 8], 2), (UINT16)_mm_extract_epi16(m_xv[ 8], 1), (UINT16)_mm_extract_epi16(m_xv[ 8], 0));
			break;
		case RSP_V9:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 9], 7), (UINT16)_mm_extract_epi16(m_xv[ 9], 6), (UINT16)_mm_extract_epi16(m_xv[ 9], 5), (UINT16)_mm_extract_epi16(m_xv[ 9], 4), (UINT16)_mm_extract_epi16(m_xv[ 9], 3), (UINT16)_mm_extract_epi16(m_xv[ 9], 2), (UINT16)_mm_extract_epi16(m_xv[ 9], 1), (UINT16)_mm_extract_epi16(m_xv[ 9], 0));
			break;
		case RSP_V10:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[10], 7), (UINT16)_mm_extract_epi16(m_xv[10], 6), (UINT16)_mm_extract_epi16(m_xv[10], 5), (UINT16)_mm_extract_epi16(m_xv[10], 4), (UINT16)_mm_extract_epi16(m_xv[10], 3), (UINT16)_mm_extract_epi16(m_xv[10], 2), (UINT16)_mm_extract_epi16(m_xv[10], 1), (UINT16)_mm_extract_epi16(m_xv[10], 0));
			break;
		case RSP_V11:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[11], 7), (UINT16)_mm_extract_epi16(m_xv[11], 6), (UINT16)_mm_extract_epi16(m_xv[11], 5), (UINT16)_mm_extract_epi16(m_xv[11], 4), (UINT16)_mm_extract_epi16(m_xv[11], 3), (UINT16)_mm_extract_epi16(m_xv[11], 2), (UINT16)_mm_extract_epi16(m_xv[11], 1), (UINT16)_mm_extract_epi16(m_xv[11], 0));
			break;
		case RSP_V12:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[12], 7), (UINT16)_mm_extract_epi16(m_xv[12], 6), (UINT16)_mm_extract_epi16(m_xv[12], 5), (UINT16)_mm_extract_epi16(m_xv[12], 4), (UINT16)_mm_extract_epi16(m_xv[12], 3), (UINT16)_mm_extract_epi16(m_xv[12], 2), (UINT16)_mm_extract_epi16(m_xv[12], 1), (UINT16)_mm_extract_epi16(m_xv[12], 0));
			break;
		case RSP_V13:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[13], 7), (UINT16)_mm_extract_epi16(m_xv[13], 6), (UINT16)_mm_extract_epi16(m_xv[13], 5), (UINT16)_mm_extract_epi16(m_xv[13], 4), (UINT16)_mm_extract_epi16(m_xv[13], 3), (UINT16)_mm_extract_epi16(m_xv[13], 2), (UINT16)_mm_extract_epi16(m_xv[13], 1), (UINT16)_mm_extract_epi16(m_xv[13], 0));
			break;
		case RSP_V14:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[14], 7), (UINT16)_mm_extract_epi16(m_xv[14], 6), (UINT16)_mm_extract_epi16(m_xv[14], 5), (UINT16)_mm_extract_epi16(m_xv[14], 4), (UINT16)_mm_extract_epi16(m_xv[14], 3), (UINT16)_mm_extract_epi16(m_xv[14], 2), (UINT16)_mm_extract_epi16(m_xv[14], 1), (UINT16)_mm_extract_epi16(m_xv[14], 0));
			break;
		case RSP_V15:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[15], 7), (UINT16)_mm_extract_epi16(m_xv[15], 6), (UINT16)_mm_extract_epi16(m_xv[15], 5), (UINT16)_mm_extract_epi16(m_xv[15], 4), (UINT16)_mm_extract_epi16(m_xv[15], 3), (UINT16)_mm_extract_epi16(m_xv[15], 2), (UINT16)_mm_extract_epi16(m_xv[15], 1), (UINT16)_mm_extract_epi16(m_xv[15], 0));
			break;
		case RSP_V16:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[16], 7), (UINT16)_mm_extract_epi16(m_xv[16], 6), (UINT16)_mm_extract_epi16(m_xv[16], 5), (UINT16)_mm_extract_epi16(m_xv[16], 4), (UINT16)_mm_extract_epi16(m_xv[16], 3), (UINT16)_mm_extract_epi16(m_xv[16], 2), (UINT16)_mm_extract_epi16(m_xv[16], 1), (UINT16)_mm_extract_epi16(m_xv[16], 0));
			break;
		case RSP_V17:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[17], 7), (UINT16)_mm_extract_epi16(m_xv[17], 6), (UINT16)_mm_extract_epi16(m_xv[17], 5), (UINT16)_mm_extract_epi16(m_xv[17], 4), (UINT16)_mm_extract_epi16(m_xv[17], 3), (UINT16)_mm_extract_epi16(m_xv[17], 2), (UINT16)_mm_extract_epi16(m_xv[17], 1), (UINT16)_mm_extract_epi16(m_xv[17], 0));
			break;
		case RSP_V18:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[18], 7), (UINT16)_mm_extract_epi16(m_xv[18], 6), (UINT16)_mm_extract_epi16(m_xv[18], 5), (UINT16)_mm_extract_epi16(m_xv[18], 4), (UINT16)_mm_extract_epi16(m_xv[18], 3), (UINT16)_mm_extract_epi16(m_xv[18], 2), (UINT16)_mm_extract_epi16(m_xv[18], 1), (UINT16)_mm_extract_epi16(m_xv[18], 0));
			break;
		case RSP_V19:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[19], 7), (UINT16)_mm_extract_epi16(m_xv[19], 6), (UINT16)_mm_extract_epi16(m_xv[19], 5), (UINT16)_mm_extract_epi16(m_xv[19], 4), (UINT16)_mm_extract_epi16(m_xv[19], 3), (UINT16)_mm_extract_epi16(m_xv[19], 2), (UINT16)_mm_extract_epi16(m_xv[19], 1), (UINT16)_mm_extract_epi16(m_xv[19], 0));
			break;
		case RSP_V20:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[20], 7), (UINT16)_mm_extract_epi16(m_xv[20], 6), (UINT16)_mm_extract_epi16(m_xv[20], 5), (UINT16)_mm_extract_epi16(m_xv[20], 4), (UINT16)_mm_extract_epi16(m_xv[20], 3), (UINT16)_mm_extract_epi16(m_xv[20], 2), (UINT16)_mm_extract_epi16(m_xv[20], 1), (UINT16)_mm_extract_epi16(m_xv[20], 0));
			break;
		case RSP_V21:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[21], 7), (UINT16)_mm_extract_epi16(m_xv[21], 6), (UINT16)_mm_extract_epi16(m_xv[21], 5), (UINT16)_mm_extract_epi16(m_xv[21], 4), (UINT16)_mm_extract_epi16(m_xv[21], 3), (UINT16)_mm_extract_epi16(m_xv[21], 2), (UINT16)_mm_extract_epi16(m_xv[21], 1), (UINT16)_mm_extract_epi16(m_xv[21], 0));
			break;
		case RSP_V22:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[22], 7), (UINT16)_mm_extract_epi16(m_xv[22], 6), (UINT16)_mm_extract_epi16(m_xv[22], 5), (UINT16)_mm_extract_epi16(m_xv[22], 4), (UINT16)_mm_extract_epi16(m_xv[22], 3), (UINT16)_mm_extract_epi16(m_xv[22], 2), (UINT16)_mm_extract_epi16(m_xv[22], 1), (UINT16)_mm_extract_epi16(m_xv[22], 0));
			break;
		case RSP_V23:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[23], 7), (UINT16)_mm_extract_epi16(m_xv[23], 6), (UINT16)_mm_extract_epi16(m_xv[23], 5), (UINT16)_mm_extract_epi16(m_xv[23], 4), (UINT16)_mm_extract_epi16(m_xv[23], 3), (UINT16)_mm_extract_epi16(m_xv[23], 2), (UINT16)_mm_extract_epi16(m_xv[23], 1), (UINT16)_mm_extract_epi16(m_xv[23], 0));
			break;
		case RSP_V24:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[24], 7), (UINT16)_mm_extract_epi16(m_xv[24], 6), (UINT16)_mm_extract_epi16(m_xv[24], 5), (UINT16)_mm_extract_epi16(m_xv[24], 4), (UINT16)_mm_extract_epi16(m_xv[24], 3), (UINT16)_mm_extract_epi16(m_xv[24], 2), (UINT16)_mm_extract_epi16(m_xv[24], 1), (UINT16)_mm_extract_epi16(m_xv[24], 0));
			break;
		case RSP_V25:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[25], 7), (UINT16)_mm_extract_epi16(m_xv[25], 6), (UINT16)_mm_extract_epi16(m_xv[25], 5), (UINT16)_mm_extract_epi16(m_xv[25], 4), (UINT16)_mm_extract_epi16(m_xv[25], 3), (UINT16)_mm_extract_epi16(m_xv[25], 2), (UINT16)_mm_extract_epi16(m_xv[25], 1), (UINT16)_mm_extract_epi16(m_xv[25], 0));
			break;
		case RSP_V26:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[26], 7), (UINT16)_mm_extract_epi16(m_xv[26], 6), (UINT16)_mm_extract_epi16(m_xv[26], 5), (UINT16)_mm_extract_epi16(m_xv[26], 4), (UINT16)_mm_extract_epi16(m_xv[26], 3), (UINT16)_mm_extract_epi16(m_xv[26], 2), (UINT16)_mm_extract_epi16(m_xv[26], 1), (UINT16)_mm_extract_epi16(m_xv[26], 0));
			break;
		case RSP_V27:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[27], 7), (UINT16)_mm_extract_epi16(m_xv[27], 6), (UINT16)_mm_extract_epi16(m_xv[27], 5), (UINT16)_mm_extract_epi16(m_xv[27], 4), (UINT16)_mm_extract_epi16(m_xv[27], 3), (UINT16)_mm_extract_epi16(m_xv[27], 2), (UINT16)_mm_extract_epi16(m_xv[27], 1), (UINT16)_mm_extract_epi16(m_xv[27], 0));
			break;
		case RSP_V28:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[28], 7), (UINT16)_mm_extract_epi16(m_xv[28], 6), (UINT16)_mm_extract_epi16(m_xv[28], 5), (UINT16)_mm_extract_epi16(m_xv[28], 4), (UINT16)_mm_extract_epi16(m_xv[28], 3), (UINT16)_mm_extract_epi16(m_xv[28], 2), (UINT16)_mm_extract_epi16(m_xv[28], 1), (UINT16)_mm_extract_epi16(m_xv[28], 0));
			break;
		case RSP_V29:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[29], 7), (UINT16)_mm_extract_epi16(m_xv[29], 6), (UINT16)_mm_extract_epi16(m_xv[29], 5), (UINT16)_mm_extract_epi16(m_xv[29], 4), (UINT16)_mm_extract_epi16(m_xv[29], 3), (UINT16)_mm_extract_epi16(m_xv[29], 2), (UINT16)_mm_extract_epi16(m_xv[29], 1), (UINT16)_mm_extract_epi16(m_xv[29], 0));
			break;
		case RSP_V30:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[30], 7), (UINT16)_mm_extract_epi16(m_xv[30], 6), (UINT16)_mm_extract_epi16(m_xv[30], 5), (UINT16)_mm_extract_epi16(m_xv[30], 4), (UINT16)_mm_extract_epi16(m_xv[30], 3), (UINT16)_mm_extract_epi16(m_xv[30], 2), (UINT16)_mm_extract_epi16(m_xv[30], 1), (UINT16)_mm_extract_epi16(m_xv[30], 0));
			break;
		case RSP_V31:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[31], 7), (UINT16)_mm_extract_epi16(m_xv[31], 6), (UINT16)_mm_extract_epi16(m_xv[31], 5), (UINT16)_mm_extract_epi16(m_xv[31], 4), (UINT16)_mm_extract_epi16(m_xv[31], 3), (UINT16)_mm_extract_epi16(m_xv[31], 2), (UINT16)_mm_extract_epi16(m_xv[31], 1), (UINT16)_mm_extract_epi16(m_xv[31], 0));
			break;
#else
		case RSP_V0:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 0, 0), (UINT16)VREG_S( 0, 1), (UINT16)VREG_S( 0, 2), (UINT16)VREG_S( 0, 3), (UINT16)VREG_S( 0, 4), (UINT16)VREG_S( 0, 5), (UINT16)VREG_S( 0, 6), (UINT16)VREG_S( 0, 7));
			break;
		case RSP_V1:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 1, 0), (UINT16)VREG_S( 1, 1), (UINT16)VREG_S( 1, 2), (UINT16)VREG_S( 1, 3), (UINT16)VREG_S( 1, 4), (UINT16)VREG_S( 1, 5), (UINT16)VREG_S( 1, 6), (UINT16)VREG_S( 1, 7));
			break;
		case RSP_V2:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 2, 0), (UINT16)VREG_S( 2, 1), (UINT16)VREG_S( 2, 2), (UINT16)VREG_S( 2, 3), (UINT16)VREG_S( 2, 4), (UINT16)VREG_S( 2, 5), (UINT16)VREG_S( 2, 6), (UINT16)VREG_S( 2, 7));
			break;
		case RSP_V3:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 3, 0), (UINT16)VREG_S( 3, 1), (UINT16)VREG_S( 3, 2), (UINT16)VREG_S( 3, 3), (UINT16)VREG_S( 3, 4), (UINT16)VREG_S( 3, 5), (UINT16)VREG_S( 3, 6), (UINT16)VREG_S( 3, 7));
			break;
		case RSP_V4:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 4, 0), (UINT16)VREG_S( 4, 1), (UINT16)VREG_S( 4, 2), (UINT16)VREG_S( 4, 3), (UINT16)VREG_S( 4, 4), (UINT16)VREG_S( 4, 5), (UINT16)VREG_S( 4, 6), (UINT16)VREG_S( 4, 7));
			break;
		case RSP_V5:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 5, 0), (UINT16)VREG_S( 5, 1), (UINT16)VREG_S( 5, 2), (UINT16)VREG_S( 5, 3), (UINT16)VREG_S( 5, 4), (UINT16)VREG_S( 5, 5), (UINT16)VREG_S( 5, 6), (UINT16)VREG_S( 5, 7));
			break;
		case RSP_V6:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 6, 0), (UINT16)VREG_S( 6, 1), (UINT16)VREG_S( 6, 2), (UINT16)VREG_S( 6, 3), (UINT16)VREG_S( 6, 4), (UINT16)VREG_S( 6, 5), (UINT16)VREG_S( 6, 6), (UINT16)VREG_S( 6, 7));
			break;
		case RSP_V7:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 7, 0), (UINT16)VREG_S( 7, 1), (UINT16)VREG_S( 7, 2), (UINT16)VREG_S( 7, 3), (UINT16)VREG_S( 7, 4), (UINT16)VREG_S( 7, 5), (UINT16)VREG_S( 7, 6), (UINT16)VREG_S( 7, 7));
			break;
		case RSP_V8:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 8, 0), (UINT16)VREG_S( 8, 1), (UINT16)VREG_S( 8, 2), (UINT16)VREG_S( 8, 3), (UINT16)VREG_S( 8, 4), (UINT16)VREG_S( 8, 5), (UINT16)VREG_S( 8, 6), (UINT16)VREG_S( 8, 7));
			break;
		case RSP_V9:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 9, 0), (UINT16)VREG_S( 9, 1), (UINT16)VREG_S( 9, 2), (UINT16)VREG_S( 9, 3), (UINT16)VREG_S( 9, 4), (UINT16)VREG_S( 9, 5), (UINT16)VREG_S( 9, 6), (UINT16)VREG_S( 9, 7));
			break;
		case RSP_V10:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(10, 0), (UINT16)VREG_S(10, 1), (UINT16)VREG_S(10, 2), (UINT16)VREG_S(10, 3), (UINT16)VREG_S(10, 4), (UINT16)VREG_S(10, 5), (UINT16)VREG_S(10, 6), (UINT16)VREG_S(10, 7));
			break;
		case RSP_V11:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(11, 0), (UINT16)VREG_S(11, 1), (UINT16)VREG_S(11, 2), (UINT16)VREG_S(11, 3), (UINT16)VREG_S(11, 4), (UINT16)VREG_S(11, 5), (UINT16)VREG_S(11, 6), (UINT16)VREG_S(11, 7));
			break;
		case RSP_V12:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(12, 0), (UINT16)VREG_S(12, 1), (UINT16)VREG_S(12, 2), (UINT16)VREG_S(12, 3), (UINT16)VREG_S(12, 4), (UINT16)VREG_S(12, 5), (UINT16)VREG_S(12, 6), (UINT16)VREG_S(12, 7));
			break;
		case RSP_V13:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(13, 0), (UINT16)VREG_S(13, 1), (UINT16)VREG_S(13, 2), (UINT16)VREG_S(13, 3), (UINT16)VREG_S(13, 4), (UINT16)VREG_S(13, 5), (UINT16)VREG_S(13, 6), (UINT16)VREG_S(13, 7));
			break;
		case RSP_V14:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(14, 0), (UINT16)VREG_S(14, 1), (UINT16)VREG_S(14, 2), (UINT16)VREG_S(14, 3), (UINT16)VREG_S(14, 4), (UINT16)VREG_S(14, 5), (UINT16)VREG_S(14, 6), (UINT16)VREG_S(14, 7));
			break;
		case RSP_V15:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(15, 0), (UINT16)VREG_S(15, 1), (UINT16)VREG_S(15, 2), (UINT16)VREG_S(15, 3), (UINT16)VREG_S(15, 4), (UINT16)VREG_S(15, 5), (UINT16)VREG_S(15, 6), (UINT16)VREG_S(15, 7));
			break;
		case RSP_V16:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(16, 0), (UINT16)VREG_S(16, 1), (UINT16)VREG_S(16, 2), (UINT16)VREG_S(16, 3), (UINT16)VREG_S(16, 4), (UINT16)VREG_S(16, 5), (UINT16)VREG_S(16, 6), (UINT16)VREG_S(16, 7));
			break;
		case RSP_V17:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(17, 0), (UINT16)VREG_S(17, 1), (UINT16)VREG_S(17, 2), (UINT16)VREG_S(17, 3), (UINT16)VREG_S(17, 4), (UINT16)VREG_S(17, 5), (UINT16)VREG_S(17, 6), (UINT16)VREG_S(17, 7));
			break;
		case RSP_V18:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(18, 0), (UINT16)VREG_S(18, 1), (UINT16)VREG_S(18, 2), (UINT16)VREG_S(18, 3), (UINT16)VREG_S(18, 4), (UINT16)VREG_S(18, 5), (UINT16)VREG_S(18, 6), (UINT16)VREG_S(18, 7));
			break;
		case RSP_V19:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(19, 0), (UINT16)VREG_S(19, 1), (UINT16)VREG_S(19, 2), (UINT16)VREG_S(19, 3), (UINT16)VREG_S(19, 4), (UINT16)VREG_S(19, 5), (UINT16)VREG_S(19, 6), (UINT16)VREG_S(19, 7));
			break;
		case RSP_V20:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(20, 0), (UINT16)VREG_S(20, 1), (UINT16)VREG_S(20, 2), (UINT16)VREG_S(20, 3), (UINT16)VREG_S(20, 4), (UINT16)VREG_S(20, 5), (UINT16)VREG_S(20, 6), (UINT16)VREG_S(20, 7));
			break;
		case RSP_V21:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(21, 0), (UINT16)VREG_S(21, 1), (UINT16)VREG_S(21, 2), (UINT16)VREG_S(21, 3), (UINT16)VREG_S(21, 4), (UINT16)VREG_S(21, 5), (UINT16)VREG_S(21, 6), (UINT16)VREG_S(21, 7));
			break;
		case RSP_V22:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(22, 0), (UINT16)VREG_S(22, 1), (UINT16)VREG_S(22, 2), (UINT16)VREG_S(22, 3), (UINT16)VREG_S(22, 4), (UINT16)VREG_S(22, 5), (UINT16)VREG_S(22, 6), (UINT16)VREG_S(22, 7));
			break;
		case RSP_V23:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(23, 0), (UINT16)VREG_S(23, 1), (UINT16)VREG_S(23, 2), (UINT16)VREG_S(23, 3), (UINT16)VREG_S(23, 4), (UINT16)VREG_S(23, 5), (UINT16)VREG_S(23, 6), (UINT16)VREG_S(23, 7));
			break;
		case RSP_V24:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(24, 0), (UINT16)VREG_S(24, 1), (UINT16)VREG_S(24, 2), (UINT16)VREG_S(24, 3), (UINT16)VREG_S(24, 4), (UINT16)VREG_S(24, 5), (UINT16)VREG_S(24, 6), (UINT16)VREG_S(24, 7));
			break;
		case RSP_V25:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(25, 0), (UINT16)VREG_S(25, 1), (UINT16)VREG_S(25, 2), (UINT16)VREG_S(25, 3), (UINT16)VREG_S(25, 4), (UINT16)VREG_S(25, 5), (UINT16)VREG_S(25, 6), (UINT16)VREG_S(25, 7));
			break;
		case RSP_V26:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(26, 0), (UINT16)VREG_S(26, 1), (UINT16)VREG_S(26, 2), (UINT16)VREG_S(26, 3), (UINT16)VREG_S(26, 4), (UINT16)VREG_S(26, 5), (UINT16)VREG_S(26, 6), (UINT16)VREG_S(26, 7));
			break;
		case RSP_V27:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(27, 0), (UINT16)VREG_S(27, 1), (UINT16)VREG_S(27, 2), (UINT16)VREG_S(27, 3), (UINT16)VREG_S(27, 4), (UINT16)VREG_S(27, 5), (UINT16)VREG_S(27, 6), (UINT16)VREG_S(27, 7));
			break;
		case RSP_V28:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(28, 0), (UINT16)VREG_S(28, 1), (UINT16)VREG_S(28, 2), (UINT16)VREG_S(28, 3), (UINT16)VREG_S(28, 4), (UINT16)VREG_S(28, 5), (UINT16)VREG_S(28, 6), (UINT16)VREG_S(28, 7));
			break;
		case RSP_V29:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(29, 0), (UINT16)VREG_S(29, 1), (UINT16)VREG_S(29, 2), (UINT16)VREG_S(29, 3), (UINT16)VREG_S(29, 4), (UINT16)VREG_S(29, 5), (UINT16)VREG_S(29, 6), (UINT16)VREG_S(29, 7));
			break;
		case RSP_V30:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(30, 0), (UINT16)VREG_S(30, 1), (UINT16)VREG_S(30, 2), (UINT16)VREG_S(30, 3), (UINT16)VREG_S(30, 4), (UINT16)VREG_S(30, 5), (UINT16)VREG_S(30, 6), (UINT16)VREG_S(30, 7));
			break;
		case RSP_V31:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(31, 0), (UINT16)VREG_S(31, 1), (UINT16)VREG_S(31, 2), (UINT16)VREG_S(31, 3), (UINT16)VREG_S(31, 4), (UINT16)VREG_S(31, 5), (UINT16)VREG_S(31, 6), (UINT16)VREG_S(31, 7));
			break;
#endif

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
	if ( m_drcuml )
	{
		auto_free(machine(), m_drcuml);
	}
	if (m_drcfe )
	{
		auto_free(machine(), m_drcfe);
	}
}

void rsp_device::device_reset()
{
	m_nextpc = ~0;
}

void rsp_device::handle_lwc2(UINT32 op)
{
	int i, end;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;

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

			ea = (base) ? m_rsp_state->r[base] + offset : offset;
			VREG_B(dest, index) = READ8(ea);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(ea);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(ea);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(ea);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));
			if (end > 16) end = 16;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(ea);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			index = 16 - ((ea & 0xf) - index);
			end = 16;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = READ8(ea);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = READ8(ea + (((16-index) + i) & 0xf)) << 8;
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = READ8(ea + (((16-index) + i) & 0xf)) << 7;
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = READ8(ea + (((16-index) + (i<<1)) & 0xf)) << 7;
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			// not sure what happens if 16-byte boundary is crossed...

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				VREG_S(dest, i) = READ8(ea) << 7;
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
			// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			// not sure what happens if 16-byte boundary is crossed...
			if ((ea & 0xf) > 0) fatalerror("RSP: LWV: 16-byte boundary crossing at %08X, recheck this!\n", m_ppc);

			end = (16 - index) + 16;

			for (i=(16 - index); i < end; i++)
			{
				VREG_B(dest, i & 0xf) = READ8(ea);
				ea += 4;
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

			int element;
			int vs = dest;
			int ve = dest + 8;
			if (ve > 32)
				ve = 32;

			element = 7 - (index >> 1);

			if (index & 1)  fatalerror("RSP: LTV: index = %d\n", index);

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			ea = ((ea + 8) & ~0xf) + (index & 1);
			for (i=vs; i < ve; i++)
			{
				element = ((8 - (index >> 1) + (i-vs)) << 1);
				VREG_B(i, (element & 0xf)) = READ8(ea);
				VREG_B(i, ((element + 1) & 0xf)) = READ8(ea + 1);

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

void rsp_device::handle_swc2(UINT32 op)
{
	int i, end;
	int eaoffset;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;

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

			ea = (base) ? m_rsp_state->r[base] + offset : offset;
			WRITE8(ea, VREG_B(dest, index));
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				WRITE8(ea, VREG_B(dest, i));
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				WRITE8(ea, VREG_B(dest, i));
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				WRITE8(ea, VREG_B(dest, i));
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));

			for (i=index; i < end; i++)
			{
				WRITE8(ea, VREG_B(dest, i & 0xf));
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

			int o;
			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (ea & 0xf);
			o = (16 - (ea & 0xf)) & 0xf;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				WRITE8(ea, VREG_B(dest, ((i + o) & 0xf)));
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					WRITE8(ea, VREG_B(dest, ((i & 0xf) << 1)));
				}
				else
				{
					WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
				}
				else
				{
					WRITE8(ea, VREG_B(dest, ((i & 0x7) << 1)));
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				UINT8 d = ((VREG_B(dest, ((index + (i << 1) + 0) & 0xf))) << 1) |
							((VREG_B(dest, ((index + (i << 1) + 1) & 0xf))) >> 7);

				WRITE8(ea, d);
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

			if (index & 0x7)    osd_printf_debug("RSP: SFV: index = %d at %08X\n", index, m_ppc);

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				WRITE8(ea + (eaoffset & 0xf), VREG_S(dest, i) >> 7);
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

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = index + 16;

			for (i=index; i < end; i++)
			{
				WRITE8(ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
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

			int element;
			int vs = dest;
			int ve = dest + 8;
			if (ve > 32)
				ve = 32;

			element = 8 - (index >> 1);

			ea = (base) ? m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = (ea & 0xf) + (element * 2);
			ea &= ~0xf;

			for (i=vs; i < ve; i++)
			{
				WRITE16(ea + (eaoffset & 0xf), VREG_S(i, element & 0x7));
				eaoffset += 2;
				element++;
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

inline UINT16 rsp_device::SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else if (slice == 1)
				{
					return ACCUM_M(accum);
				}
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else
				{
					return ACCUM_M(accum);
				}
			}
		}
	}

	return 0;
}

inline UINT16 rsp_device::SATURATE_ACCUM1(int accum, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
				return ACCUM_M(accum);
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
				return ACCUM_M(accum);
			}
		}
	}

	// never executed
	//return 0;
}

#define WRITEBACK_RESULT() {memcpy(&m_v[VDREG].s[0], &vres[0], 16);}

#if 0
static float float_round(float input)
{
	INT32 integer = (INT32)input;
	float fraction = input - (float)integer;
	float output = 0.0f;
	if( fraction >= 0.5f )
	{
		output = (float)( integer + 1 );
	}
	else
	{
		output = (float)integer;
	}
	return output;
}
#endif

void rsp_device::handle_vector_ops(UINT32 op)
{
	int i;
	UINT32 VS1REG = (op >> 11) & 0x1f;
	UINT32 VS2REG = (op >> 16) & 0x1f;
	UINT32 VDREG = (op >> 6) & 0x1f;
	UINT32 EL = (op >> 21) & 0xf;
	INT16 vres[8];

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

			int sel;
			INT32 s1, s2;
			INT64 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				if (s1 == -32768 && s2 == -32768)
				{
					// overflow
					ACCUM_H(i) = 0;
					ACCUM_M(i) = -32768;
					ACCUM_L(i) = -32768;
					vres[i] = 0x7fff;
				}
				else
				{
					r =  s1 * s2 * 2;
					r += 0x8000;    // rounding ?
					ACCUM_H(i) = (r < 0) ? 0xffff : 0;      // sign-extend to 48-bit
					ACCUM_M(i) = (INT16)(r >> 16);
					ACCUM_L(i) = (UINT16)(r);
					vres[i] = ACCUM_M(i);
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

			int sel;
			INT32 s1, s2;
			INT64 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2 * 2;
				r += 0x8000;    // rounding ?

				ACCUM_H(i) = (UINT16)(r >> 32);
				ACCUM_M(i) = (UINT16)(r >> 16);
				ACCUM_L(i) = (UINT16)(r);

				if (r < 0)
				{
					vres[i] = 0;
				}
				else if (((INT16)(ACCUM_H(i)) ^ (INT16)(ACCUM_M(i))) < 0)
				{
					vres[i] = -1;
				}
				else
				{
					vres[i] = ACCUM_M(i);
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

			int sel;
			UINT32 s1, s2;
			UINT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM_H(i) = 0;
				ACCUM_M(i) = 0;
				ACCUM_L(i) = (UINT16)(r >> 16);

				vres[i] = ACCUM_L(i);
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

			int sel;
			INT32 s1, s2;
			INT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (UINT16)VREG_S(VS2REG, sel);   // not sign-extended
				r =  s1 * s2;

				ACCUM_H(i) = (r < 0) ? 0xffff : 0;      // sign-extend to 48-bit
				ACCUM_M(i) = (INT16)(r >> 16);
				ACCUM_L(i) = (UINT16)(r);

				vres[i] = ACCUM_M(i);
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

			int sel;
			INT32 s1, s2;
			INT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT16)VREG_S(VS1REG, i);     // not sign-extended
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM_H(i) = (r < 0) ? 0xffff : 0;      // sign-extend to 48-bit
				ACCUM_M(i) = (INT16)(r >> 16);
				ACCUM_L(i) = (UINT16)(r);

				vres[i] = ACCUM_L(i);
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

			int sel;
			INT32 s1, s2;
			INT32 r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM_H(i) = (INT16)(r >> 16);
				ACCUM_M(i) = (UINT16)(r);
				ACCUM_L(i) = 0;

				if (r < -32768) r = -32768;
				if (r >  32767) r = 32767;
				vres[i] = (INT16)(r);
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

			int sel;
			INT32 s1, s2;
			INT32 r;
			UINT16 res;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 * s2;

				ACCUM(i) += (INT64)(r) << 17;
				res = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);

				vres[i] = res;
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

			UINT16 res;
			int sel;
			INT32 s1, s2, r1;
			UINT32 r2, r3;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r1 = s1 * s2;
				r2 = (UINT16)ACCUM_L(i) + ((UINT16)(r1) * 2);
				r3 = (UINT16)ACCUM_M(i) + (UINT16)((r1 >> 16) * 2) + (UINT16)(r2 >> 16);

				ACCUM_L(i) = (UINT16)(r2);
				ACCUM_M(i) = (UINT16)(r3);
				ACCUM_H(i) += (UINT16)(r3 >> 16) + (UINT16)(r1 >> 31);

				//res = SATURATE_ACCUM(i, 1, 0x0000, 0xffff);
				if ((INT16)ACCUM_H(i) < 0)
				{
					res = 0;
				}
				else
				{
					if (ACCUM_H(i) != 0)
					{
						res = 0xffff;
					}
					else
					{
						if ((INT16)ACCUM_M(i) < 0)
						{
							res = 0xffff;
						}
						else
						{
							res = ACCUM_M(i);
						}
					}
				}

				vres[i] = res;
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

			UINT16 res;
			int sel;
			UINT32 s1, s2, r1;
			UINT32 r2, r3;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r1 = s1 * s2;
				r2 = (UINT16)ACCUM_L(i) + (r1 >> 16);
				r3 = (UINT16)ACCUM_M(i) + (r2 >> 16);

				ACCUM_L(i) = (UINT16)(r2);
				ACCUM_M(i) = (UINT16)(r3);
				ACCUM_H(i) += (INT16)(r3 >> 16);

				res = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);

				vres[i] = res;
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

			UINT16 res;
			int sel;
			UINT32 s1, s2, r1;
			UINT32 r2, r3;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (UINT16)VREG_S(VS2REG, sel);   // not sign-extended
				r1 = s1 * s2;
				r2 = (UINT16)ACCUM_L(i) + (UINT16)(r1);
				r3 = (UINT16)ACCUM_M(i) + (r1 >> 16) + (r2 >> 16);

				ACCUM_L(i) = (UINT16)(r2);
				ACCUM_M(i) = (UINT16)(r3);
				ACCUM_H(i) += (UINT16)(r3 >> 16);
				if ((INT32)(r1) < 0)
					ACCUM_H(i) -= 1;

				res = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);

				vres[i] = res;
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

			INT32 s1, s2;
			UINT16 res;
			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT16)VREG_S(VS1REG, i);     // not sign-extended
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);

				ACCUM(i) += (INT64)(s1*s2)<<16;

				res = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
				vres[i] = res;
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

			UINT16 res;
			int sel;
			INT32 s1, s2;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);

				m_accum[i].l[1] += s1*s2;

				res = SATURATE_ACCUM1(i, 0x8000, 0x7fff);

				vres[i] = res;
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

			int sel;
			INT32 s1, s2, r;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 + s2 + CARRY_FLAG(i);

				ACCUM_L(i) = (INT16)(r);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;
				vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
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

			int sel;
			INT32 s1, s2, r;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
				r = s1 - s2 - CARRY_FLAG(i);

				ACCUM_L(i) = (INT16)(r);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;

				vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
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

			int sel;
			INT16 s1, s2;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (INT16)VREG_S(VS1REG, i);
				s2 = (INT16)VREG_S(VS2REG, sel);

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

				ACCUM_L(i) = vres[i];
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

			int sel;
			INT32 s1, s2, r;
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r = s1 + s2;

				vres[i] = (INT16)(r);
				ACCUM_L(i) = (INT16)(r);

				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
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

			int sel;
			INT32 s1, s2, r;
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
				r = s1 - s2;

				vres[i] = (INT16)(r);
				ACCUM_L(i) = (UINT16)(r);

				if ((UINT16)(r) != 0)
				{
					SET_ZERO_FLAG(i);
				}
				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
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
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_H(i);
					}
					break;
				}
				case 0x09:      // VSAWM
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_M(i);
					}
					break;
				}
				case 0x0a:      // VSAWL
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_L(i);
					}
					break;
				}
				default:    //fatalerror("RSP: VSAW: el = %d\n", EL);//???????
					printf("RSP: VSAW: el = %d\n", EL);//??? ???
					exit(0);
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

			int sel;
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);

				if (VREG_S(VS1REG, i) < VREG_S(VS2REG, sel))
				{
					SET_COMPARE_FLAG(i);
				}
				else if (VREG_S(VS1REG, i) == VREG_S(VS2REG, sel))
				{
					if (ZERO_FLAG(i) == 1 && CARRY_FLAG(i) != 0)
					{
						SET_COMPARE_FLAG(i);
					}
				}

				if (COMPARE_FLAG(i))
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}

				ACCUM_L(i) = vres[i];
			}

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
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

			int sel;
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);

				if ((VREG_S(VS1REG, i) == VREG_S(VS2REG, sel)) && ZERO_FLAG(i) == 0)
				{
					SET_COMPARE_FLAG(i);
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}
				ACCUM_L(i) = vres[i];
			}

			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
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

			int sel;
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)//?????????? ????
			{
				sel = VEC_EL_2(EL, i);

				if (VREG_S(VS1REG, i) != VREG_S(VS2REG, sel))
				{
					SET_COMPARE_FLAG(i);
				}
				else
				{
					if (ZERO_FLAG(i) == 1)
					{
						SET_COMPARE_FLAG(i);
					}
				}
				if (COMPARE_FLAG(i))
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}
				ACCUM_L(i) = vres[i];
			}

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
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

			int sel;
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);

				if (VREG_S(VS1REG, i) == VREG_S(VS2REG, sel))
				{
					if (ZERO_FLAG(i) == 0 || CARRY_FLAG(i) == 0)
					{
						SET_COMPARE_FLAG(i);
					}
				}
				else if (VREG_S(VS1REG, i) > VREG_S(VS2REG, sel))
				{
					SET_COMPARE_FLAG(i);
				}

				if (COMPARE_FLAG(i) != 0)
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);
				}

				ACCUM_L(i) = vres[i];
			}

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
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

			int sel;
			INT16 s1, s2;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, sel);

				if (CARRY_FLAG(i) != 0)
				{
					if (ZERO_FLAG(i) != 0)
					{
						if (COMPARE_FLAG(i) != 0)
						{
							ACCUM_L(i) = -(UINT16)s2;
						}
						else
						{
							ACCUM_L(i) = s1;
						}
					}
					else//ZERO_FLAG(i)==0
					{
						if (CLIP1_FLAG(i) != 0)
						{
							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
							{//proper fix for Harvest Moon 64, r4

								ACCUM_L(i) = s1;
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{
								ACCUM_L(i) = -((UINT16)s2);
								SET_COMPARE_FLAG(i);
							}
						}
						else
						{
							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) != 0)
							{
								ACCUM_L(i) = s1;
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{
								ACCUM_L(i) = -((UINT16)s2);
								SET_COMPARE_FLAG(i);
							}
						}
					}
				}//
				else//CARRY_FLAG(i)==0
				{
					if (ZERO_FLAG(i) != 0)
					{
						if (CLIP2_FLAG(i) != 0)
						{
							ACCUM_L(i) = s2;
						}
						else
						{
							ACCUM_L(i) = s1;
						}
					}
					else
					{
						if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
						{
							ACCUM_L(i) = s2;
							SET_CLIP2_FLAG(i);
						}
						else
						{
							ACCUM_L(i) = s1;
							CLEAR_CLIP2_FLAG(i);
						}
					}
				}

				vres[i] = ACCUM_L(i);
			}
			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP1_FLAGS();
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

			int sel;
			INT16 s1, s2;
			CLEAR_CARRY_FLAGS();
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP1_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP2_FLAGS();
			UINT32 vce = 0;

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, sel);

				if ((s1 ^ s2) < 0)
				{
					vce = (s1 + s2 == -1);
					SET_CARRY_FLAG(i);
					if (s2 < 0)
					{
						SET_CLIP2_FLAG(i);
					}

					if (s1 + s2 <= 0)
					{
						SET_COMPARE_FLAG(i);
						vres[i] = -((UINT16)s2);
					}
					else
					{
						vres[i] = s1;
					}

					if (s1 + s2 != 0)
					{
						if (s1 != ~s2)
						{
							SET_ZERO_FLAG(i);
						}
					}
				}//sign
				else
				{
					vce = 0;
					if (s2 < 0)
					{
						SET_COMPARE_FLAG(i);
					}
					if (s1 - s2 >= 0)
					{
						SET_CLIP2_FLAG(i);
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
							SET_ZERO_FLAG(i);
						}
					}
				}
				if (vce != 0)
				{
					SET_CLIP1_FLAG(i);
				}
				ACCUM_L(i) = vres[i];
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

			int sel;
			INT16 s1, s2;
			CLEAR_CARRY_FLAGS();
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP1_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, sel);

				if ((INT16)(s1 ^ s2) < 0)
				{
					if (s2 < 0)
					{
						SET_CLIP2_FLAG(i);
					}
					if ((s1 + s2) <= 0)
					{
						ACCUM_L(i) = ~((UINT16)s2);
						SET_COMPARE_FLAG(i);
					}
					else
					{
						ACCUM_L(i) = s1;
					}
				}
				else
				{
					if (s2 < 0)
					{
						SET_COMPARE_FLAG(i);
					}
					if ((s1 - s2) >= 0)
					{
						ACCUM_L(i) = s2;
						SET_CLIP2_FLAG(i);
					}
					else
					{
						ACCUM_L(i) = s1;
					}
				}

				vres[i] = ACCUM_L(i);
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

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				if (COMPARE_FLAG(i) != 0)
				{
					vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					vres[i] = VREG_S(VS2REG, sel);//??? ???????????
				}

				ACCUM_L(i) = vres[i];
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

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = VREG_S(VS1REG, i) & VREG_S(VS2REG, sel);
				ACCUM_L(i) = vres[i];
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

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = ~((VREG_S(VS1REG, i) & VREG_S(VS2REG, sel)));
				ACCUM_L(i) = vres[i];
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

			int sel;
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = VREG_S(VS1REG, i) | VREG_S(VS2REG, sel);
				ACCUM_L(i) = vres[i];
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

			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = ~((VREG_S(VS1REG, i) | VREG_S(VS2REG, sel)));
				ACCUM_L(i) = vres[i];
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

			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = VREG_S(VS1REG, i) ^ VREG_S(VS2REG, sel);
				ACCUM_L(i) = vres[i];
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

			int sel;
			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				vres[i] = ~((VREG_S(VS1REG, i) ^ VREG_S(VS2REG, sel)));
				ACCUM_L(i) = vres[i];
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
			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, sel));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
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

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
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

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
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

			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = ((UINT16)(VREG_S(VS2REG, sel)) | ((UINT32)(m_reciprocal_high) & 0xffff0000));

			INT32 datainput = rec;

			if (rec < 0)
			{
				if (m_dp_allowed)
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
				else
				{
					datainput = -datainput;
				}
			}


			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
					{
						shifter = i;
						break;
					}
				}
			}
			else
			{
				if (m_dp_allowed)
				{
					shifter = 0;
				}
				else
				{
					shifter = 0x10;
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
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

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
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

			int del = VS1REG & 7;
			int sel = EL & 7;

			m_reciprocal_high = (VREG_S(VS2REG, sel)) << 16;
			m_dp_allowed = 1;

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}

			VREG_S(VDREG, del) = (INT16)(m_reciprocal_res >> 16);

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

			int del = VS1REG & 7;
			int sel = EL & 7;

			VREG_S(VDREG, del) = VREG_S(VS2REG, sel);
			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
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

			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, sel));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
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

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
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

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
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

			int del = VS1REG & 7;
			int sel = EL & 7;
			INT32 shifter = 0;

			INT32 rec = ((UINT16)(VREG_S(VS2REG, sel)) | ((UINT32)(m_reciprocal_high) & 0xffff0000));

			INT32 datainput = rec;

			if (rec < 0)
			{
				if (m_dp_allowed)
				{
					if (rec < -32768)//VDIV.C,208
					{
						datainput = ~datainput;
					}
					else
					{
						datainput = -datainput;
					}
				}
				else
				{
					datainput = -datainput;
				}
			}

			if (datainput)
			{
				for (i = 0; i < 32; i++)
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
				if (m_dp_allowed)
				{
					shifter = 0;
				}
				else
				{
					shifter = 0x10;
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
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

			VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
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

			int del = VS1REG & 7;
			int sel = EL & 7;

			m_reciprocal_high = (VREG_S(VS2REG, sel)) << 16;
			m_dp_allowed = 1;

			for (i=0; i < 8; i++)
			{
				sel = VEC_EL_2(EL, i);
				ACCUM_L(i) = VREG_S(VS2REG, sel);
			}

			VREG_S(VDREG, del) = (INT16)(m_reciprocal_res >> 16);    // store high part
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

		default:    unimplemented_opcode(op); break;
	}
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

						if (LOG_INSTRUCTION_EXECUTION) fprintf(m_exec_output, "\n---------- break ----------\n\n");

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
				switch ((op >> 21) & 0x1f)
				{
					case 0x00:  /* MFC2 */
					{
						// 31       25      20      15      10     6         0
						// ---------------------------------------------------
						// | 010010 | 00000 | TTTTT | DDDDD | IIII | 0000000 |
						// ---------------------------------------------------
						//

						int el = (op >> 7) & 0xf;
						UINT16 b1 = VREG_B(RDREG, (el+0) & 0xf);
						UINT16 b2 = VREG_B(RDREG, (el+1) & 0xf);
						if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
						break;
					}
					case 0x02:  /* CFC2 */
					{
						// 31       25      20      15      10            0
						// ------------------------------------------------
						// | 010010 | 00010 | TTTTT | DDDDD | 00000000000 |
						// ------------------------------------------------
						//

						if (RTREG)
						{
							switch(RDREG)
							{
								case 0:
									RTVAL = ((CARRY_FLAG(0) & 1) << 0) |
											((CARRY_FLAG(1) & 1) << 1) |
											((CARRY_FLAG(2) & 1) << 2) |
											((CARRY_FLAG(3) & 1) << 3) |
											((CARRY_FLAG(4) & 1) << 4) |
											((CARRY_FLAG(5) & 1) << 5) |
											((CARRY_FLAG(6) & 1) << 6) |
											((CARRY_FLAG(7) & 1) << 7) |
											((ZERO_FLAG(0) & 1) << 8) |
											((ZERO_FLAG(1) & 1) << 9) |
											((ZERO_FLAG(2) & 1) << 10) |
											((ZERO_FLAG(3) & 1) << 11) |
											((ZERO_FLAG(4) & 1) << 12) |
											((ZERO_FLAG(5) & 1) << 13) |
											((ZERO_FLAG(6) & 1) << 14) |
											((ZERO_FLAG(7) & 1) << 15);
									if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
									break;
								case 1:
									RTVAL = ((COMPARE_FLAG(0) & 1) << 0) |
											((COMPARE_FLAG(1) & 1) << 1) |
											((COMPARE_FLAG(2) & 1) << 2) |
											((COMPARE_FLAG(3) & 1) << 3) |
											((COMPARE_FLAG(4) & 1) << 4) |
											((COMPARE_FLAG(5) & 1) << 5) |
											((COMPARE_FLAG(6) & 1) << 6) |
											((COMPARE_FLAG(7) & 1) << 7) |
											((CLIP2_FLAG(0) & 1) << 8) |
											((CLIP2_FLAG(1) & 1) << 9) |
											((CLIP2_FLAG(2) & 1) << 10) |
											((CLIP2_FLAG(3) & 1) << 11) |
											((CLIP2_FLAG(4) & 1) << 12) |
											((CLIP2_FLAG(5) & 1) << 13) |
											((CLIP2_FLAG(6) & 1) << 14) |
											((CLIP2_FLAG(7) & 1) << 15);
									if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
									break;
								case 2:
									// Anciliary clipping flags
									RTVAL = ((CARRY_FLAG(0) & 1) << 0) |
											((CARRY_FLAG(1) & 1) << 1) |
											((CARRY_FLAG(2) & 1) << 2) |
											((CARRY_FLAG(3) & 1) << 3) |
											((CARRY_FLAG(4) & 1) << 4) |
											((CARRY_FLAG(5) & 1) << 5) |
											((CARRY_FLAG(6) & 1) << 6) |
											((CARRY_FLAG(7) & 1) << 7) |
											((ZERO_FLAG(0) & 1) << 8) |
											((ZERO_FLAG(1) & 1) << 9) |
											((ZERO_FLAG(2) & 1) << 10) |
											((ZERO_FLAG(3) & 1) << 11) |
											((ZERO_FLAG(4) & 1) << 12) |
											((ZERO_FLAG(5) & 1) << 13) |
											((ZERO_FLAG(6) & 1) << 14) |
											((ZERO_FLAG(7) & 1) << 15);
									if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
							}
						}
						break;
					}
					case 0x04:  /* MTC2 */
					{
						// 31       25      20      15      10     6         0
						// ---------------------------------------------------
						// | 010010 | 00100 | TTTTT | DDDDD | IIII | 0000000 |
						// ---------------------------------------------------
						//

						int el = (op >> 7) & 0xf;
						W_VREG_B(RDREG, (el+0) & 0xf, (RTVAL >> 8) & 0xff);
						W_VREG_B(RDREG, (el+1) & 0xf, (RTVAL >> 0) & 0xff);
						break;
					}
					case 0x06:  /* CTC2 */
					{
						// 31       25      20      15      10            0
						// ------------------------------------------------
						// | 010010 | 00110 | TTTTT | DDDDD | 00000000000 |
						// ------------------------------------------------
						//

						switch(RDREG)
						{
							case 0:
								CLEAR_CARRY_FLAGS();
								CLEAR_ZERO_FLAGS();
								if (RTVAL & (1 << 0))  { SET_CARRY_FLAG(0); }
								if (RTVAL & (1 << 1))  { SET_CARRY_FLAG(1); }
								if (RTVAL & (1 << 2))  { SET_CARRY_FLAG(2); }
								if (RTVAL & (1 << 3))  { SET_CARRY_FLAG(3); }
								if (RTVAL & (1 << 4))  { SET_CARRY_FLAG(4); }
								if (RTVAL & (1 << 5))  { SET_CARRY_FLAG(5); }
								if (RTVAL & (1 << 6))  { SET_CARRY_FLAG(6); }
								if (RTVAL & (1 << 7))  { SET_CARRY_FLAG(7); }
								if (RTVAL & (1 << 8))  { SET_ZERO_FLAG(0); }
								if (RTVAL & (1 << 9))  { SET_ZERO_FLAG(1); }
								if (RTVAL & (1 << 10)) { SET_ZERO_FLAG(2); }
								if (RTVAL & (1 << 11)) { SET_ZERO_FLAG(3); }
								if (RTVAL & (1 << 12)) { SET_ZERO_FLAG(4); }
								if (RTVAL & (1 << 13)) { SET_ZERO_FLAG(5); }
								if (RTVAL & (1 << 14)) { SET_ZERO_FLAG(6); }
								if (RTVAL & (1 << 15)) { SET_ZERO_FLAG(7); }
								break;
							case 1:
								CLEAR_COMPARE_FLAGS();
								CLEAR_CLIP2_FLAGS();
								if (RTVAL & (1 << 0)) { SET_COMPARE_FLAG(0); }
								if (RTVAL & (1 << 1)) { SET_COMPARE_FLAG(1); }
								if (RTVAL & (1 << 2)) { SET_COMPARE_FLAG(2); }
								if (RTVAL & (1 << 3)) { SET_COMPARE_FLAG(3); }
								if (RTVAL & (1 << 4)) { SET_COMPARE_FLAG(4); }
								if (RTVAL & (1 << 5)) { SET_COMPARE_FLAG(5); }
								if (RTVAL & (1 << 6)) { SET_COMPARE_FLAG(6); }
								if (RTVAL & (1 << 7)) { SET_COMPARE_FLAG(7); }
								if (RTVAL & (1 << 8))  { SET_CLIP2_FLAG(0); }
								if (RTVAL & (1 << 9))  { SET_CLIP2_FLAG(1); }
								if (RTVAL & (1 << 10)) { SET_CLIP2_FLAG(2); }
								if (RTVAL & (1 << 11)) { SET_CLIP2_FLAG(3); }
								if (RTVAL & (1 << 12)) { SET_CLIP2_FLAG(4); }
								if (RTVAL & (1 << 13)) { SET_CLIP2_FLAG(5); }
								if (RTVAL & (1 << 14)) { SET_CLIP2_FLAG(6); }
								if (RTVAL & (1 << 15)) { SET_CLIP2_FLAG(7); }
								break;
							case 2:
								CLEAR_CLIP1_FLAGS();
								if (RTVAL & (1 << 0)) { SET_CLIP1_FLAG(0); }
								if (RTVAL & (1 << 1)) { SET_CLIP1_FLAG(1); }
								if (RTVAL & (1 << 2)) { SET_CLIP1_FLAG(2); }
								if (RTVAL & (1 << 3)) { SET_CLIP1_FLAG(3); }
								if (RTVAL & (1 << 4)) { SET_CLIP1_FLAG(4); }
								if (RTVAL & (1 << 5)) { SET_CLIP1_FLAG(5); }
								if (RTVAL & (1 << 6)) { SET_CLIP1_FLAG(6); }
								if (RTVAL & (1 << 7)) { SET_CLIP1_FLAG(7); }
								break;
						}
						break;
					}

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
					{
						handle_vector_ops(op);
						break;
					}

					default:    unimplemented_opcode(op); break;
				}
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
			case 0x32:  /* LWC2 */      handle_lwc2(op); break;
			case 0x3a:  /* SWC2 */      handle_swc2(op); break;

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
			static VECTOR_REG prev_vecs[32];
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

			for (i=0; i < 32; i++)
			{
				if (m_v[i].d[0] != prev_vecs[i].d[0] || m_v[i].d[1] != prev_vecs[i].d[1])
				{
					fprintf(m_exec_output, "V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X ", i,
					(UINT16)VREG_S(i,0), (UINT16)VREG_S(i,1), (UINT16)VREG_S(i,2), (UINT16)VREG_S(i,3), (UINT16)VREG_S(i,4), (UINT16)VREG_S(i,5), (UINT16)VREG_S(i,6), (UINT16)VREG_S(i,7));
				}
				prev_vecs[i].d[0] = m_v[i].d[0];
				prev_vecs[i].d[1] = m_v[i].d[1];
			}

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

	}
}
