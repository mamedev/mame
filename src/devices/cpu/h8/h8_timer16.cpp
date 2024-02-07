// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_timer16.cpp

    H8 16 bits timer

    TODO:
    - IRQs are level triggered? eg. when an interrupt enable flag gets set
      while an overflow or compare match flag is 1, will it trigger an IRQ?
      Or if it's edge triggered, will it trigger an IRQ on rising edge of
      (irq_enable & flag)?
    - H8/325 16-bit timer is shoehorned in and may have a bug lurking?
      It doesn't have TGR registers, but functionally equivalent OCR/ICR.

***************************************************************************/

#include "emu.h"
#include "h8_timer16.h"

// Verbosity level
// 0 = no messages
// 1 = everything
static constexpr int V = 0;

DEFINE_DEVICE_TYPE(H8_TIMER16,            h8_timer16_device,            "h8_timer16",            "H8 16-bit timer")
DEFINE_DEVICE_TYPE(H8_TIMER16_CHANNEL,    h8_timer16_channel_device,    "h8_timer16_channel",    "H8 16-bit timer channel")
DEFINE_DEVICE_TYPE(H8325_TIMER16_CHANNEL, h8325_timer16_channel_device, "h8325_timer16_channel", "H8/325 16-bit timer channel")
DEFINE_DEVICE_TYPE(H8H_TIMER16_CHANNEL,   h8h_timer16_channel_device,   "h8h_timer16_channel",   "H8H 16-bit timer channel")
DEFINE_DEVICE_TYPE(H8S_TIMER16_CHANNEL,   h8s_timer16_channel_device,   "h8s_timer16_channel",   "H8S 16-bit timer channel")

h8_timer16_channel_device::h8_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_timer16_channel_device(mconfig, H8_TIMER16_CHANNEL, tag, owner, clock)
{
}

h8_timer16_channel_device::h8_timer16_channel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_chained_timer(*this, finder_base::DUMMY_TAG),
	m_tier_mask(0), m_tgr_count(0), m_tbr_count(0), m_tgr_clearing(0), m_tcr(0), m_tier(0), m_ier(0), m_isr(0), m_clock_type(0),
	m_clock_divider(0), m_tcnt(0), m_last_clock_update(0), m_event_time(0), m_phase(0), m_counter_cycle(0), m_counter_incrementing(false), m_channel_active(false)
{
}

uint8_t h8_timer16_channel_device::tcr_r()
{
	return m_tcr;
}

void h8_timer16_channel_device::tcr_w(uint8_t data)
{
	update_counter();
	m_tcr = data;
	if(V>=1) logerror("tcr_w %02x\n", data);
	tcr_update();
	recalc_event();
}

uint8_t h8_timer16_channel_device::tmdr_r()
{
	return 0x00;
}

void h8_timer16_channel_device::tmdr_w(uint8_t data)
{
	if(V>=1) logerror("tmdr_w %02x\n", data);
}

uint8_t h8_timer16_channel_device::tior_r()
{
	return 0x00;
}

void h8_timer16_channel_device::tior_w(offs_t offset, uint8_t data)
{
	if(V>=1) logerror("tior_w %d, %02x\n", offset, data);
}

void h8_timer16_channel_device::set_ier(uint8_t value)
{
	update_counter();
	m_ier = value;
	recalc_event();
}

void h8_timer16_channel_device::set_enable(bool enable)
{
	update_counter();
	m_channel_active = enable;
	recalc_event();
}

uint8_t h8_timer16_channel_device::tier_r()
{
	return m_tier;
}

void h8_timer16_channel_device::tier_w(uint8_t data)
{
	update_counter();
	if(V>=1) logerror("tier_w %02x\n", data);
	m_tier = data;
	tier_update();
	if(V>=1) logerror("irq %c%c%c%c%c%c trigger=%d\n",
						m_ier & IRQ_A ? 'a' : '.',
						m_ier & IRQ_B ? 'b' : '.',
						m_ier & IRQ_C ? 'c' : '.',
						m_ier & IRQ_D ? 'd' : '.',
						m_ier & IRQ_V ? 'v' : '.',
						m_ier & IRQ_U ? 'u' : '.',
						m_ier & IRQ_TRIG ? 1 : 0);
	recalc_event();
}

uint8_t h8_timer16_channel_device::tsr_r()
{
	update_counter();
	return isr_to_sr();
}

void h8_timer16_channel_device::tsr_w(uint8_t data)
{
	update_counter();
	if(V>=1) logerror("tsr_w %02x\n", data);
	isr_update(data);
	recalc_event();
}

uint16_t h8_timer16_channel_device::tcnt_r()
{
	update_counter();
	return m_tcnt;
}

void h8_timer16_channel_device::tcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	update_counter();
	COMBINE_DATA(&m_tcnt);
	if(V>=1) logerror("tcnt_w %04x\n", m_tcnt);
	recalc_event();
}

uint16_t h8_timer16_channel_device::tgr_r(offs_t offset)
{
	return m_tgr[offset];
}

void h8_timer16_channel_device::tgr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	update_counter();
	COMBINE_DATA(m_tgr + offset);
	if(V>=1) logerror("tgr%c_w %04x\n", 'a'+offset, m_tgr[offset]);
	recalc_event();
}

uint16_t h8_timer16_channel_device::tbr_r(offs_t offset)
{
	return m_tgr[offset+m_tgr_count];
}

void h8_timer16_channel_device::tbr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_tgr + offset + m_tgr_count);
	if(V>=1) logerror("tbr%c_w %04x\n", 'a'+offset, m_tgr[offset + m_tgr_count]);
}

void h8_timer16_channel_device::device_start()
{
	m_channel_active = false;
	device_reset();

	save_item(NAME(m_tgr_clearing));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tier));
	save_item(NAME(m_ier));
	save_item(NAME(m_isr));
	save_item(NAME(m_clock_type));
	save_item(NAME(m_clock_divider));
	save_item(NAME(m_tcnt));
	save_item(NAME(m_tgr));
	save_item(NAME(m_last_clock_update));
	save_item(NAME(m_event_time));
	save_item(NAME(m_phase));
	save_item(NAME(m_counter_cycle));
	save_item(NAME(m_counter_incrementing));
	save_item(NAME(m_channel_active));
}

void h8_timer16_channel_device::device_reset()
{
	// Don't touch channel_active here, top level device handles it
	m_tcr = 0;
	m_tcnt = 0;
	memset(m_tgr, 0xff, sizeof(m_tgr));
	m_tgr_clearing = TGR_CLEAR_NONE;
	m_clock_type = DIV_1;
	m_clock_divider = 0;
	m_counter_cycle = 0x10000;
	m_phase = 0;
	m_tier = 0x40 & m_tier_mask;
	m_ier = 0;
	m_isr = 0;
	m_last_clock_update = 0;
	m_event_time = 0;
	m_counter_incrementing = true;
}

uint64_t h8_timer16_channel_device::internal_update(uint64_t current_time)
{
	if(m_event_time && current_time >= m_event_time) {
		update_counter(current_time);
		recalc_event(current_time);
	}

	return m_event_time;
}

void h8_timer16_channel_device::update_counter(uint64_t cur_time)
{
	if(m_clock_type != DIV_1)
		return;

	if(!cur_time)
		cur_time = m_cpu->total_cycles();

	if(!m_channel_active) {
		m_last_clock_update = cur_time;
		return;
	}

	uint64_t base_time = m_last_clock_update;
	uint64_t new_time = cur_time;
	if(m_clock_divider) {
		base_time = (base_time + m_phase) >> m_clock_divider;
		new_time = (new_time + m_phase) >> m_clock_divider;
	}
	if(m_counter_incrementing) {
		int tt = m_tcnt + new_time - base_time;
		m_tcnt = tt % m_counter_cycle;

		for(int i=0; i<m_tgr_count; i++)
			if(!(m_isr & (1 << i)) && (tt == m_tgr[i] || m_tcnt == m_tgr[i])) {
				m_isr |= 1 << i;
				if (m_ier & (1 << i) && m_interrupt[i] != -1)
					m_intc->internal_interrupt(m_interrupt[i]);
			}
		if(tt >= 0x10000 && !(m_isr & IRQ_V)) {
			m_isr |= IRQ_V;
			if (m_ier & IRQ_V && m_interrupt[4] != -1)
				m_intc->internal_interrupt(m_interrupt[4]);
		}
	} else
		m_tcnt = (((m_tcnt ^ 0xffff) + new_time - base_time) % m_counter_cycle) ^ 0xffff;
	m_last_clock_update = cur_time;
}

void h8_timer16_channel_device::recalc_event(uint64_t cur_time)
{
	if(!m_channel_active) {
		m_event_time = 0;
		return;
	}

	bool update_cpu = cur_time == 0;
	uint64_t old_event_time = m_event_time;

	if(m_clock_type != DIV_1) {
		m_event_time = 0;
		if(old_event_time && update_cpu)
			m_cpu->internal_update();

		return;
	}

	if(!cur_time)
		cur_time = m_cpu->total_cycles();

	if(m_counter_incrementing) {
		uint32_t event_delay = 0xffffffff;
		if(m_tgr_clearing >= 0 && m_tgr[m_tgr_clearing])
			m_counter_cycle = m_tgr[m_tgr_clearing];
		else {
			m_counter_cycle = 0x10000;
			if(m_ier & IRQ_V) {
				event_delay = m_counter_cycle - m_tcnt;
				if(!event_delay)
					event_delay = m_counter_cycle;
			}
		}
		for(int i=0; i<m_tgr_count; i++)
			if(m_ier & (1 << i)) {
				uint32_t new_delay = 0xffffffff;
				if(m_tgr[i] > m_tcnt) {
					if(m_tcnt >= m_counter_cycle || m_tgr[i] <= m_counter_cycle)
						new_delay = m_tgr[i] - m_tcnt;
				} else if(m_tgr[i] <= m_counter_cycle) {
					if(m_tcnt < m_counter_cycle)
						new_delay = (m_counter_cycle - m_tcnt) + m_tgr[i];
					else
						new_delay = (0x10000 - m_tcnt) + m_tgr[i];
				}

				if(event_delay > new_delay)
					event_delay = new_delay;
			}

		if(event_delay != 0xffffffff)
			m_event_time = ((((cur_time + (1ULL << m_clock_divider) - m_phase) >> m_clock_divider) + event_delay - 1) << m_clock_divider) + m_phase;
		else
			m_event_time = 0;

	} else {
		logerror("decrementing counter\n");
		exit(1);
	}

	if(old_event_time != m_event_time && update_cpu)
		m_cpu->internal_update();
}

h8_timer16_device::h8_timer16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H8_TIMER16, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_timer_channel(*this, "%u", 0)
{
}

void h8_timer16_device::device_start()
{
	save_item(NAME(m_tstr));
}

void h8_timer16_device::device_reset()
{
	m_tstr = m_default_tstr;
	for(int i=0; i<m_timer_count; i++)
		m_timer_channel[i]->set_enable((m_tstr >> i) & 1);
}


uint8_t h8_timer16_device::tstr_r()
{
	return m_tstr;
}

void h8_timer16_device::tstr_w(uint8_t data)
{
	if(V>=1) logerror("tstr_w %02x\n", data);
	m_tstr = data;
	for(int i=0; i<m_timer_count; i++)
		m_timer_channel[i]->set_enable((m_tstr >> i) & 1);
}

uint8_t h8_timer16_device::tsyr_r()
{
	return 0x00;
}

void h8_timer16_device::tsyr_w(uint8_t data)
{
	if(V>=1) logerror("tsyr_w %02x\n", data);
}

uint8_t h8_timer16_device::tmdr_r()
{
	return 0x00;
}

void h8_timer16_device::tmdr_w(uint8_t data)
{
	if(V>=1) logerror("tmdr_w %02x\n", data);
}

uint8_t h8_timer16_device::tfcr_r()
{
	return 0x00;
}

void h8_timer16_device::tfcr_w(uint8_t data)
{
	if(V>=1) logerror("tfcr_w %02x\n", data);
}

uint8_t h8_timer16_device::toer_r()
{
	return 0x00;
}

void h8_timer16_device::toer_w(uint8_t data)
{
	if(V>=1) logerror("toer_w %02x\n", data);
}

uint8_t h8_timer16_device::tocr_r()
{
	return 0x00;
}

void h8_timer16_device::tocr_w(uint8_t data)
{
	if(V>=1) logerror("tocr_w %02x\n", data);
}

uint8_t h8_timer16_device::tisr_r(offs_t offset)
{
	uint8_t r = 0;
	for(int i=0; i<m_timer_count; i++)
		r |= m_timer_channel[i]->tisr_r(offset) << i;
	for(int i=m_timer_count; i<4; i++)
		r |= 0x11 <<i;

	if(V>=1) logerror("tisr%c_r %02x\n", 'a'+offset, r);

	return r;
}

void h8_timer16_device::tisr_w(offs_t offset, uint8_t data)
{
	if(V>=1) logerror("tisr%c_w %02x\n", 'a'+offset, data);
	for(int i=0; i<m_timer_count; i++)
		m_timer_channel[i]->tisr_w(offset, data >> i);
}

uint8_t h8_timer16_device::tisrc_r()
{
	return tisr_r(2);
}

void h8_timer16_device::tisrc_w(uint8_t data)
{
	tisr_w(2, data);
}

void h8_timer16_device::tolr_w(uint8_t data)
{
	if(V>=1) logerror("tocr_w %02x\n", data);
}



void h8_timer16_channel_device::tier_update()
{
}

void h8_timer16_channel_device::isr_update(uint8_t val)
{
}

uint8_t h8_timer16_channel_device::isr_to_sr() const
{
	return 0x00;
}

void h8_timer16_channel_device::tcr_update()
{
}

void h8_timer16_channel_device::tisr_w(int offset, uint8_t value)
{
	update_counter();
	if(!(value & 0x01)) {
		switch(offset) {
		case 0:
			m_isr &= ~IRQ_A;
			break;
		case 1:
			m_isr &= ~IRQ_B;
			break;
		case 2:
			m_isr &= ~IRQ_V;
			break;
		}
	}
	if(value & 0x10) {
		switch(offset) {
		case 0:
			m_ier |= IRQ_A;
			break;
		case 1:
			m_ier |= IRQ_B;
			break;
		case 2:
			m_ier |= IRQ_V;
			break;
		}
	} else {
		switch(offset) {
		case 0:
			m_ier &= ~IRQ_A;
			break;
		case 1:
			m_ier &= ~IRQ_B;
			break;
		case 2:
			m_ier &= ~IRQ_V;
			break;
		}
	}
	recalc_event();
}

uint8_t h8_timer16_channel_device::tisr_r(int offset) const
{
	switch(offset) {
	case 0:
		return ((m_ier & IRQ_A) ? 0x10 : 0x00) | ((m_isr & IRQ_A) ? 0x01 : 0x00);
	case 1:
		return ((m_ier & IRQ_B) ? 0x10 : 0x00) | ((m_isr & IRQ_B) ? 0x01 : 0x00);
	case 2:
		return ((m_ier & IRQ_V) ? 0x10 : 0x00) | ((m_isr & IRQ_V) ? 0x01 : 0x00);
	}
	return 0x00;
}


h8325_timer16_channel_device::h8325_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_timer16_channel_device(mconfig, H8325_TIMER16_CHANNEL, tag, owner, clock),
	m_tcsr(0)
{
}

h8325_timer16_channel_device::~h8325_timer16_channel_device()
{
}

void h8325_timer16_channel_device::device_start()
{
	h8_timer16_channel_device::device_start();

	save_item(NAME(m_tcsr));
}

void h8325_timer16_channel_device::device_reset()
{
	h8_timer16_channel_device::device_reset();

	m_tcsr = 0;
	m_clock_divider = 1;
}

void h8325_timer16_channel_device::tcr_update()
{
	m_ier =
		(m_tcr & 0x10 ? IRQ_V : 0) |
		(m_tcr & 0x20 ? IRQ_A : 0) |
		(m_tcr & 0x40 ? IRQ_B : 0) |
		(m_tcr & 0x80 ? IRQ_C : 0);

	m_clock_type = DIV_1;
	m_clock_divider = 0;

	switch (m_tcr & 3) {
	case 0: // /2
		m_clock_divider = 1;
		break;
	case 1: // /8
		m_clock_divider = 3;
		break;
	case 2: // /32
		m_clock_divider = 5;
		break;
	case 3: // external
		m_clock_type = INPUT_A;
		break;
	}
}

void h8325_timer16_channel_device::isr_update(uint8_t val)
{
	m_tcsr = val;

	if (val & 1)
		m_tgr_clearing = 0;
	else
		m_tgr_clearing = TGR_CLEAR_NONE;

	if(!(val & 0x10))
		m_isr &= ~IRQ_V;
	if(!(val & 0x20))
		m_isr &= ~IRQ_A;
	if(!(val & 0x40))
		m_isr &= ~IRQ_B;
	if(!(val & 0x80))
		m_isr &= ~IRQ_C;
}

uint8_t h8325_timer16_channel_device::isr_to_sr() const
{
	return (m_tcsr & 0x0f) |
		(m_isr & IRQ_V ? 0x10 : 0) |
		(m_isr & IRQ_A ? 0x20 : 0) |
		(m_isr & IRQ_B ? 0x40 : 0) |
		(m_isr & IRQ_C ? 0x80 : 0);
}


h8h_timer16_channel_device::h8h_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_timer16_channel_device(mconfig, H8H_TIMER16_CHANNEL, tag, owner, clock)
{
}

h8h_timer16_channel_device::~h8h_timer16_channel_device()
{
}

void h8h_timer16_channel_device::tier_update()
{
	m_tier = m_tier | 0xf8;
	m_ier =
		(m_tier & 0x01 ? IRQ_A : 0) |
		(m_tier & 0x02 ? IRQ_B : 0) |
		(m_tier & 0x04 ? IRQ_V : 0);
}

void h8h_timer16_channel_device::isr_update(uint8_t val)
{
	if(!(val & 1))
		m_isr &= ~IRQ_A;
	if(!(val & 2))
		m_isr &= ~IRQ_B;
	if(!(val & 4))
		m_isr &= ~IRQ_V;
}

uint8_t h8h_timer16_channel_device::isr_to_sr() const
{
	return 0xf8 | (m_isr & IRQ_V ? 4 : 0) | (m_isr & IRQ_B ? 2 : 0) | (m_isr & IRQ_A ? 1 : 0);
}


void h8h_timer16_channel_device::tcr_update()
{
	switch(m_tcr & 0x60) {
	case 0x00:
		m_tgr_clearing = TGR_CLEAR_NONE;
		if(V>=1) logerror("No automatic tcnt clearing\n");
		break;
	case 0x20: case 0x40: {
		m_tgr_clearing = m_tcr & 0x20 ? 0 : 1;
		if(V>=1) logerror("Auto-clear on tgr%c (%04x)\n", 'a'+m_tgr_clearing, m_tgr[m_tgr_clearing]);
		break;
	}
	case 0x60:
		m_tgr_clearing = TGR_CLEAR_EXT;
		if(V>=1) logerror("External sync clear\n");
		break;
	}

	int count_type = m_tcr & 7;
	if(count_type < 4) {
		m_clock_type = DIV_1;
		m_clock_divider = count_type;
		if(V>=1) logerror("clock divider %d (%d)\n", m_clock_divider, 1 << m_clock_divider);
		if(count_type <= DIV_2)
			m_phase = 0;
		else {
			switch(m_tcr & 0x18) {
			case 0x00:
				m_phase = 0;
				if(V>=1) logerror("Phase 0\n");
				break;
			case 0x08:
				m_phase = 1 << (m_clock_divider-1);
				if(V>=1) logerror("Phase 180\n");
				break;
			case 0x10: case 0x18:
				m_phase = 0;
				m_clock_divider--;
				if(V>=1) logerror("Phase 0+180\n");
				break;
			}
		}
	} else {
		m_clock_type = INPUT_A + (count_type-4);
		m_clock_divider = 0;
		m_phase = 0;
		if(V>=1) logerror("counting input %c\n", 'a'+count_type-INPUT_A);
	}
}

h8s_timer16_channel_device::h8s_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_timer16_channel_device(mconfig, H8S_TIMER16_CHANNEL, tag, owner, clock)
{
}

h8s_timer16_channel_device::~h8s_timer16_channel_device()
{
}

void h8s_timer16_channel_device::tier_update()
{
	m_tier = (m_tier & ~m_tier_mask) | 0x40;
	m_ier =
		(m_tier & 0x01 ? IRQ_A : 0) |
		(m_tier & 0x02 ? IRQ_B : 0) |
		(m_tier & 0x04 ? IRQ_C : 0) |
		(m_tier & 0x08 ? IRQ_D : 0) |
		(m_tier & 0x10 ? IRQ_V : 0) |
		(m_tier & 0x20 ? IRQ_U : 0) |
		(m_tier & 0x80 ? IRQ_TRIG : 0);
}

void h8s_timer16_channel_device::isr_update(uint8_t val)
{
	m_isr &= (val | m_tier_mask | 0xc0);
}

uint8_t h8s_timer16_channel_device::isr_to_sr() const
{
	return 0xc0 | m_isr;
}

void h8s_timer16_channel_device::tcr_update()
{
	switch(m_tcr & 0x60) {
	case 0x00:
		m_tgr_clearing = TGR_CLEAR_NONE;
		if(V>=1) logerror("No automatic tcnt clearing\n");
		break;
	case 0x20: case 0x40: {
		m_tgr_clearing = m_tcr & 0x20 ? 0 : 1;
		if(m_tgr_count > 2 && (m_tcr & 0x80))
			m_tgr_clearing += 2;
		if(V>=1) logerror("Auto-clear on tgr%c\n", 'a'+m_tgr_clearing);
		break;
	}
	case 0x60:
		m_tgr_clearing = TGR_CLEAR_EXT;
		if(V>=1) logerror("External sync clear\n");
		break;
	}

	int count_type = m_count_types[m_tcr & 7];
	if(count_type >= DIV_1 && m_clock_type <= DIV_4) {
		m_clock_type = DIV_1;
		m_clock_divider = count_type - DIV_1;
		if(V>=1) logerror("clock divider %d (%d)\n", m_clock_divider, 1 << m_clock_divider);
		if(!m_clock_divider)
			m_phase = 0;
		else {
			switch(m_tcr & 0x18) {
			case 0x00:
				m_phase = 0;
				if(V>=1) logerror("Phase 0\n");
				break;
			case 0x08:
				m_phase = 1 << (m_clock_divider-1);
				if(V>=1) logerror("Phase 180\n");
				break;
			case 0x10: case 0x18:
				m_phase = 0;
				m_clock_divider--;
				if(V>=1) logerror("Phase 0+180\n");
				break;
			}
		}

	} else if(count_type == CHAIN) {
		m_clock_type = CHAIN;
		m_clock_divider = 0;
		m_phase = 0;
		if(V>=1) logerror("chained timer\n");

	} else if(count_type >= INPUT_A && count_type <= INPUT_D) {
		m_clock_type = count_type;
		m_clock_divider = 0;
		m_phase = 0;
		if(V>=1) logerror("counting input %c\n", 'a'+count_type-INPUT_A);
	}
}
