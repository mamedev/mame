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


    TODO:
    - Dip Switches (Controls credits per coin), Currently 2 coins per credit
    - Test Mode Won't Activate
    - Layout with Lamps
    - Default monitor is yellow/amber, no colour (board does have an extra
      composite-out connector though, allowing a standard tv)
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/ram.h"
#include "machine/6840ptm.h"
#include "video/tms9928a.h"
#include "sound/speaker.h"

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
	arachnid_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M6809_TAG),
			m_pia_u4(*this, PIA6821_U4_TAG),
			m_pia_u17(*this, PIA6821_U17_TAG),
			m_speaker(*this, SPEAKER_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia_u4;
	required_device<pia6821_device> m_pia_u17;
	required_device<speaker_sound_device> m_speaker;

	virtual void machine_start() override;
	DECLARE_READ8_MEMBER( pia_u4_pa_r );
	DECLARE_READ8_MEMBER( pia_u4_pb_r );
	DECLARE_READ_LINE_MEMBER( pia_u4_pca_r );
	DECLARE_READ_LINE_MEMBER( pia_u4_pcb_r );
	DECLARE_WRITE8_MEMBER( pia_u4_pa_w );
	DECLARE_WRITE8_MEMBER( pia_u4_pb_w );
	DECLARE_WRITE_LINE_MEMBER( pia_u4_pca_w );
	DECLARE_WRITE_LINE_MEMBER( pia_u4_pcb_w );

	DECLARE_READ8_MEMBER( pia_u17_pa_r );
	DECLARE_READ_LINE_MEMBER( pia_u17_pca_r );
	DECLARE_WRITE8_MEMBER( pia_u17_pb_w );
	DECLARE_WRITE_LINE_MEMBER( pia_u17_pcb_w );

	DECLARE_WRITE8_MEMBER(ptm_o1_callback);

	UINT8 read_keyboard(int pa);
};

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( arachnid_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( arachnid_map, AS_PROGRAM, 8, arachnid_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2007) AM_DEVREADWRITE(PTM6840_TAG, ptm6840_device, read, write)
	AM_RANGE(0x4004, 0x4007) AM_DEVREADWRITE(PIA6821_U4_TAG, pia6821_device, read, write)
	AM_RANGE(0x4008, 0x400b) AM_DEVREADWRITE(PIA6821_U17_TAG, pia6821_device, read, write)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE(TMS9118_TAG, tms9928a_device, vram_write)
	AM_RANGE(0x6002, 0x6002) AM_DEVWRITE(TMS9118_TAG, tms9928a_device, register_write)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(M6809_TAG, 0)
ADDRESS_MAP_END

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

WRITE8_MEMBER(arachnid_state::ptm_o1_callback)
{
	m_speaker->level_w(data);
}

UINT8 arachnid_state::read_keyboard(int pa)
{
	int i;
	UINT8 value;
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

READ8_MEMBER( arachnid_state::pia_u4_pa_r )
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

	UINT8 data = 0xff;
	data &= read_keyboard(1);

	return data;
}

READ8_MEMBER( arachnid_state::pia_u4_pb_r )
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

	UINT8 data = 0xff;
	data &= read_keyboard(2);

	return data;
}

READ_LINE_MEMBER( arachnid_state::pia_u4_pca_r )
{
	// CA1 - SW1 Coin In (Coin Door)

	UINT8 data = 1;
	data &= ioport("SW1")->read();

	return data;
}

READ_LINE_MEMBER( arachnid_state::pia_u4_pcb_r )
{
	// CB1 - SW2 Test Mode (Coin Door)

	UINT8 data = 1;
	data &= ioport("SW2")->read();

	return data;
}

READ8_MEMBER( arachnid_state::pia_u17_pa_r )
{
	// PA0 - Select
	// PA1 - Player Change
	// PA2 - Coin
	// PA3 - Test
	// PA4 thru PA7 - DIP SW1

	UINT8 data = 0xff;
	data &= read_keyboard(0);

	return data;
}

READ_LINE_MEMBER( arachnid_state::pia_u17_pca_r )
{
	// CA1 - 1000 HZ Input

	UINT8 data = 1;

	return data;
}

WRITE8_MEMBER( arachnid_state::pia_u4_pa_w )
{
	// PA0 thru PA7 Pulses to Switch Matrix Part I
}

WRITE8_MEMBER( arachnid_state::pia_u4_pb_w )
{
	// PA0 thru PA7 Pulses to Switch Matrix Part II
}

WRITE_LINE_MEMBER( arachnid_state::pia_u4_pca_w )
{
	// CA1 - Remove Darts Lamp
}

WRITE_LINE_MEMBER( arachnid_state::pia_u4_pcb_w )
{
	// CB2 - Throw Darts Lamp
}

WRITE8_MEMBER( arachnid_state::pia_u17_pb_w )
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

WRITE_LINE_MEMBER( arachnid_state::pia_u17_pcb_w )
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
    MACHINE_CONFIG_START( arachnid, arachnid_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( arachnid, arachnid_state )
	// basic machine hardware
	MCFG_CPU_ADD(M6809_TAG, M6809, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(arachnid_map)

	// devices
	MCFG_DEVICE_ADD(PIA6821_U4_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(arachnid_state, pia_u4_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(arachnid_state, pia_u4_pb_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(arachnid_state, pia_u4_pca_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(arachnid_state, pia_u4_pcb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(arachnid_state, pia_u4_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(arachnid_state, pia_u4_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(arachnid_state, pia_u4_pca_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(arachnid_state, pia_u4_pcb_w))

	MCFG_DEVICE_ADD(PIA6821_U17_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(arachnid_state, pia_u17_pa_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(arachnid_state, pia_u17_pca_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(arachnid_state, pia_u17_pb_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(arachnid_state, pia_u17_pcb_w))

	// video hardware
	MCFG_DEVICE_ADD( TMS9118_TAG, TMS9118, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(INPUTLINE(M6809_TAG, INPUT_LINE_IRQ0))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9118_TAG, tms9118_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(XTAL_8MHz / 4)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT0_CB(WRITE8(arachnid_state, ptm_o1_callback))
MACHINE_CONFIG_END

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( arac6000 )
	ROM_REGION( 0x8000, M6809_TAG, 0 )
	ROM_LOAD( "01-0140-6300-v2.7-19910208.u15", 0x0000, 0x8000, CRC(f1c4412d) SHA1(6ff9a8f25f315c2df5c0785043521d036ec0964e) )
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY                   FULLNAME */
GAME( 1990, arac6000,   0,         arachnid,        arachnid, driver_device,    0,    ROT0,  "Arachnid",         "Super Six Plus II English Mark Darts", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
