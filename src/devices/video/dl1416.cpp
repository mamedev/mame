// license:BSD-3-Clause
// copyright-holders:Dirk Best, Vas Crabb
/*****************************************************************************
 *
 *  DL1416
 *
 *
 * 4-Digit 16-Segment Alphanumeric Intelligent Display
 * with Memory/Decoder/Driver
 *
 * Notes:
 *   - Currently supports the DL1416T and by virtue of it being nearly the
 *     same, the DL1414.
 *   - The DL1416B uses a simpler sequence to control cursor display, and
 *     has a '.' segment but is otherwise fully compatible with the DL1416T.
 *   - Cursor support is implemented but not tested, as the AIM65 does not
 *     seem to use it.
 *
 * TODO:
 *   - '.' segment for DL1414T and DL1416B
 *
 * Changes:
 *   - 2007-07-30: Initial version.  [Dirk Best]
 *   - 2008-02-25: Converted to the new device interface.  [Dirk Best]
 *   - 2008-12-18: Cleanups.  [Dirk Best]
 *   - 2011-10-08: Changed the ram to store character rather than segment data. [Lord Nightmare]
 *   - 2017-02-11: Better signal-level interface, better support for variants [Vas Crabb]
 *
 *
 * We use the following order for the segments:
 *
 *   000 111
 *  7D  A  E2
 *  7 D A E 2
 *  7  DAE  2
 *   888 999
 *  6  CBF  3
 *  6 C B F 3
 *  6C  B  F3
 *   555 444
 *
 ****************************************************************************/

#include "emu.h"
#include "dl1416.h"

namespace {

/***************************************************************************
    CONSTANTS
***************************************************************************/

constexpr u16  SEG_BLANK   = 0x0000;
constexpr u16  SEG_UNDEF   = SEG_BLANK;
constexpr u16  SEG_CURSOR  = 0xffff;


/* character set DL1416T */
u16 const dl1416t_segments[128] = {
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	0x0000,    0x2421,    0x0480,    0x0f3c,    /*   ! " # */
	0x0fbb,    0x5f99,    0xa579,    0x4000,    /* $ % & ' */
	0xc000,    0x3000,    0xff00,    0x0f00,    /* ( ) * + */
	0x1000,    0x0300,    0x0020,    0x5000,    /* , - . / */
	0x0ce1,    0x0c00,    0x0561,    0x0d21,    /* 0 1 2 3 */
	0x0d80,    0x09a1,    0x09e1,    0x0c01,    /* 4 5 6 7 */
	0x0de1,    0x0da1,    0x0021,    0x1001,    /* 8 9 : ; */
	0x5030,    0x0330,    0xa030,    0x0a07,    /* < = > ? */
	0x097f,    0x03cf,    0x0e3f,    0x00f3,    /* @ A B C */
	0x0c3f,    0x01f3,    0x01c3,    0x02fb,    /* D E F G */
	0x03cc,    0x0c33,    0x0c63,    0xc1c0,    /* H I J K */
	0x00f0,    0x60cc,    0xa0cc,    0x00ff,    /* L M N O */
	0x03c7,    0x80ff,    0x83c7,    0x03bb,    /* P Q R S */
	0x0c03,    0x00fc,    0x50c0,    0x90cc,    /* T U V W */
	0xf000,    0x6800,    0x5033,    0x00e1,    /* X Y Z [ */
	0xa000,    0x001e,    0x9000,    0x0030,    /* \ ] ^ _ */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF  /* undefined */
};


class dl1414t_device : public dl1414_device
{
public:
	dl1414t_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: dl1414_device(mconfig, DL1414T, tag, owner, clock)
	{
	}

protected:
	virtual u16 translate(u8 digit, bool cursor) const override
	{
		return dl1416t_segments[digit & 0x7f];
	}
};


class dl1416b_device : public dl1416_device
{
public:
	dl1416b_device(machine_config const &mconfig, const char *tag, device_t *owner, u32 clock)
		: dl1416_device(mconfig, DL1416B, tag, owner, clock)
	{
	}

	virtual void bus_w(offs_t offset, u8 data) override
	{
		if (!cu_in())
			set_cursor_state(offset, BIT(data, 0));
		else
			dl1416_device::bus_w(offset, data);
	}

protected:
	virtual u16 translate(u8 digit, bool cursor) const override
	{
		return cursor ? SEG_CURSOR : dl1416t_segments[digit & 0x7f];
	}
};


class dl1416t_device : public dl1416_device
{
public:
	dl1416t_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: dl1416_device(mconfig, DL1416T, tag, owner, clock)
	{
	}

	virtual void bus_w(offs_t offset, u8 data) override
	{
		if (!cu_in())
			for (unsigned i = 0; 4 > i; ++i) set_cursor_state(i, BIT(data, i));
		else
			dl1416_device::bus_w(offset, data);
	}

protected:
	virtual u16 translate(u8 digit, bool cursor) const override
	{
		digit &= 0x7f;
		return (cursor && (0x20 <= digit) && (0x5f >= digit)) ? SEG_CURSOR : dl1416t_segments[digit];
	}
};

} // anonymous namespace



/*****************************************************************************
    DEVICE TYPE GLOBALS
*****************************************************************************/

DEFINE_DEVICE_TYPE_PRIVATE(DL1414T, dl1414_device, dl1414t_device, "dl1414t", "DL1414T Intelligent Display")
DEFINE_DEVICE_TYPE_PRIVATE(DL1416B, dl1416_device, dl1416b_device, "dl1416b", "DL1416B Intelligent Display")
DEFINE_DEVICE_TYPE_PRIVATE(DL1416T, dl1416_device, dl1416t_device, "dl1416t", "DL1416T Intelligent Display")
template class device_finder<dl1414_device, false>;
template class device_finder<dl1414_device, true>;
template class device_finder<dl1416_device, false>;
template class device_finder<dl1416_device, true>;



/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

dl1414_device::dl1414_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_wr_in(true)
	, m_addr_in(0x00)
	, m_data_in(0x00)
	, m_update_cb(*this)
	, m_digit_ram{ 0x00, 0x00, 0x00, 0x00 }
	, m_cursor_state{ false, false, false, false }
{
}

dl1414_device::~dl1414_device()
{
}

dl1416_device::dl1416_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: dl1414_device(mconfig, type, tag, owner, clock)
	, m_ce_in(true)
	, m_ce_latch(true)
	, m_cu_in(true)
	, m_addr_latch(0x00)
{
}

dl1416_device::~dl1416_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dl1414_device::device_start()
{
	// register for state saving
	save_item(NAME(m_digit_ram));
	save_item(NAME(m_cursor_state));
	save_item(NAME(m_wr_in));
	save_item(NAME(m_addr_in));
	save_item(NAME(m_data_in));

	// set initial state for input lines
	m_wr_in = true;
	m_addr_in = 0x00;
	m_data_in = 0x00;
}

void dl1416_device::device_start()
{
	dl1414_device::device_start();

	// register for state saving
	save_item(NAME(m_ce_in));
	save_item(NAME(m_ce_latch));
	save_item(NAME(m_cu_in));
	save_item(NAME(m_addr_latch));

	// set initial state for input lines
	m_ce_in = true;
	m_ce_latch = true;
	m_cu_in = true;
	m_addr_latch = 0x00;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dl1414_device::device_reset()
{
	// push initial display state
	for (unsigned i = 0; 4 > i; ++i)
		do_update(i);
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

void dl1414_device::wr_w(int state)
{
	if (bool(state) != m_wr_in)
	{
		m_wr_in = bool(state);
		if (m_wr_in)
			bus_w(m_addr_in, m_data_in);
	}
}

void dl1416_device::wr_w(int state)
{
	if (bool(state) != m_wr_in)
	{
		m_wr_in = bool(state);
		if (m_wr_in)
		{
			if (!m_ce_latch)
				bus_w(m_addr_latch, m_data_in);
		}
		else
		{
			m_ce_latch = m_ce_in;
			m_addr_latch = m_addr_in;
		}
	}
}

void dl1416_device::ce_w(int state)
{
	m_ce_in = bool(state);
}

void dl1416_device::cu_w(int state)
{
	m_cu_in = bool(state);
}

void dl1414_device::addr_w(u8 state)
{
	m_addr_in = state & 0x03;
}

void dl1414_device::data_w(u8 state)
{
	m_data_in = state & 0x7f;
}

void dl1414_device::bus_w(offs_t offset, u8 data)
{
	offset &= 0x03; // A0-A1
	data &= 0x7f; // D0-D6

	if (m_digit_ram[offset] != data)
	{
		m_digit_ram[offset] = data;
		do_update(offset);
	}
}

void dl1414_device::set_cursor_state(offs_t offset, bool state)
{
	offset &= 0x03; // A0-A1

	if (state != m_cursor_state[offset])
	{
		m_cursor_state[offset] = state;
		do_update(offset);
	}
}

void dl1414_device::do_update(offs_t offset)
{
	m_update_cb(offset, translate(m_digit_ram[offset], m_cursor_state[offset]), 0xffff);
}
