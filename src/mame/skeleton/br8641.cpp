// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Brandt 8641

        Currency Counter

        2012-12-24 Skeleton driver. [Micko]
        2015-09-27 Added devices & inputs based entirely on guesswork [Robbbert]

        There seems to be 15 buttons (according to images, I just have board)
        also there are 8 dips currently set at 00011100 (1 is on)

        Looks like a LCD display? I think ports 8 & 9 write to it. Also looks
        like the display should say 8641 when machine is turned on.

ToDo:
- Need manuals, schematics, etc.
- Artwork
- Everything

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "speaker.h"


namespace {

class brandt8641_state : public driver_device
{
public:
	brandt8641_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
		, m_pio3(*this, "pio3")
		, m_ctc(*this, "ctc")
		, m_sio(*this, "sio")
		, m_io_keyboard(*this, "KEY.%u", 0U)
		, m_beep(*this, "beeper")
	{ }

	void brandt8641(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	u8 port08_r();
	void port08_w(u8 data);
	void port09_w(u8 data);
	u8 m_port08 = 0U;
	u8 m_port09 = 0U;
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	required_device<z80pio_device> m_pio3;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_ioport_array<8> m_io_keyboard;
	required_device<beep_device> m_beep;
};

void brandt8641_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom(); // 27256 at U12
	map(0x8000, 0x9fff).ram(); // 8KB static ram 6264 at U12
}

void brandt8641_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x04, 0x07).rw(m_pio1, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_pio2, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0f).rw(m_pio3, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x10, 0x13).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1c, 0x1f).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
}

/* Input ports */
// No idea what each key does
static INPUT_PORTS_START( brandt8641 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
INPUT_PORTS_END

u8 brandt8641_state::port08_r()
{
	u8 i, data = 7;

	for (i = 0; i < 8; i++)
		if (BIT(m_port09, i))
			data &= m_io_keyboard[i]->read();

	return data | m_port08;
}

void brandt8641_state::port08_w(u8 data)
{
	m_port08 = data & 0xf8;
	m_beep->set_state(BIT(data, 4));
}

void brandt8641_state::port09_w(u8 data)
{
	m_port09 = data ^ 0xff;
}

void brandt8641_state::machine_start()
{
	save_item(NAME(m_port08));
	save_item(NAME(m_port09));
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "pio1" },
	{ "pio2" },
	{ "pio3" },
	{ "ctc" },
	{ "sio" },
	{ nullptr }
};



void brandt8641_state::brandt8641(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000)); // U4 ,4MHz crystal on board
	m_maincpu->set_addrmap(AS_PROGRAM, &brandt8641_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &brandt8641_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 2000).add_route(ALL_OUTPUTS, "mono", 0.50);

	// Z80APIO U9
	// Z80APIO U14
	// Z80PIO U7 - unknown which is which
	Z80PIO(config, m_pio1, XTAL(4'000'000));
	m_pio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio2, XTAL(4'000'000));
	m_pio2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio2->in_pa_callback().set(FUNC(brandt8641_state::port08_r));
	m_pio2->out_pa_callback().set(FUNC(brandt8641_state::port08_w));
	m_pio2->out_pb_callback().set(FUNC(brandt8641_state::port09_w));

	Z80PIO(config, m_pio3, XTAL(4'000'000));
	m_pio3->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc, XTAL(4'000'000)); // Z80CTC U8
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio, XTAL(4'000'000)); // unknown type
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}

/* ROM definition */
ROM_START( br8641 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "v0911he.u11", 0x0000, 0x8000, CRC(59a16951) SHA1(893dba60ec8bfa391fb2d2a30db5d42d601f5eb9))
ROM_END

} // anonymous namespace


/* Driver */
COMP( 1986, br8641, 0, 0, brandt8641, brandt8641, brandt8641_state, empty_init, "Brandt", "Brandt 8641", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
