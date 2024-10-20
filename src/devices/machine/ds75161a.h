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

#ifndef MAME_MACHINE_DS75161A_H
#define MAME_MACHINE_DS75161A_H

#pragma once

class ds75161a_device : public device_t
{
public:
	// construction/destruction
	ds75161a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto in_ren() { return m_in_ren_cb.bind(); }
	auto in_ifc() { return m_in_ifc_cb.bind(); }
	auto in_ndac() { return m_in_ndac_cb.bind(); }
	auto in_nrfd() { return m_in_nrfd_cb.bind(); }
	auto in_dav() { return m_in_dav_cb.bind(); }
	auto in_eoi() { return m_in_eoi_cb.bind(); }
	auto in_atn() { return m_in_atn_cb.bind(); }
	auto in_srq() { return m_in_srq_cb.bind(); }
	auto out_ren() { return m_out_ren_cb.bind(); }
	auto out_ifc() { return m_out_ifc_cb.bind(); }
	auto out_ndac() { return m_out_ndac_cb.bind(); }
	auto out_nrfd() { return m_out_nrfd_cb.bind(); }
	auto out_dav() { return m_out_dav_cb.bind(); }
	auto out_eoi() { return m_out_eoi_cb.bind(); }
	auto out_atn() { return m_out_atn_cb.bind(); }
	auto out_srq() { return m_out_srq_cb.bind(); }

	void te_w(int state);
	void dc_w(int state);

	int ren_r();
	int ifc_r();
	int ndac_r();
	int nrfd_r();
	int dav_r();
	int eoi_r();
	int atn_r();
	int srq_r();

	void ren_w(int state);
	void ifc_w(int state);
	void ndac_w(int state);
	void nrfd_w(int state);
	void dav_w(int state);
	void eoi_w(int state);
	void atn_w(int state);
	void srq_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

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

DECLARE_DEVICE_TYPE(DS75161A, ds75161a_device)

#endif // MAME_MACHINE_DS75161A_H
