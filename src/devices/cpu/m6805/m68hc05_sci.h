// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    Motorola M68HC05 serial communications interface (SCI)

    The asynchronous SCI of the MC68HC05C4/C8 and relatives: BAUD, SCCR1,
    SCCR2, SCSR and SCDR at $0D-$11 of the host MCU.  The rate is derived
    from the MCU's internal processor clock (E, half the oscillator
    frequency), which is this device's clock.

***************************************************************************/

#ifndef MAME_CPU_M6805_M68HC05_SCI_H
#define MAME_CPU_M6805_M68HC05_SCI_H

#pragma once


class m68hc05_sci_device : public device_t
{
public:
	// implemented bits, for the host's debugger state
	static constexpr u8 BAUD_MASK = 0x37;
	static constexpr u8 SCCR1_MASK = 0xd8;
	static constexpr u8 SCSR_MASK = 0xfe;

	m68hc05_sci_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// TDO output pin and the interrupt request towards the host MCU (level)
	auto tdo_cb() { return m_tdo_cb.bind(); }
	auto irq_cb() { return m_irq_cb.bind(); }

	// RDI input pin
	void rdi_w(int state);

	void map(address_map &map) ATTR_COLD;

	// register access for the host's debugger state (no transmitter or receiver activity,
	// except that a new rate restarts the RT clock)
	u8 baud() const { return m_baud; }
	u8 sccr1() const { return m_sccr1; }
	u8 sccr2() const { return m_sccr2; }
	u8 scsr() const { return m_scsr; }
	u8 scdr() const { return m_rdr; }
	void set_baud(u8 data);
	void set_sccr1(u8 data) { m_sccr1 = data & SCCR1_MASK; }
	void set_sccr2(u8 data);
	void set_scsr(u8 data);
	void set_scdr(u8 data) { m_rdr = data; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

private:
	enum : u8
	{
		BAUD_SCR    = 0x07,

		SCCR1_R8    = 0x80,
		SCCR1_T8    = 0x40,
		SCCR1_M     = 0x10,
		SCCR1_WAKE  = 0x08,

		SCCR2_TIE   = 0x80,
		SCCR2_TCIE  = 0x40,
		SCCR2_RIE   = 0x20,
		SCCR2_ILIE  = 0x10,
		SCCR2_TE    = 0x08,
		SCCR2_RE    = 0x04,
		SCCR2_RWU   = 0x02,
		SCCR2_SBK   = 0x01,

		SCSR_TDRE   = 0x80,
		SCSR_TC     = 0x40,
		SCSR_RDRF   = 0x20,
		SCSR_IDLE   = 0x10,
		SCSR_OR     = 0x08,
		SCSR_NF     = 0x04,
		SCSR_FE     = 0x02,

		// cleared by an SCSR read followed by an SCDR read
		SCSR_RX_FLAGS = SCSR_RDRF | SCSR_IDLE | SCSR_OR | SCSR_NF | SCSR_FE
	};

	// what the transmit shifter is sending
	enum : u8
	{
		TX_IDLE,
		TX_FRAME,
		TX_PREAMBLE,
		TX_BREAK,
		TX_MARK         // the high bit after a break
	};

	// what the receiver is doing
	enum : u8
	{
		RX_IDLE,        // waiting for a start edge
		RX_ARTIFICIAL,  // waiting for the anticipated start after a framing error
		RX_FRAME        // sampling a start bit and the frame after it
	};

	// receiver sampling (figures 5-2 and 5-3), in RT ticks
	static constexpr u32 RT_PER_BIT = 16;
	static constexpr u32 QUALIFIERS = 3;           // high samples needed before RT1
	static constexpr u32 START_SAMPLES = 4;        // RT1, then RT3, RT5 and RT7
	static constexpr u32 BIT_SAMPLES = 3;          // RT8, RT9 and RT10
	static constexpr u32 FIRST_BIT_SAMPLE = 7;     // RT8, counted from RT1

	u8 baud_r();
	void baud_w(u8 data);
	u8 sccr1_r();
	void sccr1_w(u8 data);
	u8 sccr2_r();
	void sccr2_w(u8 data);
	u8 scsr_r();
	u8 scdr_r();
	void scdr_w(u8 data);

	TIMER_CALLBACK_MEMBER(tx_tick);
	TIMER_CALLBACK_MEMBER(rx_sample);
	TIMER_CALLBACK_MEMBER(idle_detected);

	bool te() const { return m_sccr2 & SCCR2_TE; }
	bool re() const { return m_sccr2 & SCCR2_RE; }
	bool break_wanted() const { return m_break_pending || (m_sccr2 & SCCR2_SBK); }
	bool rwu() const { return m_sccr2 & SCCR2_RWU; }
	u32 frame_bits() const { return (m_sccr1 & SCCR1_M) ? 11 : 10; }

	u32 rt_clocks() const;
	u32 bit_clocks() const { return RT_PER_BIT * m_rt_clocks; }
	attotime tick_time(u64 clocks) const;
	u64 next_tick(attotime const &t, u32 period) const;
	void adjust_at(emu_timer &timer, attotime const &when);
	void restart_clock();

	void set_flags(u8 mask);
	void clear_flags(u8 mask);
	void update_irq();
	void update_rate(bool force = false);

	void set_tdo(int state);
	void tx_kick();
	void tx_schedule(u64 tick);
	void tx_next();

	void try_start(attotime const &fall);
	void rx_start(u64 rt1);
	void rx_schedule();
	void rx_idle();
	void rx_frame_done(bool stop);
	void arm_idle(attotime const &from);

	devcb_write_line    m_tdo_cb;
	devcb_write_line    m_irq_cb;
	emu_timer           *m_tx_timer;
	emu_timer           *m_rx_timer;
	emu_timer           *m_idle_timer;

	u8                  m_baud;
	u8                  m_sccr1;
	u8                  m_sccr2;
	u8                  m_scsr, m_scsr_seen;
	u8                  m_rdr;              // receive data register, read through SCDR
	u8                  m_tdr;              // transmit data register, written through SCDR
	bool                m_irq_state;

	attotime            m_epoch;            // the RT clock starts here
	u32                 m_rt_clocks;        // E clocks per RT tick

	// transmitter
	bool                m_tdr_full;         // TDR holds a byte not yet moved to the shifter
	bool                m_preamble_pending; // TE rose: a preamble goes out next
	bool                m_break_pending;    // SBK rose: a break goes out next
	bool                m_tx_scheduled;     // m_tx_timer is due at m_tx_tick
	u64                 m_tx_tick;          // next bit boundary, in E clocks from m_epoch
	u8                  m_tx_state;
	u16                 m_tx_shift;         // bits still to send, LSB first
	u8                  m_tx_count;
	u8                  m_tdo;

	// receiver
	u8                  m_rdi;
	attotime            m_rdi_rise;         // when RDI last went high
	attotime            m_rdi_fall;         // when RDI last went low
	bool                m_line_idle;        // an idle line has been detected
	bool                m_idle_armed;       // IDLE may set (once after each RDRF)
	u8                  m_rx_state;
	u64                 m_rx_rt1;           // RT1 of the start bit, in E clocks from m_epoch
	u8                  m_rx_bits;          // frame length, latched at the start bit
	u8                  m_rx_step;          // next sample: START_SAMPLES, then BIT_SAMPLES per bit
	u8                  m_rx_votes;         // high samples for the current bit (or verification)
	bool                m_rx_noise;
	u16                 m_rx_shift;
};

DECLARE_DEVICE_TYPE(M68HC05_SCI, m68hc05_sci_device)

#endif // MAME_CPU_M6805_M68HC05_SCI_H
