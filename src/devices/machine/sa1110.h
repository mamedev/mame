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
#include "machine/ucb1200.h"

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

	template <typename T> void set_codec_tag(T &&tag) { m_codec.set_tag(std::forward<T>(tag)); }

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	template <unsigned Line> void gpio_in(int state) { gpio_in(Line, state); }
	template <unsigned Line> auto gpio_out() { return m_gpio_out[Line].bind(); }

	void ssp_in(uint16_t data) { ssp_rx_fifo_push(data); }
	auto ssp_out() { return m_ssp_out.bind(); }

	auto uart3_tx_out() { return m_uart3_tx_out.bind(); }

	uint32_t udc_r(offs_t offset, uint32_t mem_mask = ~0);
	void udc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t icp_r(offs_t offset, uint32_t mem_mask = ~0);
	void icp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t uart3_r(offs_t offset, uint32_t mem_mask = ~0);
	void uart3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t mcp_r(offs_t offset, uint32_t mem_mask = ~0);
	void mcp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ssp_r(offs_t offset, uint32_t mem_mask = ~0);
	void ssp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
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
	uint32_t ppc_r(offs_t offset, uint32_t mem_mask = ~0);
	void ppc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dma_r(offs_t offset, uint32_t mem_mask = ~0);
	void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	static constexpr uint32_t INTERNAL_OSC = 3686400;

	TIMER_CALLBACK_MEMBER(icp_rx_callback);
	TIMER_CALLBACK_MEMBER(icp_tx_callback);
	TIMER_CALLBACK_MEMBER(hssp_rx_callback);
	TIMER_CALLBACK_MEMBER(hssp_tx_callback);
	void icp_uart_set_receiver_enabled(bool enabled);
	void icp_uart_set_transmitter_enabled(bool enabled);
	void icp_uart_set_receive_irq_enabled(bool enabled);
	void icp_uart_set_transmit_irq_enabled(bool enabled);
	uint8_t icp_uart_read_receive_fifo();
	void icp_uart_write_transmit_fifo(uint8_t data);
	uint16_t icp_hssp_read_receive_fifo();
	void icp_hssp_write_transmit_fifo(uint8_t data);
	void icp_uart_set_receiver_idle();
	void icp_uart_begin_of_break();
	void icp_uart_end_of_break();

	DECLARE_WRITE_LINE_MEMBER(uart3_irq_callback);
	void uart_recalculate_divisor();
	void uart_update_eif_status();
	void uart_write_receive_fifo(uint16_t data_and_flags);
	uint8_t uart_read_receive_fifo();
	void uart_write_transmit_fifo(uint8_t data);
	void uart_check_rx_fifo_service();
	void uart_check_tx_fifo_service();
	void uart_set_receiver_idle();
	void uart_begin_of_break();
	void uart_end_of_break();
	void uart_set_receiver_enabled(bool enabled);
	void uart_set_transmitter_enabled(bool enabled);
	void uart_set_receive_irq_enabled(bool enabled);
	void uart_set_transmit_irq_enabled(bool enabled);

	DECLARE_WRITE_LINE_MEMBER(mcp_irq_callback);
	TIMER_CALLBACK_MEMBER(mcp_audio_tx_callback);
	TIMER_CALLBACK_MEMBER(mcp_telecom_tx_callback);
	void mcp_update_sample_rate();
	void mcp_set_enabled(bool enabled);
	uint16_t mcp_read_audio_fifo();
	uint16_t mcp_read_telecom_fifo();
	attotime mcp_get_audio_frame_rate();
	attotime mcp_get_telecom_frame_rate();
	void mcp_audio_tx_fifo_push(const uint16_t value);
	void mcp_telecom_tx_fifo_push(const uint16_t value);
	void mcp_codec_read(offs_t offset);
	void mcp_codec_write(offs_t offset, uint16_t data);

	TIMER_CALLBACK_MEMBER(ssp_rx_callback);
	TIMER_CALLBACK_MEMBER(ssp_tx_callback);
	void ssp_update_enable_state();
	void ssp_update_rx_level();
	void ssp_update_tx_level();
	void ssp_rx_fifo_push(const uint16_t data);
	void ssp_tx_fifo_push(const uint16_t data);
	uint16_t ssp_rx_fifo_pop();

	TIMER_CALLBACK_MEMBER(ostimer_tick_cb);
	void ostimer_update_count();
	void ostimer_update_match_timer(int channel);

	TIMER_CALLBACK_MEMBER(rtc_tick_cb);

	void gpio_in(const uint32_t line, const int state);
	void gpio_update_interrupts(const uint32_t changed_mask);
	void gpio_update_direction(const uint32_t old_gpdr);
	void gpio_update_outputs(const uint32_t old_latch, const uint32_t changed);
	void gpio_update_alternate_pins(const uint32_t changed_mask);

	void set_irq_line(uint32_t line, int state);
	void update_interrupts();

	void dma_set_control_bits(int channel, uint32_t bits);
	void dma_clear_control_bits(int channel, uint32_t bits);

	// register offsets
	enum
	{
		UDC_BASE_ADDR   = 0x80000000,
		REG_UDCCR       = (0x00000000 >> 2),
		REG_UDCAR       = (0x00000004 >> 2),
		REG_UDCOMP      = (0x00000008 >> 2),
		REG_UDCIMP      = (0x0000000c >> 2),
		REG_UDCCS0      = (0x00000010 >> 2),
		REG_UDCCS1      = (0x00000014 >> 2),
		REG_UDCCS2      = (0x00000018 >> 2),
		REG_UDCD0       = (0x0000001c >> 2),
		REG_UDCWC       = (0x00000020 >> 2),
		REG_UDCDR       = (0x00000028 >> 2),
		REG_UDCSR       = (0x00000030 >> 2),

		ICP_BASE_ADDR   = 0x80030000,
		REG_UTCR4       = (0x00000010 >> 2),
		REG_HSCR0       = (0x00000060 >> 2),
		REG_HSCR1       = (0x00000064 >> 2),
		REG_HSDR        = (0x0000006c >> 2),
		REG_HSSR0       = (0x00000074 >> 2),
		REG_HSSR1       = (0x00000078 >> 2),

		UART_BASE_ADDR  = 0x80050000,
		REG_UTCR0       = (0x00000000 >> 2),
		REG_UTCR1       = (0x00000004 >> 2),
		REG_UTCR2       = (0x00000008 >> 2),
		REG_UTCR3       = (0x0000000c >> 2),
		REG_UTDR        = (0x00000014 >> 2),
		REG_UTSR0       = (0x0000001c >> 2),
		REG_UTSR1       = (0x00000020 >> 2),

		MCP_BASE_ADDR   = 0x80060000,
		REG_MCCR0       = (0x00000000 >> 2),
		REG_MCDR0       = (0x00000008 >> 2),
		REG_MCDR1       = (0x0000000c >> 2),
		REG_MCDR2       = (0x00000010 >> 2),
		REG_MCSR        = (0x00000018 >> 2),

		SSP_BASE_ADDR   = 0x80070000,
		REG_SSCR0       = (0x00000060 >> 2),
		REG_SSCR1       = (0x00000064 >> 2),
		REG_SSDR        = (0x0000006c >> 2),
		REG_SSSR        = (0x00000074 >> 2),

		OSTMR_BASE_ADDR = 0x90000000,
		REG_OSMR0       = (0x00000000 >> 2),
		REG_OSMR1       = (0x00000004 >> 2),
		REG_OSMR2       = (0x00000008 >> 2),
		REG_OSMR3       = (0x0000000c >> 2),
		REG_OSCR        = (0x00000010 >> 2),
		REG_OSSR        = (0x00000014 >> 2),
		REG_OWER        = (0x00000018 >> 2),
		REG_OIER        = (0x0000001c >> 2),

		RTC_BASE_ADDR   = 0x90010000,
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

		RESET_BASE_ADDR = 0x90030000,
		REG_RSRR        = (0x00000000 >> 2),
		REG_RCSR        = (0x00000004 >> 2),

		GPIO_BASE_ADDR  = 0x90040000,
		REG_GPLR        = (0x00000000 >> 2),
		REG_GPDR        = (0x00000004 >> 2),
		REG_GPSR        = (0x00000008 >> 2),
		REG_GPCR        = (0x0000000c >> 2),
		REG_GRER        = (0x00000010 >> 2),
		REG_GFER        = (0x00000014 >> 2),
		REG_GEDR        = (0x00000018 >> 2),
		REG_GAFR        = (0x0000001c >> 2),

		INTC_BASE_ADDR  = 0x90050000,
		REG_ICIP        = (0x00000000 >> 2),
		REG_ICMR        = (0x00000004 >> 2),
		REG_ICLR        = (0x00000008 >> 2),
		REG_ICCR        = (0x0000000c >> 2),
		REG_ICFP        = (0x00000010 >> 2),
		REG_ICPR        = (0x00000020 >> 2),

		PPC_BASE_ADDR   = 0x90060000,
		REG_PPDR        = (0x00000000 >> 2),
		REG_PPSR        = (0x00000004 >> 2),
		REG_PPAR        = (0x00000008 >> 2),
		REG_PSDR        = (0x0000000c >> 2),
		REG_PPFR        = (0x00000010 >> 2),

		DMA_BASE_ADDR   = 0xb0000000,
		REG_DDAR        = (0x00000000 >> 2),
		REG_DSSR        = (0x00000004 >> 2),
		REG_DCSR        = (0x00000008 >> 2),
		REG_DSR         = (0x0000000c >> 2),
		REG_DBSA        = (0x00000010 >> 2),
		REG_DBTA        = (0x00000014 >> 2),
		REG_DBSB        = (0x00000018 >> 2),
		REG_DBTB        = (0x0000001c >> 2)
	};

	// register contents
	enum : uint32_t
	{
		UDCCR_UDD_BIT       = 0,
		UDCCR_UDA_BIT       = 1,
		UDCCR_RESM_BIT      = 2,
		UDCCR_EIM_BIT       = 3,
		UDCCR_RIM_BIT       = 4,
		UDCCR_TIM_BIT       = 5,
		UDCCR_SUSM_BIT      = 6,
		UDCCR_WRITE_MASK    = 0x7d,

		UDCAR_WRITE_MASK    = 0x7f,

		UDCOMP_WRITE_MASK   = 0xff,

		UDCIMP_WRITE_MASK   = 0xff,

		UDCCS0_OPR_BIT      = 0,
		UDCCS0_IPR_BIT      = 1,
		UDCCS0_SST_BIT      = 2,
		UDCCS0_FST_BIT      = 3,
		UDCCS0_DE_BIT       = 4,
		UDCCS0_SE_BIT       = 5,
		UDCCS0_SO_BIT       = 6,
		UDCCS0_SSE_BIT      = 7,

		UDCCS1_RFS_BIT      = 0,
		UDCCS1_RPC_BIT      = 1,
		UDCCS1_RPE_BIT      = 2,
		UDCCS1_SST_BIT      = 3,
		UDCCS1_FST_BIT      = 4,
		UDCCS1_RNE_BIT      = 5,

		UDCCS2_TFS_BIT      = 0,
		UDCCS2_TPC_BIT      = 1,
		UDCCS2_TPE_BIT      = 2,
		UDCCS2_TUR_BIT      = 3,
		UDCCS2_SST_BIT      = 4,
		UDCCS2_FST_BIT      = 5,

		UDCWC_WRITE_MASK    = 0x0f,

		UDCSR_EIR_BIT       = 0,
		UDCSR_RIR_BIT       = 1,
		UDCSR_TIR_BIT       = 2,
		UDCSR_SUSIR_BIT     = 3,
		UDCSR_RESIR_BIT     = 4,
		UDCSR_RSTIR_BIT     = 5,

		UART3_FIFO_PRE  = 8,
		UART3_FIFO_FRE  = 9,
		UART3_FIFO_ROR  = 10,

		UTCR3_RXE_BIT   = 0,
		UTCR3_TXE_BIT   = 1,
		UTCR3_BRK_BIT   = 2,
		UTCR3_RIE_BIT   = 3,
		UTCR3_TIE_BIT   = 4,
		UTCR3_LBM_BIT   = 5,

		UTCR4_HSE_BIT   = 0,
		UTCR4_LPM_BIT   = 1,

		UTSR0_TFS_BIT   = 0,
		UTSR0_RFS_BIT   = 1,
		UTSR0_RID_BIT   = 2,
		UTSR0_RBB_BIT   = 3,
		UTSR0_REB_BIT   = 4,
		UTSR0_EIF_BIT   = 5,

		UTSR1_TBY_BIT   = 0,
		UTSR1_RNE_BIT   = 1,
		UTSR1_TNF_BIT   = 2,
		UTSR1_PRE_BIT   = 3,
		UTSR1_FRE_BIT   = 4,
		UTSR1_ROR_BIT   = 5,

		HSCR0_ITR_BIT   = 0,
		HSCR0_LBM_BIT   = 1,
		HSCR0_TUS_BIT   = 2,
		HSCR0_TXE_BIT   = 3,
		HSCR0_RXE_BIT   = 4,
		HSCR0_RIE_BIT   = 5,
		HSCR0_TIE_BIT   = 6,
		HSCR0_AME_BIT   = 7,

		HSCR2_TXP_BIT   = 18,
		HSCR2_RXP_BIT   = 19,

		HSDR_EOF_BIT    = 8,
		HSDR_CRE_BIT    = 9,
		HSDR_ROR_BIT    = 10,

		HSSR0_EIF_BIT   = 0,
		HSSR0_TUR_BIT   = 1,
		HSSR0_RAB_BIT   = 2,
		HSSR0_TFS_BIT   = 3,
		HSSR0_RFS_BIT   = 4,
		HSSR0_FRE_BIT   = 5,

		HSSR1_RSY_BIT   = 0,
		HSSR1_TBY_BIT   = 1,
		HSSR1_RNE_BIT   = 2,
		HSSR1_TNF_BIT   = 3,
		HSSR1_EOF_BIT   = 4,
		HSSR1_CRE_BIT   = 5,
		HSSR1_ROR_BIT   = 6,

		MCCR0_ASD_BIT   = 0,
		MCCR0_ASD_MASK  = 0x0000007f,
		MCCR0_TSD_BIT   = 8,
		MCCR0_TSD_MASK  = 0x00007f00,
		MCCR0_MCE_BIT   = 16,
		MCCR0_ECS_BIT   = 17,
		MCCR0_ADM_BIT   = 18,
		MCCR0_TTE_BIT   = 19,
		MCCR0_TRE_BIT   = 20,
		MCCR0_ATE_BIT   = 21,
		MCCR0_ARE_BIT   = 22,
		MCCR0_LBM_BIT   = 23,
		MCCR0_ECP_BIT   = 24,
		MCCR0_ECP_MASK  = 0x03000000,

		MCCR1_CFS_BIT   = 20,

		MCDR2_RW_BIT    = 16,
		MCDR2_ADDR_BIT  = 17,
		MCDR2_ADDR_MASK = 0x001e0000,

		MCSR_ATS_BIT    = 0,
		MCSR_ARS_BIT    = 1,
		MCSR_TTS_BIT    = 2,
		MCSR_TRS_BIT    = 3,
		MCSR_ATU_BIT    = 4,
		MCSR_ARO_BIT    = 5,
		MCSR_TTU_BIT    = 6,
		MCSR_TRO_BIT    = 7,
		MCSR_ANF_BIT    = 8,
		MCSR_ANE_BIT    = 9,
		MCSR_TNF_BIT    = 10,
		MCSR_TNE_BIT    = 11,
		MCSR_CWC_BIT    = 12,
		MCSR_CRC_BIT    = 13,
		MCSR_ACE_BIT    = 14,
		MCSR_TCE_BIT    = 15,

		SSCR0_DSS_BIT   = 0,
		SSCR0_DSS_MASK  = 0x0000000f,
		SSCR0_FRF_BIT   = 4,
		SSCR0_FRF_MASK  = 0x00000030,
		SSCR0_SSE_BIT   = 7,
		SSCR0_SCR_BIT   = 8,
		SSCR0_SCR_MASK  = 0x0000ff00,

		SSCR1_RIE_BIT   = 0,
		SSCR1_TIE_BIT   = 1,
		SSCR1_LBM_BIT   = 2,
		SSCR1_SPO_BIT   = 3,
		SSCR1_SPH_BIT   = 4,
		SSCR1_ECS_BIT   = 5,

		SSSR_TNF_BIT    = 1,
		SSSR_RNE_BIT    = 2,
		SSSR_BSY_BIT    = 3,
		SSSR_TFS_BIT    = 4,
		SSSR_RFS_BIT    = 5,
		SSSR_ROR_BIT    = 6,

		RTSR_AL_BIT     = 0,
		RTSR_AL_MASK    = (1 << RTSR_AL_BIT),
		RTSR_HZ_BIT     = 1,
		RTSR_HZ_MASK    = (1 << RTSR_HZ_BIT),
		RTSR_ALE_BIT    = 2,
		RTSR_ALE_MASK   = (1 << RTSR_ALE_BIT),
		RTSR_HZE_BIT    = 3,
		RTSR_HZE_MASK   = (1 << RTSR_HZE_BIT),

		DDAR_RW_BIT     = 0,
		DDAR_E_BIT      = 1,
		DDAR_BS_BIT     = 2,
		DDAR_DW_BIT     = 3,
		DDAR_DA0_BIT    = 4,
		DDAR_DA0_MASK   = 0x000000f0,
		DDAR_DA8_BIT    = 8,
		DDAR_DA8_MASK   = 0xffffff00,

		DSR_RUN_BIT     = 0,
		DSR_IE_BIT      = 1,
		DSR_ERROR_BIT   = 2,
		DSR_DONEA_BIT   = 3,
		DSR_STRTA_BIT   = 4,
		DSR_DONEB_BIT   = 5,
		DSR_STRTB_BIT   = 6,
		DSR_BIU_BIT     = 7,

		DBT_MASK        = 0x00001fff
	};

	// interrupt bits
	enum : uint32_t
	{
		INT_GPIO0       = 0,
		INT_GPIO1       = 1,
		INT_GPIO2       = 2,
		INT_GPIO3       = 3,
		INT_GPIO4       = 4,
		INT_GPIO5       = 5,
		INT_GPIO6       = 6,
		INT_GPIO7       = 7,
		INT_GPIO8       = 8,
		INT_GPIO9       = 9,
		INT_GPIO10      = 10,
		INT_GPIOHI      = 11,
		INT_LCD         = 12,
		INT_UDC         = 13,
		INT_UART1       = 15,
		INT_UART2       = 16,
		INT_UART3       = 17,
		INT_MCP         = 18,
		INT_SSP         = 19,
		INT_DMA0        = 20,
		INT_DMA1        = 21,
		INT_DMA2        = 22,
		INT_DMA3        = 23,
		INT_DMA4        = 24,
		INT_DMA5        = 25,
		INT_OSTIMER0    = 26,
		INT_OSTIMER1    = 27,
		INT_OSTIMER2    = 28,
		INT_OSTIMER3    = 29,
		INT_RTC_TICK    = 30,
		INT_RTC_ALARM   = 31
	};

	// UART3 interrupt sources
	enum : unsigned
	{
		UART3_TFS       = 0,
		UART3_RFS       = 1,
		UART3_RID       = 2,
		UART3_RBB       = 3,
		UART3_REB       = 4,
		UART3_EIF       = 5,
	};

	// MCP interrupt sources
	enum : unsigned
	{
		MCP_AUDIO_TX            = 0,
		MCP_AUDIO_RX            = 1,
		MCP_TELECOM_TX          = 2,
		MCP_TELECOM_RX          = 3,
		MCP_AUDIO_UNDERRUN      = 4,
		MCP_AUDIO_OVERRUN       = 5,
		MCP_TELECOM_UNDERRUN    = 6,
		MCP_TELECOM_OVERRUN     = 7
	};

	struct udc_regs
	{
		uint32_t udccr;
		uint32_t udcar;
		uint32_t udcomp;
		uint32_t udcimp;
		uint32_t udccs0;
		uint32_t udccs1;
		uint32_t udccs2;
		uint32_t udcwc;
		uint32_t udcsr;
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

		bool    rx_break_interlock;
	};

	struct hssp_regs
	{
		uint32_t hscr0;
		uint32_t hscr1;
		uint32_t hssr0;
		uint32_t hssr1;

		uint16_t rx_fifo[8];
		int rx_fifo_read_idx;
		int rx_fifo_write_idx;
		int rx_fifo_count;
		emu_timer *rx_timer;

		uint16_t tx_fifo[8];
		int tx_fifo_read_idx;
		int tx_fifo_write_idx;
		int tx_fifo_count;
		emu_timer *tx_timer;
	};

	struct icp_regs
	{
		uart_regs uart;
		uint32_t utcr4;
		emu_timer *uart_rx_timer;
		emu_timer *uart_tx_timer;

		hssp_regs hssp;
	};

	struct mcp_regs
	{
		uint32_t mccr0;
		uint32_t mccr1;
		uint32_t mcdr2;
		uint32_t mcsr;

		uint16_t audio_rx_fifo[8];
		int      audio_rx_fifo_read_idx;
		int      audio_rx_fifo_write_idx;
		int      audio_rx_fifo_count;

		uint16_t audio_tx_fifo[8];
		int      audio_tx_fifo_read_idx;
		int      audio_tx_fifo_write_idx;
		int      audio_tx_fifo_count;
		emu_timer *audio_tx_timer;

		uint16_t telecom_rx_fifo[8];
		int      telecom_rx_fifo_read_idx;
		int      telecom_rx_fifo_write_idx;
		int      telecom_rx_fifo_count;

		uint16_t telecom_tx_fifo[8];
		int      telecom_tx_fifo_read_idx;
		int      telecom_tx_fifo_write_idx;
		int      telecom_tx_fifo_count;
		emu_timer *telecom_tx_timer;
	};

	struct ssp_regs
	{
		uint32_t sscr0;
		uint32_t sscr1;
		uint32_t sssr;

		uint16_t rx_fifo[8];
		int rx_fifo_read_idx;
		int rx_fifo_write_idx;
		int rx_fifo_count;
		emu_timer *rx_timer;

		uint16_t tx_fifo[8];
		int tx_fifo_read_idx;
		int tx_fifo_write_idx;
		int tx_fifo_count;
		emu_timer *tx_timer;
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

	struct ppc_regs
	{
		uint32_t ppdr;
		uint32_t ppsr_out;
		uint32_t ppsr_in;
		uint32_t ppsr;
		uint32_t ppar;
		uint32_t psdr;
		uint32_t ppfr;
	};

	struct dma_regs
	{
		uint32_t ddar;
		uint32_t dsr;
		uint32_t dbs[2];
		uint32_t dbt[2];
	};

	udc_regs        m_udc_regs;
	uart_regs       m_uart_regs;
	icp_regs        m_icp_regs;
	mcp_regs        m_mcp_regs;
	ssp_regs        m_ssp_regs;
	ostimer_regs    m_ostmr_regs;
	rtc_regs        m_rtc_regs;
	power_regs      m_power_regs;
	uint32_t        m_rcsr;
	gpio_regs       m_gpio_regs;
	intc_regs       m_intc_regs;
	ppc_regs        m_ppc_regs;
	dma_regs        m_dma_regs[6];
	uint8_t         m_dma_active_mask;

	required_device<sa1110_cpu_device> m_maincpu;
	required_device<input_merger_device> m_uart3_irqs;
	required_device<input_merger_device> m_mcp_irqs;
	optional_device<ucb1200_device> m_codec;

	devcb_write_line::array<28> m_gpio_out;
	devcb_write16 m_ssp_out;
	devcb_write_line m_uart3_tx_out;
};

DECLARE_DEVICE_TYPE(SA1110_PERIPHERALS, sa1110_periphs_device)

#endif // MAME_MACHINE_SA1110
