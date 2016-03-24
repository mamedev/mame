// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Toshiba TMPZ84C015, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

***************************************************************************/

#pragma once

#ifndef __TMPZ84C015__
#define __TMPZ84C015__

#include "emu.h"
#include "z80.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

// If an external daisy chain is used, insert this before your own device tags:
#define TMPZ84C015_DAISY_INTERNAL { "tmpz84c015_ctc" }, { "tmpz84c015_sio" }, { "tmpz84c015_pio" }

// SIO callbacks
#define MCFG_TMPZ84C015_OUT_TXDA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_txda_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_DTRA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_dtra_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_RTSA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_rtsa_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_WRDYA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_wrdya_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_SYNCA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_synca_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_TXDB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_txdb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_DTRB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_dtrb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_RTSB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_rtsb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_WRDYB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_wrdyb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_SYNCB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_syncb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_RXDRQA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_rxdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_TXDRQA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_txdrqa_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_RXDRQB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_rxdrqb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_TXDRQB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_txdrqb_callback(*device, DEVCB_##_devcb);


// CTC callbacks
#define MCFG_TMPZ84C015_ZC0_CB(_devcb) \
	devcb = &tmpz84c015_device::set_zc0_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_ZC1_CB(_devcb) \
	devcb = &tmpz84c015_device::set_zc1_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_ZC2_CB(_devcb) \
	devcb = &tmpz84c015_device::set_zc2_callback(*device, DEVCB_##_devcb);


// PIO callbacks
#define MCFG_TMPZ84C015_IN_PA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_in_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_PA_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_ARDY_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_ardy_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_IN_PB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_in_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_PB_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C015_OUT_BRDY_CB(_devcb) \
	devcb = &tmpz84c015_device::set_out_brdy_callback(*device, DEVCB_##_devcb);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class tmpz84c015_device : public z80_device
{
public:
	tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	// static configuration helpers
	template<class _Object> static devcb_base &set_out_txda_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_txda_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtra_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_dtra_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsa_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_rtsa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdya_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_wrdya_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_synca_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_synca_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_txdb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_txdb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtrb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_dtrb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rtsb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_rtsb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wrdyb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_wrdyb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_syncb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_syncb_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_rxdrqa_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_rxdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqa_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_txdrqa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rxdrqb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_rxdrqb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_txdrqb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_txdrqb_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_zc0_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_zc0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc1_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_zc1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc2_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_zc2_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_in_pa_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_in_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pa_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ardy_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_ardy_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_in_pb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_in_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pb_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_brdy_callback(device_t &device, _Object object) { return downcast<tmpz84c015_device &>(device).m_out_brdy_cb.set_callback(object); }

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
	DECLARE_WRITE_LINE_MEMBER( rxtxcb_w ) { m_sio->rxtxcb_w(state); }
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

	DECLARE_WRITE8_MEMBER( pa_w ) { m_pio->pa_w(space, offset, data, mem_mask); }
	DECLARE_READ8_MEMBER( pa_r ) { return m_pio->pa_r(space, offset, mem_mask); }
	DECLARE_WRITE8_MEMBER( pb_w ) { m_pio->pb_w(space, offset, data, mem_mask); }
	DECLARE_READ8_MEMBER( pb_r ) { return m_pio->pb_r(space, offset, mem_mask); }

	/////////////////////////////////////////////////////////

	DECLARE_WRITE8_MEMBER( irq_priority_w );

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

	DECLARE_WRITE_LINE_MEMBER( zc0_cb_trampoline_w ) { m_zc0_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( zc1_cb_trampoline_w ) { m_zc1_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( zc2_cb_trampoline_w ) { m_zc2_cb(state); }

	DECLARE_READ8_MEMBER( in_pa_cb_trampoline_r ) { return m_in_pa_cb(); }
	DECLARE_WRITE8_MEMBER( out_pa_cb_trampoline_w ) { m_out_pa_cb(data); }
	DECLARE_WRITE_LINE_MEMBER( out_ardy_cb_trampoline_w ) { m_out_ardy_cb(state); }

	DECLARE_READ8_MEMBER( in_pb_cb_trampoline_r ) { return m_in_pb_cb(); }
	DECLARE_WRITE8_MEMBER( out_pb_cb_trampoline_w ) { m_out_pb_cb(data); }
	DECLARE_WRITE_LINE_MEMBER( out_brdy_cb_trampoline_w ) { m_out_brdy_cb(state); }

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	const address_space_config m_io_space_config;

	const address_space_config *memory_space_config(address_spacenum spacenum) const override
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			default: return z80_device::memory_space_config(spacenum);
		}
	}

private:
	// devices/pointers
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_sio;
	required_device<z80pio_device> m_pio;

	// internal state
	UINT8 m_irq_priority;

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

	devcb_write_line m_zc0_cb;
	devcb_write_line m_zc1_cb;
	devcb_write_line m_zc2_cb;

	devcb_read8 m_in_pa_cb;
	devcb_write8 m_out_pa_cb;
	devcb_write_line m_out_ardy_cb;

	devcb_read8 m_in_pb_cb;
	devcb_write8 m_out_pb_cb;
	devcb_write_line m_out_brdy_cb;
};


// device type definition
extern const device_type TMPZ84C015;


#endif // __TMPZ84C015__
