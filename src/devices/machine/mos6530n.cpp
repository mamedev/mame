// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6530 Memory, I/O, Timer Array emulation
    MOS Technology 6532 RAM, I/O, Timer Array emulation

**********************************************************************/

#include "mos6530n.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0
#define LOG_TIMER 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MOS6530n = &device_creator<mos6530_t>;
const device_type MOS6532n = &device_creator<mos6532_t>;


DEVICE_ADDRESS_MAP_START( rom_map, 8, mos6530_t )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x000, 0x3ff) AM_READ(rom_r)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( ram_map, 8, mos6530_t )
	ADDRESS_MAP_GLOBAL_MASK(0x3f)
	AM_RANGE(0x00, 0x3f) AM_READWRITE(ram_r, ram_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( io_map, 8, mos6530_t )
	ADDRESS_MAP_GLOBAL_MASK(0xf)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x8) AM_READWRITE(pa_data_r, pa_data_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x8) AM_READWRITE(pa_ddr_r, pa_ddr_w)
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x8) AM_READWRITE(pb_data_r, pb_data_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x8) AM_READWRITE(pb_ddr_r, pb_ddr_w)
	AM_RANGE(0x04, 0x07) AM_WRITE(timer_off_w)
	AM_RANGE(0x0c, 0x0f) AM_WRITE(timer_on_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0x2) AM_READ(timer_off_r)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0x2) AM_READ(timer_on_r)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0xa) AM_READ(irq_r)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( ram_map, 8, mos6532_t )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)
	AM_RANGE(0x00, 0x7f) AM_READWRITE(ram_r, ram_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( io_map, 8, mos6532_t )
	ADDRESS_MAP_GLOBAL_MASK(0x1f)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x18) AM_READWRITE(pa_data_r, pa_data_w)  // SWCHA
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x18) AM_READWRITE(pa_ddr_r, pa_ddr_w)    // SWACNT
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x18) AM_READWRITE(pb_data_r, pb_data_w)  // SWCHB
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x18) AM_READWRITE(pb_ddr_r, pb_ddr_w)    // SWBCNT
	AM_RANGE(0x14, 0x17) AM_WRITE(timer_off_w)
	AM_RANGE(0x1c, 0x1f) AM_WRITE(timer_on_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0x12) AM_READ(timer_off_r)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0x12) AM_READ(timer_on_r)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0x1a) AM_READ(irq_r)
	AM_RANGE(0x04, 0x07) AM_MIRROR(0x8) AM_WRITE(edge_w)
ADDRESS_MAP_END

READ8_MEMBER(mos6532_t::io_r)
{
	offset &= 0x1f;
	UINT8 ret = 0;

	if (offset == 0x00 || offset == 0x08 || offset == 0x10 || offset == 0x18) ret = pa_data_r(space, 0);
	if (offset == 0x01 || offset == 0x09 || offset == 0x11 || offset == 0x19) ret = pa_ddr_r(space, 0);
	if (offset == 0x02 || offset == 0x0a || offset == 0x12 || offset == 0x1a) ret = pb_data_r(space, 0);
	if (offset == 0x03 || offset == 0x0b || offset == 0x13 || offset == 0x1b) ret = pb_ddr_r(space, 0);

	if (offset == 0x04 || offset == 0x06 || offset == 0x14 || offset == 0x16) ret = timer_off_r(space, 0);
	if (offset == 0x0c || offset == 0x0e || offset == 0x1c || offset == 0x1e) ret = timer_on_r(space, 0);

	if (offset == 0x05 || offset == 0x07 || offset == 0x0d || offset == 0x0f) ret = irq_r(space, 0);
	if (offset == 0x15 || offset == 0x17 || offset == 0x1d || offset == 0x1f) ret = irq_r(space, 0);

	return ret;
}

WRITE8_MEMBER(mos6532_t::io_w)
{
	offset &= 0x1f;

	if (offset == 0x00 || offset == 0x08 || offset == 0x10 || offset == 0x18) pa_data_w(space, 0, data);
	if (offset == 0x01 || offset == 0x09 || offset == 0x11 || offset == 0x19) pa_ddr_w(space, 0, data);
	if (offset == 0x02 || offset == 0x0a || offset == 0x12 || offset == 0x1a) pb_data_w(space, 0, data);
	if (offset == 0x03 || offset == 0x0b || offset == 0x13 || offset == 0x1b) pb_ddr_w(space, 0, data);
	if (offset == 0x14 || offset == 0x15 || offset == 0x16 || offset == 0x17) timer_off_w(space, offset&3, data);
	if (offset == 0x1c || offset == 0x1d || offset == 0x1e || offset == 0x1f) timer_on_w(space, offset&3, data);

	if (offset == 0x04 || offset == 0x05 || offset == 0x06 || offset == 0x07) edge_w(space, offset&3, data);
	if (offset == 0x0c || offset == 0x0d || offset == 0xea || offset == 0x0f) edge_w(space, offset&3, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6530_base_t - constructor
//-------------------------------------------------

mos6530_base_t::mos6530_base_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_ram(*this),
	m_rom(*this, DEVICE_SELF),
	m_irq_cb(*this),
	m_in_pa_cb(*this),
	m_out_pa_cb(*this),
	m_in_pb_cb(*this),
	m_out_pb_cb(*this),
	m_in_pa0_cb(*this),
	m_in_pa1_cb(*this),
	m_in_pa2_cb(*this),
	m_in_pa3_cb(*this),
	m_in_pa4_cb(*this),
	m_in_pa5_cb(*this),
	m_in_pa6_cb(*this),
	m_in_pa7_cb(*this),
	m_out_pa0_cb(*this),
	m_out_pa1_cb(*this),
	m_out_pa2_cb(*this),
	m_out_pa3_cb(*this),
	m_out_pa4_cb(*this),
	m_out_pa5_cb(*this),
	m_out_pa6_cb(*this),
	m_out_pa7_cb(*this),
	m_in_pb0_cb(*this),
	m_in_pb1_cb(*this),
	m_in_pb2_cb(*this),
	m_in_pb3_cb(*this),
	m_in_pb4_cb(*this),
	m_in_pb5_cb(*this),
	m_in_pb6_cb(*this),
	m_in_pb7_cb(*this),
	m_out_pb0_cb(*this),
	m_out_pb1_cb(*this),
	m_out_pb2_cb(*this),
	m_out_pb3_cb(*this),
	m_out_pb4_cb(*this),
	m_out_pb5_cb(*this),
	m_out_pb6_cb(*this),
	m_out_pb7_cb(*this),
	m_pa_in(0xff),
	m_pa_out(0),
	m_pa_ddr(0),
	m_pa7(0),
	m_pa7_dir(0),
	m_pb_in(0xff),
	m_pb_out(0),
	m_pb_ddr(0),
	m_ie_timer(false),
	m_irq_timer(true),
	m_ie_edge(false),
	m_irq_edge(false)
{
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}


//-------------------------------------------------
//  mos6530_t - constructor
//-------------------------------------------------

mos6530_t::mos6530_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mos6530_base_t(mconfig, MOS6530n, "MOS6530n", tag, owner, clock, "mos6530n", __FILE__) { }


//-------------------------------------------------
//  mos6532_t - constructor
//-------------------------------------------------

mos6532_t::mos6532_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mos6530_base_t(mconfig, MOS6532n, "MOS6532n", tag, owner, clock, "mos6532n", __FILE__) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6530_base_t::device_start()
{
	// resolve callbacks
	m_irq_cb.resolve_safe();
	m_in_pa_cb.resolve();
	m_out_pa_cb.resolve();
	m_in_pb_cb.resolve();
	m_out_pb_cb.resolve();
	m_in_pa0_cb.resolve();
	m_in_pa1_cb.resolve();
	m_in_pa2_cb.resolve();
	m_in_pa3_cb.resolve();
	m_in_pa4_cb.resolve();
	m_in_pa5_cb.resolve();
	m_in_pa6_cb.resolve();
	m_in_pa7_cb.resolve();
	m_out_pa0_cb.resolve_safe();
	m_out_pa1_cb.resolve_safe();
	m_out_pa2_cb.resolve_safe();
	m_out_pa3_cb.resolve_safe();
	m_out_pa4_cb.resolve_safe();
	m_out_pa5_cb.resolve_safe();
	m_out_pa6_cb.resolve_safe();
	m_out_pa7_cb.resolve_safe();
	m_in_pb0_cb.resolve();
	m_in_pb1_cb.resolve();
	m_in_pb2_cb.resolve();
	m_in_pb3_cb.resolve();
	m_in_pb4_cb.resolve();
	m_in_pb5_cb.resolve();
	m_in_pb6_cb.resolve();
	m_in_pb7_cb.resolve();
	m_out_pb0_cb.resolve_safe();
	m_out_pb1_cb.resolve_safe();
	m_out_pb2_cb.resolve_safe();
	m_out_pb3_cb.resolve_safe();
	m_out_pb4_cb.resolve_safe();
	m_out_pb5_cb.resolve_safe();
	m_out_pb6_cb.resolve_safe();
	m_out_pb7_cb.resolve_safe();

	// allocate timer
	t_gen = timer_alloc(0);

	// state saving
	save_item(NAME(m_pa_in));
	save_item(NAME(m_pa_out));
	save_item(NAME(m_pa_ddr));
	save_item(NAME(m_pa7));
	save_item(NAME(m_pa7_dir));
	save_item(NAME(m_pb_in));
	save_item(NAME(m_pb_out));
	save_item(NAME(m_pb_ddr));
	save_item(NAME(m_ie_timer));
	save_item(NAME(m_irq_timer));
	save_item(NAME(m_ie_edge));
	save_item(NAME(m_irq_edge));
	save_item(NAME(m_prescale));
	save_item(NAME(m_timer));
}

void mos6530_t::device_start()
{
	mos6530_base_t::device_start();

	// allocate RAM
	m_ram.allocate(0x40);
}

void mos6532_t::device_start()
{
	mos6530_base_t::device_start();

	// allocate RAM
	m_ram.allocate(0x80);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6530_base_t::device_reset()
{
	m_pa_out = 0xff;
	m_pa_ddr = 0;
	m_pb_out = 0xff; // a7800 One-On-One Basketball (1on1u) needs this or you can't start a game, it doesn't initialize it.  (see MT6060)
	m_pb_ddr = 0;

	m_ie_timer = false;
	m_irq_timer = false;
	m_ie_edge = false;
	m_irq_edge = false;
	m_pa7_dir = 0;

	update_pa();
	update_pb();
	update_irq();
	edge_detect();

	m_timer = 0xff;
	m_prescale = 1024;

	if (cur_live.state != IDLE) {
		live_abort();
	}

	live_start();
	live_run();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mos6530_base_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();
	live_run();
}


//-------------------------------------------------
//  update_pa -
//-------------------------------------------------

void mos6530_base_t::update_pa()
{
	UINT8 out = m_pa_out;
	UINT8 ddr = m_pa_ddr;
	UINT8 data = (out & ddr) | (ddr ^ 0xff);

	if (m_out_pa_cb.isnull())
	{
		m_out_pa0_cb(BIT(data, 0));
		m_out_pa1_cb(BIT(data, 1));
		m_out_pa2_cb(BIT(data, 2));
		m_out_pa3_cb(BIT(data, 3));
		m_out_pa4_cb(BIT(data, 4));
		m_out_pa5_cb(BIT(data, 5));
		m_out_pa6_cb(BIT(data, 6));
		m_out_pa7_cb(BIT(data, 7));
	}
	else
	{
		m_out_pa_cb(data);
	}
}


//-------------------------------------------------
//  update_pb -
//-------------------------------------------------

void mos6530_base_t::update_pb()
{
	UINT8 out = m_pb_out;
	UINT8 ddr = m_pb_ddr;
	UINT8 data = (out & ddr) | (ddr ^ 0xff);

	if (m_out_pb_cb.isnull())
	{
		m_out_pb0_cb(BIT(data, 0));
		m_out_pb1_cb(BIT(data, 1));
		m_out_pb2_cb(BIT(data, 2));
		m_out_pb3_cb(BIT(data, 3));
		m_out_pb4_cb(BIT(data, 4));
		m_out_pb5_cb(BIT(data, 5));
		m_out_pb6_cb(BIT(data, 6));
		m_out_pb7_cb(BIT(data, 7));
	}
	else
	{
		m_out_pb_cb(data);
	}
}

void mos6530_t::update_pb()
{
	UINT8 out = m_pb_out;
	UINT8 ddr = m_pb_ddr;
	UINT8 data = (out & ddr) | (ddr ^ 0xff);

	if (m_ie_timer)
	{
		if (m_irq_timer) {
			data |= IRQ_TIMER;
		} else {
			data &= ~IRQ_TIMER;
		}
	}

	if (m_out_pb_cb.isnull())
	{
		m_out_pb0_cb(BIT(data, 0));
		m_out_pb1_cb(BIT(data, 1));
		m_out_pb2_cb(BIT(data, 2));
		m_out_pb3_cb(BIT(data, 3));
		m_out_pb4_cb(BIT(data, 4));
		m_out_pb5_cb(BIT(data, 5));
		m_out_pb6_cb(BIT(data, 6));
		m_out_pb7_cb(BIT(data, 7));
	}
	else
	{
		m_out_pb_cb(data);
	}
}


//-------------------------------------------------
//  update_irq -
//-------------------------------------------------

void mos6530_base_t::update_irq()
{
	int state = CLEAR_LINE;

	if (m_ie_timer && m_irq_timer) state = ASSERT_LINE;
	if (m_ie_edge && m_irq_edge) state = ASSERT_LINE;

	m_irq_cb(state);
}

void mos6530_t::update_irq()
{
	update_pb();
}


//-------------------------------------------------
//  get_irq_flags -
//-------------------------------------------------

UINT8 mos6530_base_t::get_irq_flags()
{
	UINT8 data = 0;

	if (m_irq_timer) data |= IRQ_TIMER;
	if (m_irq_edge) data |= IRQ_EDGE;

	return data;
}

UINT8 mos6530_t::get_irq_flags()
{
	UINT8 data = 0;

	if (m_irq_timer) data |= IRQ_TIMER;

	return data;
}


//-------------------------------------------------
//  edge_detect -
//-------------------------------------------------

void mos6530_base_t::edge_detect()
{
	UINT8 ddr_out = m_pa_ddr;
	UINT8 ddr_in = m_pa_ddr ^ 0xff;
	UINT8 data = (m_pa_out & ddr_out) | (m_pa_in & ddr_in);
	int state = BIT(data, 7);

	if ((m_pa7 ^ state) && (m_pa7_dir ^ state) == 0)
	{
		if (LOG) logerror("%s %s '%s' edge-detect IRQ\n", machine().time().as_string(), name(), tag());

		m_irq_edge = true;
		update_irq();
	}

	m_pa7 = state;
}


//-------------------------------------------------
//  pa_w -
//-------------------------------------------------

void mos6530_base_t::pa_w(int bit, int state)
{
	if (LOG) logerror("%s %s %s '%s' Port A Data Bit %u State %u\n", machine().time().as_string(), machine().describe_context(), name(), tag(), bit, state);

	m_pa_in &= ~(1 << bit);
	m_pa_in |= (state << bit);

	edge_detect();
}


//-------------------------------------------------
//  pb_w -
//-------------------------------------------------

void mos6530_base_t::pb_w(int bit, int state)
{
	if (LOG) logerror("%s %s %s '%s' Port B Data Bit %u State %u\n", machine().time().as_string(), machine().describe_context(), name(), tag(), bit, state);

	m_pb_in &= ~(1 << bit);
	m_pb_in |= (state << bit);
}


//-------------------------------------------------
//  pa_data_r -
//-------------------------------------------------

READ8_MEMBER( mos6530_base_t::pa_data_r )
{
	UINT8 in = 0;

	if (m_in_pa_cb.isnull())
	{
		in |= (m_in_pa0_cb.isnull() ? BIT(m_pa_in, 0) : m_in_pa0_cb());
		in |= (m_in_pa1_cb.isnull() ? BIT(m_pa_in, 1) : m_in_pa1_cb()) << 1;
		in |= (m_in_pa2_cb.isnull() ? BIT(m_pa_in, 2) : m_in_pa2_cb()) << 2;
		in |= (m_in_pa3_cb.isnull() ? BIT(m_pa_in, 3) : m_in_pa3_cb()) << 3;
		in |= (m_in_pa4_cb.isnull() ? BIT(m_pa_in, 4) : m_in_pa4_cb()) << 4;
		in |= (m_in_pa5_cb.isnull() ? BIT(m_pa_in, 5) : m_in_pa5_cb()) << 5;
		in |= (m_in_pa6_cb.isnull() ? BIT(m_pa_in, 6) : m_in_pa6_cb()) << 6;
		in |= (m_in_pa7_cb.isnull() ? BIT(m_pa_in, 7) : m_in_pa7_cb()) << 7;
	}
	else
	{
		in = m_in_pa_cb();
	}

	UINT8 out = m_pa_out;
	UINT8 ddr_out = m_pa_ddr;
	UINT8 ddr_in = m_pa_ddr ^ 0xff;
	UINT8 data = (out & ddr_out) | (in & ddr_in);

	if (LOG) logerror("%s %s %s '%s' Port A Data In %02x\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data);

	return data;
}


//-------------------------------------------------
//  pa_data_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::pa_data_w )
{
	m_pa_out = data;

	if (LOG) logerror("%s %s %s '%s' Port A Data Out %02x\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data);

	update_pa();
	edge_detect();
}


//-------------------------------------------------
//  pa_ddr_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::pa_ddr_w )
{
	m_pa_ddr = data;

	if (LOG) logerror("%s %s %s '%s' Port A DDR %02x\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data);

	update_pa();
	edge_detect();
}


//-------------------------------------------------
//  pb_data_r -
//-------------------------------------------------

READ8_MEMBER( mos6530_base_t::pb_data_r )
{
	UINT8 in = 0;

	if (m_in_pb_cb.isnull())
	{
		in |= (m_in_pb0_cb.isnull() ? BIT(m_pb_in, 0) : m_in_pb0_cb());
		in |= (m_in_pb1_cb.isnull() ? BIT(m_pb_in, 1) : m_in_pb1_cb()) << 1;
		in |= (m_in_pb2_cb.isnull() ? BIT(m_pb_in, 2) : m_in_pb2_cb()) << 2;
		in |= (m_in_pb3_cb.isnull() ? BIT(m_pb_in, 3) : m_in_pb3_cb()) << 3;
		in |= (m_in_pb4_cb.isnull() ? BIT(m_pb_in, 4) : m_in_pb4_cb()) << 4;
		in |= (m_in_pb5_cb.isnull() ? BIT(m_pb_in, 5) : m_in_pb5_cb()) << 5;
		in |= (m_in_pb6_cb.isnull() ? BIT(m_pb_in, 6) : m_in_pb6_cb()) << 6;
		in |= (m_in_pb7_cb.isnull() ? BIT(m_pb_in, 7) : m_in_pb7_cb()) << 7;
	}
	else
	{
		in = m_in_pb_cb();
	}

	UINT8 out = m_pb_out;
	UINT8 ddr_out = m_pb_ddr;
	UINT8 ddr_in = m_pb_ddr ^ 0xff;
	UINT8 data = (out & ddr_out) | (in & ddr_in);

	if (LOG) logerror("%s %s %s '%s' Port B Data In %02x\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data);

	return data;
}


//-------------------------------------------------
//  pb_data_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::pb_data_w )
{
	m_pb_out = data;

	if (LOG) logerror("%s %s %s '%s' Port B Data Out %02x\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data);

	update_pb();
}


//-------------------------------------------------
//  pb_ddr_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::pb_ddr_w )
{
	m_pb_ddr = data;

	if (LOG) logerror("%s %s %s '%s' Port B DDR %02x\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data);

	update_pb();
}


//-------------------------------------------------
//  timer_r -
//-------------------------------------------------

READ8_MEMBER( mos6530_base_t::timer_off_r )
{
	if (space.debugger_access())
		return 0;

	return timer_r(false);
}

READ8_MEMBER( mos6530_base_t::timer_on_r )
{
	if (space.debugger_access())
		return 0;

	return timer_r(true);
}

UINT8 mos6530_base_t::timer_r(bool ie)
{
	UINT8 data;

	live_sync();

	m_ie_timer = ie;
	if (cur_live.tm_irq != machine().time()) {
		m_irq_timer = false;
	}
	update_irq();

	data = cur_live.value;

	if (LOG_TIMER) logerror("%s %s %s '%s' Timer read %02x IE %u\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data, m_ie_timer ? 1 : 0);

	checkpoint();
	live_run();

	return data;
}


//-------------------------------------------------
//  irq_r -
//-------------------------------------------------

READ8_MEMBER( mos6530_base_t::irq_r )
{
	UINT8 data = get_irq_flags();

	if (!space.debugger_access()) {
		if (m_irq_edge) {
			m_irq_edge = false;
			update_irq();
		}
	}

	return data;
}


//-------------------------------------------------
//  timer_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::timer_off_w )
{
	timer_w(offset, data, false);
}

WRITE8_MEMBER( mos6530_base_t::timer_on_w )
{
	timer_w(offset, data, true);
}

void mos6530_base_t::timer_w(offs_t offset, UINT8 data, bool ie)
{
	live_sync();

	m_timer = data;

	switch (offset & 0x03) {
	case 0: m_prescale = 1; break;
	case 1: m_prescale = 8; break;
	case 2: m_prescale = 64; break;
	case 3: m_prescale = 1024; break;
	}

	m_ie_timer = ie;
	if (cur_live.tm_irq != machine().time()) {
		m_irq_timer = false;
	}
	update_irq();

	if (LOG_TIMER) logerror("%s %s %s '%s' Timer value %02x prescale %u IE %u\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data, m_prescale, m_ie_timer ? 1 : 0);

	checkpoint();

	if (cur_live.state != IDLE) {
		live_abort();
	}

	live_start();
	live_run();
}


//-------------------------------------------------
//  edge_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::edge_w )
{
	m_pa7_dir = BIT(data, 0);
	m_ie_edge = BIT(data, 1) ? false : true;

	if (LOG) logerror("%s %s %s '%s' %s edge-detect, %s interrupt\n", machine().time().as_string(), machine().describe_context(), name(), tag(), m_pa7_dir ? "positive" : "negative", m_ie_edge ? "enable" : "disable");
}


//-------------------------------------------------
//  live_start -
//-------------------------------------------------

void mos6530_base_t::live_start()
{
	cur_live.period = attotime::from_ticks(m_prescale, clock());
	cur_live.tm = machine().time() + attotime::from_hz(clock());
	cur_live.state = RUNNING;
	cur_live.next_state = -1;

	cur_live.value = m_timer;

	checkpoint();

	live_run();
}

void mos6530_base_t::checkpoint()
{
	checkpoint_live = cur_live;
}

void mos6530_base_t::rollback()
{
	cur_live = checkpoint_live;
}

void mos6530_base_t::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void mos6530_base_t::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
		} else {
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void mos6530_base_t::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.tm_irq = attotime::never;
}

void mos6530_base_t::live_run(const attotime &limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;) {
		switch(cur_live.state) {
		case RUNNING: {
			if (cur_live.tm > limit)
				return;

			cur_live.value--;

			if (cur_live.value == 0xff) {
				live_delay(RUNNING_SYNCPOINT);
				return;
			} else {
				if (LOG_TIMER) logerror("%s %s '%s' timer %02x\n", cur_live.tm.as_string(), name(), tag(), cur_live.value);

				cur_live.tm += cur_live.period;
			}
			break;
		}

		case RUNNING_SYNCPOINT: {
			if (LOG_TIMER) logerror("%s %s '%s' timer %02x interrupt\n", cur_live.tm.as_string(), name(), tag(), cur_live.value);

			cur_live.tm_irq = cur_live.tm;
			m_irq_timer = true;
			update_irq();

			checkpoint();

			cur_live.state = RUNNING_AFTER_INTERRUPT;
			cur_live.period = attotime::from_hz(clock());
			cur_live.tm += cur_live.period;
			break;
		}

		case RUNNING_AFTER_INTERRUPT: {
			if (cur_live.tm > limit)
				return;

			cur_live.value--;

			if (LOG_TIMER) logerror("%s %s '%s' timer %02x\n", cur_live.tm.as_string(), name(), tag(), cur_live.value);

			if (!cur_live.value) {
				cur_live.state = IDLE;
				return;
			}

			cur_live.tm += cur_live.period;
			break;
		}
		}
	}
}
