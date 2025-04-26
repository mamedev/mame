// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh4.cpp
 *   Portable Hitachi SH-4 (SH7750 family) emulator
 *
 *   By R. Belmont, based on sh2.c by Juergen Buchmueller, Mariusz Wojcieszek,
 *      Olivier Galibert, Sylvain Glaize, and James Forshaw.
 *
 *
 *   TODO: FPU
 *         DMA
 *         on-board peripherals
 *
 *   DONE: boot/reset setup
 *         64-bit data bus
 *         banked registers
 *         additional registers for supervisor mode
 *         FPU status and data registers
 *         state save for the new registers
 *         interrupts
 *         store queues
 *
 *****************************************************************************/

#include "emu.h"
#include "sh4.h"
#include "sh4regs.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"
#include "sh_dasm.h"
#include "cpu/drcumlsh.h"


DEFINE_DEVICE_TYPE(SH3, sh3_device,   "sh3", "Hitachi SH-3 (Unidentified)")
DEFINE_DEVICE_TYPE(SH7708S, sh7708s_device, "sh7708s", "Hitachi SH7708S")
DEFINE_DEVICE_TYPE(SH7709, sh7709_device, "sh7709", "Hitachi SH7709")
DEFINE_DEVICE_TYPE(SH7709S, sh7709s_device, "sh7709s", "Hitachi SH7709S")
DEFINE_DEVICE_TYPE(SH4, sh4_device,   "sh4", "Hitachi SH-4 (Unidentified)")
DEFINE_DEVICE_TYPE(SH7091, sh7091_device, "sh7091", "Hitachi SH7091")
DEFINE_DEVICE_TYPE(SH7750, sh7750_device, "sh7750", "Hitachi SH7750")
DEFINE_DEVICE_TYPE(SH7750R, sh7750r_device, "sh7750r", "Hitachi SH7750R")
DEFINE_DEVICE_TYPE(SH7750S, sh7750s_device, "sh7750s", "Hitachi SH7750S")
DEFINE_DEVICE_TYPE(SH7751, sh7751_device, "sh7751", "Hitachi SH7751")
DEFINE_DEVICE_TYPE(SH7751R, sh7751r_device, "sh7751r", "Hitachi SH7751R")


#if 0
/*When OC index mode is off (CCR.OIX = 0)*/
void sh4_base_device::sh4_internal_map(address_map &map)
{
	map(0x1C000000, 0x1C000FFF).ram().mirror(0x03FFD000);
	map(0x1C002000, 0x1C002FFF).ram().mirror(0x03FFD000);
	map(0xE0000000, 0xE000003F).ram().mirror(0x03FFFFC0);

	sh4_register_map(map);
}
#endif

/*When OC index mode is on (CCR.OIX = 1)*/
void sh4_base_device::sh4_internal_map(address_map &map)
{
	map(0x1C000000, 0x1C000FFF).ram().mirror(0x01FFF000);
	map(0x1E000000, 0x1E000FFF).ram().mirror(0x01FFF000);
	map(0xE0000000, 0xE000003F).ram().mirror(0x03FFFFC0); // todo: store queues should be write only on DC's SH4, executing PREFM shouldn't cause an actual memory read access!

	map(0xF6000000, 0xF6FFFFFF).rw(FUNC(sh4_base_device::sh4_utlb_address_array_r), FUNC(sh4_base_device::sh4_utlb_address_array_w));
	map(0xF7000000, 0xF77FFFFF).rw(FUNC(sh4_base_device::sh4_utlb_data_array1_r), FUNC(sh4_base_device::sh4_utlb_data_array1_w));
	map(0xF7800000, 0xF7FFFFFF).rw(FUNC(sh4_base_device::sh4_utlb_data_array2_r), FUNC(sh4_base_device::sh4_utlb_data_array2_w));

	sh4_register_map(map);
}

void sh4_device::sh4_register_map(address_map &map)
{
	map(0xff000030, 0xff000033).lr32([] { return PVR_SH7751; }, "pvr_r");

	ccn_map(map);
	ubc_map(map);
	bsc_map(map);
	dmac_map(map);
	cpg_7750r_map(map);
	rtc_map(map);
	intc_7750r_map(map);
	tmu_7750r_map(map);
	sci_map(map);
	scif_map(map);
	hudi_7750r_map(map);
	pci_7751_map(map);
}

void sh7091_device::sh4_register_map(address_map& map)
{
	map(0xff000030, 0xff000033).lr32([] { return PVR_SH7091; }, "pvr_r");

	ccn_map(map);
	ubc_map(map);
	bsc_map(map);
	dmac_map(map);
	cpg_map(map);
	rtc_map(map);
	intc_map(map);
	tmu_map(map);
	sci_map(map);
	scif_map(map);
	hudi_map(map);
}

void sh7750_device::sh4_register_map(address_map& map)
{
	map(0xff000030, 0xff000033).lr32([] { return PVR_SH7750; }, "pvr_r");

	ccn_map(map);
	ubc_map(map);
	bsc_map(map);
	dmac_map(map);
	cpg_map(map);
	rtc_map(map);
	intc_map(map);
	tmu_map(map);
	sci_map(map);
	scif_map(map);
	hudi_map(map);
}

void sh7750s_device::sh4_register_map(address_map& map)
{
	map(0xff000030, 0xff000033).lr32([] { return PVR_SH7750S; }, "pvr_r");

	ccn_map(map);
	ubc_map(map);
	bsc_map(map);
	dmac_map(map);
	cpg_map(map);
	rtc_map(map);
	intc_7750s_map(map);
	tmu_map(map);
	sci_map(map);
	scif_map(map);
	hudi_map(map);
}

void sh7750r_device::sh4_register_map(address_map& map)
{
	map(0xff000030, 0xff000033).lr32([] { return PVR_SH7750R; }, "pvr_r");
	map(0xff000044, 0xff000047).lr32([] { return PRR_SH7750R; }, "prr_r");

	ccn_map(map);
	ubc_map(map);
	bsc_7750r_map(map);
	dmac_7750r_map(map);
	cpg_7750r_map(map);
	rtc_7750r_map(map);
	intc_7750r_map(map);
	tmu_7750r_map(map);
	sci_map(map);
	scif_map(map);
	hudi_7750r_map(map);
}

void sh7751_device::sh4_register_map(address_map& map)
{
	map(0xff000030, 0xff000033).lr32([] { return PVR_SH7751; }, "pvr_r");

	ccn_map(map);
	ubc_map(map);
	bsc_map(map);
	dmac_map(map);
	cpg_7750r_map(map);
	rtc_map(map);
	intc_7750r_map(map);
	tmu_7750r_map(map);
	sci_map(map);
	scif_map(map);
	hudi_7750r_map(map);
	pci_7751_map(map);
}

void sh7751r_device::sh4_register_map(address_map& map)
{
	map(0xff000030, 0xff000033).lr32([] { return PVR_SH7751R; }, "pvr_r");
	map(0xff000044, 0xff000047).lr32([] { return PRR_SH7751R; }, "prr_r");

	ccn_map(map);
	ubc_map(map);
	bsc_7750r_map(map);
	dmac_7750r_map(map);
	cpg_7750r_map(map);
	rtc_7750r_map(map);
	intc_7750r_map(map);
	tmu_7750r_map(map);
	sci_map(map);
	scif_map(map);
	hudi_7750r_map(map);
	pci_7751_map(map);
}

void sh4_base_device::ccn_map(address_map &map)
{
	map(0xff000000, 0xff000003).rw(FUNC(sh4_base_device::pteh_r), FUNC(sh4_base_device::pteh_w));
	map(0xff000004, 0xff000007).rw(FUNC(sh4_base_device::ptel_r), FUNC(sh4_base_device::ptel_w));
	map(0xff000008, 0xff00000b).rw(FUNC(sh4_base_device::ttb_r), FUNC(sh4_base_device::ttb_w));
	map(0xff00000c, 0xff00000f).rw(FUNC(sh4_base_device::tea_r), FUNC(sh4_base_device::tea_w));
	map(0xff000010, 0xff000013).rw(FUNC(sh4_base_device::mmucr_r), FUNC(sh4_base_device::mmucr_w));
	map(0xff000014, 0xff000014).rw(FUNC(sh4_base_device::basra_r), FUNC(sh4_base_device::basra_w));
	map(0xff000018, 0xff000018).rw(FUNC(sh4_base_device::basrb_r), FUNC(sh4_base_device::basrb_w));
	map(0xff00001c, 0xff00001f).rw(FUNC(sh4_base_device::ccr_r), FUNC(sh4_base_device::ccr_w));
	map(0xff000020, 0xff000023).rw(FUNC(sh4_base_device::tra_r), FUNC(sh4_base_device::tra_w));
	map(0xff000024, 0xff000027).rw(FUNC(sh4_base_device::expevt_r), FUNC(sh4_base_device::expevt_w));
	map(0xff000028, 0xff00002b).rw(FUNC(sh4_base_device::intevt_r), FUNC(sh4_base_device::intevt_w));
	map(0xff000034, 0xff000037).rw(FUNC(sh4_base_device::ptea_r), FUNC(sh4_base_device::ptea_w));
	map(0xff000038, 0xff00003b).rw(FUNC(sh4_base_device::qacr0_r), FUNC(sh4_base_device::qacr0_w));
	map(0xff00003c, 0xff00003f).rw(FUNC(sh4_base_device::qacr1_r), FUNC(sh4_base_device::qacr1_w));
}

void sh4_base_device::ubc_map(address_map &map)
{
	map(0xff200000, 0xff200003).rw(FUNC(sh4_base_device::bara_r), FUNC(sh4_base_device::bara_w));
	map(0xff200004, 0xff200004).rw(FUNC(sh4_base_device::bamra_r), FUNC(sh4_base_device::bamra_w));
	map(0xff200008, 0xff200009).rw(FUNC(sh4_base_device::bbra_r), FUNC(sh4_base_device::bbra_w));
	map(0xff20000c, 0xff20000f).rw(FUNC(sh4_base_device::barb_r), FUNC(sh4_base_device::barb_w));
	map(0xff200010, 0xff200010).rw(FUNC(sh4_base_device::bamrb_r), FUNC(sh4_base_device::bamrb_w));
	map(0xff200014, 0xff200015).rw(FUNC(sh4_base_device::bbrb_r), FUNC(sh4_base_device::bbrb_w));
	map(0xff200018, 0xff20001b).rw(FUNC(sh4_base_device::bdrb_r), FUNC(sh4_base_device::bdrb_w));
	map(0xff20001c, 0xff20001f).rw(FUNC(sh4_base_device::bdmrb_r), FUNC(sh4_base_device::bdmrb_w));
	map(0xff200020, 0xff200021).rw(FUNC(sh4_base_device::brcr_r), FUNC(sh4_base_device::brcr_w));
}

void sh4_base_device::bsc_map(address_map& map)
{
	map(0xff800000, 0xff800003).rw(FUNC(sh4_base_device::bcr1_r), FUNC(sh4_base_device::bcr1_w));
	map(0xff800004, 0xff800005).rw(FUNC(sh4_base_device::bcr2_r), FUNC(sh4_base_device::bcr2_w));
	map(0xff800008, 0xff80000b).rw(FUNC(sh4_base_device::wcr1_r), FUNC(sh4_base_device::wcr1_w));
	map(0xff80000c, 0xff80000f).rw(FUNC(sh4_base_device::wcr2_r), FUNC(sh4_base_device::wcr2_w));
	map(0xff800010, 0xff800013).rw(FUNC(sh4_base_device::wcr3_r), FUNC(sh4_base_device::wcr3_w));
	map(0xff800014, 0xff800017).rw(FUNC(sh4_base_device::mcr_r), FUNC(sh4_base_device::mcr_w));
	map(0xff800018, 0xff800019).rw(FUNC(sh4_base_device::pcr_r), FUNC(sh4_base_device::pcr_w));
	map(0xff80001c, 0xff80001d).rw(FUNC(sh4_base_device::rtcsr_r), FUNC(sh4_base_device::rtcsr_w));
	map(0xff800020, 0xff800021).rw(FUNC(sh4_base_device::rtcnt_r), FUNC(sh4_base_device::rtcnt_w));
	map(0xff800024, 0xff800025).rw(FUNC(sh4_base_device::rtcor_r), FUNC(sh4_base_device::rtcor_w));
	map(0xff800028, 0xff800029).rw(FUNC(sh4_base_device::rfcr_r), FUNC(sh4_base_device::rfcr_w));
	map(0xff80002c, 0xff80002f).rw(FUNC(sh4_base_device::pctra_r), FUNC(sh4_base_device::pctra_w));
	map(0xff800030, 0xff800031).rw(FUNC(sh4_base_device::pdtra_r), FUNC(sh4_base_device::pdtra_w));
	map(0xff800040, 0xff800043).rw(FUNC(sh4_base_device::pctrb_r), FUNC(sh4_base_device::pctrb_w));
	map(0xff800044, 0xff800045).rw(FUNC(sh4_base_device::pdtrb_r), FUNC(sh4_base_device::pdtrb_w));
	map(0xff800048, 0xff800049).rw(FUNC(sh4_base_device::gpioic_r), FUNC(sh4_base_device::gpioic_w));
	map(0xff900000, 0xff90ffff).w(FUNC(sh4_base_device::sdmr2_w));
	map(0xff940000, 0xff94ffff).w(FUNC(sh4_base_device::sdmr3_w));
}

void sh4_base_device::bsc_7750r_map(address_map& map)
{
	bsc_map(map);

	map(0xff800050, 0xff800051).rw(FUNC(sh4_base_device::bcr3_r), FUNC(sh4_base_device::bcr3_w));
	map(0xfe0a00f0, 0xfe0a00f3).rw(FUNC(sh4_base_device::bcr4_r), FUNC(sh4_base_device::bcr4_w));
}

void sh4_base_device::dmac_map(address_map& map)
{
	map(0xffa00000, 0xffa00003).rw(FUNC(sh4_base_device::sar0_r), FUNC(sh4_base_device::sar0_w));
	map(0xffa00004, 0xffa00007).rw(FUNC(sh4_base_device::dar0_r), FUNC(sh4_base_device::dar0_w));
	map(0xffa00008, 0xffa0000b).rw(FUNC(sh4_base_device::dmatcr0_r), FUNC(sh4_base_device::dmatcr0_w));
	map(0xffa0000c, 0xffa0000f).rw(FUNC(sh4_base_device::chcr0_r), FUNC(sh4_base_device::chcr0_w));
	map(0xffa00010, 0xffa00013).rw(FUNC(sh4_base_device::sar1_r), FUNC(sh4_base_device::sar1_w));
	map(0xffa00014, 0xffa00017).rw(FUNC(sh4_base_device::dar1_r), FUNC(sh4_base_device::dar1_w));
	map(0xffa00018, 0xffa0001b).rw(FUNC(sh4_base_device::dmatcr1_r), FUNC(sh4_base_device::dmatcr1_w));
	map(0xffa0001c, 0xffa0001f).rw(FUNC(sh4_base_device::chcr1_r), FUNC(sh4_base_device::chcr1_w));
	map(0xffa00020, 0xffa00023).rw(FUNC(sh4_base_device::sar2_r), FUNC(sh4_base_device::sar2_w));
	map(0xffa00024, 0xffa00027).rw(FUNC(sh4_base_device::dar2_r), FUNC(sh4_base_device::dar2_w));
	map(0xffa00028, 0xffa0002b).rw(FUNC(sh4_base_device::dmatcr2_r), FUNC(sh4_base_device::dmatcr2_w));
	map(0xffa0002c, 0xffa0002f).rw(FUNC(sh4_base_device::chcr2_r), FUNC(sh4_base_device::chcr2_w));
	map(0xffa00030, 0xffa00033).rw(FUNC(sh4_base_device::sar3_r), FUNC(sh4_base_device::sar3_w));
	map(0xffa00034, 0xffa00037).rw(FUNC(sh4_base_device::dar3_r), FUNC(sh4_base_device::dar3_w));
	map(0xffa00038, 0xffa0003b).rw(FUNC(sh4_base_device::dmatcr3_r), FUNC(sh4_base_device::dmatcr3_w));
	map(0xffa0003c, 0xffa0003f).rw(FUNC(sh4_base_device::chcr3_r), FUNC(sh4_base_device::chcr3_w));
	map(0xffa00040, 0xffa00043).rw(FUNC(sh4_base_device::dmaor_r), FUNC(sh4_base_device::dmaor_w));
}

void sh4_base_device::dmac_7750r_map(address_map& map)
{
	dmac_map(map);

	map(0xffa00050, 0xffa00053).rw(FUNC(sh4_base_device::sar4_r), FUNC(sh4_base_device::sar4_w));
	map(0xffa00054, 0xffa00057).rw(FUNC(sh4_base_device::dar4_r), FUNC(sh4_base_device::dar4_w));
	map(0xffa00058, 0xffa0005b).rw(FUNC(sh4_base_device::dmatcr4_r), FUNC(sh4_base_device::dmatcr4_w));
	map(0xffa0005c, 0xffa0005f).rw(FUNC(sh4_base_device::chcr4_r), FUNC(sh4_base_device::chcr4_w));
	map(0xffa00060, 0xffa00063).rw(FUNC(sh4_base_device::sar5_r), FUNC(sh4_base_device::sar5_w));
	map(0xffa00064, 0xffa00067).rw(FUNC(sh4_base_device::dar5_r), FUNC(sh4_base_device::dar5_w));
	map(0xffa00068, 0xffa0006b).rw(FUNC(sh4_base_device::dmatcr5_r), FUNC(sh4_base_device::dmatcr5_w));
	map(0xffa0006c, 0xffa0006f).rw(FUNC(sh4_base_device::chcr5_r), FUNC(sh4_base_device::chcr5_w));
	map(0xffa00070, 0xffa00073).rw(FUNC(sh4_base_device::sar6_r), FUNC(sh4_base_device::sar6_w));
	map(0xffa00074, 0xffa00077).rw(FUNC(sh4_base_device::dar6_r), FUNC(sh4_base_device::dar6_w));
	map(0xffa00078, 0xffa0007b).rw(FUNC(sh4_base_device::dmatcr6_r), FUNC(sh4_base_device::dmatcr6_w));
	map(0xffa0007c, 0xffa0007f).rw(FUNC(sh4_base_device::chcr6_r), FUNC(sh4_base_device::chcr6_w));
	map(0xffa00080, 0xffa00083).rw(FUNC(sh4_base_device::sar7_r), FUNC(sh4_base_device::sar7_w));
	map(0xffa00084, 0xffa00087).rw(FUNC(sh4_base_device::dar7_r), FUNC(sh4_base_device::dar7_w));
	map(0xffa00088, 0xffa0008b).rw(FUNC(sh4_base_device::dmatcr7_r), FUNC(sh4_base_device::dmatcr7_w));
	map(0xffa0008c, 0xffa0008f).rw(FUNC(sh4_base_device::chcr7_r), FUNC(sh4_base_device::chcr7_w));
}

void sh4_base_device::cpg_map(address_map& map)
{
	map(0xffc00000, 0xffc00001).rw(FUNC(sh4_base_device::frqcr_r), FUNC(sh4_base_device::frqcr_w));
	map(0xffc00004, 0xffc00004).rw(FUNC(sh4_base_device::stbcr_r), FUNC(sh4_base_device::stbcr_w));
	map(0xffc00008, 0xffc00008).r(FUNC(sh4_base_device::wtcnt_r));
	map(0xffc00008, 0xffc00009).w(FUNC(sh4_base_device::wtcnt_w));
	map(0xffc0000c, 0xffc0000c).r(FUNC(sh4_base_device::wtcsr_r));
	map(0xffc0000c, 0xffc0000d).w(FUNC(sh4_base_device::wtcsr_w));
	map(0xffc00010, 0xffc00010).rw(FUNC(sh4_base_device::stbcr2_r), FUNC(sh4_base_device::stbcr2_w));
}

void sh4_base_device::cpg_7750r_map(address_map& map)
{
	cpg_map(map);

	map(0xfe0a0000, 0xfe0a0003).rw(FUNC(sh4_base_device::clkstp00_r), FUNC(sh4_base_device::clkstp00_w));
	map(0xfe0a0008, 0xfe0a000b).w(FUNC(sh4_base_device::clkstpclr_w));
}

void sh4_base_device::rtc_map(address_map& map)
{
	map(0xffc80000, 0xffc80000).r(FUNC(sh4_base_device::r64cnt_r));
	map(0xffc80004, 0xffc80004).rw(FUNC(sh4_base_device::rseccnt_r), FUNC(sh4_base_device::rseccnt_w));
	map(0xffc80008, 0xffc80008).rw(FUNC(sh4_base_device::rmincnt_r), FUNC(sh4_base_device::rmincnt_w));
	map(0xffc8000c, 0xffc8000c).rw(FUNC(sh4_base_device::rhrcnt_r), FUNC(sh4_base_device::rhrcnt_w));
	map(0xffc80010, 0xffc80010).rw(FUNC(sh4_base_device::rwkcnt_r), FUNC(sh4_base_device::rwkcnt_w));
	map(0xffc80014, 0xffc80014).rw(FUNC(sh4_base_device::rdaycnt_r), FUNC(sh4_base_device::rdaycnt_w));
	map(0xffc80018, 0xffc80018).rw(FUNC(sh4_base_device::rmoncnt_r), FUNC(sh4_base_device::rmoncnt_w));
	map(0xffc8001c, 0xffc8001d).rw(FUNC(sh4_base_device::ryrcnt_r), FUNC(sh4_base_device::ryrcnt_w));
	map(0xffc80020, 0xffc80020).rw(FUNC(sh4_base_device::rsecar_r), FUNC(sh4_base_device::rsecar_w));
	map(0xffc80024, 0xffc80024).rw(FUNC(sh4_base_device::rminar_r), FUNC(sh4_base_device::rminar_w));
	map(0xffc80028, 0xffc80028).rw(FUNC(sh4_base_device::rhrar_r), FUNC(sh4_base_device::rhrar_w));
	map(0xffc8002c, 0xffc8002c).rw(FUNC(sh4_base_device::rwkar_r), FUNC(sh4_base_device::rwkar_w));
	map(0xffc80030, 0xffc80030).rw(FUNC(sh4_base_device::rdayar_r), FUNC(sh4_base_device::rdayar_w));
	map(0xffc80034, 0xffc80034).rw(FUNC(sh4_base_device::rmonar_r), FUNC(sh4_base_device::rmonar_w));
	map(0xffc80038, 0xffc80038).rw(FUNC(sh4_base_device::rcr1_r), FUNC(sh4_base_device::rcr1_w));
	map(0xffc8003c, 0xffc8003c).rw(FUNC(sh4_base_device::rcr2_r), FUNC(sh4_base_device::rcr2_w));
}

void sh4_base_device::rtc_7750r_map(address_map& map)
{
	rtc_map(map);

	map(0xffc80050, 0xffc80050).rw(FUNC(sh4_base_device::rcr3_r), FUNC(sh4_base_device::rcr3_w));
	map(0xffc80054, 0xffc80055).rw(FUNC(sh4_base_device::ryrar_r), FUNC(sh4_base_device::ryrar_w));
}

void sh4_base_device::intc_map(address_map& map)
{
	map(0xffd00000, 0xffd00001).rw(FUNC(sh4_base_device::icr_r), FUNC(sh4_base_device::icr_w));
	map(0xffd00004, 0xffd00005).rw(FUNC(sh4_base_device::ipra_r), FUNC(sh4_base_device::ipra_w));
	map(0xffd00008, 0xffd00009).rw(FUNC(sh4_base_device::iprb_r), FUNC(sh4_base_device::iprb_w));
	map(0xffd0000c, 0xffd0000d).rw(FUNC(sh4_base_device::iprc_r), FUNC(sh4_base_device::iprc_w));
}

void sh4_base_device::intc_7750s_map(address_map& map)
{
	intc_map(map);

	map(0xffd00010, 0xffd00011).rw(FUNC(sh4_base_device::iprd_r), FUNC(sh4_base_device::iprd_w));
}

void sh4_base_device::intc_7750r_map(address_map& map)
{
	intc_7750s_map(map);

	map(0xfe080000, 0xfe080003).rw(FUNC(sh4_base_device::intpri00_r), FUNC(sh4_base_device::intpri00_w));
	map(0xfe080020, 0xfe080023).rw(FUNC(sh4_base_device::intreq00_r), FUNC(sh4_base_device::intreq00_w));
	map(0xfe080040, 0xfe080043).rw(FUNC(sh4_base_device::intmsk00_r), FUNC(sh4_base_device::intmskclr_w));
	map(0xfe080060, 0xfe080063).w(FUNC(sh4_base_device::intmsk00_w));
}

void sh4_base_device::tmu_map(address_map& map)
{
	map(0xffd80000, 0xffd80000).rw(FUNC(sh4_base_device::tocr_r), FUNC(sh4_base_device::tocr_w));
	map(0xffd80004, 0xffd80004).rw(FUNC(sh4_base_device::tstr_r), FUNC(sh4_base_device::tstr_w));
	map(0xffd80008, 0xffd8000b).rw(FUNC(sh4_base_device::tcor0_r), FUNC(sh4_base_device::tcor0_w));
	map(0xffd8000c, 0xffd8000f).rw(FUNC(sh4_base_device::tcnt0_r), FUNC(sh4_base_device::tcnt0_w));
	map(0xffd80010, 0xffd80011).rw(FUNC(sh4_base_device::tcr0_r), FUNC(sh4_base_device::tcr0_w));
	map(0xffd80014, 0xffd80017).rw(FUNC(sh4_base_device::tcor1_r), FUNC(sh4_base_device::tcor1_w));
	map(0xffd80018, 0xffd8001b).rw(FUNC(sh4_base_device::tcnt1_r), FUNC(sh4_base_device::tcnt1_w));
	map(0xffd8001c, 0xffd8001d).rw(FUNC(sh4_base_device::tcr1_r), FUNC(sh4_base_device::tcr1_w));
	map(0xffd80020, 0xffd80023).rw(FUNC(sh4_base_device::tcor2_r), FUNC(sh4_base_device::tcor2_w));
	map(0xffd80024, 0xffd80027).rw(FUNC(sh4_base_device::tcnt2_r), FUNC(sh4_base_device::tcnt2_w));
	map(0xffd80028, 0xffd80029).rw(FUNC(sh4_base_device::tcr2_r), FUNC(sh4_base_device::tcr2_w));
	map(0xffd8002c, 0xffd8002f).rw(FUNC(sh4_base_device::tcpr2_r), FUNC(sh4_base_device::tcpr2_w));
}

void sh4_base_device::tmu_7750r_map(address_map& map)
{
	tmu_map(map);

	map(0xfe100004, 0xfe100004).rw(FUNC(sh4_base_device::tstr2_r), FUNC(sh4_base_device::tstr2_w));
	map(0xfe100008, 0xfe10000b).rw(FUNC(sh4_base_device::tcor3_r), FUNC(sh4_base_device::tcor3_w));
	map(0xfe10000c, 0xfe10000f).rw(FUNC(sh4_base_device::tcnt3_r), FUNC(sh4_base_device::tcnt3_w));
	map(0xfe100010, 0xfe100011).rw(FUNC(sh4_base_device::tcr3_r), FUNC(sh4_base_device::tcr3_w));
	map(0xfe100014, 0xfe100017).rw(FUNC(sh4_base_device::tcor4_r), FUNC(sh4_base_device::tcor4_w));
	map(0xfe100018, 0xfe10001b).rw(FUNC(sh4_base_device::tcnt4_r), FUNC(sh4_base_device::tcnt4_w));
	map(0xfe10001c, 0xfe10001d).rw(FUNC(sh4_base_device::tcr4_r), FUNC(sh4_base_device::tcr4_w));
}

void sh4_base_device::sci_map(address_map& map)
{
	map(0xffe00000, 0xffe00000).rw(FUNC(sh4_base_device::scsmr1_r), FUNC(sh4_base_device::scsmr1_w));
	map(0xffe00004, 0xffe00004).rw(FUNC(sh4_base_device::scbrr1_r), FUNC(sh4_base_device::scbrr1_w));
	map(0xffe00008, 0xffe00008).rw(FUNC(sh4_base_device::scscr1_r), FUNC(sh4_base_device::scscr1_w));
	map(0xffe0000c, 0xffe0000c).rw(FUNC(sh4_base_device::sctdr1_r), FUNC(sh4_base_device::sctdr1_w));
	map(0xffe00010, 0xffe00010).rw(FUNC(sh4_base_device::scssr1_r), FUNC(sh4_base_device::scssr1_w));
	map(0xffe00014, 0xffe00014).r(FUNC(sh4_base_device::scrdr1_r));
	map(0xffe00018, 0xffe00018).rw(FUNC(sh4_base_device::scscmr1_r), FUNC(sh4_base_device::scscmr1_w));
	map(0xffe0001c, 0xffe0001c).rw(FUNC(sh4_base_device::scsptr1_r), FUNC(sh4_base_device::scsptr1_w));
}

void sh4_base_device::scif_map(address_map& map)
{
	map(0xffe80000, 0xffe80001).rw(FUNC(sh4_base_device::scsmr2_r), FUNC(sh4_base_device::scsmr2_w));
	map(0xffe80004, 0xffe80004).rw(FUNC(sh4_base_device::scbrr2_r), FUNC(sh4_base_device::scbrr2_w));
	map(0xffe80008, 0xffe80009).rw(FUNC(sh4_base_device::scscr2_r), FUNC(sh4_base_device::scscr2_w));
	map(0xffe8000c, 0xffe8000c).rw(FUNC(sh4_base_device::scftdr2_r), FUNC(sh4_base_device::scftdr2_w));
	map(0xffe80010, 0xffe80011).rw(FUNC(sh4_base_device::scfsr2_r), FUNC(sh4_base_device::scfsr2_w));
	map(0xffe80014, 0xffe80014).r(FUNC(sh4_base_device::scfrdr2_r));
	map(0xffe80018, 0xffe80019).rw(FUNC(sh4_base_device::scfcr2_r), FUNC(sh4_base_device::scfcr2_w));
	map(0xffe8001c, 0xffe8001d).r(FUNC(sh4_base_device::scfdr2_r));
	map(0xffe80020, 0xffe80021).rw(FUNC(sh4_base_device::scsptr2_r), FUNC(sh4_base_device::scsptr2_w));
	map(0xffe80024, 0xffe80025).rw(FUNC(sh4_base_device::sclsr2_r), FUNC(sh4_base_device::sclsr2_w));
}

void sh4_base_device::hudi_map(address_map &map)
{
	map(0xfff00000, 0xfff00001).rw(FUNC(sh4_base_device::sdir_r), FUNC(sh4_base_device::sdir_w));
	map(0xfff00008, 0xfff0000b).rw(FUNC(sh4_base_device::sddr_r), FUNC(sh4_base_device::sddr_w));
}

void sh4_base_device::hudi_7750r_map(address_map& map)
{
	hudi_map(map);

	map(0xfff00014, 0xfff00015).rw(FUNC(sh4_base_device::sdint_r), FUNC(sh4_base_device::sdint_w));
}

void sh4_base_device::pci_7751_map(address_map& map)
{
	// PCI
	//  0xfd000000, 0xfdffffff PCIMEM

	// PCIC
	map(0xfe200000, 0xfe200003).r(FUNC(sh4_base_device::pciconf0_r));
	map(0xfe200004, 0xfe200007).rw(FUNC(sh4_base_device::pciconf1_r), FUNC(sh4_base_device::pciconf1_w));
	map(0xfe200008, 0xfe20000b).rw(FUNC(sh4_base_device::pciconf2_r), FUNC(sh4_base_device::pciconf2_w));
	map(0xfe20000c, 0xfe20000f).rw(FUNC(sh4_base_device::pciconf3_r), FUNC(sh4_base_device::pciconf3_w));
	map(0xfe200010, 0xfe200013).rw(FUNC(sh4_base_device::pciconf4_r), FUNC(sh4_base_device::pciconf4_w));
	map(0xfe200014, 0xfe200017).rw(FUNC(sh4_base_device::pciconf5_r), FUNC(sh4_base_device::pciconf5_w));
	map(0xfe200018, 0xfe20001b).rw(FUNC(sh4_base_device::pciconf6_r), FUNC(sh4_base_device::pciconf6_w));
	map(0xfe20001c, 0xfe20001f).r(FUNC(sh4_base_device::pciconf7_r));
	map(0xfe200020, 0xfe200023).r(FUNC(sh4_base_device::pciconf8_r));
	map(0xfe200024, 0xfe200027).r(FUNC(sh4_base_device::pciconf9_r));
	map(0xfe200028, 0xfe20002b).r(FUNC(sh4_base_device::pciconf10_r));
	map(0xfe20002c, 0xfe20002f).rw(FUNC(sh4_base_device::pciconf11_r), FUNC(sh4_base_device::pciconf11_w));
	map(0xfe200030, 0xfe200033).r(FUNC(sh4_base_device::pciconf12_r));
	map(0xfe200034, 0xfe200037).r(FUNC(sh4_base_device::pciconf13_r));
	map(0xfe200038, 0xfe20003b).r(FUNC(sh4_base_device::pciconf14_r));
	map(0xfe20003c, 0xfe20003f).rw(FUNC(sh4_base_device::pciconf15_r), FUNC(sh4_base_device::pciconf15_w));
	map(0xfe200040, 0xfe200043).rw(FUNC(sh4_base_device::pciconf16_r), FUNC(sh4_base_device::pciconf16_w));
	map(0xfe200044, 0xfe200047).rw(FUNC(sh4_base_device::pciconf17_r), FUNC(sh4_base_device::pciconf17_w));
	map(0xfe200100, 0xfe200103).rw(FUNC(sh4_base_device::pcicr_r), FUNC(sh4_base_device::pcicr_w));
	map(0xfe200104, 0xfe200107).rw(FUNC(sh4_base_device::pcilsr0_r), FUNC(sh4_base_device::pcilsr0_w));
	map(0xfe200108, 0xfe20010b).rw(FUNC(sh4_base_device::pcilsr1_r), FUNC(sh4_base_device::pcilsr1_w));
	map(0xfe20010c, 0xfe20010f).rw(FUNC(sh4_base_device::pcilar0_r), FUNC(sh4_base_device::pcilar0_w));
	map(0xfe200110, 0xfe200113).rw(FUNC(sh4_base_device::pcilar1_r), FUNC(sh4_base_device::pcilar1_w));
	map(0xfe200114, 0xfe200117).rw(FUNC(sh4_base_device::pciint_r), FUNC(sh4_base_device::pciint_w));
	map(0xfe200118, 0xfe20011b).rw(FUNC(sh4_base_device::pciintm_r), FUNC(sh4_base_device::pciintm_w));
	map(0xfe20011c, 0xfe20011f).rw(FUNC(sh4_base_device::pcialr_r), FUNC(sh4_base_device::pcialr_w));
	map(0xfe200120, 0xfe200123).rw(FUNC(sh4_base_device::pciclr_r), FUNC(sh4_base_device::pciclr_w));
	map(0xfe200130, 0xfe200133).rw(FUNC(sh4_base_device::pciaint_r), FUNC(sh4_base_device::pciaint_w));
	map(0xfe200134, 0xfe200137).rw(FUNC(sh4_base_device::pciaintm_r), FUNC(sh4_base_device::pciaintm_w));
	map(0xfe200138, 0xfe20013b).rw(FUNC(sh4_base_device::pcibllr_r), FUNC(sh4_base_device::pcibllr_w));
	map(0xfe200140, 0xfe200143).rw(FUNC(sh4_base_device::pcidmabt_r), FUNC(sh4_base_device::pcidmabt_w));
	map(0xfe200180, 0xfe200183).rw(FUNC(sh4_base_device::pcidpa0_r), FUNC(sh4_base_device::pcidpa0_w));
	map(0xfe200184, 0xfe200187).rw(FUNC(sh4_base_device::pcidla0_r), FUNC(sh4_base_device::pcidla0_w));
	map(0xfe200188, 0xfe20018b).rw(FUNC(sh4_base_device::pcidtc0_r), FUNC(sh4_base_device::pcidtc0_w));
	map(0xfe20018c, 0xfe20018f).rw(FUNC(sh4_base_device::pcidcr0_r), FUNC(sh4_base_device::pcidcr0_w));
	map(0xfe200190, 0xfe200193).rw(FUNC(sh4_base_device::pcidpa1_r), FUNC(sh4_base_device::pcidpa1_w));
	map(0xfe200194, 0xfe200197).rw(FUNC(sh4_base_device::pcidla1_r), FUNC(sh4_base_device::pcidla1_w));
	map(0xfe200198, 0xfe20019b).rw(FUNC(sh4_base_device::pcidtc1_r), FUNC(sh4_base_device::pcidtc1_w));
	map(0xfe20019c, 0xfe20019f).rw(FUNC(sh4_base_device::pcidcr1_r), FUNC(sh4_base_device::pcidcr1_w));
	map(0xfe2001a0, 0xfe2001a3).rw(FUNC(sh4_base_device::pcidpa2_r), FUNC(sh4_base_device::pcidpa2_w));
	map(0xfe2001a4, 0xfe2001a7).rw(FUNC(sh4_base_device::pcidla2_r), FUNC(sh4_base_device::pcidla2_w));
	map(0xfe2001a8, 0xfe2001ab).rw(FUNC(sh4_base_device::pcidtc2_r), FUNC(sh4_base_device::pcidtc2_w));
	map(0xfe2001ac, 0xfe2001af).rw(FUNC(sh4_base_device::pcidcr2_r), FUNC(sh4_base_device::pcidcr2_w));
	map(0xfe2001b0, 0xfe2001b3).rw(FUNC(sh4_base_device::pcidpa3_r), FUNC(sh4_base_device::pcidpa3_w));
	map(0xfe2001b4, 0xfe2001b7).rw(FUNC(sh4_base_device::pcidla3_r), FUNC(sh4_base_device::pcidla3_w));
	map(0xfe2001b8, 0xfe2001bb).rw(FUNC(sh4_base_device::pcidtc3_r), FUNC(sh4_base_device::pcidtc3_w));
	map(0xfe2001bc, 0xfe2001bf).rw(FUNC(sh4_base_device::pcidcr3_r), FUNC(sh4_base_device::pcidcr3_w));
	map(0xfe2001c0, 0xfe2001c3).rw(FUNC(sh4_base_device::pcipar_r), FUNC(sh4_base_device::pcipar_w));
	map(0xfe2001c4, 0xfe2001c7).rw(FUNC(sh4_base_device::pcimbr_r), FUNC(sh4_base_device::pcimbr_w));
	map(0xfe2001c8, 0xfe2001cb).rw(FUNC(sh4_base_device::pciiobr_r), FUNC(sh4_base_device::pciiobr_w));
	map(0xfe2001cc, 0xfe2001cf).rw(FUNC(sh4_base_device::pcipint_r), FUNC(sh4_base_device::pcipint_w));
	map(0xfe2001d0, 0xfe2001d3).rw(FUNC(sh4_base_device::pcipintm_r), FUNC(sh4_base_device::pcipintm_w));
	map(0xfe2001d4, 0xfe2001d7).rw(FUNC(sh4_base_device::pciclkr_r), FUNC(sh4_base_device::pciclkr_w));
	map(0xfe2001e0, 0xfe2001e3).rw(FUNC(sh4_base_device::pcibcr1_r), FUNC(sh4_base_device::pcibcr1_w));
	map(0xfe2001e4, 0xfe2001e7).rw(FUNC(sh4_base_device::pcibcr2_r), FUNC(sh4_base_device::pcibcr2_w));
	map(0xfe2001f8, 0xfe2001fb).rw(FUNC(sh4_base_device::pcibcr3_r), FUNC(sh4_base_device::pcibcr3_w));
	map(0xfe2001e8, 0xfe2001eb).rw(FUNC(sh4_base_device::pciwcr1_r), FUNC(sh4_base_device::pciwcr1_w));
	map(0xfe2001ec, 0xfe2001ef).rw(FUNC(sh4_base_device::pciwcr2_r), FUNC(sh4_base_device::pciwcr2_w));
	map(0xfe2001f0, 0xfe2001f3).rw(FUNC(sh4_base_device::pciwcr3_r), FUNC(sh4_base_device::pciwcr3_w));
	map(0xfe2001f4, 0xfe2001f7).rw(FUNC(sh4_base_device::pcimcr_r), FUNC(sh4_base_device::pcimcr_w));
	map(0xfe200200, 0xfe200203).rw(FUNC(sh4_base_device::pcipctr_r), FUNC(sh4_base_device::pcipctr_w));
	map(0xfe200204, 0xfe200207).rw(FUNC(sh4_base_device::pcipdtr_r), FUNC(sh4_base_device::pcipdtr_w));
	map(0xfe200220, 0xfe200223).rw(FUNC(sh4_base_device::pcipdr_r), FUNC(sh4_base_device::pcipdr_w));

	// PCIIO
	//  0xfe240000, 0xfe27ffff
}

void sh3_base_device::sh3_internal_map(address_map &map)
{
	sh3_register_map(map);
}

void sh3_device::sh3_register_map(address_map& map)
{
	ccn_7709s_map(map);
	ubc_7709s_map(map);
	cpg_7709_map(map);
	bsc_7709s_map(map);
	rtc_map(map);
	intc_7709_map(map);
	dmac_7709_map(map);
	tmu_map(map);
	sci_7709_map(map);
	cmt_7709_map(map);
	ad_7709_map(map);
	da_7709_map(map);
	port_7709_map(map);
	irda_7709_map(map);
	scif_7709_map(map);
	udi_7709s_map(map);
}

void sh7708s_device::sh3_register_map(address_map& map)
{
	ccn_map(map);
	ubc_map(map);
	cpg_map(map);
	bsc_7708_map(map);
	rtc_map(map);
	intc_map(map);
	tmu_map(map);
	sci_7708_map(map);
}

void sh7709_device::sh3_register_map(address_map& map)
{
	ccn_map(map);
	ubc_map(map);
	cpg_7709_map(map);
	bsc_7709_map(map);
	rtc_map(map);
	intc_7709_map(map);
	dmac_7709_map(map);
	tmu_map(map);
	sci_7709_map(map);
	cmt_7709_map(map);
	ad_7709_map(map);
	da_7709_map(map);
	port_7709_map(map);
	irda_7709_map(map);
	scif_7709_map(map);
}

void sh7709s_device::sh3_register_map(address_map& map)
{
	ccn_7709s_map(map);
	ubc_7709s_map(map);
	cpg_7709_map(map);
	bsc_7709s_map(map);
	rtc_map(map);
	intc_7709_map(map);
	dmac_7709_map(map);
	tmu_map(map);
	sci_7709_map(map);
	cmt_7709_map(map);
	ad_7709_map(map);
	da_7709_map(map);
	port_7709_map(map);
	irda_7709_map(map);
	scif_7709_map(map);
	udi_7709s_map(map);
}

void sh3_base_device::ccn_map(address_map& map)
{
	map(0xfffffff0, 0xfffffff3).rw(FUNC(sh3_base_device::pteh_r), FUNC(sh3_base_device::pteh_w));
	map(0xfffffff4, 0xfffffff7).rw(FUNC(sh3_base_device::ptel_r), FUNC(sh3_base_device::ptel_w));
	map(0xfffffff8, 0xfffffffb).rw(FUNC(sh3_base_device::ttb_r), FUNC(sh3_base_device::ttb_w));
	map(0xfffffffc, 0xffffffff).rw(FUNC(sh3_base_device::tea_r), FUNC(sh3_base_device::tea_w));
	map(0xffffffe0, 0xffffffe3).rw(FUNC(sh3_base_device::mmucr_r), FUNC(sh3_base_device::mmucr_w));
	map(0xffffffe4, 0xffffffe4).rw(FUNC(sh3_base_device::basra_r), FUNC(sh3_base_device::basra_w));
	map(0xffffffe8, 0xffffffe8).rw(FUNC(sh3_base_device::basrb_r), FUNC(sh3_base_device::basrb_w));
	map(0xffffffec, 0xffffffef).rw(FUNC(sh3_base_device::ccr_r), FUNC(sh3_base_device::ccr_w));
	map(0xffffffd0, 0xffffffd3).rw(FUNC(sh3_base_device::tra_r), FUNC(sh3_base_device::tra_w));
	map(0xffffffd4, 0xffffffd7).rw(FUNC(sh3_base_device::expevt_r), FUNC(sh3_base_device::expevt_w));
	map(0xffffffd8, 0xffffffdb).rw(FUNC(sh3_base_device::intevt_r), FUNC(sh3_base_device::intevt_w));
}

void sh3_base_device::ccn_7709s_map(address_map& map)
{
	ccn_map(map);

	map(0x400000b0, 0x400000b3).rw(FUNC(sh3_base_device::ccr2_r), FUNC(sh3_base_device::ccr2_w));
}

void sh3_base_device::ubc_map(address_map& map)
{
	map(0xffffffb0, 0xffffffb3).rw(FUNC(sh3_base_device::bara_r), FUNC(sh3_base_device::bara_w));
	map(0xffffffb4, 0xffffffb4).rw(FUNC(sh3_base_device::bamra_r), FUNC(sh3_base_device::bamra_w));
	map(0xffffffb8, 0xffffffb9).rw(FUNC(sh3_base_device::bbra_r), FUNC(sh3_base_device::bbra_w));
	map(0xffffffa0, 0xffffffa3).rw(FUNC(sh3_base_device::barb_r), FUNC(sh3_base_device::barb_w));
	map(0xffffffa4, 0xffffffa4).rw(FUNC(sh3_base_device::bamrb_r), FUNC(sh3_base_device::bamrb_w));
	map(0xffffffa8, 0xffffffa9).rw(FUNC(sh3_base_device::bbrb_r), FUNC(sh3_base_device::bbrb_w));
	map(0xffffff90, 0xffffff93).rw(FUNC(sh3_base_device::bdrb_r), FUNC(sh3_base_device::bdrb_w));
	map(0xffffff94, 0xffffff97).rw(FUNC(sh3_base_device::bdmrb_r), FUNC(sh3_base_device::bdmrb_w));
	map(0xffffff98, 0xffffff99).rw(FUNC(sh3_base_device::brcr_r), FUNC(sh3_base_device::brcr_w));
}

void sh3_base_device::ubc_7709s_map(address_map& map)
{
	ubc_map(map);

	map(0xffffff9c, 0xffffff9d).rw(FUNC(sh3_base_device::betr_r), FUNC(sh3_base_device::betr_w));
	map(0xffffffac, 0xffffffaf).rw(FUNC(sh3_base_device::brsr_r), FUNC(sh3_base_device::brsr_w));
	map(0xffffffbc, 0xffffffbf).rw(FUNC(sh3_base_device::brdr_r), FUNC(sh3_base_device::brdr_w));
}

void sh3_base_device::cpg_map(address_map& map)
{
	map(0xffffff80, 0xffffff81).rw(FUNC(sh3_base_device::frqcr_r), FUNC(sh3_base_device::frqcr_w));
	map(0xffffff82, 0xffffff83).rw(FUNC(sh3_base_device::stbcr_r), FUNC(sh3_base_device::stbcr_w));
	map(0xffffff84, 0xffffff84).r(FUNC(sh3_base_device::wtcnt_r));
	map(0xffffff84, 0xffffff85).w(FUNC(sh3_base_device::wtcnt_w));
	map(0xffffff86, 0xffffff86).r(FUNC(sh3_base_device::wtcsr_r));
	map(0xffffff86, 0xffffff87).w(FUNC(sh3_base_device::wtcsr_w));
}

void sh3_base_device::cpg_7709_map(address_map& map)
{
	cpg_map(map);

	map(0xffffff88, 0xffffff88).rw(FUNC(sh3_base_device::stbcr2_r), FUNC(sh3_base_device::stbcr2_w));
}

void sh3_base_device::bsc_map(address_map& map)
{
	map(0xffffff60, 0xffffff61).rw(FUNC(sh3_base_device::bcr1_r), FUNC(sh3_base_device::bcr1_w));
	map(0xffffff62, 0xffffff63).rw(FUNC(sh3_base_device::bcr2_r), FUNC(sh3_base_device::bcr2_w));
	map(0xffffff64, 0xffffff65).rw(FUNC(sh3_base_device::wcr1_r), FUNC(sh3_base_device::wcr1_w));
	map(0xffffff66, 0xffffff67).rw(FUNC(sh3_base_device::wcr2_r), FUNC(sh3_base_device::wcr2_w));
	map(0xffffff68, 0xffffff69).rw(FUNC(sh3_base_device::mcr_r), FUNC(sh3_base_device::mcr_w));
	map(0xffffff6c, 0xffffff6d).rw(FUNC(sh3_base_device::pcr_r), FUNC(sh3_base_device::pcr_w));
	map(0xffffff6e, 0xffffff6f).rw(FUNC(sh3_base_device::rtcsr_r), FUNC(sh3_base_device::rtcsr_w));
	map(0xffffff70, 0xffffff71).rw(FUNC(sh3_base_device::rtcnt_r), FUNC(sh3_base_device::rtcnt_w));
	map(0xffffff72, 0xffffff73).rw(FUNC(sh3_base_device::rtcor_r), FUNC(sh3_base_device::rtcor_w));
	map(0xffffff74, 0xffffff75).rw(FUNC(sh3_base_device::rfcr_r), FUNC(sh3_base_device::rfcr_w));
	map(0xfffd0000, 0xffffeffe).rw(FUNC(sh3_base_device::sdmr_r), FUNC(sh3_base_device::sdmr_w));
}

void sh3_base_device::bsc_7708_map(address_map& map)
{
	bsc_map(map);

	map(0xffffff6a, 0xffffff6b).rw(FUNC(sh3_base_device::dcr_r), FUNC(sh3_base_device::dcr_w));
	map(0xffffff76, 0xffffff77).rw(FUNC(sh3_base_device::pctr_r), FUNC(sh3_base_device::pctr_w));
	map(0xffffff78, 0xffffff79).rw(FUNC(sh3_base_device::pdtr_r), FUNC(sh3_base_device::pdtr_w));
}

void sh3_base_device::bsc_7709_map(address_map& map)
{
	bsc_map(map);

	map(0xffffff6a, 0xffffff6b).rw(FUNC(sh3_base_device::dcr_r), FUNC(sh3_base_device::dcr_w));
	map(0xffffff7e, 0xffffff7f).rw(FUNC(sh3_base_device::bcr3_r), FUNC(sh3_base_device::bcr3_w));
}

void sh3_base_device::bsc_7709s_map(address_map& map)
{
	bsc_map(map);

	map(0xffffff50, 0xffffff51).rw(FUNC(sh3_base_device::mcscr0_r), FUNC(sh3_base_device::mcscr0_w));
	map(0xffffff52, 0xffffff53).rw(FUNC(sh3_base_device::mcscr1_r), FUNC(sh3_base_device::mcscr1_w));
	map(0xffffff54, 0xffffff55).rw(FUNC(sh3_base_device::mcscr2_r), FUNC(sh3_base_device::mcscr2_w));
	map(0xffffff56, 0xffffff57).rw(FUNC(sh3_base_device::mcscr3_r), FUNC(sh3_base_device::mcscr3_w));
	map(0xffffff58, 0xffffff59).rw(FUNC(sh3_base_device::mcscr4_r), FUNC(sh3_base_device::mcscr4_w));
	map(0xffffff5a, 0xffffff5b).rw(FUNC(sh3_base_device::mcscr5_r), FUNC(sh3_base_device::mcscr5_w));
	map(0xffffff5c, 0xffffff5d).rw(FUNC(sh3_base_device::mcscr6_r), FUNC(sh3_base_device::mcscr6_w));
	map(0xffffff5e, 0xffffff5f).rw(FUNC(sh3_base_device::mcscr7_r), FUNC(sh3_base_device::mcscr7_w));
}

void sh3_base_device::rtc_map(address_map& map)
{
	map(0xfffffec0, 0xfffffec0).r(FUNC(sh3_base_device::r64cnt_r));
	map(0xfffffec2, 0xfffffec2).rw(FUNC(sh3_base_device::rseccnt_r), FUNC(sh3_base_device::rseccnt_w));
	map(0xfffffec4, 0xfffffec4).rw(FUNC(sh3_base_device::rmincnt_r), FUNC(sh3_base_device::rmincnt_w));
	map(0xfffffec6, 0xfffffec6).rw(FUNC(sh3_base_device::rhrcnt_r), FUNC(sh3_base_device::rhrcnt_w));
	map(0xfffffec8, 0xfffffec8).rw(FUNC(sh3_base_device::rwkcnt_r), FUNC(sh3_base_device::rwkcnt_w));
	map(0xfffffeca, 0xfffffeca).rw(FUNC(sh3_base_device::rdaycnt_r), FUNC(sh3_base_device::rdaycnt_w));
	map(0xfffffecc, 0xfffffecc).rw(FUNC(sh3_base_device::rmoncnt_r), FUNC(sh3_base_device::rmoncnt_w));
	map(0xfffffece, 0xfffffece).rw(FUNC(sh3_base_device::ryrcnt_r), FUNC(sh3_base_device::ryrcnt_w));
	map(0xfffffec0, 0xfffffec0).rw(FUNC(sh3_base_device::rsecar_r), FUNC(sh3_base_device::rsecar_w));
	map(0xfffffed2, 0xfffffed2).rw(FUNC(sh3_base_device::rminar_r), FUNC(sh3_base_device::rminar_w));
	map(0xfffffed4, 0xfffffed4).rw(FUNC(sh3_base_device::rhrar_r), FUNC(sh3_base_device::rhrar_w));
	map(0xfffffed6, 0xfffffed6).rw(FUNC(sh3_base_device::rwkar_r), FUNC(sh3_base_device::rwkar_w));
	map(0xfffffed8, 0xfffffed8).rw(FUNC(sh3_base_device::rdayar_r), FUNC(sh3_base_device::rdayar_w));
	map(0xfffffeda, 0xfffffeda).rw(FUNC(sh3_base_device::rmonar_r), FUNC(sh3_base_device::rmonar_w));
	map(0xfffffedc, 0xfffffedc).rw(FUNC(sh3_base_device::rcr1_r), FUNC(sh3_base_device::rcr1_w));
	map(0xfffffede, 0xfffffede).rw(FUNC(sh3_base_device::rcr2_r), FUNC(sh3_base_device::rcr2_w));
}

void sh3_base_device::intc_map(address_map& map)
{
	map(0xfffffee0, 0xfffffee1).rw(FUNC(sh3_base_device::icr0_r), FUNC(sh3_base_device::icr0_w));
	map(0xfffffee2, 0xfffffee3).rw(FUNC(sh3_base_device::ipra_r), FUNC(sh3_base_device::ipra_w));
	map(0xfffffee4, 0xfffffee5).rw(FUNC(sh3_base_device::iprb_r), FUNC(sh3_base_device::iprb_w));
}

void sh3_base_device::intc_7709_map(address_map& map)
{
	intc_map(map);

	map(0x04000000, 0x04000003).rw(FUNC(sh3_base_device::intevt2_r), FUNC(sh3_base_device::intevt2_w));
	map(0x04000004, 0x04000004).rw(FUNC(sh3_base_device::irr0_r), FUNC(sh3_base_device::irr0_w));
	map(0x04000006, 0x04000006).rw(FUNC(sh3_base_device::irr1_r), FUNC(sh3_base_device::irr1_w));
	map(0x04000008, 0x04000008).rw(FUNC(sh3_base_device::irr2_r), FUNC(sh3_base_device::irr2_w));
	map(0x04000010, 0x04000011).rw(FUNC(sh3_base_device::icr1_r), FUNC(sh3_base_device::icr1_w));
	map(0x04000012, 0x04000013).rw(FUNC(sh3_base_device::icr2_r), FUNC(sh3_base_device::icr2_w));
	map(0x04000014, 0x04000015).rw(FUNC(sh3_base_device::pinter_r), FUNC(sh3_base_device::pinter_w));
	map(0x04000016, 0x04000017).rw(FUNC(sh3_base_device::iprc_r), FUNC(sh3_base_device::iprc_w));
	map(0x04000018, 0x04000019).rw(FUNC(sh3_base_device::iprd_r), FUNC(sh3_base_device::iprd_w));
	map(0x0400001a, 0x0400001b).rw(FUNC(sh3_base_device::ipre_r), FUNC(sh3_base_device::ipre_w));
}

void sh3_base_device::dmac_7709_map(address_map& map)
{
	map(0x04000020, 0x04000023).rw(FUNC(sh3_base_device::sar0_r), FUNC(sh3_base_device::sar0_w));
	map(0x04000024, 0x04000027).rw(FUNC(sh3_base_device::dar0_r), FUNC(sh3_base_device::dar0_w));
	map(0x04000028, 0x0400002b).rw(FUNC(sh3_base_device::dmatcr0_r), FUNC(sh3_base_device::dmatcr0_w));
	map(0x0400002c, 0x0400002f).rw(FUNC(sh3_base_device::chcr0_r), FUNC(sh3_base_device::chcr0_w));
	map(0x04000030, 0x04000033).rw(FUNC(sh3_base_device::sar1_r), FUNC(sh3_base_device::sar1_w));
	map(0x04000034, 0x04000037).rw(FUNC(sh3_base_device::dar1_r), FUNC(sh3_base_device::dar1_w));
	map(0x04000038, 0x0400003b).rw(FUNC(sh3_base_device::dmatcr1_r), FUNC(sh3_base_device::dmatcr1_w));
	map(0x0400003c, 0x0400003f).rw(FUNC(sh3_base_device::chcr1_r), FUNC(sh3_base_device::chcr1_w));
	map(0x04000040, 0x04000043).rw(FUNC(sh3_base_device::sar2_r), FUNC(sh3_base_device::sar2_w));
	map(0x04000044, 0x04000047).rw(FUNC(sh3_base_device::dar2_r), FUNC(sh3_base_device::dar2_w));
	map(0x04000048, 0x0400004b).rw(FUNC(sh3_base_device::dmatcr2_r), FUNC(sh3_base_device::dmatcr2_w));
	map(0x0400004c, 0x0400004f).rw(FUNC(sh3_base_device::chcr2_r), FUNC(sh3_base_device::chcr2_w));
	map(0x04000050, 0x04000053).rw(FUNC(sh3_base_device::sar3_r), FUNC(sh3_base_device::sar3_w));
	map(0x04000054, 0x04000057).rw(FUNC(sh3_base_device::dar3_r), FUNC(sh3_base_device::dar3_w));
	map(0x04000058, 0x0400005b).rw(FUNC(sh3_base_device::dmatcr3_r), FUNC(sh3_base_device::dmatcr3_w));
	map(0x0400005c, 0x0400005f).rw(FUNC(sh3_base_device::chcr3_r), FUNC(sh3_base_device::chcr3_w));
	map(0x04000060, 0x04000061).lrw16(
		[this](offs_t offset, uint16_t mem_mask) { return dmaor_r(offset, mem_mask); }, "sh3_base_device::dmaor_r",
		[this](offs_t offset, uint16_t data, uint16_t mem_mask) { dmaor_w(offset, data, mem_mask); }, "sh3_base_device::dmaor_w");
}

void sh3_base_device::tmu_map(address_map& map)
{
	map(0xfffffe90, 0xfffffe90).rw(FUNC(sh3_base_device::tocr_r), FUNC(sh3_base_device::tocr_w));
	map(0xfffffe92, 0xfffffe92).rw(FUNC(sh3_base_device::tstr_r), FUNC(sh3_base_device::tstr_w));
	map(0xfffffe94, 0xfffffe97).rw(FUNC(sh3_base_device::tcor0_r), FUNC(sh3_base_device::tcor0_w));
	map(0xfffffe98, 0xfffffe9b).rw(FUNC(sh3_base_device::tcnt0_r), FUNC(sh3_base_device::tcnt0_w));
	map(0xfffffe9c, 0xfffffe9d).rw(FUNC(sh3_base_device::tcr0_r), FUNC(sh3_base_device::tcr0_w));
	map(0xfffffea0, 0xfffffea3).rw(FUNC(sh3_base_device::tcor1_r), FUNC(sh3_base_device::tcor1_w));
	map(0xfffffea4, 0xfffffea7).rw(FUNC(sh3_base_device::tcnt1_r), FUNC(sh3_base_device::tcnt1_w));
	map(0xfffffea8, 0xfffffea9).rw(FUNC(sh3_base_device::tcr1_r), FUNC(sh3_base_device::tcr1_w));
	map(0xfffffeac, 0xfffffeaf).rw(FUNC(sh3_base_device::tcor2_r), FUNC(sh3_base_device::tcor2_w));
	map(0xfffffeb0, 0xfffffeb3).rw(FUNC(sh3_base_device::tcnt2_r), FUNC(sh3_base_device::tcnt2_w));
	map(0xfffffeb4, 0xfffffeb5).rw(FUNC(sh3_base_device::tcr2_r), FUNC(sh3_base_device::tcr2_w));
	map(0xfffffeb8, 0xfffffebb).rw(FUNC(sh3_base_device::tcpr2_r), FUNC(sh3_base_device::tcpr2_w));
}

void sh3_base_device::sci_7708_map(address_map& map)
{
	sci_7709_map(map);

	map(0xffffff7c, 0xffffff7c).rw(FUNC(sh3_base_device::scsptr_r), FUNC(sh3_base_device::scsptr_w));
}

void sh3_base_device::sci_7709_map(address_map& map)
{
	map(0xfffffe80, 0xfffffe80).rw(FUNC(sh3_base_device::scsmr_r), FUNC(sh3_base_device::scsmr_w));
	map(0xfffffe82, 0xfffffe82).rw(FUNC(sh3_base_device::scbrr_r), FUNC(sh3_base_device::scbrr_w));
	map(0xfffffe84, 0xfffffe84).rw(FUNC(sh3_base_device::scscr_r), FUNC(sh3_base_device::scscr_w));
	map(0xfffffe86, 0xfffffe86).rw(FUNC(sh3_base_device::sctdr_r), FUNC(sh3_base_device::sctdr_w));
	map(0xfffffe88, 0xfffffe88).rw(FUNC(sh3_base_device::scssr_r), FUNC(sh3_base_device::scssr_w));
	map(0xfffffe8a, 0xfffffe8a).r(FUNC(sh3_base_device::scrdr_r));
	map(0xfffffe8c, 0xfffffe8c).rw(FUNC(sh3_base_device::scscmr_r), FUNC(sh3_base_device::scscmr_w));
}

void sh3_base_device::cmt_7709_map(address_map& map)
{
	map(0x04000070, 0x04000071).rw(FUNC(sh3_base_device::cmstr_r), FUNC(sh3_base_device::cmstr_w));
	map(0x04000072, 0x04000073).rw(FUNC(sh3_base_device::cmscr_r), FUNC(sh3_base_device::cmscr_w));
	map(0x04000074, 0x04000075).rw(FUNC(sh3_base_device::cmcnt_r), FUNC(sh3_base_device::cmcnt_w));
	map(0x04000076, 0x04000077).rw(FUNC(sh3_base_device::cmcor_r), FUNC(sh3_base_device::cmcor_w));
}

void sh3_base_device::ad_7709_map(address_map& map)
{
	map(0x04000080, 0x04000080).rw(FUNC(sh3_base_device::addrah_r), FUNC(sh3_base_device::addrah_w));
	map(0x04000082, 0x04000082).rw(FUNC(sh3_base_device::addral_r), FUNC(sh3_base_device::addral_w));
	map(0x04000084, 0x04000084).rw(FUNC(sh3_base_device::addrbh_r), FUNC(sh3_base_device::addrbh_w));
	map(0x04000086, 0x04000086).rw(FUNC(sh3_base_device::addrbl_r), FUNC(sh3_base_device::addrbl_w));
	map(0x04000088, 0x04000088).rw(FUNC(sh3_base_device::addrch_r), FUNC(sh3_base_device::addrch_w));
	map(0x0400008a, 0x0400008a).rw(FUNC(sh3_base_device::addrcl_r), FUNC(sh3_base_device::addrcl_w));
	map(0x0400008c, 0x0400008c).rw(FUNC(sh3_base_device::addrdh_r), FUNC(sh3_base_device::addrdh_w));
	map(0x0400008e, 0x0400008e).rw(FUNC(sh3_base_device::addrdl_r), FUNC(sh3_base_device::addrdl_w));
	map(0x04000090, 0x04000090).rw(FUNC(sh3_base_device::adcsr_r), FUNC(sh3_base_device::adcsr_w));
	map(0x04000092, 0x04000092).rw(FUNC(sh3_base_device::adcr_r), FUNC(sh3_base_device::adcr_w));
}

void sh3_base_device::da_7709_map(address_map& map)
{
	map(0x040000a0, 0x040000a0).rw(FUNC(sh3_base_device::dadr0_r), FUNC(sh3_base_device::dadr0_w));
	map(0x040000a2, 0x040000a2).rw(FUNC(sh3_base_device::dadr1_r), FUNC(sh3_base_device::dadr1_w));
	map(0x040000a4, 0x040000a4).rw(FUNC(sh3_base_device::dadcr_r), FUNC(sh3_base_device::dadcr_w));
}

void sh3_base_device::port_7709_map(address_map& map)
{
	map(0x04000100, 0x04000101).rw(FUNC(sh3_base_device::pacr_r), FUNC(sh3_base_device::pacr_w));
	map(0x04000102, 0x04000103).rw(FUNC(sh3_base_device::pbcr_r), FUNC(sh3_base_device::pbcr_w));
	map(0x04000104, 0x04000105).rw(FUNC(sh3_base_device::pccr_r), FUNC(sh3_base_device::pccr_w));
	map(0x04000106, 0x04000107).rw(FUNC(sh3_base_device::pdcr_r), FUNC(sh3_base_device::pdcr_w));
	map(0x04000108, 0x04000109).rw(FUNC(sh3_base_device::pecr_r), FUNC(sh3_base_device::pecr_w));
	map(0x0400010a, 0x0400010b).rw(FUNC(sh3_base_device::pfcr_r), FUNC(sh3_base_device::pfcr_w));
	map(0x0400010c, 0x0400010d).rw(FUNC(sh3_base_device::pgcr_r), FUNC(sh3_base_device::pgcr_w));
	map(0x0400010e, 0x0400010f).rw(FUNC(sh3_base_device::phcr_r), FUNC(sh3_base_device::phcr_w));
	map(0x04000110, 0x04000111).rw(FUNC(sh3_base_device::pjcr_r), FUNC(sh3_base_device::pjcr_w));
	map(0x04000112, 0x04000113).rw(FUNC(sh3_base_device::pkcr_r), FUNC(sh3_base_device::pkcr_w));
	map(0x04000114, 0x04000115).rw(FUNC(sh3_base_device::plcr_r), FUNC(sh3_base_device::plcr_w));
	map(0x04000116, 0x04000117).rw(FUNC(sh3_base_device::scpcr_r), FUNC(sh3_base_device::scpcr_w));
	map(0x04000120, 0x04000120).rw(FUNC(sh3_base_device::padr_r), FUNC(sh3_base_device::padr_w));
	map(0x04000122, 0x04000122).rw(FUNC(sh3_base_device::pbdr_r), FUNC(sh3_base_device::pbdr_w));
	map(0x04000124, 0x04000124).rw(FUNC(sh3_base_device::pcdr_r), FUNC(sh3_base_device::pcdr_w));
	map(0x04000126, 0x04000126).rw(FUNC(sh3_base_device::pddr_r), FUNC(sh3_base_device::pddr_w));
	map(0x04000128, 0x04000128).rw(FUNC(sh3_base_device::pedr_r), FUNC(sh3_base_device::pedr_w));
	map(0x0400012a, 0x0400012a).rw(FUNC(sh3_base_device::pfdr_r), FUNC(sh3_base_device::pfdr_w));
	map(0x0400012c, 0x0400012c).rw(FUNC(sh3_base_device::pgdr_r), FUNC(sh3_base_device::pgdr_w));
	map(0x0400012e, 0x0400012e).rw(FUNC(sh3_base_device::phdr_r), FUNC(sh3_base_device::phdr_w));
	map(0x04000130, 0x04000130).rw(FUNC(sh3_base_device::pjdr_r), FUNC(sh3_base_device::pjdr_w));
	map(0x04000132, 0x04000132).rw(FUNC(sh3_base_device::pkdr_r), FUNC(sh3_base_device::pkdr_w));
	map(0x04000134, 0x04000134).rw(FUNC(sh3_base_device::pldr_r), FUNC(sh3_base_device::pldr_w));
	map(0x04000136, 0x04000136).rw(FUNC(sh3_base_device::scpdr_r), FUNC(sh3_base_device::scpdr_w));
}

void sh3_base_device::irda_7709_map(address_map& map)
{
	map(0x04000140, 0x04000140).rw(FUNC(sh3_base_device::scsmr1_r), FUNC(sh3_base_device::scsmr1_w));
	map(0x04000142, 0x04000142).rw(FUNC(sh3_base_device::scbrr1_r), FUNC(sh3_base_device::scbrr1_w));
	map(0x04000144, 0x04000144).rw(FUNC(sh3_base_device::scscr1_r), FUNC(sh3_base_device::scscr1_w));
	map(0x04000146, 0x04000146).rw(FUNC(sh3_base_device::scftdr1_r), FUNC(sh3_base_device::scftdr1_w));
	map(0x04000148, 0x04000149).rw(FUNC(sh3_base_device::scssr1_r), FUNC(sh3_base_device::scssr1_w));
	map(0x0400014a, 0x0400014a).rw(FUNC(sh3_base_device::scfrdr1_r), FUNC(sh3_base_device::scfrdr1_w));
	map(0x0400014c, 0x0400014c).rw(FUNC(sh3_base_device::scfcr1_r), FUNC(sh3_base_device::scfcr1_w));
	map(0x0400014e, 0x0400014f).rw(FUNC(sh3_base_device::scfdr1_r), FUNC(sh3_base_device::scfdr1_w));
}

void sh3_base_device::scif_7709_map(address_map& map)
{
	map(0x04000150, 0x04000150).rw(FUNC(sh3_base_device::scsmr2_r), FUNC(sh3_base_device::scsmr2_w));
	map(0x04000152, 0x04000152).rw(FUNC(sh3_base_device::scbrr2_r), FUNC(sh3_base_device::scbrr2_w));
	map(0x04000154, 0x04000154).rw(FUNC(sh3_base_device::scscr2_r), FUNC(sh3_base_device::scscr2_w));
	map(0x04000156, 0x04000156).rw(FUNC(sh3_base_device::scftdr2_r), FUNC(sh3_base_device::scftdr2_w));
	map(0x04000158, 0x04000159).rw(FUNC(sh3_base_device::scssr2_r), FUNC(sh3_base_device::scssr2_w));
	map(0x0400015a, 0x0400015a).rw(FUNC(sh3_base_device::scfrdr2_r), FUNC(sh3_base_device::scfrdr2_w));
	map(0x0400015c, 0x0400015c).rw(FUNC(sh3_base_device::scfcr2_r), FUNC(sh3_base_device::scfcr2_w));
	map(0x0400015e, 0x0400015f).rw(FUNC(sh3_base_device::scfdr2_r), FUNC(sh3_base_device::scfdr2_w));
}

void sh3_base_device::udi_7709s_map(address_map& map)
{
	map(0x4000200, 0x4000201).rw(FUNC(sh3_base_device::sdir_r), FUNC(sh3_base_device::sdir_w));
}

sh34_base_device::sh34_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness, address_map_constructor internal)
	: sh_common_execution(mconfig, type, tag, owner, clock, endianness, internal)
	, m_program_config("program", endianness, 64, 32, 0, internal)
	, m_io_config("io", endianness, 64, 8)
	, m_clock(0)
	, m_mmuhack(1)
	, m_bigendian(endianness == ENDIANNESS_BIG)
{
}

device_memory_interface::space_config_vector sh34_base_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

bool sh34_base_device::memory_translate(int spacenum, int intention, offs_t& address, address_space*& target_space)
{
	target_space = &space(spacenum);

	if (address >= 0xe0000000)
		return true;

	if (address >= 0x80000000) // P1/P2/P3 region
	{
		address &= SH34_AM;
		return true;
	}
	else // P0 region
	{
		if (!m_sh4_mmu_enabled)
		{
			address &= SH34_AM;
			return true;
		}
		else
		{
			address = get_remap(address & SH34_AM);
			return true;
		}
	}
}

sh3_base_device::sh3_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh34_base_device(mconfig, type, tag, owner, clock, endianness, address_map_constructor(FUNC(sh3_base_device::sh3_internal_map), this))
{
	m_cpu_type = CPU_TYPE_SH3;
	m_am = SH34_AM;
}


sh4_base_device::sh4_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh34_base_device(mconfig, type, tag, owner, clock, endianness, address_map_constructor(FUNC(sh4_base_device::sh4_internal_map), this))
{
	m_cpu_type = CPU_TYPE_SH4;
	m_am = SH34_AM;
}


sh3_device::sh3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh3_base_device(mconfig, SH3, tag, owner, clock, endianness)
{
}

sh7708s_device::sh7708s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh3_base_device(mconfig, SH7708S, tag, owner, clock, endianness)
{
}

sh7709_device::sh7709_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh3_base_device(mconfig, SH7709, tag, owner, clock, endianness)
{
}

sh7709s_device::sh7709s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh3_base_device(mconfig, SH7709S, tag, owner, clock, endianness)
{
}


sh4_device::sh4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness)
	: sh4_base_device(mconfig, SH4, tag, owner, clock, endianness)
{
}

sh7091_device::sh7091_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh4_base_device(mconfig, SH7091, tag, owner, clock, endianness)
{
}

sh7750_device::sh7750_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh4_base_device(mconfig, SH7750, tag, owner, clock, endianness)
{
}

sh7750s_device::sh7750s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh4_base_device(mconfig, SH7750S, tag, owner, clock, endianness)
{
}

sh7750r_device::sh7750r_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh4_base_device(mconfig, SH7750R, tag, owner, clock, endianness)
{
}

sh7751_device::sh7751_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh4_base_device(mconfig, SH7751, tag, owner, clock, endianness)
{
	m_pciconf0 = 0x35051054;
}

sh7751r_device::sh7751r_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh4_base_device(mconfig, SH7751R, tag, owner, clock, endianness)
{
	m_pciconf0 = 0x350e1054;
}

std::unique_ptr<util::disasm_interface> sh34_base_device::create_disassembler()
{
	return std::make_unique<sh_disassembler>(true);
}


/* Called for unimplemented opcodes */
void sh34_base_device::TODO(const uint16_t opcode)
{
}

void sh34_base_device::LDTLB(const uint16_t opcode)
{
	logerror("unhandled LDTLB for this CPU type\n");
}

void sh4_base_device::LDTLB(const uint16_t opcode)
{
	int replace = (m_mmucr & 0x0000fc00) >> 10;

	logerror("using LDTLB to replace UTLB entry %02x\n", replace);

	// these come from PTEH
	m_utlb[replace].VPN = (m_pteh & 0xfffffc00) >> 10;
	//  m_utlb[replace].D =    (m_pteh & 0x00000200) >> 9; // from PTEL
	//  m_utlb[replace].V =    (m_pteh & 0x00000100) >> 8; // from PTEL
	m_utlb[replace].ASID = (m_pteh & 0x000000ff) >> 0;
	// these come from PTEL
	m_utlb[replace].PPN = (m_ptel & 0x1ffffc00) >> 10;
	m_utlb[replace].V = (m_ptel & 0x00000100) >> 8;
	m_utlb[replace].PSZ = (m_ptel & 0x00000080) >> 6;
	m_utlb[replace].PSZ |= (m_ptel & 0x00000010) >> 4;
	m_utlb[replace].PPR = (m_ptel & 0x00000060) >> 5;
	m_utlb[replace].C = (m_ptel & 0x00000008) >> 3;
	m_utlb[replace].D = (m_ptel & 0x00000004) >> 2;
	m_utlb[replace].SH = (m_ptel & 0x00000002) >> 1;
	m_utlb[replace].WT = (m_ptel & 0x00000001) >> 0;
	// these come from PTEA
	m_utlb[replace].TC = (m_ptea & 0x00000008) >> 3;
	m_utlb[replace].SA = (m_ptea & 0x00000007) >> 0;
}

#if 0
int sign_of(int n)
{
	return(m_sh2_state->m_fr[n] >> 31);
}

void zero(int n, int sign)
{
	if (sign == 0)
		m_sh2_state->m_fr[n] = 0x00000000;
	else
		m_sh2_state->m_fr[n] = 0x80000000;
	if ((m_sh2_state->m_fpscr & PR) == 1)
		m_sh2_state->m_fr[n + 1] = 0x00000000;
}

int data_type_of(int n)
{
	uint32_t abs;

	abs = m_sh2_state->m_fr[n] & 0x7fffffff;
	if ((m_sh2_state->m_fpscr & PR) == 0) /* Single-precision */
	{
		if (abs < 0x00800000)
		{
			if (((m_sh2_state->m_fpscr & DN) == 1) || (abs == 0x00000000))
			{
				if (sign_of(n) == 0)
				{
					zero(n, 0);
					return(SH4_FPU_PZERO);
				}
				else
				{
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			}
			else
				return(SH4_FPU_DENORM);
		}
		else
			if (abs < 0x7f800000)
				return(SH4_FPU_NORM);
			else
				if (abs == 0x7f800000)
				{
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				}
				else
					if (abs < 0x7fc00000)
						return(SH4_FPU_qNaN);
					else
						return(SH4_FPU_sNaN);
	}
	else /* Double-precision */
	{
		if (abs < 0x00100000)
		{
			if (((m_sh2_state->m_fpscr & DN) == 1) || ((abs == 0x00000000) && (m_sh2_state->m_fr[n + 1] == 0x00000000)))
			{
				if (sign_of(n) == 0)
				{
					zero(n, 0);
					return(SH4_FPU_PZERO);
				}
				else
				{
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			}
			else
				return(SH4_FPU_DENORM);
		}
		else
			if (abs < 0x7ff00000)
				return(SH4_FPU_NORM);
			else
				if ((abs == 0x7ff00000) && (m_sh2_state->m_fr[n + 1] == 0x00000000))
				{
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				}
				else
					if (abs < 0x7ff80000)
						return(SH4_FPU_qNaN);
					else
						return(SH4_FPU_sNaN);
	}
	return(SH4_FPU_NORM);
}
#endif

inline uint8_t sh34_base_device::read_byte(offs_t offset)
{
	if (offset >= 0xe0000000)
		return m_program->read_byte(offset);

	if (offset >= 0x80000000) // P1/P2/P3 region
	{
		return m_program->read_byte(offset & SH34_AM);
	}
	else // P0 region
	{
		if (!m_sh4_mmu_enabled)
		{
			return m_program->read_byte(offset & SH34_AM);
		}
		else
		{
			offset = get_remap(offset & SH34_AM);
			return m_program->read_byte(offset);
		}
	}
}

inline uint16_t sh34_base_device::read_word(offs_t offset)
{
	if (offset >= 0xe0000000)
		return m_program->read_word(offset);

	if (offset >= 0x80000000) // P1/P2/P3 region
	{
		return m_program->read_word(offset & SH34_AM);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			return m_program->read_word(offset & SH34_AM);
		}
		else
		{
			offset = get_remap(offset & SH34_AM);
			return m_program->read_word(offset);
		}
	}
}

inline uint16_t sh34_base_device::decrypted_read_word(offs_t offset)
{
	return read_word(offset);
}

inline uint32_t sh34_base_device::read_long(offs_t offset)
{
	if (offset >= 0xe0000000)
		return m_program->read_dword(offset);

	if (offset >= 0x80000000) // P1/P2/P3 region
	{
		return m_program->read_dword(offset & SH34_AM);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			return m_program->read_dword(offset & SH34_AM);
		}
		else
		{
			offset = get_remap(offset & SH34_AM);
			return m_program->read_dword(offset);
		}
	}
}

inline void sh34_base_device::write_byte(offs_t offset, uint8_t data)
{
	if (offset >= 0xe0000000)
	{
		m_program->write_byte(offset, data);
		return;
	}

	if (offset >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_byte(offset & SH34_AM, data);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			m_program->write_byte(offset & SH34_AM, data);
		}
		else
		{
			offset = get_remap(offset & SH34_AM);
			m_program->write_byte(offset, data);
		}
	}
}

inline void sh34_base_device::write_word(offs_t offset, uint16_t data)
{
	if (offset >= 0xe0000000)
	{
		m_program->write_word(offset, data);
		return;
	}

	if (offset >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_word(offset & SH34_AM, data);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			m_program->write_word(offset & SH34_AM, data);
		}
		else
		{
			offset = get_remap(offset & SH34_AM);
			m_program->write_word(offset, data);
		}
	}
}

inline void sh34_base_device::write_long(offs_t offset, uint32_t data)
{
	if (offset >= 0xe0000000)
	{
		m_program->write_dword(offset, data);
		return;
	}

	if (offset >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_dword(offset & SH34_AM, data);
	}
	else
	{
		if (!m_sh4_mmu_enabled)
		{
			m_program->write_dword(offset & SH34_AM, data);
		}
		else
		{
			offset = get_remap(offset & SH34_AM);
			m_program->write_dword(offset, data);
		}
	}
}

inline void sh34_base_device::ILLEGAL()
{
	NOP();
}

/*  MOVCA.L     R0,@Rn */
inline void sh34_base_device::MOVCAL(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[REG_N];
	write_long(m_sh2_state->ea, m_sh2_state->r[0]);
}

inline void sh34_base_device::CLRS(const uint16_t opcode)
{
	m_sh2_state->sr &= ~SH_S;
}

inline void sh34_base_device::SETS(const uint16_t opcode)
{
	m_sh2_state->sr |= SH_S;
}

/*  LDC     Rm,SR */
inline void sh34_base_device::LDCSR(const uint16_t opcode)
{
	// important to store the value now so that it doesn't get affected by the bank change
	uint32_t reg = m_sh2_state->r[REG_N];

	if (debugger_enabled())
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);

	if ((m_sh2_state->r[REG_N] & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_sh2_state->r[REG_N] & sRB ? 1 : 0);

	m_sh2_state->sr = reg & SH34_FLAGS;
	sh4_exception_recompute();
}

/*  LDC.L   @Rm+,SR */
inline void sh34_base_device::LDCMSR(const uint16_t opcode)
{
	uint32_t old = m_sh2_state->sr;
	m_sh2_state->ea = m_sh2_state->r[REG_N];
	m_sh2_state->sr = read_long(m_sh2_state->ea) & SH34_FLAGS;
	if (debugger_enabled())
		sh4_syncronize_register_bank((old & sRB) >> 29);
	if ((old & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_sh2_state->sr & sRB ? 1 : 0);
	m_sh2_state->r[REG_N] += 4;
	m_sh2_state->icount -= 2;
	sh4_exception_recompute();
}

/*  RTE */
inline void sh34_base_device::RTE()
{
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->m_spc;

	if (debugger_enabled())
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);
	if ((m_sh2_state->m_ssr & sRB) != (m_sh2_state->sr & sRB))
		sh4_change_register_bank(m_sh2_state->m_ssr & sRB ? 1 : 0);

	m_sh2_state->sr = m_sh2_state->m_ssr;
	m_sh2_state->icount--;
	sh4_exception_recompute();
}

/*  TRAPA   #imm */
inline void sh34_base_device::TRAPA(uint32_t i)
{
	uint32_t imm = i & 0xff;

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		m_tra = imm << 2;
	}
	else /* SH3 */
	{
		m_tra = imm << 2;
	}

	m_sh2_state->m_ssr = m_sh2_state->sr;
	m_sh2_state->m_spc = m_sh2_state->pc;

	m_sh2_state->m_sgr = m_sh2_state->r[15];

	m_sh2_state->sr |= MD;
	if (debugger_enabled())
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);
	if (!(m_sh2_state->sr & sRB))
		sh4_change_register_bank(1);
	m_sh2_state->sr |= sRB;
	m_sh2_state->sr |= BL;
	sh4_exception_recompute();

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		m_expevt = 0x00000160;
	}
	else /* SH3 */
	{
		m_expevt = 0x00000160;
	}

	m_sh2_state->pc = m_sh2_state->vbr + 0x00000100;

	m_sh2_state->icount -= 7;
}

/*  STCRBANK   Rm_BANK,Rn */
inline void sh34_base_device::STCRBANK(const uint16_t opcode)
{
	uint32_t m = REG_M;

	m_sh2_state->r[REG_N] = m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7];
}

/*  STCMRBANK   Rm_BANK,@-Rn */
inline void sh34_base_device::STCMRBANK(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	write_long(m_sh2_state->ea, m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7]);
	m_sh2_state->icount--;
}

/*  STS.L   SGR,@-Rn */
inline void sh34_base_device::STCMSGR(const uint16_t opcode)
{
	uint32_t n = REG_N;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	write_long(m_sh2_state->ea, m_sh2_state->m_sgr);
}

/*  STS.L   FPUL,@-Rn */
inline void sh34_base_device::STSMFPUL(const uint16_t opcode)
{
	uint32_t n = REG_N;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	write_long(m_sh2_state->ea, m_sh2_state->m_fpul);
}

/*  STS.L   FPSCR,@-Rn */
inline void sh34_base_device::STSMFPSCR(const uint16_t opcode)
{
	uint32_t n = REG_N;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	write_long(m_sh2_state->ea, m_sh2_state->m_fpscr & 0x003FFFFF);
}

/*  STC.L   DBR,@-Rn */
inline void sh34_base_device::STCMDBR(const uint16_t opcode)
{
	uint32_t n = REG_N;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	write_long(m_sh2_state->ea, m_sh2_state->m_dbr);
}

/*  STC.L   SSR,@-Rn */
inline void sh34_base_device::STCMSSR(const uint16_t opcode)
{
	uint32_t n = REG_N;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	write_long(m_sh2_state->ea, m_sh2_state->m_ssr);
}

/*  STC.L   SPC,@-Rn */
inline void sh34_base_device::STCMSPC(const uint16_t opcode)
{
	uint32_t n = REG_N;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	write_long(m_sh2_state->ea, m_sh2_state->m_spc);
}

/*  LDS.L   @Rm+,FPUL */
inline void sh34_base_device::LDSMFPUL(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[REG_N];
	m_sh2_state->m_fpul = read_long(m_sh2_state->ea);
	m_sh2_state->r[REG_N] += 4;
}

/*  LDS.L   @Rm+,FPSCR */
inline void sh34_base_device::LDSMFPSCR(const uint16_t opcode)
{
	uint32_t s = m_sh2_state->m_fpscr;
	m_sh2_state->ea = m_sh2_state->r[REG_N];
	m_sh2_state->m_fpscr = read_long(m_sh2_state->ea);
	m_sh2_state->m_fpscr &= 0x003FFFFF;
	m_sh2_state->r[REG_N] += 4;
	if ((s & FR) != (m_sh2_state->m_fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (m_sh2_state->m_fpscr & PR))
		sh4_swap_fp_couples();
#endif
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
	m_sh2_state->m_fpu_pr = (m_sh2_state->m_fpscr & PR) ? 1 : 0;
}

/*  LDC.L   @Rm+,DBR */
inline void sh34_base_device::LDCMDBR(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[REG_N];
	m_sh2_state->m_dbr = read_long(m_sh2_state->ea);
	m_sh2_state->r[REG_N] += 4;
}

/*  LDC.L   @Rn+,Rm_BANK */
inline void sh34_base_device::LDCMRBANK(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	m_sh2_state->ea = m_sh2_state->r[n];
	m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7] = read_long(m_sh2_state->ea);
	m_sh2_state->r[n] += 4;
}

/*  LDC.L   @Rm+,SSR */
inline void sh34_base_device::LDCMSSR(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[REG_N];
	m_sh2_state->m_ssr = read_long(m_sh2_state->ea);
	m_sh2_state->r[REG_N] += 4;
}

/*  LDC.L   @Rm+,SPC */
inline void sh34_base_device::LDCMSPC(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[REG_N];
	m_sh2_state->m_spc = read_long(m_sh2_state->ea);
	m_sh2_state->r[REG_N] += 4;
}

/*  LDS     Rm,FPUL */
inline void sh34_base_device::LDSFPUL(const uint16_t opcode)
{
	m_sh2_state->m_fpul = m_sh2_state->r[REG_N];
}

/*  LDS     Rm,FPSCR */
inline void sh34_base_device::LDSFPSCR(const uint16_t opcode)
{
	uint32_t s = m_sh2_state->m_fpscr;
	m_sh2_state->m_fpscr = m_sh2_state->r[REG_N] & 0x003FFFFF;
	if ((s & FR) != (m_sh2_state->m_fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (m_sh2_state->m_fpscr & PR))
		sh4_swap_fp_couples();
#endif
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
	m_sh2_state->m_fpu_pr = (m_sh2_state->m_fpscr & PR) ? 1 : 0;
}

/*  LDC     Rm,DBR */
inline void sh34_base_device::LDCDBR(const uint16_t opcode)
{
	m_sh2_state->m_dbr = m_sh2_state->r[REG_N];
}


/*  STC     SSR,Rn */
inline void sh34_base_device::STCSSR(const uint16_t opcode)
{
	m_sh2_state->r[REG_N] = m_sh2_state->m_ssr;
}

/*  STC     SPC,Rn */
inline void sh34_base_device::STCSPC(const uint16_t opcode)
{
	m_sh2_state->r[REG_N] = m_sh2_state->m_spc;
}

/*  STC     SGR,Rn */
inline void sh34_base_device::STCSGR(const uint16_t opcode)
{
	m_sh2_state->r[REG_N] = m_sh2_state->m_sgr;
}

/*  STS     FPUL,Rn */
inline void sh34_base_device::STSFPUL(const uint16_t opcode)
{
	m_sh2_state->r[REG_N] = m_sh2_state->m_fpul;
}

/*  STS     FPSCR,Rn */
inline void sh34_base_device::STSFPSCR(const uint16_t opcode)
{
	m_sh2_state->r[REG_N] = m_sh2_state->m_fpscr & 0x003FFFFF;
}

/*  STC     DBR,Rn */
inline void sh34_base_device::STCDBR(const uint16_t opcode)
{
	m_sh2_state->r[REG_N] = m_sh2_state->m_dbr;
}

/*  SHAD    Rm,Rn */
inline void sh34_base_device::SHAD(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if ((m_sh2_state->r[m] & 0x80000000) == 0)
		m_sh2_state->r[n] = m_sh2_state->r[n] << (m_sh2_state->r[m] & 0x1F);
	else if ((m_sh2_state->r[m] & 0x1F) == 0)
	{
		if ((m_sh2_state->r[n] & 0x80000000) == 0)
			m_sh2_state->r[n] = 0;
		else
			m_sh2_state->r[n] = 0xFFFFFFFF;
	}
	else
		m_sh2_state->r[n] = (int32_t)m_sh2_state->r[n] >> ((~m_sh2_state->r[m] & 0x1F) + 1);
}

/*  SHLD    Rm,Rn */
inline void sh34_base_device::SHLD(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if ((m_sh2_state->r[m] & 0x80000000) == 0)
		m_sh2_state->r[n] = m_sh2_state->r[n] << (m_sh2_state->r[m] & 0x1F);
	else if ((m_sh2_state->r[m] & 0x1F) == 0)
		m_sh2_state->r[n] = 0;
	else
		m_sh2_state->r[n] = m_sh2_state->r[n] >> ((~m_sh2_state->r[m] & 0x1F) + 1);
}

/*  LDCRBANK   Rn,Rm_BANK */
inline void sh34_base_device::LDCRBANK(const uint16_t opcode)
{
	uint32_t m = REG_M;

	m_sh2_state->m_rbnk[m_sh2_state->sr&sRB ? 0 : 1][m & 7] = m_sh2_state->r[REG_N];
}

/*  LDC     Rm,SSR */
inline void sh34_base_device::LDCSSR(const uint16_t opcode)
{
	m_sh2_state->m_ssr = m_sh2_state->r[REG_N];
}

/*  LDC     Rm,SPC */
inline void sh34_base_device::LDCSPC(const uint16_t opcode)
{
	m_sh2_state->m_spc = m_sh2_state->r[REG_N];
}

/*  PREF     @Rn */
inline void sh34_base_device::PREFM(const uint16_t opcode)
{
	uint32_t addr = m_sh2_state->r[REG_N]; // address
	if ((addr >= 0xE0000000) && (addr <= 0xE3FFFFFF))
	{
		uint32_t dest;
		if (m_sh4_mmu_enabled)
		{
			addr = addr & 0xFFFFFFE0;
			// good enough for NAOMI GD-ROM, not much else
			dest = sh4_getsqremap(addr);
		}
		else
		{
			uint32_t sq = (addr & 0x20) >> 5;
			dest = addr & 0x03FFFFE0;
			if (sq == 0)
			{
				if (m_cpu_type == CPU_TYPE_SH4)
				{
					dest |= (m_qacr0 & 0x1C) << 24;
				}
				else
				{
					fatalerror("m_cpu_type != CPU_TYPE_SH4 but access internal regs\n");
				}
			}
			else
			{
				if (m_cpu_type == CPU_TYPE_SH4)
				{
					dest |= (m_qacr1 & 0x1C) << 24;
				}
				else
				{
					fatalerror("m_cpu_type != CPU_TYPE_SH4 but access internal regs\n");
				}

			}
			addr = addr & 0xFFFFFFE0;
		}

		for (int a = 0; a < 4; a++)
		{
			// shouldn't be causing a memory read, should store sq writes in registers.
			m_program->write_qword(dest, m_program->read_qword(addr));
			addr += 8;
			dest += 8;
		}
	}
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/

// TODO: current SZ=1(64bit) FMOVs correct for SH4 in LE mode only

/*  FMOV.S  @Rm+,FRn PR=0 SZ=0 1111nnnnmmmm1001 */
/*  FMOV    @Rm+,DRn PR=0 SZ=1 1111nnn0mmmm1001 */
/*  FMOV    @Rm+,XDn PR=0 SZ=1 1111nnn1mmmm1001 */
/*  FMOV    @Rm+,XDn PR=1      1111nnn1mmmm1001 */
inline void sh34_base_device::FMOVMRIFR(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_sz) /* SZ = 1 */
	{
		if (n & 1)
		{
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_xf[n] = read_long(m_sh2_state->ea);
			m_sh2_state->r[m] += 4;
			m_sh2_state->m_xf[n ^ 1] = read_long(m_sh2_state->ea + 4);
			m_sh2_state->r[m] += 4;
		}
		else
		{
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_fr[n] = read_long(m_sh2_state->ea);
			m_sh2_state->r[m] += 4;
			m_sh2_state->m_fr[n ^ 1] = read_long(m_sh2_state->ea + 4);
			m_sh2_state->r[m] += 4;
		}
	}
	else                /* SZ = 0 */
	{
		m_sh2_state->ea = m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = read_long(m_sh2_state->ea);
		m_sh2_state->r[m] += 4;
	}
}

/*  FMOV.S  FRm,@Rn PR=0 SZ=0 1111nnnnmmmm1010 */
/*  FMOV    DRm,@Rn PR=0 SZ=1 1111nnnnmmm01010 */
/*  FMOV    XDm,@Rn PR=0 SZ=1 1111nnnnmmm11010 */
/*  FMOV    XDm,@Rn PR=1      1111nnnnmmm11010 */
inline void sh34_base_device::FMOVFRMR(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_sz) /* SZ = 1 */
	{
		if (m & 1)
		{
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[n];
			write_long(m_sh2_state->ea, m_sh2_state->m_xf[m]);
			write_long(m_sh2_state->ea + 4, m_sh2_state->m_xf[m ^ 1]);
		}
		else
		{
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[n];
			write_long(m_sh2_state->ea, m_sh2_state->m_fr[m]);
			write_long(m_sh2_state->ea + 4, m_sh2_state->m_fr[m ^ 1]);
		}
	}
	else                /* SZ = 0 */
	{
		m_sh2_state->ea = m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_sh2_state->m_fpu_pr;
#endif
		write_long(m_sh2_state->ea, m_sh2_state->m_fr[m]);
	}
}

/*  FMOV.S  FRm,@-Rn PR=0 SZ=0 1111nnnnmmmm1011 */
/*  FMOV    DRm,@-Rn PR=0 SZ=1 1111nnnnmmm01011 */
/*  FMOV    XDm,@-Rn PR=0 SZ=1 1111nnnnmmm11011 */
/*  FMOV    XDm,@-Rn PR=1      1111nnnnmmm11011 */
inline void sh34_base_device::FMOVFRMDR(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_sz) /* SZ = 1 */
	{
		if (m & 1)
		{
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->r[n] -= 8;
			m_sh2_state->ea = m_sh2_state->r[n];
			write_long(m_sh2_state->ea, m_sh2_state->m_xf[m]);
			write_long(m_sh2_state->ea + 4, m_sh2_state->m_xf[m ^ 1]);
		}
		else
		{
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->r[n] -= 8;
			m_sh2_state->ea = m_sh2_state->r[n];
			write_long(m_sh2_state->ea, m_sh2_state->m_fr[m]);
			write_long(m_sh2_state->ea + 4, m_sh2_state->m_fr[m ^ 1]);
		}
	}
	else                /* SZ = 0 */
	{
		m_sh2_state->r[n] -= 4;
		m_sh2_state->ea = m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_sh2_state->m_fpu_pr;
#endif
		write_long(m_sh2_state->ea, m_sh2_state->m_fr[m]);
	}
}

/*  FMOV.S  FRm,@(R0,Rn) PR=0 SZ=0 1111nnnnmmmm0111 */
/*  FMOV    DRm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm00111 */
/*  FMOV    XDm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm10111 */
/*  FMOV    XDm,@(R0,Rn) PR=1      1111nnnnmmm10111 */
inline void sh34_base_device::FMOVFRS0(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_sz) /* SZ = 1 */
	{
		if (m & 1)
		{
			m &= 14;
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
			write_long(m_sh2_state->ea, m_sh2_state->m_xf[m]);
			write_long(m_sh2_state->ea + 4, m_sh2_state->m_xf[m ^ 1]);
		}
		else
		{
#ifdef LSB_FIRST
			m ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
			write_long(m_sh2_state->ea, m_sh2_state->m_fr[m]);
			write_long(m_sh2_state->ea + 4, m_sh2_state->m_fr[m ^ 1]);
		}
	}
	else                /* SZ = 0 */
	{
		m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[n];
#ifdef LSB_FIRST
		m ^= m_sh2_state->m_fpu_pr;
#endif
		write_long(m_sh2_state->ea, m_sh2_state->m_fr[m]);
	}
}

/*  FMOV.S  @(R0,Rm),FRn PR=0 SZ=0 1111nnnnmmmm0110 */
/*  FMOV    @(R0,Rm),DRn PR=0 SZ=1 1111nnn0mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=0 SZ=1 1111nnn1mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=1      1111nnn1mmmm0110 */
inline void sh34_base_device::FMOVS0FR(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_sz) /* SZ = 1 */
	{
		if (n & 1)
		{
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
			m_sh2_state->m_xf[n] = read_long(m_sh2_state->ea);
			m_sh2_state->m_xf[n ^ 1] = read_long(m_sh2_state->ea + 4);
		}
		else
		{
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
			m_sh2_state->m_fr[n] = read_long(m_sh2_state->ea);
			m_sh2_state->m_fr[n ^ 1] = read_long(m_sh2_state->ea + 4);
		}
	}
	else                /* SZ = 0 */
	{
		m_sh2_state->ea = m_sh2_state->r[0] + m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = read_long(m_sh2_state->ea);
	}
}

/*  FMOV.S  @Rm,FRn PR=0 SZ=0 1111nnnnmmmm1000 */
/*  FMOV    @Rm,DRn PR=0 SZ=1 1111nnn0mmmm1000 */
/*  FMOV    @Rm,XDn PR=0 SZ=1 1111nnn1mmmm1000 */
/*  FMOV    @Rm,XDn PR=1      1111nnn1mmmm1000 */
/*  FMOV    @Rm,DRn PR=1      1111nnn0mmmm1000 */
inline void sh34_base_device::FMOVMRFR(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_sz) /* SZ = 1 */
	{
		if (n & 1)
		{
			n &= 14;
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_xf[n] = read_long(m_sh2_state->ea);
			m_sh2_state->m_xf[n ^ 1] = read_long(m_sh2_state->ea + 4);
		}
		else
		{
#ifdef LSB_FIRST
			n ^= m_sh2_state->m_fpu_pr;
#endif
			m_sh2_state->ea = m_sh2_state->r[m];
			m_sh2_state->m_fr[n] = read_long(m_sh2_state->ea);
			m_sh2_state->m_fr[n ^ 1] = read_long(m_sh2_state->ea + 4);
		}
	}
	else                /* SZ = 0 */
	{
		m_sh2_state->ea = m_sh2_state->r[m];
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = read_long(m_sh2_state->ea);
	}
}

/*  FMOV    FRm,FRn PR=0 SZ=0 FRm -> FRn 1111nnnnmmmm1100 */
/*  FMOV    DRm,DRn PR=0 SZ=1 DRm -> DRn 1111nnn0mmm01100 */
/*  FMOV    XDm,DRn PR=1      XDm -> DRn 1111nnn0mmm11100 */
/*  FMOV    DRm,XDn PR=1      DRm -> XDn 1111nnn1mmm01100 */
/*  FMOV    XDm,XDn PR=1      XDm -> XDn 1111nnn1mmm11100 */
inline void sh34_base_device::FMOVFR(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_sz == 0) /* SZ = 0 */
	{
#ifdef LSB_FIRST
		n ^= m_sh2_state->m_fpu_pr;
		m ^= m_sh2_state->m_fpu_pr;
#endif
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[m];
	}
	else /* SZ = 1 */
	{
		if (m & 1)
		{
			if (n & 1)
			{
				m_sh2_state->m_xf[n & 14] = m_sh2_state->m_xf[m & 14];
				m_sh2_state->m_xf[n | 1] = m_sh2_state->m_xf[m | 1];
			}
			else
			{
				m_sh2_state->m_fr[n] = m_sh2_state->m_xf[m & 14];
				m_sh2_state->m_fr[n | 1] = m_sh2_state->m_xf[m | 1];
			}
		}
		else
		{
			if (n & 1)
			{
				m_sh2_state->m_xf[n & 14] = m_sh2_state->m_fr[m];
				m_sh2_state->m_xf[n | 1] = m_sh2_state->m_fr[m | 1]; // (a&14)+1 -> a|1
			}
			else
			{
				m_sh2_state->m_fr[n] = m_sh2_state->m_fr[m];
				m_sh2_state->m_fr[n | 1] = m_sh2_state->m_fr[m | 1];
			}
		}
	}
}

/*  FLDI1  FRn 1111nnnn10011101 */
inline void sh34_base_device::FLDI1(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fr[REG_N ^ m_sh2_state->m_fpu_pr] = 0x3F800000;
#else
	m_sh2_state->m_fr[REG_N] = 0x3F800000;
#endif
}

/*  FLDI0  FRn 1111nnnn10001101 */
inline void sh34_base_device::FLDI0(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fr[REG_N ^ m_sh2_state->m_fpu_pr] = 0;
#else
	m_sh2_state->m_fr[REG_N] = 0;
#endif
}

/*  FLDS FRm,FPUL 1111mmmm00011101 */
inline void sh34_base_device::FLDS(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fpul = m_sh2_state->m_fr[REG_N ^ m_sh2_state->m_fpu_pr];
#else
	m_sh2_state->m_fpul = m_sh2_state->m_fr[REG_N];
#endif
}

/*  FSTS FPUL,FRn 1111nnnn00001101 */
inline void sh34_base_device::FSTS(const uint16_t opcode)
{
#ifdef LSB_FIRST
	m_sh2_state->m_fr[REG_N ^ m_sh2_state->m_fpu_pr] = m_sh2_state->m_fpul;
#else
	m_sh2_state->m_fr[REG_N] = m_sh2_state->m_fpul;
#endif
}

/* FRCHG 1111101111111101 */
void sh34_base_device::FRCHG()
{
	m_sh2_state->m_fpscr ^= FR;
	sh4_swap_fp_registers();
}

/* FSCHG 1111001111111101 */
void sh34_base_device::FSCHG()
{
	m_sh2_state->m_fpscr ^= SZ;
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
}

/* FTRC FRm,FPUL PR=0 1111mmmm00111101 */
/* FTRC DRm,FPUL PR=1 1111mmm000111101 */
inline void sh34_base_device::FTRC(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		if (n & 1)
			fatalerror("SH-4: FTRC opcode used with n %d", n);

		n = n & 14;
		*((int32_t *)&m_sh2_state->m_fpul) = (int32_t)FP_RFD(n);
	}
	else                /* PR = 0 */
	{
		/* read m_sh2_state->m_fr[n] as float -> truncate -> fpul(32) */
		*((int32_t *)&m_sh2_state->m_fpul) = (int32_t)FP_RFS(n);
	}
}

/* FLOAT FPUL,FRn PR=0 1111nnnn00101101 */
/* FLOAT FPUL,DRn PR=1 1111nnn000101101 */
inline void sh34_base_device::FLOAT(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		if (n & 1)
			fatalerror("SH-4: FLOAT opcode used with n %d", n);

		n = n & 14;
		FP_RFD(n) = (double)*((int32_t *)&m_sh2_state->m_fpul);
	}
	else                /* PR = 0 */
	{
		FP_RFS(n) = (float)*((int32_t *)&m_sh2_state->m_fpul);
	}
}

/* FNEG FRn PR=0 1111nnnn01001101 */
/* FNEG DRn PR=1 1111nnn001001101 */
inline void sh34_base_device::FNEG(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		FP_RFD(n) = -FP_RFD(n);
	}
	else                /* PR = 0 */
	{
		FP_RFS(n) = -FP_RFS(n);
	}
}

/* FABS FRn PR=0 1111nnnn01011101 */
/* FABS DRn PR=1 1111nnn001011101 */
inline void sh34_base_device::FABS(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
#ifdef LSB_FIRST
		n = n | 1; // n & 14 + 1
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[n] & 0x7fffffff;
#else
		n = n & 14;
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[n] & 0x7fffffff;
#endif
	}
	else                /* PR = 0 */
	{
		m_sh2_state->m_fr[n] = m_sh2_state->m_fr[n] & 0x7fffffff;
	}
}

/* FCMP/EQ FRm,FRn PR=0 1111nnnnmmmm0100 */
/* FCMP/EQ DRm,DRn PR=1 1111nnn0mmm00100 */
inline void sh34_base_device::FCMP_EQ(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) == FP_RFD(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
	else                /* PR = 0 */
	{
		if (FP_RFS(n) == FP_RFS(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
}

/* FCMP/GT FRm,FRn PR=0 1111nnnnmmmm0101 */
/* FCMP/GT DRm,DRn PR=1 1111nnn0mmm00101 */
inline void sh34_base_device::FCMP_GT(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) > FP_RFD(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
	else                /* PR = 0 */
	{
		if (FP_RFS(n) > FP_RFS(m))
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
}

/* FCNVDS DRm,FPUL PR=1 1111mmm010111101 */
inline void sh34_base_device::FCNVDS(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		if (m_sh2_state->m_fpscr & RM)
			m_sh2_state->m_fr[n | NATIVE_ENDIAN_VALUE_LE_BE(0, 1)] &= 0xe0000000; /* round toward zero*/
		*((float *)&m_sh2_state->m_fpul) = (float)FP_RFD(n);
	}
}

/* FCNVSD FPUL, DRn PR=1 1111nnn010101101 */
inline void sh34_base_device::FCNVSD(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		FP_RFD(n) = (double)*((float *)&m_sh2_state->m_fpul);
	}
}

/* FADD FRm,FRn PR=0 1111nnnnmmmm0000 */
/* FADD DRm,DRn PR=1 1111nnn0mmm00000 */
inline void sh34_base_device::FADD(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) + FP_RFD(m);
	}
	else                /* PR = 0 */
	{
		FP_RFS(n) = FP_RFS(n) + FP_RFS(m);
	}
}

/* FSUB FRm,FRn PR=0 1111nnnnmmmm0001 */
/* FSUB DRm,DRn PR=1 1111nnn0mmm00001 */
inline void sh34_base_device::FSUB(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) - FP_RFD(m);
	}
	else                /* PR = 0 */
	{
		FP_RFS(n) = FP_RFS(n) - FP_RFS(m);
	}
}


/* FMUL FRm,FRn PR=0 1111nnnnmmmm0010 */
/* FMUL DRm,DRn PR=1 1111nnn0mmm00010 */
inline void sh34_base_device::FMUL(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) * FP_RFD(m);
	}
	else                /* PR = 0 */
	{
		FP_RFS(n) = FP_RFS(n) * FP_RFS(m);
	}
}

/* FDIV FRm,FRn PR=0 1111nnnnmmmm0011 */
/* FDIV DRm,DRn PR=1 1111nnn0mmm00011 */
inline void sh34_base_device::FDIV(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		m = m & 14;
		if (FP_RFD(m) == 0)
			return;
		FP_RFD(n) = FP_RFD(n) / FP_RFD(m);
	}
	else                /* PR = 0 */
	{
		if (FP_RFS(m) == 0)
			return;
		FP_RFS(n) = FP_RFS(n) / FP_RFS(m);
	}
}

/* FMAC FR0,FRm,FRn PR=0 1111nnnnmmmm1110 */
inline void sh34_base_device::FMAC(const uint16_t opcode)
{
	uint32_t m = REG_M;
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr == 0) /* PR = 0 */
	{
		FP_RFS(n) = (FP_RFS(0) * FP_RFS(m)) + FP_RFS(n);
	}
}

/* FSQRT FRn PR=0 1111nnnn01101101 */
/* FSQRT DRn PR=1 1111nnnn01101101 */
inline void sh34_base_device::FSQRT(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (m_sh2_state->m_fpu_pr) /* PR = 1 */
	{
		n = n & 14;
		if (FP_RFD(n) < 0)
			return;
		FP_RFD(n) = sqrtf(FP_RFD(n));
	}
	else                /* PR = 0 */
	{
		if (FP_RFS(n) < 0)
			return;
		FP_RFS(n) = sqrtf(FP_RFS(n));
	}
}

/* FSRRA FRn PR=0 1111nnnn01111101 */
inline void sh34_base_device::FSRRA(const uint16_t opcode)
{
	uint32_t n = REG_N;

	if (FP_RFS(n) < 0)
		return;
	FP_RFS(n) = 1.0f / sqrtf(FP_RFS(n));
}

/*  FSSCA FPUL,FRn PR=0 1111nnn011111101 */
void sh34_base_device::FSSCA(const uint16_t opcode)
{
	uint32_t n = REG_N;

	float angle = (((float)(m_sh2_state->m_fpul & 0xFFFF)) / 65536.0f) * 2.0f * (float)M_PI;
	FP_RFS(n) = sinf(angle);
	FP_RFS(n + 1) = cosf(angle);
}

/* FIPR FVm,FVn PR=0 1111nnmm11101101 */
inline void sh34_base_device::FIPR(const uint16_t opcode)
{
	uint32_t n = REG_N;
	uint32_t m = (n & 3) << 2;
	n = n & 12;

	float ml[4];
	for (int a = 0; a < 4; a++)
		ml[a] = FP_RFS(n + a) * FP_RFS(m + a);
	FP_RFS(n + 3) = ml[0] + ml[1] + ml[2] + ml[3];
}

/* FTRV XMTRX,FVn PR=0 1111nn0111111101 */
void sh34_base_device::FTRV(const uint16_t opcode)
{
	uint32_t n = REG_N;
	n = n & 12;

	float sum[4];
	for (int i = 0; i < 4; i++)
	{
		sum[i] = 0;
		for (int j = 0; j < 4; j++)
			sum[i] += FP_XFS((j << 2) + i)*FP_RFS(n + j);
	}
	for (int i = 0; i < 4; i++)
		FP_RFS(n + i) = sum[i];
}

inline void sh34_base_device::op1111_0xf13(const uint16_t opcode)
{
	if (opcode & 0x100)
	{
		if (opcode & 0x200)
		{
			switch (opcode & 0xC00)
			{
			case 0x000:
				FSCHG();
				break;
			case 0x800:
				FRCHG();
				break;
			default:
				machine().debug_break();
				break;
			}
		}
		else
		{
			FTRV(opcode);
		}
	}
	else
	{
		FSSCA(opcode);
	}
}

void sh34_base_device::dbreak(const uint16_t opcode)
{
	machine().debug_break();
}


inline void sh34_base_device::op1111_0x13(uint16_t opcode)
{
	switch ((opcode >> 4) & 0x0f)
	{
	case 0x00:  FSTS(opcode); break;
	case 0x01:  FLDS(opcode); break;
	case 0x02:  FLOAT(opcode); break;
	case 0x03:  FTRC(opcode); break;
	case 0x04:  FNEG(opcode); break;
	case 0x05:  FABS(opcode); break;
	case 0x06:  FSQRT(opcode); break;
	case 0x07:  FSRRA(opcode); break;
	case 0x08:  FLDI0(opcode); break;
	case 0x09:  FLDI1(opcode); break;
	case 0x0a:  FCNVSD(opcode); break;
	case 0x0b:  FCNVDS(opcode); break;
	case 0x0c:  dbreak(opcode); break;
	case 0x0d:  dbreak(opcode); break;
	case 0x0e:  FIPR(opcode); break;
	case 0x0f:  op1111_0xf13(opcode); break;
	}
}


/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

void sh34_base_device::device_reset()
{
	m_sh2_state->m_spc = 0;
	m_sh2_state->pr = 0;
	m_sh2_state->sr = 0;
	m_sh2_state->m_ssr = 0;
	m_sh2_state->gbr = 0;
	m_sh2_state->vbr = 0;
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
	memset(m_sh2_state->r, 0, sizeof(m_sh2_state->r));
	memset(m_sh2_state->m_rbnk, 0, sizeof(m_sh2_state->m_rbnk));
	m_sh2_state->m_sgr = 0;
	memset(m_sh2_state->m_fr, 0, sizeof(m_sh2_state->m_fr));
	memset(m_sh2_state->m_xf, 0, sizeof(m_sh2_state->m_xf));
	m_sh2_state->ea = 0;
	m_sh2_state->m_delay = 0;
	m_sh2_state->m_cpu_off = 0;
	m_sh2_state->m_pending_irq = 0;
	m_sh2_state->m_test_irq = 0;
	memset(m_exception_priority, 0, sizeof(m_exception_priority));
	memset(m_exception_requesting, 0, sizeof(m_exception_requesting));
	memset(m_irq_line_state, 0, sizeof(m_irq_line_state));

	// CCN
	m_pteh = 0;
	m_ptel = 0;
	m_ttb = 0;
	m_mmucr = 0;
	m_ccr = 0;
	m_tra = 0;
	m_expevt = 0;
	m_intevt = 0;

	// CCN 7709S
	m_ccr2 = 0;

	// CCN 7091
	m_ptea = 0;
	m_qacr0 = 0;
	m_qacr1 = 0;

	// TMU
	m_tocr = 0;
	m_tstr = 0;
	m_tcor0 = 0xffffffff;
	m_tcnt0 = 0xffffffff;
	m_tcr0 = 0;
	m_tcor1 = 0xffffffff;
	m_tcnt1 = 0xffffffff;
	m_tcr1 = 0;
	m_tcor2 = 0xffffffff;
	m_tcnt2 = 0xffffffff;
	m_tcr2 = 0;
	m_tcpr2 = 0;

	// INTC
	m_icr = 0;
	m_ipra = 0;
	m_iprc = 0;

	// INTC 7709
	m_intevt2 = 0;

	// DMAC
	m_sar0 = 0;
	m_sar1 = 0;
	m_sar2 = 0;
	m_sar3 = 0;
	m_dar0 = 0;
	m_dar1 = 0;
	m_dar2 = 0;
	m_dar3 = 0;
	m_chcr0 = 0;
	m_chcr1 = 0;
	m_chcr2 = 0;
	m_chcr3 = 0;
	m_dmatcr0 = 0;
	m_dmatcr1 = 0;
	m_dmatcr2 = 0;
	m_dmatcr3 = 0;
	m_dmaor = 0;

	m_nmi_line_state = 0;
	m_sh2_state->m_frt_input = 0;
	m_internal_irq_vector = 0;
	for (int i = 0; i < 3; i++)
	{
		if (m_timer[i])
			m_timer[i]->adjust(attotime::never, i);
	}
	for (int i = 0; i < 4; i++)
	{
		if (m_dma_timer[i])
			m_dma_timer[i]->adjust(attotime::never, i);
	}
	memset(m_dma_timer_active, 0, sizeof(m_dma_timer_active));
	memset(m_dma_source, 0, sizeof(m_dma_source));
	memset(m_dma_destination, 0, sizeof(m_dma_destination));
	memset(m_dma_count, 0, sizeof(m_dma_count));
	memset(m_dma_wordsize, 0, sizeof(m_dma_wordsize));
	memset(m_dma_source_increment, 0, sizeof(m_dma_source_increment));
	memset(m_dma_destination_increment, 0, sizeof(m_dma_destination_increment));
	memset(m_dma_mode, 0, sizeof(m_dma_mode));
	m_ioport16_pullup = 0;
	m_ioport16_direction = 0;
	m_ioport4_pullup = 0;
	m_ioport4_direction = 0;

	sh4_default_exception_priorities();

	m_sh2_state->pc = 0xa0000000;
	m_sh2_state->m_ppc = m_sh2_state->pc;
	m_sh2_state->r[15] = read_long(4);
	m_sh2_state->sr = 0x700000f0;
	m_sh2_state->m_fpscr = 0x00040001;
	m_sh2_state->m_fpu_sz = (m_sh2_state->m_fpscr & SZ) ? 1 : 0;
	m_sh2_state->m_fpu_pr = (m_sh2_state->m_fpscr & PR) ? 1 : 0;
	m_sh2_state->m_fpul = 0;
	m_sh2_state->m_dbr = 0;

	m_internal_irq_level = -1;
	m_irln = 15;
	m_sh2_state->sleep_mode = 0;

	m_sh4_mmu_enabled = false;
	m_cache_dirty = true;
}

/*-------------------------------------------------
    sh3_reset - reset the processor
-------------------------------------------------*/

void sh3_base_device::device_reset()
{
	sh34_base_device::device_reset();

	// INTC
	m_icr0 &= 0x8000;
	m_iprb = 0;

	// INTC 7709
	m_irr0 = 0;
	m_irr1 = 0;
	m_irr2 = 0;
	m_icr1 = 0;
	m_icr2 = 0;
	m_pinter = 0;
	m_iprd = 0;
	m_ipre = 0;

	// SCI
	m_scsmr = 0;
	m_scbrr = 0xff;
	m_scscr = 0;
	m_sctdr = 0xff;
	m_scssr = 0x84;
	m_scrdr = 0;
	m_scscmr = 0;

	// SCI 7709
	m_scsptr = 0;

	// CMT 7709
	m_cmstr = 0;
	m_cmscr = 0;
	m_cmcnt = 0;
	m_cmcor = 0xffff;

	// AD 7709
	m_addrah = 0;
	m_addral = 0;
	m_addrbh = 0;
	m_addrbl = 0;
	m_addrch = 0;
	m_addrcl = 0;
	m_addrdh = 0;
	m_addrdl = 0;
	m_adcsr = 0;
	m_adcr = 0x07;

	// DA 7709
	m_dadr0 = 0;
	m_dadr1 = 0;
	m_dadcr = 0x1f;

	// PORT 7709
	m_pacr = 0;
	m_pbcr = 0;
	m_pccr = 0xaaaa;
	m_pdcr = 0xaa8a;
	m_pecr = 0xaaaa; // asemd0=0x2aa8
	m_pfcr = 0xaaaa; // asemd0=0x00aa
	m_pgcr = 0xaaaa; // asemd0=0xa200
	m_phcr = 0xaaaa; // asemd0=0x8aaa
	m_pjcr = 0;
	m_pkcr = 0;
	m_plcr = 0;
	m_scpcr = 0xa888;
	m_padr = 0;
	m_pbdr = 0;
	m_pcdr = 0;
	m_pddr = 0;
	m_pedr = 0;
	m_pfdr = 0;
	m_pgdr = 0;
	m_phdr = 0;
	m_pjdr = 0;
	m_pkdr = 0;
	m_pldr = 0;
	m_scpdr = 0;

	// IRDA 7709
	m_scsmr1 = 0;
	m_scbrr1 = 0xff;
	m_scscr1 = 0;
	m_scftdr1 = 0;
	m_scssr1 = 0x60;
	m_scfrdr1 = 0;
	m_scfcr1 = 0;
	m_scfdr1 = 0;

	// SCIF 7709
	m_scsmr2 = 0;
	m_scbrr2 = 0xff;
	m_scscr2 = 0;
	m_scftdr2 = 0;
	m_scssr2 = 0x60;
	m_scfrdr2 = 0;
	m_scfcr2 = 0;
	m_scfdr2 = 0;

	// UDI 7709S
	m_sdir = 0xffff;
}

void sh4_base_device::device_reset()
{
	sh34_base_device::device_reset();

	m_rtc_timer->adjust(attotime::from_hz(128));
	m_refresh_timer->adjust(attotime::never);
	m_refresh_timer_base = 0;

	// DMAC 7750R
	m_sar4 = 0;
	m_dar4 = 0;
	m_sar5 = 0;
	m_dar5 = 0;
	m_sar6 = 0;
	m_dar6 = 0;
	m_sar7 = 0;
	m_dar7 = 0;
	m_chcr4 = 0;
	m_chcr5 = 0;
	m_chcr6 = 0;
	m_chcr7 = 0;
	m_dmatcr4 = 0;
	m_dmatcr5 = 0;
	m_dmatcr6 = 0;
	m_dmatcr7 = 0;

	// INTC
	m_iprb = 0;

	// INTC 7750S
	m_iprd = 0xda74;

	// SCI
	m_scsmr1 = 0;
	m_scbrr1 = 0xff;
	m_scscr1 = 0;
	m_sctdr1 = 0xff;
	m_scssr1 = 0x84;
	m_scrdr1 = 0x00;
	m_scscmr1 = 0x00;
	m_scsptr1 = 0;

	// SCIF
	m_scsmr2 = 0;
	m_scbrr2 = 0xff;
	m_scscr2 = 0;
	m_scftdr2 = 0;
	m_scfsr2 = 0x60;
	m_scfrdr2 = 0;
	m_scfcr2 = 0;
	m_scfdr2 = 0;
	m_scsptr2 = 0;
	m_sclsr2 = 0;
}

inline void sh34_base_device::execute_one_0000(const uint16_t opcode)
{
	switch (opcode & 0xff)
	{
	default:
		// fall through to SH2 handlers
		sh_common_execution::execute_one_0000(opcode); break;

	case 0x52:
	case 0x62:
	case 0x43:
	case 0x63:
	case 0xe3:
	case 0x68:
	case 0xe8:
	case 0x4a:
	case 0xca:
		ILLEGAL(); break; // illegal on sh4

	case 0x93:
	case 0xa3:
	case 0xb3:
		TODO(opcode); break;

	case 0x82:
	case 0x92:
	case 0xa2:
	case 0xb2:
	case 0xc2:
	case 0xd2:
	case 0xe2:
	case 0xf2:
		STCRBANK(opcode); break; // sh4 only

	case 0x32:  STCSSR(opcode); break; // sh4 only
	case 0x42:  STCSPC(opcode); break; // sh4 only
	case 0x83:  PREFM(opcode); break; // sh4 only
	case 0xc3:  MOVCAL(opcode); break; // sh4 only

	case 0x38:
	case 0xb8:
		LDTLB(opcode); break; // sh4 only

	case 0x48:
	case 0xc8:
		CLRS(opcode); break; // sh4 only

	case 0x58:
	case 0xd8:
		SETS(opcode); break; // sh4 only

	case 0x3a:
	case 0xba:
		STCSGR(opcode); break; // sh4 only

	case 0x5a:
	case 0xda:
		STSFPUL(opcode); break; // sh4 only

	case 0x6a:
	case 0xea:
		STSFPSCR(opcode); break; // sh4 only

	case 0x7a:
	case 0xfa:
		STCDBR(opcode); break; // sh4 only
	}
}

inline void sh34_base_device::execute_one_4000(const uint16_t opcode)
{
	switch (opcode & 0xff)
	{

	default: // LDCMSR (0x0e) has sh2/4 flag difference
		// fall through to SH2 handlers
		sh_common_execution::execute_one_4000(opcode); break;

	case 0x42:
	case 0x46:
	case 0x4a:
	case 0x53:
	case 0x57:
	case 0x5e:
	case 0x63:
	case 0x67:
	case 0x6e:
	case 0x82:
	case 0x86:
	case 0x8a:
	case 0x92:
	case 0x96:
	case 0x9a:
	case 0xa2:
	case 0xa6:
	case 0xaa:
	case 0xc2:
	case 0xc6:
	case 0xca:
	case 0xd2:
	case 0xd6:
	case 0xda:
	case 0xe2:
	case 0xe6:
	case 0xea:
		ILLEGAL(); break; // defined as illegal on SH4

	case 0x0c:
	case 0x1c:
	case 0x2c:
	case 0x3c:
	case 0x4c:
	case 0x5c:
	case 0x6c:
	case 0x7c:
	case 0x8c:
	case 0x9c:
	case 0xac:
	case 0xbc:
	case 0xcc:
	case 0xdc:
	case 0xec:
	case 0xfc:
		SHAD(opcode); break; // sh3/4 only

	case 0x0d:
	case 0x1d:
	case 0x2d:
	case 0x3d:
	case 0x4d:
	case 0x5d:
	case 0x6d:
	case 0x7d:
	case 0x8d:
	case 0x9d:
	case 0xad:
	case 0xbd:
	case 0xcd:
	case 0xdd:
	case 0xed:
	case 0xfd:
		SHLD(opcode); break; // sh3/4 only

	case 0x8e:
	case 0x9e:
	case 0xae:
	case 0xbe:
	case 0xce:
	case 0xde:
	case 0xee:
	case 0xfe:
		LDCRBANK(opcode); break; // sh3/4 only

	case 0x83:
	case 0x93:
	case 0xa3:
	case 0xb3:
	case 0xc3:
	case 0xd3:
	case 0xe3:
	case 0xf3:
		STCMRBANK(opcode); break; // sh3/4 only

	case 0x87:
	case 0x97:
	case 0xa7:
	case 0xb7:
	case 0xc7:
	case 0xd7:
	case 0xe7:
	case 0xf7:
		LDCMRBANK(opcode); break; // sh3/4 only

	case 0x32:  STCMSGR(opcode); break; // sh4 only
	case 0x33:  STCMSSR(opcode); break; // sh4 only
	case 0x37:  LDCMSSR(opcode); break; // sh4 only
	case 0x3e:  LDCSSR(opcode); break; // sh4 only
	case 0x43:  STCMSPC(opcode); break; // sh4 only
	case 0x47:  LDCMSPC(opcode); break; // sh4 only
	case 0x4e:  LDCSPC(opcode); break; // sh4 only
	case 0x52:  STSMFPUL(opcode); break; // sh4 only
	case 0x56:  LDSMFPUL(opcode); break; // sh4 only
	case 0x5a:  LDSFPUL(opcode); break; // sh4 only
	case 0x62:  STSMFPSCR(opcode); break; // sh4 only
	case 0x66:  LDSMFPSCR(opcode); break; // sh4 only
	case 0x6a:  LDSFPSCR(opcode); break; // sh4 only
	case 0xf2:  STCMDBR(opcode); break; // sh4 only
	case 0xf6:  LDCMDBR(opcode); break; // sh4 only
	case 0xfa:  LDCDBR(opcode); break; // sh4 only

	}
}

inline void sh34_base_device::execute_one_f000(const uint16_t opcode)
{
	// the SH3 doesn't have these?

	switch (opcode & 0x0f)
	{
	case 0x00:  FADD(opcode); break;
	case 0x01:  FSUB(opcode); break;
	case 0x02:  FMUL(opcode); break;
	case 0x03:  FDIV(opcode); break;
	case 0x04:  FCMP_EQ(opcode); break;
	case 0x05:  FCMP_GT(opcode); break;
	case 0x06:  FMOVS0FR(opcode); break;
	case 0x07:  FMOVFRS0(opcode); break;
	case 0x08:  FMOVMRFR(opcode); break;
	case 0x09:  FMOVMRIFR(opcode); break;
	case 0x0a:  FMOVFRMR(opcode); break;
	case 0x0b:  FMOVFRMDR(opcode); break;
	case 0x0c:  FMOVFR(opcode); break;
	case 0x0d:  op1111_0x13(opcode); break;
	case 0x0e:  FMAC(opcode); break;
	case 0x0f:  dbreak(opcode); break;
	}
}

/* Execute cycles - returns number of cycles actually run */
void sh34_base_device::execute_run()
{
	if (m_isdrc)
	{
		execute_run_drc();
		return;
	}

	if (m_sh2_state->m_cpu_off)
	{
		debugger_wait_hook();
		m_sh2_state->icount = 0;
		return;
	}

	do
	{
		m_sh2_state->m_ppc = m_sh2_state->pc;
		debugger_instruction_hook(m_sh2_state->pc);

		uint16_t opcode;

		if (!m_sh4_mmu_enabled) opcode = m_pr16(m_sh2_state->pc & SH34_AM);
		else opcode = read_word(m_sh2_state->pc); // should probably use a different function as this needs to go through the ITLB

		if (m_sh2_state->m_delay)
		{
			m_sh2_state->pc = m_sh2_state->m_delay;
			m_sh2_state->m_delay = 0;
		}
		else
			m_sh2_state->pc += 2;

		execute_one(opcode);

		if (m_sh2_state->m_test_irq && !m_sh2_state->m_delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}

		m_sh2_state->icount--;
	} while (m_sh2_state->icount > 0);
}

void sh3_base_device::device_start()
{
	sh34_base_device::device_start();

	// UBC
	m_bara = 0;
	m_bamra = 0;
	m_bbra = 0;
	m_barb = 0;
	m_bamrb = 0;
	m_bbrb = 0;
	m_bdrb = 0;
	m_bdmrb = 0;
	m_brcr = 0;

	// UBC 7709S
	m_betr = 0;
	m_brsr = 0;
	m_brdr = 0;

	// CPG
	m_frqcr = 0x0102;
	m_wtcnt = 0;
	m_wtcsr = 0;
	m_stbcr = 0;

	// CPG 7709
	m_stbcr2 = 0;

	// BSC
	m_bcr1 = 0;
	m_bcr2 = 0x3ff0;
	m_wcr1 = 0x3ff3;
	m_wcr2 = 0xffff;
	m_mcr = 0;
	m_pcr = 0;
	m_rtcsr = 0;
	m_rtcnt = 0;
	m_rtcor = 0;
	m_rfcr = 0;

	// BSC 7708
	m_dcr = 0;
	m_pctr = 0;
	m_pdtr = 0;

	// BSC 7709
	m_bcr3 = 0;

	// BSC 7709S
	m_mcscr0 = 0;
	m_mcscr1 = 0;
	m_mcscr2 = 0;
	m_mcscr3 = 0;
	m_mcscr4 = 0;
	m_mcscr5 = 0;
	m_mcscr6 = 0;
	m_mcscr7 = 0;

	// RTC
	m_r64cnt = 0;
	m_rseccnt = 0;
	m_rmincnt = 0;
	m_rhrcnt = 0;
	m_rwkcnt = 0;
	m_rmoncnt = 0;
	m_rdaycnt = 0;
	m_ryrcnt = 0;
	m_rsecar = 0;
	m_rminar = 0;
	m_rhrar = 0;
	m_rwkar = 0;
	m_rdayar = 0;
	m_rmonar = 0;
	m_rcr1 = 0;
	m_rcr2 = 0x09;

	// INTC
	m_icr0 = 0;

	// UBC
	save_item(NAME(m_bara));
	save_item(NAME(m_bamra));
	save_item(NAME(m_bbra));
	save_item(NAME(m_barb));
	save_item(NAME(m_bamrb));
	save_item(NAME(m_bbrb));
	save_item(NAME(m_bdrb));
	save_item(NAME(m_bdmrb));
	save_item(NAME(m_brcr));

	// UBC 7709S
	save_item(NAME(m_betr));
	save_item(NAME(m_brsr));
	save_item(NAME(m_brdr));

	// CPG
	save_item(NAME(m_frqcr));
	save_item(NAME(m_stbcr));
	save_item(NAME(m_wtcnt));
	save_item(NAME(m_wtcsr));

	// CPG 7709
	save_item(NAME(m_stbcr2));

	// BSC
	save_item(NAME(m_bcr1));
	save_item(NAME(m_bcr2));
	save_item(NAME(m_wcr1));
	save_item(NAME(m_wcr2));
	save_item(NAME(m_mcr));
	save_item(NAME(m_pcr));
	save_item(NAME(m_rtcsr));
	save_item(NAME(m_rtcnt));
	save_item(NAME(m_rtcor));
	save_item(NAME(m_rfcr));

	// BSC 7708
	save_item(NAME(m_dcr));
	save_item(NAME(m_pctr));
	save_item(NAME(m_pdtr));

	// BSC 7709
	save_item(NAME(m_bcr3));

	// BSC 7709S
	save_item(NAME(m_mcscr0));
	save_item(NAME(m_mcscr1));
	save_item(NAME(m_mcscr2));
	save_item(NAME(m_mcscr3));
	save_item(NAME(m_mcscr4));
	save_item(NAME(m_mcscr5));
	save_item(NAME(m_mcscr6));
	save_item(NAME(m_mcscr7));

	// RTC
	save_item(NAME(m_r64cnt));
	save_item(NAME(m_rseccnt));
	save_item(NAME(m_rmincnt));
	save_item(NAME(m_rhrcnt));
	save_item(NAME(m_rwkcnt));
	save_item(NAME(m_rmoncnt));
	save_item(NAME(m_rdaycnt));
	save_item(NAME(m_ryrcnt));
	save_item(NAME(m_rsecar));
	save_item(NAME(m_rminar));
	save_item(NAME(m_rhrar));
	save_item(NAME(m_rwkar));
	save_item(NAME(m_rdayar));
	save_item(NAME(m_rmonar));
	save_item(NAME(m_rcr1));
	save_item(NAME(m_rcr2));

	// INTC
	save_item(NAME(m_icr0));
	save_item(NAME(m_iprb));

	// INTC 7709
	save_item(NAME(m_irr0));
	save_item(NAME(m_irr1));
	save_item(NAME(m_irr2));
	save_item(NAME(m_icr1));
	save_item(NAME(m_icr2));
	save_item(NAME(m_pinter));
	save_item(NAME(m_iprd));
	save_item(NAME(m_ipre));

	// SCI
	save_item(NAME(m_scsmr));
	save_item(NAME(m_scbrr));
	save_item(NAME(m_scscr));
	save_item(NAME(m_sctdr));
	save_item(NAME(m_scssr));
	save_item(NAME(m_scrdr));
	save_item(NAME(m_scscmr));

	// SCI 7709
	save_item(NAME(m_scsptr));

	// CMT 7709
	save_item(NAME(m_cmstr));
	save_item(NAME(m_cmscr));
	save_item(NAME(m_cmcnt));
	save_item(NAME(m_cmcor));

	// AD 7709
	save_item(NAME(m_addrah));
	save_item(NAME(m_addral));
	save_item(NAME(m_addrbh));
	save_item(NAME(m_addrbl));
	save_item(NAME(m_addrch));
	save_item(NAME(m_addrcl));
	save_item(NAME(m_addrdh));
	save_item(NAME(m_addrdl));
	save_item(NAME(m_adcsr));
	save_item(NAME(m_adcr));

	// DA 7709
	save_item(NAME(m_dadr0));
	save_item(NAME(m_dadr1));
	save_item(NAME(m_dadcr));

	// PORT 7709
	save_item(NAME(m_pacr));
	save_item(NAME(m_pbcr));
	save_item(NAME(m_pccr));
	save_item(NAME(m_pdcr));
	save_item(NAME(m_pecr));
	save_item(NAME(m_pfcr));
	save_item(NAME(m_pgcr));
	save_item(NAME(m_phcr));
	save_item(NAME(m_pjcr));
	save_item(NAME(m_pkcr));
	save_item(NAME(m_plcr));
	save_item(NAME(m_scpcr));
	save_item(NAME(m_padr));
	save_item(NAME(m_pbdr));
	save_item(NAME(m_pcdr));
	save_item(NAME(m_pddr));
	save_item(NAME(m_pedr));
	save_item(NAME(m_pfdr));
	save_item(NAME(m_pgdr));
	save_item(NAME(m_phdr));
	save_item(NAME(m_pjdr));
	save_item(NAME(m_pkdr));
	save_item(NAME(m_pldr));
	save_item(NAME(m_scpdr));

	// IRDA 7709
	save_item(NAME(m_scsmr1));
	save_item(NAME(m_scbrr1));
	save_item(NAME(m_scscr1));
	save_item(NAME(m_scftdr1));
	save_item(NAME(m_scssr1));
	save_item(NAME(m_scfrdr1));
	save_item(NAME(m_scfcr1));
	save_item(NAME(m_scfdr1));

	// SCIF 7709
	save_item(NAME(m_scsmr2));
	save_item(NAME(m_scbrr2));
	save_item(NAME(m_scscr2));
	save_item(NAME(m_scftdr2));
	save_item(NAME(m_scssr2));
	save_item(NAME(m_scfrdr2));
	save_item(NAME(m_scfcr2));
	save_item(NAME(m_scfdr2));

	// UDI 7709S
	save_item(NAME(m_sdir));
}

void sh4_base_device::device_start()
{
	sh34_base_device::device_start();

	m_rtc_timer = timer_alloc(FUNC(sh4_base_device::sh4_rtc_timer_callback), this);
	m_refresh_timer = timer_alloc(FUNC(sh4_base_device::sh4_refresh_timer_callback), this);

	int i;
	for (i = 0;i < 64;i++)
	{
		m_utlb[i].ASID = 0;
		m_utlb[i].VPN = 0;
		m_utlb[i].V = 0;
		m_utlb[i].PPN = 0;
		m_utlb[i].PSZ = 0;
		m_utlb[i].SH = 0;
		m_utlb[i].C = 0;
		m_utlb[i].PPR = 0;
		m_utlb[i].D = 0;
		m_utlb[i].WT = 0;
		m_utlb[i].SA = 0;
		m_utlb[i].TC = 0;
	}

	// UBC
	m_bara = 0;
	m_bamra = 0;
	m_bbra = 0;
	m_barb = 0;
	m_bamrb = 0;
	m_bbrb = 0;
	m_bdrb = 0;
	m_bdmrb = 0;
	m_brcr = 0;

	// BSC
	m_bcr1 = 0;
	m_bcr2 = 0;
	m_wcr1 = 0;
	m_wcr2 = 0;
	m_wcr3 = 0;
	m_mcr = 0;
	m_pcr = 0;
	m_rtcsr = 0;
	m_rtcnt = 0;
	m_rtcor = 0;
	m_rfcr = 0;
	m_pctra = 0;
	m_pdtra = 0;
	m_pctrb = 0;
	m_pdtrb = 0;
	m_gpioic = 0;

	// BSC 7750R
	m_bcr3 = 0;
	m_bcr4 = 0;

	// CPG
	m_frqcr = 0;
	m_stbcr = 0;
	m_wtcnt = 0;
	m_wtcsr = 0;
	m_stbcr2 = 0;

	// RTC
	m_r64cnt = 0;
	m_rseccnt = 0;
	m_rmincnt = 0;
	m_rhrcnt = 0;
	m_rwkcnt = 0;
	m_rmoncnt = 0;
	m_rdaycnt = 0;
	m_ryrcnt = 0;
	m_rsecar = 0;
	m_rminar = 0;
	m_rhrar = 0;
	m_rwkar = 0;
	m_rdayar = 0;
	m_rmonar = 0;
	m_rcr1 = 0;
	m_rcr2 = 0x09;

	// RTC 7750R
	m_rcr3 = 0;
	m_ryrar = 0;

	// INTC 7750R
	m_intpri00 = 0;
	m_intreq00 = 0;
	m_intmsk00 = 0x000003ff;

	// CPG 7750R
	m_clkstp00 = 0;

	// TMU 7750R
	m_tstr2 = 0;
	m_tcor3 = 0xffffffff;
	m_tcnt3 = 0xffffffff;
	m_tcr3 = 0;
	m_tcor4 = 0xffffffff;
	m_tcnt4 = 0xffffffff;
	m_tcr4 = 0;

	// PCI 7751
	m_pciconf1 = 0x02900080;
	m_pciconf2 = 0;
	m_pciconf3 = 0;
	m_pciconf4 = 1;
	m_pciconf5 = 0;
	m_pciconf6 = 0;
	m_pciconf7 = 0;
	m_pciconf8 = 0;
	m_pciconf9 = 0;
	m_pciconf10 = 0;
	m_pciconf11 = 0;
	m_pciconf12 = 0;
	m_pciconf13 = 0x00000040;
	m_pciconf14 = 0;
	m_pciconf15 = 0x00000100;
	m_pciconf16 = 0x00010001;
	m_pciconf17 = 0;
	m_pcicr = 0;
	m_pcilsr0 = 0;
	m_pcilsr1 = 0;
	m_pcilar0 = 0;
	m_pcilar1 = 0;
	m_pciint = 0;
	m_pciintm = 0;
	m_pcialr = 0;
	m_pciclr = 0;
	m_pciaint = 0;
	m_pciaintm = 0;
	m_pcibllr = 0;
	m_pcidmabt = 0;
	m_pcidpa0 = 0;
	m_pcidla0 = 0;
	m_pcidtc0 = 0;
	m_pcidcr0 = 0;
	m_pcidpa1 = 0;
	m_pcidla1 = 0;
	m_pcidtc1 = 0;
	m_pcidcr1 = 0;
	m_pcidpa2 = 0;
	m_pcidla2 = 0;
	m_pcidtc2 = 0;
	m_pcidcr2 = 0;
	m_pcidpa3 = 0;
	m_pcidla3 = 0;
	m_pcidtc3 = 0;
	m_pcidcr3 = 0;
	m_pcipar = 0;
	m_pcimbr = 0;
	m_pciiobr = 0;
	m_pcipint = 0;
	m_pcipintm = 0;
	m_pciclkr = 0;
	m_pcibcr1 = 0;
	m_pcibcr2 = 0x00003ffc;
	m_pcibcr3 = 1;
	m_pciwcr1 = 0x77777777;
	m_pciwcr2 = 0xfffeefff;
	m_pciwcr3 = 0x07777777;
	m_pcimcr = 0;
	m_pcipctr = 0;
	m_pcipdtr = 0;
	m_pcipdr = 0;

	// H-UDI
	m_sdir = 0xffff;
	m_sddr = 0;
	m_sdint = 0;

	save_item(NAME(m_refresh_timer_base));

	for (i = 0;i < 64;i++)
	{
		save_item(NAME(m_utlb[i].ASID), i);
		save_item(NAME(m_utlb[i].VPN), i);
		save_item(NAME(m_utlb[i].V), i);
		save_item(NAME(m_utlb[i].PPN), i);
		save_item(NAME(m_utlb[i].PSZ), i);
		save_item(NAME(m_utlb[i].SH), i);
		save_item(NAME(m_utlb[i].C), i);
		save_item(NAME(m_utlb[i].PPR), i);
		save_item(NAME(m_utlb[i].D), i);
		save_item(NAME(m_utlb[i].WT), i);
		save_item(NAME(m_utlb[i].SA), i);
		save_item(NAME(m_utlb[i].TC), i);
	}

	// UBC
	save_item(NAME(m_bara));
	save_item(NAME(m_bamra));
	save_item(NAME(m_bbra));
	save_item(NAME(m_barb));
	save_item(NAME(m_bamrb));
	save_item(NAME(m_bbrb));
	save_item(NAME(m_bdrb));
	save_item(NAME(m_bdmrb));
	save_item(NAME(m_brcr));

	// BSC
	save_item(NAME(m_bcr1));
	save_item(NAME(m_bcr2));
	save_item(NAME(m_wcr1));
	save_item(NAME(m_wcr2));
	save_item(NAME(m_wcr3));
	save_item(NAME(m_mcr));
	save_item(NAME(m_pcr));
	save_item(NAME(m_rtcsr));
	save_item(NAME(m_rtcnt));
	save_item(NAME(m_rtcor));
	save_item(NAME(m_rfcr));
	save_item(NAME(m_pctra));
	save_item(NAME(m_pdtra));
	save_item(NAME(m_pctrb));
	save_item(NAME(m_pdtrb));
	save_item(NAME(m_gpioic));

	// BSC 7750R
	save_item(NAME(m_bcr3));
	save_item(NAME(m_bcr4));

	// DMAC 7750R
	save_item(NAME(m_sar4));
	save_item(NAME(m_dar4));
	save_item(NAME(m_dmatcr4));
	save_item(NAME(m_chcr4));
	save_item(NAME(m_sar5));
	save_item(NAME(m_dar5));
	save_item(NAME(m_dmatcr5));
	save_item(NAME(m_chcr5));
	save_item(NAME(m_sar6));
	save_item(NAME(m_dar6));
	save_item(NAME(m_dmatcr6));
	save_item(NAME(m_chcr6));
	save_item(NAME(m_sar7));
	save_item(NAME(m_dar7));
	save_item(NAME(m_dmatcr7));
	save_item(NAME(m_chcr7));

	// CPG
	save_item(NAME(m_frqcr));
	save_item(NAME(m_stbcr));
	save_item(NAME(m_wtcnt));
	save_item(NAME(m_wtcsr));
	save_item(NAME(m_stbcr2));

	// CPG 7750R
	save_item(NAME(m_clkstp00));

	// RTC
	save_item(NAME(m_r64cnt));
	save_item(NAME(m_rseccnt));
	save_item(NAME(m_rmincnt));
	save_item(NAME(m_rhrcnt));
	save_item(NAME(m_rwkcnt));
	save_item(NAME(m_rmoncnt));
	save_item(NAME(m_rdaycnt));
	save_item(NAME(m_ryrcnt));
	save_item(NAME(m_rsecar));
	save_item(NAME(m_rminar));
	save_item(NAME(m_rhrar));
	save_item(NAME(m_rwkar));
	save_item(NAME(m_rdayar));
	save_item(NAME(m_rmonar));
	save_item(NAME(m_rcr1));
	save_item(NAME(m_rcr2));

	// INTC
	save_item(NAME(m_iprb));

	// INTC 7750S
	save_item(NAME(m_iprd));

	// INTC 7750R
	save_item(NAME(m_intpri00));
	save_item(NAME(m_intreq00));
	save_item(NAME(m_intmsk00));

	// TMU 7750R
	save_item(NAME(m_tstr2));
	save_item(NAME(m_tcor3));
	save_item(NAME(m_tcnt3));
	save_item(NAME(m_tcr3));
	save_item(NAME(m_tcor4));
	save_item(NAME(m_tcnt4));
	save_item(NAME(m_tcr4));

	// RTC 7750R
	save_item(NAME(m_rcr3));
	save_item(NAME(m_ryrar));

	// SCI
	save_item(NAME(m_scsmr1));
	save_item(NAME(m_scbrr1));
	save_item(NAME(m_scscr1));
	save_item(NAME(m_sctdr1));
	save_item(NAME(m_scssr1));
	save_item(NAME(m_scrdr1));
	save_item(NAME(m_scscmr1));
	save_item(NAME(m_scsptr1));

	// SCIF
	save_item(NAME(m_scsmr2));
	save_item(NAME(m_scbrr2));
	save_item(NAME(m_scscr2));
	save_item(NAME(m_scftdr2));
	save_item(NAME(m_scfsr2));
	save_item(NAME(m_scfrdr2));
	save_item(NAME(m_scfcr2));
	save_item(NAME(m_scfdr2));
	save_item(NAME(m_scsptr2));
	save_item(NAME(m_sclsr2));

	// H-UDI
	save_item(NAME(m_sdir));
	save_item(NAME(m_sddr));
	save_item(NAME(m_sdint));

	// PCI 7751
	save_item(NAME(m_pciconf0));
	save_item(NAME(m_pciconf1));
	save_item(NAME(m_pciconf2));
	save_item(NAME(m_pciconf3));
	save_item(NAME(m_pciconf4));
	save_item(NAME(m_pciconf5));
	save_item(NAME(m_pciconf6));
	save_item(NAME(m_pciconf7));
	save_item(NAME(m_pciconf8));
	save_item(NAME(m_pciconf9));
	save_item(NAME(m_pciconf10));
	save_item(NAME(m_pciconf11));
	save_item(NAME(m_pciconf12));
	save_item(NAME(m_pciconf13));
	save_item(NAME(m_pciconf14));
	save_item(NAME(m_pciconf15));
	save_item(NAME(m_pciconf16));
	save_item(NAME(m_pciconf17));
	save_item(NAME(m_pcicr));
	save_item(NAME(m_pcilsr0));
	save_item(NAME(m_pcilsr1));
	save_item(NAME(m_pcilar0));
	save_item(NAME(m_pcilar1));
	save_item(NAME(m_pciint));
	save_item(NAME(m_pciintm));
	save_item(NAME(m_pcialr));
	save_item(NAME(m_pciclr));
	save_item(NAME(m_pciaint));
	save_item(NAME(m_pciaintm));
	save_item(NAME(m_pcibllr));
	save_item(NAME(m_pcidmabt));
	save_item(NAME(m_pcidpa0));
	save_item(NAME(m_pcidla0));
	save_item(NAME(m_pcidtc0));
	save_item(NAME(m_pcidcr0));
	save_item(NAME(m_pcidpa1));
	save_item(NAME(m_pcidla1));
	save_item(NAME(m_pcidtc1));
	save_item(NAME(m_pcidcr1));
	save_item(NAME(m_pcidpa2));
	save_item(NAME(m_pcidla2));
	save_item(NAME(m_pcidtc2));
	save_item(NAME(m_pcidcr2));
	save_item(NAME(m_pcidpa3));
	save_item(NAME(m_pcidla3));
	save_item(NAME(m_pcidtc3));
	save_item(NAME(m_pcidcr3));
	save_item(NAME(m_pcipar));
	save_item(NAME(m_pcimbr));
	save_item(NAME(m_pciiobr));
	save_item(NAME(m_pcipint));
	save_item(NAME(m_pcipintm));
	save_item(NAME(m_pciclkr));
	save_item(NAME(m_pcibcr1));
	save_item(NAME(m_pcibcr2));
	save_item(NAME(m_pcibcr3));
	save_item(NAME(m_pciwcr1));
	save_item(NAME(m_pciwcr2));
	save_item(NAME(m_pciwcr3));
	save_item(NAME(m_pcimcr));
	save_item(NAME(m_pcipctr));
	save_item(NAME(m_pcipdtr));
	save_item(NAME(m_pcipdr));
}



void sh34_base_device::device_start()
{
	m_isdrc = allow_drc();

	sh_common_execution::device_start();

	for (int i = 0; i < 3; i++)
	{
		m_timer[i] = timer_alloc(FUNC(sh34_base_device::sh4_timer_callback), this);
		m_timer[i]->adjust(attotime::never, i);
	}

	for (int i = 0; i < 4; i++)
	{
		m_dma_timer[i] = timer_alloc(FUNC(sh34_base_device::sh4_dmac_callback), this);
		m_dma_timer[i]->adjust(attotime::never, i);
	}

	sh4_parse_configuration();

	m_internal = &space(AS_PROGRAM);
	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);
	if (m_program->endianness() == ENDIANNESS_LITTLE)
	{
		m_program->cache(m_cache64le);
		m_pr16 = [this](offs_t address) -> u16 { return m_cache64le.read_word(address); };
		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
			m_prptr = [this](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache64le.read_ptr(address & ~7));
				ptr += (~address >> 1) & 3;
				return ptr;
			};
		else
			m_prptr = [this](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache64le.read_ptr(address & ~7));
				ptr += (address >> 1) & 3;
				return ptr;
			};
	}
	else
	{
		m_program->cache(m_cache64be);
		m_pr16 = [this](offs_t address) -> u16 { return m_cache64be.read_word(address); };
		if (ENDIANNESS_NATIVE != ENDIANNESS_BIG)
			m_prptr = [this](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache64be.read_ptr(address & ~7));
				ptr += (~address >> 1) & 3;
				return ptr;
			};
		else
			m_prptr = [this](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache64be.read_ptr(address & ~7));
				ptr += (address >> 1) & 3;
				return ptr;
			};
	}

	sh4_default_exception_priorities();
	m_irln = 15;
	m_sh2_state->m_test_irq = 0;


	save_item(NAME(m_sh2_state->m_spc));
	save_item(NAME(m_sh2_state->m_ssr));
	save_item(NAME(m_sh2_state->m_sgr));
	save_item(NAME(m_sh2_state->m_fpscr));
	save_item(NAME(m_sh2_state->m_rbnk));
	save_item(NAME(m_sh2_state->m_fr));
	save_item(NAME(m_sh2_state->m_xf));

	save_item(NAME(m_sh2_state->m_cpu_off));
	save_item(NAME(m_sh2_state->m_pending_irq));
	save_item(NAME(m_sh2_state->m_test_irq));
	save_item(NAME(m_sh2_state->m_fpul));
	save_item(NAME(m_sh2_state->m_dbr));
	save_item(NAME(m_exception_priority));
	save_item(NAME(m_exception_requesting));
	save_item(NAME(m_irq_line_state));

	// CCN
	m_tea = 0;
	m_basra = 0;
	m_basrb = 0;

	// CCN
	save_item(NAME(m_pteh));
	save_item(NAME(m_ptel));
	save_item(NAME(m_ttb));
	save_item(NAME(m_tea));
	save_item(NAME(m_mmucr));
	save_item(NAME(m_basra));
	save_item(NAME(m_basrb));
	save_item(NAME(m_ccr));
	save_item(NAME(m_tra));
	save_item(NAME(m_expevt));
	save_item(NAME(m_intevt));

	// CCN 7709S
	save_item(NAME(m_ccr2));

	// CCN 7091
	save_item(NAME(m_ptea));
	save_item(NAME(m_qacr0));
	save_item(NAME(m_qacr1));

	// TMU
	save_item(NAME(m_tocr));
	save_item(NAME(m_tstr));
	save_item(NAME(m_tcor0));
	save_item(NAME(m_tcnt0));
	save_item(NAME(m_tcr0));
	save_item(NAME(m_tcor1));
	save_item(NAME(m_tcnt1));
	save_item(NAME(m_tcr1));
	save_item(NAME(m_tcor2));
	save_item(NAME(m_tcnt2));
	save_item(NAME(m_tcr2));
	save_item(NAME(m_tcpr2));

	// INTC
	save_item(NAME(m_icr));
	save_item(NAME(m_ipra));
	save_item(NAME(m_iprc));

	// INTC 7709
	save_item(NAME(m_intevt2));

	// DMAC
	save_item(NAME(m_sar0));
	save_item(NAME(m_sar1));
	save_item(NAME(m_sar2));
	save_item(NAME(m_sar3));
	save_item(NAME(m_dar0));
	save_item(NAME(m_dar1));
	save_item(NAME(m_dar2));
	save_item(NAME(m_dar3));
	save_item(NAME(m_chcr0));
	save_item(NAME(m_chcr1));
	save_item(NAME(m_chcr2));
	save_item(NAME(m_chcr3));
	save_item(NAME(m_dmatcr0));
	save_item(NAME(m_dmatcr1));
	save_item(NAME(m_dmatcr2));
	save_item(NAME(m_dmatcr3));
	save_item(NAME(m_dmaor));

	save_item(NAME(m_nmi_line_state));

	save_item(NAME(m_sh2_state->m_frt_input));
	save_item(NAME(m_irln));
	save_item(NAME(m_internal_irq_level));
	save_item(NAME(m_internal_irq_vector));
	save_item(NAME(m_dma_timer_active));
	save_item(NAME(m_dma_source));
	save_item(NAME(m_dma_destination));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_wordsize));
	save_item(NAME(m_dma_source_increment));
	save_item(NAME(m_dma_destination_increment));
	save_item(NAME(m_dma_mode));


	save_item(NAME(m_sh2_state->m_fpu_sz));
	save_item(NAME(m_sh2_state->m_fpu_pr));
	save_item(NAME(m_ioport16_pullup));
	save_item(NAME(m_ioport16_direction));
	save_item(NAME(m_ioport4_pullup));
	save_item(NAME(m_ioport4_direction));
	save_item(NAME(m_sh4_mmu_enabled));

	// Debugger state


	state_add(SH4_DBR, "DBR", m_sh2_state->m_dbr).formatstr("%08X");

	state_add(SH4_R0_BK0, "R0 BK 0", m_sh2_state->m_rbnk[0][0]).formatstr("%08X");
	state_add(SH4_R1_BK0, "R1 BK 0", m_sh2_state->m_rbnk[0][1]).formatstr("%08X");
	state_add(SH4_R2_BK0, "R2 BK 0", m_sh2_state->m_rbnk[0][2]).formatstr("%08X");
	state_add(SH4_R3_BK0, "R3 BK 0", m_sh2_state->m_rbnk[0][3]).formatstr("%08X");
	state_add(SH4_R4_BK0, "R4 BK 0", m_sh2_state->m_rbnk[0][4]).formatstr("%08X");
	state_add(SH4_R5_BK0, "R5 BK 0", m_sh2_state->m_rbnk[0][5]).formatstr("%08X");
	state_add(SH4_R6_BK0, "R6 BK 0", m_sh2_state->m_rbnk[0][6]).formatstr("%08X");
	state_add(SH4_R7_BK0, "R7 BK 0", m_sh2_state->m_rbnk[0][7]).formatstr("%08X");
	state_add(SH4_R0_BK1, "R0 BK 1", m_sh2_state->m_rbnk[1][0]).formatstr("%08X");
	state_add(SH4_R1_BK1, "R1 BK 1", m_sh2_state->m_rbnk[1][1]).formatstr("%08X");
	state_add(SH4_R2_BK1, "R2 BK 1", m_sh2_state->m_rbnk[1][2]).formatstr("%08X");
	state_add(SH4_R3_BK1, "R3 BK 1", m_sh2_state->m_rbnk[1][3]).formatstr("%08X");
	state_add(SH4_R4_BK1, "R4 BK 1", m_sh2_state->m_rbnk[1][4]).formatstr("%08X");
	state_add(SH4_R5_BK1, "R5 BK 1", m_sh2_state->m_rbnk[1][5]).formatstr("%08X");
	state_add(SH4_R6_BK1, "R6 BK 1", m_sh2_state->m_rbnk[1][6]).formatstr("%08X");
	state_add(SH4_R7_BK1, "R7 BK 1", m_sh2_state->m_rbnk[1][7]).formatstr("%08X");
	state_add(SH4_SPC, "SPC", m_sh2_state->m_spc).formatstr("%08X");
	state_add(SH4_SSR, "SSR", m_sh2_state->m_ssr).formatstr("%08X");
	state_add(SH4_SGR, "SGR", m_sh2_state->m_sgr).formatstr("%08X");
	state_add(SH4_FPSCR, "FPSCR", m_sh2_state->m_fpscr).formatstr("%08X");
	state_add(SH4_FPUL, "FPUL", m_sh2_state->m_fpul).formatstr("%08X");

	state_add(SH4_FR0, "FR0", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR1, "FR1", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR2, "FR2", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR3, "FR3", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR4, "FR4", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR5, "FR5", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR6, "FR6", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR7, "FR7", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR8, "FR8", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR9, "FR9", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR10, "FR10", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR11, "FR11", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR12, "FR12", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR13, "FR13", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR14, "FR14", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_FR15, "FR15", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF0, "XF0", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF1, "XF1", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF2, "XF2", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF3, "XF3", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF4, "XF4", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF5, "XF5", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF6, "XF6", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF7, "XF7", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF8, "XF8", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF9, "XF9", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF10, "XF10", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF11, "XF11", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF12, "XF12", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF13, "XF13", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF14, "XF14", m_debugger_temp).callimport().formatstr("%25s");
	state_add(SH4_XF15, "XF15", m_debugger_temp).callimport().formatstr("%25s");

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callimport().callexport().noshow();
	//state_add(STATE_GENPCBASE, "CURPC", m_sh2_state->m_ppc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_sh2_state->pc).callimport().noshow();

	for (int regnum = 0; regnum < 16; regnum++)
	{
		m_fs_regmap[regnum] = uml::mem(((float *)(m_sh2_state->m_fr+(regnum))));
	}

	for (int regnum = 0; regnum < 16; regnum+=2)
	{
		m_fd_regmap[regnum] = uml::mem(((double *)(m_sh2_state->m_fr+(regnum))));
	}

	drc_start();
}

void sh34_base_device::state_import(const device_state_entry &entry)
{
#ifdef LSB_FIRST
	uint8_t fpu_xor = m_sh2_state->m_fpu_pr;
#else
	uint8_t fpu_xor = 0;
#endif

	switch (entry.index())
	{
		case STATE_GENPC:
			m_sh2_state->pc = m_debugger_temp;
			[[fallthrough]];
		case SH4_PC:
			m_sh2_state->m_delay = 0;
			break;

		case SH_SR:
			sh4_exception_recompute();
			sh4_check_pending_irq("sh4_set_info");
			break;

		case SH4_FR0:
			m_sh2_state->m_fr[0 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR1:
			m_sh2_state->m_fr[1 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR2:
			m_sh2_state->m_fr[2 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR3:
			m_sh2_state->m_fr[3 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR4:
			m_sh2_state->m_fr[4 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR5:
			m_sh2_state->m_fr[5 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR6:
			m_sh2_state->m_fr[6 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR7:
			m_sh2_state->m_fr[7 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR8:
			m_sh2_state->m_fr[8 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR9:
			m_sh2_state->m_fr[9 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR10:
			m_sh2_state->m_fr[10 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR11:
			m_sh2_state->m_fr[11 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR12:
			m_sh2_state->m_fr[12 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR13:
			m_sh2_state->m_fr[13 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR14:
			m_sh2_state->m_fr[14 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_FR15:
			m_sh2_state->m_fr[15 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF0:
			m_sh2_state->m_xf[0 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF1:
			m_sh2_state->m_xf[1 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF2:
			m_sh2_state->m_xf[2 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF3:
			m_sh2_state->m_xf[3 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF4:
			m_sh2_state->m_xf[4 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF5:
			m_sh2_state->m_xf[5 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF6:
			m_sh2_state->m_xf[6 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF7:
			m_sh2_state->m_xf[7 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF8:
			m_sh2_state->m_xf[8 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF9:
			m_sh2_state->m_xf[9 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF10:
			m_sh2_state->m_xf[10 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF11:
			m_sh2_state->m_xf[11 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF12:
			m_sh2_state->m_xf[12 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF13:
			m_sh2_state->m_xf[13 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF14:
			m_sh2_state->m_xf[14 ^ fpu_xor] = m_debugger_temp;
			break;

		case SH4_XF15:
			m_sh2_state->m_xf[15 ^ fpu_xor] = m_debugger_temp;
			break;
	}
}

void sh34_base_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case STATE_GENPC:
		m_debugger_temp = m_sh2_state->pc;
		break;
	}
}

void sh34_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
#ifdef LSB_FIRST
	uint8_t fpu_xor = m_sh2_state->m_fpu_pr;
#else
	uint8_t fpu_xor = 0;
#endif

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%s%s%s%s%c%c%d%c%c",
					m_sh2_state->sr & MD ? "MD ":"   ",
					m_sh2_state->sr & sRB ? "RB ":"   ",
					m_sh2_state->sr & BL ? "BL ":"   ",
					m_sh2_state->sr & FD ? "FD ":"   ",
					m_sh2_state->sr & SH_M ? 'M':'.',
					m_sh2_state->sr & SH_Q ? 'Q':'.',
					(m_sh2_state->sr & SH_I) >> 4,
					m_sh2_state->sr & SH_S ? 'S':'.',
					m_sh2_state->sr & SH_T ? 'T':'.');
			break;

		case SH4_FR0:
			str = string_format("%08X %f", m_sh2_state->m_fr[0 ^ fpu_xor], (double)FP_RFS(0 ^ fpu_xor));
			break;

		case SH4_FR1:
			str = string_format("%08X %f", m_sh2_state->m_fr[1 ^ fpu_xor], (double)FP_RFS(1 ^ fpu_xor));
			break;

		case SH4_FR2:
			str = string_format("%08X %f", m_sh2_state->m_fr[2 ^ fpu_xor], (double)FP_RFS(2 ^ fpu_xor));
			break;

		case SH4_FR3:
			str = string_format("%08X %f", m_sh2_state->m_fr[3 ^ fpu_xor], (double)FP_RFS(3 ^ fpu_xor));
			break;

		case SH4_FR4:
			str = string_format("%08X %f", m_sh2_state->m_fr[4 ^ fpu_xor], (double)FP_RFS(4 ^ fpu_xor));
			break;

		case SH4_FR5:
			str = string_format("%08X %f", m_sh2_state->m_fr[5 ^ fpu_xor], (double)FP_RFS(5 ^ fpu_xor));
			break;

		case SH4_FR6:
			str = string_format("%08X %f", m_sh2_state->m_fr[6 ^ fpu_xor], (double)FP_RFS(6 ^ fpu_xor));
			break;

		case SH4_FR7:
			str = string_format("%08X %f", m_sh2_state->m_fr[7 ^ fpu_xor], (double)FP_RFS(7 ^ fpu_xor));
			break;

		case SH4_FR8:
			str = string_format("%08X %f", m_sh2_state->m_fr[8 ^ fpu_xor], (double)FP_RFS(8 ^ fpu_xor));
			break;

		case SH4_FR9:
			str = string_format("%08X %f", m_sh2_state->m_fr[9 ^ fpu_xor], (double)FP_RFS(9 ^ fpu_xor));
			break;

		case SH4_FR10:
			str = string_format("%08X %f", m_sh2_state->m_fr[10 ^ fpu_xor], (double)FP_RFS(10 ^ fpu_xor));
			break;

		case SH4_FR11:
			str = string_format("%08X %f", m_sh2_state->m_fr[11 ^ fpu_xor], (double)FP_RFS(11 ^ fpu_xor));
			break;

		case SH4_FR12:
			str = string_format("%08X %f", m_sh2_state->m_fr[12 ^ fpu_xor], (double)FP_RFS(12 ^ fpu_xor));
			break;

		case SH4_FR13:
			str = string_format("%08X %f", m_sh2_state->m_fr[13 ^ fpu_xor], (double)FP_RFS(13 ^ fpu_xor));
			break;

		case SH4_FR14:
			str = string_format("%08X %f", m_sh2_state->m_fr[14 ^ fpu_xor], (double)FP_RFS(14 ^ fpu_xor));
			break;

		case SH4_FR15:
			str = string_format("%08X %f", m_sh2_state->m_fr[15 ^ fpu_xor], (double)FP_RFS(15 ^ fpu_xor));
			break;

		case SH4_XF0:
			str = string_format("%08X %f", m_sh2_state->m_xf[0 ^ fpu_xor], (double)FP_XFS(0 ^ fpu_xor));
			break;

		case SH4_XF1:
			str = string_format("%08X %f", m_sh2_state->m_xf[1 ^ fpu_xor], (double)FP_XFS(1 ^ fpu_xor));
			break;

		case SH4_XF2:
			str = string_format("%08X %f", m_sh2_state->m_xf[2 ^ fpu_xor], (double)FP_XFS(2 ^ fpu_xor));
			break;

		case SH4_XF3:
			str = string_format("%08X %f", m_sh2_state->m_xf[3 ^ fpu_xor], (double)FP_XFS(3 ^ fpu_xor));
			break;

		case SH4_XF4:
			str = string_format("%08X %f", m_sh2_state->m_xf[4 ^ fpu_xor], (double)FP_XFS(4 ^ fpu_xor));
			break;

		case SH4_XF5:
			str = string_format("%08X %f", m_sh2_state->m_xf[5 ^ fpu_xor], (double)FP_XFS(5 ^ fpu_xor));
			break;

		case SH4_XF6:
			str = string_format("%08X %f", m_sh2_state->m_xf[6 ^ fpu_xor], (double)FP_XFS(6 ^ fpu_xor));
			break;

		case SH4_XF7:
			str = string_format("%08X %f", m_sh2_state->m_xf[7 ^ fpu_xor], (double)FP_XFS(7 ^ fpu_xor));
			break;

		case SH4_XF8:
			str = string_format("%08X %f", m_sh2_state->m_xf[8 ^ fpu_xor], (double)FP_XFS(8 ^ fpu_xor));
			break;

		case SH4_XF9:
			str = string_format("%08X %f", m_sh2_state->m_xf[9 ^ fpu_xor], (double)FP_XFS(9 ^ fpu_xor));
			break;

		case SH4_XF10:
			str = string_format("%08X %f", m_sh2_state->m_xf[10 ^ fpu_xor], (double)FP_XFS(10 ^ fpu_xor));
			break;

		case SH4_XF11:
			str = string_format("%08X %f", m_sh2_state->m_xf[11 ^ fpu_xor], (double)FP_XFS(11 ^ fpu_xor));
			break;

		case SH4_XF12:
			str = string_format("%08X %f", m_sh2_state->m_xf[12 ^ fpu_xor], (double)FP_XFS(12 ^ fpu_xor));
			break;

		case SH4_XF13:
			str = string_format("%08X %f", m_sh2_state->m_xf[13 ^ fpu_xor], (double)FP_XFS(13 ^ fpu_xor));
			break;

		case SH4_XF14:
			str = string_format("%08X %f", m_sh2_state->m_xf[14 ^ fpu_xor], (double)FP_XFS(14 ^ fpu_xor));
			break;

		case SH4_XF15:
			str = string_format("%08X %f", m_sh2_state->m_xf[15 ^ fpu_xor], (double)FP_XFS(15 ^ fpu_xor));
			break;

	}
}

/*
void sh34_base_device::sh4_set_ftcsr_callback(sh4_ftcsr_callback callback)
{
    m_ftcsr_read_callback = callback;
}
*/


const opcode_desc* sh4_base_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}

void sh4_base_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh4_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}

const opcode_desc* sh3_base_device::get_desclist(offs_t pc)
{
	return m_drcfe->describe_code(pc);
}

void sh3_base_device::init_drc_frontend()
{
	m_drcfe = std::make_unique<sh4_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);
}

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

void sh34_base_device::func_CHECKIRQ() { if (m_sh2_state->m_test_irq) sh4_check_pending_irq("mame_sh4_execute"); }
static void cfunc_CHECKIRQ(void *param) { ((sh34_base_device *)param)->func_CHECKIRQ(); };

void sh34_base_device::generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception)
{
	/* check full interrupts if pending */
	if (compiler.checkints)
	{
		compiler.checkints = false;

		/* param is pc + 2 (the opcode after the current one)
		   as we're calling from opcode handlers here that will point to the current opcode instead
		   but I believe the exception function requires it to point to the next one so update the
		   local copy of the PC variable here for that? */
		UML_MOV(block, mem(&m_sh2_state->pc), param);

	//  save_fast_iregs(block);
		UML_CALLC(block, cfunc_CHECKIRQ, this);
		load_fast_iregs(block);

		/* generate a hash jump via the current mode and PC
		   pc should be pointing to either the exception address
		   or have been left on the next PC set above? */
		UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);     // hashjmp <mode>,<pc>,nocode
	}

	/* account for cycles */
	if (compiler.cycles > 0)
	{
		UML_SUB(block, mem(&m_sh2_state->icount), mem(&m_sh2_state->icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, COND_S, *m_out_of_cycles, param);
																					// exh     out_of_cycles,nextpc
	}
	compiler.cycles = 0;
}

/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/


void sh34_base_device::static_generate_entry_point()
{
	//uml::code_label const skip = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(200));

	/* forward references */
	alloc_handle(m_nocode, "nocode");
	alloc_handle(m_write32, "write32");     // necessary?
	alloc_handle(m_entry, "entry");
	UML_HANDLE(block, *m_entry);                         // handle  entry

	UML_CALLC(block, cfunc_CHECKIRQ, this);
	load_fast_iregs(block);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);     // hashjmp <mode>,<pc>,nocode

	block.end();
}


/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

void sh34_base_device::static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0 */
	int label = 1;

	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(1024));

	/* add a global entry for this */
	alloc_handle(handleptr, name);
	UML_HANDLE(block, *handleptr);                         // handle  *handleptr

#if 0
	if (A >= 0xe0000000)
	{
		m_program->write_word(A, V);
		return;
	}

	if (A >= 0x80000000) // P1/P2/P3 region
	{
		m_program->write_word(A & SH34_AM, V);
	}
	else
	{
		// MMU handling
		m_program->write_word(A & SH34_AM, V);
	}
#endif

	UML_CMP(block, I0, 0xe0000000);
	UML_JMPc(block, COND_AE, label);

	UML_AND(block, I0, I0, SH34_AM);     // and r0, r0, #AM (0x1fffffff)

	UML_LABEL(block, label++);              // label:

	if (!debugger_enabled())
	{
		for (auto & elem : m_fastram)
		{
			if (elem.base != nullptr && (!iswrite || !elem.readonly))
			{
				void *fastbase = (uint8_t *)elem.base - elem.start;
				uint32_t skip = label++;
				if (elem.end != 0xffffffff)
				{
					UML_CMP(block, I0, elem.end);   // cmp     i0,end
					UML_JMPc(block, COND_A, skip);                                      // ja      skip
				}
				if (elem.start != 0x00000000)
				{
					UML_CMP(block, I0, elem.start);// cmp     i0,fastram_start
					UML_JMPc(block, COND_B, skip);                                      // jb      skip
				}

				if (!iswrite)
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, m_bigendian ? BYTE8_XOR_BE(0) : BYTE8_XOR_LE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_BYTE, SCALE_x1);             // load    i0,fastbase,i0,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, m_bigendian ? WORD2_XOR_BE(0) : WORD2_XOR_LE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_WORD, SCALE_x1);         // load    i0,fastbase,i0,word_x1
					}
					else if (size == 4)
					{

						UML_XOR(block, I0, I0, m_bigendian ? DWORD_XOR_BE(0) : DWORD_XOR_LE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_DWORD, SCALE_x1);            // load    i0,fastbase,i0,dword_x1
					}
					UML_RET(block);                                                     // ret
				}
				else
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, m_bigendian ? BYTE8_XOR_BE(0) : BYTE8_XOR_LE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_BYTE, SCALE_x1);// store   fastbase,i0,i1,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, m_bigendian ? WORD2_XOR_BE(0) : WORD2_XOR_LE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_WORD, SCALE_x1);// store   fastbase,i0,i1,word_x1
					}
					else if (size == 4)
					{
						UML_XOR(block, I0, I0, m_bigendian ? DWORD_XOR_BE(0) : DWORD_XOR_LE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_DWORD, SCALE_x1);       // store   fastbase,i0,i1,dword_x1
					}
					UML_RET(block);                                                     // ret
				}

				UML_LABEL(block, skip);                                             // skip:
			}
		}
	}

	if (iswrite)
	{
		switch (size)
		{
			case 1:
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM); // write r0, r1, program_byte
				break;

			case 2:
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM); // write r0, r1, program_word
				break;

			case 4:
				UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);    // write r0, r1, program_dword
				break;
		}
	}
	else
	{
		switch (size)
		{
			case 1:
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);  // read r0, program_byte
				break;

			case 2:
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);  // read r0, program_word
				break;

			case 4:
				UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM); // read r0, program_dword
				break;
		}
	}

	UML_RET(block);                         // ret

	block.end();
}

bool sh34_base_device::generate_group_0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0xff)
	{
	default:
		// fall through to SH2 handlers
		return sh_common_execution::generate_group_0(block, compiler, desc, opcode, in_delay_slot, ovrpc);

	case 0x52:
	case 0x62:
	case 0x43:
	case 0x63:
	case 0xe3:
	case 0x68:
	case 0xe8:
	case 0x4a:
	case 0xca:
		return true; // ILLEGAL(); break; // illegal on sh4

	case 0x93:
	case 0xa3:
	case 0xb3:
		return true; // TODO(opcode); break;

	case 0x82:
	case 0x92:
	case 0xa2:
	case 0xb2:
	case 0xc2:
	case 0xd2:
	case 0xe2:
	case 0xf2:
		return generate_group_0_STCRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc);

	case 0x32: return generate_group_0_STCSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only
	case 0x42: return generate_group_0_STCSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only
	case 0x83: return generate_group_0_PREFM(block, compiler, desc, opcode, in_delay_slot, ovrpc);  // sh4 only
	case 0xc3: return generate_group_0_MOVCAL(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x38:
	case 0xb8:
		return generate_group_0_LDTLB(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x48:
	case 0xc8:
		return generate_group_0_CLRS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x58:
	case 0xd8:
		return generate_group_0_SETS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x3a:
	case 0xba:
		return generate_group_0_STCSGR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x5a:
	case 0xda:
		return generate_group_0_STSFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x6a:
	case 0xea:
		return generate_group_0_STSFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x7a:
	case 0xfa:
		return generate_group_0_STCDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only
	}

	return false;
}

void sh34_base_device::func_STCRBANK() { STCRBANK(m_sh2_state->arg0); }
static void cfunc_STCRBANK(void *param) { ((sh34_base_device *)param)->func_STCRBANK(); };

bool sh34_base_device::generate_group_0_STCRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCRBANK, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_0_STCSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(REG_N), mem(&m_sh2_state->m_ssr));
	return true;
}

bool sh34_base_device::generate_group_0_STCSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(REG_N), mem(&m_sh2_state->m_spc));
	return true;
}

void sh34_base_device::func_PREFM() { PREFM(m_sh2_state->arg0); }
static void cfunc_PREFM(void *param) { ((sh34_base_device *)param)->func_PREFM(); };

bool sh34_base_device::generate_group_0_PREFM(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_PREFM, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_0_MOVCAL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, I0, R32(REG_N))
	SETEA(0);
	UML_MOV(block, I1, R32(0));
	UML_CALLH(block, *m_write32);
	return true;
}

void sh34_base_device::func_LDTLB() { LDTLB(m_sh2_state->arg0); }
static void cfunc_LDTLB(void *param) { ((sh34_base_device *)param)->func_LDTLB(); };

bool sh34_base_device::generate_group_0_LDTLB(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDTLB, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_CLRS() { CLRS(m_sh2_state->arg0); }
static void cfunc_CLRS(void *param) { ((sh34_base_device *)param)->func_CLRS(); };

bool sh34_base_device::generate_group_0_CLRS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_CLRS, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_SETS() { SETS(m_sh2_state->arg0); }
static void cfunc_SETS(void *param) { ((sh34_base_device *)param)->func_SETS(); };

bool sh34_base_device::generate_group_0_SETS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_SETS, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_0_STCSGR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(REG_N), mem(&m_sh2_state->m_sgr));
	return true;

}

bool sh34_base_device::generate_group_0_STSFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(REG_N), mem(&m_sh2_state->m_fpul));
	return true;
}

bool sh34_base_device::generate_group_0_STSFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_AND(block, I0, 0x003FFFFF, mem(&m_sh2_state->m_fpscr));
	UML_MOV(block, R32(REG_N), I0);
	return true;
}

bool sh34_base_device::generate_group_0_STCDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, R32(REG_N), mem(&m_sh2_state->m_dbr));
	return true;
}

void sh34_base_device::func_RTE() { RTE(); }
static void cfunc_RTE(void *param) { ((sh34_base_device *)param)->func_RTE();  };

bool sh34_base_device::generate_group_0_RTE(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	generate_delay_slot(block, compiler, desc, 0xffffffff);
	save_fast_iregs(block);
	UML_CALLC(block, cfunc_RTE, this);
	load_fast_iregs(block);

	compiler.checkints = true;

	UML_MOV(block, mem(&m_sh2_state->pc), mem(&m_sh2_state->m_delay));
	generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->ea), true);  // <subtract cycles>
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode); // and jump to the "resume PC"
	return true;
}

void sh34_base_device::func_TRAPA() { TRAPA(m_sh2_state->arg0 & 0xff); }
static void cfunc_TRAPA(void *param) { ((sh34_base_device *)param)->func_TRAPA(); };

bool sh34_base_device::generate_group_12_TRAPA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_MOV(block, mem(&m_sh2_state->pc), desc->pc + 2); // copy the PC because we need to use it
	UML_CALLC(block, cfunc_TRAPA, this);
	load_fast_iregs(block);
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode);
	return true;
}

void sh34_base_device::func_LDCSR() { LDCSR(m_sh2_state->arg0); }
static void cfunc_LDCSR(void *param) { ((sh34_base_device *)param)->func_LDCSR(); };

bool sh34_base_device::generate_group_4_LDCSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCSR, this);
	load_fast_iregs(block);

	compiler.checkints = true;

	return true;
}

void sh34_base_device::func_LDCMSR() { LDCMSR(m_sh2_state->arg0); }
static void cfunc_LDCMSR(void *param) { ((sh34_base_device *)param)->func_LDCMSR(); };

bool sh34_base_device::generate_group_4_LDCMSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMSR, this);
	load_fast_iregs(block);

	compiler.checkints = true;
	if (!in_delay_slot)
		generate_update_cycles(block, compiler, desc->pc + 2, true);

	return true;
}

void sh34_base_device::func_SHAD() { SHAD(m_sh2_state->arg0); }
static void cfunc_SHAD(void *param) { ((sh34_base_device *)param)->func_SHAD(); };

bool sh34_base_device::generate_group_4_SHAD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_SHAD, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_SHLD() { SHLD(m_sh2_state->arg0); }
static void cfunc_SHLD(void *param) { ((sh34_base_device *)param)->func_SHLD(); };

bool sh34_base_device::generate_group_4_SHLD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_SHLD, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0xff)
	{
	default: // LDCMSR (0x0e) has sh2/4 flag difference
		// fall through to SH2 handlers
		return sh_common_execution::generate_group_4(block, compiler, desc, opcode, in_delay_slot, ovrpc); break;

	case 0x42:
	case 0x46:
	case 0x4a:
	case 0x53:
	case 0x57:
	case 0x5e:
	case 0x63:
	case 0x67:
	case 0x6e:
	case 0x82:
	case 0x86:
	case 0x8a:
	case 0x92:
	case 0x96:
	case 0x9a:
	case 0xa2:
	case 0xa6:
	case 0xaa:
	case 0xc2:
	case 0xc6:
	case 0xca:
	case 0xd2:
	case 0xd6:
	case 0xda:
	case 0xe2:
	case 0xe6:
	case 0xea:
		return true; // ILLEGAL(); break; // defined as illegal on SH4

	case 0x0c:
	case 0x1c:
	case 0x2c:
	case 0x3c:
	case 0x4c:
	case 0x5c:
	case 0x6c:
	case 0x7c:
	case 0x8c:
	case 0x9c:
	case 0xac:
	case 0xbc:
	case 0xcc:
	case 0xdc:
	case 0xec:
	case 0xfc:
		return generate_group_4_SHAD(block, compiler, desc, opcode, in_delay_slot, ovrpc); break;

	case 0x0d:
	case 0x1d:
	case 0x2d:
	case 0x3d:
	case 0x4d:
	case 0x5d:
	case 0x6d:
	case 0x7d:
	case 0x8d:
	case 0x9d:
	case 0xad:
	case 0xbd:
	case 0xcd:
	case 0xdd:
	case 0xed:
	case 0xfd:
		return generate_group_4_SHLD(block, compiler, desc, opcode, in_delay_slot, ovrpc); break;

	case 0x8e:
	case 0x9e:
	case 0xae:
	case 0xbe:
	case 0xce:
	case 0xde:
	case 0xee:
	case 0xfe:
		return generate_group_4_LDCRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh3/4 only

	case 0x83:
	case 0x93:
	case 0xa3:
	case 0xb3:
	case 0xc3:
	case 0xd3:
	case 0xe3:
	case 0xf3:
		return generate_group_4_STCMRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh3/4 only

	case 0x87:
	case 0x97:
	case 0xa7:
	case 0xb7:
	case 0xc7:
	case 0xd7:
	case 0xe7:
	case 0xf7:
		return generate_group_4_LDCMRBANK(block, compiler, desc, opcode, in_delay_slot, ovrpc); // sh4 only

	case 0x32:  return generate_group_4_STCMSGR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x33:  return generate_group_4_STCMSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x37:  return generate_group_4_LDCMSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x3e:  return generate_group_4_LDCSSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x43:  return generate_group_4_STCMSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x47:  return generate_group_4_LDCMSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x4e:  return generate_group_4_LDCSPC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x52:  return generate_group_4_STSMFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x56:  return generate_group_4_LDSMFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x5a:  return generate_group_4_LDSFPUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x62:  return generate_group_4_STSMFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x66:  return generate_group_4_LDSMFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x6a:  return generate_group_4_LDSFPSCR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0xf2:  return generate_group_4_STCMDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0xf6:  return generate_group_4_LDCMDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0xfa:  return generate_group_4_LDCDBR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	}
	return false;
}


void sh34_base_device::func_LDCRBANK() { LDCRBANK(m_sh2_state->arg0); }
static void cfunc_LDCRBANK(void *param) { ((sh34_base_device *)param)->func_LDCRBANK(); };

bool sh34_base_device::generate_group_4_LDCRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCRBANK, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMRBANK() { STCMRBANK(m_sh2_state->arg0); }
static void cfunc_STCMRBANK(void *param) { ((sh34_base_device *)param)->func_STCMRBANK(); };

bool sh34_base_device::generate_group_4_STCMRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMRBANK, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMRBANK() { LDCMRBANK(m_sh2_state->arg0); }
static void cfunc_LDCMRBANK(void *param) { ((sh34_base_device *)param)->func_LDCMRBANK(); };

bool sh34_base_device::generate_group_4_LDCMRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMRBANK, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMSGR() { STCMSGR(m_sh2_state->arg0); }
static void cfunc_STCMSGR(void *param) { ((sh34_base_device *)param)->func_STCMSGR(); };

bool sh34_base_device::generate_group_4_STCMSGR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMSGR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMSSR() { STCMSSR(m_sh2_state->arg0); }
static void cfunc_STCMSSR(void *param) { ((sh34_base_device *)param)->func_STCMSSR(); };

bool sh34_base_device::generate_group_4_STCMSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMSSR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMSSR() { LDCMSSR(m_sh2_state->arg0); }
static void cfunc_LDCMSSR(void *param) { ((sh34_base_device *)param)->func_LDCMSSR(); };

bool sh34_base_device::generate_group_4_LDCMSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMSSR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4_LDCSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, mem(&m_sh2_state->m_ssr), R32(REG_N));
	return true;
}

void sh34_base_device::func_STCMSPC() { STCMSPC(m_sh2_state->arg0); }
static void cfunc_STCMSPC(void *param) { ((sh34_base_device *)param)->func_STCMSPC(); };

bool sh34_base_device::generate_group_4_STCMSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMSPC, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMSPC() { LDCMSPC(m_sh2_state->arg0); }
static void cfunc_LDCMSPC(void *param) { ((sh34_base_device *)param)->func_LDCMSPC(); };

bool sh34_base_device::generate_group_4_LDCMSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMSPC, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4_LDCSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, mem(&m_sh2_state->m_spc), R32(REG_N));
	return true;
}

void sh34_base_device::func_STSMFPUL() { STSMFPUL(m_sh2_state->arg0); }
static void cfunc_STSMFPUL(void *param) { ((sh34_base_device *)param)->func_STSMFPUL(); };

bool sh34_base_device::generate_group_4_STSMFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STSMFPUL, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDSMFPUL() { LDSMFPUL(m_sh2_state->arg0); }
static void cfunc_LDSMFPUL(void *param) { ((sh34_base_device *)param)->func_LDSMFPUL(); };

bool sh34_base_device::generate_group_4_LDSMFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDSMFPUL, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_4_LDSFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, mem(&m_sh2_state->m_fpul), R32(REG_N));
	return true;
}

void sh34_base_device::func_STSMFPSCR() { STSMFPSCR(m_sh2_state->arg0); }
static void cfunc_STSMFPSCR(void *param) { ((sh34_base_device *)param)->func_STSMFPSCR(); };

bool sh34_base_device::generate_group_4_STSMFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STSMFPSCR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDSMFPSCR() { LDSMFPSCR(m_sh2_state->arg0); }
static void cfunc_LDSMFPSCR(void *param) { ((sh34_base_device *)param)->func_LDSMFPSCR(); };


bool sh34_base_device::generate_group_4_LDSMFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDSMFPSCR, this);
	load_fast_iregs(block);
	return true;
}


void sh34_base_device::func_LDSFPSCR() { LDSFPSCR(m_sh2_state->arg0); }
static void cfunc_LDSFPSCR(void *param) { ((sh34_base_device *)param)->func_LDSFPSCR(); };

bool sh34_base_device::generate_group_4_LDSFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDSFPSCR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_STCMDBR() { STCMDBR(m_sh2_state->arg0); }
static void cfunc_STCMDBR(void *param) { ((sh34_base_device *)param)->func_STCMDBR(); };

bool sh34_base_device::generate_group_4_STCMDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_STCMDBR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCMDBR() { LDCMDBR(m_sh2_state->arg0); }
static void cfunc_LDCMDBR(void *param) { ((sh34_base_device *)param)->func_LDCMDBR(); };

bool sh34_base_device::generate_group_4_LDCMDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCMDBR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_LDCDBR() { LDCDBR(m_sh2_state->arg0); }
static void cfunc_LDCDBR(void *param) { ((sh34_base_device *)param)->func_LDCDBR(); };

bool sh34_base_device::generate_group_4_LDCDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_LDCDBR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0x0f)
	{
	case 0x00:  return generate_group_15_FADD(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x01:  return generate_group_15_FSUB(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x02:  return generate_group_15_FMUL(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x03:  return generate_group_15_FDIV(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x04:  return generate_group_15_FCMP_EQ(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x05:  return generate_group_15_FCMP_GT(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x06:  return generate_group_15_FMOVS0FR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x07:  return generate_group_15_FMOVFRS0(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x08:  return generate_group_15_FMOVMRFR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x09:  return generate_group_15_FMOVMRIFR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0a:  return generate_group_15_FMOVFRMR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0b:  return generate_group_15_FMOVFRMDR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0c:  return generate_group_15_FMOVFR(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0d:  return generate_group_15_op1111_0x13(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0e:  return generate_group_15_FMAC(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	case 0x0f:
		return true;
		//if (opcode == 0xffff) return true;    // atomiswave uses ffff as NOP?
		//return false; // dbreak(opcode); break;
	}
	return false;
}
bool sh34_base_device::generate_group_15_FADD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDADD(block, FPD32(REG_N), FPD32(REG_N), FPD32(REG_M));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSADD(block, FPS32(REG_N), FPS32(REG_N), FPS32(REG_M));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FSUB(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDSUB(block, FPD32(REG_N), FPD32(REG_N), FPD32(REG_M));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSSUB(block, FPS32(REG_N), FPS32(REG_N), FPS32(REG_M));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FMUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDMUL(block, FPD32(REG_N), FPD32(REG_N), FPD32(REG_M));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSMUL(block, FPS32(REG_N), FPS32(REG_N), FPS32(REG_M));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FDIV(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDDIV(block, FPD32(REG_N), FPD32(REG_N), FPD32(REG_M));
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:
	UML_FSDIV(block, FPS32(REG_N), FPS32(REG_N), FPS32(REG_M));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_FCMP_EQ(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDCMP(block, FPD32(REG_M & 14), FPD32(REG_N & 14));
	UML_SETc(block, COND_Z, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSCMP(block, FPS32(REG_M), FPS32(REG_N));
	UML_SETc(block, COND_Z, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

bool sh34_base_device::generate_group_15_FCMP_GT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDCMP(block, FPD32(REG_M & 14), FPD32(REG_N & 14));
	UML_SETc(block, COND_C, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSCMP(block, FPS32(REG_M), FPS32(REG_N));
	UML_SETc(block, COND_C, I0);
	UML_ROLINS(block, uml::mem(&m_sh2_state->sr), I0, T_SHIFT, SH_T);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

void sh34_base_device::func_FMOVS0FR() { FMOVS0FR(m_sh2_state->arg0); }
static void cfunc_FMOVS0FR(void *param) { ((sh34_base_device *)param)->func_FMOVS0FR(); };

bool sh34_base_device::generate_group_15_FMOVS0FR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVS0FR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFRS0() { FMOVFRS0(m_sh2_state->arg0); }
static void cfunc_FMOVFRS0(void *param) { ((sh34_base_device *)param)->func_FMOVFRS0(); };

bool sh34_base_device::generate_group_15_FMOVFRS0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFRS0, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVMRFR() { FMOVMRFR(m_sh2_state->arg0); }
static void cfunc_FMOVMRFR(void *param) { ((sh34_base_device *)param)->func_FMOVMRFR(); };

bool sh34_base_device::generate_group_15_FMOVMRFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVMRFR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVMRIFR() { FMOVMRIFR(m_sh2_state->arg0); }
static void cfunc_FMOVMRIFR(void *param) { ((sh34_base_device *)param)->func_FMOVMRIFR(); };

bool sh34_base_device::generate_group_15_FMOVMRIFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVMRIFR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFRMR() { FMOVFRMR(m_sh2_state->arg0); }
static void cfunc_FMOVFRMR(void *param) { ((sh34_base_device *)param)->func_FMOVFRMR(); };

bool sh34_base_device::generate_group_15_FMOVFRMR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFRMR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFRMDR() { FMOVFRMDR(m_sh2_state->arg0); }
static void cfunc_FMOVFRMDR(void *param) { ((sh34_base_device *)param)->func_FMOVFRMDR(); };

bool sh34_base_device::generate_group_15_FMOVFRMDR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFRMDR, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FMOVFR() { FMOVFR(m_sh2_state->arg0); }
static void cfunc_FMOVFR(void *param) { ((sh34_base_device *)param)->func_FMOVFR(); };

bool sh34_base_device::generate_group_15_FMOVFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FMOVFR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_FMAC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_NZ, compiler.labelnum);

	UML_FSMUL(block, F0, FPS32(0), FPS32(REG_M));
	UML_FSADD(block, FPS32(REG_N), F0, FPS32(REG_N));

	UML_LABEL(block, compiler.labelnum++);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch ((opcode >> 4) & 0x0f)
	{
	case 0x00:  return generate_group_15_op1111_0x13_FSTS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSTS(opcode); break;
	case 0x01:  return generate_group_15_op1111_0x13_FLDS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLDS(opcode); break;
	case 0x02:  return generate_group_15_op1111_0x13_FLOAT(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLOAT(opcode); break;
	case 0x03:  return generate_group_15_op1111_0x13_FTRC(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FTRC(opcode); break;
	case 0x04:  return generate_group_15_op1111_0x13_FNEG(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FNEG(opcode); break;
	case 0x05:  return generate_group_15_op1111_0x13_FABS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FABS(opcode); break;
	case 0x06:  return generate_group_15_op1111_0x13_FSQRT(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSQRT(opcode); break;
	case 0x07:  return generate_group_15_op1111_0x13_FSRRA(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSRRA(opcode); break;
	case 0x08:  return generate_group_15_op1111_0x13_FLDI0(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLDI0(opcode); break;
	case 0x09:  return generate_group_15_op1111_0x13_FLDI1(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FLDI1(opcode); break;
	case 0x0a:  return generate_group_15_op1111_0x13_FCNVSD(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FCNVSD(opcode); break;
	case 0x0b:  return generate_group_15_op1111_0x13_FCNVDS(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FCNVDS(opcode); break;
	case 0x0c:  return false; // dbreak(opcode); break;
	case 0x0d:  return false; //dbreak(opcode); break;
	case 0x0e:  return generate_group_15_op1111_0x13_FIPR(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FIPR(opcode); break;
	case 0x0f:  return generate_group_15_op1111_0x13_op1111_0xf13(block, compiler, desc, opcode, in_delay_slot, ovrpc); // op1111_0xf13(opcode); break;
	}
	return false;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FSTS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
#ifdef LSB_FIRST
	UML_XOR(block, I0, REG_N, uml::mem(&m_sh2_state->m_fpu_pr));
	UML_STORE(block, m_sh2_state->m_fr, I0, uml::mem(&m_sh2_state->m_fpul), SIZE_DWORD, SCALE_x4);
#else
	UML_MOV(block, FPS32(REG_N), uml::mem(&m_sh2_state->m_fpul));
#endif
	return true;
}

void sh34_base_device::func_FLDS() { FLDS(m_sh2_state->arg0); }
static void cfunc_FLDS(void *param) { ((sh34_base_device *)param)->func_FLDS(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FLDS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FLDS, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FLOAT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDFRINT(block, FPD32(REG_N & 14), uml::mem(&m_sh2_state->m_fpul), SIZE_DWORD);
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSFRINT(block, FPS32(REG_N), uml::mem(&m_sh2_state->m_fpul), SIZE_DWORD);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:

	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FTRC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, uml::mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDTOINT(block, uml::mem(&m_sh2_state->m_fpul), FPD32(REG_N & 14), SIZE_DWORD, ROUND_TRUNC);
	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSTOINT(block, uml::mem(&m_sh2_state->m_fpul), FPS32(REG_N), SIZE_DWORD, ROUND_TRUNC);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FNEG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, I0, 0);

	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

	UML_FDFRINT(block, F1, I0, SIZE_DWORD);
	UML_FDSUB(block, FPD32(REG_N), F1, FPD32(REG_N));

	UML_JMP(block, compiler.labelnum+1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_FSFRINT(block, F1, I0, SIZE_DWORD);
	UML_FSSUB(block, FPS32(REG_N), F1, FPS32(REG_N));

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FABS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_TEST(block, mem(&m_sh2_state->m_fpu_pr), 0);
	UML_JMPc(block, COND_Z, compiler.labelnum);

#ifdef LSB_FIRST
	UML_AND(block, FPS32(((REG_N & 14) | 1)), FPS32(((REG_N & 14) | 1)), 0x7fffffff);
#else
	UML_AND(block, FPS32(REG_N & 14), FPS32(REG_N & 14), 0x7fffffff);
#endif

	UML_JMP(block, compiler.labelnum + 1);

	UML_LABEL(block, compiler.labelnum++);  // labelnum:

	UML_AND(block, FPS32(REG_N), FPS32(REG_N), 0x7fffffff);

	UML_LABEL(block, compiler.labelnum++);  // labelnum+1:
	return true;
}

void sh34_base_device::func_FSQRT() { FSQRT(m_sh2_state->arg0); }
static void cfunc_FSQRT(void *param) { ((sh34_base_device *)param)->func_FSQRT(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FSQRT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FSQRT, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FSRRA() { FSRRA(m_sh2_state->arg0); }
static void cfunc_FSRRA(void *param) { ((sh34_base_device *)param)->func_FSRRA(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FSRRA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FSRRA, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FLDI0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
#ifdef LSB_FIRST
	UML_MOV(block, I0, REG_N);
	UML_XOR(block, I0, I0, uml::mem(&m_sh2_state->m_fpu_pr));
	UML_STORE(block, m_sh2_state->m_fr, I0, 0, SIZE_DWORD, SCALE_x4);
#else
	UML_MOV(block, FP_RFS(REG_N), 0);
#endif
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_FLDI1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
#ifdef LSB_FIRST
	UML_MOV(block, I0, REG_N);
	UML_XOR(block, I0, I0, uml::mem(&m_sh2_state->m_fpu_pr));
	UML_STORE(block, m_sh2_state->m_fr, I0, 0x3F800000, SIZE_DWORD, SCALE_x4);
#else
	UML_MOV(block, FP_RFS(REG_N), 0x3F800000);
#endif
	return true;
}

void sh34_base_device::func_FCNVSD() { FCNVSD(m_sh2_state->arg0); }
static void cfunc_FCNVSD(void *param) { ((sh34_base_device *)param)->func_FCNVSD(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FCNVSD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FCNVSD, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FCNVDS() { FCNVDS(m_sh2_state->arg0); }
static void cfunc_FCNVDS(void *param) { ((sh34_base_device *)param)->func_FCNVDS(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FCNVDS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FCNVDS, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FIPR() { FIPR(m_sh2_state->arg0); }
static void cfunc_FIPR(void *param) { ((sh34_base_device *)param)->func_FIPR(); };

bool sh34_base_device::generate_group_15_op1111_0x13_FIPR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FIPR, this);
	load_fast_iregs(block);
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	if (opcode & 0x100)
	{
		if (opcode & 0x200)
		{
			switch (opcode & 0xC00)
			{
			case 0x000:
				return generate_group_15_op1111_0x13_op1111_0xf13_FSCHG(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSCHG();
				break;
			case 0x800:
				return generate_group_15_op1111_0x13_op1111_0xf13_FRCHG(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FRCHG();
				break;
			default:
				return false; //machine().debug_break();
				break;
			}
		}
		else
		{
			return generate_group_15_op1111_0x13_op1111_0xf13_FTRV(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FTRV(opcode);
		}
	}
	else
	{
		return generate_group_15_op1111_0x13_op1111_0xf13_FSSCA(block, compiler, desc, opcode, in_delay_slot, ovrpc); // FSSCA(opcode);
	}
	return false;
}

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FSCHG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_XOR(block, I0, uml::mem(&m_sh2_state->m_fpscr), SZ);
	UML_MOV(block, uml::mem(&m_sh2_state->m_fpscr), I0);
	UML_TEST(block, I0, SZ);
	UML_SETc(block, COND_NZ, uml::mem(&m_sh2_state->m_fpu_sz));
	return true;
}

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FRCHG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_XOR(block, uml::mem(&m_sh2_state->m_fpscr), uml::mem(&m_sh2_state->m_fpscr), FR);

	UML_MOV(block, I0, 0);
	UML_LABEL(block, compiler.labelnum);  // labelnum:

	UML_LOAD(block, I1, m_sh2_state->m_fr, I0, SIZE_DWORD, SCALE_x4);
	UML_LOAD(block, I2, m_sh2_state->m_xf, I0, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, m_sh2_state->m_xf, I0, I1, SIZE_DWORD, SCALE_x4);
	UML_STORE(block, m_sh2_state->m_fr, I0, I2, SIZE_DWORD, SCALE_x4);
	UML_ADD(block, I0, I0, 1);
	UML_CMP(block, I0, 16);
	UML_JMPc(block, COND_NZ, compiler.labelnum);

	compiler.labelnum++;
	return true;
}

void sh34_base_device::func_FTRV() { FTRV(m_sh2_state->arg0); }
static void cfunc_FTRV(void *param) { ((sh34_base_device *)param)->func_FTRV(); };

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FTRV(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FTRV, this);
	load_fast_iregs(block);
	return true;
}

void sh34_base_device::func_FSSCA() { FSSCA(m_sh2_state->arg0); }
static void cfunc_FSSCA(void *param) { ((sh34_base_device *)param)->func_FSSCA(); };

bool sh34_base_device::generate_group_15_op1111_0x13_op1111_0xf13_FSSCA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	save_fast_iregs(block);
	UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
	UML_CALLC(block, cfunc_FSSCA, this);
	load_fast_iregs(block);
	return true;
}

