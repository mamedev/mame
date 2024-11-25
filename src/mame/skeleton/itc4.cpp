// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for temperature controller by Oxford Instruments Ltd.

PCB
  _______________________________________________________________
 |                                                              |
 |                           ___                 ___            |
 |             ___           |__|                |  |           |
 |            HP2200                 ________    |__|           |
 |                                   |_______|                  |
 |                    ___                                       |
 |                    |LF3478N      ________     ________       |
 |                    |__|          |DIPSx10|    |DIPSx10|      |
 | ::::::::::::                                                 |
 |_||||||||||||_________________________________________________|
   ||||||||||||
  _||||||||||||__________________________________________________
 | ::::::::::::                                     ___   ___   |
 |                                       74HC273P->|  |  |  |   |
 |  ___ <-B98-3-R100K                              |  |  |  |   |
 |  |  |         ____________   ____________       |__|  |__|   |
 |  |  |        |Z84C3006PEC|  | DS1220Y   |                    |
 |  |__|        |Z80_CTC____|  |___________|                    |
 |        ___                                                   |
 |        |  |    _________     ____________       ___________  |
 |        |  |   |_74HC02P|    | EPROM     |      | NM232C   |  |
 |74HC244P->_|                 |___________|      |__________|  |
 |        ___                                                   |
 |74HC273P-> |    _________     ________________   ____________ |
 | ..     |  |   |_74HC32P|    |Z84C0006PEC    |  |D8251AFC   | |
 | ..     |__|                 |Z80_CPU________|  |___________| |
 | ..                                                           |
 | ..     ___     _________                                     |
 |74HC137P-> |   |_74HC32P|       SWITCH                        |
 |        |  |                                ___               |
 |        |__|   Xtal            _________   |  |<-L8939H       |
 |               8.000 MHz      MC14526BCP   |__|               |
 |                _________      _________               ___    |
 |               |_74HC74P|     |_74HC74P|             LF353N   |
 |                                                              |
 |     :::::::::::::   ······                                 ::|
 |______________________________________________________________|

FRONT CONTROL PANEL (o=LED, SW=switch)
  _____________________________________________________________________________________________________________________
 | __________________________________ ____________ _____________________ _____________ ___________ _________ ________ |
 ||              DISPLAY            ||  SWEEP    ||       HEATER        ||  GAS FLOW  || CONTROL  || ADJUST ||       ||
 ||         ______________________  ||           ||       ____________  ||            ||          ||        ||       ||
 ||  o 1   |     __  __  __  __  |  || o HOLD    ||  o 1 |            | ||            || o LOCK   ||  RAISE ||       ||
 ||  o 2   |+ | |__||__||__||__| |  || o SWEEP   ||  o 2 | |||||||||| | ||            || o REMOTE ||   SW   ||       ||
 ||  o 3   |- | |__||__||__||__| |  || o PROGRAM ||  o 3 |   o    o   | ||            || o LOCAL  ||        ||       ||
 ||        |_____________________|  ||           ||      |____________| ||            ||          ||        ||       ||
 ||         CAL                     ||  RUN /    ||                     ||  o     o   ||          ||        ||   o   ||
 || SENSOR   o   PROP INT DERIV SET || PROGRAM   ||  SENSOR AUTO  MAN   || AUTO  MAN  || LOC/REM  ||  LOWER || POWER ||
 ||   SW   LIMIT  SW   SW   SW   SW ||   SW      ||    SW    SW    SW   ||  SW    SW  ||    SW    ||   SW   ||   SW  ||
 ||          o                      ||           ||                     ||            ||          ||        ||       ||
 ||_________________________________||___________||_____________________||____________||__________||________||_______||
 |____________________________________________________________________________________________________________________|

BACK PANEL
 -RS-232
 -AUXILIARY (DA15)
 -SENSOR 1 (DE9)

************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"


namespace {

class itc4_state : public driver_device
{
public:
	itc4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void itc4(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
};


void itc4_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
}

void itc4_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x78, 0x7b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xd8, 0xd9).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xb8, 0xbf).noprw(); // 7-segment display?
	map(0xe8, 0xe8).nopw();
}


static INPUT_PORTS_START(itc4)
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void itc4_state::itc4(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // Z84C0006PEC
	m_maincpu->set_addrmap(AS_PROGRAM, &itc4_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &itc4_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // DS1220Y

	z80ctc_device &ctc(Z80CTC(config, "ctc", 8_MHz_XTAL / 2)); // Z84C3006PEC
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<0>(8_MHz_XTAL / 4); // guess
	ctc.set_clk<1>(8_MHz_XTAL / 4); // guess
	ctc.set_clk<2>(8_MHz_XTAL / 4); // guess

	i8251_device &usart(I8251(config, "usart", 8_MHz_XTAL / 4)); // µPD8251AFC
	usart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	usart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	usart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("usart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("usart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("usart", FUNC(i8251_device::write_cts));
}

ROM_START(itc4)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("itc4_2.04_70fa.u2", 0x0800, 0x0800, CRC(f2d3f050) SHA1(77947dd9584ffec22c67940582efbeb3e9553e07)) // M27128
	ROM_CONTINUE(0x0000, 0x0800) // "(c) OXFORD 1988"
	ROM_CONTINUE(0x1800, 0x0800)
	ROM_CONTINUE(0x1000, 0x0800)
	ROM_CONTINUE(0x2800, 0x0800)
	ROM_CONTINUE(0x2000, 0x0800)
	ROM_CONTINUE(0x3800, 0x0800)
	ROM_CONTINUE(0x3000, 0x0800)
ROM_END

} // anonymous namespace


SYST(1988, itc4, 0, 0, itc4, itc4, itc4_state, empty_init, "Oxford Instruments", "ITC-4 Intelligent Temperature Controller (Version 2.04)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
