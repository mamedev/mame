// license:BSD-3-Clause
// copyright-holders:F. Ulivi

// I found 2 undocumented instructions in 5061-3001. First I noticed that PPU processor in
// hp9845b emulator executed 2 unknown instructions at each keyboard interrupt whose opcodes
// were 0x7026 & 0x7027.
// I searched for a while for any kind of documentation about them but found nothing at all.
// Some time later I found the mnemonics in the binary dump of assembly development option ROM:
// CIM & SIM, respectively. From the mnemonic I deduced their function: Clear & Set Interrupt Mode.
// After a few experiments, crashes, etc. here's my opinion on their purpose.
// When the CPU receives an interrupt, its AEC registers can be in any state so it could
// be impossible to properly save state, fetch the interrupt vector and start executing the ISR.
// The solution is having an hidden "interrupt mode" flag that gets set when an interrupt (either
// low or high priority) is acknowledged and is cleared when the "ret 0,p" instruction that ends
// the ISR is executed. The effects of having the interrupt mode set are:
// * No interrupts are recognized
// * A few essential AEC registers are overridden to establish a "safe" environment to save state
// and execute ISR (see hp_5061_3001_cpu_device::add_mae).
// Inside the ISR, CIM & SIM instructions can be used to change the interrupt mode and switch
// between normal & overridden settings of AEC.
// As an example of CIM&SIM usage, we can have a look at the keyboard ISR in 9845B PPU processor:
// * A key is pressed and IRQ 0 is set
// * Interrupt 0 is recognized, IM is set
// * R register is used to save program counter in block = 1 (overriding any R36 value)
// * Vector is fetched and execution begins in block 5 (overriding R33 value)
// * Registers are saved to RAM (again in overridden block 1)
// * AEC registers are set to correct value for ISR execution
// * CIM is used to exit the special behaviour of AEC and to allow high-priority interrupts
// * Useful ISR processing is done
// * SIM is used to re-enter special behaviour of AEC and to block any interrupt
// * State is restored (including all AEC registers)
// * RET 0,P is executed to end ISR: return program counter is popped off the stack and IM is cleared

#include "emu.h"
#include "hphybrid.h"
#include "hphybrid_dasm.h"
#include "debugger.h"

#include "hphybrid_defs.h"


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

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Bits in m_flags
enum : unsigned {
	HPHYBRID_C_BIT       = 0,   // Carry/extend
	HPHYBRID_O_BIT       = 1,   // Overflow
	HPHYBRID_CB_BIT      = 2,   // Cb
	HPHYBRID_DB_BIT      = 3,   // Db
	HPHYBRID_INTEN_BIT   = 4,   // Interrupt enable
	HPHYBRID_DMAEN_BIT   = 5,   // DMA enable
	HPHYBRID_DMADIR_BIT  = 6,   // DMA direction (1 = OUT)
	HPHYBRID_HALT_BIT    = 7,   // Halt flag
	HPHYBRID_IRH_BIT     = 8,   // IRH requested
	HPHYBRID_IRL_BIT     = 9,   // IRL requested
	HPHYBRID_IRH_SVC_BIT = 10,  // IRH in service
	HPHYBRID_IRL_SVC_BIT = 11,  // IRL in service
	HPHYBRID_DMAR_BIT    = 12,  // DMA request
	HPHYBRID_STS_BIT     = 13,  // Status flag
	HPHYBRID_FLG_BIT     = 14,  // "Flag" flag
	HPHYBRID_DC_BIT      = 15,  // Decimal carry
	HPHYBRID_IM_BIT      = 16   // Interrupt mode
};

constexpr uint16_t HP_REG_IV_MASK = 0xfff0; // IV mask
constexpr uint16_t HP_REG_PA_MASK = 0x000f; // PA mask
constexpr uint16_t HP_REG_SE_MASK = 0x000f; // SE mask

#define CURRENT_PA      (m_reg_PA[ 0 ])

constexpr uint16_t HP_RESET_ADDR    = 0x0020;

// Part of r32-r37 that is actually output as address extension (6 bits of "BSC": block select code)
constexpr uint16_t BSC_REG_MASK = 0x3f;

// Address mask of 15-bit processor
constexpr uint16_t ADDR_MASK_15BIT = 0x7fff;

// Mask of MSB of registers
constexpr uint16_t REG_MSB_MASK = BIT_MASK<uint16_t>(15);

// Memory, I/O & register access timings
constexpr unsigned DEF_MEM_R_CYCLES = 4;    // Default memory read cycles
constexpr unsigned DEF_MEM_W_CYCLES = 4;    // Default memory write cycles
constexpr unsigned REGISTER_RW_CYCLES = 5;  // Internal register R/W cycles
constexpr unsigned IO_RW_CYCLES = 7;        // I/O R/W cycles

DEFINE_DEVICE_TYPE(HP_5061_3001, hp_5061_3001_cpu_device, "5061_3001", "Hewlett-Packard HP-5061-3001")
DEFINE_DEVICE_TYPE(HP_5061_3011, hp_5061_3011_cpu_device, "5061_3011", "Hewlett-Packard HP-5061-3011")
DEFINE_DEVICE_TYPE(HP_09825_67907, hp_09825_67907_cpu_device, "09825_67907", "Hewlett-Packard HP-09825-67907")

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::dmar_w)
{
	if (state)
		BIT_SET(m_flags, HPHYBRID_DMAR_BIT);
	else
		BIT_CLR(m_flags, HPHYBRID_DMAR_BIT);
}

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::halt_w)
{
	if (state)
		BIT_SET(m_flags, HPHYBRID_HALT_BIT);
	else
		BIT_CLR(m_flags, HPHYBRID_HALT_BIT);
}

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::status_w)
{
	if (state)
		BIT_SET(m_flags, HPHYBRID_STS_BIT);
	else
		BIT_CLR(m_flags, HPHYBRID_STS_BIT);
}

WRITE_LINE_MEMBER(hp_hybrid_cpu_device::flag_w)
{
	if (state)
		BIT_SET(m_flags, HPHYBRID_FLG_BIT);
	else
		BIT_CLR(m_flags, HPHYBRID_FLG_BIT);
}

hp_hybrid_cpu_device::hp_hybrid_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t addrwidth)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_pa_changed_func(*this)
	, m_opcode_func(*this)
	, m_stm_func(*this)
	, m_int_func(*this)
	, m_addr_mask((1U << addrwidth) - 1)
	, m_relative_mode(true)
	, m_r_cycles(DEF_MEM_R_CYCLES)
	, m_w_cycles(DEF_MEM_W_CYCLES)
	, m_boot_mode(false)
	, m_program_config("program", ENDIANNESS_BIG, 16, addrwidth, -1)
	, m_io_config("io", ENDIANNESS_BIG, 16, 6, -1)
{
	m_addr_mask_low16 = uint16_t(m_addr_mask & 0xffff);
}

device_memory_interface::space_config_vector hp_hybrid_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

void hp_hybrid_cpu_device::device_start()
{
	{
		state_add(HPHYBRID_A,      "A",        m_reg_A);
		state_add(HPHYBRID_B,      "B",        m_reg_B);
		state_add(HPHYBRID_C,      "C",        m_reg_C);
		state_add(HPHYBRID_D,      "D",        m_reg_D);
		state_add(HPHYBRID_P,      "P",        m_reg_P);
		state_add(STATE_GENPC,     "GENPC",    m_genpc).noshow();
		state_add(STATE_GENPCBASE, "CURPC",    m_genpc).noshow();
		state_add(HPHYBRID_R,      "R",        m_reg_R);
		state_add(HPHYBRID_IV,     "IV",       m_reg_IV);
		state_add(HPHYBRID_PA,     "PA",       m_reg_PA[ 0 ]);
		state_add(HPHYBRID_W,      "W",        m_reg_W).noshow();
		state_add(STATE_GENFLAGS,  "GENFLAGS", m_flags).noshow().formatstr("%12s");
		state_add(HPHYBRID_DMAPA,  "DMAPA",    m_dmapa).noshow();
		state_add(HPHYBRID_DMAMA,  "DMAMA",    m_dmama).noshow();
		state_add(HPHYBRID_DMAC,   "DMAC",     m_dmac).noshow();
		state_add(HPHYBRID_I,      "I",        m_reg_I).noshow();
	}

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_IO).specific(m_io);

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

	set_icountptr(m_icount);

	m_pa_changed_func.resolve_safe();
	m_opcode_func.resolve();
	m_stm_func.resolve();
	m_int_func.resolve();
}

void hp_hybrid_cpu_device::device_reset()
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
	m_curr_cycle = 0;
	m_forced_bsc_25 = m_boot_mode;

	m_last_pa = ~0;
	update_pa();

	m_reg_I = fetch();
}

void hp_hybrid_cpu_device::execute_run()
{
	do {
		if (BIT(m_flags , HPHYBRID_DMAEN_BIT) && BIT(m_flags , HPHYBRID_DMAR_BIT)) {
			handle_dma();
		} else {
			debugger_instruction_hook(m_genpc);

			m_reg_I = execute_one(m_reg_I);

			// Check for interrupts
			check_for_interrupts();
		}
	} while (m_icount > 0);
}

void hp_hybrid_cpu_device::execute_set_input(int inputnum, int state)
{
	if (inputnum < HPHYBRID_INT_LVLS) {
		if (state)
			BIT_SET(m_flags , HPHYBRID_IRH_BIT + inputnum);
		else
			BIT_CLR(m_flags , HPHYBRID_IRH_BIT + inputnum);
	}
}

/**
 * Execute 1 instruction
 *
 * @param opcode Opcode to be executed
 *
 * @return Next opcode to be executed
 */
uint16_t hp_hybrid_cpu_device::execute_one(uint16_t opcode)
{
	if ((opcode & 0x7fe0) == 0x7000) {
		// EXE
		m_icount -= 2;
		uint16_t fetch_addr = opcode & 0x1f;
		if (BIT(opcode , 15)) {
			fetch_addr = get_indirect_target(fetch_addr);
		}
		// Indirect addressing in EXE instruction seems to use AEC case A instead of case C
		// (because it's an opcode fetch)
		return fetch_at(add_mae(AEC_CASE_A , fetch_addr));
	} else {
		uint16_t next_P;
		if (!execute_one_bpc(opcode , next_P) &&
			!execute_no_bpc(opcode , next_P)) {
			// Unrecognized instruction, make it a NOP
			logerror("hp_hybrid: unknown opcode %04x @ %06x\n" , opcode , m_genpc);
			next_P = m_reg_P + 1;
		}
		m_reg_P = next_P & m_addr_mask_low16;
		return fetch();
	}
}

/**
 * Execute 1 BPC instruction (except EXE)
 *
 * @param opcode Opcode to be executed (no EXE instructions)
 * @param[out] next_pc new value of P register
 *
 * @return true iff instruction executed
 */
bool hp_hybrid_cpu_device::execute_one_bpc(uint16_t opcode , uint16_t& next_pc)
{
	uint32_t ea;
	uint16_t tmp;

	switch (opcode & 0x7800) {
	case 0x0000:
		// LDA
		m_icount -= 1;
		m_reg_A = RM(get_ea(opcode));
		break;

	case 0x0800:
		// LDB
		m_icount -= 1;
		m_reg_B = RM(get_ea(opcode));
		break;

	case 0x1000:
		// CPA
		m_icount -= 4;
		if (m_reg_A != RM(get_ea(opcode))) {
			// Skip next instruction
			next_pc = m_reg_P + 2;
			return true;
		}
		break;

	case 0x1800:
		// CPB
		m_icount -= 4;
		if (m_reg_B != RM(get_ea(opcode))) {
			// Skip next instruction
			next_pc = m_reg_P + 2;
			return true;
		}
		break;

	case 0x2000:
		// ADA
		m_icount -= 1;
		do_add(m_reg_A , RM(get_ea(opcode)));
		break;

	case 0x2800:
		// ADB
		m_icount -= 1;
		do_add(m_reg_B , RM(get_ea(opcode)));
		break;

	case 0x3000:
		// STA
		m_icount -= 1;
		WM(get_ea(opcode) , m_reg_A);
		break;

	case 0x3800:
		// STB
		m_icount -= 1;
		WM(get_ea(opcode) , m_reg_B);
		break;

	case 0x4000:
		// JSM
		m_icount -= 5;
		next_pc = remove_mae(get_ea(opcode));
		m_reg_R = (m_reg_R + 1) & m_addr_mask_low16;
		WM(AEC_CASE_C , m_reg_R , m_reg_P);
		return true;

	case 0x4800:
		// ISZ
		m_icount -= 1;
		ea = get_ea(opcode);
		tmp = RM(ea) + 1;
		WM(ea , tmp);
		if (tmp == 0) {
			// Skip next instruction
			next_pc = m_reg_P + 2;
			return true;
		}
		break;

	case 0x5000:
		// AND
		m_icount -= 1;
		m_reg_A &= RM(get_ea(opcode));
		break;

	case 0x5800:
		// DSZ
		m_icount -= 1;
		ea = get_ea(opcode);
		tmp = RM(ea) - 1;
		WM(ea , tmp);
		if (tmp == 0) {
			// Skip next instruction
			next_pc = m_reg_P + 2;
			return true;
		}
		break;

	case 0x6000:
		// IOR
		m_icount -= 1;
		m_reg_A |= RM(get_ea(opcode));
		break;

	case 0x6800:
		// JMP
		m_icount -= 2;
		next_pc = remove_mae(get_ea(opcode));
		return true;

	default:
		switch (opcode & 0xfec0) {
		case 0x7400:
			// RZA
			// SZA
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , m_reg_A == 0);
			return true;

		case 0x7440:
			// RIA
			// SIA
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , m_reg_A++ == 0);
			return true;

		case 0x7480:
			// SFS
			// SFC
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_FLG_BIT));
			return true;

		case 0x74c0:
			// SDS
			// SDC
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_DC_BIT));
			return true;

		case 0x7C00:
			// RZB
			// SZB
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , m_reg_B == 0);
			return true;

		case 0x7C40:
			// RIB
			// SIB
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , m_reg_B++ == 0);
			return true;

		case 0x7c80:
			// SSS
			// SSC
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_STS_BIT));
			return true;

		case 0x7cc0:
			// SHS
			// SHC
			m_icount -= 8;
			next_pc = get_skip_addr(opcode , !BIT(m_flags , HPHYBRID_HALT_BIT));
			return true;

		default:
			switch (opcode & 0xfe00) {
			case 0x7600:
				// SLA
				// RLA
				m_icount -= 8;
				next_pc = get_skip_addr_sc(opcode , m_reg_A , 0);
				return true;

			case 0x7e00:
				// SLB
				// RLB
				m_icount -= 8;
				next_pc = get_skip_addr_sc(opcode , m_reg_B , 0);
				return true;

			case 0xf400:
				// SAP
				// SAM
				m_icount -= 8;
				next_pc = get_skip_addr_sc(opcode , m_reg_A , 15);
				return true;

			case 0xf600:
				// SOC
				// SOS
				m_icount -= 8;
				next_pc = get_skip_addr_sc(opcode , m_flags , HPHYBRID_O_BIT);
				return true;

			case 0xfc00:
				// SBP
				// SBM
				m_icount -= 8;
				next_pc = get_skip_addr_sc(opcode , m_reg_B , 15);
				return true;

			case 0xfe00:
				// SEC
				// SES
				m_icount -= 8;
				next_pc = get_skip_addr_sc(opcode , m_flags , HPHYBRID_C_BIT);
				return true;

			default:
				switch (opcode & 0xfff0) {
				case 0xf100:
					// AAR
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					// A shift by 16 positions is equivalent to a shift by 15
					tmp = tmp > 15 ? 15 : tmp;
					m_reg_A = ((m_reg_A ^ 0x8000) >> tmp) - (0x8000 >> tmp);
					break;

				case 0xf900:
					// ABR
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					tmp = tmp > 15 ? 15 : tmp;
					m_reg_B = ((m_reg_B ^ 0x8000) >> tmp) - (0x8000 >> tmp);
					break;

				case 0xf140:
					// SAR
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					m_reg_A >>= tmp;
					break;

				case 0xf940:
					// SBR
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					m_reg_B >>= tmp;
					break;

				case 0xf180:
					// SAL
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					m_reg_A <<= tmp;
					break;

				case 0xf980:
					// SBL
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					m_reg_B <<= tmp;
					break;

				case 0xf1c0:
					// RAR
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					m_reg_A = (m_reg_A >> tmp) | (m_reg_A << (16 - tmp));
					break;

				case 0xf9c0:
					// RBR
					tmp = (opcode & 0xf) + 1;
					m_icount -= (3 + tmp);
					m_reg_B = (m_reg_B >> tmp) | (m_reg_B << (16 - tmp));
					break;

				default:
					if ((opcode & 0xff80) == 0xf080) {
						// RET
						m_icount -= 4;
						if (BIT(opcode , 6)) {
							// Pop PA stack
							if (BIT(m_flags , HPHYBRID_IRH_SVC_BIT)) {
								BIT_CLR(m_flags , HPHYBRID_IRH_SVC_BIT);
								memmove(&m_reg_PA[ 0 ] , &m_reg_PA[ 1 ] , HPHYBRID_INT_LVLS);
								update_pa();
							} else if (BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
								BIT_CLR(m_flags , HPHYBRID_IRL_SVC_BIT);
								memmove(&m_reg_PA[ 0 ] , &m_reg_PA[ 1 ] , HPHYBRID_INT_LVLS);
								update_pa();
							}
						}
						tmp = RM(AEC_CASE_C , m_reg_R) + (opcode & 0x1f);
						m_reg_R = (m_reg_R - 1) & m_addr_mask_low16;
						if (BIT(opcode , 6)) {
							BIT_CLR(m_flags, HPHYBRID_IM_BIT);
						}
						next_pc = BIT(opcode , 5) ? tmp - 0x20 : tmp;
						return true;
					} else {
						switch (opcode) {
						case 0x7110:
							// EIR
							m_icount -= 6;
							BIT_SET(m_flags , HPHYBRID_INTEN_BIT);
							break;

						case 0x7118:
							// DIR
							m_icount -= 6;
							BIT_CLR(m_flags , HPHYBRID_INTEN_BIT);
							break;

						case 0x7120:
							// DMA
							m_icount -= 6;
							BIT_SET(m_flags , HPHYBRID_DMAEN_BIT);
							break;

						case 0x7138:
							// DDR
							m_icount -= 6;
							BIT_CLR(m_flags , HPHYBRID_DMAEN_BIT);
							break;

						case 0xf020:
							// TCA
							m_icount -= 3;
							m_reg_A = ~m_reg_A;
							do_add(m_reg_A , 1);
							break;

						case 0xf060:
							// CMA
							m_icount -= 3;
							m_reg_A = ~m_reg_A;
							break;

						case 0xf820:
							// TCB
							m_icount -= 3;
							m_reg_B = ~m_reg_B;
							do_add(m_reg_B , 1);
							break;

						case 0xf860:
							// CMB
							m_icount -= 3;
							m_reg_B = ~m_reg_B;
							break;

						default:
							// Unrecognized instruction: pass it on for further processing by other units
							return false;
						}
					}
				}
			}
		}
	}

	next_pc = m_reg_P + 1;
	return true;
}

void hp_hybrid_cpu_device::emc_start()
{
	state_add(HPHYBRID_AR2, "Ar2" , m_reg_ar2[ 0 ]);
	state_add(HPHYBRID_AR2_2, "Ar2_2" , m_reg_ar2[ 1 ]);
	state_add(HPHYBRID_AR2_3, "Ar2_3" , m_reg_ar2[ 2 ]);
	state_add(HPHYBRID_AR2_4, "Ar2_4" , m_reg_ar2[ 3 ]);
	state_add(HPHYBRID_SE, "SE" , m_reg_se);
	state_add(HPHYBRID_R25, "R25" , m_reg_r25).noshow();
	state_add(HPHYBRID_R26, "R26" , m_reg_r26).noshow();
	state_add(HPHYBRID_R27, "R27" , m_reg_r27).noshow();

	save_item(NAME(m_reg_ar2));
	save_item(NAME(m_reg_se));
	save_item(NAME(m_reg_r25));
	save_item(NAME(m_reg_r26));
	save_item(NAME(m_reg_r27));
}

bool hp_hybrid_cpu_device::execute_emc(uint16_t opcode , uint16_t& next_pc)
{
	// EMC instructions
	uint8_t n;
	uint16_t tmp1;
	uint16_t tmp2;
	uint64_t tmp_ar;
	uint64_t tmp_ar2;
	bool carry;

	switch (opcode & 0xfff0) {
	case 0x7300:
		// XFR
		tmp1 = m_reg_A;
		tmp2 = m_reg_B;
		n = (opcode & 0xf) + 1;
		m_icount -= 15;
		while (n--) {
			WM(AEC_CASE_C , tmp2 , RM(AEC_CASE_C , tmp1));
			tmp1++;
			tmp2++;
		}
		break;

	case 0x7380:
		// CLR
		tmp1 = m_reg_A;
		n = (opcode & 0xf) + 1;
		m_icount -= 10;
		while (n--) {
			WM(AEC_CASE_C , tmp1 , 0);
			tmp1++;
		}
		break;

	default:
		switch (opcode) {
		case 0x7200:
			// MWA
			m_icount -= 22;
			tmp_ar2 = get_ar2();
			carry = do_dec_add(BIT(m_flags , HPHYBRID_DC_BIT) , tmp_ar2 , m_reg_B);
			set_ar2(tmp_ar2);
			if (carry)
				BIT_SET(m_flags, HPHYBRID_DC_BIT);
			else
				BIT_CLR(m_flags, HPHYBRID_DC_BIT);
			break;

		case 0x7220:
			// CMY
			m_icount -= 17;
			tmp_ar2 = get_ar2();
			tmp_ar2 = 0x999999999999ULL - tmp_ar2;
			do_dec_add(true , tmp_ar2 , 0);
			set_ar2(tmp_ar2);
			BIT_CLR(m_flags , HPHYBRID_DC_BIT);
			break;

		case 0x7260:
			// CMX
			m_icount -= 17;
			tmp_ar = get_ar1();
			tmp_ar = 0x999999999999ULL - tmp_ar;
			do_dec_add(true , tmp_ar , 0);
			set_ar1(tmp_ar);
			BIT_CLR(m_flags , HPHYBRID_DC_BIT);
			break;

		case 0x7280:
			// FXA
			m_icount -= 16;
			tmp_ar2 = get_ar2();
			carry = do_dec_add(BIT(m_flags , HPHYBRID_DC_BIT) , tmp_ar2 , get_ar1());
			set_ar2(tmp_ar2);
			if (carry)
				BIT_SET(m_flags, HPHYBRID_DC_BIT);
			else
				BIT_CLR(m_flags, HPHYBRID_DC_BIT);
			break;

		case 0x7340:
			// NRM
			tmp_ar2 = get_ar2();
			m_icount -= 17;
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
			m_icount -= 5;
			BIT_CLR(m_flags , HPHYBRID_DC_BIT);
			break;

		case 0x7a00:
			// FMP
			m_icount -= 15;
			m_reg_A = 0;
			n = m_reg_B & 0xf;
			if (n == 0) {
				tmp_ar = 0;
			} else {
				m_icount -= 3;
				tmp_ar = get_ar1();
				n--;
			}
			tmp_ar2 = get_ar2();
			do {
				m_icount -= 13;
				if (do_dec_add(BIT(m_flags , HPHYBRID_DC_BIT) , tmp_ar2 , tmp_ar)) {
					m_reg_A++;
				}
				BIT_CLR(m_flags , HPHYBRID_DC_BIT);
			} while (n--);
			set_ar2(tmp_ar2);
			break;

		case 0x7a21:
			// FDV
			// This instruction keeps adding AR1 to AR2 until an overflow occurs.
			// Register B will hold the number of iterations after the execution.
			// Note that if AR1 is 0 overflow never happens and the processor hangs.
			// Here we stop at 15 iterations (after all there are only 4 bits in the loop counter).
			m_icount -= 13;
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
			// Cycle count is incorrect for the case where B=0, as AR1 doesn't get read or written in real hw
			set_ar1(do_mrxy(get_ar1()));
			m_icount -= 20;
			break;

		case 0x7b21:
			// DRS
			tmp_ar = get_ar1();
			m_icount -= 14;
			m_reg_A = m_reg_se = do_dec_shift_r(0 , tmp_ar);
			set_ar1(tmp_ar);
			BIT_CLR(m_flags , HPHYBRID_DC_BIT);
			break;

		case 0x7b40:
			// MRY
			set_ar2(do_mrxy(get_ar2()));
			m_icount -= 27;
			break;

		case 0x7b61:
			// MLY
			tmp_ar2 = get_ar2();
			m_icount -= 26;
			m_reg_A = m_reg_se = do_dec_shift_l(m_reg_A & 0xf , tmp_ar2);
			set_ar2(tmp_ar2);
			BIT_CLR(m_flags , HPHYBRID_DC_BIT);
			break;

		case 0x7b8f:
			// MPY
			do_mpy();
			break;

		default:
			// Unrecognized instruction
			return false;
		}
	}

	next_pc = m_reg_P + 1;
	return true;
}

void hp_hybrid_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() == STATE_GENFLAGS) {
		str = string_format("%s %s %s %c %c",
							BIT(m_flags , HPHYBRID_DB_BIT) ? "Db":"..",
							BIT(m_flags , HPHYBRID_CB_BIT) ? "Cb":"..",
							BIT(m_flags , HPHYBRID_DC_BIT) ? "DC":"..",
							BIT(m_flags , HPHYBRID_O_BIT) ? 'O':'.',
							BIT(m_flags , HPHYBRID_C_BIT) ? 'E':'.');
	}
}

uint32_t hp_hybrid_cpu_device::add_mae(aec_cases_t aec_case , uint16_t addr)
{
	// No MAE on 5061-3011 or 09825-67907
	return addr;
}

uint16_t hp_hybrid_cpu_device::remove_mae(uint32_t addr)
{
	return uint16_t(addr & 0xffff);
}

uint16_t hp_hybrid_cpu_device::RM(aec_cases_t aec_case , uint16_t addr)
{
	return RM(add_mae(aec_case , addr));
}

uint16_t hp_hybrid_cpu_device::RM(uint32_t addr)
{
	addr &= m_addr_mask;
	uint16_t addr_wo_bsc = remove_mae(addr);

	if (addr_wo_bsc <= HP_REG_LAST_ADDR) {
		// Any access to internal registers removes forcing of BSC 2x
		m_forced_bsc_25 = false;

		if (!m_stm_func.isnull()) {
			m_stm_func(m_curr_cycle | CYCLE_RAL_MASK | CYCLE_RD_MASK);
			m_curr_cycle = 0;
		}

		// Memory mapped BPC registers
		uint16_t tmp;
		switch (addr_wo_bsc) {
		case HP_REG_A_ADDR:
			tmp = m_reg_A;
			break;

		case HP_REG_B_ADDR:
			tmp = m_reg_B;
			break;

		case HP_REG_P_ADDR:
			tmp = m_reg_P;
			break;

		case HP_REG_R_ADDR:
			tmp = m_reg_R;
			break;

		case HP_REG_R4_ADDR:
		case HP_REG_R5_ADDR:
		case HP_REG_R6_ADDR:
		case HP_REG_R7_ADDR:
			return RIO(CURRENT_PA , addr_wo_bsc - HP_REG_R4_ADDR);

		case HP_REG_IV_ADDR:
			tmp = m_reg_IV;
			break;

		case HP_REG_PA_ADDR:
			tmp = CURRENT_PA;
			break;

		case HP_REG_W_ADDR:
			tmp = m_reg_W;
			break;

		case HP_REG_DMAMA_ADDR:
			tmp = m_dmama;
			break;

		case HP_REG_DMAC_ADDR:
			tmp = m_dmac;
			break;

		case HP_REG_C_ADDR:
			tmp = m_reg_C;
			break;

		case HP_REG_D_ADDR:
			tmp = m_reg_D;
			break;

		default:
			if (!read_non_common_reg(addr_wo_bsc , tmp)) {
				// Non-existing registers are returned as 0
				tmp = 0;
			}
			break;
		}
		m_icount -= REGISTER_RW_CYCLES;
		return tmp;
	} else {
		m_icount -= m_r_cycles;
		if (!m_stm_func.isnull()) {
			m_stm_func(m_curr_cycle | CYCLE_RD_MASK);
			m_curr_cycle = 0;
		}
		return m_cache.read_word(addr);
	}
}

bool hp_hybrid_cpu_device::read_emc_reg(uint16_t addr , uint16_t& v)
{
	switch (addr) {
	case HP_REG_AR2_ADDR:
	case HP_REG_AR2_ADDR + 1:
	case HP_REG_AR2_ADDR + 2:
	case HP_REG_AR2_ADDR + 3:
		v = m_reg_ar2[ addr - HP_REG_AR2_ADDR ];
		return true;

	case HP_REG_SE_ADDR:
		v = m_reg_se;
		return true;

	case HP_REG_R25_ADDR:
		v = m_reg_r25;
		return true;

	case HP_REG_R26_ADDR:
		v = m_reg_r26;
		return true;

	case HP_REG_R27_ADDR:
		v = m_reg_r27;
		return true;

	default:
		return false;
	}
}

void hp_hybrid_cpu_device::WM(aec_cases_t aec_case , uint16_t addr , uint16_t v)
{
	WM(add_mae(aec_case , addr) , v);
}

void hp_hybrid_cpu_device::WM(uint32_t addr , uint16_t v)
{
	addr &= m_addr_mask;
	uint16_t addr_wo_bsc = remove_mae(addr);

	if (addr_wo_bsc <= HP_REG_LAST_ADDR) {
		// Any access to internal registers removes forcing of BSC 2x
		m_forced_bsc_25 = false;

		if (!m_stm_func.isnull()) {
			m_stm_func(m_curr_cycle | CYCLE_RAL_MASK | CYCLE_WR_MASK);
			m_curr_cycle = 0;
		}

		// Memory mapped BPC registers
		switch (addr_wo_bsc) {
		case HP_REG_A_ADDR:
			m_reg_A = v;
			break;

		case HP_REG_B_ADDR:
			m_reg_B = v;
			break;

		case HP_REG_P_ADDR:
			m_reg_P = v & m_addr_mask_low16;
			break;

		case HP_REG_R_ADDR:
			m_reg_R = v & m_addr_mask_low16;
			break;

		case HP_REG_R4_ADDR:
		case HP_REG_R5_ADDR:
		case HP_REG_R6_ADDR:
		case HP_REG_R7_ADDR:
			WIO(CURRENT_PA , addr_wo_bsc - HP_REG_R4_ADDR , v);
			return;

		case HP_REG_IV_ADDR:
			m_reg_IV = v & HP_REG_IV_MASK;
			break;

		case HP_REG_PA_ADDR:
			CURRENT_PA = v & HP_REG_PA_MASK;
			update_pa();
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
		m_icount -= REGISTER_RW_CYCLES;
	} else {
		m_icount -= m_w_cycles;
		if (!m_stm_func.isnull()) {
			m_stm_func(m_curr_cycle | CYCLE_WR_MASK);
			m_curr_cycle = 0;
		}
		m_program.write_word(addr , v);
	}
}

bool hp_hybrid_cpu_device::write_emc_reg(uint16_t addr , uint16_t v)
{
	switch (addr) {
	case HP_REG_AR2_ADDR:
	case HP_REG_AR2_ADDR + 1:
	case HP_REG_AR2_ADDR + 2:
	case HP_REG_AR2_ADDR + 3:
		m_reg_ar2[ addr - HP_REG_AR2_ADDR ] = v;
		return true;

	case HP_REG_SE_ADDR:
		m_reg_se = v & HP_REG_SE_MASK;
		return true;

	case HP_REG_R25_ADDR:
		m_reg_r25 = v;
		return true;

	case HP_REG_R26_ADDR:
		m_reg_r26 = v;
		return true;

	case HP_REG_R27_ADDR:
		m_reg_r27 = v;
		return true;

	default:
		return false;
	}
}

uint16_t hp_hybrid_cpu_device::fetch()
{
	m_genpc = add_mae(AEC_CASE_A , m_reg_P);
	return fetch_at(m_genpc);
}

uint16_t hp_hybrid_cpu_device::fetch_at(uint32_t addr)
{
	m_curr_cycle |= CYCLE_IFETCH_MASK;
	uint16_t opcode = RM(addr);
	if (!m_opcode_func.isnull())
		m_opcode_func(opcode);
	return opcode;
}

uint16_t hp_hybrid_cpu_device::get_indirect_target(uint32_t addr)
{
	// Single-level indirect addressing on 5061-3011 or 5061-3001
	return RM(addr);
}

uint32_t hp_hybrid_cpu_device::get_ea(uint16_t opcode)
{
	uint16_t base;
	uint16_t off;
	aec_cases_t aec;

	if (BIT(opcode , 10)) {
		if (m_relative_mode) {
			// Current page relative addressing
			base = m_reg_P;
		} else {
			// Current page absolute addressing
			base = (m_reg_P & 0xfc00) | 0x0200;
		}
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
	uint32_t ea = add_mae(aec , base);

	if (BIT(opcode , 15)) {
		// Indirect addressing
		return add_mae(AEC_CASE_C , get_indirect_target(ea));
	} else {
		// Direct addressing
		return ea;
	}
}

void hp_hybrid_cpu_device::do_add(uint16_t& addend1 , uint16_t addend2)
{
	uint32_t tmp = addend1 + addend2;

	if (BIT(tmp , 16)) {
		// Carry
		BIT_SET(m_flags , HPHYBRID_C_BIT);
	}

	if (BIT((tmp ^ addend1) & (tmp ^ addend2) , 15)) {
		// Overflow
		BIT_SET(m_flags , HPHYBRID_O_BIT);
	}

	addend1 = uint16_t(tmp);
}

uint16_t hp_hybrid_cpu_device::get_skip_addr(uint16_t opcode , bool condition) const
{
	bool skip_val = BIT(opcode , 8) != 0;

	if (condition == skip_val) {
		uint16_t off = opcode & 0x1f;

		if (BIT(opcode , 5)) {
			off -= 0x20;
		}
		return m_reg_P + off;
	} else {
		return m_reg_P + 1;
	}
}

template<typename T> uint16_t hp_hybrid_cpu_device::get_skip_addr_sc(uint16_t opcode , T& v , unsigned n)
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

void hp_hybrid_cpu_device::update_pa()
{
	if (CURRENT_PA != m_last_pa) {
		m_last_pa = CURRENT_PA;
		m_pa_changed_func(m_last_pa);
	}
}

void hp_hybrid_cpu_device::check_for_interrupts()
{
	if (!BIT(m_flags , HPHYBRID_INTEN_BIT) || BIT(m_flags , HPHYBRID_IRH_SVC_BIT) || BIT(m_flags , HPHYBRID_IM_BIT)) {
		return;
	}

	int irqline;

	if (BIT(m_flags , HPHYBRID_IRH_BIT)) {
		// Service high-level interrupt
		BIT_SET(m_flags , HPHYBRID_IRH_SVC_BIT);
		irqline = HPHYBRID_IRH;
		if (BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
			logerror("H pre-empted L @ %06x\n" , m_genpc);
		}
	} else if (BIT(m_flags , HPHYBRID_IRL_BIT) && !BIT(m_flags , HPHYBRID_IRL_SVC_BIT)) {
		// Service low-level interrupt
		BIT_SET(m_flags , HPHYBRID_IRL_SVC_BIT);
		irqline = HPHYBRID_IRL;
	} else {
		return;
	}

	standard_irq_callback(irqline);

	// Get interrupt vector in low byte (level is available on PA3)
	uint8_t vector = !m_int_func.isnull() ? m_int_func(BIT(m_flags , HPHYBRID_IRH_BIT) ? 1 : 0) : 0xff;
	uint8_t new_PA;

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
	update_pa();

	// Total time for int. ack execution = 12 + WM + RM * (1 + IND) + RR
	// WM = memory write cycles
	// RM = memory read cycles
	// IND = count of indirections (1 in 3001/3011)
	// RR = register read cycles
	m_icount -= (12 + REGISTER_RW_CYCLES);

	// Allow special processing in 5061-3001
	enter_isr();

	// Do a double-indirect JSM IV,I instruction
	// On 09825 there can be more than 2 levels of indirection
	m_reg_R = (m_reg_R + 1) & m_addr_mask_low16;
	WM(AEC_CASE_C , m_reg_R , m_reg_P);
	uint32_t addr = add_mae(AEC_CASE_C , m_reg_IV + CURRENT_PA);
	m_reg_P = get_indirect_target(addr);
	m_reg_I = fetch();
}

void hp_hybrid_cpu_device::enter_isr()
{
	// Do nothing special
}

uint16_t hp_hybrid_cpu_device::RIO(uint8_t pa , uint8_t ic)
{
	m_icount -= IO_RW_CYCLES;
	return m_io.read_word(HP_MAKE_IOADDR(pa, ic));
}

void hp_hybrid_cpu_device::WIO(uint8_t pa , uint8_t ic , uint16_t v)
{
	m_icount -= IO_RW_CYCLES;
	m_io.write_word(HP_MAKE_IOADDR(pa, ic) , v);
}

uint8_t hp_hybrid_cpu_device::do_dec_shift_r(uint8_t d1 , uint64_t& mantissa)
{
	uint8_t d12 = uint8_t(mantissa & 0xf);

	mantissa = (mantissa >> 4) | (uint64_t(d1) << 44);

	return d12;
}

uint8_t hp_hybrid_cpu_device::do_dec_shift_l(uint8_t d12 , uint64_t& mantissa)
{
	uint8_t d1 = uint8_t((mantissa >> 44) & 0xf);

	mantissa = (mantissa << 4) | uint64_t(d12);
	mantissa &= 0xffffffffffffULL;

	return d1;
}

uint64_t hp_hybrid_cpu_device::get_ar1()
{
	uint32_t addr;
	uint64_t tmp;

	addr = add_mae(AEC_CASE_B , HP_REG_AR1_ADDR + 1);
	tmp = uint64_t(RM(addr++));
	tmp <<= 16;
	tmp |= uint64_t(RM(addr++));
	tmp <<= 16;
	tmp |= uint64_t(RM(addr));

	return tmp;
}

void hp_hybrid_cpu_device::set_ar1(uint64_t v)
{
	uint32_t addr;

	addr = add_mae(AEC_CASE_B , HP_REG_AR1_ADDR + 3);
	WM(addr-- , uint16_t(v & 0xffff));
	v >>= 16;
	WM(addr-- , uint16_t(v & 0xffff));
	v >>= 16;
	WM(addr , uint16_t(v & 0xffff));
}

uint64_t hp_hybrid_cpu_device::get_ar2() const
{
	uint64_t tmp;

	tmp = uint64_t(m_reg_ar2[ 1 ]);
	tmp <<= 16;
	tmp |= uint64_t(m_reg_ar2[ 2 ]);
	tmp <<= 16;
	tmp |= uint64_t(m_reg_ar2[ 3 ]);

	return tmp;
}

void hp_hybrid_cpu_device::set_ar2(uint64_t v)
{
	m_reg_ar2[ 3 ] = uint16_t(v & 0xffff);
	v >>= 16;
	m_reg_ar2[ 2 ] = uint16_t(v & 0xffff);
	v >>= 16;
	m_reg_ar2[ 1 ] = uint16_t(v & 0xffff);
}

uint64_t hp_hybrid_cpu_device::do_mrxy(uint64_t ar)
{
	uint8_t n;

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

bool hp_hybrid_cpu_device::do_dec_add(bool carry_in , uint64_t& a , uint64_t b)
{
	uint64_t tmp = 0;
	unsigned i;
	uint8_t digit_a , digit_b;

	for (i = 0; i < 12; i++) {
		digit_a = uint8_t(a & 0xf);
		digit_b = uint8_t(b & 0xf);

		if (carry_in) {
			digit_a++;
		}

		digit_a += digit_b;

		carry_in = digit_a >= 10;

		if (carry_in) {
			digit_a = (digit_a - 10) & 0xf;
		}

		tmp |= uint64_t(digit_a) << (4 * i);

		a >>= 4;
		b >>= 4;
	}

	a = tmp;

	return carry_in;
}

void hp_hybrid_cpu_device::do_mpy()
{
	// Count 0->1 and 1->0 transitions in A register
	// Correct timing needs this count as real hw uses Booth's algorithm for multiplication
	uint16_t tmp = m_reg_A;
	uint16_t mask = ~0;
	for (unsigned i = 0; i < 16 && tmp; ++i) {
		if (BIT(tmp , 0)) {
			tmp ^= mask;
			m_icount -= 2;
		}
		tmp >>= 1;
		mask >>= 1;
	}

	int32_t a = int16_t(m_reg_A);
	int32_t b = int16_t(m_reg_B);
	int32_t p = a * b;

	m_reg_A = uint16_t(p & 0xffff);
	m_reg_B = uint16_t((p >> 16) & 0xffff);

	m_icount -= 59;
}

// ********************************************************************************
// hp_5061_3011_cpu_device
// ********************************************************************************
hp_5061_3011_cpu_device::hp_5061_3011_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp_hybrid_cpu_device(mconfig, HP_5061_3011, tag, owner, clock, 16)
{
}

hp_5061_3011_cpu_device::hp_5061_3011_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t addrwidth)
	: hp_hybrid_cpu_device(mconfig, type, tag, owner, clock, addrwidth)
{
}

bool hp_5061_3011_cpu_device::execute_no_bpc(uint16_t opcode , uint16_t& next_pc)
{
	// 16-bit IOC instructions
	if ((opcode & 0xf760) == 0x7160) {
		// Place/withdraw
		uint16_t tmp;
		uint16_t reg_addr = opcode & 7;
		uint16_t *ptr_reg;
		uint32_t b_mask;

		if (BIT(opcode , 3)) {
			ptr_reg = &m_reg_D;
			b_mask = BIT_MASK<uint32_t>(HPHYBRID_DB_BIT);
		} else {
			ptr_reg = &m_reg_C;
			b_mask = BIT_MASK<uint32_t>(HPHYBRID_CB_BIT);
		}

		if (BIT(opcode , 4)) {
			// Withdraw
			if (BIT(opcode , 11)) {
				// Byte
				uint32_t tmp_addr = uint32_t(*ptr_reg);
				if (m_flags & b_mask) {
					tmp_addr |= BIT_MASK<uint32_t>(16);
				}
				tmp = RM(AEC_CASE_C , uint16_t(tmp_addr >> 1));
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
				uint32_t tmp_addr = uint32_t(*ptr_reg);
				if (m_flags & b_mask) {
					tmp_addr |= BIT_MASK<uint32_t>(16);
				}
				tmp = BIT(tmp_addr , 0) ? (tmp & 0xff) : (tmp << 8);
				if (tmp_addr <= (HP_REG_LAST_ADDR * 2 + 1)) {
					// Single bytes can be written to registers.
					// The addressed register gets the written byte in the proper position
					// and a 0 in the other byte because access to registers is always done in
					// 16 bits units.
					WM(tmp_addr >> 1 , tmp);
				} else {
					if (!m_stm_func.isnull()) {
						m_stm_func(m_curr_cycle | CYCLE_WR_MASK);
						m_curr_cycle = 0;
					}
					// Extend address, form byte address
					uint16_t mask = BIT(tmp_addr , 0) ? 0x00ff : 0xff00;
					tmp_addr = add_mae(AEC_CASE_C , tmp_addr >> 1);
					m_program.write_word(tmp_addr , tmp , mask);
					m_icount -= m_w_cycles;
				}
			} else {
				// Word
				WM(AEC_CASE_C , *ptr_reg , tmp);
			}
		}
		m_icount -= 6;
	} else {
		switch (opcode) {
		case 0x7100:
			// SDO
			m_icount -= 6;
			BIT_SET(m_flags , HPHYBRID_DMADIR_BIT);
			break;

		case 0x7108:
			// SDI
			m_icount -= 6;
			BIT_CLR(m_flags , HPHYBRID_DMADIR_BIT);
			break;

		case 0x7140:
			// DBL
			m_icount -= 6;
			BIT_CLR(m_flags , HPHYBRID_DB_BIT);
			break;

		case 0x7148:
			// CBL
			m_icount -= 6;
			BIT_CLR(m_flags , HPHYBRID_CB_BIT);
			break;

		case 0x7150:
			// DBU
			m_icount -= 6;
			BIT_SET(m_flags , HPHYBRID_DB_BIT);
			break;

		case 0x7158:
			// CBU
			m_icount -= 6;
			BIT_SET(m_flags , HPHYBRID_CB_BIT);
			break;

		default:
			// Unrecognized instruction
			return false;
		}
	}
	next_pc = m_reg_P + 1;
	return true;
}

bool hp_5061_3011_cpu_device::read_non_common_reg(uint16_t addr , uint16_t& v)
{
	switch (addr) {
	case HP_REG_DMAPA_ADDR:
		v = m_dmapa & HP_REG_PA_MASK;
		if (BIT(m_flags , HPHYBRID_CB_BIT)) {
			BIT_SET(v , 15);
		}
		if (BIT(m_flags , HPHYBRID_DB_BIT)) {
			BIT_SET(v , 14);
		}
		return true;

	default:
		return false;
	}
}

bool hp_5061_3011_cpu_device::write_non_common_reg(uint16_t addr , uint16_t v)
{
	return false;
}

void hp_5061_3011_cpu_device::handle_dma()
{
	// Patent hints at the fact that terminal count is detected by bit 15 of dmac being 1 after decrementing
	bool tc = BIT(--m_dmac , 15) != 0;
	uint16_t tmp;

	m_curr_cycle |= CYCLE_DMA_MASK;
	// Timing here assumes that DMA transfers are isolated and not done in bursts
	if (BIT(m_flags , HPHYBRID_DMADIR_BIT)) {
		// "Outward" DMA: memory -> peripheral
		tmp = RM(AEC_CASE_D , m_dmama++);
		WIO(m_dmapa , tc ? 2 : 0 , tmp);
	} else {
		// "Inward" DMA: peripheral -> memory
		tmp = RIO(m_dmapa , tc ? 2 : 0);
		WM(AEC_CASE_D , m_dmama++ , tmp);
		m_icount += 1;
	}

	// Mystery solved: DMA is not automatically disabled at TC (test of 9845's graphic memory relies on this to work)
}

std::unique_ptr<util::disasm_interface> hp_5061_3011_cpu_device::create_disassembler()
{
	return std::make_unique<hp_5061_3011_disassembler>(m_relative_mode);
}

// ********************************************************************************
// hp_5061_3001_cpu_device
// ********************************************************************************
hp_5061_3001_cpu_device::hp_5061_3001_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp_5061_3011_cpu_device(mconfig, HP_5061_3001, tag, owner, clock, 22)
{
}

void hp_5061_3001_cpu_device::device_start()
{
	hp_hybrid_cpu_device::device_start();
	emc_start();

	state_add(HPHYBRID_R32, "R32" , m_reg_aec[ 0 ]);
	state_add(HPHYBRID_R33, "R33" , m_reg_aec[ 1 ]);
	state_add(HPHYBRID_R34, "R34" , m_reg_aec[ 2 ]);
	state_add(HPHYBRID_R35, "R35" , m_reg_aec[ 3 ]);
	state_add(HPHYBRID_R36, "R36" , m_reg_aec[ 4 ]);
	state_add(HPHYBRID_R37, "R37" , m_reg_aec[ 5 ]);
	save_item(NAME(m_reg_aec));
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

	hp_hybrid_cpu_device::device_reset();
}

bool hp_5061_3001_cpu_device::execute_no_bpc(uint16_t opcode , uint16_t& next_pc)
{
	// Try to execute opcode as an IOC-16 instruction first then as an EMC one
	if (hp_5061_3011_cpu_device::execute_no_bpc(opcode , next_pc) ||
		execute_emc(opcode , next_pc)) {
		return true;
	}

	// AEC instructions
	switch (opcode) {
	case 0x7026:
		// CIM
		// Undocumented instruction, see beginning of this file
		// Probably "Clear Interrupt Mode"
		// No idea at all about exec. time: make it 9 cycles (6 are in opcode fetch)
		m_icount -= 3;
		BIT_CLR(m_flags, HPHYBRID_IM_BIT);
		break;

	case 0x7027:
		// SIM
		// Undocumented instruction, see beginning of this file
		// Probably "Set Interrupt Mode"
		// No idea at all about exec. time: make it 9 cycles (6 are in opcode fetch)
		m_icount -= 3;
		BIT_SET(m_flags, HPHYBRID_IM_BIT);
		break;

	default:
		return false;
	}

	next_pc = m_reg_P + 1;
	return true;
}

uint32_t hp_5061_3001_cpu_device::add_mae(aec_cases_t aec_case , uint16_t addr)
{
	uint16_t bsc_reg;
	bool top_half = BIT(addr , 15) != 0;

	// Detect accesses to top half of base page
	if (aec_case == AEC_CASE_C && (addr & 0xfe00) == 0xfe00) {
		aec_case = AEC_CASE_B;
	}

	// **** IM == 0 ****
	// Case | Top | Bottom
	//   A  | R34 | R33
	//   B  | R36 | R33
	//   C  | R32 | R35
	//   D  | R32 | R37
	//
	// **** IM == 1 ****
	// Case | Top | Bottom
	//   A  | R34 |   5
	//   B  |   1 |   5
	//   C  |   0 | R35
	//   D  | R32 | R37
	switch (aec_case) {
	case AEC_CASE_A:
		if (top_half) {
			bsc_reg = m_reg_aec[ HP_REG_R34_ADDR - HP_REG_R32_ADDR ];
		} else {
			// Block 5 is used when IM bit overrides R33 value
			bsc_reg = BIT(m_flags , HPHYBRID_IM_BIT) ? 5 : m_reg_aec[ HP_REG_R33_ADDR - HP_REG_R32_ADDR ];
		}
		break;

	case AEC_CASE_B:
		if (top_half) {
			// Block 1 is used when IM bit overrides R36 value
			bsc_reg = BIT(m_flags , HPHYBRID_IM_BIT) ? 1 : m_reg_aec[ HP_REG_R36_ADDR - HP_REG_R32_ADDR ];
		} else {
			// Block 5 is used when IM bit overrides R33 value
			bsc_reg = BIT(m_flags , HPHYBRID_IM_BIT) ? 5 : m_reg_aec[ HP_REG_R33_ADDR - HP_REG_R32_ADDR ];
		}
		break;

	case AEC_CASE_C:
		if (top_half) {
			// Block 0 is used when IM bit overrides R32 value
			bsc_reg = BIT(m_flags , HPHYBRID_IM_BIT) ? 0 : m_reg_aec[ HP_REG_R32_ADDR - HP_REG_R32_ADDR ];
		} else {
			bsc_reg = m_reg_aec[ HP_REG_R35_ADDR - HP_REG_R32_ADDR ];
		}
		break;

	case AEC_CASE_D:
		bsc_reg = top_half ? m_reg_aec[ HP_REG_R32_ADDR - HP_REG_R32_ADDR ] : m_reg_aec[ HP_REG_R37_ADDR - HP_REG_R32_ADDR ];
			break;

	default:
		logerror("hphybrid: aec_case=%d\n" , aec_case);
		return 0;
	}

	uint16_t aec_reg = bsc_reg & BSC_REG_MASK;

	if (m_forced_bsc_25) {
		aec_reg = (aec_reg & 0xf) | 0x20;
	}

	return uint32_t(addr) | (uint32_t(aec_reg) << 16);
}

bool hp_5061_3001_cpu_device::read_non_common_reg(uint16_t addr , uint16_t& v)
{
	switch (addr) {
	case HP_REG_R32_ADDR:
	case HP_REG_R33_ADDR:
	case HP_REG_R34_ADDR:
	case HP_REG_R35_ADDR:
	case HP_REG_R36_ADDR:
	case HP_REG_R37_ADDR:
		v = m_reg_aec[ addr - HP_REG_R32_ADDR ];
		return true;

	default:
		return hp_5061_3011_cpu_device::read_non_common_reg(addr , v) ||
			read_emc_reg(addr , v);
	}
}

bool hp_5061_3001_cpu_device::write_non_common_reg(uint16_t addr , uint16_t v)
{
	switch (addr) {
	case HP_REG_R32_ADDR:
	case HP_REG_R33_ADDR:
	case HP_REG_R34_ADDR:
	case HP_REG_R35_ADDR:
	case HP_REG_R36_ADDR:
	case HP_REG_R37_ADDR:
		m_reg_aec[ addr - HP_REG_R32_ADDR ] = v;
		return true;

	default:
		return hp_5061_3011_cpu_device::write_non_common_reg(addr , v) ||
			write_emc_reg(addr , v);
	}
}

std::unique_ptr<util::disasm_interface> hp_5061_3001_cpu_device::create_disassembler()
{
	return std::make_unique<hp_5061_3001_disassembler>(m_relative_mode);
}

void hp_5061_3001_cpu_device::enter_isr()
{
	// Set interrupt mode when entering an ISR
	BIT_SET(m_flags, HPHYBRID_IM_BIT);
}

// ********************************************************************************
// hp_09825_67907_cpu_device
// ********************************************************************************
hp_09825_67907_cpu_device::hp_09825_67907_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp_hybrid_cpu_device(mconfig , HP_09825_67907 , tag , owner , clock , 15)
{
}

void hp_09825_67907_cpu_device::device_start()
{
	hp_hybrid_cpu_device::device_start();
	emc_start();
}

bool hp_09825_67907_cpu_device::execute_no_bpc(uint16_t opcode , uint16_t& next_pc)
{
	// 15-bit IOC instructions
	if ((opcode & 0xf760) == 0x7160) {
		// Place/withdraw
		uint16_t tmp;
		uint16_t reg_addr = opcode & 7;
		uint16_t *ptr_reg;

		if (BIT(opcode , 3)) {
			ptr_reg = &m_reg_D;
		} else {
			ptr_reg = &m_reg_C;
		}

		if (BIT(opcode , 4)) {
			// Withdraw
			tmp = RM(AEC_CASE_C , *ptr_reg);
			if (BIT(opcode , 11)) {
				// Byte
				if (BIT(*ptr_reg , 15)) {
					tmp >>= 8;
				} else {
					tmp &= 0xff;
				}
			}
			WM(reg_addr , tmp);

			// Post inc/dec
			inc_dec_cd(*ptr_reg , !BIT(opcode , 7) , BIT(opcode , 11));
		} else {
			// Place

			// Pre inc/dec
			inc_dec_cd(*ptr_reg , !BIT(opcode , 7) , BIT(opcode , 11));

			tmp = RM(reg_addr);
			uint16_t tmp_addr = *ptr_reg & ADDR_MASK_15BIT;
			if (BIT(opcode , 11)) {
				// Byte
				tmp = BIT(*ptr_reg , 15) ? (tmp << 8) : (tmp & 0xff);
				if (tmp_addr <= HP_REG_LAST_ADDR) {
					// Single bytes can be written to registers.
					// The addressed register gets the written byte in the proper position
					// and a 0 in the other byte because access to registers is always done in
					// 16 bits units.
					WM(tmp_addr , tmp);
				} else {
					if (!m_stm_func.isnull()) {
						m_stm_func(m_curr_cycle | CYCLE_WR_MASK);
						m_curr_cycle = 0;
					}
					uint16_t mask = BIT(*ptr_reg , 15) ? 0xff00 : 0x00ff;
					m_program.write_word(tmp_addr , tmp , mask);
					m_icount -= m_w_cycles;
				}
			} else {
				// Word
				WM(AEC_CASE_C , tmp_addr , tmp);
			}
		}
		m_icount -= 6;
		next_pc = m_reg_P + 1;
		return true;
	} else {
		return execute_emc(opcode , next_pc);
	}
}

void hp_09825_67907_cpu_device::inc_dec_cd(uint16_t& cd_reg , bool increment , bool byte)
{
	bool propagate;
	if (byte) {
		// Byte
		// Toggle bit 15
		cd_reg ^= REG_MSB_MASK;
		// When incrementing, propagate to 15 LSBs when bit 15 goes 0->1
		propagate = (cd_reg & REG_MSB_MASK) != 0;
		if (!increment) {
			// When decrementing, propagate when bit 15 goes 1->0
			propagate = !propagate;
		}
	} else {
		// Word
		propagate = true;
	}
	if (propagate) {
		if (increment) {
			cd_reg = (cd_reg & ~ADDR_MASK_15BIT) | ((cd_reg + 1) & ADDR_MASK_15BIT);
		} else {
			cd_reg = (cd_reg & ~ADDR_MASK_15BIT) | ((cd_reg - 1) & ADDR_MASK_15BIT);
		}
	}
}

bool hp_09825_67907_cpu_device::read_non_common_reg(uint16_t addr , uint16_t& v)
{
	switch (addr) {
	case HP_REG_DMAPA_ADDR:
		v = m_dmapa;
		return true;

	default:
		return read_emc_reg(addr , v);
	}
}

bool hp_09825_67907_cpu_device::write_non_common_reg(uint16_t addr , uint16_t v)
{
	return write_emc_reg(addr , v);
}

uint16_t hp_09825_67907_cpu_device::get_indirect_target(uint32_t addr)
{
	uint16_t tmp;
	bool ind;

	// Multi-level indirect addressing
	// TODO: It wouldn't hurt to have some limit on iterations
	do {
		tmp = RM(addr);
		ind = BIT(tmp , 15);
		addr = tmp & ADDR_MASK_15BIT;
	} while (ind);

	return tmp;
}

void hp_09825_67907_cpu_device::handle_dma()
{
	bool tc = BIT(--m_dmac , 15) != 0;
	uint16_t tmp;

	// Timing here assumes that DMA transfers are isolated and not done in bursts
	m_curr_cycle |= CYCLE_DMA_MASK;
	if (BIT(m_dmama , 15)) {
		// "Outward" DMA: memory -> peripheral
		tmp = RM(AEC_CASE_D , m_dmama);
		WIO(m_dmapa , tc ? 2 : 0 , tmp);
	} else {
		// "Inward" DMA: peripheral -> memory
		tmp = RIO(m_dmapa , tc ? 2 : 0);
		WM(AEC_CASE_D , m_dmama , tmp);
		m_icount += 1;
	}
	m_dmama = (m_dmama & ~ADDR_MASK_15BIT) | ((m_dmama + 1) & ADDR_MASK_15BIT);
}

std::unique_ptr<util::disasm_interface> hp_09825_67907_cpu_device::create_disassembler()
{
	return std::make_unique<hp_09825_67907_disassembler>(m_relative_mode);
}
