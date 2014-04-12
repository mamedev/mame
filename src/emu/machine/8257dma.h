// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    Intel 8257 Programmable DMA Controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                 _I/OR   1 |*    \_/     | 40  A7
                 _I/OW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                  MARK   5 |             | 36  TC
                 READY   6 |             | 35  A3
                  HLDA   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                   HRQ  10 |     8257    | 31  Vcc
                   _CS  11 |             | 30  D0
                   CLK  12 |             | 29  D1
                 RESET  13 |             | 28  D2
                _DACK2  14 |             | 27  D3
                _DACK3  15 |             | 26  D4
                  DRQ3  16 |             | 25  _DACK0
                  DRQ2  17 |             | 24  _DACK1
                  DRQ1  18 |             | 23  D5
                  DRQ0  19 |             | 22  D6
                   GND  20 |_____________| 21  D7

***************************************************************************/

#pragma once

#ifndef __I8257__
#define __I8257__

#include "emu.h"


#define I8257_NUM_CHANNELS      (4)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8257_OUT_HRQ_CB(_devcb) \
	devcb = &i8257_device::set_out_hrq_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_OUT_TC_CB(_devcb) \
	devcb = &i8257_device::set_out_tc_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_OUT_MARK_CB(_devcb) \
	devcb = &i8257_device::set_out_mark_callback(*device, DEVCB2_##_devcb);


#define MCFG_I8257_IN_MEMR_CB(_devcb) \
	devcb = &i8257_device::set_in_memr_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_OUT_MEMW_CB(_devcb) \
	devcb = &i8257_device::set_out_memw_callback(*device, DEVCB2_##_devcb);


#define MCFG_I8257_IN_IOR_0_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_0_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_IN_IOR_1_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_1_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_IN_IOR_2_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_2_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_IN_IOR_3_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_3_callback(*device, DEVCB2_##_devcb);


#define MCFG_I8257_OUT_IOW_0_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_0_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_OUT_IOW_1_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_1_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_OUT_IOW_2_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_2_callback(*device, DEVCB2_##_devcb);

#define MCFG_I8257_OUT_IOW_3_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_3_callback(*device, DEVCB2_##_devcb);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> i8257_device

class i8257_device :  public device_t
{
public:
	// construction/destruction
	i8257_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_out_hrq_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_hrq_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_tc_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_tc_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_mark_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_mark_cb.set_callback(object); }

	template<class _Object> static devcb2_base &set_in_memr_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_memr_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_memw_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_memw_cb.set_callback(object); }
	
	template<class _Object> static devcb2_base &set_in_ior_0_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_0_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_in_ior_1_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_1_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_in_ior_2_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_2_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_in_ior_3_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_3_cb.set_callback(object); }
	
	template<class _Object> static devcb2_base &set_out_iow_0_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_0_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_iow_1_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_1_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_iow_2_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_2_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_iow_3_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_3_cb.set_callback(object); }
	
	/* register access */
	DECLARE_READ8_MEMBER( i8257_r );
	DECLARE_WRITE8_MEMBER( i8257_w );

	/* hold acknowledge */
	WRITE_LINE_MEMBER( i8257_hlda_w ) { }

	/* ready */
	WRITE_LINE_MEMBER( i8257_ready_w ) { }

	/* data request */
	WRITE_LINE_MEMBER( i8257_drq0_w ) { i8257_drq_w(0, state); }
	WRITE_LINE_MEMBER( i8257_drq1_w ) { i8257_drq_w(1, state); }
	WRITE_LINE_MEMBER( i8257_drq2_w ) { i8257_drq_w(2, state); }
	WRITE_LINE_MEMBER( i8257_drq3_w ) { i8257_drq_w(3, state); }
	void i8257_drq_w(int channel, int state);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id TIMER_OPERATION = 0;
	static const device_timer_id TIMER_MSBFLIP = 1;
	static const device_timer_id TIMER_DRQ_SYNC = 2;

	int i8257_do_operation(int channel);
	void i8257_update_status();
	void i8257_prepare_msb_flip();

	devcb2_write_line   m_out_hrq_cb;
	devcb2_write_line   m_out_tc_cb;
	devcb2_write_line   m_out_mark_cb;

	/* accessors to main memory */
	devcb2_read8        m_in_memr_cb;
	devcb2_write8       m_out_memw_cb;

	/* channel accesors */
	devcb2_read8        m_in_ior_0_cb;
	devcb2_read8        m_in_ior_1_cb;
	devcb2_read8        m_in_ior_2_cb;
	devcb2_read8        m_in_ior_3_cb;
	devcb2_write8       m_out_iow_0_cb;
	devcb2_write8       m_out_iow_1_cb;
	devcb2_write8       m_out_iow_2_cb;
	devcb2_write8       m_out_iow_3_cb;

	emu_timer *m_timer;
	emu_timer *m_msbflip_timer;

	UINT16 m_registers[I8257_NUM_CHANNELS*2];

	UINT16 m_address[I8257_NUM_CHANNELS];
	UINT16 m_count[I8257_NUM_CHANNELS];
	UINT8  m_rwmode[I8257_NUM_CHANNELS];

	UINT8 m_mode;
	UINT8 m_rr;

	UINT8 m_msb;
	UINT8 m_drq;

	/* bits  0- 3 :  Terminal count for channels 0-3 */
	UINT8 m_status;
};


// device type definition
extern const device_type I8257;

#endif
