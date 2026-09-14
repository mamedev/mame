// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 6526/8521/8520 Complex Interface Adapter emulation

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
                   PA7   9 |   MOS6526   | 32  DB1
                   PB0  10 |   MOS8521   | 31  DB2
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

#ifndef MAME_MACHINE_MOS6526_H
#define MAME_MACHINE_MOS6526_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6526_device

class mos6526_device :  public device_t,
						public device_execute_interface
{
public:
	// construction/destruction
	mos6526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_tod_clock(int clock) { m_tod_clock = clock; }

	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto cnt_wr_callback() { return m_write_cnt.bind(); }
	auto sp_wr_callback() { return m_write_sp.bind(); }
	auto pa_rd_callback() { return m_read_pa.bind(); }
	auto pa_wr_callback() { return m_write_pa.bind(); }
	auto pb_rd_callback() { return m_read_pb.bind(); }
	auto pb_wr_callback() { return m_write_pb.bind(); }
	auto pc_wr_callback() { return m_write_pc.bind(); }

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	uint8_t pa_r() { return m_pa; }
	uint8_t pb_r() { return m_pb; }

	int sp_r() { return m_sp; }
	void sp_w(int state);
	int cnt_r() { return m_cnt; }
	void cnt_w(int state);
	void flag_w(int state);
	int irq_r() { return m_irq; }
	void tod_w(int state);

protected:
	mos6526_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	TIMER_CALLBACK_MEMBER(advance_tod_clock);

	// On the 6526 an ICR read swallows the readable Timer B flag bit of an
	// underflow landing in the next cycle
	virtual bool icr_read_loses_tb() const { return true; }

	// On the later chips the read bits are only driven to zero a cycle after the
	// read, so one more read still sees the flags it cleared
	virtual bool icr_read_sticky() const { return false; }

	// The 6526 latches IR through one more stage than the later chips, so its
	// IRQ - and the IR bit an ICR read returns - lags theirs by a cycle
	virtual bool irq_one_cycle_early() const { return false; }

	// On the 8520 a timer-high write force-loads and starts a stopped one-shot timer
	virtual bool timer_hi_starts_oneshot() const { return false; }

	// The 8520 latches the interrupt sources a cycle after they form
	virtual bool irq_sources_delayed() const { return false; }

	// The 8520's time-of-day is a plain binary counter, so it has no unused
	// register bits and powers up at zero rather than 01:00:00.0
	virtual uint8_t tod_mask(int offset) const;
	virtual uint32_t tod_reset_value() const { return 0x01000000UL; }

	int m_icount;
	int m_tod_clock;

	void update_interrupt();
	void update_alarm();
	void update_pa();
	void update_pb();
	void set_cra(uint8_t data);
	void set_crb(uint8_t data);
	void serial_input();
	void serial_load();
	void serial_output();
	void clock_ta();
	void clock_tb();
	void clock_pipeline();
	void clock_tod_divider();
	uint8_t increment_digits(uint8_t value);
	uint8_t increment_hour(uint8_t value);
	virtual void clock_tod();
	uint8_t read_tod(int offset);
	void write_tod(int offset, uint8_t data);
	void synchronize();

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
	int m_irq_pending;
	uint8_t m_icr;
	uint8_t m_imr;
	bool m_icr_read;
	bool m_icr_tb_lost;
	uint8_t m_icr_delay;
	uint8_t m_icr_sticky;
	uint8_t m_icr_sticky_next;

	// peripheral ports
	int m_pc;
	int m_prb_access;
	uint8_t m_prb_rw;
	int m_flag;
	int m_flag_pending;
	uint8_t m_pra;
	uint8_t m_prb;
	uint8_t m_ddra;
	uint8_t m_ddrb;
	uint8_t m_pa;
	uint8_t m_pb;
	uint8_t m_pa_in;
	uint8_t m_pb_in;

	// serial
	int m_sp;
	int m_cnt;
	uint8_t m_sdr;
	uint8_t m_shift;
	bool m_sdr_empty;
	bool m_shift_loaded;
	int m_bits;
	int m_sp_delay;
	int m_sdr_load_delay;
	uint8_t m_cnt_hist;
	bool m_sdr_force_finish;

	// timers
	int m_ta_out;
	int m_ta_out_last;
	int m_tb_out;
	int m_ta_pb6;
	int m_tb_pb7;
	int m_count_a0;
	int m_count_a1;
	int m_count_a2;
	int m_count_a3;
	int m_feed_a0;
	int m_load_a0;
	int m_load_a1;
	int m_load_a2;
	int m_oneshot_a0;
	int m_count_b0;
	int m_count_b1;
	int m_count_b2;
	int m_count_b3;
	int m_feed_b0;
	int m_load_b0;
	int m_load_b1;
	int m_load_b2;
	int m_oneshot_b0;
	uint16_t m_ta;
	uint16_t m_tb;
	uint16_t m_ta_latch;
	uint16_t m_tb_latch;
	uint8_t m_cra;
	uint8_t m_crb;

	// time-of-day
	int m_tod_in;
	int m_tod_pending;
	int m_tod_div;
	int m_alarm_pending;
	int m_tod_count;
	uint32_t m_tod;
	uint32_t m_tod_latch;
	uint32_t m_alarm;
	bool m_tod_stopped;
	bool m_tod_latched;
	bool m_alarm_match;
	emu_timer *m_tod_timer;
};


// ======================> mos6526a_device

class mos6526a_device : public mos6526_device
{
public:
	mos6526a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual bool icr_read_loses_tb() const override { return false; }
	virtual bool icr_read_sticky() const override { return true; }
	virtual bool irq_one_cycle_early() const override { return true; }
};


// ======================> mos8521_device

class mos8521_device : public mos6526_device
{
public:
	mos8521_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// modelled as the 6526A: the two share these differences from the 6526, but
	// what else separates them has not been established
	virtual bool icr_read_loses_tb() const override { return false; }
	virtual bool icr_read_sticky() const override { return true; }
	virtual bool irq_one_cycle_early() const override { return true; }
};


// ======================> mos8520_device

class mos8520_device : public mos6526_device
{
public:
	mos8520_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void clock_tod() override;
	virtual bool icr_read_loses_tb() const override { return false; }
	virtual bool icr_read_sticky() const override { return true; }
	virtual bool irq_one_cycle_early() const override { return true; }
	virtual bool timer_hi_starts_oneshot() const override { return true; }
	virtual bool irq_sources_delayed() const override { return true; }
	virtual uint8_t tod_mask(int offset) const override { return 0xff; }
	virtual uint32_t tod_reset_value() const override { return 0; }
};


// device type definition
DECLARE_DEVICE_TYPE(MOS6526,  mos6526_device)
DECLARE_DEVICE_TYPE(MOS6526A, mos6526a_device)
DECLARE_DEVICE_TYPE(MOS8521,  mos8521_device)
DECLARE_DEVICE_TYPE(MOS8520,  mos8520_device)

#endif // MAME_MACHINE_MOS6526_H
