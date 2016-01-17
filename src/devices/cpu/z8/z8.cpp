// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

**********************************************************************/

/*

    TODO:

    - strobed I/O
    - interrupts
    - expose register file to disassembler
    - decimal adjust instruction
    - timer Tin/Tout modes
    - serial
    - instruction pipeline

*/

#include "emu.h"
#include "debugger.h"
#include "z8.h"

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
#define Z8_P01M_P0L_MODE_A8_A11     0x02    /* not supported */
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
#define Z8_P01M_P0H_MODE_A12_A15    0x80    /* not supported */

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

#define Z8_IMR_ENABLE               0x80    /* not supported */
#define Z8_IMR_RAM_PROTECT          0x40    /* not supported */
#define Z8_IMR_ENABLE_IRQ5          0x20    /* not supported */
#define Z8_IMR_ENABLE_IRQ4          0x10    /* not supported */
#define Z8_IMR_ENABLE_IRQ3          0x08    /* not supported */
#define Z8_IMR_ENABLE_IRQ2          0x04    /* not supported */
#define Z8_IMR_ENABLE_IRQ1          0x02    /* not supported */
#define Z8_IMR_ENABLE_IRQ0          0x01    /* not supported */

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


const device_type Z8601 = &device_creator<z8601_device>;
const device_type UB8830D = &device_creator<ub8830d_device>;
const device_type Z8611 = &device_creator<z8611_device>;


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( program_2kb, AS_PROGRAM, 8, z8_device )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( program_4kb, AS_PROGRAM, 8, z8_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END


z8_device::z8_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, int size)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, ( size == 4 ) ? ADDRESS_MAP_NAME(program_4kb) : ADDRESS_MAP_NAME(program_2kb))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 2, 0)
{
}


z8601_device::z8601_device(const machine_config &mconfig, std::string _tag, device_t *_owner, UINT32 _clock)
	: z8_device(mconfig, Z8601, "Z8601", _tag, _owner, _clock, "z8601", __FILE__, 2)
{
}


ub8830d_device::ub8830d_device(const machine_config &mconfig, std::string _tag, device_t *_owner, UINT32 _clock)
	: z8_device(mconfig, UB8830D, "UB8830D", _tag, _owner, _clock, "ub8830d", __FILE__, 2)
{
}


z8611_device::z8611_device(const machine_config &mconfig, std::string _tag, device_t *_owner, UINT32 _clock)
	: z8_device(mconfig, Z8611, "Z8611", _tag, _owner, _clock, "z8611", __FILE__, 4)
{
}


offs_t z8_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( z8 );
	return CPU_DISASSEMBLE_NAME(z8)(this, buffer, pc, oprom, opram, options);
}


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

UINT8 z8_device::fetch()
{
	UINT8 data = m_direct->read_byte(m_pc);

	m_pc++;

	return data;
}


UINT8 z8_device::register_read(UINT8 offset)
{
	UINT8 data = 0xff;
	UINT8 mask = 0;

	switch (offset)
	{
	case Z8_REGISTER_P0:
		switch (P01M & Z8_P01M_P0L_MODE_MASK)
		{
		case Z8_P01M_P0L_MODE_OUTPUT:   data = m_output[offset] & 0x0f;     break;
		case Z8_P01M_P0L_MODE_INPUT:    mask = 0x0f;                                break;
		default: /* A8...A11 */         data = 0x0f;                                break;
		}

		switch (P01M & Z8_P01M_P0H_MODE_MASK)
		{
		case Z8_P01M_P0H_MODE_OUTPUT:   data |= m_output[offset] & 0xf0;    break;
		case Z8_P01M_P0H_MODE_INPUT:    mask |= 0xf0;                               break;
		default: /* A12...A15 */        data |= 0xf0;                               break;
		}

		if (!(P3M & Z8_P3M_P0_STROBED))
		{
			if (mask) m_input[offset] = m_io->read_byte(offset);
		}

		data |= m_input[offset] & mask;
		break;

	case Z8_REGISTER_P1:
		switch (P01M & Z8_P01M_P1_MODE_MASK)
		{
		case Z8_P01M_P1_MODE_OUTPUT:    data = m_output[offset];            break;
		case Z8_P01M_P1_MODE_INPUT:     mask = 0xff;                                break;
		default: /* AD0..AD7 */         data = 0xff;                                break;
		}

		if ((P3M & Z8_P3M_P33_P34_MASK) != Z8_P3M_P33_P34_DAV1_RDY1)
		{
			if (mask) m_input[offset] = m_io->read_byte(offset);
		}

		data |= m_input[offset] & mask;
		break;

	case Z8_REGISTER_P2:
		mask = m_r[Z8_REGISTER_P2M];

		if (!(P3M & Z8_P3M_P2_STROBED))
		{
			if (mask) m_input[offset] = m_io->read_byte(offset);
		}

		data = (m_input[offset] & mask) | (m_output[offset] & ~mask);
		break;

	case Z8_REGISTER_P3:
		// TODO: special port 3 modes
		if (!(P3M & 0x7c))
		{
			mask = 0x0f;
		}

		if (mask) m_input[offset] = m_io->read_byte(offset);

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

UINT16 z8_device::register_pair_read(UINT8 offset)
{
	return (register_read(offset) << 8) | register_read(offset + 1);
}

void z8_device::register_write(UINT8 offset, UINT8 data)
{
	UINT8 mask = 0;

	switch (offset)
	{
	case Z8_REGISTER_P0:
		m_output[offset] = data;
		if ((P01M & Z8_P01M_P0L_MODE_MASK) == Z8_P01M_P0L_MODE_OUTPUT) mask |= 0x0f;
		if ((P01M & Z8_P01M_P0H_MODE_MASK) == Z8_P01M_P0H_MODE_OUTPUT) mask |= 0xf0;
		if (mask) m_io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_P1:
		m_output[offset] = data;
		if ((P01M & Z8_P01M_P1_MODE_MASK) == Z8_P01M_P1_MODE_OUTPUT) mask = 0xff;
		if (mask) m_io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_P2:
		m_output[offset] = data;
		mask = m_r[Z8_REGISTER_P2M] ^ 0xff;
		if (mask) m_io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_P3:
		m_output[offset] = data;

		// TODO: special port 3 modes
		if (!(P3M & 0x7c))
		{
			mask = 0xf0;
		}

		if (mask) m_io->write_byte(offset, data & mask);
		break;

	case Z8_REGISTER_SIO:
		break;

	case Z8_REGISTER_TMR:
		if (data & Z8_TMR_LOAD_T0)
		{
			m_t0 = T0;
			m_t0_timer->adjust(attotime::zero, 0, attotime::from_hz(m_clock / 2 / 4 / ((PRE0 >> 2) + 1)));
		}

		m_t0_timer->enable(data & Z8_TMR_ENABLE_T0);

		if (data & Z8_TMR_LOAD_T1)
		{
			m_t1 = T1;
			m_t1_timer->adjust(attotime::zero, 0, attotime::from_hz(m_clock / 2 / 4 / ((PRE1 >> 2) + 1)));
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

void z8_device::register_pair_write(UINT8 offset, UINT16 data)
{
	register_write(offset, data >> 8);
	register_write(offset + 1, data & 0xff);
}

UINT8 z8_device::get_working_register(int offset)
{
	return (m_r[Z8_REGISTER_RP] & 0xf0) | (offset & 0x0f);
}

UINT8 z8_device::get_register(UINT8 offset)
{
	if ((offset & 0xf0) == 0xe0)
		return get_working_register(offset & 0x0f);
	else
		return offset;
}

UINT8 z8_device::get_intermediate_register(int offset)
{
	return register_read(get_register(offset));
}

void z8_device::stack_push_byte(UINT8 src)
{
	if (register_read(Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP - 1 */
		UINT8 sp = register_read(Z8_REGISTER_SPL) - 1;
		register_write(Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		register_write(sp, src);
	}
	else
	{
		/* SP <- SP - 1 */
		UINT16 sp = register_pair_read(Z8_REGISTER_SPH) - 1;
		register_pair_write(Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		m_data->write_byte(sp, src);
	}
}

void z8_device::stack_push_word(UINT16 src)
{
	if (register_read(Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP - 2 */
		UINT8 sp = register_read(Z8_REGISTER_SPL) - 2;
		register_write(Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		register_pair_write(sp, src);
	}
	else
	{
		/* SP <- SP - 2 */
		UINT16 sp = register_pair_read(Z8_REGISTER_SPH) - 2;
		register_pair_write(Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		m_data->write_word(sp, src);
	}
}

UINT8 z8_device::stack_pop_byte()
{
	if (register_read(Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP + 1 */
		UINT8 sp = register_read(Z8_REGISTER_SPL) + 1;
		register_write(Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		return register_read(sp);
	}
	else
	{
		/* SP <- SP + 1 */
		UINT16 sp = register_pair_read(Z8_REGISTER_SPH) + 1;
		register_pair_write(Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		return m_data->read_byte(sp);
	}
}

UINT16 z8_device::stack_pop_word()
{
	if (register_read(Z8_REGISTER_P01M) & Z8_P01M_INTERNAL_STACK)
	{
		/* SP <- SP + 2 */
		UINT8 sp = register_read(Z8_REGISTER_SPL) + 2;
		register_write(Z8_REGISTER_SPL, sp);

		/* @SP <- src */
		return register_read(sp);
	}
	else
	{
		/* SP <- SP + 2 */
		UINT16 sp = register_pair_read(Z8_REGISTER_SPH) + 2;
		register_pair_write(Z8_REGISTER_SPH, sp);

		/* @SP <- src */
		return m_data->read_word(sp);
	}
}

void z8_device::set_flag(UINT8 flag, int state)
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

#define INSTRUCTION(mnemonic) void z8_device::mnemonic(UINT8 opcode, int *cycles)

INSTRUCTION( illegal )
{
	logerror("Z8: PC = %04x, Illegal opcode = %02x\n", m_pc - 1, opcode);
}

#include "z8ops.inc"

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
		m_t0_timer->adjust(attotime::zero, 0, attotime::from_hz(m_clock / 2 / 4 / ((PRE0 >> 2) + 1)));
		m_t0_timer->enable(PRE0 & Z8_PRE0_COUNT_MODULO_N);
		m_irq[4] = ASSERT_LINE;
	}
}

TIMER_CALLBACK_MEMBER( z8_device::t1_tick )
{
	m_t1--;

	if (m_t1 == 0)
	{
		m_t1 = T1;
		m_t1_timer->adjust(attotime::zero, 0, attotime::from_hz(m_clock / 2 / 4 / ((PRE1 >> 2) + 1)));
		m_t1_timer->enable(PRE1 & Z8_PRE0_COUNT_MODULO_N);
		m_irq[5] = ASSERT_LINE;
	}
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

void z8_device::device_start()
{
	/* set up the state table */
	{
		state_add(Z8_PC,         "PC",        m_pc);
		state_add(STATE_GENPC,   "GENPC",     m_pc).noshow();
		state_add(Z8_SP,         "SP",        m_fake_sp).callimport().callexport();
		state_add(STATE_GENSP,   "GENSP",     m_fake_sp).callimport().callexport().noshow();
		state_add(Z8_RP,         "RP",        m_r[Z8_REGISTER_RP]);
		state_add(Z8_T0,         "T0",        m_t0);
		state_add(Z8_T1,         "T1",        m_t1);
		state_add(STATE_GENFLAGS, "GENFLAGS", m_r[Z8_REGISTER_FLAGS]).noshow().formatstr("%6s");

		for (int regnum = 0; regnum < 16; regnum++)
			state_add(Z8_R0 + regnum, strformat("R%d", regnum).c_str(), m_fake_r[regnum]).callimport().callexport();
	}

	/* find address spaces */
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	/* allocate timers */
	m_t0_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z8_device::t0_tick), this));
	m_t1_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z8_device::t1_tick), this));

	/* Clear state */
	for (auto & elem : m_irq)
		elem = 0;
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
	save_item(NAME(m_irq));

	m_icountptr = &m_icount;
}

/***************************************************************************
    EXECUTION
***************************************************************************/

void z8_device::execute_run()
{
	do
	{
		UINT8 opcode;
		int cycles;

		debugger_instruction_hook(this, m_pc);

		/* TODO: sample interrupts */
		m_input[3] = m_io->read_byte(3);

		/* fetch opcode */
		opcode = fetch();
		cycles = Z8601_OPCODE_MAP[opcode].execution_cycles;

		/* execute instruction */
		(this->*(Z8601_OPCODE_MAP[opcode].function))(opcode, &cycles);

		m_icount -= cycles;
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
	register_write(Z8_REGISTER_PRE1, register_read(Z8_REGISTER_PRE1) & 0xfc);
	register_write(Z8_REGISTER_PRE0, register_read(Z8_REGISTER_PRE0) & 0xfe);
	register_write(Z8_REGISTER_P2M, 0xff);
	register_write(Z8_REGISTER_P3M, 0x00);
	register_write(Z8_REGISTER_P01M, 0x4d);
	register_write(Z8_REGISTER_IRQ, 0x00);
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
		case Z8_GENSP:
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
		case Z8_GENSP:
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
		case STATE_GENFLAGS: strprintf(str, "%c%c%c%c%c%c",
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
			m_irq[0] = state;
			break;

		case INPUT_LINE_IRQ1:
			m_irq[1] = state;
			break;

		case INPUT_LINE_IRQ2:
			m_irq[2] = state;
			break;

		case INPUT_LINE_IRQ3:
			m_irq[3] = state;
			break;

	}
}
