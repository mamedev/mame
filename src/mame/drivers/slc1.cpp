// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

2011-JUL-16 SLC1 skeleton driver [Robbbert]
2011-DEC-29 Working [Robbbert]

reference: http://www.jens-mueller.org/jkcemu/slc1.html

This computer is both a Z80 trainer, and a chess computer. The keyboard
    is different between the two. So by default, the chess number buttons
    are on the main keyboard, and the one for the trainer are on the numpad.

    There is no chess board attached. You supply your own and you sync the
    pieces and the computer instructions. The chess engine was copied from
    Fidelity's Sensory Chess Challenger 8.

    When started, it is in Chess mode. Press 11111 to switch to Trainer mode.

Hardware:
    4 Kbytes ROM in the address range 0000-0FFF
    1 Kbyte RAM in the address range 5000-53ff (user area starts at 5100)
    6-digit 7-segment display
    Busy LED
    Keyboard with 12 keys

Trainer Keys:
    0-7 : hexadecimal numbers
    Shift then 0-7 : Hexadecimal 8-F (decimal points will appear)
    ADR : enter an address to work with. After the 4 digits are entered,
          the data at that address shows, and you can modify the data.
    + (inc) : Enter the data into memory, and increment the address by 1.

Pasting:
    0-7 : as is
    8-F : X, then 0-7
    + : O
    - : XO
    ADR : Z

Test Paste:
    00000Z510011O22O33O44O55O66O77OX8X8OX9X9OZ5100
    Now press O to confirm the data has been entered.

TODO:
    - Make emulation more faithful, io_w doesn't make much sense from
      a TTL wiring point of view.
    - Pasting doesn't work, probably too slow.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "slc1.lh"


namespace {

class slc1_state : public driver_device
{
public:
	slc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_inputs(*this, "IN.%u", 0U)
		, m_display(*this, "digit%u", 0U)
		, m_busyled(*this, "busyled")
	{ }

	void slc1(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	u8 m_digit = 0;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<3> m_inputs;
	output_finder<8> m_display;
	output_finder<> m_busyled;
};


/***************************************************************************
    Display
***************************************************************************/

void slc1_state::io_w(offs_t offset, u8 data)
{
	bool const segonoff = BIT(data, 7);
	bool const busyled = BIT(data, 4);
	data &= 15;

	if (data < 8)
		m_digit = data;
	else if (data < 12)
	{
		m_speaker->level_w(BIT(data, 1));
		return;
	}
	else if (offset == 0x2f07)
		return;

	u8 segdata = m_display[m_digit];
	u8 const segnum  = offset & 7;
	u8 const segmask = 1 << segnum;

	if (segonoff)
		segdata |= segmask;
	else
		segdata &= ~segmask;

	m_display[m_digit] = segdata;

	m_busyled = busyled;
}


/***************************************************************************
    Keyboard
***************************************************************************/

u8 slc1_state::io_r(offs_t offset)
{
	u8 data = 0, upper = (offset >> 8) & 7;

	if (1)
	{
		if (upper == 3)
			data = m_inputs[0]->read();
		else
		if (upper == 4)
			data = m_inputs[1]->read();
		else
		if (upper == 5)
			data = m_inputs[2]->read();
	}

	return data << 4 ^ 0xff;
}


/***************************************************************************
    Machine
***************************************************************************/

void slc1_state::machine_start()
{
	m_display.resolve();
	m_busyled.resolve();

	save_item(NAME(m_digit));
}

void slc1_state::machine_reset()
{
}


/***************************************************************************
    Address Map
***************************************************************************/

void slc1_state::mem_map(address_map &map)
{
	map.global_mask(0x4fff);
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x43ff).ram().mirror(0xc00);
}

void slc1_state::io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(slc1_state::io_r), FUNC(slc1_state::io_w));
}


/**************************************************************************
    Keyboard Layout
***************************************************************************/

static INPUT_PORTS_START( slc1 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D 4 / T / 3 B") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C 3 / L / 2 A") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B 2 / S / 1 9") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A 1 / B / 0 8") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E 5 / D / 4 C INS") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F 6 / K / 5 D DEL") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G 7 / 6 E BL") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H 8 / 7 F GO") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C / Seq / BG") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A / +/- 1 / SS") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("St / Fu / DP") PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z / ADR / BP") PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('Z')
INPUT_PORTS_END


/***************************************************************************
    Machine driver
***************************************************************************/

void slc1_state::slc1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &slc1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &slc1_state::io_map);

	/* video hardware */
	config.set_default_layout(layout_slc1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************
    Game driver
***************************************************************************/

ROM_START(slc1)
	ROM_REGION(0x1000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "bios0", "SLC-1")
	ROMX_LOAD("slc1_0000.bin",   0x0000, 0x1000, CRC(06d32967) SHA1(f25eac66a4fca9383964d509c671a7ad2e020e7e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bios1", "SC-1 v2")
	ROMX_LOAD("sc1-v2.bin",      0x0000, 0x1000, CRC(1f122a85) SHA1(d60f89f8b59d04f4cecd6e3ecfe0a24152462a36), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY               FULLNAME */
COMP( 1989, slc1, 0,      0,      slc1,    slc1,  slc1_state, empty_init, "Dieter Scheuschner", "Schach- und Lerncomputer SLC 1", MACHINE_SUPPORTS_SAVE )
