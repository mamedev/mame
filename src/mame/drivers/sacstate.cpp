// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        SacState 8008

        23/02/2009 Skeleton driver.

http://www.digibarn.com/stories/bill-pentz-story/index.html

The year is listed as '1972/73'.

All input to be in lowercase.
The weird characters that show on screen are various escape sequences.
These are different depending on the terminal type chosen. The codes
need to be understood and emulated before this system makes sense.

Known Monitor commands: (from the disassembly)
!   write buffer to AD000
*   set RAM036 flag
+   increment AD000 by 1
+n  increment AD000 by n
-   decrement AD000 by 1
-n  decrement AD000 by n
:   clear RAM036 flag
=   display AD000
=nnn    set AD000 to nnn
@   fill buffer with 026

unknown commands: / & d e l r s t u z \ ^ | ~

Other input will either result in '!' message, or halt.


****************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"
#include "machine/terminal.h"


class sacstate_state : public driver_device
{
public:
	sacstate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	void sacstate(machine_config &config);

private:
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	void kbd_put(u8 data);
	void sacstate_io(address_map &map);
	void sacstate_mem(address_map &map);

	uint8_t m_term_data;
	uint8_t m_val;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

READ8_MEMBER( sacstate_state::port01_r )
{
	uint8_t ret = m_val;
	if (m_term_data)
		ret |= 0x04; // data in
	return ret;
}

READ8_MEMBER( sacstate_state::port00_r )
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( sacstate_state::port04_r )
{
	logerror("unknown_r\n");
	return 0;
}

WRITE8_MEMBER( sacstate_state::port08_w )
{
	if (data == 0x40)
		m_val = 0x40;
	else
	if (data == 0x04)
		m_val = 0;
}

void sacstate_state::sacstate_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x7ff).rom();
	map(0x800, 0xfff).ram();
}

void sacstate_state::sacstate_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).r(FUNC(sacstate_state::port00_r));
	map(0x01, 0x01).r(FUNC(sacstate_state::port01_r));
	map(0x04, 0x04).r(FUNC(sacstate_state::port04_r));
	map(0x08, 0x08).w(FUNC(sacstate_state::port08_w));
	map(0x16, 0x16).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x17, 0x1f).nopw();
}

/* Input ports */
static INPUT_PORTS_START( sacstate )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x08, 0x08, "Terminal Type") // no idea of actual terminal types
	PORT_CONFSETTING(    0x00, "A")
	PORT_CONFSETTING(    0x08, "B")
INPUT_PORTS_END

void sacstate_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void sacstate_state::machine_reset()
{
	m_term_data = 0;
	m_val = ioport("CONFIG")->read();
}

void sacstate_state::sacstate(machine_config &config)
{
	/* basic machine hardware */
	I8008(config, m_maincpu, 800000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sacstate_state::sacstate_mem);
	m_maincpu->set_addrmap(AS_IO, &sacstate_state::sacstate_io);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(sacstate_state::kbd_put));
}

/* ROM definition */
ROM_START( sacstate )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sacst1.bin", 0x0700, 0x0100, CRC(ba020160) SHA1(6337cdf65583808768664653c937e50040aec6d4))
	ROM_LOAD( "sacst2.bin", 0x0600, 0x0100, CRC(26f3e505) SHA1(3526060dbd1bf885c2e686bc9a6082387630952a))
	ROM_LOAD( "sacst3.bin", 0x0500, 0x0100, CRC(965b3474) SHA1(6d9142e68d375fb000fd6ea48369d0801274ded6))
	ROM_LOAD( "sacst4.bin", 0x0400, 0x0100, CRC(3cd3e169) SHA1(75a99e8e4dbd6e054209a4979bb498f37e962697))
	ROM_LOAD( "sacst5.bin", 0x0300, 0x0100, CRC(30619454) SHA1(cb498880bec27c9adc44dc1267858555000452c6))
	ROM_LOAD( "sacst6.bin", 0x0200, 0x0100, CRC(a4cd2ff6) SHA1(3f4da5510c0778eb770c96c01f91f5cb7f5285fa))
	ROM_LOAD( "sacst7.bin", 0x0100, 0x0100, CRC(33971d8b) SHA1(9e0bbeef6a6a15107f270e8b285300284ee7f63f))
	ROM_LOAD( "sacst8.bin", 0x0000, 0x0100, CRC(931252ef) SHA1(e06ea6947f432f0a4ce944de74978d929920fb53))
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME         FLAGS
COMP( 1973, sacstate, 0,      0,      sacstate, sacstate, sacstate_state, empty_init, "SacState", "SacState 8008", MACHINE_NO_SOUND_HW )
