// license:BSD-3-Clause
// copyright-holders:hap,Sandro Ronco
/******************************************************************************

Fidelity Phantom (model 6100)

Fidelity licensed the design of the Milton/Phantom motorized chessboard and released
their own version. It has a small LCD panel added, the rest looks nearly the same from
the outside. After Fidelity was taken over by H+G, it was rereleased in 1990 as the
Mephisto Phantom. This is assumed to be identical.

Hardware notes:
- R65C02P4, XTAL marked 4.91?200
- 2*32KB ROM 27C256-15, 8KB RAM MS6264L-10
- LCD driver, display panel for digits
- magnetized x/y motor under chessboard, chesspieces have magnet underneath
- piezo speaker, LEDs, 8*8 chessboard buttons
- PCB label 510.1128A01

To play, wait until the motor is finished before making a move. At boot-up, the
computer will do a self-test.
After the player captures a piece, select the captured piece from the MAME sensorboard
spawn block and place it at the designated box at the edge of the chessboard.

TODO:
- sensorboard undo buffer goes out of control, probably not worth solving this issue

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/r65c02.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "fidel_phantom.lh" // clickable


namespace {

class phantom_state : public driver_device
{
public:
	phantom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_dac(*this, "dac"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_input(*this, "IN.0"),
		m_out_motor(*this, "motor.%u", 0U)
	{ }

	void fphantom(machine_config &config);
	void init_fphantom();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<dac_bit_interface> m_dac;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_ioport m_input;
	output_finder<5> m_out_motor;

	void main_map(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(motors_timer);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(lcd_w);
	DECLARE_WRITE8_MEMBER(lcd_mask_w);
	DECLARE_WRITE8_MEMBER(motors_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(rombank_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_READ8_MEMBER(motors_r);
	DECLARE_READ8_MEMBER(irq_ack_r);
	DECLARE_READ8_MEMBER(hmotor_ff_clear_r);
	DECLARE_READ8_MEMBER(vmotor_ff_clear_r);
	void update_pieces_position(int state);

	uint8_t   m_mux;
	uint8_t   m_lcd_mask;
	uint32_t  m_lcd_data;
	uint8_t   m_motors_ctrl;
	uint8_t   m_hmotor_pos;
	uint8_t   m_vmotor_pos;
	bool      m_vmotor_sensor0_ff;
	bool      m_vmotor_sensor1_ff;
	bool      m_hmotor_sensor0_ff;
	bool      m_hmotor_sensor1_ff;
	int       m_piece;
	bool      m_piece_collision;
	uint8_t   m_pieces_map[0x40][0x40];
};

void phantom_state::machine_start()
{
	m_out_motor.resolve();

	save_item(NAME(m_mux));
	save_item(NAME(m_lcd_mask));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_motors_ctrl));
	save_item(NAME(m_hmotor_pos));
	save_item(NAME(m_vmotor_pos));
	save_item(NAME(m_vmotor_sensor0_ff));
	save_item(NAME(m_vmotor_sensor1_ff));
	save_item(NAME(m_hmotor_sensor0_ff));
	save_item(NAME(m_hmotor_sensor1_ff));
	save_item(NAME(m_piece));
	save_item(NAME(m_piece_collision));
	save_item(NAME(m_pieces_map));
}

void phantom_state::machine_reset()
{
	m_mux = 0;
	m_lcd_mask = 0;
	m_lcd_data = 0;
	m_motors_ctrl = 0;
	m_hmotor_pos = 0xff;
	m_vmotor_pos = 0xff;
	m_vmotor_sensor0_ff = false;
	m_vmotor_sensor1_ff = false;
	m_hmotor_sensor0_ff = false;
	m_hmotor_sensor1_ff = false;
	m_piece = 0;
	m_piece_collision = false;
	memset(m_pieces_map, 0, sizeof(m_pieces_map));

	m_rombank->set_entry(0);
}

void phantom_state::init_fphantom()
{
	m_rombank->configure_entries(0, 2, memregion("rombank")->base(), 0x4000);
}


TIMER_DEVICE_CALLBACK_MEMBER(phantom_state::motors_timer)
{
	if (m_motors_ctrl & 0x03)  m_vmotor_sensor0_ff = true;
	if (m_motors_ctrl & 0x02)  m_vmotor_sensor1_ff = true;
	if (m_motors_ctrl & 0x0c)  m_hmotor_sensor0_ff = true;
	if (m_motors_ctrl & 0x04)  m_hmotor_sensor1_ff = true;

	if ((m_motors_ctrl & 0x01) && m_vmotor_pos > 0x00)  m_vmotor_pos--;
	if ((m_motors_ctrl & 0x02) && m_vmotor_pos < 0xff)  m_vmotor_pos++;
	if ((m_motors_ctrl & 0x04) && m_hmotor_pos > 0x00)  m_hmotor_pos--;
	if ((m_motors_ctrl & 0x08) && m_hmotor_pos < 0xff)  m_hmotor_pos++;
}

void phantom_state::update_pieces_position(int state)
{
	// convert motors position into board coordinates
	int x = m_hmotor_pos / 16 - 2;
	int y = m_vmotor_pos / 16;

	if (x < 0)
		x += 12;

	// check if the magnet is in the center of a square
	bool valid_pos = ((m_hmotor_pos & 0x0f) == 0x03 || (m_hmotor_pos & 0x0f) == 0x07) && ((m_vmotor_pos & 0x0f) == 0x09 || (m_vmotor_pos & 0x0f) == 0x0d);

	if (state)
	{
		if (m_piece_collision)
			m_piece_collision = valid_pos = false;

		if (valid_pos)
		{
			m_piece = m_board->read_piece(x, y);
			m_board->write_piece(x, y, 0);
		}
		else
			m_piece = m_pieces_map[m_vmotor_pos / 4][m_hmotor_pos / 4];

		m_pieces_map[m_vmotor_pos / 4][m_hmotor_pos / 4] = 0;
	}
	else
	{
		// check for pieces collisions
		if (valid_pos && m_board->read_piece(x, y) != 0)
		{
			valid_pos = false;
			m_piece_collision = true;
		}

		if (valid_pos)
			m_board->write_piece(x, y, m_piece);

		m_pieces_map[m_vmotor_pos / 4][m_hmotor_pos / 4] = m_piece;
		m_piece = 0;
	}

	m_board->refresh();
}


/******************************************************************************
    I/O
******************************************************************************/

WRITE8_MEMBER(phantom_state::mux_w)
{
	uint8_t mask = 1 << offset;
	m_mux = (m_mux & ~mask) | ((data & 0x02) ? mask : 0);
}

WRITE8_MEMBER(phantom_state::lcd_mask_w)
{
	m_lcd_mask = (data & 0x02) ? 0x00 : 0xff;
}

WRITE8_MEMBER(phantom_state::led_w)
{
	m_display->matrix_partial(0, 2, 1, BIT(~data, 7) << m_mux);
}

WRITE8_MEMBER(phantom_state::rombank_w)
{
	m_rombank->set_entry(data & 1);
}

WRITE8_MEMBER(phantom_state::motors_w)
{
	// bit 0: vertical motor down
	// bit 1: vertical motor up
	// bit 2: horizontal motor left
	// bit 3: horizontal motor right
	// bit 4: electromagnet
	// bit 5: speaker

	if ((m_motors_ctrl ^ data) & 0x10)
		update_pieces_position(BIT(data, 4));

	for (int i = 0; i < 5; i++)
		m_out_motor[i] = BIT(data, i);

	m_dac->write(BIT(data, 5));
	m_motors_ctrl = data;
}

WRITE8_MEMBER(phantom_state::lcd_w)
{
	data ^= m_lcd_mask;

	u32 mask = bitswap<8>(1 << offset,3,7,6,0,1,2,4,5);
	for (int i = 0; i < 4; i++)
	{
		m_lcd_data = (m_lcd_data & ~mask) | (BIT(data, i * 2) ? mask : 0);
		mask <<= 8;

		m_display->write_row(i+2, m_lcd_data >> (8*i) & 0xff);
	}

	m_display->update();
}

READ8_MEMBER(phantom_state::input_r)
{
	uint8_t data = 0xff;

	if (m_mux == 8)
	{
		if (BIT(m_input->read(), offset * 2 + 1))  data &= ~0x40;
		if (BIT(m_input->read(), offset * 2 + 0))  data &= ~0x80;
	}
	else if (offset < 4)
	{
		if (BIT(m_board->read_file(offset * 2 + 1), m_mux))  data &= ~0x40;
		if (BIT(m_board->read_file(offset * 2 + 0), m_mux))  data &= ~0x80;
	}
	else
	{
		if (BIT(m_board->read_file( 8 + (offset & 1)), m_mux))  data &= ~0x40;  // black captured pieces
		if (BIT(m_board->read_file(11 - (offset & 1)), m_mux))  data &= ~0x80;  // white captured pieces
	}

	return data;
}

READ8_MEMBER(phantom_state::motors_r)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0:
		if (!m_vmotor_sensor1_ff)  data &= ~0x40;
		if (!m_hmotor_sensor1_ff)  data &= ~0x80;
		break;
	case 1:
		if (!m_vmotor_sensor0_ff)  data &= ~0x40;
		if (!m_hmotor_sensor0_ff)  data &= ~0x80;
		break;
	}

	return data;
}

READ8_MEMBER(phantom_state::irq_ack_r)
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(R65C02_IRQ_LINE, CLEAR_LINE);
	return 0;
}

READ8_MEMBER(phantom_state::hmotor_ff_clear_r)
{
	if (!machine().side_effects_disabled())
		m_hmotor_sensor1_ff = m_hmotor_sensor0_ff = false;

	return 0;
}

READ8_MEMBER(phantom_state::vmotor_ff_clear_r)
{
	if (!machine().side_effects_disabled())
		m_vmotor_sensor1_ff = m_vmotor_sensor0_ff = false;

	return 0;
}


/******************************************************************************
    Address Maps
******************************************************************************/

void phantom_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2003).w(FUNC(phantom_state::mux_w));
	map(0x2004, 0x2004).w(FUNC(phantom_state::led_w));
	map(0x2006, 0x2006).w(FUNC(phantom_state::rombank_w));
	map(0x20ff, 0x20ff).w(FUNC(phantom_state::lcd_mask_w));
	map(0x2100, 0x2107).w(FUNC(phantom_state::lcd_w)).nopr();
	map(0x2200, 0x2200).w(FUNC(phantom_state::motors_w));
	map(0x2400, 0x2405).r(FUNC(phantom_state::input_r));
	map(0x2406, 0x2407).r(FUNC(phantom_state::motors_r));
	map(0x2500, 0x25ff).r(FUNC(phantom_state::hmotor_ff_clear_r));
	map(0x2600, 0x2600).r(FUNC(phantom_state::vmotor_ff_clear_r));
	map(0x2700, 0x2700).r(FUNC(phantom_state::irq_ack_r));
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( fphantom )
	PORT_START("IN.0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Verify / Problem")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Option / Time")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Level / New")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Take Back / Replay")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Hint / Info")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Move / Alternate")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Auto / Stop")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Shift")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void phantom_state::fphantom(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 4.9152_MHz_XTAL); // R65C02P4
	m_maincpu->set_periodic_int(FUNC(phantom_state::irq0_line_assert), attotime::from_hz(600)); // guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &phantom_state::main_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->set_size(12, 8);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	TIMER(config, "motors_timer").configure_periodic(FUNC(phantom_state::motors_timer), attotime::from_hz(120));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2+4, 9);
	m_display->set_segmask(0x3c, 0x7f);

	config.set_default_layout(layout_fidel_phantom);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fphantom ) // model 6100, PCB label 510.1128A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u_3c_yellow.u3", 0x8000, 0x8000, CRC(fb7c38ae) SHA1(a1aa7637705052cb4eec92644dc79aee7ba4d77c) ) // 27C256

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("u_4_white.u4",  0x0000, 0x8000, CRC(e4181ba2) SHA1(1f77d1867c6f566be98645fc252a01108f412c96) ) // 27C256
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     STATE          INIT           COMPANY, FULLNAME, FLAGS
CONS( 1988, fphantom, 0,      0, fphantom, fphantom, phantom_state, init_fphantom, "Fidelity Electronics", "Phantom Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_MECHANICAL )
