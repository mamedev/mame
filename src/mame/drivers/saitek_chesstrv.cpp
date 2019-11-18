// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:Sean Riddle
/******************************************************************************

SciSys Chess Traveler

Hardware notes:
- Fairchild 3870 MCU, label SL90387 (does not use the timer or irq at all)
- 256 bytes RAM(3539)
- 4-digit 7seg led panel

It was also redistributed by Acetronic as "Chess Traveller"(British spelling there),
and by Prinztronic as well, another British brand

SciSys/Novag's "Chess Champion: Pocket Chess" is assumed to be the same game,
it has the same MCU serial (SL90387). They added battery low voltage detection
to it (rightmost digit DP lights up).

******************************************************************************/

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/sensorboard.h"
#include "video/pwm.h"

// internal artwork
#include "saitek_chesstrv.lh" // clickable


namespace {

class chesstrv_state : public driver_device
{
public:
	chesstrv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void chesstrv(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_inputs;

	void chesstrv_mem(address_map &map);
	void chesstrv_io(address_map &map);

	void update_display();
	DECLARE_WRITE8_MEMBER(matrix_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(input_r);

	// 256 bytes data RAM accessed via I/O ports
	DECLARE_READ8_MEMBER(ram_address_r) { return m_ram_address; }
	DECLARE_WRITE8_MEMBER(ram_address_w) { m_ram_address = data; }
	DECLARE_READ8_MEMBER(ram_data_r) { return m_ram[m_ram_address]; }
	DECLARE_WRITE8_MEMBER(ram_data_w) { m_ram[m_ram_address] = data; }

	std::unique_ptr<u8[]> m_ram;
	u8 m_ram_address;
	u8 m_inp_mux;
	u8 m_7seg_data;
};

void chesstrv_state::machine_start()
{
	// zerofill
	m_ram = make_unique_clear<u8[]>(0x100);
	m_ram_address = 0;
	m_inp_mux = 0;
	m_7seg_data = 0;

	// register for savestates
	save_pointer(NAME(m_ram), 0x100);
	save_item(NAME(m_ram_address));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_7seg_data));
}



/******************************************************************************
    I/O
******************************************************************************/

// F3870 ports

void chesstrv_state::update_display()
{
	m_display->matrix(~m_inp_mux, m_7seg_data);
}

WRITE8_MEMBER(chesstrv_state::digit_w)
{
	// digit segments
	m_7seg_data = bitswap<8>(data,0,1,2,3,4,5,6,7) & 0x7f;
	update_display();
}

WRITE8_MEMBER(chesstrv_state::matrix_w)
{
	// d0-d3: input/digit select (active low)
	m_inp_mux = data;
	update_display();
}

READ8_MEMBER(chesstrv_state::input_r)
{
	u8 data = m_inp_mux;

	// d0-d3: multiplexed inputs from d4-d7
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i+4))
			data |= m_inputs[i]->read();

	// d4-d7: multiplexed inputs from d0-d3
	for (int i = 0; i < 4; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << (i+4);

	return data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void chesstrv_state::chesstrv_mem(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void chesstrv_state::chesstrv_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(chesstrv_state::ram_address_r), FUNC(chesstrv_state::ram_address_w));
	map(0x01, 0x01).w(FUNC(chesstrv_state::digit_w));
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( chesstrv )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A 1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B 2 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C 3 / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D 4 / Rook")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E 5 / Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F 6 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G 7 / White")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H 8 / Black")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LV / CS") // level/clear square
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("FP") // find position
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("EP") // enter position
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("CB") // clear board

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MM") // multi move
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void chesstrv_state::chesstrv(machine_config &config)
{
	/* basic machine hardware */
	F8(config, m_maincpu, 3000000/2); // Fairchild 3870, measured ~3MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &chesstrv_state::chesstrv_mem);
	m_maincpu->set_addrmap(AS_IO, &chesstrv_state::chesstrv_io);

	f38t56_device &psu(F38T56(config, "psu", 3000000/2));
	psu.read_a().set(FUNC(chesstrv_state::ram_data_r));
	psu.write_a().set(FUNC(chesstrv_state::ram_data_w));
	psu.read_b().set(FUNC(chesstrv_state::input_r));
	psu.write_b().set(FUNC(chesstrv_state::matrix_w));

	// built-in chessboard is not electronic
	sensorboard_device &board(SENSORBOARD(config, "board").set_type(sensorboard_device::NOSENSORS));
	board.init_cb().set("board", FUNC(sensorboard_device::preset_chess));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_saitek_chesstrv);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( chesstrv )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("3870-sl90387", 0x0000, 0x0800, CRC(b76214d8) SHA1(7760903a64d9c513eb54c4787f535dabec62eb64) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     STATE           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1980, chesstrv, 0,      0, chesstrv, chesstrv, chesstrv_state, empty_init, "SciSys / Novag", "Chess Traveler", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
