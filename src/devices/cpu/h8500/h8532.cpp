// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi H8/532

***************************************************************************/

#include "emu.h"
#include "h8532.h"

DEFINE_DEVICE_TYPE(HD6435328, hd6435328_device, "hd6435328", "Hitachi HD6435328 (H8/532)")
DEFINE_DEVICE_TYPE(HD6475328, hd6475328_device, "hd6475328", "Hitachi HD6475328 (H8/532)")

h8532_device::h8532_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8500_device(mconfig, type, tag, owner, clock, 20, 8, 10, 4, address_map_constructor(FUNC(h8532_device::internal_map), this))
{
}

hd6435328_device::hd6435328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8532_device(mconfig, HD6435328, tag, owner, clock)
{
}

hd6475328_device::hd6475328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8532_device(mconfig, HD6475328, tag, owner, clock)
{
}

void h8532_device::internal_map(address_map &map)
{
	if (mode_control() == 2 || mode_control() == 4 || mode_control() == 7)
		map(0x0000, 0x7fff).rom().region(DEVICE_SELF, 0);
#if 0
	map(0xff80, 0xff80).w(FUNC(h8532_device::p1ddr_w));
	map(0xff81, 0xff81).w(FUNC(h8532_device::p2ddr_w));
	map(0xff82, 0xff82).rw(FUNC(h8532_device::p1dr_r), FUNC(h8532_device::p1dr_w));
	map(0xff83, 0xff83).rw(FUNC(h8532_device::p2dr_r), FUNC(h8532_device::p2dr_w));
	map(0xff85, 0xff85).w(FUNC(h8532_device::p4ddr_w));
	map(0xff86, 0xff86).rw(FUNC(h8532_device::p3dr_r), FUNC(h8532_device::p3dr_w));
	map(0xff87, 0xff87).rw(FUNC(h8532_device::p4dr_r), FUNC(h8532_device::p4dr_w));
	map(0xff88, 0xff88).w(FUNC(h8532_device::p5ddr_w));
	map(0xff89, 0xff89).w(FUNC(h8532_device::p6ddr_w));
	map(0xff8a, 0xff8a).rw(FUNC(h8532_device::p5dr_r), FUNC(h8532_device::p5dr_w));
	map(0xff8b, 0xff8b).rw(FUNC(h8532_device::p6dr_r), FUNC(h8532_device::p6dr_w));
	map(0xff8c, 0xff8c).w(FUNC(h8532_device::p7ddr_w));
	map(0xff8e, 0xff8e).rw(FUNC(h8532_device::p7dr_r), FUNC(h8532_device::p7dr_w));
	map(0xff8f, 0xff8f).r(FUNC(h8532_device::p8dr_r));
	map(0xff90, 0xff90).rw(FUNC(h8532_device::frt1_tcr_r), FUNC(h8532_device::frt1_tcr_w));
	map(0xff91, 0xff91).rw(FUNC(h8532_device::frt1_tcsr_r), FUNC(h8532_device::frt1_tcsr_w));
	map(0xff92, 0xff92).rw(FUNC(h8532_device::frt1_frch_r), FUNC(h8532_device::frt1_frch_w));
	map(0xff93, 0xff93).rw(FUNC(h8532_device::frt1_frcl_r), FUNC(h8532_device::frt1_frcl_w));
	map(0xff94, 0xff94).rw(FUNC(h8532_device::frt1_ocrah_r), FUNC(h8532_device::frt1_ocrah_w));
	map(0xff95, 0xff95).rw(FUNC(h8532_device::frt1_ocral_r), FUNC(h8532_device::frt1_ocral_w));
	map(0xff96, 0xff96).rw(FUNC(h8532_device::frt1_ocrbh_r), FUNC(h8532_device::frt1_ocrbh_w));
	map(0xff97, 0xff97).rw(FUNC(h8532_device::frt1_ocrbl_r), FUNC(h8532_device::frt1_ocrbl_w));
	map(0xff98, 0xff98).r(FUNC(h8532_device::frt1_icrh_r));
	map(0xff99, 0xff99).r(FUNC(h8532_device::frt1_icrl_r));
	map(0xffa0, 0xffa0).rw(FUNC(h8532_device::frt2_tcr_r), FUNC(h8532_device::frt2_tcr_w));
	map(0xffa1, 0xffa1).rw(FUNC(h8532_device::frt2_tcsr_r), FUNC(h8532_device::frt2_tcsr_w));
	map(0xffa2, 0xffa2).rw(FUNC(h8532_device::frt2_frch_r), FUNC(h8532_device::frt2_frch_w));
	map(0xffa3, 0xffa3).rw(FUNC(h8532_device::frt2_frcl_r), FUNC(h8532_device::frt2_frcl_w));
	map(0xffa4, 0xffa4).rw(FUNC(h8532_device::frt2_ocrah_r), FUNC(h8532_device::frt2_ocrah_w));
	map(0xffa5, 0xffa5).rw(FUNC(h8532_device::frt2_ocral_r), FUNC(h8532_device::frt2_ocral_w));
	map(0xffa6, 0xffa6).rw(FUNC(h8532_device::frt2_ocrbh_r), FUNC(h8532_device::frt2_ocrbh_w));
	map(0xffa7, 0xffa7).rw(FUNC(h8532_device::frt2_ocrbl_r), FUNC(h8532_device::frt2_ocrbl_w));
	map(0xffa8, 0xffa8).r(FUNC(h8532_device::frt2_icrh_r));
	map(0xffa9, 0xffa9).r(FUNC(h8532_device::frt2_icrl_r));
	map(0xffb0, 0xffb0).rw(FUNC(h8532_device::frt3_tcr_r), FUNC(h8532_device::frt3_tcr_w));
	map(0xffb1, 0xffb1).rw(FUNC(h8532_device::frt3_tcsr_r), FUNC(h8532_device::frt3_tcsr_w));
	map(0xffb2, 0xffb2).rw(FUNC(h8532_device::frt3_frch_r), FUNC(h8532_device::frt3_frch_w));
	map(0xffb3, 0xffb3).rw(FUNC(h8532_device::frt3_frcl_r), FUNC(h8532_device::frt3_frcl_w));
	map(0xffb4, 0xffb4).rw(FUNC(h8532_device::frt3_ocrah_r), FUNC(h8532_device::frt3_ocrah_w));
	map(0xffb5, 0xffb5).rw(FUNC(h8532_device::frt3_ocral_r), FUNC(h8532_device::frt3_ocral_w));
	map(0xffb6, 0xffb6).rw(FUNC(h8532_device::frt3_ocrbh_r), FUNC(h8532_device::frt3_ocrbh_w));
	map(0xffb7, 0xffb7).rw(FUNC(h8532_device::frt3_ocrbl_r), FUNC(h8532_device::frt3_ocrbl_w));
	map(0xffb8, 0xffb8).r(FUNC(h8532_device::frt3_icrh_r));
	map(0xffb9, 0xffb9).r(FUNC(h8532_device::frt3_icrl_r));
	map(0xffc0, 0xffc0).rw(FUNC(h8532_device::pwm1_tcr_r), FUNC(h8532_device::pwm1_tcr_w));
	map(0xffc1, 0xffc1).rw(FUNC(h8532_device::pwm1_dtr_r), FUNC(h8532_device::pwm1_dtr_w));
	map(0xffc2, 0xffc2).rw(FUNC(h8532_device::pwm1_tcnt_r), FUNC(h8532_device::pwm1_tcnt_w));
	map(0xffc4, 0xffc4).rw(FUNC(h8532_device::pwm2_tcr_r), FUNC(h8532_device::pwm2_tcr_w));
	map(0xffc5, 0xffc5).rw(FUNC(h8532_device::pwm2_dtr_r), FUNC(h8532_device::pwm2_dtr_w));
	map(0xffc6, 0xffc6).rw(FUNC(h8532_device::pwm2_tcnt_r), FUNC(h8532_device::pwm2_tcnt_w));
	map(0xffc8, 0xffc8).rw(FUNC(h8532_device::pwm3_tcr_r), FUNC(h8532_device::pwm3_tcr_w));
	map(0xffc9, 0xffc9).rw(FUNC(h8532_device::pwm3_dtr_r), FUNC(h8532_device::pwm3_dtr_w));
	map(0xffca, 0xffca).rw(FUNC(h8532_device::pwm3_tcnt_r), FUNC(h8532_device::pwm3_tcnt_w));
	map(0xffd0, 0xffd0).rw(FUNC(h8532_device::tmr_tcr_r), FUNC(h8532_device::tmr_tcr_w));
	map(0xffd1, 0xffd1).rw(FUNC(h8532_device::tmr_tcsr_r), FUNC(h8532_device::tmr_tcsr_w));
	map(0xffd2, 0xffd2).rw(FUNC(h8532_device::tmr_tcora_r), FUNC(h8532_device::tmr_tcora_w));
	map(0xffd3, 0xffd3).rw(FUNC(h8532_device::tmr_tcorb_r), FUNC(h8532_device::tmr_tcorb_w));
	map(0xffd4, 0xffd4).rw(FUNC(h8532_device::tmr_tcnt_r), FUNC(h8532_device::tmr_tcnt_w));
	map(0xffd8, 0xffd8).rw(FUNC(h8532_device::sci1_smr_r), FUNC(h8532_device::sci1_smr_w));
	map(0xffd9, 0xffd9).rw(FUNC(h8532_device::sci1_brr_r), FUNC(h8532_device::sci1_brr_w));
	map(0xffda, 0xffda).rw(FUNC(h8532_device::sci1_scr_r), FUNC(h8532_device::sci1_scr_w));
	map(0xffdb, 0xffdb).rw(FUNC(h8532_device::sci1_tdr_r), FUNC(h8532_device::sci1_tdr_w));
	map(0xffdc, 0xffdc).rw(FUNC(h8532_device::sci1_ssr_r), FUNC(h8532_device::sci1_ssr_w));
	map(0xffdd, 0xffdd).r(FUNC(h8532_device::sci1_rdr_r));
	map(0xffe0, 0xffe0).r(FUNC(h8532_device::addrah_r));
	map(0xffe1, 0xffe1).r(FUNC(h8532_device::addral_r));
	map(0xffe2, 0xffe2).r(FUNC(h8532_device::addrbh_r));
	map(0xffe3, 0xffe3).r(FUNC(h8532_device::addrbl_r));
	map(0xffe4, 0xffe4).r(FUNC(h8532_device::addrch_r));
	map(0xffe5, 0xffe5).r(FUNC(h8532_device::addrcl_r));
	map(0xffe6, 0xffe6).r(FUNC(h8532_device::addrdh_r));
	map(0xffe7, 0xffe7).r(FUNC(h8532_device::addrdl_r));
	map(0xffe8, 0xffe8).rw(FUNC(h8532_device::adcsr_r), FUNC(h8532_device::adcsr_w));
	map(0xffe9, 0xffe9).rw(FUNC(h8532_device::adcr_r), FUNC(h8532_device::adcr_w));
	map(0xffec, 0xffec).rw(FUNC(h8532_device::wdt_tcsr_r));
	map(0xffed, 0xffed).r(FUNC(h8532_device::wdt_tcnt_r), FUNC(h8532_device::wdt_tcsr_w));
	map(0xfff0, 0xfff0).rw(FUNC(h8532_device::ipra_r), FUNC(h8532_device::ipra_w));
	map(0xfff1, 0xfff1).rw(FUNC(h8532_device::iprb_r), FUNC(h8532_device::iprb_w));
	map(0xfff2, 0xfff2).rw(FUNC(h8532_device::iprc_r), FUNC(h8532_device::iprc_w));
	map(0xfff3, 0xfff3).rw(FUNC(h8532_device::iprd_r), FUNC(h8532_device::iprd_w));
	map(0xfff4, 0xfff4).rw(FUNC(h8532_device::dtea_r), FUNC(h8532_device::dtea_w));
	map(0xfff5, 0xfff5).rw(FUNC(h8532_device::dteb_r), FUNC(h8532_device::dteb_w));
	map(0xfff6, 0xfff6).rw(FUNC(h8532_device::dtec_r), FUNC(h8532_device::dtec_w));
	map(0xfff7, 0xfff7).rw(FUNC(h8532_device::dted_r), FUNC(h8532_device::dted_w));
	map(0xfff8, 0xfff8).rw(FUNC(h8532_device::wcr_r), FUNC(h8532_device::wcr_w));
	map(0xfff9, 0xfff9).rw(FUNC(h8532_device::ramcr_r), FUNC(h8532_device::ramcr_w));
	map(0xfffa, 0xfffa).r(FUNC(h8532_device::mdcr_r));
	map(0xfffb, 0xfffb).rw(FUNC(h8532_device::sbycr_r), FUNC(h8532_device::sbycr_w));
	map(0xfffc, 0xfffc).rw(FUNC(h8532_device::p1cr_r), FUNC(h8532_device::p1cr_w));
	map(0xfffe, 0xfffe).w(FUNC(h8532_device::p9ddr_w));
	map(0xffff, 0xffff).rw(FUNC(h8532_device::p9dr_r), FUNC(h8532_device::p9dr_w));
#endif
}
