// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    RasterOps ColorVue 8LC
    1, 2, 4, and 8 bpp at 1024x768 for the Macintosh LC

    This is the same hardware as the ClearVueGS/LC board, which shows grayscale.
    Both cards are documented to only work with the original LC due to some shortcuts
    taken in how they map the address space.

    Note: the current ROM is a prototype from a former ROPS employee.
    RasterOps' graphics driver software doesn't recognize it to enable the pan/zoom functionality.
    ROM dumps from a retail card would resolve several questions.

    Map:
    0xXXX00000: VRAM (768K)
    0xXXXC0000: TMS34061 registers
    0xXXXD0000: Control register + DIP switches

    Control register layout:
    bits 0-1: oscillator select (0 = 80.0 MHz, 1 = 57.2832 MHz, 2/3 unused)
    bits 2-3: mode select (0 = 1 bpp, 1 = 2 bpp, 2 = 4 bpp, 3 = 8 bpp)
    bits 4-5: X zoom amount (0 = 1x, 1 = 2x, 2 = 4x, 3 = 8x)
    bit 6: Vblank status, negative logic (0 = active)
    bit 7: Display status (0 = disabled, 1 = enabled)
    bit 8: monitor sense (1 = connected)
    bit 11: Display enable switch on the 708+/SE this is derived from

    TODO:
    - Get real declaration ROMs for 8LC and ClearVueGS/LC and other related cards
    - Implement pan/zoom functionality if supported, which likely requires VRAM to go through the TMS34061

***************************************************************************/

#include "emu.h"
#include "8lc.h"

#include "video/tms34061.h"

#include "emupal.h"
#include "screen.h"

#define LOG_REGISTERS (1U << 1)
#define LOG_RAMDAC (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

static constexpr u32 VRAM_SIZE = 0xc0000;

class lcpds_cv8lc_device : public device_t,
						  public device_nubus_card_interface
{
public:
	// construction/destruction
	lcpds_cv8lc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	lcpds_cv8lc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<tms34061_device> m_tms34061;
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

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	std::unique_ptr<u32[]> m_vram;
	u32 m_mode, m_irq_control;
	u32 m_pal_idx, m_pal_address;
	u32 m_hres, m_vres, m_stride, m_htotal, m_vtotal, m_pixel_clock;
	u32 m_display_enable;
	u32 m_crtc[12];
};

ROM_START( cb708plc )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "8lc_prototype.bin", 0x000000, 0x008000, CRC(6f47e0f2) SHA1(a8552315d9127aee92416aa22893551851f42e5c) )
ROM_END

static INPUT_PORTS_START(monitor_config)
	PORT_START("monitor")
	PORT_CONFNAME(0x0100, 0x0000, "Monitor presence")
	PORT_CONFSETTING(0x0000, u8"19\" 1024\u00d7768 RGB");
	PORT_CONFSETTING(0x0100, u8"No monitor")
INPUT_PORTS_END

void lcpds_cv8lc_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(80'000'000, 1312, 0, 1024, 802, 0, 768);
	m_screen->set_screen_update(FUNC(lcpds_cv8lc_device::screen_update));

	PALETTE(config, m_palette).set_entries(256);

	TMS34061(config, m_tms34061, 0);
	m_tms34061->set_rowshift(10); // VRAM address is (row << rowshift) | col
	m_tms34061->set_vram_size(0xc0000);
	m_tms34061->set_screen(m_screen);
	m_tms34061->int_callback().set(FUNC(lcpds_cv8lc_device::vblank_w));
}

const tiny_rom_entry *lcpds_cv8lc_device::device_rom_region() const
{
	return ROM_NAME( cb708plc );
}

ioport_constructor lcpds_cv8lc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

lcpds_cv8lc_device::lcpds_cv8lc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	lcpds_cv8lc_device(mconfig, PDSLC_COLORVUE8LC, tag, owner, clock)
{
}

lcpds_cv8lc_device::lcpds_cv8lc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_tms34061(*this, "tms34061"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_monitor_config(*this, "monitor"),
	m_mode(0), m_irq_control(0), m_pal_idx(0), m_pal_address(0),
	m_hres(1024), m_vres(768), m_stride(1024), m_htotal(0), m_vtotal(0),
	m_pixel_clock(80'000'000), m_display_enable(0)
{
	std::fill(std::begin(m_crtc), std::end(m_crtc), 0);
}

void lcpds_cv8lc_device::device_start()
{
	set_pds_slot(0xe);

	const u32 slotspace = get_slotspace();

	install_declaration_rom("declrom");

	m_vram = std::make_unique<u32[]>(VRAM_SIZE/4);

	save_item(NAME(m_mode));
	save_item(NAME(m_irq_control));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_stride));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_display_enable));
	save_pointer(NAME(m_vram), VRAM_SIZE/4);

	install_bank(0xe00000 + slotspace, 0xe00000 + slotspace + VRAM_SIZE - 1, &m_vram[0]);
	nubus().install_device(slotspace + 0xec0000, slotspace + 0xedffff, emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::registers_r)), emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::registers_w)));
	nubus().install_device(slotspace + 0xee0000, slotspace + 0xee00ff, emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::ramdac_r)), emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::ramdac_w)));

	// The real card appears to ignore A31 (reminder: on the LC and LC II, A24-A30 are not decoded) so we need this extra
	// mapping as well.
	install_bank(0xe00000, 0xe00000 + VRAM_SIZE - 1, &m_vram[0]);
	nubus().install_device(0xec0000, 0xedffff, emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::registers_r)), emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::registers_w)));
	nubus().install_device(0xee0000, 0xee00ff, emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::ramdac_r)), emu::rw_delegate(*this, FUNC(lcpds_cv8lc_device::ramdac_w)));
}

void lcpds_cv8lc_device::device_reset()
{
	m_pal_idx = 0;
	m_pal_address = 0;
	m_irq_control = 0;
	m_mode = 0;
	m_display_enable = 0;
}

uint32_t lcpds_cv8lc_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);
	const pen_t *pens = m_palette->pens();

	m_tms34061->get_display_state();
	if (m_tms34061->blanked())
	{
		bitmap.fill(rgb_t::black(), cliprect);
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

		case 1: // 2bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres / 4; x++)
				{
					u8 const pixels = vram8[(y * m_stride) + x];

					*scanline++ = pens[(pixels & 0xc0)];
					*scanline++ = pens[((pixels << 2) & 0xc0)];
					*scanline++ = pens[((pixels << 4) & 0xc0)];
					*scanline++ = pens[(pixels << 6) & 0xc0];
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
	}

	return 0;
}

void lcpds_cv8lc_device::vblank_w(int state)
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

u32 lcpds_cv8lc_device::registers_r(offs_t offset, u32 mem_mask)
{
	if (offset != 0x34)
	{
		LOGMASKED(LOG_REGISTERS, "Read @ Cxxxx: offset %x (mask %x)\n", offset, mem_mask);
	}

	if (offset == 0x4000)
	{
		u32 result = m_monitor_config->read();
		result |= (m_screen->vblank() ? 0 : 0x0040);
		return result;
	}
	else if (offset < 0x40)
	{
		return m_tms34061->read(offset, 0, 0)<<24;
	}
	else if (offset == 0x44)
	{
		return m_screen->vpos() & 0xff;
	}
	else if (offset == 0x46)
	{
		return (m_screen->vpos() >> 8) & 0xff;
	}

	return 0;
}

void lcpds_cv8lc_device::registers_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_REGISTERS, "Write @ Cxxxx: %x to reg %x (mask %x)\n", data, offset, mem_mask);
	if (offset == 0x4000)
	{
		m_mode = (data >> 2) & 3;

		u32 new_clock;
		switch (data & 3)
		{
			case 0:
				new_clock = 80'000'000;
				break;

			case 1:
				new_clock = 57'283'200;
				break;

			default:
				fatalerror("colorboard708: invalid crystal select %d\n", data & 3);
				break;
		}

		if (m_pixel_clock != new_clock)
		{
			m_pixel_clock = new_clock;
			m_hres = m_tms34061->hvisible();
			m_htotal = m_tms34061->htotal();
			m_vres = m_tms34061->vvisible();
			m_vtotal = m_tms34061->vtotal();

			if ((m_hres != 0) && (m_vres != 0) && (m_htotal != 0) && (m_vtotal != 0))
			{
				rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
				m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock).as_attoseconds());
			}
		}
	}
	else
	{
		if (mem_mask == 0xff000000)
		{
			m_tms34061->write(offset, 0, 0, data >> 24);
		}
		else
		{
			m_tms34061->write(offset, 0, 0, data & 0xff);
		}
	}
}

u32 lcpds_cv8lc_device::ramdac_r(offs_t offset, u32 mem_mask)
{
	LOGMASKED(LOG_RAMDAC, "Read DAC @ %08x (mask %08x)\n", offset, mem_mask);
	return 0;
}

void lcpds_cv8lc_device::ramdac_w(offs_t offset, u32 data, u32 mem_mask)
{
	u8 writedata = (mem_mask == 0xff000000) ? (data >> 24) : (data & 0xff);
	LOGMASKED(LOG_RAMDAC, "%08x (write %02x) to DAC at %x (mask %08x)\n", data, writedata, offset, mem_mask);
	switch (offset)
	{
		case 0:
			m_pal_address = writedata;
			m_pal_idx = 0;
			break;

		case 1:
			switch (m_pal_idx)
			{
				case 0:
					m_palette->set_pen_red_level(m_pal_address, writedata);
					break;

				case 1:
					m_palette->set_pen_green_level(m_pal_address, writedata);
					break;

				case 2:
					m_palette->set_pen_blue_level(m_pal_address, writedata);
					break;
			}

			m_pal_idx++;
			if (m_pal_idx == 3)
			{
				m_pal_address++;
				m_pal_address &= 0xff;
				m_pal_idx = 0;
			}
			break;

		default:
			LOGMASKED(LOG_REGISTERS, "%08x to DAC @ %x\n", data & mem_mask, offset);
			break;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(PDSLC_COLORVUE8LC, device_nubus_card_interface, lcpds_cv8lc_device, "cv8lc", "RasterOps ColorVue 8LC video card")

