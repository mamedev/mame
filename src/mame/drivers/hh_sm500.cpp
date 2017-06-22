// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Sharp SM500(and derivatives) handhelds.

  TODO:
  - it's a skeleton driver

***************************************************************************/

#include "emu.h"

#include "cpu/sm510/sm500.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"

//#include "hh_sm500_test.lh" // common test-layout - use external artwork


class hh_sm500_state : public driver_device
{
public:
	hh_sm500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_speaker(*this, "speaker")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<2> m_inp_matrix; // max 2
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u16 m_inp_mux;                 // multiplexed inputs mask

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_sm500_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
}

void hh_sm500_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  IM-02 Nu, Pogodi!
  * KB1013VK1-2, die label V2-2 VK1-2

***************************************************************************/

class nupogodi_state : public hh_sm500_state
{
public:
	nupogodi_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm500_state(mconfig, type, tag)
	{ }
};

// handlers


// config

static INPUT_PORTS_START( nupogodi )
INPUT_PORTS_END

static MACHINE_CONFIG_START( nupogodi )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KB1013VK12, XTAL_32_768kHz)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( nupogodi )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "nupogodi.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )
ROM_END



//    YEAR  NAME       PARENT   CMP MACHINE    INPUT      STATE        INIT  COMPANY, FULLNAME, FLAGS
CONS( 1984, nupogodi,  0,        0, nupogodi,  nupogodi,  nupogodi_state, 0, "Elektronika", "Nu, pogodi!", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
