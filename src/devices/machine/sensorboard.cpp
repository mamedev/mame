// license:BSD-3-Clause
// copyright-holders:hap
/*

Generic sensorboard device, meant for tracking pieces, primarily made for
electronic chessboards. It supports buttons, magnets, and inductive sensors.

TODO:
- dynamically generate input defs instead of all those PORT_CONDITION, input ports
  need to be in the class first instead of static
- increase board size when needed, theoretical maximum is 16*16, and even larger
  if input ports are modernized in MAME core

*******************************************************************************

Concept/idea by Ralph Schaefer, but his code got removed from MAME when he
couldn't be reached for source relicensing. This device is made from scratch.
It uses similar I/O methods as before: MAME keeps track of sensor clicks and
whereabouts of board pieces, and the GUI is in a layout file. This means it
can be made to look completely different with external artwork.

This device 'steals' the Shift and Ctrl keys, so don't use them in the driver.
But if you must, these inputs can be disabled with set_mod_enable(false).
In here, they're used for forcing sensor/piece inputs (a normal click activates
both at the same time). Also used with board initialization.

If you use this device in a slot, or add multiple of them, make sure to override
the outputs with output_cb() to avoid collisions.

*******************************************************************************

Usage notes:

At startup, the board is in its default starting position. RESET button works the
same way, and holding CTRL while pressing it will rotate the board, eg. for placing
black at the bottom with chess.

Click on a piece to pick it up, click on a board position to drop it. Only 1 piece
can be selected at the same time. Drag & drop is not supported. To remove a piece,
select the piece and then click on REMOVE, or overwrite it with another piece.

Magnet boards are simulated faithfully. Picking up a piece is the same as picking
it up on a real board. The only helper feature is when dropping a piece, a delay is
added, so the machine can detect when a piece is 'captured'.

For button boards:

To prevent the possibility of a piece/sensor having opposite states (iow: piece
selected but sensor deactivated, the button is not clicked when dropping a piece
at the same position it was before.

Hold CTRL while clicking to activate the piece but ignore the sensor beneath it,
on magnet boards this only applies during piece capture. Hold SHIFT while clicking
to activate the sensor but ignore the piece, on magnet boards it will invert the
sensor state instead.

*/

#include "emu.h"
#include "machine/sensorboard.h"


DEFINE_DEVICE_TYPE(SENSORBOARD, sensorboard_device, "sensorboard", "Sensorboard")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

sensorboard_device::sensorboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SENSORBOARD, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_out_piece(*this, "piece_%c%u", 0U + 'a', 1U),
	m_out_pui(*this, "piece_ui%u", 0U),
	m_out_count(*this, "count_ui%u", 0U),
	m_inp_rank(*this, "RANK.%u", 1),
	m_inp_spawn(*this, "SPAWN"),
	m_inp_ui(*this, "UI"),
	m_inp_conf(*this, "CONF"),
	m_custom_init_cb(*this),
	m_custom_sensor_cb(*this),
	m_custom_spawn_cb(*this),
	m_custom_output_cb(*this)
{
	m_nvram_auto = false;
	m_nosensors = false;
	m_magnets = false;
	m_inductive = false;
	m_ui_enabled = 7;
	set_delay(attotime::never);

	// set defaults for most common use case (aka chess)
	set_size(8, 8);
	set_spawnpoints(12);
}



//-------------------------------------------------
//  device_start / reset
//-------------------------------------------------

void sensorboard_device::device_start()
{
	// resolve handlers
	m_custom_init_cb.resolve_safe();
	m_custom_sensor_cb.resolve_safe(0);
	m_custom_spawn_cb.resolve();
	m_custom_output_cb.resolve();

	if (m_custom_output_cb.isnull())
	{
		m_out_piece.resolve();
		m_out_pui.resolve();
		m_out_count.resolve();
	}

	m_undotimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sensorboard_device::undo_tick),this));
	m_sensortimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sensorboard_device::sensor_off),this));
	cancel_sensor();

	u16 wmask = ~((1 << m_width) - 1);
	u16 hmask = ~((1 << m_height) - 1);
	m_bs_mask = hmask << 16 | wmask;
	m_ss_mask = ~((1 << m_maxspawn) - 1);

	m_uselect = 0;
	m_upointer = 0;
	m_ufirst = 0;
	m_ulast = 0;
	m_usize = std::size(m_history);

	// register for savestates
	save_item(NAME(m_nosensors));
	save_item(NAME(m_magnets));
	save_item(NAME(m_inductive));
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_bs_mask));
	save_item(NAME(m_ss_mask));
	save_item(NAME(m_maxspawn));
	save_item(NAME(m_maxid));
	save_item(NAME(m_hand));
	save_item(NAME(m_handpos));
	save_item(NAME(m_droppos));
	save_item(NAME(m_sensorpos));
	save_item(NAME(m_ui_enabled));

	save_item(NAME(m_curstate));
	save_item(NAME(m_history));

	save_item(NAME(m_uselect));
	save_item(NAME(m_upointer));
	save_item(NAME(m_ufirst));
	save_item(NAME(m_ulast));
	save_item(NAME(m_usize));
	save_item(NAME(m_nvram_auto));
	save_item(NAME(m_sensordelay));
}

void sensorboard_device::preset_chess(int state)
{
	// set chessboard start position

	// 1 = white pawn      7 = black pawn
	// 2 = white knight    8 = black knight
	// 3 = white bishop    9 = black bishop
	// 4 = white rook     10 = black rook
	// 5 = white queen    11 = black queen
	// 6 = white king     12 = black king

	write_piece(0, 0, 4);
	write_piece(7, 0, 4);
	write_piece(1, 0, 2);
	write_piece(6, 0, 2);
	write_piece(2, 0, 3);
	write_piece(5, 0, 3);
	write_piece(3, 0, 5);
	write_piece(4, 0, 6);

	for (int x = 0; x < 8; x++)
	{
		write_piece(x, 1, 1);
		write_piece(x, 6, 7);
		write_piece(x, 7, read_piece(x, 0) + 6);
	}

	// note that for inductive boards, 2 additional callbacks are needed:
	// additional init_cb() for reassigning the piece ids for initial board state
	// spawn_cb() for checking if a piece is available and reassigning the piece id
}

void sensorboard_device::device_reset()
{
	cancel_sensor();
	cancel_hand();

	if (!nvram_on())
	{
		clear_board();
		m_custom_init_cb(0);
	}
	undo_reset();
	refresh();
}



//-------------------------------------------------
//  device_nvram_interface overrides
//-------------------------------------------------

void sensorboard_device::nvram_default()
{
	clear_board();
	m_custom_init_cb(1);
}

bool sensorboard_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_curstate, sizeof(m_curstate), actual) && actual == sizeof(m_curstate);
}

bool sensorboard_device::nvram_write(util::write_stream &file)
{
	// save last board position
	size_t actual;
	return !file.write(m_curstate, sizeof(m_curstate), actual) && actual == sizeof(m_curstate);
}

bool sensorboard_device::nvram_can_write()
{
	return nvram_on();
}

bool sensorboard_device::nvram_on()
{
	return (m_inp_conf->read() & 3) ? bool(m_inp_conf->read() & 2) : m_nvram_auto;
}



//-------------------------------------------------
//  public handlers
//-------------------------------------------------

sensorboard_device &sensorboard_device::set_type(sb_type type)
{
	m_nosensors = (type == NOSENSORS);
	m_inductive = (type == INDUCTIVE);
	m_magnets = (type == MAGNETS) || m_inductive;

	return *this;
}

u8 sensorboard_device::read_sensor(u8 x, u8 y)
{
	if (x >= m_width || y >= m_height)
		return 0;

	u8 live_state = BIT(m_inp_rank[y]->read(), x);
	int pos = (y << 4 & 0xf0) | (x & 0x0f);

	if (m_magnets)
	{
		u8 piece = read_piece(x, y);
		u8 state = (piece != 0 && pos != m_handpos && (m_inp_ui->read() & 2 || pos != m_droppos)) ? 1 : 0;

		// piece recognition: return piece id
		if (m_inductive)
			return (state) ? piece : 0;

		// invert state if sensor is forced
		else
			return state ^ (m_inp_ui->read() & live_state);
	}
	else
	{
		// buttons are forced
		if (m_inp_ui->read() & 1)
			return live_state;

		// buttons are blocked
		else if (m_inp_ui->read() & 2)
			return 0;

		else if (m_sensordelay == attotime::never)
			return live_state;

		return (pos == m_sensorpos) ? 1 : 0;
	}
}

u16 sensorboard_device::read_file(u8 x, bool reverse)
{
	if (m_inductive || x >= m_width)
		return 0;

	// read whole file(column) at once
	u16 data = 0;
	for (int y = 0; y < m_height; y++)
		data = data << 1 | read_sensor(x, (reverse) ? y : (m_height - 1 - y));

	return data;
}

u16 sensorboard_device::read_rank(u8 y, bool reverse)
{
	if (m_inductive || y >= m_height)
		return 0;

	// read whole rank(row) at once
	u16 data = 0;
	for (int x = 0; x < m_width; x++)
		data = data << 1 | read_sensor((reverse) ? x : (m_width - 1 - x), y);

	return data;
}

void sensorboard_device::refresh()
{
	bool custom_out = !m_custom_output_cb.isnull();

	// output spawn icons
	for (int i = 0; i < m_maxspawn; i++)
	{
		if (custom_out)
			m_custom_output_cb(i + 0x101, i + 1);
		else
			m_out_pui[i + 1] = i + 1;
	}

	// output hand piece
	if (custom_out)
		m_custom_output_cb(0x100, m_hand);
	else
		m_out_pui[0] = m_hand;

	// output board state
	for (int x = 0; x < m_width; x++)
		for (int y = 0; y < m_height; y++)
		{
			u8 piece = (m_inp_conf->read() & 4) ? 0 : read_piece(x, y);
			int pos = (y << 4 & 0xf0) | (x & 0x0f);

			// selected piece: m_maxid + piece id
			if (piece != 0 && pos == m_handpos)
				piece += m_maxid;

			if (custom_out)
				m_custom_output_cb(pos, piece);
			else
				m_out_piece[x][y] = piece;
		}

	// set new move on board state change
	if (memcmp(m_curstate, m_history[m_upointer], m_width * m_height))
	{
		m_upointer = (m_upointer + 1) % m_usize;
		m_ulast = m_upointer;
		memcpy(m_history[m_upointer], m_curstate, m_height * m_width);

		// overflow
		if (m_ufirst == m_ulast)
			m_ufirst = (m_ufirst + 1) % m_usize;
	}

	// output undo counter
	u32 last = m_ulast;
	u32 p = m_upointer;

	if (last < m_ufirst)
		last += m_usize;
	if (p < m_ufirst)
		p += m_usize;

	u32 c0 = p - m_ufirst;
	u32 c1 = last - m_ufirst;

	if (custom_out)
	{
		m_custom_output_cb(0x200, c0);
		m_custom_output_cb(0x201, c1);
	}
	else
	{
		m_out_count[0] = c0;
		m_out_count[1] = c1;
	}
}

void sensorboard_device::cancel_sensor()
{
	// stop sensor delay
	m_sensortimer->adjust(attotime::never);
	m_sensorpos = m_droppos = -1;
}

void sensorboard_device::cancel_hand()
{
	// remove piece from hand (but don't remove it from the board)
	m_hand = 0;
	m_handpos = -1;
}

void sensorboard_device::remove_hand()
{
	// remove piece from hand, and if original place was board, remove it from there
	m_hand = 0;
	if (m_handpos != -1)
	{
		write_piece(m_handpos & 0xf, m_handpos >> 4 & 0xf, 0);
		m_handpos = -1;
	}
}

bool sensorboard_device::drop_piece(u8 x, u8 y)
{
	// drop piece from hand onto the board
	u8 piece = m_hand;
	if (piece != 0)
	{
		// lock hand if spawn piece is held down
		if (m_handpos != -1 || m_inductive || !BIT(m_inp_spawn->read(), piece - 1))
			remove_hand();

		write_piece(x, y, piece);

		// magnet boards: delay when capturing a piece
		if (m_sensordelay != attotime::never)
			m_droppos = (y << 4 & 0xf0) | (x & 0x0f);
	}

	return piece != 0;
}

bool sensorboard_device::pickup_piece(u8 x, u8 y)
{
	// pick up piece from board, place in hand
	u8 piece = read_piece(x, y);
	if (piece != 0)
	{
		m_hand = piece;
		m_handpos = (y << 4 & 0xf0) | (x & 0x0f);

		// possibly remove it immediately
		if (m_inp_ui->read() & 8)
			remove_hand();
	}

	return piece != 0;
}



//-------------------------------------------------
//  input handlers (internal use)
//-------------------------------------------------

INPUT_CHANGED_MEMBER(sensorboard_device::sensor)
{
	if (!newval)
		return;

	if (m_sensorpos != -1 || (m_inp_ui->read() & 1 && !m_inductive && !m_nosensors))
		return;

	u8 pos = (u8)param;
	u8 x = pos & 0xf;
	u8 y = pos >> 4 & 0xf;
	if (x >= m_width || y >= m_height)
		return;

	// auto click / sensor delay
	if (m_sensordelay != attotime::never && (m_magnets || (pos != m_handpos && ~m_inp_ui->read() & 2)))
	{
		m_sensorpos = pos;
		m_sensortimer->adjust(m_sensordelay);
	}

	// optional custom handling:
	// return d0 = block drop piece
	// return d1 = block pick up piece
	u8 custom = m_custom_sensor_cb(pos);

	// drop piece
	if (m_hand != 0)
	{
		if (~custom & 1)
			drop_piece(x, y);
	}

	// pick up piece
	else if (~custom & 2)
		pickup_piece(x, y);

	refresh();
}

INPUT_CHANGED_MEMBER(sensorboard_device::ui_spawn)
{
	u8 pos = (newval) ? (u8)param : 32 - count_leading_zeros_32(m_inp_spawn->read());
	if (pos == 0 || pos > m_maxspawn)
		return;

	cancel_sensor();
	m_hand = pos;
	m_handpos = -1;

	// optional callback to change piece id
	if (!m_custom_spawn_cb.isnull())
		m_hand = m_custom_spawn_cb(pos);

	refresh();
}

INPUT_CHANGED_MEMBER(sensorboard_device::ui_hand)
{
	if (!newval)
		return;

	cancel_sensor();
	remove_hand();
	refresh();
}

TIMER_CALLBACK_MEMBER(sensorboard_device::undo_tick)
{
	if (m_ufirst == m_ulast)
		return;

	u32 last = m_ulast;
	u32 p = m_upointer;

	if (last < m_ufirst)
		last += m_usize;
	if (p < m_ufirst)
		p += m_usize;

	switch (param)
	{
		case 0:
			p = m_ufirst;
			break;
		case 1:
			if (p > m_ufirst)
				p--;
			break;
		case 2:
			if (p < last)
				p++;
			break;
		case 3:
			p = last;
			break;
	}

	p %= m_usize;
	if (p == m_upointer)
		return;

	cancel_sensor();
	cancel_hand();

	// set current state to undo point
	memcpy(m_curstate, m_history[p], m_height * m_width);
	m_upointer = p;
	refresh();

	// schedule next timeout
	if (m_inp_ui->read() & 0xf0)
		m_undotimer->adjust(attotime::from_msec(500), param);
}

void sensorboard_device::undo_reset()
{
	memcpy(m_history[0], m_curstate, m_height * m_width);

	m_upointer = 0;
	m_ufirst = 0;
	m_ulast = 0;
}

INPUT_CHANGED_MEMBER(sensorboard_device::ui_undo)
{
	u8 select = (u8)param;

	if (newval)
	{
		m_uselect = select;
		m_undotimer->adjust(attotime::zero, select);
	}
	else if ((m_inp_ui->read() & 0xf0) == 0 || select == m_uselect)
		m_undotimer->adjust(attotime::never);
}

INPUT_CHANGED_MEMBER(sensorboard_device::ui_init)
{
	if (!newval)
		return;

	u8 init = (u8)param;
	cancel_sensor();
	cancel_hand();

	clear_board();

	if (init)
		m_custom_init_cb(0);

	// rotate pieces
	if (m_inp_ui->read() & 2)
	{
		u8 tempstate[0x100];
		for (int y = 0; y < m_height; y++)
			for (int x = 0; x < m_width; x++)
				tempstate[m_width * (m_height - 1 - y) + (m_width - 1 - x)] = m_curstate[m_width * y + x];

		memcpy(m_curstate, tempstate, m_height * m_width);
	}

	// reset undo
	if (m_inp_ui->read() & 1)
		undo_reset();

	refresh();
}



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( sensorboard )
	PORT_START("RANK.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x00) PORT_NAME("Sensor A1")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x01) PORT_NAME("Sensor B1")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x02) PORT_NAME("Sensor C1")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x03) PORT_NAME("Sensor D1")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x04) PORT_NAME("Sensor E1")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x05) PORT_NAME("Sensor F1")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x06) PORT_NAME("Sensor G1")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x07) PORT_NAME("Sensor H1")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x08) PORT_NAME("Sensor I1")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x09) PORT_NAME("Sensor J1")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x0a) PORT_NAME("Sensor K1")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<16 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x0b) PORT_NAME("Sensor L1")

	PORT_START("RANK.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x10) PORT_NAME("Sensor A2")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x11) PORT_NAME("Sensor B2")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x12) PORT_NAME("Sensor C2")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x13) PORT_NAME("Sensor D2")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x14) PORT_NAME("Sensor E2")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x15) PORT_NAME("Sensor F2")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x16) PORT_NAME("Sensor G2")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x17) PORT_NAME("Sensor H2")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x18) PORT_NAME("Sensor I2")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x19) PORT_NAME("Sensor J2")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x1a) PORT_NAME("Sensor K2")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<17 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x1b) PORT_NAME("Sensor L2")

	PORT_START("RANK.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x20) PORT_NAME("Sensor A3")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x21) PORT_NAME("Sensor B3")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x22) PORT_NAME("Sensor C3")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x23) PORT_NAME("Sensor D3")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x24) PORT_NAME("Sensor E3")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x25) PORT_NAME("Sensor F3")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x26) PORT_NAME("Sensor G3")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x27) PORT_NAME("Sensor H3")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x28) PORT_NAME("Sensor I3")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x29) PORT_NAME("Sensor J3")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x2a) PORT_NAME("Sensor K3")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<18 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x2b) PORT_NAME("Sensor L3")

	PORT_START("RANK.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x30) PORT_NAME("Sensor A4")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x31) PORT_NAME("Sensor B4")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x32) PORT_NAME("Sensor C4")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x33) PORT_NAME("Sensor D4")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x34) PORT_NAME("Sensor E4")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x35) PORT_NAME("Sensor F4")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x36) PORT_NAME("Sensor G4")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x37) PORT_NAME("Sensor H4")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x38) PORT_NAME("Sensor I4")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x39) PORT_NAME("Sensor J4")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x3a) PORT_NAME("Sensor K4")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<19 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x3b) PORT_NAME("Sensor L4")

	PORT_START("RANK.5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x40) PORT_NAME("Sensor A5")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x41) PORT_NAME("Sensor B5")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x42) PORT_NAME("Sensor C5")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x43) PORT_NAME("Sensor D5")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x44) PORT_NAME("Sensor E5")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x45) PORT_NAME("Sensor F5")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x46) PORT_NAME("Sensor G5")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x47) PORT_NAME("Sensor H5")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x48) PORT_NAME("Sensor I5")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x49) PORT_NAME("Sensor J5")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x4a) PORT_NAME("Sensor K5")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<20 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x4b) PORT_NAME("Sensor L5")

	PORT_START("RANK.6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x50) PORT_NAME("Sensor A6")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x51) PORT_NAME("Sensor B6")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x52) PORT_NAME("Sensor C6")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x53) PORT_NAME("Sensor D6")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x54) PORT_NAME("Sensor E6")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x55) PORT_NAME("Sensor F6")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x56) PORT_NAME("Sensor G6")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x57) PORT_NAME("Sensor H6")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x58) PORT_NAME("Sensor I6")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x59) PORT_NAME("Sensor J6")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x5a) PORT_NAME("Sensor K6")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<21 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x5b) PORT_NAME("Sensor L6")

	PORT_START("RANK.7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x60) PORT_NAME("Sensor A7")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x61) PORT_NAME("Sensor B7")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x62) PORT_NAME("Sensor C7")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x63) PORT_NAME("Sensor D7")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x64) PORT_NAME("Sensor E7")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x65) PORT_NAME("Sensor F7")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x66) PORT_NAME("Sensor G7")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x67) PORT_NAME("Sensor H7")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x68) PORT_NAME("Sensor I7")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x69) PORT_NAME("Sensor J7")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x6a) PORT_NAME("Sensor K7")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<22 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x6b) PORT_NAME("Sensor L7")

	PORT_START("RANK.8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x70) PORT_NAME("Sensor A8")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x71) PORT_NAME("Sensor B8")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x72) PORT_NAME("Sensor C8")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x73) PORT_NAME("Sensor D8")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x74) PORT_NAME("Sensor E8")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x75) PORT_NAME("Sensor F8")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x76) PORT_NAME("Sensor G8")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x77) PORT_NAME("Sensor H8")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x78) PORT_NAME("Sensor I8")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x79) PORT_NAME("Sensor J8")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x7a) PORT_NAME("Sensor K8")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<23 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x7b) PORT_NAME("Sensor L8")

	PORT_START("RANK.9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x80) PORT_NAME("Sensor A9")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x81) PORT_NAME("Sensor B9")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x82) PORT_NAME("Sensor C9")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x83) PORT_NAME("Sensor D9")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x84) PORT_NAME("Sensor E9")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x85) PORT_NAME("Sensor F9")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x86) PORT_NAME("Sensor G9")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x87) PORT_NAME("Sensor H9")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x88) PORT_NAME("Sensor I9")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x89) PORT_NAME("Sensor J9")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x8a) PORT_NAME("Sensor K9")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<24 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x8b) PORT_NAME("Sensor L9")

	PORT_START("RANK.10")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x90) PORT_NAME("Sensor A10")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x91) PORT_NAME("Sensor B10")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x92) PORT_NAME("Sensor C10")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x93) PORT_NAME("Sensor D10")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x94) PORT_NAME("Sensor E10")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x95) PORT_NAME("Sensor F10")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x96) PORT_NAME("Sensor G10")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x97) PORT_NAME("Sensor H10")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x98) PORT_NAME("Sensor I10")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x99) PORT_NAME("Sensor J10")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x9a) PORT_NAME("Sensor K10")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("BS_CHECK", 1<<25 | 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, sensor, 0x9b) PORT_NAME("Sensor L10")

	PORT_START("SPAWN")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<0, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 1) PORT_NAME("Spawn Piece 1")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<1, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 2) PORT_NAME("Spawn Piece 2")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<2, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 3) PORT_NAME("Spawn Piece 3")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<3, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 4) PORT_NAME("Spawn Piece 4")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<4, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 5) PORT_NAME("Spawn Piece 5")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<5, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 6) PORT_NAME("Spawn Piece 6")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<6, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 7) PORT_NAME("Spawn Piece 7")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<7, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 8) PORT_NAME("Spawn Piece 8")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<8, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 9) PORT_NAME("Spawn Piece 9")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<9, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 10) PORT_NAME("Spawn Piece 10")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<10, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 11) PORT_NAME("Spawn Piece 11")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<11, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 12) PORT_NAME("Spawn Piece 12")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<12, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 13) PORT_NAME("Spawn Piece 13")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<13, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 14) PORT_NAME("Spawn Piece 14")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<14, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 15) PORT_NAME("Spawn Piece 15")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("SS_CHECK", 1<<15, EQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_spawn, 16) PORT_NAME("Spawn Piece 16")

	PORT_START("UI")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<0, NOTEQUALS, 0) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Modifier 2 / Force Sensor") // hold while clicking to force sensor (ignore piece)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<0, NOTEQUALS, 0) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Modifier 1 / Force Piece") // hold while clicking to force piece (ignore sensor)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(sensorboard_device, check_sensor_busy) // check if any sensor is busy / pressed (read-only)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<1, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_hand, 0) PORT_NAME("Remove Piece")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<1, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_undo, 0) PORT_NAME("Undo Buffer First")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<1, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_undo, 1) PORT_NAME("Undo Buffer Previous")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<1, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_undo, 2) PORT_NAME("Undo Buffer Next")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<1, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_undo, 3) PORT_NAME("Undo Buffer Last")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<1, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_init, 0) PORT_NAME("Board Clear")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CONDITION("UI_CHECK", 1<<1, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_init, 1) PORT_NAME("Board Reset")

	PORT_START("CONF")
	PORT_CONFNAME( 0x03, 0x00, "Remember Position" ) PORT_CONDITION("UI_CHECK", 1<<2, NOTEQUALS, 0)
	PORT_CONFSETTING(    0x01, DEF_STR( No ) )
	PORT_CONFSETTING(    0x02, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x00, "Auto" )
	PORT_CONFNAME( 0x04, 0x00, "Show Pieces" ) PORT_CONDITION("UI_CHECK", 1<<2, NOTEQUALS, 0) PORT_CHANGED_MEMBER(DEVICE_SELF, sensorboard_device, ui_refresh, 0)
	PORT_CONFSETTING(    0x04, DEF_STR( No ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("BS_CHECK") // board size (internal use)
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(sensorboard_device, check_bs_mask)

	PORT_START("SS_CHECK") // spawn size (internal use)
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(sensorboard_device, check_ss_mask)

	PORT_START("UI_CHECK") // UI enabled (internal use)
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(sensorboard_device, check_ui_enabled)
INPUT_PORTS_END

ioport_constructor sensorboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sensorboard);
}
