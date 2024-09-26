// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Intersil IM6402 Universal Asynchronous Receiver/Transmitter emulation

****************************************************************************
                            _____   _____
                   Vcc   1 |*    \_/     | 40  NOTE
                  NOTE   2 |             | 39  EPE
                   GND   3 |             | 38  CLS1
                   RRD   4 |             | 37  CLS2
                  RBR8   5 |             | 36  SBS
                  RBR7   6 |             | 35  PI
                  RBR6   7 |             | 34  CRL
                  RBR5   8 |             | 33  TBR8
                  RBR4   9 |             | 32  TBR7
                  RBR3  10 |    IM6402   | 31  TBR6
                  RBR2  11 |    IM6403   | 30  TBR5
                  RBR1  12 |             | 29  TBR4
                    PE  13 |             | 28  TBR3
                    FE  14 |             | 27  TBR2
                    OE  15 |             | 26  TBR1
                   SFD  16 |             | 25  TRO
                  NOTE  17 |             | 24  TRE
                   DRR  18 |             | 23  TRBL
                    DR  19 |             | 22  TBRE
                   RRI  20 |_____________| 21  MR


                NOTE:   PIN     IM6402      IM6403
                        ---------------------------
                        2       N/C         CONTROL
                        17      RRC         OSC IN
                        40      TRC         OSC OUT

***************************************************************************/

#ifndef MAME_MACHINE_IM6402_H
#define MAME_MACHINE_IM6402_H

#pragma once

#include "diserial.h"

class im6402_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	im6402_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t rrc, uint32_t trc)
		: im6402_device(mconfig, tag, owner, 0)
	{
		set_rrc(rrc);
		set_trc(trc);
	}

	im6402_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_rrc(int rrc) { m_rrc = rrc; }
	void set_trc(int trc) { m_trc = trc; }

	auto tro_callback() { return m_write_tro.bind(); }
	auto dr_callback() { return m_write_dr.bind(); }
	auto tbre_callback() { return m_write_tbre.bind(); }
	auto tre_callback() { return m_write_tre.bind(); }

	uint8_t read() { return m_rbr; }
	void write(uint8_t data);

	int dr_r() { return m_dr; }
	int tbre_r() { return m_tbre; }
	int tre_r() { return m_tre; }
	int pe_r() { return m_pe; }
	int fe_r() { return m_fe; }
	int oe_r() { return m_oe; }

	void rri_w(int state);
	void rrc_w(int state);
	void trc_w(int state);
	void rrd_w(int state);
	void sfd_w(int state);
	void drr_w(int state);
	void mr_w(int state);
	void crl_w(int state);
	void pi_w(int state);
	void sbs_w(int state);
	void cls1_w(int state);
	void cls2_w(int state);
	void epe_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	inline void set_dr(int state);
	inline void set_tbre(int state);
	inline void set_tre(int state);

	devcb_write_line   m_write_tro;
	devcb_write_line   m_write_dr;
	devcb_write_line   m_write_tbre;
	devcb_write_line   m_write_tre;

	// status
	int m_dr;
	int m_tbre;
	int m_tre;
	int m_pe;
	int m_fe;
	int m_oe;

	// control
	int m_cls1;
	int m_cls2;
	int m_sbs;
	int m_sfd;
	int m_epe;
	int m_pi;

	// receiver
	uint8_t m_rbr;
	int m_rrc;
	int m_rrc_count;

	// transmitter
	uint8_t m_tbr;
	int m_trc;
	int m_trc_count;
};

DECLARE_DEVICE_TYPE(IM6402, im6402_device)

#endif // MAME_MACHINE_IM6402_H
