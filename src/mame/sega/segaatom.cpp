// license:BSD-3-Clause
// copyright-holders:MetalliC
/***************************************************************************

    Atom II
    (c) Sega 200?

 Main board:
  Sega PC BD ATOM II
  837-14800
  U0150841
  (c) Sega 2007 171-8477A

   Hitachi H8S/2373 CPU
   25MHz OSC
   R1LV0408CSP 512k x8 SRAM
   M27C4001 512k x8 IC4 EPROM DIL socket
   Axell AG-2 AX51201 GPU
   25.18MHz OSC
   2x 48LC8M16A2 2M x16 x4banks SDRAM (2x 16Mbyte)
   Yamaha YMZ770C-F
   16.384MHz OSC
   Sega 315-6549 (Xilinx XC95144XL CPLD)

 ROM board:
  PC BD ATOM MEMO
  837-14806
  (c) Sega 2006 171-8363B

   3x S29GL128N 128Mbit flash ROMs

***************************************************************************/


#include "emu.h"
#include "cpu/h8/h8s2357.h"
#include "sound/ymz770.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class atom2_state : public driver_device
{
public:
	atom2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void atom2(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void atom2_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
};

void atom2_state::video_start()
{
}

uint32_t atom2_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	return 0;
}

void atom2_state::machine_start()
{
}

void atom2_state::machine_reset()
{
}

static INPUT_PORTS_START( atom2 )
INPUT_PORTS_END

void atom2_state::atom2_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
}


void atom2_state::atom2(machine_config &config)
{
	/* basic machine hardware */
	H8S2394(config, m_maincpu, 25_MHz_XTAL);  // wrong type, should be H8S/2378 family group
	m_maincpu->set_addrmap(AS_PROGRAM, &atom2_state::atom2_map);

	/* video hardware dummy */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update(FUNC(atom2_state::screen_update));

	PALETTE(config, "palette").set_entries(65536);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz770_device &ymz770(YMZ770(config, "ymz770", 16.384_MHz_XTAL));
	ymz770.add_route(0, "lspeaker", 1.0);
	ymz770.add_route(1, "rspeaker", 1.0);
}


/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( spongbob )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 )
	// Main board
	ROM_LOAD("epr-24430.ic4", 0x0000000, 0x80000, NO_DUMP )

	// ROM board
	ROM_LOAD("ic1", 0x0000000, 0x1000000, CRC(607a0989) SHA1(17706df8400e13800edd1d1cf45047d1c970d0b8) )
	ROM_LOAD("ic2", 0x1000000, 0x1000000, CRC(88c8db6b) SHA1(d716e752a3e70e6cfdf82bb1e9ef73897452bb84) )

	ROM_REGION(0x1000000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD("ic5", 0x0000000, 0x1000000, CRC(d8e9bc95) SHA1(91798ea0f47f3340bc32b754d0be8fec5d093122) )
ROM_END

} // anonymous namespace


GAME( 200?, spongbob,  0, atom2, atom2, atom2_state, empty_init, ROT0, "Sega",      "SpongeBob SquarePants Ticket Boom", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
