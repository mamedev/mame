// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Sandro Ronco, hap
// thanks-to:Berger, Sean Riddle
/******************************************************************************

* fidel_cc10.cpp, subdriver of machine/fidelbase.cpp, machine/chessbase.cpp

TODO:
- What is cc10 8255 PB.7 for? When set, maximum levels is 3, like in CC3. But
  there is no CC3 with 16 buttons, and things get glitchy in this mode.

*******************************************************************************

Fidelity Chess Challenger 10 (CCX)
-------------------
3 versions are known to exist: A,B,C. Strangely, version C(UCC10) has an 8080
instead of Z80 and no beeper, it's on CC1-based hardware.

Z80A CPU @ 4MHz, NEC D8255C
4KB ROM(NEC 2332A), 2*256 bytes RAM(4*NEC 2111AL-4)
The beeper is via a 556 timer, fixed-frequency at around 1300-1400Hz.

Checker Challenger 4 (ACR) is on the same PCB, twice less RAM and the beeper gone.

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/beep.h"
#include "speaker.h"

// internal artwork
#include "fidel_acr.lh" // clickable
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

	// machine drivers
	void acr(machine_config &config);
	void ccx(machine_config &config);

private:
	// devices/pointers
	required_device<i8255_device> m_ppi8255;
	optional_device<timer_device> m_beeper_off;
	optional_device<beep_device> m_beeper;

	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off) { m_beeper->set_state(0); }

	// address maps
	void acr_map(address_map &map);
	void ccx_map(address_map &map);
	void main_trampoline(address_map &map);
	void main_io(address_map &map);

	u8 main_trampoline_r(offs_t offset);
	void main_trampoline_w(offs_t offset, u8 data);

	// I/O handlers
	void update_display();
	DECLARE_WRITE8_MEMBER(ppi_porta_w);
	DECLARE_WRITE8_MEMBER(ppi_portb_w);
	DECLARE_READ8_MEMBER(ppi_portc_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// misc handlers

void ccx_state::update_display()
{
	// 4 7segs + 2 leds
	set_display_segmask(0xf, 0x7f);
	display_matrix(8, 6, m_7seg_data, m_led_select);
}


// I8255 PPI

WRITE8_MEMBER(ccx_state::ppi_porta_w)
{
	// d7: enable beeper on falling edge (556 monostable) (unpopulated on ACR)
	if (m_beeper != nullptr && ~data & m_7seg_data & 0x80 && !m_beeper_off->enabled())
	{
		m_beeper->set_state(1);
		m_beeper_off->adjust(attotime::from_msec(80)); // duration is approximate
	}

	// d0-d6: digit segment data
	m_7seg_data = bitswap<8>(data,7,0,1,2,3,4,5,6);
	update_display();
}

WRITE8_MEMBER(ccx_state::ppi_portb_w)
{
	// d0: lose led, d1: check(win) led
	// d2-d5: digit select
	m_led_select = bitswap<6>(data,0,1,5,4,3,2);
	update_display();
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

void ccx_state::acr_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("maincpu", 0); // _A12
	map(0x2000, 0x20ff).mirror(0x0f00).ram(); // A13
}

void ccx_state::ccx_map(address_map &map)
{
	acr_map(map);
	map(0x4000, 0x40ff).mirror(0x0f00).ram(); // A14
}

// PCB design is prone to bus conflicts, but assuming software behaves fine,
// it will access ROM at $0xxx, RAM at $3xxx and $5xxx
void ccx_state::main_trampoline_w(offs_t offset, u8 data)
{
	if (offset & 0x2000)
		m_mainmap->write8(offset & 0x2fff, data);
	if (offset & 0x4000)
		m_mainmap->write8(offset & 0x4fff, data);
}

u8 ccx_state::main_trampoline_r(offs_t offset)
{
	u8 data = 0xff;
	if (~offset & 0x1000)
		data &= m_mainmap->read8(offset & 0x0fff);
	if (offset & 0x2000)
		data &= m_mainmap->read8(offset & 0x2fff);
	if (offset & 0x4000)
		data &= m_mainmap->read8(offset & 0x4fff);

	return data;
}

void ccx_state::main_trampoline(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).rw(FUNC(ccx_state::main_trampoline_r), FUNC(ccx_state::main_trampoline_w));
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("RESET") // is not on matrix IN.0 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, ccx_state, reset_button, nullptr)
INPUT_PORTS_END

static INPUT_PORTS_START( acr )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("TO") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)

	PORT_START("RESET") // is not on matrix IN.0 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, ccx_state, reset_button, nullptr)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void ccx_state::acr(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ccx_state::main_trampoline);
	ADDRESS_MAP_BANK(config, m_mainmap).set_map(&ccx_state::acr_map).set_options(ENDIANNESS_LITTLE, 8, 16);
	m_maincpu->set_addrmap(AS_IO, &ccx_state::main_io);

	I8255(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(ccx_state::ppi_porta_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->in_pb_callback().set_constant(0);
	m_ppi8255->out_pb_callback().set(FUNC(ccx_state::ppi_portb_w));
	m_ppi8255->tri_pb_callback().set_constant(0);
	m_ppi8255->in_pc_callback().set(FUNC(ccx_state::ppi_portc_r));
	m_ppi8255->out_pc_callback().set(FUNC(ccx_state::ppi_portc_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(ccx_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_acr);
}

void ccx_state::ccx(machine_config &config)
{
	acr(config);

	/* basic machine hardware */
	m_mainmap->set_addrmap(AS_PROGRAM, &ccx_state::ccx_map);

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

ROM_START( cc10 ) // model CCX, PCB label P241C-1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cn19053n_cc10b", 0x0000, 0x1000, CRC(afd3ca99) SHA1(870d09b2b52ccb8572d69642c59b5215d5fb26ab) ) // 2332
ROM_END

ROM_START( cc10a ) // model CCX
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cc-10", 0x0000, 0x1000, CRC(853db345) SHA1(36799e204eb0aef915eb483a5f43372fca5a0fc0) ) // MCM68A332P
ROM_END


ROM_START( checkc4 ) // model ACR, PCB label P241C
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d2332c_043", 0x0000, 0x1000, CRC(4c251d90) SHA1(474d54b05971f2a3208bab56dc6e27f03781c541) ) // no custom label
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT CMP MACHINE  INPUT  STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1978, cc10,    0,      0, ccx,     ccx,   ccx_state, empty_init, "Fidelity Electronics", "Chess Challenger 10 (model CCX, rev. B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1978, cc10a,   cc10,   0, ccx,     ccx,   ccx_state, empty_init, "Fidelity Electronics", "Chess Challenger 10 (model CCX)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // aka version A

CONS( 1978, checkc4, 0,      0, acr,     acr,   ccx_state, empty_init, "Fidelity Electronics", "Checker Challenger 4", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW )
