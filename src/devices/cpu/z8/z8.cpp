// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

**********************************************************************/

/*

    TODO:

    - strobed I/O
    - expose register file to disassembler
    - timer Tin/Tout modes
    - serial
    - instruction pipeline
    - internal diagnostic ROM in data space (requires high voltage reset)

*/

#include "emu.h"
#include "z8.h"
#include "z8dasm.h"
#include "debugger.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	Z8_REGISTER_P0 = 0,
	Z8_REGISTER_P1,
	Z8_REGISTER_P2,
	Z8_REGISTER_P3,
	Z8_REGISTER_SIO = 0xf0,
	Z8_REGISTER_TMR,
	Z8_REGISTER_T1,
	Z8_REGISTER_PRE1,
	Z8_REGISTER_T0,
	Z8_REGISTER_PRE0,
	Z8_REGISTER_P2M,
	Z8_REGISTER_P3M,
	Z8_REGISTER_P01M,
	Z8_REGISTER_IPR,
	Z8_REGISTER_IRQ,
	Z8_REGISTER_IMR,
	Z8_REGISTER_FLAGS,
	Z8_REGISTER_RP,
	Z8_REGISTER_SPH,
	Z8_REGISTER_SPL
};

#define Z8_P3_DAV0                  0x04    /* not supported */
#define Z8_P3_DAV1                  0x08    /* not supported */
#define Z8_P3_DAV2                  0x02    /* not supported */
#define Z8_P3_RDY0                  0x20    /* not supported */
#define Z8_P3_RDY1                  0x10    /* not supported */
#define Z8_P3_RDY2                  0x40    /* not supported */
#define Z8_P3_IRQ0                  0x04    /* not supported */
#define Z8_P3_IRQ1                  0x08    /* not supported */
#define Z8_P3_IRQ2                  0x02    /* not supported */
#define Z8_P3_IRQ3                  0x01    /* not supported */
#define Z8_P3_DI                    0x01    /* not supported */
#define Z8_P3_DO                    0x80    /* not supported */
#define Z8_P3_TIN                   0x02    /* not supported */
#define Z8_P3_TOUT                  0x40    /* not supported */
#define Z8_P3_DM                    0x10    /* not supported */

#define Z8_PRE0_COUNT_MODULO_N      0x01

#define Z8_PRE1_COUNT_MODULO_N      0x01
#define Z8_PRE1_INTERNAL_CLOCK      0x02

#define Z8_TMR_LOAD_T0              0x01
#define Z8_TMR_ENABLE_T0            0x02
#define Z8_TMR_LOAD_T1              0x04
#define Z8_TMR_ENABLE_T1            0x08
#define Z8_TMR_TIN_MASK             0x30    /* not supported */
#define Z8_TMR_TIN_EXTERNAL_CLK     0x00    /* not supported */
#define Z8_TMR_TIN_GATE             0x10    /* not supported */
#define Z8_TMR_TIN_TRIGGER          0x20    /* not supported */
#define Z8_TMR_TIN_RETRIGGER        0x30    /* not supported */
#define Z8_TMR_TOUT_MASK            0xc0    /* not supported */
#define Z8_TMR_TOUT_OFF             0x00    /* not supported */
#define Z8_TMR_TOUT_T0              0x40    /* not supported */
#define Z8_TMR_TOUT_T1              0x80    /* not supported */
#define Z8_TMR_TOUT_INTERNAL_CLK    0xc0    /* not supported */

#define Z8_P01M_P0L_MODE_MASK       0x03
#define Z8_P01M_P0L_MODE_OUTPUT     0x00
#define Z8_P01M_P0L_MODE_INPUT      0x01
#define Z8_P01M_P0L_MODE_A8_A11     0x02
#define Z8_P01M_INTERNAL_STACK      0x04
#define Z8_P01M_P1_MODE_MASK        0x18
#define Z8_P01M_P1_MODE_OUTPUT      0x00
#define Z8_P01M_P1_MODE_INPUT       0x08
#define Z8_P01M_P1_MODE_AD0_AD7     0x10    /* not supported */
#define Z8_P01M_P1_MODE_HI_Z        0x18    /* not supported */
#define Z8_P01M_EXTENDED_TIMING     0x20    /* not supported */
#define Z8_P01M_P0H_MODE_MASK       0xc0
#define Z8_P01M_P0H_MODE_OUTPUT     0x00
#define Z8_P01M_P0H_MODE_INPUT      0x40
#define Z8_P01M_P0H_MODE_A12_A15    0x80

#define Z8_P3M_P2_ACTIVE_PULLUPS    0x01    /* not supported */
#define Z8_P3M_P0_STROBED           0x04    /* not supported */
#define Z8_P3M_P33_P34_MASK         0x18
#define Z8_P3M_P33_P34_INPUT_OUTPUT 0x00
#define Z8_P3M_P33_P34_INPUT_DM     0x08    /* not supported */
#define Z8_P3M_P33_P34_INPUT_DM_2   0x10    /* not supported */
#define Z8_P3M_P33_P34_DAV1_RDY1    0x18    /* not supported */
#define Z8_P3M_P2_STROBED           0x20    /* not supported */
#define Z8_P3M_P3_SERIAL            0x40    /* not supported */
#define Z8_P3M_PARITY               0x80    /* not supported */

#define Z8_IMR_ENABLE               0x80
#define Z8_IMR_RAM_PROTECT          0x40    /* not supported */

#define Z8_IRQ_MASK                 0x3f
#define Z8_IRQ_FLAG_IRQ5            0x20
#define Z8_IRQ_FLAG_IRQ4            0x10
#define Z8_IRQ_FLAG_IRQ3            0x08
#define Z8_IRQ_FLAG_IRQ2            0x04
#define Z8_IRQ_FLAG_IRQ1            0x02
#define Z8_IRQ_FLAG_IRQ0            0x01

#define Z8_FLAGS_F1                 0x01
#define Z8_FLAGS_F2                 0x02
#define Z8_FLAGS_H                  0x04
#define Z8_FLAGS_D                  0x08
#define Z8_FLAGS_V                  0x10
#define Z8_FLAGS_S                  0x20
#define Z8_FLAGS_Z                  0x40
#define Z8_FLAGS_C                  0x80

enum
{
	CC_F = 0, CC_LT, CC_LE, CC_ULE, CC_OV, CC_MI, CC_Z, CC_C,
	CC_T, CC_GE, CC_GT, CC_UGT, CC_NOV, CC_PL, CC_NZ, CC_NC
};

/***************************************************************************
    MACROS
***************************************************************************/

#define P01M        m_r[Z8_REGISTER_P01M]
#define P2M         m_r[Z8_REGISTER_P2M]
#define P3M         m_r[Z8_REGISTER_P3M]
#define T0          m_r[Z8_REGISTER_T0]
#define T1          m_r[Z8_REGISTER_T1]
#define PRE0        m_r[Z8_REGISTER_PRE0]
#define PRE1        m_r[Z8_REGISTER_PRE1]


DEFINE_DEVICE_TYPE(Z8601,   z8601_device,   "z8601",   "Zilog Z8601")
DEFINE_DEVICE_TYPE(UB8830D, ub8830d_device, "ub8830d", "UB8830D")
DEFINE_DEVICE_TYPE(Z8611,   z8611_device,   "z8611",   "Zilog Z8611")
DEFINE_DEVICE_TYPE(Z8671,   z8671_device,   "z8671",   "Zilog Z8671")
DEFINE_DEVICE_TYPE(Z8681,   z8681_device,   "z8681",   "Zilog Z8681")


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void z8_device::program_map(address_map &map)
{
	if (m_rom_size > 0)
		map(0x0000, m_rom_size - 1).rom().region(DEVICE_SELF, 0);
}

void z8_device::preprogrammed_map(address_map &map)
{
	map(0x0000, m_rom_size - 1).rom().region("internal", 0);
}


z8_device::z8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t rom_size, bool preprogrammed)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, preprogrammed ? address_map_constructor(FUNC(z8_device::preprogrammed_map), this) : address_map_constructor(FUNC(z8_device::program_map), this))
	, m_data_config("data", ENDIANNESS_BIG, 8, 16, 0)
	, m_input_cb{{*this}, {*this}, {*this}, {*this}}
	, m_output_cb{{*this}, {*this}, {*this}, {*this}}
	, m_rom_size(rom_size)
{
	assert(((rom_size - 1) & rom_size) == 0);
}


z8601_device::z8601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8601, tag, owner, clock, 0x800, false)
{
}


ub8830d_device::ub8830d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, UB8830D, tag, owner, clock, 0x800, false)
{
}


z8611_device::z8611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8611, tag, owner, clock, 0x1000, false)
{
}


z8671_device::z8671_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8671, tag, owner, clock, 0x800, true)
{
}

ROM_START(z8671)
	ROM_REGION(0x0800, "internal", 0)
	ROM_LOAD("z8671.bin", 0x0000, 0x0800, CRC(3fceeb76) SHA1(290a24c77debd2e280fe31380287838c5fb7cabd))
ROM_END

const tiny_rom_entry *z8671_device::device_rom_region() const
{
	return ROM_NAME(z8671);
}


z8681_device::z8681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8_device(mconfig, Z8681, tag, owner, clock, 0, false)
{
}


std::unique_ptr<util::disasm_interface> z8_device::create_disassembler()
{
	return std::make_unique<z8_disassembler>();
}


device_memory_interface::space_config_vector z8_device::memory_space_config() const
{
	// Separate data space is optional
	if (has_configured_map(AS_DATA))
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_DATA,    &m_data_config)
		};
	}
	else
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
	}
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

uint16_t z8_device::mask_external_address(uint16_t addr)
{
	switch (P01M & (Z8_P01M_P0L_MODE_A8_A11 | Z8_P01M_P0H_MODE_A12_A15))
	{
		case 0:
			addr = (addr & 0x00ff) | register_read(Z8_REGISTER_P0) << 8;
			break;

		case Z8_P01M_P0L_MODE_A8_A11:
			addr = (addr & 0x0fff) | (register_read(Z8_REGISTER_P0) & 0xf0) << 8;
			break;

		case Z8_P01M_P0H_MODE_A12_A15:
			addr = (addr & 0xf0ff) | (register_read(Z8_REGISTER_P0) & 0x0f) << 8;
			break;
	}
	return addr;
}


uint8_t z8_device::fetch()
{
	uint16_t real_pc = (m_pc < m_rom_size) ? m_pc : mask_external_address(m_pc);
	uint8_t data = m_cache->read_byte(real_pc);

	m_pc++;

	return data;
}


uint8_t z8_device::fetch_opcode()
{
	m_ppc = (m_pc < m_rom_size) ? m_pc : mask_external_address(m_pc);
	debugger_instruction_hook(m_ppc);

	uint8_t data = m_cache->read_byte(m_ppc);

	m_pc++;

	return data;
}


uint16_t z8_device::fetch_word()
{
	// ensure correct order of operations by using separate instructions
	uint16_t data = fetch() << 8;
	data |= fetch();

	return data;
}


uint8_t z8_device::register_read(uint8_t offset)
{
	uint8_t data = 0xff;
	uint8_t mask = 0;

	switch (offset)
	{
	case Z8_REGISTER_P0:
		switch (P01M & Z8_P01M_P0L_MODE_MASK)
		{
		case Z8_P01M_P0L_MODE_OUTPUT:
			data = m_output[offset] & 0x0f;
			break;
		case Z8_P01M_P0L_MODE_INPUT:
			mask = 0x0f;
			break;
		default: /* A8...A11 */
			data = 0x0f;
			break;
		}

		switch (P01M & Z8_P01M_P0H_MODE_MASK)
		{
		case Z8_P01M_P0H_MODE_OUTPUT:
			data |= m_output[offset] & 0xf0;
			break;
		case Z8_P01M_P0H_MODE_INPUT:
			mask |= 0xf0;
			break;
		default: /* A12...A15 */
			data |= 0xf0;
			break;
		}

		if (!(P3M & Z8_P3M_P0_STROBED))
		{
			if (mask) m_input[offset] = m_input_cb[0](0, mask);
		}

		data |= m_input[offset] & mask;
		break;

	case Z8_REGISTER_P1:
		switch (P01M & Z8_P01M_P1_MODE_MASK)
		{
		case Z8_P01M_P1_MODE_OUTPUT:
			data = m_output[offset];
            break;
		case Z8_P01M_P1_MODE_INPUT:
			mask = 0xff;
			break;
		default: /* AD0..AD7 */
			data = 0xff;
			break;
		}

		if ((P3M & Z8_P3M_P33_P34_MASK) != Z8_P3M_P33_P34_DAV1_RDY1)
		{
			if (mask) m_input[offset] = m_input_cb[1](0, mask);
		}

		data |= m_input[offset] & mask;
		break;

	case Z8_REGISTER_P2:
		mask = m_r[Z8_REGISTER_P2M];

		if (!(P3M & Z8_P3M_P2_STROBED))
		{
			if (mask) m_input[offset] = m_input_cb[2](0, mask);
		}

		data = (m_input[offset] & mask) | (m_output[offset] & ~mask);
		break;

	case Z8_REGISTER_P3:
		// TODO: special port 3 modes
		//if (!(P3M & 0x7c))
		//{
			mask = 0x0f;
		//}

		if (mask) m_input[offset] = m_input_cb[3](0, mask);

		data = (m_input[offset] & mask) | (m_output[offset] & ~mask);
		break;

	case Z8_REGISTER_T0:
		data = m_t0;
		break;

	case Z8_REGISTER_T1:
		data = m_t1;
		break;

	case Z8_REGISTER_PRE1:
	case Z8_REGISTER_PRE0:
	case Z8_REGISTER_P2M:
	case Z8_REGISTER_P3M:
	case Z8_REGISTER_P01M:
	case Z8_REGISTER_IPR:
		/* write only */
		break;

	default:
		data = m_r[offset];
		break;
	}

	return data;
}

uint16_t z8_device::register_pair_read(uint8_t offset)
{
	return (register_read(offset) << 8) | register_read(offset + 1);
}

void z8_device::register_write(uint8_t offset, uint8_t data)
{
	uint8_t mask = 0;

	switch (offset)
	{
	case Z8_REGISTER_P0:
		m_output[offset] = data;
		if ((P01M & Z8_P01M_P0L_MODE_MASK) == Z8_P01M_P0L_MODE_OUTPUT) mask |= 0x0f;
		if ((P01M & Z8_P01M_P0H_MODE_MASK) == Z8_P01M_P0H_MODE_OUTPUT) mask |= 0xf0;
		if (mask) m_output_cb[0](0, data & mask, mask);
		break;

	case Z8_REGISTER_P1:
		m_output[offset] = data;
		if ((P01M & Z8_P01M_P1_MODE_MASK) == Z8_P01M_P1_MODE_OUTPUT) mask = 0xff;
		if (mask) m_output_cb[1](0, data & mask, mask);
		break;

	case Z8_REGISTER_P2:
		m_output[offset] = data;
		mask = m_r[Z8_REGISTER_P2M] ^ 0xff;
		if (mask) m_output_cb[2](0, data & mask, mask);
		break;

	case Z8_REGISTER_P3:
		m_output[offset] = data;

		// TODO: special port 3 modes
		//if (!(P3M & 0x7c))
		//{
			mask = 0xf0;
		//}

		if (mask) m_output_cb[3](0, data & mask, mask);
		break;

	case Z8_REGISTER_SIO:
		break;

	case Z8_REGISTER_TMR:
		if (data & Z8_TMR_LOAD_T0)
		{
			m_t0 = T0;
			m_t0_timer->adjust(attotime::zero, 0, cycles_to_attotime(4 * ((PRE0 >> 2) + 1)));
		}

		m_t0_timer->enable(data & Z8_TMR_ENABLE_T0);

		if (data & Z8_TMR_LOAD_T1)
		{
			m_t1 = T1;
			m_t1_timer->adjust(attotime::zero, 0, cycles_to_attotime(4 * ((PRE1 >> 2) + 1)));
		}

		m_t1_timer->enable(data & Z8_TMR_ENABLE_T1);
		break;

	case Z8_REGISTER_P2M:
		break;
	case Z8_REGISTER_P3M:
		break;
	case Z8_REGISTER_P01M:
		break;
	case Z8_REGISTER_IPR:
		break;
	case Z8_REGISTER_IRQ:
		break;
	case Z8_REGISTER_IMR:
		break;
	case Z8_REGISTER_FLAGS:
		break;
	case Z8_REGISTER_RP:
		break;
	case Z8_REGISTER_SPH:
		break;
	case Z8_REGISTER_SPL:
		break;
	default:
		// TODO ignore missing registers
		break;
	}

	m_r[offset] = data;
}

void z8_device::register_pair_write(uint8_t offset, uint16_t data)
{
	register_write(offset, data >> 8);
	register_write(offset + 1, data & 0xff);
}

uint8_t z8_device::get_working_register(int offset)
{
	return (m_r[Z8_REGISTER_RP] & 0xf0) | (offset & 0x0f);
}

uint8_t z8_device::get_register(uint8_t offset)
{
	if ((offset & 0xf0) == 0xe0)
		return get_working_register(offset & 0x0f);
	else
		return offset;
}

uint8_t z8_device::get_intermediate_register(int offset)
{
	return register_read(get_register(offset));
}

void z8_device::stack_push_byte(uint8_t src)
{
	if (P01M & Z8_P01M_INTERNAL_STACK)
	{
		// SP <- SP - 1 (predecrement)
		uint8_t sp = register_read(Z8_REGISTER_SPL) - 1;
		register_write(Z8_REGISTER_SPL, sp);

		// @SP <- src
		register_write(sp, src);
	}
	else
	{
		// SP <- SP - 1 (predecrement)
		uint16_t sp = register_pair_read(Z8_REGISTER_SPH) - 1;
		register_pair_write(Z8_REGISTER_SPH, sp);

		// @SP <- src
		m_data->write_byte(mask_external_address(sp), src);
	}
}

void z8_device::stack_push_word(uint16_t src)
{
	if (P01M & Z8_P01M_INTERNAL_STACK)
	{
		// SP <- SP - 2 (predecrement)
		uint8_t sp = register_read(Z8_REGISTER_SPL) - 2;
		register_write(Z8_REGISTER_SPL, sp);

		// @SP <- src
		register_pair_write(sp, src);
	}
	else
	{
		// SP <- SP - 2 (predecrement)
		uint16_t sp = register_pair_read(Z8_REGISTER_SPH) - 2;
		register_pair_write(Z8_REGISTER_SPH, sp);

		// @SP <- src
		m_data->write_word_unaligned(mask_external_address(sp), src);
	}
}

uint8_t z8_device::stack_pop_byte()
{
	if (P01M & Z8_P01M_INTERNAL_STACK)
	{
		// @SP <- src
		uint8_t sp = register_read(Z8_REGISTER_SPL);
		uint8_t byte = register_read(sp);

		// SP <- SP + 1 (postincrement)
		register_write(Z8_REGISTER_SPL, sp + 1);

		return byte;
	}
	else
	{
		// @SP <- src
		uint16_t sp = register_pair_read(Z8_REGISTER_SPH);
		uint8_t byte = m_data->read_byte(mask_external_address(sp));

		// SP <- SP + 1 (postincrement)
		register_pair_write(Z8_REGISTER_SPH, sp + 1);

		return byte;
	}
}

uint16_t z8_device::stack_pop_word()
{
	if (P01M & Z8_P01M_INTERNAL_STACK)
	{
		// @SP <- src
		uint8_t sp = register_read(Z8_REGISTER_SPL);
		uint16_t word = register_pair_read(sp);

		// SP <- SP + 2 (postincrement)
		register_write(Z8_REGISTER_SPL, sp + 2);

		return word;
	}
	else
	{
		// @SP <- src
		uint16_t sp = register_pair_read(Z8_REGISTER_SPH);
		uint16_t word = m_data->read_word_unaligned(mask_external_address(sp));

		// SP <- SP + 2 (postincrement)
		register_pair_write(Z8_REGISTER_SPH, sp + 2);

		return word;
	}
}

void z8_device::set_flag(uint8_t flag, int state)
{
	if (state)
		m_r[Z8_REGISTER_FLAGS] |= flag;
	else
		m_r[Z8_REGISTER_FLAGS] &= ~flag;
}

#define set_flag_h(state)   set_flag(Z8_FLAGS_H, state);
#define set_flag_d(state)   set_flag(Z8_FLAGS_D, state);
#define set_flag_v(state)   set_flag(Z8_FLAGS_V, state);
#define set_flag_s(state)   set_flag(Z8_FLAGS_S, state);
#define set_flag_z(state)   set_flag(Z8_FLAGS_Z, state);
#define set_flag_c(state)   set_flag(Z8_FLAGS_C, state);

/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define INSTRUCTION(mnemonic) void z8_device::mnemonic(uint8_t opcode, int *cycles)

INSTRUCTION( illegal )
{
	logerror("Z8: PC = %04x, Illegal opcode = %02x\n", m_pc - 1, opcode);
}

#include "z8ops.hxx"

/***************************************************************************
    OPCODE TABLES
***************************************************************************/

const z8_device::z8_opcode_map z8_device::Z8601_OPCODE_MAP[256] =
{
	{ &z8_device::dec_R1, 6, 5 },   { &z8_device::dec_IR1, 6, 5 },  { &z8_device::add_r1_r2, 10, 5 },   { &z8_device::add_r1_Ir2, 10, 5 },
	{ &z8_device::add_R2_R1, 10, 5 },   { &z8_device::add_IR2_R1, 10, 5 },  { &z8_device::add_R1_IM, 10, 5 },   { &z8_device::add_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::rlc_R1, 6, 5 },   { &z8_device::rlc_IR1, 6, 5 },  { &z8_device::adc_r1_r2, 6, 5 },    { &z8_device::adc_r1_Ir2, 6, 5 },
	{ &z8_device::adc_R2_R1, 10, 5 },   { &z8_device::adc_IR2_R1, 10, 5 },  { &z8_device::adc_R1_IM, 10, 5 },   { &z8_device::adc_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::inc_R1, 6, 5 },   { &z8_device::inc_IR1, 6, 5 },  { &z8_device::sub_r1_r2, 6, 5 },    { &z8_device::sub_r1_Ir2, 6, 5 },
	{ &z8_device::sub_R2_R1, 10, 5 },   { &z8_device::sub_IR2_R1, 10, 5 },  { &z8_device::sub_R1_IM, 10, 5 },   { &z8_device::sub_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::jp_IRR1, 8, 0 },  { &z8_device::srp_IM, 6, 1 },   { &z8_device::sbc_r1_r2, 6, 5 },    { &z8_device::sbc_r1_Ir2, 6, 5 },
	{ &z8_device::sbc_R2_R1, 10, 5 },   { &z8_device::sbc_IR2_R1, 10, 5 },  { &z8_device::sbc_R1_IM, 10, 5 },   { &z8_device::sbc_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::da_R1, 8, 5 },    { &z8_device::da_IR1, 8, 5 },   { &z8_device::or_r1_r2, 6, 5 },     { &z8_device::or_r1_Ir2, 6, 5 },
	{ &z8_device::or_R2_R1, 10, 5 },    { &z8_device::or_IR2_R1, 10, 5 },   { &z8_device::or_R1_IM, 10, 5 },    { &z8_device::or_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::pop_R1, 10, 5 },  { &z8_device::pop_IR1, 10, 5 }, { &z8_device::and_r1_r2, 6, 5 },    { &z8_device::and_r1_Ir2, 6, 5 },
	{ &z8_device::and_R2_R1, 10, 5 },   { &z8_device::and_IR2_R1, 10, 5 },  { &z8_device::and_R1_IM, 10, 5 },   { &z8_device::and_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::com_R1, 6, 5 },   { &z8_device::com_IR1, 6, 5 },  { &z8_device::tcm_r1_r2, 6, 5 },    { &z8_device::tcm_r1_Ir2, 6, 5 },
	{ &z8_device::tcm_R2_R1, 10, 5 },   { &z8_device::tcm_IR2_R1, 10, 5 },  { &z8_device::tcm_R1_IM, 10, 5 },   { &z8_device::tcm_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::push_R2, 10, 1 }, { &z8_device::push_IR2, 12, 1 },{ &z8_device::tm_r1_r2, 6, 5 },     { &z8_device::tm_r1_Ir2, 6, 5 },
	{ &z8_device::tm_R2_R1, 10, 5 },    { &z8_device::tm_IR2_R1, 10, 5 },   { &z8_device::tm_R1_IM, 10, 5 },    { &z8_device::tm_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::illegal, 0, 0 },

	{ &z8_device::decw_RR1, 10, 5 },{ &z8_device::decw_IR1, 10, 5 },{ &z8_device::lde_r1_Irr2, 12, 0 }, { &z8_device::ldei_Ir1_Irr2, 18, 0 },
	{ &z8_device::illegal, 0, 0 },     { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::di, 6, 1 },

	{ &z8_device::rl_R1, 6, 5 },    { &z8_device::rl_IR1, 6, 5 },   { &z8_device::lde_r2_Irr1, 12, 0 }, { &z8_device::ldei_Ir2_Irr1, 18, 0 },
	{ &z8_device::illegal, 0, 0 },     { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::ei, 6, 1 },

	{ &z8_device::incw_RR1, 10, 5 },{ &z8_device::incw_IR1, 10, 5 },{ &z8_device::cp_r1_r2, 6, 5 },     { &z8_device::cp_r1_Ir2, 6, 5 },
	{ &z8_device::cp_R2_R1, 10, 5 },    { &z8_device::cp_IR2_R1, 10, 5 },   { &z8_device::cp_R1_IM, 10, 5 },    { &z8_device::cp_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::ret, 14, 0 },

	{ &z8_device::clr_R1, 6, 5 },   { &z8_device::clr_IR1, 6, 5 },  { &z8_device::xor_r1_r2, 6, 5 },    { &z8_device::xor_r1_Ir2, 6, 5 },
	{ &z8_device::xor_R2_R1, 10, 5 },   { &z8_device::xor_IR2_R1, 10, 5 },  { &z8_device::xor_R1_IM, 10, 5 },   { &z8_device::xor_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::iret, 16, 0 },

	{ &z8_device::rrc_R1, 6, 5 },   { &z8_device::rrc_IR1, 6, 5 },  { &z8_device::ldc_r1_Irr2, 12, 0 }, { &z8_device::ldci_Ir1_Irr2, 18, 0 },
	{ &z8_device::illegal, 0, 0 },     { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },      { &z8_device::ld_r1_x_R2, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::rcf, 6, 5 },

	{ &z8_device::sra_R1, 6, 5 },   { &z8_device::sra_IR1, 6, 5 },  { &z8_device::ldc_r2_Irr1, 12, 0 }, { &z8_device::ldci_Ir2_Irr1, 18, 0 },
	{ &z8_device::call_IRR1, 20, 0 },  { &z8_device::illegal, 0, 0 },      { &z8_device::call_DA, 20, 0 },     { &z8_device::ld_r2_x_R1, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::scf, 6, 5 },

	{ &z8_device::rr_R1, 6, 5 },    { &z8_device::rr_IR1, 6, 5 },   { &z8_device::illegal, 0, 0 },      { &z8_device::ld_r1_Ir2, 6, 5 },
	{ &z8_device::ld_R2_R1, 10, 5 },    { &z8_device::ld_IR2_R1, 10, 5 },   { &z8_device::ld_R1_IM, 10, 5 },    { &z8_device::ld_IR1_IM, 10, 5 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::ccf, 6, 5 },

	{ &z8_device::swap_R1, 8, 5 },  { &z8_device::swap_IR1, 8, 5 }, { &z8_device::illegal, 0, 0 },      { &z8_device::ld_Ir1_r2, 6, 5 },
	{ &z8_device::illegal, 0, 0 },      { &z8_device::ld_R2_IR1, 10, 5 },   { &z8_device::illegal, 0, 0 },      { &z8_device::illegal, 0, 0 },
	{ &z8_device::ld_r1_R2, 6, 5 }, { &z8_device::ld_r2_R1, 6, 5 }, { &z8_device::djnz_r1_RA, 10, 5 },  { &z8_device::jr_cc_RA, 10, 0 },
	{ &z8_device::ld_r1_IM, 6, 5 },     { &z8_device::jp_cc_DA, 10, 0 },    { &z8_device::inc_r1, 6, 5 },       { &z8_device::nop, 6, 0 }
};

/***************************************************************************
    TIMER CALLBACKS
***************************************************************************/

TIMER_CALLBACK_MEMBER( z8_device::t0_tick )
{
	m_t0--;

	if (m_t0 == 0)
	{
		m_t0 = T0;
		m_t0_timer->adjust(attotime::zero, 0, cycles_to_attotime(4 * ((PRE0 >> 2) + 1)));
		m_t0_timer->enable(PRE0 & Z8_PRE0_COUNT_MODULO_N);
		m_r[Z8_REGISTER_IRQ] |= Z8_IRQ_FLAG_IRQ4;
	}
}

TIMER_CALLBACK_MEMBER( z8_device::t1_tick )
{
	m_t1--;

	if (m_t1 == 0)
	{
		m_t1 = T1;
		m_t1_timer->adjust(attotime::zero, 0, cycles_to_attotime(4 * ((PRE1 >> 2) + 1)));
		m_t1_timer->enable(PRE1 & Z8_PRE0_COUNT_MODULO_N);
		m_r[Z8_REGISTER_IRQ] |= Z8_IRQ_FLAG_IRQ5;
	}
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

void z8_device::device_start()
{
	for (auto &cb : m_input_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_output_cb)
		cb.resolve_safe();

	/* set up the state table */
	{
		state_add(Z8_PC,         "PC",        m_pc);
		state_add(STATE_GENPC,   "GENPC",     m_pc).noshow();
		state_add(STATE_GENPCBASE, "CURPC",   m_ppc).noshow();
		state_add(Z8_SP,         "SP",        m_fake_sp).callimport().callexport();
		state_add(STATE_GENSP,   "GENSP",     m_fake_sp).callimport().callexport().noshow();
		state_add(Z8_RP,         "RP",        m_r[Z8_REGISTER_RP]);
		state_add(STATE_GENFLAGS, "GENFLAGS", m_r[Z8_REGISTER_FLAGS]).noshow().formatstr("%6s");
		state_add(Z8_IMR,        "IMR",       m_r[Z8_REGISTER_IMR]);
		state_add(Z8_IRQ,        "IRQ",       m_r[Z8_REGISTER_IRQ]);
		state_add(Z8_IPR,        "IPR",       m_r[Z8_REGISTER_IPR]);
		state_add(Z8_P01M,       "P01M",      m_r[Z8_REGISTER_P01M]);
		state_add(Z8_P3M,        "P3M",       m_r[Z8_REGISTER_P3M]);
		state_add(Z8_P2M,        "P2M",       m_r[Z8_REGISTER_P2M]);
		state_add(Z8_PRE0,       "PRE0",      m_r[Z8_REGISTER_PRE0]);
		state_add(Z8_T0,         "T0",        m_t0);
		state_add(Z8_PRE1,       "PRE1",      m_r[Z8_REGISTER_PRE1]);
		state_add(Z8_T1,         "T1",        m_t1);
		state_add(Z8_TMR,        "TMR",       m_r[Z8_REGISTER_TMR]);

		for (int regnum = 0; regnum < 16; regnum++)
			state_add(Z8_R0 + regnum, string_format("R%d", regnum).c_str(), m_fake_r[regnum]).callimport().callexport();
	}

	/* find address spaces */
	m_program = &space(AS_PROGRAM);
	m_cache = m_program->cache<0, 0, ENDIANNESS_BIG>();
	m_data = has_space(AS_DATA) ? &space(AS_DATA) : m_program;

	/* allocate timers */
	m_t0_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z8_device::t0_tick), this));
	m_t1_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z8_device::t1_tick), this));

	/* Clear state */
	for (auto & elem : m_irq_line)
		elem = CLEAR_LINE;
	for (auto & elem : m_r)
		elem = 0;
	for ( int i = 0; i < 4; i++ )
	{
		m_input[i] = 0;
		m_output[i] = 0;
	}
	for (auto & elem : m_fake_r)
		elem = 0;
	m_fake_sp = 0;
	m_t0 = 0;
	m_t1 = 0;

	/* register for state saving */
	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_irq_taken));

	set_icountptr(m_icount);
}

/***************************************************************************
    INTERRUPTS
***************************************************************************/

void z8_device::take_interrupt(int irq)
{
	//logerror("Taking IRQ%d (previous PC = %04X)\n", irq, m_pc);
	m_irq_taken = true;

	// disable interrupts
	m_r[Z8_REGISTER_IMR] &= ~Z8_IMR_ENABLE;

	// acknowledge the IRQ
	m_r[Z8_REGISTER_IRQ] &= ~(1 << irq);

	// get the interrupt vector address
	uint16_t vector = irq * 2;
	if (m_rom_size == 0)
		vector = mask_external_address(vector);

	// push registers onto stack
	stack_push_word(m_pc);
	stack_push_byte(m_r[Z8_REGISTER_FLAGS]);

	// branch to the vector
	m_pc = m_cache->read_byte(vector) << 8;
	m_pc |= m_cache->read_byte(vector + 1);
}

void z8_device::process_interrupts()
{
	m_irq_taken = false;
	uint8_t pending_irqs = m_r[Z8_REGISTER_IMR] & m_r[Z8_REGISTER_IRQ] & Z8_IRQ_MASK;
	if (!(m_r[Z8_REGISTER_IMR] & Z8_IMR_ENABLE) || pending_irqs == 0)
		return;

	int group_a[2] = { 5, 3 };
	int group_b[2] = { 2, 0 };
	int group_c[2] = { 1, 4 };

	if (BIT(m_r[Z8_REGISTER_IPR], 5))
		std::swap(group_a[0], group_a[1]);
	if (BIT(m_r[Z8_REGISTER_IPR], 2))
		std::swap(group_b[0], group_b[1]);
	if (BIT(m_r[Z8_REGISTER_IPR], 1))
		std::swap(group_c[0], group_c[1]);

	switch ((m_r[Z8_REGISTER_IPR] & 0x18) >> 2 | (m_r[Z8_REGISTER_IPR] & 0x01))
	{
		case 0: // (000) reserved according to Zilog (but must process at least IRQ4)
		case 1: // (001) C > A > B
			if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			break;

		case 2: // (010) A > B > C
			if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			break;

		case 3: // (011) A > C > B
			if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			break;

		case 4: // (100) B > C > A
			if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			break;

		case 5: // (101) C > B > A
			if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			else if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			break;

		case 6: // (110) B > A > C
		case 7: // (111) reserved according to Zilog
			if (BIT(pending_irqs, group_b[0]))
				take_interrupt(group_b[0]);
			else if (BIT(pending_irqs, group_b[1]))
				take_interrupt(group_b[1]);
			else if (BIT(pending_irqs, group_a[0]))
				take_interrupt(group_a[0]);
			else if (BIT(pending_irqs, group_a[1]))
				take_interrupt(group_a[1]);
			else if (BIT(pending_irqs, group_c[0]))
				take_interrupt(group_c[0]);
			else if (BIT(pending_irqs, group_c[1]))
				take_interrupt(group_c[1]);
			break;
	}
}

/***************************************************************************
    EXECUTION
***************************************************************************/

void z8_device::execute_run()
{
	do
	{
		process_interrupts();
		if (m_irq_taken)
		{
			// interrupt processing takes 58 external clock cycles
			m_icount -= 27;
		}
		else
		{
			/* fetch opcode */
			uint8_t opcode = fetch_opcode();
			int cycles = Z8601_OPCODE_MAP[opcode].execution_cycles;

			/* execute instruction */
			(this->*(Z8601_OPCODE_MAP[opcode].function))(opcode, &cycles);

			m_icount -= cycles;
		}
	}
	while (m_icount > 0);
}

/***************************************************************************
    RESET
***************************************************************************/

void z8_device::device_reset()
{
	m_pc = 0x000c;

	register_write(Z8_REGISTER_TMR, 0x00);
	register_write(Z8_REGISTER_PRE1, PRE1 & 0xfc);
	register_write(Z8_REGISTER_PRE0, PRE0 & 0xfe);
	register_write(Z8_REGISTER_P2M, 0xff);
	register_write(Z8_REGISTER_P3M, 0x00);
	register_write(Z8_REGISTER_P01M, 0x4d);
	register_write(Z8_REGISTER_IRQ, 0x00);
	register_write(Z8_REGISTER_IMR, 0x00);
	register_write(Z8_REGISTER_RP, 0x00);
}


/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

void z8_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case Z8_SP:
		case STATE_GENSP:
			m_r[Z8_REGISTER_SPH] = m_fake_sp >> 8;
			m_r[Z8_REGISTER_SPL] = m_fake_sp & 0xff;
			break;

		case Z8_R0: case Z8_R1: case Z8_R2: case Z8_R3: case Z8_R4: case Z8_R5: case Z8_R6: case Z8_R7: case Z8_R8: case Z8_R9: case Z8_R10: case Z8_R11: case Z8_R12: case Z8_R13: case Z8_R14: case Z8_R15:
			m_r[m_r[Z8_REGISTER_RP] + (entry.index() - Z8_R0)] = m_fake_r[entry.index() - Z8_R0];
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(z8) called for unexpected value\n");
	}
}

void z8_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case Z8_SP:
		case STATE_GENSP:
			m_fake_sp = (m_r[Z8_REGISTER_SPH] << 8) | m_r[Z8_REGISTER_SPL];
			break;

		case Z8_R0: case Z8_R1: case Z8_R2: case Z8_R3: case Z8_R4: case Z8_R5: case Z8_R6: case Z8_R7: case Z8_R8: case Z8_R9: case Z8_R10: case Z8_R11: case Z8_R12: case Z8_R13: case Z8_R14: case Z8_R15:
			m_fake_r[entry.index() - Z8_R0] = m_r[m_r[Z8_REGISTER_RP] + (entry.index() - Z8_R0)];
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(z8) called for unexpected value\n");
	}
}

void z8_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS: str = string_format("%c%c%c%c%c%c",
										m_r[Z8_REGISTER_FLAGS] & Z8_FLAGS_C ? 'C' : '.',
										m_r[Z8_REGISTER_FLAGS] & Z8_FLAGS_Z ? 'Z' : '.',
										m_r[Z8_REGISTER_FLAGS] & Z8_FLAGS_S ? 'S' : '.',
										m_r[Z8_REGISTER_FLAGS] & Z8_FLAGS_V ? 'V' : '.',
										m_r[Z8_REGISTER_FLAGS] & Z8_FLAGS_D ? 'D' : '.',
										m_r[Z8_REGISTER_FLAGS] & Z8_FLAGS_H ? 'H' : '.');   break;
	}
}


void z8_device::execute_set_input(int inputnum, int state)
{
	switch ( inputnum )
	{
		case INPUT_LINE_IRQ0:
			if (state != CLEAR_LINE && m_irq_line[0] == CLEAR_LINE)
				m_r[Z8_REGISTER_IRQ] |= Z8_IRQ_FLAG_IRQ0;
			m_irq_line[0] = state;
			break;

		case INPUT_LINE_IRQ1:
			if (state != CLEAR_LINE && m_irq_line[1] == CLEAR_LINE)
				m_r[Z8_REGISTER_IRQ] |= Z8_IRQ_FLAG_IRQ1;
			m_irq_line[1] = state;
			break;

		case INPUT_LINE_IRQ2:
			if (state != CLEAR_LINE && m_irq_line[2] == CLEAR_LINE)
				m_r[Z8_REGISTER_IRQ] |= Z8_IRQ_FLAG_IRQ2;
			m_irq_line[2] = state;
			break;

		case INPUT_LINE_IRQ3:
			if (state != CLEAR_LINE && m_irq_line[3] == CLEAR_LINE)
				m_r[Z8_REGISTER_IRQ] |= Z8_IRQ_FLAG_IRQ3;
			m_irq_line[3] = state;
			break;

	}
}
