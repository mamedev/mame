// license:BSD-3-Clause
// copyright-holders:F. Ulivi

#include "emu.h"
#include "nanoprocessor.h"
#include "nanoprocessor_dasm.h"
#include "debugger.h"

// Index of state variables
enum {
	NANO_REG_A,
	NANO_REG_R0,
	NANO_REG_R1,
	NANO_REG_R2,
	NANO_REG_R3,
	NANO_REG_R4,
	NANO_REG_R5,
	NANO_REG_R6,
	NANO_REG_R7,
	NANO_REG_R8,
	NANO_REG_R9,
	NANO_REG_R10,
	NANO_REG_R11,
	NANO_REG_R12,
	NANO_REG_R13,
	NANO_REG_R14,
	NANO_REG_R15,
	NANO_REG_PA,
	NANO_REG_SSR,
	NANO_REG_ISR,
	NANO_REG_FLAGS
};

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w, n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w, n)  ((w) |= BIT_MASK(n))

// Bits in m_flags
#define NANO_DC0_BIT    0   // DC0
#define NANO_E_BIT  (NANO_DC0_BIT + HP_NANO_DC_NO)  // Extend flag
#define NANO_I_BIT  (NANO_E_BIT + 1)    // Interrupt flag

DEFINE_DEVICE_TYPE(HP_NANOPROCESSOR, hp_nanoprocessor_device, "nanoprocessor", "Hewlett Packard HP-Nanoprocessor")

hp_nanoprocessor_device::hp_nanoprocessor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, HP_NANOPROCESSOR, tag, owner, clock),
	m_dc_changed_func(*this),
	m_read_dc_func(*this),
	m_program_config("program", ENDIANNESS_BIG, 8, 11),
	m_io_config("io", ENDIANNESS_BIG, 8, 4)
{
}

void hp_nanoprocessor_device::device_start()
{
	state_add(NANO_REG_A, "A", m_reg_A);
	state_add(NANO_REG_R0, "R0", m_reg_R[ 0 ]);
	state_add(NANO_REG_R1, "R1", m_reg_R[ 1 ]);
	state_add(NANO_REG_R2, "R2", m_reg_R[ 2 ]);
	state_add(NANO_REG_R3, "R3", m_reg_R[ 3 ]);
	state_add(NANO_REG_R4, "R4", m_reg_R[ 4 ]);
	state_add(NANO_REG_R5, "R5", m_reg_R[ 5 ]);
	state_add(NANO_REG_R6, "R6", m_reg_R[ 6 ]);
	state_add(NANO_REG_R7, "R7", m_reg_R[ 7 ]);
	state_add(NANO_REG_R8, "R8", m_reg_R[ 8 ]);
	state_add(NANO_REG_R9, "R9", m_reg_R[ 9 ]);
	state_add(NANO_REG_R10, "R10", m_reg_R[ 10 ]);
	state_add(NANO_REG_R11, "R11", m_reg_R[ 11 ]);
	state_add(NANO_REG_R12, "R12", m_reg_R[ 12 ]);
	state_add(NANO_REG_R13, "R13", m_reg_R[ 13 ]);
	state_add(NANO_REG_R14, "R14", m_reg_R[ 14 ]);
	state_add(NANO_REG_R15, "R15", m_reg_R[ 15 ]);
	state_add(NANO_REG_PA, "PA", m_reg_PA).formatstr("%03X");
	state_add(STATE_GENPC, "GENPC", m_reg_PA).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_reg_PA).noshow();
	state_add(NANO_REG_SSR, "SSR", m_reg_SSR).formatstr("%03X");
	state_add(NANO_REG_ISR, "ISR", m_reg_ISR).formatstr("%03X");
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).noshow().formatstr("%10s");

	m_program = &space(AS_PROGRAM);
	m_cache = m_program->cache<0, 0, ENDIANNESS_BIG>();
	m_io = &space(AS_IO);

	save_item(NAME(m_reg_A));
	save_item(NAME(m_reg_R));
	save_item(NAME(m_reg_PA));
	save_item(NAME(m_reg_SSR));
	save_item(NAME(m_reg_ISR));
	save_item(NAME(m_flags));

	set_icountptr(m_icount);

	m_dc_changed_func.resolve_safe();
	m_read_dc_func.resolve_safe(0xff);
}

void hp_nanoprocessor_device::device_reset()
{
	m_reg_A = 0;
	for (auto& reg : m_reg_R) {
		reg = 0;
	}
	m_reg_PA = 0;
	m_reg_SSR = 0;
	m_reg_ISR = 0;
	m_flags = 0;
	dc_update();
}

device_memory_interface::space_config_vector hp_nanoprocessor_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

void hp_nanoprocessor_device::execute_run()
{
	do {
		// Check for interrupts (interrupt line is always enabled. Masking is done
		// outside of the NP, usually by ANDing the DC7 line with the interrupt
		// request signal)
		if (BIT(m_flags, NANO_I_BIT)) {
			m_reg_ISR = m_reg_PA;
			m_reg_PA = (uint16_t)(standard_irq_callback(0) & 0xff);
			// Vector fetching takes 1 cycle
			m_icount -= 1;
			dc_clr(HP_NANO_IE_DC);
			// Need this to propagate the clearing of DC7 to the clearing of int. line
			yield();
		} else {
			debugger_instruction_hook(m_reg_PA);

			uint8_t opcode = fetch();
			execute_one(opcode);
			// All opcodes execute in 2 cycles
			m_icount -= 2;
		}
	} while (m_icount > 0);
}

void hp_nanoprocessor_device::execute_set_input(int linenum, int state)
{
	if (linenum == 0) {
		if (state) {
			BIT_SET(m_flags, NANO_I_BIT);
		} else {
			BIT_CLR(m_flags, NANO_I_BIT);
		}
	}
}

void hp_nanoprocessor_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() == STATE_GENFLAGS) {
		// DC7 is reported as "I" because it is usually used as interrupt enable
		str = string_format("%c %c%c%c%c%c%c%c%c", BIT(m_flags, NANO_E_BIT) ? 'E' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 7) ? 'I' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 6) ? '6' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 5) ? '5' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 4) ? '4' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 3) ? '3' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 2) ? '2' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 1) ? '1' : ' ',
							BIT(m_flags, NANO_DC0_BIT + 0) ? '0' : ' ');
	}

}

std::unique_ptr<util::disasm_interface> hp_nanoprocessor_device::create_disassembler()
{
	return std::make_unique<hp_nanoprocessor_disassembler>();
}

void hp_nanoprocessor_device::execute_one(uint8_t opcode)
{
	// Instructions without mask
	switch (opcode) {
	case 0x00:
		// INB
		m_reg_A++;
		if (m_reg_A == 0) {
			BIT_SET(m_flags, NANO_E_BIT);
		}
		break;

	case 0x01:
		// DEB
		m_reg_A--;
		if (m_reg_A == 0xff) {
			BIT_SET(m_flags, NANO_E_BIT);
		}
		break;

	case 0x02:
		// IND
		// Handling of non-decimal digits is entirely arbitrary
		m_reg_A++;
		if ((m_reg_A & 0x0f) >= 10) {
			m_reg_A += 6;
			if (m_reg_A >= 0xa0) {
				m_reg_A += 0x60;
				BIT_SET(m_flags, NANO_E_BIT);
			}
		}
		break;

	case 0x03:
		// DED
		// Handling of non-decimal digits is entirely arbitrary
		m_reg_A--;
		if ((m_reg_A & 0x0f) >= 10) {
			m_reg_A -= 6;
			if (m_reg_A >= 0xa0) {
				m_reg_A -= 0x60;
				BIT_SET(m_flags, NANO_E_BIT);
			}
		}
		break;

	case 0x04:
		// CLA
		m_reg_A = 0;
		break;

	case 0x05:
		// CMA
		m_reg_A = ~m_reg_A;
		break;

	case 0x06:
		// RSA
		m_reg_A >>= 1;
		break;

	case 0x07:
		// LSA
		m_reg_A <<= 1;
		break;

	case 0x08:
		// SGT
		if (m_reg_A > m_reg_R[ 0 ]) {
			skip();
		}
		break;

	case 0x09:
		// SLT
		if (m_reg_A < m_reg_R[ 0 ]) {
			skip();
		}
		break;

	case 0x0a:
		// SEQ
		if (m_reg_A == m_reg_R[ 0 ]) {
			skip();
		}
		break;

	case 0x0b:
		// SAZ
		if (m_reg_A == 0) {
			skip();
		}
		break;

	case 0x0c:
		// SLE
		if (m_reg_A <= m_reg_R[ 0 ]) {
			skip();
		}
		break;

	case 0x0d:
		// SGE
		if (m_reg_A >= m_reg_R[ 0 ]) {
			skip();
		}
		break;

	case 0x0e:
		// SNE
		if (m_reg_A != m_reg_R[ 0 ]) {
			skip();
		}
		break;

	case 0x0f:
		// SAN
		if (m_reg_A != 0) {
			skip();
		}
		break;

	case 0x1f:
		// SES
		if (BIT(m_flags, NANO_E_BIT)) {
			skip();
		}
		break;

	case 0x3f:
		// SEZ
		if (!BIT(m_flags, NANO_E_BIT)) {
			skip();
		}
		break;

	case 0x5f:
		// NOP
		break;

	case 0xb1:
		// RTE
		dc_set(HP_NANO_IE_DC);
		// Intentional fall-through to RTI!

	case 0xb0:
		// RTI
		m_reg_PA = m_reg_ISR;
		break;

	case 0xb4:
		// STE
		BIT_SET(m_flags, NANO_E_BIT);
		break;

	case 0xb5:
		// CLE
		BIT_CLR(m_flags, NANO_E_BIT);
		break;

	case 0xb9:
		// RSE
		dc_set(HP_NANO_IE_DC);
		// Intentional fall-through to RTS!

	case 0xb8:
		// RTS
		{
			uint16_t tmp = m_reg_SSR;
			m_reg_SSR = pa_offset(1);
			m_reg_PA = tmp;
		}
		break;

	case 0xcf:
		// LDR
		m_reg_A = fetch();
		break;

	default:
		// Instructions with 0xf8 mask
		switch (opcode & 0xf8) {
		case 0x10:
			// SBS
			if (BIT(m_reg_A, opcode & 7)) {
				skip();
			}
			break;

		case 0x18:
			// SFS
			{
				uint8_t tmp = m_read_dc_func();
				tmp &= (uint8_t)(m_flags >> NANO_DC0_BIT);
				if (BIT(tmp, opcode & 7)) {
					skip();
				}
			}
			break;

		case 0x20:
			// SBN
			BIT_SET(m_reg_A, opcode & 7);
			break;

		case 0x28:
			// STC
			dc_set(opcode & 7);
			break;

		case 0x30:
			// SBZ
			if (!BIT(m_reg_A, opcode & 7)) {
				skip();
			}
			break;

		case 0x38:
			// SFZ
			{
				uint8_t tmp = m_read_dc_func();
				tmp &= (uint8_t)(m_flags >> NANO_DC0_BIT);
				if (!BIT(tmp, opcode & 7)) {
					skip();
				}
			}
			break;

		case 0x80:
			// JMP
			m_reg_PA = ((uint16_t)(opcode & 7) << 8) | fetch();
			break;

		case 0x88:
			// JSB
			{
				uint16_t tmp = ((uint16_t)(opcode & 7) << 8) | fetch();
				m_reg_SSR = m_reg_PA;
				m_reg_PA = tmp;
			}
			break;

		case 0x98:
			// JAS
			m_reg_SSR = pa_offset(1);
			// Intentional fall-through to JAI!

		case 0x90:
			// JAI
			// On HP doc there's a mysterious warning about JAI:
			// "Due to the indexing structure, a JAI instruction executed with
			//  R03 set will be executed as a JAS instruction"
			// My idea on the meaning: NP recycles the instruction register to form
			// the bitwise OR of bits 3-0 of R0 and of opcode (see LDI/STI
			// instructions). Presumably this was done to save on flip-flop count.
			// So, if bit 3 of R0 (R03) is set when executing JAI the instruction
			// register turns JAI into JAS.
			// This effect is not simulated here at the moment.
			{
				uint16_t tmp = (uint16_t)((m_reg_R[ 0 ] | opcode) & 7) << 8;
				m_reg_PA = tmp | m_reg_A;
			}
			break;

		case 0xa0:
			// CBN
			BIT_CLR(m_reg_A, opcode & 7);
			break;

		case 0xa8:
			// CLC
			dc_clr(opcode & 7);
			break;

		default:
			// Instructions with 0xf0 mask
			switch (opcode & 0xf0) {
			case 0x40:
				// INA
				m_reg_A = m_io->read_byte(opcode & 0xf);
				break;

			case 0x50:
				// OTA
				m_io->write_byte(opcode & 0xf, m_reg_A);
				break;

			case 0x60:
				// LDA
				m_reg_A = m_reg_R[ opcode & 0xf ];
				break;

			case 0x70:
				// STA
				m_reg_R[ opcode & 0xf ] = m_reg_A;
				break;

			case 0xc0:
				// OTR
				m_io->write_byte(opcode & 0xf, fetch());
				break;

			case 0xd0:
				// STR
				m_reg_R[ opcode & 0xf ] = fetch();
				break;

			case 0xe0:
				// LDI
				m_reg_A = m_reg_R[ (m_reg_R[ 0 ] | opcode) & 0xf ];
				break;

			case 0xf0:
				// STI
				m_reg_R[ (m_reg_R[ 0 ] | opcode) & 0xf ] = m_reg_A;
				break;

			default:
				logerror("Unknown opcode %02x @ 0x03x\n", opcode, m_reg_PA);
				break;
			}
		}
	}
}

uint16_t hp_nanoprocessor_device::pa_offset(unsigned off) const
{
	return (uint16_t)((m_reg_PA + off) & HP_NANO_PC_MASK);
}

uint8_t hp_nanoprocessor_device::fetch(void)
{
	uint8_t res = m_cache->read_byte(m_reg_PA);
	m_reg_PA = pa_offset(1);
	return res;
}

void hp_nanoprocessor_device::skip(void)
{
	m_reg_PA = pa_offset(2);
}

void hp_nanoprocessor_device::dc_update(void)
{
	m_dc_changed_func((uint8_t)(m_flags & ((1U << HP_NANO_DC_NO) - 1)));
}

void hp_nanoprocessor_device::dc_set(unsigned bit_no)
{
	BIT_SET(m_flags, NANO_DC0_BIT + bit_no);
	dc_update();
}

void hp_nanoprocessor_device::dc_clr(unsigned bit_no)
{
	BIT_CLR(m_flags, NANO_DC0_BIT + bit_no);
	dc_update();
}
