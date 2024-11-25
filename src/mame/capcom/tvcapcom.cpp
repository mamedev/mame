// license:BSD-3-Clause
// copyright-holders:
/*

 Tatsunoko Vs Capcom : Cross Generation of Heroes

 Wii derived hardware

 -- todo, add more hardware info, seems to have been lost at some point

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "emupal.h"
#include "screen.h"


namespace {

class tvcapcom_state : public driver_device
{
public:
	tvcapcom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	void tvcapcom(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_tvcapcom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<ppc_device> m_maincpu;

	void gc_map(address_map &map) ATTR_COLD;
};

void tvcapcom_state::gc_map(address_map &map)
{
}


void tvcapcom_state::machine_start()
{
}

void tvcapcom_state::video_start()
{
}


uint32_t tvcapcom_state::screen_update_tvcapcom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( tvcapcom )
INPUT_PORTS_END

void tvcapcom_state::tvcapcom(machine_config &config)
{
	/* basic machine hardware */
	PPC603(config, m_maincpu, 72900000); // IBM PowerPC Broadway CPU @ 729 MHz  ?
	m_maincpu->set_addrmap(AS_PROGRAM, &tvcapcom_state::gc_map);
	m_maincpu->set_disable();

	config.set_maximum_quantum(attotime::from_hz(6000));


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update(FUNC(tvcapcom_state::screen_update_tvcapcom));

	PALETTE(config, "palette").set_entries(65536);
}

ROM_START( tvcapcom )
	// BIOS??

	ROM_REGION( 0x21000898, "flash", ROMREGION_ERASE) // it's possible all these dumps are bad

	ROM_LOAD("tvc_read1.u14", 0x00, 0x21000898, CRC(51de96ac) SHA1(11a631a695140efa299e6fe9e68c0026dcebc766) ) // old dump, lots of data repeats, seems to cut out in the middle of a block?!

	// alt attempts at dumping the same rom, same weird size as above.. the same CRC was read consistently each time, but then differed when read again later without being used inbetween, some blocks seem to get swapped around?!
	ROM_LOAD("tvc_read2.u14",  0x00, 0x21000898, CRC(efb5911f) SHA1(555e58c2d3744cfe649b7424937c07a3ce675838))
	ROM_LOAD("tvc_read3.u14",  0x00, 0x21000898, CRC(4a3f143d) SHA1(83bb7abc5f925df9c4e28de0298aed7b3b791e37))

ROM_END

} // anonymous namespace


GAME( 2008, tvcapcom,  0, tvcapcom,    tvcapcom, tvcapcom_state, empty_init, ROT0, "Capcom",            "Tatsunoko Vs Capcom : Cross Generation of Heroes", MACHINE_IS_SKELETON )
