// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Lapis ProColor Server 8*16 video card

  FsFF6001: DAC color # (seems to have the data bits perfectly reversed)
  FsFF6003: DAC color write (not bitswapped)
  FsFF6017: Mode (13 = 1bpp, 17 = 2bpp, 1b = 4bpp, 1e = 8bpp, 0a = 15bpp)
  FsFF7000: Bit 2 is VBL IRQ enable/ack
  FsFF7001: Bit 0 is VBL status

  All of Lapis' cards for Macs have the same architecture:
    A Xilinx FPGA that the declaration ROM reprograms for each color depth
    A TLC34075 or TLC34076 RAMDAC

***************************************************************************/

#include "emu.h"
#include "pds30_procolor816.h"

#include "video/tlc34076.h"

#include "screen.h"

#include <algorithm>

static constexpr u32 VRAM_SIZE  =   0x200000;

namespace {

ROM_START( procolor816 )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "procolor_ver60590.bin", 0x000000, 0x008000, CRC(ebef6168) SHA1(e41ecc7d12fc13bc74f9223ca02920e8a7eb072b) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

class nubus_procolor816_device : public device_t,
								 public device_video_interface,
								 public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_procolor816_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_procolor816_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void card_map(address_map &map);

	TIMER_CALLBACK_MEMBER(vbl_tick);

	bool m_has_15bpp;

private:
	u8 regs_r(offs_t offset);
	void regs_w(offs_t offset, u8 data);
	u32 vram_r(offs_t offset, u32 mem_mask = ~0);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	required_device<tlc34076_device> m_dac;

	std::unique_ptr<u32[]> m_vram;
	u8 m_video_enable, m_vbl_disable;
	emu_timer *m_timer;
};

void nubus_procolor816_device::card_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).rw(FUNC(nubus_procolor816_device::vram_r), FUNC(nubus_procolor816_device::vram_w));
	map(0xfd'6000, 0xfd'603f).rw(m_dac, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask32(0x00ff0000);
	map(0xfd'7000, 0xfd'7fff).rw(FUNC(nubus_procolor816_device::regs_r), FUNC(nubus_procolor816_device::regs_w));
	map(0xff'6000, 0xff'601f).rw(m_dac, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask32(0x00ff00ff);
	map(0xff'7000, 0xff'7fff).rw(FUNC(nubus_procolor816_device::regs_r), FUNC(nubus_procolor816_device::regs_w));
}

void nubus_procolor816_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_procolor816_device::screen_update));
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);

	TLC34076(config, m_dac, tlc34076_device::TLC34076_8_BIT);   // actually a TLC34075, but I can't find a difference
}

const tiny_rom_entry *nubus_procolor816_device::device_rom_region() const
{
	return ROM_NAME( procolor816 );
}

nubus_procolor816_device::nubus_procolor816_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_procolor816_device(mconfig, PDS030_PROCOLOR816, tag, owner, clock)
{
}

nubus_procolor816_device::nubus_procolor816_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_has_15bpp(true),
	m_screen(*this, "screen"),
	m_dac(*this, "tlc34076"),
	m_video_enable(0),
	m_vbl_disable(0),
	m_timer(nullptr)
{
	set_screen(*this, "screen");
}

void nubus_procolor816_device::device_start()
{
	m_vram = make_unique_clear<u32[]>(VRAM_SIZE / sizeof(u32));

	install_declaration_rom("declrom");
	nubus().install_map(*this, &nubus_procolor816_device::card_map);

	m_timer = timer_alloc(FUNC(nubus_procolor816_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);

	save_item(NAME(m_video_enable));
	save_item(NAME(m_vbl_disable));
	save_pointer(NAME(m_vram), VRAM_SIZE / sizeof(u32));
}

void nubus_procolor816_device::device_reset()
{
	m_vbl_disable = 1;
	m_video_enable = 0;
	std::fill_n(&m_vram[0], VRAM_SIZE / sizeof(u32), 0);
}

TIMER_CALLBACK_MEMBER(nubus_procolor816_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

u32 nubus_procolor816_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_video_enable)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + 4;
	pen_t const *const pens = m_dac->pens();
	const auto clock_select = m_dac->read(0xa); // get the TLC34076 output clock select
	const auto pixel_mask = m_dac->read(2); // get the TLC34076 pixel mask

	switch (pixel_mask)
	{
		case 0x1: // 1 bpp - SCLK = DOTCLK/32
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					u8 const pixels = vram8[(y * 640/8) + x];

					*scanline++ = pens[(pixels>>7)&1];
					*scanline++ = pens[(pixels>>6)&1];
					*scanline++ = pens[(pixels>>5)&1];
					*scanline++ = pens[(pixels>>4)&1];
					*scanline++ = pens[(pixels>>3)&1];
					*scanline++ = pens[(pixels>>2)&1];
					*scanline++ = pens[(pixels>>1)&1];
					*scanline++ = pens[pixels&1];
				}
			}
			break;

		case 0x3: // 2 bpp - SCLK = DOTCLK/16
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					u8 const pixels = vram8[(y * 640/4) + x];

					*scanline++ = pens[(pixels>>6)&3];
					*scanline++ = pens[(pixels>>4)&3];
					*scanline++ = pens[(pixels>>2)&3];
					*scanline++ = pens[pixels & 0x3];
				}
			}
			break;

		case 0xf: // 4 bpp - SCLK = DOTCLK/8
			for (int y = 0; y < 480; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/2; x++)
				{
					u8 const pixels = bitswap(vram8[(y * 640 / 2) + x], 4, 5, 6, 7, 0, 1, 2, 3);

					*scanline++ = pens[(pixels>>4)&0xf];
					*scanline++ = pens[pixels&0xf];
				}
			}
			break;

		case 0xff: // 8 bpp - SCLK = DOTCLK/4
			if ((clock_select == 0x19) && (m_has_15bpp))
			{
				auto const vram16 = util::big_endian_cast<u16 const>(&m_vram[0]);

				for (int y = 0; y < 480; y++)
				{
					u32 *scanline = &bitmap.pix(y);
					for (int x = 0; x < 640; x++)
					{
						u16 const pixels = vram16[(y * 640) + x];
						*scanline++ = rgb_t(pal5bit((pixels >> 10) & 0x1f), pal5bit((pixels >> 5) & 0x1f), pal5bit(pixels & 0x1f));
					}
				}
			}
			else
			{
				for (int y = 0; y < 480; y++)
				{
					u32 *scanline = &bitmap.pix(y);
					for (int x = 0; x < 640; x++)
					{
						u8 const pixels = bitswap(vram8[(y * 640) + x], 0, 1, 2, 3, 4, 5, 6, 7);
						*scanline++ = pens[pixels];
					}
				}
			}
			break;

		default:
			fatalerror("procolor816: unknown video mode %x\n", clock_select);
	}

	return 0;
}

void nubus_procolor816_device::regs_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:   // bit 3 = screen enable, bit 2 = VBL control
		case 3:   // ProColor 8 mirrors this here
			m_video_enable = BIT(data, 3);

			if (BIT(data, 2))
			{
				m_vbl_disable = 0;
				lower_slot_irq();
			}
			else
			{
				m_vbl_disable = 1;
			}
			break;

		case 0x800: // Xilinx XC3030 FPGA bitstream write
			break;

		default:
			logerror("%s procolor816_w: %08x @ %x\n", machine().describe_context().c_str(), data, offset);
			break;
	}
}

u8 nubus_procolor816_device::regs_r(offs_t offset)
{
	switch (offset)
	{
		case 0x1:
			return m_screen->vblank() ? 1 : 0;

		default:
			logerror("%s procolor816_r: unknown @ %x\n", machine().describe_context().c_str(), offset);
			break;
	}
	return 0;
}

void nubus_procolor816_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u32 nubus_procolor816_device::vram_r(offs_t offset, u32 mem_mask)
{
	return m_vram[offset];
}

class pds30_procolor8_device : public nubus_procolor816_device
{
public:
	// construction/destruction
	pds30_procolor8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

ROM_START( procolor8_pds )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "procolorserver8.bin", 0x000000, 0x008000, CRC(8701ba41) SHA1(ef2d7ad3309a0df16ec392a82110118830d276b5) )
ROM_END

const tiny_rom_entry *pds30_procolor8_device::device_rom_region() const
{
	return ROM_NAME(procolor8_pds);
}

pds30_procolor8_device::pds30_procolor8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_procolor816_device(mconfig, PDS030_PROCOLOR8, tag, owner, clock)
{
	m_has_15bpp = false;
}

class nubus_procolor8_device : public nubus_procolor816_device
{
public:
	// construction/destruction
	nubus_procolor8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

ROM_START( procolor8_nb )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "lapisprocolorserver.bin", 0x000000, 0x008000, CRC(cd2a726b) SHA1(bd4ceb4b229c9ce3e8f5386d5132ff26de6067e1) )
ROM_END

const tiny_rom_entry *nubus_procolor8_device::device_rom_region() const
{
	return ROM_NAME(procolor8_nb);
}

nubus_procolor8_device::nubus_procolor8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_procolor816_device(mconfig, NUBUS_PROCOLOR8, tag, owner, clock)
{
	m_has_15bpp = false;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(PDS030_PROCOLOR816, device_nubus_card_interface, nubus_procolor816_device, "pd3_pc16", "Lapis ProColor Server 8*16")
DEFINE_DEVICE_TYPE_PRIVATE(PDS030_PROCOLOR8, device_nubus_card_interface, pds30_procolor8_device, "pd3_pcs8", "Lapis ProColor Server 8 (PDS)")
DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_PROCOLOR8, device_nubus_card_interface, nubus_procolor8_device, "nb_pcs8", "Lapis ProColor Server 8")
