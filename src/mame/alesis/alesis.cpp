// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Alesis HR-16 and SR-16 drum machines

    06/04/2010 Skeleton driver.

    http://www.vintagesynth.com/misc/hr16.php
    http://www.vintagesynth.com/misc/sr16.php
    http://www.vintagesynth.com/misc/mmt8.php

****************************************************************************/

#include "emu.h"
#include "includes/alesis.h"
#include "sr16.lh"
#include "screen.h"


void alesis_state::kb_matrix_w(uint8_t data)
{
	m_kb_matrix = data;
}

uint8_t alesis_state::kb_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 6; i++)
		if (!BIT(m_kb_matrix, i))
			data &= m_col[i]->read();

	return data;
}

void alesis_state::led_w(uint8_t data)
{
	m_patt_led      = BIT(data, 0) ? 1 : 0;
	m_song_led      = BIT(data, 0) ? 0 : 1;
	m_play_led      = BIT(data, 1) ? 0 : 1;
	m_record_led    = BIT(data, 2) ? 0 : 1;
	m_voice_led     = BIT(data, 3) ? 0 : 1;
	m_tune_led      = BIT(data, 4) ? 0 : 1;
	m_mix_led       = BIT(data, 5) ? 0 : 1;
	m_tempo_led     = BIT(data, 6) ? 0 : 1;
	m_midi_led      = BIT(data, 7) ? 0 : 1;
}

uint8_t alesis_state::p3_r()
{
	uint8_t data = 0xff;

	data &= ~(m_cassette->input() > 0.01 ? 0x00 : 0x08);

	return data;
}

void alesis_state::p3_w(uint8_t data)
{
	m_cassette->output(data & 0x04 ? -1.0 : +1.0);
}

void alesis_state::sr16_lcd_w(uint8_t data)
{
	m_lcdc->write(BIT(m_kb_matrix,7), data);
}

void alesis_state::mmt8_led_w(uint8_t data)
{
	m_play_led      = BIT(data, 0) ? 0 : 1;
	m_record_led    = BIT(data, 1) ? 0 : 1;
	m_part_led      = BIT(data, 2) ? 0 : 1;
	m_edit_led      = BIT(data, 3) ? 0 : 1;
	m_song_led      = BIT(data, 4) ? 0 : 1;
	m_echo_led      = BIT(data, 5) ? 0 : 1;
	m_loop_led      = BIT(data, 6) ? 0 : 1;

	m_leds = data;
}

uint8_t alesis_state::mmt8_led_r()
{
	return m_leds;
}

void alesis_state::track_led_w(uint8_t data)
{
	for (int i=0; i < 8; i++)
		m_track_led[i] = BIT(data, i);
}

uint8_t alesis_state::mmt8_p3_r()
{
	// ---- -x--   Tape in
	// ---- x---   Start/Stop input
	uint8_t data = 0xff;

	data &= ~(m_cassette->input() > 0.01 ? 0x00 : 0x04);

	return data;
}

void alesis_state::mmt8_p3_w(uint8_t data)
{
	// ---x ----   Tape out
	// --x- ----   Click out

	m_cassette->output(data & 0x10 ? -1.0 : +1.0);
}

void alesis_state::hr16_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).mirror(0x8000).rom();
}

void alesis_state::hr16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).r(FUNC(alesis_state::kb_r));
	map(0x0002, 0x0002).w("dm3ag", FUNC(alesis_dm3ag_device::write));
	map(0x0004, 0x0004).w(FUNC(alesis_state::led_w));
	map(0x0006, 0x0007).rw(m_lcdc, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x0008, 0x0008).w(FUNC(alesis_state::kb_matrix_w));
	map(0x8000, 0xffff).ram().share("nvram");   // 32Kx8 SRAM, (battery-backed)
}

void alesis_state::sr16_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rom();
}

void alesis_state::sr16_io(address_map &map)
{
	//map.unmap_value_high();
	map(0x0000, 0x0000).mirror(0xff).w("dm3ag", FUNC(alesis_dm3ag_device::write));
	map(0x0200, 0x0200).mirror(0xff).w(FUNC(alesis_state::sr16_lcd_w));
	map(0x0300, 0x0300).mirror(0xff).w(FUNC(alesis_state::kb_matrix_w));
	map(0x0400, 0x0400).mirror(0xff).r(FUNC(alesis_state::kb_r));
	map(0x8000, 0xffff).ram().share("nvram");   // 32Kx8 SRAM, (battery-backed)
}

void alesis_state::mmt8_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().share("nvram");   // 2x32Kx8 SRAM, (battery-backed)
	map(0xff02, 0xff02).w(FUNC(alesis_state::track_led_w));
	map(0xff04, 0xff04).rw(FUNC(alesis_state::mmt8_led_r), FUNC(alesis_state::mmt8_led_w));
	map(0xff06, 0xff06).w(FUNC(alesis_state::kb_matrix_w));
	map(0xff08, 0xff09).rw(m_lcdc, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0xff0e, 0xff0e).nopr();
}

/* Input ports */
static INPUT_PORTS_START( hr16 )
	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY")   PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OFFSET") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SWING")  PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("QUANT")  PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LENGTH") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PATT")   PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI/UTIL") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TEMPO")  PORT_CODE(KEYCODE_T)
	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INSERT") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SONG")   PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")      PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")      PORT_CODE(KEYCODE_3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")      PORT_CODE(KEYCODE_4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")      PORT_CODE(KEYCODE_5)
	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ERASE")  PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAPE")   PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FILL")   PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")      PORT_CODE(KEYCODE_6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")      PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")      PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")      PORT_CODE(KEYCODE_9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")      PORT_CODE(KEYCODE_0)
	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PLAY")  PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP/CONTINUE") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RECORD") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("< -")    PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("> +")    PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("VOICE")  PORT_CODE(KEYCODE_V)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TUNE")   PORT_CODE(KEYCODE_H)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIX")    PORT_CODE(KEYCODE_M)
	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM 1")  PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM 2")  PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM 3")  PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM 4")  PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIDE")   PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CRASH")  PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PERC 1") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PERC 2") PORT_CODE(KEYCODE_F8)
	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KICK")   PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SNARE")  PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLOSED HAT") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MID HAT")    PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OPEN HAT")   PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLAPS")  PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PERC 3") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PERC 4") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("SELECT")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_NAME("SELECT Slider") PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_DOWN) PORT_CODE_INC(KEYCODE_UP)
INPUT_PORTS_END

static INPUT_PORTS_START( mmt8 )
	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<<")     PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">>")     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ERASE")  PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRANS")  PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PLAY")   PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP/CONTINUE") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY")   PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RECORD")  PORT_CODE(KEYCODE_HOME)
	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRACK 8") PORT_CODE(KEYCODE_F8)
	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TEMPO")  PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")      PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+")      PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x38, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAGE DOWN")  PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PAGE UP")    PORT_CODE(KEYCODE_UP)
	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLICK")  PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")      PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")      PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")      PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")      PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")      PORT_CODE(KEYCODE_0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI CHAN") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAPE")   PORT_CODE(KEYCODE_Y)
	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLOCK")  PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")      PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")      PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")      PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")      PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SONG")   PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MERGE")  PORT_CODE(KEYCODE_M)
	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI FILTER") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI ECHO")   PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOOP")   PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("QUANT")  PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LENGTH") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PART")   PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EDIT")   PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NAME")   PORT_CODE(KEYCODE_N)
INPUT_PORTS_END

static INPUT_PORTS_START( sr16 )
	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PRESET/USER")  PORT_CODE(KEYCODE_U)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PATTERN/SONG") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")      PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")      PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")      PORT_CODE(KEYCODE_4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")      PORT_CODE(KEYCODE_5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP")     PORT_CODE(KEYCODE_UP)
	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RECORD SETUP") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DRUM SET") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")      PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")      PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")      PORT_CODE(KEYCODE_8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")      PORT_CODE(KEYCODE_9)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")      PORT_CODE(KEYCODE_0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN")   PORT_CODE(KEYCODE_DOWN)
	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BACKUP") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ERASE")  PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PLAY")   PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP")   PORT_CODE(KEYCODE_END)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")      PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")      PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FILL")   PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY")   PORT_CODE(KEYCODE_C)
	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI SETUP") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TEMPO/PAGE UP") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM 1")  PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM 2")  PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TOM 3")  PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIDE")   PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CRASH")  PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PERC 1") PORT_CODE(KEYCODE_6_PAD)
	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PERFORM/COMPOSE") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TEMPO/PAGE DOWN") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KICK")   PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SNARE")  PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLOSED HAT") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OPEN HAT")   PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLAPS")  PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PERC 2") PORT_CODE(KEYCODE_F6)
	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void alesis_state::alesis_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void alesis_state::machine_start()
{
	m_digit.resolve();
	m_pattern.resolve();
	m_track_led.resolve();
	m_patt_led.resolve();
	m_song_led.resolve();
	m_play_led.resolve();
	m_record_led.resolve();
	m_voice_led.resolve();
	m_tune_led.resolve();
	m_mix_led.resolve();
	m_tempo_led.resolve();
	m_midi_led.resolve();
	m_part_led.resolve();
	m_edit_led.resolve();
	m_echo_led.resolve();
	m_loop_led.resolve();
	m_a_next.resolve();
	m_b_next.resolve();
	m_fill_next.resolve();
	m_user_next.resolve();
	m_play.resolve();
	m_record.resolve();
	m_compose.resolve();
	m_perform.resolve();
	m_song.resolve();
	m_b.resolve();
	m_a.resolve();
	m_fill.resolve();
	m_user.resolve();
	m_edited.resolve();
	m_set.resolve();
	m_drum.resolve();
	m_press_play.resolve();
	m_metronome.resolve();
	m_tempo.resolve();
	m_page.resolve();
	m_step_edit.resolve();
	m_swing_off.resolve();
	m_swing_62.resolve();
	m_click_l1.resolve();
	m_click_note.resolve();
	m_click_l2.resolve();
	m_click_3.resolve();
	m_backup.resolve();
	m_drum_set.resolve();
	m_swing.resolve();
	m_swing_58.resolve();
	m_click_off.resolve();
	m_click.resolve();
	m_quantize_off.resolve();
	m_quantize_3.resolve();
	m_midi_setup.resolve();
	m_record_setup.resolve();
	m_quantize.resolve();
	m_swing_54.resolve();
	m_quantize_l1.resolve();
	m_quantize_l2.resolve();
	m_quantize_l3.resolve();
	m_quantize_note.resolve();
	m_setup.resolve();
}

void alesis_state::machine_reset()
{
	m_kb_matrix = 0xff;
	m_leds = 0;
	memset(m_lcd_digits, 0, sizeof(m_lcd_digits));
}


HD44780_PIXEL_UPDATE(alesis_state::sr16_pixel_update)
{
	if (line == 1 && pos >= 6 && pos < 8)  // last 2 characters of the second line are used to control the LCD symbols
		update_lcd_symbols(bitmap, pos, y, x, state);
	else if (pos < 8)
		bitmap.pix(line*9 + y, pos*6 + x) = state;
}

void alesis_state::hr16(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &alesis_state::hr16_mem);
	m_maincpu->set_addrmap(AS_IO, &alesis_state::hr16_io);
	m_maincpu->port_in_cb<1>().set_ioport("SELECT");
	m_maincpu->port_in_cb<3>().set(FUNC(alesis_state::p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(alesis_state::p3_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(6*16, 9*2);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(alesis_state::alesis_palette), 2);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("hr16_cass");

	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 16);

	/* sound hardware */
	ALESIS_DM3AG(config, "dm3ag", 12_MHz_XTAL/2);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void alesis_state::sr16(machine_config &config)
{
	hr16(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &alesis_state::sr16_mem);
	m_maincpu->set_addrmap(AS_IO, &alesis_state::sr16_io);
	m_maincpu->port_in_cb<1>().set_constant(0);

	/* video hardware */
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(6*8, 9*2);
	screen.set_visarea_full();

	config.set_default_layout(layout_sr16);

	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(alesis_state::sr16_pixel_update));
}

void alesis_state::mmt8(machine_config &config)
{
	hr16(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &alesis_state::mmt8_io);
	m_maincpu->port_in_cb<1>().set(FUNC(alesis_state::kb_r));
	m_maincpu->port_in_cb<3>().set(FUNC(alesis_state::mmt8_p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(alesis_state::mmt8_p3_w));

	config.device_remove("dm3ag");
}

/* ROM definition */
ROM_START( hr16 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("v109")
	ROM_SYSTEM_BIOS(0, "v106", "ver 1.06")
	ROMX_LOAD("hr16-v1.06.bin",  0x0000, 0x8000, CRC(f0cdb899) SHA1(f21cd87af15ad5a0bfec992e38131c4f4e4c5102), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v107", "ver 1.07")
	ROMX_LOAD("2-19-0256-v107.u11",  0x0000, 0x8000, CRC(2582b6a2) SHA1(f1f135335578c938be63b37ed207e82b7a0e13be), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v109", "ver 1.09")
	ROMX_LOAD("2-19-0256-v109.u11",  0x0000, 0x8000, CRC(a9bdbf20) SHA1(229b4230c7b5380efbfd42fa95645723d3fd6d55), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v200", "ver 2.00")
	ROMX_LOAD("hr16-v2.0.bin",  0x0000, 0x8000, CRC(a3fcba12) SHA1(4c94be7e94e5a1d86443571cd4d375158a6e7b65), ROM_BIOS(3))

	ROM_REGION( 0x100000, "dm3ag", 0 )
	ROM_LOAD("2-27-0004.u16", 0x00000, 0x80000, CRC(8e103536) SHA1(092e1cf649fbef171cfaf91e20707d89998b7a1e))
	ROM_LOAD("2-27-0003.u15", 0x80000, 0x80000, CRC(82e9b78c) SHA1(89728cb38ae172b5e347a03018617c94a087dce0))
ROM_END

ROM_START( hr16b )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v200", "ver 2.00")
	ROMX_LOAD("2-19-0256-v200.u11",0x0000,  0x8000, CRC(19cf0fce) SHA1(f8b3786b32d68e3627a654b8b3916befbe9bc540), ROM_BIOS(0))

	ROM_REGION( 0x100000, "dm3ag", 0 )
	ROM_LOAD("2-27-0008.u16", 0x00000, 0x80000, CRC(11ca930e) SHA1(2f57fdd02f9b2146a551370a74cab1fa800145ab))
	ROM_LOAD("2-27-0007.u15", 0x80000, 0x80000, CRC(319746db) SHA1(46b32a3ab2fbad67fb4566f607f578a2e9defd63))
ROM_END

ROM_START( mmt8 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v111", "ver 1.11")
	ROMX_LOAD("mt8v1-11.bin", 0x00000, 0x08000, CRC(c9951946) SHA1(149bc5ea46466537de4074820c66a2296ea43bc1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v109", "ver 1.09")
	ROMX_LOAD("mt8v1-09.bin", 0x00000, 0x08000, CRC(0ec41dec) SHA1(2c283965e510b586a08f0290df4dd357e6b19b62), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v108", "ver 1.08")
	ROMX_LOAD("mt8v1-08.bin", 0x00000, 0x08000, CRC(a0615455) SHA1(77395c837b356b34d6b96f6f46eca8c89b57434e), ROM_BIOS(2))
ROM_END

ROM_START( sr16 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v104", "ver 1.04")
	ROMX_LOAD( "sr16_v1_04.bin", 0x0000, 0x10000, CRC(d049af6e) SHA1(0bbeb4bd25e33a9eca64d5a31480f96a0040617e), ROM_BIOS(0))

	ROM_REGION( 0x100000, "dm3ag", ROMREGION_ERASEFF )
	ROM_LOAD( "sr16.u6", 0x00000, 0x80000, CRC(6da96987) SHA1(3ec8627d440bc73841e1408a19def09a8b0b77f7))
	ROM_LOAD( "sr16.u5", 0x80000, 0x80000, CRC(8bb25cfa) SHA1(273ad59d017b54a7e8d5e1ec61c8cd807a0e4af3))
ROM_END


void alesis_state::init_hr16()
{
	uint8_t *ROM = memregion("maincpu")->base();
	uint8_t *orig = memregion("user1")->base();
	for (int i = 0; i < 0x8000; i++)
	{
		ROM[bitswap<16>(i,15,14,13,12,11,10,9,8,0,1,2,3,4,5,6,7)] = orig[i];
	}
}

/* Driver */
/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY   FULLNAME          FLAGS */
SYST( 1987, hr16,  0,      0,      hr16,    hr16,  alesis_state, init_hr16,  "Alesis", "HR-16",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST( 1987, mmt8,  0,      0,      mmt8,    mmt8,  alesis_state, empty_init, "Alesis", "MMT-8",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST( 1989, hr16b, hr16,   0,      hr16,    hr16,  alesis_state, init_hr16,  "Alesis", "HR-16B",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST( 1990, sr16,  0,      0,      sr16,    sr16,  alesis_state, empty_init, "Alesis", "SR-16 (Alesis)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
