// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Tasc SmartBoard

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
	, m_out_leds(*this, "led_%u%u", 0U, 0U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tasc_sb30_device::device_start()
{
	m_out_leds.resolve();

	save_item(NAME(m_data));
	save_item(NAME(m_position));
	save_item(NAME(m_shift));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tasc_sb30_device::device_reset()
{
	m_data = 0;
	m_position = 0;
	m_shift = 0;
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

u8 tasc_sb30_device::read()
{
	if (m_position < 0x40)
	{
		int x = 7 - m_position / 8;
		int y = 7 - m_position % 8;
		int piece_id = m_board->read_sensor(x, y);

		// each piece is identified by a single bit in a 32-bit sequence, if multiple bits are active the MSB is used
		u32 sb30_id = 0;
		if (piece_id > 0)
			sb30_id = 1UL << (piece_id - 1);

		return BIT(sb30_id, m_shift & 0x1f);
	}

	return 0;
}

void tasc_sb30_device::write(u8 data)
{
	if (!BIT(data, 3) && BIT(m_data, 3))
		m_position = 0;

	if (!BIT(data, 6) && BIT(m_data, 6))
	{
		if (m_position < 0x40)
		{
			int x = m_position / 8;
			int y = m_position % 8;
			m_out_leds[y][x] = BIT(data, 7);
		}

		m_shift = 0;
	}

	if (!BIT(data, 7) && BIT(m_data, 7))
	{
		m_position++;

		if (m_position == 0x40)
			m_shift++;
	}

	m_data = data;
}
