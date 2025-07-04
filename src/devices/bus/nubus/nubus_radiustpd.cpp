// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Radius Two Page Display (1280x960?)

  Fsx8000a = DAC data
  Fsx8000e = DAC control
  Fsx00000 = VRAM (offset 0x200, stride 0x1b0)

***************************************************************************/

#include "emu.h"
#include "nubus_radiustpd.h"
#include "screen.h"

#include <algorithm>

namespace {

static constexpr u32 VRAM_SIZE = 0x40000;   // 256k.  1152x880 1 bit per pixel fits nicely.

class nubus_radiustpd_device : public device_t,
							   public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_radiustpd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_radiustpd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vbl_tick);

	void card_map(address_map &map);

private:
	u32 radiustpd_r(offs_t offset, u32 mem_mask = ~0);
	void radiustpd_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	std::unique_ptr<u32[]> m_vram;
	u32 m_vbl_disable;
	u32 m_palette[2];
	emu_timer *m_timer;
};

ROM_START( radiustpd )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "tpd_v22.bin",  0x0000, 0x8000, CRC(7dc5ed05) SHA1(4abb64e49201e966c17a255a94b670564b229934) )
ROM_END

void nubus_radiustpd_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_radiustpd_device::screen_update));
	m_screen->set_raw(99.522_MHz_XTAL, 1536, 0, 1152, 900, 0, 880); // 64.79 kHz horizontal, 72 Hz refresh
}

const tiny_rom_entry *nubus_radiustpd_device::device_rom_region() const
{
	return ROM_NAME( radiustpd );
}

nubus_radiustpd_device::nubus_radiustpd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_radiustpd_device(mconfig, NUBUS_RADIUSTPD, tag, owner, clock)
{
}

nubus_radiustpd_device::nubus_radiustpd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_vbl_disable(0), m_timer(nullptr)
{
	m_palette[0] = rgb_t(0, 0, 0);
	m_palette[1] = rgb_t(255, 255, 255);
}

void nubus_radiustpd_device::card_map(address_map &map)
{
	map(0x00'0000, 0x03'ffff).rw(FUNC(nubus_radiustpd_device::vram_r), FUNC(nubus_radiustpd_device::vram_w)).mirror(0xf00000);
	map(0x08'0000, 0x0e'ffff).rw(FUNC(nubus_radiustpd_device::radiustpd_r), FUNC(nubus_radiustpd_device::radiustpd_w)).mirror(0xf00000);
}

void nubus_radiustpd_device::device_start()
{
	install_declaration_rom("declrom", true, true);

	m_vram = std::make_unique<u32[]>(VRAM_SIZE / sizeof(u32));

	nubus().install_map(*this, &nubus_radiustpd_device::card_map);

	m_timer = timer_alloc(FUNC(nubus_radiustpd_device::vbl_tick), this);
	m_timer->adjust(m_screen->time_until_pos(879, 0), 0);

	save_pointer(NAME(m_vram), VRAM_SIZE / sizeof(u32));
	save_item(NAME(m_vbl_disable));
}

void nubus_radiustpd_device::device_reset()
{
	m_vbl_disable = 1;
}

TIMER_CALLBACK_MEMBER(nubus_radiustpd_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(879, 0), 0);
}

u32 nubus_radiustpd_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + 0x200;

	for (int y = 0; y < 880; y++)
	{
		u32 *scanline = &bitmap.pix(y);
		for (int x = 0; x < 1152/8; x++)
		{
			u8 const pixels = vram8[(y * (1152/8)) + x];

			*scanline++ = m_palette[BIT(pixels, 7)];
			*scanline++ = m_palette[BIT(pixels, 6)];
			*scanline++ = m_palette[BIT(pixels, 5)];
			*scanline++ = m_palette[BIT(pixels, 4)];
			*scanline++ = m_palette[BIT(pixels, 3)];
			*scanline++ = m_palette[BIT(pixels, 2)];
			*scanline++ = m_palette[BIT(pixels, 1)];
			*scanline++ = m_palette[BIT(pixels, 0)];
		}
	}

	return 0;
}

void nubus_radiustpd_device::radiustpd_w(offs_t offset, u32 data, u32 mem_mask)
{
//  printf("TPD: write %08x to %x, mask %08x\n", data, offset, mem_mask);
}

u32 nubus_radiustpd_device::radiustpd_r(offs_t offset, u32 mem_mask)
{
//  printf("TPD: read @ %x, mask %08x\n", offset, mem_mask);

	if (offset == 0)
	{
		lower_slot_irq();
		m_vbl_disable = true;
	}

	if (offset == 0x8000)
	{
		m_vbl_disable = false;
	}

	if (offset == 0x18000)
	{
		return 0xffffffff;
	}

	return 0;
}

void nubus_radiustpd_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

u32 nubus_radiustpd_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_RADIUSTPD, device_nubus_card_interface, nubus_radiustpd_device, "nb_rtpd", "Radius Two Page Display video card")
