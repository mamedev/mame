// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

        6809 Portable

        12/05/2009 Skeleton driver.
        21/09/2011 connected to terminal, notes added [Robbbert]

Chips used:
- 6809E CPU
- 6845 CRTC
- 6840 CTC
- 6551 ACIA Console
- 6551 ACIA Aux
- 6850 ACIA Unknown purpose
- uPD765 FDC
- 2764 8K ROM for CPU
- 2732 4K ROM for Chargen (not dumped)
- 6x 6264 RAM
- 3x 5516 RAM
- XTALs: 14.745MHz, 16MHz

So much for the official documentation.

In practice, it reads/writes to a terminal, and doesn't use most of the other
devices.

'maincpu' (F9DD): unmapped program memory write to 00F0 = 05 & FF
'maincpu' (F9E3): unmapped program memory read from 0001 & FF <----- these 2 are CLR 0001
'maincpu' (F9E3): unmapped program memory write to 0001 = 00 & FF
'maincpu' (F9E6): unmapped program memory read from 0005 & FF <----- these 2 are CLR 0005
'maincpu' (F9E6): unmapped program memory write to 0005 = 00 & FF
'maincpu' (F9E9): unmapped program memory write to 0002 = 0B & FF <-- these 2 are STD 0002
'maincpu' (F9E9): unmapped program memory write to 0003 = 1E & FF
'maincpu' (F9EC): unmapped program memory write to 0006 = 0B & FF <-- these 2 are STD 0006
'maincpu' (F9EC): unmapped program memory write to 0007 = 1E & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF <-- the remainder seems to be disk related
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA41): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA46): unmapped program memory write to 00F0 = 04 & FF
'maincpu' (FA82): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF <-- now it gives up & prints an error


**********************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/mos6551.h"
#include "machine/terminal.h"


class d6809_state : public driver_device
{
public:
	d6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	DECLARE_READ8_MEMBER( term_r );
	DECLARE_WRITE8_MEMBER( term_w );
	void kbd_put(u8 data);

	void d6809(machine_config &config);
	void mem_map(address_map &map);
private:
	uint8_t m_term_data;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

READ8_MEMBER( d6809_state::term_r )
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( d6809_state::term_w )
{
	if ((data > 0) && (data < 0x80))
		m_terminal->write(space, 0, data);
}

void d6809_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	// 00-FF is for various devices.
	map(0x0000, 0x0003).rw("acia1", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x0004, 0x0007).rw("acia2", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x00ff, 0x00ff).rw(this, FUNC(d6809_state::term_r), FUNC(d6809_state::term_w));
	map(0x1000, 0xdfff).ram();
	map(0xe000, 0xffff).rom().region("roms", 0);
}


/* Input ports */
static INPUT_PORTS_START( d6809 )
INPUT_PORTS_END


void d6809_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void d6809_state::machine_reset()
{
}


MACHINE_CONFIG_START(d6809_state::d6809)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", MC6809E, XTAL(14'745'600) / 8) // MC68B09EP
	MCFG_DEVICE_PROGRAM_MAP(mem_map)

	MCFG_DEVICE_ADD("acia1", MOS6551, XTAL(14'745'600) / 8) // uses Q clock
	MCFG_DEVICE_ADD("acia2", MOS6551, XTAL(14'745'600) / 8) // uses Q clock

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(PUT(d6809_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( d6809 )
	ROM_REGION( 0x2000, "roms", 0 )
	ROM_LOAD( "d6809.rom", 0x0000, 0x2000, CRC(2ceb40b8) SHA1(780111541234b4f0f781a118d955df61daa56e7e))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME         FLAGS
COMP( 1983, d6809, 0,      0,      d6809,   d6809, d6809_state, empty_init, "Dunfield", "6809 Portable", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
