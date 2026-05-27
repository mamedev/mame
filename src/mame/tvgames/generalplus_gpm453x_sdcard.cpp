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

	ROM_REGION( 0x100000, "spi", ROMREGION_ERASEFF )
	ROM_LOAD( "25d80.u6", 0x00000, 0x100000, CRC(6916409d) SHA1(80dbdc568547cc347a8aca0d530ac25e20b08e34) )

	ROM_REGION( 0x80000, "spi2", ROMREGION_ERASEFF ) // does it boot from this if no card is present?
	ROM_LOAD( "fm25q04.u14", 0x00000, 0x80000, CRC(f6410672) SHA1(3ccce8e5ef1bfd08c90d90d2efdb646d2af9fba7) )

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


ROM_START( intrtvg )
	ROM_REGION( 0x100000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "25q08.u6", 0x0000, 0x100000, CRC(5aa91972) SHA1(296108e8683063c16951ff326e6ff3d63d9ed5b8) )

	DISK_REGION( "sdcard" ) // 4GB SD Card
	DISK_IMAGE( "interactivetv", 0, SHA1(7061e28c4560b763bda1157036b79c726387e430) )
ROM_END

ROM_START( bodygun )
	ROM_REGION( 0x100000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "25q08.u6", 0x0000, 0x100000, CRC(5aa91972) SHA1(296108e8683063c16951ff326e6ff3d63d9ed5b8) )

	DISK_REGION( "sdcard" ) // 4GB SD Card
	DISK_IMAGE( "bodygun", 0, SHA1(3e41a2ba9b86fb6b155c1c82a7612458c3555a64) )
ROM_END

ROM_START( kaximond )
	ROM_REGION( 0x100000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "tj25q08.u6", 0x0000, 0x100000, CRC(83d07dff) SHA1(4d6a3a823686a0955c904cc5f8c3e0e95ec53046) )

	DISK_REGION( "sdcard" ) // 32GB SD Card
	DISK_IMAGE( "kaximon dance", 0, SHA1(dd041586f8815312e95a4e678d1e292c3407abea) )
ROM_END

ROM_START( arb605 )
	ROM_REGION( 0x100000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "25q80.u6", 0x0000, 0x100000, CRC(ba2cdacd) SHA1(d47829ee5310140665146262a44e0ba91942f25c) )

	DISK_REGION( "sdcard" ) // 4GB SD Card
	DISK_IMAGE( "ar_game_console_b605", 0, SHA1(433d354529e262de9d833a7a423a37915ba3362c) )
ROM_END

ROM_START( ardancem )
	ROM_REGION( 0x100000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "25q08.u6", 0x0000, 0x100000, CRC(ba2cdacd) SHA1(d47829ee5310140665146262a44e0ba91942f25c) )

	DISK_REGION( "sdcard" ) // 16GB SD Card
	DISK_IMAGE( "ardancemat", 0, SHA1(df8cb065f5ce0ca863b205549ecc4c27647f9954) )
ROM_END

} // anonymous namespace

// JG7420_24 on sticker
CONS( 201?, lx_jg7420,    0,       0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init, "Lexibook", "Lexibook JG7420 - TV Game Console (200 Games, 32-bits)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 201?, lx_jg7440,    0,       0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init, "Lexibook", "Lexibook JG7440 - TV Game Console (250 Games, 32-bits)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2021, rizstals,     0,       0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init, "Takara Tomy", "RizSta Live Studio", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// company is called 深圳市飞讯互动科技有限公司
// surface details erased on SoC for both of these
// SPI ROMs have GPspispi header, so a GeneralPlus chip, seems to have Thumb-2 instructions only and boot ROMs are similar to lx_jg7420 etc. (so likely a Cortex-M)
// very generic packaging, boots from SPI, has game data on SD card (mostly NES games)
CONS( 202?, intrtvg,         0,        0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init,  "Shen Zhen Shi Fei Xun Hu Dong Technology",     "Interactive Game Console (Model B608, YRPRSODF)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// also very generic packaging, similar SD card content to above, including NES games, but with some extra music/videos for the dance part
CONS( 202?, ardancem,        0,        0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init,  "Shen Zhen Shi Fei Xun Hu Dong Technology",     "AR Dance Mat (Model DM02, YRPRSODF)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// likewise
CONS( 202?, arb605,          0,        0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init,  "Shen Zhen Shi Fei Xun Hu Dong Technology",     "AR Game Console (Model B605, YRPRSODF)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS( 202?, bodygun,         0,        0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init,  "Shen Zhen Shi Fei Xun Hu Dong Technology", "Body Gun Game Console (Model GC05, Damcoola)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS( 202?, kaximond,        0,        0,      gpm4530a_lexibook, gpm4530a_lexibook, gpm4530a_lexibook_state, empty_init,  "Kaximon", "Double Dance Mat with HDMI (Kaximon)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
