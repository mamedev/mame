// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Sandro Ronco, hap
// thanks-to:Berger, Sean Riddle
/*******************************************************************************

Fidelity CC10 / Fidelity ACR

TODO:
- What is cc10 8255 PB.7 for? When set, maximum levels is 3, like in CC3. But
  there is no CC3 with 16 buttons, and things get glitchy in this mode.

================================================================================

Fidelity Chess Challenger 10 (CCX)
----------------------------------
3 versions are known to exist: A,B,C. Strangely, version C(UCC10) has an 8080
instead of Z80 and no beeper, it's on CC1-based hardware (see cc1.cpp).

Z80A CPU @ 4MHz, NEC D8255C
4KB ROM(NEC 2332A), 2*256 bytes RAM(4*NEC 2111AL-4)
The beeper is via a 556 timer, fixed-frequency at around 1300-1400Hz.

Checker Challenger (ACR) is on the same PCB, twice less RAM and the beeper gone.
In the 1980s, Fidelity started naming it Checker Challenger "4" in some of their
advertisements, but box and manual still simply name it Checker Challenger.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_acr.lh"
#include "fidel_cc10.lh"


namespace {

class ccx_state : public driver_device
{
public:
	ccx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainmap(*this, "mainmap"),
		m_ppi8255(*this, "ppi8255"),
		m_display(*this, "display"),
		m_beeper_off(*this, "beeper_off"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// RE button is tied to Z80 RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	// machine configs
	void acr(machine_config &config);
	void ccx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_mainmap;
	required_device<i8255_device> m_ppi8255;
	required_device<pwm_display_device> m_display;
	optional_device<timer_device> m_beeper_off;
	optional_device<beep_device> m_beeper;
	required_ioport_array<4> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_led_select = 0;
	u8 m_7seg_data = 0;

	// address maps
	void acr_map(address_map &map) ATTR_COLD;
	void ccx_map(address_map &map) ATTR_COLD;
	void main_trampoline(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	u8 main_trampoline_r(offs_t offset);
	void main_trampoline_w(offs_t offset, u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off) { m_beeper->set_state(0); }

	// I/O handlers
	void update_display();
	void ppi_porta_w(u8 data);
	void ppi_portb_w(u8 data);
	u8 ppi_portc_r();
	void ppi_portc_w(u8 data);
};

void ccx_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_7seg_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void ccx_state::update_display()
{
	// 4 7segs + 2 leds
	m_display->matrix(m_led_select, m_7seg_data);
}

void ccx_state::ppi_porta_w(u8 data)
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

void ccx_state::ppi_portb_w(u8 data)
{
	// d0: lose led, d1: check(win) led
	// d2-d5: digit select
	m_led_select = bitswap<6>(data,0,1,5,4,3,2);
	update_display();
}

u8 ccx_state::ppi_portc_r()
{
	u8 data = 0;

	// d0-d3: multiplexed inputs (active low)
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data & 0xf;
}

void ccx_state::ppi_portc_w(u8 data)
{
	// d4-d7: input mux (inverted)
	m_inp_mux = ~data >> 4 & 0xf;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

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



/*******************************************************************************
    Input Ports
*******************************************************************************/

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
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("RESET") // is not on matrix IN.0 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, ccx_state, reset_button, 0)
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, ccx_state, reset_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ccx_state::acr(machine_config &config)
{
	// basic machine hardware
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

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_fidel_acr);
}

void ccx_state::ccx(machine_config &config)
{
	acr(config);

	// basic machine hardware
	m_mainmap->set_addrmap(AS_PROGRAM, &ccx_state::ccx_map);

	config.set_default_layout(layout_fidel_cc10);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 1360); // approximation, from 556 timer ic
	m_beeper->add_route(ALL_OUTPUTS, "speaker", 0.25);
	TIMER(config, "beeper_off").configure_generic(FUNC(ccx_state::beeper_off));
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

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



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1978, cc10,    0,      0,      ccx,     ccx,   ccx_state, empty_init, "Fidelity Electronics", "Chess Challenger \"10\" (model CCX, rev. B)", MACHINE_SUPPORTS_SAVE )
SYST( 1978, cc10a,   cc10,   0,      ccx,     ccx,   ccx_state, empty_init, "Fidelity Electronics", "Chess Challenger \"10\" (model CCX)", MACHINE_SUPPORTS_SAVE ) // aka version A

SYST( 1978, checkc4, 0,      0,      acr,     acr,   ccx_state, empty_init, "Fidelity Electronics", "Checker Challenger (model ACR, 4 levels)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
