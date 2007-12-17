/********************************************************************
 SSP1610 cpu emulator
 written by Pierpaolo Prazzoli

 Very very preliminary. Still working on it...

*********************************************************************/

#include "debugger.h"
#include "ssp1610.h"
#include "osd_cpu.h"

#define READ_OP(addr)	       (cpu_readop16(addr<<1))

static int ssp1610_ICount;

/* Registers */

enum
{
	SSP1610_REG0 = 1, SSP1610_X, SSP1610_Y, SSP1610_A, SSP1610_ST, SSP1610_STACK, SSP1610_PC,
	SSP1610_P, SSP1610_AL, SSP1610_XST, SSP1610_PL, SSP1610_SRCR, SSP1610_BRCR, SSP1610_BRER,
	SSP1610_XRD0, SSP1610_XRD1, SSP1610_AE, SSP1610_DIOR, SSP1610_GR22, SSP1610_GR23,
	SSP1610_EXT0, SSP1610_EXT1, SSP1610_EXT2, SSP1610_EXT3, SSP1610_EXT4, SSP1610_EXT5, SSP1610_EXT6, SSP1610_EXT7
};

/* Internal registers */
typedef struct
{
	UINT32 regs[32]; // some are 8bits, some 16bits and some 32bits

	UINT16 ppc;

} ssp1610_regs;

static ssp1610_regs ssp1610;

// number of bits of each register
static const UINT8 regs_width[] =
{
	16, 16, 16, 32, 16, 16, 16, 32, 16,  0,  0, 16, 16,  0,  0,  8,
	 8, 16,	 8,  8,  8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

// REGISTERS

#define PPC				ssp1610.ppc

#define REG0			ssp1610.regs[0]
#define X				ssp1610.regs[1]
#define Y				ssp1610.regs[2]
#define A				ssp1610.regs[3]
#define ST				ssp1610.regs[4]
//swapped stack and pc (docs wrong...)
#define STACK			ssp1610.regs[5]
#define PC				ssp1610.regs[6]
#define P				ssp1610.regs[7]
#define AL				ssp1610.regs[8]
// 9
// 10
#define XST				ssp1610.regs[11]
#define PL				ssp1610.regs[12]
// 13
// 14
#define SRCR			ssp1610.regs[15]
#define BRCR			ssp1610.regs[16]
#define BRER			ssp1610.regs[17]
#define XRD0			ssp1610.regs[18]
#define XRD1			ssp1610.regs[19]
#define AE				ssp1610.regs[20]
#define DIOR			ssp1610.regs[21]
#define GR22			ssp1610.regs[22]
#define GR23			ssp1610.regs[23]
#define EXT0			ssp1610.regs[24]
#define EXT1			ssp1610.regs[25]
#define EXT2			ssp1610.regs[26]
#define EXT3			ssp1610.regs[27]
#define EXT4			ssp1610.regs[28]
#define EXT5			ssp1610.regs[29]
#define EXT6			ssp1610.regs[30]
#define EXT7			ssp1610.regs[31]

// CONDITIONS

#define IS_PC			6

// CONTROL bits


#define RPL				((ST >> 0) & 7)
#define RB				((ST >> 3) & 3)
#define GP0_0			((ST >> 5) & 1)
#define GP0_1			((ST >> 6) & 1)
#define IE				((ST >> 7) & 1)
#define OP				((ST >> 8) & 1)
#define MACS			((ST >> 9) & 1)


// FLAG bits


#define GPI_0			((ST >> 10) & 1)
#define GPI_1			((ST >> 11) & 1)


#define L				((ST >> 12) & 1)
#define Z				((ST >> 13) & 1)
#define OV				((ST >> 14) & 1)
#define N				((ST >> 15) & 1)

#define SET_L(value)	ST = (ST & ~(1 << 12)) | ((value) << 12)
#define SET_Z(value)	ST = (ST & ~(1 << 13)) | ((value) << 13)
#define SET_OV(value)	ST = (ST & ~(1 << 14)) | ((value) << 14)
#define SET_N(value)	ST = (ST & ~(1 << 15)) | ((value) << 15)

#define SET_OVSUB(x,y,z)(ST = (ST & ~(1 << 14)) | ((((z) ^ (y)) & ((y) ^ (x)) & 0x80000000) ? (1 << 14): 0))

static void ssp1610_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	REG0 = 0xffff; // read-only register
}

static void ssp1610_reset(void)
{
	ST = 0;

	// latch it from 0 but it's selectable (hard-coded for now)
	PC = 0x400;
}

static void ssp1610_exit(void)
{
	// nothing to do
}

static int ssp1610_execute(int cycles)
{
	ssp1610_ICount = cycles;

	do
	{
		UINT16 op;

		PPC = PC;	/* copy PC to previous PC */

		CALL_MAME_DEBUG;

		op = READ_OP(PC);

		PC++;

		switch((op >> 9))
		{
			case 0x00:

				if(op == 0) // right?
				{
					// nop

				}
				else if((op & 0xff) == 0x65)
				{
					// ret

				}
				else
				{
					// ld d, s

					int s = op & 0xf;
					int d = (op >> 4) & 0xf;

					// TODO: handle special cases... and complete ALU operations

					if(regs_width[d] == 16 && (regs_width[s] == 16 || regs_width[s] == 8))
					{
						ssp1610.regs[d] = ssp1610.regs[s];
					}
					else if(regs_width[d] == 16 && regs_width[s] == 32)
					{
						ssp1610.regs[d] = (ssp1610.regs[s] >> 16);
					}
					else if(regs_width[d] == 32 && (regs_width[s] == 16 || regs_width[s] == 8))
					{
						ssp1610.regs[d] = (ssp1610.regs[s] << 16) | (ssp1610.regs[d] & 0xffff);
					}
					else if(regs_width[d] == 32 && regs_width[s] == 32)
					{
						// wrong! if P is s then only the top 16bits are transferred
						ssp1610.regs[d] = (ssp1610.regs[s] & 0xffff0000) | (ssp1610.regs[d] & 0xffff);
					}

					if(d == IS_PC) // if d is PC one more cycle
					{
						ssp1610_ICount--;
						change_pc(PC);
					}

				}

				break;

			// ld d, (ri) -> probably right
			case 0x01: // or ldi simm ?

				break;

			// ld (ri), s
			case 0x02:

				break;

			// ld a, addr
			case 0x03:

				break;

			// ldi d, imm
			case 0x04:


				break;

			// ld d, ((ri))
			case 0x05:

				break;

			// ldi (ri), imm
			case 0x06:

				break;

			// ld addr, a -> almost sure it's this one. some of the 1st 4 bits are set in the code
			case 0x07: // or ld d, (a) ?

				break;

			// ld d, ri
			case 0x09:

				break;

			// ld ri, s
			case 0x0a:

				break;

			// mpys (rj), (ri), b
			case 0x0b:

				break;

			// ldi ri, simm
			case 0x0c:
			case 0x0d:
			case 0x0e:
			case 0x0f:


				break;

			// sub a, s
			case 0x10:

				break;

			// sub a, (ri)
			case 0x11:

				break;

			// mpys s, (ri), b
			case 0x12:

				break;

			// sub a, adr
			case 0x13:

				break;

			// subi a, imm
			case 0x14:

				break;

			// sub a, ((ri))
			case 0x15:

				break;

			// sub a, ri
			case 0x19:

				break;

			// mset reg, (ri), s
			case 0x1a:
			case 0x1b:

				break;

			// subi simm
			case 0x1c:

				break;

			// sub a, erij
			case 0x23:

				break;

			// call cond, addr
			case 0x24: //f bit?

				break;

			// bra cond, addr
			case 0x26: //f bit?
				{
					UINT16 new_pc = READ_OP(PC);
					UINT8 f = (op >> 8) & 1;
					UINT8 cond = 0;
					PC++;

					switch((op >> 4) & 0xf)
					{
					case 0x0:
					case 0x1:
						cond = f; // to jump always
						break;

					case 0x2:
					case 0x3:
						// reserved
						break;

					case 0x4:
						cond = GPI_0 == 0;
						break;

					case 0x5:
						cond = GPI_0 == 1;
						break;

					case 0x6:
						cond = GPI_1 == 0;
						break;

					case 0x7:
						cond = GPI_1 == 1;
						break;

					case 0x8:
						cond = L == 0;
						break;

					case 0x9:
						cond = L == 1;
						break;

					case 0xa:
						cond = Z == 0;
						break;

					case 0xb:
						cond = Z == 1;
						break;

					case 0xc:
						cond = OV == 0;
						break;

					case 0xd:
						cond = OV == 1;
						break;

					case 0xe:
						cond = N == 0;
						break;

					case 0xf:
						cond = N == 1;
						break;
					}

					if((cond && f) || (!cond && !f))
					{
						PPC = PC;
						PC = new_pc;
						change_pc(PC);
					}

					ssp1610_ICount--;
				}

				break;


			case 0x28:

				if(op & 0x100)
				{
					// slow imm

				}
				else
				{
					// stop

				}

				break;

			// cmp a, s
			case 0x30:

				break;

			// cmp a, (ri)
			case 0x31:

				break;

			// cmp a, adr
			case 0x33:

				break;

			// cmpi a, imm
			case 0x34:
				{
					UINT16 imm = READ_OP(PC) << 16;
#if 0
					UINT64 tmp = (UINT64)A - (UINT64)imm;
#else
					// The previous line causes a bus error in Apple's GCC as of 10.4.9:
					// i686-apple-darwin8-gcc-4.0.1 (GCC) 4.0.1 (Apple Computer, Inc. build 5367)
					// The gcc included with the Leopard Beta fixes this error.
					// Using the volatile tempA works around the bus error.
					volatile UINT32 tempA = A;
					UINT64 tmp = (UINT64)tempA - (UINT64)imm;
#endif
					PC++;

					// right flags?

					SET_L(A < imm);
					SET_Z((A & 0xffff0000) == imm);
					SET_N(((INT32) A) < ((INT32)imm));
					SET_OVSUB(imm,A,tmp);

					// A = saturated value
					if(OP && OV)
					{
						if((INT64)tmp > 0x7fffff)
							A = 0x7fffff;
						else if((INT64)tmp < -0x800000)
							A = 0x800000;
					}
				}
				break;

			// cmp a, ((ri))
			case 0x35:

				break;

			// cmp a, ri
			case 0x39:

				break;

			// cmpi simm
			case 0x3c:

				break;

			// add a, s
			case 0x40:

				break;

			// add a, (ri)
			case 0x41:

				break;

			// mpya s, (ri), b
			case 0x42:

				break;

			// add a, adr
			case 0x43:

				break;

			// addi a, imm
			case 0x44:


				break;

			// add a, ((ri))
			case 0x45:

				break;

			// mod cond, op
			case 0x48:

				break;

			// add a, ri
			case 0x49:

				break;

			// mod f, op
			case 0x4a:

				break;

			// mpya (rj), (ri), b
			case 0x4b:

				break;

			// addi simm
			case 0x4c: // docs show a wrong opcode value

				break;

			// ld reg, erij
			case 0x4e:
			case 0x4f:

				break;

			// and a, s
			case 0x50:

				break;

			// and a, (ri)
			case 0x51:

				break;

			// mld s, (ri), b
			case 0x52:
				break;

			// and a, adr
			case 0x53:

				break;

			// andi a, imm
			case 0x54:


				break;

			// and a, ((ri))
			case 0x55:

				break;

			// and a, ri
			case 0x59:

				break;

			// mld (rj), (ri), b
			case 0x5b:

				break;

			// andi simm
			case 0x5c:
				A = (A & ~0x00ff0000) | (A & ((op & 0xff) << 16));
//              SET_L();
				SET_Z(A == 0);
				SET_N(A >> 31);
				SET_OV(0);
				break;

			// ld erij, reg
			case 0x5e:
			case 0x5f:

				break;

			// or a, s
			case 0x60:

				break;

			// or a, (ri)
			case 0x61:

				break;

			// or a, adr
			case 0x63:

				break;

			// ori a, imm
			case 0x64:

				break;

			// or a, ((ri))
			case 0x65:

				break;

			// or a, ri
			case 0x69:

				break;

			// ori simm
			case 0x6c:

				break;

			// eor a, s
			case 0x70:

				break;

			// eor a, (ri)
			case 0x71:

				break;

			// eor a, adr
			case 0x73:

				break;

			// eori a, imm
			case 0x74:

				break;

			// eor a, ((ri))
			case 0x75:

				break;

			// eor a, ri
			case 0x79:

				break;

			// eori simm
			case 0x7c:

				break;

			default:

				break;
		}

		ssp1610_ICount--;

	} while( ssp1610_ICount > 0 );

	return cycles - ssp1610_ICount;
}

static void ssp1610_get_context(void *regs)
{
	/* copy the context */
	if( regs )
		*(ssp1610_regs *)regs = ssp1610;
}

static void ssp1610_set_context(void *regs)
{
	/* copy the context */
	if (regs)
		ssp1610 = *(ssp1610_regs *)regs;
}

#ifdef MAME_DEBUG
static offs_t ssp1610_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return dasm_ssp1610( buffer, pc, oprom);
}
#endif /* MAME_DEBUG */

#if (HAS_SSP1610)

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void ssp1610_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */

//      case CPUINFO_INT_REGISTER + SSP1610_REG0:       REG0 = info->i;                         break;
		case CPUINFO_INT_REGISTER + SSP1610_X:			X = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_Y:			Y = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_ST:			ST = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_STACK:		STACK = info->i;						break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SSP1610_PC:			PC = info->i; change_pc(PC);			break;
		case CPUINFO_INT_REGISTER + SSP1610_P:			P = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_AL:			AL = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_XST:		XST = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_PL:			PL = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_SRCR:		SRCR = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_BRCR:		BRCR = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_BRER:		BRER = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_XRD0:		XRD0 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_XRD1:		XRD1 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_AE:			AE = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_DIOR:		DIOR = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_GR22:		GR22 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_GR23:		GR23 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT0:		EXT0 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT1:		EXT1 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT2:		EXT2 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT3:		EXT3 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT4:		EXT4 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT5:		EXT5 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT6:		EXT6 = info->i;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT7:		EXT7 = info->i;							break;

//      case CPUINFO_INT_INPUT_STATE + 0:               set_irq_line(0, info->i);               break;
//      case CPUINFO_INT_INPUT_STATE + 1:               set_irq_line(1, info->i);               break;
//      case CPUINFO_INT_INPUT_STATE + 2:               set_irq_line(2, info->i);               break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void ssp1610_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(ssp1610_regs);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					/* not implemented */				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_REGISTER + SSP1610_REG0:		info->i = REG0;							break;
		case CPUINFO_INT_REGISTER + SSP1610_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + SSP1610_Y:			info->i = Y;							break;
		case CPUINFO_INT_REGISTER + SSP1610_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + SSP1610_ST:			info->i = ST;							break;
		case CPUINFO_INT_REGISTER + SSP1610_STACK:		info->i = STACK;						break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SSP1610_PC:			info->i = PC; change_pc(PC);			break;
		case CPUINFO_INT_REGISTER + SSP1610_P:			info->i = P;							break;
		case CPUINFO_INT_REGISTER + SSP1610_AL:			info->i = AL;							break;
		case CPUINFO_INT_REGISTER + SSP1610_XST:		info->i = XST;							break;
		case CPUINFO_INT_REGISTER + SSP1610_PL:			info->i = PL;							break;
		case CPUINFO_INT_REGISTER + SSP1610_SRCR:		info->i = SRCR;							break;
		case CPUINFO_INT_REGISTER + SSP1610_BRCR:		info->i = BRCR;							break;
		case CPUINFO_INT_REGISTER + SSP1610_BRER:		info->i = BRER;							break;
		case CPUINFO_INT_REGISTER + SSP1610_XRD0:		info->i = XRD0;							break;
		case CPUINFO_INT_REGISTER + SSP1610_XRD1:		info->i = XRD1;							break;
		case CPUINFO_INT_REGISTER + SSP1610_AE:			info->i = AE;							break;
		case CPUINFO_INT_REGISTER + SSP1610_DIOR:		info->i = DIOR;							break;
		case CPUINFO_INT_REGISTER + SSP1610_GR22:		info->i = GR22;							break;
		case CPUINFO_INT_REGISTER + SSP1610_GR23:		info->i = GR23;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT0:		info->i = EXT0;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT1:		info->i = EXT1;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT2:		info->i = EXT2;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT3:		info->i = EXT3;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT4:		info->i = EXT4;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT5:		info->i = EXT5;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT6:		info->i = EXT6;							break;
		case CPUINFO_INT_REGISTER + SSP1610_EXT7:		info->i = EXT7;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = ssp1610_set_info;	break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = ssp1610_get_context; break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = ssp1610_set_context; break;
		case CPUINFO_PTR_INIT:							info->init = ssp1610_init;			break;
		case CPUINFO_PTR_RESET:							info->reset = ssp1610_reset;			break;
		case CPUINFO_PTR_EXIT:							info->exit = ssp1610_exit;			break;
		case CPUINFO_PTR_EXECUTE:						info->execute = ssp1610_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = ssp1610_dasm;	break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &ssp1610_ICount;		break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map = 0;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_IO:      info->internal_map = 0;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "SSP1610 CPU");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Pierpaolo Prazzoli"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "_flags_");
			break;

		case CPUINFO_STR_REGISTER + SSP1610_REG0:		sprintf(info->s, "REG0  :%04X", REG0);							break;
		case CPUINFO_STR_REGISTER + SSP1610_X:			sprintf(info->s, "X  :%04X", X);							break;
		case CPUINFO_STR_REGISTER + SSP1610_Y:			sprintf(info->s, "Y  :%04X", Y);							break;
		case CPUINFO_STR_REGISTER + SSP1610_A:			sprintf(info->s, "A  :%08X", A);							break;
		case CPUINFO_STR_REGISTER + SSP1610_ST:			sprintf(info->s, "ST  :%04X", ST);							break;
		case CPUINFO_STR_REGISTER + SSP1610_STACK:		sprintf(info->s, "STACK  :%04X", STACK);						break;
		case CPUINFO_STR_REGISTER + SSP1610_PC:  		sprintf(info->s, "PC  :%04X", PC);		break;
		case CPUINFO_STR_REGISTER + SSP1610_P:			sprintf(info->s, "P  :%08X", P);							break;
		case CPUINFO_STR_REGISTER + SSP1610_AL:			sprintf(info->s, "AL  :%04X", AL);							break;
		case CPUINFO_STR_REGISTER + SSP1610_XST:		sprintf(info->s, "XST  :%04X", XST);							break;
		case CPUINFO_STR_REGISTER + SSP1610_PL:			sprintf(info->s, "PL  :%04X", PL);							break;
		case CPUINFO_STR_REGISTER + SSP1610_SRCR:		sprintf(info->s, "SRCR  :%02X", SRCR);							break;
		case CPUINFO_STR_REGISTER + SSP1610_BRCR:		sprintf(info->s, "BRCR  :%02X", BRCR);							break;
		case CPUINFO_STR_REGISTER + SSP1610_BRER:		sprintf(info->s, "BRER  :%04X", BRER);							break;
		case CPUINFO_STR_REGISTER + SSP1610_XRD0:		sprintf(info->s, "XRD0  :%02X", XRD0);							break;
		case CPUINFO_STR_REGISTER + SSP1610_XRD1:		sprintf(info->s, "XRD1  :%02X", XRD1);							break;
		case CPUINFO_STR_REGISTER + SSP1610_AE:			sprintf(info->s, "AE  :%02X", AE);							break;
		case CPUINFO_STR_REGISTER + SSP1610_DIOR:		sprintf(info->s, "DIOR :%04X", DIOR);							break;
		case CPUINFO_STR_REGISTER + SSP1610_GR22:		sprintf(info->s, "GR22  :%04X", GR22);							break;
		case CPUINFO_STR_REGISTER + SSP1610_GR23:		sprintf(info->s, "GR23  :%04X", GR23);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT0:		sprintf(info->s, "EXT0  :%04X", EXT0);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT1:		sprintf(info->s, "EXT1  :%04X", EXT1);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT2:		sprintf(info->s, "EXT2  :%04X", EXT2);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT3:		sprintf(info->s, "EXT3  :%04X", EXT3);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT4:		sprintf(info->s, "EXT4  :%04X", EXT4);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT5:		sprintf(info->s, "EXT5  :%04X", EXT5);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT6:		sprintf(info->s, "EXT6  :%04X", EXT6);							break;
		case CPUINFO_STR_REGISTER + SSP1610_EXT7:		sprintf(info->s, "EXT7  :%04X", EXT7);							break;

	}
}
#endif
