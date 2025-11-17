// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Micron/XCEED Technologies Color 30HR
  Micron/XCEED Technologies MacroColor 30
  Emulation rewritten March/April 2025 by R. Belmont

  These cards are based around a custom ASIC called "Gambler" or "Maverick"
  which is a framebuffer controller and programmable CRTC.  The chips are mostly
  interchangable from both a software and pinout point of view - Gambler is
  the later version and Maverick is found on earlier production cards, with
  very minor ROM tweaks.

  The Color 30HR uses a Bt478 RAMDAC and is limited to 256 colors, but has
  a variety of resolutions while the MacroColor 30 cards use a Bt473 RAMDAC
  for 24-bit color.  There's also a MacroColor 30HR with both, but a ROM is
  not dumped for that version.

  Fs800000 - Mode A
  FsA00000 - Mode B
  FsC00000 - RAMDAC write offset (Bt478)
  FsC00004 - RAMDAC write data
  FsC00008 - RAMDAC write mask
  FsC0000C - RAMDAC read offset

  Hardware info:
  https://github.com/ZigZagJoe/Color30HR-ROM

  Most registers on this card are encrypted, or what Micron/XCEED called "muddled".

***************************************************************************/

#include "emu.h"
#include "pds30_30hr.h"

#include "video/bt47x.h"

#include "screen.h"

#define LOG_REGISTERS   (1U << 1)
#define LOG_CRTC        (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

namespace {

static constexpr offs_t     B_ExternalModeBits  = 0;
[[maybe_unused]] static constexpr offs_t B_RefreshCount = 1;
static constexpr offs_t     B_ModeRegB          = 2;
[[maybe_unused]] static constexpr offs_t B_ZoomFactor = 3;
[[maybe_unused]] static constexpr offs_t B_NS = 4;
static constexpr offs_t     B_InterruptClear    = 5;
static constexpr offs_t     B_External          = 8;

static constexpr int        B_ModeB_VideoEnable = 0;
static constexpr int        B_ModeB_IRQEnable   = 1;

// These external bits are specific to card ID 369, the 30HR and are different on the MacroColor 30
static constexpr int        B_External_Bartlett = 2;
[[maybe_unused]] static constexpr int B_External_NoCable  = 3;      // 0 = cable present, 1 = no cable
static constexpr int        B_External_CBlank   = 7;

// encode bits 8-10 to bits 20-23
static constexpr u8 muddle_table[8] =
{
	0xe, 0xc, 0xa, 0xb, 0x6, 0x4, 0x2, 0x0
};

// decode bits 20-23 to get the original bits 8-10
static constexpr u8 demuddle_table[16] =
{
	0x7, 0x0, 0x6, 0x0, 0x5, 0x0, 0x4, 0x0,
	0x0, 0x0, 0x2, 0x3, 0x1, 0x0, 0x0, 0x0
};

class maverick_device : public device_t,
						public device_nubus_card_interface
{
protected:
	maverick_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	TIMER_CALLBACK_MEMBER(vbl_tick);

	void base_map(address_map &map);

	void device_start_common(u32 vram_size);
	virtual u16 read_external_signals();
	virtual u8 translate_mode();

	u32 aregs_r(offs_t offset);
	void aregs_w(offs_t offset, u32 data);
	u32 bregs_r(offs_t offset);
	void bregs_w(offs_t offset, u32 data);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	screen_device *m_maverick_screen;
	bt47x_device_base *m_maverick_ramdac;

	u32 m_aregs[0x10], m_bregs[0x10];

	u32 m_skipbytes;
	u32 m_hres, m_vres, m_htotal, m_vtotal, m_pclock, m_rowbytes;

private:
	u32 encrypt(u32 value);
	u32 decrypt(u32 value);

	void compute_video_mode();

	std::unique_ptr<u32[]> m_vram;
	emu_timer *m_timer;
};

void maverick_device::base_map(address_map &map)
{
	map(0x80'0000, 0x80'003f).rw(FUNC(maverick_device::aregs_r), FUNC(maverick_device::aregs_w));
	map(0xa0'0000, 0xa0'003f).rw(FUNC(maverick_device::bregs_r), FUNC(maverick_device::bregs_w));
}

maverick_device::maverick_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_skipbytes(1024),
	m_hres(640),
	m_vres(480),
	m_htotal(896),
	m_vtotal(525),
	m_rowbytes(1024),
	m_timer(nullptr)
{
	std::fill_n(&m_aregs[0], 0x10, 0);
	std::fill_n(&m_bregs[0], 0x10, 0);
}

void maverick_device::device_start_common(u32 vram_size)
{
	install_declaration_rom("declrom");

	m_vram = make_unique_clear<u32[]>(vram_size / sizeof(u32));

	m_timer = timer_alloc(FUNC(maverick_device::vbl_tick), this);
	m_timer->adjust(m_maverick_screen->time_until_pos(479, 0), 0);

	save_pointer(NAME(m_vram), vram_size / sizeof(u32));
	save_pointer(NAME(m_aregs), 0x10);
	save_pointer(NAME(m_bregs), 0x10);
}

TIMER_CALLBACK_MEMBER(maverick_device::vbl_tick)
{
	if (BIT(m_bregs[B_ModeRegB], B_ModeB_IRQEnable))
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_maverick_screen->time_until_pos(479, 0), 0);
}

u8 maverick_device::translate_mode()
{
	return m_bregs[B_ExternalModeBits] & 3;
}

u32 maverick_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_bregs[B_ModeRegB], B_ModeB_VideoEnable))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + m_skipbytes;
	pen_t const *const pens = m_maverick_ramdac->pens();

	switch (translate_mode())
	{
		case 4: // 24 bpp
			for (int y = 0; y < m_vres; y++)
			{
				std::copy_n(&m_vram[(y * m_rowbytes)], m_hres, &bitmap.pix(y));
			}
			break;

		case 3: // 1 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/8; x++)
				{
					u8 const pixels = vram8[(y * m_rowbytes) + x];

					*scanline++ = pens[BIT(pixels, 7)];
					*scanline++ = pens[BIT(pixels, 6)];
					*scanline++ = pens[BIT(pixels, 5)];
					*scanline++ = pens[BIT(pixels, 4)];
					*scanline++ = pens[BIT(pixels, 3)];
					*scanline++ = pens[BIT(pixels, 2)];
					*scanline++ = pens[BIT(pixels, 1)];
					*scanline++ = pens[BIT(pixels, 0)];
				}
			}
			break;

		case 2: // 2 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/4; x++)
				{
					u8 const pixels = vram8[(y * m_rowbytes) + x];

					*scanline++ = pens[((pixels>>6)&3)];
					*scanline++ = pens[((pixels>>4)&3)];
					*scanline++ = pens[((pixels>>2)&3)];
					*scanline++ = pens[(pixels&3)];
				}
			}
			break;

		case 1: // 4 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/2; x++)
				{
					u8 const pixels = vram8[(y * m_rowbytes) + x];

					*scanline++ = pens[(pixels>>4)];
					*scanline++ = pens[(pixels&0xf)];
				}
			}
			break;

		case 0: // 8 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u8 const pixels = vram8[(y * m_rowbytes) + x];
					*scanline++ = pens[pixels];
				}
			}
			break;
	}
	return 0;
}

u32 maverick_device::aregs_r(offs_t offset)
{
	return encrypt(m_aregs[offset]);
}

void maverick_device::aregs_w(offs_t offset, u32 data)
{
	LOGMASKED(LOG_REGISTERS, "aregs_w: %08x @ %x (crypt %08x) %s\n", decrypt(data), offset, data, machine().describe_context().c_str());
	m_aregs[offset] = decrypt(data);
}

u32 maverick_device::bregs_r(offs_t offset)
{
	LOGMASKED(LOG_REGISTERS, "bregs_r: @ %x, %s\n", offset, machine().describe_context().c_str());
	if (offset == B_External)
	{
		return read_external_signals();
	}

	return m_bregs[offset];
}

void maverick_device::bregs_w(offs_t offset, u32 data)
{
	if (offset != B_InterruptClear)
	{
		LOGMASKED(LOG_REGISTERS, "bregs_w: %08x @ %x (crypt %08x) %s\n", decrypt(data), offset, data, machine().describe_context().c_str());
	}

	switch (offset)
	{
	case B_InterruptClear: // ack VBL
		lower_slot_irq();
		break;

	case B_ModeRegB:
		if (BIT(decrypt(data), B_ModeB_VideoEnable))
		{
			compute_video_mode();
		}
		break;
	}

	// External is NOT encrypted
	if (offset != B_External)
	{
		m_bregs[offset] = decrypt(data);
	}
	else
	{
		m_bregs[offset] = data;
	}
}

// This is a HACK to compute the video mode from the register values until
// the CRTC is understood.
void maverick_device::compute_video_mode()
{
	m_rowbytes = 512;   // most modes are this, so assume it until proven otherwise

	// start from the pixel clock
	switch (m_bregs[B_External])
	{
		case 0:     // 30.24 MHz (Apple 640x480)
			m_pclock = 30'240'000;
			m_hres = 640;
			m_vres = 480;
			m_htotal = 864;
			m_vtotal = 525;
			m_rowbytes = 1024;
			break;

		case 1:     // 15.66 MHz (SE/30 internal monitor, Apple 12" 512x384)
			m_pclock = 15'667'200;
			if (m_aregs[3] == 0xbc) // check HLine value
			{
				// 12" monitor
				m_hres = 512;
				m_vres = 384;
				m_htotal = 640;
				m_vtotal = 407;
			}
			else if (m_aregs[3] == 0x9d)    // classic Mac video mode for the SE/30 internal CRT
			{
				m_hres = 512;
				m_vres = 342;
				m_htotal = 704;
				m_vtotal = 370;
			}
			m_rowbytes = 1024;
			break;

		case 4:     // 57.28 MHz Apple portrait monitor and SVGA 800x600 @ 72 Hz and Apple 832x624 16"
			m_pclock = 57'283'200;
			switch (m_aregs[3])
			{
				case 0x6d:  // Apple portrait
					m_hres = 640;
					m_vres = 870;
					m_htotal = 832;
					m_vtotal = 918;
					break;

				case 0x80: // Apple 16" 832x624
					m_hres = 832;
					m_vres = 624;
					m_htotal = 1152;
					m_vtotal = 667;
					break;

				case 0x5c: // SVGA 800x600 @ 72 Hz
					m_hres = 800;
					m_vres = 600;
					m_htotal = 1040;
					m_vtotal = 666;
					break;
			}
			break;

		case 9:     // 25.175 MHz VGA 640x480
			m_pclock = 25'175'000;
			m_hres = 640;
			m_vres = 480;
			m_htotal = 800;
			m_vtotal = 524;
			m_rowbytes = 1024;
			break;

		case 0xd:   // 36? MHz SVGA 800x600 @ 56 Hz
			m_pclock = 38'100'000;
			m_hres = 800;
			m_vres = 600;
			m_htotal = 1088;
			m_vtotal = 619;
			break;

		case 0xf:   // 40 MHz SVGA 800x600 @ 60 Hz
			m_pclock = 40'000'000;
			m_hres = 800;
			m_vres = 600;
			m_htotal = 1056;
			m_vtotal = 628;
			break;

		case 0x15:  // 65 MHz SVGA 1024x768
			m_pclock = 65'000'000;
			m_hres = 1024;
			m_vres = 768;
			m_htotal = 1344;
			m_vtotal = 806;
			break;
	}

	LOGMASKED(LOG_CRTC, "New video mode: %d x %d at %d pixel clock, rowbytes %d\n", m_hres, m_vres, m_pclock, m_rowbytes);

	rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
	m_maverick_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pclock).as_attoseconds());
}

u16 maverick_device::read_external_signals()
{
	return 0;
}

void maverick_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u32 maverick_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset];
}

u32 maverick_device::encrypt(u32 value)
{
	u32 result = ((value & 0xff) ^ 0xff) << 24;
	result |= muddle_table[(value >> 8) & 0x7] << 20;
	return result;
}

u32 maverick_device::decrypt(u32 value)
{
	u16 result = (value >> 24) ^ 0xff;
	result |= demuddle_table[(value >> 20) & 0xf] << 8;
	return result;
}

// **** Color 30HR section
class nubus_xceed30hr_device : public maverick_device
{
public:
	nubus_xceed30hr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void card_map(address_map &map) ATTR_COLD;

	virtual u16 read_external_signals() override;

	required_device<screen_device> m_screen;
	required_device<bt47x_device_base> m_ramdac;
};

void nubus_xceed30hr_device::card_map(address_map &map)
{
	maverick_device::base_map(map);

	map(0x00'0000, 0x0f'ffff).rw(FUNC(nubus_xceed30hr_device::vram_r), FUNC(nubus_xceed30hr_device::vram_w));
	map(0xc0'0000, 0xc0'001f).rw(m_ramdac, FUNC(bt47x_device_base::read), FUNC(bt47x_device_base::write)).umask32(0x000000ff);
}

// **** Color 30HR using the SE/30 internal display
void nubus_xceed30hr_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_xceed30hr_device::screen_update));
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);

	BT478(config, m_ramdac, 0);
}

nubus_xceed30hr_device::nubus_xceed30hr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	maverick_device(mconfig, PDS030_XCEED30HR, tag, owner, clock),
	m_screen(*this, "screen"),
	m_ramdac(*this, "bt478")
{
}

void nubus_xceed30hr_device::device_start()
{
	m_maverick_screen = m_screen;
	maverick_device::device_start_common(1 * 1024 * 1024);
	nubus().install_map(*this, &nubus_xceed30hr_device::card_map);

	m_maverick_ramdac = m_ramdac;
}

u16 nubus_xceed30hr_device::read_external_signals()
{
	u32 result = 0;

	if (m_maverick_screen->vblank() || m_maverick_screen->hblank())
	{
		result |= (1 << B_External_CBlank);
	}
	return result;
}
class nubus_xceed30hr_internal_device : public maverick_device
{
public:
	nubus_xceed30hr_internal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static auto parent_rom_device_type() { return &PDS030_XCEED30HR; }

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void card_map(address_map &map) ATTR_COLD;

	virtual u16 read_external_signals() override;

	required_device<bt47x_device_base> m_ramdac;
};

void nubus_xceed30hr_internal_device::card_map(address_map &map)
{
	maverick_device::base_map(map);

	map(0x00'0000, 0x0f'ffff).rw(FUNC(nubus_xceed30hr_internal_device::vram_r), FUNC(nubus_xceed30hr_internal_device::vram_w));
	map(0xc0'0000, 0xc0'001f).rw(m_ramdac, FUNC(bt47x_device_base::read), FUNC(bt47x_device_base::write)).umask32(0x000000ff);
}

void nubus_xceed30hr_internal_device::device_add_mconfig(machine_config &config)
{
	BT478(config, m_ramdac, 0);
}

ROM_START(xceed30hr)
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD("369c.rom", 0x000000, 0x008000, CRC(b22f0a89) SHA1(be34c8604b8a1ae9c9f3b0b90faba9a1a64a5855))
ROM_END

const tiny_rom_entry *nubus_xceed30hr_device::device_rom_region() const
{
	return ROM_NAME(xceed30hr);
}

nubus_xceed30hr_internal_device::nubus_xceed30hr_internal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	maverick_device(mconfig, PDS030_XCEED30HRINT, tag, owner, clock),
	m_ramdac(*this, "bt478")
{
}

void nubus_xceed30hr_internal_device::device_start()
{
	se30_pds_bus_device &pds = downcast<se30_pds_bus_device &>(nubus());
	m_maverick_screen = pds.m_internal_screen;
	maverick_device::device_start_common(1 * 1024 * 1024);
	pds.install_map(*this, &nubus_xceed30hr_internal_device::card_map);

	m_maverick_ramdac = m_ramdac;

	m_maverick_screen->set_screen_update(*this, FUNC(nubus_xceed30hr_internal_device::screen_update));
}

u16 nubus_xceed30hr_internal_device::read_external_signals()
{
	u32 result = (1 << B_External_NoCable);

	if (m_maverick_screen->vblank() || m_maverick_screen->hblank())
	{
		result |= (1 << B_External_CBlank);
	}
	return result;
}

const tiny_rom_entry *nubus_xceed30hr_internal_device::device_rom_region() const
{
	return ROM_NAME(xceed30hr);
}

// **** MacroColor 30 section
class nubus_xceedmc30_device : public maverick_device
{
public:
	nubus_xceedmc30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void card_map(address_map &map) ATTR_COLD;

	virtual u16 read_external_signals() override;
	virtual u8 translate_mode() override;

	required_device<screen_device> m_screen;
	required_device<bt47x_device_base> m_ramdac;
};

ROM_START(xceedmc30)
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD("0390.bin", 0x000000, 0x008000, CRC(adea7a18) SHA1(9141eb1a0e5061e0409d65a89b4eaeb119ee4ffb))
ROM_END

const tiny_rom_entry *nubus_xceedmc30_device::device_rom_region() const
{
	return ROM_NAME(xceedmc30);
}

void nubus_xceedmc30_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_xceedmc30_device::screen_update));
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);

	BT473(config, m_ramdac, 0);
}

void nubus_xceedmc30_device::card_map(address_map &map)
{
	maverick_device::base_map(map);

	map(0x00'0000, 0x1f'ffff).rw(FUNC(nubus_xceedmc30_device::vram_r), FUNC(nubus_xceedmc30_device::vram_w));
	map(0xc0'0000, 0xc0'001f).rw(m_ramdac, FUNC(bt47x_device_base::read), FUNC(bt47x_device_base::write)).umask32(0xff000000);
}

nubus_xceedmc30_device::nubus_xceedmc30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	maverick_device(mconfig, PDS030_XCEEDMC30, tag, owner, clock),
	m_screen(*this, "screen"),
	m_ramdac(*this, "bt478")
{
}

void nubus_xceedmc30_device::device_start()
{
	m_maverick_screen = m_screen;
	maverick_device::device_start_common(2 * 1024 * 1024);  // more VRAM to handle 24-bit mode
	nubus().install_map(*this, &nubus_xceedmc30_device::card_map);

	m_skipbytes = 4096;
	m_maverick_ramdac = m_ramdac;
}

u16 nubus_xceedmc30_device::read_external_signals()
{
	u32 result = 0;

	if (m_screen->vblank() || m_screen->hblank())
	{
		result |= (1 << 2);
	}
	return result;
}

u8 nubus_xceedmc30_device::translate_mode()
{
	switch (m_bregs[B_ExternalModeBits])
	{
		case 7: return 0; // 8bpp
		case 6: return 1; // 4bpp
		case 5: return 2; // 2bpp
		case 4: return 3; // 1bpp
		case 0: return 4; // 24bpp
	}

	return 3;
}

// **** MacroColor 30 using the SE/30 internal display
class nubus_xceedmc30_internal_device : public maverick_device
{
public:
	nubus_xceedmc30_internal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static auto parent_rom_device_type() { return &PDS030_XCEEDMC30; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void card_map(address_map &map) ATTR_COLD;

	virtual u16 read_external_signals() override;
	virtual u8 translate_mode() override;

	required_device<bt47x_device_base> m_ramdac;
};

const tiny_rom_entry *nubus_xceedmc30_internal_device::device_rom_region() const
{
	return ROM_NAME(xceedmc30);
}

void nubus_xceedmc30_internal_device::device_add_mconfig(machine_config &config)
{
	BT473(config, m_ramdac, 0);
}

void nubus_xceedmc30_internal_device::card_map(address_map &map)
{
	maverick_device::base_map(map);

	map(0x00'0000, 0x1f'ffff).rw(FUNC(nubus_xceedmc30_internal_device::vram_r), FUNC(nubus_xceedmc30_internal_device::vram_w));
	map(0xc0'0000, 0xc0'001f).rw(m_ramdac, FUNC(bt47x_device_base::read), FUNC(bt47x_device_base::write)).umask32(0xff000000);
}

nubus_xceedmc30_internal_device::nubus_xceedmc30_internal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	maverick_device(mconfig, PDS030_XCEEDMC30INT, tag, owner, clock),
	m_ramdac(*this, "bt478")
{
}

void nubus_xceedmc30_internal_device::device_start()
{
	se30_pds_bus_device &pds = downcast<se30_pds_bus_device &>(nubus());
	m_maverick_screen = pds.m_internal_screen;
	maverick_device::device_start_common(2 * 1024 * 1024);  // more VRAM to handle 24-bit mode
	pds.install_map(*this, &nubus_xceedmc30_internal_device::card_map);

	m_skipbytes = 4096;
	m_maverick_screen->set_screen_update(*this, FUNC(nubus_xceedmc30_internal_device::screen_update));
	m_maverick_ramdac = m_ramdac;
}

u16 nubus_xceedmc30_internal_device::read_external_signals()
{
	u32 result = (1 << 1); // internal SE/30 CRT cable detect

	if (m_maverick_screen->vblank() || m_maverick_screen->hblank())
	{
		result |= (1 << 2); // combined blanking signal
	}
	return result;
}

u8 nubus_xceedmc30_internal_device::translate_mode()
{
	switch (m_bregs[B_ExternalModeBits] & 7)
	{
		case 7: return 0; // 8bpp
		case 6: return 1; // 4bpp
		case 5: return 2; // 2bpp
		case 4: return 3; // 1bpp
		case 0: return 4; // 24bpp
	}

	return 3;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(PDS030_XCEED30HR, device_nubus_card_interface, nubus_xceed30hr_device, "pd3_30hr", "Micron/XCEED Technology Color 30HR")
DEFINE_DEVICE_TYPE_PRIVATE(PDS030_XCEED30HRINT, device_nubus_card_interface, nubus_xceed30hr_internal_device, "pd3_30hrint", "Micron/XCEED Technology Color 30HR (internal grayscale)")
DEFINE_DEVICE_TYPE_PRIVATE(PDS030_XCEEDMC30, device_nubus_card_interface, nubus_xceedmc30_device, "pd3_mclr", "Micron/XCEED Technology MacroColor 30")
DEFINE_DEVICE_TYPE_PRIVATE(PDS030_XCEEDMC30INT, device_nubus_card_interface, nubus_xceedmc30_internal_device, "pd3_mclrint", "Micron/XCEED Technology MacroColor 30 (internal grayscale)")
