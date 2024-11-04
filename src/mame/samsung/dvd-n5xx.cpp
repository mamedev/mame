// license:BSD-3-Clause
// copyright-holders:

/*************************************************************************
NUON Enhanced DVD Player / Samsung DVD-N501
Hardware info by Guru

Brief Overview (summarized from info found on the internet)
--------------
NUON was a co-processor (i.e. Integrated Circuit) that was released around 2000 by VM Labs, Inc. It was initially announced in 1999 as 'Project X'.
It was added to a very limited number of DVD players so they could play special DVD discs with interactive content (usually games).
These players have features like most other DVD players and can also play VCD, CD Audio discs and MP3 audio.
There is a built-in graphical interface with a 'Light Show' known as the "Virtual Light Machine 2" written by Jeff Minter of Llamasoft fame which
shows graphical effects when CDs and MP3s are played. This is very similar to the VLM on the Atari Jaguar.
The NUON players have the ability to smooth fast forward and fast rewind and zoom/pan DVD Video discs.
The technology was abandoned after a few years due to poor sales because it was competing (and losing) against the Sony Playstation 2
and other much more powerful and well supported video game consoles that had a lot more game titles available.
Additionally VM Labs went bankrupt in 2002 so there was zero support for it after that point.

Externally the units look like any other DVD player but have a NUON logo on the front.
There were several models of NUON DVD players manufactured with Samsung being the most common with the most models released....
Motorola Blackbird (only for developers)
Motorola Streamaster 5000 (set top box)
Raite Optoelectronics RDP-741
RCA (Thomson multimedia) DRC300N
RCA (Thomson multimedia) DRC480N
Samsung - DVD-N2000 (Extiva)
          DVD-N501  Service manual available with technical info, block diagrams and schematics. Search 'dvdn501.pdf service manual'.
          DVD-N504  Asia/Europe only
          DVD-N505  Asia/Europe only
          DVD-N591  Korea only
Toshiba - SD2300
There were also two models announced but not released: Samsung DVD-N705, Oritron DVD900.

Some of the players came bundled with a game controller and a few 3rd-party controllers were produced....
Warrior Digital Gamepad HPI-2000 - Bundled with Samsung DVD-N2000. Digital only with 10 buttons and a D-pad.
Stealth Controller (HPI)         - Resembles the N64 controller and has digital buttons/D-pad and an analog stick.
Logitech Gamepad for NUON        - Digital buttons/D-pad and an analog stick.
Samsung NUON controller          - Digital only with 10 buttons and a D-pad.


NUON-Specific Discs
-------------------

Games Released                        NUON-Enhanced Movies Released
--------------                        -----------------------------
Tempest 3000 (dumped)                 The Adventures of Buckaroo Banzai Across the 8th Dimension
Freefall 3050 A.D. (dumped)           Bedazzled (2000 remake)
Merlin Racing (dumped)                Dr. Dolittle 2
Space Invaders X.L. (dumped)          Planet of the Apes (2001 remake)
Iron Soldier 3 (dumped)
Ballistic (dumped)                    Samplers/Demos Released
The Next Tetris (dumped)              -----------------------
Crayon Shin-chan 3 (dumped)           Interactive Sampler (three different versions)
                                      Nuon Games + Demos (collection from Nuon-Dome)
                                      Nuon-Dome PhillyClassic 5 Demo Disc (give-away collection)
                                      Motorola BlackBird Demonstration Pack

There were also a number of homebrew titles made available.
It was noted that only the Samsung and RCA units can run homebrew software and the Samsung DVD-N501 is the most compatible unit
although news on May 29th 2022 states that the authentication keys were discovered so that's no longer a limitation and now all other
Nuon models should be able to run homebrew titles.
More NUON information can be found at https://www.nuon-dome.com


Assembled Unit Layout (for Samsung DVD-N501)
---------------------

Rear Plate: Samsung Digital Video Disc Player DVD-N501 XAA 2001.05
Front Identification: Samsung NUON Enhanced DVD Player / DVD-N501

                               (ANALOG   )   (DIGITAL  )
  |-------- VIDEO OUT--------| (AUDIO OUT)   (AUDIO OUT)
  Y Pr Pb    SVIDEO   COMPOSITE  L     R      COAX  OPTICAL
|-||-||-||-----||-------||-------||----||------||---||-----------------------|
| |                                                  |                       |
| |                                                  |  |------------------| |
| |                      |------------------------|  |  |                  | |
| |                      |                        |  |  |                  | |
| |   DVD EXTIVA JACK    |                        |  |  |                  | |
| | AH41-00322A REV:01   |                        |  |  |                  | |
| |                      |                        |  |  |                  | |
| |    (connectors       |     DVD DRIVE UNIT     |  |  |    DVD EXTIVA    | |
| |   and power supply)  |       ASSEMBLY         |  |  |     MAIN B/D     | |
| |                      |       (ABOVE)          |  |  |                  | |
| |                      |                        |  |  |                  | |
| |                      |                        |  |  |                  | |
| |                      |                        |  |  |                  | |
| |                      |                        |  |  |                  | |
| |                      |                        |  |  |                  | |
| |----------------------|------------------------|--|  |                  | |
| |---------------|      |                        |     |                  | |
| |PWR+LED+JOY BD |      |------------------------|     |------------------| |
| |---------------|          |---TRAY FACE-----|                             |
|--||-----||-||--------------|-----------------|-||---||---||---||----||--||-|
 POWER    JOYSTICK            VF DISPLAY       OPEN   FIT  PLAY STOP BACK FWD
 BUTTON   CONNECTORS              (BELOW)      CLOSE       PAUSE


PCB Info
--------

Power Board:
DVD EXTIVA JACK AH41-00322A REV:01

The JACK PCB contains these main parts....
All of the external jacks
Switch-Mode Power Supply
Vacuum-Fluorescent Display
NEC uPD78F0233 Micro-controller with 24kB internal flash ROM and 912 bytes internal RAM; 768b + 32b + 112b VFD RAM. Clock input 5.000MHz.
  - Labelled on PCB as FIC1
  - Stock NEC part pre-programmed at the factory as a VFD driver.
  - Processes the infrared signal from the remote.
  - Processes the front panel button presses.
    Note this IC very common and used in many models of DVD players.
5.000MHz Oscillator for the uPD78F0233
ROHM BA4560 Dual Operational Amplifier
ASAHI KASEI AK4382A 24-bit 2-channel DAC
Mitsumi MM1540A Video Output Driver which provides S-Video, Y Pb Pr Component Video and Composite Video. Labelled on PCB as VIC1


Main Board:
CODE NO : AH41-00324A REV. 00
DVD EXTIVA MAIN B/D 2001.03.27
TVE1-1E/0030CY (sticker)
|-------------------------------------------------------------|
|  |------|    MIC2          20MHz             33.8688MHz     |
|  | NIC7 |                |------|                        SW7|
|  |      |                | MIC1 |           |--------| SIC2 |
|  |------|         NIC2   |      |           |        |      |
|                          |------|           | SIC1   |   SW6|
|        |--------|                           |        |      |
|  NIC12 |  NIC1  |                           |--------|   SW5|
|        |        | NIC13                                     |
|        |--------|                                        SW4|
|                                                   |-----|   |
|       108MHz  |----|                     |-----|  |SIC3 |   |
|          NIC5 |VIC1|                     |RIC1 |  |-----|SW3|
|  NIC11        |----|CN9                  |-----|   DCN2     |
|           CN8        PCN1               DCN1        DCN3 SW2|
|-------------------------------------------------------------|
Notes:
      All parts have xICy location on the PCB.
      y is the IC# and x is for the IC function group.
      M for Main processor related parts
      N for NUON related parts
      S for Digital Servo/DSP related parts
      R for Laser pick-up / RF related parts
      V for Video related parts
      F for front panel related parts (see JACK PCB above)

      MIC1 - Toshiba TMP91C219F TLCS-900/L1-Series 16-bit Micro-controller. Clock input 20.000MHz
             Has internal 2kB RAM, no main ROM and 2kB boot ROM. A vanilla boot program may have been provided by default in all chips
             produced but it is unknown if the internal boot ROM has been programmed or is used/not used.
             On the PCB the BOOT pin is tied HIGH so the chip is set to run in Multi-Chip Mode. The datasheet states....
             "Multi-Chip Mode: After a reset, the device starts executing the external memory program."
             When BOOT is tied low the datasheet states....
             "Multi-Boot Mode: This mode is used to rewrite the external flash memory by serial transfer (UART) or ATAPI transfer.
             After a reset, internal boot program starts up, executing an on-board rewrite program."
             This suggests the boot ROM is only used for re-writing the flash ROM and possibly only used for factory programming or
             not used at all when using a common external EPROM.
             Since BOOT is hardwired high the boot ROM (if programmed) is not used when the DVD player operates normally and is therefore
             not required for emulation purposes.
             The Memory Map changes slightly depending on how the BOOT pin is set but basically they are identical except for the boot
             ROM memory location.

                                Multi-Chip Mode           Multi-Boot Mode
                                ---------------           ---------------
             000000h-000fffh    Internal I/O (4kB)        Same
             001000h-0017ffh    Internal RAM (2kB)        Same
             001800h-01f7ffh    External Memory           Same
             01f800h-01ffffh    Boot ROM (2kB)            External Memory
             020000h-fff7ffh    External Memory           Same
             fff800h-fffeffh    External Memory           Boot ROM (2kB)
             ffff00h-ffffffh    Vector Table (256 bytes)  Same

      MIC2 - AMIC A290021T 256kBx8-bit DIP32 Flash ROM (compatible with 29F020/28F020/27C020 etc). Dumped as DVD-N501_XAA_VER1.2.MIC2
      NIC1 - "NUON (M) XCMMP-L3CZP 0K93E ZKAA0102 TAIWAN" in BGA272 package. (M)=Motorola logo
             This is the NUON graphics co-processor. The specific revision in the Samsung DVD-N501 is ARIES 2.1 according to the schematics.
             Clock input 108.000MHz on pin 114 X-PLL_CLKI. The chip is noted on the internet as running internally at 54MHz [108/2]
             It was stated that a tiny boot-loader inside the chip loads part of the TSOP48 flash ROM into RAM then jumps to it.
NIC2/NIC12 - Samsung K4S641632D-TC80 1Mx16-bit x4 Banks (64Mbit) SDRAM
     NIC11 - Atmel AT49BV1614 2MBx8-bit (16Mbit) TSOP48 Flash ROM. Dumped as DVD-N501_XAA_VER1.2.NIC11
     NIC13 - Burr-Brown PLL1700E Multi-Clock Generator Phase Lock Loop (PLL). Master clock input 27.000MHz [108/4] is generated
             by NIC1 on pin 237 X_VCLK. Pin 2 is tied LOW so the chip is operated in 3-wire software mode.
      NIC5 - Samsung KS24C021 256byte x8-bit (2Kbit) Serial EEPROM. Dumped as DVD-N501_XAA_VER1.2.NIC5
             Data appears to be country/region, language and other audio/video settings. The default parental lock code 9999 is present at byte 0x44h.
      NIC7 - Samsung KS999F. Unknown purpose but appears to be a PLCC44 CPLD. Also marked 'ORBIT 61744 0117 U46373.3'
             The block diagram shows this chip as 'Sally' which is likely the code name of the same chip used on the Samsung DVD-N2000 Main PCB.
      VIC1 - Philips SAA7128H Digital Video Encoder. This chip is very common and used in many models of DVD players.
      RIC1 - Samsung S5L1462A01-Q0 KA1462X-Series RF Signal Processor. This receives the optical signal from the laser pick-up
             and produces the data-generating RF signal, the servo error signal for stable servo control and the monitor signal.
             This chip is very common and used in many models of DVD players.
      SIC1 - Samsung S5L1454A01-Q0 KA1454X-Series Digital Video Disc Player/Decoder, Digital Servo Controller and Digital Signal Processor (DSP)
             Clock input 33.8688MHz on pin 95. The datasheet block diagram states there is internal ROM and SRAM.
             The SRAM stores the digital servo internal data. The amount of internal RAM and ROM is unknown.
             This chip is very common and used in many models of DVD players.
      SIC2 - EliteMT M11B416256A-35J 256kB x16-bit (4Mbit) 72MHz EDO Page Mode DRAM
      SIC3 - Fairchild KA3017 Spindle + 4-Channel Motor Driver for tracking actuator, focus actuator, sled servo motor, tray motor
             and 3-phase BLDC spindle motor.
       CN8 - 35-pin connector joining to the JACK PCB (Digital Audio and Video Output Signals)
       CN9 - 7-pin connector joining to front panel joystick PCB
      DCN1 - 35-pin connector joining to DVD drive (Data from DVD & Motor/Servo Control Signals)
      DCN2 - 5-pin connector joining to DVD drive (Tray Motor)
      DCN3 - 3-pin connector for outputs from the front panel buttons joining to CN2 on the JACK PCB. These connect to the uPD78F0233 MCU.
      PCN1 - 10-pin power input connector joining to the JACK PCB
       SW2 - Push button for DVD tray Open/Close
       SW3 - Push button for Play/Pause                                \
       SW4 - Push button labeled 'FIT'. Stretches video to fill screen | SW# taken from schematic as all buttons
       SW5 - Push button for Stop                                      | on the PCB are labelled only 'SW3'
       SW6 - Push button for skipping chapters/tracks backward         |
       SW7 - Push button for skipping chapters/tracks forward          /

Note the same basic Samsung DVD chipset was used in many DVD players. For example Hitachi DV-P415U, which appears to be a near identical
unit with identical chips and near identical VFD (minus 'MP3') compared to the Samsung DVD-N501 but without the NUON features.

*************************************************************************/


#include "emu.h"

#include "cpu/nuon/nuon.h"
#include "cpu/tlcs900/tmp95c061.h"
#include "cpu/upd78k/upd78k0.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class n5xx_state : public driver_device
{
public:
	n5xx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void n501(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

uint32_t n5xx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void n5xx_state::main_map(address_map &map)
{
	map(0xfc0000, 0xffffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( n501 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void n5xx_state::n501(machine_config &config)
{
	// basic machine hardware
	TMP95C061(config, m_maincpu, 20_MHz_XTAL); // actually TMP91C219F
	m_maincpu->set_addrmap(AS_PROGRAM, &n5xx_state::main_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(n5xx_state::screen_update));

	// NUON(config, "nuon", 108_MHz_XTAL); // ARIES 2.1, internally divided by 2

	UPD78053(config, "upd", 5_MHz_XTAL); // actually uPD78F0233

	// MM1540A

	// SAA7128H

	// S5L1462A01-Q0

	// S5L1454A01-Q0

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// AK4382A

	SOFTWARE_LIST(config, "dvd_list").set_original("nuon");
}


ROM_START( n501 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "dvd-n501_xaa_ver1.2.mic2", 0x00000, 0x40000, CRC(7308046b) SHA1(c33a77aa12908a9506495a67488349f536eee0b9) )

	ROM_REGION( 0x200000, "nuon", 0 )
	ROM_LOAD( "dvd-n501_xaa_ver1.2.nic11", 0x000000, 0x200000, CRC(0285411e) SHA1(cd8c1c210c3c15a2d88e508792c5fa58257aee43) )

	ROM_REGION( 0x6000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "upd78f0233.flash", 0x0000, 0x6000, NO_DUMP ) // undumped internal flash ROM

	ROM_REGION( 0x100, "ks24c021", 0 ) // serial EEPROM
	ROM_LOAD( "dvd-n501_xaa_ver1.2.nic5",  0x000, 0x100, CRC(1afb3e8f) SHA1(7284b1fb1394842edb97fe2decaf0e71a0643b35) )
ROM_END

} // anonymous namespace


SYST( 2001, n501, 0, 0, n501, n501, n5xx_state, empty_init, "Samsung", "Samsung NUON Enhanced DVD Player / DVD-N501", MACHINE_IS_SKELETON )
