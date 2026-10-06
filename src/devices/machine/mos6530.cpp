// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6530 MIOT (Memory, I/O, Timer Array)
    Rockwell calls it RRIOT: ROM, RAM, I/O, Timer

    MOS Technology 6532 RIOT (RAM, I/O, Timer Array)

**********************************************************************/

#include "emu.h"
#include "mos6530.h"

#define LOG_TIMER   (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_TIMER)
#include "logmacro.h"

#define LOGTIMER(...) LOGMASKED(LOG_TIMER, __VA_ARGS__)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS6530, mos6530_device, "mos6530", "MOS 6530 MIOT")
DEFINE_DEVICE_TYPE(MOS6532, mos6532_device, "mos6532", "MOS 6532 RIOT")


void mos6530_device::rom_map(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x000, 0x3ff).r(FUNC(mos6530_device::rom_r));
}

void mos6530_device::ram_map(address_map &map)
{
	map.global_mask(0x3f);
	map(0x00, 0x3f).rw(FUNC(mos6530_device::ram_r), FUNC(mos6530_device::ram_w));
}

void mos6530_device::io_map(address_map &map)
{
	map.global_mask(0xf);
	map(0x00, 0x00).mirror(0x8).rw(FUNC(mos6530_device::pa_data_r), FUNC(mos6530_device::pa_data_w));
	map(0x01, 0x01).mirror(0x8).rw(FUNC(mos6530_device::pa_ddr_r), FUNC(mos6530_device::pa_ddr_w));
	map(0x02, 0x02).mirror(0x8).rw(FUNC(mos6530_device::pb_data_r), FUNC(mos6530_device::pb_data_w));
	map(0x03, 0x03).mirror(0x8).rw(FUNC(mos6530_device::pb_ddr_r), FUNC(mos6530_device::pb_ddr_w));
	map(0x04, 0x07).w(FUNC(mos6530_device::timer_off_w));
	map(0x0c, 0x0f).w(FUNC(mos6530_device::timer_on_w));
	map(0x04, 0x04).mirror(0x2).r(FUNC(mos6530_device::timer_off_r));
	map(0x0c, 0x0c).mirror(0x2).r(FUNC(mos6530_device::timer_on_r));
	map(0x05, 0x05).mirror(0xa).r(FUNC(mos6530_device::irq_r));
}

void mos6532_device::ram_map(address_map &map)
{
	map.global_mask(0x7f);
	map(0x00, 0x7f).rw(FUNC(mos6532_device::ram_r), FUNC(mos6532_device::ram_w));
}

void mos6532_device::io_map(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x00).mirror(0x18).rw(FUNC(mos6532_device::pa_data_r), FUNC(mos6532_device::pa_data_w));
	map(0x01, 0x01).mirror(0x18).rw(FUNC(mos6532_device::pa_ddr_r), FUNC(mos6532_device::pa_ddr_w));
	map(0x02, 0x02).mirror(0x18).rw(FUNC(mos6532_device::pb_data_r), FUNC(mos6532_device::pb_data_w));
	map(0x03, 0x03).mirror(0x18).rw(FUNC(mos6532_device::pb_ddr_r), FUNC(mos6532_device::pb_ddr_w));
	map(0x14, 0x17).w(FUNC(mos6532_device::timer_off_w));
	map(0x1c, 0x1f).w(FUNC(mos6532_device::timer_on_w));
	map(0x04, 0x04).mirror(0x12).r(FUNC(mos6532_device::timer_off_r));
	map(0x0c, 0x0c).mirror(0x12).r(FUNC(mos6532_device::timer_on_r));
	map(0x05, 0x05).mirror(0x1a).r(FUNC(mos6532_device::irq_r));
	map(0x04, 0x07).mirror(0x8).w(FUNC(mos6532_device::edge_w));
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
	m_in8_pa_cb(*this, 0),
	m_out8_pa_cb(*this),
	m_in8_pb_cb(*this, 0),
	m_out8_pb_cb(*this),
	m_in_pa_cb(*this, 0),
	m_out_pa_cb(*this),
	m_in_pb_cb(*this, 0),
	m_out_pb_cb(*this),
	m_pa_in(0xff),
	m_pa_out(0),
	m_pa_ddr(0),
	m_pa7(1),
	m_pa7_dir(0),
	m_pb_in(0xff),
	m_pb_out(0),
	m_pb_ddr(0),
	m_ie_timer(false),
	m_irq_timer(false),
	m_ie_edge(false),
	m_irq_edge(false)
{ }


//-------------------------------------------------
//  mos6530_device - constructor
//-------------------------------------------------

mos6530_device::mos6530_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mos6530_device_base(mconfig, MOS6530, tag, owner, clock, 0x40)
{ }


//-------------------------------------------------
//  mos6532_device - constructor
//-------------------------------------------------

mos6532_device::mos6532_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mos6530_device_base(mconfig, MOS6532, tag, owner, clock, 0x80)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6530_device_base::device_start()
{
	// allocate timer
	m_timer = timer_alloc(FUNC(mos6530_device_base::timer_end), this);
	m_timershift = 10;
	m_timerstate = TIMER_COUNTING;
	m_timer->adjust(attotime::from_ticks(256 << m_timershift, clock()));

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

	save_item(NAME(m_timershift));
	save_item(NAME(m_timerstate));
	save_item(NAME(m_timeout));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6530_device_base::device_reset()
{
	m_pa_out = 0;
	m_pa_ddr = 0;
	m_pb_out = 0;
	m_pb_ddr = 0;

	m_ie_timer = false;
	m_ie_edge = false;
	m_irq_edge = false;
	m_pa7_dir = 0;

	update_pa();
	update_pb();
	update_irq();
	edge_detect();
}


//-------------------------------------------------
//  update_pa -
//-------------------------------------------------

void mos6530_device_base::update_pa()
{
	uint8_t data = (m_pa_out & m_pa_ddr) | (m_pa_ddr ^ 0xff);

	if (m_out8_pa_cb.isunset())
	{
		for (int i = 0; i < 8; i++)
			m_out_pa_cb[i](BIT(data, i));
	}
	else
		m_out8_pa_cb(data);
}


//-------------------------------------------------
//  update_pb -
//-------------------------------------------------

void mos6530_device_base::update_pb()
{
	uint8_t data = (m_pb_out & m_pb_ddr) | (m_pb_ddr ^ 0xff);

	if (m_out8_pb_cb.isunset())
	{
		for (int i = 0; i < 8; i++)
			m_out_pb_cb[i](BIT(data, i));
	}
	else
		m_out8_pb_cb(data);
}

void mos6530_device::update_pb()
{
	uint8_t data = (m_pb_out & m_pb_ddr) | (m_pb_ddr ^ 0xff);

	if (!BIT(m_pb_ddr, 7))
	{
		// active low!
		if (m_ie_timer && m_irq_timer)
			data &= ~IRQ_TIMER;
		else
			data |= IRQ_TIMER;
	}

	if (m_out8_pb_cb.isunset())
	{
		for (int i = 0; i < 8; i++)
			m_out_pb_cb[i](BIT(data, i));
	}
	else
		m_out8_pb_cb(data);

	m_irq_cb(BIT(data, 7) ? CLEAR_LINE: ASSERT_LINE);
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

void mos6530_device::update_irq()
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

uint8_t mos6530_device::get_irq_flags()
{
	uint8_t data = 0;

	if (m_irq_timer) data |= IRQ_TIMER;

	return data;
}


//-------------------------------------------------
//  get_timer - return the current timer value
//-------------------------------------------------

uint8_t mos6530_device_base::get_timer()
{
	// determine the number of ticks remaining
	uint8_t shift = (m_timerstate == TIMER_COUNTING) ? m_timershift : 0;
	int64_t remain = m_timer->remaining().as_ticks(clock());
	uint8_t val = remain >> shift;

	// timeout is at 255, so round it down
	return (remain & ((1 << shift) - 1)) ? val : (val - 1);
}


//-------------------------------------------------
//  timer_start - restart timer counter
//-------------------------------------------------

void mos6530_device_base::timer_start(uint8_t data)
{
	m_timerstate = TIMER_COUNTING;
	attotime curtime = machine().time();
	int64_t target = curtime.as_ticks(clock()) + 1 + (data << m_timershift);
	m_timer->adjust(attotime::from_ticks(target, clock()) - curtime);
}


//-------------------------------------------------
//  timer_end -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mos6530_device_base::timer_end)
{
	// if we finished counting, signal timer IRQ
	if (m_timerstate == TIMER_COUNTING)
	{
		m_timeout = machine().time();
		m_irq_timer = true;
		update_irq();
	}

	// if we finished, keep spinning without the prescaler
	m_timerstate = TIMER_SPINNING;
	m_timer->adjust(attotime::from_ticks(256, clock()));
}


//-------------------------------------------------
//  edge_detect -
//-------------------------------------------------

void mos6530_device_base::edge_detect()
{
	uint8_t data = (m_pa_out & m_pa_ddr) | (m_pa_in & ~m_pa_ddr);
	int state = BIT(data, 7);

	if ((m_pa7 ^ state) && !(m_pa7_dir ^ state) && !m_irq_edge)
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

void mos6530_device_base::pa_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOG("%s %s %s Port A Data Write %02X Mask %02X\n", machine().time().as_string(), machine().describe_context(), name(), data, mem_mask);

	m_pa_in = (m_pa_in & ~mem_mask) | (data & mem_mask);
	edge_detect();
}


//-------------------------------------------------
//  pb_w -
//-------------------------------------------------

void mos6530_device_base::pb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOG("%s %s %s Port B Data Write %02X Mask %02X\n", machine().time().as_string(), machine().describe_context(), name(), data, mem_mask);

	m_pb_in = (m_pb_in & ~mem_mask) | (data & mem_mask);
}


//-------------------------------------------------
//  pa_data_r -
//-------------------------------------------------

uint8_t mos6530_device_base::pa_data_r()
{
	uint8_t in = 0;

	if (m_in8_pa_cb.isunset())
	{
		for (int i = 0; i < 8; i++)
			in |= (m_in_pa_cb[i].isunset() ? BIT(m_pa_in, i) : m_in_pa_cb[i]()) << i;
	}
	else
		in = m_in8_pa_cb();

	uint8_t data = (m_pa_out & m_pa_ddr) | (in & ~m_pa_ddr);

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

	if (m_in8_pb_cb.isunset())
	{
		for (int i = 0; i < 8; i++)
			in |= (m_in_pb_cb[i].isunset() ? BIT(m_pb_in, i) : m_in_pb_cb[i]()) << i;
	}
	else
		in = m_in8_pb_cb();

	uint8_t data = (m_pb_out & m_pb_ddr) | (in & ~m_pb_ddr);

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
	return timer_r(false);
}

uint8_t mos6530_device_base::timer_on_r()
{
	return timer_r(true);
}

uint8_t mos6530_device_base::timer_r(bool ie)
{
	uint8_t data = get_timer();

	if (!machine().side_effects_disabled())
	{
		// IRQ is not cleared when reading at the same time IRQ is raised
		if (m_timeout < machine().time() - attotime::from_hz(2 * clock()))
		{
			m_irq_timer = false;

			// timer goes back to count mode
			if (m_timerstate == TIMER_SPINNING)
				timer_start(data);
		}

		m_ie_timer = ie;
		update_irq();

		LOGTIMER("%s %s %s Timer read %02x IE %u\n", machine().time().as_string(), machine().describe_context(), name(), data, m_ie_timer ? 1 : 0);
	}

	return data;
}


//-------------------------------------------------
//  irq_r -
//-------------------------------------------------

uint8_t mos6530_device_base::irq_r()
{
	uint8_t data = get_irq_flags();

	if (!machine().side_effects_disabled() && m_irq_edge)
	{
		m_irq_edge = false;
		update_irq();
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
	// A0-A1 contain the prescaler
	static const uint8_t timershift[4] = { 0, 3, 6, 10 };
	m_timershift = timershift[offset & 3];
	timer_start(data);

	m_irq_timer = false;
	m_ie_timer = ie;
	update_irq();

	LOGTIMER("%s %s %s Timer value %02x prescale %u IE %u\n", machine().time().as_string(), machine().describe_context(), name(), data, 1 << m_timershift, m_ie_timer ? 1 : 0);
}


//-------------------------------------------------
//  edge_w -
//-------------------------------------------------

void mos6530_device_base::edge_w(offs_t offset, uint8_t data)
{
	m_pa7_dir = BIT(offset, 0);
	m_ie_edge = bool(BIT(offset, 1));
	update_irq();

	LOG("%s %s %s %s edge-detect, %s interrupt\n", machine().time().as_string(), machine().describe_context(), name(), m_pa7_dir ? "positive" : "negative", m_ie_edge ? "enable" : "disable");
}
