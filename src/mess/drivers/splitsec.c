// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Split Second
  * TMS1400NLL MP7314-N2 (die labeled MP7314)


***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "splitsec.lh"

// master clock is a single stage RC oscillator: R=24K, C=100pf,
// according to the TMS 1000 series data manual this is around 375kHz
#define MASTER_CLOCK (375000)


class splitsec_state : public driver_device
{
public:
	splitsec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
//		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
//	required_ioport_array<4> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_r;
	UINT16 m_o;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();
};



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(splitsec_state::read_k)
{
	return 0;
}

WRITE16_MEMBER(splitsec_state::write_r)
{
	m_r = data;
}

WRITE16_MEMBER(splitsec_state::write_o)
{
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( splitsec )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void splitsec_state::machine_start()
{
	m_r = 0;
	m_o = 0;

	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( splitsec, splitsec_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(splitsec_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(splitsec_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(splitsec_state, write_r))

	MCFG_DEFAULT_LAYOUT(layout_splitsec)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( splitsec )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tms1400nll_mp7314", 0x0000, 0x1000, CRC(0cccdf59) SHA1(06a533134a433aaf856b80f0ca239d0498b98d5f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_splitsec_opla.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END


CONS( 1980, splitsec, 0, 0, splitsec, splitsec, driver_device, 0, "Parker Brothers", "Split Second", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
