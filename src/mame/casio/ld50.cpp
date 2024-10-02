// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*
    Casio LD-50 electronic drums

    Unlike most Casio instruments, this is either rebranded or outsourced.
    It's unclear who developed it, or if it was ever sold under other brands,
    but the data ROM mentions "SharpWin".

    Main board:
        bottom left:  "DIGITAL DRUM [LD-50] DATE: 2001/07/20"
        bottom right: "LD50 Main PCB Rev.1"

        IC1:   Burr-Brown PCM1717E DAC
        IC2:   Mitsubishi M62429FP volume control
        IC4:   Philips P87C52UBAA MCU
        IC5:   AMD AM29F040 ROM
        IC7/8/9/10: 4x 74HC374
        IC11:  Jess Technology HE80085 voice MCU (COB, letter "B" handwritten on epoxy)
               "256KB ROM 128B RAM, 24I/O, EXT, D/A, OP, PWM, 2X16BIT TIMER, WDT" (PIC clone or similar?)
               https://web.archive.org/web/20010824054714/http://www.jesstech.com/partslist.phtml?cat1=Microcontroller&cat2=8-Bits+MCU+Series&cat3=HE80+Series
        IC12:  Dream SAM9793 MIDI synth
        XTAL1: 9.6MHz (for SAM9793)
        XTAL2: 12MHz (for 87C52)

    A 10-pin header at the upper left connects to the LCD.

    Most of the sound is provided by the SAM9793, which is just a MIDI
    synth on a chip that takes in serial MIDI input and outputs I2S.
    The four sound effect pads also trigger PCM samples separately,
    via the black blob at IC11.

    The external ROM contains the demo and rhythms, which are all stored
    as standard type 0 MIDI files.

    To activate the test mode, hold "Rhythm" and "Assign" when powering on.
    From here, hitting the drum pads will display time/velocity measurements.

    Service manual and schematics:
    https://archive.org/details/casio-ld-50-sm

    TODO:
    - LCD artwork
    - clickable layout?
    - possibly connect a MIDI out port in lieu of the SAM9793
      (MCS51 core needs proper serial output first)
    - dump/emulate the HE80085 somehow
 */

#include "emu.h"

#include "bus/midi/midioutport.h"
#include "cpu/mcs51/mcs51.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class ld50_state : public driver_device
{
public:
	ld50_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_datarom(*this, "datarom")
		, m_inputs(*this, "IN%u", 0U)
		, m_pads(*this, "PADS")
		, m_dial(*this, "DIAL")
		, m_led(*this, "led%d", 0U)
	{
	}

	void ld50(machine_config &config);

	ioport_value dial_r();

private:
	u8 port0_r();
	void port0_w(u8 data);
	u8 port1_r();
	void port2_w(u8 data);
	void port3_w(u8 data);

	virtual void driver_start() override;
	virtual void driver_reset() override;

	HD44780_PIXEL_UPDATE(lcd_update);
	void palette_init(palette_device &palette);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;

	required_memory_region m_datarom;
	required_ioport_array<5> m_inputs;
	required_ioport m_pads;
	required_ioport m_dial;

	output_finder<4> m_led;

	u8 m_port[4];
	u32 m_rom_addr;
	u16 m_sound_data;
	u16 m_volume_data;
};

HD44780_PIXEL_UPDATE(ld50_state::lcd_update)
{
	if (x < 6 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(y, line * 48 + pos * 6 + x) = state;
}

void ld50_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(255, 255, 255));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}


u8 ld50_state::port0_r()
{
	u8 data = 0xf0 | (m_lcdc->db_r() >> 4);
	if (!BIT(m_port[2], 3))
		data &= m_datarom->base()[m_rom_addr & 0x7ffff];

	return data;
}

void ld50_state::port0_w(u8 data)
{
	m_lcdc->db_w(data << 4);
	m_port[0] = data;
}

u8 ld50_state::port1_r()
{
	/*
	bits 0-3: drum pads (active high)
	bits 4-7: multiplexed inputs (active low)

	The drum pads are read during the timer 0 interrupt.
	Note velocity is based on the number of timer intervals that the pad remains pressed.
	*/
	return (m_port[1] & 0xf0) | m_pads->read();
}

void ld50_state::port2_w(u8 data)
{
	/*
	bit 0: reset output
	bit 1: unused
	bit 2: auto power off
	bit 3: ROM output enable
	bit 4: ROM address low byte latch
	bit 5: ROM address mid byte latch
	bit 6: ROM address hi byte & input latch
	bit 7: LED and LCD control latch
	*/

	const u8 set = data & ~m_port[2];
	m_port[2] = data;

	if (BIT(set, 4))
	{
		m_rom_addr = (m_rom_addr & 0x7ff00) | m_port[0];
	}
	if (BIT(set, 5))
	{
		m_rom_addr = (m_rom_addr & 0x700ff) | (m_port[0] << 8);
	}
	if (BIT(set, 6))
	{
		m_rom_addr = (m_rom_addr & 0x0ffff) | ((m_port[0] & 0xe0) << 11);

		m_port[1] |= 0xf0;
		for (int i = 0; i < 5; i++)
		{
			if (!BIT(m_port[0], i))
				m_port[1] &= (m_inputs[i]->read() << 4);
		}
	}
	if (BIT(set, 7))
	{
		for (unsigned i = 0; i < 4; i++)
			m_led[i] = !BIT(m_port[0], i);

		m_lcdc->rw_w(BIT(m_port[0], 6));
		m_lcdc->rs_w(BIT(m_port[0], 7));
	}
}

void ld50_state::port3_w(u8 data)
{
	/*
	bit 0: LCD enable
	bit 1: MIDI Tx (TODO)
	bit 2: unused
	bit 3: trigger sound effect
	bit 4: shift register clock (for volume)
	bit 5: shift register data
	bit 6: shift register clock (for DAC and sound effects)
	bit 7: DAC data latch

	The LD-50 transmits MIDI both through the UART registers and manually via port 3.
	Demo/rhythm playback uses the UART, while drum pads, effect values, etc. are
	bit-banged through the port instead (why?)
	*/

	const u8 set = data & ~m_port[3];
	const u8 clr = ~data & m_port[3];
	m_port[3] = data;

	m_lcdc->e_w(BIT(data, 0));

	if (BIT(set, 3))
	{
		logerror("play sound effect %u\n", m_sound_data & 0x1f);
	}
	if (BIT(set, 4))
	{
		// 11-bit data, least significant bit first
		m_volume_data >>= 1;
		m_volume_data |= (BIT(data, 5) << 15);
	}
	if (BIT(clr, 4) && BIT(data, 5))
	{
		logerror("volume cmd (data = 0x%03x)\n", m_volume_data >> 5);
	}
	if (BIT(set, 6))
	{
		// 16-bit data, most significant bit first
		m_sound_data <<= 1;
		m_sound_data |= BIT(data, 5);
	}
	if (BIT(clr, 7))
	{
		logerror("DAC cmd (data = 0x%04x)\n", m_sound_data);
	}
}

ioport_value ld50_state::dial_r()
{
	// return the dial position as a 2-bit gray code
	const u8 val = m_dial->read();
	return (val >> 6) ^ (val >> 7);
}


void ld50_state::driver_start()
{
	m_led.resolve();

	save_item(NAME(m_port));
	save_item(NAME(m_rom_addr));
	save_item(NAME(m_sound_data));
	save_item(NAME(m_volume_data));
}

void ld50_state::driver_reset()
{
	memset(m_port, 0xff, sizeof m_port);

	m_rom_addr = 0;
	m_sound_data = 0;
	m_volume_data = 0;
}

void ld50_state::ld50(machine_config &config)
{
	// CPU
	I87C52(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->port_in_cb<0>().set(FUNC(ld50_state::port0_r));
	m_maincpu->port_out_cb<0>().set(FUNC(ld50_state::port0_w));
	m_maincpu->port_in_cb<1>().set(FUNC(ld50_state::port1_r));
	m_maincpu->port_out_cb<2>().set(FUNC(ld50_state::port2_w));
	m_maincpu->port_out_cb<3>().set(FUNC(ld50_state::port3_w));

	// LCD
	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(ld50_state::lcd_update));

	// screen (for testing only)
	// TODO: the actual LCD with custom segments
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6 * 16, 8);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ld50_state::palette_init), 2);
}

INPUT_PORTS_START(ld50)
	PORT_START("PADS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Drum Pad 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Drum Pad 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Drum Pad 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Drum Pad 1")

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_REVERSE

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("SE Pad 4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("SE Pad 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("SE Pad 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("SE Pad 1")

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(ld50_state, dial_r)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Effect")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS)    PORT_NAME("Assign")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS)     PORT_NAME("Drum Set")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0)         PORT_NAME("SE Set")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9)         PORT_NAME("Rhythm")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8)         PORT_NAME("Play Level")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7)         PORT_NAME("Light")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6)         PORT_NAME("Tempo")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5)         PORT_NAME("Start/Stop")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4)         PORT_NAME("Synchro Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3)         PORT_NAME("Demo")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2)         PORT_NAME("Rhythm Vol.")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1)         PORT_NAME("Main Vol.")
INPUT_PORTS_END

ROM_START(ld50)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("87c52.ic4", 0x0000, 0x2000, CRC(126108cc) SHA1(cb8d7359628f8e519862cec73e38078275c3bd48))

	ROM_REGION(0x80000, "datarom", 0)
	ROM_LOAD("ld50.ic5", 0x00000, 0x80000, CRC(acaee847) SHA1(7235aefd72260b6d9d1f652c643022515a880781))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME     FLAGS
SYST( 2002, ld50,    0,      0,      ld50,    ld50,    ld50_state,    empty_init, "Casio", "LD-50",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
