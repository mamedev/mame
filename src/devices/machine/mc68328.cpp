// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Motorola 68328 ("DragonBall") System-on-a-Chip implementation

    By Ryan Holtz

**********************************************************************/

#include "emu.h"
#include "mc68328.h"

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
#define LOG_ALL         (LOG_SCR | LOG_PLL | LOG_INTS | LOG_GPIO_A | LOG_GPIO_B | LOG_GPIO_C | LOG_GPIO_D | LOG_GPIO_E \
						| LOG_GPIO_F | LOG_GPIO_G | LOG_GPIO_J | LOG_GPIO_K | LOG_GPIO_M | LOG_PWM | LOG_TIMERS | LOG_TSTAT | LOG_WATCHDOG | LOG_SPIS \
						| LOG_SPIM | LOG_UART | LOG_LCD | LOG_RTC)
#define VERBOSE         (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MC68328, mc68328_device, "mc68328", "MC68328 DragonBall Processor")
DEFINE_DEVICE_TYPE(MC68EZ328, mc68ez328_device, "mc68ez328", "MC68EZ328 DragonBall-EZ Processor")

const u32 mc68328_base_device::VCO_DIVISORS[8] = { 2, 4, 8, 16, 1, 1, 1, 1 };

mc68328_base_device::mc68328_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: m68000_device(mconfig, type, tag, owner, clock)
	, m_pwm(nullptr)
	, m_rtc(nullptr)
	, m_spim(nullptr)
	, m_out_port_a_cb(*this)
	, m_out_port_b_cb(*this)
	, m_out_port_c_cb(*this)
	, m_out_port_d_cb(*this)
	, m_out_port_e_cb(*this)
	, m_out_port_f_cb(*this)
	, m_out_port_g_cb(*this)
	, m_in_port_a_cb(*this, 0)
	, m_in_port_b_cb(*this, 0)
	, m_in_port_c_cb(*this, 0)
	, m_in_port_d_cb(*this, 0)
	, m_in_port_e_cb(*this, 0)
	, m_in_port_f_cb(*this, 0)
	, m_in_port_g_cb(*this, 0)
	, m_out_pwm_cb(*this)
	, m_out_spim_cb(*this)
	, m_in_spim_cb(*this, 0)
	, m_out_flm_cb(*this)
	, m_out_llp_cb(*this)
	, m_out_lsclk_cb(*this)
	, m_out_ld_cb(*this)
	, m_lcd_info_changed_cb(*this)
{
}

mc68328_device::mc68328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mc68328_base_device(mconfig, MC68328, tag, owner, clock)
	, m_out_port_j_cb(*this)
	, m_out_port_k_cb(*this)
	, m_out_port_m_cb(*this)
	, m_in_port_j_cb(*this, 0)
	, m_in_port_k_cb(*this, 0)
	, m_in_port_m_cb(*this, 0)
{
	m_cpu_space_config.m_internal_map = address_map_constructor(FUNC(mc68328_device::cpu_space_map), this);
	auto imap = address_map_constructor(FUNC(mc68328_device::internal_map), this);
	m_program_config.m_internal_map = imap;
	m_opcodes_config.m_internal_map = imap;
	m_uprogram_config.m_internal_map = imap;
	m_uopcodes_config.m_internal_map = imap;
}

mc68ez328_device::mc68ez328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mc68328_base_device(mconfig, MC68EZ328, tag, owner, clock)
{
	m_cpu_space_config.m_internal_map = address_map_constructor(FUNC(mc68ez328_device::cpu_space_map), this);
	m_cpu_space_config.m_addr_width = 32;
	m_program_config.m_addr_width = 32;
	m_opcodes_config.m_addr_width = 32;
	m_uprogram_config.m_addr_width = 32;
	m_uopcodes_config.m_addr_width = 32;
	auto imap = address_map_constructor(FUNC(mc68ez328_device::internal_map), this);
	m_program_config.m_internal_map = imap;
	m_opcodes_config.m_internal_map = imap;
	m_uprogram_config.m_internal_map = imap;
	m_uopcodes_config.m_internal_map = imap;
}

void mc68328_base_device::base_internal_map(u32 addr_bits, address_map &map)
{
	map(addr_bits | 0x000, addr_bits | 0x000).rw(FUNC(mc68328_base_device::scr_r), FUNC(mc68328_base_device::scr_w));
	map(addr_bits | 0x100, addr_bits | 0x101).rw(FUNC(mc68328_base_device::grpbasea_r), FUNC(mc68328_base_device::grpbasea_w));
	map(addr_bits | 0x102, addr_bits | 0x103).rw(FUNC(mc68328_base_device::grpbaseb_r), FUNC(mc68328_base_device::grpbaseb_w));
	map(addr_bits | 0x104, addr_bits | 0x105).rw(FUNC(mc68328_base_device::grpbasec_r), FUNC(mc68328_base_device::grpbasec_w));
	map(addr_bits | 0x106, addr_bits | 0x107).rw(FUNC(mc68328_base_device::grpbased_r), FUNC(mc68328_base_device::grpbased_w));

	map(addr_bits | 0x200, addr_bits | 0x201).rw(FUNC(mc68328_base_device::pllcr_r), FUNC(mc68328_base_device::pllcr_w));
	map(addr_bits | 0x202, addr_bits | 0x203).rw(FUNC(mc68328_base_device::pllfsr_r), FUNC(mc68328_base_device::pllfsr_w));
	map(addr_bits | 0x207, addr_bits | 0x207).rw(FUNC(mc68328_base_device::pctlr_r), FUNC(mc68328_base_device::pctlr_w));

	map(addr_bits | 0x300, addr_bits | 0x300).rw(FUNC(mc68328_base_device::ivr_r), FUNC(mc68328_base_device::ivr_w));
	map(addr_bits | 0x302, addr_bits | 0x303).rw(FUNC(mc68328_base_device::icr_r), FUNC(mc68328_base_device::icr_w));
	map(addr_bits | 0x304, addr_bits | 0x305).rw(FUNC(mc68328_base_device::imr_msw_r), FUNC(mc68328_base_device::imr_msw_w));
	map(addr_bits | 0x306, addr_bits | 0x307).rw(FUNC(mc68328_base_device::imr_lsw_r), FUNC(mc68328_base_device::imr_lsw_w));
	map(addr_bits | 0x30c, addr_bits | 0x30d).rw(FUNC(mc68328_base_device::isr_msw_r), FUNC(mc68328_base_device::isr_msw_w));
	map(addr_bits | 0x30e, addr_bits | 0x30f).rw(FUNC(mc68328_base_device::isr_lsw_r), FUNC(mc68328_base_device::isr_lsw_w));
	map(addr_bits | 0x310, addr_bits | 0x311).rw(FUNC(mc68328_base_device::ipr_msw_r), FUNC(mc68328_base_device::ipr_msw_w));
	map(addr_bits | 0x312, addr_bits | 0x313).rw(FUNC(mc68328_base_device::ipr_lsw_r), FUNC(mc68328_base_device::ipr_lsw_w));

	map(addr_bits | 0x400, addr_bits | 0x400).rw(FUNC(mc68328_base_device::padir_r), FUNC(mc68328_base_device::padir_w));
	map(addr_bits | 0x401, addr_bits | 0x401).rw(FUNC(mc68328_base_device::padata_r), FUNC(mc68328_base_device::padata_w));
	map(addr_bits | 0x408, addr_bits | 0x408).rw(FUNC(mc68328_base_device::pbdir_r), FUNC(mc68328_base_device::pbdir_w));
	map(addr_bits | 0x409, addr_bits | 0x409).rw(FUNC(mc68328_base_device::pbdata_r), FUNC(mc68328_base_device::pbdata_w));
	map(addr_bits | 0x40b, addr_bits | 0x40b).rw(FUNC(mc68328_base_device::pbsel_r), FUNC(mc68328_base_device::pbsel_w));
	map(addr_bits | 0x410, addr_bits | 0x410).rw(FUNC(mc68328_base_device::pcdir_r), FUNC(mc68328_base_device::pcdir_w));
	map(addr_bits | 0x411, addr_bits | 0x411).rw(FUNC(mc68328_base_device::pcdata_r), FUNC(mc68328_base_device::pcdata_w));
	map(addr_bits | 0x413, addr_bits | 0x413).rw(FUNC(mc68328_base_device::pcsel_r), FUNC(mc68328_base_device::pcsel_w));
	map(addr_bits | 0x418, addr_bits | 0x418).rw(FUNC(mc68328_base_device::pddir_r), FUNC(mc68328_base_device::pddir_w));
	map(addr_bits | 0x419, addr_bits | 0x419).rw(FUNC(mc68328_base_device::pddata_r), FUNC(mc68328_base_device::pddata_w));
	map(addr_bits | 0x41a, addr_bits | 0x41a).rw(FUNC(mc68328_base_device::pdpuen_r), FUNC(mc68328_base_device::pdpuen_w));
	map(addr_bits | 0x41c, addr_bits | 0x41c).rw(FUNC(mc68328_base_device::pdpol_r), FUNC(mc68328_base_device::pdpol_w));
	map(addr_bits | 0x41d, addr_bits | 0x41d).rw(FUNC(mc68328_base_device::pdirqen_r), FUNC(mc68328_base_device::pdirqen_w));
	map(addr_bits | 0x41f, addr_bits | 0x41f).rw(FUNC(mc68328_base_device::pdirqedge_r), FUNC(mc68328_base_device::pdirqedge_w));
	map(addr_bits | 0x420, addr_bits | 0x420).rw(FUNC(mc68328_base_device::pedir_r), FUNC(mc68328_base_device::pedir_w));
	map(addr_bits | 0x421, addr_bits | 0x421).rw(FUNC(mc68328_base_device::pedata_r), FUNC(mc68328_base_device::pedata_w));
	map(addr_bits | 0x422, addr_bits | 0x422).rw(FUNC(mc68328_base_device::pepuen_r), FUNC(mc68328_base_device::pepuen_w));
	map(addr_bits | 0x423, addr_bits | 0x423).rw(FUNC(mc68328_base_device::pesel_r), FUNC(mc68328_base_device::pesel_w));
	map(addr_bits | 0x428, addr_bits | 0x428).rw(FUNC(mc68328_base_device::pfdir_r), FUNC(mc68328_base_device::pfdir_w));
	map(addr_bits | 0x429, addr_bits | 0x429).rw(FUNC(mc68328_base_device::pfdata_r), FUNC(mc68328_base_device::pfdata_w));
	map(addr_bits | 0x42a, addr_bits | 0x42a).rw(FUNC(mc68328_base_device::pfpuen_r), FUNC(mc68328_base_device::pfpuen_w));
	map(addr_bits | 0x42b, addr_bits | 0x42b).rw(FUNC(mc68328_base_device::pfsel_r), FUNC(mc68328_base_device::pfsel_w));
	map(addr_bits | 0x430, addr_bits | 0x430).rw(FUNC(mc68328_base_device::pgdir_r), FUNC(mc68328_base_device::pgdir_w));
	map(addr_bits | 0x431, addr_bits | 0x431).rw(FUNC(mc68328_base_device::pgdata_r), FUNC(mc68328_base_device::pgdata_w));
	map(addr_bits | 0x432, addr_bits | 0x432).rw(FUNC(mc68328_base_device::pgpuen_r), FUNC(mc68328_base_device::pgpuen_w));
	map(addr_bits | 0x433, addr_bits | 0x433).rw(FUNC(mc68328_base_device::pgsel_r), FUNC(mc68328_base_device::pgsel_w));

	map(addr_bits | 0x500, addr_bits | 0x501).rw(FUNC(mc68328_base_device::pwmc_r), FUNC(mc68328_base_device::pwmc_w));

	map(addr_bits | 0x600, addr_bits | 0x601).rw(FUNC(mc68328_base_device::tctl_r<0>), FUNC(mc68328_base_device::tctl_w<0>));
	map(addr_bits | 0x602, addr_bits | 0x603).rw(FUNC(mc68328_base_device::tprer_r<0>), FUNC(mc68328_base_device::tprer_w<0>));
	map(addr_bits | 0x604, addr_bits | 0x605).rw(FUNC(mc68328_base_device::tcmp_r<0>), FUNC(mc68328_base_device::tcmp_w<0>));
	map(addr_bits | 0x606, addr_bits | 0x607).rw(FUNC(mc68328_base_device::tcr_r<0>), FUNC(mc68328_base_device::tcr_w<0>));
	map(addr_bits | 0x608, addr_bits | 0x609).rw(FUNC(mc68328_base_device::tcn_r<0>), FUNC(mc68328_base_device::tcn_w<0>));
	map(addr_bits | 0x60a, addr_bits | 0x60b).rw(FUNC(mc68328_base_device::tstat_r<0>), FUNC(mc68328_base_device::tstat_w<0>));

	map(addr_bits | 0x800, addr_bits | 0x801).rw(FUNC(mc68328_base_device::spimdata_r), FUNC(mc68328_base_device::spimdata_w));
	map(addr_bits | 0x802, addr_bits | 0x803).rw(FUNC(mc68328_base_device::spimcont_r), FUNC(mc68328_base_device::spimcont_w));

	map(addr_bits | 0x900, addr_bits | 0x901).rw(FUNC(mc68328_base_device::ustcnt_r), FUNC(mc68328_base_device::ustcnt_w));
	map(addr_bits | 0x902, addr_bits | 0x903).rw(FUNC(mc68328_base_device::ubaud_r), FUNC(mc68328_base_device::ubaud_w));
	map(addr_bits | 0x904, addr_bits | 0x905).rw(FUNC(mc68328_base_device::urx_r), FUNC(mc68328_base_device::urx_w));
	map(addr_bits | 0x906, addr_bits | 0x907).rw(FUNC(mc68328_base_device::utx_r), FUNC(mc68328_base_device::utx_w));
	map(addr_bits | 0x908, addr_bits | 0x909).rw(FUNC(mc68328_base_device::umisc_r), FUNC(mc68328_base_device::umisc_w));

	map(addr_bits | 0xa00, addr_bits | 0xa01).rw(FUNC(mc68328_base_device::lssa_msw_r), FUNC(mc68328_base_device::lssa_msw_w));
	map(addr_bits | 0xa02, addr_bits | 0xa03).rw(FUNC(mc68328_base_device::lssa_lsw_r), FUNC(mc68328_base_device::lssa_lsw_w));
	map(addr_bits | 0xa05, addr_bits | 0xa05).rw(FUNC(mc68328_base_device::lvpw_r), FUNC(mc68328_base_device::lvpw_w));
	map(addr_bits | 0xa08, addr_bits | 0xa09).rw(FUNC(mc68328_base_device::lxmax_r), FUNC(mc68328_base_device::lxmax_w));
	map(addr_bits | 0xa0a, addr_bits | 0xa0b).rw(FUNC(mc68328_base_device::lymax_r), FUNC(mc68328_base_device::lymax_w));
	map(addr_bits | 0xa18, addr_bits | 0xa19).rw(FUNC(mc68328_base_device::lcxp_r), FUNC(mc68328_base_device::lcxp_w));
	map(addr_bits | 0xa1a, addr_bits | 0xa1b).rw(FUNC(mc68328_base_device::lcyp_r), FUNC(mc68328_base_device::lcyp_w));
	map(addr_bits | 0xa1c, addr_bits | 0xa1d).rw(FUNC(mc68328_base_device::lcwch_r), FUNC(mc68328_base_device::lcwch_w));
	map(addr_bits | 0xa1f, addr_bits | 0xa1f).rw(FUNC(mc68328_base_device::lblkc_r), FUNC(mc68328_base_device::lblkc_w));
	map(addr_bits | 0xa20, addr_bits | 0xa20).rw(FUNC(mc68328_base_device::lpicf_r), FUNC(mc68328_base_device::lpicf_w));
	map(addr_bits | 0xa21, addr_bits | 0xa21).rw(FUNC(mc68328_base_device::lpolcf_r), FUNC(mc68328_base_device::lpolcf_w));
	map(addr_bits | 0xa23, addr_bits | 0xa23).rw(FUNC(mc68328_base_device::lacdrc_r), FUNC(mc68328_base_device::lacdrc_w));
	map(addr_bits | 0xa25, addr_bits | 0xa25).rw(FUNC(mc68328_base_device::lpxcd_r), FUNC(mc68328_base_device::lpxcd_w));
	map(addr_bits | 0xa27, addr_bits | 0xa27).rw(FUNC(mc68328_base_device::lckcon_r), FUNC(mc68328_base_device::lckcon_w));
	map(addr_bits | 0xa2d, addr_bits | 0xa2d).rw(FUNC(mc68328_base_device::lposr_r), FUNC(mc68328_base_device::lposr_w));
	map(addr_bits | 0xa31, addr_bits | 0xa31).rw(FUNC(mc68328_base_device::lfrcm_r), FUNC(mc68328_base_device::lfrcm_w));

	map(addr_bits | 0xb00, addr_bits | 0xb01).rw(FUNC(mc68328_base_device::hmsr_msw_r), FUNC(mc68328_base_device::hmsr_msw_w));
	map(addr_bits | 0xb02, addr_bits | 0xb03).rw(FUNC(mc68328_base_device::hmsr_lsw_r), FUNC(mc68328_base_device::hmsr_lsw_w));
	map(addr_bits | 0xb04, addr_bits | 0xb05).rw(FUNC(mc68328_base_device::alarm_msw_r), FUNC(mc68328_base_device::alarm_msw_w));
	map(addr_bits | 0xb06, addr_bits | 0xb07).rw(FUNC(mc68328_base_device::alarm_lsw_r), FUNC(mc68328_base_device::alarm_lsw_w));
	map(addr_bits | 0xb0e, addr_bits | 0xb0f).rw(FUNC(mc68328_base_device::rtcisr_r), FUNC(mc68328_base_device::rtcisr_w));
	map(addr_bits | 0xb10, addr_bits | 0xb11).rw(FUNC(mc68328_base_device::rtcienr_r), FUNC(mc68328_base_device::rtcienr_w));
	map(addr_bits | 0xb12, addr_bits | 0xb13).rw(FUNC(mc68328_base_device::stpwtch_r), FUNC(mc68328_base_device::stpwtch_w));
}

void mc68328_device::internal_map(address_map &map)
{
	base_internal_map(0xfff000, map);
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

	map(0xfff308, 0xfff309).rw(FUNC(mc68328_device::iwr_msw_r), FUNC(mc68328_device::iwr_msw_w));
	map(0xfff30a, 0xfff30b).rw(FUNC(mc68328_device::iwr_lsw_r), FUNC(mc68328_device::iwr_lsw_w));

	map(0xfff403, 0xfff403).rw(FUNC(mc68328_device::pasel_r), FUNC(mc68328_device::pasel_w));

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

	map(0xfff502, 0xfff503).rw(FUNC(mc68328_device::pwmp_r), FUNC(mc68328_device::pwmp_w));
	map(0xfff504, 0xfff505).rw(FUNC(mc68328_device::pwmw_r), FUNC(mc68328_device::pwmw_w));
	map(0xfff506, 0xfff507).rw(FUNC(mc68328_device::pwmcnt_r), FUNC(mc68328_device::pwmcnt_w));

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

	map(0xfffa29, 0xfffa29).rw(FUNC(mc68328_device::llbar_r), FUNC(mc68328_device::llbar_w));
	map(0xfffa2b, 0xfffa2b).rw(FUNC(mc68328_device::lotcr_r), FUNC(mc68328_device::lotcr_w));
	map(0xfffa32, 0xfffa33).rw(FUNC(mc68328_device::lgpmr_r), FUNC(mc68328_device::lgpmr_w));

	map(0xfffb0c, 0xfffb0d).rw(FUNC(mc68328_device::rtcctl_r), FUNC(mc68328_device::rtcctl_w));
}

void mc68ez328_device::internal_map(address_map &map)
{
	base_internal_map(0xfffff000, map);

	map(0xfffff004, 0xfffff007).r(FUNC(mc68ez328_device::revision_r));

	map(0xfffff110, 0xfffff111).rw(FUNC(mc68ez328_device::csa_r), FUNC(mc68ez328_device::csa_w));
	map(0xfffff112, 0xfffff113).rw(FUNC(mc68ez328_device::csb_r), FUNC(mc68ez328_device::csb_w));
	map(0xfffff114, 0xfffff115).rw(FUNC(mc68ez328_device::csc_r), FUNC(mc68ez328_device::csc_w));
	map(0xfffff116, 0xfffff117).rw(FUNC(mc68ez328_device::csd_r), FUNC(mc68ez328_device::csd_w));
	map(0xfffff118, 0xfffff119).rw(FUNC(mc68ez328_device::emucs_r), FUNC(mc68ez328_device::emucs_w));

	map(0xfffff502, 0xfffff503).rw(FUNC(mc68ez328_device::pwms_r), FUNC(mc68ez328_device::pwms_w));
	map(0xfffff504, 0xfffff504).rw(FUNC(mc68ez328_device::pwmp_r), FUNC(mc68ez328_device::pwmp_w));
	map(0xfffff505, 0xfffff505).rw(FUNC(mc68ez328_device::pwmcnt_r), FUNC(mc68ez328_device::pwmcnt_w));

	map(0xfffffa29, 0xfffffa29).rw(FUNC(mc68ez328_device::lrra_r), FUNC(mc68ez328_device::lrra_w));
	map(0xfffffa36, 0xfffffa37).rw(FUNC(mc68ez328_device::pwmr_r), FUNC(mc68ez328_device::pwmr_w));

	map(0xfffffb0a, 0xfffffb0b).rw(FUNC(mc68ez328_device::watchdog_r), FUNC(mc68ez328_device::watchdog_w));
	map(0xfffffb0c, 0xfffffb0d).rw(FUNC(mc68ez328_device::rtcctl_r), FUNC(mc68ez328_device::rtcctl_w));
	map(0xfffffb1a, 0xfffffb1b).rw(FUNC(mc68ez328_device::dayr_r), FUNC(mc68ez328_device::dayr_w));
	map(0xfffffb1c, 0xfffffb1d).rw(FUNC(mc68ez328_device::dayalarm_r), FUNC(mc68ez328_device::dayalarm_w));
}

void mc68328_device::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).r(FUNC(mc68328_device::irq_callback)).umask16(0x00ff);
}

void mc68ez328_device::cpu_space_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).r(FUNC(mc68ez328_device::irq_callback)).umask16(0x00ff);
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mc68328_base_device::device_resolve_objects()
{
	m68000_device::device_resolve_objects();

	m_lcd_info_changed_cb.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc68328_base_device::device_start()
{
	m68000_device::device_start();

	m_refclk = timer_alloc(FUNC(mc68328_base_device::refclk_tick), this);
	m_pwm = timer_alloc(FUNC(mc68328_base_device::pwm_tick), this);
	m_rtc = timer_alloc(FUNC(mc68328_base_device::rtc_tick), this);
	m_spim = timer_alloc(FUNC(mc68328_base_device::spim_tick), this);
	m_lcd_scan = timer_alloc(FUNC(mc68328_base_device::lcd_scan_tick), this);

	m_lcd_line_buffer = std::make_unique<u16[]>(1024 / 16); // 1024px wide, up to 16 pixels per word

	register_state_save();
}

void mc68328_device::device_start()
{
	mc68328_base_device::device_start();

	m_gptimer[0] = timer_alloc(FUNC(mc68328_device::timer_tick<0>), this);
	m_gptimer[1] = timer_alloc(FUNC(mc68328_device::timer_tick<1>), this);
}

void mc68ez328_device::device_start()
{
	mc68328_base_device::device_start();

	m_gptimer = timer_alloc(FUNC(mc68ez328_device::timer_tick<0>), this);
	m_rtc_sample_timer = timer_alloc(FUNC(mc68ez328_device::sample_timer_tick), this);

	m_dayr = 0;
	m_dayalarm = 0;
	m_sam_cnt = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc68328_base_device::device_reset()
{
	m68000_device::device_reset();

	m_scr = 0x0c;
	m_grpbasea = 0x0000;
	m_grpbaseb = 0x0000;
	m_grpbasec = 0x0000;
	m_grpbased = 0x0000;

	m_refclk->adjust(attotime::from_hz(32768), 0, attotime::from_hz(32768));
	m_pllcr = 0x2400;
	m_pllfsr = 0x0123;
	m_pctlr = 0x1f;

	m_ivr = 0x00;
	m_icr = 0x0000;
	m_imr = 0x00ffffff;
	m_gisr = 0x00000000;
	m_ipr = 0x00000000;

	m_pasel = 0x00;
	m_padir = 0x00;
	m_padata = 0x00;
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
	m_pdirqedge = 0x00;
	m_pdindata = 0x00;
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

	m_spimdata = 0x0000;
	m_spimcont = 0x0000;
	m_spmtxd = false;
	m_spmrxd = false;
	m_spmclk = false;
	m_spim_bit_read_idx = 15;

	m_ustcnt = 0x0000;
	m_ubaud = 0x003f;
	m_urx = 0x0000;
	m_utx = 0x0000;
	m_umisc = 0x0000;

	m_lssa = 0x00000000;
	m_lssa_end = 0x00000000;
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
	m_lposr = 0x00;
	m_lfrcm = 0xb9;
	m_lcd_update_pending = true;

	m_hmsr = 0x00000000;
	m_alarm = 0x00000000;
	m_rtcctl = 0x00;

	m_rtcisr = 0x0000;
	m_rtcienr = 0x0000;
	m_stpwtch = 0x0000;

	m_pwm->adjust(attotime::never);
	m_pwmo = false;

	m_rtc->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
	m_spim->adjust(attotime::never);
	m_lcd_scan->adjust(attotime::never);
	m_lcd_sysmem_ptr = 0;
	m_lssa_end = 0;
	m_lcd_line_bit = 0;
	m_lcd_line_word = 0;
	m_lsclk = false;
}

void mc68328_device::device_reset()
{
	mc68328_base_device::device_reset();

	m_grpmaska = 0x0000;
	m_grpmaskb = 0x0000;
	m_grpmaskc = 0x0000;
	m_grpmaskd = 0x0000;
	std::fill(std::begin(m_csa), std::end(m_csa), 0x00010006);
	std::fill(std::begin(m_csb), std::end(m_csb), 0x00010006);
	std::fill(std::begin(m_csc), std::end(m_csc), 0x00010006);
	std::fill(std::begin(m_csd), std::end(m_csd), 0x00010006);

	m_iwr = 0x00ffffff;

	m_pasel = 0xff;
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

	m_timer_regs[0].tctl = m_timer_regs[1].tctl = 0x0000;
	m_timer_regs[0].tprer = m_timer_regs[1].tprer = 0x0000;
	m_timer_regs[0].tcmp = m_timer_regs[1].tcmp = 0xffff;
	m_timer_regs[0].tcr = m_timer_regs[1].tcr = 0x0000;
	m_timer_regs[0].tcn = m_timer_regs[1].tcn = 0x0000;
	m_timer_regs[0].tstat = m_timer_regs[1].tstat = 0x0000;
	m_wctlr = 0x0000;
	m_wcmpr = 0xffff;
	m_wcn = 0x0000;
	m_timer_regs[0].tclear = m_timer_regs[1].tclear = 0;

	m_spisr = 0x0000;

	m_llbar = 0x3e;
	m_lotcr = 0x3f;
	m_lgpmr = 0x1073;

	m_gptimer[0]->adjust(attotime::never);
	m_gptimer[1]->adjust(attotime::never);
}

void mc68ez328_device::device_reset()
{
	mc68328_base_device::device_reset();

	m_csa = 0x00e0;
	m_csb = 0x0000;
	m_csc = 0x0000;
	m_csd = 0x0020;
	m_emucs = 0x0060;

	m_pwmc = 0x0020;
	m_pwmp = 0xfe;
	m_pwmcnt = 0x00;
	std::fill(std::begin(m_pwmfifo), std::end(m_pwmfifo), 0x0000);
	m_pwmfifo_wr = 0;
	m_pwmfifo_rd = 0;
	m_pwmfifo_cnt = 0;
	m_pwm_rep_cnt = 1;

	m_gptimer->adjust(attotime::never);

	m_lrra = 0xff;
	m_pwmr = 0x0000;

	m_rtc_sample_timer->adjust(attotime::from_ticks(64, 32768), 0, attotime::from_ticks(64, 32768));
	m_watchdog = 0x0001;
	m_sam_cnt = 0;
}

void mc68328_base_device::register_state_save()
{
	save_item(NAME(m_scr));
	save_item(NAME(m_grpbasea));
	save_item(NAME(m_grpbaseb));
	save_item(NAME(m_grpbasec));
	save_item(NAME(m_grpbased));

	save_item(NAME(m_pllcr));
	save_item(NAME(m_pllfsr));
	save_item(NAME(m_pctlr));

	save_item(NAME(m_ivr));
	save_item(NAME(m_icr));
	save_item(NAME(m_imr));
	save_item(NAME(m_gisr));
	save_item(NAME(m_ipr));

	save_item(NAME(m_padir));
	save_item(NAME(m_padata));
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
	save_item(NAME(m_pdirqedge));
	save_item(NAME(m_pdindata));
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

	save_item(NAME(m_pwmc));
	save_item(NAME(m_pwmo));

	save_item(NAME(m_spimdata));
	save_item(NAME(m_spimcont));
	save_item(NAME(m_spmtxd));
	save_item(NAME(m_spmrxd));
	save_item(NAME(m_spmclk));
	save_item(NAME(m_spim_bit_read_idx));

	save_item(NAME(m_ustcnt));
	save_item(NAME(m_ubaud));
	save_item(NAME(m_urx));
	save_item(NAME(m_utx));
	save_item(NAME(m_umisc));

	save_item(NAME(m_lssa));
	save_item(NAME(m_lssa_end));
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
	save_item(NAME(m_lposr));
	save_item(NAME(m_lfrcm));
	save_item(NAME(m_lcd_update_pending));

	save_item(NAME(m_hmsr));
	save_item(NAME(m_alarm));
	save_item(NAME(m_rtcctl));
	save_item(NAME(m_rtcisr));
	save_item(NAME(m_rtcienr));
	save_item(NAME(m_stpwtch));

	save_item(NAME(m_lcd_sysmem_ptr));
	save_pointer(NAME(m_lcd_line_buffer), 1024 / 8);
	save_item(NAME(m_lcd_line_bit));
	save_item(NAME(m_lcd_line_word));
	save_item(NAME(m_lsclk));
}

void mc68328_device::register_state_save()
{
	mc68328_base_device::register_state_save();

	save_item(NAME(m_grpmaska));
	save_item(NAME(m_grpmaskb));
	save_item(NAME(m_grpmaskc));
	save_item(NAME(m_grpmaskd));
	save_item(NAME(m_csa));
	save_item(NAME(m_csb));
	save_item(NAME(m_csc));
	save_item(NAME(m_csd));

	save_item(NAME(m_iwr));

	save_item(NAME(m_pasel));
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

	save_item(NAME(m_pwmp));
	save_item(NAME(m_pwmw));
	save_item(NAME(m_pwmcnt));

	save_item(STRUCT_MEMBER(m_timer_regs, tctl));
	save_item(STRUCT_MEMBER(m_timer_regs, tprer));
	save_item(STRUCT_MEMBER(m_timer_regs, tcmp));
	save_item(STRUCT_MEMBER(m_timer_regs, tcr));
	save_item(STRUCT_MEMBER(m_timer_regs, tcn));
	save_item(STRUCT_MEMBER(m_timer_regs, tstat));
	save_item(STRUCT_MEMBER(m_timer_regs, tclear));
	save_item(NAME(m_wctlr));
	save_item(NAME(m_wcmpr));
	save_item(NAME(m_wcn));

	save_item(NAME(m_spisr));

	save_item(NAME(m_llbar));
	save_item(NAME(m_lotcr));
	save_item(NAME(m_lgpmr));
}

void mc68ez328_device::register_state_save()
{
	mc68328_base_device::register_state_save();

	save_item(NAME(m_pwmp));
	save_item(NAME(m_pwmcnt));
	save_item(NAME(m_pwmfifo));
	save_item(NAME(m_pwmfifo_wr));
	save_item(NAME(m_pwmfifo_rd));
	save_item(NAME(m_pwmfifo_cnt));
	save_item(NAME(m_pwm_rep_cnt));

	save_item(STRUCT_MEMBER(m_timer_regs, tctl));
	save_item(STRUCT_MEMBER(m_timer_regs, tprer));
	save_item(STRUCT_MEMBER(m_timer_regs, tcmp));
	save_item(STRUCT_MEMBER(m_timer_regs, tcr));
	save_item(STRUCT_MEMBER(m_timer_regs, tcn));
	save_item(STRUCT_MEMBER(m_timer_regs, tstat));
	save_item(STRUCT_MEMBER(m_timer_regs, tclear));

	save_item(NAME(m_lrra));
	save_item(NAME(m_pwmr));

	save_item(NAME(m_watchdog));
	save_item(NAME(m_dayr));
	save_item(NAME(m_dayalarm));
	save_item(NAME(m_sam_cnt));
}

//-------------------------------------------------
//  System control hardware
//-------------------------------------------------

void mc68328_base_device::scr_w(u8 data) // 0x000
{
	LOGMASKED(LOG_SCR, "%s: scr_w: SCR = %02x\n", machine().describe_context(), data);
}

u8 mc68328_base_device::scr_r() // 0x000
{
	LOGMASKED(LOG_SCR, "%s: scr_r: SCR: %02x\n", machine().describe_context(), m_scr);
	return m_scr;
}


//-------------------------------------------------
//  MMU/chip-select hardware - Standard MC68328
//-------------------------------------------------

void mc68328_base_device::grpbasea_w(u16 data) // 0x100
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbasea_w: GRPBASEA = %04x\n", machine().describe_context(), data);
	m_grpbasea = data;
}

u16 mc68328_base_device::grpbasea_r() // 0x100
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbasea_r: GRPBASEA: %04x\n", machine().describe_context(), m_grpbasea);
	return m_grpbasea;
}

void mc68328_base_device::grpbaseb_w(u16 data) // 0x102
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbaseb_w: GRPBASEB = %04x\n", machine().describe_context(), data);
	m_grpbaseb = data;
}

u16 mc68328_base_device::grpbaseb_r() // 0x102
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbaseb_r: GRPBASEB: %04x\n", machine().describe_context(), m_grpbaseb);
	return m_grpbaseb;
}

void mc68328_base_device::grpbasec_w(u16 data) // 0x104
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbasec_w: GRPBASEC = %04x\n", machine().describe_context(), data);
	m_grpbasec = data;
}

u16 mc68328_base_device::grpbasec_r() // 0x104
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbasec_r: GRPBASEC: %04x\n", machine().describe_context(), m_grpbasec);
	return m_grpbasec;
}

void mc68328_base_device::grpbased_w(u16 data) // 0x106
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbased_w: GRPBASED = %04x\n", machine().describe_context(), data);
	m_grpbased = data;
}

u16 mc68328_base_device::grpbased_r() // 0x106
{
	LOGMASKED(LOG_CS_GRP, "%s: grpbased_r: GRPBASED: %04x\n", machine().describe_context(), m_grpbased);
	return m_grpbased;
}

void mc68328_device::grpmaska_w(u16 data) // 0x108
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaska_w: GRPMASKA = %04x\n", machine().describe_context(), data);
	m_grpmaska = data;
}

u16 mc68328_device::grpmaska_r() // 0x108
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaska_r: GRPMASKA: %04x\n", machine().describe_context(), m_grpmaska);
	return m_grpmaska;
}

void mc68328_device::grpmaskb_w(u16 data) // 0x10a
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaskb_w: GRPMASKB = %04x\n", machine().describe_context(), data);
	m_grpmaskb = data;
}

u16 mc68328_device::grpmaskb_r() // 0x10a
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaskb_r: GRPMASKB: %04x\n", machine().describe_context(), m_grpmaskb);
	return m_grpmaskb;
}

void mc68328_device::grpmaskc_w(u16 data) // 0x10c
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaskc_w: GRPMASKC = %04x\n", machine().describe_context(), data);
	m_grpmaskc = data;
}

u16 mc68328_device::grpmaskc_r() // 0x10c
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaskc_r: GRPMASKC: %04x\n", machine().describe_context(), m_grpmaskc);
	return m_grpmaskc;
}

void mc68328_device::grpmaskd_w(u16 data) // 0x10e
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaskd_w: GRPMASKD = %04x\n", machine().describe_context(), data);
	m_grpmaskd = data;
}

u16 mc68328_device::grpmaskd_r() // 0x10e
{
	LOGMASKED(LOG_CS_GRP, "%s: grpmaskd_r: GRPMASKD: %04x\n", machine().describe_context(), m_grpmaskd);
	return m_grpmaskd;
}

template<int ChipSelect>
void mc68328_device::csa_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x110, 0x114, 0x118, 0x11c
{
	LOGMASKED(LOG_CS_SEL, "%s: csa_msw_w<%d>: CSA%d(16) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csa[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csa[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
u16 mc68328_device::csa_msw_r() // 0x110, 0x120, 0x130, 0x140
{
	LOGMASKED(LOG_CS_SEL, "%s: csa_msw_r: CSA%d(MSW): %04x\n", machine().describe_context(), ChipSelect, (u16)(m_csa[ChipSelect] >> 16));
	return (u16)(m_csa[ChipSelect] >> 16);
}

template<int ChipSelect>
void mc68328_device::csa_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x112, 0x116, 0x11a, 0x11e
{
	LOGMASKED(LOG_CS_SEL, "%s: csa_lsw_w<%d>: CSA%d(0) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csa[ChipSelect] &= ~(mem_mask << 16);
	m_csa[ChipSelect] |= (data & mem_mask) << 16;
}

template<int ChipSelect>
u16 mc68328_device::csa_lsw_r() // 0x112, 0x122, 0x132, 0x142
{
	LOGMASKED(LOG_CS_SEL, "%s: csa_lsw_r: CSA%d(LSW): %04x\n", machine().describe_context(), ChipSelect, (u16)m_csa[ChipSelect]);
	return (u16)m_csa[ChipSelect];
}

template<int ChipSelect>
void mc68328_device::csb_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x120, 0x124, 0x128, 0x12c
{
	LOGMASKED(LOG_CS_SEL, "%s: csb_msw_w<%d>: CSB%d(MSW) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csb[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csb[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
u16 mc68328_device::csb_msw_r() // 0x114, 0x124, 0x134, 0x144
{
	LOGMASKED(LOG_CS_SEL, "%s: csb_msw_r: CSB%d(MSW): %04x\n", machine().describe_context(), ChipSelect, (u16)(m_csb[ChipSelect] >> 16));
	return (u16)(m_csb[ChipSelect] >> 16);
}

template<int ChipSelect>
void mc68328_device::csb_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x122, 0x126, 0x12a, 0x12e
{
	LOGMASKED(LOG_CS_SEL, "%s: csb_lsw_w<%d>: CSB%d(LSW) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csb[ChipSelect] &= ~(mem_mask << 16);
	m_csb[ChipSelect] |= (data & mem_mask) << 16;
}

template<int ChipSelect>
u16 mc68328_device::csb_lsw_r() // 0x116, 0x126, 0x136, 0x146
{
	LOGMASKED(LOG_CS_SEL, "%s: csb_lsw_r: CSB%d(LSW): %04x\n", machine().describe_context(), ChipSelect, (u16)m_csb[ChipSelect]);
	return (u16)m_csb[ChipSelect];
}

template<int ChipSelect>
void mc68328_device::csc_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x130, 0x134, 0x138, 0x13c
{
	LOGMASKED(LOG_CS_SEL, "%s: csc_msw_w<%d>: CSC%d(MSW) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csc[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csc[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
u16 mc68328_device::csc_msw_r() // 0x118, 0x128, 0x138, 0x148
{
	LOGMASKED(LOG_CS_SEL, "%s: csc_msw_r: CSC%d(MSW): %04x\n", machine().describe_context(), ChipSelect, (u16)(m_csc[ChipSelect] >> 16));
	return (u16)(m_csc[ChipSelect] >> 16);
}

template<int ChipSelect>
void mc68328_device::csc_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x132, 0x136, 0x13a, 0x13e
{
	LOGMASKED(LOG_CS_SEL, "%s: csc_lsw_w<%d>: CSC%d(LSW) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csc[ChipSelect] &= ~(mem_mask << 16);
	m_csc[ChipSelect] |= (data & mem_mask) << 16;
}

template<int ChipSelect>
u16 mc68328_device::csc_lsw_r() // 0x11a, 0x12a, 0x13a, 0x14a
{
	LOGMASKED(LOG_CS_SEL, "%s: csc_lsw_r: CSC%d(LSW): %04x\n", machine().describe_context(), ChipSelect, (u16)m_csc[ChipSelect]);
	return (u16)m_csc[ChipSelect];
}

template<int ChipSelect>
void mc68328_device::csd_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x140, 0x144, 0x148, 0x14c
{
	LOGMASKED(LOG_CS_SEL, "%s: csd_msw_w<%d>: CSD%d(MSW) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csd[ChipSelect] &= 0xffff0000 | (~mem_mask);
	m_csd[ChipSelect] |= data & mem_mask;
}

template<int ChipSelect>
u16 mc68328_device::csd_msw_r() // 0x11c, 0x12c, 0x13c, 0x14c
{
	LOGMASKED(LOG_CS_SEL, "%s: csd_msw_r: CSD%d(MSW): %04x\n", machine().describe_context(), ChipSelect, (u16)(m_csd[ChipSelect] >> 16));
	return (u16)(m_csd[ChipSelect] >> 16);
}

template<int ChipSelect>
void mc68328_device::csd_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x142, 0x146, 0x14a, 0x14e
{
	LOGMASKED(LOG_CS_SEL, "%s: csd_lsw_w<%d>: CSD%d(LSW) = %04x\n", machine().describe_context(), ChipSelect, ChipSelect, data);
	m_csd[ChipSelect] &= ~(mem_mask << 16);
	m_csd[ChipSelect] |= (data & mem_mask) << 16;
}

template<int ChipSelect>
u16 mc68328_device::csd_lsw_r() // 0x11e, 0x12e, 0x13e, 0x14e
{
	LOGMASKED(LOG_CS_SEL, "%s: csd_lsw_r: CSD%d(LSW): %04x\n", machine().describe_context(), ChipSelect, (u16)m_csd[ChipSelect]);
	return (u16)m_csd[ChipSelect];
}


//-------------------------------------------------
//  MMU/chip-select hardware - EZ variant
//-------------------------------------------------

void mc68ez328_device::scr_w(u8 data)
{
	if (data & SCR_WDTH8)
	{
		m_pasel = 0xff;
	}
	mc68328_base_device::scr_w(data);
}

u8 mc68ez328_device::revision_r(offs_t offset)
{
	LOGMASKED(LOG_PLL, "%s: revision_r: Silicon Revision[%d] = %02x\n", machine().describe_context(), offset, 0x01);
	return 0x01;
}

void mc68ez328_device::csa_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const char *const SIZ_NAMES[8] = { "128K", "256K", "512K", "1M", "2M", "4M", "8M", "16M" };
	static const char *const WS_NAMES[8] = { "None", "1", "2", "3", "4", "5", "6", "External /DTACK" };
	LOGMASKED(LOG_CS_SEL, "%s: csa_w: CSA = %04x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CS_SEL, "%s:        Enable: %d\n", machine().describe_context(), BIT(data, CS_EN_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Chip-Select Size: %s\n", machine().describe_context(), SIZ_NAMES[(data & CS_SIZ_MASK) >> CS_SIZ_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Wait States: %s\n", machine().describe_context(), WS_NAMES[(data & CS_WS_MASK) >> CS_WS_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Bus Width: %d Bits\n", machine().describe_context(), BIT(data, CS_BSW_BIT) ? 16 : 8);
	LOGMASKED(LOG_CS_SEL, "%s:        Delay /LWE and /UWE for Flash: %s\n", machine().describe_context(), BIT(data, CS_FLASH_BIT) ? "Yes" : "No");
	LOGMASKED(LOG_CS_SEL, "%s:        Read-Only: %d\n", machine().describe_context(), BIT(data, CS_RO_BIT));
	m_csa = data;
}

u16 mc68ez328_device::csa_r()
{
	LOGMASKED(LOG_CS_SEL, "%s: csa_r: CSA: %04x\n", machine().describe_context(), m_csa);
	return m_csa;
}

void mc68ez328_device::csb_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const char *const SIZ_NAMES[8] = { "128K", "256K", "512K", "1M", "2M", "4M", "8M", "16M" };
	static const char *const WS_NAMES[8] = { "None", "1", "2", "3", "4", "5", "6", "External /DTACK" };
	LOGMASKED(LOG_CS_SEL, "%s: csb_w: CSB = %04x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CS_SEL, "%s:        Enable: %d\n", machine().describe_context(), BIT(data, CS_EN_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Chip-Select Size: %s\n", machine().describe_context(), SIZ_NAMES[(data & CS_SIZ_MASK) >> CS_SIZ_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Wait States: %s\n", machine().describe_context(), WS_NAMES[(data & CS_WS_MASK) >> CS_WS_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Bus Width: %d Bits\n", machine().describe_context(), BIT(data, CS_BSW_BIT) ? 16 : 8);
	LOGMASKED(LOG_CS_SEL, "%s:        Delay /LWE and /UWE for Flash: %s\n", machine().describe_context(), BIT(data, CS_FLASH_BIT) ? "Yes" : "No");
	LOGMASKED(LOG_CS_SEL, "%s:        Unprotected Block Size: %dK\n", machine().describe_context(), 32 << ((data & CS_UPSIZ_MASK) >> CS_UPSIZ_SHIFT));
	LOGMASKED(LOG_CS_SEL, "%s:        Read-Only for Protected Block: %d\n", machine().describe_context(), BIT(data, CS_ROP_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Supervisor-Only for Protected Block: %d\n", machine().describe_context(), BIT(data, CS_SOP_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Read-Only: %d\n", machine().describe_context(), BIT(data, CS_RO_BIT));
	m_csb = data;
}

u16 mc68ez328_device::csb_r()
{
	LOGMASKED(LOG_CS_SEL, "%s: csb_r: CSB: %04x\n", machine().describe_context(), m_csb);
	return m_csb;
}

void mc68ez328_device::csc_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const char *const SIZ_NAMES[8] = { "32K", "64K", "128K", "256K", "512K", "1M", "2M", "4M" };
	static const char *const WS_NAMES[8] = { "None", "1", "2", "3", "4", "5", "6", "External /DTACK" };
	LOGMASKED(LOG_CS_SEL, "%s: csc_w: CSC = %04x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CS_SEL, "%s:        Enable: %d\n", machine().describe_context(), BIT(data, CS_EN_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Chip-Select Size: %s\n", machine().describe_context(), SIZ_NAMES[(data & CS_SIZ_MASK) >> CS_SIZ_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Wait States: %s\n", machine().describe_context(), WS_NAMES[(data & CS_WS_MASK) >> CS_WS_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Bus Width: %d Bits\n", machine().describe_context(), BIT(data, CS_BSW_BIT) ? 16 : 8);
	LOGMASKED(LOG_CS_SEL, "%s:        Delay /LWE and /UWE for Flash: %s\n", machine().describe_context(), BIT(data, CS_FLASH_BIT) ? "Yes" : "No");
	LOGMASKED(LOG_CS_SEL, "%s:        Unprotected Block Size: %dK\n", machine().describe_context(), 32 << ((data & CS_UPSIZ_MASK) >> CS_UPSIZ_SHIFT));
	LOGMASKED(LOG_CS_SEL, "%s:        Read-Only for Protected Block: %d\n", machine().describe_context(), BIT(data, CS_ROP_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Supervisor-Only for Protected Block: %d\n", machine().describe_context(), BIT(data, CS_SOP_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Read-Only: %d\n", machine().describe_context(), BIT(data, CS_RO_BIT));
	m_csc = data;
}

u16 mc68ez328_device::csc_r()
{
	LOGMASKED(LOG_CS_SEL, "%s: csc_r: CSC: %04x\n", machine().describe_context(), m_csc);
	return m_csc;
}

void mc68ez328_device::csd_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const char *const SIZ_NAMES[8] = { "32K", "64K", "128K", "256K", "512K", "1M", "2M", "4M" };
	static const char *const WS_NAMES[8] = { "None", "1", "2", "3", "4", "5", "6", "External /DTACK" };
	LOGMASKED(LOG_CS_SEL, "%s: csd_w: CSD = %04x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CS_SEL, "%s:        Enable: %d\n", machine().describe_context(), BIT(data, CS_EN_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Chip-Select Size: %s\n", machine().describe_context(), SIZ_NAMES[(data & CS_SIZ_MASK) >> CS_SIZ_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Wait States: %s\n", machine().describe_context(), WS_NAMES[(data & CS_WS_MASK) >> CS_WS_SHIFT]);
	LOGMASKED(LOG_CS_SEL, "%s:        Bus Width: %d Bits\n", machine().describe_context(), BIT(data, CS_BSW_BIT) ? 16 : 8);
	LOGMASKED(LOG_CS_SEL, "%s:        Delay /LWE and /UWE for Flash: %s\n", machine().describe_context(), BIT(data, CS_FLASH_BIT) ? "Yes" : "No");
	LOGMASKED(LOG_CS_SEL, "%s:        DRAM Selection: %s\n", machine().describe_context(), BIT(data, CS_DRAM_BIT) ? "Select /CAS and /RAS" : "Select /CSC[1:0] and /CSD[1:0]");
	LOGMASKED(LOG_CS_SEL, "%s:        Use /RAS0 for /RAS1: %s\n", machine().describe_context(), BIT(data, CS_COMB_BIT) ? "Yes" : "No");
	LOGMASKED(LOG_CS_SEL, "%s:        Unprotected Block Size: %dK\n", machine().describe_context(), 32 << ((data & CS_UPSIZ_MASK) >> CS_UPSIZ_SHIFT));
	LOGMASKED(LOG_CS_SEL, "%s:        Read-Only for Protected Block: %d\n", machine().describe_context(), BIT(data, CS_ROP_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Supervisor-Only for Protected Block: %d\n", machine().describe_context(), BIT(data, CS_SOP_BIT));
	LOGMASKED(LOG_CS_SEL, "%s:        Read-Only: %d\n", machine().describe_context(), BIT(data, CS_RO_BIT));
	m_csd = data;
}

u16 mc68ez328_device::csd_r()
{
	LOGMASKED(LOG_CS_SEL, "%s: csd_r: CSD: %04x\n", machine().describe_context(), m_csd);
	return m_csd;
}

void mc68ez328_device::emucs_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const char *const WS_NAMES[8] = { "None", "1", "2", "3", "4", "5", "6", "External /DTACK" };
	LOGMASKED(LOG_CS_SEL, "%s: emucs_w: EMUCS = %04x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CS_SEL, "%s:        Wait States: %s\n", machine().describe_context(), WS_NAMES[(data & CS_WS_MASK) >> CS_WS_SHIFT]);
	m_emucs = data;
}

u16 mc68ez328_device::emucs_r()
{
	LOGMASKED(LOG_CS_SEL, "%s: emucs_r: EMUCS: %04x\n", machine().describe_context(), m_emucs);
	return m_emucs;
}


//-------------------------------------------------
//  PLL/power hardware
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mc68328_base_device::refclk_tick)
{
	m_pllfsr ^= 0x8000;
}

void mc68328_base_device::pllcr_w(u16 data) // 0x200
{
	LOGMASKED(LOG_PLL, "%s: pllcr_w: PLLCR = %04x\n", machine().describe_context(), data);
	m_pllcr = data;
}

u16 mc68328_base_device::pllcr_r() // 0x200
{
	LOGMASKED(LOG_PLL, "%s: pllcr_r: PLLCR: %04x\n", machine().describe_context(), m_pllcr);
	return m_pllcr;
}

void mc68328_base_device::pllfsr_w(u16 data) // 0x202
{
	LOGMASKED(LOG_PLL, "%s: pllfsr_w: PLLFSR = %04x\n", machine().describe_context(), data);
	m_pllfsr = data;
}

u16 mc68328_base_device::pllfsr_r() // 0x202
{
	LOGMASKED(LOG_PLL, "%s: pllfsr_r: PLLFSR: %04x\n", machine().describe_context(), m_pllfsr);
	return m_pllfsr;
}

void mc68328_base_device::pctlr_w(u8 data) // 0x207
{
	LOGMASKED(LOG_PLL, "%s: pctlr_w: PCTLR = %02x\n", machine().describe_context(), data);
	m_pctlr = data;
}

u8 mc68328_base_device::pctlr_r() // 0x207
{
	LOGMASKED(LOG_PLL, "%s: pctlr_r: PCTLR: %02x\n", machine().describe_context(), m_pctlr);
	return m_pctlr;
}


//-------------------------------------------------
//  Interrupt-related hardware - standard MC68328
//-------------------------------------------------

void mc68328_base_device::update_ipr_state(u32 changed_mask)
{
	const int irq_level = get_irq_level_for_mask(changed_mask);
	const u32 irq_mask = get_irq_mask_for_level(irq_level);

	if (!irq_level || !irq_mask)
	{
		return;
	}

	if (m_ipr & changed_mask)
	{
		// If a pending interrupt has changed, it's not masked, and it's not currently in service, raise the corresponding 68k IRQ line and mark it as
		// in-service.
		if ((~m_imr & irq_mask) && !(m_gisr & changed_mask))
		{
			m_gisr |= changed_mask;
			set_input_line(irq_level, ASSERT_LINE);
		}
	}
	else
	{
		m_gisr &= ~changed_mask;

		// If there are no other pending, unmasked interrupts at this level, lower the corresponding 68k IRQ line.
		if (!(m_ipr & ~m_imr & irq_mask))
		{
			set_input_line(irq_level, CLEAR_LINE);
		}
	}
}

void mc68328_base_device::update_imr_state(u32 changed_mask)
{
	int irq_level = get_irq_level_for_mask(changed_mask);
	u32 irq_mask = get_irq_mask_for_level(irq_level);
	u32 level_mask = irq_mask & changed_mask;

	while (irq_level && irq_mask)
	{
		if (m_ipr & ~m_gisr & ~m_imr & level_mask)
		{
			// If a newly-unmasked interrupt is pending and not currently in-service, raise the relevant line.
			m_gisr |= level_mask;
			set_input_line(irq_level, ASSERT_LINE);
		}
		else if (m_gisr & m_imr & level_mask)
		{
			// If a newly-masked interrupt is in-service, lower the relevant line.
			set_input_line(irq_level, CLEAR_LINE);
		}

		changed_mask &= ~irq_mask;
		irq_level = get_irq_level_for_mask(changed_mask);
		irq_mask = get_irq_mask_for_level(irq_level);
		level_mask = irq_mask & changed_mask;
	}
}

void mc68328_base_device::set_interrupt_line(u32 line, u32 active)
{
	const u32 mask = 1 << line;

	if (active)
	{
		m_ipr |= mask;
	}
	else
	{
		m_ipr &= ~mask;
	}

	update_ipr_state(mask);
}

int mc68328_device::get_irq_level_for_mask(u32 mask)
{
	constexpr u32 IRQ_MASKS[8] =
	{
		0,
		INT_IRQ1_MASK,
		INT_IRQ2_MASK,
		INT_IRQ3_MASK,
		INT_INT0_MASK | INT_INT1_MASK | INT_INT2_MASK | INT_INT3_MASK | INT_INT4_MASK | INT_INT5_MASK | INT_INT6_MASK | INT_INT7_MASK
			| INT_PWM_MASK | INT_KB_MASK | INT_RTC_MASK | INT_WDT_MASK | INT_UART_MASK | INT_TIMER2_MASK | INT_SPIM_MASK,
		INT_IRQ5_MASK,
		INT_IRQ6_MASK | INT_TIMER1_MASK | INT_SPIS_MASK,
		INT_IRQ7_MASK
	};

	for (int level = 7; level >= 1; level--)
	{
		if (IRQ_MASKS[level] & mask)
		{
			return level;
		}
	}
	return 0;
}

u32 mc68328_device::get_irq_mask_for_level(int level)
{
	constexpr u32 IRQ_MASKS[8] =
	{
		0,
		INT_IRQ1_MASK,
		INT_IRQ2_MASK,
		INT_IRQ3_MASK,
		INT_INT0_MASK | INT_INT1_MASK | INT_INT2_MASK | INT_INT3_MASK | INT_INT4_MASK | INT_INT5_MASK | INT_INT6_MASK | INT_INT7_MASK
			| INT_PWM_MASK | INT_KB_MASK | INT_RTC_MASK | INT_WDT_MASK | INT_UART_MASK | INT_TIMER2_MASK | INT_SPIM_MASK,
		INT_IRQ5_MASK,
		INT_IRQ6_MASK | INT_TIMER1_MASK | INT_SPIS_MASK,
		INT_IRQ7_MASK
	};
	if (level >= 0 && level <= 7)
	{
		return IRQ_MASKS[level];
	}
	return 0;
}

void mc68328_base_device::irq5_w(int state)
{
	set_interrupt_line(INT_IRQ5, state);
}

u8 mc68328_base_device::irq_callback(offs_t offset)
{
	return m_ivr | offset;
}

void mc68328_base_device::ivr_w(u8 data) // 0x300
{
	LOGMASKED(LOG_INTS, "%s: ivr_w: IVR = %02x\n", machine().describe_context(), data);
	m_ivr = data;
}

u8 mc68328_base_device::ivr_r() // 0x300
{
	LOGMASKED(LOG_INTS, "%s: ivr_r: IVR: %02x\n", machine().describe_context(), m_ivr);
	return m_ivr;
}

void mc68328_base_device::icr_w(u8 data) // 0x302
{
	LOGMASKED(LOG_INTS, "%s: icr_w: ICR = %02x\n", machine().describe_context(), data);
	m_icr = data;
}

u16 mc68328_base_device::icr_r() // 0x302
{
	LOGMASKED(LOG_INTS, "%s: icr_r: ICR: %04x\n", machine().describe_context(), m_icr);
	return m_icr;
}

void mc68328_base_device::imr_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x304
{
	const u32 imr_old = m_imr;
	LOGMASKED(LOG_INTS, "%s: imr_msw_w: IMR(MSW) = %04x\n", machine().describe_context(), data);
	m_imr &= ~(mem_mask << 16);
	m_imr |= (data & mem_mask) << 16;

	update_imr_state(imr_old ^ m_imr);
}

u16 mc68328_base_device::imr_msw_r() // 0x304
{
	LOGMASKED(LOG_INTS, "%s: imr_msw_r: IMR(MSW): %04x\n", machine().describe_context(), (u16)(m_imr >> 16));
	return (u16)(m_imr >> 16);
}

void mc68328_base_device::imr_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x306
{
	const u32 imr_old = m_imr;
	LOGMASKED(LOG_INTS, "%s: imr_lsw_w: IMR(LSW) = %04x\n", machine().describe_context(), data);
	m_imr &= 0xffff0000 | (~mem_mask);
	m_imr |= data & mem_mask;

	update_imr_state(imr_old ^ m_imr);
}

u16 mc68328_base_device::imr_lsw_r() // 0x306
{
	LOGMASKED(LOG_INTS, "%s: imr_lsw_r: IMR(LSW): %04x\n", machine().describe_context(), (u16)m_imr);
	return (u16)m_imr;
}

void mc68328_device::iwr_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x308
{
	LOGMASKED(LOG_INTS, "%s: iwr_msw_w: IWR(MSW) = %04x\n", machine().describe_context(), data);
	m_iwr &= ~(mem_mask << 16);
	m_iwr |= (data & mem_mask) << 16;
}

u16 mc68328_device::iwr_msw_r() // 0x308
{
	LOGMASKED(LOG_INTS, "%s: iwr_msw_r: IWR(MSW): %04x\n", machine().describe_context(), (u16)(m_iwr >> 16));
	return (u16)(m_iwr >> 16);
}

void mc68328_device::iwr_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x30a
{
	LOGMASKED(LOG_INTS, "%s: iwr_lsw_w: IWR(LSW) = %04x\n", machine().describe_context(), data);
	m_iwr &= 0xffff0000 | (~mem_mask);
	m_iwr |= data & mem_mask;
}

u16 mc68328_device::iwr_lsw_r() // 0x30a
{
	LOGMASKED(LOG_INTS, "%s: iwr_lsw_r: IWR(LSW): %04x\n", machine().describe_context(), (u16)m_iwr);
	return (u16)m_iwr;
}

void mc68328_base_device::isr_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x30c
{
	LOGMASKED(LOG_INTS, "%s: isr_msw_w: ISR(MSW) = %04x\n", machine().describe_context(), data);
	// Clear edge-triggered IRQ1
	if ((m_icr & ICR_ET1) == ICR_ET1 && ((data << 16) & INT_IRQ1_MASK) == INT_IRQ1_MASK)
	{
		m_gisr &= ~INT_IRQ1_MASK;
	}

	// Clear edge-triggered IRQ2
	if ((m_icr & ICR_ET2) == ICR_ET2 && ((data << 16) & INT_IRQ2_MASK) == INT_IRQ2_MASK)
	{
		m_gisr &= ~INT_IRQ2_MASK;
	}

	// Clear edge-triggered IRQ3
	if ((m_icr & ICR_ET3) == ICR_ET3 && ((data << 16) & INT_IRQ3_MASK) == INT_IRQ3_MASK)
	{
		m_gisr &= ~INT_IRQ3_MASK;
	}

	// Clear edge-triggered IRQ6
	if ((m_icr & ICR_ET6) == ICR_ET6 && ((data << 16) & INT_IRQ6_MASK) == INT_IRQ6_MASK)
	{
		m_gisr &= ~INT_IRQ6_MASK;
	}

	// Clear edge-triggered IRQ7
	if (((data << 16) & INT_IRQ7_MASK) == INT_IRQ7_MASK)
	{
		m_gisr &= ~INT_IRQ7_MASK;
	}
}

u16 mc68328_base_device::isr_msw_r() // 0x30c
{
	LOGMASKED(LOG_INTS, "%s: isr_msw_r: ISR(MSW): %04x\n", machine().describe_context(), (u16)(m_gisr >> 16));
	return (u16)(m_gisr >> 16);
}

void mc68328_base_device::isr_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x30e
{
	LOGMASKED(LOG_INTS, "%s: isr_lsw_w: ISR(LSW) = %04x (Ignored)\n", machine().describe_context(), data);
}

u16 mc68328_base_device::isr_lsw_r() // 0x30e
{
	LOGMASKED(LOG_INTS, "%s: isr_lsw_r: ISR(LSW): %04x\n", machine().describe_context(), (u16)m_gisr);
	return (u16)m_gisr;
}

void mc68328_base_device::ipr_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0x310
{
	LOGMASKED(LOG_INTS, "%s: ipr_msw_w: IPR(MSW) = %04x (Ignored)\n", machine().describe_context(), data);
}

u16 mc68328_base_device::ipr_msw_r() // 0x310
{
	LOGMASKED(LOG_INTS, "%s: ipr_msw_r: IPR(MSW): %04x\n", machine().describe_context(), (u16)(m_ipr >> 16));
	return (u16)(m_ipr >> 16);
}

void mc68328_base_device::ipr_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0x312
{
	LOGMASKED(LOG_INTS, "%s: ipr_lsw_w: IPR(LSW) = %04x (Ignored)\n", machine().describe_context(), data);
}

u16 mc68328_base_device::ipr_lsw_r() // 0x312
{
	LOGMASKED(LOG_INTS, "%s: ipr_lsw_r: IPR(LSW): %04x\n", machine().describe_context(), (u16)m_ipr);
	return (u16)m_ipr;
}


//-------------------------------------------------
//  Interrupt-related hardware - EZ variant
//-------------------------------------------------

int mc68ez328_device::get_irq_level_for_mask(u32 mask)
{
	constexpr u32 IRQ_MASKS[8] =
	{
		0,
		INT_IRQ1_MASK,
		INT_IRQ2_MASK,
		INT_IRQ3_MASK,
		INT_INT0_MASK | INT_INT1_MASK | INT_INT2_MASK | INT_INT3_MASK | INT_INT4_MASK | INT_INT5_MASK | INT_INT6_MASK | INT_INT7_MASK
			| INT_KB_MASK | INT_RTC_MASK | INT_WDT_MASK | INT_UART_MASK | INT_SPIM_MASK,
		INT_IRQ5_MASK,
		INT_PWM_MASK | INT_TIMER2_MASK | INT_SPIS_MASK,
		INT_MEMIQ_MASK,
	};

	for (int level = 1; level <= 7; level++)
	{
		if (IRQ_MASKS[level] & mask)
		{
			return level;
		}
	}
	return 0;
}

u32 mc68ez328_device::get_irq_mask_for_level(int level)
{
	constexpr u32 IRQ_MASKS[8] =
	{
		0,
		INT_IRQ1_MASK,
		INT_IRQ2_MASK,
		INT_IRQ3_MASK,
		INT_INT0_MASK | INT_INT1_MASK | INT_INT2_MASK | INT_INT3_MASK | INT_INT4_MASK | INT_INT5_MASK | INT_INT6_MASK | INT_INT7_MASK
			| INT_KB_MASK | INT_RTC_MASK | INT_WDT_MASK | INT_UART_MASK | INT_SPIM_MASK,
		INT_IRQ5_MASK,
		INT_PWM_MASK | INT_TIMER2_MASK | INT_SPIS_MASK,
		INT_MEMIQ_MASK,
	};
	if (level >= 0 && level <= 7)
	{
		return IRQ_MASKS[level];
	}
	return 0;
}


//-------------------------------------------------
//  GPIO hardware - Port A
//-------------------------------------------------

void mc68328_base_device::padir_w(u8 data) // 0x400
{
	LOGMASKED(LOG_GPIO_A, "%s: padir_w: PADIR = %02x\n", machine().describe_context(), data);
	m_padir = data;
}

u8 mc68328_base_device::padir_r() // 0x400
{
	LOGMASKED(LOG_GPIO_A, "%s: mc68328_r: PADIR: %02x\n", machine().describe_context(), m_padir);
	return m_padir;
}

void mc68328_base_device::padata_w(u8 data) // 0x401
{
	LOGMASKED(LOG_GPIO_A, "%s: padata_w: PADATA = %02x\n", machine().describe_context(), data);
	m_padata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_padir & m_pasel, i))
		{
			m_out_port_a_cb[i](BIT(m_padata, i));
		}
	}
}

u8 mc68328_base_device::padata_r() // 0x401
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pasel, i))
		{
			if (BIT(m_padir, i))
			{
				data |= m_padata & (1 << i);
			}
			else if (!m_in_port_a_cb[i].isunset())
			{
				data |= m_in_port_a_cb[i]() << i;
			}
		}
	}
	LOGMASKED(LOG_GPIO_A, "%s: padata_r: PADATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_device::pasel_w(u8 data) // 0x403
{
	LOGMASKED(LOG_GPIO_A, "%s: pasel_w: PASEL = %02x\n", machine().describe_context(), data);
	m_pasel = data;
}

u8 mc68328_device::pasel_r() // 0x403
{
	LOGMASKED(LOG_GPIO_A, "%s: mc68328_r: PASEL: %02x\n", machine().describe_context(), m_pasel);
	return m_pasel;
}


//-------------------------------------------------
//  GPIO hardware - Port B
//-------------------------------------------------

void mc68328_base_device::pbdir_w(u8 data) // 0x408
{
	LOGMASKED(LOG_GPIO_B, "%s: pbdir_w: PBDIR = %02x\n", machine().describe_context(), data);
	m_pbdir = data;
}

u8 mc68328_base_device::pbdir_r() // 0x408
{
	LOGMASKED(LOG_GPIO_B, "%s: pbdir_r: PBDIR: %02x\n", machine().describe_context(), m_pbdir);
	return m_pbdir;
}

void mc68328_base_device::pbdata_w(u8 data) // 0x409
{
	LOGMASKED(LOG_GPIO_B, "%s: pbdata_w: PBDATA = %02x (outputing %02x)\n", machine().describe_context(), data, data & m_pbdir & m_pbsel);
	m_pbdata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pbdir & m_pbsel, i))
		{
			m_out_port_b_cb[i](BIT(m_pbdata, i));
		}
	}
}

u8 mc68328_base_device::pbdata_r() // 0x409
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pbsel, i))
		{
			if (BIT(m_pbdir, i))
			{
				data |= m_pbdata & (1 << i);
			}
			else if (!m_in_port_b_cb[i].isunset())
			{
				data |= m_in_port_b_cb[i]() << i;
			}
		}
	}
	LOGMASKED(LOG_GPIO_B, "%s: pbdata_r: PBDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_base_device::pbsel_w(u8 data) // 0x40b
{
	LOGMASKED(LOG_GPIO_B, "%s: pbsel_w: PBSEL = %02x\n", machine().describe_context(), data);
	m_pbsel = data;
}

u8 mc68328_base_device::pbsel_r() // 0x40b
{
	LOGMASKED(LOG_GPIO_B, "%s: pbsel_r: PBSEL: %02x\n", machine().describe_context(), m_pbsel);
	return m_pbsel;
}


//-------------------------------------------------
//  GPIO hardware - Port C
//-------------------------------------------------

void mc68328_base_device::pcdir_w(u8 data) // 0x410
{
	LOGMASKED(LOG_GPIO_C, "%s: pcdir_w: PCDIR = %02x\n", machine().describe_context(), data);
	m_pcdir = data;
}

u8 mc68328_base_device::pcdir_r() // 0x410
{
	LOGMASKED(LOG_GPIO_C, "%s: pcdir_r: PCDIR: %02x\n", machine().describe_context(), m_pcdir);
	return m_pcdir;
}

void mc68328_base_device::pcdata_w(u8 data) // 0x411
{
	LOGMASKED(LOG_GPIO_C, "%s: pcdata_w: PCDATA = %02x (outputing %02x)\n", machine().describe_context(), data, data & m_pcdir & m_pcsel);
	m_pcdata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pcdir & m_pcsel, i))
		{
			m_out_port_c_cb[i](BIT(m_pcdata, i));
		}
		else if (BIT(~m_pcdir & m_pcsel, i))
		{
			m_out_port_c_cb[i](1);
		}
	}
}

u8 mc68328_base_device::pcdata_r() // 0x411
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pcsel, i))
		{
			if (BIT(m_pcdir, i))
			{
				data |= m_pcdata & (1 << i);
			}
			else if (!m_in_port_c_cb[i].isunset())
			{
				data |= m_in_port_c_cb[i]() << i;
			}
		}
	}
	LOGMASKED(LOG_GPIO_C, "%s: pcdata_r: PCDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_base_device::pcsel_w(u8 data) // 0x413
{
	LOGMASKED(LOG_GPIO_C, "%s: pcsel_w: PCSEL = %02x\n", machine().describe_context(), data);
	m_pcsel = data;
}

u8 mc68328_base_device::pcsel_r() // 0x413
{
	LOGMASKED(LOG_GPIO_C, "%s: pcsel_r: PCSEL: %02x\n", machine().describe_context(), m_pcsel);
	return m_pcsel;
}


//-------------------------------------------------
//  GPIO hardware - Port D
//-------------------------------------------------

void mc68328_base_device::port_d_in_w(int state, int bit)
{
	const u8 old_state = m_pdindata;
	m_pdindata &= ~(1 << bit);
	m_pdindata |= state << bit;

	// If no bit has changed state, there's nothing to do.
	if (old_state == m_pdindata)
	{
		return;
	}

	// If we're not edge-triggered, handle potential level-sensitive interrupts.
	if (!BIT(m_pdirqedge, bit))
	{
		// If the new state is low while PDPOL is active-low (0) or vice-versa, assert the interrupt for this bit.
		// Otherwise, clear it.
		set_interrupt_line(INT_KBDINTS + bit, (int)(state == BIT(m_pdpol, bit)));
		return;
	}

	set_interrupt_line(INT_KBDINTS + bit, 1);
}

void mc68328_base_device::pddir_w(u8 data) // 0x418
{
	LOGMASKED(LOG_GPIO_D, "%s: pddir_w: PDDIR = %02x\n", machine().describe_context(), data);
	m_pddir = data;
}

u8 mc68328_base_device::pddir_r() // 0x418
{
	LOGMASKED(LOG_GPIO_D, "%s: pddir_r: PDDIR: %02x\n", machine().describe_context(), m_pddir);
	return m_pddir;
}

void mc68328_base_device::pddata_w(u8 data) // 0x419
{
	LOGMASKED(LOG_GPIO_D, "%s: pddata_w: PDDATA = %02x (outputing %02x)\n", machine().describe_context(), data, data & m_pddir);
	m_pddata = data;

	for (int bit = 0; bit < 4; bit++)
	{
		if (BIT(m_pdirqedge & data, bit))
		{
			set_interrupt_line(INT_KBDINTS + bit, 0);
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pddir, i))
		{
			m_out_port_d_cb[i](BIT(m_pddata, i));
		}
	}
}

u8 mc68328_base_device::pddata_r() // 0x419
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pddir, i))
		{
			data |= m_pddata & (1 << i);
		}
		else if (!m_in_port_d_cb[i].isunset())
		{
			data |= m_in_port_d_cb[i]() << i;
		}
		else
		{
			data |= m_pdpuen & (1 << i);
		}
	}
	LOGMASKED(LOG_GPIO_D, "%s: pddata_r: PDDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_base_device::pdpuen_w(u8 data) // 0x41a
{
	LOGMASKED(LOG_GPIO_D, "%s: pdpuen_w: PDPUEN = %02x\n", machine().describe_context(), data);
	m_pdpuen = data;
}

u8 mc68328_base_device::pdpuen_r() // 0x41a
{
	LOGMASKED(LOG_GPIO_D, "%s: pdpuen_r: PDPUEN: %02x\n", machine().describe_context(), m_pdpuen);
	return m_pdpuen;
}

void mc68328_base_device::pdpol_w(u8 data) // 0x41c
{
	LOGMASKED(LOG_GPIO_D, "%s: pdpol_w: PDPOL = %02x\n", machine().describe_context(), data);
	m_pdpol = data;
}

u8 mc68328_base_device::pdpol_r() // 0x41c
{
	LOGMASKED(LOG_GPIO_D, "%s: pdpol_r: PDPOL: %02x\n", machine().describe_context(), m_pdpol);
	return m_pdpol;
}

void mc68328_base_device::pdirqen_w(u8 data) // 0x41d
{
	LOGMASKED(LOG_GPIO_D, "%s: pdirqen_w: PDIRQEN = %02x\n", machine().describe_context(), data);
	m_pdirqen = data & 0x00ff;
}

u8 mc68328_base_device::pdirqen_r() // 0x41d
{
	LOGMASKED(LOG_GPIO_D, "%s: pdirqen_r: PDIRQEN: %02x\n", machine().describe_context(), m_pdirqen);
	return m_pdirqen;
}

void mc68328_base_device::pdirqedge_w(u8 data) // 0x41f
{
	LOGMASKED(LOG_GPIO_D, "%s: pdirqedge_w: PDIRQEDGE = %02x\n", machine().describe_context(), data);
	m_pdirqedge = data;
}

u8 mc68328_base_device::pdirqedge_r() // 0x41f
{
	LOGMASKED(LOG_GPIO_D, "%s: pdirqedge_r: PDIRQEDGE: %02x\n", machine().describe_context(), m_pdirqedge);
	return m_pdirqedge;
}


//-------------------------------------------------
//  GPIO hardware - Port E
//-------------------------------------------------

void mc68328_base_device::pedir_w(u8 data) // 0x420
{
	LOGMASKED(LOG_GPIO_E, "%s: pedir_w: PEDIR = %02x\n", machine().describe_context(), data);
	m_pedir = data;
}

u8 mc68328_base_device::pedir_r() // 0x420
{
	LOGMASKED(LOG_GPIO_E, "%s: pedir_r: PEDIR: %02x\n", machine().describe_context(), m_pedir);
	return m_pedir;
}

void mc68328_base_device::pedata_w(u8 data) // 0x421
{
	LOGMASKED(LOG_GPIO_E, "%s: pedata_w: PEDATA = %02x (outputing %02x)\n", machine().describe_context(), data, data & m_pedir & m_pesel);
	m_pedata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pedir & m_pesel, i))
		{
			m_out_port_e_cb[i](BIT(m_pedata, i));
		}
	}
}

u8 mc68328_base_device::pedata_r() // 0x421
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pesel, i))
		{
			if (BIT(m_pedir, i))
			{
				data |= m_pedata & (1 << i);
			}
			else if (!m_in_port_e_cb[i].isunset())
			{
				data |= m_in_port_e_cb[i]() << i;
			}
			else
			{
				data |= m_pepuen & (1 << i);
			}
		}
	}
	LOGMASKED(LOG_GPIO_E, "%s: pedata_r: PEDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_base_device::pepuen_w(u8 data) // 0x422
{
	LOGMASKED(LOG_GPIO_E, "%s: pepuen_w: PEPUEN = %02x\n", machine().describe_context(), data);
	m_pepuen = data;
}

u8 mc68328_base_device::pepuen_r() // 0x422
{
	LOGMASKED(LOG_GPIO_E, "%s: pepuen_r: PEPUEN: %02x\n", machine().describe_context(), m_pepuen);
	return m_pepuen;
}

void mc68328_base_device::pesel_w(u8 data) // 0x423
{
	LOGMASKED(LOG_GPIO_E, "%s: pesel_w: PESEL = %02x\n", machine().describe_context(), data);
	m_pesel = data;
}

u8 mc68328_base_device::pesel_r() // 0x423
{
	LOGMASKED(LOG_GPIO_E, "%s: pesel_r: PESEL: %02x\n", machine().describe_context(), m_pesel);
	return m_pesel;
}


//-------------------------------------------------
//  GPIO hardware - Port F
//-------------------------------------------------

void mc68328_base_device::pfdir_w(u8 data) // 0x428
{
	LOGMASKED(LOG_GPIO_F, "%s: pfdir_w: PFDIR = %02x\n", machine().describe_context(), data);
	m_pfdir = data;
}

u8 mc68328_base_device::pfdir_r() // 0x428
{
	LOGMASKED(LOG_GPIO_F, "%s: pfdir_r: PFDIR: %02x\n", machine().describe_context(), m_pfdir);
	return m_pfdir;
}

void mc68328_base_device::pfdata_w(u8 data) // 0x429
{
	LOGMASKED(LOG_GPIO_F, "%s: pfdata_w: PFDATA = %02x (outputing %02x)\n", machine().describe_context(), data, data & m_pfdir & m_pfsel);
	m_pfdata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pfdir & m_pfsel, i))
		{
			m_out_port_f_cb[i](BIT(m_pfdata, i));
		}
	}
}

u8 mc68328_base_device::pfdata_r() // 0x429
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pfsel, i))
		{
			if (BIT(m_pfdir, i))
			{
				data |= m_pfdata & (1 << i);
			}
			else if (!m_in_port_f_cb[i].isunset())
			{
				data |= m_in_port_f_cb[i]() << i;
			}
			else
			{
				data |= m_pfpuen & (1 << i);
			}
		}
	}
	LOGMASKED(LOG_GPIO_F, "%s: pfdata_r: PFDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_base_device::pfpuen_w(u8 data) // 0x42a
{
	LOGMASKED(LOG_GPIO_F, "%s: pfpuen_w: PFPUEN = %02x\n", machine().describe_context(), data);
	m_pfpuen = data;
}

u8 mc68328_base_device::pfpuen_r() // 0x42a
{
	LOGMASKED(LOG_GPIO_F, "%s: pfpuen_r: PFPUEN: %02x\n", machine().describe_context(), m_pfpuen);
	return m_pfpuen;
}

void mc68328_base_device::pfsel_w(u8 data) // 0x42b
{
	LOGMASKED(LOG_GPIO_F, "%s: pfsel_w: PFSEL = %02x\n", machine().describe_context(), data);
	m_pfsel = data;
}

u8 mc68328_base_device::pfsel_r() // 0x42b
{
	LOGMASKED(LOG_GPIO_F, "%s: pfsel_r: PFSEL: %02x\n", machine().describe_context(), m_pfsel);
	return m_pfsel;
}


//-------------------------------------------------
//  GPIO hardware - Port G
//-------------------------------------------------

void mc68328_base_device::pgdir_w(u8 data) // 0x430
{
	LOGMASKED(LOG_GPIO_G, "%s: pgdir_w: PGDIR = %02x\n", machine().describe_context(), data);
	m_pgdir = data;
}

u8 mc68328_base_device::pgdir_r() // 0x430
{
	LOGMASKED(LOG_GPIO_G, "%s: pgdir_r: PGDIR: %02x\n", machine().describe_context(), m_pgdir);
	return m_pgdir;
}

void mc68328_base_device::pgdata_w(u8 data) // 0x431
{
	LOGMASKED(LOG_GPIO_G, "%s: pgdata_w: PGDATA = %02x (outputing %02x)\n", machine().describe_context(), data, data & m_pgdir & m_pgsel);
	m_pgdata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pgdir & m_pgsel, i))
		{
			m_out_port_g_cb[i](BIT(m_pgdata, i));
		}
	}
}

u8 mc68328_base_device::pgdata_r() // 0x431
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pgsel, i))
		{
			if (BIT(m_pgdir, i))
			{
				data |= m_pgdata & (1 << i);
			}
			else if (!m_in_port_g_cb[i].isunset())
			{
				data |= m_in_port_g_cb[i]() << i;
			}
			else
			{
				data |= m_pgpuen & (1 << i);
			}
		}
	}
	LOGMASKED(LOG_GPIO_G, "%s: pgdata_r: PGDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_base_device::pgpuen_w(u8 data) // 0x432
{
	LOGMASKED(LOG_GPIO_G, "%s: pgpuen_w: PGPUEN = %02x\n", machine().describe_context(), data);
	m_pgpuen = data;
}

u8 mc68328_base_device::pgpuen_r() // 0x432
{
	LOGMASKED(LOG_GPIO_G, "%s: pgpuen_r: PGPUEN: %02x\n", machine().describe_context(), m_pgpuen);
	return m_pgpuen;
}

void mc68328_base_device::pgsel_w(u8 data) // 0x433
{
	LOGMASKED(LOG_GPIO_G, "%s: pgsel_w: PGSEL = %02x\n", machine().describe_context(), data);
	m_pgsel = data;
}

u8 mc68328_base_device::pgsel_r() // 0x433
{
	LOGMASKED(LOG_GPIO_G, "%s: pgsel_r: PGSEL: %02x\n", machine().describe_context(), m_pgsel);
	return m_pgsel;
}


//-------------------------------------------------
//  GPIO hardware - Port J
//-------------------------------------------------

void mc68328_device::pjdir_w(u8 data) // 0x438
{
	LOGMASKED(LOG_GPIO_J, "%s: pjdir_w: PJDIR = %02x\n", machine().describe_context(), data);
	m_pjdir = data;
}

u8 mc68328_device::pjdir_r() // 0x438
{
	LOGMASKED(LOG_GPIO_J, "%s: pjdir_r: PJDIR: %02x\n", machine().describe_context(), m_pjdir);
	return m_pjdir;
}

void mc68328_device::pjdata_w(u8 data) // 0x439
{
	LOGMASKED(LOG_GPIO_J, "%s: pjdata_w: PJDATA = %02x\n", machine().describe_context(), data);
	m_pjdata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pjdir & m_pjsel, i))
		{
			m_out_port_j_cb[i](BIT(m_pjdata, i));
		}
	}
}

u8 mc68328_device::pjdata_r() // 0x439
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pjsel, i))
		{
			if (BIT(m_pjdir, i))
			{
				data |= m_pjdata & (1 << i);
			}
			else if (!m_in_port_j_cb[i].isunset())
			{
				data |= m_in_port_j_cb[i]() << i;
			}
		}
	}
	LOGMASKED(LOG_GPIO_J, "%s: pjdata_r: PJDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_device::pjsel_w(u8 data) // 0x43b
{
	LOGMASKED(LOG_GPIO_J, "%s: pjsel_w: PJSEL = %02x\n", machine().describe_context(), data);
	m_pjsel = data;
}

u8 mc68328_device::pjsel_r() // 0x43b
{
	LOGMASKED(LOG_GPIO_J, "%s: pjsel_r: PJSEL: %02x\n", machine().describe_context(), m_pjsel);
	return m_pjsel;
}


//-------------------------------------------------
//  GPIO hardware - Port K
//-------------------------------------------------

void mc68328_device::pkdir_w(u8 data) // 0x440
{
	LOGMASKED(LOG_GPIO_K, "%s: pkdir_w: PKDIR = %02x\n", machine().describe_context(), data);
	m_pkdir = data;
}

u8 mc68328_device::pkdir_r() // 0x440
{
	LOGMASKED(LOG_GPIO_K, "%s: pkdir_r: PKDIR: %02x\n", machine().describe_context(), m_pkdir);
	return m_pkdir;
}

void mc68328_device::pkdata_w(u8 data) // 0x441
{
	LOGMASKED(LOG_GPIO_K, "%s: pkdata_w: PKDATA = %02x\n", machine().describe_context(), data);
	m_pkdata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pkdir & m_pksel, i))
		{
			m_out_port_k_cb[i](BIT(m_pkdata, i));
		}
	}
}

u8 mc68328_device::pkdata_r() // 0x441
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pksel, i))
		{
			if (BIT(m_pkdir, i))
			{
				data |= m_pkdata & (1 << i);
			}
			else if (!m_in_port_k_cb[i].isunset())
			{
				data |= m_in_port_k_cb[i]() << i;
			}
			else
			{
				data |= m_pkpuen & (1 << i);
			}
		}
	}
	LOGMASKED(LOG_GPIO_K, "%s: pkdata_r: PKDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_device::pkpuen_w(u8 data) // 0x442
{
	LOGMASKED(LOG_GPIO_K, "%s: pkpuen_w: PKPUEN = %02x\n", machine().describe_context(), data);
	m_pkpuen = data;
}

u8 mc68328_device::pkpuen_r() // 0x442
{
	LOGMASKED(LOG_GPIO_K, "%s: pkpuen_r: PKPUEN: %02x\n", machine().describe_context(), m_pkpuen);
	return m_pkpuen;
}

void mc68328_device::pksel_w(u8 data) // 0x443
{
	LOGMASKED(LOG_GPIO_K, "%s: pksel_w: PKSEL = %02x\n", machine().describe_context(), data);
	m_pksel = data;
}

u8 mc68328_device::pksel_r() // 0x443
{
	LOGMASKED(LOG_GPIO_K, "%s: pksel_r: PKSEL: %02x\n", machine().describe_context(), m_pksel);
	return m_pksel;
}


//-------------------------------------------------
//  GPIO hardware - Port M
//-------------------------------------------------

void mc68328_device::pmdir_w(u8 data) // 0x448
{
	LOGMASKED(LOG_GPIO_M, "%s: pmdir_w: PMDIR = %02x\n", machine().describe_context(), data);
	m_pmdir = data;
}

u8 mc68328_device::pmdir_r() // 0x448
{
	LOGMASKED(LOG_GPIO_M, "%s: pmdir_r: PMDIR: %02x\n", machine().describe_context(), m_pmdir);
	return m_pmdir;
}

void mc68328_device::pmdata_w(u8 data) // 0x449
{
	LOGMASKED(LOG_GPIO_M, "%s: pmdata_w: PMDATA = %02x\n", machine().describe_context(), data);
	m_pmdata = data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pmdir & m_pmsel, i))
		{
			m_out_port_m_cb[i](BIT(m_pmdata, i));
		}
	}
}

u8 mc68328_device::pmdata_r() // 0x449
{
	u8 data = 0;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_pmsel, i))
		{
			if (BIT(m_pmdir, i))
			{
				data |= m_pmdata & (1 << i);
			}
			else if (!m_in_port_m_cb[i].isunset())
			{
				data |= m_in_port_m_cb[i]() << i;
			}
			else
			{
				data |= m_pmpuen & (1 << i);
			}
		}
	}
	LOGMASKED(LOG_GPIO_M, "%s: pmdata_r: PMDATA: %02x\n", machine().describe_context(), data);
	return data;
}

void mc68328_device::pmpuen_w(u8 data) // 0x44a
{
	LOGMASKED(LOG_GPIO_M, "%s: pmpuen_w: PMPUEN = %02x\n", machine().describe_context(), data);
	m_pmpuen = data;
}

u8 mc68328_device::pmpuen_r() // 0x44a
{
	LOGMASKED(LOG_GPIO_M, "%s: pmpuen_r: PMPUEN: %02x\n", machine().describe_context(), m_pmpuen);
	return m_pmpuen;
}

void mc68328_device::pmsel_w(u8 data) // 0x44b
{
	LOGMASKED(LOG_GPIO_M, "%s: pmsel_w: PMSEL = %02x\n", machine().describe_context(), data);
	m_pmsel = data;
}

u8 mc68328_device::pmsel_r() // 0x44b
{
	LOGMASKED(LOG_GPIO_M, "%s: pmsel_r: PMSEL: %02x\n", machine().describe_context(), m_pmsel);
	return m_pmsel;
}


//-------------------------------------------------
//  PWM hardware - Standard MC68328
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mc68328_device::pwm_tick)
{
	if (m_pwmw >= m_pwmp || !m_pwmw || !m_pwmp)
	{
		m_pwm->adjust(attotime::never);
		return;
	}

	const u32 divisor = 4 << (m_pwmc & PWMC_CLKSEL);

	if (((m_pwmc & PWMC_POL) == 0 && (m_pwmc & PWMC_PIN) != 0) ||
		((m_pwmc & PWMC_POL) != 0 && (m_pwmc & PWMC_PIN) == 0))
	{
		attotime period = attotime::from_ticks((m_pwmp - m_pwmw) * divisor, clock());
		m_pwm->adjust(period);

		if (m_pwmc & PWMC_IRQ_EN)
		{
			set_interrupt_line(INT_PWM, 1);
		}
	}
	else
	{
		attotime period = attotime::from_ticks(m_pwmw * divisor, clock());
		m_pwm->adjust(period);
	}

	m_pwmc ^= PWMC_PIN;

	m_out_pwm_cb((m_pwmc & PWMC_PIN) ? 1 : 0);
}

void mc68328_device::pwmc_w(u16 data) // 0x500
{
	LOGMASKED(LOG_PWM, "%s: pwmc_w: PWMC = %04x\n", machine().describe_context(), data);

	const u16 old_pwmc = m_pwmc;
	m_pwmc = data;
	const u16 changed = m_pwmc ^ old_pwmc;

	if (m_pwmc & PWMC_IRQ)
	{
		set_interrupt_line(INT_PWM, 1);
	}

	if (changed & (PWMC_EN | PWMC_CLKSEL | PWMC_LOAD))
	{
		const bool enable_or_update = (changed & m_pwmc & PWMC_EN) || (m_pwmc & PWMC_LOAD);
		if (enable_or_update && m_pwmw && m_pwmp)
		{
			const u32 divisor = 4 << (m_pwmc & PWMC_CLKSEL);
			attotime period = attotime::from_ticks(m_pwmw * divisor, clock());
			m_pwm->adjust(period);
			if (m_pwmc & PWMC_IRQ_EN)
			{
				set_interrupt_line(INT_PWM, 1);
			}
			m_pwmc |= PWMC_PIN;
		}
		else
		{
			m_pwm->adjust(attotime::never);
		}
	}

	m_pwmc &= ~PWMC_LOAD;
}

u16 mc68328_device::pwmc_r() // 0x500
{
	const u16 data = m_pwmc;
	LOGMASKED(LOG_PWM, "%s: pwmc_r: PWMC: %04x\n", machine().describe_context(), data);
	if (m_pwmc & PWMC_IRQ)
	{
		m_pwmc &= ~PWMC_IRQ;
		if (m_pwmc & PWMC_IRQ_EN)
		{
			set_interrupt_line(INT_PWM, 0);
		}
	}
	return data;
}

void mc68328_device::pwmp_w(u16 data) // 0x502
{
	LOGMASKED(LOG_PWM, "%s: pwmp_w: PWMP = %04x\n", machine().describe_context(), data);
	m_pwmp = data;
}

u16 mc68328_device::pwmp_r() // 0x502
{
	LOGMASKED(LOG_PWM, "%s: pwmp_r: PWMP: %04x\n", machine().describe_context(), m_pwmp);
	return m_pwmp;
}

void mc68328_device::pwmw_w(u16 data) // 0x504
{
	LOGMASKED(LOG_PWM, "%s: pwmw_w: PWMW = %04x\n", machine().describe_context(), data);
	m_pwmw = data;
}

u16 mc68328_device::pwmw_r() // 0x504
{
	LOGMASKED(LOG_PWM, "%s: pwmw_r: PWMW: %04x\n", machine().describe_context(), m_pwmw);
	return m_pwmw;
}

void mc68328_device::pwmcnt_w(u16 data) // 0x506
{
	LOGMASKED(LOG_PWM, "%s: pwmcnt_w: PWMCNT = %04x\n", machine().describe_context(), data);
	m_pwmcnt = 0;
}

u16 mc68328_device::pwmcnt_r() // 0x506
{
	LOGMASKED(LOG_PWM, "%s: pwmcnt_r: PWMCNT: %04x\n", machine().describe_context(), m_pwmcnt);
	return m_pwmcnt;
}


//-------------------------------------------------
//  PWM hardware - EZ variant
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mc68ez328_device::pwm_tick)
{
	if (!(m_pwmc & PWMC_EN))
	{
		return;
	}

	if (!param)
	{
		m_pwm_rep_cnt--;
		if (!m_pwm_rep_cnt)
		{
			m_pwm_rep_cnt = 1 << ((m_pwmc & PWMC_REPEAT) >> PWMC_REPEAT_SHIFT);
			pwm_fifo_pop();
		}
	}

	m_pwmo = param;
	update_pwm_period(!m_pwmo);
	m_out_pwm_cb((int)m_pwmo);
}

void mc68ez328_device::update_pwm_period(bool high_cycle)
{
	const u32 frequency = (m_pwmc & PWMC_CLK_SRC) ? 32768 : clock();
	const u32 prescale = ((m_pwmc & PWMC_PRESCALE) >> PWMC_PRESCALE_SHIFT) + 1;
	const u32 divisor = 2 << (m_pwmc & PWMC_CLKSEL);
	const u32 period_reg = (u32)std::min(m_pwmp + 1u, 0xffu);
	u32 sample_period = period_reg;
	if (m_pwmfifo[m_pwmfifo_rd] <= period_reg)
	{
		sample_period = high_cycle ? (period_reg - m_pwmfifo[m_pwmfifo_rd]) : m_pwmfifo[m_pwmfifo_rd];
		sample_period++;
	}
	attotime period = attotime::from_ticks(prescale * divisor * sample_period, frequency);
	m_pwm->adjust(period, (int)high_cycle);
}

void mc68ez328_device::pwm_fifo_push(u8 data)
{
	if (m_pwmfifo_cnt >= std::size(m_pwmfifo))
	{
		return;
	}
	m_pwmfifo[m_pwmfifo_wr] = data;
	m_pwmfifo_wr = (m_pwmfifo_wr + 1) % std::size(m_pwmfifo);
	const u8 old_cnt = m_pwmfifo_cnt;
	m_pwmfifo_cnt++;
	if (m_pwmfifo_cnt == std::size(m_pwmfifo))
	{
		m_pwmc &= ~PWMC_FIFO_AV;
	}
	else if (old_cnt <= 1 && m_pwmfifo_cnt > 1)
	{
		if (m_pwmc & PWMC_IRQ)
		{
			m_pwmc &= ~PWMC_IRQ;
			set_interrupt_line(INT_PWM, 0);
		}
	}
}

void mc68ez328_device::pwm_fifo_pop()
{
	if (m_pwmfifo_cnt > 0)
	{
		m_pwmfifo_rd = (m_pwmfifo_rd + 1) % std::size(m_pwmfifo);
		m_pwmfifo_cnt--;

		m_pwmc |= PWMC_FIFO_AV;
	}

	if (m_pwmfifo_cnt <= 1)
	{
		if (!(m_pwmc & PWMC_IRQ))
		{
			m_pwmc |= PWMC_IRQ;
			set_interrupt_line(INT_PWM, 1);
		}
	}
}

void mc68ez328_device::pwmc_w(u16 data) // 0x500
{
	LOGMASKED(LOG_PWM, "%s: pwmc_w: PWMC = %04x\n", machine().describe_context(), data);

	const u16 old = m_pwmc;
	m_pwmc = (m_pwmc & PWMC_FIFO_AV) | (data & ~PWMC_FIFO_AV);
	const u16 changed = old ^ m_pwmc;
	if (!changed)
	{
		return;
	}

	bool set_irq = false;
	if (m_pwmc & PWMC_IRQ_EN)
	{
		if (m_pwmc & PWMC_IRQ)
		{
			set_irq = true;
		}
	}
	else if (old & PWMC_IRQ_EN)
	{
		set_interrupt_line(INT_PWM, 0);
	}

	const bool recalculate = (changed & PWMC_RECALC_MASK);
	if (recalculate)
	{
		if (m_pwmc & PWMC_EN)
		{
			if (changed & PWMC_EN)
			{
				set_irq = true;
				m_pwmc |= PWMC_FIFO_AV;
			}
			m_pwmo = true;
			m_out_pwm_cb((int)m_pwmo);
			update_pwm_period(false);
			m_pwm_rep_cnt = 1 << ((m_pwmc & PWMC_REPEAT) >> PWMC_REPEAT_SHIFT);
		}
		else
		{
			m_pwmfifo_cnt = 0;
			m_pwmo = false;
			m_pwm->adjust(attotime::never);
		}
	}

	if (set_irq && (m_pwmc & PWMC_IRQ_EN))
	{
		m_pwmc |= PWMC_IRQ;
		set_interrupt_line(INT_PWM, BIT(m_pwmc, PWMC_IRQ_BIT));
	}
}

u16 mc68ez328_device::pwmc_r() // 0x500
{
	const u16 data = m_pwmc;
	LOGMASKED(LOG_PWM, "%s: pwmc_r: PWMC: %04x\n", machine().describe_context(), data);
	if (m_pwmc & PWMC_IRQ)
	{
		m_pwmc &= ~PWMC_IRQ;
		set_interrupt_line(INT_PWM, 0);
	}
	return data;
}

void mc68ez328_device::pwms_w(offs_t offset, u16 data, u16 mem_mask) // 0x502
{
	LOGMASKED(LOG_PWM, "%s: pwms_w: PWMS = %04x & %04x\n", machine().describe_context(), data, mem_mask);
	if (mem_mask == 0xffff)
	{
		pwm_fifo_push((u8)(data >> 8));
		pwm_fifo_push((u8)data);
	}
	else if (mem_mask == 0x00ff)
	{
		pwm_fifo_push((u8)data);
	}
}

u16 mc68ez328_device::pwms_r() // 0x502
{
	LOGMASKED(LOG_PWM, "%s: pwms_r: PWMS: %04x\n", machine().describe_context(), m_pwmfifo[m_pwmfifo_rd]);
	return m_pwmfifo[m_pwmfifo_rd];
}

void mc68ez328_device::pwmp_w(u8 data) // 0x504
{
	LOGMASKED(LOG_PWM, "%s: pwmp_w: PWMP = %02x\n", machine().describe_context(), data);
	const bool changed = (data != m_pwmp);
	m_pwmp = data;
	if (changed && (m_pwmc & PWMC_EN))
	{
		update_pwm_period(false);
	}
}

u8 mc68ez328_device::pwmp_r() // 0x504
{
	LOGMASKED(LOG_PWM, "%s: pwmp_r: PWMP: %02x\n", machine().describe_context(), m_pwmp);
	return m_pwmp;
}

void mc68ez328_device::pwmcnt_w(u8 data) // 0x505
{
	LOGMASKED(LOG_PWM, "%s: pwmcnt_w: PWMCNT = %04x (Ignored)\n", machine().describe_context(), data);
}

u8 mc68ez328_device::pwmcnt_r() // 0x505
{
	u8 data = 0;
	if (m_pwmc & PWMC_EN)
	{
		const u32 frequency = (m_pwmc & PWMC_CLK_SRC) ? 32768 : clock();
		const u32 prescale = (m_pwmc & PWMC_PRESCALE) >> PWMC_PRESCALE_SHIFT;
		const u32 divisor = 2 << (m_pwmc & PWMC_CLKSEL);
		const u8 period = std::min(m_pwmp + 1u, 0xffu);
		data = period - (u8)m_pwm->remaining().as_ticks(frequency) / (prescale * divisor);
	}

	LOGMASKED(LOG_PWM, "%s: pwmcnt_r: PWMCNT: %02x\n", machine().describe_context(), data);
	return data;
}


//-------------------------------------------------
//  Timer/Watchdog hardware
//-------------------------------------------------

emu_timer *mc68328_device::get_timer(int timer)
{
	return m_gptimer[timer];
}

emu_timer *mc68ez328_device::get_timer(int timer)
{
	return m_gptimer;
}

mc68328_base_device::timer_regs &mc68328_device::get_timer_regs(int timer)
{
	return m_timer_regs[timer];
}

mc68328_base_device::timer_regs &mc68ez328_device::get_timer_regs(int timer)
{
	return m_timer_regs;
}

u32 mc68328_device::get_timer_int(int timer)
{
	constexpr u32 TIMER_INTS[2] = { INT_TIMER1, INT_TIMER2 };
	return TIMER_INTS[timer];
}

u32 mc68ez328_device::get_timer_int(int timer)
{
	return INT_TIMER2;
}

template <int Timer>
u32 mc68328_base_device::get_timer_frequency()
{
	timer_regs &regs = get_timer_regs(Timer);
	u32 frequency = 0;

	switch (regs.tctl & TCTL_CLKSOURCE)
	{
		case TCTL_CLKSOURCE_SYSCLK:
			frequency = clock();
			break;

		case TCTL_CLKSOURCE_SYSCLK16:
			frequency = clock() / 16;
			break;

		case TCTL_CLKSOURCE_32KHZ4:
		case TCTL_CLKSOURCE_32KHZ5:
		case TCTL_CLKSOURCE_32KHZ6:
		case TCTL_CLKSOURCE_32KHZ7:
			frequency = 32768;
			break;
	}
	frequency /= (regs.tprer + 1);

	return frequency;
}

template <int Timer>
void mc68328_base_device::update_gptimer_state()
{
	timer_regs &regs = get_timer_regs(Timer);
	emu_timer *timer = get_timer(Timer);
	if (BIT(regs.tctl, TCTL_TEN_BIT) && (regs.tctl & TCTL_CLKSOURCE) > TCTL_CLKSOURCE_STOP)
	{
		if ((regs.tctl & TCTL_CLKSOURCE) == TCTL_CLKSOURCE_TIN || regs.tcmp == 0)
		{
			timer->adjust(attotime::never);
		}
		else
		{
			timer->adjust(attotime::from_hz(get_timer_frequency<Timer>()));
		}
	}
	else
	{
		timer->adjust(attotime::never);
	}
}

template <int Timer>
TIMER_CALLBACK_MEMBER(mc68328_base_device::timer_tick)
{
	timer_regs &regs = get_timer_regs(Timer);
	emu_timer *timer = get_timer(Timer);

	u32 frequency = get_timer_frequency<Timer>();
	if (frequency > 0)
	{
		attotime period = attotime::from_hz(frequency);
		timer->adjust(period);
	}
	else
	{
		timer->adjust(attotime::never);
	}

	regs.tcn++;
	if (regs.tcn == regs.tcmp)
	{
		regs.tstat |= TSTAT_COMP;
		if ((regs.tctl & TCTL_FRR) == TCTL_FRR_RESTART)
		{
			regs.tcn = 0x0000;
		}

		if ((regs.tctl & TCTL_IRQEN) == TCTL_IRQEN_ENABLE)
		{
			set_interrupt_line(get_timer_int(Timer), 1);
		}
	}
}

template <int Timer>
void mc68328_base_device::tctl_w(u16 data) // 0x600, 0x60c
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tctl_w<%d>: TCTL%d = %04x\n", machine().describe_context(), Timer, Timer + 1, data);
	const u16 old_tctl = regs.tctl;
	regs.tctl = data;

	const bool old_enable = BIT(old_tctl, TCTL_TEN_BIT);
	const bool new_enable = BIT(regs.tctl, TCTL_TEN_BIT);
	if (!old_enable && new_enable)
	{
		regs.tcn = 0x0000;
	}
	update_gptimer_state<Timer>();
}

template <int Timer>
u16 mc68328_base_device::tctl_r() // 0x600, 0x60c
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tctl_r: TCTL%d: %04x\n", machine().describe_context(), Timer + 1, regs.tctl);
	return regs.tctl;
}

template <int Timer>
void mc68328_base_device::tprer_w(u16 data) // 0x602, 0x60e
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tprer_w<%d>: TPRER%d = %04x\n", machine().describe_context(), Timer, Timer + 1, data);
	regs.tprer = data;
	update_gptimer_state<Timer>();
}

template <int Timer>
u16 mc68328_base_device::tprer_r() // 0x602, 0x60e
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tprer_r: TPRER%d: %04x\n", machine().describe_context(), Timer + 1, regs.tprer);
	return regs.tprer;
}

template <int Timer>
void mc68328_base_device::tcmp_w(u16 data) // 0x604, 0x610
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tcmp_w<%d>: TCMP%d = %04x\n", machine().describe_context(), Timer, Timer + 1, data);
	regs.tcmp = data;
	update_gptimer_state<Timer>();
}

template <int Timer>
u16 mc68328_base_device::tcmp_r() // 0x604, 0x610
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tcmp_r: TCMP%d: %04x\n", machine().describe_context(), Timer + 1, regs.tcmp);
	return regs.tcmp;
}

template <int Timer>
void mc68328_base_device::tcr_w(u16 data) // 0x606, 0x612
{
	LOGMASKED(LOG_TIMERS, "%s: tcr_w<%d>: TCR%d = %04x (Ignored)\n", machine().describe_context(), Timer, Timer + 1, data);
}

template <int Timer>
u16 mc68328_base_device::tcr_r() // 0x606, 0x612
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tcr_r: TCR%d: %04x\n", machine().describe_context(), Timer + 1, regs.tcr);
	return regs.tcr;
}

template <int Timer>
void mc68328_base_device::tcn_w(u16 data) // 0x608, 0x614
{
	LOGMASKED(LOG_TIMERS, "%s: tcn_w<%d>: TCN%d = %04x (Ignored)\n", machine().describe_context(), Timer, Timer + 1, data);
}

template <int Timer>
u16 mc68328_base_device::tcn_r() // 0x608, 0x614
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tcn_r: TCN%d: %04x\n", machine().describe_context(), Timer + 1, regs.tcn);
	return regs.tcn;
}

template <int Timer>
void mc68328_base_device::tstat_w(u16 data) // 0x60a, 0x616
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TSTAT, "%s: tstat_w<%d>: TSTAT%d = %04x\n", machine().describe_context(), Timer, Timer + 1, data);
	regs.tstat &= ~regs.tclear;
	if (!(regs.tstat & TSTAT_COMP))
	{
		set_interrupt_line(get_timer_int(Timer), 0);
	}
}

template <int Timer>
u16 mc68328_base_device::tstat_r() // 0x60a, 0x616
{
	timer_regs &regs = get_timer_regs(Timer);
	LOGMASKED(LOG_TIMERS, "%s: tstat_r: TSTAT%d: %04x\n", machine().describe_context(), Timer + 1, regs.tstat);
	regs.tclear |= regs.tstat;
	return regs.tstat;
}

void mc68328_device::wctlr_w(u16 data) // 0x618
{
	LOGMASKED(LOG_WATCHDOG, "%s: wctlr_w: WCTLR = %04x\n", machine().describe_context(), data);
	m_wctlr = data;
}

u16 mc68328_device::wctlr_r() // 0x618
{
	LOGMASKED(LOG_WATCHDOG, "%s: wctlr_r: WCTLR: %04x\n", machine().describe_context(), m_wctlr);
	return m_wctlr;
}

void mc68328_device::wcmpr_w(u16 data) // 0x61a
{
	LOGMASKED(LOG_WATCHDOG, "%s: wcmpr_w: WCMPR = %04x\n", machine().describe_context(), data);
	m_wcmpr = data;
}

u16 mc68328_device::wcmpr_r() // 0x61a
{
	LOGMASKED(LOG_WATCHDOG, "%s: wcmpr_r: WCMPR: %04x\n", machine().describe_context(), m_wcmpr);
	return m_wcmpr;
}

void mc68328_device::wcn_w(u16 data) // 0x61c
{
	LOGMASKED(LOG_WATCHDOG, "%s: wcn_w: WCN = %04x (Ignored)\n", machine().describe_context(), data);
}

u16 mc68328_device::wcn_r() // 0x61c
{
	LOGMASKED(LOG_WATCHDOG, "%s: wcn_r: WCN: %04x\n", machine().describe_context(), m_wcn);
	return m_wcn;
}


//-------------------------------------------------
//  SPIS hardware
//-------------------------------------------------

void mc68328_device::spisr_w(u16 data) // 0x700
{
	LOGMASKED(LOG_SPIS, "%s: spisr_w: SPISR = %04x\n", machine().describe_context(), data);
	m_spisr = data;
}

u16 mc68328_device::spisr_r() // 0x700
{
	LOGMASKED(LOG_SPIS, "%s: spisr_r: SPISR: %04x\n", machine().describe_context(), m_spisr);
	return m_spisr;
}


//-------------------------------------------------
//  SPIM hardware
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mc68328_base_device::spim_tick)
{
	m_spmclk = !m_spmclk;
	const bool idle_state = BIT(m_spimcont, SPIM_POL_BIT);
	const bool invert_phase = BIT(m_spimcont, SPIM_PHA_BIT);

	u16 spim_bit_index = m_spimcont & SPIM_BIT_COUNT;

	LOGMASKED(LOG_SPIM, "SPIM Tick:\n");
	LOGMASKED(LOG_SPIM, "    CLK state: %d\n", m_spmclk);
	LOGMASKED(LOG_SPIM, "    Bit index: %d\n", spim_bit_index);
	LOGMASKED(LOG_SPIM, "    Data before: %04x\n", m_spimdata);

	const bool clock_txd = (m_spmclk == idle_state && invert_phase) || (m_spmclk != idle_state && !invert_phase);
	if (clock_txd)
	{
		m_spmtxd = BIT(m_spimdata, m_spim_bit_read_idx);
		LOGMASKED(LOG_SPIM, "    Clocking TxD: %d\n", m_spmtxd);
		m_out_spim_cb(m_spmtxd);
	}
	else
	{
		m_spmrxd = m_in_spim_cb();
		LOGMASKED(LOG_SPIM, "    Clocking RxD: %d\n", m_spmrxd);
		LOGMASKED(LOG_SPIM, "    Shifting\n");
		m_spimdata = (m_spimdata << 1) | m_spmrxd;
	}
	LOGMASKED(LOG_SPIM, "    Data after: %04x\n", m_spimdata);

	if (m_spmclk == idle_state)
	{
		if (spim_bit_index == 0)
		{
			LOGMASKED(LOG_SPIM, "    Bit 0 clocked out, ending transfer\n");
			m_spim->adjust(attotime::never);

			if (BIT(m_spimcont, SPIM_IRQEN_BIT))
			{
				m_spimcont |= (1 << SPIM_SPIMIRQ_BIT);
				LOGMASKED(LOG_SPIM, "Triggering SPIM Interrupt\n" );
				set_interrupt_line(INT_SPIM, 1);
			}
		}
		else
		{
			spim_bit_index--;
			m_spimcont &= ~SPIM_BIT_COUNT;
			m_spimcont |= spim_bit_index;
		}
	}
}

void mc68328_base_device::spimdata_w(u16 data) // 0x800
{
	LOGMASKED(LOG_SPIM, "%s: spimdata_w: SPIMDATA = %04x\n", machine().describe_context(), data);
	m_spimdata = data;
}

u16 mc68328_base_device::spimdata_r() // 0x800
{
	LOGMASKED(LOG_SPIM, "%s: spimdata_r: SPIMDATA: %04x\n", machine().describe_context(), m_spimdata);
	return m_spimdata;
}

void mc68328_base_device::spimcont_w(u16 data) // 0x802
{
	LOGMASKED(LOG_SPIM, "%s: spimcont_w: SPIMCONT = %04x\n", machine().describe_context(), data);
	LOGMASKED(LOG_SPIM, "%s:             Count = %d\n", machine().describe_context(), data & SPIM_BIT_COUNT);
	LOGMASKED(LOG_SPIM, "%s:             Polarity = %s\n", machine().describe_context(), BIT(data, SPIM_POL_BIT) ? "Inverted" : "Active-high");
	LOGMASKED(LOG_SPIM, "%s:             Phase = %s\n", machine().describe_context(), BIT(data, SPIM_PHA_BIT) ? "Opposite" : "Normal");
	LOGMASKED(LOG_SPIM, "%s:             IRQ Enable = %s\n", machine().describe_context(), BIT(data, SPIM_IRQEN_BIT) ? "Enable" : "Disable");
	LOGMASKED(LOG_SPIM, "%s:             IRQ Pending = %s\n", machine().describe_context(), BIT(data, SPIM_SPIMIRQ_BIT) ? "Yes" : "No");
	LOGMASKED(LOG_SPIM, "%s:             Exchange = %s\n", machine().describe_context(), BIT(data, SPIM_XCH_BIT) ? "Initiate" : "Idle");
	LOGMASKED(LOG_SPIM, "%s:             SPIM Enable = %s\n", machine().describe_context(), BIT(data, SPIM_SPMEN_BIT) ? "Enable" : "Disable");
	LOGMASKED(LOG_SPIM, "%s:             Data Rate = Divide By %d\n", machine().describe_context(), 4 << ((data & SPIM_RATE_MASK) >> SPIM_RATE_SHIFT) );

	const u16 old = m_spimcont;
	m_spimcont = data;

	if (BIT(data, SPIM_SPMEN_BIT) && BIT(data, SPIM_XCH_BIT) && !BIT(old, SPIM_XCH_BIT))
	{
		const uint64_t divisor = 2 << ((data & SPIM_RATE_MASK) >> SPIM_RATE_SHIFT);
		const attotime rate = attotime::from_ticks(divisor, clock());
		m_spim_bit_read_idx = m_spimcont & SPIM_BIT_COUNT;
		m_spim->adjust(rate, 0, rate);

		m_spimcont &= ~(1 << SPIM_XCH_BIT);
	}

	if (!BIT(data, SPIM_IRQEN_BIT) || !BIT(data, SPIM_SPIMIRQ_BIT))
	{
		set_interrupt_line(INT_SPIM, 0);
	}
	else
	{
		set_interrupt_line(INT_SPIM, 1);
	}
}

u16 mc68328_base_device::spimcont_r() // 0x802
{
	LOGMASKED(LOG_SPIM, "%s: spimcont_r: SPIMCONT: %04x\n", machine().describe_context(), m_spimcont);
	return m_spimcont;
}


//-------------------------------------------------
//  UART hardware
//-------------------------------------------------

void mc68328_base_device::ustcnt_w(u16 data) // 0x900
{
	LOGMASKED(LOG_UART, "%s: ustcnt_w: USTCNT = %04x\n", machine().describe_context(), data);
	m_ustcnt = data;
}

u16 mc68328_base_device::ustcnt_r() // 0x900
{
	LOGMASKED(LOG_UART, "%s: ustcnt_r: USTCNT: %04x\n", machine().describe_context(), m_ustcnt);
	return m_ustcnt;
}

void mc68328_base_device::ubaud_w(u16 data) // 0x902
{
	LOGMASKED(LOG_UART, "%s: ubaud_w: UBAUD = %04x\n", machine().describe_context(), data);
	m_ubaud = data;
}

u16 mc68328_base_device::ubaud_r() // 0x902
{
	LOGMASKED(LOG_UART, "%s: ubaud_r: UBAUD: %04x\n", machine().describe_context(), m_ubaud);
	return m_ubaud;
}

void mc68328_base_device::urx_w(u16 data) // 0x904
{
	LOGMASKED(LOG_UART, "%s: urx_w: URX = %04x (Not Yet Implemented)\n", machine().describe_context(), data);
}

u16 mc68328_base_device::urx_r() // 0x904
{
	LOGMASKED(LOG_UART, "%s: urx_r: URX: %04x\n", machine().describe_context(), m_urx);
	return m_urx;
}

void mc68328_base_device::utx_w(u16 data) // 0x906
{
	LOGMASKED(LOG_UART, "%s: utx_w: UTX = %04x (Not Yet Implemented)\n", machine().describe_context(), data);
}

u16 mc68328_base_device::utx_r() // 0x906
{
	u16 data = m_utx | UTX_FIFO_EMPTY | UTX_FIFO_HALF | UTX_TX_AVAIL;
	LOGMASKED(LOG_UART, "%s: utx_r: UTX: %04x\n", machine().describe_context(), data);
	return data;
}

void mc68328_base_device::umisc_w(u16 data) // 0x908
{
	LOGMASKED(LOG_UART, "%s: umisc_w: UMISC = %04x (Not Yet Implemented)\n", machine().describe_context(), data);
	m_umisc = data;
}

u16 mc68328_base_device::umisc_r() // 0x908
{
	LOGMASKED(LOG_UART, "%s: umisc_r: UMISC: %04x\n", machine().describe_context(), m_umisc);
	return m_umisc;
}


//-------------------------------------------------
//  LCD hardware - Shared and Standard MC68328
//-------------------------------------------------

void mc68328_device::lcd_update_info()
{
	if (!m_lcd_update_pending)
	{
		return;
	}
	const u32 sysclk_divisor = VCO_DIVISORS[(m_pllcr & PLLCR_SYSCLK_SEL) >> PLLCR_SYSCLK_SHIFT];
	attotime lcd_dma_duration = attotime::from_ticks(lcd_get_line_word_count() * sysclk_divisor, clock());
	attotime lcd_scan_duration = lcd_get_line_rate();
	attotime lcd_frame_duration = (lcd_scan_duration + lcd_dma_duration) * (m_lymax + 1);
	LOGMASKED(LOG_LCD, "lxmax %d, lymax %d, divisor %d, lrra %02x, lpxcd %02x\n", m_lxmax, m_lymax + 1, sysclk_divisor, m_lpxcd + 1);

	constexpr u8 BIT_WIDTHS[4] = { 1, 2, 4, 0xff };
	m_lcd_info_changed_cb(lcd_frame_duration.as_hz(), lcd_get_width(), m_lymax + 1, BIT_WIDTHS[(m_lpicf & LPICF_PBSIZ) >> LPICF_PBSIZ_SHIFT], BIT_WIDTHS[m_lpicf & LPICF_GS]);

	m_lcd_update_pending = false;
}

u16 mc68328_device::lcd_get_lxmax_mask()
{
	constexpr u16 LXMAX_MASK = 0x03ff;
	return LXMAX_MASK;
}

int mc68328_device::lcd_get_width()
{
	return (m_lxmax & lcd_get_lxmax_mask()) + 1;
}

u32 mc68328_device::lcd_get_line_word_count()
{
	return m_lvpw != m_llbar ? (m_llbar + 1) : m_llbar;
}

attotime mc68328_device::lcd_get_line_rate()
{
	const u32 sysclk_divisor = VCO_DIVISORS[(m_pllcr & PLLCR_SYSCLK_SEL) >> PLLCR_SYSCLK_SHIFT];
	return attotime::from_ticks(m_llbar, clock() / sysclk_divisor);
}

u8 mc68328_device::lcd_get_panel_bit_size()
{
	constexpr u8 BIT_WIDTHS[4] = { 1, 2, 4, 0xff };
	return BIT_WIDTHS[(m_lpicf & LPICF_PBSIZ) >> LPICF_PBSIZ_SHIFT];
}

attotime mc68328_device::get_pixclk_rate()
{
	u32 divisor = 1;
	if (BIT(m_lckcon, LCKCON_PCDS_BIT)) // Use PIXCLK from PLL
		divisor = VCO_DIVISORS[(m_pllcr & PLLCR_PIXCLK_SEL) >> PLLCR_PIXCLK_SHIFT];
	else // Use SYSCLK from PLL
		divisor = VCO_DIVISORS[(m_pllcr & PLLCR_SYSCLK_SEL) >> PLLCR_SYSCLK_SHIFT];

	return attotime::from_ticks((m_lpxcd & LPXCD_MASK) + 1, clock() / divisor);
}

void mc68328_base_device::fill_lcd_dma_buffer()
{
	if (m_lcd_sysmem_ptr == m_lssa)
	{
		lcd_update_info();
		m_out_flm_cb(BIT(m_lpolcf, LPOLCF_FLMPOL_BIT) ? 0 : 1);
		m_lssa_end = m_lssa + ((m_lvpw * (m_lymax + 1)) << 1);
	}
	else
	{
		m_out_flm_cb(BIT(m_lpolcf, LPOLCF_FLMPOL_BIT) ? 1 : 0);
	}
	m_out_llp_cb(BIT(m_lpolcf, LPOLCF_LPPOL_BIT) ? 0 : 1);

	attotime buffer_duration = lcd_get_line_rate();
	m_lcd_scan->adjust(buffer_duration);

	address_space &prg_space = space(AS_PROGRAM);
	const u32 word_count = lcd_get_line_word_count();
	for (u32 word_index = 0; word_index < word_count; word_index++)
	{
		m_lcd_line_buffer[word_index] = prg_space.read_word(m_lcd_sysmem_ptr + (word_index << 1));
	}

	m_lcd_sysmem_ptr += m_lvpw << 1;
	if (m_lcd_sysmem_ptr >= m_lssa_end)
	{
		m_lcd_sysmem_ptr = m_lssa;
	}

	m_lcd_line_bit = 15;
	m_lcd_line_word = 0;
}

TIMER_CALLBACK_MEMBER(mc68328_base_device::lcd_scan_tick)
{
	m_out_llp_cb(BIT(m_lpolcf, LPOLCF_LPPOL_BIT) ? 1 : 0);
	m_lsclk = !m_lsclk;

	if (m_lsclk)
	{
		u8 data = 0;
		switch (lcd_get_panel_bit_size())
		{
		case 1:
			data = BIT(m_lcd_line_buffer[m_lcd_line_word], m_lcd_line_bit);
			if (m_lcd_line_bit == 0)
			{
				m_lcd_line_bit = 15;
				m_lcd_line_word++;
			}
			else
			{
				m_lcd_line_bit--;
			}
			break;
		case 2:
			data = (m_lcd_line_buffer[m_lcd_line_word] >> (m_lcd_line_bit - 1)) & 3;
			if (m_lcd_line_bit <= 1)
			{
				m_lcd_line_bit = 15;
				m_lcd_line_word++;
			}
			else
			{
				m_lcd_line_bit -= 2;
			}
			break;
		case 4:
			data = (m_lcd_line_buffer[m_lcd_line_word] >> (m_lcd_line_bit - 3)) & 15;
			if (m_lcd_line_bit <= 3)
			{
				m_lcd_line_bit = 15;
				m_lcd_line_word++;
			}
			else
			{
				m_lcd_line_bit -= 4;
			}
			break;
		default:
			// Invalid mode; don't send anything
			break;
		}
		m_out_ld_cb(data);
	}
	m_out_lsclk_cb(m_lsclk);

	if (m_lcd_line_word == lcd_get_line_word_count())
	{
		fill_lcd_dma_buffer();
	}
	else
	{
		m_lcd_scan->adjust(get_pixclk_rate());
	}
}

void mc68328_base_device::lssa_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0xa00
{
	LOGMASKED(LOG_LCD, "%s: lssa_msw_w: LSSA(MSW) = %04x\n", machine().describe_context(), data);
	m_lssa &= ~(mem_mask << 16);
	m_lssa |= (data & mem_mask) << 16;
	LOGMASKED(LOG_LCD, "%s:             Address: %08x\n", machine().describe_context(), m_lssa);
}

u16 mc68328_base_device::lssa_msw_r() // 0xa00
{
	LOGMASKED(LOG_LCD, "%s: lssa_msw_r: LSSA(MSW): %04x\n", machine().describe_context(), (u16)(m_lssa >> 16));
	return (u16)(m_lssa >> 16);
}

void mc68328_base_device::lssa_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0xa02
{
	LOGMASKED(LOG_LCD, "%s: lssa_lsw_w: LSSA(LSW) = %04x\n", machine().describe_context(), data);
	m_lssa &= 0xffff0000 | (~mem_mask);
	m_lssa |= data & mem_mask;
	LOGMASKED(LOG_LCD, "            Address: %08x\n", machine().describe_context(), m_lssa);
}

u16 mc68328_base_device::lssa_lsw_r() // 0xa02
{
	LOGMASKED(LOG_LCD, "%s: lssa_lsw_r: LSSA(LSW): %04x\n", machine().describe_context(), (u16)m_lssa);
	return (u16)m_lssa;
}

void mc68328_base_device::lvpw_w(u8 data) // 0xa05
{
	LOGMASKED(LOG_LCD, "%s: lvpw_w: LVPW = %02x\n", machine().describe_context(), data);
	m_lvpw = data;
	LOGMASKED(LOG_LCD, "%s:         Virtual Page Width: %d words\n", machine().describe_context(), m_lvpw);
}

u8 mc68328_base_device::lvpw_r() // 0xa05
{
	LOGMASKED(LOG_LCD, "%s: lvpw_r: LVPW: %02x\n", machine().describe_context(), m_lvpw);
	return m_lvpw;
}

void mc68328_base_device::lxmax_w(u16 data) // 0xa08
{
	m_lcd_update_pending = m_lcd_update_pending || (m_lxmax != (data & lcd_get_lxmax_mask()));
	LOGMASKED(LOG_LCD, "%s: lxmax_w: LXMAX = %04x\n", machine().describe_context(), data);
	m_lxmax = data & lcd_get_lxmax_mask();
	LOGMASKED(LOG_LCD, "%s:          Width: %d\n", machine().describe_context(), lcd_get_width());
}

u16 mc68328_base_device::lxmax_r() // 0xa08
{
	LOGMASKED(LOG_LCD, "%s: lxmax_r: LXMAX: %04x\n", machine().describe_context(), m_lxmax);
	return m_lxmax;
}

void mc68328_base_device::lymax_w(u16 data) // 0xa0a
{
	m_lcd_update_pending = m_lcd_update_pending || (m_lxmax != (data & LYMAX_MASK));
	LOGMASKED(LOG_LCD, "%s: lymax_w: LYMAX = %04x\n", machine().describe_context(), data);
	m_lymax = data & LYMAX_MASK;
	LOGMASKED(LOG_LCD, "%s:          Height: %d\n", machine().describe_context(), (data & 0x03ff) + 1);
}

u16 mc68328_base_device::lymax_r() // 0xa0a
{
	LOGMASKED(LOG_LCD, "%s: lymax_r: LYMAX: %04x\n", machine().describe_context(), m_lymax);
	return m_lymax;
}

void mc68328_base_device::lcxp_w(u16 data) // 0xa18
{
	LOGMASKED(LOG_LCD, "%s: lcxp_w: LCXP = %04x\n", machine().describe_context(), data);
	m_lcxp = data;
	LOGMASKED(LOG_LCD, "%s:         X Position: %d\n", machine().describe_context(), data & 0x03ff);
	switch (m_lcxp >> 14)
	{
		case 0:
			LOGMASKED(LOG_LCD, "%s:         Cursor Control: Transparent\n", machine().describe_context());
			break;

		case 1:
			LOGMASKED(LOG_LCD, "%s:         Cursor Control: Black\n", machine().describe_context());
			break;

		case 2:
			LOGMASKED(LOG_LCD, "%s:         Cursor Control: Reverse\n", machine().describe_context());
			break;

		case 3:
			LOGMASKED(LOG_LCD, "%s:         Cursor Control: Invalid\n", machine().describe_context());
			break;
	}
}

u16 mc68328_base_device::lcxp_r() // 0xa18
{
	LOGMASKED(LOG_LCD, "%s: lcxp_r: LCXP: %04x\n", machine().describe_context(), m_lcxp);
	return m_lcxp;
}

void mc68328_base_device::lcyp_w(u16 data) // 0xa1a
{
	LOGMASKED(LOG_LCD, "%s: lcyp_w: LCYP = %04x\n", machine().describe_context(), data);
	m_lcyp = data;
	LOGMASKED(LOG_LCD, "%s:         Y Position: %d\n", machine().describe_context(), data & 0x01ff);
}

u16 mc68328_base_device::lcyp_r() // 0xa1a
{
	LOGMASKED(LOG_LCD, "%s: lcyp_r: LCYP: %04x\n", machine().describe_context(), m_lcyp);
	return m_lcyp;
}

void mc68328_base_device::lcwch_w(u16 data) // 0xa1c
{
	LOGMASKED(LOG_LCD, "%s: lcwch_w: LCWCH = %04x\n", machine().describe_context(), data);
	m_lcwch = data;
	LOGMASKED(LOG_LCD, "%s:          Width:  %d\n", machine().describe_context(), (data >> 8) & 0x1f);
	LOGMASKED(LOG_LCD, "%s:          Height: %d\n", machine().describe_context(), data & 0x1f);
}

u16 mc68328_base_device::lcwch_r() // 0xa1c
{
	LOGMASKED(LOG_LCD, "%s: lcwch_r: LCWCH: %04x\n", machine().describe_context(), m_lcwch);
	return m_lcwch;
}

void mc68328_base_device::lblkc_w(u8 data) // 0xa1f
{
	LOGMASKED(LOG_LCD, "%s: lblkc_w: LBLKC = %02x\n", machine().describe_context(), data);
	m_lblkc = data;
	LOGMASKED(LOG_LCD, "%s:          Blink Enable:  %d\n", machine().describe_context(), m_lblkc >> 7);
	LOGMASKED(LOG_LCD, "%s:          Blink Divisor: %d\n", machine().describe_context(), m_lblkc & 0x7f);
}

u8 mc68328_base_device::lblkc_r() // 0xa1f
{
	LOGMASKED(LOG_LCD, "%s: lblkc_r: LBLKC: %02x\n", machine().describe_context(), m_lblkc);
	return m_lblkc;
}

void mc68328_device::lpicf_w(u8 data) // 0xa20
{
	static const char *const PBSIZ_NAMES[4] = { "1-bit", "2-bit", "4-bit", "Invalid" };
	LOGMASKED(LOG_LCD, "%s: lpicf_w: LPICF = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD, "%s:          Grayscale Mode: %d\n", machine().describe_context(), data & LPICF_GS);
	LOGMASKED(LOG_LCD, "%s:          Bus Size: %s\n", machine().describe_context(), PBSIZ_NAMES[(data & LPICF_PBSIZ) >> LPICF_PBSIZ_SHIFT]);
	m_lpicf = data;
}

u8 mc68328_base_device::lpicf_r() // 0xa20
{
	LOGMASKED(LOG_LCD, "%s: lpicf_r: LPICF: %02x\n", machine().describe_context(), m_lpicf);
	return m_lpicf;
}

void mc68328_base_device::lpolcf_w(u8 data) // 0xa21
{
	LOGMASKED(LOG_LCD, "%s: lpolcf_w: LPOLCF = %02x\n", machine().describe_context(), data);
	m_lpolcf = data;
	LOGMASKED(LOG_LCD, "%s:           LCD Shift Clock Polarity: %s\n", machine().describe_context(), (m_lpicf & 0x08) ? "Active positive edge of LCLK" : "Active negative edge of LCLK");
	LOGMASKED(LOG_LCD, "%s:           First-line marker polarity: %s\n", machine().describe_context(), (m_lpicf & 0x04) ? "Active Low" : "Active High");
	LOGMASKED(LOG_LCD, "%s:           Line-pulse polarity: %s\n", machine().describe_context(), (m_lpicf & 0x02) ? "Active Low" : "Active High");
	LOGMASKED(LOG_LCD, "%s:           Pixel polarity: %s\n", machine().describe_context(), (m_lpicf & 0x01) ? "Active Low" : "Active High");
}

u8 mc68328_base_device::lpolcf_r() // 0xa21
{
	LOGMASKED(LOG_LCD, "%s: lpolcf_r: LPOLCF: %02x\n", machine().describe_context(), m_lpolcf);
	return m_lpolcf;
}

void mc68328_base_device::lacdrc_w(u8 data) // 0xa23
{
	LOGMASKED(LOG_LCD, "%s: lacdrc_w: LACDRC = %02x\n", machine().describe_context(), data);
	m_lacdrc = data;
}

u8 mc68328_base_device::lacdrc_r() // 0xa23
{
	LOGMASKED(LOG_LCD, "%s: lacdrc_r: LACDRC: %02x\n", machine().describe_context(), m_lacdrc);
	return m_lacdrc;
}

void mc68328_base_device::lpxcd_w(u8 data) // 0xa25
{
	LOGMASKED(LOG_LCD, "%s: lpxcd_w: LPXCD = %02x\n", machine().describe_context(), data);
	m_lpxcd = data;
	LOGMASKED(LOG_LCD, "%s:          Clock Divisor: %d\n", machine().describe_context(), m_lpxcd + 1);
}

u8 mc68328_base_device::lpxcd_r() // 0xa25
{
	LOGMASKED(LOG_LCD, "%s: lpxcd_r: LPXCD: %02x\n", machine().describe_context(), m_lpxcd);
	return m_lpxcd;
}

void mc68328_device::lckcon_w(u8 data) // 0xa27
{
	LOGMASKED(LOG_LCD, "%s: lckcon_w: LCKCON = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD, "%s:           LCDC Enable: %d\n", machine().describe_context(), BIT(data, LCKCON_LCDON_BIT));
	LOGMASKED(LOG_LCD, "%s:           DMA Burst Length: %d\n", machine().describe_context(), BIT(data, LCKCON_DMA16_BIT) ? 16 : 8);
	LOGMASKED(LOG_LCD, "%s:           DMA Bursting Clock Control: %d\n", machine().describe_context(), ((data & LCKCON_WS) >> LCKCON_WS_SHIFT) + 1);
	LOGMASKED(LOG_LCD, "%s:           Bus Width: %d\n", machine().describe_context(), BIT(data, LCKCON_DWIDTH_BIT) ? 8 : 16);
	LOGMASKED(LOG_LCD, "%s:           Pixel Clock Divider Source: %s\n", machine().describe_context(), BIT(data, LCKCON_PCDS_BIT) ? "PIX" : "SYS");

	const u16 old = m_lckcon;
	m_lckcon = data;

	lcd_update_info();
	if (BIT(old, LCKCON_LCDON_BIT) && !BIT(m_lckcon, LCKCON_LCDON_BIT))
	{
		m_lcd_scan->adjust(attotime::never);
	}
	else if (!BIT(old, LCKCON_LCDON_BIT) && BIT(m_lckcon, LCKCON_LCDON_BIT))
	{
		m_lcd_scan->adjust(attotime::never);
		m_lcd_sysmem_ptr = m_lssa;
		fill_lcd_dma_buffer();
	}
}

u8 mc68328_base_device::lckcon_r() // 0xa27
{
	LOGMASKED(LOG_LCD, "%s: lckcon_r: LCKCON: %02x\n", machine().describe_context(), m_lckcon);
	return m_lckcon;
}

void mc68328_device::llbar_w(u8 data) // 0xa29
{
	LOGMASKED(LOG_LCD, "%s: llbar_w: LLBAR = %02x\n", machine().describe_context(), data);
	m_llbar = data;
	LOGMASKED(LOG_LCD, "%s:          Address: %d\n", machine().describe_context(), m_llbar << (BIT(m_lpicf, LPICF_GS_BIT) ? 4 : 5));
}

u8 mc68328_device::llbar_r() // 0xa29
{
	LOGMASKED(LOG_LCD, "%s: llbar_r: LLBAR: %02x\n", machine().describe_context(), m_llbar);
	return m_llbar;
}

void mc68328_device::lotcr_w(u8 data) // 0xa2b
{
	LOGMASKED(LOG_LCD, "%s: lotcr_w: LOTCR = %02x (Ignored)\n", machine().describe_context(), data);
}

u8 mc68328_device::lotcr_r() // 0xa2b
{
	LOGMASKED(LOG_LCD, "%s: lotcr_r: LOTCR: %02x\n", machine().describe_context(), m_lotcr);
	return m_lotcr;
}

void mc68328_base_device::lposr_w(u8 data) // 0xa2d
{
	LOGMASKED(LOG_LCD, "%s: lposr_w: LPOSR = %02x\n", machine().describe_context(), data);
	m_lposr = data;
	LOGMASKED(LOG_LCD, "%s:          Byte Offset: %d\n", machine().describe_context(), (m_lposr >> 3) & 0x01);
	LOGMASKED(LOG_LCD, "%s:          Pixel Offset: %d\n", machine().describe_context(), m_lposr & 0x07);
}

u8 mc68328_base_device::lposr_r() // 0xa2d
{
	LOGMASKED(LOG_LCD, "%s: lposr_r: LPOSR: %02x\n", machine().describe_context(), m_lposr);
	return m_lposr;
}

void mc68328_base_device::lfrcm_w(u8 data) // 0xa31
{
	LOGMASKED(LOG_LCD, "%s: lfrcm_w: LFRCM = %02x\n", machine().describe_context(), data);
	m_lfrcm = data;
	LOGMASKED(LOG_LCD, "%s:          X Modulation: %d\n", machine().describe_context(), (m_lfrcm >> 4) & 0x0f);
	LOGMASKED(LOG_LCD, "%s:          Y Modulation: %d\n", machine().describe_context(), m_lfrcm & 0x0f);
}

u8 mc68328_base_device::lfrcm_r() // 0xa31
{
	LOGMASKED(LOG_LCD, "%s: lfrcm_r: LFRCM: %02x\n", machine().describe_context(), m_lfrcm);
	return m_lfrcm;
}

void mc68328_device::lgpmr_w(u8 data) // 0xa32
{
	LOGMASKED(LOG_LCD, "%s: lgpmr_w: LGPMR = %04x\n", machine().describe_context(), data);
	m_lgpmr = data;
	LOGMASKED(LOG_LCD, "%s:          Palette 0: %d\n", machine().describe_context(), (m_lgpmr >>  8) & 0x07);
	LOGMASKED(LOG_LCD, "%s:          Palette 1: %d\n", machine().describe_context(), (m_lgpmr >> 12) & 0x07);
	LOGMASKED(LOG_LCD, "%s:          Palette 2: %d\n", machine().describe_context(), (m_lgpmr >>  0) & 0x07);
	LOGMASKED(LOG_LCD, "%s:          Palette 3: %d\n", machine().describe_context(), (m_lgpmr >>  4) & 0x07);
}

u16 mc68328_device::lgpmr_r() // 0xa32
{
	LOGMASKED(LOG_LCD, "%s: lgpmr_r: LGPMR: %04x\n", machine().describe_context(), m_lgpmr);
	return m_lgpmr;
}


//-------------------------------------------------
//  LCD hardware - EZ variant
//-------------------------------------------------

void mc68ez328_device::lcd_update_info()
{
	if (!m_lcd_update_pending)
	{
		return;
	}

	const u32 sysclk_divisor = VCO_DIVISORS[(m_pllcr & PLLCR_SYSCLK_SEL) >> PLLCR_SYSCLK_SHIFT];
	attotime lcd_dma_duration = attotime::from_ticks(lcd_get_line_word_count() * sysclk_divisor, clock());
	attotime lcd_scan_duration = lcd_get_line_rate();
	attotime lcd_frame_duration = (lcd_scan_duration + lcd_dma_duration) * (m_lymax + 1) * 2;

	constexpr u8 BIT_WIDTHS[4] = { 1, 2, 4, 0xff };
	m_lcd_info_changed_cb(lcd_frame_duration.as_hz(), lcd_get_width(), m_lymax + 1, BIT_WIDTHS[(m_lpicf & LPICF_PBSIZ) >> LPICF_PBSIZ_SHIFT], BIT_WIDTHS[m_lpicf & LPICF_GS]);

	m_lcd_update_pending = false;
}

u16 mc68ez328_device::lcd_get_lxmax_mask()
{
	constexpr u16 LXMAX_MASK = 0x03f0;
	return LXMAX_MASK;
}

int mc68ez328_device::lcd_get_width()
{
	return m_lxmax & lcd_get_lxmax_mask();
}

u32 mc68ez328_device::lcd_get_line_word_count()
{
	const u32 pixels_per_word = 16 / (1 << (m_lpicf & LPICF_GS));
	const u32 data = (m_lxmax & lcd_get_lxmax_mask()) / pixels_per_word;
	return data;
}

attotime mc68ez328_device::lcd_get_line_rate()
{
	const u32 pixclk_divisor = VCO_DIVISORS[(m_pllcr & PLLCR_PIXCLK_SEL) >> PLLCR_PIXCLK_SHIFT];
	const u32 pxcd = (m_lpxcd & LPXCD_MASK) + 1;
	const u32 lrra_factor = 6 + m_lrra + (m_lxmax & lcd_get_lxmax_mask()) * 4;
	const u32 ticks = lrra_factor * pxcd * pixclk_divisor;
	return attotime::from_ticks(ticks, clock());
}

u8 mc68ez328_device::lcd_get_panel_bit_size()
{
	constexpr u8 BIT_WIDTHS[4] = { 1, 2, 4, 1 };
	return BIT_WIDTHS[(m_lpicf & LPICF_PBSIZ) >> LPICF_PBSIZ_SHIFT];
}

attotime mc68ez328_device::get_pixclk_rate()
{
	u32 divisor = VCO_DIVISORS[(m_pllcr & PLLCR_PIXCLK_SEL) >> PLLCR_PIXCLK_SHIFT];
	return attotime::from_ticks((m_lpxcd & LPXCD_MASK) + 1, clock() / divisor);
}

void mc68ez328_device::lpicf_w(u8 data) // 0xa20
{
	static const char *const PBSIZ_NAMES[4] = { "1-bit", "2-bit", "4-bit", "Invalid" };
	static const char *const GS_NAMES[4] = { "Monochrome", "4-level Grayscale", "16-level Grayscale", "Invalid" };
	LOGMASKED(LOG_LCD, "%s: lpicf_w: LPICF = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD, "%s:          Grayscale Mode: %d\n", machine().describe_context(), GS_NAMES[data & LPICF_GS]);
	LOGMASKED(LOG_LCD, "%s:          Bus Size: %s\n", machine().describe_context(), PBSIZ_NAMES[(data & LPICF_PBSIZ) >> LPICF_PBSIZ_SHIFT]);
	m_lcd_update_pending = m_lcd_update_pending || (m_lpicf != data);
	m_lpicf = data;
}

void mc68ez328_device::lckcon_w(u8 data) // 0xa27
{
	LOGMASKED(LOG_LCD, "%s: lckcon_w: LCKCON = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD, "%s:           LCDC Enable: %d\n", machine().describe_context(), BIT(data, LCKCON_LCDON_BIT));
	LOGMASKED(LOG_LCD, "%s:           Display Wait States: %d\n", machine().describe_context(), ((data & LCKCON_DWS) >> LCKCON_DWS_SHIFT) + 1);
	LOGMASKED(LOG_LCD, "%s:           Bus Width: %d\n", machine().describe_context(), BIT(data, LCKCON_DWIDTH_BIT) ? 8 : 16);

	const u16 old = m_lckcon;
	m_lckcon = data;

	lcd_update_info();
	if (BIT(old, LCKCON_LCDON_BIT) && !BIT(m_lckcon, LCKCON_LCDON_BIT))
	{
		m_lcd_scan->adjust(attotime::never);
	}
	else if (!BIT(old, LCKCON_LCDON_BIT) && BIT(m_lckcon, LCKCON_LCDON_BIT))
	{
		m_lcd_scan->adjust(attotime::never);
		m_lcd_sysmem_ptr = m_lssa;
		fill_lcd_dma_buffer();
	}
}

void mc68ez328_device::lrra_w(u8 data) // 0xa29
{
	LOGMASKED(LOG_LCD, "%s: lrra_w: LRRA = %02x\n", machine().describe_context(), data);
	m_lcd_update_pending = m_lcd_update_pending || (m_lrra != data);
	m_lrra = data;
}

u8 mc68ez328_device::lrra_r() // 0xa29
{
	LOGMASKED(LOG_LCD, "%s: lrra_r: LRRA: %02x\n", machine().describe_context(), m_lrra);
	return m_lrra;
}

void mc68ez328_device::pwmr_w(offs_t offset, u16 data, u16 mem_mask) // 0xa36
{
	LOGMASKED(LOG_LCD, "%s: pwmr_w: PWMR = %04x\n", machine().describe_context(), data);
	m_pwmr = data;
}

u16 mc68ez328_device::pwmr_r() // 0xa36
{
	LOGMASKED(LOG_LCD, "%s: pwmr_r: PWMR: %04x\n", machine().describe_context(), m_lrra);
	return m_pwmr;
}


//-------------------------------------------------
//  RTC/alarm hardware - Standard MC68328
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mc68328_base_device::rtc_tick)
{
	if (BIT(m_rtcctl, RTCCTL_ENABLE_BIT))
	{
		const bool rtc_int_was_active = rtc_int_is_active();

		rtc_advance_seconds();

		if (rtc_get_alarm_match())
		{
			m_rtcisr |= RTCINT_ALARM;
		}

		if (!rtc_int_was_active && rtc_int_is_active())
		{
			set_interrupt_line(INT_RTC, 1);
		}
	}
}

void mc68328_base_device::rtc_advance_seconds()
{
	m_hmsr++;

	if (m_rtcienr & RTCINT_SECOND)
	{
		m_rtcisr |= RTCINT_SECOND;
	}

	if ((m_hmsr & RTCHMSR_SECONDS) == 0x0000003c)
	{
		m_hmsr &= ~RTCHMSR_SECONDS;
		m_hmsr += 1 << RTCHMSR_MINUTES_SHIFT;

		if (m_rtcienr & RTCINT_MINUTE)
		{
			m_rtcisr |= RTCINT_MINUTE;
		}

		if ((m_hmsr & RTCHMSR_MINUTES) == 0x003c0000)
		{
			m_hmsr &= ~RTCHMSR_MINUTES;
			m_hmsr += 1 << RTCHMSR_HOURS_SHIFT;

			if ((m_hmsr & RTCHMSR_HOURS) == 0x18000000)
			{
				m_hmsr &= ~RTCHMSR_HOURS;

				if (m_rtcienr & RTCINT_DAY)
				{
					m_rtcisr |= RTCINT_DAY;
				}
			}
		}

		if (m_stpwtch != RTCSTPWTCH_MASK)
		{
			m_stpwtch--;
			m_stpwtch &= RTCSTPWTCH_MASK;

			if (m_stpwtch == RTCSTPWTCH_MASK)
			{
				m_rtcisr |= RTCINT_STOPWATCH;
			}
		}
	}
}

bool mc68328_device::rtc_int_is_active()
{
	return m_rtcisr & m_rtcienr;
}

u16 mc68328_device::rtc_get_int_mask()
{
	constexpr u16 RTCIENR_MASK = 0x001f;
	return RTCIENR_MASK;
}

bool mc68328_device::rtc_get_alarm_match()
{
	return m_hmsr == m_alarm;
}

void mc68328_base_device::hmsr_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0xb00
{
	LOGMASKED(LOG_RTC, "%s: hmsr_msw_w: HMSR(MSW) = %04x\n", machine().describe_context(), data);
	m_hmsr &= ~(mem_mask << 16);
	m_hmsr |= (data & mem_mask) << 16;
	m_hmsr &= (RTCHMSR_SECONDS | RTCHMSR_MINUTES | RTCHMSR_HOURS);
}

u16 mc68328_base_device::hmsr_msw_r() // 0xb00
{
	LOGMASKED(LOG_RTC, "%s: hmsr_msw_r: HMSR(MSW): %04x\n", machine().describe_context(), (u16)(m_hmsr >> 16));
	return (u16)(m_hmsr >> 16);
}

void mc68328_base_device::hmsr_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0xb02
{
	LOGMASKED(LOG_RTC, "%s: hmsr_lsw_w: HMSR(LSW) = %04x\n", machine().describe_context(), data);
	m_hmsr &= 0xffff0000 | (~mem_mask);
	m_hmsr |= data & mem_mask;
	m_hmsr &= (RTCHMSR_SECONDS | RTCHMSR_MINUTES | RTCHMSR_HOURS);
}

u16 mc68328_base_device::hmsr_lsw_r() // 0xb02
{
	LOGMASKED(LOG_RTC, "%s: hmsr_lsw_r: HMSR(LSW): %04x\n", machine().describe_context(), (u16)m_hmsr);
	return (u16)m_hmsr;
}

void mc68328_base_device::alarm_msw_w(offs_t offset, u16 data, u16 mem_mask) // 0xb04
{
	LOGMASKED(LOG_RTC, "%s: alarm_msw_w: ALARM(MSW) = %04x\n", machine().describe_context(), data);
	m_alarm &= ~(mem_mask << 16);
	m_alarm |= (data & mem_mask) << 16;
	m_alarm &= (RTCHMSR_SECONDS | RTCHMSR_MINUTES | RTCHMSR_HOURS);
}

u16 mc68328_base_device::alarm_msw_r() // 0xb04
{
	LOGMASKED(LOG_RTC, "%s: alarm_msw_r: ALARM(MSW): %04x\n", machine().describe_context(), (u16)(m_alarm >> 16));
	return (u16)(m_alarm >> 16);
}

void mc68328_base_device::alarm_lsw_w(offs_t offset, u16 data, u16 mem_mask) // 0xb06
{
	LOGMASKED(LOG_RTC, "%s: alarm_lsw_w: ALARM(LSW) = %04x\n", machine().describe_context(), data);
	m_alarm &= 0xffff0000 | (~mem_mask);
	m_alarm |= data & mem_mask;
	m_alarm &= (RTCHMSR_SECONDS | RTCHMSR_MINUTES | RTCHMSR_HOURS);
}

u16 mc68328_base_device::alarm_lsw_r() // 0xb06
{
	LOGMASKED(LOG_RTC, "%s: alarm_lsw_r: ALARM(LSW): %04x\n", machine().describe_context(), (u16)m_alarm);
	return (u16)m_alarm;
}

void mc68328_base_device::rtcctl_w(offs_t offset, u16 data, u16 mem_mask) // 0xb0c
{
	LOGMASKED(LOG_RTC, "%s: rtcctl_w: RTCCTL = %04x\n", machine().describe_context(), data);
	m_rtcctl = data & RTCCTL_MASK;
}

u16 mc68328_base_device::rtcctl_r() // 0xb0c
{
	LOGMASKED(LOG_RTC, "%s: rtcctl_r: RTCCTL: %04x\n", machine().describe_context(), m_rtcctl);
	return m_rtcctl;
}

void mc68328_base_device::rtcisr_w(offs_t offset, u16 data, u16 mem_mask) // 0xb0e
{
	const bool rtc_int_was_active = rtc_int_is_active();
	LOGMASKED(LOG_RTC, "%s: rtcisr_w: RTCISR = %04x\n", machine().describe_context(), data);
	m_rtcisr &= ~data;

	if (rtc_int_was_active && !rtc_int_is_active())
	{
		set_interrupt_line(INT_RTC, 0);
	}
}

u16 mc68328_base_device::rtcisr_r() // 0xb0e
{
	LOGMASKED(LOG_RTC, "%s: rtcisr_r: RTCISR: %04x\n", machine().describe_context(), m_rtcisr);
	return m_rtcisr;
}

void mc68328_base_device::rtcienr_w(offs_t offset, u16 data, u16 mem_mask) // 0xb10
{
	const bool rtc_int_was_active = rtc_int_is_active();

	LOGMASKED(LOG_RTC, "%s: rtcienr_w: RTCIENR = %04x\n", machine().describe_context(), data);
	m_rtcienr = data & rtc_get_int_mask();

	const bool is_active = rtc_int_is_active();
	if (rtc_int_was_active != is_active)
	{
		set_interrupt_line(INT_RTC, (int)is_active);
	}
}

u16 mc68328_base_device::rtcienr_r() // 0xb10
{
	LOGMASKED(LOG_RTC, "%s: rtcienr_r: RTCIENR: %04x\n", machine().describe_context(), m_rtcienr);
	return m_rtcienr;
}

void mc68328_base_device::stpwtch_w(offs_t offset, u16 data, u16 mem_mask) // 0xb12
{
	LOGMASKED(LOG_RTC, "%s: stpwtch_w: STPWTCH = %04x\n", machine().describe_context(), data);
	m_stpwtch = data & 0x003f;
}

u16 mc68328_base_device::stpwtch_r() // 0xb12
{
	LOGMASKED(LOG_RTC, "%s: stpwtch_r: STPWTCH: %04x\n", machine().describe_context(), m_stpwtch);
	return m_stpwtch;
}


//-------------------------------------------------
//  RTC/alarm hardware - EZ variant
//-------------------------------------------------

bool mc68ez328_device::rtc_int_is_active()
{
	return (m_rtcisr & m_rtcienr) & RTCINT_RTCIRQ_MASK;
}

void mc68ez328_device::rtc_advance_seconds()
{
	LOGMASKED(LOG_RTC, "EZ advancing seconds!\n");
	const u32 old_hmsr = m_hmsr;
	mc68328_base_device::rtc_advance_seconds();

	if ((old_hmsr & RTCHMSR_HOURS) != (m_hmsr & RTCHMSR_HOURS))
	{
		m_rtcisr |= RTCINT_HOUR;

		if (((m_hmsr & RTCHMSR_HOURS) >> RTCHMSR_HOURS_SHIFT) == 0)
		{
			m_dayr = (m_dayr + 1) & RTC_DAYS_MASK;
		}
	}

	if (BIT(m_watchdog, WATCHDOG_EN_BIT))
	{
		m_watchdog += 1 << WATCHDOG_CNT_SHIFT;
		m_watchdog &= (WATCHDOG_MASK | WATCHDOG_CNT_MASK);
		if (((m_watchdog & WATCHDOG_CNT_MASK) >> WATCHDOG_CNT_SHIFT) == 2)
		{
			if (BIT(m_watchdog, WATCHDOG_ISEL_BIT))
			{
				set_interrupt_line(INT_WDT, 1);
			}
			else
			{
				reset();
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(mc68ez328_device::sample_timer_tick)
{
	if (!BIT(m_rtcctl, RTCCTL_ENABLE_BIT) && !BIT(m_watchdog, WATCHDOG_EN_BIT))
	{
		return;
	}

	const u8 old_sam_cnt = m_sam_cnt;
	m_sam_cnt++;

	const bool rtc_int_was_active = rtc_int_is_active();

	m_rtcisr |= RTCINT_SAM0;
	for (u8 i = 0; i < 7; i++)
	{
		if (BIT(old_sam_cnt, i) && !BIT(m_sam_cnt, i))
		{
			m_rtcisr |= RTCINT_SAM1 << i;
		}
	}

	if (!rtc_int_was_active && rtc_int_is_active())
	{
		set_interrupt_line(INT_RTC, 1);
	}
}

u16 mc68ez328_device::rtc_get_int_mask()
{
	constexpr u16 RTCIENR_MASK = 0xff3f;
	return RTCIENR_MASK;
}

bool mc68ez328_device::rtc_get_alarm_match()
{
	return m_hmsr == m_alarm && m_dayr == m_dayalarm;
}

void mc68ez328_device::watchdog_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: watchdog_w: WATCHDOG = %04x\n", machine().describe_context(), data);
	const u16 old_watchdog = m_watchdog;
	m_watchdog = data & WATCHDOG_MASK;
	if (BIT(data, WATCHDOG_INTF_BIT))
	{
		m_watchdog &= ~WATCHDOG_INTF;
		if (BIT(old_watchdog, WATCHDOG_INTF_BIT))
		{
			set_interrupt_line(INT_WDT, 0);
		}
	}
}

u16 mc68ez328_device::watchdog_r()
{
	LOGMASKED(LOG_RTC, "%s: watchdog_r: WATCHDOG: %04x\n", machine().describe_context(), m_watchdog);
	return m_watchdog;
}

void mc68ez328_device::rtcctl_w(offs_t offset, u16 data, u16 mem_mask) // 0xb0c
{
	const u16 old_rtcctl = m_rtcctl;
	mc68328_base_device::rtcctl_w(offset, data, mem_mask);

	if (BIT(old_rtcctl, RTCCTL_38_4_BIT) != BIT(m_rtcctl, RTCCTL_38_4_BIT))
	{
		const u32 frequency = BIT(m_rtcctl, RTCCTL_38_4_BIT) ? 38400 : 32768;
		m_rtc_sample_timer->adjust(attotime::from_ticks(64, frequency), 0, attotime::from_ticks(64, frequency));
	}
}

void mc68ez328_device::dayr_w(offs_t offset, u16 data, u16 mem_mask) // 0xb1a
{
	LOGMASKED(LOG_RTC, "%s: dayr_w: DAYR = %04x\n", machine().describe_context(), data);
	m_dayr = data & RTC_DAYS_MASK;
}

void mc68ez328_device::dayalarm_w(offs_t offset, u16 data, u16 mem_mask) // 0xb1c
{
	LOGMASKED(LOG_RTC, "%s: dayalarm_w: DAYALARM = %04x\n", machine().describe_context(), data);
	m_dayalarm = data & RTC_DAYS_MASK;
}

u16 mc68ez328_device::dayr_r() // 0xb1a
{
	LOGMASKED(LOG_RTC, "%s: dayalarm_r: DAYR: %04x\n", machine().describe_context(), m_dayr);
	return m_dayr;
}

u16 mc68ez328_device::dayalarm_r() // 0xb1c
{
	LOGMASKED(LOG_RTC, "%s: dayalarm_r: DAYALARM: %04x\n", machine().describe_context(), m_dayalarm);
	return m_dayalarm;
}
