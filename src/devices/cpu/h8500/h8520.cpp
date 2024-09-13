// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi H8/520

***************************************************************************/

#include "emu.h"
#include "h8520.h"

DEFINE_DEVICE_TYPE(HD6435208, hd6435208_device, "hd6435208", "Hitachi HD6435208 (H8/520)")
DEFINE_DEVICE_TYPE(HD6475208, hd6475208_device, "hd6475208", "Hitachi HD6475208 (H8/520)")

h8520_device::h8520_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8500_device(mconfig, type, tag, owner, clock, 20, 8, 9, 4, address_map_constructor(FUNC(h8520_device::internal_map), this))
{
}

hd6435208_device::hd6435208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8520_device(mconfig, HD6435208, tag, owner, clock)
{
}

hd6475208_device::hd6475208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8520_device(mconfig, HD6475208, tag, owner, clock)
{
}

void h8520_device::internal_map(address_map &map)
{
	if (mode_control() == 2 || mode_control() == 4 || mode_control() == 7)
		map(0x0000, 0x3fff).rom().region(DEVICE_SELF, 0);
	map(0xfd80, 0xff7f).ram(); // TODO: may be disabled by writing 0 to RAME bit in RAMCR
#if 0
	map(0xff80, 0xff80).w(FUNC(h8520_device::p1ddr_w));
	map(0xff81, 0xff81).w(FUNC(h8520_device::p2ddr_w));
	map(0xff82, 0xff82).rw(FUNC(h8520_device::p1dr_r), FUNC(h8520_device::p1dr_w));
	map(0xff83, 0xff83).rw(FUNC(h8520_device::p2dr_r), FUNC(h8520_device::p2dr_w));
	map(0xff85, 0xff85).w(FUNC(h8520_device::p4ddr_w));
	map(0xff86, 0xff86).rw(FUNC(h8520_device::p3dr_r), FUNC(h8520_device::p3dr_w));
	map(0xff87, 0xff87).rw(FUNC(h8520_device::p4dr_r), FUNC(h8520_device::p4dr_w));
	map(0xff88, 0xff88).w(FUNC(h8520_device::p5ddr_w));
	map(0xff89, 0xff89).w(FUNC(h8520_device::p6ddr_w));
	map(0xff8a, 0xff8a).rw(FUNC(h8520_device::p5dr_r), FUNC(h8520_device::p5dr_w));
	map(0xff8b, 0xff8b).r(FUNC(h8520_device::p6dr_r));
	map(0xff8c, 0xff8c).w(FUNC(h8520_device::p7ddr_w));
	map(0xff8e, 0xff8e).rw(FUNC(h8520_device::p7dr_r), FUNC(h8520_device::p7dr_w));
	map(0xff90, 0xff90).rw(FUNC(h8520_device::frt1_tcr_r), FUNC(h8520_device::frt1_tcr_w));
	map(0xff91, 0xff91).rw(FUNC(h8520_device::frt1_tcsr_r), FUNC(h8520_device::frt1_tcsr_w));
	map(0xff92, 0xff92).rw(FUNC(h8520_device::frt1_frch_r), FUNC(h8520_device::frt1_frch_w));
	map(0xff93, 0xff93).rw(FUNC(h8520_device::frt1_frcl_r), FUNC(h8520_device::frt1_frcl_w));
	map(0xff94, 0xff94).rw(FUNC(h8520_device::frt1_ocrah_r), FUNC(h8520_device::frt1_ocrah_w));
	map(0xff95, 0xff95).rw(FUNC(h8520_device::frt1_ocral_r), FUNC(h8520_device::frt1_ocral_w));
	map(0xff96, 0xff96).rw(FUNC(h8520_device::frt1_ocrbh_r), FUNC(h8520_device::frt1_ocrbh_w));
	map(0xff97, 0xff97).rw(FUNC(h8520_device::frt1_ocrbl_r), FUNC(h8520_device::frt1_ocrbl_w));
	map(0xff98, 0xff98).r(FUNC(h8520_device::frt1_icrh_r));
	map(0xff99, 0xff99).r(FUNC(h8520_device::frt1_icrl_r));
	map(0xffa0, 0xffa0).rw(FUNC(h8520_device::frt2_tcr_r), FUNC(h8520_device::frt2_tcr_w));
	map(0xffa1, 0xffa1).rw(FUNC(h8520_device::frt2_tcsr_r), FUNC(h8520_device::frt2_tcsr_w));
	map(0xffa2, 0xffa2).rw(FUNC(h8520_device::frt2_frch_r), FUNC(h8520_device::frt2_frch_w));
	map(0xffa3, 0xffa3).rw(FUNC(h8520_device::frt2_frcl_r), FUNC(h8520_device::frt2_frcl_w));
	map(0xffa4, 0xffa4).rw(FUNC(h8520_device::frt2_ocrah_r), FUNC(h8520_device::frt2_ocrah_w));
	map(0xffa5, 0xffa5).rw(FUNC(h8520_device::frt2_ocral_r), FUNC(h8520_device::frt2_ocral_w));
	map(0xffa6, 0xffa6).rw(FUNC(h8520_device::frt2_ocrbh_r), FUNC(h8520_device::frt2_ocrbh_w));
	map(0xffa7, 0xffa7).rw(FUNC(h8520_device::frt2_ocrbl_r), FUNC(h8520_device::frt2_ocrbl_w));
	map(0xffa8, 0xffa8).r(FUNC(h8520_device::frt2_icrh_r));
	map(0xffa9, 0xffa9).r(FUNC(h8520_device::frt2_icrl_r));
	map(0xffc0, 0xffc0).rw(FUNC(h8520_device::sci2_smr_r), FUNC(h8520_device::sci2_smr_w));
	map(0xffc1, 0xffc1).rw(FUNC(h8520_device::sci2_brr_r), FUNC(h8520_device::sci2_brr_w));
	map(0xffc2, 0xffc2).rw(FUNC(h8520_device::sci2_scr_r), FUNC(h8520_device::sci2_scr_w));
	map(0xffc3, 0xffc3).rw(FUNC(h8520_device::sci2_tdr_r), FUNC(h8520_device::sci2_tdr_w));
	map(0xffc4, 0xffc4).rw(FUNC(h8520_device::sci2_ssr_r), FUNC(h8520_device::sci2_ssr_w));
	map(0xffc5, 0xffc5).r(FUNC(h8520_device::sci2_rdr_r));
	map(0xffd0, 0xffd0).rw(FUNC(h8520_device::tmr_tcr_r), FUNC(h8520_device::tmr_tcr_w));
	map(0xffd1, 0xffd1).rw(FUNC(h8520_device::tmr_tcsr_r), FUNC(h8520_device::tmr_tcsr_w));
	map(0xffd2, 0xffd2).rw(FUNC(h8520_device::tmr_tcora_r), FUNC(h8520_device::tmr_tcora_w));
	map(0xffd3, 0xffd3).rw(FUNC(h8520_device::tmr_tcorb_r), FUNC(h8520_device::tmr_tcorb_w));
	map(0xffd4, 0xffd4).rw(FUNC(h8520_device::tmr_tcnt_r), FUNC(h8520_device::tmr_tcnt_w));
	map(0xffd8, 0xffd8).rw(FUNC(h8520_device::sci1_smr_r), FUNC(h8520_device::sci1_smr_w));
	map(0xffd9, 0xffd9).rw(FUNC(h8520_device::sci1_brr_r), FUNC(h8520_device::sci1_brr_w));
	map(0xffda, 0xffda).rw(FUNC(h8520_device::sci1_scr_r), FUNC(h8520_device::sci1_scr_w));
	map(0xffdb, 0xffdb).rw(FUNC(h8520_device::sci1_tdr_r), FUNC(h8520_device::sci1_tdr_w));
	map(0xffdc, 0xffdc).rw(FUNC(h8520_device::sci1_ssr_r), FUNC(h8520_device::sci1_ssr_w));
	map(0xffdd, 0xffdd).r(FUNC(h8520_device::sci1_rdr_r));
	map(0xffe0, 0xffe0).r(FUNC(h8520_device::addrah_r));
	map(0xffe1, 0xffe1).r(FUNC(h8520_device::addral_r));
	map(0xffe2, 0xffe2).r(FUNC(h8520_device::addrbh_r));
	map(0xffe3, 0xffe3).r(FUNC(h8520_device::addrbl_r));
	map(0xffe4, 0xffe4).r(FUNC(h8520_device::addrch_r));
	map(0xffe5, 0xffe5).r(FUNC(h8520_device::addrcl_r));
	map(0xffe6, 0xffe6).r(FUNC(h8520_device::addrdh_r));
	map(0xffe7, 0xffe7).r(FUNC(h8520_device::addrdl_r));
	map(0xffe8, 0xffe8).rw(FUNC(h8520_device::adcsr_r), FUNC(h8520_device::adcsr_w));
	map(0xffe9, 0xffe9).rw(FUNC(h8520_device::adcr_r), FUNC(h8520_device::adcr_w));
	map(0xffec, 0xffec).r(FUNC(h8520_device::wdt_tcsr_r));
	map(0xffed, 0xffed).r(FUNC(h8520_device::wdt_tcnt_r));
	map(0xffec, 0xffec).w(FUNC(h8520_device::wdt_tcnt_tcsr_w));
	map(0xfff0, 0xfff0).rw(FUNC(h8520_device::ipra_r), FUNC(h8520_device::ipra_w));
	map(0xfff1, 0xfff1).rw(FUNC(h8520_device::iprb_r), FUNC(h8520_device::iprb_w));
	map(0xfff2, 0xfff2).rw(FUNC(h8520_device::iprc_r), FUNC(h8520_device::iprc_w));
	map(0xfff3, 0xfff3).rw(FUNC(h8520_device::iprd_r), FUNC(h8520_device::iprd_w));
	map(0xfff4, 0xfff4).rw(FUNC(h8520_device::dtea_r), FUNC(h8520_device::dtea_w));
	map(0xfff5, 0xfff5).rw(FUNC(h8520_device::dteb_r), FUNC(h8520_device::dteb_w));
	map(0xfff6, 0xfff6).rw(FUNC(h8520_device::dtec_r), FUNC(h8520_device::dtec_w));
	map(0xfff7, 0xfff7).rw(FUNC(h8520_device::dted_r), FUNC(h8520_device::dted_w));
	map(0xfff8, 0xfff8).rw(FUNC(h8520_device::wcr_r), FUNC(h8520_device::wcr_w));
	map(0xfff9, 0xfff9).rw(FUNC(h8520_device::ramcr_r), FUNC(h8520_device::ramcr_w));
	map(0xfffa, 0xfffa).r(FUNC(h8520_device::mdcr_r));
	map(0xfffb, 0xfffb).rw(FUNC(h8520_device::sbycr_r), FUNC(h8520_device::sbycr_w));
	map(0xfffc, 0xfffc).rw(FUNC(h8520_device::nmicr_r), FUNC(h8520_device::nmicr_w));
	map(0xfffd, 0xfffd).rw(FUNC(h8520_device::irqcr_r), FUNC(h8520_device::irqcr_w));
	map(0xfffe, 0xffff).w(FUNC(h8520_device::wdt_rstcsr_w));
	map(0xffff, 0xffff).r(FUNC(h8520_device::wdt_rstcsr_r));
#endif
}
