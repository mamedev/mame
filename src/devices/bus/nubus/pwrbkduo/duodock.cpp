// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Apple Macintosh PowerBook Duo Dock
    Emulation by R. Belmont

    The Duo Dock is a docking station for the PowerBook Duo series of
    laptops.  It swallows the entire Duo into itself like a videocassette
    player and provides nearly an entire Macintosh's worth of hardware and
    ports, including:

    - A power supply to handle the Duo and the dock's hardware and periperals
    - A speaker
    - An ADB port for a full-size keyboard and mouse
    - Two NuBus slots
    - A SWIM II chip and a SuperDrive floppy drive
    - A second SCSI bus and space inside for a 3.5" hard disk
    - Built-in Sonora-like video with up to 1 MiB of VRAM, supporting
      most Apple monitors up to 16" as well as VGA and SVGA monitors
    - 2 standard Macintosh serial ports with their own Z8530 SCC
    - An optional 68882 FPU
    - A phone line interface for the Duo's optional built-in modem

    The Duo Dock II is similar but adds SONIC Ethernet and has a larger slot
    to acommodate later Duos which were slightly thicker.  A retrofit for the
    original Duo Dock to accomdate those machines was also available.

    Emulation status:
    - Video: works except for setting the clock rate
    - SCSI bus: supported, boots 7.1.1
    - SWIM II: supported
    - NuBus slots: supported
    - Serial ports: supported

    $1941B0 portrait & 16" 57.2832
    $284A90 640x480        30.240
    $284A90 VGA            25.175
    $294A30 SVGA           36.0

***************************************************************************/

#include "emu.h"
#include "duodock.h"

#include "bus/nubus/nubus.h"
#include "bus/nubus/cards.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68030.h"
#include "machine/applefdintf.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/pseudovia.h"
#include "machine/swim2.h"
#include "machine/z80scc.h"
#include "video/ariel.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#define LOG_DRQ         (1U << 1)
#define LOG_HANDSHAKE   (1U << 2)
#define LOG_VIDEO       (1U << 3)

#define VERBOSE (0)

#include "logmacro.h"

namespace {

enum {
	VSC_MonID = 1,
	VSC_Depth,
	VSC_BusInt,
	VSC_VidCtrl,
	VSC_IntClear,

	// CRTC
	VSC_HFP = 0x10,
	VSC_HS,
	VSC_HBP,
	VSC_HA,
	VSC_SyncA,
	VSC_VFP,
	VSC_VS,
	VSC_VBP,
	VSC_VA
};

static constexpr u8 VSC_Ctrl_EnHSync    = 0x04;
static constexpr u8 VSC_Ctrl_EnVSync    = 0x08;
static constexpr u8 VSC_Ctrl_EnCSync    = 0x10;

static constexpr u8 VSC_Show_Video = (VSC_Ctrl_EnHSync | VSC_Ctrl_EnVSync | VSC_Ctrl_EnCSync);

class duodock_device : public device_t, public device_pwrbkduo_card_interface
{
public:
	// construction/destruction
	duodock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	duodock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<pseudovia_device> m_pvia;
	required_device<ariel_device> m_ramdac;
	required_device<z80scc_device> m_scc;
	required_region_ptr<u32> m_rom;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c80_device> m_ncr;
	required_device<swim2_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_ioport m_monitor_config;

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void duodock_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);
	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);
	void hdsel_w(uint8_t hdsel);
	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	u8 vsc_r(offs_t offset);
	void vsc_w(offs_t offset, u8 data);
	u16 vidregs_r(offs_t offset, u16 mem_mask);
	void vidregs_w(offs_t offset, u16 data, u16 mem_mask);
	void vdac_w(offs_t offset, u8 data);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void dock_irq_w(int state);
	void vidhandler_w(u8 data);
	u8 scsi_r(offs_t offset);
	void scsi_w(offs_t offset, u8 data);
	u32 handshake_r(offs_t offset, u32 mem_mask);
	void handshake_w(offs_t offset, u32 data, u32 mem_mask);
	void scsi_irq_w(int state);
	void scsi_drq_w(int state);

	TIMER_CALLBACK_MEMBER(vbl_tick);

	floppy_image_device *m_cur_floppy;
	int m_hdsel;

	u8 m_vsc_regs[16];
	u16 m_video_regs[0x80];
	u32 m_hres, m_vres, m_htotal, m_vtotal, m_rowbytes;
	XTAL m_pclock;
	s32 m_drq;
	u32 m_holding;
	u8 m_holding_remaining;
	bool m_is_write, m_drq_completed;
	std::unique_ptr<u32[]> m_vram;
	screen_device *m_screen;
	emu_timer *m_vbl_timer;
	std::string m_fulltag;
};

void duodock_device::duodock_map(address_map &map)
{
	map(0x0010'0000, 0x001f'ffff).rw(FUNC(duodock_device::vram_r), FUNC(duodock_device::vram_w));
	map(0x0000'0000, 0x0000'1fff).rw(m_pvia, FUNC(pseudovia_device::read), FUNC(pseudovia_device::write)).mirror(0x00e00000);
	map(0x0000'2000, 0x0000'207f).rw(FUNC(duodock_device::scsi_r), FUNC(duodock_device::scsi_w)).mirror(0x00e00000);
	map(0x0000'4000, 0x0000'5fff).rw(FUNC(duodock_device::handshake_r), FUNC(duodock_device::handshake_w)).mirror(0x00e00000);
	map(0x0000'6000, 0x0000'7fff).rw(FUNC(duodock_device::handshake_r), FUNC(duodock_device::handshake_w)).mirror(0x00e00000);
	map(0x0000'8000, 0x0000'9fff).rw(FUNC(duodock_device::scc_r), FUNC(duodock_device::scc_w)).mirror(0x00e00000);
	map(0x0000'e000, 0x0000'e00f).w(FUNC(duodock_device::vdac_w)).mirror(0x00e00000);
	map(0x000f'e000, 0x000f'e1ff).rw(FUNC(duodock_device::vidregs_r), FUNC(duodock_device::vidregs_w)).umask32(0xffff0000).mirror(0x00e00000);
}

ROM_START( duodock )
	ROM_REGION32_BE(0x20000, "dock", 0)
	ROM_LOAD("ec-a697_f2.bin", 0x000000, 0x020000, CRC(708c6ebd) SHA1(be190b869b32f33bb1e59c7ae733853cd8133a4d))
ROM_END

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(monitor_config)
	PORT_START("monitor")
	PORT_CONFNAME(0x1ff, 0x06, "Monitor type")
	PORT_CONFSETTING(0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")    // "Full Page" or "Portrait"
	PORT_CONFSETTING(0x02, u8"Mac RGB Display (12\" 512\u00d7384)")             // "Rubik" (modified IIgs AppleColor RGB)
	PORT_CONFSETTING(0x04, u8"NTSC Monitor (512\u00d7384, 640\u00d7480)")
	PORT_CONFSETTING(0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")       // "High Res"
	PORT_CONFSETTING(ext(0, 0, 0), "PAL Encoder (640\u00d7480, 768\u00d7576)")
	PORT_CONFSETTING(ext(1, 1, 0), "NTSC Encoder (512\u00d7384, 640\u00d7480)")
	PORT_CONFSETTING(ext(1, 1, 3), "640x480 VGA")
	PORT_CONFSETTING(ext(2, 3, 1), "832x624 16\" RGB")                          // "Goldfish" or "16 inch RGB"
	PORT_CONFSETTING(ext(3, 0, 0), "PAL (640\u00d7480, 768\u00d7576)")
INPUT_PORTS_END


const tiny_rom_entry *duodock_device::device_rom_region() const
{
	return ROM_NAME(duodock);
}

ioport_constructor duodock_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config);
}

void duodock_device::device_add_mconfig(machine_config &config)
{
	APPLE_PSEUDOVIA(config, m_pvia, 15.6672_MHz_XTAL);
	m_pvia->set_is_aiv3();          // AIV3 is a "pure" pseudovia that doesn't use the 6522 back-compatible IER
	m_pvia->readmsc_handler().set(FUNC(duodock_device::vsc_r));
	m_pvia->writemsc_handler().set(FUNC(duodock_device::vsc_w));
	m_pvia->writevideo_handler().set(FUNC(duodock_device::vidhandler_w));
	m_pvia->irq_callback().set(FUNC(duodock_device::dock_irq_w));

	ARIEL(config, m_ramdac, 0);

	SCC8530(config, m_scc, 31.3344_MHz_XTAL / 4);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "dscsi:0", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "dscsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "dscsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "dscsi:3", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "dscsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "dscsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "dscsi:6", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "dscsi:7").option_set("ncr53c80", NCR53C80).machine_config([this](device_t *device)
	{
		ncr5380_device &adapter = downcast<ncr5380_device &>(*device);
		adapter.irq_handler().set(*this, FUNC(duodock_device::scsi_irq_w));
		adapter.drq_handler().set(*this, FUNC(duodock_device::scsi_drq_w));
	});

	SWIM2(config, m_fdc, 15.6672_MHz_XTAL);
	m_fdc->devsel_cb().set(FUNC(duodock_device::devsel_w));
	m_fdc->phases_cb().set(FUNC(duodock_device::phases_w));
	m_fdc->hdsel_cb().set(FUNC(duodock_device::hdsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	nubus_device &nubus(NUBUS(config, "nubus", 0));
	if (((nubus_slot_device *)owner())->get_nubus_bustag() != nullptr)
	{
		m_fulltag = string_format(":%s", ((nubus_slot_device *)owner())->get_nubus_bustag());
		nubus.set_space(m_fulltag.c_str(), AS_DATA);
	}
	// tell NuBus to leave the slot $E space alone
	nubus.set_bus_mode(nubus_device::nubus_mode_t::SE30);
	nubus.out_irqc_callback().set(m_pvia, FUNC(pseudovia_device::slot_irq_w<0x08>));
	nubus.out_irqd_callback().set(m_pvia, FUNC(pseudovia_device::slot_irq_w<0x10>));
	NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
}

duodock_device::duodock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	duodock_device(mconfig, DUODOCK_DUODOCK, tag, owner, clock)
{
}

duodock_device::duodock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pwrbkduo_card_interface(mconfig, *this),
	m_pvia(*this, "pseudovia"),
	m_ramdac(*this, "ramdac"),
	m_scc(*this, "scc"),
	m_rom(*this, "dock"),
	m_scsibus(*this, "dscsi"),
	m_ncr(*this, "dscsi:7:ncr53c80"),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%d", 0U),
	m_monitor_config(*this, "monitor"),
	m_cur_floppy(nullptr),
	m_hdsel(0),
	m_hres(640),
	m_vres(480),
	m_htotal(800),
	m_vtotal(525),
	m_rowbytes(640),
	m_pclock(30.24_MHz_XTAL),
	m_drq(0),
	m_holding(0),
	m_holding_remaining(0),
	m_is_write(false),
	m_drq_completed(false),
	m_screen(nullptr)
{
	std::fill_n(m_vsc_regs, 16, 0);
	std::fill_n(m_video_regs, 0x80/sizeof(u16), 0);
}

void duodock_device::device_start()
{
	m_vram = std::make_unique<u32[]>(0x100000 / sizeof(u32));

	pwrbkduo().install_bank(0xfefe0000, 0xfeffffff, m_rom);
	pwrbkduo().install_device(0x50f16000, 0x50f17fff, emu::rw_delegate(*this, FUNC(duodock_device::swim_r)), emu::rw_delegate(*this, FUNC(duodock_device::swim_w)));
	pwrbkduo().install_map(*this, 0xfe000000, 0xfefdffff, &duodock_device::duodock_map);

	save_pointer(NAME(m_vram), 0x200000 / sizeof(u32));
	save_item(NAME(m_vsc_regs));
	save_item(NAME(m_video_regs));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_rowbytes));
	save_item(NAME(m_drq));
	save_item(NAME(m_holding));
	save_item(NAME(m_holding_remaining));
	save_item(NAME(m_is_write));
	save_item(NAME(m_drq_completed));

	m_screen = &pwrbkduo().screen();
	m_screen->set_screen_update(*this, FUNC(duodock_device::screen_update));
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);

	m_vbl_timer = timer_alloc(FUNC(duodock_device::vbl_tick), this);
	m_vbl_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

void duodock_device::dock_irq_w(int state)
{
	pwrbkduo().dock_irq_w(state);
}

uint16_t duodock_device::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		pwrbkduo().maincpu().adjust_icount(-5);
	}

	u16 result = m_fdc->read((offset >> 8) & 0xf);
	return result << 8;
}

void duodock_device::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_fdc->write((offset >> 8) & 0xf, data >> 8);

	pwrbkduo().maincpu().adjust_icount(-5);
}

void duodock_device::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void duodock_device::devsel_w(uint8_t devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_fdc->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

void duodock_device::hdsel_w(uint8_t hdsel)
{
	if (m_cur_floppy)
	{
		m_cur_floppy->ss_w(hdsel);
	}
}

u16 duodock_device::scc_r(offs_t offset)
{
	return m_scc->dc_ab_r(offset) << 8;
}

void duodock_device::scc_w(offs_t offset, u16 data)
{
	m_scc->dc_ab_w(offset, data >> 8);
}

u8 duodock_device::vsc_r(offs_t offset)
{
	return m_vsc_regs[offset & 0xf];
}

void duodock_device::vsc_w(offs_t offset, u8 data)
{
	m_vsc_regs[offset & 0xf] = data;
}

u16 duodock_device::vidregs_r(offs_t offset, u16 mem_mask)
{
	if (offset == VSC_MonID)
	{
		u8 mon = m_monitor_config->read();
		u8 monitor_id = m_video_regs[VSC_MonID] & 0x7;
		u8 res;
		if (mon & 0x40)
		{
			res = 7;
			if (monitor_id == 0x4)
			{
				res = (mon >> 4) & 3;
			}
			if (monitor_id == 0x2)
			{
				res = (BIT(mon, 3) << 2) | BIT(mon, 2);
			}
			if (monitor_id == 0x1)
			{
				res = (mon & 3) << 1;
			}
		}
		else
		{
			res = mon;
		}

		LOGMASKED(LOG_VIDEO, "Sense result = %x (monitor_id %x)\n", res, monitor_id);
		return (res << 12);
	}

	return m_video_regs[offset];
}

void duodock_device::vidregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (mem_mask == 0xff00)
	{
		data >>= 8;
	}
	else
	{
		data = swapendian_int16(data);
	}

	if (offset == VSC_IntClear)
	{
		m_pvia->vbl_irq_w(CLEAR_LINE);
	}
	else
	{
		LOGMASKED(LOG_VIDEO, "VSC video write %04x to %x (%x) (PC=%08x)\n", data, offset, offset << 2, pwrbkduo().maincpu().pc());
	}

	m_video_regs[offset] = data;

	if (offset == VSC_Depth)
	{
		m_hres = m_video_regs[VSC_HA] * 16;
		m_htotal = (m_video_regs[VSC_HFP] + m_video_regs[VSC_HS] + m_video_regs[VSC_HBP] + m_video_regs[VSC_HA]) * 16;
		m_vres = m_video_regs[VSC_VA];
		m_vtotal = (m_video_regs[VSC_VFP] + m_video_regs[VSC_VS] + m_video_regs[VSC_VBP] + m_video_regs[VSC_VA]);
		LOGMASKED(LOG_VIDEO, "hres %d vres %d htotal %d vtotal %d\n", m_hres, m_vres, m_htotal, m_vtotal);
		rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
		m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, 30.24_MHz_XTAL).as_attoseconds());
	}
}

void duodock_device::vdac_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
			m_ramdac->address_w(data);
			break;
		case 1:
			m_ramdac->palette_w(data);
			break;
	}
}

void duodock_device::vidhandler_w(u8 data)
{
	LOGMASKED(LOG_VIDEO, "%02x to video GPIO (clock gen)\n", data);
}

u32 duodock_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset & 0x1f'ffff] & mem_mask;
}

void duodock_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset & 0x1f'ffff]);
}

u32 duodock_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Check for VSync enabled, HSync enabled, composite sync enabled, and blanking enabled
	// before showing an image.
	if ((m_video_regs[VSC_VidCtrl] & VSC_Show_Video) != VSC_Show_Video)
	{
		bitmap.fill(rgb_t::black());
		return 0;
	}

	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
	pen_t const *const pens = m_ramdac->pens();

	switch (m_video_regs[VSC_Depth])
	{
	case 0: // 1 bpp
		for (int y = 0; y < m_vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < m_hres / 8; x++)
			{
				u8 const pixels = vram8[(y * (m_hres / 8)) + x];

				*scanline++ = pens[(pixels & 0x80) | 0x7f];
				*scanline++ = pens[(((pixels >> 6) & 1) << 7) | 0x7f];
				*scanline++ = pens[(((pixels >> 5) & 1) << 7) | 0x7f];
				*scanline++ = pens[(((pixels >> 4) & 1) << 7) | 0x7f];
				*scanline++ = pens[(((pixels >> 3) & 1) << 7) | 0x7f];
				*scanline++ = pens[(((pixels >> 2) & 1) << 7) | 0x7f];
				*scanline++ = pens[(((pixels >> 1) & 1) << 7) | 0x7f];
				*scanline++ = pens[((pixels & 1) << 7) | 0x7f];
			}
		}
		break;

	case 1: // 2 bpp
		for (int y = 0; y < m_vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < m_hres / 4; x++)
			{
				u8 const pixels = vram8[(y * (m_hres / 4)) + x];

				*scanline++ = pens[(pixels & 0xc0) | 0x3f];
				*scanline++ = pens[(((pixels >> 4) & 0x3) << 6) | 0x3f];
				*scanline++ = pens[(((pixels >> 2) & 0x3) << 6) | 0x3f];
				*scanline++ = pens[((pixels & 0x3) << 6) | 0x3f];
			}
		}
		break;

	case 2: // 4 bpp
		for (int y = 0; y < m_vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < m_hres / 2; x++)
			{
				u8 const pixels = vram8[(y * (m_hres / 2)) + x];

				*scanline++ = pens[(pixels & 0xf0) | 0x0f];
				*scanline++ = pens[((pixels & 0x0f) << 4) | 0x0f];
			}
		}
		break;

	case 3: // 8 bpp
		for (int y = 0; y < m_vres; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < m_hres; x++)
			{
				u8 const pixels = vram8[(y * m_hres) + x];
				*scanline++ = pens[pixels];
			}
		}
		break;

	case 4: // 15 bpp
		{
			auto const vram16 = util::big_endian_cast<u16 const>(&m_vram[0]);

			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u16 const pixels = vram16[(y * m_hres) + x];
					*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
				}
			}
		}
		break;

	default:
		fatalerror("duodock: unknown video mode %d\n", m_video_regs[VSC_Depth]);
	}
	return 0;
}

TIMER_CALLBACK_MEMBER(duodock_device::vbl_tick)
{
	m_pvia->vbl_irq_w(ASSERT_LINE);

	m_vbl_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

u8 duodock_device::scsi_r(offs_t offset)
{
	return m_ncr->read(offset >> 4);
}

void duodock_device::scsi_w(offs_t offset, u8 data)
{
	m_ncr->write(offset >> 4, data);
}

u32 duodock_device::handshake_r(offs_t offset, u32 mem_mask)
{
	// if the DRQ handler completed this transfer while we were out, just return the result now
	if (m_drq_completed)
	{
		LOGMASKED(LOG_HANDSHAKE, "%s: Completed read in DRQ, returning\n", tag());
		m_drq_completed = false;
		return m_holding;
	}

	if (mem_mask == 0xff000000)
	{
		if (m_drq)
		{
			return m_ncr->dma_r() << 24;
		}
		else
		{
			LOGMASKED(LOG_HANDSHAKE, "Handshaking single byte, no DRQ\n");
			pwrbkduo().maincpu().restart_this_instruction();
			pwrbkduo().maincpu().suspend_until_trigger(1, true);
			return 0xffffffff;
		}
	}
	else if ((mem_mask == 0xffffffff) || (mem_mask == 0xffff0000))
	{
		// are we here from a restart?
		if (!m_holding_remaining)
		{
			m_holding = 0;
			if (mem_mask == 0xffffffff)
			{
				m_holding_remaining = 4;
			}
			else if (mem_mask == 0xffff0000)
			{
				m_holding_remaining = 2;
			}
			m_is_write = false;
		}

		// is a new byte available?
		if (m_drq && m_holding_remaining)
		{
			m_holding <<= 8;
			m_holding |= m_ncr->dma_r();
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "Holding %08x, remain %d\n", m_holding, m_holding_remaining);
		}

		if (!m_holding_remaining)
		{
			return m_holding;
		}

		LOGMASKED(LOG_HANDSHAKE, "Handshaking %d byte read\n", m_holding_remaining);
		pwrbkduo().maincpu().restart_this_instruction();
		pwrbkduo().maincpu().suspend_until_trigger(1, true);
		return 0xffffffff;
	}
	fatalerror("%s: Unhandled handshake read mask %08x\n", tag(), mem_mask);
	return 0xffffffff;
}

void duodock_device::handshake_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if the DRQ handler completed this transfer while we were out, we're done
	if (m_drq_completed)
	{
		LOGMASKED(LOG_HANDSHAKE, "%s: Completed write in DRQ, returning\n", tag());
		m_drq_completed = false;
		return;
	}

	if (mem_mask == 0xff000000)
	{
		if (m_drq)
		{
			m_ncr->dma_w(data >> 24);
			return;
		}
		else
		{
			LOGMASKED(LOG_HANDSHAKE, "Handshake single byte write\n");
			pwrbkduo().maincpu().restart_this_instruction();
			pwrbkduo().maincpu().suspend_until_trigger(1, true);
			return;
		}
	}
	else if ((mem_mask == 0xffffffff) || (mem_mask == 0xffff0000))
	{
		// are we here from a restart?
		if (!m_holding_remaining)
		{
			m_holding = data;
			if (mem_mask == 0xffffffff)
			{
				m_holding_remaining = 4;
			}
			else if (mem_mask == 0xffff0000)
			{
				m_holding_remaining = 2;
			}
			m_is_write = true;
		}

		// is a new byte available?
		while (m_drq && m_holding_remaining)
		{
			m_ncr->dma_w(m_holding >> 24);
			m_holding <<= 8;
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "Holding write %08x, remain %d\n", m_holding, m_holding_remaining);
		}

		if (!m_holding_remaining)
		{
			return;
		}

		LOGMASKED(LOG_HANDSHAKE, "Handshaking %d byte write %08x\n", m_holding_remaining, m_holding);
		pwrbkduo().maincpu().restart_this_instruction();
		pwrbkduo().maincpu().suspend_until_trigger(1, true);
		return;
	}
	fatalerror("%s: Unhandled handshake write mask %08x\n", tag(), mem_mask);
}

void duodock_device::scsi_irq_w(int state)
{
	m_pvia->scsi_irq_w(state);
}

void duodock_device::scsi_drq_w(int state)
{
	LOGMASKED(LOG_DRQ, "%s: 53C80 DRQ %d (was %d) (remain %d write %d)\n", tag(), state, m_drq, m_holding_remaining, m_is_write);
	if ((state) && (m_holding_remaining > 0))
	{
		if (m_is_write)
		{
			m_ncr->dma_w(m_holding >> 24);
			m_holding <<= 8;
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "DRQ: Holding write %08x, remain %d\n", m_holding, m_holding_remaining);
		}
		else
		{
			m_holding <<= 8;
			m_holding |= m_ncr->dma_r();
			m_holding_remaining--;
			LOGMASKED(LOG_HANDSHAKE, "DRQ: Holding %08x, remain %d\n", m_holding, m_holding_remaining);
		}

		if (m_holding_remaining == 0)
		{
			m_drq_completed = true;
			pwrbkduo().maincpu().trigger(1);
		}
	}
	m_drq = state;
	m_pvia->scsi_drq_w(state);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(DUODOCK_DUODOCK, device_nubus_card_interface, duodock_device, "duodock", "Duo Dock")

