// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// TODO:
// - DMA

#include "emu.h"
#include "debugger.h"
#include "hphybrid.h"

enum {
		HPHYBRID_A,
		HPHYBRID_B,
		HPHYBRID_C,
		HPHYBRID_D,
		HPHYBRID_P,
		HPHYBRID_R,
		HPHYBRID_IV,
		HPHYBRID_PA,
		HPHYBRID_DMAPA,
		HPHYBRID_DMAMA,
		HPHYBRID_DMAC,
		HPHYBRID_I
};

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Bits in m_flags
#define HPHYBRID_C_BIT      0   // Carry/extend
#define HPHYBRID_O_BIT      1   // Overflow
#define HPHYBRID_CB_BIT 2   // Cb
#define HPHYBRID_DB_BIT 3   // Db
#define HPHYBRID_INTEN_BIT  4   // Interrupt enable
#define HPHYBRID_DMAEN_BIT  5   // DMA enable
#define HPHYBRID_DMADIR_BIT 6   // DMA direction (1 = OUT)
#define HPHYBRID_HALT_BIT   7   // Halt flag
#define HPHYBRID_IRH_BIT    8   // IRH requested
#define HPHYBRID_IRL_BIT    9   // IRL requested
#define HPHYBRID_IRH_SVC_BIT    10  // IRH in service
#define HPHYBRID_IRL_SVC_BIT    11  // IRL in service

#define HPHYBRID_IV_MASK        0xfff0  // IV mask

#define CURRENT_PA      (m_reg_PA[ 0 ])

#define HP_RESET_ADDR   0x0020

const device_type HP_5061_3011 = &device_creator<hp_5061_3011_cpu_device>;

hp_hybrid_cpu_device::hp_hybrid_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname)
: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
		m_program_config("program", ENDIANNESS_BIG, 16, 16, -1),
		m_io_config("io", ENDIANNESS_BIG, 16, 6, -1)
{
}

void hp_hybrid_cpu_device::device_start()
{
		m_reg_A = 0;
		m_reg_B = 0;
		m_reg_P = HP_RESET_ADDR;
		m_reg_R = 0;
		m_reg_C = 0;
		m_reg_D = 0;
		m_reg_IV = 0;
		m_reg_PA[ 0 ] = 0;
		m_reg_PA[ 1 ] = 0;
		m_reg_PA[ 2 ] = 0;
		m_flags = 0;
		m_dmapa = 0;
		m_dmama = 0;
		m_dmac = 0;
		m_reg_I = 0;

		{
				state_add(HPHYBRID_A,  "A", m_reg_A);
				state_add(HPHYBRID_B,  "B", m_reg_B);
				state_add(HPHYBRID_C,  "C", m_reg_C);
				state_add(HPHYBRID_D,  "D", m_reg_D);
				state_add(HPHYBRID_P,  "P", m_reg_P);
				state_add(STATE_GENPC, "GENPC", m_reg_P).noshow();
				state_add(HPHYBRID_R,  "R", m_reg_R);
				state_add(STATE_GENSP, "GENSP", m_reg_R).noshow();
				state_add(HPHYBRID_IV, "IV", m_reg_IV);
				state_add(HPHYBRID_PA, "PA", m_reg_PA[ 0 ]);
				state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).noshow().formatstr("%9s");
				state_add(HPHYBRID_DMAPA , "DMAPA" , m_dmapa).noshow();
				state_add(HPHYBRID_DMAMA , "DMAMA" , m_dmama).noshow();
				state_add(HPHYBRID_DMAC , "DMAC" , m_dmac).noshow();
				state_add(HPHYBRID_I , "I" , m_reg_I).noshow();
		}

		m_program = &space(AS_PROGRAM);
		m_direct = &m_program->direct();
		m_io = &space(AS_IO);

		save_item(NAME(m_reg_A));
		save_item(NAME(m_reg_B));
		save_item(NAME(m_reg_C));
		save_item(NAME(m_reg_D));
		save_item(NAME(m_reg_P));
		save_item(NAME(m_reg_R));
		save_item(NAME(m_reg_IV));
		save_item(NAME(m_reg_PA[0]));
		save_item(NAME(m_reg_PA[1]));
		save_item(NAME(m_reg_PA[2]));
		save_item(NAME(m_flags));
		save_item(NAME(m_dmapa));
		save_item(NAME(m_dmama));
		save_item(NAME(m_dmac));
		save_item(NAME(m_reg_I));

		m_icountptr = &m_icount;
}

void hp_hybrid_cpu_device::device_reset()
{
		m_reg_P = HP_RESET_ADDR;
		m_reg_I = RM(m_reg_P);
		m_flags = 0;
}

void hp_hybrid_cpu_device::execute_run()
{
		do {
				debugger_instruction_hook(this, m_reg_P);

				// Check for interrupts
				check_for_interrupts();

				// TODO: check dma
				m_reg_I = execute_one(m_reg_I);
		} while (m_icount > 0);
}

void hp_hybrid_cpu_device::execute_set_input(int inputnum, int state)
{
		if (inputnum < HPHYBRID_INT_LVLS) {
				if (state) {
						BIT_SET(m_flags , HPHYBRID_IRH_BIT + inputnum);
				} else {
						BIT_CLR(m_flags , HPHYBRID_IRH_BIT + inputnum);
				}
		}
}

/**
 * Execute 1 instruction
 *
 * @param opcode Opcode to be executed
 *
 * @return Next opcode to be executed
 */
UINT16 hp_hybrid_cpu_device::execute_one(UINT16 opcode)
{
		if ((opcode & 0x7fe0) == 0x7000) {
				// EXE
				m_icount -= 8;
				return RM(opcode & 0x1f);
		} else {
				m_reg_P = execute_one_sub(opcode);
				return RM(m_reg_P);
		}
}

/**
 * Execute 1 instruction (except EXE)
 *
 * @param opcode Opcode to be executed (no EXE instructions)
 *
 * @return new value of P register
 */
UINT16 hp_hybrid_cpu_device::execute_one_sub(UINT16 opcode)
{
		UINT16 ea;
		UINT16 tmp;

		switch (opcode & 0x7800) {
		case 0x0000:
				// LDA
				m_icount -= 13;
				m_reg_A = RM(get_ea(opcode));
				break;

		case 0x0800:
				// LDB
				m_icount -= 13;
				m_reg_B = RM(get_ea(opcode));
				break;

		case 0x1000:
				// CPA
				m_icount -= 16;
				if (m_reg_A != RM(get_ea(opcode))) {
						// Skip next instruction
						return m_reg_P + 2;
				}
				break;

		case 0x1800:
				// CPB
				m_icount -= 16;
				if (m_reg_B != RM(get_ea(opcode))) {
						// Skip next instruction
						return m_reg_P + 2;
				}
				break;

		case 0x2000:
				// ADA
				m_icount -= 13;
				do_add(m_reg_A , RM(get_ea(opcode)));
				break;

		case 0x2800:
				// ADB
				m_icount -= 13;
				do_add(m_reg_B , RM(get_ea(opcode)));
				break;

		case 0x3000:
				// STA
				m_icount -= 13;
				WM(get_ea(opcode) , m_reg_A);
				break;

		case 0x3800:
				// STB
				m_icount -= 13;
				WM(get_ea(opcode) , m_reg_B);
				break;

		case 0x4000:
				// JSM
				m_icount -= 17;
				WM(++m_reg_R , m_reg_P);
				return get_ea(opcode);

		case 0x4800:
				// ISZ
				m_icount -= 19;
				ea = get_ea(opcode);
				tmp = RM(ea) + 1;
				WM(ea , tmp);
				if (tmp == 0) {
						// Skip next instruction
						return m_reg_P + 2;
				}
				break;

		case 0x5000:
				// AND
				m_icount -= 13;
				m_reg_A &= RM(get_ea(opcode));
				break;

		case 0x5800:
				// DSZ
				m_icount -= 19;
				ea = get_ea(opcode);
				tmp = RM(ea) - 1;
				WM(ea , tmp);
				if (tmp == 0) {
						// Skip next instruction
						return m_reg_P + 2;
				}
				break;

		case 0x6000:
				// IOR
				m_icount -= 13;
				m_reg_A |= RM(get_ea(opcode));
				break;

		case 0x6800:
				// JMP
				m_icount -= 8;
				return get_ea(opcode);

		default:
				switch (opcode & 0xfec0) {
				case 0x7400:
						// RZA
						// SZA
						m_icount -= 14;
						return get_skip_addr(opcode , m_reg_A == 0);

				case 0x7440:
						// RIA
						// SIA
						m_icount -= 14;
						return get_skip_addr(opcode , m_reg_A++ == 0);

				case 0x7480:
						// SFS
						// SFC
						m_icount -= 14;
						// TODO: read flag bit
						return get_skip_addr(opcode , true);

				case 0x7C00:
						// RZB
						// SZB
						m_icount -= 14;
						return get_skip_addr(opcode , m_reg_B == 0);

				case 0x7C40:
						// RIB
						// SIB
						m_icount -= 14;
						return get_skip_addr(opcode , m_reg_B++ == 0);

				case 0x7c80:
						// SSS
						// SSC
						m_icount -= 14;
						// TODO: read status bit
						return get_skip_addr(opcode , true);

				case 0x7cc0:
						// SHS
						// SHC
						m_icount -= 14;
						return get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_HALT_BIT));

				default:
						switch (opcode & 0xfe00) {
						case 0x7600:
								// SLA
								// RLA
								m_icount -= 14;
								return get_skip_addr_sc(opcode , m_reg_A , 0);

						case 0x7e00:
								// SLB
								// RLB
								m_icount -= 14;
								return get_skip_addr_sc(opcode , m_reg_B , 0);

						case 0xf400:
								// SAP
								// SAM
								m_icount -= 14;
								return get_skip_addr_sc(opcode , m_reg_A , 15);

						case 0xf600:
								// SOC
								// SOS
								m_icount -= 14;
								return get_skip_addr_sc(opcode , m_flags , HPHYBRID_O_BIT);

						case 0xfc00:
								// SBP
								// SBM
								m_icount -= 14;
								return get_skip_addr_sc(opcode , m_reg_B , 15);

						case 0xfe00:
								// SEC
								// SES
								m_icount -= 14;
								return get_skip_addr_sc(opcode , m_flags , HPHYBRID_C_BIT);

						default:
								switch (opcode & 0xfff0) {
								case 0xf100:
										// AAR
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										// A shift by 16 positions is equivalent to a shift by 15
										tmp = tmp > 15 ? 15 : tmp;
										m_reg_A = ((m_reg_A ^ 0x8000) >> tmp) - (0x8000 >> tmp);
										break;

								case 0xf900:
										// ABR
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										tmp = tmp > 15 ? 15 : tmp;
										m_reg_B = ((m_reg_B ^ 0x8000) >> tmp) - (0x8000 >> tmp);
										break;

								case 0xf140:
										// SAR
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										m_reg_A >>= tmp;
										break;

								case 0xf940:
										// SBR
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										m_reg_B >>= tmp;
										break;

								case 0xf180:
										// SAL
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										m_reg_A <<= tmp;
										break;

								case 0xf980:
										// SBL
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										m_reg_B <<= tmp;
										break;

								case 0xf1c0:
										// RAR
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										m_reg_A = (m_reg_A >> tmp) | (m_reg_A << (16 - tmp));
										break;

								case 0xf9c0:
										// RBR
										tmp = (opcode & 0xf) + 1;
										m_icount -= (9 + tmp);
										m_reg_B = (m_reg_B >> tmp) | (m_reg_B << (16 - tmp));
										break;

								default:
										if ((opcode & 0xf760) == 0x7160) {
												// Place/withdraw instructions
												m_icount -= 23;
												do_pw(opcode);
										} else if ((opcode & 0xff80) == 0xf080) {
												// RET
												m_icount -= 16;
												if (BIT(opcode , 6)) {
														// Pop PA stack
														if (BIT(m_flags , HPHYBRID_IRH_SVC_BIT)) {
																BIT_CLR(m_flags , HPHYBRID_IRH_SVC_BIT);
																memmove(&m_reg_PA[ 0 ] , &m_reg_PA[ 1 ] , HPHYBRID_INT_LVLS);
														} else if (BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
																BIT_CLR(m_flags , HPHYBRID_IRL_SVC_BIT);
																memmove(&m_reg_PA[ 0 ] , &m_reg_PA[ 1 ] , HPHYBRID_INT_LVLS);
														}
												}
												tmp = RM(m_reg_R--) + (opcode & 0x1f);
												return BIT(opcode , 5) ? tmp - 0x20 : tmp;
										} else {
												switch (opcode) {
												case 0x7100:
														// SDO
														m_icount -= 12;
														BIT_SET(m_flags , HPHYBRID_DMADIR_BIT);
														break;

												case 0x7108:
														// SDI
														m_icount -= 12;
														BIT_CLR(m_flags , HPHYBRID_DMADIR_BIT);
														break;

												case 0x7110:
														// EIR
														m_icount -= 12;
														BIT_SET(m_flags , HPHYBRID_INTEN_BIT);
														break;

												case 0x7118:
														// DIR
														m_icount -= 12;
														BIT_CLR(m_flags , HPHYBRID_INTEN_BIT);
														break;

												case 0x7120:
														// DMA
														m_icount -= 12;
														BIT_SET(m_flags , HPHYBRID_DMAEN_BIT);
														break;

												case 0x7138:
														// DDR
														m_icount -= 12;
														BIT_CLR(m_flags , HPHYBRID_DMAEN_BIT);
														break;

												case 0x7140:
														// DBL
														m_icount -= 12;
														BIT_CLR(m_flags , HPHYBRID_DB_BIT);
														break;

												case 0x7148:
														// CBL
														m_icount -= 12;
														BIT_CLR(m_flags , HPHYBRID_CB_BIT);
														break;

												case 0x7150:
														// DBU
														m_icount -= 12;
														BIT_SET(m_flags , HPHYBRID_DB_BIT);
														break;

												case 0x7158:
														// CBU
														m_icount -= 12;
														BIT_SET(m_flags , HPHYBRID_CB_BIT);
														break;

												case 0xf020:
														// TCA
														m_icount -= 9;
														m_reg_A = ~m_reg_A;
														do_add(m_reg_A , 1);
														break;

												case 0xf060:
														// CMA
														m_icount -= 9;
														m_reg_A = ~m_reg_A;
														break;

												case 0xf820:
														// TCB
														m_icount -= 9;
														m_reg_B = ~m_reg_B;
														do_add(m_reg_B , 1);
														break;

												case 0xf860:
														// CMB
														m_icount -= 9;
														m_reg_B = ~m_reg_B;
														break;

												default:
														// Unrecognized instructions: NOP
														// Execution time is fictional
														m_icount -= 6;
												}
										}
								}
						}
				}
		}

		return m_reg_P + 1;
}

void hp_hybrid_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
		if (entry.index() == STATE_GENFLAGS) {
				strprintf(str, "%s %s %c %c",
							BIT(m_flags , HPHYBRID_DB_BIT) ? "Db":"..",
							BIT(m_flags , HPHYBRID_CB_BIT) ? "Cb":"..",
							BIT(m_flags , HPHYBRID_O_BIT) ? 'O':'.',
							BIT(m_flags , HPHYBRID_C_BIT) ? 'E':'.');
		}
}

offs_t hp_hybrid_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
		extern CPU_DISASSEMBLE(hp_hybrid);
		return CPU_DISASSEMBLE_NAME(hp_hybrid)(this, buffer, pc, oprom, opram, options);
}

UINT16 hp_hybrid_cpu_device::get_ea(UINT16 opcode)
{
		UINT16 base;
		UINT16 off;

		if (BIT(opcode , 10)) {
				// Current page
				base = m_reg_P;
		} else {
				// Base page
				base = 0;
		}

		off = opcode & 0x3ff;
		if (off & 0x200) {
				off -= 0x400;
		}

		base += off;

		if (BIT(opcode , 15)) {
				// Indirect addressing
				m_icount -= 6;
				return RM(base);
		} else {
				// Direct addressing
				return base;
		}
}

void hp_hybrid_cpu_device::do_add(UINT16& addend1 , UINT16 addend2)
{
		UINT32 tmp = addend1 + addend2;

		if (BIT(tmp , 16)) {
				// Carry
				BIT_SET(m_flags , HPHYBRID_C_BIT);
		}

		if (BIT((tmp ^ addend1) & (tmp ^ addend2) , 15)) {
				// Overflow
				BIT_SET(m_flags , HPHYBRID_O_BIT);
		}

		addend1 = (UINT16)tmp;
}

UINT16 hp_hybrid_cpu_device::get_skip_addr(UINT16 opcode , bool condition) const
{
		bool skip_val = BIT(opcode , 8) != 0;

		if (condition == skip_val) {
				UINT16 off = opcode & 0x1f;

				if (BIT(opcode , 5)) {
						off -= 0x20;
				}
				return m_reg_P + off;
		} else {
				return m_reg_P + 1;
		}
}

UINT16 hp_hybrid_cpu_device::get_skip_addr_sc(UINT16 opcode , UINT16& v , unsigned n)
{
		bool val = BIT(v , n);

		if (BIT(opcode , 7)) {
				if (BIT(opcode , 6)) {
						BIT_SET(v , n);
				} else {
						BIT_CLR(v , n);
				}
		}

		return get_skip_addr(opcode , val);
}

void hp_hybrid_cpu_device::do_pw(UINT16 opcode)
{
		UINT16 tmp;
		UINT16 reg_addr = opcode & 7;
		UINT16 *ptr_reg;
		UINT16 b_mask;

		if (BIT(opcode , 3)) {
				ptr_reg = &m_reg_D;
				b_mask = BIT_MASK(HPHYBRID_DB_BIT);
		} else {
				ptr_reg = &m_reg_C;
				b_mask = BIT_MASK(HPHYBRID_CB_BIT);
		}

		if (BIT(opcode , 4)) {
				// Withdraw
				if (BIT(opcode , 11)) {
						// Byte
						UINT32 tmp_addr = (UINT32)(*ptr_reg);
						if (m_flags & b_mask) {
								tmp_addr |= 0x10000;
						}
						tmp = RM((UINT16)(tmp_addr >> 1));
						if (BIT(tmp_addr , 0)) {
								tmp &= 0xff;
						} else {
								tmp >>= 8;
						}
				} else {
						// Word
						tmp = RM(*ptr_reg);
				}
				WM(reg_addr , tmp);

				if (BIT(opcode , 7)) {
						// Post-decrement
						if ((*ptr_reg)-- == 0) {
								m_flags ^= b_mask;
						}
				} else {
						// Post-increment
						if (++(*ptr_reg) == 0) {
								m_flags ^= b_mask;
						}
				}
		} else {
				// Place
				if (BIT(opcode , 7)) {
						// Pre-decrement
						if ((*ptr_reg)-- == 0) {
								m_flags ^= b_mask;
						}
				} else {
						// Pre-increment
						if (++(*ptr_reg) == 0) {
								m_flags ^= b_mask;
						}
				}
				tmp = RM(reg_addr);
				if (BIT(opcode , 11)) {
						// Byte
						UINT32 tmp_addr = (UINT32)(*ptr_reg);
						if (m_flags & b_mask) {
								tmp_addr |= 0x10000;
						}
						WMB(tmp_addr , (UINT8)tmp);
				} else {
						// Word
						WM(*ptr_reg , tmp);
				}
		}
}

void hp_hybrid_cpu_device::check_for_interrupts(void)
{
		if (!BIT(m_flags , HPHYBRID_INTEN_BIT) || BIT(m_flags , HPHYBRID_IRH_SVC_BIT)) {
				return;
		}

		int irqline;

		if (BIT(m_flags , HPHYBRID_IRH_BIT)) {
				// Service high-level interrupt
				BIT_SET(m_flags , HPHYBRID_IRH_SVC_BIT);
				irqline = HPHYBRID_IRH;
		} else if (BIT(m_flags , HPHYBRID_IRL_BIT) && !BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
				// Service low-level interrupt
				BIT_SET(m_flags , HPHYBRID_IRL_SVC_BIT);
				irqline = HPHYBRID_IRL;
		} else {
				return;
		}

		// Get interrupt vector in low byte
		UINT8 vector = (UINT8)standard_irq_callback(irqline);
		UINT8 new_PA;

		// Get highest numbered 1
		// Don't know what happens if vector is 0, here we assume bit 7 = 1
		if (vector == 0) {
				new_PA = 7;
		} else {
				for (new_PA = 7; new_PA && !BIT(vector , 7); new_PA--, vector <<= 1) {
				}
		}
		if (irqline == HPHYBRID_IRH) {
				BIT_SET(new_PA , 3);
		}

		// Push PA stack
		memmove(&m_reg_PA[ 1 ] , &m_reg_PA[ 0 ] , HPHYBRID_INT_LVLS);

		CURRENT_PA = new_PA;

		// Is this correct? Patent @ pg 210 suggests that the whole interrupt recognition sequence
		// lasts for 32 cycles (6 are already accounted for in get_ea for one indirection)
		m_icount -= 26;

		// Do a double-indirect JSM IV,I instruction
		WM(++m_reg_R , m_reg_P);
		m_reg_P = RM(get_ea(0xc008));
		m_reg_I = RM(m_reg_P);
}

UINT16 hp_hybrid_cpu_device::RM(UINT16 addr)
{
		UINT16 tmp;

		if (addr <= HP_REG_LAST_ADDR) {
				// Memory mapped registers
				switch (addr) {
				case HP_REG_A_ADDR:
						return m_reg_A;

				case HP_REG_B_ADDR:
						return m_reg_B;

				case HP_REG_P_ADDR:
						return m_reg_P;

				case HP_REG_R_ADDR:
						return m_reg_R;

				case HP_REG_R4_ADDR:
				case HP_REG_R5_ADDR:
				case HP_REG_R6_ADDR:
				case HP_REG_R7_ADDR:
						return RIO(CURRENT_PA , addr - HP_REG_R4_ADDR);

				case HP_REG_IV_ADDR:
						// Correct?
						if (!BIT(m_flags , HPHYBRID_IRH_SVC_BIT) && !BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
								return m_reg_IV;
						} else {
								return m_reg_IV | CURRENT_PA;
						}

				case HP_REG_PA_ADDR:
						return CURRENT_PA;

				case HP_REG_DMAPA_ADDR:
						tmp = m_dmapa & HP_REG_PA_MASK;
						if (BIT(m_flags , HPHYBRID_CB_BIT)) {
								BIT_SET(tmp , 15);
						}
						if (BIT(m_flags , HPHYBRID_DB_BIT)) {
								BIT_SET(tmp , 14);
						}
						return tmp;

				case HP_REG_DMAMA_ADDR:
						return m_dmama;

				case HP_REG_DMAC_ADDR:
						return m_dmac;

				case HP_REG_C_ADDR:
						return m_reg_C;

				case HP_REG_D_ADDR:
						return m_reg_D;

				default:
						// Unknown registers are returned as 0
						return 0;
				}
		} else {
				return m_direct->read_decrypted_word((offs_t)addr << 1);
		}
}

void hp_hybrid_cpu_device::WM(UINT16 addr , UINT16 v)
{
		if (addr <= HP_REG_LAST_ADDR) {
				// Memory mapped registers
				switch (addr) {
				case HP_REG_A_ADDR:
						m_reg_A = v;
						break;

				case HP_REG_B_ADDR:
						m_reg_B = v;
						break;

				case HP_REG_P_ADDR:
						m_reg_P = v;
						break;

				case HP_REG_R_ADDR:
						m_reg_R = v;
						break;

				case HP_REG_R4_ADDR:
				case HP_REG_R5_ADDR:
				case HP_REG_R6_ADDR:
				case HP_REG_R7_ADDR:
						WIO(CURRENT_PA , addr - HP_REG_R4_ADDR , v);
						break;

				case HP_REG_IV_ADDR:
						m_reg_IV = v & HP_REG_IV_MASK;
						break;

				case HP_REG_PA_ADDR:
						CURRENT_PA = v & HP_REG_PA_MASK;
						break;

				case HP_REG_DMAPA_ADDR:
						m_dmapa = v & HP_REG_PA_MASK;
						break;

				case HP_REG_DMAMA_ADDR:
						m_dmama = v;
						break;

				case HP_REG_DMAC_ADDR:
						m_dmac = v;
						break;

				case HP_REG_C_ADDR:
						m_reg_C = v;
						break;

				case HP_REG_D_ADDR:
						m_reg_D = v;
						break;

				default:
						// Unknown registers are silently discarded
						break;
				}
		} else {
				m_program->write_word((offs_t)addr << 1 , v);
		}
}

void hp_hybrid_cpu_device::WMB(UINT32 addr , UINT8 v)
{
		if (addr <= (HP_REG_LAST_ADDR * 2 + 1)) {
				// Cannot write bytes to registers
		} else {
				m_program->write_byte(addr , v);
		}
}

UINT16 hp_hybrid_cpu_device::RIO(UINT8 pa , UINT8 ic)
{
		return m_io->read_word(HP_MAKE_IOADDR(pa, ic) << 1);
}

void hp_hybrid_cpu_device::WIO(UINT8 pa , UINT8 ic , UINT16 v)
{
		m_io->write_word(HP_MAKE_IOADDR(pa, ic) << 1 , v);
}

hp_5061_3011_cpu_device::hp_5061_3011_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: hp_hybrid_cpu_device(mconfig, HP_5061_3011, "HP_5061_3011", tag, owner, clock, "5061-3011")
{
}
