// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_sci.h

    H8 Serial Communications Interface


***************************************************************************/

#ifndef MAME_CPU_H8_H8_SCI_H
#define MAME_CPU_H8_H8_SCI_H

#pragma once

class h8_device;
class h8_intc_device;

class h8_sci_device : public device_t {
public:
	h8_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template<typename T, typename U> h8_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, int id, T &&cpu, U &&intc, int eri, int rxi, int txi, int tei)
		: h8_sci_device(mconfig, tag, owner, 0)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
		m_id = id;
		m_eri_int = eri;
		m_rxi_int = rxi;
		m_txi_int = txi;
		m_tei_int = tei;
	}

	void do_set_external_clock_period(const attotime &_period);

	void smr_w(u8 data);
	u8 smr_r();
	void brr_w(u8 data);
	u8 brr_r();
	void scr_w(u8 data);
	u8 scr_r();
	void tdr_w(u8 data);
	u8 tdr_r();
	void ssr_w(u8 data);
	u8 ssr_r();
	u8 rdr_r();
	void scmr_w(u8 data);
	u8 scmr_r();

	void do_rx_w(int state);
	void do_clk_w(int state);

	u64 internal_update(u64 current_time);
	void notify_standby(int state);

protected:
	enum {
		ST_IDLE, ST_START, ST_BIT, ST_PARITY, ST_STOP, ST_LAST_TICK
	};

	static const char *const state_names[];

	enum {
		CLK_TX = 1,
		CLK_RX = 2
	};

	enum {
		INTERNAL_ASYNC,
		INTERNAL_ASYNC_OUT,
		EXTERNAL_ASYNC,
		EXTERNAL_RATE_ASYNC,
		INTERNAL_SYNC_OUT,
		EXTERNAL_SYNC,
		EXTERNAL_RATE_SYNC
	};

	enum {
		SMR_CA   = 0x80,
		SMR_CHR  = 0x40,
		SMR_PE   = 0x20,
		SMR_OE   = 0x10,
		SMR_STOP = 0x08,
		SMR_MP   = 0x04,
		SMR_CKS  = 0x03,

		SCR_TIE  = 0x80,
		SCR_RIE  = 0x40,
		SCR_TE   = 0x20,
		SCR_RE   = 0x10,
		SCR_MPIE = 0x08,
		SCR_TEIE = 0x04,
		SCR_CKE  = 0x03,
		SCR_CKE1 = 0x02,
		SCR_CKE0 = 0x01,

		SSR_TDRE = 0x80,
		SSR_RDRF = 0x40,
		SSR_ORER = 0x20,
		SSR_FER  = 0x10,
		SSR_PER  = 0x08,
		SSR_TEND = 0x04,
		SSR_MPB  = 0x02,
		SSR_MPBT = 0x01
	};

	required_device<h8_device> m_cpu;
	required_device<h8_intc_device> m_intc;
	attotime m_external_clock_period;
	double m_external_to_internal_ratio, m_internal_to_external_ratio;
	emu_timer *m_sync_timer;

	int m_id, m_eri_int, m_rxi_int, m_txi_int, m_tei_int;

	int m_tx_state, m_rx_state, m_tx_bit, m_rx_bit, m_clock_state, m_tx_parity, m_rx_parity, m_tx_clock_counter, m_rx_clock_counter;
	u32 m_clock_mode;
	bool m_ext_clock_value, m_rx_value;

	u8 m_rdr, m_tdr, m_smr, m_scr, m_ssr, m_brr, m_rsr, m_tsr;
	u64 m_clock_event, m_clock_step, m_divider;

	std::string m_last_clock_message;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_tick);

	void clock_start(int mode);
	void clock_stop(int mode);
	void clock_update();

	void tx_start();
	void tx_async_tick();
	void tx_async_step();
	void tx_sync_tick();
	void tx_sync_step();

	void rx_start();
	void rx_done();
	void rx_async_tick();
	void rx_async_step();
	void rx_sync_tick();
	void rx_sync_step();

	bool is_sync_start() const;
	bool has_recv_error() const;
};

DECLARE_DEVICE_TYPE(H8_SCI, h8_sci_device)

#endif // MAME_CPU_H8_H8_SCI_H
