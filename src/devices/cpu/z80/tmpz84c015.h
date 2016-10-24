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
	tmpz84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);

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
	void rxa_w(int state) { m_sio->rxa_w(state); }
	void rxb_w(int state) { m_sio->rxb_w(state); }
	void ctsa_w(int state) { m_sio->ctsa_w(state); }
	void ctsb_w(int state) { m_sio->ctsb_w(state); }
	void dcda_w(int state) { m_sio->dcda_w(state); }
	void dcdb_w(int state) { m_sio->dcdb_w(state); }
	void ria_w(int state) { m_sio->ria_w(state); }
	void rib_w(int state) { m_sio->rib_w(state); }
	void rxca_w(int state) { m_sio->rxca_w(state); }
	void rxcb_w(int state) { m_sio->rxcb_w(state); }
	void txca_w(int state) { m_sio->txca_w(state); }
	void txcb_w(int state) { m_sio->txcb_w(state); }
	void rxtxcb_w(int state) { m_sio->rxtxcb_w(state); }
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

	void pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_pio->pa_w(space, offset, data, mem_mask); }
	uint8_t pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_pio->pa_r(space, offset, mem_mask); }
	void pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_pio->pb_w(space, offset, data, mem_mask); }
	uint8_t pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_pio->pb_r(space, offset, mem_mask); }

	/////////////////////////////////////////////////////////

	void irq_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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

	void zc0_cb_trampoline_w(int state) { m_zc0_cb(state); }
	void zc1_cb_trampoline_w(int state) { m_zc1_cb(state); }
	void zc2_cb_trampoline_w(int state) { m_zc2_cb(state); }

	uint8_t in_pa_cb_trampoline_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_in_pa_cb(); }
	void out_pa_cb_trampoline_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_out_pa_cb(data); }
	void out_ardy_cb_trampoline_w(int state) { m_out_ardy_cb(state); }

	uint8_t in_pb_cb_trampoline_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return m_in_pb_cb(); }
	void out_pb_cb_trampoline_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_out_pb_cb(data); }
	void out_brdy_cb_trampoline_w(int state) { m_out_brdy_cb(state); }

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
