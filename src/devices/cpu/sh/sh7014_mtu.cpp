// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Multifunction Timer Pulse Unit

  TODO list (not comprehensive):
    - Synchronized operation
    - Cascade connection operation
    - External clocks
    - Timer modes (PWM mode, phase counting mode (+ decrementing counter mode), etc)

***************************************************************************/

#include "emu.h"

#include "sh7014_mtu.h"

// #define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(SH7014_MTU, sh7014_mtu_device, "sh7014mtu", "SH7014 Multifunction Timer Pulse Unit")
DEFINE_DEVICE_TYPE(SH7014_MTU_CHANNEL, sh7014_mtu_channel_device, "sh7014mtuchan", "SH7014 Multifunction Timer Pulse Unit Channel")

sh7014_mtu_device::sh7014_mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_MTU, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_chan(*this, "ch%u", 0u)
{
}

void sh7014_mtu_device::device_start()
{
	save_item(NAME(m_tsyr));
}

void sh7014_mtu_device::device_reset()
{
	m_tsyr = 0;
}

void sh7014_mtu_device::device_add_mconfig(machine_config &config)
{
	SH7014_MTU_CHANNEL(config, m_chan[0], DERIVED_CLOCK(1, 1), m_intc,
		0, // channel
		sh7014_intc_device::INT_VECTOR_MTU_TGI0A,
		sh7014_intc_device::INT_VECTOR_MTU_TGI0B,
		sh7014_intc_device::INT_VECTOR_MTU_TGI0C,
		sh7014_intc_device::INT_VECTOR_MTU_TGI0D,
		sh7014_intc_device::INT_VECTOR_MTU_TGI0V,
		-1
	);

	SH7014_MTU_CHANNEL(config, m_chan[1], DERIVED_CLOCK(1, 1), m_intc,
		1, // channel
		sh7014_intc_device::INT_VECTOR_MTU_TGI1A,
		sh7014_intc_device::INT_VECTOR_MTU_TGI1B,
		-1,
		-1,
		sh7014_intc_device::INT_VECTOR_MTU_TGI1V,
		sh7014_intc_device::INT_VECTOR_MTU_TGI1U
	);

	SH7014_MTU_CHANNEL(config, m_chan[2], DERIVED_CLOCK(1, 1), m_intc,
		2, // channel
		sh7014_intc_device::INT_VECTOR_MTU_TGI2A,
		sh7014_intc_device::INT_VECTOR_MTU_TGI2B,
		-1,
		-1,
		sh7014_intc_device::INT_VECTOR_MTU_TGI2V,
		sh7014_intc_device::INT_VECTOR_MTU_TGI2U
	);
}

void sh7014_mtu_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(sh7014_mtu_device::tstr_r), FUNC(sh7014_mtu_device::tstr_w));
	map(0x01, 0x01).rw(FUNC(sh7014_mtu_device::tsyr_r), FUNC(sh7014_mtu_device::tsyr_w));

	map(0x20, 0x3f).m(m_chan[0], FUNC(sh7014_mtu_channel_device::map_chan0));
	map(0x40, 0x5f).m(m_chan[1], FUNC(sh7014_mtu_channel_device::map_chan1_2));
	map(0x60, 0x7f).m(m_chan[2], FUNC(sh7014_mtu_channel_device::map_chan1_2));
}

///

uint8_t sh7014_mtu_device::tstr_r()
{
	return m_chan[0]->is_enabled()
	 | (m_chan[1]->is_enabled() << 1)
	 | (m_chan[2]->is_enabled() << 2);
}

void sh7014_mtu_device::tstr_w(uint8_t data)
{
	LOG("%s: tstr_w %02x\n", machine().describe_context().c_str(), data);

	m_chan[0]->set_enable((data & TSTR_CST0) != 0);
	m_chan[1]->set_enable((data & TSTR_CST1) != 0);
	m_chan[2]->set_enable((data & TSTR_CST2) != 0);
}

uint8_t sh7014_mtu_device::tsyr_r()
{
	return m_tsyr & (TSYR_SYNC0 | TSYR_SYNC1 | TSYR_SYNC2);
}

void sh7014_mtu_device::tsyr_w(uint8_t data)
{
	LOG("%s: tsyr_w %02x\n", machine().describe_context().c_str(), data);

	m_tsyr = data;
}


//////////////////

sh7014_mtu_channel_device::sh7014_mtu_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_MTU_CHANNEL, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_timer(nullptr)
{
}

void sh7014_mtu_channel_device::device_start()
{
	save_item(NAME(m_tcr));
	save_item(NAME(m_tmdr));
	save_item(NAME(m_tiorh));
	save_item(NAME(m_tiorl));
	save_item(NAME(m_tier));
	save_item(NAME(m_tsr));
	save_item(NAME(m_tcnt));
	save_item(NAME(m_tgr));

	save_item(NAME(m_last_clock_update));
	save_item(NAME(m_clock_type));
	save_item(NAME(m_clock_divider));
	save_item(NAME(m_channel_active));
	save_item(NAME(m_phase));
	save_item(NAME(m_counter_cycle));
	save_item(NAME(m_tgr_clearing));

	m_timer = timer_alloc(FUNC(sh7014_mtu_channel_device::timer_callback), this);
}

void sh7014_mtu_channel_device::device_reset()
{
	m_tcr = 0;
	m_tmdr = 0;
	m_tier = 0;
	m_tsr = TSR_TCFD;
	m_tiorh = m_tiorl = 0;
	std::fill(std::begin(m_tgr), std::end(m_tgr), 0);

	m_tcnt = 0;
	m_last_clock_update = 0;
	m_clock_type = INPUT_INTERNAL;
	m_clock_divider = DIV_1;
	m_channel_active = false;
	m_phase = 0;
	m_counter_cycle = 0x10000;
	m_tgr_clearing = TGR_CLEAR_NONE;

	m_timer->adjust(attotime::never);
}

void sh7014_mtu_channel_device::map_chan0(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(sh7014_mtu_channel_device::tcr_r), FUNC(sh7014_mtu_channel_device::tcr_w));
	map(0x01, 0x01).rw(FUNC(sh7014_mtu_channel_device::tmdr_r), FUNC(sh7014_mtu_channel_device::tmdr_w));
	map(0x02, 0x02).rw(FUNC(sh7014_mtu_channel_device::tiorh_r), FUNC(sh7014_mtu_channel_device::tiorh_w));
	map(0x03, 0x03).rw(FUNC(sh7014_mtu_channel_device::tiorl_r), FUNC(sh7014_mtu_channel_device::tiorl_w));
	map(0x04, 0x04).rw(FUNC(sh7014_mtu_channel_device::tier_r), FUNC(sh7014_mtu_channel_device::tier_w));
	map(0x05, 0x05).rw(FUNC(sh7014_mtu_channel_device::tsr_r), FUNC(sh7014_mtu_channel_device::tsr_w));
	map(0x06, 0x07).rw(FUNC(sh7014_mtu_channel_device::tcnt_r), FUNC(sh7014_mtu_channel_device::tcnt_w));
	map(0x08, 0x09).rw(FUNC(sh7014_mtu_channel_device::tgra_r), FUNC(sh7014_mtu_channel_device::tgra_w));
	map(0x0a, 0x0b).rw(FUNC(sh7014_mtu_channel_device::tgrb_r), FUNC(sh7014_mtu_channel_device::tgrb_w));
	map(0x0c, 0x0d).rw(FUNC(sh7014_mtu_channel_device::tgrc_r), FUNC(sh7014_mtu_channel_device::tgrc_w));
	map(0x0e, 0x0f).rw(FUNC(sh7014_mtu_channel_device::tgrd_r), FUNC(sh7014_mtu_channel_device::tgrd_w));
}

void sh7014_mtu_channel_device::map_chan1_2(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(sh7014_mtu_channel_device::tcr_r), FUNC(sh7014_mtu_channel_device::tcr_w));
	map(0x01, 0x01).rw(FUNC(sh7014_mtu_channel_device::tmdr_r), FUNC(sh7014_mtu_channel_device::tmdr_w));
	map(0x02, 0x02).rw(FUNC(sh7014_mtu_channel_device::tiorh_r), FUNC(sh7014_mtu_channel_device::tiorh_w));
	map(0x04, 0x04).rw(FUNC(sh7014_mtu_channel_device::tier_r), FUNC(sh7014_mtu_channel_device::tier_w));
	map(0x05, 0x05).rw(FUNC(sh7014_mtu_channel_device::tsr_r), FUNC(sh7014_mtu_channel_device::tsr_w));
	map(0x06, 0x07).rw(FUNC(sh7014_mtu_channel_device::tcnt_r), FUNC(sh7014_mtu_channel_device::tcnt_w));
	map(0x08, 0x09).rw(FUNC(sh7014_mtu_channel_device::tgra_r), FUNC(sh7014_mtu_channel_device::tgra_w));
	map(0x0a, 0x0b).rw(FUNC(sh7014_mtu_channel_device::tgrb_r), FUNC(sh7014_mtu_channel_device::tgrb_w));
}

///

TIMER_CALLBACK_MEMBER( sh7014_mtu_channel_device::timer_callback )
{
	update_counter();
	schedule_next_event();
}

void sh7014_mtu_channel_device::set_enable(bool enabled)
{
	update_counter();

	m_channel_active = enabled;

	schedule_next_event();
}

uint8_t sh7014_mtu_channel_device::tcr_r()
{
	uint8_t r = m_tcr;

	if (m_channel_id > 0)
		r &= 0x7f;

	return r;
}

void sh7014_mtu_channel_device::tcr_w(uint8_t data)
{
	LOG("%s: tcr_w<%d> %02x\n", machine().describe_context().c_str(), m_channel_id, data);

	update_counter();

	m_tcr = data;

	constexpr uint32_t divider_type[3][8] = {
		{INPUT_INTERNAL, INPUT_INTERNAL, INPUT_INTERNAL, INPUT_INTERNAL, INPUT_A, INPUT_B, INPUT_C, INPUT_D},
		{INPUT_INTERNAL, INPUT_INTERNAL, INPUT_INTERNAL, INPUT_INTERNAL, INPUT_A, INPUT_B, INPUT_INTERNAL, INPUT_TCNT2},
		{INPUT_INTERNAL, INPUT_INTERNAL, INPUT_INTERNAL, INPUT_INTERNAL, INPUT_A, INPUT_B, INPUT_C, INPUT_INTERNAL},
	};

	const int prescaler = BIT(m_tcr, 0, 3);

	m_clock_type = divider_type[m_channel_id][prescaler];

	if (m_clock_type == INPUT_INTERNAL) {
		constexpr int32_t dividers[3][8] = {
			{DIV_1, DIV_4, DIV_16, DIV_64, -1, -1, -1, -1},
			{DIV_1, DIV_4, DIV_16, DIV_64, -1, -1, DIV_256, -1},
			{DIV_1, DIV_4, DIV_16, DIV_64, -1, -1, -1, DIV_1024},
		};

		m_clock_divider = dividers[m_channel_id][prescaler];
	} else {
		m_clock_divider = DIV_1;
	}

	const int clock_edge = BIT(m_tcr, 3, 2);
	if (m_clock_divider < DIV_4) {
		m_phase = 0;
	} else {
		switch (clock_edge) {
		case 0:
			m_phase = 0;
			break;
		case 1:
			m_phase = 1 << (m_clock_divider - 1);
			break;
		case 2:
			// If count on both rising and falling edges, input clock frequency becomes 1/2
			m_phase = 0;
			m_clock_divider--;
			break;
		}
	}

	const int counting_clear = BIT(m_tcr, 5, m_channel_id == 0 ? 3 : 2);
	if (counting_clear == 3 || (m_channel_id == 0 && counting_clear == 7))
		m_tgr_clearing = TGR_CLEAR_SYNC;
	else if (counting_clear >= 1 && counting_clear <= 2)
		m_tgr_clearing = counting_clear - 1; // TGRA, TGRB
	else if (m_channel_id == 0 && counting_clear >= 5 && counting_clear <= 6)
		m_tgr_clearing = counting_clear - 3; // TGRC, TGRD (ch 0 only)
	else
		m_tgr_clearing = TGR_CLEAR_NONE; // 0, 4, and anything else

	schedule_next_event();
}

uint8_t sh7014_mtu_channel_device::tmdr_r()
{
	uint8_t r = m_tmdr;

	if (m_channel_id > 0)
		r &= 0x0f;

	return r | 0xc0;
}

void sh7014_mtu_channel_device::tmdr_w(uint8_t data)
{
	LOG("%s: tmdr_w<%d> %02x\n", machine().describe_context().c_str(), m_channel_id, data);

	m_tmdr = data;
}

uint8_t sh7014_mtu_channel_device::tiorh_r()
{
	return m_tiorh;
}

void sh7014_mtu_channel_device::tiorh_w(uint8_t data)
{
	LOG("%s: tiorh_w<%d> %02x\n", machine().describe_context().c_str(), m_channel_id, data);

	const bool trga_is_output_compare_register = BIT(data, 3) == 0;
	const bool trga_was_output_compare_register = BIT(m_tiorh, 3) == 0;
	if (trga_is_output_compare_register) {
		const auto tgra_mode = BIT(data, 2);
		const auto old_tgra_mode = BIT(m_tiorh, 2);

		if ((tgra_mode != old_tgra_mode) || (trga_is_output_compare_register != trga_was_output_compare_register)) {
			const auto initial_output = tgra_mode ? TSR_TGFA : 0;
			m_tsr = (m_tsr & ~TSR_TGFA) | initial_output;
		}
	}

	const bool trgb_is_output_compare_register = BIT(data, 7) == 0;
	const bool trgb_was_output_compare_register = BIT(m_tiorh, 7) == 0;
	if (trgb_is_output_compare_register) {
		const auto tgrb_mode = BIT(data, 6);
		const auto old_tgrb_mode = BIT(m_tiorh, 6);

		if ((tgrb_mode != old_tgrb_mode) || (trgb_is_output_compare_register != trgb_was_output_compare_register)) {
			const auto initial_output = tgrb_mode ? TSR_TGFB : 0;
			m_tsr = (m_tsr & ~TSR_TGFB) | initial_output;
		}
	}

	m_tiorh = data;
}

uint8_t sh7014_mtu_channel_device::tiorl_r()
{
	if (m_channel_id > 0)
		return 0;

	return m_tiorl;
}

void sh7014_mtu_channel_device::tiorl_w(uint8_t data)
{
	LOG("%s: tiorl_w<%d> %02x\n", machine().describe_context().c_str(), m_channel_id, data);

	if (m_channel_id > 0)
		return;

	const bool trgc_is_output_compare_register = BIT(data, 3) == 0;
	const bool trgc_was_output_compare_register = BIT(m_tiorl, 3) == 0;
	if (trgc_is_output_compare_register) {
		const auto tgrc_mode = BIT(data, 2);
		const auto old_tgrc_mode = BIT(m_tiorl, 2);

		if ((tgrc_mode != old_tgrc_mode) || (trgc_is_output_compare_register != trgc_was_output_compare_register)) {
			const auto initial_output = tgrc_mode ? TSR_TGFC : 0;
			m_tsr = (m_tsr & ~TSR_TGFC) | initial_output;
		}
	}

	const bool trgd_is_output_compare_register = BIT(data, 7) == 0;
	const bool trgd_was_output_compare_register = BIT(m_tiorl, 7) == 0;
	if (trgd_is_output_compare_register) {
		const auto tgrd_mode = BIT(data, 6);
		const auto old_tgrd_mode = BIT(m_tiorl, 6);

		if ((tgrd_mode != old_tgrd_mode) || (trgd_is_output_compare_register != trgd_was_output_compare_register)) {
			const auto initial_output = tgrd_mode ? TSR_TGFD : 0;
			m_tsr = (m_tsr & ~TSR_TGFD) | initial_output;
		}
	}

	m_tiorl = data;
}

uint8_t sh7014_mtu_channel_device::tier_r()
{
	return m_tier | 0x40;
}

void sh7014_mtu_channel_device::tier_w(uint8_t data)
{
	LOG("%s: tier_w<%d> %02x\n", machine().describe_context().c_str(), m_channel_id, data);

	m_tier = data;
}

uint8_t sh7014_mtu_channel_device::tsr_r()
{
	uint8_t r;

	if (m_channel_id == 0)
		r = (m_tsr & (TSR_TGFA | TSR_TGFB | TSR_TGFC | TSR_TGFD | TSR_TCFV)) | TSR_TCFD;
	else
		r = m_tsr & (TSR_TGFA | TSR_TGFB | TSR_TCFV | TSR_TCFU | TSR_TCFD);

	return r | (1 << 6);
}

void sh7014_mtu_channel_device::tsr_w(uint8_t data)
{
	LOG("%s: tsr_w<%d> %02x\n", machine().describe_context().c_str(), m_channel_id, data);

	if (m_channel_id == 0) {
		const uint8_t mask = TSR_TGFA | TSR_TGFB | TSR_TGFC | TSR_TGFD | TSR_TCFV;
		m_tsr = (data & ~mask) | (m_tsr & data & mask);
	} else {
		const uint8_t mask = TSR_TGFA | TSR_TGFB | TSR_TCFV | TSR_TCFU;
		m_tsr = (data & ~mask) | (m_tsr & data & mask) | (m_tsr & TSR_TCFD);
	}
}

uint16_t sh7014_mtu_channel_device::tcnt_r()
{
	update_counter();
	return m_tcnt;
}

void sh7014_mtu_channel_device::tcnt_w(uint16_t data)
{
	LOG("%s: tcnt_w<%d> %04x -> %04x\n", machine().describe_context().c_str(), m_channel_id, m_tcnt, data);

	m_tcnt = data;
}

uint16_t sh7014_mtu_channel_device::tgra_r()
{
	return m_tgr[0];
}

void sh7014_mtu_channel_device::tgra_w(uint16_t data)
{
	LOG("%s: tgra_w<%d> %04x -> %04x\n", machine().describe_context().c_str(), m_channel_id, m_tgr[0], data);

	m_tgr[0] = data;
}

uint16_t sh7014_mtu_channel_device::tgrb_r()
{
	return m_tgr[1];
}

void sh7014_mtu_channel_device::tgrb_w(uint16_t data)
{
	LOG("%s: tgrb_w<%d> %04x -> %04x\n", machine().describe_context().c_str(), m_channel_id, m_tgr[1], data);

	m_tgr[1] = data;
}

uint16_t sh7014_mtu_channel_device::tgrc_r()
{
	return m_tgr[2];
}

void sh7014_mtu_channel_device::tgrc_w(uint16_t data)
{
	LOG("%s: tgrc_w<%d> %04x -> %04x\n", machine().describe_context().c_str(), m_channel_id, m_tgr[2], data);

	m_tgr[2] = data;
}

uint16_t sh7014_mtu_channel_device::tgrd_r()
{
	return m_tgr[3];
}

void sh7014_mtu_channel_device::tgrd_w(uint16_t data)
{
	LOG("%s: tgrd_w<%d> %04x -> %04x\n", machine().describe_context().c_str(), m_channel_id, m_tgr[3], data);

	m_tgr[3] = data;
}

void sh7014_mtu_channel_device::update_counter()
{
	if (m_clock_type != INPUT_INTERNAL)
		return;

	uint64_t cur_time = machine().time().as_ticks(clock());

	if (!m_channel_active) {
		m_last_clock_update = cur_time;
		return;
	}

	if (m_last_clock_update == cur_time)
		return;

	uint64_t base_time = (m_last_clock_update + m_phase) >> m_clock_divider;
	uint64_t new_time = (cur_time + m_phase) >> m_clock_divider;
	int tt = m_tcnt + (new_time - base_time);
	m_tcnt = tt % m_counter_cycle;

	for (int i = 0; i < m_tgr_count; i++) {
		if (!BIT(m_tsr, i) && BIT(m_tier, i) && tt >= m_tgr[i]) {
			m_tsr |= 1 << i;
			m_intc->set_interrupt(m_vectors[i], ASSERT_LINE);
		}
	}

	if (!(m_tsr & TSR_TCFV) && tt >= 0x10000) {
		// Overflowed
		m_tsr |= TSR_TCFV;

		if (m_tier & TIER_TCIEV)
			m_intc->set_interrupt(m_vectors[VECTOR_TCIV], ASSERT_LINE);
	} else if (m_channel_id == 0 && !(m_tsr & TSR_TCFU) && tt < 0) {
		// Underflowed, only usable on channel 0
		m_tsr |= TSR_TCFU;

		if (m_tier & TIER_TCIEU)
			m_intc->set_interrupt(m_vectors[VECTOR_TCIU], ASSERT_LINE);
	}

	m_last_clock_update = cur_time;
}

void sh7014_mtu_channel_device::schedule_next_event()
{
	m_timer->adjust(attotime::never);

	if (!m_channel_active || m_clock_type != INPUT_INTERNAL) {
		return;
	}

	uint32_t event_delay = 0xffffffff;
	if (m_tgr_clearing >= 0 && m_tgr[m_tgr_clearing]) {
		m_counter_cycle = m_tgr[m_tgr_clearing];
	} else {
		m_counter_cycle = 0x10000;

		if (m_tier & TIER_TCIEV) {
			// Try to schedule next event for when the overflow should happen if overflow interrupt is enabled
			event_delay = m_counter_cycle - m_tcnt;

			if (event_delay == 0)
				event_delay = m_counter_cycle;
		}
	}

	// If one of the comparison register interrupts is enabled then set the next event time to be when the next interrupt should happen
	for (int i = 0; i < m_tgr_count; i++) {
		if (BIT(m_tier, i)) {
			uint32_t new_delay = 0xffffffff;

			if (m_tgr[i] > m_tcnt) {
				if (m_tcnt >= m_counter_cycle || m_tgr[i] <= m_counter_cycle)
					new_delay = m_tgr[i] - m_tcnt;
			} else if (m_tgr[i] <= m_counter_cycle) {
				if (m_tcnt < m_counter_cycle)
					new_delay = (m_counter_cycle - m_tcnt) + m_tgr[i];
				else
					new_delay = (0x10000 - m_tcnt) + m_tgr[i];
			}

			if (event_delay > new_delay)
				event_delay = new_delay;
		}
	}

	if (event_delay != 0xffffffff) {
		const uint32_t next_event = (((((1ULL << m_clock_divider) - m_phase) >> m_clock_divider) + event_delay - 1) << m_clock_divider) + m_phase;
		m_timer->adjust(attotime::from_ticks(next_event, clock()));

	}
}
