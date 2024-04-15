// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM PALM (apparently "Put All Logic in Microcode") single-card microprocessor.
 *
 * Sources:
 *  - IBM 5100 Maintenance Information Manual, SY31-0405-3, Fourth Edition (October 1979), International Business Machines Corporation
 *
 * TODO:
 *  - machine check
 *  - instruction timing
 */

#include "emu.h"
#include "palm.h"
#include "palmd.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

 // helpers for IBM bit numbering
template <typename T, typename U> constexpr T IBIT(T x, U n) noexcept
{
	return BIT(x, sizeof(T) * 8 - n - 1);
}

template <typename T, typename U, typename V> constexpr T IBIT(T x, U n, V w)
{
	return BIT(x, sizeof(T) * 8 - n - w, w);
}

static u8 constexpr il_priority[] = { 0, 1, 2, 2, 3, 3, 3, 3 };

DEFINE_DEVICE_TYPE(PALM, palm_device, "palm", "IBM PALM")

palm_device::palm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, PALM, tag, owner, clock)
	, m_pgm_config("pgm", ENDIANNESS_BIG, 16, 16)
	, m_rws_config("rws", ENDIANNESS_BIG, 16, 16)
	, m_ioc_config("ioc", ENDIANNESS_BIG, 8, 4)
	, m_iod_config("iod", ENDIANNESS_BIG, 8, 4)
	, m_getb_bus(*this)
	, m_program_level(*this)
	, m_select_ros(*this)
	, m_icount(0)
	, m_r{}
{
}

enum ff_mask : u8
{
	FF_IPL = 0x80, // initial program load
	FF_MSS = 0x40, // microprogram storage switch

	FF_IE  = 0x08, // interrupt enable
	FF_IR3 = 0x04, // interrupt request 3
	FF_IR2 = 0x02, // interrupt request 2
	FF_IR1 = 0x01, // interrupt request 1

	FF_IR  = 0x07, // interrupt request
};

void palm_device::device_start()
{
	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add<u16>(STATE_GENPCBASE, "CURPC", [this]() { return m_r[m_il][0] & 0xfffeU; });

	state_add(0, "IL", m_il);
	for (unsigned i = 0; i < std::size(m_r[m_il]); i++)
		state_add<u16>(i + 1, util::string_format("R%d", i).c_str(), [this, i]() { return m_r[m_il][i]; }, [this, i](u16 data) { m_r[m_il][i] = data; });

	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_il));
	save_item(NAME(m_ff));

	space(AS_PGM).specific(m_pgm);
	space(AS_RWS).specific(m_rws);
	space(AS_IOC).specific(m_ioc);
	space(AS_IOD).specific(m_iod);
}

void palm_device::device_reset()
{
	space(AS_RWS).install_ram(0, sizeof(m_r) - 1, m_r);

	// select instruction source
	m_ff = FF_IPL | FF_MSS;
	m_select_ros((m_ff & FF_MSS) && !(m_ff & FF_IPL));

	// read initial PC from program memory
	m_il = 0;
	m_program_level(0);
	m_pc = m_r[m_il][0] = m_pgm.read_word(0);
}

#define Rx  r[IBIT(op, 4, 4)]
#define Ry  r[IBIT(op, 8, 4)]
#define DA  IBIT(op, 4, 4)
#define IMM IBIT(op, 8, 8)
#define MOD IBIT(op, 12, 4)

void palm_device::execute_run()
{
	while (m_icount > 0)
	{
		// handle pending interrupts
		u8 const il = il_priority[m_ff & FF_IR];
		if ((m_ff & FF_IE) && m_il != il)
		{
			m_il = il;
			m_program_level(m_il > 0);

			// notify the debugger
			if (m_il && (machine().debug_flags & DEBUG_FLAG_ENABLED))
				debug()->interrupt_hook(m_il - 1, m_r[m_il][0] & ~1);
		}

		// select active register bank
		u16 (&r)[16] = m_r[m_il];

		// notify the debugger
		debugger_instruction_hook(r[0] & ~1);

		// fetch instruction
		u16 const op = m_pgm.read_word(r[0] & ~1);

		// increment instruction address register
		r[0] += 2;

		switch (IBIT(op, 0, 4))
		{
		case 0x0:
			switch (IBIT(op, 12, 4))
			{
			case 0x0: Rx = Ry - 2; break; // move minus 2
			case 0x1: Rx = Ry - 1; break; // move minus 1
			case 0x2: Rx = Ry + 1; break; // move plus 1
			case 0x3: Rx = Ry + 2; break; // move plus 2
			case 0x4: Rx = Ry; break; // move register
			case 0x5: Rx &= 0xff00U | u8(Ry); break; // and byte
			case 0x6: Rx |= u8(Ry); break; // or byte
			case 0x7: Rx ^= u8(Ry); break; // xor byte
			case 0x8: Rx += u8(Ry); break; // add
			case 0x9: Rx -= u8(Ry); break; // subtract
			case 0xa: Rx = u8(Ry) + (Rx >> 8); break; // add special #1
			case 0xb: Rx = u8(Ry) + (Rx >> 8) - 0x100; break; // add special #2
			case 0xc: Rx = (Rx & 0xff00U) | (Ry >> 8); break; // high to low
			case 0xd: Rx = (Rx & 0x00ffU) | (Ry << 8); break; // low to high
			case 0xe: Ry = (Ry & 0xff00U) | m_iod.read_byte(DA); break; // get to register
			case 0xf: Ry += (7 - count_leading_ones_32(u32(m_iod.read_byte(DA)) << 24)) * 2; break; // get to register and add
			}
			break;
		case 0x1:
			// control
			if (DA == 0)
				control(IMM);

			m_ioc.write_byte(DA, IMM);
			break;
		case 0x2: Rx = m_rws.read_word(IMM * 2); break; // load halfword direct
		case 0x3: m_rws.write_word(IMM * 2, Rx); break; // store halfword direct
		case 0x4:
			// put byte
			m_iod.write_byte(DA, m_rws.read_byte(Ry));
			Ry += modifier(MOD);
			break;
		case 0x5:
			// store halfword indirect
			m_rws.write_word(Ry, Rx);
			Ry += modifier(MOD);
			break;
		case 0x6:
			// load byte indirect
			Rx = m_rws.read_byte(Ry);
			Ry += modifier(MOD);
			break;
		case 0x7:
			// store byte indirect
			m_rws.write_byte(Ry, Rx);
			Ry += modifier(MOD);
			break;
		case 0x8: Rx = (Rx & 0xff00U) | IMM; break; // emit byte
		case 0x9: Rx &= ~IMM; break; // clear immediate
		case 0xa: Rx += IMM + 1; break; // add immediate
		case 0xb: Rx |= IMM; break; // set immediate
		case 0xc:
			// jump (skip next on condition)
			if (condition(MOD, Rx, Ry))
				r[0] += 2;
			else
				r[1] = r[0] + 2;
			break;
		case 0xd:
			// load halfword indirect
			Rx = m_rws.read_word(Ry);
			Ry += modifier(MOD);
			break;
		case 0xe:
			if (DA == 0 && MOD >= 0xc)
			{
				switch (MOD)
				{
				case 0xc: Ry = (Ry & 0xff00U) | u8(Ry >> 1); break; // shift right 1
				case 0xd: Ry = (Ry & 0xff00U) | u8(Ry << 7) | u8(Ry) >> 1; break; // shift right and rotate 1
				case 0xe: Ry = (Ry & 0xff00U) | u8(Ry << 5) | u8(Ry) >> 3; break; // shift right and rotate 3
				case 0xf: Ry = (Ry & 0xff00U) | u8(Ry << 4) | u8(Ry) >> 4; break; // shift right and rotate 4
				}
			}
			else
			{
				if (MOD < 0xc)
				{
					// get byte
					m_rws.write_byte(Ry, m_iod.read_byte(DA));
					Ry += modifier(MOD);
				}
				else
				{
					m_getb_bus(DA, Ry);

					// get register byte
					Ry = (Ry & 0xff00U) | m_ioc.read_byte(DA);
				}
			}
			break;
		case 0xf: Rx -= IMM + 1; break; // subtract immediate
		}

		// TODO: average instruction time quoted as 1.75µs (~27 machine cycles)
		m_icount -= 27;
		m_pc = r[0];
	}
}

void palm_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case INPUT_LINE_NMI:
		// TODO: machine check
		break;

	default:
		// interrupt lines are active low
		if (!state)
			m_ff |= 1U << irqline;
		else
			m_ff &= ~(1U << irqline);
		break;
	}
}

device_memory_interface::space_config_vector palm_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PGM, &m_pgm_config),
		std::make_pair(AS_RWS, &m_rws_config),
		std::make_pair(AS_IOC, &m_ioc_config),
		std::make_pair(AS_IOD, &m_iod_config),
	};
}

std::unique_ptr<util::disasm_interface> palm_device::create_disassembler()
{
	return std::make_unique<palm_disassembler>();
}

bool palm_device::condition(unsigned const modifier, u16 const data, u8 const mask) const
{
	switch (modifier)
	{
	case 0x0: return u8(data) <= mask;               // le: low or equal
	case 0x1: return u8(data) < mask;                // lo: low
	case 0x2: return u8(data) == mask;               // eq: equal
	case 0x3: return u8(data) == 0x00;               // no: no ones
	case 0x4: return u8(data) == 0xff;               // all: all ones
	case 0x5: return u8(data & mask) == mask;        // allm: all masked
	case 0x6: return u8(data & mask) == 0x00;        // nom: no ones masked
	case 0x7: return u8((data >> 8) & mask) == mask; // ham: high all masked
	case 0x8: return u8(data) > mask;                // hi: high
	case 0x9: return u8(data) >= mask;               // he: high or equal
	case 0xa: return u8(data) != mask;               // hl: high or low
	case 0xb: return u8(data) != 0x00;               // sb: some bits
	case 0xc: return u8(data) != 0xff;               // sn: some not ones
	case 0xd: return u8(data & mask) != mask;        // snm: some not masked
	case 0xe: return u8(data & mask) != 0x00;        // sm: some masked
	case 0xf: return u8((data >> 8) & mask) != mask; // hsnm: high some bit not masked
	default:
		// can't happen
		abort();
	}
}

s16 palm_device::modifier(unsigned const modifier) const
{
	if (modifier < 4)
		return (modifier & 3) + 1;
	else if (modifier < 8)
		return -((modifier & 3) + 1);
	else
		return 0;
}

void palm_device::control(u8 data)
{
	LOG("control 0x%02x (%s)\n", data, machine().describe_context());

	// 0: reset controller errors

	// 1: 0=disable interrupts
	// 2: 0=enable interrupts
	if (!IBIT(data, 1))
		m_ff &= ~FF_IE;
	else if (!IBIT(data, 2))
		m_ff |= FF_IE;

	// 5: state transition
	if (!IBIT(data, 5))
	{
		m_ff &= ~FF_IPL;
		m_ff ^= FF_MSS;

		m_select_ros((m_ff & FF_MSS) && !(m_ff & FF_IPL));
	}
}
