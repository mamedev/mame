/*

  Sega Naomi / Naomi 2 / Atomiswave

  Driver by Samuele Zannoli, R. Belmont, ElSemi,
            David Haywood, Angelo Salese and Olivier Galibert

 Notes:
  Several early Naomi games are running on an earlier revision mainboard (HOTD2 etc.) which appears to have an earlier
   revision of the graphic chip.  Attempting to run these games on the later board results in graphical glitches and/or
   other problems.

  Naomi 2 is backwards compatible with Naomi 1

  The later revision games (released after GD-ROM had been discontinued) require the 'h' revision bios, which appears to
  have additional hardware checks.

Sega Naomi is Dreamcast based Arcade hardware.

Current Compatibility notes (GD-rom games only)
|----------|-------|-----|------|------|------|--------------------------------|-----------------------------------------------------|
|romname   | Pl(s) | Rot | Test | Boot | Game | Control Type                   | Notes                                               |
|----------|-------|-----|------|------|------|--------------------------------|-----------------------------------------------------|
|gundmgd   |   1   |  H  | Yes  | Yes  | Yes  | joystick with 4 buttons        |                                                     |
|sfz3ugd   |   2   |  H  | Yes  | Yes  | No   | joystick with 6 buttons        | Hangs after pressing start                          |
|cvsgd     |   2   |  H  | Yes  | Yes  | Yes  | joystick with 4 buttons        | 2 + 2 extra buttons                                 |
|gundmxgd  |   1   |  H  | Yes  | Yes  | Yes  | joystick with 4 buttons        |                                                     |
|cvs2gd    |   2   |  H  | Yes  | Yes  | Yes  | joystick with 6 buttons        |                                                     |
|ikaruga   |   2   |  V  | Yes  | Yes  | Yes  | joystick with 2 buttons        |                                                     |
|ggxx      |   2   |  H  | Yes  | Yes  | Yes  | joystick with 5 buttons        |                                                     |
|moeru     |   2   |  H  | No   | No   | No   | joystick with 3 buttons        | Broken i/o, stuck at the "now loading" msg          |
|chocomk   |   2   |  H  | Yes  | Yes  | Yes  | joystick with 2 buttons        |                                                     |
|quizqgd   |   2   |  V  | Yes  | Yes  | Yes  | joystick with 4 buttons        | Can play without joystick                           |
|ggxxrl    |   2   |  H  | Yes  | Yes  | Yes  | joystick with 5 buttons        |                                                     |
|tetkiwam  |   2   |  H  | No   | No   | No   | joystick with 3 buttons        | Black screen, tests the SCIF (UART) regs?           |
|shikgam2  |   2   |  V  | Yes  | Yes  | No   | joystick with 3 buttons        | Broken i/o, crashes on the proper gameplay          |
|usagui    |   1   |  H  | Yes  | Yes  | No   | mahjong panel                  |                                                     |
|bdrdown   |   2   |  H  | Yes  | Yes  | Yes  | joystick with 3 buttons        |                                                     |
|psyvar2   |   1   |  V  | Yes  | Yes  | No   | joystick with 2 buttons        | Broken i/o? Hangs after pressing start              |
|cfield    |   1   |  H  | Yes  | Yes  | Yes  | joystick with 3 buttons        |                                                     |
|trizeal   |   2   |  V  | No   | No   | No   | joystick with 3 buttons        |                                                     |
|meltybld  |   2   |  H  | Yes  | Yes  | Yes  | joystick with 4 buttons        | Crashes in attract mode                             |
|senko     |   2   |  H  | Yes  | Yes  | No   | joystick with 3 buttons        | Senko no Londe SP (undumped) uses 5 buttons         |
|senkoo    |   2   |  H  | Yes  | Yes  | No   | joystick with 3 buttons        |                                                     |
|ss2005    |   1   |  H  | Yes  | Yes  | No   | joystick with 3 buttons        | Calls a YUV FMV, crashes due of that                |
|radirgy   |   1   |  V  | Yes  | Yes  | Yes  | joystick with 3 buttons        |                                                     |
|ggxxsla   |   2   |  H  | Yes  | Yes  | Yes  | joystick with 5 buttons        |                                                     |
|kurucham  |   2   |  H  | Yes  | Yes  | No   | joystick with 3 buttons        | Broken i/o, crashes in attract with wrong mask      |
|undefeat  |   2   |  V  | No   | No   | No   | joystick with 2 buttons        | Stuck at the "now loading" msg                      |
|trgheart  |   1   |  V  | Yes  | Yes  | No   | joystick with 3 buttons        | Broken i/o                                          |
|jingystm  |   2   |  H  | Yes  | Yes  | No   | joystick with 3 buttons        | Broken i/o                                          |
|meltyb    |   2   |  H  | Yes  | Yes  | Yes  | joystick with 5 buttons        | Crashes in attract mode                             |
|meltyba   |   2   |  H  | Yes  | Yes  | Yes  | joystick with 5 buttons        | Crashes in attract mode                             |
|karous    |   1   |  V  | Yes  | Yes  | No   | joystick with 3 buttons        | Broken i/o                                          |
|takoron   |   2   |  H  | Yes  | Yes  | Yes  | joystick with 3 buttons        |                                                     |
--------------------------------------------------------------------------------------------------------------------------------------
|confmiss  |   2   |  H  | Yes  | Yes  | No   | lightgun                       |                                                     |
|sprtjam   |   2   |  H  | Yes  | Yes  | Yes  | joystick with 2 buttons        |                                                     |
|slashout  |   1   |  H  | Yes  | Yes  | Yes  | joystick with 4 buttons        |                                                     |
|spkrbtl   |   2   |  H  | Yes  | Yes  | No   | joystick with 4 buttons        | Crashes when loading the gameplay                   |
|monkeyba  |   1   |  H  | Yes  | Yes  | No   | ad stick                       | Crashes when loading the gameplay                   |
|dygolf    |   2   |  H  | No   | No   | No   | trackball + 4 buttons          | Moans about the jvs settings                        |
|wsbbgd    |   2   |  H  | No   | No   | No   | ad stick + 2 buttons           | Moans about the jvs settings                        |
|vtennisg  |   2   |  H  | Yes  | Yes  | No   | joystick with 2 buttons        | Broken gfxs, crashes after a while                  |
|vathlete  |   2   |  H  | Yes  | Yes  | No   | joystick with 3 buttons        | Broken i/o                                          |
|vtennis2  |   2   |  H  | No   | No   | No   | joystick with 3 buttons        |                                                     |
|keyboard  |   2   |  H  | Yes  | Yes  | No   | keyboard                       | Broken i/o, crashes after a while                   |
|lupinsho  |   2   |  H  | Yes  | Yes  | No   | lightgun                       | Broken i/o, crashes after a while                   |
|luptype   |   2   |  H  | Yes  | Yes  | No   | keyboard                       | Broken i/o, ARM cpu crashes when inserting a coin   |
|mok       |   2   |  H  | Yes  | Yes  | No   | lightgun                       | Broken i/o                                          |
|ngdup23a  |   2   |  H  | Yes  | Yes  | Yes  | n/a                            | Missing DIMM emulation                              |
|ngdup23c  |   2   |  H  | Yes  | Yes  | Yes  | n/a                            | Missing DIMM emulation                              |
|puyofev   |   2   |  H  | Yes  | Yes  | No   | joystick with 2 buttons        | Broken i/o? Hangs after pressing start              |
--------------------------------------------------------------------------------------------------------------------------------------

Test: If it enters into test mode (not actually working test mode, i.e. if inputs doesn't work)
Boot: If game boots and draws some gfxs
Game: If game can be coined up and can actually be played without problems.
If the three aforementioned are all yes and there aren't any notes, game is fully working.


---------------------------------------------------------------------------------------------------------------------------------------------------

Guru's Readme
-------------

Sega NAOMI Mainboard
Sega, 1998-2005

PCB Layout
----------
837-13544-01
171-7772F
837-13707 (sticker)
(C) SEGA 1999
|---------------------------------------------------|
|    CN1                           CN3              |
|PC910  62256   EPF8452AQC160-3                     |
|    BATTERY EPC1064   JP4  3771  93C46             |
|A179B    315-6188.IC31     3771                3773|
|ADM485  BIOS.IC27   5264165            5264165     |
|                    5264165  |-----|   5264165     |
|    CN3                      |POWER|               |
|                             |VR2  | 33.3333MHZ    |
|CN26                         |-----|          27MHZ|
|                                         CY2308SC-3|
|        KM416S4030      |------|     HY57V161610   |
|                        | SH4  |     HY57V161610   |
| C844G        315-6232  |      |             32MHZ |
|            33.8688MHZ  |------|     HY57V161610   |
|                         xMHz        HY57V161610   |
|      PCM1725    JP1                    62256      |
|                     HY57V161610                   |
|                          HY57V161610              |
|              315-6145                             |
|CN25                CY2308SC-1          315-6146   |
|          LED1                              93C46  |
|          LED2                     14.7456MHZ      |
|---------------------------------------------------|
Notes:
      CN1/2/3         - Connectors for ROM cart
      CN25/26         - Connectors for filter board
      EPF8452AQC160-3 - Altera FLEX EPF8452AQC160-3 FPGA (QFP160)
      315-6188.IC31   - Altera EPC1064 (DIP8)
                        According to the datasheet, it's an FPGA Configuration
                        Device which loads the Altera Flex EPF8452 with some info
                        on power-up.
      JP1             - set to 2-3. Alt setting is 1-2
      JP4             - set to 2-3. Alt setting is 1-2
      93C46           - 128 bytes serial EEPROM
      A179B 96K       - TI SN75179B Differential driver and receiver pair (like RS485)
      ADM485          - Analog Devices ADM485
      BIOS.IC27       - 27C160 EPROM
      5264165         - Hitachi 5264165FTTA60 (video RAM)
      HY57V161610     - Hyundai 57V161610DTC-8 (main program RAM)
      CY2308SC-3      - Clock generator IC
      KM416S4030      - Samsung KM416S4030 16MBit SDRAM (sound related RAM?)
      315-6232        - Sega Custom IC (QFP100)
      315-6145        - Sega Custom IC (QFP56)
      315-6146        - Sega Custom IC (QFP176)
      C844G           - ? (SOIC14)
      62256           - 32kx8 SRAM
      PCM1725         - Burr-Brown PCM1725
      xMHz            - Small round XTAL (possibly 32.768kHz for a clock?)
      SH4             - Hitachi SH4 CPU (BGAxxx, with heatsink and fan)
      POWERVR2        - POWERVR2 video generator (BGAxxx, with heatsink)


Filter Board
------------
839-1069
|----------------------------------------------------|
|SW2 SW1   DIPSW   CN5                      CN12 CN10|
|                                                    |
|                                                    |
|           DIN1                     DIN2            |
|                                                    |
|               CNTX  CNRX                           |
| CN9    CN8                                         |
|    CN7     CN6           CN4       CN3    CN2   CN1|
|----------------------------------------------------|
Notes:
      CN1/CN2   - Power input
      CN3       - HD15 (i.e. VGA connector) RGB Video Output @ 15kHz or 31.5kHz
      CN4       - RCA Audio Output connectors
      CN5       - USB connector (connection to I/O board)
      CN6       - 10 pin connector labelled 'MAPLE 0-1'
      CN7       - 11 pin connector labelled 'MAPLE 2-3'
      CN8       - RS422 connector
      CN9       - Midi connector
      CNTX/CNRX - Network connectors
      DIN1/DIN2 - Connectors joining to mainboard CN25/26
      SW1       - Test Switch
      SW2       - Service Switch
      DIPSW     - 4-position DIP switch block
      CN10      - 12 volt output for internal case exhaust fan
      CN11      - RGB connector (not populated)
      CN12      - 5 volt output connector



---------------------------------------------------------
Bios Version Information                                |
---------------------------------------------------------
    Bios                     |   Support | Support      |
    Label                    |   GD-ROM  | Cabinet Link |
---------------------------------------------------------
Naomi / GD-ROM               |           |              |
    EPR-21576D (and earlier) |   No      |    No        |
    EPR-21576E               |   Yes     |    No        |
    EPR-21576F               |   Yes     |    Yes       |
    EPR-21576G (and newer)   |   Yes     |    Yes       |
---------------------------------------------------------
Naomi 2 / GD-ROM             |           |              |
    EPR-23605                |   Yes     |    No        |
    EPR-23605A               |   Yes     |    Yes       |
    EPR-23605B (and newer)   |   Yes     |    Yes       |
---------------------------------------------------------



Sega NAOMI ROM cart usage
-------------------------

837-13668  171-7919A (C) Sega 1998
|----------------------------------------------------------|
|IC1   IC2   IC3   IC4   IC5   IC6         CN2             |
|ROM1  ROM2  ROM3  ROM4  ROM5  ROM6                     JP1|
|                                              IC42        |
|                                       71256              |
|                                       71256         IC22 |
|            IC7  IC8  IC9  IC10  IC11                     |
|            ROM7 ROM8 ROM9 ROM10 ROM11        IC41        |
|                                                          |
|                                              28MHz       |
|                                                          |
|    CN3              X76F100               CN1            |
|----------------------------------------------------------|
Notes:
      Not all MASKROM positions are populated.
      All MASKROMs (IC1 to IC21) are SOP44, either 32M or 64M
      The other side of the cart PCB just has more locations for
      SOP44 MASKROMs... IC12 to IC21 (ROM12 to ROM21)

      IC22    - DIP42 EPROM, either 27C160 or 27C322
      JP1     - Sets the size of the EPROM. 1-2 = 32M, 2-3 = 16M
      IC41    - Xilinx XC9536 (PLCC44)
      IC42    - SEGA 315-5881 (QFP100). Probably some kind of FPGA or CPLD. Usually different per game.
                On the end of the number, -JPN means it requires Japanese BIOS, -COM will run with any BIOS
      X76F100 - Xicor X76F100 secured EEPROM (SOIC8)
      71256   - IDT 71256 32kx8 SRAM (SOJ28)
      CN1/2/3 - connectors joining to main board

      Note! Generally, games that require a special I/O board or controller will not boot at all with a
            standard NAOMI I/O board. Usually they display a message saying the I/O board is not acceptable
            or not connected properly.


Games known to use this PCB include....

                           Sticker    EPROM        # of SOP44
Game                       on cart    IC22#        MASKROMs   IC41#      IC42#          Notes
--------------------------------------------------------------------------------------------------------------
Cosmic Smash               840-0044C  23428A       8          315-6213   317-0289-COM   joystick + 2 buttons
Dead Or Alive 2            841-0003C  22121        21         315-6213   317-5048-COM   joystick + 3 buttons
Dead Or Alive 2 Millenium  841-0003C  DOA2 Ver.M   21         315-6213   317-5048-COM   joystick + 3 buttons
Derby Owners Club          840-0016C  22099B       14         315-6213   317-0262-JPN   touch panel + 2 buttons + card reader
Dynamite Baseball '99      840-0019C  22141B       19         315-6213   317-0269-JPN   requires special panel (joystick + 2 buttons + bat controller for each player)
Dynamite Baseball Naomi    840-0001C  21575        21         315-6213   317-0246-JPN   requires special panel (joystick + 2 buttons + bat controller for each player)
Giant Gram Pro Wrestle 2   840-0007C  21820        9          315-6213   317-0253-JPN   joystick + 3 buttons
Heavy Metal Geo Matrix     HMG016007  23716A       11         315-6213   317-5071-COM   joystick + 2 buttons
Idol Janshi Suchie-Pai 3   841-0002C  21979        14         315-6213   317-5047-JPN   requires special I/O board and mahjong panel
Out Trigger                840-0017C  22163        19         315-6213   317-0266-COM   requires analog controllers/special panel
Power Stone                841-0001C  21597        8          315-6213   317-5046-COM   joystick + 3 buttons
Power Stone 2              841-0008C  23127        9          315-6213   317-5054-COM   joystick + 3 buttons
Samba de Amigo             840-0020C  22966B       16         315-6213   317-0270-COM   will boot but requires special controller to play it
Sega Marine Fishing        840-0027C  22221        10         315-6213   not populated  ROM 3&4 not populated. Requires special I/O board and fishing controller
Slash Out                  840-0041C  23341        17         315-6213   317-0286-COM   joystick + 4 buttons
Spawn                      841-0005C  22977B       10         315-6213   317-5051-COM   joystick + 4 buttons
Toy Fighter                840-0011C  22035        10         315-6212   317-0257-COM   joystick + 3 buttons
Virtua Striker 2 2000      840-0010C  21929C       15         315-6213   317-0258-COM   joystick + 3 buttons
Zombie Revenge             840-0003C  21707        19         315-6213   317-0249-COM   joystick + 3 buttons


Sega I/O boards
---------------

These are used on NAOMI and all other Sega games from 1999 onwards.
Not all I/O boards are listed here. If you know of others, please let us know.


838-13683
838-13683-91 (sticker)
838-13683-92 (sticker)
838-13683-93 (sticker)
|-----------------------------|
| CN6     CN7   CN2  CN1 RELAY|
| IC4 IC7          IC5    CN5 |
| IC3                  IC6    |
|CN3         IC1    IC2    IC9|
|    OSC1     JP1             |
|CN4                          |
|  IC8               IC10     |
|--|        JAMMA        |----|
   |---------------------|
Notes:
      JVS to JAMMA I/O board. Has both digital and analog inputs

              JVS test mode strings
              ---------------------
              NAME         SEGA ENTERPRISES,LTD.
                           I/O 838-13683B
                           Ver1.07
                           99/06
              CMD VER      1.1
              JVS VER      2.0
              COM VER      1.0
              SWITCH       2 PLAYER(S) 11 BITS
              COIN         2 SLOT
              ANALOG       8 CH
              ROTARY       0 CH
              KEYCODE      0
              SCREEN       X:0 Y:0 CH:0
              CARD         0 SLOT
              HOPPER OUT   0 CH
              DRIVER OUT   8 SLOT
              ANALOG OUT   0 CH
              CHARACTER    CHARA:0 LINE:0
              BACKUP       0

      CN1   - USB connector type A
      CN2   - USB connector type B
      CN3   - 14 pin connector used for switch input or lamp output via jumper setting
              1-2    +5V
              3      NC
              4-6    1P SW6-SW8
              7      NC
              8-10   2P SW6-SW8
              11-12  NC
              13-14  GND
      CN4   - 2 pin connector
      CN5   - 15 pin VGA connector
      CN6   - 6 pin connector used for 5 volt and 12 volt power input/output
              1-2 +5V
              3-4 +12V
              5-6 GND
      CN7   - 26 pin analog controls connector
              1-2  +5V              15     Reserved
              3    1P Analog Y      16     Reserved
              4    2P Analog Y      21     Reserved
              9    1P Analog X      22     Reserved
              10   2P Analog X      23-24  GND
      IC1   - Toshiba TMP90PH44 microcontroller with sticker 'SP5001-B' (SDIP64)
      IC2   - location for TMP90PH44 QFP64 (not used)
      IC3   - 74HC541 (SOIC20)
      IC4   - BA6212 (DIP20)
      IC5   - Analog Devices ADM485 (SOIC8)
      IC6   - 5W393 (SOIC8)
      IC7   - 74HC4052 (SOIC16)
      IC8   - Toshiba TD62384 (SOIC16)
      IC9   - A7666FS (SOIC16)
      IC10  - 74F86 (SOIC14)
      JP1   - Set in position A to use CN3 as input for switches. Set in position B to use as output (for lamps etc)
              When in position A, in the JVS test mode, 'SWITCH' becomes '2 PLAYER(S) 15 BITS' and 'DRIVER OUT' becomes '0 SLOT'
      RELAY - Omron G6S-2 relay
      OSC1  - 14.74MHz


837-13551
837-13551-92 (sticker)
837-13551-93 (sticker)
|-----------------------------|
|CN4 CN5         CN6   CN7 CN8|
|LED   ADM485                 |
|  RELAY         IC1A    *    |
|                             |
|                    14.745MHz|
|            PS2801-4(x8)     |
|                          LED|
|CN1 CN2      CN3             |
|-----------------------------|
Notes:
             JVS I/O board. Has both digital and analogue inputs.
             This is the most common type. Used on Sega driving games, NAOMI, Hikaru, Triforce, Chihiro etc

             JVS test mode strings
             ---------------------
             NAME         SEGA ENTERPRISES,LTD.
                          I/O BD JVS
                          837-13551
                          Ver1.00
             CMD VER      1.1
             JVS VER      2.0
             COM VER      1.0
             SWITCH       2 PLAYER(S) 13 BITS
             COIN         2 SLOT(S)
             ANALOG       8 CH
             ROTARY       0 CH
             KEYCODE      0
             SCREEN       X:0 Y:0 CH:0
             CARD         0 SLOT(S)
             HOPPER OUT   0 CH
             DRIVER OUT   6 CH
             ANALOG OUT   0 CH
             CHARACTER    CHARA:0 LINE:0
             BACKUP       0

      IC1A - Toshiba TMP90PH44 microcontroller marked '315-6215' (SDIP64)
         * - location under the PCB for TMP90PH44 QFP64 (not used)
     RELAY - NEC EB2-4.5NU relay
    ADM485 - Analog Devices ADM485
       CN1 - 5 pin connector 12 volt power input
             1-2 +12V
             3 NC
             4-5 GND
       CN2 - 5 pin connector 12 volt power output
             1-2 +12V
             3 NC
             4-5 GND
       CN3 - 60 pin digital input connector
             1-8   +5V          33    1P SW4
             9-15  GND          34    2P SW4
             16    NC           35    1P SW5
             17    1P Start     36    2P SW5
             18    2P Start     37    1P SW6
             19    1P Right     38    2P SW6
             20    2P Right     39    1P SW7
             21    1P Left      40    2P SW7
             22    2P Left      41    1P Service Credit
             23    1P Up        42    2P Service Credit
             24    2P Up        43    Test SW
             25    1P Down      44    Tilt SW
             26    2P Down      45    Coin SW1
             27    1P SW1       46    Coin SW2
             28    2P SW1       47-48 NC
             29    1P SW2       49-50 Coin Meter 1 & 2
             30    2P SW2       51-56 Output 1-6
             31    1P SW3       57-60 +12V
             32    2P SW3
             Note: For coin input to work, the coin meters need to be connected to
             pins 49 and 50, or tie +5 volts to pin 49 and 50 via a resistor.
       CN4 - USB connector type B
       CN5 - USB connector type A
       CN6 - Analog I/O connector
             1-2   VCC     15    AD 2
             3     AD 0    16    AD 6
             4     AD 4    17-18 NC
             5     GND     19-20 VCC
             6     GND     21    AD 3
             7-8   NC      22    AD 7
             9     AD 1    23    GND
             10    AD 5    24    GND
             11-14 NC      25-26 NC
       CN7 - 4 pin connector 5V power input
             1-2 +5v       3-4 GND
       CN8 - 4 pin connector 5V power out
             1-2 +5v       3-4 GND


837-13741
837-13844-02 (sticker)
|-----------------------------|
|       CN8        CN9        |
|DB9                       CN1|
|RESET_SW |----|              |
|         |IC6 |  OSC1     CN5|
|         |----|              |
|CN7      LED  DSW1(5)     CN2|
|   OSC2   RELAY              |
|CNx  USB USB  IC7 LED     CN6|
|-----------------------------|
Notes: (most info taken from poor quality pics/scans, better info is needed)

             JVS I/O board 2. Has both digital and analogue inputs.
             This board is used with F355, Ghost Squad, and many
             others including network/satellite games.

             JVS test mode strings
             ---------------------

             NAME         SEGA ENTERPRISES,LTD.;837-13741
                          I/O CONTROL BD2;Ver0.15;99/06
             CMD VER      1.1
             JVS VER      2.0
             COM VER      1.0
             SWITCH       2 PLAYERS 12BITS
             COIN         2 SLOTS
             ANALOG       8CH
             DRIVER OUT   22CH

      IC6  - Sega 315-6146 custom IC (QFP176)
      IC7  - 27C512 EPROM with label 'EPR-22082' (DIP28)
      IC8  - Sharp LH52256 32k x8 SRAM (SOP28)
      IC10 - Something by NEC? (QFP44)
      OSC1 - 14.7456MHz
      OSC2 - 32MHz
      CNx  - 6 pin connector
      CN1  - 5 pin connector 12 volt power input
             1-2 +12V
             3 NC
             4-5 GND
      CN2  - 4 pin connector 5V power input
             1-2 +5v
             3-4 GND
      CN5  - 5 pin connector 12 volt power output
             1-2 +12V
             3 NC
             4-5 GND
      CN6  - 4 pin connector 5V power output
             1-2 +5v
             3-4 GND
      CN7  - 26 pin connector (many pins unknown)
             1  +5V              14
             2                   15  Analog Output
             3  Analog Output    16
             4                   17  GND
             5  GND              18
             6                   19  +5V
             7  +5V              20
             8                   21  Analog Output
             9  Analog Output    22
             10                  23  GND
             11 GND              24
             12                  25
             13 +5V              26
      CN8  - 40 pin connector (many pins unknown)
             3-5   +5V           21    Switch
             7-9   GND           22    Switch
             13-14 Coin SW1/2    23    Switch
             15    Test SW       24    Switch
             17    1P Start SW   25    2P Start SW
             18    Service SW    31    Switch
             19    Switch        32    Switch
             20    Switch        33-40 RX1-RX8 (for communications)
      CN9  - 34 pin connector (some pins unknown)
             1-4   +5V
             5-8   GND
             9-10  Coin Meter 1 & 2
             11    Lamp
             12    Lamp
             13    Lamp
             14    Lamp
             15    Lamp
             16    Lamp
             17    Lamp
             18    Lamp
             19    Lamp
             20    Lamp
             21    Coin LED
             22-24 ?
             25-32 TX1-TX8 (for communications)
             33-34 12V


837-13938
171-7807A
|--------------------|
|CN2      CN1    IC9S|
|         OSC1       |
|      |-----|       |
|      | IC2 |OSC2   |
| CN3  |     |       |
|  IC7S|-----|    IC3|
|LED    CN4     IC4  |
|--------------------|
Notes:
      This is the I/O board used for Dynamic Golf which is
      located under the panel. It's also used for spinner
      controls such as those used on Outtrigger.
      It must be daisy-chained to the normal I/O board with a USB cable.

      CN1 - 24 pin connector (not used on Dynamic Golf, other use unknown)
      CN2 - 4 pin connector used for 5 volt power input
      CN3 - USB connector type B
      CN4 - 16 pin connector used for buttons and trackball
      IC1 - HC240 logic IC (SOIC20)
      IC2 - Sega 315-6146 custom IC (QFP176)
      IC3 - 27C512 EPROM with label 'EPR-22084' (DIP28)
      IC4 - HC4020 logic IC (SOIC16)
      These parts on the other side of the PCB....
      IC7S - Analog Devices ADM485 (SOIC8)
      IC9S - Sharp LH52256 32k x8 SRAM (SOP28)
      OSC1 - 14.7456MHz
      OSC2 - 32MHz
      Not shown above.... Sharp PC410 (x8, at PC9S to PC16S)
                          HC74 at IC8S
                          34164 (?) at IC10S (SOIC8)

Sega's I/O board has:
- spare output of 5V, 12V, and GND (from JAMMA power input via noise filter)
- analog input
- USB input (connect to NAOMI motherboard)
- USB output (not used)
- shrinked D-sub 15pin connector (connect JVS video output, the signal is routed to JAMMA connector, signal is amplified to 3Vp-p and H/V sync signal is mixed (composite))
- external I/O connector (JST 12pin)
- switch to select function of external I/O connector (extra button input or 7-seg LED(x2) output of total wins for 'Versus City' cabinet)
- spare audio input (the signal goes to JAMMA speaker output)
- JAMMA connector

external I/O connector

old version
 1 +5V
 2 +5V
 3 +5V
 4 1P PUSH 4
 5 1P PUSH 5
 6 1P PUSH 6
 7 1P PUSH 7
 8 2P PUSH 4
 9 2P PUSH 5
10 2P PUSH 6
11 2P PUSH 7
12 GND
13 GND
14 GND
(PUSH4 and 5 are common to JAMMA)

new version
 1 +5V
 2 +5V
 3 +5V
 4 1P PUSH 6
 5 1P PUSH 7
 6 1P PUSH 8
 7 1P PUSH 9
 8 2P PUSH 6
 9 2P PUSH 7
10 2P PUSH 8
11 2P PUSH 9
12 GND
13 GND
14 GND

mahjong panel uses ext. I/O 4-8 (regardless of I/O board version)
key matrix is shown in below

  +------------------------------------ ext. I/O 8
  |     +------------------------------ ext. I/O 7
  |     |     +------------------------ ext. I/O 6
  |     |     |     +------------------ ext. I/O 5
  |     |     |     |     +------------ ext. I/O 4
(LST)-( D )-( C )-( B )-( A )---------- JAMMA 17 (1p start)
  |     |     |     |     |
  |   ( H )-( G )-( F )-( E )---------- JAMMA 18 (1p up)
  |     |     |     |     |
  |   ( L )-( K )-( J )-( I )---------- JAMMA 19 (1p down)
  |     |     |     |     |
(F/F)-(PON)-(CHI)-( N )-( M )---------- JAMMA 20 (1p left)
        |     |     |     |
        +---(RON)-(RCH)-(KAN)---------- JAMMA 21 (1p right)
              |     |     |
              +---(BET)-(STR)---------- JAMMA 22 (1p push1)

* LST = Last chance, F/F = Flip flop, STR = Start


---------------------------------------------------------------------------------------------------------------------------------------------------

Guru's Readme
-------------

Atomis Wave (system overview)
Sammy/SEGA, 2002

This is one of the latest arcade systems so far, basically just a Dreamcast using ROM carts.

PCB Layout
----------

Sammy AM3AGA-04 Main PCB 2002 (top side)
1111-00006701 (bottom side)
   |--------------------------------------------|
  |-      TA8210AH               D4721   3V_BATT|
  |VGA                              BS62LV1023TC|
  |VOL                                          |
  |SER      |-----|               PCM1725U      |
  |-        |ROMEO|       D4516161              |
   |CN3     |     |                    BIOS.IC23|
|--|SW1     |-----|                             |
|                     |-----|           |-----| |
|           33.8688MHz|315- |           |ROMEO| |
|           |-----|   |6232 |           |     | |
|           |315- |   |-----|           |-----| |
|           |6258 |  32.768kHz                  |
|J          |-----|                             |
|A                 |-------------|    D4564323  |
|M                 |             |              |
|M                 |             |  |--------|  |
|A        D4516161 |  315-6267   |  |        |  |
|                  |             |  |  SH4   |  |
|                  |             |  |        |  |
|  TD62064         |             |  |        |  |
|         D4516161 |             |  |--------|  |
|                  |-------------|              |
|                                     D4564323  |
|--|             D4516161  D4516161             |
   |                             W129AG  13.5MHz|
   |--------------------------------------------|
Notes:
------
BS62LV1023TC-70 - Brilliance Semiconductor Low Power 128k x8 CMOS SRAM (TSOP32)
TA8210AH        - Toshiba 19W x 2 Channel Power Amplifier
D4516161AG5     - NEC uPD4516161AG5-A80 1M x16 (16MBit) SDRAM (SSOP50)
D4564323        - NEC uPD4564323G5-A10 512K x 32 x 4 (64MBit) SDRAM (SSOP86)
D4721           - NEC uPD4721 RS232 Line Driver Receiver IC (SSOP20)
PCM1725U        - Burr-Brown PCM1725U 16Bit Digital to Analog Converter (SOIC14)
2100            - New Japan Radio Co. Ltd. NJM2100 Dual Op Amp (SOIC8)
ROMEO           - Sammy AX0201A01 'ROMEO' 4111-00000501 0250 K13 custom ASIC (TQFP100)
315-6232        - SEGA 315-6232 custom ASIC (QFP100)
315-6258        - SEGA 315-6258 custom ASIC (QFP56)
315-6267        - SEGA 315-6267 custom ASIC (BGAxxx)
TD62064         - Toshiba TD62064 NPN 50V 1.5A Quad Darlington Driver (SOIC16)
SH4             - Hitachi SH-4 HD6417091RA CPU (BGA256)
BIOS.IC23       - Macronix 29L001MC-10 3.3volt FlashROM (SOP44)
                  This is a little strange, the ROM appears to be a standard SOP44 with reverse pinout but the
                  address lines are shifted one pin out of position compared to industry-standard pinouts.
                  The actual part number doesn't exist on the Macronix web site, even though they have datasheets for
                  everything else! So it's probably a custom design for Sammy and top-secret!
                  The size is assumed to be 1MBit and is 8-bit (D0-D7 only). The pinout appears to be this.....

                               +--\/--+
tied to WE of BS62LV1023   VCC | 1  44| VCC - tied to a transistor, probably RESET
                           NC  | 2  43| NC
                           A9  | 3  42| NC
                           A10 | 4  41| A8
                           A11 | 5  40| A7
                           A12 | 6  39| A6
                           A13 | 7  38| A5
                           A14 | 8  37| A4
                           A15 | 9  36| A3
                           A16 |10  35| A2
                           NC  |11  34| A1
                           NC  |12  33| CE - tied to 315-6267
                           GND |13  32| GND
                           A0  |14  31| OE
                           D7  |15  30| D0
                           NC  |16  29| NC
                           D6  |17  28| D1
                           NC  |18  27| NC
                           D5  |19  26| D2
                           NC  |20  25| NC
                           D4  |21  24| D3
                           VCC |22  23| NC
                               +------+

W129AG          - IC Works W129AG Programmable Clock Frequency Generator, clock input of 13.5MHz (SOIC16)
SW1             - 2-position Dip Switch
VGA             - 15 pin VGA out connector @ 31.5kHz
SER             - 9 pin Serial connector  \
VOL             - Volume pot              / These are on a small daughterboard that plugs into the main PCB via a multi-wire cable.
CN3             - Unknown 10 pin connector labelled 'CN3'
3V_BATT         - Panasonic ML2020 3 Volt Coin Battery

The bottom of the PCB contains nothing significant except some connectors. One for the game cart, one for special controls
or I/O, one for a communication module, one for a cooling fan and one for the serial connection daughterboard.

---------------------------------------------------------------------------------------------------------------------------------------------------



Atomiswave cart PCB layout and game usage (revision 0.3 19/7/2009 5:55pm)
-----------------------------------------

Type 1 ROM Board:


AM3AGB-04
MROM PCB
2002
|----------------------------|
| XC9536                     |
|         IC18 IC17*   IC10  |
|                            |
|                            |
|              IC16*   IC11  |
|                            |
|                            |
||-|           IC15*   IC12  |
|| |                         |
|| |                         |
|| |CN1        IC14*   IC13  |
|| |                         |
||-|                         |
|----------------------------|
Notes:
           * - Denotes those devices are on the other side of the PCB
         CN1 - This connector plugs into the main board.
      XC9536 - Xilinx XC9536 in-system programmable CPLD (PLCC44), stamped with a
               game code. This code is different for each different game.
               The last 3 digits seems to be for the usage.
               F01 = CPLD/protection device and M01 = MASKROM

               Game (sorted by code)                 Code
               -----------------------------------------------
               Sports Shooting USA                   AX0101F01
               Dolphin Blue                          AX0401F01
               Demolish Fist                         AX0601F01
               Guilt Gear Isuka                      AX1201F01
               Knights Of Valour Seven Spirits       AX1301F01
               Salaryman Kintaro                     AX1401F01
               Ranger Mission                        AX1601F01
               Rumble Fish                           AX1801F01
               Fist Of The North Star                AX1901F01
               Victory Furlong : Horse Racing        AX2001F01
               King Of Fighters NEOWAVE              AX2201F01
               Extreme Hunting                       AX2401F01

        IC18 - Fujitsu 29DL640E 64M TSOP48 FlashROM. This ROM has no additional custom markings
               The name in the archive has been devised purely for convenience.
               This ROM holds the main program.

IC10 to IC17 - Custom-badged 128M TSOP48 mask ROMs. I suspect they are Macronix
               ROMs because the ROM on the main board is also a custom Macronix
               ROM and they have a history of producing custom ROMs for other
               companies that hide their ROM types like Nintendo etc.

               IC10 - Not Populated for 7 ROMs or less (ROM 01 if 8 ROMs are populated)
               IC11 - ROM 01 (or ROM 02 if 8 ROMs are populated)
               IC12 - ROM 02 (or ROM 03 if 8 ROMs are populated)
               IC13 - ROM 03 (or ROM 04 if 8 ROMs are populated)
               IC14 - ROM 04 (or ROM 05 if 8 ROMs are populated)
               IC15 - ROM 05 (or ROM 06 if 8 ROMs are populated)
               IC16 - ROM 06 (or ROM 07 if 8 ROMs are populated)
               IC17 - ROM 07 (or ROM 08 if 8 ROMs are populated)

               ROM Codes
               ---------
                                                                               Number
               Game (sorted by code)                 Code                      of ROMs
               -----------------------------------------------------------------------
               Sports Shooting USA                   AX0101M01 to AX0104M01    4
               Dolphin Blue                          AX0401M01 to AX0405M01    5
               Demolish Fist                         AX0601M01 to AX0607M01    7
               Guilty Gear Isuka                     AX1201M01 to AX1208M01    8
               Knights Of Valour Seven Spirits       AX1301M01 to AX1307M01    7
               Salaryman Kintaro                     AX1401M01 to AX1407M01    7
               Ranger Mission                        AX1601M01 to AX1605M01    5
               Rumble Fish                           AX1801M01 to AX1807M01    7
               Fist Of The North Star                AX1901M01 to AX1907M01    7
               Victory Furlong : Horse Racing        AX2001M01 to AX2007M01    7
               King Of Fighters NEOWAVE              AX2201M01 to AX2206M01    6
               Extreme Hunting                       AX2401M01 to AX2406M01    6


Type 2 ROM Board:


AM3ALW-02
MROM2 PCB
2005
|----------------------------|
|     FMEM1                  |
|     FMEM2*   MROM12        |
|              MROM11*       |
|                      MROM9 |
|              MROM10  MROM8*|
| XCR3128XL*   MROM7*        |
|                            |
||-|           MROM6         |
|| |           MROM3*  MROM4 |
|| |                   MROM5*|
|| |CN1        MROM2         |
|| |           MROM1*        |
||-|                         |
|----------------------------|
Notes:
           * - Denotes those devices are on the other side of the PCB
         CN1 - This connector plugs into the main board.
   XCR3128XL - Xilinx XCR3128XL in-system programmable 128 Macro-cell CPLD (TQFP100)
               stamped with a game code. This code is different for each different game.
               The last 3 digits seems to be for the usage.
               F01 = CPLD/protection device and M01 = MASKROM

               Game (sorted by code)                 Code
               -----------------------------------------------
               Samurai Spirits Tenkaichi Kenkakuden  AX2901F01
               Neogeo Battle Coliseum                AX3301F01
               Rumble Fish 2                         AX3401F01

 FMEM1/FMEM2 - Fujitsu 29DL640E 64M TSOP48 FlashROM. This ROM has no additional custom markings
               The name in the archive has been devised purely for convenience.
               This ROM holds the main program.
               This location is wired to accept TSOP56 ROMs, however the actual chip populated
               is a TSOP48, using the middle pins. The other 2 pins on each side of the ROM
               are not connected to anything.

       MROM* - Custom-badged SSOP70 mask ROMs. These may be OKI MR26V25605 or MR26V25655 (256M)
               or possibly 26V51253 (512M) or something else similar.

               ROM Codes
               ---------
                                                                               Number
               Game (sorted by code)                 Code                      of ROMs
               -----------------------------------------------------------------------
               Samurai Spirits Tenkaichi Kenkakuden  AX2901M01 to AX2907M01    7
               Neogeo Battle Coliseum                AX3301M01 to AX3307M01    7
               Rumble Fish 2                         AX3401M01 to AX3405M01    5


Type 3 ROM Board:

This type is manufactured by Sega when Sammy merged with Sega.

171-8355A
PC BD A/W 128M FLASH
837-14608 (sticker for Extreme Hunting 2 Tournament Edition)
|----------------------------|
| XC9536*         U16   U2*  |
|                            |
|J2                          |
|                            |
|                 U4*   U14  |
|                            |
|                            |
||-|                         |
|| |              U17   U1*  |
|| |                         |
|| |J1*                      |
|| |                         |
||-|              U3*   U15  |
|----------------------------|
Notes:
           * - Denotes those parts are on the other side of the PCB
          J1 - This connector plugs into the main board.
          J2 - 6 pin connector for programming the XC9536 CPLD and/or the flash ROMs
      XC9536 - Xilinx XC9536 in-system programmable CPLD (PLCC44), stamped with a
               Sega 315-xxxx part number with a sticker over the top of it.

               Game                                  Part#      Sticker
               ---------------------------------------------------------
               Extreme Hunting 2 Tournament Edition  315-6428P  315-6248

          U* - Fujitsu MBM29PL12LM-10PCN 128M MirrorFlash TSOP56 flash ROM.
               (configured as 16Mbytes x8bit or 8Mwords x16bit)
               This ROM has no additional custom markings. The name in the archive has been devised
               purely for convenience. The number of ROMs may vary between games. So far all 8
               positions have been seen populated. It's also possible all positions are present
               when manufactured, each board is programmed to requirements and depending on the total
               data length, some ROMs may be empty.


               Other games not dumped (some may have been cancelled, * = known to be released)
               ----------------------
               Chase 1929
               Dirty Pigskin
               Faster Than Speed *
               Force Five
               Guilty Gear X Version 1.5 *
               Kenju
               Maximum Speed *
               Metal Slug 6 *
               Premier Eleven
               Sushi Bar
               The King Of Fighters XI *
               Sega Clay Challenge *
               Sega Bass Fishing Challenge *


Network Board
-------------

This board is required for Extreme Hunting 2 Tournament Edition, although it doesn't need to be connected
to a network or another Atomiswave unit to boot up. However it must be plugged into the PCB in the
communication slot or the game will not go in-game. It will boot but then displays NETWORK BOARD ERROR
if not present. Externally there's a hole for an RJ45 network cable and a slot for a PIC16C621/PIC16C622
PIC enclosed in a black plastic housing. This is the same type as used in NAOMI etc. This board probably acts
like the NAOMI network DIMM board minus the on-board DIMM RAM storage.

837-14508R
171-8324C
(C) SEGA 2005
|-----------------------------------|
| RJ45    24LC0241* IC2       CN3   |
|LLLL        K4S643232*       IC14  |
|  RTL8201 LLLL                     |
|25MHz                              |
|       6417710          IC4S*      |
|                        XC3S200    |
|MAX3221        JP1     XCF01S*     |
|CN5  33.333MHz JP2             CN2*|
|-----------------------------------|
Notes:
      *        - Denotes those parts are on the other side of the PCB
      L        - LED
      RJ45     - RJ45 network connector
      24LC0241 - EEPROM (SOIC8)
      K4S643232- Samsung K4S643232 512k x 32bit x 4 banks synchronous DRAM (TSOPII-86)
      RTL8201  - Realtek RTL8201 Single Port 10/100M Fast Ethernet IC (QFP48)
      6417710  - Renesas HD6417710BPV SH3-DSP 32-Bit RISC Microcomputer SuperHTM RISC engine Family / SH7700 Series (BGA256)
      XC3S200  - Xilinx Spartan XC3S200 FPGA (QFP100)
      XCF01S   - Xilinx XCF01S In-System Programmable 1Mbit PROM for Configuration of Xilinx FPGAs (TSSOP20)
      MAX3221  - Maxim MAX3221 3.0V to 5.5V, 250kbps, RS-232 Transceivers with Auto Shutdown (TSSOP16)
      JP1/2    - Jumpers, both set to 1-2
      CN2      - This connector plugs into the main board
      CN3      - 6 pin connector
      CN5      - 3 position connector
      IC2      - ST M29DW324DB 32M flash ROM (TSOP48)
      IC4S     - Spartan S29GL128N10TFIO1 128M flash ROM (TSOP56)
      IC14     - socket for PIC16C621 mounted in a plastic plug-in case
                 PIC Usage:
                           Game                                   Sega Part#
                           ---------------------------------------------------
                           Extreme Hunting 2 Tournament Edition   317-0445-COM

Gun Sub Board
-------------

AM3AGT-02 GUN SUB PCB
|------------------|
|CN5  CN4  CN3  CN2|
|       74HC74     |
|                  |
| 74HC74    7CHC74 |
|      74HC74      |
|                  |
|       CN1        |
|------------------|
Notes:
      CN1 - 8 pin connector joining to I/O Expansion Module (which is plugged into main board)
      CN2 - Gun connection for player 2 trigger and optical
      CN3 - Gun connection for player 2 pump switch
      CN4 - Gun connection for player 1 trigger and optical
      CN5 - Gun connection for player 1 pump switch

*/

#include "driver.h"
#include "cpu/arm7/arm7.h"
#include "video/generic.h"
#include "machine/eeprom.h"
#include "machine/intelfsh.h"
#include "naomibd.h"
#include "naomi.h"
#include "cpu/sh4/sh4.h"
#include "cpu/arm7/arm7core.h"
#include "sound/aica.h"
#include "dc.h"

#define CPU_CLOCK (200000000)
static UINT32 *dc_sound_ram;
                                             /* MD2 MD1 MD0 MD6 MD4 MD3 MD5 MD7 MD8 */
static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, CPU_CLOCK };

static INTERRUPT_GEN( naomi_vblank )
{
	dc_vblank(device->machine);
}

static READ64_HANDLER( naomi_arm_r )
{
	return *((UINT64 *)dc_sound_ram+offset);
}

static WRITE64_HANDLER( naomi_arm_w )
{
	COMBINE_DATA((UINT64 *)dc_sound_ram + offset);
}

static READ64_HANDLER( naomi_unknown1_r )
{
	if ((offset * 8) == 0xc0) // trick so that it does not "wait for multiboard sync"
		return -1;
	return 0;
}

static WRITE64_HANDLER( naomi_unknown1_w )
{
}

/*
* Non-volatile memories
*/

// some default eeprom settings
static const struct
{
	const char *name;
	const UINT8 eeprom[128];
}
jvseeprom_default_game[] =
{
	{ "karous", { 0x58, 0xE5, 0x11, 0x42, 0x4D, 0x57, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x58, 0xE5, 0x11, 0x42, 0x4D, 0x57, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "sfz3ugd", { 0xE3, 0xF2, 0x10, 0x42, 0x43, 0x59, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xE3, 0xF2, 0x10, 0x42, 0x43, 0x59, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xBE, 0x1F, 0x18, 0x18, 0xBE, 0x1F, 0x18, 0x18, 0x30, 0x59, 0x43, 0x42, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x30, 0x59, 0x43, 0x42, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "shikgam2", { 0xA4, 0xF6, 0x11, 0x42, 0x47, 0x45, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xA4, 0xF6, 0x11, 0x42, 0x47, 0x45,	0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,0x43, 0x84, 0x20, 0x20, 0x43, 0x84, 0x20, 0x20, 0x42, 0x47, 0x45, 0x30,	0x53, 0x48, 0x49, 0x4B, 0x49, 0x32, 0x52, 0x4F, 0x4D, 0x20, 0x30, 0x32,	0x32, 0x38, 0x00, 0x00, 0x4B, 0x4B, 0x00, 0x02, 0x00, 0x02, 0x01, 0x00,	0x00, 0x00, 0x00, 0x00, 0x42, 0x47, 0x45, 0x30, 0x53, 0x48, 0x49, 0x4B,	0x49, 0x32, 0x52, 0x4F, 0x4D, 0x20, 0x30, 0x32, 0x32, 0x38, 0x00, 0x00,	0x4B, 0x4B, 0x00, 0x02, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "quizqgd", { 0xAD, 0x1E, 0x11, 0x42, 0x46, 0x50, 0x31, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xAD, 0x1E, 0x11, 0x42, 0x46, 0x50,	0x31, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0xC4, 0x18, 0x1D, 0x20, 0xC4, 0x18, 0x1D, 0x20, 0x42, 0x46, 0x50, 0x31,	0x51, 0x2D, 0x4D, 0x4F, 0x44, 0x45, 0x51, 0x2D, 0x4D, 0x4F, 0x44, 0x45,	0x01, 0x01, 0x05, 0x01, 0x02, 0x00, 0x01, 0x03, 0x01, 0x00, 0x4B, 0x4B,	0x00, 0x00, 0x00, 0x00, 0x42, 0x46, 0x50, 0x31, 0x51, 0x2D, 0x4D, 0x4F,	0x44, 0x45, 0x51, 0x2D, 0x4D, 0x4F, 0x44, 0x45, 0x01, 0x01, 0x05, 0x01,	0x02, 0x00, 0x01, 0x03, 0x01, 0x00, 0x4B, 0x4B, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "senko",  { 0xCD, 0xEE, 0x10, 0x42, 0x4B, 0x47, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xCD, 0xEE, 0x10, 0x42, 0x4B, 0x47,	0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0xE5, 0x91, 0x10, 0x10, 0xE5, 0x91, 0x10, 0x10, 0x07, 0x51, 0x17, 0x03,	0x00, 0x01, 0x02, 0x02, 0x46, 0x00, 0x96, 0x00, 0x46, 0x00, 0x00, 0x00,	0x07, 0x51, 0x17, 0x03, 0x00, 0x01, 0x02, 0x02, 0x46, 0x00, 0x96, 0x00,	0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "senkoo", { 0xCD, 0xEE, 0x10, 0x42, 0x4B, 0x47, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xCD, 0xEE, 0x10, 0x42, 0x4B, 0x47,	0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x12, 0xF0, 0x0C, 0x0C, 0x12, 0xF0, 0x0C, 0x0C, 0x07, 0x51, 0x17, 0x03,	0x01, 0x02, 0x46, 0x00, 0x96, 0x00, 0x00, 0x00, 0x07, 0x51, 0x17, 0x03,	0x01, 0x02, 0x46, 0x00, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "radirgy", { 0x0C, 0x29, 0x11, 0x42, 0x4B, 0x57, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x0C, 0x29, 0x11, 0x42, 0x4B, 0x57,	0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "trgheart", { 0xF0, 0xC2, 0x11, 0x42, 0x4D, 0x41, 0x31, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xF0, 0xC2, 0x11, 0x42, 0x4D, 0x41,	0x31, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x7E, 0xE1, 0x24, 0x24, 0x7E, 0xE1, 0x24, 0x24, 0x30, 0x41, 0x4D, 0x42,	0x54, 0x52, 0x49, 0x47, 0x47, 0x45, 0x52, 0x48, 0x45, 0x41, 0x52, 0x54,	0x20, 0x45, 0x58, 0x45, 0x4C, 0x49, 0x43, 0x41, 0x00, 0x00, 0x00, 0x00,	0x01, 0x03, 0x02, 0x00, 0x09, 0x09, 0x09, 0x00, 0x30, 0x41, 0x4D, 0x42,	0x54, 0x52, 0x49, 0x47, 0x47, 0x45, 0x52, 0x48, 0x45, 0x41, 0x52, 0x54,	0x20, 0x45, 0x58, 0x45, 0x4C, 0x49, 0x43, 0x41, 0x00, 0x00, 0x00, 0x00,	0x01, 0x03, 0x02, 0x00, 0x09, 0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "gundmgd", { 0xCD, 0x77, 0x10, 0x42, 0x43, 0x56, 0x30, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xCD, 0x77, 0x10, 0x42, 0x43, 0x56,	0x30, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "gundmxgd", { 0xAA, 0xAD, 0x10, 0x42, 0x44, 0x55, 0x30, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xAA, 0xAD, 0x10, 0x42, 0x44, 0x55,	0x30, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "psyvar2", { 0xCB, 0x6E, 0x11, 0x42, 0x48, 0x42, 0x30, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xCB, 0x6E, 0x11, 0x42, 0x48, 0x42,	0x30, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "cfield", { 0xFB, 0xD4, 0x10, 0x42, 0x4A, 0x42, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xFB, 0xD4, 0x10, 0x42, 0x4A, 0x42,	0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "puyofev", { 0x65, 0x7E, 0x10, 0x42, 0x48, 0x44, 0x00, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x65, 0x7E, 0x10, 0x42, 0x48, 0x44,	0x00, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x0E, 0x0B, 0x20, 0x20, 0x0E, 0x0B, 0x20, 0x20, 0x42, 0x48, 0x44, 0x00,	0x50, 0x55, 0x59, 0x4F, 0x20, 0x50, 0x55, 0x59, 0x4F, 0x20, 0x46, 0x45,	0x56, 0x45, 0x52, 0x2E, 0x20, 0x10, 0x03, 0x20, 0x01, 0x00, 0x02, 0x03,	0x00, 0x80, 0x00, 0x00, 0x42, 0x48, 0x44, 0x00, 0x50, 0x55, 0x59, 0x4F,	0x20, 0x50, 0x55, 0x59, 0x4F, 0x20, 0x46, 0x45, 0x56, 0x45, 0x52, 0x2E,	0x20, 0x10, 0x03, 0x20, 0x01, 0x00, 0x02, 0x03, 0x00, 0x80, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "bdrdown", { 0xA0, 0x75, 0x10, 0x42, 0x47, 0x48, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xA0, 0x75, 0x10, 0x42, 0x47, 0x48,	0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0xAE, 0x7E, 0x08, 0x08, 0xAE, 0x7E, 0x08, 0x08, 0x01, 0x00, 0x00, 0x00,	0x33, 0xB8, 0x96, 0xE4, 0x01, 0x00, 0x00, 0x00, 0x33, 0xB8, 0x96, 0xE4,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "ss2005", { 0xAD, 0x0A, 0x10, 0x42, 0x4B, 0x4C, 0x30, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xAD, 0x0A, 0x10, 0x42, 0x4B, 0x4C,	0x30, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "usagui", { 0x02, 0x8C, 0x10, 0x42, 0x47, 0x46, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x02, 0x8C, 0x10, 0x42, 0x47, 0x46,	0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "undefeat", { 0x06, 0x18, 0x11, 0x42, 0x4C, 0x48, 0x30, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x06, 0x18, 0x11, 0x42, 0x4C, 0x48,	0x30, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "trizeal", { 0x8F, 0x05, 0x11, 0x42, 0x4A, 0x46, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x8F, 0x05, 0x11, 0x42, 0x4A, 0x46,	0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "moeru",  { 0xA1, 0x18, 0x10, 0x42, 0x45, 0x55, 0x20, 0x09, 0x00, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0xA1, 0x18, 0x10, 0x42, 0x45, 0x55,	0x20, 0x09, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "tetkiwam", { 0x3C, 0x11, 0x10, 0x42, 0x47, 0x43, 0x31, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x3C, 0x11, 0x10, 0x42, 0x47, 0x43,	0x31, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "keyboard", { 0x32, 0x7E, 0x10, 0x42, 0x45, 0x42, 0x30, 0x09, 0x10, 0x00, 0x01, 0x01,	0x01, 0x00, 0x11, 0x11, 0x11, 0x11, 0x32, 0x7E, 0x10, 0x42, 0x45, 0x42,	0x30, 0x09, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x11, 0x11, 0x11, 0x11,	0xF0, 0x4A, 0x0C, 0x0C, 0xF0, 0x4A, 0x0C, 0x0C, 0x18, 0x09, 0x01, 0x20,	0x02, 0x00, 0x3C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x18, 0x09, 0x01, 0x20,	0x02, 0x00, 0x3C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }
};

static NVRAM_HANDLER( aw_nvram )
{
	nvram_handler_intelflash(machine, 0, file, read_or_write);

	// TODO: save AW NVRAM at 00200000
}

static NVRAM_HANDLER( naomi_eeproms )
{
	if (read_or_write)
	{
		/* JVS 'eeprom' */
		mame_fwrite(file,maple0x86data1,0x80);

		// mainboard eeprom?
		eeprom_save(file);
	}
	else
	{
		eeprom_init(machine, &eeprom_interface_93C46);
		if (file)
		{
			UINT8 tmp[0x80];

			/* JVS 'eeprom' */
			mame_fread(file,maple0x86data1,0x80);

			mame_fread(file,&tmp,0x80);
			eeprom_set_data((UINT8 *)tmp, 0x80);

			// mainboard eeprom?
            eeprom_load(file);

		}
        	else
		{
		//  int a;

			UINT32 length, size;
			UINT8 *dat;
			const UINT8* jvseeprom_default = NULL;
			int i;

			dat = (UINT8 *)eeprom_get_data_pointer(&length, &size);
			memset(dat, 0, length * size);

			// mainboard eeprom?
			eeprom_set_data((UINT8 *)"\011\241                              0000000000000000", 48);  // 2*checksum 30*unknown 16*serial

			// some games require defaults to boot (vertical, 1 player etc.)
			for (i=0; i<ARRAY_LENGTH(jvseeprom_default_game); i++)
				if (!strcmp(machine->gamedrv->name, jvseeprom_default_game[i].name))
				{
					jvseeprom_default = jvseeprom_default_game[i].eeprom;
					break;
				}

			if (jvseeprom_default)
				memcpy(maple0x86data1, jvseeprom_default, 0x80);

		#if 0
			/* JVS 'eeprom' */
			for (a=0;a < 0x80;a++)
				maple0x86data1[a]=0x11+a;

			// checksums
			maple0x86data1[0]=0xb9;
			maple0x86data1[1]=0xb1;
			maple0x86data1[18]=0xb8;
			maple0x86data1[19]=0x8a;
		#endif
		}
	}
}

static READ64_HANDLER( eeprom_93c46a_r )
{
	int res;

	/* bit 3 is EEPROM data */
	res = eeprom_read_bit() << 4;
	return res;
}

static WRITE64_HANDLER( eeprom_93c46a_w )
{
	/* bit 4 is data */
	/* bit 2 is clock */
	/* bit 5 is cs */
	eeprom_write_bit(data & 0x8);
	eeprom_set_cs_line((data & 0x20) ? CLEAR_LINE : ASSERT_LINE);
	eeprom_set_clock_line((data & 0x4) ? ASSERT_LINE : CLEAR_LINE);
}

/* Dreamcast MAP

0 0x00000000 - 0x001FFFFF MPX System/Boot ROM
0 0x00200000 - 0x0021FFFF Flash Memory
0 0x00400000 - 0x005F67FF Unassigned
0 0x005F6800 - 0x005F69FF System Control Reg.
0 0x005F6C00 - 0x005F6CFF Maple i/f Control Reg.
0 0x005F7000 - 0x005F70FF GD-ROM
0 0x005F7400 - 0x005F74FF G1 i/f Control Reg.
0 0x005F7800 - 0x005F78FF G2 i/f Control Reg.
0 0x005F7C00 - 0x005F7CFF PVR i/f Control Reg.
0 0x005F8000 - 0x005F9FFF TA / PVR Core Reg.
0 0x00600000 - 0x006007FF MODEM
0 0x00600800 - 0x006FFFFF G2 (Reserved)
0 0x00700000 - 0x00707FFF AICA- Sound Cntr. Reg.
0 0x00710000 - 0x0071000B AICA- RTC Cntr. Reg.
0 0x00800000 - 0x00FFFFFF AICA- Wave Memory
0 0x01000000 - 0x01FFFFFF Ext. Device
0 0x02000000 - 0x03FFFFFF Image Area (Mirror Area)

1 0x04000000 - 0x04FFFFFF MPX Tex.Mem. 64bit Acc.
1 0x05000000 - 0x05FFFFFF Tex.Mem. 32bit Acc.
1 0x06000000 - 0x07FFFFFF Image Area*

2 0x08000000 - 0x0BFFFFFF Unassigned

3 0x0C000000 - 0x0CFFFFFF System Memory
3 0x0D000000 - 0x0DFFFFFF (Mirror on DC, Extra RAM on Naomi)

3 0x0E000000 - 0x0FFFFFFF Image Area (Mirror Area)

4 0x10000000 - 0x107FFFFF MPX TA FIFO Polygon Cnv.
4 0x10800000 - 0x10FFFFFF TA FIFO YUV Conv.
4 0x11000000 - 0x11FFFFFF Tex.Mem. 32/64bit Acc.
4 0x12000000 - 0x13FFFFFF Image Area (Mirror Area)

5 0x14000000 - 0x17FFFFFF MPX Ext.

6 0x18000000 - 0x1BFFFFFF Unassigned

7 0x1C000000 - 0x1FFFFFFF(SH4 Internal area)

*/

/*
 * Common address map for Naomi 1, Naomi GD-Rom, Naomi 2, Atomiswave ...
 */

 // SB_LMMODE0
 static WRITE64_HANDLER( ta_texture_directpath0_w )
 {
	int mode = pvrctrl_regs[SB_LMMODE0]&1;
	if (mode&1)
	{
		printf("ta_texture_directpath0_w 32-bit access!\n");
		COMBINE_DATA(&dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&dc_texture_ram[offset]);
	}
 }

 // SB_LMMODE1
 static WRITE64_HANDLER( ta_texture_directpath1_w )
 {
 	int mode = pvrctrl_regs[SB_LMMODE1]&1;
	if (mode&1)
	{
		printf("ta_texture_directpath0_w 32-bit access!\n");
		COMBINE_DATA(&dc_framebuffer_ram[offset]);
	}
	else
	{
		COMBINE_DATA(&dc_texture_ram[offset]);
	}
 }


/*
 * Naomi 1 address map
 */

static ADDRESS_MAP_START( naomi_map, ADDRESS_SPACE_PROGRAM, 64 )
	/* Area 0 */
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_SHARE(3) AM_REGION("maincpu", 0) // BIOS
	AM_RANGE(0xa0000000, 0xa01fffff) AM_ROM AM_SHARE(3)  // non cachable access to  0x00000000 - 0x001fffff

	AM_RANGE(0x00200000, 0x00207fff) AM_RAM                                             // bios uses it (battery backed ram ?)
	AM_RANGE(0x005f6800, 0x005f69ff) AM_READWRITE( dc_sysctrl_r, dc_sysctrl_w )
	AM_RANGE(0x005f6c00, 0x005f6cff) AM_READWRITE( naomi_maple_r, naomi_maple_w )
	AM_RANGE(0x005f7000, 0x005f70ff) AM_DEVREADWRITE("rom_board", naomibd_r, naomibd_w)
	AM_RANGE(0x005f7400, 0x005f74ff) AM_READWRITE( dc_g1_ctrl_r, dc_g1_ctrl_w )
	AM_RANGE(0x005f7800, 0x005f78ff) AM_READWRITE( dc_g2_ctrl_r, dc_g2_ctrl_w )
	AM_RANGE(0x005f7c00, 0x005f7cff) AM_READWRITE( pvr_ctrl_r, pvr_ctrl_w )
	AM_RANGE(0x005f8000, 0x005f9fff) AM_READWRITE( pvr_ta_r, pvr_ta_w )
	AM_RANGE(0x00600000, 0x006007ff) AM_READWRITE( dc_modem_r, dc_modem_w )
	AM_RANGE(0x00700000, 0x00707fff) AM_DEVREADWRITE( "aica", dc_aica_reg_r, dc_aica_reg_w )
	AM_RANGE(0x00710000, 0x0071000f) AM_READWRITE( dc_rtc_r, dc_rtc_w )
	AM_RANGE(0x00800000, 0x00ffffff) AM_READWRITE( naomi_arm_r, naomi_arm_w )           // sound RAM (8 MB)


	AM_RANGE(0x0103ff00, 0x0103ffff) AM_READWRITE( naomi_unknown1_r, naomi_unknown1_w ) // bios uses it, actual start and end addresses not known

	AM_RANGE(0x03010098, 0x0301009f) AM_RAM	// Naomi 2 BIOS tests this, needs to read back as written

	/* Area 1 */
	AM_RANGE(0x04000000, 0x04ffffff) AM_RAM	AM_BASE( &dc_texture_ram )      // texture memory 64 bit access
	AM_RANGE(0x05000000, 0x05ffffff) AM_RAM AM_BASE( &dc_framebuffer_ram ) // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 2*/
	AM_RANGE(0x08000000, 0x0bffffff) AM_NOP // 'Unassigned'

	/* Area 3 */
	AM_RANGE(0x0c000000, 0x0dffffff) AM_RAM AM_BASE(&naomi_ram64) AM_SHARE(4)
	AM_RANGE(0x0e000000, 0x0fffffff) AM_RAM AM_SHARE(4)// mirror

	AM_RANGE(0x8c000000, 0x8dffffff) AM_RAM AM_SHARE(4) // RAM access through cache

	/* Area 4 */
	AM_RANGE(0x10000000, 0x107fffff) AM_WRITE( ta_fifo_poly_w )
	AM_RANGE(0x10800000, 0x10ffffff) AM_WRITE( ta_fifo_yuv_w )
	AM_RANGE(0x11000000, 0x11ffffff) AM_WRITE( ta_texture_directpath0_w ) // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue
	/*       0x12000000 -0x13ffffff Mirror area of  0x10000000 -0x11ffffff */
	AM_RANGE(0x12000000, 0x127fffff) AM_WRITE( ta_fifo_poly_w )
	AM_RANGE(0x12800000, 0x12ffffff) AM_WRITE( ta_fifo_yuv_w )
	AM_RANGE(0x13000000, 0x13ffffff) AM_WRITE( ta_texture_directpath1_w )  // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue

	/* Area 5 */
	//AM_RANGE(0x14000000, 0x17ffffff) AM_NOP // MPX Ext.

	/* Area 6 */
	//AM_RANGE(0x18000000, 0x1bffffff) AM_NOP // Unassigned

	/* Area 7 */
	//AM_RANGE(0x1c000000, 0x1fffffff) AM_NOP // SH4 Internal
ADDRESS_MAP_END


static ADDRESS_MAP_START( naomi_port, ADDRESS_SPACE_IO, 64 )
	AM_RANGE(0x00, 0x0f) AM_READWRITE(eeprom_93c46a_r, eeprom_93c46a_w)
ADDRESS_MAP_END

/*
 * Atomiswave address map, almost identical to Dreamcast
 */

static READ64_HANDLER( aw_flash_r )
{
	return (UINT64)intelflash_read(0, offset*8) | (UINT64)intelflash_read(0, (offset*8)+1)<<8 | (UINT64)intelflash_read(0, (offset*8)+2)<<16 | (UINT64)intelflash_read(0, (offset*8)+3)<<24 |
	       (UINT64)intelflash_read(0, (offset*8)+4)<<32 | (UINT64)intelflash_read(0, (offset*8)+5)<<40 | (UINT64)intelflash_read(0, (offset*8)+6)<<48 | (UINT64)intelflash_read(0, (offset*8)+7)<<56;
}

static WRITE64_HANDLER( aw_flash_w )
{
	int i;
	UINT32 addr = offset * 8;

	for (i = 0; i < 8; i++)
	{
		if (mem_mask & ((UINT64)0xff)<< (i*8))
		{
			addr += i;
			break;
		}
	}

	data >>= (i*8);

	intelflash_write(0, addr, data);
}

INLINE int decode_reg32_64(running_machine *machine, UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		mame_printf_verbose("%s:Wrong mask!\n", cpuexec_describe_context(machine));
//      debugger_break(machine);
	}

	if (mem_mask == U64(0xffffffff00000000))
	{
		reg++;
		*shift = 32;
 	}

	return reg;
}

static READ64_HANDLER( aw_modem_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);

	if (reg == 0x280/4)
	{
		UINT32 coins = input_port_read(space->machine, "COINS");

		if (coins & 0x01)
		{
			return U64(0xffffffff00000002);	// coin A
		}
		else if (coins & 0x02)
		{
			return U64(0xffffffff00000001);	// coin B
		}

		return U64(0xffffffffffffffff);
	}

	mame_printf_verbose("MODEM:  Unmapped read %08x\n", 0x600000+reg*4);
	return 0;
}

static WRITE64_HANDLER( aw_modem_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg32_64(space->machine, offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	mame_printf_verbose("MODEM: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x600000+reg*4, dat, data, offset, mem_mask);
}

static ADDRESS_MAP_START( aw_map, ADDRESS_SPACE_PROGRAM, 64 )
	/* Area 0 */
	AM_RANGE(0x00000000, 0x0001ffff) AM_READWRITE( aw_flash_r, aw_flash_w )
	AM_RANGE(0xa0000000, 0xa001ffff) AM_READWRITE( aw_flash_r, aw_flash_w )

	AM_RANGE(0x00200000, 0x0021ffff) AM_RAM 	// battery backed up RAM
	AM_RANGE(0x005f6800, 0x005f69ff) AM_READWRITE( dc_sysctrl_r, dc_sysctrl_w )
	AM_RANGE(0x005f6c00, 0x005f6cff) AM_READWRITE( dc_maple_r, dc_maple_w )
	AM_RANGE(0x005f7000, 0x005f70ff) AM_DEVREADWRITE("rom_board", naomibd_r, naomibd_w)
	AM_RANGE(0x005f7400, 0x005f74ff) AM_READWRITE( dc_g1_ctrl_r, dc_g1_ctrl_w )
	AM_RANGE(0x005f7800, 0x005f78ff) AM_READWRITE( dc_g2_ctrl_r, dc_g2_ctrl_w )
	AM_RANGE(0x005f7c00, 0x005f7cff) AM_READWRITE( pvr_ctrl_r, pvr_ctrl_w )
	AM_RANGE(0x005f8000, 0x005f9fff) AM_READWRITE( pvr_ta_r, pvr_ta_w )
	AM_RANGE(0x00600000, 0x006007ff) AM_READWRITE( aw_modem_r, aw_modem_w )
	AM_RANGE(0x00700000, 0x00707fff) AM_DEVREADWRITE( "aica", dc_aica_reg_r, dc_aica_reg_w )
	AM_RANGE(0x00710000, 0x0071000f) AM_READWRITE( dc_rtc_r, dc_rtc_w )
	AM_RANGE(0x00800000, 0x00ffffff) AM_READWRITE( naomi_arm_r, naomi_arm_w )           // sound RAM (8 MB)


	AM_RANGE(0x0103ff00, 0x0103ffff) AM_READWRITE( naomi_unknown1_r, naomi_unknown1_w ) // bios uses it, actual start and end addresses not known

	/* Area 1 - half the texture memory, like dreamcast, not naomi */
	AM_RANGE(0x04000000, 0x047fffff) AM_RAM	AM_MIRROR(0x00800000) AM_BASE( &dc_texture_ram )      // texture memory 64 bit access
	AM_RANGE(0x05000000, 0x057fffff) AM_RAM AM_MIRROR(0x00800000) AM_BASE( &dc_framebuffer_ram ) // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 2*/
	AM_RANGE(0x08000000, 0x0bffffff) AM_NOP // 'Unassigned'

	/* Area 3 */
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM AM_BASE(&naomi_ram64) AM_SHARE(4)
	AM_RANGE(0x0d000000, 0x0dffffff) AM_RAM AM_SHARE(4)// extra ram on Naomi (mirror on DC)
	AM_RANGE(0x0e000000, 0x0effffff) AM_RAM AM_SHARE(4)// mirror
	AM_RANGE(0x0f000000, 0x0fffffff) AM_RAM AM_SHARE(4)// mirror

	AM_RANGE(0x8c000000, 0x8cffffff) AM_RAM AM_SHARE(4) // RAM access through cache
	AM_RANGE(0x8d000000, 0x8dffffff) AM_RAM AM_SHARE(4) // RAM access through cache

	/* Area 4 - half the texture memory, like dreamcast, not naomi */
	AM_RANGE(0x10000000, 0x107fffff) AM_WRITE( ta_fifo_poly_w )
	AM_RANGE(0x10800000, 0x10ffffff) AM_WRITE( ta_fifo_yuv_w )
	AM_RANGE(0x11000000, 0x117fffff) AM_WRITE( ta_texture_directpath0_w ) AM_MIRROR(0x00800000)  // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue
	/*       0x12000000 -0x13ffffff Mirror area of  0x10000000 -0x11ffffff */
	AM_RANGE(0x12000000, 0x127fffff) AM_WRITE( ta_fifo_poly_w )
	AM_RANGE(0x12800000, 0x12ffffff) AM_WRITE( ta_fifo_yuv_w )
	AM_RANGE(0x13000000, 0x137fffff) AM_WRITE( ta_texture_directpath1_w ) AM_MIRROR(0x00800000) // access to texture / fraembfufer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue


	/* Area 5 */
	//AM_RANGE(0x14000000, 0x17ffffff) AM_NOP // MPX Ext.

	/* Area 6 */
	//AM_RANGE(0x18000000, 0x1bffffff) AM_NOP // Unassigned

	/* Area 7 */
	//AM_RANGE(0x1c000000, 0x1fffffff) AM_NOP // SH4 Internal
ADDRESS_MAP_END

/*
 * Aica
 */
static void aica_irq(const device_config *device, int irq)
{
	cputag_set_input_line(device->machine, "soundcpu", ARM7_FIRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
}


static const aica_interface aica_config =
{
	TRUE,
	0,
	aica_irq
};



static ADDRESS_MAP_START( dc_audio_map, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM	AM_BASE( &dc_sound_ram )                /* shared with SH-4 */
	AM_RANGE(0x00800000, 0x00807fff) AM_DEVREADWRITE("aica", dc_arm_aica_r, dc_arm_aica_w)
ADDRESS_MAP_END

/*
* Input ports
*/

#define NAOMI_MAME_DEBUG_DIP \
	PORT_START("MAMEDEBUG") \
	PORT_DIPNAME( 0x01, 0x00, "Bilinear Filtering" ) \
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( On ) ) \


/* for now we hardwire a joystick + 6 buttons for every game.*/
static INPUT_PORTS_START( naomi )
	PORT_START("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_START("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_START("IN3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_START("IN4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(dc_coin_slots_callback, &dc_coin_counts[0])
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED(dc_coin_slots_callback, &dc_coin_counts[1])

	NAOMI_MAME_DEBUG_DIP
INPUT_PORTS_END

/* JVS mahjong panel */
static INPUT_PORTS_START( naomi_mp )
	PORT_START("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "SYSB" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x00, "SYSC" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x00, "SYSD" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(dc_coin_slots_callback, &dc_coin_counts[0])
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED(dc_coin_slots_callback, &dc_coin_counts[1])

	NAOMI_MAME_DEBUG_DIP
INPUT_PORTS_END

// Atomiswave - inputs are read as standard Dreamcast controllers/lightguns.
// Controller bit patterns:
//
// SHOT3   (1<<0)
// SHOT2   (1<<1)
// SHOT1   (1<<2)
// START   (1<<3)
// UP      (1<<4)
// DOWN    (1<<5)
// LEFT    (1<<6)
// RIGHT   (1<<7)
// SHOT5   (1<<9)
// SHOT4   (1<<10)
// SERVICE (1<<13)
// TEST    (1<<14)
// COIN    (1<<15)

static INPUT_PORTS_START( aw )
	PORT_START("P1L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_START("P1H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)

	PORT_START("P2L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_START("P2H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)

	PORT_START("P3L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_START("P3H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3)

	PORT_START("P4L")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_START("P4H")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(dc_coin_slots_callback, &dc_coin_counts[0])
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED(dc_coin_slots_callback, &dc_coin_counts[1])

	NAOMI_MAME_DEBUG_DIP
INPUT_PORTS_END

static MACHINE_RESET( naomi )
{
	MACHINE_RESET_CALL(dc);
	aica_set_ram_base(devtag_get_device(machine, "aica"), dc_sound_ram, 8*1024*1024);
}

/*
 * Common for Naomi 1, Naomi GD-Rom, Naomi 2, Atomiswave ...
 */

static MACHINE_DRIVER_START( naomi_base )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", SH4, CPU_CLOCK) // SH4!!!
	MDRV_CPU_CONFIG(sh4cpu_config)
	MDRV_CPU_PROGRAM_MAP(naomi_map)
	MDRV_CPU_IO_MAP(naomi_port)
	MDRV_CPU_VBLANK_INT("screen", naomi_vblank)

	MDRV_CPU_ADD("soundcpu", ARM7, ((XTAL_33_8688MHz*2)/3)/8)	// AICA bus clock is 2/3rds * 33.8688.  ARM7 gets 1 bus cycle out of each 8.
	MDRV_CPU_PROGRAM_MAP(dc_audio_map)

	MDRV_MACHINE_START( dc )
	MDRV_MACHINE_RESET( naomi )

	MDRV_NVRAM_HANDLER(naomi_eeproms)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(dc)
	MDRV_VIDEO_UPDATE(dc)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MDRV_SOUND_ADD("aica", AICA, 0)
	MDRV_SOUND_CONFIG(aica_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 2.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_DRIVER_END

/*
 * Naomi 1
 */

static MACHINE_DRIVER_START( naomi )
	MDRV_IMPORT_FROM(naomi_base)
	MDRV_NAOMI_ROM_BOARD_ADD("rom_board", "user1")
MACHINE_DRIVER_END

/*
 * Naomi 1 GD-Rom
 */

static MACHINE_DRIVER_START( naomigd )
	MDRV_IMPORT_FROM(naomi_base)
	MDRV_NAOMI_DIMM_BOARD_ADD("rom_board", "gdrom", "user1", "picreturn")
MACHINE_DRIVER_END

/*
 * Naomi 2
 */

// ...

/*
 * Naomi 2 GD-Rom
 */

// ...

/*
 * Atomiswave
 */

static MACHINE_DRIVER_START( aw )
	MDRV_IMPORT_FROM(naomi_base)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(aw_map)
	MDRV_NVRAM_HANDLER(aw_nvram)
	MDRV_AW_ROM_BOARD_ADD("rom_board", "user1")
MACHINE_DRIVER_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

/* BIOS info:

Revisions A through C can handle game carts only
Revisions D and later can also handle GD-Rom board
Revisions F and later can also handle GD-Rom board and or the network GD-Rom board

F355 has it's own BIOS (3 screen version) and different JVS I/O Board

Info from roms starting at 0x1ffd60

EPR-21576h - NAOMI BOOT ROM 2002 07/08  1.8- (Japan)
EPR-21576g - NAOMI BOOT ROM 2001 09/10  1.70 (Japan)
EPR-21576e - NAOMI BOOT ROM 2000 08/25  1.50 (Japan)
EPR-21576d - NAOMI BOOT ROM 1999 06/04  1.40 (Japan)
EPR-21576b - NAOMI BOOT ROM 1999 02/15  1.20 (Japan)
EPR-21576  - NAOMI BOOT ROM 1998 12/18  1.00 (Japan)
EPR-21577g - NAOMI BOOT ROM 2001 09/10  1.70 (USA)
EPR-21577e - NAOMI BOOT ROM 2000 08/25  1.50 (USA)
EPR-21577d - NAOMI BOOT ROM 1999 06/04  1.40 (USA)
EPR-21578e - NAOMI BOOT ROM 2000 08/25  1.50 (Export)
EPR-21578d - NAOMI BOOT ROM 1999 06/04  1.40 (Export)
EPR-21578b - NAOMI BOOT ROM 1999 02/15  1.20 (Export)
EPR-21579  - No known dump (Korea)
EPR-21580  - No known dump (Australia)
EPR-22851  - NAOMI BOOT ROM 1999 08/30  1.35 (Multisystem 3 screen Ferrari F355)

EPR-21577e & EPR-2178e differ by 7 bytes:

0x53e20 is the region byte (only one region byte)
0x1ffffa-0x1fffff is the BIOS checksum


House of the Dead 2 specific Naomi BIOS roms:

Info from roms starting at 0x1ff060

EPR-21330  - HOUSE OF THE DEAD 2 IPL ROM 1998 11/14 (USA)
EPR-21331  - HOUSE OF THE DEAD 2 IPL ROM 1998 11/14 (Export)

EPR-21330 & EPR-21331 differ by 7 bytes:

0x40000 is the region byte (only one region byte)
0x1ffffa-0x1fffff is the BIOS checksum


Ferrari F355 specific Naomi BIOS roms:

EPR-22850 - NAOMI BOOT ROM 1999 08/30  1.35 (USA)
EPR-22851 - NAOMI BOOT ROM 1999 08/30  1.35 (Export)

EPR-22850 & EPR-22851 differ by 7 bytes:

0x52F08 is the region byte (only one region byte)
0x1ffffa-0x1fffff is the BIOS checksum


Region byte encoding is as follows:

0x00 = Japan
0x01 = USA
0x02 = Export
0x?? = Korea
0x?? = Australia

Scan ROM for the text string "LOADING TEST MODE NOW" back up four (4) bytes for the region byte.
  NOTE: this doesn't work for the HOTD2 or multi screen boot roms

*/
// game specific bios roms quite clearly don't belong in here.
// Japan bios is default, because most games require it.
/* bios e works, the newer japan one gives 'board malfunction' with some games?' */
#define NAOMI_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr-21576e (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-21576e.bin",  0x000000, 0x200000, CRC(08c0add7) SHA1(e7c1a7673cb2ccb21748ef44105e46d1bad7266d) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr-21576g (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-21576g.bin",  0x000000, 0x200000, CRC(d2a1c6bf) SHA1(6d27d71aec4dfba98f66316ae74a1426d567698a) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "epr-21576h (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "epr-21576h.bin",  0x000000, 0x200000, CRC(d4895685) SHA1(91424d481ff99a8d3f4c45cea6d3f0eada049a6d) ) \
	ROM_SYSTEM_BIOS( 3, "bios3", "epr-21576d (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "epr-21576d.bin",  0x000000, 0x200000, CRC(3b2afa7b) SHA1(d007e1d321c198a38c5baff86eb2ab84385d150a) ) \
	ROM_SYSTEM_BIOS( 4, "bios4", "epr-21576b (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "epr-21576b.bin",  0x000000, 0x200000, CRC(755a6e07) SHA1(7e8b8ccfc063144d89668e7224dcd8a36c54f3b3) ) \
	ROM_SYSTEM_BIOS( 5, "bios5", "epr-21576 (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5,  "epr-21576.bin", 0x000000, 0x200000, CRC(9dad3495) SHA1(5fb66f9a2b68d120f059c72758e65d34f461044a) ) \
	ROM_SYSTEM_BIOS( 6, "bios6", "epr-21578e (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6, "epr-21578e.bin",  0x000000, 0x200000, CRC(087f09a3) SHA1(0418eb2cf9766f0b1b874a4e92528779e22c0a4a) ) \
	ROM_SYSTEM_BIOS( 7, "bios7", "epr-21578d (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7, "epr-21578d.bin",  0x000000, 0x200000, CRC(dfd5f42a) SHA1(614a0db4743a5e5a206190d6786ade24325afbfd) ) \
	ROM_SYSTEM_BIOS( 8, "bios8", "epr-21578b (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8, "epr-21578b.bin",  0x000000, 0x200000, CRC(6c9aad83) SHA1(555918de76d8dbee2a97d8a95297ef694b3e803f) ) \
	ROM_SYSTEM_BIOS( 9, "bios9", "epr-21577g (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 9, "epr-21577g.bin",  0x000000, 0x200000, CRC(25f64af7) SHA1(99f9e6cc0642319bd2da492611220540add573e8) ) \
	ROM_SYSTEM_BIOS( 10, "bios10", "epr-21577e (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 10, "epr-21577e.bin",  0x000000, 0x200000, CRC(cf36e97b) SHA1(b085305982e7572e58b03a9d35f17ae319c3bbc6) ) \
	ROM_SYSTEM_BIOS( 11, "bios11", "epr-21577d (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 11, "epr-21577d.bin",  0x000000, 0x200000, CRC(60ddcbbe) SHA1(58b15096d269d6df617ca1810b66b47deb184958) ) \
	ROM_SYSTEM_BIOS( 12, "bios12", "Naomi Dev BIOS" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 12,  "dcnaodev.bios", 0x000000, 0x080000, CRC(7a50fab9) SHA1(ef79f448e0bf735d1264ad4f051d24178822110f) ) /* This one comes from a dev / beta board. The eprom was a 27C4096 */

// bios for House of the Dead 2
#define HOTD2_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "HOTD2 (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "epr-21331.ic27", 0x000000, 0x200000, CRC(065f8500) SHA1(49a3881e8d76f952ef5e887200d77b4a415d47fe) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "HOTD2 (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1,  "epr-21330.ic27", 0x000000, 0x200000, CRC(9e3bfa1b) SHA1(b539d38c767b0551b8e7956c1ff795de8bbe2fbc) ) \

#define F355_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "Ferrari F355 (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "epr-22851.bin", 0x000000, 0x200000, CRC(62483677) SHA1(3e3bcacf5f972c376b569f45307ee7fd0b5031b7) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "Ferrari F355 (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1,  "epr-22850.bin", 0x000000, 0x200000, CRC(28aa539d) SHA1(14485368656af80504b212da620179c49f84c1a2) )


/* only revisions d and higher support the GDROM, and there is an additional bios (and SH4!) on the DIMM board for the CD Controller */
/* bios e works, the newer japan one gives 'board malfunction' with some games?' */
#define NAOMIGD_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr-21576e (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-21576e.bin",  0x000000, 0x200000, CRC(08c0add7) SHA1(e7c1a7673cb2ccb21748ef44105e46d1bad7266d) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr-21576g (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-21576g.bin",  0x000000, 0x200000, CRC(d2a1c6bf) SHA1(6d27d71aec4dfba98f66316ae74a1426d567698a) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "epr-21576h (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "epr-21576h.bin",  0x000000, 0x200000, CRC(d4895685) SHA1(91424d481ff99a8d3f4c45cea6d3f0eada049a6d) ) \
	ROM_SYSTEM_BIOS( 3, "bios3", "epr-21576d (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "epr-21576d.bin",  0x000000, 0x200000, CRC(3b2afa7b) SHA1(d007e1d321c198a38c5baff86eb2ab84385d150a) ) \
	ROM_SYSTEM_BIOS( 4, "bios4", "epr-21578e (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "epr-21578e.bin",  0x000000, 0x200000, CRC(087f09a3) SHA1(0418eb2cf9766f0b1b874a4e92528779e22c0a4a) ) \
	ROM_SYSTEM_BIOS( 5, "bios5", "epr-21578d (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "epr-21578d.bin",  0x000000, 0x200000, CRC(dfd5f42a) SHA1(614a0db4743a5e5a206190d6786ade24325afbfd) ) \
	ROM_SYSTEM_BIOS( 6, "bios6", "epr-21577g (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6, "epr-21577g.bin",  0x000000, 0x200000, CRC(25f64af7) SHA1(99f9e6cc0642319bd2da492611220540add573e8) ) \
	ROM_SYSTEM_BIOS( 7, "bios7", "epr-21577e (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7, "epr-21577e.bin",  0x000000, 0x200000, CRC(cf36e97b) SHA1(b085305982e7572e58b03a9d35f17ae319c3bbc6) ) \
	ROM_SYSTEM_BIOS( 8, "bios8", "epr-21577d (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8, "epr-21577d.bin",  0x000000, 0x200000, CRC(60ddcbbe) SHA1(58b15096d269d6df617ca1810b66b47deb184958) ) \
	ROM_REGION( 0x200000, "user2", 0) \
	ROM_LOAD16_WORD_SWAP( "fpr-23489c.ic14", 0x000000, 0x200000, CRC(bc38bea1) SHA1(b36fcc6902f397d9749e9d02de1bbb7a5e29d468) ) \

/* NAOMI2 BIOS:

EPR-23605  - NAOMI BOOT ROM 2001 01/19  1.50 (Japan)
EPR-23605A - NAOMI BOOT ROM 2001 06/20  1.60 (Japan)
EPR-23605B - NAOMI BOOT ROM 2001 09/10  1.70 (Japan)
EPR-23605C - NAOMI BOOT ROM 2002 07/08  1.8- (Japan)
EPR-23607  - NAOMI BOOT ROM 2001 01/19  1.50 (USA)
EPR-23607B - NAOMI BOOT ROM 2001 09/10  1.70 (USA)
EPR-23608  - NAOMI BOOT ROM 2001 01/19  1.50 (Export)
EPR-23608B - NAOMI BOOT ROM 2001 09/10  1.70 (Export)

EPR-23605B, EPR-23607B & EPR-23608B all differ by 8 bytes:

0x0553a0 is the first region byte
0x1ecf40 is a second region byte (value is the same as the first region byte )
0x1fffa-1ffff is the BIOS rom checksum

Region byte encoding is as follows:

0x00 = Japan
0x01 = USA
0x02 = Export
0x?? = Korea
0x?? = Australia

*/

#define NAOMI2_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr-23605c (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-23605c.bin",   0x000000, 0x200000, CRC(297ea6ed) SHA1(cfbfe57c80e6ee86a101fa83aec0a01e00c0f42a) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr-23605b (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-23605b.bin",   0x000000, 0x200000, CRC(3a3242d4) SHA1(aaca4df51ef91d926f8191d372f3dfe1d20d9484) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "epr-23605a (Japan)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "epr-23605a.bin",   0x000000, 0x200000, CRC(7bc3fc2d) SHA1(a4a9531a7c66ff30046908cf71f6c7b6fb59c392) ) \
	ROM_SYSTEM_BIOS( 3, "bios3", "epr-23605 (Japan)"  ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "epr-23605.bin",    0x000000, 0x200000, CRC(5731e446) SHA1(787b0844fc408cf124c12405c095c59948709ea6) ) \
	ROM_SYSTEM_BIOS( 4, "bios4", "epr-23608b (Export)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "epr-23608b.bin",   0x000000, 0x200000, CRC(a554b1e3) SHA1(343b727a3619d1c75a9b6d4cc156a9050447f155) ) \
	ROM_SYSTEM_BIOS( 5, "bios5", "epr-23608 (Export)"  ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "epr-23608.bin",    0x000000, 0x200000, CRC(929cc3a6) SHA1(47d00c818de23f733a4a33b1bbc72eb8aa729246) ) \
	ROM_SYSTEM_BIOS( 6, "bios6", "epr-23607b (USA)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6, "epr-23607b.bin",   0x000000, 0x200000, CRC(f308c5e9) SHA1(5470ab1cee6afecbd8ca8cf40f8fbe4ec2cb1471) ) \
	ROM_SYSTEM_BIOS( 7, "bios7", "epr-23607 (USA)"  ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7, "epr-23607.bin",    0x000000, 0x200000, CRC(2b55add2) SHA1(547de5f97d3183c8cd069c4fa3c09f13d8b637d9) ) \

/* First half is BIOS, second half is game settings and is blanked/reprogrammed by the BIOS as necessary */
#define AW_BIOS \
	ROM_SYSTEM_BIOS( 0, "bios0", "Atomiswave BIOS" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "bios.ic23_l",                         0x000000, 0x010000, BAD_DUMP CRC(e5693ce3) SHA1(1bde3ed87af64b0f675ebd47f12a53e1fc5709c1) ) /* Might be bad.. especially. bytes 0x0000, 0x6000, 0x8000 which gave different reads */


ROM_START( naomi )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x8400000, "user1", ROMREGION_ERASE)
ROM_END

ROM_START( naomigd )
	NAOMIGD_BIOS

	ROM_REGION( 0x8400000, "user1", ROMREGION_ERASE)
ROM_END

ROM_START( hod2bios )
	ROM_REGION( 0x200000, "maincpu", 0)
	HOTD2_BIOS

	ROM_REGION( 0x8400000, "user1", ROMREGION_ERASE)
ROM_END

ROM_START( f355bios )
	ROM_REGION( 0x200000, "maincpu", 0)
	F355_BIOS

	ROM_REGION( 0x8400000, "user1", ROMREGION_ERASE)
ROM_END

ROM_START( naomi2 )
	NAOMI2_BIOS

	ROM_REGION( 0x8400000, "user1", ROMREGION_ERASE)
ROM_END

ROM_START( awbios )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8400000, "user1", ROMREGION_ERASE)
ROM_END


/**********************************************
 *
 * Naomi Cart ROM defines
 *
 *********************************************/


/* Info above each set is automatically generated from the IC22 rom and may not be accurate */

/*
SYSTEMID: NAOMI
JAP: GUN SPIKE
USA: CANNON SPIKE
EXP: CANNON SPIKE

NO.     Type    Byte    Word
IC22    32M     0000*   0000* invalid value
IC1     64M     7AC6    C534
IC2     64M     3959    6667
IC3     64M     F60D    69E5
IC4     64M     FBD4    AE40
IC5     64M     1717    F3EC
IC6     64M     A622    1D3D
IC7     64M     33A3    4480
IC8     64M     FC26    A49D
IC9     64M     528D    5206
IC10    64M     7C94    8779
IC11    64M     271E    BEF7
IC12    64M     BA24    102F

*/

ROM_START( cspike )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x6800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23210.ic22", 0x0000000, 0x0400000, CRC(a15c54b5) SHA1(5c7872244d3d648e4c04751f120d0e9d47239921) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23198.ic1",  0x0800000, 0x0800000, CRC(ce8d3edf) SHA1(1df5bb4eb440c221b8f1e5f019b02accc235fc28) )
	ROM_LOAD("mpr-23199.ic2",  0x1000000, 0x0800000, CRC(0979392a) SHA1(7dc433da6f3e47a721a2e86720a65d9752248e92) )
	ROM_LOAD("mpr-23200.ic3",  0x1800000, 0x0800000, CRC(e4b2db33) SHA1(063bc3789f68be5fcefeeec9e1c8268feb84b7eb) )
	ROM_LOAD("mpr-23201.ic4",  0x2000000, 0x0800000, CRC(c55ca0fa) SHA1(e6fde606b9ed4fd195da304a7b57e8b7797e368f) )
	ROM_LOAD("mpr-23202.ic5",  0x2800000, 0x0800000, CRC(983bb21c) SHA1(a30f9b09370cceadf11defc85b5acd3e578477e0) )
	ROM_LOAD("mpr-23203.ic6",  0x3000000, 0x0800000, CRC(f61b8d96) SHA1(a3522963b1e13b809818ffe5a209dd4ce087ec38) )
	ROM_LOAD("mpr-23204.ic7",  0x3800000, 0x0800000, CRC(03593ecd) SHA1(5ef3ccbfb7b1cc85ad352b13d70eefcad2b209f6) )
	ROM_LOAD("mpr-23205.ic8",  0x4000000, 0x0800000, CRC(e8c9349b) SHA1(310f02c5dad84e84362f0f674afa405f7d72f8ce) )
	ROM_LOAD("mpr-23206.ic9",  0x4800000, 0x0800000, CRC(8089d80f) SHA1(821f5f24616920bf0ed4c86597c27f6a3c39b8e6) )
	ROM_LOAD("mpr-23207.ic10", 0x5000000, 0x0800000, CRC(39f692a1) SHA1(14bc86b48a995378b4dd3609d38b90cddf2d7483) )
	ROM_LOAD("mpr-23208.ic11", 0x5800000, 0x0800000, CRC(b9494f4b) SHA1(2f35b25edf5210a82d4b67e639eeae11440d065a) )
	ROM_LOAD("mpr-23209.ic12s",0x6000000, 0x0800000, CRC(560188c0) SHA1(77f14c9a031c6e5414ffa854d20c40115361d715) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: CAPCOM VS SNK  JAPAN
USA: CAPCOM VS SNK  USA
EXP: CAPCOM VS SNK  EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     B836    4AA4
IC2     64M     19C1    9965
IC3     64M     B98C    EFB2
IC4     64M     2458    31CD
IC5     64M     59D2    E957
IC6     64M     1004    7E0B
IC7     64M     C63F    B2A7
IC8     64M     9D78    342F
IC9     64M     681F    D97A
IC10    64M     7544    E4D3
IC11    64M     8351    8A4C
IC12    64M     B713    2408
IC13    64M     A12E    8DE4

*/

// ver 000904
ROM_START( capsnk )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x7800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD( "epr-23511.ic22", 0x000000, 0x400000, CRC(3dbf8eb2) SHA1(1f7b89ba99e018cc85022fa852d56d4e345e1bd2) )
        ROM_LOAD( "mpr-23504.ic1", 0x0800000, 0x1000000, CRC(e01a31d2) SHA1(e00e138f6a20175c7aadb6500f6d7541b91def14) )
        ROM_LOAD( "mpr-23505.ic2", 0x1800000, 0x1000000, CRC(3a34d5fe) SHA1(f3c5f6fcbaa7004d371923eb412ea1fcf3fa461a) )
        ROM_LOAD( "mpr-23506.ic3", 0x2800000, 0x1000000, CRC(9cbab27d) SHA1(f166352355a03c9ccafbc15f926330b3622ec040) )
        ROM_LOAD( "mpr-23507.ic4", 0x3800000, 0x1000000, CRC(363c1734) SHA1(16b0485f1aacc8925b3c6d6152680139748e6df8) )
        ROM_LOAD( "mpr-23508.ic5", 0x4800000, 0x1000000, CRC(0a3590aa) SHA1(84c0e1853f069b003d09b268caee97e58c4dacb6) )
        ROM_LOAD( "mpr-23509.ic6", 0x5800000, 0x1000000, CRC(281d633d) SHA1(d773be8e95f7bf9212ee1061f3076220d4fce9e0) )
        ROM_LOAD( "mpr-23510.ic7", 0x6800000, 0x1000000, CRC(b856fef5) SHA1(0634f86740c438b40286256a0269570d24cb845a) )

	// trojaned protection data (filename is address read from)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "a63a9260.bin", 0x000000, 0x008760, CRC(54efa963) SHA1(41874253567da56441d83de041f6e82111977fa4) )
        ROM_LOAD( "a63b19c0.bin", 0x010000, 0x008260, CRC(f0ca72ef) SHA1(87fc4654afaf763cdb3ea5b4501948bfe7b9e9dc) )
        ROM_LOAD( "a63b9c20.bin", 0x020000, 0x00a0c0, CRC(92eec22c) SHA1(c7267503a828e852edeb26f68f3f4b685b9b163e) )
        ROM_LOAD( "a63c3ce0.bin", 0x030000, 0x00d7e0, CRC(93201f32) SHA1(5aca43776306a5a5ecd68a7e8699a724703f329a) )
        ROM_LOAD( "a63d14c0.bin", 0x040000, 0x00a8e0, CRC(fb2e9f7b) SHA1(ae4fc9a82eafb5b511485eef1d65ee9d3adc27b6) )
        ROM_LOAD( "a63dbda0.bin", 0x050000, 0x008460, CRC(b4b74624) SHA1(6ae2f5d69e45d295a3d0dcf295b1fb76be201e8e) )
        ROM_LOAD( "a63e4200.bin", 0x060000, 0x00b500, CRC(69ee516d) SHA1(f358aff12b826d1685199a9d5950abd121b54192) )
        ROM_LOAD( "a63ef700.bin", 0x070000, 0x00cec0, CRC(b2f4df7e) SHA1(4aafcf65e3fc193ee191f06ffcce37e52fc12364) )
        ROM_LOAD( "a63fc5c0.bin", 0x080000, 0x004cfc, CRC(1d534f98) SHA1(32afbb1d1fd1abba9e49b7b4bcf310860e4e47dc) )
        ROM_LOAD( "a643a120.bin", 0x090000, 0x00d6e0, CRC(27a0deb7) SHA1(c8da2e212ca8f5985d9336e8200c3382d6d373a5) )
        ROM_LOAD( "a6447800.bin", 0x0a0000, 0x00c920, CRC(42c9e4b1) SHA1(d7ef0d23956b795fad888f49a4dc42d74899de71) )
        ROM_LOAD( "a6454120.bin", 0x0b0000, 0x00a540, CRC(69321860) SHA1(675767e9fd18a0d72de2decb68acd7d500b11de2) )
        ROM_LOAD( "a645e660.bin", 0x0c0000, 0x00ccc0, CRC(4919083d) SHA1(89dfc2837e5298dc691350ac470bc76557657736) )
        ROM_LOAD( "a646b320.bin", 0x0d0000, 0x00c120, CRC(a66da245) SHA1(48d543244cdc52887131e734e0643c69f9368681) )
        ROM_LOAD( "a6477440.bin", 0x0e0000, 0x0097a8, CRC(6289f812) SHA1(574459f7c79dd757ae94f8e06dbcefa5b203e510) )
        ROM_LOAD( "a6480c00.bin", 0x0f0000, 0x00eca0, CRC(a4128a65) SHA1(35625df0109a72cb508bb4a89641bcb86aa697fb) )
        ROM_LOAD( "a648f8a0.bin", 0x100000, 0x009520, CRC(45a06ed6) SHA1(e9ee99e329cf35a95440312b9d0bed40254481c2) )
        ROM_LOAD( "a6498dc0.bin", 0x110000, 0x009f40, CRC(2cea7b46) SHA1(5398b8f1ba01e21d5fee0b1c6a796ce396c9697f) )
        ROM_LOAD( "a64a2d00.bin", 0x120000, 0x00dbe0, CRC(9c75fc98) SHA1(aa1a630c77b29202a4045b1eade455f02f29b81a) )
        ROM_LOAD( "a64b08e0.bin", 0x130000, 0x00cb40, CRC(72ae0dd2) SHA1(f4e8face14dac9a5cc75c6ccdfb9911929e2d76e) )
        ROM_LOAD( "a64bd420.bin", 0x140000, 0x00e920, CRC(198e4aa9) SHA1(294b9c3f89b138898c3ef6db60ac5473516d74dc) )
        ROM_LOAD( "a64cbd40.bin", 0x150000, 0x005e94, CRC(cc4a4e58) SHA1(dce67374a8dc6d2c9b66c0cd7e68b4f9fdab3e27) )
        ROM_LOAD( "a6617be0.bin", 0x160000, 0x00e480, CRC(b7d6ce86) SHA1(c3d9a8cf6125c50a0129591d807bbb13a836a651) )
        ROM_LOAD( "a6626060.bin", 0x170000, 0x0078a8, CRC(8c258ad2) SHA1(51d4d227e037992905a7a7f2b5bb13646920670d) )
ROM_END

// ver 000804
ROM_START( capsnka )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x7800000, "user1", ROMREGION_ERASEFF)
        ROM_LOAD( "epr23511a.ic22", 0x000000, 0x400000, CRC(fe00650f) SHA1(ca8e9e9178ed2b6598bdea83be1bf0dd7aa509f9) )
        ROM_LOAD( "mpr-23504.ic1", 0x0800000, 0x1000000, CRC(e01a31d2) SHA1(e00e138f6a20175c7aadb6500f6d7541b91def14) )
        ROM_LOAD( "mpr-23505.ic2", 0x1800000, 0x1000000, CRC(3a34d5fe) SHA1(f3c5f6fcbaa7004d371923eb412ea1fcf3fa461a) )
        ROM_LOAD( "mpr-23506.ic3", 0x2800000, 0x1000000, CRC(9cbab27d) SHA1(f166352355a03c9ccafbc15f926330b3622ec040) )
        ROM_LOAD( "mpr-23507.ic4", 0x3800000, 0x1000000, CRC(363c1734) SHA1(16b0485f1aacc8925b3c6d6152680139748e6df8) )
        ROM_LOAD( "mpr-23508.ic5", 0x4800000, 0x1000000, CRC(0a3590aa) SHA1(84c0e1853f069b003d09b268caee97e58c4dacb6) )
        ROM_LOAD( "mpr-23509.ic6", 0x5800000, 0x1000000, CRC(281d633d) SHA1(d773be8e95f7bf9212ee1061f3076220d4fce9e0) )
        ROM_LOAD( "mpr-23510.ic7", 0x6800000, 0x1000000, CRC(b856fef5) SHA1(0634f86740c438b40286256a0269570d24cb845a) )

	// trojaned protection data (filename is address read from)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "a63a9260.bin", 0x000000, 0x008760, CRC(54efa963) SHA1(41874253567da56441d83de041f6e82111977fa4) )
        ROM_LOAD( "a63b19c0.bin", 0x010000, 0x008260, CRC(f0ca72ef) SHA1(87fc4654afaf763cdb3ea5b4501948bfe7b9e9dc) )
        ROM_LOAD( "a63b9c20.bin", 0x020000, 0x00a0c0, CRC(92eec22c) SHA1(c7267503a828e852edeb26f68f3f4b685b9b163e) )
        ROM_LOAD( "a63c3ce0.bin", 0x030000, 0x00d7e0, CRC(93201f32) SHA1(5aca43776306a5a5ecd68a7e8699a724703f329a) )
        ROM_LOAD( "a63d14c0.bin", 0x040000, 0x00a8e0, CRC(fb2e9f7b) SHA1(ae4fc9a82eafb5b511485eef1d65ee9d3adc27b6) )
        ROM_LOAD( "a63dbda0.bin", 0x050000, 0x008460, CRC(b4b74624) SHA1(6ae2f5d69e45d295a3d0dcf295b1fb76be201e8e) )
        ROM_LOAD( "a63e4200.bin", 0x060000, 0x00b500, CRC(69ee516d) SHA1(f358aff12b826d1685199a9d5950abd121b54192) )
        ROM_LOAD( "a63ef700.bin", 0x070000, 0x00cec0, CRC(b2f4df7e) SHA1(4aafcf65e3fc193ee191f06ffcce37e52fc12364) )
        ROM_LOAD( "a63fc5c0.bin", 0x080000, 0x004cfc, CRC(1d534f98) SHA1(32afbb1d1fd1abba9e49b7b4bcf310860e4e47dc) )
        ROM_LOAD( "a643a120.bin", 0x090000, 0x00d6e0, CRC(27a0deb7) SHA1(c8da2e212ca8f5985d9336e8200c3382d6d373a5) )
        ROM_LOAD( "a6447800.bin", 0x0a0000, 0x00c920, CRC(42c9e4b1) SHA1(d7ef0d23956b795fad888f49a4dc42d74899de71) )
        ROM_LOAD( "a6454120.bin", 0x0b0000, 0x00a540, CRC(69321860) SHA1(675767e9fd18a0d72de2decb68acd7d500b11de2) )
        ROM_LOAD( "a645e660.bin", 0x0c0000, 0x00ccc0, CRC(4919083d) SHA1(89dfc2837e5298dc691350ac470bc76557657736) )
        ROM_LOAD( "a646b320.bin", 0x0d0000, 0x00c120, CRC(a66da245) SHA1(48d543244cdc52887131e734e0643c69f9368681) )
        ROM_LOAD( "a6477440.bin", 0x0e0000, 0x0097a8, CRC(6289f812) SHA1(574459f7c79dd757ae94f8e06dbcefa5b203e510) )
        ROM_LOAD( "a6480c00.bin", 0x0f0000, 0x00eca0, CRC(a4128a65) SHA1(35625df0109a72cb508bb4a89641bcb86aa697fb) )
        ROM_LOAD( "a648f8a0.bin", 0x100000, 0x009520, CRC(45a06ed6) SHA1(e9ee99e329cf35a95440312b9d0bed40254481c2) )
        ROM_LOAD( "a6498dc0.bin", 0x110000, 0x009f40, CRC(2cea7b46) SHA1(5398b8f1ba01e21d5fee0b1c6a796ce396c9697f) )
        ROM_LOAD( "a64a2d00.bin", 0x120000, 0x00dbe0, CRC(9c75fc98) SHA1(aa1a630c77b29202a4045b1eade455f02f29b81a) )
        ROM_LOAD( "a64b08e0.bin", 0x130000, 0x00cb40, CRC(72ae0dd2) SHA1(f4e8face14dac9a5cc75c6ccdfb9911929e2d76e) )
        ROM_LOAD( "a64bd420.bin", 0x140000, 0x00e920, CRC(198e4aa9) SHA1(294b9c3f89b138898c3ef6db60ac5473516d74dc) )
        ROM_LOAD( "a64cbd40.bin", 0x150000, 0x005e94, CRC(cc4a4e58) SHA1(dce67374a8dc6d2c9b66c0cd7e68b4f9fdab3e27) )
        ROM_LOAD( "a6617be0.bin", 0x160000, 0x00e480, CRC(b7d6ce86) SHA1(c3d9a8cf6125c50a0129591d807bbb13a836a651) )
        ROM_LOAD( "a6626060.bin", 0x170000, 0x0078a8, CRC(8c258ad2) SHA1(51d4d227e037992905a7a7f2b5bb13646920670d) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: COSMIC SMASH IN JAPAN
USA: COSMIC SMASH IN USA
EXP: COSMIC SMASH IN EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000     EPR23428A.22
IC1     64M     C82B    E769     MPR23420.1
IC2     64M     E0C3    43B6     MPR23421.2
IC3     64M     C896    F766     MPR23422.3
IC4     64M     2E60    4CBF     MPR23423.4
IC5     64M     BB81    7E26     MPR23424.5
IC6     64M     B3A8    F2EA     MPR23425.6
IC7     64M     05C5    A084     MPR23426.7
?IC8     64M     9E13    7535     MPR23427.8

Serial: BCHE-01A0803

*/

ROM_START( csmash )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x4800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23428a.ic22", 0x0000000, 0x400000, CRC(d628dbce) SHA1(91ec1296ead572a64c37f8ac2c1a96742f19d50b) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23420.ic1",   0x0800000, 0x0800000, CRC(9d5991f2) SHA1(c75871db314b01935d1daaacf1a762e73e5fd411) )
	ROM_LOAD("mpr-23421.ic2",   0x1000000, 0x0800000, CRC(6c351db3) SHA1(cdd601321a38fc34152517abdc473b73a4c6f630) )
	ROM_LOAD("mpr-23422.ic3",   0x1800000, 0x0800000, CRC(a1d4bd29) SHA1(6c446fd1819f55412351f15cf57b769c0c56c1db) )
	ROM_LOAD("mpr-23423.ic4",   0x2000000, 0x0800000, CRC(08cbf373) SHA1(0d9a593f5cc5d632d85d7253c135eef2e8e01598) )
	ROM_LOAD("mpr-23424.ic5",   0x2800000, 0x0800000, CRC(f4404000) SHA1(e49d941e47e63bb7f3fddc3c3d2c1653611914ee) )
	ROM_LOAD("mpr-23425.ic6",   0x3000000, 0x0800000, CRC(47f51da2) SHA1(af5ecd460114caed3a00157ffd3a2df0fbf348c0) )
	ROM_LOAD("mpr-23426.ic7",   0x3800000, 0x0800000, CRC(7f91b13f) SHA1(2d534f77291ebfedc011bf0e803a1b9243fb477f) )
	ROM_LOAD("mpr-23427.ic8",   0x4000000, 0x0800000, CRC(5851d525) SHA1(1cb1073542d75a3bcc0d363ed31d49bcaf1fd494) )

	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
	ROM_COPY("user1", 0, 0, 0x200000)
ROM_END

ROM_START( csmasho )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x4800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23428.ic22", 0x0000000, 0x400000, CRC(f8597496) SHA1(2bb9f25b63b7410934ae4b1e052e1308a5c5a57f) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23420.ic1", 0x0800000, 0x0800000, CRC(9d5991f2) SHA1(c75871db314b01935d1daaacf1a762e73e5fd411) )
	ROM_LOAD("mpr-23421.ic2", 0x1000000, 0x0800000, CRC(6c351db3) SHA1(cdd601321a38fc34152517abdc473b73a4c6f630) )
	ROM_LOAD("mpr-23422.ic3", 0x1800000, 0x0800000, CRC(a1d4bd29) SHA1(6c446fd1819f55412351f15cf57b769c0c56c1db) )
	ROM_LOAD("mpr-23423.ic4", 0x2000000, 0x0800000, CRC(08cbf373) SHA1(0d9a593f5cc5d632d85d7253c135eef2e8e01598) )
	ROM_LOAD("mpr-23424.ic5", 0x2800000, 0x0800000, CRC(f4404000) SHA1(e49d941e47e63bb7f3fddc3c3d2c1653611914ee) )
	ROM_LOAD("mpr-23425.ic6", 0x3000000, 0x0800000, CRC(47f51da2) SHA1(af5ecd460114caed3a00157ffd3a2df0fbf348c0) )
	ROM_LOAD("mpr-23426.ic7", 0x3800000, 0x0800000, CRC(7f91b13f) SHA1(2d534f77291ebfedc011bf0e803a1b9243fb477f) )
	ROM_LOAD("mpr-23427.ic8", 0x4000000, 0x0800000, CRC(5851d525) SHA1(1cb1073542d75a3bcc0d363ed31d49bcaf1fd494) )

	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
	ROM_COPY("user1", 0, 0, 0x200000)
ROM_END

/*

SYSTEMID: NAOMI
JAP: DEATH CRIMSON OX
USA: DEATH CRIMSON OX
EXP: DEATH CRIMSON OX

*/

ROM_START( deathcox )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x5800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23524.ic22",0x0000000, 0x0400000, CRC(edc20e44) SHA1(6167ee86624f5b78b3ced0dd82259e83053f4f9d) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23514.ic1", 0x0800000, 0x0800000, CRC(1f2b090e) SHA1(f2863d306512112cd3025c9ce3300ac0a396ee2d) )
	ROM_LOAD("mpr-23515.ic2", 0x1000000, 0x0800000, CRC(dc8557eb) SHA1(855bf4a8a7a7184a64a60d30efd505eb1181d8c6) )
	ROM_LOAD("mpr-23516.ic3", 0x1800000, 0x0800000, CRC(94494cbb) SHA1(fc977c77fa424541573c5cac28dac013d3354754) )
	ROM_LOAD("mpr-23517.ic4", 0x2000000, 0x0800000, CRC(69ba6a41) SHA1(1d5528f7d3f8721492db966ec041966192bebdf8) )
	ROM_LOAD("mpr-23518.ic5", 0x2800000, 0x0800000, CRC(49882766) SHA1(f6a7a7039dc251e02d69d4c95130102dfbb25fc9) )
	ROM_LOAD("mpr-23519.ic6", 0x3000000, 0x0800000, CRC(cdc82805) SHA1(947cdcdc16fc61ba4ca1258d170483b3decdacf2) )
	ROM_LOAD("mpr-23520.ic7", 0x3800000, 0x0800000, CRC(1a268360) SHA1(b35dab00e4e656f13fcad92bebd2c256c1965f54) )
	ROM_LOAD("mpr-23521.ic8", 0x4000000, 0x0800000, CRC(cf8674b8) SHA1(bdd2a0ef98138021707b3dd06b1d9855308ed3ec) )
	ROM_LOAD("mpr-23522.ic9", 0x4800000, 0x0800000, CRC(7ae6716e) SHA1(658b794ae6e3898885524582a207faa1076a65ca) )
	ROM_LOAD("mpr-23523.ic10",0x5000000, 0x0800000, CRC(c91efb67) SHA1(3d79870551310da7a641858ffec3840714e9cc22) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DEAD OR ALIVE 2
USA: DEAD OR ALIVE 2 USA ------
EXP: DEAD OR ALIVE 2 EXPORT----

NO.     Type    Byte    Word
IC22    32M     2B49    A054
IC1     64M     B74A    1815
IC2     64M     6B34    AB5A
IC3     64M     7EEF    EA1F
IC4     64M     0700    8C2F
IC5     64M     E365    B9CC
IC6     64M     7FE0    DC66
IC7     64M     BF8D    439B
IC8     64M     84DC    2F86
IC9     64M     15CF    8961
IC10    64M     7776    B985
IC11    64M     BCE9    21E9
IC12    64M     87FA    E9C0
IC13    64M     B82E    47A7
IC14    64M     3821    846E
IC15    64M     B491    C66E
IC16    64M     5774    918D
IC17    64M     219B    A171
IC18    64M     4848    643A
IC19    64M     6E1F    2570
IC20    64M     0CED    F2A8
IC21    64M     002C    8ECA

*/

ROM_START( doa2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22121.ic22",0x0000000, 0x0400000,  CRC(30f93b5e) SHA1(0e33383e7ab9a721dab4708b063598f2e9c9f2e7) ) // partially encrypted

	ROM_LOAD("mpr-22100.ic1", 0x0800000, 0x0800000, CRC(92a53e5e) SHA1(87fcdeee9c4e65a3eb6eb345eed85d4f2df26c3c) )
	ROM_LOAD("mpr-22101.ic2", 0x1000000, 0x0800000, CRC(14cd7dce) SHA1(5df14a5dad14bc922b4f88881dc2e9c8e74d6170) )
	ROM_LOAD("mpr-22102.ic3", 0x1800000, 0x0800000, CRC(34e778b1) SHA1(750ddf5cda9622dd2b0f7069d247ffd55911c38f) )
	ROM_LOAD("mpr-22103.ic4", 0x2000000, 0x0800000, CRC(6f3db8df) SHA1(e9bbcf7897594ae47a9e3c8641ccb2c09b0809fe) )
	ROM_LOAD("mpr-22104.ic5", 0x2800000, 0x0800000, CRC(fcc2787f) SHA1(c28eaf91fa64e49e2276702678a4f8f17e09c3b9) )
	ROM_LOAD("mpr-22105.ic6", 0x3000000, 0x0800000, CRC(3e2da942) SHA1(d8f28c40ab59fa96a1fb19ad3adbee687088a5ab) )
	ROM_LOAD("mpr-22106.ic7", 0x3800000, 0x0800000, CRC(03aceaaf) SHA1(977e5b660254e7c5fdbd9d52c1f00c8a174a5d7b) )
	ROM_LOAD("mpr-22107.ic8", 0x4000000, 0x0800000, CRC(6f1705e4) SHA1(b8215dd4ef7214e75c2ec79ad974a32422c17647) )
	ROM_LOAD("mpr-22108.ic9", 0x4800000, 0x0800000, CRC(d34d3d8a) SHA1(910f1e4d8a54a621d9212e1425152c3029c96234) )
	ROM_LOAD("mpr-22109.ic10",0x5000000, 0x0800000, CRC(00ef44dd) SHA1(3fd100007daf59693de2329df1b4981dcdf435cd) )
	ROM_LOAD("mpr-22110.ic11",0x5800000, 0x0800000, CRC(a193b577) SHA1(3513853f88c491905481dadc5ce00cc5819b2663) )
	ROM_LOAD("mpr-22111.ic12",0x6000000, 0x0800000, CRC(55dddebf) SHA1(a7b8702cf578f5be4dcf8e2eaf11bf8b71d1b4ad) )
	ROM_LOAD("mpr-22112.ic13",0x6800000, 0x0800000, CRC(c5ffe564) SHA1(efe4d0cb5a536b26489c6dd31b1e446a9be643c9) )
	ROM_LOAD("mpr-22113.ic14",0x7000000, 0x0800000, CRC(12e7adf0) SHA1(2755c3efc6ca6d5680ead1489f42798c0187c5a4) )
	ROM_LOAD("mpr-22114.ic15",0x7800000, 0x0800000, CRC(d181d0a0) SHA1(2a0e46dbb31f5c11b6ae2fc8c786192bf3701ec5) )
	ROM_LOAD("mpr-22115.ic16",0x8000000, 0x0800000, CRC(ee2c842d) SHA1(8e33f241300481bb8875bda37e3917be71ed2594) )
	ROM_LOAD("mpr-22116.ic17",0x8800000, 0x0800000, CRC(224ab770) SHA1(85d849ee077e36da1df759caa4a32525395f741c) )
	ROM_LOAD("mpr-22117.ic18",0x9000000, 0x0800000, CRC(884a45a9) SHA1(d947cb3a045c5463523355fa631d55148e12c31e) )
	ROM_LOAD("mpr-22118.ic19",0x9800000, 0x0800000, CRC(8d631cbf) SHA1(fe8a65d35b1cdaed650ddde931e59f0768ffff53) )
	ROM_LOAD("mpr-22119.ic20",0xa000000, 0x0800000, CRC(d608fa86) SHA1(54c8107cccec8cbb536f13cda5b220b7972190b7) )
	ROM_LOAD("mpr-22120.ic21",0xa800000, 0x0800000, CRC(a30facb4) SHA1(70415ca34095c795297486bce1f956f6a8d4817f) )

	// on-cart X76F100 eeprom contents
	ROM_REGION( 0x100, "naomibd_eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "841-0003.sf",  0x000000, 0x000084, CRC(3a119a17) SHA1(d37a092cca7c9cfc5f2637b355af90a65d04013e) )

	// trojaned protection data (filename is word offset)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "00000500.bin", 0x000000, 0x020000, CRC(3751d50e) SHA1(08d56e7befd3e1bc2c29217c60dee0c862af65ed) )
        ROM_LOAD( "00020504.bin", 0x020000, 0x020000, CRC(6b4ab9f6) SHA1(205c355a5a55659f35645d3e66dc1dcff703727c) )
        ROM_LOAD( "00040508.bin", 0x040000, 0x020000, CRC(dae5d37d) SHA1(63d0cde73b86e77ea7b5e1d71643566c500f9933) )
        ROM_LOAD( "0006050c.bin", 0x060000, 0x020000, CRC(c2b7ea51) SHA1(59f1cb01b140e7fdea7d945acad464e08e3adf44) )
        ROM_LOAD( "00080510.bin", 0x080000, 0x020000, CRC(7b84acb4) SHA1(e48448795c058c0fc3a9e9108de18a3181db5417) )
        ROM_LOAD( "000a0514.bin", 0x0a0000, 0x020000, CRC(f954ffcd) SHA1(3015dfc298de1e16db07406e9ecb5fc0e2aae0a8) )
        ROM_LOAD( "000c0518.bin", 0x0c0000, 0x020000, CRC(28fce8ad) SHA1(180ae2dc9181bbe4215093e4e933c9e78d3af7a8) )
        ROM_LOAD( "000e051c.bin", 0x0e0000, 0x020000, CRC(a7f7a5c1) SHA1(6b39aa8f8a762ffe61b07f77713cca5c95a8dfcf) )
        ROM_LOAD( "00100520.bin", 0x100000, 0x020000, CRC(85417674) SHA1(0d35aa0f3d5f451bbf4daad343b9ad1ebd9d9551) )
        ROM_LOAD( "00118a3a.bin", 0x120000, 0x020000, CRC(72e58444) SHA1(09ae85c131f7b4a0638efd1dff0807cc20b59637) )
        ROM_LOAD( "0012c0d8.bin", 0x140000, 0x020000, CRC(84e8e517) SHA1(f1e81913181b9c6276a430988fefc00597e33e89) )
        ROM_LOAD( "00147e22.bin", 0x160000, 0x020000, CRC(66efd53e) SHA1(40cfc20df93f9ec1f260fc3f56bb5ac0be775cc6) )
        ROM_LOAD( "001645ce.bin", 0x180000, 0x020000, CRC(adc73d19) SHA1(4df4bd7db6bf2f9af06d9d3ec9466970bec7833c) )
        ROM_LOAD( "0017c6b2.bin", 0x1a0000, 0x020000, CRC(717f2e91) SHA1(e03d778a1c39cf01827f79a1b10c03a4a6047962) )
        ROM_LOAD( "0019902e.bin", 0x1c0000, 0x020000, CRC(f4882ea8) SHA1(a27e52062780e095675f749191e0ea22755152eb) )
        ROM_LOAD( "001b562a.bin", 0x1e0000, 0x013e00, CRC(01ae0721) SHA1(b9f24e0bb8609b8cb1a45e7c1e1dc1f271bbdcaf) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DEAD OR ALIVE 2
USA: DEAD OR ALIVE 2 USA ------
EXP: DEAD OR ALIVE 2 EXPORT----

NO.     Type    Byte    Word
IC22    32M     2B49    A054
IC1     64M     B74A    1815
IC2     64M     6B34    AB5A
IC3     64M     7EEF    EA1F
IC4     64M     0700    8C2F
IC5     64M     E365    B9CC
IC6     64M     7FE0    DC66
IC7     64M     BF8D    439B
IC8     64M     84DC    2F86
IC9     64M     15CF    8961
IC10    64M     7776    B985
IC11    64M     BCE9    21E9
IC12    64M     87FA    E9C0
IC13    64M     B82E    47A7
IC14    64M     3821    846E
IC15    64M     B491    C66E
IC16    64M     5774    918D
IC17    64M     219B    A171
IC18    64M     4848    643A
IC19    64M     6E1F    2570
IC20    64M     0CED    F2A8
IC21    64M     002C    8ECA

Serial: BALH-13A0175

*/

ROM_START( doa2m )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("doa2verm.ic22", 0x0000000, 0x0400000,  CRC(94b16f08) SHA1(225cd3e5dd5f21facf0a1d5e66fa17db8497573d) )

	ROM_LOAD("mpr-22100.ic1", 0x0800000, 0x0800000, CRC(92a53e5e) SHA1(87fcdeee9c4e65a3eb6eb345eed85d4f2df26c3c) )
	ROM_LOAD("mpr-22101.ic2", 0x1000000, 0x0800000, CRC(14cd7dce) SHA1(5df14a5dad14bc922b4f88881dc2e9c8e74d6170) )
	ROM_LOAD("mpr-22102.ic3", 0x1800000, 0x0800000, CRC(34e778b1) SHA1(750ddf5cda9622dd2b0f7069d247ffd55911c38f) )
	ROM_LOAD("mpr-22103.ic4", 0x2000000, 0x0800000, CRC(6f3db8df) SHA1(e9bbcf7897594ae47a9e3c8641ccb2c09b0809fe) )
	ROM_LOAD("mpr-22104.ic5", 0x2800000, 0x0800000, CRC(fcc2787f) SHA1(c28eaf91fa64e49e2276702678a4f8f17e09c3b9) )
	ROM_LOAD("mpr-22105.ic6", 0x3000000, 0x0800000, CRC(3e2da942) SHA1(d8f28c40ab59fa96a1fb19ad3adbee687088a5ab) )
	ROM_LOAD("mpr-22106.ic7", 0x3800000, 0x0800000, CRC(03aceaaf) SHA1(977e5b660254e7c5fdbd9d52c1f00c8a174a5d7b) )
	ROM_LOAD("mpr-22107.ic8", 0x4000000, 0x0800000, CRC(6f1705e4) SHA1(b8215dd4ef7214e75c2ec79ad974a32422c17647) )
	ROM_LOAD("mpr-22108.ic9", 0x4800000, 0x0800000, CRC(d34d3d8a) SHA1(910f1e4d8a54a621d9212e1425152c3029c96234) )
	ROM_LOAD("mpr-22109.ic10",0x5000000, 0x0800000, CRC(00ef44dd) SHA1(3fd100007daf59693de2329df1b4981dcdf435cd) )
	ROM_LOAD("mpr-22110.ic11",0x5800000, 0x0800000, CRC(a193b577) SHA1(3513853f88c491905481dadc5ce00cc5819b2663) )
	ROM_LOAD("mpr-22111.ic12",0x6000000, 0x0800000, CRC(55dddebf) SHA1(a7b8702cf578f5be4dcf8e2eaf11bf8b71d1b4ad) )
	ROM_LOAD("mpr-22112.ic13",0x6800000, 0x0800000, CRC(c5ffe564) SHA1(efe4d0cb5a536b26489c6dd31b1e446a9be643c9) )
	ROM_LOAD("mpr-22113.ic14",0x7000000, 0x0800000, CRC(12e7adf0) SHA1(2755c3efc6ca6d5680ead1489f42798c0187c5a4) )
	ROM_LOAD("mpr-22114.ic15",0x7800000, 0x0800000, CRC(d181d0a0) SHA1(2a0e46dbb31f5c11b6ae2fc8c786192bf3701ec5) )
	ROM_LOAD("mpr-22115.ic16",0x8000000, 0x0800000, CRC(ee2c842d) SHA1(8e33f241300481bb8875bda37e3917be71ed2594) )
	ROM_LOAD("mpr-22116.ic17",0x8800000, 0x0800000, CRC(224ab770) SHA1(85d849ee077e36da1df759caa4a32525395f741c) )
	ROM_LOAD("mpr-22117.ic18",0x9000000, 0x0800000, CRC(884a45a9) SHA1(d947cb3a045c5463523355fa631d55148e12c31e) )
	ROM_LOAD("mpr-22118.ic19",0x9800000, 0x0800000, CRC(8d631cbf) SHA1(fe8a65d35b1cdaed650ddde931e59f0768ffff53) )
	ROM_LOAD("mpr-22119.ic20",0xa000000, 0x0800000, CRC(d608fa86) SHA1(54c8107cccec8cbb536f13cda5b220b7972190b7) )
	ROM_LOAD("mpr-22120.ic21",0xa800000, 0x0800000, CRC(a30facb4) SHA1(70415ca34095c795297486bce1f956f6a8d4817f) )

	// on-cart X76F100 eeprom contents
	ROM_REGION( 0x100, "naomibd_eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "841-0003.sf",  0x000000, 0x000084, CRC(3a119a17) SHA1(d37a092cca7c9cfc5f2637b355af90a65d04013e) )

	// trojaned protection data (filename is word offset)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "00000500.bin", 0x000000, 0x020000, CRC(a42f0337) SHA1(0aaeff753a4e2f9b5d287cd80952533f0ced430c) )
        ROM_LOAD( "00020504.bin", 0x020000, 0x020000, CRC(3d3bfbe4) SHA1(4748ca4c14a31da623ebd3a1acff93635c039888) )
        ROM_LOAD( "00040508.bin", 0x040000, 0x020000, CRC(d0c3699c) SHA1(11a4e8b47cb6ae827307cd44689477ebfe23d690) )
        ROM_LOAD( "0006050c.bin", 0x060000, 0x020000, CRC(bc2e8e71) SHA1(253561a58caf15607d87d932d9b98d6ffa779669) )
        ROM_LOAD( "00080510.bin", 0x080000, 0x020000, CRC(a26a6159) SHA1(8e4f0a2091b5b2eb825bb9220c5e4dfe9ab935fe) )
        ROM_LOAD( "000a0514.bin", 0x0a0000, 0x020000, CRC(83138b3b) SHA1(6b69e43b6e6c4b75278045a883337cb7abcf9501) )
        ROM_LOAD( "000c0518.bin", 0x0c0000, 0x020000, CRC(9f246058) SHA1(bc54f3bdc23a278b14867e096c3f644447b9c0a6) )
        ROM_LOAD( "000e051c.bin", 0x0e0000, 0x020000, CRC(7cfd53d5) SHA1(d77ff8d3edbc2d41334d6416d17339ea2c764887) )
        ROM_LOAD( "00100520.bin", 0x100000, 0x020000, CRC(8dd856d3) SHA1(9952fdf9017bdb8c50b413de5817a746c24901e9) )
        ROM_LOAD( "0011a5b4.bin", 0x120000, 0x020000, CRC(484c511d) SHA1(02c8c1b46ce8a27b85652d3f032f4f42be5341ba) )
        ROM_LOAD( "0012e7c4.bin", 0x140000, 0x020000, CRC(ceffdd4b) SHA1(36dfa9fd4857429cf9b505d04d7a82d3bc161b90) )
        ROM_LOAD( "001471f6.bin", 0x160000, 0x020000, CRC(263d0706) SHA1(194e259b78d3006372ad007a7e3b9456f607b922) )
        ROM_LOAD( "001640c4.bin", 0x180000, 0x020000, CRC(3addb31e) SHA1(97794166b0811ff9535b3a3f598e0cb3da9fc7b3) )
        ROM_LOAD( "001806ca.bin", 0x1a0000, 0x020000, CRC(dbfddad2) SHA1(72696b5ed942fe3ef0bc271f6d66d5f089221192) )
        ROM_LOAD( "00199df4.bin", 0x1c0000, 0x020000, CRC(2ab0d0c3) SHA1(10642e9d8c9c24f366359fbe7890a0f1347a7a1a) )
        ROM_LOAD( "001b5d0a.bin", 0x1e0000, 0x017e00, CRC(c9c3bbd5) SHA1(6f3853e3e1614993b5d658ebec6bf5183577ab36) )
ROM_END

/*

SYSTEMID: NAOMI
JAP:  DERBY OWNERS CLUB ------------
USA:  DERBY OWNERS CLUB ------------
EXP:  DERBY OWNERS CLUB IN EXPORT --

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     8AF3    D0BC
IC2     64M     1E79    0410
IC3     64M     146D    C51E
IC4     64M     E9AD    86BE
IC5     64M     BBB2    8685
IC6     64M     A0E1    C2E0
IC7     64M     B8CF    67B5
IC8     64M     005E    C1D6
IC9     64M     1F53    9304
IC10    64M     FAC9    8AA4
IC11    64M     B6B1    5665
IC12    64M     21DB    74F5
IC13    64M     A991    A8AB
IC14    64M     05BD    428D

Serial: BAXE-02A1386

*/

ROM_START( derbyoc )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x7800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22099b.ic22", 0x0000000, 0x0400000, CRC(5e708879) SHA1(fada4f4bf29fc8f77f354167f8db4f904610fe1a) )
	ROM_LOAD("mpr-22085.ic1",   0x0800000, 0x0800000, CRC(fffe9cc5) SHA1(ce6082fc648718b3831f709ba8b6212946c72d70) )
	ROM_LOAD("mpr-22086.ic2",   0x1000000, 0x0800000, CRC(610fe214) SHA1(c982d9e4722c2b6cb87f2bc3e2ac0f764f0bae79) )
	ROM_LOAD("mpr-22087.ic3",   0x1800000, 0x0800000, CRC(f0cd2a26) SHA1(21ff7d6540cfeb5e563d3528bd4bb31c5f285f1a) )
	ROM_LOAD("mpr-22088.ic4",   0x2000000, 0x0800000, CRC(62a7e6db) SHA1(103e2413c9706a5a98f05646fd3a7d7808593ad8) )
	ROM_LOAD("mpr-22089.ic5",   0x2800000, 0x0800000, CRC(cb135eb6) SHA1(a49df8fbae1ea0fb1251d0d8f302cc8687c3be0b) )
	ROM_LOAD("mpr-22090.ic6",   0x3000000, 0x0800000, CRC(13e44d57) SHA1(450fe281d34c088e61a4c2ee6ae434f330deb482) )
	ROM_LOAD("mpr-22091.ic7",   0x3800000, 0x0800000, CRC(efa1e2fc) SHA1(058635bee7a87b8191127060c6a28c053001b466) )
	ROM_LOAD("mpr-22092.ic8",   0x4000000, 0x0800000, CRC(de1ea163) SHA1(f2b0169fac3e1074628dec75642e7c41c8160964) )
	ROM_LOAD("mpr-22093.ic9",   0x4800000, 0x0800000, CRC(ecbc523b) SHA1(952618a0966838f5b814ff1265c899481aae1ba9) )
	ROM_LOAD("mpr-22094.ic10",  0x5000000, 0x0800000, CRC(72af7a70) SHA1(b1437dbf47f95bbdb9fe7a215c5a3b0f3839d917) )
	ROM_LOAD("mpr-22095.ic11",  0x5800000, 0x0800000, CRC(ae74b61a) SHA1(1c9de865447c9993d7faff2e61837e4b74353c3a) )
	ROM_LOAD("mpr-22096.ic12",  0x6000000, 0x0800000, CRC(d8c41648) SHA1(d465f4b841164da0738336e203c5bc6e1e799a76) )
	ROM_LOAD("mpr-22097.ic13",  0x6800000, 0x0800000, CRC(f1dedac5) SHA1(9d4499cbafe80dd0b36be617de7994a96e1e9a01) )
	ROM_LOAD("mpr-22098.ic14",  0x7000000, 0x0800000, CRC(f9824d2e) SHA1(f20f8cc2b1bef9077ede1cb874da8f2a335d39de) )
ROM_END

/*
SYSTEMID: NAOMI
JAP: DERBY OWNERS CLUB II-----------
USA: DERBY OWNERS CLUB II-----------
EXP: DERBY OWNERS CLUB II-IN EXPORT
*/

ROM_START( derbyoc2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22306.ic22",  0x0000000, 0x0400000, CRC(fcac20eb) SHA1(26cec9f615cd18ce7fccfc5e273e42c58dea1995) )
        ROM_LOAD( "mpr-22295.ic1",  0x0800000, 0x1000000, CRC(1085001b) SHA1(096ac0ecc7d5324f04514d24eb338de591926a3d) )
        ROM_LOAD( "mpr-22296.ic2",  0x1800000, 0x1000000, CRC(f18cb28b) SHA1(e7f253a35e15f62e178a1ff565f8a94102eea057) )
        ROM_LOAD( "mpr-22297.ic3",  0x2800000, 0x1000000, CRC(a78a527b) SHA1(7d4e7bd93519377a8676f7ca6f72c4d0de2a0475) )
        ROM_LOAD( "mpr-22298.ic4",  0x3800000, 0x1000000, CRC(cd76c7ea) SHA1(2db200c8853d1bc9da31da6c197c6b55e570dc69) )
        ROM_LOAD( "mpr-22299.ic5",  0x4800000, 0x1000000, CRC(ffb96da5) SHA1(bfe8f4a455d3eeec0fa9728fc1bb52cebd13b2a8) )
        ROM_LOAD( "mpr-22300.ic6",  0x5800000, 0x1000000, CRC(6fb05214) SHA1(9cececc46ac463bc5df3c3f5fb9268d5ced31837) )
        ROM_LOAD( "mpr-22301.ic7",  0x6800000, 0x1000000, CRC(52eb076d) SHA1(a79bb286594cc5d196a37a17147ad1770db5cd67) )
        ROM_LOAD( "mpr-22302.ic8",  0x7800000, 0x1000000, CRC(86767b0b) SHA1(54ed418cef78506fb824e3d16cadefdb684f2b34) )
        ROM_LOAD( "mpr-22303.ic9",  0x8800000, 0x1000000, CRC(73a80bd5) SHA1(d607f1d993af4b78d3609991e47e9540664380fd) )
        ROM_LOAD( "mpr-22304.ic10", 0x9800000, 0x1000000, CRC(46c1fb1f) SHA1(6daca76a75df3501f77e473eb065d48804fcc64a) )
        ROM_LOAD( "mpr-22305.ic11", 0xa800000, 0x1000000, CRC(027d0e7b) SHA1(e3c874e60cabb6f9ce686696d9055a0c0d5289ae) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DYNAMITE BASEBALL NAOMI
USA: SAMPLE GAME IN USA--------
EXP: SAMPLE GAME

NO.     Type    Byte    Word
IC22    16M     EF41    1DBC
IC1     64M     2743    8DE9
IC2     64M     1D2B    B4D5
IC3     64M     9127    8536
IC4     64M     946A    851B
IC5     64M     BDF4    AF2C
IC6     64M     78A2    DADB
IC7     64M     9816    06D3
IC8     64M     F8D9    9C38
IC9     64M     3C7D    532A
IC10    64M     37A2    D3F1
IC11    64M     5BF2    05FC
IC12    64M     694F    A25A
IC13    64M     685C    CDA8
IC14    64M     3DFA    32A9
IC15    64M     071F    820F
IC16    64M     1E89    D6B5
IC17    64M     889C    504B
IC18    64M     8B78    1BB5
IC19    64M     9816    7EE9
IC20    64M     E5C2    CECB
IC21    64M     5C65    8F82

Serial: ??? (sticker removed)

Protection notes (same code snippet seen in Zombie Revenge):
0C0A8148: 013C   MOV.B   @(R0,R3),R1
0C0A814A: 611C   EXTU.B  R1,R1
0C0A814C: 31C7   CMP/GT  R12,R1
0C0A814E: 1F11   MOV.L   R1,@($04,R15)
0C0A8150: 8F04   BFS     $0C0A815C
0C0A8152: E500   MOV     #$00,R5
0C0A8154: D023   MOV.L   @($008C,PC),R0 [0C0A81E4]
0C0A8156: 2052   MOV.L   R5,@R0
0C0A8158: AFFE   BRA     $0C0A8158
0C0A815A: 0009   NOP

*/

ROM_START( dybbnao )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-21575.ic22",0x0000000, 0x0200000, CRC(ba61e248) SHA1(3cce5d8b307038515d7da7ec567bfa2e3aafc274) )
	ROM_LOAD("mpr-21554.ic1", 0x0800000, 0x0800000, CRC(6eb29c37) SHA1(3548a93f9efa3bd548f9e30223a9b3570031f126) )
	ROM_LOAD("mpr-21555.ic2", 0x1000000, 0x0800000, CRC(3ff79959) SHA1(abd5407fcfa5556fc3f0c56892daad0c741a681f) )
	ROM_LOAD("mpr-21556.ic3", 0x1800000, 0x0800000, CRC(79bc8caf) SHA1(8cb77c66a86a99b85f2e3c8a5fed457f75598af4) )
	ROM_LOAD("mpr-21557.ic4", 0x2000000, 0x0800000, CRC(6f88e6fb) SHA1(7a7fdf910769d451a7cfc571811180433c353e8d) )
	ROM_LOAD("mpr-21558.ic5", 0x2800000, 0x0800000, CRC(6d4416cf) SHA1(e7ea9c0fe86e84c0358797664807056d8cfdcefe) )
	ROM_LOAD("mpr-21559.ic6", 0x3000000, 0x0800000, CRC(f4afbadf) SHA1(0d30b02835968e6044334204e5e8f8e88be6e783) )
	ROM_LOAD("mpr-21560.ic7", 0x3800000, 0x0800000, CRC(3b2e6e64) SHA1(0ad1daae658d53ca9ae9b197676eafacf820a0fe) )
	ROM_LOAD("mpr-21561.ic8", 0x4000000, 0x0800000, CRC(3c5136ea) SHA1(a0c8f4a947a6c729597a0a3d2348954d35eb5b11) )
	ROM_LOAD("mpr-21562.ic9", 0x4800000, 0x0800000, CRC(e158f4be) SHA1(37b8bcaaaede70c626cee891c53c0004b1cf23df) )
	ROM_LOAD("mpr-21563.ic10",0x5000000, 0x0800000, CRC(6b15befa) SHA1(9e0fd34a878d20b249b07bf01ce167f82f67de53) )
	ROM_LOAD("mpr-21564.ic11",0x5800000, 0x0800000, CRC(cecfaa8a) SHA1(fad935bf97a05e5991f4e0894e81c2c51f920db5) )
	ROM_LOAD("mpr-21565.ic12",0x6000000, 0x0800000, CRC(7e87d973) SHA1(66e2da9f721020e4a6aa423b3922b50b774b15f7) )
	ROM_LOAD("mpr-21566.ic13",0x6800000, 0x0800000, CRC(5354d553) SHA1(7dd84c30b0554b60598cc430366227be594b8221) )
	ROM_LOAD("mpr-21567.ic14",0x7000000, 0x0800000, CRC(9e17fdb2) SHA1(f709a2723bc028553f8c538a4b891333b70c4a62) )
	ROM_LOAD("mpr-21568.ic15",0x7800000, 0x0800000, CRC(b278efcd) SHA1(aa033eb7c5bfc76c847e0e79c3ac04f56edc5688) )
	ROM_LOAD("mpr-21569.ic16",0x8000000, 0x0800000, CRC(724e4d34) SHA1(4398fbc02e70c1ccd9869d18b345e2d790f6c314) )
	ROM_LOAD("mpr-21570.ic17",0x8800000, 0x0800000, CRC(b3375b2b) SHA1(e442d5359bab5581419408ecef796a48eee373ab) )
	ROM_LOAD("mpr-21571.ic18",0x9000000, 0x0800000, CRC(4bcefff9) SHA1(47437073756351b447cc939a2c99ebabe7a6436b) )
	ROM_LOAD("mpr-21572.ic19",0x9800000, 0x0800000, CRC(a47fd15e) SHA1(b595cadedc2e378219146ce19c0338f7e0dcc769) )
	ROM_LOAD("mpr-21573.ic20",0xa000000, 0x0800000, CRC(5d822e63) SHA1(8412980b288531c294d5cf9a6394aa0b9503d7df) )
	ROM_LOAD("mpr-21574.ic21",0xa800000, 0x0800000, CRC(d794a42c) SHA1(a79c7818c6ec993e718494b1d5407eb270a29abe) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: DYNAMITE BASEBALL '99
USA: WORLD SERIES 99
EXP: WORLD SERIES 99

NO.     Type    Byte    Word
IC22    16M     0000    0000
IC1     64M     77B9    3C1B
IC2     64M     F7FB    025A
IC3     64M     B3D4    22C1
IC4     64M     060F    6279
IC5     64M     FE49    CAEB
IC6     64M     E34C    5FAD
IC7     64M     CC04    498C
IC8     64M     388C    DF17
IC9     64M     5B91    C458
IC10    64M     AF73    4A18
IC11    64M     2E5B    A198
IC12    64M     FFDB    41CA
IC13    64M     04E1    EA4C
IC14    64M     5B22    DA9A
IC15    64M     64E7    0873
IC16    64M     1EE7    BE11
IC17    64M     79C3    3608
IC18    64M     D4CE    5AEB
IC19    64M     E846    60B8

Serial: BBDE-01A0097

*/

ROM_START( dybb99 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xa000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22141b.ic22",0x0000000, 0x0200000, CRC(6d0e0785) SHA1(aa19e7bac4c266771d1e65cffa534a49d7566f51) )
	ROM_LOAD("mpr-22122.ic1",  0x0800000, 0x0800000, CRC(403da794) SHA1(321bc5b8966d05e60110bc0b17d0f37fe1facc56) )
	ROM_LOAD("mpr-22123.ic2",  0x1000000, 0x0800000, CRC(14cfeab8) SHA1(593d006bc2e6f4d1602d7045dc51d974fc5bbd4c) )
	ROM_LOAD("mpr-22124.ic3",  0x1800000, 0x0800000, CRC(19f89fce) SHA1(a442af4e3c913fd34257bc9af29e2361f98f2fa5) )
	ROM_LOAD("mpr-22125.ic4",  0x2000000, 0x0800000, CRC(a9e7298e) SHA1(287284a3d5ea230f3b17e9acb606f28498da230e) )
	ROM_LOAD("mpr-22126.ic5",  0x2800000, 0x0800000, CRC(9f6a5d94) SHA1(71849a4e0bf1bc033e7d073ecbf85793502384c4) )
	ROM_LOAD("mpr-22127.ic6",  0x3000000, 0x0800000, CRC(653d27e3) SHA1(f31c2f237f79cfcc0db657e1fb83503da65029d8) )
	ROM_LOAD("mpr-22128.ic7",  0x3800000, 0x0800000, CRC(e1fd22a1) SHA1(3d12f025ebf5323ce28508062dd2039d186b6223) )
	ROM_LOAD("mpr-22129.ic8",  0x4000000, 0x0800000, CRC(ecf90b4a) SHA1(0403ada8958c2aee56b236032359ae13267ed966) )
	ROM_LOAD("mpr-22130.ic9",  0x4800000, 0x0800000, CRC(26638b66) SHA1(915a8a9b6835b74f49594a02212a7da170c6a74b) )
	ROM_LOAD("mpr-22131.ic10", 0x5000000, 0x0800000, CRC(60e911f8) SHA1(035694e1382e3ca99d4b0cda1082a3a2bd84bcac) )
	ROM_LOAD("mpr-22132.ic11", 0x5800000, 0x0800000, CRC(093ee986) SHA1(e43743f8a93def9e56463bb99ef45a0de3b66d0f) )
	ROM_LOAD("mpr-22133.ic12", 0x6000000, 0x0800000, CRC(d4fc133d) SHA1(a04a21107c1d2dc6c52385e52627f6d97adc6934) )
	ROM_LOAD("mpr-22134.ic13", 0x6800000, 0x0800000, CRC(31497387) SHA1(a1c9626f2fe2d2c75e02513616865da87a140aa8) )
	ROM_LOAD("mpr-22135.ic14", 0x7000000, 0x0800000, CRC(42ab4b4f) SHA1(5f5ba43926ee24649d893e5087f68ef92f8ae88c) )
	ROM_LOAD("mpr-22136.ic15", 0x7800000, 0x0800000, CRC(1f313f03) SHA1(5e7b9d3935049473c128f24cb7718cb3385b03b7) )
	ROM_LOAD("mpr-22137.ic16", 0x8000000, 0x0800000, CRC(819e4cb2) SHA1(1f6a4382c6787d9453b49bca2ae2acab89710368) )
	ROM_LOAD("mpr-22138.ic17", 0x8800000, 0x0800000, CRC(59557b9f) SHA1(beda44c65c69110bdf8afb7542ae39913dab54f2) )
	ROM_LOAD("mpr-22139.ic18", 0x9000000, 0x0800000, CRC(92faa2ca) SHA1(4953f0219c3ae62de0a89473cb7b9dd30b33fcfb) )
	ROM_LOAD("mpr-22140.ic19", 0x9800000, 0x0800000, CRC(4cb54893) SHA1(a99b39cc3c82c3cf90f794bb8c8ba60638a6f921) )
ROM_END

ROM_START( smlg99 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
        ROM_LOAD( "epr-22059.ic22", 0x0000000, 0x200000, CRC(5784f970) SHA1(e9ec692206a95cc260521154305693f6022190bc) )
        ROM_LOAD( "mpr-22038.ic1",  0x0800000, 0x800000, CRC(0a59fc5b) SHA1(efcbe2f847927fba173d333c889dd7df329d6df6) )
        ROM_LOAD( "mpr-22039.ic2",  0x1000000, 0x800000, CRC(4de79b52) SHA1(c63a3ce88db316e882948baf121fa96242010c8d) )
        ROM_LOAD( "mpr-22040.ic3",  0x1800000, 0x800000, CRC(b993fd90) SHA1(609de1509f393f884813ca4bcac533e10088ca84) )
        ROM_LOAD( "mpr-22041.ic4",  0x2000000, 0x800000, CRC(ddadfabd) SHA1(64531e68b10635415d49c0304a8ba550a3a9cef1) )
        ROM_LOAD( "mpr-22042.ic5",  0x2800000, 0x800000, CRC(136c101e) SHA1(a35d8d574b263d672a08963a992a7b507c838b70) )
        ROM_LOAD( "mpr-22043.ic6",  0x3000000, 0x800000, CRC(7f15a0a5) SHA1(5c36fa580f7eef448fb32d050078843470fafb31) )
        ROM_LOAD( "mpr-22044.ic7",  0x3800000, 0x800000, CRC(94376002) SHA1(8c4b954da69a079d9a73b8f34a9f1a94cceee9cc) )
        ROM_LOAD( "mpr-22045.ic8",  0x4000000, 0x800000, CRC(e520e2d9) SHA1(c7a9306b2dafb20baaa8bd6708fb5ece775c37f0) )
        ROM_LOAD( "mpr-22046.ic9",  0x4800000, 0x800000, CRC(976edfc8) SHA1(340b701b9e1d256963a8fe056ec975d8cbfec3d8) )
        ROM_LOAD( "mpr-22047.ic10", 0x5000000, 0x800000, CRC(32b136de) SHA1(4019e9836174b47135d7e1bbc02c23dd3ab52904) )
        ROM_LOAD( "mpr-22048.ic11", 0x5800000, 0x800000, CRC(32a9488a) SHA1(70c11fbcb1e24ed120f74aa455806e7dfbfa75b5) )
        ROM_LOAD( "mpr-22049.ic12", 0x6000000, 0x800000, CRC(8295696d) SHA1(5820cac054070aa35a64abc5bf8c6f45fe9be03f) )
        ROM_LOAD( "mpr-22050.ic13", 0x6800000, 0x800000, CRC(1cfebe44) SHA1(24415f723cd6c1efaa2513f6b52e076364eb0875) )
        ROM_LOAD( "mpr-22051.ic14", 0x7000000, 0x800000, CRC(58b9dbef) SHA1(1021d4dfbd5bcf6e1703ce608560c76d60dac71c) )
        ROM_LOAD( "mpr-22052.ic15", 0x7800000, 0x800000, CRC(95b87c1d) SHA1(bf0a89703fea7bd37e4d0fd10c7729d2cfe848d6) )
        ROM_LOAD( "mpr-22053.ic16", 0x8000000, 0x800000, CRC(666589a1) SHA1(130477f247661e87bdc4f2370788ca676336f563) )
        ROM_LOAD( "mpr-22054.ic17", 0x8800000, 0x800000, CRC(9d8c82e9) SHA1(13ebf9ffac9e1e960fa9662800c5e682284a5cdd) )
        ROM_LOAD( "mpr-22055.ic18", 0x9000000, 0x800000, CRC(fa865125) SHA1(04e0d77287f4e29df514875683992ede1e385dbc) )
        ROM_LOAD( "mpr-22056.ic19", 0x9800000, 0x800000, CRC(45a23d29) SHA1(2499637a4b389cda7cc9a7aa21014696bd1dafe2) )
        ROM_LOAD( "mpr-22057.ic20", 0xa000000, 0x800000, CRC(a056c109) SHA1(637e80c2d605851265430b0fa771a4ad5233be8a) )
        ROM_LOAD( "mpr-22058.ic21", 0xa800000, 0x800000, CRC(f16edaa0) SHA1(e093f5594df43c592a9acd45002ecc65035c2435) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: F355 CHALLENGE JAPAN
USA: F355 CHALLENGE USA
EXP: F355 CHALLENGE EXPORT

*/

ROM_START( f355 )
	ROM_REGION( 0x200000, "maincpu", 0)
	F355_BIOS
//  NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-21902.ic22",0x0000000, 0x0400000, CRC(04e8acec) SHA1(82e20f99876b13b77c0393ef545316f9eeb2c29c) )

	ROM_LOAD("mpr-21881.ic1", 0x0800000, 0x0800000, CRC(00bf0d58) SHA1(cf2c58168501c77318e946a4a4d4663993a7913c) )
	ROM_LOAD("mpr-21882.ic2", 0x1000000, 0x0800000, CRC(f87923cd) SHA1(71de4f550e507c9e967331c4a17349df064608ea) )
	ROM_LOAD("mpr-21883.ic3", 0x1800000, 0x0800000, CRC(8c8280b8) SHA1(1a7003f4111ed9715b9ef0b13b0e9ace6a6f5434) )
	ROM_LOAD("mpr-21884.ic4", 0x2000000, 0x0800000, CRC(7bfa2f9a) SHA1(5796291b14ab25f8fed8d4af43558c7294d49e27) )
	ROM_LOAD("mpr-21885.ic5", 0x2800000, 0x0800000, CRC(5a999e6c) SHA1(7a60fe7d2f234c5d9c02ba403422e3a3de5a86ba) )
	ROM_LOAD("mpr-21886.ic6", 0x3000000, 0x0800000, CRC(dee42cfb) SHA1(437257e035e1b9cfbc3a0c15b24ef1aac4f2fbcb) )
	ROM_LOAD("mpr-21887.ic7", 0x3800000, 0x0800000, CRC(fdcc0334) SHA1(3e3d2094a082f3f2dac5ffe5a7e26cf9e61a279b) )
	ROM_LOAD("mpr-21888.ic8", 0x4000000, 0x0800000, CRC(0c717590) SHA1(d304351b07145252816afc9dd82587a1731f665d) )
	ROM_LOAD("mpr-21889.ic9", 0x4800000, 0x0800000, CRC(e8935135) SHA1(609788d5adf976d5313b3fca02ebc2f3c5e2758b) )
	ROM_LOAD("mpr-21890.ic10",0x5000000, 0x0800000, CRC(aeb9d086) SHA1(22f7d2c09718bf3acb910b5950b0601adad859a2) )
	ROM_LOAD("mpr-21891.ic11",0x5800000, 0x0800000, CRC(16d07b04) SHA1(6f222e226e63e30a73649735349c1928c37e011b) )
	ROM_LOAD("mpr-21892.ic12",0x6000000, 0x0800000, CRC(2d91eed2) SHA1(f3cda9776c800ac11e13b6914d59edb11f3e116b) )
	ROM_LOAD("mpr-21893.ic13",0x6800000, 0x0800000, CRC(e55ef69b) SHA1(fa62f8034728751477effcfecff2bc4cdc982b28) )
	ROM_LOAD("mpr-21894.ic14",0x7000000, 0x0800000, CRC(f1acfaea) SHA1(2684f79c6b7595075df41d1f398f228b4aedab16) )
	ROM_LOAD("mpr-21895.ic15",0x7800000, 0x0800000, CRC(98368844) SHA1(331f87def4f82ceb1bf74b16709ef61dfcda1758) )
	ROM_LOAD("mpr-21896.ic16",0x8000000, 0x0800000, CRC(4bc2ab68) SHA1(3a6d6b7599ca0f2c63cdbc3f5916e548bf3697c7) )
	ROM_LOAD("mpr-21897.ic17",0x8800000, 0x0800000, CRC(4ef4448d) SHA1(475021aec754d4526aff77776c8d2abce2b23199) )
	ROM_LOAD("mpr-21898.ic18",0x9000000, 0x0800000, CRC(cacea996) SHA1(df2b7ce00d8d6171806f676966f5f45d7fb76431) )
	ROM_LOAD("mpr-21899.ic19",0x9800000, 0x0800000, CRC(14a4b87d) SHA1(33177dea88c6aec31e2c16c8d0d3f29c7ea772c5) )
	ROM_LOAD("mpr-21900.ic20",0xa000000, 0x0800000, CRC(81901130) SHA1(1573b5c4360e29ba1a4b4901af49d5399fa1e635) )
	ROM_LOAD("mpr-21901.ic21",0xa800000, 0x0800000, BAD_DUMP CRC(55dcbd6d) SHA1(9fec353f9e58016090e177f899a799e2e8fc7c9f) ) // returns bad in Naomi test mode
ROM_END

ROM_START( hotd2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	HOTD2_BIOS

	ROM_REGION( 0xa800000, "user1", ROMREGION_ERASEFF)
        ROM_LOAD( "epr-21585.ic22", 0x0000000, 0x200000, CRC(b23d1a0c) SHA1(9e77980d1aa980c879886e53cc76a16d7a9d43a1) )
        ROM_LOAD( "mpr-21565.ic1",  0x0800000, 0x800000, CRC(88fb0562) SHA1(185a0eab68d86617cb6325d64c48a2dd4854622b) )
        ROM_LOAD( "mpr-21566.ic2",  0x1000000, 0x800000, CRC(5f4dd576) SHA1(5483c3949e587bbcca7e8fc7db9aff4cd2a33f02) )
        ROM_LOAD( "mpr-21567.ic3",  0x1800000, 0x800000, CRC(3e62fca4) SHA1(8cdebdebabc88160f458e1e779d9ebb4e6a14523) )
        ROM_LOAD( "mpr-21568.ic4",  0x2000000, 0x800000, CRC(6f73a852) SHA1(d5fd4c0800b3a1ea04231018fcaba79184fa1d87) )
        ROM_LOAD( "mpr-21569.ic5",  0x2800000, 0x800000, CRC(c7950445) SHA1(4f56768f07703452ef92d183e4ee654ab9711283) )
        ROM_LOAD( "mpr-21570.ic6",  0x3000000, 0x800000, CRC(5a812247) SHA1(7636661da0cc9bd5a1a2062f9f3ef65889c86fd5) )
        ROM_LOAD( "mpr-21571.ic7",  0x3800000, 0x800000, CRC(17e9414a) SHA1(9f291c4dd9a049eeed88d80867f7fca1d15c6095) )
        ROM_LOAD( "mpr-21572.ic8",  0x4000000, 0x800000, CRC(5d2d8134) SHA1(a2941b6afd0302822133d932064d1aad873b1c04) )
        ROM_LOAD( "mpr-21573.ic9",  0x4800000, 0x800000, CRC(eacaf26d) SHA1(21e35def0ed998a70cc982f373feb50b7974612a) )
        ROM_LOAD( "mpr-21574.ic10", 0x5000000, 0x800000, CRC(1e3686be) SHA1(7ec1b3c9c94882c5fe7b6ba6ffe9220e90824870) )
        ROM_LOAD( "mpr-21575.ic11", 0x5800000, 0x800000, CRC(5ada00a2) SHA1(981c65310c89e7a26e2b2c3e57623e78f6ad33d0) )
        ROM_LOAD( "mpr-21576.ic12", 0x6000000, 0x800000, CRC(9eff6247) SHA1(9257492fc3e48516897002dd3ff247093af27d87) )
        ROM_LOAD( "mpr-21577.ic13", 0x6800000, 0x800000, CRC(8a80b16a) SHA1(ffeb061b31027ac322c14b9050c686b2b844d2e1) )
        ROM_LOAD( "mpr-21578.ic14", 0x7000000, 0x800000, CRC(7ae20daf) SHA1(b36d8e490ac477db178b8df08f7997448308d3fd) )
        ROM_LOAD( "mpr-21579.ic15", 0x7800000, 0x800000, CRC(fbb8641b) SHA1(6cb44f0a3f80eb68a218bba97b2395961c596b9c) )
        ROM_LOAD( "mpr-21580.ic16", 0x8000000, 0x800000, CRC(3881ec23) SHA1(e4b87a6c6fd6a2eeda8e0e5ae7bed01b18386e54) )
        ROM_LOAD( "mpr-21581.ic17", 0x8800000, 0x800000, CRC(66bff6e4) SHA1(f87d618231b71b65952fc7ea7ccabdd208622a00) )
        ROM_LOAD( "mpr-21582.ic18", 0x9000000, 0x800000, CRC(8cd2f654) SHA1(77eb7061caaf0288aad04ed88c4247d27617f338) )
        ROM_LOAD( "mpr-21583.ic19", 0x9800000, 0x800000, CRC(6cf6e705) SHA1(68d7e9becefe27b556e0c5d7ba00efd2d1fb71ca) )
        ROM_LOAD( "mpr-21584.ic20", 0xa000000, 0x800000, CRC(495e6265) SHA1(57936367fec0000691641525682fb8aefc4e4f56) )
ROM_END

/*
SYSTEMID: NAOMI
JAP: GIANT GRAM
USA: GIANT GRAM
EXP: GIANT GRAM

NO.     Type    Byte    Word
IC22    16M     0000    1111
IC1     64M     E504    548E

Serial: BAJE-01A0021
*/

ROM_START( ggram2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x6000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-21820.ic22", 0x0000000, 0x0200000, CRC(0a198278) SHA1(0df5fc8b56ddafc66d92cb3923b851a5717b551d) )
	ROM_RELOAD( 0x200000, 0x200000 )
	ROM_RELOAD( 0x400000, 0x200000 )
	ROM_RELOAD( 0x600000, 0x200000 )
	ROM_LOAD("mpr-21821.ic1",  0x0800000, 0x0800000, CRC(ed127b65) SHA1(8b6d03fc733f601a48006d3268faa8983ca69d70) )
	/* IC2 empty */
	ROM_LOAD("mpr-21823.ic3",  0x1800000, 0x0800000, CRC(a304b528) SHA1(32197c74c659de2cc5f72f13c84bacac7b136d36) )
	ROM_LOAD("mpr-21824.ic4",  0x2000000, 0x0800000, CRC(6cf92c79) SHA1(378c71f506f129b6a9aebc9fc1faf96722a6d46d) )
	ROM_LOAD("mpr-21825.ic5",  0x2800000, 0x0800000, CRC(ebac834e) SHA1(bf01fa9021b79418af63d494d8ab89ef58570fb9) )
	ROM_LOAD("mpr-21826.ic6",  0x3000000, 0x0800000, CRC(da00dcb6) SHA1(744a67d7a63aa57fd5d85c5bb3dd2b2fff30dd1d) )
	ROM_LOAD("mpr-21827.ic7",  0x3800000, 0x0800000, CRC(40874dc1) SHA1(86a2958af503264ebe12928b6f3f17e2fb6675ae) )
	/* IC8 empty */
	ROM_LOAD("mpr-21829.ic9",  0x4800000, 0x0800000, CRC(c5553df2) SHA1(b97a82ac9133dab4bfd87392f803754b6d00389f) )
	ROM_LOAD("mpr-21830.ic10", 0x5000000, 0x0800000, CRC(e01ceb86) SHA1(dd5703d7712cfc0053bddfff63e78dda372b6ff2) )
	ROM_LOAD("mpr-21831.ic11", 0x5800000, 0x0800000, CRC(751848d0) SHA1(9c2267fd3c6f9ea5f2679bb2ca20d511a49b2845) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: GIANT GRAM 2000
USA: GIANT GRAM 2000
EXP: GIANT GRAM 2000

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     904A    81AE
IC2     64M     E9F7    B152
IC3     64M     A4D0    8FB7
IC4     64M     A869    64FB
IC5     64M     30CB    3483
IC6     64M     94DD    7F14
IC7     64M     BA8B    EA07
IC8     64M     6ADA    5CDA
IC9     64M     7CDA    86C1
IC10    64M     86F2    73A3
IC11    64M     44D8    1D11
IC12    64M     F25E    EDA8
IC13    64M     4804    6251
IC14    64M     E4FE    3808
IC15    64M     FD3D    D37A
IC16    64M     6D48    F5B3
IC17    64M     F0C6    CA29
IC18    64M     07C3    E4AE
IC19    64M     50F8    8500
IC20    64M     4EA2    D0CE
IC21    64M     090B    5667

Serial: BCCG-21A0451

*/

ROM_START( gram2000 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23377.ic11",        0x0000000, 0x0400000, CRC(4ca3149c) SHA1(9d25fc659658b416202b033754669be2f3abcdbe) )
	/* ic1 empty? test mode reports a bad result. */
	ROM_LOAD32_WORD("mpr-23357.ic17", 0x1000000, 0x0800000, CRC(eaf77487) SHA1(bdfc4666a6724441c11b31d89fa30c4bd11cbdd1) ) //ic 2
	ROM_LOAD32_WORD("mpr-23358.ic18", 0x1000002, 0x0800000, CRC(96819a5b) SHA1(e28c9d7b0579ab5d6116401b49f30dc8e4961618) ) //ic 3
	ROM_LOAD32_WORD("mpr-23359.ic19", 0x2000000, 0x0800000, CRC(757b9e89) SHA1(b131af1cbcb4fcebb7081b208acc86841192ff14) ) //ic 4
	ROM_LOAD32_WORD("mpr-23360.ic20", 0x2000002, 0x0800000, CRC(b38d28ff) SHA1(df4ff5be67c9812cdf8f018a9e60ec82b9faf7e4) ) //ic 5
	ROM_LOAD32_WORD("mpr-23361.ic21", 0x3000000, 0x0800000, CRC(680d7d77) SHA1(83cb29157f84739e424df7565e7bcb935564866f) ) //ic 6
	ROM_LOAD32_WORD("mpr-23362.ic22", 0x3000002, 0x0800000, CRC(84b7988d) SHA1(4e5d657be03f2c0b5e771e19c907aac60d8d8dac) ) //ic 7
	ROM_LOAD32_WORD("mpr-23363.ic23", 0x4000000, 0x0800000, CRC(17ae2b21) SHA1(8672166fbf99393ea2485ffb0fc4e64f43865bde) ) //ic 8
	ROM_LOAD32_WORD("mpr-23364.ic24", 0x4000002, 0x0800000, CRC(3d422a1c) SHA1(c82b0a9ebb56f17b4ce60293beee612c08564a25) ) //ic 9
	ROM_LOAD32_WORD("mpr-23365.ic25", 0x5000000, 0x0800000, CRC(e975496c) SHA1(6b309e28697c2884b806d17900702c62074d90a4) ) //ic 10
	ROM_LOAD32_WORD("mpr-23366.ic26", 0x5000002, 0x0800000, CRC(55e96378) SHA1(190ec23fabc60b820fd9b1486fd6cb1bfe56ba6c) ) //ic 11
	ROM_LOAD32_WORD("mpr-23367.ic27", 0x6000000, 0x0800000, CRC(5d40d017) SHA1(67a405c58687c119e774511b97399b5854ceb09b) ) //ic 12
	ROM_LOAD32_WORD("mpr-23368.ic28", 0x6000002, 0x0800000, CRC(8fb3be5f) SHA1(52c1f4537bf2dc2b47996dea87317ee9b7860cd9) ) //ic 13
	ROM_LOAD32_WORD("mpr-23369.ic29", 0x7000000, 0x0800000, CRC(a6a1671d) SHA1(4c458ce901a5cbb1cfd09bcf6926160a89c81e30) ) //ic 14
	ROM_LOAD32_WORD("mpr-23370.ic30", 0x7000002, 0x0800000, CRC(29876427) SHA1(18faeadd0c285edc94ff269b0c2faa0a3cc4c296) ) //ic 15
	ROM_LOAD32_WORD("mpr-23371.ic31", 0x8000000, 0x0800000, CRC(5cad6596) SHA1(1f19ca43c13afbe1a7cc48cf51a82aa06aec99f8) ) //ic 16
	ROM_LOAD32_WORD("mpr-23372.ic32", 0x8000002, 0x0800000, CRC(3d848b16) SHA1(328289998981dae6b593636a5bd2c6d0954c2625) ) //ic 17
	ROM_LOAD32_WORD("mpr-23373.ic33", 0x9000000, 0x0800000, CRC(369227f9) SHA1(85ce0f4f139788cda35471658196a84a36019fe6) ) //ic 18
	ROM_LOAD32_WORD("mpr-23374.ic34", 0x9000002, 0x0800000, CRC(1f8a2e08) SHA1(ff9b9bfada831baeb4830a3d1a4bfb38570b9972) ) //ic 19
	ROM_LOAD32_WORD("mpr-23375.ic35", 0xa000000, 0x0800000, CRC(7d4043db) SHA1(cadf22419e5b63c33a179bb6b0742035fc9d8028) ) //ic 20
	ROM_LOAD32_WORD("mpr-23376.ic36", 0xa000002, 0x0800000, CRC(e09cb473) SHA1(c3ec980f1a56142a0e06bae9594d6038acf0690d) ) //ic 21
ROM_END

/*
SYSTEMID: NAOMI
JAP: GUILTY GEAR X
USA: DISABLE
EXP: DISABLE
*/

ROM_START( ggx )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x7800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23356.ic22", 0x0000000, 0x0400000, CRC(ed2d289f) SHA1(d4f73c6cd25f320616e21f1ff0cdc0a566185dcb) )
	ROM_LOAD("mpr-23342.ic1",  0x0800000, 0x0800000, CRC(4fd89557) SHA1(3a687393d38e890acb0d1b0edc3ea585773c0222) )
	ROM_LOAD("mpr-23343.ic2",  0x1000000, 0x0800000, CRC(2e4417b6) SHA1(0f87fa92f01116b0acfae5f1b5a148c1a12a487f) )
	ROM_LOAD("mpr-23344.ic3",  0x1800000, 0x0800000, CRC(968eea3b) SHA1(a3bb7233b9a950f00b4dcd7bb055dbdba2b29860) )
	ROM_LOAD("mpr-23345.ic4",  0x2000000, 0x0800000, CRC(30efe1ec) SHA1(be28243ab84acb41229d42056ba051a839e7af65) )
	ROM_LOAD("mpr-23346.ic5",  0x2800000, 0x0800000, CRC(b34d9461) SHA1(44bd132189c5487fef559883300993393f9f29c6) )
	ROM_LOAD("mpr-23347.ic6",  0x3000000, 0x0800000, CRC(5a254cd1) SHA1(5ca00400c9e7f6c2565b1ff2d2552a90faadf6dd) )
	ROM_LOAD("mpr-23348.ic7",  0x3800000, 0x0800000, CRC(aff43142) SHA1(c23bbfceb47885164250ca4800a52b9e9e9e80bc) )
	ROM_LOAD("mpr-23349.ic8",  0x4000000, 0x0800000, CRC(e83871c7) SHA1(49a8140f38d896e8645fbc838f22af561bd2aa7d) )
	ROM_LOAD("mpr-23350.ic9",  0x4800000, 0x0800000, CRC(4237010b) SHA1(e757dff4c353416f99eaf3cb1945b94d2768fc4f) )
	ROM_LOAD("mpr-23351.ic10", 0x5000000, 0x0800000, CRC(b096f712) SHA1(f8e2322ba83224029cd4b91cf4d51a9376923b45) )
	ROM_LOAD("mpr-23352.ic11", 0x5800000, 0x0800000, CRC(1a01ab38) SHA1(c161d5f0d60849f4e2b51ac00ca877e1c5624bff) )
	ROM_LOAD("mpr-23353.ic12", 0x6000000, 0x0800000, CRC(daa0ca24) SHA1(afce14e213e79add7fded838e71bb4447425906a) )
	ROM_LOAD("mpr-23354.ic13", 0x6800000, 0x0800000, CRC(cea127f7) SHA1(11f12472ebfc93eb72b764c780e30afd4812dbe9) )
	ROM_LOAD("mpr-23355.ic14", 0x7000000, 0x0800000, CRC(e809685f) SHA1(dc052b4eb4fdcfdc22c4807316ce34ee7a0d58a6) )

	// trojaned protection data
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "a0100000.bin", 0x000000, 0x010000, CRC(cbe5a37a) SHA1(63f40758ef94d02a53ad860b2bed9c41e20a8263) )
        ROM_LOAD( "a010b268.bin", 0x010000, 0x010000, CRC(7f0ba37e) SHA1(2d33703c9ec08800c3fae23991859f713f8c944b) )
        ROM_LOAD( "a0115558.bin", 0x020000, 0x010000, CRC(882c945f) SHA1(60bf8fe3dcf079464f660c7c210d2a379b2ef04f) )
        ROM_LOAD( "a0120b28.bin", 0x030000, 0x010000, CRC(024952c0) SHA1(98e4ce87b5ac1169b5ab1ba9cc7553069e09cbbe) )
        ROM_LOAD( "a012987a.bin", 0x040000, 0x010000, CRC(50ea6621) SHA1(ea4e5e0b9563c5e49794a634c08be8d2e81fb773) )
        ROM_LOAD( "a012e320.bin", 0x050000, 0x010000, CRC(e445906e) SHA1(cd585100b78253bdd16f3895fdb37ff1e08690f6) )
        ROM_LOAD( "a0133cf6.bin", 0x060000, 0x00f014, CRC(5763d154) SHA1(645b0a10fd196f53ff7f656b8db798c8e753c4ed) )
        ROM_LOAD( "a0170000.bin", 0x070000, 0x010000, CRC(e153030a) SHA1(e1738ec558f0e3d5a796e34618c533cfcd6928ed) )
        ROM_LOAD( "a017cf18.bin", 0x080000, 0x010000, CRC(def783cf) SHA1(911e6b99f8ce498ddd5048ea655b76cad33f47c0) )
        ROM_LOAD( "a018a45a.bin", 0x090000, 0x010000, CRC(408cd99b) SHA1(110f9922cded7ffc5108ed964b82ac39beb6ce6b) )
        ROM_LOAD( "a0197852.bin", 0x0a0000, 0x010000, CRC(09d10a60) SHA1(04bfdba2ea9a45cb10a60fe95d7d2e0997012b4f) )
        ROM_LOAD( "a01a4916.bin", 0x0b0000, 0x010000, CRC(3895730e) SHA1(b2c0a839e330916e598c8f27f2a4873bbaaddbde) )
        ROM_LOAD( "a01b1c3e.bin", 0x0c0000, 0x010000, CRC(eecb6a13) SHA1(0203c54b98aaf203e7d1b3646102efe63c550115) )
        ROM_LOAD( "a01bef38.bin", 0x0d0000, 0x010000, CRC(7d9a497c) SHA1(680a49389e7774aa03ebe6fc844dea53b430796f) )
        ROM_LOAD( "a01cbb10.bin", 0x0e0000, 0x010000, CRC(69b770bd) SHA1(aac5801d9a8a48ef77e932195cd7c30ae562bc38) )
        ROM_LOAD( "a01d7500.bin", 0x0f0000, 0x010000, CRC(a1119fca) SHA1(1acda8f73a5b1fb180985245b8b0eb9453db0cea) )
        ROM_LOAD( "a0200000.bin", 0x100000, 0x010000, CRC(21662b41) SHA1(4dab25dcc92d1a7418d308542499bdec6c3b3b2b) )
        ROM_LOAD( "a0210004.bin", 0x110000, 0x010000, CRC(07166890) SHA1(529c28bf925243468fc00b0ad5ea57429d4de907) )
        ROM_LOAD( "a0220008.bin", 0x120000, 0x008000, CRC(09cd1354) SHA1(83f6b36aec4e7c5aa1b8bc21dc6fabf8f283ed55) )
        ROM_LOAD( "a0228000.bin", 0x130000, 0x008000, CRC(628a4917) SHA1(cf36bc2413cb63fb511936ab4b5f11b7c8cbb057) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: HEAVY METAL JAPAN
USA: HEAVY METAL USA
EXP: HEAVY METAL EURO

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     CBA3    16D2
IC2     64M     087A    079B
IC3     64M     CDB0    804C
IC4     64M     326A    E815
IC5     64M     C164    5DB4
IC6     64M     38A0    AAFC
IC7     64M     1134    DFCC
IC8     64M     6597    6975
IC9     64M     D6FB    8917
IC10    64M     6442    18AC
IC11    64M     4F77    EEFE

*/


ROM_START( hmgeo )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x6000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23716a.ic22", 0x0000000, 0x0400000,  CRC(c5cb0d3b) SHA1(20de8f5ee183e996ccde77b10564a302939662db) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23705.ic1", 0x0800000, 0x0800000, CRC(2549b57d) SHA1(02c04c8ccb0de680171d06700ca9a40208286894) )
	ROM_LOAD("mpr-23706.ic2", 0x1000000, 0x0800000, CRC(9f21865c) SHA1(a1f5aec34097cf2b86110110f586ba8b3cf28bd1) )
	ROM_LOAD("mpr-23707.ic3", 0x1800000, 0x0800000, CRC(ba2f42cd) SHA1(e924f8ef58cc81b7303d8fb3baf0e384c6387e7f) )
	ROM_LOAD("mpr-23708.ic4", 0x2000000, 0x0800000, CRC(19c4e61b) SHA1(a4619df98818d33bdaa3e6429c14d1aeec316e6a))
	ROM_LOAD("mpr-23709.ic5", 0x2800000, 0x0800000, CRC(676430b3) SHA1(5fa40c45afe97b0f09e575e3c01d44aa9259961d) )
	ROM_LOAD("mpr-23710.ic6", 0x3000000, 0x0800000, CRC(5d32dba3) SHA1(5bb5796a682cc6ee68458403c69343bf753ece7a) )
	ROM_LOAD("mpr-23711.ic7", 0x3800000, 0x0800000, CRC(650df507) SHA1(dff192b3bd4f39627779e2ba86d9dd13536221dd) )
	ROM_LOAD("mpr-23712.ic8", 0x4000000, 0x0800000, CRC(154f10ce) SHA1(67f6ff297f77632efe1965a81ed9f5c7dfa7a6b3) )
	ROM_LOAD("mpr-23713.ic9", 0x4800000, 0x0800000, CRC(2969bac7) SHA1(5f1cf6ac726c2fe183d66e4022962e44592f9ccd) )
	ROM_LOAD("mpr-23714.ic10",0x5000000, 0x0800000, CRC(da462c44) SHA1(ca450b6c07f939f96eba7b44c45b4e38abd598aa) )
	ROM_LOAD("mpr-23715.ic11",0x5800000, 0x0800000, BAD_DUMP CRC(59dfd3ad) SHA1(bb8f7a1bcf16038b18f4485a6b6d81fa72ca7bd0) )

	// trojaned protection data
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "a3075ac0.bin", 0x000000, 0x00ff04, CRC(440dca2a) SHA1(4e261ff309e7e774fa2061de30174b0e4e967c00) )
        ROM_LOAD( "a30859c4.bin", 0x010000, 0x00ff04, CRC(c22c76a7) SHA1(76d9e47730e4551e6e1a96eeee937296f1153b8b) )
        ROM_LOAD( "a30958c8.bin", 0x020000, 0x00ff04, CRC(8f29ee4d) SHA1(e1ac1ccf85dc338e9d5381dfa8ededc8af8f3e92) )
        ROM_LOAD( "a30a57cc.bin", 0x030000, 0x00ff04, CRC(479846c1) SHA1(b063e2f7d521cd8a60b770477c13946679e06dd7) )
        ROM_LOAD( "a30b56d0.bin", 0x040000, 0x00ff04, CRC(107f777e) SHA1(002f062a2073dff5f0d4722803aef0a8ffc7a61a) )
        ROM_LOAD( "a30c55d4.bin", 0x050000, 0x00ff04, CRC(d73b35c1) SHA1(951cceaf6a53d4fc4b967c3675d3fa98a6874b2a) )
        ROM_LOAD( "a30d54d8.bin", 0x060000, 0x00ff04, CRC(969a6957) SHA1(ee6909f832b96ec3e6ae0cec3f18a7983cae89d9) )
        ROM_LOAD( "a30e53dc.bin", 0x070000, 0x00ff04, CRC(002f7b41) SHA1(12bb33f1229d74b32ab481f5a2e57332f0def6e7) )
        ROM_LOAD( "a30f52e0.bin", 0x080000, 0x00a904, CRC(97e32fdd) SHA1(a40f412e40544a45784b2f8d9d5494b6491efc4a) )
        ROM_LOAD( "a30ffbe4.bin", 0x090000, 0x000084, CRC(c4311af6) SHA1(7a53061a1a1a024d72880a9a9c2e6a61d2607a46) )
        ROM_LOAD( "a30ffc80.bin", 0x0a0000, 0x00ff04, CRC(b1b5e559) SHA1(2f5963d8aa46d7aeb869769a7052d3006d56d5a4) )
        ROM_LOAD( "a310fb84.bin", 0x0b0000, 0x00ff04, CRC(91bdc0b2) SHA1(3f77fe6b67dba605296c3c038265b4e0e7eabd97) )
        ROM_LOAD( "a311fa88.bin", 0x0c0000, 0x00ff04, CRC(dc5d1334) SHA1(52287bd3ade05be4511e43a7707de8cc38117316) )
        ROM_LOAD( "a312f98c.bin", 0x0d0000, 0x00ff04, CRC(e3d9ea36) SHA1(0860f5ac3086d16132f3d0d27266c644470b4f1e) )
        ROM_LOAD( "a313f890.bin", 0x0e0000, 0x00ff04, CRC(8b23c562) SHA1(fcc9d04c150f007dcd4a9a2bbee1e438a81b7c49) )
        ROM_LOAD( "a314f794.bin", 0x0f0000, 0x009104, CRC(0c0a9aa2) SHA1(1b9063a18703ce4a03ffe20e3f13e9bae06bdb57) )
        ROM_LOAD( "a3158898.bin", 0x100000, 0x0000ac, CRC(e23f61d5) SHA1(80ff570a6846f1af2b87e5888675257728e8e253) )
        ROM_LOAD( "a0800f40.bin", 0x110000, 0x000204, CRC(f4935846) SHA1(d7eaa710274080f378afa4fb2e14f3bd406c45fe) )
        ROM_LOAD( "a0801144.bin", 0x120000, 0x000034, CRC(28a62ac0) SHA1(d9bb6baf5feaca655f333e2f0b9b200f44ef1726) )
        ROM_LOAD( "a0801180.bin", 0x130000, 0x00ff04, CRC(4b9e08b7) SHA1(dab208f7af4475210135aeb648e6bed7db7399b9) )
        ROM_LOAD( "a0811084.bin", 0x140000, 0x00ff04, CRC(4003bab7) SHA1(0b1df4b9b1d1a1072719b559a71f5ae5811be63c) )
        ROM_LOAD( "a0820f88.bin", 0x150000, 0x007e04, CRC(67fb4342) SHA1(8b8c384a1628756a08aa8131095e49756967b5e2) )
        ROM_LOAD( "a0828d8c.bin", 0x160000, 0x0000f4, CRC(71c85e2b) SHA1(680b6ee02ea24a502bc42bade23e11e6a7ddc7ad) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: GIGAWING2 JAPAN
USA: GIGAWING2 USA
EXP: GIGAWING2 EXPORT

NO.     Type    Byte    Word
IC22    16M     C1C3    618F
IC1     64M     8C09    3A15
IC2     64M     91DC    C17F
IC3     64M     25CB    2AA0
IC4     64M     EB35    C1FF
IC5     64M     8B25    914E
IC6     64M     72CB    68FA
IC7     64M     191E    2AF3
IC8     64M     EACA    12CD
IC9     64M     717F    40ED
IC10    64M     1E43    0F1A

byteswaped @ e0158 key b957 read count 400, write 404
e055c key 37ca read count 400, write 404

*/

ROM_START( gwing2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x5800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22270.ic22",0x0000000, 0x0200000,  CRC(876b3c97) SHA1(eb171d4a0521c3bea42b4aae3607faec63e10581) )
        ROM_LOAD("mpr-22271.ic1", 0x0800000, 0x1000000, CRC(9a072af5) SHA1(d5edff43d180346ba4d4f214c08f2db290a72def) )
        ROM_LOAD("mpr-22272.ic2", 0x1800000, 0x1000000, CRC(1e816ab1) SHA1(6e1fd47a21f5da7d2145caaf68094445f122a239) )
        ROM_LOAD("mpr-22273.ic3", 0x2800000, 0x1000000, CRC(cd633dcf) SHA1(f044d93802a4ba29d0e70c597d3fbe65da591335) )
        ROM_LOAD("mpr-22274.ic4", 0x3800000, 0x1000000, CRC(f8daaaf3) SHA1(8854d3f8e3d55715ede33ee918b641e251f752b4) )
        ROM_LOAD("mpr-22275.ic5", 0x4800000, 0x1000000, CRC(61aa1521) SHA1(7d9f5790e72a9151d128ac7887e236526fdf72a0) )

	// trojaned protection data (filename is address read from)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "a085ddc0.bin", 0x000000, 0x00c960, CRC(77303f1c) SHA1(5dc8799b9ec102540c6e7647eccb69dfc5e6ecfb) )
        ROM_LOAD( "a080c6c0.bin", 0x010000, 0x020000, CRC(168e551a) SHA1(03bb8ce4bb233db75a0e07a8b5ad28149b5f126e) )
        ROM_LOAD( "a082c6c4.bin", 0x030000, 0x020000, CRC(bd2fcd3a) SHA1(2eefa22472d7caf30a3467c1f7ed8dc72f1665c1) )
        ROM_LOAD( "a084c6c8.bin", 0x050000, 0x020000, CRC(c1d28ba0) SHA1(5e497758c3756fe993b481ab2009402a105fbc63) )
        ROM_LOAD( "a08c67e0.bin", 0x070000, 0x020000, CRC(ceabcb70) SHA1(bbc97218f32913eefe52b1bb1606bad60d7c2894) )
        ROM_LOAD( "a08e67e4.bin", 0x090000, 0x020000, CRC(4bc98e7b) SHA1(5f506d05a05a129bf500f0112e39b722872f1220) )
        ROM_LOAD( "a09067e8.bin", 0x0b0000, 0x020000, CRC(4cd8a958) SHA1(d3a96287986ab851acced420c4ff9f4d2035188a) )
        ROM_LOAD( "a09267ec.bin", 0x0d0000, 0x020000, CRC(17c1df0d) SHA1(62964224f27c54651518a8622756c253d777629a) )
        ROM_LOAD( "a09467f0.bin", 0x0f0000, 0x020000, CRC(99c78505) SHA1(bbb95757bc0be76c680255a0e72d2eacc4ca525e) )
        ROM_LOAD( "a09667f4.bin", 0x110000, 0x016360, CRC(bec1fd2f) SHA1(e2e9d47d6d5d35bfa9a432c7f47f95c780b17551) )
        ROM_LOAD( "a088acc0.bin", 0x130000, 0x020000, CRC(abb1e0d9) SHA1(a1d5d484faaa806b365069755660ec14c1cecada) )
        ROM_LOAD( "a08aacc4.bin", 0x150000, 0x01bb00, CRC(2acc08c8) SHA1(c04276ec940a73e4b8fe140ef9f4bec8c629ecc9) )
        ROM_LOAD( "a086a740.bin", 0x170000, 0x020000, CRC(5d90c88c) SHA1(7af4107e1930292d3c64b80cb29d31aae8a565b6) )
        ROM_LOAD( "a088a744.bin", 0x190000, 0x000560, CRC(79c8eb0f) SHA1(0216fa128d9851b96f3864446ef71e0ed6f4ccd0) )
        ROM_LOAD( "a097cb60.bin", 0x1a0000, 0x010820, CRC(371e88d9) SHA1(04f0167000e96d556227e8abf6e06e0698e61e9f) )
	ROM_LOAD( "a0807fa0.bin", 0x1c0000, 0x004700, CRC(4eed5ff3) SHA1(26425baa06959d612b3b7e9f367fbb97258f544e) )
        ROM_LOAD( "a0800000.bin", 0x1d0000, 0x006040, CRC(17651005) SHA1(500806ba917133ae604817c6ee2913d10762e290) )
        ROM_LOAD( "a0806060.bin", 0x1e0000, 0x001f20, CRC(ff25a8fe) SHA1(e8c0ddf63802e3d35478e1a491dc7162283b2474) )
ROM_END

/*
SYSTEMID: NAOMI
JAP: IDOL JANSHI SUCHIE-PAI 3
USA: DISABLE
EXP: DISABLE

NO.     Type    Byte    Word
IC22    16M     0000    0000
IC1     64M     E467    524B
IC2     64M     9D05    4992
IC3     64M     E3F7    6481
IC4     64M     6C22    25E3
IC5     64M     180F    E89F
IC6     64M     60C9    2B86
IC7     64M     4EDE    4539
IC8     64M     3AD3    0046
IC9     64M     8D37    BA16
IC10    64M     8AE3    4D71
IC11    64M     B519    1393
IC12    64M     4695    B159
IC13    64M     536F    D0C6
IC14    32M     81F9    DA1B
*/

ROM_START( suchie3 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x7800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-21979.ic22",0x0000000, 0x0200000, CRC(335c9e25) SHA1(476790fdd99a8c13336e795b4a39b071ed86a97c) )
	ROM_LOAD("mpr-21980.ic1", 0x0800000, 0x0800000, CRC(2b5f958a) SHA1(609585dda27c5e111378a92f04fa03ae11d42540) )
	ROM_LOAD("mpr-21981.ic2", 0x1000000, 0x0800000, CRC(b4fff4ee) SHA1(333fb5a662775662881154b654233f207782a8aa) )
	ROM_LOAD("mpr-21982.ic3", 0x1800000, 0x0800000, CRC(923ee0ff) SHA1(4f92cc1abfd948a1ed15fdca11251aba96bdc022) )
	ROM_LOAD("mpr-21983.ic4", 0x2000000, 0x0800000, CRC(dd659ab1) SHA1(96d9825fc5cf72a9ef83f10e480fd8925b1d6762) )
	ROM_LOAD("mpr-21984.ic5", 0x2800000, 0x0800000, CRC(b34de0c7) SHA1(dbb7a6a19af2571441b5ecbddddae6891809ffcf) )
	ROM_LOAD("mpr-21985.ic6", 0x3000000, 0x0800000, CRC(f1516e0a) SHA1(246d287df592cd69df689dc10e8647a9dbf804b7) )
	ROM_LOAD("mpr-21986.ic7", 0x3800000, 0x0800000, CRC(2779c418) SHA1(8d1a89ddf0c68f1eaf6eb0dafadf9b614492fff1) )
	ROM_LOAD("mpr-21987.ic8", 0x4000000, 0x0800000, CRC(6aaaacdd) SHA1(f5e67c88db8bce8f2f4cab73a5d0a24ba57c812b) )
	ROM_LOAD("mpr-21988.ic9", 0x4800000, 0x0800000, CRC(ed61b155) SHA1(679124f0f7c7bc4791025cff274d903cf5bcae70) )
	ROM_LOAD("mpr-21989.ic10",0x5000000, 0x0800000, CRC(ae8562cf) SHA1(e31986e53159729434a7952e8c4ed2adf8dd8e9d) )
	ROM_LOAD("mpr-21990.ic11",0x5800000, 0x0800000, CRC(57fd9fdd) SHA1(62b3bc4a2828751459557b63d900ca6d46792e24) )
	ROM_LOAD("mpr-21991.ic12",0x6000000, 0x0800000, CRC(d82f834a) SHA1(06902713bdf6f68182749916cacc9ae6528dc355) )
	ROM_LOAD("mpr-21992.ic13",0x6800000, 0x0800000, CRC(599a2fb8) SHA1(2a0007064ad2ee1e1a0fda1d5676df4ff19a9f2f) )
	ROM_LOAD("mpr-21993.ic14",0x7000000, 0x0400000, CRC(fb28cf0a) SHA1(d51b1d4514a93074d1f77bd1bc5995739604cf56) )
ROM_END

/*
SYSTEMID: NAOMI
JAP: SHANGRI-LA
USA: SHANGRI-LA
EXP: SHANGRI-LA
*/

ROM_START( shangril )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x6800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22060.ic22", 0x0000000, 0x0400000, CRC(5ae18595) SHA1(baaf8fd948b07ab9970571fecebc3c4fab5d4897) )
	ROM_LOAD("mpr-22061.ic1",  0x0800000, 0x0800000, CRC(4d760b34) SHA1(ba7dce0ab7961a77622a41c3f50c112a7e9904aa) )
	ROM_LOAD("mpr-22062.ic2",  0x1000000, 0x0800000, CRC(f713c59f) SHA1(75d8559f1b847fd6a51009fe9333b9627adcbd75) )
	ROM_LOAD("mpr-22063.ic3",  0x1800000, 0x0800000, CRC(a93ad631) SHA1(c829c58ed899fe3d4f71950c883098a215bcda1b) )
	ROM_LOAD("mpr-22064.ic4",  0x2000000, 0x0800000, CRC(56e34efd) SHA1(f810a4a0105adb7f1eaa078440e28a9bac20c3ea) )
	ROM_LOAD("mpr-22065.ic5",  0x2800000, 0x0800000, CRC(44b230bd) SHA1(d560690ddd1b9bbe919b20599d25c544df2dc808) )
	ROM_LOAD("mpr-22066.ic6",  0x3000000, 0x0800000, CRC(69f0be28) SHA1(05fc6f3b18645b165cfa0ac7b3d56013aabb360b) )
	ROM_LOAD("mpr-22067.ic7",  0x3800000, 0x0800000, CRC(344f9d01) SHA1(260e748dc265fb2b5d50f9a856ccdd157ac103fd) )
	ROM_LOAD("mpr-22068.ic8",  0x4000000, 0x0800000, CRC(48d0d510) SHA1(d3aa51f29699363c8949b20493eba1a5c585ca0e) )
	ROM_LOAD("mpr-22069.ic9",  0x4800000, 0x0800000, CRC(94e6dfa9) SHA1(83ca9ea5d2892511626be362ff2cab22f2b945cf) )
	ROM_LOAD("mpr-22070.ic10", 0x5000000, 0x0800000, CRC(8dcd2b3d) SHA1(0d8b735120fc63306516f6acc333345cc7774ff1) )
	ROM_LOAD("mpr-22071.ic11", 0x5800000, 0x0800000, CRC(1ab1f1ab) SHA1(bb8fa8d5a681115a82e9598ebe599b106f7aae9d) )
	ROM_LOAD("mpr-22072.ic12", 0x6000000, 0x0800000, CRC(cb8d2634) SHA1(03ac8fb3a1acb1f8e32d9325c4da42417752f934) )
ROM_END

/*
SYSTEMID: NAOMI
JAP: MARVEL VS. CAPCOM 2
USA: MARVEL VS. CAPCOM 2
EXP: MARVEL VS. CAPCOM 2

Note: the following game is one of the few known regular Naomi game to have a rom test item in its specific test mode menu.
So the Naomi regular board test item is unreliable in this circumstance.

*/

ROM_START( mvsc2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x8900000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23085a.ic11", 0x0000000, 0x0400000, CRC(5d5b7ad1) SHA1(f58c31b245fc33fa541f9f074548402a63f7c3d3) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23048.ic17",  0x0800000, 0x0800000, CRC(93d7a63a) SHA1(c50d10b4a3f9db51eae5749f5b665d7c8ab6c898) )
	ROM_LOAD("mpr-23049.ic18",  0x1000000, 0x0800000, CRC(003dcce0) SHA1(fb71c8ca9271d2155878c72d8fe2df3031e6c014) )
	ROM_LOAD("mpr-23050.ic19",  0x1800000, 0x0800000, CRC(1d6b88a7) SHA1(ba42e9d1d912d88a7ad839b878975ba590634320) )
	ROM_LOAD("mpr-23051.ic20",  0x2000000, 0x0800000, CRC(01226aaa) SHA1(a4c6a0eda05e53d0e51b92a4317a86a708a7efdb) )
	ROM_LOAD("mpr-23052.ic21",  0x2800000, 0x0800000, CRC(74bee120) SHA1(5a0fb48fa758a2be2e08e3b1298103c5aa748835) )
	ROM_LOAD("mpr-23053.ic22",  0x3000000, 0x0800000, CRC(d92d4401) SHA1(a868780f8d2e176ff10781e1c08bf932f34ac504) )
	ROM_LOAD("mpr-23054.ic23",  0x3800000, 0x0800000, CRC(78ba02e8) SHA1(0f696a33e1e6671001efc309ed62f084a246ad24) )
	ROM_LOAD("mpr-23055.ic24",  0x4000000, 0x0800000, CRC(84319604) SHA1(c3dde162e043a54e1325202b46191b32e8784a1c) )
	ROM_LOAD("mpr-23056.ic25",  0x4800000, 0x0800000, CRC(d7386034) SHA1(be1f3ca5f283e428dc59dc072de3e7d36e122d53) )
	ROM_LOAD("mpr-23057.ic26",  0x5000000, 0x0800000, CRC(a3f087db) SHA1(b52d7c072cb5c2fdd10d0ac0b62cebe48b229ae3) )
	ROM_LOAD("mpr-23058.ic27",  0x5800000, 0x0800000, CRC(61a6cc5d) SHA1(34e52cb076888313a80f2b87876b8d37b91d85a0) )
	ROM_LOAD("mpr-23059.ic28",  0x6000000, 0x0800000, CRC(64808024) SHA1(1a6c60c330642b273978d3dd02d95d17d36ee3f2) )
	ROM_LOAD("mpr-23060.ic29",  0x6800000, 0x0800000, CRC(67519942) SHA1(fc758d9075625f8140d5d828c8f6b7a91bcc9119) )
	ROM_LOAD("mpr-23061.ic30",  0x7000000, 0x0800000, CRC(fb1844c4) SHA1(1d1571516a6dbed0c4ded3b80efde9cc9281f66f) )
        ROM_LOAD("mpr-23083.ic31",  0x7800000, 0x0400000, CRC(c61d2dfe) SHA1(a05fb979ed7c8040de91716fc8814e6bd995efa2) )
        ROM_LOAD("mpr-23084.ic32",  0x8000000, 0x0400000, CRC(e228cdfd) SHA1(d02a2e3557bd24cf34c5ddb42d41ca15e78ae885) )

	// DMA protection data
        ROM_LOAD("88000000.bin", 0x8800000, 0x025f00, CRC(77d79823) SHA1(2545d28eee47114e8ffb9bc6d7a910e90fc48420) )
        ROM_LOAD("88026440.bin", 0x8830000, 0x016520, CRC(dad9ebbd) SHA1(39c0697caa2b5ee11d99e75726e92ed86a23f10b) )
        ROM_LOAD("8803bda0.bin", 0x8850000, 0x01e5e0, CRC(9e0b8202) SHA1(729bed557c1a00da13c990603bbadab38d90285e) )
        ROM_LOAD("8805a560.bin", 0x8870000, 0x0017a0, CRC(7bc27482) SHA1(6ce6074cf47989f42af02deb7aac52883912784a) )
        ROM_LOAD("8805b720.bin", 0x8880000, 0x02b5a0, CRC(485d0aef) SHA1(853f4b49b489cc512c906edcaf3cd8b5bf4c64c0) )
        ROM_LOAD("8808b7e0.bin", 0x88a0000, 0x013ec0, CRC(0fc8f363) SHA1(dad30d43cef89d01ce80301b1d796aabad755de6) )
ROM_END

/* toy fighter - 1999 sega */

ROM_START( toyfight )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22035.ic22",0x0000000, 0x0400000, CRC(dbc76493) SHA1(a9772bdb62610a39adf2b9f397781bcddda3e635) )

	ROM_LOAD("mpr-22025.ic1", 0x0800000, 0x0800000, CRC(30237202) SHA1(e229a7671b3a34b26a461716bd7b437da100e1c8) )
	ROM_LOAD("mpr-22026.ic2", 0x1000000, 0x0800000, CRC(f28e71ff) SHA1(019425fcf234beca2b586de5235cf9f171563533) )
	ROM_LOAD("mpr-22027.ic3", 0x1800000, 0x0800000, CRC(1a84632d) SHA1(f3880f21399c6713c48c710c06d0344a0a28f026) )
	ROM_LOAD("mpr-22028.ic4", 0x2000000, 0x0800000, CRC(2b34ccba) SHA1(76c39ea19c3be1d9a9ce9e67035be7543b71ff26) )
	ROM_LOAD("mpr-22029.ic5", 0x2800000, 0x0800000, CRC(8162953a) SHA1(15c9e10080a5f2e70c31b9b89a256050a1aed4e9) )
	ROM_LOAD("mpr-22030.ic6", 0x3000000, 0x0800000, CRC(5bf5fed6) SHA1(6c8eedb177aa49aee9a8b090f2e5f96644416c6c) )
	ROM_LOAD("mpr-22031.ic7", 0x3800000, 0x0800000, CRC(ee7c40cc) SHA1(b9d92ef5bae0e932ec8769a30ebd841a263d3e2a) )
	ROM_LOAD("mpr-22032.ic8", 0x4000000, 0x0800000, CRC(3c48c9ba) SHA1(00be199b23040f8e81db2ec489ba98cbf615652c) )
	ROM_LOAD("mpr-22033.ic9", 0x4800000, 0x0800000, CRC(5fe5586e) SHA1(3ff41ae1f81469597684faadd88e62b5e0634352) )
	ROM_LOAD("mpr-22034.ic10",0x5000000, 0x0800000, CRC(3aa5ce5e) SHA1(f00a906235e4522d6fc2ac771324114346875314) )

	// trojaned protection data (filename is address read from)
	ROM_REGION( 0x4000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "a0309000.bin", 0x000000, 0x000400, CRC(e4f8b4a2) SHA1(a0d54aa6e1ffd5b1db31002ee4a94943c2157a3e) )
        ROM_LOAD( "a0309500.bin", 0x001000, 0x000400, CRC(5d3a9b15) SHA1(30d55a686493bb2f5e7d2c7d7df3dedf2f93dff0) )
        ROM_LOAD( "a030a000.bin", 0x002000, 0x000400, CRC(22b88804) SHA1(7b53778ddbf360b364376f0358b0acfb689e8713) )
        ROM_LOAD( "a030a500.bin", 0x003000, 0x000400, CRC(6a501389) SHA1(c2c75cd8beed6bb9e3fa39722fa01b35f2e7992d) )
ROM_END


/*

SYSTEMID: NAOMI
JAP: SLASHOUT JAPAN VERSION
USA: SLASHOUT USA VERSION
EXP: SLASHOUT EXPORT VERSION

NO.  Type   Byte    Word
IC22 32M    0000    0000
IC1  64M    D1BF    FB18
IC2  64M    1F98    4295
IC3  64M    5F61    67E3
IC4  64M    C6A4    449B
IC5  64M    BB2A    58AB
IC6  64M    60B2    5262
IC7  64M    178B    3705
IC8  64M    E4B9    FF46
IC9  64M    D4FC    2273
IC10 64M    6BA5    8087
IC11 64M    7DBA    A143
IC12 64M    B708    0C61
IC13 64M    0C4A    8DF0
IC14 64M    B2FF    A057
IC15 64M    60DB    3D06
IC16 64M    B5EA    4965
IC17 64M    6586    1F3F

*/

ROM_START( slasho )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x9000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23341.ic22", 0x0000000, 0x0400000, CRC(477fa123) SHA1(d2474766dcd0b0e5fe317a858534829eb1c26789) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23324.ic1",  0x0800000, 0x0800000, CRC(8624493a) SHA1(4fe940a889619f2a75c45e15efb2b8ed9020bc55) )
	ROM_LOAD("mpr-23325.ic2",  0x1000000, 0x0800000, CRC(f952d0d4) SHA1(4b5403b98bf977c1e3a045619e1eddb4e4ab69c7) )
	ROM_LOAD("mpr-23326.ic3",  0x1800000, 0x0800000, CRC(6c5ce16e) SHA1(110b5d536557ab6610a7c32db2e6e46901da9579) )
	ROM_LOAD("mpr-23327.ic4",  0x2000000, 0x0800000, CRC(1b3d02a0) SHA1(3e02a0cb3d945e5d6cea03236d6571d45a7afd51) )
	ROM_LOAD("mpr-23328.ic5",  0x2800000, 0x0800000, CRC(50053662) SHA1(3ced87ee533fd7a32d64c41f1fcbde9c648ab188) )
	ROM_LOAD("mpr-23329.ic6",  0x3000000, 0x0800000, CRC(96148e80) SHA1(c0f30556395edb9a7558006e89d6adc2f6bdc048) )
	ROM_LOAD("mpr-23330.ic7",  0x3800000, 0x0800000, CRC(15f2f9a1) SHA1(9cea71b6f6466ccd840218f5dcb09ea7525208d8) )
	ROM_LOAD("mpr-23331.ic8",  0x4000000, 0x0800000, CRC(a084ab51) SHA1(1f5c863012004bbeefc82b172a92011a175428a6) )
	ROM_LOAD("mpr-23332.ic9",  0x4800000, 0x0800000, CRC(50539e17) SHA1(38ec16a868c892e177fbb45be563e1b649956550) )
	ROM_LOAD("mpr-23333.ic10", 0x5000000, 0x0800000, CRC(29891831) SHA1(f318bca11ac5eb24b32d5b910a596280221a44ab) )
	ROM_LOAD("mpr-23334.ic11", 0x5800000, 0x0800000, CRC(c1ad0614) SHA1(e38ff316da889eb029d0a9348f6b2284f3a36f29) )
	ROM_LOAD("mpr-23335.ic12s",0x6000000, 0x0800000, CRC(faeb25ed) SHA1(623f3f78c94ba44e77491c18a6521a19b1101a67) )
	ROM_LOAD("mpr-23336.ic13s",0x6800000, 0x0800000, CRC(63589d0f) SHA1(53770cc1268892e8cdb76b6edf2fb39e8b605554) )
	ROM_LOAD("mpr-23337.ic14s",0x7000000, 0x0800000, CRC(2bc46263) SHA1(38ec579768ac37ed3ad21911b1970241906af8ea) )
	ROM_LOAD("mpr-23338.ic15s",0x7800000, 0x0800000, CRC(323e4db2) SHA1(c5484589c1613110faef6cf8b8f4def8867a8226) )
	ROM_LOAD("mpr-23339.ic16s",0x8000000, 0x0800000, CRC(fd8c2736) SHA1(34ae1a4e35b4aac6666719fb4fc0959bd64ff3d6) )
	ROM_LOAD("mpr-23340.ic17s",0x8800000, 0x0800000, CRC(001604f8) SHA1(615ec027d383d44d4aadb1175be6320e4139d7d1) )
ROM_END


/*

SYSTEMID: NAOMI
JAP: MOERO JUSTICE GAKUEN  JAPAN
USA: PROJECT JUSTICE  USA
EXP: PROJECT JUSTICE  EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     3E87    5491
IC2     64M     2789    9802
IC3     64M     60E7    E775
IC4     64M     36F4    9353
IC5     64M     31B6    CEF6
IC6     64M     3F79    7B58
IC7     64M     620C    A31F
IC8     64M     A093    160C
IC9     64M     4DD9    4184
IC10    64M     AF3F    C64A
IC11    64M     0EE1    A0C2
IC12    64M     2EF9    E0A3
IC13    64M     72A5    3156
IC14    64M     D414    B896
IC15    64M     7BCE    3A7A
IC16    64M     E371    962D
IC17    64M     E813    E342
IC18    64M     D2B8    3989
IC19    64M     3A4B    4614
IC20    64M     11B0    9921
IC21    64M     698C    7A39

Serial: BCLE-01A2130

*/

ROM_START( pjustic )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23548a.ic22", 0x0000000, 0x0400000,  CRC(f4ccf1ec) SHA1(97485b2a4b9452ffeea2501f42d20d718410e716) )
        ROM_LOAD( "mpr-23537.ic1",  0x0800000, 0x1000000, CRC(a2462770) SHA1(2d06f2efb686b2c45e5cc0b0776ba5fb1d392951) )
        ROM_LOAD( "mpr-23538.ic2",  0x1800000, 0x1000000, CRC(e4480832) SHA1(281700b10bd6b29e4d33b5230d085f9cc102fa01) )
        ROM_LOAD( "mpr-23539.ic3",  0x2800000, 0x1000000, CRC(97e3f7f5) SHA1(89ad30782ba148777ce3aad2d41e9dfda2dd0c5c) )
        ROM_LOAD( "mpr-23540.ic4",  0x3800000, 0x1000000, CRC(b9e92d21) SHA1(a9b465e83ecfbf47168f83ad4ae8ed4b802345ac) )
        ROM_LOAD( "mpr-23541.ic5",  0x4800000, 0x1000000, CRC(95b8a9c6) SHA1(2c9df93d9f599cf01d895a37d03ba0d86b9b3033) )
        ROM_LOAD( "mpr-23542.ic6",  0x5800000, 0x1000000, CRC(dfd490f5) SHA1(13fe0a11a75f1a7ebfe40433833241e656bb1511) )
        ROM_LOAD( "mpr-23543.ic7",  0x6800000, 0x1000000, CRC(66847ebd) SHA1(853d4fc7e53ac7b19b9ba616f756eb8a8fcd242d) )
        ROM_LOAD( "mpr-23544.ic8",  0x7800000, 0x1000000, CRC(d1f5b460) SHA1(f0789630871d728113abacceff21c6328a9fa9fc) )
        ROM_LOAD( "mpr-23545.ic9",  0x8800000, 0x1000000, CRC(60bd692f) SHA1(37b508f4a821d832eafff81574e7df3fe1c729f8) )
        ROM_LOAD( "mpr-23546.ic10", 0x9800000, 0x1000000, CRC(85db2248) SHA1(37845c269a2e65ee6181a8e7500c2e7dd9b2e343) )
        ROM_LOAD( "mpr-23547.ic11", 0xa800000, 0x1000000, CRC(18b369c7) SHA1(b61cb3fda8cc685865684f7afc7dad0b29d93ca5) )

	// trojaned protection data (filename is address read from)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
        ROM_LOAD( "a0820000.bin", 0x000000, 0x00267e, CRC(15db53ed) SHA1(a28ba1abc77ca559cff2fd34f3499c556ad8674e) )
        ROM_LOAD( "a0920000.bin", 0x010000, 0x00c91c, CRC(431436f8) SHA1(0136d6b210689b2023f982300bc8842b5c0914b4) )
        ROM_LOAD( "a092c920.bin", 0x020000, 0x00a53c, CRC(43e1a419) SHA1(34020cc7b10a1c5b2860db84d133489bce53d3d4) )
        ROM_LOAD( "a0936e60.bin", 0x030000, 0x001a32, CRC(2a998df6) SHA1(c7bed2777b2358bc0f1d7a3c23f3029591eb674d) )
        ROM_LOAD( "a3480000.bin", 0x040000, 0x008e9c, CRC(c8939e48) SHA1(a8d26c2b32211a50b8cc49b51c0d2e34416cc715) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: POWER STONE JAPAN
USA: POWER STONE USA
EXP: POWER STONE EURO

NO. Type    Byte    Word
IC22    16M 0000    0000
IC1 64M 0258    45D8
IC2 64M 0DF2    0810
IC3 64M 5F93    9FAF
IC4 64M 05E0    C80F
IC5 64M F023    3F68
IC6 64M 941E    F563
IC7 64M 374E    46F6
IC8 64M C529    0501

prot

*/

ROM_START( pstone )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x4800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-21597.ic22",0x0000000, 0x0200000, CRC(62c7acc0) SHA1(bb61641a7f3650757132cde379447bdc9bd91c78) )
	ROM_LOAD("mpr-21589.ic1", 0x0800000, 0x0800000, CRC(2fa66608) SHA1(144bda75f892a1e4dbd8332439e9e44fad1d0695) )
	ROM_LOAD("mpr-21590.ic2", 0x1000000, 0x0800000, CRC(6341b399) SHA1(d123b6a3eb7c4800950cc5849d748b0edafabc7d) )
	ROM_LOAD("mpr-21591.ic3", 0x1800000, 0x0800000, CRC(7f2d99aa) SHA1(00f9ae67be0d7229c37479b6dc0ed5816035fd98) )
	ROM_LOAD("mpr-21592.ic4", 0x2000000, 0x0800000, CRC(6ebe3b25) SHA1(c7dec77d55b0fcf1d230311b24553581a90a7d22) )
	ROM_LOAD("mpr-21593.ic5", 0x2800000, 0x0800000, CRC(84366f3e) SHA1(c61985f627813db2e16182e437ab4a69d5253c9f) )
	ROM_LOAD("mpr-21594.ic6", 0x3000000, 0x0800000, CRC(ddfa0467) SHA1(e758eae50035d5f18d99dbed728513e306d9566f) )
	ROM_LOAD("mpr-21595.ic7", 0x3800000, 0x0800000, CRC(7ab218f7) SHA1(c5c022e63f926cce09d49331647cde20e8e42ab3) )
	ROM_LOAD("mpr-21596.ic8", 0x4000000, 0x0800000, CRC(f27dbdc5) SHA1(d54717d62897546968de2f049239f68bee49bdd8) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: POWER STONE 2 JAPAN
USA: POWER STONE 2 USA
EXP: POWER STONE 2 EURO

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     04FF    B3D4
IC2     64M     52D4    0BF0
IC3     64M     5273    0EB8
IC4     64M     B39A    21F5
IC5     64M     53CB    6540
IC6     64M     0AC8    74ED
IC7     64M     D05A    EB30
IC8     64M     8217    4E66
IC9     64M     193C    6851

Serial: BBJE-01A1613

*/

ROM_START( pstone2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x5000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23127.ic22", 0x0000000, 0x0400000,  CRC(185761d6) SHA1(8c91b594dd59313d249c9da7b39dee21d3c9082e) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-23118.ic1", 0x0800000, 0x0800000, CRC(c69f3c3c) SHA1(e96ad24473197f8581f5e4398244b9b76957bfdd) )
	ROM_LOAD("mpr-23119.ic2", 0x1000000, 0x0800000, CRC(a80d444d) SHA1(a7d2a5831412134a26ba37bf83e5ce38eb9f3928) )
	ROM_LOAD("mpr-23120.ic3", 0x1800000, 0x0800000, CRC(c285dd64) SHA1(e64507caedb9f312ab291b41b8d7fe8922eb434e) )
	ROM_LOAD("mpr-23121.ic4", 0x2000000, 0x0800000, CRC(1f3f6505) SHA1(da5eb3b9b5c85f5f0b4afe0c0ee8d034108300a2) )
	ROM_LOAD("mpr-23122.ic5", 0x2800000, 0x0800000, CRC(5e403a12) SHA1(e71c15e63c30e60b6db1fcd2841f66490f31579a) )
	ROM_LOAD("mpr-23123.ic6", 0x3000000, 0x0800000, CRC(4b71078b) SHA1(f3ed39402f585ae5cf6f8987bf6be6c6d46eafa1) )
	ROM_LOAD("mpr-23124.ic7", 0x3800000, 0x0800000, CRC(508c0207) SHA1(e50d97a17cdd6771fbc63a254a4d638e7daa8f57) )
	ROM_LOAD("mpr-23125.ic8", 0x4000000, 0x0800000, CRC(b9938bbc) SHA1(d55d7adecb5a5a4a276a5a17c12808085d980fd9) )
	ROM_LOAD("mpr-23126.ic9", 0x4800000, 0x0800000, CRC(fbb0325b) SHA1(21b965519d7508d84344641d43e8af2c3ca29ba4) )

	// trojaned protection data (filename is address read from)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
ROM_END

/*

SYSTEMID: NAOMI
JAP: OUTTRIGGER     JAPAN
USA: OUTTRIGGER     USA
EXP: OUTTRIGGER     EXPORT

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     362E    D34B
IC2     64M     4EF4    FF8D
IC3     64M     5E77    9052
IC4     64M     E123    41B3
IC5     64M     43A0    58D4
IC6     64M     C946    D3EE
IC7     64M     5313    3F17
IC8     64M     2591    FEB7
IC9     64M     CBA3    E150
IC10    64M     2639    D291
IC11    64M     3A96    86EA
IC12    64M     8586    3ED5
IC13    64M     9028    E59C
IC14    64M     8A42    26E2
IC15    64M     98C4    1618
IC16    64M     122B    8C85
IC17    64M     3D5E    F9B0
IC18    64M     1EFA    490E
IC19    64M     9F22    6F77

Serial (from 2 carts): BAZE-01A0288
                       BAZE-02A0217

*/

ROM_START( otrigger )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xa000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22163.ic22", 0x0000000, 0x0400000, CRC(3bdafb6a) SHA1(c4c5a4ba94d85c4353df22d70bb08be67e9c22c3) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-22142.ic1",  0x0800000, 0x0800000, CRC(5b45fa35) SHA1(7d3fbecc6f0dce2b13bfb21ed68f44632b91b94b) )
	ROM_LOAD("mpr-22143.ic2",  0x1000000, 0x0800000, CRC(b43c4d6d) SHA1(77e0b37ca3ee94b7f77d88ccb14bd0469a76aac0) )
	ROM_LOAD("mpr-22144.ic3",  0x1800000, 0x0800000, CRC(e78581af) SHA1(d1fe4da3f16dd5ebc7d9eaa092de1e16ec9c3321) )
	ROM_LOAD("mpr-22145.ic4",  0x2000000, 0x0800000, CRC(2b6274ea) SHA1(89165cf84ebb02e99163624c6d31da38aeec000e) )
	ROM_LOAD("mpr-22146.ic5",  0x2800000, 0x0800000, CRC(c24eb03f) SHA1(2f4b720b4ab106f891f4469b6e93a9979b1c1061) )
	ROM_LOAD("mpr-22147.ic6",  0x3000000, 0x0800000, CRC(578e36fd) SHA1(f39f74b046efbff7e7baf70effdd368605da496f) )
	ROM_LOAD("mpr-22148.ic7",  0x3800000, 0x0800000, CRC(e6053373) SHA1(e7bafaffeac9b6851a3fce060be21e8be8eaa71e) )
	ROM_LOAD("mpr-22149.ic8",  0x4000000, 0x0800000, CRC(cc86691b) SHA1(624958bc07eef5fac98642e9acd460cd5fe0c815) )
	ROM_LOAD("mpr-22150.ic9",  0x4800000, 0x0800000, CRC(f585d41d) SHA1(335df3d3f2631e5c03c39465cd702b77ce3f9717) )
	ROM_LOAD("mpr-22151.ic10", 0x5000000, 0x0800000, CRC(aae31a4b) SHA1(1472e477c2c6b89ca03824838757bdf20efbdf45) )
	ROM_LOAD("mpr-22152.ic11", 0x5800000, 0x0800000, CRC(5ed2c5ea) SHA1(2b9237eda566ccb87b4914db61a03e2c9035a280) )
	ROM_LOAD("mpr-22153.ic12s",0x6000000, 0x0800000, CRC(16630b85) SHA1(10e926c0d13270b5bf99d7456fe63baafc2df56a) )
	ROM_LOAD("mpr-22154.ic13s",0x6800000, 0x0800000, CRC(30a2d60b) SHA1(6431b2d4e5106e25e5517707c9667bcd714f43ac) )
	ROM_LOAD("mpr-22155.ic14s",0x7000000, 0x0800000, CRC(163993a5) SHA1(351a626a0dc9a3030b10fc0b822075f3010fdc05) )
	ROM_LOAD("mpr-22156.ic15s",0x7800000, 0x0800000, CRC(37720b4f) SHA1(bd60beadb0081ed20610c3988577bbf37bfdab07) )
	ROM_LOAD("mpr-22157.ic16s",0x8000000, 0x0800000, CRC(dfd6fa83) SHA1(e0dc9606f5521af16c29a30378e81843c8dbc188) )
	ROM_LOAD("mpr-22158.ic17s",0x8800000, 0x0800000, CRC(f5d96fe9) SHA1(d5d0ac3d6b7c9b851a18b22d5fb599710c684a76) )
	ROM_LOAD("mpr-22159.ic18s",0x9000000, 0x0800000, CRC(f8b5e99d) SHA1(bb174a6a80967d0ff05c3a7512e4f0f9c921d130) )
	ROM_LOAD("mpr-22160.ic19s",0x9800000, 0x0800000, CRC(579eef4e) SHA1(bfcabd57f623647053afcedcabfbc74e5736819f) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: AH! MY GODDESS QUIZ GAME--
USA: AH! MY GODDESS QUIZ GAME--
EXP: AH! MY GODDESS QUIZ GAME--

*/

ROM_START( qmegamis )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x9000200, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23227.ic11", 0x0000000, 0x0400000, CRC(3f76087e) SHA1(664d28ef95394590b186e7badaf96ddaf781c104) ) //ic 22
	ROM_RELOAD( 0x400000, 0x400000)
	/*ic 1 empty */
	ROM_LOAD32_WORD("mpr-23211.ic17", 0x1000000, 0x0800000, CRC(a46670e7) SHA1(dbcb72fdf444f07ce986af329e7ff2cb42721729) ) //ic 2
	ROM_LOAD32_WORD("mpr-23212.ic18", 0x1000002, 0x0800000, CRC(5e6839d5) SHA1(678c706f5c1eee65b32d9455ca4d0803c38349bd) ) //ic 3
	ROM_LOAD32_WORD("mpr-23213.ic19", 0x2000000, 0x0800000, CRC(98e5e2c1) SHA1(1d5338c625fcd979dd841c3e5de09a3bd3d239b6) ) //ic 4
	ROM_LOAD32_WORD("mpr-23214.ic20", 0x2000002, 0x0800000, CRC(37cfdd37) SHA1(e9097af91a164b6ffeed98008e85f4d4f00894df) ) //ic 5
	ROM_LOAD32_WORD("mpr-23215.ic21", 0x3000000, 0x0800000, CRC(f0d97107) SHA1(35422cd3238f516243fa6d1f282d802ff4f4ab17) ) //ic 6
	ROM_LOAD32_WORD("mpr-23216.ic22", 0x3000002, 0x0800000, CRC(68a9363f) SHA1(ca9a015f18041bff1a0c57a61a50143c42115f9d) ) //ic 7
	ROM_LOAD32_WORD("mpr-23217.ic23", 0x4000000, 0x0800000, CRC(1b49de82) SHA1(07b115d8e66f02fe3f184c23353fb11452dcb2b4) ) //ic 8
	ROM_LOAD32_WORD("mpr-23218.ic24", 0x4000002, 0x0800000, CRC(2c2c1e42) SHA1(ac8b1d580a8aaef184415ac4572eb9b2d5f37cf8) ) //ic 9
	ROM_LOAD32_WORD("mpr-23219.ic25", 0x5000000, 0x0800000, CRC(62957622) SHA1(ca7e2cd009fb38db3c25896ef8206350a7221fc8) ) //ic 10
	ROM_LOAD32_WORD("mpr-23220.ic26", 0x5000002, 0x0800000, CRC(70c2bea3) SHA1(3fe1d806358a35eced1f1c3a83e3593d92c3cf52) ) //ic 11
	ROM_LOAD32_WORD("mpr-23221.ic27", 0x6000000, 0x0800000, CRC(eb6a522e) SHA1(a7928d2296d67f7913d6582bdb6cd58b09d01673) ) //ic 12
	ROM_LOAD32_WORD("mpr-23222.ic28", 0x6000002, 0x0800000, CRC(f7d932bd) SHA1(e2b53fd30af5f45160aa988e3a80cee9330f0deb) ) //ic 13
	ROM_LOAD32_WORD("mpr-23223.ic29", 0x7000000, 0x0800000, CRC(2f8c15e0) SHA1(55c8554404263b629172d30dbb240104ab352c0f) ) //ic 14
	ROM_LOAD32_WORD("mpr-23224.ic30", 0x7000002, 0x0800000, CRC(bed270e1) SHA1(342199ac5903681f2bfdb9dfd57ce06202f14685) ) //ic 15
	ROM_LOAD32_WORD("mpr-23225.ic31", 0x8000000, 0x0800000, CRC(ea558614) SHA1(b7dfe5598639a8e59e3cbbee38b1d9a1d8e022ea) ) //ic 16
	ROM_LOAD32_WORD("mpr-23226.ic32", 0x8000002, 0x0800000, CRC(cd5da506) SHA1(2e76c8892c1d389b0f12a0046213f43d2ab07d78) ) //ic 17

	// trojaned protection data (must be in the "user1" region because it's accessed via DMA)
	ROM_LOAD( "81452000_dma.bin", 0x9000000, 0x000120, CRC(96049488) SHA1(e2b98e8986f8cbf026db50a652300081a8e470b0) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: SAMBA DE AMIGO
USA: SAMBADEAMIGO
EXP: SAMBADEAMIGO

NO.     Type    Byte    Word
IC22    32M     0000    0000
IC1     64M     B1FA    1BE9
IC2     64M     51FD    0C32
IC3     64M     8AA0    6E7A
IC4     64M     3B30    E31D
IC5     64M     D604    FBE3
IC6     64M     1D51    FF2D
IC7     64M     EE89    720D
IC8     64M     0551    7046
IC9     64M     6883    6427
IC10    64M     70E5    CEC3
IC11    64M     E70E    0C63
IC12    64M     0FD0    B1F8
IC13    64M     2D48    6B19
IC14    64M     CBFF    F163
IC15    64M     10D1    E09D
IC16    64M     A10B    DDB4

*/

ROM_START( samba )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x8800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22966b.ic22",0x0000000, 0x0400000, CRC(893116b8) SHA1(35cb4f40690ff21af5ab7cc5adbc53228d6fb0b3) )
	ROM_LOAD("mpr-22950.ic1",  0x0800000, 0x0800000, CRC(16dee15c) SHA1(b46849e492756ff406bf8956303472255fcf55a5) )
	ROM_LOAD("mpr-22951.ic2",  0x1000000, 0x0800000, CRC(f509496f) SHA1(41281576f7d58c8ede9c0a89bfd46a98d5b97033) )
	ROM_LOAD("mpr-22952.ic3",  0x1800000, 0x0800000, CRC(fb9b3ef0) SHA1(e9d44b673c273e97445a12186496a0594e291542) )
	ROM_LOAD("mpr-22953.ic4",  0x2000000, 0x0800000, CRC(07207ce0) SHA1(b802bb4e78f3737a4e333f819b9a4e0249037288) )
	ROM_LOAD("mpr-22954.ic5",  0x2800000, 0x0800000, CRC(c8e797d1) SHA1(fadbd1e24882787634229003245293ce79ba2617) )
	ROM_LOAD("mpr-22955.ic6",  0x3000000, 0x0800000, CRC(064ef007) SHA1(8325f9aa537ce329e71dce2b588a3d4fc176c37b) )
	ROM_LOAD("mpr-22956.ic7",  0x3800000, 0x0800000, CRC(fe8f2964) SHA1(3a33162f797cd93b7dbb313b531215e340719110) )
	ROM_LOAD("mpr-22957.ic8",  0x4000000, 0x0800000, CRC(74842c01) SHA1(b02884925270edb66831ab502a0aa2f9430adc9f) )
	ROM_LOAD("mpr-22958.ic9",  0x4800000, 0x0800000, CRC(b1ead447) SHA1(06b848eb7f592763768050a1ae82b4cac9499684) )
	ROM_LOAD("mpr-22959.ic10", 0x5000000, 0x0800000, CRC(d32d7983) SHA1(86a9e5eae4598b6998f0ea578d6152e66c1a0df1) )
	ROM_LOAD("mpr-22960.ic11", 0x5800000, 0x0800000, CRC(6c3b228e) SHA1(782c0fda106222be75b1973586c8bf78fd2186e7) )
	ROM_LOAD("mpr-22961.ic12s",0x6000000, 0x0800000, CRC(d6d26a8d) SHA1(7d416f8ac9fbbeb9bfe217ccc8eccf1644511110) )
	ROM_LOAD("mpr-22962.ic13s",0x6800000, 0x0800000, CRC(c2f41101) SHA1(0bf87cbffb7d6a5ab32543cef56c9759f475419a) )
	ROM_LOAD("mpr-22963.ic14s",0x7000000, 0x0800000, CRC(a53e9919) SHA1(d81eb79bc706f85ebfbc56a9b2889ae62d629e8e) )
	ROM_LOAD("mpr-22964.ic15s",0x7800000, 0x0800000, CRC(f581d5a3) SHA1(8cf769f5b0a48951246bb60e9cf58232bcee7bc8) )
	ROM_LOAD("mpr-22965.ic16s",0x8000000, 0x0800000, CRC(8f7bfa8a) SHA1(19f137b1552978d232785c4408805b71835585c6) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: SEGA MARINE FISHING IN JAPAN
USA: SEGA MARINE FISHING IN USA
EXP: SEGA MARINE FISHING IN EXPORT

*/

ROM_START( smarinef )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x6800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22221.ic22",0x0000000, 0x0400000, CRC(9d984375) SHA1(fe1185d70b4bc1529e3579fd6b2b678c7d548400) )
	ROM_LOAD("mpr-22208.ic1", 0x0800000, 0x0800000, CRC(6a1e418c) SHA1(7092c6a34ac0c2c6fb2b4b78415d08ef473785d9) )
	ROM_LOAD("mpr-22209.ic2", 0x1000000, 0x0800000, CRC(ecf5be54) SHA1(d7c264da4e232ce6f9b05c9920394f8027fa4a1d) )
	/* IC3 empty */
	/* IC4 empty */
	ROM_LOAD("mpr-22212.ic5", 0x2800000, 0x0800000, CRC(8305f462) SHA1(7993231fa71f509b3b7fec691b5a6139947a01e7) )
	ROM_LOAD("mpr-22213.ic6", 0x3000000, 0x0800000, CRC(0912eaea) SHA1(e4cb1262f3b53d3c619900767cfa192115a53d4b) )
	ROM_LOAD("mpr-22214.ic7", 0x3800000, 0x0800000, CRC(661526b6) SHA1(490321a893f706eaea49c6c35c01af6ae45adf01) )
	ROM_LOAD("mpr-22215.ic8", 0x4000000, 0x0800000, CRC(a80714fa) SHA1(b32dde5cc79a9ae9f7f34064c2382115e9303070) )
	ROM_LOAD("mpr-22216.ic9", 0x4800000, 0x0800000, CRC(cf3d1049) SHA1(a390304256dfac623b6fe1b205d918ce3eb67723) )
	ROM_LOAD("mpr-22217.ic10",0x5000000, 0x0800000, CRC(48c92fd6) SHA1(26b17a8d0130512807cf533a60c10c6d1e769de0) )
	ROM_LOAD("mpr-22218.ic11",0x5800000, 0x0800000, CRC(f9ca31b8) SHA1(ea3d0f38ca1a46c896c06f038a6362ad3c9f90b2) )
	ROM_LOAD("mpr-22219.ic12",0x6000000, 0x0800000, CRC(b3b45811) SHA1(045e7236b814f848d4c9767618ddcd4344d880ec) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: SHOOTOUT POOL
USA: SHOOTOUT POOL
EXP: SHOOTOUT POOL PRIZE

*/

ROM_START( shootpl )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x2800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-a-24065.rom0.ic11",0x0000000, 0x0400000, CRC(622a9ba0) SHA1(2f4963b8447ecda78fea0107497c2811f075c07a) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD32_WORD("opr-24060.rom1.ic17s", 0x0800000, 0x0800000, CRC(7f3d868c) SHA1(dc352981371c5479a69756bb1cbbbca43252216d) )
	ROM_LOAD32_WORD("opr-24061.rom2.ic18",  0x0800002, 0x0800000, CRC(e934267c) SHA1(fdbe2b80e309aa8d9fefd2634aef20153735019d) )
	ROM_LOAD32_WORD("opr-24062.rom3.ic19s", 0x1800000, 0x0800000, CRC(26e32af4) SHA1(49412a04198175240ef9adb4b7afb8a628eb127d) )
	ROM_LOAD32_WORD("opr-24063.rom4.ic20",  0x1800002, 0x0800000, CRC(683fdcff) SHA1(890816ef1b3e604e16289998cf66e221ef75a0fe) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: SPAWN JAPAN
USA: SPAWN USA
EXP: SPAWN EURO

NO.     Type    Byte    Word
IC22    32M     FFFF    FFFF
IC1     64M     C56E    3D11
IC2     64M     A206    CC87
IC3     64M     FD3F    C5DF
IC4     64M     5833    09A4
IC5     64M     B42C    AA08
IC6     64M     C7A4    E2DE
IC7     64M     58CB    5DFD
IC8     64M     144B    783D
IC9     64M     A4A8    D0BE
IC10    64M     94A8    401F

Serial: BAVE-02A1305

*/

ROM_START( spawn )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x5800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22977b.ic22",0x0000000, 0x0400000, CRC(814ff5d1) SHA1(5a0a9e55878927f98750000eb7d9391cbecfe21d) )
	ROM_LOAD("mpr-22967.ic1",  0x0800000, 0x0800000, CRC(78c7d914) SHA1(0a000e396f9a83c2c777cfb61212a82ec17417ba) )
	ROM_LOAD("mpr-22968.ic2",  0x1000000, 0x0800000, CRC(8c4ae1bb) SHA1(a91934a01d306c8fd8f987b3013f33aec028de70) )
	ROM_LOAD("mpr-22969.ic3",  0x1800000, 0x0800000, CRC(2928627c) SHA1(146844edd22b0caf00b40f7635c8753d5758e958) )
	ROM_LOAD("mpr-22970.ic4",  0x2000000, 0x0800000, CRC(12e27ffd) SHA1(d09096cd1ff9218cd849bfe05b34ec4d642e1663) )
	ROM_LOAD("mpr-22971.ic5",  0x2800000, 0x0800000, CRC(993d2bce) SHA1(f04e484704dbddbbff0f36ac5a019fbdde56d402) )
	ROM_LOAD("mpr-22972.ic6",  0x3000000, 0x0800000, CRC(e0f75067) SHA1(741ebef9e7ae6e5207f1819c3eea80491b934c63) )
	ROM_LOAD("mpr-22973.ic7",  0x3800000, 0x0800000, CRC(698498ca) SHA1(b3691409cbf644b8acea01abaebf7b2dea4dd4f7) )
	ROM_LOAD("mpr-22974.ic8",  0x4000000, 0x0800000, CRC(20983c51) SHA1(f7321abf8bf5f2a7329c98174e5cf9b1ebf596b2) )
	ROM_LOAD("mpr-22975.ic9",  0x4800000, 0x0800000, CRC(0d3c70d1) SHA1(22920bc5fd1dda760b5cb17482e9181be899bc08) )
	ROM_LOAD("mpr-22976.ic10", 0x5000000, 0x0800000, CRC(092d8063) SHA1(14fafd3f4c4f2b37172453d1c815fb9b8f4814f4) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: THE TYPING OF THE DEAD
USA: THE TYPING OF THE DEAD
EXP: THE TYPING OF THE DEAD

*/

ROM_START( totd )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23021a.ic22", 0x0000000, 0x0400000,  CRC(07d21033) SHA1(d1e619d13c1c01648eb1a6964aad1554dd16c6d5) )

	ROM_LOAD("mpr-23001.ic1", 0x0800000, 0x0800000, CRC(2eaab8ed) SHA1(e078bd8781e2a04e23fd18b11d118b2548fa59a8) )
	ROM_LOAD("mpr-23002.ic2", 0x1000000, 0x0800000, CRC(617edcc7) SHA1(10f92cd9be94739c7c2f94cf9a5fa54accbe6227) )
	ROM_LOAD("mpr-23003.ic3", 0x1800000, 0x0800000, CRC(37d6d9f8) SHA1(3ad3fa65f33d250eb8a620e7dc7c6b1209794a80) )
	ROM_LOAD("mpr-23004.ic4", 0x2000000, 0x0800000, CRC(e41186f2) SHA1(2f4b26d8dba1629db539736cf88ec85c21820aeb) )
	ROM_LOAD("mpr-23005.ic5", 0x2800000, 0x0800000, CRC(2b8e1fc6) SHA1(a5cd8c5840dd316dd1ad9500804b459476ca8ba0) )
	ROM_LOAD("mpr-23006.ic6", 0x3000000, 0x0800000, CRC(3de23e27) SHA1(d3aae2a7e5c78fc3bf8e296392d8f893961d946f) ) //on board but actually 0xff filled
	ROM_LOAD("mpr-23007.ic7", 0x3800000, 0x0800000, CRC(ca16cfdf) SHA1(6279bc9bd661bde2d3e36ca52625f9b91867c4b4) )
	ROM_LOAD("mpr-23008.ic8", 0x4000000, 0x0800000, CRC(8c33191c) SHA1(6227fbb3d51c4301dd1fc60ec43df7c18eef06fa) )
	ROM_LOAD("mpr-23009.ic9", 0x4800000, 0x0800000, CRC(c982d24d) SHA1(d5a15d04f19f5569709b0b1cde64814230f4f0bb) )
	ROM_LOAD("mpr-23010.ic10",0x5000000, 0x0800000, CRC(c6e129b4) SHA1(642a9e1052efcb43d2b809f13d10617b43bd38f3) )
	ROM_LOAD("mpr-23011.ic11",0x5800000, 0x0800000, CRC(9e6942ff) SHA1(8c657d7d74c4c9106756a9934bc3c850f5069e29) )
	ROM_LOAD("mpr-23012.ic12s",0x6000000, 0x0800000, CRC(20e1ebe8) SHA1(e24cb5f48101e665c90af9be333e54ec274004fb) )
	ROM_LOAD("mpr-23013.ic13s",0x6800000, 0x0800000, CRC(3de23e27) SHA1(d3aae2a7e5c78fc3bf8e296392d8f893961d946f) ) //on board but actually 0xff filled
	ROM_LOAD("mpr-23014.ic14s",0x7000000, 0x0800000, CRC(c4f95fdb) SHA1(8c0e806e27d7bed274dcb20b932897ea8b8bbf86) )
	ROM_LOAD("mpr-23015.ic15s",0x7800000, 0x0800000, CRC(5360c49d) SHA1(dbdf955d9bb9a387ded8ada18d26d222d73514d7) )
	ROM_LOAD("mpr-23016.ic16s",0x8000000, 0x0800000, CRC(fae2958b) SHA1(2bfe164723b7b2f57ae0c6e2fe348459f00dc460) )
	ROM_LOAD("mpr-23017.ic17s",0x8800000, 0x0800000, CRC(22337e15) SHA1(6a9f5569177c2936d8ff04da74e1fd036a093422) )
	ROM_LOAD("mpr-23018.ic18s",0x9000000, 0x0800000, CRC(5a608e74) SHA1(4f2ec47dad71d77ad1b8c640db236332c06d7ab7) )
	ROM_LOAD("mpr-23019.ic19s",0x9800000, 0x0800000, CRC(5cc91cc4) SHA1(66a68991f716ec23555784163aa5140b4e44c7ab) )
	ROM_LOAD("mpr-23020.ic20s",0xa000000, 0x0800000, CRC(b5943007) SHA1(d0e95084aec5e05027c21a6b4a3331408853781b) )
	//ic21 not populated
ROM_END

/*

SYSTEMID: NAOMI
JAP: VIRTUA NBA
USA: VIRTUA NBA
EXP: VIRTUA NBA

NO.     Type    Byte    Word
IC22    32M 0000    0000
IC1     64M 5C4A    BB88
IC2     64M 1799    B55E
IC3     64M FB19    6FE8
IC4     64M 6207    33FE
IC5     64M 38F0    F24C
IC6     64M A3B1    FF6F
IC7     64M 737F    B4DD
IC8     64M FD19    49CE
IC9     64M 424E    76D5
IC10    64M 84CC    B74C
IC11    64M 8FC6    D9C8
IC12    64M A838    143A
IC13    64M 88C3    456F
IC14    64M 1C72    971E
IC15    64M B950    F203
IC16    64M 39F6    54CE
IC17    64M 91C7    47B0
IC18    64M 5B94    7E77
IC19    64M DE42    F390
IC20    64M B876    73CE
IC21    64M AD60    2F74

*/

ROM_START( virnba )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23073.ic22",0x0000000, 0x0400000, CRC(ce5c3d28) SHA1(ca3eeae1cf78435787338bb7b3e71301c0f71dd9) )
	ROM_LOAD("mpr-22928.ic1", 0x0800000, 0x0800000, CRC(63245c98) SHA1(a5a542244f07c6c8b66961a231fb56c89d2cf20c) )
	ROM_LOAD("mpr-22929.ic2", 0x1000000, 0x0800000, CRC(eea89d21) SHA1(5fe184267e637f155d767f8d931462d9593eff5a) )
	ROM_LOAD("mpr-22930.ic3", 0x1800000, 0x0800000, CRC(2fbefa9a) SHA1(a6df46cb8742022e436cdc6a9a50490c7a551421) )
	ROM_LOAD("mpr-22931.ic4", 0x2000000, 0x0800000, CRC(7332e559) SHA1(9147b69f84713f8e6c2c84b71ccd48bae879c655) )
	ROM_LOAD("mpr-22932.ic5", 0x2800000, 0x0800000, CRC(ef80e18c) SHA1(51406b82c66dc1822657948c62e1c4b8e628a739) )
	ROM_LOAD("mpr-22933.ic6", 0x3000000, 0x0800000, CRC(6a374076) SHA1(3b7c1ce5e3ae027e578c60a885724deeadc07448) )
	ROM_LOAD("mpr-22934.ic7", 0x3800000, 0x0800000, CRC(72f3ee15) SHA1(cf81e47c311769c9dc38fdbbef1a5e3f6b8a0be1) )
	ROM_LOAD("mpr-22935.ic8", 0x4000000, 0x0800000, CRC(35fda6e9) SHA1(857b3c0f576d69d3637503fa53608bc6484eb331) )
	ROM_LOAD("mpr-22936.ic9", 0x4800000, 0x0800000, CRC(b26df107) SHA1(900f1d06fdc9b6951de1b7e61a27ac846b2061db) )
	ROM_LOAD("mpr-22937.ic10",0x5000000, 0x0800000, CRC(477a374b) SHA1(309b723f7d2840d6a2f24ad2f877928cc8138a12) )
	ROM_LOAD("mpr-22938.ic11",0x5800000, 0x0800000, CRC(d59431a4) SHA1(6e3cd8cbde18a6a8672aa302cb119e486c0417e0) )
	ROM_LOAD("mpr-22939.ic12",0x6000000, 0x0800000, CRC(b31d3e6d) SHA1(d55e56a66dc678b973c3d60d3cffb59032bc3c46) )
	ROM_LOAD("mpr-22940.ic13",0x6800000, 0x0800000, CRC(90a81fbf) SHA1(5066b5eda80e881f6f399722f010161c0a452922) )
	ROM_LOAD("mpr-22941.ic14",0x7000000, 0x0800000, CRC(8a72a77d) SHA1(5ce73a76c7915d5a19b05f57b1dfdcd1fe3c53a1) )
	ROM_LOAD("mpr-22942.ic15",0x7800000, 0x0800000, CRC(710f709f) SHA1(2483f0b1106bc82710457a148772e50e83a439d8) )
	ROM_LOAD("mpr-22943.ic16",0x8000000, 0x0800000, CRC(c544f593) SHA1(553af7b6c63d6d6221c4286b8a13840a86e55d5f) )
	ROM_LOAD("mpr-22944.ic17",0x8800000, 0x0800000, CRC(cb096baa) SHA1(cbc267953a749dd24a03d87b65bc19b19bebf205) )
	ROM_LOAD("mpr-22945.ic18",0x9000000, 0x0800000, CRC(f2f914e8) SHA1(ec600abde40bfb5004ec8200ee0eef9410ebca6a) )
	ROM_LOAD("mpr-22946.ic19",0x9800000, 0x0800000, CRC(c79696c5) SHA1(4a9ac8b4ae1ce5d196e6c74fecc241b74aebc4ab) )
	ROM_LOAD("mpr-22947.ic20",0xa000000, 0x0800000, CRC(5e5eb595) SHA1(401d4a11d436988d716bb014b36233f171dc576d) )
	ROM_LOAD("mpr-22948.ic21",0xa800000, 0x0800000, CRC(1b0de917) SHA1(fd1742ea9bb2f1ce871ee3266171f26634e1c8e7) )
ROM_END

ROM_START( virnbao )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22949.ic22",0x0000000, 0x0400000, CRC(fd91447e) SHA1(0759d6517aeb684d0cb809c1ae1350615cc0aecc) )
	ROM_LOAD("mpr-22928.ic1", 0x0800000, 0x0800000, CRC(63245c98) SHA1(a5a542244f07c6c8b66961a231fb56c89d2cf20c) )
	ROM_LOAD("mpr-22929.ic2", 0x1000000, 0x0800000, CRC(eea89d21) SHA1(5fe184267e637f155d767f8d931462d9593eff5a) )
	ROM_LOAD("mpr-22930.ic3", 0x1800000, 0x0800000, CRC(2fbefa9a) SHA1(a6df46cb8742022e436cdc6a9a50490c7a551421) )
	ROM_LOAD("mpr-22931.ic4", 0x2000000, 0x0800000, CRC(7332e559) SHA1(9147b69f84713f8e6c2c84b71ccd48bae879c655) )
	ROM_LOAD("mpr-22932.ic5", 0x2800000, 0x0800000, CRC(ef80e18c) SHA1(51406b82c66dc1822657948c62e1c4b8e628a739) )
	ROM_LOAD("mpr-22933.ic6", 0x3000000, 0x0800000, CRC(6a374076) SHA1(3b7c1ce5e3ae027e578c60a885724deeadc07448) )
	ROM_LOAD("mpr-22934.ic7", 0x3800000, 0x0800000, CRC(72f3ee15) SHA1(cf81e47c311769c9dc38fdbbef1a5e3f6b8a0be1) )
	ROM_LOAD("mpr-22935.ic8", 0x4000000, 0x0800000, CRC(35fda6e9) SHA1(857b3c0f576d69d3637503fa53608bc6484eb331) )
	ROM_LOAD("mpr-22936.ic9", 0x4800000, 0x0800000, CRC(b26df107) SHA1(900f1d06fdc9b6951de1b7e61a27ac846b2061db) )
	ROM_LOAD("mpr-22937.ic10",0x5000000, 0x0800000, CRC(477a374b) SHA1(309b723f7d2840d6a2f24ad2f877928cc8138a12) )
	ROM_LOAD("mpr-22938.ic11",0x5800000, 0x0800000, CRC(d59431a4) SHA1(6e3cd8cbde18a6a8672aa302cb119e486c0417e0) )
	ROM_LOAD("mpr-22939.ic12",0x6000000, 0x0800000, CRC(b31d3e6d) SHA1(d55e56a66dc678b973c3d60d3cffb59032bc3c46) )
	ROM_LOAD("mpr-22940.ic13",0x6800000, 0x0800000, CRC(90a81fbf) SHA1(5066b5eda80e881f6f399722f010161c0a452922) )
	ROM_LOAD("mpr-22941.ic14",0x7000000, 0x0800000, CRC(8a72a77d) SHA1(5ce73a76c7915d5a19b05f57b1dfdcd1fe3c53a1) )
	ROM_LOAD("mpr-22942.ic15",0x7800000, 0x0800000, CRC(710f709f) SHA1(2483f0b1106bc82710457a148772e50e83a439d8) )
	ROM_LOAD("mpr-22943.ic16",0x8000000, 0x0800000, CRC(c544f593) SHA1(553af7b6c63d6d6221c4286b8a13840a86e55d5f) )
	ROM_LOAD("mpr-22944.ic17",0x8800000, 0x0800000, CRC(cb096baa) SHA1(cbc267953a749dd24a03d87b65bc19b19bebf205) )
	ROM_LOAD("mpr-22945.ic18",0x9000000, 0x0800000, CRC(f2f914e8) SHA1(ec600abde40bfb5004ec8200ee0eef9410ebca6a) )
	ROM_LOAD("mpr-22946.ic19",0x9800000, 0x0800000, CRC(c79696c5) SHA1(4a9ac8b4ae1ce5d196e6c74fecc241b74aebc4ab) )
	ROM_LOAD("mpr-22947.ic20",0xa000000, 0x0800000, CRC(5e5eb595) SHA1(401d4a11d436988d716bb014b36233f171dc576d) )
	ROM_LOAD("mpr-22948.ic21",0xa800000, 0x0800000, CRC(1b0de917) SHA1(fd1742ea9bb2f1ce871ee3266171f26634e1c8e7) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: VIRTUA STRIKER 2 VER.2000
USA: VIRTUA STRIKER 2 VER.2000
EXP: VIRTUA STRIKER 2 VER.2000

NO.     Type    Byte    Word
IC22    32M     2B49    A054    EPR21929C.22
IC1     64M     F5DD    E983    MPR21914
IC2     64M     4CB7    198B    MPR21915
IC3     64M     5661    47C0    MPR21916
IC4     64M     CD15    DC9A    MPR21917
IC5     64M     7855    BCC7    MPR21918
IC6     64M     59D2    CB75    MPR21919
IC7     64M     B795    BE9C    MPR21920
IC8     64M     D2DE    5AF2    MPR21921
IC9     64M     7AAD    0DD5    MPR21922
IC10    64M     B31B    2C4E    MPR21923
IC11    64M     5C32    D746    MPR21924
IC12    64M     1886    D5EA    MPR21925
IC13    64M     D7B3    24D7    MPR21926
IC14    64M     9EF2    E513    MPR21927
IC15    32M     0DF9    FC01    MPR21928

*/

ROM_START( vs2_2k )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-21929.ic22", 0x0000000, 0x0400000, CRC(831af08a) SHA1(af4c74623be823fd061765cede354c6a9722fd10) )
	ROM_LOAD("mpr-21924.ic1",  0x0800000, 0x0800000, CRC(f91ef69b) SHA1(4ed23091efad7ddf1878a0bfcdcbba3cf151af84) )
	ROM_LOAD("mpr-21925.ic2",  0x1000000, 0x0800000, CRC(40128a67) SHA1(9d191c4ec33465f29bbc09491dde62f354a9ab15) )
	ROM_LOAD("mpr-21911.ic3",  0x1800000, 0x0800000, CRC(19708b3c) SHA1(7d1ef995ce870ffcb68f420a571efb084f5bfcf2) )
	ROM_LOAD("mpr-21926.ic4",  0x2000000, 0x0800000, CRC(b082379b) SHA1(42f585279da1de7e613e42b76e1b81986c48e6ea) )
	ROM_LOAD("mpr-21913.ic5",  0x2800000, 0x0800000, CRC(a3bc1a47) SHA1(0e5043ab6e118feb59f68c84c095cf5b1dba7d09) )
	ROM_LOAD("mpr-21914.ic6",  0x3000000, 0x0800000, CRC(b1dfada7) SHA1(b4c906bc96b615975f6319a1fdbd5b990e7e4124) )
	ROM_LOAD("mpr-21915.ic7",  0x3800000, 0x0800000, CRC(1c189e28) SHA1(93400de2cb803357fa17ae7e1a5297177f9bcfa1) )
	ROM_LOAD("mpr-21916.ic8",  0x4000000, 0x0800000, CRC(55bcb652) SHA1(4de2e7e584dd4999dc8e405837a18a904dfee0bf) )
	ROM_LOAD("mpr-21917.ic9",  0x4800000, 0x0800000, CRC(2ecda3ff) SHA1(54afc77a01470662d580f5676b4e8dc4d04f63f8) )
	ROM_LOAD("mpr-21918.ic10", 0x5000000, 0x0800000, CRC(a5cd42ad) SHA1(59f62e995d45311b1592434d1ffa42c261fa8ba1) )
	ROM_LOAD("mpr-21919.ic11", 0x5800000, 0x0800000, CRC(cc1a4ed9) SHA1(0e3aaeaa55f1d145fb4877b6d187a3ee78cf214e) )
	ROM_LOAD("mpr-21920.ic12s",0x6000000, 0x0800000, CRC(9452c5fb) SHA1(5a04f96d83cca6248f513de0c6240fc671bcadf9) )
	ROM_LOAD("mpr-21921.ic13s",0x6800000, 0x0800000, CRC(d6346491) SHA1(830971cbc14cab022a09ad4c6e11ee49c550e308) )
	ROM_LOAD("mpr-21922.ic14s",0x7000000, 0x0800000, CRC(a1901e1e) SHA1(2281f91ac696cc14886bcdf4b0685ce2f5bb8117) )
	ROM_LOAD("mpr-21923.ic15s",0x7800000, 0x0400000, CRC(d127d9a5) SHA1(78c95357344ea15469b84fa8b1332e76521892cd) )
ROM_END

/*

SYSTEMID: NAOMI
JAP:  POWER SMASH --------------
USA:  VIRTUA TENNIS IN USA -----
EXP:  VIRTUA TENNIS IN EXPORT --

NO. Type    Byte    Word
IC22    32M 0000    1111
IC1 64M 7422    83DD
IC2 64M 7F26    A93D
IC3 64M 8E02    D3FC
IC4 64M 2545    F734
IC5 64M E197    B75D
IC6 64M 9453    CF75
IC7 64M 29AC    2FEB
IC8 64M 0434    2E9E
IC9 64M C86E    79E6
IC10    64M C67A    BF14
IC11    64M F590    D280

*/

ROM_START( vtennis )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x6000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22927.ic22", 0x0000000, 0x0400000,  CRC(89781723) SHA1(cf644aa66abcec6964d77485a0292f11ba80dd0d) )
	ROM_RELOAD( 0x400000, 0x400000)
	ROM_LOAD("mpr-22916.ic1", 0x0800000, 0x0800000, CRC(903873e5) SHA1(09af791bc02cca0e2dc72187679830ed9f4fc772) )
	ROM_LOAD("mpr-22917.ic2", 0x1000000, 0x0800000, CRC(5f020fa6) SHA1(bd2519be8c88ff34cf2fd2b17271d2b41b64ce9f) )
	ROM_LOAD("mpr-22918.ic3", 0x1800000, 0x0800000, CRC(3c3bf533) SHA1(db43ca9332e76b968b9b388b4824b768f82b9859) )
	ROM_LOAD("mpr-22919.ic4", 0x2000000, 0x0800000, CRC(3d8dd003) SHA1(91f494b06b9977215ab726a2499b5855d4d49e81) )
	ROM_LOAD("mpr-22920.ic5", 0x2800000, 0x0800000, CRC(efd781d4) SHA1(ced5a8dc8ff7677b3cac2a4fae04670c46cc96af) )
	ROM_LOAD("mpr-22921.ic6", 0x3000000, 0x0800000, CRC(79e75be1) SHA1(82318613c947907e01bbe50569b05ef24789d7c9) )
	ROM_LOAD("mpr-22922.ic7", 0x3800000, 0x0800000, CRC(44bd3883) SHA1(5c595d903d8865bf8bf3aafb1f527bff232718ed) )
	ROM_LOAD("mpr-22923.ic8", 0x4000000, 0x0800000, CRC(9ebdf0f8) SHA1(f1b688bda387fc00c70cb6a0c374c6c13926c138) )
	ROM_LOAD("mpr-22924.ic9", 0x4800000, 0x0800000, CRC(ecde9d57) SHA1(1fbe7fdf66a56f4f1765baf113dff95142bfd114) )
	ROM_LOAD("mpr-22925.ic10",0x5000000, 0x0800000, CRC(81057e42) SHA1(d41137ae28c64dbdb50150db8cf25851bc0709c4) )
	ROM_LOAD("mpr-22926.ic11",0x5800000, 0x0800000, CRC(57eec89d) SHA1(dd8f9a9155e51ee5260f559449fb0ea245077952) )
ROM_END

/*
SYSTEMID: NAOMI
JAP: ROYAL RUMBLE
USA: ROYAL RUMBLE
EXP: ROYAL RUMBLE
*/

ROM_START( wwfroyal )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0x8800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-22261.ic22", 0x0000000, 0x0400000, CRC(60e5a6cd) SHA1(d74ee8318e40190231b94030176223da8305c053) )
        ROM_LOAD( "mpr-22262.ic1", 0x0800000, 0x1000000, CRC(f18c7920) SHA1(76fb592f62b8a359df19074265b44772ab2ecea0) )
        ROM_LOAD( "mpr-22263.ic2", 0x1800000, 0x1000000, CRC(5a397a54) SHA1(4b41b19ab7c49e09eeb5df2e688fdaecf8bb33a9) )
        ROM_LOAD( "mpr-22264.ic3", 0x2800000, 0x1000000, CRC(edca701e) SHA1(459b533d862e011f8daa0e4997d69fa7339b0755) )
        ROM_LOAD( "mpr-22265.ic4", 0x3800000, 0x1000000, CRC(7dfe71a1) SHA1(f2053544de8a177ab931c4d6a9010dfb6cc92e31) )
        ROM_LOAD( "mpr-22266.ic5", 0x4800000, 0x1000000, CRC(3e9ac148) SHA1(05a5725c72bbfc65db47aaa677b95f07aa9a3909) )
        ROM_LOAD( "mpr-22267.ic6", 0x5800000, 0x1000000, CRC(67ec1027) SHA1(2432b33983bbc9b07477459adb5ee1a62b6c0ea3) )
        ROM_LOAD( "mpr-22268.ic7", 0x6800000, 0x1000000, CRC(536f5eea) SHA1(f1de8624f82595adf75693b604fb026bf3f778ee) )
        ROM_LOAD( "mpr-22269.ic8", 0x7800000, 0x1000000, CRC(6c0cf740) SHA1(da10b33a6e54afbe1d7e52801216e7119b0b33b1) )

	// trojaned protection data (filename is address read from)
	ROM_REGION( 0x200000, "naomibd_prot", ROMREGION_ERASE00 )
	ROM_LOAD( "a24db210.bin", 0x000000, 0x00005e, CRC(79fc852b) SHA1(58229b71d1a7067dd6ce3a617edc7d8bdd132036) )
ROM_END

/*

SYSTEMID: NAOMI
JAP: ZOMBIE REVENGE IN JAPAN
USA: ZOMBIE REVENGE IN USA
EXP: ZOMBIE REVENGE IN EXPORT

NO. Type    Byte    Word
IC22    16M 0000    0000
IC1 64M 899B    97E1
IC2 64M 6F0B    2D2D
IC3 64M 4328    C898
IC4 64M 0205    57C5
IC5 64M 93A7    A717
IC6 64M 936B    A35B
IC7 64M 2F51    2BFC
IC8 64M D627    54C5
IC9 64M D2F9    B84C
IC10    64M 9B5A    B70B
IC11    64M 3F0F    9AEB
IC12    64M 5778    EBCA
IC13    64M 75DB    8563
IC14    64M 427A    577C
IC15    64M A7B7    D0D6
IC16    64M 9F01    FCFE
IC17    64M DFB4    58F7
IC18    64M C453    B313
IC19    64M 04B8    49FB

Protection notes:
0C0E6758: 013C   MOV.B   @(R0,R3),R1 ;checks $c7a45b8+94, natively it's 0xbb, it should be 0 or 1
0C0E675A: 611C   EXTU.B  R1,R1
0C0E675C: 31C7   CMP/GT  R12,R1
0C0E675E: 1F11   MOV.L   R1,@($04,R15)
0C0E6760: 8F04   BFS     $0C0E676C ;if R12 > R1 go ahead, otherwise kill yourself
0C0E6762: E500   MOV     #$00,R5
0C0E6764: D023   MOV.L   @($008C,PC),R0 [0C0E67F4]
0C0E6766: 2052   MOV.L   R5,@R0
0C0E6768: AFFE   BRA     $0C0E6768
*/

ROM_START( zombrvn )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xa000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-21707.ic22", 0x0000000, 0x0200000,  CRC(4daa11e9) SHA1(2dc219a5e0d0b41cce6d07631baff0495c479e13) )
	ROM_RELOAD( 0x200000, 0x200000)
	ROM_RELOAD( 0x400000, 0x200000)
	ROM_RELOAD( 0x600000, 0x200000)
	ROM_LOAD("mpr-21708.ic1", 0x0800000, 0x0800000, CRC(b1ca1ca0) SHA1(7f6823c8f8b58d3102e73c153a3f4ce5ad70694d) )
	ROM_LOAD("mpr-21709.ic2", 0x1000000, 0x0800000, CRC(1ccc22bb) SHA1(0d0b4b13a997e33d89c0b67e579ff5cb63f49355) )
	ROM_LOAD("mpr-21710.ic3", 0x1800000, 0x0800000, CRC(954f49ba) SHA1(67d532048eeb0e7ddd77784138708b256a9386cd) )
	ROM_LOAD("mpr-21711.ic4", 0x2000000, 0x0800000, CRC(bda785e2) SHA1(85fe90fce718f278fc90d3b64411be2b420fef17) )
	ROM_LOAD("mpr-21712.ic5", 0x2800000, 0x0800000, CRC(38309255) SHA1(f693e76b520f25bc510ab1025303cd7e544d9386) )
	ROM_LOAD("mpr-21713.ic6", 0x3000000, 0x0800000, CRC(7c66c88e) SHA1(3bac6db0a5ea65b100911a9674312d4b94f6f57a) )
	ROM_LOAD("mpr-21714.ic7", 0x3800000, 0x0800000, CRC(dd8db07e) SHA1(087299d342e86f629e4878d592540faaba78d5c1) )
	ROM_LOAD("mpr-21715.ic8", 0x4000000, 0x0800000, CRC(7243da2e) SHA1(a797ff85527224d128268cf62e028ee8b308b406) )
	ROM_LOAD("mpr-21716.ic9", 0x4800000, 0x0800000, CRC(01dd88c2) SHA1(77b8bf78d760ad769964209e881e5eddc74d45d4) )
	ROM_LOAD("mpr-21717.ic10",0x5000000, 0x0800000, CRC(95139ec0) SHA1(90db6f18e18e842f731ef34892ac520fd9f4a8d6) )
	ROM_LOAD("mpr-21718.ic11",0x5800000, 0x0800000, CRC(4d36a24a) SHA1(0bc2d80e6149b2d97582a58fdf43d0bdbcfcedfc) )
	ROM_LOAD("mpr-21719.ic12",0x6000000, 0x0800000, CRC(2b86df0a) SHA1(1d6bf4d2568df3ce3a2e60dc51167b5344b00ebd) )
	ROM_LOAD("mpr-21720.ic13",0x6800000, 0x0800000, CRC(ff681ece) SHA1(896e2c484e640d8c426f0159a1be419e476ad14f) )
	ROM_LOAD("mpr-21721.ic14",0x7000000, 0x0800000, CRC(216abba6) SHA1(0819d727a235fe6a3ccfe6474fce9b13718e235c) )
	ROM_LOAD("mpr-21722.ic15",0x7800000, 0x0800000, CRC(b2de7e5f) SHA1(626bf13c40df936a34176821d38418214a5407cb) )
	ROM_LOAD("mpr-21723.ic16",0x8000000, 0x0800000, CRC(515932ae) SHA1(978495c9f9f24d0cdae5a44c3376f7a43f0ce9f5) )
	ROM_LOAD("mpr-21724.ic17",0x8800000, 0x0800000, CRC(f048aeb7) SHA1(39b7bf0ce65f6e13aa0ae5fd6a142959b9ce5acf) )
	ROM_LOAD("mpr-21725.ic18",0x9000000, 0x0800000, CRC(2202077b) SHA1(0893a85379f994277083c0bc5b178dd34508f816) )
	ROM_LOAD("mpr-21726.ic19",0x9800000, 0x0800000, CRC(429bf290) SHA1(6733e1bcf100e73ab43273f6feedc187fcaa55d4) )
ROM_END

ROM_START( gunsur2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	NAOMI_BIOS

	ROM_REGION( 0xb800000, "user1", ROMREGION_ERASEFF)
        ROM_LOAD( "bhf1vere.2f",  0x0000000, 0x0800000, CRC(121ea283) SHA1(e4bf2b90fa3d42727b8393ffa2c5a8863914a630) )
        ROM_LOAD( "bhf1ma2.4m",   0x2000000, 0x1000000, CRC(8073dec7) SHA1(2d4173ff0de37b95a8cb02e1b572a9cdeb448c24) )
ROM_END

/* GD-ROM titles - a PIC supplies a decryption key

PIC stuff

command             response                   comment

kayjyo!?          ->:\x70\x1f\x71\x1f\0\0\0    (unlock gdrom)
C1strdf0          ->5BDA.BIN                   (lower part of boot filename string, BDA.BIN in this example)
D1strdf1          ->6\0\0\0\0\0\0\0            (upper part of filename string)
bsec_ver          ->8VER0001                   (always the same? )
atestpic          ->7TEST_OK                   (always the same? )
AKEYCODE          ->3.......                   (high 7 bytes of des key)
Bkeycode          ->4.\0\0\0\0\0\0             (low byte of des key, then \0 fill)
!.......          ->0DIMMID0                   (redefine upper 7 bytes of session key)
".......          ->1DIMMID1                   (redefine next 7 bytes)
#..               ->2DIMMID2                   (last 2 bytes)


default session key is
"NAOMIGDROMSYSTEM"

info from Elsemi:

it sends bsec_ver, and if it's ok, then the next commands are the session key changes
if you want to have the encryption described somewhere so it's not lost. it's simple:
unsigned char Enc(unsigned char val,unsigned char n)
{
    val^=Key[8+n];
    val+=Key[n];

    return val;
}

do for each value in the message to send
that will encrypt the char in the nth position in the packet to send
time to go to sleep


*/

#ifdef UNUSED_FUNCTION
// rather crude function to write out a key file
void naomi_write_keyfile(void)
q{
	// default key structure
	UINT8 response[10][8] = {
	{ ':', 0x70, 0x1f, 0x71, 0x1f, 0x00, 0x00, 0x00 }, // response to kayjyo!?
	{ '8', 'V',  'E',  'R',  '0',  '0',  '0',  '1'  }, // response to bsec_ver
	{ '7', 'T',  'E',  'S',  'T',  '_',  'O',  'K'  }, // response to atestpic
	{ '6', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // response to D1strdf1 (upper part of filename)
	{ '5', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // response to C1strdf0 (lower part of filename)
	{ '4', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // response to Bkeycode (lower byte of DES key)
	{ '3', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // response to AKEYCODE (high 7 bytes of DES key)
	{ '2', 'D',  'I',  'M',  'M',  'I',  'D',  '2'  }, // response to #..      (rewrite low 2 bytes of session key)
	{ '1', 'D',  'I',  'M',  'M',  'I',  'D',  '1'  }, // response to "....... (rewrite middle 7 bytes of session key)
	{ '0', 'D',  'I',  'M',  'M',  'I',  'D',  '0'  }, // response to !....... (rewrite upper 7 bytes of session key)
	};

	int i;
	char bootname[256];
	char picname[256];

	// ######### edit this ###########
	UINT64 key = 0x4FF16D1A9E0BFBCDULL;

	memset(bootname,0x00,14);
	memset(picname,0x00,256);

	// ######### edit this ###########
	strcpy(picname,"317-5072-com.data");
	strcpy(bootname,"BCY.BIN");

	for (i=0;i<14;i++)
	{
		if (i<7)
		{
			response[4][i+1] = bootname[i];
		}
		else
		{
			response[3][i-6] = bootname[i];
		}
	}

	for (i=0;i<8;i++)
	{
		UINT8 keybyte = (key>>(7-i)*8)&0xff;

		if (i<7)
		{
			response[6][i+1] = keybyte;
		}
		else
		{
			response[5][1] = keybyte;
		}
	}


	{
		FILE *fp;
		fp=fopen(picname, "w+b");
		if (fp)
		{
			fwrite(response, 10*8, 1, fp);
			fclose(fp);
		}
	}


}
#endif

/* All games have the regional titles at the start of the IC22 rom in the following order

  JAPAN
  USA
  EXPORT (EURO in some titles)
  KOREA (ASIA in some titles)
  AUSTRALIA
  UNUSED
  UNUSED
  UNUSED

  with the lists below it has been assumed that if the title is listed for a region
  then it is available / works in that region, this has not been confirmed as correct.

*/

/* Naomi & Naomi GD-ROM */
GAME( 1998, naomi,    0,        naomi,    naomi,    naomi, ROT0, "Sega",            "Naomi Bios", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )

/* Complete Dumps */
/* 840-xxxxx (Sega games)*/
/* 0001C */ GAME( 1998, dybbnao,  naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Dynamite Baseball NAOMI (JPN)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0003C */ GAME( 1999, zombrvn,  naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Zombie Revenge (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0007C */ GAME( 1999, ggram2,   naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Giant Gram (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0010C */ GAME( 1999, vs2_2k,   naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Virtua Striker 2 Ver. 2000 (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0011C */ GAME( 1999, toyfight, naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Toy Fighter", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0015C */ GAME( 1999, vtennis,  naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Power Smash (JPN) / Virtua Tennis (USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0016C */ GAME( 1999, derbyoc,  naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Derby Owners Club (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0017C */ GAME( 1999, otrigger, naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "OutTrigger (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0019C */ GAME( 1999, dybb99,   naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Dynamite Baseball '99 (JPN) / World Series '99 (USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 00??C */ GAME( 1999, smlg99,   naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Super Major League '99", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0020C */ GAME( 1999, samba,    naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Samba De Amigo (JPN)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0083C */ GAME( 1999, derbyoc2, naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Derby Owners Club II (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0021C */ GAME( 2000, virnba,   naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Virtua NBA (JPN, USA, EXP, KOR, AUS)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0021C */ GAME( 2000, virnbao,  virnba,   naomi,    naomi,    naomi, ROT0, "Sega",            "Virtua NBA (JPN, USA, EXP, KOR, AUS) (original)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0026C */ GAME( 2000, totd,     naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "The Typing of the Dead (JPN, USA, EXP, KOR, AUS)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0027C */ GAME( 2000, smarinef, naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Sega Marine Fishing", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0030C */ GAME( 2000, qmegamis, naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Quiz Ah Megamisama (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0039C */ GAME( 2000, gram2000, naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Giant Gram 2000 (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0040C */ GAME( 2000, wwfroyal, naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "WWF Royal Rumble (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0041C */ GAME( 2000, slasho,   naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Slashout (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0044C */ GAME( 2001, csmash,   naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Cosmic Smash (JPN, USA, EXP, KOR, AUS) (rev. A)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0044C */ GAME( 2001, csmasho,  csmash,   naomi,    naomi,    naomi, ROT0, "Sega",            "Cosmic Smash (JPN, USA, EXP, KOR, AUS) (original)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0128C */ GAME( 2001, shootpl,  naomi,    naomi,    naomi,    naomi, ROT0, "Sega",            "Shootout Pool (JPN, USA, KOR, AUS) / Shootout Pool Prize (EXP)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )

/* 841-xxxxx ("Licensed by Sega" games)*/
/* 0001C */ GAME( 1999, pstone,   naomi,    naomi,    naomi,    naomi,    ROT0, "Capcom",          "Power Stone (JPN, USA, EUR, ASI, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0002C */ GAME( 1999, suchie3,  naomi,    naomi,    naomi_mp, naomi_mp, ROT0, "Jaleco",          "Idol Janshi Suchie-Pai 3 (JPN)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0003C */ GAME( 1999, doa2,     naomi,    naomi,    naomi,    naomi,    ROT0, "Tecmo",           "Dead or Alive 2 (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0003C */ GAME( 2000, doa2m,    doa2,     naomi,    naomi,    naomi,    ROT0, "Tecmo",           "Dead or Alive 2 Millennium (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0004C */ GAME( 1999, shangril, naomi,    naomi,    naomi_mp, naomi_mp, ROT0, "Marvelous Ent.",  "Dengen Tenshi Taisen Janshi Shangri-la (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0005C */ GAME( 1999, spawn,    naomi,    naomi,    naomi,    naomi,    ROT0, "Capcom",          "Spawn (JPN, USA, EUR, ASI, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0007C */ GAME( 2000, mvsc2,    naomi,    naomi,    naomi,    naomi,    ROT0, "Capcom",          "Marvel vs. Capcom 2 (JPN, USA, EUR, ASI, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0008C */ GAME( 2000, pstone2,  naomi,    naomi,    naomi,    naomi,    ROT0, "Capcom",          "Power Stone 2 (JPN, USA, EUR, ASI, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0011C */ GAME( 2000, capsnk,   naomi,    naomi,    naomi,    naomi, ROT0, "Capcom / SNK",    "Capcom Vs. SNK Millennium Fight 2000 (000904 JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0011C */ GAME( 2000, capsnka,  capsnk,    naomi,    naomi,    naomi, ROT0, "Capcom / SNK",    "Capcom Vs. SNK Millennium Fight 2000 (000804 JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0012C */ GAME( 2000, cspike,   naomi,    naomi,    naomi,    naomi,    ROT0, "Psikyo / Capcom", "Gun Spike (JPN) / Cannon Spike (USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0013C */ GAME( 2000, ggx,      naomi,    naomi,    naomi,    naomi,    ROT0, "Arc System Works","Guilty Gear X (JPN)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0014C */ GAME( 2000, gwing2,   naomi,    naomi,    naomi,    naomi, ROT0, "Takumi / Capcom", "Giga Wing 2 (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0015C */ GAME( 2000, pjustic,  naomi,    naomi,    naomi,    naomi, ROT0, "Capcom",          "Moero Justice Gakuen (JPN) / Project Justice (USA, EXP, KOR, AUS) ", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* 0016  */ GAME( 2000, deathcox, naomi,    naomi,    naomi,    naomi,    ROT0, "Ecole",           "Death Crimson OX (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
/* xxxx  */ GAME( 2001, hmgeo,    naomi,    naomi,    naomi,    naomi,    ROT0, "Capcom",          "Heavy Metal Geomatrix (JPN, USA, EUR, ASI, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )

/* Games with incomplete dumps */
/* xxxx  */ GAME( 2001, gunsur2,  naomi,    naomi,    naomi,    naomi,    ROT0, "Capcom / Namco",  "Gun Survivor 2: Bio Hazard Code Veronica", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )

/* Games with game specific bios sets */
GAME( 1998, hod2bios, 0,        naomi,    naomi,    0,     ROT0, "Sega",            "Naomi House of the Dead 2 Bios", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 1999, hotd2,    hod2bios, naomi,    naomi,    0,     ROT0, "Sega",            "House of the Dead 2", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 1999, f355bios, 0,        naomi,    naomi,    0,     ROT0, "Sega",            "Naomi Ferrari F355 Challenge Bios", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 1999, f355,     f355bios, naomi,    naomi,    0,     ROT0, "Sega",            "Ferrari F355 Challenge", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )

/**********************************************
 *
 * Naomi GD-ROM defines
 *
 *********************************************/

ROM_START( gundmgd )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0001", 0, SHA1(615e19c22f32096f3aad557019a14313b60a4070) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5069-COM)
	//(sticker 253-5509-5069)
	ROM_LOAD("317-5069-com.data", 0x00, 0x50, CRC(8e2f0cbd) SHA1(a5f3a990a03bfa50a1a742593c5ec07645c8718d) )
ROM_END


ROM_START( sfz3ugd )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0002", 0, SHA1(af4669fdd7ce8e6ec4a170748d401e322a3d7ae8) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5072-COM)
	//(sticker 253-5509-5072)
	ROM_LOAD("317-5072-com.data", 0x00, 0x50, CRC(6d2992b9) SHA1(88e6dc6711f9f883362ba1217a3350d452a70896) )
ROM_END

ROM_START( cvsgd )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0004", 0,  SHA1(7a7fba0fbbc769c5120b08e6d692f1ac63a42225) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5076-JPN)
	//(sticker 253-5509-5076J)
	ROM_LOAD("317-5076-jpn.data", 0x00, 0x50, CRC(5004161b) SHA1(8b2cdfec12ffd9160bc74659e08d07cbc46a4011) )
ROM_END


ROM_START( gundmxgd )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0006", 0, SHA1(b28d6598711b5a9c744bbf07ad03fc60962d2e28) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5079-COM)
	//(sticker 253-5509-5079)
	ROM_LOAD("317-5079-com.data", 0x00, 0x50, CRC(e6abe978) SHA1(700e610d84e517793a22d6cabd1aef9c3b8bc092) )
ROM_END


ROM_START( cvs2gd )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0007a", 0, SHA1(56510390667b39b3915d8bc078660cbe093cf566) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5078-COM)
	//(sticker 253-5509-5078)
	ROM_LOAD("317-5078-com.data", 0x00, 0x50, CRC(1c8d94ee) SHA1(bec4a6901f62dc8f76f7b9d72284b3eaac340bf3) )
ROM_END

ROM_START( ikaruga )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0010", 0, SHA1(65dcc22dd9e9b70975096464ad8e31a4a73dc5fd) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5081-JPN)
	//(sticker 253-5509-5081J)
	ROM_LOAD("317-5081-jpn.data", 0x00, 0x50, CRC(d4cc5c8c) SHA1(44c0c5c2744fbd419b684cbc36f01973487bafbc) )
ROM_END


ROM_START( ggxx )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0011", 0, SHA1(b7328eb2c588d55284bdcea0fe89bb8e629a8669) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5082-COM)
	//(sticker 253-5509-5082)
	ROM_LOAD("317-5082-com.data", 0x00, 0x50, CRC(fa31209d) SHA1(bb18e6412a02510832f7200a06a3179ef1695ef2) )
ROM_END


ROM_START( moeru )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0013", 0, SHA1(c8869069c28bc8eec96d820886bc388d69d46143) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5084-JPN)
	//(sticker 253-5509-5084J)
	ROM_LOAD("317-5084-jpn.data", 0x00, 0x50, CRC(56de2066) SHA1(a16a6d9f7272d3f8d322c85222a0487a87811910) )
ROM_END


ROM_START( chocomk )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0014a", 0, SHA1(f88d8203c8692f51c9492d5549a3ad7d9583dc6f) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5085-JPN)
	//(sticker 253-5509-5085J)
	ROM_LOAD("317-5085-jpn.data", 0x00, 0x50, CRC(eecd8140) SHA1(471fb6b242eff646173265df891109e3e0a37a7d) )
ROM_END


ROM_START( quizqgd )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0017", 0, SHA1(94a9319633388968611892e36691b45c94b4f83f) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5090-JPN)
	//(sticker 253-5509-5090J)
	ROM_LOAD("317-5090-jpn.data", 0x00, 0x50, CRC(b4dd88f6) SHA1(c9aacd79c1088225fa5a69b7bd31a7c1286160e1) )
ROM_END


ROM_START( ggxxrl )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0019a", 0, SHA1(d44906505ff698eda6feee6c2b9402e19f64e5d3) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5092-JPN)
	//(sticker 253-5509-5092J)
	ROM_LOAD("317-5092-jpn.data", 0x00, 0x50, CRC(7c8cca4b) SHA1(92c5a0fd8916744eefc023e64daea69803573928) )
ROM_END


ROM_START( tetkiwam )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0020", 0, SHA1(7b2ef47ca2038d6a93615b760b03e8f7cb1b83c2) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5093-JPN)
	//(sticker 253-5509-5093J)
	ROM_LOAD("317-5093-jpn.data", 0x00, 0x50, CRC(06bc5013) SHA1(f7a46b7e34b20409ce2fdae80e5cdfff7adb9c64) )
ROM_END


ROM_START( shikgam2 )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0021", 0, SHA1(f5036711a28a211e8d71400a8322db3172c5733f) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5095-JPN)
	//(sticker 253-5509-5095J)
	ROM_LOAD("317-5095-jpn.data", 0x00, 0x50, CRC(6033ec89) SHA1(9e99a8ad43fa29296dbf2e13b3a3d4552130b4e8) )
ROM_END


ROM_START( usagui )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0022", 0, SHA1(45deba05a12abbf6390c0fc0e4cdeaedfa7d2ca5) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5096-JPN)
	//(sticker 253-5509-5096J)
	ROM_LOAD("317-5096-jpn.data", 0x00, 0x50, CRC(621e827a) SHA1(cdc7580f5d1dfe85d2806233f22bc4f13fd62946) )
ROM_END


ROM_START( bdrdown )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0023a", 0, SHA1(caac915104d61f2122f5afe27da1ef5fa9cf9f9a) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5097-JPN)
	//(sticker 253-5509-5097J)
	ROM_LOAD("317-5097-jpn.data", 0x00, 0x50, CRC(e689d047) SHA1(7e3e298d9a8076af0254faeb0eb89fbfce94718d) )
ROM_END


ROM_START( psyvar2 )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0024", 0,  SHA1(d346762036fb1c40a261a434b50e63459f306f14) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5100-JPN)
	//(sticker 253-5509-5100J)
	ROM_LOAD("317-5100-jpn.data", 0x00, 0x50, CRC(94316f0f) SHA1(e1ec2b4225105dbaa1e59e8a05027e73f7b725a9) )
ROM_END


ROM_START( cfield )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0025", 0, SHA1(be0d88eb4f48403a2ceaa7ef588ed60b96ba93bf) )


	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5102-COM)
	//(sticker 253-5509-5102)
	ROM_LOAD("317-5102-com.data", 0x00, 0x50, CRC(32adf2eb) SHA1(d86752e6fe9ccac093c512828fca5b7ae62a3ff2) )
ROM_END


ROM_START( trizeal )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0026", 0, SHA1(e4c1e51292a7923b25bfc61d38fe386bf596002a) )


	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5103-JPN)
	//(sticker 253-5509-5103J)
	ROM_LOAD("317-5103-jpn.data", 0x00, 0x50,  CRC(3affbf82) SHA1(268746e86e7546f4bab54bdd268f7b58f10c1aaf) )
ROM_END


ROM_START( meltybld )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0028c", 0, SHA1(66de09738551e351784cc9695a58b35fdf6b6c4b) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5104-JPN)
	//(sticker 253-5509-5104J)
	ROM_LOAD("317-5104-jpn.data", 0x00, 0x50, CRC(fedc8305) SHA1(c535545937213f726f25e6aa8eb3746a794e9100) )
ROM_END


ROM_START( senko )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0030a", 0,  SHA1(1f7ade47e37a0026451b5baf3ba746400de8d156) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5107-JPN)
	//(sticker 253-5509-5107J)
	ROM_LOAD("317-5107-jpn.data", 0x00, 0x50, CRC(7b607409) SHA1(a9946a0637453e4813bef18060d4420355cff800) )
ROM_END

ROM_START( senkoo )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0030", 0,  SHA1(c7f25c05f47a490c5da9369c588b6136e93c280e) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5107-JPN)
	//(sticker 253-5509-5107J)
	ROM_LOAD("317-5107-jpn.data", 0x00, 0x50, CRC(7b607409) SHA1(a9946a0637453e4813bef18060d4420355cff800) )
ROM_END


ROM_START( ss2005 )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0031a", 0, SHA1(6091525845fc2042ed43cae5a1b60c603e16cf97) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5108-JPN)
	//(sticker 253-5509-5108J)
	ROM_LOAD("317-5108-jpn.data", 0x00, 0x50, CRC(6a2eb334) SHA1(cab407d2e994f33aa921d50f399b17e6fbf98eb0) )
ROM_END


ROM_START( radirgy )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0032", 0, SHA1(ebd7a40e59082e660ebf9a2d4ae7cb64371dae8d) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5110-JPN)
	//(sticker 253-5509-5110J)
	ROM_LOAD("317-5110-jpn.data", 0x00, 0x50, CRC(04e4ac45) SHA1(4102a4d68f20a7e78f6c7e3494e7229018e30e39) )
ROM_END


ROM_START( ggxxsla )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0033a", 0, SHA1(29de69ae97a9099b1bbe936dfa965bb4a3195f68) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5111-JPN)
	//(sticker 253-5509-5111J)
	ROM_LOAD("317-5111-jpn.data", 0x00, 0x50, CRC(a517c70d) SHA1(5f9798941355fb9abce511508c860653d6369e72) )
ROM_END


ROM_START( kurucham )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0034", 0, SHA1(10fd7edb0b620133c003d686e5af2ed27004fa09) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5115-JPN)
	//(sticker 253-5509-5115J)
	ROM_LOAD("317-5115-jpn.data", 0x00, 0x50,  CRC(f40072a8) SHA1(366df2079a4d2ff7a93082c9bf849aad40ab079d) )
ROM_END


ROM_START( undefeat )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0035", 0, SHA1(91da482a6a082e48bee5b3bd20d9c92d23936965) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5117-JPN)
	//(sticker 253-5509-5117J)
	ROM_LOAD("317-5117-jpn.data", 0x00, 0x50,  CRC(f90f6d3b) SHA1(a18f803a8e951c375a3a55e4b0e74b698ae93f92) )
ROM_END


ROM_START( meltyb )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0039", 0, SHA1(ffc7f6e113ad69422a4f22f318bdf9b1dc5c25db) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5124-JPN)
	//(sticker 253-5509-5124J)
	ROM_LOAD("317-5124-jpn.data", 0x00, 0x50, CRC(4d6e2c77) SHA1(3bed734c291140d0a61afa40f221395369a251a9) )
ROM_END

ROM_START( meltyba )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0039a", 0, SHA1(e6aa3d65b43a20606e6754bcb8665438770a1f8c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5124-JPN)
	//(sticker 253-5509-5124J)
	ROM_LOAD("317-5124-jpn.data", 0x00, 0x50, CRC(4d6e2c77) SHA1(3bed734c291140d0a61afa40f221395369a251a9) )
ROM_END


ROM_START( trgheart )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0036a", 0, SHA1(91f1e19136997cb1e2edfb1ad342b9427d1d3bfb) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5121-JPN)
	//(sticker 253-5509-5121J)
	ROM_LOAD("317-5121-jpn.data", 0x00, 0x50, CRC(a417b20f) SHA1(af6ed7ebf95948bff3e8df915b229189b8de1e46) )
ROM_END


ROM_START( jingystm )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0037", 0, SHA1(99ffe2987e3002b3871daf276d2be45f2e9c6e74) )


	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-5122-JPN)
	//(sticker 253-5509-5122J)
	ROM_LOAD("317-5122-jpn.data", 0x00, 0x50, CRC(0b85b7e4) SHA1(f4e419682ddc4b98a330e5ae543f9276c9bde030) )
ROM_END


ROM_START( karous )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0040", 0, SHA1(a62c8d4b6c5be44a4aeeea1a1a94f3d0fe542593) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5125-COM)
	//(sticker 253-5509-5125)
	ROM_LOAD("317-5125-com.data", 0x00, 0x50, CRC(9d37b5e3) SHA1(e1d3cdc2ed82c864c9ff54d9399a80b70ba150c5) )
ROM_END


ROM_START( takoron )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdl-0042", 0, SHA1(984a4fa012d83dd8c748304958c847c9867f4125) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-5127-JPN)
	//(sticker 253-5509-5127J)
	ROM_LOAD("317-5127-jpn.data", 0x00, 0x50, CRC(e1a6dbe4) SHA1(61b458937acca55e4010f86b684aaa86b8c10eac) )
ROM_END

/* -------------------------------- 1st party -------------- */


/*
Title   CONFIDENTIAL MISSION
Media ID    FFCA
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDS-0001
Version V1.050
Release Date    20001011
Manufacturer ID SEGA ENTERPRISES
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 3788    8558928
track02.raw 3939    6071    5016816
track03.bin 45150   549299  1185760800


PIC
317-0298-COM
*/

ROM_START( confmiss )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0001", 0, SHA1(bd05f197ba8643577883dd25d9d5a74c91b27ca9) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0298-com.data", 0x00, 0x50, CRC(c989b336) SHA1(40075500888626cc2261133eec496b3e753631e5) )
ROM_END


ROM_START( sprtjam )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0003", 0, SHA1(caaba214c1faca78b3370bcd4190eb2853d7f825) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0300-COM)
	//(sticker 253-5508-0300)
	ROM_LOAD("317-0300-com.data", 0x00, 0x50, CRC(9a08413f) SHA1(d57649dcc3af578d55a93dd7a3f41da62d580f54) )
ROM_END


ROM_START( slashout )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0004", 0, SHA1(3cce788393ed194ba9b603f9896ff893691d6b00) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0302-COM)
	//(sticker 253-5508-0302)
	ROM_LOAD("317-0302-com.data", 0x00, 0x50, CRC(4bf6cd62) SHA1(c1fdf12a4d80fa3008170c89d2dc583f19e0450b) )
ROM_END


ROM_START( spkrbtl )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0005", 0, SHA1(d1c3fb2350e4a89372373e7f629c42b741af29b3) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0303-COM)
	//(sticker 253-5508-0303)
	ROM_LOAD("317-0303-com.data", 0x00, 0x50, CRC(6e7888a3) SHA1(5ca78052bcfd9e9f81934cbddd9c173e88973e0e) )
ROM_END

/*
Title   MONKEY_BALL
Media ID    43EB
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDS-0008
Version V1.008
Release Date    20010425
Manufacturer ID
Track   Start Sector    End Sector  Track Size
track01.bin 150 449 705600
track02.raw 600 2732    5016816
track03.bin 45150   549299  1185760800


PIC

253-5508-0307
317-0307-COM
*/

ROM_START( monkeyba )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0008", 0, SHA1(2fadcd141bdbde77b2b335b270959a516af44d99) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0307-com.data", 0x00, 0x50, CRC(448bedc7) SHA1(092dbe5e28890d3ee40d62ca8cbf225c3ce90304) )
ROM_END

/*
This is the I/O board used for Dynamic Golf which is
located under the panel.
It must be connected to the normal I/O board with a USB cable.

PCB Layout
----------

837-13938
|--------------------|
|CN2      CN1        |
|                    |
|      |-----|       |
|      | IC2 |       |
| CN3  |     |       |
|      |-----|    IC3|
|LED    CN4     IC4  |
|--------------------|
Notes:
      CN1 - 24 pin connector. not used
      CN2 - 4 pin connector used for 5 volt power input
      CN3 - USB connector type B
      CN4 - 16 pin connector used for buttons and trackball
      IC1 - HC240 logic IC (SOIC20)
      IC2 - Sega 315-6146 custom IC (QFP176)
      IC3 - 27C512 EPROM with label 'EPR-22084' (DIP28)
      IC4 - HC4020 logic IC (SOIC16)
*/

ROM_START( dygolf )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0009", 0, SHA1(d502155ddaf881c2c9505528004b9904aa32a59c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0308-COM)
	//(sticker 253-5508-0308)
	ROM_LOAD("317-0308-com.data", 0x00, 0x50,  CRC(56f63af0) SHA1(3c453226fc53d2f700b3634db3ef8ce206d94392) )

	ROM_REGION( 0x10000, "io_board", 0)
	ROM_LOAD("epr-22084.ic3", 0x0000, 0x10000, CRC(18cf58bb) SHA1(1494f8215231929e41bbe2a133658d01882fbb0f) )
ROM_END


ROM_START( wsbbgd )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0010", 0,  SHA1(c3135ede3a8bdadab91aed49abacbfbde8037069) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0309-COM)
	//(sticker 253-5508-0309)
	ROM_LOAD("317-0309-com.data", 0x00, 0x50, CRC(8792c550) SHA1(e8d6d91583d1673d8d3fa9ccb0ab1097c5c5ad08) )
ROM_END


ROM_START( vtennisg )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0011", 0,  SHA1(b778403d73c8cdd13383691c9be2094ddfc1ba84) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-0312-COM)
	//(sticker 253-5508-0312)
	ROM_LOAD("317-0312-com.data", 0x00, 0x50, CRC(6b24f78f) SHA1(43f89815ec46cf014d941b4b9238da044b338b4c) )
ROM_END

ROM_START( keyboard )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0017", 0, SHA1(fb86eff3ef38de7fd78cfde897d5332d2092c172) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0323-com.data", 0x00, 0x50, CRC(c1277eb3) SHA1(529ed5a133550e2854f8656cd377706060a7befa) )
ROM_END


ROM_START( vathlete )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0019", 0, SHA1(955d3c0cb991be3057138c562cff69c5ef887787) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0330-COM)
	//(sticker 253-5508-0330)
	ROM_LOAD("317-0330-com.data", 0x00, 0x50, CRC(f5e7f7d4) SHA1(3903337e82011d132993e4366475586866bd39b1) )
ROM_END

/*

Title   VIRTUA TENNIS 2 (POWER SMASH 2)
Media ID    D72C
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDS-0015A
Version V2.000
Release Date    20010827
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 449 705600
track02.raw 600 2732    5016816
track03.bin 45150   549299  1185760800


PIC

253-5508-0318
317-0318-EXP
*/

ROM_START( vtennis2 )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0015a", 0, SHA1(c6e9c9901bd4f075454b7f18baf08df81bc2f1ad) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0318-exp.data", 0x00, 0x50, CRC(7758ade6) SHA1(c62f35810bce466bfb0f55fd555066efd53e9bb6) )
ROM_END


ROM_START( lupinsho )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0018", 0, SHA1(0633a99a666f363ab30450a76b9753685d6b1f57) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0332-j.data", 0x00, 0x50, CRC(31f2b632) SHA1(bbf253bfe831308a7e7fde3a4a28e5bcd2fbb273) )
ROM_END


ROM_START( luptype )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0021a", 0,  SHA1(15c6f9434494a31693cbb8e33da36e0e8a8f7c62) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0332-JPN)
	//(sticker 253-5508-0332J)
	ROM_LOAD("317-0332-jpn.data", 0x00, 0x50, CRC(ab302661) SHA1(65164cf76d78b281772bfcbf5a733b0200e86e09) )
ROM_END

/*
Title   THE_MAZE_OF_THE_KINGS
Media ID    E3D0
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDS-0022
Version V1.001
Release Date    20020306
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 449 705600
track02.raw 600 2732    5016816
track03.bin 45150   549299  1185760800


PIC
317-0333-COM
253-5508-0333

*/
ROM_START( mok )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0022", 0, SHA1(70b41745225006e7876176cbd239edecd4c3f8b6) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0333-com.data", 0x00, 0x50, CRC(0c07970f) SHA1(8882dd2f8ed522790ea78eed80cfa9442f88f67b) )
ROM_END


ROM_START( ngdup23a )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0023a", 0, SHA1(cd9d808b59eb8f40673ec4353d476f2b9c7f783c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE) // uses the vf4 pic
	//PIC16C622A (317-0314-COM)
	//(sticker 253-5508-0314)
	ROM_LOAD("317-0314-com.data", 0x00, 0x50, CRC(91a97eb4) SHA1(059342368bc5d25b494ed3c729870695f9584fc7) )
ROM_END

ROM_START( ngdup23c )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0023c", 0, SHA1(1fcb5530748886f4c4f45487d047859182ff7496))

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE) // uses the vf4 evolution pic
	//PIC16C622A (317-0338-JPN)
	//(sticker 253-5508-0338J)
	ROM_LOAD("317-0338-jpn.data", 0x00, 0x50, CRC(eeb2c9e9) SHA1(d30b5914c603219daea9923e1cf8da2be6096742) )
ROM_END


ROM_START( puyofev )
	NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0031", 0, SHA1(da2d421da9472b149619b6931bb2fe624be75fa2) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C621A (317-0375-COM)
	//(sticker 253-5508-0375)
	ROM_LOAD("317-0375-com.data", 0x00, 0x50, CRC(32bf1825) SHA1(42dfbc6777c154d8de6c6f7350da9ea737380220) )
ROM_END


/* Naomi GD-Rom Sets */
GAME( 2001, naomigd,   0,        naomi,    naomi,    naomi,   ROT0,   "Sega",                   "Naomi GD-ROM Bios", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )

/* GDL-xxxx ("licensed by Sega" games) */
GAME( 2001, gundmgd,   naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Capcom",                 "Mobile Suit Gundam: Federation VS Zeon (GDL-0001)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, sfz3ugd,   naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Capcom",                 "Street Fighter Zero 3 Upper (GDL-0002)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0003
GAME( 2001, cvsgd,     naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Capcom / SNK",           "Capcom vs SNK Millenium Fight 2000 Pro (GDL-0004)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0005 Doki Doki Idol Star Seeker
GAME( 2001, gundmxgd,  naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Capcom",                 "Mobile Suit Gundam: Federation VS Zeon DX  (GDL-0006)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0007 Capcom vs SNK 2
GAME( 2001, cvs2gd,    naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Capcom / SNK",           "Capcom vs SNK 2 Millionaire Fighting 2001 (Rev A) (GDL-0007A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0008 Capcom vs SNK 2 Mark Of The Millenium 2001 (Export)
//GDL-0009
GAME( 2001, ikaruga,   naomigd,  naomigd,  naomi,    naomi,   ROT270, "Treasure",               "Ikaruga (GDL-0010)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, ggxx,      naomigd,  naomigd,  naomi,    ggxx,    ROT0,   "Arc System Works",       "Guilty Gear XX (GDL-0011)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0012 Cleopatra Fortune Plus
GAME( 2002, moeru,     naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Altron",                 "Moeru Casinyo (GDL-0013)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0014 Musapey's Choco Marker
GAME( 2002, chocomk,   naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Ecole Software",         "Musapey's Choco Marker (Rev A) (GDL-0014A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0015 Mazan
//GDL-0016 Yonin Uchi Mahjong MJ
GAME( 2002, quizqgd,   naomigd,  naomigd,  naomi,    naomi,   ROT270, "Amedio (Taito license)", "Quiz Keitai Q mode (GDL-0017)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0018 Azumanga Daioh Puzzle Bobble
//GDL-0019 Guilty Gear XX #Reload
GAME( 2003, ggxxrl,    naomigd,  naomigd,  naomi,    ggxxrl,  ROT0,   "Arc System Works",       "Guilty Gear XX #Reload (Rev A) (GDL-0019A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, tetkiwam,  naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Success",                "Tetris Kiwamemichi (GDL-0020)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2003, shikgam2,  naomigd,  naomigd,  naomi,    naomi,   ROT270, "Alpha System",           "Shikigami No Shiro II / The Castle of Shikigami II (GDL-0021)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2003, usagui,    naomigd,  naomigd,  naomi_mp, naomi_mp,ROT0,   "Warashi / Taito / Mahjong Kobo", "Usagi - Yamashiro Mahjong Hen (GDL-0022)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0023 Border Down
GAME( 2004, bdrdown,   naomigd,  naomigd,  naomi,    naomi,   ROT0,   "G-Rev",                  "Border Down (Rev A) (GDL-0023A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2003, psyvar2,   naomigd,  naomigd,  naomi,    naomi,   ROT270, "G-Rev",                  "Psyvariar 2 - The Will To Fabricate (GDL-0024)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, cfield,    naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Able",                   "Chaos Field (GDL-0025)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, trizeal,   naomigd,  naomigd,  naomi,    naomi,   ROT270, "Taito",                  "Trizeal (GDL-0026)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0027 Melty Blood Act Cadenza?
//GDL-0028  Melty Blood Act Cadenza Ver A
//GDL-0028A Melty Blood Act Cadenza Ver A (Rev A)
//GDL-0028B Melty Blood Act Cadenza Ver A (Rev B)
GAME( 2005, meltybld,  naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Ecole Software",         "Melty Blood Act Cadenza Ver A (Rev C) (GDL-0028C)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0029
GAME( 2005, senko,     naomigd,  naomigd,  naomi,    naomi,   ROT0,   "G-Rev",                  "Senko No Ronde NEW ver. (Rev A) (GDL-0030A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2005, senkoo,    senko,    naomigd,  naomi,    naomi,   ROT0,   "G-Rev",                  "Senko No Ronde (GDL-0030)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0031 Super Shanghai 2005
GAME( 2005, ss2005,    naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Starfish",               "Super Shanghai 2005 (Rev A) (GDL-0031A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2005, radirgy,   naomigd,  naomigd,  naomi,    naomi,   ROT270, "Milestone",              "Radirgy (GDL-0032)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0033 Guilty Gear XX Slash
GAME( 2005, ggxxsla,   naomigd,  naomigd,  naomi,    ggxxsla, ROT0,   "Arc System Works",       "Guilty Gear XX Slash (Rev A) (GDL-0033A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2006, kurucham,  naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Able",                   "Kurukuru Chameleon (GDL-0034)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2005, undefeat,  naomigd,  naomigd,  naomi,    naomi,   ROT270, "G-Rev",                  "Under Defeat (GDL-0035)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0036 Trigger Heart Exelica
//GAME( 2005, trgheart,  naomigd,  naomigd,  naomi,    naomi,   ROT270, "Warashi",                "Trigger Heart Exelica (Rev A) (GDL-0036A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GAME( 2005, jingystm,  naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Atrativa Japan",         "Jingi Storm - The Arcade (GDL-0037)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0038 Senko No Ronde Special
//GAME( 2006, meltyb,    naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Ecole Software",         "Melty Blood Act Cadenza Ver B (GDL-0039)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GAME( 2006, meltyba,   meltyb,   naomigd,  naomi,    naomi,   ROT0,   "Ecole Software",         "Melty Blood Act Cadenza Ver B (Rev A) (GDL-0039A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GAME( 2006, karous,    naomigd,  naomigd,  naomi,    naomi,   ROT270, "Milestone",              "Karous (GDL-0040)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDL-0041 Guilty Gear XX Accent Core
//GAME( 2006, takoron,   naomigd,  naomigd,  naomi,    naomi,   ROT0,   "Compile",                "Noukone Puzzle Takoron (GDL-0042)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )


/* GDS-xxxx (Sega first party games) */
GAME( 2001, confmiss,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Confidential Mission (GDS-0001)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0002 Shakatto Tambourine
GAME( 2000, sprtjam,   naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Sports Jam (GDS-0003)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2000, slashout,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Slashout (GDS-0004)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, spkrbtl,   naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Spikers Battle (GDS-0005)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0006 Virtua Striker 3 (NAOMI 2)
//GDS-0007 Shakatto Tambourine Motto Norinori Shinkyoku Tsuika
GAME( 2001, monkeyba,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Monkey Ball (GDS-0008)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, dygolf,    naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Virtua Golf / Dynamic Golf (GDS-0009)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, wsbbgd,    naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "World Series Baseball / Super Major League (GDS-0010)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, vtennisg,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Virtua Tennis (GDS-0011)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0012 Virtua Fighter 4 (NAOMI 2)
//GDS-0013
//GDS-0014 Beach Spikers (NAOMI 2)
//GDS-0015 Virtua Tennis 2 / Power Smash 2
GAME( 2001, vtennis2,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Virtua Tennis 2 (Rev A) (GDS-0015A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0016 Shakatto Tambourine Cho Powerup Chu
GAME( 2001, keyboard,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "La Keyboardxyu (GDS-0017)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, lupinsho,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Lupin The Third - The Shooting (GDS-0018)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, vathlete,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Virtua Athletics / Virtua Athlete (GDS-0019)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0020 Initial D Arcade Stage (Japan) (NAOMI 2)
//GDS-0021 Lupin The Third - The Typing
GAME( 2002, luptype,   naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Lupin The Third - The Typing (Rev A) (GDS-0021A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, mok,       naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "The Maze of the Kings (GDS-0022)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0023 Naomi DIMM Firmware Updater
GAME( 2001, ngdup23a,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Naomi DIMM Firmware Updater (GDS-0023A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0023B Naomi DIMM Firmware Updater
GAME( 2001, ngdup23c,  naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Naomi DIMM Firmware Updater (GDS-0023C)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0024 Virtua Fighter 4 Evolution (NAOMI 2)
//GDS-0025 Initial D Arcade Stage (Export) (NAOMI 2)
//GDS-0026 Initial D Arcade Stage Ver. 2 (Japan) (NAOMI 2)
//GDS-0027 Initial D Arcade Stage Ver. 2 (Export) (NAOMI 2)
//GDS-0028
//GDS-0029
//GDS-0030
GAME( 2003, puyofev,   naomigd,  naomigd,  naomi,    naomi,  ROT0, "Sega",          "Puyo Puyo Fever (GDS-0031)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
//GDS-0032 Initial D Arcade Stage Ver. 3 (Japan)
//GDS-0033 Initial D Arcade Stage Ver. 3 (Export)
//GDS-0034
//GDS-0035
//GDS-0036 Virtua Fighter 4 Final Tuned
//GDS-0037? Puyo Puyo Fever (Export)


/**********************************************
 *
 * Naomi 2 Cart defines
 *
 *********************************************/

ROM_START( vstrik3c )
	NAOMI2_BIOS

	ROM_REGION( 0xb000000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("epr-23663.ic22", 0x0000000, 0x0400000, CRC(7007fec7) SHA1(523168f0b218d0bd5c815d65bf0caba2c8468c9d) )
	/* TODO: proper rom names */
	ROM_LOAD("ic.1", 0x0800000, 0x0800000, CRC(db8bf632) SHA1(cd0c5c385dc33778eb6477f48a33d31f8461b810) )
	ROM_LOAD("ic.2", 0x1000000, 0x0800000, CRC(f5889f8b) SHA1(540aa6a9adf10d5426ac4962afdc300cc9c5c6ba) )
	ROM_LOAD("ic.3", 0x1800000, 0x0800000, CRC(74e7ef35) SHA1(6b63910fa05f98b31689882d3f7f32ea84fb11e5) )
	ROM_LOAD("ic.4", 0x2000000, 0x0800000, CRC(69e5eeb3) SHA1(1c615e6150e8c9f5211f7db55158c9b56b9b52fe) )
	ROM_LOAD("ic.5", 0x2800000, 0x0800000, CRC(7ea41bdb) SHA1(1a58c4731f9cc570f79e81a3b87b590d52cf3887) )
	ROM_LOAD("ic.6", 0x3000000, 0x0800000, CRC(cf730ca3) SHA1(f8e132463d574be906f573ca66fbc8d74959be08) )
	ROM_LOAD("ic.7", 0x3800000, 0x0800000, CRC(b8d838fe) SHA1(6722f54a170302978e06305773903a336c6fc9d8) )
	ROM_LOAD("ic.8", 0x4000000, 0x0800000, CRC(03aacb8b) SHA1(7ab12a3c79718fea8b7ae019acafb86bb7381c7e) )
	ROM_LOAD("ic.9", 0x4800000, 0x0800000, CRC(3252196e) SHA1(2e45de03931554e6500fbb83a0d2b1249f2d3015) )
	ROM_LOAD("ic.10",0x5000000, 0x0800000, CRC(b448948b) SHA1(1df5bbd14b24a952bb5289778ceea15ffe161491) )
	ROM_LOAD("ic.11",0x5800000, 0x0800000, CRC(937a3280) SHA1(46c6277402c029a55aa0e2417ea14634d17e2601) )
	ROM_LOAD("ic.12",0x6000000, 0x0800000, CRC(e3ca01df) SHA1(1c96e596d5ad6e08fcc7625788952d5ca5c80ce5) )
	ROM_LOAD("ic.13",0x6800000, 0x0800000, CRC(9bbbbbdc) SHA1(19d124743b686e4d5d38181cee547452402cdf61) )
	ROM_LOAD("ic.14",0x7000000, 0x0800000, CRC(eed0270a) SHA1(2b75a2d5bda05e68bb7d81570a7f4e180085fbd5) )
	ROM_LOAD("ic.15",0x7800000, 0x0800000, CRC(02bee590) SHA1(77ce37209f525be58d9373b5467b6be9f0e7fb4b) )
	ROM_LOAD("ic.16",0x8000000, 0x0800000, CRC(025f4f2f) SHA1(b104dc936036464dcf17524fd5e639c9852e1bd1) )
	ROM_LOAD("ic.17",0x8800000, 0x0800000, CRC(98fc3c54) SHA1(d2ba54b9bf15d2b3d623c5358bbf104b48d23223) )
	ROM_LOAD("ic.18",0x9000000, 0x0800000, CRC(e9fe50a4) SHA1(053f2a33ab7310c518a66b49d83fc817f2db3ef0) )
	ROM_LOAD("ic.19",0x9800000, 0x0800000, CRC(bbd0139c) SHA1(f87e729534d503f2bc37b7d595d25070ae33353c) )
	ROM_LOAD("ic.20",0xa000000, 0x0800000, CRC(5a69ab9e) SHA1(205a8facded5dbee8653807ef4d4b5e4210f56ef) )
	ROM_LOAD("ic.21",0xa800000, 0x0800000, CRC(d6ef7d68) SHA1(4ee396af6c5caf4c5af6e9ad0e03a7ac2c5039f4) )
ROM_END

ROM_START( wldrider )
	NAOMI2_BIOS

	ROM_REGION( 0xa800000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD( "epr-23622.ic22", 0x0000000, 0x0400000, CRC(8acafa5b) SHA1(c92bcd40bad6ba8efd1edbfd7e439fb2b3c67fb0) )
        ROM_LOAD( "mpr-23611.ic1",  0x0800000, 0x1000000, CRC(943bc32a) SHA1(11ced99e9dbd7cc93031779e00d4ee1f2dff9086) )
        ROM_LOAD( "mpr-23612.ic2",  0x1800000, 0x1000000, CRC(f71d87e5) SHA1(c36d1d07702642db282278f1cf556ed472e930d3) )
        ROM_LOAD( "mpr-23613.ic3",  0x2800000, 0x1000000, CRC(689e783e) SHA1(1264c3389610bbf2745b7e6d50f327b1df33b63b) )
        ROM_LOAD( "mpr-23614.ic4",  0x3800000, 0x1000000, CRC(e5b8c5e5) SHA1(4b81ecc6375bf731dc3423cf87c2228eb304f2b5) )
        ROM_LOAD( "mpr-23615.ic5",  0x4800000, 0x1000000, CRC(95c35866) SHA1(a3dd4cd2c8818a3c5de7aa6c4afc74f872d52b37) )
        ROM_LOAD( "mpr-23616.ic6",  0x5800000, 0x1000000, CRC(6288848f) SHA1(a69ddde96d15cee9154ed3e87514286db6bbb622) )
        ROM_LOAD( "mpr-23617.ic7",  0x6800000, 0x1000000, CRC(19298892) SHA1(78493587dc49c7c99c5a98f152b17500003316b3) )
        ROM_LOAD( "mpr-23618.ic8",  0x7800000, 0x1000000, CRC(67d7b659) SHA1(44e75e16f2740c6e147e101a8714be8c5d9b71f3) )
        ROM_LOAD( "mpr-23619.ic9",  0x8800000, 0x1000000, CRC(a5f4f6af) SHA1(bb89d3f0f5bbaf7c40fa43680c7e51ef93f7ed26) )
        ROM_LOAD( "mpr-23620.ic10", 0x9800000, 0x1000000, CRC(67aa15a9) SHA1(42c24cbf7069c27430a71509a872cd1c4224aaeb) )
ROM_END

ROM_START( vf4cart )
	NAOMI2_BIOS

	ROM_REGION( 0xb800000, "user1", ROMREGION_ERASEFF)
        ROM_LOAD( "epr-23785.ic22", 0x0000000, 0x0400000, CRC(9bd98d4b) SHA1(3b0622625317cd6b2736c5b4a23484fb8bf39e4b) )
        ROM_LOAD( "mpr-23774.ic1",  0x0800000, 0x1000000, CRC(0fe7b864) SHA1(357ca3a5d96d7ff27e06367e115ddfd00cb260e3) )
        ROM_LOAD( "mpr-23775.ic2",  0x1800000, 0x1000000, BAD_DUMP CRC(6c64d9d9) SHA1(d2033ca3ab9838cad39e17b5c1eb079c6d15b758) )
        ROM_LOAD( "mpr-23776.ic3",  0x2800000, 0x1000000, CRC(44b8429e) SHA1(0ec6b6156bef1621700791651903a4589f4b5f84) )
        ROM_LOAD( "mpr-23777.ic4",  0x3800000, 0x1000000, CRC(78a4264e) SHA1(40d045240173f330ac6f108b132f9a87884922be) )
        ROM_LOAD( "mpr-23778.ic5",  0x4800000, 0x1000000, CRC(02dee78b) SHA1(e08ce0fc0b1db2dcef957c2edb6d51db400a38cb) )
        ROM_LOAD( "mpr-23779.ic6",  0x5800000, 0x1000000, CRC(6e458eea) SHA1(4c85fcacf4ff46d4a137afcf5906092fd88fe4b1) )
        ROM_LOAD( "mpr-23780.ic7",  0x6800000, 0x1000000, CRC(a775a51c) SHA1(b3eae20e5e7d74252368fd902c4e94a6ba6cb154) )
        ROM_LOAD( "mpr-23781.ic8",  0x7800000, 0x1000000, BAD_DUMP CRC(689457fa) SHA1(734b6b5f2ba78af24a613aef8d4e739f296413f7) )
        ROM_LOAD( "mpr-23782.ic9",  0x8800000, 0x1000000, CRC(4f72e901) SHA1(a1d231c446d2c34e5a7e7145754b2313a2d03fd4) )
        ROM_LOAD( "mpr-23783.ic10", 0x9800000, 0x1000000, CRC(c8d4f6f9) SHA1(9e9df605c050b3780d7df34bd5041d30bc084d2d) )
        ROM_LOAD( "mpr-23784.ic11", 0xa800000, 0x1000000, BAD_DUMP CRC(3a7ebe78) SHA1(6f414db03761be0d2f995193df8146f3b82c18a8) )
ROM_END

ROM_START( clubkrte )
	NAOMI2_BIOS

	ROM_REGION( 0xb800000, "user1", ROMREGION_ERASEFF)
        ROM_LOAD( "epr-23704.ic22", 0x0000000, 0x0400000, CRC(ff700a0d) SHA1(e2db0d2bd7dc88b3a487077e8ce56eb6cfd9b02d) )
        ROM_LOAD( "mpr-23693.ic1",  0x0800000, 0x1000000, CRC(28995764) SHA1(a1457f9935dde2e5aaa5ef245c736c0f2f8c74b7) )
        ROM_LOAD( "mpr-22694.ic2",  0x1800000, 0x1000000, CRC(37d30111) SHA1(4c07df8cd548cac79d48709e61f692d762471f8f) )
        ROM_LOAD( "mpr-22695.ic3",  0x2800000, 0x1000000, BAD_DUMP CRC(9fdeaa70) SHA1(2d01a49b8d8d23f61584bec31d75723cee47dc44) )
        ROM_LOAD( "mpr-22696.ic4",  0x3800000, 0x1000000, BAD_DUMP CRC(cb12ae00) SHA1(cf94ae123217071c169aa79c842f0a28abba9713) )
        ROM_LOAD( "mpr-22697.ic5",  0x4800000, 0x1000000, CRC(1383c742) SHA1(6efd17632a277a4bb0e47cc912fbc9865a8b14c3) )
        ROM_LOAD( "mpr-22698.ic6",  0x5800000, 0x1000000, BAD_DUMP CRC(1701cd3a) SHA1(e217206745ce9dca51733f77eb093b3ef2a10c5e) )
        ROM_LOAD( "mpr-22699.ic7",  0x6800000, 0x1000000, BAD_DUMP CRC(e58cfa77) SHA1(27c421c37d7e3df88cb7b2240bfec1ae460001a3) )
        ROM_LOAD( "mpr-22700.ic8",  0x7800000, 0x1000000, CRC(db9e5c1d) SHA1(db918c0fa1860f4345806e574d44354aba5fcd54) )
        ROM_LOAD( "mpr-22701.ic9",  0x8800000, 0x1000000, BAD_DUMP CRC(66ee25be) SHA1(17804308aa63bf58d2bde0b4b202d2bd0974a981) )
        ROM_LOAD( "mpr-22702.ic10", 0x9800000, 0x1000000, CRC(e302b582) SHA1(787192ed9f9a08541eecc3128855485cad802a42) )
        ROM_LOAD( "mpr-22703.ic11", 0xa800000, 0x1000000, CRC(702b8b4a) SHA1(3a8dfde458f341e7db20664382b9fce2b6e5d462) )
ROM_END

/**********************************************
 *
 * Naomi 2 GD-ROM defines
 *
 *********************************************/

ROM_START( vstrik3 )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0006", 0, SHA1(aca09a88506f5e462ad3fb33eac5478a2a010609) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0304-COM)
	//(sticker 253-5508-0304)
	ROM_LOAD("317-0304-com.data", 0x00, 0x50, CRC(a181c601) SHA1(6a489904941e638ac1069b66e76ee0bcec7d0bab) )
ROM_END

ROM_START( vf4 )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0012", 0, SHA1(c34588f59c6091cd1c3ef235171dad8d5247e707) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0314-COM)
	//(sticker 253-5508-0314)
	ROM_LOAD("317-0314-com.data", 0x00, 0x50, CRC(91a97eb4) SHA1(059342368bc5d25b494ed3c729870695f9584fc7) )
ROM_END

ROM_START( vf4b )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0012b", 0, SHA1(9b8e05c3d28a09323b13c198dfcc2b771bba67cd) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0314-COM)
	//(sticker 253-5508-0314)
	ROM_LOAD("317-0314-com.data", 0x00, 0x50, CRC(91a97eb4) SHA1(059342368bc5d25b494ed3c729870695f9584fc7) )
ROM_END

ROM_START( vf4c )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0012c", 0, SHA1(0ec149d7edfb326777cdc45a2ac8ad578a32aba1) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0314-COM)
	//(sticker 253-5508-0314)
	ROM_LOAD("317-0314-com.data", 0x00, 0x50, CRC(91a97eb4) SHA1(059342368bc5d25b494ed3c729870695f9584fc7) )
ROM_END


ROM_START( vf4evo )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0024b", 0, SHA1(a829169542f3bed76095ad6bfbbde7d494d04d72) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0338-JPN)
	//(sticker 253-5508-0338J)
	ROM_LOAD("317-0338-jpn.data", 0x00, 0x50, CRC(eeb2c9e9) SHA1(d30b5914c603219daea9923e1cf8da2be6096742) )
ROM_END

ROM_START( vf4evoa )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0024a", 0, SHA1(6225e778d73db18be26f882d4f9cd3b3a136d1c9) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0338-JPN)
	//(sticker 253-5508-0338J)
	ROM_LOAD("317-0338-jpn.data", 0x00, 0x50, CRC(eeb2c9e9) SHA1(d30b5914c603219daea9923e1cf8da2be6096742) )
ROM_END

ROM_START( initdv2j )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0026", 0, SHA1(253acede106b7fbf49e24458e7fda868720e9549) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gds-0026.rom", 0x00, 0x50, NO_DUMP) // file on GD-ROM is BFK.BIN , _NOT_ BEM.BIN which is for Initial D : Arcade Stage (Japan)
ROM_END

ROM_START( vf4tuned ) // are there multiple files on this GD-ROM? it only compresses to 500 meg when the rom file is closer to half tha
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0036f", 0, SHA1(ea35d6ecdf94e5c9a545952758da80f658755df0) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0387-COM)
	//(sticker 253-5508-0387)
	ROM_LOAD("317-0387-com.data", 0x00, 0x50, CRC(ab9f3851) SHA1(8b64dc6df176eb7adb48267709a27db221d5e3c3) )
ROM_END

ROM_START( vf4tunedd )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0036d", 0, SHA1(2f7654307a4c978c5af6c8238c44e70275dd34f9) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0387-COM)
	//(sticker 253-5508-0387)
	ROM_LOAD("317-0387-com.data", 0x00, 0x50, CRC(ab9f3851) SHA1(8b64dc6df176eb7adb48267709a27db221d5e3c3) )
ROM_END


ROM_START( vf4tuneda )
	NAOMI2_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0036a", 0, SHA1(cd630fc4e8f7ed5641b85c609584d7efe0eac137) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0387-COM)
	//(sticker 253-5508-0387)
	ROM_LOAD("317-0387-com.data", 0x00, 0x50, CRC(ab9f3851) SHA1(8b64dc6df176eb7adb48267709a27db221d5e3c3) )
ROM_END


/*

Title   BEACH SPIKERS
Media ID    0897
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDS-0014
Version V1.001
Release Date    20010613
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 449 705600
track02.raw 600 2746    5049744
track03.bin 45150   549299  1185760800

PIC

253-5508-0317
317-0317-COM

*/

ROM_START( beachspi )
	NAOMI2_BIOS
//  NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0014", 0, SHA1(1ebb3695196c11a86276e034df2e1c8d7fa6b96f) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0317-COM)
	//(sticker 253-5508-0317)
	ROM_LOAD("317-0317-com.data", 0x00, 0x50, CRC(66efe433) SHA1(7f7b52202ed9b1e20516aaa7553cc3cc677a70b5) )
ROM_END

ROM_START( initd )
	NAOMI2_BIOS
//  NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0020b", 0, SHA1(c0e901623ef4fcd97b7e4d29ae556e6f2e91b8ad) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	//PIC16C622A (317-0331-JPN)
	//(sticker 253-5508-0331J)
	ROM_LOAD("317-0331-jpn.data", 0x00, 0x50, CRC(bb39742e) SHA1(b3100b18aeb80ebfd5312ba5c320e7e647710b55) )
ROM_END

ROM_START( initdexp )
	NAOMI2_BIOS
//  NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0025", 0, SHA1(8ea92cf6b493f21b9453832edad7cbc5e5b350c1) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0343-com.data", 0x00, 0x50, CRC(e9d8fac0) SHA1(85f5bbffbd9d1f7162bae46ddd49e7870fe93662) )
ROM_END

ROM_START( initdv3j )
	NAOMI2_BIOS
//  NAOMIGD_BIOS

	ROM_REGION( 0x10000000, "user1", ROMREGION_ERASE) // allocate max size in init instead?

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gds-0032b", 0, SHA1(568411aa72ca308a03a6b5b61c79833464b88bc6) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gds-0032_pic", 0x00, 0x50, NO_DUMP ) // PIC was missing
ROM_END


GAME( 2001, naomi2,   0,        naomi,    naomi,    0, ROT0, "Sega",            "Naomi 2 Bios", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )

//Naomi 2 Cart Games
/* Complete Dumps */
GAME( 2001, vstrik3c, naomi2,  naomi,    naomi,    0,  ROT0,  "Sega",          "Virtua Striker 3 (Cart) (USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, wldrider, naomi2,  naomi,    naomi,    0,  ROT0,  "Sega",          "Wild Riders (JPN, USA, EXP, KOR, AUS)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, vf4cart,  naomi2,  naomi,    naomi,    0,  ROT0,  "Sega",          "Virtua Fighter 4 (Cart)", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, clubkrte, naomi2,  naomi,    naomi,    0,  ROT0,  "Sega",          "Club Kart: European Session", GAME_UNEMULATED_PROTECTION|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )

/* GDS-xxxx (Sega first party games) */
GAME( 2001, vstrik3, naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Virtua Striker 3 (GDS-0006)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, vf4,     naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 (GDS-0012)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, vf4b,    vf4,     naomigd,    naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 (Rev B) (GDS-0012B)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2001, vf4c,    vf4,     naomigd,    naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 (Rev C) (GDS-0012C)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, initd,   naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Initial D Arcade Stage (Rev B) (Japan) (GDS-0020B)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, beachspi,naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Beach Spikers (GDS-0014)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, initdexp,naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Initial D Arcade Stage (Export) (GDS-0025)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, vf4evo,  naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 Evolution (Rev B) (GDS-0024B)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, vf4evoa, vf4evo,  naomigd,    naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 Evolution (Rev A) (GDS-0024A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2002, initdv2j,naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Initial D : Arcade Stage Ver. 2 (Japan) (GDS-0026)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, initdv3j,naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Initial D : Arcade Stage Ver. 3 (Japan) (Rev B) (GDS-0032B)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, vf4tuned,naomi2,  naomigd,    naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 Final Tuned (Rev F) (GDS-0036F)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, vf4tunedd,vf4tuned,naomigd,   naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 Final Tuned (Rev D) (GDS-0036D)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, vf4tuneda,vf4tuned,naomigd,   naomi,    0,  ROT0, "Sega",          "Virtua Fighter 4 Final Tuned (Rev A) (GDS-0036A)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )

/**********************************************
 *
 * Atomiswave cart defines
 *
 *********************************************/

typedef struct _atomiswave_key atomiswave_key;
struct _atomiswave_key
{
    int P0[16];
    int S0[32];
    int S1[16];
    int S2[16];
    int S3[8];
};

static const atomiswave_key ssu_key = {
    {8,10,1,3,7,4,11,2,5,15,6,0,12,13,9,14},
    {4,12,8,14,16,30,31,0,23,29,24,21,11,22,27,5,3,20,18,26,10,7,17,1,28,6,15,13,2,9,25,19},
    {13,1,0,9,5,12,4,14,3,15,2,10,11,6,8,7},
    {7,13,4,6,5,9,3,2,0,15,12,10,8,11,1,14},
    {4,0,1,2,5,7,3,6}
};

static const atomiswave_key rm_key = {
    {4,5,9,6,1,13,7,11,10,0,14,12,8,15,2,3},
    {3,0,14,17,10,15,31,20,13,2,29,28,9,18,25,27,6,19,30,22,7,12,1,16,23,11,24,4,8,26,21,5},
    {2,10,6,9,11,13,4,5,3,15,7,14,12,1,0,8},
    {1,13,11,3,8,7,9,10,12,15,4,14,0,5,6,2},
    {6,5,0,3,7,1,4,2}
};

static const atomiswave_key fotns_key = {
    {12,7,11,2,0,5,15,6,1,8,14,4,9,13,3,10},
    {4,7,9,22,13,8,24,19,10,5,26,27,14,21,30,28,1,20,25,17,0,11,6,23,16,12,31,3,15,29,18,2},
    {3,11,7,8,10,12,5,4,2,14,6,15,13,0,1,9},
    {10,6,0,8,3,12,2,1,7,4,15,5,11,14,13,9},
    {7,4,1,2,6,0,5,3}
};

static const atomiswave_key xh_key = {
    {12,7,11,2,0,5,15,6,1,8,14,4,9,13,3,10},
    {4,12,8,14,16,30,31,0,23,29,24,21,11,22,27,5,3,20,18,26,10,7,17,1,28,6,15,13,2,9,25,19},
    {13,1,0,9,5,12,4,14,3,15,2,10,11,6,8,7},
    {7,13,4,6,5,9,3,2,0,15,12,10,8,11,1,14},
    {4,0,1,2,5,7,3,6}
};

static const atomiswave_key xh2_key = {
    {14,1,11,15,7,3,8,13,0,4,2,12,6,10,5,9},
    {11,3,7,1,31,17,16,15,24,18,23,26,4,25,20,10,12,27,29,21,5,8,30,14,19,9,0,2,13,6,22,28},
    {7,11,10,3,15,6,14,4,9,5,8,0,1,12,2,13},
    {11,1,8,10,9,5,15,14,12,3,0,6,4,7,13,2},
    {4,0,1,2,5,7,3,6}
};

static const atomiswave_key dolphin_key = {
    {8,10,1,3,7,4,11,2,5,15,6,0,12,13,9,14},
    {11,8,6,25,2,7,23,28,5,10,21,20,1,26,17,19,14,27,22,30,15,4,9,24,31,3,16,12,0,18,29,13},
    {13,5,9,6,4,2,11,10,12,0,8,1,3,14,15,7},
    {1,13,11,3,8,7,9,10,12,15,4,14,0,5,6,2},
    {3,0,5,6,2,4,1,7}
};

static const atomiswave_key demofist_key = {
    {4,5,9,6,1,13,7,11,10,0,14,12,8,15,2,3},
    {9,15,28,7,13,24,2,23,21,1,22,16,18,8,17,31,27,6,30,12,4,20,5,19,0,25,3,29,10,14,11,26},
    {5,2,13,11,8,6,12,1,4,3,0,10,14,15,7,9},
    {11,6,10,0,12,1,8,14,2,9,13,3,7,4,15,5},
    {1,5,6,2,4,7,3,0}
};

static const atomiswave_key rumblef_key = {
    {4,5,9,6,1,13,7,11,10,0,14,12,8,15,2,3},
    {11,3,7,1,31,17,16,15,24,18,23,26,4,25,20,10,12,27,29,21,5,8,30,14,19,9,0,2,13,6,22,28},
    {7,11,10,3,15,6,14,4,9,5,8,0,1,12,2,13},
    {11,1,8,10,9,5,15,14,12,3,0,6,4,7,13,2},
    {4,0,1,2,5,7,3,6}
};

static const atomiswave_key ngbc_key = {
    {4,5,9,6,1,13,7,11,10,0,14,12,8,15,2,3},
    {17,25,29,27,5,11,10,21,2,8,13,0,30,3,14,16,22,1,7,15,31,18,4,20,9,19,26,24,23,28,12,6},
    {15,3,2,11,7,14,6,12,1,13,0,8,9,4,10,5},
    {6,12,5,7,4,8,2,3,1,14,13,11,9,10,0,15},
    {5,1,0,3,4,6,2,7}
};

static const atomiswave_key kofnw_key = {
    {4,5,9,6,1,13,7,11,10,0,14,12,8,15,2,3},
    {22,16,3,24,18,7,29,8,10,30,9,15,13,23,14,0,4,25,1,19,27,11,26,12,31,6,28,2,21,17,20,5},
    {8,15,0,6,5,11,1,12,9,14,13,7,3,2,10,4},
    {2,15,3,9,5,8,1,7,11,0,4,10,14,13,6,12},
    {4,0,3,7,1,2,6,5}
};

static const atomiswave_key kov7sprt_key = {
    {14,1,11,15,7,3,8,13,0,4,2,12,6,10,5,9},
    {17,3,31,2,28,10,9,29,6,25,24,8,13,1,19,15,22,0,14,20,16,7,21,4,18,26,27,5,12,23,11,30},
    {4,8,11,15,3,14,7,12,1,0,9,5,6,13,2,10},
    {14,0,9,11,4,1,7,5,13,6,8,12,2,3,10,15},
    {2,1,0,5,4,6,7,3}
};

static const atomiswave_key ggisuka_key = {
    {12,7,11,2,0,5,15,6,1,8,14,4,9,13,3,10},
    {14,6,2,4,26,20,21,10,29,23,18,31,1,28,17,15,9,30,24,16,0,13,27,11,22,12,5,7,8,3,19,25},
    {5,9,8,1,13,4,12,6,11,7,10,2,3,14,0,15},
    {15,5,12,14,13,1,11,10,8,7,4,2,0,3,9,6},
    {1,5,4,7,0,2,6,3}
};

static UINT16 atomiswave_decrypt(UINT16 cipherText, int address, const atomiswave_key* key)
{
    int b0,b1,b2,b3;
    int aux;

    aux = BITSWAP16(cipherText,
                    key->P0[15],key->P0[14],key->P0[13],key->P0[12],key->P0[11],key->P0[10],key->P0[9],key->P0[8],
                    key->P0[7],key->P0[6],key->P0[5],key->P0[4],key->P0[3],key->P0[2],key->P0[1],key->P0[0]);
    aux = aux ^ BITSWAP16(address/2, 13,5,2, 14,10,9,4, 15,11,6,1, 12,8,7,3,0);

    b0 = aux&0x1f;
    b1 = (aux>>5)&0xf;
    b2 = (aux>>9)&0xf;
    b3 = aux>>13;

    b0 = key->S0[b0];
    b1 = key->S1[b1];
    b2 = key->S2[b2];
    b3 = key->S3[b3];

    return (b3<<13)|(b2<<9)|(b1<<5)|b0;
}

static DRIVER_INIT( atomiswave )
{
	UINT64 *ROM = (UINT64 *)memory_region(machine, "maincpu");
#if 0
	UINT8 *dcdata = (UINT8 *)memory_region(machine, "user1");
	FILE *f;

	f = fopen("aw.bin", "wb");
	fwrite(dcdata, 0x8000000, 1, f);
	fclose(f);
#endif
	// patch out long startup delay
	ROM[0x98e/8] = (ROM[0x98e/8] & U64(0xffffffffffff)) | (UINT64)0x0009<<48;

	intelflash_init(machine, 0, FLASH_MACRONIX_29L001MC, memory_region(machine, "maincpu"));
}


static DRIVER_INIT(fotns)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &fotns_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}



static DRIVER_INIT(demofist)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &demofist_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(sprtshot)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &ssu_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(rangrmsn)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &rm_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(xtrmhunt)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &xh_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(xtrmhnt2)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &xh2_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(dolphin)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &dolphin_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(rumblef)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &rumblef_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(ngbc)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &ngbc_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(kofnw)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &kofnw_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(kov7sprt)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &kov7sprt_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

static DRIVER_INIT(ggisuka)
{
  	int i;
	UINT16 *src = (UINT16 *)(memory_region(machine, "user1"));

	long rom_size = memory_region_length(machine, "user1");

	for(i=0; i<rom_size/2; i++)
	{
		src[i] = atomiswave_decrypt(src[i], i*2, &ggisuka_key);
	}

	DRIVER_INIT_CALL(atomiswave);
}

ROM_START( fotns )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
	ROM_LOAD("ax1901p01.ic18", 0x0000000, 0x0800000,  CRC(a06998b0) SHA1(d617691db5170f6db176e40fc732966d523fd8cf) )
	ROM_LOAD("ax1901m01.ic11", 0x1000000, 0x1000000,  CRC(ff5a1642) SHA1(49cefcce173f9a811fe9c0c07bee53aeba2bc3a8) )
	ROM_LOAD("ax1902m01.ic12", 0x2000000, 0x1000000,  CRC(d9aae8a9) SHA1(bf87034088be0847b6e297b7665e0ea4d8cba631) )
	ROM_LOAD("ax1903m01.ic13", 0x3000000, 0x1000000,  CRC(1711b23d) SHA1(ab628b2ec678839c75245e245297818ef1592d3b) )
	ROM_LOAD("ax1904m01.ic14", 0x4000000, 0x1000000,  CRC(443bfb26) SHA1(6f7751afa0ca55dd0679758b27bed92b31c1b050) )
	ROM_LOAD("ax1905m01.ic15", 0x5000000, 0x1000000,  CRC(eb1cada0) SHA1(459d21d622c72606f1d3095e8a25b6c4adccf8ab) )
	ROM_LOAD("ax1906m01.ic16", 0x6000000, 0x1000000,  CRC(fe6da168) SHA1(d4ab6443383469bb5a4337005de917627a2e21cc) )
	ROM_LOAD("ax1907m01.ic17", 0x7000000, 0x1000000,  CRC(9d3a0520) SHA1(78583fd171b34439f77a04a97ebe3c9d1bab61cc) )
ROM_END

ROM_START( rangrmsn )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
        ROM_LOAD( "ax1601p01.ic18", 0x0000000, 0x0800000, CRC(00a74fbb) SHA1(57cc1eedd22d1f553956a825e69a597309ee2bef) )
        ROM_LOAD( "ax1601m01.ic11", 0x1000000, 0x1000000, CRC(f34eed33) SHA1(1c171fb8aa95877f81ed78652d4a9ff80f7713ff) )
        ROM_LOAD( "ax1602m01.ic12", 0x2000000, 0x1000000, CRC(a7d59efb) SHA1(a40938ce1399babefc8cf02f579a86cf08e211ef) )
        ROM_LOAD( "ax1603m01.ic13", 0x3000000, 0x1000000, CRC(7c0aa241) SHA1(3e0e5ff3307dcfa52998fb9b4b14bf54bd056a99) )
        ROM_LOAD( "ax1604m01.ic14", 0x4000000, 0x1000000, CRC(d2369144) SHA1(da1eae9957d27d1682c4191780cf51b32dfe6659) )
        ROM_LOAD( "ax1605m01.ic15", 0x5000000, 0x1000000, CRC(0c11c1f9) SHA1(0585db60618c5b97f9b7c203baf7e5ac90883ca6) )
ROM_END

ROM_START( sprtshot )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
        ROM_LOAD( "ax0101p01.ic18", 0x0000000, 0x0800000, CRC(b3642b5d) SHA1(85eabd9551aefb825ae8eb6422092fb5a58d60f6) )
        ROM_LOAD( "ax0101m01.ic11", 0x1000000, 0x1000000, CRC(1e39184d) SHA1(663e0cb9f43a0f89d9841e04b3d009f6c5e88d5e) )
        ROM_LOAD( "ax0102m01.ic12", 0x2000000, 0x1000000, CRC(700764d1) SHA1(310f1606f7bbed1012c119f1ef5d89d231d8489e) )
        ROM_LOAD( "ax0103m01.ic13", 0x3000000, 0x1000000, CRC(6144e7a8) SHA1(4d4341082f008dfd93ef5bf32a44c80869ef02a8) )
        ROM_LOAD( "ax0104m01.ic14", 0x4000000, 0x1000000, CRC(ccb72150) SHA1(a1032d321c27f9ff43da41f20b8687bf1958ddc9) )
ROM_END

ROM_START( xtrmhunt )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
	ROM_LOAD("ax2401p01.ic18", 0x0000000, 0x0800000,  CRC(8e2a11f5) SHA1(b5106314fb8d4483254e83ac3982039bb60a78e8) )
	ROM_LOAD("ax2401m01.ic11", 0x1000000, 0x1000000,  CRC(76dbc286) SHA1(8f36ca94b8e67c76e0f90b21debc5ac7890f0da1) )
	ROM_LOAD("ax2402m01.ic12", 0x2000000, 0x1000000,  CRC(cd590ea2) SHA1(ee5e38bf68e95da665be478ebba9cc5ffed52bb7) )
	ROM_LOAD("ax2403m01.ic13", 0x3000000, 0x1000000,  CRC(06f62eb5) SHA1(f7e8d1dda6bb59ca2bc7cfa1105889b9e8e6d55d) )
	ROM_LOAD("ax2404m01.ic14", 0x4000000, 0x1000000,  CRC(759ef5cb) SHA1(27ac2d12c6fb358b3d631c017c7b693e5ad95fd7) )
	ROM_LOAD("ax2405m01.ic15", 0x5000000, 0x1000000,  CRC(940d77f1) SHA1(eefdfcb92873032dc7d9ff9310bf5ed715c8bf4f) )
	ROM_LOAD("ax2406m01.ic16", 0x6000000, 0x1000000,  CRC(cbcf2c5d) SHA1(61362fabcbb3bfc01c996748a7ca65f8a0e02f2f) )
ROM_END

ROM_START( xtrmhnt2 )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
        ROM_LOAD( "610-0752.u3",  0x0000000, 0x1000000, CRC(bab6182e) SHA1(4d25256c81941316887cbb4524a203922f5b7104) )
        ROM_LOAD( "610-0752.u1",  0x1000000, 0x1000000, CRC(3086bc47) SHA1(eb7b04db90d296985528f0cfdd4545f184c40b64) )
        ROM_LOAD( "610-0752.u2",  0x2000000, 0x1000000, CRC(d3a88b31) SHA1(ccf14367e4e7efbc2cc835f3b001fd6d64302a5e) )
        ROM_LOAD( "610-0752.u4",  0x3000000, 0x1000000, CRC(9787f145) SHA1(8445ede0477f70fbdc113810b80356945ce498d2) )
        ROM_LOAD( "610-0752.u14", 0x4000000, 0x1000000, CRC(ce83bcc7) SHA1(e2d324a5a7eacbec7b0df9a4b9e276521bb9ab80) )
        ROM_LOAD( "610-0752.u15", 0x5000000, 0x1000000, CRC(864a6342) SHA1(fb97532d5dd00f8520fdaf68dfcd1ea627bdf90a) )
        ROM_LOAD( "610-0752.u16", 0x6000000, 0x1000000, CRC(8ac71c76) SHA1(080e41e633bf082fc536781541c6031d1ac81939) )
        ROM_LOAD( "610-0752.u17", 0x7000000, 0x1000000, CRC(a79fb1fa) SHA1(f75c5b574fd79677b926c595b369e95605a3c848) )

	ROM_REGION( 0x1400000, "network", 0)	// network board
        ROM_LOAD( "fpr-24330a.ic2", 0x000000, 0x400000, CRC(8d89877e) SHA1(6caafc49114eb0358e217bc2d1a3ab58a93c8d19) )
        ROM_LOAD( "flash128.ic4s", 0x400000, 0x1000000, CRC(866ed675) SHA1(2c4c06935b7ab1876e640cede51713b841833567) )
ROM_END

ROM_START( dolphin )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
	ROM_LOAD( "ax0401p01.ic18", 0x0000000, 0x0800000, CRC(195d6328) SHA1(cf3b5699f81235919dd3b1974d2ecb0376cb4552) )
	ROM_LOAD( "ax0401m01.ic11", 0x1000000, 0x1000000, CRC(5e5dca57) SHA1(e0623c84f66cada37d4c9399a7a8fc6866933144) )
	ROM_LOAD( "ax0402m01.ic12", 0x2000000, 0x1000000, CRC(77dd4771) SHA1(dcd23b8ddc82eab2f325266ffd7ed3fbc1bcdf71) )
	ROM_LOAD( "ax0403m01.ic13", 0x3000000, 0x1000000, CRC(911d0674) SHA1(eec35badcfbfe412b7104a86c2111f5a1b5fb5cd) )
	ROM_LOAD( "ax0404m01.ic14", 0x4000000, 0x1000000, CRC(f82a4ca3) SHA1(da686d86e176a9f24874d2916b1932f03a99a52d) )
	ROM_LOAD( "ax0405m01.ic15", 0x5000000, 0x1000000, CRC(b88298d7) SHA1(490c3ec471018895b7268ee33498dddaccbbfd5a) )
ROM_END

ROM_START( demofist )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
	ROM_LOAD( "ax0601p01.ic18", 0x0000000, 0x0800000, CRC(0efb38ad) SHA1(9400e37efe3e936474d74400ebdf28ad0869b67b) )
	ROM_LOAD( "ax0601m01.ic11", 0x1000000, 0x1000000, CRC(12fda2c7) SHA1(3afbac221ffe249386e4cb50b4edd013d9a40062) )
	ROM_LOAD( "ax0602m01.ic12", 0x2000000, 0x1000000, CRC(aea61fdf) SHA1(0a088848bbf7a47df8b44b69bf72ed0d4a1088f8) )
	ROM_LOAD( "ax0603m01.ic13", 0x3000000, 0x1000000, CRC(d5879d35) SHA1(977cd3b373c6f94eb21ffb24ff564971d3d633e5) )
	ROM_LOAD( "ax0604m01.ic14", 0x4000000, 0x1000000, CRC(a7b09048) SHA1(229fa2332b58fec2a712c3ebd672662f35a9485a) )
	ROM_LOAD( "ax0605m01.ic15", 0x5000000, 0x1000000, CRC(18d8437e) SHA1(fe2e189e40a89141335e754268d29d46e3eb3bb8) )
	ROM_LOAD( "ax0606m01.ic16", 0x6000000, 0x1000000, CRC(42c81617) SHA1(1cc686af5e3fc56143836e3dcc0067893f82fcf9) )
	ROM_LOAD( "ax0607m01.ic17", 0x7000000, 0x1000000, CRC(96e5aa84) SHA1(e9841f550f2ef409d97004542bcadabb6b9e84af) )
ROM_END

ROM_START( rumblef )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
	ROM_LOAD( "ax1801p01.ic18", 0x0000000, 0x0800000, CRC(2f7fb163) SHA1(bf819d798d9a3a7bc754e111a3f53b9db6d6042a) )
	ROM_LOAD( "ax1801m01.ic11", 0x1000000, 0x1000000, CRC(c38aa61c) SHA1(e2f688a0aa8b0119f5fd3d53c8904e035d43a4b1) )
	ROM_LOAD( "ax1802m01.ic12", 0x2000000, 0x1000000, CRC(72e0ebc8) SHA1(e85300a405ea14c4c9d857eb9685c93faaca1d56) )
	ROM_LOAD( "ax1803m01.ic13", 0x3000000, 0x1000000, CRC(d0f59d98) SHA1(b854796087e9f76a13a21da8249f7224e451e129) )
	ROM_LOAD( "ax1804m01.ic14", 0x4000000, 0x1000000, CRC(15595cba) SHA1(8dd06d1f986cd21a58d20b662b11ed7ba8a6ff7a) )
	ROM_LOAD( "ax1805m01.ic15", 0x5000000, 0x1000000, CRC(3d3f8e0d) SHA1(364a0bda890722b9fb72171f96c742b8f3fef23e) )
	ROM_LOAD( "ax1806m01.ic16", 0x6000000, 0x1000000, CRC(ac2751bb) SHA1(5070fa12bf109ab87e8f7ea46ac4ae78a73105da) )
	ROM_LOAD( "ax1807m01.ic17", 0x7000000, 0x1000000, CRC(3b2fbdb0) SHA1(f9f7e06785d3a07282247aaedd9999aa7c2670b9) )
ROM_END

ROM_START( ngbc )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
	ROM_LOAD( "ax3301p01.fmem1", 0x000000, 0x800000, CRC(f7e24e67) SHA1(8eef26d44b294faa509304b1b04f4d801337bc99) )
ROM_END

ROM_START( kofnw )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
	ROM_LOAD( "ax2201p01.ic18", 0x0000000, 0x0800000, CRC(27aab918) SHA1(41c5ddd8bd4c91481750606ab44aa115b5fe01d0) )
	ROM_LOAD( "ax2201m01.ic11", 0x1000000, 0x1000000, CRC(22ea665b) SHA1(292c92c9ae43eea2d1c27cedfb89c3956b8dea32) )
	ROM_LOAD( "ax2202m01.ic12", 0x2000000, 0x1000000, CRC(7fad1bea) SHA1(89f3f88af48973a4685955d86ef97a1487b8e7a8) )
	ROM_LOAD( "ax2203m01.ic13", 0x3000000, 0x1000000, CRC(78986ca4) SHA1(5a6c8c12955573f33361d2c6f20f85de35ac7bae) )
	ROM_LOAD( "ax2204m01.ic14", 0x4000000, 0x1000000, CRC(6ffbeb04) SHA1(975062cf364589dbdd5c5cb5ca945f76d87fc120) )
	ROM_LOAD( "ax2205m01.ic15", 0x5000000, 0x1000000, CRC(2851b791) SHA1(566ef95ea066b7bf548986085670242be217befc) )
	ROM_LOAD( "ax2206m01.ic16", 0x6000000, 0x1000000, CRC(e53eb965) SHA1(f50cd53a5859f081d8a278d24a519c9d9b49ab96) )
ROM_END

ROM_START( kov7sprt )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x8000000, "user1", ROMREGION_ERASE)
        ROM_LOAD( "ax1301p01.ic18", 0x0000000, 0x0800000, CRC(6833a334) SHA1(646aaa578e09ad23bc9c7f4fbdfb3c1486916fd3) )
        ROM_LOAD( "ax1301m01.ic11", 0x1000000, 0x1000000, CRC(58ae7ca1) SHA1(e91975697b797ea05488ace649cbb9964b4cd500) )
        ROM_LOAD( "ax1301m02.ic12", 0x2000000, 0x1000000, CRC(871ea03f) SHA1(6806910832ca271a9240aca8e91279556e5b0cb7) )
        ROM_LOAD( "ax1301m03.ic13", 0x3000000, 0x1000000, CRC(abc328bc) SHA1(d9271d4e5abe76d31de0f60a5c106260338d42e9) )
        ROM_LOAD( "ax1301m04.ic14", 0x4000000, 0x1000000, CRC(25a176d1) SHA1(6d815bf6acb645fead060733660e24fb0d44282d) )
        ROM_LOAD( "ax1301m05.ic15", 0x5000000, 0x1000000, CRC(e6573a93) SHA1(0666e52d0088263f28938e4c8aae201e604ec1f2) )
        ROM_LOAD( "ax1301m06.ic16", 0x6000000, 0x1000000, CRC(cb8cacb4) SHA1(5d008e8a934451b9bfa33fedfd492c86d9226ef5) )
        ROM_LOAD( "ax1301m07.ic17", 0x7000000, 0x1000000, CRC(0ca92213) SHA1(115c50fa55e6de3439de23e74621695510c6a7ba) )
ROM_END

ROM_START( ggisuka )
	ROM_REGION( 0x200000, "maincpu", 0)
	AW_BIOS

	ROM_REGION( 0x9000000, "user1", ROMREGION_ERASE)
        ROM_LOAD( "ax1201p01.ic18", 0x0000000, 0x0800000, CRC(0a78d52c) SHA1(e9006dc43cd11d5ba49a092a1dff31dc10700c28) )
        ROM_LOAD( "ax1202m01.ic11", 0x1000000, 0x1000000, CRC(dfc6fd67) SHA1(f9d35b18a03d22f70feda42d314b0f9dd54eea55) )
        ROM_LOAD( "ax1203m01.ic12", 0x2000000, 0x1000000, CRC(bf623df9) SHA1(8b9a8e8100ff6d2ce9a982ab8eb1d542f1c7af03) )
        ROM_LOAD( "ax1204m01.ic13", 0x3000000, 0x1000000, CRC(c80c3930) SHA1(5c39fde36e2ebbfe72967d7d0202eb454a8d3bbe) )
        ROM_LOAD( "ax1205m01.ic14", 0x4000000, 0x1000000, CRC(e99a269d) SHA1(a52148b82b0338b8bad8b52985302eaf81a4cfde) )
        ROM_LOAD( "ax1206m01.ic15", 0x5000000, 0x1000000, CRC(807ab795) SHA1(17c86b1a56333c05b68ff84f83e964d013c1819c) )
        ROM_LOAD( "ax1207m01.ic16", 0x6000000, 0x1000000, CRC(6636d1b8) SHA1(9bd8fc114557f6fbe772f85eeb246f7336d4255e) )
        ROM_LOAD( "ax1208m01.ic17", 0x7000000, 0x1000000, CRC(38bda476) SHA1(0234a6f5fbaf5e958b3ba0db311dff157f80addc) )
        ROM_LOAD( "ax1201m01.ic10", 0x8000000, 0x1000000, CRC(df96ce30) SHA1(25a9f743b1c2b11896d0c7a2dc1c198fc977aaca) )
ROM_END

/* Atomiswave */
GAME( 2001, awbios,   0,        aw,    aw,    0,        ROT0, "Sammy",                           "Atomiswave Bios", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )

GAME( 2002, sprtshot, awbios,   aw,    aw,    sprtshot, ROT0, "Sammy USA",                       "Sports Shooting USA", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2003, demofist, awbios,   aw,    aw,    demofist, ROT0, "Polygon Magic / Dimps",           "Demolish Fist", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2003, dolphin,  awbios,   aw,    aw,    dolphin,  ROT0, "Sammy",                           "Dolphin Blue", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 2003, ggisuka,  awbios,   aw,    aw,    ggisuka,  ROT0, "Sammy / Arc System Works",        "Guilty Gear Isuka", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, rumblef,  awbios,   aw,    aw,    rumblef,  ROT0, "Sammy / Dimps",                   "The Rumble Fish", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 2004, rangrmsn, awbios,   aw,    aw,    rangrmsn, ROT0, "Sammy",                           "Ranger Mission", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2004, kov7sprt, awbios,   aw,    aw,    kov7sprt, ROT0, "Sammy / IGS", 			 "Knights of Valour - The Seven Spirits", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 2005, ngbc,     awbios,   aw,    aw,    ngbc,     ROT0, "Sammy / SNK Playmore",            "Neo-Geo Battle Coliseum", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2005, fotns,    awbios,   aw,    aw,    fotns,    ROT0, "Sega / Arc System Works",         "Fist Of The North Star", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2005, kofnw,    awbios,   aw,    aw,    kofnw,    ROT0, "Sammy / SNK Playmore",            "The King of Fighters Neowave", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2005, xtrmhunt, awbios,   aw,    aw,    xtrmhunt, ROT0, "Sammy",                           "Extreme Hunting", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
GAME( 2006, xtrmhnt2, awbios,   aw,    aw,    xtrmhnt2, ROT0, "Sega",                            "Extreme Hunting 2", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
