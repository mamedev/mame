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
	AM_RANGE(0x000, 0x3ff) AM_READ(rom_r)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( ram_map, 8, mos6530_t )
	AM_RANGE(0x00, 0x3f) AM_READWRITE(ram_r, ram_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( io_map, 8, mos6530_t )
	AM_RANGE(0x00, 0x00) AM_READWRITE(pa_data_r, pa_data_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(pa_ddr_r, pa_ddr_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(pb_data_r, pb_data_w)
	AM_RANGE(0x03, 0x03) AM_READWRITE(pb_ddr_r, pb_ddr_w)
	AM_RANGE(0x04, 0x0f) AM_READ(timer_r)
	AM_RANGE(0x04, 0x0f) AM_WRITE(timer_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( ram_map, 8, mos6532_t )
	AM_RANGE(0x00, 0x7f) AM_READWRITE(ram_r, ram_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( io_map, 8, mos6532_t )
	AM_RANGE(0x00, 0x00) AM_READWRITE(pa_data_r, pa_data_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(pa_ddr_r, pa_ddr_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(pb_data_r, pb_data_w)
	AM_RANGE(0x03, 0x03) AM_READWRITE(pb_ddr_r, pb_ddr_w)
	AM_RANGE(0x04, 0x0f) AM_READ(timer_r)
	AM_RANGE(0x04, 0x07) AM_WRITE(edge_w)
	AM_RANGE(0x14, 0x1f) AM_WRITE(timer_w)
ADDRESS_MAP_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6530_base_t - constructor
//-------------------------------------------------

mos6530_base_t::mos6530_base_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_ram(*this),
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
	m_pb_in(0xff),
	m_pb_out(0),
	m_pb_ddr(0),
	m_ie_timer(false),
	m_irq_timer(true),
	m_ie_edge(false),
	m_irq_edge(false),
	m_positive(false)
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
	save_item(NAME(m_pb_in));
	save_item(NAME(m_pb_out));
	save_item(NAME(m_pb_ddr));
	save_item(NAME(m_ie_timer));
	save_item(NAME(m_irq_timer));
	save_item(NAME(m_ie_edge));
	save_item(NAME(m_irq_edge));
	save_item(NAME(m_positive));
	save_item(NAME(m_shift));
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
	m_pa_out = 0;
	m_pa_ddr = 0;
	m_pb_out = 0;
	m_pb_ddr = 0;

	m_ie_timer = false;
	m_irq_timer = true;
	m_ie_edge = false;
	m_irq_edge = false;
	m_positive = false;

	update_pa();
	update_pb();

	live_abort();
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

void mos6530_base_t::edge_detect(int old, int state)
{
	if (!m_irq_edge)
	{
		if (m_positive && !old && state) m_irq_edge = true;
		if (!m_positive && old && !state) m_irq_edge = true;

		if (m_irq_edge) {
			if (LOG) logerror("%s %s '%s' edge-detect IRQ\n", machine().time().as_string(), name(), tag());

			update_irq();
		}
	}
}


//-------------------------------------------------
//  pa_w -
//-------------------------------------------------

void mos6530_base_t::pa_w(int bit, int state)
{
	if (LOG) logerror("%s %s %s '%s' Port A Data Bit %u State %u\n", machine().time().as_string(), machine().describe_context(), name(), tag(), bit, state);

	m_pa_in &= ~(1 << bit);
	m_pa_in |= (state << bit);
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
}

WRITE8_MEMBER( mos6532_t::pa_data_w )
{
	edge_detect(BIT(m_pa_out, 7), BIT(data, 7));

	mos6530_base_t::pa_data_w(space, offset, data);
}


//-------------------------------------------------
//  pa_ddr_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::pa_ddr_w )
{
	m_pa_ddr = data;

	if (LOG) logerror("%s %s %s '%s' Port A DDR %02x\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data);

	update_pa();
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

READ8_MEMBER( mos6530_base_t::timer_r )
{
	UINT8 data = 0;

	if (space.debugger_access())
		return 0;

	if (offset & 0x01)
	{
		data |= get_irq_flags();
	}
	else
	{
		live_sync();

		data = cur_live.value;

		checkpoint();
		live_run();

		m_ie_timer = BIT(offset, 3) ? true : false;
	}

	bool irq_dirty = false;

	if (m_irq_timer) {
		m_irq_timer = false;
		irq_dirty = true;
	}

	if (m_irq_edge) {
		m_irq_edge = false;
		irq_dirty = true;
	}

	if (irq_dirty) {
		update_irq();
	}

	return data;
}


//-------------------------------------------------
//  timer_w -
//-------------------------------------------------

WRITE8_MEMBER( mos6530_base_t::timer_w )
{
	live_sync();

	m_timer = data;

	switch (offset & 0x03) {
	case 0: m_shift = 1; break;
	case 1: m_shift = 8; break;
	case 2: m_shift = 64; break;
	case 3: m_shift = 1024; break;
	}

	m_ie_timer = BIT(offset, 3) ? true : false;

	if (LOG_TIMER) logerror("%s %s %s '%s' Timer value %02x shift %u IE %u\n", machine().time().as_string(), machine().describe_context(), name(), tag(), data, m_shift, m_ie_timer ? 1 : 0);

	if (m_irq_timer) {
		m_irq_timer = false;
		update_irq();
	}

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
	m_positive = BIT(data, 0) ? true : false;
	m_ie_edge = BIT(data, 1) ? false : true;

	if (LOG) logerror("%s %s %s '%s' %s edge-detect, %s interrupt\n", machine().time().as_string(), machine().describe_context(), name(), tag(), m_positive ? "positive" : "negative", m_ie_edge ? "enable" : "disable");
}


//-------------------------------------------------
//  pa7_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( mos6532_t::pa7_w )
{
	edge_detect(BIT(m_pa_in, 7), state);

	mos6530_base_t::pa7_w(state);
}


//-------------------------------------------------
//  live_start -
//-------------------------------------------------

void mos6530_base_t::live_start()
{
	cur_live.period = attotime::from_hz(clock() / m_shift);
	cur_live.tm = machine().time() + cur_live.period;
	cur_live.state = RUNNING;
	cur_live.next_state = -1;

	cur_live.value = m_timer;
	cur_live.irq = false;

	checkpoint_live = cur_live;

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

	cur_live.irq = false;
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

			if (LOG_TIMER) logerror("%s %s '%s' timer %02x IRQ 1\n", cur_live.tm.as_string(), name(), tag(), cur_live.value);

			if (!cur_live.value) {
				cur_live.period = attotime::from_hz(clock());
				cur_live.state = RUNNING_INTERRUPT;
			}

			cur_live.tm += cur_live.period;
			break;
		}

		case RUNNING_INTERRUPT: {
			if (cur_live.tm > limit)
				return;

			cur_live.value--;
			cur_live.irq = true;

			if (LOG_TIMER) logerror("%s %s '%s' timer %02x IRQ 0\n", cur_live.tm.as_string(), name(), tag(), cur_live.value);

			live_delay(RUNNING_SYNCPOINT);

			cur_live.tm += cur_live.period;
			return;
		}

		case RUNNING_SYNCPOINT: {
			if (LOG_TIMER) logerror("%s %s '%s' timer IRQ\n", machine().time().as_string(), name(), tag());

			m_irq_timer = true;
			update_irq();

			cur_live.state = RUNNING_AFTER_INTERRUPT;
			checkpoint();
			break;
		}

		case RUNNING_AFTER_INTERRUPT: {
			if (cur_live.tm > limit)
				return;

			cur_live.value--;

			if (LOG_TIMER) logerror("%s %s '%s' timer %02x IRQ 0\n", cur_live.tm.as_string(), name(), tag(), cur_live.value);

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
