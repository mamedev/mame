// license:BSD-3-Clause
// copyright-holders:Incog, Robbbert
/***************************************************************************

  bbcbc.c

  BBC Bridge Companion

  Inputs hooked up - 2009-03-14 - Robbbert
  Clock Freq added - 2009-05-18 - incog

  From the Operations Manual, Page 1:

    The BBC BRIDGE COMPANION and its associated family of
    Cartridges are distributed in the U.K. by:

    CONTEMPORARY CHESS COMPUTERS
    2/3 Noble Corner
    Great West Road
    HOUNSLOW
    Middlesex
    TW5 0PA

    (C) 1985 Unicard Ltd.

    The title BBC Bridge is under licence from BBC Enterprises Ltd.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "machine/z80pio.h"
#include "machine/z80daisy.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"


namespace {

class bbcbc_state : public driver_device
{
public:
	bbcbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_z80pio(*this, "z80pio")
		, m_buttons(*this, "BUTTONS.%u", 0U)
	{ }

	void bbcbc(machine_config &config);

private:
	uint8_t input_r();
	void input_select_w(uint8_t data);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	uint8_t m_input_select = 0U;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_z80pio;
	required_ioport_array<3> m_buttons;
};


#define MAIN_CLOCK XTAL(4'433'619)


void bbcbc_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xbfff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0xe000, 0xe7ff).ram();
}

void bbcbc_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).lrw8(
						 NAME([this](offs_t offset) { return m_z80pio->read(offset >> 5); }),
						 NAME([this](offs_t offset, u8 data) { m_z80pio->write(offset >> 5, data); }));
	map(0x80, 0x81).rw("tms9129", FUNC(tms9129_device::read), FUNC(tms9129_device::write));
}

// Input bits are read through the PIO four at a time, then stored individually in RAM at E030-E03B
static INPUT_PORTS_START( bbcbc )
	PORT_START("BUTTONS.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pass") PORT_CODE(KEYCODE_A) // Grey button
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Spades") PORT_CODE(KEYCODE_Z) // Grey button
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Clubs") PORT_CODE(KEYCODE_V) // Grey button
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Rdbl") PORT_CODE(KEYCODE_F) // Grey button
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("BUTTONS.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("NT") PORT_CODE(KEYCODE_S) // Grey button
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Hearts, Up") PORT_CODE(KEYCODE_X) // Grey button
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Play, Yes") // Yellow button
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Back") // Red button
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("BUTTONS.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Dbl") PORT_CODE(KEYCODE_D) // Grey button
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Diamonds, Down") PORT_CODE(KEYCODE_C) // Grey button
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start") // Red button
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Play, No") // Yellow button
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


static const z80_daisy_config bbcbc_daisy_chain[] =
{
	{ "z80pio" },
	{ nullptr }
};


void bbcbc_state::bbcbc(machine_config &config)
{
	Z80(config, m_maincpu, 10.6875_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcbc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &bbcbc_state::io_map);
	m_maincpu->set_daisy_config(bbcbc_daisy_chain);

	Z80PIO(config, m_z80pio, 10.6875_MHz_XTAL / 3);
	//m_z80pio->out_pa_callback().set(???);
	m_z80pio->in_pb_callback().set(FUNC(bbcbc_state::input_r));
	m_z80pio->out_pb_callback().set(FUNC(bbcbc_state::input_select_w));

	tms9129_device &vdp(TMS9129(config, "tms9129", 10.6875_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// Software on ROM cartridges
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "bbcbc_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("bbcbc");
}


void bbcbc_state::machine_start()
{
	save_item(NAME(m_input_select));
}

void bbcbc_state::machine_reset()
{
	m_input_select = 0xff;
}

uint8_t bbcbc_state::input_r()
{
	switch (m_input_select)
	{
		case 0xef:
			return m_buttons[0]->read();
		case 0xdf:
			return m_buttons[1]->read();
		case 0xbf:
			return m_buttons[2]->read();
	}
	logerror("Unknown input select: %02x\n", m_input_select);
	return 0xff;
}

void bbcbc_state::input_select_w(uint8_t data)
{
	m_input_select = data;
}


ROM_START( bbcbc )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("br_4_1.ic3", 0x0000, 0x2000, CRC(7c880d75) SHA1(954db096bd9e8edfef72946637a12f1083841fb0))
	ROM_LOAD("br_4_2.ic4", 0x2000, 0x2000, CRC(16a33aef) SHA1(9529f9f792718a3715af2063b91a5fb18f741226))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME                FLAGS
CONS(1985, bbcbc, 0,      0,      bbcbc,   bbcbc, bbcbc_state, empty_init, "Unicard", "BBC Bridge Companion", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
