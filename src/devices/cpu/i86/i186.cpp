// license:BSD-3-Clause
// copyright-holders:Carl
// Peripheral code from rmnimbus driver by Phill Harvey-Smith which is
// based on the Leland sound driver by Aaron Giles and Paul Leaman

// Note: the X1 input (typically an XTAL) is divided by 2 internally.
// The device clock should therefore be twice the desired operating
// frequency (and twice the speed rating suffixed to the part number).
// Some high-performance AMD versions, however, generate the system
// clock with a PLL and only support XTAL dividers other than 1 for
// compatibility and power-saving modes.

#include "emu.h"
#include "i186.h"
#include "i86inline.h"

#define LOG_PORTS           (1U << 1)
#define LOG_INTERRUPTS      (1U << 2)
#define LOG_INTERRUPTS_EXT  (1U << 3)
#define LOG_TIMER           (1U << 4)
#define LOG_DMA             (1U << 5)
#define LOG_DMA_HIFREQ      (1U << 6)

#define VERBOSE (0)
#include "logmacro.h"

/* external int priority masks */

#define EXTINT_CTRL_PRI_MASK    0x07
#define EXTINT_CTRL_MSK         0x08
#define EXTINT_CTRL_LTM         0x10
#define EXTINT_CTRL_CASCADE     0x20
#define EXTINT_CTRL_SFNM        0x40

/* DMA control register */

#define DEST_MIO                0x8000
#define DEST_DECREMENT          0x4000
#define DEST_INCREMENT          0x2000
#define DEST_NO_CHANGE          (DEST_DECREMENT | DEST_INCREMENT)
#define DEST_INCDEC_MASK        (DEST_DECREMENT | DEST_INCREMENT)
#define SRC_MIO                 0x1000
#define SRC_DECREMENT           0x0800
#define SRC_INCREMENT           0x0400
#define SRC_NO_CHANGE           (SRC_DECREMENT | SRC_INCREMENT)
#define SRC_INCDEC_MASK         (SRC_DECREMENT | SRC_INCREMENT)
#define TERMINATE_ON_ZERO       0x0200
#define INTERRUPT_ON_ZERO       0x0100
#define SYNC_MASK               0x00C0
#define SYNC_SOURCE             0x0040
#define SYNC_DEST               0x0080
#define CHANNEL_PRIORITY        0x0020
#define TIMER_DRQ               0x0010
#define CHG_NOCHG               0x0004
#define ST_STOP                 0x0002
#define BYTE_WORD               0x0001

/* these come from the Intel 80186 datasheet */
const uint8_t i80186_cpu_device::m_i80186_timing[] =
{
	45,28,          /* exception, IRET */
		0, 2, 4, 3, /* INTs */
		2,              /* segment overrides */
		2, 2, 3,        /* flag operations */
		8, 7,19,15, /* arithmetic adjusts */
		4, 4,           /* decimal adjusts */
		2, 4,           /* sign extension */
		2,18, 6, 2, 6,11,   /* misc */

	14,14,14,       /* direct JMPs */
	11,17,26,       /* indirect JMPs */
	15,23,          /* direct CALLs */
	13,19,38,       /* indirect CALLs */
	16,22,18,25,    /* returns */
		4,13, 5,15, /* conditional JMPs */
		6,16, 6,16, /* loops */

	10,10, 8, 8,    /* port reads */
		9, 9, 7, 7, /* port writes */

		2, 9,12,        /* move, 8-bit */
		3,12,           /* move, 8-bit immediate */
		2, 9,12,        /* move, 16-bit */
		4,13,           /* move, 16-bit immediate */
		8, 8, 9, 9, /* move, AL/AX memory */
		2,11, 2,11, /* move, segment registers */
		4,17,           /* exchange, 8-bit */
		4,17, 3,        /* exchange, 16-bit */

	10,16, 9, 9,    /* pushes */
	10,20, 8, 8,    /* pops */

		3,10,10,        /* ALU ops, 8-bit */
		4,16,10,        /* ALU ops, 8-bit immediate */
		3,10,10,        /* ALU ops, 16-bit */
		4,16,10,        /* ALU ops, 16-bit immediate */
		4,16,10,        /* ALU ops, 16-bit w/8-bit immediate */
	26,35,32,41,    /* MUL */
	25,34,31,40,    /* IMUL */
	29,38,35,44,    /* DIV */
	44,53,50,59,    /* IDIV */
		3, 3,15,15, /* INC/DEC */
		3, 3,10,10, /* NEG/NOT */

		2, 5, 1,        /* reg shift/rotate */
	15,17, 1,       /* m8 shift/rotate */
	15,17, 1,       /* m16 shift/rotate */

	22, 5,22,       /* CMPS 8-bit */
	22, 5,22,       /* CMPS 16-bit */
	15, 5,15,       /* SCAS 8-bit */
	15, 5,15,       /* SCAS 16-bit */
	12, 6,11,       /* LODS 8-bit */
	12, 6,11,       /* LODS 16-bit */
	10, 6, 9,       /* STOS 8-bit */
	10, 6, 9,       /* STOS 16-bit */
	14, 8, 8,       /* MOVS 8-bit */
	14, 8, 8,       /* MOVS 16-bit */

	14, 8, 8,       /* (80186) INS 8-bit */
	14, 8, 8,       /* (80186) INS 16-bit */
	14, 8, 8,       /* (80186) OUTS 8-bit */
	14, 8, 8,       /* (80186) OUTS 16-bit */
	14,68,83,       /* (80186) PUSH immediate, PUSHA/POPA */
	22,29,          /* (80186) IMUL immediate 8-bit */
	25,32,          /* (80186) IMUL immediate 16-bit */
	15,25,4,16, 8,  /* (80186) ENTER/LEAVE */
	33,             /* (80186) BOUND */
};

DEFINE_DEVICE_TYPE(I80186, i80186_cpu_device, "i80186", "Intel 80186")
DEFINE_DEVICE_TYPE(I80188, i80188_cpu_device, "i80188", "Intel 80188")
DEFINE_DEVICE_TYPE(AM186EM, am186em_device, "am186em", "AMD Am186EM")
DEFINE_DEVICE_TYPE(AM188EM, am188em_device, "am188em", "AMD Am188EM")

i80186_cpu_device::i80186_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i80186_cpu_device(mconfig, I80186, tag, owner, clock, 16)
{
}

i80188_cpu_device::i80188_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i80186_cpu_device(mconfig, I80188, tag, owner, clock, 8)
{
}

am186em_device::am186em_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i80186_cpu_device(mconfig, AM186EM, tag, owner, clock, 16)
{
}

am188em_device::am188em_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i80186_cpu_device(mconfig, AM188EM, tag, owner, clock, 8)
{
}

i80186_cpu_device::i80186_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int data_bus_size)
	: i8086_common_cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_opcodes_config("opcodes", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, data_bus_size, 16, 0)
	, m_read_slave_ack_func(*this, 0)
	, m_out_chip_select_func(*this)
	, m_out_tmrout0_func(*this)
	, m_out_tmrout1_func(*this)
	, m_irmx_irq_cb(*this)
	, m_irqa_cb(*this)
	, m_irmx_irq_ack(*this)
{
	memcpy(m_timing, m_i80186_timing, sizeof(m_i80186_timing));
	set_irq_acknowledge_callback(*this, FUNC(i80186_cpu_device::inta_callback));
}

device_memory_interface::space_config_vector i80186_cpu_device::memory_space_config() const
{
	if (has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_opcodes_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
}

uint8_t i80186_cpu_device::fetch()
{
	uint8_t data = m_or8(update_pc());
	m_ip++;
	return data;
}

void i80186_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		if ((m_dma[0].drq_state && (m_dma[0].control & ST_STOP)) || (m_dma[1].drq_state && (m_dma[1].control & ST_STOP)))
		{
			int channel = m_last_dma ? 0 : 1;
			m_last_dma = !m_last_dma;
			if (!(m_dma[1].drq_state && (m_dma[1].control & ST_STOP)))
				channel = 0;
			else if (!(m_dma[0].drq_state && (m_dma[0].control & ST_STOP)))
				channel = 1;
			else if ((m_dma[0].control & CHANNEL_PRIORITY) && !(m_dma[1].control & CHANNEL_PRIORITY))
				channel = 0;
			else if ((m_dma[1].control & CHANNEL_PRIORITY) && !(m_dma[0].control & CHANNEL_PRIORITY))
				channel = 1;
			m_icount--;
			drq_callback(channel);
			continue;
		}
		if (m_seg_prefix_next)
		{
			m_seg_prefix = true;
			m_seg_prefix_next = false;
		}
		else
		{
			m_prev_ip = m_ip;
			m_seg_prefix = false;

			/* Dispatch IRQ */
			if (m_pending_irq && m_no_interrupt == 0)
			{
				if (m_pending_irq & NMI_IRQ)
				{
					interrupt(2);
					m_pending_irq &= ~NMI_IRQ;
					m_halt = false;
				}
				else if (m_IF)
				{
					/* the actual vector is retrieved after pushing flags */
					/* and clearing the IF */
					interrupt(-1);
					m_halt = false;
				}
			}

			if (m_halt)
			{
				m_icount = 0;
				return;
			}

			/* No interrupt allowed between last instruction and this one */
			if (m_no_interrupt)
			{
				m_no_interrupt--;
			}

			/* trap should allow one instruction to be executed */
			if (m_fire_trap)
			{
				if (m_fire_trap >= 2)
				{
					interrupt(1);
					m_fire_trap = 0;
				}
				else
				{
					m_fire_trap++;
				}
			}
		}

		debugger_instruction_hook( update_pc() );

		uint8_t op = fetch_op();

		switch(op)
		{
			case 0x60: // i_pusha
				{
					uint32_t tmp = m_regs.w[SP];

					PUSH(m_regs.w[AX]);
					PUSH(m_regs.w[CX]);
					PUSH(m_regs.w[DX]);
					PUSH(m_regs.w[BX]);
					PUSH(tmp);
					PUSH(m_regs.w[BP]);
					PUSH(m_regs.w[SI]);
					PUSH(m_regs.w[DI]);
					CLK(PUSHA);
				}
				break;

			case 0x61: // i_popa
				m_regs.w[DI] = POP();
				m_regs.w[SI] = POP();
				m_regs.w[BP] = POP();
				POP();
				m_regs.w[BX] = POP();
				m_regs.w[DX] = POP();
				m_regs.w[CX] = POP();
				m_regs.w[AX] = POP();
				CLK(POPA);
				break;

			case 0x62: // i_bound
				{
					m_modrm = fetch();
					uint32_t low = GetRMWord();
					uint32_t high = GetnextRMWord();
					uint32_t tmp = RegWord();
					if (tmp < low || tmp > high)
						interrupt(5);
					CLK(BOUND);
					logerror("%06x: bound %04x high %04x low %04x tmp\n", m_pc, high, low, tmp);
				}
				break;

			case 0x68: // i_push_d16
				PUSH(fetch_word());
				CLK(PUSH_IMM);
				break;

			case 0x69: // i_imul_d16
				{
					DEF_r16w();
					uint32_t tmp = fetch_word();
					m_dst = (int32_t)((int16_t)m_src) * (int32_t)((int16_t)tmp);
					m_CarryVal = m_OverVal = (((int32_t)m_dst) >> 15 != 0) && (((int32_t)m_dst) >> 15 != -1);
					RegWord(m_dst);
					CLKM(IMUL_RRI16, IMUL_RMI16);
				}
				break;

			case 0x6a: // i_push_d8
				PUSH((uint16_t)((int16_t)((int8_t)fetch())));
				CLK(PUSH_IMM);
				break;

			case 0x6b: // i_imul_d8
				{
					DEF_r16w();
					uint32_t src2 = (uint16_t)(int8_t)fetch();
					m_dst = (int32_t)((int16_t)m_src) * (int32_t)((int16_t)src2);
					m_CarryVal = m_OverVal = (((int32_t)m_dst) >> 15 != 0) && (((int32_t)m_dst) >> 15 != -1);
					RegWord(m_dst);
					CLKM(IMUL_RRI8, IMUL_RMI8);
				}
				break;

			case 0x6c: // i_insb
				i_insb();
				break;

			case 0x6d: // i_insw
				i_insw();
				break;

			case 0x6e: // i_outsb
				i_outsb();
				break;

			case 0x6f: // i_outsw
				i_outsw();
				break;

			case 0x8e: // i_mov_sregw
				m_modrm = fetch();
				m_src = GetRMWord();
				CLKM(MOV_SR, MOV_SM);
				switch (m_modrm & 0x38)
				{
				case 0x00:  /* mov es,ew */
					m_sregs[ES] = m_src;
					break;
				case 0x10:  /* mov ss,ew */
					m_sregs[SS] = m_src;
					m_no_interrupt = 1;
					break;
				case 0x18:  /* mov ds,ew */
					m_sregs[DS] = m_src;
					break;
				default:
					logerror("%06x: Mov Sreg - Invalid register\n", m_pc);
					m_ip = m_prev_ip;
					interrupt(6);
					break;
				}
				break;

			case 0xc0: // i_rotshft_bd8
				{
					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					uint8_t c = fetch() & 0x1f;
					CLKM(ROT_REG_BASE, ROT_M8_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch (m_modrm & 0x38)
						{
						case 0x00: do { ROL_BYTE();  c--; } while (c > 0); PutbackRMByte(m_dst); break;
						case 0x08: do { ROR_BYTE();  c--; } while (c > 0); PutbackRMByte(m_dst); break;
						case 0x10: do { ROLC_BYTE(); c--; } while (c > 0); PutbackRMByte(m_dst); break;
						case 0x18: do { RORC_BYTE(); c--; } while (c > 0); PutbackRMByte(m_dst); break;
						case 0x30:
						case 0x20: SHL_BYTE(c); break;
						case 0x28: SHR_BYTE(c); break;
						case 0x38: SHRA_BYTE(c); break;
						}
					}
				}
				break;

			case 0xc1: // i_rotshft_wd8
				{
					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					uint8_t c = fetch() & 0x1f;
					CLKM(ROT_REG_BASE, ROT_M16_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch (m_modrm & 0x38)
						{
						case 0x00: do { ROL_WORD();  c--; } while (c > 0); PutbackRMWord(m_dst); break;
						case 0x08: do { ROR_WORD();  c--; } while (c > 0); PutbackRMWord(m_dst); break;
						case 0x10: do { ROLC_WORD(); c--; } while (c > 0); PutbackRMWord(m_dst); break;
						case 0x18: do { RORC_WORD(); c--; } while (c > 0); PutbackRMWord(m_dst); break;
						case 0x30:
						case 0x20: SHL_WORD(c); break;
						case 0x28: SHR_WORD(c); break;
						case 0x38: SHRA_WORD(c); break;
						}
					}
				}
				break;

			case 0xc8: // i_enter
				{
					uint16_t nb = fetch();
					nb |= fetch() << 8;

					uint32_t level = fetch();
					CLK(!level ? ENTER0 : (level == 1) ? ENTER1 : ENTER_BASE);
					if (level > 1)
						m_icount -= level * m_timing[ENTER_COUNT];
					PUSH(m_regs.w[BP]);
					m_regs.w[BP] = m_regs.w[SP];
					m_regs.w[SP] -= nb;
					for (int i = 1; i < level; i++)
					{
						PUSH(GetMemW(SS, m_regs.w[BP] - i * 2));
					}
					if (level)
					{
						PUSH(m_regs.w[BP]);
					}
				}
				break;

			case 0xc9: // i_leave
				m_regs.w[SP] = m_regs.w[BP];
				m_regs.w[BP] = POP();
				CLK(LEAVE);
				break;

			case 0xd2: // i_rotshft_bcl
				{
					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					uint8_t c = m_regs.b[CL] & 0x1f;
					CLKM(ROT_REG_BASE, ROT_M16_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch (m_modrm & 0x38)
						{
						case 0x00: do { ROL_BYTE();  c--; } while (c > 0); PutbackRMByte(m_dst); break;
						case 0x08: do { ROR_BYTE();  c--; } while (c > 0); PutbackRMByte(m_dst); break;
						case 0x10: do { ROLC_BYTE(); c--; } while (c > 0); PutbackRMByte(m_dst); break;
						case 0x18: do { RORC_BYTE(); c--; } while (c > 0); PutbackRMByte(m_dst); break;
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
					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					uint8_t c = m_regs.b[CL] & 0x1f;
					CLKM(ROT_REG_BASE, ROT_M16_BASE);
					m_icount -= m_timing[ROT_REG_BIT] * c;
					if (c)
					{
						switch (m_modrm & 0x38)
						{
							case 0x00: do { ROL_WORD();  c--; } while (c > 0); PutbackRMWord(m_dst); break;
							case 0x08: do { ROR_WORD();  c--; } while (c > 0); PutbackRMWord(m_dst); break;
							case 0x10: do { ROLC_WORD(); c--; } while (c > 0); PutbackRMWord(m_dst); break;
							case 0x18: do { RORC_WORD(); c--; } while (c > 0); PutbackRMWord(m_dst); break;
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
				if (m_reloc & 0x8000)
				{
					m_ip = m_prev_ip;
					interrupt(7);
					break;
				}
				m_modrm = fetch();
				GetRMByte();
				CLK(NOP);
				// The 80187 has the FSTSW AX instruction
				if ((m_modrm == 0xe0) && (op == 0xdf))
					m_regs.w[AX] = 0xffff;  // FPU not present
				break;

			case 0xe6: // i_outal
				write_port_byte_al(fetch());
				CLK(OUT_IMM8);
				break;

			case 0xee: // i_outdxal
				write_port_byte_al(m_regs.w[DX]);
				CLK(OUT_DX8);
				break;

			case 0xf2: // i_repne
			case 0xf3:
				{
					bool pass = false;
					uint8_t next = repx_op();
					uint16_t c = m_regs.w[CX];

					switch (next)
					{
					case 0x6c:
						CLK(OVERRIDE);
						if (c)
						{
							do
							{
								i_insb();
								c--;
							} while (c > 0 && m_icount > 0);
						}
						m_regs.w[CX] = c;
						m_seg_prefix = false;
						m_seg_prefix_next = false;
						break;
					case 0x6d:
						CLK(OVERRIDE);
						if (c)
						{
							do
							{
								i_insw();
								c--;
							} while (c > 0 && m_icount > 0);
						}
						m_regs.w[CX] = c;
						m_seg_prefix = false;
						m_seg_prefix_next = false;
						break;
					case 0x6e:
						CLK(OVERRIDE);
						if (c)
						{
							do
							{
								i_outsb();
								c--;
							} while (c > 0 && m_icount > 0);
						}
						m_regs.w[CX] = c;
						m_seg_prefix = false;
						m_seg_prefix_next = false;
						break;
					case 0x6f:
						CLK(OVERRIDE);
						if (c)
						{
							do
							{
								i_outsw();
								c--;
							} while (c > 0 && m_icount > 0);
						}
						m_regs.w[CX] = c;
						m_seg_prefix = false;
						m_seg_prefix_next = false;
						break;
					default:
						// Decrement IP and pass on
						m_ip -= 1 + (m_seg_prefix_next ? 1 : 0);
						pass = true;
						break;
					}
					if (!pass)
					{
						if (c)
							m_ip = m_prev_ip;
						break;
					}
				}
				[[fallthrough]];
			default:
				if (!common_op(op))
				{
					m_icount -= 10; // UD fault timing?
					logerror("%06x: Invalid Opcode %02x\n", m_pc, op);
					m_ip = m_prev_ip;
					interrupt(6); // 80186 has #UD
					break;
				}
		}
	}
}

void i80186_cpu_device::device_start()
{
	i8086_common_cpu_device::device_start();
	state_add( I8086_ES, "ES", m_sregs[ES] ).formatstr("%04X");
	state_add( I8086_CS, "CS", m_sregs[CS] ).callimport().formatstr("%04X");
	state_add( I8086_SS, "SS", m_sregs[SS] ).formatstr("%04X");
	state_add( I8086_DS, "DS", m_sregs[DS] ).formatstr("%04X");
	state_add( I8086_VECTOR, "V", m_int_vector).formatstr("%02X");

	state_add( I8086_PC, "PC", m_pc ).callimport().formatstr("%05X");
	state_add<uint32_t>( STATE_GENPCBASE, "CURPC", [this] { return (m_sregs[CS] << 4) + m_prev_ip; }).mask(0xfffff).noshow();
	state_add( I8086_HALT, "HALT", m_halt ).mask(1);

	// Most of these mnemonics are borrowed from the Intel 80C186EA/80C188EA User's Manual.
	// (The 80C186EA/80C188EA's peripheral block is mapped incompatibly but mostly functionally analogous.)
	state_add( I80186_RELREG, "RELREG", m_reloc ).formatstr("%04X");
	state_add( I80186_UMCS, "UMCS", m_mem.upper ).formatstr("%04X");
	state_add( I80186_LMCS, "LMCS", m_mem.lower ).formatstr("%04X");
	state_add( I80186_PACS, "PACS", m_mem.peripheral ).formatstr("%04X");
	state_add( I80186_MMCS, "MMCS", m_mem.middle ).formatstr("%04X");
	state_add( I80186_MPCS, "MPCS", m_mem.middle_size ).formatstr("%04X");
	state_add( I80186_DxSRC + 0, "D0SRC", m_dma[0].source ).formatstr("%05X").mask(0xfffff);
	state_add( I80186_DxDST + 0, "D0DST", m_dma[0].dest ).formatstr("%05X").mask(0xfffff);
	state_add( I80186_DxTC + 0, "D0TC", m_dma[0].count ).formatstr("%04X");
	state_add( I80186_DxCON + 0, "D0CON", m_dma[0].control ).formatstr("%04X");
	state_add( I80186_DxSRC + 1, "D1SRC", m_dma[1].source ).formatstr("%05X").mask(0xfffff);
	state_add( I80186_DxDST + 1, "D1DST", m_dma[1].dest ).formatstr("%05X").mask(0xfffff);
	state_add( I80186_DxTC + 1, "D1TC", m_dma[1].count ).formatstr("%04X");
	state_add( I80186_DxCON + 1, "D1CON", m_dma[1].control ).formatstr("%04X");
	state_add( I80186_TxCNT + 0, "T0CNT", m_timer[0].count ).formatstr("%04X");
	state_add( I80186_TxCMPA + 0, "T0CMPA", m_timer[0].maxA ).formatstr("%04X");
	state_add( I80186_TxCMPB + 0, "T0CMPB", m_timer[0].maxB ).formatstr("%04X");
	state_add( I80186_TxCON + 0, "T0CON", m_timer[0].control ).formatstr("%04X");
	state_add( I80186_TxCNT + 1, "T1CNT", m_timer[1].count ).formatstr("%04X");
	state_add( I80186_TxCMPA + 1, "T1CMPA", m_timer[1].maxA ).formatstr("%04X");
	state_add( I80186_TxCMPB + 1, "T1CMPB", m_timer[1].maxB ).formatstr("%04X");
	state_add( I80186_TxCON + 1, "T1CON", m_timer[1].control ).formatstr("%04X");
	state_add( I80186_TxCNT + 2, "T2CNT", m_timer[2].count ).formatstr("%04X");
	state_add( I80186_TxCMPA + 2, "T2CMPA", m_timer[2].maxA ).formatstr("%04X");
	state_add( I80186_TxCON + 2, "T2CON", m_timer[2].control ).formatstr("%04X");
	state_add( I80186_INSERV, "INSERV", m_intr.in_service ).formatstr("%04X");
	state_add( I80186_REQST, "REQST", m_intr.request ).formatstr("%04X");
	state_add( I80186_PRIMSK, "PRIMSK", m_intr.priority_mask ).formatstr("%04X");
	state_add( I80186_INTSTS, "INTSTS", m_intr.status ).formatstr("%04X");
	state_add( I80186_TCUCON, "TCUCON", m_intr.timer[0] ).formatstr("%04X");
	state_add( I80186_DMA0CON, "DMA0CON", m_intr.dma[0] ).formatstr("%04X");
	state_add( I80186_DMA1CON, "DMA1CON", m_intr.dma[1] ).formatstr("%04X");
	state_add( I80186_I0CON, "I0CON", m_intr.ext[0] ).formatstr("%04X");
	state_add( I80186_I1CON, "I1CON", m_intr.ext[1] ).formatstr("%04X");
	state_add( I80186_I2CON, "I2CON", m_intr.ext[2] ).formatstr("%04X");
	state_add( I80186_I3CON, "I3CON", m_intr.ext[3] ).formatstr("%04X");
	state_add( I80186_POLLSTS, "POLLSTS", m_intr.poll_status ).formatstr("%04X");

	// register for savestates
	save_item(STRUCT_MEMBER(m_timer, control));
	save_item(STRUCT_MEMBER(m_timer, maxA));
	save_item(STRUCT_MEMBER(m_timer, maxB));
	save_item(STRUCT_MEMBER(m_timer, count));
	save_item(STRUCT_MEMBER(m_dma, source));
	save_item(STRUCT_MEMBER(m_dma, dest));
	save_item(STRUCT_MEMBER(m_dma, count));
	save_item(STRUCT_MEMBER(m_dma, control));
	save_item(NAME(m_intr.vector));
	save_item(NAME(m_intr.pending));
	save_item(NAME(m_intr.ack_mask));
	save_item(NAME(m_intr.priority_mask));
	save_item(NAME(m_intr.in_service));
	save_item(NAME(m_intr.request));
	save_item(NAME(m_intr.status));
	save_item(NAME(m_intr.poll_status));
	save_item(NAME(m_intr.timer));
	save_item(NAME(m_intr.dma));
	save_item(NAME(m_intr.ext));
	save_item(NAME(m_intr.ext_state));
	save_item(NAME(m_mem.lower));
	save_item(NAME(m_mem.upper));
	save_item(NAME(m_mem.middle));
	save_item(NAME(m_mem.middle_size));
	save_item(NAME(m_mem.peripheral));
	save_item(NAME(m_reloc));
	save_item(NAME(m_last_dma));

	// zerofill
	memset(m_timer, 0, sizeof(m_timer));
	memset(m_dma, 0, sizeof(m_dma));
	memset(&m_intr, 0, sizeof(intr_state));
	memset(&m_mem, 0, sizeof(mem_state));
	m_reloc = 0;
	m_last_dma = 0;

	m_timer[0].int_timer = timer_alloc(FUNC(i80186_cpu_device::timer_elapsed), this);
	m_timer[1].int_timer = timer_alloc(FUNC(i80186_cpu_device::timer_elapsed), this);
	m_timer[2].int_timer = timer_alloc(FUNC(i80186_cpu_device::timer_elapsed), this);

	m_irmx_irq_ack.resolve_safe(0);
}

void i80186_cpu_device::device_reset()
{
	i8086_common_cpu_device::device_reset();
	/* reset the interrupt state */
	m_intr.priority_mask    = 0x0007;
	m_intr.timer[0]         = 0x000f;
	m_intr.timer[1]         = 0x000f;
	m_intr.timer[2]         = 0x000f;
	m_intr.dma[0]           = 0x000f;
	m_intr.dma[1]           = 0x000f;
	m_intr.ext[0]           = 0x000f;
	m_intr.ext[1]           = 0x000f;
	m_intr.ext[2]           = 0x000f;
	m_intr.ext[3]           = 0x000f;
	m_intr.in_service       = 0x0000;

	m_intr.pending          = 0x0000;
	m_intr.ack_mask         = 0x0000;
	m_intr.request          = 0x0000;
	m_intr.status           = 0x0000;
	m_intr.poll_status      = 0x0000;
	m_intr.ext_state        = 0x00;

	m_reloc = 0x20ff;
	m_mem.upper = 0xfffb;

	for (dma_state &elem : m_dma)
	{
		elem.drq_state = false;
		elem.control = 0;
	}

	for (timer_state &elem : m_timer)
	{
		elem.control = 0;
		elem.maxA = 0;
		elem.maxB = 0;
		elem.count = 0;
	}
}

uint8_t i80186_cpu_device::read_port_byte(uint16_t port)
{
	if (!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		if (port & 1)
			return internal_port_r((port >> 1) & 0x7f, 0xff00) >> 8;
		else
			return internal_port_r((port >> 1) & 0x7f, 0x00ff) & 0xff;
	}
	return m_io->read_byte(port);
}

uint16_t i80186_cpu_device::read_port_word(uint16_t port)
{
	if (!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		// Unaligned reads from the internal bus are swapped rather than split
		if (port & 1)
			return swapendian_int16(internal_port_r((port >> 1) & 0x7f));
		else
			return internal_port_r((port >> 1) & 0x7f);
	}
	return m_io->read_word_unaligned(port);
}

void i80186_cpu_device::write_port_byte(uint16_t port, uint8_t data)
{
	if (!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		if (port & 1)
			internal_port_w((port >> 1) & 0x7f, data << 8);
		else
			internal_port_w((port >> 1) & 0x7f, data);
	}
	else
		m_io->write_byte(port, data);
}

void i80186_cpu_device::write_port_byte_al(uint16_t port)
{
	if (!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		// Both AH and AL are written onto the internal bus
		if (port & 1)
			internal_port_w((port >> 1) & 0x7f, swapendian_int16(m_regs.w[AX]));
		else
			internal_port_w((port >> 1) & 0x7f, m_regs.w[AX]);
	}
	else
		m_io->write_byte(port, m_regs.w[AL]);
}

void i80186_cpu_device::write_port_word(uint16_t port, uint16_t data)
{
	if (!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		// Unaligned writes to the internal bus are swapped rather than split
		if (port & 1)
			internal_port_w((port >> 1) & 0x7f, swapendian_int16(data));
		else
			internal_port_w((port >> 1) & 0x7f, data);
	}
	else
		m_io->write_word_unaligned(port, data);
}

uint8_t i80186_cpu_device::read_byte(uint32_t addr)
{
	if ((m_reloc & 0x1000) && (addr >> 8) == (m_reloc & 0xfff))
	{
		uint16_t ret = internal_port_r((addr >> 1) & 0x7f, (addr & 1) ? 0xff00 : 0x00ff);
		return (addr & 1) ? (ret >> 8) : (ret & 0xff);
	}
	return m_program->read_byte(addr);
}

uint16_t i80186_cpu_device::read_word(uint32_t addr)
{
	if ((m_reloc & 0x1000) && (addr >> 8) == (m_reloc & 0xfff))
	{
		// Unaligned reads from the internal bus are swapped rather than split
		if (addr & 1)
			return swapendian_int16(internal_port_r((addr >> 1) & 0x7f));
		else
			return internal_port_r((addr >> 1) & 0x7f);
	}
	return m_program->read_word_unaligned(addr);
}

void i80186_cpu_device::write_byte(uint32_t addr, uint8_t data)
{
	if ((m_reloc & 0x1000) && (addr >> 8) == (m_reloc & 0xfff))
		internal_port_w((addr >> 1) & 0x7f, (addr & 1) ? (data << 8) : data);
	else
		m_program->write_byte(addr, data);
}

void i80186_cpu_device::write_word(uint32_t addr, uint16_t data)
{
	if ((m_reloc & 0x1000) && (addr >> 8) == (m_reloc & 0xfff))
	{
		// Unaligned writes from the internal bus are swapped rather than split
		if (addr & 1)
			internal_port_w((addr >> 1) & 0x7f, swapendian_int16(data));
		else
			internal_port_w((addr >> 1) & 0x7f, data);
	}
	else
		m_program->write_word_unaligned(addr, data);
}

/*************************************
 *
 *  80186 interrupt controller
 *
 *************************************/
IRQ_CALLBACK_MEMBER(i80186_cpu_device::inta_callback)
{
	// s-state 0 is irqack
	m_irqa_cb(ASSERT_LINE);
	m_irqa_cb(CLEAR_LINE);
	if (BIT(m_reloc, 14))
		return m_irmx_irq_ack(device, irqline);
	return int_callback(device, irqline);
}

IRQ_CALLBACK_MEMBER(i80186_cpu_device::int_callback)
{
	LOGMASKED(LOG_INTERRUPTS, "(%f) **** Acknowledged interrupt vector %02X\n", machine().time().as_double(), m_intr.poll_status & 0x1f);

	/* clear the interrupt */
	set_input_line(0, CLEAR_LINE);
	m_intr.pending = 0;

	uint16_t oldreq = m_intr.request;

	/* clear the request and set the in-service bit */
	if (m_intr.ack_mask & 0xf0)
	{
		int i;
		for (i = 0; i < 4; i++)
			if (BIT(m_intr.ack_mask, i + 4))
				break;
		if (!(m_intr.ext[i] & EXTINT_CTRL_LTM))
			m_intr.request &= ~m_intr.ack_mask;
	}
	else
		m_intr.request &= ~m_intr.ack_mask;

	if (m_intr.request != oldreq)
		LOGMASKED(LOG_INTERRUPTS, "intr.request changed from %02X to %02X\n", oldreq, m_intr.request);

	uint16_t old = m_intr.in_service;

	m_intr.in_service |= m_intr.ack_mask;

	if (m_intr.in_service != old)
		LOGMASKED(LOG_INTERRUPTS, "intr.in_service changed from %02X to %02X\n",old,m_intr.in_service);

	uint8_t vector;
	if (!BIT(m_reloc, 14))
	{
		if (m_intr.ack_mask == 0x0001)
		{
			switch (m_intr.poll_status & 0x1f)
			{
				case 0x08:  m_intr.status &= ~0x01; break;
				case 0x12:  m_intr.status &= ~0x02; break;
				case 0x13:  m_intr.status &= ~0x04; break;
			}
		}

		/* return the vector */
		switch(m_intr.poll_status & 0x1F)
		{
			case 0x0C: vector = (m_intr.ext[0] & EXTINT_CTRL_CASCADE) ? m_read_slave_ack_func(0) : (m_intr.poll_status & 0x1f); break;
			case 0x0D: vector = (m_intr.ext[1] & EXTINT_CTRL_CASCADE) ? m_read_slave_ack_func(1) : (m_intr.poll_status & 0x1f); break;
			default:
				vector = m_intr.poll_status & 0x1f; break;
		}
	}
	else
	{
		if (m_intr.ack_mask & 0x31)
		{
			if (m_intr.ack_mask & 1)
				m_intr.status &= ~0x01;
			else if (m_intr.ack_mask & 0x10)
				m_intr.status &= ~0x02;
			else if (m_intr.ack_mask & 0x20)
				m_intr.status &= ~0x04;
		}
		vector = m_intr.poll_status & 0xff;
	}
	m_intr.ack_mask = 0;

	/* a request no longer pending */
	m_intr.poll_status &= ~0x8000;

	LOGMASKED(LOG_INTERRUPTS, "intr.ext[0]=%04X intr.ext[1]=%04X\n", m_intr.ext[0], m_intr.ext[1]);
	LOGMASKED(LOG_INTERRUPTS, "Int %02X Calling vector %02X\n", m_intr.poll_status, vector);

	return vector;
}


void i80186_cpu_device::update_interrupt_state()
{
	int new_vector = 0;

	LOGMASKED(LOG_INTERRUPTS, "update_interrupt_status: req=%04X stat=%04X serv=%04X priority_mask=%4X\n", m_intr.request, m_intr.status, m_intr.in_service, m_intr.priority_mask);

	/* loop over priorities */
	for (int priority = 0; priority <= m_intr.priority_mask; priority++)
	{
		/* note: by checking 4 bits, we also verify that the mask is off */
		if (BIT(m_reloc, 14))
		{
			for (int int_num = 0; int_num < 3; int_num++)
			{
				if ((m_intr.timer[int_num] & 0x0f) == priority)
				{
					int irq = (1 << int_num);
					/* if we're already servicing something at this level, don't generate anything new */
					if (m_intr.in_service & 0x01)
						return;

					/* if there's something pending, generate an interrupt */
					if (m_intr.status & irq)
					{
						new_vector = m_intr.vector | priority;
						/* set the clear mask and generate the int */
						m_intr.ack_mask = int_num ? (8 << int_num) : 1;
						goto generate_int;
					}
				}
			}
		}
		else
		{
			if ((m_intr.timer[0] & 0x0f) == priority)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (m_intr.in_service & 0x01)
					return;

				/* if there's something pending, generate an interrupt */
				if (m_intr.status & 0x07)
				{
					if (m_intr.status & 1)
						new_vector = 0x08;
					else if (m_intr.status & 2)
						new_vector = 0x12;
					else if (m_intr.status & 4)
						new_vector = 0x13;
					else
						logerror("Invalid timer interrupt!\n");

					/* set the clear mask and generate the int */
					m_intr.ack_mask = 0x0001;
					goto generate_int;
				}
			}
		}

		/* check DMA interrupts */
		for (int int_num = 0; int_num < 2; int_num++)
			if ((m_intr.dma[int_num] & 0x0F) == priority)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (m_intr.in_service & (0x04 << int_num))
					return;

				/* if there's something pending, generate an interrupt */
				if (m_intr.request & (0x04 << int_num))
				{
					if (BIT(m_reloc, 14))
						new_vector = m_intr.vector | priority;
					else
						new_vector = 0x0a + int_num;

					/* set the clear mask and generate the int */
					m_intr.ack_mask = 0x0004 << int_num;
					goto generate_int;
				}
			}

		if (BIT(m_reloc, 14))
			continue;

		/* check external interrupts */
		for (int int_num = 0; int_num < 4; int_num++)
		{
			if ((m_intr.ext[int_num] & 0x0f) == priority)
			{
				LOGMASKED(LOG_INTERRUPTS, "Int%d priority=%d\n", int_num, priority);

				/* if we're already servicing something at this level, don't generate anything new */
				if ((m_intr.in_service & (0x10 << int_num)) && !(m_intr.ext[int_num] & EXTINT_CTRL_SFNM))
					return;

				/* if there's something pending, generate an interrupt */
				if (m_intr.request & (0x10 << int_num))
				{
					if((int_num >= 2) && (m_intr.ext[int_num - 2] & EXTINT_CTRL_CASCADE))
					{
						logerror("i186: %06x: irq %d use when set for cascade mode\n", m_pc, int_num);
						m_intr.request &= ~(0x10 << int_num);
						continue;
					}
					/* otherwise, generate an interrupt for this request */
					new_vector = 0x0c + int_num;

					/* set the clear mask and generate the int */
					m_intr.ack_mask = 0x0010 << int_num;
					goto generate_int;
				}
				else if ((m_intr.in_service & (0x10 << int_num)) && (m_intr.ext[int_num] & EXTINT_CTRL_SFNM))
					return; // if an irq is in service and sfnm is enabled, stop here
			}
		}
	}
	m_intr.pending = 0;
	if (!BIT(m_reloc, 14))
		set_input_line(0, CLEAR_LINE);
	else
		m_irmx_irq_cb(CLEAR_LINE);
	return;

generate_int:
	/* generate the appropriate interrupt */
	m_intr.poll_status = 0x8000 | new_vector;
	if (!m_intr.pending)
	{
		if (!BIT(m_reloc, 14))
			set_input_line(0, ASSERT_LINE);
		else
			m_irmx_irq_cb(ASSERT_LINE);
	}
	m_intr.pending = 1;
	LOGMASKED(LOG_INTERRUPTS, "(%f) **** Requesting interrupt vector %02X\n", machine().time().as_double(), new_vector);
}


void i80186_cpu_device::handle_eoi(int data)
{
	bool handled = false;

	/* specific case */
	if (!(data & 0x8000))
	{
		/* turn off the appropriate in-service bit */
		switch (data & 0x1f)
		{
			case 0x08:  m_intr.in_service &= ~0x01; break;
			case 0x12:  m_intr.in_service &= ~0x01; break;
			case 0x13:  m_intr.in_service &= ~0x01; break;
			case 0x0a:  m_intr.in_service &= ~0x04; break;
			case 0x0b:  m_intr.in_service &= ~0x08; break;
			case 0x0c:  m_intr.in_service &= ~0x10; break;
			case 0x0d:  m_intr.in_service &= ~0x20; break;
			case 0x0e:  m_intr.in_service &= ~0x40; break;
			case 0x0f:  m_intr.in_service &= ~0x80; break;
			default:    logerror("%05X:ERROR - 80186 EOI with unknown vector %02X\n", m_pc, data & 0x1f);
		}
		LOGMASKED(LOG_INTERRUPTS, "(%f) **** Got EOI for vector %02X\n", machine().time().as_double(), data & 0x1f);
	}
	/* non-specific case */
	else
	{
		/* loop over priorities */
		for (int priority = 0; priority <= 7 && !handled; priority++)
		{
			/* check for in-service timers */
			if (BIT(m_reloc, 14))
			{
				for (int int_num = 0; int_num < 2 && !handled; int_num++)
				{
					int mask = int_num ? (8 << int_num) : 1;
					if ((m_intr.timer[int_num] & 0x07) == priority && (m_intr.in_service & mask))
					{
						m_intr.in_service &= ~mask;
						LOGMASKED(LOG_INTERRUPTS, "(%f) **** Got EOI for timer%d\n", machine().time().as_double(), int_num);
						handled = true;
					}
				}
			}
			else
			{
				if ((m_intr.timer[0] & 0x07) == priority && (m_intr.in_service & 0x01))
				{
					m_intr.in_service &= ~0x01;
					LOGMASKED(LOG_INTERRUPTS, "(%f) **** Got EOI for timer\n", machine().time().as_double());
					handled = true;
				}
			}

			/* check for in-service DMA interrupts */
			for (int int_num = 0; int_num < 2 && !handled; int_num++)
				if ((m_intr.dma[int_num] & 0x07) == priority && (m_intr.in_service & (0x04 << int_num)))
				{
					m_intr.in_service &= ~(0x04 << int_num);
					LOGMASKED(LOG_INTERRUPTS, "(%f) **** Got EOI for DMA%d\n", machine().time().as_double(), int_num);
					handled = true;
				}

			if (BIT(m_reloc, 14))
				continue;

			/* check external interrupts */
			for (int int_num = 0; int_num < 4 && !handled; int_num++)
				if ((m_intr.ext[int_num] & 0x07) == priority && (m_intr.in_service & (0x10 << int_num)))
				{
					m_intr.in_service &= ~(0x10 << int_num);
					LOGMASKED(LOG_INTERRUPTS, "(%f) **** Got EOI for INT%d\n", machine().time().as_double(), int_num);
					handled = true;
				}
		}
	}
	update_interrupt_state();
}

/* Trigger an external interrupt, optionally supplying the vector to take */
void i80186_cpu_device::external_int(uint16_t int_num, int state)
{
	if (BIT(m_reloc, 14))
	{
		if (!int_num)
		{
			set_input_line(0, state);
			return;
		}
		logerror("irq to line %d in irmx mode\n", int_num);
		return;
	}

	if (!(m_intr.ext_state & (1 << int_num)) == !state)
		return;

	LOGMASKED(LOG_INTERRUPTS_EXT, "generating external int %02X\n", int_num);

	if (!state)
	{
		m_intr.request &= ~(0x10 << int_num);
		m_intr.ack_mask &= ~(0x10 << int_num);
		m_intr.ext_state &= ~(1 << int_num);
	}
	else // Turn on the requested request bit and handle interrupt
	{
		m_intr.request |= 0x10 << int_num;
		m_intr.ext_state |= 1 << int_num;
	}
	update_interrupt_state();
}

/*************************************
 *
 *  80186 internal timers
 *
 *************************************/

TIMER_CALLBACK_MEMBER(i80186_cpu_device::timer_elapsed)
{
	int which = param;
	timer_state *t = &m_timer[which];

	LOGMASKED(LOG_TIMER, "Hit interrupt callback for timer %d\n", which);

	/* set the max count bit */
	t->control |= 0x0020;

	/* request an interrupt */
	if (t->control & 0x2000)
	{
		m_intr.status |= 0x01 << which;
		update_interrupt_state();
		LOGMASKED(LOG_TIMER, "  Generating timer interrupt\n");
	}

	if (which == 2)
	{
		if ((m_dma[0].control & (TIMER_DRQ | ST_STOP)) == (TIMER_DRQ | ST_STOP))
			drq_callback(0);
		if ((m_dma[1].control & (TIMER_DRQ | ST_STOP)) == (TIMER_DRQ | ST_STOP))
			drq_callback(1);
		if ((m_timer[0].control & 0x800c) == 0x8008)
			inc_timer(0);
		if ((m_timer[1].control & 0x800c) == 0x8008)
			inc_timer(1);
	}
	else
	{
		if (!(t->control & 2))
		{
			if (which)
				m_out_tmrout1_func(1);
			else
				m_out_tmrout0_func(1);
		}
		else
		{
			if (which)
				m_out_tmrout1_func((t->control & 0x1000) ? 1 : 0);
			else
				m_out_tmrout0_func((t->control & 0x1000) ? 1 : 0);
		}
	}

	/* if we're continuous or altcounting, reset */
	if ((t->control & 1) || ((t->control & 2) && !(t->control & 0x1000)))
	{
		if ((t->control & 2) && !(t->control & 0x1000))
			t->control |= 0x1000;
		else
			t->control &= ~0x1000;

		restart_timer(which);
		LOGMASKED(LOG_TIMER, "  Repriming interrupt\n");
	}
	else
	{
		t->int_timer->adjust(attotime::never, which);
		t->control &= ~0x9000;
	}
	t->count = 0;
}


void i80186_cpu_device::restart_timer(int which)
{
	timer_state *t = &m_timer[which];
	/* Only run timer 0,1 when not incremented via timer 2 pre-scaler */
	if (which != 2 && (t->control & 0x800c) == 0x8008)
		return;

	int count = (t->control & 0x1000) ? t->maxB : t->maxA;
	if (!(t->control & 4))
		t->int_timer->adjust(cycles_to_attotime(4 * (count ? count : 0x10000)), which);
}

void i80186_cpu_device::internal_timer_sync(int which)
{
	timer_state *t = &m_timer[which];

	/* if we have a timing timer running, adjust the count */
	if ((t->control & 0x8000) && !(t->control & 0x0c) && t->int_timer->enabled())
		t->count = ((t->control & 0x1000) ? t->maxB : t->maxA) - attotime_to_cycles(t->int_timer->remaining()) / 4;
}

void i80186_cpu_device::inc_timer(int which)
{
	timer_state *t = &m_timer[which];

	t->count++;
	if (t->control & 2)
	{
		if (t->count == ((t->control & 0x1000) ? t->maxB : t->maxA))
			timer_elapsed(which);
	}
	else if (t->count == t->maxA)
		timer_elapsed(which);
}

void i80186_cpu_device::internal_timer_update(int which, int new_count, int new_maxA, int new_maxB, int new_control)
{
	timer_state *t = &m_timer[which];
	bool update_int_timer = false;

	LOGMASKED(LOG_TIMER, "internal_timer_update: %d, new_count=%d, new_maxA=%d, new_maxB=%d, new_control=%d\n", which, new_count, new_maxA, new_maxB, new_control);

	/* if we have a new count and we're on, update things */
	if (new_count != -1)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = true;
		}
		t->count = new_count;
	}

	/* if we have a new max and we're on, update things */
	if (new_maxA != -1 && new_maxA != t->maxA)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = true;
		}

		t->maxA = new_maxA;

		if (new_maxA == 0)
		{
			new_maxA = 0x10000;
		}
	}

	/* if we have a new max and we're on, update things */
	if (new_maxB != -1 && new_maxB != t->maxB)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = true;
		}

		t->maxB = new_maxB;

		if (new_maxB == 0)
		{
			new_maxB = 0x10000;
		}
	}

	/* handle control changes */
	if (new_control != -1)
	{
		uint16_t resbits = (which == 2) ? 0x1fde : 0x1fc0;

		/* merge back in the bits we don't modify */
		new_control = (new_control & ~resbits) | (t->control & resbits);

		/* handle the /INH bit */
		if (!(new_control & 0x4000))
			new_control = (new_control & ~0x8000) | (t->control & 0x8000);
		new_control &= ~0x4000;

		/* if we have real changes, update things */
		uint16_t diff = new_control ^ t->control;
		if (diff != 0)
		{
			/* if we're going off, make sure our timers are gone */
			if ((diff & 0x8000) && !(new_control & 0x8000))
			{
				/* compute the final count */
				internal_timer_sync(which);
				update_int_timer = true;
			}

			/* if we're going on, start the timers running except with external clock or prescale */
			else if ((diff & 0x8000) && (new_control & 0x8000) && !(new_control & 0xc))
			{
				update_int_timer = true;
			}

			/* if something about the interrupt timer changed, force an update */
			if (!(diff & 0x8000) && (diff & 0x2000))
			{
				internal_timer_sync(which);
				update_int_timer = true;
			}
		}

		/* RIU is cleared whenever ALT = 0 */
		if (!(new_control & 0x0002))
			new_control &= ~0x1000;

		/* set the new control register */
		t->control = new_control;
	}

	/* update the interrupt timer */
	if (update_int_timer)
	{
		if ((t->control & 0x8004) == 0x8000 && (!(t->control & 0x10) || t->int_timer->enabled()))
		{
			int diff = ((t->control & 0x1000) ? t->maxB : t->maxA) - t->count;
			if (diff <= 0)
				diff += 0x10000;
			t->int_timer->adjust(cycles_to_attotime(4 * diff), which);
			LOGMASKED(LOG_TIMER, "Set interrupt timer for %d\n", which);
		}
		else
		{
			t->int_timer->adjust(attotime::never, which);
		}
	}
}

void i80186_cpu_device::external_tmrin(int which, int state)
{
	// TODO: make this an actual edge trigger
	if (state)
	{
		if ((m_timer[which].control & 0x8004) == 0x8004)
			inc_timer(which);
		else if ((m_timer[which].control & 0x8014) == 0x8010)
		{
			m_timer[which].count = 0;
			restart_timer(which);
			LOGMASKED(LOG_TIMER, "Retriggered timer %d\n", which);
		}
	}
}



/*************************************
 *
 *  80186 internal DMA
 *
 *************************************/

void i80186_cpu_device::update_dma_control(int which, int new_control)
{
	dma_state *d = &m_dma[which];

	/* handle the CHG bit */
	if (!(new_control & CHG_NOCHG))
		new_control = (new_control & ~ST_STOP) | (d->control & ST_STOP);
	new_control &= ~CHG_NOCHG;

	LOGMASKED(LOG_DMA, "Initiated DMA %d - count = %04X, source = %04X, dest = %04X\n", which, d->count, d->source, d->dest);

	/* set the new control register */
	d->control = new_control;
}

void i80186_cpu_device::drq_callback(int which)
{
	dma_state *dma = &m_dma[which];

	uint16_t  dma_word;
	uint8_t   dma_byte;
	uint8_t   incdec_size;

	LOGMASKED(LOG_DMA_HIFREQ, "Control=%04X, src=%05X, dest=%05X, count=%04X\n", dma->control, dma->source, dma->dest, dma->count);

	if (!(dma->control & ST_STOP))
	{
		LOGMASKED(LOG_DMA, "%05X:ERROR! - drq%d with dma channel stopped\n", m_pc, which);
		return;
	}

	address_space *dest_space = (dma->control & DEST_MIO) ? m_program : m_io;
	address_space *src_space = (dma->control & SRC_MIO) ? m_program : m_io;

	// Do the transfer, 80188 is incapable of word transfers
	if ((dma->control & BYTE_WORD) && (m_program->data_width() == 16))
	{
		dma_word = src_space->read_word_unaligned(dma->source);
		dest_space->write_word_unaligned(dma->dest, dma_word);
		incdec_size = 2;
	}
	else
	{
		dma_byte = src_space->read_byte(dma->source);
		dest_space->write_byte(dma->dest, dma_byte);
		incdec_size = 1;
	}

	// Increment or Decrement destination and source pointers as needed
	switch (dma->control & DEST_INCDEC_MASK)
	{
		case DEST_DECREMENT:
			dma->dest -= incdec_size;
			break;
		case DEST_INCREMENT:
			dma->dest += incdec_size;
			break;
	}

	switch (dma->control & SRC_INCDEC_MASK)
	{
		case SRC_DECREMENT:
			dma->source -= incdec_size;
			break;
		case SRC_INCREMENT:
			dma->source += incdec_size;
			break;
	}

	// decrement count
	dma->count -= 1;

	// Terminate if count is zero, and terminate flag set
	if (((dma->control & TERMINATE_ON_ZERO) || !(dma->control & SYNC_MASK)) && dma->count == 0)
	{
		dma->control &= ~ST_STOP;
		LOGMASKED(LOG_DMA, "DMA terminated\n");
	}

	// Interrupt if count is zero, and interrupt flag set
	if ((dma->control & INTERRUPT_ON_ZERO) && dma->count == 0)
	{
		LOGMASKED(LOG_DMA_HIFREQ, "DMA%d - requesting interrupt: count = %04X, source = %04X\n", which, dma->count, dma->source);
		m_intr.request |= 0x04 << which;
		update_interrupt_state();
	}
}

uint16_t i80186_cpu_device::internal_port_r(offs_t offset, uint16_t mem_mask)
{
	int temp, which;

	switch (offset)
	{
		case 0x10:
			LOGMASKED(LOG_PORTS, "%05X:read interrupt vector\n", m_pc);
			return m_intr.vector;

		case 0x11:
			LOGMASKED(LOG_PORTS, "%05X:ERROR - read from 80186 EOI\n", m_pc);
			break;

		case 0x12:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 interrupt poll\n", m_pc);
			if (m_intr.poll_status & 0x8000)
				inta_callback(*this, 0);
			return m_intr.poll_status;

		case 0x13:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 interrupt poll status\n", m_pc);
			return m_intr.poll_status;

		case 0x14:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 interrupt mask\n", m_pc);
			temp  = (m_intr.timer[0]  >> 3) & 0x01;
			temp |= (m_intr.dma[0] >> 1) & 0x04;
			temp |= (m_intr.dma[1] >> 0) & 0x08;
			if (BIT(m_reloc, 14))
			{
				temp |= (m_intr.timer[1] << 1) & 0x10;
				temp |= (m_intr.timer[2] << 2) & 0x20;
			}
			else
			{
				temp |= (m_intr.ext[0] << 1) & 0x10;
				temp |= (m_intr.ext[1] << 2) & 0x20;
				temp |= (m_intr.ext[2] << 3) & 0x40;
				temp |= (m_intr.ext[3] << 4) & 0x80;
			}
			return temp;

		case 0x15:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 interrupt priority mask\n", m_pc);
			return m_intr.priority_mask;

		case 0x16:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 interrupt in-service\n", m_pc);
			return m_intr.in_service;

		case 0x17:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 interrupt request\n", m_pc);
			temp = m_intr.request & ~0x0001;
			if (m_intr.status & 0x0007)
				temp |= 1;
			return temp;

		case 0x18:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 interrupt status\n", m_pc);
			return m_intr.status;

		case 0x19:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 timer interrupt control\n", m_pc);
			return m_intr.timer[0];

		case 0x1a:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA 0 interrupt control\n", m_pc);
			return m_intr.dma[0];

		case 0x1b:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA 1 interrupt control\n", m_pc);
			return m_intr.dma[1];

		case 0x1c:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 INT 0 interrupt control\n", m_pc);
			if (BIT(m_reloc, 14))
				return m_intr.timer[1];
			else
				return m_intr.ext[0];

		case 0x1d:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 INT 1 interrupt control\n", m_pc);
			if (BIT(m_reloc, 14))
				return m_intr.timer[2];
			else
				return m_intr.ext[1];

		case 0x1e:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 INT 2 interrupt control\n", m_pc);
			if (BIT(m_reloc, 14))
				return 0;
			else
				return m_intr.ext[2];

		case 0x1f:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 INT 3 interrupt control\n", m_pc);
			if (BIT(m_reloc, 14))
				return 0;
			else
				return m_intr.ext[3];

		case 0x28:
		case 0x2c:
		case 0x30:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 Timer %d count\n", m_pc, (offset - 0x28) / 4);
			which = (offset - 0x28) / 4;
			if (ACCESSING_BITS_0_7)
				internal_timer_sync(which);
			return m_timer[which].count;

		case 0x29:
		case 0x2d:
		case 0x31:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 Timer %d max A\n", m_pc, (offset - 0x29) / 4);
			which = (offset - 0x29) / 4;
			return m_timer[which].maxA;

		case 0x2a:
		case 0x2e:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 Timer %d max B\n", m_pc, (offset - 0x2a) / 4);
			which = (offset - 0x2a) / 4;
			return m_timer[which].maxB;

		case 0x2b:
		case 0x2f:
		case 0x33:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 Timer %d control\n", m_pc, (offset - 0x2b) / 4);
			which = (offset - 0x2b) / 4;
			return m_timer[which].control;

		case 0x50:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 upper chip select\n", m_pc);
			return m_mem.upper;

		case 0x51:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 lower chip select\n", m_pc);
			return m_mem.lower;

		case 0x52:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 peripheral chip select\n", m_pc);
			return m_mem.peripheral;

		case 0x53:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 middle chip select\n", m_pc);
			return m_mem.middle;

		case 0x54:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 middle P chip select\n", m_pc);
			return m_mem.middle_size;

		case 0x60:
		case 0x68:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA%d lower source address\n", m_pc, (offset - 0x60) / 8);
			which = (offset - 0x60) / 8;
			return m_dma[which].source;

		case 0x61:
		case 0x69:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA%d upper source address\n", m_pc, (offset - 0x61) / 8);
			which = (offset - 0x61) / 8;
			return m_dma[which].source >> 16;

		case 0x62:
		case 0x6a:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA%d lower dest address\n", m_pc, (offset - 0x62) / 8);
			which = (offset - 0x62) / 8;
			return m_dma[which].dest;

		case 0x63:
		case 0x6b:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA%d upper dest address\n", m_pc, (offset - 0x63) / 8);
			which = (offset - 0x63) / 8;
			return m_dma[which].dest >> 16;

		case 0x64:
		case 0x6c:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA%d transfer count\n", m_pc, (offset - 0x64) / 8);
			which = (offset - 0x64) / 8;
			return m_dma[which].count;

		case 0x65:
		case 0x6d:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 DMA%d control\n", m_pc, (offset - 0x65) / 8);
			which = (offset - 0x65) / 8;
			return m_dma[which].control;

		case 0x7f:
			return m_reloc;

		default:
			LOGMASKED(LOG_PORTS, "%05X:read 80186 port %02X\n", m_pc, offset);
			break;
	}

	return 0x0000;
}

/*************************************
 *
 *  80186 internal I/O writes
 *
 *************************************/

void i80186_cpu_device::internal_port_w(offs_t offset, uint16_t data)
{
	int which;

	switch (offset)
	{
		case 0x10:
			LOGMASKED(LOG_PORTS, "%05X:write interrupt vector = %04X\n", m_pc, data);
			m_intr.vector = data & 0xf8;
			break;

		case 0x11:
			LOGMASKED(LOG_PORTS, "%05X:80186 EOI = %04X\n", m_pc, data);
			handle_eoi(data);
			update_interrupt_state();
			break;

		case 0x12:
			LOGMASKED(LOG_PORTS, "%05X:ERROR - write to 80186 interrupt poll = %04X\n", m_pc, data);
			break;

		case 0x13:
			LOGMASKED(LOG_PORTS, "%05X:ERROR - write to 80186 interrupt poll status = %04X\n", m_pc, data);
			break;

		case 0x14:
			LOGMASKED(LOG_PORTS, "%05X:80186 interrupt mask = %04X\n", m_pc, data);
			m_intr.timer[0]  = (m_intr.timer[0]  & ~0x08) | ((data << 3) & 0x08);
			m_intr.dma[0] = (m_intr.dma[0] & ~0x08) | ((data << 1) & 0x08);
			m_intr.dma[1] = (m_intr.dma[1] & ~0x08) | ((data << 0) & 0x08);
			if (BIT(m_reloc, 14))
			{
				m_intr.timer[1] = (m_intr.timer[1] & ~0x08) | ((data >> 1) & 0x08);
				m_intr.timer[2] = (m_intr.timer[2] & ~0x08) | ((data >> 2) & 0x08);
			}
			else
			{
				m_intr.ext[0] = (m_intr.ext[0] & ~0x08) | ((data >> 1) & 0x08);
				m_intr.ext[1] = (m_intr.ext[1] & ~0x08) | ((data >> 2) & 0x08);
				m_intr.ext[2] = (m_intr.ext[2] & ~0x08) | ((data >> 3) & 0x08);
				m_intr.ext[3] = (m_intr.ext[3] & ~0x08) | ((data >> 4) & 0x08);
			}
			update_interrupt_state();
			break;

		case 0x15:
			LOGMASKED(LOG_PORTS, "%05X:80186 interrupt priority mask = %04X\n", m_pc, data);
			m_intr.priority_mask = data & 0x0007;
			update_interrupt_state();
			break;

		case 0x16:
			LOGMASKED(LOG_PORTS, "%05X:80186 interrupt in-service = %04X\n", m_pc, data);
			m_intr.in_service = data & 0x00ff;
			update_interrupt_state();
			break;

		case 0x17:
			LOGMASKED(LOG_PORTS, "%05X:80186 interrupt request = %04X\n", m_pc, data);
			m_intr.request = (m_intr.request & ~0x000c) | (data & 0x000c);
			update_interrupt_state();
			break;

		case 0x18:
			LOGMASKED(LOG_PORTS, "%05X:WARNING - wrote to 80186 interrupt status = %04X\n", m_pc, data);
			m_intr.status = (m_intr.status & ~0x8007) | (data & 0x8007);
			update_interrupt_state();
			break;

		case 0x19:
			LOGMASKED(LOG_PORTS, "%05X:80186 timer interrupt contol = %04X\n", m_pc, data);
			m_intr.timer[0] = data & 0x000f;
			update_interrupt_state();
			break;

		case 0x1a:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA 0 interrupt control = %04X\n", m_pc, data);
			m_intr.dma[0] = data & 0x000f;
			update_interrupt_state();
			break;

		case 0x1b:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA 1 interrupt control = %04X\n", m_pc, data);
			m_intr.dma[1] = data & 0x000f;
			update_interrupt_state();
			break;

		case 0x1c:
			LOGMASKED(LOG_PORTS, "%05X:80186 INT 0 interrupt control = %04X\n", m_pc, data);
			if (BIT(m_reloc, 14))
				m_intr.timer[1] = data & 0x000f;
			else
				m_intr.ext[0] = data & 0x007f;
			update_interrupt_state();
			break;

		case 0x1d:
			LOGMASKED(LOG_PORTS, "%05X:80186 INT 1 interrupt control = %04X\n", m_pc, data);
			if (BIT(m_reloc, 14))
				m_intr.timer[2] = data & 0x000f;
			else
				m_intr.ext[1] = data & 0x007f;
			update_interrupt_state();
			break;

		case 0x1e:
			LOGMASKED(LOG_PORTS, "%05X:80186 INT 2 interrupt control = %04X\n", m_pc, data);
			if (!BIT(m_reloc, 14))
				m_intr.ext[2] = data & 0x001f;
			update_interrupt_state();
			break;

		case 0x1f:
			LOGMASKED(LOG_PORTS, "%05X:80186 INT 3 interrupt control = %04X\n", m_pc, data);
			if (!BIT(m_reloc, 14))
				m_intr.ext[3] = data & 0x001f;
			update_interrupt_state();
			break;

		case 0x28:
		case 0x2c:
		case 0x30:
			LOGMASKED(LOG_PORTS, "%05X:80186 Timer %d count = %04X\n", m_pc, (offset - 0x28) / 4, data);
			which = (offset - 0x28) / 4;
			internal_timer_update(which, data, -1, -1, -1);
			break;

		case 0x29:
		case 0x2d:
		case 0x31:
			LOGMASKED(LOG_PORTS, "%05X:80186 Timer %d max A = %04X\n", m_pc, (offset - 0x29) / 4, data);
			which = (offset - 0x29) / 4;
			internal_timer_update(which, -1, data, -1, -1);
			break;

		case 0x2a:
		case 0x2e:
			LOGMASKED(LOG_PORTS, "%05X:80186 Timer %d max B = %04X\n", m_pc, (offset - 0x2a) / 4, data);
			which = (offset - 0x2a) / 4;
			internal_timer_update(which, -1, -1, data, -1);
			break;

		case 0x2b:
		case 0x2f:
		case 0x33:
			LOGMASKED(LOG_PORTS, "%05X:80186 Timer %d control = %04X\n", m_pc, (offset - 0x2b) / 4, data);
			which = (offset - 0x2b) / 4;
			internal_timer_update(which, -1, -1, -1, data);
			break;

		case 0x50:
			LOGMASKED(LOG_PORTS, "%05X:80186 upper chip select = %04X\n", m_pc, data);
			m_mem.upper = data | 0xc038;
			m_out_chip_select_func(0, m_mem.upper, 0xffff);
			break;

		case 0x51:
			LOGMASKED(LOG_PORTS, "%05X:80186 lower chip select = %04X\n", m_pc, data);
			m_mem.lower = (data & 0x3fff) | 0x0038;
			m_out_chip_select_func(1, m_mem.lower, 0xffff);
			break;

		case 0x52:
			LOGMASKED(LOG_PORTS, "%05X:80186 peripheral chip select = %04X\n", m_pc, data);
			m_mem.peripheral = data | 0x0038;
			m_out_chip_select_func(2, m_mem.peripheral, 0xffff);
			break;

		case 0x53:
			LOGMASKED(LOG_PORTS, "%05X:80186 middle chip select = %04X\n", m_pc, data);
			m_mem.middle = data | 0x01f8;
			m_out_chip_select_func(3, m_mem.middle, 0xffff);
			break;

		case 0x54:
			LOGMASKED(LOG_PORTS, "%05X:80186 middle P chip select = %04X\n", m_pc, data);
			m_mem.middle_size = data | 0x8038;
			m_out_chip_select_func(4, m_mem.middle_size, 0xffff);
			break;

		case 0x60:
		case 0x68:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA%d lower source address = %04X\n", m_pc, (offset - 0x60) / 8, data);
			which = (offset - 0x60) / 8;
			m_dma[which].source = (m_dma[which].source & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0x61:
		case 0x69:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA%d upper source address = %04X\n", m_pc, (offset - 0x61) / 8, data);
			which = (offset - 0x61) / 8;
			m_dma[which].source = (m_dma[which].source & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0x62:
		case 0x6a:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA%d lower dest address = %04X\n", m_pc, (offset - 0x62) / 8, data);
			which = (offset - 0x62) / 8;
			m_dma[which].dest = (m_dma[which].dest & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0x63:
		case 0x6b:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA%d upper dest address = %04X\n", m_pc, (offset - 0x63) / 8, data);
			which = (offset - 0x63) / 8;
			m_dma[which].dest = (m_dma[which].dest & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0x64:
		case 0x6c:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA%d transfer count = %04X\n", m_pc, (offset - 0x64) / 8, data);
			which = (offset - 0x64) / 8;
			m_dma[which].count = data;
			break;

		case 0x65:
		case 0x6d:
			LOGMASKED(LOG_PORTS, "%05X:80186 DMA%d control = %04X\n", m_pc, (offset - 0x65) / 8, data);
			which = (offset - 0x65) / 8;
			update_dma_control(which, data);
			if((m_dma[which].control & (SYNC_MASK | ST_STOP | TIMER_DRQ)) == ST_STOP)
			{
				// TODO: don't do this
				while(m_dma[which].control & ST_STOP)
					drq_callback(which);
			}
			break;

		case 0x7f:
			// 80188 byte output to this port is *not* masked!
			LOGMASKED(LOG_PORTS, "%05X:80186 relocation register = %04X\n", m_pc, data);
			m_reloc = data;
			break;

		default:
			LOGMASKED(LOG_PORTS, "%05X:80186 port %02X = %04X\n", m_pc, offset, data);
			break;
	}
}
