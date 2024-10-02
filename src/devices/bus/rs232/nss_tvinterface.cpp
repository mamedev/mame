// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Achim
/*******************************************************************************

Novag Super System Peripheral: TV Interface

It's a box that connects to a CRT TV, for showing an in-progress chess game.

It's meant to be connected either like: PC -> Distributor -> TV Interface, or
like: chesscomputer -> Distributor -> TV Interface. But the distributor is only
for converting to/from RS-232/TTL voltage, so MAME can ignore it.

It expects a baud rate of 9600, 8 data bits, 1 stop bit, no parity.

Hardware notes:
- PCB label: 100122 REV C
- Hitachi HD63A03RP @ 4.9152MHz
- 8KB ROM (27C64A), 4KB RAM (2*UM6116-3)
- TMT TM6310 CGA chip, 14.318MHz XTAL, 8KB CG ROM (27C64A, only 2KB used)

TODO:
- dump/add English version
- Currently, MAME only has CGA emulation on ISA cards, but this thing is not an
  ISA card, nor compatible with IBM CGA standard. So if MAME ever adds a more
  generic CGA device, use that.

*******************************************************************************/

#include "emu.h"
#include "nss_tvinterface.h"

#include "cpu/m6800/m6801.h"
#include "video/cgapal.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

class nss_tvinterface_device : public device_t, public device_rs232_port_interface
{
public:
	nss_tvinterface_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void input_txd(int state) override { m_rx_state = state ? 1 : 0; }

private:
	required_device<hd6303r_cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_cg_rom;

	u8 m_rx_state = 0;

	void palette(palette_device &palette) const;
	MC6845_UPDATE_ROW(crtc_update_row);

	u8 cga_sync_r();
	void cga_mode_control_w(u8 data);
	void cga_color_select_w(u8 data);
	void unknown_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;

	u8 p2_r();
	void p2_w(u8 data);
};

nss_tvinterface_device::nss_tvinterface_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NSS_TVINTERFACE, tag, owner, clock),
	device_rs232_port_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_crtc(*this, "crtc"),
	m_palette(*this, "palette"),
	m_screen(*this, "screen"),
	m_vram(*this, "vram"),
	m_cg_rom(*this, "cg_rom")
{ }

void nss_tvinterface_device::device_start()
{
	save_item(NAME(m_rx_state));
}


//-------------------------------------------------
//  CGA video
//-------------------------------------------------

void nss_tvinterface_device::palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
		palette.set_pen_color(i, cga_palette[i][0], cga_palette[i][1], cga_palette[i][2]);
}

MC6845_UPDATE_ROW(nss_tvinterface_device::crtc_update_row)
{
	const pen_t *pen = m_palette->pens();

	for (int x = 0; x < x_count; x++)
	{
		u16 offset = (ma + x) << 1 & 0x7ff;

		u8 chr = m_vram[offset];
		u8 data = m_cg_rom[((ra << 8 | chr) & 0x7ff) | 0x1000];

		u8 attr = m_vram[offset | 1];
		u8 fg = attr & 0xf;
		u8 bg = attr >> 4 & 0xf;

		for (int xi = 0; xi < 8; xi++)
			bitmap.pix(y, x << 3 | xi) = BIT(data, xi ^ 7) ? pen[fg] : pen[bg];
	}
}

u8 nss_tvinterface_device::cga_sync_r()
{
	return (m_crtc->vsync_r() * 9) | m_crtc->hsync_r();
}

void nss_tvinterface_device::cga_mode_control_w(u8 data)
{
	// only uses textmode
}

void nss_tvinterface_device::cga_color_select_w(u8 data)
{
	// does not use 2bpp palette
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

void nss_tvinterface_device::unknown_w(u8 data)
{
}

u8 nss_tvinterface_device::p2_r()
{
	// P23: serial in
	return m_rx_state << 3;
}

void nss_tvinterface_device::p2_w(u8 data)
{
	// P24: serial out
	output_rxd(BIT(~data, 4));
}

void nss_tvinterface_device::main_map(address_map &map)
{
	map(0x8000, 0x87ff).ram().share(m_vram);

	map(0xa3bd, 0xa3bd).portr("IN.0");
	map(0xa3be, 0xa3be).w(FUNC(nss_tvinterface_device::unknown_w));

	map(0xa3d0, 0xa3d0).mirror(0x0006).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xa3d1, 0xa3d1).mirror(0x0006).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xa3d8, 0xa3d8).w(FUNC(nss_tvinterface_device::cga_mode_control_w));
	map(0xa3d9, 0xa3d9).w(FUNC(nss_tvinterface_device::cga_color_select_w));
	map(0xa3da, 0xa3da).r(FUNC(nss_tvinterface_device::cga_sync_r));

	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xffff).rom();
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( nss_tvinterface )
	PORT_START("IN.0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Select")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Change")
INPUT_PORTS_END

ioport_constructor nss_tvinterface_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nss_tvinterface);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nss_tvinterface_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	HD6303R(config, m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &nss_tvinterface_device::main_map);
	m_maincpu->in_p2_cb().set(FUNC(nss_tvinterface_device::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(nss_tvinterface_device::p2_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL/2, 472, 0, 320, 260, 0, 200);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 14.318181_MHz_XTAL/16);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(nss_tvinterface_device::crtc_update_row));

	PALETTE(config, m_palette, FUNC(nss_tvinterface_device::palette), 16);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(nss_tvinterface)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("tvgr_011091", 0xe000, 0x2000, CRC(0ff64cec) SHA1(d35881111cdf8e85a843e3b085447d07244d8d70) )

	ROM_REGION( 0x2000, "cg_rom", 0 )
	ROM_LOAD("cg_930", 0x0000, 0x2000, CRC(98287b9d) SHA1(b20d7856e4739bafcd66434adf9881824f8a611b) )
ROM_END

const tiny_rom_entry *nss_tvinterface_device::device_rom_region() const
{
	return ROM_NAME(nss_tvinterface);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(NSS_TVINTERFACE, device_rs232_port_interface, nss_tvinterface_device, "nss_tvinterface", "Novag Super System TV Interface")
