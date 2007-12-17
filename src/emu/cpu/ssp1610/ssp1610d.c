/*

 SSP1610 disassembler
 written by Pierpaolo Prazzoli

*/

#include <math.h>
#include "debugger.h"
#include "cpuintrf.h"


static const char* reg[] =
{
	"-", "X", "Y", "A", "ST", "STACK", "PC", "P", "AL", "?", "?", "XST", "PL", "?", "?", "SRCR", "BRCR", "BRER",
	"XRD0", "XRD1", "AE", "DIOR", "GR22", "GR23", "EXT0", "EXT1", "EXT2", "EXT3", "EXT4", "EXT5", "EXT6", "EXT7"
};

static const char* rij[] =
{
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

static const char* erij[] =
{
	"er0", "er1", "er2", "er3", "er4", "er5", "er6", "er7"
};

static const char* modifier[] =
{
	"", "+!", "-", "+"
};

static const char* cond[] =
{
	"always", "always", "RESERVED", "RESERVED", "gpi0=0", "gpi0=1", "gpi1=0", "gpi1=1", "l=0", "l=1", "z=0", "z=1", "ov=0", "ov=1", "n=0", "n=1",
    "diof=0", "diof=1", "gpi2=0", "gpi2=1", "gpi3=0", "gpi3=1", "RESERVED", "RESERVED", "RESERVED", "RESERVED", "RESERVED", "RESERVED", "RESERVED", "RESERVED", "RESERVED", "RESERVED"
};

static const char* acc_op[] =
{
	"ror", "rol", "shr", "shl", "inc", "dec", "neg", "abs"
};

// pag. 81 uses different addresses!
static const char* flag_op[] =
{
	"?", "?", "resl", "setl", "resie", "setie", "?", "?", "resop", "setop", "?", "?", "?", "?", "res", "set"
};


static const char* st_mod[] =
{
	"macs", "rpl", "gpo", "rb", "gpo0", "gpo1", "gpo2", "gpo3", "wte", "diof", "bre", "reserved", "reserved", "reserved", "reserved", "reserved"
};


#define BIT_B			((op >> 8) & 1)
#define RIJ				rij[(BIT_B << 2) + (op & 3)]
#define RI(i)			rij[(i) & 3]
#define RJ(i)			rij[((i) & 3) + 4]
#define MODIFIER(i)		modifier[(i) & 3]
#define MODIFIER_LOW	MODIFIER(op >> 2)
#define MODIFIER_HIGH	MODIFIER(op >> 6)
#define COND			cond[(op >> 4) & 0xf]

static offs_t base_pc;
static const UINT8 *base_oprom;
#define READ_OP_DASM(p)	((base_oprom[(p) - base_pc] << 8) | base_oprom[(p) + 1 - base_pc])

//TODO:
// - add special Short-form Direct Addressing for r3 and r7
// - support DASM flags

unsigned dasm_ssp1610(char *buffer, unsigned pc, const UINT8 *oprom)
{
	UINT16 op;
	UINT8 op_num;
	int size = 1;
	int flags = 0;

	base_pc = pc;
	base_oprom = oprom;

	op = READ_OP_DASM(pc);
	op_num = op >> 9;

	// .....overlapping opcodes....
	//
	// ldi simm ?
	// ld d, (a) ?
	// mset reg, (ri), b ? overlaps other opcodes
	// mset (rj), (ri), b ?
	// repb simm/srcr, imm ?
	// reps simm/srcr ?
	// subi (rij)/erij/reg,imm,t ?
	/*
        // mod x, nimm, n
        case 0x11:
            sprintf(buffer, "mod x, %X, %d", op & 0xf, (op >> 4) & 1);
            break;

        // mset (rj), (ri), ss
        case 0x18, 0x19: // ss order?
            sprintf(buffer, "mset (%s%s), (%s%s), %d%d", RJ(op >> 5), MODIFIER(op >> 7), RI(op), MODIFIER_LOW, (op >> 9) & 1, (op >> 4) & 1);
            break;

        // mod st, bs, nimm
        case 0x28: //? docs say 0x24
            sprintf(buffer, "mod st, %s, %x", st_mod[(op >> 5) & 0xf], op & 0xf);
            break;
    */

	switch( op_num )
	{

		//????????????????????

		// ldi simm
//      case 0x08:
//          sprintf(buffer, "ldi %X", op & 0xff);
//          break;

		// maybe
		// mod st, bs, nimm
		case 0x25: //? docs say 0x24
			sprintf(buffer, "mod st, %s, %x", st_mod[(op >> 5) & 0xf], op & 0xf);
			break;

		//????????????????????

		case 0x00:

			if(op == 0) // right?
			{
				// nop
				sprintf(buffer, "nop");
			}
			else if((op & 0xff) == 0x65)
			{
				// ret
				sprintf(buffer, "ret");
			}
			else
			{
				// ld d, s
				sprintf(buffer, "ld %s, %s", reg[(op >> 4) & 0xf], reg[0xf]);
			}

			break;

		// ld d, (ri) -> probably right
		case 0x01: // or ldi simm ?
			sprintf(buffer, "ld %s, (%s%s)", reg[(op >> 4) & 0xf], RIJ, MODIFIER_LOW);
			break;

		// ld (ri), s
		case 0x02:
			sprintf(buffer, "ld (%s%s), %s", RIJ, MODIFIER_LOW, reg[(op >> 4) & 0xf]);
			break;

		// ld a, addr
		case 0x03:
			sprintf(buffer, "ld a, %X", op & 0x1ff);
			break;

		// ldi d, imm
		case 0x04:
			sprintf(buffer, "ld %s, %X", reg[(op >> 4) & 0xf], READ_OP_DASM(pc+2));
			size = 2;
			break;

		// ld d, ((ri))
		case 0x05:
			sprintf(buffer, "ld %s, ((%s%s))", reg[(op >> 4) & 0xf], RIJ, MODIFIER_LOW);
			break;

		// ldi (ri), imm
		case 0x06:
			sprintf(buffer, "ld (%s%s), %X", RIJ, MODIFIER_LOW, READ_OP_DASM(pc+2));
			size = 2;
			break;

		// ld addr, a -> almost sure it's this one. some of the 1st 4 bits are set in the code
		case 0x07: // or ld d, (a) ?
			sprintf(buffer, "ld %X, a", op & 0x1ff);
			break;

		// ld d, ri
		case 0x09:
			sprintf(buffer, "ld %s, %s%s", reg[(op >> 4) & 0xf], RIJ, MODIFIER_LOW);
			break;

		// ld ri, s
		case 0x0a:
			sprintf(buffer, "ld %s%s, %s", RIJ, MODIFIER_LOW, reg[(op >> 4) & 0xf]);
			break;

		// mpys (rj), (ri), b
		case 0x0b:
			sprintf(buffer, "mpya (%s%s), (%s%s), %d", RJ(op >> 4), MODIFIER_HIGH, RI(op), MODIFIER_LOW, BIT_B);
			break;

		// ldi ri, simm
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			sprintf(buffer, "ldi %s, %X", rij[(op >> 7) & 7], op & 0xff);
			break;

		// sub a, s
		case 0x10:
			sprintf(buffer, "sub a, %s", reg[op & 0xf]);
			break;

		// sub a, (ri)
		case 0x11:
			sprintf(buffer, "sub a, (%s%s)", RIJ, MODIFIER_LOW);
			break;

		// mpys s, (ri), b
		case 0x12:
			sprintf(buffer, "mpya %s, (%s%s), %d", reg[(op >> 4) & 0xf], RI(op), MODIFIER_LOW, BIT_B);
			break;

		// sub a, adr
		case 0x13:
			sprintf(buffer, "sub a, %X", op & 0xff);
			break;

		// subi a, imm
		case 0x14:
			sprintf(buffer, "subi a, %X", READ_OP_DASM(pc+2));
			size = 2;
			break;

		// sub a, ((ri))
		case 0x15:
			sprintf(buffer, "sub a, ((%s%s))", RIJ, MODIFIER_LOW);
			break;

		// sub a, ri
		case 0x19:
			sprintf(buffer, "sub a, %s%s", RIJ, MODIFIER_LOW);
			break;

		// mset reg, (ri), s
		case 0x1a:
		case 0x1b:
			sprintf(buffer, "mset %s, (%s%s), %d", reg[(op >> 5) & 0x1f], RI(op), MODIFIER_LOW, (op >> 4) & 1);
			break;

		// subi simm
		case 0x1c:
			sprintf(buffer, "subi %X", op & 0xff);
			break;

		// sub a, erij
		case 0x23:
			sprintf(buffer, "sub a, %s%s", erij[(op >> 2) & 7], MODIFIER(op));
			break;

		// call cond, addr
		case 0x24: //f bit?
			sprintf(buffer, "call %s, %X", COND, READ_OP_DASM(pc+2));
			size = 2;
			break;

		// bra cond, addr
		case 0x26: //f bit?
			sprintf(buffer, "bra %s, %X", COND, READ_OP_DASM(pc+2));
			size = 2;
			break;


		case 0x28:

			if(op & 0x100)
			{
				// slow imm
				sprintf(buffer, "slow %X", op & 0xf);
			}
			else
			{
				// stop
				sprintf(buffer, "stop");
			}

			break;

		// cmp a, s
		case 0x30:
			sprintf(buffer, "cmp a, %s", reg[op & 0xf]);
			break;

		// cmp a, (ri)
		case 0x31:
			sprintf(buffer, "cmp a, (%s%s)", RIJ, MODIFIER_LOW);
			break;

		// cmp a, adr
		case 0x33:
			sprintf(buffer, "cmp a, %X", op & 0xff);
			break;

		// cmpi a, imm
		case 0x34:
			sprintf(buffer, "cmpi a, %X", READ_OP_DASM(pc+2));
			size = 2;
			break;

		// cmp a, ((ri))
		case 0x35:
			sprintf(buffer, "cmp a, ((%s%s))", RIJ, MODIFIER_LOW);
			break;

		// cmp a, ri
		case 0x39:
			sprintf(buffer, "cmp a, %s%s", RIJ, MODIFIER_LOW);
			break;

		// cmpi simm
		case 0x3c:
			sprintf(buffer, "cmpi %X", op & 0xff);
			break;

		// add a, s
		case 0x40:
			sprintf(buffer, "add a, %s", reg[op & 0xf]);
			break;

		// add a, (ri)
		case 0x41:
			sprintf(buffer, "add a, (%s%s)", RIJ, MODIFIER_LOW);
			break;

		// mpya s, (ri), b
		case 0x42:
			sprintf(buffer, "mpya %s, (%s%s), %d", reg[(op >> 4) & 0xf], RI(op), MODIFIER_LOW, BIT_B);
			break;

		// add a, adr
		case 0x43:
			sprintf(buffer, "add a, %X", op & 0xff);
			break;

		// addi a, imm
		case 0x44:
			//sprintf(buffer, "addi a, %X", READ_OP_DASM(pc+2));
			size = 2;

			if(op & 0x1ff)
				sprintf(buffer, "addi a, %X (CHECK ME!)", READ_OP_DASM(pc+2));
			else
				sprintf(buffer, "addi a, %X", READ_OP_DASM(pc+2));

			break;

		// add a, ((ri))
		case 0x45:
			sprintf(buffer, "add a, ((%s%s))", RIJ, MODIFIER_LOW);
			break;

		// mod cond, op
		case 0x48:
			sprintf(buffer, "mod %s, %s", COND, acc_op[op & 7]);
			break;

		// add a, ri
		case 0x49:
			sprintf(buffer, "add a, %s%s", RIJ, MODIFIER_LOW);
			break;

		// mod f, op
		case 0x4a:
			sprintf(buffer, "%s", flag_op[op & 0xf]);
			break;

		// mpya (rj), (ri), b
		case 0x4b:
			sprintf(buffer, "mpya (%s%s), (%s%s), %d", RJ(op >> 4), MODIFIER_HIGH, RI(op), MODIFIER_LOW, BIT_B);
			break;

		// addi simm
		case 0x4c: // docs show a wrong opcode value
			sprintf(buffer, "addi %X", op & 0xff);
			break;

		// ld reg, erij
		case 0x4e:
		case 0x4f:
			sprintf(buffer, "ld %s, %s%s", reg[(op >> 5) & 0x1f], erij[(op >> 2) & 7], MODIFIER(op));
			break;

		// and a, s
		case 0x50:
			sprintf(buffer, "and a, %s", reg[op & 0xf]);
			break;

		// and a, (ri)
		case 0x51:
			sprintf(buffer, "and a, (%s%s)", RIJ, MODIFIER_LOW);
			break;

		// mld s, (ri), b
		case 0x52:
			sprintf(buffer, "mld %s, (%s%s), %d", reg[(op >> 4) & 0xf], RI(op), MODIFIER_LOW, BIT_B);
			break;

		// and a, adr
		case 0x53:
			sprintf(buffer, "and a, %X", op & 0xff);
			break;

		// andi a, imm
		case 0x54:
//          sprintf(buffer, "andi a, %X", READ_OP_DASM(pc+2));
			size = 2;

			if(op & 0x1ff)
				sprintf(buffer, "andi a, %X (CHECK ME!)", READ_OP_DASM(pc+2));
			else
				sprintf(buffer, "andi a, %X", READ_OP_DASM(pc+2));

			break;

		// and a, ((ri))
		case 0x55:
			sprintf(buffer, "and a, ((%s%s))", RIJ, MODIFIER_LOW);
			break;

		// and a, ri
		case 0x59:
			sprintf(buffer, "and a, %s%s", RIJ, MODIFIER_LOW);
			break;

		// mld (rj), (ri), b
		case 0x5b:
			sprintf(buffer, "mld (%s%s), (%s%s), %d", RJ(op >> 4), MODIFIER_HIGH, RI(op), MODIFIER_LOW, BIT_B);
			break;

		// andi simm
		case 0x5c:
			sprintf(buffer, "andi %X", op & 0xff);
			break;

		// ld erij, reg
		case 0x5e:
		case 0x5f:
			sprintf(buffer, "ld %s%s, %s", erij[(op >> 2) & 7], MODIFIER(op), reg[(op >> 5) & 0x1f]);
			break;

		// or a, s
		case 0x60:
			sprintf(buffer, "or a, %s", reg[op & 0xf]);
			break;

		// or a, (ri)
		case 0x61:
			sprintf(buffer, "or a, (%s%s)", RIJ, MODIFIER_LOW);
			break;

		// or a, adr
		case 0x63:
			sprintf(buffer, "or a, %X", op & 0xff);
			break;

		// ori a, imm
		case 0x64:
			sprintf(buffer, "ori a, %X", READ_OP_DASM(pc+2));
			size = 2;
			break;

		// or a, ((ri))
		case 0x65:
			sprintf(buffer, "or a, ((%s%s))", RIJ, MODIFIER_LOW);
			break;

		// or a, ri
		case 0x69:
			sprintf(buffer, "or a, %s%s", RIJ, MODIFIER_LOW);
			break;

		// ori simm
		case 0x6c:
			sprintf(buffer, "ori %X", op & 0xff);
			break;

		// eor a, s
		case 0x70:
			sprintf(buffer, "eor a, %s", reg[op & 0xf]);
			break;

		// eor a, (ri)
		case 0x71:
			sprintf(buffer, "eor a, (%s%s)", RIJ, MODIFIER_LOW);
			break;

		// eor a, adr
		case 0x73:
			sprintf(buffer, "eor a, %X", op & 0xff);
			break;

		// eori a, imm
		case 0x74:
			sprintf(buffer, "eori a, %X", READ_OP_DASM(pc+2));
			size = 2;
			break;

		// eor a, ((ri))
		case 0x75:
			sprintf(buffer, "eor a, ((%s%s))", RIJ, MODIFIER_LOW);
			break;

		// eor a, ri
		case 0x79:
			sprintf(buffer, "eor a, %s%s", RIJ, MODIFIER_LOW);
			break;

		// eori simm
		case 0x7c:
			sprintf(buffer, "eori %X", op & 0xff);
			break;

		default:
			sprintf(buffer, "OP = %02X",op_num);
			break;
	}

	return size | flags | DASMFLAG_SUPPORTED;
}
