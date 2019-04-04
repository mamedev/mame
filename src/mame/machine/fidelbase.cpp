// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Fidelity Electronics (mostly-)chess computers base driver

TODO: (see each driver for more specific)
- verify cpu speed and rom labels where uncertain
- support for printer
- improve EAS/SC12/etc CPU divider? it seems a little bit slower than the real machine.
  Currently, a dummy timer workaround is needed, or it's much worse.
  Is the problem here due to timing of CPU addressbus changes? We can only 'sense'
  the addressbus at read or write accesses.

Keypad legend:
- RE: Reset
- CL: Clear
- EN: Enter
- PB: Problem Mode
- PV: Position Verification
- LV: Playing Levels
- TB: Take Back
- DM: Display Move/Double Move
- RV: Reverse
- ST: Set/Stop
- TM: Time

Read the official manual(s) on how to play.

Peripherals, compatible with various boards:
- Fidelity Challenger Printer - thermal printer, MCU=D8048C243

Program/data cartridges, for various boards, some cross-compatible:
- CB9: Challenger Book Openings 1 - 8KB (label not known)
- CB16: Challenger Book Openings 2 - 8+8KB 101-1042A01,02
- *CG64: 64 Greatest Games
- *EOA-EOE: Challenger Book Openings - Chess Encyclopedia A-E (5 modules)

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"


// machine start/reset

void fidelbase_state::machine_start()
{
	chessbase_state::machine_start();

	// zerofill/register for savestates
	m_speech_data = 0;
	m_speech_bank = 0;
	m_div_config = 0;

	save_item(NAME(m_speech_data));
	save_item(NAME(m_speech_bank));
	save_item(NAME(m_div_status));
	save_item(NAME(m_div_config));
	save_item(NAME(m_div_scale));

	// dummy timer for cpu divider
	m_div_timer = machine().scheduler().timer_alloc(timer_expired_delegate(), this);
}

void fidelbase_state::machine_reset()
{
	chessbase_state::machine_reset();

	// init cpu divider (optional)
	div_refresh();
}


/***************************************************************************
    Helper Functions
***************************************************************************/

// cartridge

DEVICE_IMAGE_LOAD_MEMBER(fidelbase_state, scc_cartridge)
{
	u32 size = m_cart->common_get_size("rom");

	// max size is 16KB?
	if (size > 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid file size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

READ8_MEMBER(fidelbase_state::cartridge_r)
{
	if (m_cart->exists())
		return m_cart->read_rom(offset);
	else
		return 0;
}


// Offset-dependent CPU divider on some 6502-based machines

void fidelbase_state::div_set_cpu_freq(offs_t offset)
{
	if (offset != m_div_status)
	{
		// when a13/a14 is high, XTAL goes through divider(s)
		// (depending on factory-set jumper, either one or two 7474)
		m_maincpu->set_clock_scale(offset ? m_div_scale : 1.0);

		m_div_status = offset;
	}
}

void fidelbase_state::div_trampoline_w(offs_t offset, u8 data)
{
	if (m_div_config)
		div_set_cpu_freq(offset & 0x6000);

	m_mainmap->write8(offset, data);
}

u8 fidelbase_state::div_trampoline_r(offs_t offset)
{
	if (m_div_config && !machine().side_effects_disabled())
		div_set_cpu_freq(offset & 0x6000);

	return m_mainmap->read8(offset);
}

void fidelbase_state::div_trampoline(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(fidelbase_state::div_trampoline_r), FUNC(fidelbase_state::div_trampoline_w));
}

void fidelbase_state::div_refresh(ioport_value val)
{
	if (val == 0xff)
	{
		// bail out if there is no cpu divider
		ioport_port *inp = ioport("div_config");
		if (inp == nullptr)
			return;

		val = inp->read();
	}

	m_maincpu->set_clock_scale(1.0);
	m_div_status = ~0;
	m_div_config = val;
	m_div_scale = (m_div_config & 1) ? 0.25 : 0.5;

	// stop high frequency background timer if cpu divider is disabled
	attotime period = (val) ? attotime::from_hz(m_maincpu->clock()) : attotime::never;
	m_div_timer->adjust(period, 0, period);
}

INPUT_PORTS_START( fidel_cpu_div_2 )
	PORT_START("div_config") // hardwired, default to /2
	PORT_CONFNAME( 0x03, 0x02, "CPU Divider" ) PORT_CHANGED_MEMBER(DEVICE_SELF, fidelbase_state, div_changed, nullptr)
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "4" )
INPUT_PORTS_END

INPUT_PORTS_START( fidel_cpu_div_4 )
	PORT_START("div_config") // hardwired, default to /4
	PORT_CONFNAME( 0x03, 0x03, "CPU Divider" ) PORT_CHANGED_MEMBER(DEVICE_SELF, fidelbase_state, div_changed, nullptr)
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "4" )
INPUT_PORTS_END
