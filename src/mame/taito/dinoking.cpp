// license:BSD-3-Clause
// copyright-holders:

/*

TODO:
- irq 1
- blitter transfers
- i/o
- hangs at PC=288c

Taito mid-2000s medal hardware

Currently two games are dumped:
Dinoking Kids - ダイノキングキッズ (2004) - K11J0985A - F39 ROM code
Dinoking Battle - ダイノキングバトル (2005) - K11J0998A - F54 ROM code

Main CPU: H8S-2394
Video chip: Axell AG-1 AX51102A (should be similar to the chip in gunpey.cpp)
Sound chip: OKI M9810B
XTALs: 20 MHz near CPU and sound chip, 25 MHz near video chip
*/



#include "emu.h"
#include "cpu/h8/h8s2357.h"
#include "machine/timer.h"
#include "sound/okim9810.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class dinoking_state : public driver_device
{
public:
	dinoking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{
	}

	void dinoking(machine_config &config);

protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	u8 m_irq_cause = 0, m_irq_mask = 0;
	void irq_check(u8 irq_type);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void status_w(offs_t offset, u8 data);
	u8 status_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

u32 dinoking_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 dinoking_state::status_r(offs_t offset)
{
	if (offset == 1)
		return m_irq_cause;

	return m_irq_mask;
}

void dinoking_state::irq_check(u8 irq_type)
{
	m_irq_cause |= irq_type;

	if (m_irq_cause & m_irq_mask)
		m_maincpu->set_input_line(1, ASSERT_LINE);
	else
		m_maincpu->set_input_line(1, CLEAR_LINE);
}


void dinoking_state::status_w(offs_t offset, u8 data)
{
	if (offset == 1)
	{
		m_irq_cause &= ~data;
		irq_check(0);
	}

	if (offset == 0)
	{
		m_irq_mask = data;
		irq_check(0);
	}
}


void dinoking_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x400000, 0x4fffff).ram();

	map(0x61fcb8, 0x61fcb9).rw(FUNC(dinoking_state::status_r), FUNC(dinoking_state::status_w));
	map(0x61fcd2, 0x61fcd3).lrw8(
		NAME([] () {
			// read in irq service 0x4, blitter status?
			return 0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// blitter irq, after some time PC=4ce
			if (offset && data == 8)
				irq_check(0x04);
		})
	);

	map(0x800001, 0x800001).w("oki", FUNC(okim9810_device::tmp_register_w));
	map(0x800000, 0x800000).w("oki", FUNC(okim9810_device::write));
	map(0x800002, 0x800002).r("oki", FUNC(okim9810_device::read));

}

static INPUT_PORTS_START( dinoking )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(dinoking_state::scanline)
{
	int scanline = param;

	if (scanline == 240)
	{
		// checked bits in irq service = 0x3e
		// bit 5 has no meaning, just clears in service.
		irq_check(0x10);
	}
}



void dinoking_state::dinoking(machine_config &config)
{
	H8S2394(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dinoking_state::mem_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(dinoking_state::scanline), "screen", 0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(dinoking_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::RGB_555);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	okim9810_device &oki(OKIM9810(config, "oki", XTAL(4'096'000)));
	oki.add_route(0, "lspeaker", 0.80);
	oki.add_route(1, "rspeaker", 0.80);
}


ROM_START( dkkids )
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f39-03.ic3", 0x000000, 0x200000, CRC(061ea9e6) SHA1(9a6957a24e228aa1f44bf91c635894ecd4871623) ) // ST-M27C160-100

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "f39-01.ic30", 0x000000, 0x1000000, CRC(1eb9efa6) SHA1(08aa33cb9a5e1e57702d23d7692ad9a289b9a4ed) ) // ST-M59PW1282-100MIT

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "f39-02.ic27", 0x000000, 0x400000, CRC(f0a38e0e) SHA1(30ee275d4cce5bb50141e807dc64d6509eaf4937) ) // MBM29F033C-90PTN
ROM_END

ROM_START( dkbattle )
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f54-03.ic3", 0x000000, 0x200000, CRC(eeb61085) SHA1(33304c1fd068dc4fd0bacfd4e47e699aa4ae0b82) ) // ST-M27C160-100

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "f54-01.ic30", 0x000000, 0x1000000, CRC(f2006d02) SHA1(3ba1832eb3440b175014a4d00b56e24a9d8d1994) ) // ST-M59PW1282-100MIT

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "f54-02.ic27", 0x000000, 0x400000, CRC(bd8e10e9) SHA1(a86fde5860501b06f63bafbc02ebc6160c682b1e) ) // MBM29F033C-90PTN
ROM_END

} // anonymous namespace


GAME( 2004, dkkids,   0, dinoking, dinoking, dinoking_state, empty_init, ROT0, "Taito Corporation", "Dinoking Kids",   MACHINE_IS_SKELETON )
GAME( 2005, dkbattle, 0, dinoking, dinoking, dinoking_state, empty_init, ROT0, "Taito Corporation", "Dinoking Battle", MACHINE_IS_SKELETON )
