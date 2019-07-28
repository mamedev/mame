// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Tasc SmartBoard

    The SmartBoard can detect which piece is present on a specific square, more
    info on the technology used in the piece recognition system can be found in
    the US patent 5,129,654

******************************************************************************/

#include "emu.h"
#include "smartboard.h"

enum
{
	SB30_WHITE_KNIGHT1  = 0,
	SB30_WHITE_KNIGHT2  = 1,
	SB30_BLACK_KING     = 2,
	SB30_WHITE_KING     = 3,
	SB30_BLACK_QUEEN    = 4,
	SB30_WHITE_QUEEN    = 5,
	SB30_BLACK_ROOK1    = 6,
	SB30_BLACK_ROOK2    = 7,
	SB30_WHITE_ROOK1    = 8,
	SB30_WHITE_ROOK2    = 9,
	SB30_BLACK_BISHOP1  = 10,
	SB30_BLACK_BISHOP2  = 11,
	SB30_WHITE_BISHOP1  = 12,
	SB30_WHITE_BISHOP2  = 13,
	SB30_BLACK_KNIGHT1  = 14,
	SB30_BLACK_KNIGHT2  = 15,
	SB30_WHITE_PAWN1    = 16,
	SB30_WHITE_PAWN2    = 17,
	SB30_WHITE_PAWN3    = 18,
	SB30_WHITE_PAWN4    = 19,
	SB30_WHITE_PAWN5    = 20,
	SB30_WHITE_PAWN6    = 21,
	SB30_WHITE_PAWN7    = 22,
	SB30_WHITE_PAWN8    = 23,
	SB30_BLACK_PAWN1    = 24,
	SB30_BLACK_PAWN2    = 25,
	SB30_BLACK_PAWN3    = 26,
	SB30_BLACK_PAWN4    = 27,
	SB30_BLACK_PAWN5    = 28,
	SB30_BLACK_PAWN6    = 29,
	SB30_BLACK_PAWN7    = 30,
	SB30_BLACK_PAWN8    = 31,
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

tasc_sb30_device::tasc_sb30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
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
	m_leds_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tasc_sb30_device::leds_off_cb), this));

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


TIMER_CALLBACK_MEMBER(tasc_sb30_device::leds_off_cb)
{
	for (int y=0; y<9; y++)
		for (int x=0; x<9; x++)
			m_out_leds[y][x] = 0;
}

void tasc_sb30_device::init_cb(int state)
{
	m_board->clear_board();
	m_board->write_piece(0, 0, 1 + SB30_WHITE_ROOK1);
	m_board->write_piece(7, 0, 1 + SB30_WHITE_ROOK2);
	m_board->write_piece(1, 0, 1 + SB30_WHITE_KNIGHT1);
	m_board->write_piece(6, 0, 1 + SB30_WHITE_KNIGHT2);
	m_board->write_piece(2, 0, 1 + SB30_WHITE_BISHOP1);
	m_board->write_piece(5, 0, 1 + SB30_WHITE_BISHOP2);
	m_board->write_piece(3, 0, 1 + SB30_WHITE_QUEEN);
	m_board->write_piece(4, 0, 1 + SB30_WHITE_KING);
	m_board->write_piece(0, 7, 1 + SB30_BLACK_ROOK1);
	m_board->write_piece(7, 7, 1 + SB30_BLACK_ROOK2);
	m_board->write_piece(1, 7, 1 + SB30_BLACK_KNIGHT1);
	m_board->write_piece(6, 7, 1 + SB30_BLACK_KNIGHT2);
	m_board->write_piece(2, 7, 1 + SB30_BLACK_BISHOP1);
	m_board->write_piece(5, 7, 1 + SB30_BLACK_BISHOP2);
	m_board->write_piece(3, 7, 1 + SB30_BLACK_QUEEN);
	m_board->write_piece(4, 7, 1 + SB30_BLACK_KING);

	for (int x = 0; x < 8; x++)
	{
		m_board->write_piece(x, 1, 1 + SB30_WHITE_PAWN1 + x);
		m_board->write_piece(x, 6, 1 + SB30_BLACK_PAWN1 + x);
	}
}

bool tasc_sb30_device::piece_available(uint8_t id)
{
	for (int y=0; y<8; y++)
		for (int x=0; x<8; x++)
		{
			if (m_board->read_piece(x, y) == id)
				return false;
		}

	return true;
}

uint8_t tasc_sb30_device::spawn_cb(offs_t offset)
{
	int piece_id = -1;
	if (offset == 1)
	{
		for (int p = 0; p < 8; p++)
			if (piece_available(1 + SB30_WHITE_PAWN1 + p))
			{
				piece_id = SB30_WHITE_PAWN1 + p;
				break;
			}
	}
	else if (offset == 7)
	{
		for (int p = 0; p < 8; p++)
			if (piece_available(1 + SB30_BLACK_PAWN1 + p))
			{
				piece_id = SB30_BLACK_PAWN1 + p;
				break;
			}
	}
	else if (offset == 2)
	{
		if      (piece_available(1 + SB30_WHITE_KNIGHT1)) piece_id = SB30_WHITE_KNIGHT1;
		else if (piece_available(1 + SB30_WHITE_KNIGHT2)) piece_id = SB30_WHITE_KNIGHT2;
	}
	else if (offset == 3)
	{
		if      (piece_available(1 + SB30_WHITE_BISHOP1)) piece_id = SB30_WHITE_BISHOP1;
		else if (piece_available(1 + SB30_WHITE_BISHOP2)) piece_id = SB30_WHITE_BISHOP2;
	}
	else if (offset == 4)
	{
		if      (piece_available(1 + SB30_WHITE_ROOK1)) piece_id = SB30_WHITE_ROOK1;
		else if (piece_available(1 + SB30_WHITE_ROOK2)) piece_id = SB30_WHITE_ROOK2;
	}
	else if (offset == 5 && piece_available(1 + SB30_WHITE_QUEEN))
		piece_id = SB30_WHITE_QUEEN;

	else if (offset == 6 && piece_available(1 + SB30_WHITE_KING))
		piece_id = SB30_WHITE_KING;

	else if (offset == 8)
	{
		if      (piece_available(1 + SB30_BLACK_KNIGHT1)) piece_id = SB30_BLACK_KNIGHT1;
		else if (piece_available(1 + SB30_BLACK_KNIGHT2)) piece_id = SB30_BLACK_KNIGHT2;
	}
	else if (offset == 9)
	{
		if      (piece_available(1 + SB30_BLACK_BISHOP1)) piece_id = SB30_BLACK_BISHOP1;
		else if (piece_available(1 + SB30_BLACK_BISHOP2)) piece_id = SB30_BLACK_BISHOP2;
	}

	else if (offset == 10)
	{
		if      (piece_available(1 + SB30_BLACK_ROOK1)) piece_id = SB30_BLACK_ROOK1;
		else if (piece_available(1 + SB30_BLACK_ROOK2)) piece_id = SB30_BLACK_ROOK2;
	}
	else if (offset == 11 && piece_available(1 + SB30_BLACK_QUEEN))
		piece_id = SB30_BLACK_QUEEN;

	else if (offset == 12 && piece_available(1 + SB30_BLACK_KING))
		piece_id = SB30_BLACK_KING;

	if (piece_id >= 0)
		return piece_id + 1;
	else
		return 0;   // not available
}

uint8_t tasc_sb30_device::read()
{
	int x = (m_position & 0x3f) / 8;
	int y = (m_position & 0x3f) % 8;
	int piece_id = m_board->read_sensor(7 - x, 7 - y);

	// each piece is identified by a single bit in a 32-bit sequence, if multiple bits are active the MSB is used
	uint32_t sb30_id = 0;
	if (piece_id > 0)
		sb30_id = 1UL << (piece_id - 1);

	return BIT(sb30_id, m_shift & 0x1f);
}


void tasc_sb30_device::write(uint8_t data)
{
	if (BIT(data, 3) && !BIT(m_data, 3))
		m_position = 0;

	if (BIT(data, 7) && BIT(data, 6) && !BIT(m_data, 6))
	{
		out_led(m_position);
		m_leds_off_timer->adjust(attotime::from_hz(5));
	}

	if (!BIT(data, 7) && BIT(m_data, 7))
	{
		m_position++;
		if (m_position & 0x40)
		{
			m_shift++;
			if (m_position & 1)
				m_shift = 0;
		}
	}

	m_data = data;
}


void tasc_sb30_device::out_led(int pos)
{
	pos &= 0x3f;
	int x = pos / 8;
	int y = pos % 8;
	m_out_leds[y + 0][x + 0] = 1;
	m_out_leds[y + 0][x + 1] = 1;
	m_out_leds[y + 1][x + 0] = 1;
	m_out_leds[y + 1][x + 1] = 1;
}
