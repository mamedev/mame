// license:BSD-3-Clause
// copyright-holders:Jim Stolis
/*
    Arachnid - English Mark Darts

    Driver by Jim Stolis.

    --- Technical Notes ---
    Name:    English Mark Darts
    Company: Arachnid, Inc.
    Year:    1987/88/89/90

    --- Hardware ---
    A 6809 CPU (U3) is clocked by a 556 (U2) circuit with 3 Pin addressing decoding via a 74LS138 (U14)
    (this information seems incorrect: CPU clock is almost certainly sourced from the VDP's CPUCLK output)
    Program ROM is a 27256 (U15)
    Two 6821 PIAs (U4/U17) are used for I/O
    Video is processed via a TMS9118 (U11) with two TMS4416 (U12/U13) as RAM
    Main RAM is a 2K 6116 (U23) chip
    Sound is generated via a PTM 6840 (U16) directly to an amplified speaker

    --- Target Interface Board ---
    The target interface board is used to combine 33 conductors from the switch matrix
    into 16 conductors.  The middle 13 pin connector is common to all switches.

    3 connectors and their labels
    EFBHDACGH   NMPLMNJOMIKOP   EBACFDCEAHB

    Switch Matrix Table
    Score   Single  Double  Triple
    1       DN      EN      FN
    2       AL      BL      CL
    3       AN      BN      CN
    4       DL      EL      FL
    5       AP      BP      CP
    6       GL      HL      GP
    7       DO      EO      FO
    8       GI      HI      GM
    9       AO      BO      CO
    10      AI      BI      CI
    11      AK      BK      CK
    12      DP      EP      FP
    13      AM      BM      CM
    14      GK      HK      GO
    15      GJ      HJ      GN
    16      AJ      BJ      CJ
    17      DM      EM      FM
    18      DI      EI      FI
    19      DJ      EJ      FJ
    20      DK      EK      FK
    Bull    --      HM      --

Notes about dipswitches:

There's a separate additional subboard just for dipswitches:
 ___________________________________________________
 |                                                  |
 | MM74C14N       __________           _____        |___
 |  ____         |CD4013BCN|           |SW |          __|
 |  |  |                               |_1_|          __|
 |  |  |          __________           _____          __|
 |  |  |         |HCF4520BE|           |SW |          __|
 |  |__|                               |_2_|          __|
 |            __________               _____          __|
 |           |MC14093BCP|              |SW |          __|
 |                                     |_3_|         ___|
 |                                                  |
 |__________________________________________________|

And a sticker on the cab with the following manufacturer instructions about
the dipswitches:

--------------------------------------------------
Coin credit options for 6000 and 7000 series games
--------------------------------------------------

Figure 1 shows the available coinage variances for each coin slot.
Set SW1 to desired position shown at FIGURE 1 to control the right facing coin slot.
Set SW2 in the same manner to control the left facing coin slot.
Set SW3 so that at least one switch is "ON", it doesn't matter which one(s).

                                                    FIGURE 2
        FIGURE 1                                  DIP SWITCHES
        SWITCH #                            (SWITCHES ARE DISPLAYED AS
  _____________________                         SET AT THE FACTORY)
  |   | 4 | 3 | 2 | 1 |
# +---+---+---+---+---|       .....................................................
  | 1 | 0 | 0 | 0 | 1 |       |                                                   |
O +---+---+---+---+---|       |  1   2   3   4    1   2   3   4    1   2   3   4  |
F | 2 | 0 | 0 | 1 | 0 |       |  ON OFF OFF OFF   ON OFF OFF OFF   ON OFF OFF OFF |
  +---+---+---+---+---|       |       SW3              SW2              SW1       |
C | 3 | 0 | 0 | 1 | 1 |       |_________                                  ________|
R +---+---+---+---+---|                 |                                |
E | 4 | 0 | 1 | 0 | 0 |                 |__|__|__|__|__|__|__|__|__|__|__|
D +---+---+---+---+---|
I | 5 | 0 | 1 | 0 | 1 |                                FIGURE 3
T +---+---+---+---+---|   IMPORTANT: IF USING THE MAIN BOARD DIP SWITCH COIN OPTIONS, SET ALL THREE
S | 6 | 0 | 1 | 1 | 0 |   COIN CREDIT DIP SWITCHES SO THAT THEY READ 1(ON) 2(OFF) 3(OFF) 4(OFF)
  +---+---+---+---+---|          ______________________________________________________
  | 7 | 0 | 1 | 1 | 1 |          |          |                  | MAIN BOARD DIP SWITCH |
  +---+---+---+---+---|          |          |                  |      POSITIONS        |
  | 8 | 1 | 0 | 0 | 0 |          |# OF COINS|GIVES # OF CREDITS|-----+-----+-----+-----|
  +---+---+---+---+---|          |          |                  |  1  |  2  |  3  |  4  |
  | 9 | 1 | 0 | 0 | 1 |          |----------+------------------+-----+-----+-----+-----|
  +---+---+---+---+---|          |    4     |         3        |  ON | OFF | OFF | OFF |
  |10 | 1 | 0 | 1 | 0 |          |----------+------------------+-----+-----+-----+-----|
  +---+---+---+---+---|          |    4     |         5        | OFF |  ON | OFF | OFF |
  |11 | 1 | 0 | 1 | 1 |          |----------+------------------+-----+-----+-----+-----|
  +---+---+---+---+---|          |--------------------------+--------------------------|
  |12 | 1 | 1 | 0 | 0 |          |   FOR 4 COIN / 3 CREDITS | FOR 4 COIN / 5 CREDITS   |
  +---+---+---+---+---|          |--------------------------+--------------------------|
  |13 | 1 | 1 | 0 | 1 |          | COIN 1 = 0 CREDIT        | COIN 1 = 1 CREDIT        |
  +---+---+---+---+---|          | COIN 2 = 1 CREDIT        | COIN 2 = 1 CREDIT        |
  |14 | 1 | 1 | 1 | 0 |          | COIN 3 = 1 CREDIT        | COIN 3 = 1 CREDIT        |
  +---+---+---+---+---|          | COIN 4 = 1 CREDIT        | COIN 4 = 2 CREDIT        |
  |15 | 1 | 1 | 1 | 1 |          |       -----------------  |       -----------------  |
  +---+---+---+---+---|          |        = 3 CREDITS TOTAL |        = 5 CREDITS TOTAL |
                                 |__________________________|__________________________|


              FIGURE 4
     MAIN PC BOARD DIP SWITCHES
 IMPORTANT: SWITCHES 1&2 MUST BE IN
THE OFF POSIION UNLESS THE COINAGES
      LISTED IN FIG.3 ARE USED

            1   2   3   4
          OFF OFF OFF OFF


    TODO:
    - Dip Switches (Controls credits per coin), Currently 2 coins per credit
    - Test Mode Won't Activate
    - Layout with Lamps
    - Default monitor is yellow/amber, no colour (board does have an extra
      composite-out connector though, allowing a standard TV)
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"
#include "sound/spkrdev.h"
#include "video/tms9928a.h"
#include "speaker.h"


namespace {

#define SCREEN_TAG      "screen"
#define M6809_TAG       "u3"
#define TMS9118_TAG     "u11"
#define PIA6821_U4_TAG  "u4"
#define PIA6821_U17_TAG "u17"
#define PTM6840_TAG     "u16"
#define SPEAKER_TAG     "speaker"

class arachnid_state : public driver_device
{
public:
	arachnid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M6809_TAG),
			m_pia_u4(*this, PIA6821_U4_TAG),
			m_pia_u17(*this, PIA6821_U17_TAG),
			m_speaker(*this, SPEAKER_TAG)
	{ }

	void arachnid(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia_u4;
	required_device<pia6821_device> m_pia_u17;
	required_device<speaker_sound_device> m_speaker;

	virtual void machine_start() override ATTR_COLD;
	uint8_t pia_u4_pa_r();
	uint8_t pia_u4_pb_r();
	int pia_u4_pca_r();
	int pia_u4_pcb_r();
	void pia_u4_pa_w(uint8_t data);
	void pia_u4_pb_w(uint8_t data);
	void pia_u4_pca_w(int state);
	void pia_u4_pcb_w(int state);

	uint8_t pia_u17_pa_r();
	void pia_u17_pb_w(uint8_t data);
	void pia_u17_pcb_w(int state);

	void ptm_o1_callback(int state);

	uint8_t read_keyboard(int pa);
	void arachnid_map(address_map &map) ATTR_COLD;
};

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( arachnid_map )
-------------------------------------------------*/

void arachnid_state::arachnid_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2000, 0x2007).rw(PTM6840_TAG, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x4004, 0x4007).rw(m_pia_u4, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4008, 0x400b).rw(m_pia_u17, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x6000).w(TMS9118_TAG, FUNC(tms9928a_device::vram_write));
	map(0x6002, 0x6002).w(TMS9118_TAG, FUNC(tms9928a_device::register_write));
	map(0x8000, 0xffff).rom().region(M6809_TAG, 0);
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_PORTS( arachnid )
-------------------------------------------------*/

static INPUT_PORTS_START( arachnid )
	PORT_START("PA0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') // SELECT
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') // PLAYER
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) // COIN
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') // TEST
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_TOGGLE
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_TOGGLE
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	// Matrix Switch Part I
	PORT_START("PA1-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	// Matrix Switch Part II
	PORT_START("PB1-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PB1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    ptm6840_interface ptm_intf
-------------------------------------------------*/

void arachnid_state::ptm_o1_callback(int state)
{
	m_speaker->level_w(state);
}

uint8_t arachnid_state::read_keyboard(int pa)
{
	int i;
	uint8_t value;
	static const char *const keynames[3][8] =
			{
				{ "PA0-0", "PA0-1", "PA0-2", "PA0-3", "PA0-4", "PA0-5", "PA0-6", "PA0-7" },
				{ "PA1-0", "PA1-1", "PA1-2", "PA1-3", "PA1-4", "PA1-5", "PA1-6", "PA1-7" },
				{ "PB1-0", "PB1-1", "PB1-2", "PB1-3", "PB1-4", "PB1-5", "PB1-6", "PB1-7" }
			};

	for (i = 0; i < 8; i++)
	{
		value = ioport(keynames[pa][i])->read();

		if (value != 0xff)
		{
			if (value == 0xff - (1 << i))
				return value;
			else
				return value - (1 << i);
		}
	}

	return 0xff;
}

uint8_t arachnid_state::pia_u4_pa_r()
{
	// Pulses from Switch Matrix Part I
	// PA0 - G
	// PA1 - H
	// PA2 - E
	// PA3 - F
	// PA4 - C
	// PA5 - D
	// PA6 - A
	// PA7 - B

	uint8_t data = 0xff;
	data &= read_keyboard(1);

	return data;
}

uint8_t arachnid_state::pia_u4_pb_r()
{
	// Pulses from Switch Matrix Part II
	// PB0 - J
	// PB1 - I
	// PB2 - L
	// PB3 - K
	// PB4 - N
	// PB5 - M
	// PB6 - P
	// PB7 - O

	uint8_t data = 0xff;
	data &= read_keyboard(2);

	return data;
}

int arachnid_state::pia_u4_pca_r()
{
	// CA1 - SW1 Coin In (Coin Door)

	uint8_t data = 1;
	data &= ioport("SW1")->read();

	return data;
}

int arachnid_state::pia_u4_pcb_r()
{
	// CB1 - SW2 Test Mode (Coin Door)

	uint8_t data = 1;
	data &= ioport("SW2")->read();

	return data;
}

uint8_t arachnid_state::pia_u17_pa_r()
{
	// PA0 - Select
	// PA1 - Player Change
	// PA2 - Coin
	// PA3 - Test
	// PA4 thru PA7 - DIP SW1

	uint8_t data = 0xff;
	data &= read_keyboard(0);

	return data;
}

void arachnid_state::pia_u4_pa_w(uint8_t data)
{
	// PA0 thru PA7 Pulses to Switch Matrix Part I
}

void arachnid_state::pia_u4_pb_w(uint8_t data)
{
	// PA0 thru PA7 Pulses to Switch Matrix Part II
}

void arachnid_state::pia_u4_pca_w(int state)
{
	// CA1 - Remove Darts Lamp
}

void arachnid_state::pia_u4_pcb_w(int state)
{
	// CB2 - Throw Darts Lamp
}

void arachnid_state::pia_u17_pb_w(uint8_t data)
{
	// PB0 - Select Lamp
	// PB1 - Player Change Lamp
	// PB2 - Not Used
	// PB3 - Not Used
	// PB4 - Not Used
	// PB5 - Not Used
	// PB6 - Not Used
	// PB7 - N/C
}

void arachnid_state::pia_u17_pcb_w(int state)
{
	// CB2 - Target Lamp
}

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    MACHINE_START( arachnid )
-------------------------------------------------*/

void arachnid_state::machine_start()
{
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    machine_config( arachnid )
-------------------------------------------------*/

void arachnid_state::arachnid(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 10.738635_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &arachnid_state::arachnid_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z02 (or DS1220Y)

	// devices
	PIA6821(config, m_pia_u4);
	m_pia_u4->readpa_handler().set(FUNC(arachnid_state::pia_u4_pa_r));
	m_pia_u4->readpb_handler().set(FUNC(arachnid_state::pia_u4_pb_r));
	m_pia_u4->readca1_handler().set(FUNC(arachnid_state::pia_u4_pca_r));
	m_pia_u4->readcb1_handler().set(FUNC(arachnid_state::pia_u4_pcb_r));
	m_pia_u4->writepa_handler().set(FUNC(arachnid_state::pia_u4_pa_w));
	m_pia_u4->writepb_handler().set(FUNC(arachnid_state::pia_u4_pb_w));
	m_pia_u4->ca2_handler().set(FUNC(arachnid_state::pia_u4_pca_w));
	m_pia_u4->cb2_handler().set(FUNC(arachnid_state::pia_u4_pcb_w));

	PIA6821(config, m_pia_u17);
	m_pia_u17->readpa_handler().set(FUNC(arachnid_state::pia_u17_pa_r));
	m_pia_u17->ca1_w(1); // CA1 - 1000 HZ Input
	m_pia_u17->writepb_handler().set(FUNC(arachnid_state::pia_u17_pb_w));
	m_pia_u17->cb2_handler().set(FUNC(arachnid_state::pia_u17_pcb_w));

	// video hardware
	tms9118_device &vdp(TMS9118(config, TMS9118_TAG, 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.0);

	ptm6840_device &ptm(PTM6840(config, PTM6840_TAG, 10.738635_MHz_XTAL / 3 / 4));
	ptm.set_external_clocks(0, 0, 0);
	ptm.o1_callback().set(FUNC(arachnid_state::ptm_o1_callback));
}

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( arac6k33 )
	ROM_REGION( 0x8000, M6809_TAG, 0 )
	ROM_LOAD( "arachnid_6300t_v33.u15",         0x0000, 0x8000, CRC(6c1b12df) SHA1(a78f19aaa4f3d1f2786cbaf0f19b1b36839a1be0) )
ROM_END

ROM_START( arac6k27 )
	ROM_REGION( 0x8000, M6809_TAG, 0 )
	ROM_LOAD( "01-0140-6300-v2.7-19910208.u15", 0x0000, 0x8000, CRC(f1c4412d) SHA1(6ff9a8f25f315c2df5c0785043521d036ec0964e) )
ROM_END

ROM_START( arac6k28sp )
	ROM_REGION( 0x8000, M6809_TAG, 0 )
	ROM_LOAD( "1.u15",                          0x0000, 0x8000, CRC(397e890e) SHA1(5b532b046f36dcfbd7118bd5a0fab3436b0b8dc1) )
ROM_END

} // anonymous namespace


/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

//    YEAR  NAME        PARENT    MACHINE   INPUT     STATE           INIT        MONITOR  COMPANY     FULLNAME
GAME( 1994, arac6k33,   0,        arachnid, arachnid, arachnid_state, empty_init, ROT0,    "Arachnid", "Super Six Plus II English Mark Darts (v3.3)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING ) // 6300T v3.3
GAME( 1990, arac6k27,   arac6k33, arachnid, arachnid, arachnid_state, empty_init, ROT0,    "Arachnid", "Super Six Plus II English Mark Darts (v2.7)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING ) // 6300 v2.7
GAME( 1990, arac6k28sp, arac6k33, arachnid, arachnid, arachnid_state, empty_init, ROT0,    "Arachnid", "Super Six Plus II English Mark Darts (v2.8, Spanish)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING ) // 6307 v2.8
