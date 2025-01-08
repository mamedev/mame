// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 SCI Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_SCI_H
#define MAME_CPU_SH_SH7014_SCI_H

#pragma once

#include "sh2.h"
#include "sh7014_intc.h"
#include "diserial.h"

class sh7014_sci_device : public device_t, public device_serial_interface
{
public:
	sh7014_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<typename T> sh7014_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&intc, int channel_id, int eri, int rxi, int txi, int tei)
		: sh7014_sci_device(mconfig, tag, owner, clock)
	{
		m_intc.set_tag(std::forward<T>(intc));
		m_channel_id = channel_id;
		m_eri_int = eri;
		m_rxi_int = rxi;
		m_txi_int = txi;
		m_tei_int = tei;
	}

	auto write_sci_tx() { return m_sci_tx_cb.bind(); }

	void set_dma_source_tx(bool val) { m_is_dma_source_tx = val; }
	void set_dma_source_rx(bool val) { m_is_dma_source_rx = val; }

	void set_external_clock_period(const attotime &_period);
	void set_send_full_data_transmit_on_sync_hack(bool enabled);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	enum : uint32_t {
		INTERNAL_ASYNC = 0,
		INTERNAL_ASYNC_OUT,
		EXTERNAL_ASYNC,
		EXTERNAL_RATE_ASYNC,
		INTERNAL_SYNC_OUT,
		EXTERNAL_SYNC,
		EXTERNAL_RATE_SYNC
	};

	enum {
		SMR_CA   = 1 << 7,
		SMR_CHR  = 1 << 6,
		SMR_PE   = 1 << 5,
		SMR_OE   = 1 << 4,
		SMR_STOP = 1 << 3,
		SMR_MP   = 1 << 2,
		SMR_CKS  = (1 << 1) | (1 << 0),

		SCR_TIE  = 1 << 7,
		SCR_RIE  = 1 << 6,
		SCR_TE   = 1 << 5,
		SCR_RE   = 1 << 4,
		SCR_MPIE = 1 << 3,
		SCR_TEIE = 1 << 2,
		SCR_CKE  = (1 << 1) | (1 << 0),
		SCR_CKE1 = 1 << 1,
		SCR_CKE0 = 1 << 0,

		SSR_TDRE = 1 << 7,
		SSR_RDRF = 1 << 6,
		SSR_ORER = 1 << 5,
		SSR_FER  = 1 << 4,
		SSR_PER  = 1 << 3,
		SSR_TEND = 1 << 2,
		SSR_MPB  = 1 << 1,
		SSR_MPBT = 1 << 0
	};

	uint8_t smr_r();
	void smr_w(uint8_t data);

	uint8_t brr_r();
	void brr_w(uint8_t data);

	uint8_t scr_r();
	void scr_w(uint8_t data);

	uint8_t tdr_r();
	void tdr_w(uint8_t data);

	uint8_t ssr_r();
	void ssr_w(uint8_t data);

	uint8_t rdr_r();

	void update_tx_state();
	void update_clock();
	void update_data_format();

	required_device<sh7014_intc_device> m_intc;

	devcb_write_line m_sci_tx_cb;

	int m_channel_id, m_eri_int, m_rxi_int, m_txi_int, m_tei_int;
	bool m_hack_set_full_data_transmit_on_sync;

	uint8_t m_smr;
	uint8_t m_brr;
	uint8_t m_scr;
	uint8_t m_tdr;
	uint8_t m_ssr;
	uint8_t m_rdr;

	attotime m_external_clock_period, m_clock_speed;
	bool m_is_dma_source_tx, m_is_dma_source_rx;
};


DECLARE_DEVICE_TYPE(SH7014_SCI, sh7014_sci_device)

#endif // MAME_CPU_SH_SH7014_SCI_H
