// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Toshiba TMPZ84C015, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

***************************************************************************/

#ifndef MAME_CPU_Z80_TMPZ84C015_H
#define MAME_CPU_Z80_TMPZ84C015_H

#pragma once

#include "z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class tmpz84c015_device : public z80_device
{
public:
	tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <int Channel> void set_clk_trg(u32 clock) { m_ctc.lookup()->set_clk<Channel>(clock); }
	template <int Channel> void set_clk_trg(const XTAL &xtal) { m_ctc.lookup()->set_clk<Channel>(xtal); }

	// SIO callbacks
	auto out_txda_callback() { return m_out_txda_cb.bind(); }
	auto out_dtra_callback() { return m_out_dtra_cb.bind(); }
	auto out_rtsa_callback() { return m_out_rtsa_cb.bind(); }
	auto out_wrdya_callback() { return m_out_wrdya_cb.bind(); }
	auto out_synca_callback() { return m_out_synca_cb.bind(); }

	auto out_txdb_callback() { return m_out_txdb_cb.bind(); }
	auto out_dtrb_callback() { return m_out_dtrb_cb.bind(); }
	auto out_rtsb_callback() { return m_out_rtsb_cb.bind(); }
	auto out_wrdyb_callback() { return m_out_wrdyb_cb.bind(); }
	auto out_syncb_callback() { return m_out_syncb_cb.bind(); }

	auto out_rxdrqa_callback() { return m_out_rxdrqa_cb.bind(); }
	auto out_txdrqa_callback() { return m_out_txdrqa_cb.bind(); }
	auto out_rxdrqb_callback() { return m_out_rxdrqb_cb.bind(); }
	auto out_txdrqb_callback() { return m_out_txdrqb_cb.bind(); }

	auto wdtout_cb() { return m_wdtout_cb.bind(); }

	// CTC callbacks
	template<unsigned N> auto zc_callback() { return m_zc_cb[N].bind(); }

	// PIO callbacks
	auto in_pa_callback() { return m_in_pa_cb.bind(); }
	auto out_pa_callback() { return m_out_pa_cb.bind(); }
	auto out_ardy_callback() { return m_out_ardy_cb.bind(); }

	auto in_pb_callback() { return m_in_pb_cb.bind(); }
	auto out_pb_callback() { return m_out_pb_cb.bind(); }
	auto out_brdy_callback() { return m_out_brdy_cb.bind(); }

	// SIO public interface
	void rxa_w(int state) { m_sio->rxa_w(state); }
	void rxb_w(int state) { m_sio->rxb_w(state); }
	void ctsa_w(int state) { m_sio->ctsa_w(state); }
	void ctsb_w(int state) { m_sio->ctsb_w(state); }
	void dcda_w(int state) { m_sio->dcda_w(state); }
	void dcdb_w(int state) { m_sio->dcdb_w(state); }
	void rxca_w(int state) { m_sio->rxca_w(state); }
	void rxcb_w(int state) { m_sio->rxcb_w(state); }
	void txca_w(int state) { m_sio->txca_w(state); }
	void txcb_w(int state) { m_sio->txcb_w(state); }
	void synca_w(int state) { m_sio->synca_w(state); }
	void syncb_w(int state) { m_sio->syncb_w(state); }

	// CTC public interface
	void trg0(int state) { m_ctc->trg0(state); }
	void trg1(int state) { m_ctc->trg1(state); }
	void trg2(int state) { m_ctc->trg2(state); }
	void trg3(int state) { m_ctc->trg3(state); }

	// PIO public interface
	int rdy_a() { return m_pio->rdy_a(); }
	int rdy_b() { return m_pio->rdy_b(); }
	void strobe_a(int state) { m_pio->strobe_a(state); }
	void strobe_b(int state) { m_pio->strobe_b(state); }

	void pa_w(uint8_t data) { m_pio->port_a_write(data); }
	uint8_t pa_r() { return m_pio->port_a_read(); }
	void pb_w(uint8_t data) { m_pio->port_b_write(data); }
	uint8_t pb_r() { return m_pio->port_b_read(); }
	void pa0_w(int state) { m_pio->pa0_w(state); }
	void pa1_w(int state) { m_pio->pa1_w(state); }
	void pa2_w(int state) { m_pio->pa2_w(state); }
	void pa3_w(int state) { m_pio->pa3_w(state); }
	void pa4_w(int state) { m_pio->pa4_w(state); }
	void pa5_w(int state) { m_pio->pa5_w(state); }
	void pa6_w(int state) { m_pio->pa6_w(state); }
	void pa7_w(int state) { m_pio->pa7_w(state); }
	void pb0_w(int state) { m_pio->pb0_w(state); }
	void pb1_w(int state) { m_pio->pb1_w(state); }
	void pb2_w(int state) { m_pio->pb2_w(state); }
	void pb3_w(int state) { m_pio->pb3_w(state); }
	void pb4_w(int state) { m_pio->pb4_w(state); }
	void pb5_w(int state) { m_pio->pb5_w(state); }
	void pb6_w(int state) { m_pio->pb6_w(state); }
	void pb7_w(int state) { m_pio->pb7_w(state); }

	/////////////////////////////////////////////////////////

protected:
	tmpz84c015_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor io_map);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	const address_space_config m_io_space_config;

	void internal_io_map(address_map &map) const;
	virtual space_config_vector memory_space_config() const override;

private:
	// devices/pointers
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<z80pio_device> m_pio;

	// internal state
	uint8_t m_irq_priority;
	uint8_t m_wdtmr;
	emu_timer *m_watchdog_timer;

	// callbacks
	devcb_write_line m_out_txda_cb;
	devcb_write_line m_out_dtra_cb;
	devcb_write_line m_out_rtsa_cb;
	devcb_write_line m_out_wrdya_cb;
	devcb_write_line m_out_synca_cb;

	devcb_write_line m_out_txdb_cb;
	devcb_write_line m_out_dtrb_cb;
	devcb_write_line m_out_rtsb_cb;
	devcb_write_line m_out_wrdyb_cb;
	devcb_write_line m_out_syncb_cb;

	devcb_write_line m_out_rxdrqa_cb;
	devcb_write_line m_out_txdrqa_cb;
	devcb_write_line m_out_rxdrqb_cb;
	devcb_write_line m_out_txdrqb_cb;

	devcb_write_line::array<4> m_zc_cb;

	devcb_read8 m_in_pa_cb;
	devcb_write8 m_out_pa_cb;
	devcb_write_line m_out_ardy_cb;

	devcb_read8 m_in_pb_cb;
	devcb_write8 m_out_pb_cb;
	devcb_write_line m_out_brdy_cb;

	devcb_write_line m_wdtout_cb;

	uint8_t wdtmr_r();
	void wdtmr_w(uint8_t data);
	void wdtcr_w(uint8_t data);
	void watchdog_clear();
	TIMER_CALLBACK_MEMBER(watchdog_timeout);

	void irq_priority_w(uint8_t data);

	void out_txda_cb_trampoline_w(int state) { m_out_txda_cb(state); }
	void out_dtra_cb_trampoline_w(int state) { m_out_dtra_cb(state); }
	void out_rtsa_cb_trampoline_w(int state) { m_out_rtsa_cb(state); }
	void out_wrdya_cb_trampoline_w(int state) { m_out_wrdya_cb(state); }
	void out_synca_cb_trampoline_w(int state) { m_out_synca_cb(state); }

	void out_txdb_cb_trampoline_w(int state) { m_out_txdb_cb(state); }
	void out_dtrb_cb_trampoline_w(int state) { m_out_dtrb_cb(state); }
	void out_rtsb_cb_trampoline_w(int state) { m_out_rtsb_cb(state); }
	void out_wrdyb_cb_trampoline_w(int state) { m_out_wrdyb_cb(state); }
	void out_syncb_cb_trampoline_w(int state) { m_out_syncb_cb(state); }

	void out_rxdrqa_cb_trampoline_w(int state) { m_out_rxdrqa_cb(state); }
	void out_txdrqa_cb_trampoline_w(int state) { m_out_txdrqa_cb(state); }
	void out_rxdrqb_cb_trampoline_w(int state) { m_out_rxdrqb_cb(state); }
	void out_txdrqb_cb_trampoline_w(int state) { m_out_txdrqb_cb(state); }

	template<unsigned N> void zc_cb_trampoline_w(int state) { m_zc_cb[N](state); }

	uint8_t in_pa_cb_trampoline_r() { return m_in_pa_cb(); }
	void out_pa_cb_trampoline_w(uint8_t data) { m_out_pa_cb(data); }
	void out_ardy_cb_trampoline_w(int state) { m_out_ardy_cb(state); }

	uint8_t in_pb_cb_trampoline_r() { return m_in_pb_cb(); }
	void out_pb_cb_trampoline_w(uint8_t data) { m_out_pb_cb(data); }
	void out_brdy_cb_trampoline_w(int state) { m_out_brdy_cb(state); }
};


// device type definition
DECLARE_DEVICE_TYPE(TMPZ84C015, tmpz84c015_device)


#endif // MAME_CPU_Z80_TMPZ84C015_H
