// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

	Intel XScale SA1110 peripheral emulation

***************************************************************************/

#ifndef MAME_MACHINE_SA1110
#define MAME_MACHINE_SA1110

#pragma once

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "machine/input_merger.h"

#include "diserial.h"

class sa1110_periphs_device : public device_t, public device_serial_interface
{
public:
	template <typename T>
	sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: sa1110_periphs_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	void gpio_in(const uint32_t line, const int state);
	template <unsigned Line> auto gpio_out() { return m_gpio_out[Line].bind(); }

	uint32_t uart3_r(offs_t offset, uint32_t mem_mask = ~0);
	void uart3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ostimer_r(offs_t offset, uint32_t mem_mask = ~0);
	void ostimer_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtc_r(offs_t offset, uint32_t mem_mask = ~0);
	void rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t power_r(offs_t offset, uint32_t mem_mask = ~0);
	void power_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t reset_r(offs_t offset, uint32_t mem_mask = ~0);
	void reset_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gpio_r(offs_t offset, uint32_t mem_mask = ~0);
	void gpio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t intc_r(offs_t offset, uint32_t mem_mask = ~0);
	void intc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	static constexpr uint32_t INTERNAL_OSC = 3686400;
	DECLARE_WRITE_LINE_MEMBER(uart3_irq_callback);
	void uart_recalculate_divisor();
	void uart_update_eif_status();
	uint8_t uart_read_receive_fifo();
	void uart_write_transmit_fifo(uint8_t data);
	void uart_check_tx_fifo_service();
	void uart_set_receiver_idle();
	void uart_begin_of_break();
	void uart_end_of_break();
	void uart_set_receiver_enabled(bool enabled);
	void uart_set_transmitter_enabled(bool enabled);
	void uart_set_receive_irq_enabled(bool enabled);
	void uart_set_transmit_irq_enabled(bool enabled);

	TIMER_CALLBACK_MEMBER(ostimer_tick_cb);
	void ostimer_update_count();
	void ostimer_update_match_timer(int channel);

	TIMER_CALLBACK_MEMBER(rtc_tick_cb);

	void gpio_update_interrupts(const uint32_t changed_mask);
	void gpio_update_direction(const uint32_t old_gpdr);
	void gpio_update_outputs(const uint32_t old_latch, const uint32_t changed);
	void gpio_update_alternate_pins(const uint32_t changed_mask);

	void set_irq_line(uint32_t line, int state);
	void update_interrupts();

	// register offsets
	enum
	{
		UART_BASE_ADDR  = 0x80050000,
		REG_UTCR0       = (0x00000000 >> 2),
		REG_UTCR1       = (0x00000004 >> 2),
		REG_UTCR2       = (0x00000008 >> 2),
		REG_UTCR3       = (0x0000000c >> 2),
		REG_UTDR        = (0x00000014 >> 2),
		REG_UTSR0       = (0x0000001c >> 2),
		REG_UTSR1       = (0x00000020 >> 2),

		OSTMR_BASE_ADDR	= 0x90000000,
		REG_OSMR0       = (0x00000000 >> 2),
		REG_OSMR1       = (0x00000004 >> 2),
		REG_OSMR2       = (0x00000008 >> 2),
		REG_OSMR3       = (0x0000000c >> 2),
		REG_OSCR        = (0x00000010 >> 2),
		REG_OSSR        = (0x00000014 >> 2),
		REG_OWER        = (0x00000018 >> 2),
		REG_OIER        = (0x0000001c >> 2),

		RTC_BASE_ADDR	= 0x90010000,
		REG_RTAR        = (0x00000000 >> 2),
		REG_RCNR        = (0x00000004 >> 2),
		REG_RTTR        = (0x00000008 >> 2),
		REG_RTSR        = (0x00000010 >> 2),

		POWER_BASE_ADDR = 0x90020000,
		REG_PMCR        = (0x00000000 >> 2),
		REG_PSSR        = (0x00000004 >> 2),
		REG_PSPR        = (0x00000008 >> 2),
		REG_PWER        = (0x0000000c >> 2),
		REG_PCFR        = (0x00000010 >> 2),
		REG_PPCR        = (0x00000014 >> 2),
		REG_PGSR        = (0x00000018 >> 2),
		REG_POSR        = (0x0000001c >> 2),

		RESET_BASE_ADDR	= 0x90030000,
		REG_RSRR		= (0x00000000 >> 2),
		REG_RCSR		= (0x00000004 >> 2),

		GPIO_BASE_ADDR	= 0x90040000,
		REG_GPLR		= (0x00000000 >> 2),
		REG_GPDR		= (0x00000004 >> 2),
		REG_GPSR		= (0x00000008 >> 2),
		REG_GPCR		= (0x0000000c >> 2),
		REG_GRER		= (0x00000010 >> 2),
		REG_GFER		= (0x00000014 >> 2),
		REG_GEDR		= (0x00000018 >> 2),
		REG_GAFR		= (0x0000001c >> 2),

		INTC_BASE_ADDR  = 0x90050000,
		REG_ICIP        = (0x00000000 >> 2),
		REG_ICMR        = (0x00000004 >> 2),
		REG_ICLR        = (0x00000008 >> 2),
		REG_ICCR        = (0x0000000c >> 2),
		REG_ICFP        = (0x00000010 >> 2),
		REG_ICPR        = (0x00000020 >> 2)
	};

	// register contents
	enum : uint32_t
	{
		UART3_FIFO_PRE	= 8,
		UART3_FIFO_FRE	= 9,
		UART3_FIFO_ROR	= 10,

		UTCR3_RXE_BIT	= 0,
		UTCR3_TXE_BIT	= 1,
		UTCR3_BRK_BIT	= 2,
		UTCR3_RIE_BIT	= 3,
		UTCR3_TIE_BIT	= 4,
		UTCR3_LBM_BIT	= 5,

		UTSR0_TFS_BIT	= 0,
		UTSR0_RFS_BIT	= 1,
		UTSR0_RID_BIT	= 2,
		UTSR0_RBB_BIT	= 3,
		UTSR0_REB_BIT	= 4,
		UTSR0_EIF_BIT	= 5,

		UTSR1_TBY_BIT	= 0,
		UTSR1_RNE_BIT	= 1,
		UTSR1_TNF_BIT	= 2,
		UTSR1_PRE_BIT	= 3,
		UTSR1_FRE_BIT	= 4,
		UTSR1_ROR_BIT	= 5,

		RTSR_AL_BIT		= 0,
		RTSR_AL_MASK	= (1 << RTSR_AL_BIT),
		RTSR_HZ_BIT		= 1,
		RTSR_HZ_MASK	= (1 << RTSR_HZ_BIT),
		RTSR_ALE_BIT	= 2,
		RTSR_ALE_MASK	= (1 << RTSR_ALE_BIT),
		RTSR_HZE_BIT	= 3,
		RTSR_HZE_MASK	= (1 << RTSR_HZE_BIT)
	};

	// interrupt bits
	enum : uint32_t
	{
		INT_GPIO0		= 0,
		INT_GPIO1		= 1,
		INT_GPIO2		= 2,
		INT_GPIO3		= 3,
		INT_GPIO4		= 4,
		INT_GPIO5		= 5,
		INT_GPIO6		= 6,
		INT_GPIO7		= 7,
		INT_GPIO8		= 8,
		INT_GPIO9		= 9,
		INT_GPIO10		= 10,
		INT_GPIOHI		= 11,
		INT_LCD			= 12,
		INT_UDC			= 13,
		INT_UART1		= 15,
		INT_UART2		= 16,
		INT_UART3		= 17,
		INT_MCP			= 18,
		INT_SSP			= 19,
		INT_DMA0		= 20,
		INT_DMA1		= 21,
		INT_DMA2		= 22,
		INT_DMA3		= 23,
		INT_DMA4		= 24,
		INT_DMA5		= 25,
		INT_OSTIMER0	= 26,
		INT_OSTIMER1	= 27,
		INT_OSTIMER2	= 28,
		INT_OSTIMER3	= 29,
		INT_RTC_TICK	= 30,
		INT_RTC_ALARM	= 31
	};

	// UART3 interrupt sources
	enum : unsigned
	{
		UART3_TFS		= 0,
		UART3_RFS		= 1,
		UART3_RID		= 2,
		UART3_RBB		= 3,
		UART3_REB		= 4,
		UART3_EIF		= 5,
	};

	struct uart_regs
	{
		uint32_t utcr[4];
		uint32_t utsr0;
		uint32_t utsr1;

		uint16_t rx_fifo[12];
		int      rx_fifo_read_idx;
		int      rx_fifo_write_idx;
		int      rx_fifo_count;

		uint8_t tx_fifo[8];
		int     tx_fifo_read_idx;
		int     tx_fifo_write_idx;
		int     tx_fifo_count;

		bool	rx_break_interlock;
	};

	struct ostimer_regs
	{
		uint32_t osmr[4];
		uint32_t oscr;
		uint32_t ossr;
		uint32_t ower;
		uint32_t oier;

		emu_timer *timer[4];
		attotime last_count_sync;
	};

	struct rtc_regs
	{
		uint32_t rtar;
		uint32_t rcnr;
		uint32_t rttr;
		uint32_t rtsr;

		emu_timer *tick_timer;
	};

	struct power_regs
	{
		uint32_t pmcr;
		uint32_t pssr;
		uint32_t pspr;
		uint32_t pwer;
		uint32_t pcfr;
		uint32_t ppcr;
		uint32_t pgsr;
		uint32_t posr;
	};

	struct gpio_regs
	{
		uint32_t gplr;
		uint32_t gpdr;
		uint32_t grer;
		uint32_t gfer;
		uint32_t gedr;
		uint32_t gafr;

		uint32_t any_edge_mask;

		uint32_t output_latch;
		uint32_t input_latch;
		uint32_t alt_output_latch;
		uint32_t alt_input_latch;
	};

	struct intc_regs
	{
		uint32_t icip;
		uint32_t icmr;
		uint32_t iclr;
		uint32_t iccr;
		uint32_t icfp;
		uint32_t icpr;
	};

	uart_regs		m_uart_regs;
	ostimer_regs	m_ostmr_regs;
	rtc_regs		m_rtc_regs;
	power_regs		m_power_regs;
	uint32_t		m_rcsr;
	gpio_regs		m_gpio_regs;
	intc_regs		m_intc_regs;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_uart3_irqs;

	devcb_write_line::array<28> m_gpio_out;
};

DECLARE_DEVICE_TYPE(SA1110_PERIPHERALS, sa1110_periphs_device)

#endif // MAME_MACHINE_SA1110
