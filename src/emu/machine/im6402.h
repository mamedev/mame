/***************************************************************************

    Intersil IM6402 Universal Asynchronous Receiver/Transmitter emulation

    Copyright the MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_IM6402_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, IM6402, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define IM6402_INTERFACE(_name) \
	const im6402_interface (_name) =



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> im6402_interface

struct im6402_interface
{
	int m_rrc;
	int m_trc;

	devcb_read_line		m_in_rri_cb;
	devcb_write_line	m_out_tro_cb;
	devcb_write_line	m_out_dr_cb;
	devcb_write_line	m_out_tbre_cb;
	devcb_write_line	m_out_tre_cb;
};


// ======================> im6402_device

class im6402_device :  public device_t,
					   public device_serial_interface,
					   public im6402_interface
{
public:
	// construction/destruction
	im6402_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ_LINE_MEMBER( dr_r );
	DECLARE_READ_LINE_MEMBER( tbre_r );
	DECLARE_READ_LINE_MEMBER( tre_r );
	DECLARE_READ_LINE_MEMBER( pe_r );
	DECLARE_READ_LINE_MEMBER( fe_r );
	DECLARE_READ_LINE_MEMBER( oe_r );

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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_serial_interface overrides
	virtual void input_callback(UINT8 state);

private:
	inline void set_dr(int state);
	inline void set_tbre(int state);
	inline void set_tre(int state);
	inline void receive();
	inline void transmit();

	enum 
	{
		TIMER_RX,
		TIMER_TX
	};

	devcb_resolved_read_line	m_in_rri_func;
	devcb_resolved_write_line	m_out_tro_func;
	devcb_resolved_write_line	m_out_dr_func;
	devcb_resolved_write_line	m_out_tbre_func;
	devcb_resolved_write_line	m_out_tre_func;

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
	int	m_rrc_count;

	// transmitter
	UINT8 m_tbr;
	int	m_trc_count;

	// timers
	emu_timer *m_rx_timer;
	emu_timer *m_tx_timer;
};


// device type definition
extern const device_type IM6402;



#endif
