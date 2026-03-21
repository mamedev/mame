// license:BSD-3-Clause
// copyright-holders:

/****************************************************************************
Namco System 147/148, Namco 2007-2014
Hardware info by Guru
Last update: 22nd May 2024
-------------------------------------

Namco System 147/148 is a small (150mm x 130mm) low-end arcade gaming platform that was manufactured
around 2007-2014 but the technology used is from 2005. It is essentially a heavily cost-reduced
Playstation 2 Slim running games from on-board NAND ROM storage. The main differences between 147
and 148 is the type and memory size of RDRAM used and the 16-bit buffers and video DAC on 148 are a
custom Sony QFP chip on 147. A later 148(B) revision board goes back to using the custom video DAC.
The 147/148 boards would be code-compatible between the different games (the chipset is identical)
but the protection chips are game-specific and all the ROMs are surface-mounted so games can't be
easily swapped. Of course if the ROMs and protection chip are swapped onto another identical board
the game should run on that board. Later games on System148 probably won't run on a System147 board
due to it having less RDRAM.

There are at least 17 games on this system but most are unknown. Each board has a tag tied to it
with the factory-installed software revision. The tag is equivalent to the Namco game code sticker
on the top or bottom of previous Namco PCBs so the tag must be documented. The 'keycus' chip is an
off-the-shelf CPLD programmed specifically for that game and is printed with a KPxxx number.
Unfortunately there seems to be a lot of junk games on this system (coin pushers etc) and they have
been factory converted to other games so the KP number may or may not be the original game. Many
have a sticker on top with a different number but it looks like some have just been re-programmed
and the old sticker was not replaced for the new game or the sticker was removed or fell off so
there are different games with the same KP number or the same game with a different KP number! Some
of the popular games on System147 were updated to run on the later System148 or 148(B) board.
Additionally PCBs can be updated with a USB stick and the software version is only available in the
test mode of a working board. A sticker with the updated version was provided with the USB update
but that relies on the person applying the sticker to the tag and it appears that wasn't always
done. For games like Animal Kaiser there were more than a dozen updates and versions although most
were USB upgrades. There are 3 known PCB releases of Animal Kaiser with updated software versions
and different KP numbers too. Animal Kaiser Evo8 is the last software version. There was an Evo8DX
released in an attempt to trick players into thinking it was an updated game but it was actually
just a top banner and cabinet sticker change.... the game was just Evo8. A dump of Animal Kaiser
Evo8 has proven the software contains all versions of the game and they can be selected by hacking
the ROM. Also remember this board is just a cost-reduced PS2 so there's going to be some lazy ports
of standard PS2 games as well. For example Pacman's Arcade Party is basically PS2 Namco Museum 50th
Anniversary but with a different game selection menu. The dump contains text references to a PS2
memory card and other PS2-specific terms.

These boards seem to just die for no reason so they probably have deeper design-related faults and
probably a lot of that is due to the poor cooling solution consisting of a thermal pad and a small
45mm 5V fan similar to the type of cheap Chinese fan found on very low-end PC video cards from the
early 2000's. They are designed to last a specific period of time, probably ~3 years. Another issue
is the fan outlet is positioned so it blows dirt all over the custom IOP BGA chip and all other parts
nearby. After some years that dust and dirt attracts moisture and salt from the air (many arcades
are in coastal locations) then everything corrodes in that area then it's permanently game over. The
BGA should have been sealed against dirt and moisture getting into it and corroding everything or a
conformal coating should have been applied to the board. Of course Namco *wants* the game to die and
be scrapped so they make more money selling another game but this has also had a negative effect on
the manufacturer reputation whereby people no longer trust the company to produce quality long-lasting
and reliable products. Manufacturers have learned from the past mistakes of building quality products
and now they don't want any game to last longer than 3 years.

If you own one of these boards and it's still working you can do the following to prolong the life
of the game...
- Upgrade to a bigger heatsink and better quality fan.
- Always keep the board very clean otherwise it will corrode and die.
- Make a shield using paper or plastic to prevent dust from the fan getting onto the board.
- Put heatsinks or thermal pads on all hot chips.

List of System 147/148 Games (preliminary)
----------------------------

System   Game                                                 Keycus   Tag            Notes
------------------------------------------------------------------------------------------------------------------------------------------
147?     Unknown                                              KP001A                  Vanilla SYSTEM147 may not exist
147(B)   Animal Kaiser first version?                         KP002A   ANK1001-NA-A   Factory re-programmed with sticker KP005A. Previous tag code ANK unknown game but seems like it would be the first version of Animal Kaiser.
147(B)?  Unknown                                              KP003A
147(B)   Umimonogatari Lucky Marine Theater (first release?)  KP004A   ULM1001-CT-C   \ One Sea Story cab is 4 or 8 boards networked together so
147(B)   Umimonogatari Lucky Marine Theater (first release?)  KP004A   ULM1001-RF-A   / these probably came from the same machine
*147(B)  Animal Kaiser - The King of Animal (version 1)       KP005A   ANA1004-NA-B   Available in base 1-6, EVO 1-8 & 8DX. Most are upgrades via USB. This PCB has KP002A security chip with sticker KP005A on top.
147(B)   Pac-Man's Arcade Party (Home Cocktail)               KP006A
147(B)   Pac-Man's Arcade Party (Home Cocktail)               KP006B                  PCB with KP006B and Cocktail sticker seen (photo archived)
147(B)   Pac-Man Battle Royale                                KP006B   PBR1002-NA-A   This version printed in manual
*147(B)  Pac-Man Battle Royale                                KP006B   PBR1022-NA-A
*147(B)  Pac-Man's Arcade Party (Arcade Upright)              KP007A   PMAAM12-NA-A   -C also seen. Arcade version with coin mech. Tag: "UPDATED SOFTWARE VERSION C 2/11/11"
148(B)   Pac-Man's Arcade Party (Home Cocktail)               KP007B   PMACS12-NA-C   No coin mech. Set to freeplay only. Software updated to run on System148(B) board.
*148     Umimonogatari Lucky Marine Theater (Sea Story)       KP008A   ULS1001-ST-A   Shows "St.No[1] [ST-B01]" on the network connect screen.
148      Umimonogatari Lucky Marine Theater (Sea Story)       KP008A   ULS1001-RF-A
148      Umimonogatari Lucky Marine Theater (Sea Story)       KP008B   ULD1001-RF-A   \
148      Umimonogatari Lucky Marine Theater (Sea Story)       KP008B   ULD1001-ST-A   / These probably from the same machine
148(B)?  Unknown                                              KP009A
148(B)?  Unknown                                              KP010A
147(B)   Pac-Man Battle Royale Tournament Edition             KP011B   PBR1032-NA-A   Factory-updated version on the original KP006B PCB with KEYCUS re-programmed and sticker KP011B applied. s/w rev "PBR103-2-NA-MPR0-A24"
*147(B)  Pac-Man Battle Royale (unknown edition)              KP011B   PBR101-1-NA-A  s/w rev "PBR101-1-NA-MPRO-A23"
*148     Animal Kaiser - The King of Animal (Evo 1-8)         KP012B   ANA2004-NA-A   This PCB was updated to Evo8 with a 8DX cab update too. The update keys are held in battery-backed RAM so when the battery dies it reverts back to Evo1.
148(B)?  Unknown                                              KP013A
148(B)?  Unknown                                              KP014A
147(B)   Bai Shou Da Zhan (Animal Kaiser, China version)      KP014B   ANC1005-NA-A   Found on a Chinese online site. Seller says 'Battle Of Beasts'. Probably the Chinese version of "Animal Kaiser - The King of Animal (version 1)".
148(B)?  Unknown                                              KP015A
148(B)   Yokai Watch (or similar, full title unknown)         KP016B   YWD1001-KM-A   Redemption machine. PCB with KP016B sticker seen (photo archived)
148(B)   Animal Kaiser - The King of Animal (Evo 7)           KP017B   ANA2004-NA-A   This was likely USB updated to EVO 7. Has sticker KP017B and no code printed on security chip. Basically the same software as KP012B board but on newest S148B board. Hardware differences vs S148 are very minimal.
148(B)   Animal Kaiser - The King of Animal (Evo 1)           KP017B   ANA2004-NA-A   Another board with sticker KP017B and without a code printed on the security chip. Software revision in test mode shows EVO 01.00 and ROM version "ANA200-4-NA-MPRO-A22 17/06/2011 (FRI) 13:10:03"
         OnePy Berrie Match (not confirmed)
         Medal Master (not confirmed)

NOTES:
      * = Game is dumped.
      + = Secured but not dumped yet.
      Keycus numbers with unknown games may not exist.
      There might be more games, possibly only released in Japan.
      Pacman's Arcade Party Arcade Version includes the following games....
        Pacman, Galaga, Galaxian, Xevious, Bosconian, Dragon Spirit, Rolling Thunder, Dig Dug, Pacmania, Mappy, Rally X, Galaga 88.
      Pacman's Arcade Party Home Version included all the above games with the addition of Ms. Pacman.
      Pac-Man Battle Royale CHOMPionship is confirmed to be PC-based (i.e. not S147/148).
      The Sea Story board configuration is unknown. The cabinet is square with two players on each side so it might consist of 8 System 147/148 boards and they are definitely networked together.


PCB Layout
----------

System147 MAIN PCB (possibly made by Namco before merging with Bandai?)
Not seen. Possibly used on the very first game only?
KP002A is confirmed System 147(B) so it can only be used with game KP001A or this board doesn't exist

2007 NAMCO BANDAI Games Inc.
System147 MAIN(B) PCB   \ same PCB as below but using K4R271669F
8916960300 (8916970300) / RDRAM and custom SC44750PB SCEI QFP64 video DAC

2014 NAMCO BANDAI Games Inc.
System148 Main(B) PCB   \ almost identical to 2010 PCB below with custom SC44750PB SCEI QFP64
8917960203 (8917970203) / video DAC and some different power-related parts. Uses same BIOS as 2010 PCB.
Sticker: 8917960300

2010 NAMCO BANDAI Games Inc.
System148 MAIN PCB                     PSU Area
8917960102 (8917970102)             |----------\
|-------------------------------------------|   \
|CN9                                        |   |
| CY22313                          SI4340DY |   |
| 18.432MHz       K4R881869E       *MB39A106|   |
|SCY99009         K4R881869E                |   |
|  IS41LV16100                     SI4340DY |   |
| *IS41LV16100 |---------------|            |   |
|16.9344MHz    |               |            |   |
|   |--------| |    EE + GS    |    00BC0W  |   |
|   |IOP     | |               |            |   |
|   |CXD9799 | |   CXD9833GB   |      BD9781|   |
|   |AGP     | |               |LM70        |   |
|   |--------| |               |*LC07       |   |
|*VHC00        |               |            |   |
|*7673     CN10|---------------|    C807U   |   |
|3V_BATT *R4543|------| LED1 LED2   *4MHz   |   |
|  *62256      |CXD294| EPM3128       *706R |   |
|CN8           |7R    | *NAND.IC31    *1537D|   |
|    IC1       |------|                  *F1|   |
|*245            40MHz         16245        | --|
|CN7   TMC2074         CS4335  16245    LED3|
|*PSW1 *PSW2 *485 SW2          ADV7125  CN6 |
|*3222 CN1 CN2    CN3    CN4      CN5       |
|-------------------------------------------|
Notes: (All IC's shown. * denotes these parts are on bottom side of PCB)
    CY22313 - Cypress CY22313ZXC Two-PLL Clock Generator with Direct Rambus Support.
              Measured clocks: pin 4 XIN 18.4323MHz, pin 9 54MOUT 53.9997MHz, pin 14 LCLK 9.21596MHz, pin 20/21 CLKB/CLK 393.214MHz
   SCY99009 - ?. Seems to be used to generate 2.5V and 1.8V.
              On System148(B) this IC is replaced with...
              2x Mitsumi MM1662F HSOP-8 1A 2.5V Voltage Regulator
              1x Mitsumi MM1665X HSOP-8 1A 1.8V Voltage Regulator
              2x SOT753 1.8V Voltage Regulator marked "F35285"
IS41LV16100 - ISSI IS41LV16100 1M x16-bit DRAM with EDO Page Mode
     CS4335 - Cirrus Logic CS4335KSZ 24-bit, 96kHz Stereo Audio D/A Converter
    3V_BATT - 3V Coin Battery. If board has been updated, removing the battery reverts to the original version software.
              Note updates are actually *applied* with a special Barcode Update Card. Without the Update Card, the update
              is not applied even if a USB update has been done. This is of course just another DRM method to lock out
              and maintain control over the user.
        CN1 - USB-A Connector. Not connected to anything for game operation so probably only used for software updates.
              The software updates for Animal Kaiser are supplied on a 16GB USB stick that plugs into this USB connector.
        CN2 - 4 pin connector. This is another USB port but using a 4 pin JST-XH connector
        CN3 - 3 pin JST-XH connector for RS485 connection to I/O board. Pin 1 - PINK, Pin 2 - WHITE, Pin 3 - BLACK
              Pin 1 tied to RS485 IC pin 6 ('A' Driver Output / Receiver Input; Non-Inverting)
              Pin 2 tied to RS485 IC pin 7 ('B' Driver Output / Receiver Input; Inverting)
              Pin 3 tied to Ground
        CN4 - 3.5mm Stereo Audio Output Jack. This is the main audio output. The output is not amplified and a separate
              amplifier board is present in the cab to drive the cabinet speakers. Powered PC speakers also work fine.
        CN5 - DB15HD 15 pin VGA connector. Standard VGA output at 31.5kHz
        CN6 - 4 pin JST-PA connector for 12V power input. Pins 1 & 2: 12V, Pins 3 & 4: GND. All other voltages are generated on the main board.
              This also means if any of the on-board power supplies fail the game doesn't work. None of the chips require 12V so
              this is just another way to apply planned obsolescence when something power-related fails.
        CN7 - 3 pin connector for serial TX/RX/GND. Likely used for debugging (not populated) and connected directly to the
              PS2 serial port, displaying some boot-up / debug info at power-on similar to Namco System 10.
        CN8 - 8 pin connector used to program IC1 (not populated)
        CN9 - 6 pin connector
       CN10 - 2 pin connector for 5V fan. Stock part is Matsushita DC BRUSHLESS UDQFSEH12-NC DC5V 0.24A 1304D
     LED1/2 - LEDs connected to EPM3128. The flashing sequence is....
              At power-on, very quick flash on then off.
              After 5 seconds both LEDs light up solid.
              After 10 seconds both LEDs flash on/off continually.
       LED3 - LED for 12V input present. If not lit check the fuse and other serious power-related faults such as shorted MOSFETs.
    EPM3128 - Altera MAX EMP3128ATC100-10 CPLD marked with a code. This is essentially Namco's "KEYCUS" chip.
              Note PCBs could have been factory re-programmed or updated via USB so the code is no guarantee of
              the listed game being on the actual PCB or the software version being the same as the PCB tag sticker.
              Check the table at the top for the list of KP numbers and games.
        IC1 - OKI MR27V3202F 4M x8-bit TSOP48 OTP ROM (boot ROM/BIOS). Configurable as 4Mx8-bit/2Mx16-bit, BYTE pin is
              tied to GND and D8-D15 are 'no-connect' so it is set to 8-bit. Some of the ROMs are marked with a Namco game code.
              After dumping several boards it appears this ROM is common to all games on that revision PCB.
              For example the Namco code seen on a System148 Sea Story board matches the Namco code found on a System148 Animal Kaiser board.
              So IC1 is just a standard System BIOS ROM for that version PCB.
              Umimonogatari Lucky Marine Theater / Sea Story was the first game on the System148 board and has code "ULS1001 STSPR0A"
              printed on this ROM but the actual chip is the same type OKI MR27V3202F.
              So far all games running on the same board version use the same BIOS ROM.
              ROMs:

              SYSTEM               FOUND ON GAME             ROM FILE NAME                     ROM CRC32
              ------------------------------------------------------------------------------------------------
              BIOS System 147                                not dumped
              BIOS System 147(B)   Pacman's Arcade Party     common_system147b_bootrom.ic1     CRC32(5d79cfaf) \
              BIOS System 147(B)   Animal Kaiser Version 1   common_system147b_bootrom.ic1     CRC32(5d79cfaf) | Identical
              BIOS System 147(B)   Pacman Battle Royale      common_system147b_bootrom.ic1     CRC32(5d79cfaf) /
              BIOS System 148      Animal Kaiser Evo8        common_system148_bootrom.ic1      CRC32(3c574b9a) \
              BIOS System 148      Sea Story                 common_system148_bootrom.ic1      CRC32(3c574b9a) | Identical
              BIOS System 148(B)   Animal Kaiser Evo1        common_system148_bootrom.ic1      CRC32(3c574b9a) /

  NAND.IC31 - Samsung K9K8G08U0B 8Gbit (1GB x8-bit) TSOP48 NAND flash ROM
              Tag has a software version listed but software updates were done on some games. The NAND chip doesn't have a Namco game code
              printed on it so there is no way to know what version software is *actually* installed except to look in the test mode.
              Different version boards have this IC at a different PCB IC location.
              ROMs:
                                                        TEST MODE 'OTHERS'
              SYSTEM          GAME                      SOFTWARE VERSION                                 ROM FILE NAME                         ROM CRC32          Notes
              ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
              System 147(B)   Animal Kaiser Version 1   ANA100-4-NA-MPRO-B01 26/11/2009 (THU) 14:26:21   ana100-4-na-mpro-b01_kp005a.ic31      CRC32(e1e7cbaa)
              System 147(B)   Pacman's Arcade Party     PMAAM1-2-NA-MPRO-C03 26/11/2010 (FRI) 14:33:34   pmaam1-2-na-mpro-c03_kp007a.ic26      CRC32(fb42ddb0)    Tag: Software Update from rev A to rev C (replace x with correct numbers when emulated)
              System 147(B)   Pacman Battle Royale      PBR102-2-NA-MPRO-A13 26/09/2011 (MON) 19:47:42   pbr102-2-na-mpro-a13_kp006b.ic26      CRC32(bfd16bae)
              System 148      Animal Kaiser Evo8        ANA200-4-NA-MPRO-A22 17/06/2011 (FRI) 13:10:03   ana200-4-na-mpro-a22_kp012b-1.ic31    CRC32(e1e7cbaa)    \  Supposed to be same version so probably only different serial# and ROM is otherwise identical.
              System 148      Animal Kaiser Evo8        ANA200-4-NA-MPRO-A22 17/06/2011 (FRI) 13:10:03   ana200-4-na-mpro-a22_kp012b-2.ic31    CRC32(1cc153da)    /
              System 148      Umimonogatari L.MarineThr ULS100-1-NA-MPRO-B01 xx/xx/xxxx (xxx) xx:xx:xx   uls100-1-na-mpro-b01_kp008a.ic31      CRC32(b200d76f)
              System 148(B)   Animal Kaiser Evo1        ANA200-4-NA-MPRO-A22 17/06/2011 (FRI) 13:10:03   ana200-4-na-mpro-a22_kp017b-1.ic31    CRC32(9fb7855f)    Modified to suit KP017B protection chip maybe, same dates and A22 software as EVO8.

              Others secured:
                          System 147/148  MANY Unimonogatari versions with tag ULS1001, ULD1001, ULM1001 etc. Dumping these is pointless unless the
                                          whole machine is emulated including the mechanical parts.

              Others seen:
                          System 147(B) Pacmans Arcade Party PMACS1-2-NA-MPRO-A28 2010/09/13 MON 17:10:24 (seen in manual, possibly cocktail version)
                          System 147(B) Pacman Battle Royale PBR100-2-NA-MPRO-A57 MAY/26/2010 WED 20:40:54 (seen in manual)
                          System 147(b) Pacman Battle Royale Tournament Edition PBR103-2-NA-MPRO-A24 (PCB and tag seen on photo)

      16245 - 74LVC16245A 16-bit Bus Transceiver With 3-State Outputs. These chips buffer the 8-bits of R, G & B going into the ADV7125
    ADV7125 - Analog Devices ADV7125KST140 Triple 8-Bit (24-bit) High Speed Video DAC
              On System147(B) and System148(B) the 2x transceivers and ADV7125 are replaced with...
               - A custom QFP64 video DAC marked 'SC44750PB (C)2001 SCEI'. This custom chip is likely to be very similar in function to Sony CXD1178Q Triple 8-bit video DAC.
               - A Renesas EL8300 200MHz Rail-to-Rail Amplifier (on the bottom side of the board).
 K4R881869E - Samsung K4R881869E-G6T9 288Mbit (512kB x 18-bit x 32 banks) Direct RDRAM
              Earlier System147(B) boards use Samsung K4R271669F 128Mbit (256kB x 16-bit x 32 banks) Direct RDRAM
              This likely means older boards are not compatible with later software titles due to having less RDRAM.
              This is also why the same Animal Kaiser game (with different software versions) is found on System 147B, System 148 and System 148B.
    TMC2074 - SMSC TMC2074-NU Network Controller. 40MHz crystal connected to pins 82 & 83
   CXD2947R - Sony CXD2947R Sound Processor with internal 2MB EDO DRAM. Measured clock input 36.8638MHz on pin 100
        SW2 - 2 position switch for network termination. Default = ON
  CXD9833GB - Sony CXD9833GB MIPS III R5900-based EE "Emotion Engine" and GS "Graphics Synthesizer". Measured clock input 393.214MHz
 CXD9799AGP - Sony CXD9799AGP "IOP" I/O processor MIPS R3000A. Clock input 16.9344MHz
       LM70 - Texas Instruments LM70CIM Digital Temperature Sensor
      C807U - Sony C807U-1V86 System Controller. In reality this is a TLCS-870 Series Toshiba TMP87C807C / TMP87PH47U microcontroller
              with 8kB / 16kB x8-bit OTP EPROM re-badged as 'SONY C807U-1V86 (C)2000 SCEI'. Clock input 4.000MHz on pin 15.
              This chip is a QFP44 and also known as the "SYSCON".
     BD9781 - ROHM BD9781HFP Step-Down Switching Regulator with Built-in Power MOSFET
              This creates 5V from a 12V input. Pin 2 (output) is tied to 5V test point
     00BC0W - ROHM BA00BC0W LDO Regulator. Pin 2 (input) connected to 5V. Pin 4 (output) connected to 3.45V test point
   SI4340DY - Vishay Si4340DY Dual N-Channel 20V MOSFET with Schottky Diode. Marked on board as HAT2180RP.
              These two chips create 1.68V and 1.24V.
              No datasheet available for the HAT2180 but it's clearly directly compatible :-)
         F1 - Fuse 6.3A. Note it was found that if the fuse is blown one or both of the SI4340DY MOSFETs are shorted and the EE+GS is dead (i.e. GAME OVER)
      1537D - Mitsumi MM1537DFBE System Reset IC
       706R - Corebai CBM706R Supervisor Circuit, Reset and Watchdog Timer with 4.4V Trigger Level
        485 - Sipex SP485ECN Enhanced Low Power Half-Duplex RS-485 Transceiver. This chip is used to communicate with the I/O board(s).
       LC07 - Texas Instruments 74LC07A Hex Buffer/Driver With Open Collector High-Voltage Output
   MB39A106 - Fujitsu MB39A106 Symmetrical-Phase Two-Channel DC-DC Converter Pulse Width Modulation Controller.
              On System148(B) this is replaced with a Texas Instruments PS5120 and some supporting parts (diodes/resistors etc).
              This chip drives the Si4340DY MOSFETs which create 1.68V and 1.24V. There is a significant amount of current being generated,
              probably around 4A-6A. If the 1.68V and 1.24V test points measure 0V likely this controller is dead, the Si4340DY MOSFETs are
              also shorted/dead and likely the EE+GS is also dead.
       3222 - Sipex SP3222EUCY Dual RS-232 Driver/Receiver.
        245 - Fairchild 74LCX245 Bus Transceiver With 3-State Outputs
      62256 - Cypress CY62256RL-55SL 32kB x8-bit Battery-Backed SRAM. This holds high scores, book-keeping data and software update flags.
              If the battery is changed or goes flat the RAM will be reset and any software updates will be lost. This results in the
              initial factory software loading by default. For example an Animal Kaiser EVO1 updated to EVO8 will revert back to EVO1 when
              the battery dies or is removed. A dump of an EVO5 Update USB Stick is available but no other S/W updates are available currently.
       7673 - Intersil ICL7673 Automatic Battery Backup Switch
      VHC00 - Fairchild 74VHC00 Quad 2-Input NAND Gate
      R4543 - Seiko Epson R4543 Real Time Clock
     PSW1/2 - Polyfuse 110F 16V. Fuse 6.3A. If the fuse is blown the board will likely be dead along with the PWM Controller and the
              Dual-Channel MOSFETs and the GS+EE chip = GAME-OVER ;-)

I/O Board
---------

This I/O board is used for controls, for communication to a card reader/writer
and for network connection on multi-board cabinets such as Sea Story.
To get the game to boot up the minimum requirement is connection of J4 and J5.

2007 NAMCO BANDAI Games Inc.
System147 I/O PCB
also sometimes labelled 'V373 I/O PCB Assy'
8916960205 (8916970205)
|------------------------------|
|  25MHz    46272        261 J9|
|  |------|              261   |
|J7|H82328|  62256       261   |
|  |------|5.5V          261 J6|
|  TMC2074               261   |
|J8  40MHz 00BC0W        261   |
|3222 485 SW2 BD9778 F1  256   |
|    J1   J2  J3  J4  J5 256   |
|------------------------------|
Notes:
       H82328 - Renesas / Hitachi 64F2328VF25V H8S/2328 microcontroller with 256kB x8-bit internal
                flash ROM and 8kB x8-bit internal SRAM. Clock input 25.000MHz
                Each game requires a specific I/O board with specific internal ROM or the game will not boot.
                The actual I/O boards are all identical but the ROM inside the microcontroller is different for each game.
                This takes planned obsolescence to a whole new level.
                There will be a check by the I/O board in the main board NAND so it should be possible the hack the NAND to skip
                the check.
                  - For Animal Kaiser: stamped 'ANK0A' with sticker 'ANA4A' (ANK is the first (unknown) game. Probably factory re-programmed)
                  - For Pac-Man's Arcade Party: sticker 'PMA2A'. Displays "3-1 I/O ERROR 1" when used with Animal Kaiser I/O board.
                  - For Pac-Man Battle Royale: sticker 'PBR2A'. NOTE! Pacman's Arcade Party PMA2A I/O board is confirmed accepted and
                    working on Pacman Battle Royale. However Pacman Battle Royale also requires another I/O board 'A.I. PCB' otherwise
                    it will show 'A.I. BOARD.. NG' and display '3-11 I/O ERROR 3'
          261 - Sanyo FW261 N-Channel Silicon MOSFET
          256 - Sanyo FW256 N-Channel Silicon MOSFET
           J1 - DB9 connector for bar code reader unit. Communication is 3-wire serial TX/RX/GND
           J2 - 7 pin JST-PH connector for IC card reader/writer unit. Communication is 3-wire serial TX/RX/GND
           J3 - 3 pin JST-XH connector for I/O communication with main board via RS485. Pin 1: L+, Pin 2: L-, Pin 3: GROUND
                Pin 1 tied to RS485 IC pin 6 ('A' Driver Output / Receiver Input; Non-Inverting)
                Pin 2 tied to RS485 IC pin 7 ('B' Driver Output / Receiver Input; Inverting)
           J4 - 3 pin JST-PH connector with same function as J3. Tied directly in parallel to J3 with a different plug type
           J5 - 4 pin JST-PA connector for 12V power input
                Pin 1 & 2 = 12V
                Pin 3 & 4 = Ground
           J6 - 40 pin JST-XAD-SS connector for card dispenser unit, service panel and joystick/buttons (varies per game)

                Pinout (Pacman's Arcade Party)

                Pin  Function        Pin  Function         Pin  Function           Pin  Function
                -------------        -------------         -------------           -------------
                1    Ground          11   Button 1         21   +5V                31   Service Credit
                2    Ground          12   -                22   +5V                32   -
                3    Player Down     13   Button 2         23   Down (test mode)   33   Coin 1
                4    -               14   -                24   -                  34   -
                5    Player Up       15   Start 1          25   Up (test mode)     35   Coin 2
                6    -               16   -                26   -                  36   -
                7    Player Right    17   -                27   Enter (test mode)  37   Coin 3
                8    -               18   -                28   -                  38   -
                9    Player Left     19   +12V             29   Test               39   Ground
                10   -               20   +12V             30   -                  40   Ground


                Pinout (Pacman Battle Royale)

                Pin  Function        Pin  Function         Pin  Function           Pin  Function
                -------------        -------------         -------------           -------------
                1    Ground          11   -                21   +5V                31   Coin 1
                2    Ground          12   -                22   +5V                32   -
                3    Player 4 Start  13   -                23   Down (test mode)   33   Service Credit
                4    Player 4 Lamp   14   -                24   -                  34   -
                5    Player 3 Start  15   -                25   Up (test mode)     35   Coin 2
                6    Player 3 Lamp   16   -                26   -                  36   -
                7    Player 2 Start  17   -                27   Enter (test mode)  37   Coin 3
                8    Player 2 Lamp   18   -                28   -                  38   Coin Counter
                9    Player 1 Start  19   +12V             29   Test               39   Ground
                10   Player 1 Lamp   20   +12V             30   -                  40   Ground


                Pinout (Animal Kaiser)

                Pin  Function        Pin  Function         Pin  Function           Pin  Function
                -------------        -------------         -------------           -------------
                1    Ground          11   Card Disp Error  21   +5V                31   -
                2    Ground          12   -                22   +5V                32   -
                3    -               13   Card Input OK    23   Down (test mode)   33   Service Credit
                4    Player 2 Lamp Y 14   -                24   Side LED Right     34   Card Counter
                5    -               15   -                25   Up (test mode)     35   -
                6    Player 2 Lamp G 16   Card Disp Reset  26   Side LED Left      36   AA-LED
                7    -               17   -                27   Enter (test mode)  37   Coin 1
                8    Player 1 Lamp Y 18   Card Disp Data   28   -                  38   Coin Counter
                9    Card Disp Empty 19   +12V             29   Test               39   Ground
                10   Player 1 Lamp G 20   +12V             30   Coin Lockout Coil  40   Ground

                Notes:
                      - = No connect
                      Lamp Y/G = Yellow/Green
                      Card Disp = Card Dispenser
                      AA-LED is simple 12V LED board containing 4x blue LEDs that light up when the card reader/dispenser is active

           J7 - 10 pin JST-XAD-SS connector, 3 pins used for volume pot
           J8 - 10 pin JST-PH connector (not used, tied to H8S/2328 so probably for programming the microcontroller via JTAG)
           J9 - 6 pin JST-PH connector. Different pinout for each game....
                - Animal Kaiser:        For 4x control panel yellow & green button switches (SW-1PY, SW-2PY, SW-1PG, SW-2PY, GND, GND)
                - Pacmans Arcade Party: Pin 3 for Player 2 Start, other pins unused.
                - Pacman Battle Royale: not used
          485 - Sipex SP485ECN Enhanced Low Power Half-Duplex RS-485 Transceiver
      TMC2074 - SMSC TMC2074-NU Network Controller. 40MHz xtal connected to pins 82 & 83
        62256 - Cypress CY62256RL-55SL 32kB x8-bit SRAM
         5.5V - 5.5V Supercap
       BD9778 - ROHM BD9778HFP Step-Down Switching Regulator with Built-in Power MOSFET
                This creates 5V from a 12V input. Pin 2 (output) is tied to 5V test point
       00BC0W - ROHM BA00BC0W LDO Regulator. Pin 2 (input) tied to 5V. Pin 4 (output) tied to 3.3V test point
         3222 - Sipex SP3222EUCY Dual RS-232 Driver/Receiver
        46272 - ROHM BD46272G Voltage Detector
          SW2 - 2 position switch for network termination. Default = ON
           F1 - Fuse 6.3A. If the fuse is blown the board will likely be dead.

This board is used with all known System 147/148 games.


Some other I/O boards seen used with different (or unknown) System147/148 games....

Not documented - Z085 Clerk PCB 1876960202 (1876970202) - Used on Sea Story
Not Documented - Z085 Ruler PCB 1876960102 (1876970102) - Used on Sea Story


(This board layout below from a bad quality photo found online)
V343 A.I. PCB 2627960101 (2627970101)
|--------------------------------------------------|
|  I                   O                           |
|                                                  |
|                     M                            |
|                                                  |
|                                                  |
|  CN6                                             |
|                                               CN1|
|  CN5     CN4          CN3            CN2         |
|--------------------------------------------------|
Notes:
      M - 48 pin micro-controller (unknown type)
      O - Oscillator (unknown clock)
      I - Unknown SOIC8 IC, for RS485 Communication with main board
    CN1 - 2 pin JST-XA Power Input Connector. Pin 1: 12V, Pin 2: GROUND
    CN2 - 20 pin JST-XAD-SS connector used for joystick directional controls for player 1 & 2. Only pins 1-9 used, others unused.

                Pin  Function
                -------------------
                1    Player 1 Down
                2    Player 1 Up
                3    Player 1 Right
                4    Player 1 Left
                5    Player 2 Up
                6    Player 2 Down
                7    Player 2 Left
                8    Player 2 Right
                9    Ground

    CN3 - 26 pin JST-XAD-SS connector (not used)
    CN4 - 22 pin JST-XAD-SS connector used for joystick directional controls for player 3 & 4. Only pins 1-8 used, others unused.

                Pin  Function
                -------------------
                1    Player 3 Up
                2    Player 3 Down
                3    Player 3 Left
                4    Player 3 Right
                5    Player 4 Down
                6    Player 4 Up
                7    Player 4 Right
                8    Player 4 Left

    CN5 - Not populated 3 pin JST-XH connector with same function as CN6. Tied directly in parallel to CN6 with a different plug type
    CN6 - 3 pin JST-PH connector for I/O communication with main board via RS485. Pin 1: L+, Pin 2: L-, Pin 3: GROUND

Only low res photo seen so not fully documented.
Board parts unknown but it's using a small 48 pin microcontroller and some logic chips/mosfets/etc. This board is used with
Pacman Battle Royale and daisy-chains to the System147 I/O PCB via the secondary RS485 JST-XH 3 pin I/O board connector J3.
The board is used only for joystick controls and allows connection of 4 players with up, down, left and right switches
(16 connections total). The start buttons and start lamps for each player are connected to the System147 I/O PCB 40 pin
connector J6. If this board is not present the game will attempt to boot then stop at the I/O board check screen and show
ERROR 3-11. Test mode can be entered and navigated but that's all.

*****************************************************************************/


#include "emu.h"

#include "cpu/mips/mips3.h"

#include "emupal.h"
#include "screen.h"


namespace {

class namcos14x_state : public driver_device
{
public:
	namcos14x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void system14x(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<mips3_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void namcos14x_state::video_start()
{
}

uint32_t namcos14x_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void namcos14x_state::program_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram();
	map(0x1fc00000, 0x1fffffff).rom().region("bios", 0);
}


static INPUT_PORTS_START( system14x )
INPUT_PORTS_END


void namcos14x_state::system14x(machine_config &config)
{
	R5000LE(config, m_maincpu, 393'214'000); // actually R5900 'Emotion Engine'
	m_maincpu->set_icache_size(16384); // TODO
	m_maincpu->set_dcache_size(16384); // TODO
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos14x_state::program_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(namcos14x_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);

	PALETTE(config, "palette").set_entries(65536);
}


#define SYSTEM147_BIOS  \
	ROM_LOAD( "common_system147b_bootrom.ic1", 0x000000, 0x400000, CRC(5d79cfaf) SHA1(8e71206a10f614d745bce8d7505aeb405f0d610c) ) //  1xxxxxxxxxxxxxxxxxxxxx = 0x00

#define SYSTEM148_BIOS  \
	ROM_LOAD( "common_system148_bootrom.ic1", 0x000000, 0x400000, CRC(3c574b9a) SHA1(489266cd30a4109509cc9cabdd25642c6f1837c8) ) //  1xxxxxxxxxxxxxxxxxxxxx = 0x00

ROM_START( animalk ) // 2009/11/26 14:35:24 V373 ANIMAL KAISER THE KING OF ANIMALS
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM147_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "kp005a_ana1004-na-b.ic26", 0x00000000, 0x42000000, CRC(c2ee4a79) SHA1(67c751c85eb99fb3cba17832acb38f3b88d54756) )
ROM_END

ROM_START( animalke1 ) // 2011/06/17 13:26:31 V373 ANIMAL KAISER EVOLUTION
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM148_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "kp017b_ana2004-na-a.ic31", 0x00000000, 0x42000000, CRC(9fb7855f) SHA1(f0aaeb8c0c0bd9db5284d01bf59ffd97d04ada88) )
ROM_END

ROM_START( animalke2 ) // 2011/06/17 13:26:31 V373 ANIMAL KAISER EVOLUTION
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM148_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "kp012b_ana2004-na-a-1.ic31", 0x00000000, 0x42000000, CRC(1fa8d848) SHA1(d0cdba94aef450b34ddd739fc63b2d82ac72d667) )
ROM_END

ROM_START( animalke8 ) // 2011/06/17 13:26:31 V373 ANIMAL KAISER EVOLUTION
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM148_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "kp012b_ana2004-na-a.ic31", 0x00000000, 0x42000000, CRC(e1e7cbaa) SHA1(0b2ea649e94b1ff40fcbd744a2d0c319a2adec94) )
ROM_END

ROM_START( animalkeu ) // 2011/06/17 13:26:31 V373 ANIMAL KAISER EVOLUTION
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM148_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "kp017b_ana2004-na-a-1.ic31", 0x00000000, 0x42000000, CRC(3b8a08c0) SHA1(683282d3aa01d4b3a4f20283e6ad16c067826f91) )
ROM_END

ROM_START( lmarinet ) // 2010/10/20 18:24:35 SEA STORY LMT SPECIAL [ST]
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM148_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "uls100-1-na-mpro-b01_kp008a.ic31", 0x00000000, 0x42000000, CRC(b200d76f) SHA1(212bd7a784fdd1c5639ff31d84982ae8e101b072) )
ROM_END

ROM_START( pacmanap ) // 2010/09/13 17:38:24 PMA
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM147_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "kp007a_pmaam12-na-c.ic26", 0x00000000, 0x42000000, CRC(fb42ddb0) SHA1(d7ec90b2f5c1ed7872f3ca7bd0511dd17427d987) )
ROM_END

ROM_START( pacmanbr ) // 2011/09/26 21:45:22 PBR US FULL A13
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM147_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "pbr102-2-na-mpro-a13_kp006b.ic26", 0x00000000, 0x42000000, CRC(bfd16bae) SHA1(fd23030e6b065ce2db18e3f23084f52079082ae3) )
ROM_END

ROM_START( pacmanbrj ) // 2011/02/16 08:44:24 PBR JP FULL
	ROM_REGION32_LE( 0x400000, "bios", 0 )
	SYSTEM147_BIOS

	ROM_REGION( 0x42000000, "nand", 0 )
	ROM_LOAD( "pbr101-1-na-mpro-a23.ic26", 0x00000000, 0x42000000, CRC(51d6d25a) SHA1(8ecb1d9d5594954b7aabb08ac5c6d4f11b9e5c4e) )
ROM_END

} // anonymous namespace


// System 147 games
GAME( 2009, animalk,     0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Animal Kaiser - The King Of Animals (ANA1004-NA-B)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, pacmanbr,    0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Pac-Man Battle Royale (US, PBR1022-NA-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, pacmanbrj,   pacmanbr,  system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Pac-Man Battle Royale (Japan, PBR101-1-NA-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2010, pacmanap,    0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Pac-Man's Arcade Party (PMAAM12-NA-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// System 148 games
GAME( 2011, animalke1,   0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Animal Kaiser - The King of Animals (Evo 1, ANA2004-NA-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, animalke2,   0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Animal Kaiser - The King of Animals (Evo 2, ANA2004-NA-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, animalke8,   0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Animal Kaiser - The King of Animals (Evo 8, ANA2004-NA-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, animalkeu,   0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Animal Kaiser - The King of Animals (unknown Evo, ANA2004-NA-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2010, lmarinet,    0,         system14x, system14x, namcos14x_state, empty_init, ROT0, "Namco", "Umi Monogatari Lucky Marine Theater (ULS1001-ST-A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
