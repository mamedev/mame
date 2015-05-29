// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor DS75161A IEEE-488 GPIB Transceiver emulation

**********************************************************************
                            _____   _____
                    TE   1 |*    \_/     | 20  Vcc
                   REN   2 |             | 19  REN
                   IFC   3 |             | 18  IFC
                  NDAC   4 |             | 17  NDAC
                  NRFD   5 |   DS75161A  | 16  NRFD
                   DAV   6 |             | 15  DAV
                   EOI   7 |             | 14  EOI
                   ATN   8 |             | 13  ATN
                   SRQ   8 |             | 12  SRQ
                   GND  10 |_____________| 11  DC

**********************************************************************/

#pragma once

#ifndef __DS75161A__
#define __DS75161A__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_DS75161A_IN_REN_CB(_devcb) \
	devcb = &ds75161a_device::set_in_ren_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_IN_IFC_CB(_devcb) \
	devcb = &ds75161a_device::set_in_ifc_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_IN_NDAC_CB(_devcb) \
	devcb = &ds75161a_device::set_in_ndac_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_IN_NRFD_CB(_devcb) \
	devcb = &ds75161a_device::set_in_nrfd_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_IN_DAV_CB(_devcb) \
	devcb = &ds75161a_device::set_in_dav_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_IN_EOI_CB(_devcb) \
	devcb = &ds75161a_device::set_in_eoi_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_IN_ATN_CB(_devcb) \
	devcb = &ds75161a_device::set_in_atn_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_IN_SRQ_CB(_devcb) \
	devcb = &ds75161a_device::set_in_srq_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_REN_CB(_devcb) \
	devcb = &ds75161a_device::set_out_ren_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_IFC_CB(_devcb) \
	devcb = &ds75161a_device::set_out_ifc_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_NDAC_CB(_devcb) \
	devcb = &ds75161a_device::set_out_ndac_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_NRFD_CB(_devcb) \
	devcb = &ds75161a_device::set_out_nrfd_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_DAV_CB(_devcb) \
	devcb = &ds75161a_device::set_out_dav_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_EOI_CB(_devcb) \
	devcb = &ds75161a_device::set_out_eoi_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_ATN_CB(_devcb) \
	devcb = &ds75161a_device::set_out_atn_callback(*device, DEVCB_##_devcb);

#define MCFG_DS75161A_OUT_SRQ_CB(_devcb) \
	devcb = &ds75161a_device::set_out_srq_callback(*device, DEVCB_##_devcb);

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> ds75161a_device

class ds75161a_device : public device_t
{
public:
	// construction/destruction
	ds75161a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_in_ren_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_ren_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ifc_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_ifc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ndac_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_ndac_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_nrfd_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_nrfd_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_dav_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_dav_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_eoi_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_eoi_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_atn_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_atn_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_srq_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_in_srq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ren_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_ren_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ifc_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_ifc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ndac_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_ndac_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_nrfd_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_nrfd_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dav_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_dav_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_eoi_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_eoi_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_atn_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_atn_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_srq_callback(device_t &device, _Object object) { return downcast<ds75161a_device &>(device).m_out_srq_cb.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( te_w );
	DECLARE_WRITE_LINE_MEMBER( dc_w );

	DECLARE_READ_LINE_MEMBER( ren_r );
	DECLARE_READ_LINE_MEMBER( ifc_r );
	DECLARE_READ_LINE_MEMBER( ndac_r );
	DECLARE_READ_LINE_MEMBER( nrfd_r );
	DECLARE_READ_LINE_MEMBER( dav_r );
	DECLARE_READ_LINE_MEMBER( eoi_r );
	DECLARE_READ_LINE_MEMBER( atn_r );
	DECLARE_READ_LINE_MEMBER( srq_r );

	DECLARE_WRITE_LINE_MEMBER( ren_w );
	DECLARE_WRITE_LINE_MEMBER( ifc_w );
	DECLARE_WRITE_LINE_MEMBER( ndac_w );
	DECLARE_WRITE_LINE_MEMBER( nrfd_w );
	DECLARE_WRITE_LINE_MEMBER( dav_w );
	DECLARE_WRITE_LINE_MEMBER( eoi_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( srq_w );

protected:
	// device-level overrides
	virtual void device_start();

private:
	void update_signals();

	devcb_read_line     m_in_ren_cb;
	devcb_read_line     m_in_ifc_cb;
	devcb_read_line     m_in_ndac_cb;
	devcb_read_line     m_in_nrfd_cb;
	devcb_read_line     m_in_dav_cb;
	devcb_read_line     m_in_eoi_cb;
	devcb_read_line     m_in_atn_cb;
	devcb_read_line     m_in_srq_cb;

	devcb_write_line    m_out_ren_cb;
	devcb_write_line    m_out_ifc_cb;
	devcb_write_line    m_out_ndac_cb;
	devcb_write_line    m_out_nrfd_cb;
	devcb_write_line    m_out_dav_cb;
	devcb_write_line    m_out_eoi_cb;
	devcb_write_line    m_out_atn_cb;
	devcb_write_line    m_out_srq_cb;

	int m_ren;
	int m_ifc;
	int m_ndac;
	int m_nrfd;
	int m_dav;
	int m_eoi;
	int m_atn;
	int m_srq;

	int m_te;
	int m_dc;
};


// device type definition
extern const device_type DS75161A;



#endif
