// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**************************************************************************************************

    Acorn Serial ULA (SERPROC)

              ____   ____
       D0  1 |*   \_/    | 28 VI
       D1  2 |           | 27 CAS OUT
       D2  3 |           | 26 TXC
       D3  4 |           | 25 CLK
       D4  5 |           | 24 RTSI
       D5  6 |           | 23 RTSO
       D6  7 |  SERPROC  | 22 TXD
       D7  8 |           | 21 DOUT
       CS  9 |           | 20 CTSI
        E 10 |           | 19 CTSO
    CASMO 11 |           | 18 RXC
   CAS IN 12 |           | 17 RXD
      DCD 13 |           | 16 DIN
      GND 14 |___________| 15 CR

**************************************************************************************************/

#ifndef MAME_ACORN_ACORN_SERPROC_H
#define MAME_ACORN_ACORN_SERPROC_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> acorn_serproc_device

class acorn_serproc_device : public device_t
{
public:
	acorn_serproc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	typedef device_delegate<double ()> casin_delegate;
	typedef device_delegate<void (double)> casout_delegate;

	template <typename... T> void casin_handler(T &&... args) { m_casin_cb.set(std::forward<T>(args)...); }
	template <typename... T> void casout_handler(T &&... args) { m_casout_cb.set(std::forward<T>(args)...); }

	auto casmo_handler() { return m_casmo_cb.bind(); }
	auto dout_handler() { return m_dout_cb.bind(); }
	auto rtso_handler() { return m_rtso_cb.bind(); }
	auto ctso_handler() { return m_ctso_cb.bind(); }
	auto txc_handler() { return m_txc_cb.bind(); }
	auto rxc_handler() { return m_rxc_cb.bind(); }
	auto rxd_handler() { return m_rxd_cb.bind(); }
	auto dcd_handler() { return m_dcd_cb.bind(); }

	void write_txd(int state);
	void write_ctsi(int state);
	void write_din(int state);
	void write_rtsi(int state);

	void write(uint8_t data);
	uint8_t read();

protected:
	virtual void device_start() override ATTR_COLD;

private:
	casin_delegate   m_casin_cb;
	casout_delegate  m_casout_cb;
	devcb_write_line m_casmo_cb;
	devcb_write_line m_dout_cb;
	devcb_write_line m_rtso_cb;
	devcb_write_line m_ctso_cb;
	devcb_write_line m_txc_cb;
	devcb_write_line m_rxc_cb;
	devcb_write_line m_rxd_cb;
	devcb_write_line m_dcd_cb;

	void update_rxd();
	void update_dcd();
	void update_ctso();

	void receive_clock(int new_clock);

	emu_timer *m_tape_timer = nullptr;
	emu_timer *m_rxc_timer = nullptr;
	emu_timer *m_txc_timer = nullptr;

	TIMER_CALLBACK_MEMBER(tape_timer);
	TIMER_CALLBACK_MEMBER(rxc_timer);
	TIMER_CALLBACK_MEMBER(txc_timer);

	double m_last_dev_val = 0;
	int m_wav_len = 0;
	int m_len0 = 0;
	int m_len1 = 0;
	int m_len2 = 0;
	int m_len3 = 0;
	uint8_t m_control = 0;
	int m_rxd_serial = 0;
	int m_cts_serial = 0;
	int m_dcd_cass = 0;
	int m_rxd_cass = 0;
	int m_casout_enabled = 0;
	int m_txd = 0;
	uint32_t m_nr_high_tones = 0;
	int m_casout_samples_to_go = 0;
	int m_casout_bit = 0;
	int m_casout_phase = 0;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_SERPROC, acorn_serproc_device)

#endif // MAME_ACORN_ACORN_SERPROC_H
