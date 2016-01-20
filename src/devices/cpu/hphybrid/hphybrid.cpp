// license:BSD-3-Clause
// copyright-holders:F. Ulivi

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
				HPHYBRID_I,
								HPHYBRID_W,
								HPHYBRID_AR2,
								HPHYBRID_AR2_2,
								HPHYBRID_AR2_3,
								HPHYBRID_AR2_4,
								HPHYBRID_SE,
								HPHYBRID_R25,
								HPHYBRID_R26,
								HPHYBRID_R27,
								HPHYBRID_R32,
								HPHYBRID_R33,
								HPHYBRID_R34,
								HPHYBRID_R35,
								HPHYBRID_R36,
								HPHYBRID_R37
};

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Bits in m_flags
#define HPHYBRID_C_BIT          0   // Carry/extend
#define HPHYBRID_O_BIT          1   // Overflow
#define HPHYBRID_CB_BIT         2   // Cb
#define HPHYBRID_DB_BIT         3   // Db
#define HPHYBRID_INTEN_BIT      4   // Interrupt enable
#define HPHYBRID_DMAEN_BIT      5   // DMA enable
#define HPHYBRID_DMADIR_BIT     6   // DMA direction (1 = OUT)
#define HPHYBRID_HALT_BIT       7   // Halt flag
#define HPHYBRID_IRH_BIT        8   // IRH requested
#define HPHYBRID_IRL_BIT        9   // IRL requested
#define HPHYBRID_IRH_SVC_BIT    10  // IRH in service
#define HPHYBRID_IRL_SVC_BIT    11  // IRL in service
#define HPHYBRID_DMAR_BIT       12  // DMA request
#define HPHYBRID_STS_BIT        13  // Status flag
#define HPHYBRID_FLG_BIT        14  // "Flag" flag
#define HPHYBRID_DC_BIT         15  // Decimal carry

#define HPHYBRID_IV_MASK        0xfff0  // IV mask

#define HP_REG_SE_MASK  0x000f

#define CURRENT_PA      (m_reg_PA[ 0 ])

#define HP_RESET_ADDR   0x0020

// Part of r32-r37 that is actually output as address extension (6 bits of "BSC": block select code)
#define BSC_REG_MASK    0x3f

const device_type HP_5061_3001 = &device_creator<hp_5061_3001_cpu_device>;
const device_type HP_5061_3011 = &device_creator<hp_5061_3011_cpu_device>;

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::dmar_w)
{
				if (state) {
								BIT_SET(m_flags , HPHYBRID_DMAR_BIT);
				} else {
								BIT_CLR(m_flags , HPHYBRID_DMAR_BIT);
				}
}

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::halt_w)
{
		if (state) {
				BIT_SET(m_flags , HPHYBRID_HALT_BIT);
		} else {
				BIT_CLR(m_flags , HPHYBRID_HALT_BIT);
		}
}

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::status_w)
{
		if (state) {
				BIT_SET(m_flags , HPHYBRID_STS_BIT);
		} else {
				BIT_CLR(m_flags , HPHYBRID_STS_BIT);
		}
}

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::flag_w)
{
		if (state) {
				BIT_SET(m_flags , HPHYBRID_FLG_BIT);
		} else {
				BIT_CLR(m_flags , HPHYBRID_FLG_BIT);
		}
}

hp_hybrid_cpu_device::hp_hybrid_cpu_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname , UINT8 addrwidth)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
			m_pa_changed_func(*this),
			m_program_config("program", ENDIANNESS_BIG, 16, addrwidth, -1),
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
								m_reg_W = 0;
				m_flags = 0;
				m_dmapa = 0;
				m_dmama = 0;
				m_dmac = 0;
				m_reg_I = 0;
								m_forced_bsc_25 = false;

				{
								state_add(HPHYBRID_A,  "A", m_reg_A);
								state_add(HPHYBRID_B,  "B", m_reg_B);
								state_add(HPHYBRID_C,  "C", m_reg_C);
								state_add(HPHYBRID_D,  "D", m_reg_D);
								state_add(HPHYBRID_P,  "P", m_reg_P);
								state_add(STATE_GENPC, "GENPC", m_genpc).noshow();
								state_add(HPHYBRID_R,  "R", m_reg_R);
								state_add(STATE_GENSP, "GENSP", m_reg_R).noshow();
								state_add(HPHYBRID_IV, "IV", m_reg_IV);
								state_add(HPHYBRID_PA, "PA", m_reg_PA[ 0 ]);
																state_add(HPHYBRID_W, "W", m_reg_W).noshow();
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
								save_item(NAME(m_reg_W));
				save_item(NAME(m_flags));
				save_item(NAME(m_dmapa));
				save_item(NAME(m_dmama));
				save_item(NAME(m_dmac));
				save_item(NAME(m_reg_I));
								save_item(NAME(m_forced_bsc_25));

				m_icountptr = &m_icount;

								m_pa_changed_func.resolve_safe();
}

void hp_hybrid_cpu_device::device_reset()
{
				m_reg_P = HP_RESET_ADDR;
				m_reg_I = fetch();
				m_flags = 0;
}

void hp_hybrid_cpu_device::execute_run()
{
				do {
								if (BIT(m_flags , HPHYBRID_DMAEN_BIT) && BIT(m_flags , HPHYBRID_DMAR_BIT)) {
												handle_dma();
								} else {
												debugger_instruction_hook(this, m_genpc);

												// Check for interrupts
												check_for_interrupts();

												m_reg_I = execute_one(m_reg_I);
								}
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
								return fetch();
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
				UINT32 ea;
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
								WM(AEC_CASE_C , ++m_reg_R , m_reg_P);
								return remove_mae(get_ea(opcode));

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
								return remove_mae(get_ea(opcode));

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
												return get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_FLG_BIT));

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
												return get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_STS_BIT));

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
																																																																m_pa_changed_func((UINT8)CURRENT_PA);
																												} else if (BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
																																BIT_CLR(m_flags , HPHYBRID_IRL_SVC_BIT);
																																memmove(&m_reg_PA[ 0 ] , &m_reg_PA[ 1 ] , HPHYBRID_INT_LVLS);
																																																																m_pa_changed_func((UINT8)CURRENT_PA);
																												}
																								}
																								tmp = RM(AEC_CASE_C , m_reg_R--) + (opcode & 0x1f);
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
																																																		// Unrecognized instruction: pass it on for further processing (by EMC if present)
																																																		return execute_no_bpc_ioc(opcode);
																																																}
																																								}
																																}
																								}
																}
								}

				return m_reg_P + 1;
}

void hp_hybrid_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
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

UINT16 hp_hybrid_cpu_device::remove_mae(UINT32 addr)
{
		return (UINT16)(addr & 0xffff);
}

UINT16 hp_hybrid_cpu_device::RM(aec_cases_t aec_case , UINT16 addr)
{
		return RM(add_mae(aec_case , addr));
}

UINT16 hp_hybrid_cpu_device::RM(UINT32 addr)
{
		UINT16 tmp;
		UINT16 addr_wo_bsc = remove_mae(addr);

		if (addr_wo_bsc <= HP_REG_LAST_ADDR) {
				// Any access to internal registers removes forcing of BSC 2x
				m_forced_bsc_25 = false;

				// Memory mapped registers that are present in both 3001 & 3011
				switch (addr_wo_bsc) {
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
						return RIO(CURRENT_PA , addr_wo_bsc - HP_REG_R4_ADDR);

				case HP_REG_IV_ADDR:
						// Correct?
						if (!BIT(m_flags , HPHYBRID_IRH_SVC_BIT) && !BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
								return m_reg_IV;
						} else {
								return m_reg_IV | CURRENT_PA;
						}

				case HP_REG_PA_ADDR:
						return CURRENT_PA;

				case HP_REG_W_ADDR:
						return m_reg_W;

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
						return read_non_common_reg(addr_wo_bsc);
				}
		} else {
				return m_direct->read_word(addr << 1);
		}
}

void hp_hybrid_cpu_device::WM(aec_cases_t aec_case , UINT16 addr , UINT16 v)
{
		WM(add_mae(aec_case , addr) , v);
}

void hp_hybrid_cpu_device::WM(UINT32 addr , UINT16 v)
{
		UINT16 addr_wo_bsc = remove_mae(addr);

		if (addr_wo_bsc <= HP_REG_LAST_ADDR) {
				// Any access to internal registers removes forcing of BSC 2x
				m_forced_bsc_25 = false;

				// Memory mapped registers
				switch (addr_wo_bsc) {
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
						WIO(CURRENT_PA , addr_wo_bsc - HP_REG_R4_ADDR , v);
						break;

				case HP_REG_IV_ADDR:
						m_reg_IV = v & HP_REG_IV_MASK;
						break;

				case HP_REG_PA_ADDR:
						CURRENT_PA = v & HP_REG_PA_MASK;
						m_pa_changed_func((UINT8)CURRENT_PA);
						break;

				case HP_REG_W_ADDR:
						m_reg_W = v;
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
						write_non_common_reg(addr_wo_bsc , v);
						break;
				}
		} else {
				m_program->write_word(addr << 1 , v);
		}
}

UINT16 hp_hybrid_cpu_device::fetch(void)
{
		m_genpc = add_mae(AEC_CASE_A , m_reg_P);
		return RM(m_genpc);
}

UINT32 hp_hybrid_cpu_device::get_ea(UINT16 opcode)
{
		UINT16 base;
		UINT16 off;
		aec_cases_t aec;

		if (BIT(opcode , 10)) {
				// Current page
				base = m_reg_P;
				aec = AEC_CASE_A;
		} else {
				// Base page
				base = 0;
				aec = AEC_CASE_B;
		}

		off = opcode & 0x3ff;
		if (off & 0x200) {
				off -= 0x400;
		}

		base += off;

		if (BIT(opcode , 15)) {
				// Indirect addressing
				m_icount -= 6;
				return add_mae(AEC_CASE_C , RM(aec , base));
		} else {
				// Direct addressing
				return add_mae(aec , base);
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
												tmp = RM(AEC_CASE_C , (UINT16)(tmp_addr >> 1));
												if (BIT(tmp_addr , 0)) {
																tmp &= 0xff;
												} else {
																tmp >>= 8;
												}
								} else {
												// Word
												tmp = RM(AEC_CASE_C , *ptr_reg);
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
																								if (tmp_addr <= (HP_REG_LAST_ADDR * 2 + 1)) {
																										// Single bytes can be written to registers.
																										// The addressed register gets the written byte in the proper position
																										// and a 0 in the other byte because access to registers is always done in
																										// 16 bits units.
																										if (BIT(tmp_addr , 0)) {
																												tmp &= 0xff;
																										} else {
																												tmp <<= 8;
																										}
																										WM(tmp_addr >> 1 , tmp);
																								} else {
																										// Extend address, preserve LSB & form byte address
																										tmp_addr = (add_mae(AEC_CASE_C , tmp_addr >> 1) << 1) | (tmp_addr & 1);
																										m_program->write_byte(tmp_addr , (UINT8)tmp);
																								}
								} else {
												// Word
												WM(AEC_CASE_C , *ptr_reg , tmp);
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

								m_pa_changed_func((UINT8)CURRENT_PA);

				// Is this correct? Patent @ pg 210 suggests that the whole interrupt recognition sequence
				// lasts for 32 cycles
				m_icount -= 32;

				// Do a double-indirect JSM IV,I instruction
				WM(AEC_CASE_C , ++m_reg_R , m_reg_P);
				m_reg_P = RM(AEC_CASE_I , RM(HP_REG_IV_ADDR));
				m_reg_I = fetch();
}

void hp_hybrid_cpu_device::handle_dma(void)
{
				// Patent hints at the fact that terminal count is detected by bit 15 of dmac being 1 after decrementing
				bool tc = BIT(--m_dmac , 15) != 0;
				UINT16 tmp;

				if (BIT(m_flags , HPHYBRID_DMADIR_BIT)) {
								// "Outward" DMA: memory -> peripheral
																tmp = RM(AEC_CASE_D , m_dmama++);
								WIO(m_dmapa , tc ? 2 : 0 , tmp);
								m_icount -= 10;
				} else {
								// "Inward" DMA: peripheral -> memory
								tmp = RIO(m_dmapa , tc ? 2 : 0);
								WM(AEC_CASE_D , m_dmama++ , tmp);
								m_icount -= 9;
				}

				// This is the one of the biggest question marks: is the DMA automatically disabled on TC?
				// Here we assume it is. After all it would make no difference because there is no way
				// to read the DMA enable flag back, so each time the DMA is needed it has to be enabled again.
				if (tc) {
								BIT_CLR(m_flags , HPHYBRID_DMAEN_BIT);
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

hp_5061_3001_cpu_device::hp_5061_3001_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: hp_hybrid_cpu_device(mconfig, HP_5061_3001, "HP-5061-3001", tag, owner, clock, "5061-3001", 22),
			m_boot_mode(false)
{
}

void hp_5061_3001_cpu_device::device_start()
{
		hp_hybrid_cpu_device::device_start();

		state_add(HPHYBRID_AR2, "Ar2" , m_reg_ar2[ 0 ]);
		state_add(HPHYBRID_AR2_2, "Ar2_2" , m_reg_ar2[ 1 ]);
		state_add(HPHYBRID_AR2_3, "Ar2_3" , m_reg_ar2[ 2 ]);
		state_add(HPHYBRID_AR2_4, "Ar2_4" , m_reg_ar2[ 3 ]);
		state_add(HPHYBRID_SE, "SE" , m_reg_se);
		state_add(HPHYBRID_R25, "R25" , m_reg_r25).noshow();
		state_add(HPHYBRID_R26, "R26" , m_reg_r26).noshow();
		state_add(HPHYBRID_R27, "R27" , m_reg_r27).noshow();
		state_add(HPHYBRID_R32, "R32" , m_reg_aec[ 0 ]);
		state_add(HPHYBRID_R33, "R33" , m_reg_aec[ 1 ]);
		state_add(HPHYBRID_R34, "R34" , m_reg_aec[ 2 ]);
		state_add(HPHYBRID_R35, "R35" , m_reg_aec[ 3 ]);
		state_add(HPHYBRID_R36, "R36" , m_reg_aec[ 4 ]);
		state_add(HPHYBRID_R37, "R37" , m_reg_aec[ 5 ]);

		save_item(NAME(m_reg_ar2[ 0 ]));
		save_item(NAME(m_reg_ar2[ 1 ]));
		save_item(NAME(m_reg_ar2[ 2 ]));
		save_item(NAME(m_reg_ar2[ 3 ]));
		save_item(NAME(m_reg_se));
		save_item(NAME(m_reg_r25));
		save_item(NAME(m_reg_r26));
		save_item(NAME(m_reg_r27));
		save_item(NAME(m_reg_aec[ 0 ]));
		save_item(NAME(m_reg_aec[ 1 ]));
		save_item(NAME(m_reg_aec[ 2 ]));
		save_item(NAME(m_reg_aec[ 3 ]));
		save_item(NAME(m_reg_aec[ 4 ]));
		save_item(NAME(m_reg_aec[ 5 ]));
}

void hp_5061_3001_cpu_device::device_reset()
{
		// Initial state of AEC registers:
		// R32  0
		// R33  5
		// R34  0
		// R35  0
		// R36  0
		// R37  0
		m_reg_aec[ 0 ] = 0;
		m_reg_aec[ 1 ] = 5;
		m_reg_aec[ 2 ] = 0;
		m_reg_aec[ 3 ] = 0;
		m_reg_aec[ 4 ] = 0;
		m_reg_aec[ 5 ] = 0;

		m_forced_bsc_25 = m_boot_mode;

		hp_hybrid_cpu_device::device_reset();
}

UINT8 hp_5061_3001_cpu_device::do_dec_shift_r(UINT8 d1 , UINT64& mantissa)
{
		UINT8 d12 = (UINT8)(mantissa & 0xf);

		mantissa = (mantissa >> 4) | ((UINT64)d1 << 44);

		return d12;
}

UINT8 hp_5061_3001_cpu_device::do_dec_shift_l(UINT8 d12 , UINT64& mantissa)
{
		UINT8 d1 = (UINT8)((mantissa >> 44) & 0xf);

		mantissa = (mantissa << 4) | ((UINT64)d12);
		mantissa &= 0xffffffffffffULL;

		return d1;
}

UINT64 hp_5061_3001_cpu_device::get_ar1(void)
{
		UINT32 addr;
		UINT64 tmp;

		addr = add_mae(AEC_CASE_B , HP_REG_AR1_ADDR + 1);
		tmp = (UINT64)RM(addr++);
		tmp <<= 16;
		tmp |= (UINT64)RM(addr++);
		tmp <<= 16;
		tmp |= (UINT64)RM(addr);

		return tmp;
}

void hp_5061_3001_cpu_device::set_ar1(UINT64 v)
{
		UINT32 addr;

		addr = add_mae(AEC_CASE_B , HP_REG_AR1_ADDR + 3);
		WM(addr-- , (UINT16)(v & 0xffff));
		v >>= 16;
		WM(addr-- , (UINT16)(v & 0xffff));
		v >>= 16;
		WM(addr , (UINT16)(v & 0xffff));
}

UINT64 hp_5061_3001_cpu_device::get_ar2(void) const
{
		UINT64 tmp;

		tmp = (UINT64)m_reg_ar2[ 1 ];
		tmp <<= 16;
		tmp |= (UINT64)m_reg_ar2[ 2 ];
		tmp <<= 16;
		tmp |= (UINT64)m_reg_ar2[ 3 ];

		return tmp;
}

void hp_5061_3001_cpu_device::set_ar2(UINT64 v)
{
		m_reg_ar2[ 3 ] = (UINT16)(v & 0xffff);
		v >>= 16;
		m_reg_ar2[ 2 ] = (UINT16)(v & 0xffff);
		v >>= 16;
		m_reg_ar2[ 1 ] = (UINT16)(v & 0xffff);
}

UINT64 hp_5061_3001_cpu_device::do_mrxy(UINT64 ar)
{
		UINT8 n;

		n = m_reg_B & 0xf;
		m_reg_A &= 0xf;
		m_reg_se = m_reg_A;
		while (n--) {
				m_reg_se = do_dec_shift_r(m_reg_A , ar);
				m_reg_A = 0;
				m_icount -= 4;
		}
		m_reg_A = m_reg_se;
		BIT_CLR(m_flags , HPHYBRID_DC_BIT);

		return ar;
}

bool hp_5061_3001_cpu_device::do_dec_add(bool carry_in , UINT64& a , UINT64 b)
{
	UINT64 tmp = 0;
	unsigned i;
	UINT8 digit_a , digit_b;

	for (i = 0; i < 12; i++) {
		digit_a = (UINT8)(a & 0xf);
		digit_b = (UINT8)(b & 0xf);

		if (carry_in) {
			digit_a++;
		}

		digit_a += digit_b;

		carry_in = digit_a >= 10;

		if (carry_in) {
			digit_a = (digit_a - 10) & 0xf;
		}

		tmp |= (UINT64)digit_a << (4 * i);

		a >>= 4;
		b >>= 4;
	}

	a = tmp;

	return carry_in;
}

void hp_5061_3001_cpu_device::do_mpy(void)
{
		INT32 a = (INT16)m_reg_A;
		INT32 b = (INT16)m_reg_B;
		INT32 p = a * b;

		m_reg_A = (UINT16)(p & 0xffff);
		m_reg_B = (UINT16)((p >> 16) & 0xffff);

		// Not entirely correct, timing depends on initial content of A register
		m_icount -= 65;
}

UINT16 hp_5061_3001_cpu_device::execute_no_bpc_ioc(UINT16 opcode)
{
		// EMC instructions
		UINT8 n;
		UINT16 tmp1;
		UINT16 tmp2;
		UINT64 tmp_ar;
		UINT64 tmp_ar2;
		bool carry;

		switch (opcode & 0xfff0) {
		case 0x7300:
				// XFR
				tmp1 = m_reg_A;
				tmp2 = m_reg_B;
				n = (opcode & 0xf) + 1;
				m_icount -= 21;
				while (n--) {
						m_icount -= 12;
						WM(AEC_CASE_C , tmp2 , RM(AEC_CASE_C , tmp1));
						tmp1++;
						tmp2++;
				}
				break;

		case 0x7380:
				// CLR
				tmp1 = m_reg_A;
				n = (opcode & 0xf) + 1;
				m_icount -= 16;
				while (n--) {
						m_icount -= 6;
						WM(AEC_CASE_C , tmp1 , 0);
						tmp1++;
				}
				break;

		default:
				switch (opcode) {
				case 0x7200:
						// MWA
						m_icount -= 28;
						tmp_ar2 = get_ar2();
						carry = do_dec_add(BIT(m_flags , HPHYBRID_DC_BIT) , tmp_ar2 , m_reg_B);
						set_ar2(tmp_ar2);
						if (carry) {
								BIT_SET(m_flags, HPHYBRID_DC_BIT);
						} else {
								BIT_CLR(m_flags, HPHYBRID_DC_BIT);
						}
						break;

				case 0x7220:
						// CMY
						m_icount -= 23;
						tmp_ar2 = get_ar2();
						tmp_ar2 = 0x999999999999ULL - tmp_ar2;
						do_dec_add(true , tmp_ar2 , 0);
						set_ar2(tmp_ar2);
						BIT_CLR(m_flags , HPHYBRID_DC_BIT);
						break;

				case 0x7260:
						// CMX
						m_icount -= 59;
						tmp_ar = get_ar1();
						tmp_ar = 0x999999999999ULL - tmp_ar;
						do_dec_add(true , tmp_ar , 0);
						set_ar1(tmp_ar);
						BIT_CLR(m_flags , HPHYBRID_DC_BIT);
						break;

				case 0x7280:
						// FXA
						m_icount -= 40;
						tmp_ar2 = get_ar2();
						carry = do_dec_add(BIT(m_flags , HPHYBRID_DC_BIT) , tmp_ar2 , get_ar1());
						set_ar2(tmp_ar2);
						if (carry) {
								BIT_SET(m_flags, HPHYBRID_DC_BIT);
						} else {
								BIT_CLR(m_flags, HPHYBRID_DC_BIT);
						}
						break;

				case 0x7340:
						// NRM
						tmp_ar2 = get_ar2();
						m_icount -= 23;
						for (n = 0; n < 12 && (tmp_ar2 & 0xf00000000000ULL) == 0; n++) {
								do_dec_shift_l(0 , tmp_ar2);
								m_icount--;
						}
						m_reg_B = n;
						if (n < 12) {
								BIT_CLR(m_flags , HPHYBRID_DC_BIT);
								set_ar2(tmp_ar2);
						} else {
								BIT_SET(m_flags , HPHYBRID_DC_BIT);
								// When ar2 is 0, total time is 69 cycles
								// (salcazzo che cosa fa per altri 34 cicli)
								m_icount -= 34;
						}
						break;

				case 0x73c0:
						// CDC
						m_icount -= 11;
						BIT_CLR(m_flags , HPHYBRID_DC_BIT);
						break;

				case 0x7a00:
						// FMP
						m_icount -= 42;
						m_reg_A = 0;
						tmp_ar = get_ar1();
						tmp_ar2 = get_ar2();
						for (n = m_reg_B & 0xf; n > 0; n--) {
								m_icount -= 13;
								if (do_dec_add(BIT(m_flags , HPHYBRID_DC_BIT) , tmp_ar2 , tmp_ar)) {
										m_reg_A++;
								}
								BIT_CLR(m_flags , HPHYBRID_DC_BIT);
						}
						set_ar2(tmp_ar2);
						break;

				case 0x7a21:
						// FDV
						// No doc mentions any limit on the iterations done by this instruction.
						// Here we stop at 15 (after all there are only 4 bits in the loop counter). But is it correct?
						m_icount -= 37;
						m_reg_B = 0;
						tmp_ar = get_ar1();
						tmp_ar2 = get_ar2();
						while (m_reg_B < 15 && !do_dec_add(BIT(m_flags , HPHYBRID_DC_BIT) , tmp_ar2 , tmp_ar)) {
								m_icount -= 13;
								BIT_CLR(m_flags , HPHYBRID_DC_BIT);
								m_reg_B++;
						}
						set_ar2(tmp_ar2);
						break;

				case 0x7b00:
						// MRX
						set_ar1(do_mrxy(get_ar1()));
						m_icount -= 62;
						break;

				case 0x7b21:
						// DRS
						tmp_ar = get_ar1();
						m_icount -= 56;
						m_reg_A = m_reg_se = do_dec_shift_r(0 , tmp_ar);
						set_ar1(tmp_ar);
						BIT_CLR(m_flags , HPHYBRID_DC_BIT);
						break;

				case 0x7b40:
						// MRY
						set_ar2(do_mrxy(get_ar2()));
						m_icount -= 33;
						break;

				case 0x7b61:
						// MLY
						tmp_ar2 = get_ar2();
						m_icount -= 32;
						m_reg_A = m_reg_se = do_dec_shift_l(m_reg_A & 0xf , tmp_ar2);
						set_ar2(tmp_ar2);
						BIT_CLR(m_flags , HPHYBRID_DC_BIT);
						break;

				case 0x7b8f:
						// MPY
						do_mpy();
						break;

				default:
						if ((opcode & 0xfec0) == 0x74c0) {
								// SDS
								// SDC
								m_icount -= 14;
								return get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_DC_BIT));
						} else {
								// Unrecognized instructions: NOP
								// Execution time is fictional
								logerror("hp-5061-3001: unknown opcode %04x @ %06x\n" , opcode , m_genpc);
								m_icount -= 6;
						}
						break;
				}
		}

		return m_reg_P + 1;
}

offs_t hp_5061_3001_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
		extern CPU_DISASSEMBLE(hp_5061_3001);
		return CPU_DISASSEMBLE_NAME(hp_5061_3001)(this, buffer, pc, oprom, opram, options);
}

UINT32 hp_5061_3001_cpu_device::add_mae(aec_cases_t aec_case , UINT16 addr)
{
		UINT16 bsc_reg;
		bool top_half = BIT(addr , 15) != 0;

		// Detect accesses to top half of base page
		if (aec_case == AEC_CASE_C && (addr & 0xfe00) == 0xfe00) {
			aec_case = AEC_CASE_B;
		}

		switch (aec_case) {
		case AEC_CASE_A:
				bsc_reg = top_half ? HP_REG_R34_ADDR : HP_REG_R33_ADDR;
				break;

		case AEC_CASE_B:
				bsc_reg = top_half ? HP_REG_R36_ADDR : HP_REG_R33_ADDR;
				break;

		case AEC_CASE_C:
				bsc_reg = top_half ? HP_REG_R32_ADDR : HP_REG_R35_ADDR;
				break;

		case AEC_CASE_D:
				bsc_reg = HP_REG_R37_ADDR;
				break;

                case AEC_CASE_I:
                                // Behaviour of AEC during interrupt vector fetch is undocumented but it can be guessed from 9845B firmware.
                                // Basically in this case the integrated AEC seems to do what the discrete implementation in 9845A does:
                                // top half of memory is mapped to block 0 (fixed) and bottom half is mapped according to content of R35
                                // (see pg 334 of patent).
                                bsc_reg = top_half ? 0 : HP_REG_R35_ADDR;
                                break;

                default:
                                logerror("hphybrid: aec_case=%d\n" , aec_case);
                                return 0;
                }

                UINT16 aec_reg = (bsc_reg != 0) ? (m_reg_aec[ bsc_reg - HP_REG_R32_ADDR ] & BSC_REG_MASK) : 0;

		if (m_forced_bsc_25) {
				aec_reg = (aec_reg & 0xf) | 0x20;
		}

		return (UINT32)addr | ((UINT32)aec_reg << 16);
}

UINT16 hp_5061_3001_cpu_device::read_non_common_reg(UINT16 addr)
{
		switch (addr) {
		case HP_REG_AR2_ADDR:
		case HP_REG_AR2_ADDR + 1:
		case HP_REG_AR2_ADDR + 2:
		case HP_REG_AR2_ADDR + 3:
				return m_reg_ar2[ addr - HP_REG_AR2_ADDR ];

		case HP_REG_SE_ADDR:
				return m_reg_se;

		case HP_REG_R25_ADDR:
				return m_reg_r25;

		case HP_REG_R26_ADDR:
				return m_reg_r26;

		case HP_REG_R27_ADDR:
				return m_reg_r27;

		case HP_REG_R32_ADDR:
		case HP_REG_R33_ADDR:
		case HP_REG_R34_ADDR:
		case HP_REG_R35_ADDR:
		case HP_REG_R36_ADDR:
		case HP_REG_R37_ADDR:
				return m_reg_aec[ addr - HP_REG_R32_ADDR ];

		default:
				return 0;
		}
}

void hp_5061_3001_cpu_device::write_non_common_reg(UINT16 addr , UINT16 v)
{
		switch (addr) {
		case HP_REG_AR2_ADDR:
		case HP_REG_AR2_ADDR + 1:
		case HP_REG_AR2_ADDR + 2:
		case HP_REG_AR2_ADDR + 3:
				m_reg_ar2[ addr - HP_REG_AR2_ADDR ] = v;
				break;

		case HP_REG_SE_ADDR:
				m_reg_se = v & HP_REG_SE_MASK;
				break;

		case HP_REG_R25_ADDR:
				m_reg_r25 = v;
				break;

		case HP_REG_R26_ADDR:
				m_reg_r26 = v;
				break;

		case HP_REG_R27_ADDR:
				m_reg_r27 = v;
				break;

		case HP_REG_R32_ADDR:
		case HP_REG_R33_ADDR:
		case HP_REG_R34_ADDR:
		case HP_REG_R35_ADDR:
		case HP_REG_R36_ADDR:
		case HP_REG_R37_ADDR:
				m_reg_aec[ addr - HP_REG_R32_ADDR ] = v;
				break;

		default:
				break;
		}
}

hp_5061_3011_cpu_device::hp_5061_3011_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: hp_hybrid_cpu_device(mconfig, HP_5061_3011, "HP-5061-3011", tag, owner, clock, "5061-3011", 16)
{
}

UINT16 hp_5061_3011_cpu_device::execute_no_bpc_ioc(UINT16 opcode)
{
		// Unrecognized instructions: NOP
		// Execution time is fictional
		m_icount -= 6;

		return m_reg_P + 1;
}

UINT32 hp_5061_3011_cpu_device::add_mae(aec_cases_t aec_case , UINT16 addr)
{
		// No MAE on 3011
		return addr;
}

UINT16 hp_5061_3011_cpu_device::read_non_common_reg(UINT16 addr)
{
		// Non-existing registers are returned as 0
		return 0;
}

void hp_5061_3011_cpu_device::write_non_common_reg(UINT16 addr , UINT16 v)
{
		// Non-existing registers are silently discarded
}
