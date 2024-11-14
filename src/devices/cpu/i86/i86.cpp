// license:BSD-3-Clause
// copyright-holders:Carl
/****************************************************************************

    NEC V20/V30/V33 emulator modified back to a 8086/80186 emulator

    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
    on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
    on the i286 emulator by Fabrice Frances which had initial work based on
    David Hedley's pcemu(!).

****************************************************************************/

#include "emu.h"
#include "i86.h"
#include "i86inline.h"
#include "cpu/i386/i386dasm.h"

#define I8086_NMI_INT_VECTOR 2

const uint8_t i8086_cpu_device::m_i8086_timing[] =
{
	51,32,          /* exception, IRET */
		2, 0, 4, 2, /* INTs */
		2,              /* segment overrides */
		2, 4, 4,        /* flag operations */
		4, 4,83,60, /* arithmetic adjusts */
		4, 4,           /* decimal adjusts */
		2, 5,           /* sign extension */
		2,24, 2, 2, 3,11,   /* misc */

	15,15,15,       /* direct JMPs */
	11,18,24,       /* indirect JMPs */
	19,28,          /* direct CALLs */
	16,21,37,       /* indirect CALLs */
	20,32,24,31,    /* returns */
		4,16, 6,18, /* conditional JMPs */
		5,17, 6,18, /* loops */

	10,14, 8,12,    /* port reads */
	10,14, 8,12,    /* port writes */

		2, 8, 9,        /* move, 8-bit */
		4,10,           /* move, 8-bit immediate */
		2, 8, 9,        /* move, 16-bit */
		4,10,           /* move, 16-bit immediate */
	10,10,10,10,    /* move, AL/AX memory */
		2, 8, 2, 9, /* move, segment registers */
		4,17,           /* exchange, 8-bit */
		4,17, 3,        /* exchange, 16-bit */

	15,24,14,14,    /* pushes */
	12,25,12,12,    /* pops */

		3, 9,16,        /* ALU ops, 8-bit */
		4,17,10,        /* ALU ops, 8-bit immediate */
		3, 9,16,        /* ALU ops, 16-bit */
		4,17,10,        /* ALU ops, 16-bit immediate */
		4,17,10,        /* ALU ops, 16-bit w/8-bit immediate */
	70,118,76,128,  /* MUL */
	80,128,86,138,  /* IMUL */
	80,144,86,154,  /* DIV */
	101,165,107,175,/* IDIV */
		3, 2,15,15, /* INC/DEC */
		3, 3,16,16, /* NEG/NOT */

		2, 8, 4,        /* reg shift/rotate */
	15,20, 4,       /* m8 shift/rotate */
	15,20, 4,       /* m16 shift/rotate */

	22, 9,21,       /* CMPS 8-bit */
	22, 9,21,       /* CMPS 16-bit */
	15, 9,14,       /* SCAS 8-bit */
	15, 9,14,       /* SCAS 16-bit */
	12, 9,11,       /* LODS 8-bit */
	12, 9,11,       /* LODS 16-bit */
	11, 9,10,       /* STOS 8-bit */
	11, 9,10,       /* STOS 16-bit */
	18, 9,17,       /* MOVS 8-bit */
	18, 9,17,       /* MOVS 16-bit */
};

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/


/***************************************************************************/

DEFINE_DEVICE_TYPE(I8086, i8086_cpu_device, "i8086", "Intel 8086")
DEFINE_DEVICE_TYPE(I8088, i8088_cpu_device, "i8088", "Intel 8088")

i8088_cpu_device::i8088_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8086_cpu_device(mconfig, I8088, tag, owner, clock, 8)
{
	memcpy(m_timing, m_i8086_timing, sizeof(m_i8086_timing));
}

i8086_cpu_device::i8086_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8086_cpu_device(mconfig, I8086, tag, owner, clock, 16)
{
	memcpy(m_timing, m_i8086_timing, sizeof(m_i8086_timing));
}

i8086_cpu_device::i8086_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int data_bus_size)
	: i8086_common_cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_opcodes_config("opcodes", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_stack_config("stack", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_code_config("code", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_extra_config("extra", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, data_bus_size, 16, 0)
	, m_out_if_func(*this)
	, m_esc_opcode_handler(*this)
	, m_esc_data_handler(*this)
{
}

device_memory_interface::space_config_vector i8086_cpu_device::memory_space_config() const
{
	space_config_vector spaces = {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
	if(has_configured_map(AS_OPCODES))
		spaces.push_back(std::make_pair(AS_OPCODES, &m_opcodes_config));
	if(has_configured_map(AS_STACK))
		spaces.push_back(std::make_pair(AS_STACK, &m_stack_config));
	if(has_configured_map(AS_CODE))
		spaces.push_back(std::make_pair(AS_CODE, &m_code_config));
	if(has_configured_map(AS_EXTRA))
		spaces.push_back(std::make_pair(AS_EXTRA, &m_extra_config));
	return spaces;
}

uint8_t i8086_cpu_device::fetch()
{
	uint8_t data;
	data = m_or8(update_pc());
	m_ip++;
	return data;
}

address_space *i8086_cpu_device::sreg_to_space(int sreg) const
{
	switch(sreg)
	{
		default:
			return m_program;
		case CS:
			return m_code;
		case SS:
			return m_stack;
		case ES:
			return m_extra;
	}
}

uint8_t i8086_cpu_device::read_byte(uint32_t addr)
{
	return sreg_to_space(m_easeg)->read_byte(addr);
}

uint16_t i8086_cpu_device::read_word(uint32_t addr)
{
	return sreg_to_space(m_easeg)->read_word_unaligned(addr);
}

void i8086_cpu_device::write_byte(uint32_t addr, uint8_t data)
{
	sreg_to_space(m_easeg)->write_byte(addr, data);
}

void i8086_cpu_device::write_word(uint32_t addr, uint16_t data)
{
	sreg_to_space(m_easeg)->write_word_unaligned(addr, data);
}

void i8086_cpu_device::execute_run()
{
	u8 iflag = m_IF;
	while(m_icount > 0 )
	{
		if ( m_seg_prefix_next )
		{
			m_seg_prefix = true;
			m_seg_prefix_next = false;
		}
		else
		{
			m_prev_ip = m_ip;
			m_seg_prefix = false;

				/* Dispatch IRQ */
			if ( m_pending_irq && (m_no_interrupt == 0) )
			{
				if ( m_pending_irq & NMI_IRQ )
				{
					interrupt(I8086_NMI_INT_VECTOR);
					m_pending_irq &= ~NMI_IRQ;
					m_halt = false;
				}
				else if ( m_IF )
				{
					/* the actual vector is retrieved after pushing flags */
					/* and clearing the IF */
					interrupt(-1);
					m_halt = false;
				}
			}

			if(m_halt)
			{
				m_icount = 0;
				return;
			}

			/* Trap should allow one instruction to be executed.
			   CPUID.ASM (by Bob Smith, 1985) suggests that in situations where m_no_interrupt is 1,
			   (directly after POP SS / MOV_SREG), single step IRQs don't fire.
			*/
			if (m_fire_trap )
			{
				if ( (m_fire_trap >= 2) && (m_no_interrupt == 0) )
				{
					m_fire_trap = 0; // reset trap flag upon entry
					interrupt(1);
				}
				else
				{
					m_fire_trap++;
				}
			}

			/* No interrupt allowed between last instruction and this one */
			if ( m_no_interrupt )
			{
				m_no_interrupt--;
			}

		}

		if (!m_seg_prefix)
		{
			debugger_instruction_hook( update_pc() );
		}

		uint8_t op = fetch_op();

		switch(op)
		{
			case 0x0f:
				m_sregs[CS] = POP();
				CLK(POP_SEG);
				break;

			case 0xd2: // i_rotshft_bcl
				{
					uint8_t c;

					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					c = m_regs.b[CL];
					CLKM(ROT_REG_BASE,ROT_M8_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
						case 0x00: do { ROL_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x08: do { ROR_BYTE();  c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x10: do { ROLC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x18: do { RORC_BYTE(); c--; } while (c>0); PutbackRMByte(m_dst); break;
						case 0x30:
						case 0x20: SHL_BYTE(c); break;
						case 0x28: SHR_BYTE(c); break;
						case 0x38: SHRA_BYTE(c); break;
						}
					}
				}
				break;

			case 0xd3: // i_rotshft_wcl
				{
					uint8_t c;

					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					c = m_regs.b[CL];
					CLKM(ROT_REG_BASE,ROT_M16_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch ( m_modrm & 0x38 )
						{
							case 0x00: do { ROL_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x08: do { ROR_WORD();  c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x10: do { ROLC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x18: do { RORC_WORD(); c--; } while (c>0); PutbackRMWord(m_dst); break;
							case 0x30:
							case 0x20: SHL_WORD(c); break;
							case 0x28: SHR_WORD(c); break;
							case 0x38: SHRA_WORD(c); break;
						}
					}
				}
				break;

			case 0xd8: // i_esc
			case 0xd9:
			case 0xda:
			case 0xdb:
			case 0xdc:
			case 0xdd:
			case 0xde:
			case 0xdf:
				m_esc_opcode_handler(m_pc);
				m_modrm = fetch();
				if(m_modrm < 0xc0)
					m_esc_data_handler(get_ea(1, I8086_READ));
				else
					m_esc_data_handler(0);
				CLK(NOP);
				break;

			default:
				if(!common_op(op))
				{
					m_icount -= 10;
					logerror("%06x: Invalid Opcode %02x\n", m_pc, op);
					break;
				}
				break;
		}
		if(iflag != m_IF)
		{
			m_out_if_func(m_IF ? ASSERT_LINE : CLEAR_LINE);
			iflag = m_IF;
		}
	}
}

void i8086_cpu_device::device_start()
{
	i8086_common_cpu_device::device_start();
	m_stack = has_space(AS_STACK) ? &space(AS_STACK) : m_program;
	m_code = has_space(AS_CODE) ? &space(AS_CODE) : m_program;
	m_extra = has_space(AS_EXTRA) ? &space(AS_EXTRA) : m_program;
	state_add( I8086_ES, "ES", m_sregs[ES] ).formatstr("%04X");
	state_add( I8086_CS, "CS", m_sregs[CS] ).callimport().formatstr("%04X");
	state_add( I8086_SS, "SS", m_sregs[SS] ).formatstr("%04X");
	state_add( I8086_DS, "DS", m_sregs[DS] ).formatstr("%04X");
	state_add( I8086_VECTOR, "V", m_int_vector).formatstr("%02X");

	state_add( I8086_PC, "PC", m_pc ).callimport().formatstr("%05X");
	state_add<uint32_t>( STATE_GENPCBASE, "CURPC", [this] { return (m_sregs[CS] << 4) + m_prev_ip; }).mask(0xfffff).noshow();
	state_add( I8086_HALT, "HALT", m_halt ).mask(1);
}

i8086_common_cpu_device::i8086_common_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_ip(0)
	, m_TF(0)
	, m_int_vector(0)
	, m_pending_irq(0)
	, m_nmi_state(0)
	, m_irq_state(0)
	, m_test_state(1)
	, m_pc(0)
	, m_lock(false)
	, m_lock_handler(*this)
{
	static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	/* Set up parity lookup table. */
	for (uint16_t i = 0;i < 256; i++)
	{
		uint16_t c = 0;
		for (uint16_t j = i; j > 0; j >>= 1)
		{
			if (j & 1) c++;
		}
		m_parity_table[i] = !(c & 1);
	}

	for (uint16_t i = 0; i < 256; i++)
	{
		m_Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		m_Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
	}

	for (uint16_t i = 0xc0; i < 0x100; i++)
	{
		m_Mod_RM.RM.w[i] = (WREGS)( i & 7 );
		m_Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}

	memset(&m_regs, 0x00, sizeof(m_regs));
	memset(m_sregs, 0x00, sizeof(m_sregs));
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void i8086_common_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case I8086_IP:
	case I8086_CS:
		m_pc = (m_sregs[CS] << 4) + m_ip;
		break;

	case STATE_GENPC:
		if (m_pc - (m_sregs[CS] << 4) > 0xffff)
			m_sregs[CS] = m_pc >> 4;
		m_ip = m_pc - (m_sregs[CS] << 4);
		m_prev_ip = m_ip;
		break;
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void i8086_common_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			{
				uint16_t flags = CompressFlags();
				str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					flags & 0x8000 ? '1':'.',
					flags & 0x4000 ? '1':'.',
					flags & 0x2000 ? '1':'.',
					flags & 0x1000 ? '1':'.',
					flags & 0x0800 ? 'O':'.',
					flags & 0x0400 ? 'D':'.',
					flags & 0x0200 ? 'I':'.',
					flags & 0x0100 ? 'T':'.',
					flags & 0x0080 ? 'S':'.',
					flags & 0x0040 ? 'Z':'.',
					flags & 0x0020 ? '0':'.',
					flags & 0x0010 ? 'A':'.',
					flags & 0x0008 ? '0':'.',
					flags & 0x0004 ? 'P':'.',
					flags & 0x0002 ? '1':'.',
					flags & 0x0001 ? 'C':'.');
			}
			break;
	}
}

void i8086_common_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_opcodes = has_space(AS_OPCODES) ? &space(AS_OPCODES) : m_program;

	if(m_opcodes->data_width() == 8) {
		m_opcodes->cache(m_cache8);
		m_or8 = [this](offs_t address) -> u8 { return m_cache8.read_byte(address); };
	} else {
		m_opcodes->cache(m_cache16);
		m_or8 = [this](offs_t address) -> u8 { return m_cache16.read_byte(address); };
	}
	m_io = &space(AS_IO);

	save_item(NAME(m_regs.w));
	save_item(NAME(m_sregs));
	save_item(NAME(m_ip));
	save_item(NAME(m_prev_ip));
	save_item(NAME(m_TF));
	save_item(NAME(m_IF));
	save_item(NAME(m_DF));
	save_item(NAME(m_MF));
	save_item(NAME(m_NT));
	save_item(NAME(m_IOPL));
	save_item(NAME(m_SignVal));
	save_item(NAME(m_int_vector));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_AuxVal));
	save_item(NAME(m_OverVal));
	save_item(NAME(m_ZeroVal));
	save_item(NAME(m_CarryVal));
	save_item(NAME(m_ParityVal));
	save_item(NAME(m_seg_prefix));
	save_item(NAME(m_seg_prefix_next));
	save_item(NAME(m_prefix_seg));
	save_item(NAME(m_halt));

	// Register state for debugger
	state_add( I8086_IP, "IP", m_ip         ).callimport().formatstr("%04X");
	state_add( I8086_AX, "AX", m_regs.w[AX] ).formatstr("%04X");
	state_add( I8086_CX, "CX", m_regs.w[CX] ).formatstr("%04X");
	state_add( I8086_DX, "DX", m_regs.w[DX] ).formatstr("%04X");
	state_add( I8086_BX, "BX", m_regs.w[BX] ).formatstr("%04X");
	state_add( I8086_SP, "SP", m_regs.w[SP] ).formatstr("%04X");
	state_add( I8086_BP, "BP", m_regs.w[BP] ).formatstr("%04X");
	state_add( I8086_SI, "SI", m_regs.w[SI] ).formatstr("%04X");
	state_add( I8086_DI, "DI", m_regs.w[DI] ).formatstr("%04X");

	state_add( I8086_AL, "AL", m_regs.b[AL] ).noshow();
	state_add( I8086_AH, "AH", m_regs.b[AH] ).noshow();
	state_add( I8086_CL, "CL", m_regs.b[CL] ).noshow();
	state_add( I8086_CH, "CH", m_regs.b[CH] ).noshow();
	state_add( I8086_DL, "DL", m_regs.b[DL] ).noshow();
	state_add( I8086_DH, "DH", m_regs.b[DH] ).noshow();
	state_add( I8086_BL, "BL", m_regs.b[BL] ).noshow();
	state_add( I8086_BH, "BH", m_regs.b[BH] ).noshow();

	state_add(STATE_GENFLAGS, "GENFLAGS", m_TF).formatstr("%16s").noshow();

	set_icountptr(m_icount);
}


void i8086_common_cpu_device::device_reset()
{
	m_ZeroVal = 1;
	m_ParityVal = 1;
	m_regs.w[AX] = 0;
	m_regs.w[CX] = 0;
	m_regs.w[DX] = 0;
	m_regs.w[BX] = 0;
	m_regs.w[SP] = 0;
	m_regs.w[BP] = 0;
	m_regs.w[SI] = 0;
	m_regs.w[DI] = 0;
	m_sregs[ES] = 0;
	m_sregs[CS] = 0xffff;
	m_sregs[SS] = 0;
	m_sregs[DS] = 0;
	m_ip = 0;
	m_prev_ip = 0;
	m_SignVal = 0;
	m_AuxVal = 0;
	m_OverVal = 0;
	m_CarryVal = 0;
	m_TF = 0;
	m_IF = 0;
	m_DF = 0;
	m_IOPL = 3; // 8086 IOPL always 3
	m_NT = 1; // 8086 NT always 1
	m_MF = 1; // 8086 MF always 1, 80286 always 0
	m_int_vector = 0;
	m_pending_irq = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_no_interrupt = 0;
	m_fire_trap = 0;
	m_prefix_seg = 0;
	m_seg_prefix = false;
	m_seg_prefix_next = false;
	m_ea = 0;
	m_eo = 0;
	m_modrm = 0;
	m_dst = 0;
	m_src = 0;
	m_halt = false;
	m_lock = false;
	m_easeg = DS;
}



void i8086_common_cpu_device::interrupt(int int_num, int trap)
{
	PUSH( CompressFlags() );
	m_TF = m_IF = 0;

	if (int_num == -1)
		int_num = standard_irq_callback(0, (m_sregs[CS] << 4) + m_ip);
	debugger_exception_hook(int_num);

	m_easeg = CS;
	uint16_t dest_off = read_word(int_num * 4 + 0);
	uint16_t dest_seg = read_word(int_num * 4 + 2);

	PUSH(m_sregs[CS]);
	PUSH(m_ip);
	m_prev_ip = m_ip = dest_off;
	m_sregs[CS] = dest_seg;
}


void i8086_common_cpu_device::execute_set_input( int inptnum, int state )
{
	if (inptnum == INPUT_LINE_NMI)
	{
		if ( m_nmi_state == state )
		{
			return;
		}
		m_nmi_state = state;
		if (state != CLEAR_LINE)
		{
			m_pending_irq |= NMI_IRQ;
		}
	}
	else if (inptnum == INPUT_LINE_TEST)
	{
		m_test_state = state;
	}
	else
	{
		m_irq_state = state;
		if (state == CLEAR_LINE)
		{
			m_pending_irq &= ~INT_IRQ;
		}
		else
		{
			m_pending_irq |= INT_IRQ;
		}
	}
}

std::unique_ptr<util::disasm_interface> i8086_common_cpu_device::create_disassembler()
{
	return std::make_unique<i386_disassembler>(this);
}

int i8086_common_cpu_device::get_mode() const
{
	return 1;
}

uint8_t i8086_common_cpu_device::read_byte(uint32_t addr)
{
	return m_program->read_byte(addr);
}

uint16_t i8086_common_cpu_device::read_word(uint32_t addr)
{
	return m_program->read_word_unaligned(addr);
}

void i8086_common_cpu_device::write_byte(uint32_t addr, uint8_t data)
{
	m_program->write_byte(addr, data);
}

void i8086_common_cpu_device::write_word(uint32_t addr, uint16_t data)
{
	m_program->write_word_unaligned(addr, data);
}

uint8_t i8086_common_cpu_device::read_port_byte(uint16_t port)
{
	return m_io->read_byte(port);
}

uint16_t i8086_common_cpu_device::read_port_word(uint16_t port)
{
	return m_io->read_word_unaligned(port);
}

void i8086_common_cpu_device::write_port_byte(uint16_t port, uint8_t data)
{
	m_io->write_byte(port, data);
}

void i8086_common_cpu_device::write_port_byte_al(uint16_t port)
{
	if (port & 1)
		m_io->write_word(port-1, swapendian_int16(m_regs.w[AX]), 0xff00);
	else
		m_io->write_word(port, m_regs.w[AX], 0x00ff);
}

void i8086_common_cpu_device::write_port_word(uint16_t port, uint16_t data)
{
	m_io->write_word_unaligned(port, data);
}

uint32_t i8086_common_cpu_device::calc_addr(int seg, uint16_t offset, int size, int op, bool override)
{
	if ( m_seg_prefix && (seg==DS || seg==SS) && override )
	{
		m_easeg = m_seg_prefix;
		return (m_sregs[m_prefix_seg] << 4) + offset;
	}
	else
	{
		m_easeg = seg;
		return (m_sregs[seg] << 4) + offset;
	}
}

bool i8086_common_cpu_device::common_op(uint8_t op)
{
	switch(op)
	{
		case 0x00: // i_add_br8
			DEF_br8();
			set_CFB(ADDB());
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x01: // i_add_wr16
			DEF_wr16();
			set_CFW(ADDX());
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x02: // i_add_r8b
			DEF_r8b();
			set_CFB(ADDB());
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x03: // i_add_r16w
			DEF_r16w();
			set_CFW(ADDX());
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x04: // i_add_ald8
			DEF_ald8();
			set_CFB(ADDB());
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x05: // i_add_axd16
			DEF_axd16();
			set_CFW(ADDX());
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x06: // i_push_es
			PUSH(m_sregs[ES]);
			CLK(PUSH_SEG);
			break;

		case 0x07: // i_pop_es
			m_sregs[ES] = POP();
			CLK(POP_SEG);
			break;

		case 0x08: // i_or_br8
			DEF_br8();
			ORB();
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x09: // i_or_wr16
			DEF_wr16();
			ORW();
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x0a: // i_or_r8b
			DEF_r8b();
			ORB();
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x0b: // i_or_r16w
			DEF_r16w();
			ORW();
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x0c: // i_or_ald8
			DEF_ald8();
			ORB();
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x0d: // i_or_axd16
			DEF_axd16();
			ORW();
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x0e: // i_push_cs
			PUSH(m_sregs[CS]);
			CLK(PUSH_SEG);
			break;

		case 0x10: // i_adc_br8
		{
			DEF_br8();
			m_src += CF ? 1 : 0;
			uint32_t tmpcf = ADDB();
			PutbackRMByte(m_dst);
			set_CFB(tmpcf);
			CLKM(ALU_RR8,ALU_MR8);
			break;
		}
		case 0x11: // i_adc_wr16
		{
			DEF_wr16();
			m_src += CF ? 1 : 0;
			uint32_t tmpcf = ADDX();
			PutbackRMWord(m_dst);
			set_CFW(tmpcf);
			CLKM(ALU_RR16,ALU_MR16);
			break;
		}

		case 0x12: // i_adc_r8b
			DEF_r8b();
			m_src += CF ? 1 : 0;
			set_CFB(ADDB());
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x13: // i_adc_r16w
			DEF_r16w();
			m_src += CF ? 1 : 0;
			set_CFW(ADDX());
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x14: // i_adc_ald8
			DEF_ald8();
			m_src += CF ? 1 : 0;
			set_CFB(ADDB());
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x15: // i_adc_axd16
			DEF_axd16();
			m_src += CF ? 1 : 0;
			set_CFW(ADDX());
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x16: // i_push_ss
			PUSH(m_sregs[SS]);
			CLK(PUSH_SEG);
			break;

		case 0x17: // i_pop_ss
			m_sregs[SS] = POP();
			CLK(POP_SEG);
			m_no_interrupt = 1;
			break;

		case 0x18: // i_sbb_br8
		{
			uint32_t tmpcf;
			DEF_br8();
			m_src += CF ? 1 : 0;
			tmpcf = SUBB();
			PutbackRMByte(m_dst);
			set_CFB(tmpcf);
			CLKM(ALU_RR8,ALU_MR8);
			break;
		}

		case 0x19: // i_sbb_wr16
		{
			uint32_t tmpcf;
			DEF_wr16();
			m_src += CF ? 1 : 0;
			tmpcf = SUBX();
			PutbackRMWord(m_dst);
			set_CFW(tmpcf);
			CLKM(ALU_RR16,ALU_MR16);
			break;
		}

		case 0x1a: // i_sbb_r8b
			DEF_r8b();
			m_src += CF ? 1 : 0;
			set_CFB(SUBB());
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x1b: // i_sbb_r16w
			DEF_r16w();
			m_src += CF ? 1 : 0;
			set_CFW(SUBX());
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x1c: // i_sbb_ald8
			DEF_ald8();
			m_src += CF ? 1 : 0;
			set_CFB(SUBB());
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x1d: // i_sbb_axd16
			DEF_axd16();
			m_src += CF ? 1 : 0;
			set_CFW(SUBX());
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x1e: // i_push_ds
			PUSH(m_sregs[DS]);
			CLK(PUSH_SEG);
			break;

		case 0x1f: // i_pop_ds
			m_sregs[DS] = POP();
			CLK(POP_SEG);
			break;


		case 0x20: // i_and_br8
			DEF_br8();
			ANDB();
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x21: // i_and_wr16
			DEF_wr16();
			ANDX();
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x22: // i_and_r8b
			DEF_r8b();
			ANDB();
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x23: // i_and_r16w
			DEF_r16w();
			ANDX();
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x24: // i_and_ald8
			DEF_ald8();
			ANDB();
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x25: // i_and_axd16
			DEF_axd16();
			ANDX();
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x26: // i_es
			m_seg_prefix_next = true;
			m_prefix_seg = ES;
			CLK(OVERRIDE);
			break;

		case 0x27: // i_daa
			ADJ4(6,0x60);
			CLK(DAA);
			break;


		case 0x28: // i_sub_br8
			DEF_br8();
			set_CFB(SUBB());
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x29: // i_sub_wr16
			DEF_wr16();
			set_CFW(SUBX());
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x2a: // i_sub_r8b
			DEF_r8b();
			set_CFB(SUBB());
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x2b: // i_sub_r16w
			DEF_r16w();
			set_CFW(SUBX());
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x2c: // i_sub_ald8
			DEF_ald8();
			set_CFB(SUBB());
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x2d: // i_sub_axd16
			DEF_axd16();
			set_CFW(SUBX());
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x2e: // i_cs
			m_seg_prefix_next = true;
			m_prefix_seg = CS;
			CLK(OVERRIDE);
			break;

		case 0x2f: // i_das
			ADJ4(-6,-0x60);
			CLK(DAS);
			break;


		case 0x30: // i_xor_br8
			DEF_br8();
			XORB();
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x31: // i_xor_wr16
			DEF_wr16();
			XORW();
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x32: // i_xor_r8b
			DEF_r8b();
			XORB();
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x33: // i_xor_r16w
			DEF_r16w();
			XORW();
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x34: // i_xor_ald8
			DEF_ald8();
			XORB();
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x35: // i_xor_axd16
			DEF_axd16();
			XORW();
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x36: // i_ss
			m_seg_prefix_next = true;
			m_prefix_seg = SS;
			CLK(OVERRIDE);
			break;

		case 0x37: // i_aaa
			ADJB(6, (m_regs.b[AL] > 0xf9) ? 2 : 1);
			CLK(AAA);
			break;


		case 0x38: // i_cmp_br8
			DEF_br8();
			set_CFB(SUBB());
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x39: // i_cmp_wr16
			DEF_wr16();
			set_CFW(SUBX());
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x3a: // i_cmp_r8b
			DEF_r8b();
			set_CFB(SUBB());
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x3b: // i_cmp_r16w
			DEF_r16w();
			set_CFW(SUBX());
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x3c: // i_cmp_ald8
			DEF_ald8();
			set_CFB(SUBB());
			CLK(ALU_RI8);
			break;

		case 0x3d: // i_cmp_axd16
			DEF_axd16();
			set_CFW(SUBX());
			CLK(ALU_RI16);
			break;

		case 0x3e: // i_ds
			m_seg_prefix_next = true;
			m_prefix_seg = DS;
			CLK(OVERRIDE);
			break;

		case 0x3f: // i_aas
			ADJB(-6, (m_regs.b[AL] < 6) ? -2 : -1);
			CLK(AAS);
			break;


		case 0x40: // i_inc_ax
			IncWordReg(AX);
			CLK(INCDEC_R16);
			break;

		case 0x41: // i_inc_cx
			IncWordReg(CX);
			CLK(INCDEC_R16);
			break;

		case 0x42: // i_inc_dx
			IncWordReg(DX);
			CLK(INCDEC_R16);
			break;

		case 0x43: // i_inc_bx
			IncWordReg(BX);
			CLK(INCDEC_R16);
			break;

		case 0x44: // i_inc_sp
			IncWordReg(SP);
			CLK(INCDEC_R16);
			break;

		case 0x45: // i_inc_bp
			IncWordReg(BP);
			CLK(INCDEC_R16);
			break;

		case 0x46: // i_inc_si
			IncWordReg(SI);
			CLK(INCDEC_R16);
			break;

		case 0x47: // i_inc_di
			IncWordReg(DI);
			CLK(INCDEC_R16);
			break;


		case 0x48: // i_dec_ax
			DecWordReg(AX);
			CLK(INCDEC_R16);
			break;

		case 0x49: // i_dec_cx
			DecWordReg(CX);
			CLK(INCDEC_R16);
			break;

		case 0x4a: // i_dec_dx
			DecWordReg(DX);
			CLK(INCDEC_R16);
			break;

		case 0x4b: // i_dec_bx
			DecWordReg(BX);
			CLK(INCDEC_R16);
			break;

		case 0x4c: // i_dec_sp
			DecWordReg(SP);
			CLK(INCDEC_R16);
			break;

		case 0x4d: // i_dec_bp
			DecWordReg(BP);
			CLK(INCDEC_R16);
			break;

		case 0x4e: // i_dec_si
			DecWordReg(SI);
			CLK(INCDEC_R16);
			break;

		case 0x4f: // i_dec_di
			DecWordReg(DI);
			CLK(INCDEC_R16);
			break;


		case 0x50: // i_push_ax
			PUSH(m_regs.w[AX]);
			CLK(PUSH_R16);
			break;

		case 0x51: // i_push_cx
			PUSH(m_regs.w[CX]);
			CLK(PUSH_R16);
			break;

		case 0x52: // i_push_dx
			PUSH(m_regs.w[DX]);
			CLK(PUSH_R16);
			break;

		case 0x53: // i_push_bx
			PUSH(m_regs.w[BX]);
			CLK(PUSH_R16);
			break;

		case 0x54: // i_push_sp
			PUSH(m_regs.w[SP]-2);
			CLK(PUSH_R16);
			break;

		case 0x55: // i_push_bp
			PUSH(m_regs.w[BP]);
			CLK(PUSH_R16);
			break;

		case 0x56: // i_push_si
			PUSH(m_regs.w[SI]);
			CLK(PUSH_R16);
			break;

		case 0x57: // i_push_di
			PUSH(m_regs.w[DI]);
			CLK(PUSH_R16);
			break;


		case 0x58: // i_pop_ax
			m_regs.w[AX] = POP();
			CLK(POP_R16);
			break;

		case 0x59: // i_pop_cx
			m_regs.w[CX] = POP();
			CLK(POP_R16);
			break;

		case 0x5a: // i_pop_dx
			m_regs.w[DX] = POP();
			CLK(POP_R16);
			break;

		case 0x5b: // i_pop_bx
			m_regs.w[BX] = POP();
			CLK(POP_R16);
			break;

		case 0x5c: // i_pop_sp
			m_regs.w[SP] = POP();
			CLK(POP_R16);
			break;

		case 0x5d: // i_pop_bp
			m_regs.w[BP] = POP();
			CLK(POP_R16);
			break;

		case 0x5e: // i_pop_si
			m_regs.w[SI] = POP();
			CLK(POP_R16);
			break;

		case 0x5f: // i_pop_di
			m_regs.w[DI] = POP();
			CLK(POP_R16);
			break;

// 8086 'invalid opcodes', as documented at http://www.os2museum.com/wp/?p=2147 and tested on real hardware
// - 0x60 - 0x6f are aliases to 0x70 - 0x7f.
// - 0xc0, 0xc1, 0xc8, 0xc9 are also aliases where the CPU ignores BIT 1 (*).
// - 0xf1 is an alias to 0xf0.
//
//      Instructions are used in the boot sector for some versions of
//      MS-DOS  (e.g. the DEC Rainbow-100 version of DOS 2.x)
		case 0x60:
		case 0x70: // i_jo
			JMP( OF);
			break;

		case 0x61:
		case 0x71: // i_jno
			JMP(!OF);
			break;

		case 0x62:
		case 0x72: // i_jc
			JMP( CF);
			break;

		case 0x63:
		case 0x73: // i_jnc
			JMP(!CF);
			break;

		case 0x64:
		case 0x74: // i_jz
			JMP( ZF);
			break;

		case 0x65:
		case 0x75: // i_jnz
			JMP(!ZF);
			break;

		case 0x66:
		case 0x76: // i_jce
			JMP(CF || ZF);
			break;

		case 0x67:
		case 0x77: // i_jnce
			JMP(!(CF || ZF));
			break;

		case 0x68:
		case 0x78: // i_js
			JMP( SF);
			break;

		case 0x69:
		case 0x79: // i_jns
			JMP(!SF);
			break;

		case 0x6a:
		case 0x7a: // i_jp
			JMP( PF);
			break;

		case 0x6b:
		case 0x7b: // i_jnp
			JMP(!PF);
			break;

		case 0x6c:
		case 0x7c: // i_jl
			JMP((SF!=OF)&&(!ZF));
			break;

		case 0x6d:
		case 0x7d: // i_jnl
			JMP(SF==OF);
			break;

		case 0x6e:
		case 0x7e: // i_jle
			JMP((ZF)||(SF!=OF));
			break;

		case 0x6f:
		case 0x7f: // i_jnle
			JMP((SF==OF)&&(!ZF));
			break;


		case 0x80: // i_80pre
		{
			uint32_t tmpcf;
			m_modrm = fetch();
			m_dst = GetRMByte();
			m_src = fetch();
			if (m_modrm >=0xc0 )             { CLK(ALU_RI8); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_MI8_RO); }
			else                             { CLK(ALU_MI8); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      set_CFB(ADDB()); PutbackRMByte(m_dst);   break;
			case 0x08:                      ORB();  PutbackRMByte(m_dst);   break;
			case 0x10: m_src += CF ? 1 : 0; tmpcf = ADDB(); PutbackRMByte(m_dst); set_CFB(tmpcf); break;
			case 0x18: m_src += CF ? 1 : 0; tmpcf = SUBB(); PutbackRMByte(m_dst); set_CFB(tmpcf); break;
			case 0x20:                      ANDB(); PutbackRMByte(m_dst);   break;
			case 0x28:                      set_CFB(SUBB()); PutbackRMByte(m_dst);   break;
			case 0x30:                      XORB(); PutbackRMByte(m_dst);   break;
			case 0x38:                      set_CFB(SUBB());                         break;  /* CMP */
			}
			break;
		}


		case 0x81: // i_81pre
		{
			uint32_t tmpcf;
			m_modrm = fetch();
			m_dst = GetRMWord();
			m_src = fetch_word();
			if (m_modrm >=0xc0 )             { CLK(ALU_RI16); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_MI16_RO); }
			else                             { CLK(ALU_MI16); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      set_CFW(ADDX()); PutbackRMWord(m_dst);   break;
			case 0x08:                      ORW();  PutbackRMWord(m_dst);   break;
			case 0x10: m_src += CF ? 1 : 0; tmpcf = ADDX(); PutbackRMWord(m_dst); set_CFW(tmpcf); break;
			case 0x18: m_src += CF ? 1 : 0; tmpcf = SUBX(); PutbackRMWord(m_dst); set_CFW(tmpcf); break;
			case 0x20:                      ANDX(); PutbackRMWord(m_dst);   break;
			case 0x28:                      set_CFW(SUBX()); PutbackRMWord(m_dst);   break;
			case 0x30:                      XORW(); PutbackRMWord(m_dst);   break;
			case 0x38:                      set_CFW(SUBX());                         break;  /* CMP */
			}
			break;
		}


		case 0x82: // i_82pre
		{
			uint32_t tmpcf;
			m_modrm = fetch();
			m_dst = GetRMByte();
			m_src = (int8_t)fetch();
			if (m_modrm >=0xc0 )             { CLK(ALU_RI8); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_MI8_RO); }
			else                             { CLK(ALU_MI8); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      set_CFB(ADDB()); PutbackRMByte(m_dst);   break;
			case 0x08:                      ORB();  PutbackRMByte(m_dst);   break;
			case 0x10: m_src += CF ? 1 : 0; tmpcf = ADDB(); PutbackRMByte(m_dst); set_CFB(tmpcf); break;
			case 0x18: m_src += CF ? 1 : 0; tmpcf = SUBB(); PutbackRMByte(m_dst); set_CFB(tmpcf); break;
			case 0x20:                      ANDB(); PutbackRMByte(m_dst);   break;
			case 0x28:                      set_CFB(SUBB()); PutbackRMByte(m_dst);   break;
			case 0x30:                      XORB(); PutbackRMByte(m_dst);   break;
			case 0x38:                      set_CFB(SUBB());                         break; /* CMP */
			}
			break;
		}


		case 0x83: // i_83pre
		{
			uint32_t tmpcf;
			m_modrm = fetch();
			m_dst = GetRMWord();
			m_src = (uint16_t)((int16_t)((int8_t)fetch()));
			if (m_modrm >=0xc0 )             { CLK(ALU_R16I8); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_M16I8_RO); }
			else                             { CLK(ALU_M16I8); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      set_CFW(ADDX()); PutbackRMWord(m_dst); break;
			case 0x08:                      ORW();  PutbackRMWord(m_dst); break;
			case 0x10: m_src += CF ? 1 : 0; tmpcf = ADDX(); PutbackRMWord(m_dst); set_CFW(tmpcf); break;
			case 0x18: m_src += CF ? 1 : 0; tmpcf = SUBX(); PutbackRMWord(m_dst); set_CFW(tmpcf); break;
			case 0x20:                      ANDX(); PutbackRMWord(m_dst); break;
			case 0x28:                      set_CFW(SUBX()); PutbackRMWord(m_dst); break;
			case 0x30:                      XORW(); PutbackRMWord(m_dst); break;
			case 0x38:                      set_CFW(SUBX());                       break; /* CMP */
			}
			break;
		}


		case 0x84: // i_test_br8
			DEF_br8();
			ANDB();
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x85: // i_test_wr16
			DEF_wr16();
			ANDX();
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x86: // i_xchg_br8
			DEF_br8();
			RegByte(m_dst);
			PutbackRMByte(m_src);
			CLKM(XCHG_RR8,XCHG_RM8);
			break;

		case 0x87: // i_xchg_wr16
			DEF_wr16();
			RegWord(m_dst);
			PutbackRMWord(m_src);
			CLKM(XCHG_RR16,XCHG_RM16);
			break;


		case 0x88: // i_mov_br8
			m_modrm = fetch();
			m_src = RegByte();
			PutRMByte(m_src);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x89: // i_mov_wr16
			m_modrm = fetch();
			m_src = RegWord();
			PutRMWord(m_src);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x8a: // i_mov_r8b
			m_modrm = fetch();
			m_src = GetRMByte();
			RegByte(m_src);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x8b: // i_mov_r16w
			m_modrm = fetch();
			m_src = GetRMWord();
			RegWord(m_src);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x8c: // i_mov_wsreg
			m_modrm = fetch();
			PutRMWord(m_sregs[(m_modrm & 0x18) >> 3]); // confirmed on hw: modrm bit 5 ignored
			CLKM(MOV_RS,MOV_MS);
			break;

		case 0x8d: // i_lea
			m_modrm = fetch();
			get_ea(0, I8086_NONE);
			RegWord(m_eo);
			CLK(LEA);
			break;

		case 0x8e: // i_mov_sregw
			m_modrm = fetch();
			m_src = GetRMWord();
			m_sregs[(m_modrm & 0x18) >> 3] = m_src; // confirmed on hw: modrm bit 5 ignored
			CLKM(MOV_SR,MOV_SM);
			m_no_interrupt = 1; // Disable IRQ after load segment register.
			break;

		case 0x8f: // i_popw
			m_modrm = fetch();
			PutRMWord( POP() );
			CLKM(POP_R16,POP_M16);
			break;

		case 0x90: // i_nop
			CLK(NOP);
			break;

		case 0x91: // i_xchg_axcx
			XchgAXReg(CX);
			CLK(XCHG_AR16);
			break;

		case 0x92: // i_xchg_axdx
			XchgAXReg(DX);
			CLK(XCHG_AR16);
			break;

		case 0x93: // i_xchg_axbx
			XchgAXReg(BX);
			CLK(XCHG_AR16);
			break;

		case 0x94: // i_xchg_axsp
			XchgAXReg(SP);
			CLK(XCHG_AR16);
			break;

		case 0x95: // i_xchg_axbp
			XchgAXReg(BP);
			CLK(XCHG_AR16);
			break;

		case 0x96: // i_xchg_axsi
			XchgAXReg(SI);
			CLK(XCHG_AR16);
			break;

		case 0x97: // i_xchg_axdi
			XchgAXReg(DI);
			CLK(XCHG_AR16);
			break;


		case 0x98: // i_cbw
			m_regs.b[AH] = (m_regs.b[AL] & 0x80) ? 0xff : 0;
			CLK(CBW);
			break;

		case 0x99: // i_cwd
			m_regs.w[DX] = (m_regs.b[AH] & 0x80) ? 0xffff : 0;
			CLK(CWD);
			break;

		case 0x9a: // i_call_far
			{
				uint16_t tmp = fetch_word();
				uint16_t tmp2 = fetch_word();
				PUSH(m_sregs[CS]);
				PUSH(m_ip);
				m_ip = tmp;
				m_sregs[CS] = tmp2;
				CLK(CALL_FAR);
			}
			break;

		case 0x9b: // i_wait
			// Wait for assertion of /TEST
			if (m_test_state == 0)
			{
				m_icount = 0;
				m_ip--;
			}
			else
				CLK(WAIT);
			break;

		case 0x9c: // i_pushf
			PUSH( CompressFlags() );
			CLK(PUSHF);
			break;

		case 0x9d: // i_popf
			i_popf();
			break;

		case 0x9e: // i_sahf
			{
				uint32_t tmp = (CompressFlags() & 0xff00) | (m_regs.b[AH] & 0xd5);
				ExpandFlags(tmp);
				CLK(SAHF);
			}
			break;

		case 0x9f: // i_lahf
			m_regs.b[AH] = CompressFlags();
			CLK(LAHF);
			break;


		case 0xa0: // i_mov_aldisp
			{
				uint32_t addr = fetch_word();
				m_regs.b[AL] = GetMemB(DS, addr);
				CLK(MOV_AM8);
			}
			break;

		case 0xa1: // i_mov_axdisp
			{
				uint32_t addr = fetch_word();
				m_regs.w[AX] = GetMemW(DS, addr);
				CLK(MOV_AM16);
			}
			break;

		case 0xa2: // i_mov_dispal
			{
				uint32_t addr = fetch_word();
				PutMemB(DS, addr, m_regs.b[AL]);
				CLK(MOV_MA8);
			}
			break;

		case 0xa3: // i_mov_dispax
			{
				uint32_t addr = fetch_word();
				PutMemW(DS, addr, m_regs.w[AX]);
				CLK(MOV_MA16);
			}
			break;

		case 0xa4: // i_movsb
			i_movsb();
			break;

		case 0xa5: // i_movsw
			i_movsw();
			break;

		case 0xa6: // i_cmpsb
			i_cmpsb();
			break;

		case 0xa7: // i_cmpsw
			i_cmpsw();
			break;


		case 0xa8: // i_test_ald8
			DEF_ald8();
			ANDB();
			CLK(ALU_RI8);
			break;

		case 0xa9: // i_test_axd16
			DEF_axd16();
			ANDX();
			CLK(ALU_RI16);
			break;

		case 0xaa: // i_stosb
			i_stosb();
			break;

		case 0xab: // i_stosw
			i_stosw();
			break;

		case 0xac: // i_lodsb
			i_lodsb();
			break;

		case 0xad: // i_lodsw
			i_lodsw();
			break;

		case 0xae: // i_scasb
			i_scasb();
			break;

		case 0xaf: // i_scasw
			i_scasw();
			break;


		case 0xb0: // i_mov_ald8
			m_regs.b[AL] = fetch();
			CLK(MOV_RI8);
			break;

		case 0xb1: // i_mov_cld8
			m_regs.b[CL] = fetch();
			CLK(MOV_RI8);
			break;

		case 0xb2: // i_mov_dld8
			m_regs.b[DL] = fetch();
			CLK(MOV_RI8);
			break;

		case 0xb3: // i_mov_bld8
			m_regs.b[BL] = fetch();
			CLK(MOV_RI8);
			break;

		case 0xb4: // i_mov_ahd8
			m_regs.b[AH] = fetch();
			CLK(MOV_RI8);
			break;

		case 0xb5: // i_mov_chd8
			m_regs.b[CH] = fetch();
			CLK(MOV_RI8);
			break;

		case 0xb6: // i_mov_dhd8
			m_regs.b[DH] = fetch();
			CLK(MOV_RI8);
			break;

		case 0xb7: // i_mov_bhd8
			m_regs.b[BH] = fetch();
			CLK(MOV_RI8);
			break;


		case 0xb8: // i_mov_axd16
			m_regs.b[AL] = fetch();
			m_regs.b[AH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xb9: // i_mov_cxd16
			m_regs.b[CL] = fetch();
			m_regs.b[CH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xba: // i_mov_dxd16
			m_regs.b[DL] = fetch();
			m_regs.b[DH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xbb: // i_mov_bxd16
			m_regs.b[BL] = fetch();
			m_regs.b[BH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xbc: // i_mov_spd16
			m_regs.b[SPL] = fetch();
			m_regs.b[SPH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xbd: // i_mov_bpd16
			m_regs.b[BPL] = fetch();
			m_regs.b[BPH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xbe: // i_mov_sid16
			m_regs.b[SIL] = fetch();
			m_regs.b[SIH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xbf: // i_mov_did16
			m_regs.b[DIL] = fetch();
			m_regs.b[DIH] = fetch();
			CLK(MOV_RI16);
			break;

		case 0xc0: // 0xc0 is 0xc2 - see (*)
		case 0xc2: // i_ret_d16
			{
				uint32_t count = fetch_word();
				m_ip = POP();
				m_regs.w[SP] += count;
				CLK(RET_NEAR_IMM);
			}
			break;

		case 0xc1: // 0xc1 is 0xc3 - see (*)
		case 0xc3: // i_ret
			m_ip = POP();
			CLK(RET_NEAR);
			break;

		case 0xc4: // i_les_dw
			m_modrm = fetch();
			RegWord( GetRMWord() );
			m_sregs[ES] = GetnextRMWord();
			CLK(LOAD_PTR);
			break;

		case 0xc5: // i_lds_dw
			m_modrm = fetch();
			RegWord( GetRMWord() );
			m_sregs[DS] = GetnextRMWord();
			CLK(LOAD_PTR);
			break;

		case 0xc6: // i_mov_bd8
			m_modrm = fetch();
			PutImmRMByte();
			CLKM(MOV_RI8,MOV_MI8);
			break;

		case 0xc7: // i_mov_wd16
			m_modrm = fetch();
			PutImmRMWord();
			CLKM(MOV_RI16,MOV_MI16);
			break;

		case 0xc8: // 0xc8 = 0xca - see (*)
		case 0xca: // i_retf_d16
			{
				uint32_t count = fetch_word();
				m_ip = POP();
				m_sregs[CS] = POP();
				m_regs.w[SP] += count;
				CLK(RET_FAR_IMM);
			}
			break;

		case 0xc9: // 0xc9 = 0xcb  - see (*)
		case 0xcb: // i_retf
			m_ip = POP();
			m_sregs[CS] = POP();
			CLK(RET_FAR);
			break;

		case 0xcc: // i_int3
			interrupt(3, 0);
			CLK(INT3);
			break;

		case 0xcd: // i_int
			interrupt(fetch(), 0);
			CLK(INT_IMM);
			break;

		case 0xce: // i_into
			if (OF)
			{
				interrupt(4, 0);
				CLK(INTO_T);
			}
			else
				CLK(INTO_NT);
			break;

		case 0xcf: // i_iret
			m_ip = POP();
			m_sregs[CS] = POP();
			i_popf();
			CLK(IRET);
			break;

		case 0xd0: // i_rotshft_b
			m_modrm = fetch();
			m_src = GetRMByte();
			m_dst = m_src;
			CLKM(ROT_REG_1,ROT_M8_1);
			switch ( m_modrm & 0x38 )
			{
			case 0x00: ROL_BYTE();  PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
			case 0x08: ROR_BYTE();  PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
			case 0x10: ROLC_BYTE(); PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
			case 0x18: RORC_BYTE(); PutbackRMByte(m_dst); m_OverVal = (m_src ^ m_dst) & 0x80; break;
			case 0x30:
			case 0x20: SHL_BYTE(1); m_OverVal = (m_src ^ m_dst) & 0x80; break;
			case 0x28: SHR_BYTE(1); m_OverVal = (m_src ^ m_dst) & 0x80; break;
			case 0x38: SHRA_BYTE(1); m_OverVal = 0; break;
			}
			break;

		case 0xd1: // i_rotshft_w
			m_modrm = fetch();
			m_src = GetRMWord();
			m_dst = m_src;
			CLKM(ROT_REG_1,ROT_M8_1);
			switch ( m_modrm & 0x38 )
			{
			case 0x00: ROL_WORD();  PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
			case 0x08: ROR_WORD();  PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
			case 0x10: ROLC_WORD(); PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
			case 0x18: RORC_WORD(); PutbackRMWord(m_dst); m_OverVal = (m_src ^ m_dst) & 0x8000; break;
			case 0x30:
			case 0x20: SHL_WORD(1); m_OverVal = (m_src ^ m_dst) & 0x8000;  break;
			case 0x28: SHR_WORD(1); m_OverVal = (m_src ^ m_dst) & 0x8000;  break;
			case 0x38: SHRA_WORD(1); m_OverVal = 0; break;
			}
			break;

		case 0xd4: // i_aam
		{
			uint8_t base = fetch();
			if(!base)
			{
				interrupt(0);
				break;
			}
			m_regs.b[AH] = m_regs.b[AL] / base;
			m_regs.b[AL] %= base;
			set_SZPF_Word(m_regs.w[AX]);
			CLK(AAM);
			break;
		}

		case 0xd5: // i_aad
		{
			uint8_t base = fetch();
			m_regs.b[AL] = m_regs.b[AH] * base + m_regs.b[AL];
			m_regs.b[AH] = 0;
			set_SZPF_Byte(m_regs.b[AL]);
			CLK(AAD);
			break;
		}

		case 0xd6: // i_salc
			m_regs.b[AL] = (CF ? 0xff : 0);
			CLK(ALU_RR8);  // is sbb al,al
			break;

		case 0xd7: // i_trans
			m_regs.b[AL] = GetMemB( DS, m_regs.w[BX] + m_regs.b[AL] );
			CLK(XLAT);
			break;

		case 0xe0: // i_loopne
			{
				int8_t disp = (int8_t)fetch();

				m_regs.w[CX]--;
				if (!ZF && m_regs.w[CX])
				{
					m_ip = m_ip + disp;
					CLK(LOOP_T);
				}
				else
					CLK(LOOP_NT);
			}
			break;

		case 0xe1: // i_loope
			{
				int8_t disp = (int8_t)fetch();

				m_regs.w[CX]--;
				if (ZF && m_regs.w[CX])
				{
					m_ip = m_ip + disp;
					CLK(LOOPE_T);
				}
				else
					CLK(LOOPE_NT);
			}
			break;

		case 0xe2: // i_loop
			{
				int8_t disp = (int8_t)fetch();

				m_regs.w[CX]--;
				if (m_regs.w[CX])
				{
					m_ip = m_ip + disp;
					CLK(LOOP_T);
				}
				else
					CLK(LOOP_NT);
			}
			break;

		case 0xe3: // i_jcxz
			{
				int8_t disp = (int8_t)fetch();

				if (m_regs.w[CX] == 0)
				{
					m_ip = m_ip + disp;
					CLK(JCXZ_T);
				}
				else
					CLK(JCXZ_NT);
			}
			break;

		case 0xe4: // i_inal
			if (m_lock) m_lock_handler(1);
			m_regs.b[AL] = read_port_byte( fetch() );
			if (m_lock) { m_lock_handler(0); m_lock = false; }
			CLK(IN_IMM8);
			break;

		case 0xe5: // i_inax
			{
				uint8_t port = fetch();

				m_regs.w[AX] = read_port_word(port);
				CLK(IN_IMM16);
			}
			break;

		case 0xe6: // i_outal
			write_port_byte_al(fetch());
			CLK(OUT_IMM8);
			break;

		case 0xe7: // i_outax
			{
				uint8_t port = fetch();

				write_port_word(port, m_regs.w[AX]);
				CLK(OUT_IMM16);
			}
			break;


		case 0xe8: // i_call_d16
			{
				int16_t tmp = (int16_t)fetch_word();

				PUSH(m_ip);
				m_ip = m_ip + tmp;
				CLK(CALL_NEAR);
			}
			break;

		case 0xe9: // i_jmp_d16
			{
				int16_t offset = (int16_t)fetch_word();
				m_ip += offset;
				CLK(JMP_NEAR);
			}
			break;

		case 0xea: // i_jmp_far
			{
				uint16_t tmp = fetch_word();
				uint16_t tmp1 = fetch_word();

				m_sregs[CS] = tmp1;
				m_ip = tmp;
				CLK(JMP_FAR);
			}
			break;

		case 0xeb: // i_jmp_d8
			{
				int tmp = (int)((int8_t)fetch());

				CLK(JMP_SHORT);
				if (tmp==-2 && m_no_interrupt==0 && (m_pending_irq==0) && m_icount>0)
				{
					m_icount%=12; /* cycle skip */
				}
				m_ip = (uint16_t)(m_ip+tmp);
			}
			break;

		case 0xec: // i_inaldx
			m_regs.b[AL] = read_port_byte(m_regs.w[DX]);
			CLK(IN_DX8);
			break;

		case 0xed: // i_inaxdx
			{
				uint32_t port = m_regs.w[DX];

				m_regs.w[AX] = read_port_word(port);
				CLK(IN_DX16);
			}
			break;

		case 0xee: // i_outdxal
			write_port_byte_al(m_regs.w[DX]);
			CLK(OUT_DX8);
			break;

		case 0xef: // i_outdxax
			{
				uint32_t port = m_regs.w[DX];

				write_port_word(port, m_regs.w[AX]);
				CLK(OUT_DX16);
			}
			break;


		case 0xf0: // i_lock
		case 0xf1: // 0xf1 is 0xf0; verified on real CPU
			//logerror("%06x: Warning - BUSLOCK\n", m_pc); // Why warn for using lock instruction?
			m_lock = true;
			m_no_interrupt = 1;
			CLK(NOP);
			break;

		case 0xf2: // i_repne
			{
				bool invalid = false;
				uint8_t next = repx_op();
				uint16_t c = m_regs.w[CX];

				switch (next)
				{
				case 0xa4:  CLK(OVERRIDE); if (c) do { i_movsb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xa5:  CLK(OVERRIDE); if (c) do { i_movsw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xa6:  CLK(OVERRIDE); if (c) do { i_cmpsb(); c--; } while (c>0 && !ZF && m_icount>0);   m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xa7:  CLK(OVERRIDE); if (c) do { i_cmpsw(); c--; } while (c>0 && !ZF && m_icount>0);   m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xaa:  CLK(OVERRIDE); if (c) do { i_stosb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xab:  CLK(OVERRIDE); if (c) do { i_stosw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xac:  CLK(OVERRIDE); if (c) do { i_lodsb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xad:  CLK(OVERRIDE); if (c) do { i_lodsw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xae:  CLK(OVERRIDE); if (c) do { i_scasb(); c--; } while (c>0 && !ZF && m_icount>0);   m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xaf:  CLK(OVERRIDE); if (c) do { i_scasw(); c--; } while (c>0 && !ZF && m_icount>0);   m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				default:
					logerror("%06x: REPNE invalid\n", m_pc);
					// Decrement IP so the normal instruction will be executed next
					m_ip--;
					invalid = true;
					break;
				}
				if(c && !invalid)
				{
					if(!(ZF && ((next & 6) == 6)))
						m_ip = m_prev_ip;
				}
			}
			break;

		case 0xf3: // i_repe
			{
				bool invalid = false;
				uint8_t next = repx_op();
				uint16_t c = m_regs.w[CX];

				switch (next)
				{
				case 0xa4:  CLK(OVERRIDE); if (c) do { i_movsb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xa5:  CLK(OVERRIDE); if (c) do { i_movsw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xa6:  CLK(OVERRIDE); if (c) do { i_cmpsb(); c--; } while (c>0 && ZF && m_icount>0);    m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xa7:  CLK(OVERRIDE); if (c) do { i_cmpsw(); c--; } while (c>0 && ZF && m_icount>0);    m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xaa:  CLK(OVERRIDE); if (c) do { i_stosb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xab:  CLK(OVERRIDE); if (c) do { i_stosw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xac:  CLK(OVERRIDE); if (c) do { i_lodsb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xad:  CLK(OVERRIDE); if (c) do { i_lodsw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xae:  CLK(OVERRIDE); if (c) do { i_scasb(); c--; } while (c>0 && ZF && m_icount>0);    m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				case 0xaf:  CLK(OVERRIDE); if (c) do { i_scasw(); c--; } while (c>0 && ZF && m_icount>0);    m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
				default:
					logerror("%06x: REPE invalid\n", m_pc);
					// Decrement IP so the normal instruction will be executed next
					m_ip--;
					invalid = true;
					break;
				}
				if(c && !invalid)
				{
					if(!(!ZF && ((next & 6) == 6)))
						m_ip = m_prev_ip;
				}
			}
			break;

		case 0xf4: // i_hlt
			//logerror("%s: %06x: HALT\n", tag(), m_pc);
			m_icount = 0;
			m_halt = true;
			break;

		case 0xf5: // i_cmc
			m_CarryVal = !m_CarryVal;
			CLK(FLAG_OPS);
			break;

		case 0xf6: // i_f6pre
			{
				uint32_t tmp;
				uint32_t uresult,uresult2;
				int32_t result,result2;

				m_modrm = fetch();
				tmp = GetRMByte();
				switch ( m_modrm & 0x38 )
				{
				case 0x00:  /* TEST */
				case 0x08:  /* TEST (alias) */
					tmp &= fetch();
					m_CarryVal = m_OverVal = 0;
					set_SZPF_Byte(tmp);
					CLKM(ALU_RI8,ALU_MI8_RO);
					break;
				case 0x10:  /* NOT */
					PutbackRMByte(~tmp);
					CLKM(NEGNOT_R8,NEGNOT_M8);
					break;
				case 0x18:  /* NEG */
					m_dst = 0;
					m_src = tmp;
					set_CFB(SUBB());
					PutbackRMByte(m_dst);
					CLKM(NEGNOT_R8,NEGNOT_M8);
					break;
				case 0x20:  /* MUL */
					uresult = m_regs.b[AL] * tmp;
					m_regs.w[AX] = (uint16_t)uresult;
					m_CarryVal = m_OverVal = (m_regs.b[AH]!=0) ? 1 : 0;
					set_ZF(m_regs.w[AX]);
					CLKM(MUL_R8,MUL_M8);
					break;
				case 0x28:  /* IMUL */
					result = (int16_t)((int8_t)m_regs.b[AL])*(int16_t)((int8_t)tmp);
					m_regs.w[AX] = (uint16_t)result;
					m_CarryVal = m_OverVal = (m_regs.b[AH]!=0) ? 1 : 0;
					set_ZF(m_regs.w[AX]);
					CLKM(IMUL_R8,IMUL_M8);
					break;
				case 0x30:  /* DIV */
					if (tmp)
					{
						uresult = m_regs.w[AX];
						uresult2 = uresult % tmp;
						if ((uresult /= tmp) > 0xff)
						{
							interrupt(0);
						}
						else
						{
							m_regs.b[AL] = uresult;
							m_regs.b[AH] = uresult2;
						}
					}
					else
					{
						interrupt(0);
					}
					CLKM(DIV_R8,DIV_M8);
					break;
				case 0x38:  /* IDIV */
					if (tmp)
					{
						result = (int16_t)m_regs.w[AX];
						result2 = result % (int16_t)((int8_t)tmp);
						if ((result /= (int16_t)((int8_t)tmp)) > 0xff)
						{
							interrupt(0);
						}
						else
						{
							m_regs.b[AL] = result;
							m_regs.b[AH] = result2;
						}
					}
					else
					{
						interrupt(0);
					}
					CLKM(IDIV_R8,IDIV_M8);
					break;
				}
			}
			break;


		case 0xf7: // i_f7pre
			{
				uint32_t tmp,tmp2;
				uint32_t uresult,uresult2;
				int32_t result,result2;

				m_modrm = fetch();
				tmp = GetRMWord();
				switch ( m_modrm & 0x38 )
				{
				case 0x00:  /* TEST */
				case 0x08:  /* TEST (alias) */
					tmp2 = fetch_word();
					tmp &= tmp2;
					m_CarryVal = m_OverVal = 0;
					set_SZPF_Word(tmp);
					CLKM(ALU_RI16,ALU_MI16_RO);
					break;
				case 0x10:  /* NOT */
					PutbackRMWord(~tmp);
					CLKM(NEGNOT_R16,NEGNOT_M16);
					break;
				case 0x18:  /* NEG */
					m_dst = 0;
					m_src = tmp;
					set_CFW(SUBX());
					PutbackRMWord(m_dst);
					CLKM(NEGNOT_R16,NEGNOT_M16);
					break;
				case 0x20:  /* MUL */
					uresult = m_regs.w[AX]*tmp;
					m_regs.w[AX] = uresult & 0xffff;
					m_regs.w[DX] = ((uint32_t)uresult)>>16;
					m_CarryVal = m_OverVal = (m_regs.w[DX] != 0) ? 1 : 0;
					set_ZF(m_regs.w[AX] | m_regs.w[DX]);
					CLKM(MUL_R16,MUL_M16);
					break;
				case 0x28:  /* IMUL */
					result = (int32_t)((int16_t)m_regs.w[AX]) * (int32_t)((int16_t)tmp);
					m_regs.w[AX] = result & 0xffff;
					m_regs.w[DX] = result >> 16;
					m_CarryVal = m_OverVal = (m_regs.w[DX] != 0) ? 1 : 0;
					set_ZF(m_regs.w[AX] | m_regs.w[DX]);
					CLKM(IMUL_R16,IMUL_M16);
					break;
				case 0x30:  /* DIV */
					if (tmp)
					{
						uresult = (((uint32_t)m_regs.w[DX]) << 16) | m_regs.w[AX];
						uresult2 = uresult % tmp;
						if ((uresult /= tmp) > 0xffff)
						{
							interrupt(0);
						}
						else
						{
							m_regs.w[AX] = uresult;
							m_regs.w[DX] = uresult2;
						}
					}
					else
					{
						interrupt(0);
					}
					CLKM(DIV_R16,DIV_M16);
					break;
				case 0x38:  /* IDIV */
					if (tmp)
					{
						result = ((uint32_t)m_regs.w[DX] << 16) + m_regs.w[AX];
						result2 = result % (int32_t)((int16_t)tmp);
						if ((result /= (int32_t)((int16_t)tmp)) > 0xffff)
						{
							interrupt(0);
						}
						else
						{
							m_regs.w[AX] = result;
							m_regs.w[DX] = result2;
						}
					}
					else
					{
						interrupt(0);
					}
					CLKM(IDIV_R16,IDIV_M16);
					break;
				}
			}
			break;


		case 0xf8: // i_clc
			m_CarryVal = 0;
			CLK(FLAG_OPS);
			break;

		case 0xf9: // i_stc
			m_CarryVal = 1;
			CLK(FLAG_OPS);
			break;

		case 0xfa: // i_cli
			m_IF = 0;
			CLK(FLAG_OPS);
			break;

		case 0xfb: // i_sti
			m_IF = 1;
			m_no_interrupt = 1;
			CLK(FLAG_OPS);
			break;

		case 0xfc: // i_cld
			m_DF = 0;
			CLK(FLAG_OPS);
			break;

		case 0xfd: // i_std
			m_DF = 1;
			CLK(FLAG_OPS);
			break;

		case 0xfe: // i_fepre
			{
				uint32_t tmp, tmp1;
				m_modrm = fetch();
				tmp = GetRMByte();
				switch ( m_modrm & 0x38 )
				{
				case 0x00:  /* INC */
					tmp1 = tmp+1;
					m_OverVal = (tmp==0x7f);
					set_AF(tmp1,tmp,1);
					set_SZPF_Byte(tmp1);
					PutbackRMByte(tmp1);
					CLKM(INCDEC_R8,INCDEC_M8);
					break;
				case 0x08:  /* DEC */
					tmp1 = tmp-1;
					m_OverVal = (tmp==0x80);
					set_AF(tmp1,tmp,1);
					set_SZPF_Byte(tmp1);
					PutbackRMByte(tmp1);
					CLKM(INCDEC_R8,INCDEC_M8);
					break;
				default:
					logerror("%06x: FE Pre with unimplemented mod\n", m_pc);
					break;
				}
			}
			break;

		case 0xff: // i_ffpre
			{
				uint32_t tmp, tmp1;
				m_modrm = fetch();
				tmp = GetRMWord();
				switch ( m_modrm & 0x38 )
				{
				case 0x00:  /* INC */
					tmp1 = tmp+1;
					m_OverVal = (tmp==0x7fff);
					set_AF(tmp1,tmp,1);
					set_SZPF_Word(tmp1);
					PutbackRMWord(tmp1);
					CLKM(INCDEC_R16,INCDEC_M16);
					break;
				case 0x08:  /* DEC */
					tmp1 = tmp-1;
					m_OverVal = (tmp==0x8000);
					set_AF(tmp1,tmp,1);
					set_SZPF_Word(tmp1);
					PutbackRMWord(tmp1);
					CLKM(INCDEC_R16,INCDEC_M16);
					break;
				case 0x10:  /* CALL */
					PUSH(m_ip);
					m_ip = tmp;
					CLKM(CALL_R16,CALL_M16);
					break;
				case 0x18:  /* CALL FAR */
					tmp1 = m_sregs[CS];
					m_sregs[CS] = GetnextRMWord();
					PUSH(tmp1);
					PUSH(m_ip);
					m_ip = tmp;
					CLK(CALL_M32);
					break;
				case 0x20:  /* JMP */
					m_ip = tmp;
					CLKM(JMP_R16,JMP_M16);
					break;
				case 0x28:  /* JMP FAR */
					m_ip = tmp;
					m_sregs[CS] = GetnextRMWord();
					CLK(JMP_M32);
					break;
				case 0x30:
					PUSH(tmp);
					CLKM(PUSH_R16,PUSH_M16);
					break;
				default:
					m_icount -= 10;
					logerror("%06x: FF Pre with unimplemented mod\n", m_pc);
					break;
				}
			}
			break;
		default:
			return false;
	}
	return true;
}
