// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu,Sandro Ronco,hap
// thanks-to:Berger
/******************************************************************************

Fidelity Chess Challenger 10 (CCX)
-------------------
4 versions are known to exist: A,B,C,D. Strangely, version C(UCC10) has an 8080
instead of Z80. Chess Challenger 1,3 and 7 also run on very similar hardware.

Z80A CPU @ 4MHz
4KB ROM(NEC 2332A), 2*256 bytes RAM(4*NEC 2111AL-4)

This is an earlier hardware upon which the VCC and UVC were based on;
The hardware is nearly the same; in fact the only significant differences are
the RAM being located in a different place, the lack of a speech chip, and
the connections to ports A and B on the PPI:

8255 connections:
-----------------
PA.0 - segment G (W)
PA.1 - segment F (W)
PA.2 - segment E (W)
PA.3 - segment D (W)
PA.4 - segment C (W)
PA.5 - segment B (W)
PA.6 - segment A (W)
PA.7 - 'beeper' direct speaker output (W)

The beeper is via a 556 timer, fixed-frequency at around 1300-1400Hz.
Not all hardware configurations include the beeper.

PB.0 - dot commons (W)
PB.1 - NC
PB.2 - digit 0, bottom dot (W)
PB.3 - digit 1, top dot (W)
PB.4 - digit 2 (W)
PB.5 - digit 3 (W)
PB.6 - NC
PB.7 - Mode select? (cc3 vs cc10, R)

Note: there is no CC3 with 16 buttons, and things get glitchy in this mode.

PC.0 - button row 0 (R)
PC.1 - button row 1 (R)
PC.2 - button row 2 (R)
PC.3 - button row 3 (R)
PC.4 - button column A (W)
PC.5 - button column B (W)
PC.6 - button column C (W)
PC.7 - button column D (W)

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/beep.h"
#include "speaker.h"

// internal artwork
#include "fidel_cc10.lh" // clickable


namespace {

class ccx_state : public fidelbase_state
{
public:
	ccx_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255"),
		m_beeper_off(*this, "beeper_off"),
		m_beeper(*this, "beeper")
	{ }

	void ccx(machine_config &config);

private:
	// devices/pointers
	required_device<i8255_device> m_ppi8255;
	required_device<timer_device> m_beeper_off;
	required_device<beep_device> m_beeper;

	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off) { m_beeper->set_state(0); }

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	void prepare_display();
	DECLARE_WRITE8_MEMBER(ppi_porta_w);
	DECLARE_WRITE8_MEMBER(ppi_portb_w);
	DECLARE_READ8_MEMBER(ppi_portc_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// misc handlers

void ccx_state::prepare_display()
{
	// 4 7seg leds (note: sel d0 for extra leds)
	u8 outdata = (m_7seg_data & 0x7f) | (m_led_select << 7 & 0x80);
	set_display_segmask(0xf, 0x7f);
	display_matrix(8, 4, outdata, m_led_select >> 2 & 0xf);
}


// I8255 PPI

WRITE8_MEMBER(ccx_state::ppi_porta_w)
{
	// d7: enable beeper on falling edge (555 monostable)
	if (~data & m_7seg_data & 0x80 && !m_beeper_off->enabled())
	{
		m_beeper->set_state(1);
		m_beeper_off->adjust(attotime::from_msec(80)); // duration is approximate
	}

	// d0-d6: digit segment data
	m_7seg_data = bitswap<8>(data,7,0,1,2,3,4,5,6);
	prepare_display();
}

WRITE8_MEMBER(ccx_state::ppi_portb_w)
{
	// d0,d2-d5: digit/led select
	m_led_select = data;
	prepare_display();
}

READ8_MEMBER(ccx_state::ppi_portc_r)
{
	// d0-d3: multiplexed inputs (active low)
	return ~read_inputs(4) & 0xf;
}

WRITE8_MEMBER(ccx_state::ppi_portc_w)
{
	// d4-d7: input mux (inverted)
	m_inp_mux = ~data >> 4 & 0xf;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void ccx_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x10ff).mirror(0x0f00).ram();
	map(0x3000, 0x30ff).mirror(0x0f00).ram();
}

void ccx_state::main_io(address_map &map)
{
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( ccx )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Speaker") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("RESET") // is not on matrix IN.0 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, ccx_state, reset_button, nullptr)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void ccx_state::ccx(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ccx_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &ccx_state::main_io);

	I8255(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(ccx_state::ppi_porta_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->in_pb_callback().set_constant(0); // 0x80 for '3 level mode'
	m_ppi8255->out_pb_callback().set(FUNC(ccx_state::ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(ccx_state::ppi_portc_r));
	m_ppi8255->tri_pb_callback().set_constant(0);
	m_ppi8255->out_pc_callback().set(FUNC(ccx_state::ppi_portc_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_cc10);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 1360); // approximation, from 556 timer ic
	m_beeper->add_route(ALL_OUTPUTS, "speaker", 0.25);
	TIMER(config, "beeper_off").configure_generic(FUNC(ccx_state::beeper_off));
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( cc10 ) // model CCX
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cn19053n_cc10b", 0x0000, 0x1000, CRC(afd3ca99) SHA1(870d09b2b52ccb8572d69642c59b5215d5fb26ab) ) // 2332
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME  PARENT CMP MACHINE  INPUT  STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1978, cc10, 0,      0, ccx,     ccx,   ccx_state, empty_init, "Fidelity Electronics", "Chess Challenger 10 (model CCX, rev. B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
