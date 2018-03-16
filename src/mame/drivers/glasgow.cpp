// license:BSD-3-Clause
// copyright-holders:Dirk Verwiebe, Robbbert, Cowering
/***************************************************************************
Mephisto Glasgow 3 S chess computer
Dirk V.
sp_rinter@gmx.de

68000 CPU
64 KB ROM
16 KB RAM
4 Digit LC Display

3* 74LS138  Decoder/Multiplexer
1*74LS74    Dual positive edge triggered D Flip Flop
1*74LS139 1of4 Demultiplexer
1*74LS05    HexInverter
1*NE555     R=100K C=10uF
2*74LS04  Hex Inverter
1*74LS164   8 Bit Shift register
1*74121 Monostable Multivibrator with Schmitt Trigger Inputs
1*74LS20 Dual 4 Input NAND GAte
1*74LS367 3 State Hex Buffers


Made playable by Robbbert in Nov 2009.

How to play (quick guide)
1. You are the white player.
2. Click on the piece to move (LED starts flashing), then click where it goes
3. Computer plays black, it will work out its move and beep.
4. Read the move in the display, or look for the flashing LEDs.
5. Move the computer's piece in the same way you moved yours.
6. If a piece is being taken, firstly click on the piece then click the blank
    area at bottom right. This causes the piece to disappear. After that,
    move the piece that took the other piece.
7. You'll need to read the official user manual for advanced features, or if
    you get messages such as "Err1".

ToDo:
- Only glasgow works, the others are incomplete.


***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "speaker.h"

#include "glasgow.lh"


class glasgow_state : public driver_device
{
public:
	glasgow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_beep(*this, "beeper")
		, m_keyboard(*this, "LINE.%u", 0)
		, m_pieces(*this, "P%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void glasgow(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(glasgow_lcd_w);
	DECLARE_WRITE8_MEMBER(glasgow_lcd_flag_w);
	DECLARE_READ16_MEMBER(glasgow_keys_r);
	DECLARE_WRITE16_MEMBER(glasgow_keys_w);
	DECLARE_READ16_MEMBER(glasgow_board_r);
	DECLARE_WRITE16_MEMBER(glasgow_board_w);
	DECLARE_WRITE16_MEMBER(glasgow_beeper_w);
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	static uint8_t pos_to_num( uint8_t );
	void set_board();
	void glasgow_pieces_w();

	void glasgow_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
	required_ioport_array<8> m_keyboard;
	output_finder<64> m_pieces;
	output_finder<64> m_leds;
	output_finder<4> m_digits;

	uint8_t m_lcd_shift_counter;
	uint8_t m_led7;
	uint8_t m_key_select;
	bool m_lcd_invert;
	bool m_key_selector;
	uint8_t m_read_board_flag;
	uint8_t m_mouse_hold;
	uint8_t m_board_row;
	uint8_t m_mouse_down;
	uint16_t m_Line18_LED;
	uint16_t m_Line18_REED;
	uint8_t m_selected[2];

private:
	struct BOARD_FIELD { uint8_t field, piece; };
	BOARD_FIELD m_board[8][8];
	static const BOARD_FIELD s_start_board[8][8];
};


class amsterd_state : public glasgow_state
{
public:
	using glasgow_state::glasgow_state;

	void amsterd(machine_config &config);
	void dallas32(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(write_lcd);
	DECLARE_WRITE8_MEMBER(write_lcd_flag);
	DECLARE_WRITE8_MEMBER(write_beeper);
	DECLARE_READ16_MEMBER(read_board);
	DECLARE_WRITE8_MEMBER(write_board);
	DECLARE_WRITE16_MEMBER(write_lcd_invert);
	DECLARE_READ8_MEMBER(read_newkeys);
	DECLARE_READ32_MEMBER(read_board32);
	DECLARE_WRITE32_MEMBER(write_keys32);
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi32);

	void amsterd_mem(address_map &map);
	void dallas32_mem(address_map &map);
};


/* starts at bottom left corner */
const glasgow_state::BOARD_FIELD glasgow_state::s_start_board[8][8] =
{
	{ {7,10}, {6,8}, {5,9}, {4,11}, {3,12}, {2,9}, {1,8}, {0,10} },
	{ {15,7}, {14,7}, {13,7}, {12,7}, { 11,7}, {10,7}, {9,7}, {8,7} },

	{ {23,0}, {22,0}, {21,0}, {20,0}, {19,0}, {18,0}, {17,0}, {16,0} },
	{ {31,0}, {30,0}, {29,0}, {28,0}, {27,0}, {26,0}, {25,0}, {24,0} },
	{ {39,0}, {38,0}, {37,0}, {36,0}, {35,0}, {34,0}, {33,0}, {32,0} },
	{ {47,0}, {46,0}, {45,0}, {44,0}, {43,0}, {42,0}, {41,0}, {40,0} },

	{ {55,1}, {54,1}, {53,1}, {52,1}, {51,1}, {50,1}, {49,1}, {48,1} },
	{ {63,4}, {62,2}, {61,3}, {60,5}, {59,6}, {58,3}, {57,2}, {56,4} }
};

uint8_t glasgow_state::pos_to_num(uint8_t val)
{
	switch (val)
	{
	case 0xfe: return 7;
	case 0xfd: return 6;
	case 0xfb: return 5;
	case 0xf7: return 4;
	case 0xef: return 3;
	case 0xdf: return 2;
	case 0xbf: return 1;
	case 0x7f: return 0;
	default: return 0xff;
	}
}

void glasgow_state::set_board()
{
	// copy start postition to m_board
	for (uint8_t i_AH = 0; i_AH < 8; i_AH++)
		for (uint8_t i_18 = 0; i_18 < 8; i_18++)
			m_board[i_18][i_AH] = s_start_board[i_18][i_AH];
}

void glasgow_state::glasgow_pieces_w()
{
	// This causes the pieces to display on-screen
	for (uint8_t i_18 = 0; i_18 < 8; i_18++)
		for (uint8_t i_AH = 0; i_AH < 8; i_AH++)
			m_pieces[63 - m_board[i_18][i_AH].field] = m_board[i_18][i_AH].piece;
}

WRITE8_MEMBER( glasgow_state::glasgow_lcd_w )
{
	if (m_led7 == 0)
		m_digits[m_lcd_shift_counter] = data;

	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
}

WRITE8_MEMBER( glasgow_state::glasgow_lcd_flag_w )
{
	uint8_t const lcd_flag = data & 0x81;

	m_beep->set_state(BIT(lcd_flag, 0));

	if (lcd_flag)
		m_led7 = 255;
	else
	{
		m_led7 = 0;
		m_key_selector = 1;
	}
}

READ16_MEMBER( glasgow_state::glasgow_keys_r )
{
	m_board_row++;
	m_board_row &= 7;

	// See if we are moving a piece
	uint8_t data = m_keyboard[m_board_row]->read();

	if ((data < 0xff) && (m_mouse_down == 0))
	{
		uint8_t pos2num_res = pos_to_num(data);
		uint8_t piece = m_board[m_board_row][pos2num_res].piece;

		if (pos2num_res > 7)
			logerror("Position out of bound!");
		else
		if ((m_mouse_hold > 0) && ((piece == 0) || (piece > 12))) // putting a piece down onto blank spot or a shadow
		{
			if (m_selected[0] < 8)
			{
				m_board[m_selected[0]][m_selected[1]].piece = 0; // remove shadow
				m_selected[0] = 0xff;
			}
			m_board[m_board_row][pos2num_res].piece = m_mouse_hold;
			m_mouse_hold = 0;
		}
		else
		if ((m_mouse_hold == 0) && (piece) && (piece < 13)) // picking up a piece
		{
			m_mouse_hold = piece;
			m_board[m_board_row][pos2num_res].piece = m_mouse_hold + 12; // replace piece with shadow
			m_selected[0] = m_board_row;
			m_selected[1] = pos2num_res; // save position of shadow
		}

		m_mouse_down = m_board_row + 1;
	}
	else
	if ((data == 0xff) && (m_mouse_down == (m_board_row + 1))) // Wait for mouse to be released
		m_mouse_down = 0;

	// See if we are taking a piece off the board
	if (!ioport("LINE10")->read())
	{
		m_mouse_hold = 0;
		if (m_selected[0] < 8)
		{
			m_board[m_selected[0]][m_selected[1]].piece = 0; // remove shadow
			m_selected[0] = 0xff;
		}
	}

	// See if any keys pressed
	data = 3;

	if (m_key_select == ioport("LINE0")->read())
		data &= 1;

	if (m_key_select == ioport("LINE1")->read())
		data &= 2;

	return data << 8;
}

WRITE16_MEMBER( glasgow_state::glasgow_keys_w )
{
	m_key_select = data >> 8;
	glasgow_pieces_w();
}

READ16_MEMBER( glasgow_state::glasgow_board_r )
{
	uint8_t i_AH, data = 0;

	if (m_Line18_REED < 8)
	{
		// if there is a piece on the field -> set bit in data
		for (i_AH = 0; i_AH < 8; i_AH++)
		{
			if (!m_board[m_Line18_REED][i_AH].piece)
				data |= (1 << i_AH);
		}
	}

	m_read_board_flag = true;

	return data << 8;
}

WRITE16_MEMBER( glasgow_state::glasgow_board_w )
{
	m_Line18_REED = pos_to_num(data >> 8) ^ 7;

	// LED's or REED's ?
	if (m_read_board_flag)
	{
		m_Line18_LED = 0;
		m_read_board_flag = 0;
	}
	else
	{
		m_Line18_LED = data >> 8;
	}

	m_lcd_invert = 1;
}

WRITE16_MEMBER( glasgow_state::glasgow_beeper_w )
{
	uint16_t const LineAH = data >> 8;

	if (LineAH && m_Line18_LED)
	{
		for (uint8_t i_AH = 0; i_AH < 8; i_AH++)
		{
			if (LineAH & (1 << i_AH))
			{
				for (uint8_t i_18 = 0; i_18 < 8; i_18++)
				{
					if (!(m_Line18_LED & (1 << i_18)))
					{
						m_leds[m_board[i_18][i_AH].field] = 1; // LED on
					}
					else
					{
						// LED off
					}
				}
			}
		}
	}
	else
	{
		//  No LED  -> all LED's off
		for (uint8_t i_AH = 0; i_AH < 8; i_AH++)
		{
			for (uint8_t i_18 = 0; i_18 < 8; i_18++)
				m_leds[m_board[i_18][i_AH].field] = 0; // LED off
		}
	}
}

WRITE16_MEMBER( amsterd_state::write_lcd_invert )
{
	m_lcd_invert = 1;
}

WRITE8_MEMBER( amsterd_state::write_lcd )
{
	m_digits[m_lcd_shift_counter] = m_lcd_invert ? (data ^ 0xff) : data;
	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
	logerror("LCD Offset = %d Data low = %x \n", offset, data);
}

WRITE8_MEMBER( amsterd_state::write_lcd_flag )
{
	m_lcd_invert = 0;

	//beep_set_state(0, (data & 1) ? 1 : 0);
	if (!data)
		m_key_selector = 1;

	// The key function in the rom expects after writing to
	// the  a value from the second key row;
	m_led7 = data ? 255 : 0;

	logerror("LCD Flag = %x\n", data);
}

READ16_MEMBER( amsterd_state::read_board )
{
	return 0xff00; // Mephisto need it for working
}

WRITE8_MEMBER( amsterd_state::write_board )
{
	if (data == 0xff)
		m_key_selector = 0;
	// The key function in the rom expects after writing to
	// the chess board a value from  the first key row;
	logerror("Write Board = %x\n", data);
	glasgow_pieces_w();
}


WRITE8_MEMBER( amsterd_state::write_beeper )
{
	m_beep->set_state(BIT(data, 0));
}

READ8_MEMBER( amsterd_state::read_newkeys )  //Amsterdam, Roma, Dallas 32, Roma 32
{
	uint8_t data;

	if (m_key_selector)
		data = ioport("LINE1")->read();
	else
		data = ioport("LINE0")->read();

	logerror("read Keyboard Offset = %x Data = %x Select = %x \n", offset, data, m_key_selector);
	return data;
}


#ifdef UNUSED_FUNCTION
READ16_MEMBER(read_test)
{
	logerror("read test Offset = %x Data = %x\n  ",offset,data);
	return 0xffff;    // Mephisto need it for working
}
#endif

/*

    *****           32 Bit Read and write Handler   ***********

*/

WRITE32_MEMBER( amsterd_state::write_keys32 )
{
	m_lcd_invert = 1;
	m_key_select = data;
	logerror("Write Key = %x \n", m_key_select);
}

READ32_MEMBER( amsterd_state::read_board32 )
{
	logerror("read board 32 Offset = %x \n", offset);
	return 0;
}

#ifdef UNUSED_FUNCTION
READ16_MEMBER(read_board_amsterd)
{
	logerror("read board amsterdam Offset = %x \n  ", offset);
	return 0xffff;
}
#endif


TIMER_DEVICE_CALLBACK_MEMBER( glasgow_state::update_nmi)
{
	m_maincpu->set_input_line(7, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( amsterd_state::update_nmi32 )
{
	m_maincpu->set_input_line(6, HOLD_LINE); // this was 7 in the old code, which is correct?
}

void glasgow_state::machine_start()
{
	m_pieces.resolve();
	m_leds.resolve();
	m_digits.resolve();

	save_item(NAME(m_lcd_shift_counter));
	save_item(NAME(m_led7));
	save_item(NAME(m_key_select));
	save_item(NAME(m_lcd_invert));
	save_item(NAME(m_key_selector));
	save_item(NAME(m_read_board_flag));
	save_item(NAME(m_mouse_hold));
	save_item(NAME(m_board_row));
	save_item(NAME(m_mouse_down));
	save_item(NAME(m_Line18_LED));
	save_item(NAME(m_Line18_REED));
	save_item(NAME(m_selected));

	m_key_selector = 0;
}


void glasgow_state::machine_reset()
{
	m_lcd_shift_counter = 3;
	m_selected[0] = 0xff;
	set_board();
}


void glasgow_state::glasgow_mem(address_map &map)
{
	map.global_mask(0x1ffff);
	map(0x000000, 0x00ffff).rom();
	map(0x010000, 0x010000).w(this, FUNC(glasgow_state::glasgow_lcd_w));
	map(0x010002, 0x010003).rw(this, FUNC(glasgow_state::glasgow_keys_r), FUNC(glasgow_state::glasgow_keys_w));
	map(0x010004, 0x010004).w(this, FUNC(glasgow_state::glasgow_lcd_flag_w));
	map(0x010006, 0x010007).rw(this, FUNC(glasgow_state::glasgow_board_r), FUNC(glasgow_state::glasgow_beeper_w));
	map(0x010008, 0x010009).w(this, FUNC(glasgow_state::glasgow_board_w));
	map(0x01c000, 0x01ffff).ram(); // 16KB
}

void amsterd_state::amsterd_mem(address_map &map)
{
	// ADDRESS_MAP_GLOBAL_MASK(0x7FFFF)
	map(0x000000, 0x00ffff).rom();
	map(0x800002, 0x800002).w(this, FUNC(amsterd_state::write_lcd));
	map(0x800008, 0x800008).w(this, FUNC(amsterd_state::write_lcd_flag));
	map(0x800004, 0x800004).w(this, FUNC(amsterd_state::write_beeper));
	map(0x800010, 0x800010).w(this, FUNC(amsterd_state::write_board));
	map(0x800020, 0x800021).r(this, FUNC(amsterd_state::read_board));
	map(0x800040, 0x800040).r(this, FUNC(amsterd_state::read_newkeys));
	map(0x800088, 0x800089).w(this, FUNC(amsterd_state::write_lcd_invert));
	map(0xffc000, 0xffffff).ram(); // 16KB
}

void amsterd_state::dallas32_mem(address_map &map)
{
	// ADDRESS_MAP_GLOBAL_MASK(0x1FFFF)
	map(0x000000, 0x00ffff).rom();
	map(0x010000, 0x01ffff).ram(); // 64KB
	map(0x800002, 0x800002).w(this, FUNC(amsterd_state::write_lcd));
	map(0x800004, 0x800004).w(this, FUNC(amsterd_state::write_beeper));
	map(0x800008, 0x800008).w(this, FUNC(amsterd_state::write_lcd_flag));
	map(0x800010, 0x800010).w(this, FUNC(amsterd_state::write_board));
	map(0x800020, 0x800023).r(this, FUNC(amsterd_state::read_board32));
	map(0x800040, 0x800040).r(this, FUNC(amsterd_state::read_newkeys));
	map(0x800088, 0x80008b).w(this, FUNC(amsterd_state::write_keys32));
}


static INPUT_PORTS_START( chessboard )
	PORT_START("LINE.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
INPUT_PORTS_END

static INPUT_PORTS_START( new_keyboard ) //Amsterdam, Dallas 32, Roma, Roma 32
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B 2") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C 3") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D 4") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E 5") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F 6") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_0)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INF") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("POS") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEV") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLR") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G 7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8)

	PORT_INCLUDE( chessboard )
INPUT_PORTS_END

static INPUT_PORTS_START( old_keyboard )   //Glasgow,Dallas
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CL") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C 3") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D 4") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F 6") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B 2") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E 5") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INF") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("POS") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEV") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G 7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_F4)

	PORT_INCLUDE( chessboard )
INPUT_PORTS_END


MACHINE_CONFIG_START(glasgow_state::glasgow)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(glasgow_mem)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_glasgow)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 44)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", glasgow_state, update_nmi, attotime::from_hz(50))
MACHINE_CONFIG_END

MACHINE_CONFIG_START(amsterd_state::amsterd)
	glasgow(config);

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(amsterd_mem)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(amsterd_state::dallas32)
	glasgow(config);

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68020, 14000000)
	MCFG_CPU_PROGRAM_MAP(dallas32_mem)

	MCFG_DEVICE_REMOVE("nmi_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", amsterd_state, update_nmi32, attotime::from_hz(50))
MACHINE_CONFIG_END


/***************************************************************************
  ROM definitions
***************************************************************************/

ROM_START( glasgow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	//ROM_LOAD("glasgow.rom", 0x000000, 0x10000, CRC(3e73eff3) )
	ROM_LOAD16_BYTE("me3_3_1u.410",0x00000, 0x04000,CRC(bc8053ba) SHA1(57ea2d5652bfdd77b17d52ab1914de974bd6be12))
	ROM_LOAD16_BYTE("me3_1_1l.410",0x00001, 0x04000,CRC(d5263c39) SHA1(1bef1cf3fd96221eb19faecb6ec921e26ac10ac4))
	ROM_LOAD16_BYTE("me3_4_2u.410",0x08000, 0x04000,CRC(8dba504a) SHA1(6bfab03af835cdb6c98773164d32c76520937efe))
	ROM_LOAD16_BYTE("me3_2_2l.410",0x08001, 0x04000,CRC(b3f27827) SHA1(864ba897d24024592d08c4ae090aa70a2cc5f213))
ROM_END

ROM_START( amsterd )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	//ROM_LOAD16_BYTE("output.bin", 0x000000, 0x10000, CRC(3e73eff3) )
	ROM_LOAD16_BYTE("amsterda-u.bin",0x00000, 0x05a00,CRC(16cefe29) SHA1(9f8c2896e92fbfd47159a59cb5e87706092c86f4))
	ROM_LOAD16_BYTE("amsterda-l.bin",0x00001, 0x05a00,CRC(c859dfde) SHA1(b0bca6a8e698c322a8c597608db6735129d6cdf0))
ROM_END


ROM_START( dallas )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dal_g_pr.dat",0x00000, 0x04000,CRC(66deade9) SHA1(07ec6b923f2f053172737f1fc94aec84f3ea8da1))
	ROM_LOAD16_BYTE("dal_g_pl.dat",0x00001, 0x04000,CRC(c5b6171c) SHA1(663167a3839ed7508ecb44fd5a1b2d3d8e466763))
	ROM_LOAD16_BYTE("dal_g_br.dat",0x08000, 0x04000,CRC(e24d7ec7) SHA1(a936f6fcbe9bfa49bf455f2d8a8243d1395768c1))
	ROM_LOAD16_BYTE("dal_g_bl.dat",0x08001, 0x04000,CRC(144a15e2) SHA1(c4fcc23d55fa5262f5e01dbd000644a7feb78f32))
ROM_END

ROM_START( dallas16 )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dallas-u.bin",0x00000, 0x06f00,CRC(8c1462b4) SHA1(8b5f5a774a835446d08dceacac42357b9e74cfe8))
	ROM_LOAD16_BYTE("dallas-l.bin",0x00001, 0x06f00,CRC(f0d5bc03) SHA1(4b1b9a71663d5321820b4cf7da205e5fe5d3d001))
ROM_END

// This set needs checking. It cannot possibly work with this rom and hardware.
ROM_START( roma )
	ROM_REGION16_BE( 0x1000000, "maincpu", 0 )
	ROM_LOAD("roma32.bin", 0x000000, 0x10000, CRC(587d03bf) SHA1(504e9ff958084700076d633f9c306fc7baf64ffd))
ROM_END

ROM_START( dallas32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dallas32.epr", 0x000000, 0x10000, CRC(83b9ff3f) SHA1(97bf4cb3c61f8ec328735b3c98281bba44b30a28) )
ROM_END

ROM_START( roma32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roma32.bin", 0x000000, 0x10000, CRC(587d03bf) SHA1(504e9ff958084700076d633f9c306fc7baf64ffd) )
ROM_END


/***************************************************************************
  Game drivers
***************************************************************************/

/*     YEAR, NAME,     PARENT,   COMPAT, MACHINE,     INPUT,          CLASS,         INIT, COMPANY,                      FULLNAME,                 FLAGS */
CONS(  1984, glasgow,  0,        0,      glasgow,     old_keyboard,   glasgow_state, 0,    "Hegener & Glaser Muenchen",  "Mephisto III S Glasgow", 0)
CONS(  1984, amsterd,  0,        0,      amsterd,     new_keyboard,   amsterd_state, 0,    "Hegener & Glaser Muenchen",  "Mephisto Amsterdam",     MACHINE_NOT_WORKING)
CONS(  1984, dallas,   glasgow,  0,      glasgow,     old_keyboard,   glasgow_state, 0,    "Hegener & Glaser Muenchen",  "Mephisto Dallas",        MACHINE_NOT_WORKING)
CONS(  1984, roma,     amsterd,  0,      glasgow,     new_keyboard,   glasgow_state, 0,    "Hegener & Glaser Muenchen",  "Mephisto Roma",          MACHINE_NOT_WORKING)
CONS(  1984, dallas32, amsterd,  0,      dallas32,    new_keyboard,   amsterd_state, 0,    "Hegener & Glaser Muenchen",  "Mephisto Dallas 32 Bit", MACHINE_NOT_WORKING)
CONS(  1984, roma32,   amsterd,  0,      dallas32,    new_keyboard,   amsterd_state, 0,    "Hegener & Glaser Muenchen",  "Mephisto Roma 32 Bit",   MACHINE_NOT_WORKING)
CONS(  1984, dallas16, amsterd,  0,      amsterd,     new_keyboard,   amsterd_state, 0,    "Hegener & Glaser Muenchen",  "Mephisto Dallas 16 Bit", MACHINE_NOT_WORKING)
