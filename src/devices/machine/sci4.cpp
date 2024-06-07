// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SCI4 / XV833A00, 7-lines serial chip with 4 multiplexed on one and the other 3 separated

#include "emu.h"
#include "sci4.h"

sci4_device::sci4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SCI4, tag, owner, clock),
	m_tx(*this),
	m_irq(*this)
{
}


void sci4_device::map(address_map &map)
{
	map(0x00, 0x3f).rw(FUNC(sci4_device::default_r), FUNC(sci4_device::default_w));

	map(0x00, 0x00).rw(FUNC(sci4_device::data_r), FUNC(sci4_device::data_w)).select(0x18);
	map(0x01, 0x01).rw(FUNC(sci4_device::enable_r), FUNC(sci4_device::enable_w)).select(0x18);
	map(0x02, 0x02).r (FUNC(sci4_device::status_r)).select(0x18);
	map(0x03, 0x03).rw(FUNC(sci4_device::datamode_r), FUNC(sci4_device::datamode_w)).select(0x18);
	map(0x05, 0x05).r (FUNC(sci4_device::reset_r)).select(0x18);

	map(0x20, 0x20).w (FUNC(sci4_device::target_w));
}


void sci4_device::device_start()
{
	save_item(NAME(m_rx));
	save_item(NAME(m_enable));
	save_item(NAME(m_status));
	save_item(NAME(m_datamode));
	save_item(NAME(m_div));
	save_item(NAME(m_cur_rx));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_tdr_full));
	save_item(NAME(m_tx_step));
	save_item(NAME(m_tx_active));
	save_item(NAME(m_rdr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_rdr_full));
	save_item(NAME(m_rx_step));
	save_item(NAME(m_rx_active));

	save_item(NAME(m_targets));

	for(u32 i=0; i != 4; i++) {
		m_tx_timer[i] = timer_alloc(FUNC(sci4_device::tx_tick), this);
		m_rx_timer[i] = timer_alloc(FUNC(sci4_device::rx_tick), this);
	}
}

void sci4_device::device_reset()
{
	std::fill(m_rx.begin(), m_rx.end(), 1);
	std::fill(m_enable.begin(), m_enable.end(), 0);
	std::fill(m_status.begin(), m_status.end(), 0);
	std::fill(m_datamode.begin(), m_datamode.end(), 0);
	std::fill(m_div.begin(), m_div.end(), 0);
	std::fill(m_cur_rx.begin(), m_cur_rx.end(), 1);
	std::fill(m_tdr.begin(), m_tdr.end(), 0);
	std::fill(m_tsr.begin(), m_tsr.end(), 0);
	std::fill(m_tdr_full.begin(), m_tdr_full.end(), 0);
	std::fill(m_tx_step.begin(), m_tx_step.end(), 0);
	std::fill(m_tx_active.begin(), m_tx_active.end(), 0);
	std::fill(m_rdr.begin(), m_rdr.end(), 0);
	std::fill(m_rsr.begin(), m_rsr.end(), 0);
	std::fill(m_rdr_full.begin(), m_rdr_full.end(), 0);
	std::fill(m_rx_step.begin(), m_rx_step.end(), 0);
	std::fill(m_rx_active.begin(), m_rx_active.end(), 0);

	m_targets = 0;
}

void sci4_device::do_rx_w(int sci, int state)
{
	if(sci >= 30)
		sci = sci - 30 + 3;

	m_rx[sci] = state;
	if(sci < 3) {
		if(state != m_cur_rx[sci]) {
			m_cur_rx[sci] = state;
			rx_changed(sci);
		}
	}
	u8 rx = ((((m_rx[6] << 3) | (m_rx[5] << 2) | (m_rx[4] << 1) | m_rx[3]) | ~(m_targets >> 4)) & 0xf) == 0xf;
	if(rx != m_cur_rx[3]) {
		m_cur_rx[3] = rx;
		rx_changed(3);
	}
}

void sci4_device::default_w(offs_t offset, u8 data)
{
	logerror("reg_w %02x, %02x (%s)\n", offset, data, machine().describe_context());
}

u8 sci4_device::default_r(offs_t offset)
{
	logerror("reg_r %02x (%s)\n", offset, machine().describe_context());
	return 0;
}

void sci4_device::datamode_w(offs_t slot, u8 data)
{
	m_datamode[slot >> 3] = data;
}

u8 sci4_device::datamode_r(offs_t slot)
{
	return m_datamode[slot >> 3];
}

void sci4_device::data_w(offs_t slot, u8 data)
{
	slot >>= 3;
	if(m_datamode[slot] == 0x80) {
		m_div[slot] = data;
		if(data)
			logerror("channel %d baud rate %dHz\n", slot, clock()/16/data);
		else
			logerror("channel %d off\n");
	} else if(m_datamode[slot] & 2)
		fifo_w(slot, data);
}

u8 sci4_device::data_r(offs_t slot)
{
	slot >>= 3;
	if(m_datamode[slot] == 0x80)
		return m_div[slot];
	else if(m_datamode[slot] & 1)
		return fifo_r(slot);
	else
		return 0;
}

void sci4_device::enable_w(offs_t slot, u8 data)
{
	slot >>= 3;
	u8 old = m_enable[slot];
	m_enable[slot] = data;
	if((data & 2) && !(old & 2))
		tx_enabled(slot);

	else if(!(data & 2) && (old & 2)) {
		if(m_status[slot] == 2) {
			m_status[slot] = 0;
			m_irq[slot](0);
		}
	}
}

u8 sci4_device::enable_r(offs_t slot)
{
	return m_enable[slot >> 3];
}

u8 sci4_device::status_r(offs_t slot)
{
	return m_status[slot >> 3];
}

u8 sci4_device::reset_r(offs_t slot)
{
	slot >>= 3;
	m_status[slot] = 0;
	m_tx_timer[slot]->adjust(attotime::never);
	m_tx_active[slot] = 0;
	tx_set(slot, 1);
	m_tdr_full[slot] = 0;
	m_irq[slot](0);
	return 0;
}


void sci4_device::target_w(u8 data)
{
	if(data == 0x11 && m_targets == 0x07)
		machine().debug_break();
	m_targets = data;
	u8 rx = ((((m_rx[6] << 3) | (m_rx[5] << 2) | (m_rx[4] << 1) | m_rx[3]) | ~(m_targets >> 4)) & 0xf) == 0xf;
	if(rx != m_cur_rx[3]) {
		m_cur_rx[3] = rx;
		rx_changed(3);
	}
	for(u32 i=0; i != 4; i++)
		if(!(m_targets & (1<<i)))
			m_tx[i+3](1);
}

void sci4_device::tx_set(int chan, int state)
{
	if(chan < 3)
		m_tx[chan](state);
	else
		for(u32 i=0; i != 4; i++)
			if((m_targets & (1<<i)))
				m_tx[i+3](state);

}

void sci4_device::fifo_w(int chan, u8 data)
{
	if(m_tdr_full[chan] && (m_enable[chan] & 4)) {
		m_status[chan] = 6;
		m_irq[chan](1);

	} else {
		m_tdr[chan] = data;
		m_tdr_full[chan] = 1;
		if(m_status[chan] == 2) {
			m_status[chan] = 0;
			m_irq[chan](0);
		}
		if(!m_tx_active[chan] && (m_enable[chan] & 2))
			tx_start(chan);
	}
}

u8 sci4_device::fifo_r(int chan)
{
	m_rdr_full[chan] = 0;
	if(m_status[chan] == 4) {
		m_status[chan] = 0;
		m_irq[chan](0);
	}
	return m_rdr[chan];
}

void sci4_device::rx_changed(int chan)
{
	if(!m_rx_active[chan] && !m_cur_rx[chan] && (m_enable[chan] & 1)) {
		m_rx_active[chan] = 1;
		m_rx_step[chan] = 0;
		m_rsr[chan] = 0;
		wait(1, 0, chan);
	} else if(m_rx_active[chan]) {
		if(m_rx_step[chan] == 0) {
			// Start bit gone before half-time
			m_rx_active[chan] = 0;
			m_rx_timer[chan]->adjust(attotime::never);
		} else
			// Force a precise resync
			wait(1, 0, chan);
	}
}

void sci4_device::tx_enabled(int chan)
{
	if(m_tdr_full[chan])
		tx_start(chan);
	else {
		m_status[chan] |= 2;
		m_irq[chan](1);
	}
}

std::string sci4_device::chan_id(u8 chan, u8 target)
{
	return chan < 3 ? util::string_format("%d", chan) : util::string_format("3:%s%s%s%s",
																			target & 1 ? "0" : "",
																			target & 2 ? "1" : "",
																			target & 4 ? "2" : "",
																			target & 8 ? "3" : "");
}

void sci4_device::tx_start(int chan)
{
	m_tx_active[chan] = 1;
	m_tsr[chan] = m_tdr[chan];
	m_tdr_full[chan] = 0;
	m_status[chan] |= 2;
	m_irq[chan](1);

	logerror("chan %s transmit %02x\n",
			 chan_id(chan, m_targets),
			 m_tsr[chan]);

	tx_set(chan, 0);
	m_tx_step[chan] = 0;
	wait(0, 1, chan);
}

void sci4_device::wait(int timer, int full, int chan)
{
	u32 div = m_div[chan] ? m_div[chan] : 0x100;
	u32 cycles = div*(full ? 16 : 8);
	(timer ? m_rx_timer : m_tx_timer)[chan]->adjust(attotime::from_ticks(cycles, clock()), chan);
}

TIMER_CALLBACK_MEMBER(sci4_device::tx_tick)
{
	u32 step = m_tx_step[param]++;
	if(step < 9) {
		tx_set(param, step == 8 ? 1 : (m_tsr[param] >> step) & 1);
		wait(0, 1, param);

	} else {
		if((m_enable[param] & 2) && m_tdr_full[param])
			tx_start(param);
		else
			m_tx_active[param] = 0;
	}
}

TIMER_CALLBACK_MEMBER(sci4_device::rx_tick)
{
	u32 step = m_rx_step[param]++;
	if(step == 0)
		wait(1, 1, param); // Value already checked in rx_changed
	else if(step < 9) {
		if(m_cur_rx[param])
			m_rsr[param] |= 1 << (step - 1);
		wait(1, 1, param);

	} else {
		if(!m_rx[param])
			logerror("chan %s framing error/break\n", chan_id(param, m_targets >> 4));
		else {
			logerror("chan %s recieved %02x\n", chan_id(param, m_targets >> 4), m_rsr[param]);
			m_rx_active[param] = 0;
			m_rdr[param] = m_rsr[param];
			if(m_rdr_full[param] && (m_enable[param] & 4))
				m_status[param] = 6;
			else
				m_status[param] = 4;
			m_irq[param](1);
		}
	}
}

DEFINE_DEVICE_TYPE(SCI4, sci4_device, "sci4", "Yamaha SCI4 quad-serial gate array")
