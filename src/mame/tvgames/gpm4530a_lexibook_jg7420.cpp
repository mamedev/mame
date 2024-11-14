// license:BSD-3-Clause
// copyright-holders:David Haywood

// uses a GPM4530A (see https://www.generalplus.com/GPM4530A-j2tLs-1LVbPHkLN4921SVpnSNproduct_detail )

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/spi_sdcard.h"

#include "screen.h"
#include "speaker.h"

namespace {

class gpm4530a_lexibook_state : public driver_device
{
public:
	gpm4530a_lexibook_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_sdcard(*this, "sdcard")
	{ }

	void gpm4530a_lexibook(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void arm_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<spi_sdcard_device> m_sdcard;

	uint32_t screen_update_gpm4530a_lexibook(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:

};

void gpm4530a_lexibook_state::arm_map(address_map &map)
{
}

uint32_t gpm4530a_lexibook_state::screen_update_gpm4530a_lexibook(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gpm4530a_lexibook_state::machine_start()
{
}

void gpm4530a_lexibook_state::machine_reset()
{
}

static INPUT_PORTS_START( gpm4530a_lexibook )
INPUT_PORTS_END


void gpm4530a_lexibook_state::gpm4530a_lexibook(machine_config &config)
{
	ARM9(config, m_maincpu, 192'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gpm4530a_lexibook_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(gpm4530a_lexibook_state::screen_update_gpm4530a_lexibook));

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START( lx_jg7420 )
	ROM_REGION( 0x10000, "boot", ROMREGION_ERASEFF )
	ROM_LOAD( "bootrom.bin", 0x00000, 0x10000, NO_DUMP ) // unknown size/capacity/type

	DISK_REGION( "sdcard" ) // 4GB SD Card
	DISK_IMAGE( "jg7420", 0, SHA1(214a1686c7eefdb4cb5d723e98957600c8cb138d) )
ROM_END

} // anonymous namespace

// JG7420_24 on sticker
CONS( 201?, lx_jg7420,    0,       0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init, "Lexibook", "Lexibook JG7420 200-in-1", MACHINE_IS_SKELETON )
