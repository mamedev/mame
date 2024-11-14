// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor DS75161A IEEE-488 GPIB Transceiver emulation

**********************************************************************/

#include "emu.h"
#include "ds75161a.h"



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DS75161A, ds75161a_device, "ds75161a", "DS75161A GPIB Transceiver")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ds75161a_device - constructor
//-------------------------------------------------

ds75161a_device::ds75161a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DS75161A, tag, owner, clock),
	m_in_ren_cb(*this, 0),
	m_in_ifc_cb(*this, 0),
	m_in_ndac_cb(*this, 0),
	m_in_nrfd_cb(*this, 0),
	m_in_dav_cb(*this, 0),
	m_in_eoi_cb(*this, 0),
	m_in_atn_cb(*this, 0),
	m_in_srq_cb(*this, 0),
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

void ds75161a_device::te_w(int state)
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

void ds75161a_device::dc_w(int state)
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

int ds75161a_device::ren_r()
{
	return m_dc ? m_in_ren_cb() : 0;
}


//-------------------------------------------------
//  ifc_r - interface clear read
//-------------------------------------------------

int ds75161a_device::ifc_r()
{
	return m_dc ? m_in_ifc_cb() : 0;
}


//-------------------------------------------------
//  ndac_r - not data acknowledge read
//-------------------------------------------------

int ds75161a_device::ndac_r()
{
	return m_te ? m_in_ndac_cb() : 0;
}


//-------------------------------------------------
//  nrfd_r - not ready for data read
//-------------------------------------------------

int ds75161a_device::nrfd_r()
{
	return m_te ? m_in_nrfd_cb() : 0;
}


//-------------------------------------------------
//  dav_r - data valid read
//-------------------------------------------------

int ds75161a_device::dav_r()
{
	return m_te ? 0 : m_in_dav_cb();
}


//-------------------------------------------------
//  eoi_r - end or identify read
//-------------------------------------------------

int ds75161a_device::eoi_r()
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

int ds75161a_device::atn_r()
{
	return m_dc ? m_in_atn_cb() : 0;
}


//-------------------------------------------------
//  srq_r - service request read
//-------------------------------------------------

int ds75161a_device::srq_r()
{
	return m_dc ? 0 : m_in_srq_cb();
}


//-------------------------------------------------
//  ren_w - remote enable write
//-------------------------------------------------

void ds75161a_device::ren_w(int state)
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

void ds75161a_device::ifc_w(int state)
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

void ds75161a_device::ndac_w(int state)
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

void ds75161a_device::nrfd_w(int state)
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

void ds75161a_device::dav_w(int state)
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

void ds75161a_device::eoi_w(int state)
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

void ds75161a_device::atn_w(int state)
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

void ds75161a_device::srq_w(int state)
{
	if (m_srq != state)
	{
		m_srq = state;

		update_signals();
	}
}
