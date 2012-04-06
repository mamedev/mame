/*********************************************************************************************

    LUCKY 74 - WING CO.,LTD.
    ------------------------

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Lucky 74 (bootleg, set 1), 1988, Wing Co.,Ltd.
    * Lucky 74 (bootleg, set 2), 1988, Wing Co.,Ltd.


    Special thanks to Grull Osgo that helped a lot with the whole audio system and custom IC's
    reverse-engineering, and Tomasz Slanina that found how NMI interrupts are triggered.


**********************************************************************************************


    Hardware Notes:
    ---------------

    CPU:  1x Big black box stickered 'WING CPU A001' (based on a Z80 @ 3 MHz.)

    Co-Processor: 1x NPC SM7831 @ 1.5 MHz.

    Sound: 3x SN76489 @ 3 MHz.
           1x Yamaha YM-2149F @ 1.5 MHz.
           1x OKI M5205 (4-bit ADPCM samples @ 8kHz).

    ROMs: 1x 27C512 (program).
          1x 27C512 (sound samples).
          8x 27C256 (graphics).

    PROMs: 6x 24s10 (color system).


    Clock: 1x Xtal 12.000 MHz.


    Other: 2x M5M82C255ASP (2x 8255 PPI each).
           1x M5L8251AP-5  (8251 USART).
           1x DIP40 custom IC silkscreened '09R81P 7K4' @ 3 MHz.
           1x DIP28 custom IC silkscreened '06B49P 2G1' @ 3 MHz.
           2x DIP28 custom IC silkscreened '06B53P 9G3' @ 1.5 MHz.
           1x 3.6V lithium battery.
           4x 8 DIP switches banks.
           1x Reset switch.

    Connectors: 1x (2x36) edge connector.
                1x (2x10) edge connector.
                1x 6-pins connector.


    There are 2 extra marks on the black CPU box:

    Silkscreened: 'B 0L2'

    Sticker:      'WE8703 1992.10'
                    'LUCKY 74-7'


    PCB is original from WING Co.,Ltd.


    The M5M82C255ASP (82c255) is a general purpose programmable I/O. It is compatible with
    the 8255 programmable I/O. The 82c255 is equivalent to two 8255s. It has 48 I/O lines
    which may be individually programmed in 6 groups of 8 or 4 groups of 12 and used in 3
    major modes of operation.

    For custom IC's 09R81P, 06B49P, and 06B53P, see the reverse-engineering notes below.


**********************************************************************************************


    Color Circuitry
    ---------------

    Here is a little diagram showing how the color PROMs are connected to a 74174
    and therefore to a resistor network that derives to the RGB connector.


                                  220
    (E6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (E7)24s10-12 _|                        |
                                  470      |
    (E6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (E7)24s10-11 _|                        |
                                   1K      |
    (E6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (E7)24s10-10 _|                        |
                                   2K      |
    (E6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Red
    (E7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (D6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (D7)24s10-12 _|                        |
                                  470      |
    (D6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (D7)24s10-11 _|                        |
                                   1K      |
    (D6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (D7)24s10-10 _|                        |
                                   2K      |
    (D6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Green
    (D7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (C6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (C7)24s10-12 _|                        |
                                  470      |
    (C6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (C7)24s10-11 _|                        |
                                   1K      |
    (C6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (C7)24s10-10 _|                        |
                                   2K      |
    (C6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Blue
    (C7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _


    Regarding the abobe diagram, there are 2 different states controlled by both 06B53P.
    Each state arrange a different palette that will be assigned to each graphics bank.

    As we can see here, same pin of different PROMs are connected together in parallel.

    To reproduce the states, we need to create a double-sized palette and fill the first
    half with the values created through state 1, then fill the second half with proper
    values from state 2.


*****************************************************************************************


    *** Dev Notes ***


    This hardware doesn't seems to be designed for a game like Lucky 74. The co-processor
    existence, the apparently unused 2x SN76489, the YM2149 used only for input port and
    NMI trigger purposes and the USART communication system (among other things) indicate
    that a more elaborated kind of games were meant to be running on this hardware.
    For Lucky 74, is a big waste of resources.


    Regular sounds and music are driven through one SN76489 mapped at 0xf100. The other two
    (mapped at 0xf300 & 0xf500) seems to be only initialized at the beginning but not used.

    Samples are driven through a custom DIP-40 IC silkscreened '09R81P'.

    Reads to Z80 port 0x00 are in fact requests of the 09R81P /Busy signal, that is tied to
    M5202 status. The game code continue reading till the less significant bit (bit 0) is
    activated. In case of lack of response, the code loops reading the port and the game is
    stuck at this point.


    There are 14 samples inside the luckyson.15 ROM.

    Here is a table with the writes (in respective order) by event...


    Z80 PORT:                  01  00  02  03  04  05  (SAMPLE)
    -----------------------------------------------------------

    Coin                       EF  E0  00  DF  F6  01
    D-UP LOSE                  70  E0  00  DF  7B  01
    D-UP WIN                   93  E0  00  DF  AA  01

    (complete samples set)

    Test Mode HOLD1 & BET      00  00  00  FF  11  01  "geez!" or "cheers!"
    Test Mode HOLD2            12  00  00  FF  24  01  "sorry..."
    Test Mode HOLD3            25  00  00  FF  36  01  "try again"
    Test Mode HOLD4            37  00  00  DF  46  01  "whoops!"
    Test Mode HOLD5            46  E0  00  DF  56  01  "you're great"
    (still not found)          56  E0  00  DF  69  01  "oh dear..."
    D-UP BIG                   69  E0  00  7F  6E  01  "big" (*)
    D-UP SMALL                 70  E0  00  DF  7B  01  "small" (*)
    Test Mode START            7B  E0  00  DF  93  01  "you lose"
    Test Mode D-UP / D-UP WIN  93  E0  00  DF  AA  01  "you win"
    (still not found)          AA  E0  00  DF  C3  01  "boy.. you're hot!"
    Test Mode SMALL            C3  E0  00  DF  DE  01  "lucky you..."
    Test Mode CANCEL           DE  E0  00  DF  EF  01  "call attendant"
    Coin                       EF  E0  00  DF  F6  01  "ready?"

    (*) "big" and "small" are splitted from the sample "big or small".


    So, you can easily see that writes to ports 0x00-0x01 define the start (pos) offset,
    and writes to ports 0x03-0x04 the ending offset of each sample to be played.

    Then, writing 0x01 to port 0x05 (bit 0 activated) just trigger the sample.

    Once finished, the M5205 is ready again and the 09R81P clear the /Busy flag (port 0x00,
    bit 0 activated).



*****************************************************************************************


    *** Custom IC's Reverse Engineering ***


    ======================
    Custom 06B49P - DIP 28
    ======================

    This IC is a programmable clock divisor. It provides every frequency needed for all
    devices from this hardware, plus V-Sync, H-Sync and (V+H)-Sync (composite) frequencies.

    All generated clocks are proportional to the Clock In (12MHz). There are not fixed or
    harcoded frequencies.


    Pinout
    ======

    General In Lucky74             Measured     Comments
    ------------------------------------------------------------------
    01 - Clock In                  (12 MHz)
    02 - Clock Out 1 (In/2)        (6 MHz)
    03 - Clock Out 2 (In/4)        (3 MHz)
    04 - Clock Out 3 (In/4)        (3 MHz)      Yes, again!
    05 - Clock Out 4 (In/8)        (1.5 MHz)
    06 - Clock Out 5 (In/16)       (750 kHz)
    07 - GND
    08 - Clock Out 6 (In/32)       (375 kHz)    Clock for OKI M5205.
    09 - Clock Out 7 (In/64)       (187.5 kHz)
    10 - Clock Out 8 (In/128)      (93.75 kHz)
    12 - Clock Out 9 (In/256)      (46875 Hz)
    13 - +5 Vcc
    14 - GND
    -------------------------------------------------------------------
    15 - GND
    16 - Unknown...                             Logic State "0". Not Connected.
    17 - Clock Out 10 (In/?)       (7782 Hz)    In/1500 (8000)?...
    18 - Clock Out 11 (In/?)       (3920 Hz)    In/3000 (4000)?...
    19 - Clock Out 12 (In/?)       (1960 Hz)    In/6000 (2000)?...
    20 - Clock Out 13 (In/?)       (950 Hz)     In/12500 (960)?...
    21 - +5 VCC.
    22 - Clock Out 14 (In/?)       (475 Hz)     In/25000 (480)?...
    23 - Clock Out 15 (In/?)       (237 Hz)     In/50000 (240)?...
    24 - Clock Out 16 (In/100000)  (120 Hz)
    25 - Clock Out 17 (In/200000)  (60 Hz)
    26 - H-Sync                    (15625 Hz)   H-Sync (Interlaced). CLKOUT09/3.
    27 - V-Sync                    (60 Hz)      V-Sync. Same as CLKOUT17.
    28 - (H+V)-Sync                             Composite Sync (Interlaced).



    ======================
    Custom 06B53P - DIP 28
    ======================

    This IC is a custom video controller. The PCB has two of them.
    Each one handle one graphics bank, the respective video (and color) RAM,
    and switch the dual-state color circuitry to generate its own palette.


    Pinout
    ======

    LUCKY 74 - E2                Comments
    ------------------------------------------------------
    01 - I - Flip State.
    02 - I - CLK1 1.5 MHz.       In phase with 6 MHz & 3 MHz.
    03 - I - CLK2 12 MHz.
    04 - I - CLK3 1.5 MHz.       In phase with 6 MHz.
    05 - I - CLK4 3 MHz.         In phase with 6 MHz.
    06 - I - Data 0              EP11 & EP12.
    07 - I - Data 1              EP11 & EP12.
    08 - I - Data 2              EP11 & EP12.
    09 - I - Data 3              EP11 & EP12.
    10 - I - Data 4              EP11 & EP12.
    12 - I - Data 5              EP11 & EP12.
    13 - I - Data 6              EP11 & EP12.
    14 - I - Data 7              EP11 & EP12.
    ------------------------------------------------------
    15 - I - Data 7              EP13 & EP14.
    16 - I - Data 6              EP13 & EP14.
    17 - I - Data 5              EP13 & EP14.
    18 - I - Data 4              EP13 & EP14.
    19 - I - Data 3              EP13 & EP14.
    20 - I - Data 2              EP13 & EP14.
    21 - +5V VCC.
    22 - I - Data 1              EP13 & EP14.
    23 - I - Data 0              EP13 & EP14.
    24 - GND                     Ground.
    25 - O - PROM Selector       C6-D6-E6.A0.
    26 - O - PROM Selector       C6-D6-E6.A1.
    27 - O - PROM Selector       C6-D6-E6.A2.
    28 - O - PROM Selector       C6-D6-E6.A3.


    LUCKY 74 - E1                Comments
    ------------------------------------------------------
    01 - I - Flip State.
    02 - I - CLK1 1.5 MHz.       In phase with 6 MHz & 3 MHz.
    03 - I - CLK2 12 MHz.
    04 - I - CLK3 1.5 MHz.       In phase with 6 MHz.
    05 - I - CLK4 3 MHz.         In phase with 6 MHz.
    06 - I - Data 0              EP16 & EP17.
    07 - I - Data 1              EP16 & EP17.
    08 - I - Data 2              EP16 & EP17.
    09 - I - Data 3              EP16 & EP17.
    10 - I - Data 4              EP16 & EP17.
    12 - I - Data 5              EP16 & EP17.
    13 - I - Data 6              EP16 & EP17.
    14 - I - Data 7              EP16 & EP17.
    ------------------------------------------------------
    15 - I - Data 7              EP18 & EP19.
    16 - I - Data 6              EP18 & EP19.
    17 - I - Data 5              EP18 & EP19.
    18 - I - Data 4              EP18 & EP19.
    19 - I - Data 3              EP18 & EP19.
    20 - I - Data 2              EP18 & EP19.
    21 - +5V VCC.
    22 - I - Data 1              EP18 & EP19.
    23 - I - Data 0              EP18 & EP19.
    24 - GND                     Ground.
    25 - O - PROM Selector       C7-D7-E7.A0.
    26 - O - PROM Selector       C7-D7-E7.A1.
    27 - O - PROM Selector       C7-D7-E7.A2.
    28 - O - PROM Selector       C7-D7-E7.A3.



    ======================
    Custom 09R81P - DIP 40
    ======================

    This custom IC is a kind of samples system controller, driving the OKI M5205.
    The IC is connected to Z80 through ports 0x00 to 0x05. Transmit the status (/busy)
    to port 0x00 (bit 0). Load the sample start offset from ports 0x00 & 0x01 and the
    ending offset from ports 0x03 & 0x04, then trigger the sample when the bit 0 of port
    0x05 is activated.

    Still unknown the use of 3rd register (connected to Z80 port 0x02).


    REGISTER  STATE    FUNCTION
    -----------------------------------------------------------------------
    0x00      WRITE    Write the status (/busy) activating bit 0 when is ready.
    0x00      READ     Read the less significant byte of the start offset.
    0x01      READ     Read the most significant byte of the start offset.
    0x02      READ     Unknown...
    0x03      READ     Read the less significant byte of the end offset.
    0x04      READ     Read the most significant byte of the end offset.
    0x05      READ     If the bit 0 is activated, just trigger the sample.


    Pinout
    ======

    General                       Comments
    ---------------------------------------------------------------
    01 - Low Nibble Enable        Half sample rate.
    02 - High Nibble Enable       Half sample rate inverted.
    03 - /BUSY                    LOW while playing.
    04 - /Read Strobe             LOW to read from Data Bus.
    05 - /Write Strobe            LOW to write on Data Bus.
    06 - A0                       Internal Register address 0.
    07 - A1                       Internal Register address 1.
    08 - A2                       Internal Register address 2.
    09-  VCK                      From OKI M5205 VCK.
    10 - GND                      Ground.
    11 - /RES                     LOW to RESET.
    12 - D0                       Data Bus.
    13 - D1                       Data Bus.
    14 - D2                       Data Bus.
    15 - D3                       Data Bus.
    16 - D4                       Data Bus.
    17 - D5                       Data Bus.
    18 - D6                       Data Bus.
    19 - D7                       Data Bus.
    20 - SA15                     Sound ROM Address.
    ---------------------------------------------------------------
    21 - GND
    22 - SA14                     Sound ROM Address.
    23 - SA13                     Sound ROM Address.
    24 - SA12                     Sound ROM Address.
    25 - SA11                     Sound ROM Address.
    26 - SA10                     Sound ROM Address.
    27 - SA09                     Sound ROM Address.
    28 - SA08                     Sound ROM Address.
    29 - SA07                     Sound ROM Address.
    30 - +5 VCC.
    31 - SA06                     Sound ROM Address.
    32 - SA05                     Sound ROM Address.
    33 - SA04                     Sound ROM Address.
    34 - SA03                     Sound ROM Address.
    35 - SA02                     Sound ROM Address.
    36 - SA01                     Sound ROM Address.
    37 - SA00                     Sound ROM Address.
    38 - Unknown.
    39 - Unknown.
    40 - GND                      Ground.



*****************************************************************************************


    *** Game Notes ***


    This game was one of the most wanted 'classics' regarding gambling games.

    Lucky 74 is a strip poker game with anime theme. It has a nice double-up feature, and
    the objective is obviously to win hands till you can see the girl naked, like other
    strip poker games.


    To enter the test mode, press F2 and then reset. To exit, press F2 again and then reset.

    To enter the book-keeping mode, press BOOKS (key 0), and then press BOOKS again to
    change between pages. Press START (key 1) to exit the mode.



    - DIP Switch 3-7 change the poker mode, but also the pay table.

    Table 1 (per unit): 500 100 40 10 7 5 3 2 1
    Table 2 (per unit): 500 200 100 40 10 7 5 3 2


    If Bet Max (DSW3-1) is 20:

    Table 1 (max): 10000 2000 800 200 140 100 60 40 20
    Table 2 (max): 10000 4000 2000 800 200 140 100 60 40

    If Bet Max (DSW3-1) is 40:

    Table 1 (max): 20000 4000 1600 400 280 200 120 80 40
    Table 2 (max): 20000 8000 4000 1600 400 280 200 120 80



*****************************************************************************************


    *** Tech Notes ***


    The following notes were mostly 're-translated'
    from a very poor translated instructions sheet:


    In order to change the DIP switches, always turn the power OFF/ON and reset memory.


    Note 1: Auto Hold type YES

      A) Hold Type: (DIP SW 3-8 OFF)

         The game selects and holds automatically favorable cards after DEAL button is pressed.
         Selects can be cancelled pressing the CANCEL button.

      B) Discard type: (DIP SW 3-8 ON)
         Starts game as Hold type, however Non-Held card(s) will be
         automatically recalled by pressing DEAL button. Also even one
         card is discarded, all HELD cards are disappeared and returns
         to normal game.

    Note 2: Always reset memory when JACKPOT points are changed.

    Note 3: Always reset memory when BONUS points are changed.

    Note 4: Always reset memory FOR PERCENTAGE changes.

    Note 5: COIN B is 5 times COIN A. KEY IN is 10 times coin A.

    Note 6: Each time a game is won, the woman will take one of her clothes off.
            When all the clothes are taken off, a bonus point ten times the BET number
            will be scored. This bonus cannot be scored if there is no woman's figure.

    Note 7: The double up 1 follows usual system. In 2, a card except spade 7
    (sic)   will be placed between the card to be turned up and the LAST 6 DATA.
            This card will change by pressing the DOUBLE UP SW. Anticipate the
            number of the next card which will turned up and stop. When the
            game is won by pressing BIG or SMALL, and the anticipated card and
            the turned up card happens to be the same number, then the scored
            points will be four times greater.


    * During FEVER, BET greater than the BET points achieved during FEVER cannot
      be scored.

    * The KEY OUT consists of two steps. During first OUT, points over 100 will
      become cleared and during the second OUT, points below 100 will become
      cleared.



*****************************************************************************************


    --------------------
    ***  Memory Map  ***
    --------------------

    0x0000 - 0xBFFF    ; ROM space.
    0xC000 - 0xCFFF    ; NVRAM (settings, meters, etc).
    0xD000 - 0xD7FF    ; Video RAM 1 (VRAM1-1).
    0xD800 - 0xDFFF    ; Color RAM 1 (VRAM1-2).
    0xE000 - 0xE7FF    ; Video RAM 2 (VRAM2-1).
    0xE800 - 0xEFFF    ; Color RAM 2 (VRAM2-2).
    0xF000 - 0xF003    ; PPI8255_0 --> Input Ports 0 & 1.
    0xF080 - 0xF083    ; PPI8255_2 --> DSW1, DSW2, DSW3.
    0xF0C0 - 0xF0C3    ; PPI8255_3 --> DSW4, LAMPS A & B.
    0xF100 - 0xF100    ; SN76489 #1.
    0xF200 - 0xF203    ; PPI8255_1 --> Input Ports 2 & 4.
    0xF300 - 0xF300    ; SN76489 #2.
    0xF400 - 0xF400    ; YM2149 control port 0.
    0xF500 - 0xF500    ; SN76489 #3.
    0xF600 - 0xF600    ; YM2149 R/W port 0 (Input Port 3).
    0xF700 - 0xF701    ; USART 8251 port.
    0xF800 - 0xF803    ; Co-processor SM7831.


    -- Z80 I/O ports --

    0x00  R   ; 09R81P /Busy.
    0x00  W   ; 09R81P ADPCM POS LSB.
    0x01  W   ; 09R81P ADPCM POS MSB.
    0x02  W   ; 09R81P unknown...
    0x03  W   ; 09R81P ADPCM END LSB.
    0x04  W   ; 09R81P ADPCM END MSB.
    0x05  W   ; 09R81P trigger.

    0xFF  R   ; unknown...
    0xFF  W   ; unknown...


*****************************************************************************************


    DRIVER UPDATES:


    [2008-10-07]

    - Improved the button-lamps layout to be more realistic.


    [2008-08-08]

    - Reverse engineering of custom IC's 06B49P, 06B53P & 09R81P.
    - Mapped the missing 3x SN76489.
    - Measured and traced all clocks on the board.
    - Measured and fixed the interrupt system.
    - Implemented timings/clocks from custom 06B49P.
    - Added sound support. All regular game sounds/musics are working.
    - Implemented the ADPCM samples system through 09R81P + M5205 emulation.
    - Added pinouts and technical notes about custom IC's 06B49P, 06B53P & 09R81P.
    - Added flip screen mode.
    - Inverted the order of double-up difficult DIP switches.
      (Seems to be the opposite of the indicated in the instruction sheet).
    - Changed 'Key In' to be active LOW instead of HIGH (checked in the PCB).
    - Complete memory map and ports scheme.
    - Created handlers for USART port and co-processor communication.
    - Renamed the sets accordingly.
    - Updated all notes.
    - Cleaned-up the driver.


    [2008-07-28]

    - Pre-defined CPU and SND clocks.
    - Switched the color system to RESNET calculations.
    - Completed the remaining DIP switches.
    - Added lamps support. Created a layout to show them.
    - Changes on the interrupt system (need to be verified on the PCB).
    - Renamed the graphics regions to more descriptive names.
    - Corrected the manufacturer's name.
    - Splitted the driver to driver + video.
    - Updated technical notes.


    [2008-07-03]

    - Initial release.
    - Set the proper screen size.
    - Decoded graphics.
    - Decoded the dual-state color circuitry.
    - Mapped the NVRAM, VRAM1-1, VRAM1-2, VRAM2-1 and VRAM2-2 properly.
    - Emulated 2x PPI 8255 devices.
    - Mapped the 4x DIP switches banks.
    - Added PORT_DIPLOCATION to all DIP switches.
    - Added DIP switches for 'Bet Max' and 'Limit'.
    - Added DIP switches for 'Jackpot' and 'Pay Table'.
    - Added the Memory Reset Switch.
    - Added the 2nd video & color RAM.
    - Added a 2nd tilemap for background graphics.
    - Simplified the graphics banks.
    - Fixed colors for foreground graphics.
    - Fixed visible area to show the top of background graphics.
    - Finally fixed colors for background graphics.
    - Added all coinage DIP switches.
    - Mapped all remaining inputs (service and player buttons).
    - Added pulse time limitation to coins A, B & C.
    - Switched to use 4x 8255 in replace of 2x 82c255 for I/O.
    - Created a handler to feed the z80 port0 requests.
    - Promoted lucky74s to 'working' state.
    - Added an alternate set, but the program ROM looks like incomplete,
      protected or just a bad dump.
    - Parent/clone relationship.
    - Added technical notes.

    From Dox:

    - Hooked interrupts.
    - Hooked the AY8910 and therefore the NMI trigger.
    - Changed the input "Key In" to active high.


    TODO:

    - USART comm.
    - Co-processor.


*****************************************************************************************/


#define MASTER_CLOCK		XTAL_12MHz		/* confirmed */

/* custom 06B49P clocks */
#define C_06B49P_CLKOUT_01	(MASTER_CLOCK/2)		/* 6 MHz. */
#define C_06B49P_CLKOUT_02	(MASTER_CLOCK/4)		/* 3 MHz. */
#define C_06B49P_CLKOUT_03	(MASTER_CLOCK/4)		/* 3 MHz. */
#define C_06B49P_CLKOUT_04	(MASTER_CLOCK/8)		/* 1.5 MHz. */
#define C_06B49P_CLKOUT_05	(MASTER_CLOCK/16)		/* 750 kHz. */
#define C_06B49P_CLKOUT_06	(MASTER_CLOCK/32)		/* 375 kHz. */
#define C_06B49P_CLKOUT_07	(MASTER_CLOCK/64)		/* 187.5 kHz. */
#define C_06B49P_CLKOUT_08	(MASTER_CLOCK/128)		/* 93.75 kHz. */
#define C_06B49P_CLKOUT_09	(MASTER_CLOCK/256)		/* 46875 Hz. */
#define C_06B49P_CLKOUT_10	(7782)					/* 7782 Hz. measured */
#define C_06B49P_CLKOUT_11	(3920)					/* 3920 Hz. measured */
#define C_06B49P_CLKOUT_12	(1960)					/* 1960 Hz. measured */
#define C_06B49P_CLKOUT_13	(950)					/* 950 Hz. measured */
#define C_06B49P_CLKOUT_14	(475)					/* 475 Hz. measured */
#define C_06B49P_CLKOUT_15	(237)					/* 237 Hz. measured */
#define C_06B49P_CLKOUT_16	(MASTER_CLOCK/100000)	/* 120 Hz. */
#define C_06B49P_CLKOUT_17	(MASTER_CLOCK/200000)	/* 60 Hz. */
#define C_06B49P_CLKOUT_18	(MASTER_CLOCK/256/3)	/* 15625 Hz. (H-Sync) */
#define C_06B49P_CLKOUT_19	(MASTER_CLOCK/200000)	/* 60 Hz. (V-Sync) */


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"
#include "sound/msm5205.h"
#include "machine/8255ppi.h"
#include "machine/nvram.h"
#include "lucky74.lh"
#include "includes/lucky74.h"


/*****************************
*    Read/Write  Handlers    *
*****************************/

READ8_MEMBER(lucky74_state::custom_09R81P_port_r)
{
	if (offset != 0x00)
	{
		return m_adpcm_reg[offset];
	}
	else
	{
		return m_adpcm_busy_line;
	}
}

WRITE8_MEMBER(lucky74_state::custom_09R81P_port_w)
{
	m_adpcm_reg[offset] = data;
}

static WRITE8_DEVICE_HANDLER( ym2149_portb_w )
{
	lucky74_state *state = device->machine().driver_data<lucky74_state>();
/*  when is in game mode writes 0x0a.
    when is in test mode writes 0x0e.
    after reset writes 0x16.

    bit 0 contains the screen orientation.
*/
	state->m_ym2149_portb = data;
	flip_screen_set(device->machine(), data & 0x01);
}

READ8_MEMBER(lucky74_state::usart_8251_r)
{
	/* reads to USART 8251 port */
	logerror("read from USART port.\n");
	return 0xff;
}

WRITE8_MEMBER(lucky74_state::usart_8251_w)
{
	/* writes to USART 8251 port */
	m_usart_8251 = data;
	logerror("write to USART port: %02x \n", m_usart_8251);
}

READ8_MEMBER(lucky74_state::copro_sm7831_r)
{
	/* read from SM7831 co-processor */
	logerror("read from co-processor.\n");
	return 0xff;
}

WRITE8_MEMBER(lucky74_state::copro_sm7831_w)
{
	/* write to SM7831 co-processor */
	m_copro_sm7831 = data;
	logerror("write to co-processor: %2X\n", m_copro_sm7831);
}


/**************
*    Lamps    *
**************/

static WRITE8_DEVICE_HANDLER(lamps_a_w)
{
/*  LAMPSA:

    7654 3210
    ---- --xx  D-UP + TAKE SCORE (need to be individualized)
    ---- xx--  BIG + SMALL (need to be individualized)
*/

	output_set_lamp_value(8, (data >> 0) & 1);		/* D-UP */
	output_set_lamp_value(9, (data >> 1) & 1);		/* TAKE SCORE */
	output_set_lamp_value(10, (data >> 2) & 1);		/* BIG */
	output_set_lamp_value(11, (data >> 3) & 1);		/* SMALL */
}

static WRITE8_DEVICE_HANDLER(lamps_b_w)
{
/*  LAMPSB:

    7654 3210
    ---- ---x  HOLD1
    ---- --x-  HOLD2
    ---- -x--  HOLD3
    ---- x---  HOLD4
    ---x ----  HOLD5
    -xx- ----  BET + START (need to be individualized)
    x--- ----  CANCEL (should lit start too?)
*/

	output_set_lamp_value(0, (data >> 0) & 1);						/* HOLD1 */
	output_set_lamp_value(1, (data >> 1) & 1);						/* HOLD2 */
	output_set_lamp_value(2, (data >> 2) & 1);						/* HOLD3 */
	output_set_lamp_value(3, (data >> 3) & 1);						/* HOLD4 */
	output_set_lamp_value(4, (data >> 4) & 1);						/* HOLD5 */
	output_set_lamp_value(5, (data >> 5) & 1);						/* BET */
	output_set_lamp_value(6, ((data >> 6) & 1)|((data >> 7) & 1));	/* START */
	output_set_lamp_value(7, (data >> 7) & 1);						/* CANCEL */
}


/************************
*    Interrupts Gen     *
************************/

static INTERRUPT_GEN( nmi_interrupt )
{
	lucky74_state *state = device->machine().driver_data<lucky74_state>();
	if ((state->m_ym2149_portb & 0x10) == 0)	/* ym2149 portB bit 4 trigger the NMI */
	{
		device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
	}
}


/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( lucky74_map, AS_PROGRAM, 8, lucky74_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE("nvram")	/* NVRAM */
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(lucky74_fg_videoram_w) AM_BASE(m_fg_videoram)				/* VRAM1-1 */
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(lucky74_fg_colorram_w) AM_BASE(m_fg_colorram)				/* VRAM1-2 */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(lucky74_bg_videoram_w) AM_BASE(m_bg_videoram)				/* VRAM2-1 */
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(lucky74_bg_colorram_w) AM_BASE(m_bg_colorram)				/* VRAM2-2 */
	AM_RANGE(0xf000, 0xf003) AM_DEVREADWRITE_LEGACY("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports 0 & 1 */
	AM_RANGE(0xf080, 0xf083) AM_DEVREADWRITE_LEGACY("ppi8255_2", ppi8255_r, ppi8255_w)	/* DSW 1, 2 & 3 */
	AM_RANGE(0xf0c0, 0xf0c3) AM_DEVREADWRITE_LEGACY("ppi8255_3", ppi8255_r, ppi8255_w)	/* DSW 4 */
	AM_RANGE(0xf100, 0xf100) AM_DEVWRITE_LEGACY("sn1", sn76496_w)							/* SN76489 #1 */
	AM_RANGE(0xf200, 0xf203) AM_DEVREADWRITE_LEGACY("ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports 2 & 4 */
	AM_RANGE(0xf300, 0xf300) AM_DEVWRITE_LEGACY("sn2", sn76496_w)							/* SN76489 #2 */
	AM_RANGE(0xf400, 0xf400) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)						/* YM2149 control */
	AM_RANGE(0xf500, 0xf500) AM_DEVWRITE_LEGACY("sn3", sn76496_w)							/* SN76489 #3 */
	AM_RANGE(0xf600, 0xf600) AM_DEVREADWRITE_LEGACY("aysnd", ay8910_r, ay8910_data_w)			/* YM2149 (Input Port 1) */
	AM_RANGE(0xf700, 0xf701) AM_READWRITE(usart_8251_r, usart_8251_w)						/* USART 8251 port */
	AM_RANGE(0xf800, 0xf803) AM_READWRITE(copro_sm7831_r, copro_sm7831_w)					/* SM7831 Co-Processor */
ADDRESS_MAP_END

static ADDRESS_MAP_START( lucky74_portmap, AS_IO, 8, lucky74_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x05) AM_READWRITE(custom_09R81P_port_r, custom_09R81P_port_w)	/* custom 09R81P (samples system) */
//  AM_RANGE(0xff, 0xff) AM_READWRITE_LEGACY(???)
ADDRESS_MAP_END

/* unknown I/O byte R/W

    0xFF    R W  ; unknown....

   -----------------

*** init log ***

cpu #0 (PC=00000105): unmapped I/O byte write to 000000FF = 04  ; unknown...
cpu #0 (PC=00000107): unmapped I/O byte read from 000000FF
cpu #0 (PC=00000111): unmapped I/O byte write to 000000FF = FB
cpu #0 (PC=00000113): unmapped I/O byte read from 000000FF

cpu #0 (PC=0000011E): byte write to 0000F400 = 07  ; YM2149 control
cpu #0 (PC=00000123): byte write to 0000F600 = 80  ; YM2149 data
cpu #0 (PC=00000128): byte write to 0000F400 = 0F  ; YM2149 control
cpu #0 (PC=0000012D): byte write to 0000F600 = 16  ; YM2149 data

cpu #0 (PC=00000132): byte write to 0000F003 = 92  ; PPI 8255_0 --> ports A & B as input.
cpu #0 (PC=00000137): byte write to 0000F203 = 99  ; PPI 8255_1 --> ports A & C as input.
cpu #0 (PC=0000013C): byte write to 0000F083 = 9B  ; PPI 8255_2 --> ports A, B and half of C as input.
cpu #0 (PC=00000141): byte write to 0000F0C3 = 90  ; PPI 8255_3 --> port A as input.

cpu #0 (PC=0000014B): byte write to 0000F100 = 9F  ; SN76489 #1 init...
cpu #0 (PC=0000014F): byte write to 0000F100 = BF
cpu #0 (PC=00000153): byte write to 0000F100 = DF
cpu #0 (PC=00000157): byte write to 0000F100 = FF
cpu #0 (PC=0000015B): byte write to 0000F300 = 9F  ; SN76489 #2 init...
cpu #0 (PC=0000015F): byte write to 0000F300 = BF
cpu #0 (PC=00000163): byte write to 0000F300 = DF
cpu #0 (PC=00000167): byte write to 0000F300 = FF
cpu #0 (PC=0000016B): byte write to 0000F500 = 9F  ; SN76489 #3 init...
cpu #0 (PC=0000016F): byte write to 0000F500 = BF
cpu #0 (PC=00000173): byte write to 0000F500 = DF
cpu #0 (PC=00000177): byte write to 0000F500 = FF

cpu #0 (PC=0000017C): unmapped program memory byte write to 0000F800 = 00  ; Co-processor SM7831.
cpu #0 (PC=00000181): unmapped program memory byte write to 0000F802 = 80
cpu #0 (PC=00000186): unmapped program memory byte write to 0000F803 = C3
cpu #0 (PC=0000018B): unmapped program memory byte write to 0000F803 = 3C

cpu #0 (PC=0000018F): unmapped program memory byte write to 0000F701 = 00  ; USART 8251.
cpu #0 (PC=00000192): unmapped program memory byte write to 0000F701 = 00
cpu #0 (PC=00000195): unmapped program memory byte write to 0000F701 = 00
cpu #0 (PC=0000019A): unmapped program memory byte write to 0000F701 = 40
cpu #0 (PC=0000019F): unmapped program memory byte write to 0000F701 = 4F

cpu #0 (PC=000002A6): unmapped program memory byte write to 0000F803 = 55  ; Co-processor SM7831 (testing bits).
cpu #0 (PC=000002AB): unmapped program memory byte write to 0000F803 = AA
cpu #0 (PC=000002B0): unmapped program memory byte write to 0000F803 = 99
cpu #0 (PC=000002B5): unmapped program memory byte write to 0000F803 = 66
cpu #0 (PC=000002BA): unmapped program memory byte write to 0000F801 = 22

*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( lucky74 )

/*  Player buttons are the same for players 1 & 2.
    Test mode shows them as dupes. Maybe are multiplexed?
*/
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	/* 'A' in test mode */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	/* 'B' in test mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	/* 'C' in test mode */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	/* 'D' in test mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	/* 'E' in test mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )	PORT_NAME("Small")	/* 'F' in test mode */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Flip SC Off") PORT_CODE(KEYCODE_O)  /* 'G' in test mode (normal screen) */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )		PORT_NAME("Input H") PORT_CODE(KEYCODE_K)  /* 'H' in test mode */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )		/* 'I' in test mode */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )		PORT_NAME("Start")  /* 'J' in test mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	/* 'K' in test mode */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )		/* 'L' in test mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )		/* 'M' & 'Q' in test mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")	/* 'N' & 'P' in test mode */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Flip SC On")  PORT_CODE(KEYCODE_I)  /* 'O' in test mode (inverted screen) */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )		/* not in test mode */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* YM2149, port A */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )    PORT_IMPULSE(2)	/* Coin A */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )    PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )    PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )  PORT_NAME("Service")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Memory Reset Switch") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Auto Hold" )				PORT_DIPLOCATION("DSW1:1")	/* see note 1 */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Jackpot" )				PORT_DIPLOCATION("DSW1:2")	/* see note 2 */
	PORT_DIPSETTING(    0x02, "Bet x 100" )
	PORT_DIPSETTING(    0x00, "Bet x 150" )
	PORT_DIPNAME( 0x04, 0x04, "Ceiling Bonus Point" )	PORT_DIPLOCATION("DSW1:3")	/* see note 3 */
	PORT_DIPSETTING(    0x04, "Bet x 40"  )
	PORT_DIPSETTING(    0x00, "Bet x 50"  )
	PORT_DIPNAME( 0x78, 0x40, "Percentage" )			PORT_DIPLOCATION("DSW1:4,5,6,7")	/* see note 4 */
	PORT_DIPSETTING(    0x00, "90%" )	/* 110% in the instruction sheet */
	PORT_DIPSETTING(    0x08, "87%" )	/* 106% in the instruction sheet */
	PORT_DIPSETTING(    0x10, "84%" )	/* 102% in the instruction sheet */
	PORT_DIPSETTING(    0x18, "81%" )	/* 98% in the instruction sheet */
	PORT_DIPSETTING(    0x20, "78%" )	/* 94% in the instruction sheet */
	PORT_DIPSETTING(    0x28, "75%" )	/* 90% in the instruction sheet */
	PORT_DIPSETTING(    0x30, "72%" )	/* 86% in the instruction sheet */
	PORT_DIPSETTING(    0x38, "69%" )	/* 82% in the instruction sheet */
	PORT_DIPSETTING(    0x40, "66%" )	/* 78% in the instruction sheet */
	PORT_DIPSETTING(    0x48, "63%" )	/* 74% in the instruction sheet */
	PORT_DIPSETTING(    0x50, "60%" )	/* 70% in the instruction sheet */
	PORT_DIPSETTING(    0x58, "57%" )	/* 66% in the instruction sheet */
	PORT_DIPSETTING(    0x60, "54%" )	/* 62% in the instruction sheet */
	PORT_DIPSETTING(    0x68, "51%" )	/* 58% in the instruction sheet */
	PORT_DIPSETTING(    0x70, "48%" )	/* 54% in the instruction sheet */
	PORT_DIPSETTING(    0x78, "45%" )	/* 50% in the instruction sheet */
	PORT_DIPNAME( 0x80, 0x80, "Panties" )				PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x00, "Without" )
	PORT_DIPSETTING(    0x80, "With" )

	PORT_START("DSW2")
    /* DIPs 1-4 handle the harcoded coinage for Coin A, B and Remote credits (B = A x 5; R = A x 10) */
	PORT_DIPNAME( 0x0f, 0x0f, "Coinage A, B & Remote" )	PORT_DIPLOCATION("DSW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "A: 20 Coins/1 Credit; B: 4 Coins/1 Credit;   R: 2 Pulses/1 Credit   " )
	PORT_DIPSETTING(    0x01, "A: 15 Coins/1 Credit; B: 3 Coins/1 Credit;   R: 15 Pulses/10 Credits" )
	PORT_DIPSETTING(    0x02, "A: 10 Coins/1 Credit; B: 2 Coins/1 Credit;   R: 1 Pulse/1 Credit    " )
	PORT_DIPSETTING(    0x03, "A: 4 Coins/1 Credit;  B: 5 Coins/5 Credits;  R: 4 Pulses/10 Credits " )
	PORT_DIPSETTING(    0x04, "A: 3 Coins/2 Credits; B: 3 Coins/10 Credits; R: 3 Pulses/20 Credits " )
	PORT_DIPSETTING(    0x05, "A: 3 Coins/1 Credit;  B: 3 Coins/5 Credits;  R: 3 Pulses/10 Credits " )
	PORT_DIPSETTING(    0x06, "A: 2 Coins/1 Credit;  B: 2 Coins/5 Credits;  R: 1 Pulse/5 Credits   " )
	PORT_DIPSETTING(    0x07, "A: 5 Coins/1 Credit;  B: 1 Coin/1 Credit;    R: 1 Pulse/2 Credits   " )
	PORT_DIPSETTING(    0x08, "A: 5 Coins/2 Credits; B: 1 Coin/2 Credits;   R: 1 Pulse/4 Credits   " )
	PORT_DIPSETTING(    0x09, "A: 5 Coins/3 Credits; B: 1 Coin/3 Credits;   R: 1 Pulse/6 Credits   " )
	PORT_DIPSETTING(    0x0a, "A: 5 Coins/4 Credits; B: 1 Coin/4 Credits;   R: 1 Pulse/8 Credits   " )
	PORT_DIPSETTING(    0x0b, "A: 1 Coin/1 Credit;   B: 1 Coin/5 Credits;   R: 1 Pulse/10 Credits  " )
	PORT_DIPSETTING(    0x0c, "A: 5 Coins/6 Credits; B: 1 Coin/6 Credits;   R: 1 Pulse/12 Credits  " )
	PORT_DIPSETTING(    0x0d, "A: 1 Coin/2 Credits;  B: 1 Coin/10 Credits;  R: 1 Pulse/20 Credits  " )
	PORT_DIPSETTING(    0x0e, "A: 1 Coin/5 Credits;  B: 1 Coin/25 Credits;  R: 1 Pulse/50 Credits  " )
	PORT_DIPSETTING(    0x0f, "A: 1 Coin/10 Credits; B: 1 Coin/50 Credits;  R: 1 Pulse/100 Credits " )
    /* DIPs 5-8 handle the Coin C coinage */
	PORT_DIPNAME( 0xf0, 0xf0, "Coinage C" )				PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, "5 Coins/2 Credits" )		/* 2.5 coins per credit */
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xb0, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0xc0, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0xd0, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0xe0, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0xf0, "1 Coin/50 Credits" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Bet Max" )						PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x06, 0x06, "Minimum Bet" )					PORT_DIPLOCATION("DSW3:2,3")	/* Bet Min */
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x02, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x18, "Limit" )							PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(    0x18, "No Limit" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPSETTING(    0x08, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x20, 0x20, "Woman's figure in Main Game" )	PORT_DIPLOCATION("DSW3:6")	/* see note 6 */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Type of Poker" )					PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, "A - Without Wild Card" )	/* see the game notes */
	PORT_DIPSETTING(    0x00, "B - Joker Wild Poker" )	/* see the game notes */
	PORT_DIPNAME( 0x80, 0x80, "Kinds of Poker" )				PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, "A - Hold" )
	PORT_DIPSETTING(    0x00, "B - Discard" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Hopper Coin SW" )				PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x02, 0x02, "Coin Payment" )					PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPSETTING(    0x02, "Auto by PAYOUT SW" )
	PORT_DIPNAME( 0x04, 0x00, "Hopper Capacity" )				PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, "700" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x08, "Woman's figure in D-UP game" )	PORT_DIPLOCATION("DSW4:4")	/* doesn't seems to work */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Double-Up game" )				PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Stop by 6th Double-Up" )			PORT_DIPLOCATION("DSW4:6")	/* see note 7 */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xC0, 0xC0, "Double-Up difficulty" )			PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )	/* easy      (from instruction sheet) */
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )		/* ....      (from instruction sheet) */
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )	/* ....      (from instruction sheet) */
	PORT_DIPSETTING(    0xC0, DEF_STR( Easy ) )		/* difficult (from instruction sheet) */
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,4),	/* 4096 tiles */
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },	/* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( lucky74 )
	GFXDECODE_ENTRY( "fgtiles", 0, tilelayout, 0, 16 )		/* text, frames & cards */
	GFXDECODE_ENTRY( "bgtiles", 0, tilelayout, 256, 16 )	/* title & whores */
GFXDECODE_END


/********************************************
*    ADPCM sound system (09R81P + M5205)    *
********************************************/

static SOUND_START( lucky74 )
{
	lucky74_state *state = machine.driver_data<lucky74_state>();
    /* cleaning all 09R81P registers */

	UINT8 i;

	for (i = 0; i < 6; i++)
	{
		state->m_adpcm_reg[i] = 0;
	}

	state->m_adpcm_busy_line = 0x01;	/* free and ready */
}

static void lucky74_adpcm_int(device_t *device)
{
	lucky74_state *state = device->machine().driver_data<lucky74_state>();
	if (state->m_adpcm_reg[05] == 0x01)	/* register 0x05 (bit 0 activated), trigger the sample */
	{
		/* conditional zone for samples reproduction */

		if (state->m_adpcm_busy_line)     /* still not started */
		{
			/* init all 09R81P registers */
			logerror("init ADPCM registers\n");
			state->m_adpcm_end = (state->m_adpcm_reg[04] << 8) + state->m_adpcm_reg[03];
			state->m_adpcm_pos = (state->m_adpcm_reg[01] << 8) + state->m_adpcm_reg[00];
			state->m_adpcm_busy_line = 0;
			state->m_adpcm_data = -1;

			logerror("sample pos:%4X\n", state->m_adpcm_pos);
			logerror("sample end:%4X\n", state->m_adpcm_end);
		}

		if (state->m_adpcm_data == -1)
		{
			/* transferring 1st nibble */
			state->m_adpcm_data = device->machine().region("adpcm")->base()[state->m_adpcm_pos];
			state->m_adpcm_pos = (state->m_adpcm_pos + 1) & 0xffff;
			msm5205_data_w(device, state->m_adpcm_data >> 4);

			if (state->m_adpcm_pos == state->m_adpcm_end)
			{
				msm5205_reset_w(device, 0);			/* reset the M5205 */
				state->m_adpcm_reg[05] = 0;		/* clean trigger register */
				state->m_adpcm_busy_line = 0x01;	/* deactivate busy flag */
				logerror("end of sample.\n");
			}
		}
		else
		{
			/* transferring 2nd nibble */
			msm5205_data_w(device, state->m_adpcm_data & 0x0f);
			state->m_adpcm_data = -1;
		}
	}

	return;
}


/*************************************
*     PPI 82C255 (x2) Interfaces     *
*************************************/

/* Each 82C255 behaves like 2x 8255 (in mode 0).
   Since MAME doesn't support it yet, I replaced both 82C255
   with 4x 8255...
*/
static const ppi8255_interface ppi8255_intf[4] =
{
	{	/* A & B set as input */
		DEVCB_INPUT_PORT("IN0"),	/* Port A read, IN0 */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read, IN1 */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C writes: 0x00 after reset, 0xff during game, and 0xfd when tap F2 for percentage and run count */
	},
	{	/* A & C set as input */
		DEVCB_INPUT_PORT("IN2"),	/* Port A read, IN2 */
		DEVCB_NULL,					/* Port B read */
		DEVCB_INPUT_PORT("IN4"),	/* Port C read, IN4 */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("DSW1"),	/* Port A read, DSW1 */
		DEVCB_INPUT_PORT("DSW2"),	/* Port B read, DSW2 */
		DEVCB_INPUT_PORT("DSW3"),	/* Port C read, DSW3 */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVCB_INPUT_PORT("DSW4"),	/* Port A read, DSW4 */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_HANDLER(lamps_a_w),	/* Port B write, LAMPSA */
		DEVCB_HANDLER(lamps_b_w)	/* Port C write, LAMPSB */
	}
};


/*****************************
*      Sound Interfaces      *
*****************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN3"),
	DEVCB_NULL,	/* a sort of status byte */
	DEVCB_NULL,
	DEVCB_HANDLER(ym2149_portb_w)
};

static const msm5205_interface msm5205_config =
{
	lucky74_adpcm_int,	/* interrupt function */
	MSM5205_S48_4B		/* 8KHz */
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( lucky74, lucky74_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, C_06B49P_CLKOUT_03)	/* 3 MHz. */
	MCFG_CPU_PROGRAM_MAP(lucky74_map)
	MCFG_CPU_IO_MAP(lucky74_portmap)
	MCFG_CPU_VBLANK_INT("screen", nmi_interrupt)	/* 60 Hz. measured */

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SOUND_START(lucky74)

	/* 2x 82c255 (4x 8255) */
	MCFG_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MCFG_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )
	MCFG_PPI8255_ADD( "ppi8255_2", ppi8255_intf[2] )
	MCFG_PPI8255_ADD( "ppi8255_3", ppi8255_intf[3] )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(lucky74)

	MCFG_GFXDECODE(lucky74)

	MCFG_PALETTE_INIT(lucky74)
	MCFG_PALETTE_LENGTH(512)

	MCFG_VIDEO_START(lucky74)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489, C_06B49P_CLKOUT_03)	/* 3 MHz. */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("sn2", SN76489, C_06B49P_CLKOUT_03)	/* 3 MHz. */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("sn3", SN76489, C_06B49P_CLKOUT_03)	/* 3 MHz. */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("aysnd", AY8910, C_06B49P_CLKOUT_04)	/* 1.5 MHz. */
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.00)			/* not routed to audio hardware */

	MCFG_SOUND_ADD("msm", MSM5205, C_06B49P_CLKOUT_06)	/* 375 kHz. */
	MCFG_SOUND_CONFIG(msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

/*
    Bootleg, set 1.

    - The black CPU box was replaced with a mini daughterboard
      with a real Z80, the program ROM, and NVRAM.

    - The checksum routines were patched.

    - All the co-processor routines are there, but the calls were NOPed.

*/
ROM_START( lucky74 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luckychi.00",	0x0000, 0x10000, CRC(3b906f0e) SHA1(1f9abd168c60b0d22fa6c7391bfdf5f3aabd66ef) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "luckychi.12",	0x00000, 0x8000, CRC(ff934c20) SHA1(07cd2225dfc0e5b74be2e1b379c6b180e37660db) )
	ROM_LOAD( "luckychi.11",	0x08000, 0x8000, CRC(2fd6fb8a) SHA1(1a910e0a2e6db22a8d9a65d7b932f9ca39601e9c) )
	ROM_LOAD( "luckychi.13",	0x10000, 0x8000, CRC(c70a6da3) SHA1(195772ef649e21a5c54c5871e7b858967b6ebee8) )
	ROM_LOAD( "luckychi.14",	0x18000, 0x8000, CRC(b5813b67) SHA1(cce38e33a5218d6839d956174807d88e7c070d5a) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "luckychi.17",	0x00000, 0x8000, CRC(010ffa4a) SHA1(8856d61b71e951509073bc359851f47c39c4274d) )
	ROM_LOAD( "luckychi.16",	0x08000, 0x8000, CRC(15104810) SHA1(586df734740209e2a05932e31d2a301d330e8cbd) )
	ROM_LOAD( "luckychi.18",	0x10000, 0x8000, CRC(f2d45e76) SHA1(46df7bf98434c836fd38539575a35bf67c9ec2c6) )
	ROM_LOAD( "luckychi.19",	0x18000, 0x8000, CRC(6b0196f3) SHA1(277049279dcfcf07189dbdb20935c2a71b2f6061) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* 4-bit ADPCM samples @ 8kHz */
	ROM_LOAD( "luckyson.15",	0x00000, 0x10000, CRC(b896c87f) SHA1(985e625a937abd6353218f0cace14d3adec4c1bf) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "luckyprom.e6",	0x0000, 0x0100, CRC(ae793fef) SHA1(e4e2d2dccabad7d756811fb2d5e123bf30f106f3) )
	ROM_LOAD( "luckyprom.e7",	0x0100, 0x0100, CRC(7c772d0c) SHA1(9c99daa01ca56c7ebd48945505fcbae184998b13) )
	ROM_LOAD( "luckyprom.d6",	0x0200, 0x0100, CRC(61716584) SHA1(7a3e17f47ce173d79c12b2394edb8f32b7509e39) )
	ROM_LOAD( "luckyprom.d7",	0x0300, 0x0100, CRC(4003bc8f) SHA1(f830203c22a4f94b8b9f0b24e287204a742a8322) )
	ROM_LOAD( "luckyprom.c6",	0x0400, 0x0100, CRC(a8d2b3db) SHA1(7b346797bedc627fb2d49f19b18860a81c69e122) )
	ROM_LOAD( "luckyprom.c7",	0x0500, 0x0100, CRC(e62fd192) SHA1(86a189df2e2ccef6bd2a4e6d969e777fbba8cdf7) )
ROM_END

/*
    Bootleg, set 2.

    - All the co-processor routines were erased.

    - The program ROM seems incomplete or encrypted in some smart way.
      At start, just pop some registers and make a RTI. Maybe protection...

*/
ROM_START( lucky74a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "luckygde.00",	0x0000, 0x10000, CRC(e3f7db99) SHA1(5c7d9d3fed9eb19d3d666c8c08b34968a9996a96) )	/* bad dump? */

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "luckygde.12",	0x00000, 0x8000, CRC(7127465b) SHA1(3f72f91652fcab52c073744b1651fdfe772c584a) )
	ROM_LOAD( "luckygde.11",	0x08000, 0x8000, CRC(8a5ea91a) SHA1(8d22615c00ff7c8a27cd721618b5d32a8d089c95) )
	ROM_LOAD( "luckygde.13",	0x10000, 0x8000, CRC(bbb63ac1) SHA1(ab986055e34d90e81caf20c28c5ad89715209d0e) )
	ROM_LOAD( "luckygde.14",	0x18000, 0x8000, CRC(dcffdf07) SHA1(d63fd7d23e488650d3731830f07bce0ce64424b8) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "luckygde.17",	0x00000, 0x8000, CRC(18da3468) SHA1(6dc60da939bfa7528e1fe75a85328a32047c8990) )
	ROM_LOAD( "luckygde.16",	0x08000, 0x8000, CRC(0e831be5) SHA1(302a68203f565718f7f537dab50fb52250c48859) )
	ROM_LOAD( "luckygde.18",	0x10000, 0x8000, CRC(717e5f4e) SHA1(0f14c9525bf77bbc4de0d9695648acb40870a176) )
	ROM_LOAD( "luckygde.19",	0x18000, 0x8000, CRC(bb4608ae) SHA1(cc8ec596f445fe0364f254241227de368f309ebb) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* 4-bit ADPCM samples @ 8kHz */
	ROM_LOAD( "luckyson.15",	0x00000, 0x10000, CRC(b896c87f) SHA1(985e625a937abd6353218f0cace14d3adec4c1bf) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "luckyprom.e6",	0x0000, 0x0100, CRC(ae793fef) SHA1(e4e2d2dccabad7d756811fb2d5e123bf30f106f3) )
	ROM_LOAD( "luckyprom.e7",	0x0100, 0x0100, CRC(7c772d0c) SHA1(9c99daa01ca56c7ebd48945505fcbae184998b13) )
	ROM_LOAD( "luckyprom.d6",	0x0200, 0x0100, CRC(61716584) SHA1(7a3e17f47ce173d79c12b2394edb8f32b7509e39) )
	ROM_LOAD( "luckyprom.d7",	0x0300, 0x0100, CRC(4003bc8f) SHA1(f830203c22a4f94b8b9f0b24e287204a742a8322) )
	ROM_LOAD( "luckyprom.c6",	0x0400, 0x0100, CRC(a8d2b3db) SHA1(7b346797bedc627fb2d49f19b18860a81c69e122) )
	ROM_LOAD( "luckyprom.c7",	0x0500, 0x0100, CRC(e62fd192) SHA1(86a189df2e2ccef6bd2a4e6d969e777fbba8cdf7) )
ROM_END


/*********************************************
*                Game Drivers                *
**********************************************

       YEAR  NAME      PARENT   MACHINE  INPUT     INIT  ROT    COMPANY          FULLNAME                     FLAGS             LAYOUT  */
GAMEL( 1988, lucky74,  0,       lucky74, lucky74,  0,    ROT0, "Wing Co., Ltd.", "Lucky 74 (bootleg, set 1)", 0,                layout_lucky74 )
GAMEL( 1988, lucky74a, lucky74, lucky74, lucky74,  0,    ROT0, "Wing Co., Ltd.", "Lucky 74 (bootleg, set 2)", GAME_NOT_WORKING, layout_lucky74 )
