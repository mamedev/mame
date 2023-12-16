// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Brother PN-8800FXB "Super PowerNote"

    Hardware:
    - HD64180RF6X CPU
    - TC551001BFL-70 (128k RAM)
    - 2x HY6264A (2x 8k, VRAM?)
    - HG62F33R32FH UC2836-A (gate array)
    - HD63266F FDC
    - RC224ATF (modem)
    - TC8521AM RTC
    - XTAL XT1 16.000312 MHz (near modem), XT2 12.2MHz (near CPU)
    - XTAL XT3 32.768kHz (near RTC), XT4 18.0MHz (near gate array)
    - XTAL XT5 16.0MHz (near FDC)

    TODO:
    - Everything

    Notes:
    - There is a serially connected daughterbord containing the Bookman logic

****************************************************************************/

#include "emu.h"

#include "cpu/z180/z180.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pn8800fxb_state : public driver_device
{
public:
	pn8800fxb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void pn8800fxb(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80180_device> m_maincpu;

	void mem_map(address_map &map);
	void io_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pn8800fxb_state::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).rom();
}

void pn8800fxb_state::io_map(address_map &map)
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( pn8800fxb )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pn8800fxb_state::machine_start()
{
}

void pn8800fxb_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void pn8800fxb_state::pn8800fxb(machine_config &config)
{
	Z80180(config, m_maincpu, 12'200'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pn8800fxb_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pn8800fxb_state::io_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pn8800 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("uc8254-a-pn88.5", 0x000000, 0x100000, CRC(d9601c1a) SHA1(1699714befeaf2fe17232c1b4f49d4242f5367f4))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY     FULLNAME      FLAGS
COMP( 1996, pn8800, 0,      0,      pn8800fxb, pn8800fxb, pn8800fxb_state, empty_init, "Brother",  "PN-8800FXB", MACHINE_IS_SKELETON )
