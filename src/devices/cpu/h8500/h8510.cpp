// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi H8/510

***************************************************************************/

#include "emu.h"
#include "h8510.h"

DEFINE_DEVICE_TYPE(HD6415108, hd6415108_device, "hd6415108", "Hitachi HD6415108 (H8/510)")

h8510_device::h8510_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8500_device(mconfig, type, tag, owner, clock, 24, 16, 0, 4, address_map_constructor(FUNC(h8510_device::internal_map), this))
{
}

hd6415108_device::hd6415108_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8510_device(mconfig, HD6415108, tag, owner, clock)
{
}

void h8510_device::internal_map(address_map &map)
{
#if 0
	map(0xfe80, 0xfe80).w(FUNC(h8510_device::p1ddr_w));
	map(0xfe81, 0xfe81).w(FUNC(h8510_device::p2ddr_w));
	map(0xfe82, 0xfe82).rw(FUNC(h8510_device::p1dr_r), FUNC(h8510_device::p1dr_w));
	map(0xfe83, 0xfe83).rw(FUNC(h8510_device::p2dr_r), FUNC(h8510_device::p2dr_w));
	map(0xfe84, 0xfe84).w(FUNC(h8510_device::p3ddr_w));
	map(0xfe85, 0xfe85).w(FUNC(h8510_device::p4ddr_w));
	map(0xfe86, 0xfe86).rw(FUNC(h8510_device::p3dr_r), FUNC(h8510_device::p3dr_w));
	map(0xfe87, 0xfe87).rw(FUNC(h8510_device::p4dr_r), FUNC(h8510_device::p4dr_w));
	map(0xfe88, 0xfe88).w(FUNC(h8510_device::p5ddr_w));
	map(0xfe89, 0xfe89).w(FUNC(h8510_device::p6ddr_w));
	map(0xfe8a, 0xfe8a).rw(FUNC(h8510_device::p5dr_r), FUNC(h8510_device::p5dr_w));
	map(0xfe8b, 0xfe8b).rw(FUNC(h8510_device::p6dr_r), FUNC(h8510_device::p6dr_w));
	map(0xfe8d, 0xfe8d).w(FUNC(h8510_device::p8ddr_w));
	map(0xfe8e, 0xfe8e).r(FUNC(h8510_device::p7dr_r));
	map(0xfe8f, 0xfe8f).rw(FUNC(h8510_device::p8dr_r), FUNC(h8510_device::p8dr_w));
	map(0xfe90, 0xfe90).r(FUNC(h8510_device::addrah_r));
	map(0xfe91, 0xfe91).r(FUNC(h8510_device::addral_r));
	map(0xfe92, 0xfe92).r(FUNC(h8510_device::addrbh_r));
	map(0xfe93, 0xfe93).r(FUNC(h8510_device::addrbl_r));
	map(0xfe94, 0xfe94).r(FUNC(h8510_device::addrch_r));
	map(0xfe95, 0xfe95).r(FUNC(h8510_device::addrcl_r));
	map(0xfe96, 0xfe96).r(FUNC(h8510_device::addrdh_r));
	map(0xfe97, 0xfe97).r(FUNC(h8510_device::addrdl_r));
	map(0xfe98, 0xfe98).rw(FUNC(h8510_device::adcsr_r), FUNC(h8510_device::adcsr_w));
	map(0xfe99, 0xfe99).rw(FUNC(h8510_device::adcr_r), FUNC(h8510_device::adcr_w));
	map(0xfea0, 0xfea0).rw(FUNC(h8510_device::frt1_tcr_r), FUNC(h8510_device::frt1_tcr_w));
	map(0xfea1, 0xfea1).rw(FUNC(h8510_device::frt1_tcsr_r), FUNC(h8510_device::frt1_tcsr_w));
	map(0xfea2, 0xfea2).rw(FUNC(h8510_device::frt1_frch_r), FUNC(h8510_device::frt1_frch_w));
	map(0xfea3, 0xfea3).rw(FUNC(h8510_device::frt1_frcl_r), FUNC(h8510_device::frt1_frcl_w));
	map(0xfea4, 0xfea4).rw(FUNC(h8510_device::frt1_ocrah_r), FUNC(h8510_device::frt1_ocrah_w));
	map(0xfea5, 0xfea5).rw(FUNC(h8510_device::frt1_ocral_r), FUNC(h8510_device::frt1_ocral_w));
	map(0xfea6, 0xfea6).rw(FUNC(h8510_device::frt1_ocrbh_r), FUNC(h8510_device::frt1_ocrbh_w));
	map(0xfea7, 0xfea7).rw(FUNC(h8510_device::frt1_ocrbl_r), FUNC(h8510_device::frt1_ocrbl_w));
	map(0xfea8, 0xfea8).r(FUNC(h8510_device::frt1_icrh_r));
	map(0xfea9, 0xfea9).r(FUNC(h8510_device::frt1_icrl_r));
	map(0xfeb0, 0xfeb0).rw(FUNC(h8510_device::frt2_tcr_r), FUNC(h8510_device::frt2_tcr_w));
	map(0xfeb1, 0xfeb1).rw(FUNC(h8510_device::frt2_tcsr_r), FUNC(h8510_device::frt2_tcsr_w));
	map(0xfeb2, 0xfeb2).rw(FUNC(h8510_device::frt2_frch_r), FUNC(h8510_device::frt2_frch_w));
	map(0xfeb3, 0xfeb3).rw(FUNC(h8510_device::frt2_frcl_r), FUNC(h8510_device::frt2_frcl_w));
	map(0xfeb4, 0xfeb4).rw(FUNC(h8510_device::frt2_ocrah_r), FUNC(h8510_device::frt2_ocrah_w));
	map(0xfeb5, 0xfeb5).rw(FUNC(h8510_device::frt2_ocral_r), FUNC(h8510_device::frt2_ocral_w));
	map(0xfeb6, 0xfeb6).rw(FUNC(h8510_device::frt2_ocrbh_r), FUNC(h8510_device::frt2_ocrbh_w));
	map(0xfeb7, 0xfeb7).rw(FUNC(h8510_device::frt2_ocrbl_r), FUNC(h8510_device::frt2_ocrbl_w));
	map(0xfeb8, 0xfeb8).r(FUNC(h8510_device::frt2_icrh_r));
	map(0xfeb9, 0xfeb9).r(FUNC(h8510_device::frt2_icrl_r));
	map(0xfec0, 0xfec0).rw(FUNC(h8510_device::tmr_tcr_r), FUNC(h8510_device::tmr_tcr_w));
	map(0xfec1, 0xfec1).rw(FUNC(h8510_device::tmr_tcsr_r), FUNC(h8510_device::tmr_tcsr_w));
	map(0xfec2, 0xfec2).rw(FUNC(h8510_device::tmr_tcora_r), FUNC(h8510_device::tmr_tcora_w));
	map(0xfec3, 0xfec3).rw(FUNC(h8510_device::tmr_tcorb_r), FUNC(h8510_device::tmr_tcorb_w));
	map(0xfec4, 0xfec4).rw(FUNC(h8510_device::tmr_tcnt_r), FUNC(h8510_device::tmr_tcnt_w));
	map(0xfec8, 0xfec8).rw(FUNC(h8510_device::sci1_smr_r), FUNC(h8510_device::sci1_smr_w));
	map(0xfec9, 0xfec9).rw(FUNC(h8510_device::sci1_brr_r), FUNC(h8510_device::sci1_brr_w));
	map(0xfeca, 0xfeca).rw(FUNC(h8510_device::sci1_scr_r), FUNC(h8510_device::sci1_scr_w));
	map(0xfecb, 0xfecb).rw(FUNC(h8510_device::sci1_tdr_r), FUNC(h8510_device::sci1_tdr_w));
	map(0xfecc, 0xfecc).rw(FUNC(h8510_device::sci1_ssr_r), FUNC(h8510_device::sci1_ssr_w));
	map(0xfecd, 0xfecd).r(FUNC(h8510_device::sci1_rdr_r));
	map(0xfed0, 0xfed0).rw(FUNC(h8510_device::sci2_smr_r), FUNC(h8510_device::sci2_smr_w));
	map(0xfed1, 0xfed1).rw(FUNC(h8510_device::sci2_brr_r), FUNC(h8510_device::sci2_brr_w));
	map(0xfed2, 0xfed2).rw(FUNC(h8510_device::sci2_scr_r), FUNC(h8510_device::sci2_scr_w));
	map(0xfed3, 0xfed3).rw(FUNC(h8510_device::sci2_tdr_r), FUNC(h8510_device::sci2_tdr_w));
	map(0xfed4, 0xfed4).rw(FUNC(h8510_device::sci2_ssr_r), FUNC(h8510_device::sci2_ssr_w));
	map(0xfed5, 0xfed5).r(FUNC(h8510_device::sci2_rdr_r));
	map(0xfed8, 0xfed8).rw(FUNC(h8510_device::rfshcr_r), FUNC(h8510_device::rfshcr_w));
	map(0xff00, 0xff00).rw(FUNC(h8510_device::ipra_r), FUNC(h8510_device::ipra_w));
	map(0xff01, 0xff01).rw(FUNC(h8510_device::iprb_r), FUNC(h8510_device::iprb_w));
	map(0xff02, 0xff02).rw(FUNC(h8510_device::iprc_r), FUNC(h8510_device::iprc_w));
	map(0xff03, 0xff03).rw(FUNC(h8510_device::iprd_r), FUNC(h8510_device::iprd_w));
	map(0xff08, 0xff08).rw(FUNC(h8510_device::dtea_r), FUNC(h8510_device::dtea_w));
	map(0xff09, 0xff09).rw(FUNC(h8510_device::dteb_r), FUNC(h8510_device::dteb_w));
	map(0xff0a, 0xff0a).rw(FUNC(h8510_device::dtec_r), FUNC(h8510_device::dtec_w));
	map(0xff0b, 0xff0b).rw(FUNC(h8510_device::dted_r), FUNC(h8510_device::dted_w));
	map(0xff10, 0xff10).r(FUNC(h8510_device::wdt_tcsr_r));
	map(0xff11, 0xff11).r(FUNC(h8510_device::wdt_tcnt_r));
	map(0xff10, 0xff10).w(FUNC(h8510_device::wdt_tcnt_tcsr_w));
	map(0xff14, 0xff14).rw(FUNC(h8510_device::wcr_r), FUNC(h8510_device::wcr_w));
	map(0xff16, 0xff16).rw(FUNC(h8510_device::arbt_r), FUNC(h8510_device::arbt_w));
	map(0xff17, 0xff17).rw(FUNC(h8510_device::ar3t_r), FUNC(h8510_device::ar3t_w));
	map(0xff19, 0xff19).r(FUNC(h8510_device::mdcr_r));
	map(0xff1a, 0xff1a).rw(FUNC(h8510_device::sbycr_r), FUNC(h8510_device::sbycr_w));
	map(0xff1b, 0xff1b).rw(FUNC(h8510_device::brcr_r), FUNC(h8510_device::brcr_w));
	map(0xff1c, 0xff1c).rw(FUNC(h8510_device::nmicr_r), FUNC(h8510_device::nmicr_w));
	map(0xff1d, 0xff1d).rw(FUNC(h8510_device::irqcr_r), FUNC(h8510_device::irqcr_w));
	map(0xff1e, 0xff1e).r(FUNC(h8510_device::wdt_rstcsr_r));
	map(0xff1e, 0xff1f).w(FUNC(h8510_device::wdt_rstcsr_w));
#endif
}
