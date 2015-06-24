// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    cubeqcpu.c

    Implementation of the Cube Quest AM2901-based CPUs
    Copyright Philip J Bennett

    TODO:

    * Tidy up diassembly (split into different files?)

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cubeqcpu.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Am2901 Instruction Fields */
enum alu_src
{
	AQ = 0,
	AB = 1,
	ZQ = 2,
	ZB = 3,
	ZA = 4,
	DA = 5,
	DQ = 6,
	DZ = 7,
};

enum alu_ins
{
	ADD   = 0,
	SUBR  = 1,
	SUBS  = 2,
	OR    = 3,
	AND   = 4,
	NOTRS = 5,
	EXOR  = 6,
	EXNOR = 7,
};

enum alu_dst
{
	QREG  = 0,
	NOP   = 1,
	RAMA  = 2,
	RAMF  = 3,
	RAMQD = 4,
	RAMD  = 5,
	RAMQU = 6,
	RAMU  = 7,
};

/***************************************************************************
    MACROS
***************************************************************************/

#define _BIT(x, n)          ((x) & (1 << (n)))

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/


const device_type CQUESTSND = &device_creator<cquestsnd_cpu_device>;
const device_type CQUESTROT = &device_creator<cquestrot_cpu_device>;
const device_type CQUESTLIN = &device_creator<cquestlin_cpu_device>;


cquestsnd_cpu_device::cquestsnd_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, CQUESTSND, "Cube Quest Sound CPU", tag, owner, clock, "cquestsnd", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 64, 8, -3)
	, m_dac_w(*this)
	, m_sound_region_tag(NULL)
{
}


offs_t cquestsnd_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( cquestsnd );
	return CPU_DISASSEMBLE_NAME(cquestsnd)(this, buffer, pc, oprom, opram, options);
}


cquestrot_cpu_device::cquestrot_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, CQUESTROT, "Cube Quest Rotate CPU", tag, owner, clock, "cquestrot", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 64, 9, -3)
	, m_linedata_w(*this)
{
}


READ16_MEMBER( cquestrot_cpu_device::linedata_r )
{
	return m_linedata;
}


offs_t cquestrot_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( cquestrot );
	return CPU_DISASSEMBLE_NAME(cquestrot)(this, buffer, pc, oprom, opram, options);
}


cquestlin_cpu_device::cquestlin_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, CQUESTLIN, "Cube Quest Line CPU", tag, owner, clock, "cquestlin", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 64, 8, -3)
	, m_linedata_r(*this)
	, m_flags(0)
	, m_curpc(0)
{
}


offs_t cquestlin_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( cquestlin );
	return CPU_DISASSEMBLE_NAME(cquestlin)(this, buffer, pc, oprom, opram, options);
}


WRITE16_MEMBER( cquestlin_cpu_device::linedata_w )
{
	m_sram[offset] = data;
}


/***************************************************************************
    MEMORY ACCESSORS FOR 68000
***************************************************************************/

WRITE16_MEMBER( cquestsnd_cpu_device::sndram_w )
{
	COMBINE_DATA(&m_sram[offset]);
}

READ16_MEMBER( cquestsnd_cpu_device::sndram_r )
{
	return m_sram[offset];
}


WRITE16_MEMBER( cquestrot_cpu_device::rotram_w )
{
	COMBINE_DATA(&m_dram[offset]);
}

READ16_MEMBER( cquestrot_cpu_device::rotram_r )
{
	return m_dram[offset];
}


/***************************************************************************
    SOUND INITIALIZATION AND SHUTDOWN
***************************************************************************/

void cquestsnd_cpu_device::device_start()
{
	m_dac_w.resolve_safe();
	assert(m_sound_region_tag != NULL);
	m_sound_data = (UINT16*)machine().root_device().memregion(m_sound_region_tag)->base();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	memset(m_ram, 0, sizeof(m_ram));
	m_q = 0;
	m_f = 0;
	m_y = 0;
	m_cflag = 0;
	m_vflag = 0;
	m_pc = 0;
	m_platch = 0;
	m_rtnlatch = 0;
	m_adrcntr = 0;
	m_dinlatch = 0;
	m_ramwlatch = 0;
	m_prev_ipram = 0;
	m_prev_ipwrt = 0;
	m_adrlatch = 0;

	save_item(NAME(m_ram));
	save_item(NAME(m_q));
	save_item(NAME(m_f));
	save_item(NAME(m_y));
	save_item(NAME(m_cflag));
	save_item(NAME(m_vflag));

	save_item(NAME(m_pc));
	save_item(NAME(m_platch));
	save_item(NAME(m_rtnlatch));
	save_item(NAME(m_adrcntr));
	save_item(NAME(m_adrlatch));
	save_item(NAME(m_dinlatch));
	save_item(NAME(m_ramwlatch));
	save_item(NAME(m_prev_ipram));
	save_item(NAME(m_prev_ipwrt));

	state_add( CQUESTSND_PC,       "PC",     m_pc).formatstr("%02X");
	state_add( CQUESTSND_Q,        "Q",      m_q).formatstr("%04X");
	state_add( CQUESTSND_RTNLATCH, "RTN",    m_rtnlatch).formatstr("%02X");
	state_add( CQUESTSND_ADRCNTR,  "CNT",    m_adrcntr).formatstr("%02X");
	state_add( CQUESTSND_DINLATCH, "DINX",   m_dinlatch).formatstr("%04X");
	state_add( CQUESTSND_RAM0,     "RAM[0]", m_ram[0x0]).formatstr("%04X");
	state_add( CQUESTSND_RAM1,     "RAM[1]", m_ram[0x1]).formatstr("%04X");
	state_add( CQUESTSND_RAM2,     "RAM[2]", m_ram[0x2]).formatstr("%04X");
	state_add( CQUESTSND_RAM3,     "RAM[3]", m_ram[0x3]).formatstr("%04X");
	state_add( CQUESTSND_RAM4,     "RAM[4]", m_ram[0x4]).formatstr("%04X");
	state_add( CQUESTSND_RAM5,     "RAM[5]", m_ram[0x5]).formatstr("%04X");
	state_add( CQUESTSND_RAM6,     "RAM[6]", m_ram[0x6]).formatstr("%04X");
	state_add( CQUESTSND_RAM7,     "RAM[7]", m_ram[0x7]).formatstr("%04X");
	state_add( CQUESTSND_RAM8,     "RAM[8]", m_ram[0x8]).formatstr("%04X");
	state_add( CQUESTSND_RAM9,     "RAM[9]", m_ram[0x9]).formatstr("%04X");
	state_add( CQUESTSND_RAMA,     "RAM[A]", m_ram[0xa]).formatstr("%04X");
	state_add( CQUESTSND_RAMB,     "RAM[B]", m_ram[0xb]).formatstr("%04X");
	state_add( CQUESTSND_RAMC,     "RAM[C]", m_ram[0xc]).formatstr("%04X");
	state_add( CQUESTSND_RAMD,     "RAM[D]", m_ram[0xd]).formatstr("%04X");
	state_add( CQUESTSND_RAME,     "RAM[E]", m_ram[0xe]).formatstr("%04X");
	state_add( CQUESTSND_RAMF,     "RAM[F]", m_ram[0xf]).formatstr("%04X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%02X").noshow();

	m_icountptr = &m_icount;
}


void cquestsnd_cpu_device::device_reset()
{
	m_pc = 0;
}


/***************************************************************************
    ROTATE INITIALIZATION AND SHUTDOWN
***************************************************************************/

void cquestrot_cpu_device::device_start()
{
	m_linedata_w.resolve_safe();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	memset(m_ram, 0, sizeof(m_ram));
	m_q = 0;
	m_f = 0;
	m_y = 0;
	m_cflag = 0;
	m_vflag = 0;
	m_pc = 0;
	m_seqcnt = 0;
	m_dsrclatch = 0;
	m_rsrclatch = 0;
	m_dynaddr = 0;
	m_dyndata = 0;
	m_yrlatch = 0;
	m_ydlatch = 0;
	m_dinlatch = 0;
	m_divreg = 0;
	m_linedata = 0;
	m_lineaddr = 0;
	m_prev_dred = 0;
	m_prev_dwrt = 0;
	m_wc = 0;
	m_rc = 0;
	m_clkcnt = 0;

	save_item(NAME(m_ram));
	save_item(NAME(m_q));
	save_item(NAME(m_f));
	save_item(NAME(m_y));
	save_item(NAME(m_cflag));
	save_item(NAME(m_vflag));

	save_item(NAME(m_pc));
	save_item(NAME(m_seqcnt));
	save_item(NAME(m_dsrclatch));
	save_item(NAME(m_rsrclatch));
	save_item(NAME(m_dynaddr));
	save_item(NAME(m_dyndata));
	save_item(NAME(m_yrlatch));
	save_item(NAME(m_ydlatch));
	save_item(NAME(m_dinlatch));
	save_item(NAME(m_divreg));
	save_item(NAME(m_linedata));
	save_item(NAME(m_lineaddr));
	save_item(NAME(m_prev_dred));
	save_item(NAME(m_prev_dwrt));
	save_item(NAME(m_wc));

	save_pointer(NAME(m_dram), 16384);
	save_pointer(NAME(m_sram), 2048);

	state_add( CQUESTROT_PC,        "PC",        m_pc).formatstr("%02X");
	state_add( CQUESTROT_Q,         "Q",         m_q).formatstr("%04X");
	state_add( CQUESTROT_RAM0,      "RAM[0]",    m_ram[0x0]).formatstr("%04X");
	state_add( CQUESTROT_RAM1,      "RAM[1]",    m_ram[0x1]).formatstr("%04X");
	state_add( CQUESTROT_RAM2,      "RAM[2]",    m_ram[0x2]).formatstr("%04X");
	state_add( CQUESTROT_RAM3,      "RAM[3]",    m_ram[0x3]).formatstr("%04X");
	state_add( CQUESTROT_RAM4,      "RAM[4]",    m_ram[0x4]).formatstr("%04X");
	state_add( CQUESTROT_RAM5,      "RAM[5]",    m_ram[0x5]).formatstr("%04X");
	state_add( CQUESTROT_RAM6,      "RAM[6]",    m_ram[0x6]).formatstr("%04X");
	state_add( CQUESTROT_RAM7,      "RAM[7]",    m_ram[0x7]).formatstr("%04X");
	state_add( CQUESTROT_RAM8,      "RAM[8]",    m_ram[0x8]).formatstr("%04X");
	state_add( CQUESTROT_RAM9,      "RAM[9]",    m_ram[0x9]).formatstr("%04X");
	state_add( CQUESTROT_RAMA,      "RAM[A]",    m_ram[0xa]).formatstr("%04X");
	state_add( CQUESTROT_RAMB,      "RAM[B]",    m_ram[0xb]).formatstr("%04X");
	state_add( CQUESTROT_RAMC,      "RAM[C]",    m_ram[0xc]).formatstr("%04X");
	state_add( CQUESTROT_RAMD,      "RAM[D]",    m_ram[0xd]).formatstr("%04X");
	state_add( CQUESTROT_RAME,      "RAM[E]",    m_ram[0xe]).formatstr("%04X");
	state_add( CQUESTROT_RAMF,      "RAM[F]",    m_ram[0xf]).formatstr("%04X");
	state_add( CQUESTROT_SEQCNT,    "SEQCNT",    m_seqcnt).formatstr("%01X");
	state_add( CQUESTROT_DYNADDR,   "DYNADDR",   m_dynaddr).formatstr("%04X");
	state_add( CQUESTROT_DYNDATA,   "DYNDATA",   m_dyndata).formatstr("%04X");
	state_add( CQUESTROT_YRLATCH,   "YRLATCH",   m_yrlatch).formatstr("%04X");
	state_add( CQUESTROT_YDLATCH,   "YDLATCH",   m_ydlatch).formatstr("%04X");
	state_add( CQUESTROT_DINLATCH,  "DINLATCH",  m_dinlatch).formatstr("%04X");
	state_add( CQUESTROT_DSRCLATCH, "DSRCLATCH", m_dsrclatch).formatstr("%04X");
	state_add( CQUESTROT_RSRCLATCH, "RSRCLATCH", m_rsrclatch).formatstr("%04X");
	state_add( CQUESTROT_LDADDR,    "LDADDR",    m_lineaddr).formatstr("%04X");
	state_add( CQUESTROT_LDDATA,    "LDDATA",    m_linedata).formatstr("%04X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%02X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).formatstr("%3s").noshow();

	m_icountptr = &m_icount;
}


void cquestrot_cpu_device::device_reset()
{
	m_pc = 0;
	m_wc = 0;
	m_prev_dred = 1;
	m_prev_dwrt = 1;
}


void cquestrot_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c", m_cflag ? 'C' : '.',
										m_vflag ? 'V' : '.',
										m_f ? '.' : 'Z');
			break;
	}
}


/***************************************************************************
    LINE DRAWER INITIALIZATION AND SHUTDOWN
***************************************************************************/
#define FOREGROUND      0
#define BACKGROUND      1
#define ODD_FIELD       0
#define EVEN_FIELD      1

void cquestlin_cpu_device::device_start()
{
	m_linedata_r.resolve_safe(0);

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	memset(m_ram, 0, sizeof(m_ram));
	m_q = 0;
	m_f = 0;
	m_y = 0;
	m_cflag = 0;
	m_vflag = 0;
	m_pc[0] = m_pc[1] = 0;
	m_seqcnt = 0;
	m_clatch = 0;
	m_zlatch = 0;
	m_xcnt = 0;
	m_ycnt = 0;
	m_sreg = 0;
	m_fadlatch = 0;
	m_badlatch = 0;
	m_sramdlatch = 0;
	m_fglatch = 0;
	m_bglatch = 0;
	m_gt0reg = 0;
	m_fdxreg = 0;
	m_field = 0;
	m_clkcnt = 0;

	save_item(NAME(m_ram));
	save_item(NAME(m_q));
	save_item(NAME(m_f));
	save_item(NAME(m_y));
	save_item(NAME(m_cflag));
	save_item(NAME(m_vflag));

	save_item(NAME(m_pc[0]));
	save_item(NAME(m_pc[1]));
	save_item(NAME(m_seqcnt));
	save_item(NAME(m_clatch));
	save_item(NAME(m_zlatch));
	save_item(NAME(m_xcnt));
	save_item(NAME(m_ycnt));
	save_item(NAME(m_sreg));
	save_item(NAME(m_fadlatch));
	save_item(NAME(m_badlatch));
	save_item(NAME(m_sramdlatch));
	save_item(NAME(m_fglatch));
	save_item(NAME(m_bglatch));
	save_item(NAME(m_gt0reg));
	save_item(NAME(m_fdxreg));
	save_item(NAME(m_field));
	save_item(NAME(m_clkcnt));

	save_pointer(NAME(m_sram), 4096);
	save_pointer(NAME(m_ptr_ram), 1024);
	save_pointer(NAME(m_e_stack), 32768);
	save_pointer(NAME(m_o_stack), 32768);

	state_add( CQUESTLIN_FGPC,     "FPC", m_pc[FOREGROUND]).formatstr("%02X");
	state_add( CQUESTLIN_BGPC,     "BPC", m_pc[BACKGROUND]).formatstr("%02X");
	state_add( CQUESTLIN_Q,        "Q", m_q).formatstr("%04X");
	state_add( CQUESTLIN_RAM0,     "RAM[0]", m_ram[0x0]).formatstr("%04X");
	state_add( CQUESTLIN_RAM1,     "RAM[1]", m_ram[0x1]).formatstr("%04X");
	state_add( CQUESTLIN_RAM2,     "RAM[2]", m_ram[0x2]).formatstr("%04X");
	state_add( CQUESTLIN_RAM3,     "RAM[3]", m_ram[0x3]).formatstr("%04X");
	state_add( CQUESTLIN_RAM4,     "RAM[4]", m_ram[0x4]).formatstr("%04X");
	state_add( CQUESTLIN_RAM5,     "RAM[5]", m_ram[0x5]).formatstr("%04X");
	state_add( CQUESTLIN_RAM6,     "RAM[6]", m_ram[0x6]).formatstr("%04X");
	state_add( CQUESTLIN_RAM7,     "RAM[7]", m_ram[0x7]).formatstr("%04X");
	state_add( CQUESTLIN_RAM8,     "RAM[8]", m_ram[0x8]).formatstr("%04X");
	state_add( CQUESTLIN_RAM9,     "RAM[9]", m_ram[0x9]).formatstr("%04X");
	state_add( CQUESTLIN_RAMA,     "RAM[A]", m_ram[0xa]).formatstr("%04X");
	state_add( CQUESTLIN_RAMB,     "RAM[B]", m_ram[0xb]).formatstr("%04X");
	state_add( CQUESTLIN_RAMC,     "RAM[C]", m_ram[0xc]).formatstr("%04X");
	state_add( CQUESTLIN_RAMD,     "RAM[D]", m_ram[0xd]).formatstr("%04X");
	state_add( CQUESTLIN_RAME,     "RAM[E]", m_ram[0xe]).formatstr("%04X");
	state_add( CQUESTLIN_RAMF,     "RAM[F]", m_ram[0xf]).formatstr("%04X");

	state_add( CQUESTLIN_FADLATCH, "FADDR", m_fadlatch).formatstr("%04X");
	state_add( CQUESTLIN_BADLATCH, "BADDR", m_badlatch).formatstr("%04X");
	state_add( CQUESTLIN_SREG,     "SREG", m_sreg).formatstr("%04X");
	state_add( CQUESTLIN_XCNT,     "XCNT", m_xcnt).formatstr("%03X");
	state_add( CQUESTLIN_YCNT,     "YCNT", m_ycnt).formatstr("%03X");
	state_add( CQUESTLIN_CLATCH,   "CLATCH", m_clatch).formatstr("%04X");
	state_add( CQUESTLIN_ZLATCH,   "ZLATCH", m_zlatch).formatstr("%04X");

	state_add(STATE_GENPC, "curpc", m_curpc).callimport().callexport().formatstr("%02X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).formatstr("%6s").noshow();

	m_icountptr = &m_icount;
}


void cquestlin_cpu_device::device_reset()
{
	m_clkcnt = 0;
	m_pc[FOREGROUND] = 0;
	m_pc[BACKGROUND] = 0x80;
}


void cquestlin_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c|%cG", m_cflag ? 'C' : '.',
											m_vflag ? 'V' : '.',
											m_f ? '.' : 'Z',
											( m_clkcnt & 3 ) ? 'B' : 'F');
			break;
	}
}


/***************************************************************************
    SOUND CORE EXECUTION LOOP
***************************************************************************/

#define SND_PC          (m_pc)
#define SND_DATA_IN     (_ramen ? m_sound_data[m_platch] : m_dinlatch)

enum snd_latch_type
{
	PLTCH = 0,
	DAC = 1,
	ADLATCH = 2,
};

int cquestsnd_cpu_device::do_sndjmp(int jmp)
{
	switch (jmp)
	{
		/* JUMP */ case 0: return 1;
		/* MSB  */ case 2: return m_f & 0x8000 ? 0 : 1;
		/* !MSB */ case 3: return m_f & 0x8000 ? 1 : 0;
		/* ZERO */ case 5: return m_f == 0 ? 0 : 1;
		/* OVR  */ case 6: return m_vflag ? 0 : 1;
		/* LOOP */ case 7: return m_adrcntr & 0x80 ? 0: 1;
	}

	return 0;
}

void cquestsnd_cpu_device::execute_run()
{
	/* Core execution loop */
	do
	{
		/* Decode the instruction */
		UINT64 inst = m_direct->read_qword(SND_PC << 3);
		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 24) & 0xff;
		int b       = (inshig >> 20) & 0xf;
		int a       = (inshig >> 16) & 0xf;
		int ci      = (inshig >> 15) & 1;
		int i5_3    = (inshig >> 12) & 7;
		int _ramen  = (inshig >> 11) & 1;
		int i2_0    = (inshig >> 8) & 7;
		int rtnltch = (inshig >> 7) & 1;
		int jmp     = (inshig >> 4) & 7;
		int inca    = (inshig >> 3) & 1;
		int i8_6    = (inshig >> 0) & 7;
		int _ipram  = (inslow >> 31) & 1;
		int _ipwrt  = (inslow >> 30) & 1;
		int latch   = (inslow >> 28) & 3;
		int rtn     = (inslow >> 27) & 1;
		int _rin    = (inslow >> 26) & 1;

		debugger_instruction_hook(this, m_pc);

		/* Don't think this matters, but just in case */
		if (rtn)
			t = m_rtnlatch;

		/* Handle the AM2901 ALU instruction */
		{
			UINT16 r = 0;
			UINT16 s = 0;

			UINT32 res = 0;
			UINT32 cflag = 0;
			UINT32 vflag = 0;

			/* Determine the ALU sources */
			switch (i2_0)
			{
				case AQ: r = m_ram[a]; s = m_q;      break;
				case AB: r = m_ram[a]; s = m_ram[b]; break;
				case ZQ: r = 0;                s = m_q;      break;
				case ZB: r = 0;                s = m_ram[b]; break;
				case ZA: r = 0;                s = m_ram[a]; break;
				case DA: r = SND_DATA_IN;      s = m_ram[a]; break;
				case DQ: r = SND_DATA_IN;      s = m_q;      break;
				case DZ: r = SND_DATA_IN;      s = 0;                break;
			}

			/* Perform the ALU operation */
			switch (i5_3)
			{
				case ADD:
					res = r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBR:
					res = ~r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((s & 0x7fff) + (~r & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBS:
					res = r + ~s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (~s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case OR:
					res = r | s;
					break;
				case AND:
					res = r & s;
					break;
				case NOTRS:
					res = ~r & s;
					break;
				case EXOR:
					res = r ^ s;
					break;
				case EXNOR:
					res = ~(r ^ s);
					break;
			}

			m_f = res;
			m_cflag = cflag;
			m_vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					m_q = m_f;
					m_y = m_f;
					break;
				case NOP:
					m_y = m_f;
					break;
				case RAMA:
					m_y = m_ram[a];
					m_ram[b] = m_f;
					break;
				case RAMF:
					m_ram[b] = m_f;
					m_y = m_f;
					break;
				case RAMQD:
				{
					UINT16 qin;

					m_ram[b] = (_rin ? 0 : 0x8000) | (m_f >> 1);
					m_q >>= 1;
					m_y = m_f;

					/* When right shifting Q, we need to OR in a value */
					qin = (((m_y >> 15) ^ (m_y >> 1)) & 1) ? 0 : 0x8000;

					m_q |= qin;
					break;
				}
				case RAMD:
					m_ram[b] = (_rin ? 0 : 0x8000) | (m_f >> 1);
					m_y = m_f;
					break;
				case RAMQU:
					m_ram[b] = (m_f << 1) | (_rin ? 0 : 0x0001);
					m_q <<= 1;
					m_y = m_f;
					break;
				case RAMU:
					m_ram[b] = (m_f << 1) | (_rin ? 0 : 0x0001);
					m_y = m_f;
					break;
			}
		}

		/* Now handle any SRAM accesses from the previous cycle */
		if (!m_prev_ipram)
		{
			UINT16 addr = m_adrlatch | (m_adrcntr & 0x7f);

			if (!m_prev_ipwrt)
			m_sram[addr] = m_ramwlatch;
			else
			m_dinlatch = m_sram[addr];
		}

		/* Handle latches */
		if (latch == PLTCH)
		{
			m_platch = ((t & 3) << 9) | ((m_y >> 6) & 0x1ff);
		}
		else if (latch == DAC)
		{
			m_dac_w((m_y & 0xfff0) | ((m_adrcntr >> 3) & 0xf));
		}
		else if (latch == ADLATCH)
		{
			/* Load the SRAM address counter - this value is instantly loaded */
			m_adrcntr = m_y & 0x7f;

			/* Also load the SRAM address latch */
			m_adrlatch = m_y & 0x780;
		}

		/* Check for jump/return */
		if ( do_sndjmp(jmp) )
			m_pc = rtn ? m_rtnlatch : t;
		else
			m_pc++;

		/* Load the return latch? (Obviously a load and a ret in the same cycle are invalid) */
		if (rtnltch)
			m_rtnlatch = t;

		/* Only increment the sound counter if not loading */
		if (inca && latch != ADLATCH)
			m_adrcntr++;

		/* Latch data for a RAM write (do actual write on the next cycle) */
		if (!_ipwrt)
			m_ramwlatch = m_y;

		/* Save level sensitive bits */
		m_prev_ipram = _ipram;
		m_prev_ipwrt = _ipwrt;

		m_icount--;
	} while (m_icount > 0);
}


/***************************************************************************
    ROTATE CORE EXECUTION LOOP
***************************************************************************/

#define ROT_PC          (m_pc & 0x1ff)

enum rot_spf
{
	SPF_UNUSED0 = 0,
	SPF_UNUSED1 = 1,
	SPF_OP      = 2,
	SPF_RET     = 3,
	SPF_SQLTCH  = 4,
	SPF_SWRT    = 5,
	SPF_DIV     = 6,
	SPF_MULT    = 7,
	SPF_DRED    = 8,
	SPF_DWRT    = 9,
};

enum rot_yout
{
	YOUT_UNUSED0 = 0,
	YOUT_UNUSED1 = 1,
	YOUT_Y2LDA   = 2,
	YOUT_Y2LDD   = 3,
	YOUT_Y2DAD   = 4,
	YOUT_Y2DYN   = 5,
	YOUT_Y2R     = 6,
	YOUT_Y2D     = 7,
};

/* Sync is asserted for the duration of every fourth cycle */
/* The Dynamic RAM latch clocks in a value at the end of this cycle */
/* So CPU waits for sync before reading from DRAM */

int cquestrot_cpu_device::do_rotjmp(int jmp)
{
	int ret = 0;

	switch (jmp & 7)
	{
		/*        */ case 0: ret = 0;                         break;
		/* SEQ    */ case 1: ret = (m_seqcnt == 0xf); break;
		/* CAROUT */ case 2: ret = m_cflag;           break;
		/* SYNC   */ case 3: ret = !(m_clkcnt & 0x3); break;
		/* LDWAIT */ case 4: ret = 0;                         break;
		/* MSB    */ case 5: ret = BIT(m_f, 15);      break;
		/* >=1    */ case 6: ret = (!_BIT(m_f, 15) && !(m_f == 0)); break;
		/* ZERO   */ case 7: ret = (m_f == 0);        break;
	}

	return !(!ret ^ BIT(jmp, 3));
}


#define ROT_SRAM_ADDRESS    ((m_dsrclatch & 2) ? m_yrlatch : (m_rsrclatch | 0x700))


void cquestrot_cpu_device::execute_run()
{
	/* Core execution loop */
	do
	{
		/* Decode the instruction */
		UINT64 inst = m_direct->read_qword(ROT_PC << 3);

		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 20) & 0xfff;
		int jmp     = (inshig >> 16) & 0xf;
		int spf     = (inshig >> 12) & 0xf;
		int rsrc    = (inshig >> 11) & 0x1;
		int yout    = (inshig >> 8) & 0x7;
		int sel     = (inshig >> 6) & 0x3;
		int dsrc    = (inshig >> 4) & 0x3;
		int b       = (inshig >> 0) & 0xf;
		int a       = (inslow >> 28) & 0xf;
		int i8_6    = (inslow >> 24) & 0x7;
		int ci      = (inslow >> 23) & 0x1;
		int i5_3    = (inslow >> 20) & 0x7;
		int _sex    = (inslow >> 19) & 0x1;
		int i2_0    = (inslow >> 16) & 0x7;

		int dsrclatch;
		UINT16 data_in = 0xffff;

		debugger_instruction_hook(this, ROT_PC);

		/* Handle DRAM accesses - I ought to check this... */
		if (!(m_clkcnt & 3))
		{
			if (m_wc)
			{
				m_wc = 0;
				m_dram[m_dynaddr & 0x3fff] = m_dyndata;
			}
			if (m_rc)
			{
				m_rc = 0;
				m_dinlatch = m_dram[m_dynaddr & 0x3fff];
			}
		}

		/* Flag pending DRAM accesses */
		if (!m_prev_dwrt)
			m_wc = 1;
		else if (!m_prev_dred)
			m_rc = 1;

		/* What's on the D-Bus? */
		if (~m_dsrclatch & 0x10)
			data_in = m_dinlatch;
		else if (~m_dsrclatch & 0x20)
			data_in = m_sram[ROT_SRAM_ADDRESS];
		else if (~m_dsrclatch & 0x40)
			data_in = m_ydlatch;
		else if (~m_dsrclatch & 0x80)
			data_in = t & 0xfff;

		/* What's on the T-Bus? */
		if ((spf == SPF_RET) && (m_dsrclatch & 0x80))
			t = data_in;
		else if (spf == SPF_OP)
			t = (t & ~0xf) | (data_in >> 12);


		if (~m_dsrclatch & 1)
			m_sram[ROT_SRAM_ADDRESS] = data_in;


		/* Sign extend ALU input? */
		if (!_sex)
			data_in = (data_in & ~0xf000) | ((data_in & 0x800) ? 0xf000 : 0);

		/* No do the ALU operation */
		{
			UINT16 r = 0;
			UINT16 s = 0;

			UINT32 res = 0;
			UINT32 cflag = 0;
			UINT32 vflag = 0;

			/* First, determine correct I1 bit */
			if ((spf == SPF_MULT) && !_BIT(m_q, 0))
				i2_0 |= 2;

			/* Determine the ALU sources */
			switch (i2_0)
			{
				case 0: r = m_ram[a]; s = m_q;      break;
				case 1: r = m_ram[a]; s = m_ram[b]; break;
				case 2: r = 0;                s = m_q;      break;
				case 3: r = 0;                s = m_ram[b]; break;
				case 4: r = 0;                s = m_ram[a]; break;
				case 5: r = data_in;          s = m_ram[a]; break;
				case 6: r = data_in;          s = m_q;      break;
				case 7: r = data_in;          s = 0;                break;
			}

			/* Next, determine the I3 and carry bits */
			if ((spf == SPF_DIV) && m_divreg)
			{
				i5_3 |= 1;
				ci = 1;
			}

			/* Perform the ALU operation */
			switch (i5_3)
			{
				case ADD:
					res = r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBR:
					res = ~r + s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((s & 0x7fff) + (~r & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case SUBS:
					res = r + ~s + ci;
					cflag = (res >> 16) & 1;
					vflag = (((r & 0x7fff) + (~s & 0x7fff) + ci) >> 15) ^ cflag;
					break;
				case OR:
					res = r | s;
					break;
				case AND:
					res = r & s;
					break;
				case NOTRS:
					res = ~r & s;
					break;
				case EXOR:
					res = r ^ s;
					break;
				case EXNOR:
					res = ~(r ^ s);
					break;
			}

			m_f = res;
			m_cflag = cflag;
			m_vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					m_q = m_f;
					m_y = m_f;
					break;
				case NOP:
					m_y = m_f;
					break;
				case RAMA:
					m_y = m_ram[a];
					m_ram[b] = m_f;
					break;
				case RAMF:
					m_ram[b] = m_f;
					m_y = m_f;
					break;
				case RAMQD:
				{
					UINT16 q0 = m_q & 1;
					UINT16 r0 = m_f & 1;
					UINT16 q15 = 0;
					UINT16 r15 = 0;

					/* Determine Q15 and RAM15 */
					switch (sel)
					{
						case 0: q15 = r15 = 0;
								break;
						case 1: q15 = r15 = 0x8000;
								break;
						case 2: q15 = q0 << 15;
								r15 = r0 << 15;
								break;
						case 3: q15 = r0 << 15;
								r15 = (m_vflag ^ BIT(m_f, 15)) << 15;
								break;
					}

					m_ram[b] = r15 | (m_f >> 1);
					m_q = q15 | (m_q >> 1);
					m_y = m_f;
					break;
				}
				case RAMD:
				{
					UINT16 r0 = m_f & 1;
					UINT16 r15 = 0;

					switch (sel)
					{
						case 0: r15 = 0;        break;
						case 1: r15 = 0x8000;   break;
						case 2: r15 = r0 << 15; break;
						case 3:
							r15 = (m_vflag ^ BIT(m_f, 15)) << 15;
							break;
					}

					m_ram[b] = r15 | (m_f >> 1);
					m_y = m_f;
					break;
				}
				case RAMQU:
				{
					UINT16 q15 = BIT(m_q, 15);
					UINT16 r15 = BIT(m_f, 15);
					UINT16 q0 = 0;
					UINT16 r0 = 0;

					switch (sel)
					{
						case 0: q0 = 0; r0 = 0;     break;
						case 1: q0 = 1; r0 = 1;     break;
						case 2: q0 = q15; r0 = r15; break;
						case 3:
						{
							q0 = (spf == SPF_DIV) && !BIT(m_f, 15);
							r0 = q15;
							break;
						}
					}

					m_ram[b] = (m_f << 1) | r0;
					m_q = (m_q << 1) | q0;
					m_y = m_f;
					break;
				}
				case RAMU:
				{
					UINT16 q15 = BIT(m_q, 15);
					UINT16 r15 = BIT(m_f, 15);
					UINT16 r0 = 0;

					switch (sel)
					{
						case 0: r0 = 0;     break;
						case 1: r0 = 1;     break;
						case 2: r0 = r15;   break;
						case 3: r0 = q15;   break;
					}

					m_ram[b] = (m_f << 1) | r0;
					m_y = m_f;
					break;
				}
			}
		}

		/* Check for jump */
		if ( do_rotjmp(jmp) )
			m_pc = t;
		else
			m_pc = (m_pc + 1) & 0xfff;

		/* Rising edge; update the sequence counter */
		if (spf == SPF_SQLTCH)
			m_seqcnt = t & 0xf;
		else if ( (spf == SPF_MULT) || (spf == SPF_DIV) )
			m_seqcnt = (m_seqcnt + 1) & 0xf;

		/* Rising edge; write data source reg */
		dsrclatch =
				(~(0x10 << dsrc) & 0xf0)
				| (rsrc ? 0x04 : 0x02)
				| !(spf == SPF_SWRT);

		/* R-latch is written on rising edge of dsrclatch bit 2 */
		if (!_BIT(m_dsrclatch, 2) && _BIT(dsrclatch, 2))
			m_rsrclatch = t & 0xff;

		m_dsrclatch = dsrclatch;

		/* Handle latching on rising edge */
		switch (yout)
		{
			case YOUT_Y2LDA:
			{
				m_lineaddr = m_y & 0xfff;
				break;
			}
			case YOUT_Y2LDD:
			{
				m_linedata = ((t & 0xf) << 12) | (m_y & 0xfff);
				m_linedata_w( m_lineaddr, m_linedata, 0xffff );
				break;
			}
			case YOUT_Y2DAD: m_dynaddr = m_y & 0x3fff;  break;
			case YOUT_Y2DYN: m_dyndata = m_y & 0xffff;  break;
			case YOUT_Y2R:   m_yrlatch = m_y & 0x7ff;   break;
			case YOUT_Y2D:   m_ydlatch = m_y;           break;
		}

		/* Clock in the divide register */
		m_divreg = (spf == SPF_DIV) && !_BIT(m_f, 15);

		/* DRAM accessing */
		m_prev_dred = !(spf == SPF_DRED);
		m_prev_dwrt = !(spf == SPF_DWRT);

		m_clkcnt++;
		m_icount--;
	} while (m_icount > 0);
}


/***************************************************************************
    LINE DRAWER CORE EXECUTION LOOP
***************************************************************************/

enum line_spf
{
	LSPF_UNUSED  = 0,
	LSPF_FSTOP   = 1,
	LSPF_SREG    = 2,
	LSPF_FSTRT   = 3,
	LSPF_PWRT    = 4,
	LSPF_MULT    = 5,
	LSPF_LSTOP   = 6,
	LSPF_BRES    = 7,
};

enum line_latch
{
	LLATCH_UNUSED   = 0,
	LLATCH_SEQLATCH = 1,
	LLATCH_XLATCH   = 2,
	LLATCH_YLATCH   = 3,
	LLATCH_BADLATCH = 4,
	LLATCH_FADLATCH = 5,
	LLATCH_CLATCH   = 6,
	LLATCH_ZLATCH   = 7,
};

enum sreg_bits
{
	SREG_E0     = 0,
	SREG_DX_DY  = 1,
	SREG_DY     = 2,
	SREG_DX     = 3,
	SREG_LE0    = 4,
	SREG_LDX_DY = 5,
	SREG_LDY    = 6,
	SREG_LDX    = 7,
};

int cquestlin_cpu_device::do_linjmp(int jmp)
{
	int ret = 0;

	switch (jmp & 7)
	{
		/*        */ case 0: ret = 0; break;
		/* MSB    */ case 1: ret = BIT(m_f, 11); break;
		/* SEQ    */ case 2: ret = (m_seqcnt == 0xfff); break;
		/* >0     */ case 3: ret = !(m_f == 0) && !_BIT(m_f, 11); break;
		/* CAROUT */ case 4: ret = (m_cflag); break;
		/* ZERO   */ case 5: ret = (m_f == 0); break;
	}

	return !(!ret ^ BIT(jmp, 3));
}


void cquestlin_cpu_device::cubeqcpu_swap_line_banks()
{
	m_field = m_field ^ 1;
}


void cquestlin_cpu_device::cubeqcpu_clear_stack()
{
	memset(&m_ptr_ram[m_field * 256], 0, 256);
}


UINT8 cquestlin_cpu_device::cubeqcpu_get_ptr_ram_val(int i)
{
	return m_ptr_ram[((m_field^1) * 256) + i];
}


UINT32* cquestlin_cpu_device::cubeqcpu_get_stack_ram()
{
	if (m_field != ODD_FIELD)
		return m_o_stack;
	else
		return m_e_stack;
}


void cquestlin_cpu_device::execute_run()
{
#define LINE_PC ((m_pc[prog] & 0x7f) | ((prog == BACKGROUND) ? 0x80 : 0))

	UINT32  *stack_ram;
	UINT8   *ptr_ram;

	/* Check the field and set the stack/pointer RAM pointers appropriately */
	if (m_field == ODD_FIELD)
	{
		stack_ram = m_o_stack;
		ptr_ram = &m_ptr_ram[0];
	}
	else
	{
		stack_ram = m_e_stack;
		ptr_ram = &m_ptr_ram[0x100];
	}

	/* Core execution loop */
	do
	{
		/* Are we executing the foreground or backgroud program? */
		int prog = (m_clkcnt & 3) ? BACKGROUND : FOREGROUND;

		m_curpc = LINE_PC;
		UINT64 inst = m_direct->read_qword(LINE_PC << 3);

		UINT32 inslow = inst & 0xffffffff;
		UINT32 inshig = inst >> 32;

		int t       = (inshig >> 24) & 0xff;
		int jmp     = (inshig >> 20) & 0xf;
		int latch   = (inshig >> 16) & 0x7;
		int op      = (inshig >> 15) & 0x1;
		int spf     = (inshig >> 12) & 0x7;
		int b       = (inshig >> 8) & 0xf;
		int a       = (inshig >> 4) & 0xf;
		int i8_6    = (inshig >> 0) & 0x7;
		int ci      = (inslow >> 31) & 0x1;
		int i5_3    = (inslow >> 28) & 0x7;
		int _pbcs   = (inslow >> 27) & 0x1;
		int i2_0    = (inslow >> 24) & 0x7;

		UINT16  data_in = 0;

		debugger_instruction_hook(this, m_pc[prog]);

		/* Handle accesses to and from shared SRAM */
		if (prog == FOREGROUND)
		{
			if (!_BIT(m_fglatch, 5))
				data_in = m_sram[m_fadlatch];
			else
				data_in = m_linedata_r();
		}
		else
		{
			if (!_BIT(m_bglatch, 4))
				m_sram[m_badlatch] = m_sramdlatch;
			else if (_BIT(m_bglatch, 2))
				data_in = m_sram[m_badlatch];
			else
				data_in = m_linedata_r();
		}

		/* Handle a write to stack RAM (/DOWRT) */
		if ((m_clkcnt & 3) == 1)
		{
			if (_BIT(m_fglatch, 4) && (m_ycnt < 256))
			{
				/* 20-bit words */
				UINT32 data;
				UINT16 h = m_xcnt;
				UINT8 v = m_ycnt & 0xff;

				/* Clamp H between 0 and 319 */
				if (h >= 320)
					h = (h & 0x800) ? 0 : 319;

				/* Stack word type depends on STOP/#START bit */
				if ( _BIT(m_fglatch, 3) )
					data = (0 << 19) | (h << 8) | m_zlatch;
				else
					data = (1 << 19) | ((m_clatch & 0x100) << 9) | (h << 8) | (m_clatch & 0xff);

				stack_ram[(v << 7) | (ptr_ram[v] & 0x7f)] = data;

				/* Also increment the pointer RAM entry. Note that it cannot exceed 128 */
				ptr_ram[v] = (ptr_ram[v] + 1) & 0x7f;
			}
		}

		/* Override T3-0? */
		if (op)
			t = (t & ~0xf) | (data_in >> 12);

		/* Determine the correct I1 bit  */
		if ((spf == LSPF_MULT) && !_BIT(m_q, 0))
			i2_0 |= 2;

		/* Determine A0 (BRESA0) */
		if ((prog == FOREGROUND) && !_BIT(m_fglatch, 2))
			a |= m_gt0reg;

		/* Now do the ALU operation */
		{
			UINT16 r = 0;
			UINT16 s = 0;

			UINT16 res = 0;
			UINT32 cflag = 0;
			UINT32 vflag = 0;

			/* Determine the ALU sources */
			switch (i2_0)
			{
				case 0: r = m_ram[a];   s = m_q;      break;
				case 1: r = m_ram[a];   s = m_ram[b]; break;
				case 2: r = 0;                  s = m_q;      break;
				case 3: r = 0;                  s = m_ram[b]; break;
				case 4: r = 0;                  s = m_ram[a]; break;
				case 5: r = data_in;            s = m_ram[a]; break;
				case 6: r = data_in;            s = m_q;      break;
				case 7: r = data_in;            s = 0;                break;
			}

			/* 12-bits */
			r &= 0xfff;
			s &= 0xfff;

			/* Perform the 12-bit ALU operation */
			switch (i5_3)
			{
				case ADD:
					res = r + s + ci;
					cflag = (res >> 12) & 1;
					vflag = (((r & 0x7ff) + (s & 0x7ff) + ci) >> 11) ^ cflag;
					break;
				case SUBR:
					res = (r ^ 0x0FFF) + s + ci;
					cflag = (res >> 12) & 1;
					vflag = (((s & 0x7ff) + (~r & 0x7ff) + ci) >> 11) ^ cflag;
					break;
				case SUBS:
					res = r + (s ^ 0x0FFF) + ci;
					cflag = (res >> 12) & 1;
					vflag = (((r & 0x7ff) + (~s & 0x7ff) + ci) >> 11) ^ cflag;
					break;
				case OR:
					res = r | s;
					break;
				case AND:
					res = r & s;
					break;
				case NOTRS:
					res = ~r & s;
					break;
				case EXOR:
					res = r ^ s;
					break;
				case EXNOR:
					res = ~(r ^ s);
					break;
			}

			m_f = res & 0xfff;
			m_cflag = cflag;
			m_vflag = vflag;

			switch (i8_6)
			{
				case QREG:
					m_q = m_f;
					m_y = m_f;
					break;
				case NOP:
					m_y = m_f;
					break;
				case RAMA:
					m_y = m_ram[a];
					m_ram[b] = m_f;
					break;
				case RAMF:
					m_ram[b] = m_f;
					m_y = m_f;
					break;
				case RAMQD:
				{
					UINT16 r11 = (BIT(m_f, 11) ^ m_vflag) ? 0x800 : 0;
					UINT16 q11 = (prog == BACKGROUND) ? 0x800 : 0;

					m_ram[b] = r11 | (m_f >> 1);
					m_q = q11 | (m_q >> 1);
					m_y = m_f;
					break;
				}
				case RAMD:
				{
					UINT16 r11 = (BIT(m_f, 11) ^ m_vflag) ? 0x800 : 0;

					m_ram[b] = r11 | (m_f >> 1);
					m_y = m_f;
					break;
				}
				case RAMQU:
				{
					/* Determine shift inputs */
					UINT16 r0 = (prog == BACKGROUND);

					/* This should never happen - Q0 will be invalid */
					m_ram[b] = (m_f << 1) | r0;
					m_q = (m_q << 1) | 0;
					m_y = m_f;
					break;
				}
				case RAMU:
				{
					UINT16 r0 = (prog == BACKGROUND);

					m_ram[b] = (m_f << 1) | r0;
					m_y = m_f;
					break;
				}
			}
		}

		/* Adjust program counter */
		if ( do_linjmp(jmp) )
			m_pc[prog] = t & 0x7f;
		else
			m_pc[prog] = (m_pc[prog] + 1) & 0x7f;

		if (prog == BACKGROUND)
			m_pc[prog] |= 0x80;
		else
		{
			/* Handle events that happen during FG execution */
			if (latch == LLATCH_XLATCH)
				m_xcnt = m_y & 0xfff;
			else
			{
				int _xcet;
				int mux_sel = (BIT(m_sreg, SREG_DX_DY) << 1) | (BIT(m_sreg, SREG_DX) ^ BIT(m_sreg, SREG_DY));

				if (mux_sel == 0)
					_xcet = !(spf == LSPF_BRES);
				else if (mux_sel == 1)
					_xcet = _BIT(m_fglatch, 1);
				else if (mux_sel == 2)
					_xcet = !(m_gt0reg && (spf == LSPF_BRES));
				else
					_xcet = _BIT(m_fglatch, 0);

				if (!_xcet)
					m_xcnt = (m_xcnt + (_BIT(m_sreg, SREG_DX) ? 1 : -1)) & 0xfff;
			}

			if (latch == LLATCH_YLATCH)
				m_ycnt = m_y & 0xfff;
			else
			{
				int _ycet;
				int mux_sel = (BIT(m_sreg, SREG_DX_DY) << 1) | (BIT(m_sreg, SREG_DX) ^ BIT(m_sreg, SREG_DY));

				if (mux_sel == 0)
					_ycet = !(m_gt0reg && (spf == LSPF_BRES));
				else if (mux_sel == 1)
					_ycet = _BIT(m_fglatch, 0);
				else if (mux_sel == 2)
					_ycet = !(spf == LSPF_BRES);
				else
					_ycet = _BIT(m_fglatch, 1);

				if (!_ycet)
					m_ycnt = (m_ycnt + (_BIT(m_sreg, SREG_DY) ? 1 : -1)) & 0xfff;
			}
		}

		if (latch == LLATCH_CLATCH)
			m_clatch = m_y & 0x1ff;
		else if (latch == LLATCH_ZLATCH)
			m_zlatch = m_y & 0xff;
		else if (latch == LLATCH_FADLATCH)
			m_fadlatch = m_y & 0xfff;
		else if (latch == LLATCH_BADLATCH)
			m_badlatch = m_y & 0xfff;

		/* What about the SRAM dlatch? */
		if ( !_BIT(m_bglatch, 5) )
			m_sramdlatch = ((t & 0xf) << 12) | (m_y & 0x0fff);

		/* BG and FG latches */
		if (prog == FOREGROUND)
		{
			int mux_sel = (!(spf == LSPF_FSTOP) << 1) | !(spf == LSPF_LSTOP);
			int dowrt;
			int start_stop;

			/* Handle the stack write and start/stop mux */
			if (mux_sel == 0)
			{
				dowrt = 0;
				start_stop = 0;
			}
			else if (mux_sel == 1)
			{
				dowrt = m_fdxreg ^ BIT(m_sreg, SREG_DX);
				start_stop = m_fdxreg;
			}
			else if (mux_sel == 2)
			{
				dowrt = BIT(m_sreg, SREG_LDX) ^ BIT(m_sreg, SREG_DX);
				start_stop = BIT(m_sreg, SREG_DX);
			}
			else
			{
				dowrt = (spf == LSPF_BRES) && (_BIT(m_sreg, SREG_DX_DY) || m_gt0reg);
				start_stop = BIT(m_sreg, SREG_DY);
			}

			m_fglatch =
					(!(latch == LLATCH_FADLATCH) << 5)
					| (dowrt << 4)
					| (start_stop << 3)
					| (_pbcs << 2)
					| (!(spf == LSPF_BRES) << 1)
					| !(m_gt0reg && (spf == LSPF_BRES));
		}
		else
		{
			int _lpwrt = BIT(m_bglatch, 5);

			m_bglatch =
					(!(spf == LSPF_PWRT) << 5)
					| (_lpwrt << 4)
					| ((!_lpwrt || (!(spf == LSPF_PWRT) && (latch == LLATCH_BADLATCH))) << 2);
		}

		/* Clock-in another bit into the sign bit shifter? */
		if (spf == LSPF_SREG)
		{
			/* The sign bit is inverted */
			m_sreg = (m_sreg << 1) | !BIT(m_f, 11);

			/* Also latch the >0 reg */
			m_gt0reg = !(m_f == 0) && !_BIT(m_f, 11);
		}
		else if (spf == LSPF_FSTRT)
		{
			m_fdxreg = BIT(m_sreg, 3);
		}

		/* Load or increment sequence counter? */
		if (latch == LLATCH_SEQLATCH)
		{
			m_seqcnt = m_y & 0xfff;
		}
		else if (spf == LSPF_BRES)
		{
			m_seqcnt = (m_seqcnt + 1) & 0xfff;

			/* Also latch the >0 reg */
			m_gt0reg = !(m_f == 0) && !_BIT(m_f, 11);
		}

		m_icount--;
		m_clkcnt++;
	} while (m_icount > 0);
}
