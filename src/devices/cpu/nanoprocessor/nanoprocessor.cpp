// license:BSD-3-Clause
// copyright-holders:F. Ulivi

#include "emu.h"
#include "nanoprocessor.h"
#include "nanoprocessor_dasm.h"

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

DEFINE_DEVICE_TYPE(HP_NANOPROCESSOR, hp_nanoprocessor_device, "nanoprocessor", "Hewlett Packard HP-Nanoprocessor")

hp_nanoprocessor_device::hp_nanoprocessor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, HP_NANOPROCESSOR, tag, owner, clock),
	m_dc_changed_func(*this),
	m_read_dc_func(*this),
	m_int_ack_func(*this),
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

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_IO).specific(m_io);

	save_item(NAME(m_reg_A));
	save_item(NAME(m_reg_R));
	save_item(NAME(m_reg_PA));
	save_item(NAME(m_reg_SSR));
	save_item(NAME(m_reg_ISR));
	save_item(NAME(m_flags));

	set_icountptr(m_icount);

	m_dc_changed_func.resolve_safe();
	m_read_dc_func.resolve_safe(0xff);
	m_int_ack_func.resolve_safe(0xff);
}

void hp_nanoprocessor_device::device_reset()
{
	// IRL reset signal only sets the following things:
	// DC7 (int. enable) = 0
	// DC6..0 = 1
	// PA = 0
	m_reg_A = 0;
	for (auto& reg : m_reg_R) {
		reg = 0;
	}
	m_reg_PA = 0;
	m_reg_SSR = 0;
	m_reg_ISR = 0;
	m_flags = ((1U << (HP_NANO_DC_NO - 1)) - 1) << NANO_DC0_BIT;
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
			standard_irq_callback(0);
			m_reg_ISR = m_reg_PA;
			m_reg_PA = m_int_ack_func();
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
	// Apply indexing
	if ((opcode & 0xe0) == 0xe0 ||
		(opcode & 0xf0) == 0x90) {
		// This is how the real hw does indexing. Altering the opcode in this way explains
		// why a JAI instruction is turned into a JAS when R0:b3 = 1 (opcode 90..97 -> 98..9f)
		opcode |= (m_reg_R[ 0 ] & 0x0f);
	}

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
		// Handling of non-decimal digits comes from chip RE
		{
			uint8_t nibble_lo = m_reg_A & 0x0f;
			uint8_t nibble_hi = m_reg_A & 0xf0;

			if ((nibble_lo & 0x09) != 0x09) {
				nibble_lo++;
			} else {
				nibble_lo = (nibble_lo + 1) & 0x05;
				if ((nibble_hi & 0x90) != 0x90) {
					nibble_hi += 0x10;
				} else {
					nibble_hi = (nibble_hi + 0x10) & 0x50;
					BIT_SET(m_flags, NANO_E_BIT);
				}
			}
			m_reg_A = nibble_hi | nibble_lo;
		}
		break;

	case 0x03:
		// DED
		// Handling of non-decimal digits comes from chip RE
		{
			uint8_t nibble_lo = m_reg_A & 0x0f;
			uint8_t nibble_hi = m_reg_A & 0xf0;

			if (nibble_lo) {
				nibble_lo--;
			} else {
				nibble_lo = 9;
				if (nibble_hi) {
					nibble_hi -= 0x10;
				} else {
					nibble_hi = 0x90;
					BIT_SET(m_flags, NANO_E_BIT);
				}
			}
			m_reg_A = nibble_hi | nibble_lo;
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
		[[fallthrough]];
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
		// This op is implemented in the released NP mask set exactly as RTS (i.e.
		// IE is not set). I'm implementing it here as described in the manual.
		dc_set(HP_NANO_IE_DC);
		// Intentional fall-through to RTS!
		[[fallthrough]];
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
			[[fallthrough]];
		case 0x90:
			// JAI
			{
				uint16_t tmp = (uint16_t)(opcode & 7) << 8;
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
				m_reg_A = m_io.read_byte(opcode & 0xf);
				break;

			case 0x50:
				// OTA
				m_io.write_byte(opcode & 0xf, m_reg_A);
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
				m_io.write_byte(opcode & 0xf, fetch());
				break;

			case 0xd0:
				// STR
				m_reg_R[ opcode & 0xf ] = fetch();
				break;

			case 0xe0:
				// LDI
				m_reg_A = m_reg_R[ opcode & 0xf ];
				break;

			case 0xf0:
				// STI
				m_reg_R[ opcode & 0xf ] = m_reg_A;
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
	uint8_t res = m_cache.read_byte(m_reg_PA);
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
