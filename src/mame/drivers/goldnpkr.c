/******************************************************************************

    GOLDEN POKER DOUBLE UP (BONANZA ENTERPRISES, LTD)
    -------------------------------------------------

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Golden Poker Double Up (Big Boy).         1981, Bonanza Enterprises, Ltd.
    * Golden Poker Double Up (Mini Boy).        1981, Bonanza Enterprises, Ltd.
    * PlayMan Poker (german).                   1981, PlayMan.
    * Jack Potten's Poker (set 1).              198?, Bootleg.
    * Jack Potten's Poker (set 2).              198?, Bootleg in Coinmaster H/W.
    * Jack Potten's Poker (set 3).              198?, Bootleg.
    * Jack Potten's Poker (set 4).              198?, Bootleg.
    * Jack Potten's Poker (set 5).              198?, Bootleg.
    * Jack Potten's Poker (set 6).              198?, Bootleg.
    * Good Luck.                                198?, Unknown.
    * Royale (set 1).                           198?, Unknown.
    * Royale (set 2).                           198?, Unknown.
    * Witch Card (Video Klein).                 1991, Video Klein.
    * Witch Card (Spanish, witch game, set 1).  1991, Unknown.
    * Witch Card (Spanish, witch game, set 2).  1991, Unknown.
    * Witch Card (English, no witch game).      1991, Unknown.
    * Witch Card (German, set 1).               1994, Proma.
    * Witch Card (German, set 2).               1994, Unknown.
    * Witch Card (English, witch game, lamps).  1985, PlayMan.
    * Super Loco 93 (Spanish, set 1).           1993, Unknown.
    * Super Loco 93 (Spanish, set 2).           1993, Unknown.
    * Maverik.                                  198?, Unknown.
    * Brasil'89.                                1989, Unknown.
    * Poker 91.                                 1991, Unknown.
    * Buena Suerte (Spanish, set 1).            1990, Unknown.
    * Buena Suerte (Spanish, set 2).            1991, Unknown.
    * Buena Suerte (Spanish, set 3).            1991, Unknown.
    * Buena Suerte (Spanish, set 4).            1991, Unknown.
    * Buena Suerte (Spanish, set 5).            1991, Unknown.
    * Buena Suerte (Spanish, set 6).            1991, Unknown.
    * Buena Suerte (Spanish, set 7).            1991, Unknown.
    * Buena Suerte (Spanish, set 8).            1991, Unknown.
    * Buena Suerte (Spanish, set 9).            1991, Unknown.
    * Buena Suerte (Spanish, set 10).           1991, Unknown.
    * Buena Suerte (Spanish, set 11).           1991, Unknown.
    * Buena Suerte (Spanish, set 12).           1991, Unknown.
    * Buena Suerte (Spanish, set 13).           1991, Unknown.
    * Buena Suerte (Spanish, set 14).           1991, Unknown.
    * Buena Suerte (Spanish, set 15).           1991, Unknown.
    * Buena Suerte (Spanish, set 16).           1991, Unknown.
    * Buena Suerte (Spanish, set 17).           1991, Unknown.
    * Buena Suerte (Spanish, set 18).           1991, Unknown.
    * Buena Suerte (Spanish, set 19).           1991, Unknown.
    * Buena Suerte (Spanish, set 20).           1991, Unknown.
    * Buena Suerte (Spanish, set 21).           1991, Unknown.
    * Buena Suerte (Spanish, set 22).           1991, Unknown.


*******************************************************************************


    I think "Diamond Poker Double Up" from Bonanza Enterprises should run on this hardware too.
    http://www.arcadeflyers.com/?page=thumbs&id=4539

    Big-Boy and Mini-Boy are different sized cabinets for Bonanza Enterprises games.
    http://www.arcadeflyers.com/?page=thumbs&id=4616
    http://www.arcadeflyers.com/?page=thumbs&id=4274


    Preliminary Notes (pmpoker):

    - This set was found as "unknown playman-poker".
    - The ROMs didn't match any currently supported set (0.108u2 romident switch).
    - All this driver was made using reverse engineering in the program roms.


    Game Notes:
    ==========

    * goldnpkr & goldnpkb:

    "How to play"... (from "Golden Poker Double Up" instruction card)

    1st GAME
    - Insert coin / bank note.
    - Push BET button, 1-10 credits multiple play.
    - Push DEAL/DRAW button.
    - Push HOLD buttons to hold cards you need.
    - Cards held can be cancelled by pushing CANCEL button.
    - Push DEAL/DRAW button to draw cards.

    2nd GAME - Double Up game
    - When you win, choose TAKE SCORE or DOUBLE UP game.
    - Bet winnings on
        "BIG (8 or more number)" or
        "SMALL (6 and less number)" of next one card dealt.
    - Over 5,000 winnings will be storaged automatically.


    ----- Learn Mode (settings) -----
    Press LEARN (F2) to enter the learn mode for settings.
    This is a timed function and after 30-40 seconds switch back to the game.

    Press DOUBLE UP to change Double Up 7 settings between 'Even' and 'Lose'.
    Press BET to adjust the maximum bet (20-200).
    Press HOLD4 for Meter Over (5000-50000).
    Press BIG to change Double Up settings (Normal-Hard).
    Press TAKE SCORE to set Half Gamble (Yes/No).
    Press SMALL to set win sound (Yes/No).
    Press HOLD1 to set coinage 1.
    Press HOLD2 to set coinage 2.
    Press HOLD3 to set coinage 3.
    Press HOLD5 to exit.

    ----- Meters Mode -----
    Press METER SW (9) to enter the Meters mode. You also can switch between interim
    and permanent meters using METER SW (only for goldnpkr).
    This is a timed function and after 30-40 seconds switch back to the game.

    To reset meters push CANCEL + SMALL buttons. HOLD5 to exit.
    In goldnpkr you can switch between Permanent/Interim Meters.
    In goldnpkb & pmpoker you can see only Permanent Meters.

    ----- Percentage Mode -----
    Press Meter SW (9), then DEAL/DRAW, to enter the percentage mode.
    Press HOLD4 to change the value following the table below (from manual). HOLD1 to exit.

    Number    Overall scoring percentage in the LONG RUN
    - - - - - - - - - - - - - - - - - - - - - - - - - - -
      0         about 85%
      1         about 30%
      2         about 40%
      3         about 50%

    ----- Test Mode -----
    Press Meter SW (9), then DEAL/DRAW, then HOLD5 to enter the test mode.
    After a RAM test, you can see an input test matrix. Press HOLD1+HOLD2+HOLD3 to exit
    entering into a video grid test. Press HOLD5 to exit.


    * Good Luck

    This hybrid runs on Witch Card hardware.
    Even when is shown on screen "Bet 1 to 10", you can bet up to 50.
    There are extra graphics for a couple of jokers, but they never are shown.
    Maybe some settings can enable the use of them...


    * Witch Card (spanish sets)

    This game is derivated from Golden Poker.

    The hardware has a feature called BLUE KILLER.
    Using the original intensity line, the PCB has a bridge
    that allow (as default) turn the background black.

    Except goodluck, all other games running in this hardware
    were designed to wear black background.

    - Settings:

    There are 12 parameters to program. All of them are unknown.
    To program them, enter the settings mode with F2 and use the HOLD keys,
    CANCEL key, DEAL + HOLD keys, and DEAL + CANCEL. To exit the mode press BET.

    Settings are still unknown, but put the max value for each parameter to allow
    all the game features.

    First Line:  32 32 32 32 32 64
    Second Line: 40 40 40 40 50 16

    - Play:

    The game is like other poker games, but with 2 jokers.
    Each time you win a hand, a double up game appear...

    - Double Up:

    You must to choose High or Low to guess the card. If you win,
    you can take the credits or continue into double up.
    Each time you win a double up hand, the card is indicated on the screen.
    The first 3 consecutive winning hands will add a witch with a running number.
    Further winning hands only add the card/kind indication. When you lose, or
    take your credits, the 3 witches start to roll their own numbers like a
    slot machine. There is an attempt for each winning hand. the bonus ends when
    you win a prize, or when all attempts are done.


    * Super Loco 93

    I like this game!... This one has nothing to do with poker games.
    The objective is to get the higher sum with 2 or 3 cards. Is clearly
    based on a passage (Envido) of the famous argentine's game called "Truco".

    - How to play?...

    Like in Truco's envido, You must add 20 to the sum of your cards.
    If you have 7 and 5 of the same kind, you have 7 + 5 + 20 = 32 points.

    "Flor", is 3 cards of the same kind.
    "Simple", is 2 cards of the same kind.

    HAND             WIN    DESCRIPTION
    ----------------------------------------------
    38  Corazones   1000    7, 6 and 5 of hearts.
    777 Loco Loco    200    3x sevens.
    38  Flor          80    7, 6 and 5 of the same kind (except hearts).
    37  Flor          20    7, 6 and 4 of the same kind.
    36  Flor          14    7, 5 and 4 of the same kind.
    35  Flor          10    6, 5 and 4 of the same kind.
    32-33-34 Flor      6    3 of the same kind that sum 32, 33 or 34.
    33  Simple         4    7 and 6 of the same kind.
    32  Simple         2    7 and 5 of the same kind.

    After bet (apuesta) some credits, Press deal (reparte) button.
    The game will deals 3 cards. You can discard up to all your 3 cards.
    Pressing the deal button again, new cards will appear in the place
    of the previously selected cards.

    - Double Up:

    You must to choose Red or Black to guess the card.
    If you win, you can take the credits or continue into double up.
    The rest is similar to Witch Card, but with 3 big numbers instead of
    witches. Once you lose or take your credits, the big numbers start to
    run 'alla' slot game, giving 1 attempt by each time you won a double-up
    hand.

    - Settings:

    There are 12 parameters to program.
    To program them, use the HOLD keys, CANCEL key, DEAL + HOLD keys,
    and DEAL + CANCEL.



*******************************************************************************


    Hardware Notes (pmpoker):

    - CPU:            1x M6502.
    - Video:          1x MC6845.
    - RAM:            4x uPD2114LC
    - I/O             2x 6821 PIAs.
    - prg ROMs:       3x 2732 (32Kb) or similar.
    - gfx ROMs:       4x 2716 (16Kb) or similar.
    - sound:          (discrete).
    - battery backup: 2x S8423


    PCB Layout (pmpoker): (XX) = unreadable.
     _______________________________________________________________________________
    |   _________                                                                   |
    |  |         |               -- DIP SW x8 --                                    |
    |  | Battery |   _________   _______________   _________  _________   ________  |
    |  |   055   |  | 74LS32  | |1|2|3|4|5|6|7|8| | HCF4011 || HCF4096 | | LM339N | |
    |  |_________|  |_________| |_|_|_|_|_|_|_|_| |_________||_________| |________| |
    |       _________     _________   _________   _________                         |
    |      | 74LS138 |   | S-8423  | | 74LS08N | | 74LS(XX)|                        |
    |      |_________|   |_________| |_________| |_________|                        |
    |  _______________    _________   ____________________                      ____|
    | |               |  | S-8423  | |                    |                    |
    | |     2732      |  |_________| |       6502P        |                    |
    | |_______________|   _________  |____________________|                    |
    |  _______________   |  7432   |  ____________________                     |____
    | |               |  |_________| |                    |                     ____|
    | |     2732      |   _________  |       6821P        |                     ____|
    | |_______________|  | 74LS157 | |____________________|                     ____|
    |  _______________   |_________|  ____________________                      ____|
    | |               |   _________  |                    |                     ____|
    | |     2732      |  | 74LS157 | |       6821P        |                     ____|
    | |_______________|  |_________| |____________________|                     ____|
    |  _______________    _________   ____________________                      ____|
    | |               |  | 74LS157 | |                    |                     ____|
    | |     2732      |  |_________| |       6845SP       |                     ____|
    | |_______________|   _________  |____________________|                     ____|
    |                    | 2114-LC |                                            ____| 28x2
    |                    |_________|                                            ____| connector
    |       _________     _________                                             ____|
    |      | 74LS245 |   | 2114-LC |                                            ____|
    |      |_________|   |_________|                                            ____|
    |       _________     _________               _________                     ____|
    |      | 74LS245 |   | 2114-LC |             | 74LS174 |                    ____|
    |      |_________|   |_________|             |_________|                    ____|
    |  ________________   _________   _________   _________                     ____|
    | |                | | 2114-LC | | 74LS08H | | TI (XX) | <-- socketed.      ____|
    | |      2716      | |_________| |_________| |_________|       PROM         ____|
    | |________________|              _________   _________                     ____|
    |  ________________              | 74LS04P | | 74LS174 |                    ____|
    | |                |             |_________| |_________|                    ____|
    | |      2716      |              _________   _________                     ____|
    | |________________|             | 74166P  | | 74LS86C |                    ____|
    |  ________________              |_________| |_________|                    ____|
    | |                |              _________    _______                     |
    | |      2716      |             | 74166P  |  | 555TC |                    |
    | |________________|             |_________|  |_______|                    |
    |  ________________                                                        |____
    | |                |                                                        ____|
    | |      2716      |              _________   _________      ________       ____| 5x2
    | |________________|             | 74166P  | |  7407N  |    | LM380N |      ____| connector
    |                                |_________| |_________|    |________|      ____|
    |  ________  ______               _________   _________      ___            ____|
    | | 74LS04 || osc. |             | 74LS193 | |  7407N  |    /   \          |
    | |________||10 MHz|             |_________| |_________|   | POT |         |
    |           |______|                                        \___/          |
    |__________________________________________________________________________|



    Some odds:

    - There are unused pieces of code like the following sub:

    78DE: 18         clc
    78DF: 69 07      adc  #$07
    78E1: 9D 20 10   sta  $1020,x
    78E4: A9 00      lda  #$00
    78E6: 9D 20 18   sta  $1820,x
    78E9: E8         inx

    78EA: 82         DOP        ; use of DOP (double NOP)
    78EB: A2 0A      dummy (ldx #$0A)

    78ED: AD 82 08   lda  $0882

    78F0: 82         DOP        ; use of DOP (double NOP)
    78F1: 48 08      dummy
    78F3: D0 F6      bne  $78EB ; branch to the 1st DOP dummy arguments (now ldx #$0A).
    78F5: CA         dex
    78F6: D0 F8      bne  $78F0
    78F8: 29 10      and  #$10
    78FA: 60         rts

    Offset $78EA and $78F0 contains an undocumented 6502 opcode (0x82).

    At beginning, I thought that the main processor was a 65sc816, since 0x82 is a documented opcode (BRL) for this CPU.
    Even the vector $FFF8 contain 0x09 (used to indicate "Emulation Mode" for the 65sc816).
    I dropped the idea following the code. Impossible to use BRL (branch relative long) in this structure.

    Some 6502 sources list the 0x82 mnemonic as DOP (double NOP), with 2 dummy bytes as argument.
    The above routine dynamically change the X register value using the DOP undocumented opcode.
    Since the opcode DOP in fact has only 1 dummy byte as argument, they apparently dropped this
    piece of code due to it didn't work as expected. Now all have sense.


*******************************************************************************


    -----------------------------------------------
    ***  Memory Map (pmpoker/goldnpkr hardware) ***
    -----------------------------------------------

    $0000 - $00FF   RAM     ; Zero Page (pointers and registers)

                            ; $45 to $47 - Coin settings.
                            ; $50 - Input port register.
                            ; $5C - Input port register.
                            ; $5D - Input port register.
                            ; $5E - Input port register.
                            ; $5F - Input port register.

    $0100 - $01FF   RAM     ; 6502 Stack Pointer.

    $0200 - $02FF   RAM     ; R/W. (settings)
    $0300 - $03FF   RAM     ; R/W (mainly to $0383). $0340 - $035f (settings).

    $0800 - $0801   MC6845  ; MC6845 use $0800 for register addressing and $0801 for register values.

                            *** pmpoker mc6845 init at $65B9 ***
                            *** goldnpkr mc6845 init at $5E75 ***
                            *** sloco93 mc6845 init at $D765 ***
                            register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
                            value:     0x27  0x20  0x22  0x02  0x1F  0x04  0x1D  0x1E  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.

                            *** goodluck mc6845 init at $527B ***
                            register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
                            value:     0x27  0x20  0x23  0x03  0x1F  0x04  0x1D  0x1F  0x00  0x00  0x26  0x00  0x20  0x22  0x58  0xA5  0x4F  0xC9.

                            *** witchcrd mc6845 init at $D765 ***
                            register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
                            value:     0x27  0x20  0x23  0x03  0x1F  0x04  0x1D  0x1F  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.

                            *** witchcrd (Video Klein) mc6845 init at $627B ***
                            register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
                            value:     0x27  0x20  0x23  0x33  0x1F  0x04  0x1D  0x1F  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.

                            *** witchcrd (Video Klein) mc6845 mod at $6293 ($2000 and #$20) ***
                            register:   04    05    06    07
                            value:     0x26  0x00  0x20  0x22

                            *** royale mc6845 init at $6581 ***
                            register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
                            value:     0x27  0x20  0x23  0x03  0x1F  0x04  0x1D  0x1E  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.

    $0844 - $0847   PIA0    ; Muxed inputs and lamps. Initialized at $5000.
    $0848 - $084B   PIA1    ; Sound writes and muxed inputs selector. Initialized at $5000.

    $1000 - $13FF   Video RAM   ; Initialized in subroutine starting at $5042.
    $1800 - $1BFF   Color RAM   ; Initialized in subroutine starting at $5042.

    $4000 - $7FFF   ROM

    $8000 - $FFFF           ; Mirrored from $0000 - $7FFF due to lack of A15 line connection.



    ---------------------------------------
    ***  Memory Map (pottnpkr hardware) ***
    ---------------------------------------

    $0000 - $00FF   RAM     ; Zero Page (pointers and registers)
    $0100 - $01FF   RAM     ; 6502 Stack Pointer.
    $0200 - $02FF   RAM     ; R/W. (settings)
    $0300 - $03FF   RAM     ; R/W (mainly to $0383). $0340 - $035f (settings).

    $0800 - $0801   MC6845  ; MC6845 use $0800 for register addressing and $0801 for register values.

    $0844 - $0847   PIA0    ; Muxed inputs and lamps.
    $0848 - $084B   PIA1    ; Sound writes and muxed inputs selector.

    $1000 - $13FF   Video RAM
    $1800 - $1BFF   Color RAM

    $2000 - $3FFF   ROM space

    $4000 - $7FFF           ; Mirrored from $0000 - $3FFF due to lack of A14 & A15 lines connection.
    $8000 - $BFFF           ; Mirrored from $0000 - $3FFF due to lack of A14 & A15 lines connection.
    $C000 - $FFFF           ; Mirrored from $0000 - $3FFF due to lack of A14 & A15 lines connection.



*******************************************************************************


    Buttons/Inputs   goldnpkr goldnpkb  pmpoker  bsuerte goodluck pottnpkr potnpkra potnpkrc potnpkrb
    -------------------------------------------------------------------------------------------------

    HOLD (5 buttons)  mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped
    CANCEL            mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped
    BIG               mapped   mapped   mapped   mapped    ----     ----     ----     ----     ----
    SMALL             mapped   mapped   mapped   mapped    ----     ----     ----     ----     ----
    DOUBLE UP         mapped   mapped   mapped   mapped    ----     ----     ----     ----     ----
    TAKE SCORE        mapped   mapped   mapped   mapped    ----     ----     ----     ----     ----
    DEAL/DRAW         mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped
    BET               mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped

    Coin 1 (coins)    mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped
    Coin 2 (notes)    mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   fixed 1c-1c
    Coin 3 (coupons)  mapped   mapped   mapped   mapped    ----     ----     ----     ----     ----
    Payout            mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped
    Manual Collect    mapped   mapped   mapped    ----    mapped   mapped   mapped   mapped   mapped

    LEARN/SETTINGS    mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped
    METERS            mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped   mapped


    Inputs are different for some games. Normally each button has only one function.
    In pmpoker some buttons have different functions.


*******************************************************************************


    DRIVER UPDATES:


    [2006-09-02]

    - Initial release.


    [2006-09-06]

    - Understood the GFX banks:
        - 1 bank (1bpp) for text layer and minor graphics.
        - 1 bank (3bpp) for the undumped cards deck graphics.

    - Partially added inputs through 6821 PIAs.
        ("Bitte techniker rufen" error messages. Press 'W' to reset the machine)

    - Confirmed the CPU as 6502. (was in doubt due to use of illegal opcodes)


    [2006-09-15]

    - Confirmed the GFX banks (a complete dump appeared!).
    - Improved technical notes and added a PCB layout based on PCB picture.
    - Found and fixed the 3rd bitplane of BigBoy gfx.
    - Renamed Big-Boy to Golden Poker Double Up. (Big-Boy and Mini-Boy are names of cabinet models).
    - Added 'Joker Poker' (Golden Poker version without the 'double-up' feature).
    - Added 'Jack Potten's Poker' (same as Joker Poker, but with 'Aces or better' instead of jacks).
    - Simulated colors for all sets till color PROMs appear.
    - Fixed bit corruption in goldnpkr rom u40_4a.bin.
    - Completed inputs in all sets (except DIP switches).
    - Removed flags GAME_WRONG_COLORS and GAME_IMPERFECT_GRAPHICS in all sets.
    - Removed flag GAME_NOT_WORKING. All sets are now playable. :)


    [2006-10-09]

    - Added service/settings mode to pmpoker.
    - Added PORT_IMPULSE to manage correct timings for most inputs in all games.
      (jokerpkr still trigger more than 1 credit for coin pulse).


    [2007-02-01]

    - Crystal documented via #define.
    - CPU clock derived from #defined crystal value.
    - Replaced simulated colors with proper color prom decode.
    - Renamed "Golden Poker Double Up" to "Golden Poker Double Up (Big Boy)".
    - Added set Golden Poker Double Up (Mini Boy).
    - Cleaned up the driver a bit.
    - Updated technical notes.


    [2007-05-05]

    - Removed all inputs hacks.
    - Connected both PIAs properly.
    - Demuxed all inputs for each game.
    - Documented all outputs.
    - Added lamps support.
    - Created different layout files to cover each game.
    - Add NVRAM support to all games.
    - Corrected the color PROM status for each set.
    - Figured out most of the DIP switches.
    - Added diplocations to goldnpkb.
    - Replaced the remaining IPT_SERVICE with IPT_BUTTON for regular buttons.
    - Updated technical notes.
    - Cleaned up the driver. Now is better organized and documented.


    [2007-07-07]

    - Added set goldnpkc (Golden Poker without the double up feature).
    - Updated technical notes.


    [2008-10-12] *** REWRITE ***

    - Added discrete sound support to Golden Poker hardware games based on schematics.
    - Added discrete sound support to Potten's Poker hardware games based on PCB analysis.
    - Added discrete circuitry diagrams for both hardware types.
    - Adjusted the CPU adressing to 15 bits for pmpoker/goldenpkr hardware.
    - Adjusted the CPU adressing to 14 bits for pottnpkr hardware.
    - Rewrote all the ROM loads based on these changes.
    - Defined MASTER Xtal & CPU clock.
    - Fixed the visible area based on M6845 registers.
    - Improved the lamps layouts to be more realistic.
    - Added Good Luck (potten's poker hybrid running in goldnpkr hardware).
    - Added Buena Suerte (spanish) x 2 sets.
    - Added set Royale.
    - Added Witch Card and spanish variants.
    - Added Super Loco 93 (spanish) x 2 sets.
    - Renamed set goldnpkc to pottnpkr (parent Jack Potten's Poker set).
    - Renamed set jokerpkr to potnpkra, since is another Jack Potten's Poker set.
    - Added other 2 clones of Jack Potten's Poker.
    - Renamed/cleaned all sets based on code/hardware analysis.
    - Added intensity bit to the color system.
    - Implemented the blue killer bit for Witch Card hardware.
    - Implemented the extended graphics addressing bit for Witch Card hardware.
    - Added proper visible area to sloco93.
    - Rewrote the graphics & color decode system based on schematics. No more patched codes.
    - Changed the char gfx bank structure and rom load according to the new routines.
    - Adjusted the amount of color codes and PROM region size accordingly.
    - Updated all notes.


    [2008-11-29] *** REWRITE (part II) ***

    - Changed the driver name to goldnpkr.c (Golden Poker is the most representative hardware).
    - Splitted the PIA interfases to cover witchcrd/pottenpkr connections.
    - Fixed the witchcrd/pottnpkr/sloco93 double up mode.
    - Replaced the pottenpkr layout with goldnpkr one in all Jack Potten's Poker sets.
    - Updated game notes for Witch Card and Super Loco 93 sets.
    - Fixed al inputs & lamps to allow double up mode to the above games.
    - Added Witch Card (Video Klein) but still not working.
    - Added several Buena Suerte! sets.
    - Added new games: Maverik, Brasil 89 & Poker'91.
    - Reworked the sets parent-clone relationship (still in progress).


    [2008-12-26]

    - Correctly setup the MC6845 device for all systems.
    - Added common MC6845 device interface.
    - Merged witchcrd and sloco93 machine drivers.
    - Added/corrected the 50/60 Hz. DIP switches to all games.
      The 50hz mode needs to be corrected. Some games as most bsuerte sets have
      the 50/60 Hz. DIP switch connection patched.


    [2009-09-05]

    - Added 2 new Witch Card sets.
    - Reworked inputs for Witch Card (german set 1).
    - Created new inputs for Witch Card (english, witch game, lamps).
    - Added and connected lamps for both sets.
    - Added minimal bet and 50/60 Hz. switches to both sets.
    - Added DIP switches info for Witch Card (german, set 2).


    TODO:

    - Missing PIA connections.
    - Identify and hook the unknown device mapped at $2108-$210b in Witch Card (Video Klein).
    - Code analysis, Inputs & lamps for Royale.
    - Final cleanup and split the driver.


*******************************************************************************/


#define MASTER_CLOCK	XTAL_10MHz
#define CPU_CLOCK		(MASTER_CLOCK/16)

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/discrete.h"

#include "pmpoker.lh"
#include "goldnpkr.lh"


/*************************
*     Video Hardware     *
*************************/

static UINT8 *videoram;
static UINT8 *colorram;
static tilemap_t *bg_tilemap;

static WRITE8_HANDLER( goldnpkr_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( goldnpkr_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    ---- --x-   tiles bank.
    ---- ---x   tiles extended address (MSB).
    xx-- ----   unused.
*/

	int attr = colorram[tile_index];
	int code = ((attr & 1) << 8) | videoram[tile_index];
	int bank = (attr & 0x02) >> 1;	/* bit 1 switch the gfx banks */
	int color = (attr & 0x3c) >> 2;	/* bits 2-3-4-5 for color */

	SET_TILE_INFO(bank, code, color, 0);
}

static VIDEO_START( goldnpkr )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 29);
}

static VIDEO_UPDATE( goldnpkr )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

static PALETTE_INIT( goldnpkr )
{
/*  prom bits
    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   intensity.
    xxxx ----   unused.
*/
	int i;

	/* 0000IBGR */
	if (color_prom == 0) return;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0, bit1, bit2, r, g, b, inten, intenmin, intenmax;

		intenmin = 0xe0;
//      intenmin = 0xc2;    /* 2.5 Volts (75.757575% of the whole range) */
		intenmax = 0xff;	/* 3.3 Volts (the whole range) */

		/* intensity component */
		inten = (color_prom[i] >> 3) & 0x01;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		r = (bit0 * intenmin) + (inten * (bit0 * (intenmax - intenmin)));

		/* green component */
		bit1 = (color_prom[i] >> 1) & 0x01;
		g = (bit1 * intenmin) + (inten * (bit1 * (intenmax - intenmin)));

		/* blue component */
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = (bit2 * intenmin) + (inten * (bit2 * (intenmax - intenmin)));


		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static PALETTE_INIT( witchcrd )
{
/*
    This hardware has a feature called BLUE KILLER.
    Using the original intensity line, the PCB has a bridge
    that allow (as default) turn the background black.

    Except goodluck, all other games running in this hardware
    were designed to wear black background.

    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   blue killer.
    xxxx ----   unused.
*/
	int i;

	/* 0000KBGR */

	if (color_prom == 0) return;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b, bk;

		/* blue killer (from schematics) */
        bit3 = (color_prom[i] >> 3) & 0x01;
        bk = bit3;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		r = (bit0 * 0xff);

		/* green component */
		bit1 = (color_prom[i] >> 1) & 0x01;
		g = (bit1 * 0xff);

		/* blue component */
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = bk * (bit2 * 0xff);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/***********************
*     R/W Handlers     *
***********************/

static int mux_data = 0;
static UINT8 pia0_PA_data;

/* Inputs (buttons) are multiplexed.
   There are 4 sets of 5 bits each and are connected to PIA0, portA.
   The selector bits are located in PIA1, portB (bits 4-7).
*/
static READ8_DEVICE_HANDLER( goldnpkr_mux_port_r )
{
	switch( mux_data & 0xf0 )		/* bits 4-7 */
	{
		case 0x10: return input_port_read(device->machine, "IN0-0");
		case 0x20: return input_port_read(device->machine, "IN0-1");
		case 0x40: return input_port_read(device->machine, "IN0-2");
		case 0x80: return input_port_read(device->machine, "IN0-3");
	}
	return 0xff;
}

static READ8_DEVICE_HANDLER( pottnpkr_mux_port_r )
{
	UINT8 pa_0_4 = 0xff, pa_7;	/* Temporary place holder for bits 0 to 4 & 7 */

	switch( mux_data & 0xf0 )		/* bits 4-7 */
	{
		case 0x10: return input_port_read(device->machine, "IN0-0");
		case 0x20: return input_port_read(device->machine, "IN0-1");
		case 0x40: return input_port_read(device->machine, "IN0-2");
		case 0x80: return input_port_read(device->machine, "IN0-3");
	}

	pa_7 = (pia0_PA_data >> 7) & 1;	/* To do: bit PA5 to pin CB1 */
//  popmessage ("mux_port_r: %x ",((pa_0_4 & 0x3f) | (pa_7 << 6) | (pa_7 << 7))) ; /* Equates PA6 to PA7 */

	return ( (pa_0_4 & 0x3f) | (pa_7 << 6) | (pa_7 << 7) ) ;
}

static WRITE8_DEVICE_HANDLER( mux_w )
{
	mux_data = data ^ 0xff;	/* inverted */
}

static WRITE8_DEVICE_HANDLER( mux_port_w )
{
	pia0_PA_data = data;
}


/***** Lamps wiring *****

    -------------------
    pmpoker
    -------------------
    L0 = Deal
    L1 = Hold3
    L2 = Hold1
    L3 = Hold5
    L4 = Hold2 & Hold4

    -----------------------
    goldnpkr sets & bsuerte
    -----------------------
    L0 = Bet
    L1 = Deal
    L2 = Holds (all)
    L3 = Double Up & Take
    L4 = Big & Small

    ----------------------------------
    pottnpkr sets, jokerpkr & goodluck
    ----------------------------------
    L0 = Bet
    L1 = Deal
    L2 = Holds (all)

    ------------------
    witchcrd & sloco93
    ------------------
    NONE. Just they lack of lamps...

*/

static WRITE8_DEVICE_HANDLER( lamps_a_w )
{
	output_set_lamp_value(0, 1 - ((data) & 1));			/* Lamp 0 */
	output_set_lamp_value(1, 1 - ((data >> 1) & 1));	/* Lamp 1 */
	output_set_lamp_value(2, 1 - ((data >> 2) & 1));	/* Lamp 2 */
	output_set_lamp_value(3, 1 - ((data >> 3) & 1));	/* Lamp 3 */
	output_set_lamp_value(4, 1 - ((data >> 4) & 1));	/* Lamp 4 */

//  popmessage("written : %02X", data);
	coin_counter_w(device->machine, 0, data & 0x40);	/* counter1 */
	coin_counter_w(device->machine, 1, data & 0x80);	/* counter2 */
	coin_counter_w(device->machine, 2, data & 0x20);	/* counter3 */

/*  Counters:

    bit 5 = Coin out
    bit 6 = Coin counter
    bit 7 = Note counter (only goldnpkr use it)

    ONLY for witchcrd, bsuerte and sloco93 sets:

    bit3 = Coin counter (inverted).
    bit5 = Coin out (inverted).
*/
}

static WRITE8_DEVICE_HANDLER( sound_w )
{
	/* 555 voltage controlled */
	logerror("Sound Data: %2x\n",data & 0x0f);

	/* discrete sound is connected to PIA1, portA: bits 0-3 */
	discrete_sound_w(device, NODE_01, data >> 3 & 0x01);
	discrete_sound_w(device, NODE_10, data & 0x07);
}


/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( goldnpkr_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	/* battery backed RAM */
	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x0844, 0x0847) AM_DEVREADWRITE("pia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0848, 0x084b) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(goldnpkr_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(goldnpkr_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pottnpkr_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	/* battery backed RAM */
	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x0844, 0x0847) AM_DEVREADWRITE("pia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0848, 0x084b) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(goldnpkr_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(goldnpkr_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( witchcrd_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	/* battery backed RAM */
	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x0844, 0x0847) AM_DEVREADWRITE("pia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0848, 0x084b) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(goldnpkr_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(goldnpkr_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("SW2")
//  AM_RANGE(0x2108, 0x210b) AM_NOP /* unknown 40-pin device */
	AM_RANGE(0x4000, 0x7fff) AM_ROM
ADDRESS_MAP_END

/*
   Witch Card (Video klein)

   R/W:

   2108  RW
   2109   W
   210a   W
   210b   W

*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( goldnpkr )
	/* Multiplexed - 4x5bits */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Off (Payout)") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Learn Mode") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("D-31") PORT_CODE(KEYCODE_E)	/* O.A.R? (D-31 in schematics) */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Coupon (Note In)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("Weight (Coupon In)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )			PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )		PORT_DIPLOCATION("SW1:3")	/* listed in the manual as "Play Mode" */
	PORT_DIPSETTING(    0x40, "Manual" )			/*  listed in the manual as "Out Play" */
	PORT_DIPSETTING(    0x00, "Auto" )				/*  listed in the manual as "Credit Play" */
	PORT_DIPNAME( 0x80, 0x00, "Royal Flush" )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pmpoker )
	/* Multiplexed - 4x5bits */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Take Score (Kasse)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Small (Tief)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Bet (Setze)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Big (Hoch)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Double Up (Dopp.)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(3) PORT_NAME("Note 1 In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("Note 2 In")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Hohes Paar (Jacks or Better)" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pottnpkr )
	/* Multiplexed - 4x5bits */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(3) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Coupon (Note In)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
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
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )			PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	/* listed in the manual as "Play Mode" */
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )		PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )			/*  listed in the manual as "Out Play" */
	PORT_DIPSETTING(    0x00, "Auto" )				/*  listed in the manual as "Credit Play" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( potnpkra )
	/* Multiplexed - 4x5bits */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Coin 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(3) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note in")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Royal Flush Value" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "250 by bet" )
	PORT_DIPSETTING(    0x00, "500 by bet" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )		PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( potnpkrc )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( potnpkra )

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x10, 0x00, "Ace or Better" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( goodluck )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( goldnpkr )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(3) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("SW2")
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

static INPUT_PORTS_START( witchcrd )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( goldnpkr )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_IMPULSE(3) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("D-31") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SW1")
/*  Printed on epoxy module:

     40%  50%  60%  70%
SW3  OFF  OFF  ON   ON
SW4  OFF  ON   OFF  ON

    switches 1+2+5+6 = OFF
    switches 7+8 = ON
*/
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:1")	/* OFF by default */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:2")	/* OFF by default */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x04, "Percentage" )		PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "40%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x08, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:5")	/* OFF by default */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:6")	/* OFF by default */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:7")	/* ON by default */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:8")	/* ON by default */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x03, 0x01, "Max Bet" )			PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "10" )	/* OFF-OFF */
	PORT_DIPSETTING(    0x02, "20" )	/* ON-OFF */
	PORT_DIPSETTING(    0x01, "50" )	/* OFF-ON */
	PORT_DIPSETTING(    0x00, "100" )	/* ON-ON */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW2:3")	/* no connected (OFF) */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW2:4")	/* no connected (OFF) */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Minimal Hand" )		PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "2 Paar" )
	PORT_DIPSETTING(    0x00, "1 Paar" )
	PORT_DIPNAME( 0x20, 0x20, "Frequency" )			PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "50 Hz." )
	PORT_DIPSETTING(    0x00, "60 Hz." )
	PORT_DIPNAME( 0x40, 0x40, "Uncommented" )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "1 Credit" )
	PORT_DIPSETTING(    0x00, "10 Credits" )
	PORT_DIPNAME( 0x80, 0x00, "Royal Flush" )		PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( witchcda )
	/* Multiplexed - 4x5bits */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Apuesta (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Contabilidad (Bookkeeping)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Doblar (Double Up)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Reparte (Deal)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancela (Cancel)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pagar (Payout)") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Cobrar (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Alta (Big)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Baja (Small)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Configuracion (Settings)") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("D-31") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Billetes (Note In)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Fichas (Coin In)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Won Credits Counter" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:4")
	/* Note In is always 1 Note - 10 Credits */
	PORT_DIPSETTING(    0x00, "1 Coin - 1 Credit / 1 Note - 10 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin - 5 Credits / 1 Note - 10 Credits" )

	PORT_START("SW2")
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

static INPUT_PORTS_START( witchcdc )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Won Credits Counter" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:4")
	/* Note In is always 1 Note - 10 Credits */
	PORT_DIPSETTING(    0x00, "1 Coin - 1 Credit / 1 Note - 10 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin - 5 Credits / 1 Note - 10 Credits" )

	PORT_MODIFY("SW2")
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

static INPUT_PORTS_START( witchcdd )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_IMPULSE(3) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN0-1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN0-1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN0-1-8")

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN0-2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN0-2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("IN0-2-8")

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("D-31") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN0-3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN0-3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN0-3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("IN0-3-8") //IPT_UNKNOWN )

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	/* even when the following one is forced to OFF,
    turned ON behaves like "Jacks and Better" */
	PORT_DIPNAME( 0x10, 0x10, "SW 1 (always to OFF)")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("SW2")
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

static INPUT_PORTS_START( witchcde )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_IMPULSE(3) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN0-1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN0-1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN0-1-8")

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN0-2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN0-2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("IN0-2-8")

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("D-31") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN0-3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN0-3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN0-3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("IN0-3-8") //IPT_UNKNOWN )

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

/*  DIP Switches (as shown in the epoxy block)

    Schalter      1   2   3   4   5   6   7   8
    --------------------------------------------
    Bet >  10    OFF OFF
    Bet >  20    ON  OFF
    Bet >  50    OFF ON
    Bet > 100    ON  ON
    --------------------------------------------
    Jolli-Witch          OFF
    Witch-Card           ON
    --------------------------------------------
     6 Taster                ON
    12 Taster                OFF
    --------------------------------------------
    Hohes Paar                   ON
    2 Paar                       OFF
    --------------------------------------------
    1 DM - 1 PKT                     OFF OFF
    1 DM - 10 PKT                    OFF ON
    --------------------------------------------
    RF                                       ON
    RF NO                                    OFF
    --------------------------------------------
*/
	PORT_MODIFY("SW2")
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

static INPUT_PORTS_START( witchcdf )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_IMPULSE(3) PORT_NAME("Manual Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN0-1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN0-1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN0-1-8")

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN0-2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN0-2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("IN0-2-8")

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("D-31") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN0-3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN0-3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN0-3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("IN0-3-8") //IPT_UNKNOWN )

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Minimal Winning Hand" )
	PORT_DIPSETTING(    0x10, "Double Pair" )
	PORT_DIPSETTING(    0x00, "Jacks or Better" )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("SW2")
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

static INPUT_PORTS_START( sloco93 )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( witchcda )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Negro (Black)") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Rojo (Red)") PORT_CODE(KEYCODE_S)

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "32 Simple" )				PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Won Credits Counter" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, "1 Coin - 10 Credit / 1 Note - 120 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin - 100 Credits / 1 Note - 100 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START( royale )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-8") PORT_CODE(KEYCODE_L)

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

static INPUT_PORTS_START( bsuerte )
	/* Multiplexed - 4x5bits */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Apostar (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Contabilidad (Meters)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Doblar (Double Up)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Dar/Virar (Deal/Draw)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancelar (Cancel)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pagar (Payout)") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Cobrar (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Mayor (Big)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Menor (Small)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Configuracion (Settings)") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Billetes (Note In)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fichas (Coin In)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Cupones (Coupon In)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Par Simple" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )			PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x00, "Modo de Pago" )		PORT_DIPLOCATION("SW1:3")	/* left as 'auto' */
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
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

static INPUT_PORTS_START( bsuertew )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( bsuerte )

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_DIPNAME( 0x20, 0x00, "Creditos Ganados" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
INPUT_PORTS_END

static INPUT_PORTS_START( poker91 )
	/* Multiplexed - 4x5bits */
	PORT_INCLUDE( bsuerte )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Negro (Black)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Rojo (Red)")

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Switch Card 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Switch Card 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Switch Card 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Switch Card 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Switch Card 5")

	PORT_MODIFY("SW1")
	/* only bits 4-7 are connected here and were routed to SW1 1-4 */
	PORT_DIPNAME( 0x20, 0x20, "Contador de Acumulados" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Muestra" )
	PORT_DIPSETTING(    0x00, "Oculta" )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( goldnpkr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 16 )
GFXDECODE_END


/***********************
*    PIA Interfaces    *
***********************/

/***** Golden Poker Double Up *****/

static const pia6821_interface goldnpkr_pia0_intf =
{
	DEVCB_HANDLER(goldnpkr_mux_port_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(lamps_a_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface goldnpkr_pia1_intf =
{
	DEVCB_INPUT_PORT("SW1"),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_HANDLER("discrete", sound_w),		/* port A out */
	DEVCB_HANDLER(mux_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

/***** Jack Potten's Poker & Witch Card *****/

static const pia6821_interface pottnpkr_pia0_intf =
{
	DEVCB_HANDLER(pottnpkr_mux_port_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(mux_port_w),		/* port A out */
	DEVCB_HANDLER(lamps_a_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


/************************
*    CRTC Interface    *
************************/

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


/**************************************
*       Discrete Sound Routines       *
***************************************

    Golden Poker Double-Up discrete sound circuitry.
    ------------------------------------------------

           +12V                            .--------+---------------.  +5V
            |                              |        |               |   |
         .--+                              |        Z             +-------+
         |  |                             8|        Z 1K          | PC617 |
         Z  Z                         +----+----+   Z             +-------+
    330K Z  Z 47K                     |   VCC   |3  |   20K   1uF  |     |
         Z  Z             1.7uF       |        Q|---|--ZZZZZ--||---+     Z   1uF         6|\
         |  |            .-||-- GND   |         |7  |              |  1K Z<--||--+--------| \
         |  |   30K      |           4|      DIS|---+              Z     Z       |   LM380|  \  220uF
  PA0 ---|--|--ZZZZZ--.  |      .-----|R        |   |         2.2K Z     |       Z        |  8>--||----> Audio Out.
         |  |         |  |      |     |   555   |   Z              Z    -+-  10K Z     7+-|  /
         |  |   15K   |  |      |    5|         |   Z 10K          |    GND      Z     3+-| /-.1   .---> Audio Out.
  PA1 ---+--|--ZZZZZ--+--+------|-----|CV       |   Z              |             |     2+-|/  |    |
            |         |         |    2|         |6  |              |             |      |     |    |
            |  7.5K   |         |  .--|TR    THR|---+---||---------+-------------+      +-||--'    |
  PA2 ------+--ZZZZZ--'  |\     |  |  |   GND   |   |                            |      |          |
                         | \    |  |  +----+----+   |  .1uF                      |      | 10uF     |
                        9|  \   |  |      1|        |                           -+-     |          |
  PA3 -------------------|  8>--'  |      -+-       |                           GND     +----------'
                    4069 |  /      |      GND       |                                   |
                         | /       |                |                                  -+-
                         |/        '----------------'                                  GND
*/

static const discrete_555_desc goldnpkr_555_vco_desc =
	{DISC_555_OUT_DC | DISC_555_OUT_ENERGY, 5, DEFAULT_555_VALUES};

static const discrete_dac_r1_ladder dac_goldnpkr_ladder =
{
	3,									/* size of ladder */
	{RES_K(30), RES_K(15), RES_K(7.5)},	/* elements */

/*  external vBias doesn't seems to be accurate.
    using the 555 internal values sound better.
*/
	5,									/* voltage Bias resistor is tied to */
	RES_K(5),							/* additional resistor tied to vBias */
	RES_K(10),							/* resistor tied to ground */

	CAP_U(1.7)							/* filtering cap tied to ground */

//  12,                                 /* voltage Bias resistor is tied to */
//  RES_K(330),                         /* additional resistor tied to vBias */
//  0,                                  /* resistor tied to ground */
};

static DISCRETE_SOUND_START( goldnpkr )
/*
    - bits -
    76543210
    --------
    .....xxx --> sound data.
    ....x... --> enable/disable.

*/
	DISCRETE_INPUT_NOT   (NODE_01)		/* bit 3 - enable/disable */
	DISCRETE_INPUT_DATA  (NODE_10)		/* bits 0-2  - sound data */

	DISCRETE_DAC_R1(NODE_20, NODE_10, 5, &dac_goldnpkr_ladder)

	DISCRETE_555_ASTABLE_CV(NODE_30, NODE_01, RES_K(1), RES_K(10), CAP_U(.1), NODE_20, &goldnpkr_555_vco_desc)
	DISCRETE_OUTPUT(NODE_30, 3000)

DISCRETE_SOUND_END

/*
    Jack Potten's Poker discrete sound circuitry.
    ---------------------------------------------

                                           +5V
           +12V                             |
            |                               +--------.
         .--+                               |        Z
         |  |                              8|        Z 1K                                 +12V
         Z  Z                          .----+----.   Z                                     |
     47K Z  Z 220K                     |   VCC   |3  |    10K    1uF                     |\|
         Z  Z                          |        Q|---|---ZZZZZ---||--.             1K   7| \
         |  |                          |         |7  |               |         .--ZZZZZ--|+ \  470uF
         |  |   33K                   4|      DIS|---+               Z   10uF  |        6|  8>--||----> Audio Out.
  PA0 ---|--|--ZZZZZ--.         .------|R        |   |               Z<---||---+---------|- /
         |  |         |         |      |   555   |   Z               Z              LM380| /   .------> Audio Out.
         |  |   18K   |         |     5|         |   Z 1K            |                   |/|   |
  PA1 ---+--|--ZZZZZ--+---------|------|CV       |   Z               |                2,3,4|   |
            |         |         |     2|         |6  |  +            |                5,10 +---'
            |   10K   |         |  .---|TR    THR|---+---|(----------+                11,12|
  PA2 ------+--ZZZZZ--'  |\     |  |   |   GND   |   |               |                     |
                         | \    |  |   '----+----'   |   1uF        -+-                   -+-
                        9|  \ 8 |  |       1|        |              GND                   GND
  PA3 -------------------|   >--'  |       -+-       |
                  74LS04 |  /      |       GND       |
                         | /       |                 |
                         |/        '-----------------'
*/

static const discrete_dac_r1_ladder dac_pottnpkr_ladder =
{
	3,									/* size of ladder */
	{RES_K(33), RES_K(18), RES_K(10)},	/* elements */

/*  external vBias doesn't seems to be accurate.
    using the 555 internal values sound better.
*/
	5,									/* voltage Bias resistor is tied to */
	RES_K(5),							/* additional resistor tied to vBias */
	RES_K(10),							/* resistor tied to ground */

	0									/* no filtering cap tied to ground */
};

static DISCRETE_SOUND_START( pottnpkr )
/*
    - bits -
    76543210
    --------
    .....xxx --> sound data.
    ....x... --> enable/disable.

*/
	DISCRETE_INPUT_NOT   (NODE_01)		/* bit 3 - enable/disable */
	DISCRETE_INPUT_DATA  (NODE_10)		/* bits 0-2  - sound data */

	DISCRETE_DAC_R1(NODE_20, NODE_10, 5, &dac_pottnpkr_ladder)

	DISCRETE_555_ASTABLE_CV(NODE_30, NODE_01, RES_K(1), RES_K(1), CAP_U(1), NODE_20, &goldnpkr_555_vco_desc)
	DISCRETE_OUTPUT(NODE_30, 3000)

DISCRETE_SOUND_END


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_DRIVER_START( goldnpkr_base )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(goldnpkr_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_PIA6821_ADD("pia0", goldnpkr_pia0_intf)
	MDRV_PIA6821_ADD("pia1", goldnpkr_pia1_intf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((39+1)*8, (31+1)*8)                  /* From MC6845 init, registers 00 & 04 (programmed with value-1). */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 29*8-1)    /* From MC6845 init, registers 01 & 06. */

	MDRV_MC6845_ADD("crtc", MC6845, CPU_CLOCK, mc6845_intf)	/* 68B45 or 6845s @ CPU clock */

	MDRV_GFXDECODE(goldnpkr)
	MDRV_PALETTE_INIT(goldnpkr)
	MDRV_PALETTE_LENGTH(256)
	MDRV_VIDEO_START(goldnpkr)
	MDRV_VIDEO_UPDATE(goldnpkr)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( goldnpkr )

	MDRV_IMPORT_FROM(goldnpkr_base)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(goldnpkr)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pottnpkr )

	MDRV_IMPORT_FROM(goldnpkr_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(pottnpkr_map)

	MDRV_PIA6821_MODIFY("pia0", pottnpkr_pia0_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(pottnpkr)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( witchcrd )

	MDRV_IMPORT_FROM(goldnpkr_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(witchcrd_map)

	MDRV_PIA6821_MODIFY("pia0", pottnpkr_pia0_intf)

	/* video hardware */
	MDRV_PALETTE_INIT(witchcrd)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(goldnpkr)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

/*  the original goldnpkr u40_4a.bin rom is bit corrupted.
    U43_2A.bin        BADADDR      --xxxxxxxxxxx
    U38_5A.bin        1ST AND 2ND HALF IDENTICAL
    UPS39_12A.bin     0xxxxxxxxxxxxxx = 0xFF
*/
ROM_START( goldnpkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ups39_12a.bin",	0x0000, 0x8000, CRC(216b45fb) SHA1(fbfcd98cc39b2e791cceb845b166ff697f584add) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x4000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38_5a.bin",	0x4000, 0x2000, CRC(32705e1d) SHA1(84f9305af38179985e0224ae2ea54c01dfef6e12) )    /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "u43_2a.bin",	0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "u40_4a.bin",	0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    /* cards deck gfx, bitplane2 */
	ROM_COPY( "gfx1",	0x4800, 0x4000, 0x0800 )    /* cards deck gfx, bitplane3. found in the 2nd quarter of the text layer rom */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )

/*  pmpoker                 goldnpkr
    1-4.bin                 u38_5a (1st quarter)    96.582031%  \ 1st and 2nd halves are identical.
    1-3.bin                 u38_5a (2nd quarter)    IDENTICAL   /
    1-1.bin                 u43_2a (1st quarter)    IDENTICAL   ; 4 quarters are identical.
*/
ROM_END

ROM_START( goldnpkb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ups31h.12a",	0x0000, 0x8000, CRC(bee5b07a) SHA1(5da60292ecbbedd963c273eac2a1fb88ad66ada8) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x4000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38_5a.bin",	0x4000, 0x2000, CRC(32705e1d) SHA1(84f9305af38179985e0224ae2ea54c01dfef6e12) )    /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "u43_2a.bin",	0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "u40_4a.bin",	0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    /* cards deck gfx, bitplane2 */
	ROM_COPY( "gfx1",	0x4800, 0x4000, 0x0800 )    /* cards deck gfx, bitplane3. found in the 2nd quarter of the text layer rom */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )

/*  pmpoker                 goldnpkb
    1-4.bin                 u38.5a (1st quarter)    96.582031%  \ 1st and 2nd halves are identical.
    1-3.bin                 u38.5a (2nd quarter)    IDENTICAL   /
    1-1.bin                 u43.2a (1st quarter)    IDENTICAL   ; 4 quarters are identical.
*/
ROM_END



ROM_START( pmpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2-5.bin",	0x5000, 0x1000, CRC(3446a643) SHA1(e67854e3322e238c17fed4e05282922028b5b5ea) )
	ROM_LOAD( "2-6.bin",	0x6000, 0x1000, CRC(50d2d026) SHA1(7f58ab176de0f0f7666d87271af69a845faec090) )
	ROM_LOAD( "2-7.bin",	0x7000, 0x1000, CRC(a9ab972e) SHA1(477441b7ff3acae3a5d5a3e4c2a428e0b3121534) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x1000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "1-4.bin",	0x1000, 0x0800, CRC(62b9f90d) SHA1(39c61a01225027572fdb75543bb6a78ed74bb2fb) )    /* text layer */

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1-1.bin",	0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "1-2.bin",	0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "1-3.bin",	0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24sa10n.7d",		0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( pottnpkr )	/* Golden Poker style game. Code is intended to start at $6000 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic13_3.bin",	0x2000, 0x1000, CRC(23c975cd) SHA1(1d32a9ba3aa996287a823558b9d610ab879a29e8) )
	ROM_LOAD( "ic14_4.bin",	0x3000, 0x1000, CRC(86a03aab) SHA1(0c4e8699b9fc9943de1fa0a364e043b3878636dc) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "ic7_0.bin",	0x2000, 0x1000, CRC(1090e7f0) SHA1(26a7fc8853debb9a759811d7fee39410614c3895) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "ic2_7.bin",	0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "ic3_8.bin",	0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "ic5_9.bin",	0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )

/*  ic2_7.bin    1ST AND 2ND HALF IDENTICAL
    ic3_8.bin    1ST AND 2ND HALF IDENTICAL
    ic5_9.bin    1ST AND 2ND HALF IDENTICAL
    ic7_0.bin    1ST AND 2ND HALF IDENTICAL

    RB confirmed the dump. There are other games with double sized roms and identical halves.
*/
ROM_END

ROM_START( potnpkra )    /* a Coinmaster game?... seems to be a hack */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vp-5.bin",	0x2000, 0x1000, CRC(1443d0ff) SHA1(36625d24d9a871cc8c03bdeda983982ba301b385) )
	ROM_LOAD( "vp-6.bin",	0x3000, 0x1000, CRC(94f82fc1) SHA1(ce95fc429f5389eea45fec877bac992fa7ba2b3c) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x1000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "vp-4.bin",	0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    /* text layer */

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "vp-1.bin",	0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "vp-2.bin",	0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "vp-3.bin",	0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
pottpok4.bin                                                 0xxxxxxxxxx = 0x00
                        517.4a                               0xxxxxxxxxx = 0x00
pottpok1.bin            517.8a                  IDENTICAL
pottpok2.bin            517.7a                  IDENTICAL
pottpok3.bin            517.6a                  IDENTICAL
pottpok4.bin            517.4a                  IDENTICAL
pottpok5.bin            517.16a                 69.335938%
pottpok6.bin            517.17a                 2.685547%
*/

ROM_START( potnpkrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "517.16a",	0x2000, 0x1000, CRC(8892fbd4) SHA1(22a27c0c3709ca4808a9afb8848233bc4124559f) )
	ROM_LOAD( "517.17a",	0x3000, 0x1000, CRC(75a72877) SHA1(9df8fd2c98526d20aa0fa056a7b71b5c5fb5206b) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x1000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "517.8a",		0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    /* text layer */

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "517.4a",		0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "517.6a",		0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "517.7a",		0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "517_mb7052.9c",	0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/* the alternative Jack Potten set is identical, but with different sized roms.

                pot1.bin                1ST AND 2ND HALF IDENTICAL
                pot2.bin                1ST AND 2ND HALF IDENTICAL
pottpok1.bin    pot34.bin (1st half)    IDENTICAL
pottpok2.bin    pot34.bin (2nd half)    IDENTICAL
pottpok3.bin    pot2.bin (1st half)     IDENTICAL
pottpok4.bin    pot1.bin (1st half)     IDENTICAL
pottpok5.bin    pot5.bin                IDENTICAL
pottpok6.bin    pot6.bin                IDENTICAL
*/

ROM_START( potnpkrc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pottpok5.bin",	0x2000, 0x1000, CRC(d74e50f4) SHA1(c3a8a6322a3f1622898c6759e695b4e702b79b28) )
	ROM_LOAD( "pottpok6.bin",	0x3000, 0x1000, CRC(53237873) SHA1(b640cb3db2513784c8d2d8983a17352276c11e07) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x1000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "pottpok1.bin",	0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    /* text layer */

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pottpok4.bin",	0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "pottpok3.bin",	0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "pottpok2.bin",	0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/***************** NEW SETS **************************/


ROM_START( potnpkrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pot5.bin",	0x2000, 0x1000, CRC(d74e50f4) SHA1(c3a8a6322a3f1622898c6759e695b4e702b79b28) )
	ROM_LOAD( "pot6.bin",	0x3000, 0x1000, CRC(53237873) SHA1(b640cb3db2513784c8d2d8983a17352276c11e07) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "pot34.bin",	0x2000, 0x1000, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "pot1.bin",	0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "pot2.bin",	0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    /* cards deck gfx, bitplane2 */
	ROM_COPY( "gfx1",		0x2800, 0x2000, 0x0800 )    /* cards deck gfx, bitplane3. found in the 2nd quarter of the text layer rom */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( potnpkre )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g_luck_a.bin",	0x2000, 0x1000, CRC(21d3b5e9) SHA1(32f06eb26c5232738ad7e86f1a81eb9717f9c7e0) )
	ROM_LOAD( "g_luck_b.bin",	0x3000, 0x1000, CRC(7e848e5e) SHA1(45461cfcce06f6240562761d26ba7fdb7ef4986b) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "ic7_0.bin",	0x2000, 0x1000, CRC(1090e7f0) SHA1(26a7fc8853debb9a759811d7fee39410614c3895) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "ic2_7.bin",	0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "ic3_8.bin",	0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "ic5_9.bin",	0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",		0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( goodluck )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goodluck_glh6b.bin",	0x0000, 0x8000, CRC(2cfa4a2c) SHA1(720e2900f3a0ef2632aa201a63b5eba0570e6aa3) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(			0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "4.bin",	0x2000, 0x1000, CRC(41924d13) SHA1(8ab69b6efdc20858960fa5df669470ba90b5f8d7) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( royale )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "royalex.bin",	0x4000, 0x4000, CRC(ef370617) SHA1(0fc5679e9787aeea3bc592b36efcaa20e859f912) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "royalechr.bin",	0x2000, 0x1000, CRC(b1f2cbb8) SHA1(8f4930038f2e21ca90b213c35b45ed14d8fad6fb) )    /* text layer */

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "royale3.bin",	0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "royale2.bin",	0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "royale1.bin",	0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( royalea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "royal.256",	0x0000, 0x8000, CRC(9d7fdb79) SHA1(05cae00bca0f6ae696c69f531cb0fa2104ff696a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "royalechr.bin",	0x2000, 0x1000, CRC(b1f2cbb8) SHA1(8f4930038f2e21ca90b213c35b45ed14d8fad6fb) )    /* text layer */

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "royale3.bin",	0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "royale2.bin",	0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "royale1.bin",	0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

/*  Witch Card (Video Klein)
    Video Klein original with epoxy block module
*/
	ROM_START( witchcrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epoxy_27128.bin",	0x4000, 0x4000, CRC(48186272) SHA1(d211bfa89404a292e6d0f0169ed11e1e74a361d9) )	/* epoxy block program ROM */

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "wc4.7a",	0x2000, 0x0800, CRC(6a392b10) SHA1(9f36ae2e5a9a8741c6687e9c875d7b45999d9d6d) )    /* text layer */
	ROM_LOAD( "wc4.7a",	0x2800, 0x0800, CRC(6a392b10) SHA1(9f36ae2e5a9a8741c6687e9c875d7b45999d9d6d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "wc1.2a",	0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "wc2.4a",	0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "wc3.5a",	0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s129.7d",			0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* original PCB PROM */
	ROM_LOAD( "epoxy_82s129.bin",	0x0100, 0x0100, CRC(f0c012b1) SHA1(5502977404172e8c5b9fbf305581a406668ad1d9) ) /* original epoxy block PROM */
ROM_END

/*  Witch Card (spanish, set 1)
*/
ROM_START( witchcda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w_card.256",	0x0000, 0x8000, CRC(63a471f8) SHA1(96a2140e2da0050e7865a6662f707cf024130832) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "bs_4_wcspa.032",	0x2000, 0x1000, CRC(4e520c7a) SHA1(1de3ac4a150160c15f453b0d3f9d3cd3178bfedd) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

/*  Witch Card (spanish, set 2)
*/
ROM_START( witchcdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w_card.128",	0x4000, 0x4000, CRC(11ecac96) SHA1(717709b31f3dfa09be321c14fbf0e95d492ad2f2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "bs_4_wcspa.032",	0x2000, 0x1000, CRC(4e520c7a) SHA1(1de3ac4a150160c15f453b0d3f9d3cd3178bfedd) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

/*  Witch Card (english, no witch game)
*/
ROM_START( witchcdc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wc_sbruj.256",	0x0000, 0x8000, CRC(5689ae41) SHA1(c7a624ec881204137489b147ce66cc9a9900650a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "bs_4_wc.032",	0x2000, 0x1000, CRC(41924d13) SHA1(8ab69b6efdc20858960fa5df669470ba90b5f8d7) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

/*  Witch Card (german, set 1)

    TV GAME ELEKTRONIK 1994
           PROVA
    CASINOVERSION WC3050
*/
	ROM_START( witchcdd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "12a.bin",	0x0000, 0x8000, CRC(a5c1186a) SHA1(b6c662bf489fbcccc3063ce55c957e630ba96ccb) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x4000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "ce-3-tvg.bin",	0x4000, 0x2000, CRC(54b51497) SHA1(8c3a74377fde8c7c5a6b277a9c1e717e6bdd98f8) )    /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "ce-1-tvg.bin",	0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "ce-2-tvg.bin",	0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    /* cards deck gfx, bitplane2 */
	ROM_COPY( "gfx1",	0x4800, 0x4000, 0x0800 )    /* cards deck gfx, bitplane3. found in the 2nd quarter of the text layer rom */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",	0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*  Witch Card (german, set 2)
    Epoxy CPU box.

    Code jumps into a zero-filled zone.
    Addressing seems to be scrambled in some way
    maybe through a PLD.
*/
ROM_START( witchcde )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* inside epoxy block with the CPU */
	ROM_LOAD( "27c256.bin",	0x0000, 0x8000, CRC(14f05e3b) SHA1(83578f6a82b0974dd0325903926b2fd0d8e5c236) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x4000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "iii.5a",	0x4000, 0x2000, CRC(5ea338da) SHA1(5e55e17689541ffb9c23e45f689dda98a79bf789) )    /* text layer */

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "i.2a",	0x0000, 0x2000, CRC(d467f6e2) SHA1(6aaf4cdfb76f5efeeee45635fea120711483648e) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "ii.4a",	0x2000, 0x2000, CRC(044dfac0) SHA1(721f8f57e05ddcbdb838d12fd3e81a45346ee6db) )    /* cards deck gfx, bitplane2 */
	ROM_COPY( "gfx1",	0x5000, 0x4000, 0x1000 )    /* cards deck gfx, bitplane3. found in the 2nd quarter of the text layer rom */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.7d",	0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*  Witch Card (english, witch game, lights)
    PCB by PM. Hybrid hardware.

    Copyright 1983/84/85
    W.BECK ELEKTRONIK
*/
ROM_START( witchcdf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "$25.bin",	0x5000, 0x1000, CRC(afd6cb4a) SHA1(4c769e1c724bada5875e028781086c32967953a1) )
	ROM_LOAD( "$26.bin",	0x6000, 0x1000, CRC(ad11960c) SHA1(2b562cfe9401e21c9dcd90307165e2c2d1acfc5b) )
	ROM_LOAD( "$27.bin",	0x7000, 0x1000, CRC(e6f9c973) SHA1(f209d13d1565160bc2c05c6c4fce73d14a9a56ab) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "$14.bin",	0x2000, 0x1000, CRC(19b68bec) SHA1(b8ef17ba5545e2f104cd2783e5f1c97c400fcbbc) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "11.bin",	0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "12.bin",	0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "13.bin",	0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wc_bprom.bin",	0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( sloco93 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "locoloco.128",	0x4000, 0x4000, CRC(f626a770) SHA1(afbd33b3f65b8a781c716a3d6e5447aa817d856c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "confloco.032",	0x2000, 0x1000, CRC(b86f219c) SHA1(3f655a96bcf597a271a4eaaa0acbf8dd70fcdae9) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( sloco93a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "locoloco.256",	0x0000, 0x8000, CRC(ab037b0b) SHA1(16f811daaed5bf7b72549db85755c5274dfee310) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(					0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "confloco.032",	0x2000, 0x1000, CRC(b86f219c) SHA1(3f655a96bcf597a271a4eaaa0acbf8dd70fcdae9) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

/*
    checksum routine at $5f3e
    protect $4000+ & $7ff9.
    (see cmp at $5f6b)
    balanced at $7ff8.
*/
ROM_START( maverik )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* maverik: Maverik (ind arg, fixed, changed logo) */
	ROM_LOAD( "maverik.bin",	0x0000, 0x8000, CRC(65a986e9) SHA1(2e825d3fb2346036357af0e12d3a75b5ef6cfd0d) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(			0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "4s.bin",	0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( brasil89 )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* brasil89.128: Brasil 89, BS clone. */
	ROM_LOAD( "brasil89.128",	0x4000, 0x4000, CRC(9030e0db) SHA1(d073ed0ddd3e5df6a3387e10e05ca34bc491eb35) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(			0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "4s.bin",	0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( poker91 )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_pok91.bin: Poker 91. Based on witchcrd */
	ROM_LOAD( "bs_pok91.bin",	0x0000, 0x8000, CRC(90c88b45) SHA1(9b5842075ece5f96a6869d7a8c874dee2b2abde2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(			0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "4s.bin",	0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END


/******************************* bsuerte sets **************************/
/*
    checksum routine at $5827
    protect $4000+ & $7ff9.
    (see cmp at $584a)
    balanced at $7ff8.
*/
ROM_START( bsuerte )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_chica.256: good BS set... (checksum) */
	ROM_LOAD( "bs_chica.256",	0x0000, 0x8000, CRC(2e92b72b) SHA1(6c90fb265f2cb7ec40ddb0553b5b7fedfa89339c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ups39_12a.bin",	0x0000, 0x8000, CRC(e6b661b7) SHA1(b265f6814a168034d24bc1c25f67ece131281bc2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerteb )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bsrapida.128: Buena Suerte! red title, from Cordoba" */
	ROM_LOAD( "bsrapida.128",	0x4000, 0x4000, CRC(a2c633fa) SHA1(7cda3f56e6bd8e6bfc36a68c16d2e63d76d4dac3) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "x10d4esp.16c",	0x0000, 0x8000, CRC(0606bab4) SHA1(624b0cef1a23a4e7ba2d2d256f30f73b1e455fa7) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerted )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* set seen nowadays, based on bsuertec */
	ROM_LOAD( "x10d4fix.bin",	0x0000, 0x8000, CRC(c5ecc419) SHA1(5538a1336b877d1780d9a0c5595b02e9b22ee17d) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertee )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* source program for other mods  */
	ROM_LOAD( "x10bb26.bin",	0x0000, 0x8000, CRC(57011385) SHA1(3cbfdb8dd261aa8ce27441326f0916640b13b67a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertef )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* add its own logo ($0000-$4000) in the cards-back */
	ROM_LOAD( "bscat.256",	0x0000, 0x8000, CRC(944accd3) SHA1(f1ed149b9dafe9cdf3745b9344f2ce1814027005) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerteg )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* based on witchcrd (winning counter, no lamps, only 9 settings parameters) */
	ROM_LOAD( "bsjc.256",	0x0000, 0x8000, CRC(3a824d96) SHA1(1eb2b4630be10131416ff84213aa858a072896ac) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerteh )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* based on witchcrd (winning counter, no lamps, only 9 settings parameters) */
	ROM_LOAD( "jc603d.256",	0x0000, 0x8000, CRC(25df69e5) SHA1(54d2798437b61bd0e1919fb62daf24ed9df42678) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertei )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* mcs: Buena Suerte! (ind arg, Cordoba) */
	ROM_LOAD( "mcs.256",	0x0000, 0x8000, CRC(5c944e9d) SHA1(e394f8a32f4ebe622c0d0c30db5cb9d6d70b2126) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertej )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bsgemini: BS hack by SUSILU, bad texts, and need proper chars */
	ROM_LOAD( "bsgemini.256",	0x0000, 0x8000, CRC(883f94d0) SHA1(30ff337ed2f454f74dfa354c14a8ab422284d279) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertek )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bsindarg: Buena Suerte! (ind arg, Cordoba, set 2)*/
	ROM_LOAD( "bsindarg.128",	0x4000, 0x4000, CRC(a9aaff1a) SHA1(13c9fbd0e9a04f42ded4dda0bb8a850de65cc671) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertel )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bslacer128: Buena Suerte! (portugues), english settings */
	ROM_LOAD( "bslacer.128",	0x4000, 0x4000, CRC(edc254f4) SHA1(20e5543e59bfd67a0afec7cbeeb7000f6bba6c69) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertem )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bslacer128: Buena Suerte! (portugues), english settings, set 2*/
	ROM_LOAD( "bslacer.256",	0x0000, 0x8000, CRC(9f8a899a) SHA1(a1f3d0635b309d4734289b7ff48eceda69dfd3d0) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerten )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_x10.128: BS normal, fast. */
	ROM_LOAD( "bs_x10.128",	0x4000, 0x4000, CRC(2549ceeb) SHA1(8c17849c7e9c138c35df584cdc0eabf536edb3d9) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerteo )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_x10.256: BS normal, fast, set 2*/
	ROM_LOAD( "bs_x10.256",	0x0000, 0x8000, CRC(ad3427a6) SHA1(d0a954c86c0a4354b5cea4140b8da7a10f66337a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertep )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_p.128: another common BS set */
	ROM_LOAD( "bs_p.128",	0x4000, 0x4000, CRC(9503cfef) SHA1(f3246621bb9dff3d357d4c99f7075509899ed05f) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerteq )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_r4.128: BS portunhol, white title */
	ROM_LOAD( "bs_r4.128",	0x4000, 0x4000, CRC(22841e2f) SHA1(d547aa6ddb82aff0d87eeb9bae67281d22dc50d5) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerter )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_100.128: BS with 1-100 bet (only allow 50) */
	ROM_LOAD( "bs_100.128",	0x4000, 0x4000, CRC(1d3104e5) SHA1(9c0f00725270aa4d28b5a539431311bdca2f864a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertes )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_50.128: BS, normal set */
	ROM_LOAD( "bs_50.128",	0x4000, 0x4000, CRC(8c2e43ca) SHA1(3e3f0848964f4ee6f47ddcf2220ebd06d771eebf) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuertet )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_c.128: BS portunhol, with typos (Halta) */
	ROM_LOAD( "bs_c.128",	0x4000, 0x4000, CRC(8b605bdf) SHA1(a933149999937f44cb62a3b34ab55ac4b5a50f72) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END

ROM_START( bsuerteu )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* bs_origi.bin: BS portunhol, with typos (Halta & Fixa) */
	ROM_LOAD( "bs_origi.bin",	0x0000, 0x8000, CRC(63a1ba65) SHA1(2354461ec7ad75f7ff2699e89d40517463157aaa) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(				0x0000, 0x2000, 0 ) /* filling the R-G bitplanes */
	ROM_LOAD( "u38.bin",	0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    /* text layer */

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",	0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    /* cards deck gfx, bitplane1 */
	ROM_LOAD( "6.bin",	0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    /* cards deck gfx, bitplane2 */
	ROM_LOAD( "5.bin",	0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    /* cards deck gfx, bitplane3 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",		0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) /* PROM dump needed */
ROM_END


/*************************
*      Driver Init       *
*************************/

/*
    Golden Poker H/W sets:

    newname    oldname    gameplay  music      settings    testmode
    ===================================================================
    pmpoker    pmpoker    fast      minimal    hack        matrix/grill
    goldnpkr   goldnpkr   fast      y.doodle   excellent   matrix/grill
    goldnpkb   goldnpkb   normal    minimal    normal      matrix/grill


    Potten's Poker H/W sets:

    newname    oldname    gameplay  music      settings    testmode
    ===================================================================
    pottnpkr   goldnpkc   fast      y.doodle   normal      only grid
    potnpkra   jokerpkr   normal    normal     normal      only skill
    potnpkrb   pottnpkb   slow      y.doodle   normal      only grid
    potnpkrc   pottnpkr   normal    minimal    hack        only skill
    potnpkrd   --------   normal    minimal    hack        only skill
    potnpkre   --------   slow      y.doodle   normal      matrix/grill


    Witch Card H/W sets:

    newname    oldname    gameplay  music      settings    testmode
    ===================================================================
    bsuerte    --------   normal    minimal    only 1-10   matrix/grill
    bsuertea   --------   normal    minimal    only 1-10   matrix/grill
    goodluck   --------   fast      y.doodle   normal      matrix/grill
    royale     --------
    witchcrd   --------   normal    y.doodle   12-param    only grid
    witchcda   --------   fast      y.doodle   12-param    only grid
    witchcdb   --------   normal    y.doodle   12-param    only grid
    sloco93    --------   fast      custom     complete    only grid
    sloco93a   --------   fast      custom     complete    only grid

*/
static DRIVER_INIT( royale )
{
    /* $60bb, NOPing the ORA #$F0 (after read the PIA1 port B */

//  UINT8 *ROM = memory_region(machine, "maincpu");

//  ROM[0x60bb] = 0xea;
//  ROM[0x60bc] = 0xea;
}

/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME      PARENT    MACHINE   INPUT     INIT      ROT      COMPANY                      FULLNAME                                  FLAGS             LAYOUT  */
GAMEL( 1981, goldnpkr, 0,        goldnpkr, goldnpkr, 0,        ROT0,   "Bonanza Enterprises, Ltd",  "Golden Poker Double Up (Big Boy)",        0,                layout_goldnpkr )
GAMEL( 1981, goldnpkb, goldnpkr, goldnpkr, goldnpkr, 0,        ROT0,   "Bonanza Enterprises, Ltd",  "Golden Poker Double Up (Mini Boy)",       0,                layout_goldnpkr )
GAMEL( 1981, pmpoker,  0,        goldnpkr, pmpoker,  0,        ROT0,   "PlayMan",                   "PlayMan Poker (German)",                  0,                layout_pmpoker  )
GAMEL( 198?, pottnpkr, 0,        pottnpkr, pottnpkr, 0,        ROT0,   "bootleg",                   "Jack Potten's Poker (set 1)",             0,                layout_goldnpkr )
GAMEL( 198?, potnpkra, pottnpkr, pottnpkr, potnpkra, 0,        ROT0,   "bootleg",                   "Jack Potten's Poker (set 2)",             0,                layout_goldnpkr )
GAMEL( 198?, potnpkrb, pottnpkr, pottnpkr, pottnpkr, 0,        ROT0,   "bootleg",                   "Jack Potten's Poker (set 3)",             0,                layout_goldnpkr )
GAMEL( 198?, potnpkrc, pottnpkr, pottnpkr, potnpkrc, 0,        ROT0,   "bootleg",                   "Jack Potten's Poker (set 4)",             0,                layout_goldnpkr )
GAMEL( 198?, potnpkrd, pottnpkr, pottnpkr, potnpkrc, 0,        ROT0,   "bootleg",                   "Jack Potten's Poker (set 5)",             0,                layout_goldnpkr )
GAMEL( 198?, potnpkre, pottnpkr, pottnpkr, pottnpkr, 0,        ROT0,   "bootleg",                   "Jack Potten's Poker (set 6)",             0,                layout_goldnpkr )
GAMEL( 1991, goodluck, 0,        witchcrd, goodluck, 0,        ROT0,   "<unknown>",                 "Good Luck",                               0,                layout_goldnpkr )
GAMEL( 198?, royale,   0,        goldnpkr, royale,   royale,   ROT0,   "<unknown>",                 "Royale (set 1)",                          GAME_NOT_WORKING, layout_goldnpkr )
GAMEL( 198?, royalea,  royale,   goldnpkr, royale,   royale,   ROT0,   "<unknown>",                 "Royale (set 2)",                          GAME_NOT_WORKING, layout_goldnpkr )
GAME(  1991, witchcrd, 0,        witchcrd, witchcrd, 0,        ROT0,   "Video Klein",               "Witch Card (Video Klein)",                GAME_NOT_WORKING )
GAME(  1991, witchcda, witchcrd, witchcrd, witchcda, 0,        ROT0,   "<unknown>",                 "Witch Card (Spanish, witch game, set 1)", 0 )
GAME(  1991, witchcdb, witchcrd, witchcrd, witchcda, 0,        ROT0,   "<unknown>",                 "Witch Card (Spanish, witch game, set 2)", 0 )
GAME(  1991, witchcdc, witchcrd, witchcrd, witchcdc, 0,        ROT0,   "<unknown>",                 "Witch Card (English, no witch game)",     0 )
GAMEL( 1994, witchcdd, witchcrd, witchcrd, witchcdd, 0,        ROT0,   "bootleg? (Proma)",          "Witch Card (German, set 1)",              0,                layout_goldnpkr )
GAME(  1994, witchcde, witchcrd, witchcrd, witchcde, 0,        ROT0,   "bootleg?",                  "Witch Card (German, set 2)",              GAME_NOT_WORKING )
GAMEL( 1985, witchcdf, witchcrd, witchcrd, witchcdf, 0,        ROT0,   "PlayMan",                   "Witch Card (English, witch game, lamps)", 0,                layout_goldnpkr )
GAME(  1993, sloco93,  0,        witchcrd, sloco93,  0,        ROT0,   "<unknown>",                 "Super Loco 93 (Spanish, set 1)",          0 )
GAME(  1993, sloco93a, sloco93,  witchcrd, sloco93,  0,        ROT0,   "<unknown>",                 "Super Loco 93 (Spanish, set 2)",          0 )
GAME(  198?, maverik,  0,        witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Maverik",                                 0 )
GAMEL( 1989, brasil89, 0,        witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Brasil 89",                               0,                layout_goldnpkr )
GAME(  1991, poker91,  0,        witchcrd, poker91,  0,        ROT0,   "<unknown>",                 "Poker 91",                                0 )
GAMEL( 1990, bsuerte,  0,        witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 1)",           0,                layout_goldnpkr )
GAMEL( 1991, bsuertea, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 2)",           0,                layout_goldnpkr )
GAMEL( 1991, bsuerteb, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 3)",           0,                layout_goldnpkr )
GAMEL( 1991, bsuertec, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 4)",           0,                layout_goldnpkr )
GAMEL( 1991, bsuerted, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 5)",           0,                layout_goldnpkr )
GAMEL( 1991, bsuertee, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 6)",           0,                layout_goldnpkr )
GAMEL( 1991, bsuertef, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 7)",           0,                layout_goldnpkr )
GAME(  1991, bsuerteg, bsuerte,  witchcrd, bsuertew, 0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 8)",           0 )
GAME(  1991, bsuerteh, bsuerte,  witchcrd, bsuertew, 0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 9)",           0 )
GAMEL( 1991, bsuertei, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 10)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuertej, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 11)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuertek, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 12)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuertel, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 13)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuertem, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 14)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuerten, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 15)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuerteo, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 16)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuertep, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 17)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuerteq, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 18)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuerter, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 19)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuertes, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 20)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuertet, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 21)",          0,                layout_goldnpkr )
GAMEL( 1991, bsuerteu, bsuerte,  witchcrd, bsuerte,  0,        ROT0,   "<unknown>",                 "Buena Suerte (Spanish, set 22)",          0,                layout_goldnpkr )
