// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 6526/8520 Complex Interface Adapter emulation

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  CNT
                   PA0   2 |             | 39  SP
                   PA1   3 |             | 38  RS0
                   PA2   4 |             | 37  RS1
                   PA3   5 |             | 36  RS2
                   PA4   6 |             | 35  RS3
                   PA5   7 |             | 34  _RES
                   PA6   8 |             | 33  DB0
                   PA7   9 |             | 32  DB1
                   PB0  10 |   MOS6526   | 31  DB2
                   PB1  11 |   MOS8520   | 30  DB3
                   PB2  12 |             | 29  DB4
                   PB3  13 |             | 28  DB5
                   PB4  14 |             | 27  DB6
                   PB5  15 |             | 26  DB7
                   PB6  16 |             | 25  phi2
                   PB7  17 |             | 24  _FLAG
                   _PC  18 |             | 23  _CS
                   TOD  19 |             | 22  R/W
                   Vcc  20 |_____________| 21  _IRQ

                            _____   _____
                  FCO*   1 |*    \_/     | 48  FDO*
                   TED   2 |             | 47  FCI*
                  phi0   3 |             | 46  FDI*
                 CLKIN   4 |             | 45  IRQ
                 CTRLO   5 |             | 44  RSET
                 CTRLI   6 |             | 43
                  phi2   7 |             | 42
                    D7   8 |             | 41  INDEX*
                    D6   9 |             | 40  WG2*
                    D5  10 |             | 39  WPRT*
                    D4  11 |             | 38  RPULSE
                   GND  12 |   MOS5710   | 37  Q
                   Vcc  13 |             | 36  Vcc
                    D3  14 |             | 35  GND
                    D2  15 |             | 34  CS3*
                    D1  16 |             | 33  CS2*
                    D0  17 |             | 32  CS1*
                   A15  18 |             | 31  R/W*
                   A14  19 |             | 30  OSC
                   A13  20 |             | 29  XTL1
                   A12  21 |             | 28  XTL2
                   A10  22 |             | 27  A0
                    A4  23 |             | 26  A1
                    A3  24 |_____________| 25  A2

**********************************************************************/

#pragma once

#ifndef __MOS6526__
#define __MOS6526__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6526_TOD(_clock) \
	mos6526_device::static_set_tod_clock(*device, _clock);

#define MCFG_MOS6526_IRQ_CALLBACK(_write) \
	devcb = &mos6526_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6526_CNT_CALLBACK(_write) \
	devcb = &mos6526_device::set_cnt_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6526_SP_CALLBACK(_write) \
	devcb = &mos6526_device::set_sp_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6526_PA_INPUT_CALLBACK(_read) \
	devcb = &mos6526_device::set_pa_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6526_PA_OUTPUT_CALLBACK(_write) \
	devcb = &mos6526_device::set_pa_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6526_PB_INPUT_CALLBACK(_read) \
	devcb = &mos6526_device::set_pb_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6526_PB_OUTPUT_CALLBACK(_write) \
	devcb = &mos6526_device::set_pb_wr_callback(*device, DEVCB_##_write);

#define MCFG_MOS6526_PC_CALLBACK(_write) \
	devcb = &mos6526_device::set_pc_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6526_device

class mos6526_device :  public device_t,
						public device_execute_interface
{
public:
	// construction/destruction
	mos6526_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source);
	mos6526_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_tod_clock(device_t &device, int clock) { downcast<mos6526_device &>(device).m_tod_clock = clock; }

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_cnt_wr_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_write_cnt.set_callback(object); }
	template<class _Object> static devcb_base &set_sp_wr_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_write_sp.set_callback(object); }
	template<class _Object> static devcb_base &set_pa_rd_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_read_pa.set_callback(object); }
	template<class _Object> static devcb_base &set_pa_wr_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_write_pa.set_callback(object); }
	template<class _Object> static devcb_base &set_pb_rd_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_read_pb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb_wr_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_write_pb.set_callback(object); }
	template<class _Object> static devcb_base &set_pc_wr_callback(device_t &device, _Object object) { return downcast<mos6526_device &>(device).m_write_pc.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 pa_r() { return m_pa; }
	DECLARE_READ8_MEMBER( pa_r ) { return pa_r(); }
	UINT8 pb_r() { return m_pb; }
	DECLARE_READ8_MEMBER( pb_r ) { return pb_r(); }

	DECLARE_READ_LINE_MEMBER( sp_r ) { return m_sp; }
	DECLARE_WRITE_LINE_MEMBER( sp_w );
	DECLARE_READ_LINE_MEMBER( cnt_r ) { return m_cnt; }
	DECLARE_WRITE_LINE_MEMBER( cnt_w );
	DECLARE_WRITE_LINE_MEMBER( flag_w );
	DECLARE_READ_LINE_MEMBER( irq_r ) { return m_irq; }
	DECLARE_WRITE_LINE_MEMBER( tod_w );

protected:
	enum
	{
		TYPE_6526,
		TYPE_6526A,
		TYPE_8520,
		TYPE_5710
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void execute_run() override;

	int m_icount;
	int m_variant;
	int m_tod_clock;

	inline void update_interrupt();
	inline void update_pa();
	inline void update_pb();
	inline void set_cra(UINT8 data);
	inline void set_crb(UINT8 data);
	inline void serial_input();
	inline void serial_output();
	inline void clock_ta();
	inline void clock_tb();
	inline void clock_pipeline();
	inline UINT8 bcd_increment(UINT8 value);
	virtual inline void clock_tod();
	inline UINT8 read_tod(int offset);
	inline void write_tod(int offset, UINT8 data);
	inline void synchronize();

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_pc;
	devcb_write_line   m_write_cnt;
	devcb_write_line   m_write_sp;
	devcb_read8        m_read_pa;
	devcb_write8       m_write_pa;
	devcb_read8        m_read_pb;
	devcb_write8       m_write_pb;

	// interrupts
	bool m_irq;
	int m_ir0;
	int m_ir1;
	UINT8 m_icr;
	UINT8 m_imr;
	bool m_icr_read;

	// peripheral ports
	int m_pc;
	int m_flag;
	UINT8 m_pra;
	UINT8 m_prb;
	UINT8 m_ddra;
	UINT8 m_ddrb;
	UINT8 m_pa;
	UINT8 m_pb;
	UINT8 m_pa_in;
	UINT8 m_pb_in;

	// serial
	int m_sp;
	int m_cnt;
	UINT8 m_sdr;
	UINT8 m_shift;
	bool m_sdr_empty;
	int m_bits;

	// timers
	int m_ta_out;
	int m_tb_out;
	int m_ta_pb6;
	int m_tb_pb7;
	int m_count_a0;
	int m_count_a1;
	int m_count_a2;
	int m_count_a3;
	int m_load_a0;
	int m_load_a1;
	int m_load_a2;
	int m_oneshot_a0;
	int m_count_b0;
	int m_count_b1;
	int m_count_b2;
	int m_count_b3;
	int m_load_b0;
	int m_load_b1;
	int m_load_b2;
	int m_oneshot_b0;
	UINT16 m_ta;
	UINT16 m_tb;
	UINT16 m_ta_latch;
	UINT16 m_tb_latch;
	UINT8 m_cra;
	UINT8 m_crb;

	// time-of-day
	int m_tod_count;
	UINT32 m_tod;
	UINT32 m_tod_latch;
	UINT32 m_alarm;
	bool m_tod_stopped;
	bool m_tod_latched;
	emu_timer *m_tod_timer;
};


// ======================> mos6526a_device

class mos6526a_device : public mos6526_device
{
public:
	mos6526a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// ======================> mos8520_device

class mos8520_device : public mos6526_device
{
public:
	mos8520_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	virtual inline void clock_tod() override;
};


// ======================> mos5710_device

class mos5710_device : public mos6526_device
{
public:
	mos5710_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	//DECLARE_READ8_MEMBER( read );
	//DECLARE_WRITE8_MEMBER( write );
};


// device type definition
extern const device_type MOS6526;
extern const device_type MOS6526A;
extern const device_type MOS8520;
extern const device_type MOS5710;



#endif
