// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor DS75161A IEEE-488 GPIB Transceiver emulation

**********************************************************************/

#include "ds75161a.h"



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

const device_type DS75161A = &device_creator<ds75161a_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ds75161a_device - constructor
//-------------------------------------------------

ds75161a_device::ds75161a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DS75161A, "DS75161A", tag, owner, clock, "ds75161a", __FILE__),
		m_in_ren_cb(*this),
		m_in_ifc_cb(*this),
		m_in_ndac_cb(*this),
		m_in_nrfd_cb(*this),
		m_in_dav_cb(*this),
		m_in_eoi_cb(*this),
		m_in_atn_cb(*this),
		m_in_srq_cb(*this),
		m_out_ren_cb(*this),
		m_out_ifc_cb(*this),
		m_out_ndac_cb(*this),
		m_out_nrfd_cb(*this),
		m_out_dav_cb(*this),
		m_out_eoi_cb(*this),
		m_out_atn_cb(*this),
		m_out_srq_cb(*this),
		m_ren(1),
		m_ifc(1),
		m_ndac(1),
		m_nrfd(1),
		m_dav(1),
		m_eoi(1),
		m_atn(1),
		m_srq(1),
		m_te(0),
		m_dc(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds75161a_device::device_start()
{
	// resolve callbacks
	m_in_ren_cb.resolve_safe(0);
	m_in_ifc_cb.resolve_safe(0);
	m_in_ndac_cb.resolve_safe(0);
	m_in_nrfd_cb.resolve_safe(0);
	m_in_dav_cb.resolve_safe(0);
	m_in_eoi_cb.resolve_safe(0);
	m_in_atn_cb.resolve_safe(0);
	m_in_srq_cb.resolve_safe(0);

	m_out_ren_cb.resolve_safe();
	m_out_ifc_cb.resolve_safe();
	m_out_ndac_cb.resolve_safe();
	m_out_nrfd_cb.resolve_safe();
	m_out_dav_cb.resolve_safe();
	m_out_eoi_cb.resolve_safe();
	m_out_atn_cb.resolve_safe();
	m_out_srq_cb.resolve_safe();

	// register for state saving
	save_item(NAME(m_ren));
	save_item(NAME(m_ifc));
	save_item(NAME(m_ndac));
	save_item(NAME(m_nrfd));
	save_item(NAME(m_dav));
	save_item(NAME(m_eoi));
	save_item(NAME(m_atn));
	save_item(NAME(m_srq));
	save_item(NAME(m_te));
	save_item(NAME(m_dc));
}


//-------------------------------------------------
//  update_signals -
//-------------------------------------------------

void ds75161a_device::update_signals()
{
	m_out_ren_cb(m_dc ? 1 : m_ren);
	m_out_ifc_cb(m_dc ? 1 : m_ifc);
	m_out_ndac_cb(m_te ? 1 : m_ndac);
	m_out_nrfd_cb(m_te ? 1 : m_nrfd);
	m_out_dav_cb(m_te ? m_dav : 1);
	m_out_atn_cb(m_dc ? 1 : m_atn);
	m_out_srq_cb(m_dc ? m_srq : 1 );

	int atn = m_in_atn_cb();

	if (m_te && atn) m_out_eoi_cb(m_eoi);
	else if (!m_dc && !atn) m_out_eoi_cb(m_eoi);
	else m_out_eoi_cb(1);
}


//-------------------------------------------------
//  te_w - transmit enable
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::te_w )
{
	if (m_te != state)
	{
		m_te = state;

		update_signals();
	}
}


//-------------------------------------------------
//  dc_w - direction control
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::dc_w )
{
	if (m_dc != state)
	{
		m_dc = state;

		update_signals();
	}
}


//-------------------------------------------------
//  ren_r - remote enable read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::ren_r )
{
	return m_dc ? m_in_ren_cb() : 0;
}


//-------------------------------------------------
//  ifc_r - interface clear read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::ifc_r )
{
	return m_dc ? m_in_ifc_cb() : 0;
}


//-------------------------------------------------
//  ndac_r - not data acknowledge read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::ndac_r )
{
	return m_te ? m_in_ndac_cb() : 0;
}


//-------------------------------------------------
//  nrfd_r - not ready for data read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::nrfd_r )
{
	return m_te ? m_in_nrfd_cb() : 0;
}


//-------------------------------------------------
//  dav_r - data valid read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::dav_r )
{
	return m_te ? 0 : m_in_dav_cb();
}


//-------------------------------------------------
//  eoi_r - end or identify read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::eoi_r )
{
	int atn = m_in_atn_cb();
	int eoi = m_in_eoi_cb();

	if (!m_te && atn) return eoi;
	else if (m_dc && !atn) return eoi;
	else return 0;
}


//-------------------------------------------------
//  atn_r - attention read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::atn_r )
{
	return m_dc ? m_in_atn_cb() : 0;
}


//-------------------------------------------------
//  srq_r - service request read
//-------------------------------------------------

READ_LINE_MEMBER( ds75161a_device::srq_r )
{
	return m_dc ? 0 : m_in_srq_cb();
}


//-------------------------------------------------
//  ren_w - remote enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::ren_w )
{
	if (m_ren != state)
	{
		m_ren = state;

		update_signals();
	}
}


//-------------------------------------------------
//  ifc_w - interface clear write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::ifc_w )
{
	if (m_ifc != state)
	{
		m_ifc = state;

		update_signals();
	}
}


//-------------------------------------------------
//  ndac_w - not data acknowledge write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::ndac_w )
{
	if (m_ndac != state)
	{
		m_ndac = state;

		update_signals();
	}
}


//-------------------------------------------------
//  nrfd_w - not ready for data write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::nrfd_w )
{
	if (m_nrfd != state)
	{
		m_nrfd = state;

		update_signals();
	}
}


//-------------------------------------------------
//  dav_w - data valid write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::dav_w )
{
	if (m_dav != state)
	{
		m_dav = state;

		update_signals();
	}
}


//-------------------------------------------------
//  eoi_w - end or identify write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::eoi_w )
{
	if (m_eoi != state)
	{
		m_eoi = state;

		update_signals();
	}
}


//-------------------------------------------------
//  atn_w - attention write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::atn_w )
{
	if (m_atn != state)
	{
		m_atn = state;

		update_signals();
	}
}


//-------------------------------------------------
//  srq_w - service request write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds75161a_device::srq_w )
{
	if (m_srq != state)
	{
		m_srq = state;

		update_signals();
	}
}
