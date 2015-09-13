// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Mathis Rosenhauer
/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates all the functionality of 6522
    versatile interface adapters.

    This is based on the pre-existing 6821 emulation.

    Written by Mathis Rosenhauer

**********************************************************************/

#pragma once

#ifndef __6522VIA_H__
#define __6522VIA_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// TODO: REMOVE THESE
#define MCFG_VIA6522_READPA_HANDLER(_devcb) \
	devcb = &via6522_device::set_readpa_handler(*device, DEVCB_##_devcb);

#define MCFG_VIA6522_READPB_HANDLER(_devcb) \
	devcb = &via6522_device::set_readpb_handler(*device, DEVCB_##_devcb);

// TODO: CONVERT THESE TO WRITE LINE
#define MCFG_VIA6522_WRITEPA_HANDLER(_devcb) \
	devcb = &via6522_device::set_writepa_handler(*device, DEVCB_##_devcb);

#define MCFG_VIA6522_WRITEPB_HANDLER(_devcb) \
	devcb = &via6522_device::set_writepb_handler(*device, DEVCB_##_devcb);

#define MCFG_VIA6522_CA2_HANDLER(_devcb) \
	devcb = &via6522_device::set_ca2_handler(*device, DEVCB_##_devcb);

#define MCFG_VIA6522_CB1_HANDLER(_devcb) \
	devcb = &via6522_device::set_cb1_handler(*device, DEVCB_##_devcb);

#define MCFG_VIA6522_CB2_HANDLER(_devcb) \
	devcb = &via6522_device::set_cb2_handler(*device, DEVCB_##_devcb);

#define MCFG_VIA6522_IRQ_HANDLER(_devcb) \
	devcb = &via6522_device::set_irq_handler(*device, DEVCB_##_devcb);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> via6522_device

class via6522_device :  public device_t
{
public:
	// construction/destruction
	via6522_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// TODO: REMOVE THESE
	template<class _Object> static devcb_base &set_readpa_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_in_a_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_readpb_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_in_b_handler.set_callback(object); }

	// TODO: CONVERT THESE TO WRITE LINE
	template<class _Object> static devcb_base &set_writepa_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_out_a_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_writepb_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_out_b_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_ca2_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_ca2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_cb1_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_cb1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_cb2_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_cb2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<via6522_device &>(device).m_irq_handler.set_callback(object); }

	virtual DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( write_pa0 ) { write_pa(0, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pa1 ) { write_pa(1, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pa2 ) { write_pa(2, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pa3 ) { write_pa(3, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pa4 ) { write_pa(4, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pa5 ) { write_pa(5, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pa6 ) { write_pa(6, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pa7 ) { write_pa(7, state); }
	DECLARE_WRITE8_MEMBER( write_pa );
	DECLARE_WRITE_LINE_MEMBER( write_ca1 );
	DECLARE_WRITE_LINE_MEMBER( write_ca2 );

	DECLARE_WRITE_LINE_MEMBER( write_pb0 ) { write_pb(0, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pb1 ) { write_pb(1, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pb2 ) { write_pb(2, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pb3 ) { write_pb(3, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pb4 ) { write_pb(4, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pb5 ) { write_pb(5, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pb6 ) { write_pb(6, state); }
	DECLARE_WRITE_LINE_MEMBER( write_pb7 ) { write_pb(7, state); }
	DECLARE_WRITE8_MEMBER( write_pb );
	DECLARE_WRITE_LINE_MEMBER( write_cb1 );
	DECLARE_WRITE_LINE_MEMBER( write_cb2 );

	enum
	{
		VIA_PB = 0,
		VIA_PA = 1,
		VIA_DDRB = 2,
		VIA_DDRA = 3,
		VIA_T1CL = 4,
		VIA_T1CH = 5,
		VIA_T1LL = 6,
		VIA_T1LH = 7,
		VIA_T2CL = 8,
		VIA_T2CH = 9,
		VIA_SR = 10,
		VIA_ACR = 11,
		VIA_PCR = 12,
		VIA_IFR = 13,
		VIA_IER = 14,
		VIA_PANH = 15
	};

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id TIMER_SHIFT = 0;
	static const device_timer_id TIMER_T1 = 1;
	static const device_timer_id TIMER_T2 = 2;
	static const device_timer_id TIMER_CA2 = 3;

	UINT16 get_counter1_value();

	void set_int(int data);
	void clear_int(int data);
	void shift_out();
	void shift_in();
	void write_pa(int line, int state);
	void write_pb(int line, int state);

	UINT8 input_pa();
	void output_pa();
	UINT8 input_pb();
	void output_pb();
	void output_irq();

	// TODO: REMOVE THESE
	devcb_read8 m_in_a_handler;
	devcb_read8 m_in_b_handler;

	// TODO: CONVERT THESE TO WRITE LINE
	devcb_write8 m_out_a_handler;
	devcb_write8 m_out_b_handler;

	devcb_write_line m_ca2_handler;
	devcb_write_line m_cb1_handler;
	devcb_write_line m_cb2_handler;
	devcb_write_line m_irq_handler;

	UINT8 m_in_a;
	int m_in_ca1;
	int m_in_ca2;
	UINT8 m_out_a;
	int m_out_ca2;
	UINT8 m_ddr_a;
	UINT8 m_latch_a;

	UINT8 m_in_b;
	int m_in_cb1;
	int m_in_cb2;
	UINT8 m_out_b;
	int m_out_cb1;
	int m_out_cb2;
	UINT8 m_ddr_b;
	UINT8 m_latch_b;

	UINT8 m_t1cl;
	UINT8 m_t1ch;
	UINT8 m_t1ll;
	UINT8 m_t1lh;
	UINT8 m_t2cl;
	UINT8 m_t2ch;
	UINT8 m_t2ll;
	UINT8 m_t2lh;

	UINT8 m_sr;
	UINT8 m_pcr;
	UINT8 m_acr;
	UINT8 m_ier;
	UINT8 m_ifr;

	emu_timer *m_t1;
	attotime m_time1;
	UINT8 m_t1_active;
	int m_t1_pb7;
	emu_timer *m_t2;
	attotime m_time2;
	UINT8 m_t2_active;
	emu_timer *m_ca2_timer;

	emu_timer *m_shift_timer;
	UINT8 m_shift_counter;
};


// device type definition
extern const device_type VIA6522;


#endif /* __6522VIA_H__ */
