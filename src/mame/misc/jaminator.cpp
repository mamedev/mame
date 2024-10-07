// license: BSD-3-Clause
// copyright-holders: Devin Acker

/***************************************************************************
    Noise Toys Inc. "Jaminator"

    This toy guitar was originally sold by Worlds of Wonder in 1990, along with
    six optional ROM cartridges. In 1993, it was also licensed to Arrow Micro-Techs
    (AMT) and Yamaha, who distributed it along with several new cartridges.

    A message from the development team is used as ROM padding:
    """
    (C)1990 Noise Toys Inc
    Code by Steve Capps(MAD - Je t'aime beaucoup)
    Hardware by Ray DuFlon
    Music by Ed Bogas(Des - te amo)
    """

    Main hardware:
    U101: "DEVO" sound and mapper ASIC
          ("(C)1987 NOISE TOYS INC", "WOW DEVO 33073-01 CF61909N" or "AMT DEVO CF61909N")
    U102: OKI MSM80C39
    U104: 1Mbit mask ROM (DIP28, 23C1000 pinout)

    TODO:
    - Link cable
    - Clickable layout?

***************************************************************************/


#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/rescap.h"
#include "sound/cf61909.h"
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"

#include "softlist_dev.h"
#include "speaker.h"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class jaminator_state : public driver_device
{
public:
	jaminator_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_devo(*this, "devo"),
		m_cart(*this, "cart"),
		m_inputs(*this, "COL%u", 1), // labeling from PCB silkscreen
		m_bender(*this, "BENDER"),
		m_led_power(*this, "led_power")
	{ }

	void jaminator(machine_config &config);

	void input_sel_w(u8 data);
	ioport_value input_r();
	ioport_value bender_r();

	// link cable not emulated yet, but output needs to be looped back too (used for starting songs, etc)
	void link_data_w(u8 data) { m_link_data = data; }
	ioport_value link_data_r() { return m_link_data; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<i8039_device> m_maincpu;
	required_device<cf61909_device> m_devo;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<7> m_inputs;
	required_ioport m_bender;
	output_finder<> m_led_power;

	u8 m_input_sel;
	u8 m_link_data;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void jaminator_state::main_map(address_map &map)
{
	// TODO: program ROM banking for executable cartridges (do any exist?)
	map(0x000, 0x7ff).mirror(0x800).rom().region("devo", 0);
}

void jaminator_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(m_devo, FUNC(cf61909_device::read), FUNC(cf61909_device::write));
}

void jaminator_state::sound_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("devo", 0);
	map(0x20000, 0x3ffff).nopr(); // cart
}

//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( jaminator )
	PORT_START("COL1")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("String 1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("String 2")
	PORT_BIT(0x4, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("String 3")
	PORT_BIT(0x8, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Start / Next")

	PORT_START("COL2")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Key 1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Key 2")
	PORT_BIT(0x4, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Key 3")
	PORT_BIT(0x8, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Key 4")

	PORT_START("COL3")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Drum Pad 1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Drum Pad 2")
	PORT_BIT(0x4, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Drum Pad 3")
	PORT_BIT(0x8, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Finale")

	PORT_START("COL4")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_1) PORT_NAME("Fret 1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_2) PORT_NAME("Fret 2")
	PORT_BIT(0x4, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_3) PORT_NAME("Fret 3")
	PORT_BIT(0x8, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_4) PORT_NAME("Fret 4")

	PORT_START("COL5")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_5) PORT_NAME("Fret 5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_6) PORT_NAME("Fret 6")
	PORT_BIT(0x4, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_7) PORT_NAME("Fret 7")
	PORT_BIT(0x8, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_8) PORT_NAME("Fret 8")

	PORT_START("COL6")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_9) PORT_NAME("Fret 9")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_0) PORT_NAME("Fret 10")
	PORT_BIT(0x4, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Fret 11")
	PORT_BIT(0x8, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Fret 12")

	PORT_START("COL7")
	PORT_BIT(0xf, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(jaminator_state, bender_r)

	PORT_START("BENDER")
	PORT_BIT(0xff, 0x78, IPT_PADDLE) PORT_NAME("Bender Bar") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xef)

	PORT_START("P1")
	PORT_BIT(0x0f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(jaminator_state, input_r)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_OUTPUT ) // link cable clock
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(jaminator_state, link_data_w)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_BUTTON1) PORT_NAME("Select")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(jaminator_state, link_data_r)

	PORT_START("P2")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(jaminator_state, input_sel_w)

	/*
	* T0 is connected to pin 1 on the link port, which is pulled up by a 10k resistor.
	* Connecting it to ground causes percussion tracks to be omitted when playing songs
	*/
	PORT_START("T0")
	PORT_CONFNAME(0x1, 0x1, "Percussion Tracks")
	PORT_CONFSETTING(0x0, DEF_STR( Off ))
	PORT_CONFSETTING(0x1, DEF_STR( On ))
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void jaminator_state::machine_start()
{
	m_led_power.resolve();
	m_input_sel = 0;
	m_link_data = 0;

	if (m_cart->exists())
		m_devo->space().install_read_handler(0x20000, 0x3ffff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));

	save_item(NAME(m_input_sel));
	save_item(NAME(m_link_data));
}

//**************************************************************************
void jaminator_state::machine_reset()
{
	m_led_power = 1;
}

//**************************************************************************
void jaminator_state::input_sel_w(u8 data)
{
	m_input_sel = data & 0xf;

	if (m_input_sel == 0x7)
	{
		m_led_power = 0;
		m_devo->reset();
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}

//**************************************************************************
ioport_value jaminator_state::input_r()
{
	if (m_input_sel < 0x7)
		return m_inputs[m_input_sel]->read();

	return 0;
}

//**************************************************************************
ioport_value jaminator_state::bender_r()
{
	// the bender PCB only has 15 contact positions (0-14), but the ROM recognizes 16 values
	static const u8 bendval[] = {
		0xf, 0x7, 0x3, 0xb, 0x9, 0x1, 0x5, 0xd,
		0xc, 0x4, 0x0, 0x8, 0xa, 0x2, 0x6, 0xe
	};
	return bendval[m_bender->read() >> 4];
}

//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void jaminator_state::jaminator(machine_config &config)
{
	I8039(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &jaminator_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &jaminator_state::io_map);
	m_maincpu->p1_in_cb().set_ioport("P1");
	m_maincpu->p1_out_cb().set_ioport("P1");
	m_maincpu->p2_out_cb().set_ioport("P2");
	m_maincpu->t0_in_cb().set_ioport("T0");

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "jaminator", "bin");
	SOFTWARE_LIST(config, "cart_list").set_original("jaminator");

	SPEAKER(config, "speaker").front_center();

	CF61909(config, m_devo, 11_MHz_XTAL);
	m_devo->set_addrmap(0, &jaminator_state::sound_map);
	m_devo->add_route(0, "rcfilter", 1.0);
	m_maincpu->t1_in_cb().set(m_devo, FUNC(cf61909_device::sync_r));

	filter_rc_device &rcfilter(FILTER_RC(config, "rcfilter"));
	rcfilter.set_lowpass(RES_R(510) + RES_K(15), CAP_N(6.8));
	rcfilter.add_route(0, "biquad", 1.0);

	filter_biquad_device &biquad(FILTER_BIQUAD(config, "biquad"));
	biquad.opamp_sk_lowpass_setup(RES_K(10), RES_K(10), RES_K(39), RES_K(1), CAP_N(6.8), CAP_N(6.8));
	biquad.add_route(0, "speaker", 1.0);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( jaminator )
	ROM_REGION(0x20000, "devo", 0)
	ROM_LOAD("amta361.u104", 0x00000, 0x20000, CRC(f3f798ed) SHA1(08bef43e9689608f40a57b77724de5f6d2652693))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY            FULLNAME     FLAGS
SYST( 1990, jaminator, 0,      0,      jaminator, jaminator, jaminator_state, empty_init, "Noise Toys Inc.", "Jaminator", MACHINE_SUPPORTS_SAVE )
