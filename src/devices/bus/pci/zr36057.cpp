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
- Enough to pass board functions in dc10plus HW test, requires VSYNC signal from ZR36060 to continue;
- Hookup busmaster;
- What are i2c 0x8e >> 1 address device checks for?
\- Can't be adv7175 (0xd4 >> 1) nor adv7176 (0x54 >> 1)
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

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_PO)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGPO(...)              LOGMASKED(LOG_PO, __VA_ARGS__)

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

	m_pci_waitstate_control = 0;
	m_gpio_ddr = 0xff; // all inputs
	// GuestBus ID default
	// m_gpio_data = 0xf0;
	for (int i = 0; i < 4; i++)
		m_guestbus.time[i] = 0;

	m_jpeg_guest_id = 4;
	m_jpeg_guest_reg = 0;

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
			LOG("Video Front End Horizontal Configuration R\n");
			return m_vfe.horizontal_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.horizontal_config);
			LOG("Video Front End Horizontal Configuration W %08x & %08x\n", data, mem_mask);
			m_vfe.hspol = BIT(m_vfe.horizontal_config, 30);
			m_vfe.hstart = (m_vfe.horizontal_config >> 10) & 0x3ff;
			m_vfe.hend = (m_vfe.horizontal_config >> 0) & 0x3ff;
			LOG("\tVSPOL %d VSTART %d VEND %d\n", m_vfe.hspol, m_vfe.hstart, m_vfe.hend);
		})
	);
	map(0x004, 0x007).lrw32(
		NAME([this] (offs_t offset) {
			LOG("Video Front End Vertical Configuration R\n");
			return m_vfe.vertical_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_vfe.vertical_config);
			LOG("Video Front End Vertical Configuration %08x & %08\n", data, mem_mask);
			m_vfe.vspol = BIT(m_vfe.vertical_config, 30);
			m_vfe.vstart = (m_vfe.vertical_config >> 10) & 0x3ff;
			m_vfe.vend = (m_vfe.vertical_config >> 0) & 0x3ff;
			LOG("\tVSPOL %d VSTART %d VEND %d\n", m_vfe.vspol, m_vfe.vstart, m_vfe.vend);
		})
	);
//  map(0x008, 0x00b) VFE Config, Video Scaler and Pixel Format
//  map(0x00c, 0x00f) Video Display Top
//  map(0x010, 0x013) Video Display Bottom
//  map(0x014, 0x017) Video Display Stride, Status and Frame Grab
//  map(0x018, 0x01b) Video Display Configuration
//  map(0x01c, 0x01f) Masking Map Top
//  map(0x020, 0x023) Masking Map Bottom
//  map(0x024, 0x027) Overlay Control
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
//  map(0x03c, 0x03f) Interrupt Status
//  map(0x040, 0x043) Interrupt Control
	map(0x044, 0x047).lrw32(
		NAME([this] (offs_t offset) {
			LOG("I2C R\n");
			// avoid win98 stall for now
			return m_decoder_sdao_state << 1 | 1;
			//return 3;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			//printf("I2C %02x %08x\n", data, mem_mask);
			if (ACCESSING_BITS_0_7)
			{
				m_decoder->sda_write(BIT(data, 1));
				m_decoder->scl_write(BIT(data, 0));
			}
		})
	);
//  map(0x100, 0x103) JPEG Mode and Control
//  map(0x104, 0x107) JPEG Process Control
//  map(0x108, 0x10b) Vertical Sync Parameters (as sync master)
//  map(0x10c, 0x10f) Horizontal Sync Parameters (as sync master)
//  map(0x110, 0x113) Field Horizontal Active Portion
//  map(0x114, 0x117) Field Vertical Active Portion
//  map(0x118, 0x11b) Field Process Parameters
//  map(0x11c, 0x11f) JPEG Code Base Address
//  map(0x120, 0x123) JPEG Code FIFO Threshold
	map(0x124, 0x124).lrw8(
		NAME([this] (offs_t offset) {
			LOG("JPEG Codec Guest ID R\n");
			return (m_jpeg_guest_id << 4) | (m_jpeg_guest_reg << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_jpeg_guest_id = (data >> 4) & 7;
			m_jpeg_guest_reg = (data >> 0) & 7;
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
