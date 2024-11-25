// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Intel XScale SA1110 peripheral emulation

***************************************************************************/

#ifndef MAME_MACHINE_SA1110
#define MAME_MACHINE_SA1110

#pragma once

#include "cpu/arm7/arm7.h"

#include "machine/input_merger.h"
#include "machine/ucb1200.h"

#include "diserial.h"

class sa1110_periphs_device : public device_t, public device_serial_interface
{
public:
	template <typename T>
	sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
		: sa1110_periphs_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_codec_tag(T &&tag) { m_codec.set_tag(std::forward<T>(tag)); }

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	template <unsigned Line> void gpio_in(int state) { gpio_in(Line, state); }
	template <unsigned Line> auto gpio_out() { return m_gpio_out[Line].bind(); }

	void ssp_in(u16 data) { ssp_rx_fifo_push(data); }
	auto ssp_out() { return m_ssp_out.bind(); }

	auto uart3_tx_out() { return m_uart3_tx_out.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	static constexpr u32 INTERNAL_OSC = 3686400;

	TIMER_CALLBACK_MEMBER(icp_rx_callback);
	TIMER_CALLBACK_MEMBER(icp_tx_callback);
	TIMER_CALLBACK_MEMBER(hssp_rx_callback);
	TIMER_CALLBACK_MEMBER(hssp_tx_callback);
	void icp_uart_set_receiver_enabled(bool enabled);
	void icp_uart_set_transmitter_enabled(bool enabled);
	void icp_uart_set_receive_irq_enabled(bool enabled);
	void icp_uart_set_transmit_irq_enabled(bool enabled);
	u8 icp_uart_read_receive_fifo();
	void icp_uart_write_transmit_fifo(u8 data);
	u16 icp_hssp_read_receive_fifo();
	void icp_hssp_write_transmit_fifo(u8 data);
	void icp_uart_set_receiver_idle();
	void icp_uart_begin_of_break();
	void icp_uart_end_of_break();

	void uart3_irq_callback(int state);
	void uart_recalculate_divisor();
	void uart_update_eif_status();
	void uart_write_receive_fifo(u16 data_and_flags);
	u8 uart_read_receive_fifo();
	void uart_write_transmit_fifo(u8 data);
	void uart_check_rx_fifo_service();
	void uart_check_tx_fifo_service();
	void uart_set_receiver_idle();
	void uart_begin_of_break();
	void uart_end_of_break();
	void uart_set_receiver_enabled(bool enabled);
	void uart_set_transmitter_enabled(bool enabled);
	void uart_set_receive_irq_enabled(bool enabled);
	void uart_set_transmit_irq_enabled(bool enabled);

	void mcp_irq_callback(int state);
	TIMER_CALLBACK_MEMBER(mcp_audio_tx_callback);
	TIMER_CALLBACK_MEMBER(mcp_telecom_tx_callback);
	void mcp_update_sample_rate();
	void mcp_set_enabled(bool enabled);
	u16 mcp_read_audio_fifo();
	u16 mcp_read_telecom_fifo();
	attotime mcp_get_audio_frame_rate();
	attotime mcp_get_telecom_frame_rate();
	void mcp_audio_tx_fifo_push(const u16 value);
	void mcp_telecom_tx_fifo_push(const u16 value);
	void mcp_codec_read(offs_t offset);
	void mcp_codec_write(offs_t offset, u16 data);

	TIMER_CALLBACK_MEMBER(ssp_rx_callback);
	TIMER_CALLBACK_MEMBER(ssp_tx_callback);
	void ssp_update_enable_state();
	void ssp_update_rx_level();
	void ssp_update_tx_level();
	void ssp_rx_fifo_push(const u16 data);
	void ssp_tx_fifo_push(const u16 data);
	u16 ssp_rx_fifo_pop();

	TIMER_CALLBACK_MEMBER(ostimer_tick_cb);
	void ostimer_update_count();
	void ostimer_update_match_timer(int channel);

	TIMER_CALLBACK_MEMBER(rtc_tick_cb);

	void gpio_in(const u32 line, const int state);
	void gpio_update_interrupts(const u32 changed_mask);
	void gpio_update_direction(const u32 old_gpdr);
	void gpio_update_outputs(const u32 old_latch, const u32 changed);
	void gpio_update_alternate_pins(const u32 changed_mask);

	void set_irq_line(u32 line, int state);
	void update_interrupts();

	void dma_set_control_bits(int channel, u32 bits);
	void dma_clear_control_bits(int channel, u32 bits);

	u32 udc_udccr_r(offs_t offset, u32 mem_mask);
	void udc_udccr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udcar_r(offs_t offset, u32 mem_mask);
	void udc_udcar_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udcomp_r(offs_t offset, u32 mem_mask);
	void udc_udcomp_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udcimp_r(offs_t offset, u32 mem_mask);
	void udc_udcimp_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udccs0_r(offs_t offset, u32 mem_mask);
	void udc_udccs0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udccs1_r(offs_t offset, u32 mem_mask);
	void udc_udccs1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udccs2_r(offs_t offset, u32 mem_mask);
	void udc_udccs2_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udcd0_r(offs_t offset, u32 mem_mask);
	void udc_udcd0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udcwc_r(offs_t offset, u32 mem_mask);
	void udc_udcwc_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udcdr_r(offs_t offset, u32 mem_mask);
	void udc_udcdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 udc_udcsr_r(offs_t offset, u32 mem_mask);
	void udc_udcsr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 icp_utcr0_r(offs_t offset, u32 mem_mask);
	void icp_utcr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_utcr1_r(offs_t offset, u32 mem_mask);
	void icp_utcr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_utcr2_r(offs_t offset, u32 mem_mask);
	void icp_utcr2_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_utcr3_r(offs_t offset, u32 mem_mask);
	void icp_utcr3_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_utcr4_r(offs_t offset, u32 mem_mask);
	void icp_utcr4_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_utdr_r(offs_t offset, u32 mem_mask);
	void icp_utdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_utsr0_r(offs_t offset, u32 mem_mask);
	void icp_utsr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_utsr1_r(offs_t offset, u32 mem_mask);
	u32 icp_hscr0_r(offs_t offset, u32 mem_mask);
	void icp_hscr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_hscr1_r(offs_t offset, u32 mem_mask);
	void icp_hscr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_hsdr_r(offs_t offset, u32 mem_mask);
	void icp_hsdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_hssr0_r(offs_t offset, u32 mem_mask);
	void icp_hssr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 icp_hssr1_r(offs_t offset, u32 mem_mask);
	void icp_hssr1_w(offs_t offset, u32 data, u32 mem_mask);

	u32 uart3_utcr0_r(offs_t offset, u32 mem_mask);
	void uart3_utcr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 uart3_utcr1_r(offs_t offset, u32 mem_mask);
	void uart3_utcr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 uart3_utcr2_r(offs_t offset, u32 mem_mask);
	void uart3_utcr2_w(offs_t offset, u32 data, u32 mem_mask);
	u32 uart3_utcr3_r(offs_t offset, u32 mem_mask);
	void uart3_utcr3_w(offs_t offset, u32 data, u32 mem_mask);
	u32 uart3_utdr_r(offs_t offset, u32 mem_mask);
	void uart3_utdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 uart3_utsr0_r(offs_t offset, u32 mem_mask);
	void uart3_utsr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 uart3_utsr1_r(offs_t offset, u32 mem_mask);

	u32 mcp_mccr0_r(offs_t offset, u32 mem_mask);
	void mcp_mccr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mcp_mcdr0_r(offs_t offset, u32 mem_mask);
	void mcp_mcdr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mcp_mcdr1_r(offs_t offset, u32 mem_mask);
	void mcp_mcdr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mcp_mcdr2_r(offs_t offset, u32 mem_mask);
	void mcp_mcdr2_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mcp_mcsr_r(offs_t offset, u32 mem_mask);
	void mcp_mcsr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 ssp_sscr0_r(offs_t offset, u32 mem_mask);
	void ssp_sscr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ssp_sscr1_r(offs_t offset, u32 mem_mask);
	void ssp_sscr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ssp_ssdr_r(offs_t offset, u32 mem_mask);
	void ssp_ssdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ssp_sssr_r(offs_t offset, u32 mem_mask);
	void ssp_sssr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 tmr_osmr0_r(offs_t offset, u32 mem_mask);
	void tmr_osmr0_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_osmr1_r(offs_t offset, u32 mem_mask);
	void tmr_osmr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_osmr2_r(offs_t offset, u32 mem_mask);
	void tmr_osmr2_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_osmr3_r(offs_t offset, u32 mem_mask);
	void tmr_osmr3_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_oscr_r(offs_t offset, u32 mem_mask);
	void tmr_oscr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_ossr_r(offs_t offset, u32 mem_mask);
	void tmr_ossr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_ower_r(offs_t offset, u32 mem_mask);
	void tmr_ower_w(offs_t offset, u32 data, u32 mem_mask);
	u32 tmr_oier_r(offs_t offset, u32 mem_mask);
	void tmr_oier_w(offs_t offset, u32 data, u32 mem_mask);

	u32 rtc_rtar_r(offs_t offset, u32 mem_mask);
	void rtc_rtar_w(offs_t offset, u32 data, u32 mem_mask);
	u32 rtc_rcnr_r(offs_t offset, u32 mem_mask);
	void rtc_rcnr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 rtc_rttr_r(offs_t offset, u32 mem_mask);
	void rtc_rttr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 rtc_rtsr_r(offs_t offset, u32 mem_mask);
	void rtc_rtsr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 pwr_pmcr_r(offs_t offset, u32 mem_mask);
	void pwr_pmcr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pssr_r(offs_t offset, u32 mem_mask);
	void pwr_pssr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pspr_r(offs_t offset, u32 mem_mask);
	void pwr_pspr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pwer_r(offs_t offset, u32 mem_mask);
	void pwr_pwer_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pcfr_r(offs_t offset, u32 mem_mask);
	void pwr_pcfr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_ppcr_r(offs_t offset, u32 mem_mask);
	void pwr_ppcr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_pgsr_r(offs_t offset, u32 mem_mask);
	void pwr_pgsr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pwr_posr_r(offs_t offset, u32 mem_mask);
	void pwr_posr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 rst_rsrr_r(offs_t offset, u32 mem_mask);
	void rst_rsrr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 rst_rcsr_r(offs_t offset, u32 mem_mask);
	void rst_rcsr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 gpio_gplr_r(offs_t offset, u32 mem_mask);
	void gpio_gplr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 gpio_gpdr_r(offs_t offset, u32 mem_mask);
	void gpio_gpdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 gpio_gpsr_r(offs_t offset, u32 mem_mask);
	void gpio_gpsr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 gpio_gpcr_r(offs_t offset, u32 mem_mask);
	void gpio_gpcr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 gpio_grer_r(offs_t offset, u32 mem_mask);
	void gpio_grer_w(offs_t offset, u32 data, u32 mem_mask);
	u32 gpio_gfer_r(offs_t offset, u32 mem_mask);
	void gpio_gfer_w(offs_t offset, u32 data, u32 mem_mask);
	u32 gpio_gedr_r(offs_t offset, u32 mem_mask);
	void gpio_gedr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 gpio_gafr_r(offs_t offset, u32 mem_mask);
	void gpio_gafr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 intc_icip_r(offs_t offset, u32 mem_mask);
	void intc_icip_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_icmr_r(offs_t offset, u32 mem_mask);
	void intc_icmr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_iclr_r(offs_t offset, u32 mem_mask);
	void intc_iclr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_icfp_r(offs_t offset, u32 mem_mask);
	void intc_icfp_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_icpr_r(offs_t offset, u32 mem_mask);
	void intc_icpr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 intc_iccr_r(offs_t offset, u32 mem_mask);
	void intc_iccr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 ppc_ppdr_r(offs_t offset, u32 mem_mask);
	void ppc_ppdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ppc_ppsr_r(offs_t offset, u32 mem_mask);
	void ppc_ppsr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ppc_ppar_r(offs_t offset, u32 mem_mask);
	void ppc_ppar_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ppc_psdr_r(offs_t offset, u32 mem_mask);
	void ppc_psdr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ppc_ppfr_r(offs_t offset, u32 mem_mask);
	void ppc_ppfr_w(offs_t offset, u32 data, u32 mem_mask);

	template <int Channel> u32 dma_ddar_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_ddar_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Channel> u32 dma_dssr_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_dssr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Channel> u32 dma_dcsr_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_dcsr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Channel> u32 dma_dsr_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_dsr_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Channel> u32 dma_dbsa_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_dbsa_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Channel> u32 dma_dbta_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_dbta_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Channel> u32 dma_dbsb_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_dbsb_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Channel> u32 dma_dbtb_r(offs_t offset, u32 mem_mask);
	template <int Channel> void dma_dbtb_w(offs_t offset, u32 data, u32 mem_mask);

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
	enum : u32
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
	enum : u32
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
		u32 udccr;
		u32 udcar;
		u32 udcomp;
		u32 udcimp;
		u32 udccs0;
		u32 udccs1;
		u32 udccs2;
		u32 udcwc;
		u32 udcsr;
	};

	struct uart_regs
	{
		u32      utcr[4];
		u32      utsr0;
		u32      utsr1;

		u16      rx_fifo[12];
		int      rx_fifo_read_idx;
		int      rx_fifo_write_idx;
		int      rx_fifo_count;

		u8       tx_fifo[8];
		int      tx_fifo_read_idx;
		int      tx_fifo_write_idx;
		int      tx_fifo_count;

		bool     rx_break_interlock;
	};

	struct hssp_regs
	{
		u32        hscr0;
		u32        hscr1;
		u32        hssr0;
		u32        hssr1;

		u16        rx_fifo[8];
		int        rx_fifo_read_idx;
		int        rx_fifo_write_idx;
		int        rx_fifo_count;
		emu_timer *rx_timer;

		u16        tx_fifo[8];
		int        tx_fifo_read_idx;
		int        tx_fifo_write_idx;
		int        tx_fifo_count;
		emu_timer *tx_timer;
	};

	struct icp_regs
	{
		uart_regs  uart;
		u32        utcr4;
		emu_timer *uart_rx_timer;
		emu_timer *uart_tx_timer;

		hssp_regs  hssp;
	};

	struct mcp_regs
	{
		u32        mccr0;
		u32        mccr1;
		u32        mcdr2;
		u32        mcsr;

		u16        audio_rx_fifo[8];
		int        audio_rx_fifo_read_idx;
		int        audio_rx_fifo_write_idx;
		int        audio_rx_fifo_count;

		u16        audio_tx_fifo[8];
		int        audio_tx_fifo_read_idx;
		int        audio_tx_fifo_write_idx;
		int        audio_tx_fifo_count;
		emu_timer *audio_tx_timer;

		u16        telecom_rx_fifo[8];
		int        telecom_rx_fifo_read_idx;
		int        telecom_rx_fifo_write_idx;
		int        telecom_rx_fifo_count;

		u16        telecom_tx_fifo[8];
		int        telecom_tx_fifo_read_idx;
		int        telecom_tx_fifo_write_idx;
		int        telecom_tx_fifo_count;
		emu_timer *telecom_tx_timer;
	};

	struct ssp_regs
	{
		u32        sscr0;
		u32        sscr1;
		u32        sssr;

		u16        rx_fifo[8];
		int        rx_fifo_read_idx;
		int        rx_fifo_write_idx;
		int        rx_fifo_count;
		emu_timer *rx_timer;

		u16        tx_fifo[8];
		int        tx_fifo_read_idx;
		int        tx_fifo_write_idx;
		int        tx_fifo_count;
		emu_timer *tx_timer;
	};

	struct ostimer_regs
	{
		u32        osmr[4];
		u32        oscr;
		u32        ossr;
		u32        ower;
		u32        oier;

		emu_timer *timer[4];
		attotime   last_count_sync;
	};

	struct rtc_regs
	{
		u32 rtar;
		u32 rcnr;
		u32 rttr;
		u32 rtsr;

		emu_timer *tick_timer;
	};

	struct power_regs
	{
		u32 pmcr;
		u32 pssr;
		u32 pspr;
		u32 pwer;
		u32 pcfr;
		u32 ppcr;
		u32 pgsr;
		u32 posr;
	};

	struct gpio_regs
	{
		u32 gplr;
		u32 gpdr;
		u32 grer;
		u32 gfer;
		u32 gedr;
		u32 gafr;

		u32 any_edge_mask;

		u32 output_latch;
		u32 input_latch;
		u32 alt_output_latch;
		u32 alt_input_latch;
	};

	struct intc_regs
	{
		u32 icip;
		u32 icmr;
		u32 iclr;
		u32 iccr;
		u32 icfp;
		u32 icpr;
	};

	struct ppc_regs
	{
		u32 ppdr;
		u32 ppsr_out;
		u32 ppsr_in;
		u32 ppsr;
		u32 ppar;
		u32 psdr;
		u32 ppfr;
	};

	struct dma_regs
	{
		u32 ddar;
		u32 dsr;
		u32 dbs[2];
		u32 dbt[2];
	};

	udc_regs        m_udc_regs;
	uart_regs       m_uart_regs;
	icp_regs        m_icp_regs;
	mcp_regs        m_mcp_regs;
	ssp_regs        m_ssp_regs;
	ostimer_regs    m_ostmr_regs;
	rtc_regs        m_rtc_regs;
	power_regs      m_power_regs;
	u32             m_rcsr;
	gpio_regs       m_gpio_regs;
	intc_regs       m_intc_regs;
	ppc_regs        m_ppc_regs;
	dma_regs        m_dma_regs[6];
	u8              m_dma_active_mask;

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
