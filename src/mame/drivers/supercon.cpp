// license:???
// copyright-holders:Ralf Schaefer, Robbbert
/******************************************************************************
 Novag SuperConstellation Chess Computer
 2010 R. Schaefer


 CPU 6502
 Clock 4 MHz
 IRQ CLK 600 Hz

 RAM    0x0000, 0x0fff)
 ROM    0x2000, 0xffff)
 I/O    0x1c00              Unknown
        0x1d00              Unknown
        0x1e00              LED's and buttons
        0x1f00              LED's, buttons and buzzer


******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/beep.h"


class supercon_state : public driver_device
{
public:
	supercon_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beep(*this, "beeper"),
		m_b_white(*this, "B_WHITE"),
		m_b_black(*this, "B_BLACK"),
		m_b_clr(*this, "B_CLR"),
		m_status_1(*this, "STATUS_1"),
		m_status_2(*this, "STATUS_2"),
		m_status_3(*this, "STATUS_3"),
		m_status_4(*this, "STATUS_4"),
		m_status_5(*this, "STATUS_5"),
		m_status_6(*this, "STATUS_6"),
		m_status_7(*this, "STATUS_7"),
		m_status_8(*this, "STATUS_8"),
		m_board_1(*this, "BOARD_1"),
		m_board_2(*this, "BOARD_2"),
		m_board_3(*this, "BOARD_3"),
		m_board_4(*this, "BOARD_4"),
		m_board_5(*this, "BOARD_5"),
		m_board_6(*this, "BOARD_6"),
		m_board_7(*this, "BOARD_7"),
		m_board_8(*this, "BOARD_8") { }

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
	DECLARE_READ8_MEMBER(supercon_port1_r);
	DECLARE_READ8_MEMBER(supercon_port2_r);
	DECLARE_READ8_MEMBER(supercon_port3_r);
	DECLARE_READ8_MEMBER(supercon_port4_r);
	DECLARE_WRITE8_MEMBER(supercon_port1_w);
	DECLARE_WRITE8_MEMBER(supercon_port2_w);
	DECLARE_WRITE8_MEMBER(supercon_port3_w);
	DECLARE_WRITE8_MEMBER(supercon_port4_w);
	static const UINT8 m_border_pieces[12];
	emu_timer* m_timer_update_irq;
	emu_timer* m_timer_mouse_click;
	int m_emu_started;
	int m_moving_piece;
	UINT8 m_data_1E00;
	UINT8 m_data_1F00;
	UINT8 m_LED_18;
	UINT8 m_LED_AH;
	UINT8 m_LED_ST;
	UINT8 *m_last_LED;
	UINT8 m_last_LED_value;
	int m_led_update;
	int m_remove_led_flag;
	int m_selecting;
	int m_save_key_data;
	attotime m_wait_time;
	int *m_current_field;
	int m_confirm_board_click;
	int m_board[64];
	int m_save_board[64];
	void set_board();
	void set_pieces();
	void set_border_pieces();
	void clear_pieces();
	void update_leds();
	virtual void machine_reset() override;
	DECLARE_DRIVER_INIT(supercon);
	virtual void machine_start() override;
	TIMER_CALLBACK_MEMBER(mouse_click);
	TIMER_CALLBACK_MEMBER(update_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(update_artwork);
	void board_presave();
	void board_postload();

protected:
	required_ioport m_b_white;
	required_ioport m_b_black;
	required_ioport m_b_clr;
	required_ioport m_status_1;
	required_ioport m_status_2;
	required_ioport m_status_3;
	required_ioport m_status_4;
	required_ioport m_status_5;
	required_ioport m_status_6;
	required_ioport m_status_7;
	required_ioport m_status_8;
	required_ioport m_board_1;
	required_ioport m_board_2;
	required_ioport m_board_3;
	required_ioport m_board_4;
	required_ioport m_board_5;
	required_ioport m_board_6;
	required_ioport m_board_7;
	required_ioport m_board_8;
	void mouse_update();
	int get_first_cleared_bit(UINT8 data);
	int get_first_bit(UINT8 data);
};


#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

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


#define LED_LINE_AH     0x10
#define LED_LINE_ST     0x20
#define LED_LINE_18     0x40


#define MAIN_CLOCK 4000000 /* 4 MHz */

#define NOT_VALID   99

#define NO_ACTION   0
#define TAKE        1
#define SET     2

/* artwork board */

const UINT8 supercon_state::m_border_pieces[12] = {WK,WQ,WR,WB,WN,WP,BK,BQ,BR,BB,BN,BP};

void supercon_state::set_board()
{
	static const int start_board[64] = {
	BR, BN, BB, BQ, BK, BB, BN, BR,
	BP, BP, BP, BP, BP, BP, BP, BP,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	WP, WP, WP, WP, WP, WP, WP, WP,
	WR, WN, WB, WQ, WK, WB, WN, WR };

	for (UINT8 i=0; i<64; i++)
		m_board[i]=start_board[i];
}

void supercon_state::set_pieces()
{
	int i;
	for (i=0;i<64;i++)
		output().set_indexed_value("P", i, m_board[i]);
}

void supercon_state::set_border_pieces()
{
	UINT8 i;

	for (i=0;i<12;i++)
		output().set_indexed_value("Q", i, m_border_pieces[i]);
}

void supercon_state::clear_pieces()
{
	int i;
	for (i=0;i<64;i++)
	{
		output().set_indexed_value("P", i, EM);
		m_board[i]=EM;
	}
}

int supercon_state::get_first_cleared_bit(UINT8 data)
{
	int i;

	for (i = 0; i < 8; i++)
		if (!BIT(data, i))
			return i;

	return NOT_VALID;
}

int supercon_state::get_first_bit(UINT8 data)
{
	int i;

	for (i = 0; i < 8; i++)
		if (BIT(data, i))
			return i;

	return NOT_VALID;
}


void supercon_state::update_leds()
{
	int i;

	for (i = 0; i < 8; i++)
	{
		if (BIT(m_LED_18, i))
			output().set_led_value(i + 1, 1);
		else
			output().set_led_value(i + 1, 0);

		if (BIT(m_LED_AH, i))
			output().set_led_value(i + 9, 1);
		else
			output().set_led_value(i + 9, 0);

		if (BIT(m_LED_ST, i))
			output().set_led_value(i + 17, 1);
		else
			output().set_led_value(i + 17, 0);
	}
}

void supercon_state::mouse_update()
{
	UINT8 port_input; // m_left;
	int i;

/* border pieces and moving piece */

	port_input = m_b_white->read();
	if (port_input)
	{
		i = get_first_bit(port_input);
		m_moving_piece = m_border_pieces[i];
		output().set_value("MOVING", m_moving_piece);
		return;
	}


	port_input = m_b_black->read();
	if (port_input)
	{
		i = get_first_bit(port_input);
		m_moving_piece = m_border_pieces[6+i];
		output().set_value("MOVING", m_moving_piece);
		return;
	}


	port_input = m_b_clr->read();
	if (port_input)
	{
		if (m_moving_piece)
		{
			m_moving_piece=0;
			output().set_value("MOVING", m_moving_piece);
			return;
		}
	}
}

/* Driver initialization */

DRIVER_INIT_MEMBER(supercon_state,supercon)
{
	m_LED_18=0;
	m_LED_AH=0;
	m_LED_ST=0;

	m_wait_time = attotime::from_hz(4);
	m_save_key_data = 0xff;

	m_moving_piece=0;
}

/* Read 1C000 */

READ8_MEMBER( supercon_state::supercon_port1_r )
{
	LOG(("Read from %04x \n",0x1C00));
	return 0xff;
}

/* Read 1D000 */

READ8_MEMBER( supercon_state::supercon_port2_r )
{
	LOG(("Read from %04x \n",0x1D00));
	return 0xff;
}

/* Read 1E00 */

READ8_MEMBER( supercon_state::supercon_port3_r )
{
	int i;
	UINT8 key_data=0;

	static const char *const status_lines[8] =
			{ "STATUS_1", "STATUS_2", "STATUS_3", "STATUS_4", "STATUS_5", "STATUS_6", "STATUS_7", "STATUS_8" };

	LOG(("Read from %04x \n",0x1E00));

/* remove last bit (only if it was not already set) */

	if ( m_data_1F00 & LED_LINE_AH )
	{
		if (m_last_LED_value != m_LED_AH)
			m_LED_AH=m_LED_AH & ~m_data_1E00;
	}
	else if ( m_data_1F00 & LED_LINE_ST)
	{
		if (m_last_LED_value != m_LED_ST)
			m_LED_ST=m_LED_ST & ~m_data_1E00;
	}
	else if ( m_data_1F00 & LED_LINE_18 )
	{
		if (m_last_LED_value != m_LED_18)
			m_LED_18=m_LED_18 & ~m_data_1E00;
	}


	LOG(("LED_18 from %02x \n",m_LED_18));
	LOG(("LED_AH from %02x \n",m_LED_AH));
	LOG(("LED_ST from %02x \n",m_LED_ST));

	if (m_led_update)           /*No LED Update if Port 1C00,1D00 was read */
		update_leds();

	m_remove_led_flag=TRUE;
	m_led_update=TRUE;

	m_LED_18=0;
	m_LED_AH=0;
	m_LED_ST=0;


/* Start */

	if (!m_emu_started)
		return 0xbf;
	else
		m_timer_update_irq->adjust( attotime::zero, 0, attotime::from_hz(598) );  //HACK adjust timer after start ???


/* Buttons */

	i=get_first_bit(m_data_1E00);
	if (i==NOT_VALID)
		return 0xff;

	switch ( i )
	{
		case 0: key_data=m_status_1->read(); break;
		case 1: key_data=m_status_2->read(); break;
		case 2: key_data=m_status_3->read(); break;
		case 3: key_data=m_status_4->read(); break;
		case 4: key_data=m_status_5->read(); break;
		case 5: key_data=m_status_6->read(); break;
		case 6: key_data=m_status_7->read(); break;
		case 7: key_data=m_status_8->read(); break;
	}

	if (key_data != 0xc0)
	{
		LOG(("%s, key_data: %02x \n",status_lines[i],key_data));

/* Button: New Game -> initialize board */

		if (i==0 && key_data==0x80)
		{
			set_board();
			set_pieces();

			m_emu_started=FALSE;
		}

/* Button: Clear Board -> remove all pieces */

		if (i==3 && key_data==0x80)
			clear_pieces();

		if (key_data != 0xff )
			return key_data;
	}

	return 0xc0;
}

/* Read Port $1F00 */

READ8_MEMBER( supercon_state::supercon_port4_r )
{
	int i_18, i_AH;
	UINT8 key_data = 0x00;

	static const char *const board_lines[8] =
			{ "BOARD_1", "BOARD_2", "BOARD_3", "BOARD_4", "BOARD_5", "BOARD_6", "BOARD_7", "BOARD_8" };

	LOG(("Read from %04x \n",0x1F00));

/* Board buttons */

	i_18=get_first_bit(m_data_1E00);
	if (i_18==NOT_VALID)
		return 0xff;

/* if a button was pressed wait til timer -timer_mouse_click- is fired */

	if (m_selecting)
		return m_save_key_data;
	else
	{
		set_pieces();
		output().set_value("MOVING",m_moving_piece);
	}

	switch ( i_18 )
	{
		case 0: key_data=m_board_1->read(); break;
		case 1: key_data=m_board_2->read(); break;
		case 2: key_data=m_board_3->read(); break;
		case 3: key_data=m_board_4->read(); break;
		case 4: key_data=m_board_5->read(); break;
		case 5: key_data=m_board_6->read(); break;
		case 6: key_data=m_board_7->read(); break;
		case 7: key_data=m_board_8->read(); break;
	}

	if (key_data != 0xff)
	{
		LOG(("%s key_data: %02x \n",board_lines[i_18],key_data));

/* Only if valid data */

		if (key_data)
		{
/* Set or remove pieces */

			i_AH=7-get_first_cleared_bit(key_data);
			LOG(("Press -> AH: %d 18: %d Piece: %d\n",i_AH,i_18,m_board[i_18*8 + i_AH]););

			m_current_field=&m_board[i_18*8 + i_AH];

			if (m_moving_piece)
			{
				*m_current_field = m_moving_piece;
				m_moving_piece = EM;
			}
			else
			{
				m_moving_piece = *m_current_field;
				*m_current_field = EM;
			}
			m_selecting=TRUE;               /* Flag is removed in timer -timer_mouse_click- */
			m_save_key_data=key_data;       /* return same key_data til flag selecting is removed */

			m_timer_mouse_click->adjust(m_wait_time, 0);

			return key_data;
		}
	}

	return 0xff;
}

/* Write Port $1C00 */

WRITE8_MEMBER( supercon_state::supercon_port1_w )
{
	LOG(("Write from %04x data: %02x\n",0x1C00,data));
	m_led_update=FALSE;
}

/* Write Port $1D00 */

WRITE8_MEMBER( supercon_state::supercon_port2_w )
{
	LOG(("Write from %04x data: %02x\n",0x1D00,data));
	m_led_update=FALSE;
}

/* Write Port $1E00 */

WRITE8_MEMBER( supercon_state::supercon_port3_w )
{
	if (data)
		LOG(("Write from %04x data: %02x\n",0x1E00,data));

	if (data)
	{
		m_data_1E00=data;

/* Set bits for LED's */

		if ( m_data_1F00)
		{
			if (m_data_1F00 & LED_LINE_AH )
			{
				m_last_LED = &m_LED_AH;             /* save last value */
				m_last_LED_value = *m_last_LED;

				m_LED_AH=m_LED_AH | m_data_1E00;
			}
			else if (m_data_1F00 & LED_LINE_ST )
			{
				m_last_LED = &m_LED_ST;
				m_last_LED_value = *m_last_LED;

				m_LED_ST=m_LED_ST | m_data_1E00;
			}
			else if (m_data_1F00 &  LED_LINE_18)
			{
				m_last_LED = &m_LED_18;
				m_last_LED_value = *m_last_LED;

				m_LED_18=m_LED_18 | m_data_1E00;
			}
		}
	}

	// The IRQ handler writes to both 0x1E00 (1x) and 0x1F00 (2x) ports.
	// Not 100% sure if this is the correct place for clearing the IRQ
	// line.
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

/* Write Port $1F00 */

WRITE8_MEMBER( supercon_state::supercon_port4_w )
{
	if (data)
		LOG(("Write from %04x data: %02x\n",0x1F00,data));

	if (data)
		m_data_1F00=data;

/* Bit 7 is set -> Buzzer on */

	if ( m_data_1F00 & 0x80 )
	{
		m_beep->set_state(1);
		m_emu_started=TRUE;
	}
	else
		m_beep->set_state(0);
}

TIMER_CALLBACK_MEMBER(supercon_state::mouse_click)
{
	m_selecting=FALSE;
}

TIMER_DEVICE_CALLBACK_MEMBER(supercon_state::update_artwork)
{
	mouse_update();
}

TIMER_CALLBACK_MEMBER(supercon_state::update_irq)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
}

/* Save state call backs */

void supercon_state::board_presave()
{
	for (int i=0;i<64;i++)
	{
		m_save_board[i] = m_board[i];
	}
}

void supercon_state::board_postload()
{
	for (int i=0;i<64;i++)
	{
		m_board[i] = m_save_board[i];
	}

	set_pieces();
}

void supercon_state::machine_start()
{
	m_timer_update_irq = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supercon_state::update_irq),this));
	m_timer_update_irq->adjust( attotime::zero, 0, attotime::from_hz(1000) );
	m_timer_mouse_click =  machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supercon_state::mouse_click),this),nullptr);
	save_item(NAME(m_save_board));
	machine().save().register_postload(save_prepost_delegate(FUNC(supercon_state::board_postload),this));
	machine().save().register_presave(save_prepost_delegate(FUNC(supercon_state::board_presave),this));
}

void supercon_state::machine_reset()
{
	set_board();
	set_pieces();
	set_border_pieces();

	m_emu_started=FALSE;
}

/* Address maps */

static ADDRESS_MAP_START(supercon_mem, AS_PROGRAM, 8, supercon_state)
	AM_RANGE( 0x0000, 0x0fff) AM_RAM
	AM_RANGE( 0x2000, 0x7fff) AM_ROM
	AM_RANGE( 0x8000, 0xffff) AM_ROM

	AM_RANGE( 0x1C00, 0x1C00) AM_WRITE ( supercon_port1_w )
	AM_RANGE( 0x1D00, 0x1D00) AM_WRITE ( supercon_port2_w )
	AM_RANGE( 0x1E00, 0x1E00) AM_WRITE ( supercon_port3_w )
	AM_RANGE( 0x1F00, 0x1F00) AM_WRITE ( supercon_port4_w )

	AM_RANGE( 0x1C00, 0x1C00) AM_READ ( supercon_port1_r )
	AM_RANGE( 0x1D00, 0x1D00) AM_READ ( supercon_port2_r )
	AM_RANGE( 0x1E00, 0x1E00) AM_READ ( supercon_port3_r )
	AM_RANGE( 0x1F00, 0x1F00) AM_READ ( supercon_port4_r )
ADDRESS_MAP_END

/* Input ports */

static INPUT_PORTS_START( supercon )

	PORT_START("BOARD_1")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("BOARD_2")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("BOARD_3")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("BOARD_4")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("BOARD_5")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("BOARD_6")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("BOARD_7")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("BOARD_8")
	PORT_BIT(0x01,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_1")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_2")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_3")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_4")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_5")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_6")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_7")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("STATUS_8")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)

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

	PORT_START("B_CLR")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD)

INPUT_PORTS_END

/* Machine driver */
static MACHINE_CONFIG_START( supercon, supercon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(supercon_mem)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", supercon_state, update_artwork, attotime::from_hz(20))
MACHINE_CONFIG_END

/* ROM definition */

ROM_START(supercon)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("supercon_low.bin",  0x0000,  0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0 ))
	ROM_LOAD("supercon_high.bin", 0x8000,  0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe ))
ROM_END

/* Driver */

/*    YEAR  NAME          PARENT  COMPAT  MACHINE    INPUT       INIT      COMPANY  FULLNAME                     FLAGS */
CONS( 1983, supercon,     0,      0,      supercon,  supercon, supercon_state,   supercon, "Novag", "SuperConstellation", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
