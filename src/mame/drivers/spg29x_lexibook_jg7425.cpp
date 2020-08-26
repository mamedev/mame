// license:BSD-3-Clause
// copyright-holders:David Haywood


/*

a 221 games console which uses a 4GB sd-card for gamestorage and a MX29LV160 flashrom for the internal bios. (only 512kb are used from the 2mb romspace)
Starting the console without SD-Card just show's a looping video with "Please insert Memory Card".

SD card image produced with WinHex (hardware write blocker used to prevent Windows from corrupting data)
compressed with "chdman createhd -i 4GBSD.img -o lexibook_jg7425_4gbsd.chd" (is this correct?)

TODO:
is there an internal ROM / bootstrap area, or does this SunPlus core use vectors in a different way to the one in hyperscan.cpp?
If SPG290, should probably be merged with hyperscan.cpp (it is)

(only noteworthy features of PCB are ROM + RAM + Cpu Glob)

*/

#include "emu.h"
#include "screen.h"
#include "cpu/score/score.h"

class lexibook_jg7425_state : public driver_device
{
public:
	lexibook_jg7425_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram"),
		m_romregion(*this, "maincpu")
	{ }

	void lexibook_jg7425(machine_config &config);

	void map(address_map& map);

protected:
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<score7_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_mainram;
	required_region_ptr<uint32_t> m_romregion;
};



void lexibook_jg7425_state::machine_start()
{
	// I think this code should be running from RAM at least, probably some kind of bootstrap / internal ROM to copy it? (hyperscan.cpp indicates that SoC can have internal ROM at least)

	// first 0x20 bytes are probably pointers, code starts at 0x20 and is offset, maps at 0x500000

	for (int i = 0; i < 0x80000 / 4; i++)
	{
		m_mainram[i+(0x500000/4) - (0x20/4)] = m_romregion[i];
	}
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

void lexibook_jg7425_state::map(address_map &map)
{
	map(0x00000000, 0x00ffffff).ram().share("mainram");

	map(0x9f000000, 0x9fffffff).ram().share("mainram");
}


void lexibook_jg7425_state::lexibook_jg7425(machine_config &config)
{
	/* basic machine hardware */
	SCORE7(config, m_maincpu, XTAL(27'000'000) * 4);   // ? not certain on exact type
	m_maincpu->set_addrmap(AS_PROGRAM, &lexibook_jg7425_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(lexibook_jg7425_state::screen_update));
}

ROM_START( lx_jg7425 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD( "mx29lv160.u6", 0x000000, 0x200000, CRC(43c90080) SHA1(4c9e5c8f880d40bd684357ce67ae45c3f5d24b62) )

	DISK_REGION( "ata:0:hdd:image" ) /* 4GB SD Card */
	DISK_IMAGE( "lexibook_jg7425_4gbsd", 0, SHA1(dc0985103edec3992efdd493feef6185daedb3fd) )
ROM_END

ROM_START( lx_aven )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD( "29lv800.bin", 0x000000, 0x100000, CRC(7b107f6c) SHA1(3a8e37e51dab5cab9977261e0ac17ba5194a9370) )

	DISK_REGION( "ata:0:hdd:image" ) /* 4GB SD Card */
	DISK_IMAGE( "sd-card", 0, SHA1(911da7bf7dac391e3329e17e3f411caafac52f0f) )
ROM_END


CONS( 2015, lx_jg7425,   0,         0,      lexibook_jg7425,   lexibook_jg7425, lexibook_jg7425_state, empty_init, "Lexibook", "Lexibook JG7425 221-in-1", MACHINE_IS_SKELETON )
CONS( 201?, lx_aven,     0,         0,      lexibook_jg7425,   lexibook_jg7425, lexibook_jg7425_state, empty_init, "Lexibook", "Marvel Avengers TV Game Console (32-bit) (Lexibook)", MACHINE_IS_SKELETON )
