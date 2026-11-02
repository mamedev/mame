// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    ARM7 dynamic recompiler UML generators

    Emulation by R. Belmont
*/

#include "emu.h"
#include "arm7.h"

#include "arm7core.h"
#include "arm7fe.h"

#include "cpu/drcumlsh.h"

#include <array>
#include <bit>
#include <cstddef>
#include <utility>


/***************************************************************************
    CONSTANTS
***************************************************************************/

namespace {

// ARM NZCV indexed by the UML flags (C, V, Z, S in bits 0-3); a subtraction's UML carry is ARM's C inverted
constexpr std::array<uint32_t, 16> make_nzcv_table(bool invert_c)
{
	std::array<uint32_t, 16> table{};
	for (uint32_t f = 0; f < 16; f++)
	{
		table[f] = ((f & uml::FLAG_S) ? N_MASK : 0) | ((f & uml::FLAG_Z) ? Z_MASK : 0) |
				((((f & uml::FLAG_C) != 0) != invert_c) ? C_MASK : 0) | ((f & uml::FLAG_V) ? V_MASK : 0);
	}
	return table;
}

constexpr std::array<uint32_t, 16> NZCV_ADD = make_nzcv_table(false);
constexpr std::array<uint32_t, 16> NZCV_SUB = make_nzcv_table(true);

// nonzero where a multi-flag condition (HI, LS, GE, LT, GT, LE) fails, indexed [cond][nzcv]
constexpr std::array<std::array<uint8_t, 16>, 16> make_cond_fail_table()
{
	std::array<std::array<uint8_t, 16>, 16> table{};
	for (uint32_t cond = 0; cond < 16; cond++)
	{
		for (uint32_t nzcv = 0; nzcv < 16; nzcv++)
		{
			const bool n = (nzcv & 8) != 0;
			const bool z = (nzcv & 4) != 0;
			const bool c = (nzcv & 2) != 0;
			const bool v = (nzcv & 1) != 0;
			bool pass = true;
			switch (cond)
			{
				case COND_HI: pass = c && !z; break;
				case COND_LS: pass = !c || z; break;
				case COND_GE: pass = n == v; break;
				case COND_LT: pass = n != v; break;
				case COND_GT: pass = !z && (n == v); break;
				case COND_LE: pass = z || (n != v); break;
			}
			table[cond][nzcv] = pass ? 0 : 1;
		}
	}
	return table;
}

constexpr std::array<std::array<uint8_t, 16>, 16> COND_FAIL = make_cond_fail_table();

} // anonymous namespace


bool arm7_cpu_device::alu_is_logical(uint32_t opcode)
{
	return (opcode == OPCODE_AND) || (opcode == OPCODE_EOR) || (opcode == OPCODE_TST) || (opcode == OPCODE_TEQ) ||
			(opcode == OPCODE_ORR) || (opcode == OPCODE_MOV) || (opcode == OPCODE_BIC) || (opcode == OPCODE_MVN);
}


bool arm7_cpu_device::drc_generates(const opcode_desc &desc) const
{
	const uint32_t op = desc.opptr;

	// volatile code (a word that keeps changing) runs through the interpreter
	if (m_drc_volatile.count(desc.pc))
	{
		return false;
	}

	// in the 26-bit modes R15 reads with the PSR in places; those rare forms stay with the interpreter
	const bool mode26 = (m_core->mode & 1) != 0;

	if (desc.length == 2)
	{
		return (m_drc_generate & DRC_GENERATE_THUMB) && thumb_generates(uint16_t(op));
	}

	// the cond = 1111 space is the interpreter's, except PLD (a no-op without a cache model)
	if ((op >> INSN_COND_SHIFT) == COND_NV)
	{
		return ((m_archRev >= 5) && ((op & 0x0d70f000) == 0x0550f000)) && (m_drc_generate & DRC_GENERATE_MEM);
	}

	switch ((op >> 24) & 0xf)
	{
		case 0x0: case 0x1: case 0x2: case 0x3:
			if (!(op & INSN_I) && ((op & 0x90) == 0x90))
			{
				// the multiply hole: halfword and signed transfers, swap, multiplies
				if (op & 0x60)
				{
					// LDRH, STRH, LDRSB, LDRSH, LDRD, STRD; the forms that write R15 stay with the interpreter
					if (m_archRev < 3)
					{
						return false;   // the trap
					}
					const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
					const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
					if (!(op & INSN_SDT_L) && (op & 0x60) != 0x20)
					{
						// LDRD, STRD: an even Rd below R14 on a core with the DSP extensions, anything else is a trap
						if (!(m_archFlags & ARCHFLAG_E) || (rd & 1) || rd == eR14)
						{
							return false;
						}
					}
					else if ((op & INSN_SDT_L) && rd == eR15)
					{
						return false;
					}
					if (rn == eR15 && (!(op & INSN_SDT_P) || (op & INSN_SDT_W)))
					{
						return false;
					}
					if (mode26 && (rn == eR15 || rd == eR15 || (!(op & 0x00400000) && (op & 0xf) == eR15)))
					{
						return false;
					}
					return (m_drc_generate & DRC_GENERATE_MEMHALF) != 0;
				}
				if (op & 0x01000000)
				{
					return false;   // SWP, SWPB, the exclusive transfers
				}
				if ((op & 0x0ff000f0) == 0x00400090)
				{
					return false;   // UMAAL, or the trap
				}
				// the multiplies: R15 as a destination stays with the interpreter
				if (mode26 && ((op & 0xf) == eR15 || ((op >> 8) & 0xf) == eR15 || ((op >> 12) & 0xf) == eR15 || ((op >> 16) & 0xf) == eR15))
				{
					return false;
				}
				if (op & 0x00800000)
				{
					if (m_archRev < 3 || ((op >> 16) & 0xf) == eR15 || ((op >> 12) & 0xf) == eR15)
					{
						return false;
					}
				}
				else if (m_archRev < 2 || ((op & INSN_MUL_RD) >> INSN_MUL_RD_SHIFT) == eR15)
				{
					return false;
				}
				return (m_drc_generate & DRC_GENERATE_MUL) != 0;
			}
			if ((op & 0x01900000) == 0x01000000)
			{
				// the miscellaneous space: only BX, BLX, CLZ, MRS and the flags-only MSR are generated
				if (m_archRev >= 3 && (op & 0x0ffffff0) == 0x012fff10)
				{
					return !mode26 && (m_drc_generate & DRC_GENERATE_BRANCH) != 0;    // BX Rm
				}
				if (m_archRev >= 5 && (op & 0x0ff000f0) == 0x01200030)
				{
					return !mode26 && (m_drc_generate & DRC_GENERATE_BRANCH) != 0;    // BLX Rm
				}
				if (m_archRev >= 5 && (op & 0x0ff000f0) == 0x01600010)
				{
					return (m_drc_generate & DRC_GENERATE_ALU) && ((op >> 12) & 0xf) != eR15 && (op & 0xf) != eR15; // CLZ
				}
				if (!(m_drc_generate & DRC_GENERATE_PSR) || m_archRev < 3)
				{
					return false;
				}
				if (mode26 && (op & 0x00600000) == 0x00600000)
				{
					return false;   // MSR SPSR: the interpreter leaves the SPSR alone in the 26-bit modes
				}
				if (!(op & INSN_I) && (op & 0x0fbf0ff0) == 0x010f0000)
				{
					return ((op >> 12) & 0xf) != eR15;                              // MRS Rd, CPSR/SPSR
				}
				if ((op & 0x0fb0f000) == 0x0320f000 && (op & 0x000f0000) == 0x00080000)
				{
					return true;                                                    // MSR CPSR_f/SPSR_f, #imm
				}
				if (!(op & INSN_I) && (op & 0x0fb0fff0) == 0x0120f000 && (op & 0x000f0000) == 0x00080000)
				{
					return (op & 0xf) != eR15;                                      // MSR CPSR_f/SPSR_f, Rm
				}
				return false;
			}
			// data processing: a write to R15 with S (the SPSR goes to the CPSR) stays with the interpreter
			if (!(m_drc_generate & DRC_GENERATE_ALU) || (((op & INSN_RD) >> INSN_RD_SHIFT) == eR15 && (op & INSN_S)))
			{
				return false;
			}
			if ((op & INSN_S) && !(m_drc_generate & DRC_GENERATE_ALU_S))
			{
				return false;
			}
			if ((op >> INSN_COND_SHIFT) != COND_AL && !(m_drc_generate & DRC_GENERATE_ALU_COND))
			{
				return false;
			}
			if (!(op & INSN_I) && (op & 0x10) && !(m_drc_generate & DRC_GENERATE_ALU_REGSHIFT))
			{
				return false;
			}
			if (!(op & INSN_I) && !(op & 0x10) && (op & 0xff0) && !(m_drc_generate & DRC_GENERATE_ALU_IMMSHIFT))
			{
				return false;
			}
			if ((((op & INSN_RN) >> INSN_RN_SHIFT) == eR15 || (!(op & INSN_I) && ((op & 0xf) == eR15 || ((op & 0x10) && ((op >> 8) & 0xf) == eR15)))) && !(m_drc_generate & DRC_GENERATE_ALU_R15))
			{
				return false;
			}
			return true;

		case 0x4: case 0x5: case 0x6: case 0x7:
			{
				// LDR, STR, LDRB, STRB; a write to R15 through the base, and LDRB R15, stay with the interpreter
				if ((op & INSN_I) && (op & 0x10))
				{
					return false;   // the ARMv6 media instructions, or the trap
				}
				const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
				const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
				if ((op & INSN_SDT_L) && (op & INSN_SDT_B) && rd == eR15)
				{
					return false;
				}
				if (rn == eR15 && (!(op & INSN_SDT_P) || (op & INSN_SDT_W)))
				{
					return false;
				}
				if (mode26 && ((!(op & INSN_SDT_L) && rd == eR15) || ((op & INSN_I) && (op & 0xf) == eR15)))
				{
					return false;
				}
				return (m_drc_generate & DRC_GENERATE_MEM) != 0;
			}

		case 0x8: case 0x9:
			{
				// LDM, STM: an empty list, R15 as base and LDM of R15 with S (exception return) are interpreted
				if ((op & 0xffff) == 0 || ((op & INSN_RN) >> INSN_RN_SHIFT) == eR15)
				{
					return false;
				}
				if ((op & INSN_BDT_L) && (op & 0x8000) && (op & INSN_BDT_S))
				{
					return false;
				}
				if (mode26 && !(op & INSN_BDT_L) && (op & 0x8000))
				{
					return false;
				}
				return (m_drc_generate & DRC_GENERATE_MEMBLOCK) != 0;
			}

		case 0xa: case 0xb:
			return (m_drc_generate & DRC_GENERATE_BRANCH) != 0;                       // B, BL

		default:
			return false;
	}
}


// if this returns false, the instruction will be handled by the interpreter
bool arm7_cpu_device::generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	if (!drc_generates(*desc))
	{
		return false;
	}
	if (desc->length == 2)
	{
		return generate_thumb(block, compiler, desc);
	}

	const uint32_t op = desc->opptr;
	switch ((op >> 24) & 0xf)
	{
		case 0x0: case 0x1: case 0x2: case 0x3:
			if (!(op & INSN_I) && (op & 0x90) == 0x90)
			{
				if (op & 0x60)
				{
					return generate_mem_half(block, compiler, desc);
				}
				return (op & 0x00800000) ? generate_mul_long(block, compiler, desc) : generate_mul(block, compiler, desc);
			}
			if ((op & 0x01900000) == 0x01000000)
			{
				if ((op & 0x0ffffff0) == 0x012fff10 || (op & 0x0ff000f0) == 0x01200030)
				{
					return generate_bx(block, compiler, desc);
				}
				if ((op & 0x0ff000f0) == 0x01600010)
				{
					return generate_clz(block, compiler, desc);
				}
				return generate_psr_transfer(block, compiler, desc);
			}
			return generate_alu(block, compiler, desc);

		case 0x4: case 0x5: case 0x6: case 0x7:
			return generate_mem_single(block, compiler, desc);

		case 0x8: case 0x9:
			return generate_mem_block(block, compiler, desc);

		case 0xa: case 0xb:
			return generate_branch(block, compiler, desc);

		default:
			return false;
	}
}



/***************************************************************************
    SHARED HELPERS
***************************************************************************/

void arm7_cpu_device::func_privilege_hook()
{
	debugger_privilege_hook();
}

static void cfunc_privilege_hook(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_privilege_hook();
}


// a register in the bank the block is compiled for
uml::parameter arm7_cpu_device::drc_reg(uint32_t regnum) const
{
	return uml::mem(&m_r[m_reg_group[regnum]]);
}


// returns the label a failed condition skips to, or 0 when there is nothing to test
uml::code_label arm7_cpu_device::generate_condition(drcuml_block &block, compiler_state &compiler, uint32_t cond)
{
	// always, or tested once for the run this instruction is in
	if (cond == COND_AL || (compiler.run_end != nullptr && cond == compiler.run_cond))
	{
		return 0;
	}

	const uml::code_label skip = compiler.labelnum++;
	const uml::parameter cpsr = uml::mem(&m_r[eCPSR]);
	switch (cond)
	{
		case COND_EQ: UML_TEST(block, cpsr, Z_MASK); UML_JMPc(block, uml::COND_Z, skip); break; // test    cpsr,Z ; jmp skip,z
		case COND_NE: UML_TEST(block, cpsr, Z_MASK); UML_JMPc(block, uml::COND_NZ, skip); break;
		case COND_CS: UML_TEST(block, cpsr, C_MASK); UML_JMPc(block, uml::COND_Z, skip); break;
		case COND_CC: UML_TEST(block, cpsr, C_MASK); UML_JMPc(block, uml::COND_NZ, skip); break;
		case COND_MI: UML_TEST(block, cpsr, N_MASK); UML_JMPc(block, uml::COND_Z, skip); break;
		case COND_PL: UML_TEST(block, cpsr, N_MASK); UML_JMPc(block, uml::COND_NZ, skip); break;
		case COND_VS: UML_TEST(block, cpsr, V_MASK); UML_JMPc(block, uml::COND_Z, skip); break;
		case COND_VC: UML_TEST(block, cpsr, V_MASK); UML_JMPc(block, uml::COND_NZ, skip); break;

		default:
			UML_BFXU(block, uml::I2, cpsr, 28, 4);                                  // bfxu    i2,cpsr,28,4
			UML_LOAD(block, uml::I2, COND_FAIL[cond].data(), uml::I2, SIZE_BYTE, SCALE_x1); // load    i2,cond_fail,i2,byte
			UML_TEST(block, uml::I2, uml::I2);                                      // test    i2,i2
			UML_JMPc(block, uml::COND_NZ, skip);                                    // jmp     skip,nz
			break;
	}
	return skip;
}


// a skipped instruction costs 1 cycle instead of its own
void arm7_cpu_device::generate_condition_end(drcuml_block &block, compiler_state &compiler, uml::code_label skip, uint32_t cycles)
{
	if (skip.label() == 0)
	{
		return;
	}
	if (cycles > 1)
	{
		const uml::code_label done = compiler.labelnum++;
		UML_JMP(block, done);                                                       // jmp     done
		UML_LABEL(block, skip);                                                     // skip:
		UML_ADD(block, uml::mem(&m_core->icount), uml::mem(&m_core->icount), cycles - 1);       // add     icount,icount,cycles-1
		UML_LABEL(block, done);                                                     // done:
	}
	else
	{
		UML_LABEL(block, skip);                                                     // skip:
	}
}


// tests the condition once for a run of instructions sharing it; a skipped run costs a cycle each
void arm7_cpu_device::generate_condition_run(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, const opcode_desc *seqlast)
{
	if (!(m_drc_opt & DRC_OPT_RUNS) || compiler.exact || compiler.run_end != nullptr || debugger_enabled())
	{
		return;
	}

	const auto member = [this] (const opcode_desc *d, uint32_t cond, bool first)
	{
		return d->length == 4 && (d->opptr >> INSN_COND_SHIFT) == cond && !d->is_branch() && !d->regout.test(opcode_desc::REG_FLAG_NZCV)
				&& !d->compiler_page_fault() && !d->virtual_noop() && drc_generates(*d) && (first || (!d->validate_tlb() && !d->is_branch_target()));
	};

	const uint32_t cond = desc->opptr >> INSN_COND_SHIFT;
	if (desc->length != 4 || cond == COND_AL || cond == COND_NV || !member(desc, cond, true))
	{
		return;
	}
	// a store ends a run: the words after it are verified again outside it
	const opcode_desc *last = desc;
	uint32_t back = instruction_cycles(desc) - 1;
	while (last != seqlast && !last->writes_memory() && last->next() != nullptr && member(last->next(), cond, false))
	{
		last = last->next();
		back += instruction_cycles(last) - 1;
	}
	if (last == desc)
	{
		return;
	}

	// flush the owed cycles before the test, as a skipped run jumps over any flush inside it
	generate_flush_cycles(block, compiler);
	compiler.run_skip = generate_condition(block, compiler, cond);
	compiler.run_cond = cond;
	compiler.run_end = last;
	compiler.run_back = back;
	compiler.run_flushed = 0;
}


void arm7_cpu_device::generate_condition_run_end(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	if (compiler.run_end != desc)
	{
		return;
	}
	compiler.run_end = nullptr;

	// a skipped run gives back all but a cycle per instruction, less what the run flushed on the way
	const int32_t back = int32_t(compiler.run_back) - int32_t(compiler.run_flushed);
	const uml::code_label done = compiler.labelnum++;
	UML_JMP(block, done);                                                           // jmp     done
	UML_LABEL(block, compiler.run_skip);                                            // skip:
	if (back != 0)
	{
		UML_ADD(block, uml::mem(&m_core->icount), uml::mem(&m_core->icount), uint32_t(back));   // add     icount,icount,back
	}
	UML_LABEL(block, done);                                                         // done:
}


// whether a PC-relative load folds: verified memory on the instruction's own page; the word comes back unrotated
bool arm7_cpu_device::drc_literal(const opcode_desc &desc, offs_t address, const void *&host, uint32_t &word)
{
	if (!(m_drc_opt & DRC_OPT_LITERAL) || debugger_enabled() || m_drc_nofold.count(desc.pc))
	{
		return false;
	}
	offs_t physical = address & ~uint32_t(3);
	if (m_control & COPRO_CTRL_MMU_EN)
	{
		if ((address ^ desc.pc) & ~uint32_t(0xfff))
		{
			return false;
		}
		physical = (desc.physpc & ~uint32_t(0xfff)) | (physical & 0xfff);
	}
	host = insn_fetch_ptr(physical);
	if (host == nullptr)
	{
		return false;
	}
	word = insn_fetch_word(physical);
	return true;
}


// a changed word recompiles the load unfolded, with none of this instruction's cycles owed
void arm7_cpu_device::generate_literal_check(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, const void *host, uint32_t word)
{
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles - instruction_cycles(desc));   // mapvar  CYCLES,<before this instruction>
	UML_LOAD(block, uml::I0, host, 0, SIZE_DWORD, SCALE_x4);                        // load    i0,[word],0,dword
	UML_CMP(block, uml::I0, word);                                                  // cmp     i0,word
	UML_EXHc(block, uml::COND_NE, *m_nocode_literal, desc->pc);                     // exh     nocode_literal,pc,ne
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                              // mapvar  CYCLES,compiler.cycles
}


// the not-taken path of a conditional branch costs skip_cost cycles and keeps the sequence's accounting
void arm7_cpu_device::generate_branch_end(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uml::code_label skip, uint32_t skip_cost, bool dynamic)
{
	compiler_state taken(compiler);
	if (!dynamic && desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, taken, desc->targetpc);
		if (desc->intrablock_branch() && !compiler.exact)
		{
			UML_JMP(block, desc->targetpc | 0x80000000);                            // jmp     target
		}
		else
		{
			UML_HASHJMP(block, drc_hash_key(m_core->mode, desc->targetpc), desc->targetpc, *m_nocode_miss);   // hashjmp <mode>,target,nocode_miss
		}
	}
	else
	{
		// the generator has written R15, and the mode when it changed it
		generate_update_cycles(block, taken, uml::mem(&m_r[eR15]));
		generate_hashjmp_dynamic(block);                                            // hashjmp <mode>,<r15>,nocode_miss
	}
	compiler.labelnum = taken.labelnum;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                              // mapvar  CYCLES,compiler.cycles

	if (skip.label() != 0)
	{
		UML_LABEL(block, skip);                                                     // skip:
		if (skip_cost < desc->cycles)
		{
			UML_ADD(block, uml::mem(&m_core->icount), uml::mem(&m_core->icount), desc->cycles - skip_cost);  // add     icount,icount,cycles-cost
		}
	}
}


// the loaded word in I0 becomes the PC (LDR PC, LDM with R15)
void arm7_cpu_device::generate_loaded_pc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uml::code_label skip, bool interwork, bool accessed)
{
	if (m_core->mode & 1)
	{
		// 26-bit modes: the address bits alone, the PSR stays; the DE156 resumes an unaligned LDR at the next word
		if (m_ldr_pc_round_up && desc->length == 4 && ((desc->opptr >> 26) & 3) == 1)
		{
			const uml::code_label aligned = compiler.labelnum++;
			UML_TEST(block, uml::I0, 3);                                                // test    i0,3
			UML_JMPc(block, uml::COND_Z, aligned);                                      // jmp     aligned,z
			UML_ADD(block, uml::I0, uml::I0, 4);                                        // add     i0,i0,4
			UML_LABEL(block, aligned);                                                  // aligned:
		}
		UML_AND(block, uml::I0, uml::I0, 0x03fffffc);                                   // and     i0,i0,pc_bits
	}
	else if (interwork)
	{
		const uml::code_label arm = compiler.labelnum++;
		const uml::code_label done = compiler.labelnum++;
		UML_TEST(block, uml::I0, 1);                                                    // test    i0,1
		UML_JMPc(block, uml::COND_Z, arm);                                              // jmp     arm,z
		UML_AND(block, uml::I0, uml::I0, ~uint32_t(1));                                 // and     i0,i0,~1
		UML_OR(block, uml::mem(&m_r[eCPSR]), uml::mem(&m_r[eCPSR]), T_MASK);            // or      cpsr,cpsr,T
		UML_OR(block, uml::mem(&m_core->mode), uml::mem(&m_core->mode), 2);             // or      mode,mode,2
		if (debugger_enabled())
		{
			UML_CALLC(block, cfunc_privilege_hook, this);                               // callc   privilege_hook
		}
		UML_JMP(block, done);                                                           // jmp     done
		UML_LABEL(block, arm);                                                          // arm:
		UML_AND(block, uml::I0, uml::I0, ~uint32_t(3));                                 // and     i0,i0,~3
		UML_LABEL(block, done);                                                         // done:
	}
	else
	{
		UML_AND(block, uml::I0, uml::I0, ~uint32_t(3));                                 // and     i0,i0,~3
	}
	UML_MOV(block, uml::mem(&m_r[eR15]), uml::I0);                                      // mov     r15,i0
	if (accessed)
	{
		generate_loaded_pc_end(block, compiler, desc, skip);
	}
	else
	{
		generate_branch_end(block, compiler, desc, skip, 1);
	}
}


// an address map change under the access is taken before the loaded target is looked up
void arm7_cpu_device::generate_loaded_pc_end(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uml::code_label skip)
{
	UML_TEST(block, uml::mem(&m_core->stop), STOP_REMAP);                           // test    stop,REMAP
	UML_EXHc(block, uml::COND_NZ, *m_resync, uml::mem(&m_r[eR15]));                 // exh     resync,<r15>,nz
	generate_branch_end(block, compiler, desc, skip, 1);
}


// a data abort ends the instruction; a map change waits until it completes (generate_memory_end())
void arm7_cpu_device::generate_abort_check(drcuml_block &block, const opcode_desc *desc)
{
	UML_TEST(block, uml::mem(&m_core->stop), STOP_ABORT);                           // test    stop,ABORT
	UML_EXHc(block, uml::COND_NZ, *m_resync, desc->pc + desc->length);              // exh     resync,nextpc,nz
}


// leaves once the instruction completes if an access ended the slice or changed the address map
void arm7_cpu_device::generate_memory_end(drcuml_block &block, const opcode_desc *desc)
{
	// the next two words as compiled are the interpreter's prefetch if the map changes (drc_prefetch_after_remap())
	uml::code_handle *resync = m_resync;
	const opcode_desc *const next = desc->next();
	if (desc->length == 4 && next && next->pc == desc->pc + 4 && next->length == 4)
	{
		// after an unconditional branch the second word is never run (and often not described)
		const opcode_desc *const second = next->next();
		if (second && second->pc == desc->pc + 8)
		{
			m_drc_prefetch[desc->pc + 4] = std::make_pair(next->opword, second->opword);
			resync = m_resync_next;
		}
		else if (next->is_unconditional_branch())
		{
			m_drc_prefetch[desc->pc + 4] = std::make_pair(next->opword, uint32_t(0));
			resync = m_resync_next;
		}
	}

	// icount:stop as one 64-bit word, minus 1, has a bit above 30 iff icount <= 0 or stop != 0 (icount > INT_MIN)
	static_assert(offsetof(core_state, stop) == offsetof(core_state, icount) + 4);
	if constexpr (std::endian::native == std::endian::little)
	{
		UML_DSUB(block, uml::I0, uml::mem(&m_core->icount), 1);                     // dsub    i0,icount:stop,1
		UML_DTEST(block, uml::I0, ~uint64_t(0x7fffffff));                           // dtest   i0,~0x7fffffff
		UML_EXHc(block, uml::COND_NZ, *resync, desc->pc + desc->length);            // exh     resync,nextpc,nz
	}
	else
	{
		UML_CMP(block, uml::mem(&m_core->icount), 0);                               // cmp     icount,0
		UML_EXHc(block, uml::COND_LE, *resync, desc->pc + desc->length);            // exh     resync,nextpc,le
		UML_TEST(block, uml::mem(&m_core->stop), ~uint32_t(0));                     // test    stop,~0
		UML_EXHc(block, uml::COND_NZ, *resync, desc->pc + desc->length);            // exh     resync,nextpc,nz
	}
}


// NZCV from the UML flags of the add or subtract just performed
void arm7_cpu_device::generate_flags_arith(drcuml_block &block, bool subtract)
{
	UML_GETFLGS(block, uml::I2, uml::FLAG_C | uml::FLAG_V | uml::FLAG_Z | uml::FLAG_S);  // getflgs i2,CVZS
	UML_LOAD(block, uml::I2, subtract ? NZCV_SUB.data() : NZCV_ADD.data(), uml::I2, SIZE_DWORD, SCALE_x4);  // load    i2,nzcv,i2,dword
	UML_ROLINS(block, uml::mem(&m_r[eCPSR]), uml::I2, 0, N_MASK | Z_MASK | C_MASK | V_MASK);  // rolins  cpsr,i2,0,NZCV
}


// NZ from the UML flags of the operation just performed, C from the shifter; V stays
void arm7_cpu_device::generate_flags_logical(drcuml_block &block, uml::parameter carry)
{
	UML_GETFLGS(block, uml::I2, uml::FLAG_Z | uml::FLAG_S);                         // getflgs i2,ZS
	UML_LOAD(block, uml::I2, NZCV_ADD.data(), uml::I2, SIZE_DWORD, SCALE_x4);       // load    i2,nzcv,i2,dword
	UML_OR(block, uml::I2, uml::I2, carry);                                         // or      i2,i2,carry
	UML_ROLINS(block, uml::mem(&m_r[eCPSR]), uml::I2, 0, N_MASK | Z_MASK | C_MASK); // rolins  cpsr,i2,0,NZC
}


// NZ from the UML flags of the operation just performed; C and V stay
void arm7_cpu_device::generate_flags_nz(drcuml_block &block)
{
	UML_GETFLGS(block, uml::I2, uml::FLAG_Z | uml::FLAG_S);                         // getflgs i2,ZS
	UML_LOAD(block, uml::I2, NZCV_ADD.data(), uml::I2, SIZE_DWORD, SCALE_x4);       // load    i2,nzcv,i2,dword
	UML_ROLINS(block, uml::mem(&m_r[eCPSR]), uml::I2, 0, N_MASK | Z_MASK);          // rolins  cpsr,i2,0,NZ
}


// the shifter operand (decodeShift()); the carry out as C_MASK or 0 in carry when wanted
uml::parameter arm7_cpu_device::generate_operand2(drcuml_block &block, const opcode_desc *desc, uint32_t op, bool want_carry, uml::parameter &carry)
{
	const uml::parameter cpsr = uml::mem(&m_r[eCPSR]);

	if (op & INSN_I)
	{
		// a rotated immediate; a rotation of zero leaves the carry alone
		const uint32_t rotate = ((op & INSN_OP2_ROTATE) >> INSN_OP2_ROTATE_SHIFT) << 1;
		const uint32_t value = rotate ? std::rotr<uint32_t>(op & INSN_OP2_IMM, rotate) : (op & INSN_OP2);
		if (want_carry)
		{
			if (rotate)
			{
				carry = (value & SIGN_BIT) ? C_MASK : 0;
			}
			else
			{
				UML_AND(block, uml::I3, cpsr, C_MASK);                              // and     i3,cpsr,C
				carry = uml::I3;
			}
		}
		return value;
	}

	const uint32_t rm = op & INSN_OP2_RM;
	const uint32_t type = (op & INSN_OP2_SHIFT_TYPE) >> INSN_OP2_SHIFT_TYPE_SHIFT;

	if (op & 0x10)
	{
		// a shift by the bottom byte of a register: R15 reads 12 bytes ahead as Rm, 8 as Rs
		const uint32_t rs = (op >> 8) & 0xf;
		const uml::parameter rmval = (rm == eR15) ? uml::parameter(desc->pc + 12) : drc_reg(rm);
		const uml::parameter rsval = (rs == eR15) ? uml::parameter(desc->pc + 8) : drc_reg(rs);
		if ((rm == eR15) && (m_core->mode & 1))
		{
			generate_r15_psr26(block, uml::I1, desc->pc + 12, uml::I0);             // i1 = psr + pc+12 (decodeShift())
		}
		else
		{
			UML_MOV(block, uml::I1, rmval);                                         // mov     i1,rm
		}
		UML_AND(block, uml::I2, rsval, 0xff);                                       // and     i2,rs,0xff
		UML_CALLH(block, *m_shift_reg[type >> 1]);                                  // callh   shift
		if (want_carry)
		{
			carry = uml::I3;
		}
		return uml::I1;
	}

	return generate_shift_imm(block, desc, op, want_carry, carry);
}


// a register shifted by an immediate, as generate_operand2(); also the single transfers' offset
uml::parameter arm7_cpu_device::generate_shift_imm(drcuml_block &block, const opcode_desc *desc, uint32_t op, bool want_carry, uml::parameter &carry)
{
	const uml::parameter cpsr = uml::mem(&m_r[eCPSR]);
	const uint32_t rm = op & INSN_OP2_RM;
	const uint32_t type = (op & INSN_OP2_SHIFT_TYPE) >> INSN_OP2_SHIFT_TYPE_SHIFT;
	const uint32_t amount = (op & INSN_OP2_SHIFT) >> INSN_OP2_SHIFT_SHIFT;
	uml::parameter rmval = (rm == eR15) ? uml::parameter(desc->pc + 8) : drc_reg(rm);
	if ((rm == eR15) && (m_core->mode & 1))
	{
		// the 26-bit modes: R15 as Rm reads with the PSR (decodeShift())
		generate_r15_psr26(block, uml::I1, desc->pc + 8, uml::I0);                  // i1 = psr + pc+8
		rmval = uml::I1;
	}
	const auto carry_from_bit = [&block, &carry, rmval, want_carry] (uint32_t bit)
	{
		// bit n of Rm as C_MASK: rotate it up to bit 29
		if (want_carry)
		{
			UML_ROLAND(block, uml::I3, rmval, (29 + 32 - bit) & 31, C_MASK);        // roland  i3,rm,29-bit,C
			carry = uml::I3;
		}
	};

	switch (type >> 1)
	{
		case 0: // LSL
			if (amount == 0)
			{
				if (want_carry)
				{
					UML_AND(block, uml::I3, cpsr, C_MASK);                          // and     i3,cpsr,C
					carry = uml::I3;
				}
				return rmval;
			}
			carry_from_bit(32 - amount);
			UML_SHL(block, uml::I1, rmval, amount);                                 // shl     i1,rm,amount
			return uml::I1;

		case 1: // LSR; #0 is #32
			if (amount == 0)
			{
				carry_from_bit(31);
				return 0;
			}
			carry_from_bit(amount - 1);
			UML_SHR(block, uml::I1, rmval, amount);                                 // shr     i1,rm,amount
			return uml::I1;

		case 2: // ASR; #0 is #32
			if (amount == 0)
			{
				carry_from_bit(31);
				UML_SAR(block, uml::I1, rmval, 31);                                 // sar     i1,rm,31
				return uml::I1;
			}
			carry_from_bit(amount - 1);
			UML_SAR(block, uml::I1, rmval, amount);                                 // sar     i1,rm,amount
			return uml::I1;

		case 3: // ROR; #0 is RRX
		default:
			if (amount == 0)
			{
				carry_from_bit(0);
				UML_CARRY(block, cpsr, C_BIT);                                      // carry   cpsr,C
				UML_RORC(block, uml::I1, rmval, 1);                                 // rorc    i1,rm,1
				return uml::I1;
			}
			carry_from_bit(amount - 1);
			UML_ROR(block, uml::I1, rmval, amount);                                 // ror     i1,rm,amount
			return uml::I1;
	}
}


/***************************************************************************
    ARM Data Processing instructions
***************************************************************************/

bool arm7_cpu_device::generate_alu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const uint32_t opcode = (op & INSN_OPCODE) >> INSN_OPCODE_SHIFT;
	const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
	const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
	const bool sets_flags = (op & INSN_S) != 0;
	const bool logical = alu_is_logical(opcode);
	const bool compare = (opcode & 0xc) == 0x8;
	const uml::parameter cpsr = uml::mem(&m_r[eCPSR]);

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);

	// the operands: R15 as Rn reads 12 bytes ahead under a register-specified shift, 8 otherwise
	uml::parameter carry = 0;
	// the flags, only when something reads them before they are next written
	const bool want_flags = sets_flags && desc->flags_live != 0;
	const uml::parameter op2 = generate_operand2(block, desc, op, want_flags && logical, carry);
	uml::parameter rnval = 0;
	if ((opcode & 0xd) != 0xd)
	{
		rnval = (rn == eR15) ? uml::parameter(desc->pc + ((!(op & INSN_I) && (op & 0x10)) ? 12 : 8)) : drc_reg(rn);
	}

	switch (opcode)
	{
		case OPCODE_AND:
		case OPCODE_TST:
			UML_AND(block, uml::I0, rnval, op2);                                    // and     i0,rn,op2
			break;

		case OPCODE_EOR:
		case OPCODE_TEQ:
			UML_XOR(block, uml::I0, rnval, op2);                                    // xor     i0,rn,op2
			break;

		case OPCODE_SUB:
		case OPCODE_CMP:
			UML_SUB(block, uml::I0, rnval, op2);                                    // sub     i0,rn,op2
			break;

		case OPCODE_RSB:
			UML_SUB(block, uml::I0, op2, rnval);                                    // sub     i0,op2,rn
			break;

		case OPCODE_ADD:
		case OPCODE_CMN:
			UML_ADD(block, uml::I0, rnval, op2);                                    // add     i0,rn,op2
			break;

		case OPCODE_ADC:
			UML_CARRY(block, cpsr, C_BIT);                                          // carry   cpsr,C
			UML_ADDC(block, uml::I0, rnval, op2);                                   // addc    i0,rn,op2
			break;

		case OPCODE_SBC:
			// Rn - Op2 - NOT C is Rn + NOT Op2 + C, with the carry and overflow of that addition
			if (op2.is_immediate())
			{
				UML_CARRY(block, cpsr, C_BIT);                                      // carry   cpsr,C
				UML_ADDC(block, uml::I0, rnval, ~uint32_t(op2.immediate()));        // addc    i0,rn,~op2
			}
			else
			{
				UML_XOR(block, uml::I1, op2, ~uint32_t(0));                         // xor     i1,op2,~0
				UML_CARRY(block, cpsr, C_BIT);                                      // carry   cpsr,C
				UML_ADDC(block, uml::I0, rnval, uml::I1);                           // addc    i0,rn,i1
			}
			break;

		case OPCODE_RSC:
			if (rnval.is_immediate())
			{
				UML_CARRY(block, cpsr, C_BIT);                                      // carry   cpsr,C
				UML_ADDC(block, uml::I0, op2, ~uint32_t(rnval.immediate()));        // addc    i0,op2,~rn
			}
			else
			{
				UML_XOR(block, uml::I2, rnval, ~uint32_t(0));                       // xor     i2,rn,~0
				UML_CARRY(block, cpsr, C_BIT);                                      // carry   cpsr,C
				UML_ADDC(block, uml::I0, op2, uml::I2);                             // addc    i0,op2,i2
			}
			break;

		case OPCODE_ORR:
			UML_OR(block, uml::I0, rnval, op2);                                     // or      i0,rn,op2
			break;

		case OPCODE_MOV:
			UML_MOV(block, uml::I0, op2);                                           // mov     i0,op2
			if (want_flags)
			{
				UML_TEST(block, uml::I0, uml::I0);                                  // test    i0,i0
			}
			break;

		case OPCODE_BIC:
			if (op2.is_immediate())
			{
				UML_AND(block, uml::I0, rnval, ~uint32_t(op2.immediate()));         // and     i0,rn,~op2
			}
			else
			{
				UML_XOR(block, uml::I1, op2, ~uint32_t(0));                         // xor     i1,op2,~0
				UML_AND(block, uml::I0, rnval, uml::I1);                            // and     i0,rn,i1
			}
			break;

		case OPCODE_MVN:
		default:
			if (op2.is_immediate())
			{
				UML_MOV(block, uml::I0, ~uint32_t(op2.immediate()));                // mov     i0,~op2
				if (want_flags)
				{
					UML_TEST(block, uml::I0, uml::I0);                              // test    i0,i0
				}
			}
			else
			{
				UML_XOR(block, uml::I0, op2, ~uint32_t(0));                         // xor     i0,op2,~0
			}
			break;
	}

	// the UML flags of the operation are read before anything else changes them
	if (want_flags)
	{
		if (logical)
		{
			generate_flags_logical(block, carry);
		}
		else
		{
			generate_flags_arith(block, (opcode == OPCODE_SUB) || (opcode == OPCODE_CMP) || (opcode == OPCODE_RSB));
		}
	}

	if (!compare)
	{
		if (rd == eR15)
		{
			// PC writes without S ignore bits 1:0 (no ALU interworking before ARMv7); the 26-bit modes keep the PSR
			UML_AND(block, uml::I0, uml::I0, (m_core->mode & 1) ? 0x03fffffc : ~uint32_t(3));  // and     i0,i0,pc_bits
			UML_MOV(block, uml::mem(&m_r[eR15]), uml::I0);                          // mov     r15,i0
			generate_branch_end(block, compiler, desc, skip, 1);
			return true;
		}
		UML_MOV(block, drc_reg(rd), uml::I0);                                       // mov     rd,i0
	}

	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}


bool arm7_cpu_device::generate_clz(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);
	UML_LZCNT(block, uml::I0, drc_reg(op & 0xf));                                       // lzcnt   i0,rm
	UML_MOV(block, drc_reg((op >> 12) & 0xf), uml::I0);                                 // mov     rd,i0
	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}


// MRS, and MSR of the flags field only
bool arm7_cpu_device::generate_psr_transfer(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const bool spsr = (op & 0x00400000) != 0;
	const uml::parameter psr = spsr ? drc_reg(SPSR) : uml::mem(&m_r[eCPSR]);

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);

	if (op & 0x00200000)
	{
		// MSR: N, Z, C, V and Q; there is no SPSR in User or System mode
		if (!spsr || (GET_MODE != eARM7_MODE_USER && GET_MODE != eARM7_MODE_SYS))
		{
			uml::parameter value = 0;
			if (op & INSN_I)
			{
				const uint32_t rotate = ((op & INSN_OP2_ROTATE) >> INSN_OP2_ROTATE_SHIFT) << 1;
				value = rotate ? std::rotr<uint32_t>(op & INSN_OP2_IMM, rotate) : (op & INSN_OP2_IMM);
			}
			else
			{
				value = drc_reg(op & 0xf);
			}
			if (value.is_immediate())
			{
				UML_ROLINS(block, psr, value.immediate() & 0xf8000000, 0, 0xff000000);   // rolins  psr,value,0,0xff000000
			}
			else
			{
				UML_AND(block, uml::I0, value, 0xf8000000);                         // and     i0,value,NZCVQ
				UML_ROLINS(block, psr, uml::I0, 0, 0xff000000);                     // rolins  psr,i0,0,0xff000000
			}
		}
	}
	else
	{
		// MRS
		UML_MOV(block, drc_reg((op >> 12) & 0xf), psr);                             // mov     rd,psr
	}

	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}



/***************************************************************************
    ARM Branch instructions
***************************************************************************/

bool arm7_cpu_device::generate_branch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);
	if ((op & INSN_BL) && (m_core->mode & 1))
	{
		generate_r15_psr26(block, drc_reg(eR14), desc->pc + 4, uml::I0);             // r14 = psr + pc+4
	}
	else if (op & INSN_BL)
	{
		UML_MOV(block, drc_reg(eR14), desc->pc + 4);                                  // mov     r14,pc+4
	}
	generate_branch_end(block, compiler, desc, skip, 1);
	return true;
}


bool arm7_cpu_device::generate_bx(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const uint32_t rm = op & 0xf;
	const bool link = (op & 0x0ff000f0) == 0x01200030;

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);
	const uml::code_label arm = compiler.labelnum++;

	// the target is read before LR is written (BLX LR is legal); BX PC reads PC+8, BLX PC the bare PC, as the interpreter does
	UML_MOV(block, uml::I0, (rm == eR15) ? uml::parameter(link ? desc->pc : desc->pc + 8) : drc_reg(rm));   // mov     i0,rm
	if (link)
	{
		UML_MOV(block, drc_reg(eR14), desc->pc + 4);                                // mov     r14,pc+4
	}
	UML_TEST(block, uml::I0, 1);                                                    // test    i0,1
	UML_JMPc(block, uml::COND_Z, arm);                                              // jmp     arm,z
	UML_AND(block, uml::I0, uml::I0, ~uint32_t(1));                                 // and     i0,i0,~1
	UML_OR(block, uml::mem(&m_r[eCPSR]), uml::mem(&m_r[eCPSR]), T_MASK);            // or      cpsr,cpsr,T
	UML_OR(block, uml::mem(&m_core->mode), uml::mem(&m_core->mode), 2);             // or      mode,mode,2
	if (debugger_enabled())
	{
		UML_CALLC(block, cfunc_privilege_hook, this);                               // callc   privilege_hook
	}
	UML_LABEL(block, arm);                                                          // arm:
	UML_MOV(block, uml::mem(&m_r[eR15]), uml::I0);                                  // mov     r15,i0
	generate_branch_end(block, compiler, desc, skip, 1);
	return true;
}



/***************************************************************************
    ARM Load and Store instructions
***************************************************************************/

// address in I0, the written-back base in I4 (even when a post-indexed load replaces it); returns whether it is written
bool arm7_cpu_device::generate_mem_address(drcuml_block &block, const opcode_desc *desc, uint32_t op, uml::parameter offset, bool load)
{
	const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
	const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
	const bool up = (op & INSN_SDT_U) != 0;

	// R15 as the base reads 8 bytes ahead (the write-back forms are the interpreter's)
	const uml::parameter base = (rn == eR15) ? uml::parameter(desc->pc + 8) : drc_reg(rn);

	if (op & INSN_SDT_P)
	{
		if (up)
		{
			UML_ADD(block, uml::I0, base, offset);                                  // add     i0,base,offset
		}
		else
		{
			UML_SUB(block, uml::I0, base, offset);                                  // sub     i0,base,offset
		}
		if (!(op & INSN_SDT_W))
		{
			return false;
		}
		UML_MOV(block, uml::I4, uml::I0);                                           // mov     i4,i0
		return true;
	}

	UML_MOV(block, uml::I0, base);                                                  // mov     i0,base
	if (up)
	{
		UML_ADD(block, uml::I4, base, offset);                                      // add     i4,base,offset
	}
	else
	{
		UML_SUB(block, uml::I4, base, offset);                                      // sub     i4,base,offset
	}
	return !(load && rd == rn);
}


// address in I0, data in I1, result in I0, I1-I3 trashed; place the returned label with generate_access_end()
uml::code_label arm7_cpu_device::generate_access(drcuml_block &block, compiler_state &compiler, int size, bool iswrite, bool user)
{
	if (!drc_tlb_inline())
	{
		uml::code_handle *const *const handles = iswrite ? ((size == 1) ? m_write8 : (size == 2) ? m_write16 : m_write32) : ((size == 1) ? m_read8 : (size == 2) ? m_read16 : m_read32);
		UML_CALLH(block, *handles[user ? 1 : 0]);                                   // callh   access
		return uml::code_label(0);
	}

	const uml::code_label miss = compiler.labelnum++;
	const uml::code_label done = compiler.labelnum++;
	generate_tlb_probe(block, size, iswrite, user, miss);
	UML_JMP(block, done);                                                           // jmp     done
	UML_LABEL(block, miss);                                                         // miss:
	UML_CALLH(block, *m_access_miss[iswrite ? 1 : 0][(size == 1) ? 0 : (size == 2) ? 1 : 2][user ? 1 : 0]);   // callh   access_miss
	return done;
}

void arm7_cpu_device::generate_access_end(drcuml_block &block, uml::code_label done)
{
	if (done.label() != 0)
	{
		UML_LABEL(block, done);                                                     // done:
	}
}


bool arm7_cpu_device::generate_mem_single(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	if ((op >> INSN_COND_SHIFT) == COND_NV)
	{
		return true;        // PLD: nothing without a cache model
	}

	const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
	const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
	const bool load = (op & INSN_SDT_L) != 0;
	const bool byte = (op & INSN_SDT_B) != 0;
	const bool user = GET_MODE == eARM7_MODE_USER;

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);

	// a PC-relative word load from verified memory is folded to the word
	if (load && !byte && rn == eR15 && !(op & INSN_I) && (op & INSN_SDT_P) && !(op & INSN_SDT_W))
	{
		const void *host = nullptr;
		uint32_t word = 0;
		const offs_t address = desc->pc + 8 + ((op & INSN_SDT_U) ? (op & INSN_SDT_IMM) : -(op & INSN_SDT_IMM));
		if (drc_literal(*desc, address, host, word))
		{
			const uint32_t value = std::rotr(word, 8 * (address & 3));
			generate_literal_check(block, compiler, desc, host, word);
			if (rd == eR15)
			{
				// the frontend saw the word too: an aligned ARM target is a static branch
				if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
				{
					generate_branch_end(block, compiler, desc, skip, 1);
					return true;
				}
				UML_MOV(block, uml::I0, value);                                     // mov     i0,value
				generate_loaded_pc(block, compiler, desc, skip, m_archRev >= 5, false);
				return true;
			}
			UML_MOV(block, drc_reg(rd), value);                                     // mov     rd,value
			generate_condition_end(block, compiler, skip, desc->cycles);
			return true;
		}
		else if (rd == eR15 && desc->targetpc != BRANCH_TARGET_DYNAMIC)
		{
			fatalerror("%s: LDR PC at %08X was described as static but its word cannot be folded\n", tag(), desc->pc);
		}
	}

	// the offset: an immediate, or a register shifted by an immediate
	uml::parameter carry = 0;
	const uml::parameter offset = (op & INSN_I) ? generate_shift_imm(block, desc, op, false, carry) : uml::parameter(op & INSN_SDT_IMM);
	const bool writeback = generate_mem_address(block, desc, op, offset, load);

	// a store's data is read before anything is written back; R15 stores 12 bytes ahead
	if (!load)
	{
		UML_MOV(block, uml::I1, (rd == eR15) ? uml::parameter(desc->pc + 12) : drc_reg(rd));  // mov     i1,rd
	}
	const uml::code_label done = generate_access(block, compiler, byte ? 1 : 4, !load, user);

	// an aborted LDR PC costs the interpreter's 3 cycles, without the 2 a load of PC adds
	if (load && rd == eR15)
	{
		UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles - 2);                      // mapvar  CYCLES,<aborted>
	}

	// an abort: the base keeps its value, except that the ARMv3 late abort leaves it written back
	if (m_archRev < 4 && ((op & INSN_SDT_W) || !(op & INSN_SDT_P)))
	{
		const uml::code_label ok = compiler.labelnum++;
		const uml::code_label abort = compiler.labelnum++;
		UML_TEST(block, uml::mem(&m_core->stop), STOP_ABORT);                       // test    stop,ABORT
		UML_JMPc(block, uml::COND_Z, ok);                                           // jmp     ok,z
		if (!drc_static_control())
		{
			UML_LOAD(block, uml::I2, &m_control, 0, SIZE_DWORD, SCALE_x1);          // load    i2,[control],dword
			UML_TEST(block, uml::I2, COPRO_CTRL_LATE_ABORT);                        // test    i2,LATE_ABORT
			UML_JMPc(block, uml::COND_Z, abort);                                    // jmp     abort,z
			UML_MOV(block, drc_reg(rn), uml::I4);                                   // mov     rn,i4
		}
		else if (m_control & COPRO_CTRL_LATE_ABORT)
		{
			UML_MOV(block, drc_reg(rn), uml::I4);                                   // mov     rn,i4
		}
		UML_LABEL(block, abort);                                                    // abort:
		UML_EXH(block, *m_resync, desc->pc + 4);                                    // exh     resync,nextpc
		UML_LABEL(block, ok);                                                       // ok:
	}
	else
	{
		generate_abort_check(block, desc);
	}
	if (load && rd == eR15)
	{
		UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                          // mapvar  CYCLES,compiler.cycles
	}
	generate_access_end(block, done);

	// the write-back, then the data; a load of R15 is a branch, interworking from ARMv5
	if (writeback)
	{
		UML_MOV(block, drc_reg(rn), uml::I4);                                       // mov     rn,i4
	}
	if (load && rd == eR15)
	{
		generate_loaded_pc(block, compiler, desc, skip, m_archRev >= 5, true);
		return true;
	}
	if (load)
	{
		UML_MOV(block, drc_reg(rd), uml::I0);                                       // mov     rd,i0
	}

	generate_memory_end(block, desc);
	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}


// LDRH, STRH, LDRSB, LDRSH, LDRD and STRD
bool arm7_cpu_device::generate_mem_half(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
	const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
	const uint32_t rm = op & 0xf;
	const bool dword = !(op & INSN_SDT_L) && (op & 0x60) != 0x20;                  // LDRD, STRD
	const bool load = (op & INSN_SDT_L) || (op & 0x60) == 0x40;
	const bool user = GET_MODE == eARM7_MODE_USER;

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);

	// the offset: an immediate in two nibbles, or a register (R15 as it is)
	const uml::parameter offset = (op & 0x400000) ? uml::parameter(((op >> 4) & 0xf0) | (op & 0x0f)) : (rm == eR15) ? uml::parameter(desc->pc) : drc_reg(rm);
	const bool writeback = generate_mem_address(block, desc, op, offset, load);

	uml::code_label done(0);
	if (dword)
	{
		// LDRD, STRD: an abort on the first word skips the second; a load writes its registers only after both
		UML_AND(block, uml::I0, uml::I0, ~uint32_t(3));                             // and     i0,i0,~3
		UML_ADD(block, uml::I9, uml::I0, 4);                                        // add     i9,i0,4
		if (load)
		{
			const uml::code_label first = generate_access(block, compiler, 4, false, user);
			generate_abort_check(block, desc);
			generate_access_end(block, first);
			UML_MOV(block, uml::I8, uml::I0);                                       // mov     i8,i0
			UML_MOV(block, uml::I0, uml::I9);                                       // mov     i0,i9
			done = generate_access(block, compiler, 4, false, user);
		}
		else
		{
			UML_MOV(block, uml::I1, drc_reg(rd));                                   // mov     i1,rd
			const uml::code_label first = generate_access(block, compiler, 4, true, user);
			generate_abort_check(block, desc);
			generate_access_end(block, first);
			UML_MOV(block, uml::I0, uml::I9);                                       // mov     i0,i9
			UML_MOV(block, uml::I1, drc_reg(rd + 1));                               // mov     i1,rd+1
			done = generate_access(block, compiler, 4, true, user);
		}
	}
	else if (!load)
	{
		// STRH; R15 stores 12 bytes ahead
		UML_MOV(block, uml::I1, (rd == eR15) ? uml::parameter(desc->pc + 12) : drc_reg(rd));  // mov     i1,rd
		done = generate_access(block, compiler, 2, true, user);
	}
	else if (!(op & 0x40))
	{
		// LDRH
		done = generate_access(block, compiler, 2, false, user);
	}
	else if (!(op & 0x20))
	{
		// LDRSB
		done = generate_access(block, compiler, 1, false, user);
	}
	else
	{
		// LDRSH: the aligned halfword; before ARMv5 an odd address yields its high byte
		if (m_archRev < 5)
		{
			UML_AND(block, uml::I9, uml::I0, 1);                                    // and     i9,i0,1
			UML_SHL(block, uml::I9, uml::I9, 3);                                    // shl     i9,i9,3
		}
		UML_AND(block, uml::I0, uml::I0, ~uint32_t(1));                             // and     i0,i0,~1
		done = generate_access(block, compiler, 2, false, user);
	}

	// an abort: the base keeps its value
	generate_abort_check(block, desc);
	generate_access_end(block, done);

	// LDRSB, LDRSH: the sign
	if (load && !dword && (op & 0x40))
	{
		if (!(op & 0x20))
		{
			UML_SEXT(block, uml::I0, uml::I0, SIZE_BYTE);                           // sext    i0,i0,byte
		}
		else
		{
			UML_SEXT(block, uml::I0, uml::I0, SIZE_WORD);                           // sext    i0,i0,word
			if (m_archRev < 5)
			{
				UML_SAR(block, uml::I0, uml::I0, uml::I9);                          // sar     i0,i0,i9
			}
		}
	}

	// the write-back, then the data; a post-indexed LDRD writes its base back last
	const bool late_writeback = dword && load && !(op & INSN_SDT_P);
	if (writeback && !late_writeback)
	{
		UML_MOV(block, drc_reg(rn), uml::I4);                                       // mov     rn,i4
	}
	if (load)
	{
		if (dword)
		{
			UML_MOV(block, drc_reg(rd), uml::I8);                                   // mov     rd,i8
			UML_MOV(block, drc_reg(rd + 1), uml::I0);                               // mov     rd+1,i0
		}
		else
		{
			UML_MOV(block, drc_reg(rd), uml::I0);                                   // mov     rd,i0
		}
	}
	if (writeback && late_writeback)
	{
		UML_MOV(block, drc_reg(rn), uml::I4);                                       // mov     rn,i4
	}

	generate_memory_end(block, desc);
	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}

// one TLB probe for count words from I4, then words at I2 + 4n; misses go to the label returned (0: no fast path)
uml::code_label arm7_cpu_device::generate_block_probe(drcuml_block &block, compiler_state &compiler, uint32_t count, bool iswrite, bool user)
{
	if (!drc_tlb_inline() || count == 0)
	{
		return uml::code_label(0);
	}

	const uml::code_label slow = compiler.labelnum++;
	generate_tlb_lookup(block, uml::I4, iswrite, user, slow);
	UML_AND(block, uml::I3, uml::I4, 0xffc);                                        // and     i3,i4,0xffc
	UML_CMP(block, uml::I3, 0x1000 - 4 * count);                                    // cmp     i3,0x1000-4*count
	UML_JMPc(block, uml::COND_A, slow);                                             // jmp     slow,a
	if (DRC_TLB_CHECK && count > 1)
	{
		// the last word too: it must still be in the page
		UML_ADD(block, uml::mem(&m_core->arg0), uml::I4, 4 * (count - 1));          // add     arg0,i4,last
		UML_ADD(block, uml::I3, uml::I2, 4 * (count - 1));                          // add     i3,i2,last
		generate_tlb_check_call(block, uml::mem(&m_core->arg0), uml::I3, iswrite, user);
	}
	UML_AND(block, uml::I2, uml::I2, ~uint32_t(3));                                 // and     i2,i2,~3
	return slow;
}

void arm7_cpu_device::generate_block_word(drcuml_block &block, uint32_t index, bool iswrite, uml::parameter value)
{
	if (iswrite)
	{
		UML_STORE(block, m_drc_tlb_base + 4 * index, uml::I2, value, SIZE_DWORD, SCALE_x1);     // store   base+4*index,i2,value
	}
	else
	{
		UML_LOAD(block, uml::I0, m_drc_tlb_base + 4 * index, uml::I2, SIZE_DWORD, SCALE_x1);    // load    i0,base+4*index,i2
	}
}

bool arm7_cpu_device::generate_mem_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const uint32_t rb = (op & INSN_RN) >> INSN_RN_SHIFT;
	const uint32_t rlist = op & 0xffff;
	const uint32_t count = std::popcount(rlist);
	const bool load = (op & INSN_BDT_L) != 0;
	const bool up = (op & INSN_BDT_U) != 0;
	const bool pre = (op & INSN_BDT_P) != 0;
	const bool writeback = (op & INSN_BDT_W) != 0;
	const bool user = GET_MODE == eARM7_MODE_USER;

	// the S bit transfers the user bank (with R15 in the list it is the interpreter's)
	const int *const bank = (op & INSN_BDT_S) ? sRegisterTable[eARM7_MODE_USER] : m_reg_group;

	// a store with the base in the list after another register stores the written-back base (ARMv5: the old one)
	const bool base_late = (m_archRev < 5) && !load && writeback && BIT(rlist, rb) && (rlist & util::make_bitmask<uint32_t>(rb));

	// an abort: ARMv3 writes the base back in full, ARMv4+ restores an early-written one; both need the original
	const bool abort_v3 = writeback && m_archRev < 4;
	const bool abort_restore = base_late && m_archRev >= 4;
	const uint32_t stride = count << 2;

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);

	if (abort_v3 || abort_restore)
	{
		UML_MOV(block, uml::I9, drc_reg(rb));                                       // mov     i9,rb
	}

	// the lowest address: a load aligns the base first
	const int32_t start = up ? (pre ? 4 : 0) : (pre ? -int32_t(stride) : 4 - int32_t(stride));
	if (load)
	{
		UML_AND(block, uml::I4, drc_reg(rb), ~uint32_t(3));                         // and     i4,rb,~3
		if (start != 0)
		{
			UML_ADD(block, uml::I4, uml::I4, start);                                // add     i4,i4,start
		}
	}
	else if (start != 0)
	{
		UML_ADD(block, uml::I4, drc_reg(rb), start);                                // add     i4,rb,start
	}
	else
	{
		UML_MOV(block, uml::I4, drc_reg(rb));                                       // mov     i4,rb
	}

	if (base_late)
	{
		UML_ADD(block, drc_reg(rb), drc_reg(rb), up ? int32_t(stride) : -int32_t(stride));  // add     rb,rb,±stride
	}

	// a transfer the data TLB holds within one page: no aborts, the registers in order, R15 last in I0
	const uml::code_label slow = generate_block_probe(block, compiler, count, !load, user);
	uml::code_label fast_done(0);
	if (slow.label() != 0)
	{
		uint32_t index = 0;
		for (uint32_t i = 0; i < 16; i++)
		{
			if (BIT(rlist, i))
			{
				generate_block_word(block, index++, !load, (i == eR15) ? uml::parameter(desc->pc + 12) : uml::mem(&m_r[bank[i]]));
				if (load && i != eR15)
				{
					UML_MOV(block, uml::mem(&m_r[bank[i]]), uml::I0);               // mov     ri,i0
				}
			}
		}
		fast_done = compiler.labelnum++;
		UML_JMP(block, fast_done);                                                  // jmp     done
		UML_LABEL(block, slow);                                                     // slow:
	}

	// the registers, charging those done when an access aborts
	const uint32_t cycles_before = compiler.cycles - desc->cycles;
	uint32_t done = 0;
	for (uint32_t i = 0; i < 16; i++)
	{
		if (!BIT(rlist, i))
		{
			continue;
		}

		UML_MOV(block, uml::I0, uml::I4);                                           // mov     i0,i4
		if (load)
		{
			UML_CALLH(block, *m_read32[user ? 1 : 0]);                                      // callh   read32
		}
		else
		{
			UML_MOV(block, uml::I1, (i == eR15) ? uml::parameter(desc->pc + 12) : uml::mem(&m_r[bank[i]]));   // mov     i1,ri
			UML_CALLH(block, *m_write32[user ? 1 : 0]);                                     // callh   write32
		}

		UML_MAPVAR(block, MAPVAR_CYCLES, cycles_before + done + (load ? 2 : 1));    // mapvar  CYCLES,<aborted here>
		if (abort_v3 || abort_restore)
		{
			const uml::code_label ok = compiler.labelnum++;
			UML_TEST(block, uml::mem(&m_core->stop), STOP_ABORT);                   // test    stop,ABORT
			UML_JMPc(block, uml::COND_Z, ok);                                       // jmp     ok,z
			if (abort_v3)
			{
				UML_ADD(block, drc_reg(rb), uml::I9, up ? int32_t(stride) : -int32_t(stride));   // add     rb,i9,±stride
			}
			else
			{
				UML_MOV(block, drc_reg(rb), uml::I9);                               // mov     rb,i9
			}
			UML_EXH(block, *m_resync, desc->pc + 4);                                // exh     resync,nextpc
			UML_LABEL(block, ok);                                                   // ok:
		}
		else
		{
			generate_abort_check(block, desc);
		}

		if (load && i != eR15)
		{
			UML_MOV(block, uml::mem(&m_r[bank[i]]), uml::I0);                       // mov     ri,i0
		}
		done++;
		if (done < count)
		{
			UML_ADD(block, uml::I4, uml::I4, 4);                                    // add     i4,i4,4
		}
	}
	if (fast_done.label() != 0)
	{
		UML_LABEL(block, fast_done);                                                // done:
	}
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                              // mapvar  CYCLES,compiler.cycles

	// the write-back: a load of the base keeps the loaded value
	if (writeback && !base_late && !(load && ldm_loads_base(op, rb)))
	{
		UML_ADD(block, drc_reg(rb), drc_reg(rb), up ? int32_t(stride) : -int32_t(stride));  // add     rb,rb,±stride
	}

	// R15 came last and is still in I0: a branch, interworking from ARMv5
	if (load && BIT(rlist, 15))
	{
		generate_loaded_pc(block, compiler, desc, skip, m_archRev >= 5, true);
		return true;
	}

	generate_memory_end(block, desc);
	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}



/***************************************************************************
    Multiply instructions
***************************************************************************/

// 1 to 4 cycles by the significant bytes of Rs (HandleMul(), HandleSMulLong())
void arm7_cpu_device::generate_multiplier_cycles(drcuml_block &block, uint32_t op, uml::parameter rsval)
{
	UML_MOV(block, uml::I2, rsval);                                                 // mov     i2,rs
	if (!(op & 0x00800000) || (op & 0x00400000))
	{
		// MUL, MLA: the magnitude of a signed operand; SMULL, SMLAL: its complement when negative
		UML_SAR(block, uml::I3, uml::I2, 31);                                       // sar     i3,i2,31
		UML_XOR(block, uml::I2, uml::I2, uml::I3);                                  // xor     i2,i2,i3
		if (!(op & 0x00800000))
		{
			UML_SUB(block, uml::I2, uml::I2, uml::I3);                              // sub     i2,i2,i3
		}
	}
	UML_OR(block, uml::I2, uml::I2, 1);                                             // or      i2,i2,1
	UML_LZCNT(block, uml::I2, uml::I2);                                             // lzcnt   i2,i2
	UML_SHR(block, uml::I2, uml::I2, 3);                                            // shr     i2,i2,3
	UML_SUB(block, uml::I2, 4, uml::I2);                                            // sub     i2,4,i2
	UML_SUB(block, uml::mem(&m_core->icount), uml::mem(&m_core->icount), uml::I2);  // sub     icount,icount,i2
}

bool arm7_cpu_device::generate_mul(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const uint32_t rd = (op & INSN_MUL_RD) >> INSN_MUL_RD_SHIFT;
	const uint32_t rn = (op & INSN_MUL_RN) >> INSN_MUL_RN_SHIFT;
	const uint32_t rs = (op & INSN_MUL_RS) >> INSN_MUL_RS_SHIFT;
	const uint32_t rm = op & INSN_MUL_RM;
	const auto reg = [this, desc] (uint32_t regnum)
	{
		return (regnum == eR15) ? uml::parameter(desc->pc) : drc_reg(regnum);
	};

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);

	UML_MULULW(block, uml::I0, reg(rm), reg(rs));                                   // mululw  i0,rm,rs
	if (op & INSN_MUL_A)
	{
		UML_ADD(block, uml::I0, uml::I0, reg(rn));                                  // add     i0,i0,rn
	}
	if ((op & INSN_S) && desc->flags_live != 0)
	{
		UML_TEST(block, uml::I0, uml::I0);                                          // test    i0,i0
		generate_flags_nz(block);
	}

	// the multiplier's cycles come from Rs as it was (Rd may be Rs); ARM2 and ARM3 charge a flat count
	if (m_archRev >= 3)
	{
		generate_multiplier_cycles(block, op, reg(rs));
	}
	UML_MOV(block, drc_reg(rd), uml::I0);                                           // mov     rd,i0

	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}

bool arm7_cpu_device::generate_mul_long(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint32_t op = desc->opptr;
	const uint32_t rhi = (op >> 16) & 0xf;
	const uint32_t rlo = (op >> 12) & 0xf;
	const uint32_t rs = (op >> 8) & 0xf;
	const uint32_t rm = op & 0xf;
	const auto reg = [this, desc] (uint32_t regnum)
	{
		return (regnum == eR15) ? uml::parameter(desc->pc) : drc_reg(regnum);
	};

	const uml::code_label skip = generate_condition(block, compiler, op >> INSN_COND_SHIFT);

	if (op & 0x00400000)
	{
		UML_MULS(block, uml::I0, uml::I1, reg(rm), reg(rs));                        // muls    i0,i1,rm,rs
	}
	else
	{
		UML_MULU(block, uml::I0, uml::I1, reg(rm), reg(rs));                        // mulu    i0,i1,rm,rs
	}
	if (op & INSN_MUL_A)
	{
		UML_ADD(block, uml::I0, uml::I0, drc_reg(rlo));                             // add     i0,i0,rlo
		UML_ADDC(block, uml::I1, uml::I1, drc_reg(rhi));                            // addc    i1,i1,rhi
	}
	if (op & INSN_S)
	{
		// N is bit 63, Z the whole result
		UML_OR(block, uml::I2, uml::I0, uml::I1);                                   // or      i2,i0,i1
		UML_GETFLGS(block, uml::I2, uml::FLAG_Z);                                   // getflgs i2,Z
		UML_LOAD(block, uml::I2, NZCV_ADD.data(), uml::I2, SIZE_DWORD, SCALE_x4);   // load    i2,nzcv,i2,dword
		UML_AND(block, uml::I3, uml::I1, N_MASK);                                   // and     i3,i1,N
		UML_OR(block, uml::I2, uml::I2, uml::I3);                                   // or      i2,i2,i3
		UML_ROLINS(block, uml::mem(&m_r[eCPSR]), uml::I2, 0, N_MASK | Z_MASK);      // rolins  cpsr,i2,0,NZ
	}

	// the multiplier's cycles come from Rs as it was (RdHi or RdLo may be Rs)
	generate_multiplier_cycles(block, op, reg(rs));
	UML_MOV(block, drc_reg(rhi), uml::I1);                                          // mov     rhi,i1
	UML_MOV(block, drc_reg(rlo), uml::I0);                                          // mov     rlo,i0

	generate_condition_end(block, compiler, skip, desc->cycles);
	return true;
}

/***************************************************************************
    Thumb instructions
***************************************************************************/

// R15 writes other than branches, the exceptions and the ARMv6 additions stay with the interpreter
bool arm7_cpu_device::thumb_generates(uint16_t op) const
{
	switch (op >> 12)
	{
		case 0x0: case 0x1: case 0x2: case 0x3:
			return true;                                                            // shifts, add/sub, move/compare/add/sub immediate

		case 0x4:
			if (op & 0x0800)
			{
				return true;                                                        // LDR Rd, [PC, #imm]
			}
			if (!(op & 0x0400))
			{
				return true;                                                        // the ALU operations
			}
			switch ((op >> 8) & 3)
			{
				case 0: case 2:
					return (op & 0x87) != 0x87;                                     // ADD, MOV: not to R15
				case 1:
					return true;                                                    // CMP
				case 3:
				default:
					return !(op & 0x80) || m_archRev >= 5;                          // BX; BLX from ARMv5
			}

		case 0x5: case 0x6: case 0x7: case 0x8: case 0x9: case 0xa:
			return true;                                                            // loads, stores, address generation

		case 0xb:
			switch ((op >> 8) & 0xf)
			{
				case 0x0:
					return true;                                                    // ADD SP, #imm
				case 0x4: case 0x5:
					return (op & 0x1ff) != 0;                                       // PUSH with something to push
				case 0xc:
					return (op & 0xff) != 0;                                        // POP without PC
				case 0xd:
					return true;                                                    // POP {.., PC}
				default:
					return false;                                                   // BKPT, the ARMv6 forms, the traps
			}

		case 0xc:
			return (op & 0xff) != 0;                                                // LDMIA, STMIA with a list

		case 0xd:
			return ((op >> 8) & 0xf) < 0xe;                                         // Bcc; not the trap or SWI

		case 0xe:
			return !(op & 0x0800) || m_archRev >= 5;                                // B; the BLX suffix from ARMv5

		case 0xf:
		default:
			return true;                                                            // both halves of BL
	}
}


// address in I0; kind as the register offset forms encode it (STR, STRH, STRB, LDRSB, LDR, LDRH, LDRB, LDRSH)
void arm7_cpu_device::generate_thumb_access(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t kind, uint32_t rd)
{
	const bool user = GET_MODE == eARM7_MODE_USER;

	uml::code_label done(0);
	switch (kind)
	{
		case 0: // STR
			UML_MOV(block, uml::I1, drc_reg(rd));                                   // mov     i1,rd
			done = generate_access(block, compiler, 4, true, user);
			break;

		case 1: // STRH
			UML_MOV(block, uml::I1, drc_reg(rd));                                   // mov     i1,rd
			done = generate_access(block, compiler, 2, true, user);
			break;

		case 2: // STRB
			UML_MOV(block, uml::I1, drc_reg(rd));                                   // mov     i1,rd
			done = generate_access(block, compiler, 1, true, user);
			break;

		case 3: // LDRSB
		case 6: // LDRB
			done = generate_access(block, compiler, 1, false, user);
			break;

		case 4: // LDR
			done = generate_access(block, compiler, 4, false, user);
			break;

		case 5: // LDRH
			done = generate_access(block, compiler, 2, false, user);
			break;

		case 7: // LDRSH: the aligned halfword; before ARMv5 an odd address yields its high byte
		default:
			if (m_archRev < 5)
			{
				UML_AND(block, uml::I9, uml::I0, 1);                                // and     i9,i0,1
				UML_SHL(block, uml::I9, uml::I9, 3);                                // shl     i9,i9,3
			}
			UML_AND(block, uml::I0, uml::I0, ~uint32_t(1));                         // and     i0,i0,~1
			done = generate_access(block, compiler, 2, false, user);
			break;
	}

	// a hit passes over the abort test; then the sign of LDRSB and LDRSH
	generate_abort_check(block, desc);
	generate_access_end(block, done);
	if (kind == 3)
	{
		UML_SEXT(block, uml::I0, uml::I0, SIZE_BYTE);                               // sext    i0,i0,byte
	}
	else if (kind == 7)
	{
		UML_SEXT(block, uml::I0, uml::I0, SIZE_WORD);                               // sext    i0,i0,word
		if (m_archRev < 5)
		{
			UML_SAR(block, uml::I0, uml::I0, uml::I9);                              // sar     i0,i0,i9
		}
	}
	if (kind >= 3)
	{
		UML_MOV(block, drc_reg(rd), uml::I0);                                       // mov     rd,i0
	}
	generate_memory_end(block, desc);
}


// target in I0
void arm7_cpu_device::generate_thumb_bx(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uml::code_label thumb = compiler.labelnum++;
	const uml::code_label done = compiler.labelnum++;

	UML_TEST(block, uml::I0, 1);                                                    // test    i0,1
	UML_JMPc(block, uml::COND_NZ, thumb);                                           // jmp     thumb,nz
	UML_AND(block, uml::I0, uml::I0, ~uint32_t(3));                                 // and     i0,i0,~3
	UML_AND(block, uml::mem(&m_r[eCPSR]), uml::mem(&m_r[eCPSR]), ~T_MASK);          // and     cpsr,cpsr,~T
	UML_AND(block, uml::mem(&m_core->mode), uml::mem(&m_core->mode), ~uint32_t(2)); // and     mode,mode,~2
	if (debugger_enabled())
	{
		UML_CALLC(block, cfunc_privilege_hook, this);                               // callc   privilege_hook
	}
	UML_JMP(block, done);                                                           // jmp     done
	UML_LABEL(block, thumb);                                                        // thumb:
	UML_AND(block, uml::I0, uml::I0, ~uint32_t(1));                                 // and     i0,i0,~1
	UML_LABEL(block, done);                                                         // done:
	UML_MOV(block, uml::mem(&m_r[eR15]), uml::I0);                                  // mov     r15,i0
	generate_branch_end(block, compiler, desc, 0, 0);
}


// every Thumb instruction costs 3 cycles, a skipped Bcc included
bool arm7_cpu_device::generate_thumb(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr;
	const uint32_t rd = op & 7;
	const uint32_t rs = (op >> 3) & 7;
	const uml::parameter cpsr = uml::mem(&m_r[eCPSR]);
	const uml::parameter sp = drc_reg(eR13);
	const uml::parameter lr = drc_reg(eR14);
	const bool flags = desc->flags_live != 0;     // only when something reads them before they are next written

	switch (op >> 12)
	{
		case 0x0: case 0x1:
			if ((op & 0x1800) != 0x1800)
			{
				// LSL, LSR, ASR Rd, Rs, #imm: the ARM shifter's immediate form; N, Z and C from it
				const uint32_t op2 = (((op >> 6) & 0x1f) << 7) | (((op >> 11) & 3) << 5) | rs;
				uml::parameter carry = 0;
				const uml::parameter value = generate_shift_imm(block, desc, op2, flags, carry);
				UML_MOV(block, uml::I0, value);                                     // mov     i0,value
				if (flags)
				{
					UML_TEST(block, uml::I0, uml::I0);                              // test    i0,i0
					generate_flags_logical(block, carry);
				}
				UML_MOV(block, drc_reg(rd), uml::I0);                               // mov     rd,i0
			}
			else
			{
				// ADD, SUB Rd, Rs, Rn / #imm3
				const uml::parameter op2 = (op & 0x0400) ? uml::parameter((op >> 6) & 7) : drc_reg((op >> 6) & 7);
				if (op & 0x0200)
				{
					UML_SUB(block, uml::I0, drc_reg(rs), op2);                      // sub     i0,rs,op2
					if (flags)
					{
						generate_flags_arith(block, true);
					}
				}
				else
				{
					UML_ADD(block, uml::I0, drc_reg(rs), op2);                      // add     i0,rs,op2
					if (flags)
					{
						generate_flags_arith(block, false);
					}
				}
				UML_MOV(block, drc_reg(rd), uml::I0);                               // mov     rd,i0
			}
			return true;

		case 0x2: case 0x3:
			{
				// MOV, CMP, ADD, SUB Rd, #imm8
				const uint32_t reg = (op >> 8) & 7;
				const uint32_t imm = op & 0xff;
				switch ((op >> 11) & 3)
				{
					case 0:
						UML_MOV(block, uml::I0, imm);                               // mov     i0,imm
						if (flags)
						{
							UML_TEST(block, uml::I0, uml::I0);                      // test    i0,i0
							generate_flags_nz(block);
						}
						UML_MOV(block, drc_reg(reg), uml::I0);                      // mov     rd,i0
						break;
					case 1:
						UML_SUB(block, uml::I0, drc_reg(reg), imm);                 // sub     i0,rd,imm
						if (flags)
						{
							generate_flags_arith(block, true);
						}
						break;
					case 2:
						UML_ADD(block, uml::I0, drc_reg(reg), imm);                 // add     i0,rd,imm
						if (flags)
						{
							generate_flags_arith(block, false);
						}
						UML_MOV(block, drc_reg(reg), uml::I0);                      // mov     rd,i0
						break;
					case 3:
					default:
						UML_SUB(block, uml::I0, drc_reg(reg), imm);                 // sub     i0,rd,imm
						if (flags)
						{
							generate_flags_arith(block, true);
						}
						UML_MOV(block, drc_reg(reg), uml::I0);                      // mov     rd,i0
						break;
				}
			}
			return true;

		case 0x4:
			if (op & 0x0800)
			{
				// LDR Rd, [PC, #imm]: the word-aligned PC plus 4, folded from verified memory
				const void *host = nullptr;
				uint32_t word = 0;
				const offs_t address = ((desc->pc & ~uint32_t(2)) + 4) + ((op & 0xff) << 2);
				if (drc_literal(*desc, address, host, word))
				{
					generate_literal_check(block, compiler, desc, host, word);
					UML_MOV(block, drc_reg((op >> 8) & 7), word);                   // mov     rd,word
					return true;
				}
				UML_MOV(block, uml::I0, address);                                   // mov     i0,address
				generate_thumb_access(block, compiler, desc, 4, (op >> 8) & 7);
				return true;
			}
			if (!(op & 0x0400))
			{
				// the ALU operations on low registers
				switch ((op >> 6) & 0xf)
				{
					case 0x0: // AND
						UML_AND(block, uml::I0, drc_reg(rd), drc_reg(rs));          // and     i0,rd,rs
						if (flags)
						{
							generate_flags_nz(block);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0x1: // EOR
						UML_XOR(block, uml::I0, drc_reg(rd), drc_reg(rs));          // xor     i0,rd,rs
						if (flags)
						{
							generate_flags_nz(block);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0x2: case 0x3: case 0x4: case 0x7: // LSL, LSR, ASR, ROR by a register
						{
							static constexpr uint8_t SHIFT_TYPE[8] = { 0, 0, 0, 1, 2, 0, 0, 3 };
							UML_MOV(block, uml::I1, drc_reg(rd));                   // mov     i1,rd
							UML_AND(block, uml::I2, drc_reg(rs), 0xff);             // and     i2,rs,0xff
							UML_CALLH(block, *m_shift_reg[SHIFT_TYPE[(op >> 6) & 7]]); // callh   shift
							UML_MOV(block, uml::I0, uml::I1);                       // mov     i0,i1
							if (flags)
							{
								UML_TEST(block, uml::I0, uml::I0);                  // test    i0,i0
								generate_flags_logical(block, uml::I3);
							}
							UML_MOV(block, drc_reg(rd), uml::I0);                   // mov     rd,i0
						}
						break;
					case 0x5: // ADC
						UML_CARRY(block, cpsr, C_BIT);                              // carry   cpsr,C
						UML_ADDC(block, uml::I0, drc_reg(rd), drc_reg(rs));         // addc    i0,rd,rs
						if (flags)
						{
							generate_flags_arith(block, false);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0x6: // SBC: Rd + NOT Rs + C
						UML_XOR(block, uml::I1, drc_reg(rs), ~uint32_t(0));         // xor     i1,rs,~0
						UML_CARRY(block, cpsr, C_BIT);                              // carry   cpsr,C
						UML_ADDC(block, uml::I0, drc_reg(rd), uml::I1);             // addc    i0,rd,i1
						if (flags)
						{
							generate_flags_arith(block, false);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0x8: // TST
						UML_AND(block, uml::I0, drc_reg(rd), drc_reg(rs));          // and     i0,rd,rs
						if (flags)
						{
							generate_flags_nz(block);
						}
						break;
					case 0x9: // NEG
						UML_SUB(block, uml::I0, 0, drc_reg(rs));                    // sub     i0,0,rs
						if (flags)
						{
							generate_flags_arith(block, true);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0xa: // CMP
						UML_SUB(block, uml::I0, drc_reg(rd), drc_reg(rs));          // sub     i0,rd,rs
						if (flags)
						{
							generate_flags_arith(block, true);
						}
						break;
					case 0xb: // CMN
						UML_ADD(block, uml::I0, drc_reg(rd), drc_reg(rs));          // add     i0,rd,rs
						if (flags)
						{
							generate_flags_arith(block, false);
						}
						break;
					case 0xc: // ORR
						UML_OR(block, uml::I0, drc_reg(rd), drc_reg(rs));           // or      i0,rd,rs
						if (flags)
						{
							generate_flags_nz(block);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0xd: // MUL
						UML_MULULW(block, uml::I0, drc_reg(rd), drc_reg(rs));       // mululw  i0,rd,rs
						if (flags)
						{
							UML_TEST(block, uml::I0, uml::I0);                      // test    i0,i0
							generate_flags_nz(block);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0xe: // BIC
						UML_XOR(block, uml::I1, drc_reg(rs), ~uint32_t(0));         // xor     i1,rs,~0
						UML_AND(block, uml::I0, drc_reg(rd), uml::I1);              // and     i0,rd,i1
						if (flags)
						{
							generate_flags_nz(block);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
					case 0xf: // MVN
					default:
						UML_XOR(block, uml::I0, drc_reg(rs), ~uint32_t(0));         // xor     i0,rs,~0
						if (flags)
						{
							generate_flags_nz(block);
						}
						UML_MOV(block, drc_reg(rd), uml::I0);                       // mov     rd,i0
						break;
				}
				return true;
			}
			{
				// ADD, CMP, MOV with high registers; BX, BLX
				const uint32_t hd = rd | ((op >> 4) & 8);
				const uint32_t hs = rs | ((op >> 3) & 8);
				switch ((op >> 8) & 3)
				{
					case 0: // ADD: R15 as the source reads 4 ahead
						UML_ADD(block, drc_reg(hd), drc_reg(hd), (hs == eR15) ? uml::parameter(desc->pc + 4) : drc_reg(hs)); // add     hd,hd,hs
						break;
					case 1: // CMP: R15 reads as it is
						UML_SUB(block, uml::I0, (hd == eR15) ? uml::parameter(desc->pc) : drc_reg(hd), (hs == eR15) ? uml::parameter(desc->pc) : drc_reg(hs)); // sub     i0,hd,hs
						if (flags)
						{
							generate_flags_arith(block, true);
						}
						break;
					case 2: // MOV: R15 as the source reads 4 ahead
						UML_MOV(block, drc_reg(hd), (hs == eR15) ? uml::parameter(desc->pc + 4) : drc_reg(hs)); // mov     hd,hs
						break;
					case 3: // BX, BLX: the target is read before the link register is written
					default:
						if (op & 0x80)
						{
							UML_MOV(block, uml::I0, (hs == eR15) ? uml::parameter(desc->pc + 4) : drc_reg(hs)); // mov     i0,hs
							UML_MOV(block, lr, (desc->pc + 2) | 1);                 // mov     r14,(pc+2)|1
						}
						else
						{
							UML_MOV(block, uml::I0, (hs == eR15) ? uml::parameter(desc->pc + 4) : drc_reg(hs)); // mov     i0,hs
						}
						generate_thumb_bx(block, compiler, desc);
						break;
				}
			}
			return true;

		case 0x5:
			// loads and stores with a register offset
			UML_ADD(block, uml::I0, drc_reg(rs), drc_reg((op >> 6) & 7));          // add     i0,rn,rm
			generate_thumb_access(block, compiler, desc, (op >> 9) & 7, rd);
			return true;

		case 0x6:
			// LDR, STR with an immediate offset
			UML_ADD(block, uml::I0, drc_reg(rs), ((op >> 6) & 0x1f) << 2);          // add     i0,rn,imm
			generate_thumb_access(block, compiler, desc, (op & 0x0800) ? 4 : 0, rd);
			return true;

		case 0x7:
			// LDRB, STRB with an immediate offset
			UML_ADD(block, uml::I0, drc_reg(rs), (op >> 6) & 0x1f);                 // add     i0,rn,imm
			generate_thumb_access(block, compiler, desc, (op & 0x0800) ? 6 : 2, rd);
			return true;

		case 0x8:
			// LDRH, STRH with an immediate offset
			UML_ADD(block, uml::I0, drc_reg(rs), ((op >> 6) & 0x1f) << 1);          // add     i0,rn,imm
			generate_thumb_access(block, compiler, desc, (op & 0x0800) ? 5 : 1, rd);
			return true;

		case 0x9:
			// LDR, STR relative to SP
			UML_ADD(block, uml::I0, sp, (op & 0xff) << 2);                          // add     i0,sp,imm
			generate_thumb_access(block, compiler, desc, (op & 0x0800) ? 4 : 0, (op >> 8) & 7);
			return true;

		case 0xa:
			// ADD Rd, PC, #imm (the word-aligned PC plus 4); ADD Rd, SP, #imm
			if (op & 0x0800)
			{
				UML_ADD(block, drc_reg((op >> 8) & 7), sp, (op & 0xff) << 2);       // add     rd,sp,imm
			}
			else
			{
				UML_MOV(block, drc_reg((op >> 8) & 7), ((desc->pc + 4) & ~uint32_t(2)) + ((op & 0xff) << 2)); // mov     rd,address
			}
			return true;

		case 0xb:
			switch ((op >> 8) & 0xf)
			{
				case 0x0:
					// ADD SP, #imm
					UML_ADD(block, sp, sp, (op & 0x80) ? -int32_t((op & 0x7f) << 2) : int32_t((op & 0x7f) << 2)); // add     sp,sp,±imm
					return true;

				case 0x4: case 0x5:
					{
						// PUSH: from the lowest address up, the link register last; an abort stops it and keeps SP
						const bool user = GET_MODE == eARM7_MODE_USER;
						const uint32_t count = std::popcount(op & 0xffu) + ((op & 0x100) ? 1 : 0);
						UML_SUB(block, uml::I4, sp, count << 2);                    // sub     i4,sp,count*4
						const uml::code_label slow = generate_block_probe(block, compiler, count, true, user);
						uml::code_label fast_done(0);
						if (slow.label() != 0)
						{
							uint32_t index = 0;
							for (uint32_t i = 0; i < 9; i++)
							{
								if (BIT(op, i))
								{
									generate_block_word(block, index++, true, drc_reg((i == 8) ? 14 : i));
								}
							}
							fast_done = compiler.labelnum++;
							UML_JMP(block, fast_done);                              // jmp     done
							UML_LABEL(block, slow);                                 // slow:
						}
						for (uint32_t i = 0; i < 9; i++)
						{
							if (!BIT(op, i))
							{
								continue;
							}
							UML_MOV(block, uml::I0, uml::I4);                       // mov     i0,i4
							UML_MOV(block, uml::I1, drc_reg((i == 8) ? 14 : i));    // mov     i1,ri
							UML_CALLH(block, *m_write32[user ? 1 : 0]);             // callh   write32
							generate_abort_check(block, desc);
							UML_ADD(block, uml::I4, uml::I4, 4);                    // add     i4,i4,4
						}
						if (fast_done.label() != 0)
						{
							UML_LABEL(block, fast_done);                            // done:
						}
						UML_SUB(block, sp, sp, count << 2);                         // sub     sp,sp,count*4
						generate_memory_end(block, desc);
					}
					return true;

				case 0xc: case 0xd:
				default:
					{
						// POP: an abort stops the loading and keeps SP; R15 comes last and is a branch
						const bool user = GET_MODE == eARM7_MODE_USER;
						const bool pc = (op & 0x100) != 0;
						const uint32_t count = std::popcount(op & 0xffu) + (pc ? 1 : 0);
						UML_MOV(block, uml::I4, sp);                                // mov     i4,sp
						const uml::code_label slow = generate_block_probe(block, compiler, count, false, user);
						uml::code_label fast_done(0);
						if (slow.label() != 0)
						{
							// the registers in order, R15 last in I0
							uint32_t index = 0;
							for (uint32_t i = 0; i < 8; i++)
							{
								if (BIT(op, i))
								{
									generate_block_word(block, index++, false, uml::I0);
									UML_MOV(block, drc_reg(i), uml::I0);            // mov     ri,i0
								}
							}
							if (pc)
							{
								generate_block_word(block, index, false, uml::I0);
							}
							fast_done = compiler.labelnum++;
							UML_JMP(block, fast_done);                              // jmp     done
							UML_LABEL(block, slow);                                 // slow:
						}
						for (uint32_t i = 0; i < 8; i++)
						{
							if (!BIT(op, i))
							{
								continue;
							}
							UML_AND(block, uml::I0, uml::I4, ~uint32_t(3));         // and     i0,i4,~3
							UML_CALLH(block, *m_read32[user ? 1 : 0]);              // callh   read32
							generate_abort_check(block, desc);
							UML_MOV(block, drc_reg(i), uml::I0);                    // mov     ri,i0
							UML_ADD(block, uml::I4, uml::I4, 4);                    // add     i4,i4,4
						}
						if (pc)
						{
							UML_AND(block, uml::I0, uml::I4, ~uint32_t(3));         // and     i0,i4,~3
							UML_CALLH(block, *m_read32[user ? 1 : 0]);              // callh   read32
							generate_abort_check(block, desc);
						}
						if (fast_done.label() != 0)
						{
							UML_LABEL(block, fast_done);                            // done:
						}
						UML_ADD(block, sp, sp, count << 2);                         // add     sp,sp,count*4
						if (pc)
						{
							// ARMv5 interworks; an ARM target loses bits 1:0 (tg0b_d())
							if (m_archRev < 5)
							{
								UML_AND(block, uml::I0, uml::I0, ~uint32_t(1));     // and     i0,i0,~1
							}
							else
							{
								const uml::code_label thumb = compiler.labelnum++;
								const uml::code_label done = compiler.labelnum++;
								UML_TEST(block, uml::I0, 1);                        // test    i0,1
								UML_JMPc(block, uml::COND_NZ, thumb);               // jmp     thumb,nz
								UML_AND(block, uml::mem(&m_r[eCPSR]), uml::mem(&m_r[eCPSR]), ~T_MASK); // and     cpsr,cpsr,~T
								UML_AND(block, uml::mem(&m_core->mode), uml::mem(&m_core->mode), ~uint32_t(2)); // and     mode,mode,~2
								if (debugger_enabled())
								{
									UML_CALLC(block, cfunc_privilege_hook, this);   // callc   privilege_hook
								}
								UML_AND(block, uml::I0, uml::I0, ~uint32_t(3));     // and     i0,i0,~3
								UML_JMP(block, done);                               // jmp     done
								UML_LABEL(block, thumb);                            // thumb:
								UML_AND(block, uml::I0, uml::I0, ~uint32_t(1));     // and     i0,i0,~1
								UML_LABEL(block, done);                             // done:
							}
							UML_MOV(block, uml::mem(&m_r[eR15]), uml::I0);          // mov     r15,i0
							UML_TEST(block, uml::mem(&m_core->stop), STOP_REMAP);   // test    stop,REMAP
							UML_EXHc(block, uml::COND_NZ, *m_resync, uml::mem(&m_r[eR15])); // exh     resync,<r15>,nz
							generate_branch_end(block, compiler, desc, uml::code_label(0), 0);
							return true;
						}
						generate_memory_end(block, desc);
					}
					return true;
			}

		case 0xc:
			{
				// STMIA, LDMIA: the addresses are word aligned, the base ends up past the list
				const bool user = GET_MODE == eARM7_MODE_USER;
				const uint32_t rb = (op >> 8) & 7;
				const uint32_t count = std::popcount(op & 0xffu);
				UML_MOV(block, uml::I4, drc_reg(rb));                               // mov     i4,rb
				if (op & 0x0800)
				{
					// a load of the base keeps the loaded value; an abort stops the loading and keeps the base
					const uml::code_label slow = generate_block_probe(block, compiler, count, false, user);
					uml::code_label fast_done(0);
					if (slow.label() != 0)
					{
						uint32_t index = 0;
						for (uint32_t i = 0; i < 8; i++)
						{
							if (BIT(op, i))
							{
								generate_block_word(block, index++, false, uml::I0);
								UML_MOV(block, drc_reg(i), uml::I0);                // mov     ri,i0
							}
						}
						UML_ADD(block, uml::I4, uml::I4, count << 2);               // add     i4,i4,count*4
						fast_done = compiler.labelnum++;
						UML_JMP(block, fast_done);                                  // jmp     done
						UML_LABEL(block, slow);                                     // slow:
					}
					for (uint32_t i = 0; i < 8; i++)
					{
						if (!BIT(op, i))
						{
							continue;
						}
						UML_AND(block, uml::I0, uml::I4, ~uint32_t(3));             // and     i0,i4,~3
						UML_CALLH(block, *m_read32[user ? 1 : 0]);                  // callh   read32
						generate_abort_check(block, desc);
						UML_MOV(block, drc_reg(i), uml::I0);                        // mov     ri,i0
						UML_ADD(block, uml::I4, uml::I4, 4);                        // add     i4,i4,4
					}
					if (fast_done.label() != 0)
					{
						UML_LABEL(block, fast_done);                                // done:
					}
					if (!BIT(op, rb))
					{
						UML_MOV(block, drc_reg(rb), uml::I4);                       // mov     rb,i4
					}
				}
				else
				{
					// the base is stored written back unless first in the list (ARMv5: never); stores run on through an abort
					if ((m_archRev < 5) && BIT(op, rb) && (op & util::make_bitmask<uint32_t>(rb)))
					{
						UML_ADD(block, drc_reg(rb), drc_reg(rb), count << 2);       // add     rb,rb,count*4
					}
					const uml::code_label slow = generate_block_probe(block, compiler, count, true, user);
					uml::code_label fast_done(0);
					if (slow.label() != 0)
					{
						uint32_t index = 0;
						for (uint32_t i = 0; i < 8; i++)
						{
							if (BIT(op, i))
							{
								generate_block_word(block, index++, true, drc_reg(i));
							}
						}
						UML_ADD(block, uml::I4, uml::I4, count << 2);               // add     i4,i4,count*4
						fast_done = compiler.labelnum++;
						UML_JMP(block, fast_done);                                  // jmp     done
						UML_LABEL(block, slow);                                     // slow:
					}
					for (uint32_t i = 0; i < 8; i++)
					{
						if (!BIT(op, i))
						{
							continue;
						}
						UML_AND(block, uml::I0, uml::I4, ~uint32_t(3));             // and     i0,i4,~3
						UML_MOV(block, uml::I1, drc_reg(i));                        // mov     i1,ri
						UML_CALLH(block, *m_write32[user ? 1 : 0]);                 // callh   write32
						UML_ADD(block, uml::I4, uml::I4, 4);                        // add     i4,i4,4
					}
					if (fast_done.label() != 0)
					{
						UML_LABEL(block, fast_done);                                // done:
					}
					UML_MOV(block, drc_reg(rb), uml::I4);                           // mov     rb,i4
					generate_abort_check(block, desc);
				}
				generate_memory_end(block, desc);
			}
			return true;

		case 0xd:
			{
				// Bcc: taken or not, 3 cycles
				const uml::code_label skip = generate_condition(block, compiler, (op >> 8) & 0xf);
				generate_branch_end(block, compiler, desc, skip, desc->cycles);
			}
			return true;

		case 0xe:
			if (!(op & 0x0800))
			{
				// B
				generate_branch_end(block, compiler, desc, 0, 0);
			}
			else
			{
				// the BLX suffix: the target from the prefix's LR, word aligned, into ARM state
				UML_ADD(block, uml::I0, lr, (op & 0x7ff) << 1);                     // add     i0,lr,offset
				UML_AND(block, uml::I0, uml::I0, ~uint32_t(3));                     // and     i0,i0,~3
				UML_MOV(block, lr, (desc->pc + 2) | 1);                             // mov     r14,(pc+2)|1
				UML_MOV(block, uml::mem(&m_r[eR15]), uml::I0);                      // mov     r15,i0
				UML_AND(block, cpsr, cpsr, ~T_MASK);                                // and     cpsr,cpsr,~T
				UML_AND(block, uml::mem(&m_core->mode), uml::mem(&m_core->mode), ~uint32_t(2)); // and     mode,mode,~2
				if (debugger_enabled())
				{
					UML_CALLC(block, cfunc_privilege_hook, this);                   // callc   privilege_hook
				}
				generate_branch_end(block, compiler, desc, 0, 0);
			}
			return true;

		case 0xf:
		default:
			if (!(op & 0x0800))
			{
				// the BL/BLX prefix: LR takes the high part of the offset
				UML_MOV(block, lr, desc->pc + 4 + util::sext((op & 0x7ff) << 12, 23)); // mov     r14,pc+4+offset
			}
			else if (desc->targetpc != BRANCH_TARGET_DYNAMIC && !desc->is_branch_target() && desc != compiler.seqhead)
			{
				// a BL suffix right after its prefix has a static target, unless it can be entered on its own
				UML_MOV(block, lr, (desc->pc + 2) | 1);                             // mov     r14,(pc+2)|1
				generate_branch_end(block, compiler, desc, 0, 0);
			}
			else
			{
				// the BL suffix, the target from LR whatever the frontend fused
				UML_AND(block, uml::I0, lr, ~uint32_t(1));                          // and     i0,lr,~1
				UML_ADD(block, uml::I0, uml::I0, (op & 0x7ff) << 1);                // add     i0,i0,offset
				UML_MOV(block, lr, (desc->pc + 2) | 1);                             // mov     r14,(pc+2)|1
				UML_MOV(block, uml::mem(&m_r[eR15]), uml::I0);                      // mov     r15,i0
				generate_branch_end(block, compiler, desc, 0, 0, true);
			}
			return true;
	}
}
