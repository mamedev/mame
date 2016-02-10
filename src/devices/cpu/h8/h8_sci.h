// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_sci.h

    H8 Serial Communications Interface


***************************************************************************/

#ifndef __H8_SCI_H__
#define __H8_SCI_H__

#include "h8.h"
#include "h8_intc.h"

#define MCFG_H8_SCI_ADD( _tag, intc, eri, rxi, txi, tei ) \
	MCFG_DEVICE_ADD( _tag, H8_SCI, 0 ) \
	downcast<h8_sci_device *>(device)->set_info(intc, eri, rxi, txi, tei);

#define MCFG_H8_SCI_SET_EXTERNAL_CLOCK_PERIOD(_period) \
	downcast<h8_sci_device *>(device)->set_external_clock_period(_period);

#define MCFG_H8_SCI_TX_CALLBACK(_devcb) \
	devcb = &h8_sci_device::set_tx_cb(*device, DEVCB_##_devcb);

#define MCFG_H8_SCI_CLK_CALLBACK(_devcb) \
	devcb = &h8_sci_device::set_clk_cb(*device, DEVCB_##_devcb);

class h8_sci_device : public device_t {
public:
	h8_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_info(const char *intc, int eri, int rxi, int txi, int tei);
	void set_external_clock_period(const attotime &_period);

	DECLARE_WRITE8_MEMBER(smr_w);
	DECLARE_READ8_MEMBER(smr_r);
	DECLARE_WRITE8_MEMBER(brr_w);
	DECLARE_READ8_MEMBER(brr_r);
	DECLARE_WRITE8_MEMBER(scr_w);
	DECLARE_READ8_MEMBER(scr_r);
	DECLARE_WRITE8_MEMBER(tdr_w);
	DECLARE_READ8_MEMBER(tdr_r);
	DECLARE_WRITE8_MEMBER(ssr_w);
	DECLARE_READ8_MEMBER(ssr_r);
	DECLARE_READ8_MEMBER(rdr_r);
	DECLARE_WRITE8_MEMBER(scmr_w);
	DECLARE_READ8_MEMBER(scmr_r);

	DECLARE_WRITE_LINE_MEMBER(rx_w);
	DECLARE_WRITE_LINE_MEMBER(clk_w);

	template<class _Object> static devcb_base &set_tx_cb(device_t &device, _Object object) { return downcast<h8_sci_device &>(device).tx_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_clk_cb(device_t &device, _Object object) { return downcast<h8_sci_device &>(device).clk_cb.set_callback(object); }

	UINT64 internal_update(UINT64 current_time);

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
		CLKM_INTERNAL_ASYNC,
		CLKM_INTERNAL_ASYNC_OUT,
		CLKM_EXTERNAL_ASYNC,
		CLKM_EXTERNAL_RATE_ASYNC,
		CLKM_INTERNAL_SYNC_OUT,
		CLKM_EXTERNAL_SYNC,
		CLKM_EXTERNAL_RATE_SYNC
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

	required_device<h8_device> cpu;
	devcb_write_line tx_cb, clk_cb;
	h8_intc_device *intc;
	const char *intc_tag;
	attotime external_clock_period, cur_sync_time;
	double external_to_internal_ratio, internal_to_external_ratio;
	emu_timer *sync_timer;

	int eri_int, rxi_int, txi_int, tei_int;

	int tx_state, rx_state, tx_bit, rx_bit, clock_state, clock_mode, tx_parity, rx_parity, ext_clock_counter;
	bool clock_value, ext_clock_value, rx_value;

	UINT8 rdr, tdr, smr, scr, ssr, brr, rsr, tsr;
	UINT64 clock_base, divider;


	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void clock_start(int mode);
	void clock_stop(int mode);
	void clock_update();
	void tx_start();
	void tx_dropped_edge();
	void rx_start();
	void rx_done();
	void rx_raised_edge();

	bool is_sync_start() const;
	bool has_recv_error() const;
};

extern const device_type H8_SCI;

#endif
