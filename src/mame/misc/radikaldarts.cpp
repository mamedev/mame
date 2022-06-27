// license:BSD-3-Clause
// copyright-holders:
/*
    Radikal Darts / Radikal System hardware by Gaelco Darts.

    PC hardware running Linux:
    -Intel Celeron SL6C8 1200/256/100/1.5
    -Chipset: Intel NH82801BA, Intel 82815 Graphics Controller + Realtek RTM560 + IRU3013 + Intel DA82562EM Ethernet
    -IDE hard disk (40GB)
    -128MB RAM (1 x Kingston KVR133X64C3Q/128)

    Additional Gaelco custom riser PCB (REF. 050525) with:
    -IDE connector.
    -DVI connector.
    -AD9288 ADC + Altera Cyclone EP1C3T144C8N FPGA + 2 x ALVCH16374 + PCM1725U + DS90C385AMT
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

namespace {

class radikaldarts_state : public driver_device
{
public:
	radikaldarts_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void radikaldarts(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void radikaldarts_map(address_map &map);
};

void radikaldarts_state::video_start()
{
}

uint32_t radikaldarts_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void radikaldarts_state::radikaldarts_map(address_map &map)
{
}

static INPUT_PORTS_START( radikaldarts )
INPUT_PORTS_END


void radikaldarts_state::machine_start()
{
}

void radikaldarts_state::machine_reset()
{
}

void radikaldarts_state::radikaldarts(machine_config &config)
{
	// Basic machine hardware
	PENTIUM3(config, m_maincpu, 120000000); // Celeron SL6C8 1.2 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &radikaldarts_state::radikaldarts_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1440, 900);
	screen.set_visarea(0, 1440-1, 0, 900-1);
	screen.set_screen_update(FUNC(radikaldarts_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( radikaldrt )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("sst49lf004b_plcc32.u10", 0x00000, 0x80000, CRC(53ab9628) SHA1(5cd54ecb03e29352d8acd3e2e9be5dfbc4dd4064) ) // BIOS string: "10/16/2002-i815EP-627-6A69RPAVC-00 00"

	DISK_REGION( "ide:0:hdd:image" ) // Hitachi HTS541040G9AT00
	DISK_IMAGE( "hts541040g9at00_dv7.29.25", 0, BAD_DUMP SHA1(37c987c3f5493cabe9f54786702349029e0fda59) ) // Contains operator and players data
ROM_END

} // Anonymous namespace

GAME( 2011?, radikaldrt, 0, radikaldarts, radikaldarts, radikaldarts_state, empty_init, ROT0, "Gaelco Darts", "Radikal Darts (Diana Version 7.29.25)", MACHINE_IS_SKELETON )
