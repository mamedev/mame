// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

   Leapfrog IQuest

   has LCD display

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/bankdev.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"
#include "screen.h"

class leapfrog_iquest_state : public driver_device
{
public:
	leapfrog_iquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_rombank(*this, "rombank")
		, m_cart_region(nullptr)
	{ }

	void leapfrog_iquest(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void prog_map(address_map &map);
	void ext_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;

	void rom_map(address_map &map);

	required_device<address_map_bank_device> m_rombank;
	memory_region *m_cart_region;

	uint8_t m_lowerbank[2];
	uint8_t m_upperbank[2];

	uint8_t lowerbank_r(offs_t offset);
	uint8_t upperbank_r(offs_t offset);
	void lowerbank_w(offs_t offset, uint8_t data);
	void upperbank_w(offs_t offset, uint8_t data);

	uint8_t m_iobank[2];

	uint8_t iobank_r(offs_t offset);
	void iobank_w(offs_t offset, uint8_t data);


	uint8_t lowerwindow_r(offs_t offset);
	uint8_t upperwindow_r(offs_t offset);

	uint8_t iowindow_r(offs_t offset);
	void iowindow_w(offs_t offset, uint8_t data);

	uint8_t unk_ff80_r();
	void unk_ff80_w(uint8_t data);
	uint8_t m_ff80;
};



void leapfrog_iquest_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void leapfrog_iquest_state::machine_reset()
{
	m_lowerbank[0] = m_lowerbank[1] = 0x00;
	m_upperbank[0] = m_upperbank[1] = 0x00;
	m_iobank[0] = m_iobank[1] = 0x00;

	m_rombank->set_bank(0);
	m_ff80 = 0x00;
}


uint8_t leapfrog_iquest_state::lowerbank_r(offs_t offset)
{
	return m_lowerbank[offset];
}

uint8_t leapfrog_iquest_state::upperbank_r(offs_t offset)
{
	return m_upperbank[offset];
}


void leapfrog_iquest_state::lowerbank_w(offs_t offset, uint8_t data)
{
	m_lowerbank[offset] = data;
}

void leapfrog_iquest_state::upperbank_w(offs_t offset, uint8_t data)
{
	m_upperbank[offset] = data;
}

uint8_t leapfrog_iquest_state::iobank_r(offs_t offset)
{
	return m_iobank[offset];
}


void leapfrog_iquest_state::iobank_w(offs_t offset, uint8_t data)
{
	//printf("iobank_w %d, %02x\n", offset, data);
	m_iobank[offset] = data;
}


void leapfrog_iquest_state::rom_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().region("maincpu", 0);
	map(0x00400000, 0x007fffff).ram();
}

uint8_t leapfrog_iquest_state::lowerwindow_r(offs_t offset)
{
	uint32_t bank = ((m_lowerbank[0] << 8) | (m_lowerbank[1])) * 0x8000;
	return m_rombank->read8(bank + offset);
}

uint8_t leapfrog_iquest_state::upperwindow_r(offs_t offset)
{
	uint32_t bank = ((m_upperbank[0] << 8) | (m_upperbank[1])) * 0x8000;
	return m_rombank->read8(bank + offset);
}

uint8_t leapfrog_iquest_state::iowindow_r(offs_t offset)
{
	uint32_t bank = ((m_iobank[0] << 8) | (m_iobank[1])) * 0x8000;
	return m_rombank->read8(bank + offset);
}

void leapfrog_iquest_state::iowindow_w(offs_t offset, uint8_t data)
{
	uint32_t bank = ((m_iobank[0] << 8) | (m_iobank[1])) * 0x8000;
	m_rombank->write8(bank + offset, data);
}


void leapfrog_iquest_state::prog_map(address_map &map)
{
	map(0x0000, 0x7fff).r(FUNC(leapfrog_iquest_state::lowerwindow_r));
	map(0x8000, 0xffff).r(FUNC(leapfrog_iquest_state::upperwindow_r));
}

uint8_t leapfrog_iquest_state::unk_ff80_r()
{
	return m_ff80;
}

void leapfrog_iquest_state::unk_ff80_w(uint8_t data)
{
	m_ff80 = data;
}



void leapfrog_iquest_state::ext_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(leapfrog_iquest_state::iowindow_r), FUNC(leapfrog_iquest_state::iowindow_w)); // assume this accesses the same space, with different bank register

	map(0xc260, 0xc52f).ram(); // = clears 0x2d0 bytes (90*64 / 8) display buffer?
	map(0xc530, 0xc7ff).ram(); // = clears 0x2d0 bytes (90*64 / 8) display buffer?

	map(0xfc06, 0xfc07).rw(FUNC(leapfrog_iquest_state::lowerbank_r), FUNC(leapfrog_iquest_state::lowerbank_w));
	map(0xfc08, 0xfc09).rw(FUNC(leapfrog_iquest_state::upperbank_r), FUNC(leapfrog_iquest_state::upperbank_w));

	map(0xfc0a, 0xfc0b).rw(FUNC(leapfrog_iquest_state::iobank_r), FUNC(leapfrog_iquest_state::iobank_w));


	map(0xff80, 0xff80).rw(FUNC(leapfrog_iquest_state::unk_ff80_r), FUNC(leapfrog_iquest_state::unk_ff80_w));
}

DEVICE_IMAGE_LOAD_MEMBER(leapfrog_iquest_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

static INPUT_PORTS_START( leapfrog_iquest )
INPUT_PORTS_END

uint32_t leapfrog_iquest_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void leapfrog_iquest_state::leapfrog_iquest(machine_config &config)
{
	I8032(config, m_maincpu, 96000000/10); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &leapfrog_iquest_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &leapfrog_iquest_state::ext_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(90, 64);
	m_screen->set_visarea(0, 90-1, 0, 64-1);
	m_screen->set_screen_update(FUNC(leapfrog_iquest_state::screen_update));
	//m_screen->screen_vblank().set(FUNC(leapfrog_iquest_state::screen_vblank));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_iquest_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(leapfrog_iquest_state::cart_load));

	ADDRESS_MAP_BANK(config, "rombank").set_map(&leapfrog_iquest_state::rom_map).set_options(ENDIANNESS_LITTLE, 8, 31, 0x80000000);

	SOFTWARE_LIST(config, "cart_list").set_original("leapfrog_iquest_cart");
}

ROM_START( iquest )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "iquest.bin", 0x000000, 0x400000, CRC(f785dc4e) SHA1(ec002c18df536737334fe6b7db0e7342bad7b66b))
ROM_END

//    year, name,        parent,    compat, machine,            input,            class,                  init,       company,    fullname,                         flags
// it is unknown if the versions of IQuest without 4.0 on the case have different system ROM
CONS( 200?, iquest,      0,         0,      leapfrog_iquest,    leapfrog_iquest,  leapfrog_iquest_state,  empty_init, "LeapFrog", "IQuest 4.0 (US)",                    MACHINE_IS_SKELETON )
