// license:BSD-3-Clause
// copyright-holders:R. Belmont, Sven Schnelle
/***************************************************************************

  HP98544 high-resolution monochrome board

  VRAM at 0x200000, ROM and registers at 0x560000

***************************************************************************/

#include "emu.h"
#include "hp98544.h"

#include "video/topcat.h"

#include "screen.h"


#define HP98544_SCREEN_NAME   "98544_screen"
#define HP98544_ROM_REGION    "98544_rom"

namespace {

class dio16_98544_device :
	public device_t,
	public bus::hp_dio::device_dio16_card_interface,
	public device_memory_interface
{
public:
	// construction/destruction
	dio16_98544_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t rom_r(offs_t offset);
	void rom_w(offs_t offset, uint16_t data);

	required_device<topcat_device> m_topcat;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
 protected:
	dio16_98544_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
private:

	void vblank_w(int state);
	void int_w(int state);

	static constexpr int m_v_pix = 768;
	static constexpr int m_h_pix = 1024;

	const address_space_config m_space_config;
	void map(address_map &map) ATTR_COLD;
	required_region_ptr<uint8_t> m_rom;
	required_shared_ptr<uint8_t> m_vram;

	uint8_t m_intreg;
};

//*************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

ROM_START( hp98544 )
	ROM_REGION( 0x2000, HP98544_ROM_REGION, 0 )
	ROM_LOAD( "98544_1818-1999.bin", 0x000000, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END
//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dio16_98544_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, HP98544_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(dio16_98544_device::screen_update));
	screen.screen_vblank().set(FUNC(dio16_98544_device::vblank_w));
	screen.set_raw(XTAL(64'108'800), 1408, 0, 1024, 795, 0, 768);

	topcat_device &topcat0(TOPCAT(config, "topcat", XTAL(64108800)));
	topcat0.set_fb_width(1024);
	topcat0.set_fb_height(768);
	topcat0.set_planemask(1);
	topcat0.irq_out_cb().set(FUNC(dio16_98544_device::int_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dio16_98544_device::device_rom_region() const
{
	return ROM_NAME( hp98544 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio16_98544_device - constructor
//-------------------------------------------------

dio16_98544_device::dio16_98544_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98544_device(mconfig, HPDIO_98544, tag, owner, clock)
{
}

dio16_98544_device::dio16_98544_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_topcat(*this, "topcat"),
	m_space_config("vram", ENDIANNESS_BIG, 8, 20, 0, address_map_constructor(FUNC(dio16_98544_device::map), this)),
	m_rom(*this, HP98544_ROM_REGION),
	m_vram(*this, "vram"),
	m_intreg(0)
{
}

void dio16_98544_device::map(address_map& map)
{
	map(0, 0xfffff).ram().share("vram");
}

device_memory_interface::space_config_vector dio16_98544_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_space_config) };
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_98544_device::device_start()
{
	save_item(NAME(m_intreg));
	dio().install_memory(
			0x200000, 0x2fffff,
			read16s_delegate(*m_topcat, FUNC(topcat_device::vram_r)),
			write16s_delegate(*m_topcat, FUNC(topcat_device::vram_w)));
	dio().install_memory(
			0x560000, 0x563fff,
			read16sm_delegate(*this, FUNC(dio16_98544_device::rom_r)),
			write16sm_delegate(*this, FUNC(dio16_98544_device::rom_w)));
	dio().install_memory(
			0x564000, 0x567fff,
			read16_delegate(*m_topcat, FUNC(topcat_device::ctrl_r)),
			write16_delegate(*m_topcat, FUNC(topcat_device::ctrl_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_98544_device::device_reset()
{
	m_intreg = 0;
}

uint16_t dio16_98544_device::rom_r(offs_t offset)
{
	if (offset == 1)
		return m_intreg;

	return 0xff00 | m_rom[offset];
}

// the video chip registers live here, so these writes are valid
void dio16_98544_device::rom_w(offs_t offset, uint16_t data)
{
	if (offset == 1) {
		m_intreg = data;
	}
}

void dio16_98544_device::vblank_w(int state)
{
	m_topcat->vblank_w(state);
}

void dio16_98544_device::int_w(int state)
{
	int line = (m_intreg >> 3) & 7;

	if (state)
		m_intreg |= 0x40;
	else
		m_intreg &= ~0x40;
	if (!(m_intreg & 0x80))
		state = false;

	irq1_out(line == 1 && state);
	irq2_out(line == 2 && state);
	irq3_out(line == 3 && state);
	irq4_out(line == 4 && state);
	irq5_out(line == 5 && state);
	irq6_out(line == 6 && state);
	irq7_out(line == 7 && state);

}

uint32_t dio16_98544_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_topcat->has_changed())
		return UPDATE_HAS_NOT_CHANGED;

	for (int y = 0; y < m_v_pix; y++) {
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < m_h_pix; x++) {
			uint8_t tmp = m_vram[y * m_h_pix + x];
			*scanline++ = tmp ? rgb_t(255,255,255) : rgb_t(0, 0, 0);
		}
	}

	int startx, starty, endx, endy;
	m_topcat->get_cursor_pos(startx, starty, endx, endy);
	bitmap.fill(rgb_t(255, 255, 255), rectangle(startx, endx, starty, endy));

	return 0;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98544, bus::hp_dio::device_dio16_card_interface, dio16_98544_device, "dio98544", "HP98544 high-res monochrome DIO video card")
