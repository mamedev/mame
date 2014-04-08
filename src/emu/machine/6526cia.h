/**********************************************************************

    WARNING: DO NOT USE! WILL BE REMOVED IN FAVOR OF machine/mos6526.h

**********************************************************************

    MOS 6526/8520 Complex Interface Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

**********************************************************************/

#pragma once

#ifndef __6526CIA_H__
#define __6526CIA_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6526_TOD(_clock) \
	legacy_mos6526_device::static_set_tod_clock(*device, _clock);

#define MCFG_MOS6526_IRQ_CALLBACK(_write) \
	devcb = &legacy_mos6526_device::set_irq_wr_callback(*device, DEVCB2_##_write);

#define MCFG_MOS6526_CNT_CALLBACK(_write) \
	devcb = &legacy_mos6526_device::set_cnt_wr_callback(*device, DEVCB2_##_write);

#define MCFG_MOS6526_SP_CALLBACK(_write) \
	devcb = &legacy_mos6526_device::set_sp_wr_callback(*device, DEVCB2_##_write);

#define MCFG_MOS6526_PA_INPUT_CALLBACK(_read) \
	devcb = &legacy_mos6526_device::set_pa_rd_callback(*device, DEVCB2_##_read);

#define MCFG_MOS6526_PA_OUTPUT_CALLBACK(_write) \
	devcb = &legacy_mos6526_device::set_pa_wr_callback(*device, DEVCB2_##_write);

#define MCFG_MOS6526_PB_INPUT_CALLBACK(_read) \
	devcb = &legacy_mos6526_device::set_pb_rd_callback(*device, DEVCB2_##_read);

#define MCFG_MOS6526_PB_OUTPUT_CALLBACK(_write) \
	devcb = &legacy_mos6526_device::set_pb_wr_callback(*device, DEVCB2_##_write);

#define MCFG_MOS6526_PC_CALLBACK(_write) \
	devcb = &legacy_mos6526_device::set_pc_wr_callback(*device, DEVCB2_##_write);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> legacy_mos6526_device

class legacy_mos6526_device :  public device_t
{
public:
	// construction/destruction
	legacy_mos6526_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static void static_set_tod_clock(device_t &device, int clock) { downcast<legacy_mos6526_device &>(device).m_tod_clock = clock; }

	template<class _Object> static devcb2_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb2_base &set_cnt_wr_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_write_sp.set_callback(object); }
	template<class _Object> static devcb2_base &set_sp_wr_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_write_cnt.set_callback(object); }
	template<class _Object> static devcb2_base &set_pa_rd_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_read_pa.set_callback(object); }
	template<class _Object> static devcb2_base &set_pa_wr_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_write_pa.set_callback(object); }
	template<class _Object> static devcb2_base &set_pb_rd_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_read_pb.set_callback(object); }
	template<class _Object> static devcb2_base &set_pb_wr_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_write_pb.set_callback(object); }
	template<class _Object> static devcb2_base &set_pc_wr_callback(device_t &device, _Object object) { return downcast<legacy_mos6526_device &>(device).m_write_pc.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 reg_r(UINT8 offset);
	void reg_w(UINT8 offset, UINT8 data);

	/* port access */
	UINT8 pa_r(UINT8 offset) { return (m_port[0].m_latch | ~m_port[0].m_ddr); }
	UINT8 pb_r(UINT8 offset) { return (m_port[1].m_latch | ~m_port[1].m_ddr); }

	/* interrupt request */
	DECLARE_READ_LINE_MEMBER( irq_r ) { return m_irq; }

	/* time of day clock */
	DECLARE_WRITE_LINE_MEMBER( tod_w ) { if(state) clock_tod(); }

	/* serial counter */
	DECLARE_READ_LINE_MEMBER( cnt_r ) { return m_cnt; }
	DECLARE_WRITE_LINE_MEMBER( cnt_w );

	/* serial port */
	DECLARE_READ_LINE_MEMBER( sp_r ) { return m_sp; }
	DECLARE_WRITE_LINE_MEMBER( sp_w ) { m_sp = state; }

	/* flag */
	DECLARE_WRITE_LINE_MEMBER( flag_w );

	/* port mask */
	void set_port_mask_value(int port, int data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	static TIMER_CALLBACK( timer_proc );
	static TIMER_CALLBACK( clock_tod_callback );

private:
	enum
	{
		TIMER_PC,
		TIMER_TOD
	};

	inline attotime cycles_to_time(int c);
	void update_pc();
	void update_interrupts();
	void timer_bump(int timer);
	void timer_underflow(int timer);
	void increment();
	void clock_tod();

	struct cia_timer
	{
	public:
		cia_timer() { }

		UINT16 get_count();
		void update(int which, INT32 new_count);

		UINT32          m_clock;
		UINT16          m_latch;
		UINT16          m_count;
		UINT8           m_mode;
		UINT8           m_irq;
		emu_timer*      m_timer;
		legacy_mos6526_device*  m_cia;
	};

	struct cia_port
	{
	public:
		UINT8       m_ddr;
		UINT8       m_latch;
		UINT8       m_in;
		UINT8       m_out;
		UINT8       m_mask_value; /* in READ operation the value can be forced by a extern electric circuit */
	};

	devcb2_write_line   m_write_irq;
	devcb2_write_line   m_write_pc;
	devcb2_write_line   m_write_cnt;
	devcb2_write_line   m_write_sp;
	devcb2_read8        m_read_pa;
	devcb2_write8       m_write_pa;
	devcb2_read8        m_read_pb;
	devcb2_write8       m_write_pb;

	cia_port        m_port[2];
	cia_timer       m_timer[2];

	/* Time Of the Day clock (TOD) */
	int             m_tod_clock;
	UINT32          m_tod;
	UINT32          m_tod_latch;
	UINT8           m_tod_latched;
	UINT8           m_tod_running;
	UINT32          m_alarm;

	/* Interrupts */
	UINT8           m_icr;
	UINT8           m_ics;
	UINT8           m_irq;
	int             m_flag;

	/* Serial */
	UINT8           m_loaded;
	UINT8           m_sdr;
	UINT8           m_sp;
	UINT8           m_cnt;
	UINT8           m_shift;
	UINT8           m_serial;

	emu_timer *m_pc_timer;
	emu_timer *m_tod_timer;
};

class legacy_mos6526r1_device : public legacy_mos6526_device
{
public:
	legacy_mos6526r1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class legacy_mos6526r2_device : public legacy_mos6526_device
{
public:
	legacy_mos6526r2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class legacy_mos8520_device : public legacy_mos6526_device
{
public:
	legacy_mos8520_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class legacy_mos5710_device : public legacy_mos6526_device
{
public:
	legacy_mos5710_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const ATTR_DEPRECATED device_type LEGACY_MOS6526R1;
extern const ATTR_DEPRECATED device_type LEGACY_MOS6526R2;
extern const ATTR_DEPRECATED device_type LEGACY_MOS8520;
extern const ATTR_DEPRECATED device_type LEGACY_MOS5710;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* port mask */
void cia_set_port_mask_value(device_t *device, int port, int data);

#endif /* __6526CIA_H__ */
