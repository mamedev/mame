// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6530 Memory, I/O, Timer Array emulation
    MOS Technology 6532 RAM, I/O, Timer Array emulation

**********************************************************************/

#include "emu.h"
#include "mos6530n.h"

#define LOG_GENERAL (1U << 0)
#define LOG_TIMER   (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_TIMER)
#include "logmacro.h"

#define LOGTIMER(...) LOGMASKED(LOG_TIMER, __VA_ARGS__)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS6530_NEW, mos6530_new_device, "mos6530_new", "MOS 6530 (new)")
DEFINE_DEVICE_TYPE(MOS6532_NEW, mos6532_new_device, "mos6532_new", "MOS 6532 (new)")


void mos6530_new_device::rom_map(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x000, 0x3ff).r(FUNC(mos6530_new_device::rom_r));
}

void mos6530_new_device::ram_map(address_map &map)
{
	map.global_mask(0x3f);
	map(0x00, 0x3f).rw(FUNC(mos6530_new_device::ram_r), FUNC(mos6530_new_device::ram_w));
}

void mos6530_new_device::io_map(address_map &map)
{
	map.global_mask(0xf);
	map(0x00, 0x00).mirror(0x8).rw(FUNC(mos6530_new_device::pa_data_r), FUNC(mos6530_new_device::pa_data_w));
	map(0x01, 0x01).mirror(0x8).rw(FUNC(mos6530_new_device::pa_ddr_r), FUNC(mos6530_new_device::pa_ddr_w));
	map(0x02, 0x02).mirror(0x8).rw(FUNC(mos6530_new_device::pb_data_r), FUNC(mos6530_new_device::pb_data_w));
	map(0x03, 0x03).mirror(0x8).rw(FUNC(mos6530_new_device::pb_ddr_r), FUNC(mos6530_new_device::pb_ddr_w));
	map(0x04, 0x07).w(FUNC(mos6530_new_device::timer_off_w));
	map(0x0c, 0x0f).w(FUNC(mos6530_new_device::timer_on_w));
	map(0x04, 0x04).mirror(0x2).r(FUNC(mos6530_new_device::timer_off_r));
	map(0x0c, 0x0c).mirror(0x2).r(FUNC(mos6530_new_device::timer_on_r));
	map(0x05, 0x05).mirror(0xa).r(FUNC(mos6530_new_device::irq_r));
}

void mos6532_new_device::ram_map(address_map &map)
{
	map.global_mask(0x7f);
	map(0x00, 0x7f).rw(FUNC(mos6532_new_device::ram_r), FUNC(mos6532_new_device::ram_w));
}

void mos6532_new_device::io_map(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x00).mirror(0x18).rw(FUNC(mos6532_new_device::pa_data_r), FUNC(mos6532_new_device::pa_data_w));  // SWCHA
	map(0x01, 0x01).mirror(0x18).rw(FUNC(mos6532_new_device::pa_ddr_r), FUNC(mos6532_new_device::pa_ddr_w));    // SWACNT
	map(0x02, 0x02).mirror(0x18).rw(FUNC(mos6532_new_device::pb_data_r), FUNC(mos6532_new_device::pb_data_w));  // SWCHB
	map(0x03, 0x03).mirror(0x18).rw(FUNC(mos6532_new_device::pb_ddr_r), FUNC(mos6532_new_device::pb_ddr_w));    // SWBCNT
	map(0x14, 0x17).w(FUNC(mos6532_new_device::timer_off_w));
	map(0x1c, 0x1f).w(FUNC(mos6532_new_device::timer_on_w));
	map(0x04, 0x04).mirror(0x12).r(FUNC(mos6532_new_device::timer_off_r));
	map(0x0c, 0x0c).mirror(0x12).r(FUNC(mos6532_new_device::timer_on_r));
	map(0x05, 0x05).mirror(0x1a).r(FUNC(mos6532_new_device::irq_r));
	map(0x04, 0x07).mirror(0x8).w(FUNC(mos6532_new_device::edge_w));
}

uint8_t mos6532_new_device::io_r(offs_t offset)
{
	offset &= 0x1f;
	uint8_t ret = 0;

	if (offset == 0x00 || offset == 0x08 || offset == 0x10 || offset == 0x18) ret = pa_data_r();
	if (offset == 0x01 || offset == 0x09 || offset == 0x11 || offset == 0x19) ret = pa_ddr_r();
	if (offset == 0x02 || offset == 0x0a || offset == 0x12 || offset == 0x1a) ret = pb_data_r();
	if (offset == 0x03 || offset == 0x0b || offset == 0x13 || offset == 0x1b) ret = pb_ddr_r();

	if (offset == 0x04 || offset == 0x06 || offset == 0x14 || offset == 0x16) ret = timer_off_r();
	if (offset == 0x0c || offset == 0x0e || offset == 0x1c || offset == 0x1e) ret = timer_on_r();

	if (offset == 0x05 || offset == 0x07 || offset == 0x0d || offset == 0x0f) ret = irq_r();
	if (offset == 0x15 || offset == 0x17 || offset == 0x1d || offset == 0x1f) ret = irq_r();

	return ret;
}

void mos6532_new_device::io_w(offs_t offset, uint8_t data)
{
	offset &= 0x1f;

	if (offset == 0x00 || offset == 0x08 || offset == 0x10 || offset == 0x18) pa_data_w(data);
	if (offset == 0x01 || offset == 0x09 || offset == 0x11 || offset == 0x19) pa_ddr_w(data);
	if (offset == 0x02 || offset == 0x0a || offset == 0x12 || offset == 0x1a) pb_data_w(data);
	if (offset == 0x03 || offset == 0x0b || offset == 0x13 || offset == 0x1b) pb_ddr_w(data);
	if (offset == 0x14 || offset == 0x15 || offset == 0x16 || offset == 0x17) timer_off_w(offset&3, data);
	if (offset == 0x1c || offset == 0x1d || offset == 0x1e || offset == 0x1f) timer_on_w(offset&3, data);

	if (offset == 0x04 || offset == 0x05 || offset == 0x06 || offset == 0x07) edge_w(data);
	if (offset == 0x0c || offset == 0x0d || offset == 0xea || offset == 0x0f) edge_w(data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6530_device_base - constructor
//-------------------------------------------------

mos6530_device_base::mos6530_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u32 rsize) :
	device_t(mconfig, type, tag, owner, clock),
	m_ram(*this, finder_base::DUMMY_TAG, rsize, ENDIANNESS_LITTLE),
	m_rom(*this, DEVICE_SELF),
	m_irq_cb(*this),
	m_in8_pa_cb(*this),
	m_out8_pa_cb(*this),
	m_in8_pb_cb(*this),
	m_out8_pb_cb(*this),
	m_in_pa_cb(*this),
	m_out_pa_cb(*this),
	m_in_pb_cb(*this),
	m_out_pb_cb(*this),
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
//  mos6530_new_device - constructor
//-------------------------------------------------

mos6530_new_device::mos6530_new_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6530_device_base(mconfig, MOS6530_NEW, tag, owner, clock, 0x40) { }


//-------------------------------------------------
//  mos6532_new_device - constructor
//-------------------------------------------------

mos6532_new_device::mos6532_new_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos6530_device_base(mconfig, MOS6532_NEW, tag, owner, clock, 0x80) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6530_device_base::device_start()
{
	// resolve callbacks
	m_irq_cb.resolve_safe();
	m_in8_pa_cb.resolve();
	m_out8_pa_cb.resolve();
	m_in8_pb_cb.resolve();
	m_out8_pb_cb.resolve();
	m_in_pa_cb.resolve_all();
	m_out_pa_cb.resolve_all_safe();
	m_in_pb_cb.resolve_all();
	m_out_pb_cb.resolve_all_safe();

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


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6530_device_base::device_reset()
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

void mos6530_device_base::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	live_sync();
	live_run();
}


//-------------------------------------------------
//  update_pa -
//-------------------------------------------------

void mos6530_device_base::update_pa()
{
	uint8_t out = m_pa_out;
	uint8_t ddr = m_pa_ddr;
	uint8_t data = (out & ddr) | (ddr ^ 0xff);

	if (m_out8_pa_cb.isnull())
	{
		m_out_pa_cb[0](BIT(data, 0));
		m_out_pa_cb[1](BIT(data, 1));
		m_out_pa_cb[2](BIT(data, 2));
		m_out_pa_cb[3](BIT(data, 3));
		m_out_pa_cb[4](BIT(data, 4));
		m_out_pa_cb[5](BIT(data, 5));
		m_out_pa_cb[6](BIT(data, 6));
		m_out_pa_cb[7](BIT(data, 7));
	}
	else
	{
		m_out8_pa_cb(data);
	}
}


//-------------------------------------------------
//  update_pb -
//-------------------------------------------------

void mos6530_device_base::update_pb()
{
	uint8_t out = m_pb_out;
	uint8_t ddr = m_pb_ddr;
	uint8_t data = (out & ddr) | (ddr ^ 0xff);

	if (m_out8_pb_cb.isnull())
	{
		m_out_pb_cb[0](BIT(data, 0));
		m_out_pb_cb[1](BIT(data, 1));
		m_out_pb_cb[2](BIT(data, 2));
		m_out_pb_cb[3](BIT(data, 3));
		m_out_pb_cb[4](BIT(data, 4));
		m_out_pb_cb[5](BIT(data, 5));
		m_out_pb_cb[6](BIT(data, 6));
		m_out_pb_cb[7](BIT(data, 7));
	}
	else
	{
		m_out8_pb_cb(data);
	}
}

void mos6530_new_device::update_pb()
{
	uint8_t out = m_pb_out;
	uint8_t ddr = m_pb_ddr;
	uint8_t data = (out & ddr) | (ddr ^ 0xff);

	if (m_ie_timer)
	{
		if (m_irq_timer) {
			data |= IRQ_TIMER;
		} else {
			data &= ~IRQ_TIMER;
		}
	}

	if (m_out8_pb_cb.isnull())
	{
		m_out_pb_cb[0](BIT(data, 0));
		m_out_pb_cb[1](BIT(data, 1));
		m_out_pb_cb[2](BIT(data, 2));
		m_out_pb_cb[3](BIT(data, 3));
		m_out_pb_cb[4](BIT(data, 4));
		m_out_pb_cb[5](BIT(data, 5));
		m_out_pb_cb[6](BIT(data, 6));
		m_out_pb_cb[7](BIT(data, 7));
	}
	else
	{
		m_out8_pb_cb(data);
	}
}


//-------------------------------------------------
//  update_irq -
//-------------------------------------------------

void mos6530_device_base::update_irq()
{
	int state = CLEAR_LINE;

	if (m_ie_timer && m_irq_timer) state = ASSERT_LINE;
	if (m_ie_edge && m_irq_edge) state = ASSERT_LINE;

	m_irq_cb(state);
}

void mos6530_new_device::update_irq()
{
	update_pb();
}


//-------------------------------------------------
//  get_irq_flags -
//-------------------------------------------------

uint8_t mos6530_device_base::get_irq_flags()
{
	uint8_t data = 0;

	if (m_irq_timer) data |= IRQ_TIMER;
	if (m_irq_edge) data |= IRQ_EDGE;

	return data;
}

uint8_t mos6530_new_device::get_irq_flags()
{
	uint8_t data = 0;

	if (m_irq_timer) data |= IRQ_TIMER;

	return data;
}


//-------------------------------------------------
//  edge_detect -
//-------------------------------------------------

void mos6530_device_base::edge_detect()
{
	uint8_t ddr_out = m_pa_ddr;
	uint8_t ddr_in = m_pa_ddr ^ 0xff;
	uint8_t data = (m_pa_out & ddr_out) | (m_pa_in & ddr_in);
	int state = BIT(data, 7);

	if ((m_pa7 ^ state) && (m_pa7_dir ^ state) == 0 && !m_irq_edge)
	{
		LOG("%s %s edge-detect IRQ\n", machine().time().as_string(), name());

		m_irq_edge = true;
		update_irq();
	}

	m_pa7 = state;
}


//-------------------------------------------------
//  pa_w -
//-------------------------------------------------

void mos6530_device_base::pa_w(int bit, int state)
{
	LOG("%s %s %s Port A Data Bit %u State %u\n", machine().time().as_string(), machine().describe_context(), name(), bit, state);

	m_pa_in &= ~(1 << bit);
	m_pa_in |= (state << bit);

	edge_detect();
}


//-------------------------------------------------
//  pb_w -
//-------------------------------------------------

void mos6530_device_base::pb_w(int bit, int state)
{
	LOG("%s %s %s Port B Data Bit %u State %u\n", machine().time().as_string(), machine().describe_context(), name(), bit, state);

	m_pb_in &= ~(1 << bit);
	m_pb_in |= (state << bit);
}


//-------------------------------------------------
//  pa_data_r -
//-------------------------------------------------

uint8_t mos6530_device_base::pa_data_r()
{
	uint8_t in = 0;

	if (m_in8_pa_cb.isnull())
	{
		in |= (m_in_pa_cb[0].isnull() ? BIT(m_pa_in, 0) : m_in_pa_cb[0]());
		in |= (m_in_pa_cb[1].isnull() ? BIT(m_pa_in, 1) : m_in_pa_cb[1]()) << 1;
		in |= (m_in_pa_cb[2].isnull() ? BIT(m_pa_in, 2) : m_in_pa_cb[2]()) << 2;
		in |= (m_in_pa_cb[3].isnull() ? BIT(m_pa_in, 3) : m_in_pa_cb[3]()) << 3;
		in |= (m_in_pa_cb[4].isnull() ? BIT(m_pa_in, 4) : m_in_pa_cb[4]()) << 4;
		in |= (m_in_pa_cb[5].isnull() ? BIT(m_pa_in, 5) : m_in_pa_cb[5]()) << 5;
		in |= (m_in_pa_cb[6].isnull() ? BIT(m_pa_in, 6) : m_in_pa_cb[6]()) << 6;
		in |= (m_in_pa_cb[7].isnull() ? BIT(m_pa_in, 7) : m_in_pa_cb[7]()) << 7;
	}
	else
	{
		in = m_in8_pa_cb();
	}

	uint8_t out = m_pa_out;
	uint8_t ddr_out = m_pa_ddr;
	uint8_t ddr_in = m_pa_ddr ^ 0xff;
	uint8_t data = (out & ddr_out) | (in & ddr_in);

	LOG("%s %s %s Port A Data In %02x\n", machine().time().as_string(), machine().describe_context(), name(), data);

	return data;
}


//-------------------------------------------------
//  pa_data_w -
//-------------------------------------------------

void mos6530_device_base::pa_data_w(uint8_t data)
{
	m_pa_out = data;

	LOG("%s %s %s Port A Data Out %02x\n", machine().time().as_string(), machine().describe_context(), name(), data);

	update_pa();
	edge_detect();
}


//-------------------------------------------------
//  pa_ddr_w -
//-------------------------------------------------

void mos6530_device_base::pa_ddr_w(uint8_t data)
{
	m_pa_ddr = data;

	LOG("%s %s %s Port A DDR %02x\n", machine().time().as_string(), machine().describe_context(), name(), data);

	update_pa();
	edge_detect();
}


//-------------------------------------------------
//  pb_data_r -
//-------------------------------------------------

uint8_t mos6530_device_base::pb_data_r()
{
	uint8_t in = 0;

	if (m_in8_pb_cb.isnull())
	{
		in |= (m_in_pb_cb[0].isnull() ? BIT(m_pb_in, 0) : m_in_pb_cb[0]());
		in |= (m_in_pb_cb[1].isnull() ? BIT(m_pb_in, 1) : m_in_pb_cb[1]()) << 1;
		in |= (m_in_pb_cb[2].isnull() ? BIT(m_pb_in, 2) : m_in_pb_cb[2]()) << 2;
		in |= (m_in_pb_cb[3].isnull() ? BIT(m_pb_in, 3) : m_in_pb_cb[3]()) << 3;
		in |= (m_in_pb_cb[4].isnull() ? BIT(m_pb_in, 4) : m_in_pb_cb[4]()) << 4;
		in |= (m_in_pb_cb[5].isnull() ? BIT(m_pb_in, 5) : m_in_pb_cb[5]()) << 5;
		in |= (m_in_pb_cb[6].isnull() ? BIT(m_pb_in, 6) : m_in_pb_cb[6]()) << 6;
		in |= (m_in_pb_cb[7].isnull() ? BIT(m_pb_in, 7) : m_in_pb_cb[7]()) << 7;
	}
	else
	{
		in = m_in8_pb_cb();
	}

	uint8_t out = m_pb_out;
	uint8_t ddr_out = m_pb_ddr;
	uint8_t ddr_in = m_pb_ddr ^ 0xff;
	uint8_t data = (out & ddr_out) | (in & ddr_in);

	LOG("%s %s %s Port B Data In %02x\n", machine().time().as_string(), machine().describe_context(), name(), data);

	return data;
}


//-------------------------------------------------
//  pb_data_w -
//-------------------------------------------------

void mos6530_device_base::pb_data_w(uint8_t data)
{
	m_pb_out = data;

	LOG("%s %s %s Port B Data Out %02x\n", machine().time().as_string(), machine().describe_context(), name(), data);

	update_pb();
}


//-------------------------------------------------
//  pb_ddr_w -
//-------------------------------------------------

void mos6530_device_base::pb_ddr_w(uint8_t data)
{
	m_pb_ddr = data;

	LOG("%s %s %s Port B DDR %02x\n", machine().time().as_string(), machine().describe_context(), name(), data);

	update_pb();
}


//-------------------------------------------------
//  timer_r -
//-------------------------------------------------

uint8_t mos6530_device_base::timer_off_r()
{
	if (machine().side_effects_disabled())
		return 0;

	return timer_r(false);
}

uint8_t mos6530_device_base::timer_on_r()
{
	if (machine().side_effects_disabled())
		return 0;

	return timer_r(true);
}

uint8_t mos6530_device_base::timer_r(bool ie)
{
	uint8_t data;

	live_sync();

	m_ie_timer = ie;
	if (cur_live.tm_irq != machine().time()) {
		m_irq_timer = false;
	}
	update_irq();

	data = cur_live.value;

	LOGTIMER("%s %s %s Timer read %02x IE %u\n", machine().time().as_string(), machine().describe_context(), name(), data, m_ie_timer ? 1 : 0);

	checkpoint();
	live_run();

	return data;
}


//-------------------------------------------------
//  irq_r -
//-------------------------------------------------

uint8_t mos6530_device_base::irq_r()
{
	uint8_t data = get_irq_flags();

	if (!machine().side_effects_disabled()) {
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

void mos6530_device_base::timer_off_w(offs_t offset, uint8_t data)
{
	timer_w(offset, data, false);
}

void mos6530_device_base::timer_on_w(offs_t offset, uint8_t data)
{
	timer_w(offset, data, true);
}

void mos6530_device_base::timer_w(offs_t offset, uint8_t data, bool ie)
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

	LOGTIMER("%s %s %s Timer value %02x prescale %u IE %u\n", machine().time().as_string(), machine().describe_context(), name(), data, m_prescale, m_ie_timer ? 1 : 0);

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

void mos6530_device_base::edge_w(uint8_t data)
{
	m_pa7_dir = BIT(data, 0);
	m_ie_edge = BIT(data, 1) ? false : true;

	LOG("%s %s %s %s edge-detect, %s interrupt\n", machine().time().as_string(), machine().describe_context(), name(), m_pa7_dir ? "positive" : "negative", m_ie_edge ? "enable" : "disable");
}


//-------------------------------------------------
//  live_start -
//-------------------------------------------------

void mos6530_device_base::live_start()
{
	cur_live.period = attotime::from_ticks(m_prescale, clock());
	cur_live.tm = machine().time() + attotime::from_hz(clock());
	cur_live.state = RUNNING;
	cur_live.next_state = -1;

	cur_live.value = m_timer;

	checkpoint();

	live_run();
}

void mos6530_device_base::checkpoint()
{
	checkpoint_live = cur_live;
}

void mos6530_device_base::rollback()
{
	cur_live = checkpoint_live;
}

void mos6530_device_base::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void mos6530_device_base::live_sync()
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

void mos6530_device_base::live_abort()
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

void mos6530_device_base::live_run(const attotime &limit)
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
				LOGTIMER("%s %s timer %02x\n", cur_live.tm.as_string(), name(), cur_live.value);

				cur_live.tm += cur_live.period;
			}
			break;
		}

		case RUNNING_SYNCPOINT: {
			LOGTIMER("%s %s timer %02x interrupt\n", cur_live.tm.as_string(), name(), cur_live.value);

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

			LOGTIMER("%s %s timer %02x\n", cur_live.tm.as_string(), name(), cur_live.value);

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
