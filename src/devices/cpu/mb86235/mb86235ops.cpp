// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi, Matthew Daniels
/*****************************************************************************
 *
 * MB86235 "TGPx4" (c) Fujitsu
 *
 * Interpreter functions and opcodes
 *
 *****************************************************************************/

#include "emu.h"
#include "mb86235.h"

/*********************
 *
 * Misc helpers
 *
 ********************/

#define AD  0x00000001
#define AU  0x00000002
#define AV  0x00000004
#define AZ  0x00000008
#define AN  0x00000010
#define ZC  0x00000100
#define IL  0x00000200
#define NR  0x00000400
#define ZD  0x00000800
#define RP  0x00004000
#define LP  0x00008000
#define MD  0x00010000
#define MU  0x00020000
#define MV  0x00040000
#define MZ  0x00080000
#define MN  0x00100000
#define OFF 0x01000000
#define OFE 0x02000000
#define IFF 0x04000000
#define IFE 0x08000000
#define F0  0x10000000
#define F1  0x20000000
#define F2  0x40000000

#define FSET(f) m_core->st |= (f)
#define FCLR(f) m_core->st &= ~(f)

void mb86235_device::handle_single_step_execution()
{
	if (m_core->cur_fifo_state.has_stalled == true)
		return;

	// repeat opcode
	if (m_core->st & RP)
	{
		--m_core->rpc;
		if (m_core->rpc == 1)
			FCLR(RP);
	}
	else // normal operation
		m_core->pc++;
}

bool mb86235_device::check_previous_op_stall()
{
	return (m_core->cur_fifo_state.has_stalled == true) && ((m_core->st & RP) == 0);
}

inline void mb86235_device::increment_pwp()
{
	m_core->pwp++;
	if (m_core->pwp >= 24)
		m_core->pwp = 0;
}

void mb86235_device::increment_prp()
{
	m_core->prp++;
	if (m_core->prp >= 24)
		m_core->prp = 0;
}

void mb86235_device::decrement_prp()
{
	if (m_core->prp == 0)
		m_core->prp = 24;

	m_core->prp--;
}

void mb86235_device::zero_prp()
{
	m_core->prp = 0;
}

inline uint32_t mb86235_device::decode_ea(uint8_t mode, uint8_t rx, uint8_t ry, uint16_t disp, bool isbbus)
{
	uint32_t res;

	switch(mode)
	{
		case 0x00: // ARx
			return m_core->ar[rx];
		case 0x01: // ARx++
			res = m_core->ar[rx];
			if (m_core->cur_fifo_state.has_stalled == true)
				return res;
			m_core->ar[rx]++;
			m_core->ar[rx] &= 0x3fff;
			return res;
		case 0x02: // ARx--
			res = m_core->ar[rx];
			if (m_core->cur_fifo_state.has_stalled == true)
				return res;
			m_core->ar[rx]--;
			m_core->ar[rx] &= 0x3fff;
			return res;
		case 0x03: // ARx++ disp14
			res = m_core->ar[rx];
			if (m_core->cur_fifo_state.has_stalled == true)
				return res;
			m_core->ar[rx] += disp;
			m_core->ar[rx] &= 0x3fff;
			return res;
		case 0x04: // ARx + ARy
			return m_core->ar[rx] + m_core->ar[ry];
		case 0x05: // ARx + ARy++
			res = m_core->ar[ry];
			if (m_core->cur_fifo_state.has_stalled == true)
				return m_core->ar[rx] + res;
			m_core->ar[ry]++;
			m_core->ar[ry] &= 0x3fff;
			return m_core->ar[rx] + res;
		case 0x06: // ARx + ARy--
			res = m_core->ar[ry];
			if (m_core->cur_fifo_state.has_stalled == true)
				return m_core->ar[rx] + res;
			m_core->ar[ry]--;
			m_core->ar[ry] &= 0x3fff;
			return m_core->ar[rx] + res;
		case 0x07: // ARx + (ARy++ disp14)
			res = m_core->ar[ry];
			if (m_core->cur_fifo_state.has_stalled == true)
				return m_core->ar[rx] + res;
			m_core->ar[ry] += disp;
			m_core->ar[ry] &= 0x3fff;
			return m_core->ar[rx] + res;
		case 0x08: // ARx + ARyU/ARyL (A bus/B bus)
			return m_core->ar[rx] + (isbbus ? (m_core->ar[ry] & 0x7f) : (m_core->ar[ry] >> 7));
		case 0x09: // ARx + ARyL/ARyU (A bus/B bus)
			return m_core->ar[rx] + (isbbus ? (m_core->ar[ry] >> 7) : (m_core->ar[ry] & 0x7f));
		case 0x0a: // ARx + disp14
			return m_core->ar[rx] + disp;
		case 0x0b: // ARx + ARy + disp14
			return m_core->ar[rx] + m_core->ar[ry] + disp;
		case 0x0d: // ARx + [ARy++]
			res = m_core->ar[ry] & (0x1ff >> (7 - (isbbus ? ((m_core->mod >> 8) & 7) : ((m_core->mod >> 12) & 7))));
			if (m_core->cur_fifo_state.has_stalled == true)
				return m_core->ar[rx] + res;
			m_core->ar[ry]++;
			m_core->ar[ry] &= 0x3fff;
			return m_core->ar[rx] + res;
		case 0x0e: // ARx + [ARy--]
			res = m_core->ar[ry] & (0x1ff >> (7 - (isbbus ? ((m_core->mod >> 8) & 7) : ((m_core->mod >> 12) & 7))));
			if (m_core->cur_fifo_state.has_stalled == true)
				return m_core->ar[rx] + res;
			m_core->ar[ry]--;
			m_core->ar[ry] &= 0x3fff;
			return m_core->ar[rx] + res;
		case 0x0f: // ARx + [ARy++ disp14]
			res = m_core->ar[ry] & (0x1ff >> (7 - (isbbus ? ((m_core->mod >> 8) & 7) : ((m_core->mod >> 12) & 7))));
			if (m_core->cur_fifo_state.has_stalled == true)
				return m_core->ar[rx] + res;
			m_core->ar[ry] += disp;
			m_core->ar[ry] &= 0x3fff;
			return m_core->ar[rx] + res;
	}

	fatalerror("TGPx4: illegal decode_ea type %02x executed at pc=%03x\n",mode,m_core->ppc);
	return 0;
}

inline uint32_t mb86235_device::read_bus(bool isbbus, uint32_t addr)
{
	return isbbus == true ? m_datab.read_dword(addr & 0x3ff) : m_dataa.read_dword(addr & 0x3ff);
}

inline void mb86235_device::write_bus(bool isbbus, uint32_t addr, uint32_t data)
{
	if(isbbus == true)
		m_datab.write_dword(addr & 0x3ff,data);
	else
		m_dataa.write_dword(addr & 0x3ff,data);
}


/*********************
 *
 * Instruction fetch
 *
 ********************/

void mb86235_device::execute_op(uint64_t op)
{
	switch((op >> 61) & 7)
	{
		case 0:
			do_alu2_trans2_1(op);
			break;
		case 1:
			do_alu2_trans1_1(op);
			break;
		case 4:
			do_alu1_trans2_2(op);
			break;
		case 5:
			do_alu1_trans1_2(op);
			break;
		case 2:
		case 6:
			do_alu_control(op);
			break;
		case 7:
			do_trans1_3(op);
			break;
		default:
			fatalerror("TGPx4: illegal opcode type %02x executed at pc=%03x\n",(op >> 61) & 7,m_core->ppc);
	}
}

/*********************
 *
 * ALU
 *
 ********************/

inline void mb86235_device::set_alu_flagsd(uint32_t val)
{
	FCLR(AN|AZ);
	if (val & 0x80000000) FSET(AN);
	if (val == 0) FSET(AZ);
}

inline void mb86235_device::set_alu_flagsf(float val)
{
	FCLR(AN|AZ);
	if (val < 0.0f) FSET(AN);
	if (val == 0.0f) FSET(AZ);
}

inline void mb86235_device::set_alu_flagsi(int val)
{
	FCLR(AN|AZ);
	if (val < 0) FSET(AN);
	if (val == 0) FSET(AZ);
}

inline uint32_t mb86235_device::get_prx(uint8_t which)
{
	uint32_t res = m_core->pr[m_core->prp];
	switch (which & 7)
	{
		case 0: break;
		case 1: increment_prp(); break;
		case 2: decrement_prp(); break;
		case 3: zero_prp(); break;
		default: fatalerror("TGPx4: illegal get_prx %02x at pc=%03x\n",which & 7,m_core->ppc); break;
	}

	return res;
}

inline uint32_t mb86235_device::get_constfloat(uint8_t which)
{
	const float float_table[8] = { -1.0f, 0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 5.0f };
	return f2u(float_table[which & 7]);
}

inline uint32_t mb86235_device::get_constint(uint8_t which)
{
	switch (which & 7)
	{
		case 0: // A0
			return 0;
		case 1: // A1
			return 1;
		case 2: // A2
			return 0xffffffff; // -1
	}

	fatalerror("TGPx4: illegal get_constint %02x at pc=%03x\n",which,m_core->ppc);
	return 0;
}

inline uint32_t mb86235_device::get_alureg(uint8_t which, bool isfloatop)
{
	switch (which >> 3)
	{
		case 0: // AAx
			return m_core->aa[which & 7];
		case 1: // ABx
			return m_core->ab[which & 7];
		case 2: // PRx
			return get_prx(which & 7);
		case 3: // constants
			return (isfloatop == true) ? get_constfloat(which & 7) : get_constint(which & 7);
	}

	fatalerror("TGPx4: illegal get_alureg %02x at pc=%03x\n",which,m_core->ppc);
	return 0;
}

inline uint32_t mb86235_device::get_mulreg(uint8_t which, bool isfloatop)
{
	switch (which >> 3)
	{
		case 0: // MAx
			return m_core->ma[which & 7];
		case 1: // MBx
			return m_core->mb[which & 7];
		case 2: // PRx
			return get_prx(which & 7);
		case 3: // constants
			return (isfloatop == true) ? get_constfloat(which & 7) : get_constint(which & 7);
	}

	fatalerror("TGPx4: illegal get_mulreg %02x at pc=%03x\n",which,m_core->ppc);
	return 0;
}

inline void mb86235_device::set_alureg(uint8_t which, uint32_t value)
{
	switch (which >> 3)
	{
		case 0: // MAx
			m_core->ma[which & 7] = value;
			break;
		case 1: // MBx
			m_core->mb[which & 7] = value;
			break;
		case 2: // AAx
			m_core->aa[which & 7] = value;
			break;
		case 3: // ABx
			m_core->ab[which & 7] = value;
			break;
	}
}

inline void mb86235_device::decode_aluop(uint8_t opcode, uint32_t src1, uint32_t src2, uint8_t imm, uint8_t dst_which)
{
	switch(opcode)
	{
		// floating point ops
		case 0x00: // FADD
		case 0x01: // FADDZ
		case 0x02: // FSUB
		case 0x03: // FSUBZ
		{
			float f1 = u2f(src1);
			float f2 = u2f(src2);
			float d;

			if (opcode & 2)
				d = f2-f1;
			else
				d = f1+f2;

			if (opcode & 1)
			{
				FCLR(ZC);
				if(d < 0.0f)
				{
					FSET(ZC);
					d = 0.0f;
				}
			}

			set_alu_flagsf(d);
			set_alureg(dst_which, f2u(d));
			break;
		}

		case 0x04: // FCMP
		case 0x06: // FABC
		{
			float f1 = u2f(src1);
			float f2 = u2f(src2);
			float d;

			if(opcode & 2)
				d = fabs(f2) - fabs(f1);
			else
				d = f2 - f1;

			set_alu_flagsf(d);
			break;
		}

		case 0x05: // FABS
		{
			float d = u2f(src1);
			d = fabs(d);
			set_alu_flagsf(d);
			set_alureg(dst_which, f2u(d));
			break;
		}

		case 0x07: // NOP
			break;

		case 0x08: // FEA
		case 0x09: // FES
		{
			uint32_t exp = (src1 >> 23) & 0xff;
			src1 &= 0x7f800000;
			if(opcode & 1)
				exp -= imm;
			else
				exp += imm;
			exp &= 0xff;
			src1 |= exp << 23;
			set_alu_flagsd(src1);
			set_alureg(dst_which, src1);
			break;
		}

		case 0x0a: // FRCP
		{
			float f = u2f(src1);
			FCLR(ZD);
			if (f == 0.0f)
				FSET(ZD);
			f = 1.0f / f;
			set_alu_flagsf(f);
			set_alureg(dst_which, f2u(f));
			break;
		}

		case 0x0b: // FRSQ
		{
			float f = u2f(src1);
			FCLR(NR);
			if (f <= 0.0f)
				FSET(NR);
			f = 1.0f / sqrtf(f);
			set_alu_flagsf(f);
			set_alureg(dst_which, f2u(f));
			break;
		}

		case 0x0c: // FLOG
		{
			float f = u2f(src1);
			FCLR(IL);
			if(f <= 0.0f)
				FSET(IL);
			f = log(f) / 0.301030f; // log2
			set_alu_flagsf(f);
			set_alureg(dst_which, f2u(f));
			break;
		}

		case 0x0d: // CIF
		{
			int v = (int)src1;
			float f = (float)v;
			set_alu_flagsf(f);
			set_alureg(dst_which,f2u(f));
			break;
		}

		case 0x0e: // CFI
		{
			float f = u2f(src1);
			int v = (int)f;
			set_alu_flagsi(v);
			set_alureg(dst_which, (uint32_t)v);
			break;
		}

		case 0x0f: // CFIB
		{
			float f = u2f(src1);
			uint32_t res = (uint32_t)f;
			if (f < 0.0f)
			{
				FSET(AU);
				res=0;
			};
			FCLR(AZ);
			if (res == 0)
				FSET(AZ);
			if (res > 0xff)
			{
				FSET(AV);
				res=0xff;
			}
			set_alureg(dst_which,res);
			break;
		}

		// integer ops
		case 0x10: // ADD
		case 0x11: // ADDZ
		case 0x12: // SUB
		case 0x13: // SUBZ
		{
			int v1 = (((int)src1) << 0) >> 0;
			int v2 = (((int)src2) << 0) >> 0;
			int res;

			if(opcode & 2)
				res = v2-v1;
			else
				res = v1+v2;

			if(opcode & 1)
			{
				FCLR(ZC);
				if(res < 0)
				{
					FSET(ZC);
					res = 0;
				}
			}
			set_alu_flagsi(res);
			set_alureg(dst_which, (uint32_t)res);
			break;
		}

		case 0x14: // CMP
		{
			int v1 = (((int)src1) << 0) >> 0;
			int v2 = (((int)src2) << 0) >> 0;
			int res = v2 - v1;
			set_alu_flagsi(res);
			break;
		}

		case 0x15: // ABS
		{
			src1 &= 0x7fffffff;
			set_alu_flagsd(src1);
			set_alureg(dst_which, src1);
			break;
		}

		case 0x16: // ATR
		case 0x17: // ATRZ
		{
			if (opcode & 1)
			{
				FCLR(ZC);
				if (src1 & 0x80000000)
				{
					FSET(ZC);
					src1 = 0;
				}
			}
			set_alureg(dst_which, src1);
			break;
		}

		// logical ops
		case 0x18: // AND
		{
			uint32_t res = src1 & src2;
			set_alu_flagsd(res);
			set_alureg(dst_which, res);
			break;
		}

		case 0x19: // OR
		{
			uint32_t res = src1 | src2;
			set_alu_flagsd(res);
			set_alureg(dst_which, res);
			break;
		}

		case 0x1a: // XOR
		{
			uint32_t res = src1 ^ src2;
			set_alu_flagsd(res);
			set_alureg(dst_which, res);
			break;
		}

		case 0x1b: // NOT
		{
			uint32_t res = ~src1;
			set_alu_flagsd(res);
			set_alureg(dst_which, res);
			break;
		}

		case 0x1c: // LSR
		{
			uint32_t res = src1 >> imm;
			set_alu_flagsd(res);
			set_alureg(dst_which, res);
			break;
		}

		case 0x1d: // LSL
		{
			uint32_t res = src1 << imm;
			set_alu_flagsd(res);
			set_alureg(dst_which, res);
			break;
		}

		case 0x1e: // ASR
		{
			int res = ((((int)src1) << 0) >> 0) >> imm;
			set_alu_flagsi(res);
			set_alureg(dst_which, (uint32_t)res);
			break;
		}

		case 0x1f: // ASL
		{
			int res = ((((int)src1) << 0) >> 0) << imm;
			set_alu_flagsi(res);
			set_alureg(dst_which, (uint32_t)res);
			break;
		}

		default:
			fatalerror("TGPx4: illegal decode_aluop %02x at pc=%03x\n",opcode,m_core->ppc);
	}
}

void mb86235_device::decode_mulop(bool isfmul, uint32_t src1, uint32_t src2, uint8_t dst_which)
{
	if(isfmul == true) // FMUL
	{
		float f1 = u2f(src1);
		float f2 = u2f(src2);
		float res = f1 * f2;
		FCLR(MN|MZ|MD); // MV and MU are not reset
		if (res < 0.0f) FSET(MN);
		if (res == 0.0f) FSET(MZ);
		if (std::isinf(res)) FSET(MV);
		if (std::abs(res) < std::numeric_limits<float>::min()) FSET(MU);
		if (std::isnan(res)) FSET(MD);
		set_alureg(dst_which, f2u(res));
	}
	else // MUL
	{
		int v1 = (int)src1;
		int v2 = (int)src2;
		int res = v1 * v2;
		FCLR(MN|MZ);
		if (res < 0) FSET(MN);
		if (res == 0) FSET(MZ);
		set_alureg(dst_which, (uint32_t)res);
	}
}

#define GETAOP(x)  ((x >> 56) & 0x1f)
#define GETAI1(x)  ((x >> 52) & 0x0f)
#define GETAI2(x)  ((x >> 47) & 0x1f)
#define GETAO(x)   ((x >> 42) & 0x1f)

#define GETMOP(x)   ((x >> 41) & 0x01)
#define GETMI1(x)   ((x >> 37) & 0x0f)
#define GETMI2(x)   ((x >> 32) & 0x1f)
#define GETMO(x)    ((x >> 27) & 0x1f)

inline bool mb86235_device::get_alu_second_src(uint8_t which)
{
	if ((which & 0x1c) == 0x1c) // logical ops
		return false;

	if ((which & 0x1e) == 0x16) // ATRx
		return false;

	if ((which & 0x0f) == 0x05) // ABS/FABS
		return false;

	if ((which & 0x18) == 0x08) // floating point ops
		return false;

	return true;
}

void mb86235_device::do_alu1(uint64_t op)
{
	if (m_core->cur_fifo_state.has_stalled == true)
		return;

	if (op & (1ll << 41)) // ALU
	{
		uint8_t aluop = GETAOP(op);
		uint32_t alusrc1 = get_alureg(GETAI1(op),false);
		uint32_t alusrc2;
		if(get_alu_second_src(aluop) == true)
			alusrc2 = get_alureg(GETAI2(op),(aluop & 0x10) == 0);
		else
			alusrc2 = 0;
		decode_aluop(aluop, alusrc1, alusrc2, GETAI2(op), GETAO(op));
	}
	else // MUL
	{
		bool isfmul = GETAOP(op) != 0;
		uint32_t mulsrc1 = get_mulreg(GETAI1(op),false);
		uint32_t mulsrc2 = get_mulreg(GETAI2(op),isfmul);
		decode_mulop(isfmul, mulsrc1, mulsrc2, GETAO(op));
	}
}

void mb86235_device::do_alu2(uint64_t op)
{
	if (m_core->cur_fifo_state.has_stalled == true)
		return;

	// ALU
	uint8_t aluop = GETAOP(op);
	uint32_t alusrc1 = get_alureg(GETAI1(op), false);
	uint32_t alusrc2;
	if (get_alu_second_src(aluop) == true)
		alusrc2 = get_alureg(GETAI2(op), (aluop & 0x10) == 0);
	else
		alusrc2 = 0;

	// MUL
	bool isfmul = GETMOP(op) != 0;
	uint32_t mulsrc1 = get_mulreg(GETMI1(op), false);
	uint32_t mulsrc2 = get_mulreg(GETMI2(op), isfmul);

	decode_aluop(aluop, alusrc1, alusrc2, GETAI2(op), GETAO(op));
	decode_mulop(isfmul, mulsrc1, mulsrc2, GETMO(op));
}

/*********************
 *
 * Transfer types
 *
 ********************/

inline uint32_t mb86235_device::get_transfer_reg(uint8_t which)
{
	switch (which >> 3)
	{
		case 0: // MAx
			return m_core->ma[which & 7];
		case 1: // AAx
			return m_core->aa[which & 7];
		case 2:
			switch (which & 7)
			{
				case 0: return m_core->eb;
				case 1: return m_core->eb >> 14;
				case 2: return m_core->eb & 0x3fff;
				case 3: return m_core->eo;
				case 4: return m_core->sp;
				case 5: return m_core->st;
				case 6: return m_core->mod;
				case 7: return m_core->lpc;
			}

			break;
		case 3: // ARx
			return m_core->ar[which & 7];
		case 4: // MBx
			return m_core->mb[which & 7];
		case 5: // ABx
			return m_core->ab[which & 7];
		case 6:
		{
			switch(which & 7)
			{
				case 0: // PRx
					return m_core->pr[m_core->prp];

				case 1: // FI
				{
					uint32_t res = m_fifoin->pop();
					if (m_core->cur_fifo_state.has_stalled == true)
					{
						if((m_core->st & RP) == 0)
							m_core->cur_fifo_state.pc = m_core->ppc;
						//else
						//  fatalerror("check me %03x\n",m_core->ppc);
					}

					return res;
				}
				case 4: // PDR
					return m_core->pdr;
				case 5: // DDR
					return m_core->ddr;
				case 6: // PRP
					return m_core->prp;
				case 7: // PWP
					return m_core->pwp;
			}
		}
	}

	fatalerror("TGPx4: illegal get_transfer_reg %02x at pc=%03x\n",which,m_core->ppc);
	return 0;
}

inline void mb86235_device::set_transfer_reg(uint8_t which, uint32_t value)
{
	switch(which >> 3)
	{
		case 0: // MAx
			m_core->ma[which & 7] = value;
			break;
		case 1: // AAx
			m_core->aa[which & 7] = value;
			break;
		case 2:
			switch(which & 7)
			{
				case 0: m_core->eb = value; break;
				case 1: m_core->eb = (m_core->eb & 0x3fff) | (value << 14); break;
				case 2: m_core->eb = (m_core->eb & 0xffc000) | (value << 14); break;
				case 3: m_core->eo = value; break;
				case 4: m_core->sp = value; break;
				case 5: m_core->st = value; break;
				case 6: m_core->mod = value; break;
				case 7: m_core->lpc = value; break;
			}
			break;
		case 3: // ARx
			m_core->ar[which & 7] = value & 0x3fff;
			break;
		case 4: // MBx
			m_core->mb[which & 7] = value;
			break;
		case 5: // ABx
			m_core->ab[which & 7] = value;
			break;
		case 6:
			switch(which & 7)
			{
				case 0:
					m_core->pr[m_core->pwp] = value;
					if(m_core->cur_fifo_state.has_stalled == false)
						increment_pwp();
					break;
				case 2: // FO0
				case 3: // FO1 (same FIFO output buffer but sets the 33rd bit; not used by Model 2)
					if(m_fifoout)
						m_fifoout->push(u32(value));
					break;
				case 4: m_core->pdr = value; break;
				case 5: m_core->ddr = value; break;
				case 6:
					if(value >= 24)
						fatalerror("TGPx4: attempting to set prp with a %02x at pc=%03x\n",value,m_core->ppc);

					m_core->prp = value;
					break;
				case 7:
					if(value >= 24)
						fatalerror("TGPx4: attempting to set pwp with a %02x at pc=%03x\n",value,m_core->ppc);

					m_core->pwp = value;
					break;
				default:
					fatalerror("TGPx4: illegal set_transfer_reg %02x at pc=%03x\n",which,m_core->ppc);
					break;
			}

			break;
		default:
			fatalerror("TGPx4: illegal set_transfer_reg dst %02x at pc=%03x\n",which,m_core->ppc);
			break;
	}
}

// double transfer type 1
void mb86235_device::do_alu2_trans2_1(uint64_t op)
{
	uint8_t sd = (op >> 25) & 3;
	uint32_t ares, bres;

	switch (sd)
	{
	case 0:
	case 1:
	{
		uint8_t as = (op >> 20) & 0x1f;
		uint8_t bs = (op >> 10) & 0xf;
		ares = get_transfer_reg(as);
		bres = get_transfer_reg(bs | 0x20);
		break;
	}
	case 2:
	{
		uint32_t addr = decode_ea(op & 0xf, (op >> 17) & 7, (op >> 14) & 7, 0, false);
		ares = read_bus(false, addr);
		addr = decode_ea(op & 0xf, (op >> 7) & 7, (op >> 4) & 7, 0, true);
		bres = read_bus(true, addr);
		break;
	}
	default:
		fatalerror("TGPx4: illegal trans2_1 SD %02x at pc=%03x\n", sd, m_core->ppc);
		break;
	}

	// do alu
	do_alu2(op);

	switch (sd)
	{
	case 0:
	case 2:
	{
		uint8_t ad = (op >> 20) & 0x1f;
		uint8_t bd = (op >> 10) & 0xf;
		set_transfer_reg(ad, ares);
		set_transfer_reg(bd | 0x20, bres);
		break;
	}
	case 1:
	{
		uint32_t addr = decode_ea(op & 0xf, (op >> 17) & 7, (op >> 14) & 7, 0, false);
		write_bus(false, addr, ares);
		addr = decode_ea(op & 0xf, (op >> 7) & 7, (op >> 4) & 7, 0, true);
		write_bus(true, addr, bres);
		break;
	}
	}
}

// transfer type 1
void mb86235_device::do_alu2_trans1_1(uint64_t op)
{
	uint8_t sr,dr;
	uint32_t res;

	if(op & (1ll<<26)) //External transfer
	{
		if(op & (1ll<<25)) // ext -> int
		{
			uint32_t addr = m_core->eb+m_core->eo;
			res = m_dataa.read_dword(addr);

			// do alu
			do_alu2(op);

			dr = (op >> 12) & 0x7f;
			if(dr & 0x40)
			{
				bool isbbus = (dr & 0x20) == 0x20;
				addr = decode_ea(op & 0xf,dr & 7,(op >> 4) & 7, (op >> 7) & 0x1f,isbbus);
				write_bus(isbbus,addr,res);
			}
			else
				set_transfer_reg(dr,res);

			int8_t disp_offs = (op >> 19) & 0x3f;
			if (disp_offs & 0x20)
				disp_offs -= 0x40;
			m_core->eo += disp_offs;
		}
		else // int -> ext
		{
			sr = (op >> 12) & 0x7f;
			if (sr & 0x40)
			{
				bool isbbus = (sr & 0x20) == 0x20;
				uint32_t addr = decode_ea(op & 0xf, sr & 7, (op >> 4) & 7, (op >> 7) & 0x1f, isbbus);
				res = read_bus(isbbus, addr);
			}
			else
				res = get_transfer_reg(sr);

			// do alu
			do_alu2(op);

			uint32_t addr = m_core->eb + m_core->eo;
			m_dataa.write_dword(addr, res);

			int8_t disp_offs = (op >> 19) & 0x3f;
			if (disp_offs & 0x20)
				disp_offs -= 0x40;
			m_core->eo += disp_offs;
		}
	}
	else
	{
		sr = (op>>19) & 0x7f;
		if(sr & 0x40)
		{
			if (sr == 0x58)
				res = op & 0xfff;
			else
			{
				bool isbbus = (sr & 0x20) == 0x20;
				uint32_t addr = decode_ea(op & 0xf,sr & 7,(op >> 4) & 7, (op >> 7) & 0x1f,isbbus);
				res = read_bus(isbbus,addr);
			}
		}
		else
			res = get_transfer_reg(sr);

		// do alu
		do_alu2(op);

		dr = (op >> 12) & 0x7f;
		if(dr & 0x40)
		{
			if(dr == 0x58)
				fatalerror("TGPx4: illegal do_alu2_trans1_1 dr == 0x58 at pc=%03x\n",m_core->ppc);

			bool isbbus = (dr & 0x20) == 0x20;
			uint32_t addr = decode_ea(op & 0xf,dr & 7,(op >> 4) & 7, (op >> 7) & 0x1f,isbbus);
			write_bus(isbbus,addr,res);
		}
		else
			set_transfer_reg(dr,res);
	}
}

// double transfer type 2
void mb86235_device::do_alu1_trans2_2(uint64_t op)
{
	uint8_t sda = (op >> 38) & 3;
	uint8_t sdb = (op >> 18) & 3;
	uint32_t ares, bres;

	// A bus read
	switch(sda)
	{
	case 0:
	case 1:
	{
		uint8_t as = (op >> 33) & 0x1f;
		ares = get_transfer_reg(as);
		break;
	}
	case 2:
	{
		uint32_t addr = decode_ea((op >> 20) & 0xf, (op >> 30) & 7, (op >> 27) & 7, (op >> 24) & 7, false);
		ares = read_bus(false, addr);
		break;
	}
	default:
		fatalerror("TGPx4: illegal trans2_2 SDA %02x at pc=%03x\n", sda, m_core->ppc);
		break;
	}

	// B bus read
	switch(sdb)
	{
	case 0:
	case 1:
	{
		uint8_t bs = (op >> 13) & 0x1f;
		bres = get_transfer_reg(bs | 0x20);
		break;
	}
	case 2:
	{
		uint32_t addr = decode_ea(op & 0xf, (op >> 10) & 7, (op >> 7) & 7, (op >> 4) & 7, true);
		bres = read_bus(true, addr);
		break;
	}
	default:
		fatalerror("TGPx4: illegal trans2_2 SDB %02x at pc=%03x\n", sdb, m_core->ppc);
		break;
	}

	// do alu
	do_alu1(op);

	// A bus write
	switch (sda)
	{
	case 0:
	case 2:
	{
		uint8_t ad = (op >> 28) & 0x1f;
		set_transfer_reg(ad, ares);
		break;
	}
	case 1:
	{
		uint32_t addr = decode_ea((op >> 20) & 0xf, (op >> 30) & 7, (op >> 27) & 7, (op >> 24) & 7, false);
		write_bus(false, addr, ares);
		break;
	}
	}

	// B bus write
	switch (sdb)
	{
	case 0:
	case 2:
	{
		uint8_t bd = (op >> 8) & 0x1f;
		set_transfer_reg(bd | 0x20, bres);
		break;
	}
	case 1:
	{
		uint32_t addr = decode_ea(op & 0xf, (op >> 10) & 7, (op >> 7) & 7, (op >> 4) & 7, true);
		write_bus(true, addr, bres);
		break;
	}
	}
}

// transfer type 2
void mb86235_device::do_alu1_trans1_2(uint64_t op)
{
	uint8_t sr, dr;
	uint32_t res;

	if(op & 1ll<<38) // external transfer
	{
		if(op & 1ll<<37) // ext->int
		{
			uint32_t addr = m_core->eb+m_core->eo;
			uint32_t res = m_dataa.read_dword(addr);

			// do alu
			do_alu1(op);

			dr = (op >> 24) & 0x7f;
			if(dr & 0x40)
			{
				bool isbbus = (dr & 0x20) == 0x20;
				addr = decode_ea(op & 0xf, dr & 7, (op >> 4) & 7, (op >> 7) & 0x3fff, isbbus);
				write_bus(isbbus, addr, res);
			}
			else
				set_transfer_reg(dr,res);

			int8_t disp_offs = (op >> 31) & 0x3f;
			if (disp_offs & 0x20)
				disp_offs -= 0x40;
			m_core->eo += disp_offs;
		}
		else // int->ext
		{
			sr = (op >> 24) & 0x7f;
			if(sr & 0x40)
			{
				if(sr == 0x58)
					res = op & 0xffffff;
				else
				{
					bool isbbus = (sr & 0x20) == 0x20;
					uint32_t addr = decode_ea(op & 0xf, sr & 7, (op >> 4) & 7, (op >> 7) & 0x3fff, isbbus);
					res = read_bus(isbbus, addr);
				}
			}
			else
				res = get_transfer_reg(sr);

			// do alu
			do_alu1(op);

			uint32_t addr = m_core->eb + m_core->eo;
			m_dataa.write_dword(addr,res);

			int8_t disp_offs = (op >> 31) & 0x3f;
			if (disp_offs & 0x20)
				disp_offs -= 0x40;
			m_core->eo += disp_offs;
		}
	}
	else
	{
		sr = (op >> 31) & 0x7f;
		if(sr & 0x40)
		{
			if(sr == 0x58)
				res = op & 0xffffff;
			else
			{
				bool isbbus = (sr & 0x20) == 0x20;
				uint32_t addr = decode_ea(op & 0xf,sr & 7,(op >> 4) & 7, (op >> 7) & 0x3fff,isbbus);
				res = read_bus(isbbus,addr);
			}
		}
		else
			res = get_transfer_reg(sr);

		// do alu
		do_alu1(op);

		dr = (op >> 24) & 0x7f;
		if(dr & 0x40)
		{
			if(dr == 0x58)
				fatalerror("TGPx4: illegal do_alu1_trans1_2 dr == 0x58 at pc=%03x\n",m_core->ppc);

			bool isbbus = (dr & 0x20) == 0x20;
			uint32_t addr = decode_ea(op & 0xf,dr & 7,(op >> 4) & 7, (op >> 7) & 0x3fff,isbbus);
			write_bus(isbbus,addr,res);
		}
		else
			set_transfer_reg(dr,res);
	}
}

// transfer type 3
void mb86235_device::do_trans1_3(uint64_t op)
{
	uint8_t dr   = (op >> 19) & 0x7f;
	uint32_t imm = (op >> 27) & 0xffffffff;

	if(dr & 0x40)
	{
		bool isbbus = (dr & 0x20) == 0x20;
		uint32_t addr = decode_ea(op & 0xf,dr & 7,(op >> 4) & 7, (op >> 7) & 0xfff,isbbus);
		write_bus(isbbus,addr,imm);
	}
	else // direct imm reg
		set_transfer_reg(dr,imm);
}

/*********************
 *
 * Control
 *
 ********************/

inline void mb86235_device::push_pc(uint32_t pcval)
{
	m_core->pcs[m_core->pcp++] = pcval;
	m_core->pcp &= 3;
//  if(m_core->pcp & ~3)
//      fatalerror("TGPx4: push_pc overflow PCP=%08x PC=%03x\n",m_core->pcp,m_core->ppc);
}

inline uint32_t mb86235_device::pop_pc()
{
	m_core->pcp--;
	m_core->pcp &= 3;
//  if(m_core->pcp & ~3)
//      fatalerror("TGPx4: pop_pc underflow PCP=%08x PC=%03x\n",m_core->pcp,m_core->ppc);

	return m_core->pcs[m_core->pcp];
}

inline uint32_t mb86235_device::do_control_dst(uint64_t op)
{
	uint32_t addr = 0;

	switch ((op >> 13) & 7)
	{
	case 0: // immediate
		addr = op & 0xfff;
		break;
	case 1: // register ARx
		addr = m_core->ar[(op >> 6) & 7];
		break;
	case 2:
		addr = (op & 1ll << 11) ? m_core->ab[(op >> 6) & 7] : m_core->aa[(op >> 6) & 7];
		break;
	case 3:
		addr = (op & 1ll << 11) ? m_core->mb[(op >> 6) & 7] : m_core->ma[(op >> 6) & 7];
		break;
	case 4:
		addr = read_bus(false, op & 0x3ff);
		break;
	case 5:
		addr = read_bus(true, op & 0x3ff);
		break;
	case 6:
		addr = read_bus(false, m_core->ar[(op >> 6) & 7]);
		break;
	case 7:
		addr = read_bus(true, m_core->ar[(op >> 6) & 7]);
		break;
	}

	if (op & 1ll << 12)
	{
		m_core->icount--;
		return (m_core->pc + addr) & 0xfff;
	}
	else
		return addr & 0xfff;
}

inline void mb86235_device::set_mod(uint16_t mod1, uint16_t mod2)
{
	m_core->mod &= ~mod1;
	m_core->mod |= mod2;
}

inline bool mb86235_device::decode_branch_jump(uint8_t which)
{
	if (which < 19)
	{
		const uint32_t condition_table[19] = { MN, MZ, MV, MU, ZD, NR, IL, ZC, AN, AZ,
											   AV, AU, MD, AD, 0, 0, F0, F1, F2 };

		return (m_core->st & condition_table[which]) != 0;
	}
	else switch (which)
	{
	case 20: // IFF
		return m_fifoin->is_full();
	case 21: // IFE
		return m_fifoin->is_empty();
	case 22: // OFF
		return m_fifoout->is_full();
	case 23: // OFE
		return m_fifoout->is_empty();
	case 24: // IF
		return false;   // bit 33 of input FIFO; not used by Model 2
	default:
		fatalerror("TGPx4: illegal decode_branch_jump mode %02x at pc=%03x\n", which, m_core->ppc);
	}


	return false;
}

void mb86235_device::do_alu_control(uint64_t op)
{
	uint32_t cop = (op >> 22) & 0x1f;
	uint32_t ef1 = (op >> 16) & 0x3f;
	uint16_t ef2 = op & 0xffff;

	switch(cop)
	{
		case 0x00: // NOP
			break;
		case 0x01: // REP
			if(ef1 == 0x3f)
				m_core->rpc = m_core->ar[(ef2 >> 13) & 7];
			else
				m_core->rpc = ef2;

			FSET(RP); // set repeat flag
			break;
		case 0x02: // SETL
			if(ef1 == 0x3f)
				m_core->lpc = m_core->ar[(ef2 >> 13) & 7];
			else
				m_core->lpc = ef2;

			FSET(LP); // set repeat flag
			break;
		case 0x03: // CLRF
			if(ef1 & 1) // clear fifo in (CLRFI)
			{
				if(m_fifoin)
					m_fifoin->clear();
			}
			if(ef1 & 2) // clear fifo out (CLRFO)
			{
				if(m_fifoout)
					m_fifoout->clear();
			}
			break;
		case 0x04: // PUSH
			m_core->sp--;
			m_core->sp &= 0x3ff;
			m_datab.write_dword(m_core->sp, get_transfer_reg((ef2>>6) & 0x3f));
			break;
		case 0x05: // POP
			// wait until ALU operation(s) have finished before popping
			break;
		case 0x08: // SETM
			set_mod(0xffff, ef2);
			break;
		case 0x09: // SETMCBSA
			set_mod(0x7000, ef2);
			break;
		case 0x0a: // SETMCBSB
			set_mod(0x0e00, ef2);
			break;
		case 0x0b: // SETMRF
			set_mod(0x0080, ef2);
			break;
		case 0x0c: // SETMRDY
			set_mod(0x0010, ef2);
			break;
		case 0x0d: // SETMWAIT
			set_mod(0x0007, ef2);
			break;

		// control flow
		case 0x10: // DBcc
		{
			bool result = decode_branch_jump(ef1);
			if(result == true)
			{
				m_core->delay_slot = true;
				m_core->delay_pc = do_control_dst(op);
			}
			m_core->icount--;
			break;
		}
		case 0x11: // DBNcc
		{
			bool result = decode_branch_jump(ef1);
			if(result == false)
			{
				m_core->delay_slot = true;
				m_core->delay_pc = do_control_dst(op);
			}
			m_core->icount--;
			break;
		}
		case 0x12: // DJMP
		{
			m_core->delay_slot = true;
			m_core->delay_pc = do_control_dst(op);
			m_core->icount--;
			break;
		}
		case 0x13: // DBLP
		{
			if(m_core->st & LP)
			{
				m_core->delay_slot = true;
				// relative addressing only
				m_core->delay_pc = m_core->pc + (op & 0xfff);
				m_core->delay_pc&= 0xfff;
			}

			--m_core->lpc;
			if(m_core->lpc == 1)
				FCLR(LP);
			break;
		}
		case 0x14: // DBBC
		{
			bool result = m_core->ar[(op >> 13) & 7] & (1 << ((op >> 16) & 0xf));
			if (result == false)
			{
				m_core->delay_slot = true;
				m_core->delay_pc = m_core->pc + (op & 0xfff);
				m_core->delay_pc &= 0xfff;
			}
			m_core->icount -= 2;
			break;
		}
		case 0x15: // DBBS
		{
			bool result = m_core->ar[(op >> 13) & 7] & (1 << ((op >> 16) & 0xf));
			if (result == true)
			{
				m_core->delay_slot = true;
				m_core->delay_pc = m_core->pc + (op & 0xfff);
				m_core->delay_pc &= 0xfff;
			}
			m_core->icount -= 2;
			break;
		}
		case 0x18: // DCcc
		{
			bool result = decode_branch_jump(ef1);
			if(result == true)
			{
				m_core->delay_slot = true;
				m_core->delay_pc = do_control_dst(op);
				push_pc(m_core->pc+1);
			}
			m_core->icount--;
			break;
		}
		case 0x19: // DCNcc
		{
			bool result = decode_branch_jump(ef1);
			if(result == false)
			{
				m_core->delay_slot = true;
				m_core->delay_pc = do_control_dst(op);
				push_pc(m_core->pc+1);
			}
			m_core->icount--;
			break;
		}
		case 0x1a: // DCALL
			m_core->delay_slot = true;
			m_core->delay_pc = do_control_dst(op);
			push_pc(m_core->pc+1);
			break;
		case 0x1b: // DRET
			m_core->delay_slot = true;
			m_core->delay_pc = pop_pc();
			break;
		default:
			fatalerror("TGPx4: illegal control op %02x at pc=%03x\n", cop, m_core->ppc);
	}

	// do alu
	if (op & (1ll << 63))
		do_alu1(op);
	else
		do_alu2(op);

	// now we can safely pop if needed
	if (cop == 0x05)
	{
		set_transfer_reg((ef2 >> 6) & 0x3f, m_datab.read_dword(m_core->sp));
		m_core->sp++;
		m_core->sp &= 0x3ff;
	}
}


