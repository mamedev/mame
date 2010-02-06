/**********************************************************************************


    FUNWORLD / TAB.

    Original preliminary driver:    Curt Coder, Peter Trauner.
    Rewrite and aditional work:     Roberto Fresca.


    Games running on this hardware:

    * Jolly Card (austrian),                            TAB-Austria,        1985.
    * Jolly Card (3x3 deal),                            TAB-Austria,        1985.
    * Jolly Card Professional 2.0,                      Spale-Soft,         2000.
    * Jolly Card (Evona Electronic),                    Evona Electronic    1998.
    * Jolly Card (croatian, set 1),                     TAB-Austria,        1985.
    * Jolly Card (croatian, set 2),                     Soft Design,        1993.
    * Jolly Card (italian, blue TAB board, encrypted),  bootleg,            199?.
    * Jolly Card (italian, encrypted bootleg),          bootleg,            1990.
    * Super Joly 2000 - 3x,                             M.P.                1985.
    * Jolly Card (austrian, Funworld, bootleg),         Inter Games,        1986.
    * Big Deal (hungarian, set 1),                      Funworld,           1986.
    * Big Deal (hungarian, set 2),                      Funworld,           1986.
    * Jolly Card (austrian, Funworld),                  Funworld,           1986.
    * Cuore 1 (italian),                                C.M.C.,             1996.
    * Elephant Family (italian, new),                   C.M.C.,             1997.
    * Elephant Family (italian, old),                   C.M.C.,             1996.
    * Pool 10 (italian, set 1),                         C.M.C.,             1996.
    * Pool 10 (italian, set 2),                         C.M.C.,             1996.
    * Pool 10 (italian, set 3),                         C.M.C.,             1996.
    * Pool 10 (italian, set 4),                         C.M.C.,             1997.
    * Tortuga Family (italian),                         C.M.C.,             1997.
    * Pot Game (italian),                               C.M.C.,             1996.
    * Bottle 10 (italian, set 1),                       C.M.C.,             1996.
    * Bottle 10 (italian, set 2),                       C.M.C.,             1996.
    * Royal Card (austrian, set 1),                     TAB-Austria,        1991.
    * Royal Card (austrian, set 2),                     TAB-Austria,        1991.
    * Royal Card (slovak, encrypted),                   Evona Electronic,   1991.
    * Royal Card Professional 2.0,                      Digital Dreams,     1993.
    * Lucky Lady (3x3 deal),                            TAB-Austria,        1991.
    * Lucky Lady (4x1 aces),                            TAB-Austria,        1991.
    * Magic Card II (bulgarian),                        Impera,             1996.
    * Magic Card II (green TAB or Impera board),        Impera,             1996.
    * Magic Card II (blue TAB board, encrypted),        Impera,             1996.
    * Royal Vegas Joker Card (slow deal),               Funworld,           1993.
    * Royal Vegas Joker Card (fast deal),               Soft Design,        1993.
    * Royal Vegas Joker Card (fast deal, english gfx),  Soft Design,        1993.
    * Jolly Joker,                                      Impera,             198?.
    * Jolly Joker (50bet),                              Impera,             198?.
    * Joker Card (Ver.A267BC, encrypted),               Vesely Svet,        1993.
    * Mongolfier New (italian),                         bootleg,            199?.
    * Soccer New (italian),                             bootleg,            199?.
    * Saloon (french, encrypted),                       unknown,            199?.


***********************************************************************************


    The hardware is generally composed by:

    CPU:    1x 65SC02 or 65C02 at 2MHz.
    Sound:  1x AY3-8910 or YM2149F (AY8910 compatible) at 2MHz.
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
    Also support 4bpp graphics and the palette limitation is 8 bits for color codes (256 x 16colors).
    It means the hardware was designed for more elaborated graphics than Jolly Card games...
    Color PROMs from current games are 512 bytes lenght, but they only use the first 256 bytes.

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

            A) Encrypted CPU. At least two Funworld boards have custom encrypted CPUs:

                - Joker Card from Vesely Svet use a custom unknown CPU and use encrypted prg roms.
                - Royal Card (slovak, encrypted) from Evona Electronic seems to use a block
                  with CPU + extras (ICs, TTL, etc) to manage the encryption.

            B) General encryption. Managed through hardware:

                - All games using the blue TAB PCB with 2x HY18CV85 (electrically-erasable PLDs), use
                  complex operations for each byte nibble. See DRIVER_INIT for the final algorithm.
                - Saloon (french) use bitswaps to address & data in program, graphics and color PROM.

    - Microcontroller. Some games are using an extra microcontroller mainly for protection.



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

    There is at least one missing game in the family... 'Hippo Family', also from C.M.C.
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


    * Jolly Card (austrian, Funworld, bootleg)

    This one seems to have normal RAM instead of NVRAM.
    Going through the code, there's not any NVRAM initialization routine through service 1 & 2.


    * Jolly Card Professional 2.0 (Spale-Soft)

    Each 1st boot, this game show a generated code. You must enter this code in a DOS program to
    get the input codes necessary to allow boot the game.

    This set is one of the most wanted for customers because is a real SCAM.
    The program has 2 hidden menues that allow change parameters without knowledge of the players.


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

    Regarding the CPU, it seems to be a custom one, or a daughterboard with a 65c02 + PLDs/TTLs.
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
    C1CB: D0 DD         bne  $C1AA      ; if not, branch to load a new PIA adress.

    In this example, as soon as Y register increments, the indexed writes go out of range.
    To get this piece of code working and initialize the PIAs properly, the instruction 0x91 should be
    "sta ($ZP)" instead of Y indexed. (like instruction 0x92 in stock 65c02's).


    * Jolly Joker (Impera, 50bet)

    To boot this game for 1st time, DSW should be in the following position:
    1=ON 2=OFF 3=ON 4=OFF 5=OFF 6=ON 7=ON 8=OFF

    Press RESET (key F3) and then SERVICE1 & SERVICE2 (keys 9 & 0), then RESET (key F3).
    When numbers start to fill the screen, press RESET (key F3) again to start the game.


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



***********************************************************************************


    Memory Map (generic)
    --------------------


    $0000 - $07FF   NVRAM           // All registers and settings.
    $0800 - $0803   PIA1            // Input Ports 0 & 1.
    $0A00 - $0A03   PIA2            // Input Ports 2 & 3.
    $0C00 - $0C00   AY-8910 (R/C)   // Read/Control.
    $0C01 - $0C01   AY-8910 (W)     // Write. Lamps through output ports.
    $0E00 - $0E00   CRTC6845 (A)    // MC6845 adressing.
    $0E01 - $0E01   CRTC6845 (R/W)  // MC6845 Read/Write.

    $2000 - $2FFF   VideoRAM (funworld/bigdeal)
    $3000 - $3FFF   ColorRAM (funworld/bigdeal)

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


***********************************************************************************


    *** Hardware Info ***


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



    Jolly Card (austrian, Funworld, bootleg)
    ----------------------------------------

    - 1x G65SC02P (CPU)
    - 1x MC68B45P (CRTC)
    - 1x AY3-8910 (sound)
    - 2x MC6821P  (PIAs)

    RAM:  - 1x 6116
          - 1x KM6264AL-10

    - 1x Crystal : 16.000 MHz



    Jolly Card (other)
    ------------------

    - 1x G65SC02P (CPU)
    - 1x MC68B45P (CRTC)
    - 1x AY3-8910 (sound)
    - 2x MC6821P  (PIAs)

    RAM:  - 1x NVram DS1220Y (instead of 6116)
          - 1x KM6264AL-10

    - 1x Crystal : 16.000 MHz



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


    Jolly Card (italian encrypted bootleg)
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


    Big Deal (hungarian)
    ------------------

    - 1x MC6845P
    - 1x YM2149F
    - 2x MC6821P
    - 1x Crystal 16.000 MHz



    Magic Card II (bulgarian)
    ---------------------------------

    - 1x Special CPU with CM602 (??) on it
    - 1x MC6845P
    - 1x YM2149F
    - 2x MC6821P
    - 1x Crystal 16.000 MHz
    - 2x HY18CV85 (electrically-erasable PLD)



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
    at the same time press toghether HOLD 2 + HOLD 4 for some seconds.

    The following menu will show up:

    HOLD 1 = modify max value for play 1 to 10.
    HOLD 2 = modify min value foe play 1 to 5.
    Press start to exit from programming mode.

    To unlock PCB from a possible lock:
    connect GND to pins component15 and solder14 for 5 seconds

    NB. Use lamps by 12V 0,15W
    Memory version: V16



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



    Royal Card (set 1)
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



    Royal Card (set 2)
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



    Royal Card (set 3, encrypted)
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



***********************************************************************************


    *** Driver Updates by Roberto Fresca ***


    [2005/09/08]
    - Added Cuore Uno, Elephant Family and Royal Card.

    [2005/09/19]
    - Added some clones.
    - Cleaned up and renamed all sets. Made parent-clone relationship.

    [2005/12/15]
    - Corrected CPU freq (2 MHz) in cuoreuno and elephfam (both have R65c02P2).
      (I suspect more games must have their CPU running at 2 MHz).
    - Corrected videoram and colorram offsets in cuoreuno and elephfam.
    - To initialize the NVRAM in cuoreuno and elephfam:
      Start game, press and hold Service1 & Service2, press reset (F3),
      release Service1 & Service2 and press reset (F3) again.

    [2006/10/18]
    - Corrected the screen size and visible area to cuoreuno and elephfam based on mc6845 registers.
    - Added all inputs to cuoreuno and elephfam.
    - Added test mode DIP switch to cuoreuno and elephfam.
    - Managed cuoreuno and elephfam inputs to pass the initial checks. Now both games are playable.
    - Changed the cuoreuno full name to "Cuore 1" (as shown in the attract).

    [2006/10/22-28]
    - Corrected cuoreuno and elephfam graphics to 4bpp.
    - Fixed elephfam gfx planes.
    - Simulated cuoreuno palette based on screenshots.
    - Simulated elephfam palette based on screenshots.


    [2006/11/01 to 2006/12/04]

    ******** REWRITE ********

    - Merged/splitted some machine drivers, memory maps and inputs.
    - Unified get_bg_tile_info for all games.
    - Mapped the input buttons in a better format (all games). Keys: 156-QW-ZXCVBNM.
    - Added proper color PROM decode routines.
    - Rewrote the technical notes.
    - Splitted the driver to driver/video.

    - Corrected the screen size and visible area to magiccrd based on mc6845 registers.
    - Added the remaining 2 GFX planes to magiccrd, but GFX are imperfect (bad decode or bad dump?). Color PROM need to be dumped.
    - Royalcrd: Added all inputs and DIP switches.
                Fixed memory map, gfx decode
                Corrected screen size and visible area based on mc6845 registers.
                Corrected CPU clock to 2mhz.

    - New game added: Joker Poker. Not working due to use of custom encrypted CPU.
    - New game added: Royal Card (Slovakia, encrypted). Not working due to use of a custom encrypted CPU.
    - Fixed jolycdcr gfx to 4bpp.
    - Other fixes to get jolycdcr running.
    - Managed royalcdb to work, using the 2nd half of program ROM. (seems to be mapped that way)
    - Managed jolycdit to work, but with imperfect graphics due to gfx encryption.
    - Fixed CPU clock to 2MHz. in all remaining games.
    - Fixed ay8910 frequency based on elephfam audio.
    - Fixed ay8910 volume in all games to avoid clipping.
    - Reworked jolycdcr inputs: The game was designed to work only with remote credits. After nvram init, set the payout dip to "manual".
    - Reworked jolycdit inputs: After nvram init, set the payout dip to "manual" to allow work the remote mode.
    - Set jolycdat as bigdeal clone. The game has the same layout/behaviour instead of the normal jolly card games, even when are sharing gfx roms.
    - Added the bipolar PROM and GAL to jolycdit. Confirmed the GFX ROMs as good dumps.
    - Added an alternate set of Elephant Family. This one lacks of test mode and doesn't allow to switch between min-max bets through stop1.
    - Added color PROMs to cuoreuno and elephfam sets but still no routed. Also added PLDs (protected, bad dumps).
    - Corrected jollycrd screen size and visible area based on mc6845 registers.
    - Hooked, wired and decoded the color prom in jollycrd sets based on jolycdit redump. Now colors are perfect.
    - Wired and decoded the color prom in cuoreuno and elephfam sets. Now colors are perfect.
    - Wired and decoded the color prom in royalcrd. Now colors are perfect.
    - Hooked, wired and decoded the color prom in bigdeal sets based on jolycdat (jollycrd palette).
      Colors seems to be correct, but need to check against the real thing. Flagged as IMPERFECT_COLORS till a color PROM dump appear.
    - Decrypted jolycdit gfx roms.
    - Added set Jolly Card (Austria, encrypted).
    - Decrypted jolycdae and managed the planes to show correct colors. The set is working properly.


    [2006/12/24]
    - Fixed some incomplete inputs.
    - Added new working game: Pool 10.
    - Added new working game: Tortuga Family.
    - Added new game: Mongolfier New. Not working due to the lack of MCU emulation.
    - Added new game: Soccer New. Not working due to the lack of MCU emulation.
    - Updated technical notes.

    [2007/02/01]
    - All crystals documented via #defines.
    - All CPU and sound clocks derived from #defined crystal values.
    - Added DIPLOCATIONS to all games.
    - Added a pool10 alternate set.
    - Added proper tsc87c52 MCU dumps to monglfir and soccernew.
    - Modified the refresh rate to 60 fps according to some video evidences.
    - Updated technical notes.

    [2007/02/25]
    - Added new game: Snooker 10 (Ver 1.11). Preliminary.
        Properly decoded GFX
        Proper colors decoded.
    - Updated technical notes.

    [2007/09/21]
    - Added new game: Saloon (France, encrypted). Preliminary.
    - Updated technical notes.

    [2008/02/10]
    - Switched to XTAL def.
    - Fixed Magic Card II graphics issues.
    - Fixed Magic Card II inputs.
    - Fixed screen and visible area to snooker10.
    - Renamed set monglfir to mongolnw.
    - Renamed sets description based on languages instead of countries.
    - Added new game: Magic Card II (green TAB or Impera board). Not working yet.
    - Added new game: Magic Card II (blue TAB board, encrypted). Not working yet.
    - Added new game: Jolly Card (3x3 deal).
    - Added new game: Jolly Card Professional 2.0 (with 'enter code' screen to boot).
    - Added new game: Lucky Lady (3x3 deal).
    - Added new game: Lucky Lady (4x1 aces).
    - Added new game: Royal Vegas Joker Card (fast deal).
    - Added new game: Royal Vegas Joker Card (slow deal).
    - Fixed some years and manufacturers.
    - Updated technical notes.
    - Cleaned up the driver.

    [2008/02/22]
    - Switched the color decoding routines to use resnet code.
    - Added complete color connections to/from 74ls373 to the source.

    [2008/02/25]
    - Added new game: Royal Vegas Joker Card (fast deal, english gfx).
    - Added new game: Jolly Joker.
    - Added new game: Jolly Joker (50bet).
    - Added new game: Jolly Card (croatian, set 2).
    - Added new game: Jolly Card (Evona Electronic)
    - Added new game: Super Joly 2000 - 3x
    - Fixed some inputs.
    - Masked inputs to allow jolyjokra to boot.
    - Added minor corrections.
    - Updated technical notes.

    [2008/03/14]
    - Added proper inputs to jolyc980.
    - Added temporary patch to allow bypass the "code" screen in jolyc980.
    - Updated technical notes.

    [2008/03/18]
    - Added new game: Pot Game (italian).
    - Updated technical notes.

    [2008/04/18]
    - Removed the temporary hack to jolyc980.
    - Updated technical notes regarding Magic Card II & Jolly Card Professional 2.0.
    - Moved snookr10 to its own driver.
    - Minor clean-up.

    [2008/04/27]
    - Fixed AY8910 volume to all games to avoid clips.
    - Merge bigdeal and funworld machine drivers thanks to the AY8910 rewrite.
    - Removed old unaccurate commentary about magiccrd tiles.

    [2008/04/29]
    All CMC italian games:
    - Added TICKET and HOPPER buttons to allow payout through the SUPER GAME.
    - Documented the featured SUPER GAME with complete instructions.
    - Improved DIP switches to properly set the payout system.

    [2008/05/13]
    - Found the proper algorithm to decrypt the blue TAB PCB.
    - Replaced the old decryption tables with the proper decryption scheme.
    - Updated technical notes.

    [2008/07/30]
    - Added new clone: Pool 10 (italian, set 3).
    - Updated technical notes.

    [2008/09/12]
    - Added new clone: Pool 10 (italian, set 4).
    - Improved lamps layout for all games.
    - Added Pool 10 pinout and DIP switches info.
    - Updated technical notes.

    [2008/12/01]
    - Decripted saloon's program, graphics and color PROM.
    - Created a new memory map and machine driver for saloon.
    - Removed set jolycdae (it's not coming from a real board).
    - Renamed the sets magiccrd, magiccda and magiccdb, to magicrd2, magicd2a and magicd2b.
    - Updated technical notes.

    [2008/12/15]
    - Added new set: Jolly Card (italian, encrypted bootleg).
      No coins... Only remote credits. After nvram init, set the Payout DIP to 'manual'
      to allow the remote credits mode to work.
    - Created inputs from the scratch for jolycdib.
    - Updated technical notes.

    [2008-12-26]
    - Correctly setup the MC6845 device for all systems.
    - Added common MC6845 device interface.
    - Eliminated the screen size & visible area parameters to Magic Card 2.

    [2009/01/23]
    - Added new sets: Bottle 10 (italian, set 1 & 2).
    - Updated technical notes.

    [2009/09/09]
    - Discovered and documented new features in Jolly Card (Evona Electronics).
    - Removed the commented hack for joly980 driver init since is not needed anymore.
    - Fixed a bug introduced with the massive input change, that didn't allow initialize
      all sets that need the normal procedure to do it.
    - Updated technical notes.


    *** TO DO ***

    - Figure out the royalcdc & jokercrd encryption.
    - Figure out the remaining PIA connections for almost all games.
    - Fix Saloon and move it to its own driver.
    - Fix the imperfect sound in Magic Card II.
    - Reverse-engineering the boot code of Jolly Card Professional 2.0,
      and Royal Card Professional 2.0 to get the proper codes to boot.
    - Analyze the unknown writes to $2000/$4000 in some games.
    - Check for the reads to the ay8910 output ports in some games.
    - Figure out the MCU in monglfir and soccernw.
    - Correct the ROM_REGION in some games to allow the use of RGN_FRAC


***********************************************************************************/


#define MASTER_CLOCK	XTAL_16MHz

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "funworld.lh"

/* from video */
extern UINT8* funworld_videoram;
extern UINT8* funworld_colorram;

WRITE8_HANDLER( funworld_videoram_w );
WRITE8_HANDLER( funworld_colorram_w );
PALETTE_INIT( funworld );
VIDEO_START( funworld );
VIDEO_START( magicrd2 );
VIDEO_UPDATE( funworld );


/**********************
* Read/Write Handlers *
**********************/

static WRITE8_DEVICE_HANDLER(funworld_lamp_a_w)
{
	coin_counter_w(device->machine, 0, data & 0x01);					/* credit in counter */

	output_set_lamp_value(0, 1-((data >> 1) & 1));	/* button hold1 and */
	output_set_lamp_value(2, 1-((data >> 1) & 1));	/* hold3 (see pinouts) */

	coin_counter_w(device->machine, 7, data & 0x04);					/* credit out counter, mapped as coin 8 */

	output_set_lamp_value(1, 1-((data >> 3) & 1));	/* button hold2/low */
	output_set_lamp_value(5, 1-((data >> 5) & 1));	/* button 6 (collect/cancel) */
	output_set_lamp_value(3, (data >> 7) & 1);		/* button hold4/high */
}

static WRITE8_DEVICE_HANDLER(funworld_lamp_b_w)
{
	output_set_lamp_value(4, (data >> 0) & 1);		/* button hold5/bet */
	output_set_lamp_value(6, (data >> 1) & 1);		/* button 7 (start/play) */
}

static WRITE8_DEVICE_HANDLER(pia1_ca2_w)
{
/* TAB and Impera games are writting 0x01 constantly, and 0x00 with each screen change.
   This line is tied to sort of reset circuitery.
*/
//  popmessage("PIA1 CA2: %02X", data);
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( funworld_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x0800, 0x0803) AM_DEVREADWRITE("pia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x0c00, 0x0c00) AM_DEVREAD("ay8910", ay8910_r)
	AM_RANGE(0x0c00, 0x0c01) AM_DEVWRITE("ay8910", ay8910_address_data_w)
	AM_RANGE(0x0e00, 0x0e00) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0e01, 0x0e01) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x2000, 0x2fff) AM_RAM_WRITE(funworld_videoram_w) AM_BASE(&funworld_videoram)
	AM_RANGE(0x3000, 0x3fff) AM_RAM_WRITE(funworld_colorram_w) AM_BASE(&funworld_colorram)
	AM_RANGE(0x4000, 0x4000) AM_READNOP
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( magicrd2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x0800, 0x0803) AM_DEVREADWRITE("pia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x0c00, 0x0c00) AM_DEVREAD("ay8910", ay8910_r)
	AM_RANGE(0x0c00, 0x0c01) AM_DEVWRITE("ay8910", ay8910_address_data_w)
	AM_RANGE(0x0e00, 0x0e00) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0e01, 0x0e01) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x2c00, 0x2cff) AM_RAM	/* range for protection */
	AM_RANGE(0x3600, 0x36ff) AM_RAM	/* some games use $3603-05 range for protection */
	AM_RANGE(0x3c00, 0x3cff) AM_RAM	/* range for protection */
	AM_RANGE(0x4000, 0x4fff) AM_RAM_WRITE(funworld_videoram_w) AM_BASE(&funworld_videoram)
	AM_RANGE(0x5000, 0x5fff) AM_RAM_WRITE(funworld_colorram_w) AM_BASE(&funworld_colorram)
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cuoreuno_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x0800, 0x0803) AM_DEVREADWRITE("pia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x0c00, 0x0c00) AM_DEVREAD("ay8910", ay8910_r)
	AM_RANGE(0x0c00, 0x0c01) AM_DEVWRITE("ay8910", ay8910_address_data_w)
	AM_RANGE(0x0e00, 0x0e00) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0e01, 0x0e01) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x2000, 0x2000) AM_READNOP	/* some unknown reads */
	AM_RANGE(0x3e00, 0x3fff) AM_RAM	/* some games use $3e03-05 range for protection */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(funworld_videoram_w) AM_BASE(&funworld_videoram)
	AM_RANGE(0x7000, 0x7fff) AM_RAM_WRITE(funworld_colorram_w) AM_BASE(&funworld_colorram)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( royalmcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x2800, 0x2803) AM_DEVREADWRITE("pia0", pia6821_r, pia6821_w)
	AM_RANGE(0x2a00, 0x2a03) AM_DEVREADWRITE("pia1", pia6821_r, pia6821_w)
	AM_RANGE(0x2c00, 0x2c00) AM_DEVREAD("ay8910", ay8910_r)
	AM_RANGE(0x2c00, 0x2c01) AM_DEVWRITE("ay8910", ay8910_address_data_w)
	AM_RANGE(0x2e00, 0x2e00) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x2e01, 0x2e01) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x4000, 0x4fff) AM_RAM_WRITE(funworld_videoram_w) AM_BASE(&funworld_videoram)
	AM_RANGE(0x5000, 0x5fff) AM_RAM_WRITE(funworld_colorram_w) AM_BASE(&funworld_colorram)
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( saloon_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x0800, 0x0800) AM_READ_PORT("IN0")
	AM_RANGE(0x0a01, 0x0a01) AM_READ_PORT("IN1")
	AM_RANGE(0x081c, 0x081c) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x081d, 0x081d) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("IN2")
	AM_RANGE(0x1800, 0x1800) AM_DEVREAD("ay8910", ay8910_r)
	AM_RANGE(0x1800, 0x1801) AM_DEVWRITE("ay8910", ay8910_address_data_w)
//  AM_RANGE(0x2000, 0x2000) AM_READNOP /* some unknown reads... maybe a DSW */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(funworld_videoram_w) AM_BASE(&funworld_videoram)
	AM_RANGE(0x7000, 0x7fff) AM_RAM_WRITE(funworld_colorram_w) AM_BASE(&funworld_colorram)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

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


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( funworld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Halten (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Loeschen (Cancel) / Kassieren (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Geben (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )		PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )		PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Halten (Hold) 4 / Hoch (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Halten (Hold) 2 / Tief (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Halten (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )	PORT_NAME("Abschreib (Payout)")

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
	PORT_DIPNAME( 0x01, 0x01, "State" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote" )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, "Insert" )			PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Dattl Insert" )
	PORT_DIPSETTING(    0x10, "TAB Insert" )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )		/* also enable Five of a Kind */
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */
	PORT_DIPNAME( 0x80, 0x00, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( jolycdcr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Navijanje (Remote)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Stop (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Ponistavange (Cancel) / Kasiranje (Take) / Autohold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Djelenje (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Stop (Hold) 5 / Ulog (Bet) / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )		PORT_NAME("Konobar (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )		PORT_NAME("Namjestit (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Stop (Hold) 4 / Veca (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Stop (Hold) 2 / Manja (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Stop (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )	PORT_NAME("Vratiti Nazad (Payout)")

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
	PORT_DIPNAME( 0x01, 0x01, "State" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote" )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )		/* also enable Five of a Kind */
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */
	PORT_DIPNAME( 0x80, 0x00, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( jolycdit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Stop (Hold) 1 / Alta (High)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Clear / Doppio (Double) / Autohold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Stop (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Stop (Hold) 4 / Accredito (Take)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Stop (Hold) 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Stop (Hold) 3 / Bassa (Low)")
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
	PORT_DIPNAME( 0x01, 0x01, "State" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote" )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "50 Points/Pulse" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )		/* also enable Five of a Kind */
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */
	PORT_DIPNAME( 0x80, 0x00, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( jolycdib )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Stop (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Cancel / Autohold / Accredito (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Start / Doppio (Double)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Stop (Hold) 5 / Bet / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Stop (Hold) 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Stop (Hold) 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Stop (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("test1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("test2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("test3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("test4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("test5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("test6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("test7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("test8") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("test9") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("test10") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("test11") PORT_CODE(KEYCODE_R)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Remote" )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )	/* also enable Five of a Kind */
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" )			PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )	/* when is ON, allow the player to activate/deactivate the autohold through CANCEL button */

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */
	PORT_DIPNAME( 0x80, 0x00, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( jolyc980 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Navijanje (Remote)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Stop (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Ponistavange (Cancel) / Kasiranje (Take) / Autohold")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Djelenje (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Stop (Hold) 5 / Ulog (Bet) / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )		PORT_NAME("Konobar (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )		PORT_NAME("Namjestit (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Stop (Hold) 4 / Veca (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Stop (Hold) 2 / Manja (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Stop (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )	PORT_NAME("Vratiti Nazad (Payout)")

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
	PORT_DIPNAME( 0x01, 0x01, "State" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )		/* also enable Five of a Kind */
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( bigdeal )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Clear / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Hold 5 / Stake")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Hold 4 / Nagy (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Hold 2 / Icsi (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Hold 3 / Half Gamble")
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
	/* the following one should be left ON by default to allow initialization */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Remote" )			PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x04, "100 Points/Pulse" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */
	PORT_DIPNAME( 0x80, 0x00, "Payout" )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( magicrd2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Clear / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Hold 5 / Stake")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Hold 4 / High")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Hold 2 / Low")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Hold 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Hopper SW") PORT_CODE(KEYCODE_H)
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
	PORT_DIPNAME( 0x01, 0x01, "State" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:7")	/* remote credits settings are always 10 points/pulse */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */
	PORT_DIPNAME( 0x80, 0x00, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( royalcrd )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Halten (Hold) 1 / Hoch (High)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Loeschen/Gamble (Cancel/Play)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Geben (Start)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )		PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )		PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Halten (Hold) 4 / Kassieren (Take)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Halten (Hold) 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Halten (Hold) 3 / Tief (Low)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("unknown bit 08") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )	PORT_NAME("Abschreib (Payout)")

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
	PORT_DIPNAME( 0x01, 0x01, "State" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote" )			PORT_DIPLOCATION("SW1:7")	/* listed as 'Coin-C' in some sources */
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "50 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x00, "Insert" )			PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Dattl Insert" )
	PORT_DIPSETTING(    0x10, "TAB Insert" )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )		/* also enable Five of a Kind */
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x00, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */
	PORT_DIPNAME( 0x80, 0x00, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( cuoreuno )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no remote credits */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Stop 1 / Switch Bet (1-Max)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Clear / Bet / Prendi (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Start / Gioca (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Stop 5 / Half Gamble / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
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
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )					PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )	PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - User Choice 1" )
	PORT_DIPSETTING(    0x20, "Manual - Coins" )
	PORT_DIPSETTING(    0x40, "Manual - Tickets" )
	PORT_DIPSETTING(    0x60, "Manual - User Choice 2" )

	/* the following one (1st DSW) seems to be disconnected
    to avoid the use of remote credits or direct payout */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pool10 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no remote credits */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Stop 1 / Switch Bet (1-Max)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Clear / Bet / Prendi (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Start / Gioca (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Stop 5 / Half Gamble / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Ticket") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
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
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )					PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )			PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )	PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - User Choice 1" )
	PORT_DIPSETTING(    0x20, "Manual - Coins" )
	PORT_DIPSETTING(    0x40, "Manual - Tickets" )
	PORT_DIPSETTING(    0x60, "Manual - User Choice 2" )

	/* direct payout without play Super Game */
	PORT_DIPNAME( 0x80, 0x80, "Direct Payout (tickets)" )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jolyjokra )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	PORT_NAME("Remote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )	PORT_NAME("Halten (Hold) 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )	PORT_NAME("Loeschen (Cancel) / Kassieren (Take)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )			PORT_NAME("Geben (Start) / Gamble (Play)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )	PORT_NAME("Halten (Hold) 5 / Half Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )		PORT_NAME("Buchhalt (Service1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )		PORT_NAME("Einstellen (Service2)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )	PORT_NAME("Halten (Hold) 4 / Hoch (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )	PORT_NAME("Halten (Hold) 2 / Tief (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )	PORT_NAME("Halten (Hold) 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )		PORT_NAME("Hoppersch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )	PORT_NAME("Abschreib (Payout)")

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
	PORT_DIPNAME( 0x01, 0x01, "State" )				PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Keyboard Test" )
	PORT_DIPSETTING(    0x01, "Play" )
	PORT_DIPNAME( 0x02, 0x00, "Remote" )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10 Points/Pulse" )
	PORT_DIPSETTING(    0x02, "100 Points/Pulse" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x04, "10 Points/Coin" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5 Points/Coin" )
	PORT_DIPSETTING(    0x08, "10 Points/Coin" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "disabled" )
	PORT_DIPSETTING(    0x10, "enabled" )
	PORT_DIPNAME( 0x20, 0x00, "Joker" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "With Joker" )		/* also enable Five of a Kind */
	PORT_DIPSETTING(    0x20, "Without Joker" )
	PORT_DIPNAME( 0x40, 0x40, "Hold" )				PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, "No Auto Hold" )

	/* after nvram init, set the following one to 'manual'
    to allow the remote credits mode to work */

	PORT_DIPNAME( 0x80, 0x00, "Payout" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPSETTING(    0x80, "Manual Payout SW" )
INPUT_PORTS_END

static INPUT_PORTS_START( saloon )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("DSW")
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


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	4,8,
	0x1000,
//  RGN_FRAC(1,2),
	4,
	{ 0, 4, 0x8000*8, 0x8000*8+4 },
//  { RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( funworld )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 16 )
GFXDECODE_END


/***********************
*    PIA Interfaces    *
***********************/

static const pia6821_interface pia0_intf =
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

*/
{
	DEVCB_INPUT_PORT("IN0"),		/* port A in */
	DEVCB_INPUT_PORT("IN1"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface pia1_intf =
{
	DEVCB_INPUT_PORT("IN2"),		/* port A in */
	DEVCB_INPUT_PORT("DSW"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_HANDLER(pia1_ca2_w),		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


/************************
*    Sound Interface    *
************************/

static const ay8910_interface ay8910_intf =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,							/* portA in  */
	DEVCB_NULL,							/* portB in  */
	DEVCB_HANDLER(funworld_lamp_a_w),	/* portA out */
	DEVCB_HANDLER(funworld_lamp_b_w)	/* portB out */
};


/************************
*    CRTC Interface    *
************************/

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	4,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_DRIVER_START( funworld )
    /* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M65SC02, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_CPU_PROGRAM_MAP(funworld_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_PIA6821_ADD("pia0", pia0_intf)
	MDRV_PIA6821_ADD("pia1", pia1_intf)

    /* video hardware */

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((124+1)*4, (30+1)*8)				/* Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1) */
	MDRV_SCREEN_VISIBLE_AREA(0*4, 96*4-1, 0*8, 29*8-1)	/* Taken from MC6845 init, registers 01 & 06 */

	MDRV_GFXDECODE(funworld)

	MDRV_DEFAULT_LAYOUT(layout_funworld)

	MDRV_PALETTE_LENGTH(0x200)
	MDRV_PALETTE_INIT(funworld)
	MDRV_VIDEO_START(funworld)
	MDRV_VIDEO_UPDATE(funworld)

	MDRV_MC6845_ADD("crtc", MC6845, MASTER_CLOCK/8, mc6845_intf)	/* 2MHz, veryfied on jollycrd & royalcrd */

    /* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay8910", AY8910, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_SOUND_CONFIG(ay8910_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.5)	/* analyzed to avoid clips */
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( magicrd2 )
	MDRV_IMPORT_FROM(funworld)

	MDRV_CPU_REPLACE("maincpu", M65C02, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_CPU_PROGRAM_MAP(magicrd2_map)

	MDRV_GFXDECODE(funworld)
	MDRV_VIDEO_START(magicrd2)

	MDRV_SOUND_REPLACE("ay8910", AY8910, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_SOUND_CONFIG(ay8910_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.5)	/* analyzed to avoid clips */
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( royalcrd )
	MDRV_IMPORT_FROM(funworld)

	MDRV_CPU_REPLACE("maincpu", M65C02, MASTER_CLOCK/8)	/* (G65SC02P in pro version) 2MHz */
	MDRV_CPU_PROGRAM_MAP(magicrd2_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cuoreuno )
	MDRV_IMPORT_FROM(funworld)

	MDRV_CPU_REPLACE("maincpu", M65C02, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_CPU_PROGRAM_MAP(cuoreuno_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( royalmcu )
	MDRV_IMPORT_FROM(funworld)

	MDRV_CPU_REPLACE("maincpu", M65SC02, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_CPU_PROGRAM_MAP(royalmcu_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( saloon )
	MDRV_IMPORT_FROM(funworld)

	MDRV_CPU_REPLACE("maincpu", M65C02, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_CPU_PROGRAM_MAP(saloon_map)
MACHINE_DRIVER_END

static const UINT8 jolyc980_default_eeprom[0x800] = {
	0x4C,0x17,0xAC,0x4C,0x3B,0xB6,0xAE,0x65,0x33,0x79,0x40,0x00,0x00,0x20,0x20,0x20,
	0x20,0x20,0x31,0x30,0x00,0x00,0x32,0x4D,0x64,0x30,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x60,0xE5,0x69,0x03,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xD4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x60,0x05,0x02,0x02,0x02,0xEE,0x07,0x00,0x01,0x00,0x09,0x09,0x99,0x6C,0xD8,
	0xA2,0x5A,0x00,0x04,0x00,0x01,0x2A,0x06,0x31,0x12,0x3D,0x00,0x00,0x00,0x01,0x0F,
	0x00,0x6D,0x00,0x00,0x00,0x00,0xBE,0x99,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFE,0x04,0x04,0x04,0x40,0x02,0x15,0xB3,0x00,0x40,0x00,0x00,0x00,0x00,0xB8,0xFF,
	0x00,0x07,0x01,0x2A,0x38,0x18,0x03,0x3C,0x2C,0x24,0x35,0x0C,0x04,0x3F,0x06,0x31,
	0x12,0x3D,0x13,0x1A,0x23,0x11,0x21,0x24,0x1C,0x06,0x27,0xFF,0x00,0xFF,0x00,0xFF,
	0x00,0x00,0x01,0xFF,0x00,0xFF,0x00,0xF7,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x02,0x05,
	0x00,0x00,0x00,0xFF,0x00,0xFF,0x08,0xFF,0xE3,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,
	0x40,0x6D,0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,
	0x00,0xFF,0x00,0xFF,0xDD,0xFD,0x07,0x80,0x07,0x02,0x08,0x80,0x08,0x02,0x0A,0x00,
	0x00,0x00,0x07,0x80,0x02,0x00,0x3F,0x00,0xFF,0xFF,0x00,0xFF,0xF0,0xFF,0x00,0x04,
	0x00,0x00,0xF0,0xFF,0x00,0x00,0x50,0xFF,0x00,0x00,0x6C,0x0C,0x80,0xE8,0x02,0xFE,
	0x48,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,
	0xFF,0x00,0xFE,0x00,0xFF,0x00,0xEE,0xEC,0x68,0xED,0x00,0x3D,0x9B,0x81,0xC2,0x00,
	0x40,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0x00,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0x00,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0x40,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,
	0xDF,0x00,0xC2,0x00,0xDF,0x00,0xC2,0x00,0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,
	0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,
	0x40,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,0xFF,0x00,0xFE,0x00,
	0xFF,0x00,0xFE,0x00,0xFF,0xEB,0xEB,0x60,0x60,0x60,0xB1,0x60,0xB1,0x60,0x60,0x60,
	0xB1,0x89,0xB1,0x0E,0xB0,0x31,0x20,0x20,0x20,0x20,0x20,0x00,0x40,0x79,0x60,0x67,
	0x82,0x00,0x04,0x77,0x20,0xD0,0x28,0xA0,0xC3,0x8F,0x4A,0xAB,0x75,0xAB,0xA9,0x86,
	0x00,0x00,0x00,0x01,0x98,0x05,0x05,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x02,0x00,
	0x01,0x00,0x00,0x00,0x3F,0x06,0x31,0x12,0x3D,0x06,0x00,0x00,0x00,0x00,0x00,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x01,0x00,0x02,0x00,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x56,0x32,0x2E,0x30,0x20,0x05,0x04,0x00,0x00,0x03,0x05,0x08,0x02,0x02,0x00,0x00,
	0x00,0x01,0x29,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x18,0x85,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x29,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xF0,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0xF0,0x00,0x00,0x00,0x7C,0x00,0x00,0x9F,
	0x91,0x62,0x04,0x00,0x00,0x00,0x05,0x15,0x09,0x00,0x00,0x00,0x31,0x85,0x03,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x91,0x62,0x04,0x00,0x00,0x00,0x05,0x15,0x09,0x00,0x00,0x00,0x31,
	0x85,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9D,
	0x1C,0x80,0xF0,0x02,0x00,0x00,0x00,0x80,0x60,0xEB,0x00,0xF7,0x40,0xFF,0x00,0x21
};

/*
Load a default eeprom, otherwise an annoying password system should be inserted with
inputs correlated to a code that pops up on the screen.
This code should be inserted into a PC-DOS program, that program is nowhere to be found right now.
*/
static NVRAM_HANDLER( jolyc980 )
{
	if (read_or_write)
		mame_fwrite(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else if (file)
		mame_fread(file, machine->generic.nvram.v, machine->generic.nvram_size);
	else
	{
		int i;
		for(i=0;i<machine->generic.nvram_size;i++)
			machine->generic.nvram.u8[i] = jolyc980_default_eeprom[i];
	}
}

static MACHINE_DRIVER_START( jolyc980 )
	MDRV_IMPORT_FROM(cuoreuno)

	MDRV_NVRAM_HANDLER( jolyc980 )
MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( jollycrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jolycard.run", 0x8000, 0x8000, CRC(f531dd10) SHA1(969191fbfeff4349afef619d9241ef5186e6d57f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolycdae )	/* encrypted roms...*/
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "pokeru0.bin", 0x8000, 0x10000, CRC(7732f177) SHA1(b75fca596a7315d1379fa5bcf07c449ec32c90f5) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "pokeru1.bin", 0x00000, 0x10000, CRC(878d695b) SHA1(0e1ea455e991e591316a340c56b23fa26764988d) )
	ROM_LOAD( "pokeru2.bin", 0x10000, 0x10000, CRC(b7b2946a) SHA1(25b15296b32bf88db6d60991105bba667f7940cc) )
	ROM_COPY( "gfx1",	 0x10000, 0x00000, 0x8000 ) /* rgn, srcoffset, offset, length */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolyc3x3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jc3x3.bin", 0x8000, 0x8000, CRC(71e304ad) SHA1(238b792d841432582c94b21a674d46a95e8f3826) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolyc980 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "j980.bin", 0x8000, 0x8000, CRC(48249fff) SHA1(390cd0eb3399446a66363dc6760458170e1970fd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolycdev )	/* Jolly Card (lipa) from Evona */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lipa.bin", 0x8000, 0x8000, CRC(62657c98) SHA1(237466dde26540c119c631c75f51c87ea59d1a91) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jollyb.bin", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jollya.bin", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END

ROM_START( jolyccra )	/* Jolly Card (croatian, set 1) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jollyc.bin", 0x8000, 0x8000, CRC(8926d99d) SHA1(dd5d1ac03d30d823dfcfe1349328ecb7afbc37fa) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jollyb.bin", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jollya.bin", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolyccrb )	/* Jolly Card (croatian, set 2) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jollypkr.003", 0x8000, 0x8000, CRC(ea7340b4) SHA1(7dd468f28a488a4781521809d06db1d7917048ad) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolycdit )	/* blue TAB PCB, encrypted graphics */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jn.bin", 0x8000, 0x8000, CRC(6ae00ed0) SHA1(5921c2882aeb5eadd0e04a477fa505ad35e9d98c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(46805150) SHA1(63687ac44f6ace6d8924b2629536bcc7d3979ed2) )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(43bcb2df) SHA1(5022bc3a0b852a7cd433e25c3c90a720e6328261) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.bin", 0x0000, 0x0117, CRC(3ad712b1) SHA1(54214841fb178e4b59bf6051522718f7667bad28) )
ROM_END

/* jolycdit vs jolycdib:

1.BIN                   1.BIN                   IDENTICAL
2.BIN                   2.bin                   IDENTICAL
jn.BIN                  3.BIN                   97.726440%

jolycdib program seems to be the original for blue TAB PCB.
jolycdit has some code patches and redirected parts to suspicious offsets (as d500, d000, etc)
*/
ROM_START( jolycdib )	/* bootleg PCB, encrypted graphics */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin", 0x8000, 0x8000, CRC(c76fdc79) SHA1(fc75c274d64fa9c99a546d424f38e79f1acf2576) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(46805150) SHA1(63687ac44f6ace6d8924b2629536bcc7d3979ed2) )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(43bcb2df) SHA1(5022bc3a0b852a7cd433e25c3c90a720e6328261) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8b.bin", 0x0000, 0x0117, CRC(3ad712b1) SHA1(54214841fb178e4b59bf6051522718f7667bad28) )
ROM_END

ROM_START( sjcd2kx3 )	/* Super Joly 2000 3x */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sj3.bin", 0x8000, 0x8000, CRC(c530b518) SHA1(36934d8e1e2cb2f71eb44a05b86ec970c9f398cd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sj2.bin", 0x0000, 0x8000, CRC(d7253325) SHA1(ad40e662519da9b11f77690b7d41c5007f74e280) )
	ROM_LOAD( "sj1.bin", 0x8000, 0x8000, CRC(82741749) SHA1(d0bf3073dff9ba1c4913fd754f965951e9cb5b03) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END

ROM_START( jolycdab )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* program is testing/writting RAM in offset $8800-$BFFF (ROM)...?? */
	ROM_LOAD( "ig1poker.run", 0x8000, 0x8000, CRC(c96e6542) SHA1(ed6c0cf9fe8597dba9149b2225320d8d9c39219a) )
//  ROM_RELOAD(               0x4000, 0x4000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jn1poker.ch2", 0x0000, 0x8000, CRC(8d78e43d) SHA1(15c60f8e0cd88518b0dc72b92aff6d8d4b2149cf) )
	ROM_LOAD( "jn1poker.ch1", 0x8000, 0x8000, CRC(d0a87f58) SHA1(6b7925557c4e40a1ebe52ecd14391cdd5e00b59a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( bigdeal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker4.001", 0x8000, 0x8000, CRC(bb0198c1) SHA1(6e7d42beb5723a4368ae3788f83b448f86e5653d) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "poker4.003", 0x0000, 0x8000, CRC(8c33a15f) SHA1(a1c8451c99a23eeffaedb21d1a1b69f54629f8ab) )
	ROM_LOAD( "poker4.002", 0x8000, 0x8000, CRC(5f4e12d8) SHA1(014b2364879faaf4922cdb82ee07692389f20c2d) )

	ROM_REGION( 0x0200, "proms", 0 )	/* using jollycrd palette till a correct dump appear */
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( bigdealb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker8.003", 0x8000, 0x8000, CRC(09c93745) SHA1(a64e96ef3489bc37c2c642f49e62cfef371de6f1) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "poker4.003", 0x0000, 0x8000, CRC(8c33a15f) SHA1(a1c8451c99a23eeffaedb21d1a1b69f54629f8ab) )
	ROM_LOAD( "poker4.002", 0x8000, 0x8000, CRC(5f4e12d8) SHA1(014b2364879faaf4922cdb82ee07692389f20c2d) )

	ROM_REGION( 0x0200, "proms", 0 )	/* using jollycrd palette till a correct dump appear */
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolycdat )	/* there are unused pieces of code that compare or jumps within $4000-$5000 range */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bonucard.cpu", 0x8000, 0x4000, CRC(da342100) SHA1(451fa6074aad19e9efd148c3d18115a20a3d344a) )
	ROM_CONTINUE(			  0xc000, 0x4000 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "jolycard.ch2", 0x0000, 0x8000, CRC(c512b103) SHA1(1f4e78e97855afaf0332fb75e1b5571aafd01c29) )
	ROM_LOAD( "jolycard.ch1", 0x8000, 0x8000, CRC(0f24f39d) SHA1(ac1f6a8a4a2a37cbc0d45c15187b33c25371bffb) )

	ROM_REGION( 0x0200, "proms", 0 )	/* using jollycrd palette till a correct dump appear */
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( cuoreuno )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuore1a.u2", 0x8000, 0x8000, CRC(6e112184) SHA1(283ac534fc1cb33d11bbdf3447630333f2fc957f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cuore1b.u21", 0x0000, 0x8000, CRC(14eca2b8) SHA1(35cba415800c6cd3e6ed9946057f33510ad2bfc9) )
	ROM_LOAD( "cuore1c.u22", 0x8000, 0x8000, CRC(253fac84) SHA1(1ad104ab8e8d73df6397a840a4b26565b245d7a3) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_cu.bin",    0x0000, 0x0200, CRC(7ea61749) SHA1(3167acd79f9bda2078c2af3e049ad6abf160aeae) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_cu.u5",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_cu.u22", 0x0200, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_cu.u23", 0x0400, 0x0157, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( elephfam )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "eleph_a.u2", 0x8000, 0x10000, CRC(8392b842) SHA1(74c850c734ca8174167b2f826b9b1ac902669392) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "eleph_c.u22", 0x00000, 0x10000, CRC(4b909bf3) SHA1(a822b12126bc58af6d3f999ab2117370015a039b) )
	ROM_LOAD( "eleph_b.u21", 0x10000, 0x10000, CRC(e3612670) SHA1(beb65f7d2bd6d7bc68cfd876af51910cf6417bd0) )
	ROM_COPY( "gfx1",	 0x10000, 0x00000, 0x8000 ) /* rgn, srcoffset, offset, length */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_ef.u25",    0x0000, 0x0200, CRC(bca8b82a) SHA1(4aa19f5ecd9953bf8792dceb075a746f77c01cfc) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_ef.u5",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_ef.u22", 0x0200, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_ef.u23", 0x0400, 0x0157, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( elephfmb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "now.u2", 0x8000, 0x8000, CRC(7b537ce6) SHA1(b221d08c53b9e14178335632420e78070b9cfb27) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "elephb.u21", 0x0000, 0x8000, CRC(3c60549c) SHA1(c839b3ea415a877e5eac04e0522c342cce8d6e64) )
	ROM_LOAD( "elephc.u20", 0x8000, 0x8000, CRC(448ba955) SHA1(2785cbc8cd42a7dda85bd8b81d5fbec01a1ba0bd) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_ef.u25",    0x0000, 0x0200, CRC(bca8b82a) SHA1(4aa19f5ecd9953bf8792dceb075a746f77c01cfc) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_ef.u5",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_ef.u22", 0x0200, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_ef.u23", 0x0400, 0x0157, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( pool10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pool10.u2", 0x8000, 0x8000, CRC(4e928756) SHA1(d9ac3d41ea32e060a7e269502b7f22333c5e6c61) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25",    0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) /* GAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) /* GAL is read protected */
ROM_END

ROM_START( pool10b )	/* this set should be the parent */
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "u2.bin", 0x8000, 0x10000, CRC(64fee38e) SHA1(8a624a0b6eb4a3ba09e5b396dc5a01994dfdf294) )

	/* GFX ROMs are the same of pool10, but double sized with identical halves. */
	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u20.bin", 0x00000, 0x10000, CRC(3bdf1106) SHA1(fa21cbd49bb27ea4a784cf4e4b3fbd52650a285b) )
	ROM_LOAD( "u21.bin", 0x10000, 0x10000, CRC(581c4878) SHA1(5ae61af090feea1745e22f46b33b2c01e6013fbe) )
	ROM_COPY( "gfx1",	 0x10000, 0x00000, 0x8000 ) /* rgn, srcoffset, offset, length */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25",    0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10b.u5",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_p10b.u22", 0x0200, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_p10b.u23", 0x0400, 0x0157, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( pool10c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.u2", 0x8000, 0x8000, CRC(ac157b17) SHA1(f2b7eb940273bc404d3e0d8dd0f00ca757cebf69) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "b.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "c.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25",    0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) /* GAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) /* GAL is read protected */
ROM_END

ROM_START( pool10d )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "3.50.u2", 0x8000, 0x10000, CRC(4c68e1f4) SHA1(bbab63a18e0c041ce519daa32e12dd1b6a672dce) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(99c8c074) SHA1(f8082b08e895cbcd028a2b7cd961a7a2c8b2762c) )
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(9abedd0c) SHA1(f184a82e8ec2387069d631bcb77e890acd44b3f5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25",    0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) /* GAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) /* GAL is read protected */
ROM_END

ROM_START( tortufam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tortu.a.u2", 0x8000, 0x8000, CRC(6e112184) SHA1(283ac534fc1cb33d11bbdf3447630333f2fc957f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "tortu.b.u21", 0x0000, 0x8000, CRC(e7b18584) SHA1(fa1c367469d4ced5d7c83c15a25ec5fd6afcca10) )
	ROM_LOAD( "tortu.c.u20", 0x8000, 0x8000, CRC(3cda6f73) SHA1(b4f3d2d3c652ebf6973358ae33b7808de5939acd) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc_tf.u25",    0x0000, 0x0200, CRC(c6d433fb) SHA1(065de832bbe8765eb0aacc2029e587a4f5362f8a) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce20v8h_tf.u5",  0x0000, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_tf.u22", 0x0200, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_tf.u23", 0x0400, 0x0157, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( potgame )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "now.u2", 0x8000, 0x8000, CRC(7b537ce6) SHA1(b221d08c53b9e14178335632420e78070b9cfb27) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "potg.b.u21", 0x0000, 0x8000, CRC(32fc1d4f) SHA1(cc533a44498338bc0cbb7c7b9c42559ce7ff1337) )
	ROM_LOAD( "potg.c.u20", 0x8000, 0x8000, CRC(0331eb42) SHA1(a8e838d644fc6d93a9632070b305e44e4242ce94) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29_pot.u25",    0x0000, 0x0200, CRC(a221f151) SHA1(270c57c9b7de912b136686bc6720eb8f12dbb805) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_pot.u5",  0x0000, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_pot.u22", 0x0200, 0x0157, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "palce20v8h_pot.u23", 0x0400, 0x0157, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( bottle10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "boat_3.bin", 0x8000, 0x8000, CRC(e2db8334) SHA1(22ac4ce361a93b7e6d491e260635755dd562b294) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "boat_2.bin", 0x0000, 0x8000, CRC(a6b36c3f) SHA1(90b12d9552ad5dbf11a30fc7451da1f3e6763cc3) )
	ROM_LOAD( "boat_1.bin", 0x8000, 0x8000, CRC(61fd8c19) SHA1(eb8fd8bd7de38a6c8a435e9e36daf699162138a5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25",    0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) /* GAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) /* GAL is read protected */
ROM_END

ROM_START( bottl10b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.u2", 0x8000, 0x8000, CRC(e2db8334) SHA1(22ac4ce361a93b7e6d491e260635755dd562b294) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u21", 0x0000, 0x8000, CRC(9395c15b) SHA1(e4caefc6f55b07f5c4370a3b8652fa93e08987ce) )
	ROM_LOAD( "1.u20", 0x8000, 0x8000, CRC(61fd8c19) SHA1(eb8fd8bd7de38a6c8a435e9e36daf699162138a5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147an_p10.u25",    0x0000, 0x0200, CRC(1de03d14) SHA1(d8eda20865c1d885a428931f4380032e103b252c) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h_p10.u5", 0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u22",  0x0200, 0x0157, NO_DUMP ) /* GAL is read protected */
	ROM_LOAD( "gal20v8b_p10.u23",  0x0400, 0x0157, NO_DUMP ) /* GAL is read protected */
ROM_END

ROM_START( royalcrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r2.bin", 0x8000, 0x8000, CRC(25dfe0dc) SHA1(1a857a910d0c34b6b5bfc2b6ea2e08ed8ed0cae0) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x8000, CRC(85e77661) SHA1(7d7a765c1bfcfeb9eb91d2519b22d734f20eab24) )
	ROM_LOAD( "1.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin",    0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h-4.bin",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "1-peel18cv8.bin",   0x0200, 0x0155, NO_DUMP ) /* PEEL is read protected */
	ROM_LOAD( "2-peel18cv8.bin",   0x0400, 0x0155, CRC(8fdafd55) SHA1(fbb187ba682111648ea1586f400990cb81a3077a) )
ROM_END

ROM_START( royalcdb ) /* both halves have different programs. we're using the 2nd one */
	ROM_REGION( 0x20000, "maincpu", 0 )	/* 1st half prg is testing RAM in offset $8600-$BF00...?? */
	ROM_LOAD( "rc.bin", 0x10000, 0x10000, CRC(8a9a6dd6) SHA1(04c3f9f17d5404ac1414c51ef8f930df54530e72) )
	ROM_COPY( "maincpu",	0x18000, 0x8000, 0x8000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "1a.bin", 0x0000, 0x10000, CRC(8a66f22c) SHA1(67d6e8f8f5a0fd979dc498ba2cc67cf707ccdf95) )
	ROM_LOAD( "2a.bin", 0x10000, 0x10000, CRC(3af71cf8) SHA1(3a0ce0d0abebf386573c5936545dada1d3558e55) )
	ROM_COPY( "gfx1",	 0x10000, 0x00000, 0x8000 )	/* rgn, srcoffset, offset, length */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin",    0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce16v8h-4.bin",   0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( royalcdc )	/* encrypted program rom */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_1.bin", 0x8000, 0x8000, CRC(8cdcc978) SHA1(489b58760a7c8646399c8cdfb86ec4341823e7dd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "rc_3.bin", 0x0000, 0x8000, CRC(8612c6ed) SHA1(3306a252af479e0510f136020086015b60dce879) )
	ROM_LOAD( "rc_2.bin", 0x8000, 0x8000, CRC(7f934488) SHA1(c537a09ef7e88a81ee9c2e1d971b3caf9d3dba0e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",    0x0000, 0x0200, CRC(44dbf086) SHA1(43a2d615c00605db75a4fd4d57d9e056c0356f10) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8.bin",  0x0000, 0x0117, NO_DUMP )	/* not present in the set */
	ROM_LOAD( "1-peel18cv8p.bin",   0x0200, 0x0155, NO_DUMP )	/* not present in the set */
	ROM_LOAD( "2-peel18cv8p.bin",   0x0400, 0x0155, NO_DUMP )	/* not present in the set */
ROM_END

ROM_START( royalcdp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u16_m27c256b.bin", 0x8000, 0x8000, CRC(162996ff) SHA1(122c13ee9842e692d31490f216eb972df2321b7f) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u11_tms27c256.bin",0x0000, 0x8000, CRC(d6834c3a) SHA1(4b071b9826c086439b9763393b23c671261b3788) )
	ROM_LOAD( "u4_nmc27c256.bin", 0x8000, 0x8000, CRC(41f7a0b3) SHA1(9aff2b8832d2a4f868daa9849a0bfe5e44f88fc0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147a.bin",     0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )

	ROM_REGION( 0x0200, "plds", 0 )	/* correct PAL dump */
	ROM_LOAD( "palce16v8h_1.bin", 0x0000, 0x0117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
ROM_END

ROM_START( lluck3x3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "l3x3.bin", 0x8000, 0x8000, CRC(dbdb07ff) SHA1(6be43aa0b2c10d83373f20f477606cb031bc6dd9) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "l2.bin", 0x0000, 0x8000, CRC(8ca90a8f) SHA1(bc3db3f8c097f89eff488e3aca39bf24ff2b5cff) )
	ROM_LOAD( "l1.bin", 0x8000, 0x8000, CRC(beadc35c) SHA1(8a6a5954a827def8c4c3b904d8ee58a4bde53d85) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin",    0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END

ROM_START( lluck4x1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rk4x1.bin", 0x8000, 0x8000, CRC(37f8a355) SHA1(a6eb4c53464e373bdecbbaaf146f5f7cf66b4bcd) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "l2.bin", 0x0000, 0x8000, CRC(8ca90a8f) SHA1(bc3db3f8c097f89eff488e3aca39bf24ff2b5cff) )
	ROM_LOAD( "l1.bin", 0x8000, 0x8000, CRC(beadc35c) SHA1(8a6a5954a827def8c4c3b904d8ee58a4bde53d85) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147.bin",    0x0000, 0x0200, CRC(8bc86f48) SHA1(4c677ab9314a1f571e35104b22659e6811aeb194) )
ROM_END

ROM_START( magicrd2 )	/* Impera */
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "magicard.004", 0x0000, 0x8000,  CRC(f6e948b8) SHA1(7d5983015a508ab135ccbf69b7f3c526c229e3ef) ) /* only last 16kbyte visible? */
	ROM_LOAD( "magicard.01",  0x8000, 0x10000, CRC(c94767d4) SHA1(171ac946bdf2575f9e4a31e534a8e641597af519) ) /* 1st and 2nd half identical */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mc2gr2.bin",   0x0000, 0x8000, CRC(733da697) SHA1(45122c64d5a371ec91cecc67b7faf179078e714d) )
	ROM_LOAD( "mc2gr1.bin",   0x8000, 0x8000, CRC(2406b8d2) SHA1(e9d7a25562fd5adee351d7ef6ba02fff6aab021a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mc2-82s147.bin",	0x0000, 0x0200, CRC(aa91cc35) SHA1(79f9a755441500e618c4183f524f969fffd44100) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END

ROM_START( magicd2a )	/* for green TAB or Impera boards */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc2prgv1.bin", 0x8000, 0x8000,  CRC(7f759b70) SHA1(23a1a6e8eda57c4a90c51a970302f9a7bf590083) )
//    ROM_LOAD( "mc2prgv2.bin", 0x8000, 0x8000,  CRC(b0ed6b40) SHA1(7167e67608f1b0b1cd956c838dacc1310861cb4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mc2gr2.bin",   0x0000, 0x8000, CRC(733da697) SHA1(45122c64d5a371ec91cecc67b7faf179078e714d) )
	ROM_LOAD( "mc2gr1.bin",   0x8000, 0x8000, CRC(2406b8d2) SHA1(e9d7a25562fd5adee351d7ef6ba02fff6aab021a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mc2-82s147.bin",	0x0000, 0x0200, CRC(aa91cc35) SHA1(79f9a755441500e618c4183f524f969fffd44100) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END

ROM_START( magicd2b )	/* for blue TAB board (encrypted)*/
	ROM_REGION( 0x10000, "maincpu", 0 )
//    ROM_LOAD( "mc2prgv1.bin", 0x8000, 0x8000,  CRC(7f759b70) SHA1(23a1a6e8eda57c4a90c51a970302f9a7bf590083) )
	ROM_LOAD( "mc2prgv2.bin", 0x8000, 0x8000,  CRC(b0ed6b40) SHA1(7167e67608f1b0b1cd956c838dacc1310861cb4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "mc2gr1b.bin",  0x0000, 0x8000, CRC(ce2629a7) SHA1(84767ed5da8dcee44a210255537e10372bcc264b) )
	ROM_LOAD( "mc2gr2b.bin",  0x8000, 0x8000, CRC(d2bf8bde) SHA1(975b8f43a0396c09e357b96d5ae7381b12152b9e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mc2-82s147.bin",	0x0000, 0x0200, CRC(aa91cc35) SHA1(79f9a755441500e618c4183f524f969fffd44100) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8uni.bin", 0x0000, 0x0117, CRC(b81d7e0a) SHA1(7fef0b2bcea931a830d38ae0f1102434cf281d2d) )
ROM_END

ROM_START( vegasslw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vslow.bin", 0x8000, 0x8000, CRC(9cb7861a) SHA1(f934eacd5b3573a6bbeaa827f521b4a498e5bcdf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "v2.bin", 0x0000, 0x8000, CRC(af7ab460) SHA1(01ea400424152c09c10eb83a1bd569019969ccb7) )
	ROM_LOAD( "v1.bin", 0x8000, 0x8000, CRC(23e0d1c6) SHA1(98967b14d3264c444a1dfbd15c57cde70f41f09d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( vegasfst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vfast.bin", 0x8000, 0x8000, CRC(87dfb28d) SHA1(9a06e695e59722b6c97e5a9fd2c8b238661e5a4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "v2.bin", 0x0000, 0x8000, CRC(af7ab460) SHA1(01ea400424152c09c10eb83a1bd569019969ccb7) )
	ROM_LOAD( "v1.bin", 0x8000, 0x8000, CRC(23e0d1c6) SHA1(98967b14d3264c444a1dfbd15c57cde70f41f09d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( vegasfte )	/* Royal Vegas Joker Card (fast deal, english gfx) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ch3.bin", 0x8000, 0x8000, CRC(87dfb28d) SHA1(9a06e695e59722b6c97e5a9fd2c8b238661e5a4a) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ch2.bin", 0x0000, 0x8000, CRC(af7ab460) SHA1(01ea400424152c09c10eb83a1bd569019969ccb7) )
	ROM_LOAD( "ch1.bin", 0x8000, 0x8000, CRC(0a3679c0) SHA1(ce8a067e1a2eccf9fabb16733ef3a14e0e8129e5) )	/* X & Y in txt layer */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolyjokr )	/* Jolly Joker (Impera, original) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impera3orig.bin", 0x8000, 0x8000, CRC(ceb3a0d5) SHA1(25efae9f225abddfa663e6abcc91801996e5b0ea) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "impera2.bin", 0x0000, 0x8000, BAD_DUMP CRC(57c1ca04) SHA1(9d3e1f50795ebe5997532b67ba467fd6428e81d2) )
	ROM_LOAD( "impera1.bin", 0x8000, 0x8000, CRC(8e354c4a) SHA1(95d7edf83bb07b5c58312af6d8a7dff1cf87cba7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jolyjokra )	/* Jolly Joker (Impera, 50bet) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "impera50.bin", 0x8000, 0x8000, CRC(7effc044) SHA1(961438e7fb8222296fb959b510cdf646e4ac3226) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "impera2.bin", 0x0000, 0x8000, BAD_DUMP CRC(57c1ca04) SHA1(9d3e1f50795ebe5997532b67ba467fd6428e81d2) )
	ROM_LOAD( "impera1.bin", 0x8000, 0x8000, CRC(8e354c4a) SHA1(95d7edf83bb07b5c58312af6d8a7dff1cf87cba7) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.bin",	0x0000, 0x0200, CRC(5ebc5659) SHA1(8d59011a181399682ab6e8ed14f83101e9bfa0c6) )
ROM_END

ROM_START( jokercrd )
/*  (Multi) Joker Card from Vesely Svet (Sprightly World). Czech poker game.
    Program roms seems encrypted. Custom Funworld CPU based on 6502 family.
    Seems to be a Big Deal clone.
*/
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "ic41.bin",   0x8000,  0x8000, CRC(d36188b3) SHA1(3fb848fabbbde9fbb70875b3dfef62bfb3a8cbcb) ) /* only last 16kbyte visible? */
	ROM_LOAD( "ic37.bin",   0x10000, 0x8000, CRC(8e0d70c4) SHA1(018f92631acbe98e5826a41698f0e07b4b46cd71) ) /* 1st and 2nd half identical */
	ROM_COPY( "maincpu",	0x10000, 0xc000, 0x4000 ) /* rgn, srcoffset, offset, length */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "ic10.bin",	0x0000, 0x8000, CRC(2bbd27ad) SHA1(37d37899398d95beac5f3cbffc4277c97aca1a23) )
	ROM_LOAD( "ic11.bin",	0x8000, 0x8000, CRC(21d05a57) SHA1(156c18ec31b08e4c4af6f73b49cb5d5c68d1670f) ) /* bad dump?, or sprite plane */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic13.bin",	0x0000, 0x0200, CRC(e59fc06e) SHA1(88a3bb89f020fe2b20f768ca010a082e0b974831) )
ROM_END

ROM_START( mongolnw )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prgteov.2.3m.u16", 0x10000, 0x10000, CRC(996b851a) SHA1(ef4e3d036ca10b33c83749024d04c4d4c09feeb7) )
	ROM_COPY( "maincpu",	0x18000, 0x8000, 0x8000 )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* TSC87C52-16CB MCU Code */
	ROM_LOAD( "tsc87c52-mf.u40", 0x00000, 0x02000 , CRC(ae22e778) SHA1(0897e05967d68d7f23489e98717663e3a3176070) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "mong.rc.b2.u3", 0x00000, 0x10000, CRC(5e019b73) SHA1(63a544dccb9589e5a6b938e604c09d4d8fc060fc) )
	ROM_LOAD( "mong.rc.c1.u2", 0x10000, 0x10000, CRC(e3fc24c4) SHA1(ea4e67ace63b55a76365f7e11a67c7d420a52dd7) )
	ROM_COPY( "gfx1",	 0x10000, 0x8000, 0x8000 ) /* rgn, srcoffset, offset, length */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc_mf.u24",    0x0000, 0x0200, CRC(da9181af) SHA1(1b30d992f3b2a4b3bd81e3f99632311988e2e8d1) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce16v8h_mf.u5",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( soccernw )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prgteo2gv2.3.u16", 0x10000, 0x10000, CRC(c61d1937) SHA1(c516f13a108da60b7ccee338b63a025009ef9099) )
	ROM_COPY( "maincpu",	0x18000, 0x8000, 0x8000 )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* TSC87C52-16CB MCU Code */
	ROM_LOAD( "tsc87c52-sn.u40", 0x00000, 0x02000 , CRC(af0bd35b) SHA1(c6613a7bcdec2fd6060d6dcf639654568de87e75) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "soccer2.u3", 0x00000, 0x10000, CRC(db09b5bb) SHA1(a12bf2938f5482ea5ebc0db6fd6594e1beb97017) )
	ROM_LOAD( "soccer1.u2", 0x10000, 0x10000, CRC(564cc467) SHA1(8f90c4bacd97484623666b25dae77e628908e243) )
	ROM_COPY( "gfx1",	 0x10000, 0x8000, 0x8000 ) /* rgn, srcoffset, offset, length */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc_sn.u24",    0x0000, 0x0200, CRC(d02894fc) SHA1(adcdc912cc0b7a7f67b122fa94fca921c957b282) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce16v8h_sn.u5",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( saloon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1s.bin", 0x8000, 0x8000, CRC(66938330) SHA1(09118d607eff7121472db7d2cc24079e063dc7cf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2s.bin", 0x0000, 0x8000, CRC(39a792d5) SHA1(45c956a4a33587238a24eed602039115db1bb4b6) )
	ROM_LOAD( "3s.bin", 0x8000, 0x8000, CRC(babc0964) SHA1(f084465cc34ea7ac19091d3e75ef7d55c48273ae) )

    /* looks strange */
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147_saloon.bin", 0x0000, 0x0200, CRC(f424ccc1) SHA1(6df1215f58cca786e9f0ea4bf35407cf7fe21d83) )
ROM_END


/**************************
*  Driver Initialization  *
**************************/

static DRIVER_INIT( tabblue )
{
/****************************************************************************************************

   +-------------------------+
   | Blue TAB PCB Decryption |
   +-------------------------+

    It perform by byte nibble a boolean XOR against the same value shifted to the right, then shift
    the result to the left carring the less significant bit and losing the most significant one.


    Encrypted nibble:  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    Bits:             0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111
    -------------------------------------------------------------------------------------------------
    Bits:             0000 0011 0110 0101 1100 1111 1010 1001 1000 1011 1110 1101 0100 0111 0010 0001
    Decrypted nibble:  0    3    6    5    C    F    A    9    8    B    E    D    4    7    2    1


*****************************************************************************************************/

	int x, na, nb, nad, nbd;
	UINT8 *src = memory_region( machine, "gfx1" );


	for (x=0x0000; x < 0x10000; x++)
	{
		na = src[x] & 0xf0;		/* nibble A */
		nb = src[x] << 4;		/* nibble B */

			nad = (na ^ (na >> 1)) << 1;			/* nibble A decrypted */
			nbd = ((nb ^ (nb >> 1)) >> 3) & 0x0f;	/* nibble B decrypted */

		src[x] = nad + nbd;		/* decrypted byte */
	}
}

static DRIVER_INIT( magicd2a )
/*****************************************************************

  For a serie of Mexican Rockwell's 65c02
  seems that opcode 0x91 is STA ($zp) instead of STA ($zp),y
  ...or is patched with the correct opcode (0x92) by PLDs...

  In offset $C1C4, the code is trying to initialize both PIAs
  putting value 0x34 in $0800-$0803 & $0A00-$0A03.

  The code use STA ($zp),y (opcode 0x91). As soon as register 'y'
  increments, almost all writes goes out of range.

******************************************************************/
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	ROM[0xc1c6] = 0x92;
}

static DRIVER_INIT( magicd2b )
/*** same as blue TAB PCB, with the magicd2a patch ***/
{
	int x, na, nb, nad, nbd;
	UINT8 *src = memory_region( machine, "gfx1" );
	UINT8 *ROM = memory_region(machine, "maincpu");

	for (x=0x0000; x < 0x10000; x++)
	{
		na = src[x] & 0xf0;		/* nibble A */
		nb = src[x] << 4;		/* nibble B */

			nad = (na ^ (na >> 1)) << 1;			/* nibble A decrypted */
			nbd = ((nb ^ (nb >> 1)) >> 3) & 0x0f;	/* nibble B decrypted */

		src[x] = nad + nbd;		/* decrypted byte */
	}

	ROM[0xc1c6] = 0x92;
}

static DRIVER_INIT( soccernw )
{
/* temporary patch to avoid hardware errors for debug purposes */
	UINT8 *ROM = memory_region(machine, "maincpu");

	ROM[0x80b2] = 0xa9;
	ROM[0x80b3] = 0x00;

//  DEBUG
//  run to $810a

//  ROM[0xa33a] = 0xea;
//  ROM[0xa33b] = 0xea;
//  ROM[0xa33c] = 0xea;
}

static DRIVER_INIT( saloon )
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
    Still trying....


*************************************************/
{
	UINT8 *rom = memory_region(machine, "maincpu");
	int size = memory_region_length(machine, "maincpu");
	int start = 0x8000;

	UINT8 *gfxrom = memory_region(machine, "gfx1");
	int sizeg = memory_region_length(machine, "gfx1");
	int startg = 0;

	UINT8 *prom = memory_region(machine, "proms");
	int sizep = memory_region_length(machine, "proms");
	int startp = 0;

	UINT8 *buffer;
	int i, a;

    /*****************************
    *   Program ROM decryption   *
    *****************************/

	/* data lines swap: 76543210 -> 76543012 */

	for (i = start; i < size; i++)
	{
		rom[i] = BITSWAP8(rom[i], 7, 6, 5, 4, 3, 0, 1, 2);
	}

	buffer = auto_alloc_array(machine, UINT8, size);
	memcpy(buffer, rom, size);


	/* address lines swap: fedcba9876543210 -> fedcba9820134567 */

	for (i = start; i < size; i++)
	{
		a = ((i & 0xff00) | BITSWAP8(i & 0xff, 2, 0, 1, 3, 4, 5, 6, 7));
		rom[a] = buffer[i];
	}

	auto_free(machine, buffer);


    /******************************
    *   Graphics ROM decryption   *
    ******************************/

	buffer = auto_alloc_array(machine, UINT8, sizeg);
	memcpy(buffer, gfxrom, sizeg);

	/* address lines swap: fedcba9876543210 -> fedcb67584a39012 */

	for (i = startg; i < sizeg; i++)
	{
		a = BITSWAP16(i, 15, 14, 13, 12, 11, 6, 7, 5, 8, 4, 10, 3, 9, 0, 1, 2);
		gfxrom[a] = buffer[i];
	}

	auto_free(machine, buffer);


    /****************************
    *   Color PROM decryption   *
    ****************************/

	/* data lines swap: 76543210 -> 23546710 */

	for (i = startp; i < sizep; i++)
	{
		prom[i] = BITSWAP8(prom[i], 2, 3, 5, 4, 6, 7, 1, 0);
	}

	buffer = auto_alloc_array(machine, UINT8, sizep);
	memcpy(buffer, prom, sizep);


	/* address lines swap: fedcba9876543210 -> fedcba9487652013 */

	for (i = startp; i < sizep; i++)
	{
		a = BITSWAP16(i, 15, 14, 13, 12, 11, 10, 9, 4, 8, 7, 6, 5, 2, 0, 1, 3);
		prom[a] = buffer[i];
	}

	auto_free(machine, buffer);

}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT    MACHINE   INPUT     INIT      ROT    COMPANY            FULLNAME                                          FLAGS  */
GAME( 1985, jollycrd, 0,        funworld, funworld, 0,        ROT0, "TAB-Austria",     "Jolly Card (austrian)",                           0 )
GAME( 1985, jolyc3x3, jollycrd, funworld, funworld, 0,        ROT0, "TAB-Austria",     "Jolly Card (3x3 deal)",                           0 )
GAME( 2000, jolyc980, jollycrd, jolyc980, jolyc980, 0,        ROT0, "Spale-Soft",      "Jolly Card Professional 2.0",                     0 )
GAME( 1998, jolycdev, jollycrd, funworld, funworld, 0,        ROT0, "TAB/Evona",       "Jolly Card (Evona Electronic)",                   0 )
GAME( 1985, jolyccra, jollycrd, cuoreuno, jolycdcr, 0,        ROT0, "TAB-Austria",     "Jolly Card (croatian, set 1)",                    0 )
GAME( 1993, jolyccrb, jollycrd, cuoreuno, jolycdcr, 0,        ROT0, "Soft Design",     "Jolly Card (croatian, set 2)",                    0 )
GAME( 199?, jolycdit, jollycrd, cuoreuno, jolycdit, tabblue,  ROT0, "bootleg",         "Jolly Card (italian, blue TAB board, encrypted)", 0 )
GAME( 1990, jolycdib, jollycrd, cuoreuno, jolycdib, tabblue,  ROT0, "bootleg",         "Jolly Card (italian, encrypted bootleg)",         0 )	/* not a real TAB blue PCB */
GAME( 1985, sjcd2kx3, jollycrd, funworld, funworld, 0,        ROT0, "M.P.",            "Super Joly 2000 - 3x",                            0 )
GAME( 1986, jolycdab, jollycrd, funworld, funworld, 0,        ROT0, "Inter Games",     "Jolly Card (austrian, Funworld, bootleg)",        GAME_NOT_WORKING )
GAME( 1986, bigdeal,  0,        funworld, bigdeal,  0,        ROT0, "Funworld",        "Big Deal (hungarian, set 1)",                     GAME_IMPERFECT_COLORS )
GAME( 1986, bigdealb, bigdeal,  funworld, bigdeal,  0,        ROT0, "Funworld",        "Big Deal (hungarian, set 2)",                     GAME_IMPERFECT_COLORS )
GAME( 1986, jolycdat, bigdeal,  funworld, bigdeal,  0,        ROT0, "Funworld",        "Jolly Card (austrian, Funworld)",                 GAME_IMPERFECT_COLORS )
GAME( 1996, cuoreuno, 0,        cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Cuore 1 (italian)",                               0 )
GAME( 1997, elephfam, 0,        cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Elephant Family (italian, new)",                  0 )
GAME( 1996, elephfmb, elephfam, cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Elephant Family (italian, old)",                  0 )
GAME( 1996, pool10,   0,        cuoreuno, pool10,   0,        ROT0, "C.M.C.",          "Pool 10 (italian, set 1)",                        0 )
GAME( 1996, pool10b,  pool10,   cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Pool 10 (italian, set 2)",                        0 )
GAME( 1996, pool10c,  pool10,   cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Pool 10 (italian, set 3)",                        0 )
GAME( 1997, pool10d,  pool10,   cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Pool 10 (italian, set 4)",                        0 )
GAME( 1997, tortufam, 0,        cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Tortuga Family (italian)",                        0 )
GAME( 1996, potgame,  0,        cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Pot Game (italian)",                              0 )
GAME( 1996, bottle10, 0,        cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Bottle 10 (italian, set 1)",                      0 )
GAME( 1996, bottl10b, bottle10, cuoreuno, cuoreuno, 0,        ROT0, "C.M.C.",          "Bottle 10 (italian, set 2)",                      0 )
GAME( 1991, royalcrd, 0,        royalcrd, royalcrd, 0,        ROT0, "TAB-Austria",     "Royal Card (austrian, set 1)",                    0 )
GAME( 1991, royalcdb, royalcrd, royalcrd, royalcrd, 0,        ROT0, "TAB-Austria",     "Royal Card (austrian, set 2)",                    0 )
GAME( 1991, royalcdc, royalcrd, royalcrd, royalcrd, 0,        ROT0, "Evona Electronic","Royal Card (slovak, encrypted)",                  GAME_WRONG_COLORS | GAME_NOT_WORKING )
GAME( 1993, royalcdp, royalcrd, cuoreuno, royalcrd, 0,        ROT0, "Digital Dreams",  "Royal Card v2.0 Professional",                    GAME_NOT_WORKING )
GAME( 1991, lluck3x3, royalcrd, cuoreuno, royalcrd, 0,        ROT0, "TAB-Austria",     "Lucky Lady (3x3 deal)",                           0 )
GAME( 1991, lluck4x1, royalcrd, royalcrd, royalcrd, 0,        ROT0, "TAB-Austria",     "Lucky Lady (4x1 aces)",                           0 )
GAME( 1996, magicrd2, 0,        magicrd2, magicrd2, 0,        ROT0, "Impera",          "Magic Card II (bulgarian)",                       GAME_IMPERFECT_SOUND )
GAME( 1996, magicd2a, magicrd2, magicrd2, magicrd2, magicd2a, ROT0, "Impera",          "Magic Card II (green TAB or Impera board)",       GAME_NOT_WORKING )
GAME( 1996, magicd2b, magicrd2, magicrd2, magicrd2, magicd2b, ROT0, "Impera",          "Magic Card II (blue TAB board, encrypted)",       GAME_NOT_WORKING )
GAME( 1993, vegasslw, 0,        funworld, funworld, 0,        ROT0, "Funworld",        "Royal Vegas Joker Card (slow deal)",              0 )
GAME( 1993, vegasfst, vegasslw, funworld, funworld, 0,        ROT0, "Soft Design",     "Royal Vegas Joker Card (fast deal)",              0 )
GAME( 1993, vegasfte, vegasslw, funworld, funworld, 0,        ROT0, "Soft Design",     "Royal Vegas Joker Card (fast deal, english gfx)", 0 )
GAME( 198?, jolyjokr, 0,        funworld, funworld, 0,        ROT0, "Impera",          "Jolly Joker",                                     0 )
GAME( 198?, jolyjokra,jolyjokr, funworld, jolyjokra,0,        ROT0, "Impera",          "Jolly Joker (50bet)",                             0 )
GAME( 1993, jokercrd, 0,        funworld, funworld, 0,        ROT0, "Vesely Svet",     "Joker Card (Ver.A267BC, encrypted)",              GAME_WRONG_COLORS | GAME_NOT_WORKING )
GAME( 199?, mongolnw, 0,        royalmcu, royalcrd, 0,        ROT0, "bootleg",         "Mongolfier New (italian)",                        GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 199?, soccernw, 0,        royalcrd, royalcrd, soccernw, ROT0, "bootleg",         "Soccer New (italian)",                            GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 198?, saloon,   0,        saloon,   saloon,   saloon,   ROT0, "<unknown>",       "Saloon (french, encrypted)",                      GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
