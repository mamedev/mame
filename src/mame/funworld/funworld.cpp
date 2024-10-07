// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Grull Osgo, Andreas Naive, Peter Ferrie
/*********************************************************************************************

  Fun World / Amatic / TAB / Impera
  Series 7000 hardware.

  65C02/65SC02 + 2x 6821 PIAs + 6845 CRTC + AY-3-8910.
  From raw, to different ways of encryption. Even through custom CPUs.

  Also from CMC, Dino4 (encrypted), and Leopard 4.

  Driver by Roberto Fresca.
  Based on a preliminary work of Curt Coder & Peter Trauner.


**********************************************************************************************

  Games running on this hardware:

  * Jolly Card (German, set 1),                                    TAB Austria,          1985.
  * Jolly Card (German, set 2),                                    TAB Austria,          1985.
  * Jolly Card (3x3 deal),                                         TAB Austria,          1985.
  * Jolly Card Professional 2.0 (MZS Tech),                        MZS Tech,             1993.
  * Jolly Card Professional 2.0 (Spale Soft),                      Spale Soft,           2000.
  * Jolly Card (Evona Electronic),                                 Evona Electronic      1998.
  * Jolly Card (Croatian, set 1),                                  TAB Austria,          1985.
  * Jolly Card (Croatian, set 2),                                  Soft Design,          1993.
  * Super Joly 2000 - 3x,                                          M.P.                  1985.
  * Jolly Card (German, Fun World, bootleg),                       Inter Games,          1986.
  * Jolly Card (Spanish, blue TAB board, encrypted),               TAB Austria,          1992.
  * Jolly Card (Italian, different colors, set 1),                 bootleg,              1990.
  * Jolly Card (Italian, different colors, set 2),                 bootleg,              1990.
  * Jolly Card (Italian, bootleg, set 1),                          bootleg,              1990.
  * Jolly Card (Italian, bootleg, set 2),                          bootleg,              1993.
  * Jolly Card (Italian, bootleg, set 3),                          bootleg,              1990.
  * Jolly Game (V1, Otti Karl logo),                               bootleg,              1993.
  * Bonus Card (German),                                           Fun World,            1986.
  * Bonus Card (German, ATG Electronic hack),                      Fun World,            1986.
  * Big Deal (Hungarian, set 1),                                   Fun World,            1986.
  * Big Deal (Hungarian, set 2),                                   Fun World,            1986.
  * Cuore 1 (Italian, set 1),                                      C.M.C.,               1996.
  * Cuore 1 (Italian, set 2),                                      C.M.C.,               1996.
  * Cuore 1 (Italian, set 3),                                      C.M.C.,               1996.
  * Cuore 1 (Italian, set 4),                                      C.M.C.,               1996.
  * Cuore 1 (Italian, set 5),                                      C.M.C.,               1996.
  * Cuore 1 (Italian, set 6),                                      C.M.C.,               1996.
  * Cuore 1 (Italian, set 7),                                      bootleg,              1996.
  * Elephant Family (Italian, new),                                C.M.C.,               1997.
  * Elephant Family (Italian, old),                                C.M.C.,               1996.
  * Pool 10 (Italian, set 1),                                      C.M.C.,               1996.
  * Pool 10 (Italian, set 2),                                      C.M.C.,               1996.
  * Pool 10 (Italian, set 3),                                      C.M.C.,               1996.
  * Pool 10 (Italian, set 4),                                      C.M.C.,               1997.
  * Pool 10 (Italian, set 5),                                      C.M.C.,               1996.
  * Pool 10 (Italian, set 6),                                      C.M.C.,               1996.
  * Pool 10 (Italian, set 7),                                      C.M.C.,               1997.
  * Pool 10 (Italian, set 8),                                      C.M.C.,               1997.
  * Pool 10 (Italian, set 9),                                      bootleg (R.B.),       1996.
  * Pool 10 (Italian, set 10),                                     C.M.C.,               1997.
  * Magic 10 Nevada (bootleg of Pool 10),                          bootleg (Magic),      1997.
  * New Biliard 98 (Pool 10 hack),                                 bootleg,              1998.
  * Royal (Pool 10 hack),                                          unknown,              2001.
  * Tortuga Family (Italian),                                      C.M.C.,               1997.
  * Pot Game (Italian),                                            C.M.C.,               1996.
  * Bottle 10 (Italian, set 1),                                    C.M.C.,               1996.
  * Bottle 10 (Italian, set 2),                                    C.M.C.,               1996.
  * Luna Park (set 1, dual program),                               unknown,              1998.
  * Luna Park (set 2, dual program),                               unknown,              1998.
  * Luna Park (set 3),                                             unknown,              1998.
  * Crystal Colours (CMC hardware),                                J.C.D. srl,           1998.
  * Royal Card (German, set 1),                                    TAB Austria,          1991.
  * Royal Card (German, set 2),                                    TAB Austria,          1991.
  * Royal Card (German/Polish, set 3),                             TAB Austria,          1991.
  * Royal Card (German, set 4),                                    TAB Austria,          1991.
  * Royal Card (German, set 5),                                    TAB Austria,          1991.
  * Royal Card (German, set 6),                                    TAB Austria,          1991.
  * Royal Card (TAB original),                                     TAB Austria,          1991.
  * Royal Card (German, set 7, CMC C1030 HW),                      bootleg,              1991.
  * Royal Card (German, set 8),                                    TAB Austria,          1991.
  * Royal Card (French),                                           TAB Austria,          1991.
  * Royal Card v2.0 Professional,                                  Digital Dreams,       1993.
  * Royal Card / Royal Jackpot (with a third draw)(encrypted),     TAB / Video Klein,    1991
  * Witch Royal (Export version 2.1),                              Video Klein,          199?.
  * Lucky Lady (3x3 deal),                                         TAB Austria,          1991.
  * Lucky Lady (4x1 aces),                                         TAB Austria,          1991.
  * Magic Card II (Bulgarian),                                     Impera,               1996.
  * Magic Card II (Nov, Yugoslavian),                              Impera,               1996.
  * Magic Card II (green TAB or Impera board),                     Impera,               1996.
  * Magic Card II (blue TAB board, encrypted),                     Impera,               1996.
  * Royal Vegas Joker Card (Slow deal),                            Fun World,            1993.
  * Royal Vegas Joker Card (Fast deal),                            Soft Design,          1993.
  * Royal Vegas Joker Card (Fast deal, english gfx),               Soft Design,          1993.
  * Royal Vegas Joker Card (Fast deal, Mile),                      Mile,                 1993.
  * Jolly Joker (98bet, set 1).                                    Impera,               199?.
  * Jolly Joker (98bet, set 2).                                    Impera,               199?.
  * Jolly Joker (40bet, higher paytable),                          Impera,               1985.
  * Jolly Joker (40bet, lower paytable),                           Impera,               1985.
  * Jolly Joker (Apple Time),                                      Apple Time,           199?.
  * Jolly Joker (original program, interleaved GFX, Impera logo),  Impera,               199?.
  * Jolly Joker (original program, interleaved GFX, no logo),      Impera,               199?.
  * Jolly Joker (original program, interleaved GFX, Otti Karl logo, set 1), Impera,      199?.
  * Jolly Joker (original program, interleaved GFX, Otti Karl logo, set 2), Impera,      199?.
  * Jolly Joker (Solid State module in suicide board).             Impera,               199?.
  * Multi Win (Ver.0167, encrypted),                               Fun World,            1992.
  * Multi Win (Ver.0091, encrypted),                               Fun World,            1991.
  * Multi Win (EPM7032, encrypted),                                Amatic,               2001.
  * Power Card (Ver 0263, encrypted),                              Fun World,            1993.
  * Mega Card (Ver.0210, encrypted),                               Fun World,            1993.
  * Mega Card (Ver.0053, encrypted),                               Fun World,            1992.
  * Joker Card 300 (Ver.A267BC, encrypted),                        Amatic Trading,       1993.
  * Royal Card (Evona, Polish, encrypted),                         Evona Electronic,     1991.
  * Saloon (French, encrypted),                                    unknown,              199?.
  * Nevada (French, encrypted),                                    unknown,              199?.
  * Jolly Card (Italian, blue TAB board, encrypted),               bootleg,              199?.
  * Jolly Card (Italian, encrypted bootleg, set 1),                bootleg,              1990.
  * Jolly Card (Italian, encrypted bootleg, set 2),                bootleg,              1993.
  * Jolly Card (Italian, encrypted bootleg, set 3),                bootleg (Cleco),      1993.
  * Zabavni Karti (Bulgarian, cyrillic, encrypted),                bootleg (Cleco),      1993.
  * Pool 10 (Italian, Dino 4 hardware, encrypted),                 C.M.C.,               1997.
  * Royal Card (Italian, Dino 4 hardware, encrypted),              unknown,              1998.
  * China Town (Ver 1B, Dino4 HW),                                 unknown,              1998.
  * Mongolfier New (Italian),                                      unknown,              199?.
  * Soccer New (Italian),                                          unknown,              199?.
  * Fun World Quiz (German),                                       Fun World,            198?.
  * Fun World Quiz (German, 12-11-1990),                           Fun World,            1990.
  * Fun World Quiz (German, 27-04-1990),                           Fun World,            1990.
  * Novo Play Multi Card / Club Card,                              Admiral/Novomatic,    1986.
  * Novo Play Club Card (V6.2H),                                   Novo Play International, 1992.
  * Novo Play Club Card (V3.3H),                                   Novo Play International, 1991.
  * Joker Card (Inter Games),                                      Inter Games,          1991.
  * Unknown Fun World A7-11 game 1,                                Fun World,            1985.
  * Unknown Fun World A7-11 game 2,                                Fun World,            1985.
  * Unknown Fun World A0-1 game,                                   Fun World,            1991.
  * Joker Card / Multi Card (Epoxy brick CPU),                     Fun World,            1991.
  * Gratis Poker (V.204, set 1),                                   Mega Soft,            1995.
  * Gratis Poker (V.204, set 2),                                   Mega Soft,            1995.
  * Gkoulit (encrypted GFX),                                       JK Amusement,         1995.
  * Reflex Cards (English, serial protected),                      TAB Austria,          1999.
  * Club Card (ver. 1.1 English),                                  Impera / Mega Tech,   199?.
  * Royal Card (stealth with NES multigame, set 1),                bootleg,              1991.
  * Royal Card (stealth with NES multigame, set 2),                bootleg,              1991.
  * Royal Card (stealth with MSX multigame),                       bootleg,              1991.


  Supported games: 123


**********************************************************************************************

  The hardware is generally composed by:

  CPU:    1x 65SC02 or 65C02 at 2MHz.
  Sound:  1x AY-3-8910 or YM2149F (AY8910 compatible) at 2MHz.
  I/O:    2x 6821 (PIA)
  Video:  1x 6845 (CRTC)
  RAM:    1x 6116
  NVRAM:  1x 6264
  ROMs:   3x 27256 (or 27512 in some cases)
  PROMs:  1x 82S147 (or similar. 512 bytes)
  PLDs:   1 to 4 PALs, GALs or PEELs
  Clock:  1x Crystal: 16MHz.


  All current games are running from a slightly modified to heavily hacked hardware.
  Color palettes are normally stored in format GGBBBRRR inside a bipolar color PROM.

  - bits -
  7654 3210
  ---- -xxx   Red component.
  --xx x---   Blue component.
  xx-- ----   Green component.


  The hardware was designed to manage 4096 tiles with a size of 8x4 pixels each.
  Also support 4bpp graphics and the palette limitation is 8 bits for color codes (256 x 16 colors).
  It means the hardware was designed for more elaborated graphics than Jolly Card games...
  Color PROMs from current games are 512 bytes lenght, but they only can use the first or the last 256 bytes.

  Normal hardware capabilities:

  - bits -
  7654 3210
  xxxx xx--   tiles color (game tiles)    ;codes 0x00-0xdc
  xxx- x-xx   tiles color (title).        :codes 0xe9-0xeb
  xxxx -xxx   tiles color (background).   ;codes 0xf1-0xf7


  About protection, there are several degrees of protection found in the sets:

  - There are writes to unknown offsets (out of the normal memory range), and some
    checks later to see if the data is still there.

  - There are checks for code in unused RAM and therefore jumps to offsets where there
    are not pieces of code in RAM or just RAM is inexistent.
    I think this is to avoid a "ROM swap" that allow the software to run in other game boards.

  - There are "masked" unused inputs.
    The software is polling the unused input status and expect a special value to boot the game.

  - There are parts of code that are very complex and twisted with fake jumps to inexistent code,
    or pretending to initialize fake devices.

  - Encryption.

      A) Encrypted CPU. At least five Fun World boards have custom encrypted CPUs:

          - Joker Card from Vesely Svet, Mega Card, Power Card and Multi Win,
             use custom unknown CPUs and also encrypted/scrambled prg roms.
          - Royal Card (slovak, encrypted) from Evona Electronic seems to use a block
             with CPU + extras (ICs, TTL, etc) to manage the encryption.

      B) General encryption. Managed through hardware:

          - All games using the blue TAB PCB with 2x HY18CV85 (electrically-erasable PLDs), use
             complex operations for each byte nibble. See DRIVER_INIT for the final algorithm.
          - Saloon (french) use bitswaps to address & data in program, graphics and color PROM.
          - Dino4 hardware games have address/data bitswap in program, and data bitswap (sometimes
             with extra boolean XOR operations) in graphics.

  - Microcontroller. Some games (like Soccer New and Mongolfier New are using an extra MCU mainly
    for protection.

  - Mirrored video and color RAM. A derivative of CMC hardware uses this trick to avoid ROM swaps.
    If you run Luna Park in a regular CMC board, you'll get an unplayable mess of graphics.


  GENERAL NOTES:

  - It takes 46 seconds for the bigdeal/jolycdat games to boot up
    after the initial screen is displayed!!!

  - The default DIP switch settings must be used when first booting up
    the games to allow them to complete the NVRAM initialization.

  - Almost all games: Start game, press and hold Service1 & Service2, press
    reset (F3), release Service1/2 and press reset (F3) again.
    Now the NVRAM has been initialized.

  - Royalcdb needs a hard reset after NVRAM initialization.

  - For games that allow remote credits, after NVRAM init change the payout
    DIP switch from "Hopper" to "Manual Payout".



  NOTES BY GAME/SET:

  * Pool 10
  * Cuore 1
  * Elephant Family
  * Tortuga Family
  * Pot Game
  * Bottle 10
  * Luna Park

  In Italy many people became addicted to videopokers. They put so much money on them,
  and they had to sell the house. Also some engineers modified videopokers to do less
  wins and so on... Because of this the government did some laws in order to regulate
  videopokers wins. Starting from around 1996/1997 there were subsequent laws because
  engineers always found a way to elude them.

  Today all the videopokers need to be connected via AAMS net (a government society de-
  dicated to games) which check if the videopoker is regular. Nowadays it's difficult
  to trick and the videopoker has to give 75% of wins. This has made videopoker market
  to collapse and infact there aren't many videopokers left.

  Also because the laws changed very often and old videopokers became illegal was a
  very bad thing for bar owners because they couldn't earn enough money.

  Pool 10 (now found!), apparently was the "father" of other italian gambling games.
  As soon as it became illegal, was converted to Cuore 1, Elephant Family, Tortuga Family
  and maybe other games. So you can see that engineers always found a simple way to elude
  the law.

  There is another set of Cuore 1. I didn't include it because the only difference with
  the supported set is the program rom that is double sized, having identical halves.

  Luna Park, is running in a modified hardware with video RAM mirrored from 4000-4FFF to
  6000-6FFF and color RAM mirrored from 5000-5FFF to 7000-7FFF. The program writes criti-
  cal graphics (cans, strings and partial screen cleans) to the mirrored range, so if you
  run this program in a regular CMC hardware, you'll get an unplayable mess of graphics.
  Also PRG rom higher address line is connected to DIP switch #1 so it should have 2 games
  in the same PCB (2 revisions?).


  There is at least one missing game in the family... 'Hyppo Family', also from C.M.C.
  This game should be located and dumped.


  --- Super Game ---

  If you have some points accumulated and need to grab the tokens/tickets, you must to play
  a bonus game called SUPER GAME to get the points out. To enter the bonus game, you must
  press STOP5 in the attract mode. The payout system is through this game.

  5 themed items will be shown (cuores, balls, elephants, etc... depending of the game).
  The joker will start to move from item to item quickly, but decreasing the speed gradually.
  To beat the game, you need to push the start button in the exact moment when the joker is
  located exactly in the center of the screen (item 3).

  Depending of the DIP switches settings, you can grab the prize manually pressing the SCARICA
  (payout) button, and then TICKET or HOPPER buttons. Press TICKET button to print a 100 points
  ticket. Press HOPPER button to get tokens x10 points.

  You have 1 attempt for each 100 earned points. If you lose the game, you lose the points.


  * Bonus Card (German)
  * Big Deal (Hungarian)

  These ones seems to have normal RAM instead of NVRAM.
  Going through the code, there's not any NVRAM initialization routine through service 1 & 2.


  * Jolly Card Professional 2.0 (MZS Tech & Spale Soft)

  Each 1st boot, this game show a generated code. You must enter this code in a DOS program to
  get the input codes necessary to allow boot the game.

  This set is one of the most wanted for customers because is a real SCAM.
  The program has 2 hidden menues that allow change parameters without knowledge of the players.

  See more at ROM_LOAD section...


  * Jolly Card (Evona Electronic)

  This set has some hidden features.
  In the service 2 screen, press service 1 to enter to a message edit mode.


  * Jolly Card (croatian sets) and Jolly Card Professional 2.0

  These games don't operate with regular coins/tokens. Only remote credits are allowed.


  * Royal Card Professional 2.0 (Digital Dreams)

  Same generated code as Jolly Card Professional.


  * Magic Card II (Impera)

  Impera made 2 graphics sets for this game. One of them is encrypted, and meant for the TAB blue board.
  This board has two HY18CV85 (electrically-erasable PLD) that handle the encryption.
  In another hand, the sound has some weird things, but is confirmed that happen in the real thing.

  The game resolution seems to change 'on the fly' when entering the input test mode.
  There aren't any writes to the m6845 registers to manage these changes.

  Regarding the CPU, it's a Mexican Rockwell R65C02 with some mods respect the regular/stock 65C02.
  Some CPU instructions seems to be changed. The following piece of code at $C1A8 is very clear:

  C1A8: A0 00         ldy  #$00       ; clear Y register to use as counter.

  C1AA: B9 9D C1      lda  $C19D,y    ; load PIA port address
  C1AD: 85 06         sta  $06        ; into $06-$07 ZP vector.
  C1AF: B9 92 C1      lda  $C192,y    ;
  C1B2: 85 07         sta  $07        ;
  C1B4: C0 0B         cpy  #$0B
  C1B6: B0 0C         bcs  $C1C4

  ... load values to store into Accumulator...
  ...
  C1C6: 91 06         sta  ($06),y    ; store value in PIA port, indexed (Y).
  C1C8: C8            iny             ; increment Y
  C1C9: C0 0C         cpy  #$0C       ; finish?
  C1CB: D0 DD         bne  $C1AA      ; if not, branch to load a new PIA address.

  In this example, as soon as Y register increments, the indexed writes go out of range.
  To get this piece of code working and initialize the PIAs properly, the instruction 0x91 should be
  "sta ($ZP)" instead of Y indexed. (like instruction 0x92 in stock 65c02's).


  * Jolly Joker (Impera)

  To boot this game for 1st time, DSW should be in the following position:
  1=ON 2=OFF 3=ON 4=OFF 5=OFF 6=ON 7=ON 8=OFF

  Press RESET (key F3) and then SERVICE1 & SERVICE2 (keys 9 & 0), then RESET (key F3).
  When numbers start to fill the screen, press RESET (key F3) again to start the game.


  * Joker Card 300 (Ver.A267BC, encrypted).
  * Multi Win (Fun World)
  * Power Card (Fun World)
  * Mega Card (Fun World)

  These sets seems to run in the same modified hardware. They are highly encrypted.

  Joker Card has a title nag at boot. You can wait to finish nearly one minute,
  or simply press SERVICE 2 (key 0), then START (key 1) to enter the game.

  Joker Card has a weird way of play. The more you bet, the more cards burns to
  get better chances. You're not betting really. Just buying removed cards.


  * Mongolfier New
  * Soccer New

  These games are based in Royal Card. They are running in a heavely modified Royal Card
  hardware with a microcontroller as an extra (protection?) component and a TDA2003 as
  audio amplifier.

  The extra microcontroller is a 8 bit (PLCC-44) TSC87C52-16CB from Intel (now dumped!)

  Each set has double sized ROMs. One half contains the proper set and the other half store
  a complete Royal Card set, so... Is possible the existence of a shortcut,'easter egg',
  simple hack or DIP switches combination to enable the Royal Card game.

  These games should be moved to a new driver in a near future, as soon as we know a bit more
  about them and start to implement the missing pieces for an accurate emulation.


  * Saloon

  This game is totally encrypted. No PIAs to drive I/O. The PCB has printed "LEOPARDO 5", so we
  can expect a game called the same way to appear.

  This game should be moved to a new driver in a near future, as soon as we know a bit more about it.


  * Fun World Quiz

  Beschreibung / Description
  --------------------------

  Fur OES 5,-- Einwurf werden 3 Kredite gegeben. Fur einen Kredit bekommt der Spieler
  500 Spielpunkte. Bei jeder Frage kann der Spieler 100, 500 oder alle Punkte einsetzen.
  Wird die Frage richtig beantwortet, wird der Einsatz aufaddiert. Erreicht der Spieler
  nach 5 Fragen mehr als 2500 Punkte, bekommt er Bonusfragen. Das Spiel endet bei einer
  falschen Beantwortung der Frage.

  Insertion of ATS 5,- (Austrian Schilling coin) yields 5 credits. For 1 credit a player
  receives 500 points. At each question a player can bet 100, 500 or all points. If a
  question is answered correctly, the bet is added to the points. If a player reaches
  more than 2500 points after 5 questions, he gets bonus questions. The game is over
  after an incorrect answer.

  Besides notes....

  This machine is meant to work as "amusement", with high scores like another regular arcade.
  You can set it as "bet mode", allowing to choose the amount of points to play, but can't be
  turned to gambling mode (no payout, keyout, etc) for pubs, etc...

  In bookkeeping mode (DIP switch #1):

  Keep joystick left to clear bookkeeping.
  Keep joystick up to clear high scores.
  Keep joystick down to clear credits.
  Keep joystick right to exit bookkeeping.

  If the game has credits loaded, the bookkeeping mode will start
  as soon as the current game ends.


  * Novo Play Multi Card / Club Card

  This game needs a default NVRAM to work. There is a special ROM that need to be
  placed into the program socket, then turn ON the board till the OK message appear.
  After this operation, just switch the special ROM with the regular program.
  The board now is ready to operate.

  To program the game, enter the service mode (Key 9), and then keep pressed both
  HOLD2 & HOLD4 at least for 5 seconds....

  You can see:

     C1        C2          REMOTE         IN-MAX/IN-MIN         CONT

  (Coin A)  (Coin B)  (Remote credits)  (Bet Max & Minimum)  (Difficult)


  C2:     Coin B, from 1-20... Selectable through HOLD1.
  REMOTE: Remote Credits, from 10-100... Selectable through HOLD3.
  IN-MAX: Maximum Bet allowed. From 1-40. selectable through HOLD4.
  IN_MIN: Minimum Bet allowed. From 1-5. selectable through HOLD4, keeping COLLECT pressed.
  CONT:   Earnings control... From 1-4, selectable through HOLD5. 4 is the highest payment.

  Press DEAL/DRAW to exit the mode.


  * Unknown Royal Card on Dino4 hardware....

  This one is really strange. The game is running in a Dino4 hardware, plus a daughterboard
  with a mexican Rockwell R65C02 + an unknown PLCC. The program/gfx are totally decrypted.
  The game vectors are $C122 (RESET) and $C20F (IRQ)

  The code starts...

  C122: A2 FF      LDX #$FF    ; load 0xFF to reg X
  C124: 9A         TXS         ; transfer to the stack
  C125: 78         SEI         ; set interrupts
  C126: D8         CLD         ; clear decimal
  C127: 18         CLC         ; clear carry
  C128: A9 4C      LDA #$4C    ;\
  C12A: 8D 00 00   STA $0000   ; \
  C12D: A9 10      LDA #$10    ;  \ set 4C 10 C2 (JMP $C210) into $0000
  C12F: 8D 01 00   STA $0001   ;  /
  C132: A9 C2      LDA #$C2    ; /
  C134: 8D 02 00   STA $0002   ;/
  C137: 4C DC 48   JMP $48DC   ; jump to $48DC...

  And the IRQ vector pointed code... does nothing!

  C20F: 40         RTI         ; return from interrupt

  And the code pointed from $0000...

  C210: 48         PHA         ; transfer accumulator to stack
  C211: AD 01 0A   LDA $0A01   ; read the PIA #2 input
  C214: 29 F7      AND #$F7    ; \ compare with 0xF7
  C216: CD 01 0A   CMP $0A01   ; /
  C219: D0 02      BNE $C21D   ; if not... jump to $C21D
  C21B: 09 08      ORA #$08    ; \ clean the value
  C21D: 8D 01 0A   STA $0A01   ; /
  C220: 68         PLA         ; take out from the stack the previous accumulator value
  C221: 40         RTI         ; return from interrupt

  The board was later connected to a fluke, and the pieces of code dumped from the real hardware,
  match 100% the ones I decrypted here. Even with the game working properly in the real hardware.
  The only visible changes are in the NVRAM, where the $0000 offset hasn't the JMP $C210 instruction
  injected at the start...


  * Joker Card / Multi Card (Epoxy brick CPU)

  These series are running in a derivative hardware with epoxy CPU brick.
  CPU is RP65C02A, with 2x Toshiba TC5565APL-10 RAMs. 1x Intel D27128A ROM.
  and 1x PAL16L8B.

  The main program is inside the battery backed RAM, and need to be dumped.
  See more notes below in the ROM loading.


  * Royal Card (Italian, Dino 4 hardware, encrypted)

  This game is highly encrypted and is running in Dino 4 hardware.
  To initialize the NVRAM, you should press both service buttons together (keys 0-9) and then reset (key F3).
  Due to the way the routine is programmed, the procedure is time sensitive and often doesn't work.

  This game has a programming menu through the service mode (key 0), then pressing STOP 2 & STOP 4 together
  and waiting for some seconds.

  You can see two options...

  STOP 1 ---- OPZIONI 1
  STOP 3 ---- OPZIONI 2

  OPZIONI 1:
  You can change game parameters for Gioco AB, Play Max, Play Min, Difficolt, Bonus, and Rilancio.

  OPZIONI 2:
  You can change game parameters for Tabella Vincite, Velocita Simboli, Control Double, Valore Ticket,
  Limit Credit, and Limit Scarico.

  In both cases, you can exit pressing the START button (key 1).


  * Jolly Card (Italian, bootleg, set 2)

  This game PCB has a TI EP910DC-30 CPLD, like others Italian bootlegs
  that have an Altera EP910PC CPLD on board.

  To init NVRAM, DIP switches must be in the following positions:
  OFF ON ON ON ON ON ON ON

  Then press SERVICE 1 + SERVICE 2 and reset, then release both buttons.

  Once NVRAM is initialized, the hardware still complains about it due
  to a hardware check (see more at the rom load), but is now working.

  The game seems to work for a currency that needs decimal values.
  Maybe Italian Lires...


  * Jolly Card (German, Fun World, bootleg)

  This game is a bootleg made from a Fun World original program.
  The game has hidden features, as a program mode.

  In the Service menu (key 0), press HOLD 1 & HOLD 2 together for
  some seconds and the programming menu will appear.


  * Royal Card (Evona, Polish, encrypted)

  This is another highly encrypted game.
  To initialize the NVRAM all DIP switches must be in ON. Keep pressed both service buttons
  (keys 9 & 0) and then RESET (key F3), then release first the service 1 (key 9) and then
  service 2 (key 0).

  The game does some odd things, due to a possible bug in the
  decryption scheme. (see the TO DO list).


  * Zabavni Karti (ЗАБАВНИ КАРТИ)
    (cyrillic, Bulgarian, encrypted)

  This is a Bulgarian Jolly Card pseudo-clone, with cyrillic charset.
  ЗАБАВНИ КАРТИ means "Fun cards" / "Cards for fun".

  The game has encrypted GFX. Same style as blue TAB board.

  This is a timed game. You coin for an amount of playing time.
  Once the time counter ends, all earned credits/bonus dissapear.

  It looks like an amusement version.

  ---------------------------------------------------------------------

  To initialize the NVRAM you must...

  1) Press SETUP (НАСТРОЙКА) (key 0)
  2) Keep pressed CANCEL (ИЗЧИСТВАНЕ) button (key N)
  3) Press the following STOP (СТОП) buttons (keys Z-X-C-V-B) sequence:

     1 - 5 - 4 - 3 - 4 - 2

    (you will see the initialization message)

  4) Release the CANCEL (ИЗЧИСТВАНЕ) button (key N)

  ---------------------------------------------------------------------

  To configure the game...

  1) Press SETUP (НАСТРОЙКА) (key 0)

     You'll see the following counters and menu:

     КАРЕ С ЖОКЕР      0    (five of a kind)
     КЕНТ ФЛЕШ РОЯЛ    0    (royal flush)
     КЕНТ ФЛЕШ         0    (straight flush)
     КАРЕ              0    (poker)
     ФУЛ               0    (full house)
     ФЛЕШ              0    (flush)
     КЕНТА             0    (straight)
     ТРОЙКА            0    (three of a kind)
     2 ДВОЙКИ          0    (two pairs)
     ГОЛЯМА ДВОЙКИ     0    (high pair)


     ВРЕМЕ:  Натиснете бутон Стоп3
     TIME:   Press the Stop3 button

                    Изиграно време, мин. :  00
                    Played time, min.    :  00

                    Платено време, мин.  :  00
                    Paid time, min.      :  00


     НАСТРОЙКА:  Натиснете бутони Стоп2 и Стоп4
     SETUP:      Press the Stop2 and Stop4 buttons

                    Бутон Стоп1 -- Време за една игра, мин.  (1-99, 5 by default)
                    Stop1 Button -- Time for one game, min.  (1-99, 5 by default)

                    Бутон Стоп2 -- Залог  (1-99, 10 by default)
                    Stop2 button - Bet    (1-99, 10 by default)

                    Бутон Стоп3 -- Ниво на трудност      (0-3, 2 by default)
                    Stop3 button -- Level of difficulty  (0-3, 2 by default)

                    ИЗХОД:  Натиснете бутон Старт
                    EXIT:   Press the Start button


     НУЛИРАНЕ:  Стоп1 нулира часовника
     RESET:     Stop1 resets the clock

     ИЗЧИСТВАНЕ:  Натиснете 2 с. бутон Изчистване
     CLEAR:       Press the Cancel button for 2 seconds

     ИЗХОД:  Натиснете бутон Старт
     EXIT:   Press the Start button

  ---------------------------------------------------------------------

  You can enter the test mode activating the 'TEST MODE' DIP switch.
  (only will work if you have some adquired time)

  For AUTOHOLD (АВТОСТОП) function, set the 'AUTO HOLD' DIP switch ON,
  then toggle ON/OFF the function pressing CANCEL (ИЗЧИСТВАНЕ) button (key N).


**********************************************************************************************

  Memory Map (generic)
  --------------------

  $0000 - $07FF   NVRAM           // All registers and settings.
  $0800 - $0803   PIA1            // Input Ports 0 & 1.
  $0A00 - $0A03   PIA2            // Input Ports 2 & 3.
  $0C00 - $0C00   AY-8910 (R/C)   // Read/Control.
  $0C01 - $0C01   AY-8910 (W)     // Write. Lamps through output ports.
  $0E00 - $0E00   CRTC6845 (A)    // MC6845 addressing.
  $0E01 - $0E01   CRTC6845 (R/W)  // MC6845 Read/Write.

  $2000 - $2FFF   VideoRAM (bonuscrd/bigdeal)
  $3000 - $3FFF   ColorRAM (bonuscrd/bigdeal)

  $4000 - $4FFF   VideoRAM (magicrd2/royalcrd)
  $5000 - $5FFF   ColorRAM (magicrd2/royalcrd)

  $6000 - $6FFF   VideoRAM (CMC italian games)
  $7000 - $7FFF   ColorRAM (CMC italian games)

  $8000 - $FFFF   ROM (almost all games)


  *** MC6845 Initialization ***

  ----------------------------------------------------------------------------------------------------------------------
  register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
  ----------------------------------------------------------------------------------------------------------------------
  jollycrd:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  jolycdae:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  jolycdcr:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  jolycdit:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  jolycdab:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.

  bigdeal:   0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.

  cuoreuno:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  elephfam:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  pool10:    0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  tortufam:  0x7C  0x60  0x65  0x08  0x1E  0x08  0x1D  0x1D  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.

  royalcrd:  0x7C  0x60  0x65  0xA8  0x1E  0x08  0x1D  0x1C  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  magicrd2:  0x7B  0x70  0x66  0xA8  0x24  0x08  0x22  0x22  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.

  monglfir:  0x7C  0x60  0x65  0xA8  0x1E  0x08  0x1D  0x1C  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  soccernw:  0x7C  0x60  0x65  0xA8  0x1E  0x08  0x1D  0x1C  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.

  rcdino4:   0x7C  0x60  0x65  0x08  0x21  0x08  0x1F  0x1F  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.
  chinatow:  0x7C  0x60  0x65  0x08  0x21  0x08  0x1F  0x1F  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.


**********************************************************************************************


  *** TO DO ***

  - Figure out the remaining PIA connections for almost all games.
  - Reverse-engineering the boot code of Jolly Card Professional 2.0,
     and Royal Card Professional 2.0 to get the proper codes to boot.
  - Analyze the unknown writes to $2000/$4000 in some games.
  - Check for the reads to the ay8910 output ports in some games.
  - Implement the MCU in monglfir and soccernw.


*********************************************************************************************/

#include "emu.h"
#include "funworld.h"

#include "cpu/m6502/m65sc02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

#include "bonuscrd.lh"
#include "jollycrd.lh"
#include "novoplay.lh"
#include "royalcrd.lh"


#define MASTER_CLOCK    XTAL(16'000'000)
#define CPU_CLOCK      (MASTER_CLOCK/8)
#define SND_CLOCK      (MASTER_CLOCK/8)
#define CRTC_CLOCK     (MASTER_CLOCK/8)


/**********************
* Read/Write Handlers *
**********************/

void funworld_state::funworld_lamp_a_w(uint8_t data)
{
/*  - bits -
    7654 3210
    ---- ---x   Credit In counter.
    ---- --x-   HOLD1 & HOLD3 lamps (inverted).
    ---- -x--   Credit Out counter.
    ---- x---   HOLD2 lamp (inverted).
    ---x ----   Unknown (inverted).
    --x- ----   CANCEL / COLLECT (inverted).
    -x-- ----   Hopper Motor (inverted).
    x--- ----   HOLD4 lamp.
*/
	m_lamps[0] = BIT(~data, 1); // Hold1 (inverted)
	m_lamps[2] = BIT(~data, 1); // Hold3 (inverted, see pinouts)

	m_lamps[1] = BIT(~data, 3); // Hold2 / Low (inverted)
	m_lamps[3] = BIT(data, 7);  // Hold4 / High
	m_lamps[5] = BIT(~data, 5); // Cancel / Collect (inverted)

	machine().bookkeeping().coin_counter_w(0, data & 0x01); // Credit In counter
	machine().bookkeeping().coin_counter_w(7, data & 0x04); // Credit Out counter, mapped as coin 8

	m_lamps[7] = BIT(~data, 6);     // Hopper Motor (inverted)

//  popmessage("Lamps A: %02X", (data ^ 0xff));
}

void funworld_state::funworld_lamp_b_w(uint8_t data)
{
/*  - bits -
    7654 3210
    ---- ---x   HOLD5 lamp.
    ---- --x-   DEAL/DRAW lamp.
    ---- -x--   Unknown (inverted).
    xxxx x---   Unknown.
*/
	m_lamps[4] = BIT(data, 0);  // Hold5 / Bet
	m_lamps[6] = BIT(data, 1);  // Start / Deal / Draw

//  popmessage("Lamps B: %02X", data);
}

void funworld_state::pia1_ca2_w(int state)
{
// TAB and Impera games are writing 0x01 constantly, and 0x00 with each screen change.
// This line is tied to some sort of reset circuitery.

//  popmessage("PIA1 CA2: %02X", state);
}


/*************************
* Memory map information *
*************************/

void funworld_state::funworld_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2000, 0x2fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x3000, 0x3fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x4000, 0x7fff).rom();
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xffff).rom();
}

static uint8_t funquiz_question_bank = 0x80;

uint8_t funworld_state::questions_r(offs_t offset)
{
	uint8_t* quiz = memregion("questions")->base();
	int extraoffset = ((funquiz_question_bank & 0x1f) * 0x8000);

	// if 0x80 is set, read the 2nd half of the question rom (contains header info)
	if (funquiz_question_bank & 0x80) extraoffset += 0x4000;

	return quiz[offset + extraoffset];
}

void funworld_state::question_bank_w(uint8_t data)
{
//  printf("question bank write %02x\n", data);
	funquiz_question_bank = data;
}

void funworld_state::funquiz_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x1800, 0x1800).w(FUNC(funworld_state::question_bank_w));

	map(0x2000, 0x2fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x3000, 0x3fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x4000, 0x7fff).r(FUNC(funworld_state::questions_r));

	map(0xc000, 0xffff).rom();
}

void funworld_state::magicrd2_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2c00, 0x2cff).ram();  // range for protection
	map(0x3600, 0x36ff).ram();  // some games use $3603-05 range for protection
	map(0x3c00, 0x3cff).ram();  // range for protection
	map(0x4000, 0x4fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x5000, 0x5fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x6000, 0xffff).rom();
}

void funworld_state::clubcard_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x5000, 0x5fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xffff).rom();
}

void funworld_state::cuoreuno_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2000, 0x2000).nopr(); // some unknown reads
	map(0x3e00, 0x3fff).ram();  // some games use $3e03-05 range for protection
	map(0x4000, 0x5fff).rom();  // used by rcdino4 (dino4 hw )
	map(0x6000, 0x6fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x7000, 0x7fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x8000, 0xffff).rom();
}


uint8_t chinatow_state::chinatow_r_32f0(offs_t offset)
{
	logerror("read from 0x32f0 at offset %02X\n", offset);
	switch (offset)
	{
	case 0: return 0xfe;
	}
	return 0xff;
}

void chinatow_state::chinatow_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2000, 0x2000).nopr(); // some unknown reads
	map(0x32f0, 0x32ff).r(FUNC(chinatow_state::chinatow_r_32f0));
	map(0x4000, 0x5fff).rom(); // used by rcdino4 (dino4 hw)
	map(0x6000, 0x6fff).ram().w(FUNC(chinatow_state::funworld_videoram_w)).share("videoram");
	map(0x7000, 0x7fff).ram().w(FUNC(chinatow_state::funworld_colorram_w)).share("colorram");
	map(0x8000, 0xffff).rom();
}

void lunapark_state::lunapark_map(address_map &map)
{ // mirrored video RAM 4000/5000 to 6000/7000
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4fff).ram().w(FUNC(lunapark_state::funworld_videoram_w)).share("videoram").mirror(0x2000);
	map(0x5000, 0x5fff).ram().w(FUNC(lunapark_state::funworld_colorram_w)).share("colorram").mirror(0x2000);
	map(0x8000, 0xffff).bankr("bank1");
}

void funworld_state::saloon_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0800).portr("IN0");
	map(0x0801, 0x0801).portr("IN1");
	map(0x0802, 0x0802).portr("IN2"); // maybe
	map(0x0808, 0x0808).portr("IN3"); // maybe
	map(0x0810, 0x0810).nopw();
	map(0x0a00, 0x0a00).portr("IN4");
	map(0x0a01, 0x0a01).portr("IN5");
	map(0x081b, 0x081b).portw("I2C_CK");
	map(0x081c, 0x081c).w("crtc", FUNC(mc6845_device::address_w));
	map(0x081d, 0x081d).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x1000, 0x1000).portr("I2C_DI").portw("I2C_DO");
	map(0x1800, 0x1800).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x1800, 0x1801).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x2000, 0x2003).nopr();  // check for extra ROM in $2000. if exist just jump to $2100
	map(0x6000, 0x6fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x7000, 0x7fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x8000, 0xffff).rom();
}
/*
    Unknown R/W
    -----------

    0800    RW  ;input?
    081a    W   ;unknown (W 0x20)
    081b    W   ;unknown (W 0x20 & 0x30)
    0810    W   ;unknown
    0a01    RW  ;input?
    1000    RW  ;input? (W 0xff & 0xfd)

*/

void funworld_state::witchryl_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x5000, 0x5fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x6000, 0x6000).portr("DSW2");
	map(0x8000, 0xffff).rom();
}

uint8_t intergames_state::crtc_or_psg_r(offs_t offset)
{
	if (m_crtc_selected)
		return m_crtc->register_r();
	else
		return m_ay8910->data_r(); // read back from both $3000 and $3001!
}

void intergames_state::crtc_or_psg_w(offs_t offset, uint8_t data)
{
	if (!m_crtc_selected)
		m_ay8910->address_data_w(offset, data);
	else if (BIT(offset, 0))
		m_crtc->register_w(data);
	else
		m_crtc->address_w(data);
}

uint8_t intergames_state::prot_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if ((offset == 0xc1) || (offset == 0xef) || (offset == 0x99))
			m_crtc_selected = false;
		else
			logerror("%s: Protection read from $%04X\n", machine().describe_context(), offset + 0x3600);
	}
	return 0xff;
}

void intergames_state::prot_w(offs_t offset, uint8_t data)
{
	if (offset == 0xf3)
		m_crtc_selected = false;

	logerror("%s: Writing $#%02X to $%04X\n", machine().describe_context(), data, offset + 0x3600);
}

void intergames_state::machine_reset()
{
	m_crtc_selected = true;
}

void intergames_state::intergames_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x2000, 0x2fff).ram().w(FUNC(intergames_state::funworld_videoram_w)).share("videoram");
	map(0x3000, 0x3001).rw(FUNC(intergames_state::crtc_or_psg_r), FUNC(intergames_state::crtc_or_psg_w));
	map(0x3200, 0x3203).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x3400, 0x3403).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x3600, 0x37ff).rw(FUNC(intergames_state::prot_r), FUNC(intergames_state::prot_w));
	map(0x5000, 0x5fff).mirror(0x2000).ram().w(FUNC(intergames_state::funworld_colorram_w)).share("colorram");
	map(0x8000, 0xdfff).rom();
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).rom();
}

void funworld_state::fw_brick_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x2000, 0x2fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x3000, 0x3fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x4000, 0x7fff).nopr();    // check for the brick RAM programming ROM.
	map(0x8000, 0xbfff).ram().share("nvram1");
	map(0xc000, 0xffff).rom();
}

void royalcrdf_state::royalcrdf_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4fff).ram().w(FUNC(royalcrdf_state::funworld_videoram_w)).share("videoram");
	map(0x5000, 0x5fff).ram().w(FUNC(royalcrdf_state::funworld_colorram_w)).share("colorram");
	map(0x6000, 0x6003).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6800, 0x6803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xffff).rom();
}


void funworld_state::gratispk_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0a00, 0x0a03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c00).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0c00, 0x0c01).w("ay8910", FUNC(ay8910_device::address_data_w));
	map(0x0e00, 0x0e00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0e01, 0x0e01).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x4fff).ram().w(FUNC(funworld_state::funworld_videoram_w)).share("videoram");
	map(0x5000, 0x5fff).ram().w(FUNC(funworld_state::funworld_colorram_w)).share("colorram");
	map(0x6000, 0xffff).rom();
}


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( funworld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Halten (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Loeschen (Cancel) / Kassieren (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Geben (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Halten (Hold) 4 / Hoch (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Halten (Hold) 2 / Tief (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Halten (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreib (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote Value" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, "Insert" )            PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Dattl Insert" )
	PORT_DIPSETTING(    0x10, "TAB Insert" )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( jolycdcr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Navijanje (Remote)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Ponistavange (Cancel) / Kasiranje (Take) / Autohold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Djelenje (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop (Hold) 5 / Ulog (Bet) / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Konobar (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Namjestit (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop (Hold) 4 / Veca (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop (Hold) 2 / Manja (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Vratiti Nazad (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote Value" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( jolycdit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop (Hold) 1 / Alta (High)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Doppio (Double) / Autohold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop (Hold) 4 / Accredito (Take)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop (Hold) 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop (Hold) 3 / Bassa (Low)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote Value" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "50 Points/Pulse" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( jolycdib )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Autohold / Accredito (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Doppio (Double)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop (Hold) 5 / Bet / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop (Hold) 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop (Hold) 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("test5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("test6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("test7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("test8") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("test9") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("test10") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_NAME("test11") PORT_CODE(KEYCODE_R)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Remote Value" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )       // when is ON, allows the player to activate/deactivate the autohold through CANCEL button

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( jolycdic )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Autohold / Accredito (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Doppio (Double)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop (Hold) 5 / Bet / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop (Hold) 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop (Hold) 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Manual Payout SW") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")   // place '10000000' for NVRAM creation.
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Remote Value" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x04, "Coinage B" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "10 Credits" )
	PORT_DIPSETTING(    0x00, "5 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "Coinage A" )         PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, "20 Credits" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )       // when is ON, allows the player to activate/deactivate the autohold through CANCEL button
	PORT_DIPNAME( 0x80, 0x00, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( jolycdcy )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME(u8"Running / Credits (Навъртане)")                               // НАВЪРТАНЕ
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME(u8"Stop 1 (Стоп 1)")                                             // СТОП 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME(u8"Cancel (Изчистване) / Autostop (Автостоп) / Take (Качване)")  // ИЗЧИСТВАНЕ / АВТОСТОП / КАЧВАНЕ
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME(u8"Start (Старт) / Double (Дублиране)")                          // СТАРТ / ДУБЛИРАНЕ
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME(u8"Stop 5 (Стоп 5) / Half Gamble (Половин Хазарт)")              // СТОП 5 / ПОЛОВИН ХАЗАРТ
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME(u8"Turnover (Оборот)")                                           // ОБОРОТ (turnover)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME(u8"Setup (Настройка)")                                           // НАСТРОЙКА (setup)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME(u8"Stop 4 (Стоп 4) / Big (Голяма)")                              // СТОП 4 / ГОЛЯМА

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME(u8"Stop 2 (Стоп 2) / Small (Малка)")                             // СТОП 2 / МАЛКА
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME(u8"Stop 3 (Стоп 3)")                                             // СТОП 3
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test 1") PORT_CODE(KEYCODE_A)    // present on the test mode, but erased
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test 2") PORT_CODE(KEYCODE_S)    // normally coin2. present on the test mode, but erased.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test 3") PORT_CODE(KEYCODE_D)    // normally keyout. present on the test mode as "K'bo?".

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("test 4") PORT_CODE(KEYCODE_F)    // normally coin1. present on the test mode, but erased.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")   // only works with credits inserted
	PORT_DIPSETTING(    0x01, "Normal Play" )
	PORT_DIPSETTING(    0x00, "Test Mode" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPSETTING(    0x00, "With Joker" )
	PORT_DIPNAME( 0x40, 0x40, "Auto Hold" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )       // when is ON, allows the player to activate/deactivate the autohold through CANCEL button
	PORT_DIPNAME( 0x80, 0x80, "Credits In" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "Allow Credits" )
	PORT_DIPSETTING(    0x00, "Demo Mode" )
INPUT_PORTS_END


static INPUT_PORTS_START( jolyc980 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Navijanje (Remote)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Ponistavange (Cancel) / Kasiranje (Take) / Autohold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Djelenje (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop (Hold) 5 / Ulog (Bet) / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Konobar (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Namjestit (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop (Hold) 4 / Veca (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop (Hold) 2 / Manja (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Vratiti Nazad (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( bonuscrd )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Red")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Black / Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Hoch (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Tief (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Half Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Payout")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
//  DIP switch 8 should be left ON by default (all remaining ones in OFF)
//  to allow initialization. You can change settings later, after the boot.

	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus Type" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Good Luck!" )
	PORT_DIPSETTING(    0x02, "55/77/99 Bonus" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, "D-UP Type" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Forced, Red-Low-High-Black" )
	PORT_DIPSETTING(    0x10, "Classic Hi-Lo" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x00, "Payout" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( bigdeal )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Stake")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Nagy (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Icsi (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Half Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
//  DIP switch 8 should be left ON by default (all remaining ones in OFF)
//  to allow initialization. You can change settings later, after the boot.

	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus Type" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Good Luck!" )
	PORT_DIPSETTING(    0x02, "55/77/99 Bonus" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, "D-UP Type" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Forced, Red-Low-High-Black" )
	PORT_DIPSETTING(    0x10, "Classic Hi-Lo" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x00, "Payout" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( magicrd2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Stake")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / High")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Low")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper SW") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")   // remote credits settings are always 10 points/pulse
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x00, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( royalcrd )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Halten (Hold) 1 / Hoch (High)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Loeschen/Gamble (Cancel/Play)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Geben (Start)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Halten (Hold) 4 / Kassieren (Take)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Halten (Hold) 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Halten (Hold) 3 / Tief (Low)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("unknown bit 08") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreib (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x02, "Remote Value" )          PORT_DIPLOCATION("SW1:7")   // listed as 'Coin-C' in some sources
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x10, "Insert" )            PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Dattl Insert" )
	PORT_DIPSETTING(    0x10, "TAB Insert" )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( royalcrdf )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Kredyt (Remote)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Zlikwidow (Cancel)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Rachunek (Account)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Vstawiac (Service 2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Vyplatit (Payout)") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Odpis (Keyout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x02, "Remote Value" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bonus" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x40, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( royaljp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Halten (Hold) 1 / Hoch (High)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Loeschen/Gamble (Cancel/Play)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Geben (Start)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Halten (Hold) 4 / Kassieren (Take)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Halten (Hold) 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Halten (Hold) 3 / Tief (Low)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("unknown bit 08") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreib (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* ________________________________________________________________________
  |DIP Schalter 1                          | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
  |________________________________________|___|___|___|___|___|___|___|___|
  |Hopperbetrieb über Board        ein/on  |on |   |   |   |   |   |   |   |
  |(Hopper Payout)                 aus/off |off|   |   |   |   |   |   |   |
  |________________________________________|___|___|___|___|___|___|___|___|
  |Autohold                        an/yes  |   |on |   |   |   |   |   |   |
  |                                aus/no  |   |off|   |   |   |   |   |   |
  |________________________________________|___|___|___|___|___|___|___|___|
  |Gewinn ab: "Hohes Paar"/"High pair"     |   |   |on |   |   |   |   |   |
  |(Profit)   "König oder As"/"King or Ace"|   |   |off|   |   |   |   |   |
  |________________________________________|___|___|___|___|___|___|___|___|
  |Insert "Dattl"                          |   |   |   |on |   |   |   |   |
  |Insert "Royal-Jackpot"                  |   |   |   |off|   |   |   |   |
  |________________________________________|___|___|___|___|___|___|___|___|
  |Coin A = 10  Coin B = 50                |   |   |   |   |on |on |   |   |
  |Coin A = 10  Coin B = 20                |   |   |   |   |on |off|   |   |
  |Coin A = 1   Coin B = 20                |   |   |   |   |off|off|   |   |
  |Coin A = 1   Coin B = 50                |   |   |   |   |off|on |   |   |
  |________________________________________|___|___|___|___|___|___|___|___|
  |Immer / always                          |   |   |   |   |   |   |off|   |
  |________________________________________|___|___|___|___|___|___|___|___|
  |Keyboard Test                           |   |   |   |   |   |   |   |on |
  |Spielbetrieb / Maingame                 |   |   |   |   |   |   |   |off|
  |________________________________________|___|___|___|___|___|___|___|___|

  Default Settings from Video Klein:  01101100

*/
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Main Game" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:7" )    // Shown as "Always Off" in the manual
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "50 Points/Coin" )
	PORT_DIPSETTING(    0x04, "20 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "10 Points/Coin" )
	PORT_DIPSETTING(    0x08, "1 Point/Coin" )
	PORT_DIPNAME( 0x10, 0x10, "Insert" )            PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Dattl Insert" )
	PORT_DIPSETTING(    0x10, "Royal Jackpot" )
	PORT_DIPNAME( 0x20, 0x00, "Profit" )            PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "Hohes Paar" )
	PORT_DIPSETTING(    0x20, "2 Kings or Aces" )
	PORT_DIPNAME( 0x40, 0x00, "Autohold" )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Payout" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
INPUT_PORTS_END


static INPUT_PORTS_START( cuoreuno )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // no remote credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1 / Switch Bet (1-Max)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Bet / Prendi (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Gioca (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5 / Half Gamble / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )                 PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )   PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - User Choice 1" )
	PORT_DIPSETTING(    0x20, "Manual - Coins" )
	PORT_DIPSETTING(    0x40, "Manual - Tickets" )
	PORT_DIPSETTING(    0x60, "Manual - User Choice 2" )

//  the following one (1st DSW) seems to be disconnected
//  to avoid the use of remote credits or direct payout
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( pool10 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // no remote credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1 / Switch Bet (1-Max)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Bet / Prendi (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Gioca (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5 / Half Gamble / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )                 PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )   PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - User Choice 1" )
	PORT_DIPSETTING(    0x20, "Manual - Coins" )
	PORT_DIPSETTING(    0x40, "Manual - Tickets" )
	PORT_DIPSETTING(    0x60, "Manual - User Choice 2" )

//  direct payout without play Super Game
	PORT_DIPNAME( 0x80, 0x80, "Direct Payout (tickets)" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( lunapark )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // no remote credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1 / Switch Bet (1-Max)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Bet / Prendi (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Gioca (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5 / Half Gamble / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  the following one seems to be disconnected
//  to avoid the use of remote credits or direct payout
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

//  the following one is connected to 1st DSW and is meant
//  for switch between different programs stored in different
//  halves of the program ROM
	PORT_START("SELDSW")
	PORT_DIPNAME( 0x01, 0x00, "Game Selector" )           PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "PROGRAM 1, (5 TIRI LIRE 500, ABILITA VINTE)" )
	PORT_DIPSETTING(    0x01, "PROGRAM 2, (10 TIRI LIRE 500, PARTITA VINTE)" )
INPUT_PORTS_END


static INPUT_PORTS_START( jolyjokra )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Halten (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Loeschen (Cancel) / Kassieren (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Geben (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Halten (Hold) 4 / Hoch (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Halten (Hold) 2 / Tief (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Halten (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreib (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote Value" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "disabled" )
	PORT_DIPSETTING(    0x10, "enabled" )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x40, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x00, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( vegasslw )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Kasiraj (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Kockaj (Double)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Ulog (Bet)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buch (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Velika (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Mala (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Auszahlen") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper Switch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreiben (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x00, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( vegasfst )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Prihoduj (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Dupliraj (Double)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Ulog (Bet)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buch (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Velika (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Mala (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Auszahlen") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper Switch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreiben (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Remote Value" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "100 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "1000 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( vegasfte )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Kasiraj (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Kockaj (Double)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / +Ulog (Add Bet)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buch (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Velika (High) / -Ulog (Remove Bet)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Mala (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Auszahlen") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper Switch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreiben (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Remote Value" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "100 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "1000 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( vegasmil )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Prihoduj (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Dupliraj (Double)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Ulog (Bet)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buch (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Velika (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Mala (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Auszahlen") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper Switch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreiben (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x00, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( saloon )
	PORT_START("IN0")  // 0800h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )       PORT_NAME("Play / Start / Enter")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("0-2")        PORT_CODE(KEYCODE_2)  // still unknown...
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Stop 4 / High / Menu Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Take / Autohold / Exit")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )     PORT_NAME("Menu")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Menu Up")    PORT_CODE(KEYCODE_6)  // check for extra functions...
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Stop 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Stop 5 / Bet")

	PORT_START("IN1")  // 0801h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_NAME("1-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN2")  // 0802h
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_POKER_HOLD2 ) PORT_NAME("Stop 2 / Low")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_POKER_HOLD3 ) PORT_NAME("Stop 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")  // 0808h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )      PORT_NAME("Credits IN/OUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Keyout (Scarico Coin)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Warm Reset") PORT_CODE(KEYCODE_R)

	PORT_START("IN4")  // 0A00h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-8") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("IN5")  // 0A01h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-2") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-4") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-8") PORT_CODE(KEYCODE_I)

	PORT_START("I2C_CK")  // 081Bh
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("i2cmem" , i2cmem_device, write_scl)    // Serial Clock
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("I2C_DI")  // 1000h Input
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)      // Serial Data In
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("I2C_DO")  // 1000h Output
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, write_sda)   // Serial Data Out
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( funquiz )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) // start?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) // start?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 ) // start?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START4 ) // start or clear?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY   // joystick right

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY    // joystick left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY      // joystick up
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY    // joystick down
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )      // coin 2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )      // coin 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
/*
  1 | Buchhaltung / Bookkeeping.
  2 | Nicht verwendet / Not used.
  3 | Ohne Zahlen (Wien) / No numbers (Vienna).
  4 | Nicht verwendet / Not used.
  5 | Nicht verwendet / Not used.
  6 | Richtige Antwort wird angezeigt / Right answer is shown.
  7 | Frage wird bei Einsatz angezeigt / Question is shown when bet is made.
  8 | Spiel mit Einsatzwahl / Game with betting.
*/
	PORT_DIPNAME( 0x01, 0x00, "Game with betting" )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Show question in bet stage" )    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Right answer is shown" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "No numbers (Vienna)" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Bookkeeping" )           PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( funquiza )
	PORT_START("IN0")  // 0800h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Option B") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Option A") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )   PORT_NAME("Start / G")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Option F") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )  PORT_NAME("Books 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )  PORT_NAME("Books 2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Option E") PORT_CODE(KEYCODE_B)

	PORT_START("IN1")  // 0801h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Option C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Option D") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")  // 0A00h
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")  // 0A01h
/*
  1 | Buchhaltung / Bookkeeping.
  2 | Nicht verwendet / Not used.
  3 | Ohne Zahlen (Wien) / No numbers (Vienna).
  4 | Nicht verwendet / Not used.
  5 | Nicht verwendet / Not used.
  6 | Richtige Antwort wird angezeigt / Right answer is shown.
  7 | Frage wird bei Einsatz angezeigt / Question is shown when bet is made.
  8 | Spiel mit Einsatzwahl / Game with betting.
*/
	PORT_DIPNAME( 0x01, 0x00, "Game with betting" )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Show question in bet stage" )    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Right answer is shown" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "No numbers (Vienna)" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Bookkeeping" )           PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( witchryl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Halten (Hold) 1 / Hoch (High)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Loeschen/Gamble (Cancel/Play)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Geben (Start)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Halten (Hold) 4 / Kassieren (Take)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Halten (Hold) 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Halten (Hold) 3 / Tief (Low)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("unknown bit 08") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Abschreib (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Game" )
	PORT_DIPNAME( 0x02, 0x02, "Remote Value" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )        // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
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
INPUT_PORTS_END


static INPUT_PORTS_START( novoplay )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Collect (D-UP) / Autohold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Deal/Draw / Double")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Bet / Half")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Service 1 / Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Service 2 / Select")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / High")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Low")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper Switch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Collect (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Credits / Coin" )
	PORT_DIPSETTING(    0x08, "10 Credits / Coin" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Game Type" )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, "Multi Card (without Jokers)" )
	PORT_DIPSETTING(    0x00, "Club Card (with Jokers)" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Autohold" )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Payout Mode" )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


static INPUT_PORTS_START( chinatow )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // no remote credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1 / Switch Bet (1-Max)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Bet / Prendi (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Gioca (Play) / Gmable")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5 / Half Gamble / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Counter" )              PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "X10" )
	PORT_DIPSETTING(    0x00, "X1" )
	PORT_DIPNAME( 0x04, 0x04, "Royal Flush" )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "With" )
	PORT_DIPSETTING(    0x00, "Without" )
	PORT_DIPNAME( 0x08, 0x08, "5 of a kind" )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, "With" )
	PORT_DIPSETTING(    0x00, "Without" )
	PORT_DIPNAME( 0x60, 0x60, "Payout type" )          PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Ticket + Hopper" )
	PORT_DIPSETTING(    0x20, "Ticket" )
	PORT_DIPSETTING(    0x40, "Hopper" )
	PORT_DIPSETTING(    0x60, "Ticket + Hopper" )
	PORT_DIPNAME( 0x90, 0x90, "Coin/Credit ratio" )    PORT_DIPLOCATION("SW1:1,4")
	PORT_DIPSETTING(    0x00, "1 coin 1 credit" )
	PORT_DIPSETTING(    0x10, "1 coin 1 credit" )
	PORT_DIPSETTING(    0x80, "1 coin 5 credits" )
	PORT_DIPSETTING(    0x90, "1 coin 10 credits" )
INPUT_PORTS_END


static INPUT_PORTS_START( rcdino4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // no remote credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Royal Flush" )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, "5 of a Kind" )           PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(    0x08, "1C-10C" )
	PORT_DIPSETTING(    0x10, "1C-1C" )
	PORT_DIPSETTING(    0x18, "1C-2C" )
	PORT_DIPSETTING(    0x00, "1C-5C" )
	PORT_DIPNAME( 0x60, 0x60, "Payment Type" )          PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Ticket + Hopper" )
	PORT_DIPSETTING(    0x20, "Ticket" )
	PORT_DIPSETTING(    0x40, "Hopper" )
	PORT_DIPSETTING(    0x60, "Ticket + Hopper" )
	PORT_DIPNAME( 0x80, 0x80, "Pagamenti (Payment)" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, "Pagamenti (Payment) A" )
	PORT_DIPSETTING(    0x00, "Pagamenti (Payment) B" )
INPUT_PORTS_END


static INPUT_PORTS_START( royal )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        // no remote credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1 / Switch Bet (1-Max)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Bet / Prendi (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Gioca (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5 / Half Gamble / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Allow to Boot" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fw_brick1 )
	PORT_INCLUDE( novoplay )
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x00, "Game Type" )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, "Multi Card (without Jokers)" )
	PORT_DIPSETTING(    0x00, "Joker Card (with Jokers)" )
INPUT_PORTS_END


static INPUT_PORTS_START( intrgmes )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Draw")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / High")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Low")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("I /7")      PORT_CODE(KEYCODE_E)  // I /7
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("F. C.")     PORT_CODE(KEYCODE_R)  // F.C.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper SW") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("CAV")       PORT_CODE(KEYCODE_Y)  // CAV.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Collect")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("SER. IN")   PORT_CODE(KEYCODE_A)  // SER. IN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("II/D")      PORT_CODE(KEYCODE_S)  // II/D
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("SER. SEL.") PORT_CODE(KEYCODE_D)  // SER. SEL.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("II/C")      PORT_CODE(KEYCODE_F)  // II/C
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("II/3")      PORT_CODE(KEYCODE_G)  // II/3
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("II/B")      PORT_CODE(KEYCODE_H)  // II/B
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("II/2")      PORT_CODE(KEYCODE_J)  // II/2

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )                 PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "55/77/99-Bonus" )            PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Language" )                  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "English" )
	PORT_DIPSETTING(    0x00, "German" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto Hold" )                 PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( novop_ab )
	PORT_INCLUDE( intrgmes )
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )                 PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Language" )                  PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "English" )
	PORT_DIPSETTING(    0x04, "Hungarian" )
	PORT_DIPSETTING(    0x08, "German" )
	PORT_DIPSETTING(    0x0c, "English" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Double Up Mode" )            PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x00, "High-Low-Red-Black" )
	PORT_DIPNAME( 0x40, 0x40, "Auto Hold" )                 PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( clubcard )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Bet / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / High")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Low")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Hopper SW")  PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          PORT_NAME("Coin B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_NAME("Coin A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "State" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Test Mode" )
	PORT_DIPSETTING(    0x01, "Gameplay" )
	PORT_DIPNAME( 0x02, 0x02, "Remote Value" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Royal Flush / 5 of a Kind Payment" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "150 / 150" )
	PORT_DIPSETTING(    0x10, "500 / 1100" )
	PORT_DIPNAME( 0x20, 0x20, "Joker" )             PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker (also enables 5OAK" )                 // also enables Five of a Kind
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x40, "Hold" )              PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

//  after nvram init, set the following one to 'manual'
//  to allow the remote credits mode to work
	PORT_DIPNAME( 0x80, 0x80, "Payout" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/******************************
* Graphics Decode Information *
******************************/

/* The palette system is adressable through a PLD.
   The game could have 2 different palettes, located
   in the first and second half of the bipolar PROM.
*/

static GFXDECODE_START( gfx_fw1stpal )  // Adressing the first half of the palette
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_fw2ndpal )  // Adressing the second half of the palette
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0x100, 16 )
GFXDECODE_END


/***********************
*    PIA Interfaces    *
***********************/

/*
    TAB light green board
    ---------------------

                PIA 0                                       PIA 1
         .--------u--------.                         .--------u--------.
         |01 VSS     CA1 40|- GND                    |01 VSS     CA1 40|- GND
         |02 PA0     CA2 39|- N/C                    |02 PA0     CA2 39|- RESET circuitery
         |03 PA1   /IRQA 38|- N/C                    |03 PA1   /IRQA 38|- 04 CPU (65C02)
         |04 PA2   /IRQB 37|- N/C                    |04 PA2   /IRQB 37|- N/C
         |05 PA3    /RS0 36|- 10 ROM(PRG)            |05 PA3    /RS0 36|- 10 ROM(PRG)
         |06 PA4    /RS1 35|- 09 ROM(PRG)            |06 PA4    /RS1 35|- 23 AY(SOUND)
         |07 PA5  /RESET 34|- 09 GAL/PAL             |07 PA5  /RESET 34|- 37 AY(SOUND)
         |08 PA6      D0 33|                         |08 PA6      D0 33|
         |09 PA7      D1 32|                         |09 PA7      D1 32|
         |10 PB0      D2 31|                         |10 PB0      D2 31|
         |11 PB1      D3 30|                         |11 PB1      D3 30|
         |12 PB2      D4 29|                         |12 PB2      D4 29|
         |13 PB3      D5 28|                         |13 PB3      D5 28|
         |14 PB4      D6 27|                         |14 PB4      D6 27|
         |15 PB5      D7 26|                         |15 PB5      D7 26|
         |16 PB6       E 25|                         |16 PB6       E 25|
         |17 PB7     CS1 24|                         |17 PB7     CS1 24|
    GND -|18 CB1    /CS2 23|                    N/C -|18 CB1    /CS2 23|
    GND -|19 CB2     CS0 22|                    N/C -|19 CB2     CS0 22|
         |20 VCC     R/W 21|                         |20 VCC     R/W 21|
         '-----------------'                         '-----------------'


    Novo Play Multi Card
    --------------------

                PIA 0                                       PIA 1
         .--------u--------.                         .--------u--------.
         |01 VSS     CA1 40|- GND                    |01 VSS     CA1 40|- GND
         |02 PA0     CA2 39|- GND                    |02 PA0     CA2 39|- N/C
         |03 PA1   /IRQA 38|- 65C02 (-IRQ)           |03 PA1   /IRQA 38|- 65C02 (-IRQ)
         |04 PA2   /IRQB 37|- N/C                    |04 PA2   /IRQB 37|- N/C
         |05 PA3    /RS0 36|- A0                     |05 PA3    /RS0 36|- A0
         |06 PA4    /RS1 35|- A1                     |06 PA4    /RS1 35|- A1
         |07 PA5  /RESET 34|- 65C02 (-RST)           |07 PA5  /RESET 34|- 65C02 (-RST)
         |08 PA6      D0 33|                         |08 PA6      D0 33|
         |09 PA7      D1 32|                         |09 PA7      D1 32|
         |10 PB0      D2 31|                         |10 PB0      D2 31|
         |11 PB1      D3 30|                         |11 PB1      D3 30|
         |12 PB2      D4 29|                         |12 PB2      D4 29|
         |13 PB3      D5 28|                         |13 PB3      D5 28|
         |14 PB4      D6 27|                         |14 PB4      D6 27|
         |15 PB5      D7 26|                         |15 PB5      D7 26|
         |16 PB6       E 25|                         |16 PB6       E 25|
         |17 PB7     CS1 24|                         |17 PB7     CS1 24|
    GND -|18 CB1    /CS2 23|                    N/C -|18 CB1    /CS2 23|
    GND -|19 CB2     CS0 22|                    N/C -|19 CB2     CS0 22|
         |20 VCC     R/W 21|                         |20 VCC     R/W 21|
         '-----------------'                         '-----------------'

*/


// these ports are set to output anyway, but this quietens the log
uint8_t funworld_state::funquiz_ay8910_a_r()
{
	return 0x00;
}

uint8_t funworld_state::funquiz_ay8910_b_r()
{
	return 0x00;
}


/********************************
*     Machine Start & Reset     *
********************************/

void lunapark_state::machine_start()
{
	funworld_state::machine_start();
	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0], 0x8000);
}

void lunapark_state::machine_reset()
{
	uint8_t seldsw = (ioport("SELDSW")->read() );
	popmessage("ROM Bank: %02X", seldsw);

	membank("bank1")->set_entry(seldsw);
}


/**************************
*     Machine Drivers     *
**************************/

void funworld_state::fw1stpal(machine_config &config)
{
	// basic machine hardware
	M65SC02(config, m_maincpu, CPU_CLOCK);  // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::funworld_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	pia6821_device &pia0(PIA6821(config, "pia0"));
	pia0.readpa_handler().set_ioport("IN0");
	pia0.readpb_handler().set_ioport("IN1");

	pia6821_device &pia1(PIA6821(config, "pia1"));
	pia1.readpa_handler().set_ioport("IN2");
	pia1.readpb_handler().set_ioport("DSW");
	pia1.ca2_handler().set(FUNC(funworld_state::pia1_ca2_w));

	// video hardware

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((124+1)*4, (30+1)*8);           // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1)
	screen.set_visarea(0*4, 96*4-1, 0*8, 29*8-1);   // Taken from MC6845 init, registers 01 & 06
	screen.set_screen_update(FUNC(funworld_state::screen_update_funworld));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fw1stpal);
	PALETTE(config, m_palette, FUNC(funworld_state::funworld_palette), 0x200);

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));    // 2 MHz, verified on jollycrd & royalcrd
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay8910(AY8910(config, "ay8910", SND_CLOCK)); // 2 MHz.
	ay8910.port_a_write_callback().set(FUNC(funworld_state::funworld_lamp_a_w));
	ay8910.port_b_write_callback().set(FUNC(funworld_state::funworld_lamp_b_w));
	ay8910.add_route(ALL_OUTPUTS, "mono", 2.5); // analyzed to avoid clips
}


void funworld_state::fw2ndpal(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::funworld_map);

	m_gfxdecode->set_info(gfx_fw2ndpal);
}



void funworld_state::funquiz(machine_config &config)
{
	fw1stpal(config);  // gray background.
//  fw2ndpal(config);  // blue background.

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::funquiz_map);

	subdevice<ay8910_device>("ay8910")->port_a_read_callback().set(FUNC(funworld_state::funquiz_ay8910_a_r));
	subdevice<ay8910_device>("ay8910")->port_b_read_callback().set(FUNC(funworld_state::funquiz_ay8910_b_r));
}


void magicrd2_state::magicrd2(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &magicrd2_state::magicrd2_map);

	subdevice<mc6845_device>("crtc")->set_visarea_adjust(0, -56, 0, 0);

	ay8910_device &ay8910(AY8910(config.replace(), "ay8910", SND_CLOCK));   // 2 MHz.
	ay8910.port_a_write_callback().set(FUNC(magicrd2_state::funworld_lamp_a_w));
	ay8910.port_b_write_callback().set(FUNC(magicrd2_state::funworld_lamp_b_w));
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.5); // analyzed to avoid clips
}


void funworld_state::royalcd1(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // (G65SC02P in pro version) 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::magicrd2_map);
}


void funworld_state::royalcd2(machine_config &config)
{
	fw2ndpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::magicrd2_map);
}


void funworld_state::cuoreuno(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::cuoreuno_map);
}


void funworld_state::saloon(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::saloon_map);

	config.device_remove("pia0");
	config.device_remove("pia1");

	// Serial Memory
	I2C_24C02(config, "i2cmem", 0).set_e0(1); // ? or maybe 2nd half of 24C04?
}


void funworld_state::witchryl(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::witchryl_map);
}


void lunapark_state::lunapark(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &lunapark_state::lunapark_map);  // mirrored video RAM (4000/5000 to 6000/7000).
}


void chinatow_state::chinatow(machine_config &config)
{
	fw2ndpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &chinatow_state::chinatow_map);
}

void chinatow_state::rcdino4(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &chinatow_state::chinatow_map);
}


void intergames_state::intrgmes(machine_config &config)
{
	fw1stpal(config);

	M65SC02(config.replace(), m_maincpu, CPU_CLOCK);    // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &intergames_state::intergames_map);
	//m_maincpu->set_periodic_int(FUNC(intergames_state::nmi_line_pulse), attotime::from_hz(60));

	m_gfxdecode->set_info(gfx_fw2ndpal);
}


void funworld_state::fw_brick_1(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::fw_brick_map);

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0);
}


void funworld_state::fw_brick_2(machine_config &config)
{
	fw2ndpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::fw_brick_map);

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0);
}


void funworld_state::gratispk(machine_config &config)
{
//  fw1stpal(config);  // card deck wrong colors.
	fw2ndpal(config);  // correct palette

	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::gratispk_map);
}


void funworld_state::clubcard(machine_config &config)
{
//  fw1stpal(config);  // 'alla Royal Card. Card deck wrong colors.
	fw2ndpal(config);  // proper colors.

	R65C02(config.replace(), m_maincpu, CPU_CLOCK); // 2 MHz.
	m_maincpu->set_addrmap(AS_PROGRAM, &funworld_state::clubcard_map);
}


uint8_t royalcrdf_state::royalcrdf_opcode_r(offs_t offset)
{
	// address-based data bitswap; 4 address bits are involved, but only
	// 5 different bitswaps exist, with clear regularities, so the
	// hardware is probably selecting the appropriate one by
	// applying passive logic to the address bits; we encode it
	// indexed by all the involved address bits instead. A notable fact is that
	// all the permutations in royalcrdf & multiwin are odd-parity ones,
	// so an intriguing possibility is that the hardware be applying
	// a fixed odd number (3 would suffice) of address-dependent
	// 1-bit-to-1-bit bitswaps; furthermore, in both sets one of the
	// bitswaps is equal to the one applied to all the data (but notice
	// that, in those cases, we are applying it twice to opcodes here)
	constexpr std::array<std::array<int, 4>, 16> bs{{ // bitswaps for data bits 1, 2, 5 & 7, in that order
		{ 1, 5, 2, 7 },
		{ 1, 5, 2, 7 },
		{ 5, 1, 7, 2 },
		{ 7, 2, 5, 1 },
		{ 1, 5, 2, 7 },
		{ 1, 5, 2, 7 },
		{ 5, 2, 1, 7 },
		{ 7, 2, 5, 1 },
		{ 1, 5, 2, 7 },
		{ 1, 5, 2, 7 },
		{ 5, 1, 7, 2 },
		{ 1, 7, 5, 2 },
		{ 1, 5, 2, 7 },
		{ 1, 5, 2, 7 },
		{ 5, 2, 1, 7 },
		{ 1, 7, 5, 2 }
	}};

	// xor masks, dependent on the same address bits than the bitswaps,
	// and with the same 5-values structure
	constexpr std::array<uint8_t, 16> xm {
		0x02, 0x02, 0xa6, 0x82, 0x02, 0x02, 0x06, 0x82, 0x02, 0x02, 0xa6, 0x00, 0x02, 0x02, 0x06, 0x00
	};

	uint8_t data {m_maincpu->space(AS_PROGRAM).read_byte(offset)};

	if(offset < 0x800)
		data = bitswap<8>(data ^ 0x22, 2, 6, 7, 4, 3, 1, 5, 0);

	unsigned idx {bitswap<4>(offset, 8, 5, 2, 1)};

	return bitswap<8>(data, bs[idx][3], 6, bs[idx][2], 4, 3, bs[idx][1], bs[idx][0], 0) ^ xm[idx];
}

void royalcrdf_state::royalcrdf_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(royalcrdf_state::royalcrdf_opcode_r));
}

void royalcrdf_state::driver_init()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int x = 0x8000; x < 0x10000; x++)
	{
		ROM[x] = bitswap<8>(ROM[x] ^ 0x22, 2, 6, 7, 4, 3, 1, 5, 0);
	}

/*  Temporary hack to modify the value that ends to be a NOP,
    after the BRK, and balanced checksums. Will be removed once
    found a workaround in the decryption routines.
*/
	ROM[0x802d] = 0xa2;  // checksum.
	ROM[0xbc4e] = 0x68;  // future NOP after BRK.
}

void royalcrdf_state::royalcrdf(machine_config &config)
{
	fw1stpal(config);

	R65C02(config.replace(), m_maincpu, CPU_CLOCK);  // to avoid the NMI into BRK bug...
	m_maincpu->set_addrmap(AS_PROGRAM, &royalcrdf_state::royalcrdf_map);

	m_maincpu->set_addrmap(AS_OPCODES, &royalcrdf_state::royalcrdf_opcodes_map);
}


uint8_t multiwin_state::multiwin_opcode_r(offs_t offset)
{
	// same general encryption scheme than the one used by the EVONA Royald Card set;
	// 4 address bits determine which bitswap+xor is applied to the opcodes; in this case,
	// one of the address bits don't have effect on the bitswap, just on the xor;
	// again, we have just five different bitswaps, and the hardware is probably using
	// passive logic on the address bits to do the selection
	constexpr std::array<std::array<int, 5>, 8> bs{{ // bitswaps for data bits 0, 2, 4, 5 & 7, in that order
		{ 5, 2, 4, 0, 7 },
		{ 4, 0, 2, 7, 5 },
		{ 7, 4, 5, 2, 0 },
		{ 4, 0, 2, 7, 5 },
		{ 2, 7, 0, 5, 4 },
		{ 4, 0, 2, 7, 5 },
		{ 0, 5, 7, 4, 2 },
		{ 4, 0, 2, 7, 5 }
	}};

	// xor masks
	constexpr std::array<uint8_t, 16> xm {
		0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x00, 0xb5, 0x10, 0xb5, 0x20, 0xb5, 0x30, 0xb5
	};

	uint8_t data {m_maincpu->space(AS_PROGRAM).read_byte(offset)};
	unsigned idx {bitswap<4>(offset, 6,9,5,3)};

	return bitswap<8>(data, bs[idx&7][4],6,bs[idx&7][3],bs[idx&7][2],3,bs[idx&7][1],1,bs[idx&7][0]) ^ xm[idx];
}

void multiwin_state::multiwin_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(multiwin_state::multiwin_opcode_r));
}

void multiwin_state::driver_init()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int x = 0x4000; x < 0x10000; x++)
	{
		ROM[x] = bitswap<8>(ROM[x] ^ 0x91, 5, 6, 7, 2, 3, 0, 1, 4);
	}
}

void multiwin_state::multiwin(machine_config &config)
{
	fw2ndpal(config);

	m_maincpu->set_addrmap(AS_OPCODES, &multiwin_state::multiwin_opcodes_map);
}


class multiwina_state : public funworld_state
{
public:
	multiwina_state(const machine_config &mconfig, device_type type, const char* tag):
		funworld_state(mconfig, type, tag),
		m_decode(*this, "decode")
	{
	}

	void multiwina(machine_config& config);

private:
	required_region_ptr<uint8_t> m_decode;

	uint8_t opcode_r(offs_t offset);

	void opcodes_map(address_map &map) ATTR_COLD;
};

uint8_t multiwina_state::opcode_r(offs_t offset)
{

	uint8_t data {m_maincpu->space(AS_PROGRAM).read_byte(offset)};
	return m_decode[(offset & 3) * 0x100 + data];
}

void multiwina_state::opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(multiwina_state::opcode_r));
}

void multiwina_state::multiwina(machine_config &config)
{
	fw2ndpal(config);

	m_maincpu->set_addrmap(AS_OPCODES, &multiwina_state::opcodes_map);
}


class jokercrd_state : public funworld_state
{
public:
	jokercrd_state(const machine_config &mconfig, device_type type, const char* tag)
	: funworld_state(mconfig, type, tag)
	{
	}

	void jokercrd(machine_config& config);

private:
	uint8_t jokercrd_opcode_r(offs_t offset);

	void jokercrd_opcodes_map(address_map &map) ATTR_COLD;
};

uint8_t jokercrd_state::jokercrd_opcode_r(offs_t offset)
{
	// even when errors could be lurking in the least used opcodes,
	// all of them in the [8050,b369) & [c000, f063) ranges are believed
	// to be covered by these tables, with the exception of the one @c0f1:

	//  c0da: a6 6a     ldx $6a
	//  c0dc: ad 5c 00  lda $005c
	//  c0df: 29 3f     and #$3f
	//  c0e1: 85 70     sta $70
	//  c0e3: 20 fa c0  jsr $c0fa
	//  c0e6: a5 70     lda $70
	//  c0e8: 9d 33 02  sta $0233, x
	//  c0eb: a2 00     ldx #$00
	//  c0ed: e4 6a     cpx $6a
	//  c0ef: f0 08     beq $c0f9
	//  c0f1: XX 33 02  nop #$33 #$02 [YYY $0233, x]
	//  c0f4: f0 e4     beq $c0da
	//  c0f6: e8        inx
	//  c0f7: 80 f4     bra $c0ed
	//  c0f9: 60        rts
	//
	//  c0fa: ad 4b 07  lda $074b
	//  c0fd: 0d 4c 07  ora $074c
	//  c100: d0 01     bne $c103
	//  c102: 60        rts
	//  c103: a5 70     lda $70
	//  c105: 29 30     and #$30
	//  c107: 09 01     ora #$01
	//  c109: 85 70     sta $70
	//  c10b: 60        rts

	// it should be noted, however, that the subroutine @c0da seems to be called just from here:
	//  c044: 64 6a     stz $6a
	//  c046: 20 da c0  jsr $c0da
	// and, if no interrupt is messing with the accesed data, the STZ @c044 should make the BEQ @c0ef
	// become an inconditional jump, converting the opcode @c0f1 in dead code


	constexpr uint8_t UNKN {0xfc};

	constexpr std::array<std::array<uint8_t, 256>, 4> decryption_tables{{
		//  _0    _1    _2    _3    _4    _5    _6    _7    _8    _9    _a    _b    _c    _d    _e    _f
		{ UNKN, UNKN, UNKN, 0x3d, 0xe0, UNKN, UNKN, 0xb0, UNKN, 0xa0, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN,  // 0_
		  UNKN, UNKN, 0x4a, 0x65, 0x18, 0x64, 0xad, UNKN, 0x90, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN,  // 1_
		  UNKN, UNKN, 0x49, UNKN, 0xa8, 0x8c, 0x20, UNKN, UNKN, 0xf8, UNKN, UNKN, 0xe6, UNKN, 0x88, UNKN,  // 2_
		  UNKN, UNKN, UNKN, UNKN, 0xb2, UNKN, UNKN, UNKN, UNKN, 0x58, 0x38, 0x45, UNKN, UNKN, UNKN, UNKN,  // 3_
		  UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0xd5, 0xa5, 0xa9, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN,  // 4_
		  UNKN, UNKN, UNKN, UNKN, 0xda, 0xa2, UNKN, 0x4e, 0x1a, UNKN, 0xe4, UNKN, UNKN, UNKN, 0x0c, 0x98,  // 5_
		  0xa6, UNKN, UNKN, UNKN, 0x9c, 0xfa, UNKN, 0xbd, UNKN, 0xae, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN,  // 6_
		  UNKN, UNKN, 0xc0, 0xc5, UNKN, UNKN, UNKN, UNKN, UNKN, 0xcd, UNKN, UNKN, UNKN, 0x8a, UNKN, 0x80,  // 7_
		  UNKN, UNKN, 0x95, UNKN, UNKN, UNKN, UNKN, UNKN, 0x8e, UNKN, UNKN, UNKN, UNKN, 0x68, UNKN, UNKN,  // 8_
		  0xb1, UNKN, UNKN, UNKN, UNKN, 0xd0, 0x0d, 0xc9, UNKN, UNKN, UNKN, UNKN, 0x29, 0x3a, 0xc8, 0x8d,  // 9_
		  0xc6, 0xee, 0x25, UNKN, 0x05, 0x6e, UNKN, 0x2a, UNKN, 0x85, UNKN, UNKN, 0xd8, UNKN, 0xce, UNKN,  // a_
		  UNKN, UNKN, UNKN, 0xe9, 0xc4, 0xe8, UNKN, UNKN, 0x99, 0x6a, 0x0a, UNKN, 0x84, UNKN, UNKN, UNKN,  // b_
		  UNKN, UNKN, UNKN, 0x9d, 0x2d, 0x10, UNKN, UNKN, UNKN, UNKN, UNKN, 0xaa, UNKN, UNKN, 0x11, UNKN,  // c_
		  UNKN, 0xb9, 0x86, UNKN, 0xca, UNKN, UNKN, UNKN, UNKN, UNKN, 0x7a, UNKN, 0xf0, 0x09, 0x4c, UNKN,  // d_
		  0x60, 0x78, UNKN, UNKN, UNKN, 0x48, 0x5a, UNKN, UNKN, UNKN, 0x79, UNKN, UNKN, UNKN, UNKN, 0xba,  // e_
		  UNKN, UNKN, 0x6d, 0x92, UNKN, 0x26, UNKN, UNKN, 0xb5, UNKN, 0x69, UNKN, UNKN, 0xa4, UNKN, 0x1d },// f_

		//  _0    _1    _2    _3    _4    _5    _6    _7    _8    _9    _a    _b    _c    _d    _e    _f
		{ UNKN, UNKN, UNKN, UNKN, 0x48, UNKN, UNKN, 0x0c, UNKN, 0x60, 0x6d, 0x20, 0xda, UNKN, 0xf0, UNKN,  // 0_
		  UNKN, 0xcd, 0x95, UNKN, UNKN, 0xae, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x8e, UNKN, 0x29, UNKN,  // 1_
		  0x90, UNKN, UNKN, UNKN, 0x69, 0xb5, UNKN, UNKN, UNKN, UNKN, 0xd0, 0xad, UNKN, UNKN, UNKN, 0x9a,  // 2_
		  0xb6, 0x40, UNKN, 0xd5, 0xfa, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0xa4, UNKN, 0x88, UNKN,  // 3_
		  UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x2a, UNKN, UNKN, UNKN, 0xa2, UNKN, UNKN, UNKN, UNKN, UNKN,  // 4_
		  0xee, UNKN, UNKN, UNKN, 0x64, UNKN, 0xb0, 0x9d, UNKN, UNKN, UNKN, 0x80, UNKN, 0xf8, 0xe8, UNKN,  // 5_
		  UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x91, UNKN, 0xb9, 0x8a, UNKN, UNKN, UNKN, 0x10, UNKN,  // 6_
		  0x4a, UNKN, UNKN, 0xc8, 0x9c, 0x4e, UNKN, UNKN, UNKN, UNKN, 0xb2, 0x85, 0x45, 0xc9, UNKN, UNKN,  // 7_
		  0x1a, UNKN, UNKN, 0xe9, UNKN, UNKN, UNKN, UNKN, UNKN, 0xca, UNKN, 0xc6, UNKN, 0xb1, UNKN, UNKN,  // 8_
		  0x1d, UNKN, 0xa8, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0xc5,  // 9_
		  0xe0, UNKN, 0xa0, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x30, UNKN, 0x0d, UNKN, 0x8d,  // a_
		  UNKN, 0x86, UNKN, UNKN, 0xd9, 0xbd, UNKN, UNKN, 0x05, 0xaa, UNKN, 0x38, UNKN, 0x18, 0xe4, UNKN,  // b_
		  UNKN, 0xa6, UNKN, 0xa5, 0xc0, 0x2d, UNKN, UNKN, UNKN, UNKN, UNKN, 0x4c, 0x84, UNKN, 0x6e, UNKN,  // c_
		  UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x19, 0x65, UNKN, UNKN, UNKN, 0xce, 0x7a, UNKN, 0x3a,  // d_
		  UNKN, 0x7d, UNKN, 0x0a, 0x49, UNKN, UNKN, UNKN, 0xe6, 0xd8, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN,  // e_
		  UNKN, UNKN, UNKN, UNKN, 0x09, 0xa9, 0x25, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x5a, UNKN, 0x68 },// f_

		//  _0    _1    _2    _3    _4    _5    _6    _7    _8    _9    _a    _b    _c    _d    _e    _f
		{ UNKN, 0x20, UNKN, UNKN, UNKN, 0x8e, UNKN, UNKN, UNKN, 0xb0, 0x85, 0x3d, UNKN, 0x86, UNKN, UNKN,  // 0_
		  0x18, UNKN, UNKN, UNKN, 0x8a, 0xad, 0x79, 0x98, UNKN, UNKN, 0xc0, UNKN, 0x30, UNKN, 0x10, UNKN,  // 1_
		  UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0xe4, UNKN, UNKN, UNKN, 0x29, UNKN, UNKN, UNKN, UNKN,  // 2_
		  0x9e, 0x6d, 0xa6, UNKN, 0xc9, 0x6e, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN,  // 3_
		  UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0xaa, UNKN, 0x9c, 0x09, UNKN, UNKN, UNKN, 0xfa, UNKN,  // 4_
		  UNKN, 0x69, UNKN, UNKN, 0x2d, UNKN, UNKN, UNKN, UNKN, UNKN, 0xc4, UNKN, 0x1a, 0x4c, 0x5a, UNKN,  // 5_
		  UNKN, UNKN, 0xe9, UNKN, 0xb1, UNKN, 0x0c, 0xee, 0xda, 0xb2, 0xce, UNKN, 0x74, UNKN, UNKN, UNKN,  // 6_
		  0x7d, 0x0d, 0x48, UNKN, UNKN, UNKN, 0xf8, 0xd8, UNKN, UNKN, 0x0a, UNKN, UNKN, 0x45, 0xc6, UNKN,  // 7_
		  UNKN, UNKN, UNKN, UNKN, UNKN, 0x26, UNKN, UNKN, UNKN, 0xc5, UNKN, UNKN, UNKN, 0xae, UNKN, UNKN,  // 8_
		  0xa9, UNKN, 0x90, 0x3a, UNKN, UNKN, 0x6c, UNKN, 0x64, UNKN, UNKN, 0xd5, UNKN, UNKN, UNKN, UNKN,  // 9_
		  UNKN, 0x7a, UNKN, 0x05, 0xa8, UNKN, UNKN, 0x60, UNKN, UNKN, 0xa4, 0x8d, 0x2a, UNKN, UNKN, UNKN,  // a_
		  UNKN, UNKN, UNKN, UNKN, 0xa0, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x92, 0x66,  // b_
		  UNKN, UNKN, 0xd0, 0xf0, UNKN, UNKN, 0x38, 0xbd, 0xa2, UNKN, 0xed, 0x9d, 0x80, UNKN, 0xa5, UNKN,  // c_
		  UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x99, UNKN, 0x88, UNKN, 0xcd, UNKN, 0xe8,  // d_
		  0xca, 0xb5, 0x49, 0x68, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0xe6, 0xe0, 0x4a, UNKN,  // e_
		  UNKN, UNKN, UNKN, 0xc8, UNKN, 0xb9, 0x91, UNKN, UNKN, UNKN, UNKN, 0x65, 0x95, UNKN, UNKN, UNKN },// f_

		//  _0    _1    _2    _3    _4    _5    _6    _7    _8    _9    _a    _b    _c    _d    _e    _f
		{ 0x65, UNKN, UNKN, UNKN, 0x30, UNKN, UNKN, UNKN, 0x8e, UNKN, UNKN, 0xda, 0x2d, 0xc4, UNKN, 0x68,  // 0_
		  UNKN, 0xa2, UNKN, UNKN, 0x2a, 0xa6, UNKN, UNKN, UNKN, 0x86, UNKN, UNKN, 0x60, UNKN, 0xe6, 0xa4,  // 1_
		  UNKN, 0x84, UNKN, UNKN, 0xbd, UNKN, UNKN, UNKN, UNKN, UNKN, 0xce, 0xe4, 0xf8, 0xa0, UNKN, 0x8d,  // 2_
		  UNKN, 0xcd, UNKN, UNKN, UNKN, 0x88, 0x4a, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN,  // 3_
		  0xd8, UNKN, UNKN, UNKN, 0x40, 0x11, UNKN, UNKN, 0xaa, 0x79, UNKN, 0x98, UNKN, 0x85, UNKN, UNKN,  // 4_
		  0x9a, 0xc6, UNKN, UNKN, 0x74, UNKN, UNKN, UNKN, UNKN, 0x95, 0xdd, UNKN, UNKN, 0x6d, UNKN, UNKN,  // 5_
		  0x8a, 0x6e, UNKN, UNKN, 0x18, 0xc5, UNKN, 0xf9, UNKN, 0xb0, 0x7a, UNKN, UNKN, UNKN, UNKN, 0xc0,  // 6_
		  0x99, UNKN, UNKN, 0x10, 0x90, 0x38, 0x09, UNKN, UNKN, UNKN, 0xe0, UNKN, UNKN, UNKN, UNKN, UNKN,  // 7_
		  UNKN, UNKN, UNKN, 0xb5, UNKN, 0x4c, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0xfa, UNKN, UNKN,  // 8_
		  UNKN, UNKN, 0x6a, 0x29, UNKN, UNKN, 0x5a, 0xae, UNKN, 0x3a, UNKN, 0x9c, UNKN, 0x0d, 0x49, 0xee,  // 9_
		  UNKN, UNKN, UNKN, UNKN, 0xa5, 0x69, 0xf0, UNKN, 0x80, UNKN, 0xc8, UNKN, UNKN, UNKN, 0xb9, UNKN,  // a_
		  0x1a, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, UNKN, 0x0c, UNKN, UNKN, UNKN, UNKN, UNKN,  // b_
		  UNKN, UNKN, UNKN, UNKN, 0x64, UNKN, UNKN, 0xb2, 0x9d, UNKN, UNKN, 0xca, UNKN, UNKN, 0x0a, 0x05,  // c_
		  UNKN, UNKN, UNKN, 0x91, UNKN, UNKN, UNKN, 0x92, UNKN, UNKN, UNKN, 0xed, UNKN, UNKN, UNKN, UNKN,  // d_
		  UNKN, 0xec, 0x20, UNKN, 0xad, 0xd0, UNKN, 0x3d, UNKN, 0xb1, 0x6c, 0x66, UNKN, 0x4e, UNKN, 0xd5,  // e_
		  0xe9, UNKN, UNKN, 0xa9, UNKN, UNKN, 0xe8, UNKN, UNKN, 0xc9, 0xa8, UNKN, UNKN, 0x48, UNKN, UNKN } // f_
	}};

	uint8_t data {m_maincpu->space(AS_PROGRAM).read_byte(offset)};
	return decryption_tables[offset & 3][data];
}

void jokercrd_state::jokercrd_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(jokercrd_state::jokercrd_opcode_r));
}

void jokercrd_state::jokercrd(machine_config &config)
{
	fw2ndpal(config);

	m_maincpu->set_addrmap(AS_OPCODES, &jokercrd_state::jokercrd_opcodes_map);
}



/*************************
*        Rom Load        *
*************************/

/******************************** Jolly Card sets ************************************/

/*
    Jolly Card, TAB Austria
    -----------------------

    Pinouts:

    X1-01   GND                     X1-A    GND
    X1-02   GND                     X1-B    GND
    X1-03   GND                     X1-C    GND
    X1-04   +5V                     X1-D    +5V
    X1-05   +12V                    X1-E    +12V
    X1-06   NC                      X1-F    NC
    X1-07   NC                      X1-G    NC
    X1-08   NC                      X1-H    NC
    X1-09   NC                      X1-I    Coin 1
    X1-10   Coin 2                  X1-J    Pay Out SW
    X1-11   Hold 3                  X1-K    NC
    X1-12   NC                      X1-L    GND
    X1-13   Hold 4                  X1-M    Remote
    X1-14   Bookkeeping SW          X1-N    GND
    X1-15   Hold 2                  X1-O    Cancel
    X1-16   Hold 1                  X1-P    Hold 5
    X1-17   Start                   X1-Q
    X1-18   Hopper Out              X1-R
    X1-19   Red                     X1-S    Green
    X1-20   Blue                    X1-T    Sync
    X1-21   GND                     X1-U    Speaker GND
    X1-22   Speaker +               X1-V    Management SW

    X2-01   GND                     X2-A    GND
    X2-02   NC                      X2-B    NC
    X2-03   Start                   X2-C    NC
    X2-04   NC                      X2-D    NC
    X2-05   NC                      X2-E    NC
    X2-06   Lamp Start              X2-F    Lamp Hold 1+3
    X2-07   Lamp Hold 2             X2-G    Lamp Hold 4
    X2-08   Lamp Hold 5             X2-H    Lamp Cancel
    X2-09   NC                      X2-I    NC
    X2-10   Counter In              X2-J    Counter Out
    X2-11   Hopper Count            X2-K    Hopper Drive
    X2-12   Counter Remote          X2-L
    X2-13                           X2-M
    X2-14                           X2-N
    X2-15   NC                      X2-O
    X2-16                           X2-P
    X2-17                           X2-Q    Coin Counter
    X2-18                           X2-R

    ---------------------------------------------------

    DIP Switches:

        ON                  OFF

    1   Hopper              Manual Payout SW    :Payout
    2   Auto Hold           No Auto Hold        :Hold
    3   Joker               Without Joker       :Joker
    4   Dattl Insert        TAB Insert          :Inserts
    5   5 Points/Coin       10 Points/Coin      :Coin 1
    6   5 Points/Coin       10 Points/Coin      :Coin 2
    7   10 Points/Pulse     100 Points/Pulse    :Remote
    8   Play                Keyboard Test       :Mode

    ---------------------------------------------------
*/

ROM_START( jollycrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jolycard.run", 0x8000, 0x8000, CRC(f531dd10) SHA1(969191fbfeff4349afef619d9241ef5186e6d57f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jollycrd_nvram.bin", 0x0000, 0x0800, CRC(8f0a86c9) SHA1(467bd4f601ac6aa818c036f1269c2d43d27854f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END


/*
  Jolly Card (German)

  1x R65C02P2
  2x ST EF68B21P
  1x UM6845B
  1x Microchip AY-3-8910A
  1x Intel P5C090-50 (24 Macrocell CMOS PLD, DIP40)

  1x HY6264LP-15
  1x MK6116LN-15
  3x ST M27C256B
  1x 82S147 Bipolar PROM.
  1x GAL16V8S

  1x Xtal 16.000 MHz.
  1x 3.6V lithium battery.
  1x 8 DIP switches bank.

*/
ROM_START( jollycrda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pok14.bin", 0x8000, 0x8000, CRC(30936afb) SHA1(5b96cd7e425ad79163e1d55e530cbabcb116f8d2) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "tab3.bin", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "tab2.bin", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM. the game doesn't work without it
	ROM_LOAD( "jollycrda_nvram.bin", 0x0000, 0x0800, CRC(5cbb4d8f) SHA1(da5edbef20bb4f0c634939389b3a9744c5743641) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "jop.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 ) // cracked PLD
	ROM_LOAD( "tab1.0_gal16v8s.bin", 0x0000, 0x0117, CRC(574f9a48) SHA1(bb5e2e86da85130d92f61cc57038a844950f443e) )
ROM_END


ROM_START( jolyc3x3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jc3x3.bin", 0x8000, 0x8000, CRC(71e304ad) SHA1(238b792d841432582c94b21a674d46a95e8f3826) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyc3x3_nvram.bin", 0x0000, 0x0800, CRC(727c70cf) SHA1(3639b0891514064e21ebdb280791cc9c8f5ba481) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END


/*

  Jolly Card Professional 2.0
  ---------------------------

  Special version with maximum bet up to 98/980 credits.
  This game has an annoying password system through a DOS program.


  differences:
  ----------------------------------------------------------

  MZS Soft:

  - All manufacturer strings, phones and dates.
  - Doesn't allow to clean partial IN/OUT meters.

    $9FFF  LDX #$EA      ; Just clean 24 bytes from memory.
    $A001  LDA #$00
    $A003  STA $0300,X
    $A006  INX
    $A007  BNE $A003
    $A009  RTS

  - The following unknown bytes...

    $47EB - $47FF: 05 05 01 04 04  ; Unknown.

  ----------------------------------------------------------

  Spale Soft:

  - All manufacturer strings, phones and dates.

  - Allow to clean partial IN/OUT meters.

    $9FFF  LDX #$00      ; Just clean 255 bytes from memory.
    $A001  LDA #$00
    $A003  STA $0300,X
    $A006  INX
    $A007  BNE $A003
    $A009  RTS

  - The following unknown bytes...

    $47EB - $47FF: 00 00 00 00 00  ; Unknown.

  ----------------------------------------------------------

  034E-034F: Partial IN.
  0360-0361: Partial OUT.
  044E-044F: Total IN.
  0460-0461: Total OUT.

*/

ROM_START( jolycmzs )
	ROM_REGION( 0x10000, "maincpu", 0 ) // MZS Tech
	ROM_LOAD( "mzstech.bin", 0x8000, 0x8000, CRC(cebd1e56) SHA1(24d88b3383cecf3829556d75460053663aab4ef1) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

/*  Load a default NVRAM, otherwise an annoying password system appears
    generating a code that needs to be parsed by a PC-DOS program, that
    is currently missing and never would appear. This program is supossed
    to give back an hexa code to feed in the game through a combination
    of inputs.
*/
	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycmzs_nvram.bin", 0x0000, 0x0800, CRC(828ffeef) SHA1(6a52282231f1944ba79049f267a9c1941373aea2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolyc980 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Spale Soft
	ROM_LOAD( "j980.bin", 0x8000, 0x8000, CRC(48249fff) SHA1(390cd0eb3399446a66363dc6760458170e1970fd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

/*  Load a default NVRAM, otherwise an annoying password system appears
    generating a code that needs to be parsed by a PC-DOS program, that
    is currently missing and never would appear. This program is supossed
    to give back an hexa code to feed in the game through a combination
    of inputs.
*/
	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyc980_nvram.bin", 0x0000, 0x0800, CRC(ef2c89c7) SHA1(a286001e205dcd16d914e07ba2b7c820335ab9c2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END


ROM_START( jolycdev )   // Jolly Card (lipa) from Evona
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lipa.bin", 0x8000, 0x8000, CRC(62657c98) SHA1(237466dde26540c119c631c75f51c87ea59d1a91) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jollyb.bin", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jollya.bin", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdev_nvram.bin", 0x0000, 0x0800, CRC(ae2c2fb8) SHA1(912d673ea8e26ff62520a7b86e702e5260cff4c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END


/*
    Jolly Card (other)
    ------------------

    - 1x G65SC02P (CPU)
    - 1x MC68B45P (CRTC)
    - 1x AY3-8910 (sound)
    - 2x MC6821P  (PIAs)

    RAM:  - 1x NVram DS1220Y (instead of 6116)
          - 1x KM6264AL-10

    - 1x Crystal : 16.000 MHz

    These croatian sets are not meant to work with coins...
    Only remote credits, as can be seen in test mode.
*/

ROM_START( jolyccra )   // Jolly Card (Croatian, set 1)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jollyc.bin", 0x8000, 0x8000, CRC(8926d99d) SHA1(dd5d1ac03d30d823dfcfe1349328ecb7afbc37fa) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jollyb.bin", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jollya.bin", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyccra_nvram.bin", 0x0000, 0x0800, CRC(478ab0a9) SHA1(8c5160d2ac8d4a9db0ae1e478d3bb1513a04544f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END


ROM_START( jolyccrb )   // Jolly Card (Croatian, set 2)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jollypkr.003", 0x8000, 0x8000, CRC(ea7340b4) SHA1(7dd468f28a488a4781521809d06db1d7917048ad) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyccrb_nvram.bin", 0x0000, 0x0800, CRC(c1d49c88) SHA1(27ffdedfc7f09ff11c3b2537db3681473b776074) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END


/*
    Jolly Card (italian, blue Tab board, encrypted)
    -----------------------------------------------

    - 1x HY6264LP
    - 1x MC6845P
    - 1x HM6116LP
    - 1x G65SC02 (main)
    - 1x AY-3-8910 (sound)
    - 1x MC6821P
    - 1x oscillator 16.000 MHz
    - ROMs  2x TMS27c256 (1,2)
            1x M5M27256 (jn)
    - 1x prom N82S147
    - 1x GAL16V8B
    - 2x HY18CV85 (electrically-erasable PLD)
    - 1x 8 DIP switches
    - 1x 22x2 edge connector
    - 1x 18x2 edge connector
    - 1x trimmer (volume)(missing)
*/

ROM_START( jolycdit )   // blue TAB PCB, encrypted graphics
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jn.bin", 0x8000, 0x8000, CRC(6ae00ed0) SHA1(5921c2882aeb5eadd0e04a477fa505ad35e9d98c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(46805150) SHA1(63687ac44f6ace6d8924b2629536bcc7d3979ed2) )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(43bcb2df) SHA1(5022bc3a0b852a7cd433e25c3c90a720e6328261) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdit_nvram.bin", 0x0000, 0x0800, CRC(c55c6706) SHA1(a38ae926f057fb47e48ca841b2d097fc4fd06416) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.bin", 0x0000, 0x0117, CRC(3ad712b1) SHA1(54214841fb178e4b59bf6051522718f7667bad28) )
ROM_END


/*
    Jolly Card (Italian encrypted bootleg)
    --------------------------------------

    - 1x UM6845P
    - 1x R65C02P3 (main CPU)
    - 1x AY-3-8910 (sound)
    - 2x MC68B21P
    - 1x oscillator 16.000 MHz
    - ROMs  3x 27256.
    - 1x PROM M1-7649-5 (not dumped)
    - 2x PLD HY18CV8S (not dumped)
    - 1x PAL16L8ACN (not dumped)
    - 1x 22x2 edge connector
    - 1x 18x2 edge connector
    - 1x trimmer (volume)
    - 1x 8x2 DIP switches.

    jolycdit vs jolycdib:

    1.BIN                   1.BIN                   IDENTICAL
    2.BIN                   2.bin                   IDENTICAL
    jn.BIN                  3.BIN                   97.726440%

    jolycdib program seems to be the original for blue TAB PCB.
    jolycdit has some code patches and redirected parts to suspicious offsets (as d500, d000, etc)

    This sets is not meant to work with coins...
    Only remote credits, as can be seen in test mode.
*/

ROM_START( jolycdib )   // bootleg PCB, encrypted graphics
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin", 0x8000, 0x8000, CRC(c76fdc79) SHA1(fc75c274d64fa9c99a546d424f38e79f1acf2576) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(46805150) SHA1(63687ac44f6ace6d8924b2629536bcc7d3979ed2) ) // sldh
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(43bcb2df) SHA1(5022bc3a0b852a7cd433e25c3c90a720e6328261) ) // sldh

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdib_nvram.bin", 0x0000, 0x0800, CRC(038a71fe) SHA1(99d3befbee8f9f86ce7f074de7f16fb25053c077) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.bin", 0x0000, 0x0117, CRC(3ad712b1) SHA1(54214841fb178e4b59bf6051522718f7667bad28) )
ROM_END


/*
    Jolly Card (Italian encrypted bootleg, set 2)
    ---------------------------------------------

    This program works in both 'normal' and 'remote' modes,
    allowing to pay through hopper, or just through manual
    switch/button.

    To initialize the NVRAM, DIP switches should be placed
    at factory default: (On On On On On On On Off), then
    keep pressed both service buttons (key 9 & 0), reset
    using F3 key, and then finally releasing the service
    buttons.
*/

ROM_START( jolycdic )   // another bootleg PCB, encrypted graphics
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3ss.bin", 0x8000, 0x8000, CRC(3d7cde61) SHA1(6154878491f4d2f1ea035d18cdf43154c550d509) )
	ROM_IGNORE(                  0x8000 )   // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(32c24495) SHA1(0d78c4d2743401b5ec9919d09814064eeac8023f) ) // sldh
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(91093176) SHA1(b889c617f94161933c35c324c7d84fec182953d8) ) // sldh
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdic_nvram.bin", 0x0000, 0x0800, CRC(47a5890b) SHA1(6a6531fe5e8f6c1b5a9aac314b2cce6a0129f6da) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.bin", 0x0000, 0x0117, NO_DUMP ) // sldh
ROM_END

/*
  Jolly Card (Italian encrypted bootleg, set 3)
  ---------------------------------------------

  Running in a Cleco bootleg board.
  Encrypted GFX.

  Differences: 1 byte.

         jolycdib     jolycdic

  C1D6:    0x60         0x10

  C1E5:  lda #$60     lda #$10
  C1E7:  sta $0000    sta $0000
  C1EA:  sta $0003    sta $0003
  C1ED:  sta $07f8    sta $07f8

*/
ROM_START( jolycdii )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "145-3.bin", 0x8000, 0x8000, CRC(812f88f1) SHA1(d387b7e4818dc6f5f2b6161a623c6e4f1eeedf2a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "145-2.bin", 0x0000, 0x8000, CRC(46805150) SHA1(63687ac44f6ace6d8924b2629536bcc7d3979ed2) )
	ROM_LOAD( "145-1.bin", 0x8000, 0x8000, CRC(43bcb2df) SHA1(5022bc3a0b852a7cd433e25c3c90a720e6328261) )

	ROM_REGION( 0x0800, "nvram", 0 )    // Default NVRAM
	ROM_LOAD( "jolycdii_nvram.bin", 0x0000, 0x0800, CRC(c2a138ad) SHA1(463fed8b495fa5286513f52fa09cf97f29e5f40e) )

	ROM_REGION( 0x0800, "proms", 0 )    // Not a bipolar PROM. Just a replacement.
	ROM_LOAD( "ws57c291_d27hc65d.bin", 0x0000, 0x0200, CRC(d96f3760) SHA1(07e80baf9c6c08171195b7810261e043197a9c3c) )
	ROM_IGNORE(                                0x0600 ) // empty

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.bin", 0x0000, 0x0117, NO_DUMP )
ROM_END


/* Jolly Card Italian bootleg...

   This PCB has an Altera EP910PC CPLD on board

   To init NVRAM, DSW must be:
   OFF ON ON ON ON ON ON ON

   Then (SERVICE1 + SERVICE2) and reset,
   then another reset.

   Q is remote (x100)
   W is payout.

-------------------------------------

   Complete specs:

   1x Empty CPU place (surely Rockwell 65C02 bitwise family)

   1x GI AY-3-8910 Sound Processor.

   2x Motorola MC68B21P for I/O.
   1x GoldStar GM68B45S CRTC.
   1x Altera EP910PC-40.

   1x KM6264AL-10
   1x Dallas DS1220Y nonvolatile SRAM.

   2x M27C512 (ROMs 6 & 7)
   1x MBM27256 (ROM 5)

   1x N82S147N Bipolar PROM (undumped)

   1x Xtal 16 MHz.
   1x 8 DIP Switches bank.
   1x Jamma edge connector.
   1x Unknown POT between two caps, maybe for sound amp stage.

*/
ROM_START( jolycdid )   // Altera EP910PC CPLD
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.cpu", 0x8000, 0x8000, CRC(56158851) SHA1(abf1daad1198dcc017352742e3c00d57e8955bd4) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "7.bin", 0x0000, 0x8000, CRC(a4452751) SHA1(a0b32a8801ebaee7ede7873b244f1a424433fe94) )
	ROM_CONTINUE( 0x0000, 0x8000)   // Discarding 1nd half. 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "6.bin", 0x8000, 0x8000, CRC(8b64d4c6) SHA1(8106cba31cd3fbda0855e6070182d248e3d52495) )
	ROM_CONTINUE( 0x8000, 0x8000)   // Discarding 1nd half. 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdid_nvram.bin", 0x0000, 0x0800, CRC(6eb66015) SHA1(39490cf5d404c9e9fb58439f6d9876a3e9b29ba0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(fc9a8aa3) SHA1(6f0a98bd0c7a64281bb1cce35de11b76978d7123) ) // sldh
ROM_END


/*  Jolly Card Italian bootleg...

   To init NVRAM, DSW must be:
   OFF ON ON ON ON ON ON ON

   Then (SERVICE1 + SERVICE2) and reset,
   then another reset.

   This set has spam graphics, but seems
   used by another program.

*/
ROM_START( jolycdie )   // Bootleg PCB, NON encrypted graphics
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aic.cpu", 0x8000, 0x8000, CRC(56158851) SHA1(abf1daad1198dcc017352742e3c00d57e8955bd4) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "c.bin", 0x0000, 0x8000, CRC(eff5362c) SHA1(1c9a48866dc5ee37fad6d68465f326d243c821c3) )
	ROM_LOAD( "b.bin", 0x8000, 0x8000, CRC(98309e14) SHA1(0d3e3766768fafc728a08668ad693f950d1fabab) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdie_nvram.bin", 0x0000, 0x0800, CRC(1b2fba44) SHA1(be5c956517072581edaebe9ae440a542964c8490) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(fc9a8aa3) SHA1(6f0a98bd0c7a64281bb1cce35de11b76978d7123) ) // sldh
ROM_END

/* Jolly Card Italian bootleg...

   This PCB has an Altera EP910PC CPLD on board

   This set doesn't need any NVRAM initialization.

   5 & 6 are coins.
   W is payout.
*/
ROM_START( jolycdif )   // Altera EP910PC CPLD
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(80983f6a) SHA1(222ce5d4800887de92a73decbad31e96f8da3b4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(a4452751) SHA1(a0b32a8801ebaee7ede7873b244f1a424433fe94) )
	ROM_CONTINUE( 0x0000, 0x8000)   // Discarding 1nd half. 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "3.bin", 0x8000, 0x8000, CRC(2856c82d) SHA1(7ce835bc2246ffede180cff0d8d0d4528afcc297) )
	ROM_CONTINUE( 0x8000, 0x8000)   // Discarding 1nd half. 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) ) // proper dump
ROM_END


/* Jolly Card Italian bootleg...

   This PCB has a TI EP910DC-30 CPLD,
   like others italian bootlegs that
   have an Altera EP910PC CPLD on board.

   To init NVRAM, DSW must be:
   OFF ON ON ON ON ON ON ON

   Then (SERVICE1 + SERVICE2) and reset,
   then another reset.

   Once NVRAM is initialized, the hardware still complains about it.
   The code checks for values 0x45 & 0x56 at ROM space offsets $E825
   and $E826. If these values are there, the code jumps to $C8C2,
   where the game stucks with a hardware error message.

   C8B3: LDA $E825    AD 25 E8  ; load the contents of $E825.
   C8B6: CMP #$45     C9 45     ; compare with #$45, ASCII "E".
   C8B8: BEQ $C8C2    F0 08     ; if true, jumps to $C8C2.
   C8BA: LDA $E826    AD 26 E8  ; load the contents of $E826.
   C8BD: CMP #$56     C9 56     ; compare with #$56, ASCII "V".
   C8BF: BEQ $C8C2    F0 01     ; if true, jumps to $C8C2.
   C8C1: RTS          60        ; returns from subroutine.
   ....
   C8C2: JMP $C0D8    4C D8 C0  ; jumps to hardware error.

   Maybe something is mapped virtual or physically there that
   shows different values that the ones from the ROM.

*/
ROM_START( jolycdig )
	ROM_REGION( 0x10000, "maincpu", 0 )  // TI EP910DC-30 CPLD (DIP-40 Ceramic)
	ROM_LOAD( "3__27c256.bin", 0x8000, 0x8000, CRC(e939ed2a) SHA1(a413623499971517569bcf97860caad65ff5535f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2__27c512.bin", 0x0000, 0x8000, CRC(a4452751) SHA1(a0b32a8801ebaee7ede7873b244f1a424433fe94) )
	ROM_IGNORE(                        0x8000)    // discarding 2nd half (identical halves)
	ROM_LOAD( "1__27c512.bin", 0x8000, 0x8000, CRC(8b64d4c6) SHA1(8106cba31cd3fbda0855e6070182d248e3d52495) )
	ROM_IGNORE(                        0x8000)   // discarding 2nd half (identical halves)

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdig_nvram.bin", 0x0000, 0x0800, CRC(a85854cb) SHA1(601d45f936626a8df7ffe8e36aef17416bbb7cca) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "27s29pc.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolycdih )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(3b2c237c) SHA1(08f646399c17747d6b9b8e86bb549dde87d26ba4) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.bin", 0x0000, 0x8000, CRC(a4452751) SHA1(a0b32a8801ebaee7ede7873b244f1a424433fe94) )
	ROM_CONTINUE( 0x0000, 0x8000)   // Discarding 1nd half. 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "2.bin", 0x8000, 0x8000, CRC(8b64d4c6) SHA1(8106cba31cd3fbda0855e6070182d248e3d52495) )
	ROM_CONTINUE( 0x8000, 0x8000)   // Discarding 1nd half. 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm74s472n.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0xb000, "plds", 0 )
	ROM_LOAD( "ep910.bin",           0x0000, 0x0884, CRC(aa62207e) SHA1(a2e51cf3617d07ff1ce42ee1d56e1c17b6fe71de) )
	ROM_LOAD( "pal16v8h-25pc-4.bin", 0x0900, 0x0117, CRC(df5be97d) SHA1(8cd63d49ab7020cdec07c1198413dc7d93e90d35) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolycdih_nvram.bin", 0x0000, 0x0800, CRC(02b3b335) SHA1(207a1289e4298d942e4806adf8ffb87e0b9b4e58) )
ROM_END

/**********************************************************************

  ZABAVNI KARTI (ЗАБАВНИ КАРТИ)
  Bulgarian Jolly Card clone with cyrillic charset.

  ЗАБАВНИ КАРТИ means "Fun cards" / "Cards for fun".

  Encrypted GFX. Same style as blue TAB board.

  This is a timed game. You coin for an amount of playing time.
  Once the time counter ends, all earned credits/bonus dissapear.

  It looks like an amusement version.

  To see more technical notes and setup instructions, see above
  the section 'NOTES BY GAME/SET'.

**********************************************************************/

ROM_START( jolycdcy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "256.bin", 0x8000, 0x8000, CRC(b7600b68) SHA1(04a532d3d1daa0ab43a2b7345ea0e86b8701f649) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(df434aa7) SHA1(e154868304f37a982d030de0989716ab9e27298d) )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(140546c5) SHA1(4dce119cd1a149eda6f546c0656de8fb8baef83b) )

	ROM_REGION( 0x0800, "nvram", 0 )    // Default NVRAM
	ROM_LOAD( "jolycdcy_nvram.bin", 0x0000, 0x0800, CRC(4c96ea57) SHA1(f7115866e9df8d031f23971b900119902406c020) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "n82s147a.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "peel18cv8.bin", 0x0000, 0x0155, CRC(fc51ef45) SHA1(a06916ccb65d7d3b9fd03f5de042c848d13478bb) )
ROM_END


ROM_START( sjcd2kx3 )   // Super Joly 2000 3x
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sj3.bin", 0x8000, 0x8000, CRC(c530b518) SHA1(36934d8e1e2cb2f71eb44a05b86ec970c9f398cd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sj2.bin", 0x0000, 0x8000, CRC(d7253325) SHA1(ad40e662519da9b11f77690b7d41c5007f74e280) )
	ROM_LOAD( "sj1.bin", 0x8000, 0x8000, CRC(82741749) SHA1(d0bf3073dff9ba1c4913fd754f965951e9cb5b03) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "sjcd2kx3_nvram.bin", 0x0000, 0x0800, CRC(1141368b) SHA1(b4af2d59b5e8115440e1219a621cfd2fb8c2c978) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END


ROM_START( jolycdab )
	ROM_REGION( 0x10000, "maincpu", 0 )  // program is testing/writing ROM offsets $8800-$BFFF (chksum?).
	ROM_LOAD( "ig1poker.run", 0x8000, 0x8000, CRC(c96e6542) SHA1(ed6c0cf9fe8597dba9149b2225320d8d9c39219a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jn1poker.ch2", 0x0000, 0x8000, CRC(8d78e43d) SHA1(15c60f8e0cd88518b0dc72b92aff6d8d4b2149cf) )
	ROM_LOAD( "jn1poker.ch1", 0x8000, 0x8000, CRC(d0a87f58) SHA1(6b7925557c4e40a1ebe52ecd14391cdd5e00b59a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM.
	ROM_LOAD( "jolycdab_nvram.bin", 0x0000, 0x0800, CRC(30fe661b) SHA1(323c9b5e4856601dbd40f8e48aa8cd9a112e08a9) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

/*
  Jolly Game (V1, Otti Karl logo)
  Bootleg. 1993.

  Color info in 2nd half of the Bipolar PROM.

*/
ROM_START( jolygame )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jg_99.1.bin", 0x8000, 0x8000, CRC(4154478f) SHA1(050340d75aa9a4b93025eef3507a8cfe220c466b) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.bin", 0x0000, 0x8000, CRC(db147426) SHA1(f97f3fed13e2723a3b32eb146cd7c16b54088d61) )
	ROM_LOAD( "2.bin", 0x8000, 0x8000, CRC(e7e7a0fd) SHA1(232190a0b2086e12ea3d77652ef98c9e107b3dc7) )

	ROM_REGION( 0x0800, "ds1220", 0 )  // to analyze...
	ROM_LOAD( "ds1220.bin", 0x0000, 0x0800, CRC(4364b423) SHA1(01c8d5e3003f4a9cc8ec446ddc18ff83353688df) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM.
	ROM_LOAD( "jolygame_nvram.bin", 0x0000, 0x0800, CRC(51213ede) SHA1(4da080e3b5ea8090609571883bbb04ea22774aa5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29apc.bin", 0x0000, 0x0200, CRC(8992aa4d) SHA1(5a0649bff66e7cab1bcbadcdfc74c77a747cc58f) )
ROM_END


/******************************** Bonus Card based sets ************************************/

/*
  Bonus Card
  Fun World, 1986.

  Hardware Fun World/Impera/TAB...
  Seems close to Big Deal and Royal Vegas Joker Card.

  PCB Layout...
  +----------------------------------------------------------------------------------------------------------------+
  |                                                                                                                |
  |   +--------+                    +--------+            +----------+                   +--------+                |
  |   |        |   +------------+   |        |            |  LM380N  |                   |        |                |
  |   |        |   |  GD74LS157 |   |        |            +----------+                   |        |   +---+        +---+
  |   |        |   +------------+   |HYUNDAI |                                           |AY38910A|   |   | +---+    --|
  |   |        |                    |        |                                           |/P      |   |ULN| |   |    --|
  |   |        |   +------------+   |HY6116AL|                                           |        |   |200| |74L|    --|
  |   |        |   |  GD74LS157 |   |P-10    |                                           |        |   |3AN| |S04|    --|
  |   |MC68B45P|   +------------+   |        |                                           |9027CCA |   |   | |N  |    --|
  |   |        |                    |        |                                           |        |   |   | |   |    --|
  |   |        |   +------------+   |        |          +----------+                     |        |   +---+ |   |    --|
  |   |        |   |  GD74LS157 |   |        |          |XRU74HC126|                     | TAIWAN |         +---+    --|
  |   |  2JR5  |   +------------+   |        |          +----------+                     |        |                  --|
  |   |        |                    +--------+                                           |        |    +---+         --|
  |   |LLAG9319|   +------------+                                 +------------+         |        |    |   |         --|
  |   |        |   |  GD74LS157 |                                 | HD74LS245P |         |        |    |ULN|       +---+
  |   |        |   +------------+                                 +------------+         |        |    |200|       |
  |   |        |                                                                         |        |    |3AN|       |
  |   |        |                            +----+                                       |        |    |   |       |
  |   |        |     +------------------+   |XTAL|                                       |        |    |   |       |
  |   +--------+     |    KM6264BL-10   |   |    | +----------+                          +--------+    +---+       |
  |                  |                  |   |16.0| | 74LS139AN|       +--------+         +--------+                |
  | +-------------+  |    251Y KOREA    |   |0Mhz| +----------+       |        |         |        |                |
  | |  GD74LS245  |  |                  |   +----+                    |        |         |        |                |
  | +-------------+  +------------------+                             |        |         |        |                +---+
  |                                         +---+  +------------+     |R65C02P2|         |MC6821P |                  --|
  | +-------------+        +------------+   |   |  |TIBPAL16L8-2|     |        |         |        |                  --|
  | | HD74LS374P  |        | HD74LS374P |   |   |  +------------+     |11450-12|         |  0K2P  |                  --|
  | +-------------+        +------------+   |   |                     |        |         |        |                  --|
  |                                         |GD7|                     | MEXICO |         |        |                  --|
  |+--------------------+    +----------+   |4LS|                     |        |         |        |    +---+         --|
  ||Bonus Card 1        |    | 74LS194AN|   |368|                     |        |         |LLAI9320|    |  8|         --|
  ||                    |    +----------+   +---+                     |9314    |         |        |    |   |         --|
  ||              27C256|                                             |A27110-4|         |        |    |   |         --|
  ||                    |    +----------+   +---+                     |        |         |        |    |DIP|         --|
  |+--------------------+    | 74LS194AN|   |   |                     |        |         |        |    |   |         --|
  |                          +----------+   |74L|                     |        |         |        |    |   |         --|
  |                                         |S39|                     |        |         |        |    |   |         --|
  |+--------------------+    +----------+   |3N |                     |        |         |        |    |  1|         --|
  ||Bonus Card 2        |    | 74LS194AN|   |   |                     |        |         |        |    +---+         --|
  ||                    |    +----------+   |   |                     +--------+         +--------+                  --|
  ||              27C256|                   +---+                                        +--------+                  --|
  ||                    |    +----------+                   +--------------------+       |        |                  --|
  |+--------------------+    | 74LS194AN|                   |Bonus Card 3        |       |        |                  --|
  |                          +----------+                   |                    |       |        |                  --|
  |                                                         |              27C256|       |MC6821P |                  --|
  |+----------+  +----------+                               |                    |       |        |                  --|
  || GD74LS174|  |HD74LS02P |                               +--------------------+       |  0K2P  |                  --|
  |+----------+  +----------+                                                            |        |                  --|
  |                                                                                      |        |                  --|
  |+---+  +---+  +---+            +---+                                           +---+  |LLAI9320|                  --|
  ||GD7|  |   |  |HD7|            |   |                                           |   |  |        |                  --|
  ||4LS|  |74S|  |4LS|            |74L|                   JOLLY-2-T               |74L|  |        |                  --|
  ||174|  |472|  |374|            |S00|                                           |S02|  |        |                  --|
  ||   |  |N  |  |P  |            |N  |                                           |N  |  |        |                  --|
  ||   |  |   |  |   |            |   |             +----+                        |   |  |        |                  --|
  ||   |  |   |  |   |            |   |             | A00|                        |   |  |        |                  --|
  |+---+  |   |  |   |            |   |             +----+                        |   |  |        |                +---+
  |       |   |  |   |            +---+                                           +---+  |        |                |
  |       +---+  +---+                                                                   +--------+                |
  |                                                                                                                |
  +----------------------------------------------------------------------------------------------------------------+

  A00 = TL7705ACP


  DIP
  +---------------+
  |#|#| | | |#| | |
  |---------------|
  | | |#|#|#| |#|#|
  +---------------+
   1 2 3 4 5 6 7 8

*/

ROM_START( bonuscrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bonucard.cpu", 0x8000, 0x8000, CRC(da342100) SHA1(451fa6074aad19e9efd148c3d18115a20a3d344a) )    // original program

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "bonuscard_2.bin", 0x0000, 0x8000, CRC(b026823b) SHA1(8d0c80019a9b35104a3782c4fad5c2ca07440a37) )
	ROM_LOAD( "bonuscard_1.bin", 0x8000, 0x8000, CRC(e07f72de) SHA1(f4bd6bc7a8aabe76d09d48362e32f29932fff4e4) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "n82s147an.bin",  0x0000, 0x0200, BAD_DUMP CRC(136245f3) SHA1(715309982fcafbce88b08237ca46acec31273938) ) // from power card, original fun world encrypted bonus card clone.
	ROM_LOAD( "74s472n.bin",    0x0200, 0x0200, CRC(e56780cb) SHA1(c06b854f21b1dcee465ac9c8c9a2934b7e99565f) )          // original dump, but doesn't match the cards colors in real board.

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "bonuscrd_tibpal16l8.bin",  0x0000, 0x0104, CRC(9af1ac12) SHA1(2b9770eeca081b8c744ba1250bb99569816d7a85) )
ROM_END


ROM_START( bonuscrda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bonuscard_3.bin", 0x8000, 0x8000, CRC(c4c6f7af) SHA1(3d0c5c867a9473043fb0b2cde6c6b98c4580ad81) ) // identical to parent, but with 'ATG Electronic' string instead.

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "bonuscard_2.bin", 0x0000, 0x8000, CRC(b026823b) SHA1(8d0c80019a9b35104a3782c4fad5c2ca07440a37) )
	ROM_LOAD( "bonuscard_1.bin", 0x8000, 0x8000, CRC(e07f72de) SHA1(f4bd6bc7a8aabe76d09d48362e32f29932fff4e4) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "n82s147an.bin",  0x0000, 0x0200, BAD_DUMP CRC(136245f3) SHA1(715309982fcafbce88b08237ca46acec31273938) ) // from power card, original fun world encrypted bonus card clone.
	ROM_LOAD( "74s472n.bin",    0x0200, 0x0200, CRC(e56780cb) SHA1(c06b854f21b1dcee465ac9c8c9a2934b7e99565f) )          // original dump, but doesn't match the cards colors in real board.

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "bonuscrd_tibpal16l8.bin",  0x0000, 0x0104, CRC(9af1ac12) SHA1(2b9770eeca081b8c744ba1250bb99569816d7a85) )
ROM_END


/*
  Club Card (ver 1.1, English)
  Impera / Mega Tech Salzburg

  VRAM at 4000/5000
*/
ROM_START( clubcard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu_256.bin", 0x8000, 0x8000, CRC(4b028c4d) SHA1(28ba4abc36d6adcb4299bb9a35f5f0d7572db656) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "c1.bin", 0x0000, 0x8000, CRC(09012966) SHA1(5b70c760bc06c054943f18057375d2743758416f) )
	ROM_LOAD( "c2.bin", 0x8000, 0x8000, CRC(ed9a680a) SHA1(d63c49e9689f3d49818cf207761eb3430254e2cb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized NVRAM
	ROM_LOAD( "clubcard_nvram.bin", 0x0000, 0x0800, CRC(974cb47b) SHA1(da9218210847ce73887355afa56d0ccfd45e5d94) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "bprom.bin",    0x0000, 0x0200, BAD_DUMP CRC(e92f74e0) SHA1(dfc4a9d140d21b990f769c10802c4d2c33dd4132) )  // borrowed from gratispk
ROM_END


/*
    Big Deal (hungarian)
    ------------------

    - 1x MC6845P
    - 1x YM2149F
    - 2x MC6821P
    - 1x Crystal 16.000 MHz
*/

ROM_START( bigdeal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker4.001", 0x8000, 0x8000, CRC(bb0198c1) SHA1(6e7d42beb5723a4368ae3788f83b448f86e5653d) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "poker4.003", 0x0000, 0x8000, CRC(8c33a15f) SHA1(a1c8451c99a23eeffaedb21d1a1b69f54629f8ab) )
	ROM_LOAD( "poker4.002", 0x8000, 0x8000, CRC(5f4e12d8) SHA1(014b2364879faaf4922cdb82ee07692389f20c2d) )

	ROM_REGION( 0x0200, "proms", 0 )    // using joker card palette till a correct dump appear
	ROM_LOAD( "jokercrd_prom.bin", 0x0000, 0x0200, BAD_DUMP CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END


ROM_START( bigdealb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker8.003", 0x8000, 0x8000, CRC(09c93745) SHA1(a64e96ef3489bc37c2c642f49e62cfef371de6f1) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "poker4.003", 0x0000, 0x8000, CRC(8c33a15f) SHA1(a1c8451c99a23eeffaedb21d1a1b69f54629f8ab) )
	ROM_LOAD( "poker4.002", 0x8000, 0x8000, CRC(5f4e12d8) SHA1(014b2364879faaf4922cdb82ee07692389f20c2d) )

	ROM_REGION( 0x0200, "proms", 0 )    // using joker card palette till a correct dump appear
	ROM_LOAD( "jokercrd_prom.bin", 0x0000, 0x0200, BAD_DUMP CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END


/******************************** C.M.C. sets ************************************/

/*
    Cuore Uno (italian)
    -----------------------------------

    - CPU 1x G65SC02P
    - 1x MC68B45P (CRT controller)
    - 2x MC68B21CP (Peripheral Interface Adapter)
    - 1x unknown (95101) DIP40 mil600
    - 1x oscillator 16.000000 MHz
    - ROMs  1x TMS27C512
            2x TMS27C256
    - 1x PROM AM27S29
    - 2x PALCE20V8H
    - 1x PALCE16V8H (soldered)
    - Note 1x JAMMA edge connector (keep -5 disconnected)
    - 1x trimmer (volume)
    - 1x 8 DIP switches
    - 1x battery
*/

ROM_START( cuoreuno )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuore1a.u2", 0x8000, 0x8000, CRC(6e112184) SHA1(283ac534fc1cb33d11bbdf3447630333f2fc957f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cuore1b.u21", 0x0000, 0x8000, CRC(14eca2b8) SHA1(35cba415800c6cd3e6ed9946057f33510ad2bfc9) )
	ROM_LOAD( "cuore1c.u22", 0x8000, 0x8000, CRC(253fac84) SHA1(1ad104ab8e8d73df6397a840a4b26565b245d7a3) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "cuoreuno_nvram.bin", 0x0000, 0x0800, CRC(b5a1bf25) SHA1(c2996a28c080debf10ab7a7dc47c305aed172a83) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_cu.bin",    0x0000, 0x0200, CRC(7ea61749) SHA1(3167acd79f9bda2078c2af3e049ad6abf160aeae) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_cu.u5",  0x0000, 0x0117, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_cu.u22", 0x0200, 0x0157, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_cu.u23", 0x0400, 0x0157, NO_DUMP )    // PLD is read protected
ROM_END

ROM_START( cuoreunoa ) // same as pool10b but with cuoreuno GFX ROMs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmc.u2", 0x8000, 0x8000, CRC(64fee38e) SHA1(8a624a0b6eb4a3ba09e5b396dc5a01994dfdf294) )
	ROM_IGNORE(                 0x8000 )    // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2mb.u21", 0x0000, 0x8000, CRC(2a3581c8) SHA1(8fa81c55fb2eb1d51ce5995f38ec4518d035c3af) )
	ROM_LOAD( "2mc.u20", 0x8000, 0x8000, CRC(26f1162a) SHA1(c6d98e38377016f2b3daa138c3331a1aa91e422a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "cuoreunoa_nvram.bin",   0x0000, 0x0800, CRC(d9f35299) SHA1(2c3608bc9c322a9cc86f74d8fa2f660804a8cf3c) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25",   0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h.u5",  0x0000, 0x0117, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "palce20v8h.u22", 0x0200, 0x0157, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "palce20v8h.u23", 0x0400, 0x0157, NO_DUMP )  // PLD is read protected
ROM_END

ROM_START( cuoreunob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pool10tk.u2", 0x8000, 0x8000, CRC(837559be) SHA1(12b72c06cdf563c721544e6d96cac8fe3408ac50) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "b.u21", 0x0000, 0x8000, CRC(4b7d90b2) SHA1(1c3b022ebb71ed23bfd5da511525b1850f791941) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                0x8000 )
	ROM_LOAD( "c.u20", 0x8000, 0x8000, CRC(e9d107ce) SHA1(3b05cc6f52ac59b432ad1353e9a4a6534e40d2f1) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                0x8000 )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "cuoreunob_nvram.bin",   0x0000, 0x0800, CRC(0f1ca7ac) SHA1(ea7c353ff8ffde632cbd900eb61a4cb982a4539e) )

	ROM_REGION( 0x0200, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "am27s29pc.u25",   0x0000, 0x0200, BAD_DUMP CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h.u5",  0x0000, 0x0117, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "gal20v8qs.u22",  0x0200, 0x0157, NO_DUMP )  // PLD is read protected
	ROM_LOAD( "gal20v8qs.u23",  0x0400, 0x0157, NO_DUMP )  // PLD is read protected
ROM_END

ROM_START( cuoreunoc ) // same code as pool10g, same GFX as cuoreunoa, found on 3 PCBs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuore_a.u2.1", 0x8000, 0x8000, CRC(7b537ce6) SHA1(b221d08c53b9e14178335632420e78070b9cfb27) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cuore_b.u21.1", 0x0000, 0x8000, CRC(2a3581c8) SHA1(8fa81c55fb2eb1d51ce5995f38ec4518d035c3af) )
	ROM_LOAD( "cuore_c.u20.1", 0x8000, 0x8000, CRC(26f1162a) SHA1(c6d98e38377016f2b3daa138c3331a1aa91e422a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "cuoreunoc_nvram.bin",   0x0000, 0x0800, CRC(3ec39472) SHA1(aa2bb5abd16557560a19842929ad7dab852abbbf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25.1",   0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 ) // all read protected
	ROM_LOAD( "palce16v8h.u5", 0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "gal20v8a.u22",  0x0200, 0x0157, NO_DUMP )
	ROM_LOAD( "gal20v8a.u23",  0x0400, 0x0157, NO_DUMP )
ROM_END

ROM_START( cuoreunod ) // same code as pool10h, same GFX as cuoreunoa
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuore_a.u2", 0x8000, 0x8000, CRC(b671c6dd) SHA1(070617bbe304469deb98504e3ee73800bff225bb) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cuore_b.u21", 0x0000, 0x8000, CRC(2a3581c8) SHA1(8fa81c55fb2eb1d51ce5995f38ec4518d035c3af) )
	ROM_LOAD( "cuore_c.u20", 0x8000, 0x8000, CRC(26f1162a) SHA1(c6d98e38377016f2b3daa138c3331a1aa91e422a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "cuoreunod_nvram.bin",   0x0000, 0x0800, CRC(e93dee30) SHA1(195525e95a3bdc1b002b12fd27bc31c63d7a9276) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25",   0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h-25pc-4.u5", 0x0000, 0x0117, CRC(c4569166) SHA1(f725dc654fc87b51968fc9f3178e4b22bfc04a6e) )
	ROM_LOAD( "palce20v8h.u22",       0x0200, 0x0157, NO_DUMP ) // read protected
	ROM_LOAD( "palce20v8h.u23",       0x0400, 0x0157, NO_DUMP ) // read protected
ROM_END

ROM_START( cuoreunoe ) // same code as pool10f, same GFX as cuoreunoa
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.u2", 0x8000, 0x8000, CRC(e8087fb8) SHA1(c012a81f561978bd97708a52f656e7b13e41a3e2) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "b.u21", 0x0000, 0x8000, CRC(2a3581c8) SHA1(8fa81c55fb2eb1d51ce5995f38ec4518d035c3af) )
	ROM_LOAD( "c.u20", 0x8000, 0x8000, CRC(26f1162a) SHA1(c6d98e38377016f2b3daa138c3331a1aa91e422a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "cuoreunoe_nvram.bin", 0x0000, 0x0800, CRC(75dd3562) SHA1(a359cada144e7c90946649f5dd0998d0ee48f4d2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25",   0x0000, 0x0200, BAD_DUMP CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) ) // not dumped for this set

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h-25pc-4.u5", 0x0000, 0x0117, NO_DUMP ) // read protected
	ROM_LOAD( "palce20v8h.u22",       0x0200, 0x0157, NO_DUMP ) // read protected
	ROM_LOAD( "palce20v8h.u23",       0x0400, 0x0157, NO_DUMP ) // read protected
ROM_END

ROM_START( cuoreunof ) // same as cuoreunoc but with copyright removed
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.u2", 0x8000, 0x8000, CRC(ef66d40a) SHA1(ed406472a5f3d883036f0001082607f1561e6820) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cuore_b.u21", 0x0000, 0x8000, CRC(2a3581c8) SHA1(8fa81c55fb2eb1d51ce5995f38ec4518d035c3af) )
	ROM_LOAD( "cuore_c.u20", 0x8000, 0x8000, CRC(26f1162a) SHA1(c6d98e38377016f2b3daa138c3331a1aa91e422a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "cuoreunof_nvram.bin", 0x0000, 0x0800, CRC(3ec39472) SHA1(aa2bb5abd16557560a19842929ad7dab852abbbf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an.u25",   0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 ) // all read protected
	ROM_LOAD( "palce16v8h-25pc-4.u5",  0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "palce20v8h-25pc-4.u22", 0x0200, 0x0157, NO_DUMP )
	ROM_LOAD( "palce20v8h-25pc-4.u23", 0x0400, 0x0157, NO_DUMP )
ROM_END

/*
    Elephant Family (italian, old)
    -----------------------------

    - CPU 1x R65C02P2
    - 1x MC68B45P (CRT controller)
    - 2x EF6821P (Peripheral Interface Adapter)
    - 1x unknown (95101) DIP40 mil600
    - 1x oscillator 16.000 MHz
    - ROMs  2x M27C256
            1x TMS27C256
    - 1x PROM AM27S29
    - 2x PALCE20V8H (read protected)
    - 1x PALCE16V8H (read protected)
    - Note 1x JAMMA edge connector (keep -5 disconnected)
    - 1x trimmer (volume)
    - 1x 8 DIP switches
    - 1x battery

    u2.bin    1ST AND 2ND HALF IDENTICAL
    u20.bin   1ST AND 2ND HALF IDENTICAL
    u21.bin   1ST AND 2ND HALF IDENTICAL
*/

ROM_START( elephfam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eleph_a.u2",  0x8000, 0x8000, CRC(8392b842) SHA1(74c850c734ca8174167b2f826b9b1ac902669392) )
	ROM_IGNORE(                      0x8000 )   // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "eleph_b.u21", 0x0000, 0x8000, CRC(e3612670) SHA1(beb65f7d2bd6d7bc68cfd876af51910cf6417bd0) )
	ROM_IGNORE(                      0x8000 )   // Identical halves. Discarding 2nd half
	ROM_LOAD( "eleph_c.u22", 0x8000, 0x8000, CRC(4b909bf3) SHA1(a822b12126bc58af6d3f999ab2117370015a039b) )
	ROM_IGNORE(                      0x8000 )   // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "elephfam_nvram.bin", 0x0000, 0x0800, CRC(fb9b1100) SHA1(cf15ce55042f1c4399fec480c2f862622905a8b5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_ef.u25",    0x0000, 0x0200, CRC(bca8b82a) SHA1(4aa19f5ecd9953bf8792dceb075a746f77c01cfc) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_ef.u5",  0x0000, 0x0117, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_ef.u22", 0x0200, 0x0157, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_ef.u23", 0x0400, 0x0157, NO_DUMP )    // PLD is read protected
ROM_END


ROM_START( elephfmb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "now.u2", 0x8000, 0x8000, CRC(7b537ce6) SHA1(b221d08c53b9e14178335632420e78070b9cfb27) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "elephb.u21", 0x0000, 0x8000, CRC(3c60549c) SHA1(c839b3ea415a877e5eac04e0522c342cce8d6e64) )
	ROM_LOAD( "elephc.u20", 0x8000, 0x8000, CRC(448ba955) SHA1(2785cbc8cd42a7dda85bd8b81d5fbec01a1ba0bd) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "elephfmb_nvram.bin", 0x0000, 0x0800, CRC(13a0cfea) SHA1(9c8ce509ef1076e88ea853347b64c5591bc2e18c) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_ef.u25",    0x0000, 0x0200, CRC(bca8b82a) SHA1(4aa19f5ecd9953bf8792dceb075a746f77c01cfc) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_ef.u5",  0x0000, 0x0117, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_ef.u22", 0x0200, 0x0157, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_ef.u23", 0x0400, 0x0157, NO_DUMP )    // PLD is read protected
ROM_END


/*
    Pool 10 (italian)
    -----------------

    - 1x R65C02P2 (main)
    - 1x YM2149F (sound)
    - 1x HD46505 (CRT controller)
    - 2x EF6821P (Peripheral Interface Adapter)
    - 1x oscillator 16.000000 MHz

    - 2x M27256 (pool,1)
    - 1x D27256 (2)
    - 1x PROM N82S147AN
    - 2x GAL20V8B (read protected)
    - 1x PALCE16V8H (read protected)

    - 1x JAMMA edge connector
    - 1x trimmer (volume)
    - 1x 8 DIP switches
    - 1x battery


    Connector, DIPs and instructions,
    copied from an original sheet...

              Components |  | solder
    ---------------------|--|-----------------
                     gnd |01| gnd
                     gnd |02| gnd
                      +5 |03| +5
                      +5 |04| +5
                         |06|
                     +12 |07| +5
                         |08| contatore out
            motor hopper |09| contatore in
                   audio |10| audio
               sw hopper |11| gnd
                   rosso |12| verde
                     blu |13| sync
                     gnd |14| statistic
              manegement |15|
                  coin 1 |16| coin 2
               sw ticket |17| motor ticket
                  hold 3 |18| lamp hold 3
                  hold 4 |19| lamp hold 4
                  hold 2 |20| lamp hold 2
                  hold 1 |21| lamp hold 1
                  hold 5 |22| lamp hold 5
                   start |23| lamp hold start
                  cancel |24| lamp hold cancel
                      nc |25| +5 lamp
    ric. ticket + hopper |26| +12 lamp
                     gnd |27| gnd
                     gnd |28| gnd


    DIPS
                          1    2   3   4   5   6   7   8
                         on
                         off
    ticket + hopper           on  on
    hopper                    on  off
    ticket                    off on
    no ticket - no hopper     off off
    1 coin 1 credit                   off off
    1 coin 5 credits                  off on
    1 coin 10 credits                 on  off
    1 coin 50 credits                 on  on
    five of kind yes                          off
    five of kind no                           on
    royal flush yes                               off
    royal flush no                                on
    in test                                           on
    in game                                           off


    Instructions:

    STATISTICS = show in and out credits
    MANEGEMENT = It's the general statistic. It shows in and out credits.

    To cancel statistics press CANCEL for 5 seconds.
    To change max and min value for play connect to GND the manegement pin,
    at the same time press together HOLD 2 + HOLD 4 for some seconds.

    The following menu will show up:

    HOLD 1 = modify max value for play 1 to 10.
    HOLD 2 = modify min value foe play 1 to 5.
    Press start to exit from programming mode.

    To unlock PCB from a possible lock:
    connect GND to pins component15 and solder14 for 5 seconds

    NB. Use lamps by 12V 0,15W
    Memory version: V16
*/

ROM_START( pool10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pool10.u2", 0x8000, 0x8000, CRC(4e928756) SHA1(d9ac3d41ea32e060a7e269502b7f22333c5e6c61) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10_nvram.bin",  0x0000, 0x0800, CRC(2f2fab43) SHA1(f815b70c171bad99fa6a60c256e4fdc85dd6b290) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) // PLD is read protected
ROM_END


/*
  - pool10b -

  u2.bin    1ST AND 2ND HALF IDENTICAL
  u20.bin   1ST AND 2ND HALF IDENTICAL
  u21.bin   1ST AND 2ND HALF IDENTICAL
*/
ROM_START( pool10b ) // 1st set nominated for parent
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u2.bin", 0x8000, 0x8000, CRC(64fee38e) SHA1(8a624a0b6eb4a3ba09e5b396dc5a01994dfdf294) )
	ROM_IGNORE(                 0x8000 ) // Identical halves. Discarding 2nd half

//  GFX ROMs are the same of pool10, but double sized with identical halves.
	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u21.bin", 0x0000, 0x8000, CRC(581c4878) SHA1(5ae61af090feea1745e22f46b33b2c01e6013fbe) )
	ROM_IGNORE(                  0x8000 )   // Identical halves. Discarding 2nd half
	ROM_LOAD( "u20.bin", 0x8000, 0x8000, CRC(3bdf1106) SHA1(fa21cbd49bb27ea4a784cf4e4b3fbd52650a285b) )
	ROM_IGNORE(                  0x8000 )   // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10b_nvram.bin",   0x0000, 0x0800, CRC(d9f35299) SHA1(2c3608bc9c322a9cc86f74d8fa2f660804a8cf3c) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25",   0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10b.u5",  0x0000, 0x0117, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "palce20v8h_p10b.u22", 0x0200, 0x0157, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "palce20v8h_p10b.u23", 0x0400, 0x0157, NO_DUMP ) // PLD is read protected
ROM_END


ROM_START( pool10c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.u2", 0x8000, 0x8000, CRC(ac157b17) SHA1(f2b7eb940273bc404d3e0d8dd0f00ca757cebf69) ) // sldh

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "b.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "c.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10c_nvram.bin", 0x0000, 0x0800, CRC(396aefed) SHA1(066b87ff054dfb37f733a812ad0dc1b1bd2478e6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) // PLD is read protected
ROM_END


/*
    - pool10d -

    3.50.u2   1ST AND 2ND HALF IDENTICAL
*/
ROM_START( pool10d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.50.u2", 0x8000, 0x8000, CRC(4c68e1f4) SHA1(bbab63a18e0c041ce519daa32e12dd1b6a672dce) )
	ROM_IGNORE(                  0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) ) // sldh
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) ) // sldh

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10d_nvram.bin", 0x0000, 0x0800, CRC(6b5984a0) SHA1(156a94e74e33b1a15222cffff9b62e65f6f5f2f5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) // PLD is read protected
ROM_END


/*
  Pool 10...
  Dino 4 (not working) board,
  with the infamous mexican Rockwell R65C02.

  Encrypted program & graphics ROMs.

  PCB layout...
  .----------------------------------------------------------------------------------------------.
  |                                                                                              |
  |                      .----.            .----------.  .--------------.     .--------------.   |
  |                      |A00 |            |M74HC00B1 |  | SN74HCT245N  |     |PALCE16V8H-15 |   |
  |                      '----'            '----------'  '--------------'     '--------------'   |
  |                                                                                              |
  |               .------------.      .-----------.                                              |
  |               |            |      | DIP SW #1 |      .----------------------------.          |
  |               |  BATTERY   |      |           |      |          R65C02P2          |          |
  '---.           |            |      '-----------'      |          11450-12          |          |
      |           |            |                         |           MEXICO           |          |
      |           '------------'                         |        9740 S11493_2       |          |
      |                                                  '----------------------------'          |
  .---'                  .----------------------------.                                          |
  |---                   |                            |                                          |
  |---                   |          HD46821P          |                                          |
  |---                   |                            |  .--------------------.    .---------.   |
  |---                   |                            |  |     3_50.U2        |    |74HC126B1|   |
  |---                   '----------------------------'  |                    |    '---------'   |
  |---                                                   |                    |                  |
  |---                   .----------------------------.  |               27256|                  |
  |---                   |           MC6821P          |  '--------------------'                  |
  |---                   |         QL M9N8623         |                            .---------.   |
  |---  .---------.      |                            |                            |74HC139E |   |
  |---J |ULN2003A |      |                            |                            '---------'   |
  |---  '---------'      '----------------------------'  .-----------------.                     |
  |---A                                                  |                 |                     |
  |---                                                   |    JAPAN 2G3    |       .---------.   |
  |---M .---------.       .-----------.  .-----------.   |    HM6116LP-4   |       |SN74LS02N|   |
  |---  |74LS04B1 |       | 411GR-001 |  | 411GR-001 |   |                 |       '---------'   |
  |---M '---------'       '-----------'  '-----------'   '-----------------'                     |
  |---                                                                                           |
  |---A                                                                                          |
  |---                   .----------------------------.  .----------------------------.          |
  |---                   |           FILE             |  |          MC6845P           |          |
  |---                   |          KV89C72           |  |          R1A 8210          |          |
  |---                   |                            |  |                            |          |
  |---                   |                            |  |                            |          |
  |---  .-------------.  '----------------------------'  '----------------------------'          |
  |---  |  74HCT373N  |                                                                          |
  |---  '-------------'     .---------. .---------------. .-------------.         .---------.    |
  |---                      |ULN2003A | |PALCE20V8H-25PC| | SN74LS245N  |         |74157 PC |    |
  |---  .-------------.     '---------' '---------------' '-------------'         '---------'    |
  |---  |   AM27S29   |                                                                          |
  |---  '-------------'  .--------------------.                                   .---------.    |
  |---                   |      2.U21         |           .--------------------.  |74157 PC |    |
  |---  .--------.       |                    |           |      GOLDSTAR      |  '---------'    |
  |--   |74LS174B| .---. |                    |           |     GM76C88-12     |                 |
  '---. '--------' |74L| |               27256|           |     8928 KOREA     |  .---------.    |
      |            |S08| '--------------------' LC DINO 4 |                    |  |74157 PC |    |
      |            |B1 |                                  '--------------------'  '---------'    |
      | .--------. |   | .--------------------.                                                  |
  .---' |74LS02N | |   | |      1.U20         |                                                  |
  |     '--------' '---' |                    |           .-------------.         .---------.    |
  |                      |                    |           | SN74LS377N  |         |74157 PC |    |
  |                      |               27256|           '-------------'         '---------'    |
  |                      '--------------------'                                                  |
  |     .--------.                                                                               |
  |     |  XTAL  |   .---------. .---------------.        .-------------.         .---------.    |
  |     | 16 MHz |   |74LS161AN| |PALCE20V8H-25PC|        | SN74LS377N  |         |74LS174B1|    |
  |     '--------'   '---------' '---------------'        '-------------'         '---------'    |
  |                                                                                              |
  '----------------------------------------------------------------------------------------------'

  A00 = TL7705ACE

*/

ROM_START( pool10e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3_50.u2", 0x8000, 0x8000, CRC(764394bb) SHA1(0defcedc802c468c615560e47ec4064a4f084650) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(a0d54044) SHA1(c7be1f12f72095daee32ae41c3554d8ab4f99245) ) // sldh
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(55c9fcc8) SHA1(224bdf63ed345b1def4852af3b33f07790fbf123) ) // sldh

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10e_nvram.bin", 0x0000, 0x0800, CRC(e20f9a14) SHA1(617ca53263a971c9f835a95737a66fac5b99780f) )

	ROM_REGION( 0x0200, "proms", 0 ) // Same as Pool 10, but the 1st half duplicated to cover any PLD addressing
	ROM_LOAD( "am27s29.u25", 0x0000, 0x0200, CRC(2c315cbf) SHA1(f3f91329f2b8388decf26a050f8fb7da38694218) )

	ROM_REGION( 0x0600, "plds", 0 ) // all read protected
	ROM_LOAD( "palce16v8h.u5",  0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "palce20v8h.u22", 0x0200, 0x0157, NO_DUMP )
	ROM_LOAD( "palce20v8h.u23", 0x0400, 0x0157, NO_DUMP )
ROM_END


ROM_START( pool10f )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmc-pool10+a+.u2", 0x8000, 0x8000, CRC(e8087fb8) SHA1(c012a81f561978bd97708a52f656e7b13e41a3e2) ) // sldh

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cmc-pool10-b.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "cmc-pool10-c.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10f_nvram.bin", 0x0000, 0x0800, CRC(75dd3562) SHA1(a359cada144e7c90946649f5dd0998d0ee48f4d2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "27s29.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) // PLD is read protected
ROM_END


ROM_START( pool10g ) // 2nd set nominated for parent
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.u2", 0x8000, 0x8000, CRC(7b537ce6) SHA1(b221d08c53b9e14178335632420e78070b9cfb27) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10h_nvram.bin", 0x0000, 0x0800, CRC(3ec39472) SHA1(aa2bb5abd16557560a19842929ad7dab852abbbf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u25",   0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )
ROM_END


ROM_START( pool10h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmc-pool10+a+.u2", 0x8000, 0x8000, CRC(b671c6dd) SHA1(070617bbe304469deb98504e3ee73800bff225bb) ) // sldh

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cmc-pool10+b+.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "cmc-pool10+c+.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10i_nvram.bin", 0x0000, 0x0800, CRC(e93dee30) SHA1(195525e95a3bdc1b002b12fd27bc31c63d7a9276) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )
ROM_END


ROM_START( pool10i )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.u2", 0x8000, 0x8000, CRC(566bc05d) SHA1(eec88c8ba6cb664f38ebf8b71f99b4e7d04a9601) ) // sldh
	ROM_IGNORE(               0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "b.u21", 0x0000, 0x8000, CRC(581c4878) SHA1(5ae61af090feea1745e22f46b33b2c01e6013fbe) ) // sldh
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half
	ROM_LOAD( "c.u20", 0x8000, 0x8000, CRC(3bdf1106) SHA1(fa21cbd49bb27ea4a784cf4e4b3fbd52650a285b) ) // sldh
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10l_nvram.bin", 0x0000, 0x0800, CRC(89cbee4b) SHA1(ff8031a96ee40e1e62abbae7a0b3d9dc2122759f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )
ROM_END

ROM_START( pool10j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmcpool10.u2", 0x8000, 0x8000, CRC(f7fa2b12) SHA1(743563e51327b00dd638bcb2233e833ce74e62a0) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cmcpool10.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "cmcpool10.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10j_nvram.bin",  0x0000, 0x0800, CRC(48684b02) SHA1(6f2fbd0e2621e31b881edd8056ff93ee78f331ab) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 ) // all read protected
	ROM_LOAD( "palce16v8h.u5",  0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "palce20v8h.u22", 0x0200, 0x0157, NO_DUMP )
	ROM_LOAD( "palce20v8h.u23", 0x0400, 0x0157, NO_DUMP )
ROM_END

ROM_START( pool10k ) // found on two PCBs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eagle1.u2", 0x8000, 0x8000, CRC(1fdddc4e) SHA1(5c36374b7e36b615f051e20b684c5e058e82efce) ) // 0xxxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(          0x8000, 0x2000 )
	ROM_CONTINUE(          0xc000, 0x2000 )
	ROM_CONTINUE(          0xa000, 0x2000 )
	ROM_CONTINUE(          0xe000, 0x2000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "eagle2.u21", 0x0000, 0x8000, CRC(581c4878) SHA1(5ae61af090feea1745e22f46b33b2c01e6013fbe) )
	ROM_IGNORE(                     0x8000 ) // Identical halves. Discarding 2nd half
	ROM_LOAD( "eagle3.u20", 0x8000, 0x8000, CRC(3bdf1106) SHA1(fa21cbd49bb27ea4a784cf4e4b3fbd52650a285b) )
	ROM_IGNORE(                     0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "pool10k_nvram.bin",  0x0000, 0x0800, CRC(fb0e9e6a) SHA1(97fbcff4d615983321bf5d53884f7ed56f8a8998) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 ) // all read protected
	ROM_LOAD( "gal16v8ah.u5",   0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "palce20v8h.u22", 0x0200, 0x0157, NO_DUMP )
	ROM_LOAD( "palce20v8h.u23", 0x0400, 0x0157, NO_DUMP )
ROM_END

ROM_START( mag10 ) // same code as pool10g but for title and copyright changes, u20 GFX ROM contains changes
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "magic_a.u2", 0x8000, 0x8000, CRC(c2379eed) SHA1(91ca7514d6c4a8032ab6a5221b7f09c02f7d65af) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "magic_b.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "magic_c.u20", 0x8000, 0x8000, CRC(b863dead) SHA1(8f0016c39ce64a1ecde66f61b7f0db9cdfb36873) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "mag10_nvram.bin", 0x0000, 0x0800, CRC(3ec39472) SHA1(aa2bb5abd16557560a19842929ad7dab852abbbf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 ) // all read protected
	ROM_LOAD( "palce16v8h-25pc-4.u5", 0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "20s25hb1.u22",         0x0200, 0x0157, NO_DUMP )
	ROM_LOAD( "20s25hb1.u23",         0x0400, 0x0157, NO_DUMP )
ROM_END

/*
  New Biliard 98

  CPUs:
  1x    R65C02P2    u1  8-bit Microprocessor - main
  1x    MC68B45P    u8  CRT Controller (CRTC)
  2x    MC68B21CP   u29, u30    Peripheral Interface Adapter
  1x    89C10       u34     Programmable Sound Generator - sound
  1x    TDA2003     u33     Audio Amplifier - sound
  1x    oscillator  16.000000MHz    os1

  ROMs:
  3x    27C256  u2, u20, u21    dumped
  1x    AM27S29PC   u25     dumped

  RAMs:
  1x    KM6264BL-10L-12 u13
  1x    GM76C28A-10     u3

  PLDs:
  2x    GAL20V8A-25LP   u22, u23  read protected
  1x    PALCE16V8H-15PC/4   u5    read protected

  Others:
  1x 28x2 JAMMA edge connector
  1x trimmer (TR1, volume)
  1x 8 DIP switches bank (SW1)
  1x battery 3.6V

  Notes:
  PCB is marked: "top" and "C.M.C. POOL 10" on component side
  PCB is marked: "bottom" and "POOL 10 C.M.C" on solder side

*/
ROM_START( biliard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "biliard-a.u2", 0x8000, 0x8000, CRC(3b20348b) SHA1(002b9b8581adc1b253ee3babf0e4c1aad4a2f017) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cmcpool10-b.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "biliard-c.u20",   0x8000, 0x8000, CRC(b15d10ec) SHA1(9b0f32ff791063cfb2d8339a4e8041e034e73eb7) )

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "biliard_nvram.bin", 0x0000, 0x0800, CRC(2f2fab43) SHA1(f815b70c171bad99fa6a60c256e4fdc85dd6b290) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h-25pc-4.u5", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "gal20v8a-25lp.u22",  0x0200, 0x0157, NO_DUMP )   // PLD is read protected
	ROM_LOAD( "gal20v8a-25lp.u23",  0x0400, 0x0157, NO_DUMP )   // PLD is read protected
ROM_END


/*
  Royal...
  This one seems to run in royalcd1 hardware.
*/

ROM_START( royal ) // brute hack of pool 10
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.u2", 0x8000, 0x8000, CRC(d4f36273) SHA1(2049257ea9ee52fde9cabfe40e809e00526a960e) ) // sldh

//  GFX ROMs are the same of pool10, but double sized with identical halves.
	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(439eec10) SHA1(500139c16a883f0a5b0b8d91f4f067ba428d2d11) ) // sldh
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(9b59e72d) SHA1(96217272ce5abb78ff45ff116a5d921c57717ed9) ) // sldh
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 ) // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "royal_nvram.bin", 0x0000, 0x0800, CRC(9df190d5) SHA1(4be0f5c6f89f822568e45e0e8457cf51ced2dcfe) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u25", 0x0000, 0x0200, CRC(d922d4e5) SHA1(d1541eabfd9cedd9eaa4fc48a3f64b078ea456be) ) // sldh
ROM_END


/*
    Tortuga Family (italian) & Pot Game (italian)
    ---------------------------------------------

    - 1x G65SC02P2 (main)
    - 1x 95101 (sound)
    - 1x MC68B45P (CRT controller)
    - 2x MC68B21CP (Peripheral Interface Adapter)
    - 1x oscillator 16.000000 MHz

    - 3x TMS27C256
    - 1x PROM AM27S29PC
    - 2x PALCE20V8H (read protected)
    - 1x PALCE16V8H (read protected)

    - 1x JAMMA edge connector
    - 1x trimmer (volume)
    - 1x 8 DIP switches
    - 1x battery
*/

ROM_START( tortufam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tortu.a.u2", 0x8000, 0x8000, CRC(6e112184) SHA1(283ac534fc1cb33d11bbdf3447630333f2fc957f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "tortu.b.u21", 0x0000, 0x8000, CRC(e7b18584) SHA1(fa1c367469d4ced5d7c83c15a25ec5fd6afcca10) )
	ROM_LOAD( "tortu.c.u20", 0x8000, 0x8000, CRC(3cda6f73) SHA1(b4f3d2d3c652ebf6973358ae33b7808de5939acd) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "tortufam_nvram.bin", 0x0000, 0x0800, CRC(e5a08b1b) SHA1(6575ed3ec66ef0e42129225fe1679519e5e1c946) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc_tf.u25", 0x0000, 0x0200, CRC(c6d433fb) SHA1(065de832bbe8765eb0aacc2029e587a4f5362f8a) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce20v8h_tf.u5",  0x0000, 0x0157, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_tf.u22", 0x0200, 0x0157, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "palce20v8h_tf.u23", 0x0400, 0x0157, NO_DUMP )    // PLD is read protected
ROM_END


ROM_START( potgame )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "now.u2", 0x8000, 0x8000, CRC(7b537ce6) SHA1(b221d08c53b9e14178335632420e78070b9cfb27) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "potg.b.u21", 0x0000, 0x8000, CRC(32fc1d4f) SHA1(cc533a44498338bc0cbb7c7b9c42559ce7ff1337) )
	ROM_LOAD( "potg.c.u20", 0x8000, 0x8000, CRC(0331eb42) SHA1(a8e838d644fc6d93a9632070b305e44e4242ce94) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "potgame_nvram.bin",  0x0000, 0x0800, CRC(2b07fb37) SHA1(9cbd3d8fb076d683a7853b3dd8a39a27f1c8872b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_pot.u25",    0x0000, 0x0200, CRC(a221f151) SHA1(270c57c9b7de912b136686bc6720eb8f12dbb805) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_pot.u5",  0x0000, 0x0157, NO_DUMP )   // PLD is read protected
	ROM_LOAD( "palce20v8h_pot.u22", 0x0200, 0x0157, NO_DUMP )   // PLD is read protected
	ROM_LOAD( "palce20v8h_pot.u23", 0x0400, 0x0157, NO_DUMP )   // PLD is read protected
ROM_END


ROM_START( bottle10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "boat_3.bin", 0x8000, 0x8000, CRC(e2db8334) SHA1(22ac4ce361a93b7e6d491e260635755dd562b294) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "boat_2.bin", 0x0000, 0x8000, CRC(a6b36c3f) SHA1(90b12d9552ad5dbf11a30fc7451da1f3e6763cc3) )
	ROM_LOAD( "boat_1.bin", 0x8000, 0x8000, CRC(61fd8c19) SHA1(eb8fd8bd7de38a6c8a435e9e36daf699162138a5) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "bottle10_nvram.bin", 0x0000, 0x0800, CRC(82927c53) SHA1(8cde91588cb53fefc84f0b14fc5c0b26a3a445eb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP )    // PLD is read protected
ROM_END


ROM_START( bottl10b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.u2", 0x8000, 0x8000, CRC(e2db8334) SHA1(22ac4ce361a93b7e6d491e260635755dd562b294) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(9395c15b) SHA1(e4caefc6f55b07f5c4370a3b8652fa93e08987ce) )
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(61fd8c19) SHA1(eb8fd8bd7de38a6c8a435e9e36daf699162138a5) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "bottl10b_nvram.bin", 0x0000, 0x0800, CRC(59976182) SHA1(f8d26169e86444607bc5a6649f41e7f5c05ddbb4) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25", 0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP )    // PLD is read protected
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP )    // PLD is read protected
ROM_END

/*
  Luna Park sets...

  This board has mirrored video RAM 4000-4FFF to 5000-5FFF,
  and color RAM 6000-6FFF to 7000-7FFF.

  Two different programs. One in each program ROM half.
  PRG rom higher address line is connected to DIP switch #1,
  so it should have 2 games in the same PCB (2 revisions?)

         1st half:           2nd half:

  BE58:  LDA #$04            LDA #$00
         JSR $9DE2           JSR $9DE2

  BE60:  LDA #$09            LDA #$00
         JSR $B25E           JSR $B25E

  BE73:  A5 22               A5 23

  BEC0: '5 TIRI LIRE 500'   '10 TIRI LIRE 500'
  BF40:  00 00 00 00...     'wait...no coin'
  C3D0: 'ABILITA VINTE'     'PARTITA VINTE'
  DF30:  20 20 20 20 20      00 00 00 00 00

  The second set, has the same programs, but the strings on the first half
  were changed to match the second program.

*/
// The following two have mirrored video RAM 4000/5000 to 6000/7000.
ROM_START( lunapark )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Two different programs. Selectable through a DIP switch
	ROM_LOAD( "lunapark-425-95n003.u2", 0x0000, 0x10000, CRC(b3a620ee) SHA1(67b3498edf7b536e22c4d97c1f6ad5a71521e68f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "lunapark-425-95n002.u21", 0x0000, 0x8000, CRC(2bededb7) SHA1(b8d7e6fe307d347d762adf35d361ade620aab37b) )
	ROM_CONTINUE(                        0x0000, 0x8000)    // Discarding 1nd half 0xff filled
	ROM_LOAD( "lunapark-425-95n001.u20", 0x8000, 0x8000, CRC(7d91ce1f) SHA1(7e9bfad76f305d5787faffe3a07b218beb37fda8) )
	ROM_CONTINUE(                        0x8000, 0x8000)    // Discarding 1nd half 0xff filled

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "lunapark_nvram.bin", 0x0000, 0x0800, CRC(f99e749b) SHA1(fafd4205dfaacb4c21215af6997d06ab419c9281) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u25", 0x0000, 0x0200, CRC(ddb74d72) SHA1(3d5dda3a935a3100cb86017f103b855d6449f73a) )
ROM_END

ROM_START( lunaparkb )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Two different programs. Selectable through a DIP switch
	ROM_LOAD( "lunapark-number-03_lunaparkb.u2", 0x0000, 0x10000, CRC(cb819bb7) SHA1(c7fb25eab093de2f644445a713d99ee8024d8499) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "27512.u21", 0x0000, 0x8000, CRC(d64ac315) SHA1(c67d9e67a988036844efd4f980d47a90c022ba18) )   // only the first 2 bytes different
	ROM_CONTINUE(          0x0000, 0x8000)  // Discarding 1nd half 0xff filled
	ROM_LOAD( "27512.u20", 0x8000, 0x8000, CRC(7d91ce1f) SHA1(7e9bfad76f305d5787faffe3a07b218beb37fda8 ) )
	ROM_CONTINUE(          0x8000, 0x8000)  // Discarding 1nd half 0xff filled

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "lunaparkb_nvram.bin", 0x0000, 0x0800, CRC(f99e749b) SHA1(fafd4205dfaacb4c21215af6997d06ab419c9281) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u25", 0x0000, 0x0200, CRC(ddb74d72) SHA1(3d5dda3a935a3100cb86017f103b855d6449f73a) )
ROM_END

//  This one hasn't mirrored video RAM, so could run in regular Cuore 1 hardware
ROM_START( lunaparkc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lunapark-number-03_lunaparkc.u2", 0x8000, 0x8000, CRC(fdbe49c3) SHA1(a2b14a6998d5a27fba7bc360a15f17a48c91194f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "lunapark-number-01.u21", 0x0000, 0x8000, CRC(ee057944) SHA1(31b76dcadf1dd5aacac1dfed0c7c9f7190797ead) )
	ROM_LOAD( "lunapark-number-02.u20", 0x8000, 0x8000, CRC(b8795aec) SHA1(5db2e64657dee7742eb9d11e65d29c83a93332b7) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "lunaparkc_nvram.bin", 0x0000, 0x0800, CRC(005b70fc) SHA1(682c2315b4fafb6255db989f0d49255fd8d7a1a9) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u25", 0x0000, 0x0200, CRC(ddb74d72) SHA1(3d5dda3a935a3100cb86017f103b855d6449f73a) )
ROM_END

/*
  Crystal Colours...
  Version running in CMC hardware.
*/
ROM_START( crystal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "425-1995-number-03.u2", 0x8000, 0x8000, CRC(bab1ab88) SHA1(d224f2f7cd9313f51c484d781dc278e358f1e4fd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "425-1995-number-02.u21", 0x0000, 0x8000, CRC(1eaf1bd9) SHA1(eb392f4a8864c59c7792f905f165f543087cb4a0) )
	ROM_LOAD( "425-1995-number-01.u20", 0x8000, 0x8000, CRC(d3972c19) SHA1(a84ae765eeae1f9d443b0c4941b6f93dcc540f8c) )

	ROM_REGION( 0x0800, "nvram", 0 )    // pre-initialized BBRAM (hw uses SRAM + 3,6V battery)
	ROM_LOAD( "crystal_nvram.bin", 0x0000, 0x0800, CRC(21a712ee) SHA1(259d83b8268a93f96b53580562e9c6e835f7473e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u25", 0x0000, 0x0200, CRC(dd9db8bd) SHA1(6374795ae243759a887464b38ece078dc3735b6f) )
ROM_END


/******************************** Royal Card sets ************************************/

/*
    1.bin   NO MATCH
    2.bin   = 1.bin              royalcrdd   Royal Card (German, set 5)
            = u4_nmc27c256.bin   royalcrdp   Royal Card v2.0 Professional
    3.bin   NO MATCH

  This one works properly. Also Bet with Hold 5.

*/

ROM_START( royalcrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(7f920318) SHA1(acbfed8d0a0984fd0c572a4de42d8dc08fb99a82) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.bin", 0x0000, 0x8000, CRC(c46d804f) SHA1(b089821c7dae6714b49401d787f8bed859815763) )
	ROM_LOAD( "2.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrd_nvram.bin", 0x0000, 0x0800, CRC(1c775f61) SHA1(c810421eaa31a72e3f2fe9a1d82858e7cc2f6b93) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom2.bin",    0x0000, 0x0200, CRC(7e5839e6) SHA1(5e8401d2b236f73cc017a73a369b3b3b821a1deb) )
ROM_END

/*
    r.1   NO MATCH
    r.2   = 1a.bin   royalcrde   Royal Card (German, set 6)
    r.3   = 2a.bin   royalcrde   Royal Card (German, set 6)

    All ROMs are double sized, with identical halves.
    The set is working properly, with Bet (Hold 5) and normal payout. No Remote.
*/
ROM_START( royalcrda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r.1", 0x0000, 0x10000, CRC(761021ec) SHA1(3a00a1d2f74dbb06a0ab6605c32bef8013c93874) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "r.3", 0x0000, 0x8000, CRC(3af71cf8) SHA1(3a0ce0d0abebf386573c5936545dada1d3558e55) )
	ROM_IGNORE(              0x8000)
	ROM_LOAD( "r.2", 0x8000, 0x8000, CRC(8a66f22c) SHA1(67d6e8f8f5a0fd979dc498ba2cc67cf707ccdf95) )
	ROM_IGNORE(              0x8000)

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrda_nvram.bin", 0x0000, 0x0800, CRC(c42dbad4) SHA1(b38552192e3f5f6bc2a4b92bddd2f95ac17ecc8c) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom3_r.bin",    0x0000, 0x0200, CRC(7e5839e6) SHA1(5e8401d2b236f73cc017a73a369b3b3b821a1deb) )
ROM_END

/*
    rc_1_pl.bin   NO MATCH
    rc_2_pl.bin   = 1.bin              royalcrdd   Royal Card (German, set 5)
                  = u4_nmc27c256.bin   royalcrdp   Royal Card v2.0 Professional
    rc_3_pl.bin   = 2.bin              royalcrdd   Royal Card (German, set 5)

    Working properly, with Bet (Hold 5) and Hold 2 shown a credit screen that allow payout or clear.
*/
ROM_START( royalcrdb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_1_pl.bin", 0x8000, 0x8000, CRC(7a0ad63c) SHA1(7a1f6e0fd6e31f6950eeffc6a9ef073e909d85f9) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "rc_3_pl.bin", 0x0000, 0x8000, CRC(85e77661) SHA1(7d7a765c1bfcfeb9eb91d2519b22d734f20eab24) )
	ROM_LOAD( "rc_2_pl.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrdb_nvram.bin", 0x0000, 0x0800, CRC(19bb3dea) SHA1(0965fbcec48ded99c5f6793efffb1d9329cc00eb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom_epoxy.bin",    0x0000, 0x0200, CRC(b1e15441) SHA1(eb9318dd08e12281bef27a75bfdf54ec5d97fad6) )
ROM_END

/*
  Royal Card (German, set 4)

  roj.ic12   NO MATCH
  roj.ic25   NO MATCH
  roj.ic26   NO MATCH

  roj.ic26   BADADDR    xxxxxxx-xxxxxxxx
  Bad dump. Address 08 is faulty.

  GFX ROMS are double size.
  Second half of GFX ROMs seems to be from Jolly Card.

*/
ROM_START( royalcrdc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roj.ic12", 0x8000, 0x8000, CRC(16923d58) SHA1(e865b91246ae5a21bdc9787e6e6e22be5182cabb) )

	ROM_REGION( 0x10000, "gfx1", 0 )  // borrowed from parent set, that matches 100%.
	ROM_LOAD( "3.bin", 0x0000, 0x8000, CRC(c46d804f) SHA1(b089821c7dae6714b49401d787f8bed859815763) )
	ROM_LOAD( "2.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x10000, "gfxbad", 0 )  // faulty gfx set with bad addressing and mix of royal card & jolly card tiles.
	ROM_LOAD( "roj.ic26", 0x0000, 0x8000, BAD_DUMP CRC(3883cdcb) SHA1(b71a786822fe8fcb2c6fcdc463facb2738ec8c01) )
	ROM_IGNORE(                   0x8000)
	ROM_LOAD( "roj.ic25", 0x8000, 0x8000, CRC(c5b787e8) SHA1(be88aa901c1f96d171af45c3602e0ce72b8fff34) )
	ROM_IGNORE(                   0x8000)

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM
	ROM_LOAD( "royalcrdc_nvram.bin", 0x0000, 0x0800, CRC(eacb0c7b) SHA1(513816623aa3843dd5d0416fc012060c7a9f6c71) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom1.bin",    0x0000, 0x0200, CRC(8f6a33b1) SHA1(3a2f97563b7f094ef8b13a4e47a18e4a1fcf51c3) )
ROM_END

/*
    Royal Card (set 5)
    ------------------

    - 1x HM6264LP
    - 1x HD4650SP
    - 1x HM6116LP
    - 1x R65C02P2 (main)
    - 1x WB5300 (labeled YM8910)(sound)
    - 1x EF6821P
    - 1x oscillator 16.000 MHz

    - 1x D27256 (1)
    - 1x S27C256 (2)
    - 1x TMS27C256 (r2)
    - 2x PEEL18CV8 (1 protected)
    - 1x PALCE16V8H (protected)
    - 1x PROM N82S147AN

    - 1x 8 DIP Switches
    - 1x 22x2 edge connector
    - 1x 18x2 edge connector
    - 1x trimmer (volume)
*/

ROM_START( royalcrdd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r2.bin", 0x8000, 0x8000, CRC(25dfe0dc) SHA1(1a857a910d0c34b6b5bfc2b6ea2e08ed8ed0cae0) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(85e77661) SHA1(7d7a765c1bfcfeb9eb91d2519b22d734f20eab24) ) // sldh
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) ) // sldh

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrdd_nvram.bin", 0x0000, 0x0800, CRC(335bfa5a) SHA1(7e9cbb502f450c515ea03ffcf4b7fbae60af4e73) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h-4.bin", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "1-peel18cv8.bin",  0x0200, 0x0155, NO_DUMP ) // PLD is read protected
	ROM_LOAD( "2-peel18cv8.bin",  0x0400, 0x0155, CRC(8fdafd55) SHA1(fbb187ba682111648ea1586f400990cb81a3077a) )
ROM_END


/*
    Royal Card (set 6)
    ------------------

    - CPU 1x R65C02P2 (main)
    - 1x MC68B45P (CRT controller)
    - 2x MC68B21CP (Peripheral Interface Adapter)
    - 1x oscillator 16.000 MHz
    - 3x ROMs TMS 27C512
    - 1x PALCE16V8H
    - 1x prom AM27S29APC

    - 1x 28x2 connector (maybe NOT jamma)
    - 1x 10x2 connector
    - 1x 3 legs connector (solder side)
    - 1x 8 DIP Switches
    - 1x trimmer (volume)
*/

ROM_START( royalcrde )  // both halves have different programs. we're using the 2nd one.
	ROM_REGION( 0x10000, "maincpu", 0 ) // 1st half prg is testing RAM in offset $8600-$BF00...??
	ROM_LOAD( "rc.bin", 0x0000, 0x10000, CRC(8a9a6dd6) SHA1(04c3f9f17d5404ac1414c51ef8f930df54530e72) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2a.bin", 0x0000, 0x8000, CRC(3af71cf8) SHA1(3a0ce0d0abebf386573c5936545dada1d3558e55) )
	ROM_IGNORE(                 0x8000 )    // Identical halves. Discarding 2nd half
	ROM_LOAD( "1a.bin", 0x8000, 0x8000, CRC(8a66f22c) SHA1(67d6e8f8f5a0fd979dc498ba2cc67cf707ccdf95) )
	ROM_IGNORE(                 0x8000 )    // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrde_nvram.bin", 0x0000, 0x0800, CRC(3b03440f) SHA1(49e51b8c9e1404d39c479a2d0619aab43f1a6529) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce16v8h-4.bin", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
ROM_END


/*
   Royal Card (TAB Austria original)
   With respective original PLD properly dumped.
*/

ROM_START( royalcrdt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r1_0.bin", 0x8000, 0x8000, CRC(829a6a1d) SHA1(b7064e4d60e33d0875eb73525230ea3b99f10542) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "novr3_0.bin", 0x0000, 0x8000, CRC(85e77661) SHA1(7d7a765c1bfcfeb9eb91d2519b22d734f20eab24) )
	ROM_LOAD( "novr2_0.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "jop1.bin",    0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "royalcrdt_nv.bin",  0x0000, 0x0800, CRC(67a6e68b) SHA1(d7ab01c4d9bd4fe58b5d0f4a945c00c5c4906008) )

	ROM_REGION( 0x0200, "plds", 0 ) // Device type is 16L8
	ROM_LOAD( "tab01_3.bin",    0x0000, 0x0104, CRC(a13a7a0a) SHA1(28e918ece4dcfa3883d2439c226b2f125d43f386) )
ROM_END


/*
    Royal Card (set 7, encrypted)
    -----------------------------

    - Custom/encrypted CPU (epoxy block labelled "EVONA EX9511" -> www.evona.sk )
        inserted into socked with "6502" mark.

    - 1x YM2149

    - 1x HD6845 (CRT controller)
    - 1x MC68A21P (PIA)
    - 1x 40 pin IC with surface scratched (PIA)
    - 1x 8 DIP Switches
    - Sanyo LC3517B SRAM (videoram ?)
    - 6264 battery backed SRAM (battery is dead)
    - 1x PALCE16V8
    - 1x GAL16V8B
    - 1x PEEL18CV8P x2
    - 1x 82S147 PROM (near Yamaha and unknown 40pin) - "82s147.bin"
    - 1x 27256 close to CPU module - "1.bin"
    - 2x 27256 - gfx - "2.bin", "3.bin"
*/

ROM_START( royalcrdf )  // encrypted program rom
	ROM_REGION( 0x10000*2, "maincpu", 0 ) // *2 for decrypted opcodes (see init)
	ROM_LOAD( "rc_1.bin", 0x8000, 0x8000, CRC(8cdcc978) SHA1(489b58760a7c8646399c8cdfb86ec4341823e7dd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "rc_2.bin", 0x0000, 0x8000, CRC(7f934488) SHA1(c537a09ef7e88a81ee9c2e1d971b3caf9d3dba0e) )
	ROM_LOAD( "rc_3.bin", 0x8000, 0x8000, CRC(8612c6ed) SHA1(3306a252af479e0510f136020086015b60dce879) )

	ROM_REGION( 0x0800, "nvram", 0 )  // default NVRAM
	ROM_LOAD( "royalcrdf_nvram.bin", 0x0000, 0x0800, CRC(05c14b70) SHA1(03ed97a67c7cd004f2c0abbb88fa3ec4a43c3f05) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(44dbf086) SHA1(43a2d615c00605db75a4fd4d57d9e056c0356f10) ) // sldh

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8.bin",    0x0000, 0x0117, NO_DUMP )  // not present in the set
	ROM_LOAD( "1-peel18cv8p.bin", 0x0200, 0x0155, NO_DUMP )  // not present in the set
	ROM_LOAD( "2-peel18cv8p.bin", 0x0400, 0x0155, NO_DUMP )  // not present in the set
ROM_END


ROM_START( royalcrdg )  // CMC C1030 PCB, EP910EC-30 CPLD, NON encrypted graphics
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.cpu", 0x8000, 0x8000, CRC(829a6a1d) SHA1(b7064e4d60e33d0875eb73525230ea3b99f10542) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(85e77661) SHA1(7d7a765c1bfcfeb9eb91d2519b22d734f20eab24) ) // sldh
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) ) // sldh

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrdg_nvram.bin", 0x0000, 0x0800, CRC(853c7da9) SHA1(e275b22a9f470672bfc71425fcc44f547ba38b6d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(a762ee84) SHA1(40883a7c0f4365e2725eb0b29d45467e1b7a1a31) ) // sldh
ROM_END


/*
  Royal Card (German, set 8)
  Year: 1991.
  Manufacturer: TAB Austria.

*/
ROM_START( royalcrdh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "145-3.bin", 0x8000, 0x8000, CRC(c4da3e95) SHA1(fb7c3652f783a3040e6e08616df102efcb0b4134) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "145-2.bin", 0x0000, 0x8000, CRC(85e77661) SHA1(7d7a765c1bfcfeb9eb91d2519b22d734f20eab24) )
	ROM_LOAD( "145-1.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrdh_nvram.bin", 0x0000, 0x0800, CRC(7757d73a) SHA1(997bfbd847d1d21777694331be92a1a5e79e4faf) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.bin", 0x0000, 0x0200, CRC(7e5839e6) SHA1(5e8401d2b236f73cc017a73a369b3b3b821a1deb) )  // borrowed from parent set.
ROM_END


/*
  Royal Card (French)
  Year:         1991
  Manufacturer: TAB Austria

  1x 6502A     (missing)
  2x EF6821P   Peripheral Interface Adapter
  1x 68A45     CRT Controller (CRTC)
  1x YM2149F   Programmable Sound Generator
  1x oscillator 16000.000KHz

  3x 27256     1, 2, 3 (dumped)
  1x N82S147AN         (dumped)

  1x 6264
  1x 6116

  2x PEEL18CV8P-25 (read protected) -> Extracted with CmD's PALdumper
  1x GAL16V8-25 (read protected) -> Extracted with CmD's PALdumper

  1x 28x2 JAMMA edge connector
  1x trimmer (volume)
  1x 8 DIP switches
  1x battery (missing)

*/
ROM_START( royalcdfr )  // Seems bootleg PCB, non encrypted graphics
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin", 0x8000, 0x8000, CRC(69b944c1) SHA1(6ef76bff01f20376117dd7f67e5890eca754fcfb) ) // sldh

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(85e77661) SHA1(7d7a765c1bfcfeb9eb91d2519b22d734f20eab24) ) // sldh
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) ) // sldh

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcdfr_nvram.bin", 0x0000, 0x0800, CRC(bda344d4) SHA1(7793d289147bf03c0d8256d4023252c9677ac8ff) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an.bin", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END


ROM_START( royalcrdp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u16_m27c256b.bin", 0x8000, 0x8000, CRC(162996ff) SHA1(122c13ee9842e692d31490f216eb972df2321b7f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u11_tms27c256.bin",0x0000, 0x8000, CRC(d6834c3a) SHA1(4b071b9826c086439b9763393b23c671261b3788) )
	ROM_LOAD( "u4_nmc27c256.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147a.bin",     0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "royalcrdp_nvram.bin", 0x0000, 0x0800, BAD_DUMP CRC(553f8c66) SHA1(d2c21786d715f81c537d860d8515fda6d766f630) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce16v8h_1.bin", 0x0000, 0x0117, NO_DUMP )
ROM_END


/*
  Royal Card / Royal Jackpot (with third draw) (encrypted)

  Encrypted graphics 'alla TAB blue PCB.
  Maybe is the infamous encrypted Impera green PCB.

  From the manual: "With third draw"
  ...but I couldn't trigger this feature.

 */

ROM_START( royaljp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.z.1_10.bin", 0x8000, 0x8000, CRC(ad03c41c) SHA1(8241a7eb02d7c06b7ea5bea5ab0d9ed4c6fcfd50) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "02.bin", 0x0000, 0x8000, CRC(c8f6f3e4) SHA1(5d8b32609562647c698926ea75c8f806b703332e) )
	ROM_LOAD( "01.bin", 0x8000, 0x8000, CRC(3ced6423) SHA1(2309a856226404789fd219cf77ba2f378a90eee7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "rj_d27hc65d-2,82s147.bin",  0x0000, 0x0200, CRC(d6570420) SHA1(90b6c126b485db823acffbfd195964a6282e60ea) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "royaljp_nvram.bin",  0x0000, 0x0800, CRC(48e51e2a) SHA1(b81a1741eef299384d4fa98a534099b2c0332074) )

	ROM_REGION( 0x0600, "plds", 0 ) // There are 3 GALs
	ROM_LOAD( "gal20v8b.bin",    0x0000, 0x0157,   NO_DUMP )
	ROM_LOAD( "gal20v8a.bin",    0x0200, 0x0157,   NO_DUMP )
	ROM_LOAD( "gal16v8d.bin",    0x0400, 0x0117,   NO_DUMP )
ROM_END


/******************************** Lucky Lady sets ************************************/

ROM_START( lluck3x3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "l3x3.bin", 0x8000, 0x8000, CRC(dbdb07ff) SHA1(6be43aa0b2c10d83373f20f477606cb031bc6dd9) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "l2.bin", 0x0000, 0x8000, CRC(8ca90a8f) SHA1(bc3db3f8c097f89eff488e3aca39bf24ff2b5cff) )
	ROM_LOAD( "l1.bin", 0x8000, 0x8000, CRC(beadc35c) SHA1(8a6a5954a827def8c4c3b904d8ee58a4bde53d85) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "lluck3x3_nvram.bin", 0x0000, 0x0800, CRC(2fe79cff) SHA1(7839c04336b7702c7bdcd2b6917a353f4376f824) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END


ROM_START( lluck4x1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rk4x1.bin", 0x8000, 0x8000, CRC(37f8a355) SHA1(a6eb4c53464e373bdecbbaaf146f5f7cf66b4bcd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "l2.bin", 0x0000, 0x8000, CRC(8ca90a8f) SHA1(bc3db3f8c097f89eff488e3aca39bf24ff2b5cff) )
	ROM_LOAD( "l1.bin", 0x8000, 0x8000, CRC(beadc35c) SHA1(8a6a5954a827def8c4c3b904d8ee58a4bde53d85) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "lluck4x1_nvram.bin", 0x0000, 0x0800, CRC(05d2d7b8) SHA1(1188b2b4835cadd95b1e9160c2948a2e3457afd2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END


/******************************** Magic Card II sets ************************************/

/*
    Magic Card II (Impera)
    ----------------------

    - 1x Special CPU with CM602 (??) on it  <--- from dumper notes.
    - 1x MC6845P
    - 1x YM2149F
    - 2x MC6821P
    - 1x Crystal 16.000 MHz
    - 2x HY18CV8S (electrically-erasable PLD)

    Some versions have Mexican Rockwell R65C02.
    The game doesn't work with a regular 65C02 CPU.

    There are different programs that carry the same
    graphics set for green TAB / Impera boards.

    TAB blue boards can run the same programs, but needs
    the encrypted graphics set.

    All these games have some weird things...
    1) Some CPU instructions seems wrong (see below, in driver init)
    2) The CRTC is injected with some wrong register values (fact),
       that place the game wrongly, and screw up the input test screen.

*/

ROM_START( magicrd2 )   // Impera... but seems Bulgarian hack, just for copyright
	ROM_REGION( 0x10000, "maincpu", 0 ) // magicard.004 has extra code, and 2 different NVRAM contents harcoded
	ROM_LOAD( "magicard.004", 0x0000, 0x8000,  CRC(f6e948b8) SHA1(7d5983015a508ab135ccbf69b7f3c526c229e3ef) )
	ROM_LOAD( "magicard.01",  0x8000, 0x8000,  CRC(c94767d4) SHA1(171ac946bdf2575f9e4a31e534a8e641597af519) )   // 1st and 2nd half identical
	ROM_IGNORE(                       0x8000 )  // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mc2gr2.bin",   0x0000, 0x8000, CRC(733da697) SHA1(45122c64d5a371ec91cecc67b7faf179078e714d) )
	ROM_LOAD( "mc2gr1.bin",   0x8000, 0x8000, CRC(2406b8d2) SHA1(e9d7a25562fd5adee351d7ef6ba02fff6aab021a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "magicrd2_nvram.bin", 0x0000, 0x0800, CRC(343b3162) SHA1(1524959dbbc0c6d7c5c3a4a6b20976382cfbe88f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mc2-82s147.bin", 0x0000, 0x0200, CRC(aa91cc35) SHA1(79f9a755441500e618c4183f524f969fffd44100) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )  // Universal GAL
ROM_END


ROM_START( magicrd2a )  // Nov (new). Imatic Yugoslavian hack for green TAB or Impera boards
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m3_nov.bin", 0x8000, 0x8000,  CRC(ee5468e6) SHA1(f859adbad30e561fca86e60ff5b2e666d8bf4071) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "m2_nov.bin",   0x0000, 0x8000, CRC(684d71f2) SHA1(e4522844a0406b3e83fa723508a7c05dd21e7fb6) )
	ROM_LOAD( "m1_nov.bin",   0x8000, 0x8000, CRC(96151034) SHA1(3107d353705c6240a71faf308e11c45a87d95cf4) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM (passed protection)
	ROM_LOAD( "mc2_nvram.bin", 0x0000, 0x0800, CRC(2070d63d) SHA1(86c72a2e81651b0138d8551a0cfcd07176f8e7d2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mc2_82s147.bin", 0x0000, 0x0200, CRC(aa91cc35) SHA1(79f9a755441500e618c4183f524f969fffd44100) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END


ROM_START( magicrd2b )  // Imatic Yugoslavian hack for green TAB or Impera boards
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc2prgv1.bin", 0x8000, 0x8000,  CRC(7f759b70) SHA1(23a1a6e8eda57c4a90c51a970302f9a7bf590083) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mc2gr2.bin",   0x0000, 0x8000, CRC(733da697) SHA1(45122c64d5a371ec91cecc67b7faf179078e714d) )
	ROM_LOAD( "mc2gr1.bin",   0x8000, 0x8000, CRC(2406b8d2) SHA1(e9d7a25562fd5adee351d7ef6ba02fff6aab021a) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM (passed protection)
	ROM_LOAD( "mc2_v1-v2_nvram.bin", 0x0000, 0x0800, BAD_DUMP CRC(f88c493d) SHA1(8a5352b46ab68164cd7adaaad6f15f04327b7451) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mc2-82s147.bin", 0x0000, 0x0200, CRC(aa91cc35) SHA1(79f9a755441500e618c4183f524f969fffd44100) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END


ROM_START( magicrd2c )  // Imatic Yugoslavian hack for blue TAB board (encrypted)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc2prgv2.bin", 0x8000, 0x8000,  CRC(b0ed6b40) SHA1(7167e67608f1b0b1cd956c838dacc1310861cb4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mc2gr1b.bin",  0x0000, 0x8000, CRC(ce2629a7) SHA1(84767ed5da8dcee44a210255537e10372bcc264b) )
	ROM_LOAD( "mc2gr2b.bin",  0x8000, 0x8000, CRC(d2bf8bde) SHA1(975b8f43a0396c09e357b96d5ae7381b12152b9e) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM (passed protection)
	ROM_LOAD( "mc2_v1-v2_nvram.bin", 0x0000, 0x0800, BAD_DUMP CRC(f88c493d) SHA1(8a5352b46ab68164cd7adaaad6f15f04327b7451) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mc2-82s147.bin", 0x0000, 0x0200, CRC(aa91cc35) SHA1(79f9a755441500e618c4183f524f969fffd44100) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END



/******************************** Royal Vegas Joker Card sets ************************************/

ROM_START( vegasslw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vslow.bin", 0x8000, 0x8000, CRC(9cb7861a) SHA1(f934eacd5b3573a6bbeaa827f521b4a498e5bcdf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "v2.bin", 0x0000, 0x8000, CRC(af7ab460) SHA1(01ea400424152c09c10eb83a1bd569019969ccb7) )
	ROM_LOAD( "v1.bin", 0x8000, 0x8000, CRC(23e0d1c6) SHA1(98967b14d3264c444a1dfbd15c57cde70f41f09d) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "vegasslw_nvram.bin", 0x0000, 0x0800, CRC(1aa043e3) SHA1(c93d071effb2f2fe95e9dc751174c2c765595f74) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "jokercrd_prom.bin", 0x0000, 0x0200, CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END


ROM_START( vegasfst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vfast.bin", 0x8000, 0x8000, CRC(87dfb28d) SHA1(9a06e695e59722b6c97e5a9fd2c8b238661e5a4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "v2.bin", 0x0000, 0x8000, CRC(af7ab460) SHA1(01ea400424152c09c10eb83a1bd569019969ccb7) )
	ROM_LOAD( "v1.bin", 0x8000, 0x8000, CRC(23e0d1c6) SHA1(98967b14d3264c444a1dfbd15c57cde70f41f09d) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "vegasfst_nvram.bin", 0x0000, 0x0800, CRC(5034de7a) SHA1(ab2077a49d94676531c73ad8d8ce9548bbfa2b81) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "jokercrd_prom.bin", 0x0000, 0x0200, CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END


ROM_START( vegasfte )   // Royal Vegas Joker Card (fast deal, english gfx)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ch3.bin", 0x8000, 0x8000, CRC(87dfb28d) SHA1(9a06e695e59722b6c97e5a9fd2c8b238661e5a4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ch2.bin", 0x0000, 0x8000, CRC(af7ab460) SHA1(01ea400424152c09c10eb83a1bd569019969ccb7) )
	ROM_LOAD( "ch1.bin", 0x8000, 0x8000, CRC(0a3679c0) SHA1(ce8a067e1a2eccf9fabb16733ef3a14e0e8129e5) ) // X & Y in txt layer

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "vegasfte_nvram.bin", 0x0000, 0x0800, CRC(166c6055) SHA1(db2143a2a3adc92578bd3707391d2f5030cc6a6f) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "jokercrd_prom.bin", 0x0000, 0x0200, CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END


ROM_START( vegasmil )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Mile
	ROM_LOAD( "mile.bin", 0x8000, 0x8000, CRC(ef7e02e2) SHA1(7432b0e723dc528901c422ab1d7d01fd1bc1eb20) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "v2.bin", 0x0000, 0x8000, CRC(af7ab460) SHA1(01ea400424152c09c10eb83a1bd569019969ccb7) )
	ROM_LOAD( "v1.bin", 0x8000, 0x8000, CRC(23e0d1c6) SHA1(98967b14d3264c444a1dfbd15c57cde70f41f09d) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "vegasmil_nvram.bin", 0x0000, 0x0800, CRC(d2608e5f) SHA1(ac936df71dbc0bfb811a3ba3c91444a2a3e7b036) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "jokercrd_prom.bin", 0x0000, 0x0200, CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END


/******************************** Jolly Joker sets ************************************/

/*

  Impera - Jolly Joker

  PCB Layout:
   _____________________________________________________________________________________________
  |                                                   __________                                |
  |                                                  | MOP1603  |                               |
  |                               _________    __    |__________|                               |
  |    ________________________  |    A    |  |  |       _______      __    __                  |___
  |   |                        | |_________|  |  |      |       |    |  |  |  |                  ___|
  |   |                        |              |A |      |       |    |D |  |K |                  ___|
  |   |        MC6845P         |  _________   |  |      |       |    |  |  |  |                  ___|
  |   |                        | |    A    |  |  |      |AY38910|    |  |  |  |                  ___|
  |   |________________________| |_________|  |__|      |  A/P  |    |__|  |__|                  ___|
  |                                            __       |9120CCA|                                ___|
  |                     _________________     |  |      |       |    _______                     ___|
  |    _____________   |                 |    |  |      |       |   |   K   |                    ___|
  |   | T74LS245B1  |  |  KM6264BL_10    |    |A |      |       |   |_______|                    ___|
  |   |_____________|  |                 |    |  |      |       |                                ___|
  |                    |  206Y KOREA     |    |  |      |       |         _______       ___      ___|
  |                    |_________________|    |__|      |       |        |       |     |   |8   |
  |                                                     |       |        |       |     |DIP|    |
  |    _____________        _____________               |_______|        |       |     | 1 |    |
  |   | SN74LS374N  |      | SN74LS374N  |                               |MC68B21|     |   |    |___
  |   |_____________|      |_____________|   ________________________    |      P|     |   |     ___|
  |                                         |        R65C02P2        |   |       |     |___|1    ___|
  |    __________________       _________   |        11450_12        |   |0K2P   |               ___|
  |   |CH1               |     |74LS194AN|  |          9209          |   |LLEG912|    __         ___|
  |   |                  |     |_________|  |________________________|   |      6|   |  |        ___|
  |   |   M27C512        |                                               |       |   |  |        ___|
  |   |                  |                   ___    __________________   |       |   |J |        ___|
  |   |__________________|      _________   |   |  |JOKER RUN         |  |       |   |  |        ___|
  |                            |74LS194AN|  |   |  |                  |  |       |   |  |        ___|
  |    __________________      |_________|  |   |  |   M27C512        |  |       |   |__|        ___|
  |   |CH2               |                  | B |  |                  |  |_______|               ___|
  |   |                  |      _________   |   |  |__________________|                          ___|
  |   |   M27C512        |     |74LS194AN|  |   |                         _______     __         ___|
  |   |                  |     |_________|  |   |                        |       |   |  |        ___|
  |   |__________________|                  |   |      _______________   |       |   |  |        ___|
  |                                         |___|     |               |  |       |   |J |        ___|
  |    _________    ________    _________             |    HYUNDAI    |  |MC68B21|   |  |        ___|
  |   |T54LS174M|  |DL002D  |  |74LS194AN|   _______  | HY6116ALP_10  |  |      P|   |  |        ___|
  |   |_________|  |________|  |_________|  |   C   | |               |  |       |   |__|        ___|
  |                                         |_______| |_______________|  |0K2P   |               ___|
  |    __     __    __          _________                                |LLEG912|    __         ___|
  |   |  |   |  |  |  |        |XTAL     |   _______                     |      6|   |  |        ___|
  |   |  |   |  |  |  |        |         |  |   D   |                    |       |   |  |        ___|
  |   |I |   |H |  |G |        |  16 Mhz |  |_______|                    |       |   |J |        ___|
  |   |  |   |  |  |  |        |_________|                               |       |   |  |        ___|
  |   |  |   |  |  |  |         _________      ____                      |       |   |  |        ___|
  |   |__|   |  |  |  |        |    E    |    | F  |                     |       |   |__|        ___|
  |          |__|  |__|        |_________|    |____|                     |_______|               ___|
  |                                                                                             |
  |_____________________________________________________________________________________________|


  A = PC74HCT157P
  B = AMPAL16L8DC
  C = HD74LS189P
  D = T54LS14M2
  E = T74LS393B1
  F = TL7705ACP
  G = SN74LS374N
  H = AM27S29PC
  I = PC74HCT174P
  J = 4116R_001
  K = ULN2003A


  DIP1:
   ___________________
  | ON                |
  |  _______________  |
  | |#|#|#|#|#|#|#|#| |
  | | | | | | | | | | |
  | |_______________| |
  |  1 2 3 4 5 6 7 8  |
  |___________________|

*/

ROM_START( jolyjokr )   // Jolly Joker (98bet, set 1)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impera3orig.bin", 0x8000, 0x8000, CRC(ceb3a0d5) SHA1(25efae9f225abddfa663e6abcc91801996e5b0ea) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "impera2.bin", 0x0000, 0x8000, CRC(f0fa5941) SHA1(1fcade31ed6893ffcfd4efe97dfaaa31d24283ec) )
	ROM_LOAD( "impera1.bin", 0x8000, 0x8000, CRC(c3ab44dd) SHA1(e46c0fd94da561f57033647f1703fa135777ece5) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokr_nvram.bin", 0x0000, 0x0800, CRC(f33e66ed) SHA1(7a4b9a1b2f976d5d26f54915a213d5ac5eca0a42) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29.bin", 0x0000, 0x0200, CRC(0b671bba) SHA1(92d512e02b50f98b7bc5a60deee4fee722656c4f) )
ROM_END


ROM_START( jolyjokra )  // Jolly Joker (98bet, set 2)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impera50.bin", 0x8000, 0x8000, CRC(7effc044) SHA1(961438e7fb8222296fb959b510cdf646e4ac3226) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "impera2.bin", 0x0000, 0x8000, CRC(f0fa5941) SHA1(1fcade31ed6893ffcfd4efe97dfaaa31d24283ec) )
	ROM_LOAD( "impera1.bin", 0x8000, 0x8000, CRC(c3ab44dd) SHA1(e46c0fd94da561f57033647f1703fa135777ece5) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokra_nvram.bin", 0x0000, 0x0800, CRC(ed43693c) SHA1(d4aa4e539ab12c97bc9b9b1077997195a11d782b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29.bin", 0x0000, 0x0200, CRC(0b671bba) SHA1(92d512e02b50f98b7bc5a60deee4fee722656c4f) )
ROM_END


/*
  Jolly Joker (40bet)

  am27s29_ic40.bin     1ST AND 2ND HALF IDENTICAL
  ic25.bin             1ST AND 2ND HALF IDENTICAL
  ic26.bin             1ST AND 2ND HALF IDENTICAL

  These graphics ROMs have enough data to fix the bitrotten graphics from the other Jolly Joker sets.
  impera1.bin ---> bits 7 & 6
  impera2.bin ---> bits 7 & 0

  Two slightly different programs. One in each half.
  Both have Min Bet (5), Max Bet (40).
  Surely selectable through a PLD.

  This is clearly the paytable. Here the differences...

  offset:     C9CF C9D0 C9D1 C9D2 C9D3 C9D4 C9D5 C9D6 C9D7 C9D8 C9D9 C9DA C9DB C9DC C9DD C9DE C9DF C9E0 C9E1 C9E2
  program #1  0x01 0x00 0x02 0x00 0x03 0x00 0x05 0x00 0x07 0x00 0x10 0x00 0x40 0x00 0x00 0x01 0x00 0x05 0x00 0x11
  program #2  0x01 0x00 0x02 0x00 0x03 0x00 0x05 0x00 0x07 0x00 0x10 0x00 0x40 0x00 0x00 0x01 0x50 0x02 0x00 0x03
                                                                                              ^^^^ ^^^^      ^^^^

  offset:     C9E3 C9E4 C9E5 C9E6 C9E7 C9E8 C9E9 C9EA C9EB C9EC C9ED C9EE C9EF C9F0 C9F1 C9F2 C9F3 C9F4 C9F5 C9F6
  program #1  0x01 0x00 0x02 0x00 0x03 0x00 0x05 0x00 0x07 0x00 0x10 0x00 0x40 0x00 0x00 0x01 0x00 0x05 0x00 0x11
  program #2  0x01 0x00 0x02 0x00 0x03 0x00 0x05 0x00 0x07 0x00 0x10 0x00 0x40 0x00 0x00 0x01 0x50 0x02 0x00 0x03
                                                                                              ^^^^ ^^^^      ^^^^
  This change the paytable to the following values:

    Program #1                  Program #2

    Five of a Kind  1100        Five of a Kind   300
    Royak Flush      500        Royak Flush      250
    Street Flush     100        Street Flush     100
    Poker             40        Poker             40
    Full House        10        Full House        10
    Farbe              7        Farbe              7
    Strasse            5        Strasse            5
    Drilling           3        Drilling           3
    2 Paare            2        2 Paare            2
    Hohes Paar         1        Hohes Paar         1

  Will split in two different sets, since these programs were made to use as different
  game variations, and aren't user selectable.

*/
ROM_START( jolyjokrb1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Two different paytable programs. Using the 1st one (higher paytable)
	ROM_LOAD( "unbekannt.bin", 0x8000, 0x8000, CRC(327fa3d7) SHA1(2435aada2377b2f8f01d059a7aba9bc7a8993537) )
	ROM_IGNORE(                        0x8000 ) // Using the 1st program. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic26.bin", 0x0000, 0x8000, CRC(3e45dfc6) SHA1(8fd0b0cc00cdd96244ae7e7a91f6613b1c144ee0) )
	ROM_IGNORE(                   0x8000 )  // Identical halves. Discarding 2nd half
	ROM_LOAD( "ic25.bin", 0x8000, 0x8000, CRC(1bd067af) SHA1(9436fe085ba63c00a12ea80903470a84535e3dc1) )
	ROM_IGNORE(                   0x8000 )  // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokrb_nvram.bin", 0x0000, 0x0800, CRC(17007bb5) SHA1(72e08096293ce4fbde205a63b5ecd9641dbee017) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_ic40.bin",    0x0000, 0x0200, CRC(0b671bba) SHA1(92d512e02b50f98b7bc5a60deee4fee722656c4f) )
ROM_END

ROM_START( jolyjokrb2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Two different paytable programs. Using the 2nd one (lower paytable)
	ROM_LOAD( "unbekannt.bin", 0x8000, 0x8000, CRC(327fa3d7) SHA1(2435aada2377b2f8f01d059a7aba9bc7a8993537) )
	ROM_CONTINUE(              0x8000, 0x8000)  // overlapping with the 2nd program.

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic26.bin", 0x0000, 0x8000, CRC(3e45dfc6) SHA1(8fd0b0cc00cdd96244ae7e7a91f6613b1c144ee0) )
	ROM_IGNORE(                   0x8000 )  // Identical halves. Discarding 2nd half
	ROM_LOAD( "ic25.bin", 0x8000, 0x8000, CRC(1bd067af) SHA1(9436fe085ba63c00a12ea80903470a84535e3dc1) )
	ROM_IGNORE(                   0x8000 )  // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokrb_nvram.bin", 0x0000, 0x0800, CRC(17007bb5) SHA1(72e08096293ce4fbde205a63b5ecd9641dbee017) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_ic40.bin",    0x0000, 0x0200, CRC(0b671bba) SHA1(92d512e02b50f98b7bc5a60deee4fee722656c4f) )
ROM_END


/*
  Jolly Joker (Apple Time)
  With Motorola SC67334CP 44N46 instead of regular M6845.

  Program ROM is faulty...
*/
ROM_START( jolyjokrc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b+g_jj1.bin", 0x8000, 0x8000, BAD_DUMP CRC(2fd585a7) SHA1(d517141ef17da9b61ddf3e6e8c41f5173de980c3) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jje.bin", 0x0000, 0x8000, CRC(d9da6e74) SHA1(2a966e2d7849aed9d952a9d94c99955ba651d14d) )
	ROM_LOAD( "jjd.bin", 0x8000, 0x8000, CRC(9d596852) SHA1(a29e210cbdc75717465068e85b693edcc1c642da) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp28s42.bin", 0x0000, 0x0200, CRC(0b671bba) SHA1(92d512e02b50f98b7bc5a60deee4fee722656c4f) )

	ROM_REGION( 0x0200, "plds", 0 ) // cracked PLD
	ROM_LOAD( "jjpal.bin", 0x0000, 0x0117, CRC(3b084c34) SHA1(5d186b70317ef871c9a426eb420b66efcbd918de) )
ROM_END

/*
  Jolly Joker
  Impera.

  The following two sets have the same program
  but different graphics system.

  Both sets have graphics ROMs data interleaved
  inside the second half of a 16bit 27C210 EPROM.
  The second set has some 8bits data in the first half.
  Not clear if it's for obfuscation or just are the missing
  logo graphics tiles.

  The program looks original. The former sets programs
  have differents offsets patched and moved blocks respect
  this new program.

  Didn't set as parent due to the fact that seems older.
  Maybe being more original and less patched should be set
  as parent.

*/
ROM_START( jolyjokro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impera1.bin", 0x0000, 0x10000, CRC(3cad9fcf) SHA1(09f23ae8c04e6b461e17a8b3978fe44566ffc3aa) )

	ROM_REGION( 0x10000, "gfxpool", 0 )
	ROM_LOAD( "impera2.bin", 0x0000, 0x10000, CRC(aa86dba6) SHA1(fe189dde83bd855f4a0b34b20c161a9addc15017) )
	ROM_CONTINUE(            0x0000, 0x10000)   // discarding 1st empty half (0000-ffff)

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_FILL(              0x0000, 0x10000, 0xff)   // deinterleaved GFX data will be placed here

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokro_nvram.bin", 0x0000, 0x0800, CRC(1f69e567) SHA1(86695ca6f9f93c6badd092410611d8061edf8efa) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "1_impera_color_ii.bin", 0x0000, 0x0200, CRC(9d62f9f5) SHA1(68300c25c7eaa13a3fdbf91ab0711d0bc530543d) )
ROM_END

ROM_START( jolyjokrp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impera1.bin", 0x0000, 0x10000, CRC(3cad9fcf) SHA1(09f23ae8c04e6b461e17a8b3978fe44566ffc3aa) )

	ROM_REGION( 0x10000, "gfxpool", 0 )
	ROM_LOAD( "9c_1ff1.bin", 0x00000, 0x10000, CRC(4b8f0821) SHA1(0821eed07f5e98b66d87a3079756dad72ffe9665) )
	ROM_CONTINUE(            0x00000, 0x10000)  // discarding 1st half (0000-ffff), but has some data. leftover?

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_FILL(              0x0000, 0x10000, 0xff)   // deinterleaved GFX data will be placed here

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokrp_nvram.bin", 0x0000, 0x0800, CRC(c8706e75) SHA1(421420b1ee82615faf290d1204342cdde776ffaf) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "impera_color_ii.bin", 0x0000, 0x0200, CRC(9d62f9f5) SHA1(68300c25c7eaa13a3fdbf91ab0711d0bc530543d) )
ROM_END

/*
  Jolly Joker (Otti Karl logo)
  Original Impera program.

  Both sets have graphics ROMs data interleaved
  inside the second half of a 16bit 27C210 EPROM.
  The second set has full garbage in the first half.

  The PLD (PAL16L8) was bruteforced.

*/
ROM_START( jolyjokrs )  // Jolly Joker (Otti Karl logo, set 1)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jolly_joker_standard_09.90.bin", 0x0000, 0x10000, CRC(3cad9fcf) SHA1(09f23ae8c04e6b461e17a8b3978fe44566ffc3aa) )

	ROM_REGION( 0x10000, "gfxpool", 0 )
	ROM_LOAD( "02.bin", 0x0000, 0x10000, CRC(dedf3bcc) SHA1(d534005ada564aae08c02422cdb405599489aa0a) )
	ROM_CONTINUE(       0x0000, 0x10000)   // discarding 1st empty half (0000-ffff)

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_FILL(              0x0000, 0x10000, 0xff)   // deinterleaved GFX data will be placed here

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokrs_nvram.bin", 0x0000, 0x0800, CRC(1f69e567) SHA1(86695ca6f9f93c6badd092410611d8061edf8efa) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "63s481n.bin", 0x0000, 0x0200, CRC(9d62f9f5) SHA1(68300c25c7eaa13a3fdbf91ab0711d0bc530543d) )

	ROM_REGION( 0x0200, "plds", 0 )    // identical to jolyjokrt one
	ROM_LOAD( "jolyjokrs_pal16l8.bin",  0x0000, 0x0117, CRC(b235c40b) SHA1(69611fe15f54220efcdbce9efb3e2738242af692) )
ROM_END

ROM_START( jolyjokrt )  // Jolly Joker (Otti Karl logo, set 2)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.bin", 0x0000, 0x10000, CRC(3cad9fcf) SHA1(09f23ae8c04e6b461e17a8b3978fe44566ffc3aa) )

	ROM_REGION( 0x10000, "gfxpool", 0 )
	ROM_LOAD( "d27c210.bin", 0x0000, 0x10000, CRC(132e4747) SHA1(255976f672dd7cf755e198617f7d895f7b888ae7) )
	ROM_CONTINUE(            0x0000, 0x10000)   // discarding 1st half (0000-ffff) full of garbage

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_FILL(              0x0000, 0x10000, 0xff)   // deinterleaved GFX data will be placed here

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokrt_nvram.bin", 0x0000, 0x0800, CRC(1f69e567) SHA1(86695ca6f9f93c6badd092410611d8061edf8efa) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "am27s29.bin", 0x0000, 0x0200, CRC(9d62f9f5) SHA1(68300c25c7eaa13a3fdbf91ab0711d0bc530543d) )

	ROM_REGION( 0x0200, "plds", 0 )    // identical to jolyjokrs one
	ROM_LOAD( "jolyjokrt_pal16l8.bin",  0x0000, 0x0117, CRC(b235c40b) SHA1(69611fe15f54220efcdbce9efb3e2738242af692) )
ROM_END


/*
  Jolly Joker
  Impera.

  This hardware has a daughterboard that seems a solid state replacement
  for the suicide modules of funworld boards.

  It's just a theory, but the main battery is routed to the daughterboard
  connectors, when this module has not suicide program inside a RAM.

  Module contents:

  1x Rockwell R65C02P2 Mexico.
  1x 27c512 EPROM stickered 'IMPERA1'.
  1x PAL16L8A2NC.
  1x 74HC241N.
  1x 74LS245N.
  1x 74LS139N.

*/
ROM_START( jolyjokrm )  // Jolly Joker (Solid State module in suicide board)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impera1.bin", 0x0000, 0x10000, CRC(3cad9fcf) SHA1(09f23ae8c04e6b461e17a8b3978fe44566ffc3aa) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "02.bin", 0x0000, 0x8000, CRC(f0fa5941) SHA1(1fcade31ed6893ffcfd4efe97dfaaa31d24283ec) )
	ROM_LOAD( "01.bin", 0x8000, 0x8000, CRC(c3ab44dd) SHA1(e46c0fd94da561f57033647f1703fa135777ece5) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "jolyjokrm_nvram.bin", 0x0000, 0x0800, CRC(f33e66ed) SHA1(7a4b9a1b2f976d5d26f54915a213d5ac5eca0a42) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "63s481.bin", 0x0000, 0x0200, CRC(1f974802) SHA1(c9d4d72edef3c03b0c6e1a0ffc0b82f6d213d29c) )
ROM_END


/******************************** Other sets ************************************/

/*
    Title:    MULTI WIN
    Company:  FUN WORLD
    Version:  0167
    Date:     1992-11-11

    Unknown or encrypted CPU.
*/
ROM_START( multiwin )
	ROM_REGION( 0x10000*2, "maincpu", 0 )  // *2 for decrypted opcodes (see init)
	ROM_LOAD( "multiwin3.bin",  0x8000, 0x8000, CRC(995ca34d) SHA1(4d6ec10810ece493447a01af149ad8387d5f3c2f) )  // first half is full of ff's
	ROM_LOAD( "multiwin4.bin",  0x0000, 0x8000, CRC(f062125c) SHA1(93c9aa518810798f3449a28e851eb6433ba7bbf8) )  // first half is full of ff's

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "multiwin2.bin",  0x0000, 0x8000, CRC(580b3239) SHA1(362aa85c57ad0bce1d7d15a93b9998daba4d306a) )
	ROM_LOAD( "multiwin1.bin",  0x8000, 0x8000, CRC(97589aa6) SHA1(2486116637bd906cb3b32acd86fc861c48a0475e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "multi_prom.bin", 0x0000, 0x0200, BAD_DUMP CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) ) // using the joker card one
ROM_END


/*
  Multi Win
  Version 0091
  1991-09-20

  Different encryption scheme.
  Looks closer to megacard.

+--------------------------------------------------------------------------------------+
|                          +------+                                 +------+           |
|   +------+    +--------+ |      |                                 |      |           |
|   |      |    |74LS157N| |GOLDST|                                 |      | +--+      |
|   |      |    +--------+ |AR    |                                 |SOUND | |  | +--+ +---+
|   |      |    +--------+ |      |                                 |      | |  | |  |   --|
|   |GOLDST|    |74LS157N| |GM76C2|                                 |AY-3-8| |K | |L |   --|
|   |AR    |    +--------+ |8A-10 |                                 |91C   | |  | |  |   --|
|   |      |    +--------+ |      |    +-------------------+        |      | |  | |  |   --|
|   |GM68B4|    |74LS157N| |      |    | ::::::::::::::::: |        |      | +--+ +--+   --|
|   |5S    |    +--------+ +------+    +-------------------+        |      |             --|
|   |      |    +--------+                                          |      |  +--+       --|
|   |9141  |    |74LS157N|                                          |      |  |  |       --|
|   |      |    +--------+                +----------+ +----------+ |      |  |M |     +---+
|   |      |                              |PAL16L8ACN| |   EMPTY  | |      |  |  |     |
|   |      |                      +----+  +----------+ +----------+ |      |  |  |     |
|   +------+   +--------------+   |XTAL|                            |      |  +--+     |
|              |  KM6264BL-10 |   |    |   +--------------------+   +------+           |
| +----------+ |   214Y KOREA |   |16.0|   |                    |   +------+           |
| | 74LS245N | |              |   |00  |   |   FUN WORLD        |   |      |           |
| +----------+ +--------------+   +----+   |        ELEKTRONIK  |   |      |           +---+
|                                          |                    |   |      |             --|
| +----------+     +----------+            +--------------------+   |S22LD |             --|
| | 74LS374N |     | 74LS374N |                                     |      |             --|
| +----------+     +----------+  +--+     +--+   +--------------+   | 9248 |             --|
|                                |  |     |  |   |    EMPTY     |   |      |             --|
|+--------------+    +--------+  |  |     |  |   |    SOCKET    |   |EF6821|  +--+       --|
||Multi Win     |    |74LS194A|  |A |     |B |   |          IC25|   |P     |  |  |       --|
||ZG 1          |    +--------+  |  |     |  |   +--------------+   |      |  |D |       --|
||          IC10|    +--------+  |  |     |  |                      |      |  |I |       --|
|+--------------+    |74LS194A|  +--+     |  |   +--------------+   |      |  |P |       --|
|                    +--------+           +--+   |Multi Win     |   |      |  |1 |       --|
|+--------------+    +--------+                  |91/F/1        |   |      |  |  |       --|
||Multi Win     |    |74LS194A|  +--+ +--+ +--+  |          IC37|   |      |  +--+       --|
||ZG 2          |    +--------+  |  | |  | |  |  +--------------+   +------+             --|
||          IC11|    +--------+  |C | |  | |  |                     +------+             --|
|+--------------+    |74LS194A|  |  | |D | |E |  +--------------+   |      |             --|
|                    +--------+  |  | |  | |  |  |Multi Win     |   |      |             --|
|+--------+ +-------+            +--+ |  | |  |  |91/F/2        |   |      |             --|
||74LS174N| |74LS02N|                 |  | +--+  |          IC41|   |      |             --|
|+--------+ +-------+                 +--+       +--------------+   |MC68B2|             --|
|                                                                   |1P    |             --|
|+--+  +--+  +--+                                              +--+ |      |             --|
||  |  |  |  |  |          +--+        -                       |  | |0K2P  |             --|
||F |  |  |  |  |          |I |                                |J | |      |             --|
||  |  |G |  |H |          |  |     BATTERY                    |  | |LLFT92|             --|
||  |  |  |  |  |          +--+                                |  | |24    |           +---+
|+--+  |  |  |  |                      +                       +--+ |      |           |
|      |  |  |  |                                                   |      |           |
|      +--+  +--+                                                   +------+           |
+--------------------------------------------------------------------------------------+

A: 74LS368AN       H: 74LS374N
B: 74LS245N        I: MAX690CPA
C: GD74LS393       J: DM74LS02N
D: MN74HC241       K: ULN2003AN
E: 74LS139N        L: SN74LS04N
F: 74LS174N        M: ULN2003A
G: DM74S472BN


DIP 1:
+-------------------------------+
|O N                            |
|+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
|| | |#| |#| |#| | | | | | | | ||
|+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
||#| | | | | | | |#| |#| |#| |#||
|+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
| 1   2   3   4   5   6   7   8 |
+-------------------------------+



FUNWORLD CUSTOM CPU-MODULE:

       +---------------------+
 A10 ==|01                 40|==
     ==|                     |==
     ==|       [] TI74F245A  |==
     ==|                     |==
     ==|                     |==
     ==|       +--+          |==
     ==|       |  | 65SC02   |==
 VCC ==|       +--+          |== A00
 A08 ==|                     |== A01
 A09 ==|                     |== A02
     ==|       [] 7400       |== A03
     ==|                     |== A04
     ==|    +-----------+    |== A05
     ==|    | CY7C291A  |    |== A06
     ==|    |           |    |== A07
     ==|    |           |    |==
     ==|    |           |    |==
     ==|    |     1     |    |==
     ==|    +-----------+    |==
     ==|20                 21|== GND
       +---------------------+

*/
ROM_START( multiwina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "multiwin_91-f-1.ic37",  0x8000, 0x8000, CRC(88f122e0) SHA1(6b218e614ac86089321ad2ecb35b956de10031f3) )  // code in the 2nd half
	ROM_LOAD( "multiwin_91-f-2.ic41",  0x0000, 0x8000, CRC(a09eda44) SHA1(b8bb6a2aa1b703b56e5fb0887d7e5dca28c388ea) )  // code in the 2nd half

	ROM_REGION( 0x0800, "decode", 0 )   // from the CY7C291A inside of the custom CPU.
	ROM_LOAD( "multiwin_cy7c291a.cpu",  0x0000, 0x0800, CRC(874abdaa) SHA1(b6457d5d501cdf0d38b01e3dadb48b8b97193db3) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "multiwin_zg2.ic11",   0x0000, 0x8000, CRC(2502778a) SHA1(04990c454ca55f1b3e6fdafebce3f2e9e6782329) )
	ROM_LOAD( "multiwin_zg1.ic10",   0x8000, 0x8000, CRC(38547f3e) SHA1(1322a35ebc358ebc193c029500dc0abd6fd0c113) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm74s472.ic13",  0x0000, 0x0200, CRC(3bf9870c) SHA1(2bd99102a1f80ec07dbe3d41bdbcff11b237d680) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "multiwin_pal16l8acn.bin",  0x0000, 0x0104, NO_DUMP )
ROM_END


/*
  Multi Win
  Amatic, 2001.

  Unknown version.
  Game encrypted, using a daughterboard
  with a Mexican R65C02 + Altera EPM7032.

*/
ROM_START( multiwinb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "win3.bin", 0x8000, 0x8000, CRC(5f01d620) SHA1(ddd86cba383f1e40e3821a58fc0ff7c1ac24de10) )

	ROM_REGION( 0x0800, "altera", 0 )    // Altera EPM7032 object file
	ROM_LOAD( "multiw1_epm7032.pof", 0x0000, 0x07ae, CRC(922d8d72) SHA1(2ad01fd0b4ddaa1098f3ffc203b6c502d1afd73a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "win2.bin", 0x0000, 0x8000, CRC(bf031e5e) SHA1(1c971be5eb3e8c1a1b69ec9d00e46aa1fe26b8ee) )
	ROM_LOAD( "win1.bin", 0x8000, 0x8000, CRC(8ea33d92) SHA1(5aa439181f0751040229ce75101dbdf8b3d68b6c) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "291_n147.bin", 0x0000, 0x0800, CRC(e09e3a77) SHA1(2842875cff63650e15bf7eda3a730446c671c3ea) )

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "m1-20v8.bin",  0x0000, 0x0157, CRC(b05b51da) SHA1(5e43cc12cd8bcd820cb89d3008a02ea857daba6f) )
	ROM_LOAD( "m2-20v8.bin",  0x0200, 0x0157, CRC(11852de6) SHA1(33fcf6f752a116e4df0492d17e0d5e53b31ce77d) )
	ROM_LOAD( "16v8-3.bin",   0x0400, 0x0117, NO_DUMP )
	ROM_LOAD( "jo393-4.bin",  0x0800, 0x0117, CRC(6eef3e7d) SHA1(2e1bb576f431743b133833c7c2f91a41a4fcbc37) )
ROM_END


/*
  Power Card (Fun World)
  Version 0263 / 1993-10-22

  Amatic encrypted CPU based on 65SC02 (bitwise) family,
  with the following components inside:

  1x CY7C291A (prom replacement).
  1x 65SC02 (CPU).
  1x TI74F245A (3-state octal latch from Texas Instruments).
  1x 7400 (quad 2-input NAND gate).

  (Same as Mega Card CPU)


  PCB Layout...
  .----------------------------------------------------------------------------------------------------------------------.
  |                                                                                                                      |
  |   .--------.                    .--------.                                                  .-------.                |
  |   |        |   .------------.   |        |                                                  |       |                |
  |   |        |   |  GD74LS157 |   |        |                                                  |       |   .---.        '---.
  |   |        |   '------------'   |Goldstar|                                                  |YAMAHA |   |   | .---.   ---|
  |   |GOLDSTAR|                    |        |                                                  |       |   |ULN| |   |   ---|
  |   |        |   .------------.   |GM76C28A|                                                  |       |   |200| |74L|   ---|
  |   |        |   |  GD74LS157 |   |  -10   |                                                  |       |   |3AN| |S04|   ---|
  |   |GM68B45S|   '------------'   |        |                                                  |YM2149F|   |   | |P  |   ---|
  |   |        |                    |        |                                                  |       |   |   | |   |   ---|
  |   |        |   .------------.   |        |         .-------------------------------------.  |       |   '---' |   |   ---|
  |   |        |   |  GD74LS157 |   |        |         |## ooooooooooooooooooooooooooooooo ##|  |       |         '---'   ---|
  |   |        |   '------------'   |        |         |## ooooooooooooooooooooooooooooooo ##|  |       |                 ---|
  |   |        |                    '--------'         '-------------------------------------'  |       |    .---.        ---|
  |   |        |   .------------.                                                               |       |    |   |        ---|
  |   |        |   |  GD74LS157 |                           .--------------.  .--------------.  |       |    |ULN|       .---'
  |   |        |   '------------'                           |TIBPAL16L8-25C|  | EMPTY SOCKET |  |       |    |200|       |
  |   |        |                                            '--------------'  '--------------'  |       |    |3AN|       |
  |   |        |     .------------------.   .-----.         .--------------------------------.  |       |    |   |       |
  |   '--------'     |                  |   |XTAL |         |Lf. Nr: 00147                   |  '-------'    '---'       |
  |                  |    KM6264BL-10   |   |16.00|         |Type: 0-A                       |  .-------.                |
  | .-------------.  |                  |   |MHz  |         |Datum: 27.10                    |  |       |                |
  | |  GD74LS245  |  |                  |   |     |         |                                |  |       |                |
  | '-------------'  '------------------'   '-----'         |                        AMATIC  |  |       |                '---.
  |                                                         |                                |  |       |                 ---|
  | .-------------.        .------------.                   '--------------------------------'  |       |                 ---|
  | | HD74LS374P  |        | HD74LS374P |                                                       |MC6821P|                 ---|
  | '-------------'        '------------'   .---.        .---.          .--------------------.  |       |                 ---|
  |                                         |GD7|        |GD7|          |                    |  |       |                 ---|
  |.--------------------.    .----------.   |4LS|        |4LS|          |    EMPTY SOCKET    |  |       |    .---.        ---|
  ||POWER C.            |    | 74LS194AN|   |368|        |245|          |                    |  |       |    |  8|        ---|
  ||        ZG 1        |    '----------'   |A  |        |   |          |                    |  |       |    |   |        ---|
  ||              27C256|                   |   |        |   |          '--------------------'  |       |    |   |        ---|
  ||                IC10|    .----------.   '---'        |   |                                  |       |    |DIP|        ---|
  |'--------------------'    | 74LS194AN|                |   |          .--------------------.  |       |    |   |        ---|
  |                          '----------'                |   |          |LETS GO 1           |  |       |    |   |        ---|
  |                                                      '---'          |    263 / A / 1     |  |       |    |   |        ---|
  |.--------------------.    .----------.                               |              27C256|  |       |    |  1|        ---|
  ||POWER C.            |    | 74LS194AN|   .---.   .---.   .---.       |                IC37|  |       |    '---'        ---|
  ||        ZG 2        |    '----------'   |GD7|   |KS7|   |GD7|       '--------------------'  '-------'                 ---|
  ||              27C256|                   |4LS|   |4AH|   |4LS|                               .-------.                 ---|
  ||                IC11|    .----------.   |393|   |CT2|   |139|       .--------------------.  |       |                 ---|
  |'--------------------'    | 74LS194AN|   |   |   |41N|   |   |       |LETS GO 1           |  |       |                 ---|
  |                          '----------'   |   |   |   |   |   |       |    263 / A / 2     |  |       |                 ---|
  |                                         '---'   |   |   '---'       |              27C256|  |       |                 ---|
  |.----------.  .----------.                       |   |               |                IC41|  |       |                 ---|
  || GD74LS174|  |HD74LS02P |                       |   |               '--------------------'  |MC6821P|                 ---|
  |'----------'  '----------'                       '---'                                       |       |                 ---|
  |                                                                                      .---.  |       |                 ---|
  |.---.  .---.  .---.              .---.                                                |HD7|  |       |                 ---|
  ||GD7|  |   |  |HD7|              |MAX| Maxim                                          |4LS|  |       |                 ---|
  ||4LS|  |N82|  |4LS|              |690| MAX690CPA                                      |02P|  |       |                 ---|
  ||174|  |S14|  |374|              '---'                                                |   |  |       |                 ---|
  ||   |  |7N |  |P  |                                                                   |   |  |       |                 ---|
  ||   |  |   |  |   |                                                                   '---'  |       |                 ---|
  |'---'  |   |  |   |                                                                          |       |                .---'
  |       |   |  |   |                                                                          |       |                |
  |       '---'  '---'                                                                          '-------'                |
  |                                                                                                                      |
  '----------------------------------------------------------------------------------------------------------------------'

   DIP SW Bank
  .---------------.
  |#| |#|#|#|#|#|#|
  |---------------|
  | |#| | | | | | |
  '---------------'
   1 2 3 4 5 6 7 8


  Looks like a Bonus Card / Big Deal clone.
  Inside the program ROM there is a reference to "Mega Card",
  but the graphics are from Power Card.

*/
ROM_START( powercrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "263a1.bin",  0x8000, 0x8000, CRC(9e5e477d) SHA1(428464a64bea8cb478bc8033859baa47d7de0297) )  // just the 2nd half
	ROM_LOAD( "263a2.bin",  0x0000, 0x8000, CRC(11b1a13f) SHA1(766c1a45c238467d6a292795f5a159187966ceec) )  // just the 2nd half

	ROM_REGION( 0x0800, "decode", 0 )   // from the CY7C291A inside of the custom CPU
	ROM_LOAD( "powercrd_cy7c291a.bin",  0x0000, 0x0800, CRC(f1d8f35d) SHA1(2b5f9222a81a627d43fd8448385f85c71c24b914) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "power_c_zg2.ic11",   0x0000, 0x8000, CRC(108380bb) SHA1(922beffe3c06f391239125e6f4ccc86ec6980c45) )
	ROM_LOAD( "power_c_zg1.ic10",   0x8000, 0x8000, CRC(dc9e70c6) SHA1(7ac5bdc734d9829ea6349b60817445cb88d7387c) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an.bin",  0x0000, 0x0200, CRC(136245f3) SHA1(715309982fcafbce88b08237ca46acec31273938) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "powercrd_tibpal16l8.bin",  0x0000, 0x0104, CRC(b5c0a96d) SHA1(3547700e276326a27009202b2e82bc649abb33db) )
ROM_END


/*

  Mega Card (MC3).
  Ver. 0210.
  Fun World, 1993.

  Encrypted poker game.

  1x Custom encrypted Fun World CPU based on 65SC02 (Same as Power Card).
  1x GM68B45S CRTC.
  2x EF6821P PIAs.
  1x AY38910A.
  2x ULN2003AN (out).
  1x PAL16L8ACN (PLD).
  1x Maxim MAX690CPA (Microprocessor Supervisory Circuits).

  RAM:
  1x GoldStar GM76C28-10.
  1x KM6264BL-10.

  ROM:
  4x 27C256 (two for program, two for GFX).
  1x N82S147AN bipolar PROM.

  Other:

  1x 16MHz Xtal.
  1x 8 DIP switches bank.
  1x 3V. CR2025 lithium battery.

  1X 17x2 Pin male connector.
  1X 22x2 Pin edge connector.
  1X 8x2 Pin edge connector.



  PCB layout:

  .----------------------------------------------------------------------------------------------------------------------.
  |                                                                                                                      |
  |   .--------.                    .--------.                                                  .-------.                |
  |   |        |   .------------.   |        |                                                  |       |                |
  |   |        |   |  GD74LS157 |   |        |                                                  |       |   .---.        '---.
  |   |        |   '------------'   |Goldstar|                                                  |       |   |   | .---. 8 ---|
  |   |GOLDSTAR|                    |        |                                                  |       |   |ULN| |   |   ---|
  |   |        |   .------------.   |GM76C28 |                                                  |       |   |200| |74L| P ---|
  |   |        |   |  GD74LS157 |   |  -10   |                                                  |       |   |3AN| |S04| I ---|
  |   |GM68B45S|   '------------'   |        |                                                  |AY38910|   |   | |P  | N ---|
  |   |        |                    |        |                                                  |A/P    |   |   | |   |   ---|
  |   |        |   .------------.   |        |         .-------------------------------------.  |       |   '---' |   | C ---|
  |   |        |   |  GD74LS157 |   |        |         |## ooooooooooooooooooooooooooooooo ##|  |       |         '---' O ---|
  |   |        |   '------------'   |        |         |## ooooooooooooooooooooooooooooooo ##|  |       |               N ---|
  |   |        |                    '--------'         '-------------------------------------'  |       |    .---.      N ---|
  |   |        |   .------------.                                                               |       |    |   |        ---|
  |   |        |   |  GD74LS157 |                           .--------------.  .--------------.  |       |    |ULN|       .---'
  |   |        |   '------------'                           |  PAL16L8ACN  |  | EMPTY SOCKET |  |       |    |200|       |
  |   |        |                                            '--------------'  '--------------'  |       |    |3AN|       |
  |   |        |     .------------------.   .-----.         .--------------------------------.  |       |    |   |       |
  |   '--------'     |                  |   |XTAL |         |Lfnd. Nr: 0057                  |  '-------'    '---'       |
  |                  |    KM6264BL-10   |   |16.00|         |Type: 'F'                       |  .-------.                |
  | .-------------.  |                  |   |0MHz |         |Datum: 17.3.93                  |  |       |                |
  | |  GD74LS245  |  |                  |   |     |         |                                |  |       |                |
  | '-------------'  '------------------'   '-----'         |                     FUN WORLD  |  |       |                '---.
  |                                                         |                                |  |       |                 ---|
  | .-------------.        .------------.                   '--------------------------------'  |       |                 ---|
  | | HD74LS374P  |        | HD74LS374P |                                                       |EF6821P|                 ---|
  | '-------------'        '------------'   .---.        .---.          .--------------------.  |       |                 ---|
  |                                         |GD7|        |GD7|          |                    |  |       |                 ---|
  |.--------------------.    .----------.   |4LS|        |4LS|          |    EMPTY SOCKET    |  |       |    .---.        ---|
  ||MEGA                |    |74LS194AN |   |368|        |245|          |                    |  |       |    |  8|        ---|
  ||ZG 1                |    '----------'   |A  |        |   |          |                    |  |       |    |   |        ---|
  ||              27C256|                   |   |        |   |          '--------------------'  |       |    |   |     22 ---|
  ||                IC10|    .----------.   '---'        |   |                                  |       |    |DIP|        ---|
  |'--------------------'    |74LS194AN |                |   |          .--------------------.  |       |    |   |      P ---|
  |                          '----------'                |   |          |MEGA MC 3           |  |       |    |   |      I ---|
  |                                                      '---'          | 210/F/1            |  |       |    |   |      N ---|
  |.--------------------.    .----------.                               |              27C256|  |       |    |  1|        ---|
  ||MEGA                |    |74LS194AN |   .---.   .---.   .---.       |                IC37|  |       |    '---'      C ---|
  ||ZG 2                |    '----------'   |GD7|   |MN7|   |GD7|       '--------------------'  '-------'               O ---|
  ||              27C256|                   |4LS|   |4HC|   |4LS|                               .-------.               N ---|
  ||                IC11|    .----------.   |393|   |241|   |139|       .--------------------.  |       |               N ---|
  |'--------------------'    |74LS194AN |   |   |   |   |   |   |       |MEGA MC 3           |  |       |               E ---|
  |                          '----------'   |   |   |   |   |   |       | 210/F/2            |  |       |               C ---|
  |                                         '---'   |   |   '---'       |              27C256|  |       |               T ---|
  |.----------.  .----------.                       |   |               |                IC41|  |       |               O ---|
  ||GD74LS174 |  |HD74LS02P |                       |   |               '--------------------'  |EF6821P|               R ---|
  |'----------'  '----------'                       '---'                                       |       |                 ---|
  |                                           -------                                    .---.  |       |                 ---|
  |.---.  .---.  .---.              .---.   / CR-2025 \                                  |HD7|  |       |                 ---|
  ||GD7|  |   |  |HD7|              |MAX|  |  LITHIUM  |                                 |4LS|  |       |                 ---|
  ||4LS|  |N82|  |4LS|              |690|  |  BATTERY  |                                 |02P|  |       |                 ---|
  ||174|  |S14|  |374|              '---'   \  +3V.   /                                  |   |  |       |                 ---|
  ||   |  |7AN|  |P  |              Maxim     -------                                    |   |  |       |                 ---|
  ||   |  |   |  |   |            MAX690CPA                                              '---'  |       |                 ---|
  |'---'  |   |  |   |                                                                          |       |                .---'
  |       |   |  |   |                                                                          |       |                |
  |       '---'  '---'                                                                          '-------'                |
  |                                                                                                                      |
  '----------------------------------------------------------------------------------------------------------------------'


  The encrypted CPU:

  It's a DIP40 custom IC. It has inside a PLCC28 IC and 3 dies:

  The PLCC28 chip is a CY7C291A (prom replacement)
  DIE#1 is a TI74F245A, a 3-state octal latch from Texas Instruments.
  DIE#2 is the CPU, a 65SC02.
  DIE#3 is a 7400, a quad 2-input NAND gate.

  Is unknown how they are connected.

*/

ROM_START( megacard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mega_mc3_210-f-1.ic37",  0x8000, 0x8000, CRC(747f5ed1) SHA1(06757bb6a792dca93978b17b54c28e413e3720b1) )  // just the 2nd half
	ROM_LOAD( "mega_mc3_210-f-2.ic41",  0x0000, 0x8000, CRC(373094d2) SHA1(8aed2502e89b0e7522e88f351ac256f1afad7ee8) )  // just the 2nd half

	ROM_REGION( 0x0300, "die_65sc02", 0 )   // from the 65SC02 die inside of the custom CPU
	ROM_LOAD( "gteu65decoderom.bin",   0x0000, 0x02f1, CRC(089af0c6) SHA1(0f46a73a7859a694a07ebe74d476fae80e57e329) )

	ROM_REGION( 0x0800, "decode", 0 )   // from the CY7C291A inside of the custom CPU
	ROM_LOAD( "megacard_cy7291a.bin",  0x0000, 0x0800, CRC(596f01ac) SHA1(a8606a1dd385fbb46b1a96f11a759fa6580803d8) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mega_zg2.ic11",   0x0000, 0x8000, CRC(0993d380) SHA1(e5e2386948fba2fb6b79339af27bd1d0f1e198ca) )
	ROM_LOAD( "mega_zg1.ic10",   0x8000, 0x8000, CRC(de2e2dd0) SHA1(17962d84838f39de41bc7a41d399fd18cd0bd5b7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an.bin",  0x0000, 0x0200, CRC(136245f3) SHA1(715309982fcafbce88b08237ca46acec31273938) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "megacard_pal16l8.bin",  0x0000, 0x0117, CRC(3159a548) SHA1(ad904fa35b78570e44323469967803e34ef4bc0c) )
ROM_END

/*

  Mega Card (MC3).
  Ver. 0053.
  Fun World, 1992.

  Poker game with encrypted CPU: F - 10/03/1992.

*/

ROM_START( megacarda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "megamc3_0057-f-1.ic37",  0x8000, 0x8000, CRC(ca3becc0) SHA1(04dd8cebc53241d35eab842766b59e0f0e091b86) )  // just the 2nd half
	ROM_LOAD( "megamc3_0057-f-2.ic41",  0x0000, 0x8000, CRC(91add9f8) SHA1(b0fe9015fe4760de73f2ee00c89a16c19340a884) )  // just the 2nd half

	ROM_REGION( 0x0300, "die_65sc02", 0 )   // from the 65SC02 die inside of the custom CPU, version 0210.
	ROM_LOAD( "gteu65decoderom.bin",   0x0000, 0x02f1, CRC(089af0c6) SHA1(0f46a73a7859a694a07ebe74d476fae80e57e329) )

	ROM_REGION( 0x0800, "decode", 0 )   // from the CY7C291A inside of the custom CPU, version 0210.
	ROM_LOAD( "megacard_cy7291a.bin",  0x0000, 0x0800, CRC(596f01ac) SHA1(a8606a1dd385fbb46b1a96f11a759fa6580803d8) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mega_zg2.ic11",   0x0000, 0x8000, CRC(0993d380) SHA1(e5e2386948fba2fb6b79339af27bd1d0f1e198ca) )
	ROM_LOAD( "mega_zg1.ic10",   0x8000, 0x8000, CRC(de2e2dd0) SHA1(17962d84838f39de41bc7a41d399fd18cd0bd5b7) )

	ROM_REGION( 0x0200, "proms", 0 )  // original BP has bit 1 = 0 along the dump. the device is damaged. rest of bits match the parent dump.
	ROM_LOAD( "n82s147an.ic3",  0x0000, 0x0200, CRC(136245f3) SHA1(715309982fcafbce88b08237ca46acec31273938) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "megacarda_pal16l8.bin",  0x0000, 0x0117, CRC(fcda7872) SHA1(60acdb968e6229a8f71c2e29d22e132906a65bd5) )  // cracked.
ROM_END


/*
  Joker Card 300
  1993, Amatic Trading GmbH, Austria.
  Language seems to be Czech.

  There are tiles from Vesely Svet (Sprightly World). Maybe a leftover.
  Program roms are encrypted. Seems to be a Big Deal clone, running in
  Fun World Multi Win hardware.

  Unfortunatelly, the graphics ROM vesely_zg_1.ic10 has address 8 line (leg 25) shorted.
  Seems that the protection diode was blown due to a bad handling.

  With forensics techniques, a special device was constructed to process
  the faulty ROM and try to read the contents. Fortunatelly after all these
  efforts, we got a perfect and complete dump.


  Specs:

  1x Custom Fun World CPU, based on 6502 family. Silkscreened "Fun World Elektronik".
  1x Maxim MAX690CPA - Microprocessor Supervisory IC (DIP 8).

  2x MC68B21P PIAs.
  1x GM68B45S CRT Controller.

  1x AY-3-8910.
  1x TDA2003 Audio amplifier.

  4x ROM 27C256.
  1x RAM GM76C28A.
  1x RAM KM6264BL.
  1x PROM N82S147AN.

  1x PAL 16L8ACN

  1x 8 DIP switches bank.
  1x Push button (SW2). (reset?)
  1x Variable Resistor (VR1).
  1x 16 MHz. Crystal.
  1x CR2025 (3V) battery.

  1x 2x17 pin male connector.
  1x 2x22 Edge connector.
  1x 2x8  Edge connector.

  ------------------------------------------------------------------------

  In IC41.bin you can see the following strings:

  AMATIC TRADING GMBH  AUSTRIA

  JEDE  UNERLAUBTEVERAENDERUNG
  BZWKOPIE  WIRD  DENZORN DER
  GOETTERAUF SICH  ZIEHEN

  VERSION A267BC 200/300 KARTE
  1993-11-29

  ------------------------------------------------------------------------

  ic41.bin   1ST AND 2ND HALF IDENTICAL
  ic37.bin   1ST AND 2ND HALF IDENTICAL
  ic10.bin   BADADDR    xxxxxx-xxxxxxxx

*/

ROM_START( jokercrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic41.bin",   0x4000,  0x4000, CRC(d36188b3) SHA1(3fb848fabbbde9fbb70875b3dfef62bfb3a8cbcb) )
	ROM_IGNORE(                      0x4000 )   // Identical halves. Discarding 2nd half
	ROM_LOAD( "ic37.bin",   0xc000,  0x4000, CRC(8e0d70c4) SHA1(018f92631acbe98e5826a41698f0e07b4b46cd71) )
	ROM_IGNORE(                      0x4000 )   // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "vesely_zg_2.ic11", 0x0000, 0x8000, CRC(21d05a57) SHA1(156c18ec31b08e4c4af6f73b49cb5d5c68d1670f) )
	ROM_LOAD( "vesely_zg_1.ic10", 0x8000, 0x8000, CRC(f315587a) SHA1(6f24dd24ae4c48f2ee1db96d5cbff863734048de) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic13.bin", 0x0000, 0x0200, CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END


/*
    Mongolfier New
    --------------

    - 1x G65SC02P2 (main)
    - 1x KC89C72 (sound)
    - 1x TDA2003 (sound)
    - 1x MC68B45P (CRT controller)
    - 2x EF6821P (Peripheral Interface Adapter)
    - 1x TSC87C52-16CB (PLCC44)(Programmable 8bit Microcontroller, now dumped)
    - 1x M48Z08-100PC1 (Zero Power RAM - Lithium Battery)
    - 1x oscillator 16.0000 MHz

    - 3x M27C512
    - 1x PROM AM27S29PC
    - 1x PALCE16V8H (read protected)

    - 1x JAMMA edge connector
    - 1x trimmer (volume)
    - 2x 8 DIP switches
    - 1x 4 DIP switches
    - 1x green led

    ----------------------------------------------

    Needs FF in the NVRAM offset 0x82 to boot...
*/

ROM_START( mongolnw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prgteov.2.3m.u16", 0x8000, 0x8000, CRC(996b851a) SHA1(ef4e3d036ca10b33c83749024d04c4d4c09feeb7) )
	ROM_IGNORE(                           0x8000 )  // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "cpu1", 0 )    // TSC87C52-16CB MCU Code
	ROM_LOAD( "tsc87c52-mf.u40", 0x0000, 0x02000 , CRC(ae22e778) SHA1(0897e05967d68d7f23489e98717663e3a3176070) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mong.rc.b2.u3", 0x0000, 0x8000, CRC(5e019b73) SHA1(63a544dccb9589e5a6b938e604c09d4d8fc060fc) )
	ROM_IGNORE(                        0x8000 ) // Identical halves. Discarding 2nd half
	ROM_LOAD( "mong.rc.c1.u2", 0x8000, 0x8000, CRC(e3fc24c4) SHA1(ea4e67ace63b55a76365f7e11a67c7d420a52dd7) )
	ROM_IGNORE(                        0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "mongolnw_nvram.bin", 0x0000, 0x0800, CRC(700531fa) SHA1(a8bcf86df6bd06d2ee54b4898dd7822060b81dba) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc_mf.u24", 0x0000, 0x0200, CRC(da9181af) SHA1(1b30d992f3b2a4b3bd81e3f99632311988e2e8d1) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce16v8h_mf.u5", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
ROM_END


/*
    Soccer New (italian)
    --------------------

    - 1x G65SC02P2 (main)
    - 1x KC89C72 (sound)
    - 1x TDA2003 (sound)
    - 1x MC68B45P (CRT controller)
    - 1x EF68B21P (Peripheral Interface Adapter)
    - 1x EF6821P (Peripheral Interface Adapter)
    - 1x TSC87C52-16CB (PLCC44)(Programmable 8bit Microcontroller, now dumped)
    - 1x M48Z08-100PC1 (Zero Power RAM - Lithium Battery)
    - 1x oscillator 16.0000MHz

    - 3x M27C512
    - 1x PROM AM27S29PC
    - 1x PALCE16V8H (read protected)

    - 1x JAMMA edge connector
    - 1x trimmer (volume)
    - 2x 8 DIP switches
    - 1x 4 DIP switches
    - 1x green led

    am27s29pc_sn.u24    1ST AND 2ND HALF IDENTICAL
    prgteo2gv2.3.u16    1ST AND 2ND HALF IDENTICAL
    soccer1.u2          1ST AND 2ND HALF IDENTICAL
    soccer2.u3          1ST AND 2ND HALF IDENTICAL
    tsc87c52-sn.u40           1xxxxxxxxxxxx = 0xFF

    ----------------------------------------------

    Needs FF in the NVRAM offset 0x82 to boot...
*/

ROM_START( soccernw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prgteo2gv2.3.u16", 0x8000, 0x8000, CRC(c61d1937) SHA1(c516f13a108da60b7ccee338b63a025009ef9099) )
	ROM_IGNORE(                           0x8000 )  // Identical halves. Discarding 2nd half

	ROM_REGION( 0x10000, "cpu1", 0 )    // TSC87C52-16CB MCU Code
	ROM_LOAD( "tsc87c52-sn.u40", 0x0000, 0x02000 , CRC(af0bd35b) SHA1(c6613a7bcdec2fd6060d6dcf639654568de87e75) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "soccer2.u3", 0x0000, 0x8000, CRC(db09b5bb) SHA1(a12bf2938f5482ea5ebc0db6fd6594e1beb97017) )
	ROM_IGNORE(                     0x8000 )    // Identical halves. Discarding 2nd half
	ROM_LOAD( "soccer1.u2", 0x8000, 0x8000, CRC(564cc467) SHA1(8f90c4bacd97484623666b25dae77e628908e243) )
	ROM_IGNORE(                     0x8000 )    // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "soccernw_nvram.bin", 0x0000, 0x0800, CRC(607247bd) SHA1(06bbed08166d8930f14e1f41843ac7faeded263d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc_sn.u24", 0x0000, 0x0200, CRC(d02894fc) SHA1(adcdc912cc0b7a7f67b122fa94fca921c957b282) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce16v8h_sn.u5", 0x0000, 0x0117, NO_DUMP ) // PLD is read protected
ROM_END


/*
    Saloon (France, encrypted)
    --------------------------

    - 1x 65SC02 (main)
    - 1x 8948? (sound)
    - 1x MC6845P (CRT controller)
    - 1x oscillator 16.000000 MHz

    - 2x M27C512
    - 1x M27C256B
    - 1x PROM N82S147N
    - 1x GAL16V8-25LNC (read protected)
    - 1x GAL18CV8-25 (read protected)

    - 1x HY6264ALP-12
    - 1x UM6116K-3L

    - 1x JAMMA edge connector
    - 1x trimmer (volume?)
    - 1x LM380N (amplifier)
    - 1x ULN2803A (8 Darlington arrays)
    - 1x battery
    - 1x test button
    - 1x indicator LED
    - NO DIP switches

    Board has printed "LEOPARDO 5"
*/

ROM_START( saloon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1s.bin", 0x8000, 0x8000, CRC(66938330) SHA1(09118d607eff7121472db7d2cc24079e063dc7cf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3s.bin", 0x0000, 0x8000, CRC(babc0964) SHA1(f084465cc34ea7ac19091d3e75ef7d55c48273ae) )
	ROM_LOAD( "2s.bin", 0x8000, 0x8000, CRC(39a792d5) SHA1(45c956a4a33587238a24eed602039115db1bb4b6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147_saloon.bin", 0x0000, 0x0200, CRC(f424ccc1) SHA1(6df1215f58cca786e9f0ea4bf35407cf7fe21d83) )

	ROM_REGION( 0x0100, "i2cmem", 0 )
	ROM_LOAD( "saloon_i2c.bin", 0x0000, 0x0100, CRC(57e9ce5a) SHA1(3a8420a8ed50faf31a30c01f9438f1df95e5789a) )
ROM_END

ROM_START( nevadafw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1s.bin", 0x8000, 0x8000, CRC(66938330) SHA1(09118d607eff7121472db7d2cc24079e063dc7cf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3s.bin", 0x0000, 0x8000, CRC(babc0964) SHA1(f084465cc34ea7ac19091d3e75ef7d55c48273ae) )
	ROM_LOAD( "2s.bin", 0x8000, 0x8000, CRC(39a792d5) SHA1(45c956a4a33587238a24eed602039115db1bb4b6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147_saloon.bin", 0x0000, 0x0200, CRC(f424ccc1) SHA1(6df1215f58cca786e9f0ea4bf35407cf7fe21d83) )

	ROM_REGION( 0x0100, "i2cmem", 0 )
	ROM_LOAD( "nevada_i2c.bin", 0x0000, 0x0100, CRC(cdb519db) SHA1(b7781cebee3c8f788a78f8fec9314ae5af513514) )
ROM_END

/**** Fun World Quiz ****

  Fun World Quiz
  Oehlinger Ges.m.b.H.

  Horizontal Display
  Currents
  +5V   2A
  +12V  0.5A
  -5V   -

  4 way joystick.
  1 action button.


                Pinouts
  --------------+--+--+--------------
            GND |A |01| GND
            GND |B |02| GND
            GND |C |03| GND
            +5V |D |04| +5V
           +12V |E |05| +12V
                |F |06|
                |H |07|
            ... |J |08| ... Empty (*)
        Credits |K |09| ...
            ... |L |10| ...
            ... |M |11| 1P Start
            ... |N |12| 2P Start
    1P Action 1 |P |13| 1P Action 2
    2P Action 1 |R |14| 2P Action 2
        1P Left |S |15| 1P Right
        1P Down |T |16| 1P Up
        2P Left |U |17| 2P Right
        2P Down |V |18| 2P Up
    Video Green |W |19| Video Red
     Video Sync |X |20| Video Blue
        Speaker |Y |21| Video GND
            ... |Z |22| Speaker

  Some pinout letters are missing.
  This was a decision made by engineers
  to avoid mix-ups.

  (*) Normally used in System Austria pinout.


  DIP Switches (simple on/off)
  --+-----------------------------------------------------------------------
  1 | Buchhaltung / Bookkeeping.
  2 | Nicht verwendet / Not used.
  3 | Ohne Zahlen (Wien) / No numbers (Vienna).
  4 | Nicht verwendet / Not used.
  5 | Nicht verwendet / Not used.
  6 | Richtige Antwort wird angezeigt / Right answer is shown.
  7 | Frage wird bei Einsatz angezeigt / Question is shown when bet is made.
  8 | Spiel mit Einsatzwahl / Game with betting.
  --+-----------------------------------------------------------------------

*/
ROM_START( funquiz )    // Fun World Quiz
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kqu_6.bin", 0xc000, 0x4000, CRC(50f0e586) SHA1(85ce5b95283113e2ac94fd882c57ce1b26135ed0) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_FILL(              0x0000, 0x4000, 0xff)
	ROM_LOAD( "q_3.bin",   0x4000, 0x4000, CRC(0dafa07a) SHA1(5d6fa842c617f92fad14a597396249d5a4d28c9a) )
	ROM_FILL(              0x8000, 0x4000, 0xff)
	ROM_LOAD( "q_2.bin",   0xc000, 0x4000, CRC(ce07c6e1) SHA1(6b77a9198e29c195d983b856e8826e8174945321) )

//  One unpopulated questions socket... Maybe sport_1 is missing
	ROM_REGION( 0x100000, "questions", ROMREGION_ERASEFF )

	// 01 - Allgemein
	ROM_LOAD( "allg_1.bin",  0x00000, 0x8000, CRC(1351cf56) SHA1(50e89c3e6d256bcf7f1d3c0dbef935e4e8561096) )
	ROM_LOAD( "allg_2.bin",  0x08000, 0x8000, CRC(021492a4) SHA1(b59e1303f17c9e5af05a808118ae729205690bb2) )
	ROM_LOAD( "allg_3.bin",  0x10000, 0x8000, CRC(de8e055f) SHA1(593fce143ee5994087bbac8b51ac7e2d02e8701c) )
	ROM_LOAD( "allg_4.bin",  0x18000, 0x8000, CRC(5c87177a) SHA1(a8a8318165008cb3295e25d4b4d38146f44a32fc) ) // this one has the category in the rom in ALL caps, is it official?
	ROM_LOAD( "allg_5.bin",  0x20000, 0x8000, CRC(83056686) SHA1(00f14ded371751d54a391bf583d940b32ddeae58) )

	// 02 - Geschichte
	ROM_LOAD( "gesch_1.bin", 0x28000, 0x8000, CRC(6f785938) SHA1(1e3df7c262d8cb7d7981c9d424d4c1361fe55b50) )
	ROM_LOAD( "gesch_2.bin", 0x30000, 0x8000, CRC(3363c0ba) SHA1(f1a2a86e6abc73dd9312fa744b6929fae138e219) )

	// 03 - Geographie
	ROM_LOAD( "geo_1.bin",   0x38000, 0x8000, CRC(504da831) SHA1(4bef7bed4d300400c094cb30e9af55d3c6f47c29) )
	ROM_LOAD( "geo_2.bin",   0x40000, 0x8000, CRC(7c563119) SHA1(9f3ae3ba3e4f60d9ea4b5c95aa5aaada8bb446a3) )
	ROM_LOAD( "geo_3.bin",   0x48000, 0x8000, CRC(537ac6d2) SHA1(ed3eff9f7c71a6fe7ec59af2376a0b2ba17cc1c3) )

	// 04 - Technik
	ROM_LOAD( "tech_1.bin",  0x50000, 0x8000, CRC(cf5b9edc) SHA1(f1085c9915d21c4da581d06c9568d2bb47d467ed) )

	// 05 - Sport
	ROM_LOAD( "sport_2.bin", 0x58000, 0x8000, CRC(7accde63) SHA1(75ec3a02368d3a07d48ef9a9ff4ca7f8cf7798e2) )

	// 07 - Pop
	ROM_LOAD( "pop_1.bin",   0x60000, 0x8000, CRC(5c74781e) SHA1(0a50a706fd397bb220e31f1a7adaa4204b242888) )
	ROM_LOAD( "pop_2.bin",   0x68000, 0x8000, CRC(10103648) SHA1(6fdc1aa4dcc8919e46def1c19adc2b9686c0f72d) )


	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(f990a9ae) SHA1(f7133798b5f20dd5b8dbe5d1a6876341710d93a8) )
ROM_END

/*
  Fun World Quiz.
  12-11-1990

  CPU Brick:
  - Ricoh RP65C02G
  - M27128A
  - logic...

*/
ROM_START( funquiza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.ic12", 0xc000, 0x4000, CRC(e3b1fa5d) SHA1(3479db70249fd37b6e932d563b41550f910721c8) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "q3.ic7",   0x0000, 0x8000, CRC(7fe77d14) SHA1(0cc7395e1cdabd98206a2caa1cb6a1e165e56434) )
	ROM_LOAD( "q2.ic6",   0x8000, 0x8000, CRC(bc4f1b8f) SHA1(5af1423d2d1c698141848a6d6864cd2f4a54ca1c) )

	ROM_REGION( 0x100000, "questions", ROMREGION_ERASEFF )

	// 01 - Allgemein
	ROM_LOAD( "allg1.bin",   0x00000, 0x8000, CRC(1351cf56) SHA1(50e89c3e6d256bcf7f1d3c0dbef935e4e8561096) )
	ROM_LOAD( "allg2.bin",   0x08000, 0x8000, CRC(021492a4) SHA1(b59e1303f17c9e5af05a808118ae729205690bb2) )
	ROM_LOAD( "allg3.bin",   0x10000, 0x8000, CRC(4eb29de3) SHA1(5c5d47959c06261ab51fc4e821bff0fe41ac9176) )
	ROM_LOAD( "allg4.bin",   0x18000, 0x8000, CRC(5c87177a) SHA1(a8a8318165008cb3295e25d4b4d38146f44a32fc) )
	ROM_LOAD( "allg5.bin",   0x20000, 0x8000, CRC(83056686) SHA1(00f14ded371751d54a391bf583d940b32ddeae58) )

	// 02 - Technik
	ROM_LOAD( "tech1.bin",   0x28000, 0x8000, CRC(cf5b9edc) SHA1(f1085c9915d21c4da581d06c9568d2bb47d467ed) )

	// 03 - Sport
	ROM_LOAD( "sp1.bin",     0x30000, 0x8000, CRC(20fb1261) SHA1(ad029f1a2b19365543c77fcd1103f6c5783c9f68) )
	ROM_LOAD( "sp2.bin",     0x38000, 0x8000, CRC(1988f5b7) SHA1(969ae64796e432c04862cb8c159e419a4793e9f0) )

	// 04 - Film - Unterhaltung
	ROM_LOAD( "unterh1.bin", 0x40000, 0x8000, CRC(996e7adc) SHA1(7b271cdb6ef555d31c2b17c6104bd30cd2eabcb3) )

	// 05 - Geographie
	ROM_LOAD( "geo1.bin",   0x48000, 0x8000, CRC(504da831) SHA1(4bef7bed4d300400c094cb30e9af55d3c6f47c29) )
	ROM_LOAD( "geo2.bin",   0x50000, 0x8000, CRC(7c563119) SHA1(9f3ae3ba3e4f60d9ea4b5c95aa5aaada8bb446a3) )
	ROM_LOAD( "geo3.bin",   0x58000, 0x8000, CRC(537ac6d2) SHA1(ed3eff9f7c71a6fe7ec59af2376a0b2ba17cc1c3) )

	// 06 - Pop
	ROM_LOAD( "pop1.bin",   0x60000, 0x8000, CRC(5c74781e) SHA1(0a50a706fd397bb220e31f1a7adaa4204b242888) )
	ROM_LOAD( "pop2.bin",   0x68000, 0x8000, CRC(10103648) SHA1(6fdc1aa4dcc8919e46def1c19adc2b9686c0f72d) )


	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "m3-7649-5.bin", 0x0000, 0x0200, CRC(f990a9ae) SHA1(f7133798b5f20dd5b8dbe5d1a6876341710d93a8) )
ROM_END

/*
  Fun World Quiz.
  27-04-1990

*/
ROM_START( funquizb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.bin",   0xc000, 0x4000, CRC(e3b1fa5d) SHA1(3479db70249fd37b6e932d563b41550f910721c8) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_FILL(            0x0000, 0x4000, 0xff)
	ROM_LOAD( "q3.c7",   0x4000, 0x4000, CRC(0dafa07a) SHA1(5d6fa842c617f92fad14a597396249d5a4d28c9a) )
	ROM_FILL(            0x8000, 0x4000, 0xff)
	ROM_LOAD( "q2.c6",   0xc000, 0x4000, CRC(ce07c6e1) SHA1(6b77a9198e29c195d983b856e8826e8174945321) )

	ROM_REGION( 0x100000, "questions", ROMREGION_ERASEFF )

	// 01 - Allgemein
	ROM_LOAD( "allg1.bin",  0x00000, 0x8000, CRC(1351cf56) SHA1(50e89c3e6d256bcf7f1d3c0dbef935e4e8561096) )
	ROM_LOAD( "allg2.bin",  0x08000, 0x8000, CRC(021492a4) SHA1(b59e1303f17c9e5af05a808118ae729205690bb2) )
	ROM_LOAD( "allg3.bin",  0x10000, 0x8000, CRC(de8e055f) SHA1(593fce143ee5994087bbac8b51ac7e2d02e8701c) )
	ROM_LOAD( "allg4.bin",  0x18000, 0x8000, CRC(5c87177a) SHA1(a8a8318165008cb3295e25d4b4d38146f44a32fc) )
	ROM_LOAD( "allg5.bin",  0x20000, 0x8000, CRC(83056686) SHA1(00f14ded371751d54a391bf583d940b32ddeae58) )

	// 02 - Technik
	ROM_LOAD( "tech1.bin",  0x28000, 0x8000, CRC(cf5b9edc) SHA1(f1085c9915d21c4da581d06c9568d2bb47d467ed) )

	// 03 - Sport
	ROM_LOAD( "sport1.bin", 0x30000, 0x8000, CRC(b351c745) SHA1(3b64d8e935eb4cd5bdf82934983b3ffab465257a) )
	ROM_LOAD( "sport2.bin", 0x38000, 0x8000, CRC(7accde63) SHA1(75ec3a02368d3a07d48ef9a9ff4ca7f8cf7798e2) )

	// 04 - Film - Unterhaltung
	ROM_LOAD( "unt1.bin",   0x40000, 0x8000, CRC(996e7adc) SHA1(7b271cdb6ef555d31c2b17c6104bd30cd2eabcb3) )

	// 05 - Geographie
	ROM_LOAD( "geo1.bin",   0x48000, 0x8000, CRC(504da831) SHA1(4bef7bed4d300400c094cb30e9af55d3c6f47c29) )
	ROM_LOAD( "geo2.bin",   0x50000, 0x8000, CRC(7c563119) SHA1(9f3ae3ba3e4f60d9ea4b5c95aa5aaada8bb446a3) )
	ROM_LOAD( "geo3.bin",   0x58000, 0x8000, CRC(537ac6d2) SHA1(ed3eff9f7c71a6fe7ec59af2376a0b2ba17cc1c3) )

	// 06 - Pop
	ROM_LOAD( "pop1.bin",   0x60000, 0x8000, CRC(5c74781e) SHA1(0a50a706fd397bb220e31f1a7adaa4204b242888) )
	ROM_LOAD( "pop2.bin",   0x68000, 0x8000, CRC(10103648) SHA1(6fdc1aa4dcc8919e46def1c19adc2b9686c0f72d) )


	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "m3-7649-5.bin", 0x0000, 0x0200, CRC(f990a9ae) SHA1(f7133798b5f20dd5b8dbe5d1a6876341710d93a8) )
ROM_END


/*
  Witch Royal
  by Video Klein

  Original Video Klein PCB
  with CPU box.

*/
ROM_START( witchryl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hn58c256.bin", 0x8000, 0x8000, CRC(b89a47b9) SHA1(11fcdd5756f2998748d2f1084b1db459e90126df) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u11", 0x0000, 0x8000, CRC(7edc8f44) SHA1(cabad613fa8a72dc12587d19a72bc9c6861486bd) )
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half
	ROM_LOAD( "3.u4",  0x8000, 0x8000, CRC(5e4a0d59) SHA1(08eb9b1a617a7b2e6f87377819dba07082cf38b4) )
	ROM_IGNORE(                0x8000 ) // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "witchryl_nvram.bin", 0x0000, 0x0800, CRC(98366bed) SHA1(279a5ce4639b8b2ac29146b32512615253c45991) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "27s29.u26", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END


/*
  Admiral Club Card (Novo Play)
  Novomatic, 1986.

  Hardware Fun World/Impera/TAB...
  Seems close to Royal Vegas Joker Card.

*/

ROM_START( novoplay )   // Similar to Royal Vegas Joker Card
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "np1_run.bin", 0x8000, 0x8000, CRC(4078d695) SHA1(d0e39064250733968044aec216040fe62fecc880) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "np1_ch2.bin", 0x0000, 0x8000, CRC(188d6fad) SHA1(3bc9bab24d8c7beed0c5f491c19a004ca7d719a1) )
	ROM_LOAD( "np1_ch1.bin", 0x8000, 0x8000, CRC(fdc3bd67) SHA1(0ec2d5e0b1937849934f98e253e18887af0331e8) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "novoplay_nvram.bin", 0x0000, 0x0800, CRC(92019972) SHA1(e6d1e231cd2ce27e718ed9482dbe9ddc8612eb67) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "np1_27s29.bin", 0x0000, 0x0200, CRC(8992aa4d) SHA1(5a0649bff66e7cab1bcbadcdfc74c77a747cc58f) )
ROM_END

/*
  Novo Play Club Card
  V6.2H

  Novo Play International.

  PCB from Novo Play, rev 1.6 (1992).
  Similar hardware scheme than Inter Games.
*/
ROM_START( novoplaya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "np_62h.bin", 0x8000, 0x8000, CRC(5b00faf0) SHA1(9144fef9c445f32cb681d7b935ead2032b40fb05) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "np4-ch2_9250.bin", 0x0000, 0x8000, CRC(7223471c) SHA1(785c6af6cf7e06978bc178a5c1c6b21f9db58a88) )
	ROM_LOAD( "np4-ch1_9250.bin", 0x8000, 0x8000, CRC(c57bac23) SHA1(5b329ece7a0682b8ef5f9a13abb0312f7c885339) )

	ROM_REGION( 0x1000, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "novoplaya_nvram.bin", 0x0000, 0x1000, CRC(d8e47867) SHA1(a05a6ac6d37d919cad57ef4df18e1b28bad5a907) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "n82s147an.bin", 0x0000, 0x0200, CRC(8992aa4d) SHA1(5a0649bff66e7cab1bcbadcdfc74c77a747cc58f) )
ROM_END

/*
  Novo Play Club Card
  V3.3H

  Novo Play International.

  PCB from Novo Play, rev 1.5 (1991).
  Similar hardware scheme than Inter Games.
*/
ROM_START( novoplayb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "np33.bin", 0x8000, 0x8000, CRC(a4c09120) SHA1(faa40920e84dab13a792f73aacf60ad3855d68ea) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "np4-ch2_9250.bin", 0x0000, 0x8000, CRC(7223471c) SHA1(785c6af6cf7e06978bc178a5c1c6b21f9db58a88) )
	ROM_LOAD( "np4-ch1_9250.bin", 0x8000, 0x8000, CRC(c57bac23) SHA1(5b329ece7a0682b8ef5f9a13abb0312f7c885339) )

	ROM_REGION( 0x0800, "ds1220", 0 )    // Dallas DS1220 to analize.
	ROM_LOAD( "ds1220.bin", 0x0000, 0x0800, CRC(f6c65329) SHA1(b64ca34661b9690aa9b69a20a7f6683954bbe76a) )

	ROM_REGION( 0x1000, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "novoplayb_nvram.bin", 0x0000, 0x1000, CRC(5dac2bdb) SHA1(1036dc9cd6602c2a8f7183a5f83c5dea56e2a503) )

	ROM_REGION( 0x0200, "proms", 0 )    // PLD addresses the 2nd half
	ROM_LOAD( "am27s29.bin", 0x0000, 0x0200, CRC(8992aa4d) SHA1(5a0649bff66e7cab1bcbadcdfc74c77a747cc58f) )
ROM_END


/*
  Jolly Card (spanish, encrypted)
  -------------------------------

  Rare unknown board with scratched chips.
  A daughterboard with a big chip (can barely read 'mexico')
  Other can barely read '68'

  3 roms.

  Graphics are encrypted 'alla blue TAB board.
  Program is encrypted through scrambling data lines.

*/

ROM_START( jolycdsp )   // Encrypted program in a module. Blue TAB PCB encrypted graphics
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ct3.bin", 0x8000, 0x8000, CRC(0c9cbae6) SHA1(4f834370229797cac302a5185ed1e77ef2b7cabb) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ct2.bin", 0x0000, 0x8000, CRC(7569e719) SHA1(f96e1e72bc13d1888f3868f8d404fd3db94db7b2) )
	ROM_LOAD( "ct1.bin", 0x8000, 0x8000, CRC(8f438635) SHA1(3200e20f4b28173cc2a68d0f87969627570418dc) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "ctunk_nvram.bin", 0x0000, 0x0800, CRC(c55c6706) SHA1(a38ae926f057fb47e48ca841b2d097fc4fd06416) )

	ROM_REGION( 0x0200, "proms", 0 )    // Borrowed from the parent set
	ROM_LOAD( "82s147.bin", 0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END


/*
  Unknown encrypted Royal Card
  Dino4 hardware with daughterboard (CPU+PLCC)

  1x Rockwell R65C02P4 CPU (11450-14 Mexico 9802 B54074-2)
  1x Unknown sanded 44 pin PLCC

  2x 68C21
  1x 6845
  1x YM2149F

  16 Mhz. xtal

  -----------------

  BP C17C (just after the CRTC init)

  Code checks offset $32F0 for 0xFE, and has two NOPs... Maybe was requested to continue.
  Same as China Town. WTH is mapped there?

*/

ROM_START( rcdino4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ryc_1a.u2",  0x0000, 0x10000, CRC(a22704d6) SHA1(0f5721a042d7ec8f4c9a93b38c2a9e3b2d115fc5) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "m27c512.u2",  0x0000, 0x8000, CRC(915f1e59) SHA1(2eb2a7acca50318eb1775b01a00b1d3c74e1522c) )
	ROM_IGNORE(                      0x8000 )   // Identical halves. Discarding 2nd half
	ROM_LOAD( "m27c512.u20", 0x8000, 0x8000, CRC(86e55f5a) SHA1(be71301b6887e8cc5924864d0f97b54e0668875e) )
	ROM_IGNORE(                      0x8000 )   // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "rcdino4_nvram.bin", 0x0000, 0x0800, CRC(a2dc069e) SHA1(23e296cc0cc0c6f0c34eccffff8c78d2cef2dbae) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25", 0x0000, 0x0200, CRC(649e6ccc) SHA1(674a5ea3b4b2e7de766e787debef5f695bff7a40) )
ROM_END

/*
  China Town
  Dino 4 encrypted hardware

  BP C17F
  CFD5...
  Checks offset $32F0 for 0xFE to continue. Otherwise got in a stuck loop.
  WTH is mapped there?

*/
ROM_START( chinatow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ctw-1b.u2",  0x0000, 0x10000, CRC(2cdeb1b6) SHA1(942a4f77b15b0e69de5414ce63c53f4f1ef6ba78) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "27c512.u2",  0x0000, 0x8000, CRC(6ace221f) SHA1(d35a6621d9d9231a844d7043da78035855ebf572) )
	ROM_CONTINUE(           0x0000, 0x8000) // Discarding 1nd half 0xff filled
	ROM_LOAD( "27c512.u20", 0x8000, 0x8000, CRC(efb7f1ec) SHA1(260005526fc9b4087ca03f7cc585e40b6fa007fb) )
	ROM_CONTINUE(           0x8000, 0x8000) // Discarding 1nd half 0xff filled

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "chinatow_nvram.bin", 0x0000, 0x0800, CRC(eef4c5e7) SHA1(a2d9a9f617d35ccb99236114e5ce3257ad572f49) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u25", 0x0000, 0x0200, CRC(e7e989be) SHA1(309a5cf169ef04c9daea48967f47c80fb29f2fcf) )
ROM_END


/*
  Joker Card from Inter Games (Austria).

  1x G65SC02P-2
  2x MC68B21P
  1x MC68B45P
  1x AY38910A/P
  3x NMC27C256Q
  1x Dallas DS1220Y
  1x AM27S29APC
  1x LM380N
  1x Crystal 16.0000 MHz.

  The hardware has way different architecture.

  Video RAM splitted in two well separated offsets:
  Video RAM at $2000 and Attr/Color Ram at $5000 mirrored at $7000.

  PIAs at $3200 and $3400.
  AY8910 at $3000-$3001.
  CRTC at... $3000-$3001 too!!!.

  There are multiple buffers and compare from some registers,
  reminding the way some hardwares handle inputs without PIAs.

  Some jumps to hardware errors were patched till we can understand
  a bit better the hardware behaviour.

  Bipolar PROM has the first half empty, so will black the screen
  after any attempt of ROM swap.

  A lot of reverse-engineering was needed to get this game working...
*/
ROM_START( intrgmes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bonus-off_5b87d19c.bin", 0x8000, 0x8000, CRC(03a4d1ef) SHA1(375ae1de5e6e4a7a6d6cedfd08902826fa62f93b) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ig2ch2.bin", 0x0000, 0x8000, CRC(52f0bc70) SHA1(cd9ba34efb438c9610551900c3de2a09aea76cb9) )
	ROM_LOAD( "ig2ch1.bin", 0x8000, 0x8000, CRC(8fc7d74e) SHA1(bf7ee7ef5c95877fe82fb6e04a5d8ab211fc730c) )

	ROM_REGION( 0x1000, "nvram", 0 )  // Default NVRAM.
	ROM_LOAD( "intrgmes_nvram.bin", 0x0000, 0x0800, CRC(7ce7c42c) SHA1(da5b2fbecd83ae87616f3c65fc0d6fd05f9fe7c5) )
	ROM_LOAD( "ds1220y.bin",        0x0800, 0x0800, CRC(7bc4554e) SHA1(c9ad1651e673f8edd0fd354b1098db8f27697d18) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29.bin", 0x0000, 0x0200, CRC(8992aa4d) SHA1(5a0649bff66e7cab1bcbadcdfc74c77a747cc58f) )
ROM_END


// http://www.citylan.it/wiki/images/6/69/1189_PCB_component_side.jpg
ROM_START( royalcrd_nes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "me2.bin", 0x0000, 0x10000, CRC(2144efab) SHA1(47fef8e0d2b05869e667731fdaf535722b9a8f19) ) // this is one (or more?) Royal Card programs, with (c)1991 TAB Austria in them

	ROM_REGION( 0x2000, "pic16c57", 0 )
	ROM_LOAD( "pic.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", 0 ) // these are (encrypted?) GFX ROMS
	ROM_LOAD( "me3.bin", 0x00000, 0x10000, CRC(398a6f7c) SHA1(126048c75ab5df421f9dd2e39c6b68e15bf54f38) )
	ROM_LOAD( "me4.bin", 0x10000, 0x10000, CRC(6f84222a) SHA1(eee8e46d464211914cb99d15fd9f7dd4aa009664) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "24c04.bin",    0x0000, 0x0200, CRC(e349687c) SHA1(1e6da87ba771baacc7687524158a1db8b789b673) )
	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s147.bin",    0x0000, 0x0200, CRC(5377c680) SHA1(33857bbbfebfce28b8a68c69e030bf560a701e83) )

	// and this is just an untouched NES multigame..
	ROM_REGION( 0x0100000, "nes_prg", 0 )
	ROM_LOAD( "me0.bin",    0x0000, 0x0100000, CRC(83a4e841) SHA1(280e1a26737fe0b90dd19be706df89e09ef84c77) ) // == 64 in 1 [a1][p1].prg  nes:mc_64a 64 in 1 (Alt Games)
	ROM_REGION( 0x80000, "nes_chr", 0 )
	ROM_LOAD( "me1.bin",    0x0000, 0x80000, CRC(7114b404) SHA1(0f5a206af25725b2e97c404a616e341f15925431) ) // == 64 in 1 [a1][p1].chr  nes:mc_64a 64 in 1 (Alt Games)
ROM_END

ROM_START( royalcrd_nesa ) // same as royalcrd_nes but for different main CPU ROM (mostly some routines appear to have been removed in royalcrd_nes but present here)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "me2.bin", 0x0000, 0x10000, CRC(04b16c53) SHA1(b0fcf8e2049b6065b95990c3a597166a83d7c7e6) ) // SLDH, this is one (or more?) Royal Card programs, with (c)1991 TAB Austria in them

	ROM_REGION( 0x2000, "pic16c55", 0 )
	ROM_LOAD( "pic.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", 0 ) // these are (encrypted?) GFX ROMS
	ROM_LOAD( "me3.bin", 0x00000, 0x10000, CRC(398a6f7c) SHA1(126048c75ab5df421f9dd2e39c6b68e15bf54f38) )
	ROM_LOAD( "me4.bin", 0x10000, 0x10000, CRC(6f84222a) SHA1(eee8e46d464211914cb99d15fd9f7dd4aa009664) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "24c04.bin",    0x0000, 0x0200, CRC(e349687c) SHA1(1e6da87ba771baacc7687524158a1db8b789b673) )
	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s147.bin",    0x0000, 0x0200, CRC(5377c680) SHA1(33857bbbfebfce28b8a68c69e030bf560a701e83) )

	// and this is just an untouched NES multigame..
	ROM_REGION( 0x0100000, "nes_prg", 0 )
	ROM_LOAD( "me0.bin",    0x0000, 0x0100000, CRC(83a4e841) SHA1(280e1a26737fe0b90dd19be706df89e09ef84c77) ) // == 64 in 1 [a1][p1].prg  nes:mc_64a 64 in 1 (Alt Games)
	ROM_REGION( 0x80000, "nes_chr", 0 )
	ROM_LOAD( "me1.bin",    0x0000, 0x80000, CRC(7114b404) SHA1(0f5a206af25725b2e97c404a616e341f15925431) ) // == 64 in 1 [a1][p1].chr  nes:mc_64a 64 in 1 (Alt Games)
ROM_END

// http://www.citylan.it/wiki/images/5/54/1188_PCB_component_side.jpg
ROM_START( royalcrd_msx )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "me2.bin", 0x0000, 0x20000, CRC(4968fe51) SHA1(ae120f40c97e099c3f65b802c51838673958e87e) ) // this is one (or more?) Royal Card programs, with (c)1991 TAB Austria in them (2nd half is empty)

	ROM_REGION( 0x20000, "gfx1", 0 ) // these are (encrypted?) GFX ROMS
	ROM_LOAD( "me3.bin", 0x00000, 0x10000, CRC(398a6f7c) SHA1(126048c75ab5df421f9dd2e39c6b68e15bf54f38) )
	ROM_LOAD( "me4.bin", 0x10000, 0x10000, CRC(b536d59c) SHA1(8050f87c3352d7501611c49e83e34e800f28c9a7) ) // BADADDR        xxxxxx-xxxxxxxxx  , should probably just match above set

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "24c04.bin",    0x0000, 0x0200, CRC(2a1e8abe) SHA1(639a704f25af02597ead2e69d15ac47953c27f09) )

	// some kind of MSX multigame?
	ROM_REGION( 0x0100000, "msx_prg", 0 )
	ROM_LOAD( "me0.bin",    0x0000, 0x10000, CRC(bd5be18b) SHA1(b43a176db0522bcda0a17dd0c210c987dc380c97) ) // weird pattern? might be bad if not a lookup table?  BADADDR        -------xxxxxxxxx
	ROM_LOAD( "me1.bin",    0x0000, 0x40000, CRC(2152c6b7) SHA1(e512e29f4a899cc3f91a446141fd4432a487228f) )
ROM_END


/*
  Unknown Funworld A7-11 game

  CPU epoxy brick marked "Funworld A7-11", with:
  1x RP65C02A
  2x Toshiba TC5565PL1-12
  1x Toshiba TMM24128AP
  1x PAL16L8B-2CN
  3x TTL's...

  PCB components:

  2x Hitachi HD68B21P
  1x Hitachi HD46505SP-2
  1x GI AY-3-8910A

  1x Toshiba TC5565APL-10
  1x Hitachi HD6116LP-3
  2x Toshiba TMM27256AD-20
  1x Harris M3-7649A-5 Bipolar PROM.

  1x Xtal 16.000 MHz.
  1x 3.6V lithium battery.
  1x 8 DIP switches bank.

*/
ROM_START( fw_a7_11 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tmm24128ap.bin", 0xc000, 0x4000, CRC(67ac1337) SHA1(896bd30d4b061f08b8eeeb36aaceb7859f342eaf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "e-3.bin", 0x0000, 0x8000, CRC(d34f0e63) SHA1(5984248331ec632b665f60ad553b74916630d4d3) )
	ROM_LOAD( "e-2.bin", 0x8000, 0x8000, CRC(16aebfb3) SHA1(3a2c587b2c31341a83ec9e0f9dd188d4bb63e764) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "m3-7649a-5.bin", 0x0000, 0x0200, CRC(f990a9ae) SHA1(f7133798b5f20dd5b8dbe5d1a6876341710d93a8) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16l8b-2cn_block.bin", 0x0000, 0x0117, NO_DUMP )
ROM_END

ROM_START( fw_a7_11a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tmm24128ap.bin", 0xc000, 0x4000, CRC(28126b0f) SHA1(e2baa643c1858cd3ea53abf801fc9ab1a1e3ff9b) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(d34f0e63) SHA1(5984248331ec632b665f60ad553b74916630d4d3) )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(16aebfb3) SHA1(3a2c587b2c31341a83ec9e0f9dd188d4bb63e764) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "m3-7649a-5.bin", 0x0000, 0x0200, CRC(f990a9ae) SHA1(f7133798b5f20dd5b8dbe5d1a6876341710d93a8) )

	ROM_REGION( 0x0200, "plds", 0 ) // cracked PLD
	ROM_LOAD( "pal16l8acn_block.bin", 0x0000, 0x0117, CRC(fcda7872) SHA1(60acdb968e6229a8f71c2e29d22e132906a65bd5) )
ROM_END

/*
  Unknown Funworld A0-1 game
  CPU epoxy brick marked "Funworld A0-1".
*/
ROM_START( fw_a0_1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d27128.bin", 0xc000, 0x4000, CRC(67ac1337) SHA1(896bd30d4b061f08b8eeeb36aaceb7859f342eaf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "10.bin", 0x0000, 0x8000, CRC(d34f0e63) SHA1(5984248331ec632b665f60ad553b74916630d4d3) )
	ROM_LOAD( "9.bin",  0x8000, 0x8000, CRC(16aebfb3) SHA1(3a2c587b2c31341a83ec9e0f9dd188d4bb63e764) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147n.bin", 0x0000, 0x0200, CRC(f990a9ae) SHA1(f7133798b5f20dd5b8dbe5d1a6876341710d93a8) )
ROM_END

/*
  Joker Card / Multi Card
  Funworld.

  This game is running in a derivative hardware with epoxy CPU brick.
  CPU is a 65C02, with two 5565 RAMs and one 27128 ROM.
  Also there are present some PLD and logic. (see above).

  The main program is inside the battery backed RAM, and need to be dumped.
  These RAMs are connected to the mainboard battery, so once the Brick is unplugged,
  the battery stops to feed the RAM and the program just vanish.

  From factory, they used the central connector to plug a module with the game program,
  and then the code inside the ROM check for offset $4000 onward if the module is
  present through some sort of checksum. If the module is present, the code will
  copy the range 4000-7fff (the game program inside the external module) to the
  8000-bfff location, where is the battery backed RAM inside the brick.

  We need to find a new working board and reverse the central connector, to construct
  some interface that allows us to dump the content of the internal battery backed RAM.

  For debug purposes, we plugged an external game code from another similar game, that
  matches all calls from the original ROM code. We suspect is the same code with different
  language (English instead German). With this external code the game is completely playable.

  ----

  CPU epoxy brick marked "76AF".

  Epoxy Brick contents:

  - CPU: 1x Ricoh RP65C02A.
  - RAM: 2x Toshiba TC5565APL-10.
  - ROM: 1x Intel D27128A.
  - PLD: 1x PAL16L8B.

  - LOGIC: 1x HD74HC241P.
           1x M74LS245P.
           1x SN74LS139N.

*/
ROM_START( jokcrdep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e-1.bin", 0xc000, 0x4000, CRC(67ac1337) SHA1(896bd30d4b061f08b8eeeb36aaceb7859f342eaf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "200_zg_2.bin", 0x0000, 0x8000, CRC(ba994fc3) SHA1(95d2a802c38d7249f10eb2bbe46edfb9b14b6faa) )
	ROM_LOAD( "200_zg_1.bin", 0x8000, 0x8000, CRC(367db105) SHA1(400b82dc9e0be4c17a02add009aab3c43dd901f8) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "joker_nvram.bin", 0x0000, 0x0800, CRC(92019972) SHA1(E6D1E231CD2CE27E718ED9482DBE9DDC8612EB67) )     // Default NVRAM.

	ROM_REGION( 0x4000, "nvram1", 0 )
	ROM_LOAD( "e-0.bin", 0x0000, 0x4000, BAD_DUMP CRC(8FD42F3B) SHA1(208209761DE046189070B88AD4340C8D7FF55F1F) )    // Internal suicide program. Taken from Novo Play.

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "sn82s147n.bin", 0x0000, 0x0200, CRC(f990a9ae) SHA1(f7133798b5f20dd5b8dbe5d1a6876341710d93a8) )
ROM_END


/*
  Gratis Poker
  Stmk
  Version: 004
  29. Sept.  95

*/
ROM_START( gratispk )   // Gratis Poker (V.204, Set 1)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gratis_poker_club_ver_204.bin", 0x8000, 0x8000, CRC(e53da78e) SHA1(6b87af4d66082f1a18e9e00a41df2484f5373cc7) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.bin", 0x0000, 0x8000, CRC(daec20c7) SHA1(340bdb7749a7d898e96db5a470821cfe7216ea57) )
	ROM_IGNORE(                 0x8000 )    // Identical halves. Discarding 2nd half
	ROM_LOAD( "2.bin", 0x8000, 0x8000, CRC(112c5f33) SHA1(fffaa247a97f9bde7374cc80ab0bb9fb71bcc4b6) )
	ROM_IGNORE(                 0x8000 )    // Identical halves. Discarding 2nd half

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "gratispk_nvram.bin", 0x0000, 0x0800, CRC(8fcad62f) SHA1(85def713ed5b9bbdbf0d077a38f779d6f4329a1c) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp28s42n.bin", 0x0000, 0x0200, CRC(e92f74e0) SHA1(dfc4a9d140d21b990f769c10802c4d2c33dd4132) )
ROM_END

ROM_START( gratispka )  // Gratis Poker (V.204, Set 2)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "35g_ver_204.bin", 0x8000, 0x8000, CRC(e53da78e) SHA1(6b87af4d66082f1a18e9e00a41df2484f5373cc7) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.bin", 0x0000, 0x8000, CRC(4f077fa3) SHA1(d65005b7111b77ac7c3707bd12bc376506d9477b) )
	ROM_LOAD( "2.bin", 0x8000, 0x8000, CRC(0f3711b0) SHA1(ce874dbfc2d7171cff2be41d4a44fe8194526ef7) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "gratispk_nvram.bin", 0x0000, 0x0800, CRC(8fcad62f) SHA1(85def713ed5b9bbdbf0d077a38f779d6f4329a1c) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp28s42n.bin", 0x0000, 0x0200, CRC(e92f74e0) SHA1(dfc4a9d140d21b990f769c10802c4d2c33dd4132) )
ROM_END


/*
  Γκούλιτ
  Nkoúlit
  Gullit

  Greek hack of Royal Card.

*/
ROM_START( nkoulit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c256-3.bin", 0x8000, 0x8000, CRC(3f4f671d) SHA1(0aa87f813567f7a537315bf3d452343e5a449e13) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "27c256-2.bin", 0x0000, 0x8000, CRC(4a9ea5ad) SHA1(50504544265646e12ce769a1fd40d7afeae8b8d3) )
	ROM_LOAD( "27c256-1.bin", 0x8000, 0x8000, CRC(7b3a6d1a) SHA1(b7f2e534ad4467cd0a9d86019be8110caf45c024) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "nkoulit_nvram.bin", 0x0000, 0x0800, CRC(73584f67) SHA1(4aab0d4fe846592e1cbc8b3c2411ddc1bf79fd95) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.bin", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END


/*
  Reflex Cards (English)
  Need special NVRAM to boot.

  Without the factory NVRAM, you can generate one.
  The attract works, and allow coin up. When you start a game,
  a serial number entering screen appears. You must to match
  the secret number to continue:

    Serial: 00000000
       Key: FFFFFFFF

  The real serial number isn't shown. Just zeroes as mask.
  You must to enter digit by digit using the HOLD buttons,
  then turn the machine off, and on. (you can reset in MAME).

  If you entered the correct number, the game continues,
  otherwise you'll be stuck forever.

  The code and NVRAM were reversed to find the serial number
  and key offsets and the way they are stored, to construct
  a working default NVRAM.

*/
ROM_START( reflexcrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bonus_2_27c256-1.bin", 0x8000, 0x8000, CRC(8fbcf0b9) SHA1(3a45c09fbec3e4ecce24ac2562a68b453755886f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "27c256-2.bin", 0x0000, 0x8000, CRC(05fe86fc) SHA1(a58ab5d107936f7d462f8f748bb6ac27bd77429b) )
	ROM_LOAD( "27c256-3.bin", 0x8000, 0x8000, CRC(1617ed13) SHA1(312065406ce1d1d7c4ad566e2b13268f7eb524bb) )

	ROM_REGION( 0x0800, "nvram", 0 )    // default NVRAM
	ROM_LOAD( "reflexcrd_nvram.bin", 0x0000, 0x0800, CRC(c2c69e5b) SHA1(e4abbf09abc044ba17e542cc8e1cfa16ee7cf776) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "bipolar_prom.bin", 0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END


/**************************
*  Driver Initialization  *
**************************/

void funworld_state::init_tabblue()
{
/****************************************************************************************************

   +-------------------------+
   | Blue TAB PCB Decryption |
   +-------------------------+

    It performs by byte nibble a boolean XOR against the same value shifted to the right, then shift
    the result to the left, carring the less significant bit and losing the most significant one.


    Encrypted nibble:  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    Bits:             0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111
    -------------------------------------------------------------------------------------------------
    Bits:             0000 0011 0110 0101 1100 1111 1010 1001 1000 1011 1110 1101 0100 0111 0010 0001
    Decrypted nibble:  0    3    6    5    C    F    A    9    8    B    E    D    4    7    2    1


*****************************************************************************************************/

	int x, na, nb, nad, nbd;
	uint8_t *src = memregion( "gfx1" )->base();


	for (x=0x0000; x < 0x10000; x++)
	{
		na = src[x] & 0xf0;     // nibble A
		nb = src[x] << 4;       // nibble B

			nad = (na ^ (na >> 1)) << 1;            // nibble A decrypted
			nbd = ((nb ^ (nb >> 1)) >> 3) & 0x0f;   // nibble B decrypted

		src[x] = nad + nbd;     // decrypted byte
	}
}


void magicrd2_state::init_magicd2b()
{
	/*****************************************************************

	  For a serie of Mexican Rockwell's 65c02
	  seems that opcode 0x91 is STA ($zp) instead of STA ($zp),y
	  ...or is patched with the correct opcode (0x92) by PLDs...

	  In offset $C1C4, the code is trying to initialize both PIAs
	  putting value 0x34 in $0800-$0803 & $0A00-$0A03.

	  The code use STA ($zp),y (opcode 0x91). As soon as register 'y'
	  increments, almost all writes go out of range.

	******************************************************************/
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0xc1c6] = 0x92;
}


void magicrd2_state::init_magicd2c()
{
//  *** same as blue TAB PCB, with the magicd2a patch ***
	int x, na, nb, nad, nbd;
	uint8_t *src = memregion( "gfx1" )->base();
	uint8_t *ROM = memregion("maincpu")->base();

	for (x=0x0000; x < 0x10000; x++)
	{
		na = src[x] & 0xf0;     // nibble A
		nb = src[x] << 4;       // nibble B

			nad = (na ^ (na >> 1)) << 1;            // nibble A decrypted
			nbd = ((nb ^ (nb >> 1)) >> 3) & 0x0f;   // nibble B decrypted

		src[x] = nad + nbd;     // decrypted byte
	}

	ROM[0xc1c6] = 0x92;
}


void funworld_state::init_mongolnw()
{
//  temporary patch to avoid hardware errors for debug purposes
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x9115] = 0xa5;

//  prevents one test from triggering hardware error
	ROM[0xb8f3] = 0xff;
}


void funworld_state::init_soccernw()
{
//  temporary patch to avoid hardware errors for debug purposes
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x80b2] = 0xa9;
	ROM[0x80b3] = 0x00;
	ROM[0x9115] = 0xa5;

//  prevents one test from triggering hardware error
	ROM[0xb8f3] = 0xff;
}


void funworld_state::init_saloon()
/*************************************************

    LEOPARDO 5 Hardware
    -------------------

    Special thanks to Andreas Naive, that
    figured out the address encryption.

    Program:
    Low 8 bits of address are scrambled.
    Also the values have bits 0 & 2 bitswapped.

    GFX:
    Low 11 bits of address are scrambled.

    Color:
    Data has bits 2-3-5-6-7 scrambled.
    Address has the 8 lower bits scrambled.

*************************************************/
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0x8000;

	uint8_t *gfxrom = memregion("gfx1")->base();
	int sizeg = memregion("gfx1")->bytes();

	uint8_t *prom = memregion("proms")->base();
	int sizep = memregion("proms")->bytes();

	popmessage(" Please: Start with <2> and <Z> buttons pressed to run system properly");


	/*****************************
	*   Program ROM decryption   *
	*****************************/

	// data lines swap: 76543210 -> 76543012

	for (int i = start; i < size; i++)
	{
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 0, 1, 2);
	}

	{
		std::vector<uint8_t> buffer(size);
		memcpy(&buffer[0], rom, size);

		// address lines swap: fedcba9876543210 -> fedcba9820134567

		for (int i = start; i < size; i++)
		{
			int a = ((i & 0xff00) | bitswap<8>(i & 0xff, 2, 0, 1, 3, 4, 5, 6, 7));
			rom[a] = buffer[i];
		}

	}


	/******************************
	*   Graphics ROM decryption   *
	******************************/

	{
		std::vector<uint8_t> buffer(sizeg);
		memcpy(&buffer[0], gfxrom, sizeg);

		// address lines swap: fedcba9876543210 -> fedcb67584a39012

		for (int i = 0; i < sizeg; i++)
		{
			int a = bitswap<16>(i, 15, 14, 13, 12, 11, 6, 7, 5, 8, 4, 10, 3, 9, 0, 1, 2);
			gfxrom[a] = buffer[i];
		}
	}


	/****************************
	*   Color PROM decryption   *
	****************************/

	// data lines swap: 76543210 -> 23546710

	{
		std::vector<uint8_t> buffer;
		for (int i = 0; i < sizep; i++)
		{
			buffer.push_back(bitswap<8>(prom[i], 2, 3, 5, 4, 6, 7, 1, 0));
		}

		// address lines swap: fedcba9876543210 -> fedcba9487652013
		for (int i = 0; i < sizep; i++)
		{
			int a = bitswap<16>(i, 15, 14, 13, 12, 11, 10, 9, 4, 8, 7, 6, 5, 2, 0, 1, 3);
			prom[a] = buffer[i];
		}
	}

	m_palette->update();


	// inversion for i2c clock generation in i2c_rx routine
	rom[0xbf23] = 0x60; // ex 70h
	rom[0xbf2a] = 0x70; // ex 60h
}


void funworld_state::init_dino4()
/*****************************************************

  DINO 4 hardware.

  Program ROM data & address lines are swapped,
  hardcoded in the board.

  GFX ROMs address lines are also swapped, but they
  are connected to 2 PLDs that handle the encryption.

  Color PROM is straight.

  All PLD's are read protected.

******************************************************/
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0x8000;

	uint8_t *gfxrom = memregion("gfx1")->base();
	int sizeg = memregion("gfx1")->bytes();

	/*****************************
	*   Program ROM decryption   *
	*****************************/

	// data lines swap: 76543210 -> 76543120

	for (int i = start; i < size; i++)
	{
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 1, 2, 0);
	}

	{
		std::vector<uint8_t> buffer(size);
		memcpy(&buffer[0], rom, size);

		// address lines swap: fedcba9876543210 -> fedcba9867543210
		for (int i = start; i < size; i++)
		{
			int a = bitswap<16>(i, 15, 13, 14, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
			rom[a] = buffer[i];
		}
	}


	/******************************
	*   Graphics ROM decryption   *
	******************************/

	{
		std::vector<uint8_t> buffer(sizeg);
		memcpy(&buffer[0], gfxrom, sizeg);

		// address lines swap: fedcba9876543210 -> fedcb67584a39012

		for (int i = 0; i < sizeg; i++)
		{
			int a = bitswap<16>(i, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 4, 5, 3, 2, 1, 0);
			gfxrom[a] = buffer[i];
		}
	}
}


void funworld_state::init_ctunk()
/*********************************************************

  CTUNK: Rare board with blue TAB board encryption scheme
         plus a daughterboard for program encryption.

*********************************************************/
{
	uint8_t *rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	int start = 0x8000;

	/*****************************
	*   Program ROM decryption   *
	*****************************/

	// data lines swap: 76543210 -> 56734012

	for (int i = start; i < size; i++)
	{
		rom[i] = bitswap<8>(rom[i], 5, 6, 7, 3, 4, 0, 1, 2);
	}

	//buffer = std::make_unique<uint8_t[]>(size);
	//memcpy(buffer, rom, size);


	/*****************************
	*  Graphics ROMs decryption  *
	*****************************/

	uint8_t *src = memregion( "gfx1" )->base();
	//uint8_t *ROM = memregion("maincpu")->base();
	for (int x = 0x0000; x < 0x10000; x++)
	{
		int na = src[x] & 0xf0;     // nibble A
		int nb = src[x] << 4;       // nibble B

		int nad = (na ^ (na >> 1)) << 1;            // nibble A decrypted
		int nbd = ((nb ^ (nb >> 1)) >> 3) & 0x0f;   // nibble B decrypted

		src[x] = nad + nbd;         // decrypted byte
	}
}


static void decrypt_rcdino4(uint8_t *rom, int size, uint8_t *gfxrom, int sizeg, uint8_t *src)
{
	/*****************************
	*   Program ROM decryption   *
	*****************************/

	// data lines swap: 76543210 -> 76543120
	{
		std::vector<uint8_t> buffer;
		for (int i = 0; i < size; i++)
		{
			buffer.push_back(bitswap<8>(rom[i], 7, 6, 5, 4, 3, 1, 2, 0));
		}

		// address lines swap: fedcba9876543210 -> fedcba9867543210

		for (int i = 0; i < size; i++)
		{
			int a = bitswap<16>(i, 15, 13, 14, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
			rom[a] = buffer[i];
		}
	}

	/******************************
	*   Graphics ROM decryption   *
	******************************/

	{
		std::vector<uint8_t> buffer(sizeg);
		memcpy(&buffer[0], gfxrom, sizeg);

		// address lines swap: fedcba9876543210 -> fedcb67584a39012

		for (int i = 0; i < sizeg; i++)
		{
			int a = bitswap<16>(i, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 4, 5, 3, 2, 1, 0);
			gfxrom[a] = buffer[i];
		}
	}

	// d4-d5 data lines swap, plus a XOR with 0x81, implemented in two steps for an easy view

	for (int x = 0x0000; x < 0x10000; x++)
	{
		src[x] = bitswap<8>(src[x], 7, 6, 4, 5, 3, 2, 1, 0);
		src[x] = src[x] ^ 0x81;
	}
}



static uint8_t rcdino4_add[] =
{
//      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
/*0*/   1, 9, 9, 9, 9, 2, 9, 9, 1, 2, 1, 9, 9, 3, 9, 9,
/*1*/   2, 2, 9, 9, 9, 9, 9, 9, 1, 9, 1, 9, 9, 9, 9, 9,
/*2*/   3, 9, 9, 9, 9, 2, 2, 9, 1, 2, 1, 9, 9, 3, 3, 9,
/*3*/   2, 9, 9, 9, 9, 9, 9, 9, 1, 9, 1, 9, 9, 3, 9, 9,
/*4*/   1, 9, 9, 9, 9, 2, 9, 9, 1, 2, 1, 9, 3, 3, 9, 9,
/*5*/   9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 1, 9, 9, 9, 9, 9,
/*6*/   1, 9, 9, 9, 2, 2, 9, 9, 1, 2, 1, 9, 3, 3, 3, 9,
/*7*/   9, 2, 9, 9, 9, 9, 9, 9, 9, 3, 1, 9, 9, 3, 9, 9,
/*8*/   2, 9, 9, 9, 2, 2, 2, 9, 1, 9, 1, 9, 9, 3, 3, 9,
/*9*/   2, 2, 9, 9, 9, 2, 9, 9, 1, 3, 9, 9, 3, 3, 3, 9,
/*a*/   2, 9, 2, 9, 2, 2, 2, 9, 1, 2, 1, 9, 9, 3, 3, 2,
/*b*/   2, 2, 9, 9, 9, 2, 9, 9, 9, 3, 9, 9, 9, 3, 9, 9,
/*c*/   2, 9, 9, 9, 2, 2, 2, 9, 1, 2, 1, 9, 3, 3, 3, 9,
/*d*/   2, 9, 9, 9, 9, 9, 9, 9, 1, 9, 1, 9, 9, 3, 9, 9,
/*e*/   2, 9, 9, 9, 2, 9, 2, 9, 1, 2, 1, 9, 3, 9, 3, 9,
/*f*/   2, 9, 9, 9, 9, 2, 9, 9, 1, 3, 1, 9, 9, 3, 9, 9
};

static uint8_t rcdino4_keys40[] =
{
//   40    41    42    43    44    45    46    47    48    49    4a    4b    4c    4d
	0x36, 0x54, 0x47, 0x6b, 0xce, 0x95, 0xa2, 0x66, 0x3a, 0x46, 0x53, 0xd7, 0xc4, 0xa4,
//   4e    4f
	0x00, 0x00,
//   50
	0x56
};

static uint8_t rcdino4_keys80[] =
{
//   81    82    83    84    85
	0xb8, 0x32, 0x1c, 0x23, 0xe2,
//   86    87    88    89    8a    8b    8c    8d    8e    8f    90    91
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//   92    93    94    95    96    97
	0x4c, 0x00, 0x3d, 0x00, 0xd9, 0x16,
//   98    99    9a    9b    9c    9d    9e    9f    a0    a1    a2    a3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//   a4    a5    a6
	0x5e, 0x73, 0x69,
//   a7    a8    a9
	0x00, 0x00, 0x00,
//   aa
	0xa6,
//   ab    ac    ad    ae    af
	0x00, 0x00, 0x00, 0x00, 0x00,
//   b0    b1
	0xc3, 0x40,
//   b2    b3    b4    b5    b6    b7    b8    b9    ba    bb    bc    bd    be    bf    c0    c1    c2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//   c3    c4    c5
	0x92, 0xb7, 0x24,
//   c6    c7    c8    c9
	0x00, 0x00, 0x00, 0x00,
//   ca
	0x62,
//   cb    cc    cd    ce    cf
	0x00, 0x00, 0x00, 0x00, 0x00,
//   d0
	0x84,
//   d1
	0x00,
//   d2
	0xaa,
//   d3    d4    d5
	0x00, 0x00, 0x00,
//   d6
	0xea,
//   d7    d8    d9    da    db    dc    dd    de    df
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//   e0
	0x17,
//   e1    e2    e3
	0x00, 0x00, 0x00,
//   e4
	0xc0,
//   e5    e6    e7    e8    e9    ea    eb    ec    ed    ee    ef    f0    f1    f2    f3    f4    f5    f6    f7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//   f8    f9    fa    fb    fc    fd    fe
	0x06, 0x1e, 0x28, 0x5a, 0xcf, 0x79, 0x11
};

void chinatow_state::init_rcdino4()
{
	/*****************************************************

	  Dino4 hardware with CPU+PLCC daughterboard

	  Program ROM data & address lines are swapped.
	  GFX ROMs address lines are swapped and data encrypted.

	  Color PROM seems straight.

	******************************************************/
	uint8_t *rom = memregion("maincpu")->base();

	decrypt_rcdino4(rom, memregion("maincpu")->bytes(), memregion("gfx1")->base(), memregion("gfx1")->bytes(), memregion( "gfx1" )->base());

	int j = 0;
	for (int i = 0x40; i < (0x40 + std::size(rcdino4_keys40));)
	{
		uint8_t key = rcdino4_keys40[i - 0x40];

		do
		{
			uint8_t c = rom[(i << 8) + j] ^ key;
			int add = rcdino4_add[c];

			if (add == 9)
			{
				++j;
			}
			else
			{
				rom[(i << 8) + j] = c;
				j += add;
			}
		}

		while (j < 0x100);

		j &= 0xff;
		++i;
	}

	j = 1;
	int i = 0x81;

	do
	{
		uint8_t key = rcdino4_keys80[i - 0x81];

		do
		{
			uint8_t c = rom[(i << 8) + j] ^ key;
			int add = rcdino4_add[c];

			if (((i == 0x81)
				&& (j >= 0xa3) && (j <= 0xb1)   // text string
				)
				|| ((i == 0x82)
				&& (j >= 0x35) && (j <= 0x53)   // table of addresses
				)
				|| ((i == 0x85)
				&& (j >= 0x7e) && (j <= 0x8d)   // '0'-'9', 'A'-'F'
				)
				|| ((i == 0x94)
				&& (((j >= 0x4a) && (j <= 0x86))    // zeroes
				|| ((j >= 0xbf) && (j <= 0xc1))     // set of masks */
					)
				)
				|| ((i == 0x96)
				&& (j >= 0x39) && (j <= 0x3e)   // set of masks
				)
				|| ((i == 0xa6)
				&& (j >= 0x30) && (j <= 0x32)   // set of masks
				)
				|| ((i == 0xaa)
				&& (j >= 0xf2)  // table of addresses
				)
				|| ((i == 0xc3)
				&& (j >= 0x70) && (j <= 0xaf)   // set of masks
				)
				|| ((i == 0xc4)
				&& (j >= 0xdc)  // zeroes and things
				)
				|| ((i == 0xd0)
				&& (j >= 0xd2)  // text and zeroes
				)
				|| ((i == 0xd2)
				&& ((j <= 0x2f) // text and zeroes
				|| ((j >= 0x84) && (j <= 0xaf)) // text and zeroes
					)
				)
				|| (add == 9)
				)
			{
				++j;
			}
			else
			{
				rom[(i << 8) + j] = c;
				j += add;
			}
		}
		while (j < 0x100);

		j &= 0xff;

		do {} while (((++i - 0x81) < std::size(rcdino4_keys80))
				&& !rcdino4_keys80[i - 0x81]);

		if ((i - 0x81) == std::size(rcdino4_keys80))
		{
			break;
		}

		if ((i == 0xa4)
			|| (i == 0xb0)
			|| (i == 0xf8)
			)
		{
			j = 0;  // re-align offset after skipping some pages
		}
	}
	while (1);
}

void chinatow_state::init_rcdinch()
{
	/*****************************************************

	  Dino4 hardware with CPU+PLCC daughterboard

	  Less encryption than rc4dino

	******************************************************/
	decrypt_rcdino4(memregion("maincpu")->base(), memregion("maincpu")->bytes(), memregion("gfx1")->base(), memregion("gfx1")->bytes(), memregion( "gfx1" )->base());
}


void funworld_state::init_jolycdig()
{
	// covering two values in ROM space checked for hardware errors.
	// maybe some virtual or physically mapped there.

	uint8_t *rom = memregion("maincpu")->base();

	rom[0xe825] = 0xff;  // checks for #$45, ascii E.
	rom[0xe826] = 0xff;  // checks for #$56, ascii V.
}

void intergames_state::init_novop_a()
{
	// NOP'ing some values in ROM space to avoid the hardware error.

	uint8_t *rom = memregion("maincpu")->base();

	rom[0xb25e] = 0xfc;
	rom[0xadd0] = 0x40;
}

void intergames_state::init_novop_b()
{
	// NOP'ing some values in ROM space to avoid the hardware error.

	uint8_t *rom = memregion("maincpu")->base();

	rom[0xb2bf] = 0xfc;
	rom[0xae31] = 0x40;
}

void intergames_state::init_intgms()
{
	// NOP'ing some values in ROM space to avoid the hardware error.

	uint8_t *rom = memregion("maincpu")->base();

	rom[0xadc6] = 0xea;

	rom[0xadd2] = 0xea;
	rom[0xadd3] = 0xea;

	rom[0xade6] = 0xea;
	rom[0xade7] = 0xea;

	rom[0xadf8] = 0xea;
	rom[0xadf9] = 0xea;

	rom[0xae16] = 0xea;
	rom[0xae17] = 0xea;

	rom[0xaead] = 0xea;
	rom[0xaeae] = 0xea;
	rom[0xaeaf] = 0xea;

//  verify checksum against values stored at 07F2-07F3.
//  rom[0xaf69] = 0xea;
//  rom[0xaf6a] = 0xea;
//  rom[0xaf6b] = 0xea;
}


void funworld_state::init_impera16()
/*****************************************************

  Routines to deinterleave the Impera GFX stored
  in 16 bit EPROMS, on 8 bit systems.

  Regular GFX ROMs for this system, are interleaved
  in a big 27C210 ROM.

******************************************************/
{
	uint8_t *gfx16rom = memregion("gfxpool")->base();
	int size = memregion("gfxpool")->bytes();

	uint8_t *gfx8rom = memregion("gfx1")->base();
	int sizeg = memregion("gfx1")->bytes();

	/*****************************
	*    Deinterleaving GFX      *
	*****************************/

	int j = 0;

	for (int i = 0; i < size; i += 2)
	{
		gfx8rom[j] = gfx16rom[i];
		gfx8rom[j + (sizeg / 2)] = gfx16rom[i + 1];
		j++;
	}
}


/**********************************************
*                Game Drivers                 *
**********************************************/

//     YEAR  NAME        PARENT    MACHINE   INPUT      STATE           INIT           ROT    COMPANY              FULLNAME                                          FLAGS                    LAYOUT

// Jolly Card based...
GAMEL( 1985, jollycrd,   0,        fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Jolly Card (German, set 1)",                      0,                       layout_jollycrd )
GAMEL( 1985, jollycrda,  jollycrd, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Jolly Card (German, set 2)",                      0,                       layout_jollycrd )
GAMEL( 1985, jolyc3x3,   jollycrd, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Jolly Card (3x3 deal)",                           0,                       layout_jollycrd )
GAMEL( 1993, jolycmzs,   jollycrd, cuoreuno, jolyc980,  funworld_state, empty_init,    ROT0, "MZS Tech",          "Jolly Card Professional 2.0 (MZS Tech)",          0,                       layout_jollycrd )
GAMEL( 2000, jolyc980,   jollycrd, cuoreuno, jolyc980,  funworld_state, empty_init,    ROT0, "Spale Soft",        "Jolly Card Professional 2.0 (Spale Soft)",        0,                       layout_jollycrd )
GAMEL( 1998, jolycdev,   jollycrd, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "TAB Austria / Evona Electronic", "Jolly Card (Evona Electronic)",      0,                       layout_jollycrd )
GAMEL( 1985, jolyccra,   jollycrd, cuoreuno, jolycdcr,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Jolly Card (Croatian, set 1)",                    0,                       layout_jollycrd )
GAMEL( 1993, jolyccrb,   jollycrd, cuoreuno, jolycdcr,  funworld_state, empty_init,    ROT0, "Soft Design",       "Jolly Card (Croatian, set 2)",                    0,                       layout_jollycrd )
GAMEL( 1985, sjcd2kx3,   jollycrd, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "M.P.",              "Super Joly 2000 - 3x",                            0,                       layout_jollycrd )
GAMEL( 1986, jolycdab,   jollycrd, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "Inter Games",       "Jolly Card (German, Fun World, bootleg)",         0,                       layout_jollycrd )
GAMEL( 1992, jolycdsp,   jollycrd, cuoreuno, jolycdit,  funworld_state, init_ctunk,    ROT0, "TAB Austria",       "Jolly Card (Spanish, blue TAB board, encrypted)", 0,                       layout_royalcrd )
GAMEL( 1990, jolycdid,   jollycrd, cuoreuno, jolycdcr,  funworld_state, empty_init,    ROT0, "bootleg",           "Jolly Card (Italian, different colors, set 1)",   0,                       layout_jollycrd ) // italian, CPLD, different colors.
GAMEL( 1990, jolycdie,   jollycrd, cuoreuno, jolycdib,  funworld_state, empty_init,    ROT0, "bootleg",           "Jolly Card (Italian, different colors, set 2)",   0,                       layout_jollycrd ) // not from TAB blue PCB.
GAMEL( 1990, jolycdif,   jollycrd, cuoreuno, jolycdib,  funworld_state, empty_init,    ROT0, "bootleg",           "Jolly Card (Italian, bootleg, set 1)",            0,                       layout_jollycrd ) // italian, CPLD. doesn't need nvram init.
GAMEL( 1993, jolycdig,   jollycrd, cuoreuno, jolycdib,  funworld_state, init_jolycdig, ROT0, "bootleg",           "Jolly Card (Italian, bootleg, set 2)",            0,                       layout_jollycrd ) // italian, CPLD. needs nvram.
GAMEL( 1990, jolycdih,   jollycrd, cuoreuno, jolycdib,  funworld_state, empty_init,    ROT0, "bootleg",           "Jolly Card (Italian, bootleg, set 3)",            0,                       layout_jollycrd )
GAMEL( 1993, jolygame,   jollycrd, fw2ndpal, funworld,  funworld_state, empty_init,    ROT0, "bootleg",           "Jolly Game (V1, Otti Karl logo)",                 0,                       layout_jollycrd )

// Bonus Card based...
GAMEL( 1986, bonuscrd,   0,        fw2ndpal, bonuscrd,  funworld_state, empty_init,    ROT0, "Fun World",         "Bonus Card (German)",                             MACHINE_IMPERFECT_COLORS, layout_bonuscrd ) // use fw1stpal machine for green background
GAMEL( 1986, bonuscrda,  bonuscrd, fw2ndpal, bonuscrd,  funworld_state, empty_init,    ROT0, "Fun World",         "Bonus Card (German, ATG Electronic hack)",        MACHINE_IMPERFECT_COLORS, layout_bonuscrd ) // use fw1stpal machine for green background
GAMEL( 1986, bigdeal,    bonuscrd, fw2ndpal, bigdeal,   funworld_state, empty_init,    ROT0, "Fun World",         "Big Deal (Hungarian, set 1)",                     MACHINE_IMPERFECT_COLORS, layout_bonuscrd )
GAMEL( 1986, bigdealb,   bonuscrd, fw2ndpal, bigdeal,   funworld_state, empty_init,    ROT0, "Fun World",         "Big Deal (Hungarian, set 2)",                     MACHINE_IMPERFECT_COLORS, layout_bonuscrd )

// CMC Italian jamma PCB's...
GAMEL( 1996, cuoreuno,   0,        cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Cuore 1 (Italian, set 1)",                        0,                       layout_jollycrd )
GAMEL( 1996, cuoreunoa,  cuoreuno, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Cuore 1 (Italian, set 2)",                        0,                       layout_jollycrd )
GAMEL( 1996, cuoreunob,  cuoreuno, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Cuore 1 (Italian, set 3)",                        0,                       layout_jollycrd )
GAMEL( 1996, cuoreunoc,  cuoreuno, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Cuore 1 (Italian, set 4)",                        0,                       layout_jollycrd )
GAMEL( 1996, cuoreunod,  cuoreuno, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Cuore 1 (Italian, set 5)",                        0,                       layout_jollycrd )
GAMEL( 1996, cuoreunoe,  cuoreuno, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Cuore 1 (Italian, set 6)",                        0,                       layout_jollycrd )
GAMEL( 1996, cuoreunof,  cuoreuno, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "bootleg",           "Cuore 1 (Italian, set 7)",                        0,                       layout_jollycrd )
GAMEL( 1997, elephfam,   0,        cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Elephant Family (Italian, new)",                  0,                       layout_jollycrd )
GAMEL( 1996, elephfmb,   elephfam, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Elephant Family (Italian, old)",                  0,                       layout_jollycrd )
GAMEL( 1996, pool10,     0,        cuoreuno, pool10,    funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 1)",                        0,                       layout_jollycrd )
GAMEL( 1996, pool10b,    pool10,   cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 2)",                        0,                       layout_jollycrd )
GAMEL( 1996, pool10c,    pool10,   cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 3)",                        0,                       layout_jollycrd )
GAMEL( 1997, pool10d,    pool10,   cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 4)",                        0,                       layout_jollycrd )
GAMEL( 1997, pool10f,    pool10,   cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 5)",                        0,                       layout_jollycrd )
GAMEL( 1996, pool10g,    pool10,   cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 6)",                        0,                       layout_jollycrd )
GAMEL( 1996, pool10h,    pool10,   cuoreuno, pool10,    funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 7)",                        0,                       layout_jollycrd )
GAMEL( 1997, pool10i,    pool10,   cuoreuno, pool10,    funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 8)",                        0,                       layout_jollycrd )
GAMEL( 1996, pool10j,    pool10,   cuoreuno, pool10,    funworld_state, empty_init,    ROT0, "bootleg (R.B.)",    "Pool 10 (Italian, set 9)",                        0,                       layout_jollycrd )
GAMEL( 1997, pool10k,    pool10,   cuoreuno, pool10,    funworld_state, empty_init,    ROT0, "C.M.C.",            "Pool 10 (Italian, set 10)",                       0,                       layout_jollycrd )
GAMEL( 1997, mag10,      pool10,   cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "bootleg (Magic)",   "Magic 10 Nevada (bootleg of Pool 10)",            0,                       layout_jollycrd )
GAMEL( 1998, biliard,    pool10,   cuoreuno, pool10,    funworld_state, empty_init,    ROT0, "bootleg",           "New Biliard 98 (Pool 10 hack)",                   0,                       layout_jollycrd )
GAMEL( 2001, royal,      pool10,   royalcd1, royal,     funworld_state, empty_init,    ROT0, "<unknown>",         "Royal (Pool 10 hack)",                            0,                       layout_jollycrd )
GAMEL( 1997, tortufam,   0,        cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Tortuga Family (Italian)",                        0,                       layout_jollycrd )
GAMEL( 1996, potgame,    0,        cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Pot Game (Italian)",                              0,                       layout_jollycrd )
GAMEL( 1996, bottle10,   0,        cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Bottle 10 (Italian, set 1)",                      0,                       layout_jollycrd )
GAMEL( 1996, bottl10b,   bottle10, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "C.M.C.",            "Bottle 10 (Italian, set 2)",                      0,                       layout_jollycrd )
GAMEL( 1998, lunapark,   0,        lunapark, lunapark,  lunapark_state, empty_init,    ROT0, "<unknown>",         "Luna Park (set 1, dual program)",                 0,                       layout_jollycrd ) // mirrored video RAM (4000/5000 to 6000/7000).
GAMEL( 1998, lunaparkb,  lunapark, lunapark, lunapark,  lunapark_state, empty_init,    ROT0, "<unknown>",         "Luna Park (set 2, dual program)",                 0,                       layout_jollycrd ) // mirrored video RAM (4000/5000 to 6000/7000).
GAMEL( 1998, lunaparkc,  lunapark, cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "<unknown>",         "Luna Park (set 3)",                               0,                       layout_jollycrd ) // regular video RAM 6000/7000.
GAMEL( 1998, crystal,    0,        cuoreuno, cuoreuno,  funworld_state, empty_init,    ROT0, "J.C.D. srl",        "Crystal Colours (CMC hardware)",                  0,                       layout_jollycrd )

// Royal Card based...
GAMEL( 1991, royalcrd,   0,        royalcd2, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (German, set 1)",                      0,                       layout_jollycrd )
GAMEL( 1991, royalcrda,  royalcrd, royalcd2, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (German, set 2)",                      0,                       layout_jollycrd )
GAMEL( 1991, royalcrdb,  royalcrd, royalcd1, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (German/Polish, set 3)",               0,                       layout_jollycrd )
GAMEL( 1991, royalcrdc,  royalcrd, royalcd2, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (German, set 4)",                      0,                       layout_jollycrd )
GAMEL( 1991, royalcrdd,  royalcrd, royalcd1, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (German, set 5)",                      0,                       layout_royalcrd )
GAMEL( 1991, royalcrde,  royalcrd, royalcd1, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (German, set 6)",                      0,                       layout_jollycrd )
GAMEL( 1991, royalcrdt,  royalcrd, royalcd1, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (TAB original)",                       0,                       layout_jollycrd )
GAMEL( 1990, royalcrdg,  royalcrd, royalcd1, royalcrd,  funworld_state, empty_init,    ROT0, "bootleg",           "Royal Card (German, set 7, CMC C1030 HW)",        0,                       layout_jollycrd ) // big CPLD
GAMEL( 1991, royalcrdh,  royalcrd, royalcd2, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (German, set 8)",                      0,                       layout_jollycrd )
GAMEL( 1991, royalcdfr,  royalcrd, royalcd1, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Royal Card (French)",                             0,                       layout_jollycrd )
GAME(  1993, royalcrdp,  royalcrd, cuoreuno, royalcrd,  funworld_state, empty_init,    ROT0, "Digital Dreams",    "Royal Card v2.0 Professional",                    0 )
GAMEL( 199?, royaljp,    royalcrd, royalcd1, royaljp,   funworld_state, init_tabblue,  ROT0, "TAB / Video Klein", "Royal Card / Royal Jackpot (with a third draw)(encrypted)", 0,             layout_jollycrd )
GAMEL( 199?, witchryl,   0,        witchryl, witchryl,  funworld_state, empty_init,    ROT0, "Video Klein",       "Witch Royal (Export version 2.1)",                0,                       layout_jollycrd )

// Lucky Lady based...
GAMEL( 1991, lluck3x3,   royalcrd, cuoreuno, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Lucky Lady (3x3 deal)",                           0,                       layout_jollycrd )
GAMEL( 1991, lluck4x1,   royalcrd, royalcd1, royalcrd,  funworld_state, empty_init,    ROT0, "TAB Austria",       "Lucky Lady (4x1 aces)",                           0,                       layout_jollycrd )

// Magic Card 2 based...
GAMEL( 1996, magicrd2,   0,        magicrd2, magicrd2,  magicrd2_state, empty_init,    ROT0, "Impera",            "Magic Card II (Bulgarian)",                       0,                       layout_jollycrd )
GAMEL( 1996, magicrd2a,  magicrd2, magicrd2, magicrd2,  magicrd2_state, empty_init,    ROT0, "Impera",            "Magic Card II (Nov, Yugoslavian)",                0,                       layout_jollycrd )
GAME(  1996, magicrd2b,  magicrd2, magicrd2, magicrd2,  magicrd2_state, init_magicd2b, ROT0, "Impera",            "Magic Card II (green TAB or Impera board)",       0 )
GAME(  1996, magicrd2c,  magicrd2, magicrd2, magicrd2,  magicrd2_state, init_magicd2c, ROT0, "Impera",            "Magic Card II (blue TAB board, encrypted)",       0 )

// Joker Card based...
GAMEL( 1993, vegasslw,   0,        fw2ndpal, vegasslw,  funworld_state, empty_init,    ROT0, "Fun World",         "Royal Vegas Joker Card (slow deal)",              0,                       layout_jollycrd )
GAMEL( 1993, vegasfst,   vegasslw, fw2ndpal, vegasfst,  funworld_state, empty_init,    ROT0, "Soft Design",       "Royal Vegas Joker Card (fast deal)",              0,                       layout_jollycrd )
GAMEL( 1993, vegasfte,   vegasslw, fw2ndpal, vegasfte,  funworld_state, empty_init,    ROT0, "Soft Design",       "Royal Vegas Joker Card (fast deal, English gfx)", 0,                       layout_jollycrd )
GAMEL( 1993, vegasmil,   vegasslw, fw2ndpal, vegasmil,  funworld_state, empty_init,    ROT0, "Mile",              "Royal Vegas Joker Card (fast deal, Mile)",        0,                       layout_jollycrd )

// Jolly Joker based...
GAMEL( 199?, jolyjokr,   0,        fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "Impera",            "Jolly Joker (98bet, set 1)",                      0,                       layout_jollycrd )
GAMEL( 199?, jolyjokra,  jolyjokr, fw1stpal, jolyjokra, funworld_state, empty_init,    ROT0, "Impera",            "Jolly Joker (98bet, set 2)",                      0,                       layout_jollycrd )
GAMEL( 199?, jolyjokrb1, jolyjokr, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "Impera",            "Jolly Joker (40bet, higher paytable)",            0,                       layout_jollycrd )
GAMEL( 199?, jolyjokrb2, jolyjokr, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "Impera",            "Jolly Joker (40bet, lower paytable)",             0,                       layout_jollycrd )
GAMEL( 199?, jolyjokrc,  jolyjokr, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "Apple Time",        "Jolly Joker (Apple Time)",                        MACHINE_NOT_WORKING,     layout_jollycrd )  // bad program ROM...
GAMEL( 199?, jolyjokro,  jolyjokr, fw2ndpal, funworld,  funworld_state, init_impera16, ROT0, "Impera",            "Jolly Joker (original program, interleaved GFX, Impera logo)",  0,         layout_jollycrd )
GAMEL( 199?, jolyjokrp,  jolyjokr, fw2ndpal, funworld,  funworld_state, init_impera16, ROT0, "Impera",            "Jolly Joker (original program, interleaved GFX, no logo)",      0,         layout_jollycrd )
GAMEL( 199?, jolyjokrs,  jolyjokr, fw2ndpal, funworld,  funworld_state, init_impera16, ROT0, "Impera",            "Jolly Joker (original program, interleaved GFX, Otti Karl logo, set 1)", 0, layout_jollycrd )
GAMEL( 199?, jolyjokrt,  jolyjokr, fw2ndpal, funworld,  funworld_state, init_impera16, ROT0, "Impera",            "Jolly Joker (original program, interleaved GFX, Otti Karl logo, set 2)", 0, layout_jollycrd )
GAMEL( 199?, jolyjokrm,  jolyjokr, fw1stpal, funworld,  funworld_state, empty_init,    ROT0, "Impera",            "Jolly Joker (Solid State module in suicide board)", 0,                     layout_jollycrd )

// Encrypted games...
GAME(  1992, multiwin,   0,        multiwin, funworld,  multiwin_state, driver_init,   ROT0, "Fun World",         "Multi Win (Ver.0167, encrypted)",                 0 ) // original funworld, encrypted.
GAME(  1991, multiwina,  multiwin, multiwina,funworld,  multiwina_state,empty_init,    ROT0, "Fun World",         "Multi Win (Ver.0091, encrypted)",                 MACHINE_IMPERFECT_GRAPHICS ) // different encryption scheme, different tilemap / screen parameters
GAME(  2001, multiwinb,  multiwin, fw2ndpal, funworld,  funworld_state, empty_init,    ROT0, "Amatic",            "Multi Win (EPM7032, encrypted)",                  MACHINE_NOT_WORKING ) // daughterboard with R65C02 + Altera EPM7032.
GAME(  1993, powercrd,   0,        multiwina,funworld,  multiwina_state,empty_init,    ROT0, "Fun World",         "Power Card (Ver 0263, encrypted)",                0 ) // clone of Bonus Card.
GAME(  1993, megacard,   0,        multiwina,funworld,  multiwina_state,empty_init,    ROT0, "Fun World",         "Mega Card (Ver.0210, encrypted)",                 0 )
GAME(  1993, megacarda,  megacard, multiwina,funworld,  multiwina_state,empty_init,    ROT0, "Fun World",         "Mega Card (Ver.0053, encrypted)",                 0 )
GAME(  1993, jokercrd,   0,        jokercrd, funworld,  jokercrd_state, empty_init,    ROT0, "Amatic Trading",    "Joker Card 300 (Ver.A267BC, encrypted)",          0 )
GAME(  1991, royalcrdf,  royalcrd, royalcrdf,royalcrdf, royalcrdf_state,driver_init,   ROT0, "Evona Electronic",  "Royal Card (Evona, Polish, encrypted)",           0 )
GAME(  198?, saloon,     0,        saloon,   saloon,    funworld_state, init_saloon,   ROT0, "<unknown>",         "Saloon (French, encrypted)",                      0 )
GAME(  198?, nevadafw,   0,        saloon,   saloon,    funworld_state, init_saloon,   ROT0, "<unknown>",         "Nevada (French, encrypted)",                      0 )

// Encrypted TAB blue PCB...
GAMEL( 199?, jolycdit,   jollycrd, cuoreuno, jolycdit,  funworld_state, init_tabblue,  ROT0, "bootleg",           "Jolly Card (Italian, blue TAB board, encrypted)", 0,                       layout_royalcrd )
GAMEL( 1990, jolycdib,   jollycrd, cuoreuno, jolycdib,  funworld_state, init_tabblue,  ROT0, "bootleg",           "Jolly Card (Italian, encrypted bootleg, set 1)",  0,                       layout_jollycrd ) // not a real TAB blue PCB
GAMEL( 1993, jolycdic,   jollycrd, cuoreuno, jolycdic,  funworld_state, init_tabblue,  ROT0, "bootleg",           "Jolly Card (Italian, encrypted bootleg, set 2)",  0,                       layout_jollycrd ) // not a real TAB blue PCB
GAMEL( 1993, jolycdii,   jollycrd, cuoreuno, jolycdib,  funworld_state, init_tabblue,  ROT0, "bootleg (Cleco)",   "Jolly Card (Italian, encrypted bootleg, set 3)",  0,                       layout_jollycrd ) // Cleco bootleg PCB
GAMEL( 1993, jolycdcy,   jollycrd, cuoreuno, jolycdcy,  funworld_state, init_tabblue,  ROT0, "bootleg (Cleco)",   "Zabavni Karti (Bulgarian, cyrillic, encrypted)",  0,                       layout_jollycrd ) // Cleco bootleg PCB

// Dino 4 encrypted hardware...
GAMEL( 1997, pool10e,    pool10,   cuoreuno, cuoreuno,  funworld_state, init_dino4,    ROT0, "C.M.C.",            "Pool 10 (Italian, Dino 4 hardware, encrypted)",   0,                       layout_jollycrd )
GAME(  1998, rcdino4,    0,        rcdino4,  rcdino4,   chinatow_state, init_rcdino4,  ROT0, "<unknown>",         "Royal Card (Italian, Dino 4 hardware, encrypted)",0 )
GAMEL( 1998, chinatow,   0,        chinatow, chinatow,  chinatow_state, init_rcdinch,  ROT0, "<unknown>",         "China Town (Ver 1B, Dino4 HW)",                   0,                       layout_jollycrd )

// MCU based games...
GAME(  199?, mongolnw,   0,        royalcd1, royalcrd,  funworld_state, init_mongolnw, ROT0, "<unknown>",         "Mongolfier New (Italian)",                        MACHINE_UNEMULATED_PROTECTION )
GAME(  199?, soccernw,   0,        royalcd1, royalcrd,  funworld_state, init_soccernw, ROT0, "<unknown>",         "Soccer New (Italian)",                            MACHINE_UNEMULATED_PROTECTION )

// Quiz games...
GAME(  199?, funquiz,    0,        funquiz,  funquiz,   funworld_state, empty_init,    ROT0, "Fun World",         "Fun World Quiz (German)",                         0 )
GAME(  1990, funquiza,   0,        funquiz,  funquiza,  funworld_state, empty_init,    ROT0, "Fun World",         "Fun World Quiz (German, 12-11-1990)",             0 )
GAME(  1990, funquizb,   0,        funquiz,  funquiza,  funworld_state, empty_init,    ROT0, "Fun World",         "Fun World Quiz (German, 27-04-1990)",             0 )

// Other games...
GAMEL( 1986, novoplay,   0,        fw2ndpal,   novoplay,  funworld_state,   empty_init,   ROT0, "Admiral / Novomatic",      "Novo Play Multi Card / Club Card",      0,                       layout_novoplay )
GAMEL( 1992, novoplaya,  novoplay, intrgmes,   novop_ab,  intergames_state, init_novop_a, ROT0, "Novo Play International",  "Novo Play Club Card (V6.2H)",           0,                       layout_novoplay )
GAMEL( 1991, novoplayb,  novoplay, intrgmes,   novop_ab,  intergames_state, init_novop_b, ROT0, "Novo Play International",  "Novo Play Club Card (V3.3H)",           0,                       layout_novoplay )
GAME(  1991, intrgmes,   0,        intrgmes,   intrgmes,  intergames_state, init_intgms,  ROT0, "Inter Games",             "Joker Card (Inter Games)",               0 )
GAMEL( 1985, fw_a7_11,   0,        fw_brick_2, fw_brick1, funworld_state,   empty_init,   ROT0, "Fun World",          "unknown Fun World A7-11 game 1",              MACHINE_NOT_WORKING,     layout_jollycrd )
GAMEL( 1985, fw_a7_11a,  fw_a7_11, fw_brick_2, fw_brick1, funworld_state,   empty_init,   ROT0, "Fun World",          "unknown Fun World A7-11 game 2",              MACHINE_NOT_WORKING,     layout_jollycrd )
GAMEL( 1991, fw_a0_1,    0,        fw_brick_2, fw_brick1, funworld_state,   empty_init,   ROT0, "Fun World",          "unknown Fun World A0-1 game",                 MACHINE_NOT_WORKING,     layout_jollycrd )
GAMEL( 1991, jokcrdep,   0,        fw_brick_2, fw_brick1, funworld_state,   empty_init,   ROT0, "Fun World",          "Joker Card / Multi Card (Epoxy brick CPU)",   MACHINE_NOT_WORKING,     layout_jollycrd )
GAMEL( 199?, gratispk,   0,        gratispk,   funworld,  funworld_state,   empty_init,   ROT0, "Mega Soft",          "Gratis Poker (V.204, set 1)",                 0,                       layout_jollycrd )
GAMEL( 199?, gratispka,  gratispk, gratispk,   funworld,  funworld_state,   empty_init,   ROT0, "Mega Soft",          "Gratis Poker (V.204, set 2)",                 0,                       layout_jollycrd )
GAMEL( 1995, nkoulit,    royalcrd, royalcd1,   royalcrd,  funworld_state,   init_tabblue, ROT0, "JK Amusement",       "Gkoulit (encrypted GFX)",                     0,                       layout_jollycrd )
GAMEL( 1999, reflexcrd,  royalcrd, royalcd1,   royalcrd,  funworld_state,   empty_init,   ROT0, "TAB Austria",        "Reflex Cards (English, serial protected)",    0,                       layout_jollycrd )
GAMEL( 199?, clubcard,   0,        clubcard,   clubcard,  funworld_state,   empty_init,   ROT0, "Impera / Mega Tech", "Club Card (ver. 1.1 English)",                MACHINE_IMPERFECT_COLORS, layout_jollycrd ) // use fw1stpal machine for green background

// These are 2-in-1 stealth boards, they can run the Poker game, or, using completely separate hardware on the same PCB, a NES / MSX Multigames!
GAMEL( 1991, royalcrd_nes,  royalcrd, royalcd2, royalcrd, funworld_state, empty_init, ROT0, "bootleg",         "Royal Card (stealth with NES multigame, set 1)",  MACHINE_NOT_WORKING,     layout_jollycrd )
GAMEL( 1991, royalcrd_nesa, royalcrd, royalcd2, royalcrd, funworld_state, empty_init, ROT0, "bootleg",         "Royal Card (stealth with NES multigame, set 2)",  MACHINE_NOT_WORKING,     layout_jollycrd )
GAMEL( 1991, royalcrd_msx,  royalcrd, royalcd2, royalcrd, funworld_state, empty_init, ROT0, "bootleg",         "Royal Card (stealth with MSX multigame)",         MACHINE_NOT_WORKING,     layout_jollycrd )
