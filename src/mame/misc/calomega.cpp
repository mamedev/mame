// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo
/**********************************************************************************

    .-----------------------------------------.
    |                                         |
    |         CAL OMEGA / CEI / UCMC          |
    |    SYSTEMS 903 / 904 / 905 / 906-III    |
    |                                         |
    |  Driver by Roberto Fresca & Grull Osgo  |
    |                                         |
    '-----------------------------------------'


    --- Technical Notes ---

    Name:    System 903, System 904, System 905 & System 906-III.
    Company: Cal Omega / Casino Electronics Inc. (CEI) / United Coin Machine Co. (UCMC).
    Year:    1981-1991


***********************************************************************************


    Basic hardware for systems 903/904/905:
    ---------------------------------------

    CPU =  1x SY6502 @ 625 kHz.  ; M6502 compatible.
    SND =  1x AY8912 @ 1.25 MHz. ; AY-3-8910 compatible.
    I/O =  2x PIAs SY6520/6820   ; M6821 compatible.
    CRCT = 1x SY6545             ; M6845 compatible mode. It doesn't seems that is using
                                 ; the first 4 bits of register R03 (HSync width).

    CLK =  1x Xtal @ 10 MHz.     ; CPU clock = 625 kHz.
                                 ; SND clock = 1250 kHz.
                                 ; PXL clock = 5000 kHz.

    PRG ROMS at locations u5, u6, u7, u8, u9.
    GFX ROMS at locations u67(cg0), u68(cga), u69(cgb), u70(cgc).

    Inputs use 6 lines from 6520 PIA's port, and each one is multiplexed x4.
    Another line from the same PIA (bit 6) is out of the multiplexed system, and is directly
    connected to the settings DIP switches (SW1-5).

    Some games like Keno and Big Game, also have a key encoder peripheral processor
    based on a 8035, and 1x ROM 2516/2532.



    System 903/904 specific:
    ------------------------

    1x MC6850 (ACIA) for serial communication.

    ROMs u5 to u9 are 2516.

    System 903 has 3x 8 DIP switches banks, wired to ACIA6850, PIA1, and AY8912 respectively.
    System 904 has 2x 8 DIP switches banks, wired to ACIA6850 and PIA1 respectively.
    Some 903 based PCBs like TUNI games only have 1x 8 DIP switches bank, wired to PIA1.

    Systems 903/904 PCBs have 3 pots to control R, G, and B independently.
    However, it seems to be only related to the background color.

    CPU lines A14 and A15 are not connected, so the CPU is addressing only 16KB.



    System 905 specific:
    --------------------

    ROMs u5 to u9 are 2532.

    1x 8 DIP switches bank, wired to PIA1.

    CPU line A15 is not connected, so the CPU is addressing only 32KB.



    System 906-III:
    ---------------

    PCB silkscreened "CEI 906 III GAMEBOARD" "ASSY 4029063 REV"

    CPU:   1x Rockwell R65C02P1 CPU. (U49)
    Video: 1x Motorola MC6845P CRTC. (U47)
    I/O:   2x Rockwell R65C24P2 PIAT. (U46, U48)
           1x Motorola MC6850P ACIA. (U44)

    Sound: 1x Microchip AY-3-8912. (U45)
           1x LM380N (amplifier). (U34)

    RAM: 1x NEC D4364C-15L. (U50)

    PRG ROMs: 5x ST M2764AF1. (U28/EPR1, U29/EPR2, U30/EPR3, U31/EPR4, U32/EPR5)
    GFX ROMs: 3x ST M2764AF1. (U4/CG2C, U5/CG2B, U6/CG2A)

    Clock: 1x 10.000 MHz Xtal. (Y1)

    2x 8 DIP switches banks. (SW1, SW2)
    1x Battery. (BT1)
    3x 1K Pots (video). (B, G, R)
    1x 1K Pot (audio). (V)
    2x 2x28 pins edge connectors.


***********************************************************************************


    903/904 Serial Communication:


          ACIA 6850                                                  CMOS 4024
         +---------+                                          (7-stage binary control)
    D0<--|22     03|-RXC--+                                         +---------+
    D1<--|21       |      +------------------------------+-[SW1-1]--|03  (128)|
    D2<--|20     04|-TXC--+                              +-[SW1-2]--|04   (64)|
    D3<--|19       |                                     +-[SW1-3]--|05   (32)|
    D4<--|18     02|-RXD--MC7272--+- SERIAL IN (+)       +-[SW1-4]--|06   (16)|
    D5<--|17       |              +- SERIAL IN (-)       +-[SW1-5]--|09    (8)|
    D6<--|16       |                                     +-[SW1-6]--|11    (4)|
    D7<--|15     06|-TXD--MC7272--+- SERIAL OUT (+)                 |         |
         |         |              +- SERIAL OUT (-)            CLK--|19   U30 |
         | U81   01|--CLK                                           +---------+
         +---------+


    CLK = CPU clock (625 kHz)
    Switches 1-1 to 1-6 set the rate from 300 to 9600 bauds.


***********************************************************************************


   ++++ System 903/904/905/906-III official list of games (not complete) ++++

   -----+--------------------------+---------+---------+----------+---------+----------+---------+---------------+------+------+-------
    VER |  NAME                    | SYSTEM  | PROGRAM | CHAR GEN | CG P/N  | CLR PROM | CP P/N  | GAME TYPE     | DUMP | STAT | ADDED
   -----+--------------------------+---------+---------+----------+---------+----------+---------+---------------+------+------+-------
 *  7.4 | Gaming Poker (w. export) |   903   | AUG/81  | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | Yes  | OK   | Yes
    7.6 | Poker                    |   903   | AUG/81  | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
 *  7.9 | Arcade Poker             |   903   | AUG/81  | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | BD   | Yes
 *  8.0 | Arcade Black Jack        |   903   | SEP/81  | CPKCG    | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | BD   | Yes
 *  9.4 | Keno                     |   903   | DEC/81  | KCG      | 6300060 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
   10.2 | Keno                     |   903   | DEC/81  | KCG      | 6300060 | POKCLR   | 6600020 | Amusement     | No   |      | No
 * 10.7 | Big Game Lottery         |   904   | MAR/82  | LOTCG    | 6300050 | LOTCLR   | 6600030 | Gaming        | Yes  | OK   | Yes
 * 11.3 | Black Jack               |   903   |         | CPKCG?   | 6300010 | POKCLR   | 6600020 | Amusement?    | No   |      | No
 * 12.3 | Ticket Poker             |   903   | JUN/82  | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | BD   | Yes
 * 12.5 | Nudge Bingo              |   903   | JUN/82  | NBCG     | 6300040 | BCLR     | 6300040 | Amusement     | Yes  | OK   | Yes
 * 12.7 | Keno                     |   903   | JUL/82  | KCG      | 6300060 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
 * 12.8 | Arcade Game (Draw Poker) |   903   | JUL/82  | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
   13.4 | Nudge Bingo              |   903   | SEP/82  | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | Yes  | OK   | Yes
   14.5 | Pixels                   |   903   | DEC/82  | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | BD   | Yes
   14.7 | Nudge Bingo              |   903   |         | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | No   |      | No
   15.0 | Punchboard               |   903   |         | PBCG     | 6300190 | PBCLR    | 6600130 | Gaming Wash   | No   |      | No
   15.7 | Double-Draw Poker        |   903   | JAN/83  | CPKCG    | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
   15.8 | Poker Montana            |   904   |         | CPKCG    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   15.9 | Wild Double-Up           |   905   | JAN/83  | JKR2CG   | 6300100 | WLDCLR   | 6600010 | Amusement     | Yes  | OK   | Yes
   16.2 | Gaming Lottery           |   904   |         | LCG      | 6300050 | LOTCLR   | 6600030 | Gaming        | No   |      | No
   16.3 | Black Jack               |   903   |         | CPKCG    | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   16.4 | K.I.S.S. Keno            |   904   | MAY/83  | KCG      | 6300060 | PCLR     | 6600020 | Amusement     | Yes  | IC   | Yes
   16.5 | K.I.S.S. Poker           |   903   |         | POKER    | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   16.6 | Nudge Bingo              |   903   |         | NBCG     | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   16.7 | Poker                    |   903   |         | PCG      | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   16.8 | Arcade Keno              |   903   | MAR/83  | KCG      | 6300060 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
 * 16.9 | Keno                     |   904   |         | KCG      |         |          |         | Gaming        | No   |      | No
   17.0 | Amusement Poker          |   905   |         | JKRPKR   | 6300010 | PKRCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
   17.1 | Nudge Bingo              |   903   |         | NBCG     | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   17.2 | Double/Double Poker      |   905   | MAY/83  | JKRPKR   | 6300100 | WLDCLR   | 6600010 | Amusement     | Yes  | OK   | Yes
   17.3 | Casino Poker             |   904   |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   17.4 | Gin Gaming               |   904   |         | GCG0.2   | 6300080 | GINCLR   | 6600060 | Nevada Gaming | No   |      | No
   17.5 | Gaming Draw Poker        |   904   | DEC/84  | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | Yes  | OK   | Yes
   17.6 | Nudge                    |   903   | SEP/82  | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | Yes  | OK   | Yes
   17.7 | Keno                     |   904   |         | KCG      | 6300060 | POKCLR   | 6600020 | Gaming        | No   |      | No
   17.9 | AWP Poker                |   904   |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
 * 18.0 | Gin                      |   905   |         | GCG2     | 6300110 | MLTCLR   | 6600070 | Amusement     | No   |      | No
   18.1 | Nudge                    |    ?    | JUL/83  | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | Yes  | OK   | Yes
   18.2 | Amusement Poker          |   905   |         | JKRCG02  | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
   18.3 | Pixels                   |   905   | AUG/83  | PXCG     | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | OK   | Yes
   18.4 | Double Draw-Lebanon      |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   18.5 | Pixels                   |   905   | AUG/83  | PXCG     | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | OK   | Yes
   18.6 | Pixels                   |   905   | AUG/83  | PXCG     | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | OK   | Yes
   18.7 | Amusement Poker (skill)  |   905   | AUG/83  | JKRTWO   | 6300100 | MLTCLR   | 6600070 | Amusement     | Yes  | BD   | Yes
   18.8 | Poker-Lebanon            |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   18.9 | Pixels (french)          |    ?    |         | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | No   |      | No
   19.0 | Hi-Score Poker           |    ?    |         | JKRTWO   | 6300100 | WLDCLR   | 6600070 | Amusement     | No   |      | No
 * 19.1 | Super Blackjack          |   905   | AUG/83  | JKRTWO?  | 6300100 | MLTCLR   | 6600070 | Amusement     | No   |      | No
   19.2 | AWP Poker                |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   19.3 | Gin                      |   905   |         | GCG2     | 6300110 | MLTCLR   | 6600070 | Amusement     | No   |      | No
   19.4 | Pixels (french)          |    ?    |         | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | No   |      | No
   19.7 | Ind. Wild Double-Up      |    ?    |         | JKRCG    | 6300100 | WLDCLR   | 6600070 | Amusement     | No   |      | No
   19.9 | Double-Up Nudge          |    ?    |         | NUDGE    | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   20.1 | Poker                    |    ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   20.4 | Super Blackjack          |   905   | FEB/84  | JKRTWO   | 6300100 | MLTCLR   | 6600070 | Amusement     | Yes  | OK   | Yes
 * 20.5 | Blackjack?               |   905   | FEB/84  | JKRTWO?  | 6300100 | MLTCLR   | 6600070 | Amusement     | No   |      | No
   20.7 | Double-Up Nudge          |    ?    |         | NUDGECG  | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   20.8 | Winner's Choice          |    ?    | FEB/84  | MLT2CG   | 6300130 | MLTCLR   | 6600070 | Amusement     | Yes  | OK   | Yes
   20.9 | Gaming Poker-Europe      |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   21.1 | AWP Poker                |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   21.2 | Video Slot               |    ?    |         | SL1CG    | 6300180 | SLC1     | 6600120 | Gaming        | No   |      | No
   21.4 | Wild Double-Up Poker     |   905   | MAY/84  | WLDCG    | 6300100 | WLDCLR   | 6600010 | Amusement     | Yes  | IC   | Yes
   21.5 | Double/Double Poker      |    ?    |         | JKRTWO   | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
 * 21.6 | Wild Double-Up?          |   905   | JUN/84  | WLDCG    | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
 * 21.7 | Poker?                   |   905   | MAY/84  | GPKCG?   | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   21.9 | Amusement Poker          |    ?    |         | JKRCG    | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
   22.1 | Poker                    |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   22.7 | Amusement Poker (dbl/dbl)|   905   | JUL/84  | JKRPKR   | 6300100 | WLDCLR   | 6600010 | Amusement     | Yes  | OK   | Yes
   22.8 | Tennesee Wild Poker      |   905   |         | JKR2CG   | 6300100 | MLTCLR   | 6600070 | Amusement     | No   |      | No
   22.9 | Double/Double Poker      |   905   |         | JKRTWO   | 6300100 | MLTCLR   | 6600010 | Amusement     | No   |      | No
   23.0 | FC Bingo (4-card)        |   905   | JUN/84  | FCBCG    | 6300150 | FCBCLR   | 6600100 | Amusement     | Yes  | BD   | Yes
   23.2 | Pixels                   |    ?    |         | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | No   |      | No
 * 23.6 | Hotline                  |   905   | OCT/84  | HLCG     | 6300160 | HLCLR    | 6600110 | Amusement     | Yes  | OK   | Yes
   23.7 | AWP Blackjack            |    ?    |         | POKER    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   23.9 | G.Draw Poker (discard)   |   904   | JAN/85  | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | Yes  | OK   | Yes
   24.0 | Gaming Draw Poker (hold) | 903/904 | MAY/85  | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | Yes  | OK   | Yes
   24.2 | Export Poker             |    ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   24.3 | Gaming Blackjack         |    ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   24.5 | Wild Double-Up           |    ?    |         | JKPKCG   | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
   24.6 | Hotline                  |   905   | JAN/85  | HLCG     | 6300160 | HLCLR    | 6600110 | Amusement     | Yes  | OK   | Yes
   24.8 | Poker (discard)          |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   24.9 | Poker (hold)             |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   25.3 | 4-Card Bingo-Gaming      |    ?    |         | FCB1CG   | 6300150 | FCB1CLR  | 6600100 | Gaming        | No   |      | No
   25.4 | Indian 4-Card Bingo      |    ?    |         | FCBCG    | 6300150 | FCB1CLR  | 6600100 | Amer-Indian   | No   |      | No
   25.5 | Wild Double-Up Poker     |    ?    |         | JKRCG    | 6300100 | WILD     | 6600010 | Amusement     | No   |      | No
   25.6 | Mr. Domino               |    ?    |         | DOMCG    | 6300140 | DOMCLR   | 6600080 | Amusement     | No   |      | No
   25.7 | Mr. Domino               |    ?    |         | DOMCG    | 6300140 | DOMCLR   | 6600080 | Amusement     | No   |      | No
   25.8 | Poker                    |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   26.3 | Wild Double-Up Poker     |    ?    |         | JKRCG    | 6300100 | WILD     | 6600010 | Amusement     | No   |      | No
   26.5 | 4-Reel Slot              |    ?    |         | SLOT4CG  | 6300220 | SL4CLR   | 6600140 | Gaming        | No   |      | No
   26.6 | Poker                    |    ?    |         | GPKCG    | 6300250 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   26.8 | Wild Double-Up Poker     |    ?    |         | JKRCG    | 6300260 | WILD     | 6600010 | Nevada Gaming | No   |      | No
   27.1 | Tourney Gaming Poker     |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   27.2 | Arcade Keno              |    ?    | AUG/85  | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | Yes  | OK   | Yes
 * 27.2 | Arcade Keno              |    ?    | AUG/85  | KJCG     | 6300060 | POKCLR   | 6600020 | Gaming        | Yes  | OK   | Yes
   27.3 | Export Poker             |    ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   27.4 | Keno II                  |    ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | No   |      | No
   27.5 | Keno (Austria)           |    ?    |         | KENOCG   | 6300060 | POKCLR   | 6600020 | Gaming        | No   |      | No
   27.6 | Slot 8                   |    ?    |         | SL8CG    | 6300290 | SLOT4CLR | 6600140 | Gaming        | No   |      | No
   27.9 | 3-Reel Slot, Gaming      |    ?    |         | SL2CG    | 6300280 | SL2CLR   | 6600120 | Nevada Gaming | No   |      | No
   28.0 | 3-Reel Slot              |    ?    |         | SL2CG    | 6300180 | SL2CLR   | 6600120 | Gaming        | No   |      | No
   28.2 | Hotline                  |    ?    |         | HLCG     | 6300160 | HLCLR    | 6600110 | Amusement     | No   |      | No
   28.5 | Hotline                  |    ?    |         | HLCG     | 6300130 | HLCLR    | 6600110 | Amusement     | No   |      | No
   29.0x| Stand or Draw Poker      |    ?    |         | ST2CG    | 6300270 | ST2CLR   | 6600150 | Nevada Gaming | No   |      | No
   30.0x| Casino 21                |    ?    |         | GPKCG    | 6300250 | WILD     | 6600010 | Nevada Gaming | No   |      | No
   31.0x| Poker (discard)          |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   32.0x| Poker (hold)             |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   33.0x| Keno                     |    ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   34.0x| Keno (Austria)           |    ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Gaming        | No   |      | No
   35.0x| 1-3 Sign Poker (discard) |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   36.0x| 1-3 Sign Poker (hold)    |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   37.0x| Amusement Keno           |    ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | No   |      | No
   39.0x| 3-Reel Slot              |    ?    |         | SBARCG   | 6300305 | JKRSLOT  | 6600170 | Nevada Gaming | No   |      | No
   39.0x| 3-Reel Slot              |    ?    |         | PLAIN7CG | 6300309 | JKRSLOT  | 6600170 | Nevada Gaming | No   |      | No
   39.0x| 3-Reel Slot              |    ?    |         | JWCG     | 6300313 | JKRSLOT  | 6600170 | Nevada Gaming | No   |      | No
   40.0x| Poker w/Wild Card        |   903   |         | JKRCG    | 6300260 | WLDCLR   | 6600010 | Nevada Gaming | No   |      | No
   41.0x| Do-it-yourself Bingo     |    ?    |         | KCG      | 6300060 | PKRCLR   | 6600020 | Amer-Indian   | No   |      | No
   42.0x| Mr. Domino               |    ?    |         | NDMCG    | 6300300 | DOMCLR   | 6600080 | Amusement     | No   |      | No
   43.0x| Export Poker             |    ?    |         | GPKCG    | 6300170 | POKCLR   | 6600020 | Gaming        | No   |      | No
   44.0x| Tourney Poker (discard)  |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   45.0x| Tourney Poker (hold)     |    ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   46.0x| Gaming Poker             |    ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   47.0x| 4-Card Indian Bingo      |    ?    |         | FCB7CG   | 6300150 | FCB7CLR  | 6600100 | Amer-Indian   | No   |      | No
   48.0x| Keno                     |    ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | No   |      | No
        |                          |         |         |          |         |          |         |               |      |      |
   51.07| Poker (906-III)          | 906-III |         | GP2CG    | unknown | WILD     | unknown | Nevada Gaming | Yes  | OK   | Yes
   51.08| Poker (906-III)          | 906-III |         | GP2CG    | unknown | WILD     | unknown | Nevada Gaming | Yes  | OK   | Yes
 * 60.04| Poker (906-III)          | 906-III |         | GP2CG    | unknown | WILD     | unknown | Nevada Gaming | Yes  | OK   | Yes
        |                          |         |         |          |         |          |         |               |      |      |
 * 903d | System 903 Diag.PROM     |   903   | unknown | any      | unknown | any      | unknown | Testing H/W   | Yes  | OK   | Yes
 * 905d | System 905 Diag.PROM     |   905   | unknown | any      | unknown | any      | unknown | Testing H/W   | Yes  | OK   | Yes


   (*) Not in the original list.



   ++++ Unofficial games ++++

   -------------------------------------+----------------------+------+-----------+-----------+------+------+-------
    NAME                                | COMPANY              | YEAR | GAME TYPE |  SYSTEM   | DUMP | STAT | ADDED
   -------------------------------------+----------------------+------+-----------+-----------+------+------+-------
    El Grande - 5 Card Draw (New)       | Tuni Electro Service | 1982 | Amusement | 903 based | Yes  | BR   | Yes
    Jackpot Joker Poker (set 1)         | Enter-Tech (ETL)     | 1983 | Amusement | 903 based | Yes  | OK   | Yes
    Jackpot Joker Poker (set 2)         | Enter-Tech (ETL)     | 1983 | Amusement | 903 based | Yes  | OK   | Yes
    SSI Poker (v2.4)                    | SSI                  | 1988 | Amusement | 903 based | Yes  | IC   | Yes
    SSI Poker (v3.0)                    | SSI                  | 1988 | Amusement | 903 based | Yes  | IC   | Yes
    SSI Poker (v4.0)                    | SSI                  | 1990 | Amusement | 903 based | Yes  | IC   | Yes
    Casino 21 UCMC/IWC (ver 30.08)      | UCMC/IWC             | 1990 | Gambling  |  906-III  | Yes  | OK   | Yes
    Poker UCMC/IWC (ver 162.03)         | UCMC/IWC             | 1991 | Gambling  |  906-III  | Yes  | OK   | Yes
    Poker UCMC/IWC (ver 162.03 20-6-91) | UCMC/IWC             | 1991 | Gambling  |  906-III  | Yes  | OK   | Yes


    STAT: OK = Good dump.
          BD = At least 1 ROM is bad dumped.
          BR = At least 1 ROM is bit-rotten.
          IC = Incomplete set (at least 1 ROM is missing)

    (for more information see the ROM load)



***********************************************************************************


    --- General Notes ---


    Gaming Draw Poker:

    Gaming Draw Poker, formerly known as Jack Potten's poker, is an evolution of the
    mentioned game. It was created by Cal Omega in 1981 for Casino Electronics Inc.
    Cal Omega was bought out by CEI (Casino Electronics Inc.), and CEI was bought by UCMC.

    This is the most famous game from Cal Omega 903 System. In the test mode you can see
    the name "Gaming Draw Poker", but in the manual figure as "Casino Poker". Also in some
    game lists you can see it with another name: "Arcade Poker".

    The game uses the same GFX set that Jack Potten's Poker for cards and has similar
    layout, but the game is different and the old discrete pitched sounds were replaced
    with a better set of sounds through a AY8912 implementation. The empty socket in
    the PCB is for future upgrades instead of sound ROM, since sounds are hardcoded.

    Inputs are multiplexed and selected through PIA1, portB.


    Some instructions....

    * Some versions have a STAND button to conserve all cards without discards.

    * To enter to the Input Test Mode press TEST MODE (key 9).
      Press DISCARD1 (key Z) + DISCARD2 (key X) + DISCARD3 (key C) to exit.

    * To see the last hand played and stats, press DISPUTE (key 0).
      Press DEAL/DRAW button to exit.

    * Depending of the game version, TEST MODE & DISPUTE buttons only work if the door
      was previously opened. To do that, you must keep pressed DOOR OPEN (key D).

    * To pass coin and hopper errors press DOOR OPEN (key D). If the hopper is empty,
      keep pressed DOOR OPEN and press HAND PAY (key O)

    * For payout, press COLLECT (key I) and then PAYOUT for each credit (manual mode).



    El Grande - 5 Card Draw:

    This game was created by Tuni Electro Service (later known as Enter-Tech) and was
    published by E.T. Marketing, Inc., which is basically the marketing branch of
    E.T. Industries (parent company of Tuni/Enter-Tech).
    This is the new version. The old one is still undumped, but looks like Golden Poker D-Up.

    Flyer: https://flyers.arcade-museum.com/?page=flyer&db=videodb&id=4542&image=1

    The AY8912 is mapped to 0x840/0x841 but the game lacks of credits and bet sounds.
    The rest of sounds are ok. Maybe the missing ones are generated by discrete circuitry.

    Inputs are multiplexed and selected through PIA1, portB.

    There aren't meter and stats modes. Only for amusement, so... no payout.
    To clear credits press F2.
    To enter to TEST MODE press F2 twice. Press HOLD 1 + HOLD 2 + HOLD 3 to exit.



***********************************************************************************?


    *** Memory Maps ***


    --- System 903/904 ---

    $0000 - $07FF   NVRAM           ; All registers and settings.
    $0840 - $0840   AY-8912         ; Read/Control.
    $0841 - $0841   AY-8912         ; Write.
    $0880 - $0880   CRTC6845        ; MC6845 addressing.
    $0881 - $0881   CRTC6845        ; MC6845 Read/Write.
    $08C4 - $08C7   PIA0            ; I/O Ports 0 to 3 (multiplexed).
    $08C8 - $08CB   PIA1            ; I/O Port 4.
    $08D0 - $08D1   ACIA 6850       ; Serial port.

    $1000 - $13FF   VideoRAM
    $1400 - $17FF   ColorRAM

    $1800 - $3FFF   ROM space       ; Systems 903/904.

    The whole map is mirrored to $4000-$7FFF, $8000-$BFFF and $C000-$FFFF.


    --- System 905 ---

    $0000 - $07FF   NVRAM           ; All registers and settings.
    $1040 - $1040   AY-8912         ; Read/Control.
    $1041 - $1041   AY-8912         ; Write.
    $1080 - $1080   CRTC6845        ; MC6845 addressing.
    $1081 - $1081   CRTC6845        ; MC6845 Read/Write.
    $10C4 - $10C7   PIA0            ; I/O Ports 0 to 3 (multiplexed).
    $10C8 - $10CB   PIA1            ; I/O Port 4.

    $2000 - $23FF   VideoRAM
    $2400 - $27FF   ColorRAM

    $3000 - $7FFF   ROM space.      ; System 905.

    The whole map is mirrored to $8000-$FFFF.


    --- System 906-III ---

    $0000 - $1FFF   NVRAM           ; All registers and settings.
    $280C - $280F   PIAT0           ; I/O functions.
    $2810 - $2810   ACIA6850        ; ACIA read.
    $2811 - $2811   ACIA6850        ; ACIA write.
    $2824 - $2827   PIAT1           ; I/O functions.
    $2C04 - $2C04   CRTC6845        ; MC6845 addressing.
    $2C05 - $2C05   CRTC6845        ; MC6845 read/write.
    $2C08 - $2C08   AY-8912         ; Read/control.
    $2C09 - $2C09   AY-8912         ; Write.

    $2000 - $23FF   VideoRAM
    $2400 - $27FF   ColorRAM

    $6000 - $FFFF   ROM space.      ; System 906-III.

    No mirrors... Using the whole CPU addressing.



    *** MC6545 Initialization (60Hz) ***

    ----------------------------------------------------------------------------------------------------------------------
    register:  R00   R01   R02   R03   R04   R05   R06   R07   R08   R09   R10   R11   R12   R13   R14   R15   R16   R17
    ----------------------------------------------------------------------------------------------------------------------
    value:     0x27  0x20  0x23  0x03  0x1F  0x04  0x1F  0x1F  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.


    *** MC6545 conditional 50Hz change for elgrande if 0x8c4 (PIA0) has bit7 deactivated ***

    ---------------------------------------------------------
    register:  R00   R01   R02   R03   R04   R05   R06   R07
    ---------------------------------------------------------
    value:     0x27  0x20  0x23  0x03  0x26  0x00  0x20  0x22



***********************************************************************************


    DRIVER UPDATES
    --------------


    [2021-05]

    Major improvements...
    - Worked the analogic color system, adding the three R-G-B presets.
    - Added the PORT_ADJUST for RGB in all games, defining the default values.
    - Redumped the Cal Omega - Game 20.8 (Winner's Choice). Reworked proper inputs.
    - Inputs from the scratch for all the Pixels, Keno and Hotline games. Promoted to WORKING.
    - Specific inputs for games 12.8, 15.7, 15.9, 17.2, and 20.4. Promoted to WORKING.
    - Fixed Pixels games colors.
    - Connected System 903/904 PIA #0 IRQB line to CPU IRQ.
    - Set a handler for System 903/904 PIA #0 CB1 line, as IRQ ack.
    - Added external keyboard controller (i8035 + i8251) for Keno Games, system 903.
    - Added virtual clickable keyboard layout to keno games, critical to get them working.
    - Added hopper support to Cal Omega - Game 7.4 (Gaming Poker, W.Export).
    - Added the following System 903 counters: coin in, coin out, and optional (per game).
    - Fixed inputs on Cal Omega - Game 8.0 (Arcade Black Jack).
    - Fixed inputs on Cal Omega - Game 12.5 (Bingo).
    - Inputs for Cal Omega - Game 13.4 (Nudge). Promoted to WORKING.
    - Inputs for Cal Omega - Game 17.6 (Nudge). Promoted to WORKING.
    - Inputs for Cal Omega - Game 18.1 (Nudge). Promoted to WORKING.
    - Added new set: CEI 51.07 (CEI 906-III Poker).
    - Fixed/added technical notes.


    [2021-04-26]

    General improvements...
    - Added support for extended graphics.
    - Fixed cardbacks for 906-III games.
    - Fixed Nudge Bingo graphics.
    - Fixed rotten/bad bits from the program ROM with educated guess
       to make Cal Omega - Game 7.9 (Arcade Poker) playable.
    - Fixed inputs on Cal Omega - Game 7.6 (Arcade Poker). Now it's playable.
    - Created proper inputs from the scratch for Cal Omega - Game 12.5 (Bingo), and promoted to WORKING.
    - Added hopper to the 906-III games.
    - Fixed docs / technical notes.


    [2021-04-24]

    General improvements...
    - The PIAs set for each MACHINE_CONFIG were made independent.
    - All the PIAs lines were adapted according to the needs of each platform.
    - All the interrupts were set/placed as indicated in the schematics (903/905).
    - A global hopper was added, currently assigned to platform 906-III, but it can be used by other platforms.
    - Worked proper inputs for system 903: Cal Omega - Game 15.7 (Double-Draw Poker). The game was promoted to WORKING.
    - Worked proper inputs for system 905: Cal Omega - Game 15.9 (Wild Double-Up). The game was promoted to WORKING.
    - Worked proper inputs for system 905: Cal Omega - Game 17.2 (Double Double Poker). The game was promoted to WORKING.


    [2021-04-21]

    906-III improvements...
    - Proper inputs for the IWC poker games.
    - Fixed/Improved the interrupts system.
    - Preliminary button-lamps support.
    - Simplified and partially cleaned the code.
    - Added technical notes.


    [2021-04-20]

    - Reworked a lot of things to get the 906-III platform playable.
    - Switched to Rockwell R65C02 CPU.
    - Added missing 906-III inputs, timers, clocks and connections.
    - Renamed Game 51.08 to CEI 51.08 (CEI 906-III Poker).
       The game is promoted to WORKING.
    - Added Casino 21 UCMC/IWC (ver 30.08). The game is working.
    - Added Poker UCMC/IWC (ver 162.03). The game is working.
    - Added Poker UCMC/IWC (ver 162.03 20-6-91). The game is working.


    [2010-08-20]

    - Added Game 24.0 (Gaming Draw Poker, hold). The game is playable.
    - Added missing 50/60Hz output frequency selector to systems 903-905.
       However, I dunno how it looks physically (switch/jumper/bridge?)
    - Injected missing start and NMI vectors to comg080 (Arcade Black Jack),
       from an exhaustive program ROM analysis / reverse engineering.
       The game now starts ok and is triggering proper NMI's.
    - Inputs from the scratch for comg080 (Arcade Black Jack).
    - Added workaround to pass the vector-check protection.
    - Reworked the lamps circuitry by system-basis.
    - Updated games list and technical notes.
    - Promoted comg080 (Arcade Black Jack) to working state.

    - Added support for system 906-III:
        - Accurate memory map, CRTC, and PIAs mapped.
        - Preliminary PIAs support (no multiplexion yet).
        - Added proper machine driver, with correct R65C02 CPU.
        - Documented the hardware specs.
        - Added Game 51.08 (CEI Video Poker, Jacks or Better),
          running in CEI 906-III hardware. The game is not working.
        - Corrected docs about the 906-III memory map.
        - Mapped the AY8912.
        - Added AY8912 proper interfase. Tied SW2 to AY8912 port.
        - PIA0, portA is polled constantly. Tied some debug handlers
           to understand how the input system works.
        - Added notes about the PIAs R/W.


    [2009-09-03]

    - Added Game 7.4 (Gaming Poker, W.Export).
    - Added Game 7.4 proper inputs. Game is now playable with full sound.
    - Updated games list and technical notes.


    [2008-12-26]

    - Correctly setup the MC6845 device for all systems.
    - Added common MC6845 device interface.


    [2008-07-01]

    - Unified MACHINE_RESET for systems 903/904/905.
    - Created a new handler to manage the first 4 bits of system 905's PIA1 portB as input mux selector.
      (905 is still not working)
    - Updated technical notes.


    [2008-06-23]

    - Lots of improvements on the input system.
    - Adjusted the CPU addressing to 14 bits for systems 903/904.
    - Adjusted the CPU addressing to 15 bits for system 905.
    - Rewrote all the ROM loads based on these changes.
    - Defined CPU, UART and sound clocks.
    - Splitted the sound interface to cover different systems.
    - Splitted the PIAs interfaces to cover different systems.
    - Added sound to system 905 games.
    - Added MACHINE_START and MACHINE_RESET to initialize the devices properly.
    - Documented the systems 903/904 UART connections.
    - Traced and added PIAs connection diagrams for systems 903/904/905.
    - Hooked the missing DIP Switches bank to the AY8912 for system 903.
    - Added preliminary serial/UART support to systems 903/904.
    - Hooked the missing DIP Switches bank to the ACIA 6850 for systems 903/904.
      It allow to choose the proper transfer rate for UART.
    - Fixed graphics ROM loads for different Hotline versions.
    - Added support to Game 8.0 (Arcade Black Jack).
    - Added support to Game 12.3 (Ticket Poker).
    - Added support to Game 23.0 (FC Bingo (4-card)).
    - Renamed all roms of each set according to the systems schematics and official list.
    - Cleaned-up the sets.
    - Cleaned-up the driver.
    - Updated technical notes.


    [2008-06-11]

    - Improved the input system for some games.
    - Fixed some wrong color codes.
    - Updated technical notes.


    [2008-06-09]

    - Cleaned up the driver following the MAME Coding Conventions.
    - Added support to Game 18.7 (Amusement Poker).
    - Updated technical notes.


    [2008-02-26]

    - Renamed the driver to calomega.c.
    - Extended the driver to support all the other Cal Omega systems.
    - Switched to XTAL def.
    - Fixed CPU & sound clocks based on official diagrams.
    - Replaced the AY-8910 with AY-8912 to match the real hardware.
    - Unified some memory maps.
    - Splitted machine in sys903 and sys905.
    - Fixed memory map offsets for sys905.
    - Added support to the following games:

        Game 7.6 (Arcade Poker)
        Game 7.9 (Arcade Poker)
        Game 9.4 (Keno)
        Game 10.7 (Big Game)
        Game 12.5 (Bingo)
        Game 12.7 (Keno)
        Game 12.8 (Arcade Game)
        Game 13.4 (Nudge Bingo)
        Game 14.5 (Pixels)
        Game 15.7 (Double-Draw Poker)
        Game 15.9 (Wild Double-Up)
        Game 16.4 (Keno)
        Game 16.8 (Keno)
        Game 17.2 (Double Double Poker)
        Game 17.51 (Gaming Draw Poker)
        Game 17.6 (Nudge Bingo)
        Game 18.1 (Nudge Bingo)
        Game 18.3 (Pixels)
        Game 18.5 (Pixels)
        Game 18.6 (Pixels)
        Game 20.4 (Super Blackjack)
        Game 20.8 (Winner's Choice)
        Game 22.7 (Amusement Poker, d/d)
        Game 23.6 (Hotline)
        Game 23.9 (Gaming Draw Poker) --> EX gdrawpkr.
        Game 24.6 (Hotline)
        Game 27.2 (Keno, amusement)
        Game 27.2 (Keno, gaming)
        System 903 Diag.PROM
        System 905 Diag.PROM
        Jackpot Joker Poker (set 1)
        Jackpot Joker Poker (set 2)
        SSI Poker (v2.4)
        SSI Poker (v3.0)
        SSI Poker (v4.0)

    - Added a list of official/unofficial games, and their states.
    - Fixed a bug introduced when extended the color PROM region to eliminate colortable lenght.
    - Other changes/fixes.
    - Updated technical notes.


    [2007-09-15]

    - Added sound support to "El Grande - 5 Card Draw".
    - All crystals documented via #defines.
    - All CPU and sound clocks derived from #defined crystal values.
    - Splitted the driver to driver/video.
    - Some minor input cleanups.
    - Updated technical notes.


    [2007-08-13]

    - Added "El Grande - 5 Card Draw" (new).
    - Constructed a new memory map for this game.
    - Reworked a whole set of inputs for this game.
    - Patched some bad bits in GFX rom d1.u68 till a good dump appear.
    - Updated technical notes.


    [2007-07-23]

    - Cleaned up the inputs.
    - Changed the hold buttons to "discard" since they are in fact discard buttons.
    - Updated technical notes.


    [2007-03-24 to 2007-04-27]

    - Rearranged GFX in two different banks.
    - Decoded GFX properly.
    - Rewrote the memory map based on program ROMs analysis.
    - Hooked two SY6520/6280 (M6821) PIAs for I/O.
    - Hooked the SY6545 (6845) CRT controller.
    - Fixed size for screen total and visible area based on SY6545 CRTC registers.
    - Added partial inputs through PIAs.
    - Added proper sound through AY8910 (mapped at $0840-$0841).
    - Fixed AY8910 volume to avoid clips.
    - Proper colors through color PROM decode.
    - Demuxed inputs (thanks to Dox that pointed me in the right direction!)
    - Added some game-protection workaround.
    - Added NVRAM support.
    - Renamed driver, set and description to match the real game.
    - Added technical notes.


    TODO:

    - Improve 903/905 inputs.
    - Interrupts for System 903/904 UART.
    - Complete the PIAs connection.
    - 906-III: Proper PIATs 65C24 devices.
    - 906-III: GFX banks, proper banks selector.
    - 906-III: EX and Turbo+ platforms support.
    - Hopper support.
    - Parent/clone relationship.
    - Fix lamps.


***********************************************************************************/

#include "emu.h"
#include "calomega.h"

#include "kenokb.lh"

#define MASTER_CLOCK        XTAL(10'000'000)
#define CPU_CLOCK           (MASTER_CLOCK/16)
#define UART_CLOCK          (MASTER_CLOCK/16)
#define SND_CLOCK           (MASTER_CLOCK/8)


/**************************************************
*               Read/Write Handlers               *
**************************************************/

void calomega_state::update_aciabaud_scale(int state)
{
	uint8_t dsw2 = m_sw2->read();

	m_aciabaud->set_clock_scale((double)dsw2 / 128);
}

uint8_t calomega_state::s903_mux_port_r()
{
	switch( m_s903_mux_data & 0xf0 )    // bits 4-7
	{
		case 0x10: return m_in0_0->read();
		case 0x20: return m_in0_1->read();
		case 0x40: return m_in0_2->read();
		case 0x80: return m_in0_3->read();
	}

	return 0xff;
}

void calomega_state::s903_mux_w(uint8_t data)
{
	// bit 7 - bit 6 -bit 5 -bit 4 -> Mux. Scan Lines
	m_s903_mux_data = data ^ 0xf0;    // inverted

	// bit 0 - bit 1 -> Hopper
	m_hopper->motor_w(BIT(~data, 0) && BIT(~data, 1));

	// bit2 - bit3 -> No connected
}

uint8_t calomega_state::s905_mux_port_r()
{
	switch( m_s905_mux_data & 0x0f )    // bits 0-3
	{
		case 0x01: return m_in0_0->read();
		case 0x02: return m_in0_1->read();
		case 0x04: return m_in0_2->read();
		case 0x08: return m_in0_3->read();
	}
	return 0xff;
}

void calomega_state::s905_mux_w(uint8_t data)
{
//  Lower nibble
	m_s905_mux_data = data ^ 0x0f;    // inverted

/*  Upper nibble
    PIA_1_PB_4 - Hopper1.
    PIA_1_PB_5 - Hopper2.
    PIA_1_PB_6 - Not Connected.
    PIA_1_PB_7 - Lockout.
*/
	m_hopper->motor_w(BIT(~data, 4) && BIT(~data, 5));
	m_lockout = BIT(data, 7);
}


/********* 906-III PIAs debug *********/

uint8_t calomega_state::pia0_bin_r()
{
	//logerror("PIA0: Port B in\n");
	return 0xff;
}

void calomega_state::pia0_aout_w(uint8_t data)
{
	//logerror("PIA0: Port A out: %02X\n", data);
}

void calomega_state::pia0_bout_w(uint8_t data)
{
	// Meters???...M1-M5
	m_diverter = BIT(data, 3);
	m_lockout = BIT(data, 5);
	m_hopper->motor_w(BIT(~data, 6) && BIT(~data, 7));
}

uint8_t calomega_state::pia1_ain_r()
{
	switch( m_s905_mux_data & 0x03 )    // bits 0-3
	{
		case 0x00: return m_in0_0->read();
		case 0x01: return m_in0_1->read();
		case 0x02: return m_in0_2->read();
		case 0x03: return m_in0_3->read();
	}

	return 0xff;
}

uint8_t calomega_state::pia1_bin_r()
{
	// logerror("PIA1: Port B in\n");
	return 0xff;
}

void calomega_state::pia1_aout_w(uint8_t data)
{
	m_s905_mux_data = data >> 6;
}

void calomega_state::pia1_bout_w(uint8_t data)
{
/* System 906-III Lamps...

  7654 3210
  ---- ---x  Hold 1 lamp.
  ---- --x-  Hold 2 lamp.
  ---- -x--  Hold 3 lamp.
  ---- x---  Hold 4 lamp.
  ---x ----  Hold 5 lamp.
  --x- ----  (unknown)
  -x-- ----  (unknown)
  x--- ----  (unknown)

*/
	m_lamps[0] = BIT(~data, 0);  // L1 (Hold 1)
	m_lamps[1] = BIT(~data, 1);  // L2 (Hold 2)
	m_lamps[2] = BIT(~data, 2);  // L3 (Hold 3)
	m_lamps[3] = BIT(~data, 3);  // L4 (Hold 4)
	m_lamps[4] = BIT(~data, 4);  // L5 (Hold 5)
	m_lamps[5] = BIT(~data, 5);  // L6 (unknown)
	m_lamps[6] = BIT(~data, 6);  // L7 (unknown)
	m_lamps[7] = BIT(~data, 7);  // L8 (unknown)

	// logerror("PIA1: Port B out: %02X\n", data);
}

TIMER_DEVICE_CALLBACK_MEMBER( calomega_state::timer_0 )
{
	m_timer = !m_timer;
	m_pia[0]->ca1_w(m_timer);
	// logerror("Timer_0 event : state=%d\n", m_timer);
}

TIMER_DEVICE_CALLBACK_MEMBER( calomega_state::timer_1 )
{
	m_timer = !m_timer;
	m_pia[1]->cb1_w(m_timer);
	// logerror("Timer_1 event (TIMER) : state=%d\n", m_timer);
}

TIMER_DEVICE_CALLBACK_MEMBER( calomega_state::timer_2 )
{
	m_timer =! m_timer;
	m_pia[1]->ca1_w(m_timer);
	m_pia[1]->read(1);  // CRA
	// logerror("Timer_2 event : state=%d\n", m_timer);
}

void calomega_state::vblank0_w(int state)
{
	m_pia[0]->read(0);
	m_pia[0]->read(0);
	m_pia[1]->cb1_w(state);
	// m_pia[1]->read(3);  // CRB
	// logerror("V_BLANK_0 event : state=%d\n", state);
}

void calomega_state::vblank1_w(int state)
{
	m_pia[0]->read(2);
	m_pia[0]->cb1_w(state);
	// logerror("V_BLANK_1 event (sale ): state=%d\n", state);
}

void calomega_state::vblank2_w(int state)
{
	m_pia[1]->read(0);
	m_pia[1]->read(0);
	m_pia[1]->cb1_w(state);

	// logerror("V_BLANK_2 event : state=%d\n", state);
}

void calomega_state::pia1_cb2_w(int state)
{
	// Output to L9
	m_lamps[8] = ~state;  // L9 (Door?)
	// logerror("PIA1: CB2: %02X\n", state);
}

// Dummy callbacks
uint8_t calomega_state::dummy_pia_r()
{
	return 0xff;
}

void calomega_state::dummy_pia_w(uint8_t data)
{
	m_pia_data = data;
}

void calomega_state::dummy_pia_line_w(int state)
{
}



/********  Lamps debug  ********

    PIA0-B  PIA1-A

    0xff    0x7f    = OFF
    0xfd    0x7f    = Deal
    0xf1    0x7b    = 12345 Holds
    0xef    0x7f    = Double Up & Take
    0xff    0x7e    = Big & Small

    0xfe    0x7f    = Hold 1
    0xfd    0x7f    = Hold 2
    0xfb    0x7f    = Hold 3
    0xf7    0x7f    = Hold 4
    0xef    0x7f    = Hold 5
    0xff    0x7e    = Cancel
    0xff    0x7d    = Bet
    0xff    0x7b    = Take

*/
void calomega_state::lamps_903a_w(uint8_t data)
{
	// Whole 8 bits of PIA0 port B
	m_lamps[0] = BIT(~data, 0);  // L1 (Hold 1)
	m_lamps[1] = BIT(~data, 1);  // L2 (Hold 2)
	m_lamps[2] = BIT(~data, 2);  // L3 (Hold 3)
	m_lamps[3] = BIT(~data, 3);  // L4 (Hold 4)
	m_lamps[4] = BIT(~data, 4);  // L5 (Hold 5)
	machine().bookkeeping().coin_counter_w(0, BIT(~data, 5)); // M1 - Coins in.
	machine().bookkeeping().coin_counter_w(1, BIT(~data, 6)); // M2 - Coins Out( 1 x 10 coins).
	machine().bookkeeping().coin_counter_w(2, BIT(~data, 7)); // M3 - Optional (Per game).
}

void calomega_state::lamps_903b_w(uint8_t data)
{
	// First 4 bits of PIA1 port A
	m_lamps[5] = BIT(~data, 0);  // L6 (Cancel)
	m_lamps[6] = BIT(~data, 1);  // L7 (Bet)
	m_lamps[7] = BIT(~data, 2);  // L8 (Take)
	m_lamps[8] = BIT(~data, 3);  // L9 (Door?)
}

void calomega_state::lamps_905_w(uint8_t data)
{
	// Whole 8 bits of PIA0 port B
	m_lamps[0] = BIT(~data, 0);  // L1 (Hold 1)
	m_lamps[1] = BIT(~data, 1);  // L2 (Hold 2)
	m_lamps[2] = BIT(~data, 2);  // L3 (Hold 3)
	m_lamps[3] = BIT(~data, 3);  // L4 (Hold 4)
	m_lamps[4] = BIT(~data, 4);  // L5 (Hold 5)
	m_lamps[5] = BIT(~data, 5);  // L6 (unknown)
	m_lamps[6] = BIT(~data, 6);  // L7 (unknown)
	m_lamps[7] = BIT(~data, 7);  // L8 (unknown)
}

uint8_t calomega_state::keyb_903_r()
{
	if(m_kbscan == 0x0d ) m_uart->write_cts(0);
	if(m_kbscan == 0x0e ) m_uart->write_cts(1);
	m_pia[1]->read(2); // TODO: Troubleshooting with vblank irq flag stuck
	return m_key_row[m_kbscan]->read();
}


/*************************************************
*             Memory map information             *
*************************************************/

void calomega_state::sys903_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0840, 0x0841).w("ay8912", FUNC(ay8910_device::address_data_w));
	map(0x0880, 0x0880).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0881, 0x0881).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x08c4, 0x08c7).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x08c8, 0x08cb).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x08d0, 0x08d1).rw("acia6850_0", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(calomega_state::calomega_videoram_w)).share("videoram");
	map(0x1400, 0x17ff).ram().w(FUNC(calomega_state::calomega_colorram_w)).share("colorram");
	map(0x1800, 0x3fff).rom();
}

void calomega_state::s903mod_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0840, 0x0841).w("ay8912", FUNC(ay8910_device::address_data_w));
	map(0x0880, 0x0880).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0881, 0x0881).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x08c4, 0x08c7).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x08c8, 0x08cb).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(calomega_state::calomega_videoram_w)).share("videoram");
	map(0x1400, 0x17ff).ram().w(FUNC(calomega_state::calomega_colorram_w)).share("colorram");
	map(0x1800, 0x3fff).rom();
}

void calomega_state::sys905_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x1040, 0x1041).w("ay8912", FUNC(ay8910_device::address_data_w));
	map(0x1080, 0x1080).w("crtc", FUNC(mc6845_device::address_w));
	map(0x1081, 0x1081).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x10c4, 0x10c7).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x10c8, 0x10cb).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x23ff).ram().w(FUNC(calomega_state::calomega_videoram_w)).share("videoram");
	map(0x2400, 0x27ff).ram().w(FUNC(calomega_state::calomega_colorram_w)).share("colorram");
	map(0x2800, 0x7fff).rom();
}

void calomega_state::sys906_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x280c, 0x280f).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2810, 0x2811).rw("acia6850_0", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x2824, 0x2827).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2c04, 0x2c04).w("crtc", FUNC(mc6845_device::address_w));
	map(0x2c05, 0x2c05).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2c08, 0x2c09).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x2000, 0x23ff).ram().w(FUNC(calomega_state::calomega_videoram_w)).share("videoram");
	map(0x2400, 0x27ff).ram().w(FUNC(calomega_state::calomega_colorram_w)).share("colorram");
	map(0x6000, 0xffff).rom();
}

void calomega_state::kstec_mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
}

void calomega_state::kstec_io_map(address_map &map)
{
	map(0x00, 0x00).rw(m_uart, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x01, 0x01).rw(m_uart, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
}

/*************************************************
*                  Input ports                   *
*************************************************/

static INPUT_PORTS_START( stand903 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")   // baud (serial 6850-4024), SW1 in schematics...
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( keno_903 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)         // "5"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode")  // "F2"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_8)  // "8"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")   // baud (serial 6850-4024), SW1 in schematics...
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  Keyboard matrix

	PORT_START("KB_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                // #81 0X51 BET   "M"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )               // #82 0X52 START "2"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_NAME("30")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_NAME("40")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_NAME("39")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("20")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("19")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("29")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("28")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("38")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("10")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("09")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("18")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("17")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("27")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("37")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("08")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("07")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("06")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("16")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("26")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("36")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("03")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("04")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("05")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("15")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("25")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("35")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("02")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("13")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("14")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("24")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("34")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("01")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("12")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("22")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("23")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("33")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("21")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("31")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("32")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("72")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("73")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("74")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("75")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("65")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("55")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("45")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("71")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("61")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("62")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("63")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("64")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("54")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("44")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("51")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("41")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("52")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("53")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("42")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("43")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Exit") PORT_CODE(KEYCODE_X)  // #84 0x54 TEST EXIT
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                                  // #83 0x53 CANCEL "N"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("60")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("50")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("49")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("70")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("69")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("59")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("58")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("48")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("80")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("79")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("68")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("67")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("57")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("47")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KB_14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("78")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("77")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("76")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("66")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("56")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("46")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( stand905 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// For System 905, SW1 uses the whole PIA1 portA.

	PORT_START("SW1")    // settings (PIA1)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( gdrwpkrd )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)    // credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )       // connected to SW2-5.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )  PORT_NAME("Door Open")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")                      // in some games you need to open the door first
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hand Pay") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Dispute")  PORT_CODE(KEYCODE_0)  // in some games you need to open the door first
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("WT.Switch") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x30, 0x20, "Maximum Credits")    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x10, "400" )
	PORT_DIPSETTING(    0x20, "800" )
	PORT_DIPSETTING(    0x30, "1600" )
	PORT_DIPNAME( 0xC0, 0x40, "Maximum Bet")        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0xC0, "80" )

	PORT_START("SW2")   // baud (serial 6850-4024), SW1 in schematics
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( gdrwpkrh )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Holds")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )  PORT_NAME("Door Open")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")                       // in some games you need to open the door first
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hand Pay")  PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Dispute")   PORT_CODE(KEYCODE_0)  // in some games you need to open the door first
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Hold 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Hold 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Hold 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Hold 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("WT.Switch") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")   // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x30, 0x20, "Maximum Credits")    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x10, "400" )
	PORT_DIPSETTING(    0x20, "800" )
	PORT_DIPSETTING(    0x30, "1600" )
	PORT_DIPNAME( 0xC0, 0x40, "Maximum Bet")        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0xC0, "80" )

	PORT_START("SW2")    // baud (serial 6850-4024), SW1 in schematics
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")    // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")    // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg208 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Learn")              PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Service/Clear")      PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Data / Audit")       PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Play (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Tilt (Clr.Credits)") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// For System 905, SW1 uses the whole PIA1 portA.

	PORT_START("SW1")    // settings (PIA1)
	PORT_DIPNAME( 0x01, 0x01, "Black Jack" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Craps" )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Red Dog" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Dup" )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Pay Table / Win Pair / BJ Wins" )  PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "250-50-20-8-6-4-3-2-1 / Jacks / Dealer Keeps Tier" )
	PORT_DIPSETTING(    0x10, "250-50-25-10-8-5-3-2-1 / Aces" )
	PORT_DIPSETTING(    0x20, "200-40-10-8-5-4-3-2-1 / Queens/ 2 to 1" )
	PORT_DIPSETTING(    0x30, "200-40-10-8-5-4-3-2-1 / Kings / 3 to 1" )
	PORT_DIPNAME( 0xc0, 0xc0, "Lockout" )      PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "No Lockout" )
	PORT_DIPSETTING(    0x80, "No Lockout" )
	PORT_DIPSETTING(    0x40, "1 Coin Lockout" )
	PORT_DIPSETTING(    0x00, "2 Coin Lockout" )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 25, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 25, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 25, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( arcadebj )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Bet/Play")    PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START )        PORT_NAME("Start")       PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )  PORT_NAME("Double Down") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Hit")         PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stand")       PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Service/Clear") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xC0, 0x80, "Maximum Bet")        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0xC0, "50" )

	PORT_START("SW2")    // baud (serial 6850-4024), SW1 in schematics
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg074 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)    // credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )        PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )  PORT_NAME("Door Open")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")  // in some games you need to open the door first
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xC0, 0x80, "Maximum Bet")        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0xC0, "80" )

	PORT_START("SW2")    // baud (serial 6850-4024), SW1 in schematics
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")    // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")    // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg076 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)  // credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x30, 0x00, "Minimum Winning Hand") PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "Jacks or Better" )
	PORT_DIPSETTING(    0x20, "Queens or Better" )
	PORT_DIPSETTING(    0x30, "Kings or Better" )
	PORT_DIPSETTING(    0x10, "Pair of Aces" )
	PORT_DIPNAME( 0xC0, 0x80, "Maximum Bet")        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0xC0, "20" )
	PORT_DIPSETTING(    0x40, "50" )

	PORT_START("SW2")    // baud (serial 6850-4024), SW1 in schematics
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")    // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")    // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( biggame )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)  // 5 credits by coin.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2)  // 10 credits by coin.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play/Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START )      PORT_NAME("Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("Select 1")  PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("Select 2")  PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("Select 3")  PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "Coin A: 1; Coin B: 10" )
	PORT_DIPSETTING(    0x80, "Coin A: 5; Coin B: 10" )

	PORT_START("SW2")   // baud (serial 6850-4024), SW1 in schematics...
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 50, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( nudgensw )    // nudge bingo
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 1 Up") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 2 Up") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 3 Up") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 4 Up") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 5 Up") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("Replay / Credits")  PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test / Diagnostics")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 1 Down") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 2 Down") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 3 Down") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 4 Down") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Nudge 5 Down") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")   // baud (serial 6850-4024), SW1 in schematics...
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 0, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( nudgesw )

	PORT_INCLUDE( nudgensw )

	PORT_MODIFY("SW1")    // settings (PIA1), SW2 in schematics
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Options" )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "Play 1" )
	PORT_DIPSETTING(    0x40, "Play 1 to 3" )
	PORT_DIPSETTING(    0x80, "Play 1 to 4" )
	PORT_DIPSETTING(    0xc0, "Play 1 to 8" )
INPUT_PORTS_END


static INPUT_PORTS_START( hotline ) // nudge bingo
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Up 1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Up 2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Up 3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Up 4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Up 5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("Credits")    PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Learn")      PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Clear Data") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Audit")      PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Down 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Down 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Down 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Down 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move Down 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("Tilt") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // connected to SW2-5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")   // baud (serial 6850-4024), SW1 in schematics...
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 0, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg128 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Door Open")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xC0, 0x40, "Hands per Coin")      PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xC0, "5" )

	PORT_START("SW2")    // baud (serial 6850-4024), SW1 in schematics
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")    // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")    // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg157 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)      PORT_NAME("Coin-In A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                  PORT_NAME("Double")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                 PORT_NAME("Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2)      PORT_NAME("Coin-In B")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Data / Audit")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )                      PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )                  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )                  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )                  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )                  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )                  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_T) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                   PORT_NAME("Play (Bet)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                   PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x30, 0x00, "Minimum Winning Hand") PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "Jacks or Better" )
	PORT_DIPSETTING(    0x20, "Queens or Better" )
	PORT_DIPSETTING(    0x30, "Kings or Better" )
	PORT_DIPSETTING(    0x10, "Pair of Aces" )
	PORT_DIPNAME( 0xC0, 0x80, "Maximum Bet")        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0xC0, "20" )
	PORT_DIPSETTING(    0x40, "50" )

	PORT_START("SW2")    // baud (serial 6850-4024), SW1 in schematics
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")    // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg159 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Play (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Settings (%)") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Clr Data")     PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Audit")        PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stand")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_NAME("Coin-In A")    PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_NAME("Coin-In B")    PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Tilt")         PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )  PORT_NAME("Double")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// For System 905, SW1 uses the whole PIA1 portA.

	PORT_START("SW1")    // settings (PIA1)
	PORT_DIPNAME( 0x01, 0x01, "Game Type" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Hold" )
	PORT_DIPSETTING(    0x00, "Discard" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Pay Table" )         PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "250/100/25/10/8/5/4/3/2/1" )
	PORT_DIPSETTING(    0x20, "500/200/40/10/8/5/4/3/2/1" )
	PORT_DIPSETTING(    0x00, "500/250/50/20/8/6/4/3/2/1" )
	PORT_DIPSETTING(    0x10, "500/250/50/25/10/8/5/3/2/1" )
	PORT_DIPNAME( 0xc0, 0xc0, "Low Winning Pair" )  PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "Jacks" )
	PORT_DIPSETTING(    0x80, "Queens" )
	PORT_DIPSETTING(    0xc0, "Kings" )
	PORT_DIPSETTING(    0x40, "Aces" )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg170 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Play (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Learn")        PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Clr Data")     PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Audit")        PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stand")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_NAME("Coin-In A")    PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_NAME("Coin-In B")    PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Tilt")         PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )  PORT_NAME("Double")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// For System 905, SW1 uses the whole PIA1 portA.

	PORT_START("SW1")    // settings (PIA1)
	PORT_DIPNAME( 0x07, 0x07, "Max Bet" )                        PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "01" )
	PORT_DIPSETTING(    0x01, "02" )
	PORT_DIPSETTING(    0x02, "03" )
	PORT_DIPSETTING(    0x03, "04" )
	PORT_DIPSETTING(    0x04, "05" )
	PORT_DIPSETTING(    0x05, "10" )
	PORT_DIPSETTING(    0x06, "20" )
	PORT_DIPSETTING(    0x07, "50" )
	PORT_DIPNAME( 0x08, 0x08, "Attract" )                        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Lower Pair to Win" )              PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "Jacks" )
	PORT_DIPSETTING(    0x20, "Queens" )
	PORT_DIPSETTING(    0x30, "Kings" )
	PORT_DIPSETTING(    0x10, "Aces" )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin Lockout" )                   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "No Lockout" )
	PORT_DIPSETTING(    0x40, "No Lockout" )
	PORT_DIPSETTING(    0x80, "1-Coin Lockout" )
	PORT_DIPSETTING(    0x00, "2-Coin Lockout" )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg172 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Play (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Learn")        PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Clr Data")     PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Audit")        PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stand")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_NAME("Coin-In A")    PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_NAME("Coin-In B")    PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Tilt")         PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )  PORT_NAME("Double")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// For System 905, SW1 uses the whole PIA1 portA.

	PORT_START("SW1")    // settings (PIA1)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )               PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )               PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Double Deal" )                    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )               PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, "Lower Pair to Win + Pay Table" )  PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "Jacks, 250/50/20/8/6/4/3/2/1" )
	PORT_DIPSETTING(    0x20, "Queens, 200/40/10/8/5/4/3/2/1" )
	PORT_DIPSETTING(    0x30, "Kings, 200/40/10/8/5/4/3/2/1" )
	PORT_DIPSETTING(    0x10, "Aces, 250/50/25/10/8/5/3/2/1" )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin Lockout" )                   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "No Lockout" )
	PORT_DIPSETTING(    0x40, "No Lockout" )
	PORT_DIPSETTING(    0x80, "1-Coin Lockout" )
	PORT_DIPSETTING(    0x00, "2-Coin Lockout" )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( elgrande )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Holds")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Service")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Odd")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Even")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )           PORT_CONDITION("FRQ", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "1 Coin/25 Credits" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x40)
	PORT_DIPNAME( 0x30, 0x20, "Minimum Winning Hand")
	PORT_DIPSETTING(    0x20, "Jacks or Better" )
	PORT_DIPSETTING(    0x30, "Queens or Better" )
	PORT_DIPSETTING(    0x00, "Kings or Better" )
	PORT_DIPSETTING(    0x10, "Aces or Better" )
	PORT_DIPNAME( 0xC0, 0x40, "Maximum Bet")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0xC0, "50" )

	PORT_START("FRQ")    // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Coin B Modifer" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 0, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( jjpoker )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Service")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Odd")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Even")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )           PORT_CONDITION("FRQ", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "1 Coin/25 Credits" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x40)
	PORT_DIPNAME( 0x30, 0x20, "Minimum Winning Hand")
	PORT_DIPSETTING(    0x20, "Jacks or Better" )
	PORT_DIPSETTING(    0x30, "Queens or Better" )
	PORT_DIPSETTING(    0x00, "Kings or Better" )
	PORT_DIPSETTING(    0x10, "Aces or Better" )
	PORT_DIPNAME( 0xC0, 0x40, "Maximum Bet")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0xC0, "50" )

	PORT_START("FRQ")    // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Coinage Modifer" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 0, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( ssipkr )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel Discards")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Service")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Test Mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Odd")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Even")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, "1 Coin/1 Point" )          PORT_CONDITION("FRQ", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x08, "1 Coin/5 Points" )         PORT_CONDITION("FRQ", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "1 Coin/10 Points" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, "1 Coin/25 Points" )        PORT_CONDITION("FRQ", 0x40, EQUALS, 0x00)
	PORT_DIPNAME( 0x30, 0x20, "Minimum Winning Hand")
	PORT_DIPSETTING(    0x20, "Jacks or Better" )
	PORT_DIPSETTING(    0x30, "Queens or Better" )
	PORT_DIPSETTING(    0x00, "Kings or Better" )
	PORT_DIPSETTING(    0x10, "Aces or Better" )
	PORT_DIPNAME( 0xc0, 0x40, "Maximum Bet")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0xc0, "50" )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Coinage Modifer" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 0, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( comg204 )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Play (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Hit (Pedir)") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_STAND ) PORT_NAME("Stand")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )  PORT_NAME("Double")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Learn")        PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Clr Data")     PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Audit")        PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_NAME("Coin-In A")    PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_NAME("Coin-In B")    PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Tilt")         PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// For System 905, SW1 uses the whole PIA1 portA.

	PORT_START("SW1")    // settings (PIA1)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( stand906 )

	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )         PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_NAME("Deal / LKOUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )  PORT_NAME("Cancel Discards")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )  PORT_NAME("Stand / DVTR")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM )        PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Weight") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )                        PORT_NAME("Door")          PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_9) PORT_NAME("Menu")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                      PORT_NAME("Hand Pay")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE)        PORT_CODE(KEYCODE_R) PORT_NAME("Error Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )


	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )   PORT_NAME("Discard 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )   PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )   PORT_NAME("Discard 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )   PORT_NAME("Discard 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )   PORT_NAME("Discard 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_D) PORT_NAME("Drop Door") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_8) PORT_NAME("Change")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_0) PORT_NAME("Prog Hit")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")    // settings (PIAT1)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")    // Tied to AY8912 port. Covered with tape except SW2-8
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( cas21iwc )

	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2) PORT_NAME("Coin In (Fichas Entrada)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                  PORT_NAME("Take (Cobrar)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM )        PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Height (Peso Tolva)") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )                        PORT_NAME("Door (Puerta)")              PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_9) PORT_NAME("Menu")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                      PORT_NAME("Hand Pay (Pago Manual)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE)        PORT_CODE(KEYCODE_R) PORT_NAME("Error Reset (Correccion Error)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")  // TODO: BJ predef inputs...
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_CODE(KEYCODE_X) PORT_NAME("Deal (Dar) / Cierre")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )   PORT_CODE(KEYCODE_C) PORT_NAME("Double-Up (Doblar)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )   PORT_CODE(KEYCODE_V) PORT_NAME("Hit (Pedir)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )  PORT_CODE(KEYCODE_B) PORT_NAME("Stand (Plantar) / Desv")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_D) PORT_NAME("Drop Door (Cajon Abierto)") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                         PORT_NAME("Bet (Apostar)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_8) PORT_NAME("Attendant (Asistente)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")    // settings (PIAT1)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")    // Tied to AY8912 port. Covered with tape except SW2-8
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( pokeriwc )

	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )         PORT_NAME("Coin In (Fichas Entrada)")  PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )   PORT_NAME("Take (Cobrar)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_NAME("Deal (Dar) / Lockout (Cierre)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )  PORT_NAME("Cancel (Cancelar)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )  PORT_NAME("Stand (Plantar)/ DVTR (Desviar)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM )        PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Weight (Peso Tolva)") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )                        PORT_NAME("Door (Puerta Abierta)")      PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_9) PORT_NAME("Menu")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                      PORT_NAME("Hand Pay (Pago Manual)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE)        PORT_CODE(KEYCODE_R) PORT_NAME("Error Reset (Correccion de Errores)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )   PORT_NAME("Discard (Descartar) 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )   PORT_NAME("Discard (Descartar) 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )   PORT_NAME("Discard (Descartar) 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )   PORT_NAME("Discard (Descartar) 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )   PORT_NAME("Discard (Descartar) 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_D) PORT_NAME("Drop Door (Cajon Abierto)") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                         PORT_NAME("Bet (Apostar)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_8) PORT_NAME("Attendant (Asistente)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )       PORT_CODE(KEYCODE_0) PORT_NAME("Prog Hit (Pozo Acumulado)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")    // settings (PIAT1)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")    // Tied to AY8912 port. Covered with tape except SW2-8
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 0, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 0, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 75, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( pixels )  // sys905
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Bounce 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Bounce 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Bounce 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Bounce 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Bounce 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Play/Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Go/Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Learn")      PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Clear Data") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Audit")      PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Return") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Stay")   PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Tilt")   PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("Female") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("Male")   PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// For System 905, SW1 uses the whole PIA1 portA.

	PORT_START("SW1")    // settings (PIA1)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")    // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 100, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 50, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 50, "BLUE Preset" )

INPUT_PORTS_END


static INPUT_PORTS_START( pix903 )  // Pixels, sys903
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )        PORT_IMPULSE(2)  // credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Go/Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Return/Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )        PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Test Mode")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Bounce 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Bounce 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Bounce 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Bounce 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Bounce 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Tilt/Clear") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Play/Bet")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Female") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Male")   PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  SW1-5 should be wired to PIA0 portA, bit 6.
    SW1-6 should be wired to H-POL.
    SW1-7 should be wired to V-POL.
*/
	PORT_START("SW1")    // settings (PIA1), SW2 in schematics
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L6
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L7
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L8
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // L9
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")   // baud (serial 6850-4024), SW1 in schematics...
	PORT_DIPNAME( 0x3f, 0x08, "Baud Rate" )         PORT_DIPLOCATION("SW1:1,2,3,4,5,6")
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x02, "600" )
	PORT_DIPSETTING(    0x04, "1200" )
	PORT_DIPSETTING(    0x08, "2400" )
	PORT_DIPSETTING(    0x10, "4800" )
	PORT_DIPSETTING(    0x20, "9600" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")   // unknown (ay8912), SW3 in schematics
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FRQ")   // settings (PIA0)
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Frequency" )         PORT_DIPLOCATION("FRQ:1")
	PORT_DIPSETTING(    0x80, "60Hz." )
	PORT_DIPSETTING(    0x00, "50Hz." )

//  RGB analogic pots (defaults are in %)

	PORT_START("POT1_RED")
	PORT_ADJUSTER( 100, "RED Preset" )

	PORT_START("POT2_GREEN")
	PORT_ADJUSTER( 50, "GREEN Preset" )

	PORT_START("POT3_BLUE")
	PORT_ADJUSTER( 50, "BLUE Preset" )

INPUT_PORTS_END



/*************************************************
*                Graphics Layouts                *
*************************************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    // bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/*************************************************
*          Graphics Decode Information           *
*************************************************/

static GFXDECODE_START( gfx_calomega )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_sys906 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, tilelayout, 0, 32 )
GFXDECODE_END


/*************************************************
*                 PIA Interfaces                 *
*************************************************/

/********** System 903/904 PIA-0 (U54) wiring **********

   Pin |  Description   | Wired to
  -----+----------------+--------------------------
   01  |      VSS       | GND
   02  | Port A - bit 0 | U43 (4N23, multiplexer)
   03  | Port A - bit 1 | U44 (4N23, multiplexer)
   04  | Port A - bit 2 | U45 (4N23, multiplexer)
   05  | Port A - bit 3 | U46 (4N23, multiplexer)
   06  | Port A - bit 4 | U47 (4N23, multiplexer)
   07  | Port A - bit 5 | U48 (4N23, multiplexer)
   08  | Port A - bit 6 | SW1-5 (settings)
   09  | Port A - bit 7 | U80 (4023B)
   10  | Port B - bit 0 | U55 (MCT6) ---> L1
   11  | Port B - bit 1 | U56 (MCT6) ---> L2
   12  | Port B - bit 2 | U56 (MCT6) ---> L3
   13  | Port B - bit 3 | U57 (MCT6) ---> L4
   14  | Port B - bit 4 | U57 (MCT6) ---> L5
   15  | Port B - bit 5 | U58 (H74C2/OPI3023) --> M1
   16  | Port B - bit 6 | U59 (H74C2/OPI3023) --> M2
   17  | Port B - bit 7 | U60 (H74C2/OPI3023) --> M3
   18  |      CB1       | U34 (556, pin 5)
   19  |      CB2       | U62 (H74C2/OPI3023) --> M5
   20  |      VCC       | +5V
   21  |      R/W       | R/W
   22  |      CS0       | A2
   23  |      CS2       | I/O
   24  |      CS1       | +5V
   25  |       E        | 02 (= CPU clock)
   26  |  Data Bus D7   | D7
   27  |  Data Bus D6   | D6
   28  |  Data Bus D5   | D5
   29  |  Data Bus D4   | D4
   30  |  Data Bus D3   | D3
   31  |  Data Bus D2   | D2
   32  |  Data Bus D1   | D1
   33  |  Data Bus D0   | D0
   34  |      RES       | RES
   35  |      RS1       | A1
   36  |      RS0       | A0
   37  |      IRQB      | IRQ
   38  |      IRQA      | IRQ
   39  |      CA2       | U61 (H74C2/OPI3023) --> M4
   40  |      CA1       | U34 (556, pin 5)

*/

/********** Systems 903/904 PIA-1 (U39) wiring **********

   Pin |  Description   | Wired to
  -----+----------------+--------------------------
   01  |      VSS       | GND
   02  | Port A - bit 0 | U52 (MCT6) ---> L6
   03  | Port A - bit 1 | U52 (MCT6) ---> L7
   04  | Port A - bit 2 | U53 (MCT6) ---> L8
   05  | Port A - bit 3 | U53 (MCT6) ---> L9
   06  | Port A - bit 4 | SW1-1 (settings)
   07  | Port A - bit 5 | SW1-2 (settings)
   08  | Port A - bit 6 | SW1-3 (settings)
   09  | Port A - bit 7 | SW1-4 (settings)
   10  | Port B - bit 0 | U65 (H74C2/OPI3023) --> Hopper 1 (I/O)
   11  | Port B - bit 1 | U66 (H74C2/OPI3023) --> Hopper 2 (I/O)
   12  | Port B - bit 2 | DIV
   13  | Port B - bit 3 | LOCK
   14  | Port B - bit 4 | U50 (MCT6) ---> A COM
   15  | Port B - bit 5 | U50 (MCT6) ---> B COM
   16  | Port B - bit 6 | U51 (MCT6) ---> C COM
   17  | Port B - bit 7 | U51 (MCT6) ---> D COM
   18  |      CB1       | VS
   19  |      CB2       | N/C
   20  |      VCC       | +5V
   21  |      R/W       | R/W
   22  |      CS0       | A3
   23  |      CS2       | I/O
   24  |      CS1       | +5V
   25  |       E        | 02 (= CPU clock)
   26  |  Data Bus D7   | D7
   27  |  Data Bus D6   | D6
   28  |  Data Bus D5   | D5
   29  |  Data Bus D4   | D4
   30  |  Data Bus D3   | D3
   31  |  Data Bus D2   | D2
   32  |  Data Bus D1   | D1
   33  |  Data Bus D0   | D0
   34  |      RES       | RES
   35  |      RS1       | A1
   36  |      RS0       | A0
   37  |      IRQB      | IRQ
   38  |      IRQA      | N/C
   39  |      CA2       | U34 (556, pins 8 & 12) +  SW1-6 (settings)
   40  |      CA1       | GND

*/

/********** System 905 PIA-0 (U48) wiring **********

   Pin |  Description   | Wired to
  -----+----------------+--------------------------
   01  |      VSS       | GND
   02  | Port A - bit 0 | U45 (4N23, multiplexer)
   03  | Port A - bit 1 | U45 (4N23, multiplexer)
   04  | Port A - bit 2 | U46 (4N23, multiplexer)
   05  | Port A - bit 3 | U46 (4N23, multiplexer)
   06  | Port A - bit 4 | U47 (4N23, multiplexer)
   07  | Port A - bit 5 | U47 (4N23, multiplexer)
   08  | Port A - bit 6 | U61 (4023B) --> U57 (556, pin 5) [50/60]
   09  | Port A - bit 7 | U57 (556, pin 2) --> SW to GND   [50/60]
   10  | Port B - bit 0 | U49 (MCT6) ---> L1
   11  | Port B - bit 1 | U49 (MCT6) ---> L2
   12  | Port B - bit 2 | U50 (MCT6) ---> L3
   13  | Port B - bit 3 | U50 (MCT6) ---> L4
   14  | Port B - bit 4 | U51 (MCT6) ---> L5
   15  | Port B - bit 5 | U51 (MCT6) ---> L6
   16  | Port B - bit 6 | --------------> L7
   17  | Port B - bit 7 | --------------> L8
   18  |      CB1       | N/C
   19  |      CB2       | N/C
   20  |      VCC       | +5V
   21  |      R/W       | R/W
   22  |      CS0       | A2
   23  |      CS2       | I/O
   24  |      CS1       | +5V
   25  |       E        | 02 (= CPU clock)
   26  |  Data Bus D7   | D7
   27  |  Data Bus D6   | D6
   28  |  Data Bus D5   | D5
   29  |  Data Bus D4   | D4
   30  |  Data Bus D3   | D3
   31  |  Data Bus D2   | D2
   32  |  Data Bus D1   | D1
   33  |  Data Bus D0   | D0
   34  |      RES       | RES
   35  |      RS1       | A1
   36  |      RS0       | A0
   37  |      IRQB      | IRQ
   38  |      IRQA      | N/C
   39  |      CA2       | N/C
   40  |      CA1       | N/C

*/

/********** Systems 905 PIA-1 (U63) wiring **********

   Pin |  Description   | Wired to
  -----+----------------+--------------------------
   01  |      VSS       | GND
   02  | Port A - bit 0 | SW1-1 (settings)
   03  | Port A - bit 1 | SW1-2 (settings)
   04  | Port A - bit 2 | SW1-3 (settings)
   05  | Port A - bit 3 | SW1-4 (settings)
   06  | Port A - bit 4 | SW1-5 (settings)
   07  | Port A - bit 5 | SW1-6 (settings)
   08  | Port A - bit 6 | SW1-7 (settings)
   09  | Port A - bit 7 | SW1-8 (settings)
   10  | Port B - bit 0 | U64 (MCT6) ---> A COM
   11  | Port B - bit 1 | U64 (MCT6) ---> B COM
   12  | Port B - bit 2 | U65 (MCT6) ---> C COM
   13  | Port B - bit 3 | U65 (MCT6) ---> D COM
   14  | Port B - bit 4 | AC1
   15  | Port B - bit 5 | AC2
   16  | Port B - bit 6 | N/C
   17  | Port B - bit 7 | LOCK
   18  |      CB1       | IOSC
   19  |      CB2       | N/C
   20  |      VCC       | +5V
   21  |      R/W       | R/W
   22  |      CS0       | A3
   23  |      CS2       | I/O
   24  |      CS1       | +5V
   25  |       E        | 02 (= CPU clock)
   26  |  Data Bus D7   | D7
   27  |  Data Bus D6   | D6
   28  |  Data Bus D5   | D5
   29  |  Data Bus D4   | D4
   30  |  Data Bus D3   | D3
   31  |  Data Bus D2   | D2
   32  |  Data Bus D1   | D1
   33  |  Data Bus D0   | D0
   34  |      RES       | RES
   35  |      RS1       | A1
   36  |      RS0       | A0
   37  |      IRQB      | IRQ
   38  |      IRQA      | N/C
   39  |      CA2       | N/C
   40  |      CA1       | GND

*/


/*************************************************
*                 ACIA Interface                 *
*************************************************/

void calomega_state::write_acia_clock(int state)
{
	m_acia6850[0]->write_txc(state);
	m_acia6850[0]->write_rxc(state);
	update_aciabaud_scale(0);
}

void calomega_state::w_903kb_acia_clock(int state)
{
	m_acia6850[0]->write_txc(state);
	m_acia6850[0]->write_rxc(state);
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
	update_aciabaud_scale(0);
}

/*************************************************
*                Machine Drivers                 *
*************************************************/

void calomega_state::sys903(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, CPU_CLOCK);   // confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &calomega_state::sys903_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(calomega_state::s903_mux_port_r));
	m_pia[0]->readpb_handler().set(FUNC(calomega_state::dummy_pia_r));        // Debug: some dipsw maybe???
	m_pia[0]->writepa_handler().set(FUNC(calomega_state::dummy_pia_w));       // Debug: to assign lamps/counters/solenids/hopper????
	m_pia[0]->writepb_handler().set(FUNC(calomega_state::lamps_903a_w));
	m_pia[0]->ca1_w(0);                                                       // Timer Interrupt Ack.     (CPU -> PIA)
	m_pia[0]->cb1_w(0);                                                       // Timer Interrupt Ack.     (CPU -> PIA)
	m_pia[0]->irqa_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // Timer Interrupt Event.   (PIA -> CPU)
	m_pia[0]->irqb_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // Timer Interrupt Event.   (PIA -> CPU)

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("SW1");
	m_pia[1]->readpb_handler().set(FUNC(calomega_state::dummy_pia_r));        // Debug: unknown reads.
	m_pia[1]->writepa_handler().set(FUNC(calomega_state::lamps_903b_w));
	m_pia[1]->writepb_handler().set(FUNC(calomega_state::s903_mux_w));        // Mux. Scan Lines.(Upper nibble) - Hopper b0-b1 (Lower Nibble).
	m_pia[1]->ca2_handler().set(FUNC(calomega_state::dummy_pia_line_w));      // M4 pulsed out
	m_pia[1]->cb2_handler().set(FUNC(calomega_state::dummy_pia_line_w));      // M5 pulsed out
	m_pia[1]->cb1_w(0);                                                       // V_BLANK Interrupt Ack.   (CPU -> PIA)
	m_pia[1]->irqb_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // V_BLANK Interrupt Event. (PIA -> CPU)

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(100));
	screen.set_size((39+1)*8, (31+1)*8);             // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1).
	screen.set_visarea(0*8, 32*8-1, 0*8, 31*8-1);    // Taken from MC6845 init, registers 01 & 06.
	screen.set_screen_update(FUNC(calomega_state::screen_update_calomega));

	screen.screen_vblank().set(FUNC(calomega_state::vblank0_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_calomega);
	PALETTE(config, m_palette, FUNC(calomega_state::calomega_palette), 256);

	mc6845_device &crtc(MC6845(config, "crtc", CPU_CLOCK));  // 6845 @ CPU clock
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8912_device &ay8912(AY8912(config, "ay8912", SND_CLOCK));  // confirmed
	ay8912.port_a_read_callback().set_ioport("SW3");             // from schematics
	ay8912.add_route(ALL_OUTPUTS, "mono", 0.75);

	// acia 0
	ACIA6850(config, m_acia6850[0], 0);
	m_acia6850[0]->irq_handler().set_inputline("maincpu", M6502_IRQ_LINE);

	clock_device &aciabaud(CLOCK(config, "aciabaud", UART_CLOCK));
	aciabaud.signal_handler().set(FUNC(calomega_state::write_acia_clock));

	TIMER(config, "timer_0").configure_periodic(FUNC(calomega_state::timer_0), attotime::from_hz(550*2));  // (time*2) - Each timer pulse -> half period

	HOPPER(config, m_hopper, attotime::from_msec(50));
}


void calomega_state::sys903kb(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, CPU_CLOCK);   // confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &calomega_state::sys903_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I8035(config, m_kstec, XTAL(5'185'000)); // from schematics
	m_kstec->set_addrmap(AS_PROGRAM, &calomega_state::kstec_mem_map);
	m_kstec->set_addrmap(AS_IO, &calomega_state::kstec_io_map);
	m_kstec->p2_out_cb().set([this] (int data) { calomega_state::m_kbscan = (data & 0xf0) >> 4; });  // set keyboard scan lines
	m_kstec->p1_in_cb().set(FUNC(calomega_state::keyb_903_r));                                       // get keyboard input lines
	m_kstec->t0_in_cb().set("uart", FUNC(i8251_device::txrdy_r));                                    // uart tx handshake
	m_kstec->t1_in_cb().set([this] () { return calomega_state::m_rxrdy; });                          // uart rx handshake

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(calomega_state::s903_mux_port_r));
	m_pia[0]->readpb_handler().set(FUNC(calomega_state::dummy_pia_r));        // Debug: some dipsw maybe???
	m_pia[0]->writepa_handler().set(FUNC(calomega_state::dummy_pia_w));       // Debug: to assign lamps/counters/solenids/hopper????
	m_pia[0]->writepb_handler().set(FUNC(calomega_state::lamps_903a_w));
	m_pia[0]->ca1_w(0);                                                       // Timer Interrupt Ack.     (CPU -> PIA)
	m_pia[0]->cb1_w(0);                                                       // Timer Interrupt Ack.     (CPU -> PIA)
	m_pia[0]->irqa_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // Timer Interrupt Event.   (PIA -> CPU)
	m_pia[0]->irqb_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // Timer Interrupt Event.   (PIA -> CPU)

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("SW1");
	m_pia[1]->readpb_handler().set(FUNC(calomega_state::dummy_pia_r));        // Debug: unknown reads.
	m_pia[1]->writepa_handler().set(FUNC(calomega_state::lamps_903b_w));
	m_pia[1]->writepb_handler().set(FUNC(calomega_state::s903_mux_w));        // Mux. Scan Lines.(Upper nibble) - Hopper b0-b1 (Lower Nibble).
	m_pia[1]->ca2_handler().set(FUNC(calomega_state::dummy_pia_line_w));      // M4 pulsed out
	m_pia[1]->cb2_handler().set(FUNC(calomega_state::dummy_pia_line_w));      // M5 pulsed out
	m_pia[1]->cb1_w(0);                                                       // V_BLANK Interrupt Ack.   (CPU -> PIA)
	m_pia[1]->irqb_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // V_BLANK Interrupt Event. (PIA -> CPU)

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(100));
	screen.set_size((39+1)*8, (31+1)*8);             // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1).
	screen.set_visarea(0*8, 32*8-1, 0*8, 31*8-1);    // Taken from MC6845 init, registers 01 & 06.
	screen.set_screen_update(FUNC(calomega_state::screen_update_calomega));

	screen.screen_vblank().set(FUNC(calomega_state::vblank0_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_calomega);
	PALETTE(config, m_palette, FUNC(calomega_state::calomega_palette), 256);

	mc6845_device &crtc(MC6845(config, "crtc", CPU_CLOCK));  // 6845 @ CPU clock
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8912_device &ay8912(AY8912(config, "ay8912", SND_CLOCK));  // confirmed
	ay8912.port_a_read_callback().set_ioport("SW3");             // from schematics
	ay8912.add_route(ALL_OUTPUTS, "mono", 0.75);

	// acia 0
	ACIA6850(config, m_acia6850[0], 0);
	m_acia6850[0]->txd_handler().set("uart", FUNC(i8251_device::write_rxd));
	m_acia6850[0]->irq_handler().set_inputline("maincpu", M6502_IRQ_LINE);

	I8251(config, m_uart, 5'185'000);
	m_uart->txd_handler().set("acia6850_0", FUNC(acia6850_device::write_rxd));
	m_uart->rxrdy_handler().set([this] (int state) { calomega_state::m_rxrdy = state;});

	clock_device &aciabaud(CLOCK(config, "aciabaud", UART_CLOCK));
	aciabaud.signal_handler().set(FUNC(calomega_state::w_903kb_acia_clock));

	TIMER(config, "timer_0").configure_periodic(FUNC(calomega_state::timer_0), attotime::from_hz(550*2));  // (time*2) - Each timer pulse -> half period

	HOPPER(config, m_hopper, attotime::from_msec(50));
}


void calomega_state::s903mod(machine_config &config)
{
	sys903(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &calomega_state::s903mod_map);

	// sound hardware
	subdevice<ay8912_device>("ay8912")->port_a_read_callback().set_constant(0);

	config.device_remove("acia6850_0");
	config.device_remove("aciabaud");
}


void calomega_state::sys905(machine_config &config)
{
	sys903(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &calomega_state::sys905_map);

	// video hardware
	screen_device &screen(SCREEN(config.replace(), "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(100));
	screen.set_size((39+1)*8, (31+1)*8);             // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1).
	screen.set_visarea(0*8, 32*8-1, 0*8, 31*8-1);    // Taken from MC6845 init, registers 01 & 06.
	screen.set_screen_update(FUNC(calomega_state::screen_update_calomega));

	screen.screen_vblank().set(FUNC(calomega_state::vblank1_w));

	PIA6821(config.replace(), m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(calomega_state::s905_mux_port_r));    // Muxed inputs read.
	m_pia[0]->writepb_handler().set(FUNC(calomega_state::lamps_905_w));       // Full Lamps - OK.
	m_pia[0]->irqb_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // V_BLANK Interrupt Event. (PIA -> CPU)

	PIA6821(config.replace(), m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("SW1");                             // Full port.
	m_pia[1]->writepb_handler().set(FUNC(calomega_state::s905_mux_w));        // Mux. Scan Lines.(Lower Nibble). Hopper & Coin Lockout (Upper nibble).
	m_pia[1]->irqb_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // Timer Interrupt Event.    (PIA -> CPU)

	// sound hardware
	subdevice<ay8912_device>("ay8912")->port_a_read_callback().set_constant(0);

	TIMER(config.replace(), "timer_0").configure_periodic(FUNC(calomega_state::timer_1), attotime::from_hz(520*2)); // (time*2) - Each timer pulse -> half period

	config.device_remove("acia6850_0");
	config.device_remove("aciabaud");
}


void calomega_state::sys906(machine_config &config)
{
	sys903(config);

	// basic machine hardware
	R65C02(config.replace(), m_maincpu, CPU_CLOCK);  // confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &calomega_state::sys906_map);

	screen_device &screen(SCREEN(config.replace(), "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(100));
	screen.set_size((39+1)*8, (31+1)*8);             // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1).
	screen.set_visarea(0*8, 32*8-1, 0*8, 31*8-1);    // Taken from MC6845 init, registers 01 & 06.
	screen.set_screen_update(FUNC(calomega_state::screen_update_calomega));

	screen.screen_vblank().set(FUNC(calomega_state::vblank2_w));
	PIA6821(config.replace(), m_pia[0]);
	m_pia[0]->readpa_handler().set_ioport("SW1");                             // Bit 7 - CGBANK ???? input?-output?
	m_pia[0]->ca2_handler().set(FUNC(calomega_state::dummy_pia_line_w));      // Ignored
	m_pia[0]->cb2_handler().set(FUNC(calomega_state::dummy_pia_line_w));      // Diverter
	m_pia[0]->writepa_handler().set(FUNC(calomega_state::pia0_aout_w));       // Bit 7 - CGBANK ???? input?-output?
	m_pia[0]->writepb_handler().set(FUNC(calomega_state::pia0_bout_w));       // Meters ??? (M1-M5), coils, hopper

	PIA6821(config.replace(), m_pia[1]);
	m_pia[1]->readpa_handler().set(FUNC(calomega_state::pia1_ain_r));         // Mux. Button read.
	m_pia[1]->readpb_handler().set(FUNC(calomega_state::pia1_bin_r));
	m_pia[1]->ca1_w(0);                                                       // Timer Interrupt Ack.      (CPU -> PIA)
	m_pia[1]->cb1_w(0);                                                       // V_BLANK Interrupt Ack.    (CPU -> PIA)
	m_pia[1]->writepa_handler().set(FUNC(calomega_state::pia1_aout_w));       // Mux. Scan lines
	m_pia[1]->writepb_handler().set(FUNC(calomega_state::pia1_bout_w));       // Lamps 1 to 8
	m_pia[1]->cb2_handler().set(FUNC(calomega_state::pia1_cb2_w));            // V_BLANK interrupt line input (from Vertical Synch)
	m_pia[1]->irqa_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // Timer Interrupt Event.    (PIA -> CPU)
	m_pia[1]->irqb_handler().set_inputline("maincpu", M6502_IRQ_LINE);        // V_BLANK Interrupt Event.  (PIA -> CPU)

	m_gfxdecode->set_info(gfx_sys906);

	// sound hardware
	ay8912_device &ay8912(AY8912(config.replace(), "ay8912", SND_CLOCK));     // confirmed
	ay8912.port_a_read_callback().set_ioport("SW2");                          // from schematics
	ay8912.add_route(ALL_OUTPUTS, "mono", 0.75);

	TIMER(config.replace(), "timer_0").configure_periodic(FUNC(calomega_state::timer_2), attotime::from_hz(380*2));  // (time*2) - Each timer pulse -> half period
}


/*************************************************
*                    ROM Load                    *
**************************************************
*
* Notes:
*
*  jkrpkr, jkr2, jkrtwo: graphics are identical.
*  cpkcg, pkcg:          graphics are identical.
*
*************************************************/

ROM_START( comg074 )  // Cal Omega v7.4 (Gaming Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1800.u5",    0x1800, 0x0800, CRC(69759432) SHA1(d64646476b4f67088bf5996ffc272d2571e62c53) )  // Seems to be from v6.5. Unused.
	ROM_LOAD( "2000.u6",    0x2000, 0x0800, CRC(bd9044f6) SHA1(9ebfc8379fe79a84982a0176a6b26267580272de) )
	ROM_LOAD( "2800.u7",    0x2800, 0x0800, CRC(d1d2e111) SHA1(2e5b6fbaf04539851d0f0674f0fd86e9be90c0fd) )
	ROM_LOAD( "3000.u8",    0x3000, 0x0800, CRC(cac4af01) SHA1(aed2986575b8d5539581515d818cb5bb9054c7c9) )
	ROM_LOAD( "3800.u9",    0x3800, 0x0800, CRC(6d32c533) SHA1(286cd511dc42e4e98c0e5fee2c7b265a4db949cf) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "poker_cg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "poker_cg2c.u70", 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "poker_cg2b.u69", 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "poker_cg2a.u68", 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg076 )  // Cal Omega v7.6 (Arcade Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "07-62.u6",   0x2000, 0x0800, CRC(99ffa2a4) SHA1(07fd182ff86f9ab09bbf09f51d655811a09ffa03) )
	ROM_LOAD( "07-63.u7",   0x2800, 0x0800, CRC(9ed58bc5) SHA1(c545053847ec7585e4ac97c70cf33529ed0f1111) )
	ROM_LOAD( "07-64.u8",   0x3000, 0x0800, CRC(95714680) SHA1(26938903600bd0920b1dd1c6900c56c6b749976a) )
	ROM_LOAD( "07-65.u9",   0x3800, 0x0800, CRC(02be8b39) SHA1(bb056e6e0ebd5f81c61fa7aa8252330088b3088e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "pkcg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pkcgc.u70",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pkcgb.u69",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pkcga.u68",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg079 )  // Cal Omega v7.9 (Arcade Poker)
/*  The original code jumps to $2e64 where there's an inexistent mnemonic 0x32! (kill).
    Also writes to $axxx (inexistent) and $08fx-$09xx (supposed to be $0880-$0881).
    Found damaged bits on this program ROM. Fix details on init().
*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "07-91.u5",   0x1800, 0x0800, CRC(da415c27) SHA1(be42f1d36579ff8fafd6df4e30a64a6141a7a2ae) )
	ROM_LOAD( "07-92.u6",   0x2000, 0x0800, CRC(31211ed3) SHA1(799bc4ca77ee01a4d45320263e4cc2d066ec26e5) )

	ROM_LOAD( "07-93.u7",   0x2800, 0x0800, BAD_DUMP CRC(52c84b9c) SHA1(569d6384b9084a11fe84e291f1c165bc0df3ab49) )
	ROM_LOAD( "07-94.u8",   0x3000, 0x0800, CRC(ed0e7d4a) SHA1(72f512ca2da573e4571ca6f164fb055daa26dd3c) )
	ROM_LOAD( "07-95.u9",   0x3800, 0x0800, CRC(3acc8f49) SHA1(1a193cc292f79da869caddd11478d6ad55466d7b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "pkcg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pkcgc.u70",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pkcgb.u69",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pkcga.u68",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "clrprom_7.9.u28", 0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

ROM_START( comg080 )  // Cal Omega v8.0 (Arcade Black Jack)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "08-02.u6",   0x2000, 0x0800, CRC(abf8c48e) SHA1(fea059af900fd6d17725ccf7a5ff2eb0af5c8e0a) )
	ROM_LOAD( "08-03.u7",   0x2800, 0x0800, CRC(e9ccb5af) SHA1(68a45d839afba3c6eafb7c75e5660de3a6be4eb5) )
	ROM_LOAD( "08-04.u8",   0x3000, 0x0800, CRC(325bdae8) SHA1(5a94c96ec3980361570da58fb407a1dba38064b8) )
	ROM_LOAD( "08-05.u9",   0x3800, 0x0800, BAD_DUMP CRC(71bd14d2) SHA1(00b3e2d965ac7ae1fc994b81246ad2c29ef4834c) )  // bad dump. 2nd half is blank

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "gpkcg0.u67", 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "gpkcgc.u70", 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "gpkcgb.u69", 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "gpkcga.u68", 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg094 )  // Cal Omega v9.4 (Keno)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "09-42.u6",   0x2000, 0x0800, CRC(1cff1319) SHA1(97b7ed54d398f36dc476028471784fc4e50bc378) )
	ROM_LOAD( "09-43.u7",   0x2800, 0x0800, CRC(e6b123be) SHA1(f89df2dc6deeecff41be83d7a9040cfe5d872bad) )
	ROM_LOAD( "09-44.u8",   0x3000, 0x0800, CRC(13939de9) SHA1(b2f97828808f6001846049cbf9af40e32908a58f) )
	ROM_LOAD( "09-45.u9",   0x3800, 0x0800, CRC(7508de2e) SHA1(62faf65a1b815e11158cfb807090923ab368784d) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x0800, 0xff )  // empty socket (requested by the manual)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "kcgc.u70",   0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kcgb.u69",   0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kcga.u68",   0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )

	ROM_REGION(0x1000, "kstec", 0)
	ROM_LOAD( "903kstec.0", 0x0000, 0x0800, CRC(c1636ab5) SHA1(5a3ad24918751ca6a6640807e421e80f6b4cc844) )
ROM_END

ROM_START( comg107 )  // Cal Omega v10.7c (Big Game)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "10-72.u6",   0x2000, 0x0800, CRC(dd0fbefb) SHA1(caed286ba1adb4d5c5c874c56339b9d71dd41bc6) )
	ROM_LOAD( "10-73.u7",   0x2800, 0x0800, CRC(fcb7774d) SHA1(d3c89e0df0005e4bc4894156622b3d1e4cd09f2a) )
	ROM_LOAD( "10-74.u8",   0x3000, 0x0800, CRC(d19ed885) SHA1(c41e59e87ce88a5b229e334b6a563a3b21d12b15) )
	ROM_LOAD( "10-75.u9",   0x3800, 0x0800, CRC(1491f3ab) SHA1(cf545f5f5da910b98846699bff1d2e6eb40dc290) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "lotcg0.u67", 0x0000, 0x1000, CRC(6f7cffee) SHA1(ababeb49155c84b22c4bb3568cfb5f05ff1cd797) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "lotcgc.u70", 0x0000, 0x1000, CRC(bfe4df1c) SHA1(f7e055d41e16ead6b18d755e664347645f94865a) )
	ROM_LOAD( "lotcgb.u69", 0x1000, 0x1000, CRC(5bda0f42) SHA1(d4b3340e9c8ca49483fa846103f0bd81d57a5ab3) )
	ROM_LOAD( "lotcga.u68", 0x2000, 0x1000, CRC(0975e360) SHA1(7b9dbbae50c43ad99ee11798ada0a44e71c611f9) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "bgclr.u28",  0x0000, 0x0100, CRC(6c2d44c5) SHA1(01412dbb9e8e30f01cc24fbf900c02eaf8956622) )
ROM_END

ROM_START( comg123 )  // Cal Omega v12.3 (Ticket Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "12-31.u5",   0x1800, 0x0800, BAD_DUMP CRC(8a1e9b03) SHA1(d57548226a695eb43d491dd953ad0b9b8ff8eb82) )
	ROM_LOAD( "12-32.u6",   0x2000, 0x0800, CRC(c30eb9c4) SHA1(bf3c2e069ecb9763028738c29054802b605cfa92) )
	ROM_LOAD( "12-33.u7",   0x2800, 0x0800, CRC(31472f2a) SHA1(5b82ef32f1bde44a0d83629fb3ce092c3b96c7d4) )
	ROM_LOAD( "12-34.u8",   0x3000, 0x0800, CRC(15a29f3f) SHA1(cfc31240ba01641d78c114ff40ab4536183cd829) )
	ROM_LOAD( "12-35.u9",   0x3800, 0x0800, BAD_DUMP CRC(f1e8ba9e) SHA1(605db3fdbaff4ba13729371ad0c4fbab3889378e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "pkcg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pkcgc.u70",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pkcgb.u69",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pkcga.u68",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg125 )  // Cal Omega v12.5 (Bingo)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "12-52.u6",   0x2000, 0x0800, CRC(ea0a6bd6) SHA1(f138d2f29252d95dea93a1e936725de99c714b35) )
	ROM_LOAD( "12-53.u7",   0x2800, 0x0800, CRC(c888ee34) SHA1(2796c6ae196b046f12b75b1c095a430fa4be0da2) )
	ROM_LOAD( "12-54.u8",   0x3000, 0x0800, CRC(4c7d11a6) SHA1(0131f077e204250d594c9baadc0596efeb7639cc) )
	ROM_LOAD( "12-55.u9",   0x3800, 0x0800, CRC(fabe1de0) SHA1(a39583d410707930fbe32341c30bb1b8a28a8d73) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "nbcg0.u67",  0x0000, 0x0800, CRC(b4b01f22) SHA1(1214d91ae28ac40ef37fce787783cd64331ed224) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "nbcgc.u70",  0x0000, 0x1000, CRC(f7ca234f) SHA1(3b2717051341c423969ed4bfb292a9f88119c2a8) )
	ROM_LOAD( "nbcgb.u69",  0x1000, 0x1000, CRC(9d409932) SHA1(d3ffca50a059278777238d206895a0d188f4ff6f) )
	ROM_LOAD( "nbcga.u68",  0x2000, 0x1000, CRC(afe1a666) SHA1(c1530700a283d18e7136754d45904930ef424bcf) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "bclr.u28",   0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
ROM_END

ROM_START( comg127 )  // Cal Omega v12.7 (Keno)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "12-72.u6",   0x2000, 0x0800, CRC(091a4b32) SHA1(3600854152482bf18d2377aa635a9fd7f3d4b2f7) )
	ROM_LOAD( "12-73.u7",   0x2800, 0x0800, CRC(c1fb5293) SHA1(f20cdddb4c89cb2fc647ddfcff6bd6f5095a0a28) )
	ROM_LOAD( "12-74.u8",   0x3000, 0x0800, CRC(de277137) SHA1(127b95616824dfb1025f3346c1335a8bf4835e68) )
	ROM_LOAD( "12-75.u9",   0x3800, 0x0800, CRC(04485ba3) SHA1(ee22396fc23508635e43ca8b17fc1f23e670fe85) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x0800, 0xff )  // empty socket (requested by the manual)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "kcgc.u70",   0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kcgb.u69",   0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kcga.u68",   0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )

	ROM_REGION(0x1000, "kstec", 0)
	ROM_LOAD( "903kstec.0", 0x0000, 0x0800, CRC(c1636ab5) SHA1(5a3ad24918751ca6a6640807e421e80f6b4cc844) )
ROM_END

ROM_START( comg128 )  // Cal Omega v12.8 (Arcade Game)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "12-82.u6",   0x2000, 0x0800, CRC(85f0c548) SHA1(f08e26a7acb883a92e73e564fb2b58422d1784c6) )
	ROM_LOAD( "12-83.u7",   0x2800, 0x0800, CRC(57212e46) SHA1(d0bac63e0efcdb8ef4a1b6c4d53776447557e4d0) )
	ROM_LOAD( "12-84.u8",   0x3000, 0x0800, CRC(00ab3cd8) SHA1(da00457c49d3a101dc31578d07b2e986f0d73919) )
	ROM_LOAD( "12-85.u9",   0x3800, 0x0800, CRC(3b00c172) SHA1(039aa2a2b41ea29e8e22a889c3984e30b321e7e3) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "pkcg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pkcgc.u70",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pkcgb.u69",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pkcga.u68",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg134 )  // Cal Omega 13.4 (Nudge Keno)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "13-42.u6",   0x2000, 0x0800, CRC(3ff1e83d) SHA1(0c2d2faed1148909dd7d50e9eda76c6403181435) )
	ROM_LOAD( "13-43.u7",   0x2800, 0x0800, CRC(cf2125f4) SHA1(32678f3ac82c76fb8116de77ce332fe098d5e8c0) )
	ROM_LOAD( "13-44.u8",   0x3000, 0x0800, CRC(09a8d3c4) SHA1(60937b386a7fa8c30bd509633b5e98e79a1189d4) )
	ROM_LOAD( "13-45.u9",   0x3800, 0x0800, CRC(790d3347) SHA1(02bbbfd447bee596557e4c3a61649ca1330a970f) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "nbcg0.u67",  0x0000, 0x0800, CRC(b4b01f22) SHA1(1214d91ae28ac40ef37fce787783cd64331ed224) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "nbcgc.u70",  0x0000, 0x1000, CRC(f7ca234f) SHA1(3b2717051341c423969ed4bfb292a9f88119c2a8) )
	ROM_LOAD( "nbcgb.u69",  0x1000, 0x1000, CRC(9d409932) SHA1(d3ffca50a059278777238d206895a0d188f4ff6f) )
	ROM_LOAD( "nbcga.u68",  0x2000, 0x1000, CRC(afe1a666) SHA1(c1530700a283d18e7136754d45904930ef424bcf) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "bclr.u28",   0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
ROM_END

ROM_START( comg145 )  // Cal Omega v14.5 (Pixels)
	ROM_REGION( 0x10000, "maincpu", 0 )  // the set doesn't pass the checksum, so needs a redump.
	ROM_LOAD( "14-51.u5",   0x1800, 0x0800, BAD_DUMP CRC(b173af4b) SHA1(2fd7d391c765e8e5bf148d92223ffc2a2619dcfd) )
	ROM_LOAD( "14-52.u6",   0x2000, 0x0800, BAD_DUMP CRC(f12434c5) SHA1(f957831867cc8c98fb479cd2d859790d19883d26) )
	ROM_LOAD( "14-53.u7",   0x2800, 0x0800, BAD_DUMP CRC(3b22802b) SHA1(8cc563e79fd90873132e358f2ce5942ddbd65144) )
	ROM_LOAD( "14-54.u8",   0x3000, 0x0800, BAD_DUMP CRC(8a230410) SHA1(4ff31869a7389af20f6557d9306f19a5d3ba5766) )
	ROM_LOAD( "14-55.u9",   0x3800, 0x0800, BAD_DUMP CRC(0e6e7413) SHA1(6e2b37f6cbb49ce3e0111777a3b197f5b2012ed0) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "pxcg0.u67",  0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "pxcgc.u70",  0x0000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )
	ROM_LOAD( "pxcgb.u69",  0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pxcga.u68",  0x2000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pixclr.u28", 0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
ROM_END

ROM_START( comg157 )  // Cal Omega v15.7 (Double-Draw Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15-71.u5",   0x1800, 0x0800, CRC(e0c89569) SHA1(68f32b00dd8ceb08e9677438c2bfec86e1a7fe6a) )
	ROM_LOAD( "15-72.u6",   0x2000, 0x0800, CRC(a3ebd1af) SHA1(a31eff7fe205efeb5b6fe1adaed66d2f23f91844) )
	ROM_LOAD( "15-73.u7",   0x2800, 0x0800, CRC(68682479) SHA1(5644e7f86a854572f4e54d6cf9552ef9ccbcb195) )
	ROM_LOAD( "15-74.u8",   0x3000, 0x0800, CRC(1d020dd0) SHA1(a36ae43587c0dd9abcb893db45715c405bb091fe) )
	ROM_LOAD( "15-75.u9",   0x3800, 0x0800, CRC(dba3ad3e) SHA1(4ef16e7b7456e189fcbf3ceba98955960d0953fc) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "gpkcg0.u67", 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "gpkcgc.u70", 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "gpkcgb.u69", 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "gpkcga.u68", 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg159 )  // Cal Omega v15.9 (Wild Double-Up)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_FILL(               0x4000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "15-93.u7",   0x5000, 0x1000, CRC(a1ca1fc0) SHA1(a751dba148c818a25237c2ac95328b6a5643f4e0) )
	ROM_LOAD( "15-94.u8",   0x6000, 0x1000, CRC(849595ea) SHA1(c28629de84b0c0d389c52490677ecb9139fd738d) )
	ROM_LOAD( "15-95.u9",   0x7000, 0x1000, CRC(c858ea24) SHA1(c7ecaddd4064b352c061b9164f2f347c310fab39) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "jkr2cg0.u67",    0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "jkr2cgc.u70",    0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkr2cgb.u69",    0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkr2cga.u68",    0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "wldclr.u28", 0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

ROM_START( comg164 )  // Cal Omega v16.4 (Keno)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "16-41.u5",   0x1800, 0x0800, CRC(fae6b065) SHA1(a123f98e4b4815a06d62d0429697cdce71756b94) )
	ROM_LOAD( "16-42.u6",   0x2000, 0x0800, NO_DUMP )   // missing ROM???
	ROM_LOAD( "16-43.u7",   0x2800, 0x0800, CRC(15974dbc) SHA1(fe2979861b8021949c127b182b9b50975b77bdd1) )
	ROM_LOAD( "16-44.u8",   0x3000, 0x0800, CRC(64f06a75) SHA1(1cf4d89d2ee60200f84d47a3a637471e0af9239c) )
	ROM_LOAD( "16-45.u9",   0x3800, 0x0800, CRC(3a8bc80e) SHA1(a1fb58b30850b1fe2bf976b8b99c8c540d67534a) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x0800, 0xff )  // empty socket (requested by the manual)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "kcgc.u70",   0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kcgb.u69",   0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kcga.u68",   0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )

	ROM_REGION(0x1000, "kstec", 0)
	ROM_LOAD( "903kstec.0", 0x0000, 0x0800, CRC(c1636ab5) SHA1(5a3ad24918751ca6a6640807e421e80f6b4cc844) )
ROM_END

ROM_START( comg168 )  // Cal Omega v16.8 (Keno)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "16-82.u6",   0x2000, 0x0800, CRC(ba044cd6) SHA1(659ec979e61baf4e871af857b722bb6fd789ff75) )
	ROM_LOAD( "16-83.u7",   0x2800, 0x0800, CRC(e3326b68) SHA1(7326f87319c363161ba8571dd983b070ef4f8694) )
	ROM_LOAD( "16-84.u8",   0x3000, 0x0800, CRC(1f72acea) SHA1(26d0e5a36f14ccae22d216a13d0459f0389ea6c0) )
	ROM_LOAD( "16-85.u9",   0x3800, 0x0800, CRC(4f38e3b3) SHA1(4d034959f665f0fdb5a4df85bae67dbd3d38077f) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x0800, 0xff )  // empty socket (requested by the manual)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "kcgc.u70",   0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kcgb.u69",   0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kcga.u68",   0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )

	ROM_REGION(0x1000, "kstec", 0)
	ROM_LOAD( "903kstec.0", 0x0000, 0x0800, CRC(c1636ab5) SHA1(5a3ad24918751ca6a6640807e421e80f6b4cc844) )
ROM_END

/*
  Cal Omega v17.0 (Amusement Poker)

  Discard poker game with attract.
  The game is only for amusement. It doesn't give back money.

  To clean the credits, enter the audit mode and press the service button.
  In audit mode, pressing the BET button a column of extra red values appear.
  To exit the audit mode, just press DEAL.

  Test Mode: Press F2.
  To exit pres DISCARD buttons 1, 2 & 3 simultaneously.

*/
ROM_START( comg170 )  // Cal Omega v17.0 (Amusement Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_FILL(               0x4000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "17-03.u7",   0x5000, 0x1000, CRC(56ca24db) SHA1(27793d48060b6d1ee080da27d19737a724395f35) )
	ROM_LOAD( "17-04.u8",   0x6000, 0x1000, CRC(9fcc5761) SHA1(9b1fb265219517c89af8d3700b04d9487b1db45e) )
	ROM_LOAD( "17-05.u9",   0x7000, 0x1000, CRC(cfdbd4b9) SHA1(7f7d5b0d86d009c2e1af54338d9ead24c97eb3e7) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "pkcg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pkcgc.u70",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pkcgb.u69",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pkcga.u68",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg172 )  // Cal Omega v17.2 (Double Double Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_FILL(               0x4000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "17-23.u7",   0x5000, 0x1000, CRC(96efc8d1) SHA1(96836ca7188dc53e3a8af64f1fed9fe1c0c4e056) )
	ROM_LOAD( "17-24.u8",   0x6000, 0x1000, CRC(08d31a98) SHA1(85e23ffeb8fa82ec0155f54d2193511517e6ec8c) )
	ROM_LOAD( "17-25.u9",   0x7000, 0x1000, CRC(82508c71) SHA1(dd44e949f36e04ceaf1e527615c7003d0a9c0073) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "jkrpkrcg0.u67",  0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "jkrpkrcgc.u70",  0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkrpkrcgb.u69",  0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkrpkrcga.u68",  0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "wldclr.u28", 0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

ROM_START( comg175 )  // Cal Omega v17.5 (Gaming Draw Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "17-51.u5",   0x1800, 0x0800, CRC(5450b90a) SHA1(4e67a17b2353bbe89fb6be4951efd09a948a987e) )
	ROM_LOAD( "17-52.u6",   0x2000, 0x0800, CRC(29d787fd) SHA1(3f8c46f928c7fc3e68fe47efe23505f393bdb577) )
	ROM_LOAD( "17-53.u7",   0x2800, 0x0800, CRC(080f6a23) SHA1(69866fba7cc348976ed1dfbfe103a5852a60d081) )
	ROM_LOAD( "17-54.u8",   0x3000, 0x0800, CRC(7867815d) SHA1(8f3c4375f9b7c4eb5ebe413a8ac8593ff2ceb3e0) )
	ROM_LOAD( "17-55.u9",   0x3800, 0x0800, CRC(656aa3e3) SHA1(2ccf1328d134cb26a5c4a0e45841079166de19be) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "pkcg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pkcgc.u70",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pkcgb.u69",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pkcga.u68",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg176 )  // Cal Omega 17.6 (Nudge Keno)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "17-61.u5",   0x1800, 0x0800, CRC(7b201d27) SHA1(142d25c424e6bfd4327bb796f8dc9d0e6cb21797) )
	ROM_LOAD( "17-62.u6",   0x2000, 0x0800, CRC(0ce87971) SHA1(ad1fecf4ed34eccd2a5b09e8847cb4a011875b73) )
	ROM_LOAD( "17-63.u7",   0x2800, 0x0800, CRC(adce3e97) SHA1(5c12984b241c4884158ecc0743a3805115457277) )
	ROM_LOAD( "17-64.u8",   0x3000, 0x0800, CRC(2a48525a) SHA1(9e48ca9fc3cc4c42dac3a24b2a6ac78204f7d2d8) )
	ROM_LOAD( "17-65.u9",   0x3800, 0x0800, CRC(d4d929fa) SHA1(9cee198596375b810210856bc4ba5f5a3d706df4) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "nbcg0.u67",  0x0000, 0x0800, CRC(b4b01f22) SHA1(1214d91ae28ac40ef37fce787783cd64331ed224) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "nbcgc.u70",  0x0000, 0x1000, CRC(f7ca234f) SHA1(3b2717051341c423969ed4bfb292a9f88119c2a8) )
	ROM_LOAD( "nbcgb.u69",  0x1000, 0x1000, CRC(9d409932) SHA1(d3ffca50a059278777238d206895a0d188f4ff6f) )
	ROM_LOAD( "nbcga.u68",  0x2000, 0x1000, CRC(afe1a666) SHA1(c1530700a283d18e7136754d45904930ef424bcf) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "bclr.u28",   0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
ROM_END

ROM_START( comg181 )  // Cal Omega 18.1 (Nudge Keno)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "18-12.u6",   0x2000, 0x0800, CRC(dd867180) SHA1(caf703f45dea980e84fc29d2ea0d3f4e211aaa3f) )
	ROM_LOAD( "18-13.u7",   0x2800, 0x0800, CRC(39ccbddd) SHA1(1c027957ad6a3346dd3bcc0b422d2e854c6f5439) )
	ROM_LOAD( "18-14.u8",   0x3000, 0x0800, CRC(ddf23ef1) SHA1(8340a2b0125e42602045fea2a248f1ec9e9915c0) )
	ROM_LOAD( "18-15.u9",   0x3800, 0x0800, CRC(549610b5) SHA1(371e4651a400d4c8b7181391c1d9b964d1ccb6fb) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "nbcg0.u67",  0x0000, 0x0800, CRC(b4b01f22) SHA1(1214d91ae28ac40ef37fce787783cd64331ed224) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "nbcgc.u70",  0x0000, 0x1000, CRC(f7ca234f) SHA1(3b2717051341c423969ed4bfb292a9f88119c2a8) )
	ROM_LOAD( "nbcgb.u69",  0x1000, 0x1000, CRC(9d409932) SHA1(d3ffca50a059278777238d206895a0d188f4ff6f) )
	ROM_LOAD( "nbcga.u68",  0x2000, 0x1000, CRC(afe1a666) SHA1(c1530700a283d18e7136754d45904930ef424bcf) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "bclr.u28",   0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
ROM_END

ROM_START( comg183 )  // Cal Omega v18.3 (Pixels)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "18-32.u6",   0x4000, 0x1000, CRC(c793ffc1) SHA1(f2ef82f92a9e18128d28973bcf050d0c3e1819f3) )
	ROM_LOAD( "18-33.u7",   0x5000, 0x1000, CRC(415a6599) SHA1(648986310a3864652897e5d18b8be06819cce7a8) )
	ROM_LOAD( "18-34.u8",   0x6000, 0x1000, CRC(fc5d3b89) SHA1(3601401d00d7a0621eac4254da238e9c8929cac4) )
	ROM_LOAD( "18-35.u9",   0x7000, 0x1000, CRC(6552207f) SHA1(bd8a7546b4dad07d151dc7039028916386aee5e9) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "pxcg0.u67",  0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "pxcgc.u70",  0x0000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )
	ROM_LOAD( "pxcgb.u69",  0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pxcga.u68",  0x2000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pixclr.u28", 0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
ROM_END

ROM_START( comg184 )  // Cal Omega v18.4 (Pixels)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "18-42.u6",   0x4000, 0x1000, CRC(b80f6912) SHA1(8675ebb57aa9f7d9dfa38920e31bc9d0824eeaac) )
	ROM_LOAD( "18-43.u7",   0x5000, 0x1000, CRC(bb68fabc) SHA1(35b4bd0b1a9aa432fe1e341c63d839c3834b8684) )
	ROM_LOAD( "18-44.u8",   0x6000, 0x1000, CRC(5ef0b953) SHA1(6da595404505fe50bf52bcb720ec46ca020dec97) )
	ROM_LOAD( "18-45.u9",   0x7000, 0x1000, CRC(bc56398d) SHA1(9c6e328f2c81b57d797e9db4b083d1974fa1d159) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "pxcg0.u67",  0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "pxcgc.u70",  0x0000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )
	ROM_LOAD( "pxcgb.u69",  0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pxcga.u68",  0x2000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pixclr.u28", 0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
ROM_END

ROM_START( comg185 )  // Cal Omega v18.5 (Pixels)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "18-52.u6",   0x4000, 0x1000, CRC(19225f7d) SHA1(73d713ee86886f935d9b2c2ca670d8e00d466b7f) )
	ROM_LOAD( "18-53.u7",   0x5000, 0x1000, CRC(797e2b70) SHA1(83f974c6c1886eab5c90782766b72900c73045e1) )
	ROM_LOAD( "18-54.u8",   0x6000, 0x1000, CRC(6becc802) SHA1(e72783db52bffd3fa2f7d35cd8a004415e37b004) )
	ROM_LOAD( "18-55.u9",   0x7000, 0x1000, CRC(313db76a) SHA1(d81228bf0badc8a3cf08c77de27b3d973953175c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "pxcg0.u67",  0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "pxcgc.u70",  0x0000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )
	ROM_LOAD( "pxcgb.u69",  0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pxcga.u68",  0x2000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pixclr.u28", 0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
ROM_END

ROM_START( comg186 )  // Cal Omega v18.6 (Pixels)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "18-62.u6",   0x4000, 0x1000, CRC(025f4268) SHA1(750a5417fe2b077893d8252dd5eafc42fafd965e) )
	ROM_LOAD( "18-63.u7",   0x5000, 0x1000, CRC(948a6ef1) SHA1(6aeb244209f3376042a32d9accb38e3f09cb192a) )
	ROM_LOAD( "18-64.u8",   0x6000, 0x1000, CRC(9fb6e82b) SHA1(db38564e8060f1c67183f3f412a24439b2253e13) )
	ROM_LOAD( "18-65.u9",   0x7000, 0x1000, CRC(a68be5ef) SHA1(1ab2bc1b070863260fa281970f886d77e2fa7ccd) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "pxcg0.u67",  0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "pxcgc.u70",  0x0000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )
	ROM_LOAD( "pxcgb.u69",  0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pxcga.u68",  0x2000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pixclr.u28", 0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
ROM_END

ROM_START( comg187 )  // Cal Omega v18.7 (Amusement Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "18-72.u6",   0x4000, 0x1000, BAD_DUMP CRC(1a4bd46a) SHA1(76101271ff9b98c3310e1666dfba34a01a0f0bcd) )  // 1st half seems to be the 2nd one, and 2nd half is filled of 0xff
	ROM_LOAD( "18-73.u7",   0x5000, 0x1000, CRC(ca374ecb) SHA1(113495afa88da97cb7239f645fabba7125ce2b4b) )
	ROM_LOAD( "18-74.u8",   0x6000, 0x1000, CRC(5bb57ca8) SHA1(22dc6f0e2fee5408fa70e4bc60f1833534ee038f) )
	ROM_LOAD( "18-75.u9",   0x7000, 0x1000, CRC(70a8ccb3) SHA1(b5b7d6a8262ab6e47a1400681c414fd3edd0d7a8) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "jkr2cg0.u67",    0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "jkr2cgc.u70",    0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkr2cgb.u69",    0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkr2cga.u68",    0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "mltclr.u28", 0x0000, 0x0100, CRC(fefb0fa8) SHA1(66d86aa19d9d37ffd2840d6653fcec667bc716d4) )
ROM_END

ROM_START( comg204 )  // Cal Omega v20.4 (Super Blackjack)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "20-41.u5",   0x3000, 0x1000, CRC(9c2203f7) SHA1(fd566683e887cf80cd0e6c82a413aebc378397f8) )
	ROM_LOAD( "20-42.u6",   0x4000, 0x1000, CRC(31b37010) SHA1(c35ef77725c6c6dd9f369d50d9a8e55c2e3644af) )
	ROM_LOAD( "20-43.u7",   0x5000, 0x1000, CRC(a9edf684) SHA1(7d2d8f3ec00c93914bfd629fae838b7d0f4bf414) )
	ROM_LOAD( "20-44.u8",   0x6000, 0x1000, CRC(7a46e6cd) SHA1(91be6ddfd33ebab1f65e08f58c1be46fc770c1bf) )
	ROM_LOAD( "20-45.u9",   0x7000, 0x1000, CRC(1e76202a) SHA1(015bf7ba62a561f5640aad7855e5fdd46787cdbf) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "jkr2cg0.u67",    0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "jkr2cgc.u70",    0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkr2cgb.u69",    0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkr2cga.u68",    0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "mltclr.u28", 0x0000, 0x0100, CRC(fefb0fa8) SHA1(66d86aa19d9d37ffd2840d6653fcec667bc716d4) )
ROM_END

ROM_START( comg208 )  // Cal Omega v20.8 (Winner's Choice)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "20-81.u5",   0x3000, 0x1000, CRC(b042de4d) SHA1(6de8e5ca45fe8159d30da2754fab818e84bbcc0c) )
	ROM_LOAD( "20-82.u6",   0x4000, 0x1000, CRC(7d42257c) SHA1(f1487bcc2475d1bcdfbc9bf866adcb0d510acef5) )
	ROM_LOAD( "20-83.u7",   0x5000, 0x1000, CRC(b3b25958) SHA1(096f010db3164579ba81851a3f1062df0f46fc0f) )
	ROM_LOAD( "20-84.u8",   0x6000, 0x1000, CRC(0bfb9f9d) SHA1(4a3c1e88faa90a9f4a483b053beb2c8af688d52d) )
	ROM_LOAD( "20-85.u9",   0x7000, 0x1000, CRC(d61ba385) SHA1(0352a5306e942467d142982394d2d72892d1ecd6) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mlt2cg0.u67",    0x0000, 0x1000, CRC(43a095f0) SHA1(1824dd864bfc385035cc6b3da736ab6de9336440) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "mlt2cgc.u70",    0x0000, 0x1000, CRC(432ae4f9) SHA1(e4229ee743d0245b3efaed0d852bf24712508ce9) )
	ROM_LOAD( "mlt2cgb.u69",    0x1000, 0x1000, CRC(d5173679) SHA1(396c9c3eb7a0a5e5d279d079e635c8e4e5581779) )
	ROM_LOAD( "mlt2cga.u68",    0x2000, 0x1000, CRC(b7397d3a) SHA1(f35607a4cd60e4467e27474e8063b7a7a4a65d9f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "mltclr.u28", 0x0000, 0x0100, CRC(ab8960a6) SHA1(2414b445739860a0c3e533f2992291ff8a471d76) )
ROM_END

ROM_START( comg214 )  // Cal Omega v21.4 (Wild Double-Up Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_FILL(               0x4000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "21.43.bin",  0x5000, 0x1000, CRC(9ea9f6c5) SHA1(659dd68922de7142de9050e7d6495bdabbf15abb) )

	// bad programmed eprom. It has the same contents of 21.43.bin
	// the original one should have checksum16 = AD9F (as quoted in some notes)
	ROM_LOAD( "21.44.bin",  0x6000, 0x1000, BAD_DUMP CRC(9ea9f6c5) SHA1(659dd68922de7142de9050e7d6495bdabbf15abb))

	ROM_LOAD( "21.45.bin",  0x7000, 0x1000, CRC(199b77f8) SHA1(3c669469edfd83458441659866040ed57d4c1b88) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "cg0.bin",  0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "cg2c.bin",  0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "cg2b.bin",  0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "cg2a.bin",  0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "wldclr.u28", 0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

ROM_START( comg227 )  // Cal Omega v22.7 (Amusement Poker (Double Double))
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_FILL(               0x4000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "22-73.u7",   0x5000, 0x1000, BAD_DUMP CRC(152d1ff9) SHA1(8bbfea1bae9e4fe2a2ac52507dc8dd0e33fbbd06) )
	ROM_LOAD( "22-74.u8",   0x6000, 0x1000, CRC(3af0f69d) SHA1(c34a0eab3ad4e4db310727805ba1ddc73533bfa6) )
	ROM_LOAD( "22-75.u9",   0x7000, 0x1000, CRC(d1f3fe24) SHA1(8b43b521fb1be8ef4286b4bfee99b654c49cf9de) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "jkrpkrcg0.u67",  0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "jkrpkrcgc.u70",  0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkrpkrcgb.u69",  0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkrpkrcga.u68",  0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "wldclr.u28", 0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

ROM_START( comg230 )  // Cal Omega v23.0 (FC Bingo (4-card))
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "23-02.u6",   0x4000, 0x1000, NO_DUMP )
	ROM_LOAD( "23-03.u7",   0x5000, 0x1000, BAD_DUMP CRC(daf93757) SHA1(27d57007a24a5f892f7ee201072fcd5817373cad) )
	ROM_LOAD( "23-04.u8",   0x6000, 0x1000, CRC(ebb5531a) SHA1(9fd003fcba5a5120332bcbd3c845d555c60875e9) )
	ROM_LOAD( "23-05.u9",   0x7000, 0x1000, BAD_DUMP CRC(cca254a0) SHA1(9dc7c660e0f2d5766756e4ef8cb5a7eddec18069) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "fcbcg0.u67", 0x0000, 0x1000, BAD_DUMP CRC(ded1c944) SHA1(eafb30b4a2cbb6c10dba959d3839a8f8bf793972) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "fcbcgc.u70", 0x0000, 0x1000, BAD_DUMP CRC(9f101dd8) SHA1(4f6671b8b36f9351f368d8bbc92c20bd2b7b3dab) )
	ROM_LOAD( "fcbcgb.u69", 0x1000, 0x1000, BAD_DUMP CRC(a7548075) SHA1(a751289cbc8b726082b60740c0202c08e3981e24) )
	ROM_LOAD( "fcbcga.u68", 0x2000, 0x1000, BAD_DUMP CRC(3fc39df9) SHA1(223d05f8969a1846a986b29395c98f97a3218bf7) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "fcbclr.u28", 0x0000, 0x0100, BAD_DUMP CRC(6db5a344) SHA1(5f1a81ac02a2a74252decd3bb95a5436cc943930) )
ROM_END

ROM_START( comg236 )  // Cal Omega v23.6 (Hotline)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "23-62.u6",   0x4000, 0x1000, CRC(6f3dd409) SHA1(8112c3114609317fe5543bf26fa426d36d60c5e1) )
	ROM_LOAD( "23-63.u7",   0x5000, 0x1000, CRC(76e96865) SHA1(03055751efa143cff0501aaa9b2beb9d533e13e7) )
	ROM_LOAD( "23-64.u8",   0x6000, 0x1000, CRC(26a18d82) SHA1(1ca036c014f180fb1720150642be3986c053c1c9) )
	ROM_LOAD( "23-65.u9",   0x7000, 0x1000, CRC(4e24d2d3) SHA1(56200760dced8b8bf2b38eee5c26a20ffc5b5442) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hlcg0.u67",  0x0000, 0x1000, CRC(b2871c94) SHA1(001c376479701efa4542c88f0eca9e13c694667c) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "hlcgc.u70",  0x0000, 0x1000, CRC(6af901d9) SHA1(f9fcfcf2c92d1a3ef9a18b4e3e680c9de4829533) )
	ROM_LOAD( "hlcgb.u69",  0x1000, 0x1000, CRC(db2d3eb7) SHA1(45f686edf7093069b44e895547c7ec67f820447d) )
	ROM_LOAD( "hlcga.u68",  0x2000, 0x1000, CRC(a7e583fd) SHA1(d3b0aa9e24b6aedf24af55e5b149ab75d6f01a36) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "hlclr.u28",  0x0000, 0x0100, CRC(1c994cda) SHA1(5c8698b4c5e43146106c9da8a306e3099b26ca2d) )
ROM_END

/*

  Gaming Draw Poker.

  Program roms are roms 23*.*, on the board, there is a number near each roms
  looks to be the address of the rom :

          23-91   1800
          23-92   2000
          23-93   2800
          23-94   3000
          23-9    3800

  Graphics are in roms CG*.*, there is no type indication on these rams, i hope
  i read them correctly.

  There is also 3 sets of switches on the board :

          SW1     1       300     SW2     1       OPT1
                  2       600             2       OPT2
                  3       1200            3       OPT3
                  4       2400            4       OPT4
                  5       4800            5       OPT5
                  6       9600            6       DIS
                  7       -               7       +VPOL
                  8       -               8       +HPOL

          SW3     no indications on the board

  The sound rom is missing on the board :(

*/

ROM_START( comg239 )  // Cal Omega v23.9 (Gaming Draw Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "23-91.u5",   0x1800, 0x0800, CRC(b49035e2) SHA1(b94a0245ca64d15b1496d1b272ffc0ce80f85526) )
	ROM_LOAD( "23-92.u6",   0x2000, 0x0800, CRC(d9ffaa73) SHA1(e39d10121e16f89cd8d30a5391a14dc3d4b13a46) )
	ROM_LOAD( "23-93.u7",   0x2800, 0x0800, CRC(f4e44280) SHA1(a03e5f03ed86c8ad7900fab0ef6a71c76eba3232) )
	ROM_LOAD( "23-94.u8",   0x3000, 0x0800, CRC(8372f4d0) SHA1(de289b65cbe30c92b46fa87b9262ff7f9cfa0431) )
	ROM_LOAD( "23-95.u9",   0x3800, 0x0800, CRC(bfcb934d) SHA1(b7cfa049bdd773368cb8326bcdfabbf474d15bb4) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "pkcg0.u67",  0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pkcgc.u70",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pkcgb.u69",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pkcga.u68",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "82s129n.u28",    0x0000, 0x0100, CRC(6db5a344) SHA1(5f1a81ac02a2a74252decd3bb95a5436cc943930) )
ROM_END

ROM_START( comg240 )  // Cal Omega v24.0 (Gaming Draw Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "24-01.u5",       0x1800, 0x0800, CRC(445e4e1e) SHA1(11f1b2652fce0e507bde66296f57d689a8460df5) )
	ROM_LOAD( "24-02.u6",       0x2000, 0x0800, CRC(53ef572d) SHA1(14c99f94a22d93de998f6418ea9dc3eab5119a82) )
	ROM_LOAD( "24-03.u7",       0x2800, 0x0800, CRC(fe46b112) SHA1(b32ef6a09a0b9059840cef4f8847cdbb3cf96fa9) )
	ROM_LOAD( "24-04.u8",       0x3000, 0x0800, CRC(0060978d) SHA1(f5ee296592520408d2bdd7022d5b82e712cfc643) )
	ROM_LOAD( "24-05_02bt.u9",  0x3800, 0x0800, CRC(9637a6c0) SHA1(29589fcf8c4db760da95d29be74340fea65b550a) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "cgo.u67",    0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "cgc.u70",    0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "cgb.u69",    0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "cga.u68",    0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x100, "proms", 0 )  // is this prom ok?
	ROM_LOAD( "pok-6301.u28",   0x0000, 0x0100, CRC(56c2577b) SHA1(cb75882067e1e0d9f9369a37b5a829dd091d473e) )
ROM_END

ROM_START( comg246 )  // Cal Omega v24.6 (Hotline)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket
	ROM_LOAD( "24-62.u6",   0x4000, 0x1000, CRC(41f7b882) SHA1(4b532d70d5a7101952085a8fcdc0568c4266a72a) )
	ROM_LOAD( "24-63.u7",   0x5000, 0x1000, CRC(226580b7) SHA1(07e6332ace45e0ced57aed6e348ab12c1f07ff34) )
	ROM_LOAD( "24-64.u8",   0x6000, 0x1000, CRC(bf402e32) SHA1(436670b8f37caac14bb578a31dddbc0e2b0fd1ae) )
	ROM_LOAD( "24-65.u9",   0x7000, 0x1000, CRC(4eb99af3) SHA1(1af1a27f2575290aa0dce88840cd8be516a542ff) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hlcg0.u67",  0x0000, 0x1000, CRC(b2871c94) SHA1(001c376479701efa4542c88f0eca9e13c694667c) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "hlcgc.u70",  0x0000, 0x1000, CRC(6af901d9) SHA1(f9fcfcf2c92d1a3ef9a18b4e3e680c9de4829533) )
	ROM_LOAD( "hlcgb.u69",  0x1000, 0x1000, CRC(db2d3eb7) SHA1(45f686edf7093069b44e895547c7ec67f820447d) )
	ROM_LOAD( "hlcga.u68",  0x2000, 0x1000, CRC(a7e583fd) SHA1(d3b0aa9e24b6aedf24af55e5b149ab75d6f01a36) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "hlclr.u28",  0x0000, 0x0100, CRC(1c994cda) SHA1(5c8698b4c5e43146106c9da8a306e3099b26ca2d) )
ROM_END

ROM_START( comg272a )  // Cal Omega v27.2 (Keno (amusement))
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "27-22.u6",   0x2000, 0x0800, CRC(db3e1918) SHA1(4b8f33103f093ddbe750b536abc4545cf262d2e5) )
	ROM_LOAD( "27-23.u7",   0x2800, 0x0800, CRC(c9e9cfd8) SHA1(dd0615ac579331330bda070f9ed68d7972436781) )
	ROM_LOAD( "27-24.u8",   0x3000, 0x0800, CRC(d3fbab7f) SHA1(805510356bd52cf698a838daeaf16096eedcfd37) )
	ROM_LOAD( "27-25.u9",   0x3800, 0x0800, CRC(22dc5b5e) SHA1(4442b7ac0516fdad8c81687fa9683eeace5b6c2d) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x0800, 0xff )  // empty socket (requested by the manual)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "kcgc.u70",   0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kcgb.u69",   0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kcga.u68",   0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )

	ROM_REGION(0x1000, "kstec", 0)
	ROM_LOAD( "903kstec.0", 0x0000, 0x0800, CRC(c1636ab5) SHA1(5a3ad24918751ca6a6640807e421e80f6b4cc844) )
ROM_END

ROM_START( comg272b )  // Cal Omega v27.2 (Keno (gaming))
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "27-22m.u6",  0x2000, 0x0800, CRC(6dc6ec9c) SHA1(dd80d0e544ad51d1b950970e735d7ce1e05062e3) )
	ROM_LOAD( "27-23m.u7",  0x2800, 0x0800, CRC(242ce2ed) SHA1(e114cec7eb7554de14561e7c0e6aed01d7e72ca6) )
	ROM_LOAD( "27-24m.u8",  0x3000, 0x0800, CRC(e191b0e1) SHA1(cc476efa194c1b2cd0035e9b4725e81d3a6f381c) )
	ROM_LOAD( "27-25m.u9",  0x3800, 0x0800, CRC(4152b53d) SHA1(6fe577045d03362b8b988c7a9cd0542c9a20d7a7) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x0800, 0xff )  // empty socket (requested by the manual)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "kcgc.u70",   0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kcgb.u69",   0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kcga.u68",   0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "pokclr.u28", 0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )

	ROM_REGION(0x1000, "kstec", 0)
	ROM_LOAD( "903kstec.0", 0x0000, 0x0800, CRC(c1636ab5) SHA1(5a3ad24918751ca6a6640807e421e80f6b4cc844) )
ROM_END


/*
  CEI Video Poker
  Jacks or Better
  V 51.08
  Pay Schedule 05F Controled by EPR1- 50.081
  906-III board
  PROMS 2764

*/
ROM_START( comg5108 )  // Cal Omega v51.08 (Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1_s05f.u28",  0x6000, 0x2000, CRC(3d6abca9) SHA1(54a802f89bd64380abf269a2b507513c8db5319b) )  // schedule eprom 05F. checked in offset $8034
	ROM_LOAD( "epr2_5008.u29",  0x8000, 0x2000, CRC(72cf8376) SHA1(fa1682244402e1b36164c670241f585bf4017ad9) )
	ROM_LOAD( "epr3_5008.u30",  0xa000, 0x2000, CRC(c79957e5) SHA1(64afdedf5369d56790e9ae7a8d3be5f52125ca1f) )
	ROM_LOAD( "epr4_5008.u31",  0xc000, 0x2000, CRC(eb0b0a86) SHA1(4fd29700db8fe183392cc66a54a128657c7e05e0) )
	ROM_LOAD( "epr5_5008.u32",  0xe000, 0x2000, CRC(b0981cea) SHA1(e56278176df1a6c20acfb8cbf2f5e7a946d93111) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "cg2c.u4",        0x0000, 0x2000, CRC(dc77a6db) SHA1(3af5f568de3f2af1a6bbb00d673bdbff16c87a40) )
	ROM_LOAD( "cg2b.u5",        0x2000, 0x2000, CRC(1f79f76d) SHA1(b2bce60e24dd61977f7bf6ee4705ca7d104ab388) )
	ROM_LOAD( "cg2a.u6",        0x4000, 0x2000, CRC(d5fd9fc2) SHA1(68472e7271f835656197109620bb3988fc52308a) )

	ROM_REGION( 0x200, "proms", 0 )  // second half is empty
	ROM_LOAD( "bprom.u16",      0x0000, 0x0200, CRC(a6d43709) SHA1(cbff2cb60137462dc0b7c7719a64574218d96c62) )
ROM_END

ROM_START( comg5108a )  // Cal Omega v51.08a (Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1_s09f.u28",  0x6000, 0x2000, CRC(6eccf229) SHA1(e3185d4f3848ece90bfbe6b0aac95e62c8b39faf) )  // schedule eprom 09F. checked in offset $8034
	ROM_LOAD( "epr2_5008.u29",  0x8000, 0x2000, CRC(72cf8376) SHA1(fa1682244402e1b36164c670241f585bf4017ad9) )
	ROM_LOAD( "epr3_5008.u30",  0xa000, 0x2000, CRC(c79957e5) SHA1(64afdedf5369d56790e9ae7a8d3be5f52125ca1f) )
	ROM_LOAD( "epr4_5008.u31",  0xc000, 0x2000, CRC(eb0b0a86) SHA1(4fd29700db8fe183392cc66a54a128657c7e05e0) )
	ROM_LOAD( "epr5_5008.u32",  0xe000, 0x2000, CRC(b0981cea) SHA1(e56278176df1a6c20acfb8cbf2f5e7a946d93111) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "cg2c.u4",        0x0000, 0x2000, CRC(dc77a6db) SHA1(3af5f568de3f2af1a6bbb00d673bdbff16c87a40) )
	ROM_LOAD( "cg2b.u5",        0x2000, 0x2000, CRC(1f79f76d) SHA1(b2bce60e24dd61977f7bf6ee4705ca7d104ab388) )
	ROM_LOAD( "cg2a.u6",        0x4000, 0x2000, CRC(d5fd9fc2) SHA1(68472e7271f835656197109620bb3988fc52308a) )

	ROM_REGION( 0x200, "proms", 0 )  // second half is empty
	ROM_LOAD( "bprom.u16",      0x0000, 0x0200, CRC(a6d43709) SHA1(cbff2cb60137462dc0b7c7719a64574218d96c62) )
ROM_END

ROM_START( comg5108b )  // Cal Omega v51.08b (Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1_s21f.u28",  0x6000, 0x2000, CRC(33b1bb5b) SHA1(86f439ef96f7dafe320c394878ff4fda3f94c631) )  // schedule eprom 21F. checked in offset $8034
	ROM_LOAD( "epr2_5008.u29",  0x8000, 0x2000, CRC(72cf8376) SHA1(fa1682244402e1b36164c670241f585bf4017ad9) )
	ROM_LOAD( "epr3_5008.u30",  0xa000, 0x2000, CRC(c79957e5) SHA1(64afdedf5369d56790e9ae7a8d3be5f52125ca1f) )
	ROM_LOAD( "epr4_5008.u31",  0xc000, 0x2000, CRC(eb0b0a86) SHA1(4fd29700db8fe183392cc66a54a128657c7e05e0) )
	ROM_LOAD( "epr5_5008.u32",  0xe000, 0x2000, CRC(b0981cea) SHA1(e56278176df1a6c20acfb8cbf2f5e7a946d93111) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "cg2c.u4",        0x0000, 0x2000, CRC(dc77a6db) SHA1(3af5f568de3f2af1a6bbb00d673bdbff16c87a40) )
	ROM_LOAD( "cg2b.u5",        0x2000, 0x2000, CRC(1f79f76d) SHA1(b2bce60e24dd61977f7bf6ee4705ca7d104ab388) )
	ROM_LOAD( "cg2a.u6",        0x4000, 0x2000, CRC(d5fd9fc2) SHA1(68472e7271f835656197109620bb3988fc52308a) )

	ROM_REGION( 0x200, "proms", 0 )  // second half is empty
	ROM_LOAD( "bprom.u16",     0x0000, 0x0200, CRC(a6d43709) SHA1(cbff2cb60137462dc0b7c7719a64574218d96c62) )
ROM_END

ROM_START( comg5107 )  // Cal Omega v51.07 (Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1_s05f.u28",  0x6000, 0x2000, CRC(3d6abca9) SHA1(54a802f89bd64380abf269a2b507513c8db5319b) )  // schedule eprom 05F
	ROM_LOAD( "epr2_5007.u29",  0x8000, 0x2000, CRC(1cefe40d) SHA1(9169c466903485d6e78c53bd104dc61722437509) )
	ROM_LOAD( "epr3_5007.u30",  0xa000, 0x2000, CRC(0bc9be3e) SHA1(bce88a6a37ded6396954ec8c11e19af66f658e4d) )
	ROM_LOAD( "epr4_5007.u31",  0xc000, 0x2000, CRC(34e7d16f) SHA1(c4901e00a5dbaeb6277dbe9bdcad93827c5aa947) )
	ROM_LOAD( "epr5_5007.u32",  0xe000, 0x2000, CRC(e5a252b5) SHA1(7a226c771ba16944b4e50c22db0cd5d7fc530e11) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "cg2c.u4",        0x0000, 0x2000, CRC(dc77a6db) SHA1(3af5f568de3f2af1a6bbb00d673bdbff16c87a40) )
	ROM_LOAD( "cg2b.u5",        0x2000, 0x2000, CRC(1f79f76d) SHA1(b2bce60e24dd61977f7bf6ee4705ca7d104ab388) )
	ROM_LOAD( "cg2a.u6",        0x4000, 0x2000, CRC(d5fd9fc2) SHA1(68472e7271f835656197109620bb3988fc52308a) )

	ROM_REGION( 0x200, "proms", 0 )  // second half is empty
	ROM_LOAD( "bprom.u16",      0x0000, 0x0200, CRC(a6d43709) SHA1(cbff2cb60137462dc0b7c7719a64574218d96c62) )
ROM_END

/*
  CEI 906III POKER
  V60.04  1989  906III board

  Jumper 8,9
  EPR 1-5 2764
  CG's    2764
  Labeled UCMC

  EPR1   075B   E3185D4F3848ECE90BFBE6B0AAC95E62C8B39FAF
  EPR2   442A   474B6B2CC592518836C1D3804FE77CAE505FCC97
  EPR3   E8E9   AE259FCFA34B20F0C9C71CA48275426F2E9CFFBE
  EPR4   65A3   B8A16A8B8BA245901D4F347250E0DCB7E6D36E0A
  EPR5   3D40   53D33F3B251D27F6EC40E3E3B059EA7BA8A2EFE0

  CG2A   961D   68472E7271F835656197109620BB3988FC52308A
  CG2B   B319   B2BCE60E24DD61977F7BF6EE4705CA7D104AB388
  CG2C   DF06   3AF5F568DE3F2AF1A6BBB00D673BDBFF16C87A40
  Color  076A   CBFF2CB60137462DC0B7C7719A64574218D96C62

*/
ROM_START( comg6004 )  // Cal Omega v60.04 (Poker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1_s09f.u28",  0x6000, 0x2000, CRC(6eccf229) SHA1(e3185d4f3848ece90bfbe6b0aac95e62c8b39faf) )  // schedule eprom 09F
	ROM_LOAD( "epr2_6004.u29",  0x8000, 0x2000, CRC(9f5af552) SHA1(474b6b2cc592518836c1d3804fe77cae505fcc97) )
	ROM_LOAD( "epr3_6004.u30",  0xa000, 0x2000, CRC(98799fa0) SHA1(ae259fcfa34b20f0c9c71ca48275426f2e9cffbe) )
	ROM_LOAD( "epr4_6004.u31",  0xc000, 0x2000, CRC(eff6fbb3) SHA1(b8a16a8b8ba245901d4f347250e0dcb7e6d36e0a) )
	ROM_LOAD( "epr5_6004.u32",  0xe000, 0x2000, CRC(fc4b7c2f) SHA1(53d33f3b251d27f6ec40e3e3b059ea7ba8a2efe0) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "cg2c.u4",        0x0000, 0x2000, CRC(dc77a6db) SHA1(3af5f568de3f2af1a6bbb00d673bdbff16c87a40) )
	ROM_LOAD( "cg2b.u5",        0x2000, 0x2000, CRC(1f79f76d) SHA1(b2bce60e24dd61977f7bf6ee4705ca7d104ab388) )
	ROM_LOAD( "cg2a.u6",        0x4000, 0x2000, CRC(d5fd9fc2) SHA1(68472e7271f835656197109620bb3988fc52308a) )

	ROM_REGION( 0x200, "proms", 0 )  // second half is empty
	ROM_LOAD( "bprom.u16",      0x0000, 0x0200, CRC(a6d43709) SHA1(cbff2cb60137462dc0b7c7719a64574218d96c62) )
ROM_END


/*********************** Diagnostic PROMs ***********************/

ROM_START( comg903d )  // Cal Omega 903d (System 903 diag.PROM)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x1800, 0x0800, 0xff )  // empty socket (requested by the manual)
	ROM_FILL(               0x2000, 0x0800, 0xff )  // empty socket (requested by the manual)
	ROM_FILL(               0x2800, 0x0800, 0xff )  // empty socket (requested by the manual)
	ROM_FILL(               0x3000, 0x0800, 0xff )  // empty socket (requested by the manual)
	ROM_LOAD( "903diag.u9", 0x3800, 0x0800, CRC(f8092cea) SHA1(0c864419a4e1956c030b185739eca59313f20e8a) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "testcg0.u67",    0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_FILL(                   0x0000, 0x1800, 0xff )  // removed all ROMs (requested by the manual)

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "testclr.u28",    0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END

ROM_START( comg905d )  // Cal Omega 905d (System 905 diag.PROM)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(               0x3000, 0x1000, 0xff )  // empty socket (requested by the manual)
	ROM_FILL(               0x4000, 0x1000, 0xff )  // empty socket (requested by the manual)
	ROM_FILL(               0x5000, 0x1000, 0xff )  // empty socket (requested by the manual)
	ROM_FILL(               0x6000, 0x1000, 0xff )  // empty socket (requested by the manual)
	ROM_LOAD( "905diag.u9", 0x7000, 0x1000, CRC(6c20dbc7) SHA1(dbab0d2cf07bade2e3619bd5f29d406f3498a278) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "testcg0.u67",    0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_FILL(                   0x0000, 0x1800, 0xff )  // removed all ROMs (requested by the manual)

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "testclr.u28",    0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
ROM_END



/****************** Unofficial / 3rd party games *****************/

/*

  El Grande 5 Card Draw

  ROM text showed poker stuff and "TUNI" "1982"

  .u6    2716
  .u7    2516
  .u8    2516
  .u9    2516
  .u67   2516
  .u68   2516
  .u69   2716
  .u70   2716
  .u28   82s129

  6502
  HD46505
  AY-3-8912
  MC6821P x2
  TC5501  x2
  10MHz Crystal

  empty socket at u5

*/

ROM_START( elgrande )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(           0x1800, 0x0800, 0xff )  // empty socket
	ROM_LOAD( "d1.u6",  0x2000, 0x0800, CRC(8b6b505c) SHA1(5f89bb1b50b9dfacf23c50e3016b9258b0e15084) )
	ROM_LOAD( "d1.u7",  0x2800, 0x0800, CRC(d803a978) SHA1(682b73c968ef57007397d3e5eb0e78a97722da5e) )
	ROM_LOAD( "d1.u8",  0x3000, 0x0800, CRC(291fa93b) SHA1(1d57f736b11ddc916effde78e2cd08c313a62901) )
	ROM_LOAD( "d1.u9",  0x3800, 0x0800, CRC(ec3309a7) SHA1(b8ab7f3f2edf2658ea633b2b557ea37517615399) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "d1.u67", 0x0000, 0x0800, CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "d1.u70", 0x0000, 0x0800, CRC(4f12d424) SHA1(c43f1df757ac7dd76875245e73d47451d1f7f6f2) )
	ROM_LOAD( "d1.u69", 0x0800, 0x0800, CRC(ed3c83b7) SHA1(93e2134de3d9f79a6cff0391c1a32fccd3840c3f) )
	ROM_LOAD( "d1.u68", 0x1000, 0x0800, CRC(81d07f12) SHA1(c14226f8bc1d08fcdfc5cb71fcaf6e070fa2d4a8) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "d1.u28", 0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

ROM_START( jjpoker )  // tuni-83
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tuni-83.u5", 0x1800, 0x0800, CRC(46c542ee) SHA1(0b3832d8ab69427298de03d18984a220a9a35c30) )
	ROM_LOAD( "tuni-83.u6", 0x2000, 0x0800, CRC(e24b392a) SHA1(3b705b5cc60d2b33375f52958b72e70ce36fa216) )
	ROM_LOAD( "tuni-83.u7", 0x2800, 0x0800, CRC(c49e4e37) SHA1(d70321e87e6192bc65950153c13c049a52000148) )
	ROM_LOAD( "tuni-83.u8", 0x3000, 0x0800, CRC(7a20d8ff) SHA1(d9c08c5dd5fe8d9f67d4fe031b47c3dbfcc717c1) )
	ROM_LOAD( "tuni-83.u9", 0x3800, 0x0800, CRC(8cba33a6) SHA1(6998a47521f8d40425f68444bb9a45df72276c6b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tuni-83.u67",    0x0000, 0x0800, CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "tuni-83.u70",    0x0000, 0x0800, CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69",    0x0800, 0x0800, CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68",    0x1000, 0x0800, CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "tunipoker.u28",  0x0000, 0x0100, CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
ROM_END

ROM_START( jjpokerb )  // pokr_j
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pokr_j.01.u5",   0x1800, 0x0800, CRC(d0004eda) SHA1(2fd39213e3028066fd4f9b8db206f036b566e2f7) )
	ROM_LOAD( "pokr_j.02.u6",   0x2000, 0x0800, CRC(6809ccd9) SHA1(2573194a13ddf0270bccd456bda84b822036c660) )
	ROM_LOAD( "pokr_j.03.u7",   0x2800, 0x0800, CRC(20c4519a) SHA1(c58662a6404060c2d1e60953a0bf69fee5657694) )
	ROM_LOAD( "pokr_j.04.u8",   0x3000, 0x0800, CRC(d127fcbf) SHA1(ebf8ab47233c2cfe54cb8e36ec2ff7bf5f8631a5) )
	ROM_LOAD( "pokr_j.05.u9",   0x3800, 0x0800, CRC(bb376541) SHA1(1b4f91cef36d6decd2b1da9765af4e5ec976b2fc) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tuni-83.u67",    0x0000, 0x0800, CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "tuni-83.u70",    0x0000, 0x0800, CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69",    0x0800, 0x0800, CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68",    0x1000, 0x0800, CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "tunipoker.u28",  0x0000, 0x0100, CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
ROM_END

ROM_START( ssipkr24 )  // pokr02_4 (gfx and prom from jjpoker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pokr02_4.01.u5", 0x1800, 0x0800, CRC(8adf1d6d) SHA1(d83677eed9426841767d947919f6da671b5fbed4) )
	ROM_LOAD( "pokr02_4.02.u6", 0x2000, 0x0800, CRC(5298a01c) SHA1(a0085498699bc15cc6ada9e4e9541bd84b97eeae) )
	ROM_LOAD( "pokr02_4.03.u7", 0x2800, 0x0800, CRC(30b5ead6) SHA1(7650ebb5f17eae17f3a0ddee67432a7f9dbf1c13) )
	ROM_LOAD( "pokr02_4.04.u8", 0x3000, 0x0800, CRC(ade57860) SHA1(ee80e97302a4d6371fde5bacb58747075976f617) )
	ROM_LOAD( "pokr02_4.05.u9", 0x3800, 0x0800, CRC(ad15250b) SHA1(d006657df1d2e01e33a3efb906e4532a2cd5b85d) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tuni-83.u67",    0x0000, 0x0800, BAD_DUMP CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "tuni-83.u70",    0x0000, 0x0800, BAD_DUMP CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69",    0x0800, 0x0800, BAD_DUMP CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68",    0x1000, 0x0800, BAD_DUMP CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "tunipoker.u28",  0x0000, 0x0100, BAD_DUMP CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
ROM_END

ROM_START( ssipkr30 )  // pokr03_0 (gfx and prom from jjpoker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pokr03_0.01.u5", 0x1800, 0x0800, CRC(db9581fe) SHA1(605b254e0ebb96423eb522ce75242083d70f01ca) )
	ROM_LOAD( "pokr03_0.02.u6", 0x2000, 0x0800, CRC(861243ad) SHA1(290eba5c820177669e5adeac1e2f172b73789542) )
	ROM_LOAD( "pokr03_0.03.u7", 0x2800, 0x0800, CRC(b5154dc0) SHA1(3b517a8d82db62be68427378fe92f0225174ba00) )
	ROM_LOAD( "pokr03_0.04.u8", 0x3000, 0x0800, CRC(93bdef84) SHA1(3ea10a4d4d164f63082fbff4ae9b9fd14b5bcb27) )
	ROM_LOAD( "pokr03_0.05.u9", 0x3800, 0x0800, CRC(bd2ffd49) SHA1(b60c3866b11acd5053ec6dc5c37c0e322cb29478) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tuni-83.u67",    0x0000, 0x0800, BAD_DUMP CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "tuni-83.u70",    0x0000, 0x0800, BAD_DUMP CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69",    0x0800, 0x0800, BAD_DUMP CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68",    0x1000, 0x0800, BAD_DUMP CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "tunipoker.u28",  0x0000, 0x0100, BAD_DUMP CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
ROM_END

ROM_START( ssipkr40 )  // (gfx and prom from jjpoker)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "40-1.903.u5",    0x1800, 0x0800, CRC(461eb68c) SHA1(54781670930c723c993ca9ad80e06e38ddd2f035) )
	ROM_LOAD( "40-2.903.u6",    0x2000, 0x0800, CRC(099094a9) SHA1(c5a6ccb5ec0bebc79ef0b9c98595ef87c65ce361) )
	ROM_LOAD( "40-3.903.u7",    0x2800, 0x0800, CRC(1c923554) SHA1(d0050a8833f9a1a5fa0598b06a7bb265f0e814e4) )
	ROM_LOAD( "40-4.903.u8",    0x3000, 0x0800, CRC(552bf73d) SHA1(bf9197aab029c8dfaac88abcbda57547845323da) )
	ROM_LOAD( "40-5.903.u9",    0x3800, 0x0800, CRC(4d388d13) SHA1(8d46d6c227fe22f0433f02909b172f60cada1dd4) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tuni-83.u67",    0x0000, 0x0800, BAD_DUMP CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "tuni-83.u70",    0x0000, 0x0800, BAD_DUMP CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69",    0x0800, 0x0800, BAD_DUMP CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68",    0x1000, 0x0800, BAD_DUMP CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "tunipoker.u28",  0x0000, 0x0100, BAD_DUMP CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
ROM_END

/*
  Casino 21
  UMCM / IWC
  Ver 30.08

  Black Jack game. Spanish language.
  It seems to run in 906-III hardware

*/
ROM_START( cas21iwc )  // UCMC Casino 21
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x6000, 0x2000, CRC(c6c60700) SHA1(8c613211a22f5c23a9971092b996d29a4e0ae83d) )  // just tables and data to be checked.
	ROM_LOAD( "2.bin",   0x8000, 0x2000, CRC(c7bc884e) SHA1(e3a610362dc04d977b8bbd8549b9ce26a716867f) )
	ROM_LOAD( "3.bin",   0xa000, 0x2000, CRC(2be52026) SHA1(8318b340e2a5b6127c459d140078d105f16d8f32) )
	ROM_LOAD( "4.bin",   0xc000, 0x2000, CRC(0289aeac) SHA1(44bbbf4703c5dee460915e44839a53b97797f09d) )
	ROM_LOAD( "5.bin",   0xe000, 0x2000, CRC(e53b0112) SHA1(cff4d2467a6e3d322cdf5ddd9034099c8a0b7a8c) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "cgc.bin",    0x0000, 0x2000, CRC(c69ab916) SHA1(316fabf35184007b29da91b3fec0a5c4ce8cded3) )
	ROM_LOAD( "cgb.bin",    0x2000, 0x2000, CRC(62630d24) SHA1(88e89b41cae02b34767fac0a75506ee44f033422) )
	ROM_LOAD( "cga.bin",    0x4000, 0x2000, CRC(ce444eb6) SHA1(3d8c6f047c141931e55ff9101832fa5cae73576d) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s129n.u16",  0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

/*
  Poker
  UCMC / IWC
  Ver 162.03

  Poker game. Spanish language.
  It seems to run in 906-III hardware

*/
ROM_START( pokeriwc )  // UCMC Poker (ver 162.03)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "numeral_1.bin",   0x6000, 0x2000, CRC(c099dc4c) SHA1(4eec1fb0d8f1d2b31ac397991594e02bd9156348) )  // just tables and data to be checked.
	ROM_LOAD( "numeral_2.bin",   0x8000, 0x2000, CRC(20d42fb2) SHA1(aae6704719b80f3c8c994d91ceebaae070d1153f) )
	ROM_LOAD( "numeral_3.bin",   0xa000, 0x2000, CRC(2cea3ba1) SHA1(c0edc7e9102744362bdca63a75c7ba344cbb0e6c) )
	ROM_LOAD( "numeral_4.bin",   0xc000, 0x2000, CRC(742676b0) SHA1(28f565fe75120000e049fb099bced4297a0bab1f) )
	ROM_LOAD( "numeral_5.bin",   0xe000, 0x2000, CRC(d9b7c85c) SHA1(bec03c0cef38d0d4a93a8ef4c4ddbed0b67ebaf3) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "cgc.bin",    0x0000, 0x2000, CRC(c69ab916) SHA1(316fabf35184007b29da91b3fec0a5c4ce8cded3) )
	ROM_LOAD( "cgb.bin",    0x2000, 0x2000, CRC(62630d24) SHA1(88e89b41cae02b34767fac0a75506ee44f033422) )
	ROM_LOAD( "cga.bin",    0x4000, 0x2000, CRC(ce444eb6) SHA1(3d8c6f047c141931e55ff9101832fa5cae73576d) )

	ROM_REGION( 0x200, "proms", 0 )  // borrowed from other set, upper half is empty
	ROM_LOAD( "n82s129n.u16",  0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END

/*
  Poker
  UCMC / IWC
  Ver 162  20-6-91

  Poker game. Spanish language.
  It seems to run in 906-III hardware

*/
ROM_START( pokiwc162 )  // UCMC Poker (ver 162.03) alt. Joker Wild?...
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "j_k.u28",                              0x6000, 0x2000, CRC(8a6e93b0) SHA1(a411cb4cd3980934cbc9b170b118348a9a4ac896) )  // just tables and data to be checked.
	ROM_LOAD( "v162_n2_spanish_pok_20-6-91_iwc.u29",  0x8000, 0x2000, CRC(20d42fb2) SHA1(aae6704719b80f3c8c994d91ceebaae070d1153f) )
	ROM_LOAD( "v162_n3_spanish_pok_20-6-91_iwc.u30",  0xa000, 0x2000, CRC(2cea3ba1) SHA1(c0edc7e9102744362bdca63a75c7ba344cbb0e6c) )
	ROM_LOAD( "v162_n4_spanish_pok_20-6-91_iwc.u31",  0xc000, 0x2000, CRC(742676b0) SHA1(28f565fe75120000e049fb099bced4297a0bab1f) )
	ROM_LOAD( "v162_n5_spanish_pok_20-6-91_iwc.u32",  0xe000, 0x2000, CRC(d9b7c85c) SHA1(bec03c0cef38d0d4a93a8ef4c4ddbed0b67ebaf3) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "v162_cgc_iwc_argentina.u4",  0x0000, 0x2000, CRC(c69ab916) SHA1(316fabf35184007b29da91b3fec0a5c4ce8cded3) )
	ROM_LOAD( "v162_cgb_iwc_argentina.u5",  0x2000, 0x2000, CRC(62630d24) SHA1(88e89b41cae02b34767fac0a75506ee44f033422) )
	ROM_LOAD( "v162_cga_iwc_argentina.u6",  0x4000, 0x2000, CRC(ce444eb6) SHA1(3d8c6f047c141931e55ff9101832fa5cae73576d) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "n82s129n.u16",  0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
ROM_END


/*************************************************
*                  Driver Init                   *
*************************************************/

void calomega_state::init_comg079()
{
	uint8_t *PRGROM = memregion( "maincpu" )->base();

	PRGROM[0x2ad2] = 0x00;  // checksum flag (0xe0)
	PRGROM[0x2CD1] = 0x33;
	PRGROM[0x2CD3] = 0x5A;
	PRGROM[0x2CD5] = 0x76;
	PRGROM[0x2CD9] = 0x13;
	PRGROM[0x2CDB] = 0x30;
	PRGROM[0x2CDD] = 0x06;
	PRGROM[0x2D01] = 0x10;
	PRGROM[0x2D06] = 0x36;
	PRGROM[0x2D07] = 0x20;
	PRGROM[0x2D08] = 0x7D;
	PRGROM[0x2D09] = 0x3D;
	PRGROM[0x2D0A] = 0x60;
	PRGROM[0x2D30] = 0x26;
	PRGROM[0x2D31] = 0x00;
	PRGROM[0x2D33] = 0x00;
	PRGROM[0x2D35] = 0x25;
	PRGROM[0x2D36] = 0x00;
	PRGROM[0x2D37] = 0x4C;
	PRGROM[0x2D39] = 0x34;
	PRGROM[0x2D3B] = 0x00;
	PRGROM[0x2D3C] = 0x38;
	PRGROM[0x2D3E] = 0x40;
	PRGROM[0x2D3F] = 0x48;
	PRGROM[0x2D41] = 0x01;
	PRGROM[0x2D44] = 0x48;
	PRGROM[0x2D46] = 0x02;
	PRGROM[0x2D48] = 0x0F;
	PRGROM[0x2D4A] = 0x0B;
	PRGROM[0x2D4C] = 0x02;
	PRGROM[0x2D4F] = 0x01;
	PRGROM[0x2D50] = 0x68;
	PRGROM[0x2D52] = 0x00;
	PRGROM[0x2D53] = 0x4C;
	PRGROM[0x2D54] = 0x3A;
	PRGROM[0x2D55] = 0x2D;
	PRGROM[0x2D56] = 0x68;
	PRGROM[0x2D57] = 0x68;
	PRGROM[0x2D58] = 0x60;
	PRGROM[0x2D5A] = 0x00;
	PRGROM[0x2D5C] = 0x06;
	PRGROM[0x2D5E] = 0x01;
	PRGROM[0x2D60] = 0x02;
	PRGROM[0x2D62] = 0x02;
	PRGROM[0x2D63] = 0x20;
	PRGROM[0x2D64] = 0x3A;
	PRGROM[0x2D65] = 0x2D;
	PRGROM[0x2D66] = 0x60;
	PRGROM[0x2D68] = 0x00;
	PRGROM[0x2D6A] = 0x08;
	PRGROM[0x2D6C] = 0x01;
	PRGROM[0x2D6E] = 0x02;
	PRGROM[0x2D79] = 0x0A;
	PRGROM[0x2D7B] = 0x06;
	PRGROM[0x2D80] = 0x78;
	PRGROM[0x2D81] = 0x2D;
	PRGROM[0x2D82] = 0x09;
	PRGROM[0x2D84] = 0x60;
	PRGROM[0x2D86] = 0x00;
	PRGROM[0x2D87] = 0xBE;
	PRGROM[0x2D88] = 0x97;
	PRGROM[0x2D89] = 0x2D;
	PRGROM[0x2D8C] = 0x97;
	PRGROM[0x2D8F] = 0x9D;
	PRGROM[0x2D90] = 0x00;
	PRGROM[0x2D98] = 0x04;
	PRGROM[0x2D9C] = 0x00;
	PRGROM[0x2DAC] = 0x04;
	PRGROM[0x2DB0] = 0x04;
	PRGROM[0x2DB4] = 0x00;
	PRGROM[0x2DB6] = 0x0F;
	PRGROM[0x2DB8] = 0x04;
	PRGROM[0x2DBC] = 0x04;
	PRGROM[0x2DD1] = 0x20;
	PRGROM[0x2DD3] = 0x00;
	PRGROM[0x2DD5] = 0x00;
	PRGROM[0x2DD6] = 0x10;
	PRGROM[0x2DD7] = 0x99;
	PRGROM[0x2DD9] = 0x11;
	PRGROM[0x2DDB] = 0x00;
	PRGROM[0x2DDD] = 0x99;
	PRGROM[0x2DDE] = 0x00;
	PRGROM[0x2DE3] = 0x8A;
	PRGROM[0x2DE5] = 0x00;
	PRGROM[0x2DE6] = 0x99;
	PRGROM[0x2DE7] = 0x00;
	PRGROM[0x2DE8] = 0x14;
	PRGROM[0x2DE9] = 0x99;
	PRGROM[0x2DEA] = 0x00;
	PRGROM[0x2DEB] = 0x15;
	PRGROM[0x2DED] = 0x00;
	PRGROM[0x2DEE] = 0x16;
	PRGROM[0x2DEF] = 0x99;
	PRGROM[0x2E01] = 0x90;
	PRGROM[0x2E03] = 0x45;
	PRGROM[0x2E05] = 0x49;
	PRGROM[0x2E06] = 0x01;
	PRGROM[0x2E07] = 0x4A;
	PRGROM[0x2E09] = 0x90;
	PRGROM[0x2E0B] = 0x91;
	PRGROM[0x2E0D] = 0x92;
	PRGROM[0x2E0F] = 0x93;
	PRGROM[0x2E30] = 0xE0;
	PRGROM[0x2E31] = 0x12;
	PRGROM[0x2E32] = 0x90;
	PRGROM[0x2E34] = 0xAD;
	PRGROM[0x2E35] = 0xC4;
	PRGROM[0x2E36] = 0x08;
	PRGROM[0x2E37] = 0x29;
	PRGROM[0x2E38] = 0x80;
	PRGROM[0x2E39] = 0xD0;
	PRGROM[0x2E3A] = 0x13;
	PRGROM[0x2E3B] = 0xA0;
	PRGROM[0x2E3C] = 0x00;
	PRGROM[0x2E3E] = 0x04;
	PRGROM[0x2E67] = 0x2E;
	PRGROM[0x2E68] = 0x8D;
	PRGROM[0x2E69] = 0x08;
	PRGROM[0x2E6A] = 0x00;
	PRGROM[0x2E6E] = 0x8D;
	PRGROM[0x2E6F] = 0x09;
}

void calomega_state::init_comg080()
{
	/* Injecting missing Start and NMI vectors...
	   Start = $2042;  NMI = $26f8;
	   Also a fake vector at $3ff8-$3ff9. The code checks these values to continue.
	*/
	uint8_t *PRGROM = memregion( "maincpu" )->base();

	PRGROM[0x3ff8] = 0x8e;  // checked by code
	PRGROM[0x3ff9] = 0x97;  // checked by code

	PRGROM[0x3ffc] = 0x42;  // start vector
	PRGROM[0x3ffd] = 0x20;
	PRGROM[0x3ffe] = 0xf8;  // NMI vector
	PRGROM[0x3fff] = 0x26;
}

void calomega_state::init_comg145()
{
	uint8_t *PRGROM = memregion( "maincpu" )->base();

	PRGROM[0x3ff8] = 0x0a;    // patching the checksum error.
	PRGROM[0x3ff9] = 0xc1;    // patching the checksum error.
}

void calomega_state::init_comg176()
{
	uint8_t *PRGROM = memregion( "maincpu" )->base();

	PRGROM[0x25df] = 0x00;    // patching the checksum flag.
}

// for debug purposes...
void calomega_state::init_any()
{
	uint8_t *PRGROM = memregion( "maincpu" )->base();
	PRGROM[0x0000] = 0x00;

}

/*************************************************
*                  Game Drivers                  *
*************************************************/

//     YEAR  NAME      PARENT    MACHINE   INPUT     STATE           INIT          ROT    COMPANY                                  FULLNAME                                           FLAGS                                         LAYOUTS  */
GAME(  1981, comg074,  0,        sys903,   comg074,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 7.4 (Gaming Poker, W.Export)",    MACHINE_SUPPORTS_SAVE )
GAME(  1981, comg076,  0,        sys903,   comg076,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 7.6 (Arcade Poker)",              MACHINE_SUPPORTS_SAVE )
GAME(  1981, comg079,  0,        sys903,   comg076,  calomega_state, init_comg079, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 7.9 (Arcade Poker)",              MACHINE_SUPPORTS_SAVE )    // bad dump
GAME(  1981, comg080,  0,        sys903,   arcadebj, calomega_state, init_comg080, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 8.0 (Arcade Black Jack)",         MACHINE_SUPPORTS_SAVE )    // bad dump
GAMEL( 1981, comg094,  0,        sys903kb, keno_903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 9.4 (Keno)",                      MACHINE_SUPPORTS_SAVE,                        layout_kenokb )
GAME(  1982, comg107,  0,        sys903,   biggame,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 10.7c (Big Game)",                MACHINE_SUPPORTS_SAVE )
GAME(  1982, comg123,  0,        sys903,   stand903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 12.3 (Ticket Poker)",             MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )    // bad dump
GAME(  1982, comg125,  0,        sys903,   nudgesw,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 12.5 (Bingo)",                    MACHINE_SUPPORTS_SAVE )
GAMEL( 1982, comg127,  0,        sys903kb, keno_903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 12.7 (Keno)",                     MACHINE_SUPPORTS_SAVE,                        layout_kenokb )
GAME(  1982, comg128,  0,        sys903,   comg128,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 12.8 (Arcade Game)",              MACHINE_SUPPORTS_SAVE )
GAME(  1982, comg134,  0,        sys903,   nudgensw, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 13.4 (Nudge)",                    MACHINE_SUPPORTS_SAVE )
GAME(  1982, comg145,  0,        sys903,   pix903,   calomega_state, init_comg145, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 14.5 (Pixels)",                   MACHINE_SUPPORTS_SAVE )    // bad dump
GAME(  1983, comg157,  0,        sys903,   comg157,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 15.7 (Double-Draw Poker)",        MACHINE_SUPPORTS_SAVE )
GAME(  1983, comg159,  0,        sys905,   comg159,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 15.9 (Wild Double-Up)",           MACHINE_SUPPORTS_SAVE )
GAMEL( 1983, comg164,  0,        sys903kb, keno_903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 16.4 (Keno)",                     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE,  layout_kenokb )    // incomplete dump
GAMEL( 1983, comg168,  0,        sys903kb, keno_903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 16.8 (Keno)",                     MACHINE_SUPPORTS_SAVE,                        layout_kenokb )
GAME(  1983, comg170,  0,        sys905,   comg170,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 17.0 (Amusement Poker)",          MACHINE_SUPPORTS_SAVE )
GAME(  1983, comg172,  0,        sys905,   comg172,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 17.2 (Double Double Poker)",      MACHINE_SUPPORTS_SAVE )
GAME(  1984, comg175,  0,        sys903,   gdrwpkrd, calomega_state, empty_init,   ROT0, "Cal Omega / Casino Electronics Inc.",   "Cal Omega - Game 17.51 (Gaming Draw Poker, Sch 07-0T)", MACHINE_SUPPORTS_SAVE )
GAME(  1982, comg176,  0,        sys903,   nudgensw, calomega_state, init_comg176, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 17.6 (Nudge)",                    MACHINE_SUPPORTS_SAVE )
GAME(  1982, comg181,  0,        sys903,   nudgesw,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.1 (Nudge)",                    MACHINE_SUPPORTS_SAVE )
GAME(  1983, comg183,  0,        sys905,   pixels,   calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.3 (Pixels)",                   MACHINE_SUPPORTS_SAVE )
GAME(  1983, comg184,  0,        sys905,   pixels,   calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.4 (Pixels)",                   MACHINE_SUPPORTS_SAVE )
GAME(  1983, comg185,  0,        sys905,   pixels,   calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.5 (Pixels)",                   MACHINE_SUPPORTS_SAVE )
GAME(  1983, comg186,  0,        sys905,   pixels,   calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.6 (Pixels)",                   MACHINE_SUPPORTS_SAVE )
GAME(  1983, comg187,  0,        sys905,   stand905, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.7 (Amusement Poker)",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )    // bad dump
GAME(  1984, comg204,  0,        sys905,   comg204,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 20.4 (Super Blackjack)",          MACHINE_SUPPORTS_SAVE )
GAME(  1984, comg208,  0,        sys905,   comg208 , calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 20.8 (Winner's Choice)",          MACHINE_SUPPORTS_SAVE )
GAME(  1984, comg214,  0,        sys905,   stand905, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 21.4 (Wild Double-Up Poker)",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )    // incomplete dump
GAME(  1984, comg227,  0,        sys905,   stand905, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 22.7 (Amusement Poker, d/d)",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )    // bad dump
GAME(  1984, comg230,  0,        sys905,   stand905, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 23.0 (FC Bingo (4-card))",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )    // bad dump
GAME(  1984, comg236,  0,        sys905,   hotline,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 23.6 (Hotline)",                  MACHINE_SUPPORTS_SAVE )
GAME(  1985, comg239,  0,        sys903,   gdrwpkrd, calomega_state, empty_init,   ROT0, "Cal Omega / Casino Electronics Inc.",   "Cal Omega - Game 23.9 (Gaming Draw Poker)",        MACHINE_SUPPORTS_SAVE )
GAME(  1985, comg240,  0,        sys903,   gdrwpkrh, calomega_state, empty_init,   ROT0, "Cal Omega / Casino Electronics Inc.",   "Cal Omega - Game 24.0 (Gaming Draw Poker, hold)",  MACHINE_SUPPORTS_SAVE )
GAME(  1985, comg246,  0,        sys905,   hotline,  calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 24.6 (Hotline)",                  MACHINE_SUPPORTS_SAVE )
GAMEL( 1985, comg272a, 0,        sys903kb, keno_903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 27.2 (Keno, amusement)",          MACHINE_SUPPORTS_SAVE,                         layout_kenokb )
GAMEL( 1985, comg272b, 0,        sys903kb, keno_903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 27.2 (Keno, gaming)",             MACHINE_SUPPORTS_SAVE,                         layout_kenokb )

//************ Diagnostic Sets ************
GAME( 198?, comg903d,  0,        sys903,   stand903, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - System 903 Diag.PROM",                 MACHINE_SUPPORTS_SAVE )
GAME( 198?, comg905d,  0,        sys905,   stand905, calomega_state, empty_init,   ROT0, "Cal Omega Inc.",                        "Cal Omega - System 905 Diag.PROM",                 MACHINE_SUPPORTS_SAVE )

//************* 906-III games **************
GAME( 1988, comg5107,  0,        sys906,   stand906, calomega_state, empty_init,   ROT0, "Casino Electronics Inc.",               "CEI 51.07 (CEI 906-III Poker, Schedule 05F)",      MACHINE_SUPPORTS_SAVE )
GAME( 1988, comg5108,  0,        sys906,   stand906, calomega_state, empty_init,   ROT0, "Casino Electronics Inc.",               "CEI 51.08 (CEI 906-III Poker, Schedule 05F)",      MACHINE_SUPPORTS_SAVE )
GAME( 1988, comg5108a, comg5108, sys906,   stand906, calomega_state, empty_init,   ROT0, "Casino Electronics Inc.",               "CEI 51.08 (CEI 906-III Poker, Schedule 09F)",      MACHINE_SUPPORTS_SAVE )
GAME( 1988, comg5108b, comg5108, sys906,   stand906, calomega_state, empty_init,   ROT0, "Casino Electronics Inc.",               "CEI 51.08 (CEI 906-III Poker, Schedule 21F)",      MACHINE_SUPPORTS_SAVE )
GAME( 1989, comg6004,  0,        sys906,   stand906, calomega_state, empty_init,   ROT0, "Casino Electronics Inc.",               "CEI 60.04 (CEI 906-III Poker, Schedule 09F)",      MACHINE_SUPPORTS_SAVE )

//****** Unofficial 903/904/905 3rd party games *******
GAME( 1982, elgrande,  0,        s903mod,  elgrande, calomega_state, empty_init,   ROT0, "Tuni Electro Service",                  "El Grande - 5 Card Draw (New)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1983, jjpoker,   0,        s903mod,  jjpoker,  calomega_state, empty_init,   ROT0, "Enter-Tech, Ltd.",                      "Jackpot Joker Poker (set 1)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1983, jjpokerb,  jjpoker,  s903mod,  jjpoker,  calomega_state, empty_init,   ROT0, "Enter-Tech, Ltd.",                      "Jackpot Joker Poker (set 2)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1988, ssipkr24,  0,        s903mod,  ssipkr,   calomega_state, empty_init,   ROT0, "SSI",                                   "SSI Poker (v2.4)",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1988, ssipkr30,  ssipkr24, s903mod,  ssipkr,   calomega_state, empty_init,   ROT0, "SSI",                                   "SSI Poker (v3.0)",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1990, ssipkr40,  ssipkr24, s903mod,  ssipkr,   calomega_state, empty_init,   ROT0, "SSI",                                   "SSI Poker (v4.0)",                                 MACHINE_SUPPORTS_SAVE )

//****** Unofficial 906-III family 3rd party games *******
GAME( 1990, cas21iwc,  0,        sys906,   cas21iwc, calomega_state, empty_init,   ROT0, "UCMC/IWC",                              "Casino 21 UCMC/IWC (ver 30.08)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1991, pokeriwc,  0,        sys906,   pokeriwc, calomega_state, empty_init,   ROT0, "UCMC/IWC",                              "Poker UCMC/IWC (ver 162.03)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1991, pokiwc162, pokeriwc, sys906,   pokeriwc, calomega_state, empty_init,   ROT0, "UCMC/IWC",                              "Poker UCMC/IWC (ver 162.03 20-6-91)",              MACHINE_SUPPORTS_SAVE )
