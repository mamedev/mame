// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca, Grull Osgo
/******************************************************************************

   - NORAUT POKER SYSTEM -
  -------------------------

  Driver by Roberto Fresca, Angelo Salese & Grull Osgo.


  Games running on this hardware:

  -- Z80 based --

   * Noraut Poker,                                  1988,  Noraut Ltd.
   * Noraut Deluxe Poker (console),                 198?,  Noraut Ltd.
   * Noraut Deluxe Poker (bootleg),                 198?,  Unknown.
   * Noraut Joker Poker (original),                 198?,  Noraut Ltd.
   * Noraut Joker Poker (text hack),                198?,  bootleg.
   * Noraut Joker Poker (Prologic HW),              198?,  Video Fun Games Ltd.
   * Noraut Joker Poker (N1A),                      1988,  Noraut Ltd.
   * Noraut Joker Poker (alt),                      1988,  Noraut Ltd.
   * Noraut Red Hot Joker Poker,                    1988,  Noraut Ltd.
   * Noraut Red Hot Joker Poker (alt HW),           198?,  Noraut Ltd.
   * Noraut Poker (NTX10A),                         1988,  Noraut Ltd.
   * Noraut Jacks Plus / Joker Poker (V3.010a),     2002,  Noraut Ltd.
   * Noraut Jacks Plus / Joker Poker (V3.011a),     2003,  Noraut Ltd.
   * Noraut Joker Poker (console, set 1),           198?,  Noraut Ltd.
   * Noraut Joker Poker (console, set 2),           198?,  Noraut Ltd.
   * Mainline Double Joker Poker / Jacks Plus,      198?,  Mainline London.
   * Double Joker Poker (45%-75% payout),           1996,  DellFern Ltd.
   * Double Joker Poker (45%-75% payout, alt),      1996,  DellFern Ltd.
   * Royal on Ten (hack of Noraut Deluxe Poker),    2005,  Unknown.
   * Credit Poker (ver.30c, standard),              1999,  CGI.
   * Credit Poker (ver.30b, 7 & 9 bonus),           1999,  CGI.
   * Credit Poker (ver.23b, 7 & 9 bonus),           1998,  CGI.
   * Double Joker Poker (CGI),                      19??,  CGI.
   * Kimble Double HI-LO (Z80 version),             198?,  Kimble Ireland.
   * Poker / Black Jack (Model 7521),               198?,  M. Kramer Manufacturing.
   * Draw Poker (Joker Poker V.01),                 1984,  Coinmaster.

   * Winbank Joker Poker,                           1988,  Noraut Ltd.
   * Noraut Poker (6007 32k),                       1988,  Noraut Ltd.
   * Noraut Poker (6007 64k),                       1988,  Noraut Ltd.
   * Double Joker Poker Dellfern 1996,              1996,  Dellfern Ltd.
   * Dellfern V18 (77's, 99's & Suits, 1 Credit),   1997,  Dellfern Ltd.
   * Dellfern V18 (77's, 99's & Suits, 10 Credits), 1997,  Dellfern Ltd.
   * Dellfern V18 (Joker Poker, raise bet, 2p/10p), 1997,  Dellfern Ltd.
   * Dellfern V18 (£6 Jackpot Bank Poker),          1997,  Dellfern Ltd.
   * Dellfern V18 (Joker Poker / Jack Plus),        1997,  Dellfern Ltd.
   * Dellfern V18 (Amusement Poker),                1997,  Dellfern Ltd.


  -- 8080 based --

   * Draw Poker HI-LO,                              1982,  M. Kramer Manufacturing.
   * Joker Poker (Kramer, alt),                     1983,  Unknown.
   * Draw Poker HI-LO (Japanese),                   1983,  Unknown.
   * New Hi-Low Poker,                              198?,  Song won?
   * unknown poker game PKII/DM,                    198?,  Unknown.
   * PMA Poker,                                     1983,  PMA.
   * Kimble Double HI-LO,                           198?,  Kimble Ireland.
   * Double or Nothing Draw Poker,                  1983,  Game Technology Inc.
   * GTI Poker? (SMS hardware),                     1983,  Game Technology Inc.
   * HI-LO Double Up Joker Poker,                   1983,  SMS Manufacturing Corp.
   * Drews Revenge (v.2.89, set 1),                 1986,  Drews Inc.
   * Drews Revenge (v.2.89, set 2),                 1986,  Drews Inc.
   * Southern Systems Joker Poker,                  1982,  Southern Systems & Assembly, Ltd.
   * Fast Draw (poker conversion kit),              198?,  Stern Electronics
   * Sure Shot (older, dphl hardware),              1983,  SMS Manufacturing Corp.
   * Unknown Draw Poker HI-LO,                      198?,  Unknown.


  -- 8080 based with MCU --

   * Turbo Poker 2,                                 1993,  Micro Manufacturing, Inc.


  -- unknown CPU based --

   * unknown LJF Corporation poker game             1989,  LJF Corporation


*******************************************************************************

  This hardware emulation opened a big can of worms. :)

  Seems that the original hardware/game was created by M.Kramer Manufacturing,
  and then reprogrammed, copied, bootlegged, used & abused by other companies
  like Noraut Ltd, Kimble Ireland, GTI, DellFern, Merit Industries, Red Card,
  Blue Games, CGI, Micro Manufacturing, SMS Manufacturing, Drews Distributing,
  Drew Industries, Lynch Enterprises Inc, Hillside Gaming Corp, Electro Sport,
  Mainline London, Southern Systems, Americade Amusement Inc, Prologic Ireland,
  Mosfat, Unique, GEI, Southern Systems, Assembly Ltd., Coinmaster, etc...

  You can see some legal issues in the following links:
  http://cases.justia.com/us-court-of-appeals/F2/783/421/41759/


  Special thanks to Alan Griffin, that kindly helped providing good references
  that allowed me to improve the Noraut system emulation.


*******************************************************************************

  Hardware Notes:
  ---------------

  Noraut Edge Connector (pinouts):

  ------------------+----+------------------
        Component   | PN |   Solder Side
  ------------------+----+------------------
                GND | 01 | GND
                +5V | 02 | +5V
                -5V | 03 | -5V
               +12V | 04 | +12V
                    | 05 | V-sync
                    | 06 | H-sync
                    | 07 |
                GND | 08 | Readout/Audit switch
                GND | 09 | Low level hopper
                GND | 10 | 50p coin in
                GND | 11 | 1 pound in
                GND | 12 | Bet switch
                GND | 13 | Deal/Draw switch
                GND | 14 | Hold 1 switch
                GND | 15 | Half Gamble switch
                    | 16 | Strand / Change Card switch
      Refill switch | 17 | Coin count/sense from hopper
         Low Switch | 18 | High switch
      Hold 3 Switch | 19 | Hold 2 switch
      Hold 5 Switch | 20 | Hold 4 switch
           10p coin | 21 | Deflect
                    | 22 | 50p in meter
                    | 23 | Hopper Motor Drive (low volt switch line NOT 24v)
                    | 24 |
                    | 25 | SPK+
                    | 26 | Panel lamps clock
         Video sync | 27 | Hold 1 lamp
   Bet/Collect lamp | 28 | Deal/Draw lamp
   Change card lamp | 29 | Hold 4 lamp
        Hold 5 lamp | 30 | Panel lights reset
          High lamp | 31 | Half Gamble lamp
        Hold 2 lamp | 32 | Low lamp
      10p Meter out | 33 | Meter refil
        Video Green | 34 | Hold 3 lamp
         Video Blue | 35 | 10p in Meter
          Video Red | 36 | Spark Detect (Not on all boards)


*******************************************************************************

  Control Panel
  -------------

  There are 3 control panel schemes:

  * The default one (11 button-lamps) for systems without CANCEL button.

  .--------------------------------------------------------------------------.
  |                                              .-------. .------. .------. |
  | .------. .------. .------. .------. .------. |  BET  | | HALF | |  HI  | |
  | |      | |      | |      | |      | |      | |COLLECT| |GAMBLE| |      | |
  | | HOLD | | HOLD | | HOLD | | HOLD | | HOLD | '-------' '------' '------' |
  | |CANCEL| |CANCEL| |CANCEL| |CANCEL| |CANCEL| .-------. .------. .------. |
  | |      | |      | |      | |      | |      | | DEAL  | |CHANGE| |  LO  | |
  | '------' '------' '------' '------' '------' | DRAW  | | CARD | |      | |
  |                                              '-------' '------' '------' |
  '--------------------------------------------------------------------------'

   HOLD buttons              = red.
   BET, DEAL, HI & LO        = yellow.
   HALF GAMBLE & CHANGE CARD = orange.


  * The alternate one (12 button-lamps) for systems with CANCEL button.

  .-------------------------------------------------------------.
  | .------. .------. .------. .------. .------.   .----------. |
  | | HOLD | | HOLD | | HOLD | | HOLD | | HOLD |   |   HIGH   | |
  | '------' '------' '------' '------' '------'   '----------' |
  | .------. .------. .------. .------. .------.   .----------. |
  | |CANCEL| |STAND | | SAVE | | DEAL | | BET  |   |   LOW    | |
  | '------' '------' '------' '------' '------'   '----------' |
  '-------------------------------------------------------------'

   HOLD & CANCEL buttons     = yellow (1).
   STAND & DEAL buttons      = orange (1).
   SAVE (HALF GAMBLE) button = blued-green (1).
   BET button                = red (1).
   HIGH & LOW buttons        = yellow.

   (1) Circular-shaped buttons.

  Some lamps are wired in different way in this scheme.


  * The DPHL type (10 button-lamps) for systems with automatic bet.

  .----------------------------------------------.
  | .------. .------. .------. .------. .------. |
  | | HOLD | | HOLD | | HOLD | | HOLD | | HOLD | |
  | '------' '------' '------' '------' '------' |
  | .------. .------. .------. .------. .------. |
  | |CANCEL| |STAND | | BET  | | DEAL | | DRAW | |
  | '------' '------' '------' '------' '------' |
  '----------------------------------------------'

  The BET button is not connected.


*******************************************************************************

  Hardware Analysis: Narout Poker PCB Tile Decoding.
  --------------------------------------------------

  Objective: Reverse-engineer the tile decoding process
             for 16×32 graphics on Narout Poker PCBs.

  Method:    Two PCBs were analyzed and trojanned to determine
             the hardware's GFX interpretation scheme.

  Result:    The derived graphics layout (below) has been fully
             validated and is confirmed accurate.


*******************************************************************************

  Narout System Ports Map
  -----------------------

  There are two different hardware schemes regarding the system PPI ports.
  The Noraut original and the DPHL types have their PPI 8255 devices mapped
  the following way:

  System         PPI-0     PPI-1     PPI-2
  -----------------------------------------
  noraut type   60h-63h   A0h-A3h   C0h-C3h
  DPHL type     7Ch-7Fh   BCh-BFh   DCh-DFh


  (*) Confirmed lines.


  PPI-0 (60h-63h or 7Ch-7Fh); PortA IN.
  DIP Switches bank:

  7654 3210
  ---- ---x  * DIP switch 8
  ---- --x-  * DIP switch 7
  ---- -x--  * DIP switch 6
  ---- x---  * DIP switch 5
  ---x ----  * DIP switch 4
  --x- ----  * DIP switch 3
  -x-- ----  * DIP switch 2
  x--- ----  * DIP switch 1


  PPI-0 (60h-63h or 7Ch-7Fh); PortB OUT.
  Lamps:

  7654 3210
  ---- ---x  * CHANGE CARD lamp.
  ---- --x-  * SAVE / HALF GAMBLE lamp.
  ---- -x--  * HOLD 1 lamp.
  ---- x---  * HOLD 2 lamp.
  ---x ----  * HOLD 3 lamp.
  --x- ----  * HOLD 4 lamp.
  -x-- ----  * HOLD 5 lamp.
  x--- ----  * CANCEL lamp.


  PPI-0 (60h-63h or 7Ch-7Fh); PortC OUT.
  Lamps & Coin Counters:

  7654 3210
  ---- ---x  * HI lamp.
  ---- --x-  * LO lamp.
  ---- -x--  * HOPPER MOTOR DRIVE
  ---- x---  * Payout pulse.
  ---x ----  * Coin 2 counter.
  --x- ----  * Coin 1 counter.
  -x-- ----  + Coin counter related.
  x--- ----  + DEFLECT (always activated).


-----------------------------------------------------------

  PPI-1 (A0h-A3h or BCh-BFh); PortA IN.
  Regular Inputs:

  7654 3210
  ---- ---x  * DEAL / DRAW button.
  ---- --x-  * BET / CHANGE CARD button.
  ---- -x--  * COIN 1 mech.
  ---- x---  * COIN 2 mech.
  ---x ----  * READOUT button (noraut11).
  --x- ----  * HI button.
  -x-- ----  * LO button.
  x--- ----  * PAYOUT button.


  PPI-1 (A0h-A3h or BCh-BFh); PortB IN.
  Regular Inputs:

  7654 3210
  ---- ---x  * STAND / TAKE button.
  ---- --x-  * SAVE / HALF GAMBLE button.
  ---- -x--  * HOLD 1 button.
  ---- x---  * HOLD 2 button.
  ---x ----  * HOLD 3 button.
  --x- ----  * HOLD 4 button.
  -x-- ----  * HOLD 5 button.
  x--- ----  * CANCEL button.


  PPI-1 (A0h-A3h or BCh-BFh); PortC OUT.
  Sound & Lamps:

  7654 3210
  ---- ---x  * DEAL / DRAW Lamp.
  ---- --x-  * BET / COLLECT Lamp.
  ---- -x--  + PANEL LIGHTS RESET (always activated after initialize).
  ---- x---  + PANEL LAMPS CLOCK
  xxxx ----  * Discrete Sound Lines.


-----------------------------------------------------------

  PPI-2 (C0h-C3h or DCh-DFh); PortA IN/OUT
  VRAM Handlers:

  7654 3210
  xxxx xxxx  VRAM DATA.


  PPI-2 (C0h-C3h or DCh-DFh); PortB OUT
  VRAM Handlers:

  7654 3210
  xxxx xxxx  VRAM ADDRESSING.


  PPI-2 (C0h-C3h or DCh-DFh); PortC IN/OUT.
  PortA handshake lines & PC0-PC2 (noraut11 = OUT; noraut12 = IN):

  7654 3210
  ---- ---x  * N/C (noraut11).
  ---- --x-  * N/C (noraut11).
  ---- -x--  * N/C (noraut11).
  ---- ---x  * N/C (noraut12).
  ---- --x-  * READOUT SWITCH (noraut12).
  ---- -x--  * LOW LEVEL HOPPER (noraut12).
  xxxx x---  * PortA HANDSHAKE LINES (all systems).


*******************************************************************************

  PCB lockouts:

  1: "Laser Lockout" Release:
  Press together: HOLD1, HOLD2, CHANGE, and HALF GAMBLE.
  Then release
  Press: BET DEAL + LOW (after set dipsw)
  Finish with "HOLD1"

  2: "FU" Release.
  Press BET + DEAL, release.
  Then Press and hold HOLD2 + HOLD3 + HALF GAMBLE + CHANGE CARD.

  3: "FR" Release.
  Press BET + HIGH, release.
  Then Press and hold HOLD2 + HOLD3 + HALF GAMBLE + CHANGE CARD.

  4.Percentages:
  Unpower Machine. Set DIP switch #1 to ON.
  Power On.
  Use Countdown button to adjust Percentages.
  Set DIP switch #1 to OFF to memorize setting.

  5:
  "50" Appears on R/H side of blue screen
  Turn DIP switch #1 to OFF unless setting Percentage.


  6007 Program and older noraut PCBs:
  BASE PROGRAM: 25 MAX/PAY GHOST CARD / 8 POUNDS

  Switch    OFF             ON                     Function
   1         2P             10P                    Coin
   2       Credit           Payout/Hopper          Play Mode
   3      No MARS MECH      MARS MECH              Coin Control
   4      Bet setting       see below              Bet setting
   5      Raise OFF         Raise ON               Random Raise
   6      Bet setting       see below              Bet setting

  MAX BET
  Switch 4       Switch 6
  on                on         =max bet 25  (5- 8 setting)
  off               on         =max bet 20  (1- 8 setting)
  on                off        =max bet 15
  off               off        =max bet 3

  --------------------------------------------------

  switch 7        Switch 8     Payout Percentage
  OFF               OFF         =  55%
  OFF               ON          =  60%
  ON                OFF         =  65%
  ON                ON          =  70%

  --------------------------------------------------


*******************************************************************************

  *** Game Notes ***

  - norautjp:

    At the first start-up, the game will give you a very clever
    "FU" screen. Press the following buttons *together* on different times
    to get rid of it (and actually initialize the machine):

    * start + bet buttons (1+2);
    * Hold 3 + Hold 2 + Save (Half Gamble) + Change Card (C+X+F+D)

    Also notice that you actually need to map the last four buttons on the
    same button / on a joypad since MAME's steady key doesn't seem to work on
    my end...


  - the following sets have direct credits to play (no bet),
    and show paytable when coin-up through dynamic size change
    of the lines 5 and 6:

    dphl, dphljp, dphlunkb, sureshoto, smshilo, newhilo, pkii-dm,
    pma, gtipoker, fastdrawp


*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------


  Noraut HW:

  0x0000 - 0x1FFF    ; ROM space.
  0x2000 - 0x27FF    ; NVRAM.

  0x60 - 0x63        ; PPI 8255 0 - DIP Switches, lamps & counters.
  0xA0 - 0xA3        ; PPI 8255 1 - Regular Inputs, sound lines & remaining lamps.
  0xC0 - 0xC3        ; PPI 8255 2 - Video RAM access & other stuff.


  DPHL HW:

  0x0000 - 0x1FFF    ; ROM space.
  0x5000 - 0x53FF    ; NVRAM.

  0x7C - 0x7F        ; PPI 8255 0 - DIP Switches, lamps & counters.
  0xBC - 0xBF        ; PPI 8255 1 - Regular Inputs, sound lines & remaining lamps.
  0xDC - 0xDF        ; PPI 8255 2 - Video RAM access & other stuff.


*******************************************************************************

  DRIVER UPDATES:

  [2009-01-27]

  - Initial release.
  - Defined ROM, RAM.
  - Added 2x PPI 8255 for regular I/O.
  - Added complete inputs and hooked DIP switches.
  - Added video RAM support.
  - Added NVRAM.
  - Added lamps support.
  - Added coin counters.
  - Identified the sound writes.
  - Added hardware description.
  - Added pinout scheme.
  - Added technical notes.

  [2009-01-28]

  - Merged GTI Poker (gtipoker.c) with this driver.
  - Added new memory map and machine driver for gtipoker.
  - Hooked 2x PPI 8255 to gtipoker.
  - Hooked the video RAM access ports to gtipoker.
  - Changed norautpn description from Noraut Poker (No Payout),
    to Noraut Poker (bootleg), since the game has payout system.
  - Some clean-ups.

  [2009-08-21]

  - Switched to pre-defined Xtal clock.
  - Changed the way how graphics are banked/accessed.
  - Fixed the graphics offset and number of tiles per bank.
  - Added new set: Noraut Red Hot Joker Poker.
  - Added new set: Noraut Poker (NTX10A).
  - Added new set: Noraut Joker Poker (V3.010a).
  - Fixed the tile size/decode for the first GFX bank.
  - Added proper norautrh inputs, including the readout button.
  - Added partial DIP switches to norautrh.
  - Added more technical notes.

  [2009-08-23/26]

  - Added a default NVRAM to Noraut Joker Poker to bypass the 'F U' screen.
    This is due to the physical keyboard limitation when needs to enter
    4 simultaneous inputs.
  - Executed a trojan on 2 noraut systems to confirm the way 16x32 tiles are decoded.
  - Fixed the x-offset for 32x32 tiles lines.
  - Fixed the screen aspect and visible area.
  - Confirmed correct colors. No bipolar PROM involved.
  - Added Noraut Joker Poker hardware and PCB layouts.
  - Documented the discrete audio circuitry. Added a full diagram.

  [2009-08-29]

  - Fixed the coin counters.
  - Documented all the output ports.
  - Added a scheme with descriptions for every existent port.
  - Added full lamps support to naroutp, naroutjp, naroutrh and naroutpn.
  - Created lamps layouts for 11 and 12-lamps scheme.
  - Rerouted some inputs to mantain the inputs layout.
  - Renamed some inputs to match the text with the real cab buttons.
  - Removed the imperfect colors flag from the existent sets.
  - Added 2 different control panel layouts to the source.
  - Updated technical notes.

  [2009-08-30]

  - Corrected CPU clock to Xtal/8.
  - Discovered 3 new I/O lines coming from PPI-2, PC0-PC2. They are mixed with the handshake ones.
  - Added the READOUT button to noraut12 games.
  - Splitted the main machine driver to cover 2 different Noraut systems.
  - Added partial support for PPI-2, PC0-PC2 output lines on noraut11 games.
  - Figured out other remaining I/O lines.
  - Added new handlers to simulate the handshake lines. Still need real support through PPI-2.
  - Updated technical notes.

  [2009-09-03]

  - Routed the whole video RAM access through PPI-2.
    (bypassed the handshake lines for now).
  - Merged back the noraut machine drivers after the 3rd PPI connection.
  - Added Low Level Hopper manual input.
  - Added a new machine driver for extended hardware.
    It has 2 jumpers that cut the a14 and a15 addressing lines.

  [2009-09-09]

  - Added accurate discrete sound system emulation.
  - Fixed the discrete sound system diagram, based on real sound references.

  [2009-10-12]

  - Added Draw Poker HI-LO hardware support, based on 8080A CPU.
  - Mirrored the PPI's offsets to simplify/merge the hardware emulation.
  - Added hardware documentation and PCB layouts from both DPHL sets.
  - Added DPHL discrete sound circuitry scheme/documentation.
  - Added Turbo Poker 2 from Micro Manufacturing.
  - Added PMA poker.
  - Documented the Turbo Poker 2 hardware.
  - Added Turbo Poker 2 PCB layout from hi-res picture.
  - Switched to the new PPI core.
  - Commented out the 3rd PPI device till handshaked strobe lines can be
    properly emulated. For now, all VRAM access is through direct handlers.
    This allow to remove the hacks per set needed to boot the games.

  [2009-10-13]

  - Added Draw Poker HI-LO (japanese), based on 8080A CPU.
  - Merged the gtipoker memory map and machine driver with dphl.
  - Created a base machine driver and then derivatives by hardware.
  - Splitted the regular RAM and NVRAM systems.
  - Added 'HI-LO Double Up Joker Poker' from SMS Manufacturing.
  - Added smshilo hardware details and PCB layout.
  - Added smshilo discrete sound circuitry scheme/documentation.

  [2009-11-26]

  - A lot of work in memory maps and machine driver stuff.
  - Improved Inputs / DIP switches for some games.
  - Added lamps support / layouts to new working games.
  - Added preliminary support to Kimble hardware.
  - Added support to CGI's Credit Poker 30x games.
  - Added new (and fixed old) technical notes.
  - Added Noraut Deluxe Poker (console).
  - Added Noraut Joker Poker (original).
  - Added Noraut Red Hot Joker Poker (alt HW).
  - Added Noraut Joker Poker (V3.011a).
  - Added Noraut unknown set 1 (console).
  - Added Noraut unknown set 2 (console).
  - Added Mainline Double Joker Poker.
  - Added Double Joker Poker (45%-75% payout).
  - Added Royal on Ten (Noraut Deluxe hack).
  - Added Credit Poker (ver.30c, standard).
  - Added Poker / Black Jack (Model 7521).
  - Added Kimble Joker Poker.
  - Added DRHL Poker (v.2.89).
  - Renamed norautpn description to Noraut Deluxe Poker (bootleg).
  - Added a placeholder for tpoker2's undumped 68705 MCU.
  - Reorganized the driver, plus some clean-ups.
  - Renamed kimblejp to kimbldhl. Changed game description to Kimble Double HI-LO.
  - Added specific memory map & machine driver to Kimble Double HI-LO.
  - Fix the Kimble Double HI-LO CPU type.
  - Added notes about the code obfuscation and PPI's handling/offsets.

  [2009-12-04]

  - Added new technical notes.
  - Added Kimble Double HI-LO (z80 HW mod).
  - Added Noraut Joker Poker (Prologic HW).
  - Added proper discrete sound support to Kimble games,
    and Prologic-Noraut Joker Poker.
  - Slightly adjusted the visual area.
    This can be wrong and must be rechecked.
  - Relocated the hardware notes and layouts to ROM_LOAD section,
    so can be seen/maintained in a cleaner way.
  - Fixed the default lamps state.

  [2009-12-08]

  - Added Fast Draw (poker conversion kit)?. Seems based on 8080 CPU hardware.
  - Added Draw Poker HI-LO (unknown, rev 1). Seems based on 8080 CPU hardware.
  - Added Draw Poker HI-LO (unknown, rev 2). Seems based on 8080 CPU hardware.
  - Added a PCB layout for M.Kramer's Black Jack Poker, based on a hi-res pic.
  - Added undumped devices as NO_DUMP.
  - Added some technical notes.

  [2009-12-08]

  - Added Southern Systems Joker Poker. Based on 8080 CPU.
  - Added some technical notes.

  [2011-01-24]

  - Changed DRHL description to "Drews Revenge (v.2.89, set 1)"
  - Added Drews Revenge (v.2.89, set 2). Based on 8080 CPU.

  [2013-04-09]

  - Added GTI Poker? (SMS hardware). Based on 8080 CPU.
  - Added PCB description and sound hardware schematics.

  [2013-2023]

  - Added more sets.
  - Partial decryption of norautua & norautub sets.
  - Decryption of noraut3 sets.
  - Working inputs, DIP switches, and button-lamps layouts to noraut3 games.
  - Change game description to noraut3 games, since are dual games selectable
    through a DIP switch.

  [2025-03]

  - Consolidate driver in one file.
  - New palette for drhl and drhla.
  - Added new test functions for drhl, drhla, dphl, and dphla.
  - Cleaned all the conditional unused things.
  - Rewrote inputs for different sets.
  - DIP switches improvements for norautp, norautrh,
  - New inputs for norautua, dphla, drhl, ssjkrpkr.
  - New portmap for dphl, dphla.
  - Added Interrupts through vertical sync for all sets based on i8080.
  - Fixed norautua and norautub table for bin to bcd conversion.
  - New button-lamps layout with 10 buttons for dphl, dphla, dphljp,
    drhl, drhla, and ssjkrpkr.
  - Clickable buttons for all former buttons-lamps layouts.
  - Promoted drhl, drhla, norautua, norautub, and ssjkrpkr to working.

  - Implemented a custom timer and counter to get correct reads in unknown hardware.
  - Added buttons-lamps layout to smshilo.
  - Promoted dphl, dphljp, and smshilo to working.
  - Added technical notes.

  - Added hopper support.
  - Extended palette.
  - Identified dphlunka as Sure Shot.
  - Custom palette for Sure Shot.
  - Buttons-lamps layout for Sure Shot.
  - Inputs for Sure Shot.
  - Machine config for Sure Shot.
  - Promoted dphla set to working.
  - Promoted sureshoto set to working.
  - Promoted dphlunkb set to working.

  - Added a default NVRAM to fastdrwp.
  - Derivated inputs for fasdrwp.
  - Added buttons-lamps layout for fastdrwp.
  - Promoted fastdrwp set to working.

  - Added support for dynamic size chars lines.
    That fixes the paytable shown in different games
    such as dphl, dphljp, dphlunkb, sureshoto, smshilo,
    etc, allowing to remove the imperfect graphics flags.
  - norautua: fixed the overflow when drawing cards
    in the possible straight case.
  - Rewrote the screen update to save a big number of lines.
  - Removed the driver init for ssjkrpkr.
  - Added Inputs & DIP switches for gtipoker.
  - Change gtipoker game description to: Aces or Better Draw Poker.
  - Rearrange the gtipoker graphics ROM.
  - Promoted gtipoker set to working.
  - Added technical notes.

  - Added 3 new sets: df_djpkra, norautn1a, and norautpt.
  - Proper machine config and memory map for gtipokra.
  - Figured out the DIP switch for gtipoker minimal hand.
  - Fixed some ROM loads.
  - Added technical notes.

  - Unified some machine configs.
  - Added inputs & DIP switches for newhilop and pkii_dm.
  - Switched CPU to i8080 for newhilop and pkii_dm.
  - Added buttons-lamps clickable layout for newhilop and pkii_dm.
  - Promoted newhilop and pkii_dm sets to working.

  - Added inputs & DIP switches for pma.
  - Switched CPU to i8080 for pma.
  - Added buttons-lamps clickable layout for pma.
  - Promoted pma set to working.
  - Added ASCII PCB layout for Dellfern sets.
  - Improved Prologic, Kimble Z80, and Noraut JP
    ASCII PCB layouts.

  [2025-04]

  - Decoded color palettes from bipolar PROMs
    (applies to hardware types featuring these devices)
  - Implemented dynamic palette per line.
  - Unified sureshoto and dphl machine config.
  - Real colors for dphl, dphljp, newhilop, pkii_dm,
    pma, gtipoker, smshilo, fastdrwp, sureshoto,
    and dphlunkb.
  - Fixed more ASCII PCB layouts.
  - Improved edge connector / pinout diagram.
  - Fixed documentation about the 3x PPI 8255 devices
    for both narout and DPHL types of hardware.
  - Documented control panels, and added/fixed a lot
    of technical notes.
  - Added new sets, worked inputs, DIP switches, layouts, lamps,
    and promoted to working: winbnk88, nor6007a, nor6007b,
    ddellf96, ddellf97, ddellf97a, ddellf97b, ddellf97c,
    ddellf97d, ddellf97e.
  - Hopper support for winbank88, nor6007a and nor6007b.

  - Identified the unkljfpk set as Poker Plus 6 Cards.
  - Added new set of Turbo Poker II as parent.
  - Added new inputs, worked machine configs, and
    buttons-lamps layouts for new games.
  - Got rid of some now unnecessary driver inits
  - Added another set of Turbo Poker II as first clone.
  - Added support for special Turbo Poker II NVRAM.
  - Created an extended memory map and respective machine config.
  - Added support for enable/disable the NVRAM, required
    by the Turbo Poker II MCU handling.
  - Created a machine start to init all the necessary things.
  - Added new sets: cgip30e, hilokimb, norautpb,
    gtipokrb, pokplusa, pokplusb, tpoker2, & tpoker2a.
  - More sets: krampcb2, krampcb5 (now working),
    and krampcb3, krampcb4, krampcb6 (encrypted).
  - Added the PLDs to the Turbo Poker II games as binaries.
  - Unified the bipolar PROM palette size in 256 entries.
  - Added more documentation, and ASCII PCB layouts.
  - Added technical notes.

  - Added workaround to show cards in tpoker2a and tpoker2b
    attract for testing purposes.
  - Turbo Poker II palette decoded from bipolar PROMs.
  - Adjusted the Poker Plus palette to 256 entries.
  - Dedicated Turbo Poker II memory map and machine config.


  TODO:

  - Investigate and document what is connected to port EFh.
  - Parent/clone relationship.


*******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"

#include "speaker.h"

#include "noraut09_sureshot.lh"
#include "noraut10.lh"
#include "noraut11.lh"
#include "noraut12.lh"


#define NORAUT_MASTER_CLOCK     XTAL(18'432'000)
#define DPHL_MASTER_CLOCK       XTAL(18'000'000)
#define NORAUT_CPU_CLOCK        NORAUT_MASTER_CLOCK / 8     // 2.30275 MHz - Measured: 2.305 MHz
#define DPHL_CPU_CLOCK          DPHL_MASTER_CLOCK / 9       // 2 MHz (from 8224)

// Discrete Sound Input Nodes
#define NORAUTP_SND_EN          NODE_01
#define NORAUTP_FREQ_DATA       NODE_02
#define TP_NVRAM_SIZE           0x800

class norautp_state : public driver_device
{
public:
	norautp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_ppi8255(*this, "ppi8255_%u", 0),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void noraut_base(machine_config &config);
	void gtipkra(machine_config &config);
	void kimble(machine_config &config);
	void kimbldhl(machine_config &config);
	void norautp(machine_config &config);
	void norautu(machine_config &config);
	void norautx4(machine_config &config);
	void norautpl(machine_config &config);
	void tpoker2(machine_config &config);
	void nortest1(machine_config &config);
	void ssjkrpkr(machine_config &config);
	void dphl(machine_config &config);
	void dphla(machine_config &config);
	void dphlxtnd(machine_config &config);
	void drhl(machine_config &config);
	void norautxp(machine_config &config);
	void noraut3(machine_config &config);
	void cgidjp(machine_config &config);
	void cdrawpkr(machine_config &config);
	void krampcb4(machine_config &config);

	void init_enc();
	void init_unka();
	void init_kram4();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void ppi2_obf_w(int state);
	TIMER_CALLBACK_MEMBER(ppi2_ack);
	uint8_t test2_r();
	uint8_t test3_r();
	uint8_t test4_r();
	TIMER_DEVICE_CALLBACK_MEMBER(test_timer_cb);
	void mainlamps_w(uint8_t data);
	void soundlamps_w(uint8_t data);
	void counterlamps_w(uint8_t data);
	void ppi2_b_w(uint8_t data);
	void norautp_palette(palette_device &palette) const;
	void drhl_palette(palette_device &palette) const;
	void bp_based_palette(palette_device &palette) const;
	uint32_t screen_update_norautp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dphl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cgidjp_map(address_map &map) ATTR_COLD;
	void cgidjp_opcodes_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void dphl_map(address_map &map) ATTR_COLD;
	void dphl_portmap(address_map &map) ATTR_COLD;
	void dphla_portmap(address_map &map) ATTR_COLD;
	void dphla_map(address_map &map) ATTR_COLD;
	void dphlxtnd_map(address_map &map) ATTR_COLD;
	void drhl_portmap(address_map &map) ATTR_COLD;
	void tpoker2_map(address_map &map) ATTR_COLD;
	void drhl_map(address_map &map) ATTR_COLD;
	void gtipa_map(address_map &map) ATTR_COLD;
	void kimbldhl_map(address_map &map) ATTR_COLD;
	void kimble_map(address_map &map) ATTR_COLD;
	void noraut3_map(address_map &map) ATTR_COLD;
	void noraut3_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void norautp_map(address_map &map) ATTR_COLD;
	void norautp_portmap(address_map &map) ATTR_COLD;
	void norautx4_map(address_map &map) ATTR_COLD;
	void norautx8_map(address_map &map) ATTR_COLD;
	void norautxp_map(address_map &map) ATTR_COLD;
	void norautxp_portmap(address_map &map) ATTR_COLD;
	void nortest1_map(address_map &map) ATTR_COLD;
	void ssjkrpkr_map(address_map &map) ATTR_COLD;
	void krampcb4_map(address_map &map) ATTR_COLD;

	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);
	void nvunlock_w(offs_t offset, uint8_t data);

	std::unique_ptr<uint16_t[]> m_np_vram;
	required_device<cpu_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device_array<i8255_device, 3> m_ppi8255;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ticket_dispenser_device> m_hopper;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	output_finder<12> m_lamps;

	std::unique_ptr<uint8_t[]> m_nvram8;

	bool m_display_line_control = false;
	bool m_nvunlock = false;
	uint8_t m_videoram[0x800] = {};
	int m_vreg = 0;
	int m_test3 = 1;
	int m_test3b = 0;
	int m_test_count = 0;

};


void norautp_state::machine_start()
{
	m_lamps.resolve();
	m_nvram8 = std::make_unique<uint8_t[]>(TP_NVRAM_SIZE);
	m_nvram->set_base(m_nvram8.get(),TP_NVRAM_SIZE);
	save_item(NAME(m_videoram));
	save_item(NAME(m_nvunlock));
}


/*********************************************
*               Video Hardware               *
*********************************************/

void norautp_state::video_start()
{
	m_np_vram = make_unique_clear<uint16_t[]>(0x800);
}

uint32_t norautp_state::screen_update_norautp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (int y = 0, c = 0; y < 8; y++)
	{
		bool double_w = y == 2 || ((y == 4 || y == 5) && !(m_display_line_control && m_vreg));

		for (int x = 0; x < (double_w ? 16 : 32); c += double_w ? 2 : 1, x++)
		{
			int t = m_np_vram[c] & 0x3f;
			int col = ((m_np_vram[c] >> 6) + (double_w && y >= 4 ? 4 : 0)) & 3;
			m_gfxdecode->gfx(double_w ? 1 : 0)->opaque(bitmap, cliprect, t, col, 0, 0,
				double_w ? (x << 5) + 8 : x << 4, y << 5);
		}
	}
	return 0;
}

uint32_t norautp_state::screen_update_dphl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (int y = 0, c = 0; y < 8; y++)
	{
		bool double_w = y == 2 || ((y == 4 || y == 5) && !(m_display_line_control && m_vreg));

		for (int x = 0; x < (double_w ? 16 : 32); c += double_w ? 2 : 1, x++)
		{
			int t = m_np_vram[c] & 0x3f;
			int col = (((m_np_vram[c] >> 6) + (double_w && y >= 4 ? 4 : 0)) & 3 ) + (y * 4);
			m_gfxdecode->gfx(double_w ? 1 : 0)->opaque(bitmap, cliprect, t, col, 0, 0,
				double_w ? (x << 5) + 8 : x << 4, y << 5);
		}
	}
	return 0;
}


void norautp_state::norautp_palette(palette_device &palette) const
{
	// 1st gfx bank
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0xff));    // blue
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0x00));    // yellow
	palette.set_pen_color(2, rgb_t(0x00, 0x00, 0xff));    // blue
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(4, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(5, rgb_t(0xff, 0x00, 0x00));    // red
	palette.set_pen_color(6, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(7, rgb_t(0x00, 0x00, 0x00));    // black
		// 1st gfx bank
	palette.set_pen_color(8, rgb_t(0x00, 0x00, 0xff));    // blue
	palette.set_pen_color(9, rgb_t(0xff, 0xff, 0x00));    // yellow
	palette.set_pen_color(10, rgb_t(0x00, 0x00, 0xff));    // blue
	palette.set_pen_color(11, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(12, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(13, rgb_t(0xff, 0x00, 0x00));    // red
	palette.set_pen_color(14, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(15, rgb_t(0x00, 0x00, 0x00));    // black
}

void norautp_state::drhl_palette(palette_device &palette) const
{
	// 1st gfx bank  paleeta para drhl
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));    // black (blue)
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff));    // white (yellow)
	palette.set_pen_color(2, rgb_t(0xff, 0x00, 0x00));    // red   (blue)
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0xff));    // white (white)
	palette.set_pen_color(4, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(5, rgb_t(0x00, 0x00, 0xff));    // red
	palette.set_pen_color(6, rgb_t(0xff, 0xff, 0x00));    // yellow (white)
	palette.set_pen_color(7, rgb_t(0x00, 0x00, 0xff));    // blue   (black)
	palette.set_pen_color(8, rgb_t(0x00, 0x00, 0x00));    // black (blue)
	palette.set_pen_color(9, rgb_t(0xff, 0xff, 0xff));    // white (yellow)
	palette.set_pen_color(10, rgb_t(0xff, 0x00, 0x00));    // red   (blue)
	palette.set_pen_color(11, rgb_t(0xff, 0xff, 0xff));    // white (white)
	palette.set_pen_color(12, rgb_t(0xff, 0xff, 0xff));    // white
	palette.set_pen_color(13, rgb_t(0x00, 0x00, 0xff));    // red
	palette.set_pen_color(14, rgb_t(0xff, 0xff, 0x00));    // yellow (white)
	palette.set_pen_color(15, rgb_t(0x00, 0x00, 0xff));    // blue   (black)
}

void norautp_state::bp_based_palette(palette_device &palette) const
{
/*  prom bits
    7654 3210
    ---- ---x   BLUE component.
    ---- --x-   GREEN component.
    ---- -x--   RED component.
    ---- x---   intensity?.
    xxxx ----   unused.
*/

	// 0000IRGB
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		constexpr int intenmin = 0xff;  // lower intensity...
		constexpr int intenmax = 0xff;  // higher intensity...

		// intensity component
		int const inten = BIT(color_prom[i], 3);

		// red component
		int const r = BIT(color_prom[i], 2) * (inten ? intenmax : intenmin);

		// green component
		int const g = BIT(color_prom[i], 1) * (inten ? intenmax : intenmin);

		// blue component
		int const b = BIT(color_prom[i], 0) * (inten ? intenmax : intenmin);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*******************************************
*               R/W Handlers               *
*******************************************/

void norautp_state::mainlamps_w(uint8_t data)
{
/*  PPI-0 (60h-63h or 7Ch-7Fh); PortB OUT.
    Lamps:

    7654 3210
    ---- ---x  * CHANGE CARD lamp.
    ---- --x-  * SAVE / HALF GAMBLE lamp.
    ---- -x--  * HOLD 1 lamp.
    ---- x---  * HOLD 2 lamp.
    ---x ----  * HOLD 3 lamp.
    --x- ----  * HOLD 4 lamp.
    -x-- ----  * HOLD 5 lamp.
    x--- ----  * CANCEL lamp.
*/
	m_lamps[0] = BIT(data, 0);  // CHANGE CARD lamp
	m_lamps[1] = BIT(data, 1);  // SAVE / HALF GAMBLE lamp
	m_lamps[2] = BIT(data, 2);  // HOLD 1 lamp
	m_lamps[3] = BIT(data, 3);  // HOLD 2 lamp
	m_lamps[4] = BIT(data, 4);  // HOLD 3 lamp
	m_lamps[5] = BIT(data, 5);  // HOLD 4 lamp
	m_lamps[6] = BIT(data, 6);  // HOLD 5 lamp
	m_lamps[7] = BIT(data, 7);  // CANCEL lamp

	m_vreg = m_vreg ^ BIT(data, 0);

//  popmessage("lamps: %02x", data);
}

void norautp_state::counterlamps_w(uint8_t data)
{
/*  PPI-0 (60h-63h or 7Ch-7Fh); PortC OUT.
    Lamps & Coin Counters:

    7654 3210
    ---- ---x  * HI lamp.
    ---- --x-  * LO lamp.
    ---- -x--  * HOPPER MOTOR DRIVE
    ---- x---  * Payout pulse.
    ---x ----  * Coin 2 counter.
    --x- ----  * Coin 1 counter.
    -x-- ----  + Coin counter related.
    x--- ----  + DEFLECT (always activated).
*/
	m_lamps[10] = BIT(data, 0);  // HI lamp
	m_lamps[11] = BIT(data, 1);  // LO lamp

	m_hopper->motor_w(BIT(data, 2));

	machine().bookkeeping().coin_counter_w(0, data & 0x10);  // Coin1/3 counter
	machine().bookkeeping().coin_counter_w(1, data & 0x20);  // Coin2 counter
	machine().bookkeeping().coin_counter_w(2, data & 0x08);  // Payout pulse
}

void norautp_state::soundlamps_w(uint8_t data)
{
/*  PPI-1 (A0h-A3h or BCh-BFh); PortC OUT.
    Sound & Lamps:

  7654 3210
  ---- ---x  * DEAL / DRAW Lamp.
  ---- --x-  * BET / COLLECT Lamp.
  ---- -x--  + PANEL LIGHTS RESET (always activated after initialize).
  ---- x---  + PANEL LAMPS CLOCK
  xxxx ----  * Discrete Sound Lines.
*/

	m_lamps[8] = BIT(data, 0);  // DEAL / DRAW lamp
	m_lamps[9] = BIT(data, 1);  // BET / COLLECT lamp

	m_vreg = BIT(data, 0);

	// the 4 MSB are for discrete sound
	m_discrete->write(NORAUTP_SND_EN, (data >> 7) & 0x01);
	m_discrete->write(NORAUTP_FREQ_DATA, (data >> 4) & 0x07);

//  popmessage("sound bits 4-5-6-7: %02x, %02x, %02x, %02x", ((data >> 4) & 0x01), ((data >> 5) & 0x01), ((data >> 6) & 0x01), ((data >> 7) & 0x01));
}

void norautp_state::ppi2_b_w(uint8_t data)
{
/*  PPI-2 (C0h-C3h or DCh-DFh); PortB OUT.
    Unknow:

    7654 3210
    ---- ---x  *
    ---- --x-  *
    ---- -x--  *
    ---- x---  *
    ---x ----  *
    --x- ----  *
    -x-- ----  *
    x--- ----  *
*/
	//logerror("PPI2-B: pc:%04x - data:%02x\n", m_maincpu->pc(), data);

}


void norautp_state::ppi2_obf_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(norautp_state::ppi2_ack), this), state);
}

TIMER_CALLBACK_MEMBER(norautp_state::ppi2_ack)
{
	m_ppi8255[2]->pc6_w(param);
	if (param == 0)
	{
		uint8_t const np_addr = m_ppi8255[2]->pb_r();
		uint8_t const vram_data = m_ppi8255[2]->pa_r();
		m_np_vram[np_addr] = vram_data;
		m_videoram[np_addr] = vram_data;
	}
}

// game waits for bit 4 (0x10) to be reset.
uint8_t norautp_state::test2_r()
{
	return 0x00;
}

uint8_t norautp_state::test3_r()
{
	if(m_maincpu->pc() == 0x48)
	{
		m_test3b = (~m_test3b & 0x01) + (m_test_count & 0x02);
		logerror("test3: return - > m_test3b:%02x\n", m_test3b);
		return m_test3b;
	}

	if(m_maincpu->pc() == 0x747)
		return 0x10;

	return (m_test_count &0x0f) + 0x10;
}

uint8_t norautp_state::test4_r()
{
	m_test3 = (~m_test3 & 0x01) + 0x14;
	return m_test3;
}


TIMER_DEVICE_CALLBACK_MEMBER(norautp_state::test_timer_cb)
{
	m_test_count++;
}


void norautp_state::nvunlock_w(offs_t offset, uint8_t data)
{
	logerror("I/O Port: unlock NVRAM (w) access\n");
	m_nvunlock = true;
}

void norautp_state::nvram_w(offs_t offset, uint8_t data)
{
	if((offset >= 0x700) && (offset < 0x70a))
	{
		if(m_nvunlock)
			m_nvram8[offset] = data;
		else
			logerror("nvram(w) locked: offs:%04x - data: %02x\n", offset, data);
	}
	else
		m_nvram8[offset] = data;

	m_nvram8[0x721] = 0x00;
	m_nvram8[0x725] = 0x01;
	if((offset == 0x724) && (data == 6))
		m_nvram8[0x724] = 0xff;

	m_nvunlock = false;
}

uint8_t norautp_state::nvram_r(offs_t offset)
{
//  showing cards in attract
//  for testing purposes
//  sets: tpoker2a, tpoker2b

	m_nvram8[0x70b] = 0xa8;
	m_nvram8[0x70c] = 0xb8;
	m_nvram8[0x70d] = 0xc8;
	m_nvram8[0x70e] = 0xd8;
	m_nvram8[0x70f] = 0xe8;

	m_nvram8[0x710] = 0x20;
	m_nvram8[0x711] = 0x30;
	m_nvram8[0x712] = 0x40;
	m_nvram8[0x713] = 0x50;
	m_nvram8[0x714] = 0x60;

	return m_nvram8[offset];
}


/*********************************************
*           Memory Map Information           *
*********************************************/
/*

  CPU & PPI settings by set...

  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  |   Set     |   CPU   | PPI-0 offset | config | PPI-1 offset | config | PPI-2 offset |         config         |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | norautp   |   Z80   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |  0xC1 (PC0-2 as input) |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | norautdx  |   Z80   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | norautpn  |   Z80   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | norautjp  |   Z80   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |  0xC1 (PC0-2 as input) |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | norautrh  |   Z80   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | norautu   |  BOXED  |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | norautv3  |   Z80   |   unknown    |        |   unknown    |        |   unknown    |                        |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | pma       |   Z80   |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | bjpoker   |   Z80   |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | dphl      |  8080   |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | dphla     |  8080   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | dphljp    |  8080   |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | kimbldhl  |  8080   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | gtipoker  |  8080   |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | smshilo   |  8080   |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | tpoker2   |  8080   |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | drhl      |  8080?  |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | ssjkrpkr  |  8080   |  0x60-0x63   |  0x90  |  0xA0-0xA3   |  0x92  |  0xC0-0xC3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | fastdrwp  |  8080?  |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | sureshoto |  8080?  |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | dphlunkb  |  8080?  |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | pkii_dm   |   Z80?  |  0x7C-0x7F   |  0x90  |  0xBC-0xBF   |  0x92  |  0xDC-0xDF   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+
  | cdrawpkr  |   Z80   |  0x70-0x73   |  0x90  |  0xB0-0xB3   |  0x92  |  0xD0-0xD3   |          0xC0          |
  +-----------+---------+--------------+--------+--------------+--------+--------------+------------------------+

*/
void norautp_state::norautp_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).ram().share("nvram");   // 6116
}

void norautp_state::decrypted_opcodes_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x1fff).rom().share(m_decrypted_opcodes);
}


void norautp_state::norautp_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x63).mirror(0x1c).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0, 0xa3).mirror(0x1c).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc0, 0xc3).mirror(0x3c).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xef, 0xef).rw(FUNC(norautp_state::test2_r), FUNC(norautp_state::nvunlock_w));
}

/*
  Video RAM R/W:

  c0 --> W  ; data (hanshaked)
  c1 --> W  ; addressing
  c2 --> R  ; status (handshaking lines) + input (PC0-2)
  c3 --> W  ; PPI control + alternate 00 & 01 (PC1 out?)

  PPI Mirror isn't accurate.
  There are writes to 0xF7 and reads + compare to 0xEF.
  Don't know what's supposed to be mirrored there.

*/

void norautp_state::nortest1_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x2fff).rom();
	map(0x5000, 0x57ff).ram().share("nvram");
}

void norautp_state::norautxp_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x3fff).rom();  // need to be checked
	map(0x6000, 0x67ff).ram().share("nvram");  // HM6116
}

void norautp_state::norautx4_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");  // 6116
}

void norautp_state::noraut3_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0x4000);
	map(0x6000, 0x67ff).ram().share("nvram");  // 6116
	map(0x8000, 0xbfff).rom().region("maincpu", 0xc000);
}

void norautp_state::noraut3_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0x0000);
	map(0x8000, 0xbfff).rom().region("maincpu", 0x8000);
}

void norautp_state::kimble_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram().share("nvram");
	map(0xc800, 0xcfff).ram();  // working RAM?
}

void norautp_state::cgidjp_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x1fff).rom().region("maincpu", 0x2000);
	map(0x2000, 0x27ff).ram().share("nvram");   // 6116
}

void norautp_state::cgidjp_opcodes_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
}


/*************** i8080 based **************/

void norautp_state::dphl_map(address_map &map)
{
	map.global_mask(0x7fff);  // A15 not connected
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x53ff).ram().share("nvram");  // should be 2x 0x100 segments (4x 2111)
}

void norautp_state::gtipa_map(address_map &map)
{
	//map.global_mask(0x7fff);  // A15 not connected
	map(0x0000, 0x3fff).rom();
	map(0xc000, 0xc3ff).ram().share("nvram");
	map(0xd000, 0xd3ff).ram().share("nvram");

}

void norautp_state::dphla_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram().share("nvram");
}

void norautp_state::dphlxtnd_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram().share("nvram");  // should be 2x 0x100 segments (4x 2111)
}

void norautp_state::ssjkrpkr_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram().share("nvram");
}

void norautp_state::tpoker2_map(address_map &map)
{
	map(0x0000, 0x6fff).rom();
	map(0x7000, 0x7fff).ram();
	map(0x8000, 0x87ff).rw(FUNC(norautp_state::nvram_r), FUNC(norautp_state::nvram_w));
}

/*
  Kimble:

  Program obfuscation to transfer the flow control.
  Has involved calculations and boolean operations
  to modify the PC and transfer the control.

  Create dynamic code in RAM at $C276 to handle the I/O through the PPI8255's.
  Also initialize the devices and handle the handshaking lines in the same way.

  The code read on port $62, when is suppossed to be set as output.

*/
void norautp_state::kimbldhl_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().share("nvram");
}

void norautp_state::drhl_map(address_map &map)
{
	map.global_mask(0x7fff);  // A15 not connected
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x53ff).ram().share("nvram");
	map(0x5400, 0x57ff).ram();
}

void norautp_state::krampcb4_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xa000, 0xa7ff).ram().share("nvram");
//  map(0xff00, 0xffff).ram();
}


void norautp_state::dphla_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x63).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0, 0xa3).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc0, 0xc3).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xef, 0xef).r(FUNC(norautp_state::test3_r));
}


void norautp_state::dphl_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x7c, 0x7f).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xbc, 0xbf).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xdc, 0xdf).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xef, 0xef).r(FUNC(norautp_state::test3_r));
}

void norautp_state::drhl_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x7c, 0x7f).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xbc, 0xbf).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xdc, 0xdf).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xef, 0xef).r(FUNC(norautp_state::test4_r));
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( norautp )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet / Collect")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )     PORT_CODE(KEYCODE_F) PORT_NAME("Change Card")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Half Gamble / Save")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)  // Coin C

	PORT_START("IN2")   // Only 3 lines: PPI-2; PC0-PC2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_J) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Readout")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_L) PORT_NAME("Low Level Hopper")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Random Raise Bet" )    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )    PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, "A=5; B=25; C=1" )
	PORT_DIPSETTING(    0x00, "A=50; B=25; C=5" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Payment" )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Set Value" )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, "2 Pence" )
	PORT_DIPSETTING(    0x00, "10 Pence" )
INPUT_PORTS_END


static INPUT_PORTS_START( norautrh )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet / Change Card")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin B
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Readout")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Stand / Take")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Save / Half Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   // Coin C for other games

	PORT_START("IN2")   // Only 3 lines: PPI-2; PC0-PC2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	// to find the function...
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )           PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, "1" )           PORT_CONDITION("DSW1", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "20" )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "5 " )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x00, "25" )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x80)
	PORT_DIPNAME( 0x08, 0x08, "Raise Ante" )        PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, "Random" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x10, 0x00, "Type of Game" )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, "Jacks Plus" )
	PORT_DIPSETTING(    0x00, "Joker Poker" )
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:3,1")
	PORT_DIPSETTING(    0x00, "A=1; B=5" )
	PORT_DIPSETTING(    0x20, "A=10; B=5" )
	PORT_DIPSETTING(    0xa0, "A=5; B=25" )
	PORT_DIPSETTING(    0x80, "A=50; B=25" )
	PORT_DIPNAME( 0x40, 0x00, "Show Bet")           PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( norautpn )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet / Change Card")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin B
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Readout")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Stand / Take")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Save / Half Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )

	PORT_START("IN2")   // Only 3 lines: PPI-2; PC0-PC2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, "A=1; B=5" )
	PORT_DIPSETTING(    0x00, "A=50; B=5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mainline )

	PORT_INCLUDE( norautrh )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:3,1")
	PORT_DIPSETTING(    0x00, "A=1; B=10" )
	PORT_DIPSETTING(    0xa0, "A=1; B=25" )
	PORT_DIPSETTING(    0x20, "A=10; B=10" )
	PORT_DIPSETTING(    0x80, "A=50; B=25" )
INPUT_PORTS_END


static INPUT_PORTS_START( ndxron10 )

	PORT_INCLUDE( norautpn )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, "A=1; B=5" )
	PORT_DIPSETTING(    0x00, "A=1; B=25" )
INPUT_PORTS_END


static INPUT_PORTS_START( norautkl )

	PORT_INCLUDE( norautrh )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Bet Max" )               PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, "25" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Raise Ante" )            PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "Random" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown )  )     PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Type of Game" )          PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, "Deluxe Poker" )
	PORT_DIPSETTING(    0x00, "Joker Poker" )
	PORT_DIPNAME( 0x20, 0x20, "Min Bet for Jokers" )    PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x00, "Deal Speed")             PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, "A=5; B=2" )
	PORT_DIPSETTING(    0x80, "A=1; B=1" )
INPUT_PORTS_END


static INPUT_PORTS_START( cdrawpkr )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Half Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Meter Mode")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN1-80")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-01")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-02")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-04")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( noraut3 )

	PORT_INCLUDE( norautp )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )    PORT_NAME("Bet / Change Card")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )      PORT_NAME("Settings")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )   PORT_NAME("Take / Save")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Raise Bet" )             PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Type" )             PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, "Jacks Plus" )
	PORT_DIPSETTING(    0x00, "Joker Poker" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Show Bet" )              PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( norautua )

	PORT_INCLUDE( norautrh )

	PORT_MODIFY("DSW1")
	// to find the function...
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )           PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, "1" )           PORT_CONDITION("DSW1", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "20" )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "5 " )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x00, "25" )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x80)
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:3,1")
	PORT_DIPSETTING(    0x00, "A=1; B=5" )
	PORT_DIPSETTING(    0x20, "A=10; B=5" )
	PORT_DIPSETTING(    0x80, "A=5; B=25" )
	PORT_DIPSETTING(    0xa0, "A=50; B=25" )
INPUT_PORTS_END


static INPUT_PORTS_START( nor6700a )

	PORT_INCLUDE( norautp )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "WinBank Mode" )          PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Show Hopper Counter" )   PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Skill Points" )          PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, "A=50; B=25; C=5" )
	PORT_DIPSETTING(    0x00, "A=50; B=25; C=100" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Payment" )        PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nor6700b )

	PORT_INCLUDE( norautp )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "WinBank Mode" )          PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Show Hopper Counter" )   PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Skill Points" )          PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Payment" )        PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ddellf97 )

	PORT_INCLUDE( norautrh )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Bet Max" )               PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, "25" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bonus 5-Cards (D-UP)" )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( No   ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Raise Bet" )             PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Bonus" )                 PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, "77's, 99's" )
	PORT_DIPSETTING(    0x00, "77's, 99's, & Suits" )
	PORT_DIPNAME( 0x20, 0x20, "Min Bet for Jokers" )    PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x00, "Deal Speed")             PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bankpkr )

	PORT_INCLUDE( norautp )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// bets change with the 10p coinage
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )             PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )    PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, "A=50; B=25; C=5" )
	PORT_DIPSETTING(    0x00, "A=10; B=5; C=1" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Payment" )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Credits In" )          PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, "Points" )
	PORT_DIPSETTING(    0x00, "£Bank" )
INPUT_PORTS_END

static INPUT_PORTS_START( delv18jp )

	PORT_INCLUDE( norautrh )

	PORT_MODIFY("DSW1")
	// to find the function...
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )           PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, "1" )           PORT_CONDITION("DSW1", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "20" )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "5 " )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(    0x00, "25" )          PORT_CONDITION("DSW1", 0x80, EQUALS, 0x80)
	PORT_DIPNAME( 0x08, 0x08, "Raise Ante" )        PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, "Random" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x10, 0x00, "Type of Game" )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, "Jacks Plus" )
	PORT_DIPSETTING(    0x00, "Joker Poker" )
	PORT_DIPNAME( 0xa0, 0x20, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:3,1")
	PORT_DIPSETTING(    0x00, "A=1; B=10" )
	PORT_DIPSETTING(    0xa0, "A=1; B=25" )
	PORT_DIPSETTING(    0x20, "A=10; B=10" )
	PORT_DIPSETTING(    0x80, "A=50; B=25" )
	PORT_DIPNAME( 0x40, 0x00, "Show Bet")           PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( delv18ap )

	PORT_INCLUDE( norautrh )

	PORT_MODIFY("DSW1")
	// to find the function...
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x04, "Bet Max" )           PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:2,1")
	PORT_DIPSETTING(    0x40, "A=1; B=2" )
	PORT_DIPSETTING(    0x00, "A=1; B=5" )
	PORT_DIPSETTING(    0xc0, "A=1; B=5" )
	PORT_DIPSETTING(    0x80, "A=1; B=25" )
INPUT_PORTS_END


// ---------------- i8080 sets ------------------------------

static INPUT_PORTS_START( dphl )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )       PORT_NAME("Deal")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Bet / Collect")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A  OK
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("IN0-5") PORT_CODE(KEYCODE_K)  // give "ERROR" in some games
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Stand")PORT_CODE(KEYCODE_D)  // Stand: keep the hand
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Draw")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )

	PORT_START("IN2") // unused
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )  // PPI set for PORT C all outputs

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Cards to discard" )    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, "All 5 cards" )
	PORT_DIPSETTING(    0x00, "Max 3 cards" )
	PORT_DIPNAME( 0x60, 0x60, "2 Jokers in Deck" )    PORT_DIPLOCATION("DSW1:2,3")
	PORT_DIPSETTING(    0x60, "Never"  )
	PORT_DIPSETTING(    0x40, "When 8 or more is bet" )
	PORT_DIPSETTING(    0x20, "When 4 or more is bet" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( dphla )

	PORT_INCLUDE( dphl )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
INPUT_PORTS_END


static INPUT_PORTS_START( drhl )

	PORT_INCLUDE( dphla )

	PORT_MODIFY("DSW1")
	// to find the function...
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Test Mode" )           PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )             PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPSETTING(    0x30, "30" )
	PORT_DIPSETTING(    0x20, "30" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )    PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x80, 0x80, "Bonus" )               PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ssjkrpkr )

	PORT_INCLUDE( dphla )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Cards to Discard" )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, "All 5 cards" )
	PORT_DIPSETTING(    0x00, "Max 3 cards" )
	PORT_DIPNAME( 0x04, 0x00, "Double-Up" )           PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" )             PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPSETTING(    0x10, "25" )

	PORT_DIPNAME( 0x20, 0x00, "Break Game (set ON)" ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )    PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sureshoto )

	PORT_INCLUDE( dphl )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )       PORT_NAME("Break (Deal)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused bet
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused coin 2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused hi
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused lo

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Safe (Stand)") PORT_CODE(KEYCODE_D)  // Stand: keep the hand
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Shoot (Draw)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Scratch (Hold) 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Scratch (Hold) 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Scratch (Hold) 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Scratch (Hold) 4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Scratch (Hold) 5")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "2 Wild Cues" )         PORT_DIPLOCATION("DSW1:2,3")
	PORT_DIPSETTING(    0x60, "Never"  )
	PORT_DIPSETTING(    0x40, "When 4 or more is bet" )
	PORT_DIPSETTING(    0x20, "When 4 or more is bet" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fastdrwp )

	PORT_INCLUDE( dphl )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// number of cards to discard in other games.
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gtipoker )

	PORT_INCLUDE( dphl )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Minimal Hand Paid" )   PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, "Jacks or Better" )
	PORT_DIPSETTING(    0x10, "Aces or Better" )
	PORT_DIPNAME( 0x60, 0x60, "2 Jokers in Deck" )    PORT_DIPLOCATION("DSW1:2,3")
	PORT_DIPSETTING(    0x60, "Never"  )
	PORT_DIPSETTING(    0x40, "When 4 or more is bet" )
	PORT_DIPSETTING(    0x20, "When 4 or more is bet" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )           PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( newhilop )

	PORT_INCLUDE( dphl )

	PORT_MODIFY("DSW1")
	// number of cards to discard in other games.
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tpoker2 )

	PORT_INCLUDE( dphl )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Coupon" )              PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )     PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Type" )           PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, "Nickle" )
	PORT_DIPSETTING(    0x00, "Quarter" )
	PORT_DIPNAME( 0x08, 0x08, "Ticket" )              PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, "10 Ticket" )
	PORT_DIPSETTING(    0x00, "5 Ticket" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown Item 1" )      PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, "No Limit" )
	PORT_DIPSETTING(    0x10, "1 per Game" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown Item 2" )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, "DL-1275" )
	PORT_DIPSETTING(    0x00, "DL-4-PS" )
	PORT_DIPNAME( 0x40, 0x40, "Mode" )                PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, "Play Mode" )
	PORT_DIPSETTING(    0x00, "Stats Mode" )
	PORT_DIPNAME( 0x80, 0x80, "Setup" )               PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tpoker2a )

	PORT_INCLUDE( dphl )

//  PORT_MODIFY("IN0")  // in case of need ticket/hopper
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )     PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )     PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Type" )           PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, "Nickle" )
	PORT_DIPSETTING(    0x00, "Quarter" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus" )              PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Replays" )             PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "XCards" )              PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Mode" )                PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, "Play Mode" )
	PORT_DIPSETTING(    0x00, "Stats Mode" )
	PORT_DIPNAME( 0x80, 0x80, "Setup" )               PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout charlayout =

//  Reverse-engineered 2 Narout Poker PCBs to analyze the hardware's
//  tile decoding method for 16x32 graphics. The following GFX layout
//  has been verified as 100% accurate
{
	16, 32,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, 0*16, },
	16*16
};

static const gfx_layout charlayout32x32 =
{
	32, 32,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0,0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 7,7, 8,8, 9,9, 10,10, 11,11, 12,12, 13,13, 14,14, 15,15 },
	{ 0*16, 0*16, 1*16, 1*16, 2*16, 2*16, 3*16, 3*16, 4*16, 4*16, 5*16, 5*16, 6*16, 6*16, 7*16, 7*16,
		8*16, 8*16, 9*16, 9*16, 10*16,10*16,11*16,11*16,12*16,12*16,13*16,13*16,14*16,14*16,15*16,15*16 },
	16*16
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

// GFX are stored in the 2nd half... Maybe the HW could handle 2 bitplanes?
static GFXDECODE_START( gfx_norautp )
	GFXDECODE_ENTRY( "gfx", 0x800, charlayout,      0, 256 )
	GFXDECODE_ENTRY( "gfx", 0x800, charlayout32x32, 0, 256 )
GFXDECODE_END


/**************************************************
*             Discrete Sound Hardware             *
**************************************************/

static const discrete_555_desc desc_norautp_555 =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_AC,
	5,              // B+ voltage of 555
	DEFAULT_555_VALUES
};


#define NORAUTP_SOUND_CIRCUIT(_name, _r1, _r2, _c1, _c2, _c3, _c4)          \
static const discrete_comp_adder_table desc_##_name##_caps =                \
{                                                                           \
	DISC_COMP_P_CAPACITOR, _c4, 3,                                          \
	{                                                                       \
		_c3, _c2, _c1                                                       \
	}                                                                       \
};                                                                          \
																			\
DISCRETE_SOUND_START( _name##_discrete )                                    \
	/************************************************                       \
	 * Input register mapping                                               \
	 ************************************************/                      \
	DISCRETE_INPUT_LOGIC(NORAUTP_SND_EN)                                    \
	DISCRETE_INPUT_DATA (NORAUTP_FREQ_DATA)                                 \
																			\
	/************************************************                       \
	 * Tone Generator                                                       \
	 ************************************************/                      \
	DISCRETE_COMP_ADDER(NODE_20, NORAUTP_FREQ_DATA, &desc_##_name##_caps)   \
	DISCRETE_555_ASTABLE(NODE_21,                                           \
							NORAUTP_SND_EN,                 /* RESET */     \
							_r2, _r1, NODE_20, &desc_norautp_555)           \
																			\
	DISCRETE_OUTPUT(NODE_21, 65000.0/3.8)                                   \
DISCRETE_SOUND_END


/***** Noraut Poker *****/

// Parts List - Resistors
#define NORAUTP_R1      RES_K(120)
#define NORAUTP_R2      RES_K(2.2)

// Parts List - Capacitors
#define NORAUTP_C1      CAP_U(.01)
#define NORAUTP_C2      CAP_U(.022)
#define NORAUTP_C3      CAP_U(.047)
#define NORAUTP_C4      CAP_U(.01)

NORAUTP_SOUND_CIRCUIT(norautp,
	NORAUTP_R1, NORAUTP_R2,
	NORAUTP_C1, NORAUTP_C2, NORAUTP_C3, NORAUTP_C4)


/***** Draw Poker HI-LO *****/

// Parts List - Resistors
#define DPHL_R1     RES_K(120)
#define DPHL_R2     RES_K(1)

// Parts List - Capacitors
#define DPHL_C1     CAP_U(.01)
#define DPHL_C2     CAP_U(.022)
#define DPHL_C3     CAP_U(.05)
#define DPHL_C4     CAP_U(.01)

NORAUTP_SOUND_CIRCUIT(dphl,
	DPHL_R1, DPHL_R2,
	DPHL_C1, DPHL_C2, DPHL_C3, DPHL_C4)


/***** Kimble Double HI-LO *****/

// Parts List - Resistors
#define KIMBLE_R1       RES_K(100)
#define KIMBLE_R2       RES_K(1)

// Parts List - Capacitors
#define KIMBLE_C1       CAP_U(.01)
#define KIMBLE_C2       CAP_U(.022)
#define KIMBLE_C3       CAP_U(.047)
#define KIMBLE_C4       CAP_U(.01)

NORAUTP_SOUND_CIRCUIT(kimble,
	KIMBLE_R1, KIMBLE_R2,
	KIMBLE_C1, KIMBLE_C2, KIMBLE_C3, KIMBLE_C4)


/*********************************************
*              Machine Drivers               *
*********************************************/

void norautp_state::noraut_base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, NORAUT_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::norautp_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::norautp_portmap);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I8255(config, m_ppi8255[0], 0);
	// (60-63) Mode 0 - Port A set as input
	m_ppi8255[0]->in_pa_callback().set_ioport("DSW1");
	m_ppi8255[0]->out_pb_callback().set(FUNC(norautp_state::mainlamps_w));
	m_ppi8255[0]->out_pc_callback().set(FUNC(norautp_state::counterlamps_w));

	I8255(config, m_ppi8255[1], 0);
	// (a0-a3) Mode 0 - Ports A & B set as input
	m_ppi8255[1]->in_pa_callback().set_ioport("IN0");
	m_ppi8255[1]->in_pb_callback().set_ioport("IN1");
	m_ppi8255[1]->out_pc_callback().set(FUNC(norautp_state::soundlamps_w));

	I8255(config, m_ppi8255[2], 0);
	m_ppi8255[2]->out_pb_callback().set(FUNC(norautp_state::ppi2_b_w));
	// (c0-c3) Group A Mode 2 (5-lines handshacked bidirectional port).
	// PPI-2 is configured as mixed mode2 and mode0 output.
	// It means that port A should be bidirectional and port B just as output.
	// Port C high as hshk regs, and PC0-PC2 as input (norautp, norautjp) or output (other sets)
	// Group B Mode 0, output;  (see below for lines PC0-PC2)
	m_ppi8255[2]->in_pc_callback().set_ioport("IN2");
	m_ppi8255[2]->out_pc_callback().set(FUNC(norautp_state::ppi2_obf_w)).bit(7);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*16, 32*16);
	m_screen->set_visarea(2*16, 31*16-1, (0*16) + 8, 16*16-1);    // the hardware clips the top 8 pixels
	m_screen->set_screen_update(FUNC(norautp_state::screen_update_norautp));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_norautp);
	PALETTE(config, m_palette, FUNC(norautp_state::norautp_palette), 16);

	HOPPER(config, m_hopper, attotime::from_msec(150));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, norautp_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void norautp_state::norautp(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
}

void norautp_state::norautu(machine_config &config)
{
	norautp(config);

	m_maincpu->set_addrmap(AS_OPCODES, &norautp_state::decrypted_opcodes_map);
}

void norautp_state::norautpl(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	// sound hardware
	m_discrete->set_intf(kimble_discrete);
}

void norautp_state::norautxp(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::norautxp_map);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
}

void norautp_state::norautx4(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::norautx4_map);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
}

void norautp_state::noraut3(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::noraut3_map);
	m_maincpu->set_addrmap(AS_OPCODES, &norautp_state::noraut3_decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
}

void norautp_state::kimble(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::kimble_map);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	// sound hardware
	m_discrete->set_intf(kimble_discrete);
}

void norautp_state::cgidjp(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::cgidjp_map);
	m_maincpu->set_addrmap(AS_OPCODES, &norautp_state::cgidjp_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
}

void norautp_state::cdrawpkr(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
	m_screen->set_visarea(5*8, 61*8-1, (0*16) + 8, 16*16-1);
}


/************** i8080 based **************/

void norautp_state::dphl(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::dphl_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::dphl_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
	m_screen->set_screen_update(FUNC(norautp_state::screen_update_dphl));

	PALETTE(config.replace(), "palette", FUNC(norautp_state::bp_based_palette), 256);

	TIMER(config, "test_timer").configure_periodic(FUNC(norautp_state::test_timer_cb), attotime::from_usec(100));

	m_display_line_control = true;

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}


void norautp_state::nortest1(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::nortest1_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::dphl_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	TIMER(config, "test_timer").configure_periodic(FUNC(norautp_state::test_timer_cb), attotime::from_usec(100));

	m_display_line_control = true;
}


void norautp_state::dphla(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::dphla_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::dphla_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}

void norautp_state::dphlxtnd(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::dphlxtnd_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::dphl_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
	m_screen->set_screen_update(FUNC(norautp_state::screen_update_dphl));

	PALETTE(config.replace(), "palette", FUNC(norautp_state::bp_based_palette), 256);

	TIMER(config, "test_timer").configure_periodic(FUNC(norautp_state::test_timer_cb), attotime::from_usec(100));

	m_display_line_control = true;

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}

void norautp_state::gtipkra(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::gtipa_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::dphl_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}


void norautp_state::kimbldhl(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::kimbldhl_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::norautp_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	// sound hardware
	m_discrete->set_intf(kimble_discrete);
}

void norautp_state::tpoker2(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::tpoker2_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::norautp_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
	m_screen->set_screen_update(FUNC(norautp_state::screen_update_dphl));

	PALETTE(config.replace(), "palette", FUNC(norautp_state::bp_based_palette), 512);

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}

void norautp_state::drhl(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::drhl_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::drhl_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	PALETTE(config.replace(), "palette", FUNC(norautp_state::drhl_palette), 16);

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}

void norautp_state::ssjkrpkr(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::ssjkrpkr_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::norautp_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}

void norautp_state::krampcb4(machine_config &config)
{
	noraut_base(config);

	// basic machine hardware
	I8080(config.replace(), m_maincpu, DPHL_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &norautp_state::krampcb4_map);
	m_maincpu->set_addrmap(AS_IO, &norautp_state::dphl_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(norautp_state::irq0_line_hold));
	m_screen->set_screen_update(FUNC(norautp_state::screen_update_dphl));

	PALETTE(config.replace(), "palette", FUNC(norautp_state::bp_based_palette), 256);

	TIMER(config, "test_timer").configure_periodic(FUNC(norautp_state::test_timer_cb), attotime::from_usec(100));

	m_display_line_control = true;

	// sound hardware
	m_discrete->set_intf(dphl_discrete);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

//*************************************** Z80 sets ***************************************
//*                                                                                      *
//*   The following ones are 'Draw Poker HI-LO' type, running in a Z80 based hardware    *
//*                                                                                      *
//****************************************************************************************

/*

  Noraut Poker.
  -------------

  The First one released by Noraut.

  Hardware Layout:

  1x Z80
  3x PPI 8255
  2x 6116 SRAM
  1x 3.6 Vcc Battery.
  1x 18.432 MHz. Xtal.

  1x 555 + unknown yellow resonator, near the edge connector.
  1x 555 + resnet, near the battery.

  1x 10 DIP switches bank.
  2x 3pins jumpers (between the Z80 and ROM)

     JP1 (ABC);  JP2 (DEF)

  PCB silksceened:  AB+DE=512  BC+DE=256
                    (CUT BC)   EF=64/128

  PCB silksceened:  SMART-BOARD 131191 ISS.E (Made in USA)


  Noraut Poker discrete audio circuitry
  -------------------------------------

  3x ULN2003A (Darlington transistor array)
  1x D5555C   (CMOS timer)
  1x KN2222A  (Epitaxial planar general purpose NPN transistor)
  1x VR/POT

  .------.                              .------------.               .-------.
  |      |                              |   D5555C   |               |KN2222A|
  |      |                             4|            |3     R3       |       |      -->
  |   PC7|------------------------------|RST      OUT|-----ZZZZ------|B     E|>----ZZZZZ-----> Audio Out.
  |   PC6|----------.                  6|            |8              |   C   |      VR1
  |   PC5|-----.    |2-in         .-----|THR      VCC|-----------.   '---+---'          .----> Audio Out.
  |   PC4|--.  |  .-+------.      |    5|            |7          |       |              |
  |      |  |  |  |ULN2003A|      |  .--|CVOLT   DISC|--.        |       |              |
  |      |  |  |  '-+------'      |  |  |            |  |        +-------'            --+--
  |      |  |  |    |2-out   C1   |  |  |    GND     |  |        |                     GND
  | 8255 |  |  |    '--------||---+  |  '-----+------'  |        |                      -
  |  AP  |  |  '--.               |  |        |1        |        |                      '
  |      |  |     |3-in           |  |        |         |        |
  '------'  |  .--+-----.         |  |   C5   |         |        |
            |  |ULN2003A|         |  '---||---+         |        |    +5V
            |  '--+-----'         |           |         |        |    -+-
            |     |3-out     C2   |      C4   |         |    C6  |     |
            |     '----------||---+------||---+-------. | .--||--+-----'
            |2-in                 |           |       | | |      |
          .-+------.              |         --+--      '-'       |
          |ULN2003A|              |          GND        |        |
          '-+------'              |           -         |        |
            |2-out           C3   |     R1    '         |   R2   |
            '----------------||---+----ZZZZ-------------+--ZZZZ--'

  VR1 = 100 ohms

  R1 = 120 K ; Tolerance +/- 5%
  R2 = 2.2 K ; Tolerance +/- 5%
  R3 = 1 K   ; Tolerance +/- 5%

  C1 = 103J = 10000 pF  =  10 nF = 0.01 uF  ; Tolerance +/- 5%
  C2 = 223J = 22000 pF  =  22 nF = 0.022 uF ; Tolerance +/- 5%
  C3 = 473J = 47000 pF  =  47 nF = 0.047 uF ; Tolerance +/- 5%
  C4 = 103J = 10000 pF  =  10 nF = 0.01 uF  ; Tolerance +/- 5%

  C5 = 103  = 10000 pF  =  10 nF = 0.01 uF
  C6 = 104  = 100000 pF = 100 nF = 0.1 uF

  - C1, C2, C3 & C4 are polyester film / mylar capacitors.
  - C5 & C6 are ceramic capacitors.
  - All Capacitors are non-polarised.

*/

ROM_START( norautp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpoker.bin",     0x0000,  0x2000,  CRC(e22ed34d) SHA1(108f034335b5bed183ee316a61880f7b9485b34f) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "displayrom.bin", 0x00000, 0x10000, CRC(ed3605bd) SHA1(0174e880835815558328789226234e36b673b249) )
ROM_END

/*

  Noraut Deluxe Poker (console).
  From PCB with daughterboard.

  ----

  - Noraut deluxe 24 pin z80.bin

  Norauts original or first launch into the market.
  No Jokers, lose on same card in gamble.

  - Noraut deluxe 24pin console.bin

  Requires daughterboard console in z80 to run.
  Same as Noraut 24 pin chip with z80, but more stable.

*/

ROM_START( norautdx )
	ROM_REGION( 0x10000, "maincpu", 0 )  // console version
	ROM_LOAD( "noraut_deluxe_24pin_console.bin", 0x0000, 0x1000, CRC(d41bd404) SHA1(52e984ca28a15a1485ca672dd1fef973cf0c7617) )
	ROM_LOAD( "noraut_deluxe_24pin_z80.bin",     0x1000, 0x0800, CRC(c70bc8f9) SHA1(d947be4e6741f3a884ceca76d1a0fd13625a5f78) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  Poker game - manufacturer unknown.
  Seems a bootleg of Noraut Deluxe poker.

  Z80 CPU

  Program rom = 2764 (2nd Half blank)
  Character rom = 2732

  18.432 Mhz crystal

  sound discrete with ne555 timer chip (located near amp/volume control).

*/

ROM_START( norautpn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prog.bin",   0x0000, 0x2000, CRC(8b1cfd24) SHA1(d673baed1c1e5b54a34b7a5857b269a725737e92) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "char.bin",   0x0000, 0x1000, CRC(955eac6f) SHA1(470d8bad1a5d2a0a08dd129e6393c3c3a4ef2159) )
ROM_END

/*

  Noraut Joker Poker (original)

  Noraut 4bet joker chip U1 24pin z80.
  Noraut 4bet joker chip U16 24pin z80.

  ----

  Above two program chips used in old boards
  with 2x 32k program sockets.

*/

ROM_START( norautjo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "noraut4betjoker.u1",  0x0000, 0x1000, CRC(082f28c6) SHA1(995b991dad50373c8ba9b63002f124b3f87e7889) )
	ROM_LOAD( "noraut4betjoker.u16", 0x1000, 0x1000, CRC(1452cac3) SHA1(ddc4b195c3bd8a3ff56f5c7da050d6136e442323) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

// portuguese hack. same as above but hacked text and stripped out copyright.
ROM_START( norautpt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u11.bin",  0x0000, 0x1000, CRC(082f28c6) SHA1(995b991dad50373c8ba9b63002f124b3f87e7889) )
	ROM_LOAD( "u16.bin",  0x1000, 0x1000, CRC(4d970e12) SHA1(acbb0fbb316283c57ae8023721aa4da3ca7e1319) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "r20.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  Noraut Joker Poker (Prologic HW)


  - CPU:      1x Z084004PSC (Frequency measured 2.3025MHz.)
  - RAM:      4x 2114
  - I/O:      3x 8255 Peripeheral Interface Adapter.
  - Prg ROM:  2x 2732, U11,U16
  - Gfx ROM:  1x 2716 U27 Eprom
  - Sound:    Discrete.
  - Crystal:  1x 18.432 MHz


  NOTE: PIN NO 5 LIFTED FROM CPU SOCKET (A15)
  PCB DOES NOT BOOT IF PIN IS PUT BACK IN SOCKET.

  PCB silksceened:
  PROLOGIC MADE IN (IRL) - 131191


  PCB Layout (PROLOGIC):                                                       Edge Connector 36x2
  .----------------------------------------------------------------------------------------------.
  | .-----.                .---------.  .---------.  .-----.        .........   .---------.      |
  | | 555 |                |74LS174N |  |74LS153N |  | 555 |        .........   |ULN2003A |      |
  | '-----'                '---------'  '---------'  '-----'        D16-A-2K2   '---------'      |
  |   U46                      U45          U44        U43             U42          U41          |
  |                                                                                              |
  |              DIP SW x4                                                                       |
  | .--------.   .-.-.-.-.                                                                       |
  | |Battery |   |1|2|3|4| .---------.  .---------.  .---------.  .---------.   .---------.      |
  | |  3.6v  |   '-'-'-'-' |74LS157N |  |74LS153N |  |74LS161AP|  |74LS86AN |   |ULN2003A |      |
  | '--------'      U40    '---------'  '---------'  '---------'  '---------'   '---------'      |
  |                            U39          U38          U37          U36           U35          |
  |                                                                                              |
  |                                                                                              | 36
  | .---------.            .---------.   .........   .---------.  .---------.   .---------.      '---.
  | | 74LS04  |            |74LS166AP|   .........   |74LS161AN|  |74LS153N |   |ULN2003A |       ---|
  | '---------'            '---------'   D16-A-2K2   '---------'  '---------'   '---------'       ---|
  |     U34                    U33          U32          U31          U30           U29           ---|
  |                                                                           .----------------.  ---|
  |              .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |   D8255AC-2    |  ---|
  |  .---------. |    2716     |  |1|2|3|4|5|6|7|8|  |74LS161AN|  |74LS157N | |                |  ---|
  |  |  2114   | |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |  '---------' '-------------'     DIP SW x 8          U25          U24            U23          ---|
  |      U28           U27               U26                                                      ---|
  |                                                                           .----------------.  ---|
  |                                                                           |   D8255AC-2    |  ---|
  |                                                                           |                |  ---|
  |                                                                           '----------------'  ---|
  |   .---------. .---------. .---------.                                            U17          ---|
  |   |  2114   | |  2114   | |  2114   |            .---------.  .---------.                     ---|
  |   '---------' '---------' '---------'            |74LS161AN|  |74LS157N |                     ---|
  |       U22         U21         U20                '---------'  '---------'                     ---|
  |                                                      U19          U18                         ---|
  | .--------------.     .----------------.                                                       ---|
  | |     2732     |     |   D8255AC-2    |          .---------.  .---------.                     ---|
  | |              |     |                |          |74LS161AN|  |74LS157N |     .........       ---|
  | '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |       U16                   U15                      U14          U13         D16-A-2K2      .---'
  | .--------------.                                                                  U12        |
  | |     2732     |                                                                             | 1
  | |              |    .---------.                  .---------.  .---------.  .---------.       |
  | '--------------'    | 74LS32N |                  |74LS161AN|  | 74LS86P |  | 74LS04N |       |
  |       U11           '---------'                  '---------'  '---------'  '---------'       |
  |                         U10                           U9          U8           U7            |
  |                                             XTAL                                             |
  |                                            .----.                                            |
  | .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  | |     Z084004PSC     |   | 74LS138N |    | 74LS04N |  |74LS157N |  | 74LS11N |  |74LS74AN |  |
  | |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  | '--------------------'       U5               U4           U3           U2           U1      |
  |          U5                                                                                  |
  '----------------------------------------------------------------------------------------------'


  Discrete audio circuitry: SAME AS KIMBLE DIAGRAM.

*/

ROM_START( norautpl )
	ROM_REGION( 0x10000, "maincpu", 0 )  // slightly different than original JP
	ROM_LOAD( "u11.bin",  0x0000, 0x1000, CRC(2abd1b82) SHA1(8cbe9ea481ec2465faaf79fcfc22ec78d83bd98d) )
	ROM_LOAD( "u16.bin",  0x1000, 0x1000, CRC(dbc3960a) SHA1(d58ee89134f9d8db80d3e066fd01e4e484126d00) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(             0x0000, 0x0800, 0xff )
	ROM_LOAD( "char.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END


/*

  Noraut Joker Poker

  Hardware Layout:

  - CPU:             1x TMPZ84C00AP-8
  - RAM:             2x HM6116LP-4 CMOS Static Ram
  - I/O:             3x D8255AC-2 Peripeheral Interface Adapter
  - Prg ROMs:        1x 2764 Eprom
  - Gfx ROMs:        1x 2732 Eprom
  - Sound:           Discrete
  - Battery:         1x 3.6v Ni-cd 65Mah
  - Crystal:         1x 18.432Mhz
  - Resistor Array:  4x 9 Pin SIP 472G

  Program:27C64
  Marked:
  "MX10A JOKER G.L
   N.C.R  C.C  M.S"

  Char:2732
  Marked:
  "N1-057-5"


  PCB Layout (norautjp):                                                      Edge Connector 36x2
  .----------------------------------------------------------------------------------------------.
  | .-----.                .---------.  .---------.  .-----.        .........   .---------.      |
  | |D5555|                |74LS174N |  |74LS153N |  |D5555|        .........   |ULN2003A |      |
  | '-----'                '---------'  '---------'  '-----'    Resistor Array  '---------'      |
  |                                                              .---.                           |
  |                                                              |VR1|                           |
  |              DIP SW x4                                       '---'                           |
  | .--------.   .-.-.-.-.                                                                       |
  | |Battery |   |1|2|3|4| .---------.  .---------.  .---------.  .---------.   .---------.      |
  | |  3.6v  |   '-'-'-'-' |74LS157N |  |74LS153N |  |74LS161AP|  |74LS86AN |   |ULN2003A |      |
  | '--------'             '---------'  '---------'  '---------'  '---------'   '---------'      |
  |                                                                                              |
  |                                                                                              |
  |                                                                                              | 36
  | .---------.            .---------.               .---------.  .---------.   .---------.      '---.
  | |HD74LS04P|            |74LS166AP|               |74LS161AN|  |74LS153N |   |ULN2003A |       ---|
  | '---------'            '---------'               '---------'  '---------'   '---------'       ---|
  |                                                                                               ---|
  |                                   DIP SW x 8                              .----------------.  ---|
  |              .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |   D8255AC-2    |  ---|
  |              |    2732     |  |1|2|3|4|5|6|7|8|  |74LS161AN|  |74LS157N | |                |  ---|
  |              |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |              '-------------'                                                                  ---|
  |                                                                                               ---|
  |                                                                           .----------------.  ---|
  |                                                                           |   D8255AC-2    |  ---|
  | .-------------.      .-------------.                                      |                |  ---|
  | |    6116     |      |    6116     |                                      '----------------'  ---|
  | |             |      |             |                                                          ---|
  | '-------------'      '-------------'             .---------.  .---------.                     ---|
  |                                                  |74LS161AN|  |74LS157N |                     ---|
  |                                                  '---------'  '---------'                     ---|
  |                                                                                               ---|
  | .--------------.     .----------------.                                                       ---|
  | |     2764     |     |   D8255AC-2    |          .---------.  .---------.                     ---|
  | |              |     |                |          |74LS161AN|  |74LS157N |     .........       ---|
  | '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |                                                                              Resistor Array  .---'
  |                                                                                              | 01
  |                                                                                              |
  |                     .---------.                  .---------.  .---------.  .---------.       |
  |                     | 74LS32N |                  |74LS161AN|  | 74LS86P |  | 74LS04N |       |
  |                     '---------'                  '---------'  '---------'  '---------'       |
  |                                                                                              |
  |                                             XTAL                                             |
  |                                            .----.                                            |
  | .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  | |   TMPZ84C00AP-8    |   |PALce16v8H|    | 74LS04N |  |74LS157N |  | 74LS11N |  |74LS74AN |  |
  | |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  | '--------------------'                                                                       |
  |                                                                                              |
  '----------------------------------------------------------------------------------------------'

*/

ROM_START( norautjp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2764-1prog.bin",   0x0000, 0x2000, CRC(5f776ce1) SHA1(673b8c67ebd5c1334187a9407b86a43150cbe67b) )

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "2732-1char.bin",   0x0800, 0x0800, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )  // first half 0xff filled
	ROM_CONTINUE(                 0x0800, 0x0800 )

	ROM_REGION( 0x800,  "nvram", 0 )
	ROM_LOAD( "norautjp_nv.bin",  0x0000, 0x0400, CRC(0a0614b2) SHA1(eb21b2723b41743daf787cfc379bc67cce2b8538) )  // default NVRAM

ROM_END

/*

  Noraut Red Hot Joker Poker.

  Red hot joker poker scrolls across screen.
  Eprom has Red Hot on sticker.
  Char: Handwritten sticker with "Club250 grapics" on it.

  Pressing the readout button brings you to a menu with RESET / READOUT.
  Pressing on Readout brings you to "coins in", "coins out" and "balance".

  No date info on board or found in rom.

*/

ROM_START( norautrh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "norautrh.bin",  0x0000, 0x2000, CRC(f5447d1a) SHA1(75d6439481e469e82e5561146966c9c7b44f34fe) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "club250.bin",   0x0000, 0x1000, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )
ROM_END

/*

  Noraut Red Hot 1-bet Joker Poker.

  Last release of Noraut's Joker Poker. Always have joker with multi change card
  and hint card in gamble. Quite popular in Norther Ireland but some operators
  have the percentage to payout at less than 10%

*/

ROM_START( norautra )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Program ROM is 0000-1fff and identical to norautrh, the rest is filled with FF's
	ROM_LOAD( "noraut_red_hot_1bet_joker_poker.bin", 0x0000, 0x8000, CRC(f284b574) SHA1(ff683731f3dbdaed5d0d25276ca90b68a422e403) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  Unknown Noraut: "NTX10A  V6"
  None working old noraut board with daughter card upgrade
  daughter card looks like an old upgrade PCB marked:
  "Noraut LTD Game Module"
  half of which is incased in epoxy resin.
  only thing not visble on this board compared to others i have is the cpu
  with is under the epoxy not sure what else is their.

  D Card contains:
  Backup Battery

  Program Eprom:27C256
  Marked: "NTX10A  V6"
           "C201"

  CPU:
  Unknown Incased in epoxy

  NVRAM: HY6264A

  PAL:
  PAL16L8ACN

  Charcter Eprom is mounted on main board
  CHAR Eprom:2732
  Marked: "GU27"

  Daughter card is connected on to another card containing only pcb tracks no components
  This second board connects to main board with ribbon cable to the 40pin socket where
  the original cpu would of been.

  No date info on board or found in rom.

*/

ROM_START( norautu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2563.bin",   0x0000, 0x8000, CRC(6cbe68bd) SHA1(93201baaf03a9bba6c52c64cc26e8e445aa6454e) )
	ROM_RELOAD(             0x8000, 0x8000 )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "club250.bin", 0x0000, 0x1000, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )
ROM_END


/*

  NORAUT V3.010a.

  Board upgraded with daughter card.
  Daughter card looks modern and is marked
  "memory expansion module"
  "Unique Ireland"

  D Card contains:
  Backup Battery

  Program Eprom:27C512
  Marked:
  "G45P A V3.010a GU27
   Euro 27C512 20MAR02"


  PAL:PAL16l8ANC
  Marked VER.2

  CPU:
  Zilog
  Z8400APS
  Z80 CPU

  NVRAM: 6116

  Two jumpers on card , game will not boot if these are removed or placed on other pins
  cabinet beeps and shows graphics on screen. Removing while game is on cause game to freeze.
  Unknown what their for.

  Charcter Eprom is mounted on main board
  CHAR Eprom:2716
  Marked "GU27"

  No date info found in rom. Program eprom sticker: "Euro 27C512 20MAR02"

  This version contains a hidden menu with lots of different options
  to access this menu you must hold the HI and LOW button and press the readout/test switch
  the screen will go blank then you release the 3 buttons and the menu appears.

  Pressing the readout button brings you to a menu with RESET / READOUT
  Pressing on Readout brings you to "coins in", "coins out" and "balance".

  The daughter card connects direct to main pcb through 40 pins into original cpu socket
  and 12 pins to one side of original program eprom.


  PCB Layout (V3.010A + V3.011A):                                             Edge Connector 36x2
  .----------------------------------------------------------------------------------------------.
  | .-----.                .---------.  .---------.  .-----.        .........   .---------.      |
  | | 555 |                |74LS174N |  |74LS153N |  | 555 |        .........   |ULN2003A |      |
  | '-----'                '---------'  '---------'  '-----'       898-1-R4.7K  '---------'      |
  |   U46                      U45          U44        U43             U42          U41          |
  |                                                                                              |
  |              DIP SW x4                                                                       |
  | .--------.   .-.-.-.-.                                                                       |
  | |Battery |   |1|2|3|4| .---------.  .---------.  .---------.  .---------.   .---------.      |
  | |  3.6v  |   '-'-'-'-' |74LS157N |  |74LS153N |  |74LS161AP|  |74LS86AN |   |ULN2003A |      |
  | '--------'      U40    '---------'  '---------'  '---------'  '---------'   '---------'      |
  |                            U39          U38          U37          U36           U35          |
  |                                                                                              |
  |                                                                                              | 36
  | .---------.            .---------.   .........   .---------.  .---------.   .---------.      '---.
  | | 74LS04  |            |74LS166AP|   .........   |74LS161AN|  |74LS153N |   |ULN2003A |       ---|
  | '---------'            '---------'  898-1-R470   '---------'  '---------'   '---------'       ---|
  |     U34                    U33          U32          U31          U30           U29           ---|
  |                                                                           .----------------.  ---|
  |              .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |   D8255AC-2    |  ---|
  |              |    2716     |  |1|2|3|4|5|6|7|8|  |74LS161AN|  |74LS157N | |                |  ---|
  |              |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |     NO IC    '-------------'     DIP SW x 8          U25          U24            U23          ---|
  |      U28           U27              U26                                                       ---|
  |                                                                           .----------------.  ---|
  |                                                                           |   D8255AC-2    |  ---|
  |                                                                           |                |  ---|
  |                                                                           '----------------'  ---|
  |               .---------. .---------.                                            U17          ---|
  |               |  2114   | |  2114   |            .---------.  .---------.                     ---|
  |      NO IC    '---------' '---------'            |74LS161AN|  |74LS157N |                     ---|
  |       U22         U21         U20                '---------'  '---------'                     ---|
  |                                                      U19          U18                         ---|
  |                      .----------------.                                                       ---|
  |                      |   D8255AC-2    |          .---------.  .---------.                     ---|
  |                      |                |          |74LS161AN|  |74LS157N |     .........       ---|
  |      NO IC           '----------------'          '---------'  '---------'     .........       ---|
  |       U16                   U15                      U14          U13        916C471X2PE     .---'
  |                                                                                  U12         | 01
  |                                                                                              |
  |                     .---------.                  .---------.  .---------.  .---------.       |
  |      NO IC          | 74LS32N |                  |74LS161AN|  | 74LS86P |  | 74LS04N |       |
  |       U11           '---------'                  '---------'  '---------'  '---------'       |
  |                         U10                XTAL       U9          U8           U7            |
  |                                           18.432                                             |
  |                                           .----.                                             |
  | .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  | |   EXPANSION PCB    |   |74LS138N  |    | 74LS04N |  |74LS157N |  | 74LS11N |  |74LS74AN |  |
  | |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  | '--------------------'       U5               U4           U3           U2           U1      |
  |          U5                                                                                  |
  '----------------------------------------------------------------------------------------------'


  EXPANSION PCB:
  .-----------------------------------.
  |                       .---.       |
  | .-------------.       | P |       |
  | | V30 27C512  |       | A |       |
  | |             |       | L |       |
  | '-------------'       |   |       |
  | *                     '---'       |
  | * J1                     .------. |
  | *                        |      | |
  | .--------------------.   |      | |
  | |                    |   | 6116 | |
  | |     Z084004PS      |   |      | |
  |*|                    |   |      | |
  |*'--------------------'   |      | |
  |* J2                      '------' |
  '-----------------------------------'


  Discrete audio circuitry: SAME AS KIMBLE DIAGRAM
  EXCEPT FOR :R1 = 120 K ; Tolerance +/- 5%

*/

ROM_START( noraut3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g45pa.bin", 0x0000, 0x10000, CRC(f966f4d2) SHA1(99c21ceb59664f32fd1269351fa976370d486f2e) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(              0x0000, 0x0800, 0xff )
	ROM_LOAD( "gu27.bin",  0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  V3011a
  cpu :Z84C0006PEC

  Prog eprom marked
  G45P A V3.011A GU27
  EURO 27C512 10NOV03

  char marked "GU27" same asv3010a
  pal marked "VER 2" same as v3010a

  Memory expansion daughterboard etched:
  Unique (Ireland)
  +353-42-937-2147

  Everything else identical to v3010a

*/

ROM_START( noraut3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.011a.bin", 0x0000, 0x10000, CRC(82407395) SHA1(b36ef466d7fbc236f24f70f80bd9cd24ed75f51c) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(              0x0000, 0x0800, 0xff )
	ROM_LOAD( "gu27.bin",  0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  Noraut 2 bet joker hard console.
  From PCB with daughterboard.

  ----

  Requires daughterboard console in z80 to run.
  Very old program.

*/

ROM_START( norautua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "noraut_2_bet_joker_hard_console.bin", 0x0000, 0x2000, CRC(dd6b03a2) SHA1(221425534a255e5b8ef83fb567d578c127ba9a90) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  Noraut 4 bet joker console.
  From PCB with daughterboard.

  ----

  Requires daughterboard console in z80 to run.

*/

ROM_START( norautub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "noraut_4_bet_joker_console.bin", 0x0000, 0x2000, CRC(abf7725c) SHA1(997c7dbb4c4e5f1cdab28fefd17ab3c88bbc2531) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  Mainline 4 bet joker z80 28pin 60%-75% payout.
  25% Payout Hard 27c 64K uses normal pal.

*/

ROM_START( mainline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "25_payout_hard.bin", 0x0000, 0x2000, CRC(0d8a34f0) SHA1(b90c3b1949b7c2108d2202c8a5f1d54de4a572ce) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )

	ROM_REGION( 0x0200,  "plds", 0 )
	ROM_LOAD( "pal16l8_25.bin",  0x0000, 0x0104, CRC(7d9ab06c) SHA1(0f1936e24f77e95e5c95b50a421473f13929809e) )
ROM_END

/*

  DellFern 4-bet Joker z80 28pin
  45%-75% payout.

  Game title is Double Joker Poker / Jack Plus
  Board also made by DellFern.
  UK company that gone bust many years ago.

  Same program as mainline, but copied 8 times to fill the 64k device.

  PCB silksceened:
  DELLFERN U.K. LTD.
  MADE IN ENGLAND.

  PCB REDESIGNED BY M.A.BROWN.


  PCB Layout:                                                                 Edge Connector 36x2
  .----------------------------------------------------------------------------------------------.
  | .-----.                .---------.  .---------.  .-----.        .........   .---------.      |
  | | 555 |                |74LS174N |  |74LS153N |  | 555 |        .........   |ULN2003A |      |
  | '-----'                '---------'  '---------'  '-----'        D16-A-2K2   '---------'      |
  |   U46                      U45          U44        U43             U42          U41          |
  |                                                                                              |
  |              DIP SW x4                                                                       |
  | .--------.   .-.-.-.-.                                                                       |
  | |Battery |   |1|2|3|4| .---------.  .---------.  .---------.  .---------.   .---------.      |
  | |  3.6v  |   '-'-'-'-' |74LS157N |  |74LS153N |  |74LS161AP|  |74LS86AN |   |ULN2003A |      |
  | '--------'      SWB    '---------'  '---------'  '---------'  '---------'   '---------'      |
  |                            U39          U38          U37          U36           U35          |
  |                                                                                              |
  |                                                                                              | 36
  | .---------.            .---------.   .........   .---------.  .---------.   .---------.      '---.
  | | 74LS04  |            |74LS166AP|   .........   |74LS161AN|  |74LS153N |   |ULN2003A |       ---|
  | '---------'            '---------'   D16-A-2K2   '---------'  '---------'   '---------'       ---|
  |     U34                    U33          U32          U31          U30           U29           ---|
  |                                                                           .----------------.  ---|
  |              .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |   D8255AC-2    |  ---|
  |              |   27C512    |  |1|2|3|4|5|6|7|8|  |74LS161AN|  |74LS157N | |                |  ---|
  |              |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |              '-------------'     DIP SW x 8          U25          U24            U23          ---|
  |                    U27               SWA                                                      ---|
  |                                                                           .----------------.  ---|
  |                                                  .---------.  .---------. |   D8255AC-2    |  ---|
  |                                                  |74LS161AN|  |74LS157N | |                |  ---|
  | .--------------.                                 '---------'  '---------' '----------------'  ---|
  | |  GM76C28-10  |                                     U19          U18            U17          ---|
  | |              |                                                                              ---|
  | '--------------'                                                                              ---|
  |       U22                                                                                     ---|
  |                                                                                               ---|
  | .--------------.     .----------------.                                                       ---|
  | |  GM76C28-10  |     |   D8255AC-2    |          .---------.  .---------.                     ---|
  | |              |     |                |          |74LS161AN|  |74LS157N |     .........       ---|
  | '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |       U16                   U15                      U14          U13         D16-A-2K2      .---'
  |                                                                                  U12         | 01
  |                                                                                              |
  | .--------------.    .---------.                  .---------.  .---------.  .---------.       |
  | |    27C512    |    | 74LS32N |                  |74LS161AN|  | 74LS86P |  | 74LS04N |       |
  | |              |    '---------'                  '---------'  '---------'  '---------'       |
  | '--------------'        U10                18.43     U9           U8           U7            |
  |       U11                                 MHZ XTAL                                           |
  |                                            .----.                                            |
  | .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  | |      Z8400AB1      |   | 74LS138N |    | 74LS04N |  |74LS157N |  | 74LS11N |  | 74LS74A |  |
  | |      Z80A CPU      |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  | '--------------------'       U5               U4           U3           U2           U1      |
  |          U5                                                                                  |
  '----------------------------------------------------------------------------------------------'

*/

ROM_START( df_djpkr )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Program ROM is 0000-1fff, copied along the 64K of the ROM
	ROM_LOAD( "dellfern_4bet_joker_z80_28pin_45-75_payout.bin", 0x0000, 0x2000, CRC(9d150a47) SHA1(da9c0d6632faab685dd061f39b01d8e65793e1e6) )
	ROM_IGNORE(                                                         0xe000)

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

// Same as above, but with different graphics ROM.
// GFX ROM is identical to mainline one, but copied 32 times to fill the 64k device.
ROM_START( df_djpkra )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Program ROM is 0000-1fff, copied along the 64K of the ROM
	ROM_LOAD( "rom_u11.bin", 0x0000, 0x2000, CRC(9d150a47) SHA1(da9c0d6632faab685dd061f39b01d8e65793e1e6) )
	ROM_IGNORE(                      0xe000)

	ROM_REGION( 0x1000,  "gfx", 0 )  // GFX ROM is 0000-07ff, copied along the 64K of the ROM
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "rom_u27.bin", 0x0800, 0x0800, CRC(6cab56b5) SHA1(2c4a8ee6b0a021105e77d1df7d57aa6b955425ab) )
	ROM_IGNORE(                      0xf800)
ROM_END

/*

  Deluxe Poker...

  Stickered:
  AM 42
  RON 10

  Running in a standard 24-pin sockets
  Unmodified Noraut board.

  Version 18/04/2005
  New Gamble Limit=??
  Message changed to ROYAL ON 10
  CoinValue1 equ 020H ;DIP SW 2 - OFF
  CoinValue2 equ 050H ;DIP SW 2 - ON

*/

ROM_START( ndxron10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "am42_ron10.bin",  0x0000, 0x2000, CRC(7afe5bb6) SHA1(cd3ad96c0de6b58be7507526ff605a9b82894f5c) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "club250.bin",   0x0000, 0x1000, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )
ROM_END

/*
  CGI - Credit Poker, VERSION 30E,
  EXTRA BONUS, 21/12/99 BY V.S. for CGI,
  AddOn sub board, Change Gamble Card Always,
  Random Raise/Ante, SWAP HOLD-Fixed,
  Extra Bonus 7's and 9's for bet 5-9 pay 5-20,
  for bet 10-maxbet pay 5-50,
  Fixed Big Payout, GambleLimit

*/
ROM_START( cgip30e )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Program ROM is 0000-3fff, duplicated to fit the ROM size
	ROM_LOAD( "u23.bin", 0x0000, 0x8000, CRC(bc31f0b5) SHA1(8b624f825e0def9a8a44dd88a7bfa34b48a29a43) )  // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x8000,  "gfx", 0 )
	ROM_LOAD( "u1.bin",  0x0000, 0x8000, CRC(839884a3) SHA1(adb562f2b0e3e7a5c0bd9263141ee78434d00ff7) )
ROM_END

/*

  CGI - Credit Poker (v.30c, standard)

  This is a standard version, without the 7's, 9's and 5's bonus.

  Settings:

  1) Press Readout (9) button.

  You will enter into Readout options screen.

  2) Press HOLD1 to reset meters, HOLD2 for readout,
     or follow the next steps to adjust percentage.
     Press DEAL to exit.

  2) Keep pressed HI & LO buttons.
  3) Press HOLD5 (readout) button.
  4) Release both HI & LO buttons.
  5) Percentage should appear. Set with HI/LO buttons.
  6) Save and exit with DEAL button.

*/

ROM_START( cgip30cs )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Program ROM is 0000-3fff, duplicated to fit the ROM size
	ROM_LOAD( "cgi_standard_no_bonus_30c_z80_28pin.bin", 0x0000, 0x8000, CRC(7c784964) SHA1(c3deeacc73493939a11dd4cdf0fe07fcd2a9ad8a) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "graphics2716.bin", 0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

ROM_START( cgip30b )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Program ROM is 0000-3fff, duplicated to fit the ROM size
	ROM_LOAD( "u11", 0x0000, 0x8000, CRC(e32400cc) SHA1(f219aa4f35d92581b223a2172ff54cb3a6eaf7fe) )  // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(         0x0000, 0x0800, 0xff )
	ROM_LOAD( "u27",  0x0800, 0x0800, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )  // 0xxxxxxxxxxx = 0xFF
	ROM_CONTINUE(     0x0800, 0x0800 )
ROM_END

ROM_START( cgip23b )  // PCB marked 'POKER Version 1.1'. Z0840004PSC + 4 x NEC 82C55AC-2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u29", 0x0000, 0x8000, CRC(e8ac2803) SHA1(fe545fcc7dad5eb8786fae853262392ba9b067ca) )  // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "u16", 0x0000, 0x1000, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )  // 0xxxxxxxxxxx = 0xFF
ROM_END


// PCB has a sticker: Casino Games Innovation Incorporating GS Research POKER PCB
ROM_START( cgidjp )
	ROM_REGION( 0x08000, "maincpu", 0 )  // Program ROM is 0000-3fff, duplicated to fit the ROM size, opcodes are 0000-1fff, data 2000-3fff
	ROM_LOAD( "27c256.bin", 0x0000, 0x8000, CRC(6e0b8999) SHA1(5219b38292e531589d90ae3df08990f8d8664cc3) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(               0x0000, 0x0800, 0xff )
	ROM_LOAD( "27c32.bin",  0x0800, 0x0800, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )  // first half 0xff filled
	ROM_CONTINUE(           0x0800, 0x0800 )
ROM_END

ROM_START( norautn1a )
	ROM_REGION( 0x08000, "maincpu", 0 )  // Program ROM is 0000-1fff, with extra data at 3000-302f
	ROM_LOAD( "n1a-057-8.u11", 0x0000, 0x8000, CRC(96e82264) SHA1(b7514205c47f9ba651d69a44f006ce04aa00e1fa) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "n1a-057-5.u27", 0x0000, 0x1000,  CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )  // first half 0xff filled
ROM_END


/*

  Kimble Double HI-LO (Z80)
  -------------------------

  Hardware Layout:

  - CPU:             1x Z8400AB1: ON CPU ADDON BOARD
  - RAM:             2x 2114 VIDEO
  - RAM:             1x 6264 PROG: ON EXPANSION BOARD
  - I/O:             3x 8255 Peripeheral Interface Adapter.
  - Prg ROM:         1x 27C256: ON EXPANSION BOARD
  - Prg ROM:         1x 27C128: ON EXPANSION BOARD
  - Gfx ROM:         1x 2716 U27 Eprom
  - Sound:           Discrete.
  - Crystal:         1x 18.432 MHz.ON CPU ADDON BOARD
                     1X PAL16R8 ON EXPANSION BOARD

  PCB silksceened:
  SCT 18-88
  KIMBLE DOUBLE HI-LO
  CARD GAME

  PCB MARKED:SZY044

  CHAR EPROM LABELED: "QUIZ CHAR II"
  PROG EPROM (256) LABELED: "M.B POKER ALT 1 2P/10P I"
  PROG EPROM (128) LABELED: "M.B POKER ALT 1 2P/10P II"


  Frequency measured 2.3025MHz.


  PCB Layout (Kimble Double HI-LO Z80):                                       Edge Connector 36x2
  .----------------------------------------------------------------------------------------------.
  |                        .---------.  .---------.  .-----.       .........    .---------.      |
  |                        |74LS174N |  |74LS153N |  |NE555|       .........    |ULN2003A |      |
  |    BATTERY             '---------'  '---------'  '-----'       4116R 471    '---------'      |
  |  3.6V NI-CD                U45          U44        U43            U42          U41           |
  |                                                                                              |
  |                                                                                              |
  |      .------.          .---------.  .---------.  .---------.  .---------.   .---------.      |
  |      |MC1455|          |74LS157N |  | 74153N  |  | 74161N  |  |  7486N  |   |ULN2003A |      |
  |      '------'          '---------'  '---------'  '---------'  '---------'   '---------'      |
  |        U40                 U39          U38          U37          U36           U35          |
  |                                                                                              |
  |                                                                                              | 36
  |   .---------.          .---------.   4116R 471   .---------.  .---------.   .---------.      '---.
  |   |  7404N  |          | 74166N  |   .........   | 74161N  |  | 74153N  |   |ULN2003A |       ---|
  |   '---------'          '---------'   .........   '---------'  '---------'   '---------'       ---|
  |       U34                  U33          U32          U31          U30           U29           ---|
  |                                                                           .----------------.  ---|
  |              .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |   D8255AC-2    |  ---|
  | ***********  |    2716     |  |1|2|3|4|5|6|7|8|  | 74161N  |  | 74157N  | |                |  ---|
  | x11 SIL FOR  |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |EXPANSION PCB '-------------'     DIP SW x 8          U25          U24            U23          ---|
  |     U28            U27               U26                                                      ---|
  |                                                                           .----------------.  ---|
  |                                                                           |   D8255AC-2    |  ---|
  |                  .---------.     .---------.     .---------.  .---------. |                |  ---|
  |                  | 2114    |     |  2114   |     | 74161N  |  | 74157N  | '----------------'  ---|
  |    NO IC         '---------'     '---------'     '---------'  '---------'        U17          ---|
  |     U22              U21             U20             U19          U18                         ---|
  |                                                                                               ---|
  |                      .----------------.                                                       ---|
  |                      |   D8255AC-2    |          .---------.  .---------.     4116R 471       ---|
  |                      |                |          | 74161N  |  | 74157N  |     .........       ---|
  |     NO IC            '----------------'          '---------'  '---------'     .........       ---|
  |      U16                    U15                      U14          U13            U12         .---'
  |                                                                                              | 01
  | ************                              .---------.     .---------.      .---------.       |
  | x24 DIL SOCKET                            | 74161N  |     | 7486N   |      |  7404N  |       |
  | EXPANSION PCB                             '---------'     '---------'      '---------'       |
  | ************               NO IC              U9              U8               U7            |
  |       U11                   U10                                                              |
  |                                             Xtal                                             |
  | .--------------------.   .----------.      removed    .---------.  .---------.  .---------.  |
  | | Z80 EXPANSION PCB  |   |  74LS32  |                 | 74157N  |  |  7411N  |  |  7474N  |  |
  | |                    |   '----------'       NO IC     '---------'  '---------'  '---------'  |
  | '--------------------'        U5              U4           U3           U2           U1      |
  |  1        U6                                                                                 |
  '----------------------------------------------------------------------------------------------'


  PCB Layout EXPANSION BOARD (Kimble Double HI-LO 8080):
  EXPANSION BOARD  Silkscreened:SCT 34-88
  .------------------------------------------------------------------.
  |                                          5             6         |
  |            4 WIRES   ****           .---------.   .---------.    |
  |          TO MAIN PCB                | 74LS08N |   |74LS155N |    |
  |                                     '---------'   '---------'    |
  |                                                                  |
  |        EXP PIN .------.     .------.   .------.      *      *    |
  |          x11   |      |     |      |   |      |      *      *    |
  |  .---.    *    | 6264 |     | PROG |   | PROG |      * EXP  *    |
  |  | P |    *    |      |     |      |   |      |      * PIN  *    |
  |  | A |    *    |      |     |      |   |      |      *      *    |
  |  | L |    *    |      |     |27128 |   |27256 |      * x24  *    |
  |  | 1 |    *    |      |     |      |   |      |      *      *    |
  |  | 6 |    *    |      |     |      |   |      |      *      *    |
  |  | R |    *    |      |     |  II  |   |   I  |      *      *    |
  |  | 8 |    *    |      |     |      |   |      |      *      *    |
  |  |   |    *    |      |     |      |   |      |      *      *    |
  |  '---'    *    '------'     '------'   '------'      *      *    |
  |    1               2           3           4                     |
  '------------------------------------------------------------------'


  Z80 ADDON BOARD:
  Silkscreened:SCT 38-90
  .-------------------------------------------------------------.
  |                                                             |
  | .----------------------------.   .---------.   .---------.  |
  | |                            |   | 74LS14N |   | 74LS04N |  |
  | |     Z8400AB1               |   '---------'   '---------'  |
  | |                            |                              |
  | '----------------------------'                    *--*      |
  |                                              XTAL 18.432 Mhz|
  |                                                             |
  |  .---------.                 .----------------------------. |
  |  | 74LS32N |                 |                           1| |
  |  '---------'                 |  40 PINS TO MAIN PCB U6    | |
  |                              |                            | |
  |                              '----------------------------' |
  '-------------------------------------------------------------'


  Kimble Double HI-LO (Z80) discrete audio circuitry:
  ---------------------------------------------------

  3x ULN2003A (Darlington transistor array)
  1x MC1455P  (Timer)
  1x 2N2222   (Epitaxial planar general purpose NPN transistor)

  .------.                              .------------.              .-------.
  |  U17 |                              |   MC1455P  |              |2N2222 |
  |      |                             4|            |3     R3      |       |
  |   PC7|------------------------------|RST      OUT|-----ZZZZ-----|B     E|-------> Audio Out.
  |   PC6|----------.                  6|            |8             |   C   |
  |   PC5|-----.    |3-in         .-----|THR      VCC|-----------.  '---+---'  .----> Audio Out.
  |   PC4|--.  |  .-+------.      |    5|            |7          |      |      |
  |      |  |  |  |ULN2003A|      |  .--|CVOLT   DISC|--.        |      |      |
  |      |  |  |  '-+------'      |  |  |            |  |        +------'    --+--
  |      |  |  |    |3-out   C1   |  |  |    GND     |  |        |            GND
  | 8255 |  |  |    '--------||---+  |  '-----+------'  |        |             -
  |      |  |  '--.               |  |        |1        |        |             '
  |      |  |     |2-in           |  |        |         |        |
  '------'  |  .--+-----.         |  |   C5   |         |        |
            |  |ULN2003A|         |  '---||---+         |        |   +5V
            |  '--+-----'         |           |         |        |   -+-
            |     |2-out     C2   |      C4   |         |    C6  |    |
            |     '----------||---+------||---+-------. | .--||--+----'
            |2-in                 |           |       | | |      |
          .-+------.              |         --+--      '-'       |
          |ULN2003A|              |          GND        |        |
          '-+------'              |           -         |        |
            |2-out           C3   |     R1    '         |   R2   |
            '----------------||---+----ZZZZ-------------+--ZZZZ--'

  R1 = 100 K ; Tolerance +/- 5%
  R2 = 1 K   ; Tolerance +/- 5%
  R3 = 1 K   ; Tolerance +/- 5%

  C1 = 10nK 63v
  C2 = 223J
  C3 = 47nK 63v

  C4 = 10nK 63v
  C5 = 10nK 63v
  C6 = 100nK 100v

*/

ROM_START( kimblz80 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "256_mb_poker_alt_i.bin",  0x0000, 0x8000, CRC(2123c3b4) SHA1(ea9fbfc96b65bba6b193785edf926b6bba1a8d4c) )
	ROM_LOAD( "128_mb_poker_alt_ii.bin", 0x8000, 0x4000, CRC(7c4ddc78) SHA1(8da437c253e68de97190412d24bfb9d151016f1f) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_FILL(                     0x0000, 0x0800, 0xff )
	ROM_LOAD( "quiz_char_ii.bin", 0x0800, 0x0800, CRC(ad645a41) SHA1(a2c47f21609cda20a6cfee17a7bfd32fb2afd6fe) )
ROM_END

// Hi Lo Kimble, Z80
ROM_START( hilokimb )
	ROM_REGION( 0x10000, "maincpu", 0 )  // not encrypted. extended addressing.
	ROM_LOAD( "wolfie02_u11_2764.u11", 0x0000, 0x2000, CRC(ec26e205) SHA1(a778365a22c83d285c0896bab62bc04a718a5324) )
	ROM_LOAD( "wolfie03_u16_2764.u16", 0x8000, 0x2000, CRC(15adf4a0) SHA1(66936151e71cfed73faeec5075eb99c19a871576) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "5.bin", 0x0000, 0x1000, CRC(d94be899) SHA1(b7212162324fa2d67383a475052e3b351bb1af5f) )
ROM_END


/*

  PCB silkscreened PMA-32-C.
  Someone had written "poker" on it.

  CPU:   LH0080 (Sharp Z80).
  I/O:   3x PPI 8255.
  Xtal:  18 MHz.
  NVRAM: Yes, battery attached.

  ROMs: 2x 2732 (E4 & E5).
        1x 2716 (J2).

  PROM: tb24s10n (D3) read as 82s129.


  Notes: Even when the dumper specs document a Z80,
  the game is clearly from DPHL type, normally running
  a i8080 and needing an external timer to take it out
  from the kill instruction.

*/

ROM_START( pma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pma.e5", 0x0000, 0x1000, CRC(e05ab5b9) SHA1(8bd13e8ed723ac256545f19bef4fa3fe507ab9d5) )
	ROM_RELOAD(         0x1800, 0x0800 )
	ROM_CONTINUE(       0x1000, 0x0800)
	ROM_LOAD( "pma.e4", 0x2000, 0x1000, CRC(0f8b11fc) SHA1(7292b0ac368c469ff2e1ede1765c08f1ccc1a36c) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(           0x0000, 0x0800, 0xff )
	ROM_LOAD( "pma.j2", 0x0800, 0x0800, CRC(412fc492) SHA1(094ea0ffd0c22274cfe164f07c009ffe022331fd) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "pma.d3",  0x0000, 0x0200, CRC(6e172c11) SHA1(b52439a5075cc68ae2792946a5ce973d9f8e4104) )
ROM_END

/*

  Poker / Black Jack (Model 7521)

  PCB: Etched: KRAMER MFG  PWB - 000 - 40065 REV B

  - 1x Z80 CPU
  - 3x 8255
  - 1x 6116 RAM.
  - 1x Dallas DS1220Y Nonvolatile SRAM.
  - 1x Xtal 18 MHz.
  - 3x 8 DIP switches banks.


  PCB Layout:                                                                           Edge Connector 36x2
  .--------------------------------------------------------------------------------------------------------.
  |             .---------.  .-----.    .-----.     .---------. .---------. .---------.  .---------.       |
  |             |SN74LS12 |  |LM393|    |NE555|     |077B PROM| |SN74174N | |SN74LS86 |  |ULN2003AN|       |
  |    NO IC    '---------'  '-----'    '-----'     '-82S129--' '---------' '---------'  '---------'       |
  |     U64         U63        U62        U61           U51         U60         U59          U58           |
  |                                    2N3906    2N2222                                                    |
  | .---------.                                   VR1                                                      |
  | | Dallas  | .---------. .---------. .---------. .---------. .---------. .---------.  .---------.       |
  | | DS1220Y | |SN74LS00N| |TC4040BP | |ITT7402N | |SN74157N | |471RESNET| |472RESNET|  |ULN2003AN|       |
  | '---------' '---------' '---------' '---------' '---------' '---------' '---------'  '---------'       |
  |     U57         U56         U55         U54         U53         U52         U50          U49           | 36
  |                                                                                                        '---.
  | .---------. .---------. .---------. .---------. .---------. .---------. .---------.  .---------.        ---|
  | |   4F79  | |SN74LS32N| |SN74LS157| |  74161  | |SN74166N | |DIP SW x8| |DIP SW x8|  |ULN2003AN|        ---|
  | '-PAL20L10| '---------' '---------' '---------' '---------' '---------' '---------'  '---------'        ---|
  |     U48         U47         U46         U45         U44         U43         U42          U41            ---|
  |                        .---------------.  .--------------.                          .----------------.  ---|
  |             .-------.  |   CDM 6116    |  |  EPROM 2732  |  .---------. .---------. |   AMD P8255A   |  ---|
  |    NO IC    |SN7474 |  |               |  |   CF7B U31   |  | 74LS244 | | 74LS244 | |                |  ---|
  |     U26     '-------'  '---------------'  '--------------'  '---------' '---------' '----------------'  ---|
  |                U40            U39                U31            U38         U37            U36          ---|
  | .----------.                        .--------------------.                          .----------------.  ---|
  | |EPROM 2764| .-------. .---------.  |     AMD P8255A     |  .---------. .---------. |   AMD P8255A   |  ---|
  | |   U19    | |74161PC| |SN74157N |  |                    |  | 74LS244 | |DIP SW x8| |                |  ---|
  | '----------' '-------' '---------'  '--------------------'  '---------' '---------' '----------------'  ---|
  |     U19         U35        U34               U33                U32         U30            U29          ---|
  | .----------.                                                                                            ---|
  | |EPROM 2764| .---------. .--------. .---------. .---------. .---------. .--------. .---------. .------. ---|
  | |   U18    | |SN74LS32N| |DM7414N | |SN74157N | |SN74157N | |SN74LS32N| |DM7411N | |SN74LS00N| |RESNET| ---|
  | '----------' '---------' '--------' '---------' '---------' '---------' '--------' '---------' '-471--' ---|
  |     U18          U28        U27         U25         U24         U23         U22        U21       U20    ---|
  | .----------.                                                                                            ---|
  | |EPROM 2764| .---------. .--------. .---------. .---------. .---------. .---------. .-------. .-------. ---|
  | |   U12    | |SN74LS155| | RESNET | | 74161PC | | 74161PC | | 74161PC | | 74161PC | |DM7414N| |SN7486N|.---'
  | '----------' '---------' '--472---' '---------' '---------' '---------' '---------' '-------' '-------'| 01
  |     U12         U17         U16         U15         U14         U13         U11        U10        U9   |
  |                                                                                                        |
  | .-------------------.                                                                                  |
  | |    SGS Z8400B1    | .----------. .--------. .-------. .---------. .---------. .-------. .----------. |
  | |       (Z80)       | |DM74LS245N| |SN7474N | |74S04N | |SN74LS161| |SN74LS32N| |DM7414N| |SN74LS123N| |
  | '-------------------' '----------' '--------' '-------' '---------' '---------' '-------' '----------' |
  |          U8                U7          U6        U5         U4          U3         U2          U1      |
  |                                                .-----.                                                 |
  |        KRAMER MFG  PWB-000-40065 REV B.       .| === |. Xtal 18.000 MHz.                               |
  '--------------------------------------------------------------------------------------------------------'


  DIP Switches position:

  +----------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Location | #1  | #2  | #3  | #4  | #5  | #6  | #7  | #8  |
  +----------+-----+-----+-----+-----+-----+-----+-----+-----+
  |   U43    | OFF | ON  | OFF | ON  | ON  | OFF | OFF | OFF |
  +----------+-----+-----+-----+-----+-----+-----+-----+-----+
  |   U42    | OFF | OFF | OFF | OFF | OFF | OFF | OFF | OFF |
  +----------+-----+-----+-----+-----+-----+-----+-----+-----+
  |   U30    | OFF | OFF | OFF | ON  | OFF | OFF | OFF | OFF |
  +----------+-----+-----+-----+-----+-----+-----+-----+-----+

  Discrete audio circuitry: UNKNOWN.

*/

ROM_START( bjpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7521-ver.1.u12", 0x0000, 0x2000, CRC(c7b16be0) SHA1(6c875acd4e8468afa1184863a2c6bd7eb086f6e1) )
	ROM_LOAD( "7521-ver.1.u18", 0x2000, 0x2000, CRC(1eff06cc) SHA1(c2135883e14c156dbfd2f38594a896887acb5d2f) )
	ROM_LOAD( "7521-ver.1.u19", 0x4000, 0x2000, CRC(cadfe6f4) SHA1(5a0881231fd84e270441973c9d08beee2a176cbc) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "cf7b.u31",  0x0000, 0x1000, CRC(fcfc4d25) SHA1(31455903244ec8ef9005748f265f561b7a082a9c) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "82s129_077b.u51", 0x0000, 0x0100, CRC(920e8394) SHA1(309729db53b94f8ee5d3d32003288729757b140f) )

	ROM_REGION( 0x083f,  "plds", 0 )
	ROM_LOAD( "pal20l10_4f79.u48",   0x0000, 0x083f, CRC(c7f4aa8f) SHA1(a15cc8f075035a70af42eb3873faa5ebedab5dc8) )
ROM_END


ROM_START( newhilop )
	ROM_REGION( 0x10000, "cpu_data", 0 )  // multiple programs?
	ROM_LOAD( "new_hi-low.3e",  0x0000, 0x10000, CRC(8efe02a2) SHA1(e8150544f073e80ca83f2033bce64b65de08194c) )

	ROM_REGION( 0x10000, "gfx_data", 0 )
	ROM_LOAD( "new_hi-low.3a",  0x0000, 0x10000, CRC(6750f0e7) SHA1(cfb180aed9ff288cf1108071f2618587ac85ad1a) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_COPY( "cpu_data", 0xe000, 0x0000, 0x2000 )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_COPY( "gfx_data", 0xf000, 0x0000, 0x1000 )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "82s129.4d", 0x0000, 0x0100, CRC(88302127) SHA1(aed1273974917673405f1234ab64e6f8b3856c34) )  // = japan_6301.u51 / dphljp / Draw Poker HI-LO (Japanese)
ROM_END


/*

  Draw Poker
  Coinmaster.
  Based on Joker Poker V.01.

  Noraut Z80 based HW.

  1x Z80.
  3x PPI 8255:
     PPI #0:  offset 0x70-0x73  config = 0x90
     PPI #1:  offset 0xB0-0xB3  config = 0x92
     PPI #2:  offset 0xD0-0xD3  config = 0xC0

  The game needs approx 20 seconds to boot the game.

*/

ROM_START( cdrawpkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpk_a_1-1.ic29",   0x0000, 0x1000, CRC(f4eb1664) SHA1(f636279d3ef5c79277676f45388522b00625cc72) )
	ROM_LOAD( "jpk_aus_2-1.ic25", 0x1000, 0x1000, CRC(9ed796bf) SHA1(31ffd4fe3cbbebbe502dbf7fd51524636bf74d6b) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                 0x0000, 0x0800, 0xff )
	ROM_LOAD( "char_1.ic15",  0x0800, 0x0800, CRC(b114d647) SHA1(d507bad88bec5999b2b2a3f7c4472dedeaed1a29) )
ROM_END


/*

  Winbank Joker Poker

  Alt version of norautp, but this one works in credit and hopper mode.
  1/2 gamble in norautp credit mode doesn't work right.

  Also this one saves its setup SPEED/BET/TARGET unlike norautp.

*/

ROM_START( winbnk88 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "winbank.bin", 0x0000, 0x2000, CRC(3a58777c) SHA1(fcf7cae4aa5175ecc3fe2c78e97702c9a379c335) )

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_LOAD( "gu2732.bin",  0x0000, 0x1000, CRC(2f00ccae) SHA1(914967ee7f60158225defbece6d33d66f619f8e9) )
ROM_END

ROM_START( norautpb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "512.bin", 0x0000,  0x10000,   CRC(24d231b9) SHA1(838d9cbb6cec1132c3d7ab16c4ce5b20dc6f5d2b) )

	ROM_REGION( 0x10000, "other", 0 )  // unknown... can't find relation.
	ROM_LOAD( "72_percent_primer_8_4_2000.bin", 0x0000, 0x10000, CRC(b7c2e734) SHA1(3f79393c96b0c93e2629661a7796e01af8d4c6d6) )

	ROM_REGION( 0x01000, "gfx", 0 )
	ROM_LOAD( "32.bin", 0x00000, 0x1000, CRC(41112cd7) SHA1(62895300f3482f219acb3b30ddebd7f6c93ffc33) )
ROM_END


// unlock sequence:
// 1 - "F" + "D" + hold 1 + hold 2
// 2 - Bet + Deal + Small
ROM_START( nor6007a )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 6007 32k - code:0-3fff  - 2 blocks - equals
	ROM_LOAD( "600732k.bin", 0x0000, 0x8000, CRC(02a72cf6) SHA1(fa3000362a50694c106bf6d16164c5d4bf043ce2) )

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_LOAD( "gu31.bin", 0x0000, 0x1000, CRC(41112cd7) SHA1(62895300f3482f219acb3b30ddebd7f6c93ffc33) )
ROM_END

/*
  Noraut 6007 64k

  Identical halves.
  Each quarter has a different program.

  Diffs between 1st and 2nd quarters:

  offset   1st quarter  2nd quarted
  $050b       $cf          $cb
  $060f       $0f          $07
  $0b4f       $e0          $c0
  $260f       $cf          $cb
  $3a0f       $c7          $c3
  $3e4f       $6c          $64

  unlock sequence:
  1 - "F" + "D" + hold 1 + hold 2
  2 - Bet + Deal + Small

*/
ROM_START( nor6007b )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 6007 64k - code:0-7fff  - 2 blocks - equals
	ROM_LOAD( "600764k.bin", 0x4000, 0x4000, CRC(8de1271c) SHA1(02ef769fb952b093d15cec6b19be679631e2d6cf) )  // first quarter buggy program
	ROM_CONTINUE(            0x0000, 0x4000)  // second quarter good program.
	ROM_IGNORE(                      0x8000)

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_LOAD( "2516.bin", 0x0000, 0x0800, CRC(3bae5691) SHA1(32bb79c9f01a4f679e4b220ae8bd051a374771c8) )
	ROM_RELOAD(           0x0800, 0x0800 )
ROM_END


/*
  Double Dellfern 1996

  dblprog.bin   BADADDR ---xxxxxxxxxxxxx
  dblchar.bin   BADADDR -----xxxxxxxxxxx

*/
ROM_START( ddellf96 )  // 8 identical blocks of 0x2000 each
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dblprog.bin", 0x00000, 0x2000, CRC(9d150a47) SHA1(da9c0d6632faab685dd061f39b01d8e65793e1e6) )
	ROM_IGNORE(                       0xe000)

	ROM_REGION( 0x1000, "gfx", 0 )  // 16 identical blocks of 0x1000 each
	ROM_LOAD( "dblchar.bin", 0x00000, 0x1000, CRC(6cab56b5) SHA1(2c4a8ee6b0a021105e77d1df7d57aa6b955425ab) )
	ROM_IGNORE(                       0xf000)
ROM_END


/*
  Double Dellfern 1997
  --------------------

  PCB Etched:

  Dellfern UK LTD
  MADE IN ENGLAND (C)1997

  Rom: VERSION 18, 16.05.97 BY V.S

  - CPU:      1x Z84C0006PEC
  - RAM:      1x 6264A  8K X 8 - Bit CMOS SRAM
  - RAM:      1x GM76C28A - 2048 x 8 bit CMOS Static RAM
  - I/O:      3x 8255 Peripeheral Interface Adapter.
  - Prg ROM:  1x 27C512
  - Gfx ROM:  1x 27C512
  - Sound:    Discrete.
  - Crystal:  1x 18.432 MHz
  - PLDs:     1x PALCE16V8H read protected

  Pack of six programs inside for different cabs types.

  Program offsets:

  - 0000-3FFF  : 77s, 99s and Suits. Layout input BET is with change card, and CHANGE is collect.
  - 4000-7FFF  : similar to above...
  - 8000-9FFF  : normal noraut program. inputs ok.
  - A000-BFFF  : 6 pounds jackpot. same noraut inputs & layout.
  - C000-DFFF  : noraut game. different inputs, needs layout with cancel.
  - E000-FFFF  : one bet, normal game, inputs changed and wrong layout. needs the cancel one.

*/
ROM_START( ddellf97 )  //  program 0000-3FFF
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "progv18.bin", 0x0000, 0x4000, CRC(94153958) SHA1(87078ecc07f302fc9853562601e9bb0a2c8163d0) )
	ROM_IGNORE(                      0xc000)

	ROM_REGION( 0x1000, "gfx", 0 )  // BADADDR ---xxxxxxxxxxxxx. 16 identical blocks of 0x1000 each.
	ROM_LOAD( "charv18.bin", 0x0000, 0x1000, CRC(46bbe214) SHA1(9f74dfa0ae337f8e924428f9e0f984cbb5b72fbf) )
	ROM_IGNORE(                      0xf000)
ROM_END

ROM_START( ddellf97a )  // program 4000-7FFF
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "progv18.bin", 0x0000, 0x4000, CRC(94153958) SHA1(87078ecc07f302fc9853562601e9bb0a2c8163d0) )
	ROM_CONTINUE(            0x0000, 0x4000)
	ROM_IGNORE(                      0x8000)

	ROM_REGION( 0x1000, "gfx", 0 )  // BADADDR ---xxxxxxxxxxxxx. 16 identical blocks of 0x1000 each.
	ROM_LOAD( "charv18.bin", 0x0000, 0x1000, CRC(46bbe214) SHA1(9f74dfa0ae337f8e924428f9e0f984cbb5b72fbf) )
	ROM_IGNORE(                      0xf000)
ROM_END

ROM_START( ddellf97b )  // program 8000-9FFF
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "progv18.bin", 0x0000, 0x4000, CRC(94153958) SHA1(87078ecc07f302fc9853562601e9bb0a2c8163d0) )
	ROM_CONTINUE(            0x0000, 0x4000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_IGNORE(                      0x6000)

	ROM_REGION( 0x1000, "gfx", 0 )  // BADADDR ---xxxxxxxxxxxxx. 16 identical blocks of 0x1000 each.
	ROM_LOAD( "charv18.bin", 0x0000, 0x1000, CRC(46bbe214) SHA1(9f74dfa0ae337f8e924428f9e0f984cbb5b72fbf) )
	ROM_IGNORE(                      0xf000)
ROM_END

ROM_START( ddellf97c )  // program A000-BFFF
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "progv18.bin", 0x0000, 0x4000, CRC(94153958) SHA1(87078ecc07f302fc9853562601e9bb0a2c8163d0) )
	ROM_CONTINUE(            0x0000, 0x4000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_IGNORE(                      0x4000)

	ROM_REGION( 0x1000, "gfx", 0 )  // BADADDR ---xxxxxxxxxxxxx. 16 identical blocks of 0x1000 each.
	ROM_LOAD( "charv18.bin", 0x0000, 0x1000, CRC(46bbe214) SHA1(9f74dfa0ae337f8e924428f9e0f984cbb5b72fbf) )
	ROM_IGNORE(                      0xf000)
ROM_END

ROM_START( ddellf97d )  // program C000-DFFF
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "progv18.bin", 0x0000, 0x4000, CRC(94153958) SHA1(87078ecc07f302fc9853562601e9bb0a2c8163d0) )
	ROM_CONTINUE(            0x0000, 0x4000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_IGNORE(                      0x2000)

	ROM_REGION( 0x1000, "gfx", 0 )  // BADADDR ---xxxxxxxxxxxxx. 16 identical blocks of 0x1000 each.
	ROM_LOAD( "charv18.bin", 0x0000, 0x1000, CRC(46bbe214) SHA1(9f74dfa0ae337f8e924428f9e0f984cbb5b72fbf) )
	ROM_IGNORE(                      0xf000)
ROM_END

ROM_START( ddellf97e )  // program E000-EFFF
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "progv18.bin", 0x0000, 0x4000, CRC(94153958) SHA1(87078ecc07f302fc9853562601e9bb0a2c8163d0) )
	ROM_CONTINUE(            0x0000, 0x4000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_CONTINUE(            0x0000, 0x2000)
	ROM_CONTINUE(            0x0000, 0x2000)

	ROM_REGION( 0x1000, "gfx", 0 )  // BADADDR ---xxxxxxxxxxxxx. 16 identical blocks of 0x1000 each.
	ROM_LOAD( "charv18.bin", 0x0000, 0x1000, CRC(46bbe214) SHA1(9f74dfa0ae337f8e924428f9e0f984cbb5b72fbf) )
	ROM_IGNORE(                      0xf000)
ROM_END


//************************************* i8080 sets *************************************
//*                                                                                    *
//*  The following ones are 'Draw Poker HI-LO' type, running in a 8080 based hardware  *
//*                                                                                    *
//**************************************************************************************

/*

  Draw Poker HI-LO (1983).
  "NYMF O II" hardware.
  M. Kramer Inc.

  PCB layout (Draw Poker HI-LO):
  .---------------------------------------------------------------------------.
  | .---------.                     .------.                                  |
  | |HCF4093BE|           SN74174N  | U51  |  NE555P  916C472X2PE  ULN2003A   |
  | |         |                     '------'                                  |
  | |         |                                                               |
  | |MC14040  |  74123N   SN74157N            74161N  SN7486N      ULN2003A   |
  | |         |                                                               |
  | |         |                                                               |
  | |MWS5101  |  SN7404N  SN74166N  898-1-R   74161N               ULN2003A   '--.
  | |         |                                                                --|
  | |         |           .------.  .-------.                   .-----------.  --|
  | |5101E-1  |           | U31  |  |DIP-SW | 74161N  SN74157N  |AM8255 APC |  --|
  | '---------|           '------'  '-------'                   '-----------'  --|
  |  .------.                                                   .-----------.  --|
  |  | U26  |             2111A-2   2111A-2   74161N  SN74157N  | U20       |  --|
  |  '------'                                                   '-----------'  --|
  |  .------.   .------.  .-----------.                                        --|
  |  | U19  |   | U18  |  |AM8255 APC |       74161N  SN74157N     898-1-R     --|
  |  '------'   '------'  '-----------'                                       .--'
  |             .------.  .----------.                                        |
  |  74LS541N   | U12  |  | i D8228  |   OSC  74161N  SN7486N      SN7404N    |
  |             '------'  '----------'                                        |
  |             .----------.                                                  |
  |  DM7405N    |i P8080A  |     SN74LS155AN  iP8224  SN74157N 7411N 7474PC   |
  |             '----------'                                                  |
  '---------------------------------------------------------------------------'

  OSC = 18.14 MHz

  U12 = AM2732
  U18 = AM2732
  U31 = AM2732A
  U51 = N82S129N

  U26, U19, U20 = unpopulated

  Edge connector is not JAMMA

*/

ROM_START( dphl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dphl_6468.u12", 0x0000, 0x1000, CRC(d8c4fe5c) SHA1(6bc745fefb8a3a21ca281d519895828047526de7) )
	ROM_LOAD( "dphl_36e3.u18", 0x1000, 0x1000, CRC(06cf6789) SHA1(587d883c399348b518e3be4d1dc2581824055328) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "dphl_model_2_cgi_3939.u31",  0x0000, 0x1000, CRC(2028db2c) SHA1(0f81bb71e88c60df3817f58c28715ce2ea01ad4d) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "98ce.u51",  0x0000, 0x0100, CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )
ROM_END

/*

  Draw Poker HI-LO (alt)
  ----------------------

  Hardware Layout (Draw Poker HI-LO (alt)):

  Board layout/pcb tracks almost Identical to NORAUT boards.

  - CPU:             1x INTEL P8080A : L1087022 : INTEL '79.
  - RAM:             4x 2111A-2 Static Random Access Memory 256 x 4 bit.
  - I/O:             3x 8255 Peripeheral Interface Adapter.
  - Prg ROMs:        2x 2716 U11,U16 Eprom.
  - Gfx ROMs:        1x 2716 U27 Eprom :EXACT MATCH WITH NORAUT V3010A CHAR ROM.
  - Sound:           Discrete.
  - Crystal:         1x 18.000 MHz.

  PCB silksceened: REV A.
  PCB MARKED: Solderside "81 16".
  Component side "J3 018".

  U11 2716 EPROM MARKED:"2B27".
  U16 2716 EPROM MARKED:"4D30".

  Frequency measured on CPU P8080A (pins 15 & 22) = 2.00056 MHz.

  No date information found on PCB or in Roms.
  Some dates found on some of the IC's
  U6: 1979 :SOLDERED TO BOARD
  U10:1975 :SOLDERED TO BOARD
  U15:1979 :SOLDERED TO BOARD
  U23:1981 :IN SOCKET
  U27:1977 :IN SOCKET


  PCB Layout (Draw Poker HI-LO (alt)):                                       Edge Connector 36x2
  .----------------------------------------------------------------------------------------------.
  |                        .---------.  .---------.  .-----.       .........    .---------.      |
  |                        |74LS174N |  |74LS153N |  |NE555|       .........    |ULN2003A |      |
  |       NO IC            '---------'  '---------'  '-----'       16-2-472     '---------'      |
  |        U46                 U45          U44        U43            U42          U41           |
  |                                                                                              |
  |                                                                                              |
  |                        .---------.  .---------.  .---------.  .---------.   .---------.      |
  |                        |74LS157N |  | 74153N  |  | 74161N  |  |  7486N  |   |ULN2003A |      |
  |       NO IC            '---------'  '---------'  '---------'  '---------'   '---------'      |
  |        U40                 U39          U38          U37          U36           U35          |
  |                                                                                              |
  |                                                                                              | 36
  | .---------.            .---------.  916C471X2PE  .---------.  .---------.   .---------.      '---.
  | |  7404N  |            | 74166N  |   .........   | 74161N  |  | 74153N  |   |ULN2003A |       ---|
  | '---------'            '---------'   .........   '---------'  '---------'   '---------'       ---|
  |     U34                    U33          U32          U31          U30           U29           ---|
  |                                                                           .----------------.  ---|
  |              .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |    P8255A-5    |  ---|
  | .---------.  |    2716     |  |1|2|3|4|5|6|7|8|  | 74161N  |  | 74157N  | |                |  ---|
  | | 2111A-2 |  |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  | '---------'  '-------------'     DIP SW x 8          U25          U24            U23          ---|
  |     U28            U27               U26                                                      ---|
  |                                                                           .----------------.  ---|
  |                                                                           |    D8255AC-5   |  ---|
  | .---------.      .---------.     .---------.     .---------.  .---------. |                |  ---|
  | | 2111A-2 |      | 2111A-2 |     | 2111A-2 |     | 74161N  |  | 74157N  | '----------------'  ---|
  | '---------'      '---------'     '---------'     '---------'  '---------'        U17          ---|
  |     U22              U21             U20             U19          U18                         ---|
  |                                                                                               ---|
  | .--------------.     .----------------.                                                       ---|
  | |     2716     |     |   AM8255A PC   |          .---------.  .---------.    916C471X2PE      ---|
  | |              |     |                |          | 74161N  |  | 74157N  |     .........       ---|
  | '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |       U16                   U15                      U14          U13            U12         .---'
  |                                                                                              | 01
  | .--------------.       .------------.     .---------.     .---------.      .---------.       |
  | |     2716     |       |  i P8228   |     | 74161N  |     | 7486N   |      |  7404N  |       |
  | |              |       |            |     '---------'     '---------'      '---------'       |
  | '--------------'       '------------'         U9              U8               U7            |
  |       U11                   U10           XTAL                                               |
  |                                          .----. 18Mhz                                        |
  | .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  | |     i P8080A       |   |  74155N  |    | i P8224 |  | 74157N  |  |  7411N  |  |  7474N  |  |
  | |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  | '--------------------'        U5              U4           U3           U2           U1      |
  |           U6                                                                                 |
  '----------------------------------------------------------------------------------------------'


  Draw Poker HI-LO (alt) discrete audio circuitry
  -----------------------------------------------

  3x ULN2003A (Darlington transistor array)
  1x NE555P   (Timer)
  1x F 2N4401 (NPN General Purpose Amplifier)

  .------.                              .------------.              .-------.
  |  U17 |                              |   NE555P   |              |2N4401 |
  |      |                             4|            |3     R3      |       |
  |   PC7|------------------------------|RST      OUT|-----ZZZZ-----|B     E|-------> Audio Out.
  |   PC6|----------.                  6|            |8             |   C   |
  |   PC5|-----.    |3-in         .-----|THR      VCC|-----------.  '---+---'  .----> Audio Out.
  |   PC4|--.  |  .-+------.      |    5|            |7          |      |      |
  |      |  |  |  |ULN2003A|      |  .--|CVOLT   DISC|--.        |      |      |
  |      |  |  |  '-+------'      |  |  |            |  |        +------'    --+--
  |      |  |  |    |3-out   C1   |  |  |    GND     |  |        |            GND
  | 8255 |  |  |    '--------||---+  |  '-----+------'  |        |             -
  |      |  |  '--.               |  |        |1        |        |             '
  |      |  |     |2-in           |  |        |         |        |
  '------'  |  .--+-----.         |  |   C5   |         |        |
            |  |ULN2003A|         |  '---||---+         |        |   +5V
            |  '--+-----'         |           |         |        |   -+-
            |     |2-out     C2   |      C4   |         |    C6  |    |
            |     '----------||---+------||---+-------. | .--||--+----'
            |2-in                 |           |       | | |      |
          .-+------.              |         --+--      '-'       |
          |ULN2003A|              |          GND        |        |
          '-+------'              |           -         |        |
            |2-out           C3   |     R1    '         |   R2   |
            '----------------||---+----ZZZZ-------------+--ZZZZ--'

  R1 = 120 K ; Tolerance +/- 5%
  R2 = 1 K   ; Tolerance +/- 5%
  R3 = 1 K   ; Tolerance +/- 5%

  C1 = .01 Z
  C2 = .022 Z
  C3 = 503   ; 50.000 pf = 50 nf = 0.05 uf.
  C4 = .01 Z
  C5 = .01 Z
  C6 = .1 Z

  All Capacitors are Ceramic Disc.

*/

ROM_START( dphla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2b27.u11", 0x0000, 0x0800, CRC(3a7ece95) SHA1(bc7c89e3f490da0723b3a7617ab9a747f8db7ea7) )
	ROM_LOAD( "4d30.u16", 0x0800, 0x0800, CRC(32594684) SHA1(cda1ed09ec30082d23e690058261523e0d34938e) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(              0x0000, 0x0800, 0xff )
	ROM_LOAD( "char.u27",  0x0800, 0x0800, CRC(174a5eec) SHA1(44d84a0cf29a0bf99674d95084c905d3bb0445ad) )
ROM_END

/*

  Etched on top of board in copper: "MADE IN JAPAN".
  Stickered on top: "Serial No. 10147".

  Orange dot sticker dot near pin 1.
  White dot sticker at other end of connector.

  .u18    MB8516  read as 2716    stickered   13.
  .u19    MB8516  read as 2716    stickered   11.
  .u12    MB8516  read as 2716    stickered   12.
  .u31    MB8516  read as 2716    stickered   10.
  .u51    6301    read as 82s129

  1x 18.000 Crystal
  1x 8080
  3x 8255
  2x 5101
  1x 8228
  2x 2114
  1x 8 DIP Switches bank.

  Mini daughterboard attached.

*/

ROM_START( dphljp )  // close to GTI Poker
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "japan_12.u12", 0x0000, 0x0800, CRC(086a2303) SHA1(900c7241c33a38fb1a791b311e50f7d7f43bb955) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "japan_13.u18", 0x1000, 0x0800, CRC(ccaad5cb) SHA1(5f6ca497ccb7c535714a6e24df00f2831a7840c1) )
	ROM_RELOAD(               0x1800, 0x0800 )
	ROM_LOAD( "japan_11.u19", 0x2000, 0x0800, CRC(9f9c67d5) SHA1(cd11849b245406821af7ac3554805c9dd89645b2) )    // ???

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                 0x0000, 0x0800, 0xff )
	ROM_LOAD( "japan_10.u31", 0x0800, 0x0800, CRC(412fc492) SHA1(094ea0ffd0c22274cfe164f07c009ffe022331fd) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "japan_6301.u51", 0x0000, 0x0100, CRC(88302127) SHA1(aed1273974917673405f1234ab64e6f8b3856c34) )
ROM_END

/*

  Kimble Double HI-LO (8080).
   ---- Kimble Ireland ----


  - Older than Noraut stuff...
  - RAM seems at c000-c7ff


  Hardware Layout (Kimble Double HI-LO 8080):

   - CPU:             1x INTEL P8080A
   - RAM:             2x 2114 VIDEO
   - RAM:             1x 6264 PROG: ON EXPANSION BOARD
   - I/O:             3x 8255 Peripeheral Interface Adapter.
   - Prg ROM:         1x 27C256: ON EXPANSION BOARD
   - Gfx ROM:         1x 2716 U27 Eprom
   - Sound:           Discrete.
   - Crystal:         1x 18.432 MHz.
                      1X PAL16R8 ON EXPANSION BOARD

  PCB silksceened:
  "SCT 41-88"
  "KIMBLE DOUBLE HI-LO"
  "CARD GAME"

  PCB MARKED:"VZY07"

  CHAR EPROM LABELED: "QUIZ CHAR II"
  PROG EPROM LABELED: "JPCS25611"


  Frequency measured = 2.040 MHz.


  PCB Layout (Kimble Double HI-LO 8080):                                      Edge Connector 36x2
  .----------------------------------------------------------------------------------------------.
  |                        .---------.  .---------.  .-----.       .........    .---------.      |
  |                        |74LS174N |  |74LS153N |  |NE555|       .........    |ULN2003A |      |
  |    BATTERY             '---------'  '---------'  '-----'       4116R 471    '---------'      |
  |  3.6V NI-CD                U45          U44        U43            U42          U41           |
  |                                                                                              |
  |                                                                                              |
  |      .------.          .---------.  .---------.  .---------.  .---------.   .---------.      |
  |      |MC1455|          |74LS157N |  | 74153N  |  | 74161N  |  |  7486N  |   |ULN2003A |      |
  |      '------'          '---------'  '---------'  '---------'  '---------'   '---------'      |
  |        U40                 U39          U38          U37          U36           U35          |
  |                                                                                              |
  |                                                                                              | 36
  |   .---------.          .---------.   4116R 471   .---------.  .---------.   .---------.      '---.
  |   |  7404N  |          | 74166N  |   .........   | 74161N  |  | 74153N  |   |ULN2003A |       ---|
  |   '---------'          '---------'   .........   '---------'  '---------'   '---------'       ---|
  |       U34                  U33          U32          U31          U30           U29           ---|
  |                                   DIP SW x 8                              .----------------.  ---|
  |              .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |   D8255AC-2    |  ---|
  | ***********  |    2716     |  |1|2|3|4|5|6|7|8|  | 74161N  |  | 74157N  | |                |  ---|
  | x11 SIL FOR  |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |EXPANSION PCB '-------------'         U26             U25          U24            U23          ---|
  |     U28            U27                                                                        ---|
  |                                                                           .----------------.  ---|
  |                                                                           |   D8255AC-2    |  ---|
  |                  .---------.     .---------.     .---------.  .---------. |                |  ---|
  |                  |  2114   |     |  2114   |     | 74161N  |  | 74157N  | '----------------'  ---|
  |    NO IC         '---------'     '---------'     '---------'  '---------'        U17          ---|
  |     U22              U21             U20             U19          U18                         ---|
  |                                                                                               ---|
  |                      .----------------.                                                       ---|
  |                      |   D8255AC-2    |          .---------.  .---------.     4116R 471       ---|
  |                      |                |          | 74161N  |  | 74157N  |     .........       ---|
  |    NO IC             '----------------'          '---------'  '---------'     .........       ---|
  |     U16                     U15                      U14          U13            U12         .---'
  |                                                                                              |
  | ************           .------------.     .---------.     .---------.      .---------.       | 01
  | x24 DIL SOCKET         |  i P8228   |     | 74161N  |     | 7486N   |      |  7404N  |       |
  | EXPANSION PCB          |            |     '---------'     '---------'      '---------'       |
  | ************           '------------'         U9              U8               U7            |
  |       U11                   U10          XTAL                                                |
  |                                         .----. 18.432Mhz                                     |
  | .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  | |      i P8080A      |   |  74LS32  |    | i P8224 |  | 74157N  |  |  7411N  |  |  7474N  |  |
  | |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  | '--------------------'        U5              U4           U3           U2           U1      |
  |           U6                                                                                 |
  '----------------------------------------------------------------------------------------------'


  PCB Layout EXPANSION BOARD (Kimble Double HI-LO 8080):

  EXPANSION BOARD  silksceened:SCT 34-88
  .------------------------------------------------------------------.
  |                                          5             6         |
  |            4 WIRES   ****              NO IC      .---------.    |
  |          TO MAIN PCB                   14 DIL     |74LS155N |    |
  |                                                   '---------'    |
  |                                                                  |
  |        EXP PIN .------.               .-------.                  |
  |          x11   |      |               |       |      *      *    |
  |  .---.    *    | 6264 |               | PROG  |      * EXP  *    |
  |  | P |    *    |      |               |       |      * PIN  *    |
  |  | A |    *    |      |               |       |      *      *    |
  |  | L |    *    |      |     NO IC     | 27256 |      * x24  *    |
  |  | 1 |    *    |      |     28 DIL    |       |      *      *    |
  |  | 6 |    *    |      |               |       |      *      *    |
  |  | R |    *    |      |               |       |      *      *    |
  |  | 8 |    *    |      |               |       |      *      *    |
  |  |   |    *    |      |               |       |      *      *    |
  |  '---'    *    '------'               '-------'      *      *    |
  |    1              2           3          4           *      *    |
  '------------------------------------------------------------------'


  Discrete Sound System is identical to Kimble z80 hardware.

*/

ROM_START( kimbldhl )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Program ROM is 0000-6e40
	ROM_LOAD( "jpc525611.bin",  0x0000, 0x8000, CRC(4a3f1aef) SHA1(570ef733989da6e89f0387f1e80b934cec7a7663) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                   0x0000, 0x0800, 0xff )
	ROM_LOAD( "quizcharll.bin", 0x0800, 0x0800, CRC(4edb4717) SHA1(466556595abfbc11e31f2b0d9ca0213bd649253c) )
ROM_END

/*

  Has (c)1983 GTI in the roms, and was called 'Poker.zip'
  GFX roms contain 16x16 tiles of cards.
  Nothing else is known about this set/game.

  Seems to be the following one:
  https://flyers.arcade-museum.com/videogames/show/4328

*/

ROM_START( gtipoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u12.rom", 0x0000, 0x1000, CRC(abaa257a) SHA1(f830213ae0aaad5a9a44ec77c5a186e9e02fa041) )
	ROM_LOAD( "u18.rom", 0x1000, 0x1000, CRC(1b7e2877) SHA1(717fb70889804baa468203f20b1e7f73b55cc21e) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "2732.u31", 0x0000, 0x1000, CRC(ba037f69) SHA1(8f9c325d2a250ee02ac42ffeccbe7af1fc2da6a9) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "82s129an.u51", 0x0000, 0x0100, CRC(c64f5b20) SHA1(ffbd46c59516f2f69cceb0bf423c489bdbe5d46d) )
ROM_END

/*

  PCB is almost identical to SMS HI-LO.
  "Copyright GTI 1983" found in rom at U12

  Uses P8080A-1 cpu
  clock crystal is 18.000 MHz

  Need proper memory map.

  Sound circuitry
  ---------------
                                 R13    C27
  .---------------------------+--ZZZZ---||---> GND
  |                           |
  |               .--------.  |
  |  R12          |  U50   |  |
  '--ZZZZ--.      | NE555  |  |
           |      |        |  |
           +---+--| 2    7 |--'                              .--> Audio Out
       C9  |   |  |        |       R14         Q2            |
  .---||---+   +--| 6    3 |-------ZZZZ------> B             |    .-----------.
  |        |      |        |                 E/=\C--> +5VDC  |    |    U20    |
  |        |      |      4 |--------------.  |               |    | D8255AC-5 |
  |        |      |        |   C10        |  '---------------'    |           |
  |        |      |      5 |---||--> GND  '-----------------------| 10        |
  |        |      '--------'                                      |           |
  |        |                    .---------------------------------| 11        |
  |        |                    |                                 |           |
  |        |      .---------.   |  .------------------------------| 13        |
  |        |      |   U34   |   |  |                              |           |
  |        |      | ULN2003 |   |  |                              |           |
  |        |  C7  |         |   |  |                              |           |
  |        '--||--| 14    3 |---'  |                              |           |
  |               '---------'      |                              |           |
  |                                |                              |           |
  |               .---------.      |                              |           |
  |               |   U48   |      |                              '-----------'
  |               | ULN2003 |      |
  |               |         |      |
  '---------------| 15    2 |------'
                  '---------'

  R12 - 120,000 ; Tolerance +/- 5%
  R13 - 42.0 ; Tolerance +/- 20%
  R14 - 42.0 ; Tolerance +/- 20%

  C7  - 401M 50V (Ceramic Disc)
  C9  - 503M 100V (Ceramic Disc)
  C10 - .01M 50V (Ceramic Disc)
  C27 - 104

  Q2 - 2N2222A

*/

ROM_START( gtipokra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732.u12",    0x0000, 0x1000, CRC(cee5b03c) SHA1(38a5885b4a95d7b3fa0dd099c160a5e4d854e00a) )
	ROM_LOAD( "2732_db.u18", 0x1000, 0x1000, CRC(f44cce3a) SHA1(f3e2a2a164d05a7ef121a7f0e872841553b6b2fe) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "2732.u31", 0x0000, 0x1000, CRC(ba037f69) SHA1(8f9c325d2a250ee02ac42ffeccbe7af1fc2da6a9) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "82s129an.u51", 0x0000, 0x0100, CRC(c64f5b20) SHA1(ffbd46c59516f2f69cceb0bf423c489bdbe5d46d) )
ROM_END

ROM_START( gtipokrb )  // cleco, maybe wrong palette, since is blue background
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gtipokrb.u12", 0x0000, 0x1000, CRC(e78a10d7) SHA1(e0ac382b02a1fc490b5d08344a20f2eaa3899002) )
	ROM_LOAD( "gtipokrb.u18", 0x1000, 0x1000, CRC(4c3c5b15) SHA1(0584c0a6b52465686967f98ea3bd86c22b5bd526) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "gtipokrb.u31", 0x0000, 0x1000, CRC(47e63fd2) SHA1(27bd1af9c910afb4e9955842f781954d5e8785d4) )

	ROM_REGION( 0x0100,  "proms", 0 )  // EPROM 27E256 from daughterboard. High nibble is 0xf instead of 0
	ROM_LOAD( "sub-pcb_w27e512_cleco.u51", 0x0000, 0x0100, CRC(b0594d12) SHA1(1640b2e6a3d5714158b5b5f7d63422ff12be1f96) )
	ROM_IGNORE(                                    0xff00)
ROM_END

/*
  PCB etched: "Games of Tennessee 88"
  Same program from above, but real bipolar PROM for palette.
  Different to the cleco shitty stuff

*/
ROM_START( gtipokrba )  //
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "88_2732a.u12", 0x0000, 0x1000, CRC(e78a10d7) SHA1(e0ac382b02a1fc490b5d08344a20f2eaa3899002) )
	ROM_LOAD( "88_2732a.u18", 0x1000, 0x1000, CRC(4c3c5b15) SHA1(0584c0a6b52465686967f98ea3bd86c22b5bd526) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "u31_2732a.u31", 0x0000, 0x1000, CRC(47e63fd2) SHA1(27bd1af9c910afb4e9955842f781954d5e8785d4) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n.u51", 0x0000, 0x0100, CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )
ROM_END

/*
  GTI Poker, encrypted.
  PCB etched: "Games of Tennessee 88"

  Daughterboard in pos u18 with a 2732a EPROM + PAL20X4ACNS

  Etched:  GTI
           CIR rev a

  Diagram/schematics:

                2732A                               PAL20X4ACNS                       CONNECTOR
             .----v----.                             .---v---.                       .----v----.
   CON(01)---|01     24|--VCC               CON(20)|-|01   24|--VCC        2732(01)--|01     24|--VCC
   CON(02)---|02     23|-----------------. 2732(20)|-|02   23|--N/C        2732(02)--|02     23|--2732(23) & PAL(03)
   CON(03)---|03     22|---------------. '-----------|03   22|--CON(16)    2732(03)--|03     22|--2732(22) & PAL(04)
   CON(05)---|04     21|-------------. '-------------|04   21|--CON(17)    2732(05)--|04     21|--2732(21) & PAL(05)
   CON(04)---|05     20|--PAL(01-02) '---------------|05   20|--N/C        2732(04)--|05     20|--2732(20) & PAL(01-02)
   CON(08)---|06     19|----------.        2732(10)--|06   19|--N/C        2732(07)--|06     19|--2732(19) & PAL(07)
   CON(06)---|07     18|--PAL(18) '------------------|07   18|--2732(18)   2732(08)--|07     18|--/CE
   CON(07)---|08     17|-----------------------------|08   17|--N/C        2732(06)--|08     17|--PAL(21)
   CON(09)---|09     16|-----------------------------|09   16|--CON(15)    2732(09)--|09     16|--PAL(22)
   PAL(06)---|10     15|-----------------------------|10   15|-----------------------|10     15|--PAL(16)
   CON(11)---|11     14|--CON(14)          2732(13)--|11   14|--CON(13)    2732(11)--|11     14|--2732(14)
       GND---|12     13|--PAL(11)               GND--|12   13|--GND             GND--|12     13|--PAL(14)
             '---------'                             '-------'                       '---------'

*/
ROM_START( gtipokrc )  // norusa54
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "x13_2732a.u12",       0x0000, 0x1000, CRC(cee5b03c) SHA1(38a5885b4a95d7b3fa0dd099c160a5e4d854e00a) )
	ROM_LOAD( "88_x13_2732a_db.u18", 0x1000, 0x1000, CRC(f44cce3a) SHA1(f3e2a2a164d05a7ef121a7f0e872841553b6b2fe) )  // encrypted

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "gti_u31_2732a.u31", 0x0000, 0x1000, CRC(47e63fd2) SHA1(27bd1af9c910afb4e9955842f781954d5e8785d4) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "dm74s287n.u51", 0x0000, 0x0100, CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )
ROM_END

ROM_START( gtipokrd ) // norusa30
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_2732.u12",    0x0000, 0x1000, CRC(27a29a2c) SHA1(8f11405c65e0168f5ed33322c8739b6a9bcb90c8) )
	ROM_LOAD( "nosticker_db_2732.u18", 0x1000, 0x1000, CRC(e4638627) SHA1(e06ede46b064e37164ffa862501e785d1a1a1a49) )  // encrypted

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "n-c_u31_2732.u31", 0x0000, 0x1000, CRC(ba037f69) SHA1(8f9c325d2a250ee02ac42ffeccbe7af1fc2da6a9) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n.u51", 0x0000, 0x0100, CRC(c64f5b20) SHA1(ffbd46c59516f2f69cceb0bf423c489bdbe5d46d) )
ROM_END


/*

  HI-LO Double Up Joker Poker
  SMS Manufacturing Corp., 1983.
  ------------------------------

  Almost identical to DPHL.
  Only one different program rom.
  Seems to be patched with 2 extra subroutines.

  Hardware Layout (SMS HI-LO Double Up Joker Poker):

  - CPU:             1x AMD P8080A
  - RAM:             2x 2111A-2: Static Random Access Memory 256 x 4 bit.
  - RAM:             2x NEC D5101LC-1: 256x4 static CMOS RAM.
  - I/O:             3x 8255: Peripeheral Interface Adapter.
  - Prg ROMs:        2x 2732: U12,U18: Eprom.
  - Gfx ROMs:        1x 2716: U31: Eprom.
  - Sound:           Discrete.
  - Crystal:         1x 18.000 MHz.
  - PROM             1x 82S129: Bipolar PROM: U51.
                     1x 3.6 Vcc Battery.

  Frequency on CPU P8080A (pins 15 & 22) = 2.00032 MHz.


  PCB MARKED:

  Solderside:
  PCB silksceened: SMS Manufacturing Corporation.

  Component side:
  PCB silksceened: REV 2.
  PCB Engraved: "1350" "10-83".


  PCB Layout (SMS HI-LO Double Up Joker Poker):                                            Edge Connector 36x2
  .------------------------------------------------------------------------------------------------------------.
  | .---------.                          .---------.  .---------.  .-----.       .........    .---------.      |
  | |HCF4093BE|         NO IC            | 74174PC |  | 82S129N |  |NE555|       .........    |ULN2003A |      |
  | '---------'                          '---------'  '---------'  '-----'      916C471X2PE   '---------'      |
  |    U54               U53                 U52          U51        U50            U49          U48           |
  |.--------------------.                                                                                      |
  || 3.6v NI-CD BATTERY |                                                                                      |
  |'--------------------'                                                                                      |
  |.---------.        .---------.        .---------.               .---------.  .---------.   .---------.      |
  ||CD4040BE |        | 74123PC |        | 74157PC |     NO IC     | 74161   |  |  7486   |   |ULN2003A |      |
  |'---------'        '---------'        '---------'               '---------'  '---------'   '---------'      |
  |    U47                U46                U45          U44          U43          U42           U41          |
  |                                                                                                            |
  |                                                                                                            | 36
  |.---------.     .---------.           .---------.  MDP1601 471G .---------.                .---------.      '---.
  ||D5101LC-1|     |  7404   |           |SN74166J |   .........   | 74161N  |     NO IC      |ULN2003A |       ---|
  |'---------'     '---------'           '---------'   .........   '---------'                '---------'       ---|
  |    U40             U39                   U38          U37          U36          U35           U34           ---|
  |                                                                                         .----------------.  ---|
  |                            .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |   D8255AC-5    |  ---|
  |.---------.                 |    2716     |  |1|2|3|4|5|6|7|8|  | 74161   |  | 74157   | |                |  ---|
  ||D5101LC-1|       NO IC     |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |'---------'                 '-------------'     DIP SW x 8          U29          U28            U27          ---|
  |    U33            U32            U31               U30                                                      ---|
  |                                                                                         .----------------.  ---|
  |                                                                                         |   D8255AC-5    |  ---|
  |                                .---------.     .---------.     .---------.  .---------. |                |  ---|
  |   NO IC          NO IC         | 2111A-2 |     | 2111A-2 |     | 74161   |  | 74157   | '----------------'  ---|
  |                                '---------'     '---------'     '---------'  '---------'        U20          ---|
  |    U26            U25              U24             U23             U22          U21                         ---|
  |                                                                                                             ---|
  |               .--------------.     .----------------.                                                       ---|
  |               |     2732     |     |   D8255AC-5    |          .---------.  .---------.   MDP1601 471G      ---|
  |   NO IC       |              |     |                |          | 74161   |  | 74157   |     .........       ---|
  |               '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |    U19              U18                   U17                      U16          U15            U14         .---'
  |                                                                                                            | 01
  |               .--------------.       .------------.     .---------.     .---------.      .---------.       |
  |.---------.    |     2732     |       | NEC B8228  |     | 74161N  |     |  7486   |      |  7404   |       |
  ||74LS 541F|    |              |       |            |     '---------'     '---------'      '---------'       |
  |'---------'    '--------------'       '------------'         U10             U9               U8            |
  |    U13              U12                   U11           XTAL                                               |
  |                                                        .----. 18Mhz                                        |
  |.---------.    .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  ||  7405   |    |    AMD   P8080A    |   | SN74155N |    |UPB 8224 |  | 74157   |  |  7411   |  |  7474   |  |
  |'---------'    |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  |    U7         '--------------------'        U5              U4           U3           U2           U1      |
  |                        U6                                                                                  |
  '------------------------------------------------------------------------------------------------------------'


  SMS HI-LO Double Up Joker Poker discrete audio circuitry:
  --------------------------------------------------------

  3x ULN2003A (Darlington transistor array)
  1x NE555P   (Timer)
  1x PN2222   (Transistor)

  .------.                              .------------.              .-------.
  |  U17 |                              |   NE555P   |              |PN2222 |
  |      |                             4|            |3     R3      |       |
  |   PC7|------------------------------|RST      OUT|-----ZZZZ-----|B     E|-------> Audio Out.
  |   PC6|----------.                  6|            |8             |   C   |
  |   PC5|-----.    |3-in         .-----|THR      VCC|-----------.  '---+---'  .----> Audio Out.
  |   PC4|--.  |  .-+------.      |    5|            |7          |      |      |
  |      |  |  |  |ULN2003A|      |  .--|CVOLT   DISC|--.        |      |      |
  |      |  |  |  '-+------'      |  |  |            |  |        +------'    --+--
  |      |  |  |    |3-out   C1   |  |  |    GND     |  |        |            GND
  | 8255 |  |  |    '--------||---+  |  '-----+------'  |        |             -
  |      |  |  '--.               |  |        |1        |        |             '
  |      |  |     |2-in           |  |        |         |        |
  '------'  |  .--+-----.         |  |   C5   |         |        |
            |  |ULN2003A|         |  '---||---+         |        |   +5V
            |  '--+-----'         |           |         |        |   -+-
            |     |2-out     C2   |      C4   |         |    C6  |    |
            |     '----------||---+------||---+-------. | .--||--+----'
            |2-in                 |           |       | | |      |
          .-+------.              |         --+--      '-'       |
          |ULN2003A|              |          GND        |        |
          '-+------'              |           -         |        |
            |2-out           C3   |     R1    '         |   R2   |
            '----------------||---+----ZZZZ-------------+--ZZZZ--'

  R1 = 120 K ; Tolerance +/- 5%
  R2 = 1 K   ; Tolerance +/- 5%
  R3 = 1 K   ; Tolerance +/- 5%

  C1 = 103K = 10000 pF  =  10 nF = 0.01 uF
  C2 = .022 Z
  C3 = .05M
  C4 = .01 Z
  C5 = .01 Z
  C6 = .1 Z

  Similar circuitry and component values than DPHL PCB.

*/

ROM_START( smshilo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u12.bin", 0x0000, 0x1000, CRC(bd9acce8) SHA1(33e7e1805c03a704f9c8785b8e858310bfdc8b10) )
	ROM_LOAD( "u18.bin", 0x1000, 0x1000, CRC(06cf6789) SHA1(587d883c399348b518e3be4d1dc2581824055328) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(            0x0000, 0x0800, 0xff )
	ROM_LOAD( "u31.bin", 0x0800, 0x0800, CRC(412fc492) SHA1(094ea0ffd0c22274cfe164f07c009ffe022331fd) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "u51.bin", 0x0000, 0x0100, CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )
ROM_END

ROM_START( smshiloa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_27c256.u12", 0x0000, 0x8000, CRC(32e0b8ab) SHA1(8055231b8911a7bc519f8204f8b0045958ef84f2) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	//ROM_FILL(            0x0000, 0x0800, 0xff )
	ROM_LOAD( "u31_top_draw_2732a.u31", 0x0000, 0x1000, CRC(d0291d61) SHA1(4db617f1683bd07b0e124a0566691535f56b66de) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n.u51", 0x0000, 0x0100, CRC(e982df41) SHA1(ddea558af8ff95913aef42bdc712b3974d621dc0) )
ROM_END

/*

  DRHL
  Drews Revenge (poker)

  .u26  2732  stickered DRHL V2.89 U-26
  .u19  2732  stickered DRHL V2.89 U-19
  .u12  2732  stickered DRHL V2.89 U-12
  .u18  2732  stickered DRHL V2.89 U-18
  .u31  2732  stickered DRHL V1.0  U 31
  .u51  ampal16l8pc  printed 147-pal

  ROM text showed poker text and
  COPYRIGHT 1986 DREWS INC.
  COPYRIGHT 1986 DREW'S DISTRIBUTING INC.

*/

ROM_START( drhl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drhl_v2.89.u12", 0x0000, 0x1000, CRC(a0f63638) SHA1(e8046b4042ca1e203d831de70da2bf940b2094a0) )
	ROM_LOAD( "drhl_v2.89.u18", 0x1000, 0x1000, CRC(f3590633) SHA1(bb4a186d13b24ffd22e291c8d6c67f5012aa8001) )
	ROM_LOAD( "drhl_v2.89.u19", 0x2000, 0x1000, CRC(8abd7f40) SHA1(e8e489c670a17d4c48491b94bad1976cbb4742eb) )
	ROM_LOAD( "drhl_v2.89.u26", 0x3000, 0x1000, CRC(44ae3cdc) SHA1(51090c737873b652ba95435d553fb88fac730892) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "drhl_v1.0.u31",  0x0000, 0x1000, CRC(bbc7c970) SHA1(9268a430764a5ea8ba7cd18944ec254a44d9dff2) )

	ROM_REGION( 0x0200,  "plds", 0 )
	ROM_LOAD( "147-pal_ampal16l8pc.u51", 0x0000, 0x0104, CRC(4c98193f) SHA1(b6bdb6eef0d962a3aa4df0e23a8937a7e3210062) )
ROM_END

ROM_START( drhla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dspd_j_v.2.89.u12", 0x0000, 0x1000, CRC(8ac3bcfa) SHA1(9cc7c4529e18e4b3f7dabd65388604631bda2cc4) )
	ROM_LOAD( "dspd_j_v.2.89.u18", 0x1000, 0x1000, CRC(79b4c7af) SHA1(76abe5def47aaca17e5ec40f50841a12a1d8773b) )
	ROM_LOAD( "dspd_j_v.2.89.u19", 0x2000, 0x1000, CRC(6cce9025) SHA1(57d2e22df5be96082c13a3c1bcd1bc11849a6997) )
	ROM_LOAD( "dspd_j_v.2.89.u26", 0x3000, 0x1000, CRC(85be4d99) SHA1(96f067a5a3db6415929e71f3cf3c39614187ccec) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "drews.u31",  0x0000, 0x1000, CRC(bbc7c970) SHA1(9268a430764a5ea8ba7cd18944ec254a44d9dff2) )

	ROM_REGION( 0x0800,  "dallas", 0 )  // it's in fact NVRAM, but double sized...
	ROM_LOAD( "ds1220ab.u33",  0x0000, 0x0800, CRC(f357d314) SHA1(72791b2effd3ec2e98b735c9b215fc9abe3f5aea) )

	ROM_REGION( 0x0200,  "plds", 0 )  // this one is unprotected and seems ok
	ROM_LOAD( "pal16l8a.u51",  0x0000, 0x0104, CRC(4c98193f) SHA1(b6bdb6eef0d962a3aa4df0e23a8937a7e3210062) )
ROM_END

/*
  Southern Systems Joker Poker
  ----------------------------

  .u11  2723
  .u10  2732
  .u27  2716  couldn't get a good read

  8255 x3
  8080A
  unknown 28 ping chip @ u10
  open 16 pin socketa @ u41
  18.000 crystal



  $1fdf = call $0a0c --> draw 'bet' screen sector.
  $1fe2 = call $09ee --> draw 'credit' screen sector.

*/
ROM_START( ssjkrpkr )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Southern Systems
	ROM_LOAD( "oc.u11", 0x0000, 0x1000, CRC(b9072aa5) SHA1(bfa3df090e1030aaebbb784cb5e686f4f84f2263) )
	ROM_LOAD( "oc.u10", 0x1000, 0x1000, CRC(8652ebb9) SHA1(e907df4f8da99b42c425ed58da3cda9943c89fb7) )

	// The ROM is broken and can't get a consistent dump. All is garbage inside.
	// So, we are using a generic GFX ROM (from DPHLA set), modified to support
	// the apostrophe, question mark, and arrow down symbols needed for the game.
	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(           0x0000, 0x0800, 0xff )
	ROM_LOAD( "oc.u27", 0x0800, 0x0800, BAD_DUMP CRC(6de843f8) SHA1(4403178b491ba3d8115ebb631450d094be730edc) )
ROM_END


/*
  Fast Draw (Stern Electronics)?

  Text font is different to other similar games.

  The set was found as 'fastdraw'. No other info.
  Maybe is the poker conversion kit released by Stern as 'Fast Draw':

  https://flyers.arcade-museum.com/videogames/show/4362

*/
ROM_START( fastdrwp )
	ROM_REGION( 0x10000, "maincpu", 0 )  // PC=0x068b for error screen
	ROM_LOAD( "u12.bin", 0x0000, 0x1000, CRC(d020d7d3) SHA1(4808ef14adf230e3971161c9375f2b354cd9d519) )
	ROM_LOAD( "u18.bin", 0x1000, 0x1000, CRC(03de6413) SHA1(c61131244e8095b998c5e31724a21496cacad247) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(            0x0000, 0x0800, 0xff )
	ROM_LOAD( "u31.bin", 0x0800, 0x0800, CRC(6dd3a5b5) SHA1(e7978267ef8af31e65e6f278aebe82347bd5ffdd) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "u51_bpr.bin",  0x0000, 0x0100, CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )

	ROM_REGION( 0x400, "nvram", 0 )  // default NVRAM
	ROM_LOAD( "fastdrwp_nvram.bin", 0x000, 0x400, CRC(8b9f0c62) SHA1(b6c53444926811c4d3cd30317533ae8168aae843) )
ROM_END

/*
  Sure Shot
  SMS Manufacturing, 1993.

  No extra info inside the zip. Just ROM dumps...

*/
ROM_START( sureshoto )
	ROM_REGION( 0x10000, "maincpu", 0 )  // no stack, call's RET go to PC=0
	ROM_LOAD( "u-12_ss.u12", 0x0000, 0x1000, CRC(10ddbc16) SHA1(ab683d836c9223bc67701e092c2cb95afc0f0fa2) )
	ROM_LOAD( "u-18_ss.u18", 0x1000, 0x1000, CRC(ffbac2bf) SHA1(219247624e0eb0c0c805f5f9a96c4b6b60c9c5ac) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "u-31_ss.u31", 0x0000, 0x1000, CRC(7afa583e) SHA1(e897c6dbcc5452fdb99894203131886a529eed37) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n_2",  0x0000, 0x0100, CRC(ee452994) SHA1(315913ce4a92fe0ea7b76e862507c933d6104616) )
ROM_END

/*
  Unknown DPHL rev 2.

  No extra info inside the zip. Just ROM dumps...
  Maybe from SMS Manufacturing, since there are GFX tiles with the SMS logo.

*/
ROM_START( dphlunkb )
	ROM_REGION( 0x10000, "maincpu", 0 )  // PC=0x068b for error screen
	ROM_LOAD( "u-12_rev-2.u12", 0x0000, 0x1000, CRC(1b1d8ca4) SHA1(405bf8a56dfc669a0890b0af9417c1ed6a3bf374) )
	ROM_LOAD( "u-18_rev-2.u18", 0x1000, 0x1000, CRC(22dbe0c7) SHA1(ca223074b0f4b86e60a1b91c22568680845ae17e) )

	ROM_REGION( 0x1000,  "gfx", 0 )  // original dump cames with sureshot gfx surely by mistake. using gfx from dphl
	ROM_LOAD( "cgi_3939.u31",   0x0000, 0x1000, BAD_DUMP CRC(2028db2c) SHA1(0f81bb71e88c60df3817f58c28715ce2ea01ad4d) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "n82s129n_1",  0x0000, 0x0100, CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )
	ROM_LOAD( "n82s129n_2",  0x0100, 0x0100, CRC(ee452994) SHA1(315913ce4a92fe0ea7b76e862507c933d6104616) )
ROM_END

/*
  Unknown board silkscreened PKII/DM (made in Japan)

  1x Sharp LH0080A (Z80A)
  3x 8255 (2x Mitsubishi M5L8255AP, 1x Toshiba TMP8255AP)

  2x 2732 (program ROMs U12 & U18)
  1x 2716 (GFX ROM U31)
  1x 63S141N (bipolar PROM U51)

  Xtal 18.000 MHz.

*/
ROM_START( pkii_dm )
	ROM_REGION( 0x10000, "maincpu", 0 )  // no stack, call's RET go to PC=0
	ROM_LOAD( "12.u12", 0x0000, 0x1000, CRC(048e70d8) SHA1(f0eb16ba68455638de2ce68f51f305a13d0df287) )
	ROM_LOAD( "13.u18", 0x1000, 0x1000, CRC(06cf6789) SHA1(587d883c399348b518e3be4d1dc2581824055328) )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_FILL(                 0x0000, 0x0800, 0xff )
	ROM_LOAD( "cgw-f506.u31", 0x0800, 0x0800, CRC(412fc492) SHA1(094ea0ffd0c22274cfe164f07c009ffe022331fd) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "63s141n.u51",  0x0000, 0x0100, CRC(88302127) SHA1(aed1273974917673405f1234ab64e6f8b3856c34) )
ROM_END


ROM_START( pokplus ) // encrypted, has 'Copyright  1982, 1989 LJF Corporation' plaintext in ROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nickphan.u3", 0x0000, 0x8000, CRC(bb03826c) SHA1(249283427dcbfbd16d02cad8e5885e651085954f) )

	ROM_REGION( 0x1000,  "gfx", 0 ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "phan.u31", 0x0000, 0x1000, CRC(0501ac8f) SHA1(98154a049db2343a8ab12193ddf5c547590fc625) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "phanprom.pr", 0x0000, 0x0100, CRC(fc2aace1) SHA1(eba975274da310196db53f799112d8bbac1d0ca8) )
ROM_END

ROM_START( pokplusa ) // encrypted, has 'Copyright  1982, 1989 LJF Corporation' plaintext in ROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u3_subpcb.u3", 0x0000, 0x8000, CRC(aaecf689) SHA1(fe6f9763365d82a59999a70aa1bfa533f980637c) )

	ROM_REGION( 0x1000,  "gfx", 0 )  // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "u-31_char_gen_revb_2732.u31", 0x0000, 0x1000, CRC(0501ac8f) SHA1(98154a049db2343a8ab12193ddf5c547590fc625) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "82s129.r13", 0x0000, 0x0100, CRC(fc2aace1) SHA1(eba975274da310196db53f799112d8bbac1d0ca8) )
ROM_END

ROM_START( pokplusb ) // encrypted, has 'Copyright  1982, 1989 LJF Corporation' plaintext in ROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u3_subpcb.u3", 0x0000, 0x8000, CRC(fd3dddcc) SHA1(8024e4f2f4e72b8d554c34ea2a0f74ee84312f96) )

	ROM_REGION( 0x1000,  "gfx", 0 )  // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "u31_2732.u31", 0x0000, 0x1000, CRC(3cbdc7e0) SHA1(05ebefcd989e64e425b35b7bc23769214990d4d4) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "82s129.r13", 0x0000, 0x0100, CRC(ea2fa08e) SHA1(5ca2afcce9395a6421ab5ead02855583cb619328) )
ROM_END


/*
   Hardware Layout (PCB2):

  - CPU:             1x 8080
  - RAM:             2x 2111A-2: Static Random Access Memory 256 x 4 bit.
  - RAM:             1X 6116-3
  - I/O:             3x 8255: Peripeheral Interface Adapter.
  - Prg ROMs:        2x 2732: U12,U18: Eprom.
  - Gfx ROMs:        1x 2732: U31: Eprom.
  - Sound:           Discrete.
  - Crystal:         1x 18.000 MHz.
  - PROM             1x 82S129: Bipolar PROM: U51.
                     1x 3.6 Vcc Battery.

  PCB/EPROM MARKINGS = NONE
  U51 LABEL:98CC


  PCB Layout (PCB2):                                                                       Edge Connector 36x2
  .------------------------------------------------------------------------------------------------------------.
  | .---------.                          .---------.  .---------.  .-----.       .........    .---------.      |
  | |HCF4093BE|         NO IC            | 74174PC |  | 82S129N |  |NE555|       .........    |ULN2003A |      |
  | '---------'                          '---------'  '---------'  '-----'      916C471X2PE   '---------'      |
  |    U54               U53                 U52          U51        U50            U49          U48           |
  |.--------------------.                                                                                      |
  || 3.6v NI-CD BATTERY |                                                                                      |
  |'--------------------'                                                                                      |
  |.---------.        .---------.        .---------.               .---------.  .---------.   .---------.      |
  ||CD4040BE |        | 74123PC |        | 74157PC |     NO IC     | 74161   |  |  7486   |   |ULN2003A |      |
  |'---------'        '---------'        '---------'               '---------'  '---------'   '---------'      |
  |    U47                U46                U45          U44          U43          U42           U41          |
  |                                                                                                            |
  |                                                                                                            | 36
  |                .---------.           .---------.  MDP1601 471G .---------.                .---------.      '---.
  |                |  7404   |           |SN74166J |   .........   | 74161N  |     NO IC      |ULN2003A |       ---|
  |   NO IC        '---------'           '---------'   .........   '---------'                '---------'       ---|
  |    U40             U39                   U38          U37          U36          U35           U34           ---|
  |                                                                                         .----------------.  ---|
  |                            .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |      8255      |  ---|
  |                            |    2732     |  |1|2|3|4|5|6|7|8|  | 74161   |  | 74157   | |                |  ---|
  |                  NO IC     |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |   NO IC                    '-------------'         U30             U29          U28            U27          ---|
  |    U33            U32            U31            DIP SW x 8                                                  ---|
  |                                                                                         .----------------.  ---|
  |                                                                                         |      8255      |  ---|
  | .---------.                    .---------.     .---------.     .---------.  .---------. |                |  ---|
  | |  6116   |       NO IC        | 2111A-2 |     | 2111A-2 |     | 74161   |  | 74157   | '----------------'  ---|
  | '---------'                    '---------'     '---------'     '---------'  '---------'        U20          ---|
  |    U26             U25             U24             U23             U22          U21                         ---|
  |                                                                                                             ---|
  |               .--------------.     .----------------.                                                       ---|
  |               |     2732     |     |      8255      |          .---------.  .---------.   MDP1601 471G      ---|
  |   NO IC       |              |     |                |          | 74161   |  | 74157   |     .........       ---|
  |               '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |    U19              U18                   U17                      U16          U15            U14         .---'
  |                                                                                                            | 01
  |               .--------------.       .------------.     .---------.     .---------.      .---------.       |
  |.---------.    |     2732     |       |    8228    |     | 74161N  |     |  7486   |      |  7404   |       |
  ||74LS 541F|    |              |       |            |     '---------'     '---------'      '---------'       |
  |'---------'    '--------------'       '------------'         U10             U9               U8            |
  |    U13              U12                   U11           XTAL                                               |
  |                                                        .----. 18Mhz                                        |
  |.---------.    .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  ||  7405   |    |        8080        |   |   74155  |    |  8224   |  | 74157   |  |  7411   |  |  7474   |  |
  |'---------'    |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  |    U7         '--------------------'        U5              U4           U3           U2           U1      |
  |                        U6                                                                                  |
  '------------------------------------------------------------------------------------------------------------'

*/
ROM_START( krampcb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prog2732.u12", 0x0000, 0x1000, CRC(de60748c) SHA1(b1e2690ba31e3e38fd0ec2504ee8fc562ab856bf) )
	ROM_LOAD( "prog2732.u18", 0x1000, 0x1000, CRC(4559dd72) SHA1(2120bff08a125dcef01c0e133d52354d78802d15) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "char2732.u31", 0x0000, 0x1000, CRC(4711c041) SHA1(41e458050ac56d05d15d3eb4b3c093fc5aa442eb) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "98cc.u51", 0x0000, 0x0100,     CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )
ROM_END

/*
   Set similar to pkii_dm, but with different bipolar PROM,
   and running in a different hardware.

   Hardware Layout(PCB5) :

  - CPU:             1x 8080
  - RAM:             2x 2111A-2: Static Random Access Memory 256 x 4 bit.
  - I/O:             3x 8255: Peripeheral Interface Adapter.
  - Prg ROMs:        2x 2732: U12,U18: Eprom.
  - Gfx ROMs:        1x 2716: U31: Eprom.
  - Sound:           Discrete.
  - Crystal:         1x 18.000 MHz.
  - PROM             1x 82S129: Bipolar PROM: U51.
  -BATTERY ADDON PCB:
   LABELED:NONE
   BATTERY PCB CONTAINS:
   1x CD4093
   1x 3.6V NI-CD
   1x CD4040
   2x NEC D5101LC-1: 256x4 static CMOS RAM.


  PCB MARKINGS Silkscreened: NONE

  PCB MARKED:5239
  PCB MARKED:8310

  EPROM MARKINGS:
  U12: U12 ANTI STAT CB0A
  U18: U18 ANTI STAT 36E3
  U31: U31 CGIT F506
  U51: 98CE

  PCB Layout (PCB5):                                                                       Edge Connector 36x2
  .------------------------------------------------------------------------------------------------------------.
  |PIN'S BATTERY PCB                     .---------.  .---------.  .-----.       .........    .---------.      |
  | ***************                      | 74174PC |  | 82S129N |  |NE555|       .........    |ULN2003A |      |
  |   NO IC             NO IC            '---------'  '---------'  '-----'       16-2-472     '---------'      |
  |    U54               U53                 U52          U51        U50            U49          U48           |
  |                                                                                                            |
  |                                                                                                            |
  |                                                                                                            |
  |                   .---------.        .---------.               .---------.  .---------.   .---------.      |
  |                   | 74123PC |        | 74157PC |     NO IC     | 74161   |  |  7486   |   |ULN2003A |      |
  |   NO IC           '---------'        '---------'               '---------'  '---------'   '---------'      |
  |    U47                U46                U45          U44          U43          U42           U41          |
  |                                                                                                            |
  |                                                                                                            | 36
  |                .---------.           .---------.   16-2-472    .---------.                .---------.      '---.
  |                |  7404   |           |SN74166J |   .........   | 74161N  |     NO IC      |ULN2003A |       ---|
  |   NO IC        '---------'           '---------'   .........   '---------'                '---------'       ---|
  |    U40             U39                   U38          U37          U36          U35           U34           ---|
  |                                                                                         .----------------.  ---|
  |PIN'S BATTERY PCB           .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |      8255      |  ---|
  | *************              |    2716     |  |1|2|3|4|5|6|7|8|  | 74161   |  | 74157   | |                |  ---|
  |                  NO IC     |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |   NO IC                    '-------------'         U30             U29          U28            U27          ---|
  |    U33            U32            U31            DIP SW x 8                                                  ---|
  |                                                                                         .----------------.  ---|
  |                                                                                         |      8255      |  ---|
  |                                .---------.     .---------.     .---------.  .---------. |                |  ---|
  |                                | 2111A-2 |     | 2111A-2 |     | 74161   |  | 74157   | '----------------'  ---|
  |   NO IC           NO IC        '---------'     '---------'     '---------'  '---------'        U20          ---|
  |    U26             U25             U24             U23             U22          U21                         ---|
  |                                                                                                             ---|
  |               .--------------.     .----------------.                                                       ---|
  |               |     2732     |     |      8255      |          .---------.  .---------.     16-2-472        ---|
  |   NO IC       |              |     |                |          | 74161   |  | 74157   |     .........       ---|
  |               '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |    U19              U18                   U17                      U16          U15            U14         .---'
  |                                                                                                            | 01
  |               .--------------.       .------------.     .---------.     .---------.      .---------.       |
  |.---------.    |     2732     |       |    8228    |     | 74161N  |     |  7486   |      |  7404   |       |
  || 74LS541 |    |              |       |            |     '---------'     '---------'      '---------'       |
  |'---------'    '--------------'       '------------'         U10             U9               U8            |
  |    U13              U12                   U11           XTAL                                               |
  |                                                        .----. 18Mhz                                        |
  |.---------.    .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  ||  7405   |    |        8080        |   |  74155   |    |   8224  |  | 74157   |  |  7411   |  |  7474   |  |
  |'---------'    |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  |    U7         '--------------------'        U5              U4           U3           U2           U1      |
  |                        U6                                                                                  |
  '------------------------------------------------------------------------------------------------------------'

*/
ROM_START( krampcb5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "anti_stat_cb0a.u12", 0x0000, 0x1000, CRC(048e70d8) SHA1(f0eb16ba68455638de2ce68f51f305a13d0df287) )
	ROM_LOAD( "anti_stat_36e3.u18", 0x1000, 0x1000, CRC(06cf6789) SHA1(587d883c399348b518e3be4d1dc2581824055328) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_FILL(                      0x0000, 0x0800, 0xff )
	ROM_LOAD( "cg1t_f506.u31",     0x0800, 0x0800, CRC(412fc492) SHA1(094ea0ffd0c22274cfe164f07c009ffe022331fd) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "98ce.u51",        0x0000, 0x0100, CRC(812dc1f1) SHA1(b2af33ff36f2eca2f782bc2239bc9e54c2564f6a) )
ROM_END


/*
  PCB MARKINGS Silkscreened = PATENT PENDING. COPYRIGHT 1983 M.KRAMER MFG.INC. MODEL 3000 M.KRAMER INC

  PCB Layout (PCB3, PCB4, PCB6):                                                            Edge Connector 36x2
  .------------------------------------------------------------------------------------------------------------.
  |PIN'S BATTERY PCB   .-----.           .---------.  .---------.  .-----.       .........    .---------.      |
  | ***************    |LM393|           | 74174PC |  | 82S129N |  |NE555|       .........    |ULN2003A |      |
  |   NO IC            '-----'           '---------'  '---------'  '-----'       16-2-472     '---------'      |
  |    U54               U53                 U52          U51        U50            U49          U48           |
  |                                                                                                            |
  |                                                                                                            |
  |                                                                                                            |
  |                   .---------.        .---------.               .---------.  .---------.   .---------.      |
  |                   | 74123PC |        | 74157PC |     NO IC     | 74161   |  |  7486   |   |ULN2003A |      |
  |   NO IC           '---------'        '---------'               '---------'  '---------'   '---------'      |
  |    U47                U46                U45          U44          U43          U42           U41          |
  |                                                                                                            |
  |                                                                                                            | 36
  |                .---------.           .---------.   16-2-472    .---------.                .---------.      '---.
  |                |  7404   |           |SN74166J |   .........   | 74161N  |     NO IC      |ULN2003A |       ---|
  |   NO IC        '---------'           '---------'   .........   '---------'                '---------'       ---|
  |    U40             U39                   U38          U37          U36          U35           U34           ---|
  |                                                                                         .----------------.  ---|
  |PIN'S BATTERY PCB           .-------------.  .-.-.-.-.-.-.-.-.  .---------.  .---------. |      8255      |  ---|
  | *************              |    2732     |  |1|2|3|4|5|6|7|8|  | 74161   |  | 74157   | |                |  ---|
  |                  NO IC     |             |  '-'-'-'-'-'-'-'-'  '---------'  '---------' '----------------'  ---|
  |   NO IC                    '-------------'         U30             U29          U28            U27          ---|
  |    U33            U32            U31            DIP SW x 8                                                  ---|
  |                                                                                         .----------------.  ---|
  |                                                                                         |      8255      |  ---|
  |                 .---------.    .---------.     .---------.     .---------.  .---------. |                |  ---|
  |                 | 2111A-2 |    | 2111A-2 |     | 2111A-2 |     | 74161   |  | 74157   | '----------------'  ---|
  |   NO IC         '---------'    '---------'     '---------'     '---------'  '---------'        U20          ---|
  |    U26             U25             U24             U23             U22          U21                         ---|
  |                                                                                                             ---|
  |               .--------------.     .----------------.                                                       ---|
  |               |     2764     |     |      8255      |          .---------.  .---------.     16-2-472        ---|
  |   NO IC       |              |     |                |          | 74161   |  | 74157   |     .........       ---|
  |               '--------------'     '----------------'          '---------'  '---------'     .........       ---|
  |    U19              U18                   U17                      U16          U15            U14         .---'
  |                                                                                                            | 01
  |               .--------------.       .------------.     .---------.     .---------.      .---------.       |
  |.---------.    |     2764     |       |    8228    |     | 74161N  |     |  7486   |      |  7404   |       |
  || 74LS244 |    |              |       |            |     '---------'     '---------'      '---------'       |
  |'---------'    '--------------'       '------------'         U10             U9               U8            |
  |    U13              U12                   U11           XTAL                                               |
  |                                                        .----. 18Mhz                                        |
  |.---------.    .--------------------.   .----------.    .---------.  .---------.  .---------.  .---------.  |
  ||  7402   |    |        8080        |   |  74155   |    |   8224  |  | 74157   |  |  7411   |  |  7474   |  |
  |'---------'    |                    |   '----------'    '---------'  '---------'  '---------'  '---------'  |
  |    U7         '--------------------'        U5              U4           U3           U2           U1      |
  |                        U6                                                                                  |
  '------------------------------------------------------------------------------------------------------------'

   Hardware Layout(PCB3) :

  - CPU:             1x 8080
  - RAM:             3x 2111A-2: Static Random Access Memory 256 x 4 bit.
  - I/O:             3x 8255: Peripeheral Interface Adapter.
  - Prg ROMs:        2x 2764: U12, U18: Eprom.
  - Gfx ROMs:        1x 2732: U31: Eprom.
  - Sound:           Discrete.
  - Crystal:         1x 18.144 MHz.
  - PROM             1x 82S129: Bipolar PROM: U51.

  - BATTERY ADDON PCB: ENCASED IN RESIN
    LABELED: MODEL 4000 BATTERY BOARD

  PCB MARKINGS Silkscreened = PATENT PENDING. COPYRIGHT 1983 M.KRAMER MFG.INC. MODEL 3000 M.KRAMER INC
  R.A.B.
  W.S.B.
  DECMO 3084

  STICKERED: DEC 14'84
  STICKERED: DEC 17'84
  STICKERED: TESTED OK W.S.B 6998
  PCB MARKED:6998

  EPROM MARKINGS:
  U12: 6000 U12 5-25
  U18: 6000 U18
  U31: U31B MOD3000 CF7B
  U51: U51 0726

*/
ROM_START( krampcb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6000_5-25.u12",     0x0000, 0x2000, CRC(e98cf05f) SHA1(929dcb8e30535465f1229004bb35872c520ede40) )
	ROM_LOAD( "6000.u18",          0x2000, 0x2000, CRC(01a5014f) SHA1(9240259d094c77a146304f3dc7d3cd6f66d25018) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "mod3000_cf7b.u31",  0x0000, 0x1000, CRC(fcfc4d25) SHA1(31455903244ec8ef9005748f265f561b7a082a9c) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "0726.u51",          0x0000, 0x0100, CRC(d1a2e2be) SHA1(9593e345aab794a0171900bc302a89a7f4f05b27) )
ROM_END

/*
  Hardware Layout(PCB4) :

  - CPU:             1x 8080
  - RAM:             3x 2111A-2: Static Random Access Memory 256 x 4 bit.
  - I/O:             3x 8255: Peripeheral Interface Adapter.
  - Prg ROMs:        2x 2764: U12, U18: Eprom.
  - Gfx ROMs:        1x 2732: U31: Eprom.
  - Sound:           Discrete.
  - Crystal:         1x 18.144 MHz.
  - PROM             1x 82S129: Bipolar PROM: U51.

  - BATTERY ADDON PCB:ENCASED IN RESIN
    LABELED:14690 MODEL 4036 BATTERY BOARD

  PCB MARKINGS Silkscreened = PATENT PENDING. COPYRIGHT 1983 M.KRAMER MFG.INC. MODEL 3000 M.KRAMER INC
  R.A.B.
  W.S.B.

  STICKERED: TEST 1 JUN 12'84
  STICKERED: TEST 3 JUN 5 '84
  PCB MARKED:5253

  EPROM MARKINGS:
  U12: JPB M8 USA U12 5/25
  U18: JPB U18 USA M8 5/25
  U31: U31 RM03031US0 CF7B
  U51: U51-06EC

*/
ROM_START( krampcb4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpb_m8_5-25.u12",      0x0000, 0x2000, CRC(e5022d2d) SHA1(f7a7d90b03fd3158d0f2919f2c852e43e15bc89f) )
	ROM_LOAD( "jpb_usa_m8_5-25.u18",  0x2000, 0x2000, CRC(c05e1382) SHA1(08f3bc0c0b5c39cbc29dcc8921b20a8dd2392772) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "rm03031us0_cf7b.u31",  0x0000, 0x1000, CRC(fcfc4d25) SHA1(31455903244ec8ef9005748f265f561b7a082a9c) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "06ec.u51",             0x0000, 0x0100, CRC(ad33e582) SHA1(355cdf3cdc34f8c56da1f815b1ae86bba5a19821) )
ROM_END

/*
 Hardware Layout(PCB6) :

  - CPU:             1x 8080
  - RAM:             3x 2111A-2: Static Random Access Memory 256 x 4 bit.
  - I/O:             3x 8255: Peripeheral Interface Adapter.
  - Prg ROMs:        2x 2764: U12, U18: Eprom.
  - Gfx ROMs:        1x 2732: U31: Eprom.
  - Sound:           Discrete.
  - Crystal:         1x 18.144 MHz.
  - PROM             1x 82S129: Bipolar PROM: U51.

  - BATTERY ADDON PCB: ENCASED IN RESIN
    LABELED: MODEL 16304 4000 BATTERY BOARD

  PCB MARKINGS Silkscreened = PATENT PENDING. COPYRIGHT 1983 M.KRAMER MFG.INC. MODEL 3000 M.KRAMER INC
  R.A.B.
  W.S.B.
  DECMO 3384

  STICKERED: TEST 5 NOV 31 '84
  STICKERED: TEST 3 NOV 27 '84
  STICKERED: K TESTED OK W.S.B 6759 NOV 27 84
  PCB MARKED:6759

  EPROM MARKINGS:
  U12: U12 JF30 TN 5 25 6200-1F06
  U18: U18 JF30 MODEL 6000 5E4F--33CB
  U31: U31B MOD3000 CF7B
  U51: U51-06EC

*/
ROM_START( krampcb6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jf30_tn_5_25_6200-1f06.u12",      0x0000, 0x2000, CRC(411d9b9d) SHA1(1db50fe6435d838eae44c8f4d21bc9c7fdf21234) )
	ROM_LOAD( "jf30_model_6000_5e4f--33cb.u18",  0x2000, 0x2000, CRC(4efab715) SHA1(2e9f05ff917a516091642d1b213641d39bfa604a) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "mod3000_cf7b.u31",  0x0000, 0x1000, CRC(fcfc4d25) SHA1(31455903244ec8ef9005748f265f561b7a082a9c) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "06ec.u51",          0x0000, 0x0100, CRC(ad33e582) SHA1(355cdf3cdc34f8c56da1f815b1ae86bba5a19821) )
ROM_END

/*
  Original M.Kramer PCB
  with an epoxy block

  US Rev L5

*/
ROM_START( krampcb7 )  // norusa03
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u12usrevl5_rm10012us0_37b7--15a0_2764.u12", 0x0000, 0x2000, CRC(7963c301) SHA1(684a42f10dbb02b47b8365e5edf5753804852c5d) )
	ROM_LOAD( "u18usrevl5_rm10018us0_0e85--bd56_2764.u18", 0x2000, 0x2000, CRC(bf7a2e52) SHA1(5cd5319e752451aa5ec8c8650a29a9901b4314dd) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "u31_rm03031us0_cf7b_2732a.u31", 0x0000, 0x1000, CRC(fcfc4d25) SHA1(31455903244ec8ef9005748f265f561b7a082a9c) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "82s129n.u51",  0x0000, 0x0100, CRC(ad33e582) SHA1(355cdf3cdc34f8c56da1f815b1ae86bba5a19821) )
ROM_END

ROM_START( krampcb8 )  // norusa22
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_2764a.u12", 0x0000, 0x2000, CRC(4c65c740) SHA1(1c417246555c50765e7e31c3dfaf5f0e6c66ceb8) )
	ROM_LOAD( "nosticker_2764a.u18", 0x2000, 0x2000, CRC(78ffe0ac) SHA1(643de781837cb3798db355c3d17d10c392d3f7e8) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "u31_cf7b_2732a.u31", 0x0000, 0x1000, CRC(fcfc4d25) SHA1(31455903244ec8ef9005748f265f561b7a082a9c) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "06ec_n82s129n.u51",  0x0000, 0x0100, CRC(ad33e582) SHA1(355cdf3cdc34f8c56da1f815b1ae86bba5a19821) )
ROM_END

ROM_START( krampcb9 )  // norusa42
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_db_27c256.u12", 0x0000, 0x8000, CRC(32e0b8ab) SHA1(8055231b8911a7bc519f8204f8b0045958ef84f2) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "u31_2732a.u31", 0x0000, 0x1000, CRC(d0291d61) SHA1(4db617f1683bd07b0e124a0566691535f56b66de) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n.u51",  0x0000, 0x0100, CRC(e982df41) SHA1(ddea558af8ff95913aef42bdc712b3974d621dc0) )
ROM_END

ROM_START( topdraw )  // norusa36
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_db_am27c256.u12", 0x0000, 0x8000, CRC(e451df9c) SHA1(30a2c42d87301df5fb82a59326c9754007737325) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "u-31_top_draw_2732a.u31", 0x0000, 0x1000, CRC(d0291d61) SHA1(4db617f1683bd07b0e124a0566691535f56b66de) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n.u51",  0x0000, 0x0100, CRC(e982df41) SHA1(ddea558af8ff95913aef42bdc712b3974d621dc0) )
ROM_END

ROM_START( topdrawa )  // norusa38
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_db_m27c256.u12", 0x0000, 0x8000, CRC(38bfabf6) SHA1(afe1f962c7546b8fb891953efbdf68a11e8dd45a) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "nosticker_m2732a.u31", 0x0000, 0x1000, CRC(d0291d61) SHA1(4db617f1683bd07b0e124a0566691535f56b66de) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n.u51",  0x0000, 0x0100, CRC(e982df41) SHA1(ddea558af8ff95913aef42bdc712b3974d621dc0) )
ROM_END

ROM_START( topdrawb )  // norusa40
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_db_m27c256.u12", 0x0000, 0x8000, CRC(8e195061) SHA1(ed25fe65b164e048c3ba5078a7e443b696141807) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "nosticker_2732a.u31", 0x0000, 0x1000, CRC(d0291d61) SHA1(4db617f1683bd07b0e124a0566691535f56b66de) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n.u51",  0x0000, 0x0100, CRC(e982df41) SHA1(ddea558af8ff95913aef42bdc712b3974d621dc0) )
ROM_END

// identical to topdrawb, but with different bipolar PROM swapping colors for lines 5 to 8
// once the game is working, we can see if it's critical, or just remove the set
ROM_START( topdrawba )  // norusa48
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xx_db_nm27c256.u12", 0x0000, 0x8000, CRC(8e195061) SHA1(ed25fe65b164e048c3ba5078a7e443b696141807) )

	ROM_REGION( 0x1000, "gfx",0 )
	ROM_LOAD( "top_draw_u-31_bucks.u31", 0x0000, 0x1000, CRC(d0291d61) SHA1(4db617f1683bd07b0e124a0566691535f56b66de) )

	ROM_REGION( 0x0100,  "proms", 0 )
	ROM_LOAD( "n82s129n_bucks.u51",  0x0000, 0x0100, CRC(460f155c) SHA1(36be07c487fb7f474ce9249eb75ea670658767cf) )
ROM_END


/****************************** Sets with MCU ********************************/

/*
  Turbo Poker I
  Micro Manufacturing.

  Looks generic noraut hardware, but with one daughterboard
  having a program ROM, a MCU, a PLD, and a Dallas or MK48Z02

*/
ROM_START(tpoker1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nosticker_db_27256.ic2", 0x0000, 0x8000, CRC(55eb90a8) SHA1(9460cb028f3186c7c09c18db1b413088812e7eef) )

	ROM_REGION( 0x0800,  "mcu", 0 )  // 68705?
	ROM_LOAD( "mcu.u35", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "tp-u31.u31", 0x0000, 0x1000, CRC(cbf8541d) SHA1(fac979a5e3389d9fe9a1ebaaec0903807e7a5910) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "n82s131n.u51", 0x0000, 0x0200, CRC(d94373cf) SHA1(b66f4f2b2df501a134620306d4f2086952984fd2) )
ROM_END


ROM_START(tpoker1a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "coindata_turbo_ic2_ver_13060_27c256.ic2", 0x0000, 0x8000, CRC(55eb90a8) SHA1(9460cb028f3186c7c09c18db1b413088812e7eef) )

	ROM_REGION( 0x0800,  "mcu", 0 )  // 68705?
	ROM_LOAD( "mcu.u35", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "coindata_turbo_poker_u_31.u31", 0x0000, 0x1000, CRC(cbf8541d) SHA1(fac979a5e3389d9fe9a1ebaaec0903807e7a5910) )

	ROM_REGION( 0x0800,  "mk48z02", 0 )
	ROM_LOAD( "mk48z02.ic3", 0x0000, 0x0800, CRC(67405705) SHA1(6bcaf8b4877cc01db5c381ce1586de0c1f34dcb4) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "n82s131n.u51", 0x0000, 0x0200, CRC(d94373cf) SHA1(b66f4f2b2df501a134620306d4f2086952984fd2) )
ROM_END


/*
  TURBO POKER 2 (100491-1)
  1991 by MICRO MANUFACTURING.

*/
ROM_START(tpoker2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "turbo-2_u39-015_collins_101291.u39", 0x0000, 0x8000, CRC(35dac6e0) SHA1(01e73ac51b4a18f0ccc7964700cd1a7523f48b2b) )

	ROM_REGION( 0x0800,  "mcu", 0 )  // 68705
	ROM_LOAD( "tp2-01_u35-ic4_11067_032599.u35", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "turbo_poker_char_rom.u30", 0x0000, 0x1000, CRC(6df86e08) SHA1(a451f71db7b59500b99207234ef95793afc11f03) )

	ROM_REGION( 0x0800,  "nvram", 0 )  // nvram
	ROM_LOAD( "mk48z02.u44", 0x0000, 0x0800, CRC(fcb12763) SHA1(66a672c15db7f514d190f84fba023b2733d1f194) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "82s131n.u23", 0x0000, 0x0200, CRC(6834053a) SHA1(77270591aeb3ed06f72897b8f57302502f752336) )

	ROM_REGION( 0x02a1,  "plds", 0 )
	ROM_LOAD( "pal12l6.u37",  0x0000, 0x0034, CRC(25651948) SHA1(62cd4d73c6ca8ea5d4beb9ae262d1383f8149462) )
ROM_END


// second set
ROM_START(tpoker2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "turbo-2_u39-014_u5_ut81_051894.u39", 0x0000, 0x8000, CRC(543149fe) SHA1(beb61a27c2797341e23e020e754d63fde3b4fbb2) )

	ROM_REGION( 0x0800,  "mcu", 0 )  // 68705
	ROM_LOAD( "tp2_01_u35-ic4_10834_080494.u35", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "turbo_poker_char_rom.u30", 0x0000, 0x1000, CRC(6df86e08) SHA1(a451f71db7b59500b99207234ef95793afc11f03) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "82s131n.u23", 0x0000, 0x0200, CRC(6834053a) SHA1(77270591aeb3ed06f72897b8f57302502f752336) )

	ROM_REGION( 0x02a1,  "plds", 0 )
	ROM_LOAD( "pal12l6.u37",  0x0000, 0x0034, CRC(25651948) SHA1(62cd4d73c6ca8ea5d4beb9ae262d1383f8149462) )
ROM_END


/*
  Turbo Poker 2 (022190-1) by Micro MFG.
  ------------------------------------

  Hardware Layout (Turbo Poker 2 by Micro MFG):

  - CPU:             1x NEC D8080AFC-1 (U42).
  - BUS:             1x 8224 (U43)
  - RAM:             2x 2111-1 Static Random Access Memory 256 x 4 bit (U33 & U34).
  - I/O:             3x Intel P8255A Peripeheral Interface Adapter (U31, U36 & U38).
  - Prg ROMs:        1x 27256 (U39).
  - Gfx ROMs:        1x 2732 (U30).
  - Sound:           Discrete.
  - Crystal:         1x 18.000 MHz.

  - MCU:             1x Custom, based on 68705.


  Etched in copper on board:    TP2

  .U30  2732a    ; stickered  (c) 1993 MICRO MFG TURBO POKER CHAR, ROM.

  .U35  unknown  ; stickered  (c) 1993 MICRO MFG TP2#01 U35\IC4 16228 022194.

   Continuity errors when trying to read as a standard eprom.
   Silkscreened below the chip 'CUSTOM I.C.'. Looks like a normal EPROM.
   * (from other board that match 100% the set, it's a custom 68705 MCU)

  .U39  27256    ; stickered  (c) 1993 MICRO MFG TURBO-2 U39-014 US UTBK 022190.

  .U38  8255     ; stickered  MICRO MANUFACTURING, INC.  DATE: 02-24-1994  SER# LKY-PCB-142728.

  .U37  MMI PAL12L6-2  ; Blue dot on it.

  .U44  DS1220AD-150   ; Dallas 2K x 8 CMOS nonvolatile SRAM.

  .U23  82S131         ; Bipolar PROM.


        27256 @U39                                Estimated U35 pinouts
       .----------.                                   .----------.
  VPP -|01      28|- VCC                         GND -|01      28|- Pin 10 of U14 (7404)
  A12 -|02      27|- A14                         VCC -|02      27|- A7
   A7 -|03      26|- A13                         VCC -|03      26|- A6
   A6 -|04      25|- A8                          N/C -|04      25|- A5
   A5 -|05      24|- A9            Pull-up to pin 02 -|05      24|- A4
   A4 -|06      23|- A11                         VCC -|06      23|- A3
   A3 -|07      22|- /OE                         N/C -|07      22|- A2
   A2 -|08      21|- A10            Pin 9 of U37 PAL -|08      21|- A1
   A1 -|09      20|- /CE            Pin 8 of U37 PAL -|09      20|- A0
   A0 -|10      19|- D7         Pin 24 of U42 (8080) -|10      19|- D7
   D0 -|11      18|- D6             Pin 7 of U37 PAL -|11      18|- D6
   D1 -|12      17|- D5                           D0 -|12      17|- D5
   D2 -|13      16|- D4                           D1 -|13      16|- D4
  GND -|14      15|- D3                           D2 -|14      15|- D3
       '----------'                                   '----------'


  PCB Layout (Turbo Poker 2 by Micro MFG):                                        Edge Connector 36x2
  .---------------------------------------------------------------------------------------------------.
  | .---------.  .---------.  .---------.  .---------.  .---------.     .-----.    .--------.         |
  | | 74LS161 |  | 74LS161 |  | 74LS161 |  | 74LS161 |  | 74LS161 |     | 555 |    | KA2657 |  .---.  |
  | '---------'  '---------'  '---------'  '---------'  '---------'     '-----'    '--------'  |VR1|  |
  |     U1           U2           U3           U4           U5            U6           U7      '---'  |
  |                                                                                                   |
  | .---------.  .---------.  .---------.  .---------.  .---------.                .--------.         |
  | | 74LS161 |  | 74LS157 |  | 74LS157 |  | 74LS157 |  | 74LS157 |                | KA2657 |         |
  | '---------'  '---------'  '---------'  '---------'  '---------'                '--------!         |
  |     U8           U9           U10          U11          U12                       U13             |
  |                                                                                                   |
  | .---------.  .---------.  .---------.  .---------.  .---------.   .---------.  .--------.         | 36
  | | 74LS04P |  | 74LS11N |  | 74LS04P |  | DV7486N |  | DV7486N |   | CTS8427 |  | KA2657 |         '---.
  | '---------'  '---------'  '---------'  '---------'  '---------'   '---------'  '--------!          ---|
  |     U14          U15          U16          U17          U18       U19 (resnet)    U20              ---|
  |                                                                                                    ---|
  | .---------.  .---------.  .---------.     .-----------------.     .---------.  .-.-.-.-.-.-.-.-.   ---|
  | |  74123  |  | 74LS174 |  | 82S131N |     | 74LS541 (R dot) |     | CTS8427 |  |1|2|3|4|5|6|7|8|   ---|
  | '---------'  '---------'  '---------'     '-----------------'     '---------'  '-'-'-'-'-'-'-'-'   ---|
  |     U21          U22          U23                 U24             U25 (resnet)  U26 (DIP SW x 8)   ---|
  |                                                                                                    ---|
  | .---------.  .---------.  .---------.     .------------------.      .------------------------.     ---|
  | |  7474N  |  | 74LS157 |  | 74LS166 |     |      2732A       |      |     Intel  P8255A      |     ---|
  | '---------'  '---------'  '---------'     |    (char ROM)    |      |                        |     ---|
  |     U27          U28          U29         '------------------'      '------------------------'     ---|
  |                                                   U30                          U31                 ---|
  | .---------.  .----------. .----------.    .--------------------.    .------------------------.     ---|
  | |  7474N  |  | SY2111-1 | | SY2111-1 |    | Unknown custom MCU |    |     Intel  P8255A      |     ---|
  | '---------'  '----------' '----------'    |   (68705 based)    |    |                        |     ---|
  |     U32          U33          U34         '--------------------'    '------------------------'     ---|
  |                                                    U35                         U36                 ---|
  | .---------------. .--------------------.  .--------------------.                                   ---|
  | |PAL12L6 (B dot)| |  8255 (stickered)  |  |     27256 ROM      |                                  .---'
  | '---------------' |                    |  |                    |                                  | 01
  |       U37         '--------------------'  '--------------------'                 .----------.     |
  |                           U38                    U39                             | TRW 8022 |     |
  | .------------.   .---------------------.  .--------------------.                 '----------'     |
  | | Intel 8224 |   |   NEC  D8080AFC-1   |  |   8224 Clock GEN   |                     U45          |
  | '------------'   |                     |  |                    |    .-------------------.         |
  |      U41         '---------------------'  '--------------------'    |  Dallas DS1220AD  |         |
  | .------.                  U42                    U43                | Non Volatile SRAM |         |
  | | Xtal |                                                            '-------------------'         |
  | | 18MHz|                                                                     U44                  |
  | '------'                                                                                          |
  '---------------------------------------------------------------------------------------------------'


  Discrete sound circuitry:
  -------------------------
                          .---v---.
  GND --------------------|1     8|-----VCC
                    N/C---|2 555 7|------------------------------------+---|1K Ohm|--- VCC
  Volume Pot -------------|3     6|--------------------+--|100K Ohm|---+
  Pin 10 U36 (8255)-------|4     5|---|0.1uF|---GND    |
                          '-------'                    +-|0.01uF|---- GND
                                                       +-|0.01uF|---- pin 12 U13 (KA2667)
                                                       +-|0.022uF|--- pin 11 U13 (KA2667)
                                                       +-|0.05uF|---- pin 10 U13 (KA2667)
  DIP Switches:

  DIP #1: SETUP      ON/OFF         ;"setup menu to change all the settings"
  DIP #2: RAISE      ON/OFF
  DIP #3: XCARDS     ON/OFF
  DIP #4: REPLAYS    ON/OFF
  DIP #5: BONUS      ON/OFF
  DIP #6: COIN TYPE  QUARTER/NICKEL
  DIP #7: HISCORE    ON/OFF         ;game saves high scores
  DIP #8: NOT USED

*/

ROM_START( tpoker2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1993_micro_mfg_turbo-2_u39-014_us_utbk_022190.u39", 0x0000, 0x8000, CRC(543149fe) SHA1(beb61a27c2797341e23e020e754d63fde3b4fbb2) )

	ROM_REGION( 0x0800,  "mcu", 0 )  // 68705 MCU
	ROM_LOAD( "1993_micro_mfg_tp2_01_u35_ic4_16228_022194.u35", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "1993_micro_mfg_turbo_poker_char_rom.u30", 0x0000, 0x1000, CRC(6df86e08) SHA1(a451f71db7b59500b99207234ef95793afc11f03) )

	ROM_REGION( 0x0800,  "nvram", 0 )  // DS1220AD-150 ; Dallas 2K x 8 CMOS nonvolatile SRAM
	ROM_LOAD( "tpoker2a_nvram.bin", 0x0000, 0x0800, CRC(615f3888) SHA1(b7d5aeb1c52748061f8913571bc5ac3e839c3595) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "tpoker2a_82s131.u23", 0x0000, 0x0200, CRC(0222124f) SHA1(5cd8d24ee8e6525a5f9e6a93fa8854f36f4319ee) )
	ROM_IGNORE(                              0x0200)  // second half filled with 0x80

	ROM_REGION( 0x0034,  "plds", 0 )
	ROM_LOAD( "mmi_pal12l6-2_blue_dot.u37", 0x0000, 0x0034, CRC(25651948) SHA1(62cd4d73c6ca8ea5d4beb9ae262d1383f8149462) )
ROM_END

ROM_START( tpoker2c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "turbo-2_u39-015_uscpn8k_021990.u39", 0x0000, 0x8000, CRC(1149bb41) SHA1(5320abafafb612643479e9ac682a523ed76bcda1) )

	ROM_REGION( 0x0800,  "mcu", 0 )  // 68705 MCU
	ROM_LOAD( "tp2-01_u35-ic4_9862_051997.u35", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x0800,  "gfx", 0 )
	ROM_LOAD( "turbo_poker_char_rom.u30", 0x0000, 0x0800, CRC(27402bc1) SHA1(57a4dc5c69d9c5da2e416dbea21235fe20515be5) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "n82s131n.u23", 0x0000, 0x0200, CRC(6834053a) SHA1(77270591aeb3ed06f72897b8f57302502f752336) )
ROM_END

ROM_START( tpoker2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "turbo-2_u39-012_us_uc8k_060989_27c256.u39", 0x0000, 0x8000, CRC(ac70f2dd) SHA1(a4264d3a0ac39e0f46b2ee594d519eef2442a3b1) )

	ROM_REGION( 0x0800,  "mcu", 0 )  // 68705 MCU
	ROM_LOAD( "tp2.u35", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x1000,  "gfx", 0 )
	ROM_LOAD( "turbo_poker_char_rom_2732a.u30", 0x0000, 0x1000, CRC(6df86e08) SHA1(a451f71db7b59500b99207234ef95793afc11f03) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "n82s131n.u23", 0x0000, 0x0200, CRC(6834053a) SHA1(77270591aeb3ed06f72897b8f57302502f752336) )
ROM_END


/****************************** Unknown Sets ********************************/



/*********************************************
*                Driver Init                 *
*********************************************/

/*
The following tables are for the norautua and norautub sets. The encryption seems based on substitution tables
chosen by bits 0, 1, 2 and 3 of the address and by the M1 line.
However, it is improbable that games this old and simple use 0x2000 of ROM to hold such large tables, so they can
probably be calculated if the algorithm is identified.
They mostly work, however to be 100% confident the algorithm needs to be figured out or ar least more sets are needed.
*/

void norautp_state::init_enc()
{
	uint8_t unkn = 0x00;

	// opcodes
	static const uint8_t opcodes_sub_table_00[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, 0x7f, 0xb0, 0xed, 0x28, unkn, unkn, unkn, 0xd5, 0xd0, unkn, 0x7e, unkn, unkn, unkn, // 0x0x
		0x4e, unkn, unkn, unkn, unkn, unkn, unkn, 0x04, unkn, 0x2a, unkn, unkn, 0x0c, 0x09, unkn, unkn, // 0x1x
		unkn, 0xea, 0xa9, unkn, 0xbb, 0x3d, unkn, unkn, unkn, unkn, 0x66, unkn, unkn, unkn, unkn, unkn, // 0x2x
		unkn, unkn, 0x1d, unkn, 0x77, unkn, unkn, 0x3e, unkn, 0x23, 0xe5, unkn, 0xb9, 0x0e, unkn, unkn, // 0x3x
		unkn, 0x35, unkn, 0xb8, unkn, unkn, unkn, 0xf1, unkn, 0xf7, unkn, unkn, 0x36, 0xdf, unkn, unkn, // 0x4x
		unkn, unkn, unkn, unkn, 0xfe, unkn, unkn, 0x20, unkn, 0x33, 0x2b, unkn, unkn, 0x10, 0x57, unkn, // 0x5x
		unkn, 0xca, 0x13, 0xcb, 0xc1, 0xeb, 0xa3, 0xdd, 0x46, unkn, unkn, 0xb7, unkn, unkn, unkn, unkn, // 0x6x
		unkn, 0xf3, unkn, unkn, unkn, unkn, 0xc2, 0x7c, unkn, 0xcd, unkn, unkn, 0x42, 0xc8, 0x32, 0x22, // 0x7x
		unkn, unkn, 0x83, unkn, unkn, unkn, unkn, unkn, unkn, 0x06, unkn, 0x5b, unkn, unkn, 0xfd, 0xa5, // 0x8x
		unkn, 0xc3, 0xe7, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x71, unkn, 0x3a, unkn, unkn, 0x0f, // 0x9x
		0x34, unkn, 0xda, unkn, unkn, unkn, 0x85, unkn, 0xe6, unkn, unkn, unkn, unkn, 0xe3, unkn, unkn, // 0xax
		unkn, 0x1e, 0x05, unkn, 0x4f, unkn, unkn, unkn, unkn, unkn, 0xaf, unkn, 0x01, 0x5f, unkn, unkn, // 0xbx
		unkn, 0x38, 0x30, unkn, unkn, 0x6f, 0x70, 0x16, unkn, 0xc9, unkn, 0xc5, 0x78, 0x7a, unkn, unkn, // 0xcx
		unkn, 0xd1, unkn, unkn, 0xbe, unkn, unkn, 0x79, unkn, unkn, unkn, unkn, 0x6a, 0x37, unkn, unkn, // 0xdx
		unkn, 0x7d, unkn, unkn, unkn, 0x21, unkn, unkn, unkn, 0x1f, unkn, unkn, unkn, unkn, 0x11, 0xfa, // 0xex
		unkn, unkn, unkn, 0x08, 0x7b, 0x5e, 0x47, 0xdb, unkn, 0x18, 0x07, 0xc0, unkn, unkn, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_01[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, 0xdf, unkn, unkn, 0xfe, 0x83, unkn, unkn, unkn, 0x07, unkn, unkn, 0x7a, unkn, 0x30, unkn, // 0x0x
		unkn, 0x0e, unkn, unkn, unkn, 0x18, 0xba, unkn, unkn, 0xfb, unkn, unkn, 0x0b, unkn, 0x16, unkn, // 0x1x
		unkn, 0xf6, 0xb5, unkn, unkn, 0xc1, unkn, unkn, 0x78, unkn, 0x35, 0xd6, unkn, 0xc5, unkn, unkn, // 0x2x
		unkn, unkn, 0xd1, 0xb1, 0x1b, unkn, 0x46, 0xc2, 0x06, unkn, unkn, 0xe6, unkn, 0x7e, unkn, unkn, // 0x3x
		unkn, unkn, unkn, unkn, unkn, 0x34, unkn, unkn, 0xe7, unkn, 0xea, unkn, 0x2b, 0xca, unkn, unkn, // 0x4x
		0x09, 0xf5, unkn, unkn, 0x60, 0x20, unkn, unkn, 0x3d, unkn, 0x79, 0x6a, unkn, unkn, 0xc3, 0x2a, // 0x5x
		unkn, unkn, unkn, 0x7d, 0x23, unkn, unkn, unkn, unkn, 0x0f, unkn, unkn, 0x38, 0x04, unkn, 0x3e, // 0x6x
		0x4e, unkn, 0xcc, unkn, 0x81, 0xb0, 0xe5, unkn, unkn, 0xda, unkn, unkn, unkn, 0xb9, unkn, unkn, // 0x7x
		unkn, unkn, 0x21, 0xaf, unkn, 0xc8, unkn, unkn, unkn, unkn, 0x28, unkn, unkn, unkn, unkn, unkn, // 0x8x
		unkn, unkn, unkn, 0x1a, 0xcd, 0xfd, 0x10, 0xb7, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xc0, // 0x9x
		0x27, 0x14, unkn, unkn, unkn, unkn, 0x1d, unkn, unkn, 0xa0, 0x33, unkn, unkn, 0x01, 0xdd, 0xed, // 0xax
		0x11, 0x08, unkn, unkn, 0x4f, unkn, 0x1e, unkn, unkn, 0xc9, unkn, unkn, 0xcb, unkn, 0x36, 0xd5, // 0xbx
		unkn, unkn, 0x6f, unkn, unkn, unkn, unkn, 0x13, unkn, 0x22, unkn, 0xa8, 0xb8, unkn, unkn, unkn, // 0xcx
		unkn, 0x77, unkn, unkn, unkn, 0x5f, unkn, unkn, 0x00, 0x2f, unkn, unkn, unkn, 0xa3, unkn, unkn, // 0xdx
		unkn, unkn, unkn, 0x3a, unkn, unkn, unkn, 0xd2, unkn, 0x57, unkn, unkn, 0x32, unkn, unkn, unkn, // 0xex
		0x1f, unkn, unkn, unkn, unkn, 0x7b, unkn, 0x47, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x17, // 0xfx
	};

	static const uint8_t opcodes_sub_table_02[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xfe, unkn, unkn, unkn, 0x07, unkn, 0xda, 0x10, unkn, 0x32, 0x7e, unkn, 0xa7, unkn, 0xfb, unkn, // 0x0x
		0x36, 0xb0, 0x3e, unkn, 0x20, unkn, 0xe5, unkn, 0x79, 0xe6, unkn, unkn, 0x73, unkn, 0xc4, unkn, // 0x1x
		0xb8, unkn, unkn, unkn, unkn, unkn, unkn, 0x43, 0x86, 0x78, unkn, unkn, unkn, unkn, 0xc9, unkn, // 0x2x
		unkn, unkn, unkn, 0x34, unkn, unkn, 0x47, unkn, 0x4f, unkn, unkn, 0xc1, 0xcf, 0x2f, 0xed, unkn, // 0x3x
		unkn, 0xc5, unkn, unkn, unkn, unkn, unkn, unkn, 0xff, 0x11, unkn, unkn, 0xf3, unkn, unkn, 0xfd, // 0x4x
		0x3f, 0xc0, unkn, 0xa3, 0xe9, unkn, 0x1a, 0x1e, 0x0a, 0x7a, 0xd3, 0x90, unkn, 0xcc, 0x31, unkn, // 0x5x
		0xd1, unkn, unkn, unkn, 0xdf, 0x2b, unkn, 0x3a, unkn, unkn, unkn, unkn, 0x2a, 0xbf, unkn, unkn, // 0x6x
		unkn, 0xdd, 0x46, unkn, 0xc8, unkn, 0x33, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0x7x
		unkn, unkn, unkn, unkn, unkn, unkn, 0x15, unkn, unkn, unkn, unkn, unkn, unkn, 0x23, 0x06, unkn, // 0x8x
		unkn, unkn, unkn, unkn, 0xbe, unkn, unkn, unkn, 0x57, unkn, unkn, 0x21, unkn, 0x22, unkn, unkn, // 0x9x
		0xb9, unkn, unkn, 0xf6, 0xe1, unkn, unkn, 0xe7, unkn, unkn, unkn, 0xd8, unkn, 0x5f, unkn, unkn, // 0xax
		unkn, 0x87, 0x28, 0x68, unkn, 0xaf, 0x0d, unkn, unkn, 0xca, 0xbb, unkn, 0x35, unkn, unkn, unkn, // 0xbx
		unkn, 0x9e, unkn, 0xc3, unkn, unkn, 0x0e, unkn, unkn, 0xd5, 0x0f, 0x18, unkn, unkn, unkn, unkn, // 0xcx
		0x01, 0xd7, 0xb7, 0xa8, unkn, 0xcd, unkn, unkn, 0x1d, 0xf1, 0xd0, unkn, unkn, 0x4e, unkn, unkn, // 0xdx
		unkn, unkn, unkn, 0x30, 0x09, 0x7c, 0xc6, 0x77, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0xex
		unkn, unkn, unkn, 0x56, 0x7b, unkn, 0x3d, unkn, unkn, unkn, 0x38, 0xcb, unkn, unkn, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_03[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xfd, unkn, unkn, unkn, unkn, unkn, 0x3d, unkn, 0xcf, unkn, 0x06, unkn, 0x05, unkn, unkn, unkn, // 0x0x
		0xd9, unkn, unkn, 0xf1, 0xdd, 0xda, 0x01, unkn, unkn, unkn, unkn, unkn, 0x18, unkn, unkn, unkn, // 0x1x
		unkn, unkn, unkn, unkn, 0xd3, unkn, 0x87, 0xf5, unkn, unkn, 0x34, unkn, unkn, unkn, unkn, unkn, // 0x2x
		unkn, unkn, unkn, unkn, unkn, unkn, 0x1e, 0x78, unkn, unkn, unkn, 0x26, 0x22, unkn, unkn, unkn, // 0x3x
		unkn, unkn, unkn, 0x46, unkn, 0xf7, 0x3f, unkn, unkn, unkn, 0xe7, unkn, unkn, 0x80, 0xdb, unkn, // 0x4x
		unkn, 0xcc, unkn, 0xc8, 0xb8, 0x32, unkn, unkn, 0xb0, 0xd1, unkn, unkn, unkn, 0x7c, unkn, unkn, // 0x5x
		unkn, 0xcd, unkn, unkn, unkn, 0xb7, 0xc6, 0x47, unkn, unkn, unkn, 0x19, unkn, unkn, 0x23, unkn, // 0x6x
		unkn, unkn, unkn, unkn, unkn, 0x31, unkn, unkn, 0xaf, 0xb1, 0x11, 0x20, unkn, unkn, 0x16, unkn, // 0x7x
		unkn, unkn, unkn, 0x7e, unkn, unkn, unkn, unkn, unkn, 0xc2, unkn, 0x3c, unkn, unkn, unkn, unkn, // 0x8x
		0x56, unkn, unkn, 0x79, 0x7a, unkn, unkn, unkn, 0x4e, unkn, unkn, 0xbe, unkn, unkn, unkn, unkn, // 0x9x
		unkn, unkn, 0x30, 0x36, unkn, unkn, unkn, 0xa6, unkn, 0x76, unkn, unkn, 0x7b, unkn, 0xe1, unkn, // 0xax
		unkn, unkn, 0x81, unkn, 0xfa, 0xc5, 0x4f, 0x90, 0xf2, unkn, 0xcb, unkn, unkn, unkn, unkn, 0x10, // 0xbx
		0xc0, 0x7d, 0xe6, unkn, unkn, unkn, 0xca, unkn, unkn, unkn, 0xd5, unkn, unkn, 0x2f, 0xc3, unkn, // 0xcx
		0xba, unkn, 0x21, 0x2a, 0x52, unkn, unkn, 0x3a, unkn, unkn, 0x77, 0xed, unkn, unkn, unkn, unkn, // 0xdx
		0x38, 0xee, unkn, unkn, unkn, 0x0f, unkn, unkn, unkn, unkn, 0x3e, unkn, unkn, 0x0e, unkn, 0x28, // 0xex
		unkn, 0xc9, 0xb9, 0xe5, unkn, 0x24, unkn, 0xd0, 0xfe, 0xa5, 0x27, 0x15, unkn, unkn, 0xdf, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_04[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x70, 0x7e, unkn, 0xe5, 0xc1, 0xd8, 0xb8, unkn, unkn, unkn, unkn, 0xa5, 0xf3, 0xd3, unkn, 0x79, // 0x0x
		unkn, 0xd2, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xd5, unkn, 0x7f, unkn, unkn, 0x21, // 0x1x
		unkn, unkn, 0xaf, 0x31, unkn, unkn, unkn, 0x71, unkn, unkn, unkn, 0x3d, unkn, unkn, unkn, 0xa6, // 0x2x
		unkn, unkn, 0x7a, unkn, unkn, 0x37, unkn, unkn, 0xcd, unkn, 0xb1, unkn, 0xfa, unkn, unkn, unkn, // 0x3x
		unkn, unkn, 0x2a, unkn, 0x04, unkn, 0xc8, 0x2f, unkn, 0x1a, unkn, unkn, unkn, unkn, 0xca, unkn, // 0x4x
		0x30, unkn, unkn, unkn, 0x1d, unkn, unkn, 0x46, 0xb5, unkn, 0x1e, unkn, 0xee, 0xc6, 0xd6, unkn, // 0x5x
		0x0d, 0x4f, unkn, unkn, unkn, 0x0e, 0xf7, 0x57, 0xd9, 0xdf, 0x56, unkn, unkn, unkn, unkn, 0x06, // 0x6x
		0x81, unkn, 0xc3, unkn, 0x18, unkn, unkn, 0xb9, 0x10, 0x1c, unkn, unkn, unkn, unkn, unkn, unkn, // 0x7x
		unkn, unkn, unkn, 0x11, unkn, 0xcb, unkn, 0x0f, unkn, unkn, 0xeb, unkn, unkn, 0xc9, unkn, unkn, // 0x8x
		0xd7, 0x2b, unkn, unkn, unkn, unkn, 0x7d, 0x72, unkn, unkn, unkn, 0x3e, unkn, unkn, unkn, 0x23, // 0x9x
		0x5f, unkn, unkn, 0xdd, 0x7c, 0x32, 0x38, unkn, 0xe1, 0xb6, unkn, unkn, unkn, 0xf2, unkn, unkn, // 0xax
		unkn, 0xd0, unkn, 0xc4, unkn, unkn, unkn, 0x22, unkn, 0xbf, unkn, 0xda, unkn, 0xf5, 0x52, unkn, // 0xbx
		unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x47, unkn, 0xbe, 0x4e, unkn, 0x3a, 0x77, 0x01, // 0xcx
		0x6f, unkn, 0x17, unkn, 0x20, 0x07, 0xb7, unkn, 0x34, unkn, unkn, unkn, unkn, 0x35, unkn, unkn, // 0xdx
		unkn, unkn, unkn, unkn, 0x16, unkn, unkn, unkn, 0xfd, unkn, unkn, 0x28, unkn, 0x36, 0xfe, 0xe6, // 0xex
		unkn, unkn, 0xb0, unkn, 0xed, unkn, unkn, unkn, unkn, unkn, unkn, 0x78, unkn, unkn, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_05[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, 0xc9, 0x07, 0x2b, 0x09, unkn, 0xeb, 0xb2, unkn, unkn, 0xb6, 0x08, unkn, unkn, unkn, unkn, // 0x0x
		unkn, 0xc1, 0x1e, unkn, 0x78, 0xa9, unkn, 0x18, unkn, unkn, unkn, unkn, 0x2a, unkn, unkn, 0x16, // 0x1x
		unkn, unkn, unkn, 0x28, 0xb9, 0xfd, 0xed, unkn, unkn, 0xf6, unkn, unkn, 0x06, unkn, 0x20, 0x23, // 0x2x
		unkn, unkn, unkn, unkn, 0xf1, unkn, unkn, 0x3a, unkn, unkn, unkn, 0xe6, unkn, 0xe5, unkn, unkn, // 0x3x
		unkn, unkn, unkn, 0x11, 0xc8, unkn, 0x6f, unkn, unkn, unkn, unkn, unkn, 0x47, unkn, unkn, unkn, // 0x4x
		unkn, unkn, 0xdf, unkn, unkn, unkn, 0xc4, unkn, unkn, 0x36, 0xb7, unkn, 0xe1, unkn, 0xaf, unkn, // 0x5x
		0xcd, unkn, unkn, 0x7d, unkn, unkn, unkn, unkn, unkn, 0xdd, unkn, unkn, 0x30, unkn, unkn, unkn, // 0x6x
		0x7e, unkn, unkn, 0xc3, 0x14, unkn, unkn, unkn, unkn, unkn, unkn, 0x87, 0xcb, unkn, unkn, unkn, // 0x7x
		unkn, unkn, unkn, unkn, 0xb8, 0xf5, unkn, 0xdb, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0x8x
		unkn, 0xc0, unkn, 0x2f, 0x0b, unkn, unkn, 0xd5, 0x17, unkn, unkn, 0xa3, 0xa6, 0x01, 0x21, 0x22, // 0x9x
		unkn, unkn, 0x57, unkn, unkn, 0xd1, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x0e, // 0xax
		unkn, unkn, 0x1d, 0x3e, 0xfb, 0x5b, unkn, 0x13, unkn, unkn, 0xd2, unkn, 0x38, 0x10, 0x3d, 0x7a, // 0xbx
		unkn, unkn, unkn, unkn, unkn, unkn, 0xd8, 0x72, unkn, 0xbe, 0x0f, unkn, 0xca, unkn, unkn, unkn, // 0xcx
		unkn, 0x66, unkn, unkn, 0xfe, unkn, unkn, unkn, unkn, 0xb0, unkn, unkn, 0x15, 0x32, 0x79, unkn, // 0xdx
		0x0c, 0xda, 0x5f, 0xe7, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x35, 0x70, unkn, // 0xex
		unkn, unkn, unkn, 0x77, unkn, unkn, unkn, 0x7c, unkn, unkn, unkn, unkn, 0xba, 0x4e, unkn, 0xb5, // 0xfx
	};

	static const uint8_t opcodes_sub_table_06[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xfb, 0xc1, 0xaf, 0xfd, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0x0x
		0x31, unkn, unkn, unkn, unkn, unkn, 0x7e, 0xbe, 0x77, unkn, unkn, unkn, 0x1e, unkn, unkn, 0x1b, // 0x1x
		unkn, unkn, unkn, 0x3a, unkn, 0x0c, unkn, 0xc8, 0x2f, unkn, 0xe1, unkn, 0xeb, 0xa0, 0x7c, 0xa2, // 0x2x
		unkn, unkn, unkn, 0xb5, unkn, unkn, unkn, unkn, unkn, 0x35, unkn, 0xc9, unkn, unkn, unkn, unkn, // 0x3x
		unkn, unkn, unkn, unkn, unkn, 0xc4, unkn, unkn, 0x72, 0xab, 0x36, unkn, 0x32, 0xe7, unkn, 0xc0, // 0x4x
		0xd1, unkn, 0x0e, 0x19, unkn, unkn, 0xcd, 0x5b, unkn, unkn, 0xf6, unkn, unkn, 0xb9, 0x7a, unkn, // 0x5x
		unkn, 0x7b, unkn, 0xb8, 0x2b, unkn, unkn, 0xdd, 0xc5, unkn, unkn, 0xa3, 0xed, unkn, unkn, 0x38, // 0x6x
		unkn, 0x06, unkn, unkn, unkn, 0x01, unkn, 0x23, unkn, 0xcb, unkn, unkn, unkn, 0x20, unkn, unkn, // 0x7x
		0x16, 0xd5, unkn, unkn, 0xd3, 0xfe, unkn, unkn, unkn, unkn, unkn, 0x0f, 0x3e, unkn, unkn, 0x28, // 0x8x
		unkn, 0x78, unkn, unkn, unkn, 0xa8, unkn, 0xb6, 0xd6, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0x9x
		unkn, 0xd7, unkn, 0xe5, unkn, 0x18, unkn, 0x09, unkn, 0x11, unkn, 0x07, unkn, unkn, 0xcc, unkn, // 0xax
		unkn, 0x15, unkn, unkn, unkn, unkn, 0xf5, unkn, unkn, 0xdf, unkn, 0x30, unkn, unkn, unkn, unkn, // 0xbx
		unkn, 0x47, 0x4f, unkn, unkn, unkn, unkn, unkn, 0xd8, unkn, unkn, unkn, unkn, unkn, 0x3c, 0x79, // 0xcx
		0x5f, unkn, 0x08, 0x3d, unkn, unkn, 0x21, unkn, unkn, unkn, unkn, unkn, 0x37, 0x2a, 0x10, unkn, // 0xdx
		unkn, 0x1d, 0xe6, unkn, 0x57, unkn, unkn, unkn, 0x14, unkn, unkn, 0xc3, unkn, unkn, unkn, unkn, // 0xex
		unkn, unkn, unkn, 0xb0, 0xb7, unkn, unkn, 0x6f, unkn, unkn, unkn, 0x22, unkn, unkn, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_07[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x11, 0xc6, unkn, unkn, 0x2b, unkn, unkn, unkn, unkn, unkn, unkn, 0x3a, unkn, unkn, 0x21, 0x4e, // 0x0x
		unkn, unkn, 0xc3, 0x30, unkn, 0x76, 0x69, unkn, unkn, 0x04, unkn, 0xcd, unkn, 0xc2, unkn, 0x46, // 0x1x
		0xc9, unkn, unkn, unkn, 0xd3, unkn, 0xe9, 0xbe, 0xb5, unkn, unkn, unkn, 0x87, unkn, unkn, unkn, // 0x2x
		0x0f, 0x28, 0x78, unkn, unkn, unkn, 0x02, 0x10, unkn, 0xf5, unkn, unkn, unkn, unkn, unkn, 0xdf, // 0x3x
		0x60, unkn, unkn, 0x1d, 0x0d, unkn, unkn, unkn, 0xfe, 0xed, unkn, unkn, 0x7e, 0xc8, unkn, unkn, // 0x4x
		unkn, 0x47, unkn, 0xb8, 0x32, unkn, 0x1b, 0xcc, 0xd5, unkn, unkn, 0xe7, 0xfd, unkn, 0xaf, unkn, // 0x5x
		unkn, unkn, 0x2f, unkn, unkn, unkn, unkn, unkn, 0xf3, unkn, unkn, unkn, unkn, 0x2a, 0x23, unkn, // 0x6x
		unkn, 0x42, 0xa6, 0xb1, 0x1e, 0x36, unkn, 0xc1, unkn, unkn, unkn, unkn, unkn, unkn, 0xdd, unkn, // 0x7x
		0x05, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0x8x
		unkn, unkn, 0x3d, unkn, 0xe6, unkn, 0x72, 0xcf, unkn, 0x77, unkn, 0x57, 0xc0, 0xa0, 0x7a, unkn, // 0x9x
		0x52, unkn, unkn, 0x7c, unkn, unkn, unkn, unkn, unkn, 0xe1, unkn, unkn, unkn, unkn, 0xd0, 0xb9, // 0xax
		0x3e, 0x07, unkn, 0xb7, unkn, unkn, 0xd6, 0x15, unkn, unkn, unkn, unkn, unkn, 0x79, unkn, unkn, // 0xbx
		unkn, unkn, 0x20, unkn, unkn, unkn, 0x0e, 0xca, 0x1a, 0x06, unkn, unkn, unkn, 0xf6, 0xcb, 0xda, // 0xcx
		unkn, unkn, unkn, 0x4f, 0x18, unkn, 0xdb, 0xb2, unkn, 0x38, 0x27, unkn, unkn, unkn, 0x81, unkn, // 0xdx
		unkn, 0xd1, unkn, unkn, unkn, 0x7b, unkn, unkn, unkn, 0xc4, unkn, unkn, unkn, 0xd2, unkn, unkn, // 0xex
		0x56, unkn, 0x5f, unkn, 0x80, unkn, 0x16, unkn, unkn, 0xc5, unkn, unkn, unkn, unkn, 0x22, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_08[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, 0x19, unkn, unkn, unkn, unkn, 0xc8, unkn, unkn, unkn, 0x27, unkn, unkn, 0x37, unkn, 0x2b, // 0x0x
		unkn, 0x28, 0x0e, unkn, 0x69, 0x3e, unkn, 0xcc, unkn, unkn, 0x3d, unkn, unkn, 0xc3, unkn, 0x77, // 0x1x
		0x16, unkn, unkn, unkn, 0x23, unkn, 0xca, 0xe7, unkn, 0x1d, 0x4e, 0xc9, unkn, 0xc4, unkn, unkn, // 0x2x
		unkn, unkn, unkn, 0x1f, unkn, 0x47, unkn, 0x17, unkn, unkn, unkn, unkn, 0xcd, 0x0d, 0xfe, unkn, // 0x3x
		0xdf, unkn, unkn, 0x68, unkn, 0xd8, 0x22, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xe1, 0xe5, // 0x4x
		unkn, unkn, 0xb6, unkn, unkn, unkn, unkn, 0x32, unkn, unkn, 0xbe, unkn, unkn, unkn, unkn, 0x11, // 0x5x
		unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x35, unkn, 0x7a, unkn, unkn, unkn, unkn, unkn, 0x56, // 0x6x
		0xd3, unkn, unkn, unkn, unkn, 0x71, unkn, unkn, 0x06, unkn, 0xf6, unkn, unkn, 0x36, unkn, unkn, // 0x7x
		unkn, unkn, 0xcb, 0xba, unkn, unkn, unkn, unkn, unkn, 0x10, unkn, 0xdd, 0xc6, unkn, unkn, unkn, // 0x8x
		0x1e, 0x4f, unkn, unkn, unkn, 0x3a, 0xb9, 0x18, 0x78, unkn, 0x2a, unkn, 0x14, unkn, unkn, 0x52, // 0x9x
		unkn, unkn, 0x57, 0xa6, unkn, unkn, unkn, 0x38, unkn, 0xe6, unkn, 0x21, 0xc1, unkn, unkn, 0xa0, // 0xax
		0xbf, unkn, unkn, unkn, unkn, unkn, unkn, 0xb7, 0x01, 0xd5, 0x42, unkn, 0x7c, unkn, 0xf2, unkn, // 0xbx
		unkn, 0xf5, unkn, 0xf1, 0xdb, 0xb0, unkn, 0x0f, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0xcx
		unkn, unkn, unkn, unkn, 0x04, unkn, unkn, unkn, 0xd7, unkn, 0x5f, unkn, 0x34, unkn, unkn, unkn, // 0xdx
		0x30, 0x1b, 0xbb, unkn, unkn, 0xb5, 0x20, unkn, unkn, unkn, unkn, unkn, 0x07, unkn, 0x12, 0x79, // 0xex
		unkn, unkn, unkn, unkn, unkn, unkn, 0x7e, unkn, unkn, 0xaf, 0x46, unkn, unkn, unkn, unkn, 0xc0, // 0xfx
	};

	static const uint8_t opcodes_sub_table_09[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xca, 0x3d, unkn, unkn, unkn, unkn, unkn, unkn, 0x66, 0x20, unkn, unkn, 0x7c, 0xed, unkn, 0x77, // 0x0x
		0x10, unkn, 0xfa, 0xb8, unkn, unkn, unkn, 0xbb, 0x3c, 0x7f, unkn, 0xa6, unkn, unkn, unkn, unkn, // 0x1x
		unkn, 0xd7, 0x38, 0x1f, 0xdf, unkn, 0xd2, 0xf6, unkn, unkn, 0x23, 0x1e, 0xc3, unkn, 0x2f, 0x1a, // 0x2x
		0xd3, unkn, unkn, unkn, unkn, 0xb9, unkn, unkn, 0x16, unkn, 0xb5, 0xb0, 0x79, unkn, unkn, unkn, // 0x3x
		0xcd, unkn, unkn, 0xc2, unkn, unkn, unkn, unkn, unkn, 0x0b, 0x3e, unkn, 0x17, 0x12, 0xdb, 0x52, // 0x4x
		unkn, unkn, 0x28, 0x24, 0x34, 0xb6, 0xcb, 0x36, 0x6f, 0xea, unkn, unkn, unkn, 0x7e, unkn, unkn, // 0x5x
		unkn, unkn, unkn, unkn, 0xf7, 0x4e, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xc8, unkn, // 0x6x
		0xe6, 0xf5, 0xb7, unkn, unkn, 0xdd, unkn, 0x27, unkn, 0xda, 0x87, unkn, unkn, unkn, 0xa5, unkn, // 0x7x
		unkn, unkn, 0xf1, unkn, unkn, unkn, unkn, 0x47, unkn, 0xd6, unkn, unkn, 0x0e, 0x78, unkn, 0x35, // 0x8x
		unkn, unkn, unkn, 0xd1, 0x22, unkn, unkn, unkn, unkn, 0x2a, unkn, 0x15, unkn, unkn, unkn, unkn, // 0x9x
		0x0c, 0x57, unkn, unkn, unkn, unkn, 0x32, 0x67, unkn, unkn, unkn, unkn, 0x5f, unkn, unkn, unkn, // 0xax
		0x58, unkn, unkn, unkn, unkn, 0x14, unkn, 0x19, 0xe5, 0x2b, 0xaf, unkn, 0x29, unkn, unkn, 0x1d, // 0xbx
		0x11, unkn, 0x4f, unkn, 0xa9, unkn, unkn, unkn, 0x01, 0xfe, 0x30, 0xc9, unkn, 0xb3, unkn, 0x04, // 0xcx
		0x56, unkn, unkn, 0x3a, unkn, 0x0f, unkn, 0xe1, unkn, unkn, 0x18, unkn, 0x21, unkn, 0xc0, unkn, // 0xdx
		unkn, unkn, unkn, unkn, 0xd5, 0xb1, unkn, unkn, 0xc5, unkn, unkn, unkn, 0x06, unkn, unkn, 0xc1, // 0xex
		0x7a, unkn, unkn, 0x7d, unkn, 0x13, unkn, unkn, 0x07, unkn, unkn, 0xe7, unkn, unkn, unkn, 0xfb, // 0xfx
	};

	static const uint8_t opcodes_sub_table_0a[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, 0xcb, 0xb5, unkn, 0xb9, unkn, 0xca, 0x18, unkn, 0x38, unkn, 0x47, 0x04, 0x2f, 0xed, // 0x0x
		0xc1, unkn, 0x34, unkn, unkn, unkn, 0x7b, 0xb1, 0xc5, unkn, unkn, unkn, 0x1d, unkn, unkn, unkn, // 0x1x
		unkn, 0xdd, unkn, unkn, unkn, 0x27, unkn, unkn, unkn, 0x1a, unkn, unkn, 0x3d, 0xf6, unkn, unkn, // 0x2x
		unkn, unkn, 0x6f, 0x06, 0x1e, 0x13, 0xd8, unkn, unkn, 0x10, 0x1b, 0x28, unkn, 0x4f, 0x11, 0xa3, // 0x3x
		0x23, unkn, 0xe6, unkn, 0x7d, 0xc0, 0xd5, unkn, 0xc8, 0x2b, unkn, unkn, unkn, unkn, 0x3a, unkn, // 0x4x
		unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xfe, unkn, 0xc2, 0xaf, unkn, 0x20, unkn, 0x14, unkn, // 0x5x
		0x12, unkn, unkn, 0xcd, 0xda, 0xfb, unkn, unkn, unkn, unkn, unkn, unkn, 0x35, 0xa1, 0x57, unkn, // 0x6x
		unkn, 0x7c, unkn, 0x87, 0xab, unkn, unkn, 0x0e, unkn, unkn, 0xc3, unkn, unkn, unkn, 0xb7, unkn, // 0x7x
		0x79, unkn, unkn, unkn, unkn, 0x16, 0xd7, 0x01, 0x4d, unkn, unkn, 0x36, unkn, 0x58, unkn, 0x30, // 0x8x
		unkn, unkn, unkn, unkn, 0xe1, unkn, 0x81, 0x8e, unkn, 0xb8, unkn, unkn, 0xe7, 0xfd, unkn, unkn, // 0x9x
		0xb0, unkn, unkn, unkn, 0xb3, unkn, 0xbe, unkn, unkn, 0xc9, 0x67, unkn, unkn, unkn, unkn, 0x7a, // 0xax
		unkn, unkn, 0x3c, 0x22, 0x19, 0x2a, 0x5e, unkn, unkn, 0x78, unkn, unkn, unkn, 0x3e, 0x4e, unkn, // 0xbx
		unkn, 0xe5, unkn, unkn, 0xf2, unkn, unkn, 0xc6, 0x32, unkn, 0xae, unkn, 0x70, unkn, unkn, unkn, // 0xcx
		unkn, unkn, unkn, 0xf5, 0x77, 0x21, 0xa5, 0x7e, unkn, 0xdf, unkn, unkn, unkn, 0x31, 0xf3, unkn, // 0xdx
		0xc4, unkn, 0x0f, 0x7f, unkn, unkn, unkn, unkn, unkn, unkn, 0x07, 0x1f, unkn, unkn, unkn, 0x05, // 0xex
		unkn, unkn, 0x5f, unkn, unkn, unkn, 0x15, unkn, unkn, unkn, unkn, 0xd3, 0xd1, unkn, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_0b[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x38, unkn, unkn, unkn, unkn, unkn, 0xb1, unkn, 0xb5, 0x1d, 0xb6, unkn, 0x34, unkn, unkn, unkn, // 0x0x
		unkn, 0x4d, unkn, unkn, 0xcb, 0xc3, unkn, unkn, unkn, 0x3a, unkn, unkn, unkn, 0xfd, unkn, unkn, // 0x1x
		unkn, unkn, unkn, 0xb7, 0x4f, unkn, unkn, 0x28, 0x47, unkn, unkn, unkn, unkn, unkn, 0xe5, 0x18, // 0x2x
		unkn, unkn, unkn, unkn, unkn, 0xc2, unkn, 0x7c, unkn, 0xa5, unkn, unkn, unkn, unkn, unkn, unkn, // 0x3x
		0x19, unkn, unkn, unkn, unkn, unkn, unkn, 0xfe, unkn, unkn, unkn, unkn, unkn, 0x15, 0xe6, 0x10, // 0x4x
		unkn, 0xd1, 0x57, 0x3d, 0x79, unkn, unkn, 0xc5, unkn, 0x32, unkn, unkn, 0xd6, unkn, 0x2f, unkn, // 0x5x
		unkn, unkn, unkn, unkn, 0x02, unkn, unkn, unkn, 0xbe, 0x04, unkn, unkn, 0xae, 0xaf, 0xe1, 0x21, // 0x6x
		unkn, 0x0f, unkn, 0xe9, 0x23, 0xb9, 0xc6, unkn, unkn, 0x35, unkn, 0x70, unkn, 0xb0, 0x12, unkn, // 0x7x
		unkn, unkn, 0xed, unkn, 0xd7, unkn, unkn, unkn, unkn, 0xb3, unkn, 0x78, unkn, unkn, unkn, 0x1c, // 0x8x
		0xa3, unkn, 0x20, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x16, 0x1a, unkn, 0x2b, unkn, 0xc0, // 0x9x
		unkn, unkn, 0xda, unkn, 0x77, 0xf2, unkn, 0xf5, unkn, 0xd3, unkn, unkn, 0x09, unkn, 0x36, unkn, // 0xax
		0x2a, unkn, unkn, unkn, 0x7d, unkn, 0x52, unkn, 0x1e, unkn, 0x0b, unkn, unkn, unkn, unkn, 0xd8, // 0xbx
		0xa6, 0x08, 0x7e, unkn, unkn, unkn, 0x01, unkn, unkn, unkn, 0xcd, 0xca, unkn, unkn, 0xdd, unkn, // 0xcx
		0x0e, 0xe7, 0x3c, unkn, unkn, 0x7a, unkn, unkn, unkn, unkn, 0x31, unkn, unkn, 0x07, 0xc8, unkn, // 0xdx
		unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x0d, 0x46, unkn, 0x11, unkn, 0xdb, unkn, 0x5f, 0x22, // 0xex
		unkn, 0x5b, 0xd2, 0x3e, unkn, unkn, unkn, unkn, unkn, 0x13, 0x0a, 0xbb, 0xc9, 0x06, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_0c[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, 0x3c, 0x06, unkn, 0xc1, 0x0f, 0x18, 0x0d, unkn, unkn, unkn, 0xd7, unkn, 0x77, 0x2c, unkn, // 0x0x
		0x78, 0x30, unkn, unkn, unkn, 0xdb, unkn, unkn, unkn, 0xb9, 0xd1, 0x7e, unkn, unkn, 0xfe, 0x72, // 0x1x
		unkn, unkn, unkn, unkn, unkn, 0x1f, 0xd8, unkn, unkn, unkn, 0xc5, unkn, 0xd3, unkn, unkn, unkn, // 0x2x
		0xc3, 0x22, unkn, unkn, 0x7f, unkn, unkn, 0x86, unkn, 0x56, unkn, unkn, 0x52, unkn, unkn, 0xd2, // 0x3x
		unkn, unkn, 0x32, unkn, unkn, unkn, unkn, 0x3e, 0xb7, unkn, unkn, 0xb5, 0x3d, unkn, unkn, unkn, // 0x4x
		unkn, 0x47, 0x66, unkn, unkn, unkn, 0x14, unkn, unkn, 0xb3, 0x71, unkn, unkn, unkn, unkn, 0x20, // 0x5x
		unkn, unkn, unkn, 0xe6, unkn, 0x57, unkn, 0xdf, 0x2a, unkn, 0x4f, unkn, unkn, 0x79, unkn, 0xbe, // 0x6x
		unkn, 0xe7, unkn, 0xc8, unkn, 0x5b, unkn, unkn, 0x46, 0xc0, unkn, unkn, 0x5e, unkn, unkn, unkn, // 0x7x
		0x7b, unkn, 0x23, unkn, 0xd6, 0x16, 0x7d, unkn, unkn, unkn, unkn, unkn, unkn, 0xc9, 0x21, unkn, // 0x8x
		0xf1, unkn, 0xb1, unkn, 0x43, unkn, unkn, unkn, 0xf6, unkn, 0xbb, 0x27, unkn, unkn, 0x04, unkn, // 0x9x
		0xea, unkn, 0x01, 0xdd, unkn, unkn, unkn, unkn, 0x3f, unkn, unkn, unkn, unkn, unkn, unkn, 0x28, // 0xax
		unkn, unkn, 0xda, unkn, 0xcd, unkn, unkn, unkn, unkn, 0xaf, 0x38, unkn, unkn, 0xe9, 0x34, 0x36, // 0xbx
		0xf5, unkn, unkn, 0x10, unkn, 0x1a, unkn, 0x7c, unkn, unkn, 0xc2, unkn, 0x07, 0xcb, 0x11, unkn, // 0xcx
		0x3a, unkn, unkn, 0xe5, 0xd5, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0xdx
		0xca, 0xb8, unkn, unkn, 0x2f, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0xex
		unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x5f, unkn, 0x1e, unkn, 0x2b, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_0d[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, 0x35, 0xcd, unkn, unkn, 0x38, unkn, 0x47, unkn, unkn, unkn, unkn, 0xfb, 0x24, unkn, unkn, // 0x0x
		0xd7, 0x7e, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xb1, unkn, 0xf6, unkn, unkn, unkn, unkn, // 0x1x
		unkn, unkn, unkn, 0x3e, unkn, 0xd0, 0x4a, unkn, unkn, unkn, 0xb2, unkn, unkn, unkn, 0x01, 0xf7, // 0x2x
		unkn, unkn, 0xd2, unkn, 0x66, unkn, unkn, unkn, 0xaf, 0x2b, 0x16, unkn, unkn, unkn, 0xe7, 0xfd, // 0x3x
		0xf1, 0x02, 0x7c, unkn, unkn, unkn, unkn, unkn, unkn, 0xea, 0x34, unkn, unkn, unkn, 0x3d, unkn, // 0x4x
		0xdf, 0xb8, unkn, 0xd6, unkn, unkn, 0x2f, 0x18, 0x1b, unkn, unkn, 0xed, 0x77, 0x4e, 0x7b, unkn, // 0x5x
		0x11, unkn, unkn, unkn, 0x79, 0xff, 0x23, unkn, unkn, 0x26, 0x28, 0xfe, unkn, unkn, unkn, 0x57, // 0x6x
		unkn, unkn, unkn, unkn, 0xe6, 0xc5, 0x32, unkn, unkn, 0x1d, unkn, 0xf2, unkn, unkn, 0x7a, unkn, // 0x7x
		unkn, 0xcb, 0x04, unkn, 0x06, 0xb7, unkn, unkn, unkn, 0x2a, 0xbb, 0x27, unkn, unkn, unkn, unkn, // 0x8x
		0x6f, 0x0f, unkn, 0x3f, unkn, 0xcc, 0x3a, 0x4f, 0xc6, unkn, unkn, unkn, 0x1a, 0x0e, unkn, unkn, // 0x9x
		unkn, unkn, unkn, unkn, unkn, 0x22, unkn, unkn, unkn, unkn, unkn, 0xc4, 0x14, 0xc0, unkn, unkn, // 0xax
		0x07, 0x20, unkn, 0xc2, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xe5, unkn, 0x21, unkn, unkn, // 0xbx
		0xc3, unkn, 0x46, unkn, 0x3c, unkn, 0x87, unkn, unkn, unkn, unkn, unkn, unkn, 0xd8, 0xd1, unkn, // 0xcx
		0x78, unkn, 0xd3, 0xc9, unkn, unkn, unkn, 0xc1, unkn, 0x86, unkn, unkn, 0xb5, unkn, unkn, unkn, // 0xdx
		0xdd, unkn, unkn, unkn, unkn, unkn, 0xe1, unkn, 0x72, unkn, 0x15, unkn, unkn, 0x30, unkn, 0x10, // 0xex
		unkn, unkn, unkn, unkn, 0xbe, unkn, 0xda, 0xdb, unkn, 0x1e, unkn, unkn, 0x36, unkn, unkn, unkn, // 0xfx
	};

	static const uint8_t opcodes_sub_table_0e[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xfd, unkn, 0x20, unkn, unkn, unkn, 0x35, unkn, unkn, 0xdd, 0x0a, 0xd8, 0x01, unkn, unkn, unkn, // 0x0x
		unkn, 0x04, 0xbf, unkn, 0x87, unkn, 0x05, unkn, unkn, unkn, 0x16, unkn, 0xa1, 0xaf, unkn, unkn, // 0x1x
		unkn, 0xf7, 0x46, unkn, 0x42, unkn, 0xb1, 0x47, unkn, 0xc4, unkn, unkn, unkn, 0xc0, 0x38, unkn, // 0x2x
		0x1e, unkn, unkn, unkn, unkn, 0x0f, 0x3d, unkn, unkn, unkn, unkn, unkn, 0xdf, unkn, unkn, unkn, // 0x3x
		0x57, 0x56, unkn, unkn, unkn, 0x79, 0xa2, unkn, unkn, unkn, 0x6f, unkn, 0x9e, 0xd7, unkn, 0xed, // 0x4x
		0x4e, unkn, unkn, 0x19, unkn, 0x15, unkn, unkn, 0x86, unkn, 0x23, unkn, unkn, unkn, 0xbb, unkn, // 0x5x
		unkn, unkn, unkn, 0x4f, 0xe7, 0x22, unkn, unkn, unkn, unkn, unkn, 0xd0, 0xc6, 0xb0, unkn, unkn, // 0x6x
		unkn, 0x7a, 0xd2, unkn, unkn, 0xc5, 0xe5, unkn, unkn, unkn, 0x0e, unkn, unkn, unkn, unkn, unkn, // 0x7x
		0xf5, unkn, unkn, unkn, 0x11, unkn, unkn, unkn, unkn, 0x2a, unkn, 0xd3, unkn, unkn, 0xcb, unkn, // 0x8x
		0x8e, 0x7c, 0x7e, unkn, 0xc1, unkn, unkn, unkn, 0x36, unkn, unkn, unkn, 0x21, unkn, 0xe9, unkn, // 0x9x
		0x3a, 0x4a, 0xa5, 0xba, unkn, 0x77, unkn, 0xd5, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xc3, // 0xax
		unkn, unkn, 0x3e, 0x32, unkn, unkn, unkn, unkn, unkn, 0x2b, 0xc9, 0xe6, unkn, unkn, unkn, unkn, // 0xbx
		unkn, unkn, 0x75, unkn, 0xa6, unkn, unkn, 0x07, unkn, unkn, 0xcf, unkn, 0x0d, unkn, 0xc2, unkn, // 0xcx
		unkn, unkn, unkn, unkn, 0x28, unkn, unkn, 0x1a, 0xe1, 0xd1, 0xc8, unkn, unkn, 0xb9, unkn, unkn, // 0xdx
		unkn, unkn, 0x12, 0xb7, unkn, unkn, unkn, unkn, 0x7f, unkn, unkn, unkn, 0x06, unkn, unkn, unkn, // 0xex
		0xd6, 0xca, unkn, 0x66, unkn, unkn, unkn, 0x7b, 0x18, 0xcd, 0x70, 0xfe, unkn, unkn, 0x71, 0xf1, // 0xfx
	};

	static const uint8_t opcodes_sub_table_0f[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, unkn, 0xaf, 0x0d, unkn, unkn, unkn, unkn, unkn, 0xb2, 0xe7, unkn, unkn, 0xd1, unkn, // 0x0x
		unkn, unkn, unkn, unkn, unkn, 0x0f, 0x77, unkn, unkn, 0xba, 0xa5, unkn, unkn, unkn, 0x18, 0xd3, // 0x1x
		0x79, 0x5f, unkn, 0xf6, unkn, unkn, unkn, 0x0c, unkn, unkn, unkn, unkn, unkn, unkn, 0x7c, unkn, // 0x2x
		unkn, unkn, unkn, 0xc5, unkn, unkn, unkn, unkn, 0xc6, unkn, unkn, 0x6f, 0xfd, 0xeb, unkn, unkn, // 0x3x
		unkn, 0x30, 0xe6, unkn, 0xfb, unkn, unkn, unkn, 0xb1, unkn, unkn, unkn, unkn, unkn, unkn, 0x36, // 0x4x
		0xcd, 0x04, unkn, unkn, 0x1d, 0x35, unkn, 0xdd, unkn, 0xb5, 0x12, unkn, unkn, unkn, unkn, unkn, // 0x5x
		unkn, unkn, 0x4e, 0x47, 0x38, unkn, unkn, 0x3e, 0x7e, unkn, unkn, unkn, unkn, unkn, unkn, unkn, // 0x6x
		unkn, unkn, unkn, 0x32, unkn, unkn, 0x06, unkn, unkn, unkn, unkn, 0xc9, unkn, 0x75, 0x29, 0x19, // 0x7x
		unkn, 0x5b, unkn, 0x2f, 0x28, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xb9, unkn, unkn, 0xfe, // 0x8x
		0x85, 0xdb, unkn, 0x57, 0xe5, unkn, unkn, 0x15, 0x0e, unkn, 0xd8, 0x09, 0x3a, unkn, unkn, unkn, // 0x9x
		unkn, 0x22, 0x16, 0xc1, unkn, unkn, unkn, unkn, 0x05, 0xc8, 0xe3, unkn, 0xbe, unkn, unkn, unkn, // 0xax
		unkn, 0x3d, unkn, unkn, 0x3c, 0x21, unkn, unkn, unkn, 0x23, unkn, unkn, 0x11, unkn, unkn, 0x2a, // 0xbx
		0xed, 0xc3, 0xe1, 0xd6, unkn, 0xb0, unkn, 0x1e, unkn, unkn, 0xf7, unkn, unkn, unkn, 0xf1, 0x34, // 0xcx
		unkn, 0x0a, unkn, 0xd0, 0x2b, unkn, unkn, 0x13, unkn, unkn, 0xd5, unkn, unkn, unkn, 0x08, unkn, // 0xdx
		0xd2, 0x01, unkn, 0x42, unkn, 0x20, 0xca, unkn, unkn, unkn, unkn, unkn, 0xb8, unkn, 0xc2, 0xc0, // 0xex
		0x31, 0xcb, 0x07, unkn, unkn, 0x27, unkn, unkn, unkn, unkn, 0xb7, 0x7a, unkn, 0x10, unkn, unkn, // 0xfx
	};

	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x2000; i++)
	{
		uint8_t x = rom[i];

		switch(i & 0x0f)
		{
			case 0x00: m_decrypted_opcodes[i] = opcodes_sub_table_00[x]; break;
			case 0x01: m_decrypted_opcodes[i] = opcodes_sub_table_01[x]; break;
			case 0x02: m_decrypted_opcodes[i] = opcodes_sub_table_02[x]; break;
			case 0x03: m_decrypted_opcodes[i] = opcodes_sub_table_03[x]; break;
			case 0x04: m_decrypted_opcodes[i] = opcodes_sub_table_04[x]; break;
			case 0x05: m_decrypted_opcodes[i] = opcodes_sub_table_05[x]; break;
			case 0x06: m_decrypted_opcodes[i] = opcodes_sub_table_06[x]; break;
			case 0x07: m_decrypted_opcodes[i] = opcodes_sub_table_07[x]; break;
			case 0x08: m_decrypted_opcodes[i] = opcodes_sub_table_08[x]; break;
			case 0x09: m_decrypted_opcodes[i] = opcodes_sub_table_09[x]; break;
			case 0x0a: m_decrypted_opcodes[i] = opcodes_sub_table_0a[x]; break;
			case 0x0b: m_decrypted_opcodes[i] = opcodes_sub_table_0b[x]; break;
			case 0x0c: m_decrypted_opcodes[i] = opcodes_sub_table_0c[x]; break;
			case 0x0d: m_decrypted_opcodes[i] = opcodes_sub_table_0d[x]; break;
			case 0x0e: m_decrypted_opcodes[i] = opcodes_sub_table_0e[x]; break;
			case 0x0f: m_decrypted_opcodes[i] = opcodes_sub_table_0f[x]; break;
		}
	}

	// data
	static const uint8_t data_sub_table_00[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xfd, unkn, unkn, unkn, 0x86, 0x6c, unkn, 0x00, 0x0c, 0xff, 0x60, unkn, unkn, unkn, 0x24, unkn, // 0x0x
		0x07, unkn, 0x78, unkn, 0x55, unkn, 0x1a, 0xe9, 0x5b, 0x32, 0x0b, unkn, unkn, unkn, unkn, unkn, // 0x1x
		0x62, unkn, unkn, unkn, unkn, unkn, 0x66, unkn, 0x49, 0xe4, unkn, 0x16, 0x0f, 0x17, unkn, unkn, // 0x2x
		0xce, unkn, 0x04, 0x5e, 0xa5, 0x3c, 0x01, unkn, 0x50, unkn, 0x54, unkn, 0x69, 0xf8, 0xc5, 0x82, // 0x3x
		0x45, unkn, unkn, unkn, 0x0a, 0x14, unkn, 0x93, 0x1f, unkn, unkn, 0x1b, 0x13, unkn, unkn, 0x4a, // 0x4x
		unkn, unkn, 0x63, 0x83, 0x0e, unkn, unkn, 0x7e, unkn, 0xd7, 0x39, unkn, unkn, 0x3b, 0xa7, 0xc6, // 0x5x
		0x23, 0x1d, unkn, unkn, unkn, unkn, 0x56, 0x90, 0x65, 0x44, unkn, unkn, 0x67, 0x57, 0x46, 0xef, // 0x6x
		0x3f, 0xf1, unkn, unkn, unkn, unkn, 0x10, 0xf0, unkn, unkn, unkn, 0x08, unkn, unkn, unkn, 0x3e, // 0x7x
		unkn, 0x41, unkn, unkn, unkn, 0x8c, 0x03, unkn, 0xa4, unkn, unkn, 0xd8, unkn, 0xb4, 0xee, 0x6e, // 0x8x
		0x96, 0x6a, 0x27, 0xad, 0x12, unkn, unkn, 0x30, 0x2b, 0x53, 0x48, unkn, 0x80, 0x42, 0x77, unkn, // 0x9x
		0x11, unkn, unkn, 0xb0, 0x52, 0xf9, unkn, 0x85, unkn, unkn, 0x5c, 0x09, unkn, 0x22, 0x4d, unkn, // 0xax
		unkn, 0xe0, 0x2a, 0x64, 0x1c, 0x05, 0x4c, unkn, 0x18, 0x2d, 0x20, unkn, 0xd0, unkn, unkn, 0x7c, // 0xbx
		unkn, unkn, unkn, unkn, 0x0d, unkn, 0x47, 0x15, 0x75, 0xd1, unkn, unkn, 0x2c, unkn, 0xc0, 0xbb, // 0xcx
		0xa2, unkn, 0x35, unkn, 0x4f, unkn, unkn, unkn, 0x2e, 0x43, unkn, unkn, 0x68, 0x25, unkn, 0xf3, // 0xdx
		unkn, 0x4b, 0xe8, 0xf4, unkn, unkn, 0xbf, unkn, 0x31, 0xf5, 0xa0, unkn, unkn, 0xc4, 0x1e, 0x4e, // 0xex
		0x8e, unkn, unkn, unkn, unkn, 0xe6, 0x76, 0x33, 0x06, unkn, unkn, unkn, unkn, 0x02, unkn, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_01[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, 0x52, unkn, unkn, 0x85, 0x34, 0x60, unkn, unkn, 0x56, unkn, unkn, 0x0b, unkn, 0x7f, // 0x0x
		unkn, 0x27, 0x9e, 0x1d, 0x63, 0x2a, 0xb0, unkn, unkn, unkn, unkn, 0x4e, unkn, 0x35, 0xea, unkn, // 0x1x
		0x0d, 0xe2, 0x40, 0x45, unkn, unkn, unkn, unkn, 0x08, 0xef, 0x21, unkn, 0x7e, 0x10, unkn, 0x2d, // 0x2x
		0x19, 0x0a, unkn, 0x0f, unkn, unkn, 0x50, 0xf4, unkn, unkn, 0x06, unkn, unkn, 0x4d, unkn, unkn, // 0x3x
		0x30, 0xb4, unkn, 0xf8, unkn, 0xad, 0x1c, 0x3b, unkn, 0x2b, 0x93, 0xe4, unkn, unkn, unkn, unkn, // 0x4x
		0x55, unkn, 0x2e, 0x05, 0xd7, 0x84, unkn, unkn, 0x6a, unkn, 0xcc, unkn, unkn, unkn, 0xe8, unkn, // 0x5x
		0xc0, unkn, 0xe9, 0x13, unkn, 0x4a, 0x61, 0x1b, 0xaa, unkn, unkn, 0x59, unkn, unkn, unkn, unkn, // 0x6x
		0x6c, unkn, unkn, unkn, 0xfb, unkn, unkn, unkn, 0x66, unkn, unkn, unkn, 0xf9, 0x4c, unkn, unkn, // 0x7x
		unkn, 0x53, 0xe6, 0x46, 0x49, 0x1f, 0x09, unkn, 0x00, unkn, 0x16, 0xa0, unkn, 0x17, 0xa2, 0xa4, // 0x8x
		unkn, 0x1e, 0x5a, unkn, 0x74, unkn, 0x5e, unkn, 0xcf, 0xc4, 0x65, unkn, unkn, unkn, 0x26, unkn, // 0x9x
		unkn, 0x1a, 0x7c, 0x4b, unkn, unkn, unkn, 0x51, 0xc2, 0x47, unkn, unkn, unkn, 0x43, 0x3c, 0x62, // 0xax
		unkn, unkn, unkn, 0x23, 0x18, 0x2c, 0x5b, 0x04, unkn, unkn, unkn, 0x42, 0xf0, 0x0e, unkn, 0xfc, // 0xbx
		0x0c, 0xf3, unkn, 0xdd, 0x32, unkn, 0x54, 0x4f, unkn, 0x41, unkn, unkn, 0x03, unkn, unkn, unkn, // 0xcx
		unkn, 0x7b, 0x68, unkn, 0x44, 0x02, 0x37, unkn, unkn, unkn, 0x48, 0x12, 0xb6, 0x20, unkn, 0x57, // 0xdx
		0x2f, unkn, 0x8e, 0x15, 0x07, unkn, unkn, 0xfd, unkn, 0xa6, 0xa1, unkn, unkn, unkn, unkn, unkn, // 0xex
		0x80, unkn, 0x01, 0x64, 0x39, 0x3d, unkn, 0x90, unkn, unkn, 0x76, 0xff, 0xf1, unkn, 0x11, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_02[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, 0x43, 0x4a, unkn, 0xb3, 0x93, 0x1a, 0xdc, unkn, unkn, unkn, 0xa7, 0x48, unkn, 0xd7, // 0x0x
		unkn, unkn, 0x05, unkn, unkn, unkn, unkn, unkn, 0x15, unkn, 0x60, 0x42, unkn, unkn, 0x2a, unkn, // 0x1x
		0x67, unkn, 0x38, unkn, 0x55, unkn, unkn, 0xac, 0x83, unkn, 0xa1, unkn, 0xf4, unkn, unkn, 0x46, // 0x2x
		unkn, 0x50, unkn, 0x18, 0x3e, 0x11, 0xe6, unkn, unkn, 0x62, 0x76, 0x5e, 0xe0, 0x0f, 0xfd, 0x2e, // 0x3x
		0x45, 0x33, 0x61, unkn, 0x2d, unkn, 0x1c, 0x24, 0x19, unkn, unkn, 0xc9, unkn, 0x02, 0xc7, unkn, // 0x4x
		0x2c, unkn, 0x0e, 0xc0, 0x08, unkn, 0xf8, 0x14, 0x7e, 0xad, 0x3b, unkn, unkn, 0x39, unkn, 0xf6, // 0x5x
		0xa5, unkn, 0x4c, unkn, 0x23, 0x54, unkn, unkn, unkn, 0x0a, unkn, unkn, unkn, unkn, unkn, unkn, // 0x6x
		0x2b, 0xdd, 0x6b, unkn, 0x5c, unkn, 0x00, unkn, unkn, unkn, unkn, 0x36, unkn, 0x57, 0xce, unkn, // 0x7x
		0x1d, unkn, unkn, unkn, 0xbf, 0x32, 0xf0, 0x4f, 0x16, 0x35, 0x63, 0x21, 0x0c, 0x1b, unkn, unkn, // 0x8x
		0x56, unkn, 0xef, unkn, 0x06, unkn, 0x59, 0x04, 0x41, 0xe8, unkn, unkn, 0x4b, unkn, unkn, unkn, // 0x9x
		0x28, 0x7b, 0x3d, unkn, unkn, 0x12, 0xb6, unkn, 0x44, 0xb4, unkn, unkn, unkn, 0x09, unkn, unkn, // 0xax
		0xd2, unkn, 0xff, 0x3c, 0x6a, 0x20, 0xab, unkn, unkn, 0x40, 0xee, 0xa0, unkn, 0xbb, 0x68, 0x1e, // 0xbx
		0xc5, 0x49, 0x6d, unkn, 0x5f, 0x17, 0xfc, unkn, unkn, 0xb0, unkn, unkn, 0x2f, unkn, 0xbe, unkn, // 0xcx
		unkn, unkn, 0x10, unkn, 0x3a, unkn, 0x4d, unkn, 0xf2, 0x0d, 0xd0, unkn, unkn, 0x34, unkn, 0xbd, // 0xdx
		unkn, unkn, 0x4e, 0x3f, unkn, 0xcc, 0x0b, 0x7c, 0x01, unkn, 0x88, 0x5a, 0x53, 0x03, unkn, unkn, // 0xex
		0xc6, unkn, 0xf7, 0x30, unkn, 0x13, 0x85, unkn, unkn, unkn, unkn, 0x51, 0x52, 0x80, 0x07, 0xf5  // 0xfx
	};

	static const uint8_t data_sub_table_03[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xad, 0x34, unkn, unkn, unkn, unkn, 0x30, 0x09, unkn, unkn, unkn, unkn, unkn, unkn, 0x17, 0x3e, // 0x0x
		0x60, unkn, unkn, unkn, 0xa2, unkn, 0x21, 0x39, 0x80, unkn, unkn, 0x7d, 0x05, 0x28, 0x06, 0x25, // 0x1x
		0x0d, unkn, 0x10, 0xb6, 0x26, unkn, 0x24, 0x57, 0xef, unkn, 0x0e, unkn, 0xee, 0x67, unkn, unkn, // 0x2x
		unkn, 0x1f, 0x32, unkn, unkn, 0x38, unkn, 0x69, unkn, 0xf6, unkn, unkn, 0x65, 0x2c, 0x23, 0x14, // 0x3x
		0x40, unkn, 0x04, 0x3c, 0x12, 0xe3, 0x4c, unkn, unkn, 0xf8, unkn, 0xc5, 0x4e, unkn, unkn, 0x15, // 0x4x
		0xf1, 0x20, unkn, 0x93, unkn, 0x94, unkn, 0x16, 0xa5, unkn, unkn, 0xdc, 0x5a, unkn, 0x19, unkn, // 0x5x
		0xff, unkn, 0x3b, unkn, 0x55, 0x4b, unkn, unkn, 0x54, 0x47, unkn, 0x50, unkn, unkn, 0x46, unkn, // 0x6x
		unkn, unkn, unkn, 0x13, unkn, 0x6e, unkn, unkn, unkn, 0xb0, unkn, 0xb3, 0x90, 0x6c, unkn, unkn, // 0x7x
		0xd7, 0xf0, unkn, 0x42, unkn, unkn, 0xc3, unkn, unkn, 0xd5, unkn, 0x22, 0x4a, unkn, 0x4f, unkn, // 0x8x
		unkn, 0x6a, 0xdb, 0xf7, unkn, 0x37, unkn, unkn, 0xfc, 0x02, unkn, unkn, 0xbd, unkn, unkn, unkn, // 0x9x
		0x11, 0x53, unkn, unkn, unkn, 0x1c, unkn, 0x52, 0x63, 0xbc, 0x41, 0xf9, unkn, unkn, 0xc4, unkn, // 0xax
		0x18, unkn, 0xbb, 0x0f, 0x4d, 0x1b, unkn, unkn, unkn, 0xe0, unkn, 0x61, 0x43, unkn, 0x44, 0xaa, // 0xbx
		unkn, 0x0b, unkn, unkn, 0xcc, 0x98, 0x59, 0x2d, 0x35, 0x1d, 0x49, 0x0c, 0x1a, unkn, 0xe9, unkn, // 0xcx
		0x6b, 0x3f, 0x7e, unkn, unkn, unkn, 0xf4, unkn, 0x9e, unkn, 0x2b, unkn, unkn, unkn, unkn, unkn, // 0xdx
		0x56, unkn, 0xa7, 0x0a, 0x45, 0x2e, 0x62, unkn, unkn, 0x2a, 0x48, unkn, 0xe6, 0x03, unkn, 0xd8, // 0xex
		unkn, 0x66, unkn, 0xf5, unkn, 0x08, 0x01, unkn, unkn, 0x8e, 0x00, unkn, unkn, 0x1e, unkn, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_04[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, 0x2f, unkn, unkn, 0xcc, unkn, 0x35, unkn, 0x4f, 0x3e, unkn, 0x12, unkn, unkn, 0x30, // 0x0x
		unkn, unkn, unkn, 0xf0, unkn, unkn, 0x64, 0xaf, 0xd9, unkn, 0x2c, unkn, 0x2b, unkn, unkn, unkn, // 0x1x
		0xee, 0xbf, unkn, 0x13, 0x65, 0x52, unkn, unkn, 0x4a, 0x09, 0xc3, 0x08, 0xa0, 0x04, unkn, 0x62, // 0x2x
		0x4d, unkn, unkn, 0x47, unkn, unkn, unkn, unkn, unkn, 0xc4, unkn, 0x3c, 0xf3, 0x67, 0x0d, unkn, // 0x3x
		0xb9, unkn, 0x14, 0x05, unkn, 0x4c, unkn, 0x60, unkn, 0xd3, 0xd4, unkn, unkn, unkn, unkn, 0x16, // 0x4x
		0xa7, 0x69, 0x9c, unkn, 0x90, 0x1f, unkn, unkn, unkn, 0x49, unkn, 0x56, unkn, 0xfe, unkn, 0x8e, // 0x5x
		0x27, 0x54, 0x66, unkn, 0x01, 0x03, 0x20, 0x0e, 0xf5, unkn, unkn, unkn, unkn, 0xc2, unkn, 0x25, // 0x6x
		unkn, 0x21, unkn, 0xad, 0x51, 0x23, unkn, 0xd0, 0x40, 0x06, unkn, unkn, unkn, unkn, 0xe9, 0x48, // 0x7x
		unkn, 0xc5, unkn, 0x45, unkn, unkn, unkn, 0x0f, 0xfd, unkn, 0xfa, 0x32, unkn, 0xf7, unkn, 0x68, // 0x8x
		unkn, unkn, 0x0a, 0xc1, 0x8c, 0x22, 0xed, 0x28, 0xb1, 0x55, 0x6a, unkn, unkn, 0x1a, 0x46, unkn, // 0x9x
		0x42, 0x24, 0x4b, unkn, unkn, unkn, unkn, unkn, unkn, 0x3f, 0x73, 0x00, 0xcb, 0x2d, unkn, unkn, // 0xax
		0x3b, 0xfc, unkn, 0xe8, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xbc, 0x15, 0x50, unkn, // 0xbx
		0x6f, 0x57, 0x33, 0x7c, unkn, unkn, 0xc6, unkn, 0xe0, 0x2a, unkn, 0x63, unkn, unkn, 0xff, 0xe6, // 0xcx
		0x41, 0x1e, 0x43, 0xf9, unkn, 0x76, 0x2e, unkn, 0x44, unkn, 0x58, 0x0c, unkn, 0x1d, 0x53, 0x1b, // 0xdx
		0x85, unkn, 0xa5, unkn, 0xe3, 0xb2, 0x02, 0x07, unkn, 0x91, 0xd8, unkn, 0x10, 0xca, unkn, unkn, // 0xex
		unkn, 0x80, 0xe2, unkn, unkn, 0x1c, unkn, 0x4e, unkn, unkn, 0x17, 0x18, 0x11, 0x0b, unkn, 0x7f  // 0xfx
	};

	static const uint8_t data_sub_table_05[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x2a, unkn, unkn, unkn, unkn, 0xe0, unkn, 0x3b, unkn, unkn, 0x79, 0x50, unkn, unkn, unkn, unkn, // 0x0x
		unkn, unkn, unkn, 0x4b, unkn, unkn, 0x28, unkn, 0x07, 0x0a, 0xc5, unkn, unkn, 0x69, unkn, unkn, // 0x1x
		unkn, 0x06, 0xc8, unkn, unkn, unkn, 0x3c, unkn, 0x6c, unkn, unkn, unkn, 0x11, 0x17, unkn, unkn, // 0x2x
		0x66, unkn, unkn, 0x8a, unkn, 0x09, 0xfb, unkn, unkn, 0xcf, 0xc9, unkn, 0xd4, 0x1f, 0xb0, 0x7c, // 0x3x
		unkn, unkn, 0x04, unkn, 0xff, 0x20, 0x02, 0x60, unkn, unkn, 0x12, 0x84, unkn, 0xef, 0xd7, 0xaa, // 0x4x
		unkn, 0x34, unkn, unkn, unkn, unkn, unkn, 0x49, 0x56, 0xbe, 0x10, 0x4d, 0x7f, 0x19, unkn, 0x1d, // 0x5x
		unkn, unkn, 0xf6, 0x36, unkn, unkn, unkn, unkn, unkn, 0xb6, 0xce, 0xf7, 0xb4, unkn, unkn, 0x43, // 0x6x
		unkn, 0x3e, 0x4c, 0xc3, unkn, unkn, unkn, 0x00, unkn, 0x93, unkn, 0x01, 0x16, 0x0f, unkn, unkn, // 0x7x
		unkn, unkn, unkn, unkn, unkn, unkn, 0xba, 0xbf, 0xf9, 0xad, 0x13, unkn, unkn, unkn, unkn, 0x44, // 0x8x
		unkn, 0x26, unkn, 0x32, 0xc0, 0x21, unkn, unkn, 0x24, 0x64, 0x14, unkn, unkn, 0x2d, 0x4f, 0x0c, // 0x9x
		unkn, unkn, 0x58, 0x1b, unkn, unkn, 0x99, unkn, 0x80, 0x2e, 0x7e, unkn, unkn, unkn, unkn, unkn, // 0xax
		0x46, 0xac, unkn, unkn, unkn, unkn, 0x4e, unkn, unkn, 0x08, 0xfe, 0x57, unkn, unkn, 0xda, 0xa9, // 0xbx
		0x63, 0x18, unkn, 0x0e, 0x3f, unkn, 0x76, 0x4a, unkn, 0xb3, 0x03, unkn, unkn, unkn, 0x6a, 0x52, // 0xcx
		unkn, unkn, unkn, 0xf5, 0x31, 0xf0, unkn, 0x41, 0x70, 0xe9, 0x05, 0xe6, 0xfc, unkn, 0x0d, 0x90, // 0xdx
		unkn, 0xa2, unkn, unkn, unkn, unkn, 0xe5, 0xf8, 0x62, 0x55, 0x45, unkn, unkn, 0x42, 0x68, 0x1e, // 0xex
		unkn, 0x48, 0x15, 0x54, 0x1a, 0xe4, 0x33, 0x53, 0x30, 0x61, 0x1c, 0x40, 0x22, 0x0b, unkn, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_06[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, 0xe6, unkn, unkn, unkn, 0xc6, unkn, unkn, 0xf1, unkn, 0x23, 0x4f, 0xb8, unkn, unkn, // 0x0x
		0x06, unkn, 0x7c, 0xda, unkn, unkn, 0xbf, unkn, unkn, unkn, 0x47, unkn, 0x44, 0x20, 0x93, 0x37, // 0x1x
		unkn, unkn, unkn, unkn, unkn, unkn, 0x2f, unkn, 0x27, 0xed, 0x56, unkn, 0x2d, unkn, 0x45, unkn, // 0x2x
		unkn, 0x58, 0x62, unkn, 0x65, unkn, unkn, unkn, 0x2a, unkn, unkn, unkn, 0x0c, unkn, unkn, 0x86, // 0x3x
		0x05, 0xce, 0x0f, unkn, 0x11, 0x01, unkn, unkn, unkn, unkn, unkn, 0x1e, unkn, unkn, 0x21, unkn, // 0x4x
		0x02, unkn, unkn, unkn, unkn, 0xf6, unkn, 0x80, 0x1a, unkn, 0x3d, 0x7b, 0x17, 0x70, 0x36, 0x77, // 0x5x
		0x5e, 0xd1, unkn, 0xd0, 0x0b, unkn, 0x2e, unkn, 0x48, unkn, 0x24, 0xb0, unkn, unkn, 0xff, 0x10, // 0x6x
		unkn, 0x40, 0xb6, unkn, 0x51, unkn, 0xe0, 0xad, 0x28, 0xc9, 0x9c, 0x1c, 0x57, unkn, 0x2c, 0x13, // 0x7x
		0x53, unkn, 0x31, unkn, 0x8e, 0x43, unkn, 0x54, 0x49, 0xe5, unkn, unkn, 0x64, unkn, 0x34, 0x35, // 0x8x
		0x1f, 0xf0, 0x3c, 0x4b, unkn, 0x19, unkn, unkn, 0x52, unkn, 0x00, 0x8c, unkn, 0xdf, unkn, 0x0d, // 0x9x
		0x08, 0x4a, 0x1b, 0x2b, unkn, 0x4c, unkn, unkn, 0x63, unkn, 0xcf, 0xa1, 0x1d, unkn, unkn, 0x16, // 0xax
		0xf5, unkn, 0x60, unkn, unkn, unkn, unkn, 0x50, 0x7a, 0x14, 0x03, 0xcb, unkn, unkn, unkn, unkn, // 0xbx
		0x30, unkn, 0x78, 0x4d, unkn, 0x6b, unkn, unkn, unkn, 0x95, unkn, 0x42, unkn, unkn, 0xbe, 0x12, // 0xcx
		0x83, 0xfe, 0x33, 0x90, unkn, 0xf4, 0x6e, 0x46, 0x07, 0x0a, 0xcc, 0x7f, unkn, unkn, unkn, 0xc0, // 0xdx
		unkn, 0x3b, 0xb9, 0xa4, 0x09, unkn, 0x59, 0xfc, unkn, 0x4e, unkn, 0xf9, 0x41, 0x9a, 0xd3, 0x0e, // 0xex
		unkn, unkn, unkn, 0x15, 0x32, unkn, 0x8b, 0x55, 0x18, 0x25, 0x04, 0x29, unkn, unkn, unkn, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_07[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x0e, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x3e, unkn, 0x08, unkn, unkn, 0x15, unkn, // 0x0x
		unkn, 0x32, unkn, unkn, unkn, unkn, 0x87, unkn, 0x04, 0x47, 0x56, unkn, unkn, 0x69, 0x1c, 0x1e, // 0x1x
		0x53, 0xed, 0x66, unkn, 0x40, unkn, 0x16, 0x2d, unkn, unkn, unkn, 0xda, unkn, 0xe5, unkn, unkn, // 0x2x
		0x54, 0x06, 0x43, 0x24, unkn, 0x9a, 0x6e, 0x50, unkn, 0xdf, unkn, 0x20, 0xbc, 0xbf, 0xcf, unkn, // 0x3x
		0x65, unkn, 0x4c, 0xb4, unkn, unkn, unkn, 0xf9, unkn, unkn, 0x1b, 0x8e, 0xe9, unkn, 0x01, 0x1d, // 0x4x
		0xa5, 0x1f, 0xf7, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x03, 0x28, 0x4e, 0x17, // 0x5x
		0x9e, 0xff, unkn, 0x05, 0x26, unkn, 0x0d, unkn, 0x3d, 0xcb, 0x11, 0x0f, 0x25, 0x1a, unkn, 0x38, // 0x6x
		0xfb, 0x0a, 0x5a, 0x2a, unkn, unkn, 0x4d, 0xe0, unkn, unkn, unkn, unkn, 0x9f, 0xf0, 0x68, unkn, // 0x7x
		unkn, 0xe8, 0x30, 0x60, unkn, 0xf4, 0x2e, 0x8b, 0x46, unkn, 0x27, unkn, 0x0b, 0xc0, 0x21, unkn, // 0x8x
		unkn, unkn, unkn, 0x83, unkn, 0x6a, unkn, 0x84, unkn, unkn, 0x33, 0x3a, unkn, 0x18, unkn, 0xb0, // 0x9x
		0xc9, unkn, 0x80, 0x3c, 0x07, 0xd5, unkn, 0x19, 0xc3, 0xcc, unkn, 0x57, unkn, 0x90, 0xfe, unkn, // 0xax
		0xa4, 0x23, 0xa0, unkn, unkn, 0x41, unkn, 0xca, 0x35, 0x48, unkn, 0x00, unkn, 0x13, unkn, 0x44, // 0xbx
		unkn, unkn, 0x4f, unkn, unkn, unkn, 0xc2, 0x09, unkn, 0x2b, 0xba, 0x34, unkn, unkn, 0x14, 0x2c, // 0xcx
		0x86, unkn, 0x10, 0x12, unkn, 0xad, unkn, 0x7c, unkn, 0x4a, unkn, 0x45, 0x59, unkn, 0x63, unkn, // 0xdx
		0x49, unkn, unkn, 0x67, 0x02, 0x4b, unkn, unkn, 0x6d, unkn, unkn, unkn, 0x55, 0x7e, unkn, unkn, // 0xex
		0xd0, 0xf6, unkn, unkn, unkn, unkn, unkn, unkn, 0xef, 0x42, 0x51, unkn, unkn, unkn, unkn, 0x52  // 0xfx
	};

	static const uint8_t data_sub_table_08[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x21, 0x6d, unkn, unkn, unkn, unkn, 0x32, unkn, unkn, 0x1a, 0x9a, unkn, 0x38, unkn, unkn, unkn, // 0x0x
		0xcb, 0x3e, unkn, unkn, 0x16, 0x4e, unkn, unkn, 0xa4, unkn, 0xf6, 0xd0, unkn, 0x63, 0x08, 0x46, // 0x1x
		0x02, unkn, 0x23, unkn, 0xed, unkn, 0xfa, unkn, unkn, unkn, unkn, 0x00, 0x2a, 0xc5, unkn, unkn, // 0x2x
		0x3b, 0xf3, unkn, unkn, unkn, 0x67, 0xe0, 0xff, unkn, 0x4f, 0x53, 0x68, unkn, unkn, unkn, unkn, // 0x3x
		0xf9, unkn, 0x2d, unkn, 0x5c, 0x1e, 0x0e, unkn, 0x54, unkn, unkn, unkn, 0x66, 0x4b, 0x0d, 0x90, // 0x4x
		0x52, 0xc4, 0xb3, 0xa0, unkn, unkn, unkn, 0xf8, 0x45, unkn, unkn, 0x6b, unkn, 0x4a, 0x0f, 0x1c, // 0x5x
		0xfc, 0x4d, 0x0c, 0x1d, unkn, 0x17, unkn, 0xad, unkn, 0xc2, 0xe6, unkn, 0x12, 0x04, 0x1b, 0x2b, // 0x6x
		0x62, 0x6a, 0x85, 0xee, 0xb4, unkn, 0x11, 0x64, 0xd7, unkn, unkn, unkn, 0xa2, 0xe9, 0x7c, 0xca, // 0x7x
		unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xc7, 0xb7, 0x18, 0xab, unkn, unkn, 0x2c, 0xa1, // 0x8x
		0x30, unkn, unkn, 0x73, 0x9c, unkn, 0x25, 0xd6, unkn, 0x03, 0xf4, 0x0b, 0xf0, 0x58, unkn, 0x7d, // 0x9x
		unkn, 0x97, 0x14, 0xd5, unkn, unkn, unkn, unkn, 0xb6, unkn, 0x57, unkn, 0x55, 0x60, 0x59, unkn, // 0xax
		0x06, 0x42, 0xe8, unkn, 0xc0, 0x20, unkn, unkn, unkn, unkn, unkn, unkn, 0x84, unkn, 0x01, unkn, // 0xbx
		0x0a, 0x09, 0x56, unkn, 0x44, unkn, unkn, 0x49, 0x39, 0xbc, unkn, 0x05, unkn, unkn, unkn, 0x8d, // 0xcx
		0x80, unkn, unkn, 0xcc, 0x41, 0xbf, unkn, 0x50, unkn, 0x65, unkn, 0x15, unkn, unkn, unkn, 0xb0, // 0xdx
		0x47, 0x93, unkn, 0x48, 0x5d, unkn, 0x37, unkn, 0x43, 0xfe, 0x40, 0x33, 0x07, 0x10, unkn, 0x76, // 0xex
		unkn, 0x13, 0xef, 0x3c, unkn, 0x8e, unkn, 0xc3, unkn, unkn, unkn, 0x7e, unkn, 0x2e, unkn, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_09[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xb1, 0xfd, unkn, 0xdc, unkn, unkn, 0x4f, 0x96, unkn, 0x2b, 0x34, unkn, 0x56, 0x90, 0x3b, 0xf5, // 0x0x
		unkn, unkn, unkn, 0x59, unkn, 0x2e, unkn, 0x58, 0x79, 0x0f, 0xb0, unkn, unkn, unkn, unkn, unkn, // 0x1x
		0xc0, unkn, 0xfc, unkn, unkn, unkn, 0x11, 0x2c, unkn, 0x80, unkn, 0x10, unkn, 0xf6, 0xc8, 0x3c, // 0x2x
		unkn, 0x37, 0x63, unkn, 0x02, unkn, unkn, unkn, 0x33, unkn, unkn, unkn, unkn, unkn, unkn, 0x62, // 0x3x
		unkn, unkn, 0x14, 0x2a, unkn, 0x38, 0x92, unkn, unkn, unkn, unkn, 0x45, unkn, 0x18, 0x0c, 0x46, // 0x4x
		0x1b, 0x1d, unkn, unkn, 0xe0, unkn, unkn, 0xad, unkn, 0x42, unkn, 0x03, 0xc6, 0xf1, 0xff, 0xf7, // 0x5x
		unkn, unkn, unkn, unkn, 0xa1, unkn, unkn, unkn, unkn, 0x13, 0x26, 0xb6, 0x19, unkn, unkn, 0xa4, // 0x6x
		unkn, 0x6a, 0xaa, unkn, 0x01, unkn, 0xf9, 0x20, unkn, 0x3e, unkn, 0x07, unkn, 0x06, unkn, unkn, // 0x7x
		0xc3, 0xd2, unkn, 0x2d, unkn, 0x0a, 0xc5, unkn, unkn, unkn, 0x44, 0xd7, 0x52, 0xf2, unkn, unkn, // 0x8x
		0x4c, unkn, 0x28, 0x48, unkn, 0x76, unkn, unkn, 0xd5, 0x50, 0xd4, 0x32, 0x0e, 0x08, 0x09, unkn, // 0x9x
		0x4d, 0x93, unkn, 0xf0, unkn, unkn, 0xbc, unkn, unkn, 0x16, unkn, unkn, 0x55, 0x3a, 0x8e, 0x60, // 0xax
		0xcc, 0xe8, unkn, unkn, unkn, unkn, unkn, unkn, 0x05, 0x53, unkn, 0xaf, unkn, 0xfa, 0xbb, unkn, // 0xbx
		0xd0, 0xfe, unkn, unkn, 0xee, unkn, unkn, 0x12, 0x85, unkn, 0x30, 0x15, unkn, 0x04, 0x00, 0xce, // 0xcx
		0x65, 0x4a, unkn, unkn, 0x0b, unkn, 0x49, unkn, unkn, 0x6c, 0x0d, 0x41, 0x7c, unkn, 0xda, unkn, // 0xdx
		0x4e, unkn, unkn, unkn, unkn, 0x17, unkn, unkn, 0xf8, 0xd6, unkn, unkn, unkn, unkn, 0x43, 0x4b, // 0xex
		unkn, 0x54, 0x1c, 0x1e, unkn, unkn, unkn, unkn, 0xe6, unkn, unkn, unkn, 0x95, unkn, 0x57, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_0a[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x20, unkn, unkn, 0x62, 0x69, unkn, unkn, 0xc0, unkn, unkn, 0x98, unkn, unkn, unkn, unkn, 0x0e, // 0x0x
		0x70, 0xfe, 0x19, unkn, unkn, 0x18, 0x2c, 0x57, unkn, unkn, 0xe9, 0x50, 0x46, 0x16, 0xe6, unkn, // 0x1x
		unkn, unkn, 0x03, unkn, 0xe4, unkn, 0x64, 0xe8, 0xad, unkn, 0x05, 0x0a, 0x21, 0x07, unkn, unkn, // 0x2x
		unkn, 0x0c, 0x56, 0x27, 0x4b, unkn, unkn, 0x80, 0x6c, unkn, unkn, 0x3f, 0x25, 0x63, 0x00, unkn, // 0x3x
		0x4e, unkn, 0xd3, 0x41, 0x39, unkn, unkn, 0x09, unkn, 0x2a, unkn, 0x5d, unkn, 0x04, 0xd5, 0x2d, // 0x4x
		0xbc, unkn, unkn, unkn, unkn, 0x91, unkn, unkn, 0x48, unkn, unkn, 0x35, 0x8e, unkn, 0xf7, unkn, // 0x5x
		unkn, unkn, unkn, 0x1d, unkn, unkn, 0xf5, unkn, 0xf2, 0x6e, unkn, unkn, unkn, 0xf8, unkn, unkn, // 0x6x
		unkn, 0x3b, unkn, 0x4c, unkn, 0x08, unkn, unkn, 0x85, unkn, 0x52, 0xc6, 0x60, unkn, 0x0f, unkn, // 0x7x
		0x76, unkn, 0x55, 0x66, unkn, 0x45, unkn, unkn, unkn, 0x5c, 0xd0, unkn, 0xa1, unkn, unkn, unkn, // 0x8x
		unkn, 0xf0, unkn, 0x78, unkn, unkn, 0x43, 0xfd, unkn, unkn, 0xa4, 0x75, 0x2e, 0x2b, unkn, 0x1f, // 0x9x
		0xd1, unkn, 0x54, unkn, 0x10, 0xbf, unkn, unkn, unkn, 0x1e, 0xc1, unkn, 0x0d, 0x32, 0xc7, unkn, // 0xax
		0xcf, unkn, 0x14, unkn, unkn, unkn, unkn, unkn, 0x01, unkn, unkn, unkn, 0x1c, unkn, 0x3c, unkn, // 0xbx
		0xa2, 0xf9, 0xb0, 0x44, unkn, 0x24, unkn, unkn, unkn, 0x12, 0x68, 0xe0, 0x11, 0x4d, unkn, 0x13, // 0xcx
		unkn, 0x94, 0x49, unkn, unkn, unkn, unkn, 0x17, 0x33, unkn, unkn, unkn, unkn, 0xdf, 0xf4, 0xfa, // 0xdx
		unkn, unkn, 0x30, unkn, 0x06, unkn, 0x02, unkn, 0x53, unkn, unkn, 0x9a, 0x92, 0x34, unkn, 0x0b, // 0xex
		0x4a, unkn, unkn, 0xc3, 0x4f, 0xf1, unkn, unkn, 0xfc, unkn, 0xef, 0xff, unkn, unkn, unkn, 0x51  // 0xfx
	};

	static const uint8_t data_sub_table_0b[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, 0x39, unkn, unkn, unkn, 0x69, 0x27, unkn, 0xf0, unkn, unkn, unkn, unkn, unkn, unkn, 0x57, // 0x0x
		0x9a, 0x10, unkn, 0x04, 0x08, 0xd6, unkn, unkn, 0x03, 0x6b, 0x1f, 0x37, unkn, 0x30, 0xe9, unkn, // 0x1x
		0xc4, unkn, 0x25, unkn, 0x7d, unkn, 0xe5, unkn, 0x1e, unkn, 0x54, 0xd0, unkn, 0x49, 0x80, unkn, // 0x2x
		0x78, unkn, 0x3f, 0x9f, 0xf7, 0x11, 0x67, 0xa2, unkn, 0x4a, 0x33, unkn, unkn, 0xef, unkn, 0xd1, // 0x3x
		0xd7, unkn, 0xfc, unkn, 0x3d, 0xfe, unkn, 0x14, 0x05, 0x0f, 0x3e, 0xc7, 0x45, 0xca, 0x59, 0x35, // 0x4x
		unkn, unkn, 0x13, 0x41, unkn, 0xcb, 0x09, 0x4d, unkn, 0xbb, 0x51, 0x66, unkn, unkn, unkn, unkn, // 0x5x
		unkn, unkn, unkn, unkn, unkn, 0x07, 0x8e, 0x2a, 0xdf, 0xb0, 0x19, 0x18, 0x2f, unkn, 0x1d, 0xc0, // 0x6x
		unkn, unkn, 0x52, unkn, 0x00, 0xf8, unkn, unkn, 0x20, unkn, 0xad, 0x1c, 0x68, unkn, 0x34, unkn, // 0x7x
		unkn, 0x1a, 0x0a, 0x62, 0x55, 0x32, unkn, unkn, 0xf4, unkn, unkn, unkn, unkn, 0xa3, 0x44, 0x85, // 0x8x
		unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x0e, 0x3c, unkn, unkn, // 0x9x
		unkn, unkn, 0x7f, 0x16, 0x48, unkn, unkn, 0x0c, unkn, 0x64, unkn, 0x50, unkn, 0x36, unkn, unkn, // 0xax
		unkn, 0x06, 0x83, 0xa4, 0x53, unkn, unkn, unkn, unkn, unkn, unkn, 0x47, unkn, unkn, unkn, 0x46, // 0xbx
		0x90, 0x60, unkn, 0xf6, unkn, 0xf5, 0xff, 0x31, unkn, 0x17, 0x43, 0xe6, unkn, 0x40, 0x02, unkn, // 0xcx
		0x56, 0x4b, 0xda, 0x15, unkn, 0x4f, unkn, unkn, unkn, unkn, 0x63, 0x0b, 0xa5, unkn, unkn, 0xbf, // 0xdx
		unkn, 0x01, 0x12, unkn, unkn, unkn, unkn, unkn, unkn, 0x0d, unkn, unkn, unkn, 0x1b, 0x4c, unkn, // 0xex
		unkn, unkn, 0x2d, unkn, 0x26, unkn, 0x84, 0x4e, 0x28, unkn, unkn, unkn, 0x2c, 0x2e, unkn, 0xe0  // 0xfx
	};

	static const uint8_t data_sub_table_0c[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x5e, unkn, 0xcb, 0x2a, 0x3f, 0x55, 0x51, 0xc5, 0x0b, unkn, 0x79, unkn, unkn, unkn, unkn, unkn, // 0x0x
		0x8e, 0x29, unkn, 0x62, unkn, unkn, unkn, 0x1e, unkn, unkn, unkn, 0x52, unkn, 0xff, 0xe6, 0xe2, // 0x1x
		unkn, 0x09, 0x7c, 0x18, unkn, 0x63, unkn, 0xdf, 0x60, unkn, 0x16, unkn, unkn, 0x2e, 0x3b, 0x3e, // 0x2x
		0x20, unkn, 0xca, 0x34, 0x1c, unkn, unkn, 0xdd, unkn, 0x2c, unkn, unkn, unkn, 0xc0, 0xda, unkn, // 0x3x
		0x40, 0x35, 0x1d, unkn, 0x43, 0xc2, unkn, unkn, 0x3d, unkn, 0x33, 0x88, 0x4d, unkn, 0xc3, 0x54, // 0x4x
		unkn, 0x48, unkn, 0x27, unkn, unkn, 0x38, unkn, unkn, 0x0d, unkn, 0x61, 0x17, 0x10, 0x45, unkn, // 0x5x
		0x0f, unkn, unkn, 0x57, 0x05, 0xaa, unkn, unkn, 0x75, unkn, unkn, 0x90, 0x1a, unkn, 0x24, unkn, // 0x6x
		unkn, 0x12, unkn, 0xfe, 0x21, unkn, 0x01, unkn, unkn, 0xe0, unkn, 0x4f, 0x32, unkn, unkn, unkn, // 0x7x
		unkn, 0x23, 0x3c, 0x00, unkn, unkn, unkn, unkn, unkn, unkn, 0xd0, 0x0a, 0x50, 0xf9, 0x59, 0xb6, // 0x8x
		0x28, 0xa3, unkn, 0x30, unkn, 0x44, unkn, unkn, 0x58, unkn, 0x11, 0xbb, 0x03, unkn, unkn, 0x13, // 0x9x
		0xf6, 0x06, 0xbf, 0x41, unkn, 0x74, 0x9a, 0xce, 0x98, 0xf1, unkn, unkn, 0x04, unkn, unkn, 0x80, // 0xax
		0x14, 0xb4, unkn, unkn, 0x08, 0xf0, 0x25, unkn, unkn, unkn, unkn, 0xea, 0x46, 0xe9, 0x31, 0x67, // 0xbx
		0x6c, unkn, 0xc7, 0x07, unkn, unkn, 0x0c, unkn, 0x2b, unkn, unkn, 0xcf, 0x4b, unkn, 0xc4, 0x47, // 0xcx
		0x9f, unkn, unkn, 0x53, unkn, unkn, 0x65, unkn, 0xa0, 0x15, unkn, unkn, unkn, 0xf4, 0x2d, 0xf8, // 0xdx
		0x02, unkn, 0x36, 0x85, 0x1b, unkn, unkn, 0x37, unkn, unkn, unkn, unkn, 0x5d, 0x66, 0xef, 0xed, // 0xex
		0x0e, unkn, 0x4a, unkn, 0xc6, 0x49, 0x4c, unkn, unkn, unkn, unkn, 0x4e, unkn, unkn, unkn, unkn  // 0xfx
	};

	static const uint8_t data_sub_table_0d[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		unkn, unkn, 0x12, 0x5e, unkn, 0x50, 0xae, unkn, 0x52, unkn, unkn, 0x20, 0x35, 0xf6, 0xd0, 0x11, // 0x0x
		0x7f, unkn, 0xe0, 0xe4, unkn, unkn, unkn, 0x48, unkn, 0xd1, 0xb0, 0xd7, unkn, 0xdd, 0x70, 0x6d, // 0x1x
		0x61, unkn, unkn, 0x53, unkn, unkn, unkn, 0x01, unkn, 0x2d, unkn, unkn, unkn, 0x4b, unkn, unkn, // 0x2x
		unkn, unkn, 0x18, 0xff, 0x2a, 0xfe, unkn, 0x4c, 0x3c, unkn, 0x10, unkn, 0xc2, 0x1c, unkn, 0x60, // 0x3x
		0x69, 0x02, unkn, 0x39, 0x0e, 0x73, 0x0a, 0x31, unkn, 0xf8, 0x55, unkn, 0xa2, unkn, unkn, unkn, // 0x4x
		unkn, unkn, unkn, unkn, 0xf1, 0x14, unkn, unkn, unkn, unkn, unkn, 0x04, unkn, 0xad, 0x44, 0x90, // 0x5x
		0x6b, unkn, 0x8a, 0x22, 0x0b, 0xe8, 0xf3, unkn, 0x0c, 0xc0, 0x28, 0xf7, 0x1b, unkn, 0x30, unkn, // 0x6x
		unkn, unkn, 0x00, unkn, unkn, unkn, 0x8b, unkn, 0x32, unkn, unkn, 0x15, 0x05, 0x46, unkn, 0x13, // 0x7x
		unkn, 0x45, 0x34, 0x06, 0x63, 0x2c, unkn, unkn, unkn, 0xcb, unkn, 0x0d, unkn, 0x08, unkn, unkn, // 0x8x
		unkn, 0x17, 0xcd, 0x64, unkn, 0xed, 0xf4, 0xcc, unkn, unkn, unkn, 0x4d, unkn, 0x7e, unkn, 0xc4, // 0x9x
		0x09, 0x3e, unkn, 0x65, 0x3b, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xce, unkn, // 0xax
		unkn, 0x7c, unkn, 0x3f, 0x4f, unkn, unkn, unkn, unkn, unkn, 0x80, unkn, 0xe6, 0x42, unkn, unkn, // 0xbx
		0xda, unkn, unkn, 0x1f, unkn, 0x2b, unkn, unkn, 0xe9, 0x23, 0x62, 0x19, unkn, unkn, 0x86, unkn, // 0xcx
		0xf5, 0xf0, 0x41, 0x43, unkn, 0x40, unkn, 0x25, 0xf9, 0x4a, unkn, 0x4e, 0x2e, 0x03, unkn, unkn, // 0xdx
		unkn, 0x88, 0x0f, 0xee, 0x16, 0x1e, 0xa0, 0xe2, unkn, 0x59, unkn, 0xa9, unkn, unkn, unkn, 0x6a, // 0xex
		unkn, 0x47, 0x49, 0x76, 0x07, unkn, unkn, unkn, 0x54, unkn, 0xb4, unkn, 0x96, unkn, unkn, 0x38  // 0xfx
	};

	static const uint8_t data_sub_table_0e[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0xff, unkn, unkn, 0xf0, 0x30, 0xed, 0x0b, 0x63, unkn, unkn, 0x3f, 0x1e, unkn, unkn, 0x41, 0xa6, // 0x0x
		unkn, 0x47, 0x2a, 0xd0, 0x4c, unkn, unkn, unkn, 0x51, unkn, unkn, 0xb0, 0xcf, unkn, unkn, 0x90, // 0x1x
		0x1f, 0x39, unkn, unkn, 0xbf, unkn, 0x0d, 0x07, 0x52, 0xaa, 0x15, unkn, unkn, 0x49, 0x14, 0xf4, // 0x2x
		unkn, 0x43, 0xe0, unkn, unkn, 0x85, unkn, 0x4e, 0x33, 0x60, unkn, 0xf7, 0x62, unkn, unkn, unkn, // 0x3x
		0x4a, unkn, unkn, unkn, 0x2c, unkn, 0x68, unkn, 0xe6, 0x4f, 0xe9, unkn, 0xc4, 0x53, unkn, 0x18, // 0x4x
		unkn, unkn, 0x57, 0x78, 0xef, 0x03, unkn, 0xc6, 0x19, 0x1d, unkn, 0x44, 0xc9, 0x96, unkn, unkn, // 0x5x
		unkn, unkn, unkn, 0x61, 0x0c, 0x3c, 0x45, unkn, 0x08, unkn, unkn, 0xcd, unkn, 0xfc, 0x1c, unkn, // 0x6x
		unkn, unkn, unkn, unkn, 0xf6, unkn, 0x79, unkn, unkn, 0x5d, 0xc7, 0xd1, 0x29, 0xc0, 0x32, 0x46, // 0x7x
		unkn, unkn, 0x7f, 0x9c, 0xd2, unkn, unkn, unkn, unkn, 0x0f, unkn, 0x4b, 0x00, 0xf9, 0x95, unkn, // 0x8x
		0xf1, unkn, 0x26, unkn, unkn, 0x2e, 0x28, unkn, 0xe4, unkn, unkn, 0x9d, 0x5b, unkn, unkn, 0x1a, // 0x9x
		0x2d, 0x7c, unkn, 0x88, unkn, unkn, 0xdf, 0x05, unkn, 0x0a, unkn, unkn, unkn, 0xad, 0xd7, unkn, // 0xax
		unkn, unkn, 0x06, unkn, 0x59, 0x54, unkn, 0x02, 0x12, 0x34, unkn, unkn, 0x20, 0x09, unkn, unkn, // 0xbx
		0x40, unkn, 0x55, unkn, unkn, 0x23, unkn, 0x38, 0x17, unkn, 0x16, unkn, 0x65, unkn, 0xeb, 0x24, // 0xcx
		unkn, unkn, 0x2b, unkn, unkn, unkn, 0x76, unkn, unkn, 0xbb, unkn, 0x13, unkn, 0xc5, 0xa0, 0x21, // 0xdx
		0x0e, unkn, 0xda, unkn, unkn, 0x6a, unkn, 0xd5, unkn, 0x04, unkn, 0x66, 0xf5, 0x50, unkn, 0xf8, // 0xex
		0xc3, unkn, 0xc2, unkn, unkn, 0x11, 0x48, 0x01, 0x2f, unkn, 0x3b, unkn, unkn, 0x10, 0xcb, 0xf2  // 0xfx
	};

	static const uint8_t data_sub_table_0f[0x100] =
	{// 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
		0x04, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0xb4, unkn, 0x50, unkn, 0x63, unkn, unkn, 0x20, // 0x0x
		unkn, unkn, 0x85, 0x19, 0x46, 0x00, unkn, 0x58, unkn, 0xcb, unkn, 0x41, 0xc0, 0x30, unkn, 0x48, // 0x1x
		unkn, 0x07, 0xf0, 0x65, unkn, 0xbf, 0x91, 0x45, 0x62, 0x35, 0x40, unkn, 0x56, unkn, unkn, unkn, // 0x2x
		0x69, 0x4c, 0x0d, 0xe9, unkn, unkn, unkn, 0xee, 0x5e, unkn, 0xf4, 0x49, 0xa3, 0x08, unkn, 0x0b, // 0x3x
		0x18, 0x43, unkn, unkn, 0x24, 0x0f, 0x5b, unkn, 0x86, unkn, unkn, 0x73, unkn, unkn, unkn, 0x0a, // 0x4x
		0x17, unkn, 0x4b, 0x52, 0xd5, 0xba, unkn, unkn, 0x53, unkn, unkn, unkn, 0xad, 0xe8, 0x3f, unkn, // 0x5x
		0xb0, unkn, 0xae, 0x1a, 0xc4, unkn, 0x11, 0x37, 0x67, unkn, 0x44, unkn, 0xb6, 0x4a, 0x2d, unkn, // 0x6x
		0x59, 0xd0, unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn, 0x61, 0x32, unkn, 0x6b, 0x3e, 0x26, // 0x7x
		unkn, unkn, unkn, 0xce, unkn, 0x7c, 0x0e, 0xb3, unkn, unkn, unkn, unkn, 0x16, unkn, 0x80, 0xbb, // 0x8x
		unkn, unkn, 0x2b, unkn, 0xc6, unkn, unkn, 0x54, 0x90, unkn, 0x09, 0x01, unkn, unkn, unkn, 0x05, // 0x9x
		0x1c, 0x1f, 0x55, unkn, unkn, 0xf8, 0x3b, 0xfe, 0xe0, unkn, 0x12, unkn, unkn, 0xe4, unkn, 0x34, // 0xax
		unkn, unkn, 0x10, unkn, unkn, unkn, unkn, 0xf5, 0x06, 0x57, 0x4f, 0x03, 0xd1, unkn, unkn, 0x3a, // 0xbx
		0x38, 0x6f, 0xa2, 0xe5, unkn, 0xed, 0x14, unkn, unkn, 0x4d, 0x0c, 0x7e, unkn, 0x2e, unkn, 0x6a, // 0xcx
		unkn, unkn, 0xa0, 0x22, 0x33, unkn, unkn, unkn, 0x15, 0x4e, unkn, 0xfb, 0x8e, unkn, unkn, unkn, // 0xdx
		unkn, 0xcf, unkn, 0x2c, 0x60, unkn, 0x1b, 0xe6, 0x02, unkn, 0x1d, unkn, unkn, 0x1e, unkn, unkn, // 0xex
		unkn, unkn, 0x47, unkn, unkn, 0xf9, 0x8a, 0x76, 0x6c, unkn, 0xfc, unkn, 0x64, 0x25, unkn, 0xff  // 0xfx
	};

	for (int i = 0; i < 0x2000; i++)
	{
		uint8_t x = rom[i];

		switch(i & 0x0f)
		{
			case 0x00: rom[i] = data_sub_table_00[x]; break;
			case 0x01: rom[i] = data_sub_table_01[x]; break;
			case 0x02: rom[i] = data_sub_table_02[x]; break;
			case 0x03: rom[i] = data_sub_table_03[x]; break;
			case 0x04: rom[i] = data_sub_table_04[x]; break;
			case 0x05: rom[i] = data_sub_table_05[x]; break;
			case 0x06: rom[i] = data_sub_table_06[x]; break;
			case 0x07: rom[i] = data_sub_table_07[x]; break;
			case 0x08: rom[i] = data_sub_table_08[x]; break;
			case 0x09: rom[i] = data_sub_table_09[x]; break;
			case 0x0a: rom[i] = data_sub_table_0a[x]; break;
			case 0x0b: rom[i] = data_sub_table_0b[x]; break;
			case 0x0c: rom[i] = data_sub_table_0c[x]; break;
			case 0x0d: rom[i] = data_sub_table_0d[x]; break;
			case 0x0e: rom[i] = data_sub_table_0e[x]; break;
			case 0x0f: rom[i] = data_sub_table_0f[x]; break;
		}
	}
}


void norautp_state::init_unka()
{
	uint8_t *ROM = memregion("maincpu")->base();
	ROM[0x0000] = 0x00;
	ROM[0x0001] = 0xf3;
	ROM[0x0002] = 0x31;
	ROM[0x0003] = 0xff;
	ROM[0x0004] = 0xd0;
	ROM[0x0005] = 0xc3;
	ROM[0x0006] = 0x80;
	ROM[0x0007] = 0x01;
	ROM[0x01d5] = 0xaf;
}


void norautp_state::init_kram4()
{
	uint8_t *rom = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(0x4000);

	memcpy(&buffer[0], rom, 0x4000);

	for (int i = 0; i < 0x4000; i++)
	{
		if ((i & 0x03) == 0x01) rom[i] = buffer[i + 0x01];
		if ((i & 0x03) == 0x02) rom[i] = buffer[i - 0x01];
		rom[i] = bitswap<8>(rom[i], 7, 5, 6, 4, 3, 2, 1, 0);
	}
}


/*********************************************
*                Game Drivers                *
*********************************************/

//************************************** Z80 sets **************************************
//*  The following ones are 'Draw Poker HI-LO' type, running in a Z80 based hardware   *
//**************************************************************************************

//     YEAR  NAME       PARENT    MACHINE   INPUT     CLASS          INIT        ROT   COMPANY                     FULLNAME                                        FLAGS                          LAYOUT
GAMEL( 1988, norautp,   0,        norautp,  norautp,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Poker",                                 0,                             layout_noraut11 )
GAMEL( 198?, norautdx,  0,        norautp,  norautpn, norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Deluxe Poker (console)",                0,                             layout_noraut12 )
GAMEL( 198?, norautpn,  norautp,  norautp,  norautpn, norautp_state, empty_init, ROT0, "bootleg",                  "Noraut Deluxe Poker (bootleg)",                0,                             layout_noraut12 )
GAMEL( 198?, norautjo,  0,        norautp,  mainline, norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Joker Poker (original)",                0,                             layout_noraut12 )
GAMEL( 198?, norautpt,  norautjo, norautp,  mainline, norautp_state, empty_init, ROT0, "bootleg",                  "Noraut Joker Poker (text hack)",               0,                             layout_noraut12 )
GAMEL( 198?, norautpl,  0,        norautpl, mainline, norautp_state, empty_init, ROT0, "Video Fun Games Ltd.",     "Noraut Joker Poker (Prologic HW)",             0,                             layout_noraut12 )
GAMEL( 1988, norautn1a, norautp,  norautxp, norautp,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Joker Poker (N1A)",                     0,                             layout_noraut12 )
GAMEL( 1988, norautjp,  norautp,  norautp,  norautp,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Joker Poker (alt)",                     0,                             layout_noraut11 )
GAMEL( 1988, norautrh,  0,        norautp,  norautrh, norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Red Hot Joker Poker",                   0,                             layout_noraut12 )
GAMEL( 198?, norautra,  0,        norautp,  norautrh, norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Red Hot Joker Poker (alt HW)",          0,                             layout_noraut12 )
GAME(  1988, norautu,   0,        norautxp, norautp,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Poker (NTX10A)",                        MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAMEL( 2002, noraut3a,  0,        noraut3,  noraut3,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Jacks Plus / Joker Poker (V3.010a)",    MACHINE_NOT_WORKING,           layout_noraut12 )  // settings are messed-up and gives 1993 coins when exit
GAMEL( 2003, noraut3b,  0,        noraut3,  noraut3,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Jacks Plus / Joker Poker (V3.011a)",    MACHINE_NOT_WORKING,           layout_noraut12 )  // settings are messed-up and gives 1993 coins when exit
GAMEL( 198?, norautua,  0,        norautu,  norautua, norautp_state, init_enc,   ROT0, "Noraut Ltd.",              "Noraut Joker Poker (console, set 1)",          MACHINE_UNEMULATED_PROTECTION, layout_noraut12 )
GAMEL( 198?, norautub,  norautua, norautu,  norautua, norautp_state, init_enc,   ROT0, "Noraut Ltd.",              "Noraut Joker Poker (console, set 2)",          MACHINE_UNEMULATED_PROTECTION, layout_noraut12 )
GAMEL( 198?, mainline,  0,        norautp,  mainline, norautp_state, empty_init, ROT0, "Mainline London",          "Mainline Double Joker Poker / Jacks Plus)",    0,                             layout_noraut12 )
GAMEL( 1996, df_djpkr,  mainline, norautp,  mainline, norautp_state, empty_init, ROT0, "DellFern Ltd.",            "Double Joker Poker / Jacks Plus (45%-75% payout)",      0,                    layout_noraut12 )
GAMEL( 1996, df_djpkra, mainline, norautp,  mainline, norautp_state, empty_init, ROT0, "DellFern Ltd.",            "Double Joker Poker / Jacks Plus (45%-75% payout, alt)", 0,                    layout_noraut12 )
GAMEL( 2005, ndxron10,  0,        norautp,  ndxron10, norautp_state, empty_init, ROT0, "<unknown>",                "Royal on Ten (hack of Noraut Deluxe Poker)",   0,                             layout_noraut12 )
GAMEL( 1999, cgip30e,   0,        norautx4, norautkl, norautp_state, empty_init, ROT0, "CGI",                      "Credit Poker (ver.30e, Extra Bonus)",          0,                             layout_noraut12 )  // CGI - Credit Poker, VERSION 30E
GAMEL( 1999, cgip30cs,  0,        norautx4, norautkl, norautp_state, empty_init, ROT0, "CGI",                      "Credit Poker (ver.30c, standard)",             0,                             layout_noraut12 )  // CGI - Credit Poker, VERSION 30C, 21/12/99
GAMEL( 1999, cgip30b,   cgip30cs, norautx4, norautkl, norautp_state, empty_init, ROT0, "CGI",                      "Credit Poker (ver.30b, 7 & 9 bonus)",          0,                             layout_noraut12 )  // CGI - Credit Poker, VERSION 30B, 7 &9 BONUS, 21/12/99 BY V.S. for CGI
GAMEL( 1998, cgip23b,   cgip30cs, norautx4, norautkl, norautp_state, empty_init, ROT0, "CGI",                      "Credit Poker (ver.23b, 7 & 9 bonus)",          0,                             layout_noraut12 )  // CGI - Credit Poker, VERSION 23B, 7 &9 BONUS, 31.03.98. BY V.S.
GAMEL( 198?, cgidjp,    0,        cgidjp,   mainline, norautp_state, empty_init, ROT0, "CGI",                      "Double Joker Poker (CGI)",                     0,                             layout_noraut12 )  // very similar to df_djpkr
GAME(  198?, kimblz80,  0,        kimble,   norautp,  norautp_state, empty_init, ROT0, "Kimble Ireland",           "Kimble Double HI-LO (Z80 version, encrypted)",   MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME(  198?, hilokimb,  0,        kimble,   norautp,  norautp_state, empty_init, ROT0, "Kimble Ireland",           "Kimble Double HI-LO (Z80 version, unencrypted)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAME(  198?, bjpoker,   0,        norautxp, norautrh, norautp_state, empty_init, ROT0, "M.Kramer Manufacturing.",  "Poker / Black Jack (Model 7521)",              MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // starts ok, but after the jump to $180 goes to hell
GAMEL( 1984, cdrawpkr,  0,        cdrawpkr, cdrawpkr, norautp_state, empty_init, ROT0, "Coinmaster",               "Draw Poker (Joker Poker V.01)",                0,                             layout_noraut11 )

// --------------------
GAMEL( 1988, winbnk88,  0,        norautp,  norautp,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Winbank Joker Poker",                          0,                             layout_noraut11 )
GAMEL( 1988, norautpb,  0,        norautxp, norautp,  norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Joker Poker 2000 (£15, Winbank)",              0,                             layout_noraut11 )
GAMEL( 1988, nor6007a,  0,        norautxp, nor6700a, norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Poker (6007 32k)",                      0,                             layout_noraut11 )
GAMEL( 1988, nor6007b,  0,        norautxp, nor6700b, norautp_state, empty_init, ROT0, "Noraut Ltd.",              "Noraut Poker (6007 64k)",                      0,                             layout_noraut11 )
GAMEL( 1996, ddellf96,  0,        norautp,  mainline, norautp_state, empty_init, ROT0, "Dellfern Ltd.",            "Double Joker Poker Dellfern 1996",             0,                             layout_noraut11 )
GAMEL( 1997, ddellf97,  0,        norautxp, ddellf97, norautp_state, empty_init, ROT0, "Dellfern Ltd.",            "Dellfern V18 (77's, 99's & Suits, 1 Credit)",  0,                             layout_noraut12 )
GAMEL( 1997, ddellf97a, ddellf97, norautxp, ddellf97, norautp_state, empty_init, ROT0, "Dellfern Ltd.",            "Dellfern V18 (77's, 99's & Suits, 10 Credits)",0,                             layout_noraut12 )
GAMEL( 1997, ddellf97b, ddellf97, norautxp, norautp,  norautp_state, empty_init, ROT0, "Dellfern Ltd.",            "Dellfern V18 (Joker Poker, raise bet, 2p/10p)",0,                             layout_noraut11 )
GAMEL( 1997, ddellf97c, ddellf97, norautxp, bankpkr,  norautp_state, empty_init, ROT0, "Dellfern Ltd.",            "Dellfern V18 (£6 Jackpot Bank Poker)",         0,                             layout_noraut11 )
GAMEL( 1997, ddellf97d, ddellf97, norautxp, delv18jp, norautp_state, empty_init, ROT0, "Dellfern Ltd.",            "Dellfern V18 (Joker Poker / Jack Plus)",       0,                             layout_noraut12 )
GAMEL( 1997, ddellf97e, ddellf97, norautxp, delv18ap, norautp_state, empty_init, ROT0, "Dellfern Ltd.",            "Dellfern V18 (Amusement Poker)",               0,                             layout_noraut12 )


//************************************* i8080 sets **************************************
//*  The following ones are 'Draw Poker HI-LO' type, running in a i8080 based hardware  *
//***************************************************************************************

//     YEAR  NAME       PARENT    MACHINE    INPUT      STATE          INIT        ROT   COMPANY                        FULLNAME                             FLAGS                       LAYOUT
GAMEL( 1982, dphl,      0,        dphl,      dphl,      norautp_state, empty_init, ROT0, "M.Kramer Manufacturing.",     "Draw Poker HI-LO (M.Kramer)",       0,                          layout_noraut10 )
GAMEL( 1983, dphla,     0,        dphla,     dphla,     norautp_state, empty_init, ROT0, "<unknown>",                   "Joker Poker (Kramer, alt)",         0,                          layout_noraut10 )
GAMEL( 1983, dphljp,    0,        dphl,      dphl,      norautp_state, empty_init, ROT0, "<unknown>",                   "Draw Poker HI-LO (Japanese)",       0,                          layout_noraut10 )
GAMEL( 198?, newhilop,  0,        dphl,      newhilop,  norautp_state, empty_init, ROT0, "Song Won?",                   "New Hi-Low Poker",                  0,                          layout_noraut10 )
GAMEL( 198?, pkii_dm,   0,        dphl,      newhilop,  norautp_state, empty_init, ROT0, "<unknown>",                   "Draw Poker HI-LO (PKII/DM)",        0,                          layout_noraut10 )
GAMEL( 1983, krampcb2,  0,        dphl,      dphla,     norautp_state, empty_init, ROT0, "bootleg",                     "Draw Poker HI-LO (bootleg, set 1)", 0,                          layout_noraut10 )
GAMEL( 198?, krampcb5,  krampcb2, dphl,      dphl,      norautp_state, empty_init, ROT0, "bootleg",                     "Draw Poker HI-LO (bootleg, set 2)", 0,                          layout_noraut10 )
GAMEL( 1983, pma,       0,        dphl,      newhilop,  norautp_state, empty_init, ROT0, "PMA",                         "PMA Poker",                         0,                          layout_noraut10 )
GAME(  198?, kimbldhl,  0,        kimbldhl,  norautp,   norautp_state, empty_init, ROT0, "Kimble Ireland",              "Kimble Double HI-LO",               MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
GAMEL( 1983, gtipoker,  0,        dphl,      gtipoker,  norautp_state, empty_init, ROT0, "Game Technology Inc.",        "GTI Double or Nothing Draw Poker",  0,                          layout_noraut10 )
GAME(  1983, gtipokra,  0,        gtipkra,   gtipoker,  norautp_state, empty_init, ROT0, "Game Technology Inc.",        "GTI Poker (SMS hardware)",          MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // second ROM 1000-1fff encrypted
GAMEL( 1983, gtipokrb,  gtipoker, dphl,      gtipoker,  norautp_state, empty_init, ROT0, "Game Technology Inc.",        "GTI Double or Nothing Draw Poker (Cleco)",   0,                 layout_noraut10 )  // working input under test
GAMEL( 1983, gtipokrba, gtipoker, dphl,      gtipoker,  norautp_state, empty_init, ROT0, "Game Technology Inc.",        "GTI Double or Nothing Draw Poker (GoT PCB, set 1)", 0,          layout_noraut10 )  // working input under test
GAMEL( 1983, gtipokrc,  gtipoker, dphl,      gtipoker,  norautp_state, empty_init, ROT0, "Game Technology Inc.",        "GTI Double or Nothing Draw Poker (GoT PCB, set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_noraut10 )  // encrypted
GAMEL( 1983, gtipokrd,  gtipoker, dphl,      gtipoker,  norautp_state, empty_init, ROT0, "Game Technology Inc.",        "GTI Double or Nothing Draw Poker (GoT PCB, set 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_noraut10 )  // encrypted

GAMEL( 1983, smshilo,   0,        dphl,      dphl,      norautp_state, empty_init, ROT0, "SMS Manufacturing Corp.",     "HI-LO Double Up Joker Poker (set 1)", 0,                          layout_noraut10 )
GAMEL( 1983, smshiloa,  smshilo,  dphl,      dphl,      norautp_state, empty_init, ROT0, "SMS Manufacturing Corp.",     "HI-LO Double Up Joker Poker (set 2)", MACHINE_NOT_WORKING,        layout_noraut10 )
GAMEL( 1986, drhl,      0,        drhl,      drhl,      norautp_state, empty_init, ROT0, "Drews Inc.",                  "Drews Revenge (v.2.89, set 1)",     0,                          layout_noraut10 )
GAMEL( 1986, drhla,     drhl,     drhl,      drhl,      norautp_state, empty_init, ROT0, "Drews Inc.",                  "Drews Revenge (v.2.89, set 2)",     0,                          layout_noraut10 )
GAMEL( 1982, ssjkrpkr,  0,        ssjkrpkr,  ssjkrpkr,  norautp_state, empty_init, ROT0, "Southern Systems & Assembly", "Southern Systems Joker Poker",      0,                          layout_noraut10 )
GAMEL( 198?, fastdrwp,  0,        dphl,      fastdrwp,  norautp_state, empty_init, ROT0, "Stern Electronics",           "Fast Draw (poker conversion kit)",  0,                          layout_noraut10 )
GAMEL( 1983, sureshoto, 0,        dphl,      sureshoto, norautp_state, init_unka,  ROT0, "SMS Manufacturing Corp.",     "Sure Shot (older, dphl hardware)",  MACHINE_IMPERFECT_COLORS,   layout_noraut09_sureshot )
GAMEL( 198?, dphlunkb,  0,        dphl,      dphla,     norautp_state, empty_init, ROT0, "<unknown>",                   "Unknown Draw Poker HI-LO",          0,                          layout_noraut10 )
GAME(  1989, pokplus,   0,        dphlxtnd,  norautp,   norautp_state, empty_init, ROT0, "LJF Corporation",             "Poker Plus 6 Cards (set 1)",        MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  1989, pokplusa,  pokplus,  dphlxtnd,  norautp,   norautp_state, empty_init, ROT0, "LJF Corporation",             "Poker Plus 6 Cards (set 2)",        MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  1989, pokplusb,  pokplus,  dphlxtnd,  norautp,   norautp_state, empty_init, ROT0, "LJF Corporation",             "Poker Plus 6 Cards (set 3)",        MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  1984, krampcb3,  0,        krampcb4,  dphl,      norautp_state, empty_init, ROT0, "M.Kramer Manufacturing.",     "System 3000 Kramergame Poker (encrypted set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  1984, krampcb4,  0,        krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "M.Kramer Manufacturing.",     "System 3000 Kramergame Poker (encrypted set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  1984, krampcb6,  0,        krampcb4,  dphl,      norautp_state, empty_init, ROT0, "M.Kramer Manufacturing.",     "System 3000 Kramergame Poker (encrypted set 3)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  1984, krampcb7,  0,        krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "M.Kramer Manufacturing.",     "System 3000 Kramergame Poker (encrypted set 4)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  1984, krampcb8,  0,        krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "M.Kramer Manufacturing.",     "System 3000 Kramergame Poker (encrypted set 5)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  198?, krampcb9,  0,        krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "M.Kramer Manufacturing.",     "unknown Kramer Poker (encrypted)",  MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  198?, topdraw,   0,        krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "SMS Manufacturing Corp.",     "Top Draw (encrypted, set 1)",       MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  198?, topdrawa,  topdraw,  krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "SMS Manufacturing Corp.",     "Top Draw (encrypted, set 2)",       MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  198?, topdrawb,  topdraw,  krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "SMS Manufacturing Corp.",     "Top Draw (encrypted, set 3)",       MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted
GAME(  198?, topdrawba, topdraw,  krampcb4,  dphl,      norautp_state, init_kram4, ROT0, "SMS Manufacturing Corp.",     "Top Draw (encrypted, set 4)",       MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )  // encrypted

// The following ones also have a custom 68705 MCU

GAMEL( 1991, tpoker1,   0,        dphl,      dphl,      norautp_state, empty_init, ROT0, "Micro Manufacturing",         "Turbo Poker 1 (set 1)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_noraut10 )
GAMEL( 1990, tpoker1a,  tpoker1,  dphl,      dphl,      norautp_state, empty_init, ROT0, "Micro Manufacturing",         "Turbo Poker 1 (set 2)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_noraut10 )

GAMEL( 1991, tpoker2,   0,        tpoker2,   tpoker2,   norautp_state, empty_init, ROT0, "Micro Manufacturing",         "Turbo Poker 2 (set 1)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_noraut10 )
GAMEL( 1990, tpoker2a,  tpoker2,  tpoker2,   tpoker2a,  norautp_state, empty_init, ROT0, "Micro Manufacturing",         "Turbo Poker 2 (set 2)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_noraut10 )
GAMEL( 1990, tpoker2b,  tpoker2,  tpoker2,   tpoker2a,  norautp_state, empty_init, ROT0, "Micro Manufacturing",         "Turbo Poker 2 (set 3)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_noraut10 )
GAMEL( 1990, tpoker2c,  tpoker2,  tpoker2,   tpoker2a,  norautp_state, empty_init, ROT0, "Micro Manufacturing",         "Turbo Poker 2 (set 4)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_noraut10 )
GAMEL( 1990, tpoker2d,  tpoker2,  tpoker2,   tpoker2a,  norautp_state, empty_init, ROT0, "Micro Manufacturing",         "Turbo Poker 2 (set 5)",             MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_noraut10 )

//************************************ unknown sets ************************************
//* The following ones are still unknown. No info about name, CPU, manufacturer, or HW *
//**************************************************************************************

//     YEAR  NAME       PARENT    MACHINE    INPUT      STATE          INIT        ROT   COMPANY                     FULLNAME                               FLAGS                       LAYOUT
