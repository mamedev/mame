// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Motorola 68328 ("DragonBall") System-on-a-Chip implementation

    By Ryan Holtz

**********************************************************************/

#include "emu.h"
#include "machine/mc68328.h"

#define LOG_SCR         (1U << 1)
#define LOG_CS_GRP      (1U << 2)
#define LOG_CS_SEL      (1U << 3)
#define LOG_PLL         (1U << 4)
#define LOG_INTS        (1U << 5)
#define LOG_GPIO_A      (1U << 6)
#define LOG_GPIO_B      (1U << 7)
#define LOG_GPIO_C      (1U << 8)
#define LOG_GPIO_D      (1U << 9)
#define LOG_GPIO_E      (1U << 10)
#define LOG_GPIO_F      (1U << 11)
#define LOG_GPIO_G      (1U << 12)
#define LOG_GPIO_J      (1U << 13)
#define LOG_GPIO_K      (1U << 14)
#define LOG_GPIO_M      (1U << 15)
#define LOG_PWM         (1U << 16)
#define LOG_TIMERS      (1U << 17)
#define LOG_TSTAT       (1U << 18)
#define LOG_WATCHDOG    (1U << 19)
#define LOG_SPIS        (1U << 20)
#define LOG_SPIM        (1U << 21)
#define LOG_UART        (1U << 22)
#define LOG_LCD         (1U << 23)
#define LOG_RTC         (1U << 24)
#define LOG_ALL         (LOG_SCR | LOG_CS_GRP | LOG_CS_SEL | LOG_PLL | LOG_INTS | LOG_GPIO_A | LOG_GPIO_B | LOG_GPIO_C | LOG_GPIO_D | LOG_GPIO_E \
						| LOG_GPIO_F | LOG_GPIO_G | LOG_GPIO_J | LOG_GPIO_K | LOG_GPIO_M | LOG_PWM | LOG_TIMERS | LOG_TSTAT | LOG_WATCHDOG | LOG_SPIS \
						| LOG_SPIM | LOG_UART | LOG_LCD | LOG_RTC)
#define VERBOSE         (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MC68328, mc68328_device, "mc68328", "MC68328 DragonBall Integrated Processor")

void mc68328_device::internal_map(address_map &map)
{
	map(0xfff000, 0xfff000).rw(FUNC(mc68328_device::scr_r), FUNC(mc68328_device::scr_w));
	map(0xfff100, 0xfff101).rw(FUNC(mc68328_device::grpbasea_r), FUNC(mc68328_device::grpbasea_w));
	map(0xfff102, 0xfff103).rw(FUNC(mc68328_device::grpbaseb_r), FUNC(mc68328_device::grpbaseb_w));
	map(0xfff104, 0xfff105).rw(FUNC(mc68328_device::grpbasec_r), FUNC(mc68328_device::grpbasec_w));
	map(0xfff106, 0xfff107).rw(FUNC(mc68328_device::grpbased_r), FUNC(mc68328_device::grpbased_w));
	map(0xfff108, 0xfff109).rw(FUNC(mc68328_device::grpmaska_r), FUNC(mc68328_device::grpmaska_w));
	map(0xfff10a, 0xfff10b).rw(FUNC(mc68328_device::grpmaskb_r), FUNC(mc68328_device::grpmaskb_w));
	map(0xfff10c, 0xfff10d).rw(FUNC(mc68328_device::grpmaskc_r), FUNC(mc68328_device::grpmaskc_w));
	map(0xfff10e, 0xfff10f).rw(FUNC(mc68328_device::grpmaskd_r), FUNC(mc68328_device::grpmaskd_w));
	map(0xfff110, 0xfff111).rw(FUNC(mc68328_device::csa_msw_r<0>), FUNC(mc68328_device::csa_msw_w<0>));
	map(0xfff112, 0xfff113).rw(FUNC(mc68328_device::csa_lsw_r<0>), FUNC(mc68328_device::csa_lsw_w<0>));
	map(0xfff114, 0xfff115).rw(FUNC(mc68328_device::csa_msw_r<1>), FUNC(mc68328_device::csa_msw_w<1>));
	map(0xfff116, 0xfff117).rw(FUNC(mc68328_device::csa_lsw_r<1>), FUNC(mc68328_device::csa_lsw_w<1>));
	map(0xfff118, 0xfff119).rw(FUNC(mc68328_device::csa_msw_r<2>), FUNC(mc68328_device::csa_msw_w<2>));
	map(0xfff11a, 0xfff11b).rw(FUNC(mc68328_device::csa_lsw_r<2>), FUNC(mc68328_device::csa_lsw_w<2>));
	map(0xfff11c, 0xfff11d).rw(FUNC(mc68328_device::csa_msw_r<3>), FUNC(mc68328_device::csa_msw_w<3>));
	map(0xfff11e, 0xfff11f).rw(FUNC(mc68328_device::csa_lsw_r<3>), FUNC(mc68328_device::csa_lsw_w<3>));
	map(0xfff120, 0xfff121).rw(FUNC(mc68328_device::csb_msw_r<0>), FUNC(mc68328_device::csb_msw_w<0>));
	map(0xfff122, 0xfff123).rw(FUNC(mc68328_device::csb_lsw_r<0>), FUNC(mc68328_device::csb_lsw_w<0>));
	map(0xfff124, 0xfff125).rw(FUNC(mc68328_device::csb_msw_r<1>), FUNC(mc68328_device::csb_msw_w<1>));
	map(0xfff126, 0xfff127).rw(FUNC(mc68328_device::csb_lsw_r<1>), FUNC(mc68328_device::csb_lsw_w<1>));
	map(0xfff128, 0xfff129).rw(FUNC(mc68328_device::csb_msw_r<2>), FUNC(mc68328_device::csb_msw_w<2>));
	map(0xfff12a, 0xfff12b).rw(FUNC(mc68328_device::csb_lsw_r<2>), FUNC(mc68328_device::csb_lsw_w<2>));
	map(0xfff12c, 0xfff12d).rw(FUNC(mc68328_device::csb_msw_r<3>), FUNC(mc68328_device::csb_msw_w<3>));
	map(0xfff12e, 0xfff12f).rw(FUNC(mc68328_device::csb_lsw_r<3>), FUNC(mc68328_device::csb_lsw_w<3>));
	map(0xfff130, 0xfff131).rw(FUNC(mc68328_device::csc_msw_r<0>), FUNC(mc68328_device::csc_msw_w<0>));
	map(0xfff132, 0xfff133).rw(FUNC(mc68328_device::csc_lsw_r<0>), FUNC(mc68328_device::csc_lsw_w<0>));
	map(0xfff134, 0xfff135).rw(FUNC(mc68328_device::csc_msw_r<1>), FUNC(mc68328_device::csc_msw_w<1>));
	map(0xfff136, 0xfff137).rw(FUNC(mc68328_device::csc_lsw_r<1>), FUNC(mc68328_device::csc_lsw_w<1>));
	map(0xfff138, 0xfff139).rw(FUNC(mc68328_device::csc_msw_r<2>), FUNC(mc68328_device::csc_msw_w<2>));
	map(0xfff13a, 0xfff13b).rw(FUNC(mc68328_device::csc_lsw_r<2>), FUNC(mc68328_device::csc_lsw_w<2>));
	map(0xfff13c, 0xfff13d).rw(FUNC(mc68328_device::csc_msw_r<3>), FUNC(mc68328_device::csc_msw_w<3>));
	map(0xfff13e, 0xfff13f).rw(FUNC(mc68328_device::csc_lsw_r<3>), FUNC(mc68328_device::csc_lsw_w<3>));
	map(0xfff140, 0xfff141).rw(FUNC(mc68328_device::csd_msw_r<0>), FUNC(mc68328_device::csd_msw_w<0>));
	map(0xfff142, 0xfff143).rw(FUNC(mc68328_device::csd_lsw_r<0>), FUNC(mc68328_device::csd_lsw_w<0>));
	map(0xfff144, 0xfff145).rw(FUNC(mc68328_device::csd_msw_r<1>), FUNC(mc68328_device::csd_msw_w<1>));
	map(0xfff146, 0xfff147).rw(FUNC(mc68328_device::csd_lsw_r<1>), FUNC(mc68328_device::csd_lsw_w<1>));
	map(0xfff148, 0xfff149).rw(FUNC(mc68328_device::csd_msw_r<2>), FUNC(mc68328_device::csd_msw_w<2>));
	map(0xfff14a, 0xfff14b).rw(FUNC(mc68328_device::csd_lsw_r<2>), FUNC(mc68328_device::csd_lsw_w<2>));
	map(0xfff14c, 0xfff14d).rw(FUNC(mc68328_device::csd_msw_r<3>), FUNC(mc68328_device::csd_msw_w<3>));
	map(0xfff14e, 0xfff14f).rw(FUNC(mc68328_device::csd_lsw_r<3>), FUNC(mc68328_device::csd_lsw_w<3>));

	map(0xfff200, 0xfff201).rw(FUNC(mc68328_device::pllcr_r), FUNC(mc68328_device::pllcr_w));
	map(0xfff202, 0xfff203).rw(FUNC(mc68328_device::pllfsr_r), FUNC(mc68328_device::pllfsr_w));
	map(0xfff207, 0xfff207).rw(FUNC(mc68328_device::pctlr_r), FUNC(mc68328_device::pctlr_w));

	map(0xfff300, 0xfff300).rw(FUNC(mc68328_device::ivr_r), FUNC(mc68328_device::ivr_w));
	map(0xfff302, 0xfff303).rw(FUNC(mc68328_device::icr_r), FUNC(mc68328_device::icr_w));
	map(0xfff304, 0xfff305).rw(FUNC(mc68328_device::imr_msw_r), FUNC(mc68328_device::imr_msw_w));
	map(0xfff306, 0xfff307).rw(FUNC(mc68328_device::imr_lsw_r), FUNC(mc68328_device::imr_lsw_w));
	map(0xfff308, 0xfff309).rw(FUNC(mc68328_device::iwr_msw_r), FUNC(mc68328_device::iwr_msw_w));
	map(0xfff30a, 0xfff30b).rw(FUNC(mc68328_device::iwr_lsw_r), FUNC(mc68328_device::iwr_lsw_w));
	map(0xfff30c, 0xfff30d).rw(FUNC(mc68328_device::isr_msw_r), FUNC(mc68328_device::isr_msw_w));
	map(0xfff30e, 0xfff30f).rw(FUNC(mc68328_device::isr_lsw_r), FUNC(mc68328_device::isr_lsw_w));
	map(0xfff310, 0xfff311).rw(FUNC(mc68328_device::ipr_msw_r), FUNC(mc68328_device::ipr_msw_w));
	map(0xfff312, 0xfff313).rw(FUNC(mc68328_device::ipr_lsw_r), FUNC(mc68328_device::ipr_lsw_w));

	map(0xfff400, 0xfff400).rw(FUNC(mc68328_device::padir_r), FUNC(mc68328_device::padir_w));
	map(0xfff401, 0xfff401).rw(FUNC(mc68328_device::padata_r), FUNC(mc68328_device::padata_w));
	map(0xfff403, 0xfff403).rw(FUNC(mc68328_device::pasel_r), FUNC(mc68328_device::pasel_w));
	map(0xfff408, 0xfff408).rw(FUNC(mc68328_device::pbdir_r), FUNC(mc68328_device::pbdir_w));
	map(0xfff409, 0xfff409).rw(FUNC(mc68328_device::pbdata_r), FUNC(mc68328_device::pbdata_w));
	map(0xfff40b, 0xfff40b).rw(FUNC(mc68328_device::pbsel_r), FUNC(mc68328_device::pbsel_w));
	map(0xfff410, 0xfff410).rw(FUNC(mc68328_device::pcdir_r), FUNC(mc68328_device::pcdir_w));
	map(0xfff411, 0xfff411).rw(FUNC(mc68328_device::pcdata_r), FUNC(mc68328_device::pcdata_w));
	map(0xfff413, 0xfff413).rw(FUNC(mc68328_device::pcsel_r), FUNC(mc68328_device::pcsel_w));
	map(0xfff418, 0xfff418).rw(FUNC(mc68328_device::pddir_r), FUNC(mc68328_device::pddir_w));
	map(0xfff419, 0xfff419).rw(FUNC(mc68328_device::pddata_r), FUNC(mc68328_device::pddata_w));
	map(0xfff41a, 0xfff41a).rw(FUNC(mc68328_device::pdpuen_r), FUNC(mc68328_device::pdpuen_w));
	map(0xfff41c, 0xfff41c).rw(FUNC(mc68328_device::pdpol_r), FUNC(mc68328_device::pdpol_w));
	map(0xfff41d, 0xfff41d).rw(FUNC(mc68328_device::pdirqen_r), FUNC(mc68328_device::pdirqen_w));
	map(0xfff41f, 0xfff41f).rw(FUNC(mc68328_device::pdirqedge_r), FUNC(mc68328_device::pdirqedge_w));
	map(0xfff420, 0xfff420).rw(FUNC(mc68328_device::pedir_r), FUNC(mc68328_device::pedir_w));
	map(0xfff421, 0xfff421).rw(FUNC(mc68328_device::pedata_r), FUNC(mc68328_device::pedata_w));
	map(0xfff422, 0xfff422).rw(FUNC(mc68328_device::pepuen_r), FUNC(mc68328_device::pepuen_w));
	map(0xfff423, 0xfff423).rw(FUNC(mc68328_device::pesel_r), FUNC(mc68328_device::pesel_w));
	map(0xfff428, 0xfff428).rw(FUNC(mc68328_device::pfdir_r), FUNC(mc68328_device::pfdir_w));
	map(0xfff429, 0xfff429).rw(FUNC(mc68328_device::pfdata_r), FUNC(mc68328_device::pfdata_w));
	map(0xfff42a, 0xfff42a).rw(FUNC(mc68328_device::pfpuen_r), FUNC(mc68328_device::pfpuen_w));
	map(0xfff42b, 0xfff42b).rw(FUNC(mc68328_device::pfsel_r), FUNC(mc68328_device::pfsel_w));
	map(0xfff430, 0xfff430).rw(FUNC(mc68328_device::pgdir_r), FUNC(mc68328_device::pgdir_w));
	map(0xfff431, 0xfff431).rw(FUNC(mc68328_device::pgdata_r), FUNC(mc68328_device::pgdata_w));
	map(0xfff432, 0xfff432).rw(FUNC(mc68328_device::pgpuen_r), FUNC(mc68328_device::pgpuen_w));
	map(0xfff433, 0xfff433).rw(FUNC(mc68328_device::pgsel_r), FUNC(mc68328_device::pgsel_w));
	map(0xfff438, 0xfff438).rw(FUNC(mc68328_device::pjdir_r), FUNC(mc68328_device::pjdir_w));
	map(0xfff439, 0xfff439).rw(FUNC(mc68328_device::pjdata_r), FUNC(mc68328_device::pjdata_w));
	map(0xfff43b, 0xfff43b).rw(FUNC(mc68328_device::pjsel_r), FUNC(mc68328_device::pjsel_w));
	map(0xfff440, 0xfff440).rw(FUNC(mc68328_device::pkdir_r), FUNC(mc68328_device::pkdir_w));
	map(0xfff441, 0xfff441).rw(FUNC(mc68328_device::pkdata_r), FUNC(mc68328_device::pkdata_w));
	map(0xfff442, 0xfff442).rw(FUNC(mc68328_device::pkpuen_r), FUNC(mc68328_device::pkpuen_w));
	map(0xfff443, 0xfff443).rw(FUNC(mc68328_device::pksel_r), FUNC(mc68328_device::pksel_w));
	map(0xfff448, 0xfff448).rw(FUNC(mc68328_device::pmdir_r), FUNC(mc68328_device::pmdir_w));
	map(0xfff449, 0xfff449).rw(FUNC(mc68328_device::pmdata_r), FUNC(mc68328_device::pmdata_w));
	map(0xfff44a, 0xfff44a).rw(FUNC(mc68328_device::pmpuen_r), FUNC(mc68328_device::pmpuen_w));
	map(0xfff44b, 0xfff44b).rw(FUNC(mc68328_device::pmsel_r), FUNC(mc68328_device::pmsel_w));

	map(0xfff500, 0xfff501).rw(FUNC(mc68328_device::pwmc_r), FUNC(mc68328_device::pwmc_w));
	map(0xfff502, 0xfff503).rw(FUNC(mc68328_device::pwmp_r), FUNC(mc68328_device::pwmp_w));
	map(0xfff504, 0xfff505).rw(FUNC(mc68328_device::pwmw_r), FUNC(mc68328_device::pwmw_w));
	map(0xfff506, 0xfff507).rw(FUNC(mc68328_device::pwmcnt_r), FUNC(mc68328_device::pwmcnt_w));

	map(0xfff600, 0xfff601).rw(FUNC(mc68328_device::tctl_r<0>), FUNC(mc68328_device::tctl_w<0>));
	map(0xfff602, 0xfff603).rw(FUNC(mc68328_device::tprer_r<0>), FUNC(mc68328_device::tprer_w<0>));
	map(0xfff604, 0xfff605).rw(FUNC(mc68328_device::tcmp_r<0>), FUNC(mc68328_device::tcmp_w<0>));
	map(0xfff606, 0xfff607).rw(FUNC(mc68328_device::tcr_r<0>), FUNC(mc68328_device::tcr_w<0>));
	map(0xfff608, 0xfff609).rw(FUNC(mc68328_device::tcn_r<0>), FUNC(mc68328_device::tcn_w<0>));
	map(0xfff60a, 0xfff60b).rw(FUNC(mc68328_device::tstat_r<0>), FUNC(mc68328_device::tstat_w<0>));
	map(0xfff60c, 0xfff60d).rw(FUNC(mc68328_device::tctl_r<1>), FUNC(mc68328_device::tctl_w<1>));
	map(0xfff60e, 0xfff60f).rw(FUNC(mc68328_device::tprer_r<1>), FUNC(mc68328_device::tprer_w<1>));
	map(0xfff610, 0xfff611).rw(FUNC(mc68328_device::tcmp_r<1>), FUNC(mc68328_device::tcmp_w<1>));
	map(0xfff612, 0xfff613).rw(FUNC(mc68328_device::tcr_r<1>), FUNC(mc68328_device::tcr_w<1>));
	map(0xfff614, 0xfff615).rw(FUNC(mc68328_device::tcn_r<1>), FUNC(mc68328_device::tcn_w<1>));
	map(0xfff616, 0xfff617).rw(FUNC(mc68328_device::tstat_r<1>), FUNC(mc68328_device::tstat_w<1>));
	map(0xfff618, 0xfff619).rw(FUNC(mc68328_device::wctlr_r), FUNC(mc68328_device::wctlr_w));
	map(0xfff61a, 0xfff61b).rw(FUNC(mc68328_device::wcmpr_r), FUNC(mc68328_device::wcmpr_w));
	map(0xfff61c, 0xfff61d).rw(FUNC(mc68328_device::wcn_r), FUNC(mc68328_device::wcn_w));

	map(0xfff700, 0xfff701).rw(FUNC(mc68328_device::spisr_r), FUNC(mc68328_device::spisr_w));

	map(0xfff800, 0xfff801).rw(FUNC(mc68328_device::spimdata_r), FUNC(mc68328_device::spimdata_w));
	map(0xfff802, 0xfff803).rw(FUNC(mc68328_device::spimcont_r), FUNC(mc68328_device::spimcont_w));

	map(0xfff900, 0xfff901).rw(FUNC(mc68328_device::ustcnt_r), FUNC(mc68328_device::ustcnt_w));
	map(0xfff902, 0xfff903).rw(FUNC(mc68328_device::ubaud_r), FUNC(mc68328_device::ubaud_w));
	map(0xfff904, 0xfff905).rw(FUNC(mc68328_device::urx_r), FUNC(mc68328_device::urx_w));
	map(0xfff906, 0xfff907).rw(FUNC(mc68328_device::utx_r), FUNC(mc68328_device::utx_w));
	map(0xfff908, 0xfff909).rw(FUNC(mc68328_device::umisc_r), FUNC(mc68328_device::umisc_w));

	map(0xfffa00, 0xfffa01).rw(FUNC(mc68328_device::lssa_msw_r), FUNC(mc68328_device::lssa_msw_w));
	map(0xfffa02, 0xfffa03).rw(FUNC(mc68328_device::lssa_lsw_r), FUNC(mc68328_device::lssa_lsw_w));
	map(0xfffa05, 0xfffa05).rw(FUNC(mc68328_device::lvpw_r), FUNC(mc68328_device::lvpw_w));
	map(0xfffa08, 0xfffa09).rw(FUNC(mc68328_device::lxmax_r), FUNC(mc68328_device::lxmax_w));
	map(0xfffa0a, 0xfffa0b).rw(FUNC(mc68328_device::lymax_r), FUNC(mc68328_device::lymax_w));
	map(0xfffa18, 0xfffa19).rw(FUNC(mc68328_device::lcxp_r), FUNC(mc68328_device::lcxp_w));
	map(0xfffa1a, 0xfffa1b).rw(FUNC(mc68328_device::lcyp_r), FUNC(mc68328_device::lcyp_w));
	map(0xfffa1c, 0xfffa1d).rw(FUNC(mc68328_device::lcwch_r), FUNC(mc68328_device::lcwch_w));
	map(0xfffa1f, 0xfffa1f).rw(FUNC(mc68328_device::lblkc_r), FUNC(mc68328_device::lblkc_w));
	map(0xfffa20, 0xfffa20).rw(FUNC(mc68328_device::lpicf_r), FUNC(mc68328_device::lpicf_w));
	map(0xfffa21, 0xfffa21).rw(FUNC(mc68328_device::lpolcf_r), FUNC(mc68328_device::lpolcf_w));
	map(0xfffa23, 0xfffa23).rw(FUNC(mc68328_device::lacdrc_r), FUNC(mc68328_device::lacdrc_w));
	map(0xfffa25, 0xfffa25).rw(FUNC(mc68328_device::lpxcd_r), FUNC(mc68328_device::lpxcd_w));
	map(0xfffa27, 0xfffa27).rw(FUNC(mc68328_device::lckcon_r), FUNC(mc68328_device::lckcon_w));
	map(0xfffa29, 0xfffa29).rw(FUNC(mc68328_device::llbar_r), FUNC(mc68328_device::llbar_w));
	map(0xfffa2b, 0xfffa2b).rw(FUNC(mc68328_device::lotcr_r), FUNC(mc68328_device::lotcr_w));
	map(0xfffa2d, 0xfffa2d).rw(FUNC(mc68328_device::lposr_r), FUNC(mc68328_device::lposr_w));
	map(0xfffa31, 0xfffa31).rw(FUNC(mc68328_device::lfrcm_r), FUNC(mc68328_device::lfrcm_w));
	map(0xfffa32, 0xfffa33).rw(FUNC(mc68328_device::lgpmr_r), FUNC(mc68328_device::lgpmr_w));

	map(0xfffb00, 0xfffb01).rw(FUNC(mc68328_device::hmsr_msw_r), FUNC(mc68328_device::hmsr_msw_w));
	map(0xfffb02, 0xfffb03).rw(FUNC(mc68328_device::hmsr_lsw_r), FUNC(mc68328_device::hmsr_lsw_w));
	map(0xfffb04, 0xfffb05).rw(FUNC(mc68328_device::alarm_msw_r), FUNC(mc68328_device::alarm_msw_w));
	map(0xfffb06, 0xfffb07).rw(FUNC(mc68328_device::alarm_lsw_r), FUNC(mc68328_device::alarm_lsw_w));
	map(0xfffb0c, 0xfffb0d).rw(FUNC(mc68328_device::rtcctl_r), FUNC(mc68328_device::rtcctl_w));
	map(0xfffb0e, 0xfffb0f).rw(FUNC(mc68328_device::rtcisr_r), FUNC(mc68328_device::rtcisr_w));
	map(0xfffb10, 0xfffb11).rw(FUNC(mc68328_device::rtcienr_r), FUNC(mc68328_device::rtcienr_w));
	map(0xfffb12, 0xfffb13).rw(FUNC(mc68328_device::stpwtch_r), FUNC(mc68328_device::stpwtch_w));
}

void mc68328_device::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).r(FUNC(mc68328_device::irq_callback)).umask16(0x00ff);
}


mc68328_device::mc68328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m68000_device(mconfig, tag, owner, clock, MC68328, 16, 24, address_map_constructor(FUNC(mc68328_device::internal_map), this))
	, m_rtc(nullptr), m_pwm(nullptr)
	, m_out_port_a_cb(*this)
	, m_out_port_b_cb(*this)
	, m_out_port_c_cb(*this)
	, m_out_port_d_cb(*this)
	, m_out_port_e_cb(*this)
	, m_out_port_f_cb(*this)
	, m_out_port_g_cb(*this)
	, m_out_port_j_cb(*this)
	, m_out_port_k_cb(*this)
	, m_out_port_m_cb(*this)
	, m_in_port_a_cb(*this)
	, m_in_port_b_cb(*this)
	, m_in_port_c_cb(*this)
	, m_in_port_d_cb(*this)
	, m_in_port_e_cb(*this)
	, m_in_port_f_cb(*this)
	, m_in_port_g_cb(*this)
	, m_in_port_j_cb(*this)
	, m_in_port_k_cb(*this)
	, m_in_port_m_cb(*this)
	, m_out_pwm_cb(*this)
	, m_out_spim_cb(*this)
	, m_in_spim_cb(*this)
	, m_spim_xch_trigger_cb(*this)
{
	m_cpu_space_config.m_internal_map = address_map_constructor(FUNC(mc68328_device::cpu_space_map), this);
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mc68328_device::device_resolve_objects()
{
	m68000_device::device_resolve_objects();

	m_out_port_a_cb.resolve_safe();
	m_out_port_b_cb.resolve_safe();
	m_out_port_c_cb.resolve_safe();
	m_out_port_d_cb.resolve_safe();
	m_out_port_e_cb.resolve_safe();
	m_out_port_f_cb.resolve_safe();
	m_out_port_g_cb.resolve_safe();
	m_out_port_j_cb.resolve_safe();
	m_out_port_k_cb.resolve_safe();
	m_out_port_m_cb.resolve_safe();

	m_in_port_a_cb.resolve();
	m_in_port_b_cb.resolve();
	m_in_port_c_cb.resolve();
	m_in_port_d_cb.resolve();
	m_in_port_e_cb.resolve();
	m_in_port_f_cb.resolve();
	m_in_port_g_cb.resolve();
	m_in_port_j_cb.resolve();
	m_in_port_k_cb.resolve();
	m_in_port_m_cb.resolve();

	m_out_pwm_cb.resolve_safe();

	m_out_spim_cb.resolve_safe();
	m_in_spim_cb.resolve();

	m_spim_xch_trigger_cb.resolve();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc68328_device::device_start()
{
	m68000_device::device_start();

	m_gptimer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::timer_tick<0>),this));
	m_gptimer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::timer_tick<1>),this));
	m_rtc = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::rtc_tick),this));
	m_pwm = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mc68328_device::pwm_tick),this));

	register_state_save();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc68328_device::device_reset()
{
	m68000_device::device_reset();

	m_scr = 0x0c;
	m_grpbasea = 0x0000;
	m_grpbaseb = 0x0000;
	m_grpbasec = 0x0000;
	m_grpbased = 0x0000;
	m_grpmaska = 0x0000;
	m_grpmaskb = 0x0000;
	m_grpmaskc = 0x0000;
	m_grpmaskd = 0x0000;
	std::fill(std::begin(m_csa), std::end(m_csa), 0x00010006);
	std::fill(std::begin(m_csb), std::end(m_csb), 0x00010006);
	std::fill(std::begin(m_csc), std::end(m_csc), 0x00010006);
	std::fill(std::begin(m_csd), std::end(m_csd), 0x00010006);

	m_pllcr = 0x2400;
	m_pllfsr = 0x0123;
	m_pctlr = 0x1f;

	m_ivr = 0x00;
	m_icr = 0x0000;
	m_imr = 0x00ffffff;
	m_iwr = 0x00ffffff;
	m_isr = 0x00000000;
	m_ipr = 0x00000000;

	m_padir = 0x00;
	m_padata = 0x00;
	m_pasel = 0x00;
	m_pbdir = 0x00;
	m_pbdata = 0x00;
	m_pbsel = 0x00;
	m_pcdir = 0x00;
	m_pcdata = 0x00;
	m_pcsel = 0x00;
	m_pddir = 0x00;
	m_pddata = 0x00;
	m_pdpuen = 0xff;
	m_pdpol = 0x00;
	m_pdirqen = 0x00;
	m_pddataedge = 0x00;
	m_pdirqedge = 0x00;
	m_pedir = 0x00;
	m_pedata = 0x00;
	m_pepuen = 0x80;
	m_pesel = 0x80;
	m_pfdir = 0x00;
	m_pfdata = 0x00;
	m_pfpuen = 0xff;
	m_pfsel = 0xff;
	m_pgdir = 0x00;
	m_pgdata = 0x00;
	m_pgpuen = 0xff;
	m_pgsel = 0xff;
	m_pjdir = 0x00;
	m_pjdata = 0x00;
	m_pjsel = 0x00;
	m_pkdir = 0x00;
	m_pkdata = 0x00;
	m_pkpuen = 0xff;
	m_pksel = 0xff;
	m_pmdir = 0x00;
	m_pmdata = 0x00;
	m_pmpuen = 0xff;
	m_pmsel = 0xff;

	m_pwmc = 0x0000;
	m_pwmp = 0x0000;
	m_pwmw = 0x0000;
	m_pwmcnt = 0x0000;

	m_tctl[0] = m_tctl[1] = 0x0000;
	m_tprer[0] = m_tprer[1] = 0x0000;
	m_tcmp[0] = m_tcmp[1] = 0xffff;
	m_tcr[0] = m_tcr[1] = 0x0000;
	m_tcn[0] = m_tcn[1] = 0x0000;
	m_tstat[0] = m_tstat[1] = 0x0000;
	m_wctlr = 0x0000;
	m_wcmpr = 0xffff;
	m_wcn = 0x0000;

	m_spisr = 0x0000;

	m_spimdata = 0x0000;
	m_spimcont = 0x0000;

	m_ustcnt = 0x0000;
	m_ubaud = 0x003f;
	m_urx = 0x0000;
	m_utx = 0x0000;
	m_umisc = 0x0000;

	m_lssa = 0x00000000;
	m_lvpw = 0xff;
	m_lxmax = 0x03ff;
	m_lymax = 0x01ff;
	m_lcxp = 0x0000;
	m_lcyp = 0x0000;
	m_lcwch = 0x0101;
	m_lblkc = 0x7f;
	m_lpicf = 0x00;
	m_lpolcf = 0x00;
	m_lacdrc = 0x00;
	m_lpxcd = 0x00;
	m_lckcon = 0x40;
	m_llbar = 0x3e;
	m_lotcr = 0x3f;
	m_lposr = 0x00;
	m_lfrcm = 0xb9;
	m_lgpmr = 0x1073;

	m_hmsr = 0x00000000;
	m_alarm = 0x00000000;
	m_rtcctl = 0x00;
	m_rtcisr = 0x00;
	m_rtcienr = 0x00;
	m_stpwtch = 0x00;

	m_rtc->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
}


void mc68328_device::set_interrupt_line(uint32_t line, uint32_t active)
{
	if (active)
	{
		m_ipr |= line;

		if (!(m_imr & line) && !(m_isr & line))
		{
			m_isr |= line;

			if (m_isr & INT_M68K_LINE7)
			{
				set_input_line(M68K_IRQ_7, ASSERT_LINE);
			}
			else if (m_isr & INT_M68K_LINE6)
			{
				set_input_line(M68K_IRQ_6, ASSERT_LINE);
			}
			else if (m_isr & INT_M68K_LINE5)
			{
				set_input_line(M68K_IRQ_5, ASSERT_LINE);
			}
			else if (m_isr & INT_M68K_LINE4)
			{
				set_input_line(M68K_IRQ_4, ASSERT_LINE);
			}
			else if (m_isr & INT_M68K_LINE3)
			{
				set_input_line(M68K_IRQ_3, ASSERT_LINE);
			}
			else if (m_isr & INT_M68K_LINE2)
			{
				set_input_line(M68K_IRQ_2, ASSERT_LINE);
			}
			else if (m_isr & INT_M68K_LINE1)
			{
				set_input_line(M68K_IRQ_1, ASSERT_LINE);
			}
		}
	}
	else
	{
		m_isr &= ~line;

		if ((line & INT_M68K_LINE7) && !(m_isr & INT_M68K_LINE7))
		{
			set_input_line(M68K_IRQ_7, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE6) && !(m_isr & INT_M68K_LINE6))
		{
			set_input_line(M68K_IRQ_6, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE5) && !(m_isr & INT_M68K_LINE5))
		{
			set_input_line(M68K_IRQ_5, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE4) && !(m_isr & INT_M68K_LINE4))
		{
			set_input_line(M68K_IRQ_4, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE3) && !(m_isr & INT_M68K_LINE3))
		{
			set_input_line(M68K_IRQ_3, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE2) && !(m_isr & INT_M68K_LINE2))
		{
			set_input_line(M68K_IRQ_2, CLEAR_LINE);
		}
		if ((line & INT_M68K_LINE1) && !(m_isr & INT_M68K_LINE1))
		{
			set_input_line(M68K_IRQ_1, CLEAR_LINE);
		}
	}
}

void mc68328_device::poll_port_d_interrupts()
{
	uint8_t line_transitions = m_pddataedge & m_pdirqedge;
	uint8_t line_holds = m_pddata &~ m_pdirqedge;
	uint8_t line_interrupts = (line_transitions | line_holds) & m_pdirqen;

	if (line_interrupts)
	{
		set_interrupt_line(line_interrupts << 8, 1);
	}
	else
	{
		set_interrupt_line(INT_KBDINTS, 0);
	}
}

WRITE_LINE_MEMBER( mc68328_device::set_penirq_line )
{
	if (state)
	{
		set_interrupt_line(INT_PEN, 1);
	}
	else
	{
		m_ipr &= ~INT_PEN;
		set_interrupt_line(INT_PEN, 0);
	}
}

void mc68328_device::set_port_d_lines(uint8_t state, int bit)
{
	uint8_t old_button_state = m_pddata;

	if (state & (1 << bit))
	{
		m_pddata |= (1 << bit);
	}
	else
	{
		m_pddata &= ~(1 << bit);
	}

	m_pddataedge |= ~old_button_state & m_pddata;

	poll_port_d_interrupts();
}

uint8_t mc68328_device::irq_callback(offs_t offset)
{
	return m_ivr | offset;
}

template <int Timer>
uint32_t mc68328_device::get_timer_frequency()
{
	uint32_t frequency = 0;

	switch (m_tctl[Timer] & TCTL_CLKSOURCE)
	{
		case TCTL_CLKSOURCE_SYSCLK:
			frequency = 32768 * 506;
			break;

		case TCTL_CLKSOURCE_SYSCLK16:
			frequency = (32768 * 506) / 16;
			break;

		case TCTL_CLKSOURCE_32KHZ4:
		case TCTL_CLKSOURCE_32KHZ5:
		case TCTL_CLKSOURCE_32KHZ6:
		case TCTL_CLKSOURCE_32KHZ7:
			frequency = 32768;
			break;
	}
	frequency /= (m_tprer[Timer] + 1);

	return frequency;
}

template <int Timer>
void mc68328_device::maybe_start_timer(uint32_t new_enable)
{
	if ((m_tctl[Timer] & TCTL_TEN) == TCTL_TEN_ENABLE && (m_tctl[Timer] & TCTL_CLKSOURCE) > TCTL_CLKSOURCE_STOP)
	{
		if ((m_tctl[Timer] & TCTL_CLKSOURCE) == TCTL_CLKSOURCE_TIN)
		{
			m_gptimer[Timer]->adjust(attotime::never);
		}
		else if (m_tcmp[Timer] == 0)
		{
			m_gptimer[Timer]->adjust(attotime::never);
		}
		else
		{
			uint32_t frequency = get_timer_frequency<Timer>();
			attotime period = (attotime::from_hz(frequency) *  m_tcmp[Timer]);

			if (new_enable)
			{
				m_tcn[Timer] = 0x0000;
			}

			m_gptimer[Timer]->adjust(period);
		}
	}
	else
	{
		m_gptimer[Timer]->adjust(attotime::never);
	}
}

template <int Timer>
TIMER_CALLBACK_MEMBER( mc68328_device::timer_tick )
{
	m_tcn[Timer] = m_tcmp[Timer];
	m_tstat[Timer] |= TSTAT_COMP;

	if ((m_tctl[Timer] & TCTL_FRR) == TCTL_FRR_RESTART)
	{
		uint32_t frequency = get_timer_frequency<Timer>();

		if (frequency > 0)
		{
			attotime period = attotime::from_hz(frequency) * m_tcmp[Timer];

			m_tcn[Timer] = 0x0000;

			m_gptimer[Timer]->adjust(period);
		}
		else
		{
			m_gptimer[Timer]->adjust(attotime::never);
		}
	}
	else
	{
		uint32_t frequency = get_timer_frequency<Timer>();

		if (frequency > 0)
		{
			attotime period = attotime::from_hz(frequency) * 0x10000;

			m_gptimer[Timer]->adjust(period);
		}
		else
		{
			m_gptimer[Timer]->adjust(attotime::never);
		}
	}
	if ((m_tctl[Timer] & TCTL_IRQEN) == TCTL_IRQEN_ENABLE)
	{
		set_interrupt_line(Timer ? INT_TIMER2 : INT_TIMER1, 1);
	}
}

TIMER_CALLBACK_MEMBER( mc68328_device::pwm_tick )
{
	if (m_pwmw >= m_pwmp || m_pwmw == 0 || m_pwmp == 0)
	{
		m_pwm->adjust(attotime::never);
		return;
	}

	if (((m_pwmc & PWMC_POL) == 0 && (m_pwmc & PWMC_PIN) != 0) ||
		((m_pwmc & PWMC_POL) != 0 && (m_pwmc & PWMC_PIN) == 0))
	{
		uint32_t frequency = 32768 * 506;
		uint32_t divisor = 4 << (m_pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
		attotime period;

		frequency /= divisor;
		period = attotime::from_hz(frequency) * (m_pwmp - m_pwmw);

		m_pwm->adjust(period);

		if (m_pwmc & PWMC_IRQEN)
		{
			set_interrupt_line(INT_PWM, 1);
		}
	}
	else
	{
		uint32_t frequency = 32768 * 506;
		uint32_t divisor = 4 << (m_pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
		attotime period;

		frequency /= divisor;
		period = attotime::from_hz(frequency) * m_pwmw;

		m_pwm->adjust(period);
	}

	m_pwmc ^= PWMC_PIN;

	m_out_pwm_cb((m_pwmc & PWMC_PIN) ? 1 : 0);
}

TIMER_CALLBACK_MEMBER( mc68328_device::rtc_tick )
{
	if (m_rtcctl & RTCCTL_ENABLE)
	{
		uint32_t set_int = 0;

		m_hmsr++;

		if (m_rtcienr & RTCINT_SECOND)
		{
			set_int = 1;
			m_rtcisr |= RTCINT_SECOND;
		}

		if ((m_hmsr & 0x0000003f) == 0x0000003c)
		{
			m_hmsr &= 0xffffffc0;
			m_hmsr += 0x00010000;

			if (m_rtcienr & RTCINT_MINUTE)
			{
				set_int = 1;
				m_rtcisr |= RTCINT_MINUTE;
			}

			if ((m_hmsr & 0x003f0000) == 0x003c0000)
			{
				m_hmsr &= 0xffc0ffff;
				m_hmsr += 0x0100000;

				if ((m_hmsr & 0x1f000000) == 0x18000000)
				{
					m_hmsr &= 0xe0ffffff;

					if (m_rtcienr & RTCINT_DAY)
					{
						set_int = 1;
						m_rtcisr |= RTCINT_DAY;
					}
				}
			}

			if (m_stpwtch != 0x003f)
			{
				m_stpwtch--;
				m_stpwtch &= 0x003f;

				if (m_stpwtch == 0x003f)
				{
					if (m_rtcienr & RTCINT_STOPWATCH)
					{
						set_int = 1;
						m_rtcisr |= RTCINT_STOPWATCH;
					}
				}
			}
		}

		if (m_hmsr == m_alarm)
		{
			if (m_rtcienr & RTCINT_ALARM)
			{
				set_int = 1;
				m_rtcisr |= RTCINT_STOPWATCH;
			}
		}

		if (set_int)
		{
			set_interrupt_line(INT_RTC, 1);
		}
		else
		{
			set_interrupt_line(INT_RTC, 0);
		}
	}
}

void mc68328_device::scr_w(uint8_t data) // 0x000
{
	LOGMASKED(LOG_SCR, "scr_w: SCR = %02x\n", data);
}

void mc68328_device::grpbasea_w(uint16_t data) // 0x100
{
	LOGMASKED(LOG_CS_GRP, "grpbasea_w: GRPBASEA = %04x\n", data);
	m_grpbasea = data;
}

void mc68328_device::grpbaseb_w(uint16_t data) // 0x102
{
	LOGMASKED(LOG_CS_GRP, "grpbaseb_w: GRPBASEB = %04x\n", data);
	m_grpbaseb = data;
}

void mc68328_device::grpbasec_w(uint16_t data) // 0x104
{
	LOGMASKED(LOG_CS_GRP, "grpbasec_w: GRPBASEC = %04x\n", data);
	m_grpbasec = data;
}

void mc68328_device::grpbased_w(uint16_t data) // 0x106
{
	LOGMASKED(LOG_CS_GRP, "grpbased_w: GRPBASED = %04x\n", data);
	m_grpbased = data;
}

void mc68328_device::grpmaska_w(uint16_t data) // 0x108
{
	LOGMASKED(LOG_CS_GRP, "grpmaska_w: GRPMASKA = %04x\n", data);
	m_grpmaska = data;
}

void mc68328_device::grpmaskb_w(uint16_t data) // 0x10a
{
	LOGMASKED(LOG_CS_GRP, "grpmaskb_w: GRPMASKB = %04x\n", data);
	m_grpmaskb = data;
}

void mc68328_device::grpmaskc_w(uint16_t data) // 0x10c
{
	LOGMASKED(LOG_CS_GRP, "grpmaskc_w: GRPMASKC = %04x\n", data);
	m_grpmaskc = data;
}

void mc68328_device::grpmaskd_w(uint16_t data) // 0x10e
{
	LOGMASKED(LOG_CS_GRP, "grpmaskd_w: GRPMASKD = %04x\n", data);
	m_grpmaskd = data;
}

template<int ChipSelect>
void mc68328_device::csa_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x110, 0x114, 0x118, 0x11c
{
	LOGMASKED(LOG_CS_SEL, "csa_msw_w<%d>: CSA%d(16) = %04x\n", ChipSelect, ChipSelect, data);
	m_csa[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csa[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
void mc68328_device::csa_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x112, 0x116, 0x11a, 0x11e
{
	LOGMASKED(LOG_CS_SEL, "csa_lsw_w<%d>: CSA%d(0) = %04x\n", ChipSelect, ChipSelect, data);
	m_csa[ChipSelect] &= ~(mem_mask << 16);
	m_csa[ChipSelect] |= (data & mem_mask) << 16;
}

template<int ChipSelect>
void mc68328_device::csb_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x120, 0x124, 0x128, 0x12c
{
	LOGMASKED(LOG_CS_SEL, "csb_msw_w<%d>: CSB%d(MSW) = %04x\n", ChipSelect, ChipSelect, data);
	m_csb[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csb[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
void mc68328_device::csb_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x122, 0x126, 0x12a, 0x12e
{
	LOGMASKED(LOG_CS_SEL, "csb_lsw_w<%d>: CSB%d(LSW) = %04x\n", ChipSelect, ChipSelect, data);
	m_csb[ChipSelect] &= ~(mem_mask << 16);
	m_csb[ChipSelect] |= (data & mem_mask) << 16;
}

template<int ChipSelect>
void mc68328_device::csc_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x130, 0x134, 0x138, 0x13c
{
	LOGMASKED(LOG_CS_SEL, "csc_msw_w<%d>: CSC%d(MSW) = %04x\n", ChipSelect, ChipSelect, data);
	m_csc[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csc[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
void mc68328_device::csc_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x132, 0x136, 0x13a, 0x13e
{
	LOGMASKED(LOG_CS_SEL, "csc_lsw_w<%d>: CSC%d(LSW) = %04x\n", ChipSelect, ChipSelect, data);
	m_csc[ChipSelect] &= ~(mem_mask << 16);
	m_csc[ChipSelect] |= (data & mem_mask) << 16;
}

template<int ChipSelect>
void mc68328_device::csd_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x140, 0x144, 0x148, 0x14c
{
	LOGMASKED(LOG_CS_SEL, "csd_msw_w<%d>: CSD%d(MSW) = %04x\n", ChipSelect, ChipSelect, data);
	m_csd[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csd[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
void mc68328_device::csd_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x142, 0x146, 0x14a, 0x14e
{
	LOGMASKED(LOG_CS_SEL, "csd_lsw_w<%d>: CSD%d(LSW) = %04x\n", ChipSelect, ChipSelect, data);
	m_csd[ChipSelect] &= ~(mem_mask << 16);
	m_csd[ChipSelect] |= (data & mem_mask) << 16;
}

void mc68328_device::pllcr_w(uint16_t data) // 0x200
{
	LOGMASKED(LOG_PLL, "pllcr_w: PLLCR = %04x\n", data);
	m_pllcr = data;
}

void mc68328_device::pllfsr_w(uint16_t data) // 0x202
{
	LOGMASKED(LOG_PLL, "pllfsr_w: PLLFSR = %04x\n", data);
	m_pllfsr = data;
}

void mc68328_device::pctlr_w(uint8_t data) // 0x207
{
	LOGMASKED(LOG_PLL, "pctlr_w: PCTLR = %02x\n", data);
	m_pctlr = data;
}

void mc68328_device::ivr_w(uint8_t data) // 0x300
{
	LOGMASKED(LOG_INTS, "ivr_w: IVR = %02x\n", data);
	m_ivr = data;
}

void mc68328_device::icr_w(uint8_t data) // 0x302
{
	LOGMASKED(LOG_INTS, "icr_w: ICR = %02x\n", data);
	m_icr = data;
}

void mc68328_device::imr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x304
{
	const uint32_t imr_old = m_imr;
	LOGMASKED(LOG_INTS, "imr_msw_w: IMR(MSW) = %04x\n", data);
	m_imr &= ~(mem_mask << 16);
	m_imr |= (data & mem_mask) << 16;
	m_isr &= ~((data & mem_mask) << 16);

	const uint32_t imr_diff = imr_old ^ m_imr;
	set_interrupt_line(imr_diff, 0);
}

void mc68328_device::imr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x306
{
	const uint32_t imr_old = m_imr;
	LOGMASKED(LOG_INTS, "imr_lsw_w: IMR(LSW) = %04x\n", data);
	m_imr &= 0xffff0000 | (~mem_mask);
	m_imr |= data & mem_mask;
	m_isr &= ~(data & mem_mask);

	const uint32_t imr_diff = imr_old ^ m_imr;
	set_interrupt_line(imr_diff, 0);
}

void mc68328_device::iwr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x308
{
	LOGMASKED(LOG_INTS, "iwr_msw_w: IWR(MSW) = %04x\n", data);
	m_iwr &= ~(mem_mask << 16);
	m_iwr |= (data & mem_mask) << 16;
}

void mc68328_device::iwr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x30a
{
	LOGMASKED(LOG_INTS, "iwr_lsw_w: IWR(LSW) = %04x\n", data);
	m_iwr &= 0xffff0000 | (~mem_mask);
	m_iwr |= data & mem_mask;
}

void mc68328_device::isr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x30c
{
	LOGMASKED(LOG_INTS, "isr_msw_w: ISR(MSW) = %04x\n", data);
	// Clear edge-triggered IRQ1
	if ((m_icr & ICR_ET1) == ICR_ET1 && (data & INT_IRQ1_SHIFT) == INT_IRQ1_SHIFT)
	{
		m_isr &= ~INT_IRQ1;
	}

	// Clear edge-triggered IRQ2
	if ((m_icr & ICR_ET2) == ICR_ET2 && (data & INT_IRQ2_SHIFT) == INT_IRQ2_SHIFT)
	{
		m_isr &= ~INT_IRQ2;
	}

	// Clear edge-triggered IRQ3
	if ((m_icr & ICR_ET3) == ICR_ET3 && (data & INT_IRQ3_SHIFT) == INT_IRQ3_SHIFT)
	{
		m_isr &= ~INT_IRQ3;
	}

	// Clear edge-triggered IRQ6
	if ((m_icr & ICR_ET6) == ICR_ET6 && (data & INT_IRQ6_SHIFT) == INT_IRQ6_SHIFT)
	{
		m_isr &= ~INT_IRQ6;
	}

	// Clear edge-triggered IRQ7
	if ((data & INT_IRQ7_SHIFT) == INT_IRQ7_SHIFT)
	{
		m_isr &= ~INT_IRQ7;
	}
}

void mc68328_device::isr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x30e
{
	LOGMASKED(LOG_INTS, "isr_lsw_w: ISR(LSW) = %04x (Ignored)\n", data);
}

void mc68328_device::ipr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x310
{
	LOGMASKED(LOG_INTS, "ipr_msw_w: IPR(MSW) = %04x (Ignored)\n", data);
}

void mc68328_device::ipr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0x312
{
	LOGMASKED(LOG_INTS, "ipr_lsw_w: IPR(LSW) = %04x (Ignored)\n", data);
}

void mc68328_device::padir_w(uint8_t data) // 0x400
{
	LOGMASKED(LOG_GPIO_A, "padir_w: PADIR = %02x\n", data);
	m_padir = data;
}

void mc68328_device::padata_w(uint8_t data) // 0x401
{
	LOGMASKED(LOG_GPIO_A, "padata_w: PADATA = %02x\n", data);
	m_padata = data;
	m_out_port_a_cb(m_padata);
}

void mc68328_device::pasel_w(uint8_t data) // 0x403
{
	LOGMASKED(LOG_GPIO_A, "pasel_w: PASEL = %02x\n", data);
	m_pasel = data;
}

void mc68328_device::pbdir_w(uint8_t data) // 0x408
{
	LOGMASKED(LOG_GPIO_B, "pbdir_w: PBDIR = %02x\n", data);
	m_pbdir = data;
}

void mc68328_device::pbdata_w(uint8_t data) // 0x409
{
	LOGMASKED(LOG_GPIO_B, "pbdata_w: PBDATA = %02x\n", data);
	m_pbdata = data;
	m_out_port_b_cb(m_pbdata);
}

void mc68328_device::pbsel_w(uint8_t data) // 0x40b
{
	LOGMASKED(LOG_GPIO_B, "pbsel_w: PBSEL = %02x\n", data);
	m_pbsel = data;
}

void mc68328_device::pcdir_w(uint8_t data) // 0x410
{
	LOGMASKED(LOG_GPIO_C, "pcdir_w: PCDIR = %02x\n", data);
	m_pcdir = data;
}

void mc68328_device::pcdata_w(uint8_t data) // 0x411
{
	LOGMASKED(LOG_GPIO_C, "pcdata_w: PCDATA = %02x\n", data);
	m_pcdata = data;
	m_out_port_c_cb(m_pcdata);
}

void mc68328_device::pcsel_w(uint8_t data) // 0x413
{
	LOGMASKED(LOG_GPIO_C, "pcsel_w: PCSEL = %02x\n", data);
	m_pcsel = data;
}

void mc68328_device::pddir_w(uint8_t data) // 0x418
{
	LOGMASKED(LOG_GPIO_D, "pddir_w: PDDIR = %02x\n", data);
	m_pddir = data;
}

void mc68328_device::pddata_w(uint8_t data) // 0x419
{
	LOGMASKED(LOG_GPIO_D, "pddata_w: PDDATA = %02x\n", data);
	m_pddata = data;
	m_out_port_d_cb(m_pddata);
}

void mc68328_device::pdpuen_w(uint8_t data) // 0x41a
{
	LOGMASKED(LOG_GPIO_D, "pdpuen_w: PDPUEN = %02x\n", data);
	m_pdpuen = data;
}

void mc68328_device::pdpol_w(uint8_t data) // 0x41c
{
	LOGMASKED(LOG_GPIO_D, "pdpol_w: PDPOL = %02x\n", data);
	m_pdpol = data;
}

void mc68328_device::pdirqen_w(uint8_t data) // 0x41d
{
	LOGMASKED(LOG_GPIO_D, "pdirqen_w: PDIRQEN = %02x\n", data);
	m_pdirqen = data & 0x00ff;
	poll_port_d_interrupts();
}

void mc68328_device::pdirqedge_w(uint8_t data) // 0x41f
{
	LOGMASKED(LOG_GPIO_D, "pdirqedge_w: PDIRQEDGE = %02x\n", data);
	m_pdirqedge = data;
}

void mc68328_device::pedir_w(uint8_t data) // 0x420
{
	LOGMASKED(LOG_GPIO_E, "pedir_w: PEDIR = %02x\n", data);
	m_pedir = data;
}

void mc68328_device::pedata_w(uint8_t data) // 0x421
{
	LOGMASKED(LOG_GPIO_E, "pedata_w: PEDATA = %02x\n", data);
	m_pedata = data;
	m_out_port_e_cb(m_pedata);
}

void mc68328_device::pepuen_w(uint8_t data) // 0x422
{
	LOGMASKED(LOG_GPIO_E, "pepuen_w: PEPUEN = %02x\n", data);
	m_pepuen = data;
}

void mc68328_device::pesel_w(uint8_t data) // 0x423
{
	LOGMASKED(LOG_GPIO_E, "pesel_w: PESEL = %02x\n", data);
	m_pesel = data;
}

void mc68328_device::pfdir_w(uint8_t data) // 0x428
{
	LOGMASKED(LOG_GPIO_F, "pfdir_w: PFDIR = %02x\n", data);
	m_pfdir = data;
}

void mc68328_device::pfdata_w(uint8_t data) // 0x429
{
	LOGMASKED(LOG_GPIO_F, "pfdata_w: PFDATA = %02x\n", data);
	m_pfdata = data;
	m_out_port_f_cb(m_pfdata);
}

void mc68328_device::pfpuen_w(uint8_t data) // 0x42a
{
	LOGMASKED(LOG_GPIO_F, "pfpuen_w: PFPUEN = %02x\n", data);
	m_pfpuen = data;
}

void mc68328_device::pfsel_w(uint8_t data) // 0x42b
{
	LOGMASKED(LOG_GPIO_F, "pfsel_w: PFSEL = %02x\n", data);
	m_pfsel = data;
}

void mc68328_device::pgdir_w(uint8_t data) // 0x430
{
	LOGMASKED(LOG_GPIO_G, "pgdir_w: PGDIR = %02x\n", data);
	m_pgdir = data;
}

void mc68328_device::pgdata_w(uint8_t data) // 0x431
{
	LOGMASKED(LOG_GPIO_G, "pgdata_w: PGDATA = %02x\n", data);
	m_pgdata = data;
	m_out_port_g_cb(m_pgdata);
}

void mc68328_device::pgpuen_w(uint8_t data) // 0x432
{
	LOGMASKED(LOG_GPIO_G, "pgpuen_w: PGPUEN = %02x\n", data);
	m_pgpuen = data;
}

void mc68328_device::pgsel_w(uint8_t data) // 0x433
{
	LOGMASKED(LOG_GPIO_G, "pgsel_w: PGSEL = %02x\n", data);
	m_pgsel = data;
}

void mc68328_device::pjdir_w(uint8_t data) // 0x438
{
	LOGMASKED(LOG_GPIO_J, "pjdir_w: PJDIR = %02x\n", data);
	m_pjdir = data;
}

void mc68328_device::pjdata_w(uint8_t data) // 0x439
{
	LOGMASKED(LOG_GPIO_J, "pjdata_w: PJDATA = %02x\n", data);
	m_pjdata = data;
	m_out_port_j_cb(m_pjdata);
}

void mc68328_device::pjsel_w(uint8_t data) // 0x43b
{
	LOGMASKED(LOG_GPIO_J, "pjsel_w: PJSEL = %02x\n", data);
	m_pjsel = data;
}

void mc68328_device::pkdir_w(uint8_t data) // 0x440
{
	LOGMASKED(LOG_GPIO_K, "pkdir_w: PKDIR = %02x\n", data);
	m_pkdir = data;
}

void mc68328_device::pkdata_w(uint8_t data) // 0x441
{
	LOGMASKED(LOG_GPIO_K, "pkdata_w: PKDATA = %02x\n", data);
	m_pkdata = data;
	m_out_port_k_cb(m_pkdata);
}

void mc68328_device::pkpuen_w(uint8_t data) // 0x442
{
	LOGMASKED(LOG_GPIO_K, "pkpuen_w: PKPUEN = %02x\n", data);
	m_pkpuen = data;
}

void mc68328_device::pksel_w(uint8_t data) // 0x443
{
	LOGMASKED(LOG_GPIO_K, "pksel_w: PKSEL = %02x\n", data);
	m_pksel = data;
}

void mc68328_device::pmdir_w(uint8_t data) // 0x448
{
	LOGMASKED(LOG_GPIO_M, "pmdir_w: PMDIR = %02x\n", data);
	m_pmdir = data;
}

void mc68328_device::pmdata_w(uint8_t data) // 0x449
{
	LOGMASKED(LOG_GPIO_M, "pmdata_w: PMDATA = %02x\n", data);
	m_pmdata = data;
	m_out_port_m_cb(m_pmdata);
}

void mc68328_device::pmpuen_w(uint8_t data) // 0x44a
{
	LOGMASKED(LOG_GPIO_M, "pmpuen_w: PMPUEN = %02x\n", data);
	m_pmpuen = data;
}

void mc68328_device::pmsel_w(uint8_t data) // 0x44b
{
	LOGMASKED(LOG_GPIO_M, "pmsel_w: PMSEL = %02x\n", data);
	m_pmsel = data;
}

void mc68328_device::pwmc_w(uint16_t data) // 0x500
{
	LOGMASKED(LOG_PWM, "pwmc_w: PWMC = %04x\n", data);

	m_pwmc = data;

	if (m_pwmc & PWMC_PWMIRQ)
	{
		set_interrupt_line(INT_PWM, 1);
	}

	m_pwmc &= ~PWMC_LOAD;

	if ((m_pwmc & PWMC_PWMEN) != 0 && m_pwmw != 0 && m_pwmp != 0)
	{
		uint32_t frequency = 32768 * 506;
		uint32_t divisor = 4 << (m_pwmc & PWMC_CLKSEL); // ?? Datasheet says 2 <<, but then we're an octave higher than CoPilot.
		attotime period;
		frequency /= divisor;
		period = attotime::from_hz(frequency) * m_pwmw;
		m_pwm->adjust(period);
		if (m_pwmc & PWMC_IRQEN)
		{
			set_interrupt_line(INT_PWM, 1);
		}
		m_pwmc ^= PWMC_PIN;
	}
	else
	{
		m_pwm->adjust(attotime::never);
	}
}

void mc68328_device::pwmp_w(uint16_t data) // 0x502
{
	LOGMASKED(LOG_PWM, "pwmp_w: PWMP = %04x\n", data);
	m_pwmp = data;
}

void mc68328_device::pwmw_w(uint16_t data) // 0x504
{
	LOGMASKED(LOG_PWM, "pwmw_w: PWMW = %04x\n", data);
	m_pwmw = data;
}

void mc68328_device::pwmcnt_w(uint16_t data) // 0x506
{
	LOGMASKED(LOG_PWM, "pwmcnt_w: PWMCNT = %04x\n", data);
	m_pwmcnt = 0;
}

template <int Timer>
void mc68328_device::tctl_w(uint16_t data) // 0x600, 0x60c
{
	LOGMASKED(LOG_TIMERS, "tctl_w<%d>: TCTL%d = %04x\n", Timer, Timer + 1, data);
	const uint16_t temp = m_tctl[Timer];
	m_tctl[Timer] = data;
	if ((temp & TCTL_TEN) == (m_tctl[Timer] & TCTL_TEN))
	{
		maybe_start_timer<Timer>(0);
	}
	else if ((temp & TCTL_TEN) != TCTL_TEN_ENABLE && (m_tctl[Timer] & TCTL_TEN) == TCTL_TEN_ENABLE)
	{
		maybe_start_timer<Timer>(1);
	}
}

template <int Timer>
void mc68328_device::tprer_w(uint16_t data) // 0x602, 0x60e
{
	LOGMASKED(LOG_TIMERS, "tprer_w<%d>: TPRER%d = %04x\n", Timer, Timer + 1, data);
	m_tprer[Timer] = data;
	maybe_start_timer<Timer>(0);
}

template <int Timer>
void mc68328_device::tcmp_w(uint16_t data) // 0x604, 0x610
{
	LOGMASKED(LOG_TIMERS, "tcmp_w<%d>: TCMP%d = %04x\n", Timer, Timer + 1, data);
	m_tcmp[Timer] = data;
	maybe_start_timer<Timer>(0);
}

template <int Timer>
void mc68328_device::tcr_w(uint16_t data) // 0x606, 0x612
{
	LOGMASKED(LOG_TIMERS, "tcr_w<%d>: TCR%d = %04x (Ignored)\n", Timer, Timer + 1, data);
}

template <int Timer>
void mc68328_device::tcn_w(uint16_t data) // 0x608, 0x614
{
	LOGMASKED(LOG_TIMERS, "tcn_w<%d>: TCN%d = %04x (Ignored)\n", Timer, Timer + 1, data);
}

template <int Timer>
void mc68328_device::tstat_w(uint16_t data) // 0x60a, 0x616
{
	LOGMASKED(LOG_TSTAT, "tstat_w<%d>: TSTAT%d = %04x\n", Timer, Timer + 1, data);
	m_tstat[Timer] &= ~m_tclear[Timer];
	if (!(m_tstat[Timer] & TSTAT_COMP))
	{
		set_interrupt_line(Timer ? INT_TIMER2 : INT_TIMER1, 0);
	}
}

void mc68328_device::wctlr_w(uint16_t data) // 0x618
{
	LOGMASKED(LOG_WATCHDOG, "wctlr_w: WCTLR = %04x\n", data);
	m_wctlr = data;
}

void mc68328_device::wcmpr_w(uint16_t data) // 0x61a
{
	LOGMASKED(LOG_WATCHDOG, "wcmpr_w: WCMPR = %04x\n", data);
	m_wcmpr = data;
}

void mc68328_device::wcn_w(uint16_t data) // 0x61c
{
	LOGMASKED(LOG_WATCHDOG, "wcn_w: WCN = %04x (Ignored)\n", data);
}

void mc68328_device::spisr_w(uint16_t data) // 0x700
{
	LOGMASKED(LOG_SPIS, "spisr_w: SPISR = %04x\n", data);
	m_spisr = data;
}

void mc68328_device::spimdata_w(uint16_t data) // 0x800
{
	LOGMASKED(LOG_SPIM, "spimdata_w: SPIMDATA = %04x\n", data);
	m_spimdata = data;
	m_out_spim_cb(data, 0xffff);
}

void mc68328_device::spimcont_w(uint16_t data) // 0x802
{
	LOGMASKED(LOG_SPIM, "spimcont_w: SPIMCONT = %04x\n", data);
	LOGMASKED(LOG_SPIM, "            Count = %d\n", data & SPIM_CLOCK_COUNT);
	LOGMASKED(LOG_SPIM, "            Polarity = %s\n", (data & SPIM_POL) ? "Inverted" : "Active-high");
	LOGMASKED(LOG_SPIM, "            Phase = %s\n", (data & SPIM_PHA) ? "Opposite" : "Normal");
	LOGMASKED(LOG_SPIM, "            IRQ Enable = %s\n", (data & SPIM_IRQEN) ? "Enable" : "Disable");
	LOGMASKED(LOG_SPIM, "            IRQ Pending = %s\n", (data & SPIM_SPIMIRQ) ? "Yes" : "No");
	LOGMASKED(LOG_SPIM, "            Exchange = %s\n", (data & SPIM_XCH) ? "Initiate" : "Idle");
	LOGMASKED(LOG_SPIM, "            SPIM Enable = %s\n", (data & SPIM_SPMEN) ? "Enable" : "Disable");
	LOGMASKED(LOG_SPIM, "            Data Rate = Divide By %d\n", 1 << ((((data & SPIM_RATE) >> 13) & 0x0007) + 2) );
	m_spimcont = data;
	// HACK: We should probably emulate the ADS7843 A/D device properly.
	if (data & SPIM_XCH)
	{
		m_spimcont &= ~SPIM_XCH;
		if (!m_spim_xch_trigger_cb.isnull())
		{
			m_spim_xch_trigger_cb(0);
		}
		if (data & SPIM_IRQEN)
		{
			m_spimcont |= SPIM_SPIMIRQ;
			LOGMASKED(LOG_SPIM, "Triggering SPIM Interrupt\n" );
			set_interrupt_line(INT_SPIM, 1);
		}
	}
	if (!(data & SPIM_IRQEN))
	{
		set_interrupt_line(INT_SPIM, 0);
	}
}

void mc68328_device::ustcnt_w(uint16_t data) // 0x900
{
	LOGMASKED(LOG_UART, "ustcnt_w: USTCNT = %04x\n", data);
	m_ustcnt = data;
}

void mc68328_device::ubaud_w(uint16_t data) // 0x902
{
	LOGMASKED(LOG_UART, "ubaud_w: UBAUD = %04x\n", data);
	m_ubaud = data;
}

void mc68328_device::urx_w(uint16_t data) // 0x904
{
	LOGMASKED(LOG_UART, "urx_w: URX = %04x (Not Yet Implemented)\n", data);
}

void mc68328_device::utx_w(uint16_t data) // 0x906
{
	LOGMASKED(LOG_UART, "utx_w: UTX = %04x (Not Yet Implemented)\n", data);
}

void mc68328_device::umisc_w(uint16_t data) // 0x908
{
	LOGMASKED(LOG_UART, "umisc_w: UMISC = %04x (Not Yet Implemented)\n", data);
	m_umisc = data;
}

void mc68328_device::lssa_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xa00
{
	LOGMASKED(LOG_LCD, "lssa_msw_w: LSSA(MSW) = %04x\n", data);
	m_lssa &= ~(mem_mask << 16);
	m_lssa |= (data & mem_mask) << 16;
	LOGMASKED(LOG_LCD, "            Address: %08x\n", m_lssa);
}

void mc68328_device::lssa_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xa02
{
	LOGMASKED(LOG_LCD, "lssa_lsw_w: LSSA(LSW) = %04x\n", data);
	m_lssa &= 0xffff0000 | (~mem_mask);
	m_lssa |= data & mem_mask;
	LOGMASKED(LOG_LCD, "            Address: %08x\n", m_lssa);
}

void mc68328_device::lvpw_w(uint8_t data) // 0xa05
{
	LOGMASKED(LOG_LCD, "lvpw_w: LVPW = %02x\n", data);
	m_lvpw = data;
	LOGMASKED(LOG_LCD, "        Page Width: %d\n", (m_lvpw + 1) * ((m_lpicf & 0x01) ? 8 : 16));
}

void mc68328_device::lxmax_w(uint16_t data) // 0xa08
{
	LOGMASKED(LOG_LCD, "lxmax_w: LXMAX = %04x\n", data);
	m_lxmax = data;
	LOGMASKED(LOG_LCD, "         Width: %d\n", (data & 0x03ff) + 1);
}

void mc68328_device::lymax_w(uint16_t data) // 0xa0a
{
	LOGMASKED(LOG_LCD, "lymax_w: LYMAX = %04x\n", data);
	m_lxmax = data;
	LOGMASKED(LOG_LCD, "         Height: %d\n", (data & 0x03ff) + 1);
}

void mc68328_device::lcxp_w(uint16_t data) // 0xa18
{
	LOGMASKED(LOG_LCD, "lcxp_w: LCXP = %04x\n", data);
	m_lcxp = data;
	LOGMASKED(LOG_LCD, "        X Position: %d\n", data & 0x03ff);
	switch (m_lcxp >> 14)
	{
		case 0:
			LOGMASKED(LOG_LCD, "        Cursor Control: Transparent\n");
			break;

		case 1:
			LOGMASKED(LOG_LCD, "        Cursor Control: Black\n");
			break;

		case 2:
			LOGMASKED(LOG_LCD, "        Cursor Control: Reverse\n");
			break;

		case 3:
			LOGMASKED(LOG_LCD, "        Cursor Control: Invalid\n");
			break;
	}
}

void mc68328_device::lcyp_w(uint16_t data) // 0xa1a
{
	LOGMASKED(LOG_LCD, "lcyp_w: LCYP = %04x\n", data);
	m_lcyp = data;
	LOGMASKED(LOG_LCD, "        Y Position: %d\n", data & 0x01ff);
}

void mc68328_device::lcwch_w(uint16_t data) // 0xa1c
{
	LOGMASKED(LOG_LCD, "lcwch_w: LCWCH = %04x\n", data);
	m_lcwch = data;
	LOGMASKED(LOG_LCD, "         Width:  %d\n", (data >> 8) & 0x1f);
	LOGMASKED(LOG_LCD, "         Height: %d\n", data & 0x1f);
}

void mc68328_device::lblkc_w(uint8_t data) // 0xa1f
{
	LOGMASKED(LOG_LCD, "lblkc_w: LBLKC = %02x\n", data);
	m_lblkc = data;
	LOGMASKED(LOG_LCD, "         Blink Enable:  %d\n", m_lblkc >> 7);
	LOGMASKED(LOG_LCD, "         Blink Divisor: %d\n", m_lblkc & 0x7f);
}

void mc68328_device::lpicf_w(uint8_t data) // 0xa20
{
	LOGMASKED(LOG_LCD, "lpicf_w: LPICF = %02x\n", data);
	m_lpicf = data;
	switch((m_lpicf >> 1) & 0x03)
	{
		case 0:
			LOGMASKED(LOG_LCD, "         Bus Size: 1-bit\n");
			break;

		case 1:
			LOGMASKED(LOG_LCD, "         Bus Size: 2-bit\n");
			break;

		case 2:
			LOGMASKED(LOG_LCD, "         Bus Size: 4-bit\n");
			break;

		case 3:
			LOGMASKED(LOG_LCD, "         Bus Size: unused\n");
			break;
	}
	LOGMASKED(LOG_LCD, "         Gray scale enable: %d\n", m_lpicf & 0x01);
}

void mc68328_device::lpolcf_w(uint8_t data) // 0xa21
{
	LOGMASKED(LOG_LCD, "lpolcf_w: LPOLCF = %02x\n", data);
	m_lpolcf = data;
	LOGMASKED(LOG_LCD, "          LCD Shift Clock Polarity: %s\n", (m_lpicf & 0x08) ? "Active positive edge of LCLK" : "Active negative edge of LCLK");
	LOGMASKED(LOG_LCD, "          First-line marker polarity: %s\n", (m_lpicf & 0x04) ? "Active Low" : "Active High");
	LOGMASKED(LOG_LCD, "          Line-pulse polarity: %s\n", (m_lpicf & 0x02) ? "Active Low" : "Active High");
	LOGMASKED(LOG_LCD, "          Pixel polarity: %s\n", (m_lpicf & 0x01) ? "Active Low" : "Active High");
}

void mc68328_device::lacdrc_w(uint8_t data) // 0xa23
{
	LOGMASKED(LOG_LCD, "lacdrc_w: LACDRC = %02x\n", data);
	m_lacdrc = data;
}

void mc68328_device::lpxcd_w(uint8_t data) // 0xa25
{
	LOGMASKED(LOG_LCD, "lpxcd_w: LPXCD = %02x\n", data);
	m_lpxcd = data;
	LOGMASKED(LOG_LCD, "         Clock Divisor: %d\n", m_lpxcd + 1);
}

void mc68328_device::lckcon_w(uint8_t data) // 0xa27
{
	LOGMASKED(LOG_LCD, "lckcon_w: LCKCON = %02x\n", data);
	m_lckcon = data;
	LOGMASKED(LOG_LCD, "          LCDC Enable: %d\n", (m_lckcon >> 7) & 0x01);
	LOGMASKED(LOG_LCD, "          DMA Burst Length: %d\n", ((m_lckcon >> 6) & 0x01) ? 16 : 8);
	LOGMASKED(LOG_LCD, "          DMA Bursting Clock Control: %d\n", ((m_lckcon >> 4) & 0x03) + 1);
	LOGMASKED(LOG_LCD, "          Bus Width: %d\n", ((m_lckcon >> 1) & 0x01) ? 8 : 16);
	LOGMASKED(LOG_LCD, "          Pixel Clock Divider Source: %s\n", (m_lckcon & 0x01) ? "PIX" : "SYS");
}

void mc68328_device::llbar_w(uint8_t data) // 0xa29
{
	LOGMASKED(LOG_LCD, "llbar_w: LLBAR = %02x\n", data);
	m_llbar = data;
	LOGMASKED(LOG_LCD, "         Address: %d\n", (m_llbar & 0x7f) * ((m_lpicf & 0x01) ? 8 : 16));
}

void mc68328_device::lotcr_w(uint8_t data) // 0xa2b
{
	LOGMASKED(LOG_LCD, "lotcr_w: LOTCR = %02x (Ignored)\n", data);
}

void mc68328_device::lposr_w(uint8_t data) // 0xa2d
{
	LOGMASKED(LOG_LCD, "lposr_w: LPOSR = %02x\n", data);
	m_lposr = data;
	LOGMASKED(LOG_LCD, "         Byte Offset: %d\n", (m_lposr >> 3) & 0x01);
	LOGMASKED(LOG_LCD, "         Pixel Offset: %d\n", m_lposr & 0x07);
}

void mc68328_device::lfrcm_w(uint8_t data) // 0xa31
{
	LOGMASKED(LOG_LCD, "lfrcm_w: LFRCM = %02x\n", data);
	m_lfrcm = data;
	LOGMASKED(LOG_LCD, "         X Modulation: %d\n", (m_lfrcm >> 4) & 0x0f);
	LOGMASKED(LOG_LCD, "         Y Modulation: %d\n", m_lfrcm & 0x0f);
}

void mc68328_device::lgpmr_w(uint8_t data) // 0xa32
{
	LOGMASKED(LOG_LCD, "lgpmr_w: LGPMR = %04x\n", data);
	m_lgpmr = data;
	LOGMASKED(LOG_LCD, "         Palette 0: %d\n", (m_lgpmr >>  8) & 0x07);
	LOGMASKED(LOG_LCD, "         Palette 1: %d\n", (m_lgpmr >> 12) & 0x07);
	LOGMASKED(LOG_LCD, "         Palette 2: %d\n", (m_lgpmr >>  0) & 0x07);
	LOGMASKED(LOG_LCD, "         Palette 3: %d\n", (m_lgpmr >>  4) & 0x07);
}

void mc68328_device::hmsr_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb00
{
	LOGMASKED(LOG_RTC, "hmsr_msw_w: HMSR(MSW) = %04x\n", data);
	m_hmsr &= ~(mem_mask << 16);
	m_hmsr |= (data & mem_mask) << 16;
	m_hmsr &= 0x1f3f003f;
}

void mc68328_device::hmsr_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb02
{
	LOGMASKED(LOG_RTC, "hmsr_lsw_w: HMSR(LSW) = %04x\n", data);
	m_hmsr &= 0xffff0000 | (~mem_mask);
	m_hmsr |= data & mem_mask;
	m_hmsr &= 0x1f3f003f;
}

void mc68328_device::alarm_msw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb04
{
	LOGMASKED(LOG_RTC, "alarm_msw_w: ALARM(MSW) = %04x\n", data);
	m_alarm &= ~(mem_mask << 16);
	m_alarm |= (data & mem_mask) << 16;
	m_alarm &= 0x1f3f003f;
}

void mc68328_device::alarm_lsw_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb06
{
	LOGMASKED(LOG_RTC, "alarm_lsw_w: ALARM(LSW) = %04x\n", data);
	m_alarm &= 0xffff0000 | (~mem_mask);
	m_alarm |= data & mem_mask;
	m_alarm &= 0x1f3f003f;
}

void mc68328_device::rtcctl_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb0c
{
	LOGMASKED(LOG_RTC, "rtcctl_w: RTCCTL = %04x\n", data);
	m_rtcctl = data & 0x00a0;
}

void mc68328_device::rtcisr_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb0e
{
	LOGMASKED(LOG_RTC, "rtcisr_w: RTCISR = %04x\n", data);
	m_rtcisr &= ~data;
	if (m_rtcisr == 0)
	{
		set_interrupt_line(INT_RTC, 0);
	}
}

void mc68328_device::rtcienr_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb10
{
	LOGMASKED(LOG_RTC, "rtcienr_w: RTCIENR = %04x\n", data);
	m_rtcienr = data & 0x001f;
}

void mc68328_device::stpwtch_w(offs_t offset, uint16_t data, uint16_t mem_mask) // 0xb12
{
	LOGMASKED(LOG_RTC, "stpwtch_w: STPWTCH = %04x\n", data);
	m_stpwtch = data & 0x003f;
}


uint8_t mc68328_device::scr_r() // 0x000
{
	LOGMASKED(LOG_SCR, "scr_r: SCR: %02x\n", m_scr);
	return m_scr;
}

uint16_t mc68328_device::grpbasea_r() // 0x100
{
	LOGMASKED(LOG_CS_GRP, "grpbasea_r: GRPBASEA: %04x\n", m_grpbasea);
	return m_grpbasea;
}

uint16_t mc68328_device::grpbaseb_r() // 0x102
{
	LOGMASKED(LOG_CS_GRP, "grpbaseb_r: GRPBASEB: %04x\n", m_grpbaseb);
	return m_grpbaseb;
}

uint16_t mc68328_device::grpbasec_r() // 0x104
{
	LOGMASKED(LOG_CS_GRP, "grpbasec_r: GRPBASEC: %04x\n", m_grpbasec);
	return m_grpbasec;
}

uint16_t mc68328_device::grpbased_r() // 0x106
{
	LOGMASKED(LOG_CS_GRP, "grpbased_r: GRPBASED: %04x\n", m_grpbased);
	return m_grpbased;
}

uint16_t mc68328_device::grpmaska_r() // 0x108
{
	LOGMASKED(LOG_CS_GRP, "grpmaska_r: GRPMASKA: %04x\n", m_grpmaska);
	return m_grpmaska;
}

uint16_t mc68328_device::grpmaskb_r() // 0x10a
{
	LOGMASKED(LOG_CS_GRP, "grpmaskb_r: GRPMASKB: %04x\n", m_grpmaskb);
	return m_grpmaskb;
}

uint16_t mc68328_device::grpmaskc_r() // 0x10c
{
	LOGMASKED(LOG_CS_GRP, "grpmaskc_r: GRPMASKC: %04x\n", m_grpmaskc);
	return m_grpmaskc;
}

uint16_t mc68328_device::grpmaskd_r() // 0x10e
{
	LOGMASKED(LOG_CS_GRP, "grpmaskd_r: GRPMASKD: %04x\n", m_grpmaskd);
	return m_grpmaskd;
}

template<int ChipSelect>
uint16_t mc68328_device::csa_msw_r() // 0x110, 0x120, 0x130, 0x140
{
	LOGMASKED(LOG_CS_SEL, "csa_msw_r: CSA%d(MSW): %04x\n", ChipSelect, (uint16_t)(m_csa[ChipSelect] >> 16));
	return (uint16_t)(m_csa[ChipSelect] >> 16);
}

template<int ChipSelect>
uint16_t mc68328_device::csa_lsw_r() // 0x112, 0x122, 0x132, 0x142
{
	LOGMASKED(LOG_CS_SEL, "csa_lsw_r: CSA%d(LSW): %04x\n", ChipSelect, (uint16_t)m_csa[ChipSelect]);
	return (uint16_t)m_csa[ChipSelect];
}

template<int ChipSelect>
uint16_t mc68328_device::csb_msw_r() // 0x114, 0x124, 0x134, 0x144
{
	LOGMASKED(LOG_CS_SEL, "csb_msw_r: CSB%d(MSW): %04x\n", ChipSelect, (uint16_t)(m_csb[ChipSelect] >> 16));
	return (uint16_t)(m_csb[ChipSelect] >> 16);
}

template<int ChipSelect>
uint16_t mc68328_device::csb_lsw_r() // 0x116, 0x126, 0x136, 0x146
{
	LOGMASKED(LOG_CS_SEL, "csb_lsw_r: CSB%d(LSW): %04x\n", ChipSelect, (uint16_t)m_csb[ChipSelect]);
	return (uint16_t)m_csb[ChipSelect];
}

template<int ChipSelect>
uint16_t mc68328_device::csc_msw_r() // 0x118, 0x128, 0x138, 0x148
{
	LOGMASKED(LOG_CS_SEL, "csc_msw_r: CSC%d(MSW): %04x\n", ChipSelect, (uint16_t)(m_csc[ChipSelect] >> 16));
	return (uint16_t)(m_csc[ChipSelect] >> 16);
}

template<int ChipSelect>
uint16_t mc68328_device::csc_lsw_r() // 0x11a, 0x12a, 0x13a, 0x14a
{
	LOGMASKED(LOG_CS_SEL, "csc_lsw_r: CSC%d(LSW): %04x\n", ChipSelect, (uint16_t)m_csc[ChipSelect]);
	return (uint16_t)m_csc[ChipSelect];
}

template<int ChipSelect>
uint16_t mc68328_device::csd_msw_r() // 0x11c, 0x12c, 0x13c, 0x14c
{
	LOGMASKED(LOG_CS_SEL, "csd_msw_r: CSD%d(MSW): %04x\n", ChipSelect, (uint16_t)(m_csd[ChipSelect] >> 16));
	return (uint16_t)(m_csd[ChipSelect] >> 16);
}

template<int ChipSelect>
uint16_t mc68328_device::csd_lsw_r() // 0x11e, 0x12e, 0x13e, 0x14e
{
	LOGMASKED(LOG_CS_SEL, "csd_lsw_r: CSD%d(LSW): %04x\n", ChipSelect, (uint16_t)m_csd[ChipSelect]);
	return (uint16_t)m_csd[ChipSelect];
}

uint16_t mc68328_device::pllcr_r() // 0x200
{
	LOGMASKED(LOG_PLL, "pllcr_r: PLLCR: %04x\n", m_pllcr);
	return m_pllcr;
}

uint16_t mc68328_device::pllfsr_r() // 0x202
{
	LOGMASKED(LOG_PLL, "pllfsr_r: PLLFSR: %04x\n", m_pllfsr);
	m_pllfsr ^= 0x8000;
	return m_pllfsr;
}

uint8_t mc68328_device::pctlr_r() // 0x207
{
	LOGMASKED(LOG_PLL, "pctlr_r: PCTLR: %02x\n", m_pctlr);
	return m_pctlr;
}

uint8_t mc68328_device::ivr_r() // 0x300
{
	LOGMASKED(LOG_INTS, "ivr_r: IVR: %02x\n", m_ivr);
	return m_ivr;
}

uint16_t mc68328_device::icr_r() // 0x302
{
	LOGMASKED(LOG_INTS, "icr_r: ICR: %04x\n", m_icr);
	return m_icr;
}

uint16_t mc68328_device::imr_msw_r() // 0x304
{
	LOGMASKED(LOG_INTS, "imr_msw_r: IMR(MSW): %04x\n", (uint16_t)(m_imr >> 16));
	return (uint16_t)(m_imr >> 16);
}

uint16_t mc68328_device::imr_lsw_r() // 0x306
{
	LOGMASKED(LOG_INTS, "imr_lsw_r: IMR(LSW): %04x\n", (uint16_t)m_imr);
	return (uint16_t)m_imr;
}

uint16_t mc68328_device::iwr_msw_r() // 0x308
{
	LOGMASKED(LOG_INTS, "iwr_msw_r: IWR(MSW): %04x\n", (uint16_t)(m_iwr >> 16));
	return (uint16_t)(m_iwr >> 16);
}

uint16_t mc68328_device::iwr_lsw_r() // 0x30a
{
	LOGMASKED(LOG_INTS, "iwr_lsw_r: IWR(LSW): %04x\n", (uint16_t)m_iwr);
	return (uint16_t)m_iwr;
}

uint16_t mc68328_device::isr_msw_r() // 0x30c
{
	LOGMASKED(LOG_INTS, "isr_msw_r: ISR(MSW): %04x\n", (uint16_t)(m_isr >> 16));
	return (uint16_t)(m_isr >> 16);
}

uint16_t mc68328_device::isr_lsw_r() // 0x30e
{
	LOGMASKED(LOG_INTS, "isr_lsw_r: ISR(LSW): %04x\n", (uint16_t)m_isr);
	return (uint16_t)m_isr;
}

uint16_t mc68328_device::ipr_msw_r() // 0x310
{
	LOGMASKED(LOG_INTS, "ipr_msw_r: IPR(MSW): %04x\n", (uint16_t)(m_ipr >> 16));
	return (uint16_t)(m_ipr >> 16);
}

uint16_t mc68328_device::ipr_lsw_r() // 0x312
{
	LOGMASKED(LOG_INTS, "ipr_lsw_r: IPR(LSW): %04x\n", (uint16_t)m_ipr);
	return (uint16_t)m_ipr;
}

uint8_t mc68328_device::padir_r() // 0x400
{
	LOGMASKED(LOG_GPIO_A, "mc68328_r: PADIR: %02x\n", m_padir);
	return m_padir;
}

uint8_t mc68328_device::padata_r() // 0x401
{
	LOGMASKED(LOG_GPIO_A, "padata_r: PADATA: %02x\n", m_padata);
	if (!m_in_port_a_cb.isnull())
	{
		return m_in_port_a_cb(0);
	}
	else
	{
		return m_padata;
	}
}

uint8_t mc68328_device::pasel_r() // 0x403
{
	LOGMASKED(LOG_GPIO_A, "mc68328_r: PASEL: %02x\n", m_pasel);
	return m_pasel;
}

uint8_t mc68328_device::pbdir_r() // 0x408
{
	LOGMASKED(LOG_GPIO_B, "pbdir_r: PBDIR: %02x\n", m_pbdir);
	return m_pbdir;
}

uint8_t mc68328_device::pbdata_r() // 0x409
{
	LOGMASKED(LOG_GPIO_B, "pbdata_r: PBDATA: %02x\n", m_pbdata);
	if (!m_in_port_b_cb.isnull())
	{
		return m_in_port_b_cb(0);
	}
	else
	{
		return m_pbdata;
	}
}

uint8_t mc68328_device::pbsel_r() // 0x40b
{
	LOGMASKED(LOG_GPIO_B, "pbsel_r: PBSEL: %02x\n", m_pbsel);
	return m_pbsel;
}

uint8_t mc68328_device::pcdir_r() // 0x410
{
	LOGMASKED(LOG_GPIO_C, "pcdir_r: PCDIR: %02x\n", m_pcdir);
	return m_pcdir;
}

uint8_t mc68328_device::pcdata_r() // 0x411
{
	LOGMASKED(LOG_GPIO_C, "pcdata_r: PCDATA: %02x\n", m_pcdata);
	if (!m_in_port_c_cb.isnull())
	{
		return m_in_port_c_cb(0);
	}
	else
	{
		return m_pcdata;
	}
}

uint8_t mc68328_device::pcsel_r() // 0x413
{
	LOGMASKED(LOG_GPIO_C, "pcsel_r: PCSEL: %02x\n", m_pcsel);
	return m_pcsel;
}

uint8_t mc68328_device::pddir_r() // 0x418
{
	LOGMASKED(LOG_GPIO_D, "pddir_r: PDDIR: %02x\n", m_pddir);
	return m_pddir;
}

uint8_t mc68328_device::pddata_r() // 0x419
{
	LOGMASKED(LOG_GPIO_D, "pddata_r: PDDATA: %02x\n", m_pddata);
	if (!m_in_port_d_cb.isnull())
	{
		return m_in_port_d_cb(0);
	}
	return m_pddata;
}

uint8_t mc68328_device::pdpuen_r() // 0x41a
{
	LOGMASKED(LOG_GPIO_D, "pdpuen_r: PDPUEN: %02x\n", m_pdpuen);
	return m_pdpuen;
}

uint8_t mc68328_device::pdpol_r() // 0x41c
{
	LOGMASKED(LOG_GPIO_D, "pdpol_r: PDPOL: %02x\n", m_pdpol);
	return m_pdpol;
}

uint8_t mc68328_device::pdirqen_r() // 0x41d
{
	LOGMASKED(LOG_GPIO_D, "pdirqen_r: PDIRQEN: %02x\n", m_pdirqen);
	return m_pdirqen;
}

uint8_t mc68328_device::pdirqedge_r() // 0x41f
{
	LOGMASKED(LOG_GPIO_D, "pdirqedge_r: PDIRQEDGE: %02x\n", m_pdirqedge);
	return m_pdirqedge;
}

uint8_t mc68328_device::pedir_r() // 0x420
{
	LOGMASKED(LOG_GPIO_E, "pedir_r: PEDIR: %02x\n", m_pedir);
	return m_pedir;
}

uint8_t mc68328_device::pedata_r() // 0x421
{
	LOGMASKED(LOG_GPIO_E, "pedata_r: PEDATA: %02x\n", m_pedata);
	if (!m_in_port_e_cb.isnull())
	{
		return m_in_port_e_cb(0);
	}
	return m_pedata;
}

uint8_t mc68328_device::pepuen_r() // 0x422
{
	LOGMASKED(LOG_GPIO_E, "pepuen_r: PEPUEN: %02x\n", m_pepuen);
	return m_pepuen;
}

uint8_t mc68328_device::pesel_r() // 0x423
{
	LOGMASKED(LOG_GPIO_E, "pesel_r: PESEL: %02x\n", m_pesel);
	return m_pesel;
}

uint8_t mc68328_device::pfdir_r() // 0x428
{
	LOGMASKED(LOG_GPIO_F, "pfdir_r: PFDIR: %02x\n", m_pfdir);
	return m_pfdir;
}

uint8_t mc68328_device::pfdata_r() // 0x429
{
	LOGMASKED(LOG_GPIO_F, "pfdata_r: PFDATA: %02x\n", m_pfdata);
	if (!m_in_port_f_cb.isnull())
	{
		return m_in_port_f_cb(0);
	}
	return m_pfdata;
}

uint8_t mc68328_device::pfpuen_r() // 0x42a
{
	LOGMASKED(LOG_GPIO_F, "pfpuen_r: PFPUEN: %02x\n", m_pfpuen);
	return m_pfpuen;
}

uint8_t mc68328_device::pfsel_r() // 0x42b
{
	LOGMASKED(LOG_GPIO_F, "pfsel_r: PFSEL: %02x\n", m_pfsel);
	return m_pfsel;
}

uint8_t mc68328_device::pgdir_r() // 0x430
{
	LOGMASKED(LOG_GPIO_G, "pgdir_r: PGDIR: %02x\n", m_pgdir);
	return m_pgdir;
}

uint8_t mc68328_device::pgdata_r() // 0x431
{
	LOGMASKED(LOG_GPIO_G, "pgdata_r: PGDATA: %02x\n", m_pgdata);
	if (!m_in_port_g_cb.isnull())
	{
		return m_in_port_g_cb(0);
	}
	return m_pgdata;
}

uint8_t mc68328_device::pgpuen_r() // 0x432
{
	LOGMASKED(LOG_GPIO_G, "pgpuen_r: PGPUEN: %02x\n", m_pgpuen);
	return m_pgpuen;
}

uint8_t mc68328_device::pgsel_r() // 0x433
{
	LOGMASKED(LOG_GPIO_G, "pgsel_r: PGSEL: %02x\n", m_pgsel);
	return m_pgsel;
}

uint8_t mc68328_device::pjdir_r() // 0x438
{
	LOGMASKED(LOG_GPIO_J, "pjdir_r: PJDIR: %02x\n", m_pjdir);
	return m_pjdir;
}

uint8_t mc68328_device::pjdata_r() // 0x439
{
	LOGMASKED(LOG_GPIO_J, "pjdata_r: PJDATA: %02x\n", m_pjdata);
	if (!m_in_port_j_cb.isnull())
	{
		return m_in_port_j_cb(0);
	}
	return m_pjdata;
}

uint8_t mc68328_device::pjsel_r() // 0x43b
{
	LOGMASKED(LOG_GPIO_J, "pjsel_r: PJSEL: %02x\n", m_pjsel);
	return m_pjsel;
}

uint8_t mc68328_device::pkdir_r() // 0x440
{
	LOGMASKED(LOG_GPIO_K, "pkdir_r: PKDIR: %02x\n", m_pkdir);
	return m_pkdir;
}

uint8_t mc68328_device::pkdata_r() // 0x441
{
	LOGMASKED(LOG_GPIO_K, "pkdata_r: PKDATA: %02x\n", m_pkdata);
	if (!m_in_port_k_cb.isnull())
	{
		return m_in_port_k_cb(0);
	}
	return m_pkdata;
}

uint8_t mc68328_device::pkpuen_r() // 0x442
{
	LOGMASKED(LOG_GPIO_K, "pkpuen_r: PKPUEN: %02x\n", m_pkpuen);
	return m_pkpuen;
}

uint8_t mc68328_device::pksel_r() // 0x443
{
	LOGMASKED(LOG_GPIO_K, "pksel_r: PKSEL: %02x\n", m_pksel);
	return m_pksel;
}

uint8_t mc68328_device::pmdir_r() // 0x448
{
	LOGMASKED(LOG_GPIO_M, "pmdir_r: PMDIR: %02x\n", m_pmdir);
	return m_pmdir;
}

uint8_t mc68328_device::pmdata_r() // 0x449
{
	LOGMASKED(LOG_GPIO_M, "pmdata_r: PMDATA: %02x\n", m_pmdata);
	if (!m_in_port_m_cb.isnull())
	{
		return m_in_port_m_cb(0);
	}
	return m_pmdata;
}

uint8_t mc68328_device::pmpuen_r() // 0x44a
{
	LOGMASKED(LOG_GPIO_M, "pmpuen_r: PMPUEN: %02x\n", m_pmpuen);
	return m_pmpuen;
}

uint8_t mc68328_device::pmsel_r() // 0x44b
{
	LOGMASKED(LOG_GPIO_M, "pmsel_r: PMSEL: %02x\n", m_pmsel);
	return m_pmsel;
}

uint16_t mc68328_device::pwmc_r() // 0x500
{
	const uint16_t data = m_pwmc;
	LOGMASKED(LOG_PWM, "pwmc_r: PWMC: %04x\n", data);
	if (m_pwmc & PWMC_PWMIRQ)
	{
		m_pwmc &= ~PWMC_PWMIRQ;
		set_interrupt_line(INT_PWM, 0);
	}
	return data;
}

uint16_t mc68328_device::pwmp_r() // 0x502
{
	LOGMASKED(LOG_PWM, "pwmp_r: PWMP: %04x\n", m_pwmp);
	return m_pwmp;
}

uint16_t mc68328_device::pwmw_r() // 0x504
{
	LOGMASKED(LOG_PWM, "pwmw_r: PWMW: %04x\n", m_pwmw);
	return m_pwmw;
}

uint16_t mc68328_device::pwmcnt_r() // 0x506
{
	LOGMASKED(LOG_PWM, "pwmcnt_r: PWMCNT: %04x\n", m_pwmcnt);
	return m_pwmcnt;
}

template <int Timer>
uint16_t mc68328_device::tctl_r() // 0x600, 0x60c
{
	LOGMASKED(LOG_TIMERS, "tctl_r: TCTL%d: %04x\n", Timer + 1, m_tctl[Timer]);
	return m_tctl[Timer];
}

template <int Timer>
uint16_t mc68328_device::tprer_r() // 0x602, 0x60e
{
	LOGMASKED(LOG_TIMERS, "tprer_r: TPRER%d: %04x\n", Timer + 1, m_tprer[Timer]);
	return m_tprer[Timer];
}

template <int Timer>
uint16_t mc68328_device::tcmp_r() // 0x604, 0x610
{
	LOGMASKED(LOG_TIMERS, "tcmp_r: TCMP%d: %04x\n", Timer + 1, m_tcmp[Timer]);
	return m_tcmp[Timer];
}

template <int Timer>
uint16_t mc68328_device::tcr_r() // 0x606, 0x612
{
	LOGMASKED(LOG_TIMERS, "tcr_r: TCR%d: %04x\n", Timer + 1, m_tcr[Timer]);
	return m_tcr[Timer];
}

template <int Timer>
uint16_t mc68328_device::tcn_r() // 0x608, 0x614
{
	LOGMASKED(LOG_TIMERS, "tcn_r: TCN%d: %04x\n", Timer + 1, m_tcn[Timer]);
	return m_tcn[Timer];
}

template <int Timer>
uint16_t mc68328_device::tstat_r() // 0x60a, 0x616
{
	LOGMASKED(LOG_TIMERS, "tstat_r: TSTAT%d: %04x\n", Timer + 1, m_tstat[Timer]);
	m_tclear[Timer] |= m_tstat[Timer];
	return m_tstat[Timer];
}

uint16_t mc68328_device::wctlr_r() // 0x618
{
	LOGMASKED(LOG_WATCHDOG, "wctlr_r: WCTLR: %04x\n", m_wctlr);
	return m_wctlr;
}

uint16_t mc68328_device::wcmpr_r() // 0x61a
{
	LOGMASKED(LOG_WATCHDOG, "wcmpr_r: WCMPR: %04x\n", m_wcmpr);
	return m_wcmpr;
}

uint16_t mc68328_device::wcn_r() // 0x61c
{
	LOGMASKED(LOG_WATCHDOG, "wcn_r: WCN: %04x\n", m_wcn);
	return m_wcn;
}

uint16_t mc68328_device::spisr_r() // 0x700
{
	LOGMASKED(LOG_SPIS, "spisr_r: SPISR: %04x\n", m_spisr);
	return m_spisr;
}

uint16_t mc68328_device::spimdata_r() // 0x800
{
	uint16_t data = m_spimdata;
	if (!m_in_spim_cb.isnull())
	{
		data = m_in_spim_cb(0, 0xffff);
	}
	LOGMASKED(LOG_SPIM, "spimdata_r: SPIMDATA: %04x\n", data);
	return data;
}

uint16_t mc68328_device::spimcont_r() // 0x802
{
	uint16_t data = m_spimcont;
	if (m_spimcont & SPIM_XCH)
	{
		m_spimcont &= ~SPIM_XCH;
		m_spimcont |= SPIM_SPIMIRQ;
		data = ((m_spimcont | SPIM_XCH) &~ SPIM_SPIMIRQ);
	}
	LOGMASKED(LOG_SPIM, "spimcont_r: SPIMCONT: %04x\n", data);
	return data;
}

uint16_t mc68328_device::ustcnt_r() // 0x900
{
	LOGMASKED(LOG_UART, "ustcnt_r: USTCNT: %04x\n", m_ustcnt);
	return m_ustcnt;
}

uint16_t mc68328_device::ubaud_r() // 0x902
{
	LOGMASKED(LOG_UART, "ubaud_r: UBAUD: %04x\n", m_ubaud);
	return m_ubaud;
}

uint16_t mc68328_device::urx_r() // 0x904
{
	LOGMASKED(LOG_UART, "urx_r: URX: %04x\n", m_urx);
	return m_urx;
}

uint16_t mc68328_device::utx_r() // 0x906
{
	uint16_t data = m_utx | UTX_FIFO_EMPTY | UTX_FIFO_HALF | UTX_TX_AVAIL;
	LOGMASKED(LOG_UART, "utx_r: UTX: %04x\n", data);
	return data;
}

uint16_t mc68328_device::umisc_r() // 0x908
{
	LOGMASKED(LOG_UART, "umisc_r: UMISC: %04x\n", m_umisc);
	return m_umisc;
}

uint16_t mc68328_device::lssa_msw_r() // 0xa00
{
	LOGMASKED(LOG_LCD, "lssa_msw_r: LSSA(MSW): %04x\n", (uint16_t)(m_lssa >> 16));
	return (uint16_t)(m_lssa >> 16);
}

uint16_t mc68328_device::lssa_lsw_r() // 0xa02
{
	LOGMASKED(LOG_LCD, "lssa_lsw_r: LSSA(LSW): %04x\n", (uint16_t)m_lssa);
	return (uint16_t)m_lssa;
}

uint8_t mc68328_device::lvpw_r() // 0xa05
{
	LOGMASKED(LOG_LCD, "lvpw_r: LVPW: %02x\n", m_lvpw);
	return m_lvpw;
}

uint16_t mc68328_device::lxmax_r() // 0xa08
{
	LOGMASKED(LOG_LCD, "lxmax_r: LXMAX: %04x\n", m_lxmax);
	return m_lxmax;
}

uint16_t mc68328_device::lymax_r() // 0xa0a
{
	LOGMASKED(LOG_LCD, "lymax_r: LYMAX: %04x\n", m_lymax);
	return m_lymax;
}

uint16_t mc68328_device::lcxp_r() // 0xa18
{
	LOGMASKED(LOG_LCD, "lcxp_r: LCXP: %04x\n", m_lcxp);
	return m_lcxp;
}

uint16_t mc68328_device::lcyp_r() // 0xa1a
{
	LOGMASKED(LOG_LCD, "lcyp_r: LCYP: %04x\n", m_lcyp);
	return m_lcyp;
}

uint16_t mc68328_device::lcwch_r() // 0xa1c
{
	LOGMASKED(LOG_LCD, "lcwch_r: LCWCH: %04x\n", m_lcwch);
	return m_lcwch;
}

uint8_t mc68328_device::lblkc_r() // 0xa1f
{
	LOGMASKED(LOG_LCD, "lblkc_r: LBLKC: %02x\n", m_lblkc);
	return m_lblkc;
}

uint8_t mc68328_device::lpicf_r() // 0xa20
{
	LOGMASKED(LOG_LCD, "lpicf_r: LPICF: %02x\n", m_lpicf);
	return m_lpicf;
}

uint8_t mc68328_device::lpolcf_r() // 0xa21
{
	LOGMASKED(LOG_LCD, "lpolcf_r: LPOLCF: %02x\n", m_lpolcf);
	return m_lpolcf;
}

uint8_t mc68328_device::lacdrc_r() // 0xa23
{
	LOGMASKED(LOG_LCD, "lacdrc_r: LACDRC: %02x\n", m_lacdrc);
	return m_lacdrc;
}

uint8_t mc68328_device::lpxcd_r() // 0xa25
{
	LOGMASKED(LOG_LCD, "lpxcd_r: LPXCD: %02x\n", m_lpxcd);
	return m_lpxcd;
}

uint8_t mc68328_device::lckcon_r() // 0xa27
{
	LOGMASKED(LOG_LCD, "lckcon_r: LCKCON: %02x\n", m_lckcon);
	return m_lckcon;
}

uint8_t mc68328_device::llbar_r() // 0xa29
{
	LOGMASKED(LOG_LCD, "llbar_r: LLBAR: %02x\n", m_llbar);
	return m_llbar;
}

uint8_t mc68328_device::lotcr_r() // 0xa2b
{
	LOGMASKED(LOG_LCD, "lotcr_r: LOTCR: %02x\n", m_lotcr);
	return m_lotcr;
}

uint8_t mc68328_device::lposr_r() // 0xa2d
{
	LOGMASKED(LOG_LCD, "lposr_r: LPOSR: %02x\n", m_lposr);
	return m_lposr;
}

uint8_t mc68328_device::lfrcm_r() // 0xa31
{
	LOGMASKED(LOG_LCD, "lfrcm_r: LFRCM: %02x\n", m_lfrcm);
	return m_lfrcm;
}

uint16_t mc68328_device::lgpmr_r() // 0xa32
{
	LOGMASKED(LOG_LCD, "lgpmr_r: LGPMR: %04x\n", m_lgpmr);
	return m_lgpmr;
}

uint16_t mc68328_device::hmsr_msw_r() // 0xb00
{
	LOGMASKED(LOG_RTC, "hmsr_msw_r: HMSR(MSW): %04x\n", (uint16_t)(m_hmsr >> 16));
	return (uint16_t)(m_hmsr >> 16);
}

uint16_t mc68328_device::hmsr_lsw_r() // 0xb02
{
	LOGMASKED(LOG_RTC, "hmsr_lsw_r: HMSR(LSW): %04x\n", (uint16_t)m_hmsr);
	return (uint16_t)m_hmsr;
}

uint16_t mc68328_device::alarm_msw_r() // 0xb04
{
	LOGMASKED(LOG_RTC, "alarm_msw_r: ALARM(MSW): %04x\n", (uint16_t)(m_alarm >> 16));
	return (uint16_t)(m_alarm >> 16);
}

uint16_t mc68328_device::alarm_lsw_r() // 0xb06
{
	LOGMASKED(LOG_RTC, "alarm_lsw_r: ALARM(LSW): %04x\n", (uint16_t)m_alarm);
	return (uint16_t)m_alarm;
}

uint16_t mc68328_device::rtcctl_r() // 0xb0c
{
	LOGMASKED(LOG_RTC, "rtcctl_r: RTCCTL: %04x\n", m_rtcctl);
	return m_rtcctl;
}

uint16_t mc68328_device::rtcisr_r() // 0xb0e
{
	LOGMASKED(LOG_RTC, "rtcisr_r: RTCISR: %04x\n", m_rtcisr);
	return m_rtcisr;
}

uint16_t mc68328_device::rtcienr_r() // 0xb10
{
	LOGMASKED(LOG_RTC, "rtcienr_r: RTCIENR: %04x\n", m_rtcienr);
	return m_rtcienr;
}

uint16_t mc68328_device::stpwtch_r() // 0xb12
{
	LOGMASKED(LOG_RTC, "stpwtch_r: STPWTCH: %04x\n", m_stpwtch);
	return m_stpwtch;
}

/* THIS IS PRETTY MUCH TOTALLY WRONG AND DOESN'T REFLECT THE MC68328'S INTERNAL FUNCTIONALITY AT ALL! */
uint32_t mc68328_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint32_t vram_addr = m_lssa & 0x00fffffe;

	if (m_lckcon & LCKCON_LCDC_EN)
	{
		for (int y = 0; y < 160; y++)
		{
			uint16_t *const line = &bitmap.pix(y);

			for (int x = 0; x < 160; x += 16, vram_addr += 2)
			{
				uint16_t const word = space(AS_PROGRAM).read_word(vram_addr);
				for (int b = 0; b < 16; b++)
				{
					line[x + b] = (word >> (15 - b)) & 0x0001;
				}
			}
		}
	}
	else
	{
		for (int y = 0; y < 160; y++)
		{
			uint16_t *const line = &bitmap.pix(y);

			for (int x = 0; x < 160; x++)
			{
				line[x] = 0;
			}
		}
	}
	return 0;
}


void mc68328_device::register_state_save()
{
	save_item(NAME(m_scr));
	save_item(NAME(m_grpbasea));
	save_item(NAME(m_grpbaseb));
	save_item(NAME(m_grpbasec));
	save_item(NAME(m_grpbased));
	save_item(NAME(m_grpmaska));
	save_item(NAME(m_grpmaskb));
	save_item(NAME(m_grpmaskc));
	save_item(NAME(m_grpmaskd));
	save_item(NAME(m_csa));
	save_item(NAME(m_csb));
	save_item(NAME(m_csc));
	save_item(NAME(m_csd));

	save_item(NAME(m_pllcr));
	save_item(NAME(m_pllfsr));
	save_item(NAME(m_pctlr));

	save_item(NAME(m_ivr));
	save_item(NAME(m_icr));
	save_item(NAME(m_imr));
	save_item(NAME(m_iwr));
	save_item(NAME(m_isr));
	save_item(NAME(m_ipr));

	save_item(NAME(m_padir));
	save_item(NAME(m_padata));
	save_item(NAME(m_pasel));
	save_item(NAME(m_pbdir));
	save_item(NAME(m_pbdata));
	save_item(NAME(m_pbsel));
	save_item(NAME(m_pcdir));
	save_item(NAME(m_pcdata));
	save_item(NAME(m_pcsel));
	save_item(NAME(m_pddir));
	save_item(NAME(m_pddata));
	save_item(NAME(m_pdpuen));
	save_item(NAME(m_pdpol));
	save_item(NAME(m_pdirqen));
	save_item(NAME(m_pddataedge));
	save_item(NAME(m_pdirqedge));
	save_item(NAME(m_pedir));
	save_item(NAME(m_pedata));
	save_item(NAME(m_pepuen));
	save_item(NAME(m_pesel));
	save_item(NAME(m_pfdir));
	save_item(NAME(m_pfdata));
	save_item(NAME(m_pfpuen));
	save_item(NAME(m_pfsel));
	save_item(NAME(m_pgdir));
	save_item(NAME(m_pgdata));
	save_item(NAME(m_pgpuen));
	save_item(NAME(m_pgsel));
	save_item(NAME(m_pjdir));
	save_item(NAME(m_pjdata));
	save_item(NAME(m_pjsel));
	save_item(NAME(m_pkdir));
	save_item(NAME(m_pkdata));
	save_item(NAME(m_pkpuen));
	save_item(NAME(m_pksel));
	save_item(NAME(m_pmdir));
	save_item(NAME(m_pmdata));
	save_item(NAME(m_pmpuen));
	save_item(NAME(m_pmsel));

	save_item(NAME(m_pwmc));
	save_item(NAME(m_pwmp));
	save_item(NAME(m_pwmw));
	save_item(NAME(m_pwmcnt));

	save_item(NAME(m_tctl));
	save_item(NAME(m_tprer));
	save_item(NAME(m_tcmp));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tcn));
	save_item(NAME(m_tstat));
	save_item(NAME(m_wctlr));
	save_item(NAME(m_wcmpr));
	save_item(NAME(m_wcn));

	save_item(NAME(m_spisr));

	save_item(NAME(m_spimdata));
	save_item(NAME(m_spimcont));

	save_item(NAME(m_ustcnt));
	save_item(NAME(m_ubaud));
	save_item(NAME(m_urx));
	save_item(NAME(m_utx));
	save_item(NAME(m_umisc));

	save_item(NAME(m_lssa));
	save_item(NAME(m_lvpw));
	save_item(NAME(m_lxmax));
	save_item(NAME(m_lymax));
	save_item(NAME(m_lcxp));
	save_item(NAME(m_lcyp));
	save_item(NAME(m_lcwch));
	save_item(NAME(m_lblkc));
	save_item(NAME(m_lpicf));
	save_item(NAME(m_lpolcf));
	save_item(NAME(m_lacdrc));
	save_item(NAME(m_lpxcd));
	save_item(NAME(m_lckcon));
	save_item(NAME(m_llbar));
	save_item(NAME(m_lotcr));
	save_item(NAME(m_lposr));
	save_item(NAME(m_lfrcm));
	save_item(NAME(m_lgpmr));

	save_item(NAME(m_hmsr));
	save_item(NAME(m_alarm));
	save_item(NAME(m_rtcctl));
	save_item(NAME(m_rtcisr));
	save_item(NAME(m_rtcienr));
	save_item(NAME(m_stpwtch));
}
