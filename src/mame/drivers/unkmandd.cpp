// license:BSD-3-Clause
// copyright-holders:David Haywood

// unkmandd contains unsp code, but no obvious startup code / vectors, so it's probably booting from another device / bootstrapped

#include "emu.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"

class unkmandd_state : public driver_device
{
public:
	unkmandd_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void unkmandd(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

uint32_t unkmandd_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}




void unkmandd_state::machine_start()
{
}

void unkmandd_state::machine_reset()
{
}

static INPUT_PORTS_START( unkmandd )
INPUT_PORTS_END

void unkmandd_state::unkmandd(machine_config &config)
{
	// unknown CPU, unsp based

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(unkmandd_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);
}

ROM_START( unkmandd )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "8718_en25f32.bin", 0x000000, 0x400000, CRC(cc138db4) SHA1(379af3d94ae840f52c06416d6cf32e25923af5ae) )
ROM_END

CONS( 200?, unkmandd,      0,       0,      unkmandd,   unkmandd, unkmandd_state, empty_init, "M&D", "unknown M&D handheld", MACHINE_IS_SKELETON )
