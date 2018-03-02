// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of GT graphics, also known as Memory Mapped Graphics (MMG), for
 * Intergraph InterPro systems.
 *
 * TODO
 *   - pixel-perfect line drawing to match diagnostics
 *   - many other diagnostic test failures
 *   - RI aliased and anti-aliased line drawing
 *   - fifos (no information at this point)
 *   - 76Hz refresh, 2MPix boards
 *   - support GT+, GTII
 *   - reset behaviour
 *
 * GT: original 2020 graphics
 * GT+: double buffered
 * GTII: double buffered, highlight
 *
 *                 068  2400 Graphics f/1 1Mp Monitor (V-60)
 *                 069  2400 Graphics f/2 1Mp Monitors (V-60)
 *    GT+          070  2400 Graphics f/1 1Mp Monitor (V-76)
 *    GT+          071  2400 Graphics f/2 1Mp Monitors (V-76)
 *    GT+ (2MPix)  081  2400 Graphics f/1 2Mp Monitor (V-60/76)
 *    GT+          101  2400 Graphics f/1 1Mp Monitor (V-76)
 *    GT+          102  2400 Graphics f/2 1Mp Monitors (V-76)
 *                 135  GT II Graphics f/1 2Mp Monitor (V-60/76)
 *                 136  GT II Graphics f/2 2Mp Monitor (V-60/76)
 *    GT/MMG       963  2000 Graphics f/1 1Mp Monitor
 *    GT/MMG       A79  2000 Graphics f/2 1Mp Monitors
 *                 B67  GT Plus Graphics f/1 1Mp Monitor (V-76)      How is this different to 070/101?
 *    GTII         B68  GT II Graphics f/1 1Mp Monitor (V-76)
 *    GTII         B70  GT II Graphics f/2 1Mp Monitors (V-76)
 *    GTII         B92  GT II Graphics f/1 2Mp Monitor (V-60/76)     6400 board
 *    GTII         B93  GT II Graphics f/2 2Mp Monitors (V-60/76)
 *    GTII         C05  25Mhz GTII Graphics f/1 1Mp Monitor
 *    GTII         C06  25Mhz GTII Graphics f/2 1Mp Monitors
 *    GTII         C41  GTII 60/76Hz Graphics f/1 2Mp Monitor        Functionally equivalant to MPCBB92. Will operate on the 67XX series. (maybe GTDB, i.e. GT graphics for 6000)
 *    GTII         C42  GTII 60/76Hz Graphics f/2 2Mp Monitor        Functionally equivalant to MPCBB93. Will operate on the 67XX series. (maybe GTDB, i.e. GT graphics for 6000)
 *
 * Board idprom feature byte 0 contains various flags:
 *
 *   0x01 ? single : dual
 *   0x02 ? 1 MPix (1184x884) : 2 MPix (1664x1248)
 *   0x04 ? gt (memsize 0x00100000/1M) : gtplus (memsize 0x01000000/16M)
 *
 *   1MPix boards: (feature[0] & 0xc0) == 0x80 ? 76Hz (xoff 264, yoff 57) : 60Hz (xoff 296, yoff 34)
 *   2MPix boards: (feature[0] & 0xc0) == 0x80 ? 76Hz (xoff 391, yoff 74) : (feature[0] & 0xc0) == 0x00 ? 60Hz (xoff 407, yoff 48) : 60/76Hz (check control register)
 */

#include "emu.h"
#include "screen.h"

#include "gt.h"

#define LOG_GENERAL (1U << 0)
#define LOG_LINE    (1U << 1)
#define LOG_BLIT    (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_LINE | LOG_BLIT)
//#define VERBOSE (LOG_GENERAL | LOG_LINE)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(MPCB963, mpcb963_device, "mpcb963", "2000 Graphics f/1 1Mp Monitor")
DEFINE_DEVICE_TYPE(MPCBA79, mpcba79_device, "mpcba79", "2000 Graphics f/2 1Mp Monitors")
DEFINE_DEVICE_TYPE(MPCB070, mpcb070_device, "mpcb070", "2400 Graphics f/1 1Mp Monitor (V-76)")
DEFINE_DEVICE_TYPE(MPCB071, mpcb071_device, "mpcb071", "2400 Graphics f/2 1Mp Monitors (V-76)")
DEFINE_DEVICE_TYPE(MPCB081, mpcb081_device, "mpcb081", "2400 Graphics f/1 2Mp Monitor (V-60/76)")

ADDRESS_MAP_START(gt_device_base::map)
	AM_RANGE(0x0a0, 0x0a3) AM_READWRITE8(contrast_dac_r, contrast_dac_w, 0x000000ff) // w/o?
	AM_RANGE(0x0b0, 0x0b3) AM_READWRITE(control_r, control_w)

	AM_RANGE(0x0c0, 0x0c3) AM_READWRITE(blit_src_address_r, blit_src_address_w) // w/o?
	AM_RANGE(0x0c4, 0x0c7) AM_READWRITE(blit_dst_address_r, blit_dst_address_w) // w/o?
	AM_RANGE(0x0c8, 0x0cb) AM_READWRITE16(blit_width_r, blit_width_w, 0x0000ffff) // w/o?

	AM_RANGE(0x0d0, 0x0d3) AM_WRITE(blit_control_w)
	AM_RANGE(0x0d4, 0x0d7) AM_READWRITE8(plane_enable_r, plane_enable_w, 0x000000ff)
	AM_RANGE(0x0d8, 0x0db) AM_READWRITE8(plane_data_r, plane_data_w, 0x000000ff)

	AM_RANGE(0x100, 0x103) AM_READWRITE16(bsga_width_r, bsga_width_w, 0x0000ffff)
	AM_RANGE(0x100, 0x103) AM_READWRITE16(bsga_tmp_r, bsga_tmp_w, 0xffff0000)

	AM_RANGE(0x104, 0x107) AM_WRITE16(bsga_xmin_w, 0x0000ffff)
	AM_RANGE(0x108, 0x10b) AM_WRITE16(bsga_ymin_w, 0x0000ffff)
	AM_RANGE(0x10c, 0x10f) AM_READWRITE16(bsga_xmin_r, bsga_xmin_w, 0x0000ffff)
	AM_RANGE(0x10c, 0x10f) AM_READWRITE16(bsga_ymin_r, bsga_ymin_w, 0xffff0000)

	AM_RANGE(0x110, 0x113) AM_READ16(bsga_acc0_r, 0x0000ffff)
	AM_RANGE(0x110, 0x113) AM_READ16(bsga_acc1_r, 0xffff0000)

	AM_RANGE(0x114, 0x117) AM_WRITE16(bsga_xmax_w, 0x0000ffff)
	AM_RANGE(0x118, 0x11b) AM_WRITE16(bsga_ymax_w, 0x0000ffff)
	AM_RANGE(0x11c, 0x11f) AM_READWRITE16(bsga_xmax_r, bsga_xmax_w, 0x0000ffff)
	AM_RANGE(0x11c, 0x11f) AM_READWRITE16(bsga_ymax_r, bsga_ymax_w, 0xffff0000)

	AM_RANGE(0x120, 0x123) AM_READ16(bsga_src0_r, 0x0000ffff)
	AM_RANGE(0x120, 0x123) AM_READ16(bsga_src1_r, 0xffff0000)

	AM_RANGE(0x124, 0x127) AM_WRITE16(bsga_xin1_w, 0x0000ffff)
	AM_RANGE(0x128, 0x12b) AM_WRITE16(bsga_yin1_w, 0x0000ffff)
	AM_RANGE(0x12c, 0x12f) AM_READ16(bsga_xin_r, 0x0000ffff)
	AM_RANGE(0x12c, 0x12f) AM_READ16(bsga_yin_r, 0xffff0000)
	AM_RANGE(0x12c, 0x12f) AM_WRITE(bsga_xin1yin1_w)
	AM_RANGE(0x130, 0x133) AM_READ16(bsga_status_r, 0x0000ffff)
	AM_RANGE(0x134, 0x137) AM_WRITE16(bsga_xin2_w, 0x0000ffff)
	AM_RANGE(0x138, 0x13b) AM_WRITE16(bsga_yin2_w, 0x0000ffff)
	AM_RANGE(0x13c, 0x13f) AM_WRITE(bsga_xin2yin2_w)

	// FDMDISK says all the ri registers are write only?
	// possibly also all 24 bit
	AM_RANGE(0x140, 0x143) AM_READWRITE(ri_initial_distance_r, ri_initial_distance_w)
	AM_RANGE(0x144, 0x147) AM_READWRITE(ri_distance_both_r, ri_distance_both_w)
	AM_RANGE(0x148, 0x14b) AM_READWRITE(ri_distance_major_r, ri_distance_major_w)
	AM_RANGE(0x14c, 0x14f) AM_READWRITE(ri_initial_address_r, ri_initial_address_w)
	AM_RANGE(0x150, 0x153) AM_READWRITE(ri_address_both_r, ri_address_both_w)
	AM_RANGE(0x154, 0x157) AM_READWRITE(ri_address_major_r, ri_address_major_w)
	AM_RANGE(0x158, 0x15b) AM_READWRITE(ri_initial_error_r, ri_initial_error_w)
	AM_RANGE(0x15c, 0x15f) AM_READWRITE(ri_error_both_r, ri_error_both_w)
	AM_RANGE(0x160, 0x163) AM_READWRITE(ri_error_major_r, ri_error_major_w)
	AM_RANGE(0x164, 0x167) AM_READWRITE(ri_stop_count_r, ri_stop_count_w) // 16 bit?

	AM_RANGE(0x16c, 0x16f) AM_READWRITE(ri_control_r, ri_control_w) // mask 1ff?

	//AM_RANGE(0x174, 0x177) AM_READWRITE(ri_xfer_r, ri_xfer_w)
	//AM_RANGE(0x178, 0x17b) AM_READWRITE(ri_xfer_r, ri_xfer_w)
	AM_RANGE(0x17c, 0x17f) AM_READWRITE(ri_xfer_r, ri_xfer_w)

	AM_RANGE(0x1a4, 0x1a7) AM_WRITE(bsga_float_w)

	//AM_RANGE(0x1c0, 0x1c3)
	//AM_RANGE(0x1c4, 0x1c7)
	//AM_RANGE(0x1c8, 0x1cb)
	//AM_RANGE(0x1cc, 0x1cf) // write32 - float conversion control (inhibit/enable overflow detection?)

/*
 * Don't know where/how these fifos come into play yet:
 *
    #define GT_FIFO_CONTROL(slot)       GT_BASE(slot, 0x300)
    #define GT_FIFO_STATUS(slot)        GT_BASE(slot, 0x304)
    #define GT_FIFO_LOW_WATER(slot)     GT_BASE(slot, 0x330)
    #define GT_FIFO_HI_WATER(slot)      GT_BASE(slot, 0x334)

    #define GT_FIFO_LW_ENB          0x08
    #define GT_FIFO_LW_INTR         0x40
    #define GT_FIFO_HW_INTR         0x80
 */
ADDRESS_MAP_END

ADDRESS_MAP_START(single_gt_device_base::map)
	AM_IMPORT_FROM(gt_device_base::map)

	AM_RANGE(0x00000000, 0x0000007f) AM_READ(idprom_r)
	AM_RANGE(0x00000080, 0x0000008f) AM_DEVICE8("ramdac0", bt459_device, map, 0x000000ff)

	AM_RANGE(0x00400000, 0x005fffff) AM_READWRITE(buffer_r, buffer_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(dual_gt_device_base::map)
	AM_IMPORT_FROM(gt_device_base::map)

	AM_RANGE(0x00000000, 0x0000007f) AM_READ(idprom_r)
	AM_RANGE(0x00000080, 0x0000008f) AM_DEVICE8("ramdac0", bt459_device, map, 0x000000ff)
	AM_RANGE(0x00000090, 0x0000009f) AM_DEVICE8("ramdac1", bt459_device, map, 0x000000ff)

	AM_RANGE(0x00400000, 0x007fffff) AM_READWRITE(buffer_r, buffer_w)
ADDRESS_MAP_END

ROM_START(mpcb963)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcb963a.bin", 0x0, 0x20, CRC(4cf4562d) SHA1(58bcc2afb66168f1d44a0366b6a5ccc4c22e0f32))
ROM_END

ROM_START(mpcba79)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcba79a.bin", 0x0, 0x20, CRC(a223fb92) SHA1(6dbd2aa75f1052b2467638e7d6e72c151fe23cfd))
ROM_END

ROM_START(mpcb070)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcb070b.bin", 0x0, 0x20, CRC(e0c302d1) SHA1(df2086fbd667605dd2c4a61d48dbcd0a38376e20))
ROM_END

ROM_START(mpcb071)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcb071b.bin", 0x0, 0x20, CRC(a9b635d7) SHA1(1b39d98a2f74cfa0a7b430b838412fbc1ade0e5d))
ROM_END

ROM_START(mpcb081)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcb081b.bin", 0x0, 0x20, CRC(79cec897) SHA1(ccda82a61c435800b7894a256deea22ea5dc4c87))
ROM_END

// FIXME: can't account for this delta yet
#define GT_X_DELTA 20

// FIXME: the screen parameters below match what's coded in the system software,
// (except for the unexplained X_DELTA above), but produce an off-by-one mismatch
// in the cursor diagnostic tests. Visual mouse cursor positioning does seem to
// be correct however, so don't quite understand where the problem is.

// GT, 60Hz, single 1MPix
MACHINE_CONFIG_START(mpcb963_device::device_add_mconfig)
	MCFG_SCREEN_ADD("screen0", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GT_PIXCLOCK, GT_XPIXELS + 296 + GT_X_DELTA, 296 + GT_X_DELTA, GT_XPIXELS + 296 + GT_X_DELTA, GT_YPIXELS + 34, 34, GT_YPIXELS + 34)
	MCFG_SCREEN_UPDATE_DEVICE("", mpcb963_device, screen_update0)
	MCFG_DEVICE_ADD("ramdac0", BT459, 0)
	MCFG_DEVICE_ADD("bpu0", DP8510, 0)
MACHINE_CONFIG_END

// GT, 60Hz, dual 1MPix
MACHINE_CONFIG_START(mpcba79_device::device_add_mconfig)
	MCFG_SCREEN_ADD("screen0", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GT_PIXCLOCK, GT_XPIXELS + 296 + GT_X_DELTA, 296 + GT_X_DELTA, GT_XPIXELS + 296 + GT_X_DELTA, GT_YPIXELS + 34, 34, GT_YPIXELS + 34)
	MCFG_SCREEN_UPDATE_DEVICE("", mpcba79_device, screen_update0)
	MCFG_DEVICE_ADD("ramdac0", BT459, 0)
	MCFG_DEVICE_ADD("bpu0", DP8510, 0)

	MCFG_SCREEN_ADD("screen1", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GT_PIXCLOCK, GT_XPIXELS + 296 + GT_X_DELTA, 296 + GT_X_DELTA, GT_XPIXELS + 296 + GT_X_DELTA, GT_YPIXELS + 34, 34, GT_YPIXELS + 34)
	MCFG_SCREEN_UPDATE_DEVICE("", mpcba79_device, screen_update1)
	MCFG_DEVICE_ADD("ramdac1", BT459, 0)
	MCFG_DEVICE_ADD("bpu1", DP8510, 0)
MACHINE_CONFIG_END

// GT+, 76Hz, single 1MPix
MACHINE_CONFIG_START(mpcb070_device::device_add_mconfig)
	MCFG_SCREEN_ADD("screen0", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GT_PIXCLOCK, GT_XPIXELS + 264 + GT_X_DELTA, 264 + GT_X_DELTA, GT_XPIXELS + 264 + GT_X_DELTA, GT_YPIXELS + 57, 57, GT_YPIXELS + 57)
	MCFG_SCREEN_UPDATE_DEVICE("", mpcb070_device, screen_update0)
	MCFG_DEVICE_ADD("ramdac0", BT459, 0)
	MCFG_DEVICE_ADD("bpu0", DP8510, 0)
MACHINE_CONFIG_END

// GT+, 76Hz, dual 1MPix
MACHINE_CONFIG_START(mpcb071_device::device_add_mconfig)
	MCFG_SCREEN_ADD("screen0", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GT_PIXCLOCK, GT_XPIXELS + 264 + GT_X_DELTA, 264 + GT_X_DELTA, GT_XPIXELS + 264 + GT_X_DELTA, GT_YPIXELS + 57, 57, GT_YPIXELS + 57)
	MCFG_SCREEN_UPDATE_DEVICE("", mpcb071_device, screen_update0)
	MCFG_DEVICE_ADD("ramdac0", BT459, 0)
	MCFG_DEVICE_ADD("bpu0", DP8510, 0)

	MCFG_SCREEN_ADD("screen1", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GT_PIXCLOCK, GT_XPIXELS + 264 + GT_X_DELTA, 264 + GT_X_DELTA, GT_XPIXELS + 264 + GT_X_DELTA, GT_YPIXELS + 57, 57, GT_YPIXELS + 57)
	MCFG_SCREEN_UPDATE_DEVICE("", mpcb071_device, screen_update1)
	MCFG_DEVICE_ADD("ramdac1", BT459, 0)
	MCFG_DEVICE_ADD("bpu1", DP8510, 0)
MACHINE_CONFIG_END

// GT+, 76Hz, single 2MPix
MACHINE_CONFIG_START(mpcb081_device::device_add_mconfig)
	MCFG_SCREEN_ADD("screen0", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GT_PIXCLOCK, 1664 + 391 + GT_X_DELTA, 391 + GT_X_DELTA, 1664 + 391 + GT_X_DELTA, 1248 + 74, 74, 1248 + 74)
	MCFG_SCREEN_UPDATE_DEVICE("", mpcb081_device, screen_update0)
	MCFG_DEVICE_ADD("ramdac0", BT459, 0)
	MCFG_DEVICE_ADD("bpu0", DP8510, 0)
MACHINE_CONFIG_END

gt_device_base::gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sr_card_device_base(mconfig, type, tag, owner, clock)
{
}

single_gt_device_base::single_gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: gt_device_base(mconfig, type, tag, owner, clock)
	, m_screen{ { { *this, "ramdac0" },{ *this, "bpu0" } } }
{
}

dual_gt_device_base::dual_gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: gt_device_base(mconfig, type, tag, owner, clock)
	, m_screen{ { { *this, "ramdac0" },{ *this, "bpu0" } },{ { *this, "ramdac1" },{ *this, "bpu1" } } }
{
}

mpcb963_device::mpcb963_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: single_gt_device_base(mconfig, MPCB963, tag, owner, clock)
{
}

mpcba79_device::mpcba79_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dual_gt_device_base(mconfig, MPCBA79, tag, owner, clock)
{
}

mpcb070_device::mpcb070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: single_gt_device_base(mconfig, MPCB070, tag, owner, clock)
{
}

mpcb071_device::mpcb071_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dual_gt_device_base(mconfig, MPCB071, tag, owner, clock)
{
}

mpcb081_device::mpcb081_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: single_gt_device_base(mconfig, MPCB081, tag, owner, clock)
{
}

const tiny_rom_entry *mpcb963_device::device_rom_region() const
{
	return ROM_NAME(mpcb963);
}

const tiny_rom_entry *mpcba79_device::device_rom_region() const
{
	return ROM_NAME(mpcba79);
}

const tiny_rom_entry *mpcb070_device::device_rom_region() const
{
	return ROM_NAME(mpcb070);
}

const tiny_rom_entry *mpcb071_device::device_rom_region() const
{
	return ROM_NAME(mpcb071);
}

const tiny_rom_entry *mpcb081_device::device_rom_region() const
{
	return ROM_NAME(mpcb081);
}

void gt_device_base::device_start()
{
	sr_card_device_base::device_start();

	save_item(NAME(m_control));

	for (int i = 0; i < get_screen_count(); i++)
	{
		gt_screen_t &screen = get_screen(i);

		// FIXME: handle different buffer sizes
		screen.buffer.reset(new u8[GT_BUFFER_SIZE * 2]);
		screen.mask.reset(new u8[GT_BUFFER_SIZE * 2]);

		save_pointer(screen.buffer.get(), util::string_format("buffer%i", i).c_str(), GT_BUFFER_SIZE * 2);
		save_pointer(screen.mask.get(), util::string_format("mask%i", i).c_str(), GT_BUFFER_SIZE * 2);
	}

	// allocate timers
	m_blit_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt_device_base::blit), this));
	m_line_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt_device_base::line), this));
	m_done_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt_device_base::done), this));

	// FIXME: 76Hz boards
	m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt_device_base::vblank), this));

	// TODO: turn this off for now because it's not right
	//m_vblank_timer->adjust(attotime::from_hz(60), ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(gt_device_base::vblank)
{
	// restart timer
	if (param == ASSERT_LINE)
	{
		m_control |= GFX_VERT_BLNK;
		m_vblank_timer->adjust(attotime::from_msec(5), CLEAR_LINE);

		irq(param);
	}
	else
	{
		m_control &= ~GFX_VERT_BLNK;
		m_vblank_timer->adjust(attotime::attotime::from_hz(60), ASSERT_LINE);
	}
}

WRITE32_MEMBER(gt_device_base::control_w)
{
	LOG("control_w 0x%08x\n", data);
	if (data & GFX_BSGA_RST)
	{
		// set graphics busy and schedule a reset
		m_control |= GFX_GRPHCS_BUSY;

		// reset the bitblt fifo pointers
		for (int i = 0; i < get_screen_count(); i++)
			get_screen(i).bpu->reset();

		m_done_timer->adjust(attotime::from_msec(10), GFX_GRPHCS_BUSY);
	}

	// pass direction to bpu
	for (int i = 0; i < get_screen_count(); i++)
		get_screen(i).bpu->barrel_input_select((data & GFX_BLIT_DIR) ? ASSERT_LINE : CLEAR_LINE);

	m_control = data;
}

WRITE32_MEMBER(single_gt_device_base::blit_control_w)
{
	m_screen[0].bpu->control_w(
		((data & BLIT0_CONTROL_FS) >> 4) << 12 |
		((data & BLIT0_CONTROL_SN) >> 12) << 8 |
		((data & BLIT0_CONTROL_LM) >> 20) << 4 |
		((data & BLIT0_CONTROL_RM) >> 28) << 0);
}

WRITE32_MEMBER(dual_gt_device_base::blit_control_w)
{
	// TODO: find a test case with blits to second screen and verify
	m_screen[0].bpu->control_w(
		((data & BLIT0_CONTROL_FS) >> 4) << 12 |
		((data & BLIT0_CONTROL_SN) >> 12) << 8 |
		((data & BLIT0_CONTROL_LM) >> 20) << 4 |
		((data & BLIT0_CONTROL_RM) >> 28) << 0);

	m_screen[1].bpu->control_w(
		((data & BLIT1_CONTROL_FS) >> 0) << 12 |
		((data & BLIT1_CONTROL_SN) >> 8) << 8 |
		((data & BLIT1_CONTROL_LM) >> 16) << 4 |
		((data & BLIT1_CONTROL_RM) >> 24) << 0);
}

void gt_device_base::bsga_clip_status(s16 x, s16 y)
{
	// compute Cohen-Sutherland clipping outcode
	LOG("bsga_clip_status previous 0x%04x\n", m_bsga_status);
	m_bsga_status &= STATUS_CLIP1_MASK;
	m_bsga_status <<= 4;

	LOG("bsga_clip_status shifted 0x%04x\n", m_bsga_status);
	LOG("bsga_clip_status (%d,%d) (%d,%d,%d,%d)\n", x, y, (s16)m_bsga_xmin, (s16)m_bsga_xmax, (s16)m_bsga_ymin, (s16)m_bsga_ymax);

	if ((s16)m_bsga_xmin < (s16)m_bsga_xmax && (s16)m_bsga_ymin < (s16)m_bsga_ymax)
	{
		// clip x coordinate
		if (x < (s16)m_bsga_xmin)
			m_bsga_status |= STATUS_CLIP1_XMIN;
		if (x > (s16)m_bsga_xmax)
			m_bsga_status |= STATUS_CLIP1_XMAX;

		// clip y coordinate
		if (y < (s16)m_bsga_ymin)
			m_bsga_status |= STATUS_CLIP1_YMIN;
		if (y > (s16)m_bsga_ymax)
			m_bsga_status |= STATUS_CLIP1_YMAX;

		if (m_bsga_status & (STATUS_CLIP1_MASK | STATUS_CLIP0_MASK))
			m_bsga_status |= STATUS_CLIP_ANY;
	}
	else
		m_bsga_status |= STATUS_CLIP1_MASK;

	if (((m_bsga_status & STATUS_CLIP0_MASK) >> 4) & (m_bsga_status & STATUS_CLIP1_MASK))
		m_bsga_status |= STATUS_CLIP_BOTH;

	LOG("bsga_clip_status result 0x%04x\n", m_bsga_status);
}

WRITE32_MEMBER(gt_device_base::ri_xfer_w)
{
	LOG("ri_xfer_w 0x%08x mem_mask 0x%08x (%s)\n", data, mem_mask, machine().describe_context());

	// initiate ri line draw
	const gt_screen_t &gt_screen = active_screen();

	int address = m_ri_initial_address;
	int error = m_ri_initial_error;

	for (int i = 0; i < m_ri_stop_count; i++)
	{
		// FIXME: how do we know which buffer to write?
		if (i || m_control & GFX_DRAW_FIRST)
			write_vram(gt_screen, address, m_plane_data);

		if (error >= 0)
		{
			address += m_ri_address_both;
			error -= m_ri_error_both;
		}
		else
		{
			address += m_ri_address_major;
			error += m_ri_error_major;
		}
	}
}

WRITE32_MEMBER(gt_device_base::bsga_xin1yin1_w)
{
	m_bsga_xin1 = (m_bsga_xin1 & ~(mem_mask >> 0)) | ((data & mem_mask) >> 0);
	m_bsga_yin1 = (m_bsga_yin1 & ~(mem_mask >> 16)) | ((data & mem_mask) >> 16);

	LOG("bsga_xin1yin1_w data 0x%08x mem_mask 0x%08x xin1 0x%04x yin1 0x%04x\n", data, mem_mask, m_bsga_xin1, m_bsga_yin1);
	bsga_clip_status(m_bsga_xin1, m_bsga_yin1);

	m_bsga_xin = m_bsga_xin1;
	m_bsga_yin = m_bsga_yin1;

	// FIXME: always xin1, or some kind of counter?
	m_bsga_tmp = m_bsga_xin1;
}

WRITE32_MEMBER(gt_device_base::bsga_xin2yin2_w)
{
	m_bsga_xin2 = (m_bsga_xin2 & ~(mem_mask >> 0)) | ((data & mem_mask) >> 0);
	m_bsga_yin2 = (m_bsga_yin2 & ~(mem_mask >> 16)) | ((data & mem_mask) >> 16);

	LOG("bsga_xin2yin2_w data 0x%08x mem_mask 0x%08x xin2 0x%04x yin2 0x%04x control 0x%04x\n", data, mem_mask, m_bsga_xin2, m_bsga_yin2, m_control);

	// set busy status
	m_control |= GFX_GRPHCS_BUSY;

	// compute clipping status
	bsga_clip_status(m_bsga_xin2, m_bsga_yin2);

	// trigger line drawing
	if (GT_DIAG)
		m_line_timer->adjust(attotime::zero);
	else
		line(nullptr, 0);
}

WRITE16_MEMBER(gt_device_base::bsga_yin2_w)
{
	COMBINE_DATA(&m_bsga_yin2);

	// set busy status
	m_control |= GFX_GRPHCS_BUSY;

	// compute clipping status
	bsga_clip_status(m_bsga_xin2, m_bsga_yin2);

	// trigger line drawing
	if (GT_DIAG)
		m_line_timer->adjust(attotime::zero);
	else
		line(nullptr, 0);
}

READ16_MEMBER(gt_device_base::bsga_status_r)
{
	LOG("bsga_status_r 0x%04x (%s)\n", m_bsga_status, machine().describe_context());

	return m_bsga_status;
}

WRITE32_MEMBER(gt_device_base::bsga_float_w)
{
	// TODO: when we figure out exactly what this is supposed to do, convert it
	// to use softfloat instead.

	const u8 exponent = (data & 0x7f800000) >> 23;
	const int shift = 150 - exponent;
	const u32 mantissa = (data & 0x7fffff) | 0x800000;
	const bool sign = data & 0x80000000;

	LOG("bsga_float_w data 0x%08x value %f exponent 0x%02x shift %d mantissa 0x%08x mantissa sign %d\n", data, u2f(data), exponent, shift, mantissa, sign);

	m_bsga_status &= ~STATUS_FLOAT_OFLOW;

	if (shift > 0)
	{
		if (shift > 23)
			m_bsga_xin = 0;
		else
			m_bsga_xin = mantissa >> shift;

		if (sign)
			m_bsga_xin = -m_bsga_xin;

		// TODO: conditional on something?
		if (shift <= 8)
			m_bsga_status |= STATUS_FLOAT_OFLOW;
	}
	else
	{
		m_bsga_xin = 0;
		m_bsga_status |= STATUS_FLOAT_OFLOW;
	}

	LOG("bsga_float_w result 0x%04x overflow %s\n", m_bsga_xin, m_bsga_status & STATUS_FLOAT_OFLOW ? "set" : "clear");
}

WRITE16_MEMBER(gt_device_base::blit_width_w)
{
	// writing to blit width starts blit operation
	LOG("blit_width_w 0x%04x (%s)\n", data, machine().describe_context());

	COMBINE_DATA(&m_blit_width);

	// set busy status and schedule blit
	m_control |= (GFX_GRPHCS_BUSY | GFX_BLIT_BUSY);

	// trigger blit operation
	if (GT_DIAG)
		m_blit_timer->adjust(attotime::zero);
	else
		blit(nullptr, 0);
}

/*
 * Pixel data is stored as 8bpp pixels grouped in 32 bit aligned words, but is
 * processed by the BPU 16 bits at a time. This is accomplished by driving the
 * BPU through the address range twice, the first time processing the high 4
 * bits of each pixel (packing 4 pixels into 16 bits), and the second time the
 * low 4 bits of each pixel. The cycles must be run sequentially to ensure the
 * barrel input latch in the bpu contains the right data to make barrel shift
 * operations work correctly.
 *
 * Working theories which are not yet fully tested:
 *
 *   - GFX_DRAW_FIRST indicates we need to load the barrel input latch from the
 *     first word of source data.
 *   - GFX_MASK_ENA indicates we need to read masked source data as plane data
 *     when masked.
 */
#define HI(b0,b1,b2,b3) ((((b0) & 0xf0) << 8) | (((b1) & 0xf0) << 4) | (((b2) & 0xf0) << 0) | (((b3) & 0xf0) >> 4))
#define LO(b0,b1,b2,b3) ((((b0) & 0x0f) << 12) | (((b1) & 0x0f) << 8) | (((b2) & 0x0f) << 4) | (((b3) & 0x0f) << 0))

TIMER_CALLBACK_MEMBER(gt_device_base::blit)
{
	LOGMASKED(LOG_BLIT, "blit bsga_control 0x%08x src 0x%08x dst 0x%08x width 0x%04x (count %d, %s)\n",
		m_control, m_blit_src_address, m_blit_dst_address, m_blit_width,
		m_blit_width >> 2, (m_control & GFX_BLIT_DIR) ? "decrementing" : "incrementing");

	const gt_screen_t &screen = active_screen();

	const int delta = (m_control & GFX_BLIT_DIR) ? -4 : 4;
	const int count = m_blit_width >> 2;

	// first cycle
	u32 src_address = m_blit_src_address & ~0x3;
	u32 dst_address = m_blit_dst_address & ~0x3;

	// load barrel input latch
	if (!(m_control & GFX_DRAW_FIRST))
	{
		if (!(m_control & GFX_DATA_SEL))
		{
			const u8 *src_data = &screen.buffer[src_address];
			const u8 *src_mask = &screen.mask[src_address];

			if (m_control & GFX_MASK_ENA)
				screen.bpu->source_w(HI(
					src_mask[0] ? m_plane_data : src_data[0],
					src_mask[1] ? m_plane_data : src_data[1],
					src_mask[2] ? m_plane_data : src_data[2],
					src_mask[3] ? m_plane_data : src_data[3]), false);
			else
				screen.bpu->source_w(HI(src_data[0], src_data[1], src_data[2], src_data[3]), false);

			src_address += delta;
		}
		else
			screen.bpu->source_w(HI(m_plane_data, m_plane_data, m_plane_data, m_plane_data), false);
	}

	// bitblt address loop
	for (int word = 0; word < count; word++)
	{
		// drive left and right mask enables
		screen.bpu->left_mask_enable(word == ((m_control & GFX_BLIT_DIR) ? count - 1 : 0));
		screen.bpu->right_mask_enable(word == ((m_control & GFX_BLIT_DIR) ? 0 : count - 1));

		// load bpu source
		if (!(m_control & GFX_DATA_SEL))
		{
			const u8 *src_data = &screen.buffer[src_address];
			const u8 *src_mask = &screen.mask[src_address];

			LOGMASKED(LOG_BLIT, "blit src data %3d address 0x%08x %02x %02x %02x %02x\n",
				word, src_address, src_data[0], src_data[1], src_data[2], src_data[3]);

			if (m_control & GFX_MASK_ENA)
			{
				LOGMASKED(LOG_BLIT, "blit src mask %3d address 0x%08x %02x %02x %02x %02x\n",
					word, src_address, src_mask[0], src_mask[1], src_mask[2], src_mask[3]);

				screen.bpu->source_w(HI(
					src_mask[0] ? m_plane_data : src_data[0],
					src_mask[1] ? m_plane_data : src_data[1],
					src_mask[2] ? m_plane_data : src_data[2],
					src_mask[3] ? m_plane_data : src_data[3]));
			}
			else
				screen.bpu->source_w(HI(src_data[0], src_data[1], src_data[2], src_data[3]));

			src_address += delta;
		}
		else
			screen.bpu->source_w(HI(m_plane_data, m_plane_data, m_plane_data, m_plane_data));

		// load bpu destination
		{
			const u8 *dst_data = (m_control & GFX_MASK_SEL) ? &screen.mask[dst_address] : &screen.buffer[dst_address];

			LOGMASKED(LOG_BLIT, "blit dst data %3d address 0x%08x %02x %02x %02x %02x\n",
				word, dst_address, dst_data[0], dst_data[1], dst_data[2], dst_data[3]);

			screen.bpu->destination_w(HI(dst_data[0], dst_data[1], dst_data[2], dst_data[3]));
		}

		// fetch bpu output
		const u16 output = screen.bpu->output_r();

		if (!(m_control & GFX_MASK_SEL))
		{
			// write output to pixel ram
			u8 *out_data = &screen.buffer[dst_address];
			const u8 *out_mask = &screen.mask[dst_address];

			if (!(m_control & GFX_MASK_ENA) || out_mask[0] == 0)
				out_data[0] = (out_data[0] & ~(0xf0 | m_plane_enable)) | (((output & 0xf000) >> 8) & m_plane_enable);
			if (!(m_control & GFX_MASK_ENA) || out_mask[1] == 0)
				out_data[1] = (out_data[1] & ~(0xf0 | m_plane_enable)) | (((output & 0x0f00) >> 4) & m_plane_enable);
			if (!(m_control & GFX_MASK_ENA) || out_mask[2] == 0)
				out_data[2] = (out_data[2] & ~(0xf0 | m_plane_enable)) | (((output & 0x00f0) >> 0) & m_plane_enable);
			if (!(m_control & GFX_MASK_ENA) || out_mask[3] == 0)
				out_data[3] = (out_data[3] & ~(0xf0 | m_plane_enable)) | (((output & 0x000f) << 4) & m_plane_enable);
		}
		else
		{
			// write output to mask ram
			u8 *out_data = &screen.mask[dst_address];

			out_data[0] = output & 0x8000 ? 0x80 : 0x00;
			out_data[1] = output & 0x0800 ? 0x80 : 0x00;
			out_data[2] = output & 0x0080 ? 0x80 : 0x00;
			out_data[3] = output & 0x0008 ? 0x80 : 0x00;

			LOGMASKED(LOG_BLIT, "blit out mask %3d address 0x%08x %02x %02x %02x %02x\n",
				word, dst_address, out_data[0], out_data[1], out_data[2], out_data[3]);
		}

		dst_address += delta;
	}

	// second cycle
	src_address = m_blit_src_address & ~0x3;
	dst_address = m_blit_dst_address & ~0x3;

	// load barrel input latch
	if (!(m_control & GFX_DRAW_FIRST))
	{
		if (!(m_control & GFX_DATA_SEL))
		{
			const u8 *src_data = &screen.buffer[src_address];
			const u8 *src_mask = &screen.mask[src_address];

			if (m_control & GFX_MASK_ENA)
				screen.bpu->source_w(LO(
					src_mask[0] ? m_plane_data : src_data[0],
					src_mask[1] ? m_plane_data : src_data[1],
					src_mask[2] ? m_plane_data : src_data[2],
					src_mask[3] ? m_plane_data : src_data[3]), false);
			else
				screen.bpu->source_w(LO(src_data[0], src_data[1], src_data[2], src_data[3]), false);
		}
		else
			screen.bpu->source_w(LO(m_plane_data, m_plane_data, m_plane_data, m_plane_data), false);

		src_address += delta;
	}

	// bitblt address loop
	for (int word = 0; word < count; word++)
	{
		// drive left and right mask enables
		screen.bpu->left_mask_enable(word == ((m_control & GFX_BLIT_DIR) ? count - 1 : 0));
		screen.bpu->right_mask_enable(word == ((m_control & GFX_BLIT_DIR) ? 0 : count - 1));

		// load bpu source
		if (!(m_control & GFX_DATA_SEL))
		{
			const u8 *src_data = &screen.buffer[src_address];
			const u8 *src_mask = &screen.mask[src_address];

			if (m_control & GFX_MASK_ENA)
			{
				screen.bpu->source_w(LO(
					src_mask[0] ? m_plane_data : src_data[0],
					src_mask[1] ? m_plane_data : src_data[1],
					src_mask[2] ? m_plane_data : src_data[2],
					src_mask[3] ? m_plane_data : src_data[3]));
			}
			else
				screen.bpu->source_w(LO(src_data[0], src_data[1], src_data[2], src_data[3]));

			src_address += delta;
		}
		else
			screen.bpu->source_w(LO(m_plane_data, m_plane_data, m_plane_data, m_plane_data));

		// load bpu destination
		{
			const u8 *dst_data = (m_control & GFX_MASK_SEL) ? &screen.mask[dst_address] : &screen.buffer[dst_address];

			screen.bpu->destination_w(LO(dst_data[0], dst_data[1], dst_data[2], dst_data[3]));
		}

		// fetch bpu output
		const u16 output = screen.bpu->output_r();

		if (!(m_control & GFX_MASK_SEL))
		{
			// write output to pixel ram
			u8 *out_data = &screen.buffer[dst_address];
			const u8 *out_mask = &screen.mask[dst_address];

			if (!(m_control & GFX_MASK_ENA) || out_mask[0] == 0)
				out_data[0] = (out_data[0] & ~(0x0f | m_plane_enable)) | (((output & 0xf000) >> 12) & m_plane_enable);
			if (!(m_control & GFX_MASK_ENA) || out_mask[1] == 0)
				out_data[1] = (out_data[1] & ~(0x0f | m_plane_enable)) | (((output & 0x0f00) >> 8) & m_plane_enable);
			if (!(m_control & GFX_MASK_ENA) || out_mask[2] == 0)
				out_data[2] = (out_data[2] & ~(0x0f | m_plane_enable)) | (((output & 0x00f0) >> 4) & m_plane_enable);
			if (!(m_control & GFX_MASK_ENA) || out_mask[3] == 0)
				out_data[3] = (out_data[3] & ~(0x0f | m_plane_enable)) | (((output & 0x000f) >> 0) & m_plane_enable);

			LOGMASKED(LOG_BLIT, "blit dst mask %3d address 0x%08x %02x %02x %02x %02x\n",
				word, dst_address, out_mask[0], out_mask[1], out_mask[2], out_mask[3]);
			LOGMASKED(LOG_BLIT, "blit out data %3d address 0x%08x %02x %02x %02x %02x\n",
				word, dst_address, out_data[0], out_data[1], out_data[2], out_data[3]);
		}

		dst_address += delta;
	}

	// clear mask enable lines
	screen.bpu->left_mask_enable(CLEAR_LINE);
	screen.bpu->right_mask_enable(CLEAR_LINE);

	// complete with delay
	if (GT_DIAG)
		m_done_timer->adjust(attotime::from_msec(10), GFX_GRPHCS_BUSY | GFX_BLIT_BUSY);
	else
		done(nullptr, GFX_GRPHCS_BUSY | GFX_BLIT_BUSY);
}

TIMER_CALLBACK_MEMBER(gt_device_base::line)
{
	// draw a clipped line

	// FIXME: fix clipping to use >= min and < max (don't subtract 1)
	kuzmin_clip(m_bsga_xin1, m_bsga_yin1, m_bsga_xin2, m_bsga_yin2, m_bsga_xmin, m_bsga_ymin, m_bsga_xmax - 1, m_bsga_ymax - 1);

	// point #2 becomes point #1
	m_bsga_xin1 = m_bsga_xin2;
	m_bsga_yin1 = m_bsga_yin2;

	m_bsga_xin = m_bsga_xin1;
	m_bsga_yin = m_bsga_yin1;

	// complete with delay
	if (GT_DIAG)
		m_done_timer->adjust(attotime::from_nsec(100), GFX_GRPHCS_BUSY);
	else
		done(nullptr, GFX_GRPHCS_BUSY);
}

TIMER_CALLBACK_MEMBER(gt_device_base::done)
{
	m_control &= ~(u32)param;
}

WRITE8_MEMBER(gt_device_base::plane_enable_w)
{
	LOG("plane enable 0x%02x\n", data);

	if (!(m_control & GFX_GRPHCS_BUSY))
		m_plane_enable = data;
}

WRITE8_MEMBER(gt_device_base::plane_data_w)
{
	LOG("plane data 0x%02x\n", data);

	if (!(m_control & GFX_GRPHCS_BUSY))
		m_plane_data = data;
}

#define U32(b0,b1,b2,b3) (((b0) << 24) | ((b1) << 16) | ((b2) << 8) | ((b3) << 0))
u32 gt_device_base::buffer_read(const gt_screen_t &screen, const offs_t offset) const
{
	const u32 src_address = offset << 2;

	const u8 *src = &screen.buffer[src_address];
	const u8 *mask = &screen.mask[src_address];

	if (m_control & GFX_MASK_SEL)
		return U32(mask[3], mask[2], mask[1], mask[0]);
	else if (m_control & GFX_DATA_SEL)
		return U32(m_plane_data, m_plane_data, m_plane_data, m_plane_data);
	else if (m_control & GFX_MASK_ENA)
		return U32(
			mask[3] ? m_plane_data : src[3],
			mask[2] ? m_plane_data : src[2],
			mask[1] ? m_plane_data : src[1],
			mask[0] ? m_plane_data : src[0]);
	else
		return U32(src[3], src[2], src[1], src[0]);
}

void gt_device_base::buffer_write(const gt_screen_t &screen, const offs_t offset, const u32 data, const u32 mask)
{
	const u32 dst_address = offset << 2;

	u8 out_data[] = {
		u8(((m_control & GFX_DATA_SEL) ? m_plane_data : (data >> 0)) & m_plane_enable),
		u8(((m_control & GFX_DATA_SEL) ? m_plane_data : (data >> 8)) & m_plane_enable),
		u8(((m_control & GFX_DATA_SEL) ? m_plane_data : (data >> 16)) & m_plane_enable),
		u8(((m_control & GFX_DATA_SEL) ? m_plane_data : (data >> 24)) & m_plane_enable)
	};
	u32 out_mask = mask;

	if (m_control & GFX_RMW_MD)
	{
		// read/modify/write mode, use bpu to compute result
		const u8 *dst_data = &screen.buffer[dst_address];

		screen.bpu->source_w(HI(out_data[0], out_data[1], out_data[2], out_data[3]));
		screen.bpu->destination_w(HI(dst_data[0], dst_data[1], dst_data[2], dst_data[3]));

		const u16 out_hi = screen.bpu->output_r();

		screen.bpu->source_w(LO(out_data[0], out_data[1], out_data[2], out_data[3]));
		screen.bpu->destination_w(LO(dst_data[0], dst_data[1], dst_data[2], dst_data[3]));

		const u16 out_lo = screen.bpu->output_r();

		out_data[0] = ((out_hi & 0x000f) << 28 | (out_lo & 0x000f) << 24);
		out_data[1] = ((out_hi & 0x00f0) << 16 | (out_lo & 0x00f0) << 12);
		out_data[2] = ((out_hi & 0x0f00) << 4 | (out_lo & 0x0f00) >> 0);
		out_data[3] = ((out_hi & 0xf000) >> 8 | (out_lo & 0xf000) >> 12);
	}
	else if (m_control & GFX_DATA_SEL)
	{
		// bottom four bits control writing plane data to each of 4 bytes in the word
		out_mask = (data & 0x1) ? (out_mask | 0x000000ff) : (out_mask & ~0x000000ff);
		out_mask = (data & 0x2) ? (out_mask | 0x0000ff00) : (out_mask & ~0x0000ff00);
		out_mask = (data & 0x4) ? (out_mask | 0x00ff0000) : (out_mask & ~0x00ff0000);
		out_mask = (data & 0x8) ? (out_mask | 0xff000000) : (out_mask & ~0xff000000);
	}

	if (m_control & GFX_MASK_SEL)
	{
		// write to mask
		u8 *dst_data = &screen.mask[dst_address];

		if (out_mask & 0x000000ff) dst_data[0] = out_data[0] & 0x80;
		if (out_mask & 0x0000ff00) dst_data[1] = out_data[1] & 0x80;
		if (out_mask & 0x00ff0000) dst_data[2] = out_data[2] & 0x80;
		if (out_mask & 0xff000000) dst_data[3] = out_data[3] & 0x80;
	}
	else if (m_control & GFX_MASK_ENA)
	{
		// masked write to buffer
		u8 *dst_mask = &screen.mask[dst_address];
		u8 *dst_data = &screen.buffer[dst_address];

		if ((out_mask & 0x000000ff) && (dst_mask[0] == 0)) dst_data[0] = out_data[0];
		if ((out_mask & 0x0000ff00) && (dst_mask[1] == 0)) dst_data[1] = out_data[1];
		if ((out_mask & 0x00ff0000) && (dst_mask[2] == 0)) dst_data[2] = out_data[2];
		if ((out_mask & 0xff000000) && (dst_mask[3] == 0)) dst_data[3] = out_data[3];
	}
	else
	{
		// unmasked write to buffer
		u8 *dst_data = &screen.buffer[dst_address];

		if (out_mask & 0x000000ff) dst_data[0] = out_data[0];
		if (out_mask & 0x0000ff00) dst_data[1] = out_data[1];
		if (out_mask & 0x00ff0000) dst_data[2] = out_data[2];
		if (out_mask & 0xff000000) dst_data[3] = out_data[3];
	}
}

u32 single_gt_device_base::screen_update0(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const gt_screen_t &gt_screen = m_screen[0];

	gt_screen.ramdac->screen_update(screen, bitmap, cliprect, m_control & GFX_SCREEN0_DISP_BUF1 ? &gt_screen.buffer[GT_BUFFER_SIZE] : &gt_screen.buffer[0]);

	return 0;
}

u32 dual_gt_device_base::screen_update0(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const gt_screen_t &gt_screen = m_screen[0];

	gt_screen.ramdac->screen_update(screen, bitmap, cliprect, m_control & GFX_SCREEN0_DISP_BUF1 ? &gt_screen.buffer[GT_BUFFER_SIZE] : &gt_screen.buffer[0]);

	return 0;
}

u32 dual_gt_device_base::screen_update1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const gt_screen_t &gt_screen = m_screen[1];

	gt_screen.ramdac->screen_update(screen, bitmap, cliprect, m_control & GFX_SCREEN1_DISP_BUF1 ? &gt_screen.buffer[GT_BUFFER_SIZE] : &gt_screen.buffer[0]);

	return 0;
}

/*
 * This function implements the technique described in "Bresenham's Line
 * Generation Algorithm with Built-in Clipping", by Yevgeny P. Kuzmin. It has
 * been adapted to take advantage of the Cohen-Sutherland clipping outcodes
 * computed in the BSGA to reject trivial cases.
 */
bool gt_device_base::kuzmin_clip(s16 x1, s16 y1, s16 x2, s16 y2, s16 clip_xmin, s16 clip_ymin, s16 clip_xmax, s16 clip_ymax)
{
	LOG("kuzmin_clip line (%d,%d)-(%d,%d) clip (%d,%d,%d,%d) plane_data 0x%02x plane_enable 0x%02x\n",
		x1, y1, x2, y2, clip_xmin, clip_ymin, clip_xmax, clip_ymax,
		m_plane_data, m_plane_enable);

	// all trivial cases are handled by STATUS_CLIP_BOTH
	if (m_bsga_status & STATUS_CLIP_BOTH)
	{
		LOG("kuzmin_clip Cohen-Sutherland reject\n");

		return false;
	}

	// horizontal line case
	if (y1 == y2)
	{
		LOG("kuzmin_clip horizontal line case\n");
		if (x1 <= x2)
		{
			// left to right
			x1 = std::max(x1, clip_xmin);
			x2 = std::min(x2, clip_xmax);

			bresenham_line(x1, y1, 1, 0, x2 - x1 + 1, 0, 0, 0, true);
		}
		else
		{
			// right to left
			x2 = std::max(x2, clip_xmin);
			x1 = std::min(x1, clip_xmax);

			bresenham_line(x1, y1, -1, 0, x1 - x2 + 1, 0, 0, 0, true);
		}

		return true;
	}

	// vertical line case
	if (x1 == x2)
	{
		LOG("kuzmin_clip vertical line case\n");
		if (y1 <= y2)
		{
			// top to bottom
			y1 = std::max(y1, clip_ymin);
			y2 = std::min(y2, clip_ymax);

			bresenham_line(y1, x1, 1, 0, y2 - y1 + 1, 0, 0, 0, false);
		}
		else
		{
			// bottom to top
			y2 = std::max(y2, clip_ymin);
			y1 = std::min(y1, clip_ymax);

			bresenham_line(y1, x1, -1, 0, y1 - y2 + 1, 0, 0, 0, false);
		}

		return true;
	}

	// TODO: try to eliminate sign inversion
	int sign_x = 1;
	if (x1 > x2)
	{
		// invert sign, invert again before output
		x1 = -x1;
		x2 = -x2;
		clip_xmin = -clip_xmax;
		clip_xmax = -clip_xmin;

		sign_x = -1;
	}

	int sign_y = 1;
	if (y1 > y2)
	{
		// invert sign, invert again before output
		y1 = -y1;
		y2 = -y2;
		clip_ymin = -clip_ymax;
		clip_ymax = -clip_ymin;

		sign_y = -1;
	}

	const s16 delta_x = x2 - x1;
	const s16 delta_y = y2 - y1;

	int delta_x_step = 2 * delta_x;
	int delta_y_step = 2 * delta_y;

	// output coordinates
	s16 x_pos = x1;
	s16 x_pos_end = x2;
	s16 y_pos = y1;
	s16 y_pos_end = y2;

	if (delta_x >= delta_y)
	{
		LOG("kuzmin_clip mostly horizontal line case\n");

		// mostly horizontal case
		s16 error = delta_y_step - delta_x;

		// skip clipping if not required
		if (m_bsga_status & STATUS_CLIP_ANY)
		{
			bool set_exit = false;

			// line starts above clip boundary
			if (y1 < clip_ymin)
			{
				// compute x offset at intersection of top clip boundary
				div_t div = std::div(delta_x_step * (clip_ymin - y1) - delta_x, delta_y_step);

				x_pos += div.quot;

				// line does not intersect clip boundary
				if (x_pos > clip_xmax)
				{
					LOG("kuzmin_clip line wholly left or right of clipping boundary\n");

					return 0;
				}

				// line intersects top clip boundary
				if (x_pos >= clip_xmin)
				{
					y_pos = clip_ymin;
					error -= div.rem + delta_x;

					if (div.rem > 0)
					{
						x_pos += 1;
						error += delta_y_step;
					}

					set_exit = true;
				}
			}

			// line starts left of clip boundary
			if (!set_exit && (x1 < clip_xmin))
			{
				// compute y offset at intersection of left clip boundary
				div_t div = std::div(delta_y_step * (clip_xmin - x1), delta_x_step);

				y_pos += div.quot;

				// line does not intersect clip boundary
				if ((y_pos > clip_ymax) || ((y_pos == clip_ymax && div.rem >= delta_x)))
				{
					LOG("kuzmin_clip line wholly above or below clipping boundary\n");

					return 0;
				}

				// line intersects left clip boundary
				if (y_pos >= clip_ymin)
				{
					x_pos = clip_xmin;
					error += div.rem;

					if (div.rem >= delta_x)
					{
						y_pos += 1;
						error -= delta_x_step;
					}
				}
			}

			// clip the end point to the bottom clip boundary
			if (y2 > clip_ymax)
			{
				// compute x offset at intersection of bottom clip boundary
				div_t div = std::div(delta_x_step * (clip_ymax - y1) + delta_x, delta_y_step);

				x_pos_end = x1 + div.quot;

				if (div.rem == 0)
					x_pos_end -= 1;
			}

			x_pos_end = std::min(x_pos_end, clip_xmax);

		}

		// revert sign inversion
		if (sign_y == -1)
			y_pos = -y_pos;

		if (sign_x == -1)
		{
			x_pos = -x_pos;
			x_pos_end = -x_pos_end;
		}

		delta_x_step -= delta_y_step;

		LOG("kuzmin_clip clipped line (%d,%d)-(%d,%d)\n", x_pos, y_pos, x_pos_end, y_pos_end);

		// draw the line
		bresenham_line(x_pos, y_pos, sign_x, sign_y, abs(x_pos_end - x_pos) + 1, error, delta_x_step, delta_y_step, true);
	}
	else
	{
		LOG("kuzmin_clip mostly vertical line case\n");

		// mostly vertical case (same as previous with swapped x/y)
		s16 error = delta_x_step - delta_y;

		// skip clipping if not required
		if (m_bsga_status & STATUS_CLIP_ANY)
		{
			bool set_exit = false;

			// line starts left of the clip window.
			if (x1 < clip_xmin)
			{
				// compute y offset at intersection of left clip boundary
				div_t div = std::div(2 * delta_y * (clip_xmin - x1) - delta_y, delta_x_step);

				y_pos += div.quot;

				// line does not intersect clip boundary
				if (y_pos > clip_ymax)
				{
					LOG("kuzmin_clip line wholly left or right of clipping boundary\n");

					return 0;
				}

				// line intersects left clip boundary
				if (y_pos >= clip_ymin)
				{
					x_pos = clip_xmin;
					error -= div.rem + delta_y;

					if (div.rem > 0)
					{
						y_pos += 1;
						error += delta_x_step;
					}

					set_exit = true;
				}
			}

			// line starts above clip boundary
			if (!set_exit && (y1 < clip_ymin))
			{
				// compute x offset at intersection of top clip boundary
				div_t div = std::div(delta_x_step * (clip_ymin - y1), delta_y_step);

				x_pos += div.quot;

				// line does not intersect clip boundary
				if ((x_pos > clip_xmax) || ((x_pos == clip_xmax && div.rem >= delta_y)))
				{
					LOG("kuzmin_clip line wholly above or below clipping boundary\n");

					return 0;
				}

				// line intersects top clip boundary
				y_pos = clip_ymin;
				error += div.rem;

				if (div.rem >= delta_y)
				{
					x_pos += 1;
					error -= delta_y_step;
				}
			}

			// clip the end point to the right clip boundary
			if (x2 > clip_xmax)
			{
				// compute y offset at intersection of right clip boundary
				div_t div = std::div(delta_y_step * (clip_xmax - x1) + delta_y, delta_x_step);

				y_pos_end = y1 + div.quot;

				if (div.rem == 0)
					y_pos_end -= 1;
			}

			y_pos_end = std::min(y_pos_end, clip_ymax);
		}

		// revert sign inversion
		if (sign_x == -1)
			x_pos = -x_pos;

		if (sign_y == -1)
		{
			y_pos = -y_pos;
			y_pos_end = -y_pos_end;
		}

		delta_y_step -= delta_x_step;

		LOG("kuzmin_clip clipped line (%d,%d)-(%d,%d)\n", x_pos, y_pos, x_pos_end, y_pos_end);

		// draw the line
		bresenham_line(y_pos, x_pos, sign_y, sign_x, abs(y_pos_end - y_pos) + 1, error, delta_y_step, delta_x_step, false);
	}

	return true;
}

void gt_device_base::bresenham_line(s16 major, s16 minor, s16 major_step, s16 minor_step, int steps, s16 error, s16 error_major, s16 error_minor, bool shallow)
{
	const gt_screen_t &gt_screen = active_screen();

	LOG("bresenham_line begin %d,%d steps %d\n", shallow ? major : minor, shallow ? minor : major, steps);

	for (int i = 0; i < steps; i++)
	{
		// FIXME: which buffer?
		if (i || m_control & GFX_DRAW_FIRST)
			write_vram(gt_screen, shallow ? minor * GT_XPIXELS + major : major * GT_XPIXELS + minor, m_plane_data);

		if (error >= 0)
		{
			minor += minor_step;
			error -= error_major;
		}
		else
			error += error_minor;

		major += major_step;
	}

	LOG("bresenham_line end %d,%d\n", shallow ? major : minor, shallow ? minor : major);
}

// TODO: eliminate this function completely when we fully understand the buffer write pipeline.
void gt_device_base::write_vram(const gt_screen_t &screen, const offs_t offset, const u8 data)
{
	if (false)
	{
		const int shift_amount = ((offset & 0x3) << 3);

		buffer_write(screen, offset >> 2, data << shift_amount, 0xff << shift_amount);
	}
	else
		screen.buffer[offset] = data;
}
