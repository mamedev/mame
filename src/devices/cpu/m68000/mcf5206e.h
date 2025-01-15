// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_MCF5206E_H
#define MAME_CPU_M68000_MCF5206E_H

#pragma once

#include "m68kmusashi.h"
#include "machine/mc68681.h"
#include "machine/watchdog.h"

class mcf5206e_device : public m68000_musashi_device
{
public:
	// construction/destruction
	mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// Chip Select Module
	template<unsigned N> auto chip_select_w_cb() { return write_chip_select[N].bind(); }

	// Parallel Port
	auto gpio_r_cb() { return m_gpio_r_cb.bind(); }
	auto gpio_w_cb() { return m_gpio_w_cb.bind(); }
	
	// DUART
	auto irq_w_cb() { return write_irq.bind(); }
	auto tx1_w_cb() { return write_tx1.bind(); }
	auto tx2_w_cb() { return write_tx2.bind(); }
	void rx1_w(int state) { m_uart[0]->rx_a_w(state); }
	void rx2_w(int state) { m_uart[1]->rx_a_w(state); }

	// I2C / MBUS
	auto sda_w_cb() { return write_sda.bind(); }
	auto scl_w_cb() { return write_scl.bind(); }

	virtual void device_start() override;
	virtual void device_reset() override;
	//virtual void device_clock_changed() override { }
	virtual void device_add_mconfig(machine_config &config) override;

protected:
	// Interrupt control enables
	typedef enum {
		EINT1 = 1,
		EINT2,
		EINT3,
		EINT4,
		EINT5,
		EINT6,
		EINT7,
		WDINT,
		TINT1,
		TINT2,
		MBINT,
		UINT1,
		UINT2,
		DINT0,
		DINT1
	} IntSrc;

	void coldfire_regs_map(address_map &map);

	TIMER_CALLBACK_MEMBER(timer1_callback);
	TIMER_CALLBACK_MEMBER(timer2_callback);
	TIMER_CALLBACK_MEMBER(mbus_callback);
	TIMER_CALLBACK_MEMBER(swdt_callback);

	void init_regs(bool first_init);

	void ICR_info(uint8_t ICR);

	/* SIM Module */
	uint8_t SIMR_r();													// SIM Configuration Register
	void SIMR_w(uint8_t data);							
	uint8_t MARB_r();													// Bus Master Arbitration Control
	void MARB_w(uint8_t data);
	uint16_t IMR_r();													// Interrupt Mask Register
	void IMR_w(uint16_t data);	
	uint16_t IPR_r();													// Interrupt Pending Register
	uint8_t SYPCR_r();													// System Protection Control Register
	void SYPCR_w(uint8_t data);
	uint8_t SWIVR_r();													// Software Watchdog Interrupt Vector Register
	void SWIVR_w(uint8_t data);
	void SWSR_w(uint8_t data);											// Software Watchdog Service Routine (kick)
	uint8_t RSR_r();													// Reset Status Register
	void RSR_w(uint8_t data);

	uint8_t ICRx_r(offs_t offset);										// Interrupt Control Register
	void ICRx_w(offs_t offset, uint8_t data);

	uint16_t PAR_r();
	void PAR_w(uint16_t data);							

	uint8_t ICR1_r(){ return ICRx_r(1); }
	uint8_t ICR2_r(){ return ICRx_r(2); }
	uint8_t ICR3_r(){ return ICRx_r(3); }
	uint8_t ICR4_r(){ return ICRx_r(4); }
	uint8_t ICR5_r(){ return ICRx_r(5); }
	uint8_t ICR6_r(){ return ICRx_r(6); }
	uint8_t ICR7_r(){ return ICRx_r(7); }
	uint8_t ICR8_r(){ return ICRx_r(8); }
	uint8_t ICR9_r(){ return ICRx_r(9); }
	uint8_t ICR10_r(){ return ICRx_r(10); }
	uint8_t ICR11_r(){ return ICRx_r(11); }
	uint8_t ICR12_r(){ return ICRx_r(12); }
	uint8_t ICR13_r(){ return ICRx_r(13); }
	uint8_t ICR14_r(){ return ICRx_r(14); }
	uint8_t ICR15_r(){ return ICRx_r(15); }

	// ICR 1-8 are fixed interrupt levels
	void ICR1_w(uint8_t data) { ICRx_w(1, (data & 0x83) + (1 << 2)); }
	void ICR2_w(uint8_t data) { ICRx_w(2, (data & 0x83) + (2 << 2)); }
	void ICR3_w(uint8_t data) { ICRx_w(3, (data & 0x83) + (3 << 2)); }
	void ICR4_w(uint8_t data) { ICRx_w(4, (data & 0x83) + (4 << 2)); }
	void ICR5_w(uint8_t data) { ICRx_w(5, (data & 0x83) + (5 << 2)); }
	void ICR6_w(uint8_t data) { ICRx_w(6, (data & 0x83) + (6 << 2)); }
	void ICR7_w(uint8_t data) { ICRx_w(7, (data & 0x83) + (7 << 2)); }
	void ICR8_w(uint8_t data) { ICRx_w(8, (data & 0x03) + (7 << 2)); }	// IPL7 and SWDT share same level, also you cannot use autovector on SWT.
	void ICR9_w(uint8_t data) { ICRx_w(9, (data & 0x1F) + 0x80 ); }		// Timer 1 *must* use autovector
	void ICR10_w(uint8_t data) { ICRx_w(10, (data & 0x1F) + 0x80 ); }	// Timer 2 *must* use autovector
	void ICR11_w(uint8_t data) { ICRx_w(11, (data & 0x1F) + 0x80 ); }	// MBUS *must* use autovector
	void ICR12_w(uint8_t data) { ICRx_w(12, (data & 0x9F)); }
	void ICR13_w(uint8_t data) { ICRx_w(13, (data & 0x9F)); }
	void ICR14_w(uint8_t data) { ICRx_w(14, (data & 0x9F)); }
	void ICR15_w(uint8_t data) { ICRx_w(15, (data & 0x9F)); }

	void set_interrupt(IntSrc interrupt);

	const uint8_t swdt_reset_sequence[2] = { 0x55, 0xAA };
	uint8_t m_swdt_w_count;
	bool m_sypcr_locked;

	uint8_t m_SIMR;
	uint8_t m_MARB;
	uint8_t m_ICR[16];
	uint16_t m_PAR;
	uint16_t m_IMR;
	uint16_t m_IPR;
	uint8_t m_SYPCR;
	uint8_t m_SWIVR;
	uint8_t m_RSR;

	/* DRAM Controller Module */
	uint16_t DCRR_r();
	void DCRR_w(uint16_t data);
	uint16_t DCTR_r();
	void DCTR_w(uint16_t data);
	uint16_t DCAR0_r();
	void DCAR0_w(uint16_t data);
	uint32_t DCMR0_r();
	void DCMR0_w(uint32_t data);
	uint8_t DCCR0_r();
	void DCCR0_w(uint8_t data);
	uint16_t DCAR1_r();
	void DCAR1_w(uint16_t data);
	uint32_t DCMR1_r();
	void DCMR1_w(uint32_t data);
	uint8_t DCCR1_r();
	void DCCR1_w(uint8_t data);

	uint16_t m_DCRR;
	uint16_t m_DCTR;
	uint16_t m_DCAR0;
	uint32_t m_DCMR0;
	uint8_t  m_DCCR0;
	uint16_t m_DCAR1;
	uint32_t m_DCMR1;
	uint8_t  m_DCCR1;


	/* Chip Select Module */
	uint16_t CSARx_r(offs_t offset);
	void CSARx_w(offs_t offset, uint16_t data);
	uint32_t CSMRx_r(offs_t offset);
	void CSMRx_w(offs_t offset, uint32_t data);
	uint16_t CSCRx_r(offs_t offset);
	void CSCRx_w(offs_t offset, uint16_t data);

	uint16_t CSAR0_r(){ return CSARx_r(0); }
	void CSAR0_w(uint16_t data){ CSARx_w(0, data); }
	uint32_t CSMR0_r(){ return CSMRx_r(0); }
	void CSMR0_w(uint32_t data){ CSMRx_w(0, data); }
	uint16_t CSCR0_r(){ return CSCRx_r(0); }
	void CSCR0_w(uint16_t data){ CSCRx_w(0, data); }
	uint16_t CSAR1_r(){ return CSARx_r(1); }
	void CSAR1_w(uint16_t data){ CSARx_w(1, data); }
	uint32_t CSMR1_r(){ return CSMRx_r(1); }
	void CSMR1_w(uint32_t data){ CSMRx_w(1, data); }
	uint16_t CSCR1_r(){ return CSCRx_r(1); }
	void CSCR1_w(uint16_t data){ CSCRx_w(1, data); }
	uint16_t CSAR2_r(){ return CSARx_r(2); }
	void CSAR2_w(uint16_t data){ CSARx_w(2, data); }
	uint32_t CSMR2_r(){ return CSMRx_r(2); }
	void CSMR2_w(uint32_t data){ CSMRx_w(2, data); }
	uint16_t CSCR2_r(){ return CSCRx_r(2); }
	void CSCR2_w(uint16_t data){ CSCRx_w(2, data); }
	uint16_t CSAR3_r(){ return CSARx_r(3); }
	void CSAR3_w(uint16_t data){ CSARx_w(3, data); }
	uint32_t CSMR3_r(){ return CSMRx_r(3); }
	void CSMR3_w(uint32_t data){ CSMRx_w(3, data); }
	uint16_t CSCR3_r(){ return CSCRx_r(3); }
	void CSCR3_w(uint16_t data){ CSCRx_w(3, data); }
	uint16_t CSAR4_r(){ return CSARx_r(4); }
	void CSAR4_w(uint16_t data){ CSARx_w(4, data); }
	uint32_t CSMR4_r(){ return CSMRx_r(4); }
	void CSMR4_w(uint32_t data){ CSMRx_w(4, data); }
	uint16_t CSCR4_r(){ return CSCRx_r(4); }
	void CSCR4_w(uint16_t data){ CSCRx_w(4, data); }
	uint16_t CSAR5_r(){ return CSARx_r(5); }
	void CSAR5_w(uint16_t data){ CSARx_w(5, data); }
	uint32_t CSMR5_r(){ return CSMRx_r(5); }
	void CSMR5_w(uint32_t data){ CSMRx_w(5, data); }
	uint16_t CSCR5_r(){ return CSCRx_r(5); }
	void CSCR5_w(uint16_t data){ CSCRx_w(5, data); }
	uint16_t CSAR6_r(){ return CSARx_r(6); }
	void CSAR6_w(uint16_t data){ CSARx_w(6, data); }
	uint32_t CSMR6_r(){ return CSMRx_r(6); }
	void CSMR6_w(uint32_t data){ CSMRx_w(6, data); }
	uint16_t CSCR6_r(){ return CSCRx_r(6); }
	void CSCR6_w(uint16_t data){ CSCRx_w(6, data); }
	uint16_t CSAR7_r(){ return CSARx_r(7); }
	void CSAR7_w(uint16_t data){ CSARx_w(7, data); }
	uint32_t CSMR7_r(){ return CSMRx_r(7); }
	void CSMR7_w(uint32_t data){ CSMRx_w(7, data); }
	uint16_t CSCR7_r(){ return CSCRx_r(7); }
	void CSCR7_w(uint16_t data){ CSCRx_w(7, data); }
	
	uint16_t DMCR_r();
	void DMCR_w(uint16_t data);

	uint16_t m_CSAR[8];
	uint32_t m_CSMR[8];
	uint16_t m_CSCR[8];

	uint16_t m_DMCR;

	/* UART Modules */ // Stick two 68681 A channels in the CPU with no timers.
	uint8_t uart1_r(offs_t offset);
	uint8_t uart2_r(offs_t offset);
	void uart1_w(offs_t offset, uint8_t data);
	void uart2_w(offs_t offset, uint8_t data);
	void uart1_irq_w(int state);
	void uart2_irq_w(int state);

	emu_timer *m_timer_swdt;

	/* Timer Modules */
	/* Timer 1 */
	uint16_t TMR1_r();
	void TMR1_w(uint16_t data);
	uint16_t TRR1_r();
	void TRR1_w(uint16_t data);
	uint8_t TER1_r();
	void TER1_w(uint8_t data);
	uint16_t TCR1_r();
	uint16_t TCN1_r();
	void TCN1_w(uint16_t data);

	emu_timer *m_timer1;
	uint16_t m_TMR1;
	uint16_t m_TRR1;
	uint16_t m_TCR1;
	uint16_t m_TCN1;
	uint8_t m_TER1;

	/* Timer 2 */
	uint16_t TMR2_r();
	void TMR2_w(uint16_t data);
	uint16_t TRR2_r();
	void TRR2_w(uint16_t data);
	uint8_t TER2_r();
	void TER2_w(uint8_t data);
	uint16_t TCR2_r();
	uint16_t TCN2_r();
	void TCN2_w(uint16_t data);

	emu_timer *m_timer2;
	uint16_t m_TMR2;
	uint16_t m_TRR2;
	uint16_t m_TCR2;
	uint16_t m_TCN2;
	uint8_t m_TER2;

	/* Parallel Port */
	uint8_t PPDDR_r();
	void PPDDR_w(uint8_t data);
	uint8_t PPDAT_r();
	void PPDAT_w(uint8_t data);

	uint8_t m_PPDDR;
	uint8_t m_PPDATI;
	uint8_t m_PPDATO;

	/* MBUS Module */
	uint8_t MADR_r();			// MADR is only for when the cpu is a slave I2C device
	void MADR_w(uint8_t data);
	uint8_t MBCR_r();
	void MBCR_w(uint8_t data);
	uint8_t MBSR_r();
	void MBSR_w(uint8_t data);
	uint8_t MFDR_r();
	void MFDR_w(uint8_t data);
	uint8_t MBDR_r();
	void MBDR_w(uint8_t data);

	uint8_t m_MADR;
	uint8_t m_MBCR;
	uint8_t m_MBSR;
	uint8_t m_MFDR;
	uint8_t m_MBDR;

	emu_timer *m_timer_mbus;


	required_device_array<mcf5206e_uart_device, 2> m_uart;
	required_device<watchdog_timer_device> m_swdt;
	devcb_write_line::array<8> write_chip_select;

	devcb_read8 m_gpio_r_cb;
	devcb_write8 m_gpio_w_cb;

	devcb_write_line write_irq, write_tx1, write_tx2, write_sda, write_scl;

	uint32_t m_coldfire_regs[0x400/4];

};

DECLARE_DEVICE_TYPE(MCF5206E, mcf5206e_device)

#endif
