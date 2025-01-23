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

		u16 get_par(){ return m_par; }

	protected:
		void device_start() override ATTR_COLD;
		void device_reset() override ATTR_COLD;
		void device_add_mconfig(machine_config &config) override ATTR_COLD;

	private:
		void set_interrupt(int interrupt);

		TIMER_CALLBACK_MEMBER(swdt_callback);
		
		void icr_info(uint8_t icr);

		uint8_t simr_r(){ return m_simr; }		// sim configuration register
		uint8_t marb_r(){ return m_marb; }		// bus master arbitration control
		uint8_t icr_r(offs_t offset);			// interrupt control register
		uint16_t imr_r(){ return m_imr; }		// interrupt mask register
		uint16_t ipr_r(){ return m_ipr; }		// interrupt pending register
		uint8_t sypcr_r(){ return m_sypcr; }	// system protection control register
		uint8_t swivr_r(){ return m_swivr; }	// software watchdog interrupt vector register
		uint8_t rsr_r(){ return m_rsr; }		// reset status register
		uint16_t par_r(){ return m_par; }		// pin assignment register

		void simr_w(uint8_t data);							
		void marb_w(uint8_t data);
		void icr_w(offs_t offset, uint8_t data);
		void imr_w(uint16_t data);	
		void sypcr_w(uint8_t data);
		void swivr_w(uint8_t data);
		void swsr_w(uint8_t data);				// software watchdog service routine (kick)
		void rsr_w(uint8_t data);
		void par_w(uint16_t data);							

		const uint8_t swdt_reset_sequence[2] = { 0x55, 0xaa };
		uint8_t m_swdt_w_count;
		bool m_sypcr_locked;

		uint8_t m_simr;
		uint8_t m_marb;
		uint8_t m_icr[15];
		uint16_t m_par;
		uint16_t m_imr;
		uint16_t m_ipr;
		uint8_t m_sypcr;
		uint8_t m_swivr;
		uint8_t m_rsr;

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

		inline uint8_t madr_r(){ return m_madr; }			// madr is only for when the cpu is a slave i2c device
		uint8_t mbdr_r();
		inline uint8_t mbcr_r(){ return m_mbcr; }
		uint8_t mbsr_r();
		inline uint8_t mfdr_r(){ return m_mfdr; }

		void madr_w(uint8_t data);
		void mbdr_w(uint8_t data);
		void mbcr_w(uint8_t data);
		void mbsr_w(uint8_t data);
		void mfdr_w(uint8_t data);

		uint8_t m_madr;
		uint8_t m_mbcr;
		uint8_t m_mbsr;
		uint8_t m_mfdr;
		uint8_t m_mbdr;

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

		uint16_t tmr_r(){ return m_tmr; }
		uint16_t trr_r(){ return m_trr; }
		uint8_t ter_r(){ return m_ter; }
		uint16_t tcr_r(){ return m_tcr; }
		uint16_t tcn_r();

		void tmr_w(uint16_t data);
		void trr_w(uint16_t data);
		void ter_w(uint8_t data);
		void tcn_w(uint16_t data);

		TIMER_CALLBACK_MEMBER(timer_callback);

		uint16_t m_tmr;
		uint16_t m_trr;
		uint16_t m_tcr;
		uint16_t m_tcn;
		uint8_t m_ter;

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
	
	/* dram controller module */
	inline uint16_t dcrr_r(){ return m_dcrr; }
	inline uint16_t dctr_r(){ return m_dctr; }
	inline uint16_t dcar0_r(){ return m_dcar0; }
	inline uint32_t dcmr0_r(){ return m_dcmr0; }
	inline uint8_t dccr0_r(){ return m_dccr0; }
	inline uint16_t dcar1_r(){ return m_dcar1; }
	inline uint32_t dcmr1_r(){ return m_dcmr1; }
	inline uint8_t dccr1_r(){ return m_dccr1; }

	void dcrr_w(uint16_t data);
	void dctr_w(uint16_t data);
	void dcar0_w(uint16_t data);
	void dcmr0_w(uint32_t data);
	void dccr0_w(uint8_t data);
	void dcar1_w(uint16_t data);
	void dcmr1_w(uint32_t data);
	void dccr1_w(uint8_t data);

	uint16_t m_dcrr;
	uint16_t m_dctr;
	uint16_t m_dcar0;
	uint32_t m_dcmr0;
	uint8_t  m_dccr0;
	uint16_t m_dcar1;
	uint32_t m_dcmr1;
	uint8_t  m_dccr1;

	/* chip select module */
	uint16_t csar_x_r(offs_t offset);
	uint16_t cscr_x_r(offs_t offset);
	uint32_t csmr_x_r(offs_t offset);
	inline uint16_t csar0_r(){ return csar_x_r(0); }
	inline uint32_t csmr0_r(){ return csmr_x_r(0); }
	inline uint16_t cscr0_r(){ return cscr_x_r(0); }
	inline uint16_t csar1_r(){ return csar_x_r(1); }
	inline uint32_t csmr1_r(){ return csmr_x_r(1); }
	inline uint16_t cscr1_r(){ return cscr_x_r(1); }
	inline uint16_t csar2_r(){ return csar_x_r(2); }
	inline uint32_t csmr2_r(){ return csmr_x_r(2); }
	inline uint16_t cscr2_r(){ return cscr_x_r(2); }
	inline uint16_t csar3_r(){ return csar_x_r(3); }
	inline uint32_t csmr3_r(){ return csmr_x_r(3); }
	inline uint16_t cscr3_r(){ return cscr_x_r(3); }
	inline uint16_t csar4_r(){ return csar_x_r(4); }
	inline uint32_t csmr4_r(){ return csmr_x_r(4); }
	inline uint16_t cscr4_r(){ return cscr_x_r(4); }
	inline uint16_t csar5_r(){ return csar_x_r(5); }
	inline uint32_t csmr5_r(){ return csmr_x_r(5); }
	inline uint16_t cscr5_r(){ return cscr_x_r(5); }
	inline uint16_t csar6_r(){ return csar_x_r(6); }
	inline uint32_t csmr6_r(){ return csmr_x_r(6); }
	inline uint16_t cscr6_r(){ return cscr_x_r(6); }
	inline uint16_t csar7_r(){ return csar_x_r(7); }
	inline uint32_t csmr7_r(){ return csmr_x_r(7); }
	inline uint16_t cscr7_r(){ return cscr_x_r(7); }
	inline uint16_t dmcr_r(){ return m_dmcr; }

	void csar_x_w(offs_t offset, uint16_t data);
	void csmr_x_w(offs_t offset, uint32_t data);
	void cscr_x_w(offs_t offset, uint16_t data);
	inline void csar0_w(uint16_t data){ csar_x_w(0, data); }
	inline void csmr0_w(uint32_t data){ csmr_x_w(0, data); }
	inline void cscr0_w(uint16_t data){ cscr_x_w(0, data); }
	inline void csar1_w(uint16_t data){ csar_x_w(1, data); }
	inline void csmr1_w(uint32_t data){ csmr_x_w(1, data); }
	inline void cscr1_w(uint16_t data){ cscr_x_w(1, data); }
	inline void csar2_w(uint16_t data){ csar_x_w(2, data); }
	inline void csmr2_w(uint32_t data){ csmr_x_w(2, data); }
	inline void cscr2_w(uint16_t data){ cscr_x_w(2, data); }
	inline void csar3_w(uint16_t data){ csar_x_w(3, data); }
	inline void csmr3_w(uint32_t data){ csmr_x_w(3, data); }
	inline void cscr3_w(uint16_t data){ cscr_x_w(3, data); }
	inline void csar4_w(uint16_t data){ csar_x_w(4, data); }
	inline void csmr4_w(uint32_t data){ csmr_x_w(4, data); }
	inline void cscr4_w(uint16_t data){ cscr_x_w(4, data); }
	inline void csar5_w(uint16_t data){ csar_x_w(5, data); }
	inline void csmr5_w(uint32_t data){ csmr_x_w(5, data); }
	inline void cscr5_w(uint16_t data){ cscr_x_w(5, data); }
	inline void csar6_w(uint16_t data){ csar_x_w(6, data); }
	inline void csmr6_w(uint32_t data){ csmr_x_w(6, data); }
	inline void cscr6_w(uint16_t data){ cscr_x_w(6, data); }
	inline void csar7_w(uint16_t data){ csar_x_w(7, data); }
	inline void csmr7_w(uint32_t data){ csmr_x_w(7, data); }
	inline void cscr7_w(uint16_t data){ cscr_x_w(7, data); }
	void dmcr_w(uint16_t data);
	
	uint16_t m_csar[8];
	uint32_t m_csmr[8];
	uint16_t m_cscr[8];
	uint16_t m_dmcr;

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

	/* parallel port */
	// just an 8 bit gpio port
	inline uint8_t ppddr_r(){ return m_ppddr; }
	inline uint8_t ppdat_r(){ return (m_ppdat_in | (m_ppdat_out & m_ppddr)); }
	
	void ppddr_w(uint8_t data);
	void ppdat_w(uint8_t data);

	uint8_t m_ppddr;
	uint8_t m_ppdat_in;
	uint8_t m_ppdat_out;
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
