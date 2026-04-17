// license:BSD-3-Clause
// copyright-holders:hap
/*

Milton Bradley Grand Master motorized self-moving chessboard

Electronic chessboard with motors underneath, that move a magnet around for
automatically moving chesspieces (each piece has a magnet underneath).

Used in:
- Milton Bradley Grand Master
- Fidelity Phantom / Chesster Phantom
- Excalibur Mirage
- Excalibur Phantom Force

Hardware notes:
- electronic chessboard, with room on each side for captured pieces
- X/Y plotter motors, electromagnet (optionally including a hall effect sensor)

Concept/design by Milton Bradley, for use in the Grand Master. Fidelity licensed
or bought the design, and applied it nearly unchanged to Fidelity Phantom. Years
later, the ex chief engineer of by then defunct Fidelity used the same technology
while working for Excalibur.

TODO:
- optionally change output finders to callbacks, currently not needed
- sensorboard undo buffer goes out of control, probably not worth solving this issue

*/

#include "emu.h"
#include "gmboard.h"

#define LOG_MAGNET (1 << 1U)
#define LOG_DRIFT  (2 << 1U)

//#define VERBOSE (LOG_DRIFT)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MB_GMBOARD, gmboard_device, "mb_gmboard", "Milton Bradley Grand Master chessboard")

//-------------------------------------------------
//  gmboard_device - constructor
//-------------------------------------------------

gmboard_device::gmboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MB_GMBOARD, tag, owner, clock),
	m_board(*this, "board"),
	m_piece_hand(*this, "cpu_hand"),
	m_out_motor(*this, "motor%u", 0U),
	m_out_pos(*this, "pos_%c", unsigned('x')),
	m_quad_cb(*this)
{
	// (just to prevent a divide by 0 if it's not configured)
	m_width = 1;
	m_height = 1;
	m_square = 1;
	m_x_offset = 0;
	m_y_offset = 0;

	m_speed = attotime::from_msec(200);
}


//-------------------------------------------------
//  initialization
//-------------------------------------------------

void gmboard_device::device_start()
{
	// resolve outputs
	m_piece_hand.resolve();
	m_out_motor.resolve();
	m_out_pos.resolve();

	// zerofill
	m_magnet = 0;
	memset(m_pieces_map, 0, sizeof(m_pieces_map));
	memset(m_motor_dir, 0, sizeof(m_motor_dir));
	memset(m_motor_max, 0, sizeof(m_motor_max));
	memset(m_motor_pos, 0, sizeof(m_motor_pos));
	memset(m_motor_quad, 0, sizeof(m_motor_quad));
	memset(m_motor_drift, 0, sizeof(m_motor_drift));

	// register for savestates
	save_item(NAME(m_magnet));
	save_item(NAME(m_pieces_map));
	save_item(NAME(m_motor_dir));
	save_item(NAME(m_motor_pos));
	save_item(NAME(m_motor_quad));
	save_item(NAME(m_motor_remain));
	save_item(NAME(m_motor_drift));

	init_motors();
}

void gmboard_device::device_reset()
{
	memset(m_motor_drift, 0, sizeof(m_motor_drift));
	m_magnet = 0;

	output_magnet_pos();
}

void gmboard_device::init_board(u8 data)
{
	m_board->preset_chess(data);

	// reposition pieces if board will be rotated
	if (data & 2)
	{
		for (int y = 0; y < 8; y++)
			for (int x = 7; x >= 0; x--)
			{
				m_board->write_piece(x + 4, y, m_board->read_piece(x, y));
				m_board->write_piece(x, y, 0);
			}
	}
}

void gmboard_device::clear_board(u8 data)
{
	memset(m_pieces_map, 0, sizeof(m_pieces_map));
	m_piece_hand = 0;
	m_board->clear_board(data);
}


//-------------------------------------------------
//  motor sim
//-------------------------------------------------

void gmboard_device::init_motors()
{
	m_motor_max[0] = m_width - 1;
	m_motor_max[1] = m_height - 1;

	// start at A1
	m_motor_pos[0] = std::min<u32>(2 * m_square + m_x_offset, m_motor_max[0]);
	m_motor_pos[1] = std::min<u32>(7 * m_square + m_y_offset, m_motor_max[1]);

	m_motor_period = m_speed / m_square;

	for (int i = 0; i < 2; i++)
	{
		m_motor_timer[i] = timer_alloc(FUNC(gmboard_device::motor_count), this);
		m_motor_remain[i] = m_motor_period / 2;
	}
}

void gmboard_device::get_scaled_pos(double *x, double *y)
{
	// scale down to 4 counts per square
	*x = std::clamp(double(int(m_motor_pos[0]) - m_x_offset) / (m_square / 4.0) + 2.0, 0.0, 48.0);
	*y = std::clamp(double(int(m_motor_pos[1]) - m_y_offset) / (m_square / 4.0) + 2.0, 0.0, 32.0);
}

void gmboard_device::output_magnet_pos()
{
	double x, y;
	get_scaled_pos(&x, &y);

	// put active state on x bit 11
	const int active = m_magnet ? 0x800 : 0;
	m_out_pos[0] = int(x * 25.0 + 0.5) | active;
	m_out_pos[1] = int(y * 25.0 + 0.5);
	m_out_motor[4] = m_magnet;
}

void gmboard_device::realign_magnet_pos()
{
	// compensate for possible gradual drift, eg. emirage
	for (int m = 0; m < 2; )
	{
		double pos[2];
		get_scaled_pos(&pos[0], &pos[1]);

		const double limit = 4.0 / (m_square / 4.0);
		const int step = std::max(m_square / (4 * 8), 1);
		int inc = 0;

		if ((round(pos[m]) - pos[m]) > limit && m_motor_pos[m] < m_motor_max[m] - step)
			inc = 1;
		else if ((round(pos[m]) - pos[m]) < -limit && m_motor_pos[m] > step)
			inc = -1;
		else
			m++;

		if (inc != 0)
		{
			int prev = m_motor_pos[m];
			m_motor_pos[m] += inc * step;
			m_motor_drift[m] -= inc;

			LOGMASKED(LOG_DRIFT, "motor %c drift error (%4d->%4d, %d total)\n", 'X' + m, prev, m_motor_pos[m], m_motor_drift[m]);
		}
	}
}

void gmboard_device::magnet_w(int state)
{
	state = state ? 1 : 0;

	if (state == m_magnet)
		return;
	m_magnet = state;

	if (m_piece_hand != 0)
		realign_magnet_pos();
	output_magnet_pos();

	double dx, dy;
	get_scaled_pos(&dx, &dy);

	int mx = dx + 0.5;
	int my = dy + 0.5;
	int gx = mx, gy = my;

	// convert motors position into board coordinates
	int x = mx / 4 - 2;
	int y = 7 - (my / 4);

	if (x < 0)
		x += 12;

	const bool valid_pos = (mx & 3) == 2 && (my & 3) == 2;

	if (state)
	{
		bool found = false;

		if (valid_pos)
		{
			// pick up piece, unless it was picked up by the user
			const int pos = (y << 4 & 0xf0) | (x & 0x0f);
			if (pos != m_board->get_handpos())
			{
				m_piece_hand = m_board->read_piece(x, y);

				if (m_piece_hand != 0)
				{
					found = true;
					m_board->write_piece(x, y, 0);
					m_board->refresh();
				}
			}
		}

		if (!found)
		{
			int count = 0;

			// check surrounding area for piece
			for (int sy = my - 1; sy <= my + 1; sy++)
				for (int sx = mx - 1; sx <= mx + 1; sx++)
					if (sy >= 0 && sx >= 0 && m_pieces_map[sy][sx] != 0)
					{
						gx = sx; gy = sy;
						m_piece_hand = m_pieces_map[sy][sx];
						m_pieces_map[sy][sx] = 0;
						count++;
					}

			// more than one piece found (shouldn't happen)
			if (count > 1)
				popmessage("Internal collision!");
		}
	}

	if (m_piece_hand)
	{
		LOGMASKED(LOG_MAGNET, "%s piece %2d @ %2d,%2d (%2d,%2d)\n", state ? "grab" : "drop", m_piece_hand, x, y, gx, gy);

		if (!state)
		{
			if (valid_pos)
			{
				// collision with piece on board (user interference)
				if (m_board->read_piece(x, y) != 0)
					popmessage("Collision at %c%d!", x + 'A', y + 1);
				else
				{
					m_board->write_piece(x, y, m_piece_hand);
					m_board->refresh();
				}
			}
			else
			{
				// collision with internal pieces map (shouldn't happen)
				if (m_pieces_map[my][mx] != 0)
					popmessage("Internal collision!");
				else
					m_pieces_map[my][mx] = m_piece_hand;
			}

			m_piece_hand = 0;
		}
	}
}

TIMER_CALLBACK_MEMBER(gmboard_device::motor_count)
{
	const int m = param ? 1 : 0;
	assert(m_motor_dir[m] & 3);

	// 1 quarter rotation step per period
	int inc = 0;
	if (m_motor_dir[m] & 2)
	{
		if (m_motor_pos[m] < m_motor_max[m])
			inc = 1;
	}
	else if (m_motor_pos[m] > 0)
		inc = -1;

	m_motor_remain[m] = m_motor_period;

	if (inc == 0)
		return;

	m_motor_pos[m] += inc;
	m_motor_timer[m]->adjust(m_motor_period, m);

	if (m_motor_pos[m] == 0 || m_motor_pos[m] == m_motor_max[m])
	{
		m_motor_drift[m] = 0;
		LOGMASKED(LOG_DRIFT, "motor %c calibrated\n", 'X' + m);
	}

	output_magnet_pos();

	// update quadrature encoder
	static const u8 lut_quad[4] = { 0,1,3,2 };
	m_motor_quad[m] = lut_quad[m_motor_pos[m] & 3];

	m_quad_cb[m](m_motor_quad[m]);
}

void gmboard_device::motor_w(offs_t offset, u8 data)
{
	const int m = offset & 1;
	data &= 3;

	for (int i = 0; i < 2; i++)
		m_out_motor[m * 2 + i] = BIT(data, i);

	// it's not moving when both directions are set
	if (data == 3)
		data = 0;

	if (data == m_motor_dir[m])
		return;

	// remember remaining time
	if (m_motor_dir[m] & 3 && !m_motor_timer[m]->remaining().is_never())
	{
		m_motor_remain[m] = m_motor_timer[m]->remaining();
		m_motor_timer[m]->adjust(attotime::never);
	}

	// invert remaining time if direction flipped
	if ((m_motor_dir[m] ^ data) & 1)
		m_motor_remain[m] = m_motor_period - m_motor_remain[m];

	m_motor_dir[m] = data;

	// (re)start the timer
	if (data)
		m_motor_timer[m]->adjust(m_motor_remain[m], m);
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void gmboard_device::device_add_mconfig(machine_config &config)
{
	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->set_size(8+4, 8);
	m_board->clear_cb().set(FUNC(gmboard_device::clear_board));
	m_board->init_cb().set(FUNC(gmboard_device::init_board));
	m_board->set_delay(attotime::from_msec(150));
}
