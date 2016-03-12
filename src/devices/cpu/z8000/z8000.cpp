// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller,Ernesto Corvi
/*****************************************************************************
 *
 *   z8000.c
 *   Portable Z8000(2) emulator
 *   Z8000 MAME interface
 *
 *     TODO:
 *     - make the z8001 opcodes to be dynamic (i.e. to take segmented mode flag into account and use the non-segmented mode)
 *     - dissassembler doesn't work at all with the z8001
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "debug/debugcon.h"
#include "z8000.h"

#define VERBOSE 0


#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


extern int z8k_segm;
extern int z8k_segm_mode;
extern void z8k_disass_mode(running_machine &machine, int ref, int params, const char *param[]);

#include "z8000cpu.h"

const device_type Z8001 = &device_creator<z8001_device>;
const device_type Z8002 = &device_creator<z8002_device>;


z8002_device::z8002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, Z8002, "Z8002", tag, owner, clock, "z8002", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 16, 16, 0)
	, m_io_config("io", ENDIANNESS_BIG, 8, 16, 0)
	, m_mo_out(*this), m_ppc(0), m_pc(0), m_psapseg(0), m_psapoff(0), m_fcw(0), m_refresh(0), m_nspseg(0), m_nspoff(0), m_irq_req(0), m_irq_vec(0), m_op_valid(0), m_nmi_state(0), m_mi(0), m_program(nullptr), m_data(nullptr), m_direct(nullptr), m_io(nullptr), m_icount(0)
		, m_vector_mult(1)
{
}


z8002_device::z8002_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 16, 20, 0)
	, m_io_config("io", ENDIANNESS_BIG, 16, 16, 0)
	, m_mo_out(*this), m_ppc(0), m_pc(0), m_psapseg(0), m_psapoff(0), m_fcw(0), m_refresh(0), m_nspseg(0), m_nspoff(0), m_irq_req(0), m_irq_vec(0), m_op_valid(0), m_nmi_state(0), m_mi(0), m_program(nullptr), m_data(nullptr), m_direct(nullptr), m_io(nullptr), m_icount(0)
	, m_vector_mult(2)
{
}


z8001_device::z8001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z8002_device(mconfig, Z8001, "Zilog Z8001", tag, owner, clock, "z8001", __FILE__)
	, m_data_config("data", ENDIANNESS_BIG, 16, 20, 0)
{
}


offs_t z8002_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( z8000 );
	return CPU_DISASSEMBLE_NAME(z8000)(this, buffer, pc, oprom, opram, options);
}


/* opcode execution table */
Z8000_exec *z8000_exec = nullptr;

/* zero, sign and parity flags for logical byte operations */
static UINT8 z8000_zsp[256];


int z8002_device::segmented_mode()
{
	return 0;
}

int z8001_device::segmented_mode()
{
	return (m_fcw & F_SEG) ? 1 : 0;
}

UINT32 z8002_device::addr_add(UINT32 addr, UINT32 addend)
{
	return (addr & 0xffff0000) | ((addr + addend) & 0xffff);
}

UINT32 z8002_device::addr_sub(UINT32 addr, UINT32 subtrahend)
{
	return (addr & 0xffff0000) | ((addr - subtrahend) & 0xffff);
}

/* conversion table for Z8000 DAB opcode */
#include "z8000dab.h"

UINT16 z8002_device::RDOP()
{
	UINT16 res = m_program->read_word(m_pc);
	m_pc += 2;
	return res;
}

UINT32 z8002_device::get_operand(int opnum)
{
	int i;

	for (i = 0; i < opnum; i++)
	{
		assert (m_op_valid & (1 << i));
	}

	if (! (m_op_valid & (1 << opnum)))
	{
		m_op[opnum] = m_program->read_word(m_pc);
		m_pc += 2;
		m_op_valid |= (1 << opnum);
	}
	return m_op[opnum];
}

UINT32 z8002_device::get_addr_operand(int opnum)
{
	int i;

	for (i = 0; i < opnum; i++)
	{
		assert (m_op_valid & (1 << i));
	}

	if (! (m_op_valid & (1 << opnum)))
	{
		UINT32 seg = m_program->read_word(m_pc);
		m_pc += 2;
		if (segmented_mode())
		{
			if (seg & 0x8000)
			{
				m_op[opnum] = ((seg & 0x7f00) << 8) | m_program->read_word(m_pc);
				m_pc += 2;
			}
			else
				m_op[opnum] = ((seg & 0x7f00) << 8) | (seg & 0xff);
		}
		else
			m_op[opnum] = seg;
		m_op_valid |= (1 << opnum);
	}
	return m_op[opnum];
}

UINT32 z8002_device::get_raw_addr_operand(int opnum)
{
	int i;

	for (i = 0; i < opnum; i++)
	{
		assert (m_op_valid & (1 << i));
	}

	if (! (m_op_valid & (1 << opnum)))
	{
		UINT32 seg = m_program->read_word(m_pc);
		m_pc += 2;
		if (segmented_mode())
		{
			if (seg & 0x8000)
			{
				m_op[opnum] = (seg << 16) | m_program->read_word(m_pc);
				m_pc += 2;
			}
			else
				m_op[opnum] = (seg << 16) | (seg & 0xff);
		}
		else
			m_op[opnum] = seg;
		m_op_valid |= (1 << opnum);
	}
	return m_op[opnum];
}

UINT32 z8002_device::adjust_addr_for_nonseg_mode(UINT32 addr)
{
	return addr;
}

UINT32 z8001_device::adjust_addr_for_nonseg_mode(UINT32 addr)
{
	if (!(m_fcw & F_SEG))
	{
		return (addr & 0xffff) | (m_pc & 0x7f0000);
	}
	else
	{
		return addr;
	}
}

UINT8 z8002_device::RDMEM_B(address_spacenum spacenum, UINT32 addr)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	if (spacenum == AS_PROGRAM)
		return m_program->read_byte(addr);
	else
		return m_data->read_byte(addr);
}

UINT16 z8002_device::RDMEM_W(address_spacenum spacenum, UINT32 addr)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	/* hack for m20 driver: BIOS accesses 0x7f0000 and expects a segmentation violation */
	if (addr >= 0x7f0000) {
		m_irq_req = Z8000_SEGTRAP;
		return 0xffff;
	}
	if (spacenum == AS_PROGRAM)
		return m_program->read_word(addr);
	else
		return m_data->read_word(addr);
}

UINT32 z8002_device::RDMEM_L(address_spacenum spacenum, UINT32 addr)
{
	UINT32 result;
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	if (spacenum == AS_PROGRAM)
	{
		result = m_program->read_word(addr) << 16;
		return result + m_program->read_word(addr_add(addr, 2));
	}
	else
	{
		result = m_data->read_word(addr) << 16;
		return result + m_data->read_word(addr_add(addr, 2));
	}
}

void z8002_device::WRMEM_B(address_spacenum spacenum, UINT32 addr, UINT8 value)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	if (spacenum == AS_PROGRAM)
		m_program->write_byte(addr, value);
	else
		m_data->write_byte(addr, value);
}

void z8002_device::WRMEM_W(address_spacenum spacenum, UINT32 addr, UINT16 value)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	if (spacenum == AS_PROGRAM)
		m_program->write_word(addr, value);
	else
		m_data->write_word(addr, value);
}

void z8002_device::WRMEM_L(address_spacenum spacenum, UINT32 addr, UINT32 value)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	if (spacenum == AS_PROGRAM)
	{
		m_program->write_word(addr, value >> 16);
		m_program->write_word(addr_add(addr, 2), value & 0xffff);
	}
	else
	{
		m_data->write_word(addr, value >> 16);
		m_data->write_word(addr_add(addr, 2), value & 0xffff);
	}
}

UINT8 z8002_device::RDPORT_B(int mode, UINT16 addr)
{
	if(mode == 0)
	{
		return m_io->read_byte(addr);
	}
	else
	{
		/* how to handle MMU reads? for now just do it */
		return m_io->read_byte(addr);
	}
}

UINT16 z8002_device::RDPORT_W(int mode, UINT16 addr)
{
	if(mode == 0)
	{
		return m_io->read_byte((UINT16)(addr)) +
			(m_io->read_byte((UINT16)(addr+1)) << 8);
	}
	else
	{
		/* how to handle MMU reads? */
		return 0x0000;
	}
}

UINT16 z8001_device::RDPORT_W(int mode, UINT16 addr)
{
	if(mode == 0)
	{
		return m_io->read_word_unaligned((UINT16)addr);
	}
	else
	{
		/* how to handle MMU reads? */
		return 0x0000;
	}
}

void z8002_device::WRPORT_B(int mode, UINT16 addr, UINT8 value)
{
	if(mode == 0)
	{
		m_io->write_byte(addr,value);
	}
	else
	{
		/* how to handle MMU writes? for now just do it */
		m_io->write_byte(addr,value);
	}
}

void z8002_device::WRPORT_W(int mode, UINT16 addr, UINT16 value)
{
	if(mode == 0)
	{
		m_io->write_byte((UINT16)(addr),value & 0xff);
		m_io->write_byte((UINT16)(addr+1),(value >> 8) & 0xff);
	}
	else
	{
		/* how to handle MMU writes? */
	}
}

void z8001_device::WRPORT_W(int mode, UINT16 addr, UINT16 value)
{
	if(mode == 0)
	{
		m_io->write_word_unaligned((UINT16)addr, value);
	}
	else
	{
		/* how to handle MMU writes? */
	}
}

void z8002_device::cycles(int cycles)
{
	m_icount -= cycles;
}

#include "z8000ops.inc"
#include "z8000tbl.inc"

void z8002_device::set_irq(int type)
{
	switch ((type >> 8) & 255)
	{
		case Z8000_EPU >> 8:
			m_irq_req = type;
			break;
		case Z8000_TRAP >> 8:
			m_irq_req = type;
			break;
		case Z8000_NMI >> 8:
			m_irq_req = type;
			break;
		case Z8000_SEGTRAP >> 8:
			m_irq_req = type;
			break;
		case Z8000_NVI >> 8:
			m_irq_req = type;
			break;
		case Z8000_VI >> 8:
			m_irq_req = type;
			break;
		case Z8000_SYSCALL >> 8:
			LOG(("Z8K '%s' SYSCALL $%02x\n", tag(), type & 0xff));
			m_irq_req = type;
			break;
		default:
			logerror("Z8000 invalid Cause_Interrupt %04x\n", type);
			return;
	}
	/* set interrupt request flag, reset HALT flag */
	m_irq_req = type & ~Z8000_HALT;
}

void z8002_device::PUSH_PC()
{
	PUSHW(SP, m_pc);        /* save current pc */
}

void z8001_device::PUSH_PC()
{
	PUSHL(SP, make_segmented_addr(m_pc));        /* save current pc */
}


UINT32 z8002_device::GET_PC(UINT32 VEC)
{
	return RDMEM_W(AS_PROGRAM, VEC + 2);
}

UINT32 z8001_device::GET_PC(UINT32 VEC)
{
	return segmented_addr(RDMEM_L(AS_PROGRAM, VEC + 4));
}

UINT16 z8002_device::GET_FCW(UINT32 VEC)
{
	return RDMEM_W(AS_PROGRAM, VEC);
}

UINT16 z8001_device::GET_FCW(UINT32 VEC)
{
	return RDMEM_W(AS_PROGRAM, VEC + 2);
}

UINT32 z8002_device::F_SEG_Z8001()
{
	return 0;
}

UINT32 z8001_device::F_SEG_Z8001()
{
	return F_SEG;
}

UINT32 z8002_device::PSA_ADDR()
{
	return m_psapoff;
}

UINT32 z8001_device::PSA_ADDR()
{
	return segmented_addr((m_psapseg << 16) | m_psapoff);
}


void z8002_device::Interrupt()
{
	UINT16 fcw = m_fcw;

	if (m_irq_req & Z8000_NVI)
	{
		int type = standard_irq_callback(0);
		set_irq(type | Z8000_NVI);
	}

	if (m_irq_req & Z8000_VI)
	{
		int type = standard_irq_callback(1);
		set_irq(type | Z8000_VI);
	}

	/* trap ? */
	if (m_irq_req & Z8000_EPU)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, RDMEM_W(AS_PROGRAM, m_ppc));  /* for internal traps, the 1st word of the instruction is pushed */
		m_irq_req &= ~Z8000_EPU;
		CHANGE_FCW(GET_FCW(EPU));
		m_pc = GET_PC(EPU);
		LOG(("Z8K '%s' ext instr trap $%04x\n", tag(), m_pc));
	}
	else
	if (m_irq_req & Z8000_TRAP)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, RDMEM_W(AS_PROGRAM, m_ppc));  /* for internal traps, the 1st word of the instruction is pushed */
		m_irq_req &= ~Z8000_TRAP;
		CHANGE_FCW(GET_FCW(TRAP));
		m_pc = GET_PC(TRAP);
		LOG(("Z8K '%s' priv instr trap $%04x\n", tag(), m_pc));
	}
	else
	if (m_irq_req & Z8000_SYSCALL)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, RDMEM_W(AS_PROGRAM, m_ppc));  /* for internal traps, the 1st word of the instruction is pushed */
		m_irq_req &= ~Z8000_SYSCALL;
		CHANGE_FCW(GET_FCW(SYSCALL));
		m_pc = GET_PC(SYSCALL);
		LOG(("Z8K '%s' syscall $%04x\n", tag(), m_pc));
	}
	else
	if (m_irq_req & Z8000_SEGTRAP)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_req);   /* save interrupt/trap type tag */
		m_irq_req &= ~Z8000_SEGTRAP;
		CHANGE_FCW(GET_FCW(SEGTRAP));
		m_pc = GET_PC(SEGTRAP);
		LOG(("Z8K '%s' segtrap $%04x\n", tag(), m_pc));
	}
	else
	if (m_irq_req & Z8000_NMI)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_req);   /* save interrupt/trap type tag */
		m_pc = RDMEM_W(AS_PROGRAM, NMI);
		m_irq_req &= ~Z8000_NMI;
		CHANGE_FCW(GET_FCW(NMI));
		m_pc = GET_PC(NMI);
		LOG(("Z8K '%s' NMI $%04x\n", tag(), m_pc));
	}
	else
	if ((m_irq_req & Z8000_NVI) && (m_fcw & F_NVIE))
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_req);   /* save interrupt/trap type tag */
		m_pc = GET_PC(NVI);
		m_irq_req &= ~Z8000_NVI;
		CHANGE_FCW(GET_FCW(NVI));
		LOG(("Z8K '%s' NVI $%04x\n", tag(), m_pc));
	}
	else
	if ((m_irq_req & Z8000_VI) && (m_fcw & F_VIE))
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_req);   /* save interrupt/trap type tag */
		m_pc = read_irq_vector();
		m_irq_req &= ~Z8000_VI;
		CHANGE_FCW(GET_FCW(VI));
		LOG(("Z8K '%s' VI [$%04x/$%04x] fcw $%04x, pc $%04x\n", tag(), m_irq_vec, VEC00 + ( m_vector_mult * 2 ) * (m_irq_req & 0xff), m_fcw, m_pc));
	}
}

UINT32 z8002_device::read_irq_vector()
{
	return RDMEM_W(AS_PROGRAM, VEC00 + 2 * (m_irq_req & 0xff));
}


UINT32 z8001_device::read_irq_vector()
{
	return segmented_addr(RDMEM_L(AS_PROGRAM, VEC00 + 4 * (m_irq_req & 0xff)));
}


void z8002_device::clear_internal_state()
{
	m_op[0] = m_op[1] = m_op[2] = m_op[3] = 0;
	m_ppc = 0;
	m_pc = 0;
	m_psapseg = 0;
	m_psapoff = 0;
	m_fcw = 0;
	m_refresh = 0;
	m_nspseg = 0;
	m_nspoff = 0;
	m_irq_req = 0;
	m_irq_vec = 0;
	m_op_valid = 0;
	m_regs.Q[0] = m_regs.Q[1] = m_regs.Q[2] = m_regs.Q[3] = 0;
	m_nmi_state = 0;
	m_irq_state[0] = m_irq_state[1] = 0;
}

void z8002_device::register_debug_state()
{
	state_add( Z8000_PPC,     "prev PC", m_ppc     ).formatstr("%08X");
	state_add( Z8000_PC,      "PC",      m_pc      ).formatstr("%08X");
	state_add( Z8000_NSPOFF,  "NSPOFF",  m_nspoff  ).formatstr("%04X");
	state_add( Z8000_NSPSEG,  "NSPSEG",  m_nspseg  ).formatstr("%04X");
	state_add( Z8000_FCW,     "FCW",     m_fcw     ).formatstr("%04X");
	state_add( Z8000_PSAPOFF, "PSAPOFF", m_psapoff ).formatstr("%04X");
	state_add( Z8000_PSAPSEG, "PSAPSEG", m_psapseg ).formatstr("%04X");
	state_add( Z8000_REFRESH, "REFR",    m_refresh ).formatstr("%04X");
	state_add( Z8000_IRQ_REQ, "IRQR",    m_irq_req ).formatstr("%04X");
	state_add( Z8000_IRQ_VEC, "IRQV",    m_irq_vec ).formatstr("%04X");
	state_add( Z8000_R0,      "R0",      RW(0)     ).formatstr("%04X");
	state_add( Z8000_R1,      "R1",      RW(1)     ).formatstr("%04X");
	state_add( Z8000_R2,      "R2",      RW(2)     ).formatstr("%04X");
	state_add( Z8000_R3,      "R3",      RW(3)     ).formatstr("%04X");
	state_add( Z8000_R4,      "R4",      RW(4)     ).formatstr("%04X");
	state_add( Z8000_R5,      "R5",      RW(5)     ).formatstr("%04X");
	state_add( Z8000_R6,      "R6",      RW(6)     ).formatstr("%04X");
	state_add( Z8000_R7,      "R7",      RW(7)     ).formatstr("%04X");
	state_add( Z8000_R8,      "R8",      RW(8)     ).formatstr("%04X");
	state_add( Z8000_R9,      "R9",      RW(9)     ).formatstr("%04X");
	state_add( Z8000_R10,     "R10",     RW(10)    ).formatstr("%04X");
	state_add( Z8000_R11,     "R11",     RW(11)    ).formatstr("%04X");
	state_add( Z8000_R12,     "R12",     RW(12)    ).formatstr("%04X");
	state_add( Z8000_R13,     "R13",     RW(13)    ).formatstr("%04X");
	state_add( Z8000_R14,     "R14",     RW(14)    ).formatstr("%04X");
	state_add( Z8000_R15,     "R15",     RW(15)    ).formatstr("%04X");

	state_add( STATE_GENPC, "GENPC", m_pc ).noshow();
	state_add( STATE_GENPCBASE, "GENPCBASE", m_ppc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_fcw ).formatstr("%16s").noshow();
	state_add( STATE_GENSP, "GENSP", m_nspoff ).noshow();
}

void z8002_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				m_fcw & 0x8000 ? 'S':'s',
				m_fcw & 0x4000 ? 'n':'N',
				m_fcw & 0x2000 ? 'E':'e',
				m_fcw & 0x1000 ? 'V':'v',
				m_fcw & 0x0800 ? 'N':'n',
				m_fcw & 0x0400 ? '?':'.',
				m_fcw & 0x0200 ? '?':'.',
				m_fcw & 0x0100 ? '?':'.',
				m_fcw & 0x0080 ? 'C':'c',
				m_fcw & 0x0040 ? 'Z':'z',
				m_fcw & 0x0020 ? 'S':'s',
				m_fcw & 0x0010 ? 'V':'v',
				m_fcw & 0x0008 ? 'D':'d',
				m_fcw & 0x0004 ? 'H':'h',
				m_fcw & 0x0002 ? '?':'.',
				m_fcw & 0x0001 ? '?':'.');
			break;
	}

}

void z8001_device::device_start()
{
	clear_internal_state();

	m_program = &space(AS_PROGRAM);
	/* If the system decodes STn lines to distinguish between data and program memory fetches,
	   install the data space. If it doesn't, install the program memory into data memory space. */
	if (has_space(AS_DATA))
		m_data = &space(AS_DATA);
	else
		m_data = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	/* already initialized? */
	if(z8000_exec == nullptr)
		z8000_init_tables();

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
		debug_console_register_command(machine(), "z8k_disass_mode", CMDFLAG_NONE, 0, 0, 1, z8k_disass_mode);

	z8k_segm = true;

	register_debug_state();

	m_icountptr = &m_icount;
	m_mo_out.resolve_safe();
	m_mi = CLEAR_LINE;
}

void z8002_device::device_start()
{
	clear_internal_state();

	m_program = &space(AS_PROGRAM);
	/* If the system decodes STn lines to distinguish between data and program memory fetches,
	   install the data space. If it doesn't, install the program memory into data memory space. */
	if (has_space(AS_DATA))
		m_data = &space(AS_DATA);
	else
		m_data = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	/* already initialized? */
	if(z8000_exec == nullptr)
		z8000_init_tables();

	z8k_segm = false;

	register_debug_state();

	m_icountptr = &m_icount;
	m_mo_out.resolve_safe();
	m_mi = CLEAR_LINE;
}

void z8001_device::device_reset()
{
	m_fcw = RDMEM_W(AS_PROGRAM, 2); /* get reset m_fcw */
	if(m_fcw & F_SEG)
	{
		m_pc = ((RDMEM_W(AS_PROGRAM, 4) & 0x0700) << 8) | (RDMEM_W(AS_PROGRAM, 6) & 0xffff); /* get reset m_pc  */
	}
	else
	{
		m_pc = RDMEM_W(AS_PROGRAM, 4); /* get reset m_pc  */
	}
	m_ppc = m_pc;
}

void z8002_device::device_reset()
{
	m_fcw = RDMEM_W(AS_PROGRAM, 2); /* get reset m_fcw */
	m_pc = RDMEM_W(AS_PROGRAM, 4); /* get reset m_pc  */
	m_ppc = m_pc;
}

z8002_device::~z8002_device()
{
	z8000_deinit_tables();
}

void z8002_device::execute_run()
{
	do
	{
		/* any interrupt request pending? */
		if (m_irq_req)
			Interrupt();

		if (z8k_segm_mode == Z8K_SEGM_MODE_AUTO)
			z8k_segm = (m_fcw & F_SEG_Z8001()) ? 1 : 0;

		debugger_instruction_hook(this, m_pc);

		if (m_irq_req & Z8000_HALT)
		{
			m_icount = 0;
		}
		else
		{
			Z8000_exec *exec;

			m_ppc = m_pc;
			m_op[0] = RDOP();
			m_op_valid = 1;
			exec = &z8000_exec[m_op[0]];

			m_icount -= exec->cycles;
			(this->*exec->opcode)();
			m_op_valid = 0;
		}
	} while (m_icount > 0);

}

void z8002_device::execute_set_input(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (m_nmi_state == state)
			return;

		m_nmi_state = state;

		if (state != CLEAR_LINE)
		{
			m_irq_req = Z8000_NMI;
			m_irq_vec = NMI;
		}
	}
	else if (irqline < 2)
	{
		m_irq_state[irqline] = state;
		if (irqline == 0)
		{
			if (state == CLEAR_LINE)
			{
				if (!(m_fcw & F_NVIE))
					m_irq_req &= ~Z8000_NVI;
			}
			else
			{
				if (m_fcw & F_NVIE)
					m_irq_req |= Z8000_NVI;
			}
		}
		else
		{
			if (state == CLEAR_LINE)
			{
				if (!(m_fcw & F_VIE))
					m_irq_req &= ~Z8000_VI;
			}
			else
			{
				if (m_fcw & F_VIE)
					m_irq_req |= Z8000_VI;
			}
		}
	}
}
