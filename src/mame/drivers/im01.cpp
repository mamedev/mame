// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Электроника ИМ-01 (Elektronika IM-01)

Soviet chess computer, produced by Svetana from 1986-1992.
IM-01T is the same hardware, the program has more difficulty levels.

Hardware notes:
- К1801ВМ1 CPU (PDP-11 derived)
- 16KB ROM (2*К1809РЕ1), 4KB RAM(К1809РУ1)
- K1809BB1 (I/O, counter)
- 4-digit 7seg panel, beeper

TODO:
- emulate К1801ВМ1, using T11 for now and I hope it works ok
- emulate K1809BB1
- inputs, 7segs, sound
- cpu frequency, irq frequency

******************************************************************************/

#include "emu.h"

#include "cpu/t11/t11.h"


namespace {

class im01_state : public driver_device
{
public:
	im01_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void im01(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<t11_device> m_maincpu;

	void main_map(address_map &map);

	u8 irq_callback(offs_t offset);
	INTERRUPT_GEN_MEMBER(interrupt);
};

void im01_state::machine_start()
{
}

void im01_state::machine_reset()
{
	m_maincpu->set_input_line(t11_device::VEC_LINE, ASSERT_LINE);
}



/******************************************************************************
    Interrupts
******************************************************************************/

u8 im01_state::irq_callback(offs_t offset)
{
	m_maincpu->set_input_line(t11_device::CP0_LINE, CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP1_LINE, CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP3_LINE, CLEAR_LINE);
	return 0;
}

INTERRUPT_GEN_MEMBER(im01_state::interrupt)
{
	// indirect interrupt vector at 0100
	m_maincpu->set_input_line(t11_device::CP0_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(t11_device::CP1_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(t11_device::CP3_LINE, ASSERT_LINE);
}



/******************************************************************************
    I/O
******************************************************************************/



/******************************************************************************
    Address Maps
******************************************************************************/

void im01_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x2000, 0x5fff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( im01 )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void im01_state::im01(machine_config &config)
{
	// basic machine hardware
	T11(config, m_maincpu, 5'000'000);
	m_maincpu->set_initial_mode(3 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &im01_state::main_map);
	m_maincpu->in_iack().set(FUNC(im01_state::irq_callback));
	m_maincpu->set_periodic_int(FUNC(im01_state::interrupt), attotime::from_hz(50));
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( im01 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("0000148", 0x2000, 0x2000, CRC(327c6055) SHA1(b90b3b1261d677eb93014ea9e809e45b3b25152a) )
	ROM_LOAD("0000149", 0x4000, 0x2000, CRC(43b14589) SHA1(b083b631f38a26a335226bc474669ef7f332f541) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME  PARENT CMP MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, im01, 0,      0, im01,    im01,  im01_state, empty_init, "Svetlana", "Elektronika IM-01", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
