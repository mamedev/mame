// license:BSD-3-Clause
// copyright-holders:R. Belmont, Rob Justice
/*********************************************************************

    grafex.cpp

    Implementation of the Grafex-32 card

    This was a graphics board using the upd7220 for the Apple II by Ray Dahlby (RDE - Ray Dahlby Electronics)
    Instructions on how to build this were described in Radio Electronics magazine in 1986
    Febuary, March & April editions.
    http://mirrors.apple2.org.za/ftp.apple.asimov.net/documentation/hardware/video/Grafex-32_from_Radio_Electronics_1986.pdf

    This version has 128k ram emulated. The original had 32k with provision to accept 128k

    A0 selects between the 7220 gdc registers
    A1 switches AppleII video or Grafex video (not implemented)

    Manual:
    https://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Video/RDE%20Grafex/Manuals/RDE%20Grafex%20-%20Manual.pdf

    The software described in the manual does not look like its available anywhere.

    TODO:
    Support for multiple cards, each card supported 1 bit of color. The article describes using
    three cards to get 8 color RGB output. Each plane is driven directly by its dedicated 7220.

*********************************************************************/

#include "emu.h"
#include "grafex.h"

#include "video/upd7220.h"

#include "screen.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_grafex_device : public device_t,
							public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_grafex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_grafex_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	required_device<upd7220_device> m_gdc;
	required_shared_ptr<uint16_t> m_video_ram;

	UPD7220_DISPLAY_PIXELS_MEMBER(hgdc_display_pixels);

	void upd7220_map(address_map &map) ATTR_COLD;

	rgb_t m_bg, m_fg;
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_grafex_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("upd7220", FUNC(upd7220_device::screen_update));
	screen.set_raw(16_MHz_XTAL, 1024, 384, 640, 262, 62, 200);

	UPD7220(config, m_gdc, 2_MHz_XTAL);
	m_gdc->set_addrmap(0, &a2bus_grafex_device::upd7220_map);
	m_gdc->set_display_pixels(FUNC(a2bus_grafex_device::hgdc_display_pixels));
	m_gdc->set_screen("screen");
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_grafex_device::a2bus_grafex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_grafex_device(mconfig, A2BUS_GRAFEX, tag, owner, clock)
{
}

a2bus_grafex_device::a2bus_grafex_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_gdc(*this, "upd7220"),
	m_video_ram(*this, "video_ram")
{
}

void a2bus_grafex_device::device_start()
{
	m_bg = rgb_t::black();
	m_fg = rgb_t::white();
}

uint8_t a2bus_grafex_device::read_c0nx(uint8_t offset)
{
	return m_gdc->read(offset & 0x01);
}

void a2bus_grafex_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_gdc->write(offset & 0x01, data);
}

void a2bus_grafex_device::upd7220_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram().share("video_ram");
}

UPD7220_DISPLAY_PIXELS_MEMBER(a2bus_grafex_device::hgdc_display_pixels )
{
	uint16_t gfx = m_video_ram[(address & 0x1ffff)];

	for (int i=0; i<16; i++)
	{
		bitmap.pix(y, x + i) = BIT(gfx, i) ? m_fg : m_bg;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_GRAFEX, device_a2bus_card_interface, a2bus_grafex_device, "a2grafex", "Grafex-32")
