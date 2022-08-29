// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Am9516 Universal DMA Controller (UDC)
 *
 * Am9516A appears to be identical to the Am9516 but available in faster
 * clock speeds.
 *
 * It seems likely that the Am9516 was derived from the Z8016. The latter
 * supports Z80-style interrupt daisy chains, and has some additional
 * functionality for dealing with Z8000-family segmented memory.
 *
 *
 * Sources:
 *  - Personal Computer Products Data Book, © 1989 Advanced Micro Devices
 *
 * TODO:
 *  - search modes
 *  - chain load abort
 *  - hardware masks
 */

#include "emu.h"
#include "am9516.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REGR    (1U << 1)
#define LOG_REGW    (1U << 2)
#define LOG_COMMAND (1U << 3)
#define LOG_DMA     (1U << 4)

//#define VERBOSE (LOG_GENERAL|LOG_REGR|LOG_REGW|LOG_COMMAND|LOG_DMA)
#include "logmacro.h"

enum master_mode_mask : u8
{
	MM0 = 1, // chip enable
	MM1 = 2, // cpu interleave enable
	MM2 = 4, // wait line enable
	MM3 = 8, // no vector on interrupt
};

enum chain_control_mask : u16
{
	CC_CA  = 0x0001, // chain address
	CC_CM  = 0x0002, // channel mode
	CC_IV  = 0x0004, // interrupt vector
	CC_PM  = 0x0008, // pattern and mask
	CC_BOC = 0x0010, // base operation count
	CC_BAB = 0x0020, // base address b
	CC_BAA = 0x0040, // base address a
	CC_COC = 0x0080, // current operation count
	CC_CAB = 0x0100, // current address b
	CC_CAA = 0x0200, // current address a

	CC_WM  = 0x03ff,
};

enum status_mask : u16
{
	S_TC   = 0x0001, // terminal count
	S_EOP  = 0x0002, // end of process
	S_MC   = 0x0004, // match condition
	S_MCL  = 0x0008, // match condition low
	S_MCH  = 0x0010, // match condition high
	S_HRQ  = 0x0020, // hardware request
	S_HM   = 0x0040, // hardware mask
					 // reserved
					 // reserved
	S_SIP  = 0x0200, // second interrupt pending
	S_WFB  = 0x0400, // waiting for bus
	S_NAC  = 0x0800, // no auto-reload or chaining
	S_CA   = 0x1000, // chaining abort
	S_IP   = 0x2000, // interrupt pending
					 // reserved
	S_CIE  = 0x8000, // channel interrupt enable
};

enum cmh_mask : u16
{
	CMH_MC = 0x0003, // match control
	CMH_DC = 0x0004, // dack control
	CMH_HM = 0x0008, // hardware mask
	CMH_SR = 0x0010, // software request

	CMH_WM = 0x001f,
};

enum cmh_mc_mask : u16
{
	MC_00 = 0x0000, // stop on no match
	MC_01 = 0x0020, // stop on no match
	MC_10 = 0x0040, // stop on word match
	MC_11 = 0x0060, // stop on byte match
};

enum cml_mask : u16
{
	CML_OPER = 0x000f, // operation
	CML_FLIP = 0x0010, // flip bit
	CML_TT   = 0x0060, // transfer type
	CML_IEOP = 0x0080, // interrupt enable - end of process
	CML_IMC  = 0x0100, // interrupt enable - match condition
	CML_ITC  = 0x0200, // interrupt enable - terminal count
	CML_REOP = 0x0400, // reload enable - end of process
	CML_RMC  = 0x0800, // reload enable - match condition
	CML_RTC  = 0x1000, // reload enable - terminal count
	CML_CEOP = 0x2000, // chain enable - end of process
	CML_CMC  = 0x4000, // chain enable - match condition
	CML_CTC  = 0x8000, // chain enable - terminal count
};

enum cml_tt_mask : u16
{
	TT_00 = 0x0000, // single transfer
	TT_01 = 0x0020, // demand dedicated bus hold
	TT_10 = 0x0040, // demand dedicated bus release
	TT_11 = 0x0060, // demand interleave
};

enum aru_mask : u16
{
	ARU_WC = 0x0006, // wait control
	ARU_AC = 0x0018, // address control
	ARU_MI = 0x0040, // memory or i/o space
	ARU_NS = 0x0080, // normal or system space
	ARU_AR = 0x00c0, // address reference
	ARU_UA = 0xff00, // upper address

	ARU_WM = 0xffde,
};

enum aru_ac_mask : u16
{
	AC_INC = 0x0000, // increment
	AC_DEC = 0x0008, // decrement
	AC_HLD = 0x0010, // hold
};

enum is_mask : u16
{
	IS_VEC = 0x00ff, // vector
	IS_CHN = 0x0100, // channel
	IS_TC  = 0x0200, // terminal count
	IS_EOP = 0x0400, // end of process
	IS_MC  = 0x0800, // match condition
	IS_CA  = 0x1000, // chain aborted
	IS_MCL = 0x2000, // match condition low
	IS_MCH = 0x4000, // match condition high
	IS_HRQ = 0x8000, // hardware request
};

DEFINE_DEVICE_TYPE(AM9516, am9516_device, "am9516", "Am9516 Universal DMA Controller")

am9516_device::am9516_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AM9516, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config{
		{ "system_io",  ENDIANNESS_BIG, 16, 24 },
		{ "system_mem", ENDIANNESS_BIG, 16, 24 },
		{ "normal_io",  ENDIANNESS_BIG, 16, 24 },
		{ "normal_mem", ENDIANNESS_BIG, 16, 24 } }
	, m_int(*this)
	, m_eop(*this)
	, m_int_state(true)
	, m_eop_out_state(true)
	, m_eop_in_state(false)
	, m_pointer(0)
	, m_channel{ *this, *this }
{
}

void am9516_device::device_start()
{
	save_item(NAME(m_int_state));
	save_item(NAME(m_eop_out_state));
	save_item(NAME(m_eop_in_state));

	save_item(NAME(m_mode));
	save_item(NAME(m_pointer));
	save_item(NAME(m_temporary));

	save_item(STRUCT_MEMBER(m_channel, cabl));
	save_item(STRUCT_MEMBER(m_channel, babl));
	save_item(STRUCT_MEMBER(m_channel, caal));
	save_item(STRUCT_MEMBER(m_channel, baal));
	save_item(STRUCT_MEMBER(m_channel, cabu));
	save_item(STRUCT_MEMBER(m_channel, babu));
	save_item(STRUCT_MEMBER(m_channel, caau));
	save_item(STRUCT_MEMBER(m_channel, baau));
	save_item(STRUCT_MEMBER(m_channel, cal));
	save_item(STRUCT_MEMBER(m_channel, cau));
	save_item(STRUCT_MEMBER(m_channel, is));
	save_item(STRUCT_MEMBER(m_channel, status));
	save_item(STRUCT_MEMBER(m_channel, coc));
	save_item(STRUCT_MEMBER(m_channel, boc));
	save_item(STRUCT_MEMBER(m_channel, pattern));
	save_item(STRUCT_MEMBER(m_channel, mask));
	save_item(STRUCT_MEMBER(m_channel, cml));
	save_item(STRUCT_MEMBER(m_channel, cmh));
	save_item(STRUCT_MEMBER(m_channel, iv));

	m_int.resolve_safe();
	m_eop.resolve_safe();

	for (channel &ch : m_channel)
	{
		ch.flyby_byte_r.resolve_safe(0);
		ch.flyby_byte_w.resolve_safe();
		ch.flyby_word_r.resolve_safe(0);
		ch.flyby_word_w.resolve_safe();

		ch.cabl = 0;
		ch.babl = 0;
		ch.caal = 0;
		ch.baal = 0;
		ch.cabu = 0;
		ch.babu = 0;
		ch.caau = 0;
		ch.baau = 0;
		ch.cal = 0;
		ch.cau = 0;
		ch.status = 0;
		ch.coc = 0;
		ch.boc = 0;
		ch.pattern = 0;
		ch.mask = 0;
		ch.cml = 0;
		ch.cmh = 0;
		ch.iv = 0;
	}

	m_channel[0].is = 0;
	m_channel[1].is = IS_CHN;

	m_channel[0].run = timer_alloc(timer_expired_delegate(FUNC(am9516_device::operate<0>), this));
	m_channel[1].run = timer_alloc(timer_expired_delegate(FUNC(am9516_device::operate<1>), this));
}

void am9516_device::device_reset()
{
	m_mode = 0;

	for (channel &ch : m_channel)
	{
		ch.run->reset();

		ch.status = S_CA | S_NAC;
		ch.is &= IS_CHN;
	}

	if (m_eop_out_state)
	{
		m_eop_out_state = false;
		eop_w(!m_eop_out_state);
	}

	interrupt();
}

device_memory_interface::space_config_vector am9516_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(SYSTEM_IO,  &m_space_config[0]),
		std::make_pair(SYSTEM_MEM, &m_space_config[1]),
		std::make_pair(NORMAL_IO,  &m_space_config[2]),
		std::make_pair(NORMAL_MEM, &m_space_config[3]),
	};
}

u16 am9516_device::data_r()
{
	switch (m_pointer)
	{
		// address b lower
	case 0x00: return m_channel[1].cabl;
	case 0x02: return m_channel[0].cabl;
	case 0x04: return m_channel[1].babl;
	case 0x06: return m_channel[0].babl;
		// address a lower
	case 0x08: return m_channel[1].caal;
	case 0x0a: return m_channel[0].caal;
	case 0x0c: return m_channel[1].baal;
	case 0x0e: return m_channel[0].baal;
		// address b upper
	case 0x10: return m_channel[1].cabu;
	case 0x12: return m_channel[0].cabu;
	case 0x14: return m_channel[1].babu;
	case 0x16: return m_channel[0].babu;
		// address a upper
	case 0x18: return m_channel[1].caau;
	case 0x1a: return m_channel[0].caau;
	case 0x1c: return m_channel[1].baau;
	case 0x1e: return m_channel[0].baau;
		// chain address
	case 0x20: return m_channel[1].cal;
	case 0x22: return m_channel[0].cal;
	case 0x24: return m_channel[1].cau;
	case 0x26: return m_channel[0].cau;
		// interrupt save
	case 0x28: return m_channel[1].is;
	case 0x2a: return m_channel[0].is;
		// status
	case 0x2c: return m_channel[1].status;
	case 0x2e: return m_channel[0].status;
		// operation count
	case 0x30: return m_channel[1].coc;
	case 0x32: return m_channel[0].coc;
	case 0x34: return m_channel[1].boc;
	case 0x36: return m_channel[0].boc;
		// master mode
	case 0x38: return m_mode;
		// pattern
	case 0x48: return m_channel[1].pattern;
	case 0x4a: return m_channel[0].pattern;
		// mask
	case 0x4c: return m_channel[1].mask;
	case 0x4e: return m_channel[0].mask;
		// channel mode
	case 0x50: return m_channel[1].cml;
	case 0x52: return m_channel[0].cml;
	case 0x54: return m_channel[1].cmh;
	case 0x56: return m_channel[0].cmh;
		// interrupt vector
	case 0x58: return m_channel[1].iv;
	case 0x5a: return m_channel[0].iv;

	default:
		LOG("undefined register 0x%02 (%s)\n",
			m_pointer, machine().describe_context());
		return 0;
	}
}

void am9516_device::data_w(u16 data)
{
	switch (m_pointer)
	{
		// address b lower
	case 0x00: m_channel[1].cabl = data; break;
	case 0x02: m_channel[0].cabl = data; break;
	case 0x04: m_channel[1].babl = data; break;
	case 0x06: m_channel[0].babl = data; break;
		// address a lower
	case 0x08: m_channel[1].caal = data; break;
	case 0x0a: m_channel[0].caal = data; break;
	case 0x0c: m_channel[1].baal = data; break;
	case 0x0e: m_channel[0].baal = data; break;
		// address b upper
	case 0x10: m_channel[1].cabu = data & ARU_WM; break;
	case 0x12: m_channel[0].cabu = data & ARU_WM; break;
	case 0x14: m_channel[1].babu = data & ARU_WM; break;
	case 0x16: m_channel[0].babu = data & ARU_WM; break;
		// address a upper
	case 0x18: m_channel[1].caau = data & ARU_WM; break;
	case 0x1a: m_channel[0].caau = data & ARU_WM; break;
	case 0x1c: m_channel[1].baau = data & ARU_WM; break;
	case 0x1e: m_channel[0].baau = data & ARU_WM; break;
		// chain address
	case 0x20: m_channel[1].cal = data; break;
	case 0x22: m_channel[0].cal = data; break;
	case 0x24: m_channel[1].cau = data & (ARU_UA | ARU_WC); break;
	case 0x26: m_channel[0].cau = data & (ARU_UA | ARU_WC); break;
		// command
	case 0x2c:
	case 0x2e:
		command(data);
		return;
		// operation count
	case 0x30: m_channel[1].coc = data; break;
	case 0x32: m_channel[0].coc = data; break;
	case 0x34: m_channel[1].boc = data; break;
	case 0x36: m_channel[0].boc = data; break;
		// master mode
	case 0x38:
		LOGMASKED(LOG_REGW, "master mode 0x%04x (%s)\n",
			data, machine().describe_context());
		m_mode = data & 0xf;
		return;
		// pattern
	case 0x48: m_channel[1].pattern = data; break;
	case 0x4a: m_channel[0].pattern = data; break;
		// mask
	case 0x4c: m_channel[1].mask = data; break;
	case 0x4e: m_channel[0].mask = data; break;
		// channel mode
	case 0x50: m_channel[1].cml = data; m_channel[1].log_mode(LOG_GENERAL); break;
	case 0x52: m_channel[0].cml = data; m_channel[0].log_mode(LOG_GENERAL); break;
		// interrupt vector
	case 0x58: m_channel[1].iv = data; break;
	case 0x5a: m_channel[0].iv = data; break;

	default:
		LOG("undefined register 0x%02 data 0x%04x (%s)\n",
			m_pointer, data, machine().describe_context());
		return;
	}

	if (VERBOSE & LOG_REGW)
	{
		static char const *const reg_name[] =
		{
			"current address b lower", "base address b lower",
			"current address a lower", "base address a lower",
			"current address b upper", "base address b upper",
			"current address a upper", "base address a upper",
			"chain address lower", "chain address upper",
			"interrupt save", "status",
			"current operation count", "base operation count",
			nullptr, nullptr, nullptr, nullptr,
			"pattern", "mask",
			"channel mode low", "channel mode high",
			"interrupt vector", nullptr,
		};

		LOGMASKED(LOG_REGW, "channel %d %s 0x%04x (%s)\n",
			!(m_pointer & 2), reg_name[m_pointer >> 2], data, machine().describe_context());
	}
}

void am9516_device::command(u8 data)
{
	channel &ch = m_channel[BIT(data, 0)];

	switch (data & 0xe0)
	{
	case 0x00: // reset
		LOGMASKED(LOG_COMMAND, "reset (%s)\n", machine().describe_context());

		reset();
		break;

	case 0x20: // interrupt control
		LOGMASKED(LOG_COMMAND, "channel %d %s%s%s (%s)\n", BIT(data, 0), BIT(data, 1) ? "set" : "clear",
			BIT(data, 4) ? " CIE" : "", BIT(data, 2) ? " IP" : "", machine().describe_context());

		// update channel interrupt enable
		if (BIT(data, 4))
		{
			if (BIT(data, 1))
				ch.status |= S_CIE;
			else
				ch.status &= ~S_CIE;
		}

		// update interrupt pending
		if (BIT(data, 2))
			ch.interrupt(BIT(data, 1));

		interrupt();
		break;

	case 0x40: // software request
		LOGMASKED(LOG_COMMAND, "channel %d %s software request bit (%s)\n",
			BIT(data, 0), BIT(data, 1) ? "set" : "clear", machine().describe_context());

		if (BIT(data, 1))
		{
			ch.cmh |= CMH_SR;
			ch.run->adjust(attotime::zero);
		}
		else
			ch.cmh &= ~CMH_SR;
		break;

	case 0x60: // set/clear flip bit
		LOGMASKED(LOG_COMMAND, "channel %d %s flip bit (%s)\n",
			BIT(data, 0), BIT(data, 1) ? "set" : "clear", machine().describe_context());

		if (BIT(data, 1))
			ch.cml |= CML_FLIP;
		else
			ch.cml &= ~CML_FLIP;
		break;

	case 0x80: // hardware mask
		LOGMASKED(LOG_COMMAND, "channel %d %s hardware mask bit (%s)\n",
			BIT(data, 0), BIT(data, 1) ? "set" : "clear", machine().describe_context());

		if (BIT(data, 1))
			ch.cmh |= CMH_HM;
		else
			ch.cmh &= ~CMH_HM;
		break;

	case 0xa0: // start chain
		LOGMASKED(LOG_COMMAND, "channel %d start chain (%s)\n",
			BIT(data, 0), machine().describe_context());

		ch.chain();
		break;

	default:
		LOGMASKED(LOG_COMMAND, "channel %d unrecognized command 0x%02x (%s)\n",
			BIT(data, 0), data, machine().describe_context());
		break;
	}
}

WRITE_LINE_MEMBER(am9516_device::eop_w)
{
	LOGMASKED(LOG_DMA, "eop %s\n", state ? "cleared" : "asserted");
	m_eop_in_state = !state;
}

template <unsigned Channel> WRITE_LINE_MEMBER(am9516_device::dreq_w)
{
	LOGMASKED(LOG_DMA, "channel %d dreq %s\n", Channel, state ? "cleared" : "asserted");
	channel &ch = m_channel[Channel];

	if (!state)
	{
		ch.status |= S_HRQ;

		if (!(ch.status & S_HM) && ch.run->expire().is_never())
			ch.run->adjust(attotime::zero);
	}
	else
		ch.status &= ~S_HRQ;
}

template void am9516_device::dreq_w<0>(int state);
template void am9516_device::dreq_w<1>(int state);

template <unsigned Channel> void am9516_device::operate(s32 param)
{
	channel &ch = m_channel[Channel];

	if (!(m_mode & MM0) || (ch.status & S_SIP))
	{
		LOGMASKED(LOG_DMA, "channel %d bus access disabled\n", Channel);
		return;
	}

	if (!(ch.status & S_HRQ) && !(ch.cmh & CMH_SR))
	{
		LOGMASKED(LOG_DMA, "channel %d no request pending\n", Channel);
		return;
	}

	u16 status = 0;
	unsigned cycles = 0;

	switch (ch.cml & CML_OPER)
	{
		// transfer
	case 0x1:
		// byte/byte flowthru
		cycles = 6;
		m_temporary = ch.read_byte(cycles, ch.cml & CML_FLIP);
		ch.write_byte(u8(m_temporary), cycles, ch.cml & CML_FLIP);

		ch.coc--;
		break;
	case 0x8:
	case 0x9:
		cycles = 9;
		if (ch.cml & CML_FLIP)
		{
			// word/byte flowthru
			m_temporary = ch.read_word(cycles, ch.cml & CML_FLIP);

			unsigned const shift = ch.cabu & AC_DEC;
			ch.write_byte(u8(m_temporary >> (8 - shift)), cycles, ch.cml & CML_FLIP);
			ch.write_byte(u8(m_temporary >> shift), cycles, ch.cml & CML_FLIP);
		}
		else
		{
			// byte/word flowthru
			unsigned const shift = ch.cabu & AC_DEC;
			m_temporary = u16(ch.read_byte(cycles, ch.cml & CML_FLIP)) << (8 - shift);
			m_temporary |= u16(ch.read_byte(cycles, ch.cml & CML_FLIP)) << shift;

			ch.write_word(m_temporary, cycles, ch.cml & CML_FLIP);
		}

		ch.coc--;
		break;
	case 0x0:
		//  word/word flowthru
		cycles = 6;
		m_temporary = ch.read_word(cycles, ch.cml & CML_FLIP);
		ch.write_word(m_temporary, cycles, ch.cml & CML_FLIP);

		ch.coc--;
		break;
	case 0x3:
		// byte/byte flyby
		cycles = 3;
		if (ch.cml & CML_FLIP)
			// from flyby to arb
			ch.write_byte(ch.flyby_byte_r(), cycles);
		else
			// from ara to flyby
			ch.flyby_byte_w(ch.read_byte(cycles));

		ch.coc--;
		break;
	case 0x2:
		//  word/word flyby
		cycles = 3;
		if (ch.cml & CML_FLIP)
			// from flyby to arb
			ch.write_word(ch.flyby_word_r(), cycles);
		else
			// from ara to flyby
			ch.flyby_word_w(ch.read_word(cycles));

		ch.coc--;
		break;

		// transfer and search
	case 0x5:
		// byte/byte flowthru
		break;
	case 0xc:
	case 0xd:
		// byte/word flowthru
		break;
	case 0x4:
		// word/word flowthru
		break;
	case 0x7:
		// byte/byte flyby
		break;
	case 0x6:
		// word/word flyby
		break;

		// search
	case 0xf:
		// byte/byte
		break;
	case 0xe:
		// word/word
		break;
	case 0xa:
	case 0xb:
		// illegal
		break;
	}

	// check for terminal count
	if (ch.coc == 0)
		status |= S_TC;

	// check for end of process
	if (m_eop_in_state)
		status |= S_EOP;

	if (status & (S_MC | S_EOP | S_TC))
		complete(param, status);
	else if (ch.cml & CML_TT)
		ch.run->adjust(attotime::from_ticks(cycles, clock()));
}

void am9516_device::complete(unsigned const c, u16 status)
{
	channel &ch = m_channel[c];

	ch.status &= ~(S_MCH | S_MCL | S_MC | S_EOP | S_TC);
	ch.status |= status | S_NAC;

	LOGMASKED(LOG_DMA, "channel %d complete status 0x%04x\n", c, ch.status);

	m_eop(0);
	m_eop(1);

	// completion interrupt
	if (((ch.status & S_TC) && (ch.cml & CML_ITC))
		|| ((ch.status & S_EOP) && (ch.cml & CML_IEOP))
		|| ((ch.status & S_MC) && (ch.cml & CML_IMC)))
	{
		ch.interrupt(true);

		interrupt();
	}

	if (!(ch.status & S_SIP))
		ch.reload();
}

void am9516_device::interrupt()
{
	bool const int_state =
		((m_channel[0].status & S_CIE) && (m_channel[0].status & S_IP)) ||
		((m_channel[1].status & S_CIE) && (m_channel[1].status & S_IP));

	if (m_int_state != int_state)
	{
		LOG("interrupt %s\n", int_state ? "asserted" : "cleared");
		m_int_state = int_state;
		m_int(!m_int_state);
	}
}

u16 am9516_device::acknowledge()
{
	for (channel &ch : m_channel)
	{
		if ((ch.status & S_CIE) && (ch.status & S_IP))
		{
			u16 const data = (m_mode & MM3) ? 0 : ch.is;

			ch.interrupt(false);
			interrupt();

			return data;
		}
	}

	fatalerror("%s: interrupt acknowledge with no pending interrupts\n", tag());
}

#undef LOG_OUTPUT_FUNC
#define LOG_OUTPUT_FUNC udc.logerror

u32 am9516_device::channel::address(u16 &aru, u16 &arl, int delta)
{
	u32 const current = u32(aru & ARU_UA) << 8 | arl;

	if (!delta || (aru & AC_HLD))
		return current;

	if (!(aru & ARU_MI))
		delta = 2;

	if (aru & AC_DEC)
		delta = -delta;

	u32 const adjusted = current + delta;

	aru = ((adjusted >> 8) & ARU_UA) | (aru & ~ARU_UA);
	arl = u16(adjusted);

	return current;
}

u8 am9516_device::channel::read_byte(unsigned &cycles, bool flip)
{
	u16 &cau = flip ? cabu : caau;
	u16 &cal = flip ? cabl : caal;

	cycles += wait_states[BIT(cau, 1, 2)];

	return udc.space((cau & ARU_AR) >> 6).read_byte(address(cau, cal, 1));
}

void am9516_device::channel::write_byte(u8 data, unsigned &cycles, bool flip)
{
	u16 &cau = flip ? caau : cabu;
	u16 &cal = flip ? caal : cabl;

	cycles += wait_states[BIT(cau, 1, 2)];

	udc.space((cau & ARU_AR) >> 6).write_byte(address(cau, cal, 1), data);
}

u16 am9516_device::channel::read_word(unsigned &cycles, bool flip)
{
	u16 &cau = flip ? cabu : caau;
	u16 &cal = flip ? cabl : caal;

	cycles += wait_states[BIT(cau, 1, 2)];

	return udc.space((cau & ARU_AR) >> 6).read_word(address(cau, cal, 2));
}

void am9516_device::channel::write_word(u16 data, unsigned &cycles, bool flip)
{
	u16 &cau = flip ? caau : cabu;
	u16 &cal = flip ? caal : cabl;

	cycles += wait_states[BIT(cau, 1, 2)];

	udc.space((cau & ARU_AR) >> 6).write_word(address(cau, cal, 2), data);
}

void am9516_device::channel::interrupt(bool assert)
{
	if (assert && (status & S_IP))
	{
		LOG("second interrupt pending\n");
		status |= S_SIP;
		return;
	}

	if (!assert)
	{
		status &= ~S_IP;
		is &= IS_CHN;

		if (status & S_SIP)
		{
			LOG("second interrupt cleared\n");
			status &= ~S_SIP;

			reload();
		}
		else
			return;
	}

	status |= S_IP;
	is |= bitswap<u16>(status, 5, 4, 3, 12, 2, 1, 0) << 9 | iv;
}

void am9516_device::channel::reload()
{
	// reload base to current
	if (((status & S_TC) && (cml & CML_RTC))
		|| ((status & S_EOP) && (cml & CML_REOP))
		|| ((status & S_MC) && (cml & CML_RMC)))
	{
		LOGMASKED(LOG_DMA, "reload base to current\n");
		caau = baau;
		caal = baal;
		cabu = babu;
		cabl = babl;
		coc = boc;

		log_addr(LOG_DMA, "current address a", caau, caal);
		log_addr(LOG_DMA, "current address b", cabu, cabl);
		LOGMASKED(LOG_DMA, "current operation count 0x%04x\n", coc);

		status &= ~S_NAC;
	}

	// reload chain
	if (((status & S_TC) && (cml & CML_CTC))
		|| ((status & S_EOP) && (cml & CML_CEOP))
		|| ((status & S_MC) && (cml & CML_CMC)))
	{
		LOGMASKED(LOG_DMA, "reload chain\n");
		chain();
	}
}

void am9516_device::channel::chain()
{
	address_space &s(udc.space(SYSTEM_MEM));

	// TODO: abort on EOP

	// fetch reload word
	u32 chain_address = address(cau, cal);
	u16 reload = s.read_word(chain_address) & CC_WM;
	LOGMASKED(LOG_REGW, "chain address 0x%06x reload word 0x%04x\n", chain_address, reload);
	chain_address += 2;

	// current address a
	if (reload & CC_CAA)
	{
		caau = s.read_word(chain_address + 0) & ARU_WM;
		caal = s.read_word(chain_address + 2);

		LOGMASKED(LOG_REGW, "current address a 0x%04x 0x%04x\n", caau, caal);
		log_addr(LOG_DMA, "current address a", caau, caal);

		chain_address += 4;
	}

	// current address b
	if (reload & CC_CAB)
	{
		cabu = s.read_word(chain_address + 0) & ARU_WM;
		cabl = s.read_word(chain_address + 2);

		LOGMASKED(LOG_REGW, "current address b 0x%04x 0x%04x\n", cabu, cabl);
		log_addr(LOG_DMA, "current address b", cabu, cabl);

		chain_address += 4;
	}

	// current operation count
	if (reload & CC_COC)
	{
		coc = s.read_word(chain_address);

		LOGMASKED(LOG_DMA, "current operation count 0x%04x\n", coc);

		chain_address += 2;
	}

	// base address a
	if (reload & CC_BAA)
	{
		baau = s.read_word(chain_address + 0) & ARU_WM;
		baal = s.read_word(chain_address + 2);

		LOGMASKED(LOG_REGW, "base address a 0x%04x 0x%04x\n", baau, baal);

		chain_address += 4;
	}

	// base address b
	if (reload & CC_BAB)
	{
		babu = s.read_word(chain_address + 0) & ARU_WM;
		babl = s.read_word(chain_address + 2);

		LOGMASKED(LOG_REGW, "base address b 0x%04x 0x%04x\n", babu, babl);

		chain_address += 4;
	}

	// base operation count
	if (reload & CC_BOC)
	{
		boc = s.read_word(chain_address);

		LOGMASKED(LOG_REGW, "base operation count 0x%04x\n", boc);

		chain_address += 2;
	}

	// pattern and mask
	if (reload & CC_PM)
	{
		pattern = s.read_word(chain_address + 0);
		mask = s.read_word(chain_address + 2);

		LOGMASKED(LOG_REGW, "pattern 0x%04x mask %04x\n", pattern, mask);

		chain_address += 4;
	}

	// interrupt vector
	if (reload & CC_IV)
	{
		iv = s.read_word(chain_address);

		LOGMASKED(LOG_REGW, "interrupt vector 0x%04x\n", iv);

		chain_address += 2;
	}

	// channel mode
	if (reload & CC_CM)
	{
		cmh = s.read_word(chain_address + 0) & CMH_WM;
		cml = s.read_word(chain_address + 2);

		LOGMASKED(LOG_REGW, "channel mode 0x%04x %04x\n", cmh, cml);
		log_mode(LOG_DMA, true);

		chain_address += 4;
	}

	// chain address
	if (reload & CC_CA)
	{
		cau = s.read_word(chain_address + 0) & (ARU_UA | ARU_WC);
		cal = s.read_word(chain_address + 2);

		LOGMASKED(LOG_REGW, "chain address 0x%04x %04x\n", cau, cal);

		chain_address += 4;
	}
	else
	{
		// update chain address register
		cau = ((chain_address >> 8) & ARU_UA) | u8(cau);
		cal = u16(chain_address);
	}

	status &= ~(S_CA | S_NAC);

	if (cmh & CMH_SR)
		run->adjust(attotime::zero);
}

void am9516_device::channel::log_mode(unsigned mask, bool high) const
{
	if (VERBOSE & mask)
	{
		static const char *const match[] =
		{
			"no match", "no match", "word match", "byte match"
		};
		static const char *const operation[] =
		{
			"transfer, word/word flowthru",
			"transfer, byte/byte flowthru",
			"transfer, word/word flyby",
			"transfer, byte/byte flyby",
			"transfer/search, word/word flowthru",
			"transfer/search, byte/byte flowthru",
			"transfer/search, word/word flyby",
			"transfer/search, byte/byte flyby",
			"transfer, byte/word flowthru",
			"transfer, byte/word flowthru",
			"illegal",
			"illegal",
			"transfer/search, byte/word flowthru",
			"transfer/search, byte/word flowthru",
			"search, word/word",
			"search, byte/byte",
		};
		static const char *const tt[] =
		{
			"single transfer", "demand dedicated/bus hold", "demand dedicated/bus release", "demand interleave"
		};
		static const char *const flags[] =
		{
			"-", "eop", "mc", "mc|eop", "tc", "tc|eop", "tc|mc", "tc|mc|eop"
		};

		LOGMASKED(mask, "channel mode %s, %s, complete(int:%s rld:%s chn:%s)\n",
			operation[cml & CML_OPER], tt[BIT(cml, 5, 2)], flags[BIT(cml, 7, 3)], flags[BIT(cml, 10, 3)], flags[BIT(cml, 13, 3)]);

		if (high)
			LOGMASKED(mask, "channel mode stop:%s /dack:%d hm:%d sr:%d\n",
				match[BIT(cmh, 0, 2)], BIT(cmh, 2), BIT(cmh, 3), BIT(cmh, 4));
	}
}

void am9516_device::channel::log_addr(unsigned mask, const char *const name, u16 aru, u16 arl) const
{
	if (VERBOSE & mask)
	{
		static const char *const ar[] =
		{
			"system i/o", "system mem", "normal i/o", "normal mem"
		};
		static const char *const ac[] =
		{
			"inc", "dec", "hld", "hld"
		};

		LOGMASKED(mask, "%s %s 0x%06x %s wait:%d\n",
			name, ar[BIT(aru, 6, 2)], u32(aru & ARU_UA) << 8 | arl,
			ac[BIT(aru, 3, 2)], wait_states[BIT(aru, 1, 2)]);
	}
}
