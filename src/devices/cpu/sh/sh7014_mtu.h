// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Multifunction Timer Pulse Unit

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_MTU_H
#define MAME_CPU_SH_SH7014_MTU_H

#pragma once

#include "sh7014_intc.h"

DECLARE_DEVICE_TYPE(SH7014_MTU, sh7014_mtu_device)
DECLARE_DEVICE_TYPE(SH7014_MTU_CHANNEL, sh7014_mtu_channel_device)


class sh7014_mtu_device : public device_t
{
public:
	sh7014_mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<typename T> sh7014_mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&intc)
		: sh7014_mtu_device(mconfig, tag, owner, clock)
	{
		m_intc.set_tag(std::forward<T>(intc));
	}

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	enum {
		TSTR_CST0 = 1 << 0,
		TSTR_CST1 = 1 << 1,
		TSTR_CST2 = 1 << 2,

		TSYR_SYNC0 = 1 << 0,
		TSYR_SYNC1 = 1 << 1,
		TSYR_SYNC2 = 1 << 2,
	};

	uint8_t tstr_r();
	void tstr_w(uint8_t data);

	uint8_t tsyr_r();
	void tsyr_w(uint8_t data);

	required_device<sh7014_intc_device> m_intc;
	required_device_array<sh7014_mtu_channel_device, 3> m_chan;

	uint8_t m_tsyr;
};


class sh7014_mtu_channel_device : public device_t
{
public:
	sh7014_mtu_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<typename T> sh7014_mtu_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&intc, int chan_id, int32_t vector_tgia, int32_t vector_tgib, int32_t vector_tgic, int32_t vector_tgid, int32_t vector_tgiv, int32_t vector_tgiu)
		: sh7014_mtu_channel_device(mconfig, tag, owner, clock)
	{
		m_intc.set_tag(std::forward<T>(intc));
		m_channel_id = chan_id;
		m_vectors[VECTOR_TGIA] = vector_tgia;
		m_vectors[VECTOR_TGIB] = vector_tgib;
		m_vectors[VECTOR_TGIC] = vector_tgic;
		m_vectors[VECTOR_TGID] = vector_tgid;
		m_vectors[VECTOR_TCIV] = vector_tgiv;
		m_vectors[VECTOR_TCIU] = vector_tgiu;
		m_tgr_count = chan_id == 0 ? 4 : 2;
	}

	void map_chan0(address_map &map) ATTR_COLD;
	void map_chan1_2(address_map &map) ATTR_COLD;

	void set_enable(bool enabled);
	bool is_enabled() { return m_channel_active; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		INPUT_INTERNAL = 0,
		INPUT_A,
		INPUT_B,
		INPUT_C,
		INPUT_D,
		INPUT_TCNT2
	};

	enum {
		DIV_1 = 0,
		DIV_2,
		DIV_4,
		DIV_8,
		DIV_16,
		DIV_32,
		DIV_64,
		DIV_128,
		DIV_256,
		DIV_512,
		DIV_1024
	};

	enum {
		TGR_CLEAR_NONE = -1,
		TGR_CLEAR_SYNC = -2
	};

	enum {
		VECTOR_TGIA = 0,
		VECTOR_TGIB = 1,
		VECTOR_TGIC = 2,
		VECTOR_TGID = 3,
		VECTOR_TCIV = 4,
		VECTOR_TCIU = 5
	};

	enum {

		TSR_TGFA = 1 << 0,
		TSR_TGFB = 1 << 1,
		TSR_TGFC = 1 << 2, // ch 0 only
		TSR_TGFD = 1 << 3, // ch 0 only
		TSR_TCFV = 1 << 4,
		TSR_TCFU = 1 << 5, // ch 1/2 only
		TSR_TCFD = 1 << 7, // ch 1/2 only

		TIER_TGIEA = 1 << 0,
		TIER_TGIEB = 1 << 1,
		TIER_TGIEC = 1 << 2, // ch 0 only
		TIER_TGIED = 1 << 3, // ch 0 only
		TIER_TCIEV = 1 << 4,
		TIER_TCIEU = 1 << 5, // ch 1/2 only
		TIER_TTGE = 1 << 7,
	};

	uint8_t tcr_r();
	void tcr_w(uint8_t data);

	uint8_t tmdr_r();
	void tmdr_w(uint8_t data);

	uint8_t tiorh_r();
	void tiorh_w(uint8_t data);

	uint8_t tiorl_r();
	void tiorl_w(uint8_t data);

	uint8_t tier_r();
	void tier_w(uint8_t data);

	uint8_t tsr_r();
	void tsr_w(uint8_t data);

	uint16_t tcnt_r();
	void tcnt_w(uint16_t data);

	uint16_t tgra_r();
	void tgra_w(uint16_t data);

	uint16_t tgrb_r();
	void tgrb_w(uint16_t data);

	uint16_t tgrc_r();
	void tgrc_w(uint16_t data);

	uint16_t tgrd_r();
	void tgrd_w(uint16_t data);

	void update_counter();
	void schedule_next_event();

	TIMER_CALLBACK_MEMBER( timer_callback );

	required_device<sh7014_intc_device> m_intc;

	emu_timer *m_timer;

	uint32_t m_channel_id;
	int32_t m_vectors[6];
	uint32_t m_tgr_count;

	uint8_t m_tcr;
	uint8_t m_tmdr;
	uint8_t m_tiorh, m_tiorl;
	uint8_t m_tier;
	uint8_t m_tsr;
	uint16_t m_tgr[4];
	uint16_t m_tcnt;

	uint64_t m_last_clock_update;
	uint32_t m_clock_type, m_clock_divider;
	uint32_t m_phase;
	uint32_t m_counter_cycle;
	int32_t m_tgr_clearing;

	bool m_channel_active;
};

#endif // MAME_CPU_SH_SH7014_MTU_H
