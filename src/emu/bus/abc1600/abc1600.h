// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Expansion Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                              A     B
                -12 V   ---   *  1  *   --- -12V
                0 V     ---   *  2  *   --- 0 V
                BPCLK*  ---   *  3  *   --- BPCLK
                0 V     ---   *  4  *   --- 0 V
                INT*    ---   *  5  *   --- 0 V
                D7      ---   *  6  *   --- 0 V
                D6      ---   *  7  *   ---
                D5      ---   *  8  *   ---
                D4      ---   *  9  *   ---
                D3      ---   * 10  *   --- XINT*5^
                D2      ---   * 11  *   --- XINT*4^
                D1      ---   * 12  *   --- XINT*3^
                D0      ---   * 13  *   --- XINT*2^
                CSB*    ---   * 14  *   --- XCSB*2^
                BRST*   ---   * 15  *   --- XCSB*3^
                STAT*   ---   * 16  *   --- XCSB*4^
                INP*    ---   * 17  *   --- XCSB*5^
                C4*     ---   * 18  *   ---
                C3*     ---   * 19  *   ---
                C2*     ---   * 20  *   ---
                C1*     ---   * 21  *   --- EXP*^
                OUT*    ---   * 22  *   --- BUSEN*^
                CS*     ---   * 23  *   --- DSTB*
                NMI*^   ---   * 24  *   --- 0 V
                OPS*    ---   * 25  *   --- A4
                        ---   * 26  *   --- A3
                TREN*   ---   * 27  *   --- A2
                TRRQ*   ---   * 28  *   --- A1
                PRAC*   ---   * 29  *   --- A0
                PREN*   ---   * 30  *   --- DIRW/R*
                +5 V    ---   * 31  *   --- +5 V
                +12 V   ---   * 32  *   --- +12 V

                    ^ only connected on BUS0X

**********************************************************************/

#pragma once

#ifndef __ABC1600BUS__
#define __ABC1600BUS__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define ABC1600BUS_TAG          "abc1600bus"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define ABC1600BUS_INTERFACE(_name) \
	const abc1600bus_interface (_name) =


#define MCFG_ABC1600BUS_SLOT_ADD(_tag, _config, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ABC1600BUS_SLOT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc1600bus_interface

struct abc1600bus_interface
{
	devcb_write_line    m_out_int_cb;
	devcb_write_line    m_out_pren_cb;
	devcb_write_line    m_out_trrq_cb;

	// the following are connected only on BUS0X
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_xint2_cb;
	devcb_write_line    m_out_xint3_cb;
	devcb_write_line    m_out_xint4_cb;
	devcb_write_line    m_out_xint5_cb;
};


// ======================> device_abc1600bus_card_interface

class abc1600bus_slot_device;

// class representing interface-specific live abc1600bus card
class device_abc1600bus_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_abc1600bus_card_interface(const machine_config &mconfig, device_t &device);

	// required operation overrides
	virtual void abc1600bus_cs(UINT8 data) = 0;
	virtual int abc1600bus_csb() = 0;

	// optional operation overrides
	virtual UINT8 abc1600bus_inp() { return 0xff; };
	virtual void abc1600bus_out(UINT8 data) { };
	virtual UINT8 abc1600bus_stat() { return 0xff; };
	virtual UINT8 abc1600bus_ops() { return 0xff; };
	virtual void abc1600bus_c1(UINT8 data) { };
	virtual void abc1600bus_c2(UINT8 data) { };
	virtual void abc1600bus_c3(UINT8 data) { };
	virtual void abc1600bus_c4(UINT8 data) { };
	virtual void abc1600bus_tren(int state) { };
	virtual void abc1600bus_prac(int state) { };
	virtual UINT8 abc1600bus_exp() { return 0xff; };
	virtual int abc1600bus_xcsb2() { return 1; };
	virtual int abc1600bus_xcsb3() { return 1; };
	virtual int abc1600bus_xcsb4() { return 1; };
	virtual int abc1600bus_xcsb5() { return 1; };

public:
	abc1600bus_slot_device  *m_slot;
};


// ======================> abc1600bus_slot_device

class abc1600bus_slot_device : public device_t,
							   public device_slot_interface,
							   public abc1600bus_interface
{
public:
	// construction/destruction
	abc1600bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// computer interface
	void cs_w(UINT8 data) { if (m_card) m_card->abc1600bus_cs(data); }
	DECLARE_READ_LINE_MEMBER( csb_r ) { return m_card ? m_card->abc1600bus_csb() : 1; }
	void brst_w() { device_reset(); }
	UINT8 inp_r() { return m_card ? m_card->abc1600bus_inp() : 0xff; }
	void out_w(UINT8 data) { if (m_card) m_card->abc1600bus_out(data); }
	UINT8 stat_r() { return m_card ? m_card->abc1600bus_stat() : 0xff; }
	UINT8 ops_r() { return m_card ? m_card->abc1600bus_ops() : 0xff; }
	void c1_w(UINT8 data) { if (m_card) m_card->abc1600bus_c1(data); }
	void c2_w(UINT8 data) { if (m_card) m_card->abc1600bus_c2(data); }
	void c3_w(UINT8 data) { if (m_card) m_card->abc1600bus_c3(data); }
	void c4_w(UINT8 data) { if (m_card) m_card->abc1600bus_c4(data); }
	UINT8 exp_r() { return m_card ? m_card->abc1600bus_exp() : 0xff; }
	DECLARE_READ_LINE_MEMBER( xcsb2_r ) { return m_card ? m_card->abc1600bus_xcsb2() : 1; }
	DECLARE_READ_LINE_MEMBER( xcsb3_r ) { return m_card ? m_card->abc1600bus_xcsb3() : 1; }
	DECLARE_READ_LINE_MEMBER( xcsb4_r ) { return m_card ? m_card->abc1600bus_xcsb4() : 1; }
	DECLARE_READ_LINE_MEMBER( xcsb5_r ) { return m_card ? m_card->abc1600bus_xcsb5() : 1; }
	DECLARE_WRITE_LINE_MEMBER( tren_w ) { if (m_card) m_card->abc1600bus_tren(state); }
	DECLARE_WRITE_LINE_MEMBER( prac_w ) { if (m_card) m_card->abc1600bus_prac(state); }
	DECLARE_READ_LINE_MEMBER( int_r ) { return m_int; }
	DECLARE_READ_LINE_MEMBER( pren_r ) { return m_pren; }
	DECLARE_READ_LINE_MEMBER( trrq_r ) { return m_trrq; }
	DECLARE_READ_LINE_MEMBER( nmi_r ) { return m_nmi; }
	DECLARE_READ_LINE_MEMBER( xint2_r ) { return m_xint2; }
	DECLARE_READ_LINE_MEMBER( xint3_r ) { return m_xint3; }
	DECLARE_READ_LINE_MEMBER( xint4_r ) { return m_xint4; }
	DECLARE_READ_LINE_MEMBER( xint5_r ) { return m_xint5; }

	// card interface
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int = state; m_out_int_func(state); }
	DECLARE_WRITE_LINE_MEMBER( pren_w ) { m_pren = state; m_out_pren_func(state); }
	DECLARE_WRITE_LINE_MEMBER( trrq_w ) { m_trrq = state; m_out_trrq_func(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi = state; m_out_nmi_func(state); }
	DECLARE_WRITE_LINE_MEMBER( xint2_w ) { m_xint2 = state; m_out_xint2_func(state); }
	DECLARE_WRITE_LINE_MEMBER( xint3_w ) { m_xint3 = state; m_out_xint3_func(state); }
	DECLARE_WRITE_LINE_MEMBER( xint4_w ) { m_xint4 = state; m_out_xint4_func(state); }
	DECLARE_WRITE_LINE_MEMBER( xint5_w ) { m_xint5 = state; m_out_xint5_func(state); }

private:
	devcb_resolved_write_line   m_out_int_func;
	devcb_resolved_write_line   m_out_pren_func;
	devcb_resolved_write_line   m_out_trrq_func;
	devcb_resolved_write_line   m_out_nmi_func;
	devcb_resolved_write_line   m_out_xint2_func;
	devcb_resolved_write_line   m_out_xint3_func;
	devcb_resolved_write_line   m_out_xint4_func;
	devcb_resolved_write_line   m_out_xint5_func;

	device_abc1600bus_card_interface *m_card;

	int m_int;
	int m_pren;
	int m_trrq;
	int m_nmi;
	int m_xint2;
	int m_xint3;
	int m_xint4;
	int m_xint5;
};


// device type definition
extern const device_type ABC1600BUS_SLOT;


// slot devices
#include "lux4105.h"
#include "bus/abcbus/lux21046.h"

SLOT_INTERFACE_EXTERN( abc1600bus_cards );



#endif
