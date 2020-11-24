// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Yamaha PSR-340 PortaSound keyboard
    Preliminary driver by R. Belmont

    CPU and Sound: SWX00B, which is an H8S/2000 series CPU and Yamaha SWP00? (or SWP30?) on one chip.
    LCD controller: KS0066U (apparently equivalent to Samsung S6A0069)
    FDC: HD63266F (uPD765 derivative)
    RAM: 256KiB, 2x uPD431000 or equivalent

    TODO:
    - Figure out correct CPU model.  H8S/2655 seems to be the only H8 model in MAME where this actually boots, so it at
      least has similar or a superset of the 2655 peripheral layout.
      CPU is at least H8S/2xxx, and may be H8S/26xx like MU-100.
    - LCD (it's similar to the MU-5's)
    - Front panel / keyboard scanner (6305 with internal ROM manages this on hardware)
    - Sound generation
*/

#include "emu.h"

#include "cpu/h8/h8s2655.h"
#include "sound/swp00.h"

#include "screen.h"
#include "speaker.h"

class psr340_state : public driver_device
{
public:
	psr340_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_swp00(*this, "swp00")
	{ }

	void psr340(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<h8s2655_device> m_maincpu;
	required_device<swp00_device> m_swp00;

	void psr340_map(address_map &map);
	void psr340_io_map(address_map &map);

	void lcd_ctrl_w(u8 data);
	void lcd_data_w(u8 data);
};

void psr340_state::lcd_ctrl_w(u8 data)
{
	// bit 3 = E, bit 4 = RS, R/W is connected to GND so write-only
}

void psr340_state::lcd_data_w(u8 data)
{
}

void psr340_state::psr340_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x400000, 0x43ffff).ram();  // Work RAM?  Or SWP / MEG?

	map(0x600000, 0x600000).lr8(NAME([]() -> uint8_t { return 0x80; }));    // FDC?

	map(0xffe02a, 0xffe02a).w(FUNC(psr340_state::lcd_ctrl_w));
	map(0xffe02b, 0xffe02b).w(FUNC(psr340_state::lcd_data_w));
}

void psr340_state::psr340_io_map(address_map &map)
{
}

void psr340_state::machine_start()
{
}

void psr340_state::machine_reset()
{
}

static INPUT_PORTS_START(psr340)
INPUT_PORTS_END

void psr340_state::psr340(machine_config &config)
{
	/* basic machine hardware */
	H8S2655(config, m_maincpu, 8467200);    // actual type unknown, clock from schematic
	m_maincpu->set_addrmap(AS_PROGRAM, &psr340_state::psr340_map);
	m_maincpu->set_addrmap(AS_IO, &psr340_state::psr340_io_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SWP00(config, m_swp00);
	m_swp00->add_route(0, "lspeaker", 1.0);
	m_swp00->add_route(1, "rspeaker", 1.0);
}

ROM_START( psr340 )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP("xv89710.bin", 0x000000, 0x200000, CRC(271ccb8a) SHA1(ec6abbdb82a5e851b77338c79ecabfd8040f023d))

	ROM_REGION(0x200000, "swp00", 0)
	ROM_LOAD("xv89810.bin", 0x000000, 0x200000, CRC(10e68363) SHA1(5edee814bf07c49088da44474fdd5c817e7c5af0))
ROM_END

CONS(1994, psr340, 0, 0, psr340, psr340, psr340_state, empty_init, "Yamaha", "PSR-340 PortaSound", MACHINE_NOT_WORKING)
