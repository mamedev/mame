// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

	Juku E5101

	Skeleton driver

	Hardware:
	- КР580ВМ80A
	- КР580ИР82
	- КР580ВА86
	- КР580ВА87
	- КР580ВИ53 x3
	- КР580ВК38
	- КР580ВН59
	- КР580ВВ51A x2
	- КР580ВВ55A x2

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class juku_state : public driver_device
{
public:
	juku_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic"),
		m_sio(*this, "sio%u", 0U)
	{ }

	void juku(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device_array<i8251_device, 2> m_sio;

	void mem_map(address_map &map);
	void io_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void juku_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xd000, 0xffff).ram();
}

void juku_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x04, 0x07).rw("pio0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_sio[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0c, 0x0f).rw("pio1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw("pit0", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x14, 0x17).rw("pit1", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x18, 0x1b).rw("pit2", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1c, 0x1f).rw(m_sio[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( juku )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void juku_state::machine_start()
{
}

void juku_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void juku_state::juku(machine_config &config)
{
	// КР580ВМ80A @ 2 MHz
	I8080A(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &juku_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &juku_state::io_map);

	// КР580ВН59
	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	// КР580ВИ53
	PIT8253(config, "pit0", 0);

	// КР580ВИ53
	PIT8253(config, "pit1", 0);

	// КР580ВИ53
	PIT8253(config, "pit2", 0);

	// КР580ВВ55A
	I8255A(config, "pio0");

	// КР580ВВ55A
	I8255A(config, "pio1");

	// КР580ВВ51A
	I8251(config, m_sio[0], 0);
	m_sio[0]->rxrdy_handler().set("pic", FUNC(pic8259_device::ir2_w));
	m_sio[0]->txrdy_handler().set("pic", FUNC(pic8259_device::ir3_w));

	// КР580ВВ51A
	I8251(config, m_sio[1], 0);
	m_sio[1]->rxrdy_handler().set("pic", FUNC(pic8259_device::ir0_w));
	m_sio[1]->txrdy_handler().set("pic", FUNC(pic8259_device::ir1_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( juku )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("jukurom0.bin", 0x0000, 0x2000, CRC(b26f5080) SHA1(db8bab6ff7143be890d6aaa25d10386dfdac3fc7))
	ROM_LOAD("jukurom1.bin", 0x2000, 0x2000, CRC(b184e253) SHA1(d169acde61f643d7d0780cca0eeaf33ebdf75b92))

	ROM_REGION(0x2000, "basic", 0)
	ROM_LOAD("bas0.bin", 0x0000, 0x0800, CRC(c03996cd) SHA1(3c45537c2a1879998e5315b79eb44dcf7c007d69))
	ROM_LOAD("bas1.bin", 0x0800, 0x0800, CRC(d8016869) SHA1(baef9e9c55171a9192bc13d48e3b45394c7780d9))
	ROM_LOAD("bas2.bin", 0x1000, 0x0800, CRC(9a958621) SHA1(08baca27e1ccdb0a441706df267c1f82b82d56ab))
	ROM_LOAD("bas3.bin", 0x1800, 0x0800, CRC(d4ffbf67) SHA1(bced7ff2420f630dbd4cd1c0c83481ed874869f1))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY   FULLNAME      FLAGS
COMP( 1988, juku, 0,      0,      juku,    juku,  juku_state, empty_init, "Estron", "Juku E5101", MACHINE_IS_SKELETON )
