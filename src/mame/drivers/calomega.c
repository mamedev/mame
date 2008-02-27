/**********************************************************************************

    +-------------------------------------+
    |                                     |
    | CAL OMEGA - SYSTEMS 903/904/905/906 |
    |                                     |
    |      Driver by Roberto Fresca.      |
    |                                     |
    +-------------------------------------+


    --- Technical Notes ---

    Name:    System 903, System 904, System 905 & System 906.
    Company: Cal Omega / Omega Products / Casino Electronics Inc.
    Year:    1981-1989


    Hardware:

    CPU =  1x SY6502 @ 625 kHz.  ; M6502 compatible.
    SND =  1x AY8912 @ 1.25 MHz. ; AY-3-8910 compatible.
    I/O =  2x PIAs SY6520/6820   ; M6821 compatible.
    CRCT = 1x SY6545             ; M6845 compatible mode. It doesn't seems that is using
                                 ; the first 4 bits of register R03 (HSync width).

    CLK =  1x Xtal @ 10 MHz.     ; CPU clock = 625 kHz.
                                 ; SND clock = 1250 kHz.
                                 ; PXL clock = 5000 kHz.

    Systems 903/904 also have a 6850 for serial comunication.

    PRG ROMS at locations u5, u6, u7, u8, u9.
    GFX ROMS at locations u67(cgo), u68(cga), u69(cgb), u70(cgc).

    For systems 903/904, u5 to u9 are 2516
    For system 905,      u5 to u9 are 2532

    DSW = 3 banks (903/904)
    DSW = 1 bank  (905)

    Inputs use 6 lines from 6520 PIA's port, and multiplexed x4 each.

    Some games like Keno and Big Game, also have a key encoder peripheral processor
    based on a 8035, and 1x ROM 2516/2532.



***********************************************************************************


   ++++ System 903-906 official list of games (not complete) ++++

   -----+--------------------------+--------+---------+----------+---------+----------+---------+---------------+------+------+-------
    VER |  NAME                    | SYSTEM | PROGRAM | CHAR GEN | CG P/N  | CLR PROM | CP P/N  | GAME TYPE     | DUMP | STAT | ADDED
   -----+--------------------------+--------+---------+----------+---------+----------+---------+---------------+------+------+-------
    7.6 | Poker                    |  903   | AUG/81  | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
 *  7.9 | Arcade Poker             |  903   | AUG/81  | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | BD   | Yes
 *  8.0 | Arcade Black Jack        |  903   | SEP/81  | CPKCG?   | 6300010 | POKCLR   | 6600020 | Amusement?    | No   |      | No
 *  9.4 | Keno                     |  903   | DEC/81  | KCG      | 6300060 | POKCLR   | 6600020 | Amusement     | Yes  | BD   | Yes
 * 10.7 | Big Game                 |  903   | MAR/82  | LOTCG    | unknown | BCLR     | unknown | Amusement?    | Yes  | OK   | Yes
 * 11.3 | Black Jack               |  903   |         | CPKCG?   | 6300010 | POKCLR   | 6600020 | Amusement?    | No   |      | No
 * 12.5 | Bingo                    |  903   | JUN/82  | NBCG     | unknown | BCLR     | unknown | Amusement     | Yes  | OK   | Yes
 * 12.7 | Keno                     | 903/4? | JUL/82  | KCG      | 6300060 | POKCLR   | 6600020 | Amusement     | Yes  | BD   | Yes
 * 12.8 | Arcade Game (Draw Poker) |  903   | JUL/82  | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
   13.4 | Nudge Bingo              |  903   | SEP/82  | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | Yes  | OK   | Yes
   14.5 | Pixels                   |  903   | DEC/82  | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | OK   | Yes
   14.7 | Nudge Bingo              |  903   |         | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | No   |      | No
   15.0 | Punchboard               |  903   |         | PBCG     | 6300190 | PBCLR    | 6600130 | Gaming Wash   | No   |      | No
   15.7 | Double-Draw Poker        |  903   | JAN/83  | CPKCG    | 6300010 | POKCLR   | 6600020 | Amusement     | Yes  | OK   | Yes
   15.8 | Poker Montana            |   ?    |         | CPKCG    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   15.9 | Wild Double-Up           |  905   | JAN/83  | JKR2CG   | 6300100 | WLDCLR   | 6600010 | Amusement     | Yes  | OK   | Yes
   16.2 | Gaming Lottery           |   ?    |         | LCG      | 6300050 | LOTCLR   | 6600030 | Gaming        | No   |      | No
   16.3 | Black Jack               |   ?    |         | CPKCG    | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   16.4 | K.I.S.S. Keno            |  904   | MAY/83  | KCG      | 6300060 | PCLR     | 6600020 | Amusement     | Yes  | BD   | Yes
   16.5 | K.I.S.S. Poker           |   ?    |         | POKER    | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   16.6 | Nudge Bingo              |   ?    |         | NBCG     | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   16.7 | Poker                    |   ?    |         | PCG      | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   16.8 | Keno                     |  903   | MAR/83  | KCG      | 6300060 | POKCLR   | 6600020 | Amusement     | Yes  | BD   | Yes
   17.0 | Poker                    |  905   |         | POKERCGs | 6300010 | PKRCLR   | 6600020 | Amusement     | No   |      | No
   17.1 | Nudge Bingo              |   ?    |         | NBCG     | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   17.2 | Double/Double Poker      |  905   | MAY/83  | JKRPKR   | 6300100 | WILDCLR  | 6600010 | Amusement     | Yes  | OK   | Yes
   17.3 | Casino Poker             |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   17.4 | Gin Gaming               |   ?    |         | GCG0.2   | 6300080 | GINCLR   | 6600060 | Nevada Gaming | No   |      | No
   17.5 | Gaming Draw Poker        |  903   | DEC/84  | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | Yes  | OK   | Yes
   17.6 | Nudge                    |  903   | SEP/82  | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | Yes  | OK   | Yes
   17.7 | Keno                     |   ?    |         | KCG      | 6300060 | POKCLR   | 6600020 | Gaming        | No   |      | No
   17.9 | AWP Poker                |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   18.1 | Nudge                    |   ?    | JUL/83  | NBCG     | 6300040 | BCLR     | 6600040 | Amusement     | Yes  | OK   | Yes
   18.2 | Amusement Poker          |   ?    |         | JKRCG02  | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
   18.3 | Pixels                   |  905   | AUG/83  | PXCG     | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | OK   | Yes
   18.4 | Double Draw-Lebanon      |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   18.5 | Pixels                   |  905   | AUG/83  | PXCG     | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | OK   | Yes
   18.6 | Pixels                   |  905   | AUG/83  | PXCG     | 6300070 | PIXCLR   | 6600050 | Amusement     | Yes  | OK   | Yes
   18.7 | Amusement Poker (skill)  |  905   | AUG/83  | JKRTWO   | 6300100 | MULTCLR  | 6600070 | Amusement     | No   |      | No
   18.8 | Poker-Lebanon            |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   18.9 | Pixels (french)          |   ?    |         | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | No   |      | No
   19.0 | Hi-Score Poker           |   ?    |         | JKRTWO   | 6300100 | WLDCLR   | 6600070 | Amusement     | No   |      | No
 * 19.1 | Super Blackjack?         |  905   | AUG/83  | JKRTWO?  | 6300100 | MLTCLR   | 6600070 | Amusement?    | No   |      | No
   19.2 | AWP Poker                |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   19.3 | Gin                      |   ?    |         | GCG2     | 6300110 | MLTCLR   | 6600070 | Amusement     | No   |      | No
   19.4 | Pixels (french)          |   ?    |         | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | No   |      | No
   19.7 | Ind. Wild Double-Up      |   ?    |         | JKRCG    | 6300100 | WLDCLR   | 6600070 | Amusement     | No   |      | No
   19.9 | Double-Up Nudge          |   ?    |         | NUDGE    | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   20.1 | Poker                    |   ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Amusement     | No   |      | No
   20.4 | Super Blackjack          |  905   | FEB/84  | JKRTWO   | 6300100 | MLTCLR   | 6600070 | Amusement     | Yes  | OK   | Yes
 * 20.5 | Blackjack?               |  905   | FEB/84  | JKRTWO?  | 6300100 | MLTCLR   | 6600070 | Amusement?    | No   |      | No
   20.7 | Double-Up Nudge          |   ?    |         | NUDGECG  | 6300040 | BCOLOR   | 6600040 | Amusement     | No   |      | No
   20.8 | Winner's Choice          |   ?    | FEB/84  | MLT2CG   | 6300130 | MLTCLR   | 6600070 | Amusement     | Yes  | OK   | Yes
   20.9 | Gaming Poker-Europe      |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   21.1 | AWP Poker                |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   21.2 | Video Slot               |   ?    |         | SL1CG    | 6300180 | SLC1     | 6600120 | Gaming        | No   |      | No
   21.4 | Wild Double-Up Poker     |   ?    |         | WLDCG    | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
   21.5 | Double/Double Poker      |   ?    |         | JKRTWO   | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
 * 21.6 | Wild Double-Up?          |  905   | JUN/84  | WLDCG    | 6300100 | WLDCLR   | 6600010 | Amusement?    | No   |      | No
 * 21.7 | Poker?                   |  905   | MAY/84  | GPKCG?   | 6300010 | POKCLR   | 6600020 | Amusement?    | No   |      | No
   21.9 | Amusement Poker          |   ?    |         | JKRCG    | 6300100 | WILDCLR  | 6600010 | Amusement     | No   |      | No
   22.1 | Poker                    |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   22.7 | Amusement Poker (dbl/dbl)|  905   | JUL/84  | JKRPKR   | 6300100 | WLDCLR   | 6600010 | Amusement     | Yes  | OK   | Yes
   22.8 | Tennesee Wild Poker      |  905   |         | JKR2CG   | 6300100 | MLTCLR   | 6600070 | Amusement     | No   |      | No
   22.9 | Double/Double Poker      |  905   |         | JKRTWO   | 6300100 | MLTCLR   | 6600010 | Amusement     | No   |      | No
   23.0 | FC Bingo (4-card)        |  905   | JUN/84  | FCBCG    | 6300150 | FCBCLR   | 6600100 | Amusement     | No   |      | No
   23.2 | Pixels                   |   ?    |         | PIXCG    | 6300070 | PIXCLR   | 6600050 | Amusement     | No   |      | No
 * 23.6 | Hotline                  |  905   | OCT/84  | HLCG     | 6300160 | HLCLR    | 6600110 | Amusement     | Yes  | OK   | Yes
   23.7 | AWP Blackjack            |   ?    |         | POKER    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   23.9 | G.Draw Poker (discard)   |   ?    | JAN/85  | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | Yes  | OK   | Yes
   24.0 | Poker (hold)             |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   24.2 | Export Poker             |   ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   24.3 | Gaming Blackjack         |   ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   24.5 | Wild Double-Up           |   ?    |         | JKPKCG   | 6300100 | WLDCLR   | 6600010 | Amusement     | No   |      | No
   24.6 | Hotline                  |  905   | JAN/85  | HLCG     | 6300160 | HLCLR    | 6600110 | Amusement     | Yes  | OK   | Yes
   24.8 | Poker (discard)          |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   24.9 | Poker (hold)             |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   25.3 | 4-Card Bingo-Gaming      |   ?    |         | FCB1CG   | 6300150 | FCB1CLR  | 6600100 | Gaming        | No   |      | No
   25.4 | Indian 4-Card Bingo      |   ?    |         | FCBCG    | 6300150 | FCB1CLR  | 6600100 | Amer-Indian   | No   |      | No
   25.5 | Wild Double-Up Poker     |   ?    |         | JKRCG    | 6300100 | WILD     | 6600010 | Amusement     | No   |      | No
   25.6 | Mr. Domino               |   ?    |         | DOMCG    | 6300140 | DOMCLR   | 6600080 | Amusement     | No   |      | No
   25.7 | Mr. Domino               |   ?    |         | DOMCG    | 6300140 | DOMCLR   | 6600080 | Amusement     | No   |      | No
   25.8 | Poker                    |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   26.3 | Wild Double-Up Poker     |   ?    |         | JKRCG    | 6300100 | WILD     | 6600010 | Amusement     | No   |      | No
   26.5 | 4-Reel Slot              |   ?    |         | SLOT4CG  | 6300220 | SL4CLR   | 6600140 | Gaming        | No   |      | No
   26.6 | Poker                    |   ?    |         | GPKCG    | 6300250 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   26.8 | Wild Double-Up Poker     |   ?    |         | JKRCG    | 6300260 | WILD     | 6600010 | Nevada Gaming | No   |      | No
   27.1 | Tourney Gaming Poker     |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   27.2 | Arcade Keno              |   ?    | AUG/85  | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | Yes  | BD   | Yes
 * 27.2 | Arcade Keno              |   ?    | AUG/85  | KJCG     | 6300060 | POKCLR   | 6600020 | Gaming        | Yes  | BD   | Yes
   27.3 | Export Poker             |   ?    |         | PKCG     | 6300010 | POKCLR   | 6600020 | Gaming        | No   |      | No
   27.4 | Keno II                  |   ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | No   |      | No
   27.5 | Keno (Austria)           |   ?    |         | KENOCG   | 6300060 | POKCLR   | 6600020 | Gaming        | No   |      | No
   27.6 | Slot 8                   |   ?    |         | SL8CG    | 6300290 | SLOT4CLR | 6600140 | Gaming        | No   |      | No
   27.9 | 3-Reel Slot, Gaming      |   ?    |         | SL2CG    | 6300280 | SL2CLR   | 6600120 | Nevada Gaming | No   |      | No
   28.0 | 3-Reel Slot              |   ?    |         | SL2CG    | 6300180 | SL2CLR   | 6600120 | Gaming        | No   |      | No
   28.2 | Hotline                  |   ?    |         | HLCG     | 6300160 | HLCLR    | 6600110 | Amusement     | No   |      | No
   28.5 | Hotline                  |   ?    |         | HLCG     | 6300130 | HLCLR    | 6600110 | Amusement     | No   |      | No
   29.0x| Stand or Draw Poker      |   ?    |         | ST2CG    | 6300270 | ST2CLR   | 6600150 | Nevada Gaming | No   |      | No
   30.0x| Casino 21                |   ?    |         | GPKCG    | 6300250 | WILD     | 6600010 | Nevada Gaming | No   |      | No
   31.0x| Poker (discard)          |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   32.0x| Poker (hold)             |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   33.0x| Keno                     |   ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   34.0x| Keno (Austria)           |   ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Gaming        | No   |      | No
   35.0x| 1-3 Sign Poker (discard) |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   36.0x| 1-3 Sign Poker (hold)    |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   37.0x| Amusement Keno           |   ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | No   |      | No
   39.0x| 3-Reel Slot              |   ?    |         | SBARCG   | 6300305 | JKRSLOT  | 6600170 | Nevada Gaming | No   |      | No
   39.0x| 3-Reel Slot              |   ?    |         | PLAIN7CG | 6300309 | JKRSLOT  | 6600170 | Nevada Gaming | No   |      | No
   39.0x| 3-Reel Slot              |   ?    |         | JWCG     | 6300313 | JKRSLOT  | 6600170 | Nevada Gaming | No   |      | No
   40.0x| Poker w/Wild Card        |  903   |         | JKRCG    | 6300260 | WLDCLR   | 6600010 | Nevada Gaming | No   |      | No
   41.0x| Do-it-yourself Bingo     |   ?    |         | KCG      | 6300060 | PKRCLR   | 6600020 | Amer-Indian   | No   |      | No
   42.0x| Mr. Domino               |   ?    |         | NDMCG    | 6300300 | DOMCLR   | 6600080 | Amusement     | No   |      | No
   43.0x| Export Poker             |   ?    |         | GPKCG    | 6300170 | POKCLR   | 6600020 | Gaming        | No   |      | No
   44.0x| Tourney Poker (discard)  |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   45.0x| Tourney Poker (hold)     |   ?    |         | POKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   46.0x| Gaming Poker             |   ?    |         | GPKCG    | 6300010 | POKCLR   | 6600020 | Nevada Gaming | No   |      | No
   47.0x| 4-Card Indian Bingo      |   ?    |         | FCB7CG   | 6300150 | FCB7CLR  | 6600100 | Amer-Indian   | No   |      | No
   48.0x| Keno                     |   ?    |         | KJCG     | 6300060 | POKCLR   | 6600020 | Montana       | No   |      | No
   51.0x| Poker (906-III)          |  906   |         | GP2CG    | unknown | WILD     | unknown | Nevada Gaming | No   |      | No
        |                          |        |         |          |         |          |         |               |      |      |
 * 903d | System 903 Diag.PROM     |  903   | unknown | any      | unknown | any      | unknown | Testing H/W   | Yes  | OK   | Yes
 * 905d | System 905 Diag.PROM     |  905   | unknown | any      | unknown | any      | unknown | Testing H/W   | Yes  | OK   | Yes

  
   (*) Not in the original list.



   ++++ Unofficial games ++++

   -------------------------------+----------------------+------+-----------+--------+------+------+-------
    NAME                          | COMPANY              | YEAR | GAME TYPE | SYSTEM | DUMP | STAT | ADDED
   -------------------------------+----------------------+------+-----------+--------+------+------+-------
    El Grande - 5 Card Draw (New) | Tuni Electro Service | 1982 | Amusement |  903   | Yes  | BR   | Yes
    Jackpot Joker Poker (set 1)   | Enter-Tech (ETL)     | 1983 | Amusement |  903   | Yes  | OK   | Yes
    Jackpot Joker Poker (set 2)   | Enter-Tech (ETL)     | 1983 | Amusement |  903   | Yes  | OK   | Yes
    SSI Poker (v2.4)              | SSI                  | 1988 | Amusement |  903   | Yes  | IC   | Yes
    SSI Poker (v3.0)              | SSI                  | 1988 | Amusement |  903   | Yes  | IC   | Yes
    SSI Poker (v4.0)              | SSI                  | 1990 | Amusement |  903   | Yes  | IC   | Yes


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

    This game was made before the CEI 906iii system.

    The game use the same GFX set that Jack Potten's poker for cards and have similar
    layout, but the game is different and the old discrete pitched sounds were replaced
    with a better set of sounds through a AY8910/12 implementation. The empty socket in
    the pcb is maybe for future upgrades instead of sound ROM, since sounds are hardcoded.

    Inputs are multiplexed and selected through PIA1, portB.

    In game, use "Stand" instead of "Deal/Draw" to conserve all cards without discards.
    To enter to TEST MODE press F2. Press "Discard 1" + "Discard 2" + "Discard 3" to exit.
    To enter the STATS MODE press "Show Stats". Press "Deal/Draw" to exit.
    To pass coin and hopper errors press "Hopper SW". Also keep it pressed to see the status.
    For payout, press "Manual Collect" an then "Payout" for each credit (manual mode).


    El Grande - 5 Card Draw:

    This game was created by Tuni Electro Service and was licenced to E.T. Marketing,Inc.
    This is the new version. The old one is still undumped, but looks like Golden Poker D-Up.

    Flyer: http://www.arcadeflyers.com/?page=thumbs&db=videodb&id=4542

    The AY8912 is mapped to 0x840/0x841 but the game lacks of credits and bet sounds.
    The rest of sounds are ok. Maybe the missing ones are generated by discrete circuitry.

    Inputs are multiplexed and selected through PIA1, portB.

    There aren't meter and stats modes. Only for amusement, so... no payout.
    To clear credits press F2.
    To enter to TEST MODE press F2 twice. Press "Hold 1" + "Hold 2" + "Hold 3" to exit.



***********************************************************************************?


    *** Memory Maps ***

    -- System 903 --

    $0000 - $07FF   NVRAM           ; All registers and settings.
    $0840 - $0840   AY-8912         ; Read/Control.
    $0841 - $0841   AY-8912         ; Write.
    $0880 - $0880   CRTC6845        ; MC6845 adressing.
    $0881 - $0881   CRTC6845        ; MC6845 Read/Write.
    $08C4 - $08C7   PIA0            ; I/O Ports 0, 1, 2, 3.
    $08C8 - $08CB   PIA1            ; I/O Port  4.

    $1000 - $13FF   VideoRAM
    $1400 - $17FF   ColorRAM

    $1800 - $FFFF   ROM


    -- System 905 --

    $0000 - $07FF   NVRAM           ; All registers and settings.
    $1080 - $1080   CRTC6845        ; MC6845 adressing.
    $1081 - $1081   CRTC6845        ; MC6845 Read/Write.
    $10C4 - $10C7   PIA0            ; I/O Ports 0, 1, 2, 3.
    $10C8 - $10CB   PIA1            ; I/O Port  4, AY-8912.

    $2000 - $23FF   VideoRAM
    $2400 - $27FF   ColorRAM

    $2800 - $FFFF   ROM


    *** MC6545 Initialization ***

    ----------------------------------------------------------------------------------------------------------------------
    register:  R00   R01   R02   R03   R04   R05   R06   R07   R08   R09   R10   R11   R12   R13   R14   R15   R16   R17
    ----------------------------------------------------------------------------------------------------------------------
    value:     0x27  0x20  0x23  0x03  0x1F  0x04  0x1F  0x1F  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.



    *** MC6545 conditional change for elgrande if 0x8c4 (PIA0) has bit7 activated ***

    ---------------------------------------------------------
    register:  R00   R01   R02   R03   R04   R05   R06   R07
    ---------------------------------------------------------
    value:     0x27  0x20  0x23  0x03  0x26  0x00  0x20  0x22



***********************************************************************************


    Dumper notes (old)


    -- Gaming Draw Poker --------------------------------------------------------

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


    -- El Grande 5 Card Draw ----------------------------------------------------

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


***********************************************************************************


    UPDATES
    -------


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

    - Parent/clone relationship.
    - System 905 inputs.
    - System 905 sound through PIAs.
    - Fix lamps.


***********************************************************************************/


#define MASTER_CLOCK	XTAL_10MHz

#include "driver.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"

/* from video */
WRITE8_HANDLER( calomega_videoram_w );
WRITE8_HANDLER( calomega_colorram_w );
WRITE8_HANDLER( calomega_mc6845_address_w );
READ8_HANDLER( calomega_mc6845_register_r );
WRITE8_HANDLER( calomega_mc6845_register_w );
PALETTE_INIT( calomega );
VIDEO_START( calomega );
VIDEO_UPDATE( calomega );


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( sys903_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x0840, 0x0840) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x0841, 0x0841) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x0880, 0x0880) AM_WRITE(calomega_mc6845_address_w)
	AM_RANGE(0x0881, 0x0881) AM_READWRITE(calomega_mc6845_register_r, calomega_mc6845_register_w)
	AM_RANGE(0x08c4, 0x08c7) AM_READWRITE(pia_0_r, pia_0_w)
	AM_RANGE(0x08c8, 0x08cb) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_WRITE(calomega_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x1400, 0x17ff) AM_RAM AM_WRITE(calomega_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x1800, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sys905_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x0880, 0x0880) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x0881, 0x0881) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x1080, 0x1080) AM_WRITE(calomega_mc6845_address_w)
	AM_RANGE(0x1081, 0x1081) AM_READWRITE(calomega_mc6845_register_r, calomega_mc6845_register_w)
	AM_RANGE(0x10c4, 0x10c7) AM_READWRITE(pia_0_r, pia_0_w)
	AM_RANGE(0x10c8, 0x10cb) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0x2000, 0x23ff) AM_RAM AM_WRITE(calomega_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x2400, 0x27ff) AM_RAM AM_WRITE(calomega_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x2800, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( gdrawpkr )
	PORT_START_TAG("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )	/* credits */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double Up") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal/Draw") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Cancel Discards") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("Stand") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_8)
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 3") PORT_CODE(KEYCODE_F)
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2)	/* should be splitted? */
	PORT_BIT( 0x14, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper SW") PORT_CODE(KEYCODE_9)	/* should be splitted? */
	PORT_BIT( 0x24, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Show Stats") PORT_CODE(KEYCODE_0)	/* should be splitted? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Discard 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Discard 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Discard 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Discard 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Discard 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Small") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Take") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Big") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("SW1")
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
	PORT_DIPNAME( 0xC0, 0x40, "Maximum Bet")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0xC0, "80" )
INPUT_PORTS_END

static INPUT_PORTS_START( comg076 )
	PORT_START_TAG("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )	/* credits */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double Up") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal/Draw") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Cancel Discards") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("Stand") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )	/* credits */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_8)
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 3") PORT_CODE(KEYCODE_F)
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 4") PORT_CODE(KEYCODE_G)
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Unk.")   PORT_CODE(KEYCODE_8)	/* should be splitted? */
	PORT_BIT( 0x14, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper SW") PORT_CODE(KEYCODE_9)	/* should be splitted? */
	PORT_BIT( 0x24, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2)	/* should be splitted? */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Discard 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Discard 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Discard 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Discard 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Discard 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Small") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Take") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Big") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("SW1")
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
	PORT_DIPNAME( 0x30, 0x00, "Minimum Winning Hand")
	PORT_DIPSETTING(    0x00, "Jacks or Better" )
	PORT_DIPSETTING(    0x20, "Queens or Better" )
	PORT_DIPSETTING(    0x30, "Kings or Better" )
	PORT_DIPSETTING(    0x10, "Pair of Aces" )
	PORT_DIPNAME( 0xC0, 0x80, "Maximum Bet")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0xC0, "20" )
	PORT_DIPSETTING(    0x40, "50" )
INPUT_PORTS_END

static INPUT_PORTS_START( comg128 )
	PORT_START_TAG("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )	/* credits */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double Up") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal/Draw") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Cancel Discards") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Discard 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Discard 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Discard 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Discard 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Discard 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Take") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Big") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Small") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("SW1")
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
	PORT_DIPNAME( 0xC0, 0x40, "Hands per Coin")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xC0, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( elgrande )
	PORT_START_TAG("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double Up") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal/Draw") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Cancel Holds") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Take") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Odd") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Even") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("SW1")
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
INPUT_PORTS_END

static INPUT_PORTS_START( jjpoker )
	PORT_START_TAG("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double Up") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal/Draw") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Cancel Discards") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Discard 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Discard 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Discard 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Discard 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Discard 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Take") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Odd") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Even") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("SW1")
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
	PORT_DIPSETTING(    0x08, "1 coin 50 credits" )
	PORT_DIPSETTING(    0x00, "1 coin 25 credits" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( ssipkr )
	PORT_START_TAG("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Double Up") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal/Draw") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Cancel Discards") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Discard 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Discard 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Discard 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Discard 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Discard 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Take") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Odd") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Even") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("SW1")
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
	PORT_DIPSETTING(    0x08, "1 coin 1 credits" )
	PORT_DIPSETTING(    0x00, "1 coin 10 credits" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( standard )
	PORT_START_TAG("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("SW1")
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*************************
*    Graphics Layouts    *
*************************/

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
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( calomega )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout, (8 * 3) + 128, 16 )
GFXDECODE_END


/***********************
*    Write Handlers    *
***********************/

static int mux_data = 0;

static READ8_HANDLER( mux_port_r )
{
	switch( mux_data&0xf0 )	/* bits 4-7 */
	{
		case 0x10: return input_port_0_r(0);
		case 0x20: return input_port_1_r(0);
		case 0x40: return input_port_2_r(0);
		case 0x80: return input_port_3_r(0);
	}
	return 0xff;
}

static WRITE8_HANDLER( mux_w )
{
	mux_data = data ^ 0xff;	/* inverted */
}


/****  Lamps debug  ****

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
static WRITE8_HANDLER( lamps_a_w )
{
	output_set_lamp_value(0, 1-((data) & 1));		// 0
	output_set_lamp_value(1, 1-((data >> 1) & 1));	// 1
	output_set_lamp_value(2, 1-((data >> 2) & 1));	// 2
	output_set_lamp_value(3, 1-((data >> 3) & 1));	// 3
	output_set_lamp_value(4, 1-((data >> 4) & 1));	// 4
	output_set_lamp_value(5, 1-((data >> 5) & 1));	// 5
	output_set_lamp_value(6, 1-((data >> 6) & 1));	// 6
	output_set_lamp_value(7, 1-((data >> 7) & 1));	// 7
}

static WRITE8_HANDLER( lamps_b_w )
{
	output_set_lamp_value(8, 1-((data) & 1));		// 0
	output_set_lamp_value(9, 1-((data >> 1) & 1));	// 1
	output_set_lamp_value(10, 1-((data >> 2) & 1));	// 2
	output_set_lamp_value(11, 1-((data >> 3) & 1));	// 3
	output_set_lamp_value(12, 1-((data >> 4) & 1));	// 4
	output_set_lamp_value(13, 1-((data >> 5) & 1));	// 5
	output_set_lamp_value(14, 1-((data >> 6) & 1));	// 6
	output_set_lamp_value(15, 1-((data >> 7) & 1));	// 7
}


/***********************
*    PIA Interfaces    *
***********************/

static const pia6821_interface pia0_intf =
{
	/* PIA inputs: A, B, CA1, CB1, CA2, CB2 */
	mux_port_r, 0, 0, 0, 0, 0,

	/* PIA outputs: A, B, CA2, CB2 */
	0, lamps_a_w, 0, 0,

	/* PIA IRQs: A, B */
	0, 0
};

static const pia6821_interface pia1_intf =
{
	/* PIA inputs: A, B, CA1, CB1, CA2, CB2 */
	input_port_4_r, 0, 0, 0, 0, 0,	/* 5 & 6 not tested */

	/* PIA outputs: A, B, CA2, CB2 */

	lamps_b_w, mux_w, 0, 0,

	/* PIA IRQs: A, B */
	0, 0
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_DRIVER_START( sys903 )
	// basic machine hardware
	MDRV_CPU_ADD_TAG("main", M6502, MASTER_CLOCK/16)	/* confirmed */
	MDRV_CPU_PROGRAM_MAP(sys903_map, 0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold, 1)

	MDRV_NVRAM_HANDLER(generic_0fill)

	// video hardware
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((39+1)*8, (31+1)*8)                  /* Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1) */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 31*8-1)    /* Taken from MC6845 init, registers 01 & 06 */

	MDRV_GFXDECODE(calomega)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_PALETTE_INIT(calomega)
	MDRV_VIDEO_START(calomega)
	MDRV_VIDEO_UPDATE(calomega)

	MDRV_DEVICE_ADD("crtc", MC6845)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(AY8912, MASTER_CLOCK/8)		/* confirmed */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sys905 )
	// basic machine hardware
	MDRV_IMPORT_FROM( sys903 )

	MDRV_CPU_REPLACE("main", M6502, MASTER_CLOCK/16)	/* confirmed */
	MDRV_CPU_PROGRAM_MAP(sys905_map, 0)
MACHINE_DRIVER_END


/*************************
*        ROM Load        *
*************************/

ROM_START( comg076 )	/* Cal Omega v7.6 (Arcade Poker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "7.62",	0x2000, 0x0800, CRC(99ffa2a4) SHA1(07fd182ff86f9ab09bbf09f51d655811a09ffa03) )
	ROM_LOAD( "7.63",	0x2800, 0x0800, CRC(9ed58bc5) SHA1(c545053847ec7585e4ac97c70cf33529ed0f1111) )
	ROM_LOAD( "7.64",	0x3000, 0x0800, CRC(95714680) SHA1(26938903600bd0920b1dd1c6900c56c6b749976a) )
	ROM_LOAD( "7.65",	0x3800, 0x0800, CRC(02be8b39) SHA1(bb056e6e0ebd5f81c61fa7aa8252330088b3088e) )
	ROM_RELOAD(			0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgo",	 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgc",	 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pokcgb",	 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pokcga",	 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg079 )	/* Cal Omega v7.9 (Arcade Poker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "7.91",	0x1800, 0x0800, CRC(da415c27) SHA1(be42f1d36579ff8fafd6df4e30a64a6141a7a2ae) )
	ROM_LOAD( "7.92",	0x2000, 0x0800, CRC(31211ed3) SHA1(799bc4ca77ee01a4d45320263e4cc2d066ec26e5) )
 /* code jumps to $2e64 where there's an inexistent mnemonic 0x32! */
	ROM_LOAD( "7.93",	0x2800, 0x0800, BAD_DUMP CRC(52c84b9c) SHA1(569d6384b9084a11fe84e291f1c165bc0df3ab49) )
	ROM_LOAD( "7.94",	0x3000, 0x0800, CRC(ed0e7d4a) SHA1(72f512ca2da573e4571ca6f164fb055daa26dd3c) )
	ROM_LOAD( "7.95",	0x3800, 0x0800, CRC(3acc8f49) SHA1(1a193cc292f79da869caddd11478d6ad55466d7b) )
	ROM_RELOAD(			0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgo",	 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgc",	 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pokcgb",	 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pokcga",	 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg094 )	/* Cal Omega v9.4 (Keno) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "keno9_4.02",	0x2000, 0x0800, CRC(1cff1319) SHA1(97b7ed54d398f36dc476028471784fc4e50bc378) )
	ROM_LOAD( "keno9_4.03",	0x2800, 0x0800, CRC(e6b123be) SHA1(f89df2dc6deeecff41be83d7a9040cfe5d872bad) )
	ROM_LOAD( "keno9_4.04",	0x3000, 0x0800, CRC(13939de9) SHA1(b2f97828808f6001846049cbf9af40e32908a58f) )
	ROM_LOAD( "keno9_4.05",	0x3800, 0x0800, CRC(7508de2e) SHA1(62faf65a1b815e11158cfb807090923ab368784d) )
	ROM_RELOAD(				0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgo",	 0x0000, 0x0800, BAD_DUMP CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )	// bad dump?

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgc",	 0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kenocg.cgb",	 0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kenocg.cga",	 0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg107 )	/* Cal Omega v10.7 (Big Game) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "10.72",	0x2000, 0x0800, CRC(dd0fbefb) SHA1(caed286ba1adb4d5c5c874c56339b9d71dd41bc6) )
	ROM_LOAD( "10.73",	0x2800, 0x0800, CRC(fcb7774d) SHA1(d3c89e0df0005e4bc4894156622b3d1e4cd09f2a) )
	ROM_LOAD( "10.74",	0x3000, 0x0800, CRC(d19ed885) SHA1(c41e59e87ce88a5b229e334b6a563a3b21d12b15) )
	ROM_LOAD( "10.75",	0x3800, 0x0800, CRC(1491f3ab) SHA1(cf545f5f5da910b98846699bff1d2e6eb40dc290) )
	ROM_RELOAD(			0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "lotclr.cgo",	 0x0000, 0x1000, CRC(6f7cffee) SHA1(ababeb49155c84b22c4bb3568cfb5f05ff1cd797) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "lotclr.cgc",	 0x0000, 0x1000, CRC(bfe4df1c) SHA1(f7e055d41e16ead6b18d755e664347645f94865a) )
	ROM_LOAD( "lotclr.cgb",	 0x1000, 0x1000, CRC(5bda0f42) SHA1(d4b3340e9c8ca49483fa846103f0bd81d57a5ab3) )
	ROM_LOAD( "lotclr.cga",	 0x2000, 0x1000, CRC(0975e360) SHA1(7b9dbbae50c43ad99ee11798ada0a44e71c611f9) )

	ROM_REGION( 0x0800, REGION_USER1, 0 )	/* keyboard interfase ROM */
	ROM_LOAD( "lotkbd.sub",	 0x0000, 0x0800, CRC(c1636ab5) SHA1(5a3ad24918751ca6a6640807e421e80f6b4cc844) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "bclr",		0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg125 )	/* Cal Omega v12.5 (Bingo) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "bngo12_5.02", 0xe000, 0x0800, CRC(ea0a6bd6) SHA1(f138d2f29252d95dea93a1e936725de99c714b35) )
	ROM_LOAD( "bngo12_5.03", 0xe800, 0x0800, CRC(c888ee34) SHA1(2796c6ae196b046f12b75b1c095a430fa4be0da2) )
	ROM_LOAD( "bngo12_5.04", 0xf000, 0x0800, CRC(4c7d11a6) SHA1(0131f077e204250d594c9baadc0596efeb7639cc) )
	ROM_LOAD( "bngo12_5.05", 0xf800, 0x0800, CRC(fabe1de0) SHA1(a39583d410707930fbe32341c30bb1b8a28a8d73) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bingo.cgo", 0x0000, 0x1000, CRC(ded1c944) SHA1(eafb30b4a2cbb6c10dba959d3839a8f8bf793972) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "bingo.cga", 0x0000, 0x1000, CRC(3fc39df9) SHA1(223d05f8969a1846a986b29395c98f97a3218bf7) )
	ROM_LOAD( "bingo.cgb", 0x1000, 0x1000, CRC(a7548075) SHA1(a751289cbc8b726082b60740c0202c08e3981e24) )
	ROM_LOAD( "bingo.cgc", 0x2000, 0x1000, CRC(9f101dd8) SHA1(4f6671b8b36f9351f368d8bbc92c20bd2b7b3dab) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "82s129n.u28", 0x0000, 0x0100, CRC(6db5a344) SHA1(5f1a81ac02a2a74252decd3bb95a5436cc943930) )
	ROM_RELOAD(				 0x0100, 0x0100 )
	ROM_RELOAD(				 0x0200, 0x0100 )
	ROM_RELOAD(				 0x0300, 0x0100 )
ROM_END

ROM_START( comg127 )	/* Cal Omega v12.7 (Keno) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "keno12_7.02",	0xe000, 0x0800, CRC(091a4b32) SHA1(3600854152482bf18d2377aa635a9fd7f3d4b2f7) )
	ROM_LOAD( "keno12_7.03",	0xe800, 0x0800, CRC(c1fb5293) SHA1(f20cdddb4c89cb2fc647ddfcff6bd6f5095a0a28) )
	ROM_LOAD( "keno12_7.04",	0xf000, 0x0800, CRC(de277137) SHA1(127b95616824dfb1025f3346c1335a8bf4835e68) )
	ROM_LOAD( "keno12_7.05",	0xf800, 0x0800, CRC(04485ba3) SHA1(ee22396fc23508635e43ca8b17fc1f23e670fe85) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgo",	 0x0000, 0x0800, BAD_DUMP CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )	// bad dump?

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgc",	 0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kenocg.cgb",	 0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kenocg.cga",	 0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg128 )	/* Cal Omega v12.8 (Arcade Game) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "12.82",	0xe000, 0x0800, CRC(85f0c548) SHA1(f08e26a7acb883a92e73e564fb2b58422d1784c6) )
	ROM_LOAD( "12.83",	0xe800, 0x0800, CRC(57212e46) SHA1(d0bac63e0efcdb8ef4a1b6c4d53776447557e4d0) )
	ROM_LOAD( "12.84",	0xf000, 0x0800, CRC(00ab3cd8) SHA1(da00457c49d3a101dc31578d07b2e986f0d73919) )
	ROM_LOAD( "12.85",	0xf800, 0x0800, CRC(3b00c172) SHA1(039aa2a2b41ea29e8e22a889c3984e30b321e7e3) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgo",	 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgc",	 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pokcgb",	 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pokcga",	 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg134 )	/* Cal Omega 13.4 (Nudge Keno) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "13.42",	0xe000, 0x0800, CRC(3ff1e83d) SHA1(0c2d2faed1148909dd7d50e9eda76c6403181435) )
	ROM_LOAD( "13.43",	0xe800, 0x0800, CRC(cf2125f4) SHA1(32678f3ac82c76fb8116de77ce332fe098d5e8c0) )
	ROM_LOAD( "13.44",	0xf000, 0x0800, CRC(09a8d3c4) SHA1(60937b386a7fa8c30bd509633b5e98e79a1189d4) )
	ROM_LOAD( "13.45",	0xf800, 0x0800, CRC(790d3347) SHA1(02bbbfd447bee596557e4c3a61649ca1330a970f) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nbcg.cgo",	0x0000, 0x0800, CRC(b4b01f22) SHA1(1214d91ae28ac40ef37fce787783cd64331ed224) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "nbcg.cgc",	0x0000, 0x1000, CRC(f7ca234f) SHA1(3b2717051341c423969ed4bfb292a9f88119c2a8) )
	ROM_LOAD( "nbcg.cgb",	0x1000, 0x1000, CRC(9d409932) SHA1(d3ffca50a059278777238d206895a0d188f4ff6f) )
	ROM_LOAD( "nbcg.cga",	0x2000, 0x1000, CRC(afe1a666) SHA1(c1530700a283d18e7136754d45904930ef424bcf) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "bclr",		0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg145 )	/* Cal Omega v14.5 (Pixels) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "14.51", 0xd800, 0x0800, CRC(b173af4b) SHA1(2fd7d391c765e8e5bf148d92223ffc2a2619dcfd) )
	ROM_LOAD( "14.52", 0xe000, 0x0800, CRC(f12434c5) SHA1(f957831867cc8c98fb479cd2d859790d19883d26) )
	ROM_LOAD( "14.53", 0xe800, 0x0800, CRC(3b22802b) SHA1(8cc563e79fd90873132e358f2ce5942ddbd65144) )
	ROM_LOAD( "14.54", 0xf000, 0x0800, CRC(8a230410) SHA1(4ff31869a7389af20f6557d9306f19a5d3ba5766) )
	ROM_LOAD( "14.55", 0xf800, 0x0800, CRC(0e6e7413) SHA1(6e2b37f6cbb49ce3e0111777a3b197f5b2012ed0) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cgo", 0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cga", 0x0000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )
	ROM_LOAD( "pixcg.cgb", 0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pixcg.cgc", 0x2000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pixclr",		0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg157 )	/* Cal Omega v15.7 (Double-Draw Poker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "15.71",	 0xd800, 0x0800, CRC(e0c89569) SHA1(68f32b00dd8ceb08e9677438c2bfec86e1a7fe6a) )
	ROM_LOAD( "15.72",	 0xe000, 0x0800, CRC(a3ebd1af) SHA1(a31eff7fe205efeb5b6fe1adaed66d2f23f91844) )
	ROM_LOAD( "15.73",	 0xe800, 0x0800, CRC(68682479) SHA1(5644e7f86a854572f4e54d6cf9552ef9ccbcb195) )
	ROM_LOAD( "15.74",	 0xf000, 0x0800, CRC(1d020dd0) SHA1(a36ae43587c0dd9abcb893db45715c405bb091fe) )
	ROM_LOAD( "15.75",	 0xf800, 0x0800, CRC(dba3ad3e) SHA1(4ef16e7b7456e189fcbf3ceba98955960d0953fc) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gpkcg.cgo",	 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gpkcg.cgc",	 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "gpkcg.cgb",	 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "gpkcg.cga",	 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg159 )	/* Cal Omega v15.9 (Wild Double-Up) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "15.93",	 0xd000, 0x1000, CRC(a1ca1fc0) SHA1(a751dba148c818a25237c2ac95328b6a5643f4e0) )
	ROM_LOAD( "15.94",	 0xe000, 0x1000, CRC(849595ea) SHA1(c28629de84b0c0d389c52490677ecb9139fd738d) )
	ROM_LOAD( "15.95",	 0xf000, 0x1000, CRC(c858ea24) SHA1(c7ecaddd4064b352c061b9164f2f347c310fab39) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "jkr2.cgo",	 0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "jkr2.cgc",	0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkr2.cgb",	0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkr2.cga",	0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "wildclr",	0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg164 )	/* Cal Omega v16.4 (Keno) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "keno16_4.02",	0xd800, 0x0800, CRC(fae6b065) SHA1(a123f98e4b4815a06d62d0429697cdce71756b94) )
	ROM_LOAD( "keno16_4.03",	0xe000, 0x0800, CRC(15974dbc) SHA1(fe2979861b8021949c127b182b9b50975b77bdd1) )
	ROM_LOAD( "keno16_4.04",	0xe800, 0x0800, CRC(64f06a75) SHA1(1cf4d89d2ee60200f84d47a3a637471e0af9239c) )
	ROM_LOAD( "keno16_4.05",	0xf000, 0x0800, CRC(3a8bc80e) SHA1(a1fb58b30850b1fe2bf976b8b99c8c540d67534a) )
	ROM_RELOAD(					0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgo",	 0x0000, 0x0800, BAD_DUMP CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )	// bad dump?

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgc",	 0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kenocg.cgb",	 0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kenocg.cga",	 0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg168 )	/* Cal Omega v16.8 (Keno) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "keno16_8.02",	0xe000, 0x0800, CRC(ba044cd6) SHA1(659ec979e61baf4e871af857b722bb6fd789ff75) )
	ROM_LOAD( "keno16_8.03",	0xe800, 0x0800, CRC(e3326b68) SHA1(7326f87319c363161ba8571dd983b070ef4f8694) )
	ROM_LOAD( "keno16_8.04",	0xf000, 0x0800, CRC(1f72acea) SHA1(26d0e5a36f14ccae22d216a13d0459f0389ea6c0) )
	ROM_LOAD( "keno16_8.05",	0xf800, 0x0800, CRC(4f38e3b3) SHA1(4d034959f665f0fdb5a4df85bae67dbd3d38077f) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgo",	 0x0000, 0x0800, BAD_DUMP CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )	// bad dump?

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgc",	 0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kenocg.cgb",	 0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kenocg.cga",	 0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg172 )	/* Cal Omega v17.2 (Double Double Poker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "17.23",	 0xd000, 0x1000, CRC(96efc8d1) SHA1(96836ca7188dc53e3a8af64f1fed9fe1c0c4e056) )
	ROM_LOAD( "17.24",	 0xe000, 0x1000, CRC(08d31a98) SHA1(85e23ffeb8fa82ec0155f54d2193511517e6ec8c) )
	ROM_LOAD( "17.25",	 0xf000, 0x1000, CRC(82508c71) SHA1(dd44e949f36e04ceaf1e527615c7003d0a9c0073) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "jkrpkr.cgo",	 0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "jkrpkr.cgc",	 0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkrpkr.cgb",	 0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkrpkr.cga",	 0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "wildclr",	0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg175 )	/* Cal Omega v17.5 (Gaming Draw Poker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "17.51",	0xd800, 0x0800, CRC(5450b90a) SHA1(4e67a17b2353bbe89fb6be4951efd09a948a987e) )
	ROM_LOAD( "17.52",	0xe000, 0x0800, CRC(29d787fd) SHA1(3f8c46f928c7fc3e68fe47efe23505f393bdb577) )
	ROM_LOAD( "17.53",	0xe800, 0x0800, CRC(080f6a23) SHA1(69866fba7cc348976ed1dfbfe103a5852a60d081) )
	ROM_LOAD( "17.54",	0xf000, 0x0800, CRC(7867815d) SHA1(8f3c4375f9b7c4eb5ebe413a8ac8593ff2ceb3e0) )
	ROM_LOAD( "17.55",	0xf800, 0x0800, CRC(656aa3e3) SHA1(2ccf1328d134cb26a5c4a0e45841079166de19be) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgo",	 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pokcgc",	 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "pokcgb",	 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "pokcga",	 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg176 )	/* Cal Omega 17.6 (Nudge Keno) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "17.61",	0xd800, 0x0800, CRC(7b201d27) SHA1(142d25c424e6bfd4327bb796f8dc9d0e6cb21797) )
	ROM_LOAD( "17.62",	0xe000, 0x0800, CRC(0ce87971) SHA1(ad1fecf4ed34eccd2a5b09e8847cb4a011875b73) )
	ROM_LOAD( "17.63",	0xe800, 0x0800, CRC(adce3e97) SHA1(5c12984b241c4884158ecc0743a3805115457277) )
	ROM_LOAD( "17.64",	0xf000, 0x0800, CRC(2a48525a) SHA1(9e48ca9fc3cc4c42dac3a24b2a6ac78204f7d2d8) )
	ROM_LOAD( "17.65",	0xf800, 0x0800, CRC(d4d929fa) SHA1(9cee198596375b810210856bc4ba5f5a3d706df4) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nbcg.cgo",	0x0000, 0x0800, CRC(b4b01f22) SHA1(1214d91ae28ac40ef37fce787783cd64331ed224) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "nbcg.cgc",	0x0000, 0x1000, CRC(f7ca234f) SHA1(3b2717051341c423969ed4bfb292a9f88119c2a8) )
	ROM_LOAD( "nbcg.cgb",	0x1000, 0x1000, CRC(9d409932) SHA1(d3ffca50a059278777238d206895a0d188f4ff6f) )
	ROM_LOAD( "nbcg.cga",	0x2000, 0x1000, CRC(afe1a666) SHA1(c1530700a283d18e7136754d45904930ef424bcf) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "bclr",		0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg181 )	/* Cal Omega 18.1 (Nudge Keno) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "18.12",	0xe000, 0x0800, CRC(dd867180) SHA1(caf703f45dea980e84fc29d2ea0d3f4e211aaa3f) )
	ROM_LOAD( "18.13",	0xe800, 0x0800, CRC(39ccbddd) SHA1(1c027957ad6a3346dd3bcc0b422d2e854c6f5439) )
	ROM_LOAD( "18.14",	0xf000, 0x0800, CRC(ddf23ef1) SHA1(8340a2b0125e42602045fea2a248f1ec9e9915c0) )
	ROM_LOAD( "18.15",	0xf800, 0x0800, CRC(549610b5) SHA1(371e4651a400d4c8b7181391c1d9b964d1ccb6fb) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "nbcg.cgo",	0x0000, 0x0800, CRC(b4b01f22) SHA1(1214d91ae28ac40ef37fce787783cd64331ed224) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "nbcg.cgc",	0x0000, 0x1000, CRC(f7ca234f) SHA1(3b2717051341c423969ed4bfb292a9f88119c2a8) )
	ROM_LOAD( "nbcg.cgb",	0x1000, 0x1000, CRC(9d409932) SHA1(d3ffca50a059278777238d206895a0d188f4ff6f) )
	ROM_LOAD( "nbcg.cga",	0x2000, 0x1000, CRC(afe1a666) SHA1(c1530700a283d18e7136754d45904930ef424bcf) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "bclr",		0x0000, 0x0100, CRC(0ec45d01) SHA1(da73ae7e1c74913921dc378a97795c6da47dcbfb) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg183 )	/* Cal Omega v18.3 (Pixels) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "18.32", 0xC000, 0x1000, CRC(c793ffc1) SHA1(f2ef82f92a9e18128d28973bcf050d0c3e1819f3) )
	ROM_LOAD( "18.33", 0xd000, 0x1000, CRC(415a6599) SHA1(648986310a3864652897e5d18b8be06819cce7a8) )
	ROM_LOAD( "18.34", 0xe000, 0x1000, CRC(fc5d3b89) SHA1(3601401d00d7a0621eac4254da238e9c8929cac4) )
	ROM_LOAD( "18.35", 0xf000, 0x1000, CRC(6552207f) SHA1(bd8a7546b4dad07d151dc7039028916386aee5e9) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cgo", 0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cga", 0x0000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )
	ROM_LOAD( "pixcg.cgb", 0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pixcg.cgc", 0x2000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pixclr",		0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg185 )	/* Cal Omega v18.5 (Pixels) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "18.52", 0xC000, 0x1000, CRC(19225f7d) SHA1(73d713ee86886f935d9b2c2ca670d8e00d466b7f) )
	ROM_LOAD( "18.53", 0xd000, 0x1000, CRC(797e2b70) SHA1(83f974c6c1886eab5c90782766b72900c73045e1) )
	ROM_LOAD( "18.54", 0xe000, 0x1000, CRC(6becc802) SHA1(e72783db52bffd3fa2f7d35cd8a004415e37b004) )
	ROM_LOAD( "18.55", 0xf000, 0x1000, CRC(313db76a) SHA1(d81228bf0badc8a3cf08c77de27b3d973953175c) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cgo", 0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cga", 0x0000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )
	ROM_LOAD( "pixcg.cgb", 0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pixcg.cgc", 0x2000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pixclr",		0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg186 )	/* Cal Omega v18.6 (Pixels) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "18.62", 0xC000, 0x1000, CRC(025f4268) SHA1(750a5417fe2b077893d8252dd5eafc42fafd965e) )
	ROM_LOAD( "18.63", 0xd000, 0x1000, CRC(948a6ef1) SHA1(6aeb244209f3376042a32d9accb38e3f09cb192a) )
	ROM_LOAD( "18.64", 0xe000, 0x1000, CRC(9fb6e82b) SHA1(db38564e8060f1c67183f3f412a24439b2253e13) )
	ROM_LOAD( "18.65", 0xf000, 0x1000, CRC(a68be5ef) SHA1(1ab2bc1b070863260fa281970f886d77e2fa7ccd) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cgo", 0x0000, 0x0800, CRC(4b487d88) SHA1(ae8bf1c84c475a70ea98eb8419a920389bac1761) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "pixcg.cga", 0x0000, 0x1000, CRC(d80f064a) SHA1(1b22ca3e446ed3c6fb49a90c463394dec96bc4ec) )
	ROM_LOAD( "pixcg.cgb", 0x1000, 0x1000, CRC(a3bed6b1) SHA1(078cface4af9720bee3288f5f0236725c8bfb575) )
	ROM_LOAD( "pixcg.cgc", 0x2000, 0x1000, CRC(9750eea2) SHA1(4f8b04a161501840ad2576379f23e8be2d46a488) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pixclr",		0x0000, 0x0100, CRC(67d23e76) SHA1(826cf77ca5a4d492d66e45ee96a7780a94fbe634) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg204 )	/* Cal Omega v20.4 (Super Blackjack) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "bj20.41",	0xb000, 0x1000, CRC(9c2203f7) SHA1(fd566683e887cf80cd0e6c82a413aebc378397f8) )
	ROM_LOAD( "bj20.42",	0xc000, 0x1000, CRC(31b37010) SHA1(c35ef77725c6c6dd9f369d50d9a8e55c2e3644af) )
	ROM_LOAD( "bj20.43",	0xd000, 0x1000, CRC(a9edf684) SHA1(7d2d8f3ec00c93914bfd629fae838b7d0f4bf414) )
	ROM_LOAD( "bj20.44",	0xe000, 0x1000, CRC(7a46e6cd) SHA1(91be6ddfd33ebab1f65e08f58c1be46fc770c1bf) )
	ROM_LOAD( "bj20.45",	0xf000, 0x1000, CRC(1e76202a) SHA1(015bf7ba62a561f5640aad7855e5fdd46787cdbf) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bj-jkr2.cgo",	0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "bj-jkr2.cgc",	0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "bj-jkr2.cgb",	0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "bj-jkr2.cga",	0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "mltclr",		0x0000, 0x0100, CRC(fefb0fa8) SHA1(66d86aa19d9d37ffd2840d6653fcec667bc716d4) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg208 )	/* Cal Omega v20.8 (Winner's Choice) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "20.81",	0xb000, 0x1000, CRC(938be03a) SHA1(606252b5160a7af340faa3d4ed6af9dff849c9ac) )
	ROM_LOAD( "20.82",	0xc000, 0x1000, CRC(7d42257c) SHA1(f1487bcc2475d1bcdfbc9bf866adcb0d510acef5) )
	ROM_LOAD( "20.83",	0xd000, 0x1000, CRC(b3b25958) SHA1(096f010db3164579ba81851a3f1062df0f46fc0f) )
	ROM_LOAD( "20.84",	0xe000, 0x1000, CRC(0bfb9f9d) SHA1(4a3c1e88faa90a9f4a483b053beb2c8af688d52d) )
	ROM_LOAD( "20.85",	0xf000, 0x1000, CRC(d61ba385) SHA1(0352a5306e942467d142982394d2d72892d1ecd6) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mlt2cg.cgo",	0x0000, 0x1000, CRC(43a095f0) SHA1(1824dd864bfc385035cc6b3da736ab6de9336440) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "mlt2cg.cgc",	0x0000, 0x1000, CRC(432ae4f9) SHA1(e4229ee743d0245b3efaed0d852bf24712508ce9) )
	ROM_LOAD( "mlt2cg.cgb",	0x1000, 0x1000, CRC(d5173679) SHA1(396c9c3eb7a0a5e5d279d079e635c8e4e5581779) )
	ROM_LOAD( "mlt2cg.cga",	0x2000, 0x1000, CRC(b7397d3a) SHA1(f35607a4cd60e4467e27474e8063b7a7a4a65d9f) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "mltclr",		0x0000, 0x0100, CRC(fefb0fa8) SHA1(66d86aa19d9d37ffd2840d6653fcec667bc716d4) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg227 )	/* Cal Omega v22.7 (Amusement Poker (Double Double)) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "22.73",	 0xd000, 0x1000, CRC(152d1ff9) SHA1(8bbfea1bae9e4fe2a2ac52507dc8dd0e33fbbd06) )
	ROM_LOAD( "22.74",	 0xe000, 0x1000, CRC(3af0f69d) SHA1(c34a0eab3ad4e4db310727805ba1ddc73533bfa6) )
	ROM_LOAD( "22.75",	 0xf000, 0x1000, CRC(d1f3fe24) SHA1(8b43b521fb1be8ef4286b4bfee99b654c49cf9de) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "jkrpkr.cgo",	 0x0000, 0x0800, CRC(987b3e4d) SHA1(ae4b5dc0e4d1195bbf18b375c1a4dbf880f5f38c) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "jkrpkr.cgc",	 0x0000, 0x0800, CRC(5b96f5e0) SHA1(4733349798eb059998a4814331f57e7f09e02490) )
	ROM_LOAD( "jkrpkr.cgb",	 0x0800, 0x0800, CRC(d77dda31) SHA1(e11b476cf0b609a8a40981b81b4d83b3c86678dc) )
	ROM_LOAD( "jkrpkr.cga",	 0x1000, 0x0800, CRC(def60756) SHA1(fe71424fc638761d9ff65391261a030a2889ad5e) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "wildclr",	0x0000, 0x0100, CRC(a26a8fae) SHA1(d570fe9443a0912bd34b81ac4c3e4c5f8901f523) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg236 )	/* Cal Omega v23.6 (Hotline) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "23.62",	 0xc000, 0x1000, CRC(6f3dd409) SHA1(8112c3114609317fe5543bf26fa426d36d60c5e1) )
	ROM_LOAD( "23.63",	 0xd000, 0x1000, CRC(76e96865) SHA1(03055751efa143cff0501aaa9b2beb9d533e13e7) )
	ROM_LOAD( "23.64",	 0xe000, 0x1000, CRC(26a18d82) SHA1(1ca036c014f180fb1720150642be3986c053c1c9) )
	ROM_LOAD( "23.65",	 0xf000, 0x1000, CRC(4e24d2d3) SHA1(56200760dced8b8bf2b38eee5c26a20ffc5b5442) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "hlcg.cgo",	 0x0000, 0x1000, CRC(b2871c94) SHA1(001c376479701efa4542c88f0eca9e13c694667c) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "hlcg.cgc",	 0x0000, 0x1000, CRC(6af901d9) SHA1(f9fcfcf2c92d1a3ef9a18b4e3e680c9de4829533) )
	ROM_LOAD( "hlcg.cgb",	 0x1000, 0x1000, CRC(db2d3eb7) SHA1(45f686edf7093069b44e895547c7ec67f820447d) )
	ROM_LOAD( "hlcg.cga",	 0x2000, 0x1000, CRC(a7e583fd) SHA1(d3b0aa9e24b6aedf24af55e5b149ab75d6f01a36) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "hlclr",		0x0000, 0x0100, CRC(1c994cda) SHA1(5c8698b4c5e43146106c9da8a306e3099b26ca2d) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg239 )	/* Cal Omega v23.9 (Gaming Draw Poker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "23-91.u5",	 0xd800, 0x0800, CRC(b49035e2) SHA1(b94a0245ca64d15b1496d1b272ffc0ce80f85526) )
	ROM_LOAD( "23-92.u6",	 0xe000, 0x0800, CRC(d9ffaa73) SHA1(e39d10121e16f89cd8d30a5391a14dc3d4b13a46) )
	ROM_LOAD( "23-93.u7",	 0xe800, 0x0800, CRC(f4e44280) SHA1(a03e5f03ed86c8ad7900fab0ef6a71c76eba3232) )
	ROM_LOAD( "23-94.u8",	 0xf000, 0x0800, CRC(8372f4d0) SHA1(de289b65cbe30c92b46fa87b9262ff7f9cfa0431) )
	ROM_LOAD( "23-9.u9",	 0xf800, 0x0800, CRC(bfcb934d) SHA1(b7cfa049bdd773368cb8326bcdfabbf474d15bb4) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cg-0.u67",	 0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cg-2c.u70",	 0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )
	ROM_LOAD( "cg-2b.u69",	 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )
	ROM_LOAD( "cg-2a.u68",	 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "82s129n.u28",	0x0000, 0x0100, CRC(6db5a344) SHA1(5f1a81ac02a2a74252decd3bb95a5436cc943930) )
	ROM_RELOAD(					0x0100, 0x0100 )
	ROM_RELOAD(					0x0200, 0x0100 )
	ROM_RELOAD(					0x0300, 0x0100 )
ROM_END

ROM_START( comg246 )	/* Cal Omega v24.6 (Hotline) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "24.62",	 0xc000, 0x1000, CRC(41f7b882) SHA1(4b532d70d5a7101952085a8fcdc0568c4266a72a) )
	ROM_LOAD( "24.63",	 0xd000, 0x1000, CRC(226580b7) SHA1(07e6332ace45e0ced57aed6e348ab12c1f07ff34) )
	ROM_LOAD( "24.64",	 0xe000, 0x1000, CRC(bf402e32) SHA1(436670b8f37caac14bb578a31dddbc0e2b0fd1ae) )
	ROM_LOAD( "24.65",	 0xf000, 0x1000, CRC(4eb99af3) SHA1(1af1a27f2575290aa0dce88840cd8be516a542ff) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "hlcg.cgo",	 0x0000, 0x1000, CRC(b2871c94) SHA1(001c376479701efa4542c88f0eca9e13c694667c) )

	ROM_REGION( 0x3000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "hlcg.cgc",	 0x0000, 0x1000, CRC(6af901d9) SHA1(f9fcfcf2c92d1a3ef9a18b4e3e680c9de4829533) )
	ROM_LOAD( "hlcg.cgb",	 0x1000, 0x1000, CRC(db2d3eb7) SHA1(45f686edf7093069b44e895547c7ec67f820447d) )
	ROM_LOAD( "hlcg.cga",	 0x2000, 0x1000, CRC(a7e583fd) SHA1(d3b0aa9e24b6aedf24af55e5b149ab75d6f01a36) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "hlclr",		0x0000, 0x0100, CRC(1c994cda) SHA1(5c8698b4c5e43146106c9da8a306e3099b26ca2d) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg272a )	/* Cal Omega v27.2 (Keno (amusement)) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "keno27_2.02",	0xe000, 0x0800, CRC(db3e1918) SHA1(4b8f33103f093ddbe750b536abc4545cf262d2e5) )
	ROM_LOAD( "keno27_2.03",	0xe800, 0x0800, CRC(c9e9cfd8) SHA1(dd0615ac579331330bda070f9ed68d7972436781) )
	ROM_LOAD( "keno27_2.04",	0xf000, 0x0800, CRC(d3fbab7f) SHA1(805510356bd52cf698a838daeaf16096eedcfd37) )
	ROM_LOAD( "keno27_2.05",	0xf800, 0x0800, CRC(22dc5b5e) SHA1(4442b7ac0516fdad8c81687fa9683eeace5b6c2d) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgo",	 0x0000, 0x0800, BAD_DUMP CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )	// bad dump?

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgc",	 0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kenocg.cgb",	 0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kenocg.cga",	 0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg272b )	/* Cal Omega v27.2 (Keno (gaming)) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "keno_x1.02m",	0xe000, 0x0800, CRC(6dc6ec9c) SHA1(dd80d0e544ad51d1b950970e735d7ce1e05062e3) )
	ROM_LOAD( "keno_x1.03m",	0xe800, 0x0800, CRC(242ce2ed) SHA1(e114cec7eb7554de14561e7c0e6aed01d7e72ca6) )
	ROM_LOAD( "keno_x1.04m",	0xf000, 0x0800, CRC(e191b0e1) SHA1(cc476efa194c1b2cd0035e9b4725e81d3a6f381c) )
	ROM_LOAD( "keno_x1.05m",	0xf800, 0x0800, CRC(4152b53d) SHA1(6fe577045d03362b8b988c7a9cd0542c9a20d7a7) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgo",	 0x0000, 0x0800, BAD_DUMP CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )	// bad dump?

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "kenocg.cgc",	 0x0000, 0x0800, CRC(fb721236) SHA1(33ef355913b8acb5017a24ca1c46dec1c391a528) )
	ROM_LOAD( "kenocg.cgb",	 0x0800, 0x0800, CRC(2b9205d9) SHA1(48ed4dcef38e9567246f09bd9bea5bf291e7e1b9) )
	ROM_LOAD( "kenocg.cga",	 0x1000, 0x0800, CRC(c4491e35) SHA1(44acb8bd7af287350b99d159b6f83015fcdbd93c) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "pokerclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg903d )	/* Cal Omega 903d (System 903 diag.PROM) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "903diag",	0x3800, 0x0800, CRC(f8092cea) SHA1(0c864419a4e1956c030b185739eca59313f20e8a) )
	ROM_RELOAD(				0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "testcgo",	0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_FILL(				0x0000, 0x1800, 0xff )	/* removed all roms, requested by the instructions sheet */

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "testclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( comg905d )	/* Cal Omega 905d (System 905 diag.PROM) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "905diag",	0xf000, 0x1000, CRC(6c20dbc7) SHA1(dbab0d2cf07bade2e3619bd5f29d406f3498a278) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "testcgo",	0x0000, 0x0800, CRC(b626ad89) SHA1(551b75f4559d11a4f8f56e38982114a21c77d4e7) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_FILL(				0x0000, 0x1800, 0xff )	/* removed all roms, requested by the instructions sheet */

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "testclr",	0x0000, 0x0100, CRC(a8191ef7) SHA1(d6f777980179ab091e2713ee815d46bf9c0ac486) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

/* Unofficial/3rd part games */

ROM_START( elgrande )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "d1.u6",	0x2000, 0x0800, CRC(8b6b505c) SHA1(5f89bb1b50b9dfacf23c50e3016b9258b0e15084) )
	ROM_LOAD( "d1.u7",	0x2800, 0x0800, CRC(d803a978) SHA1(682b73c968ef57007397d3e5eb0e78a97722da5e) )
	ROM_LOAD( "d1.u8",	0x3000, 0x0800, CRC(291fa93b) SHA1(1d57f736b11ddc916effde78e2cd08c313a62901) )
	ROM_LOAD( "d1.u9",	0x3800, 0x0800, CRC(ec3309a7) SHA1(b8ab7f3f2edf2658ea633b2b557ea37517615399) )
	ROM_RELOAD(			0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "d1.u67",	0x0000, 0x0800, CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "d1.u70",	0x0000, 0x0800, CRC(4f12d424) SHA1(c43f1df757ac7dd76875245e73d47451d1f7f6f2) )
	ROM_LOAD( "d1.u69",	0x0800, 0x0800, CRC(ed3c83b7) SHA1(93e2134de3d9f79a6cff0391c1a32fccd3840c3f) )
	ROM_LOAD( "d1.u68",	0x1000, 0x0800, BAD_DUMP CRC(3ab70570) SHA1(5ff84015a78d15a5207499f84ce637e49bca136f) ) /* bad bits */

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "d1.u28",		0x0000, 0x0200, CRC(a6d43709) SHA1(cbff2cb60137462dc0b7c7719a64574218d96c62) )
	ROM_RELOAD(				0x0200, 0x0200 )
ROM_END

ROM_START( jjpoker )	/* tuni-83 */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "tuni-83.u5",	 0x1800, 0x0800, CRC(46c542ee) SHA1(0b3832d8ab69427298de03d18984a220a9a35c30) )
	ROM_LOAD( "tuni-83.u6",	 0x2000, 0x0800, CRC(e24b392a) SHA1(3b705b5cc60d2b33375f52958b72e70ce36fa216) )
	ROM_LOAD( "tuni-83.u7",	 0x2800, 0x0800, CRC(c49e4e37) SHA1(d70321e87e6192bc65950153c13c049a52000148) )
	ROM_LOAD( "tuni-83.u8",	 0x3000, 0x0800, CRC(7a20d8ff) SHA1(d9c08c5dd5fe8d9f67d4fe031b47c3dbfcc717c1) )
	ROM_LOAD( "tuni-83.u9",	 0x3800, 0x0800, CRC(8cba33a6) SHA1(6998a47521f8d40425f68444bb9a45df72276c6b) )
	ROM_RELOAD(				 0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u67", 0x0000, 0x0800, CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u70", 0x0000, 0x0800, CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69", 0x0800, 0x0800, CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68", 0x1000, 0x0800, CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "tunipoker",	0x0000, 0x0100, CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( jjpokerb )	/* pokr_j */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pokr_j.01",	 0x1800, 0x0800, CRC(d0004eda) SHA1(2fd39213e3028066fd4f9b8db206f036b566e2f7) )
	ROM_LOAD( "pokr_j.02",	 0x2000, 0x0800, CRC(6809ccd9) SHA1(2573194a13ddf0270bccd456bda84b822036c660) )
	ROM_LOAD( "pokr_j.03",	 0x2800, 0x0800, CRC(20c4519a) SHA1(c58662a6404060c2d1e60953a0bf69fee5657694) )
	ROM_LOAD( "pokr_j.04",	 0x3000, 0x0800, CRC(d127fcbf) SHA1(ebf8ab47233c2cfe54cb8e36ec2ff7bf5f8631a5) )
	ROM_LOAD( "pokr_j.05",	 0x3800, 0x0800, CRC(bb376541) SHA1(1b4f91cef36d6decd2b1da9765af4e5ec976b2fc) )
	ROM_RELOAD(				 0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u67", 0x0000, 0x0800, CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u70", 0x0000, 0x0800, CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69", 0x0800, 0x0800, CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68", 0x1000, 0x0800, CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "tunipoker",	0x0000, 0x0100, CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( ssipkr24 )	/* pokr02_4 (gfx and prom from jjpoker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pokr02_4.01",	 0x1800, 0x0800, CRC(8adf1d6d) SHA1(d83677eed9426841767d947919f6da671b5fbed4) )
	ROM_LOAD( "pokr02_4.02",	 0x2000, 0x0800, CRC(5298a01c) SHA1(a0085498699bc15cc6ada9e4e9541bd84b97eeae) )
	ROM_LOAD( "pokr02_4.03",	 0x2800, 0x0800, CRC(30b5ead6) SHA1(7650ebb5f17eae17f3a0ddee67432a7f9dbf1c13) )
	ROM_LOAD( "pokr02_4.04",	 0x3000, 0x0800, CRC(ade57860) SHA1(ee80e97302a4d6371fde5bacb58747075976f617) )
	ROM_LOAD( "pokr02_4.05",	 0x3800, 0x0800, CRC(ad15250b) SHA1(d006657df1d2e01e33a3efb906e4532a2cd5b85d) )
	ROM_RELOAD(				 0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u67", 0x0000, 0x0800, BAD_DUMP CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u70", 0x0000, 0x0800, BAD_DUMP CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69", 0x0800, 0x0800, BAD_DUMP CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68", 0x1000, 0x0800, BAD_DUMP CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "tunipoker",	0x0000, 0x0100, BAD_DUMP CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( ssipkr30 )	/* pokr03_0 (gfx and prom from jjpoker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pokr03_0.01",	 0x1800, 0x0800, CRC(db9581fe) SHA1(605b254e0ebb96423eb522ce75242083d70f01ca) )
	ROM_LOAD( "pokr03_0.02",	 0x2000, 0x0800, CRC(861243ad) SHA1(290eba5c820177669e5adeac1e2f172b73789542) )
	ROM_LOAD( "pokr03_0.03",	 0x2800, 0x0800, CRC(b5154dc0) SHA1(3b517a8d82db62be68427378fe92f0225174ba00) )
	ROM_LOAD( "pokr03_0.04",	 0x3000, 0x0800, CRC(93bdef84) SHA1(3ea10a4d4d164f63082fbff4ae9b9fd14b5bcb27) )
	ROM_LOAD( "pokr03_0.05",	 0x3800, 0x0800, CRC(bd2ffd49) SHA1(b60c3866b11acd5053ec6dc5c37c0e322cb29478) )
	ROM_RELOAD(				 0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u67", 0x0000, 0x0800, BAD_DUMP CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u70", 0x0000, 0x0800, BAD_DUMP CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69", 0x0800, 0x0800, BAD_DUMP CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68", 0x1000, 0x0800, BAD_DUMP CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "tunipoker",	0x0000, 0x0100, BAD_DUMP CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END

ROM_START( ssipkr40 )	/* (gfx and prom from jjpoker) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "40-1.903",	 0x1800, 0x0800, CRC(461eb68c) SHA1(54781670930c723c993ca9ad80e06e38ddd2f035) )
	ROM_LOAD( "40-2.903",	 0x2000, 0x0800, CRC(099094a9) SHA1(c5a6ccb5ec0bebc79ef0b9c98595ef87c65ce361) )
	ROM_LOAD( "40-3.903",	 0x2800, 0x0800, CRC(1c923554) SHA1(d0050a8833f9a1a5fa0598b06a7bb265f0e814e4) )
	ROM_LOAD( "40-4.903",	 0x3000, 0x0800, CRC(552bf73d) SHA1(bf9197aab029c8dfaac88abcbda57547845323da) )
	ROM_LOAD( "40-5.903",	 0x3800, 0x0800, CRC(4d388d13) SHA1(8d46d6c227fe22f0433f02909b172f60cada1dd4) )
	ROM_RELOAD(				 0xf800, 0x0800 )    /* for vectors/pointers */

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u67", 0x0000, 0x0800, BAD_DUMP CRC(a8ac979d) SHA1(f7299d3f7c4aded028a65ae4365c174f0e953824) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tuni-83.u70", 0x0000, 0x0800, BAD_DUMP CRC(c131bf96) SHA1(3fb6717955a7312061395e5770c0f1ca9716d77c) )
	ROM_LOAD( "tuni-83.u69", 0x0800, 0x0800, BAD_DUMP CRC(3483b4fb) SHA1(ac04b68c5fb8f8f142582181ad13bee87636cead) )
	ROM_LOAD( "tuni-83.u68", 0x1000, 0x0800, BAD_DUMP CRC(e055a148) SHA1(d80e4330dce96b98df5bec731876f185476d6058) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "tunipoker",	0x0000, 0x0100, BAD_DUMP CRC(5101a33b) SHA1(a36bc421064d0ed96beb27b549f69adce0a553c2) )
	ROM_RELOAD(				0x0100, 0x0100 )
	ROM_RELOAD(				0x0200, 0x0100 )
	ROM_RELOAD(				0x0300, 0x0100 )
ROM_END


/*************************
*      Driver Init       *
*************************/

static DRIVER_INIT( calomega )
{
	/* Palette transformed by PLDs? */
	int x;
	UINT8 *BPR = memory_region( REGION_PROMS );

	for (x=0x0000;x<0x0400;x++)
	{
		if (BPR[x] == 0x07)
			BPR[x] = 0x04;	/* blue background */
	}

	/* Initializing PIAs... */
	pia_config(0, &pia0_intf);
	pia_config(1, &pia1_intf);
}

static DRIVER_INIT( elgrande )
{
	int x;
	UINT8 *BPR = memory_region( REGION_PROMS );
	UINT8 *ROM = memory_region( REGION_GFX2 );

	/* Palette transformed by PLDs? */
	for (x=0x0000;x<0x0400;x++)
	{
		if (BPR[x] == 0x07)
			BPR[x] = 0x00; /* black background */
	}

	/* Temporary patch to fix some bad bits in ROM d1.u68 till a correct dump appear */
	ROM[0x171c] = 0xff;
	ROM[0x171d] = 0xff;
	ROM[0x171e] = 0xff;
	ROM[0x1737] = 0xff;
	ROM[0x173d] = 0xff;

	/* Initializing PIAs... */
	pia_config(0, &pia0_intf);
	pia_config(1, &pia1_intf);
}

static DRIVER_INIT( jjpoker )
{
	/* Palette transformed by PLDs? */
	int x;
	UINT8 *BPR = memory_region( REGION_PROMS );

	for (x=0x0000;x<0x0400;x++)
	{
		if (BPR[x] == 0x02)
			BPR[x] = 0x00;	/* blue background */
	}

	/* Initializing PIAs... */
	pia_config(0, &pia0_intf);
	pia_config(1, &pia1_intf);
}

static DRIVER_INIT( standard )
{
	/* Palette transformed by PLDs? */
	int x;
	UINT8 *BPR = memory_region( REGION_PROMS );

	for (x=0x0000;x<0x0400;x++)
	{
		if (BPR[x] == 0x07)
			BPR[x] = 0x04;	/* blue background */
	}

	/* Initializing PIAs... */
	pia_config(0, &pia0_intf);
	pia_config(1, &pia1_intf);
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT    MACHINE   INPUT     INIT      ROT    COMPANY                                 FULLNAME                                     FLAGS   */
GAME( 1981, comg076,  0,        sys903,   comg076,  calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 7.6 (Arcade Poker)",           0 )
GAME( 1981, comg079,  0,        sys903,   comg076,  calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 7.9 (Arcade Poker)",           0 )
GAME( 1981, comg094,  0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 9.4 (Keno)",                   GAME_NOT_WORKING )
GAME( 1982, comg107,  0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 10.7 (Big Game)",              GAME_NOT_WORKING )
GAME( 1982, comg125,  0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 12.5 (Bingo)",                 GAME_NOT_WORKING )
GAME( 1982, comg127,  0,        sys903,   standard, standard, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 12.7 (Keno)",                  GAME_NOT_WORKING )
GAME( 1982, comg128,  0,        sys903,   comg128,  standard, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 12.8 (Arcade Game)",           0 )
GAME( 1982, comg134,  0,        sys903,   standard, standard, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 13.4 (Nudge Bingo)",           GAME_NOT_WORKING )
GAME( 1982, comg145,  0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 14.5 (Pixels)",                GAME_NOT_WORKING )
GAME( 1983, comg157,  0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 15.7 (Double-Draw Poker)",     GAME_NOT_WORKING )
GAME( 1983, comg159,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 15.9 (Wild Double-Up)",        GAME_NOT_WORKING )
GAME( 1983, comg164,  0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 16.4 (Keno)",                  GAME_NOT_WORKING )
GAME( 1983, comg168,  0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 16.8 (Keno)",                  GAME_NOT_WORKING )
GAME( 1983, comg172,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 17.2 (Double Double Poker)",   GAME_NOT_WORKING )
GAME( 1984, comg175,  0,        sys903,   gdrawpkr, calomega, ROT0, "Cal Omega / Casino Electronics Inc.",   "Cal Omega - Game 17.51 (Gaming Draw Poker)",    0 )
GAME( 1982, comg176,  0,        sys903,   standard, standard, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 17.6 (Nudge Bingo)",           GAME_NOT_WORKING )
GAME( 1982, comg181,  0,        sys903,   standard, standard, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.1 (Nudge Bingo)",           GAME_NOT_WORKING )
GAME( 1983, comg183,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.3 (Pixels)",                GAME_NOT_WORKING )
GAME( 1983, comg185,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.5 (Pixels)",                GAME_NOT_WORKING )
GAME( 1983, comg186,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 18.6 (Pixels)",                GAME_NOT_WORKING )
GAME( 1984, comg204,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 20.4 (Super Blackjack)",       GAME_NOT_WORKING )
GAME( 1984, comg208,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 20.8 (Winner's Choice)",       GAME_NOT_WORKING )
GAME( 1984, comg227,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 22.7 (Amusement Poker, d/d)",  GAME_NOT_WORKING )
GAME( 1984, comg236,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 23.6 (Hotline)",               GAME_NOT_WORKING )
GAME( 1985, comg239,  0,        sys903,   gdrawpkr, calomega, ROT0, "Cal Omega / Casino Electronics Inc.",   "Cal Omega - Game 23.9 (Gaming Draw Poker)",     0 )
GAME( 1985, comg246,  0,        sys905,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 24.6 (Hotline)",               GAME_NOT_WORKING )
GAME( 1985, comg272a, 0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 27.2 (Keno, amusement)",       GAME_NOT_WORKING )
GAME( 1985, comg272b, 0,        sys903,   standard, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - Game 27.2 (Keno, gaming)",          GAME_NOT_WORKING )
GAME( 198?, comg903d, 0,        sys903,   gdrawpkr, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - System 903 Diag.PROM",              GAME_NOT_WORKING )
GAME( 198?, comg905d, 0,        sys905,   gdrawpkr, calomega, ROT0, "Cal Omega Inc.",                        "Cal Omega - System 905 Diag.PROM",              GAME_NOT_WORKING )
GAME( 1982, elgrande, 0,        sys903,   elgrande, elgrande, ROT0, "Tuni Electro Service / E.T. Marketing", "El Grande - 5 Card Draw (New)",                 0 )
GAME( 1983, jjpoker,  0,        sys903,   jjpoker,  jjpoker,  ROT0, "Enter-Tech (ETL)",                      "Jackpot Joker Poker (set 1)",                   0 )
GAME( 1983, jjpokerb, jjpoker,  sys903,   jjpoker,  jjpoker,  ROT0, "Enter-Tech (ETL)",                      "Jackpot Joker Poker (set 2)",                   0 )
GAME( 1988, ssipkr24, jjpoker,  sys903,   ssipkr,   jjpoker,  ROT0, "SSI",                                   "SSI Poker (v2.4)",                              0 )
GAME( 1988, ssipkr30, jjpoker,  sys903,   ssipkr,   jjpoker,  ROT0, "SSI",                                   "SSI Poker (v3.0)",                              0 )
GAME( 1990, ssipkr40, jjpoker,  sys903,   ssipkr,   jjpoker,  ROT0, "SSI",                                   "SSI Poker (v4.0)",                              0 )
