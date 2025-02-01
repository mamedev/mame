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
	friend class mcf5206e_device; 

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
		coldfire_sim_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
			: coldfire_sim_device(mconfig, tag, owner, clock)
		{
			m_maincpu.set_tag(std::forward<T>(cpu_tag));
		}

		coldfire_sim_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		void sim_map(address_map &map);

		u8 interrupt_callback(offs_t int_level);

		void write_irq_1(int state) { set_external_interrupt(1, state); }
		void write_irq_4(int state) { set_external_interrupt(4, state); }
		void write_irq_7(int state) { set_external_interrupt(7, state); }

		void set_external_interrupt_level(int level){ set_external_interrupt(level, HOLD_LINE); };
		void set_external_interrupt(int level, int state);
		void set_internal_interrupt_request(int device, int state){ set_interrupt(device, state); }

		u16 get_par(){ return m_par; }

	protected:
		void device_start() override ATTR_COLD;
		void device_reset() override ATTR_COLD;
		void device_add_mconfig(machine_config &config) override ATTR_COLD;

	private:
		void set_interrupt(int interrupt, int state);

		TIMER_CALLBACK_MEMBER(swdt_callback);
		
		void icr_info(u8 icr);

		devcb_read8 irq_vector_cb;

		u8 simr_r(){ return m_simr; }		// sim configuration register
		u8 marb_r(){ return m_marb; }		// bus master arbitration control
		u8 icr_r(offs_t offset);			// interrupt control register
		u16 imr_r(){ return m_imr; }		// interrupt mask register
		u16 ipr_r(){ return m_ipr; }		// interrupt pending register
		u8 sypcr_r(){ return m_sypcr; }		// system protection control register
		u8 swivr_r(){ return m_swivr; }		// software watchdog interrupt vector register
		u8 rsr_r(){ return m_rsr; }			// reset status register
		u16 par_r(){ return m_par; }		// pin assignment register

		void simr_w(u8 data);							
		void marb_w(u8 data);
		void icr_w(offs_t offset, u8 data);
		void imr_w(u16 data);	
		void sypcr_w(u8 data);
		void swivr_w(u8 data);
		void swsr_w(u8 data);				// software watchdog service routine (kick)
		void rsr_w(u8 data);
		void par_w(u16 data);							

		const u8 swdt_reset_sequence[2] = { 0x55, 0xaa };
		u8 m_swdt_w_count;
		bool m_sypcr_locked;

		u8 m_simr;
		u8 m_marb;
		u8 m_icr[15];
		u16 m_par;
		u16 m_imr;
		u16 m_ipr;
		u8 m_sypcr;
		u8 m_swivr;
		u8 m_rsr;

		u8 m_external_ipl;

		emu_timer *m_timer_swdt;
		
		required_device<mcf5206e_device> m_maincpu;
		required_device<watchdog_timer_device> m_swdt;
		
};

class coldfire_dma_device : public device_t {
	// Manual states that the DMA channels cannot read from the internal SRAM
	public:
		coldfire_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		void dma_map(address_map &map) ATTR_COLD;
		auto irq_cb() ATTR_COLD { return write_irq.bind(); }
		u8 get_irq_vector() { return m_divr; }

	protected:
		void device_start() override ATTR_COLD;
		void device_reset() override ATTR_COLD;

	private:
		
		//TIMER_CALLBACK_MEMBER(dma_callback);
		//IRQ_CALLBACK_MEMBER(dma_int_callback)

		u32 sar_r(){ return m_sar; }
		u32 dar_r(){ return m_dar; }
		u16 dcr_r(){ return m_dcr; }
		u16 bcr_r(){ return m_bcr; }
		u8  dsr_r(){ return m_dsr; }
		u8  divr_r(){ return m_divr; }

		void sar_w(u32 data);
		void dar_w(u32 data);
		void dcr_w(u16 data);
		void bcr_w(u16 data);
		void dsr_w(u8 data);
		void divr_w(u8 data);

		u32 m_sar;
		u32 m_dar;
		u16 m_dcr;
		u16 m_bcr;
		u8 m_dsr;
		u8 m_divr;

		//bool m_tx_in_progress;

		//emu_timer *m_timer_dma;
		devcb_write_line write_irq;
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
		IRQ_CALLBACK_MEMBER(mbus_int_callback);

		inline u8 madr_r(){ return m_madr; }			// madr is only for when the cpu is a slave i2c device
		u8 mbdr_r();
		inline u8 mbcr_r(){ return m_mbcr; }
		u8 mbsr_r();
		inline u8 mfdr_r(){ return m_mfdr; }

		void madr_w(u8 data);
		void mbdr_w(u8 data);
		void mbcr_w(u8 data);
		void mbsr_w(u8 data);
		void mfdr_w(u8 data);

		u8 m_madr;
		u8 m_mbcr;
		u8 m_mbsr;
		u8 m_mfdr;
		u8 m_mbdr;

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
		auto irq_cb() { return write_irq.bind(); }

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

		u16 tmr_r(){ return m_tmr; }
		u16 trr_r(){ return m_trr; }
		u8 ter_r(){ return m_ter; }
		u16 tcr_r(){ return m_tcr; }
		u16 tcn_r();

		void tmr_w(u16 data);
		void trr_w(u16 data);
		void ter_w(u8 data);
		void tcn_w(u16 data);

		TIMER_CALLBACK_MEMBER(timer_callback);

		u16 m_tmr;
		u16 m_trr;
		u16 m_tcr;
		u16 m_tcn;
		u8 m_ter;

		attotime m_timer_start_time;

		emu_timer *m_timer;
		devcb_write_line write_irq;

		// Todo: Add output support
};

class mcf5206e_device : public m68000_musashi_device
{
	friend class coldfire_sim_device;
public:
	// construction/destruction
	mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	void set_tin1(int clk){ m_tin[0] = clk; }
	void set_tin2(int clk){ m_tin[1] = clk; }
	void set_tin(int tin1, int tin2){ set_tin1(tin1); set_tin2(tin2); }
	void set_tin1(const XTAL &clk) { set_tin1(clk.value()); }
	void set_tin2(const XTAL &clk) { set_tin2(clk.value()); }
	void set_tin(const XTAL &tin1, const XTAL &tin2) { set_tin1(tin1); set_tin2(tin2); }

	// Chip Select Module
	template<unsigned N> auto chip_select_w_cb() { return write_chip_select[N].bind(); }

	auto set_irq_acknowledge_callback() ATTR_COLD { return m_sim->irq_vector_cb.bind(); }

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

	virtual space_config_vector memory_space_config() const override;
	address_space_config m_coldfire_register_map;
	address_space_config m_coldfire_vector_map;

private:

	void coldfire_register_map(address_map &map) ATTR_COLD;
	void coldfire_vector_map(address_map &map) ATTR_COLD;

	void init_regs(bool first_init);

	u32 m_tin[2];

	/* System Intergration Module */
	required_device<coldfire_sim_device> m_sim;

	/* Timer Modules */
	void timer_1_irq(int state){ m_sim->set_internal_interrupt_request(TIMER_1_IRQ, state); }
	void timer_2_irq(int state){ m_sim->set_internal_interrupt_request(TIMER_2_IRQ, state); }

	required_device_array<coldfire_timer_device, 2> m_timer;
	
	/* dram controller module */
	inline u16 dcrr_r(){ return m_dcrr; }
	inline u16 dctr_r(){ return m_dctr; }
	inline u16 dcar0_r(){ return m_dcar0; }
	inline u32 dcmr0_r(){ return m_dcmr0; }
	inline u8 dccr0_r(){ return m_dccr0; }
	inline u16 dcar1_r(){ return m_dcar1; }
	inline u32 dcmr1_r(){ return m_dcmr1; }
	inline u8 dccr1_r(){ return m_dccr1; }

	void dcrr_w(u16 data);
	void dctr_w(u16 data);
	void dcar0_w(u16 data);
	void dcmr0_w(u32 data);
	void dccr0_w(u8 data);
	void dcar1_w(u16 data);
	void dcmr1_w(u32 data);
	void dccr1_w(u8 data);

	u16 m_dcrr;
	u16 m_dctr;
	u16 m_dcar0;
	u32 m_dcmr0;
	u8  m_dccr0;
	u16 m_dcar1;
	u32 m_dcmr1;
	u8  m_dccr1;

	/* chip select module */
	u16 csar_x_r(offs_t offset);
	u8 cscr_x_r(offs_t offset);
	u32 csmr_x_r(offs_t offset);
	inline u16 csar0_r(){ return csar_x_r(0); }
	inline u32 csmr0_r(){ return csmr_x_r(0); }
	inline u8 cscr0_r(){ return cscr_x_r(0); }
	inline u16 csar1_r(){ return csar_x_r(1); }
	inline u32 csmr1_r(){ return csmr_x_r(1); }
	inline u8 cscr1_r(){ return cscr_x_r(1); }
	inline u16 csar2_r(){ return csar_x_r(2); }
	inline u32 csmr2_r(){ return csmr_x_r(2); }
	inline u8 cscr2_r(){ return cscr_x_r(2); }
	inline u16 csar3_r(){ return csar_x_r(3); }
	inline u32 csmr3_r(){ return csmr_x_r(3); }
	inline u8 cscr3_r(){ return cscr_x_r(3); }
	inline u16 csar4_r(){ return csar_x_r(4); }
	inline u32 csmr4_r(){ return csmr_x_r(4); }
	inline u8 cscr4_r(){ return cscr_x_r(4); }
	inline u16 csar5_r(){ return csar_x_r(5); }
	inline u32 csmr5_r(){ return csmr_x_r(5); }
	inline u8 cscr5_r(){ return cscr_x_r(5); }
	inline u16 csar6_r(){ return csar_x_r(6); }
	inline u32 csmr6_r(){ return csmr_x_r(6); }
	inline u8 cscr6_r(){ return cscr_x_r(6); }
	inline u16 csar7_r(){ return csar_x_r(7); }
	inline u32 csmr7_r(){ return csmr_x_r(7); }
	inline u8 cscr7_r(){ return cscr_x_r(7); }
	inline u16 dmcr_r(){ return m_dmcr; }

	void csar_x_w(offs_t offset, u16 data);
	void csmr_x_w(offs_t offset, u32 data);
	void cscr_x_w(offs_t offset, u8 data);
	inline void csar0_w(u16 data){ csar_x_w(0, data); }
	inline void csmr0_w(u32 data){ csmr_x_w(0, data); }
	inline void cscr0_w(u8 data){ cscr_x_w(0, data); }
	inline void csar1_w(u16 data){ csar_x_w(1, data); }
	inline void csmr1_w(u32 data){ csmr_x_w(1, data); }
	inline void cscr1_w(u8 data){ cscr_x_w(1, data); }
	inline void csar2_w(u16 data){ csar_x_w(2, data); }
	inline void csmr2_w(u32 data){ csmr_x_w(2, data); }
	inline void cscr2_w(u8 data){ cscr_x_w(2, data); }
	inline void csar3_w(u16 data){ csar_x_w(3, data); }
	inline void csmr3_w(u32 data){ csmr_x_w(3, data); }
	inline void cscr3_w(u8 data){ cscr_x_w(3, data); }
	inline void csar4_w(u16 data){ csar_x_w(4, data); }
	inline void csmr4_w(u32 data){ csmr_x_w(4, data); }
	inline void cscr4_w(u8 data){ cscr_x_w(4, data); }
	inline void csar5_w(u16 data){ csar_x_w(5, data); }
	inline void csmr5_w(u32 data){ csmr_x_w(5, data); }
	inline void cscr5_w(u8 data){ cscr_x_w(5, data); }
	inline void csar6_w(u16 data){ csar_x_w(6, data); }
	inline void csmr6_w(u32 data){ csmr_x_w(6, data); }
	inline void cscr6_w(u8 data){ cscr_x_w(6, data); }
	inline void csar7_w(u16 data){ csar_x_w(7, data); }
	inline void csmr7_w(u32 data){ csmr_x_w(7, data); }
	inline void cscr7_w(u8 data){ cscr_x_w(7, data); }
	void dmcr_w(u16 data);
	
	u16 m_csar[8];
	u32 m_csmr[8];
	u16 m_cscr[8];
	u16 m_dmcr;

	devcb_write_line::array<8> write_chip_select;

	/* UART Modules */ 
	// Stick two 68681 A channels in the CPU with no timers.
	void uart_1_irq(int state){ m_sim->set_internal_interrupt_request(UART_1_IRQ, state); }
	void uart_2_irq(int state){ m_sim->set_internal_interrupt_request(UART_2_IRQ, state); }

	required_device_array<mcf5206e_uart_device, 2> m_uart;
	devcb_write_line write_tx1, write_tx2;

	/* parallel port */
	// just an 8 bit gpio port
	inline u8 ppddr_r(){ return m_ppddr; }
	inline u8 ppdat_r(){ return (m_ppdat_in | (m_ppdat_out & m_ppddr)); }
	
	void ppddr_w(u8 data);
	void ppdat_w(u8 data);

	u8 m_ppddr;
	u8 m_ppdat_in;
	u8 m_ppdat_out;
	devcb_write8 m_gpio_w_cb;

	/* MBUS Module */
	// I2C host/device but Motorola
	void mbus_sda_w(int state){ write_sda(state); }
	void mbus_scl_w(int state){ write_scl(state); }
	void mbus_irq_w(int state){ m_sim->set_external_interrupt(MBUS_IRQ, state); }
	required_device<coldfire_mbus_device> m_mbus;
	devcb_write_line write_sda, write_scl;

	/* DMA Modules */
	required_device_array<coldfire_dma_device, 2> m_dma;
	void dma0_irq_w(int state){ m_sim->set_external_interrupt(DMA_0_IRQ, state); }
	void dma1_irq_w(int state){ m_sim->set_external_interrupt(DMA_1_IRQ, state); }

};

DECLARE_DEVICE_TYPE(MCF5206E, mcf5206e_device)
DECLARE_DEVICE_TYPE(COLDFIRE_SIM, coldfire_sim_device)
DECLARE_DEVICE_TYPE(COLDFIRE_TIMER, coldfire_timer_device)
DECLARE_DEVICE_TYPE(COLDFIRE_MBUS, coldfire_mbus_device)
DECLARE_DEVICE_TYPE(COLDFIRE_DMA, coldfire_dma_device)

#endif
