// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * ym3802.h - Yamaha YM3802/YM3523 MCS MIDI Communication and Service Controller
 *
 * CLK input - anywhere from 1MHz up to 4MHz
 * CLKM input - usually either 1MHz or 0.5MHz, or CLKF input - usually 614.4kHz
 *
 * Registers:
 *     reg0 : IVR (read-only)
 *     reg1 : RGR (bit 8 = reset, bits 0-3 = register bank select)
 *     reg2 : ISR (read-only)
 *     reg3 : ICR (write-only)
 *     reg4-reg7  banked registers
 *       reg4 : IOR  DMR  RRR  RSR  TRR  TSR  FSR  SRR     GTR(L)  EDR
 *       reg5 : IMR  DCR  RMR  RCR  TMR  TCR  FCR  SCR     GTR(H)  ---
 *       reg6 : IER  DSR  AMR  RDR  ---  TDR  CCR  SPR(L)  MTR(L)  EOR/EIR
 *       reg7 : ---  DNR  ADR  ---  ---  ---  CDR  SPR(H)  MTR(H)  ---
 */
#ifndef MAME_MACHINE_YM3802_H
#define MAME_MACHINE_YM3802_H

#pragma once

#include "diserial.h"

#include <queue>

class ym3802_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	ym3802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }
	auto txd_handler() { return m_txd_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t vector() { return m_vector; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum
	{
		REG_IVR = 0,   // Interrupt Vector (read only)
		REG_RGR,       // Register Group / System Control
		REG_ISR,       // Interrupt Service (read only)
		REG_ICR,       // Interrupt Clear (write only)

		REG_IOR,       // Interrupt Vector Offset Request
		REG_IMR,       // Interrupt Mode Control
		REG_IER,       // Interrupt Enable Request
		REG_UNUSED1,

		REG_DMR = 14,  // Real Time Message Control
		REG_DCR,       // Real Time Message Request
		REG_DSR,       // FIFO IRx Data
		REG_DNR,       // FIFO IRx Control

		REG_RRR = 24,  // Rx Rate
		REG_RMR,       // Rx Mode
		REG_AMR,       // Address Hunter Maker
		REG_ADR,       // Address Hunter Device

		REG_RSR = 34,  // FIFO Rx Buffer Status
		REG_RCR,       // FIFO Rx Buffer Control
		REG_RDR,       // FIFO Rx Data
		REG_UNUSED2,

		REG_TRR = 44,  // Tx Rate
		REG_TMR,       // Tx Mode
		REG_UNUSED3,
		REG_UNUSED4,

		REG_TSR = 54,  // FIFO Tx Status
		REG_TCR,       // FIFO Tx Control
		REG_TDR,       // FIFO Tx Data
		REG_UNUSED5,

		REG_FSR = 64,  // FSK status
		REG_FCR,       // FSK control
		REG_CCR,       // Click Counter Control
		REG_CDR,       // Click Counter Data (7-bit)

		REG_SRR = 74,  // Recording Counter current value
		REG_SCR,       // Sequencer Control
		REG_SPR_LOW,   // Playback Counter (low 8-bits)
		REG_SPR_HIGH,  // Playback Counter (high 7-bits)

		REG_GTR_LOW = 84,  // General Timer (low 8-bits)
		REG_GTR_HIGH,      // General Timer (high 6-bits)
		REG_MTR_LOW,       // MIDI Clock Timer (low 8-bits)
		REG_MTR_HIGH,      // MIDI Clock Timer (high 6-bits)

		REG_EDR = 94,  // External I/O Direction
		REG_EOR,       // External I/O Output Data
		REG_EIR,       // External I/O Input Data
		REG_UNUSED7,

		REG_MAX = 100
	};

	enum
	{
		IRQ_MIDI_MSG = 0x01,
		IRQ_CLICK = 0x02,
		IRQ_MIDI_CLK = 0x02,
		IRQ_PLAYBACK_COUNT = 0x04,
		IRQ_RECORDING_COUNT = 0x08,
		IRQ_OFFLINE = 0x10,
		IRQ_BREAK = 0x10,
		IRQ_FIFORX_RDY = 0x20,
		IRQ_FIFOTX_EMPTY = 0x40,
		IRQ_GENERAL_TIMER = 0x80
	};

	TIMER_CALLBACK_MEMBER(transmit_clk);
	TIMER_CALLBACK_MEMBER(midi_clk);
	void reset_midi_timer();
	void set_comms_mode();
	void set_irq(uint8_t irq);
	void reset_irq(uint8_t irq);

	devcb_write_line m_irq_handler;
	devcb_write_line m_txd_handler;
	devcb_read_line m_rxd_handler;
	emu_timer* m_midi_timer;
	emu_timer* m_midi_counter_timer;

	std::vector<uint8_t> m_reg;
	uint8_t m_wdr;
	uint64_t m_prev_rate;
	uint8_t m_irq_status;
	uint16_t m_general_counter;
	uint16_t m_midi_counter;
	uint16_t m_midi_counter_base;
	//uint8_t m_midi_counter_divider;
	uint8_t m_click_counter;
	uint8_t m_click_counter_base;
	uint8_t m_vector;

	std::queue<uint8_t> m_tx_fifo;
	std::queue<uint8_t> m_rx_fifo;
	std::queue<uint8_t> m_itx_fifo;
	std::queue<uint8_t> m_irx_fifo;
	bool m_tx_busy;

	uint64_t m_clkm_rate;
	uint64_t m_clkf_rate;
};

DECLARE_DEVICE_TYPE(YM3802, ym3802_device)

#endif // MAME_MACHINE_YM3802_H
