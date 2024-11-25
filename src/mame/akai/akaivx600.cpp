// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Akai VX600 analog synthesizer.

***************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/upd78k/upd78k3.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
//#include "screen.h"


namespace {

class akaivx600_state : public driver_device
{
public:
	akaivx600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
	{
	}

	void vx600(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_device<upd78310_device> m_maincpu;
	required_device<upd78c11_device> m_subcpu;
};


void akaivx600_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program1", 0);
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xc7ff).ram().share("nvram");
	map(0xf020, 0xf023).w("pit", FUNC(pit8253_device::write));
}

void akaivx600_state::sub_map(address_map &map)
{
	map(0x4000, 0x5fff).rom().region("program2", 0);
}


static INPUT_PORTS_START(vx600)
INPUT_PORTS_END

void akaivx600_state::vx600(machine_config &config)
{
	UPD78310(config, m_maincpu, 12_MHz_XTAL); // μPD78310G
	m_maincpu->set_addrmap(AS_PROGRAM, &akaivx600_state::main_map);

	UPD78C11(config, m_subcpu, 12_MHz_XTAL); // μPD78C11G-044
	m_subcpu->set_addrmap(AS_PROGRAM, &akaivx600_state::sub_map);

	NVRAM(config, "nvram"); // CXK5816-PN12L + BR2032

	PIT8253(config, "pit"); // μPD8253-2

	//LC7981(config, "lcdc");
}

ROM_START(vx600)
	ROM_REGION(0x8000, "program1", 0)
	ROM_LOAD("vx600_-1-_v1.2.ic45", 0x0000, 0x8000, CRC(19a32cc4) SHA1(9190cb47456f12959272f1b160c73298da638ba2)) // 27C256

	ROM_REGION(0x1000, "subcpu", 0)
	ROM_LOAD("upd78c11g-044-36.ic46", 0x000, 0x1000, NO_DUMP)
	ROM_FILL(0x0000, 1, 0x54) // dummy reset vector
	ROM_FILL(0x0001, 1, 0x00)
	ROM_FILL(0x0002, 1, 0x40)
	ROM_FILL(0x0018, 1, 0x54) // dummy interrupt vector
	ROM_FILL(0x0019, 1, 0x18)
	ROM_FILL(0x001a, 1, 0x40)
	ROM_FILL(0x0090, 1, 0xca) // dummy CALT vectors
	ROM_FILL(0x0091, 1, 0x41)
	ROM_FILL(0x0092, 1, 0xca)
	ROM_FILL(0x0093, 1, 0x41)
	ROM_FILL(0x0094, 1, 0xca)
	ROM_FILL(0x0095, 1, 0x41)
	ROM_FILL(0x0096, 1, 0xca)
	ROM_FILL(0x0097, 1, 0x41)
	ROM_FILL(0x0098, 1, 0xca)
	ROM_FILL(0x0099, 1, 0x41)
	ROM_FILL(0x009a, 1, 0xca)
	ROM_FILL(0x009b, 1, 0x41)

	ROM_REGION(0x2000, "program2", 0)
	ROM_LOAD("vx600_-2-_v1.12.ic47", 0x0000, 0x2000, CRC(1b3ee178) SHA1(20a319392824f9f3074f370d9d9eb2f312d69ac4)) // 27C64
ROM_END

} // anonymous namespace


SYST(1988, vx600, 0, 0, vx600, vx600, akaivx600_state, empty_init, "Akai", "VX600 Programmable Matrix Synthesizer", MACHINE_IS_SKELETON)

