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
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class tmpz84c015_device : public z80_device
{
public:
	tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);

	// configuration helpers

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
	DECLARE_WRITE_LINE_MEMBER( rxa_w ) { m_sio->rxa_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxb_w ) { m_sio->rxb_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ctsa_w ) { m_sio->ctsa_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ctsb_w ) { m_sio->ctsb_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dcda_w ) { m_sio->dcda_w(state); }
	DECLARE_WRITE_LINE_MEMBER( dcdb_w ) { m_sio->dcdb_w(state); }
	DECLARE_WRITE_LINE_MEMBER( ria_w ) { m_sio->ria_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rib_w ) { m_sio->rib_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxca_w ) { m_sio->rxca_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rxcb_w ) { m_sio->rxcb_w(state); }
	DECLARE_WRITE_LINE_MEMBER( txca_w ) { m_sio->txca_w(state); }
	DECLARE_WRITE_LINE_MEMBER( txcb_w ) { m_sio->txcb_w(state); }
	DECLARE_WRITE_LINE_MEMBER( synca_w ) { m_sio->synca_w(state); }
	DECLARE_WRITE_LINE_MEMBER( syncb_w ) { m_sio->syncb_w(state); }

	// CTC public interface
	DECLARE_WRITE_LINE_MEMBER( trg0 ) { m_ctc->trg0(state); }
	DECLARE_WRITE_LINE_MEMBER( trg1 ) { m_ctc->trg1(state); }
	DECLARE_WRITE_LINE_MEMBER( trg2 ) { m_ctc->trg2(state); }
	DECLARE_WRITE_LINE_MEMBER( trg3 ) { m_ctc->trg3(state); }

	// PIO public interface
	DECLARE_READ_LINE_MEMBER( rdy_a ) { return m_pio->rdy_a(); }
	DECLARE_READ_LINE_MEMBER( rdy_b ) { return m_pio->rdy_b(); }
	DECLARE_WRITE_LINE_MEMBER( strobe_a ) { m_pio->strobe_a(state); }
	DECLARE_WRITE_LINE_MEMBER( strobe_b ) { m_pio->strobe_b(state); }

	DECLARE_WRITE8_MEMBER( pa_w ) { m_pio->port_a_write(data); }
	DECLARE_READ8_MEMBER( pa_r ) { return m_pio->port_a_read(); }
	DECLARE_WRITE8_MEMBER( pb_w ) { m_pio->port_b_write(data); }
	DECLARE_READ8_MEMBER( pb_r ) { return m_pio->port_b_read(); }
	DECLARE_WRITE_LINE_MEMBER( pa0_w ) { m_pio->pa0_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pa1_w ) { m_pio->pa1_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pa2_w ) { m_pio->pa2_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pa3_w ) { m_pio->pa3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pa4_w ) { m_pio->pa4_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pa5_w ) { m_pio->pa5_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pa6_w ) { m_pio->pa6_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pa7_w ) { m_pio->pa7_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb0_w ) { m_pio->pb0_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb1_w ) { m_pio->pb1_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb2_w ) { m_pio->pb2_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb3_w ) { m_pio->pb3_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb4_w ) { m_pio->pb4_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb5_w ) { m_pio->pb5_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb6_w ) { m_pio->pb6_w(state); }
	DECLARE_WRITE_LINE_MEMBER( pb7_w ) { m_pio->pb7_w(state); }

	/////////////////////////////////////////////////////////

	DECLARE_WRITE8_MEMBER( irq_priority_w );

	void tmpz84c015_internal_io_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	const address_space_config m_io_space_config;

	virtual space_config_vector memory_space_config() const override;

private:
	// devices/pointers
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_sio;
	required_device<z80pio_device> m_pio;

	// internal state
	uint8_t m_irq_priority;

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

	devcb_write_line m_zc_cb[4];

	devcb_read8 m_in_pa_cb;
	devcb_write8 m_out_pa_cb;
	devcb_write_line m_out_ardy_cb;

	devcb_read8 m_in_pb_cb;
	devcb_write8 m_out_pb_cb;
	devcb_write_line m_out_brdy_cb;

	DECLARE_WRITE_LINE_MEMBER( out_txda_cb_trampoline_w ) { m_out_txda_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_dtra_cb_trampoline_w ) { m_out_dtra_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_rtsa_cb_trampoline_w ) { m_out_rtsa_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_wrdya_cb_trampoline_w ) { m_out_wrdya_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_synca_cb_trampoline_w ) { m_out_synca_cb(state); }

	DECLARE_WRITE_LINE_MEMBER( out_txdb_cb_trampoline_w ) { m_out_txdb_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_dtrb_cb_trampoline_w ) { m_out_dtrb_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_rtsb_cb_trampoline_w ) { m_out_rtsb_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_wrdyb_cb_trampoline_w ) { m_out_wrdyb_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_syncb_cb_trampoline_w ) { m_out_syncb_cb(state); }

	DECLARE_WRITE_LINE_MEMBER( out_rxdrqa_cb_trampoline_w ) { m_out_rxdrqa_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_txdrqa_cb_trampoline_w ) { m_out_txdrqa_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_rxdrqb_cb_trampoline_w ) { m_out_rxdrqb_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( out_txdrqb_cb_trampoline_w ) { m_out_txdrqb_cb(state); }

	template<unsigned N> DECLARE_WRITE_LINE_MEMBER( zc_cb_trampoline_w ) { m_zc_cb[N](state); }

	DECLARE_READ8_MEMBER( in_pa_cb_trampoline_r ) { return m_in_pa_cb(); }
	DECLARE_WRITE8_MEMBER( out_pa_cb_trampoline_w ) { m_out_pa_cb(data); }
	DECLARE_WRITE_LINE_MEMBER( out_ardy_cb_trampoline_w ) { m_out_ardy_cb(state); }

	DECLARE_READ8_MEMBER( in_pb_cb_trampoline_r ) { return m_in_pb_cb(); }
	DECLARE_WRITE8_MEMBER( out_pb_cb_trampoline_w ) { m_out_pb_cb(data); }
	DECLARE_WRITE_LINE_MEMBER( out_brdy_cb_trampoline_w ) { m_out_brdy_cb(state); }
};


// device type definition
DECLARE_DEVICE_TYPE(TMPZ84C015, tmpz84c015_device)


#endif // MAME_CPU_Z80_TMPZ84C015_H
