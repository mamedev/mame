// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Intel XScale SA1110 peripheral emulation

***************************************************************************/

#include "emu.h"
#include "sa1110.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_ICP         (1U << 2)
#define LOG_UART3       (1U << 3)
#define LOG_UART3_HF    (1U << 4)
#define LOG_MCP         (1U << 5)
#define LOG_SSP         (1U << 6)
#define LOG_OSTIMER     (1U << 7)
#define LOG_OSTIMER_HF  (1U << 8)
#define LOG_RTC         (1U << 9)
#define LOG_RTC_HF      (1U << 10)
#define LOG_POWER       (1U << 11)
#define LOG_POWER_HF    (1U << 12)
#define LOG_RESET       (1U << 13)
#define LOG_GPIO        (1U << 14)
#define LOG_GPIO_HF     (1U << 15)
#define LOG_INTC        (1U << 16)
#define LOG_PPC         (1U << 17)
#define LOG_DMA         (1U << 18)
#define LOG_UDC         (1U << 19)
#define LOG_ALL         (LOG_UNKNOWN | LOG_ICP | LOG_UART3 | LOG_MCP | LOG_OSTIMER | LOG_RTC | LOG_POWER | LOG_RESET | LOG_GPIO | LOG_INTC | LOG_PPC | LOG_DMA | LOG_UDC)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SA1110_PERIPHERALS, sa1110_periphs_device, "sa1110_periphs", "Intel XScale SA1110 Peripherals")

sa1110_periphs_device::sa1110_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SA1110_PERIPHERALS, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_uart3_irqs(*this, "uart3irq")
	, m_mcp_irqs(*this, "mcpirq")
	, m_codec(*this, finder_base::DUMMY_TAG)
	, m_gpio_out(*this)
	, m_ssp_out(*this)
	, m_uart3_tx_out(*this)
{
}

void sa1110_periphs_device::map(address_map &map)
{
	map(0x00000000, 0x00000003).rw(FUNC(sa1110_periphs_device::udc_udccr_r), FUNC(sa1110_periphs_device::udc_udccr_w));
	map(0x00000004, 0x00000007).rw(FUNC(sa1110_periphs_device::udc_udcar_r), FUNC(sa1110_periphs_device::udc_udcar_w));
	map(0x00000008, 0x0000000b).rw(FUNC(sa1110_periphs_device::udc_udcomp_r), FUNC(sa1110_periphs_device::udc_udcomp_w));
	map(0x0000000c, 0x0000000f).rw(FUNC(sa1110_periphs_device::udc_udcimp_r), FUNC(sa1110_periphs_device::udc_udcimp_w));
	map(0x00000010, 0x00000013).rw(FUNC(sa1110_periphs_device::udc_udccs0_r), FUNC(sa1110_periphs_device::udc_udccs0_w));
	map(0x00000014, 0x00000017).rw(FUNC(sa1110_periphs_device::udc_udccs1_r), FUNC(sa1110_periphs_device::udc_udccs1_w));
	map(0x00000018, 0x0000001b).rw(FUNC(sa1110_periphs_device::udc_udccs2_r), FUNC(sa1110_periphs_device::udc_udccs2_w));
	map(0x0000001c, 0x0000001f).rw(FUNC(sa1110_periphs_device::udc_udcd0_r), FUNC(sa1110_periphs_device::udc_udcd0_w));
	map(0x00000020, 0x00000023).rw(FUNC(sa1110_periphs_device::udc_udcwc_r), FUNC(sa1110_periphs_device::udc_udcwc_w));
	map(0x00000028, 0x0000002b).rw(FUNC(sa1110_periphs_device::udc_udcdr_r), FUNC(sa1110_periphs_device::udc_udcdr_w));
	map(0x00000030, 0x00000033).rw(FUNC(sa1110_periphs_device::udc_udcsr_r), FUNC(sa1110_periphs_device::udc_udcsr_w));

	map(0x00030000, 0x00030003).rw(FUNC(sa1110_periphs_device::icp_utcr0_r), FUNC(sa1110_periphs_device::icp_utcr0_w));
	map(0x00030004, 0x00030007).rw(FUNC(sa1110_periphs_device::icp_utcr1_r), FUNC(sa1110_periphs_device::icp_utcr1_w));
	map(0x00030008, 0x0003000b).rw(FUNC(sa1110_periphs_device::icp_utcr2_r), FUNC(sa1110_periphs_device::icp_utcr2_w));
	map(0x0003000c, 0x0003000f).rw(FUNC(sa1110_periphs_device::icp_utcr3_r), FUNC(sa1110_periphs_device::icp_utcr3_w));
	map(0x00030010, 0x00030013).rw(FUNC(sa1110_periphs_device::icp_utcr4_r), FUNC(sa1110_periphs_device::icp_utcr4_w));
	map(0x00030014, 0x00030017).rw(FUNC(sa1110_periphs_device::icp_utdr_r), FUNC(sa1110_periphs_device::icp_utdr_w));
	map(0x0003001c, 0x0003001f).rw(FUNC(sa1110_periphs_device::icp_utcr0_r), FUNC(sa1110_periphs_device::icp_utcr0_w));
	map(0x00030020, 0x00030023).r(FUNC(sa1110_periphs_device::icp_utcr1_r));
	map(0x00030060, 0x00030063).rw(FUNC(sa1110_periphs_device::icp_hscr0_r), FUNC(sa1110_periphs_device::icp_hscr0_w));
	map(0x00030064, 0x00030067).rw(FUNC(sa1110_periphs_device::icp_hscr1_r), FUNC(sa1110_periphs_device::icp_hscr1_w));
	map(0x0003006c, 0x0003006f).rw(FUNC(sa1110_periphs_device::icp_hsdr_r), FUNC(sa1110_periphs_device::icp_hsdr_w));
	map(0x00030074, 0x00030077).rw(FUNC(sa1110_periphs_device::icp_hssr0_r), FUNC(sa1110_periphs_device::icp_hssr0_w));
	map(0x00030078, 0x0003007b).rw(FUNC(sa1110_periphs_device::icp_hssr1_r), FUNC(sa1110_periphs_device::icp_hssr1_w));

	map(0x00050000, 0x00050003).rw(FUNC(sa1110_periphs_device::uart3_utcr0_r), FUNC(sa1110_periphs_device::uart3_utcr0_w));
	map(0x00050004, 0x00050007).rw(FUNC(sa1110_periphs_device::uart3_utcr1_r), FUNC(sa1110_periphs_device::uart3_utcr1_w));
	map(0x00050008, 0x0005000b).rw(FUNC(sa1110_periphs_device::uart3_utcr2_r), FUNC(sa1110_periphs_device::uart3_utcr2_w));
	map(0x0005000c, 0x0005000f).rw(FUNC(sa1110_periphs_device::uart3_utcr3_r), FUNC(sa1110_periphs_device::uart3_utcr3_w));
	map(0x00050014, 0x00050017).rw(FUNC(sa1110_periphs_device::uart3_utdr_r), FUNC(sa1110_periphs_device::uart3_utdr_w));
	map(0x0005001c, 0x0005001f).r(FUNC(sa1110_periphs_device::uart3_utsr0_r));
	map(0x00050020, 0x00050023).rw(FUNC(sa1110_periphs_device::uart3_utsr1_r), FUNC(sa1110_periphs_device::uart3_utsr1_w));

	map(0x00060000, 0x00060003).rw(FUNC(sa1110_periphs_device::mcp_mccr0_r), FUNC(sa1110_periphs_device::mcp_mccr0_w));
	map(0x00060008, 0x0006000b).rw(FUNC(sa1110_periphs_device::mcp_mcdr0_r), FUNC(sa1110_periphs_device::mcp_mcdr0_w));
	map(0x0006000c, 0x0006000f).rw(FUNC(sa1110_periphs_device::mcp_mcdr1_r), FUNC(sa1110_periphs_device::mcp_mcdr1_w));
	map(0x00060010, 0x00060013).rw(FUNC(sa1110_periphs_device::mcp_mcdr2_r), FUNC(sa1110_periphs_device::mcp_mcdr2_w));
	map(0x00060018, 0x0006001b).rw(FUNC(sa1110_periphs_device::mcp_mcsr_r), FUNC(sa1110_periphs_device::mcp_mcsr_w));

	map(0x00070060, 0x00070063).rw(FUNC(sa1110_periphs_device::ssp_sscr0_r), FUNC(sa1110_periphs_device::ssp_sscr0_w));
	map(0x00070064, 0x00070067).rw(FUNC(sa1110_periphs_device::ssp_sscr1_r), FUNC(sa1110_periphs_device::ssp_sscr1_w));
	map(0x0007006c, 0x0007006f).rw(FUNC(sa1110_periphs_device::ssp_ssdr_r), FUNC(sa1110_periphs_device::ssp_ssdr_w));
	map(0x00070074, 0x00070077).rw(FUNC(sa1110_periphs_device::ssp_sssr_r), FUNC(sa1110_periphs_device::ssp_sssr_w));

	map(0x10000000, 0x10000003).rw(FUNC(sa1110_periphs_device::tmr_osmr0_r), FUNC(sa1110_periphs_device::tmr_osmr0_w));
	map(0x10000004, 0x10000007).rw(FUNC(sa1110_periphs_device::tmr_osmr1_r), FUNC(sa1110_periphs_device::tmr_osmr1_w));
	map(0x10000008, 0x1000000b).rw(FUNC(sa1110_periphs_device::tmr_osmr2_r), FUNC(sa1110_periphs_device::tmr_osmr2_w));
	map(0x1000000c, 0x1000000f).rw(FUNC(sa1110_periphs_device::tmr_osmr3_r), FUNC(sa1110_periphs_device::tmr_osmr3_w));
	map(0x10000010, 0x10000013).rw(FUNC(sa1110_periphs_device::tmr_oscr_r), FUNC(sa1110_periphs_device::tmr_oscr_w));
	map(0x10000014, 0x10000017).rw(FUNC(sa1110_periphs_device::tmr_ossr_r), FUNC(sa1110_periphs_device::tmr_ossr_w));
	map(0x10000018, 0x1000001b).rw(FUNC(sa1110_periphs_device::tmr_ower_r), FUNC(sa1110_periphs_device::tmr_ower_w));
	map(0x1000001c, 0x1000001f).rw(FUNC(sa1110_periphs_device::tmr_oier_r), FUNC(sa1110_periphs_device::tmr_oier_w));

	map(0x10010000, 0x10010003).rw(FUNC(sa1110_periphs_device::rtc_rtar_r), FUNC(sa1110_periphs_device::rtc_rtar_w));
	map(0x10010004, 0x10010007).rw(FUNC(sa1110_periphs_device::rtc_rcnr_r), FUNC(sa1110_periphs_device::rtc_rcnr_w));
	map(0x10010008, 0x1001000b).rw(FUNC(sa1110_periphs_device::rtc_rttr_r), FUNC(sa1110_periphs_device::rtc_rttr_w));
	map(0x10010010, 0x10010013).rw(FUNC(sa1110_periphs_device::rtc_rtsr_r), FUNC(sa1110_periphs_device::rtc_rtsr_w));

	map(0x10020000, 0x10020003).rw(FUNC(sa1110_periphs_device::pwr_pmcr_r), FUNC(sa1110_periphs_device::pwr_pmcr_w));
	map(0x10020004, 0x10020007).rw(FUNC(sa1110_periphs_device::pwr_pssr_r), FUNC(sa1110_periphs_device::pwr_pssr_w));
	map(0x10020008, 0x1002000b).rw(FUNC(sa1110_periphs_device::pwr_pspr_r), FUNC(sa1110_periphs_device::pwr_pspr_w));
	map(0x1002000c, 0x1002000f).rw(FUNC(sa1110_periphs_device::pwr_pwer_r), FUNC(sa1110_periphs_device::pwr_pwer_w));
	map(0x10020010, 0x10020013).rw(FUNC(sa1110_periphs_device::pwr_pcfr_r), FUNC(sa1110_periphs_device::pwr_pcfr_w));
	map(0x10020014, 0x10020017).rw(FUNC(sa1110_periphs_device::pwr_ppcr_r), FUNC(sa1110_periphs_device::pwr_ppcr_w));
	map(0x10020018, 0x1002001b).rw(FUNC(sa1110_periphs_device::pwr_pgsr_r), FUNC(sa1110_periphs_device::pwr_pgsr_w));
	map(0x1002001c, 0x1002001f).rw(FUNC(sa1110_periphs_device::pwr_posr_r), FUNC(sa1110_periphs_device::pwr_posr_w));

	map(0x10030000, 0x10030003).rw(FUNC(sa1110_periphs_device::rst_rsrr_r), FUNC(sa1110_periphs_device::rst_rsrr_w));
	map(0x10030004, 0x10030007).rw(FUNC(sa1110_periphs_device::rst_rcsr_r), FUNC(sa1110_periphs_device::rst_rcsr_w));

	map(0x10040000, 0x10040003).rw(FUNC(sa1110_periphs_device::gpio_gplr_r), FUNC(sa1110_periphs_device::gpio_gplr_w));
	map(0x10040004, 0x10040007).rw(FUNC(sa1110_periphs_device::gpio_gpdr_r), FUNC(sa1110_periphs_device::gpio_gpdr_w));
	map(0x10040008, 0x1004000b).rw(FUNC(sa1110_periphs_device::gpio_gpsr_r), FUNC(sa1110_periphs_device::gpio_gpsr_w));
	map(0x1004000c, 0x1004000f).rw(FUNC(sa1110_periphs_device::gpio_gpcr_r), FUNC(sa1110_periphs_device::gpio_gpcr_w));
	map(0x10040010, 0x10040013).rw(FUNC(sa1110_periphs_device::gpio_grer_r), FUNC(sa1110_periphs_device::gpio_grer_w));
	map(0x10040014, 0x10040017).rw(FUNC(sa1110_periphs_device::gpio_gfer_r), FUNC(sa1110_periphs_device::gpio_gfer_w));
	map(0x10040018, 0x1004001b).rw(FUNC(sa1110_periphs_device::gpio_gedr_r), FUNC(sa1110_periphs_device::gpio_gedr_w));
	map(0x1004001c, 0x1004001f).rw(FUNC(sa1110_periphs_device::gpio_gafr_r), FUNC(sa1110_periphs_device::gpio_gafr_w));

	map(0x10050000, 0x10050003).rw(FUNC(sa1110_periphs_device::intc_icip_r), FUNC(sa1110_periphs_device::intc_icip_w));
	map(0x10050004, 0x10050007).rw(FUNC(sa1110_periphs_device::intc_icmr_r), FUNC(sa1110_periphs_device::intc_icmr_w));
	map(0x10050008, 0x1005000b).rw(FUNC(sa1110_periphs_device::intc_iclr_r), FUNC(sa1110_periphs_device::intc_iclr_w));
	map(0x1005000c, 0x1005000f).rw(FUNC(sa1110_periphs_device::intc_iccr_r), FUNC(sa1110_periphs_device::intc_iccr_w));
	map(0x10050010, 0x10050013).rw(FUNC(sa1110_periphs_device::intc_icfp_r), FUNC(sa1110_periphs_device::intc_icfp_w));
	map(0x10050014, 0x10050017).rw(FUNC(sa1110_periphs_device::intc_icpr_r), FUNC(sa1110_periphs_device::intc_icpr_w));

	map(0x10060000, 0x10060003).rw(FUNC(sa1110_periphs_device::ppc_ppdr_r), FUNC(sa1110_periphs_device::ppc_ppdr_w));
	map(0x10060004, 0x10060007).rw(FUNC(sa1110_periphs_device::ppc_ppsr_r), FUNC(sa1110_periphs_device::ppc_ppsr_w));
	map(0x10060008, 0x1006000b).rw(FUNC(sa1110_periphs_device::ppc_ppar_r), FUNC(sa1110_periphs_device::ppc_ppar_w));
	map(0x1006000c, 0x1006000f).rw(FUNC(sa1110_periphs_device::ppc_psdr_r), FUNC(sa1110_periphs_device::ppc_psdr_w));
	map(0x10060010, 0x10060013).rw(FUNC(sa1110_periphs_device::ppc_ppfr_r), FUNC(sa1110_periphs_device::ppc_ppfr_w));

	map(0x30000000, 0x30000003).rw(FUNC(sa1110_periphs_device::dma_ddar_r<0>), FUNC(sa1110_periphs_device::dma_ddar_w<0>));
	map(0x30000004, 0x30000007).rw(FUNC(sa1110_periphs_device::dma_dssr_r<0>), FUNC(sa1110_periphs_device::dma_dssr_w<0>));
	map(0x30000008, 0x3000000b).rw(FUNC(sa1110_periphs_device::dma_dcsr_r<0>), FUNC(sa1110_periphs_device::dma_dcsr_w<0>));
	map(0x3000000c, 0x3000000f).rw(FUNC(sa1110_periphs_device::dma_dsr_r<0>), FUNC(sa1110_periphs_device::dma_dsr_w<0>));
	map(0x30000010, 0x30000013).rw(FUNC(sa1110_periphs_device::dma_dbsa_r<0>), FUNC(sa1110_periphs_device::dma_dbsa_w<0>));
	map(0x30000014, 0x30000017).rw(FUNC(sa1110_periphs_device::dma_dbta_r<0>), FUNC(sa1110_periphs_device::dma_dbta_w<0>));
	map(0x30000018, 0x3000001b).rw(FUNC(sa1110_periphs_device::dma_dbsb_r<0>), FUNC(sa1110_periphs_device::dma_dbsb_w<0>));
	map(0x3000001c, 0x3000001f).rw(FUNC(sa1110_periphs_device::dma_dbtb_r<0>), FUNC(sa1110_periphs_device::dma_dbtb_w<0>));
	map(0x30000020, 0x30000023).rw(FUNC(sa1110_periphs_device::dma_ddar_r<1>), FUNC(sa1110_periphs_device::dma_ddar_w<1>));
	map(0x30000024, 0x30000027).rw(FUNC(sa1110_periphs_device::dma_dssr_r<1>), FUNC(sa1110_periphs_device::dma_dssr_w<1>));
	map(0x30000028, 0x3000002b).rw(FUNC(sa1110_periphs_device::dma_dcsr_r<1>), FUNC(sa1110_periphs_device::dma_dcsr_w<1>));
	map(0x3000002c, 0x3000002f).rw(FUNC(sa1110_periphs_device::dma_dsr_r<1>), FUNC(sa1110_periphs_device::dma_dsr_w<1>));
	map(0x30000030, 0x30000033).rw(FUNC(sa1110_periphs_device::dma_dbsa_r<1>), FUNC(sa1110_periphs_device::dma_dbsa_w<1>));
	map(0x30000034, 0x30000037).rw(FUNC(sa1110_periphs_device::dma_dbta_r<1>), FUNC(sa1110_periphs_device::dma_dbta_w<1>));
	map(0x30000038, 0x3000003b).rw(FUNC(sa1110_periphs_device::dma_dbsb_r<1>), FUNC(sa1110_periphs_device::dma_dbsb_w<1>));
	map(0x3000003c, 0x3000003f).rw(FUNC(sa1110_periphs_device::dma_dbtb_r<1>), FUNC(sa1110_periphs_device::dma_dbtb_w<1>));
	map(0x30000040, 0x30000043).rw(FUNC(sa1110_periphs_device::dma_ddar_r<2>), FUNC(sa1110_periphs_device::dma_ddar_w<2>));
	map(0x30000044, 0x30000047).rw(FUNC(sa1110_periphs_device::dma_dssr_r<2>), FUNC(sa1110_periphs_device::dma_dssr_w<2>));
	map(0x30000048, 0x3000004b).rw(FUNC(sa1110_periphs_device::dma_dcsr_r<2>), FUNC(sa1110_periphs_device::dma_dcsr_w<2>));
	map(0x3000004c, 0x3000004f).rw(FUNC(sa1110_periphs_device::dma_dsr_r<2>), FUNC(sa1110_periphs_device::dma_dsr_w<2>));
	map(0x30000050, 0x30000053).rw(FUNC(sa1110_periphs_device::dma_dbsa_r<2>), FUNC(sa1110_periphs_device::dma_dbsa_w<2>));
	map(0x30000054, 0x30000057).rw(FUNC(sa1110_periphs_device::dma_dbta_r<2>), FUNC(sa1110_periphs_device::dma_dbta_w<2>));
	map(0x30000058, 0x3000005b).rw(FUNC(sa1110_periphs_device::dma_dbsb_r<2>), FUNC(sa1110_periphs_device::dma_dbsb_w<2>));
	map(0x3000005c, 0x3000005f).rw(FUNC(sa1110_periphs_device::dma_dbtb_r<2>), FUNC(sa1110_periphs_device::dma_dbtb_w<2>));
	map(0x30000060, 0x30000063).rw(FUNC(sa1110_periphs_device::dma_ddar_r<3>), FUNC(sa1110_periphs_device::dma_ddar_w<3>));
	map(0x30000064, 0x30000067).rw(FUNC(sa1110_periphs_device::dma_dssr_r<3>), FUNC(sa1110_periphs_device::dma_dssr_w<3>));
	map(0x30000068, 0x3000006b).rw(FUNC(sa1110_periphs_device::dma_dcsr_r<3>), FUNC(sa1110_periphs_device::dma_dcsr_w<3>));
	map(0x3000006c, 0x3000006f).rw(FUNC(sa1110_periphs_device::dma_dsr_r<3>), FUNC(sa1110_periphs_device::dma_dsr_w<3>));
	map(0x30000070, 0x30000073).rw(FUNC(sa1110_periphs_device::dma_dbsa_r<3>), FUNC(sa1110_periphs_device::dma_dbsa_w<3>));
	map(0x30000074, 0x30000077).rw(FUNC(sa1110_periphs_device::dma_dbta_r<3>), FUNC(sa1110_periphs_device::dma_dbta_w<3>));
	map(0x30000078, 0x3000007b).rw(FUNC(sa1110_periphs_device::dma_dbsb_r<3>), FUNC(sa1110_periphs_device::dma_dbsb_w<3>));
	map(0x3000007c, 0x3000007f).rw(FUNC(sa1110_periphs_device::dma_dbtb_r<3>), FUNC(sa1110_periphs_device::dma_dbtb_w<3>));
	map(0x30000080, 0x30000083).rw(FUNC(sa1110_periphs_device::dma_ddar_r<4>), FUNC(sa1110_periphs_device::dma_ddar_w<4>));
	map(0x30000084, 0x30000087).rw(FUNC(sa1110_periphs_device::dma_dssr_r<4>), FUNC(sa1110_periphs_device::dma_dssr_w<4>));
	map(0x30000088, 0x3000008b).rw(FUNC(sa1110_periphs_device::dma_dcsr_r<4>), FUNC(sa1110_periphs_device::dma_dcsr_w<4>));
	map(0x3000008c, 0x3000008f).rw(FUNC(sa1110_periphs_device::dma_dsr_r<4>), FUNC(sa1110_periphs_device::dma_dsr_w<4>));
	map(0x30000090, 0x30000093).rw(FUNC(sa1110_periphs_device::dma_dbsa_r<4>), FUNC(sa1110_periphs_device::dma_dbsa_w<4>));
	map(0x30000094, 0x30000097).rw(FUNC(sa1110_periphs_device::dma_dbta_r<4>), FUNC(sa1110_periphs_device::dma_dbta_w<4>));
	map(0x30000098, 0x3000009b).rw(FUNC(sa1110_periphs_device::dma_dbsb_r<4>), FUNC(sa1110_periphs_device::dma_dbsb_w<4>));
	map(0x3000009c, 0x3000009f).rw(FUNC(sa1110_periphs_device::dma_dbtb_r<4>), FUNC(sa1110_periphs_device::dma_dbtb_w<4>));
	map(0x300000a0, 0x300000a3).rw(FUNC(sa1110_periphs_device::dma_ddar_r<5>), FUNC(sa1110_periphs_device::dma_ddar_w<5>));
	map(0x300000a4, 0x300000a7).rw(FUNC(sa1110_periphs_device::dma_dssr_r<5>), FUNC(sa1110_periphs_device::dma_dssr_w<5>));
	map(0x300000a8, 0x300000ab).rw(FUNC(sa1110_periphs_device::dma_dcsr_r<5>), FUNC(sa1110_periphs_device::dma_dcsr_w<5>));
	map(0x300000ac, 0x300000af).rw(FUNC(sa1110_periphs_device::dma_dsr_r<5>), FUNC(sa1110_periphs_device::dma_dsr_w<5>));
	map(0x300000b0, 0x300000b3).rw(FUNC(sa1110_periphs_device::dma_dbsa_r<5>), FUNC(sa1110_periphs_device::dma_dbsa_w<5>));
	map(0x300000b4, 0x300000b7).rw(FUNC(sa1110_periphs_device::dma_dbta_r<5>), FUNC(sa1110_periphs_device::dma_dbta_w<5>));
	map(0x300000b8, 0x300000bb).rw(FUNC(sa1110_periphs_device::dma_dbsb_r<5>), FUNC(sa1110_periphs_device::dma_dbsb_w<5>));
	map(0x300000bc, 0x300000bf).rw(FUNC(sa1110_periphs_device::dma_dbtb_r<5>), FUNC(sa1110_periphs_device::dma_dbtb_w<5>));
}

/*

  Intel SA-1110 UDC - USB Device Controller

  pg. 235 to 258 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

u32 sa1110_periphs_device::udc_udccr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udccr;
	LOGMASKED(LOG_UDC, "%s: udc_udccr_r: UDC Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udccr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udccr_w: UDC Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udccr);
}

u32 sa1110_periphs_device::udc_udcar_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udcar;
	LOGMASKED(LOG_UDC, "%s: udc_udcar_r: UDC Address Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udcar_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udcar_w: UDC Address Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udcar);
}

u32 sa1110_periphs_device::udc_udcomp_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udcomp;
	LOGMASKED(LOG_UDC, "%s: udc_udcomp_r: UDC OUT Max Packet Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udcomp_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udcomp_w: UDC OUT Max Packet Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udcomp);
}

u32 sa1110_periphs_device::udc_udcimp_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udcimp;
	LOGMASKED(LOG_UDC, "%s: udc_udiomp_r: UDC IN Max Packet Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udcimp_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udcimp_w: UDC IN Max Packet Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udcimp);
}

u32 sa1110_periphs_device::udc_udccs0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udccs0;
	LOGMASKED(LOG_UDC, "%s: udc_udccs0_r: UDC Endpoint 0 Control/Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udccs0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udccs0_w: UDC Endpoint 0 Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udccs0);
}

u32 sa1110_periphs_device::udc_udccs1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udccs1;
	LOGMASKED(LOG_UDC, "%s: udc_udccs1_r: UDC Endpoint 1 (OUT) Control/Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udccs1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udccs1_w: UDC Endpoint 1 (OUT) Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udccs1);
}

u32 sa1110_periphs_device::udc_udccs2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udccs2;
	LOGMASKED(LOG_UDC, "%s: udc_udccs2_r: UDC Endpoint 2 (IN) Control/Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udccs2_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udccs2_w: UDC Endpoint 2 (IN) Control/Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udccs2);
}

u32 sa1110_periphs_device::udc_udcd0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0;
	LOGMASKED(LOG_UDC, "%s: udc_udcd0_r: UDC Endpoint 0 Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udcd0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udcd0_w: UDC Endpoint 0 Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::udc_udcwc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udcwc;
	LOGMASKED(LOG_UDC, "%s: udc_udcwc_r: UDC Endpoint 0 Write Count Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udcwc_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udcwc_w: UDC Endpoint 0 Write Count Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_udc_regs.udcwc);
}

u32 sa1110_periphs_device::udc_udcdr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0; //udc_rx_fifo_pop();
	LOGMASKED(LOG_UDC, "%s: udc_udcdr_r: UDC Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udcdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udcdr_w: UDC Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::udc_udcsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_udc_regs.udcsr;
	LOGMASKED(LOG_UDC, "%s: udc_udcsr_r: UDC Status/Interrupt Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::udc_udcsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UDC, "%s: udc_udcsr_w: UDC Status/Interrupt Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


/*

  Intel SA-1110 ICP - Serial Port 2

  pg. 264 to 288 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::icp_rx_callback)
{
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::icp_tx_callback)
{
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::hssp_rx_callback)
{
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::hssp_tx_callback)
{
}

void sa1110_periphs_device::icp_uart_set_receiver_enabled(bool enabled)
{
}

void sa1110_periphs_device::icp_uart_set_transmitter_enabled(bool enabled)
{
}

void sa1110_periphs_device::icp_uart_set_receive_irq_enabled(bool enabled)
{
}

void sa1110_periphs_device::icp_uart_set_transmit_irq_enabled(bool enabled)
{
}

u8 sa1110_periphs_device::icp_uart_read_receive_fifo()
{
	return 0;
}

void sa1110_periphs_device::icp_uart_write_transmit_fifo(u8 data)
{
}

u16 sa1110_periphs_device::icp_hssp_read_receive_fifo()
{
	return 0;
}

void sa1110_periphs_device::icp_hssp_write_transmit_fifo(u8 data)
{
}

void sa1110_periphs_device::icp_uart_set_receiver_idle()
{
}

void sa1110_periphs_device::icp_uart_begin_of_break()
{
}

void sa1110_periphs_device::icp_uart_end_of_break()
{
}

u32 sa1110_periphs_device::icp_utcr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.uart.utcr[0];
	LOGMASKED(LOG_ICP, "%s: icp_utcr0_r: UART Control Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_utcr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_utcr0_w: UART Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_ICP, "%s:              Parity Enable: %d\n", machine().describe_context(), BIT(data, 0));
	LOGMASKED(LOG_ICP, "%s:              Parity Mode: %s\n", machine().describe_context(), BIT(data, 1) ? "Even" : "Odd");
	LOGMASKED(LOG_ICP, "%s:              Stop Bits: %d\n", machine().describe_context(), BIT(data, 2) + 1);
	LOGMASKED(LOG_ICP, "%s:              Data Size: %d\n", machine().describe_context(), BIT(data, 3) ? 8 : 7);
	LOGMASKED(LOG_ICP, "%s:              Sample Clock: %s\n", machine().describe_context(), BIT(data, 4) ? "External" : "Internal");
	LOGMASKED(LOG_ICP, "%s:              Receive Edge: %s\n", machine().describe_context(), BIT(data, 5) ? "Falling" : "Rising");
	LOGMASKED(LOG_ICP, "%s:              Transmit Edge: %s\n", machine().describe_context(), BIT(data, 6) ? "Falling" : "Rising");

	//stop_bits_t stop_bits = (BIT(data, 2) ? STOP_BITS_2 : STOP_BITS_1);

	//parity_t parity = PARITY_NONE;
	//if (BIT(data, 0))
	//{
	//  parity = (BIT(data, 1) ? PARITY_EVEN : PARITY_ODD);
	//}

	//set_data_frame(1, BIT(data, 3) ? 8 : 7, parity, stop_bits);
	//receive_register_reset();
	//transmit_register_reset();

	COMBINE_DATA(&m_icp_regs.uart.utcr[0]);
}

u32 sa1110_periphs_device::icp_utcr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.uart.utcr[1];
	LOGMASKED(LOG_ICP, "%s: icp_utcr1_r: UART Control Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_utcr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_utcr1_w: UART Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_ICP, "%s:              Baud Rate Divisor MSB: %02x\n", machine().describe_context(), data & 0x0f);
	//const u8 old = m_uart_regs.utcr[1] & 0x0f;
	COMBINE_DATA(&m_icp_regs.uart.utcr[1]);
	//if ((m_uart_regs.utcr[1] & 0x0f) != old)
	//  icp_uart_recalculate_divisor();
}

u32 sa1110_periphs_device::icp_utcr2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.uart.utcr[2];
	LOGMASKED(LOG_ICP, "%s: icp_utcr2_r: UART Control Register 2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_utcr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_utcr2_w: UART Control Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_ICP, "%s:              Baud Rate Divisor LSB: %02x\n", machine().describe_context(), (u8)data);
	//const u8 old = m_uart_regs.utcr[2] & 0xff;
	COMBINE_DATA(&m_icp_regs.uart.utcr[2]);
	//if ((m_uart_regs.utcr[2] & 0xff) != old)
	//  icp_uart_recalculate_divisor();
}

u32 sa1110_periphs_device::icp_utcr3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.uart.utcr[3];
	LOGMASKED(LOG_ICP, "%s: icp_utcr3_r: UART Control Register 3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_utcr3_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_utcr3_w: UART Control Register 3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_ICP, "%s:              Receive Enable: %d\n", machine().describe_context(), BIT(data, 0));
	LOGMASKED(LOG_ICP, "%s:              Transmit Enable: %d\n", machine().describe_context(), BIT(data, 1));
	LOGMASKED(LOG_ICP, "%s:              Send Break: %d\n", machine().describe_context(), BIT(data, 2));
	LOGMASKED(LOG_ICP, "%s:              Receive FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 3));
	LOGMASKED(LOG_ICP, "%s:              Transmit FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 4));
	LOGMASKED(LOG_ICP, "%s:              Loopback Enable: %d\n", machine().describe_context(), BIT(data, 5));
	const u32 old = m_icp_regs.uart.utcr[3];
	COMBINE_DATA(&m_icp_regs.uart.utcr[3]);
	const u32 changed = old ^ m_icp_regs.uart.utcr[3];
	if (BIT(changed, 0))
		icp_uart_set_receiver_enabled(BIT(data, 0));
	if (BIT(changed, 1))
		icp_uart_set_transmitter_enabled(BIT(data, 1));
	if (BIT(changed, 3))
		icp_uart_set_receive_irq_enabled(BIT(data, 3));
	if (BIT(changed, 4))
		icp_uart_set_transmit_irq_enabled(BIT(data, 4));
}

u32 sa1110_periphs_device::icp_utcr4_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.utcr4;
	LOGMASKED(LOG_ICP, "%s: icp_utcr4_r: UART Control Register 4: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_utcr4_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_utcr4_w: UART Control Register 4 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_ICP, "%s:              HP-SIR enable: %d\n", machine().describe_context(), BIT(data, UTCR4_HSE_BIT), mem_mask);
	LOGMASKED(LOG_ICP, "%s:              Low-Power enable: %d\n", machine().describe_context(), BIT(data, UTCR4_LPM_BIT), mem_mask);
	COMBINE_DATA(&m_icp_regs.utcr4);
}

u32 sa1110_periphs_device::icp_utdr_r(offs_t offset, u32 mem_mask)
{
	const u8 data = icp_uart_read_receive_fifo();
	LOGMASKED(LOG_ICP, "%s: icp_utdr_r: UART Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_utdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_utdr_w: UART Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (data == 0x0d || data == 0x0a || (data >= 0x20 && data < 0x7f))
	{
		osd_printf_debug("%c", (char)data);
	}
	icp_uart_write_transmit_fifo((u8)data);
}

u32 sa1110_periphs_device::icp_utsr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.uart.utsr0;
	LOGMASKED(LOG_ICP, "%s: icp_utsr0_r: UART Status Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_utsr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_utsr0_w: UART Status Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_ICP, "%s:              Receiver Idle Status: %d\n", machine().describe_context(), BIT(data, 2));
	LOGMASKED(LOG_ICP, "%s:              Receiver Begin of Break Status: %d\n", machine().describe_context(), BIT(data, 3));
	LOGMASKED(LOG_ICP, "%s:              Receiver End of Break Status: %d\n", machine().describe_context(), BIT(data, 4));
	if (BIT(data, 2))
		icp_uart_set_receiver_idle();
	if (BIT(data, 3))
		icp_uart_begin_of_break();
	if (BIT(data, 4))
		icp_uart_end_of_break();
}

u32 sa1110_periphs_device::icp_utsr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.uart.utsr1;
	LOGMASKED(LOG_ICP, "%s: icp_utsr1_r: UART Status Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

u32 sa1110_periphs_device::icp_hscr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.hssp.hscr0;
	LOGMASKED(LOG_ICP, "%s: icp_hscr0_r: HSSP Control Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_hscr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_hscr0_w: HSSP Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::icp_hscr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.hssp.hscr1;
	LOGMASKED(LOG_ICP, "%s: icp_hscr1_r: HSSP Control Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_hscr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_hscr1_w: HSSP Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::icp_hsdr_r(offs_t offset, u32 mem_mask)
{
	const u16 data = icp_hssp_read_receive_fifo();
	LOGMASKED(LOG_ICP, "%s: icp_r: HSSP Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_hsdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_hsdr_w: HSSP Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	icp_hssp_write_transmit_fifo((u8)data);
}

u32 sa1110_periphs_device::icp_hssr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.hssp.hssr0;
	LOGMASKED(LOG_ICP, "%s: icp_hssr0_r: HSSP Status Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_hssr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_hssr0_w: HSSP Status Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::icp_hssr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_icp_regs.hssp.hssr1;
	LOGMASKED(LOG_ICP, "%s: icp_hssr1_r: HSSP Status Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::icp_hssr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_ICP, "%s: icp_hssr1_w: HSSP Status Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


/*

  Intel SA-1110 Serial Port 3 - UART

  pg. 289 to 306 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::uart3_irq_callback(int state)
{
	set_irq_line(INT_UART3, state);
}

// Rx completed receiving byte
void sa1110_periphs_device::rcv_complete()
{
	receive_register_extract();

	u16 data_and_flags = 0;
	if (is_receive_framing_error())
		data_and_flags |= 0x200;
	if (is_receive_parity_error())
		data_and_flags |= 0x100;
	data_and_flags |= get_received_char();

	uart_write_receive_fifo(data_and_flags);
}

// Tx completed sending byte
void sa1110_periphs_device::tra_complete()
{
	m_uart_regs.tx_fifo_count--;
	m_uart_regs.tx_fifo_read_idx = (m_uart_regs.tx_fifo_read_idx + 1) % std::size(m_uart_regs.tx_fifo);
	m_uart_regs.utsr1 |= (1 << UTSR1_TNF_BIT);

	if (m_uart_regs.tx_fifo_count)
		transmit_register_setup(m_uart_regs.tx_fifo[m_uart_regs.tx_fifo_read_idx]);
	else
		m_uart_regs.utsr1 &= ~(1 << UTSR1_TBY_BIT);

	uart_check_tx_fifo_service();
}

// Tx send bit
void sa1110_periphs_device::tra_callback()
{
	// TODO: Handle loopback mode
	m_uart3_tx_out(transmit_register_get_data_bit());
}

void sa1110_periphs_device::uart_recalculate_divisor()
{
	// TODO: Handle external UART clocking
	const int multiplier = (((m_uart_regs.utcr[1] & 0x0f) << 8) | (m_uart_regs.utcr[2] & 0xff)) + 1;
	set_rcv_rate(INTERNAL_OSC, multiplier * 16);
	set_tra_rate(INTERNAL_OSC, multiplier * 16);

	receive_register_reset();
	transmit_register_reset();
}

void sa1110_periphs_device::uart_update_eif_status()
{
	bool has_error = false;
	for (int i = 0; i < 4 && i < m_uart_regs.rx_fifo_count; i++)
	{
		const int read_idx = (m_uart_regs.rx_fifo_read_idx + i) % std::size(m_uart_regs.rx_fifo);
		if (m_uart_regs.rx_fifo[read_idx] & 0x700)
		{
			has_error = true;
			break;
		}
	}

	if (has_error)
	{
		m_uart_regs.utsr0 |= (1 << UTSR0_EIF_BIT);
		m_uart3_irqs->in_w<UART3_EIF>(1);
	}
	else
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_EIF_BIT);
		m_uart3_irqs->in_w<UART3_EIF>(0);
	}
}

void sa1110_periphs_device::uart_write_receive_fifo(u16 data_and_flags)
{
	if (m_uart_regs.rx_fifo_count >= std::size(m_uart_regs.rx_fifo))
		return;
	if (!BIT(m_uart_regs.utcr[3], UTCR3_RXE_BIT))
		return;

	// fill FIFO entry
	m_uart_regs.rx_fifo[m_uart_regs.rx_fifo_write_idx] = data_and_flags;
	m_uart_regs.rx_fifo_count++;
	m_uart_regs.rx_fifo_write_idx = (m_uart_regs.rx_fifo_write_idx + 1) % std::size(m_uart_regs.rx_fifo);

	// update error flags
	uart_update_eif_status();

	// update FIFO-service interrupt
	uart_check_rx_fifo_service();
}

u8 sa1110_periphs_device::uart_read_receive_fifo()
{
	const u8 data = m_uart_regs.rx_fifo[m_uart_regs.rx_fifo_read_idx];
	if (m_uart_regs.rx_fifo_count)
	{
		m_uart_regs.rx_fifo_read_idx = (m_uart_regs.rx_fifo_read_idx + 1) % std::size(m_uart_regs.rx_fifo);
		m_uart_regs.rx_fifo_count--;
		if (m_uart_regs.rx_fifo_count)
		{
			const u16 fifo_bottom_flags = ((m_uart_regs.rx_fifo[m_uart_regs.rx_fifo_read_idx]) >> 8) & 7;
			m_uart_regs.utsr1 &= ~((1 << UTSR1_PRE_BIT) | (1 << UTSR1_FRE_BIT) | (1 << UTSR1_ROR_BIT));
			m_uart_regs.utsr1 |= fifo_bottom_flags << UTSR1_PRE_BIT;
		}
		uart_update_eif_status();
	}
	uart_check_rx_fifo_service();
	return data;
}

void sa1110_periphs_device::uart_check_rx_fifo_service()
{
	if (m_uart_regs.rx_fifo_count != 0)
		m_uart_regs.utsr1 |= (1 << UTSR1_RNE_BIT);
	else
		m_uart_regs.utsr1 &= ~(1 << UTSR1_RNE_BIT);

	if (m_uart_regs.rx_fifo_count > 4)
	{
		m_uart_regs.utsr0 |= (1 << UTSR0_RFS_BIT);
		if (BIT(m_uart_regs.utcr[3], UTCR3_RIE_BIT))
		{
			m_uart3_irqs->in_w<UART3_RFS>(1);
		}
	}
	else
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_RFS_BIT);
		m_uart3_irqs->in_w<UART3_RFS>(0);
	}
}

void sa1110_periphs_device::uart_write_transmit_fifo(u8 data)
{
	if (m_uart_regs.tx_fifo_count >= std::size(m_uart_regs.tx_fifo))
		return;
	if (!BIT(m_uart_regs.utcr[3], UTCR3_TXE_BIT))
		return;

	// immediately start transmitting if FIFO is empty
	if (m_uart_regs.tx_fifo_count == 0)
	{
		m_uart_regs.utsr1 |= (1 << UTSR1_TBY_BIT);
		transmit_register_setup(data);
	}

	// fill FIFO entry
	m_uart_regs.tx_fifo[m_uart_regs.tx_fifo_write_idx] = data;
	m_uart_regs.tx_fifo_count++;
	m_uart_regs.tx_fifo_write_idx = (m_uart_regs.tx_fifo_write_idx + 1) % std::size(m_uart_regs.tx_fifo);

	// update FIFO-service interrupt
	uart_check_tx_fifo_service();
}

void sa1110_periphs_device::uart_check_tx_fifo_service()
{
	if (m_uart_regs.tx_fifo_count < std::size(m_uart_regs.tx_fifo))
		m_uart_regs.utsr1 |= (1 << UTSR1_TNF_BIT);
	else
		m_uart_regs.utsr1 &= ~(1 << UTSR1_TNF_BIT);

	if (m_uart_regs.tx_fifo_count <= 4)
	{
		m_uart_regs.utsr0 |= (1 << UTSR0_TFS_BIT);
		if (BIT(m_uart_regs.utcr[3], UTCR3_TIE_BIT))
		{
			m_uart3_irqs->in_w<UART3_TFS>(1);
		}
	}
	else
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_TFS_BIT);
		m_uart3_irqs->in_w<UART3_TFS>(0);
	}
}

void sa1110_periphs_device::uart_set_receiver_idle()
{
}

void sa1110_periphs_device::uart_begin_of_break()
{
}

void sa1110_periphs_device::uart_end_of_break()
{
}

void sa1110_periphs_device::uart_set_receiver_enabled(bool enabled)
{
	if (!enabled)
	{
		m_uart_regs.utsr0 &= ~(1 << UTSR0_RFS_BIT);
		m_uart3_irqs->in_w<UART3_RFS>(0);

		m_uart_regs.utsr1 &= ~(1 << UTSR1_RNE_BIT);

		m_uart_regs.rx_fifo_count = 0;
		m_uart_regs.rx_fifo_read_idx = 0;
		m_uart_regs.rx_fifo_write_idx = 0;

		receive_register_reset();
	}
}

void sa1110_periphs_device::uart_set_transmitter_enabled(bool enabled)
{
	if (enabled)
	{
		//m_uart_regs.utsr0 |= (1 << UTSR0_TFS_BIT);
		//m_uart3_irqs->in_w<UART3_TFS>(1);

		//m_uart_regs.utsr1 |= (1 << UTSR1_TNF_BIT);
	}
	else
	{
		//m_uart_regs.utsr0 &= ~(1 << UTSR0_TFS_BIT);
		//m_uart3_irqs->in_w<UART3_TFS>(0);

		//m_uart_regs.utsr1 &= ~(1 << UTSR1_TBY_BIT);
		//m_uart_regs.utsr1 &= ~(1 << UTSR1_TNF_BIT);

		m_uart_regs.tx_fifo_count = 0;
		m_uart_regs.tx_fifo_read_idx = 0;
		m_uart_regs.tx_fifo_write_idx = 0;

		transmit_register_reset();
	}

	uart_check_tx_fifo_service();
}

void sa1110_periphs_device::uart_set_receive_irq_enabled(bool enabled)
{
}

void sa1110_periphs_device::uart_set_transmit_irq_enabled(bool enabled)
{
}

u32 sa1110_periphs_device::uart3_utcr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_regs.utcr[0];
	LOGMASKED(LOG_UART3, "%s: uart3_utcr0_r: UART Control Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::uart3_utcr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UART3, "%s: uart3_utcr0_w: UART Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_UART3, "%s:                Parity Enable: %d\n", machine().describe_context(), BIT(data, 0));
	LOGMASKED(LOG_UART3, "%s:                Parity Mode: %s\n", machine().describe_context(), BIT(data, 1) ? "Even" : "Odd");
	LOGMASKED(LOG_UART3, "%s:                Stop Bits: %d\n", machine().describe_context(), BIT(data, 2) + 1);
	LOGMASKED(LOG_UART3, "%s:                Data Size: %d\n", machine().describe_context(), BIT(data, 3) ? 8 : 7);
	LOGMASKED(LOG_UART3, "%s:                Sample Clock: %s\n", machine().describe_context(), BIT(data, 4) ? "External" : "Internal");
	LOGMASKED(LOG_UART3, "%s:                Receive Edge: %s\n", machine().describe_context(), BIT(data, 5) ? "Falling" : "Rising");
	LOGMASKED(LOG_UART3, "%s:                Transmit Edge: %s\n", machine().describe_context(), BIT(data, 6) ? "Falling" : "Rising");

	stop_bits_t stop_bits = (BIT(data, 2) ? STOP_BITS_2 : STOP_BITS_1);

	parity_t parity = PARITY_NONE;
	if (BIT(data, 0))
	{
		parity = (BIT(data, 1) ? PARITY_EVEN : PARITY_ODD);
	}

	set_data_frame(1, BIT(data, 3) ? 8 : 7, parity, stop_bits);
	receive_register_reset();
	transmit_register_reset();

	COMBINE_DATA(&m_uart_regs.utcr[0]);
}

u32 sa1110_periphs_device::uart3_utcr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_regs.utcr[1];
	LOGMASKED(LOG_UART3, "%s: uart3_utcr1_r: UART Control Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::uart3_utcr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UART3, "%s: uart3_utcr1_w: UART Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_UART3, "%s:                Baud Rate Divisor MSB: %02x\n", machine().describe_context(), data & 0x0f);
	const u8 old = m_uart_regs.utcr[1] & 0x0f;
	COMBINE_DATA(&m_uart_regs.utcr[1]);
	if ((m_uart_regs.utcr[1] & 0x0f) != old)
		uart_recalculate_divisor();
}

u32 sa1110_periphs_device::uart3_utcr2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_regs.utcr[2];
	LOGMASKED(LOG_UART3, "%s: uart3_utcr2_r: UART Control Register 2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::uart3_utcr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UART3, "%s: uart3_utcr2_w: UART Control Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_UART3, "%s:                Baud Rate Divisor LSB: %02x\n", machine().describe_context(), (u8)data);
	const u8 old = m_uart_regs.utcr[2] & 0xff;
	COMBINE_DATA(&m_uart_regs.utcr[2]);
	if ((m_uart_regs.utcr[2] & 0xff) != old)
		uart_recalculate_divisor();
}

u32 sa1110_periphs_device::uart3_utcr3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_regs.utcr[3];
	LOGMASKED(LOG_UART3, "%s: uart3_utcr3_r: UART Control Register 3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::uart3_utcr3_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UART3, "%s: uart3_utcr3_w: UART Control Register 3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_UART3, "%s:                Receive Enable: %d\n", machine().describe_context(), BIT(data, 0));
	LOGMASKED(LOG_UART3, "%s:                Transmit Enable: %d\n", machine().describe_context(), BIT(data, 1));
	LOGMASKED(LOG_UART3, "%s:                Send Break: %d\n", machine().describe_context(), BIT(data, 2));
	LOGMASKED(LOG_UART3, "%s:                Receive FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 3));
	LOGMASKED(LOG_UART3, "%s:                Transmit FIFO IRQ Enable: %d\n", machine().describe_context(), BIT(data, 4));
	LOGMASKED(LOG_UART3, "%s:                Loopback Enable: %d\n", machine().describe_context(), BIT(data, 5));
	const u32 old = m_uart_regs.utcr[3];
	COMBINE_DATA(&m_uart_regs.utcr[3]);
	const u32 changed = old ^ m_uart_regs.utcr[3];
	if (BIT(changed, 0))
		uart_set_receiver_enabled(BIT(data, 0));
	if (BIT(changed, 1))
		uart_set_transmitter_enabled(BIT(data, 1));
	if (BIT(changed, 3))
		uart_set_receive_irq_enabled(BIT(data, 3));
	if (BIT(changed, 4))
		uart_set_transmit_irq_enabled(BIT(data, 4));
}

u32 sa1110_periphs_device::uart3_utdr_r(offs_t offset, u32 mem_mask)
{
	const u8 data = uart_read_receive_fifo();
	LOGMASKED(LOG_UART3, "%s: uart3_utdr_r: UART Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::uart3_utdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UART3, "%s: uart3_utdr_w: UART Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (data == 0x0d || data == 0x0a || (data >= 0x20 && data < 0x7f))
	{
		osd_printf_debug("%c", (char)data);
	}
	uart_write_transmit_fifo((u8)data);
}

u32 sa1110_periphs_device::uart3_utsr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_regs.utsr0;
	LOGMASKED(LOG_UART3, "%s: uart3_utsr0_r: UART Status Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::uart3_utsr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_UART3, "%s: uart3_utsr1_w: UART Status Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_UART3, "%s:                Receiver Idle Status: %d\n", machine().describe_context(), BIT(data, 2));
	LOGMASKED(LOG_UART3, "%s:                Receiver Begin of Break Status: %d\n", machine().describe_context(), BIT(data, 3));
	LOGMASKED(LOG_UART3, "%s:                Receiver End of Break Status: %d\n", machine().describe_context(), BIT(data, 4));
	if (BIT(data, 2))
		uart_set_receiver_idle();
	if (BIT(data, 3))
		uart_begin_of_break();
	if (BIT(data, 4))
		uart_end_of_break();
}

u32 sa1110_periphs_device::uart3_utsr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_uart_regs.utsr1;
	LOGMASKED(LOG_UART3_HF, "%s: uart3_utsr1_r: UART Status Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}


/*

  Intel SA-1110 MCP - Serial Port 4

  pg. 306 to 346 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::mcp_irq_callback(int state)
{
	set_irq_line(INT_MCP, state);
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::mcp_audio_tx_callback)
{
	if (!m_codec)
		return;

	const u16 sample = m_mcp_regs.audio_tx_fifo[m_mcp_regs.audio_tx_fifo_read_idx];
	m_codec->audio_sample_in(sample);

	if (m_mcp_regs.audio_tx_fifo_count)
	{
		m_mcp_regs.audio_tx_fifo_count--;
		m_mcp_regs.audio_tx_fifo_read_idx = (m_mcp_regs.audio_tx_fifo_read_idx + 1) % std::size(m_mcp_regs.audio_tx_fifo);

		m_mcp_regs.mcsr &= ~(1 << MCSR_ATU_BIT);
		m_mcp_irqs->in_w<MCP_AUDIO_UNDERRUN>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_ATU_BIT);
		m_mcp_irqs->in_w<MCP_AUDIO_UNDERRUN>(1);
	}

	m_mcp_regs.mcsr |= (1 << MCSR_ANF_BIT);
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::mcp_telecom_tx_callback)
{
	if (!m_codec)
		return;

	const u16 sample = m_mcp_regs.telecom_tx_fifo[m_mcp_regs.telecom_tx_fifo_read_idx];
	m_codec->telecom_sample_in(sample);

	if (m_mcp_regs.telecom_tx_fifo_count)
	{
		m_mcp_regs.telecom_tx_fifo_count--;
		m_mcp_regs.telecom_tx_fifo_read_idx = (m_mcp_regs.telecom_tx_fifo_read_idx + 1) % std::size(m_mcp_regs.telecom_tx_fifo);

		m_mcp_regs.mcsr &= ~(1 << MCSR_TTU_BIT);
		m_mcp_irqs->in_w<MCP_TELECOM_UNDERRUN>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_TTU_BIT);
		m_mcp_irqs->in_w<MCP_TELECOM_UNDERRUN>(1);
	}

	m_mcp_regs.mcsr |= (1 << MCSR_TNF_BIT);
}

u16 sa1110_periphs_device::mcp_read_audio_fifo()
{
	const u16 data = m_mcp_regs.audio_rx_fifo[m_mcp_regs.audio_rx_fifo_read_idx];
	if (m_mcp_regs.audio_rx_fifo_count)
	{
		m_mcp_regs.audio_rx_fifo_count--;
		m_mcp_regs.audio_rx_fifo_read_idx = (m_mcp_regs.audio_rx_fifo_read_idx + 1) % std::size(m_mcp_regs.audio_rx_fifo);

		const bool half_full = m_mcp_regs.audio_rx_fifo_count >= 4;
		m_mcp_regs.mcsr &= ~(1 << MCSR_ARS_BIT);
		if (half_full)
		{
			m_mcp_regs.mcsr |= (1 << MCSR_ARS_BIT);
		}
		bool fifo_interrupt = BIT(m_mcp_regs.mccr0, MCCR0_ARE_BIT) && half_full;
		m_mcp_irqs->in_w<MCP_AUDIO_RX>((int)fifo_interrupt);

		if (m_mcp_regs.audio_rx_fifo_count)
			m_mcp_regs.mcsr &= ~(1 << MCSR_ANE_BIT);
		else
			m_mcp_regs.mcsr |= (1 << MCSR_ANE_BIT);
	}
	return data;
}

u16 sa1110_periphs_device::mcp_read_telecom_fifo()
{
	const u16 data = m_mcp_regs.telecom_rx_fifo[m_mcp_regs.telecom_rx_fifo_read_idx];
	if (m_mcp_regs.telecom_rx_fifo_count)
	{
		m_mcp_regs.telecom_rx_fifo_count--;
		m_mcp_regs.telecom_rx_fifo_read_idx = (m_mcp_regs.telecom_rx_fifo_read_idx + 1) % std::size(m_mcp_regs.telecom_rx_fifo);

		const bool half_full = m_mcp_regs.telecom_rx_fifo_count >= 4;
		m_mcp_regs.mcsr &= ~(1 << MCSR_TRS_BIT);
		if (half_full)
		{
			m_mcp_regs.mcsr |= (1 << MCSR_TRS_BIT);
		}
		bool fifo_interrupt = BIT(m_mcp_regs.mccr0, MCCR0_TRE_BIT) && half_full;
		m_mcp_irqs->in_w<MCP_TELECOM_RX>((int)fifo_interrupt);

		if (m_mcp_regs.telecom_rx_fifo_count)
			m_mcp_regs.mcsr &= ~(1 << MCSR_TNE_BIT);
		else
			m_mcp_regs.mcsr |= (1 << MCSR_TNE_BIT);
	}
	return data;
}

attotime sa1110_periphs_device::mcp_get_audio_frame_rate()
{
	const u32 bit_rate = BIT(m_mcp_regs.mccr1, MCCR1_CFS_BIT) ? 9585000 : 11981000;
	const uint64_t ticks = 32 * ((m_mcp_regs.mccr0 & MCCR0_ASD_MASK) >> MCCR0_ASD_BIT);
	return attotime::from_ticks(ticks, bit_rate);
}

attotime sa1110_periphs_device::mcp_get_telecom_frame_rate()
{
	const u32 bit_rate = BIT(m_mcp_regs.mccr1, MCCR1_CFS_BIT) ? 9585000 : 11981000;
	const uint64_t ticks = 32 * ((m_mcp_regs.mccr0 & MCCR0_TSD_MASK) >> MCCR0_TSD_BIT);
	return attotime::from_ticks(ticks, bit_rate);
}

void sa1110_periphs_device::mcp_update_sample_rate()
{
	const attotime audio_rate = mcp_get_audio_frame_rate();
	m_mcp_regs.audio_tx_timer->adjust(audio_rate, 0, audio_rate);

	const attotime telecom_rate = mcp_get_telecom_frame_rate();
	m_mcp_regs.telecom_tx_timer->adjust(telecom_rate, 0, telecom_rate);
}

void sa1110_periphs_device::mcp_set_enabled(bool enabled)
{
	if (enabled)
	{
		mcp_update_sample_rate();
	}
	else
	{
		m_mcp_regs.audio_tx_timer->adjust(attotime::never);
		m_mcp_regs.telecom_tx_timer->adjust(attotime::never);
	}
}

void sa1110_periphs_device::mcp_audio_tx_fifo_push(const u16 value)
{
	if (m_mcp_regs.audio_rx_fifo_count == std::size(m_mcp_regs.audio_tx_fifo))
		return;

	m_mcp_regs.audio_tx_fifo[m_mcp_regs.audio_tx_fifo_write_idx] = value;
	m_mcp_regs.audio_rx_fifo_write_idx = (m_mcp_regs.audio_tx_fifo_write_idx + 1) % std::size(m_mcp_regs.audio_tx_fifo);
	m_mcp_regs.audio_rx_fifo_count++;

	if (m_mcp_regs.audio_tx_fifo_count == std::size(m_mcp_regs.audio_tx_fifo))
		m_mcp_regs.mcsr &= ~(1 << MCSR_ANF_BIT);

	if (m_mcp_regs.audio_tx_fifo_count >= 4)
	{
		m_mcp_regs.mcsr &= ~(1 << MCSR_ATS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_ATE_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_TX>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_ATS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_ATE_BIT))
			m_mcp_irqs->in_w<MCP_AUDIO_TX>(1);
	}
}

void sa1110_periphs_device::mcp_telecom_tx_fifo_push(const u16 value)
{
	if (m_mcp_regs.telecom_rx_fifo_count == std::size(m_mcp_regs.telecom_tx_fifo))
		return;

	m_mcp_regs.telecom_tx_fifo[m_mcp_regs.telecom_tx_fifo_write_idx] = value;
	m_mcp_regs.telecom_rx_fifo_write_idx = (m_mcp_regs.telecom_tx_fifo_write_idx + 1) % std::size(m_mcp_regs.telecom_tx_fifo);
	m_mcp_regs.telecom_rx_fifo_count++;

	if (m_mcp_regs.telecom_tx_fifo_count == std::size(m_mcp_regs.telecom_tx_fifo))
		m_mcp_regs.mcsr &= ~(1 << MCSR_TNF_BIT);

	if (m_mcp_regs.audio_tx_fifo_count >= 4)
	{
		m_mcp_regs.mcsr &= ~(1 << MCSR_TTS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_TTE_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_TX>(0);
	}
	else
	{
		m_mcp_regs.mcsr |= (1 << MCSR_TTS_BIT);
		if (BIT(m_mcp_regs.mccr0, MCCR0_TTE_BIT))
			m_mcp_irqs->in_w<MCP_TELECOM_TX>(1);
	}
}

void sa1110_periphs_device::mcp_codec_read(offs_t offset)
{
	if (!m_codec)
		return;

	const u16 data = m_codec->read(offset);
	m_mcp_regs.mcdr2 &= 0xffff0000;
	m_mcp_regs.mcdr2 |= data;

	m_mcp_regs.mcsr |= (1 << MCSR_CRC_BIT);
	m_mcp_regs.mcsr &= ~(1 << MCSR_CWC_BIT);
}

void sa1110_periphs_device::mcp_codec_write(offs_t offset, u16 data)
{
	if (!m_codec)
		return;

	m_codec->write(offset, data);
	m_mcp_regs.mcsr |= (1 << MCSR_CWC_BIT);
	m_mcp_regs.mcsr &= ~(1 << MCSR_CRC_BIT);
}

u32 sa1110_periphs_device::mcp_mccr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mcp_regs.mccr0;
	LOGMASKED(LOG_MCP, "%s: mcp_mccr0_r: MCP Control Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::mcp_mccr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_MCP, "%s: mcp_mccr0_w: MCP Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_MCP, "%s:              Audio Sample Rate Divisor: %02x\n", machine().describe_context(), data & MCCR0_ASD_MASK);
	LOGMASKED(LOG_MCP, "%s:              Telecom Sample Rate Divisor: %02x\n", machine().describe_context(), (data & MCCR0_TSD_MASK) >> MCCR0_TSD_BIT);
	LOGMASKED(LOG_MCP, "%s:              MCP Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_MCE_BIT));
	LOGMASKED(LOG_MCP, "%s:              Clock Select: %s\n", machine().describe_context(), BIT(data, MCCR0_ECS_BIT) ? "External" : "Internal");
	LOGMASKED(LOG_MCP, "%s:              A/D Data Sampling Mode: %s Valid\n", machine().describe_context(), BIT(data, MCCR0_ADM_BIT) ? "First" : "Each");
	LOGMASKED(LOG_MCP, "%s:              Telecom Tx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_TTE_BIT));
	LOGMASKED(LOG_MCP, "%s:              Telecom Rx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_TRE_BIT));
	LOGMASKED(LOG_MCP, "%s:              Audio Tx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_ATE_BIT));
	LOGMASKED(LOG_MCP, "%s:              Audio Rx FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_ARE_BIT));
	LOGMASKED(LOG_MCP, "%s:              Loopback Enable: %d\n", machine().describe_context(), BIT(data, MCCR0_LBM_BIT));
	LOGMASKED(LOG_MCP, "%s:              External Clock Prescaler: %d\n", machine().describe_context(), ((data & MCCR0_ECP_MASK) >> MCCR0_ECP_BIT) + 1);
	const u32 old = m_mcp_regs.mccr0;
	COMBINE_DATA(&m_mcp_regs.mccr0);
	const u32 changed = old ^ m_mcp_regs.mccr0;
	if (BIT(m_mcp_regs.mcsr, MCSR_ATS_BIT) && BIT(changed, MCCR0_ATE_BIT))
		m_mcp_irqs->in_w<MCP_AUDIO_TX>(BIT(m_mcp_regs.mcsr, MCSR_ATS_BIT));
	if (BIT(m_mcp_regs.mcsr, MCSR_ARS_BIT) && BIT(changed, MCCR0_ARE_BIT))
		m_mcp_irqs->in_w<MCP_AUDIO_RX>(BIT(m_mcp_regs.mcsr, MCSR_ARS_BIT));
	if (BIT(m_mcp_regs.mcsr, MCSR_TTS_BIT) && BIT(changed, MCCR0_TTE_BIT))
		m_mcp_irqs->in_w<MCP_TELECOM_TX>(BIT(m_mcp_regs.mcsr, MCSR_TTS_BIT));
	if (BIT(m_mcp_regs.mcsr, MCSR_TRS_BIT) && BIT(changed, MCCR0_TRE_BIT))
		m_mcp_irqs->in_w<MCP_TELECOM_RX>(BIT(m_mcp_regs.mcsr, MCSR_TRS_BIT));
	if (BIT(old, MCCR0_MCE_BIT) != BIT(m_mcp_regs.mccr0, MCCR0_MCE_BIT))
		mcp_set_enabled(BIT(m_mcp_regs.mccr0, MCCR0_MCE_BIT));
}

u32 sa1110_periphs_device::mcp_mcdr0_r(offs_t offset, u32 mem_mask)
{
	const u16 data = mcp_read_audio_fifo() << 4;
	LOGMASKED(LOG_MCP, "%s: mcp_mcdr0_r: MCP Data Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::mcp_mcdr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_MCP, "%s: mcp_mcdr0_w: MCP Data Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	mcp_audio_tx_fifo_push((u16)data);
}

u32 sa1110_periphs_device::mcp_mcdr1_r(offs_t offset, u32 mem_mask)
{
	const u16 data = mcp_read_telecom_fifo() << 4;
	LOGMASKED(LOG_MCP, "%s: mcp_mcdr1_r: MCP Data Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::mcp_mcdr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_MCP, "%s: mcp_mcdr1_w: MCP Data Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	mcp_telecom_tx_fifo_push((u16)data);
}

u32 sa1110_periphs_device::mcp_mcdr2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mcp_regs.mcdr2;
	LOGMASKED(LOG_MCP, "%s: mcp_mcdr2_r: MCP Data Register 2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_MCP, "%s:              Value: %04x\n", machine().describe_context(), (u16)m_mcp_regs.mcdr2);
	LOGMASKED(LOG_MCP, "%s:              Read/Write: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcdr2, 16));
	LOGMASKED(LOG_MCP, "%s:              Address: %01x\n", machine().describe_context(), (m_mcp_regs.mcdr2 >> 17) & 0xf);
	return data;
}

void sa1110_periphs_device::mcp_mcdr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	const offs_t addr = (data & MCDR2_ADDR_MASK) >> MCDR2_ADDR_BIT;
	LOGMASKED(LOG_MCP, "%s: mcp_mcdr2_w: MCP Data Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_mcp_regs.mcdr2);
	m_mcp_regs.mcdr2 &= ~(1 << MCDR2_RW_BIT);

	if (BIT(data, MCDR2_RW_BIT))
		mcp_codec_write(addr, (u16)data);
	else
		mcp_codec_read(addr);
}

u32 sa1110_periphs_device::mcp_mcsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_mcp_regs.mcsr;
	LOGMASKED(LOG_MCP, "%s: mcp_mcsr_r: MCP Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_MCP, "%s:             Audio Xmit FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 0));
	LOGMASKED(LOG_MCP, "%s:             Audio Recv FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 1));
	LOGMASKED(LOG_MCP, "%s:             Telecom Xmit FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 2));
	LOGMASKED(LOG_MCP, "%s:             Telecom Recv FIFO Service Request: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 3));
	LOGMASKED(LOG_MCP, "%s:             Audio Xmit FIFO Underrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 4));
	LOGMASKED(LOG_MCP, "%s:             Audio Recv FIFO Overrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 5));
	LOGMASKED(LOG_MCP, "%s:             Telcom Xmit FIFO Underrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 6));
	LOGMASKED(LOG_MCP, "%s:             Telcom Recv FIFO Overrun: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 7));
	LOGMASKED(LOG_MCP, "%s:             Audio Xmit FIFO Not Full: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 8));
	LOGMASKED(LOG_MCP, "%s:             Audio Recv FIFO Not Empty: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 9));
	LOGMASKED(LOG_MCP, "%s:             Telcom Xmit FIFO Not Full: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 10));
	LOGMASKED(LOG_MCP, "%s:             Telcom Recv FIFO Not Empty: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 11));
	LOGMASKED(LOG_MCP, "%s:             Codec Write Complete: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 12));
	LOGMASKED(LOG_MCP, "%s:             Codec Read Complete: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 13));
	LOGMASKED(LOG_MCP, "%s:             Audio Codec Enabled: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 14));
	LOGMASKED(LOG_MCP, "%s:             Telecom Codec Enabled: %d\n", machine().describe_context(), BIT(m_mcp_regs.mcsr, 15));
	return data;
}

void sa1110_periphs_device::mcp_mcsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_MCP, "%s: mcp_w: MCP Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_mcp_regs.mcsr;
	const u32 sticky_mask = (1 << MCSR_ATU_BIT) | (1 << MCSR_ARO_BIT) | (1 << MCSR_TTU_BIT) | (1 << MCSR_TRO_BIT);
	m_mcp_regs.mcsr &= ~(data & mem_mask & sticky_mask);
	if (BIT(old, MCSR_ATU_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_ATU_BIT))
		m_mcp_irqs->in_w<MCP_AUDIO_UNDERRUN>(0);
	if (BIT(old, MCSR_ARO_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_ARO_BIT))
		m_mcp_irqs->in_w<MCP_AUDIO_OVERRUN>(0);
	if (BIT(old, MCSR_TTU_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_TTU_BIT))
		m_mcp_irqs->in_w<MCP_TELECOM_UNDERRUN>(0);
	if (BIT(old, MCSR_TRO_BIT) && !BIT(m_mcp_regs.mcsr, MCSR_TRO_BIT))
		m_mcp_irqs->in_w<MCP_TELECOM_OVERRUN>(0);
}


/*

  Intel SA-1110 SSP - Synchronous Serial Port

  pg. 331 to 347 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::ssp_rx_callback)
{
	// TODO: Implement receiving data serially rather than in bulk.
}

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::ssp_tx_callback)
{
	// TODO: Implement transmitting data serially rather than in bulk.
	if (m_ssp_regs.tx_fifo_count)
	{
		const u16 data = m_ssp_regs.tx_fifo[m_ssp_regs.tx_fifo_read_idx];
		m_ssp_out(data);
		LOGMASKED(LOG_SSP, "SSP: Transmitting %04x, new Tx FIFO count %d\n", data, m_ssp_regs.tx_fifo_count);

		m_ssp_regs.tx_fifo_read_idx = (m_ssp_regs.tx_fifo_read_idx + 1) % std::size(m_ssp_regs.tx_fifo);
		m_ssp_regs.tx_fifo_count--;

		m_ssp_regs.sssr |= (1 << SSSR_TNF_BIT);

		if (m_ssp_regs.tx_fifo_count || m_ssp_regs.rx_fifo_count)
			m_ssp_regs.sssr |= (1 << SSSR_BSY_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_BSY_BIT);

		ssp_update_tx_level();
	}
}

void sa1110_periphs_device::ssp_update_enable_state()
{
	if (BIT(m_ssp_regs.sscr0, SSCR0_SSE_BIT))
	{
		if (m_ssp_regs.tx_fifo_count != std::size(m_ssp_regs.tx_fifo))
			m_ssp_regs.sssr |= (1 << SSSR_TNF_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_TNF_BIT);

		if (m_ssp_regs.rx_fifo_count != 0)
			m_ssp_regs.sssr |= (1 << SSSR_RNE_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_RNE_BIT);

		if (m_ssp_regs.tx_fifo_count != 0)
			m_ssp_regs.sssr |= (1 << SSSR_BSY_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_BSY_BIT);

		if (m_ssp_regs.tx_fifo_count <= 4)
			m_ssp_regs.sssr |= (1 << SSSR_TFS_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_TFS_BIT);

		if (m_ssp_regs.rx_fifo_count >= 4)
			m_ssp_regs.sssr |= (1 << SSSR_RFS_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_RFS_BIT);

		uint64_t bit_count = (m_ssp_regs.sscr0 & SSCR0_DSS_MASK) >> SSCR0_DSS_BIT;
		u32 clock_rate = 2 * (((m_ssp_regs.sscr0 & SSCR0_SCR_MASK) >> SSCR0_SCR_BIT) + 1);
		attotime packet_rate = attotime::from_ticks(bit_count * clock_rate, 3686400);
		m_ssp_regs.rx_timer->adjust(packet_rate, 0, packet_rate);
		m_ssp_regs.tx_timer->adjust(packet_rate, 0, packet_rate);
	}
	else
	{
		m_ssp_regs.sssr &= ~(1 << SSSR_TFS_BIT);
		m_ssp_regs.sssr &= ~(1 << SSSR_RFS_BIT);

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

void sa1110_periphs_device::ssp_update_rx_level()
{
	if (m_ssp_regs.rx_fifo_count >= 4)
		m_ssp_regs.sssr |= (1 << SSSR_RFS_BIT);
	else
		m_ssp_regs.sssr &= ~(1 << SSSR_RFS_BIT);
}

void sa1110_periphs_device::ssp_rx_fifo_push(const u16 data)
{
	if (m_ssp_regs.rx_fifo_count < std::size(m_ssp_regs.rx_fifo))
	{
		m_ssp_regs.rx_fifo[m_ssp_regs.rx_fifo_write_idx] = data;
		m_ssp_regs.rx_fifo_write_idx = (m_ssp_regs.rx_fifo_write_idx + 1) % std::size(m_ssp_regs.rx_fifo);
		m_ssp_regs.rx_fifo_count++;

		m_ssp_regs.sssr |= (1 << SSSR_RNE_BIT);

		ssp_update_rx_level();
	}
}

void sa1110_periphs_device::ssp_update_tx_level()
{
	if (m_ssp_regs.tx_fifo_count <= 4)
		m_ssp_regs.sssr |= (1 << SSSR_TFS_BIT);
	else
		m_ssp_regs.sssr &= ~(1 << SSSR_TFS_BIT);
}

void sa1110_periphs_device::ssp_tx_fifo_push(const u16 data)
{
	if (m_ssp_regs.tx_fifo_count < std::size(m_ssp_regs.tx_fifo))
	{
		m_ssp_regs.tx_fifo[m_ssp_regs.tx_fifo_write_idx] = data;
		m_ssp_regs.tx_fifo_write_idx = (m_ssp_regs.tx_fifo_write_idx + 1) % std::size(m_ssp_regs.tx_fifo);
		m_ssp_regs.tx_fifo_count++;

		if (m_ssp_regs.tx_fifo_count != std::size(m_ssp_regs.tx_fifo))
			m_ssp_regs.sssr |= (1 << SSSR_TNF_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_TNF_BIT);

		ssp_update_tx_level();
	}

	if (m_ssp_regs.tx_fifo_count || m_ssp_regs.rx_fifo_count)
		m_ssp_regs.sssr |= (1 << SSSR_BSY_BIT);
	else
		m_ssp_regs.sssr &= ~(1 << SSSR_BSY_BIT);
}

u16 sa1110_periphs_device::ssp_rx_fifo_pop()
{
	u16 data = m_ssp_regs.rx_fifo[m_ssp_regs.rx_fifo_read_idx];
	if (m_ssp_regs.rx_fifo_count)
	{
		m_ssp_regs.rx_fifo_read_idx = (m_ssp_regs.rx_fifo_read_idx + 1) % std::size(m_ssp_regs.rx_fifo);
		m_ssp_regs.rx_fifo_count--;

		if (m_ssp_regs.rx_fifo_count == 0)
			m_ssp_regs.sssr &= ~(1 << SSSR_RNE_BIT);

		if (m_ssp_regs.tx_fifo_count || m_ssp_regs.rx_fifo_count)
			m_ssp_regs.sssr |= (1 << SSSR_BSY_BIT);
		else
			m_ssp_regs.sssr &= ~(1 << SSSR_BSY_BIT);

		ssp_update_rx_level();
	}
	return data;
}

u32 sa1110_periphs_device::ssp_sscr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ssp_regs.sscr0;
	LOGMASKED(LOG_SSP, "%s: ssp_sscr0_r: SSP Control Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::ssp_sscr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	static const char *const s_dss_sizes[16] =
	{
		"Invalid [1]", "Invalid [2]", "Invalid [3]", "4-bit",
		"5-bit", "6-bit", "7-bit", "8-bit",
		"9-bit", "10-bit", "11-bit", "12-bit",
		"13-bit", "14-bit", "15-bit", "16-bit"
	};
	static const char *const s_frf_formats[4] = { "Motorola SPI", "TI Synchronous Serial", "National Microwire", "Reserved" };
	LOGMASKED(LOG_SSP, "%s: ssp_sscr0_w: SSP Control Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP, "%s:              Data Size Select: %s\n", machine().describe_context(), s_dss_sizes[(data & SSCR0_DSS_MASK) >> SSCR0_DSS_BIT]);
	LOGMASKED(LOG_SSP, "%s:              Frame Format: %s\n", machine().describe_context(), s_frf_formats[(data & SSCR0_FRF_MASK) >> SSCR0_FRF_BIT]);
	LOGMASKED(LOG_SSP, "%s:              SSP Enable: %d\n", machine().describe_context(), BIT(data, SSCR0_SSE_BIT));
	LOGMASKED(LOG_SSP, "%s:              Serial Clock Rate Divisor: %d\n", machine().describe_context(), 2 * (data & SSCR0_DSS_MASK) >> SSCR0_DSS_BIT);
	const u32 old = m_ssp_regs.sscr0;
	COMBINE_DATA(&m_ssp_regs.sscr0);
	if (BIT(old ^ m_ssp_regs.sscr0, SSCR0_SSE_BIT))
		ssp_update_enable_state();
}

u32 sa1110_periphs_device::ssp_sscr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ssp_regs.sscr1;
	LOGMASKED(LOG_SSP, "%s: ssp_sscr1_r: SSP Control Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::ssp_sscr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: ssp_sscr1_w: SSP Control Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP, "%s:              Receive FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, SSCR1_RIE_BIT));
	LOGMASKED(LOG_SSP, "%s:              Transmit FIFO Interrupt Enable: %d\n", machine().describe_context(), BIT(data, SSCR1_TIE_BIT));
	LOGMASKED(LOG_SSP, "%s:              Loopback Mode Enable: %d\n", machine().describe_context(), BIT(data, SSCR1_LBM_BIT));
	LOGMASKED(LOG_SSP, "%s:              Serial Clock Polarity: %d\n", machine().describe_context(), BIT(data, SSCR1_SPO_BIT));
	LOGMASKED(LOG_SSP, "%s:              Serial Clock Phase: %d\n", machine().describe_context(), BIT(data, SSCR1_SPH_BIT));
	LOGMASKED(LOG_SSP, "%s:              External Clock Select: %d\n", machine().describe_context(), BIT(data, SSCR1_ECS_BIT));
	COMBINE_DATA(&m_ssp_regs.sscr1);
}

u32 sa1110_periphs_device::ssp_ssdr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = ssp_rx_fifo_pop();
	LOGMASKED(LOG_SSP, "%s: ssp_ssdr_r: SSP Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::ssp_ssdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: ssp_ssdr_w: SSP Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	ssp_tx_fifo_push((u16)data);
}

u32 sa1110_periphs_device::ssp_sssr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ssp_regs.sssr;
	LOGMASKED(LOG_SSP, "%s: ssp_r: SSP Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP, "%s:        Transmit FIFO Not Full: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_TNF_BIT));
	LOGMASKED(LOG_SSP, "%s:        Receive FIFO Not Empty: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_RNE_BIT));
	LOGMASKED(LOG_SSP, "%s:        SSP Busy: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_BSY_BIT));
	LOGMASKED(LOG_SSP, "%s:        Transmit FIFO Service Request: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_TFS_BIT));
	LOGMASKED(LOG_SSP, "%s:        Receive FIFO Service Request: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_RFS_BIT));
	LOGMASKED(LOG_SSP, "%s:        Receive Overrun: %d\n", machine().describe_context(), BIT(m_ssp_regs.sssr, SSSR_ROR_BIT));
	return data;
}

void sa1110_periphs_device::ssp_sssr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_SSP, "%s: ssp_sssr_w: SSP Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	LOGMASKED(LOG_SSP, "%s:             Clear Receive Overrun: %d\n", machine().describe_context(), BIT(data, SSSR_ROR_BIT));
}


/*

  Intel SA-1110 Operating System Timer

  pg. 92 to 96 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::ostimer_tick_cb)
{
	const int channel = param;
	if (BIT(m_ostmr_regs.oier, channel))
	{
		m_ostmr_regs.ossr |= (1 << channel);
		set_irq_line(INT_OSTIMER0 + channel, 1);
		// TODO: Handle Channel 3, watchdog timer mode
	}
}

void sa1110_periphs_device::ostimer_update_count()
{
	const attotime time_delta = machine().time() - m_ostmr_regs.last_count_sync;
	const uint64_t ticks_elapsed = time_delta.as_ticks(INTERNAL_OSC);
	if (ticks_elapsed == 0ULL) // Accrue time until we can tick at least once
		return;

	const u32 wrapped_ticks = (u32)ticks_elapsed;
	m_ostmr_regs.oscr += wrapped_ticks;
	m_ostmr_regs.last_count_sync = machine().time();
}

void sa1110_periphs_device::ostimer_update_match_timer(int channel)
{
	ostimer_update_count();
	uint64_t ticks_remaining = m_ostmr_regs.osmr[channel] - m_ostmr_regs.oscr;
	if (m_ostmr_regs.oscr >= m_ostmr_regs.osmr[channel])
		ticks_remaining += 0x100000000ULL;
	m_ostmr_regs.timer[channel]->adjust(attotime::from_ticks(ticks_remaining, INTERNAL_OSC), channel);
	if (channel != 0)
		LOGMASKED(LOG_OSTIMER, "ostimer_update_match_timer: %d ticks at %dHz (%4.fms)\n", ticks_remaining, INTERNAL_OSC, attotime::from_ticks(ticks_remaining, INTERNAL_OSC).as_double() * 1000.0);
}

u32 sa1110_periphs_device::tmr_osmr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostmr_regs.osmr[0];
	LOGMASKED(LOG_OSTIMER_HF, "%s: tmr_osmr0_r: OS Timer Match Register 0: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_osmr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER_HF, "%s: tmr_osmr0_w: OS Timer Match Register 0 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ostmr_regs.osmr[0]);
	ostimer_update_match_timer(0);
}

u32 sa1110_periphs_device::tmr_osmr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostmr_regs.osmr[1];
	LOGMASKED(LOG_OSTIMER, "%s: tmr_osmr1_r: OS Timer Match Register 1: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_osmr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_osmr1_w: OS Timer Match Register 1 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ostmr_regs.osmr[1]);
	ostimer_update_match_timer(1);
}

u32 sa1110_periphs_device::tmr_osmr2_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostmr_regs.osmr[2];
	LOGMASKED(LOG_OSTIMER, "%s: tmr_osmr2_r: OS Timer Match Register 2: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_osmr2_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_osmr2_w: OS Timer Match Register 2 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ostmr_regs.osmr[2]);
	ostimer_update_match_timer(2);
}

u32 sa1110_periphs_device::tmr_osmr3_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostmr_regs.osmr[3];
	LOGMASKED(LOG_OSTIMER, "%s: tmr_osmr3_r: OS Timer Match Register 3: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_osmr3_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_osmr3_w: OS Timer Match Register 3 = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ostmr_regs.osmr[3]);
	ostimer_update_match_timer(3);
}

u32 sa1110_periphs_device::tmr_oscr_r(offs_t offset, u32 mem_mask)
{
	ostimer_update_count();

	const u32 data = m_ostmr_regs.oscr;
	LOGMASKED(LOG_OSTIMER_HF, "%s: tmr_oscr_r: OS Timer Counter Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_oscr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_oscr_w: OS Timer Counter Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ostmr_regs.oscr);
	m_ostmr_regs.last_count_sync = machine().time();
	for (int channel = 0; channel < 4; channel++)
	{
		if (m_ostmr_regs.oscr == m_ostmr_regs.osmr[channel] && BIT(m_ostmr_regs.oier, channel))
		{
			if (!BIT(m_ostmr_regs.ossr, channel))
			{
				m_ostmr_regs.ossr |= (1 << channel);
				set_irq_line(INT_OSTIMER0 + channel, 1);
			}
		}
		else
		{
			ostimer_update_match_timer(channel);
		}
	}
}

u32 sa1110_periphs_device::tmr_ossr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostmr_regs.ossr;
	LOGMASKED(LOG_OSTIMER, "%s: tmr_ossr_r: OS Timer Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_ossr_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (data != 1)
		LOGMASKED(LOG_OSTIMER, "%s: tmr_ossr_w: OS Timer Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_ostmr_regs.ossr;
	m_ostmr_regs.ossr &= ~(data & mem_mask);
	const u32 cleared = old & ~ m_ostmr_regs.ossr;
	for (int channel = 0; channel < 4; channel++)
	{
		if (BIT(cleared, channel))
			set_irq_line(INT_OSTIMER0 + channel, 0);
	}
}

u32 sa1110_periphs_device::tmr_ower_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostmr_regs.ower;
	LOGMASKED(LOG_OSTIMER, "%s: tmr_ower_r: OS Timer Watchdog Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_ower_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_ower_w: OS Timer Watchdog Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (!m_ostmr_regs.ower)
	{
		m_ostmr_regs.ower = data & mem_mask & 1;
	}
}

u32 sa1110_periphs_device::tmr_oier_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostmr_regs.oier;
	LOGMASKED(LOG_OSTIMER, "%s: tmr_oier_r: OS Timer Interrupt Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::tmr_oier_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_oier_w: OS Timer Interrupt Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ostmr_regs.oier);
}


/*

  Intel SA-1110 Real-Time Clock

  pg. 88 to 92 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

TIMER_CALLBACK_MEMBER(sa1110_periphs_device::rtc_tick_cb)
{
	m_rtc_regs.rcnr++;
	m_rtc_regs.rtsr |= (1 << RTSR_HZ_BIT);

	if (m_rtc_regs.rcnr == m_rtc_regs.rtar)
	{
		m_rtc_regs.rtsr |= (1 << RTSR_AL_BIT);
		if (BIT(m_rtc_regs.rtsr, RTSR_ALE_BIT))
			set_irq_line(INT_RTC_ALARM, 1);
	}

	if (BIT(m_rtc_regs.rtsr, RTSR_HZE_BIT))
		set_irq_line(INT_RTC_TICK, 1);
}

u32 sa1110_periphs_device::rtc_rtar_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rtar;
	LOGMASKED(LOG_RTC, "%s: rtc_rtar_r: RTC Alarm Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::rtc_rtar_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_rtar_w: RTC Alarm Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_rtc_regs.rtar);
}

u32 sa1110_periphs_device::rtc_rcnr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rcnr;
	LOGMASKED(LOG_RTC_HF, "%s: rtc_rcnr_r: RTC Count Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::rtc_rcnr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_rcnr_w: RTC Count Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_rtc_regs.rcnr);
}

u32 sa1110_periphs_device::rtc_rttr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rttr;
	LOGMASKED(LOG_RTC, "%s: rtc_rttr_r: RTC Timer Trim Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::rtc_rttr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_rttr_w: RTC Timer Trim Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_rtc_regs.rttr);
	// TODO: Implement timer trimming
}

u32 sa1110_periphs_device::rtc_rtsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rtsr;
	LOGMASKED(LOG_RTC, "%s: rtc_rtsr_r: RTC Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::rtc_rtsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_w: RTC Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);

	const u32 old = m_rtc_regs.rtsr;
	const bool old_alarm_int = BIT(old, RTSR_AL_BIT) && BIT(m_rtc_regs.rtsr, RTSR_ALE_BIT);
	const bool old_tick_int = BIT(old, RTSR_HZ_BIT) && BIT(m_rtc_regs.rtsr, RTSR_HZE_BIT);

	m_rtc_regs.rtsr &= ~(data & (RTSR_AL_MASK | RTSR_HZ_MASK) & mem_mask);
	m_rtc_regs.rtsr &= ~(RTSR_ALE_MASK | RTSR_HZE_MASK);
	m_rtc_regs.rtsr |= (data & (RTSR_ALE_MASK | RTSR_HZE_MASK) & mem_mask);

	const bool new_alarm_int = BIT(m_rtc_regs.rtsr, RTSR_AL_BIT) && BIT(m_rtc_regs.rtsr, RTSR_ALE_BIT);
	const bool new_tick_int = BIT(m_rtc_regs.rtsr, RTSR_HZ_BIT) && BIT(m_rtc_regs.rtsr, RTSR_HZE_BIT);

	if (old_alarm_int != new_alarm_int)
		set_irq_line(INT_RTC_ALARM, (int)new_alarm_int);
	if (old_tick_int != new_tick_int)
		set_irq_line(INT_RTC_TICK, (int)new_tick_int);
}


/*

  Intel SA-1110 Power Controller

  pg. 104 to 111 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

u32 sa1110_periphs_device::pwr_pmcr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pmcr;
	LOGMASKED(LOG_POWER, "%s: pwr_pmcr_r: Power Manager Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_pmcr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pmcr_w: Power Manager Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pmcr);
}

u32 sa1110_periphs_device::pwr_pssr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pssr;
	LOGMASKED(LOG_POWER, "%s: pwr_pssr_r: Power Manager Sleep Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_pssr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pssr_w: Power Manager Sleep Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_power_regs.pssr &= ~(data & 0x0000001f);
}

u32 sa1110_periphs_device::pwr_pspr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pspr;
	LOGMASKED(LOG_POWER_HF, "%s: pwr_pspr_r: Power Manager Scratch Pad Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_pspr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER_HF, "%s: pwr_pspr_w: Power Manager Scratch Pad Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pspr);
}

u32 sa1110_periphs_device::pwr_pwer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pwer;
	LOGMASKED(LOG_POWER, "%s: pwr_pwer_r: Power Manager Wake-up Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_pwer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pwer_w: Power Manager Wake-Up Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pwer);
}

u32 sa1110_periphs_device::pwr_pcfr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pcfr;
	LOGMASKED(LOG_POWER, "%s: pwr_pcfr_r: Power Manager General Configuration Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_pcfr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pcfr_w: Power Manager General Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pcfr);
}

u32 sa1110_periphs_device::pwr_ppcr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.ppcr;
	LOGMASKED(LOG_POWER, "%s: pwr_ppcr_r: Power Manager PLL Configuration Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_ppcr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_ppcr_w: Power Manager PLL Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.ppcr);
}

u32 sa1110_periphs_device::pwr_pgsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pgsr;
	LOGMASKED(LOG_POWER, "%s: pwr_pgsr_r: Power Manager GPIO Sleep State Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_pgsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pgsr_w: Power Manager GPIO Sleep State Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pgsr);
}

u32 sa1110_periphs_device::pwr_posr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.posr;
	LOGMASKED(LOG_POWER, "%s: pwr_posr_r: Power Manager Oscillator Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::pwr_posr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: power_w: Power Manager Oscillator Status Register (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}


/*

  Intel SA-1110 Reset Controller

  pg. 112 to 114 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

u32 sa1110_periphs_device::rst_rsrr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0;
	LOGMASKED(LOG_RESET, "%s: rst_rsrr_r: Reset Controller Software Reset Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::rst_rsrr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RESET, "%s: rst_rsrr_w: Reset Controller Software Reset Register (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::rst_rcsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rcsr;
	LOGMASKED(LOG_RESET, "%s: rst_rcsr_r: Reset Controller Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::rst_rcsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RESET, "%s: rst_rcsr_w: Reset Controller Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_rcsr &= ~(data & mem_mask);
}


/*

  Intel SA-1110 GPIO Controller

  pg. 71 to 80 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::gpio_in(const u32 line, const int state)
{
	const u32 mask = (1 << line);
	const u32 old_latch = m_gpio_regs.input_latch;
	m_gpio_regs.input_latch &= ~mask;
	m_gpio_regs.input_latch |= (state << line);

	LOGMASKED(LOG_GPIO, "gpio_in: Line %d, state %d\n", line, state);
	if (old_latch != m_gpio_regs.input_latch && !BIT(m_gpio_regs.gafr, line))
	{
		// TODO: The manual is unclear if edge detection functions on both inputs and outputs.
		//       If it can also function on outputs, remove the GPDR check below.
		if (!BIT(m_gpio_regs.gpdr, line) && BIT(m_gpio_regs.any_edge_mask, line))
		{
			const u32 old_edge = m_gpio_regs.gedr;
			if (state && BIT(m_gpio_regs.grer, line))
				m_gpio_regs.gedr |= mask;
			if (!state && BIT(m_gpio_regs.gfer, line))
				m_gpio_regs.gedr |= mask;
			if (old_edge != m_gpio_regs.gedr)
				gpio_update_interrupts(mask);
		}

		m_gpio_regs.gplr = (m_gpio_regs.input_latch & ~m_gpio_regs.gafr) | (m_gpio_regs.alt_input_latch & m_gpio_regs.gafr);
		LOGMASKED(LOG_GPIO, "gpio_in: New GPLR: %08x\n", m_gpio_regs.gplr);
	}
}

void sa1110_periphs_device::gpio_update_interrupts(const u32 changed_mask)
{
	u32 remaining_mask = changed_mask;
	for (u32 line = 0; line < 11; line++)
	{
		if (!BIT(remaining_mask, line))
			continue;

		set_irq_line(INT_GPIO0 + line, BIT(m_gpio_regs.gedr, line));
		remaining_mask &= ~(1 << line);
	}

	if (!remaining_mask)
		return;

	set_irq_line(INT_GPIOHI, (m_gpio_regs.gedr & 0x0ffff800) ? 1 : 0);
}

void sa1110_periphs_device::gpio_update_direction(const u32 old_gpdr)
{
	const u32 new_outputs = ~old_gpdr & m_gpio_regs.gpdr & ~m_gpio_regs.gafr;
	if (new_outputs)
	{
		for (u32 line = 0; line < 28; line++)
		{
			if (BIT(new_outputs, line))
			{
				m_gpio_out[line](BIT(m_gpio_regs.gplr, line));
			}
		}
	}

	// TODO: Do we need to check rising/falling edges based on the transition from output to input?
}

void sa1110_periphs_device::gpio_update_outputs(const u32 old_latch, const u32 changed)
{
	u32 remaining_changed = changed;

	for (u32 line = 0; line < 28 && remaining_changed != 0; line++)
	{
		if (BIT(remaining_changed, line))
		{
			m_gpio_out[line](BIT(m_gpio_regs.output_latch, line));
			remaining_changed &= ~(1 << line);
		}
	}
}

void sa1110_periphs_device::gpio_update_alternate_pins(const u32 changed_mask)
{
	// TODO
}

u32 sa1110_periphs_device::gpio_gplr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gplr;
	LOGMASKED(LOG_GPIO, "%s: gpio_gplr_r: GPIO Pin-Level Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_gplr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gplr_w: GPIO Pin-Level Register (ignored) = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::gpio_gpdr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gpdr;
	LOGMASKED(LOG_GPIO, "%s: gpio_gpdr_r: GPIO Direction Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_gpdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gpdr_w: GPIO Pin Direction Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_gpio_regs.gpdr;
	COMBINE_DATA(&m_gpio_regs.gpdr);
	if (old != m_gpio_regs.gpdr)
		gpio_update_direction(old);
 }

u32 sa1110_periphs_device::gpio_gpsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0;
	LOGMASKED(LOG_GPIO, "%s: gpio_gpsr_r: GPIO Output Set Register (ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_gpsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gpsr_w: GPIO Pin Output Set Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_gpio_regs.output_latch;
	m_gpio_regs.output_latch |= (data & mem_mask);
	const u32 changed = ((old ^ m_gpio_regs.output_latch) & m_gpio_regs.gpdr) & ~m_gpio_regs.gafr;
	if (changed)
		gpio_update_outputs(old, changed);
}

u32 sa1110_periphs_device::gpio_gpcr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0;
	LOGMASKED(LOG_GPIO, "%s: gpio_gpcr_r: GPIO Output Clear Register (ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_gpcr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gpcr_w: GPIO Pin Output Clear Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_gpio_regs.output_latch;
	m_gpio_regs.output_latch &= ~(data & mem_mask);
	const u32 changed = ((old ^ m_gpio_regs.output_latch) & m_gpio_regs.gpdr) & ~m_gpio_regs.gafr;
	if (changed)
		gpio_update_outputs(old, changed);
}

u32 sa1110_periphs_device::gpio_grer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.grer;
	LOGMASKED(LOG_GPIO, "%s: gpio_grer_r: GPIO Rising-Edge Detect Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_grer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_grer_w: GPIO Rising-Edge Detect Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_gpio_regs.grer);
	m_gpio_regs.any_edge_mask = m_gpio_regs.grer | m_gpio_regs.gfer;
}

u32 sa1110_periphs_device::gpio_gfer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gfer;
	LOGMASKED(LOG_GPIO, "%s: gpio_gfer_r: GPIO Falling-Edge Detect Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_gfer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gfer_w: GPIO Falling-Edge Detect Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_gpio_regs.gfer);
	m_gpio_regs.any_edge_mask = m_gpio_regs.grer | m_gpio_regs.gfer;
}

u32 sa1110_periphs_device::gpio_gedr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gedr;
	if (data != 0)
		LOGMASKED(LOG_GPIO, "%s: gpio_gedr_r: GPIO Edge Detect Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_gedr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gedr_w: GPIO Edge Detect Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_gpio_regs.gedr;
	m_gpio_regs.gedr &= ~(data & mem_mask);
	if (old != m_gpio_regs.gedr)
		gpio_update_interrupts(old ^ m_gpio_regs.gedr);
}

u32 sa1110_periphs_device::gpio_gafr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gafr;
	LOGMASKED(LOG_GPIO, "%s: gpio_gafr_r: GPIO Alternate Function Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::gpio_gafr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gafr_w: GPIO Alternate Function Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_gpio_regs.gafr;
	COMBINE_DATA(&m_gpio_regs.gafr);
	if (old != m_gpio_regs.gafr)
		gpio_update_alternate_pins(old ^ m_gpio_regs.gafr);
}


/*

  Intel SA-1110 Interrupt Controller

  pg. 81 to 88 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::set_irq_line(u32 line, int irq_state)
{
	if (irq_state == 1 && line != INT_OSTIMER0)
	{
		static const char *const INT_NAMES[32] =
		{
			"GPIO0", "GPIO1", "GPIO2", "GPIO3", "GPIO4", "GPIO5", "GPIO6", "GPIO7", "GPIO8", "GPIO9", "GPIO10", "GPIOHI",
			"LCD", "UDC", "reserved", "UART1", "UART2", "UART3", "MCP", "SSP", "DMA0", "DMA1", "DMA2", "DMA3", "DMA4", "DMA5",
			"OSTIMER0", "OSTIMER1", "OSTIMER2", "OSTIMER3", "RTC_TICK", "RTC_ALARM"
		};
		LOGMASKED(LOG_INTC, "Setting interrupt line %s\n", INT_NAMES[line]);
	}
	const u32 line_mask = (1 << line);
	const u32 old_status = m_intc_regs.icpr;
	m_intc_regs.icpr &= ~line_mask;
	m_intc_regs.icpr |= irq_state ? line_mask : 0;

	if (m_intc_regs.icpr == old_status)
		return;

	update_interrupts();
}

void sa1110_periphs_device::update_interrupts()
{
	const u32 old_fiq = m_intc_regs.icfp;
	m_intc_regs.icfp = (m_intc_regs.icpr & m_intc_regs.icmr) & m_intc_regs.iclr;
	if (old_fiq != m_intc_regs.icfp)
	{
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_intc_regs.icfp ? ASSERT_LINE : CLEAR_LINE);
	}

	const u32 old_irq = m_intc_regs.icip;
	m_intc_regs.icip = (m_intc_regs.icpr & m_intc_regs.icmr) & (~m_intc_regs.iclr);
	if (old_irq != m_intc_regs.icip)
	{
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_intc_regs.icip ? ASSERT_LINE : CLEAR_LINE);
	}
}

u32 sa1110_periphs_device::intc_icip_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icip;
	if (data && data != 0x04000000)
	{
		LOGMASKED(LOG_INTC, "%s: intc_icip_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	}
	return data;
}

void sa1110_periphs_device::intc_icip_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icip_w: (Invalid Write) Interrupt Controller IRQ Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::intc_icmr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icmr;
	LOGMASKED(LOG_INTC, "%s: intc_icmr_r: Interrupt Controller Mask Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::intc_icmr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icmr_w: Interrupt Controller Mask Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_intc_regs.icmr;
	COMBINE_DATA(&m_intc_regs.icmr);
	if (old != m_intc_regs.icmr)
		update_interrupts();
}

u32 sa1110_periphs_device::intc_iclr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.iclr;
	LOGMASKED(LOG_INTC, "%s: intc_iclr_r: Interrupt Controller Level Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::intc_iclr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_iclr_w: Interrupt Controller Level Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_intc_regs.iclr;
	COMBINE_DATA(&m_intc_regs.iclr);
	if (old != m_intc_regs.iclr)
		update_interrupts();
}

u32 sa1110_periphs_device::intc_icfp_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icfp;
	LOGMASKED(LOG_INTC, "%s: intc_icfp_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::intc_icfp_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icfp_w: (Invalid Write) Interrupt Controller FIQ Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::intc_icpr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icpr;
	LOGMASKED(LOG_INTC, "%s: intc_icpr_r: Interrupt Controller Pending Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::intc_icpr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icpr_w: (Invalid Write) Interrupt Controller Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 sa1110_periphs_device::intc_iccr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.iccr;
	LOGMASKED(LOG_INTC, "%s: intc_iccr_r: Interrupt Controller Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::intc_iccr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_iccr_w: Interrupt Controller Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_intc_regs.iccr = BIT(data, 0);
}


/*

  Intel SA-1110 Peripheral Pin Controller

  pg. 347 to 357 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

u32 sa1110_periphs_device::ppc_ppdr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ppc_regs.ppdr;
	LOGMASKED(LOG_PPC, "%s: ppc_ppdr_r: PPC Pin Direction Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::ppc_ppdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_PPC, "%s: ppc_ppdr_w: PPC Pin Direction Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ppc_regs.ppdr);
	//const u32 old_ppsr = m_ppc_regs.ppsr;
	m_ppc_regs.ppsr = (m_ppc_regs.ppsr_out & m_ppc_regs.ppdr) | (m_ppc_regs.ppsr_in & ~m_ppc_regs.ppdr);
	//const u32 changed_states = old_ppsr ^ m_ppc_regs.ppsr;
	//if (changed_states)
	//{
	//}
}

u32 sa1110_periphs_device::ppc_ppsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ppc_regs.ppsr;
	LOGMASKED(LOG_PPC, "%s: ppc_ppsr_r: PPC Pin State Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	//machine().debug_break();
	return data;
}

void sa1110_periphs_device::ppc_ppsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_PPC, "%s: ppc_ppsr_w: PPC Pin State Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	//const u32 old_latch = m_ppc_regs.ppsr_out;
	COMBINE_DATA(&m_ppc_regs.ppsr_out);
	m_ppc_regs.ppsr = (m_ppc_regs.ppsr_out & m_ppc_regs.ppdr) | (m_ppc_regs.ppsr_in & ~m_ppc_regs.ppdr);
	//const u32 changed_outputs = (old ^ m_ppc_regs.ppsr_out) & m_ppc_regs.ppdr;
	//if (changed_outputs)
	//{
		// Do stuff
	//}
}

u32 sa1110_periphs_device::ppc_ppar_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ppc_regs.ppar;
	LOGMASKED(LOG_PPC, "%s: ppc_ppar_r: PPC Pin Assignment Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::ppc_ppar_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_PPC, "%s: ppc_ppar_w: PPC Pin Assignment Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ppc_regs.ppar);
}

u32 sa1110_periphs_device::ppc_psdr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ppc_regs.psdr;
	LOGMASKED(LOG_PPC, "%s: ppc_psdr_r: PPC Sleep Mode Direction Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::ppc_psdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_PPC, "%s: ppc_psdr_w: PPC Sleep Mode Direction Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ppc_regs.psdr);
}

u32 sa1110_periphs_device::ppc_ppfr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ppc_regs.ppfr;
	LOGMASKED(LOG_PPC, "%s: ppc_ppfr_r: PPC Pin Flag Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void sa1110_periphs_device::ppc_ppfr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_PPC, "%s: ppc_ppfr_w: PPC Pin Flag Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ppc_regs.ppfr);
}


/*

  Intel SA-1110 Peripheral Pin Controller

  pg. 186 to 194 Intel StrongARM SA-1110 Microprocessor Developer's Manual

*/

void sa1110_periphs_device::dma_set_control_bits(int channel, u32 bits)
{
	dma_regs &regs = m_dma_regs[channel];
	const u32 old = regs.dsr;
	regs.dsr |= bits;
	const u32 newly_set = ~old & bits;
	if (newly_set == 0)
		return;

	const u32 irq_mask = (1 << DSR_ERROR_BIT) | (1 << DSR_DONEA_BIT) | (1 << DSR_DONEB_BIT);

	if (BIT(newly_set, DSR_RUN_BIT))
		regs.dsr &= ~(1 << DSR_ERROR_BIT);
	if (BIT(newly_set, DSR_DONEA_BIT) || BIT(newly_set, DSR_STRTA_BIT))
		regs.dsr &= ~(1 << DSR_DONEA_BIT);
	if (BIT(newly_set, DSR_DONEB_BIT) || BIT(newly_set, DSR_STRTB_BIT))
		regs.dsr &= ~(1 << DSR_DONEB_BIT);

	if (BIT(regs.dsr, DSR_RUN_BIT))
	{
		const u32 buf = BIT(regs.dsr, DSR_BIU_BIT);
		const u32 start_mask = (buf ? (1 << DSR_STRTB_BIT) : (1 << DSR_STRTA_BIT));
		const u32 done_mask = (buf ? (1 << DSR_DONEB_BIT) : (1 << DSR_DONEA_BIT));
		if (regs.dsr & start_mask)
		{
			const u32 count = regs.dbt[buf];
			if (count && regs.ddar == 0x81400580)
			{
				const u32 addr = regs.dbs[buf];
				address_space &space = m_maincpu->space(AS_PROGRAM);
				for (u32 i = 0; i < count; i++)
				{
					const u8 value = space.read_byte(addr + i);
					if (value == 0x0d || value == 0x0a || (value >= 0x20 && value < 0x7f))
					{
						osd_printf_debug("%c", (char)value);
					}
				}
				osd_printf_debug("\n");
				set_irq_line(INT_DMA0 + channel, (BIT(regs.dsr, DSR_IE_BIT) && (regs.dsr & irq_mask)) ? 1 : 0);
			}
			regs.dsr &= ~(1 << DSR_RUN_BIT);
			regs.dsr &= ~start_mask;
			regs.dsr |= done_mask;
			regs.dsr ^= (1 << DSR_BIU_BIT);
		}
	}
	//set_irq_line(INT_DMA0 + channel, (BIT(regs.dsr, DSR_IE_BIT) && (regs.dsr & irq_mask)) ? 1 : 0);
}

void sa1110_periphs_device::dma_clear_control_bits(int channel, u32 bits)
{
	dma_regs &regs = m_dma_regs[channel];

	const u32 irq_mask = (1 << DSR_ERROR_BIT) | (1 << DSR_DONEA_BIT) | (1 << DSR_DONEB_BIT);

	regs.dsr &= ~bits;
	set_irq_line(INT_DMA0 + channel, (BIT(regs.dsr, DSR_IE_BIT) && (regs.dsr & irq_mask)) ? 1 : 0);
}

template <int Channel>
u32 sa1110_periphs_device::dma_ddar_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].ddar;
	LOGMASKED(LOG_DMA, "%s: dma_ddar_r: DMA%d Device Address Register: %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_ddar_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_ddar_w: DMA%d Device Address Register = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	COMBINE_DATA(&m_dma_regs[Channel].ddar);
}

template <int Channel>
u32 sa1110_periphs_device::dma_dssr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].dsr;
	LOGMASKED(LOG_DMA, "%s: dma_dssr_r: DMA%d Control/Status Register (DSSR): %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_dssr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dssr_w: DMA%d Control/Status Register (1S) = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	LOGMASKED(LOG_DMA, "%s:             Run Set: %d\n", machine().describe_context(), BIT(data, DSR_RUN_BIT));
	LOGMASKED(LOG_DMA, "%s:             Interrupt Enable Set: %d\n", machine().describe_context(), BIT(data, DSR_IE_BIT));
	LOGMASKED(LOG_DMA, "%s:             Error Set: %d\n", machine().describe_context(), BIT(data, DSR_ERROR_BIT));
	LOGMASKED(LOG_DMA, "%s:             Done A Set: %d\n", machine().describe_context(), BIT(data, DSR_DONEA_BIT));
	LOGMASKED(LOG_DMA, "%s:             Start A Set: %d\n", machine().describe_context(), BIT(data, DSR_STRTA_BIT));
	LOGMASKED(LOG_DMA, "%s:             Done B Set: %d\n", machine().describe_context(), BIT(data, DSR_DONEB_BIT));
	LOGMASKED(LOG_DMA, "%s:             Start B Set: %d\n", machine().describe_context(), BIT(data, DSR_STRTB_BIT));
	LOGMASKED(LOG_DMA, "%s:             Buffer In Use Set: %d\n", machine().describe_context(), BIT(data, DSR_BIU_BIT));
	dma_set_control_bits(Channel, data & mem_mask);
}

template <int Channel>
u32 sa1110_periphs_device::dma_dcsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].dsr;
	LOGMASKED(LOG_DMA, "%s: dma_dcsr_r: DMA%d Control/Status Register (DCSR): %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_dcsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dcsr_w: DMA%d Control/Status Register (1C) = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	LOGMASKED(LOG_DMA, "%s:             Run Clear: %d\n", machine().describe_context(), BIT(data, DSR_RUN_BIT));
	LOGMASKED(LOG_DMA, "%s:             Interrupt Enable Clear: %d\n", machine().describe_context(), BIT(data, DSR_IE_BIT));
	LOGMASKED(LOG_DMA, "%s:             Error Clear: %d\n", machine().describe_context(), BIT(data, DSR_ERROR_BIT));
	LOGMASKED(LOG_DMA, "%s:             Done A Clear: %d\n", machine().describe_context(), BIT(data, DSR_DONEA_BIT));
	LOGMASKED(LOG_DMA, "%s:             Start A Clear: %d\n", machine().describe_context(), BIT(data, DSR_STRTA_BIT));
	LOGMASKED(LOG_DMA, "%s:             Done B Clear: %d\n", machine().describe_context(), BIT(data, DSR_DONEB_BIT));
	LOGMASKED(LOG_DMA, "%s:             Start B Clear: %d\n", machine().describe_context(), BIT(data, DSR_STRTB_BIT));
	LOGMASKED(LOG_DMA, "%s:             Buffer In Use Clear: %d\n", machine().describe_context(), BIT(data, DSR_BIU_BIT));
	dma_clear_control_bits(Channel, data & mem_mask);
}

template <int Channel>
u32 sa1110_periphs_device::dma_dsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].dsr;
	LOGMASKED(LOG_DMA, "%s: dma_dsr_r: DMA%d Control/Status Register (DSR): %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_dsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dsr_w: DMA%d Control/Status Register (Read-Only, Ignored) = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
}

template <int Channel>
u32 sa1110_periphs_device::dma_dbsa_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].dbs[0];
	LOGMASKED(LOG_DMA, "%s: dma_dbsa_r: DMA%d Buffer A Start Address: %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_dbsa_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dbsa_w: DMA%d Buffer A Start Address = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	if (!BIT(m_dma_regs[Channel].dsr, DSR_STRTA_BIT))
		COMBINE_DATA(&m_dma_regs[Channel].dbs[0]);
}

template <int Channel>
u32 sa1110_periphs_device::dma_dbta_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].dbt[0];
	LOGMASKED(LOG_DMA, "%s: dma_dbta_r: DMA%d Buffer A Transfer Count: %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_dbta_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dbta_w: DMA%d Buffer A Transfer Count = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	if (!BIT(m_dma_regs[Channel].dsr, DSR_STRTA_BIT))
	{
		COMBINE_DATA(&m_dma_regs[Channel].dbt[0]);
		m_dma_regs[Channel].dbt[0] &= DBT_MASK;
	}
}

template <int Channel>
u32 sa1110_periphs_device::dma_dbsb_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].dbs[1];
	LOGMASKED(LOG_DMA, "%s: dma_dbsb_r: DMA%d Buffer B Start Address: %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_dbsb_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dbsb_w: DMA%d Buffer B Start Address = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	if (!BIT(m_dma_regs[Channel].dsr, DSR_STRTB_BIT))
		COMBINE_DATA(&m_dma_regs[Channel].dbs[1]);
}

template <int Channel>
u32 sa1110_periphs_device::dma_dbtb_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs[Channel].dbt[1];
	LOGMASKED(LOG_DMA, "%s: dma_dbtb_r: DMA%d Buffer B Transfer Count: %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	return data;
}

template <int Channel>
void sa1110_periphs_device::dma_dbtb_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dbtb_w: DMA%d Buffer B Transfer Count = %08x & %08x\n", machine().describe_context(), Channel, data, mem_mask);
	if (!BIT(m_dma_regs[Channel].dsr, DSR_STRTB_BIT))
	{
		COMBINE_DATA(&m_dma_regs[Channel].dbt[1]);
		m_dma_regs[Channel].dbt[1] &= DBT_MASK;
	}
}

void sa1110_periphs_device::device_start()
{
	save_item(NAME(m_udc_regs.udccr));
	save_item(NAME(m_udc_regs.udcar));
	save_item(NAME(m_udc_regs.udcomp));
	save_item(NAME(m_udc_regs.udcimp));
	save_item(NAME(m_udc_regs.udccs0));
	save_item(NAME(m_udc_regs.udccs1));
	save_item(NAME(m_udc_regs.udccs2));
	save_item(NAME(m_udc_regs.udcwc));
	save_item(NAME(m_udc_regs.udcsr));

	save_item(NAME(m_icp_regs.uart.utcr));
	save_item(NAME(m_icp_regs.uart.utsr0));
	save_item(NAME(m_icp_regs.uart.utsr1));
	save_item(NAME(m_icp_regs.uart.rx_fifo));
	save_item(NAME(m_icp_regs.uart.rx_fifo_read_idx));
	save_item(NAME(m_icp_regs.uart.rx_fifo_write_idx));
	save_item(NAME(m_icp_regs.uart.rx_fifo_count));
	m_icp_regs.uart_rx_timer = timer_alloc(FUNC(sa1110_periphs_device::icp_rx_callback), this);
	save_item(NAME(m_icp_regs.uart.tx_fifo));
	save_item(NAME(m_icp_regs.uart.tx_fifo_read_idx));
	save_item(NAME(m_icp_regs.uart.tx_fifo_write_idx));
	save_item(NAME(m_icp_regs.uart.tx_fifo_count));
	m_icp_regs.uart_tx_timer = timer_alloc(FUNC(sa1110_periphs_device::icp_tx_callback), this);
	save_item(NAME(m_icp_regs.uart.rx_break_interlock));

	save_item(NAME(m_icp_regs.utcr4));
	save_item(NAME(m_icp_regs.hssp.hscr0));
	save_item(NAME(m_icp_regs.hssp.hscr1));
	save_item(NAME(m_icp_regs.hssp.hssr0));
	save_item(NAME(m_icp_regs.hssp.hssr1));
	save_item(NAME(m_icp_regs.hssp.rx_fifo));
	save_item(NAME(m_icp_regs.hssp.rx_fifo_read_idx));
	save_item(NAME(m_icp_regs.hssp.rx_fifo_write_idx));
	save_item(NAME(m_icp_regs.hssp.rx_fifo_count));
	m_icp_regs.hssp.rx_timer = timer_alloc(FUNC(sa1110_periphs_device::hssp_rx_callback), this);
	save_item(NAME(m_icp_regs.hssp.tx_fifo));
	save_item(NAME(m_icp_regs.hssp.tx_fifo_read_idx));
	save_item(NAME(m_icp_regs.hssp.tx_fifo_write_idx));
	save_item(NAME(m_icp_regs.hssp.tx_fifo_count));
	m_icp_regs.hssp.tx_timer = timer_alloc(FUNC(sa1110_periphs_device::hssp_tx_callback), this);

	save_item(NAME(m_uart_regs.utcr));
	save_item(NAME(m_uart_regs.utsr0));
	save_item(NAME(m_uart_regs.utsr1));
	save_item(NAME(m_uart_regs.rx_fifo));
	save_item(NAME(m_uart_regs.rx_fifo_read_idx));
	save_item(NAME(m_uart_regs.rx_fifo_write_idx));
	save_item(NAME(m_uart_regs.rx_fifo_count));
	save_item(NAME(m_uart_regs.tx_fifo));
	save_item(NAME(m_uart_regs.tx_fifo_read_idx));
	save_item(NAME(m_uart_regs.tx_fifo_write_idx));
	save_item(NAME(m_uart_regs.tx_fifo_count));
	save_item(NAME(m_uart_regs.rx_break_interlock));

	save_item(NAME(m_mcp_regs.mccr0));
	save_item(NAME(m_mcp_regs.mccr1));
	save_item(NAME(m_mcp_regs.mcdr2));
	save_item(NAME(m_mcp_regs.mcsr));
	save_item(NAME(m_mcp_regs.audio_rx_fifo));
	save_item(NAME(m_mcp_regs.audio_rx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.audio_rx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.audio_rx_fifo_count));
	save_item(NAME(m_mcp_regs.audio_tx_fifo));
	save_item(NAME(m_mcp_regs.audio_tx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.audio_tx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.audio_tx_fifo_count));
	m_mcp_regs.audio_tx_timer = timer_alloc(FUNC(sa1110_periphs_device::mcp_audio_tx_callback), this);
	save_item(NAME(m_mcp_regs.telecom_rx_fifo));
	save_item(NAME(m_mcp_regs.telecom_rx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.telecom_rx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.telecom_rx_fifo_count));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo_read_idx));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo_write_idx));
	save_item(NAME(m_mcp_regs.telecom_tx_fifo_count));
	m_mcp_regs.telecom_tx_timer = timer_alloc(FUNC(sa1110_periphs_device::mcp_telecom_tx_callback), this);

	save_item(NAME(m_ssp_regs.sscr0));
	save_item(NAME(m_ssp_regs.sscr1));
	save_item(NAME(m_ssp_regs.sssr));
	save_item(NAME(m_ssp_regs.rx_fifo));
	save_item(NAME(m_ssp_regs.rx_fifo_read_idx));
	save_item(NAME(m_ssp_regs.rx_fifo_write_idx));
	save_item(NAME(m_ssp_regs.rx_fifo_count));
	m_ssp_regs.rx_timer = timer_alloc(FUNC(sa1110_periphs_device::ssp_rx_callback), this);
	save_item(NAME(m_ssp_regs.tx_fifo));
	save_item(NAME(m_ssp_regs.tx_fifo_read_idx));
	save_item(NAME(m_ssp_regs.tx_fifo_write_idx));
	save_item(NAME(m_ssp_regs.tx_fifo_count));
	m_ssp_regs.tx_timer = timer_alloc(FUNC(sa1110_periphs_device::ssp_tx_callback), this);

	save_item(NAME(m_ostmr_regs.osmr));
	save_item(NAME(m_ostmr_regs.oscr));
	save_item(NAME(m_ostmr_regs.ossr));
	save_item(NAME(m_ostmr_regs.ower));
	save_item(NAME(m_ostmr_regs.oier));
	for (int i = 0; i < 4; i++)
	{
		m_ostmr_regs.timer[i] = timer_alloc(FUNC(sa1110_periphs_device::ostimer_tick_cb), this);
	}

	save_item(NAME(m_rtc_regs.rtar));
	save_item(NAME(m_rtc_regs.rcnr));
	save_item(NAME(m_rtc_regs.rttr));
	save_item(NAME(m_rtc_regs.rtsr));
	m_rtc_regs.tick_timer = timer_alloc(FUNC(sa1110_periphs_device::rtc_tick_cb), this);

	save_item(NAME(m_power_regs.pmcr));
	save_item(NAME(m_power_regs.pssr));
	save_item(NAME(m_power_regs.pspr));
	save_item(NAME(m_power_regs.pwer));
	save_item(NAME(m_power_regs.pcfr));
	save_item(NAME(m_power_regs.ppcr));
	save_item(NAME(m_power_regs.pgsr));
	save_item(NAME(m_power_regs.posr));

	save_item(NAME(m_rcsr));

	save_item(NAME(m_gpio_regs.gplr));
	save_item(NAME(m_gpio_regs.gpdr));
	save_item(NAME(m_gpio_regs.grer));
	save_item(NAME(m_gpio_regs.gfer));
	save_item(NAME(m_gpio_regs.gedr));
	save_item(NAME(m_gpio_regs.gafr));
	save_item(NAME(m_gpio_regs.any_edge_mask));
	save_item(NAME(m_gpio_regs.output_latch));
	save_item(NAME(m_gpio_regs.input_latch));
	save_item(NAME(m_gpio_regs.alt_output_latch));
	save_item(NAME(m_gpio_regs.alt_input_latch));

	save_item(NAME(m_intc_regs.icip));
	save_item(NAME(m_intc_regs.icmr));
	save_item(NAME(m_intc_regs.iclr));
	save_item(NAME(m_intc_regs.iccr));
	save_item(NAME(m_intc_regs.icfp));
	save_item(NAME(m_intc_regs.icpr));

	save_item(NAME(m_ppc_regs.ppdr));
	save_item(NAME(m_ppc_regs.ppsr));
	save_item(NAME(m_ppc_regs.ppar));
	save_item(NAME(m_ppc_regs.psdr));
	save_item(NAME(m_ppc_regs.ppfr));

	save_item(STRUCT_MEMBER(m_dma_regs, ddar));
	save_item(STRUCT_MEMBER(m_dma_regs, dsr));
	save_item(STRUCT_MEMBER(m_dma_regs, dbs));
	save_item(STRUCT_MEMBER(m_dma_regs, dbt));
	save_item(NAME(m_dma_active_mask));
}

void sa1110_periphs_device::device_reset()
{
	m_udc_regs.udccr = (1 << UDCCR_SUSM_BIT) | (1 << UDCCR_UDD_BIT);
	m_udc_regs.udcar = 0;
	m_udc_regs.udcomp = 8;
	m_udc_regs.udcimp = 8;
	m_udc_regs.udccs0 = 0;
	m_udc_regs.udccs1 = 0;
	m_udc_regs.udccs2 = 0;
	m_udc_regs.udcwc = 0;
	m_udc_regs.udcsr = 0;

	// init ICP
	std::fill_n(&m_icp_regs.uart.utcr[0], 4, 0);
	m_icp_regs.uart.utsr0 = 0;
	m_icp_regs.uart.utsr1 = 0;
	std::fill_n(&m_icp_regs.uart.rx_fifo[0], 12, 0);
	m_icp_regs.uart.rx_fifo_read_idx = 0;
	m_icp_regs.uart.rx_fifo_write_idx = 0;
	m_icp_regs.uart.rx_fifo_count = 0;
	m_icp_regs.uart_rx_timer->adjust(attotime::never);
	std::fill_n(&m_icp_regs.uart.tx_fifo[0], 8, 0);
	m_icp_regs.uart.tx_fifo_read_idx = 0;
	m_icp_regs.uart.tx_fifo_write_idx = 0;
	m_icp_regs.uart.tx_fifo_count = 0;
	m_icp_regs.uart_tx_timer->adjust(attotime::never);
	m_icp_regs.uart.rx_break_interlock = false;

	m_icp_regs.utcr4 = 0;
	m_icp_regs.hssp.hscr0 = 0;
	m_icp_regs.hssp.hscr1 = 0;
	m_icp_regs.hssp.hssr0 = 0;
	m_icp_regs.hssp.hssr1 = 0;
	std::fill_n(&m_icp_regs.hssp.rx_fifo[0], 4, 0);
	m_icp_regs.hssp.rx_fifo_read_idx = 0;
	m_icp_regs.hssp.rx_fifo_write_idx = 0;
	m_icp_regs.hssp.rx_fifo_count = 0;
	m_icp_regs.hssp.rx_timer->adjust(attotime::never);
	std::fill_n(&m_icp_regs.hssp.tx_fifo[0], 12, 0);
	m_icp_regs.hssp.tx_fifo_read_idx = 0;
	m_icp_regs.hssp.tx_fifo_write_idx = 0;
	m_icp_regs.hssp.tx_fifo_count = 0;
	m_icp_regs.hssp.tx_timer->adjust(attotime::never);

	// init UART3
	std::fill_n(&m_uart_regs.utcr[0], 4, 0);
	m_uart_regs.utsr0 = 0;
	m_uart_regs.utsr1 = 0;
	std::fill_n(&m_uart_regs.rx_fifo[0], 12, 0);
	m_uart_regs.rx_fifo_read_idx = 0;
	m_uart_regs.rx_fifo_write_idx = 0;
	m_uart_regs.rx_fifo_count = 0;
	std::fill_n(&m_uart_regs.tx_fifo[0], 8, 0);
	m_uart_regs.tx_fifo_read_idx = 0;
	m_uart_regs.tx_fifo_write_idx = 0;
	m_uart_regs.tx_fifo_count = 0;
	m_uart_regs.rx_break_interlock = false;

	transmit_register_reset();
	receive_register_reset();

	// init MCP regs
	m_mcp_regs.mccr0 = 0;
	m_mcp_regs.mccr1 = 0;
	m_mcp_regs.mcdr2 = 0;
	m_mcp_regs.mcsr = (1 << MCSR_ANF_BIT) | (1 << MCSR_TNF_BIT);
	std::fill(std::begin(m_mcp_regs.audio_rx_fifo), std::end(m_mcp_regs.audio_rx_fifo), 0);
	m_mcp_regs.audio_rx_fifo_read_idx = 0;
	m_mcp_regs.audio_rx_fifo_write_idx = 0;
	m_mcp_regs.audio_rx_fifo_count = 0;
	std::fill(std::begin(m_mcp_regs.audio_tx_fifo), std::end(m_mcp_regs.audio_tx_fifo), 0);
	m_mcp_regs.audio_tx_fifo_read_idx = 0;
	m_mcp_regs.audio_tx_fifo_write_idx = 0;
	m_mcp_regs.audio_tx_fifo_count = 0;
	m_mcp_regs.audio_tx_timer->adjust(attotime::never);
	std::fill(std::begin(m_mcp_regs.telecom_rx_fifo), std::end(m_mcp_regs.telecom_rx_fifo), 0);
	m_mcp_regs.telecom_rx_fifo_read_idx = 0;
	m_mcp_regs.telecom_rx_fifo_write_idx = 0;
	m_mcp_regs.telecom_rx_fifo_count = 0;
	std::fill(std::begin(m_mcp_regs.telecom_tx_fifo), std::end(m_mcp_regs.telecom_tx_fifo), 0);
	m_mcp_regs.telecom_tx_fifo_read_idx = 0;
	m_mcp_regs.telecom_tx_fifo_write_idx = 0;
	m_mcp_regs.telecom_tx_fifo_count = 0;
	m_mcp_regs.telecom_tx_timer->adjust(attotime::never);

	// init SSP regs
	m_ssp_regs.sscr0 = 0;
	m_ssp_regs.sscr1 = 0;
	m_ssp_regs.sssr = (1 << SSSR_TNF_BIT);
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

	// init OS timers
	std::fill_n(&m_ostmr_regs.osmr[0], 4, 0);
	m_ostmr_regs.ower = 0;
	m_ostmr_regs.ossr = 0;
	m_ostmr_regs.oier = 0;
	for (int i = 0; i < 4; i++)
	{
		m_ostmr_regs.timer[i]->adjust(attotime::never);
	}
	m_ostmr_regs.last_count_sync = attotime::zero;

	// init RTC
	m_rtc_regs.rtar = 0;
	m_rtc_regs.rcnr = 0;
	m_rtc_regs.rttr = 0;
	m_rtc_regs.rtsr = 0;
	m_rtc_regs.tick_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	// init power regs
	m_power_regs.pmcr = 0;
	m_power_regs.pssr = 0;
	m_power_regs.pspr = 0;
	m_power_regs.pwer = 0;
	m_power_regs.pcfr = 0;
	m_power_regs.ppcr = 0;
	m_power_regs.pgsr = 0;
	m_power_regs.posr = 1; // flag oscillator OK

	// init PPC regs
	m_ppc_regs.ppdr = 0;
	m_ppc_regs.ppsr = 0;
	m_ppc_regs.ppar = 0;
	m_ppc_regs.psdr = 0x003fffff;
	m_ppc_regs.ppfr = 0x0007f001;

	// init DMA regs
	for (dma_regs &regs : m_dma_regs)
	{
		regs.ddar = 0;
		regs.dsr = 0;
		std::fill_n(&regs.dbs[0], 2, 0);
		std::fill_n(&regs.dbt[0], 2, 0);
	}

	m_rcsr = 0x00000001; // indicate hardware reset

	m_gpio_regs.gplr = 0;
	m_gpio_regs.gpdr = 0;
	m_gpio_regs.grer = 0;
	m_gpio_regs.gfer = 0;
	m_gpio_regs.gedr = 0;
	m_gpio_regs.gafr = 0;
	m_gpio_regs.any_edge_mask = 0;
	m_gpio_regs.output_latch = 0;
	m_gpio_regs.input_latch = 0;
	m_gpio_regs.alt_output_latch = 0;
	m_gpio_regs.alt_input_latch = 0;

	m_intc_regs.icip = 0;
	m_intc_regs.icmr = 0;
	m_intc_regs.iclr = 0;
	m_intc_regs.iccr = 0;
	m_intc_regs.icfp = 0;
	m_intc_regs.icpr = 0;

	uart_check_rx_fifo_service();
	uart_check_tx_fifo_service();
}

void sa1110_periphs_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_uart3_irqs).output_handler().set(FUNC(sa1110_periphs_device::uart3_irq_callback));
	INPUT_MERGER_ANY_HIGH(config, m_mcp_irqs).output_handler().set(FUNC(sa1110_periphs_device::mcp_irq_callback));
}
