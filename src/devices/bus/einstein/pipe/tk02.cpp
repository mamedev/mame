// license: GPL-2.0+
// copyright-holders: Dirk Best, Phill Harvey-Smith
/***************************************************************************

    TK02 80 Column Monochrome Unit

***************************************************************************/

#include "emu.h"
#include "tk02.h"
#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TK02_80COL, tk02_device, "tk02", "TK02 80 Column Monochrome Unit")

//-------------------------------------------------
//  device_address_map
//-------------------------------------------------

void tk02_device::map(address_map &map)
{
//  AM_RANGE(0x00, 0x07) AM_SELECT(0xff00) AM_READWRITE(ram_r, ram_w) // no AM_SELECT (or AM_MASK) support here
	map(0x08, 0x08).mirror(0xff00).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x09, 0x09).mirror(0xff00).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0x0c, 0x0c).mirror(0xff00).r(FUNC(tk02_device::status_r));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( tk02 )
	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("tk02-v100.bin", 0x0000, 0x2000, CRC(ad3c4346) SHA1(cd57e630371b4d0314e3f15693753fb195c7257d))
ROM_END

const tiny_rom_entry *tk02_device::device_rom_region() const
{
	return ROM_NAME( tk02 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( tk02_links )
	PORT_START("M001")
	PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "M001:1")

	PORT_START("M002")
	PORT_DIPNAME(0x01, 0x00, "TV Standard")      PORT_DIPLOCATION("M002:1")
	PORT_DIPSETTING(0x00, "625 lines/50 Hz")
	PORT_DIPSETTING(0x01, "525 lines/60 Hz")

	PORT_START("M003")
	PORT_DIPNAME(0x01, 0x00, "Startup Mode")     PORT_DIPLOCATION("M003:1")
	PORT_DIPSETTING(0x00, "Normal")
	PORT_DIPSETTING(0x01, "Automatic 80 Column")

	PORT_START("M004")
	PORT_DIPNAME(0x01, 0x00, "Character Set")    PORT_DIPLOCATION("M004:1")
	PORT_DIPSETTING(0x00, "Modified")
	PORT_DIPSETTING(0x01, "Normal")
INPUT_PORTS_END

ioport_constructor tk02_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tk02_links );
}

//-------------------------------------------------
//  gfx_layout - only for the char viewer
//-------------------------------------------------

static const gfx_layout tk02_charlayout =
{
	8, 10,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0x800*8, 0x801*8 },
	8*8
};

static GFXDECODE_START( gfx_tk02 )
	GFXDECODE_ENTRY("gfx", 0x0000, tk02_charlayout, 0, 1)
	GFXDECODE_ENTRY("gfx", 0x1000, tk02_charlayout, 0, 1)
GFXDECODE_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tk02_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "mono", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(XTAL(8'000'000) * 2, 1024, 0, 640, 312, 0, 250);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", gfx_tk02);

	MC6845(config, m_crtc, XTAL(8'000'000) / 4);
	m_crtc->set_screen("mono");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(tk02_device::crtc_update_row), this);
	m_crtc->out_de_callback().set(FUNC(tk02_device::de_w));

	TATUNG_PIPE(config, m_pipe, DERIVED_CLOCK(1, 1), tatung_pipe_cards, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tk02_device - constructor
//-------------------------------------------------

tk02_device::tk02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TK02_80COL, tag, owner, clock),
	device_tatung_pipe_interface(mconfig, *this),
	m_pipe(*this, "pipe"),
	m_crtc(*this, "crtc"),
	m_palette(*this, "palette"),
	m_gfx(*this, "gfx"),
	m_links(*this, "M00%u", 1),
	m_de(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tk02_device::device_start()
{
	// setup ram
	m_ram = std::make_unique<uint8_t[]>(0x800);
	memset(m_ram.get(), 0xff, 0x800);

	// register for save states
	save_pointer(NAME(m_ram), 0x800);
	save_item(NAME(m_de));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tk02_device::device_reset()
{
	io_space().install_device(0x40, 0x4f, *this, &tk02_device::map);
	io_space().install_readwrite_handler(0x40, 0x47, 0, 0, 0xff00, read8_delegate(FUNC(tk02_device::ram_r), this), write8_delegate(FUNC(tk02_device::ram_w), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW( tk02_device::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < x_count; i++)
	{
		uint8_t code = m_ram[(ma + i) & 0x07ff];

		// 12--------------  link M004
		// --11------------  ra3
		// ----109876543---  data from ram
		// -------------210  ra0..2
		uint8_t data = m_gfx->as_u8((m_links[3]->read() << 12) | ((ra & 0x08) << 8) | (code << 3) | (ra & 0x07));

		if (i == cursor_x)
			data ^= 0xff;

		bitmap.pix32(y, i * 8 + 0) = pen[BIT(data, 7)];
		bitmap.pix32(y, i * 8 + 1) = pen[BIT(data, 6)];
		bitmap.pix32(y, i * 8 + 2) = pen[BIT(data, 5)];
		bitmap.pix32(y, i * 8 + 3) = pen[BIT(data, 4)];
		bitmap.pix32(y, i * 8 + 4) = pen[BIT(data, 3)];
		bitmap.pix32(y, i * 8 + 5) = pen[BIT(data, 2)];
		bitmap.pix32(y, i * 8 + 6) = pen[BIT(data, 1)];
		bitmap.pix32(y, i * 8 + 7) = pen[BIT(data, 0)];
	}
}

WRITE_LINE_MEMBER( tk02_device::de_w )
{
	m_de = state;
}

// lower 3 bits of address define a 256-byte "row"
// upper 8 bits define the offset in the row
READ8_MEMBER( tk02_device::ram_r )
{
	return m_ram[((offset & 0x07) << 8) | ((offset >> 8) & 0xff)];
}

WRITE8_MEMBER( tk02_device::ram_w )
{
	m_ram[((offset & 0x07) << 8) | ((offset >> 8) & 0xff)] = data;
}

READ8_MEMBER( tk02_device::status_r )
{
	// 7654----  unused
	// ----3---  link M001
	// -----2--  link M002
	// ------1-  link M003
	// -------0  mc6845 display enabled

	uint8_t data = 0xf0;

	data |= m_links[0]->read() << 3;
	data |= m_links[1]->read() << 2;
	data |= m_links[2]->read() << 1;
	data |= m_de << 0;

	return data;
}
