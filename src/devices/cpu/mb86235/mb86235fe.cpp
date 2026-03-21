// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for MB86235 recompiler

******************************************************************************/

#include "emu.h"
#include "mb86235fe.h"

#include "cpu/drcfe.ipp"


mb86235_device::frontend::frontend(mb86235_device *core, uint32_t window_start, uint32_t window_end, uint32_t max_sequence) :
	drc_frontend_base(core->space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence),
	m_core(core)
{
}

mb86235_device::frontend::~frontend()
{
}


mb86235_device::opcode_desc const *mb86235_device::frontend::describe_code(offs_t startpc)
{
	return do_describe_code(
			[this] (opcode_desc &desc, opcode_desc const *prev) { return describe(desc, prev); },
			startpc);
}


bool mb86235_device::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	uint64_t opcode = desc.opptr = m_core->m_pcache.read_qword(desc.pc, 0);

	desc.length = 1;

	// repeatable instruction needs an entry point
	if (prev != nullptr)
	{
		if (prev->repeat())
		{
			desc.set_is_branch_target();
			desc.set_repeated_op();
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

void mb86235_device::frontend::describe_alu_input(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			desc.set_aa_used(reg & 7);
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			desc.set_ab_used(reg & 7);
			break;

		case 0x10:  // PR
			break;

		case 0x11:  // PR++
			desc.set_pr_inc();
			break;
		case 0x12:  // PR--
			desc.set_pr_dec();
			break;
		case 0x13:  // PR#0
			desc.set_pr_zero();
			break;


		default:
			break;
	}
}

void mb86235_device::frontend::describe_mul_input(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			desc.set_ma_used(reg & 7);
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			desc.set_mb_used(reg & 7);
			break;

		case 0x10:  // PR
			break;

		case 0x11:  // PR++
			if (desc.pr() == 0)    // ALU PR update has higher priority
			{
				desc.set_pr_inc();
			}
			break;
		case 0x12:  // PR--
			if (desc.pr() == 0)    // ALU PR update has higher priority
			{
				desc.set_pr_dec();
			}
			break;
		case 0x13:  // PR#0
			if (desc.pr() == 0)    // ALU PR update has higher priority
			{
				desc.set_pr_zero();
			}
			break;

		default:
			break;
	}
}

void mb86235_device::frontend::describe_alumul_output(opcode_desc &desc, int reg)
{
	switch (reg >> 3)
	{
		case 0:
			desc.set_ma_modified(reg & 7);
			break;

		case 1:
			desc.set_mb_modified(reg & 7);
			break;

		case 2:
			desc.set_aa_modified(reg & 7);
			break;

		case 3:
			desc.set_ab_modified(reg & 7);
			break;

		default:
			break;
	}
}

void mb86235_device::frontend::describe_reg_read(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			// MA0-7
			desc.set_ma_used(reg & 7);
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			// AA0-7
			desc.set_aa_used(reg & 7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			// AR0-7
			desc.set_ar_used(reg & 7);
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			// MB0-7
			desc.set_mb_used(reg & 7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			// AB0-7
			desc.set_ab_used(reg & 7);
			break;

		case 0x31:      // FI
			desc.set_fifoin();
			desc.set_is_branch_target();      // fifo check makes this a branch target
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
			if (desc.pr() == 0)        // ALU and MUL PR updates have higher priority
			{
				desc.set_pr_inc();
			}
			break;
	}
}

void mb86235_device::frontend::describe_reg_write(opcode_desc &desc, int reg)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			// MA0-7
			desc.set_ma_modified(reg & 7);
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			// AA0-7
			desc.set_aa_modified(reg & 7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			// AR0-7
			desc.set_ar_modified(reg & 7);
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			// MB0-7
			desc.set_mb_modified(reg & 7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			// AB0-7
			desc.set_ab_modified(reg & 7);
			break;

		case 0x31:      // FI
			break;

		case 0x32:      // FO0
		case 0x33:      // FO1
			desc.set_fifoout();
			desc.set_is_branch_target();      // fifo check makes this a branch target
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
			desc.set_pw_inc();
			break;
	}
}


void mb86235_device::frontend::describe_alu(opcode_desc &desc, uint32_t aluop)
{
	int i1 = (aluop >> 10) & 0xf;
	int i2 = (aluop >> 5) & 0x1f;
	int io = aluop & 0x1f;
	int op = (aluop >> 14) & 0x1f;

	switch (op)
	{
		case 0x00:      // FADD
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			break;
		case 0x01:      // FADDZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_zc_modified();
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x02:      // FSUB
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x03:      // FSUBZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_zc_modified();
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x04:      // FCMP
			describe_alu_input(desc, i1); describe_alu_input(desc, i2);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x05:      // FABS
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_ad_modified();
			break;
		case 0x06:      // FABC
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x07:      // NOP
			break;
		case 0x08:      // FEA
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_ad_modified();
			break;
		case 0x09:      // FES
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x0a:      // FRCP
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_zd_modified();
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x0b:      // FRSQ
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_nr_modified();
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_au_modified();
			desc.set_ad_modified();
			break;
		case 0x0c:      // FLOG
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_il_modified();
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_ad_modified();
			break;
		case 0x0d:      // CIF
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			break;
		case 0x0e:      // CFI
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_ad_modified();
			break;
		case 0x0f:      // CFIB
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_ad_modified();
			desc.set_au_modified();
			break;
		case 0x10:      // ADD
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			break;
		case 0x11:      // ADDZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_zc_modified();
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			break;
		case 0x12:      // SUB
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			break;
		case 0x13:      // SUBZ
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_zc_modified();
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			break;
		case 0x14:      // CMP
			describe_alu_input(desc, i1); describe_alu_input(desc, i2);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			break;
		case 0x15:      // ABS
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			break;
		case 0x16:      // ATR
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			break;
		case 0x17:      // ATRZ
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_zc_modified();
			break;
		case 0x18:      // AND
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			break;
		case 0x19:      // OR
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			break;
		case 0x1a:      // XOR
			describe_alu_input(desc, i1); describe_alu_input(desc, i2); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			break;
		case 0x1b:      // NOT
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			break;
		case 0x1c:      // LSR
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			break;
		case 0x1d:      // LSL
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			desc.set_av_modified();
			desc.set_au_modified();
			break;
		case 0x1e:      // ASR
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			break;
		case 0x1f:      // ASL
			describe_alu_input(desc, i1); describe_alumul_output(desc, io);
			desc.set_an_modified();
			desc.set_az_modified();
			break;
	}
}

void mb86235_device::frontend::describe_mul(opcode_desc &desc, uint32_t mulop)
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
		desc.set_mn_modified();
		desc.set_mz_modified();
		desc.set_mv_modified();
		desc.set_mu_modified();
		desc.set_md_modified();
	}
	else
	{
		// MUL
		desc.set_mn_modified();
		desc.set_mz_modified();
		desc.set_mv_modified();
	}
}

void mb86235_device::frontend::describe_ea(opcode_desc &desc, int md, int arx, int ary, int disp)
{
	switch (md)
	{
		case 0x0:       // @ARx
			desc.set_ar_used(arx);
			break;
		case 0x1:       // @ARx++
			desc.set_ar_used(arx); desc.set_ar_modified(arx);
			break;
		case 0x2:       // @ARx--
			desc.set_ar_used(arx); desc.set_ar_modified(arx);
			break;
		case 0x3:       // @ARx++disp
			desc.set_ar_used(arx); desc.set_ar_modified(arx);
			break;
		case 0x4:       // @ARx+ARy
			desc.set_ar_used(arx); desc.set_ar_used(ary);
			break;
		case 0x5:       // @ARx+ARy++
			desc.set_ar_used(arx); desc.set_ar_used(ary); desc.set_ar_modified(ary);
			break;
		case 0x6:       // @ARx+ARy--
			desc.set_ar_used(arx); desc.set_ar_used(ary); desc.set_ar_modified(ary);
			break;
		case 0x7:       // @ARx+ARy++disp
			desc.set_ar_used(arx); desc.set_ar_used(ary); desc.set_ar_modified(ary);
			break;
		case 0x8:       // @ARx+ARyU
			desc.set_ar_used(arx); desc.set_ar_used(ary);
			break;
		case 0x9:       // @ARx+ARyL
			desc.set_ar_used(arx); desc.set_ar_used(ary);
			break;
		case 0xa:       // @ARx+disp
			desc.set_ar_used(arx);
			break;
		case 0xb:       // @ARx+ARy+disp
			desc.set_ar_used(arx); desc.set_ar_used(ary);
			break;
		case 0xc:       // @disp
			break;
		case 0xd:       // @ARx+[ARy++]
			desc.set_ar_used(arx); desc.set_ar_used(ary); desc.set_ar_modified(ary);
			break;
		case 0xe:       // @ARx+[ARy--]
			desc.set_ar_used(arx); desc.set_ar_used(ary); desc.set_ar_modified(ary);
			break;
		case 0xf:       // @ARx+[ARy++disp]
			desc.set_ar_used(arx); desc.set_ar_used(ary); desc.set_ar_modified(ary);
			break;
	}
}

void mb86235_device::frontend::describe_xfer1(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr;

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
			desc.set_reads_memory();
		}

		if ((dr & 0x40) == 0)
		{
			describe_reg_write(desc, dr & 0x3f);
		}
		else
		{
			describe_ea(desc, md, dr & 7, ary, disp5);
			desc.set_writes_memory();
		}
	}
	else
	{
		// external transfer
		if (dir == 0)
		{
			describe_reg_read(desc, dr & 0x3f);
			desc.set_writes_memory();
		}
		else
		{
			describe_reg_write(desc, dr & 0x3f);
			desc.set_reads_memory();
		}
	}
}

void mb86235_device::frontend::describe_double_xfer1(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr;

	fatalerror("mb86235_device::frontend: describe_double_xfer1 at %08X (%08X%08X)", desc.pc, (uint32_t)(opcode >> 32), (uint32_t)(opcode));
}

void mb86235_device::frontend::describe_xfer2(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr;

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
				desc.set_reads_memory();
			}

			if ((dr & 0x40) == 0)
			{
				describe_reg_write(desc, dr & 0x3f);
			}
			else
			{
				describe_ea(desc, md, dr & 7, ary, disp14);
				desc.set_writes_memory();
			}
		}
		else
		{
			// external transfer
			if (dir == 0)
			{
				describe_reg_read(desc, dr & 0x3f);
				desc.set_writes_memory();
			}
			else
			{
				describe_reg_write(desc, dr & 0x3f);
				desc.set_reads_memory();
			}
		}
	}
	else if (op == 2)   // MOV4
	{
		fatalerror("mb86235_device::frontend: describe_xfer2 MOV4 at %08X (%08X%08X)", desc.pc, (uint32_t)(opcode >> 32), (uint32_t)(opcode));
	}

}

void mb86235_device::frontend::describe_double_xfer2(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr;
	fatalerror("mb86235_device::frontend: describe_double_xfer2 at %08X (%08X%08X)", desc.pc, (uint32_t)(opcode >> 32), (uint32_t)(opcode));
}

void mb86235_device::frontend::describe_xfer3(opcode_desc &desc)
{
	uint64_t opcode = desc.opptr;

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
			desc.set_writes_memory();
			describe_ea(desc, md, dr & 7, ary, disp);
			break;
	}
}

void mb86235_device::frontend::describe_control(opcode_desc &desc)
{
	int ef1 = (desc.opptr >> 16) & 0x3f;
	int ef2 = desc.opptr & 0xffff;
	int cop = (desc.opptr >> 22) & 0x1f;
	int rel12 = util::sext<unsigned>(desc.opptr, 12);

	switch (cop)
	{
		case 0x00:      // NOP
			break;
		case 0x01:      // REP
			if (ef1 != 0)   // ARx
				desc.set_ar_used((ef2 >> 12) & 7);

			desc.set_repeat();
			break;
		case 0x02:      // SETL
			if (ef1 != 0)   // ARx
				desc.set_ar_used((ef2 >> 12) & 7);
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
			desc.set_is_conditional_branch();
			desc.targetpc = desc.pc + rel12;
			desc.delayslots = 1;
			break;
		case 0x14:      // DBBC ARx:y, rel12
			desc.set_is_conditional_branch();
			desc.targetpc = desc.pc + rel12;
			desc.delayslots = 1;
			desc.set_ar_used(((desc.opptr >> 13) & 7));
			break;
		case 0x15:      // DBBS ARx:y, rel12
			desc.set_is_conditional_branch();
			desc.targetpc = desc.pc + rel12;
			desc.delayslots = 1;
			desc.set_ar_used(((desc.opptr >> 13) & 7));
			break;
		case 0x1b:      // DRET
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.delayslots = 1;
			break;

		case 0x10:      // DBcc
		case 0x11:      // DBNcc
		case 0x18:      // DCcc
		case 0x19:      // DCNcc
		{
			switch ((desc.opptr >> 12) & 0xf)
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

			desc.set_is_conditional_branch();
			desc.delayslots = 1;
			break;
		}

		case 0x1a:      // DCALL
		case 0x12:      // DJMP
		{
			switch ((desc.opptr >> 12) & 0xf)
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

			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			desc.delayslots = 1;
			break;
		}
	}
}
