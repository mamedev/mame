// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple Macintosh II Video Card (630-0153) emulation

  Video ASIC is TFB 1.x, 344-0001
  RAMDAC is Bt453

  ***************************************************************************/

#include "emu.h"
#include "nubus_m2video.h"

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
	LENGTH,
	MISC,
	BASEHI,
	BASELO,
	SYNCINTERVAL,   // 10
	VLINES_VPP,     // 14
	VLINES,         // 18
	VBACKPORCH,     // 1c
	SYNCINTERVAL8,
	HSYNCSTART,
	HSYNCFINISH,
	HALFLINE_EARLY,
	HALFLINE,
	HPIXELS_HLATE,
	HPIXELS,
	MISC2
};

static constexpr u32 VRAM_SIZE = 0x80000;   // 512k max

class nubus_m2video_device : public device_t,
							 public device_video_interface,
							 public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_m2video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_m2video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vbl_tick);

private:
	u8 vbl_r(offs_t offset);
	void vbl_w(offs_t offset, u8 data);
	u8 ramdac_r(offs_t offset);
	void ramdac_w(offs_t offset, u8 data);
	void tfb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void calc_screen_params();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void card_map(address_map &map);

	required_device<screen_device> m_screen;
	required_device<bt453_device> m_ramdac;

	std::unique_ptr<u32[]> m_vram;
	u32 m_mode, m_vbl_disable;
	u32 m_htotal, m_hres, m_vtotal, m_vres;
	u8 m_regs[0x10];
	emu_timer *m_timer;
};

ROM_START( m2video )
	ROM_REGION(0x1000, "declrom", 0)
	ROM_LOAD( "342-0008-a.bin", 0x000000, 0x001000, CRC(bf50850d) SHA1(abe85d8a882bb2b8187a28bd6707fc2f5d77eedd) )
ROM_END

void nubus_m2video_device::card_map(address_map &map)
{
	map(0x00'0000, 0x07'ffff).rw(FUNC(nubus_m2video_device::vram_r), FUNC(nubus_m2video_device::vram_w)).mirror(0xf00000);
	map(0x08'0000, 0x08'ffff).w(FUNC(nubus_m2video_device::tfb_w)).mirror(0xf00000);
	map(0x09'0000, 0x09'001f).rw(FUNC(nubus_m2video_device::ramdac_r), FUNC(nubus_m2video_device::ramdac_w)).umask32(0xff000000).mirror(0xf00000);
	map(0x0a'0000, 0x0a'ffff).w(FUNC(nubus_m2video_device::vbl_w)).umask32(0xffffffff).mirror(0xf00000);
	map(0x0d'0000, 0x0d'ffff).r(FUNC(nubus_m2video_device::vbl_r)).umask32(0x000000ff).mirror(0xf00000);
}

void nubus_m2video_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_m2video_device::screen_update));
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);

	BT453(config, m_ramdac, 0);
}

const tiny_rom_entry *nubus_m2video_device::device_rom_region() const
{
	return ROM_NAME( m2video );
}

nubus_m2video_device::nubus_m2video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_m2video_device(mconfig, NUBUS_M2VIDEO, tag, owner, clock)
{
}

nubus_m2video_device::nubus_m2video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_ramdac(*this, "bt453"),
	m_mode(0),
	m_vbl_disable(0),
	m_htotal(896),
	m_hres(640),
	m_vtotal(525),
	m_vres(480),
	m_timer(nullptr)
{
	set_screen(*this, "screen");
}

void nubus_m2video_device::device_start()
{
	install_declaration_rom("declrom", true, true);

	m_vram = std::make_unique<u32[]>(VRAM_SIZE / sizeof(u32));
	nubus().install_map(*this, &nubus_m2video_device::card_map);

	std::fill_n(&m_regs[0], 16, 0);

	m_timer = timer_alloc(FUNC(nubus_m2video_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

void nubus_m2video_device::device_reset()
{
	m_vbl_disable = 1;
	m_mode = 0;
}

TIMER_CALLBACK_MEMBER(nubus_m2video_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

u32 nubus_m2video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + 0x20;
	pen_t const *const pens = m_ramdac->pens();

	switch (m_mode)
	{
		case 0: // 1 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/8; x++)
				{
					u8 const pixels = vram8[(y * 128) + x];

					*scanline++ = pens[(pixels&0x80)];
					*scanline++ = pens[((pixels<<1)&0x80)];
					*scanline++ = pens[((pixels<<2)&0x80)];
					*scanline++ = pens[((pixels<<3)&0x80)];
					*scanline++ = pens[((pixels<<4)&0x80)];
					*scanline++ = pens[((pixels<<5)&0x80)];
					*scanline++ = pens[((pixels<<6)&0x80)];
					*scanline++ = pens[((pixels<<7)&0x80)];
				}
			}
			break;

		case 1: // 2 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/4; x++)
				{
					u8 const pixels = vram8[(y * 256) + x];

					*scanline++ = pens[(pixels&0xc0)];
					*scanline++ = pens[((pixels<<2)&0xc0)];
					*scanline++ = pens[((pixels<<4)&0xc0)];
					*scanline++ = pens[((pixels<<6)&0xc0)];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/2; x++)
				{
					u8 const pixels = vram8[(y * 512) + x];

					*scanline++ = pens[(pixels&0xf0)];
					*scanline++ = pens[((pixels&0x0f)<<4)];
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u8 const pixels = vram8[(y * 1024) + x];
					*scanline++ = pens[pixels];
				}
			}
			break;

		default:
			fatalerror("m2video: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_m2video_device::tfb_w(offs_t offset, u32 data, u32 mem_mask)
{
	data ^= 0xffffffff;
	offset &= 0xf;
	LOGMASKED(LOG_REGISTERS, "%s m2video_w: %08x @ %x (mask %08x)\n", machine().describe_context(), data, offset, mem_mask);
	if (mem_mask == 0xff00'0000)
	{
		data >>= 24;
	}
	m_regs[offset & 0xf] = data & 0xff;

	if ((offset & 0xf) == 0xf)
	{
		calc_screen_params();
	}
}

void nubus_m2video_device::vbl_w(offs_t offset, u8 data)
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


u8 nubus_m2video_device::vbl_r(offs_t offset)
{
	u8 result = m_screen->vblank() ? 0 : 0xff;
	return result;
}

u8 nubus_m2video_device::ramdac_r(offs_t offset)
{
	switch (offset & 3)
	{
		case 2:
			return m_ramdac->palette_r(nubus().space()) ^ 0xff;

		case 1:
		case 3:
			return m_ramdac->address_r() ^ 0xff;
	}

	return 0;
}

void nubus_m2video_device::ramdac_w(offs_t offset, u8 data)
{
	data ^= 0xff;
	switch (offset & 3)
	{
		case 1:
		case 3:
			m_ramdac->address_w(data);
			break;

		case 2:
			m_ramdac->palette_w(data);
			break;
	}
}

void nubus_m2video_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

u32 nubus_m2video_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}

// This chip packs the registers in an unpleasant way.
// TFB 2.2 does it much more pleasantly (see nubus_m2hires.cpp).
void nubus_m2video_device::calc_screen_params()
{
	m_mode = (m_regs[MISC2] >> 4) & 3;

	const u32 halfline = (m_regs[HALFLINE] | (BIT(m_regs[HALFLINE_EARLY], 7) << 8)) + 2;
	const u32 hpixels = ((m_regs[HPIXELS] << 2) | BIT(m_regs[HPIXELS_HLATE], 6, 2)) + 2;
	const u32 hsyncstart = m_regs[HSYNCSTART] + 2;
	const u32 hsyncfinish = m_regs[HSYNCFINISH] + 2;
	const u32 hearly = (m_regs[HALFLINE_EARLY] & 0x7f) + 2;
	const u32 hlate = (m_regs[HPIXELS_HLATE] & 0x3f) + 2;
	const u32 vfrontporch = (m_regs[VLINES_VPP] & 0x1f) + 1;
	const u32 vsyncfinish = (m_regs[SYNCINTERVAL8] & 0x7f) + 1;
	const u32 vbackporch = (m_regs[VBACKPORCH] & 0x3f) + 8;
	const u32 vlines = ((m_regs[VLINES] << 3) | BIT(m_regs[VLINES_VPP], 5, 3)) + 1;

	m_hres = (halfline + hpixels) * (16 >> m_mode);
	m_htotal = (halfline + hpixels + hsyncstart + hsyncfinish + hearly + hlate) * (16 >> m_mode);
	m_vres = vlines / 2;
	m_vtotal = (vfrontporch * 2) + vsyncfinish + (vbackporch * 2) + (vlines / 2);

	LOGMASKED(LOG_CRTC, "halfline %x hpixels %x depth %d\n", halfline, hpixels, m_mode);
	LOGMASKED(LOG_CRTC, "hsyncstart %x hsyncfinish %x hearly %x hlate %x\n", hsyncstart, hsyncfinish, hearly, hlate);
	LOGMASKED(LOG_CRTC, "hvis = %d, htotal = %d\n", m_hres, m_htotal);
	LOGMASKED(LOG_CRTC, "vfrontporch %x vsyncfinish %x vbackporch %x vlines %x\n", vfrontporch, vsyncfinish, vbackporch, vlines);
	LOGMASKED(LOG_CRTC, "vvis = %d, vtotal = %d\n", m_vres, m_vtotal);

	rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
	m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, 30.24_MHz_XTAL).as_attoseconds());
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_M2VIDEO, device_nubus_card_interface, nubus_m2video_device, "nb_m2vc", "Apple Macintosh II Video Card")
