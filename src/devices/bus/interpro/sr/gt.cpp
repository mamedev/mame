// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of GT graphics, also known as Memory Mapped Graphics (MMG), for
 * Intergraph InterPro systems.
 *
 * TODO
 *   - pixel-perfect line drawing to match diagnostics
 *   - RI aliased and anti-aliased line drawing
 *   - highlight plane display
 *   - fifos (no information at this point)
 *   - 76Hz refresh, 2MPix boards
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
 * GT/GT+:
 *       feature[0] & 0x01 ? single : dual
 *       feature[0] & 0x02 ? 1 MPix (1184x884) : 2 MPix (1664x1248)
 *       feature[0] & 0x04 ? gt (memsize 0x00100000/1M) : gtplus (memsize 0x01000000/16M)
 *
 * GTDB:
 *       feature[0] & 0x03: 1=2MPix, default 1MPix
 *       feature[0] & 0x18 ? dual : single
 *       feature[1] & 0x04 ? no burst I/O : burst I/O
 *
 * All types:
 *
 *   1MPix boards: (feature[0] & 0xc0) == 0x80 ? 76Hz (xoff 264, yoff 57) : 60Hz (xoff 296, yoff 34)
 *   2MPix boards: (feature[0] & 0xc0) == 0x80 ? 76Hz (xoff 391, yoff 74) : (feature[0] & 0xc0) == 0x00 ? 60Hz (xoff 407, yoff 48) : 60/76Hz (check control register)
 *
 * GT (PCB963/PCBA79)
 *
 *   Ref   Part                      Function
 *   U13   Bt438KPJ                  Clock generator
 *   U?    83.0208 MHz crystal       Pixel clock
 *   U22   Bt459KG110                RAMDAC
 *   U66   NS DP8510V                Bitblt unit
 *   U67   NS DP8510V                Bitblt unit
 *   U102  Bt459KG110                RAMDAC (not populated on PCB963)
 *   U?    NS S9030                  BSGA ASIC?
 *         SCX6B64ABM
 *         /NU6
 *   U?    CICD91201                 Bus interface ASIC?
 *         TC110G17AT
 *         0078 9037NAS
 *
 *         ?                         256Kx4 Video DRAM? (total 1MiB/2MiB)
 *   U?-U?                           8 parts
 *   U?-U?                           8 parts (not populated on PCB963)
 *
 *         ?                         256Kx4 Fast Page DRAM? (total 128KiB/256KiB)
 *   U?                              1 part
 *   U?                              2 part (not populated on PCB963)
 *
 * GT+ (SMT070/SMT071)
 *
 *   Ref   Part                      Function
 *   U3    Bt438KPJ                  Clock generator
 *   U5    105.5610 MHz crystal      Pixel clock
 *   U6    Bt459KPF110               RAMDAC
 *   U36   NS DP8510V                Bitblt unit
 *   U37   NS DP8510V                Bitblt unit
 *   U58   Bt459KPF110               RAMDAC (unpopulated on SMT070)
 *   U99   12.0 MHz crystal
 *   U104  NS S9336AB                BSGA ASIC?
 *         SCX6B64ABM
 *         /NU6
 *   U156  CICD91201                 Bus interface ASIC?
 *         TC110G17AT
 *         0078 9336NAS
 *
 *         M5M442256AL-8             256Kx4 Video DRAM (total 2MiB/4MiB)
 *   U20-U35                         16 parts
 *   U42-U57                         16 parts  (not populated on SMT070)
 *
 *         M5M44256AL                256Kx4 Fast Page DRAM (total 128KiB/256KiB)
 *   U19                             1 part
 *   U41                             1 part (not populated on SMT070)
 */

#include "emu.h"

#include "gt.h"

#define LOG_GENERAL (1U << 0)
#define LOG_LINE    (1U << 1)
#define LOG_BLIT    (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_LINE | LOG_BLIT)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(MPCB963, mpcb963_device, "mpcb963", "2000 Graphics f/1 1Mp Monitor")
DEFINE_DEVICE_TYPE(MPCBA79, mpcba79_device, "mpcba79", "2000 Graphics f/2 1Mp Monitors")
DEFINE_DEVICE_TYPE(MSMT070, msmt070_device, "msmt070", "2400 Graphics f/1 1Mp Monitor (V-76)")
DEFINE_DEVICE_TYPE(MSMT071, msmt071_device, "msmt071", "2400 Graphics f/2 1Mp Monitors (V-76)")
DEFINE_DEVICE_TYPE(MSMT081, msmt081_device, "msmt081", "2400 Graphics f/1 2Mp Monitor (V-60/76)")
DEFINE_DEVICE_TYPE(MPCBB68, mpcbb68_device, "mpcbb68", "GT II Graphics f/1 1Mp Monitor (V-76)")
DEFINE_DEVICE_TYPE(MPCBB92, mpcbb92_device, "mpcbb92", "GT II Graphics f/1 2Mp Monitor (V-60/76)")

void gt_device_base::map(address_map &map)
{
	map(0x080, 0x08f).m(m_ramdac[0], FUNC(bt459_device::map)).umask32(0x000000ff);
	if (m_ramdac[1].found())
		map(0x090, 0x09f).m(m_ramdac[1], FUNC(bt459_device::map)).umask32(0x000000ff);

	map(0x0a0, 0x0a0).w(FUNC(gt_device_base::contrast_dac_w));
	map(0x0b0, 0x0b3).rw(FUNC(gt_device_base::control_r), FUNC(gt_device_base::control_w));

	map(0x0c0, 0x0c3).w(FUNC(gt_device_base::blit_src_address_w));
	map(0x0c4, 0x0c7).w(FUNC(gt_device_base::blit_dst_address_w));
	map(0x0c8, 0x0c9).w(FUNC(gt_device_base::blit_width_w));

	map(0x0d0, 0x0d3).w(FUNC(gt_device_base::bpu_control_w));
	map(0x0d4, 0x0d4).rw(FUNC(gt_device_base::plane_enable_r), FUNC(gt_device_base::plane_enable_w));
	map(0x0d8, 0x0d8).rw(FUNC(gt_device_base::plane_data_r), FUNC(gt_device_base::plane_data_w));

	map(0x100, 0x101).rw(FUNC(gt_device_base::bsga_width_r), FUNC(gt_device_base::bsga_width_w));
	map(0x102, 0x103).rw(FUNC(gt_device_base::bsga_tmp_r), FUNC(gt_device_base::bsga_tmp_w));

	map(0x104, 0x105).w(FUNC(gt_device_base::bsga_xmin_w));
	map(0x108, 0x109).w(FUNC(gt_device_base::bsga_ymin_w));
	map(0x10c, 0x10d).rw(FUNC(gt_device_base::bsga_xmin_r), FUNC(gt_device_base::bsga_xmin_w));
	map(0x10e, 0x10f).rw(FUNC(gt_device_base::bsga_ymin_r), FUNC(gt_device_base::bsga_ymin_w));

	map(0x110, 0x111).r(FUNC(gt_device_base::bsga_acc0_r));
	map(0x112, 0x113).r(FUNC(gt_device_base::bsga_acc1_r));

	map(0x114, 0x115).w(FUNC(gt_device_base::bsga_xmax_w));
	map(0x118, 0x119).w(FUNC(gt_device_base::bsga_ymax_w));
	map(0x11c, 0x11d).rw(FUNC(gt_device_base::bsga_xmax_r), FUNC(gt_device_base::bsga_xmax_w));
	map(0x11e, 0x11f).rw(FUNC(gt_device_base::bsga_ymax_r), FUNC(gt_device_base::bsga_ymax_w));

	map(0x120, 0x121).r(FUNC(gt_device_base::bsga_src0_r));
	map(0x122, 0x123).r(FUNC(gt_device_base::bsga_src1_r));

	map(0x124, 0x125).w(FUNC(gt_device_base::bsga_xin1_w));
	map(0x128, 0x129).w(FUNC(gt_device_base::bsga_yin1_w));
	map(0x12c, 0x12d).r(FUNC(gt_device_base::bsga_xin_r));
	map(0x12e, 0x12f).r(FUNC(gt_device_base::bsga_yin_r));
	map(0x12c, 0x12f).w(FUNC(gt_device_base::bsga_xin1yin1_w));
	map(0x130, 0x131).r(FUNC(gt_device_base::bsga_status_r));
	map(0x134, 0x135).w(FUNC(gt_device_base::bsga_xin2_w));
	map(0x138, 0x139).w(FUNC(gt_device_base::bsga_yin2_w));
	map(0x13c, 0x13f).w(FUNC(gt_device_base::bsga_xin2yin2_w));

	map(0x140, 0x143).w(FUNC(gt_device_base::ri_initial_distance_w));
	map(0x144, 0x147).w(FUNC(gt_device_base::ri_distance_both_w));
	map(0x148, 0x14b).w(FUNC(gt_device_base::ri_distance_major_w));
	map(0x14c, 0x14f).w(FUNC(gt_device_base::ri_initial_address_w));
	map(0x150, 0x153).w(FUNC(gt_device_base::ri_address_both_w));
	map(0x154, 0x157).w(FUNC(gt_device_base::ri_address_major_w));
	map(0x158, 0x15b).w(FUNC(gt_device_base::ri_initial_error_w));
	map(0x15c, 0x15f).w(FUNC(gt_device_base::ri_error_both_w));
	map(0x160, 0x163).w(FUNC(gt_device_base::ri_error_major_w));
	map(0x164, 0x167).w(FUNC(gt_device_base::ri_stop_count_w)); // 16 bit?

	map(0x16c, 0x16f).w(FUNC(gt_device_base::ri_control_w)); // mask 1ff?

	//AM_RANGE(0x174, 0x177) AM_READWRITE(ri_xfer_r, ri_xfer_w)
	//AM_RANGE(0x178, 0x17b) AM_READWRITE(ri_xfer_r, ri_xfer_w)
	map(0x17c, 0x17f).w(FUNC(gt_device_base::ri_xfer_w));

	map(0x1a4, 0x1ab).w(FUNC(gt_device_base::bsga_float_w));

	map(0x1b0, 0x1b3).nopr(); //?

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
 */
}

void gt_device::map(address_map &map)
{
	gt_device_base::map(map);

	map(0x00400000, 0x005fffff).rw(FUNC(gt_device::buffer_r), FUNC(gt_device::buffer_w));
	//map(0x00600000, 0x007fffff).rw(FUNC(dual_gt_device_base::buffer_r), FUNC(dual_gt_device_base::buffer_w)); // does this really exist?
}

void gtdb_device::map(address_map &map)
{
	gt_device_base::map(map);

	map(0x200, 0x203).rw(FUNC(gtdb_device::mouse_int_r), FUNC(gtdb_device::mouse_int_w));

	map(0x208, 0x20b).r(FUNC(gtdb_device::mouse_x_r));
	map(0x20c, 0x20f).r(FUNC(gtdb_device::mouse_y_r));

	// Note: FDMDISK GTII register ODT gives a different serial mapping, but does
	// not seem to be correct; the mapping here matches software usage elsewhere.
	map(0x210, 0x21f).rw(m_scc, FUNC(z80scc_device::dc_ab_r), FUNC(z80scc_device::dc_ab_w)).umask32(0x000000ff);

	map(0x300, 0x303).r(FUNC(gtdb_device::fifo_control_r));

	map(0x310, 0x313).w(FUNC(gtdb_device::srx_mapping_w));

	// TODO:
	// 304 system status
	// 330 fifo low wmark (w/o)
	// 334 fifo hi wmark (w/o)
	// 9100 vfifo int line (w/o)
	// 9300 vfifo int disp (w/o)
	// 9500 vfifo flt line (w/o)
	// 9700 vfifo flt disp (w/o)
}

void gtdb_device::map_dynamic(address_map &map)
{
	map(0x00000000, 0x001fffff).rw(FUNC(gtdb_device::buffer_r), FUNC(gtdb_device::buffer_w));
}

ROM_START(mpcb963)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcb963a.bin", 0x0, 0x20, CRC(4cf4562d) SHA1(58bcc2afb66168f1d44a0366b6a5ccc4c22e0f32))
ROM_END

ROM_START(mpcba79)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcba79a.bin", 0x0, 0x20, CRC(7b4c5a95) SHA1(a35f7117cb657122dedd71864e58d8c08ca12190))
ROM_END

ROM_START(msmt070)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt070b.bin", 0x0, 0x20, CRC(ad11a4e6) SHA1(e620cd37a1e36d2c548b7783ece336428ec75b0e))
ROM_END

ROM_START(msmt071)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt071b.bin", 0x0, 0x20, CRC(e46493e0) SHA1(49bd1890cc71dd8a7cbf5e17bf04843d6e075299))
ROM_END

ROM_START(msmt081)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt081b.bin", 0x0, 0x20, CRC(341c6ea0) SHA1(a5da37c3d9e040fc6d9ca99b82a25ed3ce4c57ff))
ROM_END

ROM_START(mpcbb68)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcbb68b.bin", 0x0, 0x20, CRC(faa95c4d) SHA1(5c286e87f051c6bd38137f47f89975f507b11b12))
ROM_END

ROM_START(mpcbb92)
	ROM_REGION(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcbb92a.bin", 0x0, 0x20, CRC(20547394) SHA1(9ddc6cccc80fee2a5ac77307b33a70b074b8c7d6))
ROM_END

// FIXME: can't account for this delta yet
#define GT_X_DELTA 20

// FIXME: the screen parameters below match what's coded in the system software,
// (except for the unexplained X_DELTA above), but produce an off-by-one mismatch
// in the cursor diagnostic tests. Visual mouse cursor positioning does seem to
// be correct however, so don't quite understand where the problem is.

void gt_device_base::device_add_mconfig(machine_config &config)
{
	DP8510(config, m_bpu[0], 0);
	DP8510(config, m_bpu[1], 0);
}

void interpro_digitizer_devices(device_slot_interface &device)
{
	device.option_add("loopback", RS232_LOOPBACK);
	//device.option_add("digitizer", ?);
}

void gtdb_device::device_add_mconfig(machine_config &config)
{
	gt_device_base::device_add_mconfig(config);

	SCC8530N(config, m_scc, 4.9152_MHz_XTAL);

	interpro_keyboard_port_device &keyboard(INTERPRO_KEYBOARD_PORT(config, "kbd", interpro_keyboard_devices, "lle_en_us"));
	keyboard.rxd_handler_cb().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_txda_callback().set(keyboard, FUNC(interpro_keyboard_port_device::write_txd));

	rs232_port_device &digitizer(RS232_PORT(config, "dig", interpro_digitizer_devices, nullptr));
	digitizer.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	digitizer.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(digitizer, FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(digitizer, FUNC(rs232_port_device::write_txd));

	m_scc->out_int_callback().set(*this, FUNC(gtdb_device::serial_irq));

	interpro_mouse_port_device &mouse(INTERPRO_MOUSE_PORT(config, "mse", interpro_mouse_devices, "interpro_mouse"));
	mouse.state_func().set(*this, FUNC(gtdb_device::mouse_status_w));
}

/*
 * MPCB963: GT graphics, 1 megapixel, single screen, 60Hz refresh.
 * MPCBA79: GT graphics, 1 megapixel, dual screen, 60Hz refresh.
 *
 * System software gives visible pixels 1184x884 and offsets h=296 v=34. Board
 * documentation gives pixel clock 83.0208MHz. Vertical refresh is assumed to
 * be 60Hz. Web source gives horizontal sync as 55.2kHz.
 *
 * These inputs give htotal=1504 and vtotal=920 with high confidence.
 */
void mpcb963_device::device_add_mconfig(machine_config &config)
{
	const u32 pixclock = 83'020'800;

	gt_device_base::device_add_mconfig(config);

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_raw(pixclock, 1504, 296 + GT_X_DELTA, 1184 + 296 + GT_X_DELTA, 920, 34, 884 + 34);
	m_screen[0]->set_screen_update(FUNC(mpcb963_device::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(device_cbus_card_interface::irq3));
	BT459(config, m_ramdac[0], pixclock);
	RAM(config, m_vram[0], 0).set_default_size("1M");
	RAM(config, m_mram[0], 0).set_default_size("128K");
}

void mpcba79_device::device_add_mconfig(machine_config &config)
{
	const u32 pixclock = 83'020'800;

	gt_device_base::device_add_mconfig(config);

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_raw(pixclock, 1504, 296 + GT_X_DELTA, 1184 + 296 + GT_X_DELTA, 920, 34, 884 + 34);
	m_screen[0]->set_screen_update(FUNC(mpcba79_device::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(device_cbus_card_interface::irq3));
	BT459(config, m_ramdac[0], pixclock);
	RAM(config, m_vram[0], 0).set_default_size("1M");
	RAM(config, m_mram[0], 0).set_default_size("128K");

	SCREEN(config, m_screen[1], SCREEN_TYPE_RASTER);
	m_screen[1]->set_raw(pixclock, 1504, 296 + GT_X_DELTA, 1184 + 296 + GT_X_DELTA, 920, 34, 884 + 34);
	m_screen[1]->set_screen_update(FUNC(mpcba79_device::screen_update<1>));
	BT459(config, m_ramdac[1], pixclock);
	RAM(config, m_vram[1], 0).set_default_size("1M");
	RAM(config, m_mram[1], 0).set_default_size("128K");
}

/*
 * MSMT070: GT+ graphics, 1 megapixel, single screen, 76Hz refresh.
 * MSMT071: GT+ graphics, 1 megapixel, dual screen, 76Hz refresh.
 *
 * System software gives visible pixels 1184x884 and offsets h=264 v=57. Board
 * documentation gives pixel clock 105.561MHz. Vertical refresh is assumed to
 * be 76Hz.
 *
 * These inputs give htotal=1472 and vtotal=944 with medium confidence, also
 * giving hsync=71.744kHz and vsync~=75.97Hz.
 */
void msmt070_device::device_add_mconfig(machine_config &config)
{
	const u32 pixclock = 105'561'000;

	gt_device_base::device_add_mconfig(config);

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_raw(pixclock, 1472, 264 + GT_X_DELTA, 1184 + 264 + GT_X_DELTA, 944, 57, 884 + 57);
	m_screen[0]->set_screen_update(FUNC(msmt070_device::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(device_cbus_card_interface::irq3));
	BT459(config, m_ramdac[0], pixclock);
	RAM(config, m_vram[0], 0).set_default_size("2M");
	RAM(config, m_mram[0], 0).set_default_size("128K");
}

void msmt071_device::device_add_mconfig(machine_config &config)
{
	const u32 pixclock = 105'561'000;

	gt_device_base::device_add_mconfig(config);

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_raw(pixclock, 1472, 264 + GT_X_DELTA, 1184 + 264 + GT_X_DELTA, 944, 57, 884 + 57);
	m_screen[0]->set_screen_update(FUNC(msmt071_device::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(device_cbus_card_interface::irq3));
	BT459(config, m_ramdac[0], pixclock);
	RAM(config, m_vram[0], 0).set_default_size("2M");
	RAM(config, m_mram[0], 0).set_default_size("128K");

	SCREEN(config, m_screen[1], SCREEN_TYPE_RASTER);
	m_screen[1]->set_raw(pixclock, 1472, 264 + GT_X_DELTA, 1184 + 264 + GT_X_DELTA, 944, 57, 884 + 57);
	m_screen[1]->set_screen_update(FUNC(msmt071_device::screen_update<1>));
	BT459(config, m_ramdac[1], pixclock);
	RAM(config, m_vram[1], 0).set_default_size("2M");
	RAM(config, m_mram[1], 0).set_default_size("128K");
}

/*
* MSMT081: GT+ graphics, 2 megapixel, single screen, 60Hz/76Hz refresh.
*
* System software gives visible pixels 1664x1248 and offsets h=391 v=74 (76Hz)
* and h=407 v=48 (60Hz).
* Vertical sync is assumed to be 76Hz. Web source gives horizontal sync as
* 100.8kHz.
*
* These inputs give htotal 2076 and vtotal 1324 with low confidence, also
* giving pixel clock 209.2608MHz and vsync 76.13Hz.
*/
void msmt081_device::device_add_mconfig(machine_config &config)
{
	const u32 pixclock = 209'260'800;

	gt_device_base::device_add_mconfig(config);

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_raw(pixclock, 2076, 391 + GT_X_DELTA, 1664 + 391 + GT_X_DELTA, 1324, 74, 1248 + 74);
	m_screen[0]->set_screen_update(FUNC(msmt081_device::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(device_cbus_card_interface::irq3));
	BT459(config, m_ramdac[0], pixclock);

	// FIXME: following memory sizes are pure speculation
	RAM(config, m_vram[0], 0).set_default_size("4M"); // guess
	RAM(config, m_mram[0], 0).set_default_size("256K"); // guess
}

/*
 * MPCBB68: GT II graphics (GTDB), 1 megapixel, single screen, 76Hz refresh.
 */
void mpcbb68_device::device_add_mconfig(machine_config &config)
{
	const u32 pixclock = 105'561'000;

	gtdb_device::device_add_mconfig(config);

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_raw(pixclock, 1472, 264 + GT_X_DELTA, 1184 + 264 + GT_X_DELTA, 944, 57, 884 + 57);
	m_screen[0]->set_screen_update(FUNC(mpcbb68_device::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(device_srx_card_interface::irq3));
	BT459(config, m_ramdac[0], pixclock);

	// FIXME: pure speculation
	RAM(config, m_vram[0], 0).set_default_size("2M");
	RAM(config, m_mram[0], 0).set_default_size("128K");
	RAM(config, m_hram[0], 0).set_default_size("512K");
}

/*
 * MPCBB92: GT II graphics (GTDB), 2 megapixel, single screen, 60Hz/76Hz refresh.
 *
 * System software gives visible pixels 1664x1248 and offsets h=391 v=74 (76Hz)
 * and h=407 v=48 (60Hz). Vertical sync is assumed to be 60Hz.
 *
 * These inputs give htotal 2076 and vtotal 1324 with low confidence, also
 * giving pixel clock 209.2608MHz and vsync 76.13Hz.
 */
void mpcbb92_device::device_add_mconfig(machine_config &config)
{
	const u32 pixclock = 209'260'800;

	gtdb_device::device_add_mconfig(config);

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_raw(pixclock, 2076, 391 + GT_X_DELTA, 1664 + 391 + GT_X_DELTA, 1324, 74, 1248 + 74);
	m_screen[0]->set_screen_update(FUNC(mpcbb92_device::screen_update<0>));
	m_screen[0]->screen_vblank().set(FUNC(device_srx_card_interface::irq3));
	BT459(config, m_ramdac[0], pixclock);

	// FIXME: following memory sizes are pure speculation (40 parts @ 256Kx4?)
	RAM(config, m_vram[0], 0).set_default_size("4M");
	RAM(config, m_mram[0], 0).set_default_size("256K");
	RAM(config, m_hram[0], 0).set_default_size("1M");
}

gt_device_base::gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool double_buffered, const bool masked_reads)
	: device_t(mconfig, type, tag, owner, clock)
	, m_screen(*this, "screen%u", 0)
	, m_ramdac(*this, "ramdac%u", 0)
	, m_vram(*this, "vram%u", 0)
	, m_mram(*this, "mram%u", 0)
	, m_bpu(*this, "bpu%u", 0)
	, m_double_buffered(double_buffered)
	, m_masked_reads(masked_reads)
{
}

gt_device::gt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const bool double_buffered)
	: gt_device_base(mconfig, type, tag, owner, clock, double_buffered, true)
	, device_cbus_card_interface(mconfig, *this)
{
}

gtdb_device::gtdb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: gt_device_base(mconfig, type, tag, owner, clock, true, false)
	, device_srx_card_interface(mconfig, *this)
	, m_hram(*this, "hram%u", 0)
	, m_scc(*this, "scc")
{
}

mpcb963_device::mpcb963_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gt_device(mconfig, MPCB963, tag, owner, clock, false)
{
}

mpcba79_device::mpcba79_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gt_device(mconfig, MPCBA79, tag, owner, clock, false)
{
}

msmt070_device::msmt070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gt_device(mconfig, MSMT070, tag, owner, clock, true)
{
}

msmt071_device::msmt071_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gt_device(mconfig, MSMT071, tag, owner, clock, true)
{
}

msmt081_device::msmt081_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gt_device(mconfig, MSMT081, tag, owner, clock, true)
{
}

mpcbb68_device::mpcbb68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gtdb_device(mconfig, MPCBB68, tag, owner, clock)
{
}

mpcbb92_device::mpcbb92_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gtdb_device(mconfig, MPCBB92, tag, owner, clock)
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

const tiny_rom_entry *msmt070_device::device_rom_region() const
{
	return ROM_NAME(msmt070);
}

const tiny_rom_entry *msmt071_device::device_rom_region() const
{
	return ROM_NAME(msmt071);
}

const tiny_rom_entry *msmt081_device::device_rom_region() const
{
	return ROM_NAME(msmt081);
}

const tiny_rom_entry *mpcbb68_device::device_rom_region() const
{
	return ROM_NAME(mpcbb68);
}

const tiny_rom_entry *mpcbb92_device::device_rom_region() const
{
	return ROM_NAME(mpcbb92);
}

void gt_device_base::device_validity_check(validity_checker &valid) const
{
	if (!m_screen[0].found())
		osd_printf_error("screen[0] is required");

	if (!m_ramdac[0].found())
		osd_printf_error("ramdac[0] is required");

	if (!m_vram[0].found())
		osd_printf_error("vram[0] is required");

	if (!m_mram[0].found())
		osd_printf_error("mram[0] is required");
}

void gt_device_base::device_start()
{
	save_item(NAME(m_control));

	// allocate timers
	m_blit_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt_device_base::blit), this));
	m_line_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt_device_base::line), this));
	m_done_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gt_device_base::done), this));
}

WRITE32_MEMBER(gt_device_base::control_w)
{
	if (data & GFX_BSGA_RST)
	{
		// set graphics busy and schedule a reset
		m_control |= GFX_GRPHCS_BUSY;

		// reset the bitblt fifo pointers
		bpu_reset();

		m_done_timer->adjust(attotime::from_msec(10), -1);
	}

	// pass direction to bpu
	bpu_barrel_input_select((data & GFX_BLIT_DIR) ? ASSERT_LINE : CLEAR_LINE);

	// don't write to read-only fields
	mem_mask &= ~(GFX_MONSENSE_MASK | GFX_VFIFO_EMPTY | GFX_GRPHCS_BUSY | GFX_BLIT_BUSY | GFX_VERT_BLNK);

	COMBINE_DATA(&m_control);
}

// bsga test = 121780
//  12184c = preparation
//  1218ea = execution
//  122922 = simulate clip
// bp 122922,1,{ logerror "simulate xy=%08x, min=%08x, max=%08x status=%04x", r0, r1, pd@(r15+4), pd@(r15+8); g }
// bp 1229ee,1,{ logerror " -> %04x\n", r0; g }

void gt_device_base::bsga_clip_status(s16 x, s16 y)
{
	// compute Cohen-Sutherland clipping outcode
	LOG("bsga_clip_status previous 0x%04x\n", m_bsga_status);
	m_bsga_status &= STATUS_CLIP1_MASK;
	m_bsga_status <<= 4;

	LOG("bsga_clip_status shifted 0x%04x\n", m_bsga_status);
	LOG("bsga_clip_status (%04x,%04x) (%04x,%04x,%04x,%04x)\n", u16(x), u16(y), m_bsga_xmin, m_bsga_xmax, m_bsga_ymin, m_bsga_ymax);

	// clip x coordinate
	if (x < (s16)m_bsga_xmin)
		m_bsga_status |= STATUS_LEFT;
	if (x > (s16)m_bsga_xmax)
		m_bsga_status |= STATUS_RIGHT;

	// clip y coordinate
	if (y < (s16)m_bsga_ymin)
		m_bsga_status |= STATUS_ABOVE;
	if (y > (s16)m_bsga_ymax)
		m_bsga_status |= STATUS_BELOW;

	if (m_bsga_status & (STATUS_CLIP0_MASK | STATUS_CLIP1_MASK))
		m_bsga_status |= STATUS_ACCEPT;

	if (((m_bsga_status & STATUS_CLIP0_MASK) >> 4) & (m_bsga_status & STATUS_CLIP1_MASK))
		m_bsga_status |= STATUS_REJECT;

	LOG("bsga_clip_status result 0x%04x\n", m_bsga_status);
}

WRITE32_MEMBER(gt_device_base::ri_xfer_w)
{
	LOG("ri_xfer_w 0x%08x mem_mask 0x%08x (%s)\n", data, mem_mask, machine().describe_context());

	// initiate ri line draw
	u32 address = m_ri_initial_address;
	u32 error = m_ri_initial_error;

	for (int i = 0; i < m_ri_stop_count; i++)
	{
		if (i || (m_control & GFX_DRAW_FIRST))
			vram_w(address >> 2, m_plane_data, 0xff << ((address & 0x3) << 3));

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

	LOG("xin = %04x\n", m_bsga_xin1);
	LOG("yin = %04x\n", m_bsga_yin1);

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
	m_line_timer->adjust(attotime::zero);
}

WRITE16_MEMBER(gt_device_base::bsga_yin2_w)
{
	COMBINE_DATA(&m_bsga_yin2);

	// set busy status
	m_control |= GFX_GRPHCS_BUSY;

	// compute clipping status
	bsga_clip_status(m_bsga_xin2, m_bsga_yin2);

	// trigger line drawing
	m_line_timer->adjust(attotime::zero);
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

	LOG("bsga_float_w %d data 0x%08x value %f exponent 0x%02x shift %d mantissa 0x%08x mantissa sign %d\n", offset, data, u2f(data), exponent, shift, mantissa, sign);

	m_bsga_status &= ~STATUS_FLOAT_OFLOW;
	bool overflow = false;

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
			overflow = true;
	}
	else
	{
		m_bsga_xin = 0;
		overflow = true;
	}

	if (offset)
		m_bsga_yin1 = m_bsga_xin;
	else
		m_bsga_xin1 = m_bsga_xin;

	bsga_clip_status(m_bsga_xin1, m_bsga_yin1);

	if (overflow)
		m_bsga_status = (m_bsga_status | STATUS_FLOAT_OFLOW | STATUS_ACCEPT) & ~STATUS_REJECT;

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
	m_blit_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(gt_device_base::blit)
{
	LOGMASKED(LOG_BLIT, "blit bsga_control 0x%08x src 0x%08x dst 0x%08x width 0x%04x (count %d, %s)\n",
		m_control, m_blit_src_address, m_blit_dst_address, m_blit_width,
		m_blit_width >> 2, (m_control & GFX_BLIT_DIR) ? "decrementing" : "incrementing");

	const int delta = (m_control & GFX_BLIT_DIR) ? -1 : 1;
	const int count = m_blit_width >> 2;

	u32 src_address = m_blit_src_address >> 2;
	u32 dst_address = m_blit_dst_address >> 2;

	// load barrel input latch
	if (!(m_control & GFX_DRAW_FIRST))
	{
		if (!(m_control & GFX_DATA_SEL))
		{
			const u32 data = vram_r(src_address, true);
			const u32 mask = mram_r(src_address);

			if (m_control & GFX_MASK_ENA)
				bpu_source_w((m_plane_data & mask) | (data & ~mask), false);
			else
				bpu_source_w(data, false);

			src_address += delta;
		}
		else
			bpu_source_w(m_plane_data, false);
	}

	// bitblt address loop
	for (int word = 0; word < count; word++)
	{
		// drive left and right mask enables
		bpu_left_mask_enable(word == ((m_control & GFX_BLIT_DIR) ? count - 1 : 0));
		bpu_right_mask_enable(word == ((m_control & GFX_BLIT_DIR) ? 0 : count - 1));

		// load bpu source
		if (!(m_control & GFX_DATA_SEL))
		{
			const u32 data = vram_r(src_address, true);

			LOGMASKED(LOG_BLIT, "blit src %3d address 0x%08x data 0x%08x\n",
				word, src_address, data);

			if (m_control & GFX_MASK_ENA)
			{
				const u32 mask = mram_r(src_address);

				LOGMASKED(LOG_BLIT, "blit src %3d address 0x%08x mask 0x%08x\n",
					word, src_address, mask);

				bpu_source_w((m_plane_data & mask) | (data & ~mask));
			}
			else
				bpu_source_w(data);

			src_address += delta;
		}
		else
			bpu_source_w(m_plane_data);

		// load bpu destination
		{
			const u32 data = (m_control & GFX_MASK_SEL) ? mram_r(dst_address) & GT_MASK_BITS : vram_r(dst_address, true);

			LOGMASKED(LOG_BLIT, "blit dst %3d address 0x%08x data 0x%08x\n",
				word, dst_address, data);

			bpu_destination_w(data);
		}

		// fetch bpu output
		const u32 output = bpu_output_r();

		// write to video or mask ram
		if (!(m_control & GFX_MASK_SEL))
		{
			// write to pixel buffer
			vram_w(dst_address, output, m_plane_enable, true);

			LOGMASKED(LOG_BLIT, "blit out %3d address 0x%08x output 0x%08x\n",
				word, dst_address, output);
		}
		else
		{
			// write to mask
			mram_w(dst_address, output, m_plane_enable);

			LOGMASKED(LOG_BLIT, "blit out %3d address 0x%08x output 0x%08x result 0x%08x\n",
				word, dst_address, output, output & GT_MASK_BITS);
		}

		dst_address += delta;
	}

	// clear mask enable lines
	bpu_left_mask_enable(CLEAR_LINE);
	bpu_right_mask_enable(CLEAR_LINE);

	// complete with delay
	if (GT_DIAG)
		m_done_timer->adjust(attotime::from_msec(10), GFX_GRPHCS_BUSY | GFX_BLIT_BUSY);
	else
		m_control &= ~(GFX_GRPHCS_BUSY | GFX_BLIT_BUSY);
}

TIMER_CALLBACK_MEMBER(gt_device_base::line)
{
	// draw a clipped line

	// FIXME: fix clipping to use >= min and < max
	kuzmin_clip(m_bsga_xin1, m_bsga_yin1, m_bsga_xin2, m_bsga_yin2, m_bsga_xmin, m_bsga_ymin, m_bsga_xmax, m_bsga_ymax);

	// point #2 becomes point #1
	m_bsga_xin1 = m_bsga_xin2;
	m_bsga_yin1 = m_bsga_yin2;

	m_bsga_xin = m_bsga_xin1;
	m_bsga_yin = m_bsga_yin1;

	// complete with delay
	if (GT_DIAG)
		m_done_timer->adjust(attotime::from_nsec(100), GFX_GRPHCS_BUSY);
	else
		m_control &= ~(GFX_GRPHCS_BUSY);
}

TIMER_CALLBACK_MEMBER(gt_device_base::done)
{
	m_control &= ~u32(param);
}

WRITE8_MEMBER(gt_device_base::plane_enable_w)
{
	if (m_control & GFX_GRPHCS_BUSY)
		return;

	LOG("plane enable 0x%02x\n", data);

	// replicate to u32 to simplify operations
	m_plane_enable = (data << 24) | (data << 16) | (data << 8) | (data << 0);
}

WRITE8_MEMBER(gt_device_base::plane_data_w)
{
	if (m_control & GFX_GRPHCS_BUSY)
		return;

	LOG("plane data 0x%02x\n", data);

	// replicate to u32 to simplify operations
	m_plane_data = (data << 24) | (data << 16) | (data << 8) | (data << 0);
}

u32 gt_device_base::buffer_r(const offs_t offset)
{
	if (m_control & GFX_MASK_SEL)
		return mram_r(offset) & GT_MASK_BITS;
	else if ((m_control & GFX_MASK_ENA) && (m_masked_reads || (m_control & GFX_MASK_READ_ENA)))
		return (m_plane_data & mram_r(offset)) | (vram_r(offset) & ~mram_r(offset));
	else
		return vram_r(offset);
}

void gt_device_base::buffer_w(const offs_t offset, u32 data, u32 mem_mask)
{
	// data select mode: four bits control pixel selection
	if (m_control & GFX_DATA_SEL)
	{
		mem_mask =
			((data & 0x1) ? 0x000000ff : 0) |
			((data & 0x2) ? 0x0000ff00 : 0) |
			((data & 0x4) ? 0x00ff0000 : 0) |
			((data & 0x8) ? 0xff000000 : 0);

		data = m_plane_data;
	}

	// read/modify/write mode: bpu computes output data
	if (m_control & GFX_RMW_MD)
	{
		bpu_source_w(data);
		bpu_destination_w(vram_r(offset));

		data = bpu_output_r();
	}

	// write to video or mask ram
	if (m_control & GFX_MASK_SEL)
		mram_w(offset, data, mem_mask);
	else
		vram_w(offset, data, mem_mask);
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

	// all trivial cases are handled by STATUS_REJECT
	if (m_bsga_status & STATUS_REJECT)
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
		if (m_bsga_status & STATUS_ACCEPT)
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
		if (m_bsga_status & STATUS_ACCEPT)
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
	LOG("bresenham_line begin %d,%d steps %d\n", shallow ? major : minor, shallow ? minor : major, steps);

	const int screen_width = m_screen[0]->visible_area().width();

	for (int i = 0; i < steps; i++)
	{
		if (i || m_control & GFX_DRAW_FIRST)
		{
			const offs_t offset = shallow
				? minor * screen_width + major
				: major * screen_width + minor;

			// generate mask from pixel position
			vram_w(offset >> 2, m_plane_data, 0xff << ((offset & 0x3) << 3));
		}

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

WRITE8_MEMBER(gt_device_base::contrast_dac_w)
{
	m_ramdac[0]->set_contrast(data);

	if (m_ramdac[1].found())
		m_ramdac[1]->set_contrast(data);
}

/*
 * GTDB support (SRX, SCC and mouse).
 */
WRITE32_MEMBER(gtdb_device::srx_mapping_w)
{
	const offs_t srx_base = data << 24;

	m_bus->install_map(*this, srx_base, srx_base | 0xffffff, &gtdb_device::map_dynamic);
}

WRITE_LINE_MEMBER(gtdb_device::serial_irq)
{
	if (state)
		m_mouse_int |= SERIAL;
	else
		m_mouse_int &= ~SERIAL;

	irq0(state);
}

WRITE32_MEMBER(gtdb_device::mouse_status_w)
{
	if (mem_mask & interpro_mouse_device::state_mask::MOUSE_XPOS)
	{
		m_mouse_x = (data & interpro_mouse_device::state_mask::MOUSE_XPOS) >> 8;
		m_mouse_int |= MOUSE_X;
	}

	if (mem_mask & interpro_mouse_device::state_mask::MOUSE_YPOS)
	{
		m_mouse_y = (data & interpro_mouse_device::state_mask::MOUSE_YPOS) >> 0;
		m_mouse_int |= MOUSE_Y;
	}

	if (mem_mask & interpro_mouse_device::state_mask::MOUSE_BUTTONS)
	{
		// left and right button bit positions are swapped when compared to ioga
		const u8 buttons = bitswap<8>(((data >> 16) & 0x7), 7, 6, 5, 4, 3, 0, 1, 2);

		m_mouse_int &= ~0x0f;
		m_mouse_int |= buttons;
		m_mouse_int |= MOUSE_BTN;
	}

	irq0(ASSERT_LINE);
	irq0(CLEAR_LINE);
}

READ32_MEMBER(gtdb_device::mouse_x_r)
{
	const u32 result = m_mouse_x;

	m_mouse_x = 0;

	return result;
}

READ32_MEMBER(gtdb_device::mouse_y_r)
{
	const u32 result = m_mouse_y;

	m_mouse_y = 0;

	return result;
}

/*
 * The following helpers read and write data to video and mask RAM, applying
 * the screen and buffer select flags in the control register as necessary.
 * Pixel data is stored in pixel drawing order (little-endian), and mask data
 * is stored with the least significant bit corresponding to the first drawn
 * pixel. Writes to video ram are first masked by the plane enable register,
 * and by the mask RAM if enabled.
 *
 * Mask RAM consists of a single bit for each pixel, presented on the 32-bit
 * host data bus using the most significant bit of each byte. To make masking
 * arithmetic simpler, the helper below replicates this single bit to every
 * bit in each pixel instead; the unused bits are easily discarded with an
 * additional mask when needed.
 */
u32 gt_device_base::vram_r(offs_t offset, const bool linear) const
{
	// determine selected screen
	const int selected = (m_control & GFX_SCR1_SEL) ? 1 : 0;
	if (!m_vram[selected].found())
		return 0;

	// get the base memory pointer
	u32 *const vram = reinterpret_cast<u32 *>(m_vram[selected]->pointer());

	// adjust for second buffer
	if (m_double_buffered && !linear && (m_control & GFX_BUF1_SEL))
		offset += m_vram[selected]->size() >> 3;

	// vram data is always in pixel order (little endian)
	return little_endianize_int32(vram[offset]);
}

void gt_device_base::vram_w(offs_t offset, const u32 data, u32 mem_mask, const bool linear) const
{
	// determine selected screen
	const int selected = (m_control & GFX_SCR1_SEL) ? 1 : 0;
	if (!m_vram[selected].found())
		return;

	// only write to enabled planes
	mem_mask &= m_plane_enable;

	// apply mask
	if (m_control & GFX_MASK_ENA)
		mem_mask &= ~mram_r(offset);

	// get the base memory pointer
	u32 *const vram = reinterpret_cast<u32 *>(m_vram[selected]->pointer());

	// adjust for second buffer
	if (m_double_buffered && !linear && (m_control & GFX_BUF1_SEL))
		offset += m_vram[selected]->size() >> 3;

	// vram data is always in pixel order (little endian)
	vram[offset] = little_endianize_int32((little_endianize_int32(vram[offset]) & ~mem_mask) | (data & mem_mask));
}

u32 gt_device_base::mram_r(const offs_t offset) const
{
	// map 4-bit mask RAM data to 32-bit equivalent
	static const u32 mask_map[] =
	{
		0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
		0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
		0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
		0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff
	};

	// determine selected screen
	const int selected = (m_control & GFX_SCR1_SEL) ? 1 : 0;
	if (!m_mram[selected].found())
		return 0;

	// read the mask RAM data
	const u8 data = (offset & 1) ?
		m_mram[selected]->read(offset >> 1) >> 4 :
		m_mram[selected]->read(offset >> 1) >> 0;

	// return the mapped mask
	return mask_map[data & 0xf];
}

void gt_device_base::mram_w(const offs_t offset, const u32 data, const u32 mem_mask) const
{
	// determine selected screen
	const int selected = (m_control & GFX_SCR1_SEL) ? 1 : 0;
	if (!m_mram[selected].found())
		return;

	// compute the stored mask data and mask
	const u8 bits = (offset & 1) ?
		((data & 0x80000000) >> 24) | ((data & 0x00800000) >> 17) | ((data & 0x00008000) >> 10) | ((data & 0x00000080) >> 3) :
		((data & 0x80000000) >> 28) | ((data & 0x00800000) >> 21) | ((data & 0x00008000) >> 14) | ((data & 0x00000080) >> 7);
	const u8 mask = (offset & 1) ?
		((mem_mask & 0x80000000) >> 24) | ((mem_mask & 0x00800000) >> 17) | ((mem_mask & 0x00008000) >> 10) | ((mem_mask & 0x00000080) >> 3) :
		((mem_mask & 0x80000000) >> 28) | ((mem_mask & 0x00800000) >> 21) | ((mem_mask & 0x00008000) >> 14) | ((mem_mask & 0x00000080) >> 7);

	// store the mask data
	m_mram[selected]->write(offset >> 1, (m_mram[selected]->read(offset >> 1) & ~mask) | (bits & mask));
}

/*
 * Highlight RAM contains two bits per pixel per buffer, and is presented on
 * the 32-bit host data bus using the least significant 2 bits of each byte.
 */
u32 gtdb_device::vram_r(offs_t offset, const bool linear) const
{
	if (m_control & GFX_HILITE_SEL)
	{
		// determine selected screen
		const int selected = (m_control & GFX_SCR1_SEL) ? 1 : 0;
		if (!m_hram[selected].found())
			return 0;

		// read the hilight RAM data
		const u8 data = m_hram[selected]->read(offset);

		// return the expanded hilight data
		return ((data & 0x03) << 24) | ((data & 0x0c) << 14) | ((data & 0x30) << 4) | ((data & 0xc0) >> 6);
	}
	else
		return gt_device_base::vram_r(offset, linear);
}

void gtdb_device::vram_w(const offs_t offset, const u32 data, const u32 mem_mask, const bool linear) const
{
	if (m_control & GFX_HILITE_SEL)
	{
		// determine selected screen
		const int selected = (m_control & GFX_SCR1_SEL) ? 1 : 0;
		if (!m_hram[selected].found())
			return;

		// compute the stored highlight RAM data and mask
		const u8 bits = ((data & 0x03000000) >> 24) | ((data & 0x00030000) >> 14) | ((data & 0x00000300) >> 4) | ((data & 0x00000003) << 6);
		const u8 mask = ((mem_mask & 0x03000000) >> 24) | ((mem_mask & 0x00030000) >> 14) | ((mem_mask & 0x00000300) >> 4) | ((mem_mask & 0x00000003) << 6);

		// store the highlight data
		m_hram[selected]->write(offset, (m_hram[selected]->read(offset) & ~mask) | (bits & mask));
	}
	else
		gt_device_base::vram_w(offset, data, mem_mask, linear);
}

/*
 * The bitblit processing unit (BPU) is composed of a pair of 16-bit DP8510
 * devices. These are used to process four 8-bit pixels at once, with each of
 * the devices handling 4 bits of each pixel. The DP8510 devices use big-endian
 * pixel encoding, where bit 15 corresponds to the most significant bit of the
 * first drawn pixel, and bit 0 to the least significant bit of the last drawn
 * pixel. These devices are coupled to the little-endian InterPro 32-bit bus
 * in a reversed, interleaved manner so this ordering is handled automatically.
 *
 * Given four 8-bit pixels in display order A, B, C, and D; the little-endian,
 * 32-bit host encoding, broken into 4 bit components is DdCcBbAa. When written
 * to the DP8510's, this value is deinterleaved and reversed, becoming ABCD and
 * abcd respectively, and the opposite transformation occuring when read.
 */

// deinterleave and reverse a host 32-bit value into 16-bit hi/lo parts for the bpu
gt_device_base::bpu_pair_t gt_device_base::bpu_from_u32(const u32 data) const
{
	return {
		u16(((data & 0x000000f0) <<  8) | ((data & 0x0000f000) >> 4) | ((data & 0x00f00000) >> 16) | ((data & 0xf0000000) >> 28)),
		u16(((data & 0x0000000f) << 12) | ((data & 0x00000f00) << 0) | ((data & 0x000f0000) >> 12) | ((data & 0x0f000000) >> 24))
	};
}

// interleave and reverse 16-bit bpu hi/lo parts to form a host 32-bit value
u32 gt_device_base::bpu_to_u32(bpu_pair_t data) const
{
	return
		((data.hi & 0xf000) >>  8) | ((data.hi & 0x0f00) << 4) | ((data.hi & 0x00f0) << 16) | ((data.hi & 0x000f) << 28) |
		((data.lo & 0xf000) >> 12) | ((data.lo & 0x0f00) << 0) | ((data.lo & 0x00f0) << 12) | ((data.lo & 0x000f) << 24);
}

void gt_device_base::bpu_control_w(const u32 data)
{
	const bpu_pair_t pair = bpu_from_u32(data);

	m_bpu[0]->control_w(pair.hi);
	m_bpu[1]->control_w(pair.lo);
}

void gt_device_base::bpu_source_w(const u32 data, const bool fifo_write)
{
	const bpu_pair_t pair = bpu_from_u32(data);

	m_bpu[0]->source_w(pair.hi, fifo_write);
	m_bpu[1]->source_w(pair.lo, fifo_write);
}

void gt_device_base::bpu_destination_w(const u32 data, const bool fifo_write)
{
	const bpu_pair_t pair = bpu_from_u32(data);

	m_bpu[0]->destination_w(pair.hi, fifo_write);
	m_bpu[1]->destination_w(pair.lo, fifo_write);
}

u32 gt_device_base::bpu_output_r()
{
	const bpu_pair_t pair = { m_bpu[0]->output_r(), m_bpu[1]->output_r() };

	return bpu_to_u32(pair);
}

void gt_device_base::bpu_reset()
{
	m_bpu[0]->reset();
	m_bpu[1]->reset();
}

void gt_device_base::bpu_barrel_input_select(const int state)
{
	m_bpu[0]->barrel_input_select(state);
	m_bpu[1]->barrel_input_select(state);
}

void gt_device_base::bpu_left_mask_enable(const int state)
{
	m_bpu[0]->left_mask_enable(state);
	m_bpu[1]->left_mask_enable(state);
}

void gt_device_base::bpu_right_mask_enable(const int state)
{
	m_bpu[0]->right_mask_enable(state);
	m_bpu[1]->right_mask_enable(state);
}
