// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    BASF 7100

    Models:
    - 7120: 24k disk controller memory, 3x 5.25" single sided
    - 7125: 32k disk controller memory, 3x 5.25" double sided
    - 7130: 32k disk controller memory, 1x 5.25" double sided, 1x Winchester

    Hardware:
    - Z-80A
    - I8259A
    - I8251
    - COM8116
    - 3x 8255A
    - 64k memory
    - 5.0688 MHz XTAL
    - 4x DSW8
    - External ports: Printer, Dialer, Data Comm I/O, Aux I/O

    Video board:
    - Motorola 160A002-B (?)
    - 2x 8255A
    - 18.720 MHz XTAL

    Floppy controller:
    - Z-80A
    - 24k/32k memory
    - FD1791B-02

    Aux I/O board:
    - I8251
    - COM8116
    - 5.0688 MHz XTAL

    Keyboard:
    - 30293E-054 20-04592-054 (?)

    TODO:
    - Everything

    Notes:
    - Runs the BOS operating system, possibly also CP/M?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class basf7100_state : public driver_device
{
public:
	basf7100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fdccpu(*this, "fdccpu")
	{ }

	void basf7100(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_fdccpu;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	void fdc_mem_map(address_map &map);
	void fdc_io_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void basf7100_state::mem_map(address_map &map)
{
}

void basf7100_state::io_map(address_map &map)
{
}

void basf7100_state::fdc_mem_map(address_map &map)
{
	map(0xfc00, 0xffff).rom().region("fdccpu", 0);
}

void basf7100_state::fdc_io_map(address_map &map)
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( basf7100 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void basf7100_state::machine_start()
{
}

void basf7100_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_fdccpu->set_pc(0xfc00);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void basf7100_state::basf7100(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &basf7100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &basf7100_state::io_map);

	Z80(config, m_fdccpu, 4000000);
	m_fdccpu->set_addrmap(AS_PROGRAM, &basf7100_state::fdc_mem_map);
	m_fdccpu->set_addrmap(AS_IO, &basf7100_state::fdc_io_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( basf7120 )
	ROM_REGION(0x400, "fdccpu", 0)
	ROM_LOAD("19-2130-2h.u45", 0x000, 0x400, CRC(cb077c69) SHA1(dfa16082b88275442c48082aeb5f62fe1238ae3e))

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD("19-2114-01e.bin", 0x000, 0x400, CRC(d694b5dd) SHA1(6262379ba565c1de072b2b21dc3141db1ec5129c))

	ROM_REGION(0x170, "pals", 0)
	ROM_LOAD("cpu_board_im561cje_192113-2.u21",      0x000, 0x020, CRC(4405e26f) SHA1(0f93c47e9f546b42a85e5eced58337e0add443c4))
	ROM_LOAD("floppy_board_pal10l8mj_19-2131-1.u23", 0x000, 0x028, CRC(f37ed4bc) SHA1(824b4405f396c262cf8116f85eb0b548eabb4c04))
	ROM_LOAD("floppy_board_pal10l8mj_19-2132-1.u24", 0x000, 0x028, CRC(b918ff18) SHA1(c6d7cd9642ed32e56b5c1df1ddf3afe09d744ebc))
	ROM_LOAD("video_board_30_82s129n.bin",           0x000, 0x100, CRC(89175ac9) SHA1(69b2055bee87e11cc74c70cef2f2bebcbd0004c9))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME  FLAGS
COMP( 1982, basf7120, 0,      0,      basf7100, basf7100, basf7100_state, empty_init, "BASF",   "7120",   MACHINE_IS_SKELETON )
