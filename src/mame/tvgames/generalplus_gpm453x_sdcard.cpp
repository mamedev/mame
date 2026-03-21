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

	void gpm4530a_lexibook(machine_config &config) ATTR_COLD;

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

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( lx_jg7420 )
	ROM_REGION( 0x10000, "boot", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.bin", 0x00000, 0x10000, NO_DUMP ) // unknown size/capacity/type (internal?)

	ROM_REGION( 0x100000, "spi", ROMREGION_ERASEFF ) // probably undumped for this unit
	ROM_LOAD( "spi_boot", 0x00000, 0x100000, NO_DUMP )

	DISK_REGION( "sdcard" ) // 4GB SD Card
	DISK_IMAGE( "jg7420", 0, SHA1(214a1686c7eefdb4cb5d723e98957600c8cb138d) )
ROM_END

ROM_START( lx_jg7440 ) // CPU details erased, assuming the same as JG7420 as there's ARM Thumb code in here
	ROM_REGION( 0x10000, "boot", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.bin", 0x00000, 0x10000, NO_DUMP ) // unknown size/capacity/type (internal?)

	ROM_REGION( 0x100000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD( "fm25q08.u6", 0x00000, 0x100000, CRC(6103d8ff) SHA1(4c8f06c200a4dc24bb321e4728ffaf61e51443a6) )

	DISK_REGION( "sdcard" ) // 4GB SD Card
	DISK_IMAGE( "jg7440", 0, SHA1(f29b9c981de6948a8d7b92ec5fd3fbd91e2c8bf7) )
ROM_END

ROM_START( rizstals )
	ROM_REGION( 0x10000, "boot", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.bin", 0x00000, 0x10000, NO_DUMP ) // unknown size/capacity/type (internal?)

	ROM_REGION( 0x100000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD( "mx25v8035f.u5", 0x0000, 0x100000, CRC(1ba0c7b8) SHA1(d3f4fdabf07c1d8bbd73da54280e3fab006a72b6) )

	DISK_REGION( "sdcard" ) // 8GB SD Card
	DISK_IMAGE( "rizstals", 0, SHA1(79462dd4a632d9b9710581ed170a696c059e8a2d) )
ROM_END

} // anonymous namespace

// JG7420_24 on sticker
CONS( 201?, lx_jg7420,    0,       0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init, "Lexibook", "Lexibook JG7420 - TV Game Console (200 Games, 32-bits)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 201?, lx_jg7440,    0,       0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init, "Lexibook", "Lexibook JG7440 - TV Game Console (250 Games, 32-bits)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2021, rizstals,     0,       0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init, "Takara Tomy", "RizSta Live Studio", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
