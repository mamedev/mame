// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    suprterminal.cpp
    M&R Enterprises SUP'R'TERMINAL 80-column card.

    This card only works in Slot 3.  The firmware will crash in any other slot.
    Emulation tested and works with DOS 3.3, ProDOS, and CP/M.

    $C0B2: Select VRAM at $C800 and disable ROM at $C800
    $C0B4: Select font RAM at $C800 and disable ROM at $C800
    $C0B6: Select bank 2 of (VRAM? font RAM?).  This is unused on this card,
           and there are not enough DRAMs (or places for them!) to make it work.
    $C0B8: 6845 address
    $C0B9: 6845 data

    The $C800 handling on this card is unusual.
    Disabling the card's $C800 space by accessing $CFFF resets the card to ROM
    at $C800.  Enabling either VRAM or font RAM at $C800 disables ROM until
    $CFFF is accessed.

*********************************************************************/

#include "emu.h"
#include "suprterminal.h"

#include "video/mc6845.h"

#include "screen.h"


namespace {

ROM_START(a2suprterm)
	ROM_REGION(0x800, "s3firmware", ROMREGION_ERASEFF)
	ROM_LOAD("suprterminal.bin", 0x000000, 0x000800, CRC(4c49b0f5) SHA1(66a7411c46c04a8089a7ddfb5ffd9809dd08a21f))
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_suprterminal_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_suprterminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	a2bus_suprterminal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual u8 read_cnxx(u8 offset) override;
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_c800(u16 offset) override;
	virtual void write_c800(u16 offset, u8 data) override;
	virtual bool take_c800() override { return true; }

private:
	required_device<mc6845_device> m_crtc;
	required_region_ptr<u8> m_rom;

	MC6845_UPDATE_ROW(crtc_update_row);

	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_fontram;
//  u8 m_fontram[0x400];
	bool m_bRasterRAM, m_bCharBank1, m_bC800IsRAM;
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_suprterminal_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(17.43_MHz_XTAL, 1116, 0, 720, 260, 0, 216);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	HD6845S(config, m_crtc, 17.43_MHz_XTAL / 9);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(a2bus_suprterminal_device::crtc_update_row));
}

const tiny_rom_entry *a2bus_suprterminal_device::device_rom_region() const
{
	return ROM_NAME(a2suprterm);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_suprterminal_device::a2bus_suprterminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	a2bus_suprterminal_device(mconfig, A2BUS_SUPRTERMINAL, tag, owner, clock)
{
}

a2bus_suprterminal_device::a2bus_suprterminal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_crtc(*this, "crtc"),
	m_rom(*this, "s3firmware")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_suprterminal_device::device_start()
{
	m_vram = std::make_unique<u8[]>(0x800); // 4 2114 DRAMs
	m_fontram = std::make_unique<u8[]>(0x400); // 2 2114 DRAMs

	m_bRasterRAM = true;
	m_bCharBank1 = false;

	save_item(NAME(m_bRasterRAM));
	save_item(NAME(m_bCharBank1));
	save_item(NAME(m_bC800IsRAM));
	save_pointer(NAME(m_vram), 0x800);
	save_pointer(NAME(m_fontram), 0x400);
}

void a2bus_suprterminal_device::device_reset()
{
}

u8 a2bus_suprterminal_device::read_cnxx(u8 offset)
{
	return m_rom[offset+0x300];
}

u8 a2bus_suprterminal_device::read_c0nx(u8 offset)
{
	switch (offset)
	{
		case 9:
			return m_crtc->register_r();
	}

	return 0xff;
}

void a2bus_suprterminal_device::write_c0nx(u8 offset, u8 data)
{
	switch (offset)
	{
		case 2:
			m_bRasterRAM = true;
			m_bC800IsRAM = true;
			break;

		case 4:
			m_bRasterRAM = false;
			m_bC800IsRAM = true;
			break;

		case 6:
			m_bCharBank1 ^= 1;
			break;

		case 8:
			m_crtc->address_w(data);
			break;

		case 9:
			m_crtc->register_w(data);
			break;
	}
}

u8 a2bus_suprterminal_device::read_c800(u16 offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 0x7ff)
		{
			m_bC800IsRAM = false;
		}
	}

	if (m_bC800IsRAM)
	{
		if (!m_bRasterRAM)
		{
			return m_vram[offset];
		}
		else
		{
			return m_fontram[offset&0x3ff];
		}
	}

	return m_rom[offset];
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_suprterminal_device::write_c800(u16 offset, u8 data)
{
	if (offset == 0x7ff)
	{
		m_bC800IsRAM = false;
	}

	if (!m_bRasterRAM)
	{
		m_vram[offset] = data;
	}
	else
	{
		m_fontram[offset&0x3ff] = data;
	}
}

MC6845_UPDATE_ROW(a2bus_suprterminal_device::crtc_update_row)
{
	u32 *p = &bitmap.pix(y);
	u16 chr_base = ra;

	for (int i = 0; i < x_count; i++)
	{
		u16 offset = (ma + i) & 0x7ff;
		u8 chr = m_vram[offset] & 0x7f;
		u8 data = m_fontram[chr_base + chr * 8];
		rgb_t fg = rgb_t::white();
		rgb_t bg = rgb_t::black();

		if (i == cursor_x)
		{
			std::swap(fg, bg);
		}

		for (int j = 9; j > 0; j--)
		{
			*p++ = BIT(data, 7) ? fg : bg;
			data <<= 1;
		}
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SUPRTERMINAL, device_a2bus_card_interface, a2bus_suprterminal_device, "a2suprterm", "M&R Enterprises SUP'R'TERMINAL")
