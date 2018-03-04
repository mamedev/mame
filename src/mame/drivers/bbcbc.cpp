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
#include "cpu/z80/z80daisy.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

class bbcbc_state : public driver_device
{
public:
	bbcbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_z80pio(*this, "z80pio")
		, m_buttons(*this, "BUTTONS.%u", 0)
	{ }

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(input_select_w);

	void bbcbc(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	uint8_t m_input_select;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_z80pio;
	required_ioport_array<3> m_buttons;
};


#define MAIN_CLOCK XTAL(4'433'619)


ADDRESS_MAP_START(bbcbc_state::mem_map)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0xbfff) AM_DEVREAD("cartslot", generic_slot_device, read_rom)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(bbcbc_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	;map(0x00, 0x7f).lrw8("z80pio_rw", [this](address_space &space, offs_t offset, u8 mem_mask){ return m_z80pio->read(space, offset >> 5, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask){ m_z80pio->write(space, offset >> 5, data, mem_mask); });
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("tms9129", tms9129_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_DEVREADWRITE("tms9129", tms9129_device, register_read, register_write)
ADDRESS_MAP_END

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


MACHINE_CONFIG_START(bbcbc_state::bbcbc)
	MCFG_CPU_ADD( "maincpu", Z80, MAIN_CLOCK / 8 )
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_Z80_DAISY_CHAIN(bbcbc_daisy_chain)

	MCFG_DEVICE_ADD("z80pio", Z80PIO, MAIN_CLOCK/8)
	//MCFG_Z80PIO_OUT_PA_CB(???)
	//MCFG_Z80PIO_IN_STROBE_CB(???)
	MCFG_Z80PIO_IN_PB_CB(READ8(bbcbc_state, input_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(bbcbc_state, input_select_w))

	MCFG_DEVICE_ADD( "tms9129", TMS9129, XTAL(10'738'635) / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(INPUTLINE("maincpu", 0))
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9129", tms9928a_device, screen_update )

	// Software on ROM cartridges
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "bbcbc_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list","bbcbc")
MACHINE_CONFIG_END


void bbcbc_state::machine_start()
{
	save_item(NAME(m_input_select));
}

void bbcbc_state::machine_reset()
{
	m_input_select = 0xff;
}

READ8_MEMBER(bbcbc_state::input_r)
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

WRITE8_MEMBER(bbcbc_state::input_select_w)
{
	m_input_select = data;
}


ROM_START( bbcbc )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("br_4_1.ic3", 0x0000, 0x2000, CRC(7c880d75) SHA1(954db096bd9e8edfef72946637a12f1083841fb0))
	ROM_LOAD("br_4_2.ic4", 0x2000, 0x2000, CRC(16a33aef) SHA1(9529f9f792718a3715af2063b91a5fb18f741226))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//   YEAR  NAME   PARENT  COMPAT  MACHINE INPUT  CLASS        INIT  COMPANY    FULLNAME                FLAGS
CONS(1985, bbcbc, 0,      0,      bbcbc,  bbcbc, bbcbc_state, 0,    "Unicard", "BBC Bridge Companion", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE)
