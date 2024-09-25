// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Mattel Computer Chess

Teletape Productions was a small company founded by Julio Kaplan, the chess
engine is by them. This was 2 years before he founded Heuristic Software and
worked almost exclusively for SciSys/Saitek.

The power switch has a SAVE setting, this keeps the LCD chips powered on, and
the program reads the chessboard position from LCD RAM on the next boot. To save
a game in progress, make sure to press CLEAR before powering off. It does not
save the level setting.

There is no button to start a new game. The user is meant to switch power off/on.
In MAME, reset (or exit/restart) while the save switch is off.

Hardware notes:
- INS8050 CPU @ 6MHz (4KB internal ROM, 256 bytes internal RAM)
- 2*HLCD0569(also seen with 2*HLCD0601, functionally same?)
- custom LCD screen with chess squares background

It was also released in the USSR as Электроника ИМ-29 Шахматный партнёр,
assumed to be an unlicensed clone.

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "video/hlcd0515.h"

#include "screen.h"

// internal artwork
#include "mchess.lh"


namespace {

class mchess_state : public driver_device
{
public:
	mchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcd(*this, "lcd%u", 0),
		m_inputs(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u.%u", 0U, 0U, 0U)
	{ }

	void mchess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<mcs48_cpu_device> m_maincpu;
	required_device_array<hlcd0569_device, 2> m_lcd;
	required_ioport_array<4> m_inputs;
	output_finder<2, 8, 22> m_out_x;

	u8 m_inp_mux = 0;
	u8 m_lcd_control = 0;

	// I/O handlers
	template<int Sel> void lcd_output_w(offs_t offset, u32 data);
	void input_w(u8 data);
	u8 input_r();
	void lcd_w(u8 data);
	u8 lcd_r();
};

void mchess_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_control));
}

void mchess_state::machine_reset()
{
	// clear nvram if SAVE is off
	if (~m_inputs[3]->read() & 1)
	{
		m_lcd[0]->nvram_reset();
		m_lcd[1]->nvram_reset();
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

template<int Sel>
void mchess_state::lcd_output_w(offs_t offset, u32 data)
{
	// output to x.y.z where x = chip, y = row, z = col
	// up to 22 columns used
	for (int i = 0; i < 22; i++)
		m_out_x[Sel][offset][i] = BIT(data, i) & m_lcd_control;
}

void mchess_state::input_w(u8 data)
{
	// d0,d5,d6: input mux
	m_inp_mux = (~data >> 4 & 6) | (~data & 1);
}

u8 mchess_state::input_r()
{
	u8 data = 0;

	// d1-d4,d7: multiplexed inputs
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}

void mchess_state::lcd_w(u8 data)
{
	// d0: both LCDC VDRIVE
	// d1: N/C

	// d3: 1st LCDC _CS
	// d6: 2nd LCDC _CS
	m_lcd[0]->cs_w(BIT(data, 3));
	m_lcd[1]->cs_w(BIT(data, 6));

	// d4: both LCDC CLOCK
	// d5: both LCDC DATA IN
	for (int i = 0; i < 2; i++)
	{
		m_lcd[i]->data_w(BIT(data, 5));
		m_lcd[i]->clock_w(BIT(~data, 4));
	}

	m_lcd_control = data;
}

u8 mchess_state::lcd_r()
{
	// d2: 1st LCDC DATA OUT
	// d7: 2nd LCDC DATA OUT
	u8 r0 = m_lcd[0]->data_r();
	u8 r1 = m_lcd[1]->data_r();
	return ~0x84 | r0 << 2 | r1 << 7;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mchess )
	PORT_START("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Player vs. Player")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Machine vs. Machine")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Player vs. Machine")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Move")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")

	PORT_START("IN.2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter Move")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Take Back")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_TOGGLE PORT_NAME("Save Switch")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mchess_state::mchess(machine_config &config)
{
	// basic machine hardware
	I8050(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->p1_out_cb().set(FUNC(mchess_state::input_w));
	m_maincpu->p1_in_cb().set(FUNC(mchess_state::input_r));
	m_maincpu->p2_out_cb().set(FUNC(mchess_state::lcd_w));
	m_maincpu->p2_in_cb().set(FUNC(mchess_state::lcd_r));

	// video hardware
	HLCD0569(config, m_lcd[0], 500); // C=0.01uF
	m_lcd[0]->write_cols().set(FUNC(mchess_state::lcd_output_w<0>));
	m_lcd[0]->nvram_enable_backup(true);

	HLCD0569(config, m_lcd[1], 500); // C=0.01uF
	m_lcd[1]->write_cols().set(FUNC(mchess_state::lcd_output_w<1>));
	m_lcd[1]->nvram_enable_backup(true);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(977, 1080);
	screen.set_visarea_full();

	config.set_default_layout(layout_mchess);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mchess )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("ins8050-6hwu_n", 0x0000, 0x1000, CRC(de272323) SHA1(9ba323b614504e20b25c86d290c0667f0bbf6c6b) )

	ROM_REGION( 796406, "screen", 0)
	ROM_LOAD("mchess.svg", 0, 796406, CRC(795d66e0) SHA1(5f786c00bf33793bfba7065d8e9ec476e02e5c46) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, mchess, 0,      0,      mchess,  mchess, mchess_state, empty_init, "Mattel Electronics / Teletape Productions", "Computer Chess (Mattel)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
