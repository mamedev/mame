// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/******************************************************************************

Tasc SmartBoard SB30 (analog)

Chessboard controller for use with Tasc R30 chesscomputer, or as PC peripheral.

The SmartBoard can detect which piece is present on a specific square, more
info on the technology used in the piece recognition system can be found in
the US patent 5,129,654

SmartBoard I is SB30 (81 LEDs, analog chesspieces)
SmartBoard II is SB20 (64 LEDs, digital chesspieces)
SmartBoard III is SB30 again, but digital

SB20 and the newer SB30 are not emulated. They're on different hardware, with
embedded CPU to reduce I/O overhead. Note, those are not compatible with old
versions of Tasc R30.

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


DEFINE_DEVICE_TYPE(TASC_SB30, tasc_sb30_device, "tasc_sb30", "Tasc SmartBoard SB30")

//-------------------------------------------------
//  tasc_sb30_device - constructor
//-------------------------------------------------

tasc_sb30_device::tasc_sb30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TASC_SB30, tag, owner, clock)
	, m_board(*this, "board")
	, m_out_leds(*this, "sb30_led_%u.%u", 0U, 0U)
	, m_conf(*this, "CONF")
	, m_data_out(*this)
	, m_led_out(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tasc_sb30_device::device_start()
{
	if (m_led_out.isunset())
		m_out_leds.resolve();

	// zerofill
	m_data0 = 0;
	m_data1 = 0;
	m_output = 0;
	m_scan_pending = false;
	m_pos = 0;
	std::fill_n(m_squares, std::size(m_squares), 0);

	// register for savestates
	save_item(NAME(m_data0));
	save_item(NAME(m_data1));
	save_item(NAME(m_output));
	save_item(NAME(m_scan_pending));
	save_item(NAME(m_pos));
	save_item(NAME(m_squares));
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( smartboard )
	PORT_START("CONF")
	PORT_CONFNAME( 0x01, 0x00, "Duplicate Piece IDs" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END

ioport_constructor tasc_sb30_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(smartboard);
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

void tasc_sb30_device::init_cb(u8 data)
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
				return bool(m_conf->read() & 1);
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

void tasc_sb30_device::data0_w(int state)
{
	state = state ? 1 : 0;

	if (!state && m_data0)
	{
		if (m_scan_pending)
		{
			for (int i = 0; i < 64; i++)
			{
				// each piece is identified by a single bit in a 32-bit sequence
				int piece_id = m_board->read_sensor(i >> 3 ^ 7, ~i & 7);
				m_squares[i] = piece_id ? (1 << (piece_id - 1)) : 0;
			}

			m_scan_pending = false;
			m_pos = 0;
			update_output();
		}
		else
		{
			// output board led(s)
			if (m_led_out.isunset())
				m_out_leds[m_pos & 7][m_pos >> 3 & 7] = m_data1;
			else
				m_led_out(m_pos & 0x3f, m_data1);
		}
	}

	m_data0 = state;
}

void tasc_sb30_device::data1_w(int state)
{
	state = state ? 1 : 0;

	// clock position counter
	if (!state && m_data1)
	{
		m_pos++;

		if (m_data0)
			m_scan_pending = true;

		update_output();
	}

	m_data1 = state;
}

void tasc_sb30_device::update_output()
{
	m_output = BIT(m_squares[m_pos & 0x3f], m_pos >> 6 & 0x1f);
	m_data_out(m_output);
}
