// license:???
// copyright-holders:Ralf Schaefer, Cowering
/**********************************************************************

 Mephisto Chess Computers

**********************************************************************/
#include "emu.h"
#include "includes/mboard.h"
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define IsPiece(x)      ((m_board[x] >=1) && (m_board[x] <=12))

const int mboard_state::start_board[64] =
{
	BR, BN, BB, BQ, BK, BB, BN, BR,
	BP, BP, BP, BP, BP, BP, BP, BP,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	WP, WP, WP, WP, WP, WP, WP, WP,
	WR, WN, WB, WQ, WK, WB, WN, WR
};

UINT8 mboard_state::border_pieces[12] = {WK,WQ,WR,WB,WN,WP,BK,BQ,BR,BB,BN,BP,};


int mboard_state::get_first_bit(UINT8 data)
{
	int i;

	for (i = 0; i < 8; i++)
		if (BIT(data, i))
			return i;

	return NOT_VALID;
}


inline UINT8 mboard_state::pos_to_num(UINT8 val)
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

int mboard_state::get_first_cleared_bit(UINT8 data)
{
	int i;

	for (i=0;i<8;i++)
		if (!BIT(data, i))
			return i;

	return NOT_VALID;
}

UINT8 mboard_state::read_board()
{
		UINT8 i_18, i_AH;
		UINT8 data;

		data = 0xff;

/*

Example board scan:
Starting postion and pawn on E2 is lifted


mask: 7f 0111 1111  Line 8
data:  0 0000 0000  all fields occupied
mask: bf 1011 1111  Line 7
data:  0 0000 0000  all fields occupied
mask: df 1101 1111  Line 6
data: ff 1111 1111  all fields empty
mask: ef 1110 1111  Line 5
data: ff 1111 1111  all fields empty
mask: f7 1111 0111  Line 4
data: ff 1111 1111  all fields empty
mask: fb 1111 1011  Line 3
data: ff 1111 1111  all fields empty
mask: fd 1111 1101  Line 2
data: 10 0001 0000  E2 is empty rest is occupied
mask: fe 1111 1110  Line 1
data:  0 0000 0000  all fields occupied


*/

/* looking for cleared bit in mask Line18_REED => current line */

	if (data && Line18_REED)
	{
		i_18 = get_first_cleared_bit(Line18_REED);

		if (i_18 == NOT_VALID)
			printf("No cleared bit in mask Line18_REED!\n");
		else
		{
			/* looking for a piece in this line and clear bit in data if found */

			for (i_AH = 0; i_AH < 8; i_AH = i_AH + 1)
				if (IsPiece(64 - (i_18 * 8 + 8 - i_AH)))
					data &= ~(1 << i_AH);           // clear bit

			read_board_flag = TRUE;
		}
	}

	return data;
}


void mboard_state::write_board(UINT8 data)
{
	Line18_REED=data;

	if (read_board_flag && !strcmp(machine().system().name,"glasgow") ) //HACK
		Line18_LED = 0;
	else
		Line18_LED = data;

		read_board_flag = FALSE;

	if (data == 0xff)
		mboard_key_selector = 0;
}



void mboard_state::write_LED(UINT8 data)
{
	int i;
	UINT8 i_AH, i_18;
	UINT8 LED;

	mboard_lcd_invert = 1;
/*

Example: turn led E2 on

mask:  fd 1111 1101 Line 2
data:  10 0001 0000 Line E

*/

	for (i=0; i < 64; i++)                          /* all  LED's off */
		output_set_led_value(i, 0);

	if (Line18_LED)
	{
		for (i_AH = 0; i_AH < 8; i_AH++)                /* turn  LED on depending on bit masks */
		{
			if (BIT(data,i_AH))
			{
				for (i_18 = 0; i_18 < 8; i_18++)
				{
					LED = (i_18*8 + 8-i_AH-1);
					if (!(Line18_LED & (1 << i_18)))    /* cleared bit */
						output_set_led_value(LED, 1);
					//else
					//  output_set_led_value(LED, 0);
				}
			}
		}
	}

}



READ8_MEMBER(mboard_state::mboard_read_board_8)
{
	UINT8 data;

	data=read_board();
	logerror("Read Board Port  Data = %d\n  ",data);
	return data;
}

READ16_MEMBER(mboard_state::mboard_read_board_16)
{
	UINT8 data;

	data=read_board();
	return data << 8;
}

READ32_MEMBER(mboard_state::mboard_read_board_32)
{
	UINT8 data;

	data=read_board();
	return data<<24;
}

WRITE8_MEMBER(mboard_state::mboard_write_board_8)
{
	write_board(data);
	logerror("Write Board Port  Data = %02x\n",data);
}

WRITE16_MEMBER(mboard_state::mboard_write_board_16)
{
	if (data & 0xff) write_board(data);
	logerror("write board 16 %08x\n",data);
	write_board(data>>8);
}

WRITE32_MEMBER(mboard_state::mboard_write_board_32)
{
//  data |= data << 24;
//printf("write board %08x %08x\n",offset,data);
	logerror("write board 32 o: %08x d: %08x\n",offset,data);
	if (offset) write_board(data);
	else write_board(data>>24);
}

WRITE8_MEMBER(mboard_state::mboard_write_LED_8)
{
	write_LED(data);
	space.device().execute().spin_until_time(attotime::from_usec(7));
}

WRITE16_MEMBER(mboard_state::mboard_write_LED_16)
{
		write_LED(data >> 8);
		space.device().execute().spin_until_time(attotime::from_usec(9));
}

WRITE32_MEMBER(mboard_state::mboard_write_LED_32)
{
//  data = data | data << 24;
//printf("write LED %08x %08x\n",offset,data);
	if (offset) write_LED(data);
	else write_LED(data >> 24);
	logerror("write LED   32 o: %08x d: %08x\n",offset,data);
//  space.device().execute().spin_until_time(ATTOTIME_IN_USEC(20));
}


/* save states callback */

void mboard_state::board_presave()
{
	int i;
	for (i=0;i<64;i++)
		save_board[i]=m_board[i];
}

void mboard_state::board_postload()
{
	int i;
	for (i=0;i<64;i++)
		m_board[i]=save_board[i];

}

void mboard_state::mboard_savestate_register()
{
	save_item(NAME(save_board));
	machine().save().register_postload(save_prepost_delegate(FUNC(mboard_state::board_postload),this));
	machine().save().register_presave(save_prepost_delegate(FUNC(mboard_state::board_presave),this));
}

void mboard_state::mboard_set_board()
{
	read_board_flag = TRUE;
	int i;
	for (i=0;i<64;i++)
		m_board[i]=start_board[i];
}

void mboard_state::clear_board()
{
	int i;
	for (i=0;i<64;i++)
		m_board[i]=EM;
}

void mboard_state::set_artwork()
{
	int i;
	for (i=0;i<64;i++)
		output_set_indexed_value("P", i, m_board[i]);
}

void mboard_state::mboard_set_border_pieces()
{
	int i;
	for (i=0;i<12;i++)
		output_set_indexed_value("Q", i, border_pieces[i]);
}

TIMER_DEVICE_CALLBACK_MEMBER(mboard_state::mboard_update_artwork )
{
	check_board_buttons();
	set_artwork();
	mboard_set_border_pieces();
}

void mboard_state::check_board_buttons()
{
	int field;
	int i;
	UINT8 port_input=0;
	UINT8 data = 0xff;
	static UINT8 board_row = 0;
	static UINT16 mouse_down = 0;
	UINT8 pos2num_res = 0;
	board_row++;
	board_row &= 7;
	int click_on_border_piece=FALSE;


/* check click on border pieces */
	i=0;
	port_input = m_b_black->read();
	if (port_input)
	{
		i=get_first_bit(port_input)+6;
		click_on_border_piece=TRUE;
	}

	port_input = m_b_white->read();
	if (port_input)
	{
		i=get_first_bit(port_input);
		click_on_border_piece=TRUE;
	}

	if (click_on_border_piece)
	{
		if (!mouse_down)
		{
			if (border_pieces[i] > 12 )     /* second click on selected border piece */
			{
				mouse_hold_border_piece=FALSE;
				border_pieces[i]=border_pieces[i]-12;
				mouse_hold_from=0;
				mouse_hold_piece=0;
			}
			else if (!mouse_hold_piece)     /*select border piece */
			{
				if  (mouse_hold_border_piece)
					border_pieces[mouse_hold_from]=border_pieces[mouse_hold_from]-12;

				mouse_hold_from=i;
				mouse_hold_piece=border_pieces[i];
				border_pieces[i]=border_pieces[i]+12;
				mouse_hold_border_piece=TRUE;
			}

			mouse_down = board_row + 1;

		}
		return;
	}


/* check click on board */
	data = m_line[board_row]->read();

	if ((data != 0xff) && (!mouse_down) )
	{
		pos2num_res = pos_to_num(data);
		field=64-(board_row*8+8-pos2num_res);


		if (!(pos2num_res < 8))
			logerror("Position out of bound!");

		else if ((mouse_hold_piece) && (!IsPiece(field)))
		{
			/* Moving a piece onto a blank */
			m_board[field] = mouse_hold_piece;

			if (mouse_hold_border_piece)
			{
				border_pieces[mouse_hold_from]=border_pieces[mouse_hold_from]-12;
			}else if ( field != mouse_hold_from  )  /* Put a selected piece back to the source field */
				m_board[mouse_hold_from] = 0;


			mouse_hold_from  = 0;
			mouse_hold_piece = 0;
			mouse_hold_border_piece=FALSE;
		}
		else if ((!mouse_hold_piece) )
		{
			/* Picking up a piece */

			if (IsPiece(field))
			{
				mouse_hold_from  = field;
				mouse_hold_piece = m_board[field];
				m_board[field] = m_board[field]+12;
			}

		}

		mouse_down = board_row + 1;
	}
	else if ((data == 0xff) && (mouse_down == (board_row + 1))) /* Wait for mouse to be released */
		mouse_down = 0;

/* check click on border - remove selected piece*/
	if (m_line10->read())
	{
		if (mouse_hold_piece)
		{
			if (mouse_hold_border_piece)
				border_pieces[mouse_hold_from]=border_pieces[mouse_hold_from]-12;
			else
				m_board[mouse_hold_from] = 0;

			mouse_hold_from  = 0;
			mouse_hold_piece = 0;
			mouse_hold_border_piece = FALSE;
		}

		return;
	}

/* check additional buttons */
	if (data == 0xff)
	{
		port_input = m_b_buttons->read();
		if (port_input==0x01)
		{
			clear_board();
			return;
		}else if (port_input==0x02)
		{
			mboard_set_board();
			return;
		}


	}

}

extern INPUT_PORTS_START( chessboard )

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("B_WHITE")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("B_BLACK")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("B_BUTTONS")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("MOUSE_X")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100)

	PORT_START("MOUSE_Y")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100)

	PORT_START("BUTTON_L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("left button")

	PORT_START("BUTTON_R")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("right button")


INPUT_PORTS_END
