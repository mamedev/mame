// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "zr36060.h"

#define VERBOSE       (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(ZR36060, zr36060_device, "zr36060", "Zoran ZR36060 Integrated JPEG codec")

zr36060_device::zr36060_device(const machine_config &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ZR36060, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
{
	m_space_config = address_space_config("regs", ENDIANNESS_BIG, 8, 10, 0, address_map_constructor(FUNC(zr36060_device::regs_map), this));

}

device_memory_interface::space_config_vector zr36060_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


void zr36060_device::device_start()
{
}

void zr36060_device::device_reset()
{
	m_load = false;
	m_syncrst = false;
	m_code_mstr = true;
	m_codec_mode = 0x80;
}

void zr36060_device::regs_map(address_map &map)
{
	// Codec Control Registers
	map(0x000, 0x000).rw(FUNC(zr36060_device::load_r), FUNC(zr36060_device::load_w));
	map(0x001, 0x001).r(FUNC(zr36060_device::code_fifo_status_r));
	map(0x002, 0x002).rw(FUNC(zr36060_device::code_if_r), FUNC(zr36060_device::code_if_w));
	map(0x003, 0x003).rw(FUNC(zr36060_device::codec_mode_r), FUNC(zr36060_device::codec_mode_w));

	map(0x005, 0x005).rw(FUNC(zr36060_device::mbcv_r), FUNC(zr36060_device::mbcv_w));
	map(0x006, 0x006).rw(FUNC(zr36060_device::markers_enable_r), FUNC(zr36060_device::markers_enable_w));
	map(0x007, 0x007).rw(FUNC(zr36060_device::irq_mask_r), FUNC(zr36060_device::irq_mask_w));
	map(0x008, 0x008).r(FUNC(zr36060_device::irq_status_r));
	map(0x009, 0x00c).rw(FUNC(zr36060_device::tcv_net_r), FUNC(zr36060_device::tcv_net_w));
	map(0x00d, 0x010).rw(FUNC(zr36060_device::tcv_data_r), FUNC(zr36060_device::tcv_data_w));
	map(0x011, 0x012).rw(FUNC(zr36060_device::sf_r), FUNC(zr36060_device::sf_w));
	map(0x013, 0x015).rw(FUNC(zr36060_device::af_r), FUNC(zr36060_device::af_w));
	map(0x016, 0x019).rw(FUNC(zr36060_device::acv_r), FUNC(zr36060_device::acv_w));
	map(0x01a, 0x01d).rw(FUNC(zr36060_device::act_r), FUNC(zr36060_device::act_w));
	map(0x01e, 0x021).rw(FUNC(zr36060_device::acv_trun_r), FUNC(zr36060_device::acv_trun_w));
	map(0x022, 0x022).lr8(NAME([this] (offs_t offset) { LOG("Read Device ID\n"); return 0x33; }));
	map(0x023, 0x023).lr8(NAME([this] (offs_t offset) { LOG("Read Revision ID\n"); return 0x01; }));
//  map(0x024, 0x025) Test Control

	// Video Registers
	map(0x030, 0x030).rw(FUNC(zr36060_device::video_control_r), FUNC(zr36060_device::video_control_w));
	map(0x031, 0x031).rw(FUNC(zr36060_device::video_polarity_r), FUNC(zr36060_device::video_polarity_w));
	map(0x032, 0x032).rw(FUNC(zr36060_device::video_scaling_r), FUNC(zr36060_device::video_scaling_w));
	// Background Color
	map(0x033, 0x033).lrw8(
		NAME([this] (offs_t offset) { return m_bg_color.y; }),
		NAME([this] (offs_t offset, u8 data) { m_bg_color.y = data; LOG("Background Color Y %02x\n", data); })
	);
	map(0x034, 0x034).lrw8(
		NAME([this] (offs_t offset) { return m_bg_color.u; }),
		NAME([this] (offs_t offset, u8 data) { m_bg_color.u = data; LOG("Background Color U %02x\n", data); })
	);
	map(0x035, 0x035).lrw8(
		NAME([this] (offs_t offset) { return m_bg_color.v; }),
		NAME([this] (offs_t offset, u8 data) { m_bg_color.v = data; LOG("Background Color V %02x\n", data); })
	);
	// Sync Generator
	map(0x036, 0x037).lrw8(
		NAME([this] (offs_t offset) {
			return (m_sync_gen.vtotal >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_sync_gen.vtotal &= offset ? 0xff00 : 0x00ff;
			m_sync_gen.vtotal |= (data << shift);
			LOG("Sync Generator: Vtotal [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_sync_gen.vtotal, m_sync_gen.vtotal + 1
			);
		})
	);
	map(0x038, 0x039).lrw8(
		NAME([this] (offs_t offset) {
			return (m_sync_gen.htotal >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_sync_gen.htotal &= offset ? 0x0300 : 0x00ff;
			m_sync_gen.htotal |= (data << shift);
			LOG("Sync Generator: Htotal [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_sync_gen.htotal, m_sync_gen.htotal + 1
			);
		})
	);
	map(0x03a, 0x03a).lrw8(
		NAME([this] (offs_t offset) { return m_sync_gen.vsync_size; }),
		NAME([this] (offs_t offset, u8 data) {
			m_sync_gen.vsync_size = data;
			LOG("Sync Generator: VsyncSize %02x -> %d\n"
				, data
				, m_sync_gen.vsync_size + 1
			);
		})
	);
	map(0x03b, 0x03b).lrw8(
		NAME([this] (offs_t offset) { return m_sync_gen.hsync_size; }),
		NAME([this] (offs_t offset, u8 data) {
			m_sync_gen.hsync_size = data;
			LOG("Sync Generator: HsyncSize %02x -> %d\n"
				, data
				, m_sync_gen.hsync_size + 1
			);
		})
	);
	map(0x03c, 0x03c).lrw8(
		NAME([this] (offs_t offset) { return m_sync_gen.bvstart; }),
		NAME([this] (offs_t offset, u8 data) {
			m_sync_gen.bvstart = data;
			LOG("Sync Generator: BVstart %02x -> %d\n"
				, data
				, m_sync_gen.bvstart + 1
			);
		})
	);
	map(0x03d, 0x03d).lrw8(
		NAME([this] (offs_t offset) { return m_sync_gen.bhstart; }),
		NAME([this] (offs_t offset, u8 data) {
			m_sync_gen.bhstart = data;
			LOG("Sync Generator: BHstart %02x -> %d\n"
				, data
				, m_sync_gen.bhstart + 1
			);
		})
	);
	map(0x03e, 0x03f).lrw8(
		NAME([this] (offs_t offset) {
			return (m_sync_gen.bvend >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_sync_gen.bvend &= offset ? 0xff00 : 0x00ff;
			m_sync_gen.bvend |= (data << shift);
			LOG("Sync Generator: BVend [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_sync_gen.bvend, m_sync_gen.bvend
			);
		})
	);
	map(0x040, 0x041).lrw8(
		NAME([this] (offs_t offset) {
			return (m_sync_gen.bhend >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_sync_gen.bhend &= offset ? 0x0300 : 0x00ff;
			m_sync_gen.bhend |= (data << shift);
			LOG("Sync Generator: BHend [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_sync_gen.bhend, m_sync_gen.bhend
			);
		})
	);

	// Active Area
	map(0x042, 0x043).lrw8(
		NAME([this] (offs_t offset) {
			return (m_active_area.vstart >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_active_area.vstart &= offset ? 0xff00 : 0x00ff;
			m_active_area.vstart |= (data << shift);
			LOG("Active Area: Vstart [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_active_area.vstart, m_active_area.vstart + 1
			);
		})
	);
	map(0x044, 0x045).lrw8(
		NAME([this] (offs_t offset) {
			return (m_active_area.vend >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_active_area.vend &= offset ? 0xff00 : 0x00ff;
			m_active_area.vend |= (data << shift);
			LOG("Active Area: Vend [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_active_area.vend, m_active_area.vend
			);
		})
	);
	map(0x046, 0x047).lrw8(
		NAME([this] (offs_t offset) {
			return (m_active_area.hstart >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_active_area.hstart &= offset ? 0x0300 : 0x00ff;
			m_active_area.hstart |= (data << shift);
			LOG("Active Area: Hstart [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_active_area.hstart, m_active_area.hstart + 1
			);
		})
	);
	map(0x048, 0x049).lrw8(
		NAME([this] (offs_t offset) {
			return (m_active_area.hend >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_active_area.hend &= offset ? 0x0300 : 0x00ff;
			m_active_area.hend |= (data << shift);
			LOG("Active Area: Hend [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_active_area.hend, m_active_area.hend + 1
			);
		})
	);

	// SUBIMG Window
	map(0x04a, 0x04b).lrw8(
		NAME([this] (offs_t offset) {
			return (m_subimg_window.svstart >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_subimg_window.svstart &= offset ? 0xff00 : 0x00ff;
			m_subimg_window.svstart |= (data << shift);
			LOG("SUBIMG Window: SVstart [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_subimg_window.svstart, m_subimg_window.svstart + 1
			);
		})
	);
	map(0x04c, 0x04d).lrw8(
		NAME([this] (offs_t offset) {
			return (m_subimg_window.svend >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_subimg_window.svend &= offset ? 0xff00 : 0x00ff;
			m_subimg_window.svend |= (data << shift);
			LOG("SUBIMG Window: SVend [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_subimg_window.svend, m_subimg_window.svend
			);
		})
	);
	map(0x04e, 0x04f).lrw8(
		NAME([this] (offs_t offset) {
			return (m_subimg_window.shstart >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_subimg_window.shstart &= offset ? 0x0300 : 0x00ff;
			m_subimg_window.shstart |= (data << shift);
			LOG("SUBIMG Window: SHstart [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_subimg_window.shstart, m_subimg_window.shstart + 1
			);
		})
	);
	map(0x050, 0x051).lrw8(
		NAME([this] (offs_t offset) {
			return (m_subimg_window.shend >> (offset ? 0 : 8)) & 0xff; }
		),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset ? 0 : 8;
			m_subimg_window.shend &= offset ? 0x0300 : 0x00ff;
			m_subimg_window.shend |= (data << shift);
			LOG("SUBIMG Window: SHend [%01x] %02x -> %04x %d\n"
				, offset, data
				, m_subimg_window.shend, m_subimg_window.shend + 1
			);
		})
	);

	// JPEG Markers Array
	map(0x060, 0x3ff).ram();
}

/**************************************
 *
 * Codec Control Registers
 *
 *************************************/

/*
 * [0x000] LOAD Parameters
 * x--- ---- LOAD parameters if 1
 * ---- ---x SyncRst resets video sync generator if 1 (keeps h/vpos to 0,0 until goes off)
 */
u8 zr36060_device::load_r(offs_t offset)
{
	return (m_load << 7) | (m_syncrst << 0);
}

void zr36060_device::load_w(offs_t offset, u8 data)
{
	m_load = !!(BIT(data, 7));
	m_syncrst = !!(BIT(data, 0));
	LOG("LOAD Parameters %02x: LOAD %d SYNCRST %d\n", data, m_load, m_syncrst);
}

/*
 * [0x001] Code FIFO status (r/o)
 * x--- ---- Busy Status for LOAD operation (takes ~100ms to complete)
 * ---- -x-- CBUSY Code FIFO empty/full condition (same as /CBUSY pin)
 * ---- -0-- Code FIFO not full/not empty
 * ---- -1-- Code FIFO full/empty
 * ---- --xx CFIFO Code FIFO fullness state
 * ---- --00 < 1/4
 * ---- --01 >= 1/4 && < 1/2
 * ---- --10 >= 1/2 && < 3/4
 * ---- --11 >= 3/4
 */
u8 zr36060_device::code_fifo_status_r(offs_t offset)
{
	return 0;
}

/*
 * [0x002] Code Interface
 * x--- ---- Code16 code bus width, slave mode only
 * 0--- ---- 8-bit code bus
 * 1--- ---- 16-bit code bus
 * -x-- ---- Endian, must be 0 in Master mode or 8-bit Slave
 * ---- -x-- CFIS number of clocks in Master mode, must be 0 in Code Slave
 * ---- ---x CodeMstr ZR36060 is Master or Slave of the Code bus
 * ---- ---0 Code Slave mode
 * ---- ---1 Code Master mode
 */
u8 zr36060_device::code_if_r(offs_t offset)
{
	return (m_code16 << 7) | (m_endian << 6) | (m_cfis << 2) | (m_code_mstr);
}

void zr36060_device::code_if_w(offs_t offset, u8 data)
{
	m_code16 = !!(BIT(data, 7));
	m_endian = !!(BIT(data, 6));
	m_cfis = !!(BIT(data, 2));
	m_code_mstr = !!(BIT(data, 0));
	LOG("Code Interface %02x:\n", data);
	LOG("\tCODE16 %d-bit, Endian %d, CFIS %d VCLKx2, CodeMstr %s\n"
		, m_code16 ? 16 : 8
		, m_endian
		, m_cfis + 1
		, m_code_mstr ? "Code Master" : "Code Slave"
	);
}

/*
 * [0x003] Codec Mode
 * x--- ---- COMP
 * -x-- ---- ATP
 * --x- ---- PASS2
 * ---x ---- TLM
 * ---- -x-- BRC
 * ---- --x- FSF
 * 1100 0100 Auto Two-Pass Compression
 * 1000 0100 Statistical Compression Pass
 * 1010 0100 Compression Pass with Variable Scale Factor
 * 1010 0110 Compression Pass with Fixed Scale Factor
 * 1011 0000 Tables-Only Compression Pass
 * 0000 0000 Decompression Pass
 * ???? ???? <any other value illegal>
 */
u8 zr36060_device::codec_mode_r(offs_t offset)
{
	return m_codec_mode;
}

void zr36060_device::codec_mode_w(offs_t offset, u8 data)
{
	m_codec_mode = data;
	LOG("Codec Mode %02x\n", data);
	LOG("\tCOMP %d, ATP %d, PASS2 %d, TLM %d, BRC %d, FSF %d\n"
		, BIT(data, 7)
		, BIT(data, 6)
		, BIT(data, 5)
		, BIT(data, 4)
		, BIT(data, 2)
		, BIT(data, 1)
	);
}

/*
 * [0x005] Maxiumum Block Code Volume
 * xxxx xxxx value * 2
 */
u8 zr36060_device::mbcv_r(offs_t offset)
{
	return m_mbcv;
}

void zr36060_device::mbcv_w(offs_t offset, u8 data)
{
	m_mbcv = data;
	LOG("Maximum Block Code Volume %02x -> %d\n", m_mbcv, m_mbcv * 2);
}

/*
 * [0x006] Markers Enable [for Compression modes only]
 * x--- ---- APP
 * -x-- ---- COM
 * --x- ---- DRI
 * ---x ---- DQT
 * ---- x--- DHT
 * ---- ---? <unknown, used on HW test startup and discarded quickly afterwards>
 */
u8 zr36060_device::markers_enable_r(offs_t offset)
{
	return m_markers_enable;
}

void zr36060_device::markers_enable_w(offs_t offset, u8 data)
{
	m_markers_enable = data;
	LOG("Markers Enable %02x\n", data);
	LOG("\tAPP %d COM %d DRI %d DQT %d DHT %d\n"
		, BIT(data, 7)
		, BIT(data, 6)
		, BIT(data, 5)
		, BIT(data, 4)
		, BIT(data, 3)
	);
}

/*
 * [0x007] Interrupt Mask
 * ---- x--- EOAV End-Of-Active-Video
 * ---- -x-- EOI
 * ---- --x- END
 * ---- ---x DATERR
 */
u8 zr36060_device::irq_mask_r(offs_t offset)
{
	return m_irq_mask;
}

void zr36060_device::irq_mask_w(offs_t offset, u8 data)
{
	m_irq_mask = data;
	LOG("IRQ Mask %02x\n", data);
}

/*
 * [0x008] Interrupt Status
 * same as above plus
 * xx-- ---- ProCnt 2-bit cyclic Process counter
 */
u8 zr36060_device::irq_status_r(offs_t offset)
{
	return m_irq_status;
}

/*
 * [0x009 - 0x00c] Target Net Code Volume
 */
u8 zr36060_device::tcv_net_r(offs_t offset)
{
	return m_tcv_net[offset];
}

void zr36060_device::tcv_net_w(offs_t offset, u8 data)
{
	m_tcv_net[offset] = data;
	LOG("Target Net Code Volume [%01x] %02x\n", offset, data);
}

/*
 * [0x00d - 0x010] Target Data Code Volume
 */
u8 zr36060_device::tcv_data_r(offs_t offset)
{
	return m_tcv_data[offset];
}

void zr36060_device::tcv_data_w(offs_t offset, u8 data)
{
	m_tcv_data[offset] = data;
	LOG("Target Data Code Volume [%01x] %02x\n", offset, data);
}

/*
 * [0x011 - 0x012] Scale Factor
 */
 u8 zr36060_device::sf_r(offs_t offset)
 {
	 return m_sf[offset];
 }

void zr36060_device::sf_w(offs_t offset, u8 data)
{
	m_sf[offset] = data;
	LOG("Scale Factor [%01x] %02x\n", offset, data);
}

/*
 * [0x013 - 0x015] Allocation Factor
 */
u8 zr36060_device::af_r(offs_t offset)
{
	return m_af[offset];
}

void zr36060_device::af_w(offs_t offset, u8 data)
{
	m_af[offset] = data;
	LOG("Allocation Factor [%01x] %02x\n", offset, data);
}

/*
 * [0x016 - 0x019] Accumulated Code Volume
 */
u8 zr36060_device::acv_r(offs_t offset)
{
	return m_acv[offset];
}

void zr36060_device::acv_w(offs_t offset, u8 data)
{
	m_acv[offset] = data;
	LOG("Accumulated Code Volume [%01x] %02x\n", offset, data);
}

/*
 * [0x01a - 0x01d] Accumulated Total Activity
 */
u8 zr36060_device::act_r(offs_t offset)
{
	return m_act[offset];
}

void zr36060_device::act_w(offs_t offset, u8 data)
{
	m_act[offset] = data;
	LOG("Accumulated Total Activity [%01x] %02x\n", offset, data);
}

/*
 * [0x01e - 0x021] Accumulated Truncated Bits
 */
u8 zr36060_device::acv_trun_r(offs_t offset)
{
	return m_acv_trun[offset];
}

void zr36060_device::acv_trun_w(offs_t offset, u8 data)
{
	m_acv_trun[offset] = data;
	LOG("Accumulated Truncated Bits [%01x] %02x\n", offset, data);
}

/**************************************
 *
 * Video Registers
 *
 *************************************/

u8 zr36060_device::video_control_r(offs_t offset)
{
	return m_video_control;
}

void zr36060_device::video_control_w(offs_t offset, u8 data)
{
	m_video_control = data;
	LOG("Video Control %02x\n", data);
	LOG("\tVideo8 %d, Range %s, FIDet %s, FIVedge %s, FIExt %d SyncMstr %s\n"
		, BIT(data, 7) ? 8 : 16
		, BIT(data, 6) ? "CCIR-601" : "Full scale"
		, BIT(data, 3) ? "FI high/VSYNC latch middle of line" : "FI low/VSYNC latch HSYNC pulse"
		, BIT(data, 2) ? "Trailing edge VSYNC" : "Leading edge VSYNC"
		, BIT(data, 1) // uses FI pin if high
		, BIT(data, 0) ? "Master" : "Slave"
	);
}

u8 zr36060_device::video_polarity_r(offs_t offset)
{
	return m_video_polarity;
}

void zr36060_device::video_polarity_w(offs_t offset, u8 data)
{
	m_video_polarity = data;
	LOG("Video Polarity %02x\n", data);
	LOG("\tVCLKPol %d, PValPol %d, PoePol %d, SImgPol %d, BLPol %d, FIPol %d, HSPol %d, VSPol %d\n"
		, BIT(data, 7)
		, BIT(data, 6)
		, BIT(data, 5)
		, BIT(data, 4)
		, BIT(data, 3)
		, BIT(data, 2)
		, BIT(data, 1)
		, BIT(data, 0)
	);
}

u8 zr36060_device::video_scaling_r(offs_t offset)
{
	return (m_vscale << 2) | (m_hscale);
}

void zr36060_device::video_scaling_w(offs_t offset, u8 data)
{
	m_vscale = BIT(data, 2);
	m_hscale = data & 3;
	LOG("Scaling %02x\n", data);
	LOG("\tVScale %d:1 ratio, HScale %d:1 ratio\n"
		, m_vscale + 1
		// NOTE: 8:1 is illegal
		, 1 << m_hscale
	);
}

/**************************************
 *
 * Host I/F
 *
 *************************************/

u8 zr36060_device::read(offs_t offset)
{
	switch(offset & 3)
	{
		case 0:
			LOG("CODE FIFO read\n");
			return 0;
		case 1:
			return (m_address >> 8) & 3;
		case 2:
			return m_address & 0xff;
		case 3:
			return space(0).read_byte(m_address);
	}
	return 0;
}

void zr36060_device::write(offs_t offset, u8 data)
{
	switch(offset & 3)
	{
		case 0:
			LOG("CODE FIFO write %02x\n", data);
			break;
		case 1:
			m_address = (data << 8) | (m_address & 0xff);
			break;
		case 2:
			m_address = (data & 0xff) | (m_address & 0x300);
			break;
		case 3:
			space(0).write_byte(m_address, data);
			break;
	}
}
