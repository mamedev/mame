// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:David Viens, Sean Riddle
/***************************************************************************

  Chrysler Electronic Voice Alert
  
  11-function board "EVA-11"
  - TMS1000 MCU (label 4230625-N1LL 32045B, die label 1000F M32045B)
  - TMS5110A, TMS6125 CM73002
  - 2 Nat.Semi. 20-pin SDIP, I/O expanders?
  
  24-function board "EVA-24"
  - COP400 family? MCU
  - TMS5110A, TMS6100 CM63002 (have dump)

  TODO:
  - make it work
  - add EVA-24 (need MCU dump)

***************************************************************************/

#include "emu.h"
#include "cpu/tms1000/tms1000.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"
#include "speaker.h"


class eva_state : public driver_device
{
public:
	eva_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;

	// EVA-11
	DECLARE_READ8_MEMBER(eva11_read_k);
	DECLARE_WRITE16_MEMBER(eva11_write_o);
	DECLARE_WRITE16_MEMBER(eva11_write_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void eva_state::machine_start()
{
}

void eva_state::machine_reset()
{
}



/***************************************************************************

  I/O

***************************************************************************/

WRITE16_MEMBER(eva_state::eva11_write_r)
{
	// R7: TMS5100 PDC pin
	m_tms5100->pdc_w(data >> 7 & 1);
}

WRITE16_MEMBER(eva_state::eva11_write_o)
{
	// O3210: TMS5100 CTL8124
	u8 ctl = BITSWAP8(data,7,6,5,4,3,0,1,2) & 0xf;
	m_tms5100->ctl_w(space, 0, ctl);
}

READ8_MEMBER(eva_state::eva11_read_k)
{
	// K8421: TMS5100 CTL8124
	u8 ctl = m_tms5100->ctl_r(space, 0);
	return BITSWAP8(ctl,7,6,5,4,3,0,1,2);
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( eva11 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( tms5110_route )

	/* sound hardware */
	MCFG_TMS5110_M0_CB(DEVWRITELINE("tms6100", tms6100_device, m0_w))
	MCFG_TMS5110_M1_CB(DEVWRITELINE("tms6100", tms6100_device, m1_w))
	MCFG_TMS5110_ADDR_CB(DEVWRITE8("tms6100", tms6100_device, add_w))
	MCFG_TMS5110_DATA_CB(DEVREADLINE("tms6100", tms6100_device, data_line_r))
	MCFG_TMS5110_ROMCLK_CB(DEVWRITELINE("tms6100", tms6100_device, clk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( eva11 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, XTAL_640kHz/2)
	MCFG_TMS1XXX_READ_K_CB(READ8(eva_state, eva11_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(eva_state, eva11_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(eva_state, eva11_write_r))

	/* sound hardware */
	MCFG_DEVICE_ADD("tms6100", TMS6100, XTAL_640kHz/4)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tms5100", TMS5110A, XTAL_640kHz)
	MCFG_FRAGMENT_ADD(tms5110_route)
MACHINE_CONFIG_END



/***************************************************************************

  ROM Defs, Game driver(s)

***************************************************************************/

ROM_START( eva11 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "32045b", 0x0000, 0x0400, CRC(eea36ebe) SHA1(094755b60965654ddc3e57cbd69f4749abd3b526) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_eva11_micro.pla", 0, 867, CRC(38fcc108) SHA1(be265300e0828b6ac83832b3279985a43817bb64) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_eva11_output.pla", 0, 365, CRC(f0f36970) SHA1(a6ad1f5e804ac98e5e1a1d07466b3db3a8d6c256) )

	ROM_REGION( 0x1000, "tms6100", ROMREGION_ERASEFF )
	ROM_LOAD( "cm73002.vsm", 0x0000, 0x1000, CRC(d5340bf8) SHA1(81195e8f870275d39a1abe1c8e2a6afdfdb15725) )
ROM_END



//    YEAR  NAME   PARENT CMP MACHINE INPUT  STATE   INIT  COMPANY, FULLNAME, FLAGS
SYST( 1983, eva11, 0,      0, eva11,  eva11, eva_state, 0, "Chrysler", "Electronic Voice Alert (11-function)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
