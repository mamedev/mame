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

#pragma once

#ifndef __IM6402__
#define __IM6402__

#include "emu.h"



/***************************************************************************
    INTERFACE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_IM6402_ADD(_tag, _rrc, _trc) \
	MCFG_DEVICE_ADD(_tag, IM6402, 0) \
	im6402_device::set_rrc(*device, _rrc); \
	im6402_device::set_trc(*device, _trc);

#define MCFG_IM6402_TRO_CALLBACK(_write) \
	devcb = &im6402_device::set_tro_wr_callback(*device, DEVCB_##_write);

#define MCFG_IM6402_DR_CALLBACK(_write) \
	devcb = &im6402_device::set_dr_wr_callback(*device, DEVCB_##_write);

#define MCFG_IM6402_TBRE_CALLBACK(_write) \
	devcb = &im6402_device::set_tbre_wr_callback(*device, DEVCB_##_write);

#define MCFG_IM6402_TRE_CALLBACK(_write) \
	devcb = &im6402_device::set_tre_wr_callback(*device, DEVCB_##_write);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> im6402_device

class im6402_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	im6402_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_rrc(device_t &device, int rrc) { downcast<im6402_device &>(device).m_rrc = rrc; }
	static void set_trc(device_t &device, int trc) { downcast<im6402_device &>(device).m_trc = trc; }
	template<class _Object> static devcb_base &set_tro_wr_callback(device_t &device, _Object object) { return downcast<im6402_device &>(device).m_write_tro.set_callback(object); }
	template<class _Object> static devcb_base &set_dr_wr_callback(device_t &device, _Object object) { return downcast<im6402_device &>(device).m_write_dr.set_callback(object); }
	template<class _Object> static devcb_base &set_tbre_wr_callback(device_t &device, _Object object) { return downcast<im6402_device &>(device).m_write_tbre.set_callback(object); }
	template<class _Object> static devcb_base &set_tre_wr_callback(device_t &device, _Object object) { return downcast<im6402_device &>(device).m_write_tre.set_callback(object); }

	DECLARE_READ8_MEMBER( read ) { return m_rbr; }
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ_LINE_MEMBER( dr_r ) { return m_dr; }
	DECLARE_READ_LINE_MEMBER( tbre_r ) { return m_tbre; }
	DECLARE_READ_LINE_MEMBER( tre_r ) { return m_tre; }
	DECLARE_READ_LINE_MEMBER( pe_r ) { return m_pe; }
	DECLARE_READ_LINE_MEMBER( fe_r ) { return m_fe; }
	DECLARE_READ_LINE_MEMBER( oe_r ) { return m_oe; }

	DECLARE_WRITE_LINE_MEMBER( write_rri ); // receiver register input
	DECLARE_WRITE_LINE_MEMBER( rrc_w );
	DECLARE_WRITE_LINE_MEMBER( trc_w );
	DECLARE_WRITE_LINE_MEMBER( rrd_w );
	DECLARE_WRITE_LINE_MEMBER( sfd_w );
	DECLARE_WRITE_LINE_MEMBER( drr_w );
	DECLARE_WRITE_LINE_MEMBER( mr_w );
	DECLARE_WRITE_LINE_MEMBER( crl_w );
	DECLARE_WRITE_LINE_MEMBER( pi_w );
	DECLARE_WRITE_LINE_MEMBER( sbs_w );
	DECLARE_WRITE_LINE_MEMBER( cls1_w );
	DECLARE_WRITE_LINE_MEMBER( cls2_w );
	DECLARE_WRITE_LINE_MEMBER( epe_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
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
	UINT8 m_rbr;
	int m_rrc;
	int m_rrc_count;

	// transmitter
	UINT8 m_tbr;
	int m_trc;
	int m_trc_count;
};


// device type definition
extern const device_type IM6402;



#endif
