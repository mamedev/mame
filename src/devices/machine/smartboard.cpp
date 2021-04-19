// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/******************************************************************************

Tasc SmartBoard SB30

Chessboard controller for use with Tasc R30 chesscomputer, or as PC peripheral.

SB30 (81 LEDs) is "SmartBoard I"
SB20 (64 LEDs) is "SmartBoard II"

The SmartBoard can detect which piece is present on a specific square, more
info on the technology used in the piece recognition system can be found in
the US patent 5,129,654

******************************************************************************/

#include "emu.h"
#include "smartboard.h"

// SB30 chesspiece IDs: 0-31 (+1 for MAME sensorboard_device)
enum
{
	SB30_WHITE_KNIGHT1 = 1,
	SB30_WHITE_KNIGHT2,
	SB30_BLACK_KING,
	SB30_WHITE_KING,
	SB30_BLACK_QUEEN,
	SB30_WHITE_QUEEN,
	SB30_BLACK_ROOK1,
	SB30_BLACK_ROOK2,
	SB30_WHITE_ROOK1,
	SB30_WHITE_ROOK2,
	SB30_BLACK_BISHOP1,
	SB30_BLACK_BISHOP2,
	SB30_WHITE_BISHOP1,
	SB30_WHITE_BISHOP2,
	SB30_BLACK_KNIGHT1,
	SB30_BLACK_KNIGHT2,
	SB30_WHITE_PAWN1,
	SB30_WHITE_PAWN2,
	SB30_WHITE_PAWN3,
	SB30_WHITE_PAWN4,
	SB30_WHITE_PAWN5,
	SB30_WHITE_PAWN6,
	SB30_WHITE_PAWN7,
	SB30_WHITE_PAWN8,
	SB30_BLACK_PAWN1,
	SB30_BLACK_PAWN2,
	SB30_BLACK_PAWN3,
	SB30_BLACK_PAWN4,
	SB30_BLACK_PAWN5,
	SB30_BLACK_PAWN6,
	SB30_BLACK_PAWN7,
	SB30_BLACK_PAWN8
};

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TASC_SB30, tasc_sb30_device, "tasc_sb30", "Tasc SmartBoard SB30")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tasc_sb30_device - constructor
//-------------------------------------------------

tasc_sb30_device::tasc_sb30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TASC_SB30, tag, owner, clock)
	, m_board(*this, "board")
	, m_out_leds(*this, "sb30_led_%u.%u", 0U, 0U)
	, m_data_out(*this)
	, m_led_out(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tasc_sb30_device::device_start()
{
	m_led_out.resolve();
	if (m_led_out.isnull())
		m_out_leds.resolve();

	m_data_out.resolve_safe();

	std::fill(std::begin(m_squares), std::end(m_squares), 0);

	// register for savestates
	save_item(NAME(m_reset));
	save_item(NAME(m_data0));
	save_item(NAME(m_data1));
	save_item(NAME(m_output));
	save_item(NAME(m_pos));
	save_item(NAME(m_shift));
	save_item(NAME(m_squares));
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void tasc_sb30_device::device_add_mconfig(machine_config &config)
{
	SENSORBOARD(config, m_board);
	m_board->set_type(sensorboard_device::INDUCTIVE);
	m_board->set_max_id(32);
	m_board->init_cb().set(FUNC(tasc_sb30_device::init_cb));
	m_board->spawn_cb().set(FUNC(tasc_sb30_device::spawn_cb));
}


//-------------------------------------------------
//  sensorboard_device interface
//-------------------------------------------------

void tasc_sb30_device::init_cb(int state)
{
	m_board->clear_board();
	m_board->write_piece(0, 0, SB30_WHITE_ROOK1);
	m_board->write_piece(7, 0, SB30_WHITE_ROOK2);
	m_board->write_piece(1, 0, SB30_WHITE_KNIGHT1);
	m_board->write_piece(6, 0, SB30_WHITE_KNIGHT2);
	m_board->write_piece(2, 0, SB30_WHITE_BISHOP1);
	m_board->write_piece(5, 0, SB30_WHITE_BISHOP2);
	m_board->write_piece(3, 0, SB30_WHITE_QUEEN);
	m_board->write_piece(4, 0, SB30_WHITE_KING);
	m_board->write_piece(0, 7, SB30_BLACK_ROOK1);
	m_board->write_piece(7, 7, SB30_BLACK_ROOK2);
	m_board->write_piece(1, 7, SB30_BLACK_KNIGHT1);
	m_board->write_piece(6, 7, SB30_BLACK_KNIGHT2);
	m_board->write_piece(2, 7, SB30_BLACK_BISHOP1);
	m_board->write_piece(5, 7, SB30_BLACK_BISHOP2);
	m_board->write_piece(3, 7, SB30_BLACK_QUEEN);
	m_board->write_piece(4, 7, SB30_BLACK_KING);

	for (int x = 0; x < 8; x++)
	{
		m_board->write_piece(x, 1, SB30_WHITE_PAWN1 + x);
		m_board->write_piece(x, 6, SB30_BLACK_PAWN1 + x);
	}
}

bool tasc_sb30_device::piece_available(u8 id)
{
	for (int y = 0; y < 8; y++)
		for (int x = 0; x < 8; x++)
		{
			if (m_board->read_piece(x, y) == id)
				return false;
		}

	return true;
}

u8 tasc_sb30_device::spawn_cb(offs_t offset)
{
	int piece_id = 0;

	// While most software works fine as long as color and piece type can be distinguished,
	// each individual chesspiece is expected to have a unique id.
	switch (offset)
	{
		case 1+0:
			for (int p = 0; p < 8; p++)
				if (piece_available(SB30_WHITE_PAWN1 + p))
				{
					piece_id = SB30_WHITE_PAWN1 + p;
					break;
				}
			break;

		case 1+6:
			for (int p = 0; p < 8; p++)
				if (piece_available(SB30_BLACK_PAWN1 + p))
				{
					piece_id = SB30_BLACK_PAWN1 + p;
					break;
				}
			break;

		case 2+0:
			if (piece_available(SB30_WHITE_KNIGHT1))
				piece_id = SB30_WHITE_KNIGHT1;
			else if (piece_available(SB30_WHITE_KNIGHT2))
				piece_id = SB30_WHITE_KNIGHT2;
			break;

		case 2+6:
			if (piece_available(SB30_BLACK_KNIGHT1))
				piece_id = SB30_BLACK_KNIGHT1;
			else if (piece_available(SB30_BLACK_KNIGHT2))
				piece_id = SB30_BLACK_KNIGHT2;
			break;

		case 3+0:
			if (piece_available(SB30_WHITE_BISHOP1))
				piece_id = SB30_WHITE_BISHOP1;
			else if (piece_available(SB30_WHITE_BISHOP2))
				piece_id = SB30_WHITE_BISHOP2;
			break;

		case 3+6:
			if (piece_available(SB30_BLACK_BISHOP1))
				piece_id = SB30_BLACK_BISHOP1;
			else if (piece_available(SB30_BLACK_BISHOP2))
				piece_id = SB30_BLACK_BISHOP2;
			break;

		case 4+0:
			if (piece_available(SB30_WHITE_ROOK1))
				piece_id = SB30_WHITE_ROOK1;
			else if (piece_available(SB30_WHITE_ROOK2))
				piece_id = SB30_WHITE_ROOK2;
			break;

		case 4+6:
			if (piece_available(SB30_BLACK_ROOK1))
				piece_id = SB30_BLACK_ROOK1;
			else if (piece_available(SB30_BLACK_ROOK2))
				piece_id = SB30_BLACK_ROOK2;
			break;

		case 5+0:
			if (piece_available(SB30_WHITE_QUEEN))
				piece_id = SB30_WHITE_QUEEN;
			break;

		case 5+6:
			if (piece_available(SB30_BLACK_QUEEN))
				piece_id = SB30_BLACK_QUEEN;
			break;

		case 6+0:
			if (piece_available(SB30_WHITE_KING))
				piece_id = SB30_WHITE_KING;
			break;

		case 6+6:
			if (piece_available(SB30_BLACK_KING))
				piece_id = SB30_BLACK_KING;
			break;

		default:
			break;
	}

	return piece_id;
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void tasc_sb30_device::reset_w(int state)
{
	state = state ? 1 : 0;

	if (!state && m_reset)
	{
		m_pos = 0;
		update_output();
	}

	m_reset = state;
}

void tasc_sb30_device::data0_w(int state)
{
	state = state ? 1 : 0;

	if (!state && m_data0)
	{
		if (m_pos < 0x40)
		{
			// output board led(s)
			if (m_led_out.isnull())
				m_out_leds[m_pos & 7][m_pos >> 3] = m_data1;
			else
				m_led_out(m_pos, m_data1);
		}
	}

	m_data0 = state;
}

void tasc_sb30_device::data1_w(int state)
{
	state = state ? 1 : 0;

	if (!state && m_data1)
	{
		m_pos++;

		if ((m_pos & 0x3f) == 0)
			m_shift++;

		if (m_data0)
		{
			m_shift = 0;

			// start scan
			scan_board();
		}

		update_output();
	}

	m_data1 = state;
}

void tasc_sb30_device::scan_board()
{
	for (int i = 0; i < 64; i++)
	{
		// each piece is identified by a single bit in a 32-bit sequence
		int piece_id = m_board->read_sensor(i >> 3 ^ 7, ~i & 7);
		m_squares[i] = piece_id ? (1 << (piece_id - 1)) : 0;
	}
}

void tasc_sb30_device::update_output()
{
	m_output = BIT(m_squares[m_pos & 0x3f], m_shift & 0x1f);
	m_data_out(m_output);
}
