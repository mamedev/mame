// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb
#ifndef MAME_CPU_E132XS_E132XSDRC_OPS_HXX
#define MAME_CPU_E132XS_E132XSDRC_OPS_HXX

#pragma once

#include "e132xs.h"

constexpr uint32_t WRITE_ONLY_REGMASK = (1 << BCR_REGISTER) | (1 << TPR_REGISTER) | (1 << FCR_REGISTER) | (1 << MCR_REGISTER);

uint32_t hyperstone_device::generate_get_const(const opcode_desc *desc)
{
	const uint16_t imm_1 = m_pr16(desc->pc + 2);

	if (imm_1 & 0x8000)
	{
		const uint16_t imm_2 = m_pr16(desc->pc + 4);

		uint32_t imm = imm_2 | (uint32_t(imm_1 & 0x3fff) << 16);

		if (imm_1 & 0x4000)
			imm |= 0xc0000000;

		return imm;
	}
	else
	{
		uint32_t imm = imm_1 & 0x3fff;

		if (imm_1 & 0x4000)
			imm |= 0xffffc000;

		return imm;
	}
}

uint32_t hyperstone_device::generate_get_immediate_s(const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];

	switch (op & 0xf)
	{
		case 0:
			return 16;
		case 1:
			return (uint32_t(m_pr16(desc->pc + 2)) << 16) | m_pr16(desc->pc + 4);
		case 2:
			return m_pr16(desc->pc + 2);
		case 3:
			return 0xffff0000 | m_pr16(desc->pc + 2);
		default:
			return s_immediate_values[op & 0xf];
	}
}

uint32_t hyperstone_device::generate_get_pcrel(const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];

	if (op & 0x80)
	{
		const uint16_t next = m_pr16(desc->pc + 2);
		uint32_t offset = (uint32_t(op & 0x7f) << 16) | (next & 0xfffe);
		if (next & 1)
			offset |= 0xff800000;

		return offset;
	}
	else
	{
		uint32_t offset = op & 0x7e;
		if (op & 1)
			offset |= 0xffffff80;

		return offset;
	}
}

std::pair<uint16_t, uint32_t> hyperstone_device::generate_get_d_code_dis(const opcode_desc *desc)
{
	const uint16_t next_1 = m_pr16(desc->pc + 2);
	const uint16_t d_code = (next_1 & 0x3000) >> 12;

	uint32_t dis;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = m_pr16(desc->pc + 4);

		dis = next_2 | (uint32_t(next_1 & 0x0fff) << 16);

		if (next_1 & 0x4000)
			dis |= 0xf0000000;
	}
	else
	{
		dis = next_1 & 0xfff;

		if (next_1 & 0x4000)
			dis |= 0xfffff000;
	}

	return std::make_pair(d_code, dis);
}

inline void hyperstone_device::generate_add_dis(drcuml_block &block, compiler_state &compiler, uml::parameter dst, uml::parameter base, uint32_t dis, unsigned alignment)
{
	const uint32_t mask = ~uint32_t(alignment - 1);
	if (base.is_immediate())
	{
		UML_MOV(block, dst, (base.immediate() + (dis & mask)) & mask);
	}
	else if (dis)
	{
		UML_ADD(block, dst, base, dis & mask);
		if (alignment > 1)
			UML_AND(block, dst, dst, mask);
	}
	else if (alignment > 1)
	{
		UML_AND(block, dst, base, mask);
	}
	else if (dst != base)
	{
		UML_MOV(block, dst, base);
	}
}


void hyperstone_device::generate_get_global_register_high(drcuml_block &block, compiler_state &compiler, uint32_t code, uml::parameter dst)
{
	// Expects cycles in I7 (cleared after use)
	code |= 0x10;
	if (code == TR_REGISTER)
	{
		UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I7);
		UML_MOV(block, I7, 0);

		UML_SHR(block, dst, mem(&m_core->tr_clocks_per_tick), 1);
		UML_CMP(block, mem(&m_core->icount), dst);
		UML_MOVc(block, uml::COND_BE, dst, 0);
		UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), dst);

		UML_CALLC(block, &c_funcs::compute_tr, this);
		UML_MOV(block, dst, mem(&m_core->tr_result));
	}
	else
	{
		UML_MOV(block, dst, mem(&m_core->global_regs[code]));
	}
}

void hyperstone_device::generate_set_global_register_low(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t dst_code, uml::parameter src)
{
	// clobbers I4 when setting SR
	if (dst_code == PC_REGISTER)
	{
		UML_AND(block, DRC_PC, src, ~uint32_t(1));
	}
	else if (dst_code == SR_REGISTER)
	{
		UML_MOV(block, I4, DRC_SR);
		if (compiler.supervisor_mode() || (src.is_immediate() && !(src.immediate() & L_MASK)))
		{
			UML_ROLINS(block, I4, src, 0, 0x0000ffff);
			UML_AND(block, I4, I4, ~0x40); // keep reserved bit clear
		}
		else
		{
			const int no_exception = compiler.next_label();
			UML_TEST(block, I4, L_MASK);
			UML_JMPc(block, uml::COND_NZ, no_exception);
			if (src.is_immediate())
			{
				UML_ROLINS(block, I4, src.immediate() & ~0x40, 0, 0x0000ffff); // keep reserved bit clear
			}
			else
			{
				UML_TEST(block, src, L_MASK);
				UML_JMPc(block, uml::COND_Z, no_exception);
				UML_ROLINS(block, I4, src, 0, 0x0000ffff);
				UML_AND(block, I4, I4, ~0x40); // keep reserved bit clear
			}
			generate_raise_exception(block, compiler, desc, TRAPNO_PRIVILEGE_ERROR, uml::I4);
			UML_LABEL(block, no_exception);
			if (src.is_immediate())
			{
				UML_ROLINS(block, I4, src.immediate() & ~0x40, 0, 0x0000ffff); // keep reserved bit clear
			}
			else
			{
				UML_ROLINS(block, I4, src, 0, 0x0000ffff);
				UML_AND(block, I4, I4, ~0x40); // keep reserved bit clear
			}
		}
		UML_MOV(block, DRC_SR, I4);
	}
	else
	{
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
	}
}

void hyperstone_device::generate_set_global_register_high(drcuml_block &block, compiler_state &compiler, uint32_t dst_code, uml::parameter src)
{
	// Expects cycles in I7 (cleared after use), clobbers I6
	dst_code |= 0x10;

	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I7);
	UML_MOV(block, I7, 0);
	switch (dst_code)
	{
	case 16:            // G16 reserved
	case 17:            // G17 reserved
	case WCR_REGISTER:  // G24 Watchdog Compare Register
	case 28:            // G28 reserved
	case 29:            // G29 reserved
	case 30:            // G30 reserved
	case 31:            // G31 reserved
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
		break;
	case SP_REGISTER:   // G18 Stack Pointer
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
		break;
	case UB_REGISTER:   // G19 Upper Stack Bound
		UML_AND(block, mem(&m_core->global_regs[dst_code]), src, ~uint32_t(3));
		break;
	case BCR_REGISTER:  // G20 Bus Control Register
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
		UML_CALLC(block, &c_funcs::update_bus_control, this);
		break;
	case TPR_REGISTER:  // G21 Timer Prescaler Register
		{
			const int skip_compute_tr = compiler.next_label();
			UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
			UML_TEST(block, src, 0x80000000);
			UML_JMPc(block, uml::COND_NZ, skip_compute_tr);
			UML_CALLC(block, &c_funcs::compute_tr, this);
			UML_CALLC(block, &c_funcs::update_timer_prescale, this);
			UML_LABEL(block, skip_compute_tr);
			UML_CALLC(block, &c_funcs::adjust_timer_interrupt, this);
		}
		break;
	case TCR_REGISTER:  // G22 Timer Compare Register
		{
			const int done = compiler.next_label();
			UML_MOV(block, I6, mem(&m_core->global_regs[dst_code]));
			UML_CMP(block, I6, src);
			UML_JMPc(block, uml::COND_E, done);
			UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
			UML_CALLC(block, &c_funcs::adjust_timer_interrupt, this);
			UML_LABEL(block, done);
		}
		break;
	case TR_REGISTER:   // G23 Timer Register
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
		UML_MOV(block, mem(&m_core->tr_base_value), src);
		UML_CALLC(block, &c_funcs::total_cycles, this);
		UML_DMOV(block, mem(&m_core->tr_base_cycles), mem(&m_core->numcycles));
		UML_CALLC(block, &c_funcs::adjust_timer_interrupt, this);
		break;
	case ISR_REGISTER:  // G25 Input Status Register (read-only)
		break;
	case FCR_REGISTER:  // G26 Function Control Register
		{
			const int skip_adjust_timer = compiler.next_label();
			UML_MOV(block, I6, mem(&m_core->global_regs[dst_code]));
			UML_XOR(block, I6, I6, src);
			UML_TEST(block, I6, 0x80000000);
			UML_JMPc(block, uml::COND_Z, skip_adjust_timer);
			UML_CALLC(block, &c_funcs::adjust_timer_interrupt, this);
			UML_LABEL(block, skip_adjust_timer);
			UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
		}
		break;
	case MCR_REGISTER:  // G27 Memory Control Register
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), src);
		UML_CALLC(block, &c_funcs::update_memory_control, this);
		UML_TEST(block, mem(&m_core->powerdown), ~uint32_t(0));
		UML_CALLHc(block, uml::COND_NZ, *m_eat_all_cycles);
		break;
	default:
		throw emu_fatalerror("%s: invalid high global register G%u\n", dst_code);
	}
}


void hyperstone_device::generate_load_operand(drcuml_block &block, compiler_state &compiler, reg_bank global, uint32_t code, uml::parameter dst, uml::parameter localidx)
{
	// expects frame pointer in I3 if local
	// sets localidx if local before setting dst
	if (global)
	{
		UML_MOV(block, dst, mem(&m_core->global_regs[code]));
	}
	else
	{
		UML_ADD(block, localidx, I3, code);
		UML_AND(block, localidx, localidx, 0x3f);
		UML_LOAD(block, dst, (void *)m_core->local_regs, localidx, SIZE_DWORD, SCALE_x4);
	}
}

void hyperstone_device::generate_load_src_addsub(drcuml_block &block, compiler_state &compiler, reg_bank global, uint32_t code, uml::parameter dst, uml::parameter localidx, uml::parameter sr)
{
	// expects frame pointer in I3 if local
	// sets localidx if local before setting dst
	if (global)
	{
		if (code == SR_REGISTER)
			UML_AND(block, dst, sr, C_MASK);
		else
			UML_MOV(block, dst, mem(&m_core->global_regs[code]));
	}
	else
	{
		UML_ADD(block, localidx, I3, code);
		UML_AND(block, localidx, localidx, 0x3f);
		UML_LOAD(block, dst, (void *)m_core->local_regs, localidx, SIZE_DWORD, SCALE_x4);
	}
}

uml::parameter hyperstone_device::generate_load_address_ad(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, reg_bank global, uint32_t code, uml::parameter dst, uml::parameter localidx)
{
	// expects frame pointer in I3 if local
	// sets localidx if local before setting dst
	// returns an immediate for SR or constant PC, otherwise returns dst

	if (global)
	{
		if (code == SR_REGISTER)
		{
			return 0;
		}
		else if ((code == PC_REGISTER) && ((desc->flags & OPFLAG_IN_DELAY_SLOT) || !compiler.check_delay()))
		{
			if (!compiler.pc())
				generate_raise_exception(block, compiler, desc, TRAPNO_POINTER_ERROR);
			return compiler.pc();
		}
		else
		{
			UML_MOV(block, dst, mem(&m_core->global_regs[code]));
		}
	}
	else
	{
		UML_ADD(block, localidx, I3, code);
		UML_AND(block, localidx, localidx, 0x3f);
		UML_LOAD(block, dst, (void *)m_core->local_regs, localidx, SIZE_DWORD, SCALE_x4);
	}

	const int no_exception = compiler.next_label();
	UML_TEST(block, dst, ~uint32_t(0));
	UML_JMPc(block, uml::COND_NZ, no_exception);
	generate_raise_exception(block, compiler, desc, TRAPNO_POINTER_ERROR);
	UML_LABEL(block, no_exception);

	return dst;
}

void hyperstone_device::generate_load_address_ns(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, reg_bank global, uint32_t code, uml::parameter dst, uml::parameter localidx, uint16_t d_code, uint32_t dis)
{
	// expects frame pointer in I3 if local
	// sets localidx if local before setting dst

	generate_load_operand(block, compiler, global, code, dst, localidx);

	const int no_exception = compiler.next_label();
	UML_TEST(block, dst, ~uint32_t(0));
	UML_JMPc(block, uml::COND_NZ, no_exception);

	switch (d_code)
	{
		case 0: // LDBS.N
		case 1: // LDBU.N
			UML_ADD(block, dst, dst, dis);
			break;
		case 2: // LDHS.N, LDHU.N
			UML_ADD(block, dst, dst, dis & ~uint32_t(1));
			break;
		case 3: // LDW.N, LDD.N, LDW.S
			UML_ADD(block, dst, dst, dis & ~uint32_t(3));
			break;
	}
	if (global)
		UML_MOV(block, mem(&m_core->global_regs[code]), dst);
	else
		UML_STORE(block, (void *)m_core->local_regs, localidx, dst, SIZE_DWORD, SCALE_x4);
	generate_raise_exception(block, compiler, desc, TRAPNO_POINTER_ERROR);

	UML_LABEL(block, no_exception);
}

void hyperstone_device::generate_load_address_rp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t code, uml::parameter dst, uml::parameter localidx, uint32_t dis)
{
	// expects frame pointer in I3 if local
	// sets I0 to masked address
	// sets localidx if local before setting dst
	// localidx must not conflict with dst if dis is non-zero

	generate_load_operand(block, compiler, LOCAL, code, dst, localidx);

	const int no_exception = compiler.next_label();
	UML_TEST(block, dst, ~uint32_t(0));
	UML_JMPc(block, uml::COND_NZ, no_exception);
	if (dis)
	{
		UML_ADD(block, dst, dst, dis);
		UML_STORE(block, (void *)m_core->local_regs, localidx, dst, SIZE_DWORD, SCALE_x4);
	}
	generate_raise_exception(block, compiler, desc, TRAPNO_POINTER_ERROR);
	UML_LABEL(block, no_exception);

	UML_AND(block, I0, dst, ~uint32_t(3));
}


void hyperstone_device::generate_set_register(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, reg_bank global, uint32_t code, uml::parameter src, uml::parameter localidx, bool calcidx)
{
	// expects frame pointer in I3 if local and calcidx is true
	// sets localidx if local and calcidx is true before storing src
	// localidx is input if local and calcidx is false
	// clobbers I4 when setting SR
	if (global)
	{
		generate_set_global_register_low(block, compiler, desc, code, src);
	}
	else
	{
		if (calcidx)
		{
			UML_ADD(block, localidx, I3, code);
			UML_AND(block, localidx, localidx, 0x3f);
		}
		UML_STORE(block, (void *)m_core->local_regs, localidx, src, SIZE_DWORD, SCALE_x4);
	}
}

void hyperstone_device::generate_set_dst(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, reg_bank global, uint32_t code, uml::parameter src, uml::parameter localidx, bool calcidx)
{
	// expects frame pointer in I3 if local and calcidx is true
	// sets localidx if local and calcidx is true before storing src
	// localidx is input if local and calcidx is false
	// clobbers I4 when setting SR
	if (global)
	{
		generate_set_global_register_low(block, compiler, desc, code, src);
		if (code == PC_REGISTER)
		{
			UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
			if (src.is_int_register() && (desc->targetpc == BRANCH_TARGET_DYNAMIC))
			{
				UML_AND(block, src, src, ~uint32_t(1));
				generate_branch(block, compiler, compiler.mode(), src, desc);
			}
			else if (src.is_immediate() && (desc->targetpc == BRANCH_TARGET_DYNAMIC))
			{
				generate_branch(block, compiler, compiler.mode(), src.immediate() & ~uint32_t(1), desc);
			}
			else
			{
				generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
			}
		}
	}
	else
	{
		if (calcidx)
		{
			UML_ADD(block, localidx, I3, code);
			UML_AND(block, localidx, localidx, 0x3f);
		}
		UML_STORE(block, (void *)m_core->local_regs, localidx, src, SIZE_DWORD, SCALE_x4);
	}
}


void hyperstone_device::generate_update_flags_addsub(drcuml_block &block, compiler_state &compiler, uml::parameter sr)
{
	// expects result in I0 and UML flags set by ADD/SUB
	// clobbers I1, I4 and I5

	UML_SETc(block, uml::COND_V, I4);               // I4 = ...V
	UML_SETc(block, uml::COND_Z, I5);               // I5 = ...Z
	UML_SETc(block, uml::COND_C, I1);               // I1 = ...C
	UML_SHL(block, I4, I4, V_SHIFT);                // I4 = V...
	UML_OR(block, I1, I1, I4);                      // I1 = V..C
	UML_SHL(block, I4, I5, Z_SHIFT);                // I4 = ..Z.
	UML_OR(block, I1, I1, I4);                      // I1 = V.ZC
	UML_ROLAND(block, I4, I0, N_SHIFT + 1, N_MASK); // I4 = .N..
	UML_OR(block, I1, I1, I4);                      // I1 = VNZC

	UML_ROLINS(block, sr, I1, 0, (V_MASK | N_MASK | Z_MASK | C_MASK));
}

void hyperstone_device::generate_update_flags_addsubc(drcuml_block &block, compiler_state &compiler, uml::parameter sr)
{
	// expects result in I0 and UML flags set by ADD/SUB
	// clobbers I1, I4 and I5

	UML_SETc(block, uml::COND_V, I4);               // I4 = ...V
	UML_SETc(block, uml::COND_Z, I5);               // I5 = ...Z
	UML_SETc(block, uml::COND_C, I1);               // I1 = ...C
	UML_SHL(block, I4, I4, V_SHIFT);                // I4 = V...
	UML_OR(block, I1, I1, I4);                      // I1 = V..C
	UML_SHL(block, I4, I5, Z_SHIFT);                // I4 = ..Z.
	UML_OR(block, I1, I1, I4);                      // I1 = V.ZC
	UML_ROLAND(block, I4, I0, N_SHIFT + 1, N_MASK); // I4 = .N..
	UML_OR(block, I1, I1, I4);                      // I1 = VNZC
	UML_OR(block, I4, I2, ~Z_MASK);                 // combine with old Z flag
	UML_AND(block, I1, I1, I4);

	UML_ROLINS(block, sr, I1, 0, (V_MASK | N_MASK | Z_MASK | C_MASK));
}

void hyperstone_device::generate_update_flags_addsubs(drcuml_block &block, compiler_state &compiler, uml::parameter sr)
{
	// expects UML flags set by ADD/SUB
	// clobbers I1, I4 and I5

	UML_SETc(block, uml::COND_V, I4);       // I4 = ...V
	UML_SETc(block, uml::COND_S, I1);       // I1 = ...S
	UML_SETc(block, uml::COND_Z, I5);       // I5 = ...Z
	UML_SHL(block, I4, I4, V_SHIFT);        // I4 = V...
	UML_SHL(block, I1, I1, N_SHIFT);        // I1 = .N..
	UML_OR(block, I1, I1, I4);              // I1 = VN..
	UML_SHL(block, I4, I5, Z_SHIFT);        // I4 = ..Z.
	UML_OR(block, I1, I1, I4);              // I1 = VNZ.

	UML_ROLINS(block, sr, I1, 0, (V_MASK | N_MASK | Z_MASK));
}

void hyperstone_device::generate_update_flags_cmp(drcuml_block &block, compiler_state &compiler, uml::parameter sr)
{
	// expects UML flags set by ADD/SUB
	// clobbers I0, I1, I3 and I4

	UML_SETc(block, uml::COND_V, I0);               // I0 = ...V
	UML_SETc(block, uml::COND_L, I1);               // I1 = ...N
	UML_SETc(block, uml::COND_Z, I3);               // I3 = ...Z
	UML_SETc(block, uml::COND_C, I4);               // I4 = ...C
	UML_AND(block, sr, sr, ~(V_MASK | N_MASK | Z_MASK | C_MASK));
	UML_SHL(block, I0, I0, V_SHIFT);                // I0 = V...
	UML_SHL(block, I1, I1, N_SHIFT);                // I1 = .N..
	UML_SHL(block, I3, I3, Z_SHIFT);                // I3 = ..Z.
	UML_SHL(block, I4, I4, C_SHIFT);                // I4 = ...C
	UML_OR(block, I0, I0, I1);                      // I0 = VN..
	UML_OR(block, I3, I3, I4);                      // I1 = ..ZC
	UML_OR(block, sr, sr, I0);
	UML_OR(block, sr, sr, I3);
}

void hyperstone_device::generate_update_nz(drcuml_block &block, compiler_state &compiler, uml::parameter sr)
{
	// expects result in I0 and UML Z flag to be set
	// clobbers I1

	UML_SETc(block, uml::COND_Z, I1);
	UML_SHL(block, I1, I1, Z_SHIFT);
	UML_ROLINS(block, I1, I0, N_SHIFT + 1, N_MASK);
	UML_OR(block, sr, sr, I1);
}

void hyperstone_device::generate_update_nz_d(drcuml_block &block, compiler_state &compiler, uml::parameter sr)
{
	// expects result in I0 and UML Z flag to be set
	// assumes Z and N bits in sr have already been cleared
	// clobbers I1

	UML_SETc(block, uml::COND_Z, I1);
	UML_SHL(block, I1, I1, Z_SHIFT);
	UML_OR(block, sr, sr, I1);
	UML_DROLAND(block, I1, I0, N_SHIFT + 1, N_MASK);
	UML_OR(block, sr, sr, I1);
}


void hyperstone_device::generate_raise_exception(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint8_t trapno, uml::parameter sr)
{
	if (desc->flags & OPFLAG_IN_DELAY_SLOT)
	{
		// definitely in delay slot - saved PC/ILC reflect delayed branch
		UML_OR(block, sr, sr, P_MASK);
		UML_MOV(block, DRC_PC, desc->pc);
	}
	else if (!compiler.check_delay())
	{
		// not in delay slot - saved PC/ILC reflect current instruction
		UML_ROLINS(block, sr, ((desc->length >> 1) << ILC_SHIFT) | P_MASK, 0, ILC_MASK | P_MASK);
	}
	else
	{
		// possibly in delay slot
		const int set_delay_pc = compiler.next_label();
		const int done = compiler.next_label();
		UML_TEST(block, mem(&m_core->delay_slot_taken), ~uint32_t(0));
		UML_JMPc(block, uml::COND_NZ, set_delay_pc);

		UML_ROLINS(block, sr, ((desc->length >> 1) << ILC_SHIFT) | P_MASK, 0, ILC_MASK | P_MASK);
		UML_JMP(block, done);

		UML_LABEL(block, set_delay_pc);
		UML_OR(block, sr, sr, P_MASK);
		UML_MOV(block, DRC_PC, desc->pc);
		UML_LABEL(block, done);
	}
	UML_MOV(block, DRC_SR, sr);
	UML_EXH(block, *m_exception, trapno);
}

void hyperstone_device::generate_raise_exception(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint8_t trapno)
{
	UML_MOV(block, I2, DRC_SR);
	generate_raise_exception(block, compiler, desc, trapno, uml::I2);
}

void hyperstone_device::generate_exception_on_overflow(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uml::parameter sr)
{
	const int no_exception = compiler.next_label();
	UML_TEST(block, sr, V_MASK);
	UML_JMPc(block, uml::COND_Z, no_exception);
	generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR, sr);
	UML_LABEL(block, no_exception);
}


template <hyperstone_device::trap_exception_or_int TYPE>
void hyperstone_device::generate_trap_exception_or_int(drcuml_block &block, uml::code_label &label, uml::parameter trapno)
{
	// expects cycles in I7 (updated)
	// clobbers I0, I1, I2, I3 and I4 after using trapno

	const uint32_t set_flags = (TYPE == IS_INT) ? (S_MASK | L_MASK | I_MASK) : (S_MASK | L_MASK);
	const uint32_t clear_flags = T_MASK | M_MASK;
	const uint32_t update_sr = FP_MASK | FL_MASK | set_flags | clear_flags;

	if ((TYPE != IS_INT) && debugger_enabled())
	{
		if (!trapno.is_int_register())
		{
			UML_MOV(block, I0, trapno);
			trapno = uml::I0;
		}

		UML_MOV(block, mem(&m_core->arg0), trapno);                   // let the debugger know
		UML_CALLC(block, &c_funcs::debugger_exception_hook, this);
	}
	generate_get_trap_addr(block, label, trapno);                     // I0 = target PC

	UML_MOV(block, I4, DRC_SR);                                       // I4 = old SR

	UML_MOV(block, I1, I4);                                           // I1 = SR to be updated
	UML_BFXU(block, I3, I4, FP_SHIFT, 7);                             // I3 = old FP
	UML_BFXU(block, I2, I4, FL_SHIFT, 4);                             // I2 = old FL
	UML_MOVc(block, uml::COND_Z, I2, 16);                             // convert FL == 0 to 16
	UML_ADD(block, I3, I3, I2);                                       // I3 = updated FP

	UML_SHL(block, I2, I3, FP_SHIFT);                                 // I2 = updated FP:...
	UML_OR(block, I2, I2, (((TYPE != IS_TRAP) ? 2 : 6) << FL_SHIFT) | set_flags);
	UML_ROLINS(block, I1, I2, 0, update_sr);                          // update SR value
	UML_MOV(block, DRC_SR, I1);                                       // store updated SR

	UML_AND(block, I3, I3, 0x3f);                                     // save old PC at updated (FP)^
	UML_AND(block, I2, DRC_PC, ~uint32_t(1));
	UML_ROLINS(block, I2, I4, 32 - S_SHIFT, 1);
	UML_STORE(block, (void *)m_core->local_regs, I3, I2, SIZE_DWORD, SCALE_x4);
	UML_ADD(block, I3, I3, 1);                                        // save old SR at updated (FP + 1)^
	UML_AND(block, I3, I3, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I3, I4, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I7, I7, mem(&m_core->clock_cycles_2));             // assume exception dispatch takes two cycles
	UML_MOV(block, DRC_PC, I0);                                       // branch to exception handler
	generate_update_cycles(block);
	UML_HASHJMP(block, 1, I0, *m_nocode);                             // T cleared and S set - mode will always be 1
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal, typename T>
inline void hyperstone_device::generate_logic_op(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, T &&body)
{
	// body takes operands in dst and src and should update dst and set Z flag
	// body must not clobber I2 or I3

	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	uml::parameter src = uml::I1;
	if (!SrcGlobal)
		generate_load_operand(block, compiler, SrcGlobal, src_code, src, src);
	else
		src = uml::mem(&m_core->global_regs[src_code]);

	uml::parameter dst = uml::I0;
	if (!DstGlobal || (dst_code <= SR_REGISTER))
		generate_load_operand(block, compiler, DstGlobal, dst_code, dst, uml::I3);
	else
		dst = uml::mem(&m_core->global_regs[dst_code]);

	body(dst, src);

	UML_SETc(block, uml::COND_Z, I1);
	UML_ROLINS(block, I2, I1, Z_SHIFT, Z_MASK);
	UML_MOV(block, DRC_SR, I2);

	if (!DstGlobal || (dst_code <= SR_REGISTER))
		generate_set_dst(block, compiler, desc, DstGlobal, dst_code, dst, uml::I3, false);
}

template <hyperstone_device::reg_bank DstGlobal, typename T>
inline void hyperstone_device::generate_logic_op_imm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t dst_code, T &&body)
{
	// clobbers I0, I1 and I3
	// body should update dst and set Z flag
	// body must not clobber I2 or I3

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if (!DstGlobal || (dst_code <= SR_REGISTER))
	{
		generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

		body(uml::I0);
	}
	else
	{
		body(uml::mem(&m_core->global_regs[dst_code]));
	}

	UML_SETc(block, uml::COND_Z, I1);
	UML_ROLINS(block, I2, I1, Z_SHIFT, Z_MASK);
	UML_MOV(block, DRC_SR, I2);

	if (!DstGlobal || (dst_code <= SR_REGISTER))
		generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}



void hyperstone_device::generate_software(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_6));

	const uint16_t op = desc->opptr.w[0];

	const uint32_t num = op >> 8;
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7); // I3 = FP

	UML_ADD(block, I1, I3, src_code);
	UML_AND(block, I1, I1, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4); // I0 = sreg
	UML_ADD(block, I1, I1, 1);
	UML_AND(block, I1, I1, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4); // I1 = sregf

	UML_AND(block, I2, I2, ~ILC_MASK);
	UML_OR(block, I2, I2, 1 << ILC_SHIFT);
	UML_MOV(block, DRC_SR, I2);

	UML_BFXU(block, I4, I2, FL_SHIFT, 4);
	UML_MOVc(block, uml::COND_Z, I4, 16);
	UML_ADD(block, I4, I4, I3); // I4 = reg

	UML_AND(block, I2, mem(&SP), 0xffffff00);
	UML_ADD(block, I6, I2, 0x100); // I6 = (SP & ~0xff) + 0x100
	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I2, I2, 0x3f);
	UML_SHL(block, I2, I2, 2); // I2 = (((fp + DST_CODE) & 0x3f) << 2)
	UML_ADD(block, I6, I6, I2); // I6 = (SP & ~0xff) + 0x100 + (((fp + DST_CODE) & 0x3f) << 2)

	UML_AND(block, I2, I4, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I6, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 0) & 0x3f] = stack_of_dst;
	UML_ADD(block, I2, I2, 1);
	UML_AND(block, I2, I2, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 1) & 0x3f] = sreg;
	UML_ADD(block, I2, I2, 1);
	UML_AND(block, I2, I2, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 2) & 0x3f] = sregf;

	UML_AND(block, I0, DRC_PC, ~uint32_t(1));
	UML_ROLINS(block, I0, DRC_SR, 32 - S_SHIFT, 1);
	UML_ADD(block, I2, I2, 1);
	UML_AND(block, I2, I2, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 3) & 0x3f] = (PC & ~1) | GET_S;

	UML_ADD(block, I2, I2, 1);
	UML_AND(block, I2, I2, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, DRC_SR, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 4) & 0x3f] = oldSR;

	const int mem3 = compiler.next_label();
	const int have_code_addr = compiler.next_label();
	UML_MOV(block, I1, mem(&m_core->trap_entry));
	UML_CMP(block, I1, 0xffffff00);
	UML_JMPc(block, uml::COND_E, mem3);
	UML_OR(block, I1, I1, (0x10c | ((0xcf - num) << 4)));
	UML_JMP(block, have_code_addr);

	UML_LABEL(block, mem3);
	UML_SUB(block, I1, I1, 0x100);
	UML_OR(block, I1, I1, ((num & 0xf) << 4));

	UML_LABEL(block, have_code_addr);
	UML_MOV(block, DRC_PC, I1); // PC = addr

	UML_MOV(block, I0, DRC_SR);
	UML_ROLINS(block, I0, (6 << FL_SHIFT) | L_MASK, 0, FL_MASK | T_MASK | L_MASK | M_MASK); // FL = 6, T = 0, L = 1, M = 0
	UML_ROLINS(block, I0, I4, FP_SHIFT, FP_MASK); // SET_FP(reg)
	UML_MOV(block, DRC_SR, I0);

	generate_branch(block, compiler, compiler.mode() & 0x1, uml::I1, desc); // T cleared - only keep S in bit zero of mode
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_chk(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	// checking a register other than PC against itself is a NOP
	if ((DstGlobal == SrcGlobal) && (src_code == dst_code) && (!DstGlobal || (src_code > SR_REGISTER)))
		return;

	// checking PC against itself will always trap
	const bool unconditional = DstGlobal && SrcGlobal && (src_code == PC_REGISTER) && (dst_code == PC_REGISTER);

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal || !SrcGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I0);

	int done;
	if (!unconditional)
	{
		done = compiler.next_label();
		if (SrcGlobal)
		{
			if (src_code == SR_REGISTER)
			{
				UML_TEST(block, I0, ~uint32_t(0));
				UML_JMPc(block, uml::COND_NZ, done);
			}
			else
			{
				UML_MOV(block, I1, mem(&m_core->global_regs[src_code]));
				UML_CMP(block, I0, I1);
				if (src_code == PC_REGISTER)
					UML_JMPc(block, uml::COND_B, done);
				else
					UML_JMPc(block, uml::COND_BE, done);
			}
		}
		else
		{
			UML_ADD(block, I3, I3, src_code);
			UML_AND(block, I3, I3, 0x3f);
			UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);

			UML_CMP(block, I0, I1);
			UML_JMPc(block, uml::COND_BE, done);
		}
	}

	generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR, uml::I2);

	if (!unconditional)
		UML_LABEL(block, done);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_movd(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];

	const uint32_t src_code = op & 0xf;
	const uint32_t srcf_code = src_code + 1;
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;

	if (DstGlobal && (dst_code == PC_REGISTER))
	{
		if (SrcGlobal && (src_code < 2))
		{
			osd_printf_error("Denoted PC or SR in RET instruction. PC = %08X\n", desc->pc);
			return;
		}

		UML_MOV(block, I3, DRC_SR);
		if (SrcGlobal)
		{
			UML_MOV(block, I0, mem(&m_core->global_regs[src_code]));
			UML_MOV(block, I2, mem(&m_core->global_regs[srcf_code]));
		}
		else
		{
			UML_BFXU(block, I2, I3, FP_SHIFT, 7);
			UML_ADD(block, I0, I2, src_code);
			UML_AND(block, I0, I0, 0x3f);
			UML_LOAD(block, I0, (void *)m_core->local_regs, I0, SIZE_DWORD, SCALE_x4);

			UML_ADD(block, I2, I2, srcf_code);
			UML_AND(block, I2, I2, 0x3f);
			UML_LOAD(block, I2, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
		}

		UML_AND(block, I2, I2, ~(ILC_MASK | S_MASK)); // clear ILC, restore S from bit zero of Rs
		UML_ROLINS(block, I2, I0, S_SHIFT, S_MASK);
		UML_MOV(block, DRC_SR, I2);

		UML_AND(block, I0, I0, ~uint32_t(1));
		UML_MOV(block, DRC_PC, I0);

		if (compiler.user_mode())
		{
			// privilege exception on setting S or L
			UML_XOR(block, I3, I3, ~uint32_t(0));
			UML_AND(block, I3, I3, S_MASK | L_MASK);
			UML_TEST(block, I3, I2);
			UML_EXHc(block, uml::COND_NZ, *m_exception, TRAPNO_PRIVILEGE_ERROR);
		}
		else
		{
			// privilege exception on setting L while clearing S
			UML_XOR(block, I3, I3, ~uint32_t(0));               // I3(15) = !L
			UML_AND(block, I3, I3, I2);                         // I3(15) = !L && L'
			UML_XOR(block, I3, I3, ~uint32_t(0));               // I3(15) = !(!L && L') = L || !L'
			UML_SHL(block, I3, I3, S_SHIFT - L_SHIFT);          // I3(18) = L || !L'
			UML_OR(block, I3, I3, I2);                          // I3(18) = L || !L' || S'
			UML_TEST(block, I3, S_MASK);
			UML_EXHc(block, uml::COND_Z, *m_exception, TRAPNO_PRIVILEGE_ERROR);
		}

		const int pop_next = compiler.next_label();
		const int done_ret = compiler.next_label();
		UML_MOV(block, I0, mem(&SP));                           // I0 = SP
		UML_BFXU(block, I1, I0, 2, 7);                          // I3 = FP - SP(8..2)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);
		UML_SUB(block, I3, I3, I1);
		UML_BFXS(block, I3, I3, 0, 7);                          // sign-extend 7-bit number
		UML_JMPc(block, uml::COND_NS, done_ret);                // nothing to pull if not negative
		UML_LABEL(block, pop_next);
		UML_SUB(block, I0, I0, 4);                              // pull a word
		UML_CALLH(block, *m_mem_read32);
		UML_AND(block, I4, I0, 0x3f << 2);
		UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x1);
		UML_ADD(block, I3, I3, 1);                              // increment counter
		UML_JMPc(block, uml::COND_S, pop_next);                 // done if not negative
		UML_MOV(block, mem(&SP), I0);                           // SP = I0
		UML_LABEL(block, done_ret);

		generate_branch(block, compiler, BRANCH_TARGET_DYNAMIC, desc->targetpc, nullptr); // don't pass desc - must not update ILC and P
		return;
	}
	else if (SrcGlobal && (src_code == SR_REGISTER)) // Rd doesn't denote PC and Rs denotes SR
	{
		UML_MOV(block, I2, DRC_SR);
		UML_OR(block, I2, I2, Z_MASK);
		UML_AND(block, I2, I2, ~N_MASK);
		UML_MOV(block, DRC_SR, I2);
		if (DstGlobal)
		{
			generate_set_global_register_low(block, compiler, desc, dst_code, 0);
			generate_set_global_register_low(block, compiler, desc, dstf_code, 0);
		}
		else
		{
			UML_BFXU(block, I0, I2, FP_SHIFT, 7);
			UML_ADD(block, I0, I0, dst_code);
			UML_AND(block, I0, I0, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I0, 0, SIZE_DWORD, SCALE_x4);
			UML_ADD(block, I0, I0, 1);
			UML_AND(block, I0, I0, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I0, 0, SIZE_DWORD, SCALE_x4);
		}
	}
	else // Rd doesn't denote PC and Rs doesn't denote SR
	{
		UML_MOV(block, I2, DRC_SR);
		if (!SrcGlobal || !DstGlobal)
			UML_BFXU(block, I3, I2, FP_SHIFT, 7);

		if (SrcGlobal)
		{
			UML_MOV(block, I0, mem(&m_core->global_regs[src_code]));
			UML_MOV(block, I1, mem(&m_core->global_regs[srcf_code]));
		}
		else
		{
			UML_ADD(block, I1, I3, src_code);
			UML_AND(block, I1, I1, 0x3f);
			UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);

			UML_ADD(block, I1, I1, 1);
			UML_AND(block, I1, I1, 0x3f);
			UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
		}

		UML_AND(block, I2, I2, ~(Z_MASK | N_MASK));
		UML_OR(block, I4, I0, I1);
		UML_SETc(block, uml::COND_Z, I4);
		UML_ROLINS(block, I2, I4, Z_SHIFT, Z_MASK);
		UML_ROLAND(block, I4, I0, N_SHIFT + 1, N_MASK);
		UML_OR(block, I2, I2, I4);
		UML_MOV(block, DRC_SR, I2);

		if (DstGlobal)
		{
			generate_set_global_register_low(block, compiler, desc, dst_code, uml::I0);
			generate_set_global_register_low(block, compiler, desc, dstf_code, uml::I1);
		}
		else
		{
			UML_ADD(block, I3, I3, dst_code);
			UML_AND(block, I3, I3, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);

			UML_ADD(block, I3, I3, 1);
			UML_AND(block, I3, I3, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
		}
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal, hyperstone_device::sign_mode SIGNED>
void hyperstone_device::generate_divsu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_36));

	const uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;
	const uint32_t src_code = op & 0xf;

	if ((SrcGlobal == DstGlobal && (src_code == dst_code || src_code == dstf_code)) || (SrcGlobal && src_code < 2))
	{
		osd_printf_error("Denoted the same register code or PC/SR as source in generate_divsu. PC = %08X\n", desc->pc);
		return;
	}

	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I0, uml::I0);

	if (DstGlobal)
	{
		UML_MOV(block, I1, mem(&m_core->global_regs[dst_code]));
		UML_MOV(block, I2, mem(&m_core->global_regs[dstf_code]));
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
		UML_ADD(block, I4, I3, dstf_code);
		UML_AND(block, I6, I4, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	}

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I1, I1, I2);

	int no_result = compiler.next_label();
	int done = compiler.next_label();
	UML_TEST(block, I0, ~0);
	UML_JMPc(block, uml::COND_Z, no_result);
	if (SIGNED)
	{
		UML_DTEST(block, I1, 0x8000000000000000LL);
		UML_JMPc(block, uml::COND_NZ, no_result);
		UML_DSEXT(block, I0, I0, SIZE_DWORD);
	}

	if (SIGNED)
		UML_DDIVS(block, I2, I4, I1, I0);
	else
		UML_DDIVU(block, I2, I4, I1, I0);

	UML_AND(block, I3, DRC_SR, ~(V_MASK | Z_MASK | N_MASK));
	UML_TEST(block, I2, ~uint32_t(0));
	UML_SETc(block, uml::COND_Z, I0);
	UML_SHL(block, I0, I0, Z_SHIFT);
	UML_ROLINS(block, I0, I2, N_SHIFT + 1, N_MASK);
	UML_OR(block, DRC_SR, I3, I0);

	if (DstGlobal)
	{
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), I4);
		UML_MOV(block, mem(&m_core->global_regs[dstf_code]), I2);
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
		UML_STORE(block, (void *)m_core->local_regs, I6, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_JMP(block, done);

	UML_LABEL(block, no_result);
	generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR);

	UML_LABEL(block, done);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_xm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	const uint32_t next = m_pr16(desc->pc + 2);
	const uint8_t sub_type = (next & 0x7000) >> 12;

	uint32_t extra_u = next & 0xfff;
	if (next & 0x8000)
		extra_u = ((extra_u & 0xfff) << 16) | m_pr16(desc->pc + 4);

	if ((SrcGlobal && (src_code == SR_REGISTER)) || (DstGlobal && (dst_code <= SR_REGISTER)))
	{
		// reserved for future expansion
		osd_printf_error("Denoted PC or SR in hyperstone_xm. PC = %08X\n", desc->pc);
		return;
	}

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I1, uml::I1);

	UML_SHL(block, I0, I1, sub_type & 3);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, true);

	if (sub_type < 4)
	{
		const int no_exception = compiler.next_label();
		UML_CMP(block, I1, extra_u);
		if (SrcGlobal && (src_code == PC_REGISTER))
			UML_JMPc(block, uml::COND_B, no_exception);
		else
			UML_JMPc(block, uml::COND_BE, no_exception);

		generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR, uml::I2);
		UML_LABEL(block, no_exception);
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_mask(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const uint32_t src = generate_get_const(desc);

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I0, uml::I0);

	UML_AND(block, I0, I0, src);
	UML_SETc(block, uml::COND_Z, I1);
	UML_ROLINS(block, I2, I1, Z_SHIFT, Z_MASK);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, true);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_sum(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const uint32_t src = generate_get_const(desc);

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if (SrcGlobal && (src_code == PC_REGISTER) && ((desc->flags & OPFLAG_IN_DELAY_SLOT) || !compiler.check_delay()))
	{
		const uint64_t result = uint64_t(compiler.pc()) + src;
		const uint32_t flags =
				(BIT(result, 32) << C_SHIFT) |
				(!uint32_t(result) ? Z_MASK : 0) |
				(BIT(result, 31) << N_SHIFT) |
				(BIT((compiler.pc() ^ result) & (src ^ result), 31) << V_SHIFT);

		UML_ROLINS(block, I2, flags, 0, V_MASK | N_MASK | Z_MASK | C_MASK);
		UML_MOV(block, DRC_SR, I2);

		generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uint32_t(result), uml::I3, true);
	}
	else
	{
		generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I0, uml::I0);

		UML_ADD(block, I0, I0, src);

		generate_update_flags_addsub(block, compiler, uml::I2);
		UML_MOV(block, DRC_SR, I2);

		generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, true);
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_sums(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const uint32_t src = generate_get_const(desc);

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I0, uml::I0);

	UML_ADD(block, I0, I0, src);

	generate_update_flags_addsubs(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_register(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, true);
	generate_exception_on_overflow(block, compiler, desc, uml::I2);

	if (DstGlobal && (dst_code == PC_REGISTER))
	{
		UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
		if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
		{
			UML_AND(block, I0, I0, ~uint32_t(1));
			generate_branch(block, compiler, compiler.mode(), uml::I0, desc);
		}
		else
		{
			generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
		}
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_cmp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I1, uml::I1, uml::I2);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_CMP(block, I0, I1);

	generate_update_flags_cmp(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);
}

template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_mov(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if (DstGlobal && compiler.user_mode())
	{
		const int no_exception = compiler.next_label();
		UML_TEST(block, I2, H_MASK);
		UML_JMPc(block, uml::COND_Z, no_exception);
		generate_raise_exception(block, compiler, desc, TRAPNO_PRIVILEGE_ERROR, uml::I2);
		UML_LABEL(block, no_exception);
	}

	UML_AND(block, I2, I2, ~(Z_MASK | N_MASK));

	if (SrcGlobal)
	{
		if (!DstGlobal || compiler.supervisor_mode())
		{
			const int highglobal = compiler.next_label();
			const int done = compiler.next_label();

			UML_TEST(block, I2, H_MASK);
			UML_JMPc(block, uml::COND_NZ, highglobal);

			UML_MOV(block, I0, mem(&m_core->global_regs[src_code]));
			UML_JMP(block, done);

			UML_LABEL(block, highglobal);
			generate_get_global_register_high(block, compiler, src_code, uml::I0);

			UML_LABEL(block, done);
		}
		else
		{
			UML_MOV(block, I0, mem(&m_core->global_regs[src_code]));
		}
	}
	else if (DstGlobal || (src_code == dst_code))
	{
		UML_ADD(block, I3, I3, src_code);
		UML_AND(block, I3, I3, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I0, I3, src_code);
		UML_AND(block, I0, I0, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I0, SIZE_DWORD, SCALE_x4);
	}

	UML_TEST(block, I0, ~uint32_t(0));
	generate_update_nz(block, compiler, uml::I2);

	if (DstGlobal)
	{
		if (compiler.supervisor_mode())
		{
			const int highglobal = compiler.next_label();
			const int done = compiler.next_label();

			UML_TEST(block, I2, H_MASK);
			UML_JMPc(block, uml::COND_NZ, highglobal);

			UML_MOV(block, DRC_SR, I2);
			generate_set_global_register_low(block, compiler, desc, dst_code, uml::I0);
			if (dst_code == PC_REGISTER)
			{
				UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
				UML_AND(block, I0, I0, ~uint32_t(1));
				generate_branch(block, compiler, compiler.mode(), uml::I0, desc);
			}
			else
			{
				UML_JMP(block, done);
			}

			UML_LABEL(block, highglobal);
			UML_AND(block, I2, I2, ~H_MASK);
			UML_MOV(block, DRC_SR, I2);
			generate_set_global_register_high(block, compiler, dst_code, uml::I0);

			UML_LABEL(block, done);
		}
		else
		{
			UML_MOV(block, DRC_SR, I2);
			generate_set_global_register_low(block, compiler, desc, dst_code, uml::I0);
			if (dst_code == PC_REGISTER)
			{
				UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
				UML_AND(block, I0, I0, ~uint32_t(1));
				generate_branch(block, compiler, compiler.mode(), uml::I0, desc);
			}
		}
	}
	else
	{
		UML_AND(block, I2, I2, ~H_MASK);
		UML_MOV(block, DRC_SR, I2);

		if (SrcGlobal || (src_code != dst_code))
		{
			UML_ADD(block, I3, I3, dst_code);
			UML_AND(block, I3, I3, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
		}
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_add(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if ((desc->flags & OPFLAG_IN_DELAY_SLOT) || !compiler.check_delay())
	{
		const bool srcpc = SrcGlobal && (src_code == PC_REGISTER);
		const bool dstpc = DstGlobal && (dst_code == PC_REGISTER);

		if (srcpc && dstpc)
		{
			// degenerate case - effectively left-shift PC
			const uint32_t result = compiler.pc() << 1;
			const uint32_t flags =
					(BIT(compiler.pc(), 31) << C_SHIFT) |
					(!result ? Z_MASK : 0) |
					(BIT(result, 31) << N_SHIFT) |
					(BIT((compiler.pc() ^ result), 31) << V_SHIFT);

			UML_ROLINS(block, I2, flags, 0, M_MASK | V_MASK | N_MASK | Z_MASK | C_MASK);
			UML_MOV(block, DRC_SR, I2);

			UML_MOV(block, DRC_PC, result);
			generate_branch(block, compiler, compiler.mode(), result, desc);
			return;
		}
		else if (srcpc)
		{
			generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

			UML_ADD(block, I0, I0, compiler.pc());

			generate_update_flags_addsub(block, compiler, uml::I2);
			UML_MOV(block, DRC_SR, I2);

			generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
			return;
		}
		else if (dstpc)
		{
			generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I0, uml::I3, uml::I2);

			UML_ADD(block, I0, I0, compiler.pc());

			generate_update_flags_addsub(block, compiler, uml::I2);
			UML_AND(block, I2, I2, ~M_MASK);
			UML_MOV(block, DRC_SR, I2);

			UML_AND(block, I0, I0, ~uint32_t(1));
			UML_MOV(block, DRC_PC, I0);
			generate_branch(block, compiler, compiler.mode(), uml::I0, desc);
			return;
		}
	}

	generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I1, uml::I1, uml::I2);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_ADD(block, I0, I0, I1);

	generate_update_flags_addsub(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_adds(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I1, uml::I1, uml::I2);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_ADD(block, I0, I0, I1);

	generate_update_flags_addsubs(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_register(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
	generate_exception_on_overflow(block, compiler, desc, uml::I2);

	if (DstGlobal && (dst_code == PC_REGISTER))
	{
		UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
		if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
		{
			UML_AND(block, I0, I0, ~uint32_t(1));
			generate_branch(block, compiler, compiler.mode(), uml::I0, desc);
		}
		else
		{
			generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
		}
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_cmpb(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I1, uml::I1);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_TEST(block, I0, I1);
	UML_SETc(block, uml::COND_Z, I0);
	UML_ROLINS(block, DRC_SR, I0, Z_SHIFT, Z_MASK);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_subc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal,src_code, uml::I1, uml::I1, uml::I2);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	if (!SrcGlobal || (src_code != SR_REGISTER))
	{
		UML_CARRY(block, I2, C_SHIFT);
		UML_SUBB(block, I0, I0, I1);
	}
	else
	{
		UML_SUB(block, I0, I0, I1);
	}

	generate_update_flags_addsubc(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_sub(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I1, uml::I1, uml::I2);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_SUB(block, I0, I0, I1);

	generate_update_flags_addsub(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_subs(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I1, uml::I1, uml::I2);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_SUB(block, I0, I0, I1);

	generate_update_flags_addsubs(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_register(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
	generate_exception_on_overflow(block, compiler, desc, uml::I2);

	if (DstGlobal && (dst_code == PC_REGISTER))
	{
		UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
		if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
		{
			UML_AND(block, I0, I0, ~uint32_t(1));
			generate_branch(block, compiler, compiler.mode(), uml::I0, desc);
		}
		else
		{
			generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
		}
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_addc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal,src_code, uml::I1, uml::I1, uml::I2);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	if (!SrcGlobal || (src_code != SR_REGISTER))
	{
		UML_CARRY(block, I2, C_SHIFT);
		UML_ADDC(block, I0, I0, I1);
	}
	else
	{
		UML_ADD(block, I0, I0, I1);
	}

	generate_update_flags_addsubc(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_neg(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I0, uml::I0, uml::I2);

	UML_SUB(block, I0, 0, I0);

	generate_update_flags_addsub(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, true);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_negs(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_src_addsub(block, compiler, SrcGlobal, src_code, uml::I0, uml::I0, uml::I2);

	UML_SUB(block, I0, 0, I0);

	generate_update_flags_addsubs(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_register(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, true);

	if (!SrcGlobal || (src_code != SR_REGISTER)) // negating carry cannot result in overflow
		generate_exception_on_overflow(block, compiler, desc, uml::I2);

	if (DstGlobal && (dst_code == PC_REGISTER))
	{
		UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
		if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
		{
			UML_AND(block, I0, I0, ~uint32_t(1));
			generate_branch(block, compiler, compiler.mode(), uml::I0, desc);
		}
		else
		{
			generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
		}
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_and(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	generate_logic_op<DstGlobal, SrcGlobal>(
			block,
			compiler,
			desc,
			[&block] (uml::parameter dst, uml::parameter src)
			{
				UML_AND(block, dst, dst, src);
			});
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_andn(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	generate_logic_op<DstGlobal, SrcGlobal>(
			block,
			compiler,
			desc,
			[&block] (uml::parameter dst, uml::parameter src)
			{
				UML_XOR(block, I1, src, ~uint32_t(0));
				UML_AND(block, dst, dst, I1);
			});
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_or(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	generate_logic_op<DstGlobal, SrcGlobal>(
			block,
			compiler,
			desc,
			[&block] (uml::parameter dst, uml::parameter src)
			{
				UML_OR(block, dst, dst, src);
			});
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_xor(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	generate_logic_op<DstGlobal, SrcGlobal>(
			block,
			compiler,
			desc,
			[&block] (uml::parameter dst, uml::parameter src)
			{
				UML_XOR(block, dst, dst, src);
			});
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_not(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I0, uml::I0);

	UML_XOR(block, I0, I0, ~uint32_t(0));
	UML_SETc(block, uml::COND_Z, I1);
	UML_ROLINS(block, I2, I1, Z_SHIFT, Z_MASK);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, true);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_cmpi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	uint32_t src;
	if (ImmLong)
		src = generate_get_immediate_s(desc);
	else
		src = op & 0x0f;

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I0);

	UML_CMP(block, I0, src);

	generate_update_flags_cmp(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_movi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	uint32_t src;
	if (ImmLong)
		src = generate_get_immediate_s(desc);
	else
		src = op & 0x0f;

	UML_MOV(block, I2, DRC_SR);

	if (DstGlobal && compiler.user_mode())
	{
		const int no_exception = compiler.next_label();
		UML_TEST(block, I2, H_MASK);
		UML_JMPc(block, uml::COND_Z, no_exception);
		generate_raise_exception(block, compiler, desc, TRAPNO_PRIVILEGE_ERROR, uml::I2);
		UML_LABEL(block, no_exception);
	}

	UML_AND(block, I2, I2, ~(Z_MASK | N_MASK | V_MASK));
	if (!src)
		UML_OR(block, I2, I2, Z_MASK);
	else if (src & 0x80000000)
		UML_OR(block, I2, I2, N_MASK);

	if (DstGlobal)
	{
		if (compiler.supervisor_mode())
		{
			const int highglobal = compiler.next_label();
			const int done = compiler.next_label();

			UML_TEST(block, I2, H_MASK);
			UML_JMPc(block, uml::COND_NZ, highglobal);

			UML_MOV(block, DRC_SR, I2);
			generate_set_global_register_low(block, compiler, desc, dst_code, src);
			if (dst_code == PC_REGISTER)
			{
				UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
				generate_branch(block, compiler, compiler.mode(), src & ~uint32_t(1), desc);
			}
			else
			{
				UML_JMP(block, done);
			}

			UML_LABEL(block, highglobal);
			UML_AND(block, I2, I2, ~H_MASK);
			UML_MOV(block, DRC_SR, I2);
			generate_set_global_register_high(block, compiler, dst_code, src);

			UML_LABEL(block, done);
		}
		else
		{
			UML_MOV(block, DRC_SR, I2);
			generate_set_global_register_low(block, compiler, desc, dst_code, src);
			if (dst_code == PC_REGISTER)
			{
				UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
				generate_branch(block, compiler, compiler.mode(), src & ~uint32_t(1), desc);
			}
		}
	}
	else
	{
		UML_AND(block, I2, I2, ~H_MASK);
		UML_MOV(block, DRC_SR, I2);

		UML_BFXU(block, I2, I2, FP_SHIFT, 7);
		UML_ADD(block, I2, I2, dst_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I2, src, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_addi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const bool roundeven = !(op & 0x10f);

	uint32_t src;
	if (ImmLong)
		src = generate_get_immediate_s(desc);
	else
		src = op & 0x0f;

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if (DstGlobal && (dst_code == PC_REGISTER) && ((desc->flags & OPFLAG_IN_DELAY_SLOT) || !compiler.check_delay()))
	{
		uint64_t result;
		uint32_t flags;
		if (roundeven)
		{
			// PC is always even so rounding to even can't cause carry or overflow
			result = compiler.pc();
			flags = (!uint32_t(result) ? Z_MASK : 0) | (BIT(result, 31) << N_SHIFT);
		}
		else
		{
			result = uint64_t(compiler.pc()) + src;
			flags =
					(BIT(result, 32) << C_SHIFT) |
					(!uint32_t(result) ? Z_MASK : 0) |
					(BIT(result, 31) << N_SHIFT) |
					(BIT((compiler.pc() ^ result) & (src ^ result), 31) << V_SHIFT);
		}

		UML_ROLINS(block, I2, flags, 0, M_MASK | V_MASK | N_MASK | Z_MASK | C_MASK);
		UML_MOV(block, DRC_SR, I2);

		result = uint32_t(result) & uint32_t(1);
		UML_MOV(block, DRC_PC, result);
		generate_branch(block, compiler, compiler.mode(), result, desc);
	}
	else
	{
		generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

		uml::parameter srcp = roundeven ? uml::I1 : src;
		if (roundeven)
		{
			UML_AND(block, I1, I0, 1);              // Rd(0)
			UML_AND(block, I1, I1, I2);             // & C
			UML_TEST(block, I2, Z_MASK);
			UML_MOVc(block, uml::COND_NZ, I1, 0);   // & ~Z
		}

		UML_ADD(block, I0, I0, srcp);

		generate_update_flags_addsub(block, compiler, uml::I2);
		UML_MOV(block, DRC_SR, I2);

		generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_addsi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	osd_printf_error("Unimplemented: generate_addsi (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_cmpbi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = ((op & 0x100) >> 4) | (op & 0x0f);

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if (!DstGlobal || !n)
		generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	if (n)
	{
		uint32_t src;
		if (n == 31)
			src = 0x7fffffff;
		else if (ImmLong)
			src = generate_get_immediate_s(desc);
		else
			src = op & 0xf;

		if (DstGlobal)
			UML_TEST(block, mem(&m_core->global_regs[dst_code]), src);
		else
			UML_TEST(block, I0, src);
		UML_SETc(block, uml::COND_Z, I1);
		UML_ROLINS(block, I2, I1, Z_SHIFT, Z_MASK);
	}
	else
	{
		const int or_mask = compiler.next_label();
		const int done = compiler.next_label();
		UML_TEST(block, I0, 0xff000000);
		UML_JMPc(block, uml::COND_Z, or_mask);
		UML_TEST(block, I0, 0x00ff0000);
		UML_JMPc(block, uml::COND_Z, or_mask);
		UML_TEST(block, I0, 0x0000ff00);
		UML_JMPc(block, uml::COND_Z, or_mask);
		UML_TEST(block, I0, 0x000000ff);
		UML_JMPc(block, uml::COND_Z, or_mask);
		UML_AND(block, I2, I2, ~Z_MASK);
		UML_JMP(block, done);

		UML_LABEL(block, or_mask);
		UML_OR(block, I2, I2, Z_MASK);

		UML_LABEL(block, done);
	}

	UML_MOV(block, DRC_SR, I2);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_andni(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	uint32_t src;
	if (DRC_N_OP_MASK == 0x10f)
		src = 0x7fffffff;
	else if (ImmLong)
		src = generate_get_immediate_s(desc);
	else
		src = op & 0x0f;
	src = ~src;

	generate_logic_op_imm<DstGlobal>(
			block,
			compiler,
			desc,
			(op & 0xf0) >> 4,
			[&block, src] (uml::parameter dst) { UML_AND(block, dst, dst, src); });
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_ori(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	uint32_t src;
	if (ImmLong)
		src = generate_get_immediate_s(desc);
	else
		src = op & 0x0f;

	generate_logic_op_imm<DstGlobal>(
			block,
			compiler,
			desc,
			(op & 0xf0) >> 4,
			[&block, src] (uml::parameter dst) { UML_OR(block, dst, dst, src); });
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::imm_size ImmLong>
void hyperstone_device::generate_xori(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	uint32_t src;
	if (ImmLong)
		src = generate_get_immediate_s(desc);
	else
		src = op & 0x0f;

	generate_logic_op_imm<DstGlobal>(
			block,
			compiler,
			desc,
			(op & 0xf0) >> 4,
			[&block, src] (uml::parameter dst) { UML_XOR(block, dst, dst, src); });
}


template <hyperstone_device::shift_type HiN>
void hyperstone_device::generate_shrdi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = HiN ? DRC_HI_N_VALUE : DRC_LO_N_VALUE;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I1, uml::I4);
	generate_load_operand(block, compiler, LOCAL, dst_code + 1, uml::I0, uml::I3);

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I0, I0, I1);

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	if (n)
	{
		UML_DROLAND(block, I1, I0, 64 - C_SHIFT + 1 - n, C_MASK);
		UML_OR(block, I2, I2, I1);
	}

	UML_DSHR(block, I0, I0, n);
	generate_update_nz_d(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shrd(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if ((src_code == dst_code) || (src_code == (dst_code + 1)))
	{
		return; // undefined if Ls overlaps Ld or Ldf
	}

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I1, uml::I4);
	generate_load_operand(block, compiler, LOCAL, dst_code + 1, uml::I0, uml::I5);

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I0, I0, I1);

	generate_load_operand(block, compiler, LOCAL, src_code, uml::I1, uml::I3);

	const int no_shift = compiler.next_label();
	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	UML_AND(block, I1, I1, 0x1f);
	UML_JMPc(block, uml::COND_Z, no_shift);
	UML_SUB(block, I3, 64 - C_SHIFT + 1, I1);
	UML_DROLAND(block, I3, I0, I3, C_MASK);
	UML_OR(block, I2, I2, I3);
	UML_LABEL(block, no_shift);

	UML_DSHR(block, I0, I0, I1);
	generate_update_nz_d(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, (void *)m_core->local_regs, I5, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, src_code, uml::I1, uml::I1);
	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I0, uml::I3);

	const int no_shift = compiler.next_label();
	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	UML_AND(block, I1, I1, 0x1f);
	UML_JMPc(block, uml::COND_Z, no_shift);
	UML_SUB(block, I4, 32 - C_SHIFT + 1, I1);
	UML_ROLAND(block, I4, I0, I4, C_MASK);
	UML_OR(block, I2, I2, I4);
	UML_LABEL(block, no_shift);

	UML_SHR(block, I0, I0, I1);
	generate_update_nz(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::shift_type HiN, hyperstone_device::reg_bank DstGlobal>
void hyperstone_device::generate_shri(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = HiN ? DRC_HI_N_VALUE : DRC_LO_N_VALUE;

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	if (n)
	{
		UML_ROLAND(block, I4, I0, 32 - C_SHIFT + 1 - n, C_MASK);
		UML_OR(block, I2, I2, I4);
	}

	UML_SHR(block, I0, I0, n);
	generate_update_nz(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}


template <hyperstone_device::shift_type HiN>
void hyperstone_device::generate_sardi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = HiN ? DRC_HI_N_VALUE : DRC_LO_N_VALUE;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I1, uml::I4);
	generate_load_operand(block, compiler, LOCAL, dst_code + 1, uml::I0, uml::I3);

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I0, I0, I1);

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	if (n)
	{
		UML_DROLAND(block, I1, I0, 64 - C_SHIFT + 1 - n, C_MASK);
		UML_OR(block, I2, I2, I1);
	}

	UML_DSAR(block, I0, I0, n);
	generate_update_nz_d(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_sard(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if ((src_code == dst_code) || (src_code == (dst_code + 1)))
	{
		return; // undefined if Ls overlaps Ld or Ldf
	}

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I1, uml::I4);
	generate_load_operand(block, compiler, LOCAL, dst_code + 1, uml::I0, uml::I5);

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I0, I0, I1);

	generate_load_operand(block, compiler, LOCAL, src_code, uml::I1, uml::I3);

	const int no_shift = compiler.next_label();
	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	UML_AND(block, I1, I1, 0x1f);
	UML_JMPc(block, uml::COND_Z, no_shift);
	UML_SUB(block, I3, 64 - C_SHIFT + 1, I1);
	UML_DROLAND(block, I3, I0, I3, C_MASK);
	UML_OR(block, I2, I2, I3);
	UML_LABEL(block, no_shift);

	UML_DSAR(block, I0, I0, I1);
	generate_update_nz_d(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, (void *)m_core->local_regs, I5, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_sar(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, src_code, uml::I1, uml::I1);
	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I0, uml::I3);

	const int no_shift = compiler.next_label();
	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	UML_AND(block, I1, I1, 0x1f);
	UML_JMPc(block, uml::COND_Z, no_shift);
	UML_SUB(block, I4, 32 - C_SHIFT + 1, I1);
	UML_ROLAND(block, I4, I0, I4, C_MASK);
	UML_OR(block, I2, I2, I4);
	UML_LABEL(block, no_shift);

	UML_SAR(block, I0, I0, I1);
	generate_update_nz(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::shift_type HiN, hyperstone_device::reg_bank DstGlobal>
void hyperstone_device::generate_sari(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = HiN ? DRC_HI_N_VALUE : DRC_LO_N_VALUE;

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK));
	if (n)
	{
		UML_ROLAND(block, I4, I0, 32 - C_SHIFT + 1 - n, C_MASK);
		UML_OR(block, I2, I2, I4);
	}

	UML_SAR(block, I0, I0, n);
	generate_update_nz(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}


template <hyperstone_device::shift_type HiN>
void hyperstone_device::generate_shldi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = HiN ? DRC_HI_N_VALUE : DRC_LO_N_VALUE;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I1, uml::I4);
	generate_load_operand(block, compiler, LOCAL, dst_code + 1, uml::I0, uml::I5);

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK | V_MASK));
	if (n)
		UML_SHR(block, I3, I1, 32 - n);

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I0, I0, I1);

	if (n)
	{
		UML_ROLAND(block, I1, I3, C_SHIFT, C_MASK);
		UML_OR(block, I2, I2, I1);
	}

	UML_DSHL(block, I0, I0, n);
	generate_update_nz_d(block, compiler, uml::I2);

	if (n)
	{
		UML_DTEST(block, I0, ~uint64_t(0));
		UML_MOV(block, I1, util::make_bitmask<uint32_t>(n));
		UML_MOVc(block, uml::COND_NS, I1, 0);
		UML_CMP(block, I3, I1);
		UML_MOV(block, I1, V_MASK);
		UML_MOVc(block, uml::COND_E, I1, 0);
		UML_OR(block, I2, I2, I1);
	}

	UML_MOV(block, DRC_SR, I2);

	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, (void *)m_core->local_regs, I5, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shld(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if ((src_code == dst_code) || (src_code == (dst_code + 1)))
	{
		return; // undefined if Ls overlaps Ld or Ldf
	}

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, dst_code, uml::I1, uml::I5);
	generate_load_operand(block, compiler, LOCAL, dst_code + 1, uml::I0, uml::I6);

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I0, I0, I1);

	generate_load_operand(block, compiler, LOCAL, src_code, uml::I1, uml::I3);

	const int no_shift = compiler.next_label();
	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK | V_MASK));
	UML_AND(block, I1, I1, 0x1f);
	UML_JMPc(block, uml::COND_Z, no_shift);
	UML_SUB(block, I3, 64, I1);
	UML_DSHR(block, I4, I0, I3);
	UML_ROLINS(block, I2, I4, C_SHIFT, C_MASK);
	UML_LABEL(block, no_shift);

	UML_DSHL(block, I0, I0, I1);

	const int no_overflow = compiler.next_label();
	UML_TEST(block, I1, ~uint32_t(0));
	UML_JMPc(block, uml::COND_Z, no_overflow);
	UML_SHR(block, I1, ~uint32_t(0), I3);
	UML_DTEST(block, I0, ~uint64_t(0));
	UML_MOVc(block, uml::COND_NS, I1, 0);
	UML_CMP(block, I4, I1);
	UML_JMPc(block, uml::COND_E, no_overflow);
	UML_OR(block, I2, I2, V_MASK);
	UML_LABEL(block, no_overflow);

	UML_DTEST(block, I0, I0);
	generate_update_nz_d(block, compiler, uml::I2);
	UML_MOV(block, DRC_SR, I2);

	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I5, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, (void *)m_core->local_regs, I6, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shl(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if (dst_code != src_code)
	{
		generate_load_operand(block, compiler, LOCAL, src_code, uml::I1, uml::I1);
		generate_load_operand(block, compiler, LOCAL, dst_code, uml::I0, uml::I3);
		UML_AND(block, I1, I1, 0x1f);
	}
	else
	{
		generate_load_operand(block, compiler, LOCAL, dst_code, uml::I0, uml::I3);
		UML_AND(block, I1, I0, 0x1f);
	}

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK | V_MASK));

	UML_SUB(block, I4, 32, I1);
	UML_SHR(block, I5, ~uint32_t(0), I4);
	UML_SHR(block, I4, I0, I4);
	UML_TEST(block, I1, 0xffffffff);
	UML_MOVc(block, uml::COND_Z, I4, 0);
	UML_MOVc(block, uml::COND_Z, I5, 0);
	UML_ROLINS(block, I2, I4, C_SHIFT, C_MASK);

	UML_SHL(block, I0, I0, I1);
	UML_MOVc(block, uml::COND_NS, I5, 0);
	generate_update_nz(block, compiler, uml::I2);

	UML_CMP(block, I4, I5);
	UML_MOV(block, I1, V_MASK);
	UML_MOVc(block, uml::COND_E, I1, 0);
	UML_OR(block, I2, I2, I1);

	UML_MOV(block, DRC_SR, I2);

	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::shift_type HiN, hyperstone_device::reg_bank DstGlobal>
void hyperstone_device::generate_shli(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = HiN ? DRC_HI_N_VALUE : DRC_LO_N_VALUE;

	UML_MOV(block, I2, DRC_SR);
	if (!DstGlobal)
		UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I0, uml::I3);

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK | V_MASK));
	if (n)
	{
		UML_SHR(block, I4, I0, 32 - n);
		UML_MOV(block, I5, util::make_bitmask<uint32_t>(n));
		UML_ROLAND(block, I1, I4, C_SHIFT, C_MASK);
		UML_OR(block, I2, I2, I1);
	}

	UML_SHL(block, I0, I0, n);
	if (n)
	{
		UML_MOVc(block, uml::COND_NS, I5, 0);
	}
	generate_update_nz(block, compiler, uml::I2);

	if (n)
	{
		UML_CMP(block, I4, I5);
		UML_MOV(block, I1, V_MASK);
		UML_MOVc(block, uml::COND_E, I1, 0);
		UML_OR(block, I2, I2, I1);
	}

	UML_MOV(block, DRC_SR, I2);

	generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I3, false);
}


void hyperstone_device::generate_testlz(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	generate_load_operand(block, compiler, LOCAL, src_code, uml::I0, uml::I0);

	UML_LZCNT(block, I0, I0);

	generate_set_dst(block, compiler, desc, LOCAL, dst_code, uml::I0, uml::I3, true);
}


void hyperstone_device::generate_rol(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	UML_MOV(block, I2, DRC_SR);
	UML_BFXU(block, I3, I2, FP_SHIFT, 7);

	if (dst_code != src_code)
	{
		generate_load_operand(block, compiler, LOCAL, src_code, uml::I1, uml::I1);
		generate_load_operand(block, compiler, LOCAL, dst_code, uml::I0, uml::I3);
		UML_AND(block, I1, I1, 0x1f);
	}
	else
	{
		generate_load_operand(block, compiler, LOCAL, dst_code, uml::I0, uml::I3);
		UML_AND(block, I1, I0, 0x1f);
	}

	UML_AND(block, I2, I2, ~(C_MASK | Z_MASK | N_MASK | V_MASK));

	UML_SUB(block, I4, 32, I1);
	UML_SHR(block, I4, ~uint32_t(0), I4);
	UML_TEST(block, I1, 0xffffffff);
	UML_MOVc(block, uml::COND_Z, I4, 0);
	UML_MOV(block, I5, I4);

	UML_ROL(block, I0, I0, I1);
	UML_MOVc(block, uml::COND_NS, I4, 0);
	generate_update_nz(block, compiler, uml::I2);

	UML_AND(block, I1, I5, I0);
	UML_ROLINS(block, I2, I1, C_SHIFT, C_MASK);
	UML_CMP(block, I4, I1);
	UML_MOV(block, I1, V_MASK);
	UML_MOVc(block, uml::COND_E, I1, 0);
	UML_OR(block, I2, I2, I1);

	UML_MOV(block, DRC_SR, I2);

	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_ldxx1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t srcf_code = src_code + 1;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const auto [sub_type, extra_s] = generate_get_d_code_dis(desc);

	if (!DstGlobal || !SrcGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	const uml::parameter dstp = generate_load_address_ad(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I0);

	switch (sub_type)
	{
		case 0: // LDBS.D/A
		case 1: // LDBU.D/A
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			generate_add_dis(block, compiler, uml::I0, dstp, extra_s, 1);
			UML_CALLH(block, *m_mem_read8);
			if (sub_type == 0) // LDBS.D/A
				UML_SEXT(block, I1, I1, SIZE_BYTE);

			if (SrcGlobal)
			{
				generate_set_global_register_low(block, compiler, desc, src_code, uml::I1);
				if (src_code == PC_REGISTER)
				{
					UML_AND(block, I1, I1, ~uint32_t(1));
					generate_branch(block, compiler, compiler.mode(), uml::I1, desc);
				}
			}
			else
			{
				UML_ADD(block, I3, I3, src_code);
				UML_AND(block, I3, I3, 0x3f);
				UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
			}
			break;

		case 2:
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			generate_add_dis(block, compiler, uml::I0, dstp, extra_s, 2);
			UML_CALLH(block, *m_mem_read16);
			if (extra_s & 1) // LDHS.A/D
				UML_SEXT(block, I1, I1, SIZE_WORD);

			if (SrcGlobal)
			{
				generate_set_global_register_low(block, compiler, desc, src_code, uml::I1);
				if (src_code == PC_REGISTER)
				{
					UML_AND(block, I1, I1, ~uint32_t(1));
					generate_branch(block, compiler, compiler.mode(), uml::I1, desc);
				}
			}
			else
			{
				UML_ADD(block, I3, I3, src_code);
				UML_AND(block, I3, I3, 0x3f);
				UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
			}
			break;

		case 3:
			generate_add_dis(block, compiler, uml::I0, dstp, extra_s, 4);
			switch (extra_s & 3)
			{
				case 0: // LDW.D/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_CALLH(block, *m_mem_read32);

					if (SrcGlobal)
					{
						generate_set_global_register_low(block, compiler, desc, src_code, uml::I1);
						if (src_code == PC_REGISTER)
						{
							UML_AND(block, I1, I1, ~uint32_t(1));
							generate_branch(block, compiler, compiler.mode(), uml::I1, desc);
						}
					}
					else
					{
						UML_ADD(block, I3, I3, src_code);
						UML_AND(block, I3, I3, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
					}
					break;

				case 1: // LDD.D/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_CALLH(block, *m_mem_read32);
					generate_set_register(block, compiler, desc, SrcGlobal, src_code, uml::I1, uml::I2, true);

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_mem_read32);
					generate_set_register(block, compiler, desc, SrcGlobal, srcf_code, uml::I1, uml::I3, true);

					if (src_code == PC_REGISTER)
						generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
					break;

				case 2: // LDW.IOD/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_CALLH(block, *m_io_read32);

					if (SrcGlobal)
					{
						generate_set_global_register_low(block, compiler, desc, src_code, uml::I1);
						if (src_code == PC_REGISTER)
						{
							UML_AND(block, I1, I1, ~uint32_t(1));
							generate_branch(block, compiler, compiler.mode(), uml::I1, desc);
						}
					}
					else
					{
						UML_ADD(block, I3, I3, src_code);
						UML_AND(block, I3, I3, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
					}
					break;

				case 3: // LDD.IOD/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_CALLH(block, *m_io_read32);
					generate_set_register(block, compiler, desc, SrcGlobal, src_code, uml::I1, uml::I2, true);

					UML_ADD(block, I0, I0, 1 << 13);
					UML_CALLH(block, *m_io_read32);
					generate_set_register(block, compiler, desc, SrcGlobal, srcf_code, uml::I1, uml::I3, true);

					if (src_code == PC_REGISTER)
						generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
					break;
			}
			break;
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_ldxx2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t srcf_code = src_code + 1;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const auto [sub_type, extra_s] = generate_get_d_code_dis(desc);

	if (DstGlobal && (dst_code <= SR_REGISTER))
	{
		osd_printf_error("Denoted PC or SR in hyperstone_ldxx2. PC = %08X\n", desc->pc);
		return;
	}

	if (!DstGlobal || !SrcGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	generate_load_address_ns(block, compiler, desc, DstGlobal, dst_code, uml::I6, uml::I2, sub_type, extra_s);

	switch (sub_type)
	{
		case 0: // LDBS.N
		case 1: // LDBU.N
		case 2: // LDHS.N, LDHU.N
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			if (sub_type == 2)
			{
				UML_AND(block, I0, I6, ~1);
				UML_CALLH(block, *m_mem_read16);
				if (extra_s & 1)
					UML_SEXT(block, I1, I1, SIZE_WORD);
			}
			else
			{
				UML_MOV(block, I0, I6);
				UML_CALLH(block, *m_mem_read8);
				if (sub_type == 0)
					UML_SEXT(block, I1, I1, SIZE_BYTE);
			}

			generate_set_dst(block, compiler, desc, SrcGlobal, src_code, uml::I1, uml::I3, true);

			if ((DstGlobal != SrcGlobal) || (src_code != dst_code))
			{
				if (sub_type == 2)
					UML_ADD(block, I4, I6, extra_s & ~1);
				else
					UML_ADD(block, I4, I6, extra_s);

				generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I4, uml::I2, false);
			}
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // LDW.N
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_AND(block, I0, I6, ~3);
					UML_CALLH(block, *m_mem_read32);

					generate_set_dst(block, compiler, desc, SrcGlobal, src_code, uml::I1, uml::I3, true);

					if ((DstGlobal != SrcGlobal) || (src_code != dst_code))
					{
						UML_ADD(block, I4, I6, extra_s & ~3);

						generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I4, uml::I2, false);
					}
					break;

				case 1: // LDD.N
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_AND(block, I0, I6, ~3);
					UML_CALLH(block, *m_mem_read32);

					if (SrcGlobal)
					{
						generate_set_global_register_low(block, compiler, desc, src_code, uml::I1);

						UML_ADD(block, I0, I0, 4);
						UML_CALLH(block, *m_mem_read32);

						generate_set_global_register_low(block, compiler, desc, srcf_code, uml::I1);
					}
					else
					{
						UML_ADD(block, I3, I3, src_code);
						UML_AND(block, I3, I3, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);

						UML_ADD(block, I0, I0, 4);
						UML_CALLH(block, *m_mem_read32);

						UML_ADD(block, I3, I3, 1);
						UML_AND(block, I3, I3, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
					}

					if ((DstGlobal != SrcGlobal) || (src_code != dst_code))
					{
						UML_ADD(block, I4, I6, extra_s & ~3);

						generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I4, uml::I2, false);
					}
					break;

				case 2: // Reserved
					osd_printf_error("Reserved instruction in generate_ldxx2. PC = %08X\n", desc->pc);
					break;

				case 3: // LDW.S
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_3));

					const int below_sp = compiler.next_label();
					const int done = compiler.next_label();

					UML_CMP(block, I6, mem(&m_core->global_regs[SP_REGISTER]));
					UML_JMPc(block, uml::COND_B, below_sp);

					UML_BFXU(block, I0, I6, 2, 6);
					UML_LOAD(block, I1, (void *)m_core->local_regs, I0, SIZE_DWORD, SCALE_x4);
					UML_JMP(block, done);

					UML_LABEL(block, below_sp);
					UML_AND(block, I0, I6, ~3);
					UML_CALLH(block, *m_mem_read32);

					UML_LABEL(block, done);

					generate_set_dst(block, compiler, desc, SrcGlobal, src_code, uml::I1, uml::I3, true);

					if ((DstGlobal != SrcGlobal) || (src_code != dst_code))
					{
						UML_ADD(block, I4, I6, extra_s & ~3);

						generate_set_dst(block, compiler, desc, DstGlobal, dst_code, uml::I4, uml::I2, false);
					}
					break;
				}
			}
			break;
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_stxx1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const auto [sub_type, extra_s] = generate_get_d_code_dis(desc);

	if (!DstGlobal || !SrcGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	const uml::parameter dstp = generate_load_address_ad(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I0);

	if (SrcGlobal)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_MOV(block, I1, mem(&m_core->global_regs[src_code]));
	}
	else
	{
		UML_ADD(block, I1, I3, src_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	switch (sub_type)
	{
		case 0: // STBS.D/A
		case 1: // STBU.D/A
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			generate_add_dis(block, compiler, uml::I0, dstp, extra_s, 1);
			UML_CALLH(block, *m_mem_write8);

			if (sub_type == 0)
			{
				const int no_exception = compiler.next_label();
				UML_TEST(block, I1, 0xffffff00);
				UML_JMPc(block, uml::COND_Z, no_exception);
				UML_SEXT(block, I0, I1, SIZE_BYTE);
				UML_CMP(block, I0, I1);
				UML_JMPc(block, uml::COND_E, no_exception);
				generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR);
				UML_LABEL(block, no_exception);
			}
			break;

		case 2: // STHS.D/A, STHU.D/A
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			generate_add_dis(block, compiler, uml::I0, dstp, extra_s, 2);
			UML_CALLH(block, *m_mem_write16);

			if (extra_s & 1)
			{
				const int no_exception = compiler.next_label();
				UML_TEST(block, I1, 0xffff0000);
				UML_JMPc(block, uml::COND_Z, no_exception);
				UML_SEXT(block, I0, I1, SIZE_WORD);
				UML_CMP(block, I0, I1);
				UML_JMPc(block, uml::COND_E, no_exception);
				generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR);
				UML_LABEL(block, no_exception);
			}
			break;

		case 3:
			generate_add_dis(block, compiler, uml::I0, dstp, extra_s, 4);
			switch (extra_s & 3)
			{
				case 0: // STW.D/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_CALLH(block, *m_mem_write32);
					break;
				case 1: // STD.D/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_CALLH(block, *m_mem_write32);

					if (SrcGlobal)
					{
						if (src_code == SR_REGISTER)
							UML_MOV(block, I1, 0);
						else
							UML_MOV(block, I1, mem(&m_core->global_regs[src_code + 1]));
					}
					else
					{
						UML_ADD(block, I3, I3, src_code + 1);
						UML_AND(block, I3, I3, 0x3f);
						UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
					}

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_mem_write32);
					break;
				case 2: // STW.IOD/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_CALLH(block, *m_io_write32);
					break;
				case 3: // STD.IOD/A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_CALLH(block, *m_io_write32);

					if (SrcGlobal)
					{
						if (src_code == SR_REGISTER)
							UML_MOV(block, I1, 0);
						else
							UML_MOV(block, I1, mem(&m_core->global_regs[src_code + 1]));
					}
					else
					{
						UML_ADD(block, I3, I3, src_code + 1);
						UML_AND(block, I3, I3, 0x3f);
						UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
					}

					UML_ADD(block, I0, I0, 1 << 13);
					UML_CALLH(block, *m_io_write32);
					break;
			}
			break;
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_stxx2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const auto [sub_type, extra_s] = generate_get_d_code_dis(desc);

	if (DstGlobal && (dst_code <= SR_REGISTER))
	{
		osd_printf_error("Denoted PC or SR in hyperstone_stxx2. PC = %08X\n", desc->pc);
		UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
		return;
	}

	if (!DstGlobal || !SrcGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	generate_load_address_ns(block, compiler, desc, DstGlobal, dst_code, uml::I0, uml::I6, sub_type, extra_s);

	if (SrcGlobal)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_MOV(block, I1, mem(&m_core->global_regs[src_code]));
	}
	else
	{
		UML_ADD(block, I1, I3, src_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	switch (sub_type)
	{
		case 0: // STBS.N
		case 1: // STBU.N
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_CALLH(block, *m_mem_write8);

			UML_ADD(block, I0, I0, extra_s);
			if (DstGlobal)
				UML_MOV(block, mem(&m_core->global_regs[dst_code]), I0);
			else
				UML_STORE(block, (void *)m_core->local_regs, I6, I0, SIZE_DWORD, SCALE_x4);

			if (sub_type == 0)
			{
				const int no_exception = compiler.next_label();
				UML_TEST(block, I1, 0xffffff00);
				UML_JMPc(block, uml::COND_Z, no_exception);
				UML_SEXT(block, I0, I1, SIZE_BYTE);
				UML_CMP(block, I0, I1);
				UML_JMPc(block, uml::COND_E, no_exception);
				generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR);
				UML_LABEL(block, no_exception);
			}
			break;

		case 2: // STHS.N, STHU.N
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_MOV(block, I4, I0);
			UML_AND(block, I0, I0, ~1);
			UML_CALLH(block, *m_mem_write16);

			UML_ADD(block, I4, I4, extra_s & ~1);
			if (DstGlobal)
				UML_MOV(block, mem(&m_core->global_regs[dst_code]), I4);
			else
				UML_STORE(block, (void *)m_core->local_regs, I6, I4, SIZE_DWORD, SCALE_x4);

			if (extra_s & 1)
			{
				const int no_exception = compiler.next_label();
				UML_TEST(block, I1, 0xffff0000);
				UML_JMPc(block, uml::COND_Z, no_exception);
				UML_SEXT(block, I0, I1, SIZE_WORD);
				UML_CMP(block, I0, I1);
				UML_JMPc(block, uml::COND_E, no_exception);
				generate_raise_exception(block, compiler, desc, TRAPNO_RANGE_ERROR);
				UML_LABEL(block, no_exception);
			}
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.N
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_MOV(block, I5, I0);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);
					UML_ADD(block, I5, I5, extra_s);

					if (DstGlobal)
						UML_MOV(block, mem(&m_core->global_regs[dst_code]), I5);
					else
						UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
					break;

				case 1: // STD.N
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_MOV(block, I5, I0);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);

					UML_ADD(block, I5, I5, extra_s & ~1);
					if (DstGlobal)
						UML_MOV(block, mem(&m_core->global_regs[dst_code]), I5);
					else
						UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);

					generate_load_operand(block, compiler, SrcGlobal, src_code + 1, uml::I1, uml::I4);

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_mem_write32);
					break;

				case 2: // Reserved
					osd_printf_error("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", desc->pc);
					break;

				case 3: // STW.S
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_3));

					int less_than_sp = compiler.next_label();
					int store_done = compiler.next_label();

					UML_MOV(block, I5, I0);
					UML_CMP(block, I5, mem(&SP));
					UML_JMPc(block, uml::COND_B, less_than_sp);

					UML_BFXU(block, I4, I0, 2, 6);
					UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
					UML_JMP(block, store_done);

					UML_LABEL(block, less_than_sp);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);

					UML_LABEL(block, store_done);
					UML_ADD(block, I5, I5, extra_s & ~3);
					if (DstGlobal)
						UML_MOV(block, mem(&m_core->global_regs[dst_code]), I5);
					else
						UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
					break;
				}
			}
			break;
	}
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal, hyperstone_device::sign_mode SIGNED>
void hyperstone_device::generate_mulsu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_36));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;
	const uint32_t src_code = op & 0xf;

	if ((SrcGlobal && src_code < 2) || (DstGlobal && dst_code < 2))
	{
		osd_printf_error("Denoted PC or SR in hyperstone_muls/u instruction. PC = %08X\n", desc->pc);
		return;
	}

	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I0, uml::I4);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I1, uml::I6);

	if (SIGNED == IS_SIGNED)
		UML_MULS(block, I4, I5, I0, I1);
	else
		UML_MULU(block, I4, I5, I0, I1);

	UML_SETc(block, uml::COND_Z, I2);
	UML_SHL(block, I2, I2, Z_SHIFT);
	UML_ROLINS(block, I2, I5, N_SHIFT + 1, N_MASK);
	UML_ROLINS(block, DRC_SR, I2, 0, (N_MASK | Z_MASK));

	if (DstGlobal)
	{
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), I5);
		UML_MOV(block, mem(&m_core->global_regs[dstf_code]), I4);
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
		UML_ADD(block, I2, I3, dstf_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
	}

	int done = compiler.next_label();
	UML_MOV(block, I7, mem(&m_core->clock_cycles_6));
	if (SIGNED == IS_SIGNED)
	{
		UML_CMP(block, I0, 0xffff8000);
		UML_JMPc(block, uml::COND_B, done);
		UML_CMP(block, I0, 0x00008000);
		UML_JMPc(block, uml::COND_AE, done);
		UML_CMP(block, I1, 0xffff8000);
		UML_JMPc(block, uml::COND_B, done);
		UML_CMP(block, I1, 0x00008000);
		UML_JMPc(block, uml::COND_AE, done);
	}
	else
	{
		UML_CMP(block, I0, 0x0000ffff);
		UML_JMPc(block, uml::COND_A, done);
		UML_CMP(block, I1, 0x0000ffff);
		UML_JMPc(block, uml::COND_A, done);
	}
	UML_SUB(block, I7, I7, mem(&m_core->clock_cycles_2));

	UML_LABEL(block, done);
}


template <hyperstone_device::reg_bank DstGlobal, hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_mul(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if ((SrcGlobal && src_code < 2) || (DstGlobal && dst_code < 2))
	{
		osd_printf_error("Denoted PC or SR in hyperstone_mul instruction. PC = %08X\n", desc->pc);
		return;
	}

	if (!SrcGlobal || !DstGlobal)
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	generate_load_operand(block, compiler, SrcGlobal, src_code, uml::I0, uml::I1);
	generate_load_operand(block, compiler, DstGlobal, dst_code, uml::I1, uml::I6);

	UML_MOV(block, I7, mem(&m_core->clock_cycles_3));
	const int add_cycles = compiler.next_label();
	const int set_cycles = compiler.next_label();
	UML_CMP(block, I0, 0xffff8000);
	UML_JMPc(block, uml::COND_L, add_cycles);
	UML_CMP(block, I0, 0x00008000);
	UML_JMPc(block, uml::COND_GE, add_cycles);
	UML_CMP(block, I1, 0xffff8000);
	UML_JMPc(block, uml::COND_L, add_cycles);
	UML_CMP(block, I1, 0x00008000);
	UML_JMPc(block, uml::COND_L, set_cycles);
	UML_LABEL(block, add_cycles);
	UML_ADD(block, I7, I7, mem(&m_core->clock_cycles_2));
	UML_LABEL(block, set_cycles);

	UML_MULULW(block, I2, I0, I1);

	UML_SETc(block, uml::COND_Z, I3);
	UML_SHL(block, I3, I3, Z_SHIFT);
	UML_ROLINS(block, I3, I2, N_SHIFT + 1, N_MASK);
	UML_ROLINS(block, DRC_SR, I3, 0, (Z_MASK | N_MASK));

	if (DstGlobal)
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), I2);
	else
		UML_STORE(block, (void *)m_core->local_regs, I6, I2, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::shift_type HiN, hyperstone_device::reg_bank DstGlobal>
void hyperstone_device::generate_set(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = op & 0xf;

	if (DstGlobal && dst_code < 2)
	{
		return;
	}

	if (HiN)
	{
		if (n >= 4 || n == 2)
		{
			static const uint32_t   set_result[16] = { 0, 0, 0,          0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0 };
			static const uint32_t unset_result[16] = { 0, 0, 0xffffffff, 0,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff };
			static const uint32_t mask[16] = { 0, 0, 0, 0, (N_MASK | Z_MASK), (N_MASK | Z_MASK), N_MASK, N_MASK,
				(C_MASK | Z_MASK), (C_MASK | Z_MASK), C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, V_MASK };

			UML_TEST(block, DRC_SR, mask[n]);
			UML_MOVc(block, uml::COND_NZ, I0, set_result[n]);
			UML_MOVc(block, uml::COND_Z, I0, unset_result[n]);
		}
		else
		{
			osd_printf_error("Used reserved N value (%d) in hyperstone_set. PC = %08X\n", n, desc->pc);
			return;
		}
	}
	else
	{
		if (n == 0)
		{
			int no_low_bit = compiler.next_label();
			UML_MOV(block, I1, mem(&m_core->global_regs[SP_REGISTER]));
			UML_AND(block, I0, I1, 0xfffffe00);
			UML_ROLINS(block, I0, DRC_SR, 32 - FP_SHIFT + 2, 0x000001fc);
			UML_TEST(block, I1, 0x100);
			UML_JMPc(block, uml::COND_Z, no_low_bit);
			UML_TEST(block, DRC_SR, 0x80000000);
			UML_JMPc(block, uml::COND_NZ, no_low_bit);
			UML_OR(block, I0, I0, 1);
			UML_LABEL(block, no_low_bit);
		}
		else if (n >= 2)
		{
			static const uint32_t   set_result[16] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
			static const uint32_t unset_result[16] = { 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
			static const uint32_t mask[16] = { 0, 0, 0, 0, (N_MASK | Z_MASK), (N_MASK | Z_MASK), N_MASK, N_MASK,
				(C_MASK | Z_MASK), (C_MASK | Z_MASK), C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, V_MASK };

			UML_TEST(block, DRC_SR, mask[n]);
			UML_MOVc(block, uml::COND_NZ, I0, set_result[n]);
			UML_MOVc(block, uml::COND_Z, I0, unset_result[n]);
		}
		else
		{
			osd_printf_error("Used reserved N value (%d) in hyperstone_set. PC = %08X\n", n, desc->pc);
			return;
		}
	}

	if (DstGlobal)
	{
		UML_MOV(block, mem(&m_core->global_regs[dst_code]), I0);
	}
	else
	{
		UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);
		UML_ADD(block, I3, I3, dst_code);
		UML_AND(block, I3, I3, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_ldwr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal || (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I0, uml::I2, 0);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I0, uml::I3, 0);

	UML_CALLH(block, *m_mem_read32);

	if (SrcGlobal || (src_code != dst_code))
		generate_set_dst(block, compiler, desc, SrcGlobal, src_code, uml::I1, uml::I3, true);
	else
		UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_lddr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (!SrcGlobal && (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I0, uml::I2, 0);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I0, uml::I3, 0);

	UML_CALLH(block, *m_mem_read32);

	if (SrcGlobal)
	{
		generate_set_global_register_low(block, compiler, desc, src_code, uml::I1);

		UML_ADD(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_read32);

		generate_set_global_register_low(block, compiler, desc, src_code + 1, uml::I1);
	}
	else
	{
		if (src_code != dst_code)
		{
			UML_ADD(block, I3, I3, src_code);
			UML_AND(block, I3, I3, 0x3f);
		}
		UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);

		UML_ADD(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_read32);

		UML_ADD(block, I3, I3, 1);
		UML_AND(block, I3, I3, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_ldwp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal || (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I4, uml::I2, 4);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I4, uml::I3, 4);

	UML_CALLH(block, *m_mem_read32);

	if (SrcGlobal || (src_code != dst_code))
	{
		UML_ADD(block, I4, I4, 4);
		UML_STORE(block, (void *)m_core->local_regs, I2, I4, SIZE_DWORD, SCALE_x4);

		generate_set_dst(block, compiler, desc, SrcGlobal, src_code, uml::I1, uml::I3, true);
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_lddp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal || (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I4, uml::I2, 8);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I4, uml::I3, 8);

	UML_CALLH(block, *m_mem_read32);

	if (SrcGlobal || ((src_code != dst_code) && ((src_code + 1) != dst_code)))
	{
		UML_ADD(block, I4, I4, 8);
		UML_STORE(block, (void *)m_core->local_regs, I2, I4, SIZE_DWORD, SCALE_x4);
	}

	if (SrcGlobal)
	{
		generate_set_global_register_low(block, compiler, desc, src_code, uml::I1);

		UML_ADD(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_read32);

		generate_set_global_register_low(block, compiler, desc, src_code + 1, uml::I1);
	}
	else
	{
		if (src_code != dst_code)
		{
			UML_ADD(block, I3, I3, src_code);
			UML_AND(block, I3, I3, 0x3f);
		}
		UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);

		UML_ADD(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_read32);

		UML_ADD(block, I3, I3, 1);
		UML_AND(block, I3, I3, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_stwr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal || (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I0, uml::I2, 0);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I1, uml::I3, 0);

	if (SrcGlobal)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_MOV(block, I1, mem(&m_core->global_regs[src_code]));
	}
	else if (src_code != dst_code)
	{
		UML_ADD(block, I3, I3, src_code);
		UML_AND(block, I3, I3, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
	}

	UML_CALLH(block, *m_mem_write32);
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_stdr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal || (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I0, uml::I2, 0);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I1, uml::I3, 0);

	uml::parameter srcfp = uml::I2;
	if (SrcGlobal)
	{
		if (src_code == SR_REGISTER)
		{
			UML_MOV(block, I1, 0);
			srcfp = uml::I1;
		}
		else
		{
			UML_MOV(block, I1, mem(&m_core->global_regs[src_code]));
			UML_MOV(block, srcfp, mem(&m_core->global_regs[src_code + 1]));
		}
	}
	else
	{
		if (src_code != dst_code)
		{
			UML_ADD(block, I3, I3, src_code);
			UML_AND(block, I3, I3, 0x3f);
			UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
		}
		UML_ADD(block, I3, I3, 1);
		UML_AND(block, I3, I3, 0x3f);
		UML_LOAD(block, srcfp, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
	}

	UML_CALLH(block, *m_mem_write32);
	UML_ADD(block, I0, I0, 4);
	if (srcfp != uml::I1)
		UML_MOV(block, I1, srcfp);
	UML_CALLH(block, *m_mem_write32);
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_stwp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal || (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I4, uml::I2, 4);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I1, uml::I3, 4);

	if (SrcGlobal)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_MOV(block, I1, mem(&m_core->global_regs[src_code]));
	}
	else if (src_code != dst_code)
	{
		UML_ADD(block, I3, I3, src_code);
		UML_AND(block, I3, I3, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
	}

	UML_CALLH(block, *m_mem_write32);

	if (SrcGlobal || (src_code != dst_code))
	{
		UML_ADD(block, I4, I4, 4);
		UML_STORE(block, (void *)m_core->local_regs, I2, I4, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I1, I1, 4);
		UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_stdp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal || (src_code != dst_code))
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I4, uml::I2, 8);
	else
		generate_load_address_rp(block, compiler, desc, dst_code, uml::I1, uml::I3, 8);

	if (SrcGlobal)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_MOV(block, I1, mem(&m_core->global_regs[src_code]));

		UML_CALLH(block, *m_mem_write32);

		UML_ADD(block, I4, I4, 8);
		UML_STORE(block, (void *)m_core->local_regs, I2, I4, SIZE_DWORD, SCALE_x4);

		if (src_code != SR_REGISTER)
			UML_MOV(block, I1, mem(&m_core->global_regs[src_code + 1]));
	}
	else
	{
		if (src_code != dst_code)
		{
			UML_ADD(block, I3, I3, src_code);
			UML_AND(block, I3, I3, 0x3f);
			UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
		}

		UML_CALLH(block, *m_mem_write32);

		if (src_code == dst_code)
		{
			UML_ADD(block, I1, I1, 8);
			UML_STORE(block, (void *)m_core->local_regs, I3, I1, SIZE_DWORD, SCALE_x4);
		}
		else
		{
			UML_ADD(block, I4, I4, 8);
			UML_STORE(block, (void *)m_core->local_regs, I2, I4, SIZE_DWORD, SCALE_x4);
		}

		if ((src_code + 1) == dst_code)
		{
			UML_MOV(block, I1, I4);
		}
		else
		{
			UML_ADD(block, I3, I3, 1);
			UML_AND(block, I3, I3, 0x3f);
			UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
		}
	}

	UML_ADD(block, I0, I0, 4);
	UML_CALLH(block, *m_mem_write32);
}


template <hyperstone_device::branch_condition Condition, hyperstone_device::condition_set CondSet>
void hyperstone_device::generate_b(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	static const uint32_t condition_masks[6] = { V_MASK, Z_MASK, C_MASK, C_MASK | Z_MASK, N_MASK, N_MASK | Z_MASK };

	uml::condition_t condition = CondSet ? uml::COND_Z : uml::COND_NZ;

	const int skip = compiler.next_label();
	UML_TEST(block, DRC_SR, condition_masks[Condition]);
	UML_JMPc(block, condition, skip);
	generate_br(block, compiler, desc);

	UML_LABEL(block, skip);
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
}


void hyperstone_device::generate_br(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint32_t target = generate_get_pcrel(desc);

	UML_ADD(block, DRC_PC, DRC_PC, target);
	UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);

	generate_branch(block, compiler, compiler.mode(), desc->targetpc, desc);
	// TODO: correct cycle count
}


template <hyperstone_device::branch_condition Condition, hyperstone_device::condition_set CondSet>
void hyperstone_device::generate_db(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	static const uint32_t condition_masks[6] = { V_MASK, Z_MASK, C_MASK, C_MASK | Z_MASK, N_MASK, N_MASK | Z_MASK };
	const int skip_jump = compiler.next_label();
	const int done = compiler.next_label();

	UML_TEST(block, DRC_SR, condition_masks[Condition]);
	if (CondSet)
		UML_JMPc(block, uml::COND_Z, skip_jump);
	else
		UML_JMPc(block, uml::COND_NZ, skip_jump);

	generate_dbr(block, compiler, desc);
	UML_JMP(block, done);

	UML_LABEL(block, skip_jump);
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	UML_LABEL(block, done);
}


void hyperstone_device::generate_dbr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint32_t target = generate_get_pcrel(desc);

	UML_ADD(block, I0, DRC_PC, target);
	UML_MOV(block, mem(&m_core->delay_slot), 1);
	UML_MOV(block, mem(&m_core->delay_pc), I0);
	UML_MOV(block, mem(&m_core->intblock), 2);

	auto const *delayslot = desc->delay.first();
	if (delayslot)
	{
		assert(desc->targetpc != BRANCH_TARGET_DYNAMIC);
		assert(!delayslot->next());

		UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT) | P_MASK, 0, ILC_MASK | P_MASK);

		generate_update_cycles(block);

		UML_MOV(block, mem(&m_core->intblock), 1);

#if E132XS_LOG_DRC_REGS
		UML_CALLC(block, &c_funcs::dump_registers, this);
#endif

		// if we are debugging, call the debugger
		if (debugger_enabled())
		{
			//save_fast_iregs(block);
			UML_DEBUG(block, delayslot->pc);
		}

		// set the PC map variable
		UML_MAPVAR(block, MAPVAR_PC, compiler.set_pc(desc->targetpc));

		UML_MOV(block, DRC_PC, I0);
		UML_MOV(block, mem(&m_core->delay_slot), 0);
		UML_MOV(block, mem(&m_core->delay_slot_taken), 1);

		if (generate_opcode(block, compiler, delayslot))
		{
			UML_MOV(block, mem(&m_core->delay_slot_taken), 0);
			generate_update_cycles(block);
			if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
				UML_JMP(block, desc->targetpc | 0x80000000);
			else
				UML_HASHJMP(block, compiler.mode(), desc->targetpc, *m_nocode);
		}
		else
		{
			UML_MOV(block, mem(&m_core->arg0), delayslot->opptr.w[0]);
			UML_CALLC(block, &c_funcs::unimplemented, this);
		}
	}

	compiler.m_check_delay = 2;
}


void hyperstone_device::generate_frame(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_MOV(block, I2, DRC_SR);                                // I2 = SR
	UML_BFXU(block, I1, I2, FP_SHIFT, 7);                      // I1 = FP -= Ls
	UML_SUB(block, I1, I1, op & 0xf);
	UML_ROLAND(block, I0, I1, FP_SHIFT, FP_MASK);
	UML_OR(block, I0, I0, dst_code << FL_SHIFT);               // FL = Ld
	UML_ROLINS(block, I2, I0, 0, FP_MASK | FL_MASK | M_MASK);  // clear M as well
	UML_MOV(block, DRC_SR, I2);                                // update SR

	const int done = compiler.next_label();
	UML_AND(block, I0, mem(&SP), ~uint32_t(3));
	UML_ADD(block, I1, I1, dst_code ? dst_code : 16);          // difference = ((SP & 0x1fc) >> 2) + (64 - 10) - ((GET_FP - SRC_CODE) + GET_FL)
	UML_ROLAND(block, I3, I0, 30, 0x7f);
	UML_ADD(block, I3, I3, (64 - 10));
	UML_SUB(block, I3, I3, I1);
	UML_BFXS(block, I3, I3, 0, 7);                             // sign-extend 7-bit value
	UML_JMPc(block, uml::COND_NS, done);

	UML_CMP(block, I0, mem(&UB));                              // check stack pointer against upper bound
	UML_SETc(block, uml::COND_AE, I4);

	const int push_next = compiler.next_label();
	UML_LABEL(block, push_next);
	UML_AND(block, I2, I0, 0x3f << 2);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x1);
	UML_CALLH(block, *m_mem_write32);
	UML_ADD(block, I0, I0, 4);
	UML_ADD(block, I3, I3, 1);
	UML_JMPc(block, uml::COND_S, push_next);

	UML_MOV(block, mem(&SP), I0);

	UML_TEST(block, I4, ~uint32_t(0));
	UML_JMPc(block, uml::COND_Z, done);
	generate_raise_exception(block, compiler, desc, TRAPNO_FRAME_ERROR);

	UML_LABEL(block, done);
}

template <hyperstone_device::reg_bank SrcGlobal>
void hyperstone_device::generate_call(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
	UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT) | P_MASK, 0, ILC_MASK | P_MASK);

	const uint16_t op = desc->opptr.w[0];
	uint16_t imm_1 = m_pr16(desc->pc + 2);

	int32_t extra_s = 0;

	if (imm_1 & 0x8000)
	{
		uint16_t imm_2 = m_pr16(desc->pc + 4);

		extra_s = imm_2;
		extra_s |= ((imm_1 & 0x3fff) << 16);

		if (imm_1 & 0x4000)
			extra_s |= 0xc0000000;
	}
	else
	{
		extra_s = imm_1 & 0x3fff;

		if (imm_1 & 0x4000)
			extra_s |= 0xffffc000;
	}

	UML_MOV(block, I1, extra_s);

	const uint32_t src_code = op & 0xf;
	uint32_t dst_code = (op & 0xf0) >> 4;

	if (!dst_code)
		dst_code = 16;

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	if (SrcGlobal)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I2, 0);
		else
			UML_MOV(block, I2, mem(&m_core->global_regs[src_code]));
	}
	else
	{
		UML_ADD(block, I4, I3, src_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, I4, DRC_PC, ~1);
	UML_ROLINS(block, I4, DRC_SR, 32 - S_SHIFT, 1);

	UML_ADD(block, I1, I3, dst_code);
	UML_AND(block, I6, I1, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I6, I4, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I4, I6, 1);
	UML_AND(block, I5, I4, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I5, DRC_SR, SIZE_DWORD, SCALE_x4);

	UML_ROLINS(block, DRC_SR, I1, FP_SHIFT, FP_MASK);
	UML_ROLINS(block, DRC_SR, 6, FL_SHIFT, FL_MASK);
	UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);

	UML_ADD(block, I2, I2, extra_s & ~uint32_t(1));
	UML_MOV(block, DRC_PC, I2);

	UML_MOV(block, mem(&m_core->intblock), 2);

	generate_branch(block, compiler, compiler.mode(), uml::I2, nullptr);
	//TODO: add interrupt locks, errors, ....
}



void hyperstone_device::generate_trap_op(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1)); // TODO: with the latency it can change

	static const uint32_t conditions[16] = {
		0, 0, 0, 0, N_MASK | Z_MASK, N_MASK | Z_MASK, N_MASK, N_MASK, C_MASK | Z_MASK, C_MASK | Z_MASK, C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, 0
	};
	static const bool trap_if_set[16] = {
		false, false, false, false, true, false, true, false, true, false, true, false, true, false, true, false
	};

	const uint16_t op = desc->opptr.w[0];
	const uint8_t trapno = (op & 0xfc) >> 2;
	const uint8_t code = ((op & 0x300) >> 6) | (op & 0x03);

	UML_TEST(block, DRC_SR, conditions[code]);

	const int skip_trap = compiler.next_label();
	if (trap_if_set[code])
		UML_JMPc(block, uml::COND_Z, skip_trap);
	else
		UML_JMPc(block, uml::COND_NZ, skip_trap);

	UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT) | P_MASK, 0, ILC_MASK | P_MASK);
	generate_trap_exception_or_int<IS_TRAP>(block, compiler.m_labelnum, trapno);

	UML_LABEL(block, skip_trap);
}

void hyperstone_device::generate_extend(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	const uint16_t func = m_pr16(desc->pc + 2);

	UML_BFXU(block, I3, DRC_SR, FP_SHIFT, 7);

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I2, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4); // I0: vals

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I2, I2, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4); // I1: vald

	switch (func)
	{
		// signed or unsigned multiplication, single word product
		case EMUL:
		case EMUL_N: // used in "N" type cpu
			UML_MULULW(block, mem(&m_core->global_regs[15]), I0, I1);
			break;

		case EMULU: // unsigned multiplication, double word product
			UML_MULU(block, mem(&m_core->global_regs[15]), mem(&m_core->global_regs[14]), I0, I1);
			break;

		case EMULS: // signed multiplication, double word product
			UML_MULS(block, mem(&m_core->global_regs[15]), mem(&m_core->global_regs[14]), I0, I1);
			break;

		case EMAC:  // signed multiply/add, single word product sum
		case EMSUB: // signed multiply/substract, single word product difference
			UML_MULSLW(block, I2, I0, I1);
			if (func == EMAC)
				UML_ADD(block, mem(&m_core->global_regs[15]), mem(&m_core->global_regs[15]), I2);
			else
				UML_SUB(block, mem(&m_core->global_regs[15]), mem(&m_core->global_regs[15]), I2);
			break;

		case EMACD:  // signed multiply/add, double word product sum
		case EMSUBD: // signed multiply/substract, double word product difference
			UML_DSEXT(block, I0, I0, SIZE_DWORD);
			UML_DSEXT(block, I1, I1, SIZE_DWORD);
			UML_DMULSLW(block, I2, I0, I1);
			UML_MOV(block, I3, mem(&m_core->global_regs[14]));
			UML_MOV(block, I4, mem(&m_core->global_regs[15]));
			UML_DSHL(block, I3, I3, 32);
			UML_DOR(block, I3, I3, I4);
			if (func == EMACD)
				UML_DADD(block, I3, I3, I2);
			else
				UML_DSUB(block, I3, I3, I2);
			UML_MOV(block, mem(&m_core->global_regs[15]), I3);
			UML_DSHR(block, I3, I3, 32);
			UML_MOV(block, mem(&m_core->global_regs[14]), I3);
			break;

		// signed half-word multiply/add, single word product sum
		case EHMAC:
			UML_SEXT(block, I2, I0, SIZE_WORD);
			UML_SEXT(block, I3, I1, SIZE_WORD);
			UML_MULSLW(block, I2, I2, I3);
			UML_SAR(block, I0, I0, 16);
			UML_SAR(block, I1, I1, 16);
			UML_MULSLW(block, I0, I0, I1);
			UML_ADD(block, I0, I0, I2);
			UML_ADD(block, mem(&m_core->global_regs[15]), mem(&m_core->global_regs[15]), I0);
			break;

		// signed half-word multiply/add, double word product sum
		case EHMACD:
			osd_printf_error("Unimplemented extended opcode, EHMACD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;

		// half-word complex multiply
		case EHCMULD:
			osd_printf_error("Unimplemented extended opcode, EHCMULD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;

		// half-word complex multiply/add
		case EHCMACD:
			osd_printf_error("Unimplemented extended opcode, EHCMACD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;

		// half-word (complex) add/subtract
		// Ls is not used and should denote the same register as Ld
		case EHCSUMD:
			osd_printf_error("Unimplemented extended opcode, EHCSUMD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;

		// half-word (complex) add/subtract with fixed point adjustment
		// Ls is not used and should denote the same register as Ld
		case EHCFFTD:
			osd_printf_error("Unimplemented extended opcode, EHCFFTD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;

		// half-word (complex) add/subtract with fixed point adjustment and shift
		// Ls is not used and should denote the same register as Ld
		case EHCFFTSD:
			osd_printf_error("Unimplemented extended opcode, EHCFFTSD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;

		default:
			osd_printf_error("Unknown extended opcode (%04x), PC = %08x\n", func, desc->pc);
			fatalerror(" ");
			break;
	}
}


void hyperstone_device::generate_reserved(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	osd_printf_error("Unimplemented: generate_reserved (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}

void hyperstone_device::generate_do(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	osd_printf_error("Unimplemented: generate_do (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}

#endif // MAME_CPU_E132XS_E132XSDRC_OPS_HXX
