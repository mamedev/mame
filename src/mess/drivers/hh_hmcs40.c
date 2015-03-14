// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  Hitachi HMCS40 MCU tabletops/handhelds or other simple devices.

  known chips:

  serial  device    etc.
----------------------------------------------------------------
 *07      HD38750A  1979, Bambino Knock-Em Out Boxing (ET-06B)
 @08      HD38750A  1979, Bambino Basketball (ET-05)
 *45      HD38750A  1981, Vtech Invaders
 *58      HD38750A  1982, Ludotronic(Hanzawa) Grand Prix Turbo
 *62      HD38750A  1982, Actronics(Hanzawa) Pack'n Maze

 *04      HD38800A  1980, Gakken Heiankyo Alien
 *23      HD38800A  198?, Tomy Kingman
 *24      HD38800B  1982, Actronics(Hanzawa) Wanted G-Man
 @25      HD38800A  1981, Coleco Alien Attack
 @27      HD38800A  1981, Bandai Packri Monster (DM-21Z)
 *29      HD38800B  1984, Tomy Portable 6000 Bombman
 *35      HD38800B  1983, Bandai Gundam vs Gelgoog Zaku
 *43      HD38800B  1983, Bandai Dokodemo Dorayaki Doraemon
 *51      HD38800A  1981, Actronics(Hanzawa) Twinvader
 @70      HD38800A  1982, Coleco Galaxian

 @13      HD38820A  1981, Entex Galaxian 2
 @23      HD38820A  1981, Entex Pac Man 2
 @28      HD38820A  1981, Coleco Pac-Man (ver 1)
 @29      HD38820A  1981, Coleco Pac-Man (ver 2)
 *32      HD38820A  198?, Gakken Super Cobra
 *38      HD38820A  1982, Entex Crazy Climber
 *43      HD38820A  1982, Entex Turtles (have dump, +COP411 for audio)
 @45      HD38820A  1982, Coleco Donkey Kong
 @49      HD38820A  1983, Bandai Zackman
 @70      HD38820A  1983, Parker Brothers Q*Bert
 @88      HD38820A  1984, Tomy Tron (THN-02)

  (* denotes not yet emulated by MESS, @ denotes it's in this driver)

***************************************************************************/

#include "emu.h"
#include "cpu/hmcs40/hmcs40.h"
#include "sound/speaker.h"

#include "hh_hmcs40_test.lh" // test-layout - use external artwork


class hh_hmcs40_state : public driver_device
{
public:
	hh_hmcs40_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<7> m_inp_matrix; // max 7
	optional_device<speaker_sound_device> m_speaker;
	
	// misc common
	UINT16 m_inp_mux;

	UINT16 read_inputs(int columns);

	virtual void machine_start();

	// display common
	int m_display_wait;
	int m_display_maxy;
	int m_display_maxx;
	
	UINT32 m_grid;
	UINT32 m_plate;
	
	UINT32 m_display_state[0x20];
	UINT32 m_display_cache[0x20];
	UINT8 m_display_decay[0x20][0x20];
	UINT16 m_7seg_mask[0x20];

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety);

	// game-specific handlers
	DECLARE_WRITE8_MEMBER(alnattck_plate_w);
	DECLARE_READ16_MEMBER(alnattck_d_r);
	DECLARE_WRITE16_MEMBER(alnattck_d_w);
};


void hh_hmcs40_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, 0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_7seg_mask, 0, sizeof(m_7seg_mask));
	
	m_inp_mux = 0;
	m_grid = 0;
	m_plate = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_cache));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_7seg_mask));

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_hmcs40_state::display_update()
{
	UINT32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x < m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			int ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_7seg_mask[y] != 0)
				output_set_digit_value(y, active_state[y] & m_7seg_mask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x < m_display_maxx; x++)
				output_set_lamp_value(y * mul + x, active_state[y] >> x & 1);
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_hmcs40_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x < m_display_maxx; x++)
			if (!(m_display_state[y] >> x & 1) && m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;
	
	display_update();
}

void hh_hmcs40_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? (setx & mask) : 0;
	
	display_update();
}


UINT16 hh_hmcs40_state::read_inputs(int columns)
{
	UINT16 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}



/***************************************************************************

  Minidrivers (I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Bambino Basketball (manufactured in Japan)
  * boards are labeled Emix Corp. ET-05
  * Hitachi HD38750A08 MCU
  * green VFD display Emix-106

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( bambball )
INPUT_PORTS_END


static MACHINE_CONFIG_START( bambball, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38750, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Packri Monster (manufactured in Japan)
  * board label DM-21ZA2
  * Hitachi HD38800A27 MCU
  * cyan/red/green VFD display Futaba DM-21ZK 2B

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( packmon )
INPUT_PORTS_END


static MACHINE_CONFIG_START( packmon, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Bandai Zackman
  * Hitachi HD38820A49 MCU
  * cyan/red/yellow VFD display Futaba DM-53Z 3E, with color overlay

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( zackman )
INPUT_PORTS_END


static MACHINE_CONFIG_START( zackman, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Alien Attack (manufactured in Taiwan)
  * Hitachi HD38800A25 MCU
  * cyan/red VFD display Futaba DM-19Z 1J

  NOTE!: MESS external artwork is recommended

***************************************************************************/

WRITE8_MEMBER(hh_hmcs40_state::alnattck_plate_w)
{
	// R0x-R3x, D0-D3: vfd matrix plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	UINT32 plate = BITSWAP16((UINT16)m_plate,11,9,8,10,7,2,0,1,3,4,5,6,12,13,14,15) | (m_plate & 0xf0000);
	
	display_matrix(20, 10, plate, m_grid);
}

READ16_MEMBER(hh_hmcs40_state::alnattck_d_r)
{
	// D5: inputs
	return (read_inputs(7) & 1) << 5;
}

WRITE16_MEMBER(hh_hmcs40_state::alnattck_d_w)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);
	
	// D7-D13: input mux
	m_inp_mux = data >> 7 & 0x7f;

	// D6-D15: vfd matrix grid
	m_grid = data >> 6 & 0x3ff;
	
	// D0-D3: plate 16-19 (update display there)
	alnattck_plate_w(space, 4, data & 0xf);
}


static INPUT_PORTS_START( alnattck )
	PORT_START("IN.0") // D5 D7
	PORT_CONFNAME( 0x01, 0x00, "Skill" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )

	PORT_START("IN.1") // D5 D8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.2") // D5 D9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.3") // D5 D10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.4") // D5 D11
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN.5") // D5 D12
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Move")

	PORT_START("IN.6") // D5 D13
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fire")
INPUT_PORTS_END


static MACHINE_CONFIG_START( alnattck, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(hh_hmcs40_state, alnattck_plate_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(hh_hmcs40_state, alnattck_plate_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(hh_hmcs40_state, alnattck_plate_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(hh_hmcs40_state, alnattck_plate_w))
	MCFG_HMCS40_READ_D_CB(READ16(hh_hmcs40_state, alnattck_d_r))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(hh_hmcs40_state, alnattck_d_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Donkey Kong (manufactured in Taiwan)
  * board label Coleco Rev C 75790 DK
  * Hitachi HD38820A45 MCU
  * cyan/red VFD display Futaba DM-47ZK 2K, with color overlay

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( cdkong )
INPUT_PORTS_END


static MACHINE_CONFIG_START( cdkong, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Galaxian (manufactured in Taiwan)
  * board label Coleco Rev A 75718
  * Hitachi HD38800A70 MCU
  * cyan/red VFD display Futaba DM-36Z 2H, with color overlay

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( cgalaxn )
INPUT_PORTS_END


static MACHINE_CONFIG_START( cgalaxn, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Pac-Man (manufactured in Taiwan)
  * board label Coleco 75690
  * Hitachi HD38820A28/29 MCU
  * cyan/red VFD display Futaba DM-34Z 2A, with color overlay

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( cpacman )
INPUT_PORTS_END


static MACHINE_CONFIG_START( cpacman, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Galaxian 2 (manufactured in Japan)
  * Hitachi HD38820A13 MCU
  * cyan/red/green VFD display Futaba DM-20

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( egalaxn2 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( egalaxn2, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Pac Man 2 (manufactured in Japan)
  * Hitachi HD38820A23 MCU
  * cyan/red VFD display Futaba DM-28Z 1G

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( epacman2 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( epacman2, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Q*Bert
  * Hitachi HD38820A70 MCU
  * cyan/red/green/darkgreen VFD display Itron CP5137

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( pbqbert )
INPUT_PORTS_END


static MACHINE_CONFIG_START( pbqbert, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38820, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronic) Tron (manufactured in Japan)
  * boards are labeled THN-02 2E114E07
  * Hitachi HD38800A88 MCU
  * cyan/red/green VFD display NEC FIP10AM24T

  NOTE!: MESS external artwork is recommended

***************************************************************************/

static INPUT_PORTS_START( tmtron )
INPUT_PORTS_END


static MACHINE_CONFIG_START( tmtron, hh_hmcs40_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD38800, 400000) // approximation - RC osc.

//	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_hmcs40_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_hmcs40_test)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bambball )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38750a08", 0x0000, 0x0800, CRC(907fef18) SHA1(73fe7ca7c6332268a3a9abc5ac88ada2991012fb) )
	ROM_CONTINUE(           0x0f00, 0x0080 )
ROM_END


ROM_START( packmon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a27", 0x0000, 0x1000, CRC(86e09e84) SHA1(ac7d3c43667d5720ca513f8ff51d146d9f2af124) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( zackman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a49", 0x0000, 0x1000, CRC(b97f5ef6) SHA1(7fe20e8107361caf9ea657e504be1f8b10b8b03f) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( alnattck )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a25", 0x0000, 0x1000, CRC(18b50869) SHA1(11e9d5f7b4ae818b077b0ee14a3b43190e20bff3) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( cdkong )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a45", 0x0000, 0x1000, CRC(196b8070) SHA1(da85d1eb4b048b77f3168630662ab94ec9baa262) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( cgalaxn )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a70", 0x0000, 0x1000, CRC(a4c5ed1d) SHA1(0f647cb78437d7e62411febf7c9ce3c5b6753a80) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( cpacman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a29", 0x0000, 0x1000, CRC(1082d577) SHA1(0ef73132bd41f6ca1e4c001ae19f7f7c97eaa8d1) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END

ROM_START( cpacmanr1 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a28", 0x0000, 0x1000, CRC(d2ed57e5) SHA1(f56f1341485ac28ea9e6cc4d162fab18d8a4c977) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( egalaxn2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a13", 0x0000, 0x1000, CRC(112b721b) SHA1(4a185bc57ea03fe64f61f7db4da37b16eeb0cb54) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( epacman2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a23", 0x0000, 0x1000, CRC(6eab640f) SHA1(509bdd02be915089e13769f22a08e03509f03af4) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( pbqbert )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a70", 0x0000, 0x1000, CRC(be7c80b4) SHA1(0617a80ef7fe188ea221de32e760d45fd4318c67) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END


ROM_START( tmtron )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a88", 0x0000, 0x1000, CRC(33db9670) SHA1(d6f747a59356526698784047bcfdbb59e79b9a23) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1979, bambball,  0,        0, bambball, bambball, driver_device, 0, "Bambino", "Basketball (Bambino)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )

CONS( 1981, packmon,   0,        0, packmon,  packmon,  driver_device, 0, "Bandai", "Packri Monster", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1983, zackman,   0,        0, zackman,  zackman,  driver_device, 0, "Bandai", "Zackman", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )

CONS( 1981, alnattck,  0,        0, alnattck, alnattck, driver_device, 0, "Coleco", "Alien Attack", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1982, cdkong,    0,        0, cdkong,   cdkong,   driver_device, 0, "Coleco", "Donkey Kong (Coleco)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1982, cgalaxn,   0,        0, cgalaxn,  cgalaxn,  driver_device, 0, "Coleco", "Galaxian (Coleco)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1981, cpacman,   0,        0, cpacman,  cpacman,  driver_device, 0, "Coleco", "Pac-Man (Coleco, Rev. 29)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1981, cpacmanr1, cpacman,  0, cpacman,  cpacman,  driver_device, 0, "Coleco", "Pac-Man (Coleco, Rev. 28)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )

CONS( 1981, egalaxn2,  0,        0, egalaxn2, egalaxn2, driver_device, 0, "Entex", "Galaxian 2 (Entex)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1981, epacman2,  0,        0, epacman2, epacman2, driver_device, 0, "Entex", "Pac Man 2 (Entex)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
CONS( 1983, pbqbert,   0,        0, pbqbert,  pbqbert,  driver_device, 0, "Parker Brothers", "Q*Bert (Parker Brothers)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )

CONS( 1982, tmtron,    0,        0, tmtron,   tmtron,   driver_device, 0, "Tomy", "Tron (Tomy)", GAME_SUPPORTS_SAVE | GAME_REQUIRES_ARTWORK | GAME_NOT_WORKING )
