// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Zoran ZR36057 / ZR36067 PCI-based chipsets

PCI glue logic for multimedia MJPEG, MPEG1 & DVD.
Paired with every single TV standard for video capture in the fairly decent number of subvendor
iterations.
- https://www.kernel.org/doc/html/v6.13/driver-api/media/drivers/zoran.html
- Currently using DC10+ configuration:
  ZR36067 + ZR36060 (ZR36050 + ZR36016 glued together)
  SAA7110a (TV decoder) + adv7176 (TV encoder, Linux driver name is adv7175)
ZR36057 is known to have two HW quirks that are been fixed with ZR36067.

TODO:
- Enough to pass board functions in dc10plus HW test, requires GIRQ1 signal source to continue
(from SAA7110? On its own it doesn't touch any of the sync registers);
- Hookup busmaster;
- What are i2c 0x8e >> 1 address device checks for?
\- Can't be adv7175 (0xd4 >> 1) nor adv7176 (0x54 >> 1)
- Noisy on undocumented guest ID 7 once VidCap capturing is enabled;
...
- Soft Reset & Write lock mechanisms (each register have separate macro-groups);
- GuestBus slot mechanism (relevant when multiple devices are hooked);
- eventually decouple AV PCI controller part from the actual client cards;

Known mix-ins:
- s3virge.cpp uses SAA7110 for Scenic Highway overlay available with its S3 LPB connector;
- apple/pippin.cpp uses bt856, reused by LML33 as TV encoder;
- jaleco/tetrisp2.cpp stepstag uses adv7176;
- misc/sliver.cpp uses ZR36050 + ZR36011 + MD0208 (alias for something else?);
- misc/magictg.cpp uses ZR36120 + ZR36050 + ZR36016;
- namco/namcos23.cpp gunwars camera uses VPX3220A, shared with DC10(old) and DC30/DC30+;
- nintendo/cham24.cpp has SAA7111A on-board, reused with Iomega Buz cards in non-A variant;

**************************************************************************************************/

#include "emu.h"
#include "zr36057.h"

#define LOG_WARN      (1U << 1)
#define LOG_PO        (1U << 2) // PostOffice interactions
#define LOG_VFE       (1U << 3) // Log Video Front End
#define LOG_SYNC      (1U << 4) // Log Sync signals and Active Area (as SyncMstr)

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_VFE | LOG_SYNC)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGPO(...)              LOGMASKED(LOG_PO, __VA_ARGS__)
#define LOGVFE(...)             LOGMASKED(LOG_VFE, __VA_ARGS__)
#define LOGSYNC(...)            LOGMASKED(LOG_SYNC, __VA_ARGS__)

DEFINE_DEVICE_TYPE(ZR36057_PCI, zr36057_device,   "zr36057",   "Zoran ZR36057-based Enhanced Multimedia Controller")
//DEFINE_DEVICE_TYPE(ZR36067_PCI, zr36067_device,   "zr36067",   "Zoran ZR36067-based AV Controller")


zr36057_device::zr36057_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_guest(*this, "guest")
	, m_decoder(*this, "decoder")
{
	// ZR36057PQC Video cutting chipset
	// device ID reportedly same for ZR36057 and ZR36067, revision 0x02 for latter.
	// Known subvendors list (earlier cards don't define one):
	// - 0x10317efe: Pinnacle/Miro DC10+
	// - 0x1031d801: Pinnacle/Miro DC30+
	// - 0x1031fc00: Pinnacle/Miro DC50, Motion JPEG Capture/CODEC Board
	// - 0x12f88a02: Electronic Design GmbH Tekram Video Kit a.k.a. Linux Media Labs LML33R10
	// - 0x13ca4231: Iomega Buz JPEG/TV Card
	// NOTE: subvendor is omitted in '36057 design (missing?), driven at PCIRST time to 32 pins in
	// '36067 thru pull-up or pull-down resistors (subvendor responsibility?)
	set_ids(0x11de6057, 0x02, 0x040000, 0x10317efe);
}

zr36057_device::zr36057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: zr36057_device(mconfig, ZR36057_PCI, tag, owner, clock)
{
}

void zr36057_device::device_add_mconfig(machine_config &config)
{
	ZR36060(config, m_guest, XTAL(27'000'000));

	SAA7110A(config, m_decoder, XTAL(26'800'000));
	m_decoder->sda_callback().set([this](int state) { m_decoder_sdao_state = state; });
	//m_decoder->vs_callback().set(FUNC(zr36057_device::girq1_w));

	//ADV7176(config, m_encoder, XTAL(27'000'000));

	// S-Video input/output
	// composite video input/output

	// video and audio input/output pins on PCB, for cross connection with other boards

	// DC30 combines the two audio input/output as an external jack option
}

void zr36057_device::device_start()
{
	pci_card_device::device_start();

	add_map(4 * 1024, M_MEM, FUNC(zr36057_device::asr_map));

	// INTA#
	intr_pin = 1;
}

void zr36057_device::device_reset()
{
	pci_card_device::device_reset();

	// fast DEVSEL# (00), can enable busmaster
	command = 0x0000;
	command_mask = 0x0006;
	status = 0x0000;
	intr_line = 0x0a;
	// TODO: PCI regs 0x3e/0x3f max_lat = 0x10 (4 usec), min_gnt = 0x02 (0.5 usec)

	remap_cb();

	m_softreset = 0;
	software_reset();
}

void zr36057_device::software_reset()
{
	LOG("SoftReset\n");
	m_vfe.hspol = m_vfe.vspol = 0;
	m_vfe.hstart = m_vfe.vstart = 0x001;
	m_vfe.hend = m_vfe.vend = 0x3ff;
	m_vfe.horizontal_config = (m_vfe.hspol << 30) | (m_vfe.hstart << 10) | (m_vfe.hend << 0);
	m_vfe.vertical_config = (m_vfe.vspol << 30) | (m_vfe.vstart << 10) | (m_vfe.vend << 0);

	m_vfe.ext_fi = 0;
	m_vfe.top_field = 1;
	m_vfe.vclk_pol = 0;
	m_vfe.hfilter = 0;
	m_vfe.dup_fld = 0;
	m_vfe.hor_dcm = 0;
	m_vfe.ver_dcm = 0;
	m_vfe.disp_mod = 0;
	m_vfe.yuv2rgb = 2;
	m_vfe.err_dif = 0;
	m_vfe.pack24 = 0;
	m_vfe.little_endian = 1;
	m_vfe.format_config = (m_vfe.top_field << 25) | (m_vfe.yuv2rgb << 3) | (m_vfe.little_endian << 0);
	m_vfe.vid_bottom_base = m_vfe.vid_top_base = m_vfe.mask_bottom_base = m_vfe.mask_top_base = 0xffff'fffc;
	m_vfe.disp_stride = 0xfffc;
	m_vfe.vid_ovf = m_vfe.snapshot = m_vfe.frame_grab = false;
	m_vfe.vid_en = false;
	m_vfe.min_pix = 0xf;
	m_vfe.triton = false;
	m_vfe.window_height = 0x0f0;
	m_vfe.window_width = 0x3ff;
	m_vfe.display_config = (m_vfe.vid_en << 31) | (m_vfe.min_pix << 24) | (m_vfe.window_height << 12) | (m_vfe.window_width);

	m_vfe.ovl_enable = false;
	m_vfe.mask_stride = 0xff;

	// TODO: these also go in process_reset fn
	// defaults in MPEG mode
	m_jpeg.mode = false;
	m_jpeg.sub_mode = 3;
	m_jpeg.rtbsy_fb = m_jpeg.go_en = m_jpeg.sync_mstr = m_jpeg.fld_per_buff = false;
	m_jpeg.vfifo_fb = m_jpeg.cfifo_fb = false;
	m_jpeg.still_lendian = true;
	m_jpeg.p_reset = true;
	m_jpeg.cod_trns_en = false;
	m_jpeg.active = false;

	m_sync_gen.vsync_size = 6;
	m_sync_gen.vtotal = 525;
	m_sync_gen.hsync_start = 640;
	m_sync_gen.htotal = 780;

	m_active_area.nax = 0;
	m_active_area.pax = 640;
	m_active_area.nay = 10;
	m_active_area.pay = 240;
	m_active_area.odd = true;

	m_jpeg.guest_id = 4;
	m_jpeg.guest_reg = 0;

	m_irq_status = m_irq_enable = 0;
	m_inta_pin_enable = false;

	m_pci_waitstate_control = 0;
	m_gpio_ddr = 0xff; // all inputs
	// GuestBus ID default
	// m_gpio_data = 0xf0;
	for (int i = 0; i < 4; i++)
		m_guestbus.time[i] = 0;

	m_po.pending = false;
	m_po.time_out = false;
	m_po.dir = true;
	m_po.guest_id = 0;
	m_po.guest_reg = 0;
}

void zr36057_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x3e, 0x3e).lr8(NAME([] () { return 0x02; }));
	map(0x3f, 0x3f).lr8(NAME([] () { return 0x10; }));
}

// Application Specific Register(s)
void zr36057_device::asr_map(address_map &map)
{
	map(0x000, 0x003).lrw32(
		NAME([this] (offs_t offset) {
			// NOTE: wants to read-back here, throws "Bus Master ASIC error" otherwise (?)
			LOGVFE("Video Front End: Horizontal Configuration R\n");
			return m_vfe.horizontal_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.horizontal_config);
			LOGVFE("Video Front End: Horizontal Configuration W %08x & %08x\n", data, mem_mask);
			m_vfe.hspol = BIT(m_vfe.horizontal_config, 30);
			m_vfe.hstart = (m_vfe.horizontal_config >> 10) & 0x3ff;
			m_vfe.hend = (m_vfe.horizontal_config >> 0) & 0x3ff;
			LOGVFE("\tHSPol %d, HStart %d, HEnd %d\n", m_vfe.hspol, m_vfe.hstart, m_vfe.hend);
		})
	);
	map(0x004, 0x007).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Video Front End: Vertical Configuration R\n");
			return m_vfe.vertical_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.vertical_config);
			LOGVFE("Video Front End: Vertical Configuration W %08x & %08x\n", data, mem_mask);
			m_vfe.vspol = BIT(m_vfe.vertical_config, 30);
			m_vfe.vstart = (m_vfe.vertical_config >> 10) & 0x3ff;
			m_vfe.vend = (m_vfe.vertical_config >> 0) & 0x3ff;
			LOGVFE("\tVSPol %d, VStart %d, VEnd %d\n", m_vfe.vspol, m_vfe.vstart, m_vfe.vend);
		})
	);
	map(0x008, 0x00b).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Video Front End: Format Configuration R\n");
			return m_vfe.format_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.format_config);
			LOGVFE("Video Front End: Format Configuration W %08x & %08x\n", data, mem_mask);
			m_vfe.ext_fi = BIT(m_vfe.format_config, 26);
			m_vfe.top_field = BIT(m_vfe.format_config, 25);
			m_vfe.vclk_pol = BIT(m_vfe.format_config, 24);
			LOGVFE("\tExtFI %d, TopField %d, VCLKPol %d\n"
				, m_vfe.ext_fi, m_vfe.top_field, m_vfe.vclk_pol
			);
			// Video Scaler
			m_vfe.hfilter = (m_vfe.format_config >> 21) & 7;
			m_vfe.dup_fld = BIT(m_vfe.format_config, 20);
			m_vfe.hor_dcm = (m_vfe.format_config >> 14) & 0x3f;
			m_vfe.ver_dcm = (m_vfe.format_config >> 8) & 0x3f;
			LOGVFE("\tHFilter %d, DupFld %d, HorDcm %d, VerDcm %d\n"
				, m_vfe.hfilter
				, m_vfe.dup_fld
				, m_vfe.hor_dcm
				, m_vfe.ver_dcm
			);
			// Pixel Formatter
			m_vfe.disp_mod = BIT(m_vfe.format_config, 6);
			m_vfe.yuv2rgb = (m_vfe.format_config >> 3) & 3;
			m_vfe.err_dif = BIT(m_vfe.format_config, 2);
			m_vfe.pack24 = BIT(m_vfe.format_config, 1);
			m_vfe.little_endian = BIT(m_vfe.format_config, 0);
			LOGVFE("\tDispMod %d, YUV2RGB %d, ErrDif %d, Pack24 %d, LittleEndian %d\n"
				, m_vfe.disp_mod
				, m_vfe.yuv2rgb
				, m_vfe.err_dif
				, m_vfe.pack24
				, m_vfe.little_endian
			);
		})
	);
	map(0x00c, 0x00f).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Video Display Top R\n");
			return m_vfe.vid_top_base;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.vid_top_base);
			m_vfe.vid_top_base &= 0xffff'fffc;
			LOGVFE("Video Display Top W %08x & %08x\n", data, mem_mask);
		})
	);
	map(0x010, 0x013).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Video Display Top R\n");
			return m_vfe.vid_bottom_base;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.vid_bottom_base);
			m_vfe.vid_bottom_base &= 0xffff'fffc;
			LOGVFE("Video Display Bottom W %08x & %08x\n", data, mem_mask);
		})
	);
	map(0x014, 0x017).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Video Stride, Status and Frame Grab R\n");
			return (m_vfe.disp_stride << 16) | (m_vfe.vid_ovf << 8) | (m_vfe.snapshot << 1) | (m_vfe.frame_grab << 0);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGVFE("Video Stride, Status and Frame Grab W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_16_31)
			{
				COMBINE_DATA(&m_vfe.disp_stride);
				m_vfe.disp_stride &= 0xfffc;
				LOGVFE("\tDispStride %d\n", m_vfe.disp_stride);
			}

			if (ACCESSING_BITS_8_15)
			{
				// bit 8 high clears the overflow flag
				if (BIT(data, 8))
				{
					m_vfe.vid_ovf = false;
					LOGVFE("\tVidOvf clear\n");
				}
			}

			if (ACCESSING_BITS_0_7)
			{
				m_vfe.snapshot = !!(BIT(data, 1));
				m_vfe.frame_grab = !!(BIT(data, 0));
				LOGVFE("\tSnapShot %d, FrameGrab %d\n", m_vfe.snapshot, m_vfe.frame_grab);
				// Presumably this will stall the emulation (goes off after two fields)
				if (m_vfe.frame_grab)
					popmessage("zr36057: FrameGrab enabled");
			}
		})
	);
	map(0x018, 0x01b).lrw32(
		NAME([this] (offs_t offset) {
			LOG("Video Display Configuration R\n");
			return m_vfe.display_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.display_config);
			LOGVFE("Video Display Configuration W %08x & %08x\n", data, mem_mask);
			m_vfe.vid_en = !!BIT(m_vfe.display_config, 31);
			// Not a mistake: min_pix overlaps with the /Triton setting
			m_vfe.min_pix = (m_vfe.display_config >> 24) & 0x7f;
			m_vfe.triton = !!BIT(~m_vfe.display_config, 24);
			LOGVFE("\tVidEn %d, MinPix %d, Triton %s Controller\n"
				, m_vfe.vid_en
				, m_vfe.min_pix
				, m_vfe.triton ? "Intel 'Triton' Bridge" : "Other PCI Bridge"
			);
			m_vfe.window_height = (m_vfe.display_config >> 12) & 0x3ff;
			m_vfe.window_width = (m_vfe.display_config >> 0) & 0x3ff;
			LOGVFE("\tVidWinHt %d, VidWinWid %d\n", m_vfe.window_height, m_vfe.window_width);
		})
	);
	map(0x01c, 0x01f).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Masking Map Top R\n");
			return m_vfe.mask_top_base;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.mask_top_base);
			m_vfe.mask_top_base &= 0xffff'fffc;
			LOGVFE("Masking Map Top W %08x & %08x\n", data, mem_mask);
		})
	);
	map(0x020, 0x023).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Masking Map Bottom R\n");
			return m_vfe.mask_bottom_base;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.mask_bottom_base);
			m_vfe.mask_bottom_base &= 0xffff'fffc;
			LOGVFE("Masking Map Bottom W %08x & %08x\n", data, mem_mask);
		})
	);
	map(0x024, 0x027).lrw32(
		NAME([this] (offs_t offset) {
			LOGVFE("Overlay Control R\n");
			return (m_vfe.ovl_enable << 15) | (m_vfe.mask_stride << 0);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGVFE("Overlay Control W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_8_15)
			{
				m_vfe.ovl_enable = !!(BIT(data, 15));
				LOGVFE("\tOvlEnable %d\n", m_vfe.ovl_enable);
			}

			if (ACCESSING_BITS_0_7)
			{
				m_vfe.mask_stride = data & 0xff;
				LOGVFE("\tMaskStride %d\n", m_vfe.mask_stride);
			}
		})
	);
	map(0x028, 0x02b).lrw32(
		NAME([this] (offs_t offset) {
			return (m_softreset << 24) | (m_pci_waitstate_control << 16) | m_gpio_ddr;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("System, PCI and General Purpose Pins Control %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_24_31)
			{
				m_softreset = !!BIT(data, 24);
				// TODO: will lock all writes in asr_map but this bit
				// (inclusive of the "all" group?)
				if (!m_softreset)
				{
					software_reset();
					return;
				}
			}
			if (ACCESSING_BITS_16_23)
				m_pci_waitstate_control = (data >> 16) & 7;

			if (ACCESSING_BITS_0_7)
				m_gpio_ddr = data & 0xff;
		})
	);
	map(0x02c, 0x02f).lrw32(
		NAME([this] (offs_t offset) {
			LOG("General Purpose Pins and GuestBus Control R\n");

			// The doc claims 0xf0 default for GPIO, but win98 driver will throw "subvendor ID failed"
			// while testing various ID combinations here
			// This should come from GDAT pin at strapping time
			return (0x7e << 24) | (m_guestbus.time[3] << 12) | (m_guestbus.time[2] << 8) | (m_guestbus.time[1] << 4) | (m_guestbus.time[0] << 0);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("General Purpose Pins and GuestBus Control W %08x & %08x\n", data, mem_mask);

			// if (ACCESSING_BITS_24_31)
			// GenPurIO writes, TBD

			if (ACCESSING_BITS_8_15)
			{
				m_guestbus.time[3] = (data >> 12) & 0xf;
				m_guestbus.time[2] = (data >> 8) & 0xf;
			}

			if (ACCESSING_BITS_0_7)
			{
				m_guestbus.time[1] = (data >> 4) & 0xf;
				m_guestbus.time[0] = (data >> 0) & 0xf;
			}
		})
	);
//  map(0x030, 0x033) MPEG Code Source Address
//  map(0x034, 0x037) MPEG Code Transfer Control
//  map(0x038, 0x03b) MPEG Code Memory Pointer
	map(0x03c, 0x03f).lrw32(
		NAME([this] (offs_t offset) {
			return m_irq_status;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_24_31)
			{
				m_irq_status &= ~data;
				m_irq_status &= 0x7800'0000;
				update_irq_status();
			}
		})
	);
	map(0x040, 0x043).lrw32(
		NAME([this] (offs_t offset) {
			return m_irq_enable | (m_inta_pin_enable << 24);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_24_31)
			{
				LOG("IRQ Enable W %08x & %08x\n", data, mem_mask);
				m_irq_enable = data;
				m_irq_enable &= 0x7800'0000;
				m_inta_pin_enable = BIT(data, 24);
				LOG("\tINTA enable %d\n", m_inta_pin_enable);
				LOG("\tGIRQ1 %d GIRQ0 %d CodRepIRQ %d JPEGRepIRQ\n"
					, BIT(data, 30)
					, BIT(data, 29)
					, BIT(data, 28)
					, BIT(data, 27)
				);
				update_irq_status();
			}
		})
	);
	map(0x044, 0x047).lrw32(
		NAME([this] (offs_t offset) {
			//LOG("I2C R\n");
			// avoid win98 stall for now
			return m_decoder_sdao_state << 1 | 1;
			//return 3;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			//LOG("I2C %02x %08x\n", data, mem_mask);
			if (ACCESSING_BITS_0_7)
			{
				m_decoder->sda_write(BIT(data, 1));
				m_decoder->scl_write(BIT(data, 0));
			}
		})
	);
	map(0x100, 0x103).lrw32(
		NAME([this] (offs_t offset) {
			LOG("JPEG Mode and Control R\n");
			return (m_jpeg.mode << 31) | (m_jpeg.sub_mode << 29)
				| (m_jpeg.rtbsy_fb << 6) | (m_jpeg.go_en << 5) | (m_jpeg.sync_mstr << 4)
				| (m_jpeg.fld_per_buff << 3) | (m_jpeg.vfifo_fb << 2)
				| (m_jpeg.cfifo_fb << 1) | (m_jpeg.still_lendian << 0);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("JPEG Mode and Control W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_24_31)
			{
				m_jpeg.mode = !!BIT(data, 31);
				// --x- (0) Still Image (1) Motion Video
				// ---x (0) Decompression (1) Compression
				m_jpeg.sub_mode = (data >> 29) & 3;
				LOG("\tJPG %s Mode, JPGMode %d\n"
					, m_jpeg.mode ? "JPEG" : "MPEG"
					, m_jpeg.sub_mode
				);
			}

			if (ACCESSING_BITS_0_7)
			{
				m_jpeg.rtbsy_fb = !!BIT(data, 6);
				m_jpeg.go_en = !!BIT(data, 5);
				m_jpeg.sync_mstr = !!BIT(data, 4);
				m_jpeg.fld_per_buff = !!BIT(data, 3);
				m_jpeg.vfifo_fb = !!BIT(data, 2);
				m_jpeg.cfifo_fb = !!BIT(data, 1);
				m_jpeg.still_lendian = !!BIT(data, 0);
				LOG("\tRTBSY_FB %d, Go_en %d, SyncMstr %d, Fld_per_buff %s, VFIFO_FB %d, CFIFO_FB %d, Still_LitEndian %s\n"
					, m_jpeg.rtbsy_fb
					, m_jpeg.go_en
					, m_jpeg.sync_mstr
					, m_jpeg.fld_per_buff ? "1 (One code field)" : "0 (Two code fields, one code frame)"
					, m_jpeg.vfifo_fb
					, m_jpeg.cfifo_fb
					, m_jpeg.still_lendian ? "Little" : "Gib"
				);
				if (m_jpeg.go_en)
					popmessage("zr36057.cpp: go_en enabled");
			}
		})
	);
	map(0x104, 0x107).lrw32(
		NAME([this] (offs_t offset) {
			LOG("JPEG Process Control R\n");
			return (m_jpeg.p_reset << 7) | (m_jpeg.cod_trns_en << 5) | (m_jpeg.active << 0);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("JPEG Process Control W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_0_7)
			{
				m_jpeg.p_reset = !!BIT(data, 7);
				m_jpeg.cod_trns_en = !!BIT(data, 5);
				m_jpeg.active = !!BIT(data, 0);
				LOG("\tP_reset %d, CodTrnsEn %d, Active %d\n"
					, m_jpeg.p_reset
					, m_jpeg.cod_trns_en
					, m_jpeg.active
				);
			}
		})
	);
	map(0x108, 0x10b).lrw32(
		NAME([this] (offs_t offset) {
			LOGSYNC("Vertical Sync Parameters R\n");
			return (m_sync_gen.vsync_size << 16) | (m_sync_gen.vtotal);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGSYNC("Vertical Sync Parameters W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_16_23)
			{
				m_sync_gen.vsync_size = (data >> 16) & 0xff;
				LOGSYNC("\tVsyncSize %d\n", m_sync_gen.vsync_size);
			}

			if (ACCESSING_BITS_0_15)
			{
				m_sync_gen.vtotal = data & 0xffff;
				LOGSYNC("\tFrmTot %d\n", m_sync_gen.vtotal);
			}
		})
	);
	map(0x10c, 0x10f).lrw32(
		NAME([this] (offs_t offset) {
			LOGSYNC("Horizontal Sync Parameters R\n");
			return (m_sync_gen.hsync_start << 16) | (m_sync_gen.htotal);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGSYNC("Horizontal Sync Parameters W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_16_31)
			{
				m_sync_gen.hsync_start = (data >> 16) & 0xffff;
				LOGSYNC("\tHsyncStart %d\n", m_sync_gen.hsync_start);
			}

			if (ACCESSING_BITS_0_15)
			{
				m_sync_gen.htotal = data & 0xffff;
				LOGSYNC("\tLineTot %d\n", m_sync_gen.htotal);
			}
		})
	);
	map(0x110, 0x113).lrw32(
		NAME([this] (offs_t offset) {
			LOGSYNC("Field Horizontal Active Portion R\n");
			return (m_active_area.nax << 16) | (m_active_area.pax);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGSYNC("Field Horizontal Active Portion W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_16_31)
			{
				m_active_area.nax = (data >> 16) & 0xffff;
				LOGSYNC("\tNAX %d\n", m_active_area.nax);
			}

			if (ACCESSING_BITS_0_15)
			{
				m_active_area.pax = data & 0xffff;
				LOGSYNC("\tPAX %d\n", m_active_area.pax);
			}
		})
	);
	map(0x114, 0x117).lrw32(
		NAME([this] (offs_t offset) {
			LOGSYNC("Field Vertical Active Portion R\n");
			return (m_active_area.nay << 16) | (m_active_area.pay);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGSYNC("Field Vertical Active Portion W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_16_31)
			{
				m_active_area.nay = (data >> 16) & 0xffff;
				LOGSYNC("\tNAY %d\n", m_active_area.nay);
			}

			if (ACCESSING_BITS_0_15)
			{
				m_active_area.pay = data & 0xffff;
				LOGSYNC("\tPAY %d\n", m_active_area.pay);
			}
		})
	);
	map(0x118, 0x11b).lrw32(
		NAME([this] (offs_t offset) {
			LOGSYNC("Field Process Parameters R\n");
			return m_active_area.odd;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGSYNC("Field Process Parameters W %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_0_7)
			{
				m_active_area.odd = !!BIT(data, 0);
				LOGSYNC("\tOdd_Even: %s\n", m_active_area.odd ? "Odd" : "Even");
			}
		})
	);
//  map(0x11c, 0x11f) JPEG Code Base Address
//  map(0x120, 0x123) JPEG Code FIFO Threshold
	map(0x124, 0x124).lrw8(
		NAME([this] (offs_t offset) {
			LOG("JPEG Codec Guest ID R\n");
			return (m_jpeg.guest_id << 4) | (m_jpeg.guest_reg << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_jpeg.guest_id = (data >> 4) & 7;
			m_jpeg.guest_reg = (data >> 0) & 7;
			LOG("JPEG Codec Guest ID W %02x\n", data);
		})
	);
	map(0x12c, 0x12f).lrw32(
		NAME([this] (offs_t offset) {
			LOG("GuestBus Control (II) R\n");

			return (0 << 16) | (m_guestbus.time[7] << 12) | (m_guestbus.time[6] << 8) | (m_guestbus.time[5] << 4) | (m_guestbus.time[4] << 0);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("GuestBus Control (II) W %08x & %08x\n", data, mem_mask);

			if (ACCESSING_BITS_8_15)
			{
				m_guestbus.time[7] = (data >> 12) & 0xf;
				m_guestbus.time[6] = (data >> 8) & 0xf;
			}

			if (ACCESSING_BITS_0_7)
			{
				m_guestbus.time[5] = (data >> 4) & 0xf;
				m_guestbus.time[4] = (data >> 0) & 0xf;
			}
		})
	);

	map(0x200, 0x2ff).rw(FUNC(zr36057_device::postoffice_r), FUNC(zr36057_device::postoffice_w));
//  map(0x300, 0x303) Still Transfer
}

// TODO: PostOffice accesses thru GuestBus are dictated with PCI clock cycles, asynchronous
// We pretend they are synchronous as a starting point, and one port only (which may matter later
// with CODE transfers). A time out happens 64 PCI cycles later in ZR36067, '120 halves that threshold.
// This should eventually be expressed in a osd_work_queue, with guestbus address_space roughly as:
// for (int i = 0; i < 8; i++)
// {
//  if (<is_device_installed>)
//      map(0 | (i << 2), 3 | (i << 2)).flags(<fn>).m(m_guest[i], map);
//  else
//      map(0 | (i << 2), 3 | (i << 2)).flags(<abort_fn>);
// }
u32 zr36057_device::postoffice_r(offs_t offset)
{
	//LOGPO("PO R %d %d\n", m_po.guest_id, m_po.guest_reg);
	u8 res = 0;
	if (m_po.guest_id == 0)
	{
		if (m_po.dir == false)
			res = m_guest->read(m_po.guest_reg);
	}
	else if (!machine().side_effects_disabled())
	{
		m_po.time_out = true;
		LOGWARN("Warning: PO access unmapped POGuestID read %d %02x\n", m_po.guest_id, m_po.guest_reg);
	}

	return (m_po.pending << 25)
		| (m_po.time_out << 24)
		| (m_po.dir << 23)
		| (m_po.guest_id << 20)
		| (m_po.guest_reg << 16)
		| res;
}

void zr36057_device::postoffice_w(offs_t offset, u32 data, u32 mem_mask)
{
	// clear a previously set time out flag
	if (BIT(data, 24))
		m_po.time_out = false;
	m_po.dir = !!(BIT(data, 23));
	m_po.guest_id = (data >> 20) & 7;
	m_po.guest_reg = (data >> 16) & 7;
	LOGPO("PO W [%08x] %08x & %08x PODir %s POGuestID %d POGuestReg %d POData %02x\n"
		, offset << 2
		, data
		, mem_mask
		, m_po.dir ? "Write" : "Read"
		, m_po.guest_id
		, m_po.guest_reg
		, data & 0xff
	);
	if (m_po.guest_id == 0)
	{
		if (m_po.dir == true)
			m_guest->write(m_po.guest_reg, data & 0xff);
	}
	else
	{
		m_po.time_out = true;
		LOGWARN("Warning: PO access unmapped POGuestID write %d %02x\n", m_po.guest_id, m_po.guest_reg);
	}
}

void zr36057_device::update_irq_status()
{
	if (m_inta_pin_enable)
	{
		irq_pin_w(0, m_irq_status & m_irq_enable);
	}
}
