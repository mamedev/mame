// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Currah Speech 64 cartridge emulation

**********************************************************************/

/*

PCB Layout
----------

|===========================|
|=|                         |
|=|         VLSI            |
|=|                         |
|=|                         |
|=|         ROM             |
|=|                         |
|=|                         |
|=|         SP0256          |
|===========================|

Notes:
    All IC's shown.

    VLSI   - General Instruments LA05-164 custom
    ROM    - General Instruments R09864CS-2030 8Kx8 ROM "778R01"
    SP0256 - General Instruments SP0256A-AL2 Speech Synthesizer


LA05-164 Pinout
---------------
            _____   _____
DOTCLK   1 |*    \_/     | 28  +5V
   CA7   2 |             | 27  CD7
   CA6   3 |             | 26  CA8
   CA5   4 |             | 25  CA9
   CA4   5 |             | 24  CA11
   CA3   6 |             | 23  BA
   CA2   7 |  LA05-164   | 22  CA10
   CA1   8 |             | 21  SP0256 _ALD
   CA0   9 |             | 20  SP0256 OSC1
        10 |             | 19  SP0256 SBY
_GA+EX  11 |             | 18  CA15
  I/O1  12 |             | 17  CA12
 _CR/W  13 |             | 16  CA13
   GND  14 |_____________| 15  CA14

Notes:
    _GA+EX  - _GAME and _EXROM tied together

*/

/*

    BASIC Commands
    --------------

    INIT        Initialize the cartridge
    BYE         Disable the cartridge
    KON 0       Enable keyvoices, low voice
    KON 1       Enable keyvoices, high voice
    KOFF        Disable keyvoices
    SAY 0/1 ""  Say words

*/

#include "emu.h"
#include "currah_speech.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_CURRAH_SPEECH, c64_currah_speech_cartridge_device, "c64_cs", "C64 Currah Speech")


//-------------------------------------------------
//  ROM( c64_currah_speech )
//-------------------------------------------------

ROM_START( c64_currah_speech )
	ROM_REGION( 0x10000, "sp0256", 0 )
	ROM_LOAD( "sp0256a-al2", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_currah_speech_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_currah_speech );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_currah_speech_cartridge_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, 4000000); // ???
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.00);
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_osc1 -
//-------------------------------------------------

void c64_currah_speech_cartridge_device::set_osc1(int voice, int intonation)
{
	int dotclock = m_slot->dotclock();

	// TODO intonation and correct dividers
	m_nsp->set_clock(dotclock / (2 << voice));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_currah_speech_cartridge_device - constructor
//-------------------------------------------------

c64_currah_speech_cartridge_device::c64_currah_speech_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_CURRAH_SPEECH, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_nsp(*this, "sp0256")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_currah_speech_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_currah_speech_cartridge_device::device_reset()
{
	m_game = 1;
	m_exrom = 1;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_currah_speech_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!romh)
	{
		data = m_romh[offset & 0x1fff];
	}
	else if (!io1)
	{
		/*

		    bit     description

		    0
		    1
		    2
		    3
		    4
		    5
		    6
		    7       SBY

		*/

		data = m_nsp->sby_r() << 7;
	}

	if (!machine().side_effects_disabled() && (offset == 0xa7f0))
	{
		m_game = !m_game;
		m_exrom = !m_exrom;
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_currah_speech_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		/*

		    bit     description

		    0       A1
		    1       A2
		    2       A3
		    3       A4
		    4       A5
		    5       A6
		    6
		    7       intonation

		*/

		int voice = BIT(offset, 0);
		int intonation = BIT(data, 7);

		set_osc1(voice, intonation);

		m_nsp->ald_w(data & 0x3f);
	}
}
