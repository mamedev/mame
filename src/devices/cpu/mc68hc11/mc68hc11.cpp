// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese, hap
/*
   Motorola MC68HC11 emulator

   Written by Ville Linde & Angelo Salese

TODO:
- Interrupts handling is really bare-bones, just to make Hit Poker happy;
- Timers are really sketchy as per now, only TOC1 is emulated so far;
- Complete opcodes hook-up;
- Emulate the MC68HC12 (same as HC11 with a bunch of new opcodes);

 */

#include "emu.h"
#include "debugger.h"
#include "mc68hc11.h"

enum
{
	HC11_PC = 1,
	HC11_SP,
	HC11_A,
	HC11_B,
	HC11_IX,
	HC11_IY
};

#define CC_S    0x80
#define CC_X    0x40
#define CC_H    0x20
#define CC_I    0x10
#define CC_N    0x08
#define CC_Z    0x04
#define CC_V    0x02
#define CC_C    0x01

static const int div_tab[4] = { 1, 4, 8, 16 };


const device_type MC68HC11 = &device_creator<mc68hc11_cpu_device>;


mc68hc11_cpu_device::mc68hc11_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MC68HC11, "MC68HC11", tag, owner, clock, "mc68hc11", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0 )
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
	/* defaults it to the HC11M0 version for now (I might strip this down on a later date) */
	, m_has_extended_io(1)
	, m_internal_ram_size(1280)
	, m_init_value(0x01)
{
}


offs_t mc68hc11_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( hc11 );
	return CPU_DISASSEMBLE_NAME(hc11)(this, buffer, pc, oprom, opram, options);
}


#define HC11OP(XX)      mc68hc11_cpu_device::hc11_##XX

/*****************************************************************************/
/* Internal registers */

UINT8 mc68hc11_cpu_device::hc11_regs_r(UINT32 address)
{
	int reg = address & 0xff;

	switch(reg)
	{
		case 0x00:      /* PORTA */
			return m_io->read_byte(MC68HC11_IO_PORTA);
		case 0x01:      /* DDRA */
			return 0;
		case 0x02:      /* PIOC */
			return 0;
		case 0x03:      /* PORTC */
			return m_io->read_byte(MC68HC11_IO_PORTC);
		case 0x04:      /* PORTB */
			return m_io->read_byte(MC68HC11_IO_PORTB);
		case 0x08:      /* PORTD */
			return m_io->read_byte(MC68HC11_IO_PORTD);
		case 0x09:      /* DDRD */
			return 0;
		case 0x0a:      /* PORTE */
			return m_io->read_byte(MC68HC11_IO_PORTE);
		case 0x0e:      /* TCNT */
			return m_tcnt >> 8;
		case 0x0f:
			return m_tcnt & 0xff;
		case 0x16:      /* TOC1 */
			return m_toc1 >> 8;
		case 0x17:
			return m_toc1 & 0xff;
		case 0x23:
			return m_tflg1;
		case 0x28:      /* SPCR1 */
			return 0;
		case 0x30:      /* ADCTL */
			return 0x80;
		case 0x31:      /* ADR1 */
		{
			if (m_adctl & 0x10)
			{
				return m_io->read_byte((m_adctl & 0x4) + MC68HC11_IO_AD0);
			}
			else
			{
				return m_io->read_byte((m_adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x32:      /* ADR2 */
		{
			if (m_adctl & 0x10)
			{
				return m_io->read_byte((m_adctl & 0x4) + MC68HC11_IO_AD1);
			}
			else
			{
				return m_io->read_byte((m_adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x33:      /* ADR3 */
		{
			if (m_adctl & 0x10)
			{
				return m_io->read_byte((m_adctl & 0x4) + MC68HC11_IO_AD2);
			}
			else
			{
				return m_io->read_byte((m_adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x34:      /* ADR4 */
		{
			if (m_adctl & 0x10)
			{
				return m_io->read_byte((m_adctl & 0x4) + MC68HC11_IO_AD3);
			}
			else
			{
				return m_io->read_byte((m_adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x38:      /* OPT2 */
			return 0;
		case 0x70:      /* SCBDH */
			return 0;
		case 0x71:      /* SCBDL */
			return 0;
		case 0x72:      /* SCCR1 */
			return 0;
		case 0x73:      /* SCCR2 */
			return 0;
		case 0x74:      /* SCSR1 */
			return 0x40;
		case 0x7c:      /* PORTH */
			return m_io->read_byte(MC68HC11_IO_PORTH);
		case 0x7e:      /* PORTG */
			return m_io->read_byte(MC68HC11_IO_PORTG);
		case 0x7f:      /* DDRG */
			return 0;

		case 0x88:      /* SPCR2 */
			return 0;
		case 0x89:      /* SPSR2 */
			return 0x80;
		case 0x8a:      /* SPDR2 */
			return m_io->read_byte(MC68HC11_IO_SPI2_DATA);

		case 0x8b:      /* OPT4 */
			return 0;
	}

	logerror("HC11: regs_r %02X\n", reg);
	return 0; // Dummy
}

void mc68hc11_cpu_device::hc11_regs_w(UINT32 address, UINT8 value)
{
	int reg = address & 0xff;

	switch(reg)
	{
		case 0x00:      /* PORTA */
			m_io->write_byte(MC68HC11_IO_PORTA, value);
			return;
		case 0x01:      /* DDRA */
			//osd_printf_debug("HC11: ddra = %02X\n", value);
			return;
		case 0x03:      /* PORTC */
			m_io->write_byte(MC68HC11_IO_PORTC, value);
			return;
		case 0x04:      /* PORTC */
			m_io->write_byte(MC68HC11_IO_PORTB, value);
			return;
		case 0x08:      /* PORTD */
			m_io->write_byte(MC68HC11_IO_PORTD, value); //mask & 0x3f?
			return;
		case 0x09:      /* DDRD */
			//osd_printf_debug("HC11: ddrd = %02X\n", value);
			return;
		case 0x0a:      /* PORTE */
			m_io->write_byte(MC68HC11_IO_PORTE, value);
			return;
		case 0x0e:      /* TCNT */
		case 0x0f:
			logerror("HC11: TCNT register write %02x %02x!\n",address,value);
			return;
		case 0x16:      /* TOC1 */
			/* TODO: inhibit for one bus cycle */
			m_toc1 = (value << 8) | (m_toc1 & 0xff);
			return;
		case 0x17:
			m_toc1 = (value & 0xff) | (m_toc1 & 0xff00);
			return;
		case 0x22:      /* TMSK1 */
			m_tmsk1 = value;
			return;
		case 0x23:
			m_tflg1 &= ~value;
			return;
		case 0x24:      /* TMSK2 */
			m_pr = value & 3;
			return;
		case 0x28:      /* SPCR1 */
			return;
		case 0x30:      /* ADCTL */
			m_adctl = value;
			return;
		case 0x38:      /* OPT2 */
			return;
		case 0x39:      /* OPTION */
			return;
		case 0x3a:      /* COPRST (watchdog) */
			return;

		case 0x3d:      /* INIT */
		{
			int reg_page = value & 0xf;
			int ram_page = (value >> 4) & 0xf;

			if (reg_page == ram_page) {
				m_reg_position = reg_page << 12;
				m_ram_position = (ram_page << 12) + ((m_has_extended_io) ? 0x100 : 0x80);
			} else {
				m_reg_position = reg_page << 12;
				m_ram_position = ram_page << 12;
			}
			return;
		}

		case 0x3f:      /* CONFIG */
			return;

		case 0x70:      /* SCBDH */
			return;
		case 0x71:      /* SCBDL */
			return;
		case 0x72:      /* SCCR1 */
			return;
		case 0x73:      /* SCCR2 */
			return;
		case 0x77:      /* SCDRL */
			return;
		case 0x7c:      /* PORTH */
			m_io->write_byte(MC68HC11_IO_PORTH, value);
			return;
		case 0x7d:      /* DDRH */
			//osd_printf_debug("HC11: ddrh = %02X at %04X\n", value, m_pc);
			return;
		case 0x7e:      /* PORTG */
			m_io->write_byte(MC68HC11_IO_PORTG, value);
			return;
		case 0x7f:      /* DDRG */
			//osd_printf_debug("HC11: ddrg = %02X at %04X\n", value, m_pc);
			return;

		case 0x88:      /* SPCR2 */
			return;
		case 0x89:      /* SPSR2 */
			return;
		case 0x8a:      /* SPDR2 */
			m_io->write_byte(MC68HC11_IO_SPI2_DATA, value);
			return;

		case 0x8b:      /* OPT4 */
			return;

	}

	logerror("HC11: regs_w %02X, %02X\n", reg, value);
}

/*****************************************************************************/

UINT8 mc68hc11_cpu_device::FETCH()
{
	return m_direct->read_byte(m_pc++);
}

UINT16 mc68hc11_cpu_device::FETCH16()
{
	UINT16 w;
	w = (m_direct->read_byte(m_pc) << 8) | (m_direct->read_byte(m_pc+1));
	m_pc += 2;
	return w;
}

UINT8 mc68hc11_cpu_device::READ8(UINT32 address)
{
	if(address >= m_reg_position && address < m_reg_position+(m_has_extended_io ? 0x100 : 0x40))
	{
		return hc11_regs_r(address);
	}
	else if(address >= m_ram_position && address < m_ram_position+m_internal_ram_size)
	{
		return m_internal_ram[address-m_ram_position];
	}
	return m_program->read_byte(address);
}

void mc68hc11_cpu_device::WRITE8(UINT32 address, UINT8 value)
{
	if(address >= m_reg_position && address < m_reg_position+(m_has_extended_io ? 0x100 : 0x40))
	{
		hc11_regs_w(address, value);
		return;
	}
	else if(address >= m_ram_position && address < m_ram_position+m_internal_ram_size)
	{
		m_internal_ram[address-m_ram_position] = value;
		return;
	}
	m_program->write_byte(address, value);
}

UINT16 mc68hc11_cpu_device::READ16(UINT32 address)
{
	return (READ8(address) << 8) | (READ8(address+1));
}

void mc68hc11_cpu_device::WRITE16(UINT32 address, UINT16 value)
{
	WRITE8(address+0, (value >> 8) & 0xff);
	WRITE8(address+1, (value >> 0) & 0xff);
}

/*****************************************************************************/


#include "hc11ops.inc"
#include "hc11ops.h"

void mc68hc11_cpu_device::device_start()
{
	int i;

	/* clear the opcode tables */
	for(i=0; i < 256; i++) {
		hc11_optable[i] = &HC11OP(invalid);
		hc11_optable_page2[i] = &HC11OP(invalid);
		hc11_optable_page3[i] = &HC11OP(invalid);
		hc11_optable_page4[i] = &HC11OP(invalid);
	}
	/* fill the opcode tables */
	for(i=0; i < sizeof(hc11_opcode_list)/sizeof(hc11_opcode_list_struct); i++)
	{
		switch(hc11_opcode_list[i].page)
		{
			case 0x00:
				hc11_optable[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0x18:
				hc11_optable_page2[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0x1A:
				hc11_optable_page3[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0xCD:
				hc11_optable_page4[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
		}
	}

	m_internal_ram.resize(m_internal_ram_size);

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	save_item(NAME(m_pc));
	save_item(NAME(m_ix));
	save_item(NAME(m_iy));
	save_item(NAME(m_sp));
	save_item(NAME(m_ppc));
	save_item(NAME(m_ccr));
	save_item(NAME(m_d.d8.a));
	save_item(NAME(m_d.d8.b));
	save_item(NAME(m_adctl));
	save_item(NAME(m_ad_channel));
	save_item(NAME(m_ram_position));
	save_item(NAME(m_reg_position));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_has_extended_io));
	save_item(NAME(m_internal_ram_size));
	save_item(NAME(m_init_value));
	save_item(NAME(m_internal_ram));
	save_item(NAME(m_wait_state));
	save_item(NAME(m_stop_state));
	save_item(NAME(m_tflg1));
	save_item(NAME(m_tmsk1));
	save_item(NAME(m_toc1));
	save_item(NAME(m_tcnt));
//  save_item(NAME(m_por));
	save_item(NAME(m_pr));
	save_item(NAME(m_frc_base));

	m_pc = 0;
	m_d.d16 = 0;
	m_ix = 0;
	m_iy = 0;
	m_sp = 0;
	m_ppc = 0;
	m_adctl = 0;
	m_ad_channel = 0;
	m_irq_state[0] = m_irq_state[1] = 0;
	m_ram_position = 0;
	m_reg_position = 0;
	m_tflg1 = 0;
	m_tmsk1 = 0;

	state_add( HC11_PC, "PC", m_pc).formatstr("%04X");
	state_add( HC11_SP, "SP", m_sp).formatstr("%04X");
	state_add( HC11_A,  "A", m_d.d8.a).formatstr("%02X");
	state_add( HC11_B,  "B", m_d.d8.b).formatstr("%02X");
	state_add( HC11_IX, "IX", m_ix).formatstr("%04X");
	state_add( HC11_IY, "IY", m_iy).formatstr("%04X");

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_ccr).formatstr("%8s").noshow();

	m_icountptr = &m_icount;
}


void mc68hc11_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c",
				(m_ccr & CC_S) ? 'S' : '.',
				(m_ccr & CC_X) ? 'X' : '.',
				(m_ccr & CC_H) ? 'H' : '.',
				(m_ccr & CC_I) ? 'I' : '.',
				(m_ccr & CC_N) ? 'N' : '.',
				(m_ccr & CC_Z) ? 'Z' : '.',
				(m_ccr & CC_V) ? 'V' : '.',
				(m_ccr & CC_C) ? 'C' : '.');
			break;
	}
}


void mc68hc11_cpu_device::device_reset()
{
	m_pc = READ16(0xfffe);
	m_wait_state = 0;
	m_stop_state = 0;
	m_ccr = CC_X | CC_I | CC_S;
	hc11_regs_w(0x3d,m_init_value);
	m_toc1 = 0xffff;
	m_tcnt = 0xffff;
//  m_por = 1; // for first timer overflow / compare stuff
	m_pr = 3; // timer prescale
	m_frc_base = 0;
}

/*
IRQ table vectors:
0xffd6: SCI
0xffd8: SPI
0xffda: Pulse Accumulator Input Edge
0xffdc: Pulse Accumulator Overflow
0xffde: Timer Overflow
0xffe0: Timer Output Capture 5
0xffe2: Timer Output Capture 4
0xffe4: Timer Output Capture 3
0xffe6: Timer Output Capture 2
0xffe8: Timer Output Capture 1
0xffea: Timer Input Capture 3
0xffec: Timer Input Capture 2
0xffee: Timer Input Capture 1
0xfff0: Real Time Int
0xfff2: IRQ
0xfff4: XIRQ
0xfff6: SWI (Trap IRQ)
0xfff8: Illegal Opcode (NMI)
0xfffa: CO-Processor Fail
0xfffc: Clock Monitor
0xfffe: RESET
*/

void mc68hc11_cpu_device::check_irq_lines()
{
	if( m_irq_state[MC68HC11_IRQ_LINE]!=CLEAR_LINE && (!(m_ccr & CC_I)) )
	{
		UINT16 pc_vector;

		if(m_wait_state == 0)
		{
			PUSH16(m_pc);
			PUSH16(m_iy);
			PUSH16(m_ix);
			PUSH8(REG_A);
			PUSH8(REG_B);
			PUSH8(m_ccr);
		}
		pc_vector = READ16(0xfff2);
		SET_PC(pc_vector);
		m_ccr |= CC_I; //irq taken, mask the flag
		if(m_wait_state == 1) { m_wait_state = 2; }
		if(m_stop_state == 1) { m_stop_state = 2; }
		standard_irq_callback(MC68HC11_IRQ_LINE);
	}

	/* check timers here */
	{
		int divider = div_tab[m_pr & 3];
		UINT64 cur_time = total_cycles();
		UINT32 add = (cur_time - m_frc_base) / divider;

		if (add > 0)
		{
			for(UINT32 i=0;i<add;i++)
			{
				m_tcnt++;
				if(m_tcnt == m_toc1)
				{
					m_tflg1 |= 0x80;
					m_irq_state[MC68HC11_TOC1_LINE] = ASSERT_LINE;
				}
			}

			m_frc_base = cur_time;
		}
	}

	if( m_irq_state[MC68HC11_TOC1_LINE]!=CLEAR_LINE && (!(m_ccr & CC_I)) && m_tmsk1 & 0x80)
	{
		UINT16 pc_vector;

		if(m_wait_state == 0)
		{
			PUSH16(m_pc);
			PUSH16(m_iy);
			PUSH16(m_ix);
			PUSH8(REG_A);
			PUSH8(REG_B);
			PUSH8(m_ccr);
		}
		pc_vector = READ16(0xffe8);
		SET_PC(pc_vector);
		m_ccr |= CC_I; //irq taken, mask the flag
		if(m_wait_state == 1) { m_wait_state = 2; }
		if(m_stop_state == 1) { m_stop_state = 2; }
		standard_irq_callback(MC68HC11_TOC1_LINE);
		m_irq_state[MC68HC11_TOC1_LINE] = CLEAR_LINE; // auto-ack irq
	}

}

void mc68hc11_cpu_device::execute_set_input(int inputnum, int state)
{
	m_irq_state[inputnum] = state;
	if (state == CLEAR_LINE) return;
	check_irq_lines();
}

void mc68hc11_cpu_device::execute_run()
{
	while(m_icount > 0)
	{
		UINT8 op;

		check_irq_lines();

		m_ppc = m_pc;
		debugger_instruction_hook(this, m_pc);

		op = FETCH();
		(this->*hc11_optable[op])();
	}
}
