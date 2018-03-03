// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
    Hitachi HD63450 DMA Controller
*/
#ifndef MAME_MACHINE_HD63450_H
#define MAME_MACHINE_HD63450_H

#pragma once



#define MCFG_HD63450_DMA_END_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_end_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_ERROR_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_error_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_READ_0_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_read_0_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_READ_1_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_read_1_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_READ_2_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_read_2_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_READ_3_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_read_3_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_WRITE_0_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_write_0_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_WRITE_1_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_write_1_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_WRITE_2_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_write_2_callback(DEVCB_##_devcb);

#define MCFG_HD63450_DMA_WRITE_3_CB(_devcb) \
	devcb = &downcast<hd63450_device &>(*device).set_dma_write_3_callback(DEVCB_##_devcb);

#define MCFG_HD63450_CPU(_tag) \
	downcast<hd63450_device &>(*device).set_cpu_tag(_tag);

#define MCFG_HD63450_CLOCKS(_clk1, _clk2, _clk3, _clk4) \
	downcast<hd63450_device &>(*device).set_our_clocks(_clk1, _clk2, _clk3, _clk4);

#define MCFG_HD63450_BURST_CLOCKS(_clk1, _clk2, _clk3, _clk4) \
	downcast<hd63450_device &>(*device).set_burst_clocks(_clk1, _clk2, _clk3, _clk4);

class hd63450_device : public device_t
{
public:
	hd63450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_dma_end_callback(Object &&cb) { return m_dma_end.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_error_callback(Object &&cb) { return m_dma_error.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_read_0_callback(Object &&cb) { return m_dma_read_0.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_read_1_callback(Object &&cb) { return m_dma_read_1.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_read_2_callback(Object &&cb) { return m_dma_read_2.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_read_3_callback(Object &&cb) { return m_dma_read_3.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_write_0_callback(Object &&cb) { return m_dma_write_0.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_write_1_callback(Object &&cb) { return m_dma_write_1.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_write_2_callback(Object &&cb) { return m_dma_write_2.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_write_3_callback(Object &&cb) { return m_dma_write_3.set_callback(std::forward<Object>(cb)); }

	void set_cpu_tag(const char *tag) { m_cpu_tag = tag; }
	void set_our_clocks(const attotime &clk1, const attotime &clk2, const attotime &clk3, const attotime &clk4)
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

	void single_transfer(int x);
	void set_timer(int channel, const attotime &tm);
	int get_vector(int channel);
	int get_error_vector(int channel);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	struct hd63450_regs
	{  // offsets in bytes
		unsigned char csr;  // [00] Channel status register (R/W)
		unsigned char cer;  // [01] Channel error register (R)
		unsigned char dcr;  // [04] Device control register (R/W)
		unsigned char ocr;  // [05] Operation control register (R/W)
		unsigned char scr;  // [06] Sequence control register (R/W)
		unsigned char ccr;  // [07] Channel control register (R/W)
		unsigned short mtc;  // [0a,0b]  Memory Transfer Counter (R/W)
		unsigned long mar;  // [0c-0f]  Memory Address Register (R/W)
		unsigned long dar;  // [14-17]  Device Address Register (R/W)
		unsigned short btc;  // [1a,1b]  Base Transfer Counter (R/W)
		unsigned long bar;  // [1c-1f]  Base Address Register (R/W)
		unsigned char niv;  // [25]  Normal Interrupt Vector (R/W)
		unsigned char eiv;  // [27]  Error Interrupt Vector (R/W)
		unsigned char mfc;  // [29]  Memory Function Code (R/W)
		unsigned char cpr;  // [2d]  Channel Priority Register (R/W)
		unsigned char dfc;  // [31]  Device Function Code (R/W)
		unsigned char bfc;  // [39]  Base Function Code (R/W)
		unsigned char gcr;  // [3f]  General Control Register (R/W)
	};

	devcb_write8 m_dma_end;
	devcb_write8 m_dma_error;
	devcb_read8 m_dma_read_0;
	devcb_read8 m_dma_read_1;
	devcb_read8 m_dma_read_2;
	devcb_read8 m_dma_read_3;
	devcb_write8 m_dma_write_0;
	devcb_write8 m_dma_write_1;
	devcb_write8 m_dma_write_2;
	devcb_write8 m_dma_write_3;

	const char *m_cpu_tag;
	attotime m_our_clock[4];
	attotime m_burst_clock[4];

	// internal state
	hd63450_regs m_reg[4];
	emu_timer* m_timer[4];  // for timing data reading/writing each channel
	int m_in_progress[4];  // if a channel is in use
	int m_transfer_size[4];
	int m_halted[4];  // non-zero if a channel has been halted, and can be continued later.
	cpu_device *m_cpu;
	bool m_drq_state[4];

	TIMER_CALLBACK_MEMBER(dma_transfer_timer);
	void dma_transfer_abort(int channel);
	void dma_transfer_halt(int channel);
	void dma_transfer_continue(int channel);
	void dma_transfer_start(int channel);
};

DECLARE_DEVICE_TYPE(HD63450, hd63450_device)

#endif // MAME_MACHINE_HD63450_H
