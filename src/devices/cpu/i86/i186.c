// license:BSD-3-Clause
// copyright-holders:Carl
// Peripheral code from rmnimbus driver by Phill Harvey-Smith which is
// based on the Leland sound driver by Aaron Giles and Paul Leaman

#include "i186.h"
#include "debugger.h"
#include "i86inline.h"

#define LOG_PORTS           0
#define LOG_INTERRUPTS      0
#define LOG_INTERRUPTS_EXT  0
#define LOG_TIMER           0
#define LOG_DMA             0

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
#define SRC_MIO                 0X1000
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
const UINT8 i80186_cpu_device::m_i80186_timing[] =
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

const device_type I80186 = &device_creator<i80186_cpu_device>;
const device_type I80188 = &device_creator<i80188_cpu_device>;

i80188_cpu_device::i80188_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i80186_cpu_device(mconfig, I80188, "I80188", tag, owner, clock, "i80188", __FILE__, 8)
{
	memcpy(m_timing, m_i80186_timing, sizeof(m_i80186_timing));
	m_fetch_xor = 0;
	static_set_irq_acknowledge_callback(*this, device_irq_acknowledge_delegate(FUNC(i80186_cpu_device::int_callback), this));
}

i80186_cpu_device::i80186_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8086_common_cpu_device(mconfig, I80186, "I80186", tag, owner, clock, "i80186", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0)
	, m_read_slave_ack_func(*this)
	, m_out_chip_select_func(*this)
	, m_out_tmrout0_func(*this)
	, m_out_tmrout1_func(*this)
{
	memcpy(m_timing, m_i80186_timing, sizeof(m_i80186_timing));
	m_fetch_xor = BYTE_XOR_LE(0);
	static_set_irq_acknowledge_callback(*this, device_irq_acknowledge_delegate(FUNC(i80186_cpu_device::int_callback), this));
}

i80186_cpu_device::i80186_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int data_bus_size)
	: i8086_common_cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, data_bus_size, 16, 0)
	, m_read_slave_ack_func(*this)
	, m_out_chip_select_func(*this)
	, m_out_tmrout0_func(*this)
	, m_out_tmrout1_func(*this)
{
}

UINT8 i80186_cpu_device::fetch_op()
{
	UINT8 data = m_direct->read_byte(pc(), m_fetch_xor);
	m_ip++;
	return data;
}

UINT8 i80186_cpu_device::fetch()
{
	UINT8 data = m_direct->read_byte(pc(), m_fetch_xor);
	m_ip++;
	return data;
}

void i80186_cpu_device::execute_run()
{
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
			if ( m_pending_irq && m_no_interrupt == 0 )
			{
				if ( m_pending_irq & NMI_IRQ )
				{
					interrupt(2);
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

			/* No interrupt allowed between last instruction and this one */
			if ( m_no_interrupt )
			{
				m_no_interrupt--;
			}

			/* trap should allow one instruction to be executed */
			if ( m_fire_trap )
			{
				if ( m_fire_trap >= 2 )
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

		debugger_instruction_hook( this, pc() );

		UINT8 op = fetch_op();

		switch(op)
		{
			case 0x60: // i_pusha
				{
					UINT32 tmp = m_regs.w[SP];

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
					UINT32 low,high,tmp;
					m_modrm = fetch();
					low = GetRMWord();
					high = GetnextRMWord();
					tmp = RegWord();
					if (tmp<low || tmp>high)
						interrupt(5);
					CLK(BOUND);
					logerror("%s: %06x: bound %04x high %04x low %04x tmp\n", tag(), pc(), high, low, tmp);
				}
				break;

			case 0x68: // i_push_d16
				PUSH( fetch_word() );
				CLK(PUSH_IMM);
				break;

			case 0x69: // i_imul_d16
				{
					UINT32 tmp;
					DEF_r16w();
					tmp = fetch_word();
					m_dst = (INT32)((INT16)m_src)*(INT32)((INT16)tmp);
					m_CarryVal = m_OverVal = (((INT32)m_dst) >> 15 != 0) && (((INT32)m_dst) >> 15 != -1);
					RegWord(m_dst);
					CLKM(IMUL_RRI16, IMUL_RMI16);
				}
				break;

			case 0x6a: // i_push_d8
				PUSH( (UINT16)((INT16)((INT8)fetch())) );
				CLK(PUSH_IMM);
				break;

			case 0x6b: // i_imul_d8
				{
					UINT32 src2;
					DEF_r16w();
					src2= (UINT16)((INT16)((INT8)fetch()));
					m_dst = (INT32)((INT16)m_src)*(INT32)((INT16)src2);
					m_CarryVal = m_OverVal = (((INT32)m_dst) >> 15 != 0) && (((INT32)m_dst) >> 15 != -1);
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
				CLKM(MOV_SR,MOV_SM);
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
					logerror("%s: %06x: Mov Sreg - Invalid register\n", tag(), pc());
					m_ip = m_prev_ip;
					interrupt(6);
					break;
				}
				break;

			case 0xc0: // i_rotshft_bd8
				{
					UINT8 c;
					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					c = fetch() & 0x1f;
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

			case 0xc1: // i_rotshft_wd8
				{
					UINT8 c;
					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					c = fetch() & 0x1f;
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

			case 0xc8: // i_enter
				{
					UINT16 nb = fetch();
					UINT32 level;

					nb |= fetch() << 8;
					level = fetch();
					CLK(!level ? ENTER0 : (level == 1) ? ENTER1 : ENTER_BASE);
					if(level > 1)
						m_icount -= level * m_timing[ENTER_COUNT];
					PUSH(m_regs.w[BP]);
					m_regs.w[BP] = m_regs.w[SP];
					m_regs.w[SP] -= nb;
					for (int i=1; i<level; i++)
					{
						PUSH( GetMemW(SS,m_regs.w[BP] - i*2) );
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
					UINT8 c;

					m_modrm = fetch();
					m_src = GetRMByte();
					m_dst = m_src;
					c = m_regs.b[CL] & 0x1f;
					CLKM(ROT_REG_BASE,ROT_M16_BASE);
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
					UINT8 c;

					m_modrm = fetch();
					m_src = GetRMWord();
					m_dst = m_src;
					c = m_regs.b[CL] & 0x1f;
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
				if(m_reloc & 0x8000)
				{
					m_ip = m_prev_ip;
					interrupt(7);
					break;
				}
				m_modrm = fetch();
				GetRMByte();
				CLK(NOP);
				// The 80187 has the FSTSW AX instruction
				if((m_modrm == 0xe0) && (op == 0xdf))
					m_regs.w[AX] = 0xffff;  // FPU not present
				break;

			case 0xf2: // i_repne
			case 0xf3:
				{
					bool pass = false;
					UINT8 next = repx_op();
					UINT16 c = m_regs.w[CX];

					switch (next)
					{
					case 0x6c:  CLK(OVERRIDE); if (c) do { i_insb();  c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6d:  CLK(OVERRIDE); if (c) do { i_insw();  c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6e:  CLK(OVERRIDE); if (c) do { i_outsb(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					case 0x6f:  CLK(OVERRIDE); if (c) do { i_outsw(); c--; } while (c>0 && m_icount>0);          m_regs.w[CX]=c; m_seg_prefix = false; m_seg_prefix_next = false; break;
					default:
						// Decrement IP and pass on
						m_ip -= 1 + (m_seg_prefix_next ? 1 : 0);
						pass = true;
						break;
					}
					if(!pass)
					{
						if(c)
							m_ip = m_prev_ip;
						break;
					}
				}
				// through to default
			default:
				if(!common_op(op))
				{
					m_icount -= 10; // UD fault timing?
					logerror("%s: %06x: Invalid Opcode %02x\n", tag(), pc(), op);
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
	state_add( I8086_ES, "ES", m_sregs[ES] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_CS, "CS", m_sregs[CS] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_SS, "SS", m_sregs[SS] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_DS, "DS", m_sregs[DS] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_VECTOR, "V", m_int_vector).callimport().callexport().formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).callimport().callexport().formatstr("%05X");

	// register for savestates
	save_item(NAME(m_timer[0].control));
	save_item(NAME(m_timer[0].maxA));
	save_item(NAME(m_timer[0].maxB));
	save_item(NAME(m_timer[0].active_count));
	save_item(NAME(m_timer[0].count));
	save_item(NAME(m_timer[1].control));
	save_item(NAME(m_timer[1].maxA));
	save_item(NAME(m_timer[1].maxB));
	save_item(NAME(m_timer[1].active_count));
	save_item(NAME(m_timer[1].count));
	save_item(NAME(m_timer[2].control));
	save_item(NAME(m_timer[2].maxA));
	save_item(NAME(m_timer[2].count));
	save_item(NAME(m_dma[0].source));
	save_item(NAME(m_dma[0].dest));
	save_item(NAME(m_dma[0].count));
	save_item(NAME(m_dma[0].control));
	save_item(NAME(m_dma[1].source));
	save_item(NAME(m_dma[1].dest));
	save_item(NAME(m_dma[1].count));
	save_item(NAME(m_dma[1].control));
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

	// zerofill
	memset(m_timer, 0, sizeof(m_timer));
	memset(m_dma, 0, sizeof(m_dma));
	memset(&m_intr, 0, sizeof(intr_state));
	memset(&m_mem, 0, sizeof(mem_state));
	m_reloc = 0;

	m_timer[0].int_timer = timer_alloc(TIMER_INT0);
	m_timer[1].int_timer = timer_alloc(TIMER_INT1);
	m_timer[2].int_timer = timer_alloc(TIMER_INT2);

	m_out_tmrout0_func.resolve_safe();
	m_out_tmrout1_func.resolve_safe();
	m_read_slave_ack_func.resolve_safe(0);
	m_out_chip_select_func.resolve_safe();
}

void i80186_cpu_device::device_reset()
{
	i8086_common_cpu_device::device_reset();
	/* reset the interrupt state */
	m_intr.priority_mask    = 0x0007;
	m_intr.timer            = 0x000f;
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

	for (int i = 0; i < ARRAY_LENGTH(m_dma); i++)
	{
		m_dma[i].drq_state = false;
		m_dma[i].control = 0;
	}

	for (int i = 0; i < ARRAY_LENGTH(m_timer); i++)
	{
		m_timer[i].control = 0;
		m_timer[i].maxA = 0;
		m_timer[i].maxB = 0;
		m_timer[i].active_count = false;
		m_timer[i].count = 0;
	}
}

UINT8 i80186_cpu_device::read_port_byte(UINT16 port)
{
	if(!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		UINT16 ret = internal_port_r(*m_io, (port >> 1) - ((m_reloc & 0xff) << 7), (port & 1) ? 0xff00 : 0x00ff);
		return (port & 1) ? (ret >> 8) : (ret & 0xff);
	}
	return m_io->read_byte(port);
}

UINT16 i80186_cpu_device::read_port_word(UINT16 port)
{
	if(!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		if(port & 1)
		{
			UINT8 low = read_port_byte(port);
			return read_port_byte(port + 1) << 8 | low;
		}
		return internal_port_r(*m_io, (port >> 1) - ((m_reloc & 0xff) << 7));
	}
	return m_io->read_word_unaligned(port);
}

void i80186_cpu_device::write_port_byte(UINT16 port, UINT8 data)
{
	if(!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
		internal_port_w(*m_io, (port >> 1) - ((m_reloc & 0xff) << 7), (port & 1) ? (data << 8) : data, (port & 1) ? 0xff00 : 0x00ff);
	else
		m_io->write_byte(port, data);
}

void i80186_cpu_device::write_port_word(UINT16 port, UINT16 data)
{
	if(!(m_reloc & 0x1000) && (port >> 8) == (m_reloc & 0xff))
	{
		if(port & 1)
		{
			write_port_byte(port, data & 0xff);
			write_port_byte(port + 1, data >> 8);
		}
		else
			internal_port_w(*m_io, (port >> 1) - ((m_reloc & 0xff) << 7), data);
	}
	else
		m_io->write_word_unaligned(port, data);
}

/*************************************
 *
 *  80186 interrupt controller
 *
 *************************************/
IRQ_CALLBACK_MEMBER(i80186_cpu_device::int_callback)
{
	UINT8   vector;
	UINT16  old;
	UINT16  oldreq;

	if (LOG_INTERRUPTS)
		logerror("(%f) **** Acknowledged interrupt vector %02X\n", machine().time().as_double(), m_intr.poll_status & 0x1f);

	/* clear the interrupt */
	set_input_line(0, CLEAR_LINE);
	m_intr.pending = 0;

	oldreq = m_intr.request;

	/* clear the request and set the in-service bit */
	if(m_intr.ack_mask & 0xf0)
	{
		int i;
		for(i = 0; i < 4; i++)
			if((m_intr.ack_mask >> (i + 4)) & 1)
				break;
		if(!(m_intr.ext[i] & EXTINT_CTRL_LTM))
			m_intr.request &= ~m_intr.ack_mask;
	}
	else
		m_intr.request &= ~m_intr.ack_mask;

	if((LOG_INTERRUPTS) && (m_intr.request!=oldreq))
		logerror("intr.request changed from %02X to %02X\n",oldreq,m_intr.request);

	old = m_intr.in_service;

	m_intr.in_service |= m_intr.ack_mask;

	if((LOG_INTERRUPTS) && (m_intr.in_service!=old))
		logerror("intr.in_service changed from %02X to %02X\n",old,m_intr.in_service);

	if (m_intr.ack_mask == 0x0001)
	{
		switch (m_intr.poll_status & 0x1f)
		{
			case 0x08:  m_intr.status &= ~0x01; break;
			case 0x12:  m_intr.status &= ~0x02; break;
			case 0x13:  m_intr.status &= ~0x04; break;
		}
	}
	m_intr.ack_mask = 0;

	/* a request no longer pending */
	m_intr.poll_status &= ~0x8000;

	/* return the vector */
	switch(m_intr.poll_status & 0x1F)
	{
		case 0x0C: vector = (m_intr.ext[0] & EXTINT_CTRL_CASCADE) ? m_read_slave_ack_func(0) : (m_intr.poll_status & 0x1f); break;
		case 0x0D: vector = (m_intr.ext[1] & EXTINT_CTRL_CASCADE) ? m_read_slave_ack_func(1) : (m_intr.poll_status & 0x1f); break;
		default:
			vector = m_intr.poll_status & 0x1f; break;
	}

	if (LOG_INTERRUPTS)
	{
		logerror("intr.ext[0]=%04X intr.ext[1]=%04X\n",m_intr.ext[0],m_intr.ext[1]);
		logerror("Int %02X Calling vector %02X\n",m_intr.poll_status,vector);
	}

	return vector;
}


void i80186_cpu_device::update_interrupt_state()
{
	int new_vector = 0;
	int Priority;
	int IntNo;

	if (LOG_INTERRUPTS)
		logerror("update_interrupt_status: req=%04X stat=%04X serv=%04X priority_mask=%4X\n", m_intr.request, m_intr.status, m_intr.in_service, m_intr.priority_mask);

	/* loop over priorities */
	for (Priority = 0; Priority <= m_intr.priority_mask; Priority++)
	{
		/* note: by checking 4 bits, we also verify that the mask is off */
		if ((m_intr.timer & 0x0F) == Priority)
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

		/* check DMA interrupts */
		for (IntNo = 0; IntNo < 2; IntNo++)
			if ((m_intr.dma[IntNo] & 0x0F) == Priority)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (m_intr.in_service & (0x04 << IntNo))
					return;

				/* if there's something pending, generate an interrupt */
				if (m_intr.request & (0x04 << IntNo))
				{
					new_vector = 0x0a + IntNo;

					/* set the clear mask and generate the int */
					m_intr.ack_mask = 0x0004 << IntNo;
					goto generate_int;
				}
			}

		/* check external interrupts */
		for (IntNo = 0; IntNo < 4; IntNo++)
		{
			if ((m_intr.ext[IntNo] & 0x0F) == Priority)
			{
				if (LOG_INTERRUPTS)
					logerror("Int%d priority=%d\n",IntNo,Priority);

				/* if we're already servicing something at this level, don't generate anything new */
				if ((m_intr.in_service & (0x10 << IntNo)) && !(m_intr.ext[IntNo] & EXTINT_CTRL_SFNM))
					return;

				/* if there's something pending, generate an interrupt */
				if (m_intr.request & (0x10 << IntNo))
				{
					if((IntNo >= 2) && (m_intr.ext[IntNo - 2] & EXTINT_CTRL_CASCADE))
					{
						logerror("i186: %06x: irq %d use when set for cascade mode\n", pc(), IntNo);
						m_intr.request &= ~(0x10 << IntNo);
						continue;
					}
					/* otherwise, generate an interrupt for this request */
					new_vector = 0x0c + IntNo;

					/* set the clear mask and generate the int */
					m_intr.ack_mask = 0x0010 << IntNo;
					goto generate_int;
				}
				else if ((m_intr.in_service & (0x10 << IntNo)) && (m_intr.ext[IntNo] & EXTINT_CTRL_SFNM))
					return; // if an irq is in service and sfnm is enabled, stop here
			}
		}
	}
	m_intr.pending = 0;
	set_input_line(0, CLEAR_LINE);
	return;

generate_int:
	/* generate the appropriate interrupt */
	m_intr.poll_status = 0x8000 | new_vector;
	if (!m_intr.pending)
		set_input_line(0, ASSERT_LINE);
	m_intr.pending = 1;
	if (LOG_INTERRUPTS) logerror("(%f) **** Requesting interrupt vector %02X\n", machine().time().as_double(), new_vector);
}


void i80186_cpu_device::handle_eoi(int data)
{
	int Priority;
	int IntNo;
	int handled=0;

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
			default:    logerror("%05X:ERROR - 80186 EOI with unknown vector %02X\n", pc(), data & 0x1f);
		}
		if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for vector %02X\n", machine().time().as_double(), data & 0x1f);
	}

	/* non-specific case */
	else
	{
		/* loop over priorities */
		for (Priority = 0; ((Priority <= 7) && !handled); Priority++)
		{
			/* check for in-service timers */
			if ((m_intr.timer & 0x07) == Priority && (m_intr.in_service & 0x01))
			{
				m_intr.in_service &= ~0x01;
				if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for timer\n", machine().time().as_double());
				handled=1;
			}

			/* check for in-service DMA interrupts */
			for (IntNo = 0; ((IntNo < 2) && !handled) ; IntNo++)
				if ((m_intr.dma[IntNo] & 0x07) == Priority && (m_intr.in_service & (0x04 << IntNo)))
				{
					m_intr.in_service &= ~(0x04 << IntNo);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for DMA%d\n", machine().time().as_double(), IntNo);
					handled=1;
				}

			/* check external interrupts */
			for (IntNo = 0; ((IntNo < 4) && !handled) ; IntNo++)
				if ((m_intr.ext[IntNo] & 0x07) == Priority && (m_intr.in_service & (0x10 << IntNo)))
				{
					m_intr.in_service &= ~(0x10 << IntNo);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for INT%d\n", machine().time().as_double(), IntNo);
					handled=1;
				}
		}
	}
	update_interrupt_state();
}

/* Trigger an external interrupt, optionally supplying the vector to take */
void i80186_cpu_device::external_int(UINT16 intno, int state)
{
	if (!(m_intr.ext_state & (1 << intno)) == !state)
		return;

	if (LOG_INTERRUPTS_EXT) logerror("generating external int %02X\n",intno);

	if (!state)
	{
		m_intr.request &= ~(0x10 << intno);
		m_intr.ack_mask &= ~(0x10 << intno);
		m_intr.ext_state &= ~(1 << intno);
	}
	else // Turn on the requested request bit and handle interrupt
	{
		m_intr.request |= (0x10 << intno);
		m_intr.ext_state |= (1 << intno);
	}
	update_interrupt_state();
}

/*************************************
 *
 *  80186 internal timers
 *
 *************************************/

void i80186_cpu_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_INT0:
		case TIMER_INT1:
		case TIMER_INT2:
		{
			int which = param;
			timer_state *t = &m_timer[which];

			if (LOG_TIMER) logerror("Hit interrupt callback for timer %d\n", which);

			/* set the max count bit */
			t->control |= 0x0020;

			/* request an interrupt */
			if (t->control & 0x2000)
			{
				m_intr.status |= 0x01 << which;
				update_interrupt_state();
				if (LOG_TIMER) logerror("  Generating timer interrupt\n");
			}

			if(which == 2)
			{
				if((m_dma[0].control & (TIMER_DRQ | ST_STOP)) == TIMER_DRQ)
					drq_callback(0);
				if((m_dma[1].control & (TIMER_DRQ | ST_STOP)) == TIMER_DRQ)
					drq_callback(1);
				if((m_timer[0].control & 0x800c) == 0x8008)
					inc_timer(0);
				if((m_timer[1].control & 0x800c) == 0x8008)
					inc_timer(1);
			}
			else
			{
				if(!(t->control & 2))
				{
					if(which)
						m_out_tmrout1_func(1);
					else
						m_out_tmrout0_func(1);
				}
				else
				{
					if(which)
						m_out_tmrout1_func(t->active_count);
					else
						m_out_tmrout0_func(t->active_count);
				}
			}

			/* if we're continuous, reset */
			if (t->control & 0x0001)
			{
				int count;
				if((t->control & 2) && (which != 2))
				{
					count = t->active_count ? t->maxA : t->maxB;
					t->active_count = !t->active_count;
				}
				else
					count = t->maxA;

				count = count ? count : 0x10000;
				if(!(t->control & 4))
					t->int_timer->adjust((attotime::from_hz(clock()/8) * count), which);
				if (LOG_TIMER) logerror("  Repriming interrupt\n");
			}
			else
			{
				t->int_timer->adjust(attotime::never, which);
				t->control &= ~0x8000;
			}
			t->count = 0;
			break;
		}

		default:
			break;
	}
}


void i80186_cpu_device::internal_timer_sync(int which)
{
	timer_state *t = &m_timer[which];

	/* if we have a timing timer running, adjust the count */
	if ((t->control & 0x8000) && !(t->control & 0x0c))
		t->count = (((which != 2) && t->active_count) ? t->maxB : t->maxA) - t->int_timer->remaining().as_ticks(clock() / 8);
}

void i80186_cpu_device::inc_timer(int which)
{
	timer_state *t = &m_timer[which];

	t->count++;
	if (t->control & 2)
	{
		if (t->count == (t->active_count ? t->maxB : t->maxA))
			device_timer(*t->int_timer, which, which, NULL);
	}
	else if (t->count == t->maxA)
		device_timer(*t->int_timer, which, which, NULL);
}

void i80186_cpu_device::internal_timer_update(int which, int new_count, int new_maxA, int new_maxB, int new_control)
{
	timer_state *t = &m_timer[which];
	int update_int_timer = 0;

	if (LOG_TIMER)
		logerror("internal_timer_update: %d, new_count=%d, new_maxA=%d, new_maxB=%d,new_control=%d\n", which, new_count, new_maxA, new_maxB, new_control);

	/* if we have a new count and we're on, update things */
	if (new_count != -1)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = 1;
		}
		t->count = new_count;
	}

	/* if we have a new max and we're on, update things */
	if (new_maxA != -1 && new_maxA != t->maxA)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(which);
			update_int_timer = 1;
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
			update_int_timer = 1;
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
		int diff;
		UINT16 resbits = (which == 2) ? 0x1fde : 0x1fc0;

		/* merge back in the bits we don't modify */
		new_control = (new_control & ~resbits) | (t->control & resbits);

		/* handle the /INH bit */
		if (!(new_control & 0x4000))
			new_control = (new_control & ~0x8000) | (t->control & 0x8000);
		new_control &= ~0x4000;

		/* check for control bits we don't handle */
		diff = new_control ^ t->control;
		if (diff & 0x0010)
			logerror("%05X:ERROR! -unsupported timer mode %04X\n", pc(), new_control);

		/* if we have real changes, update things */
		if (diff != 0)
		{
			/* if we're going off, make sure our timers are gone */
			if ((diff & 0x8000) && !(new_control & 0x8000))
			{
				/* compute the final count */
				internal_timer_sync(which);
				update_int_timer = 1;
			}

			/* if we're going on, start the timers running except with external clock or prescale */
			else if ((diff & 0x8000) && (new_control & 0x8000) && !(new_control & 0xc))
			{
				update_int_timer = 1;
			}

			/* if something about the interrupt timer changed, force an update */
			if (!(diff & 0x8000) && (diff & 0x2000))
			{
				internal_timer_sync(which);
				update_int_timer = 1;
			}
		}

		/* set the new control register */
		t->control = new_control;
	}

	/* update the interrupt timer */
	if (update_int_timer)
	{
		t->active_count = 0;
		if ((t->control & 0x8000) && !(t->control & 4))
		{
			int diff = t->maxA - t->count;
			if (diff <= 0)
				diff += 0x10000;
			t->int_timer->adjust(attotime::from_hz(clock()/8) * diff, which);
			if (LOG_TIMER) logerror("Set interrupt timer for %d\n", which);
		}
		else
		{
			t->int_timer->adjust(attotime::never, which);
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
	int diff;

	/* handle the CHG bit */
	if (!(new_control & CHG_NOCHG))
		new_control = (new_control & ~ST_STOP) | (d->control & ST_STOP);
	new_control &= ~CHG_NOCHG;

	/* check for control bits we don't handle */
	diff = new_control ^ d->control;
	if ((LOG_DMA) && (diff & 0x6811))
		logerror("%05X:ERROR! - unsupported DMA mode %04X\n", pc(), new_control);

	if (LOG_DMA) logerror("Initiated DMA %d - count = %04X, source = %04X, dest = %04X\n", which, d->count, d->source, d->dest);

	/* set the new control register */
	d->control = new_control;
}

void i80186_cpu_device::drq_callback(int which)
{
	dma_state *dma = &m_dma[which];

	UINT16  dma_word;
	UINT8   dma_byte;
	UINT8   incdec_size;

	if (LOG_DMA>1)
		logerror("Control=%04X, src=%05X, dest=%05X, count=%04X\n",dma->control,dma->source,dma->dest,dma->count);

	if (!(dma->control & ST_STOP))
	{
		if(LOG_DMA)
			logerror("%05X:ERROR! - drq%d with dma channel stopped\n", pc(), which);
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
	if (((dma->control & TERMINATE_ON_ZERO) || !(dma->control & SYNC_MASK)) && (dma->count == 0))
	{
		dma->control &= ~ST_STOP;
		if (LOG_DMA) logerror("DMA terminated\n");
	}

	// Interrupt if count is zero, and interrupt flag set
	if ((dma->control & INTERRUPT_ON_ZERO) && (dma->count == 0))
	{
		if (LOG_DMA>1) logerror("DMA%d - requesting interrupt: count = %04X, source = %04X\n", which, dma->count, dma->source);
		m_intr.request |= 0x04 << which;
		update_interrupt_state();
	}
}

READ16_MEMBER(i80186_cpu_device::internal_port_r)
{
	int temp, which;

	switch (offset)
	{
		case 0x11:
			logerror("%05X:ERROR - read from 80186 EOI\n", pc());
			break;

		case 0x12:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll\n", pc());
			if (m_intr.poll_status & 0x8000)
				int_callback(*this, 0);
			return m_intr.poll_status;

		case 0x13:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll status\n", pc());
			return m_intr.poll_status;

		case 0x14:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt mask\n", pc());
			temp  = (m_intr.timer  >> 3) & 0x01;
			temp |= (m_intr.dma[0] >> 1) & 0x04;
			temp |= (m_intr.dma[1] >> 0) & 0x08;
			temp |= (m_intr.ext[0] << 1) & 0x10;
			temp |= (m_intr.ext[1] << 2) & 0x20;
			temp |= (m_intr.ext[2] << 3) & 0x40;
			temp |= (m_intr.ext[3] << 4) & 0x80;
			return temp;

		case 0x15:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt priority mask\n", pc());
			return m_intr.priority_mask;

		case 0x16:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt in-service\n", pc());
			return m_intr.in_service;

		case 0x17:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt request\n", pc());
			temp = m_intr.request & ~0x0001;
			if (m_intr.status & 0x0007)
				temp |= 1;
			return temp;

		case 0x18:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt status\n", pc());
			return m_intr.status;

		case 0x19:
			if (LOG_PORTS) logerror("%05X:read 80186 timer interrupt control\n", pc());
			return m_intr.timer;

		case 0x1a:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 0 interrupt control\n", pc());
			return m_intr.dma[0];

		case 0x1b:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 1 interrupt control\n", pc());
			return m_intr.dma[1];

		case 0x1c:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 0 interrupt control\n", pc());
			return m_intr.ext[0];

		case 0x1d:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 1 interrupt control\n", pc());
			return m_intr.ext[1];

		case 0x1e:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 2 interrupt control\n", pc());
			return m_intr.ext[2];

		case 0x1f:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 3 interrupt control\n", pc());
			return m_intr.ext[3];

		case 0x28:
		case 0x2c:
		case 0x30:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d count\n", pc(), (offset - 0x28) / 4);
			which = (offset - 0x28) / 4;
			if (ACCESSING_BITS_0_7)
				internal_timer_sync(which);
			return m_timer[which].count;

		case 0x29:
		case 0x2d:
		case 0x31:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d max A\n", pc(), (offset - 0x29) / 4);
			which = (offset - 0x29) / 4;
			return m_timer[which].maxA;

		case 0x2a:
		case 0x2e:
			logerror("%05X:read 80186 Timer %d max B\n", pc(), (offset - 0x2a) / 4);
			which = (offset - 0x2a) / 4;
			return m_timer[which].maxB;

		case 0x2b:
		case 0x2f:
		case 0x33:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d control\n", pc(), (offset - 0x2b) / 4);
			which = (offset - 0x2b) / 4;
			return m_timer[which].control;

		case 0x50:
			if (LOG_PORTS) logerror("%05X:read 80186 upper chip select\n", pc());
			return m_mem.upper;

		case 0x51:
			if (LOG_PORTS) logerror("%05X:read 80186 lower chip select\n", pc());
			return m_mem.lower;

		case 0x52:
			if (LOG_PORTS) logerror("%05X:read 80186 peripheral chip select\n", pc());
			return m_mem.peripheral;

		case 0x53:
			if (LOG_PORTS) logerror("%05X:read 80186 middle chip select\n", pc());
			return m_mem.middle;

		case 0x54:
			if (LOG_PORTS) logerror("%05X:read 80186 middle P chip select\n", pc());
			return m_mem.middle_size;

		case 0x60:
		case 0x68:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower source address\n", pc(), (offset - 0x60) / 8);
			which = (offset - 0x60) / 8;
			return m_dma[which].source;

		case 0x61:
		case 0x69:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper source address\n", pc(), (offset - 0x61) / 8);
			which = (offset - 0x61) / 8;
			return m_dma[which].source >> 16;

		case 0x62:
		case 0x6a:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower dest address\n", pc(), (offset - 0x62) / 8);
			which = (offset - 0x62) / 8;
			return m_dma[which].dest;

		case 0x63:
		case 0x6b:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper dest address\n", pc(), (offset - 0x63) / 8);
			which = (offset - 0x63) / 8;
			return m_dma[which].dest >> 16;

		case 0x64:
		case 0x6c:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d transfer count\n", pc(), (offset - 0x64) / 8);
			which = (offset - 0x64) / 8;
			return m_dma[which].count;

		case 0x65:
		case 0x6d:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d control\n", pc(), (offset - 0x65) / 8);
			which = (offset - 0x65) / 8;
			return m_dma[which].control;

		case 0x7f:
			return m_reloc;

		default:
			logerror("%05X:read 80186 port %02X\n", pc(), offset);
			break;
	}

	return 0x0000;
}

/*************************************
 *
 *  80186 internal I/O writes
 *
 *************************************/

WRITE16_MEMBER(i80186_cpu_device::internal_port_w)
{
	int which;

	switch (offset)
	{
		case 0x11:
			if (LOG_PORTS) logerror("%05X:80186 EOI = %04X\n", pc(), data);
			handle_eoi(0x8000);
			update_interrupt_state();
			break;

		case 0x12:
			logerror("%05X:ERROR - write to 80186 interrupt poll = %04X\n", pc(), data);
			break;

		case 0x13:
			logerror("%05X:ERROR - write to 80186 interrupt poll status = %04X\n", pc(), data);
			break;

		case 0x14:
			if (LOG_PORTS) logerror("%05X:80186 interrupt mask = %04X\n", pc(), data);
			m_intr.timer  = (m_intr.timer  & ~0x08) | ((data << 3) & 0x08);
			m_intr.dma[0] = (m_intr.dma[0] & ~0x08) | ((data << 1) & 0x08);
			m_intr.dma[1] = (m_intr.dma[1] & ~0x08) | ((data << 0) & 0x08);
			m_intr.ext[0] = (m_intr.ext[0] & ~0x08) | ((data >> 1) & 0x08);
			m_intr.ext[1] = (m_intr.ext[1] & ~0x08) | ((data >> 2) & 0x08);
			m_intr.ext[2] = (m_intr.ext[2] & ~0x08) | ((data >> 3) & 0x08);
			m_intr.ext[3] = (m_intr.ext[3] & ~0x08) | ((data >> 4) & 0x08);
			update_interrupt_state();
			break;

		case 0x15:
			if (LOG_PORTS) logerror("%05X:80186 interrupt priority mask = %04X\n", pc(), data);
			m_intr.priority_mask = data & 0x0007;
			update_interrupt_state();
			break;

		case 0x16:
			if (LOG_PORTS) logerror("%05X:80186 interrupt in-service = %04X\n", pc(), data);
			m_intr.in_service = data & 0x00ff;
			update_interrupt_state();
			break;

		case 0x17:
			if (LOG_PORTS) logerror("%05X:80186 interrupt request = %04X\n", pc(), data);
			m_intr.request = (m_intr.request & ~0x00c0) | (data & 0x00c0);
			update_interrupt_state();
			break;

		case 0x18:
			if (LOG_PORTS) logerror("%05X:WARNING - wrote to 80186 interrupt status = %04X\n", pc(), data);
			m_intr.status = (m_intr.status & ~0x8007) | (data & 0x8007);
			update_interrupt_state();
			break;

		case 0x19:
			if (LOG_PORTS) logerror("%05X:80186 timer interrupt contol = %04X\n", pc(), data);
			m_intr.timer = data & 0x000f;
			update_interrupt_state();
			break;

		case 0x1a:
			if (LOG_PORTS) logerror("%05X:80186 DMA 0 interrupt control = %04X\n", pc(), data);
			m_intr.dma[0] = data & 0x000f;
			update_interrupt_state();
			break;

		case 0x1b:
			if (LOG_PORTS) logerror("%05X:80186 DMA 1 interrupt control = %04X\n", pc(), data);
			m_intr.dma[1] = data & 0x000f;
			update_interrupt_state();
			break;

		case 0x1c:
			if (LOG_PORTS) logerror("%05X:80186 INT 0 interrupt control = %04X\n", pc(), data);
			m_intr.ext[0] = data & 0x007f;
			update_interrupt_state();
			break;

		case 0x1d:
			if (LOG_PORTS) logerror("%05X:80186 INT 1 interrupt control = %04X\n", pc(), data);
			m_intr.ext[1] = data & 0x007f;
			update_interrupt_state();
			break;

		case 0x1e:
			if (LOG_PORTS) logerror("%05X:80186 INT 2 interrupt control = %04X\n", pc(), data);
			m_intr.ext[2] = data & 0x001f;
			update_interrupt_state();
			break;

		case 0x1f:
			if (LOG_PORTS) logerror("%05X:80186 INT 3 interrupt control = %04X\n", pc(), data);
			m_intr.ext[3] = data & 0x001f;
			update_interrupt_state();
			break;

		case 0x28:
		case 0x2c:
		case 0x30:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d count = %04X\n", pc(), (offset - 0x28) / 4, data);
			which = (offset - 0x28) / 4;
			internal_timer_update(which, data, -1, -1, -1);
			break;

		case 0x29:
		case 0x2d:
		case 0x31:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max A = %04X\n", pc(), (offset - 0x29) / 4, data);
			which = (offset - 0x29) / 4;
			internal_timer_update(which, -1, data, -1, -1);
			break;

		case 0x2a:
		case 0x2e:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max B = %04X\n", pc(), (offset - 0x2a) / 4, data);
			which = (offset - 0x2a) / 4;
			internal_timer_update(which, -1, -1, data, -1);
			break;

		case 0x2b:
		case 0x2f:
		case 0x33:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d control = %04X\n", pc(), (offset - 0x2b) / 4, data);
			which = (offset - 0x2b) / 4;
			internal_timer_update(which, -1, -1, -1, data);
			break;

		case 0x50:
			if (LOG_PORTS) logerror("%05X:80186 upper chip select = %04X\n", pc(), data);
			m_mem.upper = data | 0xc038;
			m_out_chip_select_func(0, m_mem.upper, 0xffff);
			break;

		case 0x51:
			if (LOG_PORTS) logerror("%05X:80186 lower chip select = %04X\n", pc(), data);
			m_mem.lower = (data & 0x3fff) | 0x0038;
			m_out_chip_select_func(1, m_mem.lower, 0xffff);
			break;

		case 0x52:
			if (LOG_PORTS) logerror("%05X:80186 peripheral chip select = %04X\n", pc(), data);
			m_mem.peripheral = data | 0x0038;
			m_out_chip_select_func(2, m_mem.peripheral, 0xffff);
			break;

		case 0x53:
			if (LOG_PORTS) logerror("%05X:80186 middle chip select = %04X\n", pc(), data);
			m_mem.middle = data | 0x01f8;
			m_out_chip_select_func(3, m_mem.middle, 0xffff);
			break;

		case 0x54:
			if (LOG_PORTS) logerror("%05X:80186 middle P chip select = %04X\n", pc(), data);
			m_mem.middle_size = data | 0x8038;
			m_out_chip_select_func(4, m_mem.middle_size, 0xffff);
			break;

		case 0x60:
		case 0x68:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower source address = %04X\n", pc(), (offset - 0x60) / 8, data);
			which = (offset - 0x60) / 8;
			m_dma[which].source = (m_dma[which].source & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0x61:
		case 0x69:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper source address = %04X\n", pc(), (offset - 0x61) / 8, data);
			which = (offset - 0x61) / 8;
			m_dma[which].source = (m_dma[which].source & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0x62:
		case 0x6a:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower dest address = %04X\n", pc(), (offset - 0x62) / 8, data);
			which = (offset - 0x62) / 8;
			m_dma[which].dest = (m_dma[which].dest & ~0x0ffff) | (data & 0x0ffff);
			break;

		case 0x63:
		case 0x6b:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper dest address = %04X\n", pc(), (offset - 0x63) / 8, data);
			which = (offset - 0x63) / 8;
			m_dma[which].dest = (m_dma[which].dest & ~0xf0000) | ((data << 16) & 0xf0000);
			break;

		case 0x64:
		case 0x6c:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d transfer count = %04X\n", pc(), (offset - 0x64) / 8, data);
			which = (offset - 0x64) / 8;
			m_dma[which].count = data;
			break;

		case 0x65:
		case 0x6d:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d control = %04X\n", pc(), (offset - 0x65) / 8, data);
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
			if (LOG_PORTS) logerror("%05X:80186 relocation register = %04X\n", pc(), data);
			if ((data & 0x1fff) != (m_reloc & 0x1fff))
			{
				UINT32 newmap = (data & 0xfff) << 8;
				UINT32 oldmap = (m_reloc & 0xfff) << 8;
				if (!(data & 0x1000) || ((data & 0x1000) && (m_reloc & 0x1000)))
					m_program->unmap_readwrite(oldmap, oldmap + 0xff);
				if (data & 0x1000) // TODO: make work with 80188 if needed
					m_program->install_readwrite_handler(newmap, newmap + 0xff, read16_delegate(FUNC(i80186_cpu_device::internal_port_r), this), write16_delegate(FUNC(i80186_cpu_device::internal_port_w), this));
			}
			m_reloc = data;

			break;

		default:
			logerror("%05X:80186 port %02X = %04X\n", pc(), offset, data);
			break;
	}
}
