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
	via6522_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void write_pa0(int state) { write_pa(0, state); }
	void write_pa1(int state) { write_pa(1, state); }
	void write_pa2(int state) { write_pa(2, state); }
	void write_pa3(int state) { write_pa(3, state); }
	void write_pa4(int state) { write_pa(4, state); }
	void write_pa5(int state) { write_pa(5, state); }
	void write_pa6(int state) { write_pa(6, state); }
	void write_pa7(int state) { write_pa(7, state); }
	void write_pa(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_ca1(int state);
	void write_ca2(int state);

	void write_pb0(int state) { write_pb(0, state); }
	void write_pb1(int state) { write_pb(1, state); }
	void write_pb2(int state) { write_pb(2, state); }
	void write_pb3(int state) { write_pb(3, state); }
	void write_pb4(int state) { write_pb(4, state); }
	void write_pb5(int state) { write_pb(5, state); }
	void write_pb6(int state) { write_pb(6, state); }
	void write_pb7(int state) { write_pb(7, state); }
	void write_pb(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_cb1(int state);
	void write_cb2(int state);

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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	static const device_timer_id TIMER_SHIFT = 0;
	static const device_timer_id TIMER_T1 = 1;
	static const device_timer_id TIMER_T2 = 2;
	static const device_timer_id TIMER_CA2 = 3;

	uint16_t get_counter1_value();

	void set_int(int data);
	void clear_int(int data);
	void shift_out();
	void shift_in();
	void write_pa(int line, int state);
	void write_pb(int line, int state);

	uint8_t input_pa();
	void output_pa();
	uint8_t input_pb();
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

	uint8_t m_in_a;
	int m_in_ca1;
	int m_in_ca2;
	uint8_t m_out_a;
	int m_out_ca2;
	uint8_t m_ddr_a;
	uint8_t m_latch_a;

	uint8_t m_in_b;
	int m_in_cb1;
	int m_in_cb2;
	uint8_t m_out_b;
	int m_out_cb1;
	int m_out_cb2;
	uint8_t m_ddr_b;
	uint8_t m_latch_b;

	uint8_t m_t1cl;
	uint8_t m_t1ch;
	uint8_t m_t1ll;
	uint8_t m_t1lh;
	uint8_t m_t2cl;
	uint8_t m_t2ch;
	uint8_t m_t2ll;
	uint8_t m_t2lh;

	uint8_t m_sr;
	uint8_t m_pcr;
	uint8_t m_acr;
	uint8_t m_ier;
	uint8_t m_ifr;

	emu_timer *m_t1;
	attotime m_time1;
	uint8_t m_t1_active;
	int m_t1_pb7;
	emu_timer *m_t2;
	attotime m_time2;
	uint8_t m_t2_active;
	emu_timer *m_ca2_timer;

	emu_timer *m_shift_timer;
	uint8_t m_shift_counter;
};


// device type definition
extern const device_type VIA6522;


#endif /* __6522VIA_H__ */
