// license:BSD-3-Clause
// copyright-holders:hap, Charles MacDonald
/**********************************************************************

    Sega 315-5296 I/O chip

**********************************************************************/

#pragma once

#ifndef _SEGA_315_5296_H
#define _SEGA_315_5296_H

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// A to H 8-bit input ports
#define MCFG_315_5296_IN_PORTA_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_pa_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_IN_PORTB_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_pb_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_IN_PORTC_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_pc_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_IN_PORTD_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_pd_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_IN_PORTE_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_pe_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_IN_PORTF_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_pf_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_IN_PORTG_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_pg_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_IN_PORTH_CB(_devcb) \
	devcb = &sega_315_5296_device::set_in_ph_callback(*device, DEVCB_##_devcb);

// A to H 8-bit output ports
#define MCFG_315_5296_OUT_PORTA_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_pa_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_PORTB_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_pb_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_PORTC_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_pc_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_PORTD_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_pd_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_PORTE_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_pe_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_PORTF_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_pf_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_PORTG_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_pg_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_PORTH_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_ph_callback(*device, DEVCB_##_devcb);

// CNT output pins
#define MCFG_315_5296_OUT_CNT0_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_cnt0_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_CNT1_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_cnt1_callback(*device, DEVCB_##_devcb);
#define MCFG_315_5296_OUT_CNT2_CB(_devcb) \
	devcb = &sega_315_5296_device::set_out_cnt2_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sega_315_5296_device

class sega_315_5296_device : public device_t
{
public:
	sega_315_5296_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_in_pa_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pb_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pc_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_pc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pd_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_pd_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pe_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_pe_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pf_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_pf_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pg_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_pg_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ph_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_in_ph_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_pa_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pb_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pc_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_pc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pd_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_pd_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pe_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_pe_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pf_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_pf_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pg_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_pg_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ph_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_ph_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_cnt0_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_cnt0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_cnt1_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_cnt1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_cnt2_callback(device_t &device, _Object object) { return downcast<sega_315_5296_device &>(device).m_out_cnt2_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 debug_peek_output(offs_t offset) { return m_output_latch[offset & 7]; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read8 m_in_pa_cb;
	devcb_read8 m_in_pb_cb;
	devcb_read8 m_in_pc_cb;
	devcb_read8 m_in_pd_cb;
	devcb_read8 m_in_pe_cb;
	devcb_read8 m_in_pf_cb;
	devcb_read8 m_in_pg_cb;
	devcb_read8 m_in_ph_cb;

	devcb_write8 m_out_pa_cb;
	devcb_write8 m_out_pb_cb;
	devcb_write8 m_out_pc_cb;
	devcb_write8 m_out_pd_cb;
	devcb_write8 m_out_pe_cb;
	devcb_write8 m_out_pf_cb;
	devcb_write8 m_out_pg_cb;
	devcb_write8 m_out_ph_cb;

	devcb_write_line m_out_cnt0_cb;
	devcb_write_line m_out_cnt1_cb;
	devcb_write_line m_out_cnt2_cb;

	devcb_read8 *m_in_port_cb[8];
	devcb_write8 *m_out_port_cb[8];
	devcb_write_line *m_out_cnt_cb[3];

	UINT8 m_output_latch[8];
	UINT8 m_cnt;
	UINT8 m_dir;
};

// device type definition
extern const device_type SEGA_315_5296;


#endif /* _SEGA_315_5296_H */
