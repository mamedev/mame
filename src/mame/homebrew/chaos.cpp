// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

    Chaos2

    2010-04-08 Skeleton driver.
    2012-05-19 Connected to a terminal, system is usable [Robbbert]

    This is a homebrew system: http://koo.corpus.cam.ac.uk/chaos/

    There are no schematics or manuals, so the results might not be
    totally accurate.

    With the DOS config switch turned off, the only accepted input
    is a line starting with '&'. The actual commands are unknown.

    With DOS enabled, a large number of commands become available.
    These are:
    access, ask, ascdis, bpclr, bpset, close, control, copy, devfive, dir,
    end, exec, execute, fill, find, goto, if, input, let, list, load, lowercase,
    memdis, memset, open, port, read, reboot, runhex, run, save, type, typesl,
    verify.
    An example is: memdis 0 8 (memory dump starting at 0, show 8 lines)
    Don't try 'fill' - it fills all memory with zeroes, crashing the system.

    ToDo:
    - Connect up floppy disk (WD1771 fdc, 5.25", single density,
      no other info available)

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/terminal.h"


namespace {

class chaos_state : public driver_device
{
public:
	chaos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_terminal(*this, "terminal")
		, m_p_ram(*this, "ram")
		, m_maincpu(*this, "maincpu")
	{ }

	void chaos(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	u8 port1e_r();
	void port1f_w(u8 data);
	u8 port90_r();
	u8 port91_r();
	void kbd_put(u8 data);
	void data_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	u8 m_term_data = 0U;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<u8> m_p_ram;
	required_device<cpu_device> m_maincpu;
};


void chaos_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram().share("ram");
}

void chaos_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x1e, 0x1e).r(FUNC(chaos_state::port1e_r));
	map(0x1f, 0x1f).rw(FUNC(chaos_state::port90_r), FUNC(chaos_state::port1f_w));
	map(0x90, 0x90).r(FUNC(chaos_state::port90_r));
	map(0x91, 0x91).r(FUNC(chaos_state::port91_r));
	map(0x92, 0x92).w(m_terminal, FUNC(generic_terminal_device::write));
}

void chaos_state::data_map(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).noprw(); // stops error log filling up while using debug
}

/* Input ports */
static INPUT_PORTS_START( chaos )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Enable DOS")
	PORT_CONFSETTING(    0x01, DEF_STR(No))
	PORT_CONFSETTING(    0x00, DEF_STR(Yes))
INPUT_PORTS_END


// Port 1E - Bit 0 indicates key pressed, Bit 1 indicates ok to output

u8 chaos_state::port1e_r()
{
	return (m_term_data) ? 1 : 0;
}

void chaos_state::port1f_w(u8 data)
{
	// make the output readable on our terminal
	if (data == 0x09)
		return;
	else
	if (!data)
		data = 0x24;

	m_terminal->write(data);

	if (data == 0x0d)
		m_terminal->write(0x0a);
}

u8 chaos_state::port90_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

// Status port
// Bit 0 = L use ports 1E & 1F; H use ports 90 & 92
// Bit 3 = key pressed
// Bit 7 = ok to output

u8 chaos_state::port91_r()
{
	u8 ret = 0x80 | ioport("CONFIG")->read();
	ret |= (m_term_data) ? 8 : 0;
	return ret;
}

void chaos_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void chaos_state::machine_start()
{
	save_item(NAME(m_term_data));

	m_term_data = 0;
}

void chaos_state::machine_reset()
{
	// copy the roms into ram
	u8* ROM = memregion("roms")->base();
	memcpy(m_p_ram, ROM, 0x3000);
	memcpy(m_p_ram+0x7000, ROM+0x3000, 0x1000);
}

void chaos_state::chaos(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(1'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &chaos_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &chaos_state::io_map);
	m_maincpu->set_addrmap(AS_DATA, &chaos_state::data_map);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(chaos_state::kbd_put));
}

/* ROM definition */
ROM_START( chaos )
	ROM_REGION( 0x4000, "roms", 0 )
	ROM_LOAD( "chaos.001", 0x0000, 0x1000, CRC(3b433e72) SHA1(5b487337d71253d0e64e123f405da9eaf20e87ac))
	ROM_LOAD( "chaos.002", 0x1000, 0x1000, CRC(8b0b487f) SHA1(0d167cf3004a81c87446f2f1464e3debfa7284fe))
	ROM_LOAD( "chaos.003", 0x2000, 0x1000, CRC(5880db81) SHA1(29b8f1b03c83953f66464ad1fbbfe2e019637ce1))
	ROM_LOAD( "chaos.004", 0x3000, 0x1000, CRC(5d6839d6) SHA1(237f52f0780ac2e29d57bf06d0f7a982eb523084))
ROM_END

} // Anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY          FULLNAME   FLAGS
COMP( 1983, chaos, 0,      0,      chaos,   chaos, chaos_state, empty_init, "David Greaves", "Chaos 2", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
