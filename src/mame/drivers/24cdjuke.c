/*

 Midcoin 24 CD Coin-operated Jukebox

CPUs
QTY     Type            clock   position        function
1x      D780C-1                 ic1             8-bit Microprocessor - main
1x      MAB8441T-T042           ic10            8-bit Single Chip Microcontroller - main (internal ROM not dumped)
3x      P8255A-5                ic11,ic25,ic31  Programmable Peripheral Interface
1x      MM5450N                 ic29            LED Display Driver - main
2x      LM358                   ic64,ic65       Dual Operational Amplifier - sound
1x      oscillator      6.0MHz  Q1

ROMs
QTY     Type                    position        status
1x      D27128D                 ic5             dumped
2x      M1-7611A-5              ic27,ic28       dumped
2x      DM74LS471N              ic20,ic21       dumped

RAMs
QTY     Type                    position
1x      HY6116ALP-10            ic22
1x      D43256C-12L             ic6

Others

QTY     Type
1x      6 legs connector (ALIM - power)
1x      10 pins flat cable connector purple (ALIM DISP - power display)
1x      10 pins flat cable connector yellow (AMPLI - amplificator)
1x      10 pins flat cable connector green (MOTORE - engine)
1x      10 pins flat cable connector black (CD)
1x      10 pins flat cable connector blu (SERVICE)
1x      10 pins flat cable connector red (CONTROL MOT - engine control)
2x      jumper (J1 set, RIF unset) 5x trimmer (RP1,RP2,RP4,RP50,RP51)
2x      8x2 switches DIP (MD1,MD3)
1x      battery 3.6V
2x      red LED (5V,12V)
4x      red LED (CONTR. RAMPA - ramp control - L1,L2,L3,L4)
8x      red LED (HO, BR.D, P.D, D.I, BR.F, P.UP, ENC, MAB)
1x      red LED (5V)
1x      red LED (5V)
1x      red LED (5V)
1x      16 digits LED display (on solder side), each digit is made by 18 segments
Notes

This is the PCB for one of the first ever CD based Juke Box, made in 1988 by Midcoin, some info here:
http://www.tilt.it/deb/i-midcoin.html


*/

#include "emu.h"
#include "cpu/z80/z80.h"

class midcoin24cdjuke_state : public driver_device
{
public:
	midcoin24cdjuke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start();
	virtual void machine_reset();
};


static ADDRESS_MAP_START( midcoin24cdjuke_map, AS_PROGRAM, 8, midcoin24cdjuke_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( midcoin24cdjuke )
INPUT_PORTS_END


void midcoin24cdjuke_state::machine_start()
{
}

void midcoin24cdjuke_state::machine_reset()
{
}

static MACHINE_CONFIG_START( midcoin24cdjuke, midcoin24cdjuke_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,6000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(midcoin24cdjuke_map)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", midcoin24cdjuke_state,  irq0_line_hold)
MACHINE_CONFIG_END



ROM_START( 24cdjuke )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1.ic5", 0x0000, 0x4000,  CRC(df2419ad) SHA1(dd9dd85011d46581dccabcfdb5959a8b018df937)  )

	// MAB8441T-T042 internal ROM?

	ROM_REGION( 0x80000, "misc", 0 )
	ROM_LOAD( "dm74ls471n.ic20", 0x000, 0x100, CRC(d05765e6) SHA1(119ec6ca1a4afa0ea6ab1020ba2a8b02fd434e3f) )
	ROM_LOAD( "dm74ls471n.ic21", 0x000, 0x100, CRC(e12d5a04) SHA1(be52ee4e4a5ea225fce39c759645a7cf49cea370) )
	ROM_LOAD( "m1-7611a-5.ic27", 0x000, 0x100, CRC(29b068e8) SHA1(477e2445c58b7d14c56a3ad4050eb22474d56005) )
	ROM_LOAD( "m1-7611a-5.ic28", 0x000, 0x100, CRC(29b068e8) SHA1(477e2445c58b7d14c56a3ad4050eb22474d56005) )
ROM_END


GAME( 1988, 24cdjuke,  0,    midcoin24cdjuke, midcoin24cdjuke, driver_device,  0, ROT0, "Midcoin", "Midcoin Juke Box 24CD", GAME_NO_SOUND | GAME_NOT_WORKING ) // what name was it sold under? name is from the PCB text
