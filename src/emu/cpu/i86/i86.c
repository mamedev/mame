/****************************************************************************

    NEC V20/V30/V33 emulator modified back to a 8086/80186 emulator

    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
    on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
    on the i286 emulator by Fabrice Frances which had initial work based on
    David Hedley's pcemu(!).

****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "i86.h"

enum SREGS { ES=0, CS, SS, DS };
enum WREGS { AX=0, CX, DX, BX, SP, BP, SI, DI };

#define I8086_NMI_INT_VECTOR 2

enum BREGS {
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6),
	SPL = NATIVE_ENDIAN_VALUE_LE_BE(0x8, 0x9),
	SPH = NATIVE_ENDIAN_VALUE_LE_BE(0x9, 0x8),
	BPL = NATIVE_ENDIAN_VALUE_LE_BE(0xa, 0xb),
	BPH = NATIVE_ENDIAN_VALUE_LE_BE(0xb, 0xa),
	SIL = NATIVE_ENDIAN_VALUE_LE_BE(0xc, 0xd),
	SIH = NATIVE_ENDIAN_VALUE_LE_BE(0xd, 0xc),
	DIL = NATIVE_ENDIAN_VALUE_LE_BE(0xe, 0xf),
	DIH = NATIVE_ENDIAN_VALUE_LE_BE(0xf, 0xe)
};

enum
{
	EXCEPTION, IRET,                                /* EXCEPTION, iret */
	INT3, INT_IMM, INTO_NT, INTO_T,                 /* intS */
	OVERRIDE,                                       /* SEGMENT OVERRIDES */
	FLAG_OPS, LAHF, SAHF,                           /* FLAG OPERATIONS */
	AAA, AAS, AAM, AAD,                             /* ARITHMETIC ADJUSTS */
	DAA, DAS,                                       /* DECIMAL ADJUSTS */
	CBW, CWD,                                       /* SIGN EXTENSION */
	HLT, LOAD_PTR, LEA, NOP, WAIT, XLAT,            /* MISC */

	JMP_SHORT, JMP_NEAR, JMP_FAR,                   /* DIRECT jmpS */
	JMP_R16, JMP_M16, JMP_M32,                      /* INDIRECT jmpS */
	CALL_NEAR, CALL_FAR,                            /* DIRECT callS */
	CALL_R16, CALL_M16, CALL_M32,                   /* INDIRECT callS */
	RET_NEAR, RET_FAR, RET_NEAR_IMM, RET_FAR_IMM,   /* RETURNS */
	JCC_NT, JCC_T, JCXZ_NT, JCXZ_T,                 /* CONDITIONAL jmpS */
	LOOP_NT, LOOP_T, LOOPE_NT, LOOPE_T,             /* LOOPS */

	IN_IMM8, IN_IMM16, IN_DX8, IN_DX16,             /* PORT READS */
	OUT_IMM8, OUT_IMM16, OUT_DX8, OUT_DX16,         /* PORT WRITES */

	MOV_RR8, MOV_RM8, MOV_MR8,                      /* MOVE, 8-BIT */
	MOV_RI8, MOV_MI8,                               /* MOVE, 8-BIT IMMEDIATE */
	MOV_RR16, MOV_RM16, MOV_MR16,                   /* MOVE, 16-BIT */
	MOV_RI16, MOV_MI16,                             /* MOVE, 16-BIT IMMEDIATE */
	MOV_AM8, MOV_AM16, MOV_MA8, MOV_MA16,           /* MOVE, al/ax MEMORY */
	MOV_SR, MOV_SM, MOV_RS, MOV_MS,                 /* MOVE, SEGMENT REGISTERS */
	XCHG_RR8, XCHG_RM8,                             /* EXCHANGE, 8-BIT */
	XCHG_RR16, XCHG_RM16, XCHG_AR16,                /* EXCHANGE, 16-BIT */

	PUSH_R16, PUSH_M16, PUSH_SEG, PUSHF,            /* PUSHES */
	POP_R16, POP_M16, POP_SEG, POPF,                /* POPS */

	ALU_RR8, ALU_RM8, ALU_MR8,                      /* alu OPS, 8-BIT */
	ALU_RI8, ALU_MI8, ALU_MI8_RO,                   /* alu OPS, 8-BIT IMMEDIATE */
	ALU_RR16, ALU_RM16, ALU_MR16,                   /* alu OPS, 16-BIT */
	ALU_RI16, ALU_MI16, ALU_MI16_RO,                /* alu OPS, 16-BIT IMMEDIATE */
	ALU_R16I8, ALU_M16I8, ALU_M16I8_RO,             /* alu OPS, 16-BIT W/8-BIT IMMEDIATE */
	MUL_R8, MUL_R16, MUL_M8, MUL_M16,               /* mul */
	IMUL_R8, IMUL_R16, IMUL_M8, IMUL_M16,           /* imul */
	DIV_R8, DIV_R16, DIV_M8, DIV_M16,               /* div */
	IDIV_R8, IDIV_R16, IDIV_M8, IDIV_M16,           /* idiv */
	INCDEC_R8, INCDEC_R16, INCDEC_M8, INCDEC_M16,   /* inc/dec */
	NEGNOT_R8, NEGNOT_R16, NEGNOT_M8, NEGNOT_M16,   /* neg/not */

	ROT_REG_1, ROT_REG_BASE, ROT_REG_BIT,           /* REG SHIFT/ROTATE */
	ROT_M8_1, ROT_M8_BASE, ROT_M8_BIT,              /* M8 SHIFT/ROTATE */
	ROT_M16_1, ROT_M16_BASE, ROT_M16_BIT,           /* M16 SHIFT/ROTATE */

	CMPS8, REP_CMPS8_BASE, REP_CMPS8_COUNT,         /* cmps 8-BIT */
	CMPS16, REP_CMPS16_BASE, REP_CMPS16_COUNT,      /* cmps 16-BIT */
	SCAS8, REP_SCAS8_BASE, REP_SCAS8_COUNT,         /* scas 8-BIT */
	SCAS16, REP_SCAS16_BASE, REP_SCAS16_COUNT,      /* scas 16-BIT */
	LODS8, REP_LODS8_BASE, REP_LODS8_COUNT,         /* lods 8-BIT */
	LODS16, REP_LODS16_BASE, REP_LODS16_COUNT,      /* lods 16-BIT */
	STOS8, REP_STOS8_BASE, REP_STOS8_COUNT,         /* stos 8-BIT */
	STOS16, REP_STOS16_BASE, REP_STOS16_COUNT,      /* stos 16-BIT */
	MOVS8, REP_MOVS8_BASE, REP_MOVS8_COUNT,         /* movs 8-BIT */
	MOVS16, REP_MOVS16_BASE, REP_MOVS16_COUNT,      /* movs 16-BIT */

	INS8, REP_INS8_BASE, REP_INS8_COUNT,            /* (80186) ins 8-BIT */
	INS16, REP_INS16_BASE, REP_INS16_COUNT,         /* (80186) ins 16-BIT */
	OUTS8, REP_OUTS8_BASE, REP_OUTS8_COUNT,         /* (80186) outs 8-BIT */
	OUTS16, REP_OUTS16_BASE, REP_OUTS16_COUNT,      /* (80186) outs 16-BIT */
	PUSH_IMM, PUSHA, POPA,                          /* (80186) push IMMEDIATE, pusha/popa */
	IMUL_RRI8, IMUL_RMI8,                           /* (80186) imul IMMEDIATE 8-BIT */
	IMUL_RRI16, IMUL_RMI16,                         /* (80186) imul IMMEDIATE 16-BIT */
	ENTER0, ENTER1, ENTER_BASE, ENTER_COUNT, LEAVE, /* (80186) enter/leave */
	BOUND                                           /* (80186) bound */
};

const UINT8 i8086_cpu_device::m_i8086_timing[] =
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

#define CF      (m_CarryVal!=0)
#define SF      (m_SignVal<0)
#define ZF      (m_ZeroVal==0)
#define PF      m_parity_table[(UINT8)m_ParityVal]
#define AF      (m_AuxVal!=0)
#define OF      (m_OverVal!=0)


/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/


/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

/***************************************************************************/

const device_type I8086 = &device_creator<i8086_cpu_device>;
const device_type I8088 = &device_creator<i8088_cpu_device>;
const device_type I80186 = &device_creator<i80186_cpu_device>;
const device_type I80188 = &device_creator<i80188_cpu_device>;

i80188_cpu_device::i80188_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i80186_cpu_device(mconfig, I80188, "I80188", tag, owner, clock, "i80188", __FILE__, 8)
{
	memcpy(m_timing, m_i80186_timing, sizeof(m_i80186_timing));
}

i80186_cpu_device::i80186_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8086_common_cpu_device(mconfig, I80186, "I80186", tag, owner, clock, "i80186", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0)
{
	memcpy(m_timing, m_i80186_timing, sizeof(m_i80186_timing));
}

i80186_cpu_device::i80186_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int data_bus_size)
	: i8086_common_cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, data_bus_size, 16, 0)
{
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
					interrupt(I8086_NMI_INT_VECTOR);
					m_pending_irq &= ~NMI_IRQ;
				}
				else if ( m_IF )
				{
					/* the actual vector is retrieved after pushing flags */
					/* and clearing the IF */
					interrupt(-1);
				}
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
					CLK(!level ? ENTER0 : (level == 1) ? ENTER1 : (ENTER_BASE + (level * ENTER_COUNT)));
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
						m_ip -= 1;
						pass = true;
					}
					if(!pass)
					{
						if(c)
							m_ip = m_prev_ip;
						break;
					}
				}

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

i8088_cpu_device::i8088_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8086_cpu_device(mconfig, I8088, "I8088", tag, owner, clock, "i8088", __FILE__, 8)
{
	memcpy(m_timing, m_i8086_timing, sizeof(m_i8086_timing));
}

i8086_cpu_device::i8086_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8086_common_cpu_device(mconfig, I8086, "I8086", tag, owner, clock, "i8086", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0)
{
	memcpy(m_timing, m_i8086_timing, sizeof(m_i8086_timing));
}

i8086_cpu_device::i8086_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int data_bus_size)
	: i8086_common_cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, data_bus_size, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, data_bus_size, 16, 0)
{
}

void i8086_cpu_device::execute_run()
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
					interrupt(I8086_NMI_INT_VECTOR);
					m_pending_irq &= ~NMI_IRQ;
				}
				else if ( m_IF )
				{
					/* the actual vector is retrieved after pushing flags */
					/* and clearing the IF */
					interrupt(-1);
				}
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
			case 0x0f:
				m_sregs[CS] = POP();
				CLK(POP_SEG);
				break;

			case 0xd2: // i_rotshft_bcl
				{
					UINT8 c;

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
					UINT8 c;

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

			default:
				if(!common_op(op))
				{
					m_icount -= 10;
					logerror("%s: %06x: Invalid Opcode %02x\n", tag(), pc(), op);
					break;
				}
				break;
		}
	}
}

i8086_common_cpu_device::i8086_common_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_ip(0)
	, m_TF(0)
	, m_int_vector(0)
	, m_pending_irq(0)
	, m_nmi_state(0)
	, m_irq_state(0)
	, m_test_state(0)
	, m_pc(0)
{
	static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	/* Set up parity lookup table. */
	for (UINT16 i = 0;i < 256; i++)
	{
		UINT16 c = 0;
		for (UINT16 j = i; j > 0; j >>= 1)
		{
			if (j & 1) c++;
		}
		m_parity_table[i] = !(c & 1);
	}

	for (UINT16 i = 0; i < 256; i++)
	{
		m_Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		m_Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
	}

	for (UINT16 i = 0xc0; i < 0x100; i++)
	{
		m_Mod_RM.RM.w[i] = (WREGS)( i & 7 );
		m_Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}
}

void i8086_common_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			string.printf("%08X", pc() );
			break;

		case STATE_GENFLAGS:
			{
				UINT16 flags = CompressFlags();
				string.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
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
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	save_item(NAME(m_regs.w));
	save_item(NAME(m_sregs));
	save_item(NAME(m_ip));
	save_item(NAME(m_prev_ip));
	save_item(NAME(m_TF));
	save_item(NAME(m_IF));
	save_item(NAME(m_DF));
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

	// Register state for debugger
//  state_add( I8086_PC, "PC", m_PC ).callimport().callexport().formatstr("%04X");
	state_add( I8086_IP, "IP", m_ip         ).callimport().callexport().formatstr("%04X");
	state_add( I8086_AX, "AX", m_regs.w[AX] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_CX, "CX", m_regs.w[CS] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_DX, "DX", m_regs.w[DX] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_BX, "BX", m_regs.w[BX] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_SP, "SP", m_regs.w[SP] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_BP, "BP", m_regs.w[BP] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_SI, "SI", m_regs.w[SI] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_DI, "DI", m_regs.w[DI] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_ES, "ES", m_sregs[ES] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_CS, "CS", m_sregs[CS] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_SS, "SS", m_sregs[SS] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_DS, "DS", m_sregs[DS] ).callimport().callexport().formatstr("%04X");
	state_add( I8086_VECTOR, "V", m_int_vector).callimport().callexport().formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).callimport().callexport().formatstr("%05X");
	state_add(STATE_GENFLAGS, "GENFLAGS", m_TF).callimport().callexport().formatstr("%16s").noshow();

	m_icountptr = &m_icount;
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
	m_int_vector = 0;
	m_pending_irq = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_no_interrupt = 0;
	m_fire_trap = 0;
	m_prefix_base = 0;
	m_seg_prefix = false;
	m_seg_prefix_next = false;
	m_ea = 0;
	m_eo = 0;
	m_e16 = 0;
	m_modrm = 0;
	m_dst = 0;
	m_src = 0;
}


inline UINT32 i8086_common_cpu_device::pc()
{
	m_pc = ( m_sregs[CS] << 4 ) + m_ip;
	return m_pc;
}


inline UINT8 i8086_common_cpu_device::read_byte(UINT32 addr)
{
	return m_program->read_byte(addr);
}


inline UINT16 i8086_common_cpu_device::read_word(UINT32 addr)
{
	return m_program->read_word_unaligned(addr);
}


inline void i8086_common_cpu_device::write_byte(UINT32 addr, UINT8 data)
{
	m_program->write_byte(addr, data);
}


inline void i8086_common_cpu_device::write_word(UINT32 addr, UINT16 data)
{
	m_program->write_word_unaligned(addr, data);
}


inline UINT8 i8086_common_cpu_device::read_port_byte(UINT16 port)
{
	return m_io->read_byte(port);
}

inline UINT16 i8086_common_cpu_device::read_port_word(UINT16 port)
{
	return m_io->read_word_unaligned(port);
}

inline void i8086_common_cpu_device::write_port_byte(UINT16 port, UINT8 data)
{
	m_io->write_byte(port, data);
}

inline void i8086_common_cpu_device::write_port_word(UINT16 port, UINT16 data)
{
	m_io->write_word_unaligned(port, data);
}

inline UINT8 i8086_common_cpu_device::fetch_op()
{
	UINT8 data = m_direct->read_decrypted_byte( pc() );
	m_ip++;
	return data;
}


inline UINT8 i8086_common_cpu_device::fetch()
{
	UINT8 data = m_direct->read_raw_byte( pc() );
	m_ip++;
	return data;
}


inline UINT16 i8086_common_cpu_device::fetch_word()
{
	UINT16 data = fetch();
	data |= ( fetch() << 8 );
	return data;
}


inline UINT8 i8086_common_cpu_device::repx_op()
{
	UINT8 next = fetch_op();
	bool seg_prefix = false;
	int seg = 0;

	switch (next)
	{
	case 0x26:
		seg_prefix = true;
		seg = ES;
		break;
	case 0x2e:
		seg_prefix = true;
		seg = CS;
		break;
	case 0x36:
		seg_prefix = true;
		seg = SS;
		break;
	case 0x3e:
		seg_prefix = true;
		seg = DS;
		break;
	}

	if ( seg_prefix )
	{
		m_seg_prefix = true;
		m_seg_prefix_next = true;
		m_prefix_base = m_sregs[seg] << 4;
		next = fetch_op();
		CLK(OVERRIDE);
	}

	return next;
}


inline void i8086_common_cpu_device::CLK(UINT8 op)
{
	m_icount -= m_timing[op];
}


inline void i8086_common_cpu_device::CLKM(UINT8 op_reg, UINT8 op_mem)
{
	m_icount -= ( m_modrm >= 0xc0 ) ? m_timing[op_reg] : m_timing[op_mem];
}


inline UINT32 i8086_common_cpu_device::default_base(int seg)
{
	if ( m_seg_prefix && (seg==DS || seg==SS) )
	{
		return m_prefix_base;
	}
	else
	{
		return m_sregs[seg] << 4;
	}
}


inline UINT32 i8086_common_cpu_device::get_ea()
{
	switch( m_modrm & 0xc7 )
	{
	case 0x00:
		m_eo = m_regs.w[BX] + m_regs.w[SI];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x01:
		m_eo = m_regs.w[BX] + m_regs.w[DI];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x02:
		m_eo = m_regs.w[BP] + m_regs.w[SI];
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x03:
		m_eo = m_regs.w[BP] + m_regs.w[DI];
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x04:
		m_eo = m_regs.w[SI];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x05:
		m_eo = m_regs.w[DI];
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x06:
		m_eo = fetch_word();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x07:
		m_eo = m_regs.w[BX];
		m_ea = default_base(DS) + m_eo;
		break;

	case 0x40:
		m_eo = m_regs.w[BX] + m_regs.w[SI] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x41:
		m_eo = m_regs.w[BX] + m_regs.w[DI] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x42:
		m_eo = m_regs.w[BP] + m_regs.w[SI] + (INT8)fetch();
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x43:
		m_eo = m_regs.w[BP] + m_regs.w[DI] + (INT8)fetch();
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x44:
		m_eo = m_regs.w[SI] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x45:
		m_eo = m_regs.w[DI] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x46:
		m_eo = m_regs.w[BP] + (INT8)fetch();
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x47:
		m_eo = m_regs.w[BX] + (INT8)fetch();
		m_ea = default_base(DS) + m_eo;
		break;

	case 0x80:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BX] + m_regs.w[SI] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x81:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BX] + m_regs.w[DI] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x82:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BP] + m_regs.w[SI] + (INT16)m_e16;
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x83:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BP] + m_regs.w[DI] + (INT16)m_e16;
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x84:
		m_e16 = fetch_word();
		m_eo = m_regs.w[SI] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x85:
		m_e16 = fetch_word();
		m_eo = m_regs.w[DI] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	case 0x86:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BP] + (INT16)m_e16;
		m_ea = default_base(SS) + m_eo;
		break;
	case 0x87:
		m_e16 = fetch_word();
		m_eo = m_regs.w[BX] + (INT16)m_e16;
		m_ea = default_base(DS) + m_eo;
		break;
	}

	return m_ea;
}


inline void i8086_common_cpu_device::PutbackRMByte(UINT8 data)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = data;
	}
	else
	{
		write_byte( m_ea, data );
	}
}


inline void i8086_common_cpu_device::PutbackRMWord(UINT16 data)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = data;
	}
	else
	{
		write_word( m_ea, data );
	}
}

inline void i8086_common_cpu_device::PutImmRMWord()
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = fetch_word();
	}
	else
	{
		UINT32 addr = get_ea();
		write_word( addr, fetch_word() );
	}
}

inline void i8086_common_cpu_device::PutRMWord(UINT16 val)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ] = val;
	}
	else
	{
		write_word( get_ea(), val );
	}
}


inline void i8086_common_cpu_device::PutRMByte(UINT8 val)
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = val;
	}
	else
	{
		write_byte( get_ea(), val );
	}
}


inline void i8086_common_cpu_device::PutImmRMByte()
{
	if ( m_modrm >= 0xc0 )
	{
		m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ] = fetch();
	}
	else
	{
		UINT32 addr = get_ea();
		write_byte( addr, fetch() );
	}
}


inline void i8086_common_cpu_device::DEF_br8()
{
	m_modrm = fetch();
	m_src = RegByte();
	m_dst = GetRMByte();
}


inline void i8086_common_cpu_device::DEF_wr16()
{
	m_modrm = fetch();
	m_src = RegWord();
	m_dst = GetRMWord();
}


inline void i8086_common_cpu_device::DEF_r8b()
{
	m_modrm = fetch();
	m_dst = RegByte();
	m_src = GetRMByte();
}


inline void i8086_common_cpu_device::DEF_r16w()
{
	m_modrm = fetch();
	m_dst = RegWord();
	m_src = GetRMWord();
}


inline void i8086_common_cpu_device::DEF_ald8()
{
	m_src = fetch();
	m_dst = m_regs.b[AL];
}


inline void i8086_common_cpu_device::DEF_axd16()
{
	m_src = fetch_word();
	m_dst = m_regs.w[AX];
}



inline void i8086_common_cpu_device::RegByte(UINT8 data)
{
	m_regs.b[ m_Mod_RM.reg.b[ m_modrm ] ] = data;
}


inline void i8086_common_cpu_device::RegWord(UINT16 data)
{
	m_regs.w[ m_Mod_RM.reg.w[ m_modrm ] ] = data;
}


inline UINT8 i8086_common_cpu_device::RegByte()
{
	return m_regs.b[ m_Mod_RM.reg.b[ m_modrm ] ];
}


inline UINT16 i8086_common_cpu_device::RegWord()
{
	return m_regs.w[ m_Mod_RM.reg.w[ m_modrm ] ];
}


inline UINT16 i8086_common_cpu_device::GetRMWord()
{
	if ( m_modrm >= 0xc0 )
	{
		return m_regs.w[ m_Mod_RM.RM.w[ m_modrm ] ];
	}
	else
	{
		return read_word( get_ea() );
	}
}


inline UINT16 i8086_common_cpu_device::GetnextRMWord()
{
	UINT32 addr = ( m_ea & 0xf0000 ) | ( ( m_ea + 2 ) & 0xffff );

	return read_word( addr );
}


inline UINT8 i8086_common_cpu_device::GetRMByte()
{
	if ( m_modrm >= 0xc0 )
	{
		return m_regs.b[ m_Mod_RM.RM.b[ m_modrm ] ];
	}
	else
	{
		return read_byte( get_ea() );
	}
}


inline void i8086_common_cpu_device::PutMemB(int seg, UINT16 offset, UINT8 data)
{
	write_byte( default_base( seg ) + offset, data);
}


inline void i8086_common_cpu_device::PutMemW(int seg, UINT16 offset, UINT16 data)
{
	// if offset == 0xffff, 8086 writes to 0xffff and 0, 80186 writes to 0xffff and 0x10000
	write_word( default_base( seg ) + offset, data);
}


inline UINT8 i8086_common_cpu_device::GetMemB(int seg, UINT16 offset)
{
	return read_byte( default_base(seg) + offset );
}


inline UINT16 i8086_common_cpu_device::GetMemW(int seg, UINT16 offset)
{
	return read_word( default_base(seg) + offset );
}


// Setting flags

inline void i8086_common_cpu_device::set_CFB(UINT32 x)
{
	m_CarryVal = x & 0x100;
}

inline void i8086_common_cpu_device::set_CFW(UINT32 x)
{
	m_CarryVal = x & 0x10000;
}

inline void i8086_common_cpu_device::set_AF(UINT32 x,UINT32 y,UINT32 z)
{
	m_AuxVal = (x ^ (y ^ z)) & 0x10;
}

inline void i8086_common_cpu_device::set_SF(UINT32 x)
{
	m_SignVal = x;
}

inline void i8086_common_cpu_device::set_ZF(UINT32 x)
{
	m_ZeroVal = x;
}

inline void i8086_common_cpu_device::set_PF(UINT32 x)
{
	m_ParityVal = x;
}

inline void i8086_common_cpu_device::set_SZPF_Byte(UINT32 x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (INT8)x;
}

inline void i8086_common_cpu_device::set_SZPF_Word(UINT32 x)
{
	m_SignVal = m_ZeroVal = m_ParityVal = (INT16)x;
}

inline void i8086_common_cpu_device::set_OFW_Add(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x8000;
}

inline void i8086_common_cpu_device::set_OFB_Add(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (x ^ y) & (x ^ z) & 0x80;
}

inline void i8086_common_cpu_device::set_OFW_Sub(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x8000;
}

inline void i8086_common_cpu_device::set_OFB_Sub(UINT32 x,UINT32 y,UINT32 z)
{
	m_OverVal = (z ^ y) & (z ^ x) & 0x80;
}


inline UINT16 i8086_common_cpu_device::CompressFlags()
{
	return (CF ? 1 : 0)
		| (1 << 1)
		| (PF ? 4 : 0)
		| (AF ? 0x10 : 0)
		| (ZF ? 0x40 : 0)
		| (SF ? 0x80 : 0)
		| (m_TF << 8)
		| (m_IF << 9)
		| (m_DF << 10)
		| (OF << 11)
		| (0xf << 12);
}

inline void i8086_common_cpu_device::ExpandFlags(UINT16 f)
{
	m_CarryVal = (f) & 1;
	m_ParityVal = !((f) & 4);
	m_AuxVal = (f) & 16;
	m_ZeroVal = !((f) & 64);
	m_SignVal = (f) & 128 ? -1 : 0;
	m_TF = ((f) & 256) == 256;
	m_IF = ((f) & 512) == 512;
	m_DF = ((f) & 1024) == 1024;
	m_OverVal = (f) & 2048;
}

inline void i8086_common_cpu_device::i_insb()
{
	PutMemB( ES, m_regs.w[DI], read_port_byte( m_regs.w[DX] ) );
	m_regs.w[DI] += -2 * m_DF + 1;
	CLK(IN_IMM8);
}

inline void i8086_common_cpu_device::i_insw()
{
	PutMemW( ES, m_regs.w[DI], read_port_word( m_regs.w[DX] ) );
	m_regs.w[DI] += -4 * m_DF + 2;
	CLK(IN_IMM16);
}

inline void i8086_common_cpu_device::i_outsb()
{
	write_port_byte( m_regs.w[DX], GetMemB( DS, m_regs.w[SI] ) );
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(OUT_IMM8);
}

inline void i8086_common_cpu_device::i_outsw()
{
	write_port_word( m_regs.w[DX], GetMemW( DS, m_regs.w[SI] ) );
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(OUT_IMM16);
}

inline void i8086_common_cpu_device::i_movsb()
{
	UINT8 tmp = GetMemB( DS, m_regs.w[SI] );
	PutMemB( ES, m_regs.w[DI], tmp);
	m_regs.w[DI] += -2 * m_DF + 1;
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(MOVS8);
}

inline void i8086_common_cpu_device::i_movsw()
{
	UINT16 tmp = GetMemW( DS, m_regs.w[SI] );
	PutMemW( ES, m_regs.w[DI], tmp );
	m_regs.w[DI] += -4 * m_DF + 2;
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(MOVS16);
}

inline void i8086_common_cpu_device::i_cmpsb()
{
	m_src = GetMemB( ES, m_regs.w[DI] );
	m_dst = GetMemB( DS, m_regs.w[SI] );
	SUBB();
	m_regs.w[DI] += -2 * m_DF + 1;
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(CMPS8);
}

inline void i8086_common_cpu_device::i_cmpsw()
{
	m_src = GetMemW( ES, m_regs.w[DI] );
	m_dst = GetMemW( DS, m_regs.w[SI] );
	SUBX();
	m_regs.w[DI] += -4 * m_DF + 2;
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(CMPS16);
}

inline void i8086_common_cpu_device::i_stosb()
{
	PutMemB( ES, m_regs.w[DI], m_regs.b[AL] );
	m_regs.w[DI] += -2 * m_DF + 1;
	CLK(STOS8);
}

inline void i8086_common_cpu_device::i_stosw()
{
	PutMemW( ES, m_regs.w[DI], m_regs.w[AX] );
	m_regs.w[DI] += -4 * m_DF + 2;
	CLK(STOS16);
}

inline void i8086_common_cpu_device::i_lodsb()
{
	m_regs.b[AL] = GetMemB( DS, m_regs.w[SI] );
	m_regs.w[SI] += -2 * m_DF + 1;
	CLK(LODS8);
}

inline void i8086_common_cpu_device::i_lodsw()
{
	m_regs.w[AX] = GetMemW( DS, m_regs.w[SI] );
	m_regs.w[SI] += -4 * m_DF + 2;
	CLK(LODS16);
}

inline void i8086_common_cpu_device::i_scasb()
{
	m_src = GetMemB( ES, m_regs.w[DI] );
	m_dst = m_regs.b[AL];
	SUBB();
	m_regs.w[DI] += -2 * m_DF + 1;
	CLK(SCAS8);
}

inline void i8086_common_cpu_device::i_scasw()
{
	m_src = GetMemW( ES, m_regs.w[DI] );
	m_dst = m_regs.w[AX];
	SUBX();
	m_regs.w[DI] += -4 * m_DF + 2;
	CLK(SCAS16);
}


inline void i8086_common_cpu_device::i_popf()
{
	UINT32 tmp = POP();

	ExpandFlags(tmp);
	CLK(POPF);
	if (m_TF)
	{
		m_fire_trap = 1;
	}
}


inline void i8086_common_cpu_device::ADDB()
{
	UINT32 res = m_dst + m_src;

	set_CFB(res);
	set_OFB_Add(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Byte(res);
	m_dst = res & 0xff;
}


inline void i8086_common_cpu_device::ADDX()
{
	UINT32 res = m_dst + m_src;

	set_CFW(res);
	set_OFW_Add(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Word(res);
	m_dst = res & 0xffff;
}


inline void i8086_common_cpu_device::SUBB()
{
	UINT32 res = m_dst - m_src;

	set_CFB(res);
	set_OFB_Sub(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Byte(res);
	m_dst = res & 0xff;
}


inline void i8086_common_cpu_device::SUBX()
{
	UINT32 res = m_dst - m_src;

	set_CFW(res);
	set_OFW_Sub(res,m_src,m_dst);
	set_AF(res,m_src,m_dst);
	set_SZPF_Word(res);
	m_dst = res & 0xffff;
}


inline void i8086_common_cpu_device::ORB()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void i8086_common_cpu_device::ORW()
{
	m_dst |= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void i8086_common_cpu_device::ANDB()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void i8086_common_cpu_device::ANDX()
{
	m_dst &= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void i8086_common_cpu_device::XORB()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Byte(m_dst);
}


inline void i8086_common_cpu_device::XORW()
{
	m_dst ^= m_src;
	m_CarryVal = m_OverVal = m_AuxVal = 0;
	set_SZPF_Word(m_dst);
}


inline void i8086_common_cpu_device::ROL_BYTE()
{
	m_CarryVal = m_dst & 0x80;
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
}

inline void i8086_common_cpu_device::ROL_WORD()
{
	m_CarryVal = m_dst & 0x8000;
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
}

inline void i8086_common_cpu_device::ROR_BYTE()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) | (CF ? 0x80 : 0x00);
}

inline void i8086_common_cpu_device::ROR_WORD()
{
	m_CarryVal = m_dst & 0x1;
	m_dst = (m_dst >> 1) + (CF ? 0x8000 : 0x0000);
}

inline void i8086_common_cpu_device::ROLC_BYTE()
{
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
	set_CFB(m_dst);
}

inline void i8086_common_cpu_device::ROLC_WORD()
{
	m_dst = (m_dst << 1) | ( CF ? 1 : 0 );
	set_CFW(m_dst);
}

inline void i8086_common_cpu_device::RORC_BYTE()
{
	m_dst |= ( CF ? 0x100 : 0x00);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}

inline void i8086_common_cpu_device::RORC_WORD()
{
	m_dst |= ( CF ? 0x10000 : 0);
	m_CarryVal = m_dst & 0x01;
	m_dst >>= 1;
}

inline void i8086_common_cpu_device::SHL_BYTE(UINT8 c)
{
	m_dst <<= c;
	set_CFB(m_dst);
	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void i8086_common_cpu_device::SHL_WORD(UINT8 c)
{
	m_dst <<= c;
	set_CFW(m_dst);
	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}

inline void i8086_common_cpu_device::SHR_BYTE(UINT8 c)
{
	m_dst >>= c-1;
	m_CarryVal = m_dst & 0x1;
	m_dst >>= 1;
	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void i8086_common_cpu_device::SHR_WORD(UINT8 c)
{
	m_dst >>= c-1;
	m_CarryVal = m_dst & 0x1;
	m_dst >>= 1;
	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}

inline void i8086_common_cpu_device::SHRA_BYTE(UINT8 c)
{
	m_dst = ((INT8)m_dst) >> (c-1);
	m_CarryVal = m_dst & 0x1;
	m_dst = m_dst >> 1;
	set_SZPF_Byte(m_dst);
	PutbackRMByte(m_dst);
}

inline void i8086_common_cpu_device::SHRA_WORD(UINT8 c)
{
	m_dst = ((INT16)m_dst) >> (c-1);
	m_CarryVal = m_dst & 0x1;
	m_dst = m_dst >> 1;
	set_SZPF_Word(m_dst);
	PutbackRMWord(m_dst);
}


inline void i8086_common_cpu_device::XchgAXReg(UINT8 reg)
{
	UINT16 tmp = m_regs.w[reg];

	m_regs.w[reg] = m_regs.w[AX];
	m_regs.w[AX] = tmp;
}


inline void i8086_common_cpu_device::IncWordReg(UINT8 reg)
{
	UINT32 tmp = m_regs.w[reg];
	UINT32 tmp1 = tmp+1;

	m_OverVal = (tmp == 0x7fff);
	set_AF(tmp1,tmp,1);
	set_SZPF_Word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void i8086_common_cpu_device::DecWordReg(UINT8 reg)
{
	UINT32 tmp = m_regs.w[reg];
	UINT32 tmp1 = tmp-1;

	m_OverVal = (tmp == 0x8000);
	set_AF(tmp1,tmp,1);
	set_SZPF_Word(tmp1);
	m_regs.w[reg] = tmp1;
}


inline void i8086_common_cpu_device::PUSH(UINT16 data)
{
	m_regs.w[SP] -= 2;
	write_word( ( m_sregs[SS] << 4 ) + m_regs.w[SP], data );
}


inline UINT16 i8086_common_cpu_device::POP()
{
	UINT16 data = read_word( ( m_sregs[SS] << 4 ) + m_regs.w[SP] );

	m_regs.w[SP] += 2;
	return data;
}


inline void i8086_common_cpu_device::JMP(bool cond)
{
	int rel  = (int)((INT8)fetch());

	if (cond)
	{
		m_ip += rel;
		CLK(JCC_T);
	}
	else
		CLK(JCC_NT);
}


inline void i8086_common_cpu_device::ADJ4(INT8 param1,INT8 param2)
{
	if (AF || ((m_regs.b[AL] & 0xf) > 9))
	{
		UINT16 tmp;
		tmp = m_regs.b[AL] + param1;
		m_regs.b[AL] = tmp;
		m_AuxVal = 1;
		m_CarryVal |= tmp & 0x100;
	}
	if (CF || (m_regs.b[AL]>0x9f))
	{
		m_regs.b[AL] += param2;
		m_CarryVal = 1;
	}
	set_SZPF_Byte(m_regs.b[AL]);
}


inline void i8086_common_cpu_device::ADJB(INT8 param1, INT8 param2)
{
	if (AF || ((m_regs.b[AL] & 0xf) > 9))
	{
		m_regs.b[AL] += param1;
		m_regs.b[AH] += param2;
		m_AuxVal = 1;
		m_CarryVal = 1;
	}
	else
	{
		m_AuxVal = 0;
		m_CarryVal = 0;
	}
	m_regs.b[AL] &= 0x0F;
}


void i8086_common_cpu_device::interrupt(int int_num)
{
	PUSH( CompressFlags() );
	m_TF = m_IF = 0;

	if (int_num == -1)
	{
		int_num = standard_irq_callback(0);

		m_irq_state = CLEAR_LINE;
		m_pending_irq &= ~INT_IRQ;
	}

	UINT16 dest_off = read_word( int_num * 4 + 0 );
	UINT16 dest_seg = read_word( int_num * 4 + 2 );

	PUSH(m_sregs[CS]);
	PUSH(m_ip);
	m_ip = dest_off;
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
	else if(inptnum == INPUT_LINE_TEST)
		m_test_state = state;
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


offs_t i8086_common_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern int i386_dasm_one(char *buffer, offs_t eip, const UINT8 *oprom, int mode);
	return i386_dasm_one(buffer, pc, oprom, 1);
}


bool i8086_common_cpu_device::common_op(UINT8 op)
{
	switch(op)
	{
		case 0x00: // i_add_br8
			DEF_br8();
			ADDB();
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x01: // i_add_wr16
			DEF_wr16();
			ADDX();
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x02: // i_add_r8b
			DEF_r8b();
			ADDB();
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x03: // i_add_r16w
			DEF_r16w();
			ADDX();
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x04: // i_add_ald8
			DEF_ald8();
			ADDB();
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x05: // i_add_axd16
			DEF_axd16();
			ADDX();
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
			DEF_br8();
			m_src += CF ? 1 : 0;
			ADDB();
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x11: // i_adc_wr16
			DEF_wr16();
			m_src += CF ? 1 : 0;
			ADDX();
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x12: // i_adc_r8b
			DEF_r8b();
			m_src += CF ? 1 : 0;
			ADDB();
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x13: // i_adc_r16w
			DEF_r16w();
			m_src += CF ? 1 : 0;
			ADDX();
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x14: // i_adc_ald8
			DEF_ald8();
			m_src += CF ? 1 : 0;
			ADDB();
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x15: // i_adc_axd16
			DEF_axd16();
			m_src += CF ? 1 : 0;
			ADDX();
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
			DEF_br8();
			m_src += CF ? 1 : 0;
			SUBB();
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x19: // i_sbb_wr16
			DEF_wr16();
			m_src += CF ? 1 : 0;
			SUBX();
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x1a: // i_sbb_r8b
			DEF_r8b();
			m_src += CF ? 1 : 0;
			SUBB();
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x1b: // i_sbb_r16w
			DEF_r16w();
			m_src += CF ? 1 : 0;
			SUBX();
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x1c: // i_sbb_ald8
			DEF_ald8();
			m_src += CF ? 1 : 0;
			SUBB();
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x1d: // i_sbb_axd16
			DEF_axd16();
			m_src += CF ? 1 : 0;
			SUBX();
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
			m_prefix_base = m_sregs[ES]<<4;
			CLK(OVERRIDE);
			break;

		case 0x27: // i_daa
			ADJ4(6,0x60);
			CLK(DAA);
			break;


		case 0x28: // i_sub_br8
			DEF_br8();
			SUBB();
			PutbackRMByte(m_dst);
			CLKM(ALU_RR8,ALU_MR8);
			break;

		case 0x29: // i_sub_wr16
			DEF_wr16();
			SUBX();
			PutbackRMWord(m_dst);
			CLKM(ALU_RR16,ALU_MR16);
			break;

		case 0x2a: // i_sub_r8b
			DEF_r8b();
			SUBB();
			RegByte(m_dst);
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x2b: // i_sub_r16w
			DEF_r16w();
			SUBX();
			RegWord(m_dst);
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x2c: // i_sub_ald8
			DEF_ald8();
			SUBB();
			m_regs.b[AL] = m_dst;
			CLK(ALU_RI8);
			break;

		case 0x2d: // i_sub_axd16
			DEF_axd16();
			SUBX();
			m_regs.w[AX] = m_dst;
			CLK(ALU_RI16);
			break;

		case 0x2e: // i_cs
			m_seg_prefix_next = true;
			m_prefix_base = m_sregs[CS]<<4;
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
			m_prefix_base = m_sregs[SS]<<4;
			CLK(OVERRIDE);
			break;

		case 0x37: // i_aaa
			ADJB(6, (m_regs.b[AL] > 0xf9) ? 2 : 1);
			CLK(AAA);
			break;


		case 0x38: // i_cmp_br8
			DEF_br8();
			SUBB();
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x39: // i_cmp_wr16
			DEF_wr16();
			SUBX();
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x3a: // i_cmp_r8b
			DEF_r8b();
			SUBB();
			CLKM(ALU_RR8,ALU_RM8);
			break;

		case 0x3b: // i_cmp_r16w
			DEF_r16w();
			SUBX();
			CLKM(ALU_RR16,ALU_RM16);
			break;

		case 0x3c: // i_cmp_ald8
			DEF_ald8();
			SUBB();
			CLK(ALU_RI8);
			break;

		case 0x3d: // i_cmp_axd16
			DEF_axd16();
			SUBX();
			CLK(ALU_RI16);
			break;

		case 0x3e: // i_ds
			m_seg_prefix_next = true;
			m_prefix_base = m_sregs[DS]<<4;
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


		case 0x70: // i_jo
			JMP( OF);
			break;

		case 0x71: // i_jno
			JMP(!OF);
			break;

		case 0x72: // i_jc
			JMP( CF);
			break;

		case 0x73: // i_jnc
			JMP(!CF);
			break;

		case 0x74: // i_jz
			JMP( ZF);
			break;

		case 0x75: // i_jnz
			JMP(!ZF);
			break;

		case 0x76: // i_jce
			JMP(CF || ZF);
			break;

		case 0x77: // i_jnce
			JMP(!(CF || ZF));
			break;

		case 0x78: // i_js
			JMP( SF);
			break;

		case 0x79: // i_jns
			JMP(!SF);
			break;

		case 0x7a: // i_jp
			JMP( PF);
			break;

		case 0x7b: // i_jnp
			JMP(!PF);
			break;

		case 0x7c: // i_jl
			JMP((SF!=OF)&&(!ZF));
			break;

		case 0x7d: // i_jnl
			JMP((ZF)||(SF==OF));
			break;

		case 0x7e: // i_jle
			JMP((ZF)||(SF!=OF));
			break;

		case 0x7f: // i_jnle
			JMP((SF==OF)&&(!ZF));
			break;


		case 0x80: // i_80pre
			m_modrm = fetch();
			m_dst = GetRMByte();
			m_src = fetch();
			if (m_modrm >=0xc0 )             { CLK(ALU_RI8); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_MI8_RO); }
			else                             { CLK(ALU_MI8); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      ADDB(); PutbackRMByte(m_dst);   break;
			case 0x08:                      ORB();  PutbackRMByte(m_dst);   break;
			case 0x10: m_src += CF ? 1 : 0; ADDB(); PutbackRMByte(m_dst);   break;
			case 0x18: m_src += CF ? 1 : 0; SUBB(); PutbackRMByte(m_dst);   break;
			case 0x20:                      ANDB(); PutbackRMByte(m_dst);   break;
			case 0x28:                      SUBB(); PutbackRMByte(m_dst);   break;
			case 0x30:                      XORB(); PutbackRMByte(m_dst);   break;
			case 0x38:                      SUBB();                         break;  /* CMP */
			}
			break;


		case 0x81: // i_81pre
			m_modrm = fetch();
			m_dst = GetRMWord();
			m_src = fetch_word();
			if (m_modrm >=0xc0 )             { CLK(ALU_RI16); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_MI16_RO); }
			else                             { CLK(ALU_MI16); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      ADDX(); PutbackRMWord(m_dst);   break;
			case 0x08:                      ORW();  PutbackRMWord(m_dst);   break;
			case 0x10: m_src += CF ? 1 : 0; ADDX(); PutbackRMWord(m_dst);   break;
			case 0x18: m_src += CF ? 1 : 0; SUBX(); PutbackRMWord(m_dst);   break;
			case 0x20:                      ANDX(); PutbackRMWord(m_dst);   break;
			case 0x28:                      SUBX(); PutbackRMWord(m_dst);   break;
			case 0x30:                      XORW(); PutbackRMWord(m_dst);   break;
			case 0x38:                      SUBX();                         break;  /* CMP */
			}
			break;


		case 0x82: // i_82pre
			m_modrm = fetch();
			m_dst = GetRMByte();
			m_src = (INT8)fetch();
			if (m_modrm >=0xc0 )             { CLK(ALU_RI8); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_MI8_RO); }
			else                             { CLK(ALU_MI8); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      ADDB(); PutbackRMByte(m_dst);   break;
			case 0x08:                      ORB();  PutbackRMByte(m_dst);   break;
			case 0x10: m_src += CF ? 1 : 0; ADDB(); PutbackRMByte(m_dst);   break;
			case 0x18: m_src += CF ? 1 : 0; SUBB(); PutbackRMByte(m_dst);   break;
			case 0x20:                      ANDB(); PutbackRMByte(m_dst);   break;
			case 0x28:                      SUBB(); PutbackRMByte(m_dst);   break;
			case 0x30:                      XORB(); PutbackRMByte(m_dst);   break;
			case 0x38:                      SUBB();                         break; /* CMP */
			}
			break;


		case 0x83: // i_83pre
			m_modrm = fetch();
			m_dst = GetRMWord();
			m_src = (UINT16)((INT16)((INT8)fetch()));
			if (m_modrm >=0xc0 )             { CLK(ALU_R16I8); }
			else if ((m_modrm & 0x38)==0x38) { CLK(ALU_M16I8_RO); }
			else                             { CLK(ALU_M16I8); }
			switch (m_modrm & 0x38)
			{
			case 0x00:                      ADDX(); PutbackRMWord(m_dst); break;
			case 0x08:                      ORW();  PutbackRMWord(m_dst); break;
			case 0x10: m_src += CF ? 1 : 0; ADDX(); PutbackRMWord(m_dst); break;
			case 0x18: m_src += CF ? 1 : 0; SUBX(); PutbackRMWord(m_dst); break;
			case 0x20:                      ANDX(); PutbackRMWord(m_dst); break;
			case 0x28:                      SUBX(); PutbackRMWord(m_dst); break;
			case 0x30:                      XORW(); PutbackRMWord(m_dst); break;
			case 0x38:                      SUBX();                       break; /* CMP */
			}
			break;


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
			PutRMWord(m_sregs[(m_modrm & 0x38) >> 3]);
			CLKM(MOV_RS,MOV_MS);
			break;

		case 0x8d: // i_lea
			m_modrm = fetch();
			get_ea();
			RegWord(m_eo);
			CLK(LEA);
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
			case 0x08:  /* mov cs,ew */
				m_sregs[CS] = m_src;
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
			}
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
				UINT16 tmp = fetch_word();
				UINT16 tmp2 = fetch_word();
				PUSH(m_sregs[CS]);
				PUSH(m_ip);
				m_ip = tmp;
				m_sregs[CS] = tmp2;
				CLK(CALL_FAR);
			}
			break;

		case 0x9b: // i_wait
			if(m_test_state)
			{
				m_icount = 0;
				m_pc--;
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
				UINT32 tmp = (CompressFlags() & 0xff00) | (m_regs.b[AH] & 0xd5);
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
				UINT32 addr = fetch_word();
				m_regs.b[AL] = GetMemB(DS, addr);
				CLK(MOV_AM8);
			}
			break;

		case 0xa1: // i_mov_axdisp
			{
				UINT32 addr = fetch_word();
				m_regs.b[AL] = GetMemB(DS, addr);
				m_regs.b[AH] = GetMemB(DS, addr+1);
				CLK(MOV_AM16);
			}
			break;

		case 0xa2: // i_mov_dispal
			{
				UINT32 addr = fetch_word();
				PutMemB(DS, addr, m_regs.b[AL]);
				CLK(MOV_MA8);
			}
			break;

		case 0xa3: // i_mov_dispax
			{
				UINT32 addr = fetch_word();
				PutMemB(DS, addr, m_regs.b[AL]);
				PutMemB(DS, addr+1, m_regs.b[AH]);
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


		case 0xc2: // i_ret_d16
			{
				UINT32 count = fetch_word();
				m_ip = POP();
				m_regs.w[SP] += count;
				CLK(RET_NEAR_IMM);
			}
			break;

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


		case 0xca: // i_retf_d16
			{
				UINT32 count = fetch_word();
				m_ip = POP();
				m_sregs[CS] = POP();
				m_regs.w[SP] += count;
				CLK(RET_FAR_IMM);
			}
			break;

		case 0xcb: // i_retf
			m_ip = POP();
			m_sregs[CS] = POP();
			CLK(RET_FAR);
			break;

		case 0xcc: // i_int3
			interrupt(3);
			CLK(INT3);
			break;

		case 0xcd: // i_int
			interrupt(fetch());
			CLK(INT_IMM);
			break;

		case 0xce: // i_into
			if (OF)
			{
				interrupt(4);
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
			fetch();
			m_regs.b[AH] = m_regs.b[AL] / 10;
			m_regs.b[AL] %= 10;
			set_SZPF_Word(m_regs.w[AX]);
			CLK(AAM);
			break;

		case 0xd5: // i_aad
			fetch();
			m_regs.b[AL] = m_regs.b[AH] * 10 + m_regs.b[AL];
			m_regs.b[AH] = 0;
			set_SZPF_Byte(m_regs.b[AL]);
			CLK(AAD);
			break;

		case 0xd7: // i_trans
			m_regs.b[AL] = GetMemB( DS, m_regs.w[BX] + m_regs.b[AL] );
			CLK(XLAT);
			break;

		case 0xd8: // i_fpo
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:
			m_modrm = fetch();
			GetRMByte();
			CLK(NOP);
			logerror("%s: %06x: Unimplemented floating point escape %02x%02x\n", tag(), pc(), op, m_modrm);
			break;


		case 0xe0: // i_loopne
			{
				INT8 disp = (INT8)fetch();

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
				INT8 disp = (INT8)fetch();

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
				INT8 disp = (INT8)fetch();

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
				INT8 disp = (INT8)fetch();

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
			m_regs.b[AL] = read_port_byte( fetch() );
			CLK(IN_IMM8);
			break;

		case 0xe5: // i_inax
			{
				UINT8 port = fetch();

				m_regs.w[AX] = read_port_word(port);
				CLK(IN_IMM16);
			}
			break;

		case 0xe6: // i_outal
			write_port_byte( fetch(), m_regs.b[AL]);
			CLK(OUT_IMM8);
			break;

		case 0xe7: // i_outax
			{
				UINT8 port = fetch();

				write_port_word(port, m_regs.w[AX]);
				CLK(OUT_IMM16);
			}
			break;


		case 0xe8: // i_call_d16
			{
				INT16 tmp = (INT16)fetch_word();

				PUSH(m_ip);
				m_ip = m_ip + tmp;
				CLK(CALL_NEAR);
			}
			break;

		case 0xe9: // i_jmp_d16
			{
				INT16 offset = (INT16)fetch_word();
				m_ip += offset;
				CLK(JMP_NEAR);
			}
			break;

		case 0xea: // i_jmp_far
			{
				UINT16 tmp = fetch_word();
				UINT16 tmp1 = fetch_word();

				m_sregs[CS] = tmp1;
				m_ip = tmp;
				CLK(JMP_FAR);
			}
			break;

		case 0xeb: // i_jmp_d8
			{
				int tmp = (int)((INT8)fetch());

				CLK(JMP_SHORT);
				if (tmp==-2 && m_no_interrupt==0 && (m_pending_irq==0) && m_icount>0)
				{
					m_icount%=12; /* cycle skip */
				}
				m_ip = (UINT16)(m_ip+tmp);
			}
			break;

		case 0xec: // i_inaldx
			m_regs.b[AL] = read_port_byte(m_regs.w[DX]);
			CLK(IN_DX8);
			break;

		case 0xed: // i_inaxdx
			{
				UINT32 port = m_regs.w[DX];

				m_regs.w[AX] = read_port_word(port);
				CLK(IN_DX16);
			}
			break;

		case 0xee: // i_outdxal
			write_port_byte(m_regs.w[DX], m_regs.b[AL]);
			CLK(OUT_DX8);
			break;

		case 0xef: // i_outdxax
			{
				UINT32 port = m_regs.w[DX];

				write_port_word(port, m_regs.w[AX]);
				CLK(OUT_DX16);
			}
			break;


		case 0xf0: // i_lock
			logerror("%s: %06x: Warning - BUSLOCK\n", tag(), pc());
			m_no_interrupt = 1;
			CLK(NOP);
			break;

		case 0xf2: // i_repne
			{
				bool invalid = false;
				UINT8 next = repx_op();
				UINT16 c = m_regs.w[CX];

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
					logerror("%s: %06x: REPNE invalid\n", tag(), pc());
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
				UINT8 next = repx_op();
				UINT16 c = m_regs.w[CX];

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
					logerror("%s: %06x: REPE invalid\n", tag(), pc());
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
			logerror("%s: %06x: HALT\n", tag(), pc());
			m_icount = 0;
			break;

		case 0xf5: // i_cmc
			m_CarryVal = !m_CarryVal;
			CLK(FLAG_OPS);
			break;

		case 0xf6: // i_f6pre
			{
				UINT32 tmp;
				UINT32 uresult,uresult2;
				INT32 result,result2;

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
					m_CarryVal = (tmp!=0) ? 1 : 0;
					tmp = (~tmp)+1;
					set_SZPF_Byte(tmp);
					PutbackRMByte(tmp&0xff);
					CLKM(NEGNOT_R8,NEGNOT_M8);
					break;
				case 0x20:  /* MUL */
					uresult = m_regs.b[AL] * tmp;
					m_regs.w[AX] = (UINT16)uresult;
					m_CarryVal = m_OverVal = (m_regs.b[AH]!=0) ? 1 : 0;
					set_ZF(m_regs.w[AX]);
					CLKM(MUL_R8,MUL_M8);
					break;
				case 0x28:  /* IMUL */
					result = (INT16)((INT8)m_regs.b[AL])*(INT16)((INT8)tmp);
					m_regs.w[AX] = (UINT16)result;
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
						result = (INT16)m_regs.w[AX];
						result2 = result % (INT16)((INT8)tmp);
						if ((result /= (INT16)((INT8)tmp)) > 0xff)
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
				UINT32 tmp,tmp2;
				UINT32 uresult,uresult2;
				INT32 result,result2;

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
					break;
				case 0x10:  /* NOT */
					PutbackRMWord(~tmp);
					CLKM(NEGNOT_R16,NEGNOT_M16);
					break;
				case 0x18:  /* NEG */
					m_CarryVal = (tmp!=0) ? 1 : 0;
					tmp = (~tmp) + 1;
					set_SZPF_Word(tmp);
					PutbackRMWord(tmp);
					CLKM(NEGNOT_R16,NEGNOT_M16);
					break;
				case 0x20:  /* MUL */
					uresult = m_regs.w[AX]*tmp;
					m_regs.w[AX] = uresult & 0xffff;
					m_regs.w[DX] = ((UINT32)uresult)>>16;
					m_CarryVal = m_OverVal = (m_regs.w[DX] != 0) ? 1 : 0;
					set_ZF(m_regs.w[AX] | m_regs.w[DX]);
					CLKM(MUL_R16,MUL_M16);
					break;
				case 0x28:  /* IMUL */
					result = (INT32)((INT16)m_regs.w[AX]) * (INT32)((INT16)tmp);
					m_regs.w[AX] = result & 0xffff;
					m_regs.w[DX] = result >> 16;
					m_CarryVal = m_OverVal = (m_regs.w[DX] != 0) ? 1 : 0;
					set_ZF(m_regs.w[AX] | m_regs.w[DX]);
					CLKM(IMUL_R16,IMUL_M16);
					break;
				case 0x30:  /* DIV */
					if (tmp)
					{
						uresult = (((UINT32)m_regs.w[DX]) << 16) | m_regs.w[AX];
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
						result = ((UINT32)m_regs.w[DX] << 16) + m_regs.w[AX];
						result2 = result % (INT32)((INT16)tmp);
						if ((result /= (INT32)((INT16)tmp)) > 0xffff)
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
				UINT32 tmp, tmp1;
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
					logerror("%s: %06x: FE Pre with unimplemented mod\n", tag(), pc());
					break;
				}
			}
			break;

		case 0xff: // i_ffpre
			{
				UINT32 tmp, tmp1;
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
					logerror("%s: %06x: FF Pre with unimplemented mod\n", tag(), pc());
					break;
				}
			}
			break;
		default:
			return false;
	}
	return true;
}
