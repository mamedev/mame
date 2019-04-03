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
  - COP420 MCU (custom label)
  - TMS5110A, TMS6100 CM63002
  - monitor board (unknown MCU, no dump), VFD panel

  TODO:
  - add eva11/eva24 sensors
  - add eva24 VFD

***************************************************************************/

#include "emu.h"
#include "cpu/cop400/cop400.h"
#include "cpu/tms1000/tms1000.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"
#include "speaker.h"


class eva_base_state : public driver_device
{
public:
	eva_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100")
	{ }

	void eva_sound(machine_config &config);

protected:
	// devices
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
};

class eva11_state : public eva_base_state
{
public:
	eva11_state(const machine_config &mconfig, device_type type, const char *tag) :
		eva_base_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void eva(machine_config &config);

private:
	// devices
	required_device<tms1000_cpu_device> m_maincpu;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);
};

class eva24_state : public eva_base_state
{
public:
	eva24_state(const machine_config &mconfig, device_type type, const char *tag) :
		eva_base_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void eva(machine_config &config);

private:
	virtual void machine_start() override;

	// devices
	required_device<cop420_cpu_device> m_maincpu;

	DECLARE_READ8_MEMBER(read_g);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_d);

	u8 m_g;
};

void eva24_state::machine_start()
{
	// zerofill
	m_g = 0;

	// register for savestates
	save_item(NAME(m_g));
}



/***************************************************************************

  I/O

***************************************************************************/

// EVA-24

WRITE8_MEMBER(eva24_state::write_g)
{
	// G3: TMS5100 PDC pin
	m_tms5100->pdc_w(data >> 3 & 1);
	m_g = data;
}

READ8_MEMBER(eva24_state::read_g)
{
	return m_g;
}

WRITE8_MEMBER(eva24_state::write_d)
{
	// D3210: TMS5100 CTL8421
	m_tms5100->ctl_w(space, 0, data & 0xf);
}


// EVA-11

WRITE16_MEMBER(eva11_state::write_r)
{
	// R7: TMS5100 PDC pin
	m_tms5100->pdc_w(data >> 7 & 1);
}

WRITE16_MEMBER(eva11_state::write_o)
{
	// O3210: TMS5100 CTL8421
	m_tms5100->ctl_w(space, 0, data & 0xf);
}

READ8_MEMBER(eva11_state::read_k)
{
	// K84: TMS5100 CTL81(O30)
	u8 ctl = m_tms5100->ctl_r(space, 0);
	ctl = bitswap<2>(ctl, 3,0) << 2;

	// TODO: sensors

	return ctl;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( eva24 )
INPUT_PORTS_END

static INPUT_PORTS_START( eva11 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void eva_base_state::eva_sound(machine_config &config)
{
	/* sound hardware */
	TMS6100(config, m_tms6100, 640_kHz_XTAL/4);

	SPEAKER(config, "mono").front_center();
	TMS5110A(config, m_tms5100, 640_kHz_XTAL);
	m_tms5100->m0().set(m_tms6100, FUNC(tms6100_device::m0_w));
	m_tms5100->m1().set(m_tms6100, FUNC(tms6100_device::m1_w));
	m_tms5100->addr().set(m_tms6100, FUNC(tms6100_device::add_w));
	m_tms5100->data().set(m_tms6100, FUNC(tms6100_device::data_line_r));
	m_tms5100->romclk().set(m_tms6100, FUNC(tms6100_device::clk_w));
	m_tms5100->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void eva24_state::eva(machine_config &config)
{
	/* basic machine hardware */
	COP420(config, m_maincpu, 640_kHz_XTAL/2); // guessed
	m_maincpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_maincpu->write_d().set(FUNC(eva24_state::write_d));
	m_maincpu->write_g().set(FUNC(eva24_state::write_g));
	m_maincpu->read_g().set(FUNC(eva24_state::read_g));

	eva_sound(config);
}

void eva11_state::eva(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 640_kHz_XTAL/2); // from TMS5110A CPU CK
	m_maincpu->k().set(FUNC(eva11_state::read_k));
	m_maincpu->o().set(FUNC(eva11_state::write_o));
	m_maincpu->r().set(FUNC(eva11_state::write_r));

	eva_sound(config);
}



/***************************************************************************

  ROM Defs, Game driver(s)

***************************************************************************/

ROM_START( eva24 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "4232345", 0x0000, 0x0400, CRC(0326c2fe) SHA1(c4c73badee68f682871b42ba4f5ca115cd68fb8a) )

	ROM_REGION( 0x4000, "tms6100", 0 )
	ROM_LOAD( "cm63002.vsm", 0x0000, 0x4000, CRC(cb63c807) SHA1(df7323eebcd2a8a5401c2e0addbbabb700182302) )
ROM_END

ROM_START( eva11 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "32045b", 0x0000, 0x0400, CRC(eea36ebe) SHA1(094755b60965654ddc3e57cbd69f4749abd3b526) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_eva11_micro.pla", 0, 867, CRC(38fcc108) SHA1(be265300e0828b6ac83832b3279985a43817bb64) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_eva11_output.pla", 0, 365, CRC(f0f36970) SHA1(a6ad1f5e804ac98e5e1a1d07466b3db3a8d6c256) )

	ROM_REGION( 0x1000, "tms6100", 0 )
	ROM_LOAD( "cm73002.vsm", 0x0000, 0x1000, CRC(d5340bf8) SHA1(81195e8f870275d39a1abe1c8e2a6afdfdb15725) )
ROM_END



//    YEAR  NAME   PARENT CMP MACHINE INPUT  CLASS        INIT        COMPANY     FULLNAME                                FLAGS
SYST( 1984, eva24, 0,      0, eva,    eva24, eva24_state, empty_init, "Chrysler", "Electronic Voice Alert (24-function)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
SYST( 1983, eva11, eva24,  0, eva,    eva11, eva11_state, empty_init, "Chrysler", "Electronic Voice Alert (11-function)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
