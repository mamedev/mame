/***************************************************************************

        Ondra driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "includes/ondra.h"
#include "machine/ram.h"


static cassette_image_device *cassette_device_image(running_machine &machine)
{
	return machine.device<cassette_image_device>(CASSETTE_TAG);
}


READ8_MEMBER(ondra_state::ondra_keyboard_r)
{
	UINT8 retVal = 0x00;
	UINT8 ondra_keyboard_line = offset & 0x000f;
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8", "LINE9" };
	double valcas = (cassette_device_image(machine())->input());
	if ( valcas < 0.00) {
		retVal = 0x80;
	}
	if (ondra_keyboard_line > 9) {
		retVal |= 0x1f;
	} else {
		retVal |= ioport(keynames[ondra_keyboard_line])->read();
	}
	return retVal;
}

static void ondra_update_banks(running_machine &machine)
{
	ondra_state *state = machine.driver_data<ondra_state>();
	UINT8 *mem = state->memregion("maincpu")->base();
	if (state->m_bank1_status==0) {
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0x0000, 0x3fff);
		state->membank("bank1")->set_base(mem + 0x010000);
	} else {
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_write_bank(0x0000, 0x3fff, "bank1");
		state->membank("bank1")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0x0000);
	}
	state->membank("bank2")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0x4000);
	if (state->m_bank2_status==0) {
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xe000, 0xffff, "bank3");
		state->membank("bank3")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0xe000);
	} else {
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0xe000, 0xffff);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_handler (0xe000, 0xffff, read8_delegate(FUNC(ondra_state::ondra_keyboard_r),state));
	}
}

WRITE8_MEMBER(ondra_state::ondra_port_03_w)
{
	m_video_enable = data & 1;
	m_bank1_status = (data >> 1) & 1;
	m_bank2_status = (data >> 2) & 1;
	ondra_update_banks(machine());
	cassette_device_image(machine())->output(((data >> 3) & 1) ? -1.0 : +1.0);
}

WRITE8_MEMBER(ondra_state::ondra_port_09_w)
{
}

WRITE8_MEMBER(ondra_state::ondra_port_0a_w)
{
}

static TIMER_CALLBACK(nmi_check_callback)
{
	if ((machine.root_device().ioport("NMI")->read() & 1) == 1)
	{
		machine.device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

void ondra_state::machine_reset()
{
	m_bank1_status = 0;
	m_bank2_status = 0;
	ondra_update_banks(machine());
}

void ondra_state::machine_start()
{
	machine().scheduler().timer_pulse(attotime::from_hz(10), FUNC(nmi_check_callback));
}
