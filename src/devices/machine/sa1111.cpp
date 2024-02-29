// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Intel SA1111 Microprocessor Companion Chip skeleton

***************************************************************************/

#include "emu.h"
#include "sa1111.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_SBI         (1U << 2)
#define LOG_SK          (1U << 3)
#define LOG_USB         (1U << 4)
#define LOG_AUDIO       (1U << 5)
#define LOG_SSP         (1U << 6)
#define LOG_SSP_HF      (1U << 7)
#define LOG_TRACK       (1U << 8)
#define LOG_MOUSE       (1U << 9)
#define LOG_GPIO        (1U << 10)
#define LOG_INTC        (1U << 11)
#define LOG_CARD        (1U << 12)
#define LOG_AUDIO_DMA   (1U << 13)
#define LOG_ALL         (LOG_UNKNOWN | LOG_SBI | LOG_SK | LOG_USB | LOG_AUDIO | LOG_SSP | LOG_TRACK | LOG_MOUSE | LOG_GPIO | LOG_INTC | LOG_CARD)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SA1111, sa1111_device, "sa1111", "Intel SA1111 Microprocessor Companion Chip")

sa1111_device::sa1111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SA1111, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_audio_codec(*this, finder_base::DUMMY_TAG)
	, m_irq_out(*this)
	, m_gpio_out(*this)
	, m_ssp_out(*this)
	, m_l3_addr_out(*this)
	, m_l3_data_out(*this)
	, m_i2s_out(*this)
{
}

void sa1111_device::map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(sa1111_device::unknown_r), FUNC(sa1111_device::unknown_w));

	map(0x0000, 0x0003).rw(FUNC(sa1111_device::skcr_r), FUNC(sa1111_device::skcr_w));
	map(0x0004, 0x0007).rw(FUNC(sa1111_device::smcr_r), FUNC(sa1111_device::smcr_w));
	map(0x0008, 0x000b).r(FUNC(sa1111_device::skid_r));

	map(0x0200, 0x0203).rw(FUNC(sa1111_device::skpcr_r), FUNC(sa1111_device::skpcr_w));
	map(0x0204, 0x0207).rw(FUNC(sa1111_device::skcdr_r), FUNC(sa1111_device::skcdr_w));
	map(0x0208, 0x020b).rw(FUNC(sa1111_device::skaud_r), FUNC(sa1111_device::skaud_w));
	map(0x020c, 0x020f).rw(FUNC(sa1111_device::skpmc_r), FUNC(sa1111_device::skpmc_w));
	map(0x0210, 0x0213).rw(FUNC(sa1111_device::skptc_r), FUNC(sa1111_device::skptc_w));
	map(0x0214, 0x0217).rw(FUNC(sa1111_device::skpen0_r), FUNC(sa1111_device::skpen0_w));
	map(0x0218, 0x021b).rw(FUNC(sa1111_device::skpwm0_r), FUNC(sa1111_device::skpwm0_w));
	map(0x021c, 0x021f).rw(FUNC(sa1111_device::skpen1_r), FUNC(sa1111_device::skpen1_w));
	map(0x0220, 0x0223).rw(FUNC(sa1111_device::skpwm1_r), FUNC(sa1111_device::skpwm1_w));

	map(0x0400, 0x0457).rw(FUNC(sa1111_device::ohci_r), FUNC(sa1111_device::ohci_w));
	map(0x0518, 0x051b).r(FUNC(sa1111_device::usb_status_r));
	map(0x051c, 0x051f).rw(FUNC(sa1111_device::usb_reset_r), FUNC(sa1111_device::usb_reset_w));
	map(0x0520, 0x0523).w(FUNC(sa1111_device::usb_int_test_w));
	map(0x0530, 0x055f).r(FUNC(sa1111_device::usb_fifo_r));

	map(0x0600, 0x0603).rw(FUNC(sa1111_device::sacr0_r), FUNC(sa1111_device::sacr0_w));
	map(0x0604, 0x0607).rw(FUNC(sa1111_device::sacr1_r), FUNC(sa1111_device::sacr1_w));
	map(0x0608, 0x060b).rw(FUNC(sa1111_device::sacr2_r), FUNC(sa1111_device::sacr2_w));
	map(0x060c, 0x060f).r(FUNC(sa1111_device::sasr0_r));
	map(0x0610, 0x0613).r(FUNC(sa1111_device::sasr1_r));
	map(0x0618, 0x061b).w(FUNC(sa1111_device::sascr_w));
	map(0x061c, 0x061f).rw(FUNC(sa1111_device::l3car_r), FUNC(sa1111_device::l3car_w));
	map(0x0620, 0x0623).rw(FUNC(sa1111_device::l3cdr_r), FUNC(sa1111_device::l3cdr_w));
	map(0x0624, 0x0627).rw(FUNC(sa1111_device::accar_r), FUNC(sa1111_device::accar_w));
	map(0x0628, 0x062b).rw(FUNC(sa1111_device::accdr_r), FUNC(sa1111_device::accdr_w));
	map(0x062c, 0x062f).rw(FUNC(sa1111_device::acsar_r), FUNC(sa1111_device::acsar_w));
	map(0x0630, 0x0633).rw(FUNC(sa1111_device::acsdr_r), FUNC(sa1111_device::acsdr_w));
	map(0x0634, 0x0637).rw(FUNC(sa1111_device::sadtcs_r), FUNC(sa1111_device::sadtcs_w));
	map(0x0638, 0x063b).rw(FUNC(sa1111_device::sadtsa_r), FUNC(sa1111_device::sadtsa_w));
	map(0x063c, 0x063f).rw(FUNC(sa1111_device::sadtca_r), FUNC(sa1111_device::sadtca_w));
	map(0x0640, 0x0643).rw(FUNC(sa1111_device::sadtsb_r), FUNC(sa1111_device::sadtsb_w));
	map(0x0644, 0x0647).rw(FUNC(sa1111_device::sadtcb_r), FUNC(sa1111_device::sadtcb_w));
	map(0x0648, 0x064b).rw(FUNC(sa1111_device::sadrcs_r), FUNC(sa1111_device::sadrcs_w));
	map(0x064c, 0x064f).rw(FUNC(sa1111_device::sadrsa_r), FUNC(sa1111_device::sadrsa_w));
	map(0x0650, 0x0653).rw(FUNC(sa1111_device::sadrca_r), FUNC(sa1111_device::sadrca_w));
	map(0x0654, 0x0657).rw(FUNC(sa1111_device::sadrsb_r), FUNC(sa1111_device::sadrsb_w));
	map(0x0658, 0x065b).rw(FUNC(sa1111_device::sadrcb_r), FUNC(sa1111_device::sadrcb_w));
	map(0x065c, 0x065f).w(FUNC(sa1111_device::saitr_w));
	map(0x0680, 0x06bf).rw(FUNC(sa1111_device::sadr_r), FUNC(sa1111_device::sadr_w));

	map(0x0800, 0x0803).rw(FUNC(sa1111_device::sspcr0_r), FUNC(sa1111_device::sspcr0_w));
	map(0x0804, 0x0807).rw(FUNC(sa1111_device::sspcr1_r), FUNC(sa1111_device::sspcr1_w));
	map(0x0810, 0x0813).rw(FUNC(sa1111_device::sspsr_r), FUNC(sa1111_device::sspsr_w));
	map(0x0814, 0x0817).w(FUNC(sa1111_device::sspitr_w));
	map(0x0840, 0x087f).rw(FUNC(sa1111_device::sspdr_r), FUNC(sa1111_device::sspdr_w));

	map(0x0a00, 0x0a03).rw(FUNC(sa1111_device::track_kbdcr_r), FUNC(sa1111_device::track_kbdcr_w));
	map(0x0a04, 0x0a07).rw(FUNC(sa1111_device::track_kbdstat_r), FUNC(sa1111_device::track_kbdstat_w));
	map(0x0a08, 0x0a0b).rw(FUNC(sa1111_device::track_kbddata_r), FUNC(sa1111_device::track_kbddata_w));
	map(0x0a0c, 0x0a0f).rw(FUNC(sa1111_device::track_kbdclkdiv_r), FUNC(sa1111_device::track_kbdclkdiv_w));
	map(0x0a10, 0x0a13).rw(FUNC(sa1111_device::track_kbdprecnt_r), FUNC(sa1111_device::track_kbdprecnt_w));
	map(0x0a14, 0x0a17).w(FUNC(sa1111_device::track_kbditr_w));

	map(0x0c00, 0x0c03).rw(FUNC(sa1111_device::mouse_kbdcr_r), FUNC(sa1111_device::mouse_kbdcr_w));
	map(0x0c04, 0x0c07).rw(FUNC(sa1111_device::mouse_kbdstat_r), FUNC(sa1111_device::mouse_kbdstat_w));
	map(0x0c08, 0x0c0b).rw(FUNC(sa1111_device::mouse_kbddata_r), FUNC(sa1111_device::mouse_kbddata_w));
	map(0x0c0c, 0x0c0f).rw(FUNC(sa1111_device::mouse_kbdclkdiv_r), FUNC(sa1111_device::mouse_kbdclkdiv_w));
	map(0x0c10, 0x0c13).rw(FUNC(sa1111_device::mouse_kbdprecnt_r), FUNC(sa1111_device::mouse_kbdprecnt_w));
	map(0x0c14, 0x0c17).w(FUNC(sa1111_device::mouse_kbditr_w));

	map(0x1000, 0x1003).rw(FUNC(sa1111_device::ddr_r<0>), FUNC(sa1111_device::ddr_w<0>));
	map(0x1004, 0x1007).rw(FUNC(sa1111_device::drr_r<0>), FUNC(sa1111_device::dwr_w<0>));
	map(0x1008, 0x100b).rw(FUNC(sa1111_device::sdr_r<0>), FUNC(sa1111_device::sdr_w<0>));
	map(0x100c, 0x100f).rw(FUNC(sa1111_device::ssr_r<0>), FUNC(sa1111_device::ssr_w<0>));
	map(0x1010, 0x1013).rw(FUNC(sa1111_device::ddr_r<1>), FUNC(sa1111_device::ddr_w<1>));
	map(0x1014, 0x1017).rw(FUNC(sa1111_device::drr_r<1>), FUNC(sa1111_device::dwr_w<1>));
	map(0x1018, 0x101b).rw(FUNC(sa1111_device::sdr_r<1>), FUNC(sa1111_device::sdr_w<1>));
	map(0x101c, 0x101f).rw(FUNC(sa1111_device::ssr_r<1>), FUNC(sa1111_device::ssr_w<1>));
	map(0x1020, 0x1023).rw(FUNC(sa1111_device::ddr_r<2>), FUNC(sa1111_device::ddr_w<2>));
	map(0x1024, 0x1027).rw(FUNC(sa1111_device::drr_r<2>), FUNC(sa1111_device::dwr_w<2>));
	map(0x1028, 0x102b).rw(FUNC(sa1111_device::sdr_r<2>), FUNC(sa1111_device::sdr_w<2>));
	map(0x102c, 0x102f).rw(FUNC(sa1111_device::ssr_r<2>), FUNC(sa1111_device::ssr_w<2>));

	map(0x1600, 0x1603).rw(FUNC(sa1111_device::inttest_r<0>), FUNC(sa1111_device::inttest_w<0>));
	map(0x1604, 0x1607).rw(FUNC(sa1111_device::inttest_r<1>), FUNC(sa1111_device::inttest_w<1>));
	map(0x1608, 0x160b).rw(FUNC(sa1111_device::inten_r<0>), FUNC(sa1111_device::inten_w<0>));
	map(0x160c, 0x160f).rw(FUNC(sa1111_device::inten_r<1>), FUNC(sa1111_device::inten_w<1>));
	map(0x1610, 0x1613).rw(FUNC(sa1111_device::intpol_r<0>), FUNC(sa1111_device::intpol_w<0>));
	map(0x1614, 0x1617).rw(FUNC(sa1111_device::intpol_r<1>), FUNC(sa1111_device::intpol_w<1>));
	map(0x1618, 0x161b).rw(FUNC(sa1111_device::inttstsel_r), FUNC(sa1111_device::inttstsel_w));
	map(0x161c, 0x161f).rw(FUNC(sa1111_device::intstat_r<0>), FUNC(sa1111_device::intclr_w<0>));
	map(0x1620, 0x1623).rw(FUNC(sa1111_device::intstat_r<1>), FUNC(sa1111_device::intclr_w<1>));
	map(0x1624, 0x1627).w(FUNC(sa1111_device::intset_w<0>));
	map(0x1628, 0x162b).w(FUNC(sa1111_device::intset_w<1>));
	map(0x162c, 0x162f).rw(FUNC(sa1111_device::wake_en_r<0>), FUNC(sa1111_device::wake_en_w<0>));
	map(0x1630, 0x1633).rw(FUNC(sa1111_device::wake_en_r<1>), FUNC(sa1111_device::wake_en_w<1>));
	map(0x1634, 0x1637).rw(FUNC(sa1111_device::wake_pol_r<0>), FUNC(sa1111_device::wake_pol_w<0>));
	map(0x1638, 0x163b).rw(FUNC(sa1111_device::wake_pol_r<1>), FUNC(sa1111_device::wake_pol_w<1>));

	map(0x1800, 0x1803).rw(FUNC(sa1111_device::pccr_r), FUNC(sa1111_device::pccr_w));
	map(0x1804, 0x1807).rw(FUNC(sa1111_device::pcssr_r), FUNC(sa1111_device::pcssr_w));
	map(0x1808, 0x180b).r(FUNC(sa1111_device::pcsr_r));
}

uint32_t sa1111_device::unknown_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_UNKNOWN, "%s: unknown_r: Unknown Register: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

void sa1111_device::unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_UNKNOWN, "%s: unknown_w: Unknown Register: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

/*

  Intel SA-1111 System Bus Interface

  pg. 33 to 50 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

uint32_t sa1111_device::skcr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SBI, "%s: skcr_r: Control Register: %08x & %08x\n", machine().describe_context(), m_sbi_regs.skcr, mem_mask);
	return m_sbi_regs.skcr;
}

uint32_t sa1111_device::smcr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SBI, "%s: smcr_r: Shared Memory Controller Register: %08x & %08x\n", machine().describe_context(), m_sbi_regs.smcr, mem_mask);
	return m_sbi_regs.smcr;
}

uint32_t sa1111_device::skid_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SBI, "%s: skid_r: ID Register: %08x & %08x\n", machine().describe_context(), m_sbi_regs.skid, mem_mask);
	return m_sbi_regs.skid;
}

void sa1111_device::skcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SBI, "%s: skcr_w: Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SBI, "%s:         Use External Clock: %d\n", machine().describe_context(), BIT(data, SKCR_PLLB_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enable System Bus Clocks: %d\n", machine().describe_context(), BIT(data, SKCR_RCLK_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enter Sleep Mode: %d\n", machine().describe_context(), BIT(data, SKCR_SLEEP_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enter Doze Mode: %d\n", machine().describe_context(), BIT(data, SKCR_DOZE_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enable System PLL: %d\n", machine().describe_context(), BIT(data, SKCR_VCO_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enable Scan Test: %d\n", machine().describe_context(), BIT(data, SKCR_SCANTST_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enable Clock Test: %d\n", machine().describe_context(), BIT(data, SKCR_CLKTST_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enable RDY Response: %d\n", machine().describe_context(), BIT(data, SKCR_RDY_BIT));
	LOGMASKED(LOG_SBI, "%s:         Audio Feature Select: %s\n", machine().describe_context(), BIT(data, SKCR_SACMDSL_BIT) ? "AC Link" : "I2S");
	LOGMASKED(LOG_SBI, "%s:         Out-Only Pad Control: %d\n", machine().describe_context(), BIT(data, SKCR_OPPC_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enable PII Test: %d\n", machine().describe_context(), BIT(data, SKCR_PII_BIT));
	LOGMASKED(LOG_SBI, "%s:         USB IO Cell Test: %d\n", machine().describe_context(), BIT(data, SKCR_UIOTEN_BIT));
	LOGMASKED(LOG_SBI, "%s:         Enable /OE on SDRAM DMA Read Cycles: %d\n", machine().describe_context(), 1 - BIT(data, SKCR_OEEN_BIT));
	const bool audio_mode_changed = BIT(m_sbi_regs.skcr ^ ((m_sbi_regs.skcr & ~mem_mask) | (data & mem_mask)), SKCR_SACMDSL_BIT);
	const bool audio_enabled = BIT(m_audio_regs.sacr0, SACR0_ENB_BIT);
	if (audio_mode_changed && audio_enabled)
	{
		// If we're changing audio output modes while the audio interface is active, bring it down in the old mode,
		// then bring it back up in the new mode.
		audio_set_enabled(false);
		COMBINE_DATA(&m_sbi_regs.skcr);
		audio_set_enabled(true);
	}
	else
	{
		COMBINE_DATA(&m_sbi_regs.skcr);
	}
}

void sa1111_device::smcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SBI, "%s: smcr_w: Shared Memory Controller Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SBI, "%s:         DRAM Type: %s\n", machine().describe_context(), BIT(data, SMCR_DTIM_BIT) ? "SDRAM" : "Unknown");
	LOGMASKED(LOG_SBI, "%s:         Shared-Memory Grant Enable: %d\n", machine().describe_context(), BIT(data, SMCR_MBGE_BIT));
	LOGMASKED(LOG_SBI, "%s:         DRAM Row Address Bits: %d\n", machine().describe_context(), 9 + ((data & SMCR_DRAC_MASK) >> SMCR_DRAC_BIT));
	LOGMASKED(LOG_SBI, "%s:         CAS Latency: %d\n", machine().describe_context(), 2 + BIT(data, SMCR_CLAT_BIT));
	COMBINE_DATA(&m_sbi_regs.smcr);
}

/*

  Intel SA-1111 System Controller

  pg. 59 to 66 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

uint32_t sa1111_device::skpcr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpcr_r: Power Control Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skpcr, mem_mask);
	return m_sk_regs.skpcr;
}

uint32_t sa1111_device::skcdr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skcdr_r: Clock Divider Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skcdr, mem_mask);
	return m_sk_regs.skcdr;
}

uint32_t sa1111_device::skaud_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skaud_r: Audio Clock Divider Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skaud, mem_mask);
	return m_sk_regs.skaud;
}

uint32_t sa1111_device::skpmc_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpmc_r: PS/2 Mouse Clock Divider Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skpmc, mem_mask);
	return m_sk_regs.skpmc;
}

uint32_t sa1111_device::skptc_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skptc_r: PS/2 Track Pad Clock Divider Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skptc, mem_mask);
	return m_sk_regs.skptc;
}

uint32_t sa1111_device::skpen0_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpen0_r: PWM0 Enable Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skpen0, mem_mask);
	return m_sk_regs.skpen0;
}

uint32_t sa1111_device::skpwm0_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpwm0_r: PWM0 Clock Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skpwm0, mem_mask);
	return m_sk_regs.skpwm0;
}

uint32_t sa1111_device::skpen1_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpen1_r: PWM1 Enable Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skpen1, mem_mask);
	return m_sk_regs.skpen1;
}

uint32_t sa1111_device::skpwm1_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpwm1_r: PWM1 Clock Register: %08x & %08x\n", machine().describe_context(), m_sk_regs.skpwm1, mem_mask);
	return m_sk_regs.skpwm1;
}

void sa1111_device::skpcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpcr_w: Power Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SK, "%s:          USB Host Controller Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_UCLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          Audio Controller AC Link Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_ACCLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          Audio Controller I2S Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_ISCLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          Audio Controller L3 Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_L3CLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          SSP Controller Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_SCLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          PS/2 Mouse Port Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_PMCLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          PS/2 Track Pad Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_PTCLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          Shared Memory Controller Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_DCLKE_BIT));
	LOGMASKED(LOG_SK, "%s:          PWM Clock Enable: %d\n", machine().describe_context(), BIT(data, SKPCR_PWMCLKE_BIT));
	COMBINE_DATA(&m_sk_regs.skpcr);
}

void sa1111_device::skcdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	static const int s_opd_values[4] = { 1, 4, 2, 8 };
	LOGMASKED(LOG_SK, "%s: skcdr_w: Clock Divider Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SK, "%s:          Feedback Divider: %02x\n", machine().describe_context(), (data & SKCDR_FBD_MASK) >> SKCDR_FBD_BIT);
	LOGMASKED(LOG_SK, "%s:          Input Divider: %02x\n", machine().describe_context(), (data & SKCDR_IPD_MASK) >> SKCDR_IPD_BIT);
	LOGMASKED(LOG_SK, "%s:          Output Divider: %d\n", machine().describe_context(), s_opd_values[(data & SKCDR_OPD_MASK) >> SKCDR_OPD_BIT]);
	LOGMASKED(LOG_SK, "%s:          PLL-Bypass Output Phase: %s\n", machine().describe_context(), BIT(data, SKCDR_OPS_BIT) ? "Inverted" : "In-Phase");
	COMBINE_DATA(&m_sk_regs.skcdr);
}

void sa1111_device::skaud_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t audio_divider = ((data & SKAUD_ACD_MASK) >> SKAUD_ACD_BIT) + 1;
	LOGMASKED(LOG_SK, "%s: skaud_w: Audio Clock Divider Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SK, "%s:          Audio Clock Divider: %02x\n", machine().describe_context(), audio_divider);
	COMBINE_DATA(&m_sk_regs.skaud);
	const uint32_t pll_clock = clock() * 39;
	if (m_audio_codec)
		m_audio_codec->set_unscaled_clock(pll_clock / audio_divider);
}

void sa1111_device::skpmc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpmc_w: PS/2 Mouse Clock Divider Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SK, "%s:          PS/2 Clock Divider: %02x\n", machine().describe_context(), ((data & SKPMC_PMCD_MASK) >> SKPMC_PMCD_BIT) + 1);
	COMBINE_DATA(&m_sk_regs.skpmc);
}

void sa1111_device::skptc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skptc_w: PS/2 Track Pad Clock Divider Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SK, "%s:          PS/2 Clock Divider: %02x\n", machine().describe_context(), ((data & SKPTC_PTCD_MASK) >> SKPTC_PTCD_BIT) + 1);
	COMBINE_DATA(&m_sk_regs.skptc);
}

void sa1111_device::skpen0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpen0_w: PWM0 Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_sk_regs.skpen0);
}

void sa1111_device::skpwm0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpwm0_w: PWM0 Clock Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SK, "%s:           PWM0 Duty Cycle: %02x\n", machine().describe_context(), (data & SKPWM0_PWM0CK_MASK) >> SKPWM0_PWM0CK_BIT);
	COMBINE_DATA(&m_sk_regs.skpwm0);
}

void sa1111_device::skpen1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpen1_w: PWM1 Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_sk_regs.skpen1);
}

void sa1111_device::skpwm1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SK, "%s: skpwm1_w: PWM1 Clock Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SK, "%s:           PWM1 Duty Cycle: %02x\n", machine().describe_context(), (data & SKPWM1_PWM1CK_MASK) >> SKPWM1_PWM1CK_BIT);
	COMBINE_DATA(&m_sk_regs.skpwm1);
}

/*

  Intel SA-1111 USB Host Interface Controller

  pg. 67 to 77 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

uint32_t sa1111_device::ohci_r(offs_t offset, uint32_t mem_mask)
{
	static const char *const s_ohci_names[22] =
	{
		"Revision", "Control", "Command Status", "Interrupt Status",
		"Interrupt Enable", "Interrupt Disable", "HCCA", "Period Current ED",
		"Control Head ED", "Control Current ED", "Bulk Head ED", "Bulk Current ED",
		"Done Head", "Fm Interval", "Fm Remaining", "Fm Number",
		"Periodic Start", "LS Threshold", "Rh Descriptor A", "Rh Descriptor B",
		"Rh Status", "Rh Port Status<1>"
	};
	LOGMASKED(LOG_USB, "%s: ohci_r: %s: %08x & %08x\n", machine().describe_context(), s_ohci_names[offset], m_usb_regs.ohci[offset], mem_mask);
	return m_usb_regs.ohci[offset];
}

uint32_t sa1111_device::usb_status_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_usb_regs.status;
	LOGMASKED(LOG_USB, "%s: usb_status_r: Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_USB, "%s:               HCI Remote Wake-Up Event: %d\n", machine().describe_context(), BIT(data, USBSTAT_IHRW_BIT));
	LOGMASKED(LOG_USB, "%s:               HCI Buffer Active: %d\n", machine().describe_context(), BIT(data, USBSTAT_IHBA_BIT));
	LOGMASKED(LOG_USB, "%s:               Normal HCI Interrupt Active: %d\n", machine().describe_context(), BIT(data, USBSTAT_NHT_BIT));
	LOGMASKED(LOG_USB, "%s:               HCI Interface Clear Signals Active: %d\n", machine().describe_context(), BIT(data, USBSTAT_NHFCT_BIT));
	LOGMASKED(LOG_USB, "%s:               Port Over-Current: %d\n", machine().describe_context(), BIT(data, USBSTAT_UPRT_BIT));
	return data;
}

uint32_t sa1111_device::usb_reset_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_USB, "%s: usb_reset_r: Reset Register: %08x & %08x\n", machine().describe_context(), m_usb_regs.reset, mem_mask);
	return m_usb_regs.reset;
}

uint32_t sa1111_device::usb_fifo_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_USB, "%s: usb_fifo_r: Data FIFO RAM %02x: %08x & %08x\n", machine().describe_context(), offset, m_usb_regs.fifo[offset], mem_mask);
	return m_usb_regs.fifo[offset];
}

void sa1111_device::ohci_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	static const char *const s_ohci_names[22] =
	{
		"Revision", "Control", "Command Status", "Interrupt Status",
		"Interrupt Enable", "Interrupt Disable", "HCCA", "Period Current ED",
		"Control Head ED", "Control Current ED", "Bulk Head ED", "Bulk Current ED",
		"Done Head", "Fm Interval", "Fm Remaining", "Fm Number",
		"Periodic Start", "LS Threshold", "Rh Descriptor A", "Rh Descriptor B",
		"Rh Status", "Rh Port Status<1>"
	};
	LOGMASKED(LOG_USB, "%s: ohci_w: %s = %08x & %08x\n", machine().describe_context(), s_ohci_names[offset], data, mem_mask);
	COMBINE_DATA(&m_usb_regs.ohci[offset]);
}

void sa1111_device::usb_reset_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_USB, "%s: usb_reset_w: Reset Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_USB, "%s:              Reset ASB Master Interface: %d\n", machine().describe_context(), BIT(data, USBRST_FIR_BIT));
	LOGMASKED(LOG_USB, "%s:              Reset USB Host Controller: %d\n", machine().describe_context(), BIT(data, USBRST_FHR_BIT));
	LOGMASKED(LOG_USB, "%s:              Reset Host Controller Clock-Gen Block: %d\n", machine().describe_context(), BIT(data, USBRST_CGR_BIT));
	LOGMASKED(LOG_USB, "%s:              Scale-down 1ms Clock: %d\n", machine().describe_context(), BIT(data, USBRST_SSDC_BIT));
	LOGMASKED(LOG_USB, "%s:              Interrupt Test Enable: %d\n", machine().describe_context(), BIT(data, USBRST_UIT_BIT));
	LOGMASKED(LOG_USB, "%s:              Enable Sleep Standby: %d\n", machine().describe_context(), BIT(data, USBRST_SSE_BIT));
	LOGMASKED(LOG_USB, "%s:              USB Power Sense Polarity: %s\n", machine().describe_context(), BIT(data, USBRST_PSPL_BIT) ? "Active-Low" : "Active-High");
	LOGMASKED(LOG_USB, "%s:              PwrCtrlPolLow Polarity: %s\n", machine().describe_context(), BIT(data, USBRST_PCPL_BIT) ? "Active-Low" : "Active-High");
	COMBINE_DATA(&m_usb_regs.reset);
}

void sa1111_device::usb_int_test_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_USB, "%s: usb_int_test_w: Interrupt Test Register = %08x & %08x\n", machine().describe_context(), m_usb_regs.int_test, mem_mask);
	LOGMASKED(LOG_USB, "%s:                 Force HCI Remote Wake-Up Interrupt: %d\n", machine().describe_context(), BIT(data, USBINT_IHRWT_BIT));
	LOGMASKED(LOG_USB, "%s:                 Force HCI Buffer Active Interrupt: %d\n", machine().describe_context(), BIT(data, USBINT_IHBAT_BIT));
	LOGMASKED(LOG_USB, "%s:                 Force Normal HC Interrupt: %d\n", machine().describe_context(), BIT(data, USBINT_NHT_BIT));
	LOGMASKED(LOG_USB, "%s:                 Force HCI Interface Transfer Abort Interrupt: %d\n", machine().describe_context(), BIT(data, USBINT_NHFCT_BIT));
	LOGMASKED(LOG_USB, "%s:                 Force USB Port Resume Interrupt: %d\n", machine().describe_context(), BIT(data, USBINT_UPRT_BIT));
	COMBINE_DATA(&m_usb_regs.int_test);
}

/*

  Intel SA-1111 Serial Audio Controller

  pg. 79 to 108 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

void sa1111_device::l3wd_in(int state)
{
	if (state)
		m_audio_regs.sasr0 |= (1 << SASR0_L3WD_BIT);
}

TIMER_CALLBACK_MEMBER(sa1111_device::audio_rx_dma_callback)
{
	// TODO: Audio input
}

TIMER_CALLBACK_MEMBER(sa1111_device::audio_rx_callback)
{
	// TODO: Audio input
}

TIMER_CALLBACK_MEMBER(sa1111_device::audio_tx_dma_callback)
{
	const uint32_t buf = BIT(m_audio_regs.sadtcs, SADTCS_TBIU_BIT);
	const uint32_t remaining = m_audio_regs.sadtcc >> 2;
	const uint32_t avail = std::size(m_audio_regs.tx_fifo) - m_audio_regs.tx_fifo_count;
	if (remaining == 0 || avail == 0)
		return;

	address_space &space = m_maincpu->space(AS_PROGRAM);
	const uint32_t data = space.read_dword(m_audio_regs.sadta);
	LOGMASKED(LOG_AUDIO_DMA, "audio_tx_dma_callback: read data %08x from %08x, pushing to FIFO\n", data, m_audio_regs.sadta);
	audio_tx_fifo_push(data);
	m_audio_regs.sadta += 4;

	m_audio_regs.sadtcc = (remaining - 1) << 2;
	if (!m_audio_regs.sadtcc)
	{
		static constexpr uint32_t s_start_masks[2] = { (1 << SADTCS_TDSTA_BIT), (1 << SADTCS_TDSTB_BIT) };
		static constexpr uint32_t s_done_masks[2] = { (1 << SADTCS_TDBDA_BIT), (1 << SADTCS_TDBDB_BIT) };
		static constexpr uint32_t s_done_ints[2] = { INT_AUDTXA, INT_AUDTXB };
		m_audio_regs.sadtcs &= ~s_start_masks[buf];
		m_audio_regs.sadtcs |= s_done_masks[buf];
		set_irq_line(s_done_ints[buf], 1);
		m_audio_regs.sadtcs ^= (1 << SADTCS_TBIU_BIT);
		m_audio_regs.sadta = m_audio_regs.sadts[1 - buf];
		m_audio_regs.sadtcc = m_audio_regs.sadtc[1 - buf];
		if (!BIT(m_audio_regs.sadtcs, s_start_masks[1 - buf]))
		{
			m_audio_regs.tx_dma_timer->adjust(attotime::never);
		}
	}
}

TIMER_CALLBACK_MEMBER(sa1111_device::audio_tx_callback)
{
	m_i2s_out(audio_tx_fifo_pop());
}

void sa1111_device::audio_update_mode()
{
	audio_set_enabled(BIT(m_audio_regs.sacr0, SACR0_ENB_BIT));
}

void sa1111_device::audio_clear_interrupts()
{
	set_irq_line(INT_AUDTXA, 0);
	set_irq_line(INT_AUDRXA, 0);
	set_irq_line(INT_AUDTXB, 0);
	set_irq_line(INT_AUDRXB, 0);
	set_irq_line(INT_AUDTFS, 0);
	set_irq_line(INT_AUDRFS, 0);
	set_irq_line(INT_AUDTUR, 0);
	set_irq_line(INT_AUDROR, 0);
	set_irq_line(INT_AUDDTS, 0);
	set_irq_line(INT_AUDRDD, 0);
	set_irq_line(INT_AUDSTO, 0);
}

void sa1111_device::audio_controller_reset()
{
	m_audio_regs.rx_fifo_read_idx = 0;
	m_audio_regs.rx_fifo_write_idx = 0;
	m_audio_regs.rx_fifo_count = 0;
	m_audio_regs.tx_fifo_read_idx = 0;
	m_audio_regs.tx_fifo_write_idx = 0;
	m_audio_regs.tx_fifo_count = 0;
}

void sa1111_device::audio_set_enabled(bool enabled)
{
	if (enabled)
	{
		audio_set_tx_dma_enabled(BIT(m_audio_regs.sadtcs, SADTCS_TDEN_BIT));
		audio_set_rx_dma_enabled(BIT(m_audio_regs.sadrcs, SADRCS_RDEN_BIT));
		audio_update_tx_fifo_levels();
		audio_update_rx_fifo_levels();
		audio_update_busy_flag();

		uint32_t &status = BIT(m_sbi_regs.skcr, SKCR_SACMDSL_BIT) ? m_audio_regs.sasr1 : m_audio_regs.sasr0;
		set_irq_line(INT_AUDTUR, BIT(status, SASR_TUR_BIT));
		set_irq_line(INT_AUDROR, BIT(status, SASR_ROR_BIT));
		set_irq_line(INT_AUDDTS, BIT(status, SASR_SEND_BIT));
		set_irq_line(INT_AUDRDD, BIT(status, SASR_RECV_BIT));
		set_irq_line(INT_AUDSTO, BIT(m_audio_regs.sasr1, SASR1_RSTO_BIT));
	}
	else
	{
		audio_set_tx_dma_enabled(false);
		audio_set_rx_dma_enabled(false);
		audio_clear_interrupts();
	}
}

void sa1111_device::audio_set_tx_dma_enabled(bool enabled)
{
	LOGMASKED(LOG_AUDIO_DMA, "audio_set_tx_dma_enabled: %d\n", enabled);
	if (enabled)
	{
		if (m_audio_regs.tx_dma_timer->remaining() == attotime::never)
		{
			const uint32_t buf = BIT(m_audio_regs.sadtcs, SADTCS_TBIU_BIT);
			if ((buf == 0 && BIT(m_audio_regs.sadtcs, SADTCS_TDSTA_BIT)) || (buf == 1 && BIT(m_audio_regs.sadtcs, SADTCS_TDSTB_BIT)))
			{
				LOGMASKED(LOG_AUDIO_DMA, "audio_set_tx_dma_enabled, starting Tx DMA from buffer %d\n", buf);
				audio_start_tx_dma(buf);
			}
		}
	}
	else
	{
		m_audio_regs.tx_timer->adjust(attotime::never);
		m_audio_regs.tx_dma_timer->adjust(attotime::never);

		set_irq_line(INT_AUDTXA, 0);
		set_irq_line(INT_AUDTXB, 0);
		set_irq_line(INT_AUDTFS, 0);
		set_irq_line(INT_AUDTUR, 0);
	}
}

void sa1111_device::audio_set_rx_dma_enabled(bool enabled)
{
	if (enabled)
	{
		if (m_audio_regs.rx_dma_timer->remaining() == attotime::never)
		{
			const uint32_t buf = BIT(m_audio_regs.sadrcs, SADRCS_RBIU_BIT);
			if ((buf == 0 && BIT(m_audio_regs.sadrcs, SADRCS_RDSTA_BIT)) || (buf == 1 && BIT(m_audio_regs.sadrcs, SADRCS_RDSTB_BIT)))
			{
				audio_start_rx_dma(buf);
			}
		}
	}
	else
	{
		m_audio_regs.rx_timer->adjust(attotime::never);
		m_audio_regs.rx_dma_timer->adjust(attotime::never);

		set_irq_line(INT_AUDRXA, 0);
		set_irq_line(INT_AUDRXB, 0);
		set_irq_line(INT_AUDRFS, 0);
		set_irq_line(INT_AUDROR, 0);
	}
}

void sa1111_device::audio_start_tx_dma(const uint32_t buf)
{
	if (!m_audio_codec)
		return;

	m_audio_regs.sadta = m_audio_regs.sadts[buf];
	m_audio_regs.sadtcc = m_audio_regs.sadtc[buf];

	const uint32_t divisor = ((m_sk_regs.skaud & SKAUD_ACD_MASK) >> SKAUD_ACD_BIT) + 1;
	const uint32_t pll_clock = clock() * 39;
	attotime clock_period = attotime::from_ticks(divisor * 128, pll_clock);
	m_audio_regs.tx_dma_timer->adjust(clock_period, 0, clock_period);

	LOGMASKED(LOG_AUDIO_DMA, "audio_start_tx_dma, setting start address to %08x, Tx clock to %d / %d\n", m_audio_regs.sadta, pll_clock, divisor);
}

void sa1111_device::audio_start_rx_dma(const uint32_t buf)
{
	if (!m_audio_codec)
		return;

	m_audio_regs.sadra = m_audio_regs.sadrs[buf];

	const uint32_t divisor = ((m_sk_regs.skaud & SKAUD_ACD_MASK) >> SKAUD_ACD_BIT) + 1;
	const uint32_t pll_clock = clock() * 39;
	attotime clock_period = attotime::from_ticks(divisor * 256, pll_clock);
	m_audio_regs.rx_dma_timer->adjust(clock_period, 0, clock_period);
}

void sa1111_device::audio_update_tx_fifo_levels()
{
	uint32_t &status = BIT(m_sbi_regs.skcr, SKCR_SACMDSL_BIT) ? m_audio_regs.sasr1 : m_audio_regs.sasr0;
	if (m_audio_regs.tx_fifo_count < std::size(m_audio_regs.tx_fifo))
		status |= (1 << SASR_TNF_BIT);
	else
		status &= ~(1 << SASR_TNF_BIT);

	const uint32_t tfl = ((m_audio_regs.tx_fifo_count == std::size(m_audio_regs.tx_fifo)) ? (m_audio_regs.tx_fifo_count - 1) : m_audio_regs.tx_fifo_count);
	status &= ~SASR_TFL_MASK;
	status |= (tfl << SASR_TFL_BIT);

	const uint32_t tfth = ((m_audio_regs.sacr0 & SACR0_TFTH_MASK) >> SACR0_TFTH_BIT) + 1;
	if (tfl <= tfth)
	{
		status |= (1 << SASR_TFS_BIT);
		set_irq_line(INT_AUDTFS, 1);
	}
	else
	{
		status &= ~(1 << SASR_TFS_BIT);
		set_irq_line(INT_AUDTFS, 0);
	}
}

void sa1111_device::audio_update_rx_fifo_levels()
{
	uint32_t &status = BIT(m_sbi_regs.skcr, SKCR_SACMDSL_BIT) ? m_audio_regs.sasr1 : m_audio_regs.sasr0;
	if (m_audio_regs.rx_fifo_count != 0)
		status |= (1 << SASR_RNE_BIT);
	else
		status &= ~(1 << SASR_RNE_BIT);

	const uint32_t rfl = ((m_audio_regs.rx_fifo_count == std::size(m_audio_regs.rx_fifo)) ? (m_audio_regs.rx_fifo_count - 1) : m_audio_regs.rx_fifo_count);
	status &= ~SASR_RFL_MASK;
	status |= (rfl << SASR_RFL_BIT);

	const uint32_t rfth = ((m_audio_regs.sacr0 & SACR0_RFTH_MASK) >> SACR0_RFTH_BIT) + 1;
	if (rfl >= rfth)
	{
		status |= (1 << SASR_RFS_BIT);
		set_irq_line(INT_AUDRFS, 1);
	}
	else
	{
		status &= ~(1 << SASR_RFS_BIT);
		set_irq_line(INT_AUDRFS, 0);
	}
}

void sa1111_device::audio_update_busy_flag()
{
	uint32_t &status = BIT(m_sbi_regs.skcr, SKCR_SACMDSL_BIT) ? m_audio_regs.sasr1 : m_audio_regs.sasr0;
	if (m_audio_regs.rx_fifo_count > 0 || m_audio_regs.tx_fifo_count > 0)
		status |= (1 << SASR_BSY_BIT);
	else
		status &= ~(1 << SASR_BSY_BIT);
}

void sa1111_device::audio_tx_fifo_push(uint32_t data)
{
	if (m_audio_regs.tx_fifo_count < std::size(m_audio_regs.tx_fifo))
	{
		m_audio_regs.tx_fifo[m_audio_regs.tx_fifo_write_idx] = data;
		m_audio_regs.tx_fifo_write_idx = (m_audio_regs.tx_fifo_write_idx + 1) % std::size(m_audio_regs.tx_fifo);
		m_audio_regs.tx_fifo_count++;
		audio_update_tx_fifo_levels();
		if (m_audio_regs.tx_timer->remaining() == attotime::never)
		{
			const uint32_t divisor = ((m_sk_regs.skaud & SKAUD_ACD_MASK) >> SKAUD_ACD_BIT) + 1;
			const uint32_t pll_clock = clock() * 39;
			attotime clock_period = attotime::from_ticks(divisor * 96, pll_clock);
			m_audio_regs.tx_timer->adjust(clock_period, 0, clock_period);
		}
	}
}

uint32_t sa1111_device::audio_tx_fifo_pop()
{
	if (m_audio_regs.tx_fifo_count > 0)
	{
		const uint32_t data = m_audio_regs.tx_fifo[m_audio_regs.tx_fifo_read_idx];
		m_audio_regs.tx_fifo_read_idx = (m_audio_regs.tx_fifo_read_idx + 1) % std::size(m_audio_regs.tx_fifo);
		m_audio_regs.tx_fifo_count--;
		audio_update_tx_fifo_levels();
		if (m_audio_regs.tx_fifo_count == 0)
		{
			m_audio_regs.tx_timer->adjust(attotime::never);
		}
		return data;
	}
	else
	{
		uint32_t &status = BIT(m_sbi_regs.skcr, SKCR_SACMDSL_BIT) ? m_audio_regs.sasr1 : m_audio_regs.sasr0;
		status |= (1 << SASR_TUR_BIT);
		set_irq_line(INT_AUDTUR, 1);
		return m_audio_regs.tx_fifo[m_audio_regs.tx_fifo_read_idx];
	}
}

void sa1111_device::audio_rx_fifo_push(uint32_t data)
{
	if (m_audio_regs.rx_fifo_count < std::size(m_audio_regs.rx_fifo))
	{
		m_audio_regs.rx_fifo[m_audio_regs.rx_fifo_write_idx] = data;
		m_audio_regs.rx_fifo_write_idx = (m_audio_regs.rx_fifo_write_idx + 1) % std::size(m_audio_regs.rx_fifo);
		m_audio_regs.rx_fifo_count++;
		audio_update_rx_fifo_levels();
	}
	else
	{
		uint32_t &status = BIT(m_sbi_regs.skcr, SKCR_SACMDSL_BIT) ? m_audio_regs.sasr1 : m_audio_regs.sasr0;
		status |= (1 << SASR_ROR_BIT);
		set_irq_line(INT_AUDROR, 1);
	}
}

uint32_t sa1111_device::audio_rx_fifo_pop()
{
	if (m_audio_regs.rx_fifo_count > 0)
	{
		const uint32_t data = m_audio_regs.rx_fifo[m_audio_regs.rx_fifo_read_idx];
		m_audio_regs.rx_fifo_read_idx = (m_audio_regs.rx_fifo_read_idx + 1) % std::size(m_audio_regs.rx_fifo);
		m_audio_regs.rx_fifo_count--;
		audio_update_rx_fifo_levels();
		return data;
	}
	return m_audio_regs.rx_fifo[m_audio_regs.rx_fifo_read_idx];
}

uint32_t sa1111_device::sacr0_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sacr0_r: Serial Audio Common Control Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.sacr0, mem_mask);
	return m_audio_regs.sacr0;
}

uint32_t sa1111_device::sacr1_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sacr1_r: Serial Audio Alternate Mode Control Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.sacr1, mem_mask);
	return m_audio_regs.sacr1;
}

uint32_t sa1111_device::sacr2_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sacr2_r: Serial Audio AC-Link Control Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.sacr2, mem_mask);
	return m_audio_regs.sacr2;
}

uint32_t sa1111_device::sasr0_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_audio_regs.sasr0;
	LOGMASKED(LOG_AUDIO, "%s: sasr0_r: Serial Audio Status Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Not Full: %d\n", machine().describe_context(), BIT(data, SASR_TNF_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Not Empty: %d\n", machine().describe_context(), BIT(data, SASR_RNE_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Serial Audio Controller Busy: %d\n", machine().describe_context(), BIT(data, SASR_BSY_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SASR_TFS_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SASR_RFS_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Underrun: %d\n", machine().describe_context(), BIT(data, SASR_TUR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Overrun: %d\n", machine().describe_context(), BIT(data, SASR_ROR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Level: %02x\n", machine().describe_context(), (data & SASR_TFL_MASK) >> SASR_TFL_BIT);
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Level: %02x\n", machine().describe_context(), (data & SASR_RFL_MASK) >> SASR_RFL_BIT);
	LOGMASKED(LOG_AUDIO, "%s:          L3 Control Bus Data Write Done: %d\n", machine().describe_context(), BIT(data, SASR0_L3WD_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          L3 Control Bus Data Read Done: %d\n", machine().describe_context(), BIT(data, SASR0_L3RD_BIT));
	return data;
}

uint32_t sa1111_device::sasr1_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_audio_regs.sasr1;
	LOGMASKED(LOG_AUDIO, "%s: sasr1_r: Serial Audio Status Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Not Full: %d\n", machine().describe_context(), BIT(data, SASR_TNF_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Not Empty: %d\n", machine().describe_context(), BIT(data, SASR_RNE_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Serial Audio Controller Busy: %d\n", machine().describe_context(), BIT(data, SASR_BSY_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SASR_TFS_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SASR_RFS_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Underrun: %d\n", machine().describe_context(), BIT(data, SASR_TUR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Overrun: %d\n", machine().describe_context(), BIT(data, SASR_ROR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Level: %02x\n", machine().describe_context(), (data & SASR_TFL_MASK) >> SASR_TFL_BIT);
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Level: %02x\n", machine().describe_context(), (data & SASR_RFL_MASK) >> SASR_RFL_BIT);
	LOGMASKED(LOG_AUDIO, "%s:          AC-Link Command Address and Data Transmitted: %d\n", machine().describe_context(), BIT(data, SASR1_CADT_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          AC-Link Status Address and Data Received: %d\n", machine().describe_context(), BIT(data, SASR1_SADR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Read Status Time-Out: %d\n", machine().describe_context(), BIT(data, SASR1_RSTO_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          AC'97 Codec BIT_CLK in Low-Power Mode: %d\n", machine().describe_context(), BIT(data, SASR1_CLPM_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          AC'97 Codec Ready: %d\n", machine().describe_context(), BIT(data, SASR1_CRDY_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Received Slot 3 Valid (Left Channel Valid): %d\n", machine().describe_context(), BIT(data, SASR1_RS3V_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Received Slot 4 Valid (Right Channel Valid): %d\n", machine().describe_context(), BIT(data, SASR1_RS4V_BIT));
	return data;
}

uint32_t sa1111_device::l3car_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: l3car_r: L3 Control Bus Address Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.l3car, mem_mask);
	return m_audio_regs.l3car;
}

uint32_t sa1111_device::l3cdr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: l3car_r: L3 Control Bus Data Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.l3cdr, mem_mask);
	return m_audio_regs.l3cdr;
}

uint32_t sa1111_device::accar_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: accar_r: AC-Link Command Address Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.accar, mem_mask);
	return m_audio_regs.accar;
}

uint32_t sa1111_device::accdr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: accdr_r: AC-Link Command Data Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.accdr, mem_mask);
	return m_audio_regs.accdr;
}

uint32_t sa1111_device::acsar_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: acsar_r: AC-Link Status Address Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.acsar, mem_mask);
	return m_audio_regs.acsar;
}

uint32_t sa1111_device::acsdr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: acsdr_r: AC-Link Status Data Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.acsdr, mem_mask);
	return m_audio_regs.acsdr;
}

uint32_t sa1111_device::sadtcs_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtcs_r: Serial Audio DMA Transmit Control/Status Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadtcs, mem_mask);
	return m_audio_regs.sadtcs;
}

uint32_t sa1111_device::sadtsa_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtsa_r: Serial Audio DMA Transmit Buffer Start Address Register A: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadts[0], mem_mask);
	return m_audio_regs.sadts[0];
}

uint32_t sa1111_device::sadtca_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtca_r: Serial Audio DMA Transmit Buffer Count Register A: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadtc[0], mem_mask);
	return m_audio_regs.sadtc[0];
}

uint32_t sa1111_device::sadtsb_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtsb_r: Serial Audio DMA Transmit Buffer Start Address Register B: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadts[1], mem_mask);
	return m_audio_regs.sadts[1];
}

uint32_t sa1111_device::sadtcb_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtcb_r: Serial Audio DMA Transmit Buffer Count Register B: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadtc[1], mem_mask);
	return m_audio_regs.sadtc[1];
}

uint32_t sa1111_device::sadrcs_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrcs_r: Serial Audio DMA Receive Control/Status Register: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadrcs, mem_mask);
	return m_audio_regs.sadrcs;
}

uint32_t sa1111_device::sadrsa_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrsa_r: Serial Audio DMA Receive Buffer Start Address Register A: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadrs[0], mem_mask);
	return m_audio_regs.sadrs[0];
}

uint32_t sa1111_device::sadrca_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrca_r: Serial Audio DMA Receive Buffer Count Register A: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadrc[0], mem_mask);
	return m_audio_regs.sadrc[0];
}

uint32_t sa1111_device::sadrsb_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrsb_r: Serial Audio DMA Receive Buffer Start Address Register B: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadrs[1], mem_mask);
	return m_audio_regs.sadrs[1];
}

uint32_t sa1111_device::sadrcb_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrcb_r: Serial Audio DMA Receive Buffer Count Register B: %08x & %08x\n", machine().describe_context(), m_audio_regs.sadrc[1], mem_mask);
	return m_audio_regs.sadrc[1];
}

uint32_t sa1111_device::sadr_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = audio_rx_fifo_pop();
	LOGMASKED(LOG_AUDIO, "%s: sadr_r: Serial Audio Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1111_device::sacr0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sacr0_w: Serial Audio Common Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:          Enable Serial Audio Controller: %d\n", machine().describe_context(), BIT(data, SACR0_ENB_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          BIT_CLK Pin Direction: %s\n", machine().describe_context(), BIT(data, SACR0_BCKD_BIT) ? "Input" : "Output");
	LOGMASKED(LOG_AUDIO, "%s:          Reset SAC Control and FIFOs: %d\n", machine().describe_context(), BIT(data, SACR0_RST_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Threshold: %02x\n", machine().describe_context(), (data & SACR0_TFTH_MASK) >> SACR0_TFTH_BIT);
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Threshold: %02x\n", machine().describe_context(), (data & SACR0_RFTH_MASK) >> SACR0_RFTH_BIT);

	const uint32_t old = m_audio_regs.sacr0;
	COMBINE_DATA(&m_audio_regs.sacr0);
	const uint32_t changed = old ^ m_audio_regs.sacr0;

	if (BIT(m_audio_regs.sacr0, SACR0_RST_BIT))
		audio_controller_reset();

	if (BIT(changed, SACR0_ENB_BIT))
	{
		audio_set_enabled(BIT(m_audio_regs.sacr0, SACR0_ENB_BIT));
	}
	else
	{
		if (changed & SACR0_TFTH_MASK)
			audio_update_tx_fifo_levels();
		if (changed & SACR0_RFTH_MASK)
			audio_update_rx_fifo_levels();
	}
}

void sa1111_device::sacr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sacr1_w: Serial Audio Alternate Mode Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:          Alternate Mode Operation: %s\n", machine().describe_context(), BIT(data, SACR1_AMSL_BIT) ? "MSB-Justified" : "I2S");
	LOGMASKED(LOG_AUDIO, "%s:          Enable L3 Control Bus: %d\n", machine().describe_context(), BIT(data, SACR1_L3EN_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          L3 Control Bus Data Multi-Byte Transfer: %s\n", machine().describe_context(), BIT(data, SACR1_L3MB_BIT) ? "Multiple-Byte" : "Last Byte");
	LOGMASKED(LOG_AUDIO, "%s:          Disable Recording Function: %d\n", machine().describe_context(), BIT(data, SACR1_DREC_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Disable Replaying Function: %d\n", machine().describe_context(), BIT(data, SACR1_DRPL_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Enable L3 and I2S/MSB-Justified Loopback: %d\n", machine().describe_context(), BIT(data, SACR1_ENLBF_BIT));
	COMBINE_DATA(&m_audio_regs.sacr1);
}

void sa1111_device::sacr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sacr2_w: Serial Audio AC-Link Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:          Transmit Slot 3 Valid (Left Out Valid): %d\n", machine().describe_context(), BIT(data, SACR2_TS3V_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit Slot 4 Valid (Right Out Valid): %d\n", machine().describe_context(), BIT(data, SACR2_TS4V_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Wake Up AC'97 Codec: %d\n", machine().describe_context(), BIT(data, SACR2_WKUP_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Disable Recording of AC-Link Interface: %d\n", machine().describe_context(), BIT(data, SACR2_DREC_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Disable Replaying of AC-Link Interface: %d\n", machine().describe_context(), BIT(data, SACR2_DRPL_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Enable AC-Link Loopback: %d\n", machine().describe_context(), BIT(data, SACR2_ENLBF_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Specify Reset# Signal to SYS_CLK Output: %s\n", machine().describe_context(), BIT(data, SACR2_RESET_BIT) ? "Inactive" : "Active-Low");
	COMBINE_DATA(&m_audio_regs.sacr2);
}

void sa1111_device::sascr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sascr_w: Serial Audio Status Clear Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:          Clear Transmit FIFO Underrun: %d\n", machine().describe_context(), BIT(data, SASCR_TUR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Clear Receive FIFO Overrun: %d\n", machine().describe_context(), BIT(data, SASCR_ROR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Clear L3C/AC-Link Data Sent Status Bit: %d\n", machine().describe_context(), BIT(data, SASCR_DTS_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Clear L3C/AC-Link Data Read Done Status Bit: %d\n", machine().describe_context(), BIT(data, SASCR_RDD_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Clear AC-Link Read Status Time-Out Bit: %d\n", machine().describe_context(), BIT(data, SASCR_STO_BIT));

	if (BIT(data, SASCR_TUR_BIT))
	{
		m_audio_regs.sasr0 &= ~(1 << SASR_TUR_BIT);
		m_audio_regs.sasr1 &= ~(1 << SASR_TUR_BIT);
		set_irq_line(INT_AUDTUR, 0);
	}
	if (BIT(data, SASCR_ROR_BIT))
	{
		m_audio_regs.sasr0 &= ~(1 << SASR_ROR_BIT);
		m_audio_regs.sasr1 &= ~(1 << SASR_ROR_BIT);
		set_irq_line(INT_AUDROR, 0);
	}
	if (BIT(data, SASCR_DTS_BIT))
	{
		m_audio_regs.sasr0 &= ~(1 << SASR0_L3WD_BIT);
		m_audio_regs.sasr1 &= ~(1 << SASR1_CADT_BIT);
		set_irq_line(INT_AUDDTS, 0);
	}
	if (BIT(data, SASCR_RDD_BIT))
	{
		m_audio_regs.sasr0 &= ~(1 << SASR0_L3RD_BIT);
		m_audio_regs.sasr1 &= ~(1 << SASR1_SADR_BIT);
		set_irq_line(INT_AUDRDD, 0);
	}
	if (BIT(data, SASCR_STO_BIT))
	{
		m_audio_regs.sasr1 &= ~(1 << SASR1_RSTO_BIT);
		set_irq_line(INT_AUDSTO, 0);
	}
}

void sa1111_device::l3car_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: l3car_w: L3 Control Bus Address Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.l3car);
	m_l3_addr_out((uint8_t)data);
}

void sa1111_device::l3cdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: l3cdr_w: L3 Control Bus Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.l3cdr);
	m_l3_data_out((uint8_t)data);
}

void sa1111_device::accar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: accar_w: AC-Link Command Address Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.accar);
}

void sa1111_device::accdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: accdr_w: AC-Link Command Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.accdr);
}

void sa1111_device::acsar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: acsar_w: AC-Link Status Address Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.acsar);
}

void sa1111_device::acsdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: acsdr_w: AC-Link Status Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.acsdr);
}

void sa1111_device::sadtcs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtcs_w: Serial Audio DMA Transmit Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:           Serial Audio DMA Transmit Enable: %d\n", machine().describe_context(), BIT(data, SADTCS_TDEN_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Clear Serial Audio DMA Transmit Buffer Done A: %d\n", machine().describe_context(), BIT(data, SADTCS_TDBDA_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Serial Audio DMA Transmit Buffer Start Transfer A: %d\n", machine().describe_context(), BIT(data, SADTCS_TDSTA_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Clear Serial Audio DMA Transmit Buffer Done B: %d\n", machine().describe_context(), BIT(data, SADTCS_TDBDB_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Serial Audio DMA Transmit Buffer Start Transfer B: %d\n", machine().describe_context(), BIT(data, SADTCS_TDSTB_BIT));

	const uint32_t old = m_audio_regs.sadtcs;

	static constexpr uint32_t start_mask = (1 << SADTCS_TDSTA_BIT) | (1 << SADTCS_TDSTB_BIT);
	static constexpr uint32_t write_mask = (1 << SADTCS_TDEN_BIT) | start_mask;
	m_audio_regs.sadtcs &= ~write_mask;
	m_audio_regs.sadtcs |= data & write_mask & mem_mask;

	if (BIT(data, SADTCS_TDBDA_BIT) || BIT(data, SADTCS_TDSTA_BIT))
	{
		LOGMASKED(LOG_AUDIO_DMA, "%s: sadtcs_w: Clearing done A bit, lowering AUDTXA IRQ\n", machine().describe_context());
		m_audio_regs.sadtcs &= ~(1 << SADTCS_TDBDA_BIT);
		set_irq_line(INT_AUDTXA, 0);
	}
	if (BIT(data, SADTCS_TDBDB_BIT) || BIT(data, SADTCS_TDSTB_BIT))
	{
		LOGMASKED(LOG_AUDIO_DMA, "%s: sadtcs_w: Clearing done B bit, lowering AUDTXB IRQ\n", machine().describe_context());
		m_audio_regs.sadtcs &= ~(1 << SADTCS_TDBDB_BIT);
		set_irq_line(INT_AUDTXB, 0);
	}

	const uint32_t changed = old ^ m_audio_regs.sadtcs;

	if (BIT(changed, SADTCS_TDEN_BIT))
	{
		audio_set_tx_dma_enabled(BIT(changed, SADTCS_TDEN_BIT));
	}
	else if (BIT(m_audio_regs.sadtcs, SADTCS_TDEN_BIT) && (changed & start_mask))
	{
		audio_set_tx_dma_enabled(true);
	}
}

void sa1111_device::sadtsa_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtsa_w: Serial Audio DMA Transmit Buffer Start Address Register A = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadts[0]);
}

void sa1111_device::sadtca_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtca_w: Serial Audio DMA Transmit Buffer Count Register A = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadtc[0]);
}

void sa1111_device::sadtsb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtsb_w: Serial Audio DMA Transmit Buffer Start Address Register B = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadts[1]);
}

void sa1111_device::sadtcb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadtcb_w: Serial Audio DMA Transmit Buffer Count Register B = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadtc[1]);
}

void sa1111_device::sadrcs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrcs_w: Serial Audio DMA Receive Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:           Serial Audio DMA Receive Enable: %d\n", machine().describe_context(), BIT(data, SADRCS_RDEN_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Clear Serial Audio DMA Receive Buffer Done A: %d\n", machine().describe_context(), BIT(data, SADRCS_RDBDA_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Serial Audio DMA Receive Buffer Start Transfer A: %d\n", machine().describe_context(), BIT(data, SADRCS_RDSTA_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Clear Serial Audio DMA Receive Buffer Done B: %d\n", machine().describe_context(), BIT(data, SADRCS_RDBDB_BIT));
	LOGMASKED(LOG_AUDIO, "%s:           Serial Audio DMA Receive Buffer Start Transfer B: %d\n", machine().describe_context(), BIT(data, SADRCS_RDSTB_BIT));

	const uint32_t old = m_audio_regs.sadrcs;

	static constexpr uint32_t start_mask = (1 << SADRCS_RDSTA_BIT) | (1 << SADRCS_RDSTB_BIT);
	static constexpr uint32_t write_mask = (1 << SADRCS_RDEN_BIT) | start_mask;
	m_audio_regs.sadrcs &= ~write_mask;
	m_audio_regs.sadrcs |= data & write_mask & mem_mask;

	if (BIT(data, SADRCS_RDBDA_BIT))
	{
		m_audio_regs.sadrcs &= ~(1 << SADRCS_RDBDA_BIT);
		set_irq_line(INT_AUDRXA, 0);
	}
	if (BIT(data, SADRCS_RDBDB_BIT))
	{
		m_audio_regs.sadrcs &= ~(1 << SADRCS_RDBDB_BIT);
		set_irq_line(INT_AUDRXB, 0);
	}

	const uint32_t changed = old ^ m_audio_regs.sadrcs;

	if (BIT(changed, SADRCS_RDEN_BIT))
	{
		audio_set_rx_dma_enabled(BIT(changed, SADRCS_RDEN_BIT));
	}
	else if (BIT(m_audio_regs.sadrcs, SADRCS_RDEN_BIT) && (changed & start_mask))
	{
		audio_set_rx_dma_enabled(true);
	}
}

void sa1111_device::sadrsa_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrsa_w: Serial Audio DMA Receive Buffer Start Address Register A = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadrs[0]);
}

void sa1111_device::sadrca_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrca_w: Serial Audio DMA Receive Buffer Count Register A = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadrc[0]);
}

void sa1111_device::sadrsb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrsb_w: Serial Audio DMA Receive Buffer Start Address Register B = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadrs[1]);
}

void sa1111_device::sadrcb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadrcb_w: Serial Audio DMA Receive Buffer Count Register B = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_audio_regs.sadrc[1]);
}

void sa1111_device::saitr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: saitr_w: Serial Audio Interrupt Test Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SAITR_TFS_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SAITR_RFS_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Transmit FIFO Underrun: %d\n", machine().describe_context(), BIT(data, SAITR_TUR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Receive FIFO Overrun: %d\n", machine().describe_context(), BIT(data, SAITR_ROR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Command Address/Data Transfer Done: %d\n", machine().describe_context(), BIT(data, SAITR_CADT_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Status Address/Data Receive Done: %d\n", machine().describe_context(), BIT(data, SAITR_SADR_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          Read Status Time-Out: %d\n", machine().describe_context(), BIT(data, SAITR_RSTO_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          DMA Transmit Buffer Done A: %d\n", machine().describe_context(), BIT(data, SAITR_TDBDA_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          DMA Transmit Buffer Done B: %d\n", machine().describe_context(), BIT(data, SAITR_TDBDB_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          DMA Receive Buffer Done A: %d\n", machine().describe_context(), BIT(data, SAITR_RDBDA_BIT));
	LOGMASKED(LOG_AUDIO, "%s:          DMA Receive Buffer Done B: %d\n", machine().describe_context(), BIT(data, SAITR_RDBDB_BIT));
}

void sa1111_device::sadr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "%s: sadr_w: Serial Audio Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	audio_tx_fifo_push(data);
}

/*

  Intel SA-1111 SSP Serial Port

  pg. 109 to 124 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1111_device::ssp_rx_callback)
{
	// TODO: Implement receiving serial data rather than in bulk.
}

TIMER_CALLBACK_MEMBER(sa1111_device::ssp_tx_callback)
{
	// TODO: Implement transmitting data serially rather than in bulk.
	if (m_ssp_regs.tx_fifo_count)
	{
		const uint16_t data = m_ssp_regs.tx_fifo[m_ssp_regs.tx_fifo_read_idx];
		m_ssp_out(data);

		m_ssp_regs.tx_fifo_read_idx = (m_ssp_regs.tx_fifo_read_idx + 1) % std::size(m_ssp_regs.tx_fifo);
		m_ssp_regs.tx_fifo_count--;

		m_ssp_regs.sspsr |= (1 << SSPSR_TNF_BIT);

		ssp_update_tx_level();
	}
}

void sa1111_device::ssp_update_enable_state()
{
	if (BIT(m_ssp_regs.sspcr0, SSPCR0_SSPEN_BIT))
	{
		const uint32_t tfl = (m_ssp_regs.sspsr & SSPSR_TFL_MASK) >> SSPSR_TFL_BIT;
		const uint32_t rfl = (m_ssp_regs.sspsr & SSPSR_RFL_MASK) >> SSPSR_RFL_BIT;
		const uint32_t tft = (m_ssp_regs.sspsr & SSPCR1_TFT_MASK) >> SSPCR1_TFT_BIT;
		const uint32_t rft = (m_ssp_regs.sspsr & SSPCR1_RFT_MASK) >> SSPCR1_RFT_BIT;

		if (tfl != (std::size(m_ssp_regs.tx_fifo) - 1))
			m_ssp_regs.sspsr |= (1 << SSPSR_TNF_BIT);
		else
			m_ssp_regs.sspsr &= ~(1 << SSPSR_TNF_BIT);

		if (rfl != 0)
			m_ssp_regs.sspsr |= (1 << SSPSR_RNE_BIT);
		else
			m_ssp_regs.sspsr &= ~(1 << SSPSR_RNE_BIT);

		if (tfl != 0 || rfl != 0)
			m_ssp_regs.sspsr |= (1 << SSPSR_BSY_BIT);
		else
			m_ssp_regs.sspsr &= ~(1 << SSPSR_BSY_BIT);

		if (tfl <= tft)
			m_ssp_regs.sspsr |= (1 << SSPSR_TFS_BIT);
		else
			m_ssp_regs.sspsr &= ~(1 << SSPSR_TFS_BIT);

		if (rfl <= rft)
			m_ssp_regs.sspsr |= (1 << SSPSR_RFS_BIT);
		else
			m_ssp_regs.sspsr &= ~(1 << SSPSR_RFS_BIT);

		uint64_t bit_count = (m_ssp_regs.sspcr0 & SSPCR0_DSS_MASK) >> SSPCR0_DSS_BIT;
		uint32_t clock_rate = 2 * (((m_ssp_regs.sspcr0 & SSPCR0_SCR_MASK) >> SSPCR0_SCR_BIT) + 1);
		attotime packet_rate = attotime::from_ticks(bit_count * clock_rate, 3686400);
		m_ssp_regs.rx_timer->adjust(packet_rate, 0, packet_rate);
		m_ssp_regs.tx_timer->adjust(packet_rate, 0, packet_rate);
	}
	else
	{
		m_ssp_regs.sspsr &= ~(1 << SSPSR_TFS_BIT);
		m_ssp_regs.sspsr &= ~(1 << SSPSR_RFS_BIT);

		m_ssp_regs.rx_fifo_read_idx = 0;
		m_ssp_regs.rx_fifo_write_idx = 0;
		m_ssp_regs.rx_fifo_count = 0;
		m_ssp_regs.tx_fifo_read_idx = 0;
		m_ssp_regs.tx_fifo_write_idx = 0;
		m_ssp_regs.tx_fifo_count = 0;

		m_ssp_regs.rx_timer->adjust(attotime::never);
		m_ssp_regs.tx_timer->adjust(attotime::never);
	}
}

void sa1111_device::ssp_update_rx_level()
{
	const uint32_t rfl = m_ssp_regs.rx_fifo_count;
	m_ssp_regs.sspsr &= ~SSPSR_RFL_MASK;
	m_ssp_regs.sspsr |= (rfl << SSPSR_RFL_BIT);

	const uint32_t rft = (m_ssp_regs.sspcr1 & SSPCR1_RFT_MASK) >> SSPCR1_RFT_BIT;
	if (rfl >= rft)
		m_ssp_regs.sspsr |= (1 << SSPSR_RFS_BIT);
	else
		m_ssp_regs.sspsr &= ~(1 << SSPSR_RFS_BIT);
}

void sa1111_device::ssp_rx_fifo_push(const uint16_t data)
{
	if (m_ssp_regs.rx_fifo_count < std::size(m_ssp_regs.rx_fifo))
	{
		m_ssp_regs.rx_fifo[m_ssp_regs.rx_fifo_write_idx] = data;
		m_ssp_regs.rx_fifo_write_idx = (m_ssp_regs.rx_fifo_write_idx + 1) % std::size(m_ssp_regs.rx_fifo);
		m_ssp_regs.rx_fifo_count++;

		m_ssp_regs.sspsr |= (1 << SSPSR_RNE_BIT);

		ssp_update_rx_level();
	}
}

void sa1111_device::ssp_update_tx_level()
{
	const uint32_t tfl = m_ssp_regs.tx_fifo_count;
	m_ssp_regs.sspsr &= ~SSPSR_TFL_MASK;
	m_ssp_regs.sspsr |= (tfl << SSPSR_TFL_BIT);

	const uint32_t tft = (m_ssp_regs.sspcr1 & SSPCR1_TFT_MASK) >> SSPCR1_TFT_BIT;
	if (tfl >= tft)
		m_ssp_regs.sspsr |= (1 << SSPSR_TFS_BIT);
	else
		m_ssp_regs.sspsr &= ~(1 << SSPSR_TFS_BIT);
}

void sa1111_device::ssp_tx_fifo_push(const uint16_t data)
{
	if (m_ssp_regs.tx_fifo_count < std::size(m_ssp_regs.tx_fifo))
	{
		m_ssp_regs.tx_fifo[m_ssp_regs.tx_fifo_write_idx] = data;
		m_ssp_regs.tx_fifo_write_idx = (m_ssp_regs.tx_fifo_write_idx + 1) % std::size(m_ssp_regs.tx_fifo);
		m_ssp_regs.tx_fifo_count++;

		if (m_ssp_regs.tx_fifo_count != std::size(m_ssp_regs.tx_fifo))
			m_ssp_regs.sspsr |= (1 << SSPSR_TNF_BIT);
		else
			m_ssp_regs.sspsr &= ~(1 << SSPSR_TNF_BIT);

		ssp_update_tx_level();
	}
}

uint16_t sa1111_device::ssp_rx_fifo_pop()
{
	uint16_t data = m_ssp_regs.rx_fifo[m_ssp_regs.rx_fifo_read_idx];
	if (m_ssp_regs.rx_fifo_count)
	{
		m_ssp_regs.rx_fifo_read_idx = (m_ssp_regs.rx_fifo_read_idx + 1) % std::size(m_ssp_regs.rx_fifo);
		m_ssp_regs.rx_fifo_count--;

		if (m_ssp_regs.rx_fifo_count == 0)
			m_ssp_regs.sspsr &= ~(1 << SSPSR_RNE_BIT);

		ssp_update_rx_level();
	}
	return data;
}

uint32_t sa1111_device::sspcr0_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: sspcr0_r: SSP Control Register 0: %08x & %08x\n", machine().describe_context(), m_ssp_regs.sspcr0, mem_mask);
	return m_ssp_regs.sspcr0;
}

uint32_t sa1111_device::sspcr1_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: sspcr1_r: SSP Control Register 1: %08x & %08x\n", machine().describe_context(), m_ssp_regs.sspcr1, mem_mask);
	return m_ssp_regs.sspcr1;
}

uint32_t sa1111_device::sspsr_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_ssp_regs.sspsr;
	LOGMASKED(LOG_SSP_HF, "%s: sspsr_r: SSP Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP_HF, "%s:          Transmit FIFO Not Full: %d\n", machine().describe_context(), BIT(data, SSPSR_TNF_BIT));
	LOGMASKED(LOG_SSP_HF, "%s:          Receive FIFO Not Empty: %d\n", machine().describe_context(), BIT(data, SSPSR_RNE_BIT));
	LOGMASKED(LOG_SSP_HF, "%s:          Transmit/Receive In Progress: %d\n", machine().describe_context(), BIT(data, SSPSR_BSY_BIT));
	LOGMASKED(LOG_SSP_HF, "%s:          Transmit FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SSPSR_TFS_BIT));
	LOGMASKED(LOG_SSP_HF, "%s:          Receive FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SSPSR_RFS_BIT));
	LOGMASKED(LOG_SSP_HF, "%s:          Receive FIFO Overrun: %d\n", machine().describe_context(), BIT(data, SSPSR_ROR_BIT));
	LOGMASKED(LOG_SSP_HF, "%s:          Transmit FIFO Level: %02x\n", machine().describe_context(), (data & SSPSR_TFL_MASK) >> SSPSR_TFL_BIT);
	LOGMASKED(LOG_SSP_HF, "%s:          Receive FIFO Level: %02x\n", machine().describe_context(), (data & SSPSR_RFL_MASK) >> SSPSR_RFL_BIT);
	return data;
}

uint32_t sa1111_device::sspdr_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = ssp_rx_fifo_pop();
	LOGMASKED(LOG_SSP, "%s: sspdr_r: SSP Data Read Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1111_device::sspcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	static const char *const s_dss_sizes[16] =
	{
		"Invalid [1]", "Invalid [2]", "Invalid [3]", "4-bit",
		"5-bit", "6-bit", "7-bit", "8-bit",
		"9-bit", "10-bit", "11-bit", "12-bit",
		"13-bit", "14-bit", "15-bit", "16-bit"
	};
	static const char *const s_frf_formats[4] = { "Motorola SPI", "TI Synchronous Serial", "National Microwire", "Reserved" };
	LOGMASKED(LOG_SSP, "%s: sspcr0_w: SSP Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP, "%s:           Data Size Select: %s\n", machine().describe_context(), s_dss_sizes[(data & SSPCR0_DSS_MASK) >> SSPCR0_DSS_BIT]);
	LOGMASKED(LOG_SSP, "%s:           Frame Format: %s\n", machine().describe_context(), s_frf_formats[(data & SSPCR0_FRF_MASK) >> SSPCR0_FRF_BIT]);
	LOGMASKED(LOG_SSP, "%s:           SSP Enable: %d\n", machine().describe_context(), BIT(data, SSPCR0_SSPEN_BIT));
	LOGMASKED(LOG_SSP, "%s:           Serial Clock Rate Divisor: %03x\n", machine().describe_context(), (data & SSPCR0_SCR_MASK) >> SSPCR0_SCR_BIT);
	const uint32_t old = m_ssp_regs.sspcr0;
	COMBINE_DATA(&m_ssp_regs.sspcr0);
	const uint32_t changed = old ^ m_ssp_regs.sspcr0;
	if (BIT(changed, SSPCR0_SSPEN_BIT))
		ssp_update_enable_state();
}

void sa1111_device::sspcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: sspcr1_w: SSP Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP, "%s:           Loopback Mode: %d\n", machine().describe_context(), BIT(data, SSPCR1_LBM_BIT));
	LOGMASKED(LOG_SSP, "%s:           Clock Polarity, Idle State: %s\n", machine().describe_context(), BIT(data, SSPCR1_SPO_BIT) ? "High" : "Low");
	LOGMASKED(LOG_SSP, "%s:           Clock Phase: %d\n", machine().describe_context(), BIT(data, SSPCR1_SPH_BIT));
	LOGMASKED(LOG_SSP, "%s:           Transmit FIFO Threshold: %02x\n", machine().describe_context(), (data & SSPCR1_TFT_MASK) >> SSPCR1_TFT_BIT);
	LOGMASKED(LOG_SSP, "%s:           Receive FIFO Threshold: %02x\n", machine().describe_context(), (data & SSPCR1_RFT_MASK) >> SSPCR1_RFT_BIT);
	COMBINE_DATA(&m_ssp_regs.sspcr1);
}

void sa1111_device::sspsr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: sspsr_w: SSP Status Register (ignored, all read-only flags) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

void sa1111_device::sspitr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: sspitr_w: SSP Interrupt Test Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP, "%s:           Transmit FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SSPITR_TFS_BIT));
	LOGMASKED(LOG_SSP, "%s:           Receive FIFO Service Request: %d\n", machine().describe_context(), BIT(data, SSPITR_RFS_BIT));
	LOGMASKED(LOG_SSP, "%s:           Receive FIFO Overrun: %d\n", machine().describe_context(), BIT(data, SSPITR_ROR_BIT));
	COMBINE_DATA(&m_ssp_regs.sspitr);
}

void sa1111_device::sspdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: sspdr_w: SSP Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	ssp_tx_fifo_push((uint16_t)data);
}

/*

  Intel SA-1111 PS/2 Trackpad and Mouse Interfaces

  pg. 125 to 131 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

uint32_t sa1111_device::track_kbdcr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbdcr_r: Track Pad Control Register: %08x & %08x\n", machine().describe_context(), m_track_regs.kbdcr, mem_mask);
	return m_track_regs.kbdcr;
}

uint32_t sa1111_device::track_kbdstat_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_track_regs.kbdstat;
	LOGMASKED(LOG_TRACK, "%s: track_kbdstat_r: Track Pad Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_TRACK, "%s:                  KBCLK Pin State: %d\n", machine().describe_context(), BIT(data, KBDSTAT_KBC_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  KBDATA Pin State: %d\n", machine().describe_context(), BIT(data, KBDSTAT_KBD_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  Data Byte Parity Bit: %d\n", machine().describe_context(), BIT(data, KBDSTAT_RXP_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  Track Pad Enabled: %d\n", machine().describe_context(), BIT(data, KBDSTAT_ENA_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  Receiver Busy: %d\n", machine().describe_context(), BIT(data, KBDSTAT_RXB_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  Receiver Full: %d\n", machine().describe_context(), BIT(data, KBDSTAT_RXF_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  Transmitter Busy: %d\n", machine().describe_context(), BIT(data, KBDSTAT_TXB_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  Transmitter Empty: %d\n", machine().describe_context(), BIT(data, KBDSTAT_TXE_BIT));
	LOGMASKED(LOG_TRACK, "%s:                  Stop Bit Error: %d\n", machine().describe_context(), BIT(data, KBDSTAT_STP_BIT));
	return data;
}

uint32_t sa1111_device::track_kbddata_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbddata_r: Track Pad Received Data Byte: %08x & %08x\n", machine().describe_context(), m_track_regs.kbddata_rx, mem_mask);
	return m_track_regs.kbddata_rx;
}

uint32_t sa1111_device::track_kbdclkdiv_r(offs_t offset, uint32_t mem_mask)
{
	static const char *const s_clkdiv_values[4] = { "8MHz", "4MHz", "2MHz", "Reserved" };
	LOGMASKED(LOG_TRACK, "%s: track_kbdclkdiv_r: Track Pad Clock Division Register: %08x & %08x\n", machine().describe_context(), m_track_regs.kbdclkdiv, mem_mask);
	LOGMASKED(LOG_TRACK, "%s:                    Incoming KbdClk: %s\n", machine().describe_context(), s_clkdiv_values[(m_track_regs.kbdclkdiv & KBDCLKDIV_DV_MASK) >> KBDCLKDIV_DV_BIT]);
	return m_track_regs.kbdclkdiv;
}

uint32_t sa1111_device::track_kbdprecnt_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbdprecnt_r: Track Pad Clock Precount Register: %08x & %08x\n", machine().describe_context(), m_track_regs.kbdprecnt, mem_mask);
	return m_track_regs.kbdprecnt;
}

uint32_t sa1111_device::mouse_kbdcr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdcr_r: Mouse Control Register: %08x & %08x\n", machine().describe_context(), m_mouse_regs.kbdcr, mem_mask);
	return m_mouse_regs.kbdcr;
}

uint32_t sa1111_device::mouse_kbdstat_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_mouse_regs.kbdstat;
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdstat_r: Mouse Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_MOUSE, "%s:                  KBCLK Pin State: %d\n", machine().describe_context(), BIT(data, KBDSTAT_KBC_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  KBDATA Pin State: %d\n", machine().describe_context(), BIT(data, KBDSTAT_KBD_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  Data Byte Parity Bit: %d\n", machine().describe_context(), BIT(data, KBDSTAT_RXP_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  Mouse Enabled: %d\n", machine().describe_context(), BIT(data, KBDSTAT_ENA_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  Receiver Busy: %d\n", machine().describe_context(), BIT(data, KBDSTAT_RXB_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  Receiver Full: %d\n", machine().describe_context(), BIT(data, KBDSTAT_RXF_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  Transmitter Busy: %d\n", machine().describe_context(), BIT(data, KBDSTAT_TXB_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  Transmitter Empty: %d\n", machine().describe_context(), BIT(data, KBDSTAT_TXE_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                  Stop Bit Error: %d\n", machine().describe_context(), BIT(data, KBDSTAT_STP_BIT));
	return data;
}

uint32_t sa1111_device::mouse_kbddata_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbddata_r: Mouse Received Data Byte: %08x & %08x\n", machine().describe_context(), m_mouse_regs.kbddata_rx, mem_mask);
	return m_mouse_regs.kbddata_rx;
}

uint32_t sa1111_device::mouse_kbdclkdiv_r(offs_t offset, uint32_t mem_mask)
{
	static const char *const s_clkdiv_values[4] = { "8MHz", "4MHz", "2MHz", "Reserved" };
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdclkdiv_r: Mouse Clock Division Register: %08x & %08x\n", machine().describe_context(), m_mouse_regs.kbdclkdiv, mem_mask);
	LOGMASKED(LOG_MOUSE, "%s:                    Incoming KbdClk: %s\n", machine().describe_context(), s_clkdiv_values[(m_mouse_regs.kbdclkdiv & KBDCLKDIV_DV_MASK) >> KBDCLKDIV_DV_BIT]);
	return m_mouse_regs.kbdclkdiv;
}

uint32_t sa1111_device::mouse_kbdprecnt_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdprecnt_r: Mouse Clock Precount Register: %08x & %08x\n", machine().describe_context(), m_mouse_regs.kbdprecnt, mem_mask);
	return m_mouse_regs.kbdprecnt;
}

void sa1111_device::track_kbdcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbdcr_w: Track Pad Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_TRACK, "%s:                Force TPCLK Low: %d\n", machine().describe_context(), BIT(data, KBDCR_FKC_BIT));
	LOGMASKED(LOG_TRACK, "%s:                Force TPDATA Low: %d\n", machine().describe_context(), BIT(data, KBDCR_FKD_BIT));
	LOGMASKED(LOG_TRACK, "%s:                Enable Track Pad: %d\n", machine().describe_context(), BIT(data, KBDCR_ENA_BIT));
	COMBINE_DATA(&m_track_regs.kbdcr);
}

void sa1111_device::track_kbdstat_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbdstat_w: Track Pad Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_track_regs.kbdstat &= ~(data & mem_mask & (1 << KBDSTAT_STP_BIT));
}

void sa1111_device::track_kbddata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbddata_w: Track Pad Transmit Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_track_regs.kbddata_tx);
}

void sa1111_device::track_kbdclkdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbdclkdiv_w: Track Pad Clock Division Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_track_regs.kbdclkdiv);
}

void sa1111_device::track_kbdprecnt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbdprecnt_w: Track Pad Clock Precount Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_track_regs.kbdprecnt);
}

void sa1111_device::track_kbditr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_TRACK, "%s: track_kbditr_w: Track Pad Interrupt Test Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_track_regs.kbditr);
}

void sa1111_device::mouse_kbdcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdcr_w: Mouse Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_MOUSE, "%s:                Force TPCLK Low: %d\n", machine().describe_context(), BIT(data, KBDCR_FKC_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                Force TPDATA Low: %d\n", machine().describe_context(), BIT(data, KBDCR_FKD_BIT));
	LOGMASKED(LOG_MOUSE, "%s:                Enable Mouse: %d\n", machine().describe_context(), BIT(data, KBDCR_ENA_BIT));
	COMBINE_DATA(&m_mouse_regs.kbdcr);
}

void sa1111_device::mouse_kbdstat_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdstat_w: Mouse Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_mouse_regs.kbdstat &= ~(data & mem_mask & (1 << KBDSTAT_STP_BIT));
}

void sa1111_device::mouse_kbddata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbddata_w: Mouse Transmit Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_mouse_regs.kbddata_tx);
}

void sa1111_device::mouse_kbdclkdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdclkdiv_w: Mouse Clock Division Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_mouse_regs.kbdclkdiv);
}

void sa1111_device::mouse_kbdprecnt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbdprecnt_w: Mouse Clock Precount Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_mouse_regs.kbdprecnt);
}

void sa1111_device::mouse_kbditr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_MOUSE, "%s: mouse_kbditr_w: Mouse Interrupt Test Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_mouse_regs.kbditr);
}

/*

  Intel SA-1111 General-Purpose I/O Interface

  pg. 133 to 138 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

void sa1111_device::gpio_in(const uint32_t line, const int state)
{
}

void sa1111_device::gpio_update_direction(const uint32_t block, const uint32_t old_dir)
{
	const uint32_t new_outputs = old_dir & ~m_gpio_regs.ddr[block];
	if (new_outputs)
	{
		for (uint32_t line = 0; line < 8; line++)
		{
			if (BIT(new_outputs, line))
			{
				m_gpio_out[block * 8 + line](BIT(m_gpio_regs.out_latch[block], line));
			}
		}
	}
}

void sa1111_device::gpio_update_outputs(const uint32_t block, const uint32_t changed)
{
	uint32_t remaining_changed = changed;

	for (uint32_t line = 0; line < 8 && remaining_changed != 0; line++)
	{
		if (BIT(remaining_changed, line))
		{
			m_gpio_out[block * 8 + line](BIT(m_gpio_regs.level[block], line));
			remaining_changed &= ~(1 << line);
		}
	}
}

template <int Block>
uint32_t sa1111_device::ddr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: ddr_r: GPIO Block %c Data Direction: %08x & %08x\n", machine().describe_context(), 'A' + Block, m_gpio_regs.ddr[Block], mem_mask);
	return m_gpio_regs.ddr[Block];
}

template <int Block>
uint32_t sa1111_device::drr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: drr_r: GPIO Block %c Data Value Register: %08x & %08x\n", machine().describe_context(), 'A' + Block, m_gpio_regs.level[Block], mem_mask);
	return m_gpio_regs.level[Block];
}

template <int Block>
uint32_t sa1111_device::sdr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: sdr_r: GPIO Block %c Sleep Direction: %08x & %08x\n", machine().describe_context(), 'A' + Block, m_gpio_regs.sdr[Block], mem_mask);
	return m_gpio_regs.sdr[Block];
}

template <int Block>
uint32_t sa1111_device::ssr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: ssr_r: GPIO Block %c Sleep State: %08x & %08x\n", machine().describe_context(), 'A' + Block, m_gpio_regs.ssr[Block], mem_mask);
	return m_gpio_regs.ssr[Block];
}

template <int Block>
void sa1111_device::ddr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: ddr_w: GPIO Block %c Data Direction = %08x & %08x\n", machine().describe_context(), 'A' + Block, data, mem_mask);
	const uint32_t old = m_gpio_regs.ddr[Block];
	COMBINE_DATA(&m_gpio_regs.ddr[Block]);
	if (old != m_gpio_regs.ddr[Block])
		gpio_update_direction(Block, old);
}

template <int Block>
void sa1111_device::dwr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: dwr_w: GPIO Block %c Data Value Register = %08x & %08x\n", machine().describe_context(), 'A' + Block, data, mem_mask);
	const uint32_t old = m_gpio_regs.level[Block];
	COMBINE_DATA(&m_gpio_regs.out_latch[Block]);
	m_gpio_regs.level[Block] = (m_gpio_regs.ddr[Block] & m_gpio_regs.in_latch[Block]) | (~m_gpio_regs.ddr[Block] & m_gpio_regs.out_latch[Block]);
	if (old != m_gpio_regs.level[Block])
		gpio_update_outputs(Block, old ^ m_gpio_regs.level[Block]);
}

template <int Block>
void sa1111_device::sdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: sdr_w: GPIO Block %c Sleep Direction = %08x & %08x\n", machine().describe_context(), 'A' + Block, data, mem_mask);
	COMBINE_DATA(&m_gpio_regs.sdr[Block]);
}

template <int Block>
void sa1111_device::ssr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: ssr_w: GPIO Block %c Sleep State = %08x & %08x\n", machine().describe_context(), 'A' + Block, data, mem_mask);
	COMBINE_DATA(&m_gpio_regs.ssr[0]);
}

/*

  Intel SA-1111 Interrupt Controller

  pg. 139 to 146 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

void sa1111_device::set_irq_line(uint32_t line, int state)
{
	const uint32_t set = BIT(line, 5);
	const uint32_t local_bit = line & 0x1f;
	const uint32_t mask = (1 << local_bit);
	const uint32_t old_raw = BIT(m_intc_regs.intraw[set], local_bit);
	//LOGMASKED(LOG_INTC, "Setting IRQ line %d to state %d. Current intraw[%d] state %08x, bit state %d\n", line, state, set, m_intc_regs.intraw[set], old_raw);
	if (old_raw == state)
		return;

	m_intc_regs.intraw[set] &= ~mask;
	m_intc_regs.intraw[set] |= (state << local_bit);

	const uint32_t falling_edge = BIT(m_intc_regs.intpol[set], local_bit);
	const bool new_rising = (!old_raw && state && !falling_edge);
	const bool new_falling = (old_raw && !state && falling_edge);

	if (new_rising || new_falling)
		m_intc_regs.intstat[set] |= mask;

	if (BIT(m_intc_regs.inten[set], local_bit))
	{
		LOGMASKED(LOG_INTC, "IRQ line %d is enabled, updating interrupts\n", line);
		update_interrupts();
	}
}

void sa1111_device::update_interrupts()
{
	const bool any_interrupt = (m_intc_regs.intstat[0] & m_intc_regs.inten[0]) || (m_intc_regs.intstat[1] & m_intc_regs.inten[1]);
	m_irq_out(any_interrupt);
}

template <int Set>
uint32_t sa1111_device::inttest_r(offs_t offset, uint32_t mem_mask)
{
	if (Set == 0)
	{
		const uint32_t any_interrupts = ((m_intc_regs.inttest[0] | m_intc_regs.inttest[1]) != 0) ? 1 : 0;
		LOGMASKED(LOG_INTC, "%s: inttest_r: Interrupt Test Register %d: %08x & %08x\n", machine().describe_context(), Set, any_interrupts, mem_mask);
		return any_interrupts;
	}
	else
	{
		LOGMASKED(LOG_INTC, "%s: inttest_r: Interrupt Test Register %d: %08x & %08x\n", machine().describe_context(), Set, 0, mem_mask);
		return 0;
	}
}

template <int Set>
uint32_t sa1111_device::inten_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: inten_r: Interrupt Enable Register %d: %08x & %08x\n", machine().describe_context(), Set, m_intc_regs.inten[Set], mem_mask);
	return m_intc_regs.inten[Set];
}

template <int Set>
uint32_t sa1111_device::intpol_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intpol_r: Interrupt Polarity Register %d: %08x & %08x\n", machine().describe_context(), Set, m_intc_regs.intpol[Set], mem_mask);
	return m_intc_regs.intpol[Set];
}

uint32_t sa1111_device::inttstsel_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: inttstsel_r: Interrupt Test Mode Select Register: %08x & %08x\n", machine().describe_context(), m_intc_regs.inttstsel, mem_mask);
	return m_intc_regs.inttstsel;
}

template <int Set>
uint32_t sa1111_device::intstat_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intstat_r: Interrupt Status Register %d: %08x & %08x\n", machine().describe_context(), Set, m_intc_regs.intstat[Set], mem_mask);
	return m_intc_regs.intstat[Set];
}

template <int Set>
uint32_t sa1111_device::wake_en_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: wake_en_r: Interrupt Wake-Up Enable Register %d: %08x & %08x\n", machine().describe_context(), Set, m_intc_regs.wake_en[Set], mem_mask);
	return m_intc_regs.wake_en[Set];
}

template <int Set>
uint32_t sa1111_device::wake_pol_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: wake_pol_r: Interrupt Wake-Up Polarity Register %d: %08x & %08x\n", machine().describe_context(), Set, m_intc_regs.wake_pol[Set], mem_mask);
	return m_intc_regs.wake_pol[Set];
}

template <int Set>
void sa1111_device::inttest_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: inttest_w: Interrupt Test Register %d = %08x & %08x\n", machine().describe_context(), Set, data, mem_mask);
	COMBINE_DATA(&m_intc_regs.inttest[Set]);
}

template <int Set>
void sa1111_device::inten_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: inten_w: Interrupt Enable Register %d = %08x & %08x\n", machine().describe_context(), Set, data, mem_mask);
	const uint32_t old = m_intc_regs.inten[Set];
	COMBINE_DATA(&m_intc_regs.inten[Set]);
	if (old != m_intc_regs.inten[Set] && (m_intc_regs.intstat[Set] & old) != 0)
	{
		update_interrupts();
	}
}

template <int Set>
void sa1111_device::intpol_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_w: Interrupt Polarity Register %d = %08x & %08x\n", machine().describe_context(), Set, data, mem_mask);
	COMBINE_DATA(&m_intc_regs.intpol[Set]);
}

void sa1111_device::inttstsel_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: inttestsel_w: Interrupt Test Mode Select Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_intc_regs.inttstsel);
}

template <int Set>
void sa1111_device::intclr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_w: Interrupt Clear Register %d = %08x & %08x\n", machine().describe_context(), Set, data, mem_mask);
	const uint32_t old = m_intc_regs.intstat[Set];
	m_intc_regs.intstat[Set] &= ~(data & mem_mask);
	if (old != m_intc_regs.intstat[Set] && (old & m_intc_regs.inten[Set]) != 0)
	{
		update_interrupts();
	}
}

template <int Set>
void sa1111_device::intset_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intset_w: Interrupt Set Register %d = %08x & %08x\n", machine().describe_context(), Set, data, mem_mask);
	const uint32_t old = m_intc_regs.intstat[Set];
	m_intc_regs.intstat[Set] |= (data & mem_mask);
	if (old != m_intc_regs.intstat[Set] && (m_intc_regs.intstat[Set] & m_intc_regs.inten[Set]) != 0)
	{
		update_interrupts();
	}
}

template <int Set>
void sa1111_device::wake_en_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: wake_en_w: Interrupt Wake-Up Enable Register %d = %08x & %08x\n", machine().describe_context(), Set, data, mem_mask);
	COMBINE_DATA(&m_intc_regs.wake_en[Set]);
}

template <int Set>
void sa1111_device::wake_pol_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: wake_pol_w: Interrupt Wake-Up Polarity Register %d = %08x & %08x\n", machine().describe_context(), Set, data, mem_mask);
	COMBINE_DATA(&m_intc_regs.wake_pol[Set]);
}

/*

  Intel SA-1111 PCMCIA Interface

  pg. 147 to 160 Intel StrongARM SA-1111 Microprocessor Companion Chip Developer's Manual

*/

uint32_t sa1111_device::pccr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_CARD, "%s: pccr_r: PCMCIA Control Register: %08x & %08x\n", machine().describe_context(), m_card_regs.pccr, mem_mask);
	return m_card_regs.pccr;
}

uint32_t sa1111_device::pcssr_r(offs_t offset, uint32_t mem_mask)
{
	LOGMASKED(LOG_CARD, "%s: pcssr_r: PCMCIA Sleep State Register: %08x & %08x\n", machine().describe_context(), m_card_regs.pcssr, mem_mask);
	return m_card_regs.pcssr;
}

uint32_t sa1111_device::pcsr_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = m_card_regs.pcsr;
	LOGMASKED(LOG_CARD, "%s: pcsr_r: PCMCIA Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_CARD, "%s:         Socket 0 Ready: %d\n", machine().describe_context(), BIT(data, PCSR_S0R_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 Ready: %d\n", machine().describe_context(), BIT(data, PCSR_S1R_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 Card Detect: %d\n", machine().describe_context(), BIT(data, PCSR_S0CD_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 Card Detect: %d\n", machine().describe_context(), BIT(data, PCSR_S1CD_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 Voltage Sense 1: %d\n", machine().describe_context(), BIT(data, PCSR_S0VS1_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 Voltage Sense 2: %d\n", machine().describe_context(), BIT(data, PCSR_S0VS2_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 Voltage Sense 1: %d\n", machine().describe_context(), BIT(data, PCSR_S1VS1_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 Voltage Sense 2: %d\n", machine().describe_context(), BIT(data, PCSR_S1VS2_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 Write Protect: %d\n", machine().describe_context(), BIT(data, PCSR_S0WP_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 Write Protect: %d\n", machine().describe_context(), BIT(data, PCSR_S1WP_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 BVD1: %d\n", machine().describe_context(), BIT(data, PCSR_S0BVD1_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 BVD2: %d\n", machine().describe_context(), BIT(data, PCSR_S0BVD2_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 BVD1: %d\n", machine().describe_context(), BIT(data, PCSR_S1BVD1_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 BVD2: %d\n", machine().describe_context(), BIT(data, PCSR_S1BVD2_BIT));
	return data;
}

void sa1111_device::pccr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CARD, "%s: pccr_w: PCMCIA Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_CARD, "%s:         Socket 0 Reset: %d\n", machine().describe_context(), BIT(data, PCCR_S0RST_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 Reset: %d\n", machine().describe_context(), BIT(data, PCCR_S1RST_BIT));
	LOGMASKED(LOG_CARD, "%s:         Set Socket 0 Floating: %d\n", machine().describe_context(), BIT(data, PCCR_S0FLT_BIT));
	LOGMASKED(LOG_CARD, "%s:         Set Socket 1 Floating: %d\n", machine().describe_context(), BIT(data, PCCR_S1FLT_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 PWAIT Enable: %d\n", machine().describe_context(), BIT(data, PCCR_S0PWEN_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 1 PWAIT Enable: %d\n", machine().describe_context(), BIT(data, PCCR_S1PWEN_BIT));
	LOGMASKED(LOG_CARD, "%s:         Socket 0 Power Mode: %s\n", machine().describe_context(), BIT(data, PCCR_S0PSE_BIT) ? "5V" : "3V");
	LOGMASKED(LOG_CARD, "%s:         Socket 1 Power Mode: %s\n", machine().describe_context(), BIT(data, PCCR_S1PSE_BIT) ? "5V" : "3V");
	COMBINE_DATA(&m_card_regs.pccr);
}

void sa1111_device::pcssr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_CARD, "%s: pcssr_w: PCMCIA Sleep State Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_CARD, "%s:          Socket 0 Sleep Mode: %d\n", machine().describe_context(), BIT(data, PCSSR_S0SLP_BIT));
	LOGMASKED(LOG_CARD, "%s:          Socket 1 Sleep Mode: %d\n", machine().describe_context(), BIT(data, PCSSR_S1SLP_BIT));
	COMBINE_DATA(&m_card_regs.pcssr);
}

void sa1111_device::device_start()
{
	save_item(NAME(m_sbi_regs.skcr));
	save_item(NAME(m_sbi_regs.smcr));
	save_item(NAME(m_sbi_regs.skid));

	save_item(NAME(m_sk_regs.skpcr));
	save_item(NAME(m_sk_regs.skcdr));
	save_item(NAME(m_sk_regs.skaud));
	save_item(NAME(m_sk_regs.skpmc));
	save_item(NAME(m_sk_regs.skptc));
	save_item(NAME(m_sk_regs.skpen0));
	save_item(NAME(m_sk_regs.skpwm0));
	save_item(NAME(m_sk_regs.skpen1));
	save_item(NAME(m_sk_regs.skpwm1));

	save_item(NAME(m_usb_regs.ohci));
	save_item(NAME(m_usb_regs.status));
	save_item(NAME(m_usb_regs.reset));
	save_item(NAME(m_usb_regs.int_test));
	save_item(NAME(m_usb_regs.fifo));

	save_item(NAME(m_audio_regs.sacr0));
	save_item(NAME(m_audio_regs.sacr1));
	save_item(NAME(m_audio_regs.sacr2));
	save_item(NAME(m_audio_regs.sasr0));
	save_item(NAME(m_audio_regs.sasr1));
	save_item(NAME(m_audio_regs.l3car));
	save_item(NAME(m_audio_regs.l3cdr));
	save_item(NAME(m_audio_regs.accar));
	save_item(NAME(m_audio_regs.accdr));
	save_item(NAME(m_audio_regs.acsar));
	save_item(NAME(m_audio_regs.acsdr));
	save_item(NAME(m_audio_regs.sadtcs));
	save_item(NAME(m_audio_regs.sadts));
	save_item(NAME(m_audio_regs.sadtc));
	save_item(NAME(m_audio_regs.sadta));
	save_item(NAME(m_audio_regs.sadtcc));
	save_item(NAME(m_audio_regs.sadrcs));
	save_item(NAME(m_audio_regs.sadrs));
	save_item(NAME(m_audio_regs.sadrc));
	save_item(NAME(m_audio_regs.sadra));
	save_item(NAME(m_audio_regs.sadrcc));
	save_item(NAME(m_audio_regs.saitr));
	save_item(NAME(m_audio_regs.rx_fifo));
	save_item(NAME(m_audio_regs.rx_fifo_read_idx));
	save_item(NAME(m_audio_regs.rx_fifo_write_idx));
	save_item(NAME(m_audio_regs.rx_fifo_count));
	save_item(NAME(m_audio_regs.tx_fifo));
	save_item(NAME(m_audio_regs.tx_fifo_read_idx));
	save_item(NAME(m_audio_regs.tx_fifo_write_idx));
	save_item(NAME(m_audio_regs.tx_fifo_count));

	save_item(NAME(m_ssp_regs.sspcr0));
	save_item(NAME(m_ssp_regs.sspcr1));
	save_item(NAME(m_ssp_regs.sspsr));
	save_item(NAME(m_ssp_regs.sspitr));
	save_item(NAME(m_ssp_regs.rx_fifo));
	save_item(NAME(m_ssp_regs.rx_fifo_read_idx));
	save_item(NAME(m_ssp_regs.rx_fifo_write_idx));
	save_item(NAME(m_ssp_regs.rx_fifo_count));
	save_item(NAME(m_ssp_regs.tx_fifo));
	save_item(NAME(m_ssp_regs.tx_fifo_read_idx));
	save_item(NAME(m_ssp_regs.tx_fifo_write_idx));
	save_item(NAME(m_ssp_regs.tx_fifo_count));

	save_item(NAME(m_track_regs.kbdcr));
	save_item(NAME(m_track_regs.kbdstat));
	save_item(NAME(m_track_regs.kbddata_tx));
	save_item(NAME(m_track_regs.kbddata_rx));
	save_item(NAME(m_track_regs.kbdclkdiv));
	save_item(NAME(m_track_regs.kbdprecnt));
	save_item(NAME(m_track_regs.kbditr));

	save_item(NAME(m_mouse_regs.kbdcr));
	save_item(NAME(m_mouse_regs.kbdstat));
	save_item(NAME(m_mouse_regs.kbddata_tx));
	save_item(NAME(m_mouse_regs.kbddata_rx));
	save_item(NAME(m_mouse_regs.kbdclkdiv));
	save_item(NAME(m_mouse_regs.kbdprecnt));
	save_item(NAME(m_mouse_regs.kbditr));

	save_item(NAME(m_gpio_regs.ddr));
	save_item(NAME(m_gpio_regs.level));
	save_item(NAME(m_gpio_regs.sdr));
	save_item(NAME(m_gpio_regs.ssr));
	save_item(NAME(m_gpio_regs.out_latch));
	save_item(NAME(m_gpio_regs.in_latch));

	save_item(NAME(m_intc_regs.inttest));
	save_item(NAME(m_intc_regs.inten));
	save_item(NAME(m_intc_regs.intpol));
	save_item(NAME(m_intc_regs.inttstsel));
	save_item(NAME(m_intc_regs.intstat));
	save_item(NAME(m_intc_regs.wake_en));
	save_item(NAME(m_intc_regs.wake_pol));
	save_item(NAME(m_intc_regs.intraw));

	save_item(NAME(m_card_regs.pccr));
	save_item(NAME(m_card_regs.pcssr));
	save_item(NAME(m_card_regs.pcsr));

	m_audio_regs.rx_timer = timer_alloc(FUNC(sa1111_device::audio_rx_callback), this);
	m_audio_regs.rx_dma_timer = timer_alloc(FUNC(sa1111_device::audio_rx_dma_callback), this);
	m_audio_regs.tx_timer = timer_alloc(FUNC(sa1111_device::audio_tx_callback), this);
	m_audio_regs.tx_dma_timer = timer_alloc(FUNC(sa1111_device::audio_tx_dma_callback), this);

	m_ssp_regs.rx_timer = timer_alloc(FUNC(sa1111_device::ssp_rx_callback), this);
	m_ssp_regs.tx_timer = timer_alloc(FUNC(sa1111_device::ssp_tx_callback), this);
}

void sa1111_device::device_reset()
{
	m_sbi_regs.skcr = (1 << SKCR_RDY_BIT);
	m_sbi_regs.smcr = (1 << SMCR_CLAT_BIT) | (5 << SMCR_DRAC_BIT) | (1 << SMCR_DTIM_BIT);
	m_sbi_regs.skid = 0x690cc211;

	m_sk_regs.skpcr = 0;
	m_sk_regs.skcdr = (0x4c << SKCDR_FBD_BIT);
	m_sk_regs.skaud = (0x18 << SKAUD_ACD_BIT);
	m_sk_regs.skpmc = (0x11 << SKPMC_PMCD_BIT);
	m_sk_regs.skptc = (0x11 << SKPTC_PTCD_BIT);
	m_sk_regs.skpen0 = 0;
	m_sk_regs.skpwm0 = 0;
	m_sk_regs.skpen1 = 0;
	m_sk_regs.skpwm1 = 0;

	std::fill(std::begin(m_usb_regs.ohci), std::end(m_usb_regs.ohci), 0);
	m_usb_regs.status = 0;
	m_usb_regs.reset = (1 << USBRST_FHR_BIT) | (1 << USBRST_FIR_BIT);
	m_usb_regs.int_test = 0;
	std::fill(std::begin(m_usb_regs.fifo), std::end(m_usb_regs.fifo), 0);

	m_audio_regs.sacr0 = (0x7 << SACR0_RFTH_BIT) | (0x7 << SACR0_TFTH_BIT);
	m_audio_regs.sacr1 = 0;
	m_audio_regs.sacr2 = 0;
	m_audio_regs.sasr0 = 0;
	m_audio_regs.sasr1 = 0;
	m_audio_regs.l3car = 0;
	m_audio_regs.l3cdr = 0;
	m_audio_regs.accar = 0;
	m_audio_regs.accdr = 0;
	m_audio_regs.acsar = 0;
	m_audio_regs.acsdr = 0;
	m_audio_regs.sadtcs = 0;
	std::fill_n(&m_audio_regs.sadts[0], 2, 0);
	std::fill_n(&m_audio_regs.sadtc[0], 2, 0);
	m_audio_regs.sadta = 0;
	m_audio_regs.sadtcc = 0;
	m_audio_regs.sadrcs = 0;
	std::fill_n(&m_audio_regs.sadrs[0], 2, 0);
	std::fill_n(&m_audio_regs.sadrc[0], 2, 0);
	m_audio_regs.sadra = 0;
	m_audio_regs.sadrcc = 0;
	m_audio_regs.saitr = 0;
	std::fill(std::begin(m_audio_regs.rx_fifo), std::end(m_audio_regs.rx_fifo), 0);
	m_audio_regs.rx_fifo_read_idx = 0;
	m_audio_regs.rx_fifo_write_idx = 0;
	m_audio_regs.rx_fifo_count = 0;
	m_audio_regs.rx_timer->adjust(attotime::never);
	m_audio_regs.rx_dma_timer->adjust(attotime::never);
	std::fill(std::begin(m_audio_regs.tx_fifo), std::end(m_audio_regs.tx_fifo), 0);
	m_audio_regs.tx_fifo_read_idx = 0;
	m_audio_regs.tx_fifo_write_idx = 0;
	m_audio_regs.tx_fifo_count = 0;
	m_audio_regs.tx_timer->adjust(attotime::never);
	m_audio_regs.tx_dma_timer->adjust(attotime::never);

	m_ssp_regs.sspcr0 = 0;
	m_ssp_regs.sspcr1 = (0x7 << SSPCR1_RFT_BIT) | (0x7 << SSPCR1_TFT_BIT);
	m_ssp_regs.sspsr = 0;
	m_ssp_regs.sspitr = 0;
	std::fill(std::begin(m_ssp_regs.rx_fifo), std::end(m_ssp_regs.rx_fifo), 0);
	m_ssp_regs.rx_fifo_read_idx = 0;
	m_ssp_regs.rx_fifo_write_idx = 0;
	m_ssp_regs.rx_fifo_count = 0;
	m_ssp_regs.rx_timer->adjust(attotime::never);
	std::fill(std::begin(m_ssp_regs.tx_fifo), std::end(m_ssp_regs.tx_fifo), 0);
	m_ssp_regs.tx_fifo_read_idx = 0;
	m_ssp_regs.tx_fifo_write_idx = 0;
	m_ssp_regs.tx_fifo_count = 0;
	m_ssp_regs.tx_timer->adjust(attotime::never);

	for (ps2_regs &regs : { std::ref(m_track_regs), std::ref(m_mouse_regs) })
	{
		regs.kbdcr = 0;
		regs.kbdstat = 0;
		regs.kbddata_tx = 0;
		regs.kbddata_rx = 0;
		regs.kbdclkdiv = 0;
		regs.kbdprecnt = 0;
		regs.kbditr = 0;
	}

	std::fill(std::begin(m_gpio_regs.ddr), std::end(m_gpio_regs.ddr), 0);
	std::fill(std::begin(m_gpio_regs.level), std::end(m_gpio_regs.level), 0);
	std::fill(std::begin(m_gpio_regs.sdr), std::end(m_gpio_regs.sdr), 0);
	std::fill(std::begin(m_gpio_regs.ssr), std::end(m_gpio_regs.ssr), 0);
	std::fill(std::begin(m_gpio_regs.out_latch), std::end(m_gpio_regs.out_latch), 0);
	std::fill(std::begin(m_gpio_regs.in_latch), std::end(m_gpio_regs.in_latch), 0);

	std::fill(std::begin(m_intc_regs.inttest), std::end(m_intc_regs.inttest), 0);
	std::fill(std::begin(m_intc_regs.inten), std::end(m_intc_regs.inten), 0);
	std::fill(std::begin(m_intc_regs.intpol), std::end(m_intc_regs.intpol), 0);
	m_intc_regs.inttstsel = 0;
	std::fill(std::begin(m_intc_regs.intstat), std::end(m_intc_regs.intstat), 0);
	std::fill(std::begin(m_intc_regs.wake_en), std::end(m_intc_regs.wake_en), 0);
	std::fill(std::begin(m_intc_regs.wake_pol), std::end(m_intc_regs.wake_pol), 0);
	std::fill(std::begin(m_intc_regs.intraw), std::end(m_intc_regs.intraw), 0);

	m_card_regs.pccr = 0;
	m_card_regs.pcssr = 0;
	m_card_regs.pcsr = 0x000000fc;
}

void sa1111_device::device_add_mconfig(machine_config &config)
{
}
