// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
    Hitachi HD63450 DMA Controller
*/
#ifndef MAME_MACHINE_HD63450_H
#define MAME_MACHINE_HD63450_H

#pragma once

class hd63450_device : public device_t
{
public:
	template <typename T>
	hd63450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cputag)
		: hd63450_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cputag));
	}

	hd63450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_irq_callback.bind(); }
	auto dma_end() { return m_dma_end.bind(); }
	template<int Ch> auto dma_read() { return m_dma_read[Ch].bind(); }
	template<int Ch> auto dma_write() { return m_dma_write[Ch].bind(); }

	template <typename T> void set_cpu_tag(T &&cpu_tag) { m_cpu.set_tag(std::forward<T>(cpu_tag)); }
	void set_clocks(const attotime &clk1, const attotime &clk2, const attotime &clk3, const attotime &clk4)
	{
		m_our_clock[0] = clk1;
		m_our_clock[1] = clk2;
		m_our_clock[2] = clk3;
		m_our_clock[3] = clk4;
	}
	void set_burst_clocks(const attotime &clk1, const attotime &clk2, const attotime &clk3, const attotime &clk4)
	{
		m_burst_clock[0] = clk1;
		m_burst_clock[1] = clk2;
		m_burst_clock[2] = clk3;
		m_burst_clock[3] = clk4;
	}

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER(drq0_w);
	DECLARE_WRITE_LINE_MEMBER(drq1_w);
	DECLARE_WRITE_LINE_MEMBER(drq2_w);
	DECLARE_WRITE_LINE_MEMBER(drq3_w);
	uint8_t iack();

	void single_transfer(int x);
	void set_timer(int channel, const attotime &tm);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	struct hd63450_regs
	{  // offsets in bytes
		uint8_t csr;  // [00] Channel status register (R/W)
		uint8_t cer;  // [01] Channel error register (R)
		uint8_t dcr;  // [04] Device control register (R/W)
		uint8_t ocr;  // [05] Operation control register (R/W)
		uint8_t scr;  // [06] Sequence control register (R/W)
		uint8_t ccr;  // [07] Channel control register (R/W)
		uint16_t mtc; // [0a,0b]  Memory Transfer Counter (R/W)
		uint32_t mar; // [0c-0f]  Memory Address Register (R/W)
		uint32_t dar; // [14-17]  Device Address Register (R/W)
		uint16_t btc; // [1a,1b]  Base Transfer Counter (R/W)
		uint32_t bar; // [1c-1f]  Base Address Register (R/W)
		uint8_t niv;  // [25]  Normal Interrupt Vector (R/W)
		uint8_t eiv;  // [27]  Error Interrupt Vector (R/W)
		uint8_t mfc;  // [29]  Memory Function Code (R/W)
		uint8_t cpr;  // [2d]  Channel Priority Register (R/W)
		uint8_t dfc;  // [31]  Device Function Code (R/W)
		uint8_t bfc;  // [39]  Base Function Code (R/W)
		uint8_t gcr;  // [3f]  General Control Register (R/W)
	};

	devcb_write_line m_irq_callback;
	devcb_write8 m_dma_end;
	devcb_read8 m_dma_read[4];
	devcb_write8 m_dma_write[4];

	attotime m_our_clock[4];
	attotime m_burst_clock[4];

	// internal state
	hd63450_regs m_reg[4];
	emu_timer* m_timer[4];  // for timing data reading/writing each channel
	uint16_t m_transfer_size[4];
	bool m_halted[4];  // non-zero if a channel has been halted, and can be continued later.
	required_device<cpu_device> m_cpu;
	bool m_drq_state[4];

	int8_t m_irq_channel;

	// tell if a channel is in use
	bool dma_in_progress(int channel) const { return (m_reg[channel].csr & 0x08) != 0; }

	TIMER_CALLBACK_MEMBER(dma_transfer_timer);
	void dma_transfer_abort(int channel);
	void dma_transfer_halt(int channel);
	void dma_transfer_continue(int channel);
	void dma_transfer_start(int channel);
	void set_error(int channel, uint8_t code);

	// interrupt helpers
	void set_irq(int channel);
	void clear_irq(int channel);
};

DECLARE_DEVICE_TYPE(HD63450, hd63450_device)

#endif // MAME_MACHINE_HD63450_H
