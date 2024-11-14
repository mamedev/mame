// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Radius Thunder IV GX video card emulation

  NuBus video card, 1/4/8/24 bit color, works up to at least MacOS 9.1.
  Emulation by R. Belmont, with hat tips to Vas Crabb and Al Kossow.

  This is Radius branded, but it was released shortly after the Radius/SuperMac
  merger and the hardware is distinctly SuperMac flavored.  So it's likely a
  finished/near finished SuperMac design that was just rebranded.  The accelerator
  registers look a lot like an evolved version of the blitter in the Spectrum PDQ,
  and the CRTC is similar to the SuperMac CRTC in nubus/supermac.cpp.

  Usage:
  - Hold down "T" when the Radius screen appears after the beep, and keep it
    held down until you see the video mode you want.  Many more options are
    available if you install RadiusWare 3.4.1 (earlier versions may not
    support this card).
  - Do NOT enable acceleration in RadiusWare, however.  The emulation isn't there yet.

  TODO:
  - Monitor sensing (working for standard Apple monitors, but there are others)
  - Sub-model sensing (We're IDing as the 1366, the 1600 is the top-end model)
  - Acceleration/blitter (it's part-way there but not fully)
  - Photoshop acceleration DSPs.  These didn't work past MacOS 7.6 on hardware
    and the PPC-native version of Photoshop was as fast for cheaper, but it'd
    still be fun.

***************************************************************************/

#include "emu.h"
#include "thunder4gx.h"

#include "emupal.h"
#include "screen.h"

#define LOG_BLITTER (1U << 1)
#define LOG_CLOCKGEN (1U << 2)
#define LOG_MONSENSE (1U << 3)
#define LOG_RAMDAC (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

static constexpr u32 VRAM_SIZE = 0x600000;

class nubus_thunder4gx_device : public device_t,
						  public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_thunder4gx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_thunder4gx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport m_monitor_config;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	u32 registers_r(offs_t offset, u32 mem_mask);
	void registers_w(offs_t offset, u32 data, u32 mem_mask);
	u32 ramdac_r(offs_t offset, u32 mem_mask);
	void ramdac_w(offs_t offset, u32 data, u32 mem_mask);
	u32 accel_r(offs_t offset, u32 mem_mask);
	void accel_w(offs_t offset, u32 data, u32 mem_mask);
	void clockgen_w(offs_t offset, u32 data, u32 mem_mask);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	std::unique_ptr<u32[]> m_vram;
	u32 m_mode, m_irq_control;
	u32 m_pal_idx, m_pal_address;
	u32 m_hres, m_vres, m_stride, m_htotal, m_vtotal;
	u32 m_display_enable;
	u32 m_pixel_clock;
	u32 m_crtc[12];
	u32 m_accel_regs[0x2000/4];
	u32 m_monitor_id;

	u32 m_blitter_dest_address, m_blitter_src_address, m_blitter_param1, m_blitter_param2, m_blitter_small_pattern, m_blitter_mask;
	u32 m_blitter_medium_pattern[16];
	u32 m_blitter_large_pattern[64];
};

ROM_START( thundergx )
	ROM_REGION(0x10000, "declrom", 0)
	ROM_LOAD("radius thunder gx v1.2.3.rom", 0x000000, 0x010000, CRC(b8e035b6) SHA1(c5a1eab749b76aaf3e0ede2120fa99dd7f720196))
ROM_END

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static constexpr u8 ext6(u8 bc, u8 ac, u8 ab)
{
	return 0xc0 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(monitor_config)
	PORT_START("monitor")
	PORT_CONFNAME(0x1ff, ext6(0, 2, 3), "Monitor type")
	PORT_CONFSETTING(0x00, u8"Mac 21\" Color Display (1152\u00d7870)")          // "RGB 2 Page" or "Kong"
	PORT_CONFSETTING(0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")    // "Full Page" or "Portrait"
	PORT_CONFSETTING(0x02, u8"Mac RGB Display (12\" 512\u00d7384)")             // "Rubik" (modified IIgs AppleColor RGB)
	PORT_CONFSETTING(0x03, u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")   // "2 Page"
	PORT_CONFSETTING(0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")       // "High Res"
	PORT_CONFSETTING(ext(1, 1, 3), "640\u00d7480 VGA")
	PORT_CONFSETTING(ext(2, 3, 1), "832\u00d7624 16\" RGB")                     // "Goldfish" or "16 inch RGB"
	PORT_CONFSETTING(ext(3, 2, 2), "1024x\u00d7768 19\" RGB")
	PORT_CONFSETTING(ext6(0, 0, 3), "Multiple Scan 14\"")
	PORT_CONFSETTING(ext6(0, 2, 3), "Multiple Scan 16\"")
	PORT_CONFSETTING(ext6(2, 0, 3), "Multiple Scan 21\"")
INPUT_PORTS_END

void nubus_thunder4gx_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_thunder4gx_device::screen_update));
	m_screen->set_size(1600, 1200);
	m_screen->set_visarea(0, 1152 - 1, 0, 870 - 1);
	m_screen->set_refresh_hz(60);
	m_screen->screen_vblank().set(FUNC(nubus_thunder4gx_device::vblank_w));

	PALETTE(config, m_palette).set_entries(256);
}

const tiny_rom_entry *nubus_thunder4gx_device::device_rom_region() const
{
	return ROM_NAME( thundergx );
}

ioport_constructor nubus_thunder4gx_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

nubus_thunder4gx_device::nubus_thunder4gx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_thunder4gx_device(mconfig, NUBUS_THUNDERIVGX, tag, owner, clock)
{
}

nubus_thunder4gx_device::nubus_thunder4gx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_monitor_config(*this, "monitor"),
	m_mode(0), m_irq_control(0), m_pal_idx(0), m_pal_address(0),
	m_hres(0), m_vres(0), m_stride(128), m_htotal(0), m_vtotal(0),
	m_display_enable(0), m_pixel_clock(30282182), m_monitor_id(0xffff),
	m_blitter_dest_address(0), m_blitter_src_address(0), m_blitter_param1(0),
	m_blitter_param2(0), m_blitter_small_pattern(0), m_blitter_mask(0)
{
	std::fill(std::begin(m_crtc), std::end(m_crtc), 0);
	std::fill(std::begin(m_blitter_medium_pattern), std::end(m_blitter_medium_pattern), 0);
	std::fill(std::begin(m_blitter_large_pattern), std::end(m_blitter_large_pattern), 0);}

void nubus_thunder4gx_device::device_start()
{
	const u32 slotspace = get_slotspace();

	install_declaration_rom("declrom");

	m_vram = std::make_unique<u32[]>(VRAM_SIZE);

	save_item(NAME(m_mode));
	save_item(NAME(m_irq_control));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_stride));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_display_enable));
	save_pointer(NAME(m_vram), VRAM_SIZE);

	install_bank(slotspace, slotspace+VRAM_SIZE-1, &m_vram[0]);

	nubus().install_writeonly_device(slotspace+0xc00000, slotspace+0xc0000f, emu::rw_delegate(*this, FUNC(nubus_thunder4gx_device::clockgen_w)));
	nubus().install_device(slotspace+0xc40000, slotspace+0xc8ffff, emu::rw_delegate(*this, FUNC(nubus_thunder4gx_device::registers_r)), emu::rw_delegate(*this, FUNC(nubus_thunder4gx_device::registers_w)));
	nubus().install_device(slotspace+0xcc0000, slotspace+0xcc1fff, emu::rw_delegate(*this, FUNC(nubus_thunder4gx_device::accel_r)), emu::rw_delegate(*this, FUNC(nubus_thunder4gx_device::accel_w)));
	nubus().install_device(slotspace+0xd00000, slotspace+0xd000ff, emu::rw_delegate(*this, FUNC(nubus_thunder4gx_device::ramdac_r)), emu::rw_delegate(*this, FUNC(nubus_thunder4gx_device::ramdac_w)));
}

void nubus_thunder4gx_device::device_reset()
{
	m_pal_idx = 0;
	m_pal_address = 0;
	m_irq_control = 0;
	m_mode = 0;
	m_display_enable = 0;
}

u32 nubus_thunder4gx_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
	const pen_t *pens = m_palette->pens();

	if ((m_display_enable != 0xffff) || (m_hres == 0) || (m_vres == 0))
	{
		return 0;
	}

	switch (m_mode)
	{
		case 0: // 1bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/8; x++)
				{
					u8 const pixels = vram8[(y * m_stride) + x];

					*scanline++ = pens[pixels&0x80];
					*scanline++ = pens[(pixels<<1)&0x80];
					*scanline++ = pens[(pixels<<2)&0x80];
					*scanline++ = pens[(pixels<<3)&0x80];
					*scanline++ = pens[(pixels<<4)&0x80];
					*scanline++ = pens[(pixels<<5)&0x80];
					*scanline++ = pens[(pixels<<6)&0x80];
					*scanline++ = pens[(pixels<<7)&0x80];
				}
			}
			break;

		case 2: // 4bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/2; x++)
				{
					u8 const pixels = vram8[(y * m_stride) + x];

					*scanline++ = pens[((pixels << 0) & 0xf0)];
					*scanline++ = pens[((pixels << 4) & 0xf0)];
				}
			}
			break;

		case 3: // 8bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u8 const pixels = vram8[(y * m_stride) + x];

					*scanline++ = pens[pixels];
				}
			}
			break;

		case 6: // 16bpp x555
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u16 const pixels = (vram8[(y * m_stride) + (x << 1)] << 8) | vram8[(y * m_stride) + (x << 1) + 1];
					*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
				}
			}
			break;

		case 7: // 24bpp (arranged as 32 bits, so there's a wasted byte per pixel)
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				u32 const *base = &m_vram[(y * (m_stride >> 2))];
				for (int x = 0; x < m_hres; x++)
				{
					*scanline++ = *base++;
				}
			}
			break;
	}

	return 0;
}

void nubus_thunder4gx_device::vblank_w(int state)
{
	if ((state) && (BIT(m_irq_control, 1)))
	{
		raise_slot_irq();
	}
}

u32 nubus_thunder4gx_device::registers_r(offs_t offset, u32 mem_mask)
{
	switch (offset)
	{
		case 0:
			return 0x6;

		// bit 16 = accelerator ready flag (or active-low busy)
		// bit 17 = vblank
		case 1:
			return m_screen->vblank() ? 0x30000 : 0x10000;

		case 2:
			return m_irq_control;

		case 3:     // IRQ routine reads this immediately after writing to ack the IRQ (or does reading this ack it?)
			return 0;

		case 9:
			// 0a0028 = RadiusColor Twin
			return 0x0a0028;

		case 0x10000:   // mode
			return m_mode;

		case 0x10002:   // monitor sense
		{
			u8 mon = m_monitor_config->read();
			u8 monitor_id = ((m_monitor_id >> 2) ^ 7) & 0x7;
			u8 res;
			if (mon & 0x40)
			{
				res = 7;

				if (mon & 0x80)
				{
					res = 6;
				}

				if (monitor_id == 0x4)
				{
					res &= 4 | (BIT(mon, 5) << 1) | BIT(mon, 4);
				}
				if (monitor_id == 0x2)
				{
					res &= (BIT(mon, 3) << 2) | 2 | BIT(mon, 2);
				}
				if (monitor_id == 0x1)
				{
					res &= (BIT(mon, 1) << 2) | (BIT(mon, 0) << 1) | 1;
				}
			}
			else
			{
				res = mon;
			}

			LOGMASKED(LOG_MONSENSE, "Sense result = %x\n", res);
			return (res << 2) | 0x3;
		}

		default:
			LOGMASKED(LOG_GENERAL, "Read @ C4xxxx %08x mask %08x\n", offset, mem_mask);
			break;
		}
			return 0;
}

void nubus_thunder4gx_device::registers_w(offs_t offset, u32 data, u32 mem_mask)
{
	data &= mem_mask;
	switch (offset)
	{
		case 1:
			if (data & 2)
			{
				lower_slot_irq();
			}
			break;

		case 2:
				m_irq_control = data;
				break;

		case 9:
			LOGMASKED(LOG_MONSENSE, "Write %08x to extended sense, mask %08x\n", data, mem_mask);
			break;

		case 0x10000:
				m_mode = data & 0xf;
				LOGMASKED(LOG_GENERAL, "%x to mode\n", data);
				break;

		case 0x10002:
				LOGMASKED(LOG_MONSENSE, "%x to monitor drive\n", data);
				m_monitor_id = data;
				break;

		case 0x10005: case 0x10006: case 0x10007: case 0x10008: case 0x10009: case 0x1000a:
		case 0x1000b: case 0x1000c: case 0x1000d: case 0x1000e: case 0x1000f:
			m_crtc[offset - 0x10005] = data & mem_mask;
			LOGMASKED(LOG_GENERAL, "%04x to CRTC @ %x (%x)\n", data & mem_mask, offset, offset - 0x10005);
			break;

		default:
				LOGMASKED(LOG_GENERAL, "%s Write @ C4xxxx: %x to reg %x (mask %x)\n", machine().describe_context(), data & mem_mask, offset, mem_mask);
				break;
	}
}

u32 nubus_thunder4gx_device::ramdac_r(offs_t offset, u32 mem_mask)
{
	LOGMASKED(LOG_RAMDAC, "%s: Read DAC @ %08x (mask %08x)\n", machine().describe_context(), offset, mem_mask);

	return 0;
}

void nubus_thunder4gx_device::ramdac_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RAMDAC, "%s: %x to DAC at %x\n", machine().describe_context(), data & mem_mask, offset);
	switch (offset)
	{
		case 0:
			m_pal_address = (data&0xff);
			m_pal_idx = 0;
			break;

		case 1:
			switch (m_pal_idx)
			{
				case 0:
					m_palette->set_pen_red_level(m_pal_address, ((data>>2) & 0xff));
					break;

				case 1:
					m_palette->set_pen_green_level(m_pal_address, ((data >> 2) & 0xff));
					break;

				case 2:
					m_palette->set_pen_blue_level(m_pal_address, ((data >> 2) & 0xff));
					break;
			}

			m_pal_idx++;
			if (m_pal_idx == 3)
			{
				m_pal_address++;
				m_pal_idx = 0;
			}
			break;

		case 2:
			m_display_enable = data & mem_mask;
			LOGMASKED(LOG_GENERAL, "%x to display enable\n", m_display_enable);
			break;

		default:
			LOGMASKED(LOG_GENERAL, "%08x to DAC @ %x\n", data & mem_mask, offset);
			break;
	}
}

u32 nubus_thunder4gx_device::accel_r(offs_t offset, u32 mem_mask)
{
	LOGMASKED(LOG_BLITTER, "Read accelerator @ %08x (mask %08x)\n", offset, mem_mask);

	return m_accel_regs[offset];
}

void nubus_thunder4gx_device::accel_w(offs_t offset, u32 data, u32 mem_mask)
{
	data &= mem_mask;

	switch (offset)
	{
		case 2:
			LOGMASKED(LOG_GENERAL, "%d to stride\n", data);
			m_stride = data;
			break;

		case 0x41:  // command
			// 0x1201 is a small pattern fill from the 4 byte pattern in register 0x47
			// 0x1401 is a medium pattern fill from the 64 bytes of pattern data in registers 0x80-0x8f
			// 0x1045 is a large pattern fill from the 256 bytes of pattern data in registers 0x100-0x13f
			// 0x1002 is a copy from the source VRAM address in register 0x42 to the destination VRAM address in register 0x43

			LOGMASKED(LOG_BLITTER, "Blitter opcode %04x\n", data);

			// The blitter is only used in 8, 16, and 32 bpp modes.  1/2/4 bpp are unaccelerated.
			assert(m_mode >= 3);
			switch (data)
			{
				case 0x1002:    // VRAM to VRAM copy
					switch (m_mode)
					{
						case 3: // 8 bpp
						{
							u32 vram_src = (m_blitter_src_address & 0x7fffff);
							u32 vram_dest = (m_blitter_dest_address & 0x7fffff);
							const u16 width = m_blitter_param1 & 0xffff;
							const u16 height = m_blitter_param1 >> 16;
							u8 *vram8 = (u8 *)&m_vram[0];
							for (int y = 0; y < height; y++)
							{
								for (int x = 0; x < width; x++)
								{
									vram8[BYTE4_XOR_BE(vram_dest + x)] = vram8[BYTE4_XOR_BE(vram_src + x)];
								}
								vram_src += m_stride;
								vram_dest += m_stride;
							}
						}
						break;

						case 5: // 16 bpp
						{
							u32 vram_src = (m_blitter_src_address & 0x7fffff) >> 1;
							u32 vram_dest = (m_blitter_dest_address & 0x7fffff) >> 1;
							const u16 width = m_blitter_param1 & 0xffff;
							const u16 height = m_blitter_param1 >> 16;
							u16 *vram16 = (u16 *)&m_vram[0];
							for (int y = 0; y < height; y++)
							{
								for (int x = 0; x < width; x++)
								{
									vram16[WORD_XOR_BE(vram_dest + x)] = vram16[WORD_XOR_BE(vram_src + x)];
								}
								vram_src += (m_stride >> 1);
								vram_dest += (m_stride >> 1);
							}
						}
						break;

						case 6: // 32 bpp
						{
							u32 vram_src = (m_blitter_src_address & 0x7fffff) >> 2;
							u32 vram_dest = (m_blitter_dest_address & 0x7fffff) >> 2;
							const u16 width = m_blitter_param1 & 0xffff;
							const u16 height = m_blitter_param1 >> 16;
							for (int y = 0; y < height; y++)
							{
								for (int x = 0; x < width; x++)
								{
									m_vram[vram_dest + x] = m_vram[vram_src + x];
								}
								vram_src += (m_stride >> 2);
								vram_dest += (m_stride >> 2);
							}
						}
						break;
					}
				break;

				case 0x1201:
					switch (m_mode)
					{
						case 3: // 8 bpp
						{
							u32 vram_offs = (m_blitter_dest_address & 0x7fffff);
							const u16 width = m_blitter_param1 & 0xffff;
							const u16 height = m_blitter_param1 >> 16;
							u8 *vram8 = (u8 *)&m_vram[0];
							u8 const *const pattern8 = (u8 *)&m_blitter_small_pattern; // FIXME: endianness

							for (int y = 0; y < height; y++)
							{
								for (int x = 0; x < width; x++)
								{
									vram8[BYTE4_XOR_BE(vram_offs + x)] = pattern8[x % 3];   // is this relative to the start or is the pattern fixed?
								}
								vram_offs += m_stride;
							}
						}
						break;

						case 5: // 16 bpp
						{
							u32 vram_offs = (m_blitter_dest_address & 0x7fffff) >> 1;
							const u16 width = m_blitter_param1 & 0xffff;
							const u16 height = m_blitter_param1 >> 16;
							u16 *vram16 = (u16 *)&m_vram[0];
							u16 const *const pattern16 = (u16 *)&m_blitter_small_pattern; // FIXME: endianness

							for (int y = 0; y < height; y++)
							{
								for (int x = 0; x < width; x++)
								{
									vram16[WORD_XOR_BE(vram_offs + x)] = pattern16[x & 1];
								}
								vram_offs += (m_stride >> 1);
							}
						}
						break;

						case 6: // 32 bpp
						{
							u32 vram_offs = (m_blitter_dest_address & 0x7fffff) >> 2;
							const u16 width = m_blitter_param1 & 0xffff;
							const u16 height = m_blitter_param1 >> 16;
							for (int y = 0; y < height; y++)
							{
								for (int x = 0; x < width; x++)
								{
									m_vram[vram_offs + x] = m_blitter_small_pattern;
								}
								vram_offs += (m_stride >> 2);
							}
						}
						break;
					}
					break;

				case 0x1401:
					switch (m_mode)
					{
						case 3: // 8 bpp
						{
							u32 vram_offs = (m_blitter_dest_address & 0x7fffff);
							const u16 width = (m_blitter_param1 & 0xffff);
							const u16 height = m_blitter_param1 >> 16;
							u16 pat_width = (m_blitter_param2 & 0xff);
							u16 pat_height = (m_blitter_param2 >> 8) & 0xff;
							u8 *vram8 = (u8 *)&m_vram[0];
							u8 const *const pattern8 = (u8 *)&m_blitter_medium_pattern; // FIXME: endianness

							if (pat_width == 0)
							{
								pat_width = (m_blitter_param2 >> 16) & 0xff;
							}

							if (pat_height == 0)
							{
								pat_height = (m_blitter_param2 >> 24) & 0xff;
							}

							// is this some kind of indirection or protection or something?
							if (pat_width == 0)
							{
								pat_width = pat_height = 8;
							}

							LOGMASKED(LOG_BLITTER, "Medium pattern fill 8: offs %08x width %d height %d pat width %d pat height %d\n", vram_offs, width, height, pat_width, pat_height);
							for (int y = 0; y < height; y++)
							{
								const int pat_offs = (y % pat_height) * pat_width;
								LOGMASKED(LOG_BLITTER, "Line %d vram offs %08x pattern offs %d\n", y, vram_offs, pat_offs % (16 * 4));
								for (int x = 0; x < width; x++)
								{
									vram8[BYTE4_XOR_BE(vram_offs + x)] = pattern8[(pat_offs + (x % pat_width)) % (16 * 4)];
								}
								vram_offs += m_stride;
							}
						}
						break;

						case 5: // 16 bpp
						{
							u32 vram_offs = (m_blitter_dest_address & 0x7fffff)>>1;
							const u16 width = (m_blitter_param1 & 0xffff);
							const u16 height = m_blitter_param1 >> 16;
							const u16 pat_width = (m_blitter_param2 & 0xff);
							const u16 pat_height = (m_blitter_param2 >> 8) & 0xff;
							u16 *vram16 = (u16 *)&m_vram[0];
							u16 const *const pattern16 = (u16 *)&m_blitter_medium_pattern; // FIXME: endianness

							LOGMASKED(LOG_BLITTER, "Medium pattern fill 16: offs %08x width %d height %d pat width %d pat height %d\n", vram_offs, width, height, pat_width, pat_height);
							for (int y = 0; y < height; y++)
							{
								const int pat_offs = (y % pat_height) * pat_width;
								LOGMASKED(LOG_BLITTER, "Line %d vram offs %08x pattern offs %d\n", y, vram_offs, pat_offs % (16 * 2));
								for (int x = 0; x < width; x++)
								{
									vram16[WORD_XOR_BE(vram_offs + x)] = pattern16[(pat_offs + (x % pat_width)) % (16 * 2)];
								}
								vram_offs += m_stride;
							}
						}
						break;

						case 6: // 32 bpp
						{
							u32 vram_offs = (m_blitter_dest_address & 0x7fffff) >> 2; // convert address to offset in 32-bit words
							const u16 width = (m_blitter_param1 & 0xffff) >> 2;
							const u16 height = m_blitter_param1 >> 16;
							const u16 pat_width = (m_blitter_param2 & 0xff) >> 2;
							const u16 pat_height = (m_blitter_param2 >> 8) & 0xff;
							LOGMASKED(LOG_BLITTER, "Medium pattern fill 32: offs %08x width %d height %d pat width %d pat height %d\n", vram_offs, width, height, pat_width, pat_height);
							for (int y = 0; y < height; y++)
							{
								const int pat_offs = (y % pat_height) * pat_width;
								LOGMASKED(LOG_BLITTER, "Line %d vram offs %08x pattern offs %d\n", y, vram_offs, pat_offs);
								for (int x = 0; x < width; x++)
								{
									m_vram[vram_offs + x] = m_blitter_medium_pattern[(pat_offs + (x % pat_width)) % 16];
								}
								vram_offs += (m_stride >> 2);
							}
						}
						break;
					}
					break;
				}
			break;

		case 0x42: // physical source VRAM address (does this mean the card can accelerate cards in other slots?)
			LOGMASKED(LOG_BLITTER, "%08x to blitter source\n", data);
			m_blitter_src_address = data;
			break;

		case 0x43:  // physical destination VRAM address (does this mean the card can accelerate cards in other slots?)
			LOGMASKED(LOG_BLITTER, "%08x to blitter destination\n", data);
			m_blitter_dest_address = data;
			break;

		case 0x44:  // Blitter parameter 1
			LOGMASKED(LOG_BLITTER, "%08x to blitter param 1 (mask %08x)\n", data, mem_mask);
			m_blitter_param1 = data;
			break;

		case 0x45: // Blitter parameter 2
			LOGMASKED(LOG_BLITTER, "%08x to blitter param 2 (mask %08x)\n", data, mem_mask);
			m_blitter_param2 = data;
			break;

		case 0x49:  // Small blitter pattern
			LOGMASKED(LOG_BLITTER, "%08x to small pattern\n", data);
			m_blitter_small_pattern = data;
			break;

		case 0x48:  // Blitter mask?
			LOGMASKED(LOG_BLITTER, "%08x to mask?\n", data);
			m_blitter_mask = data;
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			LOGMASKED(LOG_BLITTER, "%08x to medium pattern @ %x\n", data, offset & 0xf);
			m_blitter_medium_pattern[offset & 0xf] = data;
			break;

		case 0x100: case 0x101: case 0x102: case 0x103: case 0x104: case 0x105: case 0x106: case 0x107:
		case 0x108: case 0x109: case 0x10a: case 0x10b: case 0x10c: case 0x10d: case 0x10e: case 0x10f:
		case 0x110: case 0x111: case 0x112: case 0x113: case 0x114: case 0x115: case 0x116: case 0x117:
		case 0x118: case 0x119: case 0x11a: case 0x11b: case 0x11c: case 0x11d: case 0x11e: case 0x11f:
		case 0x120: case 0x121: case 0x122: case 0x123: case 0x124: case 0x125: case 0x126: case 0x127:
		case 0x128: case 0x129: case 0x12a: case 0x12b: case 0x12c: case 0x12d: case 0x12e: case 0x12f:
		case 0x130: case 0x131: case 0x132: case 0x133: case 0x134: case 0x135: case 0x136: case 0x137:
		case 0x138: case 0x139: case 0x13a: case 0x13b: case 0x13c: case 0x13d: case 0x13e: case 0x13f:
			LOGMASKED(LOG_BLITTER, "%08x to large pattern @ %x\n", data, offset - 0x100);
			m_blitter_large_pattern[offset - 0x100] = data;
			break;

		default:
			LOGMASKED(LOG_BLITTER, "%s: %08x to blitter @ %08x (mask %08x)\n", machine().describe_context(), data, offset, mem_mask);
			break;
	}

	m_accel_regs[offset] = data;
}

// IC Designs ICD2062B frequency synthesizer frontend.
// The chip has an I2C-style CLK/DATA 2 wire serial interface, but on this card
// you just write all 21 bits as a longword and the ASIC does it for you.
void nubus_thunder4gx_device::clockgen_w(offs_t offset, u32 data, u32 mem_mask)
{
	const int irange = (data >> 17) & 0xf;
	const int p = ((data >> 10) & 0x7f) + 3;
	const int mux = (data >> 7) & 0x7;
	const int q = (data & 0x7f) + 2;

	LOGMASKED(LOG_CLOCKGEN, "I %d P %d Mux %d Q %d\n", irange, p, mux, q);

	double result = 2.0f * 14.3181818f * (double)p;
	result /= (double)q;
	result /= (double)(1 << mux);
	m_pixel_clock = (int)(result * 1000000 + .5);
	LOGMASKED(LOG_CLOCKGEN, "result %f, pixel clock %d\n", result, m_pixel_clock);

	m_hres = (m_crtc[2] - m_crtc[1]) << 2;
	m_vres = (m_crtc[7] - m_crtc[6]);
	m_htotal = m_crtc[3] << 2;
	m_vtotal = m_crtc[8];

	if ((m_hres > 0) && (m_vres > 0))
	{
		const double refresh = (double)m_pixel_clock / (double)(m_htotal * m_vtotal);
		LOGMASKED(LOG_CLOCKGEN, "hres %d vres %d htotal %d vtotal %d refresh %f stride %d mode %d\n", m_hres, m_vres, m_htotal, m_vtotal, refresh, m_stride, m_mode);

		rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
		m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock).as_attoseconds());
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_THUNDERIVGX, device_nubus_card_interface, nubus_thunder4gx_device, "nb_thungx", "Radius Thunder IV GX video card")
