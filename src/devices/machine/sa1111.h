// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Intel SA1111 Microprocessor Companion Chip skeleton

***************************************************************************/

#ifndef MAME_MACHINE_SA1111
#define MAME_MACHINE_SA1111

#pragma once

#include "cpu/arm7/arm7.h"

class sa1111_device : public device_t
{
public:
	template <typename T>
	sa1111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: sa1111_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	sa1111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_audio_codec_tag(T &&tag) { m_audio_codec.set_tag(std::forward<T>(tag)); }

	auto irq_out() { return m_irq_out.bind(); }

	template <int Line> void pa_in(int state) { gpio_in(0 + Line, state); }
	template <int Line> void pb_in(int state) { gpio_in(8 + Line, state); }
	template <int Line> void pc_in(int state) { gpio_in(16 + Line, state); }
	template <int Line> auto pa_out() { return m_gpio_out[0 + Line].bind(); }
	template <int Line> auto pb_out() { return m_gpio_out[8 + Line].bind(); }
	template <int Line> auto pc_out() { return m_gpio_out[16 + Line].bind(); }

	void ssp_in(uint16_t data) { ssp_rx_fifo_push(data); }
	auto ssp_out() { return m_ssp_out.bind(); }

	auto l3_addr_out() { return m_l3_addr_out.bind(); }
	auto l3_data_out() { return m_l3_data_out.bind(); }
	auto i2s_out() { return m_i2s_out.bind(); }

	void l3wd_in(int state);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void set_irq_line(uint32_t line, int state);
	void update_interrupts();

	TIMER_CALLBACK_MEMBER(ssp_rx_callback);
	TIMER_CALLBACK_MEMBER(ssp_tx_callback);

	TIMER_CALLBACK_MEMBER(audio_rx_dma_callback);
	TIMER_CALLBACK_MEMBER(audio_rx_callback);
	TIMER_CALLBACK_MEMBER(audio_tx_dma_callback);
	TIMER_CALLBACK_MEMBER(audio_tx_callback);
	void audio_update_mode();
	void audio_clear_interrupts();
	void audio_set_enabled(bool enabled);
	void audio_controller_reset();
	void audio_set_tx_dma_enabled(bool enabled);
	void audio_set_rx_dma_enabled(bool enabled);
	void audio_start_tx_dma(const uint32_t buf);
	void audio_start_rx_dma(const uint32_t buf);
	void audio_update_tx_fifo_levels();
	void audio_update_rx_fifo_levels();
	void audio_update_busy_flag();
	void audio_tx_fifo_push(uint32_t data);
	uint32_t audio_tx_fifo_pop();
	void audio_rx_fifo_push(uint32_t data);
	uint32_t audio_rx_fifo_pop();

	uint32_t unknown_r(offs_t offset, uint32_t mem_mask);
	void unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t skcr_r(offs_t offset, uint32_t mem_mask);
	uint32_t smcr_r(offs_t offset, uint32_t mem_mask);
	uint32_t skid_r(offs_t offset, uint32_t mem_mask);
	void skcr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void smcr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t skpcr_r(offs_t offset, uint32_t mem_mask);
	uint32_t skcdr_r(offs_t offset, uint32_t mem_mask);
	uint32_t skaud_r(offs_t offset, uint32_t mem_mask);
	uint32_t skpmc_r(offs_t offset, uint32_t mem_mask);
	uint32_t skptc_r(offs_t offset, uint32_t mem_mask);
	uint32_t skpen0_r(offs_t offset, uint32_t mem_mask);
	uint32_t skpwm0_r(offs_t offset, uint32_t mem_mask);
	uint32_t skpen1_r(offs_t offset, uint32_t mem_mask);
	uint32_t skpwm1_r(offs_t offset, uint32_t mem_mask);
	void skpcr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skcdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skaud_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skpmc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skptc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skpen0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skpwm0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skpen1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void skpwm1_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t ohci_r(offs_t offset, uint32_t mem_mask);
	uint32_t usb_status_r(offs_t offset, uint32_t mem_mask);
	uint32_t usb_reset_r(offs_t offset, uint32_t mem_mask);
	uint32_t usb_fifo_r(offs_t offset, uint32_t mem_mask);
	void ohci_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void usb_reset_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void usb_int_test_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t sacr0_r(offs_t offset, uint32_t mem_mask);
	uint32_t sacr1_r(offs_t offset, uint32_t mem_mask);
	uint32_t sacr2_r(offs_t offset, uint32_t mem_mask);
	uint32_t sasr0_r(offs_t offset, uint32_t mem_mask);
	uint32_t sasr1_r(offs_t offset, uint32_t mem_mask);
	uint32_t l3car_r(offs_t offset, uint32_t mem_mask);
	uint32_t l3cdr_r(offs_t offset, uint32_t mem_mask);
	uint32_t accar_r(offs_t offset, uint32_t mem_mask);
	uint32_t accdr_r(offs_t offset, uint32_t mem_mask);
	uint32_t acsar_r(offs_t offset, uint32_t mem_mask);
	uint32_t acsdr_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadtcs_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadtsa_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadtca_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadtsb_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadtcb_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadrcs_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadrsa_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadrca_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadrsb_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadrcb_r(offs_t offset, uint32_t mem_mask);
	uint32_t sadr_r(offs_t offset, uint32_t mem_mask);
	void sacr0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sacr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sacr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sascr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void l3car_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void l3cdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void accar_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void accdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void acsar_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void acsdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadtcs_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadtsa_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadtca_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadtsb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadtcb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadrcs_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadrsa_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadrca_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadrsb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadrcb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void saitr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sadr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t sspcr0_r(offs_t offset, uint32_t mem_mask);
	uint32_t sspcr1_r(offs_t offset, uint32_t mem_mask);
	uint32_t sspsr_r(offs_t offset, uint32_t mem_mask);
	uint32_t sspdr_r(offs_t offset, uint32_t mem_mask);
	void sspcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sspcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sspsr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sspitr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void sspdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t track_kbdcr_r(offs_t offset, uint32_t mem_mask);
	uint32_t track_kbdstat_r(offs_t offset, uint32_t mem_mask);
	uint32_t track_kbddata_r(offs_t offset, uint32_t mem_mask);
	uint32_t track_kbdclkdiv_r(offs_t offset, uint32_t mem_mask);
	uint32_t track_kbdprecnt_r(offs_t offset, uint32_t mem_mask);
	void track_kbdcr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void track_kbdstat_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void track_kbddata_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void track_kbdclkdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void track_kbdprecnt_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void track_kbditr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t mouse_kbdcr_r(offs_t offset, uint32_t mem_mask);
	uint32_t mouse_kbdstat_r(offs_t offset, uint32_t mem_mask);
	uint32_t mouse_kbddata_r(offs_t offset, uint32_t mem_mask);
	uint32_t mouse_kbdclkdiv_r(offs_t offset, uint32_t mem_mask);
	uint32_t mouse_kbdprecnt_r(offs_t offset, uint32_t mem_mask);
	void mouse_kbdcr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void mouse_kbdstat_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void mouse_kbddata_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void mouse_kbdclkdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void mouse_kbdprecnt_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void mouse_kbditr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	template <int Block> uint32_t ddr_r(offs_t offset, uint32_t mem_mask);
	template <int Block> uint32_t drr_r(offs_t offset, uint32_t mem_mask);
	template <int Block> uint32_t sdr_r(offs_t offset, uint32_t mem_mask);
	template <int Block> uint32_t ssr_r(offs_t offset, uint32_t mem_mask);
	template <int Block> void ddr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Block> void dwr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Block> void sdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Block> void ssr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	template <int Set> uint32_t inttest_r(offs_t offset, uint32_t mem_mask);
	template <int Set> uint32_t inten_r(offs_t offset, uint32_t mem_mask);
	template <int Set> uint32_t intpol_r(offs_t offset, uint32_t mem_mask);
	uint32_t inttstsel_r(offs_t offset, uint32_t mem_mask);
	template <int Set> uint32_t intstat_r(offs_t offset, uint32_t mem_mask);
	template <int Set> uint32_t wake_en_r(offs_t offset, uint32_t mem_mask);
	template <int Set> uint32_t wake_pol_r(offs_t offset, uint32_t mem_mask);
	template <int Set> void inttest_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Set> void inten_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Set> void intpol_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void inttstsel_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Set> void intclr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Set> void intset_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Set> void wake_en_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Set> void wake_pol_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t pccr_r(offs_t offset, uint32_t mem_mask);
	uint32_t pcssr_r(offs_t offset, uint32_t mem_mask);
	uint32_t pcsr_r(offs_t offset, uint32_t mem_mask);
	void pccr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void pcssr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void ssp_update_enable_state();
	void ssp_update_rx_level();
	void ssp_update_tx_level();
	void ssp_rx_fifo_push(const uint16_t data);
	void ssp_tx_fifo_push(const uint16_t data);
	uint16_t ssp_rx_fifo_pop();

	void gpio_in(const uint32_t line, const int state);
	void gpio_update_direction(const uint32_t block, const uint32_t old_dir);
	void gpio_update_outputs(const uint32_t block, const uint32_t changed);

	// register contents
	enum : uint32_t
	{
		SKCR_PLLB_BIT       = 0,
		SKCR_RCLK_BIT       = 1,
		SKCR_SLEEP_BIT      = 2,
		SKCR_DOZE_BIT       = 3,
		SKCR_VCO_BIT        = 4,
		SKCR_SCANTST_BIT    = 5,
		SKCR_CLKTST_BIT     = 6,
		SKCR_RDY_BIT        = 7,
		SKCR_SACMDSL_BIT    = 8,
		SKCR_OPPC_BIT       = 9,
		SKCR_PII_BIT        = 10,
		SKCR_UIOTEN_BIT     = 11,
		SKCR_OEEN_BIT       = 12,

		SMCR_DTIM_BIT       = 0,
		SMCR_MBGE_BIT       = 1,
		SMCR_DRAC_BIT       = 2,
		SMCR_DRAC_MASK      = 0x0000001c,
		SMCR_CLAT_BIT       = 5,

		SKPCR_UCLKE_BIT     = 0,
		SKPCR_ACCLKE_BIT    = 1,
		SKPCR_ISCLKE_BIT    = 2,
		SKPCR_L3CLKE_BIT    = 3,
		SKPCR_SCLKE_BIT     = 4,
		SKPCR_PMCLKE_BIT    = 5,
		SKPCR_PTCLKE_BIT    = 6,
		SKPCR_DCLKE_BIT     = 7,
		SKPCR_PWMCLKE_BIT   = 8,

		SKCDR_FBD_BIT       = 0,
		SKCDR_FBD_MASK      = 0x0000007f,
		SKCDR_IPD_BIT       = 7,
		SKCDR_IPD_MASK      = 0x00000f80,
		SKCDR_OPD_BIT       = 12,
		SKCDR_OPD_MASK      = 0x00003000,
		SKCDR_OPS_BIT       = 14,

		SKAUD_ACD_BIT       = 0,
		SKAUD_ACD_MASK      = 0x0000007f,

		SKPMC_PMCD_BIT      = 0,
		SKPMC_PMCD_MASK     = 0x000000ff,

		SKPTC_PTCD_BIT      = 0,
		SKPTC_PTCD_MASK     = 0x000000ff,

		SKPEN0_PWM0EN_BIT   = 0,

		SKPWM0_PWM0CK_BIT   = 0,
		SKPWM0_PWM0CK_MASK  = 0x000000ff,

		SKPEN1_PWM1EN_BIT   = 0,

		SKPWM1_PWM1CK_BIT   = 0,
		SKPWM1_PWM1CK_MASK  = 0x000000ff,

		USBSTAT_IHRW_BIT    = 7,
		USBSTAT_IHBA_BIT    = 8,
		USBSTAT_NHT_BIT     = 9,
		USBSTAT_NHFCT_BIT   = 10,
		USBSTAT_UPRT_BIT    = 11,

		USBRST_FIR_BIT      = 0,
		USBRST_FHR_BIT      = 1,
		USBRST_CGR_BIT      = 2,
		USBRST_SSDC_BIT     = 3,
		USBRST_UIT_BIT      = 4,
		USBRST_SSE_BIT      = 5,
		USBRST_PSPL_BIT     = 6,
		USBRST_PCPL_BIT     = 7,

		USBINT_IHRWT_BIT    = 7,
		USBINT_IHBAT_BIT    = 8,
		USBINT_NHT_BIT      = 9,
		USBINT_NHFCT_BIT    = 10,
		USBINT_UPRT_BIT     = 11,

		SACR0_ENB_BIT       = 0,
		SACR0_BCKD_BIT      = 2,
		SACR0_RST_BIT       = 3,
		SACR0_TFTH_BIT      = 8,
		SACR0_TFTH_MASK     = 0x00000f00,
		SACR0_RFTH_BIT      = 12,
		SACR0_RFTH_MASK     = 0x0000f000,

		SACR1_AMSL_BIT      = 0,
		SACR1_L3EN_BIT      = 1,
		SACR1_L3MB_BIT      = 2,
		SACR1_DREC_BIT      = 3,
		SACR1_DRPL_BIT      = 4,
		SACR1_ENLBF_BIT     = 5,

		SACR2_TS3V_BIT      = 0,
		SACR2_TS4V_BIT      = 1,
		SACR2_WKUP_BIT      = 2,
		SACR2_DREC_BIT      = 3,
		SACR2_DRPL_BIT      = 4,
		SACR2_ENLBF_BIT     = 5,
		SACR2_RESET_BIT     = 6,

		SASCR_TUR_BIT       = 5,
		SASCR_ROR_BIT       = 6,
		SASCR_DTS_BIT       = 16,
		SASCR_RDD_BIT       = 17,
		SASCR_STO_BIT       = 18,

		SASR_TNF_BIT        = 0,
		SASR_RNE_BIT        = 1,
		SASR_BSY_BIT        = 2,
		SASR_TFS_BIT        = 3,
		SASR_RFS_BIT        = 4,
		SASR_TUR_BIT        = 5,
		SASR_ROR_BIT        = 6,
		SASR_TFL_BIT        = 8,
		SASR_TFL_MASK       = 0x00000f00,
		SASR_RFL_BIT        = 12,
		SASR_RFL_MASK       = 0x0000f000,
		SASR_SEND_BIT       = 16,
		SASR_RECV_BIT       = 17,

		SASR0_L3WD_BIT      = 16,
		SASR0_L3RD_BIT      = 17,

		SASR1_CADT_BIT      = 16,
		SASR1_SADR_BIT      = 17,
		SASR1_RSTO_BIT      = 18,
		SASR1_CLPM_BIT      = 19,
		SASR1_CRDY_BIT      = 20,
		SASR1_RS3V_BIT      = 21,
		SASR1_RS4V_BIT      = 22,

		SADTCS_TDEN_BIT     = 0,
		SADTCS_TDBDA_BIT    = 3,
		SADTCS_TDSTA_BIT    = 4,
		SADTCS_TDBDB_BIT    = 5,
		SADTCS_TDSTB_BIT    = 6,
		SADTCS_TBIU_BIT     = 7,

		SADRCS_RDEN_BIT     = 0,
		SADRCS_RDBDA_BIT    = 3,
		SADRCS_RDSTA_BIT    = 4,
		SADRCS_RDBDB_BIT    = 5,
		SADRCS_RDSTB_BIT    = 6,
		SADRCS_RBIU_BIT     = 7,

		SAITR_TFS_BIT       = 0,
		SAITR_RFS_BIT       = 1,
		SAITR_TUR_BIT       = 2,
		SAITR_ROR_BIT       = 3,
		SAITR_CADT_BIT      = 4,
		SAITR_SADR_BIT      = 5,
		SAITR_RSTO_BIT      = 6,
		SAITR_TDBDA_BIT     = 8,
		SAITR_TDBDB_BIT     = 9,
		SAITR_RDBDA_BIT     = 10,
		SAITR_RDBDB_BIT     = 11,

		SSPCR0_DSS_BIT      = 0,
		SSPCR0_DSS_MASK     = 0x0000000f,
		SSPCR0_FRF_BIT      = 4,
		SSPCR0_FRF_MASK     = 0x00000030,
		SSPCR0_FRF_SPI      = 0,
		SSPCR0_FRF_SSP      = 1,
		SSPCR0_FRF_MWIRE    = 2,
		SSPCR0_FRF_RESV     = 3,
		SSPCR0_SSPEN_BIT    = 7,
		SSPCR0_SCR_BIT      = 8,
		SSPCR0_SCR_MASK     = 0x0000ff00,

		SSPCR1_LBM_BIT      = 2,
		SSPCR1_SPO_BIT      = 3,
		SSPCR1_SPH_BIT      = 4,
		SSPCR1_TFT_BIT      = 7,
		SSPCR1_TFT_MASK     = 0x00000780,
		SSPCR1_RFT_BIT      = 11,
		SSPCR1_RFT_MASK     = 0x00007800,

		SSPSR_TNF_BIT       = 2,
		SSPSR_RNE_BIT       = 3,
		SSPSR_BSY_BIT       = 4,
		SSPSR_TFS_BIT       = 5,
		SSPSR_RFS_BIT       = 6,
		SSPSR_ROR_BIT       = 7,
		SSPSR_TFL_BIT       = 8,
		SSPSR_TFL_MASK      = 0x00000f00,
		SSPSR_RFL_BIT       = 12,
		SSPSR_RFL_MASK      = 0x0000f000,

		SSPITR_TFS_BIT      = 2,
		SSPITR_RFS_BIT      = 3,
		SSPITR_ROR_BIT      = 4,

		KBDCR_FKC_BIT       = 0,
		KBDCR_FKD_BIT       = 1,
		KBDCR_ENA_BIT       = 3,

		KBDSTAT_KBC_BIT     = 0,
		KBDSTAT_KBD_BIT     = 1,
		KBDSTAT_RXP_BIT     = 2,
		KBDSTAT_ENA_BIT     = 3,
		KBDSTAT_RXB_BIT     = 4,
		KBDSTAT_RXF_BIT     = 5,
		KBDSTAT_TXB_BIT     = 6,
		KBDSTAT_TXE_BIT     = 7,
		KBDSTAT_STP_BIT     = 8,

		KBDCLKDIV_DV_BIT    = 0,
		KBDCLKDIV_DV_MASK   = 0x00000003,

		PCSR_S0R_BIT        = 0,
		PCSR_S1R_BIT        = 1,
		PCSR_S0CD_BIT       = 2,
		PCSR_S1CD_BIT       = 3,
		PCSR_S0VS1_BIT      = 4,
		PCSR_S0VS2_BIT      = 5,
		PCSR_S1VS1_BIT      = 6,
		PCSR_S1VS2_BIT      = 7,
		PCSR_S0WP_BIT       = 8,
		PCSR_S1WP_BIT       = 9,
		PCSR_S0BVD1_BIT     = 10,
		PCSR_S0BVD2_BIT     = 11,
		PCSR_S1BVD1_BIT     = 12,
		PCSR_S1BVD2_BIT     = 13,

		PCCR_S0RST_BIT      = 0,
		PCCR_S1RST_BIT      = 1,
		PCCR_S0FLT_BIT      = 2,
		PCCR_S1FLT_BIT      = 3,
		PCCR_S0PWEN_BIT     = 4,
		PCCR_S1PWEN_BIT     = 5,
		PCCR_S0PSE_BIT      = 6,
		PCCR_S1PSE_BIT      = 7,

		PCSSR_S0SLP_BIT     = 0,
		PCSSR_S1SLP_BIT     = 1
	};

	// interrupt lines
	enum : uint32_t
	{
		INT_GPA0            = 0,
		INT_GPA1            = 1,
		INT_GPA2            = 2,
		INT_GPA3            = 3,
		INT_GPB0            = 4,
		INT_GPB1            = 5,
		INT_GPB2            = 6,
		INT_GPB3            = 7,
		INT_GPB4            = 8,
		INT_GPB5            = 9,
		INT_GPC0            = 10,
		INT_GPC1            = 11,
		INT_GPC2            = 12,
		INT_GPC3            = 13,
		INT_GPC4            = 14,
		INT_GPC5            = 15,
		INT_GPC6            = 16,
		INT_GPC7            = 17,
		INT_MSTX            = 18,
		INT_MSRX            = 19,
		INT_MSERR           = 20,
		INT_TPTX            = 21,
		INT_TPRX            = 22,
		INT_TPERR           = 23,
		INT_SSPTX           = 24,
		INT_SSPRX           = 25,
		INT_SSPROR          = 26,
		INT_AUDTXA          = 32,
		INT_AUDRXA          = 33,
		INT_AUDTXB          = 34,
		INT_AUDRXB          = 35,
		INT_AUDTFS          = 36,
		INT_AUDRFS          = 37,
		INT_AUDTUR          = 38,
		INT_AUDROR          = 39,
		INT_AUDDTS          = 40,
		INT_AUDRDD          = 41,
		INT_AUDSTO          = 42,
		INT_USBPWR          = 43,
		INT_USBHCIM         = 44,
		INT_USBHCIBUF       = 45,
		INT_USBHCIWAKE      = 46,
		INT_USBHCIMFC       = 47,
		INT_USBRESUME       = 48,
		INT_S0RDY           = 49,
		INT_S1RDY           = 50,
		INT_S0CD            = 51,
		INT_S1CD            = 52,
		INT_S0BVD           = 53,
		INT_S1BVD           = 54
	};

	struct sbi_regs
	{
		uint32_t skcr;
		uint32_t smcr;
		uint32_t skid;
	};

	struct sysctrl_regs
	{
		uint32_t skpcr;
		uint32_t skcdr;
		uint32_t skaud;
		uint32_t skpmc;
		uint32_t skptc;
		uint32_t skpen0;
		uint32_t skpwm0;
		uint32_t skpen1;
		uint32_t skpwm1;
	};

	struct usb_regs
	{
		uint32_t ohci[22];
		uint32_t status;
		uint32_t reset;
		uint32_t int_test;
		uint32_t fifo[12];
	};

	struct audio_regs
	{
		uint32_t sacr0;
		uint32_t sacr1;
		uint32_t sacr2;
		uint32_t sasr0;
		uint32_t sasr1;
		uint32_t l3car;
		uint32_t l3cdr;
		uint32_t accar;
		uint32_t accdr;
		uint32_t acsar;
		uint32_t acsdr;
		uint32_t sadtcs;
		uint32_t sadts[2];
		uint32_t sadtc[2];
		uint32_t sadta;
		uint32_t sadtcc;
		uint32_t sadrcs;
		uint32_t sadrs[2];
		uint32_t sadrc[2];
		uint32_t sadra;
		uint32_t sadrcc;
		uint32_t saitr;

		uint32_t rx_fifo[16];
		int rx_fifo_read_idx;
		int rx_fifo_write_idx;
		int rx_fifo_count;
		emu_timer *rx_timer;
		emu_timer *rx_dma_timer;

		uint32_t tx_fifo[16];
		int tx_fifo_read_idx;
		int tx_fifo_write_idx;
		int tx_fifo_count;
		emu_timer *tx_timer;
		emu_timer *tx_dma_timer;
	};

	struct ssp_regs
	{
		uint32_t sspcr0;
		uint32_t sspcr1;
		uint32_t sspsr;
		uint32_t sspitr;

		uint16_t rx_fifo[16];
		int rx_fifo_read_idx;
		int rx_fifo_write_idx;
		int rx_fifo_count;
		emu_timer *rx_timer;

		uint16_t tx_fifo[16];
		int tx_fifo_read_idx;
		int tx_fifo_write_idx;
		int tx_fifo_count;
		emu_timer *tx_timer;
	};

	struct ps2_regs
	{
		uint32_t kbdcr;
		uint32_t kbdstat;
		uint32_t kbddata_tx;
		uint32_t kbddata_rx;
		uint32_t kbdclkdiv;
		uint32_t kbdprecnt;
		uint32_t kbditr;
	};

	struct gpio_regs
	{
		uint32_t ddr[3];
		uint32_t level[3];
		uint32_t sdr[3];
		uint32_t ssr[3];
		uint32_t out_latch[3];
		uint32_t in_latch[3];
	};

	struct intc_regs
	{
		uint32_t inttest[2];
		uint32_t inten[2];
		uint32_t intpol[2];
		uint32_t inttstsel;
		uint32_t intstat[2];
		uint32_t wake_en[2];
		uint32_t wake_pol[2];

		uint32_t intraw[2];
	};

	struct card_regs
	{
		uint32_t pccr;
		uint32_t pcssr;
		uint32_t pcsr;
	};

	sbi_regs m_sbi_regs;
	sysctrl_regs m_sk_regs;
	usb_regs m_usb_regs;
	audio_regs m_audio_regs;
	ssp_regs m_ssp_regs;
	ps2_regs m_track_regs;
	ps2_regs m_mouse_regs;
	gpio_regs m_gpio_regs;
	intc_regs m_intc_regs;
	card_regs m_card_regs;

	required_device<sa1110_cpu_device> m_maincpu;
	optional_device<device_t> m_audio_codec;

	devcb_write_line m_irq_out;
	devcb_write_line::array<24> m_gpio_out;
	devcb_write16 m_ssp_out;
	devcb_write8 m_l3_addr_out;
	devcb_write8 m_l3_data_out;
	devcb_write32 m_i2s_out;
};

DECLARE_DEVICE_TYPE(SA1111, sa1111_device)

#endif // MAME_MACHINE_SA1111
