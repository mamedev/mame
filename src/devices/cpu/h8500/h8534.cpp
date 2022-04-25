// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi H8/534 & H8/536

***************************************************************************/

#include "emu.h"
#include "h8534.h"

DEFINE_DEVICE_TYPE(HD6435348, hd6435348_device, "hd6435348", "Hitachi HD6435348 (H8/534)")
DEFINE_DEVICE_TYPE(HD6475348, hd6475348_device, "hd6475348", "Hitachi HD6475348 (H8/534)")
DEFINE_DEVICE_TYPE(HD6435368, hd6435368_device, "hd6435368", "Hitachi HD6435368 (H8/536)")
DEFINE_DEVICE_TYPE(HD6475368, hd6475368_device, "hd6475368", "Hitachi HD6475368 (H8/536)")

h8534_device::h8534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: h8500_device(mconfig, type, tag, owner, clock, 20, 8, 11, 4, map)
{
}

h8534_device::h8534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8534_device::internal_map), this))
{
}

hd6435348_device::hd6435348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, HD6435348, tag, owner, clock)
{
}

hd6475348_device::hd6475348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, HD6475348, tag, owner, clock)
{
}

h8536_device::h8536_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8536_device::internal_map), this))
{
}

hd6435368_device::hd6435368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8536_device(mconfig, HD6435368, tag, owner, clock)
{
}

hd6475368_device::hd6475368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8536_device(mconfig, HD6475368, tag, owner, clock)
{
}

void h8534_device::internal_map(address_map &map)
{
	if (mode_control() == 2 || mode_control() == 4 || mode_control() == 7)
		map(0x0000, 0x7fff).rom().region(DEVICE_SELF, 0);
	register_field_map(map);
}

void h8536_device::internal_map(address_map &map)
{
	if (mode_control() == 2)
		map(0x0000, 0xee7f).rom().region(DEVICE_SELF, 0);
	else if (mode_control() == 4 || mode_control() == 7)
		map(0x0000, 0xf67f).rom().region(DEVICE_SELF, 0);
	register_field_map(map);
}

void h8534_device::register_field_map(address_map &map)
{
#if 0
	map(0xfe80, 0xfe80).w(FUNC(h8534_device::p1ddr_w));
	map(0xfe81, 0xfe81).w(FUNC(h8534_device::p2ddr_w));
	map(0xfe82, 0xfe82).rw(FUNC(h8534_device::p1dr_r), FUNC(h8534_device::p1dr_w));
	map(0xfe83, 0xfe83).rw(FUNC(h8534_device::p2dr_r), FUNC(h8534_device::p2dr_w));
	map(0xfe84, 0xfe84).w(FUNC(h8534_device::p3ddr_w));
	map(0xfe85, 0xfe85).w(FUNC(h8534_device::p4ddr_w));
	map(0xfe86, 0xfe86).rw(FUNC(h8534_device::p3dr_r), FUNC(h8534_device::p3dr_w));
	map(0xfe87, 0xfe87).rw(FUNC(h8534_device::p4dr_r), FUNC(h8534_device::p4dr_w));
	map(0xfe88, 0xfe88).w(FUNC(h8534_device::p5ddr_w));
	map(0xfe89, 0xfe89).w(FUNC(h8534_device::p6ddr_w));
	map(0xfe8a, 0xfe8a).rw(FUNC(h8534_device::p5dr_r), FUNC(h8534_device::p5dr_w));
	map(0xfe8b, 0xfe8b).rw(FUNC(h8534_device::p6dr_r), FUNC(h8534_device::p6dr_w));
	map(0xfe8c, 0xfe8c).w(FUNC(h8534_device::p7ddr_w));
	map(0xfe8e, 0xfe8e).rw(FUNC(h8534_device::p7dr_r), FUNC(h8534_device::p7dr_w));
	map(0xfe8f, 0xfe8f).r(FUNC(h8534_device::p8dr_r));
	map(0xfe90, 0xfe90).rw(FUNC(h8534_device::frt1_tcr_r), FUNC(h8534_device::frt1_tcr_w));
	map(0xfe91, 0xfe91).rw(FUNC(h8534_device::frt1_tcsr_r), FUNC(h8534_device::frt1_tcsr_w));
	map(0xfe92, 0xfe92).rw(FUNC(h8534_device::frt1_frch_r), FUNC(h8534_device::frt1_frch_w));
	map(0xfe93, 0xfe93).rw(FUNC(h8534_device::frt1_frcl_r), FUNC(h8534_device::frt1_frcl_w));
	map(0xfe94, 0xfe94).rw(FUNC(h8534_device::frt1_ocrah_r), FUNC(h8534_device::frt1_ocrah_w));
	map(0xfe95, 0xfe95).rw(FUNC(h8534_device::frt1_ocral_r), FUNC(h8534_device::frt1_ocral_w));
	map(0xfe96, 0xfe96).rw(FUNC(h8534_device::frt1_ocrbh_r), FUNC(h8534_device::frt1_ocrbh_w));
	map(0xfe97, 0xfe97).rw(FUNC(h8534_device::frt1_ocrbl_r), FUNC(h8534_device::frt1_ocrbl_w));
	map(0xfe98, 0xfe98).r(FUNC(h8534_device::frt1_icrh_r));
	map(0xfe99, 0xfe99).r(FUNC(h8534_device::frt1_icrl_r));
	map(0xfea0, 0xfea0).rw(FUNC(h8534_device::frt2_tcr_r), FUNC(h8534_device::frt2_tcr_w));
	map(0xfea1, 0xfea1).rw(FUNC(h8534_device::frt2_tcsr_r), FUNC(h8534_device::frt2_tcsr_w));
	map(0xfea2, 0xfea2).rw(FUNC(h8534_device::frt2_frch_r), FUNC(h8534_device::frt2_frch_w));
	map(0xfea3, 0xfea3).rw(FUNC(h8534_device::frt2_frcl_r), FUNC(h8534_device::frt2_frcl_w));
	map(0xfea4, 0xfea4).rw(FUNC(h8534_device::frt2_ocrah_r), FUNC(h8534_device::frt2_ocrah_w));
	map(0xfea5, 0xfea5).rw(FUNC(h8534_device::frt2_ocral_r), FUNC(h8534_device::frt2_ocral_w));
	map(0xfea6, 0xfea6).rw(FUNC(h8534_device::frt2_ocrbh_r), FUNC(h8534_device::frt2_ocrbh_w));
	map(0xfea7, 0xfea7).rw(FUNC(h8534_device::frt2_ocrbl_r), FUNC(h8534_device::frt2_ocrbl_w));
	map(0xfea8, 0xfea8).r(FUNC(h8534_device::frt2_icrh_r));
	map(0xfea9, 0xfea9).r(FUNC(h8534_device::frt2_icrl_r));
	map(0xfeb0, 0xfeb0).rw(FUNC(h8534_device::frt3_tcr_r), FUNC(h8534_device::frt3_tcr_w));
	map(0xfeb1, 0xfeb1).rw(FUNC(h8534_device::frt3_tcsr_r), FUNC(h8534_device::frt3_tcsr_w));
	map(0xfeb2, 0xfeb2).rw(FUNC(h8534_device::frt3_frch_r), FUNC(h8534_device::frt3_frch_w));
	map(0xfeb3, 0xfeb3).rw(FUNC(h8534_device::frt3_frcl_r), FUNC(h8534_device::frt3_frcl_w));
	map(0xfeb4, 0xfeb4).rw(FUNC(h8534_device::frt3_ocrah_r), FUNC(h8534_device::frt3_ocrah_w));
	map(0xfeb5, 0xfeb5).rw(FUNC(h8534_device::frt3_ocral_r), FUNC(h8534_device::frt3_ocral_w));
	map(0xfeb6, 0xfeb6).rw(FUNC(h8534_device::frt3_ocrbh_r), FUNC(h8534_device::frt3_ocrbh_w));
	map(0xfeb7, 0xfeb7).rw(FUNC(h8534_device::frt3_ocrbl_r), FUNC(h8534_device::frt3_ocrbl_w));
	map(0xfeb8, 0xfeb8).r(FUNC(h8534_device::frt3_icrh_r));
	map(0xfeb9, 0xfeb9).r(FUNC(h8534_device::frt3_icrl_r));
	map(0xfec0, 0xfec0).rw(FUNC(h8534_device::pwm1_tcr_r), FUNC(h8534_device::pwm1_tcr_w));
	map(0xfec1, 0xfec1).rw(FUNC(h8534_device::pwm1_dtr_r), FUNC(h8534_device::pwm1_dtr_w));
	map(0xfec2, 0xfec2).rw(FUNC(h8534_device::pwm1_tcnt_r), FUNC(h8534_device::pwm1_tcnt_w));
	map(0xfec4, 0xfec4).rw(FUNC(h8534_device::pwm2_tcr_r), FUNC(h8534_device::pwm2_tcr_w));
	map(0xfec5, 0xfec5).rw(FUNC(h8534_device::pwm2_dtr_r), FUNC(h8534_device::pwm2_dtr_w));
	map(0xfec6, 0xfec6).rw(FUNC(h8534_device::pwm2_tcnt_r), FUNC(h8534_device::pwm2_tcnt_w));
	map(0xfec8, 0xfec8).rw(FUNC(h8534_device::pwm3_tcr_r), FUNC(h8534_device::pwm3_tcr_w));
	map(0xfec9, 0xfec9).rw(FUNC(h8534_device::pwm3_dtr_r), FUNC(h8534_device::pwm3_dtr_w));
	map(0xfeca, 0xfeca).rw(FUNC(h8534_device::pwm3_tcnt_r), FUNC(h8534_device::pwm3_tcnt_w));
	map(0xfed0, 0xfed0).rw(FUNC(h8534_device::tmr_tcr_r), FUNC(h8534_device::tmr_tcr_w));
	map(0xfed1, 0xfed1).rw(FUNC(h8534_device::tmr_tcsr_r), FUNC(h8534_device::tmr_tcsr_w));
	map(0xfed2, 0xfed2).rw(FUNC(h8534_device::tmr_tcora_r), FUNC(h8534_device::tmr_tcora_w));
	map(0xfed3, 0xfed3).rw(FUNC(h8534_device::tmr_tcorb_r), FUNC(h8534_device::tmr_tcorb_w));
	map(0xfed4, 0xfed4).rw(FUNC(h8534_device::tmr_tcnt_r), FUNC(h8534_device::tmr_tcnt_w));
	map(0xfed8, 0xfed8).rw(FUNC(h8534_device::sci1_smr_r), FUNC(h8534_device::sci1_smr_w));
	map(0xfed9, 0xfed9).rw(FUNC(h8534_device::sci1_brr_r), FUNC(h8534_device::sci1_brr_w));
	map(0xfeda, 0xfeda).rw(FUNC(h8534_device::sci1_scr_r), FUNC(h8534_device::sci1_scr_w));
	map(0xfedb, 0xfedb).rw(FUNC(h8534_device::sci1_tdr_r), FUNC(h8534_device::sci1_tdr_w));
	map(0xfedc, 0xfedc).rw(FUNC(h8534_device::sci1_ssr_r), FUNC(h8534_device::sci1_ssr_w));
	map(0xfedd, 0xfedd).r(FUNC(h8534_device::sci1_rdr_r));
	map(0xfee0, 0xfee0).r(FUNC(h8534_device::addrah_r));
	map(0xfee1, 0xfee1).r(FUNC(h8534_device::addral_r));
	map(0xfee2, 0xfee2).r(FUNC(h8534_device::addrbh_r));
	map(0xfee3, 0xfee3).r(FUNC(h8534_device::addrbl_r));
	map(0xfee4, 0xfee4).r(FUNC(h8534_device::addrch_r));
	map(0xfee5, 0xfee5).r(FUNC(h8534_device::addrcl_r));
	map(0xfee6, 0xfee6).r(FUNC(h8534_device::addrdh_r));
	map(0xfee7, 0xfee7).r(FUNC(h8534_device::addrdl_r));
	map(0xfee8, 0xfee8).rw(FUNC(h8534_device::adcsr_r), FUNC(h8534_device::adcsr_w));
	map(0xfee9, 0xfee9).rw(FUNC(h8534_device::adcr_r), FUNC(h8534_device::adcr_w));
	map(0xfeec, 0xfeec).rw(FUNC(h8534_device::wdt_tcsr_r));
	map(0xfeed, 0xfeed).r(FUNC(h8534_device::wdt_tcnt_r), FUNC(h8534_device::wdt_tcsr_w));
	map(0xfef0, 0xfef0).rw(FUNC(h8534_device::sci2_smr_r), FUNC(h8534_device::sci2_smr_w));
	map(0xfef1, 0xfef1).rw(FUNC(h8534_device::sci2_brr_r), FUNC(h8534_device::sci2_brr_w));
	map(0xfef2, 0xfef2).rw(FUNC(h8534_device::sci2_scr_r), FUNC(h8534_device::sci2_scr_w));
	map(0xfef3, 0xfef3).rw(FUNC(h8534_device::sci2_tdr_r), FUNC(h8534_device::sci2_tdr_w));
	map(0xfef4, 0xfef4).rw(FUNC(h8534_device::sci2_ssr_r), FUNC(h8534_device::sci2_ssr_w));
	map(0xfef5, 0xfef5).r(FUNC(h8534_device::sci2_rdr_r));
	map(0xfefc, 0xfefc).rw(FUNC(h8534_device::syscr1_r), FUNC(h8534_device::syscr1_w));
	map(0xfefd, 0xfefd).rw(FUNC(h8534_device::syscr2_r), FUNC(h8534_device::syscr2_w));
	map(0xfefe, 0xfefe).w(FUNC(h8534_device::p9ddr_w));
	map(0xfeff, 0xfeff).rw(FUNC(h8534_device::p9dr_r), FUNC(h8534_device::p9dr_w));
	map(0xff00, 0xff00).rw(FUNC(h8534_device::ipra_r), FUNC(h8534_device::ipra_w));
	map(0xff01, 0xff01).rw(FUNC(h8534_device::iprb_r), FUNC(h8534_device::iprb_w));
	map(0xff02, 0xff02).rw(FUNC(h8534_device::iprc_r), FUNC(h8534_device::iprc_w));
	map(0xff03, 0xff03).rw(FUNC(h8534_device::iprd_r), FUNC(h8534_device::iprd_w));
	map(0xff04, 0xff04).rw(FUNC(h8534_device::ipre_r), FUNC(h8534_device::ipre_w));
	map(0xff05, 0xff05).rw(FUNC(h8534_device::iprf_r), FUNC(h8534_device::iprf_w));
	map(0xff08, 0xff08).rw(FUNC(h8534_device::dtea_r), FUNC(h8534_device::dtea_w));
	map(0xff09, 0xff09).rw(FUNC(h8534_device::dteb_r), FUNC(h8534_device::dteb_w));
	map(0xff0a, 0xff0a).rw(FUNC(h8534_device::dtec_r), FUNC(h8534_device::dtec_w));
	map(0xff0b, 0xff0b).rw(FUNC(h8534_device::dted_r), FUNC(h8534_device::dted_w));
	map(0xff0c, 0xff0c).rw(FUNC(h8534_device::dtee_r), FUNC(h8534_device::dtee_w));
	map(0xff0d, 0xff0d).rw(FUNC(h8534_device::dtef_r), FUNC(h8534_device::dtef_w));
	map(0xff10, 0xff10).rw(FUNC(h8534_device::wcr_r), FUNC(h8534_device::wcr_w));
	map(0xff11, 0xff11).rw(FUNC(h8534_device::ramcr_r), FUNC(h8534_device::ramcr_w));
	map(0xff12, 0xff12).r(FUNC(h8534_device::mdcr_r));
	map(0xff13, 0xff13).rw(FUNC(h8534_device::sbycr_r), FUNC(h8534_device::sbycr_w));
	map(0xff14, 0xff14).w(FUNC(h8534_device::wdt_rstcsr_w));
	map(0xff15, 0xff15).r(FUNC(h8534_device::wdt_rstcsr_r));
#endif
}
