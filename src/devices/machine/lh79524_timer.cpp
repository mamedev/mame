// license:BSD-3-Clause
// copyright-holders:Myrtle Shah
/*****************************************************************************

        LH79524 Timer

*****************************************************************************/

#include "emu.h"
#include "lh79524_timer.h"


DEFINE_DEVICE_TYPE(LH79524_TIMER, lh79524_timer_device, "lh79524_timer", "LH79524 Timer")


lh79524_timer_device::lh79524_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LH79524_TIMER, tag, owner, clock)
	, m_irq_cb(*this) {
}

void lh79524_timer_device::set_timer_index(int index) {
	timer_index = index;
}


void lh79524_timer_device::device_start() {
	m_tick_timer = timer_alloc(FUNC(lh79524_timer_device::timer_update), this);

	save_item(NAME(m_control));
	save_item(NAME(m_cap_control));
	save_item(NAME(m_inten));
	save_item(NAME(m_status));
	save_item(NAME(m_cnt));
	save_item(NAME(m_cmp0));
	save_item(NAME(m_cmp1));
}

void lh79524_timer_device::device_reset() {
	m_control = 0;
	m_cap_control = 0;
	m_inten = 0;
	m_status = 0;
	m_cnt = 0;
	m_cmp0 = 0;
	m_cmp1 = 0;

	m_tick_timer->adjust(attotime::never);

	update_interrupt();
}

void lh79524_timer_device::update_interrupt() {
	if ((m_status & 0x7) & (m_inten & 0x7)) {
		m_irq_cb(ASSERT_LINE);
	} else {
		m_irq_cb(CLEAR_LINE);
	}

}

void lh79524_timer_device::device_clock_changed() {
	if (BIT(m_control, 1)) {
		uint32_t hclk_div_value = (m_control >> 2) & 0x7;
		m_tick_timer->adjust(attotime::from_hz(clock() / (2U << hclk_div_value)), 0, attotime::from_hz(clock() / (2U << hclk_div_value)));
	} else {
		m_tick_timer->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(lh79524_timer_device::timer_update) {
	if (!BIT(m_control, 1))
		return;

	bool tc = (timer_index == 0) ?  BIT(m_cap_control, 14) : BIT(m_control, 13);

	uint16_t limit = tc ? m_cmp1 : 0xffff;

	if (m_cnt == m_cmp0) {
		m_status |= 0x0002;
	}

	if (m_cnt == m_cmp1) {
		m_status |= 0x0004;
	}

	if (m_cnt == limit) {
		m_cnt = 0;
		m_status |= 0x0001;
	} else {
		m_cnt++;
	}

	update_interrupt();
}

uint32_t lh79524_timer_device::read(offs_t offset, uint32_t mem_mask) {
	offs_t addr = offset << 2;
	if (timer_index >= 1 && addr >= 0x04) {
		addr += 0x04;
	}
	switch (addr) {
	case 0x00:
		return m_control;
	case 0x04:
		return m_cap_control;
	case 0x08:
		return m_inten;
	case 0x0c:
		return m_status;
	case 0x10:
		return m_cnt;
	case 0x14:
		return m_cmp0;
	case 0x18:
		return m_cmp1;
	default:
		logerror("[%s] %s: unknown read %x\n", tag(), machine().describe_context(), offset << 2);
	}

	return 0;
}

void lh79524_timer_device::write(offs_t offset, uint32_t data, uint32_t mem_mask) {
	offs_t addr = offset << 2;
	if (timer_index >= 1 && addr >= 0x04) {
		addr += 0x04;
	}
	switch (addr) {
	case 0x00:
		if (data & 0x1) {
			m_cnt = 0;
			data &= ~0x1;
		}
		COMBINE_DATA(&m_control);
		lh79524_timer_device::device_clock_changed();
		break;
	case 0x04:
		COMBINE_DATA(&m_cap_control);
		break;
	case 0x08:
		COMBINE_DATA(&m_inten);
		update_interrupt();
		break;
	case 0x0c:
		if (BIT(mem_mask, 0) && BIT(data, 0)) {
			m_status &= ~0x1;
		}
		if (BIT(mem_mask, 1) && BIT(data, 1)) {
			m_status &= ~0x2;
		}
		if (BIT(mem_mask, 2) && BIT(data, 2)) {
			m_status &= ~0x4;
		}
		update_interrupt();
		break;
	case 0x10:
		COMBINE_DATA(&m_cnt);
		break;
	case 0x14:
		COMBINE_DATA(&m_cmp0);
		break;
	case 0x18:
		COMBINE_DATA(&m_cmp1);
		break;
	default:
		logerror("[%s] %s: unknown write %x = %x\n", tag(), machine().describe_context(), offset << 2, data);
	}
}
