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
#include "z8000.h"
#include "z8000cpu.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(Z8001, z8001_device, "z8001", "Zilog Z8001")
DEFINE_DEVICE_TYPE(Z8002, z8002_device, "z8002", "Zilog Z8002")


z8002_device::z8002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8002_device(mconfig, Z8002, tag, owner, clock, 16, 1)
{
}


z8002_device::z8002_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int addrbits, int vecmult)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, addrbits, 0)
	, m_data_config("data", ENDIANNESS_BIG, 16, addrbits, 0)
	, m_io_config("io_std", ENDIANNESS_BIG, 16, 16, 0)
	, m_opcodes_config("first_word", ENDIANNESS_BIG, 16, addrbits, 0)
	, m_stack_config("stack", ENDIANNESS_BIG, 16, addrbits, 0)
	, m_sio_config("io_spc", ENDIANNESS_BIG, 16, 16, 0)
	, m_iack_in(*this, 0xffff)
	, m_mo_out(*this)
	, m_ppc(0), m_pc(0), m_psapseg(0), m_psapoff(0), m_fcw(0), m_refresh(0), m_nspseg(0), m_nspoff(0), m_irq_req(0), m_irq_vec(0), m_op_valid(0), m_nmi_state(0), m_mi(0), m_halt(false), m_icount(0)
	, m_vector_mult(vecmult)
{
}


z8001_device::z8001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8002_device(mconfig, Z8001, tag, owner, clock, 23, 2)
{
}


device_memory_interface::space_config_vector z8002_device::memory_space_config() const
{
	auto spaces = space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
	if (has_configured_map(AS_DATA))
		spaces.push_back(std::make_pair(AS_DATA, &m_data_config));
	if (has_configured_map(AS_OPCODES))
		spaces.push_back(std::make_pair(AS_OPCODES, &m_opcodes_config));
	if (has_configured_map(AS_STACK))
		spaces.push_back(std::make_pair(AS_STACK, &m_stack_config));
	if (has_configured_map(AS_SIO))
		spaces.push_back(std::make_pair(AS_SIO, &m_sio_config));
	return spaces;
}

bool z8002_device::get_segmented_mode() const
{
	return false;
}

bool z8001_device::get_segmented_mode() const
{
	return (m_fcw & F_SEG) ? true : false;
}

uint32_t z8002_device::addr_add(uint32_t addr, uint32_t addend)
{
	return (addr & 0xffff0000) | ((addr + addend) & 0xffff);
}

uint32_t z8002_device::addr_sub(uint32_t addr, uint32_t subtrahend)
{
	return (addr & 0xffff0000) | ((addr - subtrahend) & 0xffff);
}

/* conversion table for Z8000 DAB opcode */
#include "z8000dab.h"

uint16_t z8002_device::RDOP()
{
	uint16_t res = m_opcache.read_word(m_pc);
	m_pc += 2;
	return res;
}

uint32_t z8002_device::get_operand(int opnum)
{
	int i;

	for (i = 0; i < opnum; i++)
	{
		assert (m_op_valid & (1 << i));
	}

	if (! (m_op_valid & (1 << opnum)))
	{
		m_op[opnum] = m_cache.read_word(m_pc);
		m_pc += 2;
		m_op_valid |= (1 << opnum);
	}
	return m_op[opnum];
}

uint32_t z8002_device::get_addr_operand(int opnum)
{
	int i;

	for (i = 0; i < opnum; i++)
	{
		assert (m_op_valid & (1 << i));
	}

	if (! (m_op_valid & (1 << opnum)))
	{
		uint32_t seg = m_cache.read_word(m_pc);
		m_pc += 2;
		if (get_segmented_mode())
		{
			if (seg & 0x8000)
			{
				m_op[opnum] = ((seg & 0x7f00) << 8) | m_cache.read_word(m_pc);
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

uint32_t z8002_device::get_raw_addr_operand(int opnum)
{
	int i;

	for (i = 0; i < opnum; i++)
	{
		assert (m_op_valid & (1 << i));
	}

	if (! (m_op_valid & (1 << opnum)))
	{
		uint32_t seg = m_cache.read_word(m_pc);
		m_pc += 2;
		if (get_segmented_mode())
		{
			if (seg & 0x8000)
			{
				m_op[opnum] = (seg << 16) | m_cache.read_word(m_pc);
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

uint32_t z8002_device::adjust_addr_for_nonseg_mode(uint32_t addr)
{
	return addr;
}

uint32_t z8001_device::adjust_addr_for_nonseg_mode(uint32_t addr)
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

uint8_t z8002_device::RDMEM_B(memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space, uint32_t addr)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	return space.read_byte(addr);
}

uint16_t z8002_device::RDMEM_W(memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space, uint32_t addr)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	/* hack for m20 driver: BIOS accesses 0x7f0000 and expects a segmentation violation */
	if (addr >= 0x7f0000) {
		m_irq_req = Z8000_SEGTRAP;
		return 0xffff;
	}
	return space.read_word(addr);
}

uint32_t z8002_device::RDMEM_L(memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space, uint32_t addr)
{
	uint32_t result;
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	result = space.read_word(addr) << 16;
	return result + space.read_word(addr_add(addr, 2));
}

void z8002_device::WRMEM_B(memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space, uint32_t addr, uint8_t value)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	uint16_t value16 = value | (value << 8);
	space.write_word(addr & ~1, value16, BIT(addr, 0) ? 0x00ff : 0xff00);
}

void z8002_device::WRMEM_W(memory_access<23, 1, 0, ENDIANNESS_BIG>::specific  &space, uint32_t addr, uint16_t value)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	space.write_word(addr, value);
}

void z8002_device::WRMEM_L(memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space, uint32_t addr, uint32_t value)
{
	addr = adjust_addr_for_nonseg_mode(addr);
	addr &= ~1;
	space.write_word(addr, value >> 16);
	space.write_word(addr_add(addr, 2), value & 0xffff);
}

uint8_t z8002_device::RDPORT_B(int mode, uint16_t addr)
{
	memory_access<16, 1, 0, ENDIANNESS_BIG>::specific &space = (mode == 0) ? m_io : m_sio;
	return space.read_byte(addr);
}

uint16_t z8002_device::RDPORT_W(int mode, uint16_t addr)
{
	memory_access<16, 1, 0, ENDIANNESS_BIG>::specific &space = (mode == 0) ? m_io : m_sio;
	if (BIT(addr, 0))
		return swapendian_int16(space.read_word(addr & ~1, 0xffff));
	else
		return space.read_word(addr);
}

void z8002_device::WRPORT_B(int mode, uint16_t addr, uint8_t value)
{
	memory_access<16, 1, 0, ENDIANNESS_BIG>::specific &space = (mode == 0) ? m_io : m_sio;
	uint16_t value16 = value | (value << 8);
	space.write_word(addr & ~1, value16, BIT(addr, 0) ? 0x00ff : 0xff00);
}

void z8002_device::WRPORT_W(int mode, uint16_t addr, uint16_t value)
{
	memory_access<16, 1, 0, ENDIANNESS_BIG>::specific &space = (mode == 0) ? m_io : m_sio;
	if (BIT(addr, 0))
		space.write_word(addr & ~1, swapendian_int16(value), 0xffff);
	else
		space.write_word(addr, value, 0xffff);
}

void z8002_device::cycles(int cycles)
{
	m_icount -= cycles;
}

#include "z8000ops.hxx"
#include "z8000tbl.hxx"

void z8002_device::PUSH_PC()
{
	PUSHW(SP, m_pc);        /* save current pc */
}

void z8001_device::PUSH_PC()
{
	PUSHL(SP, make_segmented_addr(m_pc));        /* save current pc */
}


uint32_t z8002_device::GET_PC(uint32_t VEC)
{
	return RDMEM_W(m_program, VEC + 2);
}

uint32_t z8001_device::GET_PC(uint32_t VEC)
{
	return segmented_addr(RDMEM_L(m_program, VEC + 4));
}

uint32_t z8002_device::get_reset_pc()
{
	return RDMEM_W(m_program, 4);
}

uint32_t z8001_device::get_reset_pc()
{
	return segmented_addr(RDMEM_L(m_program, 4));
}

uint16_t z8002_device::GET_FCW(uint32_t VEC)
{
	return RDMEM_W(m_program, VEC);
}

uint16_t z8001_device::GET_FCW(uint32_t VEC)
{
	return RDMEM_W(m_program, VEC + 2);
}

uint32_t z8002_device::F_SEG_Z8001()
{
	return 0;
}

uint32_t z8001_device::F_SEG_Z8001()
{
	return F_SEG;
}

uint32_t z8002_device::PSA_ADDR()
{
	return m_psapoff;
}

uint32_t z8001_device::PSA_ADDR()
{
	return segmented_addr((m_psapseg << 16) | m_psapoff);
}


void z8002_device::Interrupt()
{
	uint16_t fcw = m_fcw;

	if (m_irq_req & Z8000_RESET)
	{
		m_irq_req &= Z8000_NVI | Z8000_VI;
		CHANGE_FCW(RDMEM_W(m_program, 2)); /* get reset m_fcw */
		m_pc = get_reset_pc(); /* get reset m_pc  */
	}
	else
	/* trap ? */
	if (m_irq_req & Z8000_EPU)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_op[0]);   /* for internal traps, the 1st word of the instruction is pushed */
		m_irq_req &= ~Z8000_EPU;
		CHANGE_FCW(GET_FCW(EPU));
		m_pc = GET_PC(EPU);
		LOG("Z8K ext instr trap $%04x\n", m_pc);
	}
	else
	if (m_irq_req & Z8000_TRAP)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_op[0]);   /* for internal traps, the 1st word of the instruction is pushed */
		m_irq_req &= ~Z8000_TRAP;
		CHANGE_FCW(GET_FCW(TRAP));
		m_pc = GET_PC(TRAP);
		LOG("Z8K priv instr trap $%04x\n", m_pc);
	}
	else
	if (m_irq_req & Z8000_SYSCALL)
	{
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_op[0]);   /* for internal traps, the 1st word of the instruction is pushed */
		m_irq_req &= ~Z8000_SYSCALL;
		CHANGE_FCW(GET_FCW(SYSCALL));
		m_pc = GET_PC(SYSCALL);
		LOG("Z8K syscall [$%02x/$%04x]\n", m_op[0] & 0xff, m_pc);
	}
	else
	if (m_irq_req & Z8000_SEGTRAP)
	{
		//standard_irq_callback(SEGT_LINE, m_pc);
		m_irq_vec = m_iack_in[0](m_pc);

		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_vec);   /* save interrupt/trap type tag */
		m_irq_req &= ~Z8000_SEGTRAP;
		CHANGE_FCW(GET_FCW(SEGTRAP));
		m_pc = GET_PC(SEGTRAP);
		LOG("Z8K segtrap $%04x\n", m_pc);
	}
	else
	if (m_irq_req & Z8000_NMI)
	{
		standard_irq_callback(NMI_LINE, m_pc);
		m_irq_vec = m_iack_in[1](m_pc);
		m_halt = false;

		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_vec);   /* save interrupt/trap type tag */
		m_pc = RDMEM_W(m_program, NMI);
		m_irq_req &= ~Z8000_NMI;
		CHANGE_FCW(GET_FCW(NMI));
		m_pc = GET_PC(NMI);
		LOG("Z8K NMI $%04x\n", m_pc);
	}
	else
	if ((m_irq_req & Z8000_NVI) && (m_fcw & F_NVIE))
	{
		standard_irq_callback(NVI_LINE, m_pc);
		m_irq_vec = m_iack_in[2](m_pc);
		m_halt = false;

		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_vec);   /* save interrupt/trap type tag */
		m_pc = GET_PC(NVI);
		m_irq_req &= ~Z8000_NVI;
		CHANGE_FCW(GET_FCW(NVI));
		LOG("Z8K NVI $%04x\n", m_pc);
	}
	else
	if ((m_irq_req & Z8000_VI) && (m_fcw & F_VIE))
	{
		standard_irq_callback(VI_LINE, m_pc);
		m_irq_vec = m_iack_in[3](m_pc);
		m_halt = false;

		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());/* switch to segmented (on Z8001) system mode */
		PUSH_PC();
		PUSHW(SP, fcw);       /* save current m_fcw */
		PUSHW(SP, m_irq_vec);   /* save interrupt/trap type tag */
		m_pc = read_irq_vector();
		m_irq_req &= ~Z8000_VI;
		CHANGE_FCW(GET_FCW(VI));
		LOG("Z8K VI [$%04x/$%04x] fcw $%04x, pc $%04x\n", m_irq_vec, VEC00 + 2 * (m_irq_vec & 0xff), m_fcw, m_pc);
	}
}

uint32_t z8002_device::read_irq_vector()
{
	return RDMEM_W(m_program, VEC00 + 2 * (m_irq_vec & 0xff));
}


uint32_t z8001_device::read_irq_vector()
{
	return segmented_addr(RDMEM_L(m_program, VEC00 + 2 * (m_irq_vec & 0xff)));
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
	state_add( Z8000_PC,      "PC",      m_pc      ).mask(m_program.space().addrmask());
	state_add( Z8000_NSPOFF,  "NSPOFF",  m_nspoff  ).formatstr("%04X");
	state_add( Z8000_NSPSEG,  "NSPSEG",  m_nspseg  ).formatstr("%04X");
	state_add( Z8000_FCW,     "FCW",     m_fcw     ).formatstr("%04X");
	state_add( Z8000_PSAPOFF, "PSAPOFF", m_psapoff ).formatstr("%04X");
	state_add( Z8000_PSAPSEG, "PSAPSEG", m_psapseg ).formatstr("%04X");
	state_add( Z8000_REFRESH, "REFR",    m_refresh ).formatstr("%04X");
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
	state_add( STATE_GENPCBASE, "CURPC", m_ppc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_fcw ).formatstr("%16s").noshow();
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

void z8002_device::register_save_state()
{
	save_item(NAME(m_op));
	save_item(NAME(m_ppc));
	save_item(NAME(m_pc));
	save_item(NAME(m_psapseg));
	save_item(NAME(m_psapoff));
	save_item(NAME(m_fcw));
	save_item(NAME(m_refresh));
	save_item(NAME(m_nspseg));
	save_item(NAME(m_nspoff));
	save_item(NAME(m_irq_req));
	save_item(NAME(m_irq_vec));
	save_item(NAME(m_op_valid));
	save_item(NAME(m_regs.Q));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_mi));
	save_item(NAME(m_halt));
	save_item(NAME(m_icount));
}

void z8002_device::init_spaces()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	/* If the system decodes STn lines to distinguish between data and program memory fetches,
	   install the data space. If it doesn't, install the program memory into data memory space. */
	space(has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_data);
	space(has_space(AS_STACK) ? AS_STACK : has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_stack);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_opcache);
	space(AS_IO).specific(m_io);
	space(has_space(AS_SIO) ? AS_SIO : AS_IO).specific(m_sio);
}

void z8002_device::init_tables()
{
	/* set up the zero, sign, parity lookup table */
	for (int i = 0; i < 256; i++)
		z8000_zsp[i] = ((i == 0) ? F_Z : 0) |
						((i & 128) ? F_S : 0) |
						((((i>>7)^(i>>6)^(i>>5)^(i>>4)^(i>>3)^(i>>2)^(i>>1)^i) & 1) ? 0 : F_PV);

	for (const Z8000_init *opc = table; opc->size; opc++)
		for (u32 val = opc->beg; val <= opc->end; val += opc->step)
			z8000_exec[val] = opc - table;
}

std::unique_ptr<util::disasm_interface> z8002_device::create_disassembler()
{
	return std::make_unique<z8000_disassembler>(this);
}

void z8002_device::device_start()
{
	clear_internal_state();

	init_spaces();
	init_tables();

	register_debug_state();
	register_save_state();

	set_icountptr(m_icount);
	m_mi = CLEAR_LINE;
}

void z8002_device::device_reset()
{
	m_irq_req |= Z8000_RESET;
	m_refresh &= 0x7fff;
	m_halt = false;
}

z8002_device::~z8002_device()
{
}

void z8002_device::execute_run()
{
	do
	{
		/* any interrupt request pending? */
		if (m_irq_req)
			Interrupt();

		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

		if (m_halt)
		{
			debugger_wait_hook();
			m_icount = 0;
		}
		else
		{
			m_op[0] = RDOP();
			m_op_valid = 1;
			const Z8000_init &exec = table[z8000_exec[m_op[0]]];

			m_icount -= exec.cycles;
			(this->*exec.opcode)();
			m_op_valid = 0;
		}
	} while (m_icount > 0);

}

void z8002_device::execute_set_input(int irqline, int state)
{
	if (irqline == NMI_LINE)
	{
		if (m_nmi_state == state)
			return;

		m_nmi_state = state;

		if (state != CLEAR_LINE)
		{
			m_irq_req |= Z8000_NMI;
		}
	}
	else if (irqline < 2)
	{
		m_irq_state[irqline] = state;
		if (irqline == NVI_LINE)
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
