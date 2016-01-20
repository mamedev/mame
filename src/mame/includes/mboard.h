// license:???
// copyright-holders:Ralf Schaefer, Cowering
/**********************************************************************

     Mephisto Chess Computers

**********************************************************************/

#ifndef __MBOARD_H__
#define __MBOARD_H__

/***************************************************************************
    MACROS
***************************************************************************/

enum
{
	EM,     /*No piece*/
	BP,
	BN,
	BB,
	BR,
	BQ,
	BK,
	WP,
	WN,
	WB,
	WR,
	WQ,
	WK
};

#define NOT_VALID       99
#define BORDER_PIECE    64


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class mboard_state : public driver_device
{
public:
	mboard_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_line(*this, "LINE"),
		m_line10(*this, "LINE10"),
		m_b_white(*this, "B_WHITE"),
		m_b_black(*this, "B_BLACK"),
		m_b_buttons(*this, "B_BUTTONS"),
		m_mouse_x(*this, "MOUSE_X"),
		m_mouse_y(*this, "MOUSE_Y"),
		m_button_l(*this, "BUTTON_L"),
		m_button_r(*this, "BUTTON_R")
	{ }

	DECLARE_READ8_MEMBER(mboard_read_board_8);
	DECLARE_WRITE8_MEMBER(mboard_write_board_8);
	DECLARE_WRITE8_MEMBER(mboard_write_LED_8);

	DECLARE_READ16_MEMBER(mboard_read_board_16);
	DECLARE_WRITE16_MEMBER(mboard_write_board_16);
	DECLARE_WRITE16_MEMBER(mboard_write_LED_16);

	DECLARE_READ32_MEMBER(mboard_read_board_32);
	DECLARE_WRITE32_MEMBER(mboard_write_board_32);
	DECLARE_WRITE32_MEMBER(mboard_write_LED_32);

	TIMER_DEVICE_CALLBACK_MEMBER( mboard_update_artwork);

	void mboard_savestate_register();

	void mboard_set_board();
	void mboard_set_border_pieces();

	inline UINT8 pos_to_num(UINT8 val);

	UINT8 mboard_lcd_invert;
	UINT8 mboard_key_select;
	UINT8 mboard_key_selector;

	int get_first_cleared_bit(UINT8 data);
private:
	static const int start_board[64];
	static UINT8 border_pieces[12];

	int m_board[64];
	int save_board[64];
	UINT16 Line18_LED;
	UINT16 Line18_REED;

	int mouse_hold_border_piece;
	UINT8 mouse_hold_from;
	UINT8 mouse_hold_piece;

	int read_board_flag;
	int get_first_bit(UINT8 data);
	UINT8 read_board();
	void write_board(UINT8 data);
	void write_LED(UINT8 data);
	void board_presave();
	void board_postload();
	void clear_board();
	void set_artwork();
	void check_board_buttons();
public:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<8> m_line;
	required_ioport m_line10;
	required_ioport m_b_white;
	required_ioport m_b_black;
	required_ioport m_b_buttons;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_button_l;
	required_ioport m_button_r;
};



#endif /* __MBOARD_H__ */
