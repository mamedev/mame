// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
Known Games running in this hardware:
Soccer 2002
Soccer 2004
Soccer 2006
Soccer 2010
SuperV 2010

No progression until the internal rom of main cpu is dumped

Soccer 2004
Main board
PCB Labeled M991205-A
Display controller
c1 Altera epm7064lc84-7
c2 tms 27c512-20 - eeprom
c3 hm6116lp-2 - video ram
c4 p8051ah fujitsu - microcontroller - display controller

main controller
C5 W78E52B-40 - with internal rom 8kb. - Protected
c6 tms 27c512-2jl eeprom
c7 hm6116lp-3
c20 um3567
??? DAC0800CN
c21 Altera epm7128elc84-15
c22 M27C801

Led Board
Labeled CS111P076 At front back
 
4x 16x16 led display matrix scroll.
5 buttons 
1x dip switch 8
??? p8255a
??? File KC8279P
*/

#include "emu.h"

#include "cpu/mcs51/i80c51.h"
#include "cpu/mcs51/i80c52.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "speaker.h"

namespace {

class my6_state : public driver_device
{
public:
	my6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	
	{ }

	void my6(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void display_map(address_map &map) ATTR_COLD;
	void display_data_map(address_map &map) ATTR_COLD;
};

static INPUT_PORTS_START( socc2004 )
INPUT_PORTS_END

void my6_state::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0xffff).rom().region("eeprom",  0x2000); 
}

void my6_state::data_map(address_map &map)
{
}

void my6_state::display_map(address_map &map)
{
	map(0x0000, 0x7FFF).rom(); // Has two program rom.
}

void my6_state::display_data_map(address_map &map)
{
	map(0xE000, 0xE7FF).ram().share("vram"); //Video ram. 64x64
    map(0xA000, 0xA000).noprw(); // Input for display controller handled by maincpu
}

void my6_state::machine_start()
{
}

void my6_state::my6(machine_config &config)
{
    // basic machine hardware
	i8052_device &maincpu(I8052(config, "maincpu", XTAL(10'738'635)));
	maincpu.set_addrmap(AS_PROGRAM, &my6_state::program_map);
	maincpu.set_addrmap(AS_DATA, &my6_state::data_map);
    maincpu.set_disable(); // Disabled for now.
	
	/* Keyboard & display interface */
	I8279(config, "i8279", XTAL(10'738'635) / 6); // Divisor not verified
    
	// Programmable Peripheral Interface
	I8255A(config, "ppi1");
	
	// Display Controller
    i8051_device &display(I8051(config, "display", XTAL(10'738'635)));
    display.set_addrmap(AS_PROGRAM, &my6_state::display_map);
	display.set_addrmap(AS_DATA, &my6_state::display_data_map);

   	// sound hardware
	SPEAKER(config, "mono").front_center();
	
	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);
	
	DAC0800(config, "snd").add_route(ALL_OUTPUTS, "mono", 1.0);

}

ROM_START( socc2004 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "W78E52B.c5", 0x00000, 0x2000,  NO_DUMP ) // Protected. Contains internal rom code.
	
	ROM_REGION( 0x10000, "eeprom", 0 )
	ROM_LOAD( "2_tms2c5122jl.c6", 0x00000, 0x10000,  CRC(05EF99CD) SHA1(591c51ced0acc3231c9629a060f9c42a2db9fbe0) ) // Sticker labeled 2. Hex FF filled at 0x0000-0x1fff.
   
    ROM_REGION( 0x10000, "display", 0 )
    ROM_LOAD( "tms27c512_2jl.c2", 0x0000, 0x10000,   CRC(3FBD0A4A) SHA1(d2b5d09d1f4209411ca884c9fa4a73276846c780) ) //  32kb Two Program Rom code. Soccer 2004 and Soccer 2002 bpp1 gfx.
   
    ROM_REGION( 0x100000, "snd", 0 )
    ROM_LOAD( "m27c801.bin", 0x00000, 0x100000,  CRC(BBF4A74C) SHA1(662aaaea0df23c14c2b802d117a342a9bdf13845) ) // Unsigned 8-bit pcm.
ROM_END

} // anonymous namespace


//    YEAR  NAME         PARENT   MACHINE   INPUT       STATE        INIT          ROT      COMPANY                        FULLNAME              FLAGS
GAME( 2004?, socc2004,   0,       my6,      socc2004,   my6_state,   empty_init,   ROT0,   "Ming-Yang Electronic / TSK",   "Soccer 2004",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND   )
