// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple Macintosh II High Resolution Video Card
  Apple Portrait Card
  Apple Workstation Card

  Same hardware, different ROMs, crystals, and speed grades of the Bt453/454.
  The High Resolution card is intended to drive the color 640x480 13" standard
  AppleColor High Resolution RGB monitor.
  The Workstation/Portrait card was sold in two configurations: as the Portrait
  card to drive the 640x870 15" Portrait monochrome monitor, and as the
  Workstation card to drive the 1152x870 21" monochrome monitor.

  Video ASIC for High Resolution and Portrait is TFB 2.2, Apple part number 344S0077
  Video ASIC for Workstation is TFB 2.0, Apple part number 344S0013
  RAMDAC is Bt453 for Mac II Hi-Res, Bt454 for Workstation and Portrait

***************************************************************************/

#include "emu.h"

#include "nubus_m2hires.h"
#include "video/bt45x.h"

#include "screen.h"

#include <algorithm>

#define LOG_REGISTERS (1U << 1)
#define LOG_CRTC (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

namespace {

enum
{
	BASE = 0,           // Offset to start drawing from the base of VRAM, in 32-bit words
	LENGTH,             // Stride of each scanline, in 32-bit words
	MISC,               // A whole bunch of random small parameters, including the pixel depth
	SYNCINTERVAL,       // Time from the middle of the scanline to the rising edge of the RS170 composite sync vertical serration
	VFRONTPORCH,        // V front porch length, minus 1
	VBACKPORCH,         // V back porch length, minus 8
	VLINES,             // Total visible half-lines, minus 1
	HFRONTPORCH,        // Front porch in pixel clocks, minus 2
	HSYNCPULSE,         // H sync pulse width in pixel clocks, minus 4
	HBACKPORCH,         // Back porch in pixel clocks, minus 4
	HFIRST,             // First section of visible area, minus 2
	HLAST,              // Second section of visible area, minus 2
	SOFTRESET,          // Bit 0 = 1 to enable video generation
};

static constexpr u32 VRAM_SIZE = 0x80000;   // 512k max

class nubus_m2hires_device : public device_t,
							 public device_video_interface,
							 public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_m2hires_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_m2hires_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vbl_tick);

	void card_map(address_map &map);

	virtual u32 vblank_r(offs_t offset, u32 mem_mask = ~0);
	virtual void ramdac_w(offs_t offset, u32 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	required_device<bt45x_rgb_device_base> m_ramdac;

	std::vector<u32> m_vram;
	u32 m_htotal, m_hres, m_vtotal, m_vres, m_mode;
	int m_bt_type;
	XTAL m_pclock;

private:
	void registers_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void vbl_w(offs_t offset, u8 data);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 m_vbl_disable;
	u32 m_registers[0x10];
	emu_timer *m_timer;
};

void nubus_m2hires_device::card_map(address_map &map)
{
	map(0x00'0000, 0x07'ffff).rw(FUNC(nubus_m2hires_device::vram_r), FUNC(nubus_m2hires_device::vram_w)).mirror(0xf00000);
	map(0x08'0000, 0x08'ffff).w(FUNC(nubus_m2hires_device::registers_w)).mirror(0xf00000);
	map(0x09'0000, 0x09'ffff).rw(FUNC(nubus_m2hires_device::vblank_r), FUNC(nubus_m2hires_device::ramdac_w)).mirror(0xf00000);
	map(0x0a'0000, 0x0a'ffff).w(FUNC(nubus_m2hires_device::vbl_w)).umask32(0xffffffff).mirror(0xf00000);
}

ROM_START( m2hires )
	ROM_REGION(0x2000, "declrom", 0)
	ROM_LOAD( "341-0660.bin", 0x0000, 0x2000, CRC(ea6f7913) SHA1(37c59f38ae34021d0cb86c2e76a598b7e6077c0d) )
ROM_END

void nubus_m2hires_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_m2hires_device::screen_update));
	m_screen->set_raw(30.24_MHz_XTAL, 896, 0, 640, 525, 0, 480);

	BT453(config, m_ramdac, 0);
}

const tiny_rom_entry *nubus_m2hires_device::device_rom_region() const
{
	return ROM_NAME( m2hires );
}

nubus_m2hires_device::nubus_m2hires_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_m2hires_device(mconfig, NUBUS_M2HIRES, tag, owner, clock)
{
}

nubus_m2hires_device::nubus_m2hires_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_ramdac(*this, "bt453"),
	m_htotal(896),
	m_hres(640),
	m_vtotal(525),
	m_vres(480),
	m_mode(0),
	m_bt_type(453),
	m_pclock(30.24_MHz_XTAL),
	m_vbl_disable(0), m_timer(nullptr)
{
	set_screen(*this, "screen");
}

void nubus_m2hires_device::device_start()
{
	install_declaration_rom("declrom", true);

	m_vram.resize(VRAM_SIZE / sizeof(u32));

	nubus().install_map(*this, &nubus_m2hires_device::card_map);

	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_registers));
	save_item(NAME(m_htotal));
	save_item(NAME(m_hres));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_vres));
	save_item(NAME(m_mode));

	m_timer = timer_alloc(FUNC(nubus_m2hires_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

void nubus_m2hires_device::device_reset()
{
	m_vbl_disable = 1;
	std::fill(m_vram.begin(), m_vram.end(), 0);
}

TIMER_CALLBACK_MEMBER(nubus_m2hires_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(m_vres - 1, 0), 0);
}

u32 nubus_m2hires_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_registers[SOFTRESET], 0))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + ((m_registers[BASE] & 0x1ffff) << 2);
	pen_t const *const pens = m_ramdac->pens();
	const u32 stride = (m_registers[LENGTH] & 0x3ff) << 2;

	switch (m_mode)
	{
		case 0: // 1 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/8; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

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

		case 1: // 2 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/4; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[((pixels>>6)&3)];
					*scanline++ = pens[((pixels>>4)&3)];
					*scanline++ = pens[((pixels>>2)&3)];
					*scanline++ = pens[(pixels&3)];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);

				for (int x = 0; x < m_hres/2; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[((pixels&0xf0)>>4)];
					*scanline++ = pens[(pixels&0xf)];
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);

				for (int x = 0; x < m_hres; x++)
				{
					*scanline++ = pens[vram8[(y * stride) + x]];
				}
			}
			break;
	}
	return 0;
}

void nubus_m2hires_device::registers_w(offs_t offset, u32 data, u32 mem_mask)
{
	data ^= 0xffffffff;
	data = swapendian_int32(data);

	LOGMASKED(LOG_REGISTERS, "%s: %08x @ %x, mask %08x %s\n", tag(), data, offset, mem_mask, machine().describe_context());
	m_registers[offset] = data;

	if (offset == BASE && BIT(m_registers[SOFTRESET], 0))
	{
		const u32 mode = (m_registers[MISC] >> 8) & 7;
		if (m_bt_type == 453)
		{
			m_mode = mode - 4;
		}
		else if (m_bt_type == 454)
		{
			switch (mode)
			{
			case 0: // 4 bpp
				m_mode = 2;
				break;

			case 2: // 1 bpp
				m_mode = 0;
				break;

			case 3: // 2 bpp
				m_mode = 1;
				break;
			}
		}

		const int hmult = (16 >> m_mode);

		m_htotal = ((m_registers[HFRONTPORCH] + 2) +
				   (m_registers[HSYNCPULSE] + 2) +
				   (m_registers[HBACKPORCH] + 4) +
				   (m_registers[HFIRST] + 2) +
				   (m_registers[HLAST] + 2)) * hmult;

		m_vtotal = ((m_registers[VFRONTPORCH] + 1) +
				   (m_registers[VBACKPORCH] + 8) +
				   (m_registers[VLINES] + 1)) / 2;
		m_vtotal += 3;

		m_vres = (m_registers[VLINES] / 2) + 1;
		m_hres = (m_registers[HFIRST] + 2 + m_registers[HLAST] + 2) * hmult;

		LOGMASKED(LOG_CRTC, "htotal %d vtotal %d hres %d vres %d\n", m_htotal, m_vtotal, m_hres, m_vres);

		rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
		m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pclock).as_attoseconds());
	}
}

void nubus_m2hires_device::ramdac_w(offs_t offset, u32 data)
{
	data ^= 0xffffffff;
	switch (offset & 1)
	{
	case 0:
		m_ramdac->address_w(data);
		break;

	case 1:
		m_ramdac->palette_w(data);
		break;
	}
}

u32 nubus_m2hires_device::vblank_r(offs_t offset, u32 mem_mask)
{
	if (offset == 0x10/4)
	{
		return (m_screen->vblank() << 16) | (1<<17);  // monitor ID bits in 17-20, but inverted
	}
	return 0;
}

void nubus_m2hires_device::vbl_w(offs_t offset, u8 data)
{
	if (offset & 4)
	{
		m_vbl_disable = 1;
	}
	else
	{
		m_vbl_disable = 0;
		lower_slot_irq();
	}
}

void nubus_m2hires_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

u32 nubus_m2hires_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}

// ***************** Portrait Card support *****************

class nubus_portrait_device : public nubus_m2hires_device
{
public:
	nubus_portrait_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: nubus_m2hires_device(mconfig, NUBUS_WSPORTRAIT, tag, owner, clock),
		m_color_index(0)
	{
	}

protected:
	nubus_portrait_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual u32 vblank_r(offs_t offset, u32 mem_mask = ~0) override;
	virtual void ramdac_w(offs_t offset, u32 data) override;

private:
	u8 m_color_index;
};

void nubus_portrait_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_portrait_device::screen_update));
	m_screen->set_raw(57.2832_MHz_XTAL, 832, 0, 640, 918, 0, 832);
	m_screen->set_physical_aspect(3, 4);

	BT454(config, m_ramdac, 0);
}

ROM_START(wsportrait)
	ROM_REGION(0x1000, "declrom", 0)
	ROM_LOAD("341-0732.bin", 0x000000, 0x001000, CRC(ddc35b78) SHA1(ce2bf2374bb994c17962dba8f3d11bc1260e2644))
ROM_END

const tiny_rom_entry *nubus_portrait_device::device_rom_region() const
{
	return ROM_NAME(wsportrait);
}

nubus_portrait_device::nubus_portrait_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	nubus_m2hires_device(mconfig, type, tag, owner, clock)
{
}

void nubus_portrait_device::device_start()
{
	nubus_m2hires_device::device_start();
	save_item(NAME(m_color_index));

	m_pclock = 57.2832_MHz_XTAL;
	m_bt_type = 454;
}

u32 nubus_portrait_device::vblank_r(offs_t offset, u32 mem_mask)
{
	if (offset == 0x10 / 4)
	{
		return (m_screen->vblank() << 16) | (6 << 17);
	}

	return 0;
}

void nubus_portrait_device::ramdac_w(offs_t offset, u32 data)
{
	data ^= 0xffffffff;
	data >>= 24;
	switch (offset & 1)
	{
	case 0:
		m_ramdac->address_w(data);
		m_color_index = 0;
		break;

	case 1:
		if (m_color_index == 2)
		{
			m_ramdac->palette_w(data);
			m_ramdac->palette_w(data);
			m_ramdac->palette_w(data);
			m_color_index = 0;
		}
		else
		{
			m_color_index++;
		}
		break;
	}
}

// ***************** Workstation Card support *****************

class nubus_workstation_device : public nubus_portrait_device
{
public:
	nubus_workstation_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		nubus_portrait_device(mconfig, NUBUS_WORKSTATION, tag, owner, clock)
	{
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual u32 vblank_r(offs_t offset, u32 mem_mask = ~0) override;
};

ROM_START(workstation)
	ROM_REGION(0x1000, "declrom", 0)
	ROM_LOAD("341-0245a.bin", 0x000000, 0x001000, CRC(aff72693) SHA1(697412843cfd56be2b382f8952ef3f5a8323066a))
ROM_END

	const tiny_rom_entry *nubus_workstation_device::device_rom_region() const
	{
		return ROM_NAME(workstation);
	}

void nubus_workstation_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_workstation_device::screen_update));
	m_screen->set_raw(100_MHz_XTAL, 1448, 0, 1152, 913, 0, 832);

	BT454(config, m_ramdac, 0);
}

void nubus_workstation_device::device_start()
{
	nubus_portrait_device::device_start();
	m_pclock = 100_MHz_XTAL;
}

u32 nubus_workstation_device::vblank_r(offs_t offset, u32 mem_mask)
{
	if (offset == 0x10 / 4)
	{
		return (m_screen->vblank() << 16) | (7 << 17);
	}

	return 0;
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_M2HIRES, device_nubus_card_interface, nubus_m2hires_device, "nb_m2hr", "Apple Macintosh II High Resolution Video Card")
DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_WSPORTRAIT, device_nubus_card_interface, nubus_portrait_device, "nb_wspt", "Macintosh II Portrait Video Card")
DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_WORKSTATION, device_nubus_card_interface, nubus_workstation_device, "nb_wkstn", "Macintosh II Workstation Video Card")
