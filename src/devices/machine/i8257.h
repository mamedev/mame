// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Intel 8257 DMA Controller emulation

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



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8257_OUT_HRQ_CB(_devcb) \
	devcb = &i8257_device::set_out_hrq_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_TC_CB(_devcb) \
	devcb = &i8257_device::set_out_tc_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_IN_MEMR_CB(_devcb) \
	devcb = &i8257_device::set_in_memr_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_MEMW_CB(_devcb) \
	devcb = &i8257_device::set_out_memw_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_IN_IOR_0_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_0_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_IN_IOR_1_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_1_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_IN_IOR_2_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_2_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_IN_IOR_3_CB(_devcb) \
	devcb = &i8257_device::set_in_ior_3_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_IOW_0_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_0_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_IOW_1_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_1_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_IOW_2_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_2_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_IOW_3_CB(_devcb) \
	devcb = &i8257_device::set_out_iow_3_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_DACK_0_CB(_devcb) \
	devcb = &i8257_device::set_out_dack_0_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_DACK_1_CB(_devcb) \
	devcb = &i8257_device::set_out_dack_1_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_DACK_2_CB(_devcb) \
	devcb = &i8257_device::set_out_dack_2_callback(*device, DEVCB_##_devcb);

#define MCFG_I8257_OUT_DACK_3_CB(_devcb) \
	devcb = &i8257_device::set_out_dack_3_callback(*device, DEVCB_##_devcb);

// HACK: the radio86 and alikes require this, is it a bug in the soviet clone or is there something else happening?
#define MCFG_I8257_REVERSE_RW_MODE(_flag) \
		i8257_device::static_set_reverse_rw_mode(*device, _flag);

// ======================> i8257_device

class i8257_device :  public device_t,
						public device_execute_interface
{
public:
	// construction/destruction
	i8257_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( hlda_w );
	DECLARE_WRITE_LINE_MEMBER( ready_w );

	DECLARE_WRITE_LINE_MEMBER( dreq0_w );
	DECLARE_WRITE_LINE_MEMBER( dreq1_w );
	DECLARE_WRITE_LINE_MEMBER( dreq2_w );
	DECLARE_WRITE_LINE_MEMBER( dreq3_w );

	template<class _Object> static devcb_base &set_out_hrq_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_hrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_tc_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_tc_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_in_memr_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_memr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_memw_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_memw_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_in_ior_0_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_1_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_2_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_3_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_in_ior_3_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_iow_0_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_1_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_2_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_3_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_iow_3_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_dack_0_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_dack_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_1_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_dack_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_2_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_dack_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_3_callback(device_t &device, _Object object) { return downcast<i8257_device &>(device).m_out_dack_3_cb.set_callback(object); }

	static void static_set_reverse_rw_mode(device_t &device, bool flag) { downcast<i8257_device &>(device).m_reverse_rw = flag; }
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void execute_run();

	int m_icount;

private:
	inline void dma_request(int channel, int state);
	inline bool is_request_active(int channel);
	inline void set_hreq(int state);
	inline void set_dack();
	inline void dma_read();
	inline void dma_write();
	inline void advance();
	inline void set_tc(int state);
	bool next_channel();

	bool m_reverse_rw;
	bool m_tc;
	int m_msb;
	int m_hreq;
	int m_hack;
	int m_ready;
	int m_state;
	int m_current_channel;
	int m_last_channel;
	UINT8 m_transfer_mode;
	UINT8 m_status;
	UINT8 m_request;
	UINT8 m_temp;

	devcb_write_line   m_out_hrq_cb;
	devcb_write_line   m_out_tc_cb;

	/* accessors to main memory */
	devcb_read8        m_in_memr_cb;
	devcb_write8       m_out_memw_cb;

	/* channel accessors */
	devcb_read8        m_in_ior_0_cb;
	devcb_read8        m_in_ior_1_cb;
	devcb_read8        m_in_ior_2_cb;
	devcb_read8        m_in_ior_3_cb;
	devcb_write8       m_out_iow_0_cb;
	devcb_write8       m_out_iow_1_cb;
	devcb_write8       m_out_iow_2_cb;
	devcb_write8       m_out_iow_3_cb;
	devcb_write_line   m_out_dack_0_cb;
	devcb_write_line   m_out_dack_1_cb;
	devcb_write_line   m_out_dack_2_cb;
	devcb_write_line   m_out_dack_3_cb;

	struct
	{
		UINT16 m_address;
		UINT16 m_count;
		UINT8 m_mode;
	} m_channel[4];
};


// device type definition
extern const device_type I8257;



#endif
