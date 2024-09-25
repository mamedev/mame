// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/***********************************************************************************

  GOLDEN POKER DOUBLE UP (BONANZA ENTERPRISES, LTD)
  -------------------------------------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Golden Poker Double Up (Big Boy),                  1981, Bonanza Enterprises, Ltd.
  * Golden Poker Double Up (Mini Boy),                 1981, Bonanza Enterprises, Ltd.
  * Golden Poker Double Up (bootleg, set 1),           198?, Bootleg.
  * Golden Poker Double Up (bootleg, set 2),           198?, Bootleg.
  * Golden Poker Double Up (bootleg, set 3),           1983, Intercoast (bootleg).
  * Golden Poker Double Up (bootleg, set 4),           1983, Intercoast (bootleg).
  * Videotron Poker (cards selector, set 1),           198?, Unknown.
  * Videotron Poker (cards selector, set 2),           198?, Unknown.
  * Videotron Poker (normal controls),                 198?, Unknown.
  * Jack Potten's Poker (set 1),                       198?, Bootleg.
  * Jack Potten's Poker (set 2),                       198?, Bootleg in Coinmaster H/W.
  * Jack Potten's Poker (set 3),                       198?, Bootleg.
  * Jack Potten's Poker (set 4),                       198?, Bootleg.
  * Jack Potten's Poker (set 5),                       198?, Bootleg.
  * Jack Potten's Poker (set 6),                       198?, Bootleg.
  * Jack Potten's Poker (set 7, Royale GFX),           198?, Bootleg.
  * Jack Potten's Poker (set 8, Australian),           198?, Bootleg.
  * Jack Potten's Poker (set 9, 'just 4 fun'),         198?, Bootleg.
  * Jack Potten's Poker (set 10, ICP-1 PCB),           198?, Bootleg.
  * Jack Potten's Poker (set 11, German, W.W.),        198?, Bootleg.
  * Jack Potten's Poker (set 12, no Double-Up),        198?, Bootleg.
  * Jack Potten's Poker (set 13, ICP-1 PCB),           198?, Bootleg.
  * Jack Potten's Poker (NGold, set 1),                198?, Unknown.
  * Jack Potten's Poker (NGold, set 2),                198?, Unknown.
  * Jack Potten's Poker (NGold, set 3),                198?, Unknown.
  * Buena Suerte (Spanish, set 1),                     1990, Unknown.
  * Buena Suerte (Spanish, set 2),                     1991, Unknown.
  * Buena Suerte (Spanish, set 3),                     1991, Unknown.
  * Buena Suerte (Spanish, set 4),                     1991, Unknown.
  * Buena Suerte (Spanish, set 5),                     1991, Unknown.
  * Buena Suerte (Spanish, set 6),                     1991, Unknown.
  * Buena Suerte (Spanish, set 7),                     1991, Unknown.
  * Buena Suerte (Spanish, set 8),                     1991, Unknown.
  * Buena Suerte (Spanish, set 9),                     1991, Unknown.
  * Buena Suerte (Spanish, set 10),                    1991, Unknown.
  * Buena Suerte (Spanish, set 11),                    1991, Unknown.
  * Buena Suerte (Spanish, set 12),                    1991, Unknown.
  * Buena Suerte (Spanish, set 13),                    1991, Unknown.
  * Buena Suerte (Spanish, set 14),                    1991, Unknown.
  * Buena Suerte (Spanish, set 15),                    1991, Unknown.
  * Buena Suerte (Spanish, set 16),                    1991, Unknown.
  * Buena Suerte (Spanish, set 17),                    1991, Unknown.
  * Buena Suerte (Spanish, set 18),                    1991, Unknown.
  * Buena Suerte (Spanish, set 19),                    1991, Unknown.
  * Buena Suerte (Spanish, set 20),                    1991, Unknown.
  * Buena Suerte (Spanish, set 21),                    1991, Unknown.
  * Buena Suerte (Spanish, set 22),                    1991, Unknown.
  * Buena Suerte (Spanish/Portuguese, set 23),         1991, Unknown.
  * Good Luck,                                         198?, Unknown.
  * Falcons Wild - World Wide Poker,                   1983, Falcon.
  * Falcons Wild - World Wide Poker (VK set 1),        1990, Video Klein.
  * Falcons Wild - World Wide Poker (VK set 2),        1990, Video Klein.
  * Falcons Wild - Wild Card 1991,                     1991, TVG.
  * Witch Card (Video Klein CPU box, set 1),           1991, Video Klein.
  * Witch Card (Video Klein CPU box, set 2),           1991, Video Klein.
  * Witch Card (Spanish, witch game, set 1),           1991, Unknown.
  * Witch Card (Spanish, witch game, set 2),           1991, Unknown.
  * Witch Card (English, no witch game),               1991, Unknown.
  * Witch Card (German, WC3050, set 1 ),               1994, Proma.
  * Witch Card (English, witch game, lamps),           1985, PM / Beck Elektronik.
  * Witch Card (Falcon, enhanced sound),               199?, Falcon.
  * Witch Card (German, WC3050, set 2 ),               1994, Proma.
  * Witch Card (German, WC3050, 27-4-94),              1994, Proma.
  * Witch Card (ICP-1, encrypted),                     199?, Unknown.
  * Witch Game (Video Klein, set 1),                   1991, Video Klein.
  * Witch Game (Video Klein, set 2),                   1991, Video Klein.
  * Joker Card (witch game),                           199?, Unknown.
  * Jolli Witch (Export, 6T/12T ver 1.57D),            1994, Video Klein?.
  * Wild Witch (Export, 6T/12T ver 1.57-SP),           1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.57-TE),           1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.62A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.62B),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.62A-F),           1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.62A alt),         1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.62B alt),         1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.65A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.65A-S),           1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.65A-S alt),       1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.65A-N),           1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.70A beta),        1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.70A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.70A alt),         1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.74A-SP-BELG),     1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.74A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.74A alt),         1992-2001, Video Klein.
  * Wild Witch (Export, 6B/12B ver 1.75A-E English),   1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.76A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.77A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.79A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.83A),             1992-2001, Video Klein.
  * Wild Witch (Export, 6T/12T ver 1.84A),             1992-2001, Video Klein.
  * Witch Up & Down (Export, 6T/12T ver 0.99, set 1),  1998, Video Klein.
  * Witch Up & Down (Export, 6T/12T ver 0.99, set 2),  1998, Video Klein.
  * Witch Up & Down (Export, 6T/12T ver 0.99, set 3),  1998, Video Klein.
  * Witch Up & Down (Export, 6T/12T ver 0.99T),        1998, Video Klein.
  * Witch Up & Down (Export, 6T/12T ver 1.02),         1998, Video Klein.
  * Witch Strike (Export, 6T/12T ver 1.01A),           1992, Video Klein.
  * Witch Strike (Export, 6T/12T ver 1.01B),           1992, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.40),              1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.40T),             1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.62),              1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.64),              1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.65),              1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.70S),             1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.70P),             1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.87),              1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.87-88),           1992-1996, Video Klein.
  * Witch Jack (Export, 6T/12T ver 0.87-89),           1992-1996, Video Klein.
  * Witch Jackpot (Export, 6T/12T ver 0.25),           1992-1996, Video Klein.
  * PlayMan Poker (German),                            1981, PM / Beck Elektronik.
  * Casino Poker (Ver PM86LO-35-5, German),            1987, PM / Beck Elektronik.
  * Casino Poker (Ver PM86-35-1, German),              1986, PM / Beck Elektronik.
  * Casino Poker (Ver PM88-01-21, German),             1988, PM / Beck Elektronik.
  * Royale (set 1),                                    198?, Unknown.
  * Royale (set 2),                                    198?, Unknown.
  * Super Loco 93 (Spanish, set 1),                    1993, Unknown.
  * Super Loco 93 (Spanish, set 2),                    1993, Unknown.
  * Maverik,                                           198?, Unknown.
  * Brasil 86,                                         1986, Unknown.
  * Brasil 87,                                         1987, Unknown.
  * Brasil 89 (set 1),                                 1989, Unknown.
  * Brasil 89 (set 2),                                 1989, Unknown.
  * Brasil 93,                                         1993, Unknown.
  * Poker 91,                                          1991, Unknown.
  * Genie (ICP-1, set 1),                              198?, Video Fun Games Ltd.
  * Genie (ICP-1, set 2),                              198?, Unknown.
  * Silver Game,                                       1983, Unknown.
  * Silver Game,                                       1983, Unknown.
  * Bonus Poker,                                       1984, Galanthis Inc.
  * "Unknown French poker game",                       198?, Unknown.
  * "Unknown encrypted poker game",                    198?, Unknown.
  * "Good Luck! poker (Sisteme France)",               198?, Sisteme France.
  * Bonne Chance! (Golden Poker prequel HW, set 1),    198?, Unknown.
  * Bonne Chance! (Golden Poker prequel HW, set 2),    198?, Unknown.
  * Boa Sorte! (Golden Poker prequel HW),              198?, Unknown.
  * Mundial/Mondial (Italian/French),                  1987, Unknown.
  * Super 98 (3-hands, ICP-1),                         199?, Unknown.
  * unknown rocket/animal-themed poker,                199?, Unknown.
  * Super 21,                                          1987, Public MNG.
  * Open 5 Cards,                                      1987, MNG.
  * Le Super Pendu (V1, words set #1),                 198?, Voyageur de L'Espace Inc..
  * Le Super Pendu (V1, words set #2),                 198?, Voyageur de L'Espace Inc..
  * Mega Double Poker (conversion kit, set 1),         1990, Blitz System Inc.
  * Mega Double Poker (conversion kit, set 2),         1990, Blitz System Inc.
  * Maxi Double Poker (version 1.8),                   1990, Blitz System Inc.
  * Wild Card (Olympic Games, v2.0),                   1989, Olympic Video Gaming PTY LTD.
  * Black jack (Olympic Games, v5.04, upgrade for WC), 1989, Olympic Video Gaming PTY LTD.


************************************************************************************


  I think "Diamond Poker Double Up" from Bonanza Enterprises should run on this hardware too.
  https://flyers.arcade-museum.com/?page=thumbs&db=videodb&id=4539

  Big-Boy and Mini-Boy are different sized cabinets for Bonanza Enterprises games.
  https://flyers.arcade-museum.com/?page=thumbs&db=videodb&id=4616
  https://flyers.arcade-museum.com/?page=thumbs&db=videodb&id=4274


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


  * Witch Card (Spanish sets)

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


  * Wild Witch / Jolli Witch

  These sets have a switch to change the game. Wild Witch comes with a complete
  Witch Game as switchable alternative, and Jolli Witch has Witch Card in the same
  package. Both are based in the 6T/12T program made by Video Klein, However, Jolli
  Witch seems to be a bootleg CPU box on an original Bonanza mainboard.

  The first time the game boots, will show a black & red screen with some options
  due to the lack or corrupt NVRAM. You must choose HOLD1 to create a new default
  NVRAM. In case you have corrupt NVRAM (not first boot), you can choose HOLD5 to
  attempt recover the old settings.


  * Witch Card (Proma)

  For the first time: You must coin-up and play at least one hand, then Payout
  to get the proper coinage settings.


  * Casino Poker

  Bipolar PROM 24sa10 is filled with 0x09, so has at least
  fixed bits 0 and 3 along the whole data. Needs a redump using a supported
  EEPROM programmer.

  Discrete sound circuitry was traced, being identical to the Golden Poker one.
  Only difference is the PC617 replaced by one PC817.

  The sound is ugly and seems that was programmed that way.
  Also the lamps work in test mode, but seems to be avoided in the game code.

  The game has 2 service switches/buttons:
  One for settings, and other just for bookkeeping.

  Here the original Service Card (in German), and the English translation:
   _____________________________________     _____________________________________
  |                                     |   |                                     |
  |          SERVICE ANLEITUNG          |   |           SERVICE MANUAL            |
  |   _____________   _____________     |   |   _____________   _____________     |
  |  |  +-------+  | |  +-------+  |    |   |  |  +-------+  | |  +-------+  |    |
  |  |  |0000000|  | |  |0000000|  |    |   |  |  |0000000|  | |  |0000000|  |    |
  |  |  +-------+  | |  +-------+  |    |   |  |  +-------+  | |  +-------+  |    |
  |  |             | |             |    |   |  |             | |             |    |
  |  |             | |             |    |   |  |             | |             |    |
  |  |             | |             |    |   |  |             | |             |    |
  |  | +---------+ | |+-----------+|    |   |  | +---------+ | |+-----------+|    |
  |  | |Einnahmen| | ||Kredit Off ||    |   |  | |Earnings | | ||Credit Off ||    |
  |  | +---------+ | |+-----------+|    |   |  | +---------+ | |+-----------+|    |
  |  |             | |             |    |   |  |             | |             |    |
  |  |_____________| |_____________|    |   |  |_____________| |_____________|    |
  |                                     |   |                                     |
  |  Zaehler zeigen volle DM-Betraege   |   |     METER SHOW FULL DM-AMOUNTS      |
  |                                     |   |                                     |
  |       _____            _____        |   |       _____            _____        |
  |      |     |     |    |     |       |   |      |     |     |    |     |       |
  |      | SW  |     |    | RT  |       |   |      | SW  |     |    | RT  |       |
  |      |_____|     |    |_____|       |   |      |_____|     |    |_____|       |
  |                  |                  |   |                  |                  |
  |      SERVICE     |     METER        |   |      SERVICE     |     METER        |
  |                  |                  |   |                  |                  |
  |    +---------+   | ZEIGT DIE        |   |    +---------+   | SHOWS THE        |
  |    |  LEARN  |   | ELEKT. ZAEHLER-  |   |    |  LEARN  |   | ELECTRONIC METER-|
  |    +---------+   | STAENDE UND      |   |    +---------+   | READING AND      |
  |                  | STATISTIK AN     |   |                  | STATISTIC        |
  |   PROGRAMMIER.   |                  |   |   PROGRAMMING    |                  |
  |   DER KREDIT-    |                  |   |   THE CREDIT-    |                  |
  |   EINGAENGE UND  +------------------|   |   INPUT AND      +------------------|
  |   GEWINNQUOTE    |                  |   |   PROFIT SHARE   |                  |
  |                  |    < POT >       |   |                  |    < POT >       |
  |    +---------+   |  +         -     |   |    +---------+   |  +         -     |
  |    |   NEU   |   |  LAUTSTAERKE     |   |    |   NEW   |   |  SOUND VOLUME    |
  |    +---------+   |                  |   |    +---------+   |                  |
  |                  |                  |   |                  |                  |
  |   SETZT DIE      |                  |   |   RESET THE      |                  |
  |   ELEKT. ZAEHLER |    < POT >       |   |   ELECTRONIC     |    < POT >       |
  |   ZURUECK        |                  |   |   COUNTER        |                  |
  |                  | BLAU<->SCHWARZ   |   |                  | BLUE <-> BLACK   |
  |    +---------+   | HINTERGRUND      |   |    +---------+   | BACKGROUND       |
  |    |  TEST   |   |                  |   |    |  TEST   |   |                  |
  |    +---------+   |                  |   |    +---------+   |                  |
  |                  |                  |   |                  |                  |
  |   CPU, TASTATUR  |                  |   |   CPU, KEYBOARD  |                  |
  |   UND LAMPEN     |                  |   |   AND LAMPS      |                  |
  |   TEST           |                  |   |   TEST           |                  |
  |   GITTERMUSTER   |                  |   |   LATTICE DESIGN |                  |
  |                  |                  |   |                  |                  |
  |    +---------+   |                  |   |    +---------+   |                  |
  |    |  RESET  |   |                  |   |    |  RESET  |   |                  |
  |    +---------+   |                  |   |    +---------+   |                  |
  |                  |                  |   |                  |                  |
  |   LOESCHT ALLES  |                  |   |    DELETE ALL    |                  |
  |                  |                  |   |                  |                  |
  |------------------+------------------|   |------------------+------------------|
  |        DIPSWITCHEINSTELLUNG         |   |        DIPSWITCH SETTINGS           |
  |                                     |   |                                     |
  |            ON      OFF              |   |            ON      OFF              |
  |           +-----------+             |   |           +-----------+             |
  |           | +-------+ |             |   |           | +-------+ |             |
  |       ON  | |###|   |8|             |   |       ON  | |###|   |8|             |
  |           | +---+---+ |             |   |           | +---+---+ |             |
  |       ON  | |###|   | |             |   |       ON  | |###|   | |             |
  |           | +-------+ |             |   |           | +-------+ |             |
  |           | |   |###| |             |   |           | |   |###| |             |
  |           | +---+---+ |             |   |           | +---+---+ |             |
  |           | |   |###| |             |   |           | |   |###| |             |
  |           | +-------+ |             |   |           | +-------+ |             |
  |R-FLUSH EIN| |###|   | |R-FLUSH AUS  |   | R-FLUSH ON| |###|   | |R-FLUSH OFF  |
  |           | +---+---+ |             |   |           | +---+---+ |             |
  |           | |###|   | |             |   |           | |###|   | |             |
  |           | +-------+ |             |   |           | +-------+ |             |
  |           | |   |###| |AUSZAHLUNG   |   |           | |   |###| |PAYOUT       |
  |           | +---+---+ |             |   |           | +---+---+ |             |
  |           | |###|   |1|AUSZAHLUNG   |   |           | |###|   |1|PAYOUT       |
  |           | +-------+ |             |   |           | +-------+ |             |
  |           +-----------+             |   |           +-----------+             |
  |                                     |   |                                     |
  |  0=WENIG GEWINNE    1=STANDARD      |   |  0=LITTLE GAININGS   1=DEFAULT      |
  |  2=MEHR GEWINNE     3=VIELE GEWIN.  |   |  2=MORE GAININGS    3=MANY GAININGS |
  |                                     |   |                                     |
  |                                     |   |                                     |
  |   PM 1987 - BECK D-6330 WETZLAR     |   |   PM 1987 - BECK D-6330 WETZLAR     |
  |_____________________________________|   |_____________________________________|


  * Royale

  These sets are running in Golden Poker hardware (A0-A14), but with a hardware mod.
  The multiplexer selector writes 3F-2F-1F-0F for all the different input states,
  instead of 7F-BF-DF-EF (the normal ones for Golden Poker hardware). This turns
  the inputs system unusable if you do a ROM swap. Just meant for protection.

  Now that the mux system is totally understood, both sets are working properly.


  * Bonne Chance! (Golden Poker prequel hardware)

  The hardware is a sequel of Magic Fly, prequel of Golden Poker.
  The color PROM data is inverted through gates, latches or PLDs
  to get the final palette.


  * Mundial/Mondial (Italian/French)

  This game has two different programs inside the program ROM in banks of 0x4000 each.
  The first program is meant for Italian language, while the second one is for French.

  There is nothing (no writes) that point to a banking. Maybe is driven by PLDs, or
  just routed to the unused DIP switches of the bank (4 lines of the port are used by
  discrete sound). Otherwise should be splitted in different games.... Need to check
  the real board behaviour.

  For now, I implemented the banking and set the first program (Italian) fixed into
  the driver init till we can get more evidence about.


  * Super 98

  This game looks like a Golden Poker / Potten's Poker set, but could be set to play
  2 or 3 deals per hand. It's running in a ICP-1 PCB.

  Entering the service mode (key 0), you can enter to a submenu for settings pressing
  DEAL (key 2):

    Win Limit (0-9 the first time, then 3-9)
    Payout Limit (0-2)
    Double Odds % (0-9)
    Payout % (0-9)
    Hard Setting (0-4)
    Note Value (fixed in 5)

    Bonus (0 to 10000, step x100)
    (if you rech 10000, settings exits and go to game. You can't change bonus anymore.

  Use HOLD keys (keys ZXCVB) to navigate through the menu and change
  the values. Press CANCEL to exit the settings menu.

  ** NOTE **
  2-3 deals, turbo mode, bonus and royal flush could be set through DIP switches.


  Program is currently not working because seems to fill some zeropage registers and
  check for existent values and changes... Maybe an external device is writing them.
  This is NVRAM zone, so some values could be previously hardcoded.

  Also seems to manipulate the stack pointer expecting different values, and some inputs
  combination entered to boot.

  To run...
  1) Start the game.
  2) Break into debugger and do a pc=cfa1

  Debug notes...

  From interrupts routine:

  CF99: LDA $0846    ; load from PIA
  CF9C: TSX          ; transfer stack pointer to X
  CF9D: CPX #$C8     ; compare with 0xC8
  CF9F: BCS $CFA4    ; not?... branch to $CFA4
  CFA1: JMP $CEC6    ; yes?... jump to $CEC6
  CFA4: JSR $C0E1    ; continue...
  ...

  Forcing the first time the comparation at $CF9D --> true, the game boots and is
  fully working.


  For now will patch the dead end to get it working.
  e9f4: jmp $ea56 ---> death
  e9f7: jmp $cec6 ---> works

  The game needs ~25 seconds to check all the things to start.


************************************************************************************


  Hardware Notes (pmpoker):

  - CPU:            1x M6502.
  - Video:          1x MC6845.
  - RAM:            2x SCM21C14E, 4x uPD2114LC
  - I/O             2x 6821 PIAs.
  - prg ROMs:       3x 2732 (32Kb) or similar.
  - gfx ROMs:       4x 2716 (16Kb) or similar.
  - sound:          (discrete).


  PCB Layout (pmpoker):
   _______________________________________________________________________________
  |   _________                                                                   |
  |  |         |               -- DIP SW x8 --                                    |
  |  | Battery |   _________   _______________   _________  _________   ________  |
  |  |   055   |  | 74LS32  | |1|2|3|4|5|6|7|8| | HCF4011 || HCF4096 | | LM339N | |
  |  |_________|  |_________| |_|_|_|_|_|_|_|_| |_________||_________| |________| |
  |       _________     _________   _________   _________                         |
  |      | 74LS138 |   | 21C14E  | | 74LS08N | | 74LS42  |                        |
  |      |_________|   |_________| |_________| |_________|                        |
  |  _______________    _________   ____________________                      ____|
  | |               |  | 21C14E  | |                    |                    |
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
  | |                | | 2114-LC | | 74LS08H | |TBP24SA10| <-- socketed.      ____|
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


************************************************************************************

  Mega Double Poker is distributed as standalone PCB, or as upgrade kit for
  modified Golden Poker boards.

  Hardware Notes (Mega Double Poker, kit):

  - CPU:            1x R6502AP.
  - MCU:            1x MC68705P5S.
  - Video:          1x MC6845.
  - RAM:            4x uPD2114LC or similar
  - I/O             2x 6821 PIAs.
  - prg ROMs:       2x 2732 (32KB) or similar.
  - gfx ROMs:       3x 2732 (32KB) or similar.
  - sound:          (discrete).
  - battery backup: 2x S8423


  PCB Layout: Main board.
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


  PCB Layout: Daughterboard.
   ________________________________________________________
  |                                ::::::::::::::::::::    |
  |   __________                     40-pin connector      |
  |  | GD74LS04 | U8                                       |
  |  |__________|                                          |
  |   ____________                ______________________   |
  |  |KS74HCTLS08N| U9           |                      |  |
  |  |____________|              |     R6502AP (U6)     |  |
  |   ____________               |______________________|  |
  |  |KS74HCTLS32N| U10                                    |
  |  |____________|                                        |
  |   _____________                                        |
  |  |KS74HCTLS74AN| U11                            _______|
  |  |_____________|         _______________       |
  |   _____________         |  Unknown RAM  |      |
  |  |KS74HCTLS139N| U12    |     (U5)      |      |
  |  |_____________|        |_______________|      |
  |                          _________________     |
  |                         |   27C256 (U2)   |    |
  |                         |     MEGA-2      |    |
  |                         |_________________|    |
  |                          _________________     |
  |                         |   27C256 (U3)   |    |
  |     _____________       |     MEGA-3      |    |
  |    |KS74HCTLS374N|      |_________________|    |
  |    |_____________|       _________________     |
  |      U13                |Empty Socket (U4)|    |
  |                         |  'SPARE EPROM'  |    |
  |   _______               |_________________|    |
  |  |       |                                     |
  |  |MC68705|   ____________                      |
  |  |  P5S  |  |KS74HCTLS86N| U14                 |
  |  |       |  |____________|                     |
  |  |MEGA-1 |   ____________     _____________    |
  |  |       |  |KS74HCTLS86N|   |KS74HCTLS245N|   |
  |  | (U1)  |  |____________|   |_____________|   |
  |  |_______|    U15              U16  __         |
  |                                    /--\ BLITZ  |
  | Model B0-BL-01B                    \__/ SYSTEM |
  |________________________________________________|


  Connections... (pins in parenthesis)
  ------------------------------------

  The following diagrams are still incomplete and could have errors.
  At simple sight, mega-2.u2 & mega-3.u3 ROMs are sharing the same
  addressing space, but /CE line for both devices are connected to
  the same places... Need to be traced from the scratch.

  Also CPU lines A14 & A15 should be traced to know the addressing
  system accurately.


                                                     CPU  R6502AP (U6)
                                                   .--------\ /--------.
  MCU (01-05-07), MEGA-2 (14-20), MEGA-3 (14-20) --|01 VSS   '  /RES 40|--
                                        MCU (02) --|02 RDY    PH2(O) 39|--
                                        MCU (04) --|03 PH1(O)    /SO 38|--
                                                 --|04 /IRQ   PH0(I) 37|--
                                                 --|05 (NC)     (NC) 36|--
  MCU (03-06), MEGA-2 (01-27-28), MEGA-3 (01-28) --|06 /NMI     (NC) 35|--
                                                 --|07 SYNC      R/W 34|--
  MCU (03-06), MEGA-2 (01-27-28), MEGA-3 (01-28) --|08 VCC        D0 33|-- MEGA-2 (11)
                        MEGA-2 (10), MEGA-3 (10) --|09 A0         D1 32|-- MEGA-2 (12)
                        MEGA-2 (09), MEGA-3 (09) --|10 A1         D2 31|--
                        MEGA-2 (08), MEGA-3 (08) --|11 A2         D3 30|-- MEGA-2 (15)
                        MEGA-2 (07), MEGA-3 (07) --|12 A3         D4 29|-- MEGA-2 (16)
                        MEGA-2 (06), MEGA-3 (06) --|13 A4         D5 28|-- MEGA-2 (17)
                        MEGA-2 (05), MEGA-3 (05) --|14 A5         D6 27|-- MEGA-2 (18)
                        MEGA-2 (04), MEGA-3 (04) --|15 A6         D7 26|-- MEGA-2 (19)
                        MEGA-2 (03), MEGA-3 (03) --|16 A7        A15 25|--
                        MEGA-2 (25), MEGA-3 (25) --|17 A8        A14 24|--
                        MEGA-2 (24), MEGA-3 (24) --|18 A9        A13 23|-- MEGA-2 (26), MEGA-3 (26)
                        MEGA-2 (21), MEGA-3 (21) --|19 A10       A12 22|-- MEGA-2 (02), MEGA-3 (02)
                        MEGA-2 (23), MEGA-3 (23) --|20 A11       VSS 21|-- MCU (01-05-07), MEGA-2 (14-20), MEGA-3 (14-20)
                                                   '-------------------'


                                      MCU MC68705P5S (U1)
                                     .--------\ /--------.
  MEGA-2 (12, 20), MEGA-3 (12, 20) --|01 VSS   '  /RES 28|--
                          CPU (02) --|02 /INT      PA7 27|--
  MEGA-2 (01, 28), MEGA-3 (01, 28) --|03 VCC       PA6 26|--
                          CPU (03) --|04 EXTAL     PA5 25|--
  MEGA-2 (14, 20), MEGA-3 (14, 20) --|05 XTAL      PA4 24|--
  MEGA-2 (27, 28), MEGA-3 (27, 28) --|06 VPP       PA3 23|--
  MEGA-2 (14, 20), MEGA-3 (14, 20) --|07 TIMER     PA2 22|--
                                   --|08 PC0       PA1 21|--
                                   --|09 PC1       PA0 20|--
                                   --|10 PC2       PB7 19|--
                                   --|11 PC3       PB6 18|--
                                   --|12 PB0       PB5 17|--
                                   --|13 PB1       PB4 16|--
                                   --|14 PB2       PB3 15|--
                                     '-------------------'


                         MEGA-2 27C256 (U2)                               MEGA-3 27C256 (U3)
                        .-------\ /-------.                              .-------\ /-------.
                      --|01 VPP  '  VCC 28|--                          --|01 VPP  '  VCC 28|--
                      --|02 A12     A14 27|--                          --|02 A12     A14 27|--
                      --|03 A7      A13 26|--                          --|03 A7      A13 26|--
                      --|04 A6       A8 25|--                          --|04 A6       A8 25|--
                      --|05 A5       A9 24|--                          --|05 A5       A9 24|--
                      --|06 A4      A11 23|--                          --|06 A4      A11 23|--
                      --|07 A3      /OE 22|--                          --|07 A3      /OE 22|--
                      --|08 A2      A10 21|--  .-- MCU (01-05-07)      --|08 A2      A10 21|--  .-- MCU (01-05-07)
                      --|09 A1      /CE 20|----+-- MEGA-3 (14-20)      --|09 A1      /CE 20|----+-- MEGA-2 (14-20)
                      --|10 A0       D7 19|--  '-- CPU (01-21)         --|10 A0       D7 19|--  '-- CPU (01-21)
                      --|11 D0       D6 18|--                          --|11 D0       D6 18|--
                      --|12 D1       D5 17|--                          --|12 D1       D5 17|--
  MCU (01-05-07) --.  --|13 D2       D4 16|--      MCU (01-05-07) --.  --|13 D2       D4 16|--
  MEGA-3 (14-20) --+----|14 GND      D3 15|--      MEGA-2 (14-20) --+----|14 GND      D3 15|--
     CPU (01-21) --'    '-----------------'           CPU (01-21) --'    '-----------------'


************************************************************************************


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


************************************************************************************


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


************************************************************************************


  TODO:

  - Missing PIA connections.
  - Final cleanup and split the driver.


************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6502/r65c02.h"
#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "machine/6821pia.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/segacrpt_device.h"
#include "machine/timekpr.h"
#include "sound/ay8910.h"
#include "sound/discrete.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "pmpoker.lh"
#include "goldnpkr.lh"
#include "upndown.lh"
#include "lespendu.lh"


namespace {

#define MASTER_CLOCK    XTAL(10'000'000)
#define CPU_CLOCK       (MASTER_CLOCK/16)
#define PIXEL_CLOCK     (MASTER_CLOCK/2)


class goldnpkr_state : public driver_device
{
public:
	goldnpkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia%u", 0U),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_ay8910(*this, "ay8910"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void wildcard(machine_config &config);
	void wildcrdb(machine_config &config);
	void witchcrd(machine_config &config);
	void mondial(machine_config &config);
	void wcfalcon(machine_config &config);
	void geniea(machine_config &config);
	void genie(machine_config &config);
	void pottnpkr(machine_config &config);
	void goldnpkr(machine_config &config);
	void witchcdj(machine_config &config);
	void wcrdxtnd(machine_config &config);
	void super21p(machine_config &config);
	void op5cards(machine_config &config);
	void caspoker(machine_config &config);
	void icp_ext(machine_config &config);
	void gldnirq0(machine_config &config);
	void lespendu(machine_config &config);

	void init_vkdlswwh();
	void init_icp1db();
	void init_vkdlswwp();
	void init_vkdlsww();
	void init_vkdlsb();
	void init_vkdlsc();
	void init_vkdlswwl();
	void init_vkdlswwu();
	void init_vkdlswwo();
	void init_vkdlswwa();
	void init_vkdlsa();
	void init_vkdlswwt();
	void init_vkdlswwd();
	void init_wstrike();
	void init_vkdlswws();
	void init_vkdlswwc();
	void init_vkdlswwr();
	void init_vkdlswwv();
	void init_bchancep();
	void init_bonuspkr();
	void init_super98();
	void init_pokersis();
	void init_lespendu();
	void init_lespenduj();
	void init_op5cards();
	void init_olym65();

	uint8_t pottnpkr_mux_port_r();
	void lamps_a_w(uint8_t data);
	void lespendu_lamps_a_w(uint8_t data);
	void sound_w(uint8_t data);
	void mux_w(uint8_t data);
	void lespendu_mux_w(uint8_t data);

	uint32_t screen_update_goldnpkr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }

	virtual void video_start() override ATTR_COLD;

	void goldnpkr_videoram_w(offs_t offset, uint8_t data);
	void goldnpkr_colorram_w(offs_t offset, uint8_t data);

	void witchcrd_palette(palette_device &palette) const;
	void super21p_palette(palette_device &palette) const;

	void goldnpkr_base(machine_config &config);

	required_device<cpu_device> m_maincpu;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<discrete_device> m_discrete;

private:
	uint8_t goldnpkr_mux_port_r();
	void mux_port_w(uint8_t data);
	uint8_t ay8910_data_r();
	void ay8910_data_w(uint8_t data);
	void ay8910_control_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(wcrdxtnd_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(super21p_get_bg_tile_info);
	void goldnpkr_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(wcrdxtnd);
	DECLARE_VIDEO_START(super21p);
	void wcrdxtnd_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(mondial);
	DECLARE_MACHINE_START(lespendu);
	DECLARE_MACHINE_RESET(mondial);
	DECLARE_MACHINE_RESET(lespendu);

	void genie_map(address_map &map) ATTR_COLD;
	void goldnpkr_map(address_map &map) ATTR_COLD;
	void mondial_map(address_map &map) ATTR_COLD;
	void witchcdj_map(address_map &map) ATTR_COLD;
	void pottnpkr_map(address_map &map) ATTR_COLD;
	void wcrdxtnd_map(address_map &map) ATTR_COLD;
	void wildcard_map(address_map &map) ATTR_COLD;
	void wildcrdb_map(address_map &map) ATTR_COLD;
	void wildcrdb_mcu_io_map(address_map &map) ATTR_COLD;
	void wildcrdb_mcu_map(address_map &map) ATTR_COLD;
	void wildcrdb_mcu_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void witchcrd_falcon_map(address_map &map) ATTR_COLD;
	void witchcrd_map(address_map &map) ATTR_COLD;
	void super21p_map(address_map &map) ATTR_COLD;
	void op5cards_map(address_map &map) ATTR_COLD;
	void icp_ext_map(address_map &map) ATTR_COLD;
	void lespendu_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_device<ay8910_device> m_ay8910;
	output_finder<5> m_lamps;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_mux_data = 0;
	uint8_t m_pia0_PA_data = 0;
	uint8_t m_ay8910_data = 0;
	uint8_t m_ay8910_control = 0;
};


class blitz_state : public goldnpkr_state
{
public:
	blitz_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldnpkr_state(mconfig, type, tag),
		m_cpubank(*this, "cpubank"),
		m_mcu(*this, "mcu"),
		m_bankdev(*this, "bankdev"),
		m_cpubank_xor(0),
		m_portc_data(0x0f)
	{ }

	void megadpkr(machine_config &config);

private:
	uint8_t cpubank_decrypt_r(offs_t offset);
	void mcu_command_w(uint8_t data);
	void mcu_portb_w(uint8_t data);
	void mcu_portc_w(uint8_t data);
	void megadpkr_banked_map(address_map &map) ATTR_COLD;
	void megadpkr_map(address_map &map) ATTR_COLD;

	required_region_ptr<uint8_t> m_cpubank;

	required_device<m68705p_device> m_mcu;
	required_device<address_map_bank_device> m_bankdev;

	uint8_t m_cpubank_xor;
	uint8_t m_portc_data;
};


/*********************************************
*               Video Hardware               *
*********************************************/


void goldnpkr_state::goldnpkr_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void goldnpkr_state::goldnpkr_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(goldnpkr_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    ---- --x-   tiles bank.
    ---- ---x   tiles extended address (MSB).
    xx-- ----   unused.
*/

	int attr = m_colorram[tile_index];
	int code = ((attr & 1) << 8) | m_videoram[tile_index];
	int bank = (attr & 0x02) >> 1;      // bit 1 switch the gfx banks
	int color = (attr & 0x3c) >> 2;     // bits 2-3-4-5 for color

	tileinfo.set(bank, code, color, 0);
}

TILE_GET_INFO_MEMBER(goldnpkr_state::wcrdxtnd_get_bg_tile_info)
{
/* 16 graphics banks system for VK extended (up & down) PCB's

    - bits -
    7654 3210
    --xx xx--   tiles color.
    xx-- --xx   tiles bank.
*/

	int attr = m_colorram[tile_index];
	int code = ((attr & 1) << 8) | m_videoram[tile_index];
	int bank = (attr & 0x03) + ((attr & 0xc0) >> 4);    // bits 0, 1, 6 & 7 switch the gfx banks
	int color = (attr & 0x3c) >> 2;                     // bits 2-3-4-5 for color

	tileinfo.set(bank, code, color, 0);
}

TILE_GET_INFO_MEMBER(goldnpkr_state::super21p_get_bg_tile_info)
{
/* 4 graphics banks system for Super 21 extended graphics.

    - bits -
    7654 3210
    -xxx ----   tiles color.
    x--- --xx   tiles bank.
*/

	int attr = m_colorram[tile_index];
	int code = ((attr & 1) << 8) | m_videoram[tile_index];
	int bank = (attr & 0x03);       // bits 0-1, switch the gfx banks
	int color = (attr & 0x70) >> 3; // bits 4-5-6 for color, shifted x2 to match the color groups used.

	tileinfo.set(bank, code, color, 0);

/*
  Color codes GFX bank 0 (chars)

  00 = Black.
  10 = Red.
  20 = Green.
  30 = Yellow.
  40 = Blue.
  50 = Magenta.
  60 = Cyan.
  70 = White.

  In test mode, the DIP switches status are assigned with code 00,
  so you can't see them since they are black on black background.

*/
}

void goldnpkr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goldnpkr_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

VIDEO_START_MEMBER(goldnpkr_state, wcrdxtnd)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goldnpkr_state::wcrdxtnd_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

VIDEO_START_MEMBER(goldnpkr_state, super21p)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goldnpkr_state::super21p_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t goldnpkr_state::screen_update_goldnpkr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void goldnpkr_state::goldnpkr_palette(palette_device &palette) const
{
/*  prom bits
    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   intensity.
    xxxx ----   unused.
*/

	// 0000IBGR
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		constexpr int intenmin = 0xe0;
//      constexpr int intenmin = 0xc2;    // 2.5 Volts (75.757575% of the whole range)
		constexpr int intenmax = 0xff;    // 3.3 Volts (the whole range)

		// intensity component
		int const inten = BIT(color_prom[i], 3);

		// red component
		int const r = BIT(color_prom[i], 0) * (inten ? intenmax : intenmin);

		// green component
		int const g = BIT(color_prom[i], 1) * (inten ? intenmax : intenmin);

		// blue component
		int const b = BIT(color_prom[i], 2) * (inten ? intenmax : intenmin);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void goldnpkr_state::witchcrd_palette(palette_device &palette) const
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

	// 0000KBGR
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		// blue killer (from schematics)
		int const bk = BIT(color_prom[i], 3);

		// red component
		int const r = BIT(color_prom[i], 0) * 0xff;

		// green component
		int const g = BIT(color_prom[i], 1) * 0xff;

		// blue component
		int const b = bk * BIT(color_prom[i], 2) * 0xff;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void goldnpkr_state::wcrdxtnd_palette(palette_device &palette) const
{
/*
    Using the original intensity line, the PCB has a bridge
    that allow (as default) turn the background dark blue.

    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   intensity / blue killer.
    xxxx ----   unused.
*/
	// 0000KBGR
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		// blue killer (from schematics)
		int const bk = BIT(color_prom[i], 3);

		// red component
		int const r = BIT(color_prom[i], 0) * 0xff;

		// green component
		int const g = BIT(color_prom[i], 1) * 0xff;

		// blue component
		int b = bk * BIT(color_prom[i], 2) * 0xff;
		//if (!b & bk)   --> needs better implementation
		//  b = 0x3f;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void goldnpkr_state::super21p_palette(palette_device &palette) const
{
/*
    Each graphics bank has its own palette. The first 3 are tied to
    bipolar PROMs, and the las one is RGB direct.

    GFX bank 0 ---> bipolar PROM 1
    GFX bank 1 ---> bipolar PROM 2
    GFX bank 2 ---> bipolar PROM 3
    GFX bank 3 ---> RGB direct.

    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   unknown (maybe intensity).
    xxxx ----   unused.
*/

	// 0000?BGR
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < (palette.entries() * 3) / 4; i++)
	{
//      last quarter of palette is RGB direct, for gfx bank 3 (title)
//      todo: implement bit 3.

		// red component
		int const r = BIT(color_prom[i], 0) * 0xff;

		// green component
		int const g = BIT(color_prom[i], 1) * 0xff;

		// blue component
		int const b = BIT(color_prom[i], 2) * 0xff;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*******************************************
*               R/W Handlers               *
*******************************************/

/* Inputs (buttons) are multiplexed.
   There are 4 sets of 5 bits each and are connected to PIA0, portA.
   The selector bits are located in PIA1, portB (bits 4-7).
*/
uint8_t goldnpkr_state::goldnpkr_mux_port_r()
{
	switch( m_mux_data & 0xf0 )     // bits 4-7
	{
		// normal selector writes 7F-BF-DF-EF
		case 0x10: return ioport("IN0-0")->read();
		case 0x20: return ioport("IN0-1")->read();
		case 0x40: return ioport("IN0-2")->read();
		case 0x80: return ioport("IN0-3")->read();

		// royale selector writes 3F-2F-1F-0F.
		// worth to split a whole machine driver just for this?
		case 0xc0: return ioport("IN0-3")->read();
		case 0xd0: return ioport("IN0-2")->read();
		case 0xe0: return ioport("IN0-1")->read();
		case 0xf0: return ioport("IN0-0")->read();
	}
	return 0xff;
}

uint8_t goldnpkr_state::pottnpkr_mux_port_r()
{
	uint8_t pa_0_4 = 0xff, pa_7;    // temporary placeholder for bits 0 to 4 & 7

	switch( m_mux_data & 0xf0 )     // bits 4-7
	{
		case 0x10: return ioport("IN0-0")->read();
		case 0x20: return ioport("IN0-1")->read();
		case 0x40: return ioport("IN0-2")->read();
		case 0x80: return ioport("IN0-3")->read();
	}

	pa_7 = (m_pia0_PA_data >> 7) & 1;   // to do: bit PA5 to pin CB1

	return ( (pa_0_4 & 0x3f) | (pa_7 << 6) | (pa_7 << 7) ) ;
}

void goldnpkr_state::mux_w(uint8_t data)
{
	m_mux_data = data ^ 0xff;   // inverted
}

void goldnpkr_state::lespendu_mux_w(uint8_t data)
{
	m_mux_data = data ^ 0xff;   // inverted

	if(data == 0x00)
		data = 0xff;

	membank("bank0")->set_entry(data & 0x07); // for both sets
}


void goldnpkr_state::mux_port_w(uint8_t data)
{
	m_pia0_PA_data = data;
}


// demuxing ay8910 data/address from Falcon board, PIA portA out
uint8_t goldnpkr_state::ay8910_data_r()
{
	return (m_ay8910_control & 0xc0) == 0x40 ? m_ay8910->data_r() : 0xff;
}

void goldnpkr_state::ay8910_data_w(uint8_t data)
{
	m_ay8910_data = data;
}

void goldnpkr_state::ay8910_control_w(uint8_t data)
{
	if (BIT(data, 7))
		m_ay8910->data_address_w(BIT(data, 6), m_ay8910_data);

	m_ay8910_control = data;

	m_lamps[0] = !BIT(data, 0);
	m_lamps[1] = !BIT(data, 1);
	m_lamps[2] = !BIT(data, 2);
	m_lamps[3] = !BIT(data, 3);
	m_lamps[4] = !BIT(data, 4);
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

  --------
  wupndown
  --------

  7654 3210
  ---- ---x  Bet Lamp.
  ---- --x-  Deal Lamp.
  ---- -x--  Holds + Cancel Lamps.
  ---- x---  Take Lamp.

*/
void goldnpkr_state::lamps_a_w(uint8_t data)
{
/***** General Lamps and Counters wiring *****

  7654 3210
  ---- ---x  Bet lamp.
  ---- --x-  Deal lamp.
  ---- -x--  Holds + Cancel lamps.
  ---- x---  Double Up & Take lamps. (Coin In counter (inverted) for witchcrd, bsuerte and sloco93 sets)
  ---x ----  Big & Small lamps.
  --x- ----  Coin Out counter. Inverted for witchcrd, bsuerte and sloco93 sets.
  -x-- ----  Coin In counter.
  x--- ----  Note In counter (only goldnpkr).

*/
	data = data ^ 0xff;

	m_lamps[0] = BIT(data, 0);  // lamp 0
	m_lamps[1] = BIT(data, 1);  // lamp 1
	m_lamps[2] = BIT(data, 2);  // lamp 2
	m_lamps[3] = BIT(data, 3);  // lamp 3
	m_lamps[4] = BIT(data, 4);  // lamp 4

	machine().bookkeeping().coin_counter_w(0, data & 0x40);  // counter 1
	machine().bookkeeping().coin_counter_w(1, data & 0x80);  // counter 2
	machine().bookkeeping().coin_counter_w(2, data & 0x20);  // counter 3
}

void goldnpkr_state::lespendu_lamps_a_w(uint8_t data)
{
/* Le Super Pendu lamps and counters wiring *

  7654 3210
  ---- ---x  Lamp 1.
  ---- --x-  Lamp 2.
  ---- -x--  Lamp 3.
  ---- x---  Lamp 4.
  ---x ----  Lamp 5.
  --x- ----  Coin In counter.
  xx-- ----  Unused.
*/
	data = data ^ 0xff;

	m_lamps[0] = BIT(data, 0);  // lamp 0
	m_lamps[1] = BIT(data, 1);  // lamp 1
	m_lamps[2] = BIT(data, 2);  // lamp 2
	m_lamps[3] = BIT(data, 3);  // lamp 3
	m_lamps[4] = BIT(data, 4);  // lamp 4

	// counter 0, adding extra coin when boot for difference of polarisation.
	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
}


void goldnpkr_state::sound_w(uint8_t data)
{
	// 555 voltage controlled
	// discrete sound is connected to PIA1, portA: bits 0-3
	m_discrete->write(NODE_01, data >> 3 & 0x01);
	m_discrete->write(NODE_10, data & 0x07);
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void goldnpkr_state::goldnpkr_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x7fff).rom();  // superdbl uses 0x2000..0x3fff address space
}

void goldnpkr_state::witchcdj_map(address_map &map)
{
	goldnpkr_map(map);
	map(0x0801, 0x0801).unmaprw();
	map(0x0802, 0x0802).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

void goldnpkr_state::pottnpkr_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x3fff).rom();
}

void goldnpkr_state::witchcrd_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x2000).portr("SW2");
//  map(0x2108, 0x210b).noprw(); // unknown 40-pin device
	map(0x2800, 0x2fff).ram();
	map(0x4000, 0x7fff).rom();
}
/*
   Witch Card (Video klein)

   R/W:

   2108  RW
   2109   W
   210a   W
   210b   W

*/

void goldnpkr_state::witchcrd_falcon_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x2000).portr("SW2");
	map(0x2100, 0x2100).w("crtc", FUNC(mc6845_device::address_w));
	map(0x2101, 0x2101).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x7fff).rom();
}

void goldnpkr_state::wildcard_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x2000).portr("SW2");
	map(0x2200, 0x27ff).rom();  // for VK set
	map(0x2800, 0x2fff).ram();  // for VK set
	map(0x3000, 0xffff).rom();  // for VK set. bootleg starts from 4000
}

/*
  Video Klein extended hardware
  Extended graphics plus DS1210 + RAM
*/
void goldnpkr_state::wcrdxtnd_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); //.share("nvram"); // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x2000).portr("SW2");
	map(0x2200, 0x27ff).rom();  // for VK hardware
	map(0x2800, 0x2fff).ram().share("nvram");   // Dallas ds1210 + battery backed RAM
	map(0x3000, 0xffff).rom();  // for VK hardware. bootleg starts from 4000
}

/*

  VK = BP 703f

*/

void goldnpkr_state::wildcrdb_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x2000).portr("SW2");
	map(0x2100, 0x2100).w("crtc", FUNC(mc6845_device::address_w));
	map(0x2101, 0x2101).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2800, 0x2fff).ram().share("main_mcu_shared"); // TODO: verify it's the correct range
	map(0x3000, 0x7fff).rom();
}

void goldnpkr_state::wildcrdb_mcu_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x27ff).ram();
	map(0x8000, 0x87ff).ram().share("main_mcu_shared"); // TODO: verify it's the correct range
}

void goldnpkr_state::wildcrdb_mcu_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().share("decrypted_opcodes");
}

void goldnpkr_state::wildcrdb_mcu_io_map(address_map &map)
{
	map.global_mask(0xff);

	// map(0x00, 0x00).w() // TODO: what's this? alternates between 0x00 and 0x01
}

/*
  wildcrdb:

  Code checks if 2A00-2A03 contains read only 00 to 03 values.
  At some point transfer the control into the range 2A00-2FFF and die due the lack of code.
  There is no rom with these sequential values. Seems injected by the extra encrypted CPU.

*/

void goldnpkr_state::genie_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x17ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1fff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x3fff).rom();
}


void goldnpkr_state::mondial_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x4000, 0x7fff).bankr("bank1");
}

void goldnpkr_state::super21p_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0804, 0x0807).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0808, 0x080b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x2000).w("crtc", FUNC(mc6845_device::address_w));
	map(0x2001, 0x2001).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8000, 0xffff).rom();
}

void goldnpkr_state::icp_ext_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x3fff).rom();
	map(0x6000, 0x7fff).rom();
}

void goldnpkr_state::lespendu_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");

	map(0x5000, 0x5fff).bankr("bank0");
	map(0x6000, 0x7fff).rom();
}

void goldnpkr_state::op5cards_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x13ff).ram().w(FUNC(goldnpkr_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(goldnpkr_state::goldnpkr_colorram_w)).share("colorram");
	map(0x2000, 0x2000).portr("SW2");
	map(0xc000, 0xffff).rom();
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( goldnpkr )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                 PORT_NAME("Off (Payout)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("D-31") PORT_CODE(KEYCODE_E)     // O.A.R? (D-31 in schematics)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(3) PORT_NAME("Coupon (Note In)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("Weight (Coupon In)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("SW1:3")   // listed in the manual as "Play Mode"
	PORT_DIPSETTING(    0x40, "Manual" )            //  listed in the manual as "Out Play"
	PORT_DIPSETTING(    0x00, "Auto" )              //  listed in the manual as "Credit Play"
	PORT_DIPNAME( 0x80, 0x00, "Royal Flush" )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pmpoker )
	// Multiplexed - 4x5bits
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )                   PORT_NAME("Hold 1 / Take Score (Kasse)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )                   PORT_NAME("Hold 2 / Small (Tief)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )                   PORT_NAME("Hold 3 / Bet (Setze)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )                   PORT_NAME("Hold 4 / Big (Hoch)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )                   PORT_NAME("Hold 5 / Double Up (Dopp.)")
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
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
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
	// only bits 4-7 are connected here and were routed to SW1 1-4
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
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	// listed in the manual as "Play Mode"
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )            //  listed in the manual as "Out Play"
	PORT_DIPSETTING(    0x00, "Auto" )              //  listed in the manual as "Credit Play"
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( potnpkra )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )         PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
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
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Royal Flush Value" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "250 by bet" )
	PORT_DIPSETTING(    0x00, "500 by bet" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( animpkr )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )         PORT_IMPULSE(3) PORT_NAME("Coin 1 + Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("IN0-0 80") PORT_CODE(KEYCODE_G)

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("IN0-3 02") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Coin 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("IN0-3 10") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "High Pair (11-13)" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )             PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( potnpkrc )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( potnpkra )

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x10, 0x00, "Ace or Better" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ngold )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Coupon (Note In)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
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
	PORT_DIPNAME( 0x10, 0x00, "Minimal Hand" )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Pair of Aces" )
	PORT_DIPSETTING(    0x10, "Double Pair" )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	// listed in the manual as "Play Mode"
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )            //  listed in the manual as "Out Play"
	PORT_DIPSETTING(    0x00, "Auto" )              //  listed in the manual as "Credit Play"
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ngoldb )   // only coinage changes against ngold...
	// Multiplexed - 4x5bits
	PORT_INCLUDE( ngold )

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(3) PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In)")
INPUT_PORTS_END

static INPUT_PORTS_START( goodluck )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( goldnpkr )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

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
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
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
	// Multiplexed - 4x5bits
	PORT_INCLUDE( goldnpkr )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

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
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")   // OFF by default
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")   // OFF by default
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x04, "Percentage" )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "40%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x08, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")   // OFF by default
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")   // OFF by default
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")   // ON by default
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")   // ON by default
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x03, 0x01, "Max Bet" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "10" )    // OFF-OFF
	PORT_DIPSETTING(    0x02, "20" )    // ON-OFF
	PORT_DIPSETTING(    0x01, "50" )    // OFF-ON
	PORT_DIPSETTING(    0x00, "100" )   // ON-ON
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")   // no connected (OFF)
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")   // no connected (OFF)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Minimal Hand" )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Two Pairs" )
	PORT_DIPSETTING(    0x00, "High Pair" )
	PORT_DIPNAME( 0x20, 0x20, "Frequency" )         PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "50 Hz." )
	PORT_DIPSETTING(    0x00, "60 Hz." )
	PORT_DIPNAME( 0x40, 0x40, "Uncommented" )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "1 Credit" )
	PORT_DIPSETTING(    0x00, "10 Credits" )
	PORT_DIPNAME( 0x80, 0x00, "Royal Flush" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( witchcda )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                    PORT_NAME("Apuesta (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Contabilidad (Bookkeeping)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                   PORT_NAME("Doblar (Double Up)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Reparte (Deal)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                  PORT_NAME("Cancela (Cancel)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                 PORT_NAME("Pagar (Payout)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                   PORT_NAME("Cobrar (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Alta (Big)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Baja (Small)")
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
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Won Credits Counter" )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4")
	// Note In is always: 1 Note - 10 Credits
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
	// Multiplexed - 4x5bits
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Won Credits Counter" )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4")
	// Note In is always: 1 Note - 10 Credits
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
	// Multiplexed - 4x5bits
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
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
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	// even when the following one is forced to OFF,
	// turned ON behaves like "Jacks and Better"
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

static INPUT_PORTS_START( witchjol )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN0-1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN0-1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN0-1-8")

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Take")      PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Small")     PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Bet")       PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Small")     PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Double Up") PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
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
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
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
	PORT_DIPNAME( 0x03, 0x00, "Max Bet" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "10" )    // OFF-OFF
	PORT_DIPSETTING(    0x02, "20" )    // ON-OFF
	PORT_DIPSETTING(    0x01, "50" )    // OFF-ON
	PORT_DIPSETTING(    0x00, "100" )   // ON-ON
	PORT_DIPNAME( 0x04, 0x04, "Game Type" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Jolli Witch" )
	PORT_DIPSETTING(    0x00, "Witch Card" )
	PORT_DIPNAME( 0x08, 0x08, "Control Type" )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "6-Button" )
	PORT_DIPSETTING(    0x08, "12-Button" )
	PORT_DIPNAME( 0x10, 0x00, "Minimal Hand" )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Two Pairs" )
	PORT_DIPSETTING(    0x00, "High Pair" )
	PORT_DIPNAME( 0x60, 0x20, "Uncommented 1" )     PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, "1 DM - 1 PKT" )
	PORT_DIPSETTING(    0x20, "1 DM - 10 PKT" )
	PORT_DIPNAME( 0x80, 0x00, "Royal Flush" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( witchcdf )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( witchcrd )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
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
	// only bits 4-7 are connected here and were routed to SW1 1-4
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

static INPUT_PORTS_START( wldwitch )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                    PORT_NAME("Bet (Setzen)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Bookkeeping / Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                   PORT_NAME("Double Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal (Geben)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Manual Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                   PORT_NAME("Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )                                 PORT_CONDITION("SW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Take")      PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Small")     PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Bet")       PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Small")     PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Double Up") PORT_CONDITION("SW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("Weight (Coupon In)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
/*       _______________________
        | 40% | 50% | 60% | 70% |
 _______________________________
| PIN 3 | off | off | on  | on  |
 _______________________________
| PIN 4 | off | on  | off | on  |
 _______________________________

 _______________________________
| PIN 1 | on = Wirteb.          |
 _______________________________
| PIN 2 | off = Tab. 500        |
|       |  on = Tab. 1100       |
 _______________________________
| PIN 5,6,7,8 = off             |
 _______________________________

*/
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")   // OFF by default
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")   // OFF by default
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x04, "Percentage" )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "40%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x08, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")   // OFF by default
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")   // OFF by default
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")   // OFF by default
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")   // OFF by default
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
/*  DIP Switches (as shown in the epoxy block)

    Schalter      1   2   3   4   5   6   7   8
    --------------------------------------------
    Bet >  10    OFF OFF
    Bet >  20    ON  OFF
    Bet >  50    OFF ON
    Bet > 100    ON  ON
    --------------------------------------------
    Wild Witch           OFF
    Witch Game           ON
    --------------------------------------------
    12 Taster                OFF
     6 Taster                ON
    --------------------------------------------
    2 Paar                       OFF
    Hohes Paar                   ON
    --------------------------------------------
    64er                             OFF
    128er                            ON
    --------------------------------------------
    10 Credit                            OFF
    1 Credit                             ON
    --------------------------------------------
    R.Flush NO                               OFF
    R.Flush YES                              ON
    --------------------------------------------
*/
	PORT_DIPNAME( 0x03, 0x01, "Max Bet" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "10" )    // OFF-OFF
	PORT_DIPSETTING(    0x02, "20" )    // ON-OFF
	PORT_DIPSETTING(    0x01, "50" )    // OFF-ON
	PORT_DIPSETTING(    0x00, "100" )   // ON-ON
	PORT_DIPNAME( 0x04, 0x04, "Game Type" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Wild Witch" )
	PORT_DIPSETTING(    0x00, "Witch Game" )
	PORT_DIPNAME( 0x08, 0x08, "Control Type" )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "6-Button" )
	PORT_DIPSETTING(    0x08, "12-Button" )
	PORT_DIPNAME( 0x10, 0x00, "Minimal Hand" )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Two Pairs" )
	PORT_DIPSETTING(    0x00, "High Pair" )
	PORT_DIPNAME( 0x20, 0x20, "Uncommented 1" )     PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "64er" )
	PORT_DIPSETTING(    0x00, "128er" )
	PORT_DIPNAME( 0x40, 0x40, "Uncommented 2" )     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "1 Credit" )
	PORT_DIPSETTING(    0x00, "10 Credits" )
	PORT_DIPNAME( 0x80, 0x00, "Royal Flush" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wupndown )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet (Setzen)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )    PORT_NAME("Bookkeeping / Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Deal (Geben)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )    PORT_NAME("Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Take")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Double-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )                   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin 1 In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(3) PORT_NAME("Coin 2 In")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
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

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Minimal Hand" )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Two Pairs" )
	PORT_DIPSETTING(    0x00, "High Pair" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wstrike )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )    PORT_NAME("Bet (Setzen)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   PORT_NAME("Bookkeeping / Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )   PORT_NAME("Double Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_NAME("Deal (Geben)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
//  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )   PORT_NAME("Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )    PORT_NAME("Small")
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_F2)
//  PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )                   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )                   PORT_NAME("Weight (Coupon In)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
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

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Title" )             PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Witch Game" )
	PORT_DIPSETTING(    0x00, "Witch Strike" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wtchjack )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )    PORT_NAME("Bet (Setzen)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   PORT_NAME("Bookkeeping / Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )   PORT_NAME("Double Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_NAME("Deal (Geben)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )   PORT_NAME("Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )    PORT_NAME("Small")
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )                   PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )                   PORT_NAME("Weight (Coupon In)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
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

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Minimal Hand" )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Two Pairs" )
	PORT_DIPSETTING(    0x00, "High Pair" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sloco93 )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( witchcda )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Negro (Black)") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Rojo (Red)")    PORT_CODE(KEYCODE_S)

	PORT_MODIFY("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "32 Simple" )             PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Won Credits Counter" )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, "1 Coin - 10 Credit / 1 Note - 120 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin - 100 Credits / 1 Note - 100 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START( bsuerte )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                    PORT_NAME("Apostar (Bet)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Contabilidad (Meters)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                   PORT_NAME("Doblar (Double Up)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Dar/Virar (Deal/Draw)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                  PORT_NAME("Cancelar (Cancel)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                 PORT_NAME("Pagar (Payout)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                   PORT_NAME("Cobrar (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Mayor (Big)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Menor (Small)")
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )                 PORT_NAME("Billetes (Note In)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fichas (Coin In)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )                 PORT_NAME("Cupones (Coupon In)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Par Simple" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "50hz/60hz" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x00, "Modo de Pago" )      PORT_DIPLOCATION("SW1:3")   // left as 'auto'
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
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
	// Multiplexed - 4x5bits
	PORT_INCLUDE( bsuerte )

	PORT_MODIFY("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_DIPNAME( 0x20, 0x00, "Creditos Ganados" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Show" )
	PORT_DIPSETTING(    0x00, "Hide" )
INPUT_PORTS_END

static INPUT_PORTS_START( poker91 )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( bsuerte )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Negro (Black)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Rojo (Red)")

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Switch Card 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Switch Card 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Switch Card 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Switch Card 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Switch Card 5")

	PORT_MODIFY("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_DIPNAME( 0x20, 0x20, "Contador de Acumulados" )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "Muestra" )
	PORT_DIPSETTING(    0x00, "Oculta" )
INPUT_PORTS_END

static INPUT_PORTS_START( wildcard )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )  PORT_NAME("Meters/Settings")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )  PORT_NAME("Double-Up/Next")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Deal/Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )  PORT_NAME("Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("High/Red")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Low/Black")
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Display Paytable" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Double-Up Type" )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "High/Low" )
	PORT_DIPSETTING(    0x00, "Red/Black" )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, "Hold" )
	PORT_DIPSETTING(    0x00, "Discard" )

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

static INPUT_PORTS_START( genie )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   PORT_NAME("Bookkeeping")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_NAME("Play")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Collect")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )                   PORT_NAME("Coupon (Note In)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Coin In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Min Wining Hand" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, "Double Pair" )
	PORT_DIPSETTING(    0x00, "Pair of 11's" )
	PORT_DIPNAME( 0x20, 0x20, "50hz/60hz" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( caspoker )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Bookkeeping")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )                   PORT_NAME("Hold 1 / Take Score (Kasse)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )                   PORT_NAME("Hold 2 / Small (Tief)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )                   PORT_NAME("Hold 3 / Bet (Setzen)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )                   PORT_NAME("Hold 4 / Big (Hoch)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )                   PORT_NAME("Hold 5 / Double Up (Doppeln)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 )   PORT_IMPULSE(3)
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

static INPUT_PORTS_START( mondial )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* the following one is connected to DIP switches and is meant
	for switch between different programs stored in different
	halves of the program ROM */
	PORT_START("SELDSW")
	PORT_DIPNAME( 0x01, 0x00, "Game Selector" )
	PORT_DIPSETTING(    0x00, "Game 1 (Italian" )
	PORT_DIPSETTING(    0x01, "Game 2 (French)" )
INPUT_PORTS_END

static INPUT_PORTS_START( videtron )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )                       PORT_NAME("Card Selector") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )                       PORT_NAME("Hold Card")     PORT_CODE(KEYCODE_X)

	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Settings") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )      PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )      PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )      PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Jacks or Better" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( super98 )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( bsuerte )

	PORT_MODIFY("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Attendant Key") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Supervisor Key") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                               // Key '3'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Settings")  // Key '2'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                              // Key 'N'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3) PORT_NAME("Note In")      // Key '5'
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                                       // Key '4'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Black")              // Key 'A'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Small / Red")              // Key 'S'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  // Key 'Z'
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  // Key 'X'
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  // Key 'C'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  // Key 'V'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  // Key 'B'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Unknown IN0-3 0x01") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Unknown IN0-3 0x02") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Unknown IN0-3 0x04") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                                        // Key 'M'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("Unknown IN0-3 0x10") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Hand Games" )
	PORT_DIPSETTING(    0x10, "2 Hand Games" )
	PORT_DIPSETTING(    0x00, "3 Hand Games" )
	PORT_DIPNAME( 0x20, 0x00, "Turbo" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Bonus" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Royal Flush" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( geniea )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( bsuerte )

	PORT_MODIFY("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )       PORT_IMPULSE(3) PORT_NAME("Coin In")      // Key '5'
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Attendant Key") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                               // Key '3'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Settings")  // Key '2'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                              // Key 'N'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("Unknown IN0-1 0x01") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )     PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                                       // Key '4'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Black")              // Key 'A'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Small / Red")              // Key 'S'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  // Key 'Z'
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  // Key 'X'
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  // Key 'C'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  // Key 'V'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  // Key 'B'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("Supervisor Key") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("Unknown IN0-3 0x02") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )      PORT_IMPULSE(3) PORT_NAME("Note In")     // Key '6'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                                          // Key 'M'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("Unknown IN0-3 0x10") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, "Frequency" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, "50Hz." )
	PORT_DIPSETTING(    0x00, "60Hz." )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Payment" )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bsuertev )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( bsuerte )

	PORT_MODIFY("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )       PORT_IMPULSE(3) PORT_NAME("Coin In")      // Key '5'
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_NAME("Attendant Key") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )                               // Key '3'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Settings")  // Key '2'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                              // Key 'N'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("Unknown IN0-1 0x01") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )     PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )                                       // Key '4'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Black")              // Key 'A'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )  PORT_NAME("Small / Red")              // Key 'S'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  // Key 'Z'
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  // Key 'X'
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  // Key 'C'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  // Key 'V'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  // Key 'B'
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("Supervisor Key") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("Unknown IN0-3 0x02") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )      PORT_IMPULSE(3) PORT_NAME("Note In")     // Key '6'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                                          // Key 'M'
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("Unknown IN0-3 0x10") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, "Par Simple" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bonuspkr )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                   PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )                   PORT_NAME("Deal / Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_IMPULSE(3) PORT_NAME("Out (Manual Collect)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                 PORT_NAME("Off (Payout)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )                   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )                    PORT_NAME("Small")
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("IN0-3 02")   PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Pair of Aces" )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "50hz/60hz" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, "50hz" )
	PORT_DIPSETTING(    0x00, "60hz" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( super21p )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )  PORT_NAME("Meters")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )  PORT_NAME("Hit")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Not Use")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )       PORT_NAME("Not Use") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Pay")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )   PORT_NAME("Take Score")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )   PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )    PORT_NAME("Small")
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )      PORT_NAME("Not Use") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )      PORT_NAME("C.A.R.")  PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x03, "Limit" )        PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, "3000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x04, 0x04, "4 Of a Kind" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x08, "Credit Limit" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x10, 0x10, "Cr. Limit" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x20, 0x20, "W. Point" )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x40, 0x40, "Fever Chance" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "Play" )
	PORT_DIPSETTING(    0x00, "Port Check" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin1-Coin2-NoteIn" ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x07, "10-100-50" )
	PORT_DIPSETTING(    0x06, "5-50-25" )
	PORT_DIPSETTING(    0x05, "4-40-20" )
	PORT_DIPSETTING(    0x04, "3-30-15" )
	PORT_DIPSETTING(    0x03, "2-20-10" )
	PORT_DIPSETTING(    0x02, "1-10-5" )
	PORT_DIPSETTING(    0x01, "1/2 - 6 - 3" )
	PORT_DIPSETTING(    0x00, "1/5 - 2 - 1" )
	PORT_DIPNAME( 0x08, 0x08, "PayOut" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, "Direct" )
	PORT_DIPSETTING(    0x00, "Credit" )
	PORT_DIPNAME( 0x10, 0x10, "Win-Up Win" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x20, 0x20, "Duty" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, "Weak" )
	PORT_DIPSETTING(    0x00, "Strong" )
	PORT_DIPNAME( 0xc0, 0xc0, "Game Win" )     PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0xc0, "90%" )
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0x40, "70%" )
	PORT_DIPSETTING(    0x00, "60%" )
INPUT_PORTS_END


static INPUT_PORTS_START( op5cards )
	// Multiplexed - 4x5bits
	PORT_INCLUDE( super21p )

	PORT_MODIFY("IN0-0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )

	PORT_MODIFY("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_MODIFY("IN0-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Note In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Royal Flush" )       PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x04, "Lose" )
	PORT_DIPSETTING(    0x00, "OK" )
	PORT_DIPNAME( 0x08, 0x00, "Credit Max" )        PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "Disable" )
	PORT_DIPSETTING(    0x08, "Enable" )
	PORT_DIPNAME( 0x10, 0x00, "Credit Max Amount" ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x10, "200" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Double Up" )         PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x40, "90%" )
	PORT_DIPNAME( 0x80, 0x80, "Port Check" )        PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("SW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:!6,!7,!8")  // coin2 = (coin1 x5); note = (coin1 x10)
	PORT_DIPSETTING(    0x00, "Coin1 = 5C-1C;   Coin2 = 1C-1C;   Note = 1C-2C" )
	PORT_DIPSETTING(    0x01, "Coin1 = 2C-1C;   Coin2 = 1C-3C;   Note = 1C-6C" )
	PORT_DIPSETTING(    0x02, "Coin1 = 1C-1C;   Coin2 = 1C-5C;   Note = 1C-10C" )
	PORT_DIPSETTING(    0x03, "Coin1 = 1C-2C;   Coin2 = 1C-10C;  Note = 1C-20C" )
	PORT_DIPSETTING(    0x04, "Coin1 = 1C-3C;   Coin2 = 1C-15C;  Note = 1C-30C" )
	PORT_DIPSETTING(    0x05, "Coin1 = 1C-4C;   Coin2 = 1C-20C;  Note = 1C-40C" )
	PORT_DIPSETTING(    0x06, "Coin1 = 1C-5C;   Coin2 = 1C-25C;  Note = 1C-50C" )
	PORT_DIPSETTING(    0x07, "Coin1 = 1C-10C;  Coin2 = 1C-50C;  Note = 1C-100C" )
	PORT_DIPNAME( 0x08, 0x00, "Bet Max" )           PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Game Duty" )         PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(    0x20, "Weak" )
	PORT_DIPSETTING(    0x00, "Strong" )
	PORT_DIPNAME( 0xc0, 0xc0, "Pay Rate" )          PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x40, "70%" )
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0xc0, "90%" )
INPUT_PORTS_END


static INPUT_PORTS_START(lespendu)
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_NAME("Stats / Meters")                PORT_CODE(KEYCODE_0)  // stats
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 )  PORT_NAME("Button 4 / Stats Input Test")   PORT_CODE(KEYCODE_V)  // button 4 / stats test mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_NAME("Button 1")                      PORT_CODE(KEYCODE_Z)  // button 1
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 )  PORT_NAME("Button 5 / Stats Exit")         PORT_CODE(KEYCODE_B)  // button 5
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_NAME("Button 2")                      PORT_CODE(KEYCODE_X)  // button 2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )  PORT_NAME("Button 3")                      PORT_CODE(KEYCODE_C)  // button 3
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )    // 25¢ coin, 50¢ to play.
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


/*********************************************
*              Graphics Layouts              *
*********************************************/

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

static const gfx_layout fixedtilelayout =
{
	8, 8,
	0x100,
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_goldnpkr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_wcrdxtnd )
	GFXDECODE_ENTRY( "gfx0", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx6", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx7", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx8", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx9", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx10", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx11", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx12", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx13", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx14", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx15", 0, tilelayout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_super21p )
	GFXDECODE_ENTRY( "gfx1", 0, fixedtilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, fixedtilelayout, 0x100, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, fixedtilelayout, 0x200, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, fixedtilelayout, 0x300, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_op5cards )
	GFXDECODE_ENTRY( "gfx1", 0, fixedtilelayout, 8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, fixedtilelayout, 8, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, fixedtilelayout, 8, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, fixedtilelayout, 8, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_caspoker )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 128, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 128, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_lespendu )
	GFXDECODE_ENTRY( "gfx1", 0, fixedtilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, fixedtilelayout, 0, 16 )
GFXDECODE_END


/**********************************************************
*                 Discrete Sound Routines                 *
***********************************************************

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
	3,                                  // size of ladder
	{RES_K(30), RES_K(15), RES_K(7.5)}, // elements

/*  external vBias doesn't seems to be accurate.
    using the 555 internal values sound better.
*/
	5,                                  // voltage Bias resistor is tied to
	RES_K(5),                           // additional resistor tied to vBias
	RES_K(10),                          // resistor tied to ground

	CAP_U(4.7)                          // filtering cap tied to ground
};

static DISCRETE_SOUND_START( goldnpkr_discrete )
/*
    - bits -
    76543210
    --------
    .....xxx --> sound data.
    ....x... --> enable/disable.

*/
	DISCRETE_INPUT_NOT   (NODE_01)      // bit 3: enable/disable
	DISCRETE_INPUT_DATA  (NODE_10)      // bits 0-2: sound data

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
	3,                                  // size of ladder
	{RES_K(33), RES_K(18), RES_K(10)},  // elements

/*  external vBias doesn't seems to be accurate.
    using the 555 internal values sound better.
*/
	5,                                  // voltage Bias resistor is tied to
	RES_K(5),                           // additional resistor tied to vBias
	RES_K(10),                          // resistor tied to ground

	0                                   // no filtering cap tied to ground
};

static DISCRETE_SOUND_START( pottnpkr_discrete )
/*
    - bits -
    76543210
    --------
    .....xxx --> sound data.
    ....x... --> enable/disable.

*/
	DISCRETE_INPUT_NOT   (NODE_01)      // bit 3: enable/disable
	DISCRETE_INPUT_DATA  (NODE_10)      // bits 0-2: sound data

	DISCRETE_DAC_R1(NODE_20, NODE_10, 5, &dac_pottnpkr_ladder)

	DISCRETE_555_ASTABLE_CV(NODE_30, NODE_01, RES_K(1), RES_K(1), CAP_U(1), NODE_20, &goldnpkr_555_vco_desc)
	DISCRETE_OUTPUT(NODE_30, 3000)

DISCRETE_SOUND_END


/******************************************
*          Machine Start & Reset          *
******************************************/

MACHINE_START_MEMBER(goldnpkr_state, mondial)
{
	m_lamps.resolve();

	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0], 0x4000);
}

MACHINE_RESET_MEMBER(goldnpkr_state, mondial)
{
	uint8_t seldsw = (ioport("SELDSW")->read() );
	popmessage("ROM Bank: %02X", seldsw);

	membank("bank1")->set_entry(seldsw);
}

MACHINE_START_MEMBER(goldnpkr_state, lespendu)
{
	m_lamps.resolve();

	uint8_t *ROM = memregion("data")->base();
	membank("bank0")->configure_entries(0, 8, &ROM[0], 0x1000);
}

MACHINE_RESET_MEMBER(goldnpkr_state, lespendu)
{
	membank("bank0")->set_entry(7);
}


/*********************************************
*              Machine Drivers               *
*********************************************/

void goldnpkr_state::goldnpkr_base(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::goldnpkr_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::goldnpkr_mux_port_r));
	m_pia[0]->writepb_handler().set(FUNC(goldnpkr_state::lamps_a_w));

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("SW1");
	m_pia[1]->writepa_handler().set(FUNC(goldnpkr_state::sound_w));
	m_pia[1]->writepb_handler().set(FUNC(goldnpkr_state::mux_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, (39 + 1) * 8, 0, 32 * 8, ((31 + 1) * 8) + 4, 0, 29 * 8); // from MC6845 parameters
	m_screen->set_screen_update(FUNC(goldnpkr_state::screen_update_goldnpkr));

	mc6845_device &crtc(MC6845(config, "crtc", CPU_CLOCK)); // 68B45 or 6845s @ CPU clock
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goldnpkr);
	PALETTE(config, m_palette, FUNC(goldnpkr_state::goldnpkr_palette), 256);
}

void goldnpkr_state::goldnpkr(machine_config &config)
{
	goldnpkr_base(config);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::pottnpkr(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::pottnpkr_map);

	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, pottnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::icp_ext(machine_config &config)
{
	goldnpkr_base(config);
	R65C02(config.replace(), m_maincpu, CPU_CLOCK);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::icp_ext_map);

	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, pottnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::witchcrd(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::witchcrd_map);

	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));

	// video hardware
	m_screen->set_raw(PIXEL_CLOCK, (39 + 1) * 8, 0, 32 * 8, (38 + 1) * 8, 0, 32 * 8);
	m_palette->set_init(FUNC(goldnpkr_state::witchcrd_palette));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, "discrete", goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::witchcdj(machine_config &config)
{
	witchcrd(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::witchcdj_map);
}

void goldnpkr_state::wcfalcon(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::witchcrd_falcon_map);

	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));
	m_pia[0]->writepb_handler().set(FUNC(goldnpkr_state::ay8910_control_w));
	m_pia[1]->readpa_handler().set(FUNC(goldnpkr_state::ay8910_data_r));
	m_pia[1]->writepa_handler().set(FUNC(goldnpkr_state::ay8910_data_w));

	// video hardware
	m_palette->set_init(FUNC(goldnpkr_state::witchcrd_palette));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay8910, MASTER_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 1.00);    // guess, seems ok
}

void goldnpkr_state::super21p(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::super21p_map);

	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));
	m_pia[0]->writepb_handler().set(FUNC(goldnpkr_state::ay8910_control_w));
	m_pia[1]->readpa_handler().set(FUNC(goldnpkr_state::ay8910_data_r));
	m_pia[1]->writepa_handler().set(FUNC(goldnpkr_state::ay8910_data_w));

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(goldnpkr_state, super21p)
	m_gfxdecode->set_info(gfx_super21p);

	PALETTE(config.replace(), m_palette, FUNC(goldnpkr_state::super21p_palette), 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay8910, MASTER_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 1.00);  // guess, seems ok
	m_ay8910->port_a_read_callback().set_ioport("SW1");
	m_ay8910->port_b_read_callback().set_ioport("SW2");
}

void goldnpkr_state::op5cards(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::op5cards_map);

	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));
	m_pia[0]->writepb_handler().set(FUNC(goldnpkr_state::ay8910_control_w));
	m_pia[1]->readpa_handler().set(FUNC(goldnpkr_state::ay8910_data_r));
	m_pia[1]->writepa_handler().set(FUNC(goldnpkr_state::ay8910_data_w));

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(goldnpkr_state, super21p)
	m_gfxdecode->set_info(gfx_op5cards);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay8910, MASTER_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 1.00);  // guess, seems ok
	m_ay8910->port_a_read_callback().set_ioport("SW1");
	m_ay8910->port_b_read_callback().set_ioport("SW2");
}


void goldnpkr_state::wildcard(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::wildcard_map);

	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));

	// video hardware
//  m_gfxdecode->set_info(gfx_wildcard);
	m_palette->set_init(FUNC(goldnpkr_state::witchcrd_palette));
//  MCFG_VIDEO_START_OVERRIDE(goldnpkr_state,wildcard)

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::wcrdxtnd(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::wcrdxtnd_map);

	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));

	// video hardware
	m_screen->set_raw(PIXEL_CLOCK, (39 + 1) * 8, 0, 32 * 8, (38 + 1) * 8, 0, 32 * 8);
	m_gfxdecode->set_info(gfx_wcrdxtnd);
	m_palette->set_init(FUNC(goldnpkr_state::wcrdxtnd_palette));
	MCFG_VIDEO_START_OVERRIDE(goldnpkr_state, wcrdxtnd)

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::wildcrdb(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::wildcrdb_map);

	sega_315_5018_device &mcu(SEGA_315_5018(config, "mcu", MASTER_CLOCK / 8));    // guess
	mcu.set_addrmap(AS_PROGRAM, &goldnpkr_state::wildcrdb_mcu_map);
	mcu.set_addrmap(AS_IO, &goldnpkr_state::wildcrdb_mcu_io_map);
	mcu.set_addrmap(AS_OPCODES, &goldnpkr_state::wildcrdb_mcu_decrypted_opcodes_map);
	mcu.set_decrypted_tag(":decrypted_opcodes");
	mcu.set_size(0x1000);
	mcu.set_vblank_int("screen", FUNC(goldnpkr_state::irq0_line_hold));

	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));
	m_pia[0]->writepb_handler().set(FUNC(goldnpkr_state::ay8910_control_w));
	m_pia[1]->readpa_handler().set(FUNC(goldnpkr_state::ay8910_data_r));
	m_pia[1]->writepa_handler().set(FUNC(goldnpkr_state::ay8910_data_w));

	// video hardware
//  m_gfxdecode->set_info(gfx_wildcard);
	m_palette->set_init(FUNC(goldnpkr_state::witchcrd_palette));
//  MCFG_VIDEO_START_OVERRIDE(goldnpkr_state,wildcard)

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay8910, MASTER_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 1.00);    // guess, seems ok
}

void goldnpkr_state::genie(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::genie_map);

	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
	m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));

	// video hardware
	m_palette->set_init(FUNC(goldnpkr_state::witchcrd_palette));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::geniea(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::goldnpkr_map);

//  m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
//  m_pia[0]->writepa_handler().set(FUNC(goldnpkr_state::mux_port_w));

	// video hardware
	m_palette->set_init(FUNC(goldnpkr_state::witchcrd_palette));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void goldnpkr_state::mondial(machine_config &config)
{
	goldnpkr_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::mondial_map);

	MCFG_MACHINE_START_OVERRIDE(goldnpkr_state, mondial)
	MCFG_MACHINE_RESET_OVERRIDE(goldnpkr_state, mondial)

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void goldnpkr_state::caspoker(machine_config &config)
{
	goldnpkr_base(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::goldnpkr_map);

	m_gfxdecode->set_info(gfx_caspoker);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void goldnpkr_state::gldnirq0(machine_config &config)
{
	goldnpkr(config);

	// basic machine hardware
	mc6845_device &crtc(MC6845(config.replace(), "crtc", CPU_CLOCK)); // 68B45 or 6845s @ CPU clock
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}


void goldnpkr_state::lespendu(machine_config &config)
{
	goldnpkr(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &goldnpkr_state::lespendu_map);

	m_pia[0]->writepb_handler().set(FUNC(goldnpkr_state::lespendu_lamps_a_w));

	m_pia[1]->readpa_handler().set_ioport("SW1");
	m_pia[1]->readpb_handler().set_ioport("SW2");
	m_pia[1]->writepb_handler().set(FUNC(goldnpkr_state::lespendu_mux_w)); // ++ bankswitch

	MCFG_MACHINE_START_OVERRIDE(goldnpkr_state, lespendu)
	MCFG_MACHINE_RESET_OVERRIDE(goldnpkr_state, lespendu)

	// video hardware
	m_gfxdecode->set_info(gfx_lespendu);

	mc6845_device &crtc(HD6845S(config.replace(), "crtc", CPU_CLOCK)); // Hitachi HD46505SP
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}


/*********************************************
*                Blitz System                *
*********************************************/

uint8_t blitz_state::cpubank_decrypt_r(offs_t offset)
{
	return m_cpubank[offset] ^ m_cpubank_xor;
}

void blitz_state::mcu_command_w(uint8_t data)
{
	m_mcu->pa_w(data);
	if (BIT(m_portc_data, 0))
	{
		m_mcu->set_input_line(M6805_IRQ_LINE, ASSERT_LINE);
		m_maincpu->suspend(SUSPEND_REASON_HALT, true);
	}
}

void blitz_state::mcu_portb_w(uint8_t data)
{
	m_cpubank_xor = data;
}

void blitz_state::mcu_portc_w(uint8_t data)
{
	if (!BIT(data, 0))
	{
		m_mcu->set_input_line(M6805_IRQ_LINE, CLEAR_LINE);
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}

	m_bankdev->set_bank((BIT(data, 2) << 1) | BIT(data, 3));

	m_portc_data = data;
}


void blitz_state::megadpkr_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); //.share("nvram");   // battery backed RAM
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x0847).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0848, 0x084b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));

/*  There is another set of PIAs controlled by the code.
    Maybe they are just mirrors...

    map(0x10f4, 0x10f7).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
    map(0x10f8, 0x10fb).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
*/
	map(0x1000, 0x13ff).ram().w(FUNC(blitz_state::goldnpkr_videoram_w)).share("videoram");
	map(0x1800, 0x1bff).ram().w(FUNC(blitz_state::goldnpkr_colorram_w)).share("colorram");

	map(0x4000, 0x7fff).rw(m_bankdev, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0x8000, 0xbfff).nopr().w(FUNC(blitz_state::mcu_command_w));
	map(0xc000, 0xffff).rom();
}

void blitz_state::megadpkr_banked_map(address_map &map)
{
	map(0x00000, 0x07fff).r(FUNC(blitz_state::cpubank_decrypt_r));
	map(0x08000, 0x087ff).rw("timekpr", FUNC(m48t02_device::read), FUNC(m48t02_device::write));
}


static INPUT_PORTS_START( megadpkr )
	// Multiplexed - 4x5bits
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // not used?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Coins Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Menu")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // not used?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Credit")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coupon")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1")
	// only bits 4-7 are connected here and were routed to SW1 1-4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
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


void blitz_state::megadpkr(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &blitz_state::megadpkr_map);

	ADDRESS_MAP_BANK(config, "bankdev").set_map(&blitz_state::megadpkr_banked_map).set_data_width(8).set_addr_width(16).set_stride(0x4000);

	M68705P5(config, m_mcu, CPU_CLOCK); // unknown
	m_mcu->portb_w().set(FUNC(blitz_state::mcu_portb_w));
	m_mcu->portc_w().set(FUNC(blitz_state::mcu_portc_w));

	M48T02(config, "timekpr", 0);

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(goldnpkr_state::pottnpkr_mux_port_r));
	m_pia[0]->writepb_handler().set(FUNC(goldnpkr_state::lamps_a_w));

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("SW1");
	m_pia[1]->writepa_handler().set(FUNC(goldnpkr_state::sound_w));
	m_pia[1]->writepb_handler().set(FUNC(goldnpkr_state::mux_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, (37 + 1) * 8, 0, 32 * 8, (38 + 1) * 8, 0, 32 * 8); // from MC6845 parameters
	m_screen->set_screen_update(FUNC(goldnpkr_state::screen_update_goldnpkr));

	mc6845_device &crtc(MC6845(config, "crtc", CPU_CLOCK));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, 0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goldnpkr);
	PALETTE(config, m_palette, FUNC(blitz_state::witchcrd_palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, goldnpkr_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

/******************************* GOLDEN POKER SETS *******************************/

/*  the original goldnpkr u40_4a.bin rom is bit corrupted.
    U43_2A.bin        BADADDR      --xxxxxxxxxxx
    U38_5A.bin        1ST AND 2ND HALF IDENTICAL
    UPS39_12A.bin     0xxxxxxxxxxxxxx = 0xFF

    pmpoker                 goldnpkr
    1-4.bin                 u38_5a (1st quarter)    96.582031%  \ 1st and 2nd halves are identical.
    1-3.bin                 u38_5a (2nd quarter)    IDENTICAL   /
    1-1.bin                 u43_2a (1st quarter)    IDENTICAL   ; 4 quarters are identical.
*/
ROM_START( goldnpkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ups39_12a.bin",  0x0000, 0x8000, CRC(216b45fb) SHA1(fbfcd98cc39b2e791cceb845b166ff697f584add) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38_5a.bin", 0x4000, 0x2000, CRC(32705e1d) SHA1(84f9305af38179985e0224ae2ea54c01dfef6e12) )    // char ROM

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "u43_2a.bin", 0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "u40_4a.bin", 0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x4800, 0x4000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",       0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*  pmpoker                 goldnpkb
    1-4.bin                 u38.5a (1st quarter)    96.582031%  \ 1st and 2nd halves are identical.
    1-3.bin                 u38.5a (2nd quarter)    IDENTICAL   /
    1-1.bin                 u43.2a (1st quarter)    IDENTICAL   ; 4 quarters are identical.
*/
ROM_START( goldnpkb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ups31h.12a", 0x0000, 0x8000, CRC(bee5b07a) SHA1(5da60292ecbbedd963c273eac2a1fb88ad66ada8) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38_5a.bin", 0x4000, 0x2000, CRC(32705e1d) SHA1(84f9305af38179985e0224ae2ea54c01dfef6e12) )    // char ROM

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "u43_2a.bin", 0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "u40_4a.bin", 0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x4800, 0x4000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",       0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Unknown Golden Poker.

  Bootleg (maybe French) board.
  Program mapped at 0x5000-0x7fff
  GFX ROMs are missing.

*/
ROM_START( goldnpkc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732-1.bin",  0x5000, 0x1000, CRC(33a3b7c7) SHA1(b9713f534811963284d96239e4d8ab567adfb15a) )
	ROM_LOAD( "2732-2.bin",  0x6000, 0x1000, CRC(12f403ba) SHA1(e84c0bf235ff3a4b2d54141468e4867c49ea0bd7) )
	ROM_LOAD( "2732-3.bin",  0x7000, 0x1000, CRC(96a51764) SHA1(c175fadaa87a85af60619edfdb32e0ec7faf6682) )

	ROM_REGION( 0x6000, "gfx1", 0 )  // gfx roms borrowed from golden poker
	ROM_FILL(                 0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes.
	ROM_LOAD( "gfx-3.bin",    0x4000, 0x2000, BAD_DUMP CRC(32705e1d) SHA1(84f9305af38179985e0224ae2ea54c01dfef6e12) )    // char rom + cards deck gfx, bitplane 3.

	ROM_REGION( 0x6000, "gfx2", 0 )  // gfx roms borrowed from golden poker
	ROM_LOAD( "gfx-1.bin",    0x0000, 0x2000, BAD_DUMP CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    // cards deck gfx, bitplane 1.
	ROM_LOAD( "gfx-2.bin",    0x2000, 0x2000, BAD_DUMP CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    // cards deck gfx, bitplane 2.
	ROM_COPY( "gfx1", 0x4800, 0x4000, 0x0800 )    // cards deck gfx, bitplane 3. found in the 2nd quarter of the char rom.

	ROM_REGION( 0x0100, "proms", 0 )  // bipolar prom borrowed from golden poker
	ROM_LOAD( "82s129n.bin",  0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Unknown Golden Poker.
  Bio5 set.

  Maybe bootleg
  Running in original Bonanza board.

  Program mapped at 0x5000-0x7fff
  GFX ROMs are missing.

  Always get a winning Flush hand.
  (Seems protection. Need to analyze the code)

*/
ROM_START( goldnpkd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bio5_1-20_1.bin",  0x5000, 0x1000, CRC(d6612e28) SHA1(ec0e05035283642966f416d3361b94a74076a452) )
	ROM_LOAD( "bio5_1-20_2.bin",  0x6000, 0x1000, CRC(6b2ade97) SHA1(66adbe69f132f849c0a2a32d5a9575b0740c7a4c) )
	ROM_LOAD( "bio5_1-20_3.bin",  0x7000, 0x1000, CRC(d1ee95e2) SHA1(95ad7f86f83fda94476508954bda1270fb5f17ad) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(            0x0000, 0x1000, 0x00 )  // filling the R-G bitplanes.
	ROM_LOAD( "u38.5a",  0x1000, 0x0800, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )  // chars rom + cards deck gfx, bitplane 3.
	ROM_IGNORE(                  0x0800)         // discarding 2nd half (cards deck gfx).

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "n43.2a",  0x0000, 0x0800, CRC(1419298b) SHA1(9e07c94c858f055d1c4987efd03c76cce936f4da) )  // cards deck gfx, bitplane 1.
	ROM_IGNORE(                  0x0800)
	ROM_LOAD( "n40.4a",  0x0800, 0x0800, CRC(e0b96dcf) SHA1(b06af94361dd951573f187df575b31a9ada0c3e9) )  // cards deck gfx, bitplane 2.
	ROM_IGNORE(                  0x0800)
	ROM_LOAD( "u38.5a",  0x1000, 0x0800, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )  // chars rom + cards deck gfx, bitplane 3.
	ROM_CONTINUE(        0x1000, 0x0800)         // discarding 1nd half (chars).

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.7d",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Unknown Golden Poker.
  G set. Alternate HI-LO game, french text in copyright.

  Bootleg from Intercoast
  Running in original Bonanza board.

  Program mapped at 0x5000-0x7fff
  GFX ROMs are missing.

  The main difference is the way to play the HI-LO game:

  1- When the player wins a hand, game will automatically switch to HI-LO
     for double-up (no need to press the "double" button)

  2- A card is shown to the player, then the game is to guess the drawn card
    (BIG to guess bigger than actual card, LO to guess a lower card)

  3- The player can still collect without playing double-up by pressing the "TAKE" button

  Also,the "WIN" message looks to be misplaced on the screen (too low)
  Will show correctly if the system runs in 50hz.

  This set has the following sequence of coin2/credits in setup:
  1-6-4-5-8-10-20-50-100. This is odd...

*/
ROM_START( goldnpke )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g_1.bin",  0x5000, 0x1000, CRC(d475cd13) SHA1(7c12b44ab938f26701587e57784f08e248e3afd2) )
	ROM_LOAD( "g_2.bin",  0x6000, 0x1000, CRC(ce080d66) SHA1(c5e11f7dc52a4d1661661a06d39316ba6a944adc) )
	ROM_LOAD( "g_3.bin",  0x7000, 0x1000, CRC(9d02b6f4) SHA1(bd01477268543d0edb2cec2a26bab0627a6d3414) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(            0x0000, 0x1000, 0x00 )  // filling the R-G bitplanes.
	ROM_LOAD( "u38.5a",  0x1000, 0x0800, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )  // chars rom + cards deck gfx, bitplane 3.
	ROM_IGNORE(                  0x0800)         // discarding 2nd half (cards deck gfx).

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "n43.2a",  0x0000, 0x0800, CRC(1419298b) SHA1(9e07c94c858f055d1c4987efd03c76cce936f4da) )  // cards deck gfx, bitplane 1.
	ROM_IGNORE(                  0x0800)
	ROM_LOAD( "n40.4a",  0x0800, 0x0800, CRC(e0b96dcf) SHA1(b06af94361dd951573f187df575b31a9ada0c3e9) )  // cards deck gfx, bitplane 2.
	ROM_IGNORE(                  0x0800)
	ROM_LOAD( "u38.5a",  0x1000, 0x0800, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )  // chars rom + cards deck gfx, bitplane 3.
	ROM_CONTINUE(        0x1000, 0x0800)         // discarding 1nd half (chars).

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.7d",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Unknown Golden Poker.
  HL Alternate HI-LO game, french text in copyright

  Bootleg from Intercoast
  Running in original Bonanza board.

  Program mapped at 0x5000-0x7fff
  GFX ROMs are missing.

  The main difference is the way to play the HI-LO game:

  1- When the player wins a hand, game will automatically switch to HI-LO
     for double-up (no need to press the "double" button)

  2- A card is shown to the player, then the game is to guess the drawn card
    (BIG to guess bigger than actual card, LO to guess a lower card)

  3- The player can still collect without playing double-up by pressing the "TAKE" button

  Also,the "WIN" message looks to be misplaced on the screen (too low)
  Will show correctly if the system runs in 50hz.

  Coin2 and Coupon/Note are fixed in 1 credit and cannot be changed.

*/
ROM_START( goldnpkf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hl_1.bin",  0x5000, 0x1000, CRC(d475cd13) SHA1(7c12b44ab938f26701587e57784f08e248e3afd2) )
	ROM_LOAD( "hl_2.bin",  0x6000, 0x1000, CRC(304eb644) SHA1(c876e0d6121dee594c4f5d75273c74982c5bd524) )
	ROM_LOAD( "hl_3.bin",  0x7000, 0x1000, CRC(47c15f44) SHA1(da7af46a8d17abffd30fffe6eb091d15f9f8f92c) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(            0x0000, 0x1000, 0x00 )  // filling the R-G bitplanes.
	ROM_LOAD( "u38.5a",  0x1000, 0x0800, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )  // chars rom + cards deck gfx, bitplane 3.
	ROM_IGNORE(                  0x0800)         // discarding 2nd half (cards deck gfx).

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "n43.2a",  0x0000, 0x0800, CRC(1419298b) SHA1(9e07c94c858f055d1c4987efd03c76cce936f4da) )  // cards deck gfx, bitplane 1.
	ROM_IGNORE(                  0x0800)
	ROM_LOAD( "n40.4a",  0x0800, 0x0800, CRC(e0b96dcf) SHA1(b06af94361dd951573f187df575b31a9ada0c3e9) )  // cards deck gfx, bitplane 2.
	ROM_IGNORE(                  0x0800)
	ROM_LOAD( "u38.5a",  0x1000, 0x0800, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )  // chars rom + cards deck gfx, bitplane 3.
	ROM_CONTINUE(        0x1000, 0x0800)         // discarding 1nd half (chars).

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.7d",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/*  Videotron Poker.
    Alternative controls set, with cards selector...
*/
ROM_START( videtron )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4.bin", 0x4000, 0x2000, CRC(0f00f87d) SHA1(3cd061463b0ed52cef88900f1d4511708588bfac) )
	ROM_LOAD( "5.bin", 0x6000, 0x2000, CRC(395fbc5c) SHA1(f742d7a9312828997a4323ac2b957048687fbed2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(          0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "3.bin", 0x2000, 0x0800, CRC(23e83e89) SHA1(0c6352d46e3dfe176b0e970dd163e2bc01246890) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "0.bin", 0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    // cards deck gfx, bitplane1
	ROM_LOAD( "1.bin", 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "2.bin", 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.bin", 0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( videtron2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // different from videtron
	ROM_LOAD( "4.bin", 0x5000, 0x1000, CRC(4a7dab42) SHA1(7fcdab985b783d90879a99b2a53a6814ca4278eb) ) // sldh
	ROM_LOAD( "5.bin", 0x6000, 0x1000, CRC(c70e8127) SHA1(7db2d4a29cba7c336f254393955fad71f30a539a) ) // sldh
	ROM_LOAD( "6.bin", 0x7000, 0x1000, CRC(490c7304) SHA1(1a6c6112571fd0e35b640ed58f66582a2d99c58b) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(          0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "3.bin", 0x2000, 0x0800, CRC(23e83e89) SHA1(0c6352d46e3dfe176b0e970dd163e2bc01246890) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "0.bin", 0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    // cards deck gfx, bitplane1
	ROM_LOAD( "1.bin", 0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "2.bin", 0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.bin", 0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*  Videotron Poker.
    Only program. No gfx or prom dumps...
    Normal controls.
*/
ROM_START( videtrna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker_videotron.prg", 0x4000, 0x2000, CRC(38494ffb) SHA1(defa03546fd21d854c2d2413e6e2bf575d0518d7) )
	ROM_LOAD( "videotron_poker.prg", 0x6000, 0x2000, CRC(960dcb61) SHA1(a7da40383b0149d21156b461c144d345603d747a) )

	ROM_REGION( 0x3000, "gfx1", 0 ) // taken from videtron
	ROM_FILL(          0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "3.bin", 0x2000, 0x0800, BAD_DUMP CRC(23e83e89) SHA1(0c6352d46e3dfe176b0e970dd163e2bc01246890) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 ) // taken from videtron
	ROM_LOAD( "0.bin", 0x0000, 0x0800, BAD_DUMP CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    // cards deck gfx, bitplane1
	ROM_LOAD( "1.bin", 0x0800, 0x0800, BAD_DUMP CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "2.bin", 0x1000, 0x0800, BAD_DUMP CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )    // taken from videtron
	ROM_LOAD( "82s129.bin", 0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/******************************* JACK POTTEN'S POKER SETS *******************************/

/*  ic2_7.bin    1ST AND 2ND HALF IDENTICAL
    ic3_8.bin    1ST AND 2ND HALF IDENTICAL
    ic5_9.bin    1ST AND 2ND HALF IDENTICAL
    ic7_0.bin    1ST AND 2ND HALF IDENTICAL

    RB confirmed the dump. There are other games with double sized roms and identical halves.
*/
ROM_START( pottnpkr )   // golden poker style game. code is intended to start at $6000. is potten's poker the real name?
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic13_3.bin", 0x2000, 0x1000, CRC(23c975cd) SHA1(1d32a9ba3aa996287a823558b9d610ab879a29e8) )
	ROM_LOAD( "ic14_4.bin", 0x3000, 0x1000, CRC(86a03aab) SHA1(0c4e8699b9fc9943de1fa0a364e043b3878636dc) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "ic7_0.bin",  0x2000, 0x1000, CRC(1090e7f0) SHA1(26a7fc8853debb9a759811d7fee39410614c3895) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "ic2_7.bin",  0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    // cards deck gfx, bitplane1
	ROM_LOAD( "ic3_8.bin",  0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    // cards deck gfx, bitplane2
	ROM_LOAD( "ic5_9.bin",  0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",       0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( potnpkra )    // a coinmaster game?... seems to be a hack
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vp-5.bin",   0x2000, 0x1000, CRC(1443d0ff) SHA1(36625d24d9a871cc8c03bdeda983982ba301b385) )
	ROM_LOAD( "vp-6.bin",   0x3000, 0x1000, CRC(94f82fc1) SHA1(ce95fc429f5389eea45fec877bac992fa7ba2b3c) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "vp-4.bin",   0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "vp-1.bin",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "vp-2.bin",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "vp-3.bin",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
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
	ROM_LOAD( "517.16a",    0x2000, 0x1000, CRC(8892fbd4) SHA1(22a27c0c3709ca4808a9afb8848233bc4124559f) )
	ROM_LOAD( "517.17a",    0x3000, 0x1000, CRC(75a72877) SHA1(9df8fd2c98526d20aa0fa056a7b71b5c5fb5206b) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "517.8a",     0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "517.4a",     0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "517.6a",     0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "517.7a",     0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "517_mb7052.9c",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
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
	ROM_LOAD( "pottpok5.bin",   0x2000, 0x1000, CRC(d74e50f4) SHA1(c3a8a6322a3f1622898c6759e695b4e702b79b28) )
	ROM_LOAD( "pottpok6.bin",   0x3000, 0x1000, CRC(53237873) SHA1(b640cb3db2513784c8d2d8983a17352276c11e07) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "pottpok1.bin",   0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pottpok4.bin",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "pottpok3.bin",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "pottpok2.bin",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( potnpkrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pot5.bin",   0x2000, 0x1000, CRC(d74e50f4) SHA1(c3a8a6322a3f1622898c6759e695b4e702b79b28) )
	ROM_LOAD( "pot6.bin",   0x3000, 0x1000, CRC(53237873) SHA1(b640cb3db2513784c8d2d8983a17352276c11e07) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "pot34.bin",  0x2000, 0x1000, CRC(52fd35d2) SHA1(ad8bf8c222ceb2e9b3b6d9033866867f1977c65f) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "pot1.bin",   0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    // cards deck gfx, bitplane1
	ROM_LOAD( "pot2.bin",   0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",       0x2800, 0x2000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( potnpkre )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g_luck_a.bin",   0x2000, 0x1000, CRC(21d3b5e9) SHA1(32f06eb26c5232738ad7e86f1a81eb9717f9c7e0) )
	ROM_LOAD( "g_luck_b.bin",   0x3000, 0x1000, CRC(7e848e5e) SHA1(45461cfcce06f6240562761d26ba7fdb7ef4986b) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "ic7_0.bin",  0x2000, 0x1000, CRC(1090e7f0) SHA1(26a7fc8853debb9a759811d7fee39410614c3895) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "ic2_7.bin",  0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    // cards deck gfx, bitplane1
	ROM_LOAD( "ic3_8.bin",  0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    // cards deck gfx, bitplane2
	ROM_LOAD( "ic5_9.bin",  0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",       0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*
  Jack Potten's Poker set, with Royale GFX...

  pok-0-.bin          = 0.bin                 royaleb    Royale (set 3)
  pok-1-.bin          = 1.bin                 royaleb    Royale (set 3)
  pok-2-.bin          = 2.bin                 royaleb    Royale (set 3)
  pok-3-.bin          = 3.bin                 royaleb    Royale (set 3)
  82s129.bin          = 82s129.9c             royaleb    Royale (set 3)

  prg.bin             NO MATCH

*/
ROM_START( potnpkrf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg.bin",    0x4000, 0x4000, CRC(d7a932a2) SHA1(c940ea90378a631c217a09c4a9e73c382acaa48d) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "3.bin",  0x2000, 0x0800, CRC(23e83e89) SHA1(0c6352d46e3dfe176b0e970dd163e2bc01246890) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "0.bin",  0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    // cards deck gfx, bitplane1
	ROM_LOAD( "1.bin",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "2.bin",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/* Unknown australian hard to dump set
   No marks, nothing. PCB was rotten.
*/
ROM_START( potnpkrg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unknown.16a", 0x2000, 0x1000, CRC(64ede0e4) SHA1(b7b1872e7e2edec871089f88ec44e7d458564f31) )
	ROM_LOAD( "unknown.17a", 0x3000, 0x1000, CRC(1a9cfbf9) SHA1(47b3767dbcb016ae9ba4437d8f2790681553a5b0) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "unknown1.bin",  0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "unknown2.bin",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "unknown3.bin",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "unknown4.bin",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3. need to be redumped

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bprom.bin",    0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/* Unknown australian set
   String "just 4 fun" replaces the "insert coins" one.
*/
ROM_START( potnpkrh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_2732.15a", 0x2000, 0x1000, CRC(441ad1b2) SHA1(248b8e03723989c7766b50858c9a5d0abbb5e055) )
	ROM_LOAD( "unk_2732.17a", 0x3000, 0x1000, CRC(9d72e145) SHA1(526d88c70e03bcff18072436ca3d498a0bb39913) )

	// Backcard logo is different, showing 'FR'. The rest matches the common char gfx
	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                  0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "unk_2716.8a",   0x1000, 0x0800, CRC(a138afa6) SHA1(80c6d11086f78e36dfc01c15b23e70667fcf17fc) )    // char ROM

	// Backplane at 5a has two bits different against the common cards gfx.
	// Offsets 0x380 and 0x400, bit0 is set to 0
	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "unk_2716.4a",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "unk_2716.5a",  0x0800, 0x0800, CRC(daf38d03) SHA1(6b518494688756ad7b753fdec46b6392c4a9ebbe) )    // cards deck gfx, bitplane2
	ROM_LOAD( "unk_2716.7a",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bprom.bin",    0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/* Potten's Poker bootleg set
   PCB ICP-1 (unencrypted).

  1x R6502P
  2x MC6821P
  1x unknown DIL40.

  1x Xtal 10 MHz.
*/
ROM_START( potnpkri )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7.16a",   0x2000, 0x1000, CRC(ec5c2a2f) SHA1(e2f7c8f02077e890396223d474456613fade5ae2) )
	ROM_LOAD( "8.17a",   0x3000, 0x1000, CRC(3cb1031d) SHA1(bb814577f79f26789752ec29d7ae17ddae822222) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4.8a",   0x1000, 0x0800, CRC(17d0d174) SHA1(ecf507439b6df950052e65ec014830f0287b98b9) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1.4a",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "2.6a",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "3.7a",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "prom.9c",    0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump OK
ROM_END

/*
  Unknown Potten's Poker.
  German bootleg (W.W.).
  Set 11.

  Need to BET in each hand.
*/
ROM_START( potnpkrj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pok5.a11",  0x5000, 0x1000, CRC(60140393) SHA1(f24a4435fe44d14053e28c7cb144ae00f66d74cb) )
	ROM_LOAD( "pok6.a13",  0x6000, 0x1000, CRC(58161c72) SHA1(c5607a843ca1b4bde9999f265acae835ab9832fc) )
	ROM_LOAD( "pok7.a14",  0x7000, 0x1000, CRC(38d92f95) SHA1(9523ef5fae57ad7cdd3fe52667a9670fdca8aa75) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "pok4_ww.a7",  0x1000, 0x0800, CRC(5cafb3a9) SHA1(efec24d4dd1f83f40a1b7ec66bc6bf36c4b1e541) )  // text chars

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "7.a2",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )  // cards deck gfx, bitplane 1
	ROM_LOAD( "8.a4",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )  // cards deck gfx, bitplane 2
	ROM_LOAD( "9.a5",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )  // cards deck gfx, bitplane 3

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM, otherwise settings parameters are incorrect
	ROM_LOAD( "potnpkrj_nvram.bin", 0x0000, 0x0800, CRC(7c5206ce) SHA1(fc6e9b8d21184b8220b8835696d7eb2d071f5475) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10.d7",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // color PROM
ROM_END

/*
  Unknown Potten's Poker.
  English bootleg with Bonanza's graphics.
  Set 12.

  No Double-Up.
  Need to BET in each hand.
*/
ROM_START( potnpkrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "07.a13",   0x6000, 0x1000, CRC(cdb86a7d) SHA1(d1c20aa6484bbaa794bfba993f21ae0bc199db7a) )
	ROM_LOAD( "08.a15",   0x7000, 0x1000, CRC(cb4502f4) SHA1(abe0916f01a1b3742e99e0da5f1b5c6a0e7e7dbd) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "8.a7",   0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )  // text chars

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "4.a2",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )  // cards deck gfx, bitplane 1
	ROM_LOAD( "6.a4",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )  // cards deck gfx, bitplane 2
	ROM_LOAD( "7.a5",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )  // cards deck gfx, bitplane 3

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM, otherwise settings parameters are incorrect
	ROM_LOAD( "potnpkrk_nvram.bin", 0x0000, 0x0800, CRC(87970f3f) SHA1(3130594c83407e13f2b08027188b333f29dce568) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s129n.d7",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // color PROM
ROM_END

/* Potten's Poker bootleg set
   PCB ICP-1 (unencrypted).
*/
ROM_START( potnpkrl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unknown.16a",  0x2000, 0x1000, CRC(2ed4f11f) SHA1(5900ace86929584d49efd055c748d5dd110cf9c0) )
	ROM_LOAD( "unknown.17a",  0x3000, 0x1000, CRC(f0992aa3) SHA1(90377024b19b7ca60baa99a6c8ea96cf625d21b4) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                 0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "unknown.9a",   0x1000, 0x0800, CRC(2c53493f) SHA1(9e71db51499294bb4b16e7d8013e5daf6f1f9d18) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "unknown.4a",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "unknown.6a",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "unknown.7a",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",    0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END


ROM_START( goodluck )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goodluck_glh6b.bin", 0x0000, 0x8000, CRC(2cfa4a2c) SHA1(720e2900f3a0ef2632aa201a63b5eba0570e6aa3) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4.bin",  0x2000, 0x1000, CRC(41924d13) SHA1(8ab69b6efdc20858960fa5df669470ba90b5f8d7) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END


/************************************************************

  Ngold (3 sets).
  198?.

  Nothing about them, except that they are running
  on ICP-1 type boards.

  It's a supossed Jack Potten's Poker upgrade.

*************************************************************/

ROM_START( ngold )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ngold_1a.16a",   0x2000, 0x1000, CRC(ca259396) SHA1(32bd647fcba99029f916c2a6df4152efc5a70fcb) )
	ROM_LOAD( "ngold_2a.17a",   0x3000, 0x1000, CRC(9d07f0fc) SHA1(493b2e778342e1d6b7753902b714c5478bd22bd5) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4.8a",   0x1000, 0x0800, CRC(f54c6f43) SHA1(fe66542b95259c10f7954d52d1bd5747ce99df42) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1.4a",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "2.5a",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "3.7a",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s129n.9c",    0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( ngolda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1b.bin", 0x2000, 0x1000, CRC(c7b0585c) SHA1(6ed57ee2045991e3f233aecea7e0f147f3e41977) )
	ROM_LOAD( "2a.bin", 0x3000, 0x1000, CRC(9d07f0fc) SHA1(493b2e778342e1d6b7753902b714c5478bd22bd5) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4.8a",   0x1000, 0x0800, CRC(f54c6f43) SHA1(fe66542b95259c10f7954d52d1bd5747ce99df42) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1.4a",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "2.5a",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "3.7a",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s129n.9c",    0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( ngoldb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pkr_1a.bin", 0x2000, 0x1000, CRC(9140b1dc) SHA1(1fbbe5479c7fac0a3f667ca5a20f2119620c54b1) )
	ROM_LOAD( "pkr_2.bin",  0x3000, 0x1000, CRC(de03a57d) SHA1(db696a892497ead7aa4ed2c600ba819c3b41a082) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "0.bin",  0x1000, 0x0800, CRC(f54c6f43) SHA1(fe66542b95259c10f7954d52d1bd5747ce99df42) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "8.bin",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "9.bin",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s129n.9c",    0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END


/*********************************************

  Amstar Draw Poker
  -----------------

  PCB chips:

  SY6502 main CPU
  MC8621P x 2
  MC6845P CRT Controller

  TI NE555P precision timer
  MB7051 BPROM

  OSC: Unknown, no markings

  8-switch dipswitch on PCB
  6-switch dipswitch on riser board with 6502 CPU
  Off/On slider switch

  2x 2732 program ROMs
  4x 2716 data/graphics? ROMs
  1x MB7051 BPROM

-----------------------------------------

  Program 2000-3fff / A14 & A15 disconnected.

  NMI  : 2DF0
  Start: 2D36
  Reset: 2F0B

  There are code just before the vectors.
  Curious about it... Will be analyzed.

*********************************************/

ROM_START( adpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732.16a",   0x2000, 0x1000, CRC(8892fbd4) SHA1(22a27c0c3709ca4808a9afb8848233bc4124559f) )
	ROM_LOAD( "2732.17a",   0x3000, 0x1000, CRC(b4db832f) SHA1(af1f26c5b703a9031690d4b63fb8555236cd6ddd) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "2716.8a",    0x1000, 0x0800, CRC(7ce15156) SHA1(13c604b4d97186f1cef2cffbc437e76990f7e4bb) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "2716.4a",     0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "loson.6a",    0x0800, 0x0800, CRC(5afb7cc7) SHA1(334deb71f2d59dd7264150d1c15af01e49f9bd86) )    // cards deck gfx, bitplane2
	ROM_LOAD( "loson.7a",    0x1000, 0x0800, CRC(19fec412) SHA1(580a56d9a2ae94abf1185ed7fc0280d51e9d5964) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7052.9c",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END



/**************************************** BUENA SUERTE SETS ****************************************/

/*
    checksum routine at $5827
    protect $4000+ & $7ff9.
    (see cmp at $584a)
    balanced at $7ff8.
*/
ROM_START( bsuerte )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_chica.256: good BS set... (checksum)
	ROM_LOAD( "bs_chica.256",   0x0000, 0x8000, CRC(2e92b72b) SHA1(6c90fb265f2cb7ec40ddb0553b5b7fedfa89339c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ups39_12a.bin",  0x0000, 0x8000, CRC(e6b661b7) SHA1(b265f6814a168034d24bc1c25f67ece131281bc2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuerteb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bsrapida.128: Buena Suerte! red title, from Cordoba"
	ROM_LOAD( "bsrapida.128",   0x4000, 0x4000, CRC(a2c633fa) SHA1(7cda3f56e6bd8e6bfc36a68c16d2e63d76d4dac3) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "x10d4esp.16c",   0x0000, 0x8000, CRC(0606bab4) SHA1(624b0cef1a23a4e7ba2d2d256f30f73b1e455fa7) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuerted )
	ROM_REGION( 0x10000, "maincpu", 0 ) // set seen nowadays, based on bsuertec
	ROM_LOAD( "x10d4fix.bin",   0x0000, 0x8000, CRC(c5ecc419) SHA1(5538a1336b877d1780d9a0c5595b02e9b22ee17d) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertee )
	ROM_REGION( 0x10000, "maincpu", 0 ) // source program for other mods
	ROM_LOAD( "x10bb26.bin",    0x0000, 0x8000, CRC(57011385) SHA1(3cbfdb8dd261aa8ce27441326f0916640b13b67a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertef )
	ROM_REGION( 0x10000, "maincpu", 0 ) // add its own logo ($0000-$4000) in the cards-back
	ROM_LOAD( "bscat.256",  0x0000, 0x8000, CRC(944accd3) SHA1(f1ed149b9dafe9cdf3745b9344f2ce1814027005) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuerteg )
	ROM_REGION( 0x10000, "maincpu", 0 ) // based on witchcrd (winning counter, no lamps, only 9 settings parameters)
	ROM_LOAD( "bsjc.256",   0x0000, 0x8000, CRC(3a824d96) SHA1(1eb2b4630be10131416ff84213aa858a072896ac) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuerteh )
	ROM_REGION( 0x10000, "maincpu", 0 ) // based on witchcrd (winning counter, no lamps, only 9 settings parameters)
	ROM_LOAD( "jc603d.256", 0x0000, 0x8000, CRC(25df69e5) SHA1(54d2798437b61bd0e1919fb62daf24ed9df42678) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertei )
	ROM_REGION( 0x10000, "maincpu", 0 ) // mcs: Buena Suerte! (ind arg, Cordoba)
	ROM_LOAD( "mcs.256",    0x0000, 0x8000, CRC(5c944e9d) SHA1(e394f8a32f4ebe622c0d0c30db5cb9d6d70b2126) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertej )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bsgemini: BS hack by SUSILU, bad texts, and need proper chars
	ROM_LOAD( "bsgemini.256",   0x0000, 0x8000, CRC(883f94d0) SHA1(30ff337ed2f454f74dfa354c14a8ab422284d279) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertek )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bsindarg: Buena Suerte! (ind arg, Cordoba, set 2)
	ROM_LOAD( "bsindarg.128",   0x4000, 0x4000, CRC(a9aaff1a) SHA1(13c9fbd0e9a04f42ded4dda0bb8a850de65cc671) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertel )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bslacer128: Buena Suerte! (portugues), English settings
	ROM_LOAD( "bslacer.128",    0x4000, 0x4000, CRC(edc254f4) SHA1(20e5543e59bfd67a0afec7cbeeb7000f6bba6c69) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertem )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bslacer128: Buena Suerte! (portugues), English settings, set 2
	ROM_LOAD( "bslacer.256",    0x0000, 0x8000, CRC(9f8a899a) SHA1(a1f3d0635b309d4734289b7ff48eceda69dfd3d0) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*
  Buena Suerte.

  Prodel HW.
  Copacabana, Rio de Janeiro.
  Brazil.

  Golden Poker derivative, with blue killer circuitry.
  Suitable for "Good Luck" and "Buena Suerte" games.
  Addressing, ROM banks, and edge connector close to Magic Fly.

  Discrete sound need to be traced...

  30x2-pins Edge connector + 10-pin connector.

  The PCB looks like a cheap copy of a more vintage hardware.


  *********** Edge connector (WIP) ************

     Solder side  |Conn|  Components side
  ----------------+----+---------------------
                  |    |
            +5V.  | 01 |  +5V.
  (GND) Speaker-  | 02 |  Speaker- (GND)
                  | 03 |  Speaker+
                  | 04 |
                  |    |
            Sync  | 05 |  Red
           Green  | 06 |  Blue
                  |    |
           (out)  | 07 |  (out)
           (out)  | 08 |  (out)
           (out)  | 09 |  12VAC. *IN* (0)
                  |    |
         (input)  | 10 |  (input)
         (input)  | 11 |  (input)
         (input)  | 12 |  (input)
         (input)  | 13 |  (input)
         (input)  | 14 |  (input)
         (input)  | 15 |  (input)
         (input)  | 16 |  (input)
         (input)  | 17 |  (input)
         (input)  | 18 |  (input)
         (input)  | 19 |  (input)
                  |    |
    Common C (3)  | 20 |  Common A (1)
    Common D (4)  | 21 |  Common B (2)
                  |    |
                  | 22 |  n/c (5)
                  | 23 |
                  | 24 |
                  |    |
             GND  | 25 |  GND
         +12VAC.  | 26 |  +12VAC.
         +12VAC.  | 27 |  +12VAC.
         +10VAC.  | 28 |  +10VAC.
         +10VAC.  | 29 |  +10VAC.
             GND  | 30 |  GND
                  |    |

  (0) 12V. AV *IN*: for lamps, audio, and mech counters.

  (1) =
  (2) =
  (3) =
  (4) =

  Note: Each Common GND (A-B-C-D) are for their respective
  Multiplexed groups of inputs, since there are 4 groups
  with 5 valid inputs each one.

  (5) = Not connected, but there is a wire patch from here to
        the internal Power Supply diode, to enter 12Vcc. from
        a external power supply instead of the normal 12VAC.
        expected for the hardware.


  ** 10-pin connector **

  Only the last 4 have a male connector. The rest are just marked on the PCB.

  0000000000
  ++++
  ||||
  |||'-- Out Mech Counter.
  ||'--- Out Mech Counter.
  |'---- n/c (is routed to a place where another transistor could be mounted).
  '----- 12V.

*/
ROM_START( bsuerten )
	ROM_REGION( 0x10000, "maincpu", 0 )  // bs_x10: BS normal, fast. Prodel PCB.
	ROM_LOAD( "bs_x10__27c128.16a", 0x4000, 0x4000, CRC(2549ceeb) SHA1(8c17849c7e9c138c35df584cdc0eabf536edb3d9) )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "1.4a", 0x0000, 0x2000, CRC(943d200b) SHA1(e0c9d626be8e075e2087efc020c710aed3ca7511) )  // 4th quarter has the cards bitplane.
	ROM_LOAD( "2.6a", 0x2000, 0x2000, CRC(e0c7fb67) SHA1(26b6dc9615121b86160352bb969e9fb0f5ed3618) )  // 4th quarter has the cards bitplane.
	ROM_LOAD( "3.7a", 0x4000, 0x2000, CRC(2b888258) SHA1(e16587119f548298a5d23d0cb9250badc0321b93) )  // 3rd quarter has the cards bitplane, 4th quarter has the charset.

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(         0x0000, 0x1000, 0x0000 )  // filling the R-G bitplanes.
	ROM_COPY( "gfx",  0x5800, 0x1000, 0x0800 )  // chars.

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "gfx",  0x1800, 0x0000, 0x0800 )  // cards deck gfx, bitplane 1.
	ROM_COPY( "gfx",  0x3800, 0x0800, 0x0800 )  // cards deck gfx, bitplane 2.
	ROM_COPY( "gfx",  0x5000, 0x1000, 0x0800 )  // cards deck gfx, bitplane 3.

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s129an.9c", 0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // PROM dump verified.
ROM_END


ROM_START( bsuerteo )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_x10.256: BS normal, fast, set 2
	ROM_LOAD( "bs_x10.256", 0x0000, 0x8000, CRC(ad3427a6) SHA1(d0a954c86c0a4354b5cea4140b8da7a10f66337a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertep )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_p.128: another common BS set
	ROM_LOAD( "bs_p.128",   0x4000, 0x4000, CRC(9503cfef) SHA1(f3246621bb9dff3d357d4c99f7075509899ed05f) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuerteq )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_r4.128: BS portunhol, white title
	ROM_LOAD( "bs_r4.128",  0x4000, 0x4000, CRC(22841e2f) SHA1(d547aa6ddb82aff0d87eeb9bae67281d22dc50d5) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuerter )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_100.128: BS with 1-100 bet (only allow 50)
	ROM_LOAD( "bs_100.128", 0x4000, 0x4000, CRC(1d3104e5) SHA1(9c0f00725270aa4d28b5a539431311bdca2f864a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertes )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_50.128: BS, normal set
	ROM_LOAD( "bs_50.128",  0x4000, 0x4000, CRC(8c2e43ca) SHA1(3e3f0848964f4ee6f47ddcf2220ebd06d771eebf) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuertet )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_c.128: BS portunhol, with typos (Halta)
	ROM_LOAD( "bs_c.128",   0x4000, 0x4000, CRC(8b605bdf) SHA1(a933149999937f44cb62a3b34ab55ac4b5a50f72) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( bsuerteu )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_origi.bin: BS portunhol, with typos (Halta & Fixa)
	ROM_LOAD( "bs_origi.bin",   0x0000, 0x8000, CRC(63a1ba65) SHA1(2354461ec7ad75f7ff2699e89d40517463157aaa) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "u38.bin",    0x2000, 0x1000, CRC(0a159dfa) SHA1(0a9c8e6177b36831b365917a10042aac3383983d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*
  Buena Suerte.

  Brazilian copy of Protel HW, but with inferior quality.

  Golden Poker derivative, with blue killer circuitry.
  Suitable for "Good Luck" and "Buena Suerte" games.
  Addressing, ROM banks, and edge connector close to Magic Fly.

  Discrete sound need to be traced...

  30x2-pins Edge connector + 10-pin connector.
  (see Protel set for pinouts)

  Software is a hack that mix spanish and portuguese words.
  Suitable for southern Brazil, or north of Argentina.

  DIP switches are just replaced with fixed bridges.
  Minimal hand is set in Double Pair.

  GFX ROMs are identical to the Protel set.

*/
ROM_START( bsuertev )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Brazilian copy of Protel PCB.
	ROM_LOAD( "main.16a", 0x4000, 0x4000, CRC(d56849ce) SHA1(2bba9821e53679b024d74ac1e023bf6bc6750c29) )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "1.4a", 0x0000, 0x2000, CRC(943d200b) SHA1(e0c9d626be8e075e2087efc020c710aed3ca7511) )  // 4th quarter has the cards bitplane.
	ROM_LOAD( "2.6a", 0x2000, 0x2000, CRC(e0c7fb67) SHA1(26b6dc9615121b86160352bb969e9fb0f5ed3618) )  // 4th quarter has the cards bitplane.
	ROM_LOAD( "3.7a", 0x4000, 0x2000, CRC(2b888258) SHA1(e16587119f548298a5d23d0cb9250badc0321b93) )  // 3rd quarter has the cards bitplane, 4th quarter has the charset.

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(         0x0000, 0x1000, 0x0000 )  // filling the R-G bitplanes.
	ROM_COPY( "gfx",  0x5800, 0x1000, 0x0800 )  // chars.

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "gfx",  0x1800, 0x0000, 0x0800 )  // cards deck gfx, bitplane 1.
	ROM_COPY( "gfx",  0x3800, 0x0800, 0x0800 )  // cards deck gfx, bitplane 2.
	ROM_COPY( "gfx",  0x5000, 0x1000, 0x0800 )  // cards deck gfx, bitplane 3.

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s129an.9c", 0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // PROM dump verified.
ROM_END


/**************************************** VIDEO KLEIN SETS ****************************************/


/*********** WITCH CARD SETS *************/

/*  Witch Card (Video Klein)
    Video Klein original with epoxy block module
*/
ROM_START( witchcrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epoxy_27128.bin",    0x4000, 0x4000, CRC(48186272) SHA1(d211bfa89404a292e6d0f0169ed11e1e74a361d9) )  // epoxy block program ROM

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "wc4.7a", 0x2000, 0x0800, CRC(6a392b10) SHA1(9f36ae2e5a9a8741c6687e9c875d7b45999d9d6d) )    // char ROM
	ROM_LOAD( "wc4.7a", 0x2800, 0x0800, CRC(6a392b10) SHA1(9f36ae2e5a9a8741c6687e9c875d7b45999d9d6d) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "wc1.2a", 0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    // cards deck gfx, bitplane1
	ROM_LOAD( "wc2.4a", 0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    // cards deck gfx, bitplane2
	ROM_LOAD( "wc3.5a", 0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s129.7d",          0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // original PCB PROM
	ROM_LOAD( "epoxy_82s129.bin",   0x0100, 0x0100, CRC(f0c012b1) SHA1(5502977404172e8c5b9fbf305581a406668ad1d9) ) // original epoxy block PROM
ROM_END

/*  Witch Card (Spanish, set 1)
    Unknown argentine manufacturer.
*/
ROM_START( witchcrda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w_card.256", 0x0000, 0x8000, CRC(63a471f8) SHA1(96a2140e2da0050e7865a6662f707cf024130832) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "bs_4_wcspa.032", 0x2000, 0x1000, CRC(4e520c7a) SHA1(1de3ac4a150160c15f453b0d3f9d3cd3178bfedd) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*  Witch Card (Spanish, set 2)
    Unknown argentine manufacturer.
*/
ROM_START( witchcrdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w_card.128", 0x4000, 0x4000, CRC(11ecac96) SHA1(717709b31f3dfa09be321c14fbf0e95d492ad2f2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "bs_4_wcspa.032", 0x2000, 0x1000, CRC(4e520c7a) SHA1(1de3ac4a150160c15f453b0d3f9d3cd3178bfedd) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*  Witch Card (English, no witch game)
    Hack?
*/
ROM_START( witchcrdc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wc_sbruj.256",   0x0000, 0x8000, CRC(5689ae41) SHA1(c7a624ec881204137489b147ce66cc9a9900650a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "bs_4_wc.032",    0x2000, 0x1000, CRC(41924d13) SHA1(8ab69b6efdc20858960fa5df669470ba90b5f8d7) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/***************************************

  Witch Card (German, WC3050, set 1 )

  TV GAME ELEKTRONIK 1994
         PROMA
   CASINOVERSION WC3050

***************************************/
ROM_START( witchcrdd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "12a.bin",    0x0000, 0x8000, CRC(a5c1186a) SHA1(b6c662bf489fbcccc3063ce55c957e630ba96ccb) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x4000, 0x00000 ) // filling the R-G bitplanes
	ROM_LOAD( "ce-3-tvg.bin",   0x4000, 0x2000, CRC(54b51497) SHA1(8c3a74377fde8c7c5a6b277a9c1e717e6bdd98f8) )    // char ROM

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "ce-1-tvg.bin",   0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "ce-2-tvg.bin",   0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x4800, 0x4000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*  Witch Card (Video Klein)
    Video Klein original with epoxy block module.
    Alt set....
*/
ROM_START( witchcrde )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27128_epoxy.bin",    0x4000, 0x4000, CRC(48186272) SHA1(d211bfa89404a292e6d0f0169ed11e1e74a361d9) )  // epoxy block program ROM

	ROM_REGION( 0x4000, "temp", 0 )
	ROM_LOAD( "wc1.a2", 0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )
	ROM_LOAD( "wc2.a4", 0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )
	ROM_LOAD( "wc3.a5", 0x2000, 0x1000, CRC(a03f2d68) SHA1(6d81b1e92f40f7150498b65941d5a9ab64a89790) )
	ROM_LOAD( "wc4.a7", 0x3000, 0x1000, CRC(d3694522) SHA1(0f66ff2dd5c7ac9bf91fa9f48eb9f356572e814c) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x1000, 0x00000 ) // filling the R-G bitplanes
	ROM_COPY( "temp",   0x3800, 0x1000, 0x0800 )    // 0800-0fff of wc4.a7 - charset

	ROM_REGION( 0x1800, "gfx2", 0 ) // 2nd half of each ROM
	ROM_COPY( "temp",   0x0800, 0x0000, 0x0800 )    // 0800-0fff of wc1.a2 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x1800, 0x0800, 0x0800 )    // 0800-0fff of wc2.a4 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x2800, 0x1000, 0x0800 )    // 0800-0fff of wc3.a5 - regular cards gfx, bitplane 3

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "24s10.bin",          0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // original PCB PROM
	ROM_LOAD( "24s10_epoxy.bin",    0x0100, 0x0100, CRC(ddfd7034) SHA1(78dee69ab4ba759485ee7f00446c2d86f08cc50f) ) // original epoxy block PROM
ROM_END

/*  Witch Card (English, witch game, lights)
    PCB by PM. Hybrid hardware.

    Copyright 1983/84/85
    W.BECK ELEKTRONIK
*/
ROM_START( witchcrdf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "@25.bin",    0x5000, 0x1000, CRC(afd6cb4a) SHA1(4c769e1c724bada5875e028781086c32967953a1) )
	ROM_LOAD( "@26.bin",    0x6000, 0x1000, CRC(ad11960c) SHA1(2b562cfe9401e21c9dcd90307165e2c2d1acfc5b) )
	ROM_LOAD( "@27.bin",    0x7000, 0x1000, CRC(e6f9c973) SHA1(f209d13d1565160bc2c05c6c4fce73d14a9a56ab) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "@14.bin",    0x2000, 0x1000, CRC(19b68bec) SHA1(b8ef17ba5545e2f104cd2783e5f1c97c400fcbbc) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "11.bin", 0x0000, 0x1000, CRC(b5a1f5a3) SHA1(a34aaaab5443c6962177a5dd35002bd09d0d2772) )    // cards deck gfx, bitplane1
	ROM_LOAD( "12.bin", 0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    // cards deck gfx, bitplane2
	ROM_LOAD( "13.bin", 0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wc_bprom.bin",   0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*******************************************

  Witch Card (Falcon)
  Original Falcon PCB marked
  "831 1.1 MADE IN JAPAN"

  Same board as Falcons Wild, but without
  extra RAM / ROM / encrypted 2nd CPU.

  AY8910 is present.

*******************************************/
ROM_START( witchcrdg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6.b9",   0x5000, 0x1000, CRC(70462a63) SHA1(9dfa18bf7d4e0803f2a68e64661ece392a7983cc) )
	ROM_LOAD( "7.b11",  0x6000, 0x1000, CRC(227b3801) SHA1(aebabce01b1abdb42b3e49c38f4fe429e65c1a88) )
	ROM_LOAD( "8.b13",  0x7000, 0x1000, CRC(6bb0059e) SHA1(c5f515b692c3353323aff77f087bf0a92a8d99cf) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "3.b5",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_FILL(           0x0800, 0x0800, 0x00000 ) // filling the bitplane
	ROM_LOAD( "2.b3",   0x1000, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_FILL(           0x1800, 0x0800, 0x00000 ) // filling the bitplane
	ROM_LOAD( "1.b1",   0x2000, 0x1000, CRC(8a17d1a7) SHA1(488e4eae287b05923bd6b378574e91cfe49d8c24) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x00000 )   // filling the R-G bitplanes
	ROM_COPY( "gfx2",   0x2800, 0x2000, 0x0800 )    // srctag, srcoffs, offset, length

	ROM_REGION( 0x0100, "proms", 0 )
//  ROM_LOAD( "82s129.7d",          0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // original PCB PROM
	ROM_LOAD( "tbp24s10n.d2",   0x0000, 0x0100, BAD_DUMP CRC(3db3b9e0) SHA1(c956493d5d754665d214b416e6a473d73c22716c) )
ROM_END

/***************************************

  Witch Card (German, WC3050, set 2 )

  TV GAME ELEKTRONIK 1994
         PROMA
   CASINOVERSION WC3050

***************************************/
ROM_START( witchcrdh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prog3000.a12",   0x0000, 0x8000, CRC(a5c1186a) SHA1(b6c662bf489fbcccc3063ce55c957e630ba96ccb) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x4000, 0x00000 ) // filling the R-G bitplanes
	ROM_LOAD( "wc3050.a5",  0x4000, 0x2000, CRC(6f35b9c4) SHA1(df86687164f18f2bfe71e73cccd28fe4117e748c) )    // char ROM, alt gfx

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "wc1.a2", 0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    // cards deck gfx, bitplane1 // sldh
	ROM_LOAD( "wc1.a4", 0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    // cards deck gfx, bitplane2 // sldh
	ROM_COPY( "gfx1",   0x4800, 0x4000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/***************************************

  Witch Card (German, WC3050, 27-4-94)

  TV GAME ELEKTRONIK 1994
         PROMA
   CASINOVERSION WC3050

****************************************

01.a2                                           BADADDR           --xxxxxxxxxx
02.a4                                           BADADDR           --xxxxxxxxxx
03.a5                                           1ST AND 2ND HALF IDENTICAL
27s21.d7                                        FIXED BITS (0000xxxx)
                        tbp24s10n.7d            FIXED BITS (0000xxxx)
                        ce-2-tvg.bin            BADADDR           --xxxxxxxxxx
                        ce-1-tvg.bin            BADADDR           --xxxxxxxxxx
01.a2                   ce-1-tvg.bin            IDENTICAL
02.a4                   ce-2-tvg.bin            IDENTICAL
27s21.d7                tbp24s10n.7d            IDENTICAL
03.a5        [2/4]      ce-3-tvg.bin [2/4]      IDENTICAL
04.a12       [2/4]      12a.bin      [2/4]      IDENTICAL
03.a5        [2/4]      ce-3-tvg.bin [4/4]      IDENTICAL
03.a5        [4/4]      ce-3-tvg.bin [2/4]      IDENTICAL
03.a5        [4/4]      ce-3-tvg.bin [4/4]      IDENTICAL
04.a12       [1/4]      12a.bin      [1/4]      99.609375%
04.a12       [3/4]      12a.bin      [3/4]      99.414063%
04.a12       [4/4]      12a.bin      [4/4]      96.020508%
03.a5        [1/4]      ce-3-tvg.bin [1/4]      88.378906%
03.a5        [3/4]      ce-3-tvg.bin [1/4]      88.378906%

***************************************/
ROM_START( witchcrdi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "04.a12", 0x0000, 0x8000, CRC(0f662e02) SHA1(71d7344f63c11082beb4fb4eeb20b04780a9b14c) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "03.a5",  0x4000, 0x2000, CRC(f181e5aa) SHA1(44a7696bd223effbc7542142a0c3c623c628071d) )    // char ROM

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "01.a2",  0x0000, 0x2000, CRC(10b34856) SHA1(52e4cc81b36b4c807b1d4471c0f7bea66108d3fd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "02.a4",  0x2000, 0x2000, CRC(5fc965ef) SHA1(d9ecd7e9b4915750400e76ca604bec8152df1fe4) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x4800, 0x4000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "27s21.d7",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/******************************************

  Witch Game (Video Klein)

  Another evil hardware from Video Klein
  with CPU box. Marked "12T1"

******************************************/
ROM_START( witchgme )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Video Klein
	ROM_LOAD( "hn58c256p.box12t1",  0x0000, 0x8000, CRC(26c334cb) SHA1(d8368835c88668f09560f6096148a6e528806f65) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "1.2a",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )  // cards deck gfx, bitplane1
	ROM_FILL(           0x0800, 0x0800, 0x0000 ) // filling the bitplane
	ROM_LOAD( "2.4a",   0x1000, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )  // cards deck gfx, bitplane2
	ROM_FILL(           0x1800, 0x0800, 0x0000 ) // filling the bitplane
	ROM_LOAD( "3.5a",   0x2000, 0x1000, CRC(8a17d1a7) SHA1(488e4eae287b05923bd6b378574e91cfe49d8c24) )  // char ROM

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 )    // filling the R-G bitplanes
	ROM_COPY( "gfx2",   0x2800, 0x2000, 0x0800 )    // srctag, srcoffs, offset, length

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s137f.box",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )

	ROM_REGION( 0x0100, "proms2", 0 )
	ROM_LOAD( "tbp24s10n.2c",   0x0000, 0x0100, CRC(7c2aa098) SHA1(539ff9239b1b553b3883c9f0223aafcf217f9fc7) )
ROM_END

/*  Witch Game (Video Klein)
    Video Klein original with epoxy block module.
    Alt set....
*/
ROM_START( witchgmea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wc_epoxy.bin",   0x0000, 0x8000, CRC(33f1acd9) SHA1(2facb3d807b5b2a2978e567d0c1106c0a027621a) )  // epoxy block program ROM

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(            0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "wc4.7a",  0x2000, 0x1000, BAD_DUMP CRC(3bf07c44) SHA1(f6e859b142b7d4585b89ca609d8bc85c84fe2b09) )    // chars ROM, corrupt // sldh
	ROM_COPY( "gfx1",    0x2800, 0x2000, 0x0800 )   // srctag, srcoffs, offset, length

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "wc1.2a", 0x0000, 0x1000, CRC(f59c6fd2) SHA1(bea4b6043728311ca9fff36e2d7e24254af5b97a) )    // cards deck gfx, bitplane1 // sldh
	ROM_LOAD( "wc2.4a", 0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )    // cards deck gfx, bitplane2
	ROM_LOAD( "wc3.5a", 0x2000, 0x1000, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10.bin",          0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // original PCB PROM
	ROM_LOAD( "82s137_epoxy.bin",   0x0100, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) ) // original epoxy block PROM
ROM_END


/*
  Joker Card (witch game)
  PCB with daugtherboard c/6502 and rom.

  Program tries to show jokers in the attract,
  but these aren't in the graphics set.

  The GFX roms for cards, have identical halves,
  and the jokers extended GFX should be placed
  in the second half.

  Fortunatelly we have an identical GFX set from
  another game (that match 100% the first half),
  so for now could assume we can use it instead
  till the original devices appear...

  The game apparently uses jokers and has 3's
  and 7's bonus.

  Depending of the bet, you can have one or two
  witches as bonus if you win a hand.

*/
ROM_START( jokercar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-5.sub", 0x4000, 0x4000, CRC(fa99f263) SHA1(209c8801a253de562bac091e37b091c8176c0943) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                  0x0000, 0x2000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "4u.8a", 0x2000, 0x1000, CRC(85044094) SHA1(06db79dd47a47886480bd7a6546a2252cd48b3e3) )    // chars gfx

	ROM_REGION( 0x3000, "gfx2", 0 )  // these are from another set that match 100% the first half of the original devices, that have identical halves...
	ROM_LOAD( "7.4a",  0x0000, 0x1000, BAD_DUMP CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane 1
	ROM_LOAD( "6.6a",  0x1000, 0x1000, BAD_DUMP CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane 2
	ROM_LOAD( "5.7a",  0x2000, 0x1000, BAD_DUMP CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/************************************

  Jolli Witch (German)
  Epoxy CPU box.

  Using the whole addressing

************************************/

ROM_START( witchjol )
	ROM_REGION( 0x10000, "maincpu", 0 ) // inside epoxy block with the CPU
	ROM_LOAD( "27c256.bin", 0x8000, 0x8000, CRC(14f05e3b) SHA1(83578f6a82b0974dd0325903926b2fd0d8e5c236) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "iii.5a", 0x00000, 0x2000, CRC(5ea338da) SHA1(5e55e17689541ffb9c23e45f689dda98a79bf789) )
	ROM_LOAD( "ii.4a",  0x10000, 0x2000, CRC(044dfac0) SHA1(721f8f57e05ddcbdb838d12fd3e81a45346ee6db) )
	ROM_LOAD( "i.2a",   0x08000, 0x2000, CRC(d467f6e2) SHA1(6aaf4cdfb76f5efeeee45635fea120711483648e) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of iii.5a - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of i.2a - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of ii.4a - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of iii.5a - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of i.2a - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of ii.4a - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of iii.5a - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of i.2a - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of ii.4a - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of iii.5a - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "witchjol_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END

/****************************************************

  Wild Witch sets...

  Another evil hardware from Video Klein
  with CPU box.

  The program ask you to install a new block
  or retain the former buggy one.


****************************************************/

ROM_START( wldwitch )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 184A, 2001-09-12
	ROM_LOAD( "ww184a.bin", 0x8000, 0x8000, CRC(f45edc9b) SHA1(9a7400a84b685b84081d424d6da096632b845de8) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitcha )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 157-SP, 1992-12-25
	ROM_LOAD( "ww157-sp.bin",   0x8000, 0x8000, CRC(34396a51) SHA1(823e817a01fab49deacf8af474e31732b96a15d1) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )         // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 157-TE, 1992-12-25
	ROM_LOAD( "ww157-te.bin",   0x8000, 0x8000, CRC(9bf25a7c) SHA1(a78f946403254a30d9afa3c43ca42dfc02edd8fb) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )         // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchc )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 162A, 1994-04-26
	ROM_LOAD( "ww162a.bin", 0x8000, 0x8000, CRC(59765e59) SHA1(474119fe179e0950b082fff8b014ceae8c82b44b) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchd )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 162B, 1994-04-26
	ROM_LOAD( "ww162b.bin", 0x8000, 0x8000, CRC(a60e0f28) SHA1(88e41b9cfe76e2c70d0ebfb73801478412cd4ba4) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitche )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 162A-F, 1994-04-26
	ROM_LOAD( "ww162a-f.bin",   0x8000, 0x8000, CRC(1aba84c1) SHA1(a825bd6312385c5a1768e8156fd7dad770926564) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchf )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 162A alt, 1994-11-03
	ROM_LOAD( "ww162a-alt.bin", 0x8000, 0x8000, CRC(0a8175b9) SHA1(23b300397491140a03de43140d0a05f154e90eab) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchg )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 162B alt, 1994-11-03
	ROM_LOAD( "ww162b-alt.bin", 0x8000, 0x8000, CRC(f5f924c8) SHA1(a49ae2c8c3f3ec9fd1727564220aa1e8da633774) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchh )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 165A, 1995-11-16
	ROM_LOAD( "ww165a.bin", 0x8000, 0x8000, CRC(9119add6) SHA1(fcf13831d968498d09daec993924a08ffefb80c8) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchi )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 165A-S, 1996-03-26
	ROM_LOAD( "ww165a-s.bin",   0x8000, 0x8000, CRC(c5827a07) SHA1(474d6a715c230d3a1e19f9d4850eb52443cd975f) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchj )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 165A-S alt, 1996-05-26
	ROM_LOAD( "wn165a-s-alt.bin",   0x8000, 0x8000, CRC(8b01bb4b) SHA1(bd42ce4ce46561ce2e094130710a55a122c5cc3e) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchk )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 165A-N, 1996-05-29
	ROM_LOAD( "wn165a-n.bin",   0x8000, 0x8000, CRC(df9a6c9a) SHA1(07a09ad77b1e5b88b065e4c1ddaa201e7f904888) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchl )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 170A-beta, 1996-06-25
	ROM_LOAD( "ww170a-beta.bin",    0x8000, 0x8000, CRC(32dbaa23) SHA1(5f5b0b7ad56abe20a2b9b3670b98a4741ea8aaab) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchm )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 170A, 1996-09-30
	ROM_LOAD( "ww170a.bin", 0x8000, 0x8000, CRC(1d976e56) SHA1(c03014f3b0e682cd0f025363108b5aa410c2b54b) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchn )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 170A alt, 1997-06-11
	ROM_LOAD( "ww170a-alt.bin", 0x8000, 0x8000, CRC(4266b71c) SHA1(c0fd545ae629f3456c447b3b695caeec42521a71) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitcho )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 174A-SP-BELG, 1998-05-11
	ROM_LOAD( "ww174a-sp-belg.bin", 0x8000, 0x8000, CRC(d490c676) SHA1(9403bcc003c9b48b25857bd142a73c8d23c5f5b5) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchp )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 174A, 1998-09-20
	ROM_LOAD( "ww174a.bin", 0x8000, 0x8000, CRC(d4129f5a) SHA1(139c4b6f5972b5d7c549b27b114a81fd2de178c3) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchq )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 174A alt, box, 1998-09-25
	ROM_LOAD( "wn174a_hn58c256p_box.bin",   0x8000, 0x8000, CRC(1de736a7) SHA1(e714a97999555fe0107390c8c9c2c3c1e822809a) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchr )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 175A-E, 1999-01-11
	ROM_LOAD( "ww175a-e.bin",   0x8000, 0x8000, CRC(6fcb5732) SHA1(a5a62f35b775230c62ca55ed4497e8cb9e17c17d) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchs )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 176A, 1999-??-??
	ROM_LOAD( "ww176a.bin", 0x8000, 0x8000, CRC(509d0355) SHA1(62922423f868563acaa3eb637f8edb2755a5fcf6) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitcht )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 177A, 1999-??-??
	ROM_LOAD( "ww177a.bin", 0x8000, 0x8000, CRC(c6761e20) SHA1(2bda4218c46c9fbc5719f7d3ade225faeec43d33) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchu )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 179A, 2000-05-10
	ROM_LOAD( "ww179a.bin", 0x8000, 0x8000, CRC(ffcb48c0) SHA1(829c81b8c057a2fa95b8656d77c93899bae5c892) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


ROM_START( wldwitchv )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Ver 183A, 2001-06-13
	ROM_LOAD( "ww183a.bin", 0x8000, 0x8000, CRC(2929b9b2) SHA1(3afe58b5619e818911ee142edce6a5a1468a1f97) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "03.a3",  0x00000, 0x8000, CRC(ae474414) SHA1(6dee760cee18e125791c17b562ca8aabe1f4593e) )
	ROM_LOAD( "02.a2",  0x10000, 0x8000, CRC(f6450111) SHA1(8b44c90c62d5026ccfba88b31e1113e01c6bcf85) )
	ROM_LOAD( "01.a1",  0x08000, 0x8000, CRC(6d644987) SHA1(26243abe051f3266e2d1743ec599d4e8bbb692e4) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of 03.a3 - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of 01.a1 - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of 02.a2 - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of 03.a3 - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of 01.a1 - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of 02.a2 - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of 03.a3 - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of 01.a1 - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of 02.a2 - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of 03.a3 - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wldwitch_tbp.bin",   0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )

	ROM_REGION( 0x0400, "proms2", 0 )
	ROM_LOAD( "82s137.box", 0x0000, 0x0400, CRC(4ae3ecf5) SHA1(e1e540ae13e7ce5ac6391f325160ec997ea6cc2f) )
ROM_END


/************************************************

  Witch Up & Down sets...
  (1988, Video Klein)

  16 gfx banks!!!.
  an insane work :)

************************************************/

ROM_START( wupndown )   // Witch Up & Down (Export, 6T/12T ver 1.02)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "updown_19981024.bin",    0x8000, 0x8000, CRC(cea0dcbd) SHA1(732ec0c60529f4a4a6c3d7a9bfcab741f3cb4787) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "updown1.bin",    0x00000, 0x8000, CRC(c37aad3e) SHA1(1c957838a0d50bb8a5808a58c87d22dfc13c645d) )
	ROM_LOAD( "updown2.bin",    0x08000, 0x8000, CRC(47cdd068) SHA1(fe641c66915153ae6e8e5492c225157cbd02bd4c) )
	ROM_LOAD( "updown3.bin",    0x10000, 0x8000, CRC(905c3224) SHA1(6356f2bd8a1f8952b186dc6f9ed1705d1e918a64) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of updown1.bin - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of updown2.bin - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of updown3.bin - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of updown1.bin - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of updown2.bin - upper-left box tiles, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of updown3.bin - upper-left box tiles, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of updown1.bin - upper-left box tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of updown2.bin - giant 'Video Klein' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of updown3.bin - giant 'Video Klein' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of updown1.bin - giant 'Video Klein' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of updown2.bin - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of updown3.bin - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of updown2.bin - 'Up & Down' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of updown3.bin - 'Up & Down' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of updown1.bin - 'Up & Down' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of updown2.bin - D-UP ladder tiles, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of updown3.bin - D-UP ladder tiles, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - D-UP ladder tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "updown_tbp.bin", 0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wupndowna )  // Witch Up & Down (Export, 6T/12T ver 0.99, set 1)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "updown_19980409.bin",    0x8000, 0x8000, CRC(f0f0da16) SHA1(06d89881347d9bf2a09734ec4d405ff1c9fea0a8) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "updown1.bin",    0x00000, 0x8000, CRC(c37aad3e) SHA1(1c957838a0d50bb8a5808a58c87d22dfc13c645d) )
	ROM_LOAD( "updown2.bin",    0x08000, 0x8000, CRC(47cdd068) SHA1(fe641c66915153ae6e8e5492c225157cbd02bd4c) )
	ROM_LOAD( "updown3.bin",    0x10000, 0x8000, CRC(905c3224) SHA1(6356f2bd8a1f8952b186dc6f9ed1705d1e918a64) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of updown1.bin - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of updown2.bin - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of updown3.bin - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of updown1.bin - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of updown2.bin - upper-left box tiles, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of updown3.bin - upper-left box tiles, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of updown1.bin - upper-left box tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of updown2.bin - giant 'Video Klein' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of updown3.bin - giant 'Video Klein' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of updown1.bin - giant 'Video Klein' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of updown2.bin - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of updown3.bin - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of updown2.bin - 'Up & Down' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of updown3.bin - 'Up & Down' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of updown1.bin - 'Up & Down' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of updown2.bin - D-UP ladder tiles, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of updown3.bin - D-UP ladder tiles, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - D-UP ladder tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "updown_tbp.bin", 0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wupndownb )  // Witch Up & Down (Export, 6T/12T ver 0.99, set 2)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5099.bin",   0x8000, 0x8000, CRC(b0e9ac64) SHA1(31496ddea75cf9512661f049ddab50ebdb518a44) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "updown1.bin",    0x00000, 0x8000, CRC(c37aad3e) SHA1(1c957838a0d50bb8a5808a58c87d22dfc13c645d) )
	ROM_LOAD( "updown2.bin",    0x08000, 0x8000, CRC(47cdd068) SHA1(fe641c66915153ae6e8e5492c225157cbd02bd4c) )
	ROM_LOAD( "updown3.bin",    0x10000, 0x8000, CRC(905c3224) SHA1(6356f2bd8a1f8952b186dc6f9ed1705d1e918a64) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of updown1.bin - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of updown2.bin - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of updown3.bin - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of updown1.bin - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of updown2.bin - upper-left box tiles, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of updown3.bin - upper-left box tiles, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of updown1.bin - upper-left box tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of updown2.bin - giant 'Video Klein' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of updown3.bin - giant 'Video Klein' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of updown1.bin - giant 'Video Klein' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of updown2.bin - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of updown3.bin - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of updown2.bin - 'Up & Down' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of updown3.bin - 'Up & Down' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of updown1.bin - 'Up & Down' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of updown2.bin - D-UP ladder tiles, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of updown3.bin - D-UP ladder tiles, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - D-UP ladder tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "updown_tbp.bin", 0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wupndownc )  // Witch Up & Down (Export, 6T/12T ver 0.99, set 3)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5099alt.bin",    0x8000, 0x8000, CRC(2355afbd) SHA1(f511375ca12dc71497601ca5b0a74deb0977d85e) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "updown1.bin",    0x00000, 0x8000, CRC(c37aad3e) SHA1(1c957838a0d50bb8a5808a58c87d22dfc13c645d) )
	ROM_LOAD( "updown2.bin",    0x08000, 0x8000, CRC(47cdd068) SHA1(fe641c66915153ae6e8e5492c225157cbd02bd4c) )
	ROM_LOAD( "updown3.bin",    0x10000, 0x8000, CRC(905c3224) SHA1(6356f2bd8a1f8952b186dc6f9ed1705d1e918a64) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of updown1.bin - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of updown2.bin - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of updown3.bin - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of updown1.bin - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of updown2.bin - upper-left box tiles, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of updown3.bin - upper-left box tiles, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of updown1.bin - upper-left box tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of updown2.bin - giant 'Video Klein' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of updown3.bin - giant 'Video Klein' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of updown1.bin - giant 'Video Klein' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of updown2.bin - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of updown3.bin - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of updown2.bin - 'Up & Down' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of updown3.bin - 'Up & Down' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of updown1.bin - 'Up & Down' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of updown2.bin - D-UP ladder tiles, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of updown3.bin - D-UP ladder tiles, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - D-UP ladder tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "updown_tbp.bin", 0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wupndownd )  // Witch Up & Down (Export, 6T/12T ver 0.99T)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5099t.bin",  0x8000, 0x8000, CRC(a722c659) SHA1(41f43133992b012754577d788745ff8223d88077) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "updown1.bin",    0x00000, 0x8000, CRC(c37aad3e) SHA1(1c957838a0d50bb8a5808a58c87d22dfc13c645d) )
	ROM_LOAD( "updown2.bin",    0x08000, 0x8000, CRC(47cdd068) SHA1(fe641c66915153ae6e8e5492c225157cbd02bd4c) )
	ROM_LOAD( "updown3.bin",    0x10000, 0x8000, CRC(905c3224) SHA1(6356f2bd8a1f8952b186dc6f9ed1705d1e918a64) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of updown1.bin - char rom, bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of updown2.bin - regular cards gfx, bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of updown3.bin - regular cards gfx, bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of updown1.bin - regular cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of updown2.bin - upper-left box tiles, bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of updown3.bin - upper-left box tiles, bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of updown1.bin - upper-left box tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of updown2.bin - giant 'Video Klein' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of updown3.bin - giant 'Video Klein' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of updown1.bin - giant 'Video Klein' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of updown2.bin - extended cards gfx, bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of updown3.bin - extended cards gfx, bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of updown1.bin - extended cards gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of updown2.bin - 'Up & Down' logo tiles, bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of updown3.bin - 'Up & Down' logo tiles, bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of updown1.bin - 'Up & Down' logo tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of updown2.bin - D-UP ladder tiles, bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of updown3.bin - D-UP ladder tiles, bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of updown1.bin - D-UP ladder tiles, bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of updown2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of updown3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of updown1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "updown_tbp.bin", 0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


/*********************************************

  Witch Strike (Export, 6T/12T ver 1.01A)
  Witch Strike (Export, 6T/12T ver 1.01B)

  1992, Video Klein.   Prototypes??

*********************************************/

ROM_START( wstrike )    // Witch Strike (Export, 6T/12T ver 1.01A)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wstrike_101a.bin",   0x8000, 0x8000, CRC(1e5a1c5c) SHA1(f6dcfae0f860196983378327864a9271e7d0b21f) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wsrom1.bin", 0x00000, 0x8000, CRC(006ad9cf) SHA1(2c4f2faeb9b9c268b79f3890aad5d421ecf9f58a) )
	ROM_LOAD( "wsrom2.bin", 0x08000, 0x8000, CRC(5030609b) SHA1(f51ad4bc450e94f40cf714842a5e992900220030) )
	ROM_LOAD( "wsrom3.bin", 0x10000, 0x8000, CRC(62692e92) SHA1(534a64abba4dabefa2fa1d2dfed0dc8a00d95156) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of wsrom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of wsrom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of wsrom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of wsrom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of wsrom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of wsrom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of wsrom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of wsrom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of wsrom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of wsrom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of wsrom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of wsrom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of wsrom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of wsrom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of wsrom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of wsrom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of wsrom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of wsrom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of wsrom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wstrike_tbp.bin",    0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wstrikea )   // Witch Strike (Export, 6T/12T ver 1.01B)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wstrike_101b.bin",   0x8000, 0x8000, CRC(52be1662) SHA1(42c9377b3af54d5e9373b17884ae8f841edc34de) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wsrom1.bin", 0x00000, 0x8000, CRC(006ad9cf) SHA1(2c4f2faeb9b9c268b79f3890aad5d421ecf9f58a) )
	ROM_LOAD( "wsrom2.bin", 0x08000, 0x8000, CRC(5030609b) SHA1(f51ad4bc450e94f40cf714842a5e992900220030) )
	ROM_LOAD( "wsrom3.bin", 0x10000, 0x8000, CRC(62692e92) SHA1(534a64abba4dabefa2fa1d2dfed0dc8a00d95156) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of wsrom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of wsrom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of wsrom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of wsrom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of wsrom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of wsrom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of wsrom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of wsrom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of wsrom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of wsrom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of wsrom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of wsrom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of wsrom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of wsrom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of wsrom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of wsrom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of wsrom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of wsrom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of wsrom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of wsrom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of wsrom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of wsrom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wstrike_tbp.bin",    0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


/*********************************************

   Witch Jack sets...
   1992-1996 Video Klein

*********************************************/

ROM_START( wtchjack )   // Witch Jack 0.87-89 / 1996-10-08, GFX OK
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5089.bin",   0x8000, 0x8000, CRC(91e37ca4) SHA1(abce8447550165547cd3c89dffb41c5394a2c901) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjacka )  // Witch Jack 0.87-88 / 1996-10-02, GFX OK
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5088.bin",   0x8000, 0x8000, CRC(08143537) SHA1(ef16531bcf955daded1be406dd3f61f37070298c) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjackb )  // Witch Jack 0.87 / 1996-07-16, GFX OK
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5087.bin",   0x8000, 0x8000, CRC(b027d8a5) SHA1(f06b92cc7aacadca1b8c98bda19efa670596891c) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjackc )  // Witch Jack 0.70S / 1996-03-26
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x570s.bin",   0x8000, 0x8000, CRC(294e3ac6) SHA1(e58c38ae341dcb48572f8852ec858ef8433c6f95) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjackd )  // Witch Jack 0.70P / 1996-03-26
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x570p.bin",   0x8000, 0x8000, CRC(d77d8119) SHA1(c8999e3c55257750f27f1683f5b416e8e4e4297d) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjacke )  // Witch Jack 0.65 / 1995-10-19
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x565p.bin",   0x8000, 0x8000, CRC(20c23876) SHA1(ca2309766a68ba280d71b3b62e00d38d70b8536a) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjackf )  // Witch Jack 0.64 / 1995-09-13
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x564p.bin",   0x8000, 0x8000, CRC(7ee61b69) SHA1(313b750a7949f4d08cdf79c068d01ed91fc66dce) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjackg )  // Witch Jack 0.62 / 1995-08-02
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x563.bin",    0x8000, 0x8000, CRC(55e74382) SHA1(af0a890c55db36e8a2f847ea103858cc7b7115be) )  //

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjackh )  // Witch Jack 0.40T / 1995-02-27, Not Working
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5t40.bin",   0x8000, 0x8000, CRC(a3a4f1d3) SHA1(16dcaf880134f10152703cb3ca81dfcbe48bff8b) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjacki )  // Witch Jack 0.40 / 1995-02-27
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5040.bin",   0x8000, 0x8000, CRC(2f0f1d7a) SHA1(048d0641a4e03a77f4964898ad2c224cb487aa36) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


ROM_START( wtchjackj )  // Witch Jackpot 0.25 / 1994-11-24
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wj5x5015.bin",   0x8000, 0x8000, CRC(bc0e6b78) SHA1(29482035f6122385447009abd695ed52fc669158) )

	ROM_REGION( 0x18000, "temp", 0 )
	ROM_LOAD( "wj5x5_1zs.bin",  0x00000, 0x8000, BAD_DUMP CRC(a780ba7f) SHA1(dde75187df298392333cfe1a19beed5b9d172aad) )
	ROM_LOAD( "wj5x5_2zs.bin",  0x08000, 0x8000, BAD_DUMP CRC(0f4e9f82) SHA1(a22bbbf0130dd6ece61189ce81a3376213617509) )
	ROM_LOAD( "wj5x5_3zs.bin",  0x10000, 0x8000, BAD_DUMP CRC(708e1d7f) SHA1(518312fd0bc24d7895eae0cfa9dbad99e1adf67c) )

	ROM_REGION( 0x1800, "gfx0", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // 0000-07ff of rom1.bin - char rom (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "temp",   0x08800, 0x0000, 0x0800 )   // 0800-0fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x10800, 0x0800, 0x0800 )   // 0800-0fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x01000, 0x1000, 0x0800 )   // 1000-17ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x08000, 0x0000, 0x0800 )   // 0000-07ff of rom2.bin - regular pin gfx (placed ok), bitplane 1
	ROM_COPY( "temp",   0x10000, 0x0800, 0x0800 )   // 0000-07ff of rom3.bin - regular pin gfx (placed ok), bitplane 2
	ROM_COPY( "temp",   0x00800, 0x1000, 0x0800 )   // 0800-0fff of rom1.bin - regular pin gfx (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "temp",   0x0c000, 0x0000, 0x0800 )   // 4000-47ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x14000, 0x0800, 0x0800 )   // 4000-47ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x04800, 0x1000, 0x0800 )   // 4800-4fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "temp",   0x09800, 0x0000, 0x0800 )   // 1800-1fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x11800, 0x0800, 0x0800 )   // 1800-1fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x02000, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx5", 0 )
	ROM_COPY( "temp",   0x0a800, 0x0000, 0x0800 )   // 2800-2fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x12800, 0x0800, 0x0800 )   // 2800-2fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x03000, 0x1000, 0x0800 )   // 3000-37ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx6", 0 )
	ROM_COPY( "temp",   0x0a000, 0x0000, 0x0800 )   // 2000-27ff of rom2.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x12000, 0x0800, 0x0800 )   // 2000-27ff of rom3.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x02800, 0x1000, 0x0800 )   // 2800-2fff of rom1.bin - 'Video Klein' logo lower tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx7", 0 )
	ROM_COPY( "temp",   0x0e800, 0x0000, 0x0800 )   // 6800-6fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16800, 0x0800, 0x0800 )   // 6800-6fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07000, 0x1000, 0x0800 )   // 7000-77ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx8", 0 )
	ROM_COPY( "temp",   0x0b800, 0x0000, 0x0800 )   // 3800-3fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x13800, 0x0800, 0x0800 )   // 3800-3fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x04000, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx9", 0 )
	ROM_COPY( "temp",   0x0c800, 0x0000, 0x0800 )   // 4800-4fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x14800, 0x0800, 0x0800 )   // 4800-4fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x05000, 0x1000, 0x0800 )   // 4000-47ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx10", 0 )
	ROM_COPY( "temp",   0x09000, 0x0000, 0x0800 )   // 1000-17ff of rom2.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 1
	ROM_COPY( "temp",   0x11000, 0x0800, 0x0800 )   // 1000-17ff of rom3.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 2
	ROM_COPY( "temp",   0x01800, 0x1000, 0x0800 )   // 1800-1fff of rom1.bin - extended pin gfx and logo upper tiles (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx11", 0 )
	ROM_COPY( "temp",   0x0d000, 0x0000, 0x0800 )   // 5000-57ff of rom2.bin - empty (placed ok), bitplane 1
	ROM_COPY( "temp",   0x15000, 0x0800, 0x0800 )   // 5000-57ff of rom3.bin - empty (placed ok), bitplane 2
	ROM_COPY( "temp",   0x05800, 0x1000, 0x0800 )   // 5800-5fff of rom1.bin - empty (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx12", 0 )
	ROM_COPY( "temp",   0x0d800, 0x0000, 0x0800 )   // 5800-5fff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x15800, 0x0800, 0x0800 )   // 5800-5fff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06000, 0x1000, 0x0800 )   // 6000-67ff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx13", 0 )
	ROM_COPY( "temp",   0x0e000, 0x0000, 0x0800 )   // 6000-67ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x16000, 0x0800, 0x0800 )   // 6000-67ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x06800, 0x1000, 0x0800 )   // 6800-6fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x1800, "gfx14", 0 )
	ROM_COPY( "temp",   0x0b000, 0x0000, 0x0800 )   // 3000-37ff of rom2.bin - garbage (placed ok), bitplane 1
	ROM_COPY( "temp",   0x13000, 0x0800, 0x0800 )   // 3000-37ff of rom3.bin - garbage (placed ok), bitplane 2
	ROM_COPY( "temp",   0x03800, 0x1000, 0x0800 )   // 3800-3fff of rom1.bin - garbage (placed ok), bitplane 3

	ROM_REGION( 0x1800, "gfx15", 0 )
	ROM_COPY( "temp",   0x0f000, 0x0000, 0x0800 )   // 7000-77ff of rom2.bin - empty, bitplane 1
	ROM_COPY( "temp",   0x17000, 0x0800, 0x0800 )   // 7000-77ff of rom3.bin - empty, bitplane 2
	ROM_COPY( "temp",   0x07800, 0x1000, 0x0800 )   // 7800-7fff of rom1.bin - empty, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "wjack_tbp.bin",  0x0000, 0x0100, BAD_DUMP CRC(ed15125b) SHA1(56fc00f2ce4ebe9cee73a45b142c33c00432b66b) )
ROM_END


/******************************* FALCONS WILD SETS *******************************/


/*********************************************

    Falcons Wild - Wild Card 1991.
    1992-1992 TVG D-6310 GRUENBERG.
    (bootleg in real Bonanza hardware).

  dm74s287n.7d     FIXED BITS (0000xxxx)
  fw1.2a           BADADDR     x-xxxxxxxxxxx
  fw2.4a           BADADDR     x-xxxxxxxxxxx
  fw3.5a           1ST AND 2ND HALF IDENTICAL
  nosticker.12a    x0xxxxxxxxxxxxxx = 0x00

  fw3.5a is bitrotten.

**********************************************/
ROM_START( falcnwld )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Falcons Wild
	ROM_LOAD( "nosticker.12a",  0x0000, 0x10000, CRC(54ae4a8a) SHA1(0507098b53d807059b78ec098203d095d19028f8) )

	ROM_REGION( 0x6000, "temp", 0 )
	ROM_LOAD( "fw1.2a", 0x0000, 0x2000, CRC(d5a58098) SHA1(9c8860949b0adcd20222e9b3e3e8e7e864e8f39f) )  // cards deck gfx, bitplane1
	ROM_LOAD( "fw2.4a", 0x2000, 0x2000, CRC(b28b7759) SHA1(513229cee451f59f824b7a64932679f91fbb324d) )  // cards deck gfx, bitplane2
	ROM_LOAD( "fw3.5a", 0x4000, 0x2000, BAD_DUMP CRC(98edfc82) SHA1(e3dd597245b55c3bc6ea86acf80ee024ca28f564) )  // chars + cards deck gfx, bitplane3

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x4000, 0x1000, 0x0800 )    // first quarter of fw3.5a

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x0000, 0x0000, 0x0800 )    // first quarter of fw1.2a
	ROM_COPY( "temp",   0x2000, 0x0800, 0x0800 )    // first quarter of fw2.4a
	ROM_COPY( "temp",   0x4800, 0x1000, 0x0800 )    // second quarter of fw3.5a

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dm74s287n.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/************************************

  Falcon's Wild - World Wide Poker.
  1991, Video Klein

  CPU BOX

************************************/
ROM_START( falcnwlda )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Falcons Wild, Video Klein
	ROM_LOAD( "nmc27c256.box",  0x0000, 0x8000, CRC(a0072c55) SHA1(27b84a896ff06a423450d8f0851f42f3e8ec5466) )
	ROM_RELOAD(                 0x8000, 0x8000 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling bitplanes
	ROM_LOAD( "fw4.7a", 0x1000, 0x0800, CRC(f0517b0d) SHA1(474bcf429f2539ff1f3d7d32d259c5973ccb0234) )  // chars gfx

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "fw1.2a", 0x0000, 0x0800, BAD_DUMP CRC(229cedde) SHA1(5b6d0b900714924c7a2390151ee65f36bdb02e8b) )  // cards deck gfx, bitplane1 // sldh
	ROM_IGNORE(                 0x0800)
	ROM_LOAD( "fw2.4a", 0x0800, 0x0800, BAD_DUMP CRC(9ad3c578) SHA1(a69385a807e3270d90040c44721bfff21e95706a) )  // cards deck gfx, bitplane2 // sldh
	ROM_LOAD( "fw3.5a", 0x1000, 0x0800, BAD_DUMP CRC(87abebe5) SHA1(5950082b563718476576dbc9f45439019209493e) )  // cards deck gfx, bitplane3 // sldh

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s137f.box",   0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( falcnwldb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // World Wide Poker / 1992-11-04
	ROM_LOAD( "fw12t1_19921104.bin",    0x0000, 0x8000, CRC(8b4f8cac) SHA1(e3bcbadaa157db48a41369a3fcdba536f8ca679e) )
	ROM_RELOAD(                         0x8000, 0x8000 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling bitplanes
	ROM_LOAD( "fw4.7a", 0x1000, 0x0800, CRC(f0517b0d) SHA1(474bcf429f2539ff1f3d7d32d259c5973ccb0234) )  // chars gfx

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "fw1.2a", 0x0000, 0x0800, BAD_DUMP CRC(229cedde) SHA1(5b6d0b900714924c7a2390151ee65f36bdb02e8b) )  // cards deck gfx, bitplane1 // sldh
	ROM_IGNORE(                 0x0800)
	ROM_LOAD( "fw2.4a", 0x0800, 0x0800, BAD_DUMP CRC(9ad3c578) SHA1(a69385a807e3270d90040c44721bfff21e95706a) )  // cards deck gfx, bitplane2 // sldh
	ROM_LOAD( "fw3.5a", 0x1000, 0x0800, BAD_DUMP CRC(87abebe5) SHA1(5950082b563718476576dbc9f45439019209493e) )  // cards deck gfx, bitplane3 // sldh

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s137f.box",   0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/***********************************************

  Falcon's Wild - World Wide Poker
  1983, Falcon.

  Original Falcon PCB marked
  "831 1.1 MADE IN JAPAN"

  Same board as Witch Card (Falcon), but with
  extra RAM + ROM + encrypted 2nd CPU + AY8910.

  The encrypted 40-pin CPU is scratched,
  and seems a copy of Sega's 315-5018 Z80 based
  encryption device.

***********************************************/
ROM_START( falcnwldc )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Falcons Wild, Falcon original
//  ROM_LOAD( "nosticker.12a",  0x0000, 0x10000, CRC(54ae4a8a) SHA1(0507098b53d807059b78ec098203d095d19028f8) )
	ROM_LOAD( "4.b6",           0x3000, 0x1000, CRC(88684a8f) SHA1(5ffa0808b502e93ddcb8f13929008aec2836a773) )
	ROM_LOAD( "5.b8",           0x4000, 0x1000, CRC(aa5de05c) SHA1(98559b35c7c31a41b1818a6e60ec82f43a5d1b4a) )
	ROM_LOAD( "6-syncmod.b9",   0x5000, 0x1000, CRC(21cfa807) SHA1(ff908a5a43b3736494127539d6485648d8be1a9a) )  // ok
	ROM_LOAD( "7.b11",          0x6000, 0x1000, CRC(d63bba8e) SHA1(09902574985a945117ec22d738c94fee72e673af) )
	ROM_LOAD( "8.b13",          0x7000, 0x1000, CRC(251d6abf) SHA1(2384ae674bfbe96c19a3b66c7efa1e5e8b444f48) )  // ok

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "9.f10",  0x0000, 0x1000, CRC(22f1c52a) SHA1(6429a802e92f6b77446550a303567798a231f6d7) )  // MCU prg

	ROM_REGION( 0x6000, "temp", 0 )
	ROM_LOAD( "1.b1",   0x0000, 0x1000, CRC(fd95955d) SHA1(e5c029bc5683d06c2e5250c1271613232a058fcd) )
	ROM_LOAD( "2.b3",   0x1000, 0x0800, CRC(9ad3c578) SHA1(a69385a807e3270d90040c44721bfff21e95706a) )
	ROM_LOAD( "3.b4",   0x1800, 0x0800, CRC(d9246780) SHA1(4ceb24131ec6208b742ba80373201aa53c50732d) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 )    // filling bitplanes
	ROM_COPY( "temp",   0x0800, 0x1000, 0x0800 )    // second half of 1.b1

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "temp",   0x1800, 0x0000, 0x0800 )    // first half of 3.b4
	ROM_COPY( "temp",   0x1000, 0x0800, 0x0800 )    // whole 2.b3
	ROM_COPY( "temp",   0x0000, 0x1000, 0x0800 )    // first half of 1.b1

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "falcon_1.bin",   0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/**************************************** OTHER SETS ****************************************/


ROM_START( pmpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2-5.bin",    0x5000, 0x1000, CRC(3446a643) SHA1(e67854e3322e238c17fed4e05282922028b5b5ea) )
	ROM_LOAD( "2-6.bin",    0x6000, 0x1000, CRC(50d2d026) SHA1(7f58ab176de0f0f7666d87271af69a845faec090) )
	ROM_LOAD( "2-7.bin",    0x7000, 0x1000, CRC(a9ab972e) SHA1(477441b7ff3acae3a5d5a3e4c2a428e0b3121534) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "1-4.bin",    0x1000, 0x0800, CRC(62b9f90d) SHA1(39c61a01225027572fdb75543bb6a78ed74bb2fb) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1-1.bin",    0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "1-2.bin",    0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "1-3.bin",    0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24sa10.bin",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump confirmed OK
ROM_END

/***************************************************************

  Casino Poker.
  1988, PM / Beck Elektronik.
  Ver PM88-01-21.

  1x Xtal 10.000 MHz.
  1x Unknown DIL40 CPU with sticker "23-3-88" and "BECK".
  2x HD46821P.
  1x UM6845.

  4x 2716 for graphics.
  4x 2732 for program.

  1x TBP24SA10 bipolar PROM for colors palette.

  CPU has no marks to avoid recognizement.
  Was identified as Rockwell R65C02, after exhaustive code analysis.

  The program has some protection things, as routines using extra
  opcodes to hang the program if it's running in a hardware based
  on a stock 6502 CPU.

  Graphics are different from the other Casino Poker sets.

  Bipolar PROM is different from the common in this kind of games.
  The game uses a banked palette to get green or blue background
  using each half of the palette.

***************************************************************/
ROM_START( caspoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "514.bin",    0x4000, 0x1000, CRC(4fadd660) SHA1(a06ef3e89ae09536a2f159c16726091a42430140) )
	ROM_LOAD( "515.bin",    0x5000, 0x1000, CRC(07d8b4e0) SHA1(105a1595a1a4e2d8c976ffc852636938acdd5922) )
	ROM_LOAD( "516.bin",    0x6000, 0x1000, CRC(da067462) SHA1(308368057c3126d053c89c36701be446001d34cf) )
	ROM_LOAD( "517.bin",    0x7000, 0x1000, CRC(df4c2976) SHA1(425e9f05df2e7c30422d1828c3c6471635249c7a) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x1000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "433.bin",    0x1000, 0x0800, CRC(434a7cbb) SHA1(447bf44e04d023aab8a58c3973f83a12af5b1b2b) )  // text chars

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "430.bin",  0x0000, 0x0800, CRC(46927b19) SHA1(d24c8f81bc1d34d52c759268b582a61f1455299b) )  // cards deck gfx, bitplane 1
	ROM_LOAD( "431.bin",  0x0800, 0x0800, CRC(082a5585) SHA1(580ee2a824bed4b483d88dc99793c3a06dad12e0) )  // cards deck gfx, bitplane 2
	ROM_LOAD( "432.bin",  0x1000, 0x0800, CRC(04adfcb8) SHA1(3aabbd997dec65cb5e4f044f16c742902a775e98) )  // cards deck gfx, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24sa10.bin",  0x0000, 0x0100, CRC(079d26c4) SHA1(b8adf9bdc36107f3e4f6f41f2337a8b67b70e0da) )
ROM_END

/***************************************************************

  Casino Poker
  1987, PM / Beck Elektronik
  Ver PM86LO-35-5

  Based in Golden Poker hardware.

  1x Xtal 10.000 MHz.
  1x UM6502A
  2x UM6521A
  1x UM6845

  GFX ROMS 051, 052, 053 and 054 have duplicated halves.

  Bipolar PROM was faulty, but another clone appeared with the
  same GFX set, so assume the BP is the same.

  Discrete sound circuitry was traced, being identical to the
  Golden Poker one. Only difference is the PC617 replaced by one
  PC817.

  The sound is ugly and seems that was programmed that way.

***************************************************************/
ROM_START( caspokera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "234.bin",    0x4000, 0x1000, CRC(174bc526) SHA1(faef01484f0e0ea769d7bd2c5ad03369a6fdf037) )
	ROM_LOAD( "235.bin",    0x5000, 0x1000, CRC(2e43552f) SHA1(5fbe0e62dec960850ef5f937254858fcd4da9e64) )
	ROM_LOAD( "236.bin",    0x6000, 0x1000, CRC(3f4cfa39) SHA1(e2750a9c5d12c668e599181ee3972c5d78bd0006) )
	ROM_LOAD( "237.bin",    0x7000, 0x1000, CRC(b411d0c4) SHA1(0617cd312026da78a171fc23f4788393d70371cf) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x1000, 0x0000 )  // filling the R-G bitplanes...
	ROM_LOAD( "054.bin",    0x1000, 0x0800, CRC(7b401a09) SHA1(affb90a52761c36be7c67f7606f3f982f6dc724e) )  // chars ROM
	ROM_IGNORE(                     0x0800)  // identical halves, discarding the 2nd half

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "051.bin",    0x0000, 0x0800, CRC(82d823e5) SHA1(75bdf427a6204ef87444be0d8b06a07c5a2fc38f) )  // cards deck gfx, bitplane1
	ROM_IGNORE(                     0x0800)  // identical halves, discarding the 2nd half
	ROM_LOAD( "052.bin",    0x0800, 0x0800, CRC(eda12738) SHA1(ec7806c2bf1a238f489459c3c3653f43febaa464) )  // cards deck gfx, bitplane2
	ROM_IGNORE(                     0x0800)  // identical halves, discarding the 2nd half
	ROM_LOAD( "053.bin",    0x1000, 0x0800, CRC(d147ae0a) SHA1(dfdf0a42eb0a6f2afc9f301b0cf01411085247bd) )  // cards deck gfx, bitplane3
	ROM_IGNORE(                     0x0800)  // identical halves, discarding the 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM, otherwise settings parameters are incorrect
	ROM_LOAD( "caspokera_nvram.bin", 0x0000, 0x0800, CRC(be6e2671) SHA1(aef1b09d09e07eb39480a7901ed8535f74e461fa) )

	ROM_REGION( 0x0100, "proms", 0 )  // from other games
	ROM_LOAD( "24sa10.bin", 0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Casino Poker.
  Ver. PM86-35-1.
  COPYRIGHT PM 1985,1986

  Based in Golden Poker hardware.

  COPYRIGHT 1985,1986 BECK COMPUTER,
  D-6330 WETZLAR / *DG* / PM86-35-1 / 08.12.86
*/
ROM_START( caspokerb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "215b_neu.bin",    0x5000, 0x1000, CRC(54b789e3) SHA1(96f1fa8ee3cdde338f5748bfe39b8d8fd6bafd0a) )
	ROM_LOAD( "216b_neu.bin",    0x6000, 0x1000, CRC(be7793f2) SHA1(e29dd20591d39f404e5a3bec44701aab71102846) )
	ROM_LOAD( "217b_neu.bin",    0x7000, 0x1000, CRC(9344ac66) SHA1(8735e18652e36a5fb534ebf259a195bc2b58fdf4) )

	ROM_REGION( 0x1800, "gfx1", 0 )  // borrowed from parent set.
	ROM_FILL(               0x0000, 0x1000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "054.bin",    0x1000, 0x0800, BAD_DUMP CRC(7b401a09) SHA1(affb90a52761c36be7c67f7606f3f982f6dc724e) )  // chars ROM
	ROM_IGNORE(                     0x0800)  // identical halves, discarding the 2nd half

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "b.poker_051_16.bin",  0x0000, 0x0800, CRC(598c9a21) SHA1(901c15529e0a72f750a0e64e220d27be45a2a628) )  // cards deck gfx, bitplane 1
	ROM_CONTINUE(                    0x0000, 0x0800)  // Discarding 1nd half (empty)
	ROM_LOAD( "b.poker_052_16.bin",  0x0800, 0x0800, CRC(42aa83fe) SHA1(e7d87a37993774ac1bee824ba4750b92d637ec85) )  // cards deck gfx, bitplane 2
	ROM_LOAD( "b.poker_053_16.bin",  0x1000, 0x0800, CRC(b8765304) SHA1(afdab2de3b3140e90241f3adf4069ccce95a54fd) )  // cards deck gfx, bitplane 3
	ROM_CONTINUE(                    0x1000, 0x0800)  // Discarding 1nd half (empty)

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM, otherwise settings parameters are incorrect
	ROM_LOAD( "caspokerb_nvram.bin", 0x0000, 0x0800, CRC(b0c63467) SHA1(0a031686821ce7da43816076ea498389310b98c6) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24sa10.bin",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // PROM dump confirmed OK
ROM_END

/*
  Bonus Poker.
  Copyright 1984 Galanthis Inc.
  For amusement only.

  From inside another rom:
  Copyright (C) 1983
  Zenitone Limited.
  Malcom Mailer.
  S/N 24165483.
*/
ROM_START( bonuspkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5000_bonus_12000.bin",  0x5000, 0x1000, CRC(eb93b60b) SHA1(6d6dcc0899caf1cb2179264c311bb41050834aa4) )
	ROM_LOAD( "5_6000.bin",            0x6000, 0x1000, CRC(9f444112) SHA1(cae3a092898b4cc12a4f3eac955d8c8590a7b1fd) )
	ROM_LOAD( "hn462732g.bin",         0x7000, 0x1000, CRC(6fd3bf27) SHA1(8f9a53cd20dd6198dd0ed329ac0b9b3d9a62a323) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                    0x0000, 0x1000, 0x0000 )  // filling the R-G bitplanes
	ROM_LOAD( "poke_5.4_x.bin",  0x1000, 0x0800, CRC(ca9182b1) SHA1(4c440eb53a46c4f751cf8807cbe3187ad9dbc214) )  // chars ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "bp_c1.bin",  0x0000, 0x0800, CRC(89a8c5f9) SHA1(e3aab20b2a962778a221ab96c691c37fad5b6877) )  // cards deck gfx, bitplane 1
	ROM_LOAD( "bp_c2.bin",  0x0800, 0x0800, CRC(b59a8a11) SHA1(d378ac2615ec9990903d1ad9c63f785cb65f41ab) )  // cards deck gfx, bitplane 2
	ROM_LOAD( "bp_c3.bin",  0x1000, 0x0800, CRC(cb4e8cb9) SHA1(c99f685f2f2ef4d360b48449e9520b40df58b3ae) )  // cards deck gfx, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "74s387.b",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // color PROM

	ROM_REGION( 0x0100, "proms2", 0 )
	ROM_LOAD( "74s387.a",  0x0000, 0x0100, CRC(6cc491b9) SHA1(0a97518d11bfc9eb2b8a70ed6c75882209e1abd7) )  // decode?
ROM_END


ROM_START( royale )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "royalex.bin",    0x4000, 0x4000, CRC(ef370617) SHA1(0fc5679e9787aeea3bc592b36efcaa20e859f912) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "royalechr.bin",  0x2000, 0x1000, CRC(b1f2cbb8) SHA1(8f4930038f2e21ca90b213c35b45ed14d8fad6fb) )    // chars ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "royale3.bin",    0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    // cards deck gfx, bitplane1
	ROM_LOAD( "royale2.bin",    0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "royale1.bin",    0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( royalea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "royal.256",  0x0000, 0x8000, CRC(9d7fdb79) SHA1(05cae00bca0f6ae696c69f531cb0fa2104ff696a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "royalechr.bin",  0x2000, 0x1000, CRC(b1f2cbb8) SHA1(8f4930038f2e21ca90b213c35b45ed14d8fad6fb) )    // chars ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "royale3.bin",    0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    // cards deck gfx, bitplane1
	ROM_LOAD( "royale2.bin",    0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "royale1.bin",    0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END


ROM_START( sloco93 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "locoloco.128",   0x4000, 0x4000, CRC(f626a770) SHA1(afbd33b3f65b8a781c716a3d6e5447aa817d856c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "confloco.032",   0x2000, 0x1000, CRC(b86f219c) SHA1(3f655a96bcf597a271a4eaaa0acbf8dd70fcdae9) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( sloco93a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "locoloco.256",   0x0000, 0x8000, CRC(ab037b0b) SHA1(16f811daaed5bf7b72549db85755c5274dfee310) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                   0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "confloco.032",   0x2000, 0x1000, CRC(b86f219c) SHA1(3f655a96bcf597a271a4eaaa0acbf8dd70fcdae9) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*
    checksum routine at $5f3e
    protect $4000+ & $7ff9.
    (see cmp at $5f6b)
    balanced at $7ff8.
*/
ROM_START( maverik )
	ROM_REGION( 0x10000, "maincpu", 0 ) // maverik: Maverik (ind arg, fixed, changed logo)
	ROM_LOAD( "maverik.bin",    0x0000, 0x8000, CRC(65a986e9) SHA1(2e825d3fb2346036357af0e12d3a75b5ef6cfd0d) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4s.bin", 0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*****************************

  Brasil XX sets...

 ****************************/
ROM_START( brasil86 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // brasil86.128: Brasil 86, BS clone.
	ROM_LOAD( "brasil86.128",   0x4000, 0x4000, CRC(0e88b434) SHA1(80f921c277f4253c29ee80e9cfb046ade1f66300) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4s.bin", 0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( brasil87 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // brasil87.128: Brasil 87, BS clone.
	ROM_LOAD( "brasil87.128",   0x4000, 0x4000, CRC(6cfdaea9) SHA1(0704e61c1c573e99e130c22787b529ac5544c631) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4s.bin", 0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( brasil89 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // brasil89.128: Brasil 89, BS clone.
	ROM_LOAD( "brasil89.128",   0x4000, 0x4000, CRC(9030e0db) SHA1(d073ed0ddd3e5df6a3387e10e05ca34bc491eb35) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4s.bin", 0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( brasil89a )
	ROM_REGION( 0x10000, "maincpu", 0 ) // brasil89a.128: Brasil 89a, BS clone.
	ROM_LOAD( "brasil89a.128",  0x4000, 0x4000, CRC(41a93a99) SHA1(70eeaddbdd9d3a587d1330b81d21d881ab0a8c91) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4s.bin", 0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( brasil93 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // brasil93.128: Brasil 93, BS clone. No lights
	ROM_LOAD( "brasil93.128",   0x4000, 0x4000, CRC(cc25909f) SHA1(635184022bcb8936c396cb9fcfa6367fcae906fb) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4s.bin", 0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

ROM_START( poker91 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // bs_pok91.bin: Poker 91. Based on witchcrd
	ROM_LOAD( "bs_pok91.bin",   0x0000, 0x8000, CRC(90c88b45) SHA1(9b5842075ece5f96a6869d7a8c874dee2b2abde2) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4s.bin", 0x2000, 0x1000, CRC(0ac197eb) SHA1(fdf2b134c662f3c4d4a19d93a82d130ba643ace8) )    // chars ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x1000, CRC(28ecfaea) SHA1(19d73ed0fdb5a873447b46e250ad6e71abe257cd) )    // cards deck gfx, bitplane1
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(eeec8862) SHA1(ae03aba1bd43c3ffd140f76770fc1c8cf89ea115) )    // cards deck gfx, bitplane2
	ROM_LOAD( "5.bin",  0x2000, 0x1000, CRC(2712f297) SHA1(d3cc1469d07c3febbbe4a645cd6bdb57e09cf504) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.9c",      0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END


/****************************************************

  Genie (Video Fun Games Ltd.)
  Skill game. Only for amusement.

  PCB is a heavily modified Golden Poker hardware.
  Silkscreened "ICP-1".

  CPU:   1x SY6502.
  Video: 1x HD6845P CRTC.
  I/O:   2x HD6821P PIAs.

  Sound: Discrete.

  RAMs:  2x M5L5101LP-1.

  ROMs:  2x 2732 for program. (2m.16a, 3m.17a)
         1x 2716 for char gen. (4.8a)
         3x 2716 for gfx bitplanes. (1.4a, 2.6a, 3.7a)

  1x Reset switch. (R.SW)
  1x 8 DIP switches bank.
  1x 2x10 Edge connector. (GM1)
  1x 2x22 Edge connector. (GM2)


*****************************************************/
ROM_START( genie )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2m.16a", 0x2000, 0x1000, CRC(30df75f5) SHA1(0696fb3db0b9927e6366db7316d605914ff8d464) )
	ROM_LOAD( "3m.17a", 0x3000, 0x1000, CRC(9d67f5c9) SHA1(d3bc13ce07a7b1713544756d7723dd0bcd59cd1a) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4.8a",   0x1000, 0x0800, CRC(1cdd1db9) SHA1(1940c6654b4a892abc3e4557666d341f407ac54f) )  // chars gfx

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1.4a",   0x0000, 0x0800, CRC(40c52b9d) SHA1(64145bd2aa19b584fa56022303dc595320952c24) )  // tiles, bitplane1
	ROM_LOAD( "2.6a",   0x0800, 0x0800, CRC(b0b61ffa) SHA1(d0a01027bd6acd7c72eb5bbdb37d6dd97df8aced) )  // tiles, bitplane2
	ROM_LOAD( "3.7a",   0x1000, 0x0800, CRC(151e4af7) SHA1(a44feaa69a00a6db31c018267b8b67a248e7c66e) )  // tiles, bitplane3

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM, otherwise the game isn't stable
	ROM_LOAD( "genie_nvram.bin", 0x0000, 0x0800, CRC(1b062ae7) SHA1(9d01635f3968d4b91b4a5d9fadfaf6edd0dea7ba) )

	ROM_REGION( 0x0100, "proms", 0 )    // using original golden poker color prom
	ROM_LOAD( "n82s129.9c", 0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

// Unknown on Blue PCB ICP-1
// In fact alt set of Genie...
ROM_START( geniea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732.16a", 0x2000, 0x1000, CRC(c96ef87b) SHA1(a67f290d13fbe33dc7c29271be6f5ef0ec13e927) )
	ROM_LOAD( "2732.17a", 0x3000, 0x1000, CRC(dcbfc29b) SHA1(a512b4bd4ab682810d8c432cca03f4320df9928b) )
	ROM_LOAD( "2732.15a", 0x7000, 0x1000, CRC(7137aa06) SHA1(1a2af7dfe41e54fc9c3b4e641319d1a504e84a18) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "2732.9a",    0x2000, 0x1000, BAD_DUMP CRC(ffb7bca3) SHA1(b58175c0342f963cb42a04195e296db952e071b6) )    // chars + bitplane3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "2716.4a",   0x0000, 0x0800, CRC(40c52b9d) SHA1(64145bd2aa19b584fa56022303dc595320952c24) )  // tiles, bitplane1
	ROM_LOAD( "2716.6a",   0x0800, 0x0800, CRC(b0b61ffa) SHA1(d0a01027bd6acd7c72eb5bbdb37d6dd97df8aced) )  // tiles, bitplane2
	ROM_COPY( "gfx1",      0x2800, 0x1000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the chars rom

	ROM_REGION( 0x0100, "proms", 0 )    // using original golden poker color prom
	ROM_LOAD( "n82s129.9c", 0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/****************************************************

  Silver Game.
  1983.

  6502 CPU
  2*6821 PIAs
  MC6845 CRTC
  10mhz xtal.

  PCB is similar to Prodel, with internal PSU and
  30x2 edge connector.

  There are French strings related to the game, so maybe is
  a leftover, or maybe there is a unknown way to switch the
  language.


*****************************************************/
ROM_START( silverga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "14", 0x5000, 0x1000, CRC(e4691878) SHA1(376c3910030f27517d798aac759553d5634b8ffc) )
	ROM_LOAD( "55", 0x6000, 0x2000, CRC(aad57b3c) SHA1(9508026c1a7b227a70d89ad2f7245e75a615b932) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "13",     0x2000, 0x1000, CRC(98b8cb4f) SHA1(420ea544a41e24478a8eb1c7076f4569607d0379) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "11",     0x0000, 0x0800, CRC(1f41c541) SHA1(00df5079193f78db0617a6b8a613d8a0616fc8e9) )    // cards deck gfx, bitplane1
	ROM_LOAD( "12",     0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x2800, 0x1000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd half of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "s287",       0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/****************************************************

  Super Double (Karateco)

  French text with some intentional typos to fit size.
  Uses both 0x2000..0x3fff and 0x7000..0x7fff ROM range.

  This is either the game advertised as "The Double",
  or a successor thereof.

*****************************************************/
ROM_START( superdbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sd6",  0x7000, 0x1000, CRC(3cf1ccb8) SHA1(c589ddf2e97abb9d95375d0964fd0aa6f7e2e468) )
	ROM_LOAD( "sd7",  0x2000, 0x1000, CRC(f5136f82) SHA1(f086cd5495097ede037ea6cae584e95bfcd7b239) )
	ROM_LOAD( "8",    0x3000, 0x1000, CRC(157332c2) SHA1(3c66200c49641b9d876c5fa134dd2f0e80136beb) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                 0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4",    0x1000, 0x0800, CRC(1e1d4e33) SHA1(22831984489fdf712ca616c1af3c874a5b12b522) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1",    0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "2",    0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "3",    0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24sa10n.7d",      0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/****************************************************

  Unknown poker game, set 1.
  198?.

  There are French strings related to the game into
  the program ROM.

  The dump lacks of 1 program ROM located at 17a.
  (empty socket in the PCB picture)

*****************************************************/
ROM_START( pokerdub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15a_f_83.15a",   0x2000, 0x1000, CRC(06571884) SHA1(6823f5d4a2fc5adf51f1588273f808a2a25a15bc) )
	ROM_LOAD( "unknown.17a",    0x3000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "4.8a",   0x1000, 0x0800, CRC(1e1d4e33) SHA1(22831984489fdf712ca616c1af3c874a5b12b522) )    // char ROM (cracked title)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1.4a",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "2.6a",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "3.7a",   0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "s287.8c",    0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/****************************************************

  Witch Card (ICP1 board)
  198?.

  This one is totally encrypted.
  The PCB has a daughterboard coated with some plastic
  or epoxy resin.

  Char ROM is identical to the Witch Card one.

*****************************************************/
ROM_START( witchcrdj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c",  0x2000, 0x1000, CRC(b35b4108) SHA1(6504ba55511637334c65e88ee5c60b1503b854b3) )
	ROM_LOAD( "d",  0x3000, 0x1000, CRC(c48096ed) SHA1(279ba433369c7dc9cd902a19200e889eea45d115) )
	ROM_LOAD( "b",  0x7000, 0x1000, CRC(8627fba5) SHA1(b94665f0bf425ff71f78c1258f910323c2a948f0) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "a.8a",   0x1000, 0x0800, CRC(c70a3e49) SHA1(eb2f15b344f4dec5f05701415848c854bb27aaa3) )    // chars ROM (cracked title)

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "1.4a",   0x0000, 0x0800, CRC(f2f94661) SHA1(f37f7c0dff680fd02897dae64e13e297d0fdb3e7) )    // cards deck gfx, bitplane1
	ROM_LOAD( "2.6a",   0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "3.7a",   0x1000, 0x0800, CRC(232374f3) SHA1(b75907edbf769b8c46fb1ebdb301c325c556e6c2) )    // cards deck gfx, bitplane3
	ROM_IGNORE(                 0x0800) // identical halves

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM, otherwise the game is not working
	ROM_LOAD( "witchcdj_nvram.bin", 0x0000, 0x0800, CRC(39766c6a) SHA1(9de4c5886d1ee12898f7d3b0224ab99d49e5e43d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10.9c",    0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*

  Bonne Chance!
  This PCB came with PIAs 6821 for IO

  Color system seems to pass the BP data through
  gates, latches or PLDs and get finally inverted.

  Cards GFX are similar to Golden Poker ones,
  but the back cards GFX are different...

  debug: bp 5042
            5f63

*/
ROM_START( bchancep )   // Bonne Chance! with PIAs 6821
	ROM_REGION( 0x3000, "gfx", 0 )
	ROM_LOAD( "84.bin",  0x0000, 0x1000, CRC(31f8104e) SHA1(b99f79019517ca90c48e9f303f41256d68faea91) )     // cards deck gfx bitplane 3, identical halves
	ROM_LOAD( "85.bin",  0x1000, 0x1000, CRC(40e426af) SHA1(7e7cb30dafc96bcb87a05d3e0ef5c2d426ed6a74) )     // cards deck gfx bitplane 2, identical halves
	ROM_LOAD( "87.bin",  0x2000, 0x1000, CRC(79c3578a) SHA1(9f25749f59385e6b80684ba9d4b218eb2a546e8c) )     // 1st half text layer, 2nd half cards deck gfx bitplane 1

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "88.bin",  0x5000, 0x1000, CRC(c617b037) SHA1(55b58496d12dc8bcaa252e8ee847dbcb7d2c417d) )
	ROM_LOAD( "89.bin",  0x6000, 0x1000, CRC(15599de0) SHA1(5e7a87dded97ce7829759ed9524809241526b6d8) )
	ROM_LOAD( "90.bin",  0x7000, 0x1000, CRC(86690685) SHA1(b8a8039b58f2cdfce77266ac523d87b0d627f213) )
//  ROM_LOAD( "ups39_12a.bin",  0x0000, 0x8000, CRC(216b45fb) SHA1(fbfcd98cc39b2e791cceb845b166ff697f584add) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(            0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_COPY( "gfx",     0x2000, 0x1000, 0x0800 )                                                           // chars and cards logo

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "gfx",     0x0000, 0x0000, 0x0800 )                                                           // cards deck gfx, bitplane 1
	ROM_COPY( "gfx",     0x1000, 0x0800, 0x0800 )                                                           // cards deck gfx, bitplane 2
	ROM_COPY( "gfx",     0x2800, 0x1000, 0x0800 )                                                           // cards deck gfx, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bchancep_bp.bin", 0x0000, 0x0100, CRC(70fe1582) SHA1(118c743d445a37ad760e4163b61c3c562d7adda6) )
ROM_END

ROM_START( bchanceq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pok4-5.014",  0x4000, 0x4000, CRC(92f1f515) SHA1(f79ab453458f71d7e62e895d04a2d6161dbf2aad) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(           0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "pok3.014",  0x1000, 0x0800, CRC(fb00e263) SHA1(879660bc3a3eb3d41f80741b157cdefaa7bd9a18) )    // chars ROM, different cardback logo

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pok0.014",  0x0000, 0x0800, CRC(124f131f) SHA1(35b18d1d6b0146ecc5b52f3222a270c6b868742a) )    // cards deck gfx, bitplane1
	ROM_LOAD( "pok1.014",  0x0800, 0x0800, CRC(6bbb1e2d) SHA1(51ee282219bf84218886ad11a24bc6a8e7337527) )    // cards deck gfx, bitplane2
	ROM_LOAD( "pok2.014",  0x1000, 0x0800, CRC(6e3e9b1d) SHA1(14eb8d14ce16719a6ad7d13db01e47c8f05955f0) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.bin", 0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( boasorte )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic32",  0x4000, 0x4000, CRC(ef0f1e65) SHA1(6a11722ca8089bb57d4e5648266c0f7de9a46303) )

	ROM_REGION( 0x14000, "gfx", 0 )
	ROM_LOAD( "ic34", 0x0000, 0x4000, CRC(6f23f224) SHA1(243617b9e1050b404020ea581c3e2acf8e5cca81) )    // chars ROM, different cardback logo
	ROM_LOAD( "ic15", 0x4000, 0x8000, CRC(9b5a50ca) SHA1(07ab334421dfc119939314b7026a60132b02a054) )    // cards deck gfx, bitplane1
	ROM_LOAD( "ic24", 0xC000, 0x8000, CRC(805f1a73) SHA1(a2f275de377db5dd3b493d10572ac13d5a48c50f) )    // cards deck gfx, bitplane2

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL( 0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_COPY( "gfx", 0x0000,  0x1000, 0x0800 )

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "gfx", 0x8000,  0x0000, 0x0800 )
	ROM_COPY( "gfx", 0xC000,  0x0800, 0x0800 )
	ROM_COPY( "gfx", 0x0800,  0x1000, 0x0800 )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.bin", 0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*

  PCB marked "MONDIAL"
  REV 2.1 - ALG

  Sticker APP:
  Copie prohibee.
  loi du 3 7 1985.

  1x UM6502.
  1x UM6845.
  2x ST EF6821P.

  2x UM6116-2L
  1x MK48202B-25 Zeropower RAM.

  1x M27C256 (MBV BI)
  1x TMS27C256 (3M)
  2x TMS27C128 (1M. 2M)

  1X Bipolar PROM.

  1x LM555CN.
  1x MAXIM MAX691CPE.
  1x 8 DIP switches bank.
  1x Xtal 10 MHz.

  The program ROM contains 2 different programs
  in banks of 0x4000 each.

*/
ROM_START( pokermon )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 2 programs, selectable via DIP switch
	ROM_LOAD( "mbv_bi.bin",      0x0000, 0x8000, CRC(da00e08a) SHA1(98e52915178e29ab3ae674e6b895da14626d3dd8) )

	ROM_REGION( 0x18000, "gfx", 0 )
	ROM_LOAD( "1m.bin",  0x00000, 0x4000, CRC(1b9e73ef) SHA1(fc9b67ab4c233a7e8ec8dc799732884f74166db0) )
	ROM_LOAD( "2m.bin",  0x08000, 0x4000, CRC(c51ace9b) SHA1(af84324c097beb0fefa54ccb7807c5ebb9acdcc3) )
	ROM_LOAD( "3m.bin",  0x10000, 0x8000, CRC(b2237068) SHA1(ece4f089776bbd5224c63c6a41a2e86a5e89d0c5) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                 0x0000, 0x1000, 0x0000 )   // filling the R-G bitplanes
	ROM_COPY( "gfx", 0x14800, 0x1000, 0x0800 )      // chars, numbers and soccer ball tiles

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "gfx", 0x0000, 0x0000, 0x0800 )   // soccer player gfx, bitplane 1
	ROM_COPY( "gfx", 0x08000, 0x0800, 0x0800 )  // soccer player gfx, bitplane 2
	ROM_COPY( "gfx", 0x12000, 0x1000, 0x0800 )  // soccer player gfx, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb.bin",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( pokersis )
	ROM_REGION( 0x10000, "maincpu", 0 ) // seems  to contains 4 selectable programs, but vectors lack of sense
	ROM_LOAD( "gsub1.bin",      0x0000, 0x10000, BAD_DUMP CRC(d585dd64) SHA1(acc371aa8c6c9d1ae784e62eae9c90fd05fad0fc) )

	ROM_REGION( 0x18000, "gfx", 0 )
	ROM_LOAD( "gs1.bin",  0x00000, 0x8000, CRC(47834a0b) SHA1(5fbc7443fe22ebb35a2449647259dc06420ba3fd) )
	ROM_LOAD( "gs2.bin",  0x08000, 0x8000, CRC(e882a2cc) SHA1(97819db7cef02a60ed689bb8c0c074807c08dc40) )
	ROM_LOAD( "gs3.bin",  0x10000, 0x8000, CRC(12c37991) SHA1(e63a0504e697daddcdfcf90b2a136c4180a431a7) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(                 0x0000, 0x1000, 0x0000 )   // filling the R-G bitplanes
	ROM_COPY( "gfx", 0x14800, 0x1000, 0x0800 )      // text and suppossed 1bpp gfx

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "gfx", 0x04000, 0x0000, 0x0800 )  // cards gfx, bitplane 1
	ROM_COPY( "gfx", 0x0c000, 0x0800, 0x0800 )  // cards gfx, bitplane 2
	ROM_COPY( "gfx", 0x14000, 0x1000, 0x0800 )  // cards gfx, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "n82s129n.bin",  0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Super 98',
  running in the ICP-1 boardset.

  Please read the 'Games Notes' section
  for game and debug notes / issues...
*/
ROM_START( super98 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c256.17a",   0x0000, 0x8000, CRC(dfa319c5) SHA1(e1b2ef40350ee1f40272604cbe33b245210de003) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "2732.9a",    0x2000, 0x1000, CRC(9a478c39) SHA1(614171fa3184f6ceb663d5650d05fac4d4025c9f) )    // char ROM

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "2732.4a",  0x0000, 0x1000, CRC(733b72f0) SHA1(b9255b9de24d9bd7277b18d8d1e12c7cdd3813fb) )    // cards deck gfx, bitplane1
	ROM_LOAD( "2732.6a",  0x1000, 0x1000, CRC(02595bcf) SHA1(5d01baed66152cca4b7a14fdfee83f31304e3be3) )    // cards deck gfx, bitplane2
	ROM_LOAD( "2732.7a",  0x2000, 0x1000, CRC(42072981) SHA1(1cfbbfe33afc6f147ce5828d96455f5aeb090cd3) )    // cards deck gfx, bitplane3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bipolar_prom.bin",  0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) ) // PROM dump needed
ROM_END

/*-------------------------------------------------------------------------

  Unknown rocket/animals themed banked game.

  The game looks like a poker game using rockets instead of cards.

  But the game also has graphics tiles for 4 characters:

  - pig
  - duck
  - donkey
  - mouse

  Duck and mouse are very close to Disney's characters Donald Duck
  and Mickey Mouse.

  The PCB has a lot of wire-hacks doing a weird banking,
  and other unknown things (maybe addressing scramble).
  These hacks need to be documented.

  Bottom and Top programs ROMs are soldered one over the other, and a wire
  hack enable/disable each set through the CE pin.

---------------------------------------------------------------------------

  * First game program:

  NMI vector is OK ($29FD).
  RES & IRQ/BRK vectors are pointing to 0x2EE3 and $3065 (JMP $2EE3)
  $2EE3 --> JMP $FBBB (where is in middle of a routine. Not the real start)

---------------------------------------------------------------------------

  * Second game program:

  Expects the string '#2D' placed in offset FAh-FCh (NVRAM)

  fill FAh = 0x23
       FBh = 0x32
       FCh = 0x44

       #2D

  ...to pass the checks at $638c: JSR $6760

  Another odd thing:

  bp 6394

  639f: lda #$20
  63a1: sta $a0
  63a3: lda $a0   \
  63a5: bne $63a3 / ---> loop waiting for register $a0 cleared!

-------------------------------------------------------------------------*/
ROM_START( animpkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732_bottom.15a",  0x2000, 0x1000, CRC(036f7639) SHA1(7d548dd71692fcde41c260a4a59ccdfa2aa5b07e) )
	ROM_LOAD( "2732_bottom.17a",  0x3000, 0x1000, CRC(92c19e72) SHA1(034d077ede5608160ba882227e981751a5dde26d) )
	ROM_LOAD( "2732_top.15a",     0x6000, 0x1000, CRC(ef6b36ff) SHA1(2ca520502ce32c4327f9bcc85d5c7b6e2f22eeb5) )
	ROM_LOAD( "2732_top.17a",     0x7000, 0x1000, CRC(13fae924) SHA1(c1c92fdb6e7036e6d9349c9b017e9daf3577345b) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_FILL(             0x0000, 0x1000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "2716.8a",  0x1000, 0x0800, CRC(21c9c7f1) SHA1(daa0eddd4f4a9eec0cff3aebe884792adf830238) )    // char ROM

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "2716.4a",  0x0000, 0x0800, CRC(0b7f11a2) SHA1(c5e347a377e307d12b2b2d2edf7c48be21ef5cdb) )    // characters gfx, bitplane 1
	ROM_LOAD( "2716.5a",  0x0800, 0x0800, CRC(0b2c3c25) SHA1(c69b15c1cea9abc437b12211bc4087d4d5baf084) )    // characters gfx, bitplane 2
	ROM_LOAD( "2716.7a",  0x1000, 0x0800, CRC(c48d17b0) SHA1(7c446339ab3aaa49004780fa90a3624b5a382cb1) )    // characters gfx, bitplane 3

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bprom.bin", 0x0000, 0x0100, BAD_DUMP CRC(dc4c4728) SHA1(6c779cd32d5b8d659f971b30f63267d81ad57afb) ) // PROM dump needed
ROM_END


/******************************************

  MEGA DOUBLE POKER
  BLITZ SYSTEM INC.

  Conversion kit for Golden Poker boards.

******************************************/

ROM_START( megadpkr )
	ROM_REGION( 0x10000, "maincpu", 0 ) // program ROM
	ROM_LOAD( "mega-2.u2",  0x8000, 0x8000, CRC(2b133b92) SHA1(97bc21c42897cfd13c0247e239aebb18f73cde91) )

	ROM_REGION( 0x8000, "cpubank", 0 ) // banked through MCU
	ROM_LOAD( "mega-3.u3",  0x0000, 0x8000, CRC(ff0a46c6) SHA1(df053c323c0e2dd0e41e22286d38e889bfda3aa5) )

	ROM_REGION( 0x0800, "mcu", 0 )  // 68705P5 microcontroller
	ROM_LOAD( "mega-1.u11",  0x0000, 0x0800, CRC(621a7971) SHA1(49121f7b0d428a825ccd219622dcc4abe3572968) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x000000 ) // filling the R-G bitplanes
	ROM_LOAD( "car1.5a",    0x2000, 0x1000, CRC(29e244d2) SHA1(c309a5ee6922bf2752d218c134edb3ef5f808afa) )    // chars / cards deck gfx, bitplane 3

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "car3.2a",    0x0000, 0x1000, CRC(819c06c4) SHA1(45b874554fb487173acf12daa4ff99e49e335362) )    // cards deck gfx, bitplane1
	ROM_LOAD( "car2.4a",    0x1000, 0x1000, CRC(41eec680) SHA1(3723f66e1def3908f2e6ba2989def229d9846b02) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x2800, 0x2000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the chars rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "m3-7611-5.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Manufacturer : Blitz system
  Game name :    Mega Double Poker
  Platform  :    Bonanza golden poker interface

  BoardID
  BO-BL-01

  Protection:    U11  MC68705P5S  microcontroller with window

  Main CPU:
  U6  UM6502
  U5  MK48T02B-15   time/clock backup RAM

  U2.bin  27C256 ROM
  U3.bin  27C256 ROM

  Graphics IC
  car1_5a.bin  27C32 ROM
  car2_4a.bin  27C32 ROM
  car3_2a.bin  27C32 ROM

  note : MC68705P5S is protected
*/

ROM_START( megadpkrb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // program ROM
	ROM_LOAD( "u2.bin", 0x8000, 0x8000, CRC(0efdf472) SHA1(4b1ae10427c2ae8d7cbbe525a6b30973372d4420) )

	ROM_REGION( 0x8000, "cpubank", 0 ) // banked through MCU
	ROM_LOAD( "u3.bin", 0x0000, 0x8000, CRC(c973e345) SHA1(aae9da8cbaf0cf07086e5acacf9052e49fbdd896) )

	ROM_REGION( 0x0800, "mcu", 0 )  // 68705P5 microcontroller - might not be for this set
	ROM_LOAD( "mega-1.u11",  0x0000, 0x0800, CRC(621a7971) SHA1(49121f7b0d428a825ccd219622dcc4abe3572968) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(               0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "car1_5a.bin",    0x2000, 0x1000, CRC(29e244d2) SHA1(c309a5ee6922bf2752d218c134edb3ef5f808afa) )    // chars / cards deck gfx, bitplane 3

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "car3_2a.bin",    0x0000, 0x1000, CRC(819c06c4) SHA1(45b874554fb487173acf12daa4ff99e49e335362) )    // cards deck gfx, bitplane1
	ROM_LOAD( "car2_4a.bin",    0x1000, 0x1000, CRC(41eec680) SHA1(3723f66e1def3908f2e6ba2989def229d9846b02) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x2800, 0x2000, 0x0800 )    // cards deck gfx, bitplane 3. found in the 2nd quarter of the chars rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "m3-7611-5.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

/*
  Maxi Draw Poker
  Blitz System Inc.

  Ver 1.8

*/
ROM_START( maxidpkr )
	ROM_REGION( 0x10000, "maincpu", 0 ) // program ROM
	ROM_LOAD( "maxi-2_1.8.u4",  0x8000, 0x8000, CRC(98981016) SHA1(a655cd9528d5e3bf40034b4e65f50b91f7c4c59c) )

	ROM_REGION( 0x8000, "cpubank", 0 ) // banked through MCU
	ROM_LOAD( "maxi-3_1.8.u3",  0x0000, 0x8000, CRC(3fc6eae7) SHA1(81709db8c744406846d279be2b5cbb7c3ec60896) )

	ROM_REGION( 0x0800, "mcu", 0 )  // 68705P5 microcontroller, borrowed from parent
	ROM_LOAD( "mega-1.u11",  0x0000, 0x0800, BAD_DUMP CRC(621a7971) SHA1(49121f7b0d428a825ccd219622dcc4abe3572968) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                0x0000, 0x2000, 0x000000 ) // filling the R-G bitplanes
	ROM_LOAD( "car-1.a5",    0x2000, 0x1000, CRC(e2b97357) SHA1(606c2d0abfd235866fa5f3e9178f72ab91422103) )    // chars / cards deck gfx, bitplane 3

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "car-3.a2",    0x0000, 0x1000, CRC(819c06c4) SHA1(45b874554fb487173acf12daa4ff99e49e335362) )    // cards deck gfx, bitplane1
	ROM_LOAD( "car-2.a4",    0x1000, 0x1000, CRC(41eec680) SHA1(3723f66e1def3908f2e6ba2989def229d9846b02) )    // cards deck gfx, bitplane2
	ROM_COPY( "gfx1",   0x2800, 0x2000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the chars rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "m3-7611-5.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/*
  Super 21.

  ROMs stickered with "SE Paradise".
  PCB stickered with "Paradais" and etched "SUPN-072"

  1x Scratched CPU (seems 6502 family)
  1x GI AY-3-8910.
  2x Hitachi HD6821P.
  1x IC CIC8645BE (looks like a 6845 CRTC variant)

  3x 27256 EPROMs.
  3x N82S129AN Bipolar PROMs.

  2x HM6116LP-3 SRAM.

  1x 10.000 MHz. Xtal.
  2x 8 DIP switches bank.

  Edge connector adapter from 2x 22+10 ro 2x 32+10, with optoisolators
  and one big relay, with unknown purposes. Etched "PS Public Softwear".

*/

ROM_START( super21p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13.ic4", 0xa000, 0x2000, CRC(6f414354) SHA1(290e97b876ce7aa9e273fe5f597caaa2e31992a8) ) // ok
	ROM_CONTINUE(       0x8000, 0x2000) // ok
	ROM_CONTINUE(       0xe000, 0x2000) // ok
	ROM_CONTINUE(       0xc000, 0x2000) // ok

	ROM_REGION( 0x6000, "gfxpool", 0 )
	ROM_LOAD( "1.ic10", 0x0000, 0x4000, CRC(40bb114e) SHA1(be4636455c6dd303255d21799cd17c590d8f1423) ) // identical halves, 2 bitplanes.
	ROM_IGNORE( 0x4000) // discarding the 2nd half.
	ROM_LOAD( "2.ic9", 0x4000, 0x2000, CRC(51c08823) SHA1(123dab7485cac23ee1d72fd50e4af273c946fc56) ) // identical halves, 1 bitplane.
	ROM_IGNORE( 0x6000) // discarding the 2nd half and the unused 0x2000.

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_COPY( "gfxpool", 0x0000, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x2000, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x4000, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_COPY( "gfxpool", 0x0800, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x2800, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x4800, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_COPY( "gfxpool", 0x1000, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x3000, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x5000, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_COPY( "gfxpool", 0x1800, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x3800, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x5800, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129_1.ic31", 0x0000, 0x0100, CRC(c3d777b4) SHA1(5a3c0325dcbddde3f8ae2ffbc1cb56cfccda308d) )
	ROM_LOAD( "82s129_2.ic30", 0x0100, 0x0100, CRC(c9c12b13) SHA1(e0b26febb265af01f2caa891e14f4999400820b8) )
	ROM_LOAD( "82s129_3.ic29", 0x0200, 0x0100, CRC(f079b80c) SHA1(c76706ad90a67ea7eda4e191840f95e18f3788d0) )
ROM_END


/********************************************************

  Le Super Pendu.
  Voyageur de L'Espace Inc.

  French language hangman style game manufactured in Canada.
  Conversion kit for modified Bonanza Enterprises hardware.

  Prequel of "Le Pendu" game.

  For more details, see the lependu.cpp driver.


********************************************************/

ROM_START( lespendu )  // board #1
	ROM_REGION( 0x10000, "maincpu", 0 ) // program ROMs
	ROM_LOAD( "pendu_2.12a",  0x6000, 0x1000, CRC(bd1c4763) SHA1(7f33b9866afcc456e3623e478abee77e1610d99b) )
	ROM_LOAD( "pendu_3.14a",  0x7000, 0x1000, CRC(4c973c2d) SHA1(edaf488019fc1b72b9344488b898a27ef886c1f9) )

	ROM_REGION( 0x08000, "data", 0 ) // banked data
	ROM_LOAD( "pendu_1.11a",  0x0000, 0x8000, CRC(acaecf36) SHA1(290867f18c5376189d389e6e508b34637c726352) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                  0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "pendu_c.5a",    0x2000, 0x1000, CRC(c7cfc375) SHA1(d46b633ef007d0928fbd49b9703ab5248de8d545) )    // chars / other gfx, bitplane 3

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "pendu_a.2a",    0x0000, 0x0800, CRC(43770310) SHA1(e6c0d1d1b07a4c14fa26cfb4aedf1f94017bc9c3) )    // other gfx, bitplane2
	ROM_LOAD( "pendu_b.3a",    0x0800, 0x0800, CRC(19471258) SHA1(d4e2cf05e00945e034f968dcc314cc8e5832b840) )    // other gfx, bitplane1
	ROM_COPY( "gfx1",  0x2800, 0x1000, 0x0800 )    // other gfx, bitplane3. found in the 2nd half of the chars rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tbp24s10n.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END

ROM_START( lespenduj )  // board #2
	ROM_REGION( 0x10000, "maincpu", 0 ) // program ROMs
	ROM_LOAD( "pendu_jeje.12a",    0x6000, 0x1000, CRC(71d0430b) SHA1(6b6e5e3a3f5c51809953189e53191a175b03cfb3) )
	ROM_LOAD( "pendu_jeje_3.14a",  0x7000, 0x1000, CRC(8e1863a1) SHA1(0cfdd961fbc83219ee8f0e62432320a90c8296bf) )

	ROM_REGION( 0x08000, "data", 0 ) // banked data
	ROM_LOAD( "nosticker.11a",  0x0000, 0x8000, CRC(0a9853a3) SHA1(b9dabfd5b13b6ddddc2d8b266adc6e55f094e981) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_FILL(                      0x0000, 0x2000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "1a_pendu_jeje.5a",  0x2000, 0x1000, CRC(c7cfc375) SHA1(d46b633ef007d0928fbd49b9703ab5248de8d545) )    // chars / other gfx

	ROM_REGION( 0x1800, "gfx2", 0 )
	ROM_LOAD( "2a_pendu_jeje.4a",    0x0000, 0x0800, CRC(19471258) SHA1(d4e2cf05e00945e034f968dcc314cc8e5832b840) )    // other gfx
	ROM_LOAD( "3a_pendu_jeje.2a",    0x0800, 0x0800, CRC(2aefe346) SHA1(6540f126777c942737aeffd7f6f356ef2a71e146) )    // other gfx
	ROM_COPY( "gfx1",        0x2800, 0x1000, 0x0800 )    // other gfx

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "pk.7d",   0x0000, 0x0100, CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )
ROM_END


/*
  Olympic Games R6511 CPU encrypted system.

  Wild Card (v2.0)
  Black jack (v5.04)

  Since we currently haven't a R6511 core, and due to the encrypted program ROMs,
  I decided to hook the games here till can get some improvements. GFX decode seems OK.
  Will see if it's necessary to move these games to a different driver later...

  There are tiles for "Wild Card" title.
  Both sets use the same GFX ROMs.

  Wild Card only uses the main board.
  Black Jack is an upgrade kit for Wild Card and also uses the I/O board.
  Black Jack has a hopper tied to the I/O board.

  The IO Card also controls hard meters, coin input, door switch/optic
  and button switch's BUT Black Jack is wired to the edge connector for
  buttons still and IO card section for switch's is not used.


  Main Board:
  -----------

  CPU:         R6511AQ (U19)
  Video CRTC:  MC6845P (U2)
  Sound:       AY8910 (U1)

  PRG ROMs:    3x 27C128 (U34, U35, U36)
  GFX ROMs:    2x 2732 (but they used 27C64 and tied the not used pins to +5V)(U27 and U28)
               1x 27C64 (U29)

  RAM:         2x 6116 (U21, U22)
  NVRAM:       Dallas 48Z02 (U40)

  Bipolar PROMs:  1x 74S288 (U10)
                  1x 74S472 (U45)

  PLDs:        2x PAL16L8 (U4, U5)
               2x PAL10L8 (U7, U8)

  Xtal 10 MHz.


  IO Board:
  ---------

  CPU:         R6511AQ (U25)
  I/O ROM:     1x 27C512 (U26)

  NVRAM:       Dallas 48Z02 (U27)

  I/O devices: 2x PIAs 6821 (U23, U24)
  Sound:       AY8910 (U19)

  PLDs:        1x PAL16L8 (U5)

  Xtal 4.0 MHz.

*/
ROM_START( olym65wc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wild_card_u34_v2.0__27c128.u34", 0x2000, 0x4000, CRC(24e422a6) SHA1(d8e84af682a773cd913c88c0cf86d501e7d49290) )
	ROM_LOAD( "wild_card_u35_v2.0__27c128.u35", 0x8000, 0x4000, CRC(1a155c3c) SHA1(ec89848d7e8c60bcbb63c31b319a146131d7e678) )
	ROM_LOAD( "wild_card_u36_v2.0__27c128.u36", 0xc000, 0x4000, CRC(4e4a8bbc) SHA1(05219847b92f54af0b6e098e048265a8dbec7800) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(                                       0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "wild_card_u29_v2.0_text__27c64.u29", 0x4000, 0x2000, CRC(382a2a19) SHA1(99e9d1b7b1a7b6d8d17e677b12e2bd1a4fcd51d9) )    // char ROM, plus title and 3rd card deck bitplane
	ROM_IGNORE(                                             0x2000)         // discarding 2nd half (identical halves).

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "wild_card_v2.0_u28_gfx__2732.u28", 0x0000, 0x1000, CRC(cbf49e79) SHA1(227c2628e9d70008e3f116638e05b57184463cf3) )    // cards deck gfx, bitplane2
	ROM_LOAD( "wild_card_v2.0_u27_gfx__2732.u27", 0x1000, 0x1000, CRC(5469dcf4) SHA1(e5f8573eb6963eb63bc4d7022b8fc0d6b83a5d92) )    // cards deck gfx, bitplane1
	ROM_COPY( "gfx1",                     0x4800, 0x2000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom
	ROM_COPY( "gfx1",                     0x5800, 0x2800, 0x0800 )    // cards deck gfx, bitplane3. found in the 4th quarter of the char rom

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bprom.bin",     0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // borrowed from Golden Poker, seems to match
ROM_END

ROM_START( olym65bj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "black_jack_v5.04_hx1.u34", 0x2000, 0x4000, CRC(b1ce68da) SHA1(4ed10b7d77cd45a3233b55f852147e19313c5d22) )
	ROM_LOAD( "black_jack_v5.04_hx2.u35", 0x8000, 0x4000, CRC(d6da3199) SHA1(062595ba775b1548d9acdeeb5c44057a220a5aa0) )
	ROM_LOAD( "black_jack_v5.04_hx3.u36", 0xc000, 0x4000, CRC(f8b1d506) SHA1(5e4b2c20601526e3e8e76e981e1c37d535a046cb) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_FILL(                         0x0000, 0x4000, 0x0000 ) // filling the R-G bitplanes
	ROM_LOAD( "v2.0_text__27c64.u29", 0x4000, 0x2000, CRC(382a2a19) SHA1(99e9d1b7b1a7b6d8d17e677b12e2bd1a4fcd51d9) )  // char ROM, plus title and 3rd card deck bitplane
	ROM_IGNORE(                               0x2000)         // discarding 2nd half (identical halves).

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "gfx__2732.u28", 0x0000, 0x1000, CRC(cbf49e79) SHA1(227c2628e9d70008e3f116638e05b57184463cf3) )    // cards deck gfx, bitplane2
	ROM_LOAD( "gfx__2732.u27", 0x1000, 0x1000, CRC(5469dcf4) SHA1(e5f8573eb6963eb63bc4d7022b8fc0d6b83a5d92) )    // cards deck gfx, bitplane1
	ROM_COPY( "gfx1",                     0x4800, 0x2000, 0x0800 )    // cards deck gfx, bitplane3. found in the 2nd quarter of the char rom
	ROM_COPY( "gfx1",                     0x5800, 0x2800, 0x0800 )    // cards deck gfx, bitplane3. found in the 4th quarter of the char rom

	ROM_REGION( 0x0800, "dallas", 0 )    // original Dallas NVRAM, for reverse-engineering purposes.
	ROM_LOAD( "black_jack_v5.04__dallas.u40", 0x0000, 0x0800, CRC(64f6b4ed) SHA1(baa3451ac3b275bf4d771bc3dd14a032fe77cd1c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bprom.bin",     0x0000, 0x0100, BAD_DUMP CRC(7f31066b) SHA1(15420780ec6b2870fc4539ec3afe4f0c58eedf12) )  // borrowed from Golden Poker, seems to match
ROM_END


/*
  Open 5 Cards
  1987 MNG

  1x 6502
  1x AY8910
  2x PIAs 6821
  1x CRTC 6845

  3x 27C256 (One for program ROM, other 2 for GFX)
  2x 6116 SRAMs

  1x 82S129 Bipolar PROM

  1x PAL18L8 (Locked, tried to brute force it but didn't have much luck)

  XTAL 10.0 MHz.


  Looks like the second half of the program ROM contains an unrelated program
  using the whole CPU addressing and IRQ instead of NMI.

*/
 ROM_START( op5cards )
	ROM_REGION( 0x10000, "maincpu", 0 )
//  ROM_LOAD( "noname.ic4", 0x8000, 0x8000, CRC(af0ea127) SHA1(466de9a3e2ebe81eac30bbd9139edd71738d33d4) )  // mapping the unrelated program, to check...
	ROM_LOAD( "noname.ic4", 0xc000, 0x4000, CRC(af0ea127) SHA1(466de9a3e2ebe81eac30bbd9139edd71738d33d4) )
	ROM_IGNORE(                     0x4000)         // discarding 2nd half (the unrelated program).

//  noname.ic10  [1/2]      noname.ic9   [2/2]      IDENTICAL
//  noname.ic10  [2/2]      noname.ic9   [1/2]      IDENTICAL
//  noname.ic10  [1/4]      noname.ic9   [3/4]      IDENTICAL
//  noname.ic10  [2/4]      noname.ic9   [4/4]      IDENTICAL
//  noname.ic10  [3/4]      noname.ic9   [1/4]      IDENTICAL
//  noname.ic10  [4/4]      noname.ic9   [2/4]      IDENTICAL
//  noname.ic10  [even 1/2] noname.ic9   [even 2/2] IDENTICAL
//  noname.ic10  [odd 1/2]  noname.ic9   [odd 2/2]  IDENTICAL
//  noname.ic10  [even 2/2] noname.ic9   [even 1/2] IDENTICAL
//  noname.ic10  [odd 2/2]  noname.ic9   [odd 1/2]  IDENTICAL

	ROM_REGION( 0x10000, "gfxpool", 0 )
	ROM_LOAD( "noname.ic10", 0x0000, 0x8000, CRC(35321abc) SHA1(4abb37a9aab6ddfd94e4275de8ff6ca841923ce8) )  // chars, title and cards GFX,
	ROM_LOAD( "noname.ic9",  0x8000, 0x8000, CRC(9af786b1) SHA1(7ea5d0119abf221bc0da37783cfbc53a5c0f69d0) )  // same as IC10, but with scrambled quarters...

	ROM_REGION( 0x1800, "gfx1", 0 )  // chars
	ROM_COPY( "gfxpool", 0x0000, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x2000, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x4000, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x1800, "gfx2", 0 )  // cards
	ROM_COPY( "gfxpool", 0x0800, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x2800, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x4800, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x1800, "gfx3", 0 )  // nag, held, cards back
	ROM_COPY( "gfxpool", 0x1000, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x3000, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x5000, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x1800, "gfx4", 0 )  // title
	ROM_COPY( "gfxpool", 0x1800, 0x1000, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x3800, 0x0800, 0x0800 ) // src-dest-size
	ROM_COPY( "gfxpool", 0x5800, 0x0000, 0x0800 ) // src-dest-size

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.ic31",     0x0000, 0x0100, CRC(b4e1ccd6) SHA1(bb1ce6ff60b92886cd8689b9c9f2fdfa9b33fe09) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

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


void goldnpkr_state::init_vkdlsa()
{
	/* $e097-e098, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe097] = 0xea;
	ROM[0xe098] = 0xea;
}

void goldnpkr_state::init_vkdlsb()
{
	/* $e87b-e87c, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe87b] = 0xea;
	ROM[0xe87c] = 0xea;
}

void goldnpkr_state::init_vkdlsc()
{
	/* $453a-453b, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x453a] = 0xea;
	ROM[0x453b] = 0xea;
}

void goldnpkr_state::init_vkdlsww()
{
	/* $f2c9-f2ca, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xf2c9] = 0xea;
	ROM[0xf2ca] = 0xea;
}

void goldnpkr_state::init_vkdlswwa()
{
	/* $df80-df81, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xdf80] = 0xea;
	ROM[0xdf81] = 0xea;
}

void goldnpkr_state::init_vkdlswwc()
{
	/* $e42f-e430, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe42f] = 0xea;
	ROM[0xe430] = 0xea;
}

void goldnpkr_state::init_vkdlswwd()
{
	/* $e442-e443, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe442] = 0xea;
	ROM[0xe443] = 0xea;
}

void goldnpkr_state::init_vkdlswwh()
{
	/* $e4d5-e4d6, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe4d5] = 0xea;
	ROM[0xe4d6] = 0xea;
}

void goldnpkr_state::init_vkdlswwl()
{
	/* $e87c-e87d, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe87c] = 0xea;
	ROM[0xe87d] = 0xea;
}

void goldnpkr_state::init_vkdlswwo()
{
	/* $e7d5-e7d6, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe7d5] = 0xea;
	ROM[0xe7d6] = 0xea;
}

void goldnpkr_state::init_vkdlswwp()
{
	/* $e7d9-e7da, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe7d9] = 0xea;
	ROM[0xe7da] = 0xea;
}

void goldnpkr_state::init_vkdlswwr()
{
	/* $e7f7-e7f8, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe7f7] = 0xea;
	ROM[0xe7f8] = 0xea;
}

void goldnpkr_state::init_vkdlswws()
{
	/* $e8a5-e8a6, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe8a5] = 0xea;
	ROM[0xe8a6] = 0xea;
}

void goldnpkr_state::init_vkdlswwt()
{
	/* $e955-e956, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xe955] = 0xea;
	ROM[0xe956] = 0xea;
}

void goldnpkr_state::init_vkdlswwu()
{
	/* $ee6b-ee6c, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xee6b] = 0xea;
	ROM[0xee6c] = 0xea;
}

void goldnpkr_state::init_vkdlswwv()
{
	/* $f052-f053, NOPing the BNE-->KILL
	   after compare with Dallas TK data
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xf052] = 0xea;
	ROM[0xf053] = 0xea;
}


/***********************************************

  ICP1 Daughterboard encryption

  The PCB has a daughterboard coated with some plastic
  or epoxy resin.

***********************************************/

void goldnpkr_state::init_icp1db()
{
	uint8_t *rom = memregion("maincpu")->base();

	// apply XORs depending on address
	for (int i = 0x00000; i < 0x10000; i++)
	{
		uint8_t x = rom[i];

		switch (i & 0x58)
		{
			case 0x00: x ^= 0x00; break;
			case 0x08: x ^= 0x04; break;
			case 0x10: x ^= 0x02; break;
			case 0x18: x ^= 0x06; break;
			case 0x40: x ^= 0x01; break;
			case 0x48: x ^= 0x05; break;
			case 0x50: x ^= 0x03; break;
			case 0x58: x ^= 0x07; break;
		}

		if (i & 0x80)
			x ^= 0x40;

		rom[i] = x;
	}

	std::vector<uint8_t> buffer(0x10000);

	memcpy(&buffer[0], rom, 0x10000);

	// descramble address
	for (int i = 0; i < 0x10000; i++)
	{
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 9, 8, 11, 10, 0, 1, 2, 3, 4, 5, 6, 7)];
	}
}

/**********************************************

  Witch Strike protection

  (a default Dallas TK NVRAM should be made)

  Each time the DEAL button is pressed, the program
  do the following execution....

  C9F5:  JSR $F2D4

  and then...

  F2D4: 48            pha
  F2D5: 8A            txa
  F2D6: 48            pha
  F2D7: A2 00         ldx  #$00
  F2D9: BD EE F2      lda  $F2EE,x    ; read a char.
  F2DC: F0 08         beq  $F2E6      ; if 0 (end of string), branch to end.
  F2DE: DD E0 2F      cmp  $2FE0,x    ; compare with dallas offset.
  F2E1: D0 07         bne  $F2EA      ; if different, go to $F2EA (kill)
  F2E3: E8            inx             ; increment X-register.
  F2E4: D0 F3         bne  $F2D9      ; loop to read the next char....
  F2E6: 68            pla             ; end.
  F2E7: AA            tax
  F2E8: 68            pla
  F2E9: 60            rts

  F2EA: 02            kil  $02        ; kill.
  F2EB: 4C EA F2      jmp  $F2EA      ; just in case the 1st time fails, go to kill again.

  The following string is tested...

  F2EE:  76 69 64 65 6F 20 6B 6C 65 69 6E 20 74 65 73 74 64 61 74 61 00  |  video klein testdata.

  So nasty... and unnecessary.

***********************************************/

void goldnpkr_state::init_wstrike()
{
	/* NOPing the BNE --> $F2EA (KILL)
	   after compare with a string inside the Dallas TK RAM
	*/

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xf2e1] = 0xea;
	ROM[0xf2e2] = 0xea;
}


void goldnpkr_state::init_bchancep()
{
	// Attempt to invert the color data from the bipolar PROM

	uint8_t *ROM = memregion("proms")->base();
	int size = memregion("proms")->bytes();
	int start = 0x0000;

	for (int i = start; i < size; i++)
	{
		ROM[i] = ROM[i] ^ 0x0f;
	}

	m_palette->update();
}


/*
  Bonus Poker protection.

  Code checks for a ROM offset value (at $FFC8) and XOR it with the next value.
  if the result is 0, the code continues. If not 0, just jump to a previous
  subroutine, getting an infinite loop...

  7B7A: A9 FF    lda #$ff         ; Set the offset pointer to $30/$31...
  7B7C: 85 31    sta $31
  7B7E: A9 19    lda #$19
  7B80: 0A       asl a
  7B81: 0A       asl a
  7B82: 0A       asl a
  7B83: 85 30    sta $30
  7B85: A0 01    ldy #$01         ; load Y with 1.
  7B87: B1 30    lda ($30), y     ; load A with the content of $FFC8,y (0xAA).
  7B89: 88       dey              ; decrement Y...
  7B8A: 51 30    eor ($30), y     ; XOR against the content of $FFC8,y (now 0x55).
  7B8C: F0 03    beq $7b91        ; if 0, jumps to $7B91 (where the game execution continues).

  7B8E: 4C 4C E0 jmp $e04c        ; jumps to an infinite loop.

  7B91: A2 1D    ldx #$1d         ; the game continue executing...

*/

void goldnpkr_state::init_bonuspkr()
{
//  NOPing the DEY, so the XOR is against the same value
//  instead of the next offset one.

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x7b89] = 0xea;
}


/*
  Super 98 protection.

  Code checks for different things on NVRAM space.
  There are involved zeropage registers, inputs
  combinations, and stack pointer handling.

  The game takes 25 seconds to do all the checks.

  E9F4: jmp $EA56   ; infinite loop / dead end.
  E9F7: jmp $CEC6   ; the game continue executing...

*/

void goldnpkr_state::init_super98()
{
//  NOPing the deadly call for now.

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x69f4] = 0xea;
	ROM[0x69f5] = 0xea;
	ROM[0x69f6] = 0xea;
}


void goldnpkr_state::init_pokersis()
{
//  bad dump fix

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x5f26] = 0x20;
	ROM[0x5f29] = 0x20;
	ROM[0x5fff] = 0x48;
	ROM[0x6001] = 0x48;
	ROM[0x7000] = 0x00;
	ROM[0x7001] = 0xa0;
	ROM[0x7002] = 0x08;
	ROM[0x7003] = 0xad;
	ROM[0x7004] = 0x48;
	ROM[0x7005] = 0x08;
	ROM[0x7007] = 0x10;
	ROM[0x700a] = 0xa0;
	ROM[0x700b] = 0x07;
	ROM[0x700c] = 0xa2;
	ROM[0x7fbf] = 0x7d;
	ROM[0x7fc0] = 0x60;
	ROM[0x7fc1] = 0xb9;
	ROM[0x7fc2] = 0x00;
	ROM[0x7fc3] = 0x00;
	ROM[0x7ffa] = 0xfd;
	ROM[0x7ffb] = 0x5f;
	ROM[0x7ffc] = 0x22;
	ROM[0x7ffd] = 0x5f;
}


void goldnpkr_state::init_lespendu()
{
	uint8_t *ROM0 = memregion("maincpu")->base();
	uint8_t *ROM1 = memregion("data")->base();

	ROM0[0x643d] = 0xff;  // skip checksum
	ROM0[0x643f] = 0xff;  // skip checksum
	ROM0[0x6461] = 0xff;  // changing value to store at $01c1 (RAM security patch)

	ROM1[0x7aa3] = 0x20;  // restore to original call, before RAM security patch
	ROM1[0x7aa4] = 0xc3;
	ROM1[0x7aa5] = 0x78;

	ROM0[0x75de] = 0x1f;  // fix lamps bug
}

void goldnpkr_state::init_lespenduj()
{
	uint8_t *ROM0 = memregion("maincpu")->base();
	uint8_t *ROM1 = memregion("data")->base();

	ROM0[0x643d] = 0xff;  // skip checksum
	ROM0[0x643f] = 0xff;  // skip checksum
	ROM0[0x6461] = 0xff;  // changing value to store at $01c1 (RAM security patch)

	ROM1[0x7aa3] = 0x20;  // restore to original call, before RAM security patch
	ROM1[0x7aa4] = 0xc8;
	ROM1[0x7aa5] = 0x78;

	ROM0[0x766c] = 0x17;  // fix lamps bug
	ROM0[0x7749] = 0x17;  // fix lamps bug
}

void goldnpkr_state::init_op5cards()
{
	/* Seems to do a sort of blend
	   between a tile bytes from 1468-146f (top left backcard edge)
	   with 7468-746f range. In this game seems to be only for this tile.
	*/

	uint8_t *ROM = memregion("gfx3")->base();

	ROM[0x1468] = 0x3f;
	ROM[0x1469] = 0x7f;
	ROM[0x146a] = 0xff;
	ROM[0x146b] = 0xf0;
	ROM[0x146c] = 0xe0;
	ROM[0x146d] = 0xe0;
	ROM[0x146e] = 0xe0;
	ROM[0x146f] = 0xe0;
}

void goldnpkr_state::init_olym65()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0x2000; i <= 0x10000; i++)
	{
		if (i & 0x01) rom[i] ^= 0x02;
		if (i & 0x02) rom[i] ^= 0x04;
		if (i & 0x04) rom[i] ^= 0x09;
		if (i & 0x08) rom[i] ^= 0x20;
		if (i & 0x10) rom[i] ^= 0x40;
		if (i & 0x20) rom[i] ^= 0x90;
	}
}


} // anonymous namespace



/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT      COMPANY                     FULLNAME                                     FLAGS             LAYOUT
GAMEL( 1981, goldnpkr,  0,        goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "Bonanza Enterprises, Ltd", "Golden Poker Double Up (Big Boy)",           0,                layout_goldnpkr )
GAMEL( 1981, goldnpkb,  goldnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "Bonanza Enterprises, Ltd", "Golden Poker Double Up (Mini Boy)",          0,                layout_goldnpkr )
GAMEL( 198?, goldnpkc,  goldnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Golden Poker Double Up (bootleg, set 1)",    0,                layout_goldnpkr )
GAMEL( 198?, goldnpkd,  goldnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Golden Poker Double Up (bootleg, set 2)",    MACHINE_NOT_WORKING, layout_goldnpkr )  // always get a winning flush
GAMEL( 1983, goldnpke,  goldnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "Intercoast (bootleg)",     "Golden Poker Double Up (bootleg, set 3)",    0,                layout_goldnpkr )
GAMEL( 1983, goldnpkf,  goldnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "Intercoast (bootleg)",     "Golden Poker Double Up (bootleg, set 4)",    0,                layout_goldnpkr )

GAMEL( 198?, videtron,  0,        goldnpkr, videtron, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Videotron Poker (cards selector, set 1)",    0,                layout_goldnpkr )
GAMEL( 198?, videtron2, videtron, goldnpkr, videtron, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Videotron Poker (cards selector, set 2)",    0,                layout_goldnpkr )
GAMEL( 198?, videtrna,  videtron, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Videotron Poker (normal controls)",          0,                layout_goldnpkr )

GAMEL( 198?, pottnpkr,  0,        pottnpkr, pottnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 1)",                0,                layout_goldnpkr )
GAMEL( 198?, potnpkra,  pottnpkr, pottnpkr, potnpkra, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 2)",                0,                layout_goldnpkr )
GAMEL( 198?, potnpkrb,  pottnpkr, pottnpkr, pottnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 3)",                0,                layout_goldnpkr )
GAMEL( 198?, potnpkrc,  pottnpkr, pottnpkr, potnpkrc, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 4)",                0,                layout_goldnpkr )
GAMEL( 198?, potnpkrd,  pottnpkr, pottnpkr, potnpkrc, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 5)",                0,                layout_goldnpkr )
GAMEL( 198?, potnpkre,  pottnpkr, pottnpkr, pottnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 6)",                0,                layout_goldnpkr )
GAMEL( 198?, potnpkrf,  pottnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 7, Royale GFX)",    0,                layout_goldnpkr )
GAMEL( 198?, potnpkrg,  pottnpkr, pottnpkr, potnpkra, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 8, Australian)",    0,                layout_goldnpkr )
GAMEL( 198?, potnpkrh,  pottnpkr, pottnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 9, 'just 4 fun')",  0,                layout_goldnpkr )
GAMEL( 198?, potnpkri,  pottnpkr, pottnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 10, ICP-1 PCB)",    0,                layout_goldnpkr )  // unencrypted IPC-1 PCB.
GAMEL( 198?, potnpkrj,  pottnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 11, German, W.W.)", 0,                layout_goldnpkr )
GAMEL( 198?, potnpkrk,  pottnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "bootleg",                  "Jack Potten's Poker (set 12, no Double-Up)", 0,                layout_goldnpkr )
GAMEL( 198?, potnpkrl,  pottnpkr, pottnpkr, potnpkra, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Jack Potten's Poker (set 13, ICP-1 PCB)",    0,                layout_goldnpkr )  // unencrypted IPC-1 PCB.
GAMEL( 198?, ngold,     pottnpkr, pottnpkr, ngold,    goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Jack Potten's Poker (NGold, set 1)",         0,                layout_goldnpkr )
GAMEL( 198?, ngolda,    pottnpkr, pottnpkr, ngold,    goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Jack Potten's Poker (NGold, set 2)",         0,                layout_goldnpkr )
GAMEL( 198?, ngoldb,    pottnpkr, pottnpkr, ngoldb,   goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Jack Potten's Poker (NGold, set 3)",         0,                layout_goldnpkr )
GAMEL( 198?, adpoker,   0,        pottnpkr, pottnpkr, goldnpkr_state, empty_init,    ROT0,   "Amstar?",                  "Amstar Draw Poker",                          0,                layout_goldnpkr )

GAMEL( 1990, bsuerte,   0,        witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 1)",              0,                layout_goldnpkr )
GAMEL( 1991, bsuertea,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 2)",              0,                layout_goldnpkr )
GAMEL( 1991, bsuerteb,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 3)",              0,                layout_goldnpkr )
GAMEL( 1991, bsuertec,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 4)",              0,                layout_goldnpkr )
GAMEL( 1991, bsuerted,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 5)",              0,                layout_goldnpkr )
GAMEL( 1991, bsuertee,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 6)",              0,                layout_goldnpkr )
GAMEL( 1991, bsuertef,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 7)",              0,                layout_goldnpkr )
GAME(  1991, bsuerteg,  bsuerte,  witchcrd, bsuertew, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 8)",              0 )
GAME(  1991, bsuerteh,  bsuerte,  witchcrd, bsuertew, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 9)",              0 )
GAMEL( 1991, bsuertei,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 10)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertej,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 11)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertek,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 12)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertel,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 13)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertem,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 14)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuerten,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 15, Prodel PCB)", 0,                layout_goldnpkr )
GAMEL( 1991, bsuerteo,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 16)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertep,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 17)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuerteq,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 18)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuerter,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 19)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertes,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 20)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertet,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 21)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuerteu,  bsuerte,  witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish, set 22)",             0,                layout_goldnpkr )
GAMEL( 1991, bsuertev,  bsuerte,  witchcrd, bsuertev, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Buena Suerte (Spanish/Portuguese, set 23)",  0,                layout_goldnpkr )
GAMEL( 1991, goodluck,  bsuerte,  witchcrd, goodluck, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Good Luck",                                  0,                layout_goldnpkr )

GAMEL( 1991, falcnwld,  0,        wildcard, wildcard, goldnpkr_state, empty_init,    ROT0,   "TVG",                      "Falcons Wild - Wild Card 1991 (TVG)",        0,                layout_goldnpkr )
GAMEL( 1990, falcnwlda, falcnwld, wildcard, wildcard, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Falcons Wild - World Wide Poker (Video Klein, set 1)", 0,      layout_goldnpkr )
GAMEL( 1990, falcnwldb, falcnwld, wildcard, wildcard, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Falcons Wild - World Wide Poker (Video Klein, set 2)", 0,      layout_goldnpkr )
GAME(  1983, falcnwldc, falcnwld, wildcrdb, wildcard, goldnpkr_state, empty_init,    ROT0,   "Falcon",                   "Falcons Wild - World Wide Poker (Falcon original)",    MACHINE_NOT_WORKING ) // MCU hook up incomplete, currently game runs only after a soft reset. Then you can coin up but bet doesn't work

GAMEL( 1991, witchcrd,  0,        witchcrd, witchcrd, goldnpkr_state, init_vkdlsc,   ROT0,   "Video Klein?",             "Witch Card (Video Klein CPU box, set 1)",    0,                   layout_goldnpkr )
GAME(  1991, witchcrda, witchcrd, witchcrd, witchcda, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Witch Card (Spanish, witch game, set 1)",    0 )
GAME(  1991, witchcrdb, witchcrd, witchcrd, witchcda, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Witch Card (Spanish, witch game, set 2)",    0 )
GAME(  1991, witchcrdc, witchcrd, witchcrd, witchcdc, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Witch Card (English, no witch game)",        0 )
GAMEL( 1994, witchcrdd, witchcrd, witchcrd, witchcdd, goldnpkr_state, empty_init,    ROT0,   "Proma",                    "Witch Card (German, WC3050, set 1 )",        0,                   layout_goldnpkr )
GAMEL( 1991, witchcrde, witchcrd, witchcrd, witchcrd, goldnpkr_state, init_vkdlsc,   ROT0,   "Video Klein",              "Witch Card (Video Klein CPU box, set 2)",    0,                   layout_goldnpkr )
GAMEL( 1985, witchcrdf, witchcrd, witchcrd, witchcdf, goldnpkr_state, empty_init,    ROT0,   "PM / Beck Elektronik",     "Witch Card (English, witch game, lamps)",    0,                   layout_goldnpkr )
GAMEL( 199?, witchcrdg, witchcrd, wcfalcon, witchcrd, goldnpkr_state, empty_init,    ROT0,   "Falcon",                   "Witch Card (Falcon, enhanced sound)",        0,                   layout_goldnpkr )
GAMEL( 1994, witchcrdh, witchcrd, witchcrd, witchcdd, goldnpkr_state, empty_init,    ROT0,   "Proma",                    "Witch Card (German, WC3050, set 2 )",        0,                   layout_goldnpkr )
GAMEL( 1994, witchcrdi, witchcrd, witchcrd, witchcdd, goldnpkr_state, empty_init,    ROT0,   "Proma",                    "Witch Card (German, WC3050, 27-4-94)",       0,                   layout_goldnpkr )
GAME(  199?, witchcrdj, witchcrd, witchcdj, witchcrd, goldnpkr_state, init_icp1db,   ROT0,   "<unknown>",                "Witch Card (ICP-1, encrypted)",              0 )

GAMEL( 1991, witchgme,  0,        witchcrd, witchcrd, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Game (Video Klein, set 1)",            0,                   layout_goldnpkr )
GAMEL( 1997, witchgmea, witchgme, witchcrd, witchcrd, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Game (Video Klein, set 2)",            MACHINE_NOT_WORKING, layout_goldnpkr )

GAME(  199?, jokercar,  witchcrd, witchcrd, witchcda, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Joker Card (witch game)",                    0 )

GAMEL( 1994, witchjol,  0,        wcrdxtnd, witchjol, goldnpkr_state, init_vkdlsa,   ROT0,   "Video Klein",              "Jolli Witch (Export, 6T/12T ver 1.57D)",     0,                   layout_goldnpkr )

GAMEL( 2001, wldwitch,  0,        wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlsww,  ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.84A)",       0,                   layout_goldnpkr )  // Ver 184A, 2001-09-12
GAMEL( 1992, wldwitcha, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwa, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.57-SP)",     0,                   layout_goldnpkr )  // Ver 157-SP, 1992-12-25
GAMEL( 1992, wldwitchb, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.57-TE)",     MACHINE_NOT_WORKING, layout_goldnpkr )  // Ver 157-TE, 1992-12-25
GAMEL( 1994, wldwitchc, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwc, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.62A)",       0,                   layout_goldnpkr )  // Ver 162A, 1994-04-26
GAMEL( 1994, wldwitchd, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwd, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.62B)",       0,                   layout_goldnpkr )  // Ver 162B, 1994-04-26
GAMEL( 1994, wldwitche, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.62A-F)",     MACHINE_NOT_WORKING, layout_goldnpkr )  // Ver 162A-F, 1994-04-26
GAMEL( 1994, wldwitchf, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwc, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.62A alt)",   0,                   layout_goldnpkr )  // Ver 162A alt, 1994-11-03
GAMEL( 1994, wldwitchg, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwd, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.62B alt)",   0,                   layout_goldnpkr )  // Ver 162B alt, 1994-11-03
GAMEL( 1995, wldwitchh, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwh, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.65A)",       0,                   layout_goldnpkr )  // Ver 165A, 1995-11-16
GAMEL( 1996, wldwitchi, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwh, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.65A-S)",     0,                   layout_goldnpkr )  // Ver 165A-S (Fast Deal), 1996-03-26
GAMEL( 1996, wldwitchj, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwh, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.65A-S alt)", 0,                   layout_goldnpkr )  // Ver 165A-S alt (Fast Deal), 1996-05-26
GAMEL( 1996, wldwitchk, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwh, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.65A-N)",     0,                   layout_goldnpkr )  // Ver 165A-N, 1996-05-29
GAMEL( 1996, wldwitchl, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwl, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.70A beta)",  0,                   layout_goldnpkr )  // Ver 170A-beta, 1996-06-25
GAMEL( 1996, wldwitchm, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwl, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.70A)",       0,                   layout_goldnpkr )  // Ver 170A, 1996-09-30
GAMEL( 1997, wldwitchn, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwl, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.70A alt)",   0,                   layout_goldnpkr )  // Ver 170A alt, 1997-06-11
GAMEL( 1998, wldwitcho, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwo, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.74A-SP-BELG)", 0,                 layout_goldnpkr )  // Ver 174A-SP-BELG (no D-UP, no payout), 1998-05-11
GAMEL( 1998, wldwitchp, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwp, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.74A)",       0,                   layout_goldnpkr )  // Ver 174A (no D-UP, no payout), 1998-09-20
GAMEL( 1998, wldwitchq, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlsb,   ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.74A alt)",   0,                   layout_goldnpkr )  // Ver 174A alt, box, 1998-09-25
GAMEL( 1999, wldwitchr, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwr, ROT0,   "Video Klein",              "Wild Witch (Export, 6B/12B ver 1.75A-E English)", 0,               layout_goldnpkr )  // Ver 175A-E (English), 1999-01-11
GAMEL( 1999, wldwitchs, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswws, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.76A)",       0,                   layout_goldnpkr )  // Ver 176A, 1999-??-??
GAMEL( 1999, wldwitcht, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwt, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.77A)",       0,                   layout_goldnpkr )  // Ver 177A, 1999-??-??
GAMEL( 2000, wldwitchu, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwu, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.79A)",       0,                   layout_goldnpkr )  // Ver 179A, 2000-05-10
GAMEL( 2001, wldwitchv, wldwitch, wcrdxtnd, wldwitch, goldnpkr_state, init_vkdlswwv, ROT0,   "Video Klein",              "Wild Witch (Export, 6T/12T ver 1.83A)",       0,                   layout_goldnpkr )  // Ver 183A, 2001-06-13

GAMEL( 1998, wupndown,  0,        wcrdxtnd, wupndown, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Up & Down (Export, 6T/12T ver 1.02)",        0,           layout_upndown )    // Ver 1.02, 1998-10-26
GAMEL( 1998, wupndowna, wupndown, wcrdxtnd, wupndown, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Up & Down (Export, 6T/12T ver 0.99, set 1)", 0,           layout_upndown )    // Ver 0.99, 1998-04-09
GAMEL( 1998, wupndownb, wupndown, wcrdxtnd, wupndown, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Up & Down (Export, 6T/12T ver 0.99, set 2)", 0,           layout_upndown )    // Ver 0.99, 1998-03-23
GAMEL( 1998, wupndownc, wupndown, wcrdxtnd, wupndown, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Up & Down (Export, 6T/12T ver 0.99, set 3)", 0,           layout_upndown )    // Ver 0.99 alt, 1998-05-11
GAMEL( 1998, wupndownd, wupndown, wcrdxtnd, wupndown, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Up & Down (Export, 6T/12T ver 0.99T)",       0,           layout_upndown )    // Ver 0.99T, 1998-03-23

GAMEL( 1992, wstrike,   0,        wcrdxtnd, wstrike,  goldnpkr_state, init_wstrike,  ROT0,   "Video Klein",              "Witch Strike (Export, 6T/12T ver 1.01A)",     0,                layout_goldnpkr )
GAMEL( 1992, wstrikea,  wstrike,  wcrdxtnd, wstrike,  goldnpkr_state, init_wstrike,  ROT0,   "Video Klein",              "Witch Strike (Export, 6T/12T ver 1.01B)",     0,                layout_goldnpkr )

GAMEL( 1996, wtchjack,  0,        wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.87-89)", 0,                          layout_goldnpkr )    // Ver 0.87-89 / 1996-10-08 GFX OK
GAMEL( 1996, wtchjacka, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.87-88)", 0,                          layout_goldnpkr )    // Ver 0.87-88 / 1996-10-02, GFX OK
GAMEL( 1996, wtchjackb, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.87)",    0,                          layout_goldnpkr )    // Ver 0.87 / 1996-07-16, GFX OK
GAMEL( 1996, wtchjackc, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.70S)",   MACHINE_IMPERFECT_GRAPHICS, layout_goldnpkr )    // Ver 0.70S / 1996-03-26
GAMEL( 1996, wtchjackd, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.70P)",   MACHINE_IMPERFECT_GRAPHICS, layout_goldnpkr )    // Ver 0.70P / 1996-03-26
GAMEL( 1995, wtchjacke, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.65)",    MACHINE_IMPERFECT_GRAPHICS, layout_goldnpkr )    // Ver 0.65 / 1995-10-19
GAMEL( 1995, wtchjackf, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.64)",    MACHINE_IMPERFECT_GRAPHICS, layout_goldnpkr )    // Ver 0.64 / 1995-09-13
GAMEL( 1995, wtchjackg, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.62)",    MACHINE_IMPERFECT_GRAPHICS, layout_goldnpkr )    // Ver 0.62 / 1995-08-02
GAMEL( 1995, wtchjackh, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.40T)",   MACHINE_NOT_WORKING,        layout_goldnpkr )    // Ver 0.40T / 1995-02-27
GAMEL( 1995, wtchjacki, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jack (Export, 6T/12T ver 0.40)",    MACHINE_IMPERFECT_GRAPHICS, layout_goldnpkr )    // Ver 0.40 / 1995-02-27
GAMEL( 1994, wtchjackj, wtchjack, wcrdxtnd, wtchjack, goldnpkr_state, empty_init,    ROT0,   "Video Klein",              "Witch Jackpot (Export, 6T/12T ver 0.25)", MACHINE_IMPERFECT_GRAPHICS, layout_goldnpkr )    // Ver 0.25 / 1994-11-24


/*************************************** OTHER SETS ***************************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT      COMPANY                     FULLNAME                                  FLAGS                LAYOUT
GAMEL( 1981, pmpoker,   0,        goldnpkr, pmpoker,  goldnpkr_state, empty_init,    ROT0,   "PM / Beck Elektronik",     "PlayMan Poker (German)",                  0,                   layout_pmpoker  )
GAMEL( 1988, caspoker,  0,        caspoker, caspoker, goldnpkr_state, empty_init,    ROT0,   "PM / Beck Elektronik",     "Casino Poker (Ver PM88-01-21, German)",   0,                   layout_pmpoker  )
GAMEL( 1987, caspokera, caspoker, goldnpkr, caspoker, goldnpkr_state, empty_init,    ROT0,   "PM / Beck Elektronik",     "Casino Poker (Ver PM86LO-35-5, German)",  0,                   layout_pmpoker  )
GAMEL( 1986, caspokerb, caspoker, goldnpkr, caspoker, goldnpkr_state, empty_init,    ROT0,   "PM / Beck Elektronik",     "Casino Poker (Ver PM86-35-1, German)",    0,                   layout_pmpoker  )
GAMEL( 198?, royale,    0,        goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Royale (set 1)",                          0,                   layout_goldnpkr )
GAMEL( 198?, royalea,   royale,   goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Royale (set 2)",                          0,                   layout_goldnpkr )
GAME(  1993, sloco93,   0,        witchcrd, sloco93,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Super Loco 93 (Spanish, set 1)",          0 )
GAME(  1993, sloco93a,  sloco93,  witchcrd, sloco93,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Super Loco 93 (Spanish, set 2)",          0 )
GAME(  198?, maverik,   0,        witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Maverik",                                 0 )
GAMEL( 1986, brasil86,  0,        witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Brasil 86",                               0,                layout_goldnpkr )
GAMEL( 1987, brasil87,  0,        witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Brasil 87",                               0,                layout_goldnpkr )
GAMEL( 1989, brasil89,  0,        witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Brasil 89 (set 1)",                       0,                layout_goldnpkr )
GAMEL( 1989, brasil89a, brasil89, witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Brasil 89 (set 2)",                       0,                layout_goldnpkr )
GAME(  1993, brasil93,  0,        witchcrd, bsuerte,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Brasil 93",                               0 )              // no lamps
GAME(  1991, poker91,   0,        witchcrd, poker91,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Poker 91",                                0 )
GAME(  198?, genie,     0,        genie,    genie,    goldnpkr_state, empty_init,    ROT0,   "Video Fun Games Ltd.",     "Genie (ICP-1, set 1)",                    0 )
GAME(  198?, geniea,    genie,    geniea,   geniea,   goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Genie (ICP-1, set 2)",                    0 )
GAMEL( 1983, silverga,  0,        goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Silver Game",                             0,                layout_goldnpkr )
GAMEL( 1984, bonuspkr,  0,        goldnpkr, bonuspkr, goldnpkr_state, init_bonuspkr, ROT0,   "Galanthis Inc.",           "Bonus Poker",                             0,                layout_goldnpkr )

GAMEL( 198?, superdbl,  pottnpkr, goldnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "Karateco",                 "Super Double (French)",                   0,                layout_goldnpkr )
GAME(  198?, pokerdub,  0,        pottnpkr, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "unknown French poker game",               MACHINE_NOT_WORKING )                // lacks of 2nd program ROM.
GAMEL( 198?, pokersis,  0,        goldnpkr, goldnpkr, goldnpkr_state, init_pokersis, ROT0,   "Sisteme France",           "Good Luck! poker (Sisteme France)",       0,                layout_goldnpkr )  // fix banking (4 prgs?)...

GAME(  1987, pokermon,  0,        mondial,  mondial,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "Mundial/Mondial (Italian/French)",        0 )  // banked selectable program.
GAME(  1998, super98,   bsuerte,  witchcrd, super98,  goldnpkr_state, init_super98,  ROT0,   "<unknown>",                "Super 98 (3-hands, ICP-1)",               0 )  // complex protection. see notes.

GAME(  198?, animpkr,   0,        icp_ext,  animpkr,  goldnpkr_state, empty_init,    ROT0,   "<unknown>",                "unknown rocket/animal-themed poker",      MACHINE_IMPERFECT_COLORS )  // banked program. how to switch gfx?

GAME(  1987, super21p,  0,        super21p, super21p, goldnpkr_state, empty_init,    ROT0,   "Public MNG",               "Super 21",                                MACHINE_IMPERFECT_COLORS )
GAME(  1987, op5cards,  0,        op5cards, op5cards, goldnpkr_state, init_op5cards, ROT0,   "MNG",                      "Open 5 Cards",                            0 )  // initialize lamps but doesn't seems to use them

GAMEL( 198?, lespendu,  0,        lespendu, lespendu, goldnpkr_state, init_lespendu, ROT0,   "Voyageur de L'Espace Inc.", "Le Super Pendu (V1, words set #1)",      0,                layout_lespendu )
GAMEL( 198?, lespenduj, 0,        lespendu, lespendu, goldnpkr_state, init_lespenduj,ROT0,   "Voyageur de L'Espace Inc.", "Le Super Pendu (V1, words set #2)",      0,                layout_lespendu )


/*************************************** SETS W/IRQ0 ***************************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT      COMPANY          FULLNAME                                             FLAGS      LAYOUT
GAMEL( 198?, bchancep,  0,        gldnirq0, goldnpkr, goldnpkr_state, init_bchancep, ROT0,   "<unknown>",     "Bonne Chance! (Golden Poker prequel HW, set 1)",     0,         layout_goldnpkr )
GAMEL( 198?, bchanceq,  0,        gldnirq0, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "<unknown>",     "Bonne Chance! (Golden Poker prequel HW, set 2)",     0,         layout_goldnpkr )
GAMEL( 198?, boasorte,  bchanceq, gldnirq0, goldnpkr, goldnpkr_state, empty_init,    ROT0,   "<unknown>",     "Boa Sorte! (Golden Poker prequel HW)",               0,         layout_goldnpkr )


/*************************************** SETS W/MCU ***************************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT      COMPANY          FULLNAME                                             FLAGS
GAME(  1990, megadpkr,  0,        megadpkr, megadpkr, blitz_state,    empty_init,    ROT0,   "Blitz System",  "Mega Double Poker (conversion kit, version 2.3 MD)", 0 )
GAME(  1990, megadpkrb, megadpkr, megadpkr, megadpkr, blitz_state,    empty_init,    ROT0,   "Blitz System",  "Mega Double Poker (conversion kit, version 2.1 MD)", 0 )
GAME(  1990, maxidpkr,  0,        megadpkr, megadpkr, blitz_state,    empty_init,    ROT0,   "Blitz System",  "Maxi Double Poker (version 1.8)",                    MACHINE_NOT_WORKING )


/*************************************** SETS W/R6511 ***************************************/

GAME(  1989, olym65wc,  0,        goldnpkr, goldnpkr, goldnpkr_state, init_olym65,   ROT0,   "Olympic Video Gaming PTY LTD", "Wild Card (Olympic Games, v2.0)",                              MACHINE_NOT_WORKING )
GAME(  1989, olym65bj,  0,        goldnpkr, goldnpkr, goldnpkr_state, init_olym65,   ROT0,   "Olympic Video Gaming PTY LTD", "Black jack (Olympic Games, v5.04, upgrade kit for Wild Card)", MACHINE_NOT_WORKING )
