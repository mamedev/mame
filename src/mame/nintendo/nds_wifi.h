// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    nds_wifi.h
    Nintendo DS wireless module (MAC, baseband and RF chips)

***************************************************************************/
#ifndef MAME_NINTENDO_NDS_WIFI_H
#define MAME_NINTENDO_NDS_WIFI_H

#pragma once


class nds_wifi_device : public device_t
{
public:
	static constexpr feature_type unemulated_features()
	{
		return feature::LAN;
	}

	nds_wifi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_cb()
	{
		return m_irq_cb.bind();
	}

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr int NUM_REGS = 0x1000 / 2;
	static constexpr int RAM_WORDS = 0x2000 / 2;

	uint16_t &reg(offs_t address)
	{
		return m_regs[(address & 0xfff) >> 1];
	}

	uint16_t reg(offs_t address) const
	{
		return m_regs[(address & 0xfff) >> 1];
	}

	uint16_t reg_r(offs_t address);
	void reg_w(offs_t address, uint16_t data);
	uint16_t ram_r16(offs_t address) const;
	void ram_w16(offs_t address, uint16_t data);

	void set_irq(int bit);
	void update_irq();

	void reset_mac();
	void mode_reset_w(uint16_t data);
	void check_power_request();
	void power_down();
	void power_up(bool wakeup_irq);

	bool us_running() const;
	uint64_t us_now() const;
	void us_latch();
	void us_schedule();
	TIMER_CALLBACK_MEMBER(ms_tick);
	TIMER_CALLBACK_MEMBER(pre_beacon);
	void beacon_event(bool reload);

	uint8_t bb_read(uint8_t index) const;
	void bb_write(uint8_t index, uint8_t data);
	void rf_transfer();

	void tx_check();
	void tx_start(int slot);
	TIMER_CALLBACK_MEMBER(tx_event);

	devcb_write_line m_irq_cb;
	memory_share_creator<uint16_t> m_ram;

	emu_timer *m_ms_timer;
	emu_timer *m_pre_beacon_timer;
	emu_timer *m_tx_timer;

	uint16_t m_regs[NUM_REGS];
	uint8_t m_bb[0x100];
	uint32_t m_rf[0x40];

	uint64_t m_us_base;         // W_US_COUNT at m_us_time
	attotime m_us_time;
	uint64_t m_us_next;         // W_US_COUNT at the next millisecond tick
	attotime m_contentfree_time;
	attotime m_random_time;
	uint16_t m_random;
	bool m_irq_state;

	int8_t m_tx_slot;           // transmit in progress (-1 = none)
	uint8_t m_tx_phase;         // 0 = preamble, 1 = data
	bool m_tx_ok;
};

DECLARE_DEVICE_TYPE(NDS_WIFI, nds_wifi_device)

#endif // MAME_NINTENDO_NDS_WIFI_H
