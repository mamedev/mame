// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************
 *
 *  Implementation of ASIC65
 *
 *************************************/

#include "emu.h"
#include "asic65.h"

#include <algorithm>

#define LOG_ASIC        (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

#define PARAM_WRITE     0
#define COMMAND_WRITE   1
#define DATA_READ       2

#define OP_UNKNOWN      0
#define OP_REFLECT      1
#define OP_CHECKSUM     2
#define OP_VERSION      3
#define OP_RAMTEST      4
#define OP_RESET        5
#define OP_SIN          6
#define OP_COS          7
#define OP_ATAN         8
#define OP_TMATRIXMULT  9
#define OP_MATRIXMULT   10
#define OP_TRANSFORM    11
#define OP_YORIGIN      12
#define OP_INITBANKS    13
#define OP_SETBANK      14
#define OP_VERIFYBANK   15

#define MAX_COMMANDS    0x2b

static const u8 command_map[3][MAX_COMMANDS] =
{
	{
		/* standard version */
		OP_UNKNOWN,     OP_REFLECT,     OP_CHECKSUM,    OP_VERSION,     /* 00-03 */
		OP_RAMTEST,     OP_UNKNOWN,     OP_UNKNOWN,     OP_RESET,       /* 04-07 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 08-0b */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_TMATRIXMULT, OP_UNKNOWN,     /* 0c-0f */
		OP_MATRIXMULT,  OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 10-13 */
		OP_SIN,         OP_COS,         OP_YORIGIN,     OP_TRANSFORM,   /* 14-17 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 18-1b */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 1c-1f */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 20-23 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 24-27 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN                      /* 28-2a */
	},
	{
		/* Steel Talons version */
		OP_UNKNOWN,     OP_REFLECT,     OP_CHECKSUM,    OP_VERSION,     /* 00-03 */
		OP_RAMTEST,     OP_UNKNOWN,     OP_UNKNOWN,     OP_RESET,       /* 04-07 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 08-0b */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 0c-0f */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 10-13 */
		OP_TMATRIXMULT, OP_UNKNOWN,     OP_MATRIXMULT,  OP_UNKNOWN,     /* 14-17 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 18-1b */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_SIN,         OP_COS,         /* 1c-1f */
		OP_ATAN,        OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 20-23 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 24-27 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN                      /* 28-2a */
	},
	{
		/* Guardians version */
		OP_UNKNOWN,     OP_REFLECT,     OP_CHECKSUM,    OP_VERSION,     /* 00-03 */
		OP_RAMTEST,     OP_UNKNOWN,     OP_UNKNOWN,     OP_RESET,       /* 04-07 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 08-0b */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_INITBANKS,   OP_SETBANK,     /* 0c-0f */
		OP_VERIFYBANK,  OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 10-13 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 14-17 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 18-1b */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 1c-1f */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 20-23 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN,     /* 24-27 */
		OP_UNKNOWN,     OP_UNKNOWN,     OP_UNKNOWN                      /* 28-2a */
	}
};

DEFINE_DEVICE_TYPE(ASIC65, asic65_device, "asic65", "Atari ASIC65")

asic65_device::asic65_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ASIC65, tag, owner, clock)
	, m_synced_write_timer(nullptr)
	, m_asic65_type(0)
	, m_command(0)
	, m_yorigin(0x1800)
	, m_param_index(0)
	, m_result_index(0)
	, m_reset_state(0)
	, m_last_bank(0)
	, m_ourcpu(*this, "asic65cpu")
	, m_tfull(0)
	, m_68full(0)
	, m_cmd(0)
	, m_xflg(0)
	, m_68data(0)
	, m_tdata(0)
{
	std::fill(std::begin(m_param), std::end(m_param), 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void asic65_device::device_start()
{
	save_item(NAME(m_asic65_type));
	save_item(NAME(m_command));
	save_item(NAME(m_yorigin));
	save_item(NAME(m_param_index));
	save_item(NAME(m_result_index));
	save_item(NAME(m_reset_state));
	save_item(NAME(m_last_bank));
	save_item(NAME(m_tfull));
	save_item(NAME(m_68full));
	save_item(NAME(m_cmd));
	save_item(NAME(m_xflg));
	save_item(NAME(m_68data));
	save_item(NAME(m_tdata));
	save_item(NAME(m_param));

	m_synced_write_timer = timer_alloc(FUNC(asic65_device::synced_write), this);
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void asic65_device::device_reset()
{
}

void asic65_device::reset_line(int state)
{
	/* rom-based means reset and clear states */
	if (m_asic65_type == ASIC65_ROMBASED)
		m_ourcpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);

	/* otherwise, do it manually */
	else
	{
		m_ourcpu->suspend(SUSPEND_REASON_DISABLE, 1);

		/* if reset is being signalled, clear everything */
		if (state && !m_reset_state)
			m_command = -1;

		/* if reset is going high, latch the command */
		else if (!state && m_reset_state)
		{
			if (m_command != -1)
				data_w(1, m_command);
		}

		/* update the state */
		m_reset_state = state;
	}
}

/*************************************
 *
 *  Handle writes to the chip
 *
 *************************************/

TIMER_CALLBACK_MEMBER(asic65_device::synced_write)
{
	m_tfull = 1;
	m_cmd = param >> 16;
	m_tdata = param;
	if (m_asic65_type == ASIC65_ROMBASED)
		m_ourcpu->set_input_line(0, ASSERT_LINE);
}


void asic65_device::data_w(offs_t offset, u16 data)
{
	/* rom-based use a deferred write mechanism */
	if (m_asic65_type == ASIC65_ROMBASED)
	{
		m_synced_write_timer->adjust(attotime::zero, data | (offset << 16));
		machine().scheduler().perfect_quantum(attotime::from_usec(20));
		return;
	}

	/* parameters go to offset 0 */
	if (!(offset & 1))
	{
		LOGMASKED(LOG_ASIC, "%s: Write %04X", machine().describe_context(), data);

		/* add to the parameter list, but don't overflow */
		m_param[m_param_index++] = data;
		if (m_param_index >= 32)
			m_param_index = 32;
	}

	/* commands go to offset 2 */
	else
	{
		int command = (data < MAX_COMMANDS) ? command_map[m_asic65_type][data] : OP_UNKNOWN;
		LOGMASKED(LOG_ASIC, "%s: Command %c %04X", machine().describe_context(), (command == OP_UNKNOWN) ? '*' : ' ', data);

		/* set the command number and reset the parameter/result indices */
		m_command = data;
		m_result_index = m_param_index = 0;
	}
}


u16 asic65_device::read()
{
	s64 element, result64 = 0;
	u16 result = 0;

	/* rom-based just returns latched data */
	if (m_asic65_type == ASIC65_ROMBASED)
	{
		if (!machine().side_effects_disabled())
		{
			m_68full = 0;
			machine().scheduler().perfect_quantum(attotime::from_usec(5));
		}
		return m_68data;
	}

	int command = (m_command < MAX_COMMANDS) ? command_map[m_asic65_type][m_command] : OP_UNKNOWN;

	/* update results */
	switch (command)
	{
		case OP_UNKNOWN:    /* return bogus data */
			if (!machine().side_effects_disabled())
				popmessage("ASIC65: Unknown cmd %02X", m_command);
			break;

		case OP_REFLECT:    /* reflect data */
			if (m_param_index >= 1)
				result = m_param[--m_param_index];
			break;

		case OP_CHECKSUM:   /* compute checksum (should be XX27) */
			result = 0x0027;
			break;

		case OP_VERSION:    /* get version (returns 1.3) */
			result = 0x0013;
			break;

		case OP_RAMTEST:    /* internal RAM test (result should be 0) */
			result = 0;
			break;

		case OP_RESET:  /* reset */
			if (!machine().side_effects_disabled())
				m_result_index = m_param_index = 0;
			break;

		case OP_SIN:    /* sin */
			if (m_param_index >= 1)
				result = (int)(16384. * sin(M_PI * (double)(s16)m_param[0] / 32768.));
			break;

		case OP_COS:    /* cos */
			if (m_param_index >= 1)
				result = (int)(16384. * cos(M_PI * (double)(s16)m_param[0] / 32768.));
			break;

		case OP_ATAN:   /* vector angle */
			if (m_param_index >= 4)
			{
				s32 xint = (s32)((m_param[0] << 16) | m_param[1]);
				s32 yint = (s32)((m_param[2] << 16) | m_param[3]);
				double a = atan2((double)yint, (double)xint);
				result = (s16)(a * 32768. / M_PI);
			}
			break;

		case OP_TMATRIXMULT:    /* matrix multiply by transpose */
			/* if this is wrong, the labels on the car selection screen */
			/* in Race Drivin' will be off */
			if (m_param_index >= 9+6)
			{
				s32 v0 = (s32)((m_param[9] << 16) | m_param[10]);
				s32 v1 = (s32)((m_param[11] << 16) | m_param[12]);
				s32 v2 = (s32)((m_param[13] << 16) | m_param[14]);

				/* 2 results per element */
				switch (m_result_index / 2)
				{
					case 0:
						result64 = (s64)v0 * (s16)m_param[0] +
									(s64)v1 * (s16)m_param[3] +
									(s64)v2 * (s16)m_param[6];
						break;

					case 1:
						result64 = (s64)v0 * (s16)m_param[1] +
									(s64)v1 * (s16)m_param[4] +
									(s64)v2 * (s16)m_param[7];
						break;

					case 2:
						result64 = (s64)v0 * (s16)m_param[2] +
									(s64)v1 * (s16)m_param[5] +
									(s64)v2 * (s16)m_param[8];
						break;
				}

				/* remove lower 14 bits and pass back either upper or lower words */
				result64 >>= 14;
				result = (m_result_index & 1) ? (result64 & 0xffff) : ((result64 >> 16) & 0xffff);
				if (!machine().side_effects_disabled())
					m_result_index++;
			}
			break;

		case OP_MATRIXMULT: /* matrix multiply???? */
			if (m_param_index >= 9+6)
			{
				s32 v0 = (s32)((m_param[9] << 16) | m_param[10]);
				s32 v1 = (s32)((m_param[11] << 16) | m_param[12]);
				s32 v2 = (s32)((m_param[13] << 16) | m_param[14]);

				/* 2 results per element */
				switch (m_result_index / 2)
				{
					case 0:
						result64 = (s64)v0 * (s16)m_param[0] +
									(s64)v1 * (s16)m_param[1] +
									(s64)v2 * (s16)m_param[2];
						break;

					case 1:
						result64 = (s64)v0 * (s16)m_param[3] +
									(s64)v1 * (s16)m_param[4] +
									(s64)v2 * (s16)m_param[5];
						break;

					case 2:
						result64 = (s64)v0 * (s16)m_param[6] +
									(s64)v1 * (s16)m_param[7] +
									(s64)v2 * (s16)m_param[8];
						break;
				}

				/* remove lower 14 bits and pass back either upper or lower words */
				result64 >>= 14;
				result = (m_result_index & 1) ? (result64 & 0xffff) : ((result64 >> 16) & 0xffff);
				if (!machine().side_effects_disabled())
					m_result_index++;
			}
			break;

		case OP_YORIGIN:
			if (m_param_index >= 1)
			{
				if (!machine().side_effects_disabled())
					m_yorigin = m_param[m_param_index - 1];
			}
			break;

		case OP_TRANSFORM:  /* 3d transform */
			if (m_param_index >= 2)
			{
				/* param 0 == 1/z */
				/* param 1 == height */
				/* param 2 == X */
				/* param 3 == Y */
				/* return 0 == scale factor for 1/z */
				/* return 1 == transformed X */
				/* return 2 == transformed Y, taking height into account */
				element = (s16)m_param[0];
				if (m_param_index == 2)
				{
					result64 = (element * (s16)m_param[1]) >> 8;
					result64 -= 1;
					if (result64 > 0x3fff) result64 = 0;
				}
				else if (m_param_index == 3)
				{
					result64 = (element * (s16)m_param[2]) >> 15;
					result64 += 0xa8;
				}
				else if (m_param_index == 4)
				{
					result64 = (s16)((element * (s16)m_param[3]) >> 10);
					result64 = (s16)m_yorigin - result64 - (result64 << 1);
				}
				result = result64 & 0xffff;
			}
			break;

		case OP_INITBANKS:  /* initialize banking */
			if (!machine().side_effects_disabled())
				m_last_bank = 0;
			break;

		case OP_SETBANK:    /* set a bank */
		{
			static const u8 banklist[] =
			{
				1,4,0,4,4,3,4,2, 4,4,4,4,4,4,4,4,
				3,3,4,4,1,1,0,0, 4,4,4,4,2,2,4,4,
				4,4
			};
			static const u16 bankaddr[][8] =
			{
				{ 0x77c0,0x77ce,0x77c2,0x77cc,0x77c4,0x77ca,0x77c6,0x77c8 },
				{ 0x77d0,0x77de,0x77d2,0x77dc,0x77d4,0x77da,0x77d6,0x77d8 },
				{ 0x77e0,0x77ee,0x77e2,0x77ec,0x77e4,0x77ea,0x77e6,0x77e8 },
				{ 0x77f0,0x77fe,0x77f2,0x77fc,0x77f4,0x77fa,0x77f6,0x77f8 },
				{ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000 },
			};
			if (m_param_index >= 1)
			{
				if (m_param_index < sizeof(banklist) && banklist[m_param[0]] < 4)
				{
					if (!machine().side_effects_disabled())
						m_last_bank = banklist[m_param[0]];
				}
				result = bankaddr[m_last_bank][(m_result_index < 8) ? m_result_index : 7];
				if (!machine().side_effects_disabled())
					m_result_index++;
			}
			break;
		}

		case OP_VERIFYBANK: /* verify a bank */
		{
			static const u16 bankverify[] =
			{
				0x0eb2,0x1000,0x171b,0x3d28
			};
			result = bankverify[m_last_bank];
			break;
		}
	}

	LOGMASKED(LOG_ASIC, "%s: Read %04X", machine().describe_context(), result);

	return result;
}


u16 asic65_device::io_r()
{
	if (m_asic65_type == ASIC65_ROMBASED)
	{
		/* bit 15 = TFULL */
		/* bit 14 = 68FULL */
		/* bit 13 = XFLG */
		/* bit 12 = controlled by jumper */
		if (!machine().side_effects_disabled())
			machine().scheduler().perfect_quantum(attotime::from_usec(5));
		return (m_tfull << 15) | (m_68full << 14) | (m_xflg << 13) | 0x0000;
	}
	else
	{
		/* indicate that we always are ready to accept data and always ready to send */
		return 0x4000;
	}
}



/*************************************
 *
 *  Read/write handlers for TMS320C15
 *
 *************************************/

void asic65_device::m68k_w(u16 data)
{
	m_68full = 1;
	m_68data = data;
}


u16 asic65_device::m68k_r()
{
	if (!machine().side_effects_disabled())
	{
		m_tfull = 0;
		if (m_asic65_type == ASIC65_ROMBASED)
			m_ourcpu->set_input_line(0, CLEAR_LINE);
	}
	return m_tdata;
}


void asic65_device::stat_w(u16 data)
{
	m_xflg = data & 1;
}


u16 asic65_device::stat_r()
{
	/* bit 15 = 68FULL */
	/* bit 14 = TFULL */
	/* bit 13 = CMD */
	/* bit 12 = controlled by jumper (0 = test?) */
	return (m_68full << 15) | (m_tfull << 14) | (m_cmd << 13) | 0x1000;
}


int asic65_device::get_bio()
{
	if (!machine().side_effects_disabled())
	{
		if (!m_tfull)
			m_ourcpu->spin_until_interrupt();
	}
	return m_tfull ? CLEAR_LINE : ASSERT_LINE;
}



/*************************************
 *
 *  Address maps for TMS320C15
 *
 *************************************/

void asic65_device::asic65_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0xfff).rom();
}

void asic65_device::asic65_io_map(address_map &map)
{
	map(0, 0).mirror(6).rw(FUNC(asic65_device::m68k_r), FUNC(asic65_device::m68k_w));
	map(1, 1).mirror(6).rw(FUNC(asic65_device::stat_r), FUNC(asic65_device::stat_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void asic65_device::device_add_mconfig(machine_config &config)
{
	/* ASIC65 */
	TMS320C10(config, m_ourcpu, 20'000'000);
	m_ourcpu->set_addrmap(AS_PROGRAM, &asic65_device::asic65_program_map);
	m_ourcpu->set_addrmap(AS_IO, &asic65_device::asic65_io_map);
	m_ourcpu->bio().set(FUNC(asic65_device::get_bio));
}



/***********************************************************************

    Information about various versions:

    Notation:
        C = command write
        W = write
        R = read
        7 = wait for bit 7
        6 = wait for bit 6

    Guardians of the Hood:
        Version = 0040
        Checksum = ????

        Command $08: C7
        Command $09: C7W6R
        Command $0a: C7
        Command $0b: C7W7W7W7W7WW7W7W7W
        Command $0c: C7W7W7W7W7W7W
        Command $0d: C7W7W
        Command $0e: C7W7W7W7W7WW7W7W7WWWWWWW6R6R6R6R6R6R
        Command $0f: C7W7W7W7W7W7W6R6R6R6R6R6R
        Command $10: C7W7W7W7W7WW7W7W7WWWWWWW6R6R6R6R6R6R
        Command $11: C7W7W7W7W7W7W6R6R6R6R6R6R
        Command $12: C7W7W7W7W7WW7W7W7W6R6R6R6R6R6R6R6R6R
        Command $13: C7W7W7W7W7WW7W7W7W
        Command $14: C7W6R
        Command $15: C7W6R
        Command $16: C7W6R6R
        Command $17: C7W
        Command $18: C76R6R6R6R6R6R6R6R6R
        Command $19: C7
        Command $1a: C76R6R6R6R6R6R6R6R6R
        Command $1b: C7
        Command $1c: C76R6R6R6R6R6R6R6R6R
        Command $1d: C7
        Command $1e: C76R6R6R6R6R6R6R6R6R
        Command $1f: C7
        Command $20: C76R6R6R6R6R6R6R6R6R
        Command $21: C7
        Command $22: C76R6R6R6R6R6R6R6R6R
        Command $23: C7
        Command $24: C7

        Command $0e: C76RRRRRRRRR
        Command $0f: C7W6RRRRRRRR
        Command $16: C7W
        Command $17: C7W7W7W7W6R6R6R

    Road Riot 4WD:
        Version = ????
        Checksum = ????

        Command $08: C7
        Command $09: C7W6R
        Command $0a: C7
        Command $0b: C7W7WWWWWWWW
        Command $0c: C7W7WWWWW
        Command $0d: C7W7W
        Command $0e: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $0f: C7W7WWWWW6RRRRRR
        Command $10: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $11: C7W7WWWWW6RRRRRR
        Command $12: C7W7WWWWWWWW6RRRRRRRRR
        Command $13: C7W7WWWWWWWW
        Command $14: C7W6R
        Command $15: C7W6R
        Command $16: C7W6RR
        Command $17: C7W
        Command $18: C76RRRRRRRRR
        Command $19: C7
        Command $1a: C76RRRRRRRRR
        Command $1b: C7
        Command $1c: C76RRRRRRRRR
        Command $1d: C7
        Command $1e: C76RRRRRRRRR
        Command $1f: C7
        Command $20: C76RRRRRRRRR
        Command $21: C7
        Command $22: C76RRRRRRRRR
        Command $23: C7
        Command $24: C7

        Command $16: C7W
        Command $17: C7W7WWW6RRR
        Command $17: C7WRWRWR

    Race Drivin':
        Version = ????
        Checksum = ????

        Command $08: C7
        Command $09: C7W6R
        Command $0a: C7
        Command $0b: C7W7WWWWWWWW
        Command $0c: C7W7WWWWW
        Command $0d: C7W7W
        Command $0e: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $0f: C7W7WWWWW6RRRRRR
        Command $10: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $11: C7W7WWWWW6RRRRRR
        Command $12: C7W7WWWWWWWW6RRRRRRRRR
        Command $13: C7W7WWWWWWWW
        Command $14: C7W6R
        Command $15: C7W6R
        Command $16: C7W6RR
        Command $17: C7W
        Command $18: C76RRRRRRRRR
        Command $19: C7
        Command $1a: C76RRRRRRRRR
        Command $1b: C7
        Command $1c: C76RRRRRRRRR
        Command $1d: C7
        Command $1e: C76RRRRRRRRR
        Command $1f: C7
        Command $20: C76RRRRRRRRR
        Command $21: C7
        Command $22: C76RRRRRRRRR
        Command $23: C7
        Command $24: C7

        Command $0e: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $14: C7W6R
        Command $15: C7W6R

    Steel Talons:
        Version = ????
        Checksum = ????

        Command $08: C7
        Command $09: C7W6R
        Command $0a: C7
        Command $0b: C7W7WWWWWWWW
        Command $0c: C7W7WWWWW
        Command $0d: C7W7W
        Command $14: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $15: C7W7WWWWW6RRRRRR
        Command $16: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $17: C7W7WWWWW6RRRRRR
        Command $12: C7W7WWWWWWWW6RRRRRRRRR
        Command $13: C7W7WWWWWWWW
        Command $1e: C7W6R
        Command $1f: C7W6R
        Command $16: C7W6RR
        Command $17: C7W
        Command $20: C7W7WWW6R
        Command $18: C76RRRRRRRRR
        Command $19: C7
        Command $1a: C76RRRRRRRRR
        Command $1b: C7
        Command $1c: C76RRRRRRRRR
        Command $1d: C7
        Command $1e: C76RRRRRRRRR
        Command $1f: C7
        Command $20: C76RRRRRRRRR
        Command $21: C7
        Command $22: C76RRRRRRRRR
        Command $23: C7
        Command $24: C7

    Hard Drivin's Airborne:
        Version = ????
        Checksum = ????

        Command $0e: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $14: C7W6R

        Command $08: C7
        Command $09: C7W6R
        Command $0a: C7
        Command $0b: C7W7WWWWWWWW
        Command $0c: C7W7WWWWW
        Command $0d: C7W7W
        Command $0e: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $0f: C7W7WWWWW6RRRRRR
        Command $10: C7W7WWWWWWWWWWWWWW6RRRRRR
        Command $11: C7W7WWWWW6RRRRRR
        Command $12: C7W7WWWWWWWW6RRRRRRRRR
        Command $13: C7W7WWWWWWWW
        Command $14: C7W6R
        Command $15: C7W6R
        Command $16: C7W6RR
        Command $17: C7W
        Command $18: C76RRRRRRRRR
        Command $19: C7
        Command $1a: C76RRRRRRRRR
        Command $1b: C7
        Command $1c: C76RRRRRRRRR
        Command $1d: C7
        Command $1e: C76RRRRRRRRR
        Command $1f: C7
        Command $20: C76RRRRRRRRR
        Command $21: C7
        Command $22: C76RRRRRRRRR
        Command $23: C7
        Command $24: C7
        Command $26: C7W
        Command $27: C7W7WWWWW
        Command $28: C7
        Command $2a: C76RRRRRRRRR

***********************************************************************/
