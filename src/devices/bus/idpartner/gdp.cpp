// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    Iskra Delta GDP Card

**********************************************************************/

#include "emu.h"
#include "gdp.h"

#include "machine/timer.h"
#include "machine/z80pio.h"
#include "video/ef9365.h"
#include "video/scn2674.h"

#include "screen.h"

namespace {

class idpartner_gdp_device :
	public device_t,
	public bus::idpartner::device_exp_card_interface
{
public:
	// construction/destruction
	idpartner_gdp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void char_w(u8 data) { m_avdc->buffer_w(data); }
	void attr_w(u8 data) { m_avdc->attr_buffer_w(data); }
	void scroll_w(u8 data) { m_scroll = data; }
	u8 common_r() { m_common ^= 0x10; return m_common; }
	void porta_w(u8 data);
	void portb_w(u8 data);
	void vram_w(offs_t offset, u8 data);
	u8 vram_r(offs_t offset);
	void msl_w(u8 data);
	void gdc_map(address_map &map) ATTR_COLD;
	void int_w(int state) { m_bus->int_w(state); }
	void nmi_w(int state) { m_bus->nmi_w(state); }

	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline) { m_gdc->update_scanline(param); }

	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	required_device<ef9365_device> m_gdc;
	memory_share_creator<u8> m_vram;
	required_device<scn2674_device> m_avdc;
	required_device<z80pio_device> m_pio;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;

	u8 m_scroll;
	u8 m_common;
};


idpartner_gdp_device::idpartner_gdp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IDPARTNER_GDP, tag, owner, clock)
	, bus::idpartner::device_exp_card_interface(mconfig, *this)
	, m_gdc(*this, "gdc")
	, m_vram(*this, "vram", 0x20000, ENDIANNESS_LITTLE)
	, m_avdc(*this, "avdc")
	, m_pio(*this, "pio")
	, m_palette(*this, "palette")
	, m_p_chargen(*this, "chargen")
	, m_scroll(0)
	, m_common(0)
{
}

/*----------------------------------
  device_t implementation
----------------------------------*/

void idpartner_gdp_device::device_start()
{
	save_item(NAME(m_scroll));
}

void idpartner_gdp_device::device_reset()
{
	m_bus->io().install_read_handler (0x20, 0x2f, emu::rw_delegate(m_gdc, FUNC(ef9365_device::data_r))); // Thomson GDP EF9367
	m_bus->io().install_write_handler(0x20, 0x2f, emu::rw_delegate(m_gdc, FUNC(ef9365_device::data_w))); // Thomson GDP EF9367
	m_bus->io().install_read_handler (0x30, 0x33, emu::rw_delegate(m_pio, FUNC(z80pio_device::read_alt))); // PIO - Graphics
	m_bus->io().install_write_handler(0x30, 0x33, emu::rw_delegate(m_pio, FUNC(z80pio_device::write_alt))); // PIO - Graphics
	m_bus->io().install_write_handler(0x34, 0x34, emu::rw_delegate(*this, FUNC(idpartner_gdp_device::char_w))); // char reg
	m_bus->io().install_write_handler(0x35, 0x35, emu::rw_delegate(*this, FUNC(idpartner_gdp_device::attr_w))); // attr reg
	m_bus->io().install_write_handler(0x36, 0x36, emu::rw_delegate(*this, FUNC(idpartner_gdp_device::scroll_w))); // scroll reg/common input
	m_bus->io().install_read_handler (0x36, 0x36, emu::rw_delegate(*this, FUNC(idpartner_gdp_device::common_r))); // common input
	m_bus->io().install_read_handler (0x38, 0x3f, emu::rw_delegate(m_avdc, FUNC(scn2674_device::read)));  // AVDC SCN2674
	m_bus->io().install_write_handler(0x38, 0x3f, emu::rw_delegate(m_avdc, FUNC(scn2674_device::write))); // AVDC SCN2674
}

void idpartner_gdp_device::porta_w(u8 data)
{
	//printf("porta_w:%02x\n",data);
}

void idpartner_gdp_device::portb_w(u8 data)
{
	//printf("portb_w:%02x\n",data);
}

void idpartner_gdp_device::msl_w(u8 data)
{
	//printf("msl_w:%02x\n",data);
}

void idpartner_gdp_device::vram_w(offs_t offset, u8 data)
{
	u16 addr = (m_scroll << 7) - offset;
	m_vram[addr] = data;
}

u8 idpartner_gdp_device::vram_r(offs_t offset)
{
	u16 addr = (m_scroll << 7) - offset;
	return m_vram[addr];
}

// 16 * 4164 DRAM
void idpartner_gdp_device::gdc_map(address_map &map)
{
	map(0x0000, 0x1ffff).rw(FUNC(idpartner_gdp_device::vram_r), FUNC(idpartner_gdp_device::vram_w));
}


// 2 * 6116 SRAM for chars
void idpartner_gdp_device::char_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("charram");
	map(0x1000, 0x2fff).ram(); // TODO: Check in schematics
}

// 2 * 6116 SRAM for attributes
void idpartner_gdp_device::attr_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("attrmap");
	map(0x1000, 0x2fff).ram(); // TODO: Check in schematics
}

/* F4 Character Displayer */
static const gfx_layout gdp_charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	256,                    /* 128 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8  },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_partner )
	GFXDECODE_ENTRY( "chargen", 0x0000, gdp_charlayout, 0, 1 )
GFXDECODE_END

// one 2732 EPROM for chargen
ROM_START(gdp_char_rom)
	ROM_REGION( 0x1000, "chargen",0 )
	ROM_LOAD( "gdp_chgi.ic46",  0x0000, 0x1000, CRC(f4aaf0dd) SHA1(7c7c2d855fdfb90c86ed47c1b33c354c922ed4d0))
ROM_END

const tiny_rom_entry *idpartner_gdp_device::device_rom_region() const
{
	return ROM_NAME( gdp_char_rom );
}

SCN2674_DRAW_CHARACTER_MEMBER( idpartner_gdp_device::draw_character )
{
	uint16_t data = m_p_chargen[charcode << 4 | linecount];
	const pen_t *const pen = m_palette->pens();

	if (cursor)
		data = ~data;

	// draw 9 pixels of the character
	for (int i = 0; i < 9; i++)
		bitmap.pix(y, x + i) = BIT(data, i) ? pen[0] : pen[1];
}

void idpartner_gdp_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(m_gdc, FUNC(ef9365_device::screen_update));
	screen.set_size(1024, 512);
	screen.set_visarea(0, 1024-1, 0, 512-1);
	screen.set_refresh_hz(50);
	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);


	screen_device &screen2(SCREEN(config, "screen2", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen2.set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));
	screen2.set_size(1024, 512);
	screen2.set_visarea(0, 1024-1, 0, 512-1);
	screen2.set_refresh_hz(50);

	GFXDECODE(config, "gfxdecode", "palette", gfx_partner);

	EF9365(config, m_gdc, XTAL(24'000'000) / 16); // EF9367 1.5 MHz
	m_gdc->set_screen("screen");
	m_gdc->set_addrmap(0, &idpartner_gdp_device::gdc_map);
	m_gdc->set_palette_tag("palette");
	m_gdc->set_nb_bitplanes(1);
	m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_1024x512);
	m_gdc->write_msl().set(FUNC(idpartner_gdp_device::msl_w));

	TIMER(config, "scanline").configure_scanline(FUNC(idpartner_gdp_device::scanline), "screen", 0, 1);

	SCN2674(config, m_avdc, XTAL(24'000'000) / 16); // SCN2674B
	m_avdc->set_screen("screen2");
	m_avdc->set_character_width(12);
	m_avdc->set_addrmap(0, &idpartner_gdp_device::char_map);
	m_avdc->set_addrmap(1, &idpartner_gdp_device::attr_map);
	m_avdc->set_display_callback(FUNC(idpartner_gdp_device::draw_character));
	 //m_avdc->intr_callback().set(FUNC(idpartner_gdp_device::nmi_w));

	Z80PIO(config, m_pio, XTAL(8'000'000) / 2);
	m_pio->out_int_callback().set(FUNC(idpartner_gdp_device::int_w));
	m_pio->out_pa_callback().set(FUNC(idpartner_gdp_device::porta_w));
	m_pio->out_pb_callback().set(FUNC(idpartner_gdp_device::portb_w));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(IDPARTNER_GDP, bus::idpartner::device_exp_card_interface, idpartner_gdp_device, "partner_gdp", "Iskra Delta Partner GDP")
