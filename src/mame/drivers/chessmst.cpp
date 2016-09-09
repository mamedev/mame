// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
//                   fhub (support for external artwork with chess pieces)
/***************************************************************************

        Chess-Master

        TODO:
        - figure out why chessmsta won't work, for starters it assume z80 carry flag is set at poweron?

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/speaker.h"
#include "chessmst.lh"

class chessmst_state : public driver_device
{
public:
	chessmst_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
	{ }

	DECLARE_WRITE8_MEMBER( pio1_port_a_w );
	DECLARE_WRITE8_MEMBER( pio1_port_b_w );
	DECLARE_READ8_MEMBER( pio2_port_a_r );
	DECLARE_WRITE8_MEMBER( pio2_port_b_w );
	DECLARE_INPUT_CHANGED_MEMBER(chessmst_sensor);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	DECLARE_INPUT_CHANGED_MEMBER(clear_pieces);
	TIMER_DEVICE_CALLBACK_MEMBER(led_update);

private:

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void set_board();
	void set_pieces();
	void clear_pieces();
	void setup_board(UINT8 pos);
	void board_presave();
	void board_postload();
	UINT8 m_lastfield, m_lastdata, m_lastpiece, m_thinking;
	UINT16 m_matrix;
	UINT16 m_led_sel;
	bool m_started, m_moveok, m_setup, m_capture, m_castling, m_enpassant, m_cm;
	UINT8 m_save_board[64], m_save_sensor[64], m_sensor[64], m_board[64];
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
};

enum
{
	EM,     // 0 = No piece
	BP, BN, BB, BR, BQ, BK, // 1.. 6 = Black pieces
	WP, WN, WB, WR, WQ, WK  // 7..12 = White pieces
};

void chessmst_state::set_board()
{
	static const UINT8 start_board[64] = {
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

void chessmst_state::set_pieces()
{
	for (UINT8 i=0;i<64;i++)
		output().set_indexed_value("P", i, m_board[i]);
}

INPUT_CHANGED_MEMBER(chessmst_state::clear_pieces)
{
	if (!m_started)
	{ // only immediately after start or reset!
		m_started=true;
		memset(m_sensor, 1, sizeof(m_sensor));
		for (UINT8 i=0;i<64;i++)
		{
			m_board[i]=EM;
			output().set_indexed_value("P", i, EM);
		}
	}
}

void chessmst_state::board_presave()
{
	for (UINT8 i=0;i<64;i++)
		m_save_sensor[i] = m_sensor[i];
	for (UINT8 i=0;i<64;i++)
		m_save_board[i] = m_board[i];
}

void chessmst_state::board_postload()
{
	for (UINT8 i=0;i<64;i++)
		m_sensor[i] = m_save_sensor[i];
	for (UINT8 i=0;i<64;i++)
		m_board[i] = m_save_board[i];

	set_pieces();
}

void chessmst_state::machine_start()
{
	save_item(NAME(m_save_sensor));
	save_item(NAME(m_save_board));
	machine().save().register_postload(save_prepost_delegate(FUNC(chessmst_state::board_postload),this));
	machine().save().register_presave(save_prepost_delegate(FUNC(chessmst_state::board_presave),this));
}

void chessmst_state::machine_reset()
{
	//reset all sensors
	memset(m_sensor, 1, sizeof(m_sensor));

	//positioning the pieces for start next game
	for (int i=0; i<64; i+=8)
		m_sensor[i+0] = m_sensor[i+1] = m_sensor[i+6] = m_sensor[i+7] = 0;

	set_board();
	set_pieces();
	m_lastfield=99;
	m_lastpiece=EM;
	m_thinking=0;
	m_started=false;
	m_setup=false;
	m_capture=false; 
	m_castling=false;
	m_enpassant=false;
	m_moveok=true;
	m_cm = false;
}

INPUT_CHANGED_MEMBER(chessmst_state::reset_button)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	machine_reset();
}

TIMER_DEVICE_CALLBACK_MEMBER(chessmst_state::led_update)
{
	switch (m_thinking)
	{
		case 0:
			m_thinking=1;
			break;
		case 1: if (m_started)
				{
				if(!m_capture && !m_castling && !m_enpassant)
				{
					m_thinking=2;
					for (UINT8 i=1; i<=8; i++)
					{
						output().set_indexed_value("led_a", i, 1);
						output().set_indexed_value("led_b", i, 1);
						output().set_indexed_value("led_c", i, 1);
						output().set_indexed_value("led_d", i, 1);
						output().set_indexed_value("led_e", i, 1);
						output().set_indexed_value("led_f", i, 1);
						output().set_indexed_value("led_g", i, 1);
						output().set_indexed_value("led_h", i, 1);
						output().set_indexed_value("led_i", i, 1);
						output().set_indexed_value("led_j", i, 1);
					}
				}
				m_cm = !m_cm;
				output().set_indexed_value("led_j", 6, m_cm);
			}
			break;
		case 2: m_cm = !m_cm;
			output().set_indexed_value("led_j", 6, m_cm);
	}
}

void chessmst_state::setup_board(UINT8 pos)
{
	static const UINT8 s_board[64] = {
	BR, BN, BB, BQ, BK, BB, BN, BR,
	BP, BP, BP, BP, BP, BP, BP, BP,
	BR, BN, BB, BQ, BK, BB, BN, BR,
	BR, BN, BB, BQ, BK, BB, BN, BR,
	WR, WN, WB, WQ, WK, WB, WN, WR,
	WR, WN, WB, WQ, WK, WB, WN, WR,
	WP, WP, WP, WP, WP, WP, WP, WP,
	WR, WN, WB, WQ, WK, WB, WN, WR };

	UINT8 color, currfield, currpiece;

	color=(m_lastpiece?((m_lastpiece-1)%12)/6:2);
	currfield=(7-pos%8)*8+pos/8;
	currpiece=m_board[currfield];
	if (m_setup)
	{
		if (m_sensor[pos])
		{ // piece up! 0 -> 1
			if (m_lastfield!=99)
			{ // remove last marked piece
				m_board[m_lastfield]=EM;
				output().set_indexed_value("P", m_lastfield, EM);
			}
			m_board[currfield]+=12;
			output().set_indexed_value("P", currfield, m_board[currfield]);
			m_lastfield=currfield;
			m_lastpiece=currpiece;
		}
		else
		{ // piece down! 1 -> 0
			if (m_lastfield!=99) { // set last marked piece
				m_board[m_lastfield]=EM;
				output().set_indexed_value("P", m_lastfield, EM);
				m_board[currfield]=m_lastpiece;
				output().set_indexed_value("P", currfield, m_lastpiece);
			}
			else
			{ // set new piece
				currpiece=s_board[currfield];
				m_board[currfield]=currpiece;
				output().set_indexed_value("P", currfield, currpiece);
			}
			m_lastfield=99;
			m_lastpiece=EM;
		}
	}
	else
	{
		if (m_sensor[pos])
		{ // piece up! 0 -> 1
			if (currpiece==EM)
				return;
			if (m_moveok)
			{ // 1st piece up!
				if (m_enpassant)
				{
					m_board[currfield]=EM;
					output().set_indexed_value("P", currfield, EM);
					m_enpassant=false;
					return;
				}
				if (m_castling || (m_lastpiece==EM) || ((currpiece-1)/6!=color))
				{
					m_board[currfield]+=12;
					output().set_indexed_value("P", currfield, m_board[currfield]);
				}
				else
				{
					m_board[currfield]=EM;
					output().set_indexed_value("P", currfield, EM);
				}
				m_moveok=false;
			}
			else
			{ // 2nd piece up!
				if (m_board[m_lastfield]>12)
				{
					m_board[currfield]=EM;
					output().set_indexed_value("P", currfield, EM);
					m_capture=true;
					return;
				}
				else
				{
					m_board[currfield]+=12;
					output().set_indexed_value("P", currfield, m_board[currfield]);
				}
			}
			m_lastfield=currfield;
			m_lastpiece=currpiece;
		}
		else
		{ // piece down! 1 -> 0
			if (m_lastpiece==EM)
				return;
			if (currfield==m_lastfield)
			{ // correct wrong click!
				m_board[currfield]=m_lastpiece;
				output().set_indexed_value("P", currfield, m_lastpiece);
				m_lastpiece=(m_lastpiece+5)%12+1;
			}
			else
			{ // normal move
				m_board[m_lastfield]=EM;
				output().set_indexed_value("P", m_lastfield, EM);
				if ((m_lastpiece%6==1) && (abs(2*(currfield/8)-7)==7))
					m_lastpiece=(color?WQ:BQ); // promotion?
				m_board[currfield]=m_lastpiece;
				output().set_indexed_value("P", currfield, m_lastpiece);
				m_capture=false;
				m_castling=false;
				m_enpassant=false;
				if ((m_lastpiece%6==0) && (abs(currfield-m_lastfield)==2))
					m_castling=true; // castling?
				else
				if ((m_lastpiece%6==1) && (m_lastfield/8+color==4) && abs(8-abs(currfield-m_lastfield))==1 
					&& (m_board[m_lastfield+(2*color-1)*(8-abs(currfield-m_lastfield))]%6==1))
						m_enpassant=true; // enpassant?
			}
			m_moveok=true;
		}
	}
}

INPUT_CHANGED_MEMBER(chessmst_state::chessmst_sensor)
{
	UINT8 pos = (UINT8)(FPTR)param;

	if (newval)
	{
		m_sensor[pos] = !m_sensor[pos];
		setup_board(pos);
	}
}

WRITE8_MEMBER( chessmst_state::pio1_port_a_w )
{
	for (int row=1; row<=8; row++)
	{
		if (m_led_sel & 0x01)
			output().set_indexed_value("led_a", row, BIT(data, 8-row));
		if (m_led_sel & 0x02)
			output().set_indexed_value("led_b", row, BIT(data, 8-row));
		if (m_led_sel & 0x04)
			output().set_indexed_value("led_c", row, BIT(data, 8-row));
		if (m_led_sel & 0x08)
			output().set_indexed_value("led_d", row, BIT(data, 8-row));
		if (m_led_sel & 0x10)
			output().set_indexed_value("led_e", row, BIT(data, 8-row));
		if (m_led_sel & 0x20)
			output().set_indexed_value("led_f", row, BIT(data, 8-row));
		if (m_led_sel & 0x40)
			output().set_indexed_value("led_g", row, BIT(data, 8-row));
		if (m_led_sel & 0x80)
			output().set_indexed_value("led_h", row, BIT(data, 8-row));
		if (m_led_sel & 0x100)
			output().set_indexed_value("led_i", row, BIT(data, 8-row));
		if (m_led_sel & 0x200)
		{
			output().set_indexed_value("led_j", row, BIT(data, 8-row));
			if ((data&0x01)==m_setup)
			{ // 'CHANGEBOARD'-LED -> switch setup mode
				m_setup=!BIT(data, 0);
				m_started=true;
				if ((m_lastfield!=99) && (m_board[m_lastfield]>12))
				{
					m_board[m_lastfield]=EM;
					output().set_indexed_value("P", m_lastfield, EM);
				}
				m_lastfield=99;
				m_lastpiece=EM;
				m_capture=false;
				m_castling=false;
				m_enpassant=false;
				m_moveok=true;
			}
		}
	}

	m_led_sel = 0;
	m_thinking=0;
}

WRITE8_MEMBER( chessmst_state::pio1_port_b_w )
{
	m_matrix = (m_matrix & 0xff) | ((data & 0x01)<<8);
	m_led_sel = (m_led_sel & 0xff) | ((data & 0x03)<<8);

	m_speaker->level_w(BIT(data, 6));
}

READ8_MEMBER( chessmst_state::pio2_port_a_r )
{
	UINT8 data = 0x00;

	// The pieces position on the chessboard is identified by 64 Hall
	// sensors, which are in a 8x8 matrix with the corresponding LEDs.
	for (int i=0; i<8; i++)
	{
		if (m_matrix & 0x01)
			data |= (m_sensor[0+i] ? (1<<i) : 0);
		if (m_matrix & 0x02)
			data |= (m_sensor[8+i] ? (1<<i) : 0);
		if (m_matrix & 0x04)
			data |= (m_sensor[16+i] ? (1<<i) : 0);
		if (m_matrix & 0x08)
			data |= (m_sensor[24+i] ? (1<<i) : 0);
		if (m_matrix & 0x10)
			data |= (m_sensor[32+i] ? (1<<i) : 0);
		if (m_matrix & 0x20)
			data |= (m_sensor[40+i] ? (1<<i) : 0);
		if (m_matrix & 0x40)
			data |= (m_sensor[48+i] ? (1<<i) : 0);
		if (m_matrix & 0x80)
			data |= (m_sensor[56+i] ? (1<<i) : 0);
	}
	if (m_matrix & 0x100)
	{
		data |= ioport("BUTTONS")->read();
		if (data != m_lastdata)
		{
			m_lastdata=data;
			if ((data&0x80) || (data&0x10))
				m_started=true; // NEWGAME or BOARD -> switch OFF clear board
			else
			if (m_setup)
			{
				if (BIT(data, 0))
				{ // HINT -> delete current piece
					if (m_lastfield!=99)
					{
						m_board[m_lastfield]=EM;
						output().set_indexed_value("P", m_lastfield, EM);
						m_lastfield=99;
						m_lastpiece=EM;
					}
				}
			}
		}
	}
	return data;
}

WRITE8_MEMBER( chessmst_state::pio2_port_b_w )
{
	m_matrix = (data & 0xff) | (m_matrix & 0x100);
	m_led_sel = (data & 0xff) | (m_led_sel & 0x300);
}

static const z80_daisy_config chessmst_daisy_chain[] =
{
	{ "z80pio1" },
	{ nullptr }
};

static ADDRESS_MAP_START(chessmst_mem, AS_PROGRAM, 8, chessmst_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7fff) // A15 not connected
	AM_RANGE( 0x0000, 0x27ff ) AM_ROM
	AM_RANGE( 0x3400, 0x3bff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( chessmst_io , AS_IO, 8, chessmst_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x03) AM_MIRROR(0xf0) read/write in both, not used by the software
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xf0) AM_DEVREADWRITE("z80pio1", z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xf0) AM_DEVREADWRITE("z80pio2", z80pio_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( chessmst )
	PORT_START("COL_A")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  2)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  5)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  6)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  7)
	PORT_START("COL_B")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  8)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  9)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 10)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 11)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 12)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 13)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 14)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 15)
	PORT_START("COL_C")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 16)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 17)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 18)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 19)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 20)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 21)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 22)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 23)
	PORT_START("COL_D")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 24)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 25)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 26)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 27)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 28)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 29)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 30)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 31)
	PORT_START("COL_E")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 32)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 33)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 34)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 35)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 36)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 37)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 38)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 39)
	PORT_START("COL_F")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 40)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 41)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 42)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 43)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 44)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 45)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 46)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 47)
	PORT_START("COL_G")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 48)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 49)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 50)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 51)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 52)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 53)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 54)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 55)
	PORT_START("COL_H")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 56)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 57)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 58)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 59)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 60)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 61)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 62)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 63)

	PORT_START("BUTTONS")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Hint     [7]")    PORT_CODE(KEYCODE_7)    PORT_CODE(KEYCODE_H)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Random   [6]")    PORT_CODE(KEYCODE_6)    PORT_CODE(KEYCODE_R)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Referee  [5]")    PORT_CODE(KEYCODE_5)    PORT_CODE(KEYCODE_F)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Selfplay [4]")    PORT_CODE(KEYCODE_4)    PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board    [3]")    PORT_CODE(KEYCODE_3)    PORT_CODE(KEYCODE_B)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Color    [2]")    PORT_CODE(KEYCODE_2)    PORT_CODE(KEYCODE_C)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Level    [1]")    PORT_CODE(KEYCODE_1)    PORT_CODE(KEYCODE_L)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("New Game [0]")    PORT_CODE(KEYCODE_0)    PORT_CODE(KEYCODE_N)

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Halt")  PORT_CODE(KEYCODE_ESC) PORT_WRITE_LINE_DEVICE_MEMBER("z80pio1", z80pio_device, strobe_a) // -> PIO(1) ASTB pin
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, reset_button, 0) // -> Z80 RESET pin

	PORT_START("CLEAR")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, clear_pieces, 0)
INPUT_PORTS_END

static MACHINE_CONFIG_START( chessmst, chessmst_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_9_8304MHz/4) // U880 Z80 clone
	MCFG_CPU_PROGRAM_MAP(chessmst_mem)
	MCFG_CPU_IO_MAP(chessmst_io)
	MCFG_Z80_DAISY_CHAIN(chessmst_daisy_chain)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("led_update", chessmst_state, led_update, attotime::from_msec(250))

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL_9_8304MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(chessmst_state, pio1_port_a_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio1_port_b_w))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL_9_8304MHz/4)
	MCFG_Z80PIO_IN_PA_CB(READ8(chessmst_state, pio2_port_a_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio2_port_b_w))

	MCFG_DEFAULT_LAYOUT(layout_chessmst)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( chessmsta, chessmst_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz/4) // U880 Z80 clone
	MCFG_CPU_PROGRAM_MAP(chessmst_mem)
	MCFG_CPU_IO_MAP(chessmst_io)
	MCFG_Z80_DAISY_CHAIN(chessmst_daisy_chain)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("led_update", chessmst_state, led_update, attotime::from_msec(310))

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL_8MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(chessmst_state, pio1_port_a_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio1_port_b_w))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL_8MHz/4)
	MCFG_Z80PIO_IN_PA_CB(READ8(chessmst_state, pio2_port_a_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio2_port_b_w))

	MCFG_DEFAULT_LAYOUT(layout_chessmst)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( chessmst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "056.bin", 0x0000, 0x0400, CRC(2b90e5d3) SHA1(c47445964b2e6cb11bd1f27e395cf980c97af196) )
	ROM_LOAD( "057.bin", 0x0400, 0x0400, CRC(e666fc56) SHA1(3fa75b82cead81973bea94191a5c35f0acaaa0e6) )
	ROM_LOAD( "058.bin", 0x0800, 0x0400, CRC(6a17fbec) SHA1(019051e93a5114477c50eaa87e1ff01b02eb404d) )
	ROM_LOAD( "059.bin", 0x0c00, 0x0400, CRC(e96e3d07) SHA1(20fab75f206f842231f0414ebc473ce2a7371e7f) )
	ROM_LOAD( "060.bin", 0x1000, 0x0400, CRC(0e31f000) SHA1(daac924b79957a71a4b276bf2cef44badcbe37d3) )
	ROM_LOAD( "061.bin", 0x1400, 0x0400, CRC(69ad896d) SHA1(25d999b59d4cc74bd339032c26889af00e64df60) )
	ROM_LOAD( "062.bin", 0x1800, 0x0400, CRC(c42925fe) SHA1(c42d8d7c30a9b6d91ac994cec0cc2723f41324e9) )
	ROM_LOAD( "063.bin", 0x1c00, 0x0400, CRC(86be4cdb) SHA1(741f984c15c6841e227a8722ba30cf9e6b86d878) )
	ROM_LOAD( "064.bin", 0x2000, 0x0400, CRC(e82f5480) SHA1(38a939158052f5e6484ee3725b86e522541fe4aa) )
	ROM_LOAD( "065.bin", 0x2400, 0x0400, CRC(4ec0e92c) SHA1(0b748231a50777391b04c1778750fbb46c21bee8) )
ROM_END

ROM_START( chessmsta )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2764.bin",       0x0000, 0x2000, CRC(6be28876) SHA1(fd7d77b471e7792aef3b2b3f7ff1de4cdafc94c9) )
	ROM_LOAD( "u2616bm108.bin", 0x2000, 0x0800, CRC(6e69ace3) SHA1(e099b6b6cc505092f64b8d51ab9c70aa64f58f70) )
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT COMPAT MACHINE    INPUT     INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1984, chessmst,  0,        0,  chessmst,  chessmst, driver_device, 0, "VEB Mikroelektronik Erfurt", "Chess-Master (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
COMP( 1984, chessmsta, chessmst, 0,  chessmsta, chessmst, driver_device, 0, "VEB Mikroelektronik Erfurt", "Chess-Master (set 2)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
