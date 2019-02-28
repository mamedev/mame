// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  Mitsubishi MELPS 4 MCU tabletops/handhelds or other simple devices,
  most of them are VFD electronic games/toys.

***************************************************************************/

#include "emu.h"

#include "cpu/melps4/m58846.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"

//#include "hh_melps4_test.lh" // common test-layout - no svg artwork(yet), use external artwork


class hh_melps4_state : public driver_device
{
public:
	hh_melps4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<m58846_device> m_maincpu;
	optional_ioport_array<4> m_inp_matrix; // max 4
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u16 m_inp_mux;                  // multiplexed inputs mask

	u8 read_inputs(int columns);
	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	// display common
	int m_display_wait;             // led/lamp off-delay in milliseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_grid;                     // VFD current row data
	u32 m_plate;                    // VFD current column data

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_melps4_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();
	m_out_a.resolve();
	m_out_digit.resolve();

	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_inp_mux = 0;
	m_grid = 0;
	m_plate = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_melps4_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_melps4_state::display_update()
{
	for (int y = 0; y < m_display_maxy; y++)
	{
		u32 active_state = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			u32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state |= (ds << x);

			// output to y.x, or y.a when always-on
			if (x != m_display_maxx)
				m_out_x[y][x] = ds;
			else
				m_out_a[y] = ds;
		}

		// output to digity
		if (m_display_segmask[y] != 0)
			m_out_digit[y] = active_state & m_display_segmask[y];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_melps4_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_melps4_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_melps4_state::display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// generic input handlers

u8 hh_melps4_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}

INPUT_CHANGED_MEMBER(hh_melps4_state::reset_button)
{
	// for when reset button is directly tied to MCU reset pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Coleco Frogger (manufactured in Japan, licensed from Sega)
  * PCB label Coleco Frogger Code No. 01-81543, KS-003282 Japan
  * Mitsubishi M58846-701P MCU
  * cyan/red/green VFD display Itron CP5090GLR R1B, with partial color overlay

***************************************************************************/

class cfrogger_state : public hh_melps4_state
{
public:
	cfrogger_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_melps4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	DECLARE_READ16_MEMBER(input_r);
	void cfrogger(machine_config &config);
};

// handlers

void cfrogger_state::prepare_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,0,1,2,3,4,5,6,7,8,9,10,11);
	u16 plate = bitswap<16>(m_plate,12,4,13,5,14,6,15,7,3,11,2,10,1,9,0,8);
	display_matrix(16, 12, plate, grid);
}

WRITE8_MEMBER(cfrogger_state::plate_w)
{
	// F0,F1: input mux
	if (offset == MELPS4_PORTF)
		m_inp_mux = data & 3;

	// Sx,Fx,Gx: vfd plate
	int mask = (offset == MELPS4_PORTS) ? 0xff : 0xf; // port S is 8-bit
	int shift = (offset == MELPS4_PORTS) ? 0 : (offset + 1) * 4;
	m_plate = (m_plate & ~(mask << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(cfrogger_state::grid_w)
{
	// D0-D11: vfd grid
	m_grid = data;
	prepare_display();
}

WRITE_LINE_MEMBER(cfrogger_state::speaker_w)
{
	// T: speaker out
	m_speaker->level_w(state);
}

READ16_MEMBER(cfrogger_state::input_r)
{
	// K0,K1: multiplexed inputs
	// K2: N/C
	// K3: fixed input
	return (m_inp_matrix[2]->read() & 8) | (read_inputs(2) & 3);
}

// config

static INPUT_PORTS_START( cfrogger )
	PORT_START("IN.0") // F0 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.1") // F1 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN.2") // K3
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("IN.3") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_melps4_state, reset_button, nullptr)
INPUT_PORTS_END

void cfrogger_state::cfrogger(machine_config &config)
{
	/* basic machine hardware */
	M58846(config, m_maincpu, 600_kHz_XTAL);
	m_maincpu->read_k().set(FUNC(cfrogger_state::input_r));
	m_maincpu->write_s().set(FUNC(cfrogger_state::plate_w));
	m_maincpu->write_f().set(FUNC(cfrogger_state::plate_w));
	m_maincpu->write_g().set(FUNC(cfrogger_state::plate_w));
	m_maincpu->write_d().set(FUNC(cfrogger_state::grid_w));
	m_maincpu->write_t().set(FUNC(cfrogger_state::speaker_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", "svg"));
	screen.set_refresh_hz(50);
	screen.set_size(500, 1080);
	screen.set_visarea(0, 500-1, 0, 1080-1);
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_melps4_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Gakken Jungler (manufactured in Japan, licensed from Konami)
  * PCB label Konami Gakken GR503
  * Mitsubishi M58846-702P MCU
  * cyan/red/green VFD display Itron CP5143GLR SGA, with light-yellow color overlay

***************************************************************************/

class gjungler_state : public hh_melps4_state
{
public:
	gjungler_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_melps4_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(plate_w);
	DECLARE_WRITE16_MEMBER(grid_w);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	DECLARE_READ16_MEMBER(input_r);
	void gjungler(machine_config &config);
};

// handlers

void gjungler_state::prepare_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,1);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,18,8,9,10,11,13,16,15,14,13,12,7,0,6,1,5,2,4,3) | 0x2000;
	display_matrix(18, 12, plate, grid);
}

WRITE8_MEMBER(gjungler_state::plate_w)
{
	// G0,G1: input mux
	if (offset == MELPS4_PORTG)
		m_inp_mux = data & 3;

	// Sx,Fx,Gx,U: vfd plate
	int mask = (offset == MELPS4_PORTS) ? 0xff : 0xf; // port S is 8-bit
	int shift = (offset == MELPS4_PORTS) ? 0 : (offset + 1) * 4;
	m_plate = (m_plate & ~(mask << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(gjungler_state::grid_w)
{
	// D0-D11: vfd grid
	m_grid = data;
	prepare_display();
}

WRITE_LINE_MEMBER(gjungler_state::speaker_w)
{
	// T: speaker out
	m_speaker->level_w(state);
}

READ16_MEMBER(gjungler_state::input_r)
{
	// K0,K1: multiplexed inputs
	// K2,K3: fixed inputs
	return (m_inp_matrix[2]->read() & 0xc) | (read_inputs(2) & 3);
}

// config

static INPUT_PORTS_START( gjungler )
	PORT_START("IN.0") // G0 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.1") // G1 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.2") // K2,K3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_CONFNAME( 0x08, 0x00, "Game Mode" )
	PORT_CONFSETTING(    0x00, "A" )
	PORT_CONFSETTING(    0x08, "B" )

	PORT_START("IN.3") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_melps4_state, reset_button, nullptr)
INPUT_PORTS_END

void gjungler_state::gjungler(machine_config &config)
{
	/* basic machine hardware */
	M58846(config, m_maincpu, 600_kHz_XTAL);
	m_maincpu->read_k().set(FUNC(gjungler_state::input_r));
	m_maincpu->write_s().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_f().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_g().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_u().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_d().set(FUNC(gjungler_state::grid_w));
	m_maincpu->write_t().set(FUNC(gjungler_state::speaker_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", "svg"));
	screen.set_refresh_hz(50);
	screen.set_size(481, 1080);
	screen.set_visarea(0, 481-1, 0, 1080-1);
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_melps4_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cfrogger )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m58846-701p", 0x0000, 0x1000, CRC(ba52a242) SHA1(7fa53b617f4bb54be32eb209e9b88131e11cb518) )

	ROM_REGION( 786255, "svg", 0)
	ROM_LOAD( "cfrogger.svg", 0, 786255, CRC(d8d6e2b6) SHA1(bc9a0260b211ed07021dfe1cc19a993569f4c544) )
ROM_END


ROM_START( gjungler )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m58846-702p", 0x0000, 0x1000, CRC(94ab7060) SHA1(3389bc115d1df8d01a30611fa9e95a900d32b29b) )

	ROM_REGION( 419696, "svg", 0)
	ROM_LOAD( "gjungler.svg", 0, 419696, CRC(c5f6d1f2) SHA1(5032f35326ca689c8e329f760e380cdc9f5dff86) )
ROM_END



//    YEAR  NAME      PARENT CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1981, cfrogger, 0,      0, cfrogger, cfrogger, cfrogger_state, empty_init, "Coleco", "Frogger (Coleco)", MACHINE_SUPPORTS_SAVE )

CONS( 1982, gjungler, 0,      0, gjungler, gjungler, gjungler_state, empty_init, "Gakken / Konami", "Jungler (Gakken)", MACHINE_SUPPORTS_SAVE )
