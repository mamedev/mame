// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard
/******************************************************************************

Fidelity Sensory Chess Challenger 6 (model SC6):
- PCB label 510-1045B01
- INS8040N-11 MCU, 11MHz XTAL
- external 4KB ROM 2332 101-1035A01, in module slot
- buzzer, 2 7seg LEDs, 8*8 chessboard buttons

released modules, * denotes not dumped yet:
- *BO6: Book Openings I
- *CG6: Greatest Chess Games 1
- SC6: pack-in, original program

SC6 program is contained in BO6 and CG6.

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_sc6.lh" // clickable


namespace {

class sc6_state : public fidelbase_state
{
public:
	sc6_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sc6(machine_config &config);

private:
	// devices/pointers
	required_device<mcs48_cpu_device> m_maincpu;

	void main_map(address_map &map);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cartridge);

	// I/O handlers
	void prepare_display();
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(select_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_READ_LINE_MEMBER(input6_r);
	DECLARE_READ_LINE_MEMBER(input7_r);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// cartridge

DEVICE_IMAGE_LOAD_MEMBER(sc6_state, cartridge)
{
	u32 size = m_cart->common_get_size("rom");

	// 4KB ROM only?
	if (size != 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid file size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


// MCU ports/generic

void sc6_state::prepare_display()
{
	// 2 7seg leds
	set_display_segmask(3, 0x7f);
	display_matrix(7, 2, m_7seg_data, m_led_select);
}

WRITE8_MEMBER(sc6_state::mux_w)
{
	// P24-P27: 7442 A-D
	u16 sel = 1 << (data >> 4 & 0xf) & 0x3ff;

	// 7442 0-8: input mux, 7seg data
	m_inp_mux = sel & 0x1ff;
	m_7seg_data = sel & 0x7f;
	prepare_display();

	// 7442 9: speaker out
	m_dac->write(BIT(sel, 9));
}

WRITE8_MEMBER(sc6_state::select_w)
{
	// P16,P17: digit select
	m_led_select = ~data >> 6 & 3;
	prepare_display();
}

READ8_MEMBER(sc6_state::input_r)
{
	// P10-P15: multiplexed inputs low
	return (~read_inputs(9) & 0x3f) | 0xc0;
}

READ_LINE_MEMBER(sc6_state::input6_r)
{
	// T0: multiplexed inputs bit 6
	return ~read_inputs(9) >> 6 & 1;
}

READ_LINE_MEMBER(sc6_state::input7_r)
{
	// T1: multiplexed inputs bit 7
	return ~read_inputs(9) >> 7 & 1;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void sc6_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).r("cartslot", FUNC(generic_slot_device::read_rom));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sc6 )
	PORT_INCLUDE( fidel_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void sc6_state::sc6(machine_config &config)
{
	/* basic machine hardware */
	I8040(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc6_state::main_map);
	m_maincpu->p2_out_cb().set(FUNC(sc6_state::mux_w));
	m_maincpu->p1_in_cb().set(FUNC(sc6_state::input_r));
	m_maincpu->p1_out_cb().set(FUNC(sc6_state::select_w));
	m_maincpu->t0_in_cb().set(FUNC(sc6_state::input6_r));
	m_maincpu->t1_in_cb().set(FUNC(sc6_state::input7_r));

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_sc6);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_sc6", "bin"));
	cartslot.set_device_load(device_image_load_delegate(&sc6_state::device_image_load_cartridge, this));
	cartslot.set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_sc6");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fscc6 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	// none here, it's in the module slot
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME   PARENT  CMP MACHINE  INPUT  CLASS      INIT        COMPANY                 FULLNAME                      FLAGS
CONS( 1982, fscc6, 0,       0, sc6,     sc6,   sc6_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger 6", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
