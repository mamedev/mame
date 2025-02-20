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
  SAA7110a (TV decoder, PAL B/G / NTSC M / SECAM) + adv7176 (TV encoder, Linux driver name is adv7175)
ZR36057 is known to have two HW quirks that are been fixed with ZR36067.

TODO:
- Currently at dc10plus HW test "Error at video decoder", requires SAA7110a to continue;
- Hookup busmaster;
- Stub, eventually decouple AV PCI controller part from the actual client cards;
- Soft Reset & Write lock mechanisms (each register have separate macro-groups);

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

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(ZR36057_PCI, zr36057_device,   "zr36057",   "Zoran ZR36057-based Enhanced Multimedia Controller")
//DEFINE_DEVICE_TYPE(ZR36067_PCI, zr36067_device,   "zr36067",   "Zoran ZR36067-based AV Controller")


zr36057_device::zr36057_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
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
	// 27'000'000 xtal near ZR36060

	SAA7110A(config, m_decoder, XTAL(26'800'000));
	m_decoder->sda_callback().set([this](int state) { m_decoder_sdao_state = state; });

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
	m_video_frontend.horizontal_config = (0 << 30) | (0x001 << 10) | (0x3ff << 0);
	m_video_frontend.vertical_config = (0 << 30) | (0x001 << 10) | (0x3ff << 0);

	m_pci_waitstate_control = 0;
	m_gpio_ddr = 0xff; // all inputs
	// GuestBus ID default
	// m_gpio_data = 0xf0;
	for (int i = 0; i < 4; i++)
		m_guestbus.time[i] = 0;
}

void zr36057_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x3e, 0x3e).lr8(NAME([] () { return 0x02; }));
	map(0x3f, 0x3f).lr8(NAME([] () { return 0x10; }));
}

// Application Specific Register
void zr36057_device::asr_map(address_map &map)
{
	map(0x000, 0x003).lrw32(
		NAME([this] (offs_t offset) {
			// NOTE: wants to read-back here, throws "Bus Master ASIC error" otherwise (?)
			LOG("Video Front End Horizontal Configuration R\n");
			return m_video_frontend.horizontal_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_video_frontend.horizontal_config);
			LOG("Video Front End Horizontal Configuration W %08x & %08x\n", data, mem_mask);
		})
	);
	map(0x004, 0x007).lrw32(
		NAME([this] (offs_t offset) {
			LOG("Video Front End Vertical Configuration R\n");
			return m_video_frontend.vertical_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_video_frontend.vertical_config);
			LOG("Video Front End Vertical Configuration %08x & %08\n", data, mem_mask);
		})
	);

	// ...

	map(0x028, 0x02b).lrw32(
		NAME([this] (offs_t offset) {
			return (m_softreset << 24) | (m_pci_waitstate_control << 16) | m_gpio_ddr;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("System, PCI and General Purpose Pins Control %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_24_31)
			{
				m_softreset = !!BIT(data, 24);
				// TODO: will lock all writes in config_map but this bit
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
}
