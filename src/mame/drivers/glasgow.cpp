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
#include "glasgow.lh"
#include "sound/beep.h"

class glasgow_state : public driver_device
{
public:
	glasgow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_beep(*this, "beeper")
		{ }

	DECLARE_WRITE16_MEMBER(glasgow_lcd_w);
	DECLARE_WRITE16_MEMBER(glasgow_lcd_flag_w);
	DECLARE_READ16_MEMBER(glasgow_keys_r);
	DECLARE_WRITE16_MEMBER(glasgow_keys_w);
	DECLARE_READ16_MEMBER(glasgow_board_r);
	DECLARE_WRITE16_MEMBER(glasgow_board_w);
	DECLARE_WRITE16_MEMBER(glasgow_beeper_w);
	DECLARE_READ16_MEMBER(read_board);
	DECLARE_WRITE16_MEMBER(write_board);
	DECLARE_WRITE16_MEMBER(write_beeper);
	DECLARE_WRITE16_MEMBER(write_lcd);
	DECLARE_WRITE16_MEMBER(write_lcd_flag);
	DECLARE_WRITE16_MEMBER(write_irq_flag);
	DECLARE_READ16_MEMBER(read_newkeys16);
	DECLARE_WRITE32_MEMBER(write_beeper32);
	DECLARE_READ32_MEMBER(read_board32);
	DECLARE_WRITE32_MEMBER(write_board32);
	DECLARE_WRITE32_MEMBER(write_keys32);
	DECLARE_WRITE32_MEMBER(write_lcd32);
	DECLARE_WRITE32_MEMBER(write_lcd_flag32);
	DECLARE_READ32_MEMBER(read_newkeys32);
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi);
	TIMER_DEVICE_CALLBACK_MEMBER(update_nmi32);
	DECLARE_MACHINE_START(dallas32);
	DECLARE_MACHINE_START(glasgow);
	DECLARE_MACHINE_RESET(glasgow);

private:
	UINT8 m_lcd_shift_counter;
	UINT8 m_led7;
	UINT8 m_key_select;
	bool m_lcd_invert;
	bool m_key_selector;
	UINT8 m_read_board_flag;
	UINT8 m_mouse_hold;
	UINT8 m_board_row;
	UINT8 m_mouse_down;
	UINT16 m_Line18_LED;
	UINT16 m_Line18_REED;

	UINT8 pos_to_num( UINT8 );
	void set_board( );
	void glasgow_pieces_w( );

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;

};

typedef struct
	{
	UINT8 field;
	UINT8 piece;
	} BOARD_FIELD;

BOARD_FIELD l_board[8][8];

	/* starts at bottom left corner */
const BOARD_FIELD l_start_board[8][8] =
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

UINT8 glasgow_state::pos_to_num( UINT8 val )
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

void glasgow_state::set_board( )
{
	UINT8 i_AH, i_18;

	for (i_AH = 0; i_AH < 8; i_AH++)
	{
		for (i_18 = 0; i_18 < 8; i_18++)
		{
			// copy start postition to l_board
			l_board[i_18][i_AH] = l_start_board[i_18][i_AH];
		}
	}
}

void glasgow_state::glasgow_pieces_w( )
{
	// This causes the pieces to display on-screen
	UINT8 i_18, i_AH;

	for (i_18 = 0; i_18 < 8; i_18++)
		for (i_AH = 0; i_AH < 8; i_AH++)
			output().set_indexed_value("P", 63 - l_board[i_18][i_AH].field, l_board[i_18][i_AH].piece);
}

WRITE16_MEMBER( glasgow_state::glasgow_lcd_w )
{
	UINT8 lcd_data = data >> 8;

	if (m_led7 == 0)
		output().set_digit_value(m_lcd_shift_counter, lcd_data);

	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
}

WRITE16_MEMBER( glasgow_state::glasgow_lcd_flag_w )
{
	UINT16 lcd_flag = data & 0x8100;

	m_beep->set_state(BIT(lcd_flag, 8));

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
	static const char *const keynames[] = { "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8", "LINE9" };
	UINT8 pos2num_res = 0;
	m_board_row++;
	m_board_row &= 7;

	// See if we are moving a piece
	UINT8 data = ioport(keynames[m_board_row])->read();

	if ((data != 0xff) && (!m_mouse_down))
	{
		pos2num_res = pos_to_num(data);

		if (!(pos2num_res < 8))
			logerror("Position out of bound!");
		else
		if ((m_mouse_hold) && (!l_board[m_board_row][pos2num_res].piece))
		{
			// Moving a piece onto a blank
			l_board[m_board_row][pos2num_res].piece = m_mouse_hold;
			m_mouse_hold = 0;
		}
		else
		if ((!m_mouse_hold) && (l_board[m_board_row][pos2num_res].piece))
		{
			// Picking up a piece
			m_mouse_hold = l_board[m_board_row][pos2num_res].piece;
			l_board[m_board_row][pos2num_res].piece = 0;
		}

		m_mouse_down = m_board_row + 1;
	}
	else
	if ((data == 0xff) && (m_mouse_down == (m_board_row + 1))) // Wait for mouse to be released
		m_mouse_down = 0;

	// See if we are taking a piece off the board
	if (!ioport("LINE10")->read())
		m_mouse_hold = 0;

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
	UINT8 i_AH, data = 0;

	if (m_Line18_REED < 8)
	{
		// if there is a piece on the field -> set bit in data
		for (i_AH = 0; i_AH < 8; i_AH++)
		{
			if (!l_board[m_Line18_REED][i_AH].piece)
				data |= (1 << i_AH);
		}
	}

	m_read_board_flag = TRUE;

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
	UINT8 i_AH, i_18;
	UINT8 LED;
	UINT16 LineAH = data >> 8;

	if (LineAH && m_Line18_LED)
	{
		for (i_AH = 0; i_AH < 8; i_AH++)
		{
			if (LineAH & (1 << i_AH))
			{
				for (i_18 = 0; i_18 < 8; i_18++)
				{
					if (!(m_Line18_LED & (1 << i_18)))
					{
						LED = l_board[i_18][i_AH].field;
						output().set_led_value(LED, 1); // LED on
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
		for (i_AH = 0; i_AH < 8; i_AH++)
		{
			for (i_18 = 0; i_18 < 8; i_18++)
			{
				// LED off
				LED = l_board[i_18][i_AH].field;
				output().set_led_value(LED, 0);
			}
		}
	}
}

WRITE16_MEMBER( glasgow_state::write_beeper )
{
	m_lcd_invert = 1;
}

WRITE16_MEMBER( glasgow_state::write_lcd )
{
	UINT8 lcd_data = data >> 8;

	output().set_digit_value(m_lcd_shift_counter, m_lcd_invert ? lcd_data^0xff : lcd_data);
	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
	logerror("LCD Offset = %d Data low = %x \n", offset, lcd_data);
}

WRITE16_MEMBER( glasgow_state::write_lcd_flag )
{
	m_lcd_invert = 0;
	UINT8 lcd_flag = data >> 8;
	//beep_set_state(0, lcd_flag & 1 ? 1 : 0);
	if (lcd_flag == 0)
		m_key_selector = 1;

	// The key function in the rom expects after writing to
	// the  a value from the second key row;
	m_led7 = (lcd_flag) ? 255 : 0;

	logerror("LCD Flag 16 = %x \n", data);
}

READ16_MEMBER( glasgow_state::read_board )
{
	return 0xff00; // Mephisto need it for working
}

WRITE16_MEMBER( glasgow_state::write_board )
{
	UINT8 board = data >> 8;

	if (board == 0xff)
		m_key_selector = 0;
	// The key function in the rom expects after writing to
	// the chess board a value from  the first key row;
	logerror("Write Board = %x \n", data >> 8);
	glasgow_pieces_w();
}


WRITE16_MEMBER( glasgow_state::write_irq_flag )
{
	m_beep->set_state(BIT(data, 8));
	logerror("Write 0x800004 = %x \n", data);
}

READ16_MEMBER( glasgow_state::read_newkeys16 )  //Amsterdam, Roma
{
	UINT16 data;

	if (m_key_selector)
		data = ioport("LINE1")->read();
	else
		data = ioport("LINE0")->read();

	logerror("read Keyboard Offset = %x Data = %x Select = %x \n", offset, data, m_key_selector);
	data <<= 8;
	return data ;
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

WRITE32_MEMBER( glasgow_state::write_lcd32 )
{
	UINT8 lcd_data = data >> 8;

	output().set_digit_value(m_lcd_shift_counter, m_lcd_invert ? lcd_data^0xff : lcd_data);
	m_lcd_shift_counter--;
	m_lcd_shift_counter &= 3;
	// logerror("LCD Offset = %d Data   = %x \n  ", offset, lcd_data);
}

WRITE32_MEMBER( glasgow_state::write_lcd_flag32 )
{
	UINT8 lcd_flag = data >> 24;

	m_lcd_invert = 0;

	if (lcd_flag == 0)
		m_key_selector = 1;

	logerror("LCD Flag 32 = %x \n", lcd_flag);
	// beep_set_state(0, lcd_flag & 1 ? 1 : 0);

	m_led7 = (lcd_flag) ? 255 : 0;
}


WRITE32_MEMBER( glasgow_state::write_keys32 )
{
	m_lcd_invert = 1;
	m_key_select = data;
	logerror("Write Key = %x \n", m_key_select);
}

READ32_MEMBER( glasgow_state::read_newkeys32 ) // Dallas 32, Roma 32
{
	UINT32 data;

	if (m_key_selector)
		data = ioport("LINE1")->read();
	else
		data = ioport("LINE0")->read();
	//if (key_selector == 1) data = input_port_read(machine, "LINE0"); else data = 0;
	logerror("read Keyboard Offset = %x Data = %x\n", offset, data);
	data <<= 24;
	return data ;
}

READ32_MEMBER( glasgow_state::read_board32 )
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

WRITE32_MEMBER( glasgow_state::write_board32 )
{
	UINT8 board = data >> 24;
	if (board == 0xff)
		m_key_selector = 0;
	logerror("Write Board = %x \n", data);
	glasgow_pieces_w();
}


WRITE32_MEMBER( glasgow_state::write_beeper32 )
{
	m_beep->set_state(data & 0x01000000);
	logerror("Write 0x8000004 = %x \n", data);
}


TIMER_DEVICE_CALLBACK_MEMBER( glasgow_state::update_nmi)
{
	m_maincpu->set_input_line(7, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( glasgow_state::update_nmi32 )
{
	m_maincpu->set_input_line(6, HOLD_LINE); // this was 7 in the old code, which is correct?
}

MACHINE_START_MEMBER( glasgow_state, glasgow )
{
	m_key_selector = 0;
	m_lcd_shift_counter = 3;
}


MACHINE_START_MEMBER( glasgow_state, dallas32 )
{
	m_lcd_shift_counter = 3;
}


MACHINE_RESET_MEMBER( glasgow_state, glasgow )
{
	m_lcd_shift_counter = 3;
	set_board();
}


static ADDRESS_MAP_START(glasgow_mem, AS_PROGRAM, 16, glasgow_state)
	ADDRESS_MAP_GLOBAL_MASK(0x1FFFF)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x010000, 0x010001) AM_WRITE( glasgow_lcd_w )
	AM_RANGE(0x010002, 0x010003) AM_READWRITE( glasgow_keys_r, glasgow_keys_w )
	AM_RANGE(0x010004, 0x010005) AM_WRITE( glasgow_lcd_flag_w )
	AM_RANGE(0x010006, 0x010007) AM_READWRITE( glasgow_board_r, glasgow_beeper_w )
	AM_RANGE(0x010008, 0x010009) AM_WRITE( glasgow_board_w )
	AM_RANGE(0x01c000, 0x01ffff) AM_RAM // 16KB
ADDRESS_MAP_END

static ADDRESS_MAP_START(amsterd_mem, AS_PROGRAM, 16, glasgow_state)
	// ADDRESS_MAP_GLOBAL_MASK(0x7FFFF)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x800002, 0x800003) AM_WRITE( write_lcd )
	AM_RANGE(0x800008, 0x800009) AM_WRITE( write_lcd_flag )
	AM_RANGE(0x800004, 0x800005) AM_WRITE( write_irq_flag )
	AM_RANGE(0x800010, 0x800011) AM_WRITE( write_board )
	AM_RANGE(0x800020, 0x800021) AM_READ( read_board )
	AM_RANGE(0x800040, 0x800041) AM_READ( read_newkeys16 )
	AM_RANGE(0x800088, 0x800089) AM_WRITE( write_beeper )
	AM_RANGE(0xffc000, 0xffffff) AM_RAM // 16KB
ADDRESS_MAP_END

static ADDRESS_MAP_START(dallas32_mem, AS_PROGRAM, 32, glasgow_state)
	// ADDRESS_MAP_GLOBAL_MASK(0x1FFFF)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x010000, 0x01ffff) AM_RAM // 64KB
	AM_RANGE(0x800000, 0x800003) AM_WRITE( write_lcd32 )
	AM_RANGE(0x800004, 0x800007) AM_WRITE( write_beeper32 )
	AM_RANGE(0x800008, 0x80000B) AM_WRITE( write_lcd_flag32 )
	AM_RANGE(0x800010, 0x800013) AM_WRITE( write_board32 )
	AM_RANGE(0x800020, 0x800023) AM_READ( read_board32 )
	AM_RANGE(0x800040, 0x800043) AM_READ( read_newkeys32 )
	AM_RANGE(0x800088, 0x80008b) AM_WRITE( write_keys32 )
ADDRESS_MAP_END


static INPUT_PORTS_START( chessboard )
	PORT_START("LINE2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE9")
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


static MACHINE_CONFIG_START( glasgow, glasgow_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(glasgow_mem)
	MCFG_MACHINE_START_OVERRIDE(glasgow_state, glasgow )
	MCFG_MACHINE_RESET_OVERRIDE(glasgow_state, glasgow )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_glasgow)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 44)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", glasgow_state, update_nmi, attotime::from_hz(50))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( amsterd, glasgow )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(amsterd_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dallas32, glasgow )
	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68020, 14000000)
	MCFG_CPU_PROGRAM_MAP(dallas32_mem)
	MCFG_MACHINE_START_OVERRIDE(glasgow_state, dallas32 )

	MCFG_DEVICE_REMOVE("nmi_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", glasgow_state, update_nmi32, attotime::from_hz(50))
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
CONS(  1984, glasgow,  0,        0,      glasgow,     old_keyboard,   driver_device, 0,    "Hegener & Glaser Muenchen",  "Mephisto III S Glasgow", 0)
CONS(  1984, amsterd,  0,        0,      amsterd,     new_keyboard,   driver_device, 0,    "Hegener & Glaser Muenchen",  "Mephisto Amsterdam",     MACHINE_NOT_WORKING)
CONS(  1984, dallas,   glasgow,  0,      glasgow,     old_keyboard,   driver_device, 0,    "Hegener & Glaser Muenchen",  "Mephisto Dallas",        MACHINE_NOT_WORKING)
CONS(  1984, roma,     amsterd,  0,      glasgow,     new_keyboard,   driver_device, 0,    "Hegener & Glaser Muenchen",  "Mephisto Roma",          MACHINE_NOT_WORKING)
CONS(  1984, dallas32, amsterd,  0,      dallas32,    new_keyboard,   driver_device, 0,    "Hegener & Glaser Muenchen",  "Mephisto Dallas 32 Bit", MACHINE_NOT_WORKING)
CONS(  1984, roma32,   amsterd,  0,      dallas32,    new_keyboard,   driver_device, 0,    "Hegener & Glaser Muenchen",  "Mephisto Roma 32 Bit",   MACHINE_NOT_WORKING)
CONS(  1984, dallas16, amsterd,  0,      amsterd,     new_keyboard,   driver_device, 0,    "Hegener & Glaser Muenchen",  "Mephisto Dallas 16 Bit", MACHINE_NOT_WORKING)
