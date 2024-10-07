// license:BSD-3-Clause
// copyright-holders:David Haywood

// HD641016 + HD63484 + YM3812
// seems very much like B52, but reworked to use a newer CPU so the skeleton hookups are copy+pasted from there

#include "emu.h"

#include "cpu/h16/hd641016.h"
#include "machine/6840ptm.h"
#include "sound/ymopl.h"
#include "video/hd63484.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class sigmab88_state : public driver_device
{
public:
	sigmab88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
	{
	}

	void sigmab88(machine_config &config);

private:
	void h16_map(address_map &map) ATTR_COLD;
	void hd63484_map(address_map &map) ATTR_COLD;

	required_device<hd641016_device> m_maincpu;
	required_device<palette_device> m_palette;
};


void sigmab88_state::h16_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x0c0000, 0x0dffff).ram();
	map(0x100000, 0x10001f).unmaprw(); // numeric LED segment controller?
	map(0x102800, 0x10280f).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x103000, 0x103003).rw("acrtc", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x104000, 0x104001).unmapr(); // ?
	map(0x104400, 0x104401).unmapr(); // ?
	map(0x104600, 0x104601).unmapr(); // ?
	map(0x104800, 0x104801).unmapr(); // ?
	map(0x104a00, 0x104a01).unmapr(); // ?
	map(0x104e00, 0x104e01).unmapr(); // ?
	map(0x104e80, 0x104e81).unmapr(); // ?
	map(0x104f00, 0x104f01).unmapr(); // ?
	map(0x104f80, 0x104f81).unmapr(); // ?
	map(0x105000, 0x105003).w("ymsnd", FUNC(ym3812_device::write)).umask16(0xff00);
	map(0x106000, 0x106001).unmapw(); // ?
	map(0x107000, 0x107003).unmapw(); // ?
	//map(0x107900, 0x10790f).w("outlatch0", FUNC(addressable_latch_device::write_d0)).umask16(0x00ff);
	//map(0x107a00, 0x107a0f).w("outlatch1", FUNC(addressable_latch_device::write_d0)).umask16(0x00ff);
	//map(0x107c00, 0x107c0f).w("outlatch2", FUNC(addressable_latch_device::write_d0)).umask16(0x00ff);
	map(0xc00000, 0xc7ffff).rom().region("maincpu", 0);
}

void sigmab88_state::hd63484_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
}

static INPUT_PORTS_START(sigmab88)
INPUT_PORTS_END

void sigmab88_state::sigmab88(machine_config &config)
{
	HD641016(config, m_maincpu, XTAL(20'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sigmab88_state::h16_map);

	PTM6840(config, "ptm", XTAL(20'000'000)/2/16);
	//ptm.irq_callback().set_inputline(m_maincpu, hd641016_device::IRQ1_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1024, 1024);
	screen.set_visarea(0, 544-1, 0, 436-1);
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette(m_palette);

	HD63484(config, "acrtc", XTAL(8'000'000)).set_addrmap(0, &sigmab88_state::hd63484_map);

	PALETTE(config, m_palette).set_entries(16);

	YM3812(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);

	SPEAKER(config, "mono").front_center();
}

// should these have a colour PROM like b52?

ROM_START( cool104 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_BYTE("wwp2080-7213-1.bin", 0x00000, 0x40000, CRC(95b4b518) SHA1(1ac64131508999a5206a5b3e3efbc965419eb247))
	ROM_LOAD16_BYTE("wwp2080-7213-2.bin", 0x00001, 0x40000, CRC(3489277d) SHA1(c51e165d6382f9b709f4b0070a5a43aa519cb0d6))

	ROM_REGION(0x100000, "data", 0) // does this map to CPU space?
	ROM_LOAD16_BYTE("grp0301-010-1.bin", 0x00000, 0x80000, CRC(fc00a817) SHA1(30f9d6ce9091e35e341cc6e57765f97453aa863f))
	ROM_LOAD16_BYTE("grp0301-010-2.bin", 0x00001, 0x80000, CRC(a5b19862) SHA1(6552bcd481ba807c66c1b20f6809dbd4d886183a))
ROM_END

ROM_START( freedeal )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_BYTE("wwp2047-1712-1.bin", 0x00000, 0x40000, CRC(da552ce7) SHA1(d09902d8fcb8cbd908df97d3a265ea8b185e4963))
	ROM_LOAD16_BYTE("wwp2047-1712-2.bin", 0x00001, 0x40000, CRC(322020e7) SHA1(640c0d24e17424cad68f00a5df293d8bbbc30309))

	ROM_REGION(0x100000, "data", 0) // does this map to CPU space?
	ROM_LOAD16_BYTE("grp8001-010-1.bin", 0x00000, 0x80000, CRC(5071d33d) SHA1(3adf92cfc322e3636ce3587ca86c7848f2cb49ca))
	ROM_LOAD16_BYTE("grp8001-010-2.bin", 0x00001, 0x80000, CRC(09913d97) SHA1(ab77d220a477be9b677bc1c101876b0e7d3596f9))
ROM_END

} // anonymous namespace

GAME( 1997, cool104,  0,        sigmab88, sigmab88, sigmab88_state, empty_init, ROT0, "Sigma", "Cool 104", MACHINE_IS_SKELETON )
GAME( 1997, freedeal, 0,        sigmab88, sigmab88, sigmab88_state, empty_init, ROT0, "Sigma", "Free Deal Twin Jokers Progressive", MACHINE_IS_SKELETON )
