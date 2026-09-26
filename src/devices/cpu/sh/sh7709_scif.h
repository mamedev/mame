// license:BSD-3-Clause
// copyright-holders:buffi
/***************************************************************************

  SH7709 SCIF (Serial Communication Interface with FIFO)

***************************************************************************/

#ifndef MAME_CPU_SH_SH7709_SCIF_H
#define MAME_CPU_SH_SH7709_SCIF_H

#pragma once

#include "diserial.h"

class sh7709_scif_device : public device_t, public device_serial_interface
{
public:
	sh7709_scif_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Serial data out (to the peripheral's RXD).
	auto write_txd() { return m_txd_cb.bind(); }

	// Interrupt request lines.
	// The SH3 INTC has no SCIF sources wired up yet, so these
	// are left unconnected.
	auto write_eri() { return m_eri_cb.bind(); }
	auto write_rxi() { return m_rxi_cb.bind(); }
	auto write_bri() { return m_bri_cb.bind(); }
	auto write_txi() { return m_txi_cb.bind(); }

	// Serial data in (from the peripheral's TXD).
	void rxd_w(int state);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	static constexpr unsigned FIFO_LENGTH = 16;

	enum : uint8_t
	{
		SCSMR_CHR  = 1 << 6,
		SCSMR_PE   = 1 << 5,
		SCSMR_OE   = 1 << 4,
		SCSMR_STOP = 1 << 3,
		SCSMR_CKS  = 3 << 0
	};

	enum : uint8_t
	{
		SCSCR_TIE = 1 << 7,
		SCSCR_RIE = 1 << 6,
		SCSCR_TE  = 1 << 5,
		SCSCR_RE  = 1 << 4,
		SCSCR_CKE = 3 << 0
	};

	enum : uint16_t
	{
		SCSSR_DR   = 1 << 0,
		SCSSR_RDF  = 1 << 1,
		SCSSR_PER  = 1 << 2,
		SCSSR_FER  = 1 << 3,
		SCSSR_BRK  = 1 << 4,
		SCSSR_TDFE = 1 << 5,
		SCSSR_TEND = 1 << 6,
		SCSSR_ER   = 1 << 7,

		// Bits software is allowed to clear by writing 0 after reading 1.
		SCSSR_RW   = SCSSR_DR | SCSSR_RDF | SCSSR_BRK | SCSSR_TDFE | SCSSR_ER
	};

	enum : uint8_t
	{
		SCFCR_LOOP  = 1 << 0,
		SCFCR_RFRST = 1 << 1,
		SCFCR_TFRST = 1 << 2,
		SCFCR_MCE   = 1 << 3,
		SCFCR_TTRG  = 3 << 4,
		SCFCR_RTRG  = 3 << 6
	};

	uint8_t scsmr_r();
	void scsmr_w(uint8_t data);
	uint8_t scbrr_r();
	void scbrr_w(uint8_t data);
	uint8_t scscr_r();
	void scscr_w(uint8_t data);
	void scftdr_w(uint8_t data);
	uint16_t scssr_r();
	void scssr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t scfrdr_r();
	uint8_t scfcr_r();
	void scfcr_w(uint8_t data);
	uint16_t scfdr_r();

	unsigned rx_trigger() const;
	unsigned tx_trigger() const;
	void update_status();
	void update_interrupts();
	void update_tx_state();
	void update_data_format();
	void update_clock();

	devcb_write_line m_txd_cb;
	devcb_write_line m_eri_cb;
	devcb_write_line m_rxi_cb;
	devcb_write_line m_bri_cb;
	devcb_write_line m_txi_cb;

	uint8_t m_scsmr;
	uint8_t m_scbrr;
	uint8_t m_scscr;
	uint16_t m_scssr;
	uint8_t m_scfcr;

	uint8_t m_rx_fifo[FIFO_LENGTH];
	uint8_t m_tx_fifo[FIFO_LENGTH];
	uint8_t m_rx_head, m_rx_count;
	uint8_t m_tx_head, m_tx_count;

	attotime m_clock_speed;
};

DECLARE_DEVICE_TYPE(SH7709_SCIF, sh7709_scif_device)

#endif // MAME_CPU_SH_SH7709_SCIF_H
