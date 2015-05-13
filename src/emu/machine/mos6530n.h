// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6530 Memory, I/O, Timer Array emulation

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  PA1
                   PA0   2 |             | 39  PA2
                  phi2   3 |             | 38  PA3
                   RS0   4 |             | 37  PA4
                    A9   5 |             | 36  PA5
                    A8   6 |             | 35  PA6
                    A7   7 |             | 34  PA7
                    A6   8 |             | 33  DB0
                   R/W   9 |             | 32  DB1
                    A5  10 |   MCS6530   | 31  DB2
                    A4  11 |             | 30  DB3
                    A3  12 |             | 29  DB4
                    A2  13 |             | 28  DB5
                    A1  14 |             | 27  DB6
                    A0  15 |             | 26  DB7
                  _RES  16 |             | 25  PB0
               IRQ/PB7  17 |             | 24  PB1
               CS1/PB6  18 |             | 23  PB2
               CS2/PB5  19 |             | 22  PB3
                   Vcc  20 |_____________| 21  PB4

**********************************************************************/

#pragma once

#ifndef __MOS6530n__
#define __MOS6530n__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6530n_IRQ_CB(_write) \
	devcb = &mos6530_t::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_IN_PA_CB(_read) \
	devcb = &mos6530_t::set_pa_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_OUT_PA_CB(_write) \
	devcb = &mos6530_t::set_pa_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_IN_PB_CB(_read) \
	devcb = &mos6530_t::set_pb_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_OUT_PB_CB(_write) \
	devcb = &mos6530_t::set_pb_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_IN_PA0_CB(_read) \
	devcb = &mos6530_t::set_pa0_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PA1_CB(_read) \
	devcb = &mos6530_t::set_pa1_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PA2_CB(_read) \
	devcb = &mos6530_t::set_pa2_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PA3_CB(_read) \
	devcb = &mos6530_t::set_pa3_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PA4_CB(_read) \
	devcb = &mos6530_t::set_pa4_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PA5_CB(_read) \
	devcb = &mos6530_t::set_pa5_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PA6_CB(_read) \
	devcb = &mos6530_t::set_pa6_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PA7_CB(_read) \
	devcb = &mos6530_t::set_pa7_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_OUT_PA0_CB(_write) \
	devcb = &mos6530_t::set_pa0_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PA1_CB(_write) \
	devcb = &mos6530_t::set_pa1_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PA2_CB(_write) \
	devcb = &mos6530_t::set_pa2_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PA3_CB(_write) \
	devcb = &mos6530_t::set_pa3_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PA4_CB(_write) \
	devcb = &mos6530_t::set_pa4_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PA5_CB(_write) \
	devcb = &mos6530_t::set_pa5_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PA6_CB(_write) \
	devcb = &mos6530_t::set_pa6_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PA7_CB(_write) \
	devcb = &mos6530_t::set_pa7_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_IN_PB0_CB(_read) \
	devcb = &mos6530_t::set_pb0_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PB1_CB(_read) \
	devcb = &mos6530_t::set_pb1_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PB2_CB(_read) \
	devcb = &mos6530_t::set_pb2_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PB3_CB(_read) \
	devcb = &mos6530_t::set_pb3_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PB4_CB(_read) \
	devcb = &mos6530_t::set_pb4_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PB5_CB(_read) \
	devcb = &mos6530_t::set_pb5_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PB6_CB(_read) \
	devcb = &mos6530_t::set_pb6_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_IN_PB7_CB(_read) \
	devcb = &mos6530_t::set_pb7_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6530n_OUT_PB0_CB(_write) \
	devcb = &mos6530_t::set_pb0_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PB1_CB(_write) \
	devcb = &mos6530_t::set_pb1_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PB2_CB(_write) \
	devcb = &mos6530_t::set_pb2_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PB3_CB(_write) \
	devcb = &mos6530_t::set_pb3_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PB4_CB(_write) \
	devcb = &mos6530_t::set_pb4_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PB5_CB(_write) \
	devcb = &mos6530_t::set_pb5_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PB6_CB(_write) \
	devcb = &mos6530_t::set_pb6_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6530n_OUT_PB7_CB(_write) \
	devcb = &mos6530_t::set_pb7_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6530_t

class mos6530_t :  public device_t
{
public:
	// construction/destruction
	mos6530_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa0_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa1_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa2_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa3_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa4_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa4_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa5_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa5_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa6_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa6_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa7_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pa7_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa0_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa1_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa2_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa3_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa4_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa4_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa5_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa5_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa6_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa6_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pa7_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pa7_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb0_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb1_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb2_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb3_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb4_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb4_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb5_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb5_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb6_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb6_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb7_rd_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_in_pb7_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb0_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb1_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb2_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb3_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb4_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb4_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb5_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb5_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb6_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb6_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb7_wr_callback(device_t &device, _Object object) { return downcast<mos6530_t &>(device).m_out_pb7_cb.set_callback(object); }

	virtual DECLARE_ADDRESS_MAP(rom_map, 8);
	virtual DECLARE_ADDRESS_MAP(ram_map, 8);
	virtual DECLARE_ADDRESS_MAP(io_map, 8);

	DECLARE_WRITE_LINE_MEMBER( pa0_w ) { pa_w(0, state); }
	DECLARE_WRITE_LINE_MEMBER( pa1_w ) { pa_w(1, state); }
	DECLARE_WRITE_LINE_MEMBER( pa2_w ) { pa_w(2, state); }
	DECLARE_WRITE_LINE_MEMBER( pa3_w ) { pa_w(3, state); }
	DECLARE_WRITE_LINE_MEMBER( pa4_w ) { pa_w(4, state); }
	DECLARE_WRITE_LINE_MEMBER( pa5_w ) { pa_w(5, state); }
	DECLARE_WRITE_LINE_MEMBER( pa6_w ) { pa_w(6, state); }
	DECLARE_WRITE_LINE_MEMBER( pa7_w ) { pa_w(7, state); }

	DECLARE_WRITE_LINE_MEMBER( pb0_w ) { pb_w(0, state); }
	DECLARE_WRITE_LINE_MEMBER( pb1_w ) { pb_w(1, state); }
	DECLARE_WRITE_LINE_MEMBER( pb2_w ) { pb_w(2, state); }
	DECLARE_WRITE_LINE_MEMBER( pb3_w ) { pb_w(3, state); }
	DECLARE_WRITE_LINE_MEMBER( pb4_w ) { pb_w(4, state); }
	DECLARE_WRITE_LINE_MEMBER( pb5_w ) { pb_w(5, state); }
	DECLARE_WRITE_LINE_MEMBER( pb6_w ) { pb_w(6, state); }
	DECLARE_WRITE_LINE_MEMBER( pb7_w ) { pb_w(7, state); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	enum
	{
		IRQ_TIMER = 0x80
	};

	void update_pa();
	void update_pb();
	void pa_w(int bit, int state);
	void pb_w(int bit, int state);

	DECLARE_READ8_MEMBER( rom_r ) { return m_region->base()[offset]; }
	DECLARE_READ8_MEMBER( ram_r ) { return m_ram[offset]; }
	DECLARE_WRITE8_MEMBER( ram_w ) { m_ram[offset] = data; }
	DECLARE_READ8_MEMBER( pa_data_r );
	DECLARE_WRITE8_MEMBER( pa_data_w );
	DECLARE_READ8_MEMBER( pb_data_r );
	DECLARE_WRITE8_MEMBER( pb_data_w );
	DECLARE_READ8_MEMBER( pa_ddr_r ) { return m_pa_ddr; }
	DECLARE_WRITE8_MEMBER( pa_ddr_w );
	DECLARE_READ8_MEMBER( pb_ddr_r ) { return m_pb_ddr; }
	DECLARE_WRITE8_MEMBER( pb_ddr_w );
	DECLARE_READ8_MEMBER( irq_r ) { return m_irq ? 0x80 : 0x00; }
	DECLARE_READ8_MEMBER( timer_r );
	DECLARE_WRITE8_MEMBER( timer_w );

	optional_shared_ptr<UINT8> m_ram;

	devcb_write_line m_irq_cb;
	devcb_read8    m_in_pa_cb;
	devcb_write8   m_out_pa_cb;
	devcb_read8    m_in_pb_cb;
	devcb_write8   m_out_pb_cb;
	devcb_read_line m_in_pa0_cb;
	devcb_read_line m_in_pa1_cb;
	devcb_read_line m_in_pa2_cb;
	devcb_read_line m_in_pa3_cb;
	devcb_read_line m_in_pa4_cb;
	devcb_read_line m_in_pa5_cb;
	devcb_read_line m_in_pa6_cb;
	devcb_read_line m_in_pa7_cb;
	devcb_write_line m_out_pa0_cb;
	devcb_write_line m_out_pa1_cb;
	devcb_write_line m_out_pa2_cb;
	devcb_write_line m_out_pa3_cb;
	devcb_write_line m_out_pa4_cb;
	devcb_write_line m_out_pa5_cb;
	devcb_write_line m_out_pa6_cb;
	devcb_write_line m_out_pa7_cb;
	devcb_read_line m_in_pb0_cb;
	devcb_read_line m_in_pb1_cb;
	devcb_read_line m_in_pb2_cb;
	devcb_read_line m_in_pb3_cb;
	devcb_read_line m_in_pb4_cb;
	devcb_read_line m_in_pb5_cb;
	devcb_read_line m_in_pb6_cb;
	devcb_read_line m_in_pb7_cb;
	devcb_write_line m_out_pb0_cb;
	devcb_write_line m_out_pb1_cb;
	devcb_write_line m_out_pb2_cb;
	devcb_write_line m_out_pb3_cb;
	devcb_write_line m_out_pb4_cb;
	devcb_write_line m_out_pb5_cb;
	devcb_write_line m_out_pb6_cb;
	devcb_write_line m_out_pb7_cb;

	UINT8 m_pa_in;
	UINT8 m_pa_out;
	UINT8 m_pa_ddr;

	UINT8 m_pb_in;
	UINT8 m_pb_out;
	UINT8 m_pb_ddr;

	bool m_ie;
	bool m_irq;

	int m_shift;
	UINT8 m_timer;

	enum {
		IDLE,
		RUNNING,
		RUNNING_INTERRUPT,
		RUNNING_SYNCPOINT,
		RUNNING_AFTER_INTERRUPT
	};

	struct live_info {
		attotime tm;
		attotime period;
		int state, next_state;
		UINT8 value;
		bool irq;
	};

	live_info cur_live, checkpoint_live;
	emu_timer *t_gen;

	void live_start();
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_abort();
	void live_run(const attotime &limit = attotime::never);
};


// device type definition
extern const device_type MOS6530n;



#endif
