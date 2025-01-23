// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_MCF5206E_H
#define MAME_CPU_M68000_MCF5206E_H

#pragma once

#include "m68kmusashi.h"
#include "machine/mc68681.h"
#include "machine/watchdog.h"

enum {
	EXTERNAL_IPL_1 = 1,
	EXTERNAL_IPL_2,
	EXTERNAL_IPL_3,
	EXTERNAL_IPL_4,
	EXTERNAL_IPL_5,
	EXTERNAL_IPL_6,
	EXTERNAL_IPL_7,
	WATCHDOG_IRQ,
	TIMER_1_IRQ,
	TIMER_2_IRQ,
	MBUS_IRQ,
	UART_1_IRQ,
	UART_2_IRQ,
	DMA_0_IRQ,
	DMA_1_IRQ,
	EXTERNAL_IRQ_1 = 1,
	EXTERNAL_IRQ_4 = 4,
	EXTERNAL_IRQ_7 = 7
};

class mcf5206e_device;	// Forward declaration

class coldfire_sim_device : public device_t {
	public:
		enum {
			ICR1 = 0,	// Bit 1
			ICR2,		// Bit 2
			ICR3,		// Bit 3
			ICR4,		// Bit 4
			ICR5,		// Bit 5
			ICR6,		// Bit 6
			ICR7,       // Bit 7
			ICR_SWDT,   // Bit 8
			ICR_TMR1,   // Bit 9
			ICR_TMR2,   // Bit 10
			ICR_MBUS,   // Bit 11
			ICR_UART1,  // Bit 12
			ICR_UART2,  // Bit 13
			ICR_DMA0,	// Bit 14
			ICR_DMA1,	// Bit 15
			MAX_ICR
		};

		template <typename T>
		coldfire_sim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
			: coldfire_sim_device(mconfig, tag, owner, clock)
		{
			m_maincpu.set_tag(std::forward<T>(cpu_tag));
		}

		coldfire_sim_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		void sim_map(address_map &map);

		void write_irq_1(int state) { set_external_interrupt(1, state); }
		void write_irq_4(int state) { set_external_interrupt(4, state); }
		void write_irq_7(int state) { set_external_interrupt(7, state); }

		void set_external_interrupt_level(int level){ set_external_interrupt(level, HOLD_LINE); };
		void set_external_interrupt(int level, int state);
		void set_internal_interrupt_request(int device, int state){ set_interrupt(device); }

		u16 get_par(){ return m_PAR; }

	protected:
		void device_start() override ATTR_COLD;
		void device_reset() override ATTR_COLD;
		void device_add_mconfig(machine_config &config) override ATTR_COLD;

	private:
		void set_interrupt(int interrupt);

		TIMER_CALLBACK_MEMBER(swdt_callback);
		
		void ICR_info(uint8_t ICR);

		uint8_t SIMR_r(){ return m_SIMR; }		// SIM Configuration Register
		uint8_t MARB_r(){ return m_MARB; }		// Bus Master Arbitration Control
		uint8_t ICR_r(offs_t offset);			// Interrupt Control Register
		uint16_t IMR_r(){ return m_IMR; }		// Interrupt Mask Register
		uint16_t IPR_r(){ return m_IPR; }		// Interrupt Pending Register
		uint8_t SYPCR_r(){ return m_SYPCR; }	// System Protection Control Register
		uint8_t SWIVR_r(){ return m_SWIVR; }	// Software Watchdog Interrupt Vector Register
		uint8_t RSR_r(){ return m_RSR; }		// Reset Status Register
		uint16_t PAR_r(){ return m_PAR; }		// Pin Assignment Register

		void SIMR_w(uint8_t data);							
		void MARB_w(uint8_t data);
		void ICR_w(offs_t offset, uint8_t data);
		void IMR_w(uint16_t data);	
		void SYPCR_w(uint8_t data);
		void SWIVR_w(uint8_t data);
		void SWSR_w(uint8_t data);				// Software Watchdog Service Routine (kick)
		void RSR_w(uint8_t data);
		void PAR_w(uint16_t data);							

		const uint8_t swdt_reset_sequence[2] = { 0x55, 0xAA };
		uint8_t m_swdt_w_count;
		bool m_sypcr_locked;

		uint8_t m_SIMR;
		uint8_t m_MARB;
		uint8_t m_ICR[15];
		uint16_t m_PAR;
		uint16_t m_IMR;
		uint16_t m_IPR;
		uint8_t m_SYPCR;
		uint8_t m_SWIVR;
		uint8_t m_RSR;

		uint8_t m_external_ipl;

		emu_timer *m_timer_swdt;
		
		required_device<mcf5206e_device> m_maincpu;
		required_device<watchdog_timer_device> m_swdt;
		
};

class coldfire_mbus_device : public device_t {
	public:
		coldfire_mbus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		void mbus_map(address_map &map) ATTR_COLD;

		void sda_write(u8 state){ m_tx_in = state; }
		auto sda_cb() ATTR_COLD { return write_sda.bind(); }
		auto scl_cb() ATTR_COLD { return write_scl.bind(); }
		auto irq_cb() ATTR_COLD { return write_irq.bind(); }

	protected:
		void device_start() override ATTR_COLD;
		void device_reset() override ATTR_COLD;

	private:
		// MBCR
		enum {
			RSTA = (1 << 2),
			TXAK = (1 << 3),
			MTX  = (1 << 4),
			MSTA = (1 << 5),
			MIEN = (1 << 6),
			MEN  = (1 << 7)
		};

		TIMER_CALLBACK_MEMBER(mbus_callback);

		inline uint8_t MADR_r(){ return m_MADR; }			// MADR is only for when the cpu is a slave I2C device
		uint8_t MBDR_r();
		inline uint8_t MBCR_r(){ return m_MBCR; }
		uint8_t MBSR_r();
		inline uint8_t MFDR_r(){ return m_MFDR; }

		void MADR_w(uint8_t data);
		void MBDR_w(uint8_t data);
		void MBCR_w(uint8_t data);
		void MBSR_w(uint8_t data);
		void MFDR_w(uint8_t data);

		uint8_t m_MADR;
		uint8_t m_MBCR;
		uint8_t m_MBSR;
		uint8_t m_MFDR;
		uint8_t m_MBDR;

		bool m_tx_in_progress;
		bool m_clk_state;
		u8 m_tx_bit;
		u8 m_tx_out;
		u8 m_tx_in;

		emu_timer *m_timer_mbus;
		devcb_write_line write_sda, write_scl, write_irq;
};

class coldfire_timer_device : public device_t {
	public:
		coldfire_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
		
		void timer_map(address_map &map) ATTR_COLD;
		auto irq_cb() { return m_timer_irq.bind(); }

	protected:
		void device_start() override ATTR_COLD;
		void device_reset() override ATTR_COLD;

	private:
		enum {
			T_RST = (1 << 0),
			T_CL0 = (1 << 1),
			T_CL1 = (1 << 2),
			T_FRR = (1 << 3),
			T_ORI = (1 << 4),
			T_OM  = (1 << 5),
			T_CE0 = (1 << 6),
			T_CE1 = (1 << 7),
			T_PS0 = (1 << 8),
			T_PS1 = (1 << 9),
			T_PS2 = (1 << 10),
			T_PS3 = (1 << 11),
			T_PS4 = (1 << 12),
			T_PS5 = (1 << 13),
			T_PS6 = (1 << 14),
			T_PS7 = (1 << 15)
		};

		enum {
			T_ECAP = (1 << 0),
			T_EREF = (1 << 1)
		};

		uint16_t TMR_r(){ return m_TMR; }
		uint16_t TRR_r(){ return m_TRR; }
		uint8_t TER_r(){ return m_TER; }
		uint16_t TCR_r(){ return m_TCR; }
		uint16_t TCN_r();

		void TMR_w(uint16_t data);
		void TRR_w(uint16_t data);
		void TER_w(uint8_t data);
		void TCN_w(uint16_t data);

		TIMER_CALLBACK_MEMBER(timer_callback);

		uint16_t m_TMR;
		uint16_t m_TRR;
		uint16_t m_TCR;
		uint16_t m_TCN;
		uint8_t m_TER;

		attotime m_timer_start_time;

		emu_timer *m_timer;
		devcb_write_line m_timer_irq;

		// Todo: Add output support
};

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
	auto gpio_w_cb() { return m_gpio_w_cb.bind(); }
	void gpio_pin_w(int pin, int state);
	void gpio_port_w(u8 state);
	
	// DUART
	auto tx1_w_cb() { return write_tx1.bind(); }
	auto tx2_w_cb() { return write_tx2.bind(); }
	void rx1_w(int state) { m_uart[0]->rx_a_w(state); }
	void rx2_w(int state) { m_uart[1]->rx_a_w(state); }

	// I2C / MBUS
	void sda_write(u8 state){ m_mbus->sda_write(state); }
	auto sda_w_cb() { return write_sda.bind(); }
	auto scl_w_cb() { return write_scl.bind(); }

	// IRQ/IPL - If using levels, recomended to use the 74148 device with this.
	void write_ipl(int state){ m_sim->set_external_interrupt_level(state); }
	void write_irq_1(int state){ m_sim->write_irq_1(state); }
	void write_irq_4(int state){ m_sim->write_irq_4(state); }
	void write_irq_7(int state){ m_sim->write_irq_7(state); }

protected:
	friend class coldfire_sim_device;

	virtual void device_start() override;
	virtual void device_reset() override;
	//virtual void device_clock_changed() override { }
	virtual void device_add_mconfig(machine_config &config) override;

	void coldfire_vector_map(address_map &map) ATTR_COLD;
	void coldfire_regs_map(address_map &map) ATTR_COLD;

	void init_regs(bool first_init);

	/* System Intergration Module */
	required_device<coldfire_sim_device> m_sim;

	/* TImer Modules */
	void timer_1_irq(int state){ m_sim->set_internal_interrupt_request(TIMER_1_IRQ, state); }
	void timer_2_irq(int state){ m_sim->set_internal_interrupt_request(TIMER_2_IRQ, state); }

	required_device_array<coldfire_timer_device, 2> m_timer;
	
	/* DRAM Controller Module */
	inline uint16_t DCRR_r(){ return m_DCRR; }
	inline uint16_t DCTR_r(){ return m_DCTR; }
	inline uint16_t DCAR0_r(){ return m_DCAR0; }
	inline uint32_t DCMR0_r(){ return m_DCMR0; }
	inline uint8_t DCCR0_r(){ return m_DCCR0; }
	inline uint16_t DCAR1_r(){ return m_DCAR1; }
	inline uint32_t DCMR1_r(){ return m_DCMR1; }
	inline uint8_t DCCR1_r(){ return m_DCCR1; }

	void DCRR_w(uint16_t data);
	void DCTR_w(uint16_t data);
	void DCAR0_w(uint16_t data);
	void DCMR0_w(uint32_t data);
	void DCCR0_w(uint8_t data);
	void DCAR1_w(uint16_t data);
	void DCMR1_w(uint32_t data);
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
	uint16_t CSCRx_r(offs_t offset);
	uint32_t CSMRx_r(offs_t offset);
	inline uint16_t CSAR0_r(){ return CSARx_r(0); }
	inline uint32_t CSMR0_r(){ return CSMRx_r(0); }
	inline uint16_t CSCR0_r(){ return CSCRx_r(0); }
	inline uint16_t CSAR1_r(){ return CSARx_r(1); }
	inline uint32_t CSMR1_r(){ return CSMRx_r(1); }
	inline uint16_t CSCR1_r(){ return CSCRx_r(1); }
	inline uint16_t CSAR2_r(){ return CSARx_r(2); }
	inline uint32_t CSMR2_r(){ return CSMRx_r(2); }
	inline uint16_t CSCR2_r(){ return CSCRx_r(2); }
	inline uint16_t CSAR3_r(){ return CSARx_r(3); }
	inline uint32_t CSMR3_r(){ return CSMRx_r(3); }
	inline uint16_t CSCR3_r(){ return CSCRx_r(3); }
	inline uint16_t CSAR4_r(){ return CSARx_r(4); }
	inline uint32_t CSMR4_r(){ return CSMRx_r(4); }
	inline uint16_t CSCR4_r(){ return CSCRx_r(4); }
	inline uint16_t CSAR5_r(){ return CSARx_r(5); }
	inline uint32_t CSMR5_r(){ return CSMRx_r(5); }
	inline uint16_t CSCR5_r(){ return CSCRx_r(5); }
	inline uint16_t CSAR6_r(){ return CSARx_r(6); }
	inline uint32_t CSMR6_r(){ return CSMRx_r(6); }
	inline uint16_t CSCR6_r(){ return CSCRx_r(6); }
	inline uint16_t CSAR7_r(){ return CSARx_r(7); }
	inline uint32_t CSMR7_r(){ return CSMRx_r(7); }
	inline uint16_t CSCR7_r(){ return CSCRx_r(7); }
	inline uint16_t DMCR_r(){ return m_DMCR; }

	void CSARx_w(offs_t offset, uint16_t data);
	void CSMRx_w(offs_t offset, uint32_t data);
	void CSCRx_w(offs_t offset, uint16_t data);
	inline void CSAR0_w(uint16_t data){ CSARx_w(0, data); }
	inline void CSMR0_w(uint32_t data){ CSMRx_w(0, data); }
	inline void CSCR0_w(uint16_t data){ CSCRx_w(0, data); }
	inline void CSAR1_w(uint16_t data){ CSARx_w(1, data); }
	inline void CSMR1_w(uint32_t data){ CSMRx_w(1, data); }
	inline void CSCR1_w(uint16_t data){ CSCRx_w(1, data); }
	inline void CSAR2_w(uint16_t data){ CSARx_w(2, data); }
	inline void CSMR2_w(uint32_t data){ CSMRx_w(2, data); }
	inline void CSCR2_w(uint16_t data){ CSCRx_w(2, data); }
	inline void CSAR3_w(uint16_t data){ CSARx_w(3, data); }
	inline void CSMR3_w(uint32_t data){ CSMRx_w(3, data); }
	inline void CSCR3_w(uint16_t data){ CSCRx_w(3, data); }
	inline void CSAR4_w(uint16_t data){ CSARx_w(4, data); }
	inline void CSMR4_w(uint32_t data){ CSMRx_w(4, data); }
	inline void CSCR4_w(uint16_t data){ CSCRx_w(4, data); }
	inline void CSAR5_w(uint16_t data){ CSARx_w(5, data); }
	inline void CSMR5_w(uint32_t data){ CSMRx_w(5, data); }
	inline void CSCR5_w(uint16_t data){ CSCRx_w(5, data); }
	inline void CSAR6_w(uint16_t data){ CSARx_w(6, data); }
	inline void CSMR6_w(uint32_t data){ CSMRx_w(6, data); }
	inline void CSCR6_w(uint16_t data){ CSCRx_w(6, data); }
	inline void CSAR7_w(uint16_t data){ CSARx_w(7, data); }
	inline void CSMR7_w(uint32_t data){ CSMRx_w(7, data); }
	inline void CSCR7_w(uint16_t data){ CSCRx_w(7, data); }
	void DMCR_w(uint16_t data);
	
	uint16_t m_CSAR[8];
	uint32_t m_CSMR[8];
	uint16_t m_CSCR[8];
	uint16_t m_DMCR;

	devcb_write_line::array<8> write_chip_select;

	/* UART Modules */ 
	// Stick two 68681 A channels in the CPU with no timers.
	uint8_t uart1_r(offs_t offset);
	uint8_t uart2_r(offs_t offset);
	void uart1_w(offs_t offset, uint8_t data);
	void uart2_w(offs_t offset, uint8_t data);
	void uart_1_irq(int state){ m_sim->set_external_interrupt(UART_1_IRQ, state); }
	void uart_2_irq(int state){ m_sim->set_external_interrupt(UART_2_IRQ, state); }

	required_device_array<mcf5206e_uart_device, 2> m_uart;
	devcb_write_line write_tx1, write_tx2;

	/* Parallel Port */
	// Just an 8 bit GPIO port
	inline uint8_t PPDDR_r(){ return m_PPDDR; }
	inline uint8_t PPDAT_r(){ return (m_PPDATI | (m_PPDATO & m_PPDDR)); }
	
	void PPDDR_w(uint8_t data);
	void PPDAT_w(uint8_t data);

	uint8_t m_PPDDR;
	uint8_t m_PPDATI;
	uint8_t m_PPDATO;
	devcb_write8 m_gpio_w_cb;

	/* MBUS Module */
	// I2C host/device but Motorola
	void mbus_sda_w(int state){ write_sda(state); }
	void mbus_scl_w(int state){ write_scl(state); }
	void mbus_irq_w(int state){ m_sim->set_external_interrupt(MBUS_IRQ, state); }
	required_device<coldfire_mbus_device> m_mbus;
	devcb_write_line write_sda, write_scl;

};

DECLARE_DEVICE_TYPE(MCF5206E, mcf5206e_device)
DECLARE_DEVICE_TYPE(COLDFIRE_SIM, coldfire_sim_device)
DECLARE_DEVICE_TYPE(COLDFIRE_TIMER, coldfire_timer_device)
DECLARE_DEVICE_TYPE(COLDFIRE_MBUS, coldfire_mbus_device)

#endif
