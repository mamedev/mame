// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M68000_TMP68301_H
#define MAME_CPU_M68000_TMP68301_H

#pragma once

#include "m68000mcu.h"

class tmp68301_device : public m68000_mcu_device
{
public:
	tmp68301_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_serial_external_clock(u32 hz) { m_serial_external_clock = hz; }
	auto parallel_r_cb() { return m_parallel_r_cb.bind(); }
	auto parallel_w_cb() { return m_parallel_w_cb.bind(); }

	void rx0_w(int state) { serial_rx_w(0, state); }
	void rx1_w(int state) { serial_rx_w(1, state); }
	void rx2_w(int state) { serial_rx_w(2, state); }

	auto tx0_handler() { return m_tx_cb[0].bind(); }
	auto tx1_handler() { return m_tx_cb[1].bind(); }
	auto tx2_handler() { return m_tx_cb[2].bind(); }

	void timer_in_w(int state);

protected:
	tmp68301_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	devcb_read16  m_parallel_r_cb;
	devcb_write16 m_parallel_w_cb;
	devcb_write_line::array<3> m_tx_cb;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void execute_set_input(int inputnum, int state) override;

	void internal_update(uint64_t current_time = 0) override;

	virtual u8 base_timer_irq() const noexcept { return 4; }

	void internal_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	// Address decoder
	u8 m_amar[2], m_aamr[2], m_aacr[3];
	u8 m_ator;
	u16 m_arelr;

	void show_cs(int cs);
	void show_iack();

	void amar_w(int reg, u8 data);
	void aamr_w(int reg, u8 data);
	void aacr_w(int reg, u8 data);

	u8 amar0_r();
	void amar0_w(u8 data);
	u8 amar1_r();
	void amar1_w(u8 data);

	u8 aamr0_r();
	void aamr0_w(u8 data);
	u8 aamr1_r();
	void aamr1_w(u8 data);

	u8 aacr0_r();
	void aacr0_w(u8 data);
	u8 aacr1_r();
	void aacr1_w(u8 data);
	u8 aacr2_r();
	void aacr2_w(u8 data);

	u8 ator_r();
	void ator_w(u8 data);

	u16 arelr_r();
	void arelr_w(offs_t, u16 data, u16 mem_mask);


	// Interrupt controller
	static const char *const interrupt_slot_names[11];
	static const int interrupt_vector_to_slot[32];
	static const int interrupt_slot_to_priority[11];

	u16 m_external_interrupt_state;
	u32 m_interrupt_state;
	u8 m_icr[10];
	u16 m_imr, m_ipr, m_iisr;
	u8 m_ivnr, m_ieir;

	u8 icr_r(offs_t reg);
	void icr_w(offs_t reg, u8 data);
	u16 imr_r();
	void imr_w(offs_t, u16 data, u16 mem_mask);
	u16 ipr_r();
	void ipr_w(offs_t, u16 data, u16 mem_mask);
	u16 iisr_r();
	void iisr_w(offs_t, u16 data, u16 mem_mask);
	u8 ivnr_r();
	void ivnr_w(u8 data);
	u8 ieir_r();
	void ieir_w(u8 data);
	u8 interrupt_callback();
	void interrupt_update();
	void interrupt_internal_trigger(int vector);
	std::tuple<u32, u8, u32> interrupt_get_current() const;

	// Parallel interface
	static const int parallel_mode_table[16];
	static const char *const parallel_mode_names[10];

	u16 m_pdir;
	u8 m_pcr, m_psr, m_pcmr, m_pmr;
	u16 m_pdr;
	u8 m_ppr1, m_ppr2;
	u8 m_parallel_mode;

	u8 parallel_get_interrupt() const;

	u16 pdir_r();
	void pdir_w(offs_t, u16 data, u16 mem_mask);
	u8 pcr_r();
	void pcr_w(u8 data);
	u8 psr_r();
	void psr_w(u8 data);
	u8 pcmr_r();
	void pcmr_w(u8 data);
	u8 pmr_r();
	void pmr_w(u8 data);
	u16 pdr_r();
	void pdr_w(offs_t, u16 data, u16 mem_mask);
	u8 ppr1_r();
	void ppr1_w(u8 data);
	u8 ppr2_r();
	void ppr2_w(u8 data);


	// Serial interface
	enum {
		SCR_INTM   = 0x01,
		SCR_RES    = 0x20,
		SCR_CKSE   = 0x80,

		SMR_ST     = 0x01,
		SMR_TXINTM = 0x02,
		SMR_CL_SFT = 2,
		SMR_PEN    = 0x10,
		SMR_PEO    = 0x20,
		SMR_ERINTM = 0x40,
		SMR_RXINTM = 0x80,

		SCMR_TXEN  = 0x01,
		SCMR_DTR   = 0x02,
		SCMR_RXEN  = 0x04,
		SCMR_SBRK  = 0x08,
		SCMR_ERS   = 0x10,
		SCMR_RTS   = 0x20,

		SSR_TXRDY  = 0x01,
		SSR_RXRDY  = 0x02,
		SSR_TXE    = 0x04,
		SSR_PE     = 0x08,
		SSR_OE     = 0x10,
		SSR_FE     = 0x20,
		SSR_RBRK   = 0x40,
		SSR_DSR    = 0x80,

		SR_INT_ERR = 0,
		SR_INT_RX  = 1,
		SR_INT_TX  = 2,
		SR_INT_UNK = 3,

	};

	enum {
		SR_IDLE,
		SR_START,
		SR_BIT_0,
		SR_BIT_1,
		SR_BIT_2,
		SR_BIT_3,
		SR_BIT_4,
		SR_BIT_5,
		SR_BIT_6,
		SR_BIT_7,
		SR_PARITY,
		SR_STOP,
		SR_DONE,
	};

	u64 m_serial_tx_next_event[3], m_serial_rx_next_event[3];
	u64 m_serial_gclk[3];
	u32 m_serial_external_clock;
	u8 m_smr[3], m_scmr[3], m_sbrr[3], m_ssr[3], m_sdrr[3], m_sdrt[3], m_sr_intr[3], m_serial_rx[3], m_serial_tx[3];
	u8 m_serial_tx_state[3], m_serial_rx_state[3], m_serial_rx_line[3];
	u8 m_spr, m_scr;

	static u8 sbrr_to_div(u8 value);
	void serial_clock_update(int ch);
	u64 serial_get_next_edge(int ch);
	bool serial_validate_interrupt(u8 vector) const;
	void serial_check_interrupt(int ch);
	void serial_tx_update(int ch);
	void serial_rx_update(int ch);
	void serial_rx_w(int ch, int state);
	void ssr_set(int ch, u8 val, u8 mask);

	void smr_w(int ch, u8 data);
	void scmr_w(int ch, u8 data);
	void sbrr_w(int ch, u8 data);
	u8 sdr_r(int ch);
	void sdr_w(int ch, u8 data);

	u8 smr0_r();
	void smr0_w(u8 data);
	u8 smr1_r();
	void smr1_w(u8 data);
	u8 smr2_r();
	void smr2_w(u8 data);
	u8 scmr0_r();
	void scmr0_w(u8 data);
	u8 scmr1_r();
	void scmr1_w(u8 data);
	u8 scmr2_r();
	void scmr2_w(u8 data);
	u8 sbrr0_r();
	void sbrr0_w(u8 data);
	u8 sbrr1_r();
	void sbrr1_w(u8 data);
	u8 sbrr2_r();
	void sbrr2_w(u8 data);
	u8 ssr0_r();
	u8 ssr1_r();
	u8 ssr2_r();
	u8 sdr0_r();
	void sdr0_w(u8 data);
	u8 sdr1_r();
	void sdr1_w(u8 data);
	u8 sdr2_r();
	void sdr2_w(u8 data);
	u8 spr_r();
	void spr_w(u8 data);
	u8 scr_r();
	void scr_w(u8 data);

	// 16-bit timer
	enum {
		TCR_CK  = 14,
		TCR_P   = 10,
		TCR_T   = 8,
		TCR_N1  = 0x0080,
		TCR_RP  = 0x0040,
		TCR_MR  = 4,
		TCR_INT = 0x0004,
		TCR_CS  = 0x0002,
		TCR_TS  = 0x0001
	};

	u64 m_timer_next_event[3], m_timer_last_sync[3];
	u16 m_tcr[3], m_tmcr1[3], m_tmcr2[3], m_tctr[3];

	static const int timer_source_id[3][2];
	static const char *const timer_source_names[3][4];
	static const int timer_divider[16];

	void timer_update(int ch);
	void timer_sync(int ch);
	void timer_predict(int ch);

	void tcr_w(int ch, u16 data, u16 mem_mask);
	void tmcr1_w(int ch, u16 data, u16 mem_mask);
	void tmcr2_w(int ch, u16 data, u16 mem_mask);
	void tctr_w(int ch, u16 data, u16 mem_mask);

	void tcr0_w(offs_t, u16 data, u16 mem_mask);
	u16 tcr0_r();
	void tcr1_w(offs_t, u16 data, u16 mem_mask);
	u16 tcr1_r();
	void tcr2_w(offs_t, u16 data, u16 mem_mask);
	u16 tcr2_r();

	void tmcr01_w(offs_t, u16 data, u16 mem_mask);
	u16 tmcr01_r();

	void tmcr11_w(offs_t, u16 data, u16 mem_mask);
	u16 tmcr11_r();
	void tmcr12_w(offs_t, u16 data, u16 mem_mask);
	u16 tmcr12_r();

	void tmcr21_w(offs_t, u16 data, u16 mem_mask);
	u16 tmcr21_r();
	void tmcr22_w(offs_t, u16 data, u16 mem_mask);
	u16 tmcr22_r();

	void tctr0_w(offs_t, u16 data, u16 mem_mask);
	u16 tctr0_r();
	void tctr1_w(offs_t, u16 data, u16 mem_mask);
	u16 tctr1_r();
	void tctr2_w(offs_t, u16 data, u16 mem_mask);
	u16 tctr2_r();

};

class tmp68303_device : public tmp68301_device
{
public:
	tmp68303_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	virtual u8 base_timer_irq() const noexcept override { return 3; }
};

DECLARE_DEVICE_TYPE(TMP68301, tmp68301_device)
DECLARE_DEVICE_TYPE(TMP68303, tmp68303_device)


#endif
