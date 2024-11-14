// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Millennium Systems 9520 and 9540 Software Development systems.

2013-08-30 Skeleton driver

The only roms known to be dumped are the main roms of the 9540, and the disk
controller roms of the 9520.

There's no known documents for the 9540, so everything in here is a guess.

The 9520 is Z80-based, but is currently not emulated.
The 9540 is 68000-based.

Chips:
- Main board: MC68000L8, 2x SCN2661C, uPD8253C. Crystals: 5.0688, 16MHz
  ROMS: 0950-0131-01, 0950-0132-01, 0954-0133-01, 0954-0134-01,
        0954-0135-01, 0954-0135-02
- Disk board: FD1797A-02, AM9517A, 0954-0039-01
- Serial board: 6x SCN2661C, 2x AM9517A, 0954-0029-01. Crystal: 5.0688MHz

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"


namespace {

class ms9540_state : public driver_device
{
public:
	ms9540_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "mainram")
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	void ms9540(machine_config &config);

private:
	uint8_t latch_1e001_r();
	void latch_1e001_w(u8 data);
	uint8_t latch_1f001_r();
	void latch_1f001_w(u8 data);
	void kbd_put(u8 data);
	void mem_map(address_map &map) ATTR_COLD;
	uint8_t m_term_data = 0U;
	uint8_t m_latch_1e001 = 0U;
	uint8_t m_latch_1f001 = 0U;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	required_shared_ptr<uint16_t> m_ram;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};


uint8_t ms9540_state::latch_1e001_r()
{
	return m_latch_1e001;
}

void ms9540_state::latch_1e001_w(uint8_t data)
{
	logerror("%s: $1e001 <- #$%02x\n", machine().describe_context(), data);
	m_latch_1e001 = data;
}

uint8_t ms9540_state::latch_1f001_r()
{
	return m_latch_1f001;
}

void ms9540_state::latch_1f001_w(uint8_t data)
{
	logerror("%s: $1f001 <- #$%02x\n", machine().describe_context(), data);
	m_latch_1f001 = data;
}

void ms9540_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x00ffff).ram().share("mainram");
	map(0x010000, 0x013fff).rom().region("9540", 0);
	map(0x018000, 0x018fff).ram();
	map(0x01e001, 0x01e001).rw(FUNC(ms9540_state::latch_1e001_r), FUNC(ms9540_state::latch_1e001_w));
	map(0x01f001, 0x01f001).rw(FUNC(ms9540_state::latch_1f001_r), FUNC(ms9540_state::latch_1f001_w));
}


/* Input ports */
static INPUT_PORTS_START( ms9540 )
INPUT_PORTS_END


void ms9540_state::machine_reset()
{
	uint8_t* ROM = memregion("9540")->base();
	memcpy(m_ram, ROM, 8);
}

void ms9540_state::machine_start()
{
	save_item(NAME(m_term_data));
	save_item(NAME(m_latch_1e001));
	save_item(NAME(m_latch_1f001));
}

void ms9540_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void ms9540_state::ms9540(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &ms9540_state::mem_map);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(ms9540_state::kbd_put));
}

/* ROM definition */
ROM_START( ms9540 )
	ROM_REGION16_BE(0x4000, "9540", 0)
	ROM_LOAD16_BYTE("0954-0135-01.20n", 0x00001, 0x2000, CRC(93ee9363) SHA1(73bc09e0379e06e0da96279cb5cc1581a0f0bf77) )
	ROM_LOAD16_BYTE("0954-0135-02.16n", 0x00000, 0x2000, CRC(a21077c5) SHA1(51dcbe543317d2042fb1acb1885461ba1790721e) )

	ROM_REGION(0x1800, "9520", 0)
	ROM_LOAD( "z80-hd.bin",   0x0000, 0x1000, CRC(b1c37286) SHA1(36b38fec9ef46e3e594423bbd1c64c5e9a4bc74d) )
	ROM_LOAD( "z80-flp.bin",  0x1000, 0x0800, CRC(f256b8c3) SHA1(780b444e999c3149eb0f137733703682d65746b4) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY               FULLNAME  FLAGS
COMP( 198?, ms9540, 0,      0,      ms9540,  ms9540, ms9540_state, empty_init, "Millennium Systems", "ms9540", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
