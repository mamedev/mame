// license:BSD-3-Clause
// copyright-holders:David Haywood


/*

a 221 games console which uses a 4GB sd-card for gamestorage and a MX29LV160 flashrom for the internal bios. (only 512kb are used from the 2mb romspace)
Starting the console without SD-Card just show's a looping video with "Please insert Memory Card".

SD card image produced with WinHex (hardware write blocker used to prevent Windows from corrupting data)
compressed with "chdman createhd -i 4GBSD.img -o lexibook_jg7425_4gbsd.chd" (is this correct?)

TODO:
identify CPU type and if there's any kind of additional internal boot ROM
(only noteworthy features of PCB are ROM + RAM + Cpu Glob)

*/

#include "emu.h"
#include "screen.h"

class lexibook_jg7425_state : public driver_device
{
public:
	lexibook_jg7425_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen")
	{ }

	void lexibook_jg7425(machine_config &config);

protected:
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<screen_device> m_screen;
};



void lexibook_jg7425_state::machine_start()
{
}

void lexibook_jg7425_state::machine_reset()
{
}

static INPUT_PORTS_START( lexibook_jg7425 )
INPUT_PORTS_END


uint32_t lexibook_jg7425_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void lexibook_jg7425_state::lexibook_jg7425(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(lexibook_jg7425_state::screen_update));
}

ROM_START( lx_jg7425 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mx29lv160.u6", 0x000000, 0x200000, CRC(43c90080) SHA1(4c9e5c8f880d40bd684357ce67ae45c3f5d24b62) )

	DISK_REGION( "ata:0:hdd:image" ) /* 4GB SD Card */
	DISK_IMAGE( "lexibook_jg7425_4gbsd", 0, SHA1(dc0985103edec3992efdd493feef6185daedb3fd) )
ROM_END

CONS( 2015, lx_jg7425,   0,         0,      lexibook_jg7425,   lexibook_jg7425, lexibook_jg7425_state, empty_init, "Lexibook", "Lexibook JG7425 221-in-1", MACHINE_IS_SKELETON )
