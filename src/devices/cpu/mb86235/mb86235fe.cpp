// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for MB86235 recompiler

******************************************************************************/

#include "emu.h"
#include "mb86235fe.h"

#include "mb86235defs.h"


#define AA_USED(desc,x)             do { (desc).regin[0] |= 1 << (x); } while(0)
#define AA_MODIFIED(desc,x)         do { (desc).regout[0] |= 1 << (x); } while(0)
#define AB_USED(desc,x)             do { (desc).regin[0] |= 1 << (8+(x)); } while(0)
#define AB_MODIFIED(desc,x)         do { (desc).regout[0] |= 1 << (8+(x)); } while(0)
#define MA_USED(desc,x)             do { (desc).regin[0] |= 1 << (16+(x)); } while(0)
#define MA_MODIFIED(desc,x)         do { (desc).regout[0] |= 1 << (16+(x)); } while(0)
#define MB_USED(desc,x)             do { (desc).regin[0] |= 1 << (24+(x)); } while(0)
#define MB_MODIFIED(desc,x)         do { (desc).regout[0] |= 1 << (24+(x)); } while(0)
#define AR_USED(desc,x)             do { (desc).regin[1] |= 1 << (24+(x)); } while(0)
#define AR_MODIFIED(desc,x)         do { (desc).regout[1] |= 1 << (24+(x)); } while(0)

#define AZ_USED(desc)               do { (desc).regin[1] |= 1 << 0; } while (0)
#define AZ_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 0; } while (0)
#define AN_USED(desc)               do { (desc).regin[1] |= 1 << 1; } while (0)
#define AN_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 1; } while (0)
#define AV_USED(desc)               do { (desc).regin[1] |= 1 << 2; } while (0)
#define AV_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 2; } while (0)
#define AU_USED(desc)               do { (desc).regin[1] |= 1 << 3; } while (0)
#define AU_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 3; } while (0)
#define AD_USED(desc)               do { (desc).regin[1] |= 1 << 4; } while (0)
#define AD_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 4; } while (0)
#define ZC_USED(desc)               do { (desc).regin[1] |= 1 << 5; } while (0)
#define ZC_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 5; } while (0)
#define IL_USED(desc)               do { (desc).regin[1] |= 1 << 6; } while (0)
#define IL_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 6; } while (0)
#define NR_USED(desc)               do { (desc).regin[1] |= 1 << 7; } while (0)
#define NR_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 7; } while (0)
#define ZD_USED(desc)               do { (desc).regin[1] |= 1 << 8; } while (0)
#define ZD_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 8; } while (0)
#define MN_USED(desc)               do { (desc).regin[1] |= 1 << 9; } while (0)
#define MN_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 9; } while (0)
#define MZ_USED(desc)               do { (desc).regin[1] |= 1 << 10; } while (0)
#define MZ_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 10; } while (0)
#define MV_USED(desc)               do { (desc).regin[1] |= 1 << 11; } while (0)
#define MV_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 11; } while (0)
#define MU_USED(desc)               do { (desc).regin[1] |= 1 << 12; } while (0)
#define MU_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 12; } while (0)
#define MD_USED(desc)               do { (desc).regin[1] |= 1 << 13; } while (0)
#define MD_MODIFIED(desc)           do { (desc).regout[1] |= 1 << 13; } while (0)


mb86235_frontend::mb86235_frontend(mb86235_device *core, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend(*core, window_start, window_end, max_sequence),
	m_core(core)
{
}


bool mb86235_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	uint64_t opcode = desc.opptr.q[0] = m_core->m_pcache.read_qword(desc.pc, 0);

	desc.length = 1;
	desc.cycles = 1;

	// repeatable instruction needs an entry point
	if (prev != nullptr)
	{
		if (prev->userflags & OP_USERFLAG_REPEAT)
		{
			desc.flags |= OPFLAG_IS_BRANCH_TARGET;
			desc.userflags |= OP_USERFLAG_REPEATED_OP;
		}
	}

	switch ((opcode >> 61) & 7)
	{
		case 0:     // ALU / MUL / double transfer (type 1)
			describe_alu(desc, (opcode >> 42) & 0x7ffff);
			describe_mul(desc, (opcode >> 27) & 0x7fff);
			describe_double_xfer1(desc);
			break;
		case 1:     // ALU / MUL / transfer (type 1)
			describe_alu(desc, (opcode >> 42) & 0x7ffff);
			describe_mul(desc, (opcode >> 27) & 0x7fff);
			describe_xfer1(desc);
			break;
		case 2:     // ALU / MUL / control
			describe_alu(desc, (opcode >> 42) & 0x7ffff);
			describe_mul(desc, (opcode >> 27) & 0x7fff);
			describe_control(desc);
			break;
		case 4:     // ALU or MUL / double transfer (type 2)
			if (opcode & ((uint64_t)(1) << 41))
				describe_alu(desc, (opcode >> 42) & 0x7ffff);
			else
				describe_mul(desc, (opcode >> 42) & 0x7fff);
			describe_double_xfer2(desc);
			break;
		case 5:     // ALU or MUL / transfer (type 2)
			if (opcode & ((uint64_t)(1) << 41))
				describe_alu(desc, (opcode >> 42) & 0x7ffff);
			else
				describe_mul(desc, (opcode >> 42) & 0x7fff);
			describe_xfer2(desc);
			break;
		case 6:     // ALU or MUL / control
			if (opcode & ((uint64_t)(1) << 41))
				describe_alu(desc, (opcode >> 42) & 0x7ffff);
			else
				describe_mul(desc, (opcode >> 42) & 0x7fff);
			describe_control(desc);
			break;
		case 7:     // transfer (type 3)
			describe_xfer3(desc);
			break;

		default:
			return false;
	}

	return true;
}

void mb86235_frontend::describe_alu_input(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			AA_USED(desc, reg & 7);
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			AB_USED(desc, reg & 7);
			break;

		case 0x10:  // PR
			break;

		case 0x11:  // PR++
			desc.userflags &= ~OP_USERFLAG_PR_MASK;
			desc.userflags |= OP_USERFLAG_PR_INC;
			break;
		case 0x12:  // PR--
			desc.userflags &= ~OP_USERFLAG_PR_MASK;
			desc.userflags |= OP_USERFLAG_PR_DEC;
			break;
		case 0x13:  // PR#0
			desc.userflags &= ~OP_USERFLAG_PR_MASK;
			desc.userflags |= OP_USERFLAG_PR_ZERO;
			break;


		default:
			break;
	}
}

void mb86235_frontend::describe_mul_input(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			MA_USED(desc, reg & 7);
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			MB_USED(desc, reg & 7);
			break;

		case 0x10:  // PR
			break;

		case 0x11:  // PR++
			if ((desc.userflags & OP_USERFLAG_PR_MASK) == 0)    // ALU PR update has higher priority
			{
				desc.userflags |= OP_USERFLAG_PR_INC;
			}
			break;
		case 0x12:  // PR--
			if ((desc.userflags & OP_USERFLAG_PR_MASK) == 0)    // ALU PR update has higher priority
			{
				desc.userflags |= OP_USERFLAG_PR_DEC;
			}
			break;
		case 0x13:  // PR#0
			if ((desc.userflags & OP_USERFLAG_PR_MASK) == 0)    // ALU PR update has higher priority
			{
				desc.userflags |= OP_USERFLAG_PR_ZERO;
			}
			break;

		default:
			break;
	}
}

void mb86235_frontend::describe_alumul_output(opcode_desc &desc, int reg)
{
	switch (reg >> 3)
	{
		case 0:
			MA_MODIFIED(desc, reg & 7);
			break;

		case 1:
			MB_MODIFIED(desc, reg & 7);
			break;

		case 2:
			AA_MODIFIED(desc, reg & 7);
			break;

		case 3:
			AB_MODIFIED(desc, reg & 7);
			break;

		default:
			break;
	}
}

void mb86235_frontend::describe_reg_read(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			// MA0-7
			MA_USED(desc, reg & 7);
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			// AA0-7
			AA_USED(desc, reg & 7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			// AR0-7
			AR_USED(desc, reg & 7);
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			// MB0-7
			MB_USED(desc, reg & 7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			// AB0-7
			AB_USED(desc, reg & 7);
			break;

		case 0x31:      // FI
			desc.userflags |= OP_USERFLAG_FIFOIN;
			desc.flags |= OPFLAG_IS_BRANCH_TARGET;      // fifo check makes this a branch target
			break;

		case 0x32:      // FO0
		case 0x33:      // FO1
			break;

		case 0x10:      // EB
		case 0x11:      // EBU
		case 0x12:      // EBL
		case 0x13:      // EO
		case 0x15:      // ST
		case 0x16:      // MOD
		case 0x17:      // LRPC
		case 0x34:      // PDR
		case 0x35:      // DDR
		case 0x36:      // PRP
		case 0x37:      // PWP
			break;

		case 0x30:      // PR
			if ((desc.userflags & OP_USERFLAG_PR_MASK) == 0)        // ALU and MUL PR updates have higher priority
			{
				desc.userflags |= OP_USERFLAG_PR_INC;
			}
			break;
	}
}

void mb86235_frontend::describe_reg_write(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			// MA0-7
			MA_MODIFIED(desc, reg & 7);
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			// AA0-7
			AA_MODIFIED(desc, reg & 7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			// AR0-7
			AR_MODIFIED(desc, reg & 7);
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			// MB0-7
			MB_MODIFIED(desc, reg & 7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			// AB0-7
			AB_MODIFIED(desc, reg & 7);
			break;

		case 0x31:      // FI
			break;

		case 0x32:      // FO0
		case 0x33:      // FO1
			desc.userflags |= OP_USERFLAG_FIFOOUT;
			desc.flags |= OPFLAG_IS_BRANCH_TARGET;      // fifo check makes this a branch target
			break;

		case 0x10:      // EB
		case 0x11:      // EBU
		case 0x12:      // EBL
		case 0x13:      // EO
		case 0x15:      // ST
		case 0x16:      // MOD
		case 0x17:      // LRPC
		case 0x34:      // PDR
		case 0x35:      // DDR
		case 0x36:      // PRP
		case 0x37:      // PWP
			break;

		case 0x30:      // PR
			desc.userflags &= ~OP_USERFLAG_PW_MASK;
			desc.userflags |= OP_USERFLAG_PW_INC;
			break;
	}
}


void mb86235_frontend::describe_alu(opcode_desc &desc, uint32_t aluop)
{
	int i1 = (aluop >> 10) & 0xf;
	int i2 = (aluop >> 5) & 0x1f;
	int io = aluop & 0x1f;
	int op = (aluop >> 14) & 0x1f;

	switch (op)
	{
		case 0x00:      // FADD
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x01:      // FADDZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			ZC_MODIFIED(desc);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x02:      // FSUB
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x03:      // FSUBZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			ZC_MODIFIED(desc);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x04:      // FCMP
			describe_alu_input(desc, i1); describe_alu_input(desc, i2);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x05:      // FABS
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x06:      // FABC
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x07:      // NOP
			break;
		case 0x08:      // FEA
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x09:      // FES
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x0a:      // FRCP
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			ZD_MODIFIED(desc);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x0b:      // FRSQ
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			NR_MODIFIED(desc);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AU_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x0c:      // FLOG
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			IL_MODIFIED(desc);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x0d:      // CIF
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			break;
		case 0x0e:      // CFI
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AD_MODIFIED(desc);
			break;
		case 0x0f:      // CFIB
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AD_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x10:      // ADD
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			break;
		case 0x11:      // ADDZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			ZC_MODIFIED(desc);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			break;
		case 0x12:      // SUB
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			break;
		case 0x13:      // SUBZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			ZC_MODIFIED(desc);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			break;
		case 0x14:      // CMP
			describe_alu_input(desc, i1); describe_alu_input(desc, i2);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			break;
		case 0x15:      // ABS
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			break;
		case 0x16:      // ATR
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			break;
		case 0x17:      // ATRZ
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			ZC_MODIFIED(desc);
			break;
		case 0x18:      // AND
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x19:      // OR
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x1a:      // XOR
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x1b:      // NOT
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x1c:      // LSR
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x1d:      // LSL
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			AV_MODIFIED(desc);
			AU_MODIFIED(desc);
			break;
		case 0x1e:      // ASR
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			break;
		case 0x1f:      // ASL
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			AN_MODIFIED(desc);
			AZ_MODIFIED(desc);
			break;
	}
}

void mb86235_frontend::describe_mul(opcode_desc &desc, uint32_t mulop)
{
	int i1 = (mulop >> 10) & 0xf;
	int i2 = (mulop >> 5) & 0x1f;
	int io = mulop & 0x1f;
	int m = mulop & 0x4000;

	describe_mul_input(desc, i1);
	describe_mul_input(desc, i2);
	describe_alumul_output(desc, io);

	if (m)
	{
		// FMUL
		MN_MODIFIED(desc);
		MZ_MODIFIED(desc);
		MV_MODIFIED(desc);
		MU_MODIFIED(desc);
		MD_MODIFIED(desc);
	}
	else
	{
		// MUL
		MN_MODIFIED(desc);
		MZ_MODIFIED(desc);
		MV_MODIFIED(desc);
	}
}

void mb86235_frontend::describe_ea(opcode_desc &desc, int md, int arx, int ary, int disp)
{
	switch (md)
	{
		case 0x0:       // @ARx
			AR_USED(desc, arx);
			break;
		case 0x1:       // @ARx++
			AR_USED(desc, arx); AR_MODIFIED(desc, arx);
			break;
		case 0x2:       // @ARx--
			AR_USED(desc, arx); AR_MODIFIED(desc, arx);
			break;
		case 0x3:       // @ARx++disp
			AR_USED(desc, arx); AR_MODIFIED(desc, arx);
			break;
		case 0x4:       // @ARx+ARy
			AR_USED(desc, arx); AR_USED(desc, ary);
			break;
		case 0x5:       // @ARx+ARy++
			AR_USED(desc, arx); AR_USED(desc, ary); AR_MODIFIED(desc, ary);
			break;
		case 0x6:       // @ARx+ARy--
			AR_USED(desc, arx); AR_USED(desc, ary); AR_MODIFIED(desc, ary);
			break;
		case 0x7:       // @ARx+ARy++disp
			AR_USED(desc, arx); AR_USED(desc, ary); AR_MODIFIED(desc, ary);
			break;
		case 0x8:       // @ARx+ARyU
			AR_USED(desc, arx); AR_USED(desc, ary);
			break;
		case 0x9:       // @ARx+ARyL
			AR_USED(desc, arx); AR_USED(desc, ary);
			break;
		case 0xa:       // @ARx+disp
			AR_USED(desc, arx);
			break;
		case 0xb:       // @ARx+ARy+disp
			AR_USED(desc, arx); AR_USED(desc, ary);
			break;
		case 0xc:       // @disp
			break;
		case 0xd:       // @ARx+[ARy++]
			AR_USED(desc, arx); AR_USED(desc, ary); AR_MODIFIED(desc, ary);
			break;
		case 0xe:       // @ARx+[ARy--]
			AR_USED(desc, arx); AR_USED(desc, ary); AR_MODIFIED(desc, ary);
			break;
		case 0xf:       // @ARx+[ARy++disp]
			AR_USED(desc, arx); AR_USED(desc, ary); AR_MODIFIED(desc, ary);
			break;
	}
}

void mb86235_frontend::describe_xfer1(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr.q[0];

	int dr = (opcode >> 12) & 0x7f;
	int sr = (opcode >> 19) & 0x7f;
	int md = opcode & 0xf;
	int ary = (opcode >> 4) & 7;
	int disp5 = (opcode >> 7) & 0x1f;
	int trm = (opcode >> 26) & 1;
	int dir = (opcode >> 25) & 1;

	if (trm == 0)
	{
		if ((sr & 0x40) == 0)
		{
			describe_reg_read(desc, sr & 0x3f);
		}
		else if (sr == 0x58)
		{
			// MOV1 #imm12, DR
		}
		else
		{
			describe_ea(desc, md, sr & 7, ary, disp5);
			desc.flags |= OPFLAG_READS_MEMORY;
		}

		if ((dr & 0x40) == 0)
		{
			describe_reg_write(desc, dr & 0x3f);
		}
		else
		{
			describe_ea(desc, md, dr & 7, ary, disp5);
			desc.flags |= OPFLAG_WRITES_MEMORY;
		}
	}
	else
	{
		// external transfer
		if (dir == 0)
		{
			describe_reg_read(desc, dr & 0x3f);
			desc.flags |= OPFLAG_WRITES_MEMORY;
		}
		else
		{
			describe_reg_write(desc, dr & 0x3f);
			desc.flags |= OPFLAG_READS_MEMORY;
		}
	}
}

void mb86235_frontend::describe_double_xfer1(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr.q[0];

	fatalerror("mb86235_frontend: describe_double_xfer1 at %08X (%08X%08X)", desc.pc, (uint32_t)(opcode >> 32), (uint32_t)(opcode));
}

void mb86235_frontend::describe_xfer2(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr.q[0];

	int op = (opcode >> 39) & 3;
	int trm = (opcode >> 38) & 1;
	int dir = (opcode >> 37) & 1;
	int sr = (opcode >> 31) & 0x7f;
	int dr = (opcode >> 24) & 0x7f;
	int ary = (opcode >> 4) & 7;
	int md = opcode & 0xf;
	int disp14 = (opcode >> 7) & 0x3fff;

	if (op == 0)    // MOV2
	{
		if (trm == 0)
		{
			if ((sr & 0x40) == 0)
			{
				describe_reg_read(desc, sr & 0x3f);
			}
			else if (sr == 0x58)
			{
				// MOV2 #imm24, DR
			}
			else
			{
				describe_ea(desc, md, sr & 7, ary, disp14);
				desc.flags |= OPFLAG_READS_MEMORY;
			}

			if ((dr & 0x40) == 0)
			{
				describe_reg_write(desc, dr & 0x3f);
			}
			else
			{
				describe_ea(desc, md, dr & 7, ary, disp14);
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
		}
		else
		{
			// external transfer
			if (dir == 0)
			{
				describe_reg_read(desc, dr & 0x3f);
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
			else
			{
				describe_reg_write(desc, dr & 0x3f);
				desc.flags |= OPFLAG_READS_MEMORY;
			}
		}
	}
	else if (op == 2)   // MOV4
	{
		fatalerror("mb86235_frontend: describe_xfer2 MOV4 at %08X (%08X%08X)", desc.pc, (uint32_t)(opcode >> 32), (uint32_t)(opcode));
	}

}

void mb86235_frontend::describe_double_xfer2(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr.q[0];
	fatalerror("mb86235_frontend: describe_double_xfer2 at %08X (%08X%08X)", desc.pc, (uint32_t)(opcode >> 32), (uint32_t)(opcode));
}

void mb86235_frontend::describe_xfer3(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr.q[0];

	int dr = (opcode >> 19) & 0x7f;
	int disp = (opcode >> 7) & 0xfff;
	int ary = (opcode >> 4) & 7;
	int md = opcode & 0xf;

	switch (dr >> 5)
	{
		case 0:
		case 1:     // reg
			describe_reg_write(desc, dr & 0x3f);
			break;

		case 2:     // RAM-A
		case 3:     // RAM-B
			desc.flags |= OPFLAG_WRITES_MEMORY;
			describe_ea(desc, md, dr & 7, ary, disp);
			break;
	}
}

void mb86235_frontend::describe_control(opcode_desc &desc)
{
	int ef1 = (desc.opptr.q[0] >> 16) & 0x3f;
	int ef2 = desc.opptr.q[0] & 0xffff;
	int cop = (desc.opptr.q[0] >> 22) & 0x1f;
	int rel12 = util::sext<int>(desc.opptr.q[0], 12);

	switch (cop)
	{
		case 0x00:      // NOP
			break;
		case 0x01:      // REP
			if (ef1 != 0)   // ARx
				AR_USED(desc, (ef2 >> 12) & 7);

			desc.userflags |= OP_USERFLAG_REPEAT;
			break;
		case 0x02:      // SETL
			if (ef1 != 0)   // ARx
				AR_USED(desc, (ef2 >> 12) & 7);
			break;
		case 0x03:      // CLRFI/CLRFO/CLRF
			break;
		case 0x04:      // PUSH
			describe_reg_read(desc, (ef2 >> 6) & 0x3f);
			break;
		case 0x05:      // POP
			describe_reg_write(desc, (ef2 >> 6) & 0x3f);
			break;
		case 0x08:      // SETM #imm16
			break;
		case 0x09:      // SETM #imm3, CBSA
			break;
		case 0x0a:      // SETM #imm3, CBSB
			break;
		case 0x0b:      // SETM #imm1, RF
			break;
		case 0x0c:      // SETM #imm1, RDY
			break;
		case 0x0d:      // SETM #imm1, WAIT
			break;
		case 0x13:      // DBLP rel12
			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.targetpc = desc.pc + rel12;
			desc.delayslots = 1;
			break;
		case 0x14:      // DBBC ARx:y, rel12
			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.targetpc = desc.pc + rel12;
			desc.delayslots = 1;
			AR_USED(desc, ((desc.opptr.q[0] >> 13) & 7));
			break;
		case 0x15:      // DBBS ARx:y, rel12
			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.targetpc = desc.pc + rel12;
			desc.delayslots = 1;
			AR_USED(desc, ((desc.opptr.q[0] >> 13) & 7));
			break;
		case 0x1b:      // DRET
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.delayslots = 1;
			break;

		case 0x10:      // DBcc
		case 0x11:      // DBNcc
		case 0x18:      // DCcc
		case 0x19:      // DCNcc
		{
			switch ((desc.opptr.q[0] >> 12) & 0xf)
			{
				case 0x0: desc.targetpc = ef2 & 0xfff; break;
				case 0x1: desc.targetpc = desc.pc + rel12; break;
				case 0x2: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x3: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x4: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x5: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x6: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x7: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x8: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0x9: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0xa: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0xb: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0xc: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0xd: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0xe: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0xf: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
			}

			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.delayslots = 1;
			break;
		}

		case 0x1a:      // DCALL
		case 0x12:      // DJMP
		{
			switch ((desc.opptr.q[0] >> 12) & 0xf)
			{
				case 0x0: desc.targetpc = ef2 & 0xfff; break;
				case 0x1: desc.targetpc = desc.pc + rel12; break;
				case 0x2: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x3: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x4: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x5: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x6: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x7: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0x8: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0x9: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0xa: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0xb: desc.targetpc = BRANCH_TARGET_DYNAMIC; break;
				case 0xc: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0xd: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0xe: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
				case 0xf: desc.targetpc = BRANCH_TARGET_DYNAMIC; describe_reg_read(desc, (ef2 >> 6) & 0x3f); break;
			}

			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.delayslots = 1;
			break;
		}
	}
}
