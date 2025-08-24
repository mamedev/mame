// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************************************************************
 *
 * Intel XScale PXA255 peripheral emulation
 *
 * TODO:
 *   Most things
 *
 **************************************************************************/

#include "emu.h"
#include "pxa255.h"

#include "screen.h"
#include "speaker.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_I2S         (1U << 2)
#define LOG_DMA         (1U << 3)
#define LOG_OSTIMER     (1U << 4)
#define LOG_INTC        (1U << 5)
#define LOG_GPIO        (1U << 6)
#define LOG_LCD_DMA     (1U << 7)
#define LOG_LCD         (1U << 8)
#define LOG_POWER       (1U << 9)
#define LOG_RTC         (1U << 10)
#define LOG_CLOCKS      (1U << 11)
#define LOG_ALL         (LOG_UNKNOWN | LOG_I2S | LOG_DMA | LOG_OSTIMER | LOG_INTC | LOG_GPIO | LOG_LCD_DMA | LOG_LCD | LOG_POWER | LOG_RTC | LOG_CLOCKS)

#define VERBOSE         (LOG_GPIO)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PXA255_PERIPHERALS, pxa255_periphs_device, "pxa255_periphs", "Intel XScale PXA255 Peripherals")

pxa255_periphs_device::pxa255_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PXA255_PERIPHERALS, tag, owner, clock)
	, m_gpio_w(*this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_dmadac(*this, "dac%u", 1U)
	, m_palette(*this, "palette")
	, m_screen(*this, "screen")
{
}

void pxa255_periphs_device::map(address_map &map)
{
	map(0x00000000, 0x0000003f).rw(FUNC(pxa255_periphs_device::dma_dcsr_r), FUNC(pxa255_periphs_device::dma_dcsr_w));
	map(0x000000f0, 0x000000f3).rw(FUNC(pxa255_periphs_device::dma_dint_r), FUNC(pxa255_periphs_device::dma_dint_w));
	map(0x00000100, 0x0000019f).rw(FUNC(pxa255_periphs_device::dma_drcmr_r), FUNC(pxa255_periphs_device::dma_drcmr_w));
	map(0x00000200, 0x00000203).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<0>), FUNC(pxa255_periphs_device::dma_ddadr_w<0>));
	map(0x00000204, 0x00000207).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<0>), FUNC(pxa255_periphs_device::dma_dsadr_w<0>));
	map(0x00000208, 0x0000020b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<0>), FUNC(pxa255_periphs_device::dma_dtadr_w<0>));
	map(0x0000020c, 0x0000020f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<0>), FUNC(pxa255_periphs_device::dma_dcmd_w<0>));
	map(0x00000210, 0x00000213).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<1>), FUNC(pxa255_periphs_device::dma_ddadr_w<1>));
	map(0x00000214, 0x00000217).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<1>), FUNC(pxa255_periphs_device::dma_dsadr_w<1>));
	map(0x00000218, 0x0000021b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<1>), FUNC(pxa255_periphs_device::dma_dtadr_w<1>));
	map(0x0000021c, 0x0000021f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<1>), FUNC(pxa255_periphs_device::dma_dcmd_w<1>));
	map(0x00000220, 0x00000223).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<2>), FUNC(pxa255_periphs_device::dma_ddadr_w<2>));
	map(0x00000224, 0x00000227).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<2>), FUNC(pxa255_periphs_device::dma_dsadr_w<2>));
	map(0x00000228, 0x0000022b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<2>), FUNC(pxa255_periphs_device::dma_dtadr_w<2>));
	map(0x0000022c, 0x0000022f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<2>), FUNC(pxa255_periphs_device::dma_dcmd_w<2>));
	map(0x00000230, 0x00000233).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<3>), FUNC(pxa255_periphs_device::dma_ddadr_w<3>));
	map(0x00000234, 0x00000237).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<3>), FUNC(pxa255_periphs_device::dma_dsadr_w<3>));
	map(0x00000238, 0x0000023b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<3>), FUNC(pxa255_periphs_device::dma_dtadr_w<3>));
	map(0x0000023c, 0x0000023f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<3>), FUNC(pxa255_periphs_device::dma_dcmd_w<3>));
	map(0x00000240, 0x00000243).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<4>), FUNC(pxa255_periphs_device::dma_ddadr_w<4>));
	map(0x00000244, 0x00000247).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<4>), FUNC(pxa255_periphs_device::dma_dsadr_w<4>));
	map(0x00000248, 0x0000024b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<4>), FUNC(pxa255_periphs_device::dma_dtadr_w<4>));
	map(0x0000024c, 0x0000024f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<4>), FUNC(pxa255_periphs_device::dma_dcmd_w<4>));
	map(0x00000250, 0x00000253).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<5>), FUNC(pxa255_periphs_device::dma_ddadr_w<5>));
	map(0x00000254, 0x00000257).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<5>), FUNC(pxa255_periphs_device::dma_dsadr_w<5>));
	map(0x00000258, 0x0000025b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<5>), FUNC(pxa255_periphs_device::dma_dtadr_w<5>));
	map(0x0000025c, 0x0000025f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<5>), FUNC(pxa255_periphs_device::dma_dcmd_w<5>));
	map(0x00000260, 0x00000263).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<6>), FUNC(pxa255_periphs_device::dma_ddadr_w<6>));
	map(0x00000264, 0x00000267).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<6>), FUNC(pxa255_periphs_device::dma_dsadr_w<6>));
	map(0x00000268, 0x0000026b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<6>), FUNC(pxa255_periphs_device::dma_dtadr_w<6>));
	map(0x0000026c, 0x0000026f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<6>), FUNC(pxa255_periphs_device::dma_dcmd_w<6>));
	map(0x00000270, 0x00000273).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<7>), FUNC(pxa255_periphs_device::dma_ddadr_w<7>));
	map(0x00000274, 0x00000277).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<7>), FUNC(pxa255_periphs_device::dma_dsadr_w<7>));
	map(0x00000278, 0x0000027b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<7>), FUNC(pxa255_periphs_device::dma_dtadr_w<7>));
	map(0x0000027c, 0x0000027f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<7>), FUNC(pxa255_periphs_device::dma_dcmd_w<7>));
	map(0x00000280, 0x00000283).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<8>), FUNC(pxa255_periphs_device::dma_ddadr_w<8>));
	map(0x00000284, 0x00000287).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<8>), FUNC(pxa255_periphs_device::dma_dsadr_w<8>));
	map(0x00000288, 0x0000028b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<8>), FUNC(pxa255_periphs_device::dma_dtadr_w<8>));
	map(0x0000028c, 0x0000028f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<8>), FUNC(pxa255_periphs_device::dma_dcmd_w<8>));
	map(0x00000290, 0x00000293).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<9>), FUNC(pxa255_periphs_device::dma_ddadr_w<9>));
	map(0x00000294, 0x00000297).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<9>), FUNC(pxa255_periphs_device::dma_dsadr_w<9>));
	map(0x00000298, 0x0000029b).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<9>), FUNC(pxa255_periphs_device::dma_dtadr_w<9>));
	map(0x0000029c, 0x0000029f).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<9>), FUNC(pxa255_periphs_device::dma_dcmd_w<9>));
	map(0x000002a0, 0x000002a3).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<10>), FUNC(pxa255_periphs_device::dma_ddadr_w<10>));
	map(0x000002a4, 0x000002a7).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<10>), FUNC(pxa255_periphs_device::dma_dsadr_w<10>));
	map(0x000002a8, 0x000002ab).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<10>), FUNC(pxa255_periphs_device::dma_dtadr_w<10>));
	map(0x000002ac, 0x000002af).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<10>), FUNC(pxa255_periphs_device::dma_dcmd_w<10>));
	map(0x000002b0, 0x000002b3).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<11>), FUNC(pxa255_periphs_device::dma_ddadr_w<11>));
	map(0x000002b4, 0x000002b7).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<11>), FUNC(pxa255_periphs_device::dma_dsadr_w<11>));
	map(0x000002b8, 0x000002bb).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<11>), FUNC(pxa255_periphs_device::dma_dtadr_w<11>));
	map(0x000002bc, 0x000002bf).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<11>), FUNC(pxa255_periphs_device::dma_dcmd_w<11>));
	map(0x000002c0, 0x000002c3).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<12>), FUNC(pxa255_periphs_device::dma_ddadr_w<12>));
	map(0x000002c4, 0x000002c7).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<12>), FUNC(pxa255_periphs_device::dma_dsadr_w<12>));
	map(0x000002c8, 0x000002cb).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<12>), FUNC(pxa255_periphs_device::dma_dtadr_w<12>));
	map(0x000002cc, 0x000002cf).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<12>), FUNC(pxa255_periphs_device::dma_dcmd_w<12>));
	map(0x000002d0, 0x000002d3).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<13>), FUNC(pxa255_periphs_device::dma_ddadr_w<13>));
	map(0x000002d4, 0x000002d7).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<13>), FUNC(pxa255_periphs_device::dma_dsadr_w<13>));
	map(0x000002d8, 0x000002db).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<13>), FUNC(pxa255_periphs_device::dma_dtadr_w<13>));
	map(0x000002dc, 0x000002df).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<13>), FUNC(pxa255_periphs_device::dma_dcmd_w<13>));
	map(0x000002e0, 0x000002e3).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<14>), FUNC(pxa255_periphs_device::dma_ddadr_w<14>));
	map(0x000002e4, 0x000002e7).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<14>), FUNC(pxa255_periphs_device::dma_dsadr_w<14>));
	map(0x000002e8, 0x000002eb).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<14>), FUNC(pxa255_periphs_device::dma_dtadr_w<14>));
	map(0x000002ec, 0x000002ef).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<14>), FUNC(pxa255_periphs_device::dma_dcmd_w<14>));
	map(0x000002f0, 0x000002f3).rw(FUNC(pxa255_periphs_device::dma_ddadr_r<15>), FUNC(pxa255_periphs_device::dma_ddadr_w<15>));
	map(0x000002f4, 0x000002f7).rw(FUNC(pxa255_periphs_device::dma_dsadr_r<15>), FUNC(pxa255_periphs_device::dma_dsadr_w<15>));
	map(0x000002f8, 0x000002fb).rw(FUNC(pxa255_periphs_device::dma_dtadr_r<15>), FUNC(pxa255_periphs_device::dma_dtadr_w<15>));
	map(0x000002fc, 0x000002ff).rw(FUNC(pxa255_periphs_device::dma_dcmd_r<15>), FUNC(pxa255_periphs_device::dma_dcmd_w<15>));
	map(0x00900000, 0x00900003).rw(FUNC(pxa255_periphs_device::rtc_rcnr_r), FUNC(pxa255_periphs_device::rtc_rcnr_w));
	map(0x00900004, 0x00900007).rw(FUNC(pxa255_periphs_device::rtc_rtar_r), FUNC(pxa255_periphs_device::rtc_rtar_w));
	map(0x00900008, 0x0090000b).rw(FUNC(pxa255_periphs_device::rtc_rtsr_r), FUNC(pxa255_periphs_device::rtc_rtsr_w));
	map(0x0090000c, 0x0090000f).rw(FUNC(pxa255_periphs_device::rtc_rttr_r), FUNC(pxa255_periphs_device::rtc_rttr_w));
	map(0x00400000, 0x00400003).rw(FUNC(pxa255_periphs_device::i2s_sacr0_r), FUNC(pxa255_periphs_device::i2s_sacr0_w));
	map(0x00400004, 0x00400007).rw(FUNC(pxa255_periphs_device::i2s_sacr1_r), FUNC(pxa255_periphs_device::i2s_sacr1_w));
	map(0x0040000c, 0x0040000f).rw(FUNC(pxa255_periphs_device::i2s_sasr0_r), FUNC(pxa255_periphs_device::i2s_sasr0_w));
	map(0x00400014, 0x00400017).rw(FUNC(pxa255_periphs_device::i2s_saimr_r), FUNC(pxa255_periphs_device::i2s_saimr_w));
	map(0x00400018, 0x0040001b).rw(FUNC(pxa255_periphs_device::i2s_saicr_r), FUNC(pxa255_periphs_device::i2s_saicr_w));
	map(0x00400060, 0x00400063).rw(FUNC(pxa255_periphs_device::i2s_sadiv_r), FUNC(pxa255_periphs_device::i2s_sadiv_w));
	map(0x00400080, 0x00400083).rw(FUNC(pxa255_periphs_device::i2s_sadr_r), FUNC(pxa255_periphs_device::i2s_sadr_w));
	map(0x00a00000, 0x00a00003).rw(FUNC(pxa255_periphs_device::tmr_osmr_r<0>), FUNC(pxa255_periphs_device::tmr_osmr_w<0>));
	map(0x00a00004, 0x00a00007).rw(FUNC(pxa255_periphs_device::tmr_osmr_r<1>), FUNC(pxa255_periphs_device::tmr_osmr_w<1>));
	map(0x00a00008, 0x00a0000b).rw(FUNC(pxa255_periphs_device::tmr_osmr_r<2>), FUNC(pxa255_periphs_device::tmr_osmr_w<2>));
	map(0x00a0000c, 0x00a0000f).rw(FUNC(pxa255_periphs_device::tmr_osmr_r<3>), FUNC(pxa255_periphs_device::tmr_osmr_w<3>));
	map(0x00a00010, 0x00a00013).rw(FUNC(pxa255_periphs_device::tmr_oscr_r), FUNC(pxa255_periphs_device::tmr_oscr_w));
	map(0x00a00014, 0x00a00017).rw(FUNC(pxa255_periphs_device::tmr_ossr_r), FUNC(pxa255_periphs_device::tmr_ossr_w));
	map(0x00a00018, 0x00a0001b).rw(FUNC(pxa255_periphs_device::tmr_ower_r), FUNC(pxa255_periphs_device::tmr_ower_w));
	map(0x00a0001c, 0x00a0001f).rw(FUNC(pxa255_periphs_device::tmr_oier_r), FUNC(pxa255_periphs_device::tmr_oier_w));
	map(0x00d00000, 0x00d00003).rw(FUNC(pxa255_periphs_device::intc_icip_r), FUNC(pxa255_periphs_device::intc_icip_w));
	map(0x00d00004, 0x00d00007).rw(FUNC(pxa255_periphs_device::intc_icmr_r), FUNC(pxa255_periphs_device::intc_icmr_w));
	map(0x00d00008, 0x00d0000b).rw(FUNC(pxa255_periphs_device::intc_iclr_r), FUNC(pxa255_periphs_device::intc_iclr_w));
	map(0x00d0000c, 0x00d0000f).rw(FUNC(pxa255_periphs_device::intc_icfp_r), FUNC(pxa255_periphs_device::intc_icfp_w));
	map(0x00d00010, 0x00d00013).rw(FUNC(pxa255_periphs_device::intc_icpr_r), FUNC(pxa255_periphs_device::intc_icpr_w));
	map(0x00d00014, 0x00d00017).rw(FUNC(pxa255_periphs_device::intc_iccr_r), FUNC(pxa255_periphs_device::intc_iccr_w));
	map(0x00e00000, 0x00e00003).rw(FUNC(pxa255_periphs_device::gpio_gplr_r<0>), FUNC(pxa255_periphs_device::gpio_gplr_w<0>));
	map(0x00e00004, 0x00e00007).rw(FUNC(pxa255_periphs_device::gpio_gplr_r<1>), FUNC(pxa255_periphs_device::gpio_gplr_w<1>));
	map(0x00e00008, 0x00e0000b).rw(FUNC(pxa255_periphs_device::gpio_gplr_r<2>), FUNC(pxa255_periphs_device::gpio_gplr_w<2>));
	map(0x00e0000c, 0x00e0000f).rw(FUNC(pxa255_periphs_device::gpio_gpdr_r<0>), FUNC(pxa255_periphs_device::gpio_gpdr_w<0>));
	map(0x00e00010, 0x00e00013).rw(FUNC(pxa255_periphs_device::gpio_gpdr_r<1>), FUNC(pxa255_periphs_device::gpio_gpdr_w<1>));
	map(0x00e00014, 0x00e00017).rw(FUNC(pxa255_periphs_device::gpio_gpdr_r<2>), FUNC(pxa255_periphs_device::gpio_gpdr_w<2>));
	map(0x00e00018, 0x00e0001b).rw(FUNC(pxa255_periphs_device::gpio_gpsr_r<0>), FUNC(pxa255_periphs_device::gpio_gpsr_w<0>));
	map(0x00e0001c, 0x00e0001f).rw(FUNC(pxa255_periphs_device::gpio_gpsr_r<1>), FUNC(pxa255_periphs_device::gpio_gpsr_w<1>));
	map(0x00e00020, 0x00e00023).rw(FUNC(pxa255_periphs_device::gpio_gpsr_r<2>), FUNC(pxa255_periphs_device::gpio_gpsr_w<2>));
	map(0x00e00024, 0x00e00027).rw(FUNC(pxa255_periphs_device::gpio_gpcr_r<0>), FUNC(pxa255_periphs_device::gpio_gpcr_w<0>));
	map(0x00e00028, 0x00e0002b).rw(FUNC(pxa255_periphs_device::gpio_gpcr_r<1>), FUNC(pxa255_periphs_device::gpio_gpcr_w<1>));
	map(0x00e0002c, 0x00e0002f).rw(FUNC(pxa255_periphs_device::gpio_gpcr_r<2>), FUNC(pxa255_periphs_device::gpio_gpcr_w<2>));
	map(0x00e00030, 0x00e00033).rw(FUNC(pxa255_periphs_device::gpio_grer_r<0>), FUNC(pxa255_periphs_device::gpio_grer_w<0>));
	map(0x00e00034, 0x00e00037).rw(FUNC(pxa255_periphs_device::gpio_grer_r<1>), FUNC(pxa255_periphs_device::gpio_grer_w<1>));
	map(0x00e00038, 0x00e0003b).rw(FUNC(pxa255_periphs_device::gpio_grer_r<2>), FUNC(pxa255_periphs_device::gpio_grer_w<2>));
	map(0x00e0003c, 0x00e0003f).rw(FUNC(pxa255_periphs_device::gpio_gfer_r<0>), FUNC(pxa255_periphs_device::gpio_gfer_w<0>));
	map(0x00e00040, 0x00e00043).rw(FUNC(pxa255_periphs_device::gpio_gfer_r<1>), FUNC(pxa255_periphs_device::gpio_gfer_w<1>));
	map(0x00e00044, 0x00e00047).rw(FUNC(pxa255_periphs_device::gpio_gfer_r<2>), FUNC(pxa255_periphs_device::gpio_gfer_w<2>));
	map(0x00e00048, 0x00e0004b).rw(FUNC(pxa255_periphs_device::gpio_gedr_r<0>), FUNC(pxa255_periphs_device::gpio_gedr_w<0>));
	map(0x00e0004c, 0x00e0004f).rw(FUNC(pxa255_periphs_device::gpio_gedr_r<1>), FUNC(pxa255_periphs_device::gpio_gedr_w<1>));
	map(0x00e00050, 0x00e00053).rw(FUNC(pxa255_periphs_device::gpio_gedr_r<2>), FUNC(pxa255_periphs_device::gpio_gedr_w<2>));
	map(0x00e00054, 0x00e00057).rw(FUNC(pxa255_periphs_device::gpio_gafrl_r<0>), FUNC(pxa255_periphs_device::gpio_gafrl_w<0>));
	map(0x00e00058, 0x00e0005b).rw(FUNC(pxa255_periphs_device::gpio_gafru_r<0>), FUNC(pxa255_periphs_device::gpio_gafru_w<0>));
	map(0x00e0005c, 0x00e0005f).rw(FUNC(pxa255_periphs_device::gpio_gafrl_r<1>), FUNC(pxa255_periphs_device::gpio_gafrl_w<1>));
	map(0x00e00060, 0x00e00063).rw(FUNC(pxa255_periphs_device::gpio_gafru_r<1>), FUNC(pxa255_periphs_device::gpio_gafru_w<1>));
	map(0x00e00064, 0x00e00067).rw(FUNC(pxa255_periphs_device::gpio_gafrl_r<2>), FUNC(pxa255_periphs_device::gpio_gafrl_w<2>));
	map(0x00e00068, 0x00e0006b).rw(FUNC(pxa255_periphs_device::gpio_gafru_r<2>), FUNC(pxa255_periphs_device::gpio_gafru_w<2>));
	map(0x00f00000, 0x00f00003).rw(FUNC(pxa255_periphs_device::pwr_pmcr_r), FUNC(pxa255_periphs_device::pwr_pmcr_w));
	map(0x00f00004, 0x00f00007).rw(FUNC(pxa255_periphs_device::pwr_pssr_r), FUNC(pxa255_periphs_device::pwr_pssr_w));
	map(0x00f00008, 0x00f0000b).rw(FUNC(pxa255_periphs_device::pwr_pspr_r), FUNC(pxa255_periphs_device::pwr_pspr_w));
	map(0x00f0000c, 0x00f0000f).rw(FUNC(pxa255_periphs_device::pwr_pwer_r), FUNC(pxa255_periphs_device::pwr_pwer_w));
	map(0x00f00010, 0x00f00013).rw(FUNC(pxa255_periphs_device::pwr_prer_r), FUNC(pxa255_periphs_device::pwr_prer_w));
	map(0x00f00014, 0x00f00017).rw(FUNC(pxa255_periphs_device::pwr_pfer_r), FUNC(pxa255_periphs_device::pwr_pfer_w));
	map(0x00f00018, 0x00f0001b).rw(FUNC(pxa255_periphs_device::pwr_pedr_r), FUNC(pxa255_periphs_device::pwr_pedr_w));
	map(0x00f0001c, 0x00f0001f).rw(FUNC(pxa255_periphs_device::pwr_pcfr_r), FUNC(pxa255_periphs_device::pwr_pcfr_w));
	map(0x00f00020, 0x00f00023).rw(FUNC(pxa255_periphs_device::pwr_pgsr_r<0>), FUNC(pxa255_periphs_device::pwr_pgsr_w<0>));
	map(0x00f00024, 0x00f00027).rw(FUNC(pxa255_periphs_device::pwr_pgsr_r<1>), FUNC(pxa255_periphs_device::pwr_pgsr_w<1>));
	map(0x00f00028, 0x00f0002b).rw(FUNC(pxa255_periphs_device::pwr_pgsr_r<2>), FUNC(pxa255_periphs_device::pwr_pgsr_w<2>));
	map(0x00f00030, 0x00f00033).r(FUNC(pxa255_periphs_device::pwr_rcsr_r));
	map(0x00f00034, 0x00f00037).rw(FUNC(pxa255_periphs_device::pwr_pmfw_r), FUNC(pxa255_periphs_device::pwr_pmfw_w));
	map(0x01300000, 0x01300003).rw(FUNC(pxa255_periphs_device::clk_cccr_r), FUNC(pxa255_periphs_device::clk_cccr_w));
	map(0x01300004, 0x01300007).rw(FUNC(pxa255_periphs_device::clk_cken_r), FUNC(pxa255_periphs_device::clk_cken_w));
	map(0x01300008, 0x0130000b).rw(FUNC(pxa255_periphs_device::clk_oscc_r), FUNC(pxa255_periphs_device::clk_oscc_w));
	map(0x04000000, 0x04000003).rw(FUNC(pxa255_periphs_device::lcd_lccr_r<0>), FUNC(pxa255_periphs_device::lcd_lccr_w<0>));
	map(0x04000004, 0x04000007).rw(FUNC(pxa255_periphs_device::lcd_lccr_r<1>), FUNC(pxa255_periphs_device::lcd_lccr_w<1>));
	map(0x04000008, 0x0400000b).rw(FUNC(pxa255_periphs_device::lcd_lccr_r<2>), FUNC(pxa255_periphs_device::lcd_lccr_w<2>));
	map(0x0400000c, 0x0400000f).rw(FUNC(pxa255_periphs_device::lcd_lccr_r<3>), FUNC(pxa255_periphs_device::lcd_lccr_w<3>));
	map(0x04000020, 0x04000023).rw(FUNC(pxa255_periphs_device::lcd_fbr_r<0>), FUNC(pxa255_periphs_device::lcd_fbr_w<0>));
	map(0x04000024, 0x04000027).rw(FUNC(pxa255_periphs_device::lcd_fbr_r<1>), FUNC(pxa255_periphs_device::lcd_fbr_w<1>));
	map(0x04000038, 0x0400003b).rw(FUNC(pxa255_periphs_device::lcd_lcsr_r), FUNC(pxa255_periphs_device::lcd_lcsr_w));
	map(0x0400003c, 0x0400003f).rw(FUNC(pxa255_periphs_device::lcd_liidr_r), FUNC(pxa255_periphs_device::lcd_liidr_w));
	map(0x04000040, 0x04000043).rw(FUNC(pxa255_periphs_device::lcd_trgbr_r), FUNC(pxa255_periphs_device::lcd_trgbr_w));
	map(0x04000044, 0x04000047).rw(FUNC(pxa255_periphs_device::lcd_tcr_r), FUNC(pxa255_periphs_device::lcd_tcr_w));
	map(0x04000200, 0x04000203).rw(FUNC(pxa255_periphs_device::lcd_fdadr_r<0>), FUNC(pxa255_periphs_device::lcd_fdadr_w<0>));
	map(0x04000204, 0x04000207).rw(FUNC(pxa255_periphs_device::lcd_fsadr_r<0>), FUNC(pxa255_periphs_device::lcd_fsadr_w<0>));
	map(0x04000208, 0x0400020b).rw(FUNC(pxa255_periphs_device::lcd_fidr_r<0>), FUNC(pxa255_periphs_device::lcd_fidr_w<0>));
	map(0x0400020c, 0x0400020f).rw(FUNC(pxa255_periphs_device::lcd_ldcmd_r<0>), FUNC(pxa255_periphs_device::lcd_ldcmd_w<0>));
	map(0x04000210, 0x04000213).rw(FUNC(pxa255_periphs_device::lcd_fdadr_r<1>), FUNC(pxa255_periphs_device::lcd_fdadr_w<1>));
	map(0x04000214, 0x04000217).rw(FUNC(pxa255_periphs_device::lcd_fsadr_r<1>), FUNC(pxa255_periphs_device::lcd_fsadr_w<1>));
	map(0x04000218, 0x0400021b).rw(FUNC(pxa255_periphs_device::lcd_fidr_r<1>), FUNC(pxa255_periphs_device::lcd_fidr_w<1>));
	map(0x0400021c, 0x0400021f).rw(FUNC(pxa255_periphs_device::lcd_ldcmd_r<1>), FUNC(pxa255_periphs_device::lcd_ldcmd_w<1>));
}

void pxa255_periphs_device::device_start()
{
	for (int index = 0; index < 16; index++)
	{
		if (index != 3)
		{
			m_dma_regs.timer[index] = timer_alloc(FUNC(pxa255_periphs_device::dma_end_tick), this);
		}
		else
		{
			m_dma_regs.timer[index] = timer_alloc(FUNC(pxa255_periphs_device::audio_dma_end_tick), this);
		}
	}

	for (int index = 0; index < 4; index++)
	{
		m_ostimer_regs.timer[index] = timer_alloc(FUNC(pxa255_periphs_device::ostimer_match_tick), this);
	}

	m_lcd_regs.dma[0].eof = timer_alloc(FUNC(pxa255_periphs_device::lcd_dma_eof_tick), this);
	m_lcd_regs.dma[1].eof = timer_alloc(FUNC(pxa255_periphs_device::lcd_dma_eof_tick), this);

	m_lcd_palette = make_unique_clear<u32[]>(0x100);
	m_lcd_framebuffer = make_unique_clear<u8[]>(0x100000);
	m_samples = make_unique_clear<s16[]>(0x1000);

	m_rtc_regs.timer = timer_alloc(FUNC(pxa255_periphs_device::rtc_tick), this);
}

void pxa255_periphs_device::device_reset()
{
	for (int index = 0; index < 16; index++)
	{
		m_dma_regs.dcsr[index] = 0x00000008;
	}

	m_rtc_regs.rcnr = 0x00000000;
	m_rtc_regs.rtar = 0x00000000;
	m_rtc_regs.rtsr = 0x00000000;
	m_rtc_regs.rttr = 0x00007fff;
	m_rtc_regs.timer->adjust(attotime::from_hz(1));

	memset(&m_intc_regs, 0, sizeof(m_intc_regs));

	m_lcd_regs.trgbr = 0x00aa5500;
	m_lcd_regs.tcr = 0x0000754f;

	memset(&m_gpio_regs, 0, sizeof(m_gpio_regs));
	memset(&m_power_regs, 0, sizeof(m_power_regs));
	memset(&m_clk_regs, 0, sizeof(m_clk_regs));
}

void pxa255_periphs_device::device_add_mconfig(machine_config &config)
{
	// TODO: should be SCREEN_TYPE_LCD, but that dislikes dynamic configure
	// will stay stuck at 296x480 aspect ratio.
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(1024, 1024);
	m_screen->set_visarea(0, 295, 0, 479);
	m_screen->set_screen_update(FUNC(pxa255_periphs_device::screen_update));

	PALETTE(config, m_palette).set_entries(256);

	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

/*

  PXA255 Inter-Integrated-Circuit Sound (I2S) Controller

  pg. 489 to 504, PXA255 Processor Developers Manual [278693-002].pdf

*/

u32 pxa255_periphs_device::i2s_sacr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_i2s_regs.sacr0;
	LOGMASKED(LOG_I2S, "%s: i2s_sacr0_r: Serial Audio Controller Global Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::i2s_sacr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_I2S, "%s: i2s_sacr0_r: Serial Audio Controller Global Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_i2s_regs.sacr0 = data & 0x0000ff3d;
}

u32 pxa255_periphs_device::i2s_sacr1_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_i2s_regs.sacr1;
	LOGMASKED(LOG_I2S, "%s: i2s_sacr1_r: Serial Audio Controller I2S/MSB-Justified Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::i2s_sacr1_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_I2S, "%s: i2s_sacr1_w: Serial Audio Controller I2S/MSB-Justified Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_i2s_regs.sacr1 = data & 0x00000039;
}

u32 pxa255_periphs_device::i2s_sasr0_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_i2s_regs.sasr0;
	LOGMASKED(LOG_I2S, "%s: i2s_sasr0_r: Serial Audio Controller I2S/MSB-Justified Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::i2s_sasr0_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_I2S, "%s: i2s_sasr0_w: Serial Audio Controller I2S/MSB-Justified Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_i2s_regs.sasr0 = data & 0x0000ff7f;
}

u32 pxa255_periphs_device::i2s_saimr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_i2s_regs.saimr;
	LOGMASKED(LOG_I2S, "%s: i2s_saimr_r: Serial Audio Interrupt Mask Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::i2s_saimr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_I2S, "%s: i2s_saimr_w: Serial Audio Interrupt Mask Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_i2s_regs.saimr = data & 0x00000078;
}

u32 pxa255_periphs_device::i2s_saicr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_i2s_regs.saicr;
	LOGMASKED(LOG_I2S, "%s: i2s_saimr_r: Serial Audio Interrupt Clear Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::i2s_saicr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_I2S, "%s: i2s_saicr_w: Serial Audio Interrupt Clear Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (m_i2s_regs.saicr & SAICR_ROR)
	{
		m_i2s_regs.sasr0 &= ~SASR0_ROR;
	}
	if (m_i2s_regs.saicr & SAICR_TUR)
	{
		m_i2s_regs.sasr0 &= ~SASR0_TUR;
	}
}

u32 pxa255_periphs_device::i2s_sadiv_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_i2s_regs.sadiv;
	LOGMASKED(LOG_I2S, "%s: i2s_sadiv_r: Serial Audio Clock Divider Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::i2s_sadiv_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_I2S, "%s: i2s_saicr_w: Serial Audio Clock Divider Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_i2s_regs.sadiv = data & 0x0000007f;
	for (int i = 0; i < 2; i++)
	{
		m_dmadac[i]->set_frequency(((double)147600000 / (double)m_i2s_regs.sadiv) / 256.0);
		m_dmadac[i]->enable(1);
	}
}

u32 pxa255_periphs_device::i2s_sadr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_i2s_regs.sadr;
	LOGMASKED(LOG_I2S, "%s: i2s_sadr_r: Serial Audio Data Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::i2s_sadr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_I2S, "%s: i2s_sadr_r: Serial Audio Data Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_i2s_regs.sadr = data;
}


/*

  PXA255 DMA controller

  pg. 151 to 182, PXA255 Processor Developers Manual [278693-002].pdf

*/

void pxa255_periphs_device::dma_irq_check()
{
	int set_irq = 0;
	for (int channel = 0; channel < 16; channel++)
	{
		if (m_dma_regs.dcsr[channel] & (DCSR_ENDINTR | DCSR_STARTINTR | DCSR_BUSERRINTR))
		{
			m_dma_regs.dint |= 1 << channel;
			set_irq = 1;
		}
		else
		{
			m_dma_regs.dint &= ~(1 << channel);
		}
	}

	set_irq_line(INT_DMA, set_irq);
}

void pxa255_periphs_device::dma_load_descriptor_and_start(int channel)
{
	// Shut down any transfers that are currently going on, software should be smart enough to check if a
	// transfer is running before starting another one on the same channel.
	if (m_dma_regs.timer[channel]->enabled())
	{
		m_dma_regs.timer[channel]->adjust(attotime::never);
	}

	// Load the next descriptor
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_dma_regs.dsadr[channel] = space.read_dword(m_dma_regs.ddadr[channel] + 0x4);
	m_dma_regs.dtadr[channel] = space.read_dword(m_dma_regs.ddadr[channel] + 0x8);
	m_dma_regs.dcmd[channel]  = space.read_dword(m_dma_regs.ddadr[channel] + 0xc);
	m_dma_regs.ddadr[channel] = space.read_dword(m_dma_regs.ddadr[channel]);

	// Start our end-of-transfer timer
	switch (channel)
	{
		case 3:
			m_dma_regs.timer[channel]->adjust(attotime::from_hz((147600000 / m_i2s_regs.sadiv) / (4 * 64)) * (m_dma_regs.dcmd[channel] & 0x00001fff), channel);
			break;
		default:
			m_dma_regs.timer[channel]->adjust(attotime::from_hz(100000000) * (m_dma_regs.dcmd[channel] & 0x00001fff), channel);
			break;
	}

	// Interrupt as necessary
	if (m_dma_regs.dcmd[channel] & DCMD_STARTIRQEN)
	{
		m_dma_regs.dcsr[channel] |= DCSR_STARTINTR;
	}

	m_dma_regs.dcsr[channel] &= ~DCSR_STOPSTATE;
}

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::audio_dma_end_tick)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 count = m_dma_regs.dcmd[3] & 0x00001fff;
	u32 sadr = m_dma_regs.dsadr[3];

	s16 *out_samples = &m_samples[0];
	for (u32 index = 0; index < count; index += 4, sadr += 4)
	{
		const u32 word = space.read_dword(sadr);
		*out_samples++ = (s16)(word >> 16);
		*out_samples++ = (s16)(word & 0xffff);
	}

	for (int index = 0; index < 2; index++)
	{
		m_dmadac[index]->flush();
		m_dmadac[index]->transfer(index, 2, 2, count/4, m_samples.get());
	}

	dma_finish(param);
}

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::dma_end_tick)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 count = m_dma_regs.dcmd[param] & 0x00001fff;
	u32 sadr = m_dma_regs.dsadr[param];
	u32 tadr = m_dma_regs.dtadr[param];

	static const u32 s_inc_size[4] = { 0, 1, 2, 4 };
	const u32 inc_index = (m_dma_regs.dcmd[param] >> DCMD_SIZE_SHIFT) & DCMD_SIZE_MASK;
	const u32 inc_val = s_inc_size[inc_index];
	const u32 sadr_inc = (m_dma_regs.dcmd[param] & DCMD_INCSRCADDR) ? inc_val : 0;
	const u32 tadr_inc = (m_dma_regs.dcmd[param] & DCMD_INCTRGADDR) ? inc_val : 0;

	if (inc_val > 0)
	{
		switch (inc_val)
		{
			case DCMD_SIZE_8:
				for (u32 index = 0; index < count; index += inc_val, sadr += sadr_inc, tadr += tadr_inc)
					space.write_byte(tadr, space.read_byte(sadr));
				break;
			case DCMD_SIZE_16:
				for (u32 index = 0; index < count; index += inc_val, sadr += sadr_inc, tadr += tadr_inc)
					space.write_word(tadr, space.read_byte(sadr));
				break;
			case DCMD_SIZE_32:
				for (u32 index = 0; index < count; index += inc_val, sadr += sadr_inc, tadr += tadr_inc)
					space.write_dword(tadr, space.read_byte(sadr));
				break;
			default:
				LOGMASKED(LOG_DMA, "pxa255_dma_dma_end: Unsupported DMA size\n");
				break;
		}
	}

	dma_finish(param);
}

void pxa255_periphs_device::dma_finish(int channel)
{
	if (m_dma_regs.dcmd[channel] & DCMD_ENDIRQEN)
	{
		m_dma_regs.dcsr[channel] |= DCSR_ENDINTR;
	}

	if (!(m_dma_regs.ddadr[channel] & DDADR_STOP) && (m_dma_regs.dcsr[channel] & DCSR_RUN))
	{
		if (m_dma_regs.dcsr[channel] & DCSR_RUN)
		{
			dma_load_descriptor_and_start(channel);
		}
		else
		{
			m_dma_regs.dcsr[channel] &= ~DCSR_RUN;
			m_dma_regs.dcsr[channel] |= DCSR_STOPSTATE;
		}
	}
	else
	{
		m_dma_regs.dcsr[channel] &= ~DCSR_RUN;
		m_dma_regs.dcsr[channel] |= DCSR_STOPSTATE;
	}

	dma_irq_check();
}

u32 pxa255_periphs_device::dma_dcsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs.dcsr[offset];
	LOGMASKED(LOG_DMA, "%s: dma_dcsr_r: DMA Channel Control/Status Register %d: %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	return data;
}

void pxa255_periphs_device::dma_dcsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dcsr_w: DMA Channel Control/Status Register %d = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	m_dma_regs.dcsr[offset] &= ~(data & 0x00000007);
	m_dma_regs.dcsr[offset] &= ~0x60000000;
	m_dma_regs.dcsr[offset] |= data & 0x60000000;
	if ((data & DCSR_RUN) && !(m_dma_regs.dcsr[offset] & DCSR_RUN))
	{
		m_dma_regs.dcsr[offset] |= DCSR_RUN;
		if (data & DCSR_NODESCFETCH)
		{
			LOGMASKED(LOG_DMA, "%s:             No-Descriptor-Fetch mode is not supported.\n", machine().describe_context());
			return;
		}

		dma_load_descriptor_and_start(offset);
	}
	else if (!(data & DCSR_RUN))
	{
		m_dma_regs.dcsr[offset] &= ~DCSR_RUN;
	}

	dma_irq_check();
}

u32 pxa255_periphs_device::dma_dint_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs.dint;
	LOGMASKED(LOG_DMA, "%s: dma_dint_r: DMA Interrupt Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::dma_dint_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dint_w: DMA Interrupt Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_dma_regs.dint &= ~data;
}

u32 pxa255_periphs_device::dma_drcmr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs.drcmr[offset];
	LOGMASKED(LOG_DMA, "%s: dma_drcmr_r: DMA Request to Channel Map Register %d: %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	return data;
}

void pxa255_periphs_device::dma_drcmr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_drcmr_w: DMA Request to Channel Map Register %d = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	m_dma_regs.drcmr[offset] = data & 0x0000008f;
}

template <int Which>
u32 pxa255_periphs_device::dma_ddadr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs.ddadr[Which];
	LOGMASKED(LOG_DMA, "%s: dma_ddadr_r: DMA Descriptor Address Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::dma_ddadr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_ddadr_w: DMA Descriptor Address Register %d = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	m_dma_regs.ddadr[offset] = data & 0xfffffff1;
}

template <int Which>
u32 pxa255_periphs_device::dma_dsadr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs.dsadr[Which];
	LOGMASKED(LOG_DMA, "%s: dma_dsadr_r: DMA Source Address Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::dma_dsadr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dsadr_w: DMA Source Address Register %d = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	m_dma_regs.dsadr[offset] = data & 0xfffffffc;
}

template <int Which>
u32 pxa255_periphs_device::dma_dtadr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs.dtadr[Which];
	LOGMASKED(LOG_DMA, "%s: dma_dtadr_r: DMA Target Address Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::dma_dtadr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dtadr_w: DMA Target Address Register %d = %08x & %08x\n", machine().describe_context(), offset, data, mem_mask);
	m_dma_regs.dtadr[Which] = data & 0xfffffffc;
}

template <int Which>
u32 pxa255_periphs_device::dma_dcmd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_dma_regs.dcmd[Which];
	LOGMASKED(LOG_DMA, "%s: dma_dcmd_r: DMA Command Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::dma_dcmd_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DMA, "%s: dma_dcmd_w: DMA Command Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_dma_regs.dcmd[offset] = data & 0xf067dfff;
}


/*

  PXA255 Real-Time Clock

  pg. 132 to 138, PXA255 Processor Developers Manual [278693-002].pdf

*/

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::rtc_tick)
{
	m_rtc_regs.rcnr++;
	if (BIT(m_rtc_regs.rtsr, 3))
	{
		m_rtc_regs.rtsr |= (1 << 1);
		set_irq_line(INT_RTC_HZ, 1);
	}

	if (m_rtc_regs.rcnr == m_rtc_regs.rtar)
	{
		if (BIT(m_rtc_regs.rtsr, 2))
		{
			m_rtc_regs.rtsr |= (1 << 0);
			set_irq_line(INT_RTC_ALARM, 1);
		}
	}
}

u32 pxa255_periphs_device::rtc_rcnr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rcnr;
	LOGMASKED(LOG_RTC, "%s: rtc_rcnr_r: RTC Counter Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::rtc_rcnr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_rcnr_w: RTC Counter Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_rtc_regs.rcnr);
}

u32 pxa255_periphs_device::rtc_rtar_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rtar;
	LOGMASKED(LOG_RTC, "%s: rtc_rtar_r: RTC Alarm Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::rtc_rtar_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_rtar_w: RTC Alarm Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_rtc_regs.rtar);
}

u32 pxa255_periphs_device::rtc_rtsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rtsr;
	LOGMASKED(LOG_RTC, "%s: rtc_rtsr_r: RTC Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::rtc_rtsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_rtsr_w: RTC Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	const u32 old = m_rtc_regs.rtsr;
	m_rtc_regs.rtsr &= ~(data & 0x00000003);
	m_rtc_regs.rtsr &= ~0x0000000c;
	m_rtc_regs.rtsr |= data & 0x0000000c;
	const u32 diff = old ^ m_rtc_regs.rtsr;
	if (BIT(diff, 1))
		set_irq_line(INT_RTC_HZ, 0);
	if (BIT(diff, 0))
		set_irq_line(INT_RTC_ALARM, 0);
}

u32 pxa255_periphs_device::rtc_rttr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_rtc_regs.rttr;
	LOGMASKED(LOG_RTC, "%s: rtc_rttr_r: RTC Trim Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::rtc_rttr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RTC, "%s: rtc_rttr_w: RTC Trim Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (!BIT(m_rtc_regs.rttr, 31))
	{
		COMBINE_DATA(&m_rtc_regs.rttr);
	}
}


/*

  PXA255 OS Timer register

  pg. 138 to 142, PXA255 Processor Developers Manual [278693-002].pdf

*/

void pxa255_periphs_device::ostimer_irq_check()
{
	set_irq_line(INT_OSTIMER0, (m_ostimer_regs.oier & OIER_E0) ? ((m_ostimer_regs.ossr & OSSR_M0) ? 1 : 0) : 0);
	//set_irq_line(INT_OSTIMER1, (m_ostimer_regs.oier & OIER_E1) ? ((m_ostimer_regs.ossr & OSSR_M1) ? 1 : 0) : 0);
	//set_irq_line(INT_OSTIMER2, (m_ostimer_regs.oier & OIER_E2) ? ((m_ostimer_regs.ossr & OSSR_M2) ? 1 : 0) : 0);
	//set_irq_line(INT_OSTIMER3, (m_ostimer_regs.oier & OIER_E3) ? ((m_ostimer_regs.ossr & OSSR_M3) ? 1 : 0) : 0);
}

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::ostimer_match_tick)
{
	m_ostimer_regs.ossr |= (1 << param);
	m_ostimer_regs.oscr = m_ostimer_regs.osmr[param];
	ostimer_irq_check();
}

template <int Which>
void pxa255_periphs_device::ostimer_update_interrupts()
{
	if ((m_ostimer_regs.oier & (OIER_E0 << Which)) && Which != 3)
	{
		m_ostimer_regs.timer[Which]->adjust(attotime::from_hz(3846400) * (m_ostimer_regs.osmr[Which] - m_ostimer_regs.oscr), Which);
	}
}

void pxa255_periphs_device::ostimer_update_count()
{
	const attotime time_delta = machine().time() - m_ostimer_regs.last_count_sync;
	const uint64_t ticks_elapsed = time_delta.as_ticks(INTERNAL_OSC);
	if (ticks_elapsed == 0ULL) // Accrue time until we can tick at least once
		return;

	const uint32_t wrapped_ticks = (uint32_t)ticks_elapsed;
	m_ostimer_regs.oscr += wrapped_ticks;
	m_ostimer_regs.last_count_sync = machine().time();
	ostimer_update_interrupts<0>();
	ostimer_update_interrupts<1>();
	ostimer_update_interrupts<2>();
	ostimer_update_interrupts<3>();
}

template <int Which>
u32 pxa255_periphs_device::tmr_osmr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostimer_regs.osmr[Which];
	LOGMASKED(LOG_OSTIMER, "%s: tmr_osmr_r: OS Timer Match Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::tmr_osmr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: pxa255_ostimer_w: OS Timer Match Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	ostimer_update_count();
	m_ostimer_regs.osmr[Which] = data;
	ostimer_update_count();
	ostimer_update_interrupts<Which>();
}

u32 pxa255_periphs_device::tmr_oscr_r(offs_t offset, u32 mem_mask)
{
	ostimer_update_count();
	const u32 data = m_ostimer_regs.oscr;
	LOGMASKED(LOG_OSTIMER, "%s: tmr_oscr_r: OS Timer Count Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return m_ostimer_regs.oscr;
}

void pxa255_periphs_device::tmr_oscr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_oscr_w: OS Timer Count Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_ostimer_regs.oscr = data;
	m_ostimer_regs.last_count_sync = machine().time();
}

u32 pxa255_periphs_device::tmr_ossr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostimer_regs.ossr;
	LOGMASKED(LOG_OSTIMER, "%s: tmr_ossr_r: OS Timer Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::tmr_ossr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_ossr_w: OS Timer Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_ostimer_regs.ossr &= ~data;
	ostimer_irq_check();
}

u32 pxa255_periphs_device::tmr_ower_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostimer_regs.ower;
	LOGMASKED(LOG_OSTIMER, "%s: tmr_ower_r: OS Timer Watchdog Match Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::tmr_ower_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_ower_w: OS Timer Watchdog Match Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_ostimer_regs.ower = data & 0x00000001;
}

u32 pxa255_periphs_device::tmr_oier_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_ostimer_regs.oier;
	LOGMASKED(LOG_OSTIMER, "%s: tmr_oier_r: OS Timer Interrupt Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::tmr_oier_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_OSTIMER, "%s: tmr_oier_w: OS Timer Interrupt Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_ostimer_regs.oier = data & 0x0000000f;
}


/*

  PXA255 Interrupt registers

  pg. 124 to 132, PXA255 Processor Developers Manual [278693-002].pdf

*/

void pxa255_periphs_device::update_interrupts()
{
	m_intc_regs.icfp = (m_intc_regs.icpr & m_intc_regs.icmr) & m_intc_regs.iclr;
	m_intc_regs.icip = (m_intc_regs.icpr & m_intc_regs.icmr) & (~m_intc_regs.iclr);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_intc_regs.icfp ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE,  m_intc_regs.icip ? ASSERT_LINE : CLEAR_LINE);
}

void pxa255_periphs_device::set_irq_line(u32 line, int irq_state)
{
	m_intc_regs.icpr &= ~line;
	m_intc_regs.icpr |= irq_state ? line : 0;
	update_interrupts();
}

u32 pxa255_periphs_device::intc_icip_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icip;
	LOGMASKED(LOG_INTC, "%s: intc_icip_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::intc_icip_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icip_w: (Invalid Write) Interrupt Controller IRQ Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 pxa255_periphs_device::intc_icmr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icmr;
	LOGMASKED(LOG_INTC, "%s: intc_icmr_r: Interrupt Controller Mask Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::intc_icmr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icmr_w: Interrupt Controller Mask Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_intc_regs.icmr = data & 0xfffe7f00;
}

u32 pxa255_periphs_device::intc_iclr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.iclr;
	LOGMASKED(LOG_INTC, "%s: intc_iclr_r: Interrupt Controller Level Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::intc_iclr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_iclr_w: Interrupt Controller Level Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_intc_regs.iclr = data & 0xfffe7f00;
}

u32 pxa255_periphs_device::intc_icfp_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icfp;
	LOGMASKED(LOG_INTC, "%s: intc_icfp_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::intc_icfp_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icfp_w: (Invalid Write) Interrupt Controller FIQ Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 pxa255_periphs_device::intc_icpr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.icpr;
	LOGMASKED(LOG_INTC, "%s: intc_icpr_r: Interrupt Controller Pending Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::intc_icpr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_icpr_w: (Invalid Write) Interrupt Controller Pending Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 pxa255_periphs_device::intc_iccr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_intc_regs.iccr;
	LOGMASKED(LOG_INTC, "%s: intc_iccr_r: Interrupt Controller control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::intc_iccr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_INTC, "%s: intc_iccr_w: Interrupt Controller Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_intc_regs.iccr = data & 0x00000001;
}


/*

  PXA255 General-Purpose I/O registers

  pg. 105 to 124, PXA255 Processor Developers Manual [278693-002].pdf

*/

template <int Which>
void pxa255_periphs_device::update_gpio_outputs(const u32 old)
{
	const u32 new_data = (m_gpio_regs.in_data[Which] & ~m_gpio_regs.gpdr[Which]) | (m_gpio_regs.out_data[Which] & m_gpio_regs.gpdr[Which]);
	const u32 changed = old ^ new_data;
	if (changed == 0)
		return;

	for (u32 bit = 0; bit < 32; bit++)
	{
		if (!BIT(changed, bit))
			continue;
		LOGMASKED(LOG_GPIO, "Setting GPIO bit %d: %d\n", Which * 32 + bit, BIT(new_data, bit));
		m_gpio_w[Which * 32 + bit](BIT(new_data, bit));
	}
}

template <int Which>
void pxa255_periphs_device::check_gpio_irqs(const u32 old)
{
	const u32 new_data = (m_gpio_regs.in_data[Which] & ~m_gpio_regs.gpdr[Which]) | (m_gpio_regs.out_data[Which] & m_gpio_regs.gpdr[Which]);
	if (old == new_data)
		return;

	const u32 rising = ~old & new_data;
	const u32 falling = old & ~new_data;

	LOGMASKED(LOG_GPIO, "pxa255: Rising %08x, Falling %08x\n", rising, falling);

	const u32 old_gedr = m_gpio_regs.gedr[Which];
	m_gpio_regs.gedr[Which] |= rising & m_gpio_regs.grer[Which];
	m_gpio_regs.gedr[Which] |= falling & m_gpio_regs.gfer[Which];

	LOGMASKED(LOG_GPIO, "pxa255: Old GEDR%d %08x, New GEDR%d %08x\n", Which, old_gedr, Which, m_gpio_regs.gedr[Which]);
	const u32 changed_gedr = old_gedr ^ m_gpio_regs.gedr[Which];
	if (changed_gedr == 0)
		return;

	for (u32 bit = 0; bit < 32; bit++)
	{
		if (!BIT(changed_gedr, bit))
			continue;

		LOGMASKED(LOG_GPIO, "pxa255: Edge detected on GPIO%d Pin %d\n", Which, bit);
		if (Which == 0)
		{
			if (bit > 1)
				set_irq_line(INT_GPIO84_2, 1);
			else if (bit == 1)
				set_irq_line(INT_GPIO1, 1);
			else
				set_irq_line(INT_GPIO0, 1);
		}
		else if (Which == 1)
		{
			set_irq_line(INT_GPIO84_2, 1);
		}
		else if (Which == 2)
		{
			set_irq_line(INT_GPIO84_2, 1);
		}
	}
}

template void pxa255_periphs_device::gpio_in<0>(int state);
template void pxa255_periphs_device::gpio_in<1>(int state);
template void pxa255_periphs_device::gpio_in<2>(int state);
template void pxa255_periphs_device::gpio_in<3>(int state);
template void pxa255_periphs_device::gpio_in<4>(int state);
template void pxa255_periphs_device::gpio_in<5>(int state);
template void pxa255_periphs_device::gpio_in<6>(int state);
template void pxa255_periphs_device::gpio_in<7>(int state);
template void pxa255_periphs_device::gpio_in<8>(int state);
template void pxa255_periphs_device::gpio_in<9>(int state);
template void pxa255_periphs_device::gpio_in<10>(int state);
template void pxa255_periphs_device::gpio_in<11>(int state);
template void pxa255_periphs_device::gpio_in<12>(int state);
template void pxa255_periphs_device::gpio_in<13>(int state);
template void pxa255_periphs_device::gpio_in<14>(int state);
template void pxa255_periphs_device::gpio_in<15>(int state);
template void pxa255_periphs_device::gpio_in<16>(int state);
template void pxa255_periphs_device::gpio_in<17>(int state);
template void pxa255_periphs_device::gpio_in<18>(int state);
template void pxa255_periphs_device::gpio_in<19>(int state);
template void pxa255_periphs_device::gpio_in<20>(int state);
template void pxa255_periphs_device::gpio_in<21>(int state);
template void pxa255_periphs_device::gpio_in<22>(int state);
template void pxa255_periphs_device::gpio_in<23>(int state);
template void pxa255_periphs_device::gpio_in<24>(int state);
template void pxa255_periphs_device::gpio_in<25>(int state);
template void pxa255_periphs_device::gpio_in<26>(int state);
template void pxa255_periphs_device::gpio_in<27>(int state);
template void pxa255_periphs_device::gpio_in<28>(int state);
template void pxa255_periphs_device::gpio_in<29>(int state);
template void pxa255_periphs_device::gpio_in<30>(int state);
template void pxa255_periphs_device::gpio_in<31>(int state);
template void pxa255_periphs_device::gpio_in<32>(int state);
template void pxa255_periphs_device::gpio_in<33>(int state);
template void pxa255_periphs_device::gpio_in<34>(int state);
template void pxa255_periphs_device::gpio_in<35>(int state);
template void pxa255_periphs_device::gpio_in<36>(int state);
template void pxa255_periphs_device::gpio_in<37>(int state);
template void pxa255_periphs_device::gpio_in<38>(int state);
template void pxa255_periphs_device::gpio_in<39>(int state);
template void pxa255_periphs_device::gpio_in<40>(int state);
template void pxa255_periphs_device::gpio_in<41>(int state);
template void pxa255_periphs_device::gpio_in<42>(int state);
template void pxa255_periphs_device::gpio_in<43>(int state);
template void pxa255_periphs_device::gpio_in<44>(int state);
template void pxa255_periphs_device::gpio_in<45>(int state);
template void pxa255_periphs_device::gpio_in<46>(int state);
template void pxa255_periphs_device::gpio_in<47>(int state);
template void pxa255_periphs_device::gpio_in<48>(int state);
template void pxa255_periphs_device::gpio_in<49>(int state);
template void pxa255_periphs_device::gpio_in<50>(int state);
template void pxa255_periphs_device::gpio_in<51>(int state);
template void pxa255_periphs_device::gpio_in<52>(int state);
template void pxa255_periphs_device::gpio_in<53>(int state);
template void pxa255_periphs_device::gpio_in<54>(int state);
template void pxa255_periphs_device::gpio_in<55>(int state);
template void pxa255_periphs_device::gpio_in<56>(int state);
template void pxa255_periphs_device::gpio_in<57>(int state);
template void pxa255_periphs_device::gpio_in<58>(int state);
template void pxa255_periphs_device::gpio_in<59>(int state);
template void pxa255_periphs_device::gpio_in<60>(int state);
template void pxa255_periphs_device::gpio_in<61>(int state);
template void pxa255_periphs_device::gpio_in<62>(int state);
template void pxa255_periphs_device::gpio_in<63>(int state);
template void pxa255_periphs_device::gpio_in<64>(int state);
template void pxa255_periphs_device::gpio_in<65>(int state);
template void pxa255_periphs_device::gpio_in<66>(int state);
template void pxa255_periphs_device::gpio_in<67>(int state);
template void pxa255_periphs_device::gpio_in<68>(int state);
template void pxa255_periphs_device::gpio_in<69>(int state);
template void pxa255_periphs_device::gpio_in<70>(int state);
template void pxa255_periphs_device::gpio_in<71>(int state);
template void pxa255_periphs_device::gpio_in<72>(int state);
template void pxa255_periphs_device::gpio_in<73>(int state);
template void pxa255_periphs_device::gpio_in<74>(int state);
template void pxa255_periphs_device::gpio_in<75>(int state);
template void pxa255_periphs_device::gpio_in<76>(int state);
template void pxa255_periphs_device::gpio_in<77>(int state);
template void pxa255_periphs_device::gpio_in<78>(int state);
template void pxa255_periphs_device::gpio_in<79>(int state);
template void pxa255_periphs_device::gpio_in<80>(int state);
template void pxa255_periphs_device::gpio_in<81>(int state);
template void pxa255_periphs_device::gpio_in<82>(int state);
template void pxa255_periphs_device::gpio_in<83>(int state);
template void pxa255_periphs_device::gpio_in<84>(int state);
template void pxa255_periphs_device::gpio_in<85>(int state);
template void pxa255_periphs_device::gpio_in<86>(int state);
template void pxa255_periphs_device::gpio_in<87>(int state);
template void pxa255_periphs_device::gpio_in<88>(int state);
template void pxa255_periphs_device::gpio_in<89>(int state);
template void pxa255_periphs_device::gpio_in<90>(int state);
template void pxa255_periphs_device::gpio_in<91>(int state);
template void pxa255_periphs_device::gpio_in<92>(int state);
template void pxa255_periphs_device::gpio_in<93>(int state);
template void pxa255_periphs_device::gpio_in<94>(int state);
template void pxa255_periphs_device::gpio_in<95>(int state);

template <int Bit>
void pxa255_periphs_device::gpio_in(int state)
{
	LOGMASKED(LOG_GPIO, "pxa255: GPIO Pin %d written: %d\n", Bit, state);

	const u32 which = Bit >> 5;
	const u32 old = m_gpio_regs.in_data[which] & ~m_gpio_regs.gpdr[which];
	if (state)
		m_gpio_regs.in_data[which] |= (1 << (Bit & 31));
	else
		m_gpio_regs.in_data[which] &= ~(1 << (Bit & 31));

	const u32 new_inputs = m_gpio_regs.in_data[which] & ~m_gpio_regs.gpdr[which];
	LOGMASKED(LOG_GPIO, "pxa255: Old GPIO Pin %d Input %08x, New GPIO Pin %d Input %08x\n", Bit, old, Bit, new_inputs);

	if (Bit < 32)
		check_gpio_irqs<0>(old);
	else if (Bit < 64)
		check_gpio_irqs<1>(old);
	else
		check_gpio_irqs<2>(old);
}

template <int Which>
u32 pxa255_periphs_device::gpio_gplr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_gpio_regs.in_data[Which] & ~m_gpio_regs.gpdr[Which]) | (m_gpio_regs.out_data[Which] & m_gpio_regs.gpdr[Which]);
	LOGMASKED(LOG_GPIO, "%s: gpio_gplr_r: GPIO Pin-Level Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gplr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gplr_w: (Invalid Write) GPIO Pin-Level Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 pxa255_periphs_device::gpio_gpdr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gpdr[Which];
	LOGMASKED(LOG_GPIO, "%s: gpio_gpdr_r: GPIO Pin Direction Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gpdr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gpdr_w: GPIO Pin Direction Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_gpio_regs.gpdr[Which] = data;
}

template <int Which>
u32 pxa255_periphs_device::gpio_gpsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0;
	LOGMASKED(LOG_GPIO, "%s: gpio_gpsr_r: (Invalid Read) GPIO Pin Output Set Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gpsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gpsr_w: GPIO Pin Output Set Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_gpio_regs.out_data[Which] |= data & mem_mask;
	const u32 set = data & mem_mask & m_gpio_regs.gpdr[Which];
	for (u32 i = 0; i < 32; i++)
	{
		if (BIT(set, i))
			m_gpio_w[Which * 32 + i](1);
	}
}

template <int Which>
u32 pxa255_periphs_device::gpio_gpcr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = 0;
	LOGMASKED(LOG_GPIO, "%s: gpio_gpcr_r: (Invalid Read) GPIO Pin Output Clear Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gpcr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gpcr_w: GPIO Pin Output Clear Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_gpio_regs.out_data[Which] &= ~(data & mem_mask);
	const u32 cleared = data & mem_mask & m_gpio_regs.gpdr[Which];
	for (u32 i = 0; i < 32; i++)
	{
		if (BIT(cleared, i))
			m_gpio_w[Which * 32 + i](0);
	}
}

template <int Which>
u32 pxa255_periphs_device::gpio_grer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.grer[Which];
	LOGMASKED(LOG_GPIO, "%s: gpio_grer_r: GPIO Rising Edge Detect Enable Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_grer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_grer_w: GPIO Rising Edge Detect Enable Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_gpio_regs.grer[Which] = data;
}

template <int Which>
u32 pxa255_periphs_device::gpio_gfer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gfer[Which];
	LOGMASKED(LOG_GPIO, "%s: gpio_grer_r: GPIO Falling Edge Detect Enable Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gfer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gfer_w: GPIO Falling Edge Detect Enable Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_gpio_regs.gfer[Which] = data;
}
template <int Which>
u32 pxa255_periphs_device::gpio_gedr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gedr[Which];
	LOGMASKED(LOG_GPIO, "%s: gpio_gedr_r: GPIO Edge Detect Status Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gedr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gedr_w: GPIO Edge Detect Status Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	const u32 old = m_gpio_regs.gedr[Which];
	m_gpio_regs.gedr[Which] &= ~data;
	const u32 lowered = old & ~m_gpio_regs.gedr[Which];
	if (Which == 0)
	{
		if (BIT(lowered, 0))
		{
			set_irq_line(INT_GPIO0, 0);
			return;
		}
		else if (BIT(lowered, 1))
		{
			set_irq_line(INT_GPIO1, 0);
			return;
		}
		else if (!(lowered & 0xfffffffc))
		{
			return;
		}
	}

	if (!m_gpio_regs.gedr[0] && !m_gpio_regs.gedr[1] && !m_gpio_regs.gedr[2])
		set_irq_line(INT_GPIO84_2, 0);
}

template <int Which>
u32 pxa255_periphs_device::gpio_gafrl_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gafrl[Which];
	LOGMASKED(LOG_GPIO, "%s: gpio_gafrl_r: GPIO Alternate Function Register %d Lower: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gafrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gafrl_w: GPIO Alternate Function Register %d Lower = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_gpio_regs.gafrl[Which] = data;
}

template <int Which>
u32 pxa255_periphs_device::gpio_gafru_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_gpio_regs.gafru[Which];
	LOGMASKED(LOG_GPIO, "%s: gpio_gafru_r: GPIO Alternate Function Register %d Upper: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::gpio_gafru_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_GPIO, "%s: gpio_gafru_w: GPIO Alternate Function Register %d Upper = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_gpio_regs.gafru[Which] = data;
}


/*

  PXA255 LCD Controller

  pg. 265 to 310, PXA255 Processor Developers Manual [278693-002].pdf

*/

u32 pxa255_periphs_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y <= (m_lcd_regs.lccr[2] & LCCR2_LPP); y++)
	{
		u32 *dst = &bitmap.pix(y);
		for (int x = 0; x <= (m_lcd_regs.lccr[1] & LCCR1_PPL); x++)
		{
			*dst++ = m_lcd_palette[m_lcd_framebuffer[y * ((m_lcd_regs.lccr[1] & LCCR1_PPL) + 1) + x]];
		}
	}
	return 0;
}

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::lcd_dma_eof_tick)
{
	LOGMASKED(LOG_LCD_DMA, "End of frame callback\n" );
	if (m_lcd_regs.dma[param].ldcmd & LDCMD_EOFINT)
	{
		m_lcd_regs.liidr = m_lcd_regs.dma[param].fidr;
		m_lcd_regs.lcsr |= LCSR_EOF;
	}
	lcd_check_load_next_branch(param);
	lcd_irq_check();
}

void pxa255_periphs_device::lcd_load_dma_descriptor(u32 address, int channel)
{
	address_space & space = m_maincpu->space(AS_PROGRAM);
	m_lcd_regs.dma[channel].fdadr = space.read_dword(address);
	m_lcd_regs.dma[channel].fsadr = space.read_dword(address + 0x04);
	m_lcd_regs.dma[channel].fidr  = space.read_dword(address + 0x08);
	m_lcd_regs.dma[channel].ldcmd = space.read_dword(address + 0x0c);
	LOGMASKED(LOG_LCD_DMA, "lcd_load_dma_descriptor, address = %08x, channel = %d\n", address, channel);
	LOGMASKED(LOG_LCD_DMA, "    DMA Frame Descriptor: %08x\n", m_lcd_regs.dma[channel].fdadr );
	LOGMASKED(LOG_LCD_DMA, "    DMA Frame Source Address: %08x\n", m_lcd_regs.dma[channel].fsadr );
	LOGMASKED(LOG_LCD_DMA, "    DMA Frame ID: %08x\n", m_lcd_regs.dma[channel].fidr );
	LOGMASKED(LOG_LCD_DMA, "    DMA Command: %08x\n", m_lcd_regs.dma[channel].ldcmd );
}

void pxa255_periphs_device::lcd_irq_check()
{
	if (((m_lcd_regs.lcsr & LCSR_BS) != 0 && (m_lcd_regs.lccr[0] & LCCR0_BM)  == 0) ||
	   ((m_lcd_regs.lcsr & LCSR_EOF) != 0 && (m_lcd_regs.lccr[0] & LCCR0_EFM) == 0) ||
	   ((m_lcd_regs.lcsr & LCSR_SOF) != 0 && (m_lcd_regs.lccr[0] & LCCR0_SFM) == 0))
	{
		set_irq_line(INT_LCD, 1);
	}
	else
	{
		set_irq_line(INT_LCD, 0);
	}
}

void pxa255_periphs_device::lcd_dma_kickoff(int channel)
{
	if (m_lcd_regs.dma[channel].fdadr != 0)
	{
		attotime period = attotime::from_hz(20000000) * (m_lcd_regs.dma[channel].ldcmd & 0x000fffff);

		m_lcd_regs.dma[channel].eof->adjust(period, channel);

		if (m_lcd_regs.dma[channel].ldcmd & LDCMD_SOFINT)
		{
			m_lcd_regs.liidr = m_lcd_regs.dma[channel].fidr;
			m_lcd_regs.lcsr |= LCSR_SOF;
			lcd_irq_check();
		}

		if (m_lcd_regs.dma[channel].ldcmd & LDCMD_PAL)
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			int length = m_lcd_regs.dma[channel].ldcmd & 0x000fffff;
			int index = 0;
			for(index = 0; index < length; index += 2)
			{
				u16 color = space.read_word((m_lcd_regs.dma[channel].fsadr &~ 1) + index);
				m_lcd_palette[index >> 1] = (((((color >> 11) & 0x1f) << 3) | (color >> 13)) << 16) | (((((color >> 5) & 0x3f) << 2) | ((color >> 9) & 0x3)) << 8) | (((color & 0x1f) << 3) | ((color >> 2) & 0x7));
				m_palette->set_pen_color(index >> 1, (((color >> 11) & 0x1f) << 3) | (color >> 13), (((color >> 5) & 0x3f) << 2) | ((color >> 9) & 0x3), ((color & 0x1f) << 3) | ((color >> 2) & 0x7));
			}
		}
		else
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			int length = m_lcd_regs.dma[channel].ldcmd & 0x000fffff;
			int index = 0;
			for(index = 0; index < length; index++)
			{
				m_lcd_framebuffer[index] = space.read_byte(m_lcd_regs.dma[channel].fsadr + index);
			}
		}
	}
}

void pxa255_periphs_device::lcd_check_load_next_branch(int channel)
{
	if (m_lcd_regs.fbr[channel] & 1)
	{
		LOGMASKED(LOG_LCD_DMA, "lcd_check_load_next_branch: Taking branch\n" );
		m_lcd_regs.fbr[channel] &= ~1;
		address_space &space = m_maincpu->space(AS_PROGRAM);
		lcd_load_dma_descriptor(m_lcd_regs.fbr[channel] & 0xfffffff0, 0);
		m_lcd_regs.fbr[channel] = (space.read_dword(m_lcd_regs.fbr[channel] & 0xfffffff0) & 0xfffffff0) | (m_lcd_regs.fbr[channel] & 0x00000003);
		lcd_dma_kickoff(0);
		if (m_lcd_regs.fbr[channel] & 2)
		{
			m_lcd_regs.fbr[channel] &= ~2;
			if (!(m_lcd_regs.lccr[0] & LCCR0_BM))
			{
				m_lcd_regs.lcsr |= LCSR_BS;
			}
		}
	}
	else
	{
		LOGMASKED(LOG_LCD_DMA, "lcd_check_load_next_branch: Not taking branch\n" );
	}
}

template <int Which>
u32 pxa255_periphs_device::lcd_lccr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.lccr[Which];
	LOGMASKED(LOG_LCD, "%s: lcd_lccr_r: LCD Control Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::lcd_lccr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_lccr_w: LCD Control Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);

	if (Which == 0)
		m_lcd_regs.lccr[Which] = data & 0x00fffeff;
	else
	{
		m_lcd_regs.lccr[Which] = data;
		if (Which == 1 || Which == 2)
		{
			const u16 lpp = (m_lcd_regs.lccr[2] & LCCR2_LPP);
			const u16 ppl = (m_lcd_regs.lccr[1] & LCCR1_PPL);

			if (lpp && ppl)
			{
				rectangle rect(0, ppl, 0, lpp);
				m_screen->configure(1024, 1024, rect, HZ_TO_ATTOSECONDS(60));
			}
		}
	}
}

template <int Which>
u32 pxa255_periphs_device::lcd_fbr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.fbr[Which];
	LOGMASKED(LOG_LCD, "%s: lcd_fbr_r: LCD Frame Branch Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::lcd_fbr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_lccr_w: LCD Frame Branch Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	m_lcd_regs.fbr[Which] = data & 0xfffffff3;
	if (!m_lcd_regs.dma[Which].eof->enabled())
	{
		LOGMASKED(LOG_LCD, "ch%d EOF timer is not enabled, taking branch now\n", Which);
		lcd_check_load_next_branch(Which);
		lcd_irq_check();
	}
}

u32 pxa255_periphs_device::lcd_lcsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.lcsr;
	LOGMASKED(LOG_LCD, "%s: lcd_lcsr_r: LCD Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::lcd_lcsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_lcsr_w: LCD Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_lcd_regs.lcsr &= ~data;
	lcd_irq_check();
}

u32 pxa255_periphs_device::lcd_liidr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.liidr;
	LOGMASKED(LOG_LCD, "%s: lcd_liidr_r: LCD Interrupt ID Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::lcd_liidr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_liidr_w: LCD Interrupt ID Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

u32 pxa255_periphs_device::lcd_trgbr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.trgbr;
	LOGMASKED(LOG_LCD, "%s: lcd_trgbr_r: TMED RGB Seed Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::lcd_trgbr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_trgbr_w: TMED RGB Seed Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_lcd_regs.trgbr = data & 0x00ffffff;
}

u32 pxa255_periphs_device::lcd_tcr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.tcr;
	LOGMASKED(LOG_LCD, "%s: lcd_tcr_r: TMED Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::lcd_tcr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_tcr_w: TMED Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_lcd_regs.tcr = data & 0x00004fff;
}

template <int Which>
u32 pxa255_periphs_device::lcd_fdadr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.dma[Which].fdadr;
	LOGMASKED(LOG_LCD, "%s: lcd_fdadr_r: LCD DMA Frame Descriptor Address Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::lcd_fdadr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_fdadr_w: LCD DMA Frame Descriptor Address Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	if (!m_lcd_regs.dma[Which].eof->enabled())
	{
		lcd_load_dma_descriptor(data & 0xfffffff0, Which);
	}
	else
	{
		m_lcd_regs.fbr[Which] &= 0x00000003;
		m_lcd_regs.fbr[Which] |= data & 0xfffffff0;
	}
}

template <int Which>
u32 pxa255_periphs_device::lcd_fsadr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.dma[Which].fsadr;
	LOGMASKED(LOG_LCD, "%s: lcd_fsadr_r: LCD DMA Frame Source Address Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::lcd_fsadr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_fsadr_w: (Ignored) LCD DMA Frame Source Address Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 pxa255_periphs_device::lcd_fidr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.dma[Which].fidr;
	LOGMASKED(LOG_LCD, "%s: lcd_fidr_r: LCD DMA Frame ID Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::lcd_fidr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_fidr_w: (Ignored) LCD DMA Frame ID Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}

template <int Which>
u32 pxa255_periphs_device::lcd_ldcmd_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_lcd_regs.dma[Which].ldcmd & 0xfff00000;
	LOGMASKED(LOG_LCD, "%s: lcd_ldcmd_r: LCD DMA Command Register %d: %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::lcd_ldcmd_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_LCD, "%s: lcd_ldcmd_w: (Ignored) LCD DMA Command Register %d = %08x & %08x\n", machine().describe_context(), Which, data, mem_mask);
}


/*

  PXA255 Power Controller

  pg. 85 to 96, PXA255 Processor Developers Manual [278693-002].pdf

*/

u32 pxa255_periphs_device::pwr_pmcr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pmcr;
	LOGMASKED(LOG_POWER, "%s: pwr_pmcr_r: Power Manager Control Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pmcr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pmcr_w: Power Manager Control Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pmcr);
}

u32 pxa255_periphs_device::pwr_pssr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pssr;
	LOGMASKED(LOG_POWER, "%s: pwr_pssr_r: Power Manager Sleep Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pssr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pssr_w: Power Manager Sleep Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_power_regs.pssr &= ~(data & 0x00000037);
}

u32 pxa255_periphs_device::pwr_pspr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pspr;
	LOGMASKED(LOG_POWER, "%s: pwr_pspr_r: Power Manager Scratch Pad Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pspr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pspr_w: Power Manager Scratch Pad Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pspr);
}

u32 pxa255_periphs_device::pwr_pwer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pwer;
	LOGMASKED(LOG_POWER, "%s: pwr_pwer_r: Power Manager Wake-Up Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pwer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pwer_w: Power Manager Wake-Up Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pwer);
}

u32 pxa255_periphs_device::pwr_prer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.prer;
	LOGMASKED(LOG_POWER, "%s: pwr_prer_r: Power Manager GPIO Rising-Edge Detect Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_prer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_prer_w: Power Manager GPIO Rising-Edge Detect Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.prer);
}

u32 pxa255_periphs_device::pwr_pfer_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pfer;
	LOGMASKED(LOG_POWER, "%s: pwr_pfer_r: Power Manager GPIO Falling-Edge Detect Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pfer_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pfer_w: Power Manager GPIO Falling-Edge Detect Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pfer);
}

u32 pxa255_periphs_device::pwr_pedr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pedr;
	LOGMASKED(LOG_POWER, "%s: pwr_pedr_r: Power Manager GPIO Edge Detect Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pedr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pedr_w: Power Manager GPIO Edge-Detect Status Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_power_regs.pedr &= ~(data & 0x0000ffff);
}

u32 pxa255_periphs_device::pwr_pcfr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pcfr;
	LOGMASKED(LOG_POWER, "%s: pwr_pcfr_r: Power Manager General Configuration Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pcfr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pcfr_w: Power Manager General Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pcfr);
}

template <int Which>
u32 pxa255_periphs_device::pwr_pgsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pgsr[Which];
	LOGMASKED(LOG_POWER, "%s: pwr_pgsr_r: Power Manager GPIO Sleep State Register for GPIO%d-%d: %08x & %08x\n", machine().describe_context(), Which * 32, Which * 32 + 31, data, mem_mask);
	return data;
}

template <int Which>
void pxa255_periphs_device::pwr_pgsr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pgsr_w: Power Manager GPIO Sleep State Register for GPIO%d-%d = %08x & %08x\n", machine().describe_context(), Which * 32, Which * 32 + 31, data, mem_mask);
	COMBINE_DATA(&m_power_regs.pgsr[Which]);
}

u32 pxa255_periphs_device::pwr_rcsr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.rcsr;
	LOGMASKED(LOG_POWER, "%s: pwr_rcsr_r: Reset Controller Status Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

u32 pxa255_periphs_device::pwr_pmfw_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_power_regs.pmfw;
	LOGMASKED(LOG_POWER, "%s: pwr_pmfw_r: Power Manager Fast Sleep Wake-Up Configuration Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::pwr_pmfw_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_POWER, "%s: pwr_pmfw_w: Power Manager Fast Sleep Wake-Up Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_power_regs.pmfw);
}


/*
  PXA255 Clock controller

  pg. 96 to 100, PXA255 Processor Developers Manual [278693-002].pdf

*/

u32 pxa255_periphs_device::clk_cccr_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clk_regs.cccr;
	LOGMASKED(LOG_CLOCKS, "%s: clk_cccr_r: Core Clock Configuration Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::clk_cccr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_CLOCKS, "%s: clk_cccr_w: Core Clock Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_clk_regs.cccr);
}

u32 pxa255_periphs_device::clk_cken_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clk_regs.cken;
	LOGMASKED(LOG_CLOCKS, "%s: clk_cken_r: Clock Enable Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::clk_cken_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_CLOCKS, "%s: clk_cken_w: Clock Enable Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_clk_regs.cken);
}

u32 pxa255_periphs_device::clk_oscc_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_clk_regs.cccr;
	LOGMASKED(LOG_CLOCKS, "%s: clk_oscc_r: Oscillator Configuration Register: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void pxa255_periphs_device::clk_oscc_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_CLOCKS, "%s: clk_oscc_w: Oscillator Configuration Register = %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (BIT(data, 1))
	{
		m_clk_regs.oscc |= 0x00000003;
	}
}
