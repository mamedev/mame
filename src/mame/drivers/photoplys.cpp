// license:BSD-3-Clause
// copyright-holders:
/*
Funworld Photo Play motherboard 6WEV (used on games like Photo Play Smart, Photo Play Spirit, etc.)

Motherboard 6WEV0:

CPU: Intel CELERON FV524RX366 128 SL36C
RAM: 64MB
PCB: Intel FW82801AA, IT8888F PCI-to-ISA Bridge, CMI8738 audio
I/O: Winbond W83977, Winbond W83627HF
BIOS: Intel N82802AB
Dongle: Parallel port + keyboard DIN (uses both ports) with a SX28AC/DP MCU (Parallax, Ubicom, etc.). On some versions there's also a SEEPROM (93C46LN, etc.).
Net: Optional Ethernet PCI card with RTL8139C

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

namespace {

class photoplays_state : public driver_device
{
public:
	photoplays_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void photoplays(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void photoplays_map(address_map &map);
};

void photoplays_state::video_start()
{
}

uint32_t photoplays_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void photoplays_state::photoplays_map(address_map &map)
{
}

static INPUT_PORTS_START( photoplays )
INPUT_PORTS_END


void photoplays_state::machine_start()
{
}

void photoplays_state::machine_reset()
{
}

void photoplays_state::photoplays(machine_config &config)
{
	// Basic machine hardware
	PENTIUM2(config, m_maincpu, 366'000'000); // Actually an Intel CELERON FV524RX366 128 SL36C
	m_maincpu->set_addrmap(AS_PROGRAM, &photoplays_state::photoplays_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(photoplays_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( photoply2k1sp )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("photoplay_6wev0_n82802ab8.bin", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION(0x4000, "dongle", 0)
	ROM_LOAD("sx28ac_dp_g.bin", 0x0000, 0x4000, NO_DUMP ) // 2Kbytes flash

	DISK_REGION( "ide:0:hdd:image" ) // 06/06/2001
	DISK_IMAGE( "photoplay2001es", 0, BAD_DUMP SHA1(43aa4e38d57bfe4c6decddadf77d322bf30426a5) ) // May contain operator data / configuration
ROM_END

} // Anonymous namespace


GAME( 2001, photoply2k1sp, 0, photoplays, photoplays, photoplays_state, empty_init, ROT0, "Funworld", "Photo Play 2001 (Spanish)", MACHINE_IS_SKELETON )
