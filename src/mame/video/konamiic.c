/***************************************************************************

TODO:
- It seems shadows can both affect underlying sprites and not. This is currently
  hardcoded in the drivers; there might be a control bit somewhere.
  Games requiring shadows to affect sprites behind them:
  - Surprise Attack (dark glass walls in level 3)
  - 88 Games (angle indicator in the long jump event).
  - Sunset Riders (bull's eye in the saloon cutscene)
  Games requiring shadows to NOT affect sprites behind them:
  - Asterix (Asterix's shadow would be over his feet otherwise)
  - X-Men is dubious, see enemies halfway through level 1 coming from above with
    boulders over their heads.

- scrollcontrol = 30 in Golfing Greats (leader board)

- detatwin: sprites are left on screen during attract mode


                      Emulated
                         |
                  board #|year    CPU      tiles        sprites  priority palette    other
                    -----|---- ------- ------------- ------------- ------ ------ ----------------
Hyper Crash         GX401 1985                   GX400
Twinbee             GX412*1985   68000           GX400
Yie Ar Kung Fu      GX407*1985    6809
Gradius / Nemesis   GX456*1985   68000           GX400
Shao-lins Road      GX477*1985    6809
Jail Break          GX507*1986 KONAMI-1          005849                   PROMs
Finalizer           GX523*1985 KONAMI-1          005885                   PROMs
Konami's Ping Pong  GX555*1985     Z80
Iron Horse          GX560*1986    6809           005885                   PROMs
Konami GT           GX561*1985   68000           GX400
Green Beret         GX577*1985     Z80           005849                   PROMs
Galactic Warriors   GX578*1985   68000           GX400
Salamander          GX587*1986   68000           GX400
WEC Le Mans 24      GX602*1986 2x68000
BAW / Black Panther GX604*1987   68000           GX400                    007593
Combat School /     GX611*1987    6309           007121(x2)               007327
  Boot Camp
Rock 'n Rage /      GX620*1986    6309 007342        007420               007327
  Koi no Hotrock
Mr Kabuki/Mr Goemon GX621*1986     Z80           005849
Jackal              GX631*1986    6809           005885(x2)               007327 007343 (address decoder)
Contra / Gryzor     GX633*1987    6809?          007121(x2)               007593
Flak Attack         GX669*1987    6309           007121                   007327 007452
Devil World / Dark  GX687*1987 2x68000           TWIN16
  Adventure / Majuu no Oukoku
Double Dribble      GX690*1986  3x6809           005885(x2)               007327 007452
Kitten Kaboodle /   GX712*1988                   GX400                    007593 051550
  Nyan Nyan Panic
Chequered Flag      GX717*1988  052001               051960 051937(x2)           051316(x2) (roz) 051733 (protection)
Fast Lane           GX752*1987    6309           007121                          051733 (protection) 007801
Hot Chase           GX763*1988 2x68000                                           051316(x3) (roz) 007634 007635 007558 007557
Rack 'Em Up /       GX765*1987    6309 007342        007420               007327 007324
  The Hustler
Haunted Castle      GX768*1988  052001           007121(x2)               007327
Ajax / Typhoon      GX770*1987   6309+ 052109 051962 051960 051937  PROM  007327 051316 (roz)
                                052001
Labyrinth Runner /  GX771*1987    6309           007121                   007593 051733 (protection) 051550
  Trick Trap
Super Contra        GX775*1988  052001 052109 051962 051960 051937  PROM  007327
Battlantis          GX777*1987    6309 007342        007420               007327 007324
Vulcan Venture /    GX785*1988 2x68000           TWIN16
  Gradius 2
City Bomber         GX787*1987   68000           GX400                    007593 051550
Over Drive          GX789*1990 2x68000               053247 053246 053251 051316(x2) (roz) 053249 053250(x2) (road) 053252(*)
Hyper Crash         GX790 1987
Blades of Steel     GX797*1987    6309 007342        007420               007327 051733 (protection)
The Main Event      GX799*1988    6309 052109 051962 051960 051937  PROM
Missing in Action   GX808*1989   68000 052109 051962 051960 051937  PROM
Missing in Action J GX808*1989 2x68000           TWIN16
Crime Fighters      GX821*1989  052526 052109 051962 051960 051937  PROM
Special Project Y   GX857*1989    6309 052109 051962 051960 051937  PROM         052591 (protection)
'88 Games           GX861*1988  052001 052109 051962 051960 051937  PROM         051316 (roz)
Final Round /       GX870*1988 1x68000           TWIN16?
  Hard Puncher
Thunder Cross       GX873*1988  052001 052109 051962 051960 051937  PROM  007327 052591 (protection)
Aliens              GX875*1990  052526 052109 051962 051960 051937  PROM
Gang Busters        GX878*1988  052526 052109 051962 051960 051937  PROM
Devastators         GX890*1988    6309 052109 051962 051960 051937  PROM         007324 051733 (protection)
Bottom of the Ninth GX891*1989    6809 052109 051962 051960 051937  PROM         051316 (roz)
Cue Brick           GX903*1989   68000 052109 051962 051960 051937  PROM
Cue Brick           GX903*1989 2x68000           TWIN16
Punk Shot           GX907*1990   68000 052109 051962 051960 051937 053251
Ultraman            GX910*1991   68000 ------ ------ 051960 051937  PROM         051316(x3) (roz) 051550
Surprise Attack     GX911*1990  053248 052109 051962 053245 053244 053251
Lightning Fighters /GX939*1990   68000 052109 051962 053245 053244 053251
  Trigon
Gradius 3           GX945*1989 2x68000 052109 051962 051960 051937  PROM
Parodius            GX955*1990  053248 052109 051962 053245 053244 053251
TMNT                GX963*1989   68000 052109 051962 051960 051937  PROM
Block Hole          GX973*1989  052526 052109 051962 051960 051937  PROM
Escape Kids         GX975*1991  053248 052109 051962 053247 053246 053251        053252(*)
Rollergames         GX999*1991  053248 ------ ------ 053245 053244               051316 (roz) 053252(*)
Bells & Whistles /  GX060*1991   68000 052109 051962 053245 053244 053251        054000 (collision)
  Detana!! Twin Bee
Golfing Greats      GX061*1991   68000 052109 051962 053245 053244 053251        053936 (roz+)
TMNT 2              GX063*1991   68000 052109 051962 053245 053244 053251        053990 (protection) 051550
Sunset Riders       GX064*1991   68000 052109 051962 053245 053244 053251        054358
X-Men               GX065*1992   68000 052109 051962 053247 053246 053251        054539 (sound)
XEXEX               GX067*1991   68000 054157 054156 053247 053246 053251        053250?("road") 054338 (alpha blending) 054539 (sound)
Asterix             GX068*1992   68000 054157 054156 053245 053244 053251        054358
G.I. Joe            GX069*1992   68000 054157 054156 053247 053246 053251        054539 (sound)
The Simpsons        GX072*1991  053248 052109 051962 053247 053246 053251
Thunder Cross 2     GX073*1991   68000 052109 051962 051960 051937 053251        054000 (collision)
Vendetta /          GX081*1991  053248 052109 051962 053247 053246 053251        054000 (collision)
  Crime Fighters 2
Premier Soccer      GX101*1993   68000 052109 051962 053245 053244 053251        053936 (roz+) 054986
Hexion              GX122*1992     Z80                                           052591 (protection) 053252(*)
Entapous /          GX123*1993   68000 054157 054156 055673 053246 055555        053252(*) 054000 053936 (roz+)
  Gaiapolis
Mystic Warrior      GX128*1993   68000 054157 054156 055673 053246 055555        054338 (alpha blending) 053252(*) 054539(x2) (sound)
Cowboys of Moo Mesa GX151*1992   68000 054157 054156 053247 053246 053251        053252(*) 054338 (alpha blending) 053990 (protection)
Violent Storm       GX168*1993   68000 054157 054156 055673 053246 055555        054338 (alpha blending) 055550 054539(x2) (sound)
Monster Maulers /   GX170*1993   68000 054157 054156 055673 053246 055555        053252(*) 055550 054338 (alpha blending) 054539 (sound) 053936 (roz+)
  Ultimate Battler Dadandarn
Bucky 'O Hare       GX173*1992   68000 054157 054156 053247 053246 053251        054338 (alpha blending) 054539 (sound)
Potrio              GX174 1992
Lethal Enforcers    GX191*1992    6309 054157(x2) 054156 053245 053244(x2)       054000 054539 (sound)
Metamorphic Force   GX224*1993   68000 054157 054157 055673 053246 055555
Martial Champion    GX234*1993   68000 054157 054156 055673 053246 055555        053252(*) 054338 (alpha blending) 053990 054539 (sound)
Run and Gun         GX247*1993   68000 (TTL tilemap) 055673 053246               053253(x2) 053252(*) 053936 (roz+) 054539(x2) (sound)
Quiz Gakumon no     GX248*1993   68000 052109 051962 053245 053244 053251        053990 (protection) 051550 - same board as TMNT2
  Susume
Polygonet Commander GX305+1993   68020 (TTL tilemap)                             XC56156-40(3D DSP) 054009(x2) 054010(x2) 054539 (sound)
System GX (ver 1)   GX300*1993   68020 056832 054156 055673 053246 055555        054338 (alpha blending) 054539(x2) (sound) 053252(*) 053936 (optional on ROM board, roz+)
System GX (ver 2)   GX300*1995   68020 056832 058143 055673 058142 055555        058144 (alpha blending) 058141 (sound) 053252(*) 053936 (optional on ROM board, roz+)
Beatmania DJ Main   GX858+1996   68020 056832 058143 056766        055555        058144 (alpha blending) 058141 (sound) 053252(*)
Tail to Nose             *1989   68000          V-System                         051316 (roz)
F-1 Grand Prix           *1991 2x68000          V-System                         053936 (roz+)
F-1 Grand Prix Part II   *1992 2x68000          V-System                         053936 (roz+)
Lethal Crash Race        *1993   68000          V-System                         053936 (roz+)
Super Slams              *1995   68000          V-System                         053936 (roz+)
Blazing Tornado          *1991   68000            Metro                          053936 (roz+)
Dragonball Z 2           *1994   68000 054157 054156 053247 053246 053251(x2)    053936(x2) (roz+) 053252(*)


Notes:
* 053252 seems to be just a timing/interrupt controller (see Vendetta schematics).

- Old games use 051961 instead of 052109, it is an earlier version functionally
  equivalent (maybe 052109 had bugs fixed). The list always shows 052109 because
  the two are exchangeable and 052109's are found also on original boards whose
  schematics show a 051961.

- Starting with the version 2 System GX mainboard, the following chip substitutions took place.
  All "new" chips are equivalent to their older counterparts, but are in a smaller package (and
  presumably are made on a smaller process).  The exception is the 058141, which is equivalent
  to 2 54539s (and yet takes less board space than even 1).

  058141 = 054539 (x2) (2 sound chips in one)
  058142 = 053246 (sprites)
  058143 = 054156 (tiles)
  058144 = 054338 (alpha blending)



Status of the ROM tests in the emulated games:

Chequered Flag      pass
Ajax / Typhoon      pass
Super Contra        pass
Over Drive          fails 16..20 (053250)
The Main Event      pass
Missing in Action   pass
Crime Fighters      pass
Special Project Y   pass
Konami 88           pass
Thunder Cross       pass
Aliens              pass
Gang Busters        pass
Devastators         pass
Bottom of the Ninth pass
Punk Shot           pass
Surprise Attack     fails D05-6 (052109) because it uses mirror addresses to
                    select banks, and supporting those addresses breaks the
                    normal game ;-(
Lightning Fighters  pass
Gradius 3           pass
Parodius            pass
TMNT                pass
Block Hole          pass
Escape Kids         pass
Rollergames         pass
Bells & Whistles    pass
Golfing Greats      pass
TMNT 2              pass
Sunset Riders       pass
X-Men               pass
The Simpsons        pass
Thunder Cross 2     pass
Xexex               pass
Asterix             pass
GiJoe               pass
Vendetta            pass
Premier Soccer      fails 16D 18D 18F (053936)
Hexion              pass
Run and Gun         fails 36M (053936) 2U 2Y 5U 5Y (sprites)
Quiz Gakumon no Susume  pass
Dragonball Z 2      fails


THE FOLLOWING INFORMATION IS PRELIMINARY AND INACCURATE. DON'T RELY ON IT.


005885
------
Some games use two of these in pair. Jackal even puts together the two 4bpp
tilemaps to form a single 8bpp one.
It manages sprites and 32x32 or 64x32 tilemap (only Double Dribble uses the
64x32 one).
The chip also generates clock and interrupt signals suitable for a 6809.
It uses 0x2000 bytes of RAM for the tilemaps and sprites, and an additional
0x100 bytes, maybe for scroll RAM and line buffers. The maximum addressable
ROM is 0x20000 bytes (addressed 16 bits at a time). Tile and sprite data both
come from the same ROM space. Double Dribble and Jackal have external circuitry
to extend the limits and use separated addressing spaces for sprites and tiles.
All games use external circuitry to reuse one or both the tile flip attributes
as an additional address bit.
Two 256x4 lookup PROMs are also used to increase the color combinations.
All tilemap / sprite priority handling is done internally and the chip exports
5 bits of color code, composed of 1 bit indicating tile or sprite, and 4 bits
of ROM data remapped through the PROM.

inputs:
- address lines (A0-A13)
- data lines (DB0-DB7)
- misc interface stuff
- data from the gfx ROMs (RDL0-RDL7, RDU0-RDU7)
- data from the tile lookup PROMs (VCD0-VCD3)
- data from the sprite lookup PROMs (OCD0-OCD3)

outputs:
- address lines for tilemap RAM (AX0-AX12)
- data lines for tilemap RAM (VO0-VO7)
- address lines for the small RAM (FA0-FA7)
- data lines for the small RAM (FD0-FD7)
- address lines for the gfx ROMs (R0-R15)
- address lines for the tile lookup PROMs (VCF0-VCF3, VCB0-VCB3)
- address lines for the sprite lookup PROMs (OCB0-OCB3, OCF0-OCF3)
- NNMI, NIRQ, NFIR, NCPE, NCPQ, NEQ for the main CPU
- misc interface stuff
- color code to be output on screen (COL0-COL4)


control registers
000:          scroll y
001:          scroll x (low 8 bits)
002: -------x scroll x (high bit)
     ----xxx- row/colscroll control
              000 = solid scroll (finalizr, ddribble bg)
              100 = solid scroll (jackal)
              001 = ? (ddribble fg)
              011 = colscroll (jackal high scores)
              101 = rowscroll (ironhors, jackal map)
003: ------xx high bits of the tile code
     -----x-- unknown (finalizr)
     ----x--- selects sprite buffer (and makes a copy to a private buffer?)
     --x----- unknown (ironhors)
     -x------ unknown (ironhors)
     x------- unknown (ironhors, jackal)
004: -------x nmi enable
     ------x- irq enable
     -----x-- firq enable
     ----x--- flip screen



007121
------
This is an interesting beast. It is an evolution of the 005885, with more
features. Many games use two of these in pair.
It manages sprites and two 32x32 tilemaps. The tilemaps can be joined to form
a single 64x32 one, or one of them can be moved to the side of screen, giving
a high score display suitable for vertical games.
The chip also generates clock and interrupt signals suitable for a 6809.
It uses 0x2000 bytes of RAM for the tilemaps and sprites, and an additional
0x100 bytes, maybe for scroll RAM and line buffers. The maximum addressable
ROM is 0x80000 bytes (addressed 16 bits at a time). Tile and sprite data both
come from the same ROM space.
Two 256x4 lookup PROMs are also used to increase the color combinations.
All tilemap / sprite priority handling is done internally and the chip exports
7 bits of color code, composed of 2 bits of palette bank, 1 bit indicating tile
or sprite, and 4 bits of ROM data remapped through the PROM.

inputs:
- address lines (A0-A13)
- data lines (DB0-DB7)
- misc interface stuff
- data from the gfx ROMs (RDL0-RDL7, RDU0-RDU7)
- data from the tile lookup PROMs (VCD0-VCD3)
- data from the sprite lookup PROMs (OCD0-OCD3)

outputs:
- address lines for tilemap RAM (AX0-AX12)
- data lines for tilemap RAM (VO0-VO7)
- address lines for the small RAM (FA0-FA7)
- data lines for the small RAM (FD0-FD7)
- address lines for the gfx ROMs (R0-R17)
- address lines for the tile lookup PROMs (VCF0-VCF3, VCB0-VCB3)
- address lines for the sprite lookup PROMs (OCB0-OCB3, OCF0-OCF3)
- NNMI, NIRQ, NFIR, NE, NQ for the main CPU
- misc interface stuff
- color code to be output on screen (COA0-COA6)


control registers
000:          scroll x (low 8 bits)
001: -------x scroll x (high bit)
     ------x- enable rowscroll? (combasc)
     ----x--- this probably selects an alternate screen layout used in combat
              school where tilemap #2 is overlayed on front and doesn't scroll.
              The 32 lines of the front layer can be individually turned on or
              off using the second 32 bytes of scroll RAM.
002:          scroll y
003: -------x bit 13 of the tile code
     ------x- unknown (contra)
     -----x-- might be sprite / tilemap priority (0 = sprites have priority)
              (combat school, contra, haunted castle(0/1), labyrunr)
     ----x--- selects sprite buffer (and makes a copy to a private buffer?)
     ---x---- screen layout selector:
              when this is set, 5 columns are added on the left of the screen
              (that means 5 rows at the top for vertical games), and the
              rightmost 2 columns are chopped away.
              Tilemap #2 is used to display the 5 additional columns on the
              left. The rest of tilemap #2 is not used and can be used as work
              RAM by the program.
              The visible area becomes 280x224.
              Note that labyrunr changes this at runtime, setting it during
              gameplay and resetting it on the title screen and crosshatch.
     --x----- might be sprite / tilemap priority (0 = sprites have priority)
              (combat school, contra, haunted castle(0/1), labyrunr)
     -x------ Chops away the leftmost and rightmost columns, switching the
              visible area from 256 to 240 pixels. This is used by combasc on
              the scrolling stages, and by labyrunr on the title screen.
              At first I thought that this enabled an extra bank of 0x40
              sprites, needed by combasc, but labyrunr proves that this is not
              the case
     x------- unknown (contra)
004: ----xxxx bits 9-12 of the tile code. Only the bits enabled by the following
              mask are actually used, and replace the ones selected by register
              005.
     xxxx---- mask enabling the above bits
005: selects where in the attribute byte to pick bits 9-12 of the tile code,
     output to pins R12-R15. The bit of the attribute byte to use is the
     specified bit (0-3) + 3, that is one of bits 3-6. Bit 7 is hardcoded as
     bit 8 of the code. Bits 0-2 are used for the color, however note that
     some games use bit 3 as well (see below).
     ------xx attribute bit to use for tile code bit  9
     ----xx-- attribute bit to use for tile code bit 10
     --xx---- attribute bit to use for tile code bit 11
     xx------ attribute bit to use for tile code bit 12
006: ----xxxx select additional effect for bits 3-6 of the tile attribute (the
              same ones indexed by register 005). Note that an attribute bit
              can therefore be used at the same time to be BOTH a tile code bit
              and an additional effect.
     -------x bit 3 of attribute is bit 3 of color (combasc, fastlane, flkatck)
     ------x- bit 4 of attribute is tile flip X (assumption - no game uses this)
     -----x-- bit 5 of attribute is tile flip Y (flkatck)
     ----x--- bit 6 of attribute is tile priority over sprites (combasc, hcastle,
              labyrunr)
              Note that hcastle sets this bit for layer 0, and bit 6 of the
              attribute is also used as bit 12 of the tile code, however that
              bit is ALWAYS set throughout the game.
              combasc uses the bit in the "graduation" scene during attract mode,
              to place soldiers behind the stand.
              Use in labyrunr has not been investigated yet.
     --xx---- palette bank (both tiles and sprites, see contra)
007: -------x nmi enable
     ------x- irq enable
     -----x-- firq enable
     ----x--- flip screen
     ---x---- unknown (contra, labyrunr)



007342
------
The 007342 manages 2 64x32 scrolling tilemaps with 8x8 characters, and
optionally generates timing clocks and interrupt signals. It uses 0x2000
bytes of RAM, plus 0x0200 bytes for scrolling, and a variable amount of ROM.
It cannot read the ROMs.

control registers
000: ------x- INT control
     ---x---- flip screen (TODO: doesn't work with thehustl)
001: Used for banking in Rock'n'Rage
002: -------x MSB of x scroll 1
     ------x- MSB of x scroll 2
     ---xxx-- layer 1 row/column scroll control
              000 = disabled
              010 = unknown (bladestl shootout between periods)
              011 = 32 columns (Blades of Steel)
              101 = 256 rows (Battlantis, Rock 'n Rage)
     x------- enable sprite wraparound from bottom to top (see Blades of Steel
              high score table)
003: x scroll 1
004: y scroll 1
005: x scroll 2
006: y scroll 2
007: not used


007420
------
Sprite generator. 8 bytes per sprite with zoom. It uses 0x200 bytes of RAM,
and a variable amount of ROM. Nothing is known about its external interface.



052109/051962
-------------
These work in pair.
The 052109 manages 3 64x32 scrolling tilemaps with 8x8 characters, and
optionally generates timing clocks and interrupt signals. It uses 0x4000
bytes of RAM, and a variable amount of ROM. It cannot read the ROMs:
instead, it exports 21 bits (16 from the tilemap RAM + 3 for the character
raster line + 2 additional ones for ROM banking) and these are externally
used to generate the address of the required data on the ROM; the output of
the ROMs is sent to the 051962, along with a color code. In theory you could
have any combination of bits in the tilemap RAM, as long as they add to 16.
In practice, all the games supported so far standardize on the same format
which uses 3 bits for the color code and 13 bits for the character code.
The 051962 multiplexes the data of the three layers and converts it into
palette indexes and transparency bits which will be mixed later in the video
chain.
Priority is handled externally: these chips only generate the tilemaps, they
don't mix them.
Both chips are interfaced with the main CPU. When the RMRD pin is asserted,
the CPU can read the gfx ROM data. This is done by telling the 052109 which
dword to read (this is a combination of some banking registers, and the CPU
address lines), and then reading it from the 051962.

052109 inputs:
- address lines (AB0-AB15, AB13-AB15 seem to have a different function)
- data lines (DB0-DB7)
- misc interface stuff

052109 outputs:
- address lines for the private RAM (RA0-RA12)
- data lines for the private RAM (VD0-VD15)
- NMI, IRQ, FIRQ for the main CPU
- misc interface stuff
- ROM bank selector (CAB1-CAB2)
- character "code" (VC0-VC10)
- character "color" (COL0-COL7); used foc color but also bank switching and tile
  flipping. Exact meaning depends on externl connections. All evidence indicates
  that COL2 and COL3 select the tile bank, and are replaced with the low 2 bits
  from the bank register. The top 2 bits of the register go to CAB1-CAB2.
  However, this DOES NOT WORK with Gradius III. "color" seems to pass through
  unaltered.
- layer A horizontal scroll (ZA1H-ZA4H)
- layer B horizontal scroll (ZB1H-ZB4H)
- ????? (BEN)

051962 inputs:
- gfx data from the ROMs (VC0-VC31)
- color code (COL0-COL7); only COL4-COL7 seem to really be used for color; COL0
  is tile flip X.
- layer A horizontal scroll (ZA1H-ZA4H)
- layer B horizontal scroll (ZB1H-ZB4H)
- let main CPU read the gfx ROMs (RMRD)
- address lines to be used with RMRD (AB0-AB1)
- data lines to be used with RMRD (DB0-DB7)
- ????? (BEN)
- misc interface stuff

051962 outputs:
- FIX layer palette index (DFI0-DFI7)
- FIX layer transparency (NFIC)
- A layer palette index (DSA0-DSAD); DSAA-DSAD seem to be unused
- A layer transparency (NSAC)
- B layer palette index (DSB0-DSBD); DSBA-DSBD seem to be unused
- B layer transparency (NSBC)
- misc interface stuff


052109 memory layout:
0000-07ff: layer FIX tilemap (attributes)
0800-0fff: layer A tilemap (attributes)
1000-1fff: layer B tilemap (attributes)
180c-1833: A y scroll
1a00-1bff: A x scroll
1c00     : ?
1c80     : row/column scroll control
           ------xx layer A row scroll
                    00 = disabled
                    01 = disabled? (gradius3, vendetta)
                    10 = 32 lines
                    11 = 256 lines
           -----x-- layer A column scroll
                    0 = disabled
                    1 = 64 (actually 40) columns
           ---xx--- layer B row scroll
           --x----- layer B column scroll
           surpratk sets this register to 70 during the second boss. There is
           nothing obviously wrong so it's not clear what should happen.
           glfgreat sets it to 30 when showing the leader board
1d00     : bits 0 & 1 might enable NMI and FIRQ, not sure
         : bit 2 = IRQ enable
1d80     : ROM bank selector bits 0-3 = bank 0 bits 4-7 = bank 1
1e00     : ROM subbank selector for ROM testing
1e80     : bit 0 = flip screen (applies to tilemaps only, not sprites)
         : bit 1 = set by crimfght, mainevt, surpratk, xmen, mia, punkshot, thndrx2, spy
         :         it seems to enable tile flip X, however flip X is handled by the
         :         051962 and it is not hardwired to a specific tile attribute.
         :         Note that xmen, punkshot and thndrx2 set the bit but the current
         :         drivers don't use flip X and seem to work fine.
         : bit 2 = enables tile flip Y when bit 1 of the tile attribute is set
1f00     : ROM bank selector bits 0-3 = bank 2 bits 4-7 = bank 3
2000-27ff: layer FIX tilemap (code)
2800-2fff: layer A tilemap (code)
3000-37ff: layer B tilemap (code)
3800-3807: nothing here, so the chip can share address space with a 051937
380c-3833: B y scroll
3a00-3bff: B x scroll
3c00-3fff: nothing here, so the chip can share address space with a 051960
3d80     : mirror of 1d80, but ONLY during ROM test (surpratk)
3e00     : mirror of 1e00, but ONLY during ROM test (surpratk)
3f00     : mirror of 1f00, but ONLY during ROM test (surpratk)
EXTRA ADDRESSING SPACE USED BY X-MEN:
4000-47ff: layer FIX tilemap (code high bits)
4800-4fff: layer A tilemap (code high bits)
5000-57ff: layer B tilemap (code high bits)

The main CPU doesn't have direct acces to the RAM used by the 052109, it has
to through the chip.



054156/054157
054156/056832
-------------

[Except for tilemap sizes, all numbers are in hex]

These work in pairs.  Similar in principle to the 052109/051962, they
manage 4 64x32 or 64x64 tilemaps.  They also handle linescroll on each
layer, and optional tile banking.  They use 4000 to 10000 bytes of
RAM, organized in 1000 or 2000 bytes banks.

The 56832 is a complete superset of the 54157 and supports higher color
depths (the 156/157 combo only goes to 5 bpp, the 156/832 combo goes to 8bpp).

These chips work in a fairly unusual way.  There are 4, 8, or 16 pages of VRAM, arranged
conceptually in a 4x4 2 dimensional grid.  Each page is a complete 64x32 tile tilemap.

The 4 physical tilemaps A, B, C, and, D are made up of these pages "glued together".
Each physical tilemap has an X and Y position in the 4x4 page grid indicating where
the page making up it's upper left corner is, as well as a width and height in pages.
If two tilemaps try to use the same page, the higher-letter one wins and the lower-letter
one is disabled completely.  E.g. A > B > C > D, so if A and B both try to use the
same page only A will be displayed.  Some games rely on this behavior to implicitly
disable tilemaps which otherwise should be displayed.

Tile encoding 2 bytes/tile (banks of 1000 bytes):
        pppx bbcc cccc cccc
  p = color palette
  x = flip x
  b = tile bank (0..3)
  c = tile code (0..3ff)


Tile encoding 4 bytes/tile (banks of 2000 bytes):
        ---- ---- pppp --yx  cccc cccc cccc cccc
  p = color palette
  x = flip x
  y = flip y
  b = tile bank (0..3)
  c = tile code (0..3ff)


Communication with these ics go through 4 memory zones:
  1000/2000 bytes: access to the currently selected ram bank
       2000 bytes: readonly access the the currently select tile
                   rom bank for rom checksumming
         40 bytes: writeonly access to the first register bank
          8 bytes: writeonly access to the second register bank

One of the register banks is probably on the 054156, and the other on
the 054157.

First register bank map (offsets in bytes, '-' means unused):
00    ---- ---- ??yx ????
  flip control

02    ---- ---- ???? ????
  unknown

04    ---- ---- ???? ????
  unknown (bit 1 may be bank count selection, 0 in xexex, 1 everywhere
  else)

06    ---- ---- ???? ???e
  enable irq

08    ---- ---- ???? ????
  unknown

0a    ---- ---- 3322 1100
  linescroll control, each pair of bits indicates the mode for the
  corresponding layer:
    0: per-line linescroll
    1: unused/unknown
    2: per-8 lines linescroll
    3: no linescroll

0c    ---- ---- ???? ????
  unknown (bit 1 may be bank size selection, 1 in asterix, 0 everywhere
  else)

0e    ---- ---- ---- ----

10-13 ---- ---- ---y y-hh
   layer Y position in the VRAM grid and height in pages

14-17 ---- ---- ---x x-ww
   layer X position in the VRAM grid and width in pages
18-1f ---- ---- ???? ????

20-27 yyyy yyyy yyyy yyyy
  scroll y position for each layer

28-2f xxxx xxxx xxxx xxxx
  scroll x position for each layer

30    ---- ---- ---b b--b
  linescroll ram bank selection

32    ---- ---- ---b b--b
  cpu-accessible ram bank selection

34    bbbb bbbb bbbb bbbb
  rom bank selection for checksumming (each bank is 0x2000 bytes)

36    ---- ---- ---- bbbb
  secondary rom bank selection for checksumming when tile banking is
  used

38    3333 2222 1111 0000
  tile banking look up table.  4 bits are looked up here for the two
  bits in the tile data.

3a    ???? ???? ???? ????
  unknown

3c    ???? ???? ???? ????
  unknown

3e    ---- ---- ---- ----


Second register bank map:
00    ---- ---- ???? ????
  unknown

02-07 are copies of the 02-07 registers from the first bank.


  Linescroll:

The linescroll is controlled by the register 0b, and uses the data in
the ram bank pointed by register 31.  The data for tilemap <n> starts
at offset 400*n in the bank for 1000 bytes ram banks, and 800*n+2 for
2000 bytes ram banks.  The scrolling information is a vector of half
words separated by 1 word padding for 2000 bytes banks.

This is a source-oriented linescroll, i.e. the first word is
associated to the first one of the tilemap, not matter what the
current scrolly position is.

In per-line mode, each word indicates the horizontal scroll of the
associated line.  Global scrollx is ignored.

In per-8 lines mode, each word associated to a line multiple of 8
indicates the horizontal scroll for that line and the 7 following
ones.  The other 7 words are ignored.  Global scrollx is ignored.



051960/051937
-------------
Sprite generators. Designed to work in pair. The 051960 manages the sprite
list and produces and address that is fed to the gfx ROMs. The data from the
ROMs is sent to the 051937, along with color code and other stuff from the
051960. The 051937 outputs up to 12 bits of palette index, plus "shadow" and
transparency information.
Both chips are interfaced to the main CPU, through 8-bit data buses and 11
bits of address space. The 051937 sits in the range 000-007, while the 051960
in the range 400-7ff (all RAM). The main CPU can read the gfx ROM data though
the 051937 data bus, while the 051960 provides the address lines.
The 051960 is designed to directly address 1MB of ROM space, since it produces
18 address lines that go to two 16-bit wide ROMs (the 051937 has a 32-bit data
bus to the ROMs). However, the addressing space can be increased by using one
or more of the "color attribute" bits of the sprites as bank selectors.
Moreover a few games store the gfx data in the ROMs in a format different from
the one expected by the 051960, and use external logic to reorder the address
lines.
The 051960 can also genenrate IRQ, FIRQ and NMI signals.

memory map:
000-007 is for the 051937, but also seen by the 051960
400-7ff is 051960 only
000     R  bit 0 = unknown, looks like a status flag or something
                   aliens waits for it to be 0 before starting to copy sprite data
                   thndrx2 needs it to pulse for the startup checks to succeed
000     W  bit 0 = irq enable/acknowledge?
           bit 2 = nmi enable?
           bit 3 = flip screen (applies to sprites only, not tilemaps)
           bit 4 = unknown, used by Devastators, TMNT, Aliens, Chequered Flag, maybe others
                   aliens sets it just after checking bit 0, and before copying
                   the sprite data
           bit 5 = enable gfx ROM reading
001     W  Devastators sets bit 1, function unknown.
           Ultraman sets the register to 0x0f.
           None of the other games I tested seem to set this register to other than 0.
002-003 W  selects the portion of the gfx ROMs to be read.
004     W  Aliens uses this to select the ROM bank to be read, but Punk Shot
           and TMNT don't, they use another bit of the registers above. Many
           other games write to this register before testing.
           It is possible that bits 2-7 of 003 go to OC0-OC5, and bits 0-1 of
           004 go to OC6-OC7.
004-007 R  reads data from the gfx ROMs (32 bits in total). The address of the
           data is determined by the register above and by the last address
           accessed on the 051960; plus bank switch bits for larger ROMs.
           It seems that the data can also be read directly from the 051960
           address space: 88 Games does this. First it reads 004 and discards
           the result, then it reads from the 051960 the data at the address
           it wants. The normal order is the opposite, read from the 051960 at
           the address you want, discard the result, and fetch the data from
           004-007.
400-7ff RW sprite RAM, 8 bytes per sprite



053245/053244
-------------
Sprite generators. The 053245 has a 16-bit data bus to the main CPU.
The sprites are buffered, a write to 006 activates to copy between the
main ram and the buffer.

053244 memory map (but the 053245 sees and processes them too):
000-001  W global X offset
002-003  W global Y offset
004      W unknown
005      W bit 0 = flip screen X
           bit 1 = flip screen Y
           bit 2 = unknown, used by Parodius
           bit 4 = enable gfx ROM reading
           bit 5 = unknown, used by Rollergames
006     RW accessing this register copies the sprite ram to the internal buffer
007      W unknown
008-009  W low 16 bits of the ROM address to read
00a-00b  W high bits of the ROM address to read.  3 bits for most games, 1 for asterix
00c-00f R  reads data from the gfx ROMs (32 bits in total). The address of the
           data is determined by the registers above; plus bank switch bits for
           larger ROMs.



053247/053246
-------------
Sprite generators. Nothing is known about their external interface.
The sprite RAM format is very similar to the 053245.

053246 memory map (but the 053247 sees and processes them too):
000-001 W  global X offset
002-003 W  global Y offset
004     W  low 8 bits of the ROM address to read
005     W  bit 0 = flip screen X
           bit 1 = flip screen Y
           bit 2 = unknown
           bit 4 = interrupt enable
           bit 5 = unknown
006-007 W  high 16 bits of the ROM address to read

???-??? R  reads data from the gfx ROMs (16 bits in total). The address of the
           data is determined by the registers above



051316
------
Manages a 32x32 tilemap (16x16 tiles, 512x512 pixels) which can be zoomed,
distorted and rotated.
It uses two internal 24 bit counters which are incremented while scanning the
picture. The coordinates of the pixel in the tilemap that has to be drawn to
the current beam position are the counters / (2^11).
The chip doesn't directly generate the color information for the pixel, it
just generates a 24 bit address (whose top 16 bits are the contents of the
tilemap RAM), and a "visible" signal. It's up to external circuitry to convert
the address into a pixel color. Most games seem to use 4bpp graphics, but Ajax
uses 7bpp.
If the value in the internal counters is out of the visible range (0..511), it
is truncated and the corresponding address is still generated, but the "visible"
signal is not asserted. The external circuitry might ignore that signal and
still generate the pixel, therefore making the tilemap a continuous playfield
that wraps around instead of a large sprite.

control registers
000-001 X counter starting value / 256
002-003 amount to add to the X counter after each horizontal pixel
004-005 amount to add to the X counter after each line (0 = no rotation)
006-007 Y counter starting value / 256
008-009 amount to add to the Y counter after each horizontal pixel (0 = no rotation)
00a-00b amount to add to the Y counter after each line
00c-00d ROM bank to read, used during ROM testing
00e     bit 0 = enable ROM reading (active low). This only makes the chip output the
                requested address: the ROM is actually read externally, not through
                the chip's data bus.
        bit 1 = unknown
        bit 2 = unknown
00f     unused



053936
------
Evolution of the 051316. The data bus is 16-bit instead of 8-bit.
When used in "simple" mode it can generate the same effects of the 051316, but it
doesn't have internal tilemap RAM, so it just generates a couple of X/Y coordinates
indicating the pixel to display at each moment. Therefore, the tilemap and tile
sizes are not fixed.
The important addition over the 051316 is 512x4 words of internal RAM used to control
rotation and zoom scanline by scanline instead that on the whole screen, allowing for
effects like linescroll (Super Slams) or 3D rotation of the tilemap (Golfing Greats,
Premier Soccer).

control registers
000 X counter starting value / 256
001 Y counter starting value / 256
002 ["simple" mode only] amount to add to the X counter after each line (0 = no rotation)
003 ["simple" mode only] amount to add to the Y counter after each line
004 ["simple" mode only] amount to add to the X counter after each horizontal pixel
005 ["simple" mode only] amount to add to the Y counter after each horizontal pixel (0 = no rotation)
006 x------- -------- when set, register (line*4)+2 must be multiplied by 256
    -x------ -------- when set, registers 002 and 003 must be multiplied by 256
    --xxxxxx -------- clipping for the generated address? usually 3F, Premier Soccer
                      sets it to 07 before penalty kicks
    -------- x------- when set, register (line*4)+3 must be multiplied by 256
    -------- -x------ when set, registers 004 and 005 must be multiplied by 256
    -------- --xxxxxx clipping for the generated address? usually 3F, Premier Soccer
                      sets it to 0F before penalty kicks
007 -------- -x------ enable "super" mode
    -------- --x----- unknown (enable address clipping from register 006?)
    -------- ---x---- unknown
    -------- ------x- (not sure) enable clipping with registers 008-00b
008 min x screen coordinate to draw to (only when enabled by register 7)
009 max x screen coordinate to draw to (only when enabled by register 7)
00a min y screen coordinate to draw to (only when enabled by register 7)
00b max y screen coordinate to draw to (only when enabled by register 7)
00c unknown
00d unknown
00e unknown
00f unknown

additional control from extra RAM:
(line*4)+0 X counter starting value / 256 (add to register 000)
(line*4)+1 Y counter starting value / 256 (add to register 001)
(line*4)+2 amount to add to the X counter after each horizontal pixel
(line*4)+3 amount to add to the Y counter after each horizontal pixel



053251
------
Priority encoder.

The chip has inputs for 5 layers (CI0-CI4); only 4 are used (CI1-CI4)
CI0-CI2 are 9(=5+4) bits inputs, CI3-CI4 8(=4+4) bits

The input connctions change from game to game. E.g. in Simpsons,
CI0 = grounded (background color)
CI1 = sprites
CI2 = FIX
CI3 = A
CI4 = B

in lgtnfght:
CI0 = grounded
CI1 = sprites
CI2 = FIX
CI3 = B
CI4 = A

there are three 6 bit priority inputs, PR0-PR2

simpsons:
PR0 = 111111
PR1 = xxxxx0 x bits coming from the sprite attributes
PR2 = 111111

lgtnfght:
PR0 = 111111
PR1 = 1xx000 x bits coming from the sprite attributes
PR2 = 111111

also two shadow inputs, SDI0 and SDI1 (from the sprite attributes)

the chip outputs the 11 bit palette index, CO0-CO10, and two shadow bits.

16 internal registers; registers are 6 bits wide (input is D0-D5)
For the most part, their meaning is unknown
All registers are write only.
There must be a way to enable/disable the three external PR inputs.
Some games initialize the priorities of the sprite & background layers,
others don't. It isn't clear whether the data written to those registers is
actually used, since the priority is taken from the external ports.

 0  priority of CI0 (higher = lower priority)
    punkshot: unused?
    lgtnfght: unused?
    simpsons: 3f = 111111
    xmen:     05 = 000101  default value
    xmen:     09 = 001001  used to swap CI0 and CI2
 1  priority of CI1 (higher = lower priority)
    punkshot: 28 = 101000
    lgtnfght: unused?
    simpsons: unused?
    xmen:     02 = 000010
 2  priority of CI2 (higher = lower priority)
    punkshot: 24 = 100100
    lgtnfght: 24 = 100100
    simpsons: 04 = 000100
    xmen:     09 = 001001  default value
    xmen:     05 = 000101  used to swap CI0 and CI2
 3  priority of CI3 (higher = lower priority)
    punkshot: 34 = 110100
    lgtnfght: 34 = 110100
    simpsons: 28 = 101000
    xmen:     00 = 000000
 4  priority of CI4 (higher = lower priority)
    punkshot: 2c = 101100  default value
    punkshot: 3c = 111100  used to swap CI3 and CI4
    punkshot: 26 = 100110  used to swap CI1 and CI4
    lgtnfght: 2c = 101100
    simpsons: 18 = 011000
    xmen:     fe = 111110
 5  unknown
    punkshot: unused?
    lgtnfght: 2a = 101010
    simpsons: unused?
    xmen: unused?
 6  unknown
    punkshot: 26 = 100110
    lgtnfght: 30 = 110000
    simpsons: 17 = 010111
    xmen:     03 = 000011 (written after initial tests)
 7  unknown
    punkshot: unused?
    lgtnfght: unused?
    simpsons: 27 = 100111
    xmen:     07 = 000111 (written after initial tests)
 8  unknown
    punkshot: unused?
    lgtnfght: unused?
    simpsons: 37 = 110111
    xmen:     ff = 111111 (written after initial tests)
 9  ----xx CI0 palette index base (CO9-CO10)
    --xx-- CI1 palette index base (CO9-CO10)
    xx---- CI2 palette index base (CO9-CO10)
10  ---xxx CI3 palette index base (CO8-CO10)
    xxx--- CI4 palette index base (CO8-CO10)
11  unknown
    punkshot: 00 = 000000
    lgtnfght: 00 = 000000
    simpsons: 00 = 000000
    xmen:     00 = 000000 (written after initial tests)
12  unknown
    punkshot: 04 = 000100
    lgtnfght: 04 = 000100
    simpsons: 05 = 000101
    xmen:     05 = 000101
13  unused
14  unused
15  unused


054000
------
Sort of a protection device, used for collision detection.
It is passed a few parameters, and returns a boolean telling if collision
happened. It has no access to gfx data, it only does arithmetical operations
on the parameters.

Memory map:
00      unused
01-03 W A center X
04    W unknown, needed by thndrx2 to pass the startup check, we use a hack
05      unused
06    W A semiaxis X
07    W A semiaxis Y
08      unused
09-0b W A center Y
0c    W unknown, needed by thndrx2 to pass the startup check, we use a hack
0d      unused
0e    W B semiaxis X
0f    W B semiaxis Y
10      unused
11-13 W B center Y
14      unused
15-17 W B center X
18    R 0 = collision, 1 = no collision


051733
------
Sort of a protection device, used for collision detection, and for
arithmetical operations.
It is passed a few parameters, and returns the result.

Memory map(preliminary):
------------------------
00-01 W operand 1
02-03 W operand 2
04-05 W operand 3

00-01 R operand 1 / operand 2
02-03 R operand 1 % operand 2?
04-05 R sqrt(operand 3<<16)
06    R unknown - return value written to 13?

06-07 W distance for collision check
08-09 W Y pos of obj1
0a-0b W X pos of obj1
0c-0d W Y pos of obj2
0e-0f W X pos of obj2
13    W unknown

07    R collision (0x80 = no, 0x00 = yes)
0a-0b R unknown (chequered flag), might just read back X pos
0e-0f R unknown (chequered flag), might just read back X pos

Other addresses are unknown or unused.

Fast Lane:
----------
$9def:
This routine is called only after a collision.
(R) 0x0006: unknown. Only bits 0-3 are used.

Blades of Steel:
----------------
$ac2f:
(R) 0x2f86: unknown. Only uses bit 0.

$a5de:
writes to 0x2f84-0x2f85, waits a little, and then reads from 0x2f84.

$7af3:
(R) 0x2f86: unknown. Only uses bit 0.


Devastators:
------------
$6ce8:
reads from 0x0006, and only uses bit 1.


K055550
-------

Protection chip which performs a memset() operation.

Used in Violent Storm and Ultimate Battler to clear VRAM between scenes, among
other things.  May also perform other functions since Violent Storm still isn't
happy...

Has word-wide registers as follows:

0: Count of units to transfer.  The write here triggers the transfer.
1-6: Unknown
7: Destination address, MSW
8: Destination address, LSW
9: Unknown
10: Size of transfer units, MSW
11: Size of transfer units, LSW
12: Unknown
13: Value to fill destination region with
14-15: Unknown


K055555
-------

Priority encoder.  Always found in conjunction with K054338, but the reverse
isn't true.  The 55555 has 8 inputs: "A", "B", "C", and "D" intended for a 156/157
type tilemap chip, "OBJ" intended for a '246 type sprite chip, and "SUB1-SUB3"
which can be used for 3 additional layers.

When used in combintion with a K054338, each input can be chosen to participate
in shadow/highlight operations, R/G/B alpha blending, and R/G/B brightness control.
Per-tile priority is supported for tilemap planes A and B.

There are also 3 shadow priority registers.  When these are enabled, layers and
sprites with a priority greater than or equal to them become a shadow, and either
then gets drawn as a shadow/highlight or not at all (I'm not sure what selects
this yet.  Dadandarn relies on this mechanism to hide the 53936 plane when
it doesn't want it visible).

It also appears that brightness control and alpha blend can be decided per-tile
and per-sprite, although this is not certain.

Additionally the 55555 can provide a gradient background with one palette entry
per scanline.  This is fairly rarely used, but does turn up in Gokujou Parodius as
well as the Sexy Parodius title screen.

Lots of byte-wise registers.  A partial map:

0: Palette index(?) for the gradient background
1: related to tilemap brightness control
2-5: COLSEL for various inputs (?)
6: COLCHG ON
7-18: priority levels (VA1/VA2/VAC/VB1/VB2/VBC/VC/VD/OBJ/S1/S2/S3)
19-22: INPRI for OBJ/S1/S2/S3
23-32: palette bases (VA/VB/VC/VD/OBJ/S1/S2/S3)
37: shadow 1 priority
38: shadow 2 priority
39: shadow 3 priority
40: shadow/highlight master enable
41: master shadow/highlight priority
42: VBRI: enables brightness control for each VRAM layer (bits: x x x x D B C A)
43: OSBRI: enables brightness control for OBJ and SUB layers, depending for OBJ on attr bits
44: OSBRI_ON: not quite sure
45: input enables.  bits as follows: (MSB) S3 S2 S1 OB VD VC VB VA (LSB)


K054338
-------
Color combiner engine.  Designed for use with the 55555, but also found in games
without one.

Registers (word-wise):

0: first 8 bits unknown, second 8 bits are the R component of the background color
1: G and B components (8 bits each) of the background color
2-4: shadow 1 R/G/B (16 bits per component.  In shadow mode, determines a blend
     value between total blackness and the original color.  In highlight mode,
     determines a blend value between total whiteness and the original color.
     The hardware clamps at black or white as necessary: see the Graphics Test
     in many System GX games).
5-7: shadow 2 R/G/B
8-10: shadow 3 R/G/B
11-12: brightness R/G/B (external circuit such as the 55555 decides which layers
       this applies to)
13-14: alpha blend R/G/B (external circuit such as the 55555 decides which layers
       this applies to)

***************************************************************************/

#include "driver.h"
#include "video/konamiic.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/*
    This recursive function doesn't use additional memory
    (it could be easily converted into an iterative one).
    It's called shuffle because it mimics the shuffling of a deck of cards.
*/
static void shuffle(UINT16 *buf,int len)
{
	int i;
	UINT16 t;

	if (len == 2) return;

	if (len % 4) fatalerror("shuffle() - not modulo 4");   /* must not happen */

	len /= 2;

	for (i = 0;i < len/2;i++)
	{
		t = buf[len/2 + i];
		buf[len/2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	shuffle(buf,len);
	shuffle(buf + len,len);
}


/* helper function to join two 16-bit ROMs and form a 32-bit data stream */
void konami_rom_deinterleave_2(running_machine *machine, const char *mem_region)
{
	shuffle((UINT16 *)memory_region(machine, mem_region),memory_region_length(machine, mem_region)/2);
}

/* hacked version of rom_deinterleave_2_half for Lethal Enforcers */
void konami_rom_deinterleave_2_half(running_machine *machine, const char *mem_region)
{
	UINT8 *rgn = memory_region(machine, mem_region);

	shuffle((UINT16 *)rgn,memory_region_length(machine, mem_region)/4);
	shuffle((UINT16 *)(rgn+memory_region_length(machine, mem_region)/2),memory_region_length(machine, mem_region)/4);
}

/* helper function to join four 16-bit ROMs and form a 64-bit data stream */
void konami_rom_deinterleave_4(running_machine *machine, const char *mem_region)
{
	konami_rom_deinterleave_2(machine, mem_region);
	konami_rom_deinterleave_2(machine, mem_region);
}


static void decode_gfx(running_machine *machine, int gfx_index, UINT8 *data, UINT32 total, const gfx_layout *layout, int bpp)
{
	gfx_layout gl;

	memcpy(&gl, layout, sizeof(gl));
	gl.total = total;
	machine->gfx[gfx_index] = gfx_element_alloc(machine, &gl, data, machine->config->total_colors >> bpp, 0);
}





/***************************************************************************/
/*                                                                         */
/*                                 007121                                  */
/*                                                                         */
/***************************************************************************/

UINT8 K007121_ctrlram[MAX_K007121][8];
static int K007121_flipscreen[MAX_K007121];


void K007121_ctrl_w(running_machine *machine,int chip,int offset,int data)
{
	switch (offset)
	{
		case 6:
/* palette bank change */
if ((K007121_ctrlram[chip][offset] & 0x30) != (data & 0x30))
	tilemap_mark_all_tiles_dirty_all(machine);
			break;
		case 7:
			K007121_flipscreen[chip] = data & 0x08;
			break;
	}

	K007121_ctrlram[chip][offset] = data;
}

WRITE8_HANDLER( K007121_ctrl_0_w )
{
	K007121_ctrl_w(space->machine,0,offset,data);
}

WRITE8_HANDLER( K007121_ctrl_1_w )
{
	K007121_ctrl_w(space->machine,1,offset,data);
}


/*
 * Sprite Format
 * ------------------
 *
 * There are 0x40 sprites, each one using 5 bytes. However the number of
 * sprites can be increased to 0x80 with a control register (Combat School
 * sets it on and off during the game).
 *
 * Byte | Bit(s)   | Use
 * -----+-76543210-+----------------
 *   0  | xxxxxxxx | sprite code
 *   1  | xxxx---- | color
 *   1  | ----xx-- | sprite code low 2 bits for 16x8/8x8 sprites
 *   1  | ------xx | sprite code bank bits 1/0
 *   2  | xxxxxxxx | y position
 *   3  | xxxxxxxx | x position (low 8 bits)
 *   4  | xx------ | sprite code bank bits 3/2
 *   4  | --x----- | flip y
 *   4  | ---x---- | flip x
 *   4  | ----xxx- | sprite size 000=16x16 001=16x8 010=8x16 011=8x8 100=32x32
 *   4  | -------x | x position (high bit)
 *
 * Flack Attack uses a different, "wider" layout with 32 bytes per sprite,
 * mapped as follows, and the priority order is reversed. Maybe it is a
 * compatibility mode with an older custom IC. It is not known how this
 * alternate layout is selected.
 *
 * 0 -> e
 * 1 -> f
 * 2 -> 6
 * 3 -> 4
 * 4 -> 8
 *
 */

void K007121_sprites_draw(int chip,bitmap_t *bitmap,const rectangle *cliprect, gfx_element **gfxs, colortable_t *ctable,
						  const UINT8 *source,int base_color,
						  int global_x_offset,int bank_base, UINT32 pri_mask)
{
	const gfx_element *gfx = gfxs[chip];
	bitmap_t *priority_bitmap = gfx->machine->priority_bitmap;
	int flipscreen = K007121_flipscreen[chip];
	int i,num,inc,offs[5];
	int is_flakatck = (ctable == NULL);

#if 0
popmessage("%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x  %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
	K007121_ctrlram[0][0x00],K007121_ctrlram[0][0x01],K007121_ctrlram[0][0x02],K007121_ctrlram[0][0x03],K007121_ctrlram[0][0x04],K007121_ctrlram[0][0x05],K007121_ctrlram[0][0x06],K007121_ctrlram[0][0x07],
	K007121_ctrlram[1][0x00],K007121_ctrlram[1][0x01],K007121_ctrlram[1][0x02],K007121_ctrlram[1][0x03],K007121_ctrlram[1][0x04],K007121_ctrlram[1][0x05],K007121_ctrlram[1][0x06],K007121_ctrlram[1][0x07]);

if (input_code_pressed(gfx->machine, KEYCODE_D))
{
	FILE *fp;
	fp=fopen(chip?"SPRITE1.DMP":"SPRITE0.DMP", "w+b");
	if (fp)
	{
		fwrite(source, 0x800, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif

	if (is_flakatck)
	{
		num = 0x40;
		inc = -0x20;
		source += 0x3f*0x20;
		offs[0] = 0x0e;
		offs[1] = 0x0f;
		offs[2] = 0x06;
		offs[3] = 0x04;
		offs[4] = 0x08;
	}
	else	/* all others */
	{
		//num = (K007121_ctrlram[chip][0x03] & 0x40) ? 0x80 : 0x40; /* WRONG!!! (needed by combasc)  */
		num = 0x40; // Combasc writes 70 sprites to VRAM at peak but the chip only processes the first 64.

		inc = 5;
		offs[0] = 0x00;
		offs[1] = 0x01;
		offs[2] = 0x02;
		offs[3] = 0x03;
		offs[4] = 0x04;
		/* when using priority buffer, draw front to back */
		if (pri_mask != -1)
		{
			source += (num-1)*inc;
			inc = -inc;
		}
	}

	for (i = 0;i < num;i++)
	{
		int number = source[offs[0]];				/* sprite number */
		int sprite_bank = source[offs[1]] & 0x0f;	/* sprite bank */
		int sx = source[offs[3]];					/* vertical position */
		int sy = source[offs[2]];					/* horizontal position */
		int attr = source[offs[4]];				/* attributes */
		int xflip = source[offs[4]] & 0x10;		/* flip x */
		int yflip = source[offs[4]] & 0x20;		/* flip y */
		int color = base_color + ((source[offs[1]] & 0xf0) >> 4);
		int width,height;
		int transparent_mask;
		static const int x_offset[4] = {0x0,0x1,0x4,0x5};
		static const int y_offset[4] = {0x0,0x2,0x8,0xa};
		int x,y, ex, ey, flipx, flipy, destx, desty;

		if (attr & 0x01) sx -= 256;
		if (sy >= 240) sy -= 256;

		number += ((sprite_bank & 0x3) << 8) + ((attr & 0xc0) << 4);
		number = number << 2;
		number += (sprite_bank >> 2) & 3;

		/* Flak Attack doesn't use a lookup PROM, it maps the color code directly */
		/* to a palette entry */
		if (is_flakatck)
			transparent_mask = 1 << 0;
		else
			transparent_mask = colortable_get_transpen_mask(ctable, gfx, color, 0);

		if (!is_flakatck || source[0x00])	/* Flak Attack needs this */
		{
			number += bank_base;

			switch( attr&0xe )
			{
				case 0x06: width = height = 1; break;
				case 0x04: width = 1; height = 2; number &= (~2); break;
				case 0x02: width = 2; height = 1; number &= (~1); break;
				case 0x00: width = height = 2; number &= (~3); break;
				case 0x08: width = height = 4; number &= (~3); break;
				default: width = 1; height = 1;
//                  logerror("Unknown sprite size %02x\n",attr&0xe);
//                  popmessage("Unknown sprite size %02x\n",attr&0xe);
			}

			for (y = 0;y < height;y++)
			{
				for (x = 0;x < width;x++)
				{
					ex = xflip ? (width-1-x) : x;
					ey = yflip ? (height-1-y) : y;

					if (flipscreen)
					{
						flipx = !xflip;
						flipy = !yflip;
						destx = 248-(sx+x*8);
						desty = 248-(sy+y*8);
					}
					else
					{
						flipx = xflip;
						flipy = yflip;
						destx = global_x_offset+sx+x*8;
						desty = sy+y*8;
					}

					if (pri_mask != -1)
						pdrawgfx_transmask(bitmap,cliprect,gfx,
							number + x_offset[ex] + y_offset[ey],
							color,
							flipx,flipy,
							destx,desty,
							priority_bitmap,pri_mask,
							transparent_mask);
					else
						drawgfx_transmask(bitmap,cliprect,gfx,
							number + x_offset[ex] + y_offset[ey],
							color,
							flipx,flipy,
							destx,desty,
							transparent_mask);
				}
			}
		}

		source += inc;
	}
}



/***************************************************************************/
/*                                                                         */
/*                                 007342                                  */
/*                                                                         */
/***************************************************************************/

static UINT8 *K007342_ram,*K007342_scroll_ram;
static int K007342_gfxnum;
static int K007342_int_enabled;
static int K007342_flipscreen;
static UINT16 K007342_scrollx[2];
static UINT8 K007342_scrolly[2];
static UINT8 *K007342_videoram_0,*K007342_colorram_0;
static UINT8 *K007342_videoram_1,*K007342_colorram_1;
static UINT8 K007342_regs[8];
static void (*K007342_callback)(int tmap, int bank, int *code, int *color, int *flags);
static tilemap *K007342_tilemap[2];

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/*
  data format:
  video RAM     xxxxxxxx    tile number (bits 0-7)
  color RAM     x-------    tiles with priority over the sprites
  color RAM     -x------    depends on external conections
  color RAM     --x-----    flip Y
  color RAM     ---x----    flip X
  color RAM     ----xxxx    depends on external connections (usually color and banking)
*/

static TILEMAP_MAPPER( K007342_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

INLINE void K007342_get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int layer,UINT8 *cram,UINT8 *vram)
{
	int color, code, flags;

	color = cram[tile_index];
	code = vram[tile_index];
	flags = TILE_FLIPYX((color & 0x30) >> 4);

	tileinfo->category = (color & 0x80) >> 7;

	(*K007342_callback)(layer, K007342_regs[1], &code, &color, &flags);

	SET_TILE_INFO(
			K007342_gfxnum,
			code,
			color,
			flags);
}

static TILE_GET_INFO( K007342_get_tile_info0 ) { K007342_get_tile_info(machine,tileinfo,tile_index,0,K007342_colorram_0,K007342_videoram_0); }
static TILE_GET_INFO( K007342_get_tile_info1 ) { K007342_get_tile_info(machine,tileinfo,tile_index,1,K007342_colorram_1,K007342_videoram_1); }



void K007342_vh_start(running_machine *machine, int gfx_index, void (*callback)(int tmap, int bank, int *code, int *color, int *flags))
{
	K007342_gfxnum = gfx_index;
	K007342_callback = callback;

	K007342_tilemap[0] = tilemap_create(machine, K007342_get_tile_info0,K007342_scan,8,8,64,32);
	K007342_tilemap[1] = tilemap_create(machine, K007342_get_tile_info1,K007342_scan,8,8,64,32);

	K007342_ram = auto_alloc_array(machine, UINT8, 0x2000);
	K007342_scroll_ram = auto_alloc_array(machine, UINT8, 0x0200);

	memset(K007342_ram,0,0x2000);

	K007342_colorram_0 = &K007342_ram[0x0000];
	K007342_colorram_1 = &K007342_ram[0x1000];
	K007342_videoram_0 = &K007342_ram[0x0800];
	K007342_videoram_1 = &K007342_ram[0x1800];

	tilemap_set_transparent_pen(K007342_tilemap[0],0);
	tilemap_set_transparent_pen(K007342_tilemap[1],0);

	state_save_register_global_pointer(machine, K007342_ram, 0x2000);
	state_save_register_global_pointer(machine, K007342_scroll_ram, 0x0200);
	state_save_register_global(machine, K007342_int_enabled);
	state_save_register_global(machine, K007342_flipscreen);
	state_save_register_global_array(machine, K007342_scrollx);
	state_save_register_global_array(machine, K007342_scrolly);
	state_save_register_global_array(machine, K007342_regs);
}

READ8_HANDLER( K007342_r )
{
	return K007342_ram[offset];
}

WRITE8_HANDLER( K007342_w )
{
	K007342_ram[offset] = data;
	if (offset < 0x1000)
	{		/* layer 0 */
		tilemap_mark_tile_dirty(K007342_tilemap[0],offset & 0x7ff);
	}
	else
	{						/* layer 1 */
		tilemap_mark_tile_dirty(K007342_tilemap[1],offset & 0x7ff);
	}
}

READ8_HANDLER( K007342_scroll_r )
{
	return K007342_scroll_ram[offset];
}

WRITE8_HANDLER( K007342_scroll_w )
{
	K007342_scroll_ram[offset] = data;
}

WRITE8_HANDLER( K007342_vreg_w )
{
	switch(offset)
	{
		case 0x00:
			/* bit 1: INT control */
			K007342_int_enabled = data & 0x02;
			K007342_flipscreen = data & 0x10;
			tilemap_set_flip(K007342_tilemap[0],K007342_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(K007342_tilemap[1],K007342_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			break;
		case 0x01:  /* used for banking in Rock'n'Rage */
			if (data != K007342_regs[1])
				tilemap_mark_all_tiles_dirty_all(space->machine);
		case 0x02:
			K007342_scrollx[0] = (K007342_scrollx[0] & 0xff) | ((data & 0x01) << 8);
			K007342_scrollx[1] = (K007342_scrollx[1] & 0xff) | ((data & 0x02) << 7);
			break;
		case 0x03:  /* scroll x (register 0) */
			K007342_scrollx[0] = (K007342_scrollx[0] & 0x100) | data;
			break;
		case 0x04:  /* scroll y (register 0) */
			K007342_scrolly[0] = data;
			break;
		case 0x05:  /* scroll x (register 1) */
			K007342_scrollx[1] = (K007342_scrollx[1] & 0x100) | data;
			break;
		case 0x06:  /* scroll y (register 1) */
			K007342_scrolly[1] = data;
		case 0x07:  /* unused */
			break;
	}
	K007342_regs[offset] = data;
}

void K007342_tilemap_update(void)
{
	int offs;


	/* update scroll */
	switch (K007342_regs[2] & 0x1c)
	{
		case 0x00:
		case 0x08:	/* unknown, blades of steel shootout between periods */
			tilemap_set_scroll_rows(K007342_tilemap[0],1);
			tilemap_set_scroll_cols(K007342_tilemap[0],1);
			tilemap_set_scrollx(K007342_tilemap[0],0,K007342_scrollx[0]);
			tilemap_set_scrolly(K007342_tilemap[0],0,K007342_scrolly[0]);
			break;

		case 0x0c:	/* 32 columns */
			tilemap_set_scroll_rows(K007342_tilemap[0],1);
			tilemap_set_scroll_cols(K007342_tilemap[0],512);
			tilemap_set_scrollx(K007342_tilemap[0],0,K007342_scrollx[0]);
			for (offs = 0;offs < 256;offs++)
				tilemap_set_scrolly(K007342_tilemap[0],(offs + K007342_scrollx[0]) & 0x1ff,
						K007342_scroll_ram[2*(offs/8)] + 256 * K007342_scroll_ram[2*(offs/8)+1]);
			break;

		case 0x14:	/* 256 rows */
			tilemap_set_scroll_rows(K007342_tilemap[0],256);
			tilemap_set_scroll_cols(K007342_tilemap[0],1);
			tilemap_set_scrolly(K007342_tilemap[0],0,K007342_scrolly[0]);
			for (offs = 0;offs < 256;offs++)
				tilemap_set_scrollx(K007342_tilemap[0],(offs + K007342_scrolly[0]) & 0xff,
						K007342_scroll_ram[2*offs] + 256 * K007342_scroll_ram[2*offs+1]);
			break;

		default:
//          popmessage("unknown scroll ctrl %02x",K007342_regs[2] & 0x1c);
			break;
	}

	tilemap_set_scrollx(K007342_tilemap[1],0,K007342_scrollx[1]);
	tilemap_set_scrolly(K007342_tilemap[1],0,K007342_scrolly[1]);

#if 0
	{
		static int current_layer = 0;

		if (input_code_pressed_once(machine, KEYCODE_Z)) current_layer = !current_layer;
		tilemap_set_enable(K007342_tilemap[current_layer], 1);
		tilemap_set_enable(K007342_tilemap[!current_layer], 0);

		popmessage("regs:%02x %02x %02x %02x-%02x %02x %02x %02x:%02x",
			K007342_regs[0], K007342_regs[1], K007342_regs[2], K007342_regs[3],
			K007342_regs[4], K007342_regs[5], K007342_regs[6], K007342_regs[7],
			current_layer);
	}
#endif
}

void K007342_tilemap_set_enable(int tmap, int enable)
{
	tilemap_set_enable(K007342_tilemap[tmap], enable);
}

void K007342_tilemap_draw(bitmap_t *bitmap,const rectangle *cliprect,int num,int flags,UINT32 priority)
{
	tilemap_draw(bitmap,cliprect,K007342_tilemap[num],flags,priority);
}

int K007342_is_INT_enabled(void)
{
	return K007342_int_enabled;
}



/***************************************************************************/
/*                                                                         */
/*                                 007420                                  */
/*                                                                         */
/***************************************************************************/

static gfx_element *K007420_gfx;
static void (*K007420_callback)(int *code,int *color);
static UINT8 *K007420_ram;
static int K007420_banklimit;

void K007420_vh_start(running_machine *machine, int gfxnum, void (*callback)(int *code,int *color))
{
	K007420_gfx = machine->gfx[gfxnum];
	K007420_callback = callback;
	K007420_ram = auto_alloc_array_clear(machine, UINT8, 0x200);

	K007420_banklimit = -1;

	state_save_register_global_pointer(machine, K007420_ram, 0x0200);
	state_save_register_global(machine, K007420_banklimit);
}

READ8_HANDLER( K007420_r )
{
	return K007420_ram[offset];
}

WRITE8_HANDLER( K007420_w )
{
	K007420_ram[offset] = data;
}

/*
 * Sprite Format
 * ------------------
 *
 * Byte | Bit(s)   | Use
 * -----+-76543210-+----------------
 *   0  | xxxxxxxx | y position
 *   1  | xxxxxxxx | sprite code (low 8 bits)
 *   2  | xxxxxxxx | depends on external conections. Usually banking
 *   3  | xxxxxxxx | x position (low 8 bits)
 *   4  | x------- | x position (high bit)
 *   4  | -xxx---- | sprite size 000=16x16 001=8x16 010=16x8 011=8x8 100=32x32
 *   4  | ----x--- | flip y
 *   4  | -----x-- | flip x
 *   4  | ------xx | zoom (bits 8 & 9)
 *   5  | xxxxxxxx | zoom (low 8 bits)  0x080 = normal, < 0x80 enlarge, > 0x80 reduce
 *   6  | xxxxxxxx | unused
 *   7  | xxxxxxxx | unused
 */

void K007420_sprites_draw(bitmap_t *bitmap,const rectangle *cliprect)
{
#define K007420_SPRITERAM_SIZE 0x200
	int offs;
	int codemask = K007420_banklimit;
	int bankmask = ~K007420_banklimit;

	for (offs = K007420_SPRITERAM_SIZE - 8; offs >= 0; offs -= 8)
	{
		int ox,oy,code,color,flipx,flipy,zoom,w,h,x,y,bank;
		static const int xoffset[4] = { 0, 1, 4, 5 };
		static const int yoffset[4] = { 0, 2, 8, 10 };

		code = K007420_ram[offs+1];
		color = K007420_ram[offs+2];
		ox = K007420_ram[offs+3] - ((K007420_ram[offs+4] & 0x80) << 1);
		oy = 256 - K007420_ram[offs+0];
		flipx = K007420_ram[offs+4] & 0x04;
		flipy = K007420_ram[offs+4] & 0x08;

		(*K007420_callback)(&code,&color);

		bank = code & bankmask;
		code &= codemask;

		/* 0x080 = normal scale, 0x040 = double size, 0x100 half size */
		zoom = K007420_ram[offs+5] | ((K007420_ram[offs+4] & 0x03) << 8);
		if (!zoom) continue;
		zoom = 0x10000 * 128 / zoom;

		switch (K007420_ram[offs+4] & 0x70)
		{
			case 0x30: w = h = 1; break;
			case 0x20: w = 2; h = 1; code &= (~1); break;
			case 0x10: w = 1; h = 2; code &= (~2); break;
			case 0x00: w = h = 2; code &= (~3); break;
			case 0x40: w = h = 4; code &= (~3); break;
			default: w = 1; h = 1;
//logerror("Unknown sprite size %02x\n",(K007420_ram[offs+4] & 0x70)>>4);
		}

		if (K007342_flipscreen)
		{
			ox = 256 - ox - ((zoom * w + (1<<12)) >> 13);
			oy = 256 - oy - ((zoom * h + (1<<12)) >> 13);
			flipx = !flipx;
			flipy = !flipy;
		}

		if (zoom == 0x10000)
		{
			int sx,sy;

			for (y = 0;y < h;y++)
			{
				sy = oy + 8 * y;

				for (x = 0;x < w;x++)
				{
					int c = code;

					sx = ox + 8 * x;
					if (flipx) c += xoffset[(w-1-x)];
					else c += xoffset[x];
					if (flipy) c += yoffset[(h-1-y)];
					else c += yoffset[y];

					if (c & bankmask) continue; else c += bank;

					drawgfx_transpen(bitmap,cliprect,K007420_gfx,
						c,
						color,
						flipx,flipy,
						sx,sy,0);

					if (K007342_regs[2] & 0x80)
						drawgfx_transpen(bitmap,cliprect,K007420_gfx,
							c,
							color,
							flipx,flipy,
							sx,sy-256,0);
				}
			}
		}
		else
		{
			int sx,sy,zw,zh;
			for (y = 0;y < h;y++)
			{
				sy = oy + ((zoom * y + (1<<12)) >> 13);
				zh = (oy + ((zoom * (y+1) + (1<<12)) >> 13)) - sy;

				for (x = 0;x < w;x++)
				{
					int c = code;

					sx = ox + ((zoom * x + (1<<12)) >> 13);
					zw = (ox + ((zoom * (x+1) + (1<<12)) >> 13)) - sx;
					if (flipx) c += xoffset[(w-1-x)];
					else c += xoffset[x];
					if (flipy) c += yoffset[(h-1-y)];
					else c += yoffset[y];

					if (c & bankmask) continue; else c += bank;

					drawgfxzoom_transpen(bitmap,cliprect,K007420_gfx,
						c,
						color,
						flipx,flipy,
						sx,sy,
						(zw << 16) / 8,(zh << 16) / 8,0);

					if (K007342_regs[2] & 0x80)
						drawgfxzoom_transpen(bitmap,cliprect,K007420_gfx,
							c,
							color,
							flipx,flipy,
							sx,sy-256,
							(zw << 16) / 8,(zh << 16) / 8,0);
				}
			}
		}
	}
#if 0
	{
		static int current_sprite = 0;

		if (input_code_pressed_once(machine, KEYCODE_Z)) current_sprite = (current_sprite+1) & ((K007420_SPRITERAM_SIZE/8)-1);
		if (input_code_pressed_once(machine, KEYCODE_X)) current_sprite = (current_sprite-1) & ((K007420_SPRITERAM_SIZE/8)-1);

		popmessage("%02x:%02x %02x %02x %02x %02x %02x %02x %02x", current_sprite,
			K007420_ram[(current_sprite*8)+0], K007420_ram[(current_sprite*8)+1],
			K007420_ram[(current_sprite*8)+2], K007420_ram[(current_sprite*8)+3],
			K007420_ram[(current_sprite*8)+4], K007420_ram[(current_sprite*8)+5],
			K007420_ram[(current_sprite*8)+6], K007420_ram[(current_sprite*8)+7]);
	}
#endif
}

void K007420_set_banklimit(int limit)
{
	K007420_banklimit = limit;
}



/***************************************************************************/
/*                                                                         */
/*                                 052109                                  */
/*                                                                         */
/***************************************************************************/

static const char *K052109_memory_region;
static int K052109_gfxnum;
static void (*K052109_callback)(int tmap,int bank,int *code,int *color,int *flags,int *priority);
static UINT8 *K052109_ram;
static UINT8 *K052109_videoram_F,*K052109_videoram2_F,*K052109_colorram_F;
static UINT8 *K052109_videoram_A,*K052109_videoram2_A,*K052109_colorram_A;
static UINT8 *K052109_videoram_B,*K052109_videoram2_B,*K052109_colorram_B;
static UINT8 K052109_charrombank[4];
static UINT8 K052109_charrombank_2[4];
static UINT8 has_extra_video_ram;
static INT32 K052109_RMRD_line;
static int K052109_tileflip_enable;
static UINT8 K052109_irq_enabled;
static INT32 K052109_dx[3], K052109_dy[3];
static UINT8 K052109_romsubbank,K052109_scrollctrl;
tilemap *K052109_tilemap[3];



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/*
  data format:
  video RAM    xxxxxxxx  tile number (low 8 bits)
  color RAM    xxxx----  depends on external connections (usually color and banking)
  color RAM    ----xx--  bank select (0-3): these bits are replaced with the 2
                         bottom bits of the bank register before being placed on
                         the output pins. The other two bits of the bank register are
                         placed on the CAB1 and CAB2 output pins.
  color RAM    ------xx  depends on external connections (usually banking, flip)
*/

INLINE void K052109_get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int layer,UINT8 *cram,UINT8 *vram1,UINT8 *vram2)
{
	int flipy = 0;
	int code = vram1[tile_index] + 256 * vram2[tile_index];
	int color = cram[tile_index];
	int flags = 0;
	int priority = 0;
	int bank = K052109_charrombank[(color & 0x0c) >> 2];
if (has_extra_video_ram) bank = (color & 0x0c) >> 2;	/* kludge for X-Men */
	color = (color & 0xf3) | ((bank & 0x03) << 2);
	bank >>= 2;

	flipy = color & 0x02;

	(*K052109_callback)(layer,bank,&code,&color,&flags,&priority);

	/* if the callback set flip X but it is not enabled, turn it off */
	if (!(K052109_tileflip_enable & 1)) flags &= ~TILE_FLIPX;

	/* if flip Y is enabled and the attribute but is set, turn it on */
	if (flipy && (K052109_tileflip_enable & 2)) flags |= TILE_FLIPY;

	SET_TILE_INFO(
			K052109_gfxnum,
			code,
			color,
			flags);

	tileinfo->category = priority;
}

static TILE_GET_INFO( K052109_get_tile_info0 ) { K052109_get_tile_info(machine,tileinfo,tile_index,0,K052109_colorram_F,K052109_videoram_F,K052109_videoram2_F); }
static TILE_GET_INFO( K052109_get_tile_info1 ) { K052109_get_tile_info(machine,tileinfo,tile_index,1,K052109_colorram_A,K052109_videoram_A,K052109_videoram2_A); }
static TILE_GET_INFO( K052109_get_tile_info2 ) { K052109_get_tile_info(machine,tileinfo,tile_index,2,K052109_colorram_B,K052109_videoram_B,K052109_videoram2_B); }


static STATE_POSTLOAD( K052109_tileflip_reset )
{
	int data = K052109_ram[0x1e80];
	tilemap_set_flip(K052109_tilemap[0],(data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	tilemap_set_flip(K052109_tilemap[1],(data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	tilemap_set_flip(K052109_tilemap[2],(data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	K052109_tileflip_enable = ((data & 0x06) >> 1);
}


#define XOR(a) WORD_XOR_BE(a)

void K052109_vh_start(running_machine *machine,const char *gfx_memory_region,int plane_order,
		void (*callback)(int tmap,int bank,int *code,int *color,int *flags,int *priority))
{
	int gfx_index, i;
	UINT32 total;
	static const gfx_layout charlayout =
	{
		8,8,
		0,
		4,
		{ 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
		32*8
	};
	static const gfx_layout charlayout_gradius3 =
	{
		8,8,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ XOR(0)*4, XOR(1)*4, XOR(2)*4, XOR(3)*4, XOR(4)*4, XOR(5)*4, XOR(6)*4, XOR(7)*4 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
		32*8
	};


	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine->gfx[gfx_index] == 0)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* decode the graphics */
	switch (plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, gfx_memory_region) / 32;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout, 4);
		break;

	case GRADIUS3_PLANE_ORDER:
		total = 0x1000;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout_gradius3, 4);
		break;

	default:
		fatalerror("Unsupported plane_order");
	}

	K052109_memory_region = gfx_memory_region;
	K052109_gfxnum = gfx_index;
	K052109_callback = callback;
	K052109_RMRD_line = CLEAR_LINE;
	K052109_irq_enabled = 0;

	has_extra_video_ram = 0;

	K052109_tilemap[0] = tilemap_create(machine, K052109_get_tile_info0,tilemap_scan_rows,8,8,64,32);
	K052109_tilemap[1] = tilemap_create(machine, K052109_get_tile_info1,tilemap_scan_rows,8,8,64,32);
	K052109_tilemap[2] = tilemap_create(machine, K052109_get_tile_info2,tilemap_scan_rows,8,8,64,32);

	K052109_ram = auto_alloc_array(machine, UINT8, 0x6000);

	memset(K052109_ram,0,0x6000);

	K052109_colorram_F = &K052109_ram[0x0000];
	K052109_colorram_A = &K052109_ram[0x0800];
	K052109_colorram_B = &K052109_ram[0x1000];
	K052109_videoram_F = &K052109_ram[0x2000];
	K052109_videoram_A = &K052109_ram[0x2800];
	K052109_videoram_B = &K052109_ram[0x3000];
	K052109_videoram2_F = &K052109_ram[0x4000];
	K052109_videoram2_A = &K052109_ram[0x4800];
	K052109_videoram2_B = &K052109_ram[0x5000];

	tilemap_set_transparent_pen(K052109_tilemap[0],0);
	tilemap_set_transparent_pen(K052109_tilemap[1],0);
	tilemap_set_transparent_pen(K052109_tilemap[2],0);

	for (i = 0; i < 3; i++)
	{
		K052109_dx[i] = K052109_dy[i] = 0;
	}

	state_save_register_global_pointer(machine, K052109_ram, 0x6000);
	state_save_register_global(machine, K052109_RMRD_line);
	state_save_register_global(machine, K052109_romsubbank);
	state_save_register_global(machine, K052109_scrollctrl);
	state_save_register_global(machine, K052109_irq_enabled);
	state_save_register_global_array(machine, K052109_charrombank);
	state_save_register_global_array(machine, K052109_charrombank_2);
	state_save_register_global_array(machine, K052109_dx);
	state_save_register_global_array(machine, K052109_dy);
	state_save_register_global(machine, has_extra_video_ram);

	state_save_register_postload(machine, K052109_tileflip_reset, NULL);
}

READ8_HANDLER( K052109_r )
{
	if (K052109_RMRD_line == CLEAR_LINE)
	{
		if ((offset & 0x1fff) >= 0x1800)
		{
			if (offset >= 0x180c && offset < 0x1834)
			{	/* A y scroll */	}
			else if (offset >= 0x1a00 && offset < 0x1c00)
			{	/* A x scroll */	}
			else if (offset == 0x1d00)
			{	/* read for bitwise operations before writing */	}
			else if (offset >= 0x380c && offset < 0x3834)
			{	/* B y scroll */	}
			else if (offset >= 0x3a00 && offset < 0x3c00)
			{	/* B x scroll */	}
//          else
//logerror("%04x: read from unknown 052109 address %04x\n",cpu_get_pc(space->cpu),offset);
		}

		return K052109_ram[offset];
	}
	else	/* Punk Shot and TMNT read from 0000-1fff, Aliens from 2000-3fff */
	{
		int code = (offset & 0x1fff) >> 5;
		int color = K052109_romsubbank;
		int flags = 0;
		int priority = 0;
		int bank = K052109_charrombank[(color & 0x0c) >> 2] >> 2;   /* discard low bits (TMNT) */
		int addr;

		bank |= (K052109_charrombank_2[(color & 0x0c) >> 2] >> 2); // Surprise Attack uses this 2nd bank in the rom test

if (has_extra_video_ram) code |= color << 8;	/* kludge for X-Men */
else
		(*K052109_callback)(0,bank,&code,&color,&flags,&priority);

		addr = (code << 5) + (offset & 0x1f);
		addr &= memory_region_length(space->machine, K052109_memory_region)-1;

//      logerror("%04x: off = %04x sub = %02x (bnk = %x) adr = %06x\n",cpu_get_pc(space->cpu),offset,K052109_romsubbank,bank,addr);

		return memory_region(space->machine, K052109_memory_region)[addr];
	}
}

WRITE8_HANDLER( K052109_w )
{
	if ((offset & 0x1fff) < 0x1800) /* tilemap RAM */
	{
		if (offset >= 0x4000) has_extra_video_ram = 1;  /* kludge for X-Men */
		K052109_ram[offset] = data;
		tilemap_mark_tile_dirty(K052109_tilemap[(offset & 0x1800) >> 11],offset & 0x7ff);
	}
	else	/* control registers */
	{
		K052109_ram[offset] = data;

		if (offset >= 0x180c && offset < 0x1834)
		{	/* A y scroll */	}
		else if (offset >= 0x1a00 && offset < 0x1c00)
		{	/* A x scroll */	}
		else if (offset == 0x1c80)
		{
			if (K052109_scrollctrl != data)
			{
//popmessage("scrollcontrol = %02x",data);
//logerror("%04x: rowscrollcontrol = %02x\n",cpu_get_pc(space->cpu),data);
				K052109_scrollctrl = data;
			}
		}
		else if (offset == 0x1d00)
		{
//logerror("%04x: 052109 register 1d00 = %02x\n",cpu_get_pc(space->cpu),data);
			/* bit 2 = irq enable */
			/* the custom chip can also generate NMI and FIRQ, for use with a 6809 */
			K052109_irq_enabled = data & 0x04;
		}
		else if (offset == 0x1d80)
		{
			int dirty = 0;

			if (K052109_charrombank[0] != (data & 0x0f)) dirty |= 1;
			if (K052109_charrombank[1] != ((data >> 4) & 0x0f)) dirty |= 2;
			if (dirty)
			{
				int i;

				K052109_charrombank[0] = data & 0x0f;
				K052109_charrombank[1] = (data >> 4) & 0x0f;

				for (i = 0;i < 0x1800;i++)
				{
					int bank = (K052109_ram[i]&0x0c) >> 2;
					if ((bank == 0 && (dirty & 1)) || (bank == 1 && (dirty & 2)))
					{
						tilemap_mark_tile_dirty(K052109_tilemap[(i & 0x1800) >> 11],i & 0x7ff);
					}
				}
			}
		}
		else if (offset == 0x1e00 || offset == 0x3e00) // Surprise Attack uses offset 0x3e00
		{
//logerror("%04x: 052109 register 1e00 = %02x\n",cpu_get_pc(space->cpu),data);
			K052109_romsubbank = data;
		}
		else if (offset == 0x1e80)
		{
//if ((data & 0xfe)) logerror("%04x: 052109 register 1e80 = %02x\n",cpu_get_pc(space->cpu),data);
			tilemap_set_flip(K052109_tilemap[0],(data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(K052109_tilemap[1],(data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(K052109_tilemap[2],(data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			if (K052109_tileflip_enable != ((data & 0x06) >> 1))
			{
				K052109_tileflip_enable = ((data & 0x06) >> 1);

				tilemap_mark_all_tiles_dirty(K052109_tilemap[0]);
				tilemap_mark_all_tiles_dirty(K052109_tilemap[1]);
				tilemap_mark_all_tiles_dirty(K052109_tilemap[2]);
			}
		}
		else if (offset == 0x1f00)
		{
			int dirty = 0;

			if (K052109_charrombank[2] != (data & 0x0f)) dirty |= 1;
			if (K052109_charrombank[3] != ((data >> 4) & 0x0f)) dirty |= 2;
			if (dirty)
			{
				int i;

				K052109_charrombank[2] = data & 0x0f;
				K052109_charrombank[3] = (data >> 4) & 0x0f;

				for (i = 0;i < 0x1800;i++)
				{
					int bank = (K052109_ram[i] & 0x0c) >> 2;
					if ((bank == 2 && (dirty & 1)) || (bank == 3 && (dirty & 2)))
						tilemap_mark_tile_dirty(K052109_tilemap[(i & 0x1800) >> 11],i & 0x7ff);
				}
			}
		}
		else if (offset >= 0x380c && offset < 0x3834)
		{	/* B y scroll */	}
		else if (offset >= 0x3a00 && offset < 0x3c00)
		{	/* B x scroll */	}
		else if (offset == 0x3d80) // Surprise Attack uses offset 0x3d80 in rom test
		{
			// mirroring this write, breaks Surprise Attack in game tilemaps
			K052109_charrombank_2[0] = data & 0x0f;
			K052109_charrombank_2[1] = (data >> 4) & 0x0f;
		}
		else if (offset == 0x3f00) // Surprise Attack uses offset 0x3f00 in rom test
		{
			// mirroring this write, breaks Surprise Attack in game tilemaps
			K052109_charrombank_2[2] = data & 0x0f;
			K052109_charrombank_2[3] = (data >> 4) & 0x0f;
		}
//      else
//          logerror("%04x: write %02x to unknown 052109 address %04x\n",cpu_get_pc(space->cpu),data,offset);
	}
}

READ16_HANDLER( K052109_word_r )
{
	return K052109_r(space,offset + 0x2000) | (K052109_r(space,offset) << 8);
}

WRITE16_HANDLER( K052109_word_w )
{
	if (ACCESSING_BITS_8_15)
		K052109_w(space,offset,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		K052109_w(space,offset + 0x2000,data & 0xff);
}

READ16_HANDLER(K052109_lsb_r)
{
	return K052109_r(space,offset);
}

WRITE16_HANDLER(K052109_lsb_w)
{
	if(ACCESSING_BITS_0_7)
		K052109_w(space, offset, data & 0xff);
}

void K052109_set_RMRD_line(int state)
{
	K052109_RMRD_line = state;
}

int K052109_get_RMRD_line(void)
{
	return K052109_RMRD_line;
}


void K052109_tilemap_update(void)
{
#if 0
{
popmessage("%x %x %x %x",
	K052109_charrombank[0],
	K052109_charrombank[1],
	K052109_charrombank[2],
	K052109_charrombank[3]);
}
#endif
	if ((K052109_scrollctrl & 0x03) == 0x02)
	{
		int xscroll,yscroll,offs;
		UINT8 *scrollram = &K052109_ram[0x1a00];


		tilemap_set_scroll_rows(K052109_tilemap[1],256);
		tilemap_set_scroll_cols(K052109_tilemap[1],1);
		yscroll = K052109_ram[0x180c];
		tilemap_set_scrolly(K052109_tilemap[1],0,yscroll+K052109_dy[1]);
		for (offs = 0;offs < 256;offs++)
		{
			xscroll = scrollram[2*(offs&0xfff8)+0] + 256 * scrollram[2*(offs&0xfff8)+1];
			xscroll -= 6;
			tilemap_set_scrollx(K052109_tilemap[1],(offs+yscroll)&0xff,xscroll+K052109_dx[1]);
		}
	}
	else if ((K052109_scrollctrl & 0x03) == 0x03)
	{
		int xscroll,yscroll,offs;
		UINT8 *scrollram = &K052109_ram[0x1a00];


		tilemap_set_scroll_rows(K052109_tilemap[1],256);
		tilemap_set_scroll_cols(K052109_tilemap[1],1);
		yscroll = K052109_ram[0x180c];
		tilemap_set_scrolly(K052109_tilemap[1],0,yscroll+K052109_dy[1]);
		for (offs = 0;offs < 256;offs++)
		{
			xscroll = scrollram[2*offs+0] + 256 * scrollram[2*offs+1];
			xscroll -= 6;
			tilemap_set_scrollx(K052109_tilemap[1],(offs+yscroll)&0xff,xscroll+K052109_dx[1]);
		}
	}
	else if ((K052109_scrollctrl & 0x04) == 0x04)
	{
		int xscroll,yscroll,offs;
		UINT8 *scrollram = &K052109_ram[0x1800];


		tilemap_set_scroll_rows(K052109_tilemap[1],1);
		tilemap_set_scroll_cols(K052109_tilemap[1],512);
		xscroll = K052109_ram[0x1a00] + 256 * K052109_ram[0x1a01];
		xscroll -= 6;
		tilemap_set_scrollx(K052109_tilemap[1],0,xscroll+K052109_dx[1]);
		for (offs = 0;offs < 512;offs++)
		{
			yscroll = scrollram[offs/8];
			tilemap_set_scrolly(K052109_tilemap[1],(offs+xscroll)&0x1ff,yscroll+K052109_dy[1]);
		}
	}
	else
	{
		int xscroll,yscroll;
		UINT8 *scrollram = &K052109_ram[0x1a00];


		tilemap_set_scroll_rows(K052109_tilemap[1],1);
		tilemap_set_scroll_cols(K052109_tilemap[1],1);
		xscroll = scrollram[0] + 256 * scrollram[1];
		xscroll -= 6;
		yscroll = K052109_ram[0x180c];
		tilemap_set_scrollx(K052109_tilemap[1],0,xscroll+K052109_dx[1]);
		tilemap_set_scrolly(K052109_tilemap[1],0,yscroll+K052109_dy[1]);
	}

	if ((K052109_scrollctrl & 0x18) == 0x10)
	{
		int xscroll,yscroll,offs;
		UINT8 *scrollram = &K052109_ram[0x3a00];


		tilemap_set_scroll_rows(K052109_tilemap[2],256);
		tilemap_set_scroll_cols(K052109_tilemap[2],1);
		yscroll = K052109_ram[0x380c];
		tilemap_set_scrolly(K052109_tilemap[2],0,yscroll+K052109_dy[2]);
		for (offs = 0;offs < 256;offs++)
		{
			xscroll = scrollram[2*(offs&0xfff8)+0] + 256 * scrollram[2*(offs&0xfff8)+1];
			xscroll -= 6;
			tilemap_set_scrollx(K052109_tilemap[2],(offs+yscroll)&0xff,xscroll+K052109_dx[2]);
		}
	}
	else if ((K052109_scrollctrl & 0x18) == 0x18)
	{
		int xscroll,yscroll,offs;
		UINT8 *scrollram = &K052109_ram[0x3a00];


		tilemap_set_scroll_rows(K052109_tilemap[2],256);
		tilemap_set_scroll_cols(K052109_tilemap[2],1);
		yscroll = K052109_ram[0x380c];
		tilemap_set_scrolly(K052109_tilemap[2],0,yscroll+K052109_dy[2]);
		for (offs = 0;offs < 256;offs++)
		{
			xscroll = scrollram[2*offs+0] + 256 * scrollram[2*offs+1];
			xscroll -= 6;
			tilemap_set_scrollx(K052109_tilemap[2],(offs+yscroll)&0xff,xscroll+K052109_dx[2]);
		}
	}
	else if ((K052109_scrollctrl & 0x20) == 0x20)
	{
		int xscroll,yscroll,offs;
		UINT8 *scrollram = &K052109_ram[0x3800];


		tilemap_set_scroll_rows(K052109_tilemap[2],1);
		tilemap_set_scroll_cols(K052109_tilemap[2],512);
		xscroll = K052109_ram[0x3a00] + 256 * K052109_ram[0x3a01];
		xscroll -= 6;
		tilemap_set_scrollx(K052109_tilemap[2],0,xscroll+K052109_dx[2]);
		for (offs = 0;offs < 512;offs++)
		{
			yscroll = scrollram[offs/8];
			tilemap_set_scrolly(K052109_tilemap[2],(offs+xscroll)&0x1ff,yscroll+K052109_dy[2]);
		}
	}
	else
	{
		int xscroll,yscroll;
		UINT8 *scrollram = &K052109_ram[0x3a00];


		tilemap_set_scroll_rows(K052109_tilemap[2],1);
		tilemap_set_scroll_cols(K052109_tilemap[2],1);
		xscroll = scrollram[0] + 256 * scrollram[1];
		xscroll -= 6;
		yscroll = K052109_ram[0x380c];
		tilemap_set_scrollx(K052109_tilemap[2],0,xscroll+K052109_dx[2]);
		tilemap_set_scrolly(K052109_tilemap[2],0,yscroll+K052109_dy[2]);
	}

#if 0
if ((K052109_scrollctrl & 0x03) == 0x01 ||
		(K052109_scrollctrl & 0x18) == 0x08 ||
		((K052109_scrollctrl & 0x04) && (K052109_scrollctrl & 0x03)) ||
		((K052109_scrollctrl & 0x20) && (K052109_scrollctrl & 0x18)) ||
		(K052109_scrollctrl & 0xc0) != 0)
	popmessage("scrollcontrol = %02x",K052109_scrollctrl);

if (input_code_pressed(machine, KEYCODE_F))
{
	FILE *fp;
	fp=fopen("TILE.DMP", "w+b");
	if (fp)
	{
		fwrite(K052109_ram, 0x6000, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
}

int K052109_is_IRQ_enabled(void)
{
	return K052109_irq_enabled;
}

void K052109_set_layer_offsets(int layer, int dx, int dy)
{
	K052109_dx[layer] = dx;
	K052109_dy[layer] = dy;
}


/***************************************************************************/
/*                                                                         */
/*                                 051960                                  */
/*                                                                         */
/***************************************************************************/

static const char *K051960_memory_region;
static gfx_element *K051960_gfx;
static void (*K051960_callback)(int *code,int *color,int *priority,int *shadow);
static int K051960_romoffset;
static int K051960_spriteflip,K051960_readroms;
static UINT8 K051960_spriterombank[3];
static UINT8 *K051960_ram;
static int K051960_dx, K051960_dy;
static int K051960_irq_enabled, K051960_nmi_enabled;


void K051960_vh_start(running_machine *machine,const char *gfx_memory_region,int plane_order,
		void (*callback)(int *code,int *color,int *priority,int *shadow))
{
	int gfx_index;
	UINT32 total;
	static const gfx_layout spritelayout =
	{
		16,16,
		0,
		4,
		{ 0, 8, 16, 24 },
		{ 0, 1, 2, 3, 4, 5, 6, 7,
				8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
				16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
		128*8
	};
	static const gfx_layout spritelayout_reverse =
	{
		16,16,
		0,
		4,
		{ 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7,
				8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
				16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
		128*8
	};
	static const gfx_layout spritelayout_gradius3 =
	{
		16,16,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			32*8+2*4, 32*8+3*4, 32*8+0*4, 32*8+1*4, 32*8+6*4, 32*8+7*4, 32*8+4*4, 32*8+5*4 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			64*8+0*32, 64*8+1*32, 64*8+2*32, 64*8+3*32, 64*8+4*32, 64*8+5*32, 64*8+6*32, 64*8+7*32 },
		128*8
	};

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine->gfx[gfx_index] == 0)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* decode the graphics */
	switch (plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, gfx_memory_region) / 128;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &spritelayout, 4);
		break;

	case REVERSE_PLANE_ORDER:
		total = memory_region_length(machine, gfx_memory_region) / 128;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &spritelayout_reverse, 4);
		break;

	case GRADIUS3_PLANE_ORDER:
		total = 0x4000;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &spritelayout_gradius3, 4);
		break;

	default:
		fatalerror("Unknown plane_order");
	}

	if (VERBOSE && !(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	K051960_dx = K051960_dy = 0;

	K051960_memory_region = gfx_memory_region;
	K051960_gfx = machine->gfx[gfx_index];
	K051960_callback = callback;
	K051960_ram = auto_alloc_array_clear(machine, UINT8, 0x400);

	state_save_register_global(machine, K051960_romoffset);
	state_save_register_global(machine, K051960_spriteflip);
	state_save_register_global(machine, K051960_readroms);
	state_save_register_global_array(machine, K051960_spriterombank);
	state_save_register_global_pointer(machine, K051960_ram, 0x400);
	state_save_register_global(machine, K051960_irq_enabled);
	state_save_register_global(machine, K051960_nmi_enabled);
}


static int K051960_fetchromdata(running_machine *machine, int byte)
{
	int code,color,pri,shadow,off1,addr;


	addr = K051960_romoffset + (K051960_spriterombank[0] << 8) +
			((K051960_spriterombank[1] & 0x03) << 16);
	code = (addr & 0x3ffe0) >> 5;
	off1 = addr & 0x1f;
	color = ((K051960_spriterombank[1] & 0xfc) >> 2) + ((K051960_spriterombank[2] & 0x03) << 6);
	pri = 0;
	shadow = color & 0x80;
	(*K051960_callback)(&code,&color,&pri,&shadow);

	addr = (code << 7) | (off1 << 2) | byte;
	addr &= memory_region_length(machine, K051960_memory_region)-1;

//  popmessage("%s: addr %06x",cpuexec_describe_context(machine),addr);

	return memory_region(machine, K051960_memory_region)[addr];
}

READ8_HANDLER( K051960_r )
{
	if (K051960_readroms)
	{
		/* the 051960 remembers the last address read and uses it when reading the sprite ROMs */
		K051960_romoffset = (offset & 0x3fc) >> 2;
		return K051960_fetchromdata(space->machine, offset & 3);	/* only 88 Games reads the ROMs from here */
	}
	else
		return K051960_ram[offset];
}

WRITE8_HANDLER( K051960_w )
{
	K051960_ram[offset] = data;
}

READ16_HANDLER( K051960_word_r )
{
	return K051960_r(space,offset*2 + 1) | (K051960_r(space,offset*2) << 8);
}

WRITE16_HANDLER( K051960_word_w )
{
	if (ACCESSING_BITS_8_15)
		K051960_w(space,offset*2,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		K051960_w(space,offset*2 + 1,data & 0xff);
}

READ8_HANDLER( K051937_r )
{
	if (K051960_readroms && offset >= 4 && offset < 8)
	{
		return K051960_fetchromdata(space->machine, offset & 3);
	}
	else
	{
		if (offset == 0)
		{
			static int counter;

			/* some games need bit 0 to pulse */
			return (counter++) & 1;
		}
//logerror("%04x: read unknown 051937 address %x\n",cpu_get_pc(space->cpu),offset);
		return 0;
	}
}

WRITE8_HANDLER( K051937_w )
{
	if (offset == 0)
	{

//if (data & 0xc2) popmessage("051937 reg 00 = %02x",data);

		/* bit 0 is IRQ enable */
		K051960_irq_enabled = (data & 0x01);

		/* bit 1: probably FIRQ enable */

		/* bit 2 is NMI enable */
		K051960_nmi_enabled = (data & 0x04);

		/* bit 3 = flip screen */
		K051960_spriteflip = data & 0x08;

		/* bit 4 used by Devastators and TMNT, unknown */

		/* bit 5 = enable gfx ROM reading */
		K051960_readroms = data & 0x20;
//logerror("%04x: write %02x to 051937 address %x\n",cpu_get_pc(space->cpu),data,offset);
	}
	else if (offset == 1)
	{
//  popmessage("%04x: write %02x to 051937 address %x",cpu_get_pc(space->cpu),data,offset);
//logerror("%04x: write %02x to unknown 051937 address %x\n",cpu_get_pc(space->cpu),data,offset);
	}
	else if (offset >= 2 && offset < 5)
	{
		K051960_spriterombank[offset - 2] = data;
	}
	else
	{
//  popmessage("%04x: write %02x to 051937 address %x",cpu_get_pc(space->cpu),data,offset);
//logerror("%04x: write %02x to unknown 051937 address %x\n",cpu_get_pc(space->cpu),data,offset);
	}
}

READ16_HANDLER( K051937_word_r )
{
	return K051937_r(space,offset*2 + 1) | (K051937_r(space,offset*2) << 8);
}

WRITE16_HANDLER( K051937_word_w )
{
	if (ACCESSING_BITS_8_15)
		K051937_w(space,offset*2,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		K051937_w(space,offset*2 + 1,data & 0xff);
}


/*
 * Sprite Format
 * ------------------
 *
 * Byte | Bit(s)   | Use
 * -----+-76543210-+----------------
 *   0  | x------- | active (show this sprite)
 *   0  | -xxxxxxx | priority order
 *   1  | xxx----- | sprite size (see below)
 *   1  | ---xxxxx | sprite code (high 5 bits)
 *   2  | xxxxxxxx | sprite code (low 8 bits)
 *   3  | xxxxxxxx | "color", but depends on external connections (see below)
 *   4  | xxxxxx-- | zoom y (0 = normal, >0 = shrink)
 *   4  | ------x- | flip y
 *   4  | -------x | y position (high bit)
 *   5  | xxxxxxxx | y position (low 8 bits)
 *   6  | xxxxxx-- | zoom x (0 = normal, >0 = shrink)
 *   6  | ------x- | flip x
 *   6  | -------x | x position (high bit)
 *   7  | xxxxxxxx | x position (low 8 bits)
 *
 * Example of "color" field for Punk Shot:
 *   3  | x------- | shadow
 *   3  | -xx----- | priority
 *   3  | ---x---- | use second gfx ROM bank
 *   3  | ----xxxx | color code
 *
 * shadow enables transparent shadows. Note that it applies to pen 0x0f ONLY.
 * The rest of the sprite remains normal.
 * Note that Aliens also uses the shadow bit to select the second sprite bank.
 */

void K051960_sprites_draw(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int min_priority,int max_priority)
{
#define NUM_SPRITES 128
	int offs,pri_code;
	int sortedlist[NUM_SPRITES];
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	for (offs = 0;offs < NUM_SPRITES;offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (offs = 0;offs < 0x400;offs += 8)
	{
		if (K051960_ram[offs] & 0x80)
		{
			if (max_priority == -1)	/* draw front to back when using priority buffer */
				sortedlist[(K051960_ram[offs] & 0x7f) ^ 0x7f] = offs;
			else
				sortedlist[K051960_ram[offs] & 0x7f] = offs;
		}
	}

	for (pri_code = 0;pri_code < NUM_SPRITES;pri_code++)
	{
		int ox,oy,code,color,pri,shadow,size,w,h,x,y,flipx,flipy,zoomx,zoomy;
		/* sprites can be grouped up to 8x8. The draw order is
             0  1  4  5 16 17 20 21
             2  3  6  7 18 19 22 23
             8  9 12 13 24 25 28 29
            10 11 14 15 26 27 30 31
            32 33 36 37 48 49 52 53
            34 35 38 39 50 51 54 55
            40 41 44 45 56 57 60 61
            42 43 46 47 58 59 62 63
        */
		static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
		static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
		static const int width[8] =  { 1, 2, 1, 2, 4, 2, 4, 8 };
		static const int height[8] = { 1, 1, 2, 2, 2, 4, 4, 8 };


		offs = sortedlist[pri_code];
		if (offs == -1) continue;

		code = K051960_ram[offs+2] + ((K051960_ram[offs+1] & 0x1f) << 8);
		color = K051960_ram[offs+3] & 0xff;
		pri = 0;
		shadow = color & 0x80;
		(*K051960_callback)(&code,&color,&pri,&shadow);

		if (max_priority != -1)
			if (pri < min_priority || pri > max_priority) continue;

		size = (K051960_ram[offs+1] & 0xe0) >> 5;
		w = width[size];
		h = height[size];

		if (w >= 2) code &= ~0x01;
		if (h >= 2) code &= ~0x02;
		if (w >= 4) code &= ~0x04;
		if (h >= 4) code &= ~0x08;
		if (w >= 8) code &= ~0x10;
		if (h >= 8) code &= ~0x20;

		ox = (256 * K051960_ram[offs+6] + K051960_ram[offs+7]) & 0x01ff;
		oy = 256 - ((256 * K051960_ram[offs+4] + K051960_ram[offs+5]) & 0x01ff);
		ox += K051960_dx;
		oy += K051960_dy;
		flipx = K051960_ram[offs+6] & 0x02;
		flipy = K051960_ram[offs+4] & 0x02;
		zoomx = (K051960_ram[offs+6] & 0xfc) >> 2;
		zoomy = (K051960_ram[offs+4] & 0xfc) >> 2;
		zoomx = 0x10000 / 128 * (128 - zoomx);
		zoomy = 0x10000 / 128 * (128 - zoomy);

		if (K051960_spriteflip)
		{
			ox = 512 - (zoomx * w >> 12) - ox;
			oy = 256 - (zoomy * h >> 12) - oy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawmode_table[K051960_gfx->color_granularity-1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		if (zoomx == 0x10000 && zoomy == 0x10000)
		{
			int sx,sy;

			for (y = 0;y < h;y++)
			{
				sy = oy + 16 * y;

				for (x = 0;x < w;x++)
				{
					int c = code;

					sx = ox + 16 * x;
					if (flipx) c += xoffset[(w-1-x)];
					else c += xoffset[x];
					if (flipy) c += yoffset[(h-1-y)];
					else c += yoffset[y];

					if (max_priority == -1)
						pdrawgfx_transtable(bitmap,cliprect,K051960_gfx,
								c,
								color,
								flipx,flipy,
								sx & 0x1ff,sy,
								machine->priority_bitmap,pri,
								drawmode_table,machine->shadow_table);
					else
						drawgfx_transtable(bitmap,cliprect,K051960_gfx,
								c,
								color,
								flipx,flipy,
								sx & 0x1ff,sy,
								drawmode_table,machine->shadow_table);
				}
			}
		}
		else
		{
			int sx,sy,zw,zh;

			for (y = 0;y < h;y++)
			{
				sy = oy + ((zoomy * y + (1<<11)) >> 12);
				zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

				for (x = 0;x < w;x++)
				{
					int c = code;

					sx = ox + ((zoomx * x + (1<<11)) >> 12);
					zw = (ox + ((zoomx * (x+1) + (1<<11)) >> 12)) - sx;
					if (flipx) c += xoffset[(w-1-x)];
					else c += xoffset[x];
					if (flipy) c += yoffset[(h-1-y)];
					else c += yoffset[y];

					if (max_priority == -1)
						pdrawgfxzoom_transtable(bitmap,cliprect,K051960_gfx,
								c,
								color,
								flipx,flipy,
								sx & 0x1ff,sy,
								(zw << 16) / 16,(zh << 16) / 16,
								machine->priority_bitmap,pri,
								drawmode_table,machine->shadow_table);
					else
						drawgfxzoom_transtable(bitmap,cliprect,K051960_gfx,
								c,
								color,
								flipx,flipy,
								sx & 0x1ff,sy,
								(zw << 16) / 16,(zh << 16) / 16,
								drawmode_table,machine->shadow_table);
				}
			}
		}
	}
#if 0
if (input_code_pressed(machine, KEYCODE_D))
{
	FILE *fp;
	fp=fopen("SPRITE.DMP", "w+b");
	if (fp)
	{
		fwrite(K051960_ram, 0x400, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}

int K051960_is_IRQ_enabled(void)
{
	return K051960_irq_enabled;
}

int K051960_is_NMI_enabled(void)
{
	return K051960_nmi_enabled;
}

void K051960_set_sprite_offsets(int dx, int dy)
{
	K051960_dx = dx;
	K051960_dy = dy;
}




READ8_HANDLER( K052109_051960_r )
{
	if (K052109_RMRD_line == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return K051937_r(space,offset - 0x3800);
		else if (offset < 0x3c00)
			return K052109_r(space,offset);
		else
			return K051960_r(space,offset - 0x3c00);
	}
	else return K052109_r(space,offset);
}

WRITE8_HANDLER( K052109_051960_w )
{
	if (offset >= 0x3800 && offset < 0x3808)
		K051937_w(space,offset - 0x3800,data);
	else if (offset < 0x3c00)
		K052109_w(space,offset,data);
	else
		K051960_w(space,offset - 0x3c00,data);
}



/***************************************************************************/
/*                                                                         */
/*                      05324x Family Sprite Generators                    */
/*                                                                         */
/***************************************************************************/

static int K05324x_z_rejection;

/*
    In a K053247+K055555 setup objects with Z-code 0x00 should be ignored
    when PRFLIP is cleared, while objects with Z-code 0xff should be
    ignored when PRFLIP is set.

    These behaviors can also be seen in older K053245(6)+K053251 setups.
    Bucky'O Hare, The Simpsons and Sunset Riders rely on their implications
    to prepare and retire sprites. They probably apply to many other Konami
    games but it's hard to tell because most artifacts have been filtered
    by exclusion sort.

    A driver may call K05324x_set_z_rejection() to set which zcode to ignore.
    Parameter:
               -1 = accept all(default)
        0x00-0xff = zcode to ignore
*/
void K05324x_set_z_rejection(int zcode)
{
	K05324x_z_rejection = zcode;
}



/***************************************************************************/
/*                                                                         */
/*                                 053245                                  */
/*                                                                         */
/***************************************************************************/

#define MAX_K053245_CHIPS 2

static const char *K053245_memory_region[MAX_K053245_CHIPS];
static gfx_element *K053245_gfx[MAX_K053245_CHIPS];
static void (*K053245_callback[MAX_K053245_CHIPS])(int *code,int *color,int *priority);
static int K053244_rombank[MAX_K053245_CHIPS];
static int K053245_ramsize[MAX_K053245_CHIPS];
static UINT16 *K053245_ram[MAX_K053245_CHIPS], *K053245_buffer[MAX_K053245_CHIPS];
static UINT8 K053244_regs[MAX_K053245_CHIPS][0x10];
static int K053245_dx[MAX_K053245_CHIPS], K053245_dy[MAX_K053245_CHIPS];

void K053245_vh_start(running_machine *machine,int chip, const char *gfx_memory_region,int plane_order,
		void (*callback)(int *code,int *color,int *priority))
{
	int gfx_index;
	UINT32 total;
	static const gfx_layout spritelayout =
	{
		16,16,
		0,
		4,
  		{ 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7,
				8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
				16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
		128*8
	};

	assert_always(chip<MAX_K053245_CHIPS, "K053245_vh_start chip >= MAX_K053245_CHIPS");

	K053245_memory_region[chip]="gfx1";



	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine->gfx[gfx_index] == 0)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* decode the graphics */
	switch (plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, gfx_memory_region) / 128;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &spritelayout, 4);
		break;

	default:
		fatalerror("Unsupported plane_order");
	}

	if (VERBOSE && !(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");


	K05324x_z_rejection = -1;
	K053245_memory_region[chip] = gfx_memory_region;
	K053245_gfx[chip] = machine->gfx[gfx_index];
	K053245_callback[chip] = callback;
	K053244_rombank[chip] = 0;
	K053245_ramsize[chip] = 0x800;
	K053245_ram[chip] = auto_alloc_array_clear(machine, UINT16, K053245_ramsize[chip]/2);
	K053245_dx[chip] = K053245_dy[chip] = 0;

	K053245_buffer[chip] = auto_alloc_array_clear(machine, UINT16, K053245_ramsize[chip]/2);

	state_save_register_item_pointer(machine, "K053245", NULL, chip, K053245_ram[chip], K053245_ramsize[chip] / 2);
	state_save_register_item_pointer(machine, "K053245", NULL, chip, K053245_buffer[chip], K053245_ramsize[chip] / 2);
	state_save_register_item(machine, "K053245", NULL, chip, K053244_rombank[chip]);
	state_save_register_item(machine, "K053245", NULL, chip, K053245_dx[chip]);
	state_save_register_item(machine, "K053245", NULL, chip, K053245_dy[chip]);
	state_save_register_item_array(machine, "K053245", NULL, chip, K053244_regs[chip]);
}

void K053245_set_SpriteOffset(int chip,int offsx, int offsy)
{
	K053245_dx[chip] = offsx;
	K053245_dy[chip] = offsy;
}

READ16_HANDLER( K053245_word_r )
{
	return K053245_ram[0][offset];
}

WRITE16_HANDLER( K053245_word_w )
{
	COMBINE_DATA(K053245_ram[0]+offset);
}

READ8_HANDLER( K053245_r )
{
	if(offset & 1)
		return K053245_ram[0][offset>>1] & 0xff;
	else
		return (K053245_ram[0][offset>>1]>>8) & 0xff;
}


static void K053245_chip_w(int chip,int offset,int data)
{
	if(offset & 1)
		K053245_ram[chip][offset>>1] = (K053245_ram[chip][offset>>1] & 0xff00) | data;
	else
		K053245_ram[chip][offset>>1] = (K053245_ram[chip][offset>>1] & 0x00ff) | (data<<8);
}

WRITE8_HANDLER( K053245_w )
{
	K053245_chip_w(0,offset,data);
}

/* 2nd chip */
WRITE8_HANDLER( K053245_1_w )
{
	K053245_chip_w(1,offset,data);
}

void K053245_clear_buffer(int chip)
{
	int i, e;
	for (e=K053245_ramsize[chip]/2, i=0; i<e; i+=8) K053245_buffer[chip][i] = 0;
}

INLINE void K053245_update_buffer( int chip )
{
	memcpy(K053245_buffer[chip], K053245_ram[chip], K053245_ramsize[chip]);
}

static UINT8 K053244_chip_r (running_machine *machine, int chip, int offset)
{
	if ((K053244_regs[chip][5] & 0x10) && offset >= 0x0c && offset < 0x10)
	{
		int addr;

		addr = (K053244_rombank[chip] << 19) | ((K053244_regs[chip][11] & 0x7) << 18)
			| (K053244_regs[chip][8] << 10) | (K053244_regs[chip][9] << 2)
			| ((offset & 3) ^ 1);
		addr &= memory_region_length(machine, K053245_memory_region[chip])-1;

//  popmessage("%s: offset %02x addr %06x",cpuexec_describe_context(machine),offset&3,addr);

		return memory_region(machine, K053245_memory_region[chip])[addr];
	}
	else if (offset == 0x06)
	{
		K053245_update_buffer(chip);
		return 0;
	}
	else
	{
//logerror("%s: read from unknown 053244 address %x\n",cpuexec_describe_context(machine),offset);
		return 0;
	}
}

READ8_HANDLER( K053244_r )
{
	return K053244_chip_r(space->machine,0,offset);
}

static void K053244_chip_w(int chip, int offset, int data)
{
	K053244_regs[chip][offset] = data;

	switch(offset) {
	case 0x05: {
//      if (data & 0xc8)
//          popmessage("053244 reg 05 = %02x",data);
		/* bit 2 = unknown, Parodius uses it */
		/* bit 5 = unknown, Rollergames uses it */
//      logerror("%s: write %02x to 053244 address 5\n",cpuexec_describe_context(machine),data);
		break;
	}
	case 0x06:
		K053245_update_buffer(chip);
		break;
	}
}

WRITE8_HANDLER( K053244_w )
{
	K053244_chip_w(0,offset,data);
}

/* 2nd chip */
WRITE8_HANDLER( K053244_1_w )
{
	K053244_chip_w(1,offset,data);
}


READ16_HANDLER( K053244_lsb_r )
{
	return K053244_r(space, offset);
}

WRITE16_HANDLER( K053244_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		K053244_w(space, offset, data & 0xff);
}

READ16_HANDLER( K053244_word_r )
{
	return (K053244_r(space, offset*2)<<8)|K053244_r(space, offset*2+1);
}

WRITE16_HANDLER( K053244_word_w )
{
	if (ACCESSING_BITS_8_15)
		K053244_w(space, offset*2, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		K053244_w(space, offset*2+1, data & 0xff);
}

void K053244_bankselect(int chip, int bank)
{
	K053244_rombank[chip] = bank;
}

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | x--------------- | active (show this sprite)
 *   0  | -x-------------- | maintain aspect ratio (when set, zoom y acts on both axis)
 *   0  | --x------------- | flip y
 *   0  | ---x------------ | flip x
 *   0  | ----xxxx-------- | sprite size (see below)
 *   0  | ---------xxxxxxx | priority order
 *   1  | --xxxxxxxxxxxxxx | sprite code. We use an additional bit in TMNT2, but this is
 *                           probably not accurate (protection related so we can't verify)
 *   2  | ------xxxxxxxxxx | y position
 *   3  | ------xxxxxxxxxx | x position
 *   4  | xxxxxxxxxxxxxxxx | zoom y (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   5  | xxxxxxxxxxxxxxxx | zoom x (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   6  | ------x--------- | mirror y (top half is drawn as mirror image of the bottom)
 *   6  | -------x-------- | mirror x (right half is drawn as mirror image of the left)
 *   6  | --------x------- | shadow
 *   6  | ---------xxxxxxx | "color", but depends on external connections
 *   7  | ---------------- |
 *
 * shadow enables transparent shadows. Note that it applies to pen 0x0f ONLY.
 * The rest of the sprite remains normal.
 */

void K053245_sprites_draw(running_machine *machine, int chip, bitmap_t *bitmap,const rectangle *cliprect) //*
{
#define NUM_SPRITES 128
	int offs,pri_code,i;
	int sortedlist[NUM_SPRITES];
	int flipscreenX, flipscreenY, spriteoffsX, spriteoffsY;
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	flipscreenX = K053244_regs[chip][5] & 0x01;
	flipscreenY = K053244_regs[chip][5] & 0x02;
	spriteoffsX = (K053244_regs[chip][0] << 8) | K053244_regs[chip][1];
	spriteoffsY = (K053244_regs[chip][2] << 8) | K053244_regs[chip][3];

	for (offs = 0;offs < NUM_SPRITES;offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (i=K053245_ramsize[chip]/2, offs=0; offs<i; offs+=8)
	{
		pri_code = K053245_buffer[chip][offs];
		if (pri_code & 0x8000)
		{
			pri_code &= 0x007f;

			if (offs && pri_code == K05324x_z_rejection) continue;

			if (sortedlist[pri_code] == -1) sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES-1;pri_code >= 0;pri_code--)
	{
		int ox,oy,color,code,size,w,h,x,y,flipx,flipy,mirrorx,mirrory,shadow,zoomx,zoomy,pri;

		offs = sortedlist[pri_code];
		if (offs == -1) continue;

		/* the following changes the sprite draw order from
             0  1  4  5 16 17 20 21
             2  3  6  7 18 19 22 23
             8  9 12 13 24 25 28 29
            10 11 14 15 26 27 30 31
            32 33 36 37 48 49 52 53
            34 35 38 39 50 51 54 55
            40 41 44 45 56 57 60 61
            42 43 46 47 58 59 62 63

            to

             0  1  2  3  4  5  6  7
             8  9 10 11 12 13 14 15
            16 17 18 19 20 21 22 23
            24 25 26 27 28 29 30 31
            32 33 34 35 36 37 38 39
            40 41 42 43 44 45 46 47
            48 49 50 51 52 53 54 55
            56 57 58 59 60 61 62 63
        */

		/* NOTE: from the schematics, it looks like the top 2 bits should be ignored */
		/* (there are not output pins for them), and probably taken from the "color" */
		/* field to do bank switching. However this applies only to TMNT2, with its */
		/* protection mcu creating the sprite table, so we don't know where to fetch */
		/* the bits from. */
		code = K053245_buffer[chip][offs+1];
		code = ((code & 0xffe1) + ((code & 0x0010) >> 2) + ((code & 0x0008) << 1)
				 + ((code & 0x0004) >> 1) + ((code & 0x0002) << 2));
		color = K053245_buffer[chip][offs+6] & 0x00ff;
		pri = 0;

		(*K053245_callback[chip])(&code,&color,&pri);

		size = (K053245_buffer[chip][offs] & 0x0f00) >> 8;

		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* zoom control:
           0x40 = normal scale
          <0x40 enlarge (0x20 = double size)
          >0x40 reduce (0x80 = half size)
        */
		zoomy = K053245_buffer[chip][offs+4];
		if (zoomy > 0x2000) continue;
		if (zoomy) zoomy = (0x400000+zoomy/2) / zoomy;
		else zoomy = 2 * 0x400000;
		if ((K053245_buffer[chip][offs] & 0x4000) == 0)
		{
			zoomx = K053245_buffer[chip][offs+5];
			if (zoomx > 0x2000) continue;
			if (zoomx) zoomx = (0x400000+zoomx/2) / zoomx;
			else zoomx = 2 * 0x400000;
//          else zoomx = zoomy; /* workaround for TMNT2 */
		}
		else zoomx = zoomy;

		ox = K053245_buffer[chip][offs+3] + spriteoffsX;
		oy = K053245_buffer[chip][offs+2];

		ox += K053245_dx[chip];
		oy += K053245_dy[chip];

		flipx = K053245_buffer[chip][offs] & 0x1000;
		flipy = K053245_buffer[chip][offs] & 0x2000;
		mirrorx = K053245_buffer[chip][offs+6] & 0x0100;
		if (mirrorx) flipx = 0; // documented and confirmed
		mirrory = K053245_buffer[chip][offs+6] & 0x0200;
		shadow = K053245_buffer[chip][offs+6] & 0x0080;

		if (flipscreenX)
		{
			ox = 512 - ox;
			if (!mirrorx) flipx = !flipx;
		}
		if (flipscreenY)
		{
			oy = -oy;
			if (!mirrory) flipy = !flipy;
		}

		ox = (ox + 0x5d) & 0x3ff;
		if (ox >= 768) ox -= 1024;
		oy = (-(oy + spriteoffsY + 0x07)) & 0x3ff;
		if (oy >= 640) oy -= 1024;

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		drawmode_table[K053245_gfx[chip]->color_granularity-1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0;y < h;y++)
		{
			int sx,sy,zw,zh;

			sy = oy + ((zoomy * y + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

			for (x = 0;x < w;x++)
			{
				int c,fx,fy;

				sx = ox + ((zoomx * x + (1<<11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1<<11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ (2*x < w))
					{
						/* mirror left/right */
						c += (w-x-1);
						fx = 1;
					}
					else
					{
						c += x;
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += w-1-x;
					else c += x;
					fx = flipx;
				}
				if (mirrory)
				{
					if ((flipy == 0) ^ (2*y >= h))
					{
						/* mirror top/bottom */
						c += 8*(h-y-1);
						fy = 1;
					}
					else
					{
						c += 8*y;
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += 8*(h-1-y);
					else c += 8*y;
					fy = flipy;
				}

				/* the sprite can start at any point in the 8x8 grid, but it must stay */
				/* in a 64 entries window, wrapping around at the edges. The animation */
				/* at the end of the saloon level in Sunset Riders breaks otherwise. */
				c = (c & 0x3f) | (code & ~0x3f);

				if (zoomx == 0x10000 && zoomy == 0x10000)
				{
					pdrawgfx_transtable(bitmap,cliprect,K053245_gfx[chip],
							c,
							color,
							fx,fy,
							sx,sy,
							machine->priority_bitmap,pri,
							drawmode_table,machine->shadow_table);
				}
				else
				{
					pdrawgfxzoom_transtable(bitmap,cliprect,K053245_gfx[chip],
							c,
							color,
							fx,fy,
							sx,sy,
							(zw << 16) / 16,(zh << 16) / 16,
							machine->priority_bitmap,pri,
							drawmode_table,machine->shadow_table);

				}
			}
		}
	}
#if 0
if (input_code_pressed(machine, KEYCODE_D))
{
	FILE *fp;
	fp=fopen("SPRITE.DMP", "w+b");
	if (fp)
	{
		fwrite(K053245_buffer, 0x800, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}

/* Lethal Enforcers has 2 of these chips hooked up in parallel to give 6bpp gfx.. lets cheat a
  bit and make emulating it a little less messy by using a custom function instead */
void K053245_sprites_draw_lethal(running_machine *machine,int chip, bitmap_t *bitmap,const rectangle *cliprect) //*
{
#define NUM_SPRITES 128
	int offs,pri_code,i;
	int sortedlist[NUM_SPRITES];
	int flipscreenX, flipscreenY, spriteoffsX, spriteoffsY;
	UINT8 drawmode_table[256];

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;

	flipscreenX = K053244_regs[chip][5] & 0x01;
	flipscreenY = K053244_regs[chip][5] & 0x02;
	spriteoffsX = (K053244_regs[chip][0] << 8) | K053244_regs[chip][1];
	spriteoffsY = (K053244_regs[chip][2] << 8) | K053244_regs[chip][3];

	for (offs = 0;offs < NUM_SPRITES;offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (i=K053245_ramsize[chip]/2, offs=0; offs<i; offs+=8)
	{
		pri_code = K053245_buffer[chip][offs];
		if (pri_code & 0x8000)
		{
			pri_code &= 0x007f;

			if (offs && pri_code == K05324x_z_rejection) continue;

			if (sortedlist[pri_code] == -1) sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES-1;pri_code >= 0;pri_code--)
	{
		int ox,oy,color,code,size,w,h,x,y,flipx,flipy,mirrorx,mirrory,shadow,zoomx,zoomy,pri;

		offs = sortedlist[pri_code];
		if (offs == -1) continue;

		/* the following changes the sprite draw order from
             0  1  4  5 16 17 20 21
             2  3  6  7 18 19 22 23
             8  9 12 13 24 25 28 29
            10 11 14 15 26 27 30 31
            32 33 36 37 48 49 52 53
            34 35 38 39 50 51 54 55
            40 41 44 45 56 57 60 61
            42 43 46 47 58 59 62 63

            to

             0  1  2  3  4  5  6  7
             8  9 10 11 12 13 14 15
            16 17 18 19 20 21 22 23
            24 25 26 27 28 29 30 31
            32 33 34 35 36 37 38 39
            40 41 42 43 44 45 46 47
            48 49 50 51 52 53 54 55
            56 57 58 59 60 61 62 63
        */

		/* NOTE: from the schematics, it looks like the top 2 bits should be ignored */
		/* (there are not output pins for them), and probably taken from the "color" */
		/* field to do bank switching. However this applies only to TMNT2, with its */
		/* protection mcu creating the sprite table, so we don't know where to fetch */
		/* the bits from. */
		code = K053245_buffer[chip][offs+1];
		code = ((code & 0xffe1) + ((code & 0x0010) >> 2) + ((code & 0x0008) << 1)
				 + ((code & 0x0004) >> 1) + ((code & 0x0002) << 2));
		color = K053245_buffer[chip][offs+6] & 0x00ff;
		pri = 0;

		(*K053245_callback[chip])(&code,&color,&pri);

		size = (K053245_buffer[chip][offs] & 0x0f00) >> 8;

		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* zoom control:
           0x40 = normal scale
          <0x40 enlarge (0x20 = double size)
          >0x40 reduce (0x80 = half size)
        */
		zoomy = K053245_buffer[chip][offs+4];
		if (zoomy > 0x2000) continue;
		if (zoomy) zoomy = (0x400000+zoomy/2) / zoomy;
		else zoomy = 2 * 0x400000;
		if ((K053245_buffer[chip][offs] & 0x4000) == 0)
		{
			zoomx = K053245_buffer[chip][offs+5];
			if (zoomx > 0x2000) continue;
			if (zoomx) zoomx = (0x400000+zoomx/2) / zoomx;
			else zoomx = 2 * 0x400000;
//          else zoomx = zoomy; /* workaround for TMNT2 */
		}
		else zoomx = zoomy;

		ox = K053245_buffer[chip][offs+3] + spriteoffsX;
		oy = K053245_buffer[chip][offs+2];

		ox += K053245_dx[chip];
		oy += K053245_dy[chip];

		flipx = K053245_buffer[chip][offs] & 0x1000;
		flipy = K053245_buffer[chip][offs] & 0x2000;
		mirrorx = K053245_buffer[chip][offs+6] & 0x0100;
		if (mirrorx) flipx = 0; // documented and confirmed
		mirrory = K053245_buffer[chip][offs+6] & 0x0200;
		shadow = K053245_buffer[chip][offs+6] & 0x0080;

		if (flipscreenX)
		{
			ox = 512 - ox;
			if (!mirrorx) flipx = !flipx;
		}
		if (flipscreenY)
		{
			oy = -oy;
			if (!mirrory) flipy = !flipy;
		}

		ox = (ox + 0x5d) & 0x3ff;
		if (ox >= 768) ox -= 1024;
		oy = (-(oy + spriteoffsY + 0x07)) & 0x3ff;
		if (oy >= 640) oy -= 1024;

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		drawmode_table[machine->gfx[0]->color_granularity-1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0;y < h;y++)
		{
			int sx,sy,zw,zh;

			sy = oy + ((zoomy * y + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

			for (x = 0;x < w;x++)
			{
				int c,fx,fy;

				sx = ox + ((zoomx * x + (1<<11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1<<11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ (2*x < w))
					{
						/* mirror left/right */
						c += (w-x-1);
						fx = 1;
					}
					else
					{
						c += x;
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += w-1-x;
					else c += x;
					fx = flipx;
				}
				if (mirrory)
				{
					if ((flipy == 0) ^ (2*y >= h))
					{
						/* mirror top/bottom */
						c += 8*(h-y-1);
						fy = 1;
					}
					else
					{
						c += 8*y;
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += 8*(h-1-y);
					else c += 8*y;
					fy = flipy;
				}

				/* the sprite can start at any point in the 8x8 grid, but it must stay */
				/* in a 64 entries window, wrapping around at the edges. The animation */
				/* at the end of the saloon level in Sunset Riders breaks otherwise. */
				c = (c & 0x3f) | (code & ~0x3f);

				if (zoomx == 0x10000 && zoomy == 0x10000)
				{
					pdrawgfx_transtable(bitmap,cliprect,machine->gfx[0], /* hardcoded to 0 (decoded 6bpp gfx) for le */
							c,
							color,
							fx,fy,
							sx,sy,
							machine->priority_bitmap,pri,
							drawmode_table,machine->shadow_table);
				}
				else
				{
					pdrawgfxzoom_transtable(bitmap,cliprect,machine->gfx[0],  /* hardcoded to 0 (decoded 6bpp gfx) for le */
							c,
							color,
							fx,fy,
							sx,sy,
							(zw << 16) / 16,(zh << 16) / 16,
							machine->priority_bitmap,pri,
							drawmode_table,machine->shadow_table);

				}
			}
		}
	}
#if 0
if (input_code_pressed(machine, KEYCODE_D))
{
	FILE *fp;
	fp=fopen("SPRITE.DMP", "w+b");
	if (fp)
	{
		fwrite(K053245_buffer, 0x800, 1, fp);
		popmessage("saved");
		fclose(fp);
	}
}
#endif
#undef NUM_SPRITES
}


/***************************************************************************/
/*                                                                         */
/*                                 053246/053247                           */
/*                                                                         */
/***************************************************************************/

static const char *K053247_memory_region;
static int K053247_dx, K053247_dy, K053247_wraparound;
static UINT8  K053246_regs[8];
static UINT16 K053247_regs[16];
static UINT16 *K053247_ram=0;
static gfx_element *K053247_gfx;
static void (*K053247_callback)(int *code,int *color,int *priority);
static UINT8 K053246_OBJCHA_line;

void K053247_export_config(UINT16 **ram, gfx_element **gfx, void (**callback)(int *, int *, int *), int *dx, int *dy)
{
	if(ram)
		*ram = K053247_ram;
	if(gfx)
		*gfx = K053247_gfx;
	if(callback)
		*callback = K053247_callback;
	if(dx)
		*dx = K053247_dx;
	if(dy)
		*dy = K053247_dy;
}

int K053246_read_register(int regnum) { return(K053246_regs[regnum]); }
int K053247_read_register(int regnum) { return(K053247_regs[regnum]); }

void K053247_set_SpriteOffset(int offsx, int offsy)
{
	K053247_dx = offsx;
	K053247_dy = offsy;
}

void K053247_wraparound_enable(int status)
{
	K053247_wraparound = status;
}

void K053247_vh_start(running_machine *machine, const char *gfx_memory_region, int dx, int dy, int plane_order,
					 void (*callback)(int *code,int *color,int *priority))
{
	int gfx_index;
	UINT32 total;
	static const gfx_layout spritelayout =
	{
		16,16,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
				10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
				8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
		128*8
	};


	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine->gfx[gfx_index] == 0)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* decode the graphics */
	switch (plane_order)
	{
	case NORMAL_PLANE_ORDER:
		total = memory_region_length(machine, gfx_memory_region) / 128;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &spritelayout, 4);
		break;

	default:
		fatalerror("Unsupported plane_order");
	}

	if (VERBOSE)
	{
	if (video_screen_get_format(machine->primary_screen) == BITMAP_FORMAT_RGB32)
	{
		if ((machine->config->video_attributes & (VIDEO_HAS_SHADOWS|VIDEO_HAS_HIGHLIGHTS)) != VIDEO_HAS_SHADOWS+VIDEO_HAS_HIGHLIGHTS)
			popmessage("driver missing SHADOWS or HIGHLIGHTS flag");
	}
	else
	{
		if (!(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
			popmessage("driver should use VIDEO_HAS_SHADOWS");
	}
	}

	K053247_dx = dx;
	K053247_dy = dy;
	K053247_wraparound = 1;
	K05324x_z_rejection = -1;
	K053247_memory_region = gfx_memory_region;
	K053247_gfx = machine->gfx[gfx_index];
	K053247_callback = callback;
	K053246_OBJCHA_line = CLEAR_LINE;
	K053247_ram = auto_alloc_array(machine, UINT16, 0x1000/2);

	memset(K053247_ram,  0, 0x1000);
	memset(K053246_regs, 0, 8);
	memset(K053247_regs, 0, 32);

	state_save_register_global_pointer(machine, K053247_ram, 0x800);
	state_save_register_global_array(machine, K053246_regs);
	state_save_register_global_array(machine, K053247_regs);
	state_save_register_global(machine, K053246_OBJCHA_line);
}

/* K055673 used with the 54246 in PreGX/Run and Gun/System GX games */
void K055673_vh_start(running_machine *machine, const char *gfx_memory_region, int layout, int dx, int dy, void (*callback)(int *code,int *color,int *priority))
{
	int gfx_index;
	UINT32 total;

	static const gfx_layout spritelayout =	/* System GX sprite layout */
	{
		16,16,
		0,
		5,
		{ 32, 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 40, 41, 42, 43, 44, 45, 46, 47 },
		{ 0, 10*8, 10*8*2, 10*8*3, 10*8*4, 10*8*5, 10*8*6, 10*8*7, 10*8*8,
		  10*8*9, 10*8*10, 10*8*11, 10*8*12, 10*8*13, 10*8*14, 10*8*15 },
		16*16*5
	};
	static const gfx_layout spritelayout2 =	/* Run and Gun sprite layout */
	{
		16,16,
		0,
		4,
		{ 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39 },
		{ 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 },
		16*16*4
	};
	static const gfx_layout spritelayout3 =	/* Lethal Enforcers II sprite layout */
	{
		16,16,
		0,
		8,
		{ 8*1,8*0,8*3,8*2,8*5,8*4,8*7,8*6 },
		{  0,1,2,3,4,5,6,7,64+0,64+1,64+2,64+3,64+4,64+5,64+6,64+7 },
		{ 128*0, 128*1, 128*2,  128*3,  128*4,  128*5,  128*6,  128*7,
		  128*8, 128*9, 128*10, 128*11, 128*12, 128*13, 128*14, 128*15 },
		128*16
	};
	static const gfx_layout spritelayout4 =	/* System GX 6bpp sprite layout */
	{
		16,16,
		0,
		6,
		{ 40, 32, 24, 16, 8, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7, 48, 49, 50, 51, 52, 53, 54, 55 },
		{ 0, 12*8, 12*8*2, 12*8*3, 12*8*4, 12*8*5, 12*8*6, 12*8*7, 12*8*8,
		  12*8*9, 12*8*10, 12*8*11, 12*8*12, 12*8*13, 12*8*14, 12*8*15 },
		16*16*6
	};
	UINT8 *s1, *s2, *d;
	long i;
	UINT16 *K055673_rom;
	int size4;

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine->gfx[gfx_index] == 0)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	K055673_rom = (UINT16 *)memory_region(machine, gfx_memory_region);

	/* decode the graphics */
	switch(layout)
	{
	case K055673_LAYOUT_GX:
		size4 = (memory_region_length(machine, gfx_memory_region)/(1024*1024))/5;
		size4 *= 4*1024*1024;
		/* set the # of tiles based on the 4bpp section */
		K055673_rom = auto_alloc_array(machine, UINT16, size4 * 5 / 2);
		d = (UINT8 *)K055673_rom;
		// now combine the graphics together to form 5bpp
		s1 = memory_region(machine, gfx_memory_region); // 4bpp area
		s2 = s1 + (size4);	 // 1bpp area
		for (i = 0; i < size4; i+= 4)
		{
			*d++ = *s1++;
			*d++ = *s1++;
			*d++ = *s1++;
			*d++ = *s1++;
			*d++ = *s2++;
		}

		total = size4 / 128;
		decode_gfx(machine, gfx_index, (UINT8 *)K055673_rom, total, &spritelayout, 4);
		break;

	case K055673_LAYOUT_RNG:
		total = memory_region_length(machine, gfx_memory_region) / (16*16/2);
		decode_gfx(machine, gfx_index, (UINT8 *)K055673_rom, total, &spritelayout2, 4);
		break;

	case K055673_LAYOUT_LE2:
		total = memory_region_length(machine, gfx_memory_region) / (16*16);
		decode_gfx(machine, gfx_index, (UINT8 *)K055673_rom, total, &spritelayout3, 4);
		break;

	case K055673_LAYOUT_GX6:
		total = memory_region_length(machine, gfx_memory_region) / (16*16*6/8);
		decode_gfx(machine, gfx_index, (UINT8 *)K055673_rom, total, &spritelayout4, 4);
		break;

	default:
		fatalerror("Unsupported layout");
	}

	if (VERBOSE && !(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		popmessage("driver should use VIDEO_HAS_SHADOWS");

	K053247_dx = dx;
	K053247_dy = dy;
	K053247_wraparound = 1;
	K05324x_z_rejection = -1;
	K053247_memory_region = gfx_memory_region;
	K053247_gfx = machine->gfx[gfx_index];
	K053247_callback = callback;
	K053246_OBJCHA_line = CLEAR_LINE;
	K053247_ram = auto_alloc_array(machine, UINT16, 0x1000/2);

	memset(K053247_ram,  0, 0x1000);
	memset(K053246_regs, 0, 8);
	memset(K053247_regs, 0, 32);

	state_save_register_global_pointer(machine, K053247_ram, 0x800);
	state_save_register_global_array(machine, K053246_regs);
	state_save_register_global_array(machine, K053247_regs);
	state_save_register_global(machine, K053246_OBJCHA_line);
}

WRITE16_HANDLER( K053247_reg_word_w ) // write-only OBJSET2 registers (see p.43 table 6.1)
{
	COMBINE_DATA(K053247_regs + offset);
}

WRITE32_HANDLER( K053247_reg_long_w )
{
	offset <<= 1;
	COMBINE_DATA(K053247_regs + offset + 1);
	mem_mask >>= 16;
	data >>= 16;
	COMBINE_DATA(K053247_regs + offset);
}

READ16_HANDLER( K053247_word_r )
{
	return K053247_ram[offset];
}

WRITE16_HANDLER( K053247_word_w )
{
	COMBINE_DATA(K053247_ram + offset);
}

READ32_HANDLER( K053247_long_r )
{
	return K053247_ram[offset*2+1] | (K053247_ram[offset*2]<<16);
}

WRITE32_HANDLER( K053247_long_w )
{
	offset <<= 1;
	COMBINE_DATA(K053247_ram + offset + 1);
	mem_mask >>= 16;
	data >>= 16;
	COMBINE_DATA(K053247_ram + offset);
}

READ8_HANDLER( K053247_r )
{
	int offs = offset >> 1;

	if (offset & 1)
		return(K053247_ram[offs] & 0xff);
	else
		return(K053247_ram[offs] >> 8);
}

WRITE8_HANDLER( K053247_w )
{
	int offs = offset >> 1;

	if (offset & 1)
		K053247_ram[offs] = (K053247_ram[offs] & 0xff00) | data;
	else
		K053247_ram[offs] = (K053247_ram[offs] & 0x00ff) | (data<<8);
}

// Mystic Warriors hardware games support a non-OBJCHA based ROM readback
// write the address to the 246 as usual, but there's a completely separate ROM
// window that works without needing an OBJCHA line.
// in this window, +0 = 32 bits from one set of ROMs, and +8 = 32 bits from another set
READ16_HANDLER( K055673_rom_word_r )	// 5bpp
{
	UINT8 *ROM8 = (UINT8 *)memory_region(space->machine, K053247_memory_region);
	UINT16 *ROM = (UINT16 *)memory_region(space->machine, K053247_memory_region);
	int size4 = (memory_region_length(space->machine, K053247_memory_region)/(1024*1024))/5;
	int romofs;

	size4 *= 4*1024*1024;	// get offset to 5th bit
	ROM8 += size4;

	romofs = K053246_regs[6]<<16 | K053246_regs[7]<<8 | K053246_regs[4];

	switch (offset)
	{
		case 0:	// 20k / 36u
			return ROM[romofs+2];
		case 1:	// 17k / 36y
			return ROM[romofs+3];
		case 2: // 10k / 32y
		case 3:
			romofs /= 2;
			return ROM8[romofs+1];
		case 4:	// 22k / 34u
			return ROM[romofs];
		case 5:	// 19k / 34y
			return ROM[romofs+1];
		case 6:	// 12k / 29y
		case 7:
			romofs /= 2;
			return ROM8[romofs];
		default:
			LOG(("55673_rom_word_r: Unknown read offset %x\n", offset));
			break;
	}

	return 0;
}

READ16_HANDLER( K055673_GX6bpp_rom_word_r )
{
	UINT16 *ROM = (UINT16 *)memory_region(space->machine, K053247_memory_region);
	int romofs;

	romofs = K053246_regs[6]<<16 | K053246_regs[7]<<8 | K053246_regs[4];

	romofs /= 4;	// romofs increments 4 at a time
	romofs *= 12/2;	// each increment of romofs = 12 new bytes (6 new words)

	switch (offset)
	{
		case 0:
			return ROM[romofs+3];
		case 1:
			return ROM[romofs+4];
		case 2:
		case 3:
			return ROM[romofs+5];
		case 4:
			return ROM[romofs];
		case 5:
			return ROM[romofs+1];
		case 6:
		case 7:
			return ROM[romofs+2];
		default:
			LOG(("55673_rom_word_r: Unknown read offset %x (PC=%x)\n", offset, cpu_get_pc(space->cpu)));
			break;
	}

	return 0;
}

READ8_HANDLER( K053246_r )
{
	if (K053246_OBJCHA_line == ASSERT_LINE)
	{
		int addr;

		addr = (K053246_regs[6] << 17) | (K053246_regs[7] << 9) | (K053246_regs[4] << 1) | ((offset & 1) ^ 1);
		addr &= memory_region_length(space->machine, K053247_memory_region)-1;
		if (VERBOSE)
			popmessage("%04x: offset %02x addr %06x",cpu_get_pc(space->cpu),offset,addr);
		return memory_region(space->machine, K053247_memory_region)[addr];
	}
	else
	{
		LOG(("%04x: read from unknown 053246 address %x\n",cpu_get_pc(space->cpu),offset));
		return 0;
	}
}

WRITE8_HANDLER( K053246_w )
{
	K053246_regs[offset] = data;
}

READ16_HANDLER( K053246_word_r )
{
	offset <<= 1;
	return K053246_r(space, offset + 1) | (K053246_r(space, offset) << 8);
}

WRITE16_HANDLER( K053246_word_w )
{
	if (ACCESSING_BITS_8_15)
		K053246_w(space, offset<<1,(data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		K053246_w(space, (offset<<1) + 1,data & 0xff);
}

READ32_HANDLER( K053246_long_r )
{
	offset <<= 1;
	return (K053246_word_r(space, offset+1, 0xffff) | K053246_word_r(space, offset, 0xffff)<<16);
}

WRITE32_HANDLER( K053246_long_w )
{
	offset <<= 1;
	K053246_word_w(space, offset, data>>16, mem_mask >> 16);
	K053246_word_w(space, offset+1, data, mem_mask);
}

void K053246_set_OBJCHA_line(int state)
{
	K053246_OBJCHA_line = state;
}

int K053246_is_IRQ_enabled(void)
{
	// This bit enables obj DMA rather than obj IRQ even though the two functions usually coincide.
	return K053246_regs[5] & 0x10;
}

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | x--------------- | active (show this sprite)
 *   0  | -x-------------- | maintain aspect ratio (when set, zoom y acts on both axis)
 *   0  | --x------------- | flip y
 *   0  | ---x------------ | flip x
 *   0  | ----xxxx-------- | sprite size (see below)
 *   0  | --------xxxxxxxx | zcode
 *   1  | xxxxxxxxxxxxxxxx | sprite code
 *   2  | ------xxxxxxxxxx | y position
 *   3  | ------xxxxxxxxxx | x position
 *   4  | xxxxxxxxxxxxxxxx | zoom y (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   5  | xxxxxxxxxxxxxxxx | zoom x (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   6  | x--------------- | mirror y (top half is drawn as mirror image of the bottom)
 *   6  | -x-------------- | mirror x (right half is drawn as mirror image of the left)
 *   6  | --xx------------ | reserved (sprites with these two bits set don't seem to be graphics data at all)
 *   6  | ----xx---------- | shadow code: 0=off, 0x400=preset1, 0x800=preset2, 0xc00=preset3
 *   6  | ------xx-------- | effect code: flicker, upper palette, full shadow...etc. (game dependent)
 *   6  | --------xxxxxxxx | "color", but depends on external connections (implies priority)
 *   7  | xxxxxxxxxxxxxxxx | game dependent
 *
 * shadow enables transparent shadows. Note that it applies to the last sprite pen ONLY.
 * The rest of the sprite remains normal.
 */

void K053247_sprites_draw(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect) //*
{
#define NUM_SPRITES 256

	/* sprites can be grouped up to 8x8. The draw order is
         0  1  4  5 16 17 20 21
         2  3  6  7 18 19 22 23
         8  9 12 13 24 25 28 29
        10 11 14 15 26 27 30 31
        32 33 36 37 48 49 52 53
        34 35 38 39 50 51 54 55
        40 41 44 45 56 57 60 61
        42 43 46 47 58 59 62 63
    */
	static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
	static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };

	int sortedlist[NUM_SPRITES];
	int offs,zcode;
	int ox,oy,color,code,size,w,h,x,y,xa,ya,flipx,flipy,mirrorx,mirrory,shadow,zoomx,zoomy,primask;
	int shdmask,nozoom,count,temp;

	int flipscreenx = K053246_regs[5] & 0x01;
	int flipscreeny = K053246_regs[5] & 0x02;
	int offx = (short)((K053246_regs[0] << 8) | K053246_regs[1]);
	int offy = (short)((K053246_regs[2] << 8) | K053246_regs[3]);

	int screen_width = video_screen_get_width(machine->primary_screen);
	UINT8 drawmode_table[256];
	UINT8 shadowmode_table[256];
	UINT8 *whichtable;

	memset(drawmode_table, DRAWMODE_SOURCE, sizeof(drawmode_table));
	drawmode_table[0] = DRAWMODE_NONE;
	memset(shadowmode_table, DRAWMODE_SHADOW, sizeof(shadowmode_table));
	shadowmode_table[0] = DRAWMODE_NONE;

	/*
        safeguard older drivers missing any of the following video attributes:

        VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS
    */
	if (machine->config->video_attributes & VIDEO_HAS_SHADOWS)
	{
		if (bitmap->bpp == 32 && (machine->config->video_attributes & VIDEO_HAS_HIGHLIGHTS))
			shdmask = 3; // enable all shadows and highlights
		else
			shdmask = 0; // enable default shadows
	}
	else
		shdmask = -1; // disable everything

	/*
        The K053247 does not draw pixels on top of those with equal or smaller Z-values
        regardless of priority. Embedded shadows inherit Z-values from their host sprites
        but do not assume host priorities unless explicitly told. In other words shadows
        can have priorities different from that of normal pens in the same sprite,
        in addition to the ability of masking themselves from specific layers or pixels
        on the other sprites.

        In front-to-back rendering, sprites cannot sandwich between alpha blended layers
        or the draw code will have to figure out the percentage opacities of what is on
        top and beneath each sprite pixel and blend the target accordingly. The process
        is overly demanding for realtime software and is thus another shortcoming of
        pdrawgfx and pixel based mixers. Even mahjong games with straight forward video
        subsystems are feeling the impact by which the girls cannot appear under
        translucent dialogue boxes.

        These are a small part of the K053247's feature set but many games expect them
        to be the minimum compliances. The specification will undoubtedly require
        redesigning the priority system from the ground up. Drawgfx.c and tilemap.c must
        also undergo heavy facelifts but in the end the changes could hurt simpler games
        more than they help complex systems; therefore the new engine should remain
        completely stand alone and self-contained. Implementation details are being
        hammered down but too early to make propositions.
    */

	// Prebuild a sorted table by descending Z-order.
	zcode = K05324x_z_rejection;
	offs = count = 0;

	if (zcode == -1)
	{
		for (; offs<0x800; offs+=8)
			if (K053247_ram[offs] & 0x8000) sortedlist[count++] = offs;
	}
	else
	{
		for (; offs<0x800; offs+=8)
			if ((K053247_ram[offs] & 0x8000) && ((K053247_ram[offs] & 0xff) != zcode)) sortedlist[count++] = offs;
	}

	w = count;
	count--;
	h = count;

	if (!(K053247_regs[0xc/2] & 0x10))
	{
		// sort objects in decending order(smaller z closer) when OPSET PRI is clear
		for (y=0; y<h; y++)
		{
			offs = sortedlist[y];
			zcode = K053247_ram[offs] & 0xff;
			for (x=y+1; x<w; x++)
			{
				temp = sortedlist[x];
				code = K053247_ram[temp] & 0xff;
				if (zcode <= code) { zcode = code; sortedlist[x] = offs; sortedlist[y] = offs = temp; }
			}
		}
	}
	else
	{
		// sort objects in ascending order(bigger z closer) when OPSET PRI is set
		for (y=0; y<h; y++)
		{
			offs = sortedlist[y];
			zcode = K053247_ram[offs] & 0xff;
			for (x=y+1; x<w; x++)
			{
				temp = sortedlist[x];
				code = K053247_ram[temp] & 0xff;
				if (zcode >= code) { zcode = code; sortedlist[x] = offs; sortedlist[y] = offs = temp; }
			}
		}
	}

	for (; count>=0; count--)
	{
		offs = sortedlist[count];

		code = K053247_ram[offs+1];
		shadow = color = K053247_ram[offs+6];
		primask = 0;

		(*K053247_callback)(&code,&color,&primask);

		temp = K053247_ram[offs];

		size = (temp & 0x0f00) >> 8;
		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* the sprite can start at any point in the 8x8 grid. We have to */
		/* adjust the offsets to draw it correctly. Simpsons does this all the time. */
		xa = 0;
		ya = 0;
		if (code & 0x01) xa += 1;
		if (code & 0x02) ya += 1;
		if (code & 0x04) xa += 2;
		if (code & 0x08) ya += 2;
		if (code & 0x10) xa += 4;
		if (code & 0x20) ya += 4;
		code &= ~0x3f;

		oy = (short)K053247_ram[offs+2];
		ox = (short)K053247_ram[offs+3];

		if (K053247_wraparound)
		{
			offx &= 0x3ff;
			offy &= 0x3ff;
			oy &= 0x3ff;
			ox &= 0x3ff;
		}

		/* zoom control:
           0x40 = normal scale
          <0x40 enlarge (0x20 = double size)
          >0x40 reduce (0x80 = half size)
        */
		y = zoomy = K053247_ram[offs+4] & 0x3ff;
		if (zoomy) zoomy = (0x400000+(zoomy>>1)) / zoomy; else zoomy = 0x800000;
		if (!(temp & 0x4000))
		{
			x = zoomx = K053247_ram[offs+5] & 0x3ff;
			if (zoomx) zoomx = (0x400000+(zoomx>>1)) / zoomx;
			else zoomx = 0x800000;
		}
		else { zoomx = zoomy; x = y; }

// ************************************************************************************
//  for Escape Kids (GX975)
// ************************************************************************************
//    Escape Kids use 053246 #5 register's UNKNOWN Bit #5, #3 and #2.
//    Bit #5, #3, #2 always set "1".
//    Maybe, Bit #5 or #3 or #2 or combination means "FIX SPRITE WIDTH TO HALF" ?????
//    Below 7 lines supports this 053246's(???) function.
//    Don't rely on it, Please.  But, Escape Kids works correctly!
// ************************************************************************************
		if ( K053246_regs[5] & 0x08 ) // Check only "Bit #3 is '1'?" (NOTE: good guess)
		{
			zoomx >>= 1;		// Fix sprite width to HALF size
			ox = (ox >> 1) + 1;	// Fix sprite draw position
			if (flipscreenx) ox += screen_width;
			nozoom = 0;
		}
		else
			nozoom = (x == 0x40 && y == 0x40);

		flipx = temp & 0x1000;
		flipy = temp & 0x2000;
		mirrorx = shadow & 0x4000;
		if (mirrorx) flipx = 0; // documented and confirmed
		mirrory = shadow & 0x8000;

		whichtable = drawmode_table;
		if (color == -1)
		{
			// drop the entire sprite to shadow unconditionally
			if (shdmask < 0) continue;
			color = 0;
			shadow = -1;
			whichtable = shadowmode_table;
			palette_set_shadow_mode(machine, 0);
		}
		else
		{
			if (shdmask >= 0)
			{
				shadow = (color & K053247_CUSTOMSHADOW) ? (color>>K053247_SHDSHIFT) : (shadow>>10);
				if (shadow &= 3) palette_set_shadow_mode(machine, (shadow-1) & shdmask);
			}
			else
				shadow = 0;
		}

		color &= 0xffff; // strip attribute flags

		if (flipscreenx)
		{
			ox = -ox;
			if (!mirrorx) flipx = !flipx;
		}
		if (flipscreeny)
		{
			oy = -oy;
			if (!mirrory) flipy = !flipy;
		}

		// apply wrapping and global offsets
		if (K053247_wraparound)
		{
			ox = ( ox - offx) & 0x3ff;
			oy = (-oy - offy) & 0x3ff;
			if (ox >= 0x300) ox -= 0x400;
			if (oy >= 0x280) oy -= 0x400;
		}
		else
		{
			ox =  ox - offx;
			oy = -oy - offy;
		}
		ox += K053247_dx;
		oy -= K053247_dy;

		// apply global and display window offsets

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		drawmode_table[K053247_gfx->color_granularity-1] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = 0;y < h;y++)
		{
			int sx,sy,zw,zh;

			sy = oy + ((zoomy * y + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

			for (x = 0;x < w;x++)
			{
				int c,fx,fy;

				sx = ox + ((zoomx * x + (1<<11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1<<11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ ((x<<1) < w))
					{
						/* mirror left/right */
						c += xoffset[(w-1-x+xa)&7];
						fx = 1;
					}
					else
					{
						c += xoffset[(x+xa)&7];
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += xoffset[(w-1-x+xa)&7];
					else c += xoffset[(x+xa)&7];
					fx = flipx;
				}
				if (mirrory)
				{
					if ((flipy == 0) ^ ((y<<1) >= h))
					{
						/* mirror top/bottom */
						c += yoffset[(h-1-y+ya)&7];
						fy = 1;
					}
					else
					{
						c += yoffset[(y+ya)&7];
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += yoffset[(h-1-y+ya)&7];
					else c += yoffset[(y+ya)&7];
					fy = flipy;
				}

				if (nozoom)
				{
					pdrawgfx_transtable(bitmap,cliprect,K053247_gfx,
							c,
							color,
							fx,fy,
							sx,sy,
							machine->priority_bitmap,primask,
							whichtable,machine->shadow_table);
				}
				else
				{
					pdrawgfxzoom_transtable(bitmap,cliprect,K053247_gfx,
							c,
							color,
							fx,fy,
							sx,sy,
							(zw << 16) >> 4,(zh << 16) >> 4,
							machine->priority_bitmap,primask,
							whichtable,machine->shadow_table);
				}

				if (mirrory && h == 1)  /* Simpsons shadows */
				{
					if (nozoom)
					{
						pdrawgfx_transtable(bitmap,cliprect,K053247_gfx,
								c,
								color,
								fx,!fy,
								sx,sy,
								machine->priority_bitmap,primask,
								whichtable,machine->shadow_table);
					}
					else
					{
						pdrawgfxzoom_transtable(bitmap,cliprect,K053247_gfx,
								c,
								color,
								fx,!fy,
								sx,sy,
								(zw << 16) >> 4,(zh << 16) >> 4,
								machine->priority_bitmap,primask,
								whichtable,machine->shadow_table);
					}
				}
			} // end of X loop
		} // end of Y loop

	} // end of sprite-list loop
#undef NUM_SPRITES
}



/***************************************************************************/
/*                                                                         */
/*                                 051316                                  */
/*                                                                         */
/***************************************************************************/

#define MAX_K051316 3

static const char *K051316_memory_region[MAX_K051316];
static int K051316_gfxnum[MAX_K051316];
static int K051316_wraparound[MAX_K051316];
static int K051316_offset[MAX_K051316][2];
static int K051316_bpp[MAX_K051316];
static void (*K051316_callback[MAX_K051316])(int *code,int *color,int *flags);
static UINT8 *K051316_ram[MAX_K051316];
static UINT8 K051316_ctrlram[MAX_K051316][16];
static tilemap *K051316_tilemap[MAX_K051316];

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void K051316_get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int chip)
{
	int code = K051316_ram[chip][tile_index];
	int color = K051316_ram[chip][tile_index + 0x400];
	int flags = 0;

	(*K051316_callback[chip])(&code,&color,&flags);

	SET_TILE_INFO(
			K051316_gfxnum[chip],
			code,
			color,
			flags);
}

static TILE_GET_INFO( K051316_get_tile_info0 ) { K051316_get_tile_info(machine,tileinfo,tile_index,0); }
static TILE_GET_INFO( K051316_get_tile_info1 ) { K051316_get_tile_info(machine,tileinfo,tile_index,1); }
static TILE_GET_INFO( K051316_get_tile_info2 ) { K051316_get_tile_info(machine,tileinfo,tile_index,2); }


static void K051316_vh_start(running_machine *machine,int chip, const char *gfx_memory_region,int bpp,
		int pen_is_mask,int transparent_pen,
		void (*callback)(int *code,int *color,int *flags))
{
	int gfx_index;
	UINT32 total;
	static const gfx_layout charlayout4 =
	{
		16,16,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
				8*4, 9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
				8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
		128*8
	};
	static const gfx_layout charlayout7 =
	{
		16,16,
		0,
		7,
		{ 1,2,3,4,5,6,7 },
		{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
				8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
		{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
				8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
		256*8
	};
	static const gfx_layout charlayout8 =
	{
		16,16,
		0,
		8,
		{ 0,1,2,3,4,5,6,7 },
		{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
				8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
		{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
				8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
		256*8
	};
	static const gfx_layout charlayout_tail2nos =
	{
		16,16,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ XOR(0)*4, XOR(1)*4, XOR(2)*4, XOR(3)*4, XOR(4)*4, XOR(5)*4, XOR(6)*4, XOR(7)*4,
				XOR(8)*4, XOR(9)*4, XOR(10)*4, XOR(11)*4, XOR(12)*4, XOR(13)*4, XOR(14)*4, XOR(15)*4 },
		{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
				8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
		128*8
	};
	static const tile_get_info_func get_tile_info[3] = { K051316_get_tile_info0,K051316_get_tile_info1,K051316_get_tile_info2 };

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine->gfx[gfx_index] == 0)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* decode the graphics */
	switch (bpp)
	{
	case -4:
		total = 0x400;
		bpp = 4;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout_tail2nos, 4);
		break;

	case 4:
		total = memory_region_length(machine, gfx_memory_region) / 128;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout4, 4);
		break;

	case 7:
		total = memory_region_length(machine, gfx_memory_region) / 256;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout7, 7);
		break;

	case 8:
		total = memory_region_length(machine, gfx_memory_region) / 256;
		decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout8, 8);
		break;

	default:
		fatalerror("Unsupported bpp");
	}

	K051316_memory_region[chip] = gfx_memory_region;
	K051316_gfxnum[chip] = gfx_index;
	K051316_bpp[chip] = bpp;
	K051316_callback[chip] = callback;

	K051316_tilemap[chip] = tilemap_create(machine, get_tile_info[chip],tilemap_scan_rows,16,16,32,32);

	K051316_ram[chip] = auto_alloc_array(machine, UINT8, 0x800);

	if (!pen_is_mask)
		tilemap_set_transparent_pen(K051316_tilemap[chip],transparent_pen);
	else
	{
		tilemap_map_pens_to_layer(K051316_tilemap[chip], 0, 0, 0, TILEMAP_PIXEL_LAYER1);
		tilemap_map_pens_to_layer(K051316_tilemap[chip], 0, transparent_pen, transparent_pen, TILEMAP_PIXEL_LAYER0);
	}

	K051316_wraparound[chip] = 0;	/* default = no wraparound */
	K051316_offset[chip][0] = K051316_offset[chip][1] = 0;

	state_save_register_item_pointer(machine, "K051316", NULL, chip, K051316_ram[chip], 0x800);
	state_save_register_item_array(machine, "K051316", NULL, chip, K051316_ctrlram[chip]);
	state_save_register_item(machine, "K051316", NULL, chip, K051316_wraparound[chip]);
	state_save_register_item(machine, "K051316", NULL, chip, K051316_offset[chip][0]);
	state_save_register_item(machine, "K051316", NULL, chip, K051316_offset[chip][1]);
}

void K051316_vh_start_0(running_machine *machine,const char *gfx_memory_region,int bpp,
		int pen_is_mask,int transparent_pen,
		void (*callback)(int *code,int *color,int *flags))
{
	K051316_vh_start(machine,0,gfx_memory_region,bpp,pen_is_mask,transparent_pen,callback);
}

void K051316_vh_start_1(running_machine *machine,const char *gfx_memory_region,int bpp,
		int pen_is_mask,int transparent_pen,
		void (*callback)(int *code,int *color,int *flags))
{
	K051316_vh_start(machine,1,gfx_memory_region,bpp,pen_is_mask,transparent_pen,callback);
}

void K051316_vh_start_2(running_machine *machine,const char *gfx_memory_region,int bpp,
		int pen_is_mask,int transparent_pen,
		void (*callback)(int *code,int *color,int *flags))
{
	K051316_vh_start(machine,2,gfx_memory_region,bpp,pen_is_mask,transparent_pen,callback);
}


static int K051316_r(int chip, int offset)
{
	return K051316_ram[chip][offset];
}

READ8_HANDLER( K051316_0_r )
{
	return K051316_r(0, offset);
}

READ8_HANDLER( K051316_1_r )
{
	return K051316_r(1, offset);
}

READ8_HANDLER( K051316_2_r )
{
	return K051316_r(2, offset);
}


static void K051316_w(int chip,int offset,int data)
{
	K051316_ram[chip][offset] = data;
	tilemap_mark_tile_dirty(K051316_tilemap[chip],offset & 0x3ff);
}

WRITE8_HANDLER( K051316_0_w )
{
	K051316_w(0,offset,data);
}

WRITE8_HANDLER( K051316_1_w )
{
	K051316_w(1,offset,data);
}

WRITE8_HANDLER( K051316_2_w )
{
	K051316_w(2,offset,data);
}


static int K051316_rom_r(running_machine *machine, int chip, int offset)
{
	if ((K051316_ctrlram[chip][0x0e] & 0x01) == 0)
	{
		int addr;

		addr = offset + (K051316_ctrlram[chip][0x0c] << 11) + (K051316_ctrlram[chip][0x0d] << 19);
		if (K051316_bpp[chip] <= 4) addr /= 2;
		addr &= memory_region_length(machine, K051316_memory_region[chip])-1;

//  popmessage("%s: offset %04x addr %04x",cpuexec_describe_context(machine),offset,addr);

		return memory_region(machine, K051316_memory_region[chip])[addr];
	}
	else
	{
//logerror("%s: read 051316 ROM offset %04x but reg 0x0c bit 0 not clear\n",cpuexec_describe_context(machine),offset);
		return 0;
	}
}

READ8_HANDLER( K051316_rom_0_r )
{
	return K051316_rom_r(space->machine,0,offset);
}

READ8_HANDLER( K051316_rom_1_r )
{
	return K051316_rom_r(space->machine,1,offset);
}

READ8_HANDLER( K051316_rom_2_r )
{
	return K051316_rom_r(space->machine,2,offset);
}



static void K051316_ctrl_w(int chip,int offset,int data)
{
	K051316_ctrlram[chip][offset] = data;
//if (offset >= 0x0c) logerror("%s: write %02x to 051316 reg %x\n",cpuexec_describe_context(machine),data,offset);
}

WRITE8_HANDLER( K051316_ctrl_0_w )
{
	K051316_ctrl_w(0,offset,data);
}

WRITE8_HANDLER( K051316_ctrl_1_w )
{
	K051316_ctrl_w(1,offset,data);
}

WRITE8_HANDLER( K051316_ctrl_2_w )
{
	K051316_ctrl_w(2,offset,data);
}

void K051316_wraparound_enable(int chip, int status)
{
	K051316_wraparound[chip] = status;
}

void K051316_set_offset(int chip, int xoffs, int yoffs)
{
	K051316_offset[chip][0] = xoffs;
	K051316_offset[chip][1] = yoffs;
}


static void K051316_zoom_draw(int chip, bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT32 priority)
{
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;

	startx = 256 * ((INT16)(256 * K051316_ctrlram[chip][0x00] + K051316_ctrlram[chip][0x01]));
	incxx  =        (INT16)(256 * K051316_ctrlram[chip][0x02] + K051316_ctrlram[chip][0x03]);
	incyx  =        (INT16)(256 * K051316_ctrlram[chip][0x04] + K051316_ctrlram[chip][0x05]);
	starty = 256 * ((INT16)(256 * K051316_ctrlram[chip][0x06] + K051316_ctrlram[chip][0x07]));
	incxy  =        (INT16)(256 * K051316_ctrlram[chip][0x08] + K051316_ctrlram[chip][0x09]);
	incyy  =        (INT16)(256 * K051316_ctrlram[chip][0x0a] + K051316_ctrlram[chip][0x0b]);

	startx -= (16 + K051316_offset[chip][1]) * incyx;
	starty -= (16 + K051316_offset[chip][1]) * incyy;

	startx -= (89 + K051316_offset[chip][0]) * incxx;
	starty -= (89 + K051316_offset[chip][0]) * incxy;

	tilemap_draw_roz(bitmap,cliprect,K051316_tilemap[chip],startx << 5,starty << 5,
			incxx << 5,incxy << 5,incyx << 5,incyy << 5,
			K051316_wraparound[chip],
			flags,priority);

#if 0
	popmessage("%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
			K051316_ctrlram[chip][0x00],
			K051316_ctrlram[chip][0x01],
			K051316_ctrlram[chip][0x02],
			K051316_ctrlram[chip][0x03],
			K051316_ctrlram[chip][0x04],
			K051316_ctrlram[chip][0x05],
			K051316_ctrlram[chip][0x06],
			K051316_ctrlram[chip][0x07],
			K051316_ctrlram[chip][0x08],
			K051316_ctrlram[chip][0x09],
			K051316_ctrlram[chip][0x0a],
			K051316_ctrlram[chip][0x0b],
			K051316_ctrlram[chip][0x0c],	/* bank for ROM testing */
			K051316_ctrlram[chip][0x0d],
			K051316_ctrlram[chip][0x0e],	/* 0 = test ROMs */
			K051316_ctrlram[chip][0x0f]);
#endif
}

void K051316_zoom_draw_0(bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT32 priority)
{
	K051316_zoom_draw(0,bitmap,cliprect,flags,priority);
}

void K051316_zoom_draw_1(bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT32 priority)
{
	K051316_zoom_draw(1,bitmap,cliprect,flags,priority);
}

void K051316_zoom_draw_2(bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT32 priority)
{
	K051316_zoom_draw(2,bitmap,cliprect,flags,priority);
}



/***************************************************************************/
/*                                                                         */
/*                                 053936                                  */
/*                                                                         */
/***************************************************************************/

#define K053936_MAX_CHIPS 2

UINT16 *K053936_0_ctrl,*K053936_0_linectrl;
UINT16 *K053936_1_ctrl,*K053936_1_linectrl;
static int K053936_offset[K053936_MAX_CHIPS][2];
static int K053936_wraparound[K053936_MAX_CHIPS];

// there is another implementation of this in  machine/konamigx.c (!)
//  why?

static void K053936_zoom_draw(int chip,UINT16 *ctrl,UINT16 *linectrl, bitmap_t *bitmap,const rectangle *cliprect,tilemap *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	if (!tmap)
		return;

	if (ctrl[0x07] & 0x0040)
	{
		UINT32 startx,starty;
		int incxx,incxy;
		rectangle my_clip;
		int y,maxy;

		// Racin' Force will get to here if glfgreat_hack is enabled, and it ends
		// up setting a maximum y value of '13', thus causing nothing to be drawn.
		// It looks like the roz output should be flipped somehow as it seems to be
		// displaying the wrong areas of the tilemap and is rendered upside down,
		// although due to the additional post-processing the voxel renderer performs
		// it's difficult to know what the output SHOULD be.  (hold W in Racin' Force
		// to see the chip output)

		if (((ctrl[0x07] & 0x0002) && ctrl[0x09]) && (glfgreat_hack))	/* wrong, but fixes glfgreat */
		{
			my_clip.min_x = ctrl[0x08] + K053936_offset[chip][0]+2;
			my_clip.max_x = ctrl[0x09] + K053936_offset[chip][0]+2 - 1;
			if (my_clip.min_x < cliprect->min_x)
				my_clip.min_x = cliprect->min_x;
			if (my_clip.max_x > cliprect->max_x)
				my_clip.max_x = cliprect->max_x;

			y = ctrl[0x0a] + K053936_offset[chip][1]-2;
			if (y < cliprect->min_y)
				y = cliprect->min_y;
			maxy = ctrl[0x0b] + K053936_offset[chip][1]-2 - 1;
			if (maxy > cliprect->max_y)
				maxy = cliprect->max_y;
		}
		else
		{
			my_clip.min_x = cliprect->min_x;
			my_clip.max_x = cliprect->max_x;

			y = cliprect->min_y;
			maxy = cliprect->max_y;
		}

		while (y <= maxy)
		{
			UINT16 *lineaddr = linectrl + 4*((y - K053936_offset[chip][1]) & 0x1ff);
			my_clip.min_y = my_clip.max_y = y;



			startx = 256 * (INT16)(lineaddr[0] + ctrl[0x00]);
			starty = 256 * (INT16)(lineaddr[1] + ctrl[0x01]);
			incxx  =       (INT16)(lineaddr[2]);
			incxy  =       (INT16)(lineaddr[3]);

			if (ctrl[0x06] & 0x8000) incxx *= 256;
			if (ctrl[0x06] & 0x0080) incxy *= 256;

			startx -= K053936_offset[chip][0] * incxx;
			starty -= K053936_offset[chip][0] * incxy;

			tilemap_draw_roz(bitmap,&my_clip,tmap,startx << 5,starty << 5,
					incxx << 5,incxy << 5,0,0,
					K053936_wraparound[chip],
					flags,priority);

			y++;
		}
	}
	else	/* "simple" mode */
	{
		UINT32 startx,starty;
		int incxx,incxy,incyx,incyy;

		startx = 256 * (INT16)(ctrl[0x00]);
		starty = 256 * (INT16)(ctrl[0x01]);
		incyx  =       (INT16)(ctrl[0x02]);
		incyy  =       (INT16)(ctrl[0x03]);
		incxx  =       (INT16)(ctrl[0x04]);
		incxy  =       (INT16)(ctrl[0x05]);

		if (ctrl[0x06] & 0x4000) { incyx *= 256; incyy *= 256; }
		if (ctrl[0x06] & 0x0040) { incxx *= 256; incxy *= 256; }

		startx -= K053936_offset[chip][1] * incyx;
		starty -= K053936_offset[chip][1] * incyy;

		startx -= K053936_offset[chip][0] * incxx;
		starty -= K053936_offset[chip][0] * incxy;

		tilemap_draw_roz(bitmap,cliprect,tmap,startx << 5,starty << 5,
				incxx << 5,incxy << 5,incyx << 5,incyy << 5,
				K053936_wraparound[chip],
				flags,priority);
	}

#if 0
if (input_code_pressed(machine, KEYCODE_D))
	popmessage("%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x",
			ctrl[0x00],
			ctrl[0x01],
			ctrl[0x02],
			ctrl[0x03],
			ctrl[0x04],
			ctrl[0x05],
			ctrl[0x06],
			ctrl[0x07],
			ctrl[0x08],
			ctrl[0x09],
			ctrl[0x0a],
			ctrl[0x0b],
			ctrl[0x0c],
			ctrl[0x0d],
			ctrl[0x0e],
			ctrl[0x0f]);
#endif
}


void K053936_0_zoom_draw(bitmap_t *bitmap,const rectangle *cliprect,tilemap *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	K053936_zoom_draw(0,K053936_0_ctrl,K053936_0_linectrl,bitmap,cliprect,tmap,flags,priority, glfgreat_hack);
}

void K053936_1_zoom_draw(bitmap_t *bitmap,const rectangle *cliprect,tilemap *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	K053936_zoom_draw(1,K053936_1_ctrl,K053936_1_linectrl,bitmap,cliprect,tmap,flags,priority, glfgreat_hack);
}


void K053936_wraparound_enable(int chip, int status)
{
	K053936_wraparound[chip] = status;
}


void K053936_set_offset(int chip, int xoffs, int yoffs)
{
	K053936_offset[chip][0] = xoffs;
	K053936_offset[chip][1] = yoffs;
}



/***************************************************************************/
/*                                                                         */
/*                                 053251                                  */
/*                                                                         */
/***************************************************************************/

static UINT8 K053251_ram[16];
static int K053251_palette_index[5];
static tilemap *K053251_tilemaps[5];
static int K053251_tilemaps_set;

static STATE_POSTLOAD( K053251_reset_indexes )
{
	K053251_palette_index[0] = 32 * ((K053251_ram[9] >> 0) & 0x03);
	K053251_palette_index[1] = 32 * ((K053251_ram[9] >> 2) & 0x03);
	K053251_palette_index[2] = 32 * ((K053251_ram[9] >> 4) & 0x03);
	K053251_palette_index[3] = 16 * ((K053251_ram[10] >> 0) & 0x07);
	K053251_palette_index[4] = 16 * ((K053251_ram[10] >> 3) & 0x07);
}

void K053251_vh_start(running_machine *machine)
{
	K053251_set_tilemaps(NULL,NULL,NULL,NULL,NULL);

	state_save_register_global_array(machine, K053251_ram);
	state_save_register_postload(machine, K053251_reset_indexes, NULL);
}

void K053251_set_tilemaps(tilemap *ci0,tilemap *ci1,tilemap *ci2,tilemap *ci3,tilemap *ci4)
{
	K053251_tilemaps[0] = ci0;
	K053251_tilemaps[1] = ci1;
	K053251_tilemaps[2] = ci2;
	K053251_tilemaps[3] = ci3;
	K053251_tilemaps[4] = ci4;

	K053251_tilemaps_set = (ci0 || ci1 || ci2 || ci3 || ci4) ? 1 : 0;
}

WRITE8_HANDLER( K053251_w )
{
	int i,newind;

	data &= 0x3f;

	if (K053251_ram[offset] != data)
	{
		K053251_ram[offset] = data;
		if (offset == 9)
		{
			/* palette base index */
			for (i = 0;i < 3;i++)
			{
				newind = 32 * ((data >> 2*i) & 0x03);
				if (K053251_palette_index[i] != newind)
				{
					K053251_palette_index[i] = newind;
					if (K053251_tilemaps[i])
						tilemap_mark_all_tiles_dirty(K053251_tilemaps[i]);

				}
			}

			if (!K053251_tilemaps_set)
				tilemap_mark_all_tiles_dirty_all(space->machine);
		}
		else if (offset == 10)
		{
			/* palette base index */
			for (i = 0;i < 2;i++)
			{
				newind = 16 * ((data >> 3*i) & 0x07);
				if (K053251_palette_index[3+i] != newind)
				{
					K053251_palette_index[3+i] = newind;
					if (K053251_tilemaps[3+i])
						tilemap_mark_all_tiles_dirty(K053251_tilemaps[3+i]);

				}
			}

			if (!K053251_tilemaps_set)
				tilemap_mark_all_tiles_dirty_all(space->machine);
		}
	}
}

WRITE16_HANDLER( K053251_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		K053251_w(space, offset, data & 0xff);
}

WRITE16_HANDLER( K053251_msb_w )
{
	if (ACCESSING_BITS_8_15)
		K053251_w(space, offset, (data >> 8) & 0xff);
}

int K053251_get_priority(int ci)
{
	return K053251_ram[ci];
}

int K053251_get_palette_index(int ci)
{
	return K053251_palette_index[ci];
}



/***************************************************************************/
/*                                                                         */
/*                                 054000                                  */
/*                                                                         */
/***************************************************************************/

static UINT8 K054000_ram[0x20];

WRITE8_HANDLER( K054000_w )
{
//logerror("%04x: write %02x to 054000 address %02x\n",cpu_get_pc(space->cpu),data,offset);

	K054000_ram[offset] = data;
}

READ8_HANDLER( K054000_r )
{
	int Acx,Acy,Aax,Aay;
	int Bcx,Bcy,Bax,Bay;

//logerror("%04x: read 054000 address %02x\n",cpu_get_pc(space->cpu),offset);

	if (offset != 0x18) return 0;

	Acx = (K054000_ram[0x01] << 16) | (K054000_ram[0x02] << 8) | K054000_ram[0x03];
	Acy = (K054000_ram[0x09] << 16) | (K054000_ram[0x0a] << 8) | K054000_ram[0x0b];
/* TODO: this is a hack to make thndrx2 pass the startup check. It is certainly wrong. */
if (K054000_ram[0x04] == 0xff) Acx+=3;
if (K054000_ram[0x0c] == 0xff) Acy+=3;
	Aax = K054000_ram[0x06] + 1;
	Aay = K054000_ram[0x07] + 1;

	Bcx = (K054000_ram[0x15] << 16) | (K054000_ram[0x16] << 8) | K054000_ram[0x17];
	Bcy = (K054000_ram[0x11] << 16) | (K054000_ram[0x12] << 8) | K054000_ram[0x13];
	Bax = K054000_ram[0x0e] + 1;
	Bay = K054000_ram[0x0f] + 1;

	if (Acx + Aax < Bcx - Bax)
		return 1;

	if (Bcx + Bax < Acx - Aax)
		return 1;

	if (Acy + Aay < Bcy - Bay)
		return 1;

	if (Bcy + Bay < Acy - Aay)
		return 1;

	return 0;
}

READ16_HANDLER( K054000_lsb_r )
{
	return K054000_r(space, offset);
}

WRITE16_HANDLER( K054000_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		K054000_w(space, offset, data & 0xff);
}



/***************************************************************************/
/*                                                                         */
/*                                 051733                                  */
/*                                                                         */
/***************************************************************************/

static UINT8 K051733_ram[0x20];

WRITE8_HANDLER( K051733_w )
{
//logerror("%04x: write %02x to 051733 address %02x\n",cpu_get_pc(space->cpu),data,offset);

	K051733_ram[offset] = data;
}


static int int_sqrt(UINT32 op)
{
	UINT32 i,step;

	i = 0x8000;
	step = 0x4000;
	while (step)
	{
		if (i*i == op) return i;
		else if (i*i > op) i -= step;
		else i += step;
		step >>= 1;
	}
	return i;
}

READ8_HANDLER( K051733_r )
{
	int op1 = (K051733_ram[0x00] << 8) | K051733_ram[0x01];
	int op2 = (K051733_ram[0x02] << 8) | K051733_ram[0x03];
	int op3 = (K051733_ram[0x04] << 8) | K051733_ram[0x05];

	int rad = (K051733_ram[0x06] << 8) | K051733_ram[0x07];
	int yobj1c = (K051733_ram[0x08] << 8) | K051733_ram[0x09];
	int xobj1c = (K051733_ram[0x0a] << 8) | K051733_ram[0x0b];
	int yobj2c = (K051733_ram[0x0c] << 8) | K051733_ram[0x0d];
	int xobj2c = (K051733_ram[0x0e] << 8) | K051733_ram[0x0f];

//logerror("%04x: read 051733 address %02x\n",cpu_get_pc(space->cpu),offset);

	switch(offset){
		case 0x00:
			if (op2) return	(op1 / op2) >> 8;
			else return 0xff;
		case 0x01:
			if (op2) return	(op1 / op2) & 0xff;
			else return 0xff;

		/* this is completely unverified */
		case 0x02:
			if (op2) return	(op1 % op2) >> 8;
			else return 0xff;
		case 0x03:
			if (op2) return	(op1 % op2) & 0xff;
			else return 0xff;

		case 0x04:
			return int_sqrt(op3<<16) >> 8;

		case 0x05:
			return int_sqrt(op3<<16) & 0xff;

		case 0x06:
			return K051733_ram[0x13];

		case 0x07:{
			if (xobj1c + rad < xobj2c)
				return 0x80;

			if (xobj2c + rad < xobj1c)
				return 0x80;

			if (yobj1c + rad < yobj2c)
				return 0x80;

			if (yobj2c + rad < yobj1c)
				return 0x80;

			return 0;
		}
		case 0x0e:
			return ~K051733_ram[offset];
		case 0x0f:
			return ~K051733_ram[offset];
		default:
			return K051733_ram[offset];
	}
}

/***************************************************************************/
/*                                                                         */
/*                                 054157 / 056832                         */
/*                                                                         */
/***************************************************************************/

#define K056832_PAGE_COLS 64
#define K056832_PAGE_ROWS 32
#define K056832_PAGE_HEIGHT (K056832_PAGE_ROWS*8)
#define K056832_PAGE_WIDTH  (K056832_PAGE_COLS*8)
#define K056832_PAGE_COUNT 16

static tilemap *K056832_tilemap[K056832_PAGE_COUNT];
static bitmap_t *K056832_pixmap[K056832_PAGE_COUNT];

static UINT16 K056832_regs[0x20];	// 157/832 regs group 1
static UINT16 K056832_regsb[4];	// 157/832 regs group 2, board dependent

static UINT8 *K056832_rombase;	// pointer to tile gfx data
static UINT16 *K056832_videoram;
static int K056832_NumGfxBanks;		// depends on size of graphics ROMs
static int K056832_CurGfxBank;		// cached info for K056832_regs[0x1a]
static int K056832_gfxnum;			// graphics element index for unpacked tiles
static const char *K056832_memory_region;	// memory region for tile gfx data
static int K056832_bpp;

// ROM readback involves reading 2 halves of a word
// from the same location in a row.  Reading the
// RAM window resets this state so you get the first half.
static int K056832_rom_half;

// locally cached values
static int K056832_LayerAssociatedWithPage[K056832_PAGE_COUNT];
static int K056832_LayerOffset[8][2];
static int K056832_LSRAMPage[8][2];
static INT32 K056832_X[8];	// 0..3 left
static INT32 K056832_Y[8];	// 0..3 top
static INT32 K056832_W[8];	// 0..3 width  -> 1..4 pages
static INT32 K056832_H[8];	// 0..3 height -> 1..4 pages
static INT32 K056832_dx[8];	// scroll
static INT32 K056832_dy[8];	// scroll
static UINT32 K056832_LineDirty[K056832_PAGE_COUNT][8];
static UINT8 K056832_AllLinesDirty[K056832_PAGE_COUNT];
static UINT8 K056832_PageTileMode[K056832_PAGE_COUNT];
static UINT8 K056832_LayerTileMode[8];
static int K056832_DefaultLayerAssociation;
static int K056832_LayerAssociation;
static int K056832_ActiveLayer;
static int K056832_SelectedPage;
static int K056832_SelectedPagex4096;
static int K056832_UpdateMode;
static int K056832_linemap_enabled;
static int K056832_use_ext_linescroll;
static int K056832_uses_tile_banks, K056832_cur_tile_bank;

static int K056832_djmain_hack;

#define K056832_mark_line_dirty(P,L) if (L<0x100) K056832_LineDirty[P][L>>5] |= 1<<(L&0x1f)
#define K056832_mark_all_lines_dirty(P) K056832_AllLinesDirty[P] = 1

static void K056832_mark_page_dirty(int page)
{
	if (K056832_PageTileMode[page])
		tilemap_mark_all_tiles_dirty(K056832_tilemap[page]);
	else
		K056832_mark_all_lines_dirty(page);
}

void K056832_mark_plane_dirty(int layer)
{
	int tilemode, i;

	tilemode = K056832_LayerTileMode[layer];

	for (i=0; i<K056832_PAGE_COUNT; i++)
	{
		if (K056832_LayerAssociatedWithPage[i] == layer)
		{
			K056832_PageTileMode[i] = tilemode;
			K056832_mark_page_dirty(i);
		}
	}
}

void K056832_MarkAllTilemapsDirty(void)
{
	int i;

	for (i=0; i<K056832_PAGE_COUNT; i++)
	{
		if (K056832_LayerAssociatedWithPage[i] != -1)
		{
			K056832_PageTileMode[i] = K056832_LayerTileMode[K056832_LayerAssociatedWithPage[i]];
			K056832_mark_page_dirty(i);
		}
	}
}

static void K056832_UpdatePageLayout(void)
{
	int layer, rowstart, rowspan, colstart, colspan, r, c, pageIndex, setlayer;

	// enable layer association by default
	K056832_LayerAssociation = K056832_DefaultLayerAssociation;

	// disable association if a layer grabs the entire 4x4 map (happens in Twinbee and Dadandarn)
	for (layer=0; layer<4; layer++)
	{
		if (!K056832_Y[layer] && !K056832_X[layer] && K056832_H[layer]==3 && K056832_W[layer]==3)
		{
			K056832_LayerAssociation = 0;
			break;
		}
	}

	// winning spike doesn't like layer association..
	if (K056832_djmain_hack==2)
		K056832_LayerAssociation = 0;


	// disable all tilemaps
	for (pageIndex=0; pageIndex<K056832_PAGE_COUNT; pageIndex++)
	{
		K056832_LayerAssociatedWithPage[pageIndex] = -1;
	}


	// enable associated tilemaps
	for (layer=0; layer<4; layer++)
	{
		rowstart = K056832_Y[layer];
		colstart = K056832_X[layer];
		rowspan  = K056832_H[layer]+1;
		colspan  = K056832_W[layer]+1;

		setlayer = (K056832_LayerAssociation) ? layer : K056832_ActiveLayer;

		for (r=0; r<rowspan; r++)
		{
			for (c=0; c<colspan; c++)
			{
				pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);
if (!(K056832_djmain_hack==1) || K056832_LayerAssociatedWithPage[pageIndex] == -1) //*
					K056832_LayerAssociatedWithPage[pageIndex] = setlayer;
			}
		}
	}

	// refresh associated tilemaps
	K056832_MarkAllTilemapsDirty();
}

int K056832_get_lookup(int bits)
{
	int res;

	res = (K056832_regs[0x1c] >> (bits << 2)) & 0x0f;

	if (K056832_uses_tile_banks)	/* Asterix */
		res |= K056832_cur_tile_bank << 4;

	return res;
}

static void (*K056832_callback)(int layer, int *code, int *color, int *flags);

INLINE void K056832_get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int pageIndex )
{
	static const struct K056832_SHIFTMASKS
	{
		int flips, palm1, pals2, palm2;
	}
	K056832_shiftmasks[4] = {{6,0x3f,0,0x00},{4,0x0f,2,0x30},{2,0x03,2,0x3c},{0,0x00,2,0x3f}};

	const struct K056832_SHIFTMASKS *smptr;
	int layer, flip, fbits, attr, code, color, flags;
	UINT16 *pMem;

	pMem  = &K056832_videoram[(pageIndex<<12)+(tile_index<<1)];

	if (K056832_LayerAssociation)
	{
		layer = K056832_LayerAssociatedWithPage[pageIndex];
		if (layer == -1) layer = 0;	// use layer 0's palette info for unmapped pages
	}
	else
		layer = K056832_ActiveLayer;

	fbits = K056832_regs[3]>>6 & 3;
	flip  = K056832_regs[1]>>(layer<<1) & 0x3; // tile-flip override (see p.20 3.2.2 "REG2")
	smptr = &K056832_shiftmasks[fbits];
	attr  = pMem[0];
	code  = pMem[1];

	// normalize the flip/palette flags
	// see the tables on pages 4 and 10 of the Pt. 2-3 "VRAM" manual
	// for a description of these bits "FBIT0" and "FBIT1"
	flip &= attr>>smptr->flips & 3;
	color = (attr & smptr->palm1) | (attr>>smptr->pals2 & smptr->palm2);
	flags = TILE_FLIPYX(flip);

	(*K056832_callback)(layer, &code, &color, &flags);

	SET_TILE_INFO(K056832_gfxnum,
			code,
			color,
			flags);
}

static TILE_GET_INFO( K056832_get_tile_info0 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x0); }
static TILE_GET_INFO( K056832_get_tile_info1 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x1); }
static TILE_GET_INFO( K056832_get_tile_info2 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x2); }
static TILE_GET_INFO( K056832_get_tile_info3 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x3); }
static TILE_GET_INFO( K056832_get_tile_info4 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x4); }
static TILE_GET_INFO( K056832_get_tile_info5 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x5); }
static TILE_GET_INFO( K056832_get_tile_info6 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x6); }
static TILE_GET_INFO( K056832_get_tile_info7 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x7); }
static TILE_GET_INFO( K056832_get_tile_info8 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x8); }
static TILE_GET_INFO( K056832_get_tile_info9 ) { K056832_get_tile_info(machine,tileinfo,tile_index,0x9); }
static TILE_GET_INFO( K056832_get_tile_infoa ) { K056832_get_tile_info(machine,tileinfo,tile_index,0xa); }
static TILE_GET_INFO( K056832_get_tile_infob ) { K056832_get_tile_info(machine,tileinfo,tile_index,0xb); }
static TILE_GET_INFO( K056832_get_tile_infoc ) { K056832_get_tile_info(machine,tileinfo,tile_index,0xc); }
static TILE_GET_INFO( K056832_get_tile_infod ) { K056832_get_tile_info(machine,tileinfo,tile_index,0xd); }
static TILE_GET_INFO( K056832_get_tile_infoe ) { K056832_get_tile_info(machine,tileinfo,tile_index,0xe); }
static TILE_GET_INFO( K056832_get_tile_infof ) { K056832_get_tile_info(machine,tileinfo,tile_index,0xf); }

static void K056832_change_rambank(void)
{
	/* ------xx page col
     * ---xx--- page row
     */
	int bank = K056832_regs[0x19];

	if (K056832_regs[0] & 0x02)	// external linescroll enable
	{
		K056832_SelectedPage = K056832_PAGE_COUNT;
	}
	else
	{
		K056832_SelectedPage = ((bank>>1)&0xc)|(bank&3);
	}
	K056832_SelectedPagex4096 = K056832_SelectedPage << 12;

	// refresh associated tilemaps
	K056832_MarkAllTilemapsDirty();
}

int K056832_get_current_rambank(void)
{
	int bank = K056832_regs[0x19];

	return ((bank>>1)&0xc)|(bank&3);
}

static void K056832_change_rombank(void)
{
	int bank;

	if (K056832_uses_tile_banks)	/* Asterix */
	{
		bank = (K056832_regs[0x1a] >> 8) | (K056832_regs[0x1b] << 4) | (K056832_cur_tile_bank << 6);
	}
	else
	{
		bank = K056832_regs[0x1a] | (K056832_regs[0x1b] << 16);
	}

	K056832_CurGfxBank = bank % K056832_NumGfxBanks;
}

void K056832_set_tile_bank(int bank)
{
	K056832_uses_tile_banks = 1;

	if (K056832_cur_tile_bank != bank)
	{
		K056832_cur_tile_bank = bank;

		K056832_mark_plane_dirty(0);
		K056832_mark_plane_dirty(1);
		K056832_mark_plane_dirty(2);
		K056832_mark_plane_dirty(3);
	}

	K056832_change_rombank();
}

static STATE_POSTLOAD( K056832_postload )
{
	K056832_UpdatePageLayout();
	K056832_change_rambank();
	K056832_change_rombank();
}

void K056832_vh_start(running_machine *machine, const char *gfx_memory_region, int bpp, int big,
	int (*scrolld)[4][2],
	void (*callback)(int layer, int *code, int *color, int *flags),
	int djmain_hack)
{
	tilemap *tmap;
	int gfx_index;
	int i;
	UINT32 total;
	static const gfx_layout charlayout8 =
	{
		8, 8,
		0,
		8,
		{ 8*7,8*3,8*5,8*1,8*6,8*2,8*4,8*0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 8*8, 8*8*2, 8*8*3, 8*8*4, 8*8*5, 8*8*6, 8*8*7 },
		8*8*8
	};
	static const gfx_layout charlayout8le =
	{
		8, 8,
		0,
		8,
//      { 0, 1, 2, 3, 0+(0x200000*8), 1+(0x200000*8), 2+(0x200000*8), 3+(0x200000*8) },
		{ 0+(0x200000*8), 1+(0x200000*8), 2+(0x200000*8), 3+(0x200000*8), 0, 1, 2, 3 },
		{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
		8*8*4
	};
	static const gfx_layout charlayout6 =
	{
		8, 8,
		0,
		6,
		{ 40, 32, 24, 8, 16, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 6*8, 6*8*2, 6*8*3, 6*8*4, 6*8*5, 6*8*6, 6*8*7 },
		8*8*6
	};
	static const gfx_layout charlayout5 =
	{
		8, 8,
		0,
		5,
		{ 32, 24, 8, 16, 0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 5*8, 5*8*2, 5*8*3, 5*8*4, 5*8*5, 5*8*6, 5*8*7 },
		8*8*5
	};
	static const gfx_layout charlayout4 =
	{
		8, 8,
		0,
		4,
		{ 0, 1, 2, 3 },
		{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
		8*8*4
	};
	static const gfx_layout charlayout4dj =
	{
		8, 8,
		0,
		4,
		{ 8*3,8*1,8*2,8*0 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 8*4, 8*4*2, 8*4*3, 8*4*4, 8*4*5, 8*4*6, 8*4*7 },
		8*8*4
	};

	K056832_bpp = bpp;

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
	{
		if (machine->gfx[gfx_index] == 0) break;
	}
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* handle the various graphics formats */
	i = (big) ? 8 : 16;

	/* decode the graphics */
	switch (bpp)
	{
		case K056832_BPP_4:
			total = memory_region_length(machine, gfx_memory_region) / (i*4);
			decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout4, 4);
			break;

		case K056832_BPP_5:
			total = memory_region_length(machine, gfx_memory_region) / (i*5);
			decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout5, 4);
			break;

		case K056832_BPP_6:
			total = memory_region_length(machine, gfx_memory_region) / (i*6);
			decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout6, 4);
			break;

		case K056832_BPP_8:
			total = memory_region_length(machine, gfx_memory_region) / (i*8);
			decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout8, 4);
			break;

		case K056832_BPP_8LE:
			total = memory_region_length(machine, gfx_memory_region) / (i*8);
			decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout8le, 4);
			break;

		case K056832_BPP_4dj:
			total = memory_region_length(machine, gfx_memory_region) / (i*4);
			decode_gfx(machine, gfx_index, memory_region(machine, gfx_memory_region), total, &charlayout4dj, 4);
			break;

		default:
			fatalerror("Unsupported bpp");
	}

	machine->gfx[gfx_index]->color_granularity = 16; /* override */

	K056832_memory_region = gfx_memory_region;
	K056832_gfxnum = gfx_index;
	K056832_callback = callback;

	K056832_rombase = memory_region(machine, gfx_memory_region);
	K056832_NumGfxBanks = memory_region_length(machine, gfx_memory_region) / 0x2000;
	K056832_CurGfxBank = 0;
	K056832_use_ext_linescroll = 0;
	K056832_uses_tile_banks = 0;

	K056832_djmain_hack = djmain_hack;

	for (i=0; i<4; i++)
	{
		K056832_LayerOffset[i][0] = 0;
		K056832_LayerOffset[i][1] = 0;
		K056832_LSRAMPage[i][0] = i;
		K056832_LSRAMPage[i][1] = i << 11;
		K056832_X[i] = 0;
		K056832_Y[i] = 0;
		K056832_W[i] = 0;
		K056832_H[i] = 0;
		K056832_dx[i] = 0;
		K056832_dy[i] = 0;
		K056832_LayerTileMode[i] = 1;
	}

	K056832_DefaultLayerAssociation = 1;
	K056832_ActiveLayer = 0;
	K056832_UpdateMode = 0;
	K056832_linemap_enabled = 0;

	memset(K056832_LineDirty, 0, sizeof(UINT32) * K056832_PAGE_COUNT * 8);

	for (i=0; i<K056832_PAGE_COUNT; i++)
	{
		K056832_AllLinesDirty[i] = 0;
		K056832_PageTileMode[i] = 1;
	}

	K056832_videoram = auto_alloc_array(machine, UINT16, 0x2000 * (K056832_PAGE_COUNT+1) / 2);

	K056832_tilemap[0x0] = tilemap_create(machine, K056832_get_tile_info0, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x1] = tilemap_create(machine, K056832_get_tile_info1, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x2] = tilemap_create(machine, K056832_get_tile_info2, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x3] = tilemap_create(machine, K056832_get_tile_info3, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x4] = tilemap_create(machine, K056832_get_tile_info4, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x5] = tilemap_create(machine, K056832_get_tile_info5, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x6] = tilemap_create(machine, K056832_get_tile_info6, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x7] = tilemap_create(machine, K056832_get_tile_info7, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x8] = tilemap_create(machine, K056832_get_tile_info8, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0x9] = tilemap_create(machine, K056832_get_tile_info9, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0xa] = tilemap_create(machine, K056832_get_tile_infoa, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0xb] = tilemap_create(machine, K056832_get_tile_infob, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0xc] = tilemap_create(machine, K056832_get_tile_infoc, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0xd] = tilemap_create(machine, K056832_get_tile_infod, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0xe] = tilemap_create(machine, K056832_get_tile_infoe, tilemap_scan_rows,  8, 8, 64, 32);
	K056832_tilemap[0xf] = tilemap_create(machine, K056832_get_tile_infof, tilemap_scan_rows,  8, 8, 64, 32);

	for (i=0; i<K056832_PAGE_COUNT; i++)
	{
		tmap = K056832_tilemap[i];

		K056832_pixmap[i] = tilemap_get_pixmap(tmap);

		tilemap_set_transparent_pen(tmap, 0);
	}

	memset(K056832_videoram, 0x00, 0x20000);
	memset(K056832_regs,     0x00, sizeof(K056832_regs) );
	memset(K056832_regsb,    0x00, sizeof(K056832_regsb) );

	K056832_UpdatePageLayout();

	K056832_change_rambank();
	K056832_change_rombank();

	state_save_register_global_pointer(machine, K056832_videoram, 0x10000);
	state_save_register_global_array(machine, K056832_regs);
	state_save_register_global_array(machine, K056832_regsb);
	state_save_register_global_array(machine, K056832_X);
	state_save_register_global_array(machine, K056832_Y);
	state_save_register_global_array(machine, K056832_W);
	state_save_register_global_array(machine, K056832_H);
	state_save_register_global_array(machine, K056832_dx);
	state_save_register_global_array(machine, K056832_dy);
	state_save_register_global_array(machine, K056832_LayerTileMode);

	state_save_register_postload(machine, K056832_postload, NULL);
}

/* call if a game uses external linescroll */
void K056832_SetExtLinescroll(void)
{
	K056832_use_ext_linescroll = 1;
}

/* generic helper routine for ROM checksumming */
static int K056832_rom_read_b(running_machine *machine, int offset, int blksize, int blksize2, int zerosec)
{
	UINT8 *rombase;
	int base, ret;

	rombase = (UINT8 *)memory_region(machine, K056832_memory_region);

	if ((K056832_rom_half) && (zerosec))
	{
		return 0;
	}

	// add in the bank offset
	offset += (K056832_CurGfxBank * 0x2000);

	// figure out the base of the ROM block
	base = (offset / blksize) * blksize2;

	// get the starting offset of the proper word inside the block
	base += (offset % blksize) * 2;

	if (K056832_rom_half)
	{
		ret = rombase[base+1];
	}
	else
	{
		ret = rombase[base];
		K056832_rom_half = 1;
	}

	return ret;
}

READ16_HANDLER( K056832_5bpp_rom_word_r )
{
	if (mem_mask == 0xff00)
	{
		return K056832_rom_read_b(space->machine, offset*2, 4, 5, 0)<<8;
	}
	else if (mem_mask == 0x00ff)
	{
		return K056832_rom_read_b(space->machine, offset*2+1, 4, 5, 0)<<16;
	}
	else
	{
		LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(space->cpu), mem_mask));
	}
	return 0;
}

READ32_HANDLER( K056832_5bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
	{
		return K056832_rom_read_b(space->machine, offset*4, 4, 5, 0)<<24;
	}
	else if (mem_mask == 0x00ff0000)
	{
		return K056832_rom_read_b(space->machine, offset*4+1, 4, 5, 0)<<16;
	}
	else if (mem_mask == 0x0000ff00)
	{
		return K056832_rom_read_b(space->machine, offset*4+2, 4, 5, 0)<<8;
	}
	else if (mem_mask == 0x000000ff)
	{
		return K056832_rom_read_b(space->machine, offset*4+3, 4, 5, 1);
	}
	else
	{
		LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(space->cpu), mem_mask));
	}
	return 0;
}

READ32_HANDLER( K056832_6bpp_rom_long_r )
{
	if (mem_mask == 0xff000000)
	{
		return K056832_rom_read_b(space->machine, offset*4, 4, 6, 0)<<24;
	}
	else if (mem_mask == 0x00ff0000)
	{
		return K056832_rom_read_b(space->machine, offset*4+1, 4, 6, 0)<<16;
	}
	else if (mem_mask == 0x0000ff00)
	{
		return K056832_rom_read_b(space->machine, offset*4+2, 4, 6, 0)<<8;
	}
	else if (mem_mask == 0x000000ff)
	{
		return K056832_rom_read_b(space->machine, offset*4+3, 4, 6, 0);
	}
	else
	{
		LOG(("Non-byte read of tilemap ROM, PC=%x (mask=%x)\n", cpu_get_pc(space->cpu), mem_mask));
	}
	return 0;
}

READ16_HANDLER( K056832_rom_word_r )
{
	int ofs16, ofs8;
	UINT8 *rombase;
	int ret;

	ofs16 = (offset / 8)*5;
	ofs8 = (offset / 4)*5;

	ofs16 += (K056832_CurGfxBank*5*1024);
	ofs8 += (K056832_CurGfxBank*10*1024);

	if (!K056832_rombase)
	{
		K056832_rombase = memory_region(space->machine, K056832_memory_region);
	}
	rombase = (UINT8 *)K056832_rombase;

	ret = (rombase[ofs8+4]<<8);
	if ((offset % 8) >= 4)
	{
		ret |= (rombase[ofs16+1]<<24) | (rombase[ofs16+3]<<16);
	}
	else
	{
		ret |= (rombase[ofs16]<<24) | (rombase[ofs16+2]<<16);
	}

	return ret;
}

// data is arranged like this:
// 0000 1111 22 0000 1111 22
READ16_HANDLER( K056832_mw_rom_word_r )
{
	int bank = 10240*K056832_CurGfxBank;
	int addr;

	if (!K056832_rombase)
	{
		K056832_rombase = memory_region(space->machine, K056832_memory_region);
	}

	if (K056832_regsb[2] & 0x8)
	{
		// we want only the 2s
		int bit;
		int res, temp;

		bit = offset % 4;
		addr = (offset / 4) * 5;

		temp = K056832_rombase[addr+4+bank];

		switch (bit)
		{
			default:
			case 0:
				res = (temp & 0x80) << 5;
				res |= ((temp & 0x40) >> 2);
				break;

			case 1:
				res = (temp & 0x20) << 7;
				res |= (temp & 0x10);
				break;

			case 2:
				res = (temp & 0x08) << 9;
				res |= ((temp & 0x04) << 2);
				break;

			case 3:
				res = (temp & 0x02) << 11;
				res |= ((temp & 0x01) << 4);
				break;
		}

		return res;
	}
	else
	{
		// we want only the 0s and 1s.

		addr = (offset>>1) * 5;

		if (offset & 1)
		{
			addr += 2;
		}

		addr += bank;

		return K056832_rombase[addr+1] | (K056832_rombase[addr] << 8);
	}

}

READ16_HANDLER( K056832_bishi_rom_word_r )
{
	int addr = 0x4000*K056832_CurGfxBank+offset;

	if (!K056832_rombase)
	{
		K056832_rombase = memory_region(space->machine, K056832_memory_region);
	}

	return K056832_rombase[addr+2] | (K056832_rombase[addr] << 8);
}

READ16_HANDLER( K056832_rom_word_8000_r )
{
	int addr = 0x8000*K056832_CurGfxBank + 2*offset;

	if (!K056832_rombase)
	{
		K056832_rombase = memory_region(space->machine, K056832_memory_region);
	}

	return K056832_rombase[addr+2] | (K056832_rombase[addr] << 8);
}

READ16_HANDLER( K056832_old_rom_word_r )
{
	int addr = 0x2000*K056832_CurGfxBank + (2*offset);

	if (!K056832_rombase)
	{
		K056832_rombase = memory_region(space->machine, K056832_memory_region);
	}

	return K056832_rombase[addr+1] | (K056832_rombase[addr] << 8);
}

READ32_HANDLER( K056832_rom_long_r )
{
	offset <<= 1;
	return (K056832_rom_word_r(space, offset+1, 0xffff) | (K056832_rom_word_r(space, offset, 0xffff)<<16));
}

/* only one page is mapped to videoram at a time through a window */
READ16_HANDLER( K056832_ram_word_r )
{
	// reading from tile RAM resets the ROM readback "half" offset
	K056832_rom_half = 0;

	return K056832_videoram[K056832_SelectedPagex4096+offset];
}

READ16_HANDLER( K056832_ram_half_word_r )
{
	return K056832_videoram[K056832_SelectedPagex4096+(((offset << 1) & 0xffe) | ((offset >> 11) ^ 1))];
}

READ32_HANDLER( K056832_ram_long_r )
{
	UINT16 *pMem = &K056832_videoram[K056832_SelectedPagex4096+offset*2];

	// reading from tile RAM resets the ROM readback "half" offset
	K056832_rom_half = 0;

	return(pMem[0]<<16 | pMem[1]);
}

/* special 8-bit handlers for Lethal Enforcers */
READ8_HANDLER( K056832_ram_code_lo_r )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)+1];

	return *adr & 0xff;
}

READ8_HANDLER( K056832_ram_code_hi_r )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)+1];

	return *adr>>8;
}

READ8_HANDLER( K056832_ram_attr_lo_r )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)];

	return *adr & 0xff;
}

READ8_HANDLER( K056832_ram_attr_hi_r )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)];

	return *adr>>8;
}

WRITE8_HANDLER( K056832_ram_code_lo_w )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)+1];

	*adr &= 0xff00;
	*adr |= data;

	if (!(K056832_regs[0] & 0x02))	// external linescroll enable
	{
		if (K056832_PageTileMode[K056832_SelectedPage])
			tilemap_mark_tile_dirty(K056832_tilemap[K056832_SelectedPage], offset);
		else
			K056832_mark_line_dirty(K056832_SelectedPage, offset);
	}
}

WRITE8_HANDLER( K056832_ram_code_hi_w )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)+1];

	*adr &= 0x00ff;
	*adr |= data<<8;

	if (!(K056832_regs[0] & 0x02))	// external linescroll enable
	{
		if (K056832_PageTileMode[K056832_SelectedPage])
			tilemap_mark_tile_dirty(K056832_tilemap[K056832_SelectedPage], offset);
		else
			K056832_mark_line_dirty(K056832_SelectedPage, offset);
	}
}

WRITE8_HANDLER( K056832_ram_attr_lo_w )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)];

	*adr &= 0xff00;
	*adr |= data;

	if (!(K056832_regs[0] & 0x02))	// external linescroll enable
	{
		if (K056832_PageTileMode[K056832_SelectedPage])
			tilemap_mark_tile_dirty(K056832_tilemap[K056832_SelectedPage], offset);
		else
			K056832_mark_line_dirty(K056832_SelectedPage, offset);
	}
}

WRITE8_HANDLER( K056832_ram_attr_hi_w )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(offset*2)];

	*adr &= 0x00ff;
	*adr |= data<<8;

	if (!(K056832_regs[0] & 0x02))	// external linescroll enable
	{
		if (K056832_PageTileMode[K056832_SelectedPage])
			tilemap_mark_tile_dirty(K056832_tilemap[K056832_SelectedPage], offset);
		else
			K056832_mark_line_dirty(K056832_SelectedPage, offset);
	}
}

WRITE16_HANDLER( K056832_ram_word_w )
{
	UINT16 *tile_ptr;
	UINT16 old_mask, old_data;

	tile_ptr = &K056832_videoram[K056832_SelectedPagex4096+offset];
	old_mask = ~mem_mask;
	old_data = *tile_ptr;
	data = (data & mem_mask) | (old_data & old_mask);

	if(data != old_data)
	{
		offset >>= 1;
		*tile_ptr = data;

		if (K056832_PageTileMode[K056832_SelectedPage])
			tilemap_mark_tile_dirty(K056832_tilemap[K056832_SelectedPage], offset);
		else
			K056832_mark_line_dirty(K056832_SelectedPage, offset);
	}
}

WRITE16_HANDLER( K056832_ram_half_word_w )
{
	UINT16 *adr = &K056832_videoram[K056832_SelectedPagex4096+(((offset << 1) & 0xffe) | 1)];
	UINT16 old = *adr;

	COMBINE_DATA(adr);
	if(*adr != old)
	{
		int dofs = (((offset << 1) & 0xffe) | 1);

		dofs >>= 1;

		if (K056832_PageTileMode[K056832_SelectedPage])
			tilemap_mark_tile_dirty(K056832_tilemap[K056832_SelectedPage], dofs);
		else
       			K056832_mark_line_dirty(K056832_SelectedPage, dofs);
	}
}

WRITE32_HANDLER( K056832_ram_long_w )
{
	UINT16 *tile_ptr;
	UINT32 old_mask, old_data;

	tile_ptr = &K056832_videoram[K056832_SelectedPagex4096+offset*2];
	old_mask = ~mem_mask;
	old_data = (UINT32)tile_ptr[0]<<16 | (UINT32)tile_ptr[1];
	data = (data & mem_mask) | (old_data & old_mask);

	if (data != old_data)
	{
		tile_ptr[0] = data>>16;
		tile_ptr[1] = data;

		if (K056832_PageTileMode[K056832_SelectedPage])
			tilemap_mark_tile_dirty(K056832_tilemap[K056832_SelectedPage], offset);
		else
			K056832_mark_line_dirty(K056832_SelectedPage, offset);
	}
}

WRITE16_HANDLER( K056832_word_w )
{
	int layer, flip, mask, i;
	UINT32 old_data, new_data;

	old_data = K056832_regs[offset];
	COMBINE_DATA(&K056832_regs[offset]);
	new_data = K056832_regs[offset];

	if (new_data != old_data)
	{
		switch(offset)
		{
			/* -x-- ---- dotclock select: 0=8Mhz, 1=6Mhz (not used by GX)
             * --x- ---- screen flip y
             * ---x ---- screen flip x
             * ---- --x- external linescroll RAM page enable
             */
			case 0x00/2:
				if ((new_data & 0x30) != (old_data & 0x30))
				{
					flip = 0;
					if (new_data & 0x20) flip |= TILEMAP_FLIPY;
					if (new_data & 0x10) flip |= TILEMAP_FLIPX;
					for (i=0; i<K056832_PAGE_COUNT; i++)
					{
						tilemap_set_flip(K056832_tilemap[i], flip);
					}
				}

				if ((new_data & 0x02) != (old_data & 0x02))
				{
					K056832_change_rambank();
				}
			break;

			/* -------- -----xxx external irqlines enable (not used by GX)
             * -------- xx------ tilemap attribute config (FBIT0 and FBIT1)
             */
			//case 0x06/2: break;

			// -------- ----DCBA tile mode: 0=512x1, 1=8x8
			// -------- DCBA---- synchronous scroll: 0=off, 1=on
			case 0x08/2:
				for (layer=0; layer<4; layer++)
				{
					mask = 1<<layer;
					i = new_data & mask;
					if (i != (old_data & mask))
					{
						K056832_LayerTileMode[layer] = i;
						K056832_mark_plane_dirty(layer);
					}
				}
			break;

			/* -------- ------xx layer A linescroll config
             * -------- ----xx-- layer B linescroll config
             * -------- --xx---- layer C linescroll config
             * -------- xx------ layer D linescroll config
             *
             * 0: linescroll
             * 2: rowscroll
             * 3: xy scroll
             */
			//case 0x0a/2: break;

			case 0x32/2:
				K056832_change_rambank();
			break;

			case 0x34/2: /* ROM bank select for checksum */
			case 0x36/2: /* secondary ROM bank select for use with tile banking */
				K056832_change_rombank();
			break;

			// extended tile address
			//case 0x38/2: break;

			// 12 bit (signed) horizontal offset if global HFLIP enabled
			//case 0x3a/2: break;

			// 11 bit (signed) vertical offset if global VFLIP enabled
			//case 0x3c/2: break;

			default:
				layer = offset & 3;

				if (offset >= 0x10/2 && offset <= 0x16/2)
				{
					K056832_Y[layer] = (new_data&0x18)>>3;
					K056832_H[layer] = (new_data&0x3);
					K056832_ActiveLayer = layer;
					K056832_UpdatePageLayout();
				} else

				if (offset >= 0x18/2 && offset <= 0x1e/2)
				{
					K056832_X[layer] = (new_data&0x18)>>3;
					K056832_W[layer] = (new_data&0x03);
					K056832_ActiveLayer = layer;
					K056832_UpdatePageLayout();
				} else

				if (offset >= 0x20/2 && offset <= 0x26/2)
				{
					K056832_dy[layer] = (INT16)new_data;
				} else

				if (offset >= 0x28/2 && offset <= 0x2e/2)
				{
					K056832_dx[layer] = (INT16)new_data;
				}
			break;
		}
	}
}

WRITE32_HANDLER( K056832_long_w )
{
	// GX does access of all 3 widths (8/16/32) so we can't do the
	// if (ACCESSING_xxx) trick.  in particular, 8-bit writes
	// are used to the tilemap bank register.
	offset <<= 1;
	K056832_word_w(space, offset, data>>16, mem_mask >> 16);
	K056832_word_w(space, offset+1, data, mem_mask);
}

WRITE16_HANDLER( K056832_b_word_w )
{
	COMBINE_DATA( &K056832_regsb[offset] );
}

WRITE8_HANDLER( K056832_w )
{
	if (offset & 1)
	{
		K056832_word_w(space, (offset>>1), data, 0x00ff);
	}
	else
	{
		K056832_word_w(space, (offset>>1), data<<8, 0xff00);
	}
}

WRITE8_HANDLER( K056832_b_w )
{
	if (offset & 1)
	{
		K056832_b_word_w(space, (offset>>1), data, 0x00ff);
	}
	else
	{
		K056832_b_word_w(space, (offset>>1), data<<8, 0xff00);
	}
}

WRITE32_HANDLER( K056832_b_long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		K056832_b_word_w(space, offset<<1, data>>16, mem_mask >> 16);
	}
	if (ACCESSING_BITS_0_15)
	{
		K056832_b_word_w(space, (offset<<1)+1, data, mem_mask);
	}
}

static int K056832_update_linemap(running_machine *machine, bitmap_t *bitmap, int page, int flags)
{

	if (K056832_PageTileMode[page]) return(0);
	if (!K056832_linemap_enabled) return(1);


	{

		rectangle zerorect;
		tilemap *tmap;
		UINT32 *dirty;
		int all_dirty;
		bitmap_t* xprmap;
		UINT8 *xprdata;

		tmap = K056832_tilemap[page];
		xprmap  = tilemap_get_flagsmap(tmap);
		xprdata = tilemap_get_tile_flags(tmap);

		dirty = K056832_LineDirty[page];
		all_dirty = K056832_AllLinesDirty[page];

		if (all_dirty)
		{
			dirty[7]=dirty[6]=dirty[5]=dirty[4]=dirty[3]=dirty[2]=dirty[1]=dirty[0] = 0;
			K056832_AllLinesDirty[page] = 0;

			// force tilemap into a clean, static state
			// *really ugly but it minimizes alteration to tilemap.c
			memset (&zerorect, 0, sizeof(rectangle));	// zero dimension
			tilemap_draw(bitmap, &zerorect, tmap, 0, 0);	// dummy call to reset tile_dirty_map
			bitmap_fill(xprmap, 0, 0);						// reset pixel transparency_bitmap;
			memset(xprdata, TILEMAP_PIXEL_LAYER0, 0x800);	// reset tile transparency_data;
		}
		else
		{
			if (!(dirty[0]|dirty[1]|dirty[2]|dirty[3]|dirty[4]|dirty[5]|dirty[6]|dirty[7])) return(0);
		}

#if 0	/* this code is broken.. really broken .. gijoe uses it for some line/column scroll style effects (lift level of attract mode)
            we REALLY shouldn't be writing directly back into the pixmap, surely this should
            be done when rendering instead

        */
		{


			bitmap_t *pixmap;

			UINT8 code_transparent, code_opaque;
			const pen_t *pal_ptr;
			const UINT8  *src_ptr;
			UINT8  *xpr_ptr;
			UINT16 *dst_ptr;
			UINT16 pen, basepen;
			int count, src_pitch, src_modulo;
			int	dst_pitch;
			int line;
			const gfx_element *src_gfx;
			int offs, mask;

			#define LINE_WIDTH 512

			#define DRAW_PIX(N) \
				pen = src_ptr[N]; \
				if (pen) \
				{ pen += basepen; xpr_ptr[count+N] = TILEMAP_PIXEL_LAYER0; dst_ptr[count+N] = pen; } else \
				{ xpr_ptr[count+N] = 0; }

			pixmap  = K056832_pixmap[page];
			pal_ptr    = machine->pens;
			src_gfx    = machine->gfx[K056832_gfxnum];
			src_pitch  = src_gfx->line_modulo;
			src_modulo = src_gfx->char_modulo;
			dst_pitch  = pixmap->rowpixels;

			for (line=0; line<256; line++)
			{


				tile_data tileinfo = {0};

				dst_ptr = BITMAP_ADDR16(pixmap, line, 0);
				xpr_ptr = BITMAP_ADDR8(xprmap, line, 0);

				if (!all_dirty)
				{
					offs = line >> 5;
					mask = 1 << (line & 0x1f);
					if (!(dirty[offs] & mask)) continue;
					dirty[offs] ^= mask;
				}

				for (count = 0; count < LINE_WIDTH; count+=8)
				{
					K056832_get_tile_info(machine, &tileinfo, line, page);
					basepen = tileinfo.palette_base;
					code_transparent = tileinfo.category;
					code_opaque = code_transparent | TILEMAP_PIXEL_LAYER0;

					src_ptr = tileinfo.pen_data + count*8;//src_base + ((tileinfo.tile_number & ~7) << 6);

					DRAW_PIX(0)
					DRAW_PIX(1)
					DRAW_PIX(2)
					DRAW_PIX(3)
					DRAW_PIX(4)
					DRAW_PIX(5)
					DRAW_PIX(6)
					DRAW_PIX(7)
				}
			}

			#undef LINE_WIDTH
			#undef DRAW_PIX
		}
#endif

	}

	return(0);
}

void K056832_tilemap_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int layer, UINT32 flags, UINT32 priority)
{
	static int last_colorbase[K056832_PAGE_COUNT];

	UINT32 last_dx, last_visible, new_colorbase, last_active;
	int sx, sy, ay, tx, ty, width, height;
	int clipw, clipx, cliph, clipy, clipmaxy;
	int line_height, line_endy, line_starty, line_y;
	int sdat_start, sdat_walk, sdat_adv, sdat_wrapmask, sdat_offs;
	int pageIndex, flipx, flipy, corr, r, c;
	int cminy, cmaxy, cminx, cmaxx;
	int dminy, dmaxy, dminx, dmaxx;
	rectangle drawrect;
	tilemap *tmap;
	UINT16 *pScrollData;
	UINT16 ram16[2];

	int rowstart = K056832_Y[layer];
	int colstart = K056832_X[layer];
	int rowspan  = K056832_H[layer]+1;
	int colspan  = K056832_W[layer]+1;
	int dy = K056832_dy[layer];
	int dx = K056832_dx[layer];
	int scrollbank = ((K056832_regs[0x18]>>1) & 0xc) | (K056832_regs[0x18] & 3);
	int scrollmode = K056832_regs[0x05]>>(K056832_LSRAMPage[layer][0]<<1) & 3;

	if (K056832_use_ext_linescroll)
	{
		scrollbank = K056832_PAGE_COUNT;
	}

	height = rowspan * K056832_PAGE_HEIGHT;
	width  = colspan * K056832_PAGE_WIDTH;

	cminx = cliprect->min_x;
	cmaxx = cliprect->max_x;
	cminy = cliprect->min_y;
	cmaxy = cliprect->max_y;

	// flip correction registers
	flipy = K056832_regs[0] & 0x20;
	if (flipy)
	{
		corr = K056832_regs[0x3c/2];
		if (corr & 0x400)
			corr |= 0xfffff800;
	} else corr = 0;
	dy += corr;
	ay = (unsigned)(dy - K056832_LayerOffset[layer][1]) % height;

	flipx = K056832_regs[0] & 0x10;
	if (flipx)
	{
		corr = K056832_regs[0x3a/2];
		if (corr & 0x800)
			corr |= 0xfffff000;
	} else corr = 0;
	corr -= K056832_LayerOffset[layer][0];

	if (scrollmode == 0 && (flags & K056382_DRAW_FLAG_FORCE_XYSCROLL))
	{
		scrollmode = 3;
		flags &= ~K056382_DRAW_FLAG_FORCE_XYSCROLL;
	}

	switch( scrollmode )
	{
		case 0: // linescroll
			pScrollData = &K056832_videoram[scrollbank<<12] + (K056832_LSRAMPage[layer][1]>>1);
			line_height = 1;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 2;
		break;
		case 2: // rowscroll

			pScrollData = &K056832_videoram[scrollbank<<12] + (K056832_LSRAMPage[layer][1]>>1);
			line_height = 8;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 16;
		break;
		default: // xyscroll
			pScrollData = ram16;
			line_height = K056832_PAGE_HEIGHT;
			sdat_wrapmask = 0;
			sdat_adv = 0;
			ram16[0] = 0;
			ram16[1] = dx;
	}
	if (flipy) sdat_adv = -sdat_adv;

	last_active = K056832_ActiveLayer;
	new_colorbase = (K056832_UpdateMode) ? K055555_get_palette_index(layer) : 0;

  for (r=0; r<rowspan; r++)
  {
	if (rowspan > 1)
	{
		sy = ay;
		ty = r * K056832_PAGE_HEIGHT;

		if (!flipy)
		{
			// handle bottom-edge wraparoundness and cull off-screen tilemaps
			if ((r == 0) && (sy > height - K056832_PAGE_HEIGHT)) sy -= height;
			if ((sy + K056832_PAGE_HEIGHT <= ty) || (sy - K056832_PAGE_HEIGHT >= ty)) continue;

			// switch frame of reference and clip y
			if ((ty -= sy) >= 0)
			{
				cliph = K056832_PAGE_HEIGHT - ty;
				clipy = line_starty = ty;
				line_endy = K056832_PAGE_HEIGHT;
				sdat_start = 0;
			}
			else
			{
				cliph = K056832_PAGE_HEIGHT + ty;
				ty = -ty;
				clipy = line_starty = 0;
				line_endy = cliph;
				sdat_start = ty;
				if (scrollmode == 2) { sdat_start &= ~7; line_starty -= ty & 7; }
			}
		}
		else
		{
			ty += K056832_PAGE_HEIGHT;

			// handle top-edge wraparoundness and cull off-screen tilemaps
			if ((r == rowspan-1) && (sy < K056832_PAGE_HEIGHT)) sy += height;
			if ((sy + K056832_PAGE_HEIGHT <= ty) || (sy - K056832_PAGE_HEIGHT >= ty)) continue;

			// switch frame of reference and clip y
			if ((ty -= sy) <= 0)
			{
				cliph = K056832_PAGE_HEIGHT + ty;
				clipy = line_starty = -ty;
				line_endy = K056832_PAGE_HEIGHT;
				sdat_start = K056832_PAGE_HEIGHT-1;
				if (scrollmode == 2) sdat_start &= ~7;
			}
			else
			{
				cliph = K056832_PAGE_HEIGHT - ty;
				clipy = line_starty = 0;
				line_endy = cliph;
				sdat_start = cliph-1;
				if (scrollmode == 2) { sdat_start &= ~7; line_starty -= ty & 7; }
			}
		}
	}
	else
	{
		cliph = line_endy = K056832_PAGE_HEIGHT;
		clipy = line_starty = 0;

		if (!flipy)
			sdat_start = dy;
		else
			/*
                doesn't work with Metamorphic Force and Martial Champion (software Y-flipped) but
                LE2U (naturally Y-flipped) seems to expect this condition as an override.

                sdat_start = K056832_PAGE_HEIGHT-1 -dy;
            */
			sdat_start = K056832_PAGE_HEIGHT-1;

		if (scrollmode == 2) { sdat_start &= ~7; line_starty -= dy & 7; }
	}

	sdat_start += r * K056832_PAGE_HEIGHT;
	sdat_start <<= 1;

	clipmaxy = clipy + cliph - 1;

	for (c=0; c<colspan; c++)
	{
		pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);

		if (K056832_LayerAssociation)
		{
			if (K056832_LayerAssociatedWithPage[pageIndex] != layer) continue;
		}
		else
		{
			if (K056832_LayerAssociatedWithPage[pageIndex] == -1) continue;
			K056832_ActiveLayer = layer;
		}

		if (K056832_UpdateMode)
		{
			if (last_colorbase[pageIndex] != new_colorbase)
			{
				last_colorbase[pageIndex] = new_colorbase;
				K056832_mark_page_dirty(pageIndex);
			}
		}
		else
			if (!pageIndex) K056832_ActiveLayer = 0;

		if (K056832_update_linemap(machine, bitmap, pageIndex, flags)) continue;

		tmap = K056832_tilemap[pageIndex];
		tilemap_set_scrolly(tmap, 0, ay);

		last_dx = 0x100000;
		last_visible = 0;

		for (sdat_walk=sdat_start, line_y=line_starty; line_y<line_endy; sdat_walk+=sdat_adv, line_y+=line_height)
		{
			dminy = line_y;
			dmaxy = line_y + line_height - 1;

			if (dminy < clipy) dminy = clipy;
			if (dmaxy > clipmaxy) dmaxy = clipmaxy;
			if (dminy > cmaxy || dmaxy < cminy) continue;

			sdat_offs = sdat_walk & sdat_wrapmask;

			drawrect.min_y = (dminy < cminy ) ? cminy : dminy;
			drawrect.max_y = (dmaxy > cmaxy ) ? cmaxy : dmaxy;

			dx = ((int)pScrollData[sdat_offs]<<16 | (int)pScrollData[sdat_offs+1]) + corr;

			if (last_dx == dx) { if (last_visible) goto LINE_SHORTCIRCUIT; continue; }
			last_dx = dx;

			if (colspan > 1)
			{
				//sx = (unsigned)dx % width;
				sx = (unsigned)dx & (width-1);

				//tx = c * K056832_PAGE_WIDTH;
				tx = c << 9;

				if (!flipx)
				{
					// handle right-edge wraparoundness and cull off-screen tilemaps
					if ((c == 0) && (sx > width - K056832_PAGE_WIDTH)) sx -= width;
					if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
						{ last_visible = 0; continue; }

					// switch frame of reference and clip x
					if ((tx -= sx) <= 0) { clipw = K056832_PAGE_WIDTH + tx; clipx = 0; }
					else { clipw = K056832_PAGE_WIDTH - tx; clipx = tx; }
				}
				else
				{
					tx += K056832_PAGE_WIDTH;

					// handle left-edge wraparoundness and cull off-screen tilemaps
					if ((c == colspan-1) && (sx < K056832_PAGE_WIDTH)) sx += width;
					if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
						{ last_visible = 0; continue; }

					// switch frame of reference and clip y
					if ((tx -= sx) >= 0) { clipw = K056832_PAGE_WIDTH - tx; clipx = 0; }
					else { clipw = K056832_PAGE_WIDTH + tx; clipx = -tx; }
				}
			}
			else { clipw = K056832_PAGE_WIDTH; clipx = 0; }

			last_visible = 1;

			dminx = clipx;
			dmaxx = clipx + clipw - 1;

			drawrect.min_x = (dminx < cminx ) ? cminx : dminx;
			drawrect.max_x = (dmaxx > cmaxx ) ? cmaxx : dmaxx;

			// soccer superstars visible area is >512 pixels, this causes problems with the logic because
			// the tilemaps are 512 pixels across.  Assume that if the limits were set as below that we
			// want the tilemap to be drawn on the right hand side..  this is probably not the correct
			// logic, but it works.
			if ((drawrect.min_x>0) && (drawrect.max_x==511)) drawrect.max_x=cliprect->max_x;

			tilemap_set_scrollx(tmap, 0, dx);

			LINE_SHORTCIRCUIT:
			tilemap_draw(bitmap, &drawrect, tmap, flags, priority);

		} // end of line loop
	} // end of column loop
  } // end of row loop

	K056832_ActiveLayer = last_active;

} // end of function

void K056832_tilemap_draw_dj(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int layer, UINT32 flags, UINT32 priority) //*
{
	static int last_colorbase[K056832_PAGE_COUNT];

	UINT32 last_dx, last_visible, new_colorbase, last_active;
	int sx, sy, ay, tx, ty, width, height;
	int clipw, clipx, cliph, clipy, clipmaxy;
	int line_height, line_endy, line_starty, line_y;
	int sdat_start, sdat_walk, sdat_adv, sdat_wrapmask, sdat_offs;
	int pageIndex, flipx, flipy, corr, r, c;
	int cminy, cmaxy, cminx, cmaxx;
	int dminy, dmaxy, dminx, dmaxx;
	rectangle drawrect;
	tilemap *tmap;
	UINT16 *pScrollData;
	UINT16 ram16[2];

	int rowstart = K056832_Y[layer];
	int colstart = K056832_X[layer];
	int rowspan  = K056832_H[layer]+1;
	int colspan  = K056832_W[layer]+1;
	int dy = K056832_dy[layer];
	int dx = K056832_dx[layer];
	int scrollbank = ((K056832_regs[0x18]>>1) & 0xc) | (K056832_regs[0x18] & 3);
	int scrollmode = K056832_regs[0x05]>>(K056832_LSRAMPage[layer][0]<<1) & 3;
	int need_wrap = -1;

	height = rowspan * K056832_PAGE_HEIGHT;
	width  = colspan * K056832_PAGE_WIDTH;

	cminx = cliprect->min_x;
	cmaxx = cliprect->max_x;
	cminy = cliprect->min_y;
	cmaxy = cliprect->max_y;

	// flip correction registers
	flipy = K056832_regs[0] & 0x20;
	if (flipy)
	{
		corr = K056832_regs[0x3c/2];
		if (corr & 0x400)
			corr |= 0xfffff800;
	} else corr = 0;
	dy += corr;
	ay = (unsigned)(dy - K056832_LayerOffset[layer][1]) % height;

	flipx = K056832_regs[0] & 0x10;
	if (flipx)
	{
		corr = K056832_regs[0x3a/2];
		if (corr & 0x800)
			corr |= 0xfffff000;
	} else corr = 0;
	corr -= K056832_LayerOffset[layer][0];

	if (scrollmode == 0 && (flags & K056382_DRAW_FLAG_FORCE_XYSCROLL))
	{
		scrollmode = 3;
		flags &= ~K056382_DRAW_FLAG_FORCE_XYSCROLL;
	}

	switch( scrollmode )
	{
		case 0: // linescroll
			pScrollData = &K056832_videoram[scrollbank<<12] + (K056832_LSRAMPage[layer][1]>>1);
			line_height = 1;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 2;
		break;
		case 2: // rowscroll
			pScrollData = &K056832_videoram[scrollbank<<12] + (K056832_LSRAMPage[layer][1]>>1);
			line_height = 8;
			sdat_wrapmask = 0x3ff;
			sdat_adv = 16;
		break;
		default: // xyscroll
			pScrollData = ram16;
			line_height = K056832_PAGE_HEIGHT;
			sdat_wrapmask = 0;
			sdat_adv = 0;
			ram16[0] = 0;
			ram16[1] = dx;
	}
	if (flipy) sdat_adv = -sdat_adv;

	last_active = K056832_ActiveLayer;
	new_colorbase = (K056832_UpdateMode) ? K055555_get_palette_index(layer) : 0;

  for (r=0; r<=rowspan; r++)
  {
		sy = ay;
		if (r == rowspan)
		{
			if (need_wrap < 0)
				continue;

			ty = need_wrap * K056832_PAGE_HEIGHT;
		}
		else
		{
			ty = r * K056832_PAGE_HEIGHT;
		}

		// cull off-screen tilemaps
		if ((sy + height <= ty) || (sy - height >= ty)) continue;

		// switch frame of reference
		ty -= sy;

			// handle top-edge wraparoundness
			if (r == rowspan)
			{
				cliph = K056832_PAGE_HEIGHT + ty;
				clipy = line_starty = 0;
				line_endy = cliph;
				ty = -ty;
				sdat_start = ty;
				if (scrollmode == 2) { sdat_start &= ~7; line_starty -= ty & 7; }
			}

			// clip y
			else
			{
				if (ty < 0)
					ty += height;

				clipy = ty;
				cliph = K056832_PAGE_HEIGHT;

				if (clipy + cliph > height)
				{
					cliph = height - clipy;
					need_wrap =r;
				}

				line_starty = ty;
				line_endy = line_starty + cliph;
				sdat_start = 0;
			}

	if (r == rowspan)
		sdat_start += need_wrap * K056832_PAGE_HEIGHT;
	else
		sdat_start += r * K056832_PAGE_HEIGHT;
	sdat_start <<= 1;

	clipmaxy = clipy + cliph - 1;

	for (c=0; c<colspan; c++)
	{
		if (r == rowspan)
			pageIndex = (((rowstart + need_wrap) & 3) << 2) + ((colstart + c) & 3);
		else
			pageIndex = (((rowstart + r) & 3) << 2) + ((colstart + c) & 3);

		if (K056832_LayerAssociation)
		{
			if (K056832_LayerAssociatedWithPage[pageIndex] != layer) continue;
		}
		else
		{
			if (K056832_LayerAssociatedWithPage[pageIndex] == -1) continue;
			K056832_ActiveLayer = layer;
		}

		if (K056832_UpdateMode)
		{
			if (last_colorbase[pageIndex] != new_colorbase)
			{
				last_colorbase[pageIndex] = new_colorbase;
				K056832_mark_page_dirty(pageIndex);
			}
		}
		else
			if (!pageIndex) K056832_ActiveLayer = 0;

		if (K056832_update_linemap(machine, bitmap, pageIndex, flags)) continue;

		tmap = K056832_tilemap[pageIndex];
		tilemap_set_scrolly(tmap, 0, ay);

		last_dx = 0x100000;
		last_visible = 0;

		for (sdat_walk=sdat_start, line_y=line_starty; line_y<line_endy; sdat_walk+=sdat_adv, line_y+=line_height)
		{
			dminy = line_y;
			dmaxy = line_y + line_height - 1;

			if (dminy < clipy) dminy = clipy;
			if (dmaxy > clipmaxy) dmaxy = clipmaxy;
			if (dminy > cmaxy || dmaxy < cminy) continue;

			sdat_offs = sdat_walk & sdat_wrapmask;

			drawrect.min_y = (dminy < cminy ) ? cminy : dminy;
			drawrect.max_y = (dmaxy > cmaxy ) ? cmaxy : dmaxy;

			dx = ((int)pScrollData[sdat_offs]<<16 | (int)pScrollData[sdat_offs+1]) + corr;

			if (last_dx == dx) { if (last_visible) goto LINE_SHORTCIRCUIT; continue; }
			last_dx = dx;

			if (colspan > 1)
			{
				//sx = (unsigned)dx % width;
				sx = (unsigned)dx & (width-1);

				//tx = c * K056832_PAGE_WIDTH;
				tx = c << 9;

				if (!flipx)
				{
					// handle right-edge wraparoundness and cull off-screen tilemaps
					if ((c == 0) && (sx > width - K056832_PAGE_WIDTH)) sx -= width;
					if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
						{ last_visible = 0; continue; }

					// switch frame of reference and clip x
					if ((tx -= sx) <= 0) { clipw = K056832_PAGE_WIDTH + tx; clipx = 0; }
					else { clipw = K056832_PAGE_WIDTH - tx; clipx = tx; }
				}
				else
				{
					tx += K056832_PAGE_WIDTH;

					// handle left-edge wraparoundness and cull off-screen tilemaps
					if ((c == colspan-1) && (sx < K056832_PAGE_WIDTH)) sx += width;
					if ((sx + K056832_PAGE_WIDTH <= tx) || (sx - K056832_PAGE_WIDTH >= tx))
						{ last_visible = 0; continue; }

					// switch frame of reference and clip y
					if ((tx -= sx) >= 0) { clipw = K056832_PAGE_WIDTH - tx; clipx = 0; }
					else { clipw = K056832_PAGE_WIDTH + tx; clipx = -tx; }
				}
			}
			else { clipw = K056832_PAGE_WIDTH; clipx = 0; }

			last_visible = 1;

			dminx = clipx;
			dmaxx = clipx + clipw - 1;

			drawrect.min_x = (dminx < cminx ) ? cminx : dminx;
			drawrect.max_x = (dmaxx > cmaxx ) ? cmaxx : dmaxx;

			tilemap_set_scrollx(tmap, 0, dx);

			LINE_SHORTCIRCUIT:
			tilemap_draw(bitmap, &drawrect, tmap, flags, priority);

		} // end of line loop
	} // end of column loop
  } // end of row loop

	K056832_ActiveLayer = last_active;

} // end of function

void K056832_set_LayerAssociation(int status)
{
	K056832_DefaultLayerAssociation = status;
}

int K056832_get_LayerAssociation(void)
{
	return(K056832_LayerAssociation);
}

void K056832_set_LayerOffset(int layer, int offsx, int offsy)
{
	K056832_LayerOffset[layer][0] = offsx;
	K056832_LayerOffset[layer][1] = offsy;
}

void K056832_set_LSRAMPage(int logical_page, int physical_page, int physical_offset)
{
	K056832_LSRAMPage[logical_page][0] = physical_page;
	K056832_LSRAMPage[logical_page][1] = physical_offset;
}

void K056832_set_UpdateMode(int mode)
{
	K056832_UpdateMode = mode;
}

void K056832_linemap_enable(int enable)
{
	K056832_linemap_enabled = enable;
}

int K056832_is_IRQ_enabled(int irqline)
{
	return(K056832_regs[0x06/2] & (1<<irqline & 7));
}

void K056832_read_AVAC(int *mode, int *data)
{
	*mode = K056832_regs[0x04/2] & 7;
	*data = K056832_regs[0x38/2];
}

int K056832_read_register(int regnum)
{
	return(K056832_regs[regnum]);
}



/***************************************************************************/
/*                                                                         */
/*                                 055555                                  */
/*                                                                         */
/***************************************************************************/

/* K055555 5-bit-per-pixel priority encoder */
/* This device has 48 8-bit-wide registers */

static UINT8 k55555_regs[128];

void K055555_vh_start(running_machine *machine)
{
	state_save_register_global_array(machine, k55555_regs);

	memset(k55555_regs, 0, 64*sizeof(UINT8));
}

void K055555_write_reg(UINT8 regnum, UINT8 regdat)
{
	static const char *const rnames[46] =
	{
		"BGC CBLK", "BGC SET", "COLSET0", "COLSET1", "COLSET2", "COLSET3", "COLCHG ON",
		"A PRI 0", "A PRI 1", "A COLPRI", "B PRI 0", "B PRI 1", "B COLPRI", "C PRI", "D PRI",
		"OBJ PRI", "SUB1 PRI", "SUB2 PRI", "SUB3 PRI", "OBJ INPRI ON", "S1 INPRI ON", "S2 INPRI ON",
		"S3 INPRI ON", "A PAL", "B PAL", "C PAL", "D PAL", "OBJ PAL", "SUB1 PAL", "SUB2 PAL", "SUB3 PAL",
		"SUB2 PAL ON", "SUB3 PAL ON", "V INMIX", "V INMIX ON", "OS INMIX", "OS INMIX ON", "SHD PRI 1",
		"SHD PRI 2", "SHD PRI 3", "SHD ON", "SHD PRI SEL", "V BRI", "OS INBRI", "OS INBRI ON", "ENABLE"
	};

	if (regdat != k55555_regs[regnum])
	{
		LOG(("5^5: %x to reg %x (%s)\n", regdat, regnum, rnames[regnum]));
	}

	k55555_regs[regnum] = regdat;
}

WRITE32_HANDLER( K055555_long_w )
{
	UINT8 regnum, regdat;

	if (ACCESSING_BITS_24_31)
	{
		regnum = offset<<1;
		regdat = data>>24;
	}
	else
	{
		if (ACCESSING_BITS_8_15)
		{
			regnum = (offset<<1)+1;
			regdat = data>>8;
		}
		else
		{
//          logerror("5^5: unknown mem_mask %08x\n", mem_mask);
			return;
		}
	}

	K055555_write_reg(regnum, regdat);
}

WRITE16_HANDLER( K055555_word_w )
{
	if (mem_mask == 0x00ff)
	{
		K055555_write_reg(offset, data&0xff);
	}
	else
	{
		K055555_write_reg(offset, data>>8);
	}
}

int K055555_read_register(int regnum)
{
	return(k55555_regs[regnum]);
}

int K055555_get_palette_index(int idx)
{
	return(k55555_regs[K55_PALBASE_A + idx]);
}



/***************************************************************************/
/*                                                                         */
/*                                 054338                                  */
/*                                                                         */
/***************************************************************************/

static UINT16 k54338_regs[32];
static int K054338_shdRGB[9];
static int K054338_alphainverted;


// K054338 alpha blend / final mixer (normally used with the 55555)
// because the implementation is video dependant, this is just a
// register-handling shell.
void K054338_vh_start(running_machine *machine)
{
	memset(k54338_regs, 0, sizeof(UINT16)*32);
	memset(K054338_shdRGB, 0, sizeof(int)*9);
	K054338_alphainverted = 1;

	state_save_register_global_array(machine, k54338_regs);
}

WRITE16_HANDLER( K054338_word_w )
{
	COMBINE_DATA(k54338_regs + offset);
}

WRITE32_HANDLER( K054338_long_w )
{
	offset <<= 1;
	K054338_word_w(space, offset, data>>16, mem_mask>>16);
	K054338_word_w(space, offset+1, data, mem_mask);
}

// returns a 16-bit '338 register
int K054338_read_register(int reg)
{
	return k54338_regs[reg];
}

void K054338_update_all_shadows(running_machine *machine, int rushingheroes_hack)
{
	int i, d;
	int noclip = k54338_regs[K338_REG_CONTROL] & K338_CTL_CLIPSL;

	for (i=0; i<9; i++)
	{
		d = k54338_regs[K338_REG_SHAD1R+i] & 0x1ff;
		if (d >= 0x100) d -= 0x200;
		K054338_shdRGB[i] = d;
	}

	if (!rushingheroes_hack)
	{
		palette_set_shadow_dRGB32(machine, 0, K054338_shdRGB[0], K054338_shdRGB[1], K054338_shdRGB[2], noclip);
		palette_set_shadow_dRGB32(machine, 1, K054338_shdRGB[3], K054338_shdRGB[4], K054338_shdRGB[5], noclip);
		palette_set_shadow_dRGB32(machine, 2, K054338_shdRGB[6], K054338_shdRGB[7], K054338_shdRGB[8], noclip);
	}
	else // rushing heroes seems to specify shadows in another format, or it's not being interpreted properly.
	{
		palette_set_shadow_dRGB32(machine, 0, -80, -80, -80, 0);
		palette_set_shadow_dRGB32(machine, 1, -80, -80, -80, 0);
		palette_set_shadow_dRGB32(machine, 2, -80, -80, -80, 0);
	}
}

// K054338 BG color fill
void K054338_fill_solid_bg(bitmap_t *bitmap)
{
	UINT32 bgcolor;
	UINT32 *pLine;
	int x, y;

	bgcolor = (K054338_read_register(K338_REG_BGC_R)&0xff)<<16;
	bgcolor |= K054338_read_register(K338_REG_BGC_GB);

	/* and fill the screen with it */
	for (y = 0; y < bitmap->height; y++)
	{
		pLine = (UINT32 *)bitmap->base;
		pLine += (bitmap->rowpixels*y);
		for (x = 0; x < bitmap->width; x++)
			*pLine++ = bgcolor;
	}
}

// Unified K054338/K055555 BG color fill
void K054338_fill_backcolor(running_machine *machine, bitmap_t *bitmap, int mode) // (see p.67)
{
	int clipx, clipy, clipw, cliph, i, dst_pitch;
	int BGC_CBLK, BGC_SET;
	UINT32 *dst_ptr, *pal_ptr;
	int bgcolor;
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	clipx = visarea->min_x & ~3;
	clipy = visarea->min_y;
	clipw = (visarea->max_x - clipx + 4) & ~3;
	cliph = visarea->max_y - clipy + 1;

	dst_ptr = BITMAP_ADDR32(bitmap, clipy, 0);
	dst_pitch = bitmap->rowpixels;
	dst_ptr += clipx;

	BGC_SET = 0;
	pal_ptr = paletteram32;

	if (!mode)
	{
		// single color output from CLTC
		bgcolor = (int)(k54338_regs[K338_REG_BGC_R]&0xff)<<16 | (int)k54338_regs[K338_REG_BGC_GB];
	}
	else
	{
		BGC_CBLK = K055555_read_register(0);
		BGC_SET  = K055555_read_register(1);
		pal_ptr += BGC_CBLK << 9;

		// single color output from PCU2
		if (!(BGC_SET & 2)) { bgcolor = *pal_ptr; mode = 0; } else bgcolor = 0;
	}

	if (!mode)
	{
		// single color fill
		dst_ptr += clipw;
		i = clipw = -clipw;
		do
		{
			do { dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor; } while (i += 4);
			dst_ptr += dst_pitch;
			i = clipw;
		}
		while (--cliph);
	}
	else
	{
		if (!(BGC_SET & 1))
		{
			// vertical gradient fill
			pal_ptr += clipy;
			dst_ptr += clipw;
			bgcolor = *pal_ptr++;
			i = clipw = -clipw;
			do
			{
				do { dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor; } while (i += 4);
				dst_ptr += dst_pitch;
				bgcolor = *pal_ptr++;
				i = clipw;
			}
			while (--cliph);
		}
		else
		{
			// horizontal gradient fill
			pal_ptr += clipx;
			clipw <<= 2;
			do
			{
				memcpy(dst_ptr, pal_ptr, clipw);
				dst_ptr += dst_pitch;
			}
			while (--cliph);
		}
	}
}

// addition blending unimplemented (requires major changes to drawgfx and tilemap.c)
int K054338_set_alpha_level(int pblend)
{
	UINT16 *regs;
	int ctrl, mixpri, mixset, mixlv;

	if (pblend <= 0 || pblend > 3)
	{
		return(255);
	}

	regs   = k54338_regs;
	ctrl   = k54338_regs[K338_REG_CONTROL];
	mixpri = ctrl & K338_CTL_MIXPRI;
	mixset = regs[K338_REG_PBLEND + (pblend>>1 & 1)] >> (~pblend<<3 & 8);
	mixlv  = mixset & 0x1f;

	if (K054338_alphainverted) mixlv = 0x1f - mixlv;

	if (!(mixset & 0x20))
	{
		mixlv = mixlv<<3 | mixlv>>2;
    }
	else
	{
		if (!mixpri)
		{
			// source x alpha  +  target (clipped at 255)
		}
		else
		{
			// source  +  target x alpha (clipped at 255)
		}

		// DUMMY
		if (mixlv && mixlv<0x1f) mixlv = 0x10;
		mixlv = mixlv<<3 | mixlv>>2;

		if (VERBOSE)
			popmessage("MIXSET%1d %s addition mode: %02x",pblend,(mixpri)?"dst":"src",mixset&0x1f);
	}

	return(mixlv);
}

void K054338_invert_alpha(int invert)
{
	K054338_alphainverted = invert;
}

void K054338_export_config(int **shdRGB)
{
	*shdRGB = K054338_shdRGB;
}



/***************************************************************************/
/*                                                                         */
/*                                 053250                                  */
/*                                                                         */
/***************************************************************************/

struct K053250_CHIPTAG
{
	UINT8 regs[8];
	UINT8 *base;
	UINT16 *ram, *rammax;
	UINT16 *buffer[2];
	UINT32 rommask;
	int page[2];
	int frame, offsx, offsy;
};

static struct
{
	int chips;
	struct K053250_CHIPTAG chip[2];
} K053250_info;

void K053250_set_LayerOffset(int chip, int offsx, int offsy)
{
	K053250_info.chip[chip].offsx = offsx;
	K053250_info.chip[chip].offsy = offsy;
}

// The DMA process should be instantaneous but since rendering in MAME is performed at VIDEO_UPDATE()
// the K053250 memory must be buffered to maintain visual integrity.
void K053250_dma(running_machine *machine, int chip, int limiter)
{
	struct K053250_CHIPTAG *chip_ptr;
	int last_frame, current_frame;

	chip_ptr = &K053250_info.chip[chip];

	current_frame = video_screen_get_frame_number(machine->primary_screen);
	last_frame = chip_ptr->frame;

	if (limiter && current_frame == last_frame) return; // make sure we only do DMA transfer once per frame

	chip_ptr->frame = current_frame;
	memcpy(chip_ptr->buffer[chip_ptr->page[chip]], chip_ptr->ram, 0x1000);
	chip_ptr->page[chip] ^= 1;
}

// Pixel data of the K053250 is nibble packed. It's preferable to be unpacked into byte format.
void K053250_unpack_pixels(running_machine *machine, const char *region)
{
	UINT8 *src_ptr, *dst_ptr;
	int hi_nibble, lo_nibble, offset;

	dst_ptr = src_ptr = memory_region(machine, region);
	offset = memory_region_length(machine, region) / 2 - 1;

	do
	{
		lo_nibble = hi_nibble = src_ptr[offset];
		hi_nibble >>= 4;
		lo_nibble &= 0xf;
		dst_ptr[offset*2  ] = hi_nibble;
		dst_ptr[offset*2+1] = lo_nibble;
	}
	while ((--offset) >= 0);
}

void K053250_vh_start(running_machine *machine, int chips, const char **region)
{
	UINT16 *ram;
	int chip;

	K053250_info.chips = chips;

	for(chip=0; chip<chips; chip++)
	{
		K053250_info.chip[chip].base = memory_region(machine, region[chip]);
		ram = auto_alloc_array(machine, UINT16, 0x6000/2);
		K053250_info.chip[chip].ram = ram;
		K053250_info.chip[chip].rammax = ram + 0x800;
		K053250_info.chip[chip].buffer[0] = ram + 0x2000;
		K053250_info.chip[chip].buffer[1] = ram + 0x2800;
		memset(ram+0x2000, 0, 0x2000);
		K053250_info.chip[chip].rommask = memory_region_length(machine, region[chip]);
		K053250_info.chip[chip].page[1] = K053250_info.chip[chip].page[0] = 0;
		K053250_info.chip[chip].offsy = K053250_info.chip[chip].offsx = 0;
		K053250_info.chip[chip].frame = -1;

		state_save_register_item_pointer(machine, "K053250", NULL, chip, K053250_info.chip[chip].ram,  0x800);
		state_save_register_item_array(machine, "K053250", NULL, chip, K053250_info.chip[chip].regs);
	}
}

WRITE16_HANDLER( K053250_0_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// start LVC DMA transfer at the falling edge of control register's bit1
		if (offset == 4 && !(data & 2) && (K053250_info.chip[0].regs[4] & 2)) K053250_dma(space->machine, 0, 1);

		K053250_info.chip[0].regs[offset] = data;
	}
}

READ16_HANDLER( K053250_0_r )
{
	return K053250_info.chip[0].regs[offset];
}

WRITE16_HANDLER( K053250_0_ram_w )
{
	COMBINE_DATA( K053250_info.chip[0].ram + offset);
}

READ16_HANDLER( K053250_0_ram_r )
{
	return K053250_info.chip[0].ram[offset];
}

READ16_HANDLER( K053250_0_rom_r )
{
//  if (!(K053250_info.chip[0].regs[5] & 1)) logerror("Back: Reading rom memory with enable=0\n");

	return *(K053250_info.chip[0].base + 0x80000*K053250_info.chip[0].regs[6] + 0x800*K053250_info.chip[0].regs[7] + (offset>>1));
}

WRITE16_HANDLER( K053250_1_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// start LVC DMA transfer at the falling edge of control register's bit1
		if (offset == 4 && !(data & 2) && (K053250_info.chip[1].regs[4] & 2)) K053250_dma(space->machine, 1, 1);

		K053250_info.chip[1].regs[offset] = data;
	}
}

READ16_HANDLER( K053250_1_r )
{
	return K053250_info.chip[1].regs[offset];
}

WRITE16_HANDLER( K053250_1_ram_w )
{
	COMBINE_DATA( K053250_info.chip[1].ram + offset);
}

READ16_HANDLER( K053250_1_ram_r )
{
	return K053250_info.chip[1].ram[offset];
}

READ16_HANDLER( K053250_1_rom_r )
{
//  if (!(K053250_info.chip[1].regs[5] & 1)) logerror("Back: Reading rom memory with enable=0\n");

	return *(K053250_info.chip[1].base + 0x80000*K053250_info.chip[1].regs[6] + 0x800*K053250_info.chip[1].regs[7] + (offset>>1));
}

#if 0

static void K053250_pdraw_scanline8(
		bitmap_t *bitmap,int x,int y,int length,
		const UINT8 *src,pen_t *pens,int transparent_pen,UINT32 orient,bitmap_t *priority,UINT8 pri)
{
	/* 8bpp destination */
	if (bitmap->bpp == 8)
	{
		/* adjust in case we're oddly oriented */
		ADJUST_FOR_ORIENTATION(UINT8, orient, bitmap, priority, x, y);

		/* with pen lookups */
		if (pens)
		{
			if (transparent_pen == -1)
				while (length--)
				{
					*dsti = pens[*src++];
					*dstp = pri;
					dsti += xadv;
					dstp += xadv;
				}
			else
				while (length--)
				{
					UINT32 spixel = *src++;
					if (spixel != transparent_pen)
					{
						*dsti = pens[spixel];
						*dstp = pri;
					}
					dsti += xadv;
					dstp += xadv;
				}
		}

		/* without pen lookups */
		else
		{
			if (transparent_pen == -1)
				while (length--)
				{
					*dsti = *src++;
					*dstp = pri;
					dsti += xadv;
					dstp += xadv;
				}
			else
				while (length--)
				{
					UINT32 spixel = *src++;
					if (spixel != transparent_pen)
					{
						*dsti = spixel;
						*dstp = pri;
					}
					dsti += xadv;
					dstp += xadv;
				}
		}
	}

	/* 16bpp destination */
	else if(bitmap->bpp == 16)
	{
		/* adjust in case we're oddly oriented */
		ADJUST_FOR_ORIENTATION(UINT16, orient, bitmap, priority, x, y);
		/* with pen lookups */
		if (pens)
		{
			if (transparent_pen == -1)
				while (length--)
				{
					*dsti = pens[*src++];
					*dstp = pri;
					dsti += xadv;
					dstp += xadv;
				}
			else
				while (length--)
				{
					UINT32 spixel = *src++;
					if (spixel != transparent_pen)
					{
						*dsti = pens[spixel];
						*dstp = pri;
					}
					dsti += xadv;
					dstp += xadv;
				}
		}

		/* without pen lookups */
		else
		{
			if (transparent_pen == -1)
				while (length--)
				{
					*dsti = *src++;
					*dstp = pri;
					dsti += xadv;
					dstp += xadv;
				}
			else
				while (length--)
				{
					UINT32 spixel = *src++;
					if (spixel != transparent_pen)
					{
						*dsti = spixel;
						*dstp = pri;
					}
					dsti += xadv;
					dstp += xadv;
				}
		}
	}

	/* 32bpp destination */
	else
	{
		/* adjust in case we're oddly oriented */
		ADJUST_FOR_ORIENTATION(UINT32, orient, bitmap, priority, x, y);
		/* with pen lookups */
		if (pens)
		{
			if (transparent_pen == -1)
				while (length--)
				{
					*dsti = pens[*src++];
					*dstp = pri;
					dsti += xadv;
					dstp += xadv;
				}
			else
				while (length--)
				{
					UINT32 spixel = *src++;
					if (spixel != transparent_pen)
					{
						*dsti = pens[spixel];
						*dstp = pri;
					}
					dsti += xadv;
					dstp += xadv;
				}
		}

		/* without pen lookups */
		else
		{
			if (transparent_pen == -1)
				while (length--)
				{
					*dsti = *src++;
					*dstp = pri;
					dsti += xadv;
					dstp += xadv;
				}
			else
				while (length--)
				{
					UINT32 spixel = *src++;
					if (spixel != transparent_pen)
					{
						*dsti = spixel;
						*dstp = pri;
					}
					dsti += xadv;
					dstp += xadv;
				}
		}
	}
}

void K053250_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int chip, int colorbase, int pri)
{
	static int pmode[2] = {-1,-1};
	static int kc=-1, kk=0, kxx=-105, kyy=0;

	const rectangle area = *video_screen_get_visible_area(machine->primary_screen);
	UINT16 *line;
	int delta, dim1, dim1_max, dim2_max;
	UINT32 mask1, mask2;
	int sp;
#if 1
	int orientation = ((K053250_info.chip[chip].regs[4] &  8) ? ORIENTATION_FLIP_X : 0)
		| ((K053250_info.chip[chip].regs[4] & 16) ? ORIENTATION_FLIP_Y : 0)
		| ((K053250_info.chip[chip].regs[4] &  1) ? 0 : ORIENTATION_SWAP_XY);
#else
	int orientation = (!K053250_info.chip[chip].regs[4] & 8 ? ORIENTATION_FLIP_X : 0)
		| (K053250_info.chip[chip].regs[4] & 16 ? ORIENTATION_FLIP_Y : 0)
		| (!(K053250_info.chip[chip].regs[4] & 1) ? 0 : ORIENTATION_SWAP_XY);
#endif

	INT16 cur_x = (K053250_info.chip[chip].regs[0] << 8) | K053250_info.chip[chip].regs[1];
	INT16 cur_y = (K053250_info.chip[chip].regs[2] << 8) | K053250_info.chip[chip].regs[3];

	if(pmode[chip] != K053250_info.chip[chip].regs[4]) {
		pmode[chip] = K053250_info.chip[chip].regs[4];
#if 0
		fprintf(stderr, "NEW MODE %d %02x [%x %c%c%c%c%c]\n", chip,
				pmode[chip],
				pmode[chip] >> 5,
				pmode[chip] & 16 ? 'y' : '-',
				pmode[chip] & 8 ? 'x' : '-',
				pmode[chip] & 4 ? 'w' : '-',
				pmode[chip] & 2 ? '?' : '-',
				pmode[chip] & 1 ? 's' : '-');
#endif
	}

	colorbase <<= 4;

	if(orientation & ORIENTATION_SWAP_XY) {
		dim1_max = area.max_x - area.min_x + 1;
		dim2_max = area.max_y - area.min_y + 1;
		// -358 for level 1 boss, huh? -495
		if(orientation & ORIENTATION_FLIP_Y)
			delta = 238 - cur_y;
		else
			delta = kyy + cur_y;
		line = K053250_info.chip[chip].ram + (((area.min_x + cur_x + kxx) & 0x1ff) << 2);
	} else {
		dim1_max = area.max_y - area.min_y + 1;
		dim2_max = area.max_x - area.min_x + 1;
		delta = cur_x + 49;
		line = K053250_info.chip[chip].ram + (((area.min_y + cur_y + 16) & 0x1ff) << 2);
	}

	if(chip && ++kk == 3) {
		int kx=0, kkc = 0;
		kk = 0;
		if(input_code_pressed(machine, KEYCODE_Y)) {
			kx = 1;
			kc--;
			if(kc<-1)
				kc = 511;
		}
		if(input_code_pressed(machine, KEYCODE_U)) {
			kx = 1;
			kc++;
			if(kc==512)
				kc = -1;
		}

		if(input_code_pressed(machine, KEYCODE_T)) {
			kkc = 1;
			kyy--;
		}
		if(input_code_pressed(machine, KEYCODE_V)) {
			kkc = 1;
			kyy++;
		}
		if(input_code_pressed(machine, KEYCODE_F)) {
			kkc = 1;
			kxx--;
		}
		if(input_code_pressed(machine, KEYCODE_G)) {
			kkc = 1;
			kxx++;
		}
#if 0
		if(kx) {
			UINT16 *l1 = line + ((4*kc) & 0x7ff);
			popmessage("Line %d [%02x] (%04x %04x %04x %04x)", kc,
								K053250_info.chip[chip].regs[4],
								l1[0],
								l1[1],
								l1[2],
								l1[3]);
		}

		if(kkc)
			popmessage("(%d, %d)", kxx, kyy);
#endif
	}


	switch(K053250_info.chip[chip].regs[4] >> 5) {
	case 0: // Not sure.  Warp level
		mask1 = 0xffff0000;
		mask2 = 0x0000ffff;
		sp = 0;
		break;
	case 1: // Elaine's background
		mask1 = 0xffff8000;
		mask2 = 0x00007fff;
		sp = 0;
		break;
	case 2:
		mask1 = 0xffff0000;
		mask2 = 0x0000ffff;
		sp = 0;
		break;
	case 4: // Level 1 boss, water level (planet)
		mask1 = 0xffffc000;
		mask2 = 0x00003fff;
		sp = 0;
		break;
	case 7: // Organic level
		mask1 = 0xffff0000;
		mask2 = 0x0000ffff;
		sp = 1;
		break;
	default:
//      logerror("Unknown mode %02x\n", K053250_info.chip[chip].regs[4] & 0xe0);
		mask1 = 0xffff0000;
		mask2 = 0x0000ffff;
		sp = 0;
		break;
	}

	if(K053250_info.chip[chip].regs[4] & 4)
		mask1 = 0;

	for(dim1 = 0; dim1 < dim1_max; dim1++) {
		UINT16 color  = *line++;
		UINT16   start  = *line++;
		UINT16 inc    = *line++;
		INT16    offset = *line++;
		int dim2;
		UINT8 *pixel;
		UINT32 cpos;
		UINT8 scanline[512];
		UINT8 *gbase = K053250_info.chip[chip].base + (((start & 0xff00) << 7) % K053250_info.chip[chip].rommask);

		if(offset >= 0x500)
			offset -= 0x800;

		if(line == K053250_info.chip[chip].rammax)
			line = K053250_info.chip[chip].ram;

		if(!color && !start)
			continue;

		pixel = scanline;
		start = (start & 0xff) << 7;
		cpos = (offset + delta)*inc;
		//      fprintf(stderr, "%3d %08x %04x\n", dim1, cpos, inc);

		for(dim2 = 0; dim2 < dim2_max; dim2++) {
			int romp;
			UINT32 rcpos = cpos;

			if(sp && (rcpos & mask1))
				rcpos += inc << 9;

			if(rcpos & mask1) {
				*pixel++ = 0;
				cpos += inc;
				continue;
			}

			romp = gbase[(((rcpos & mask2)>>7) + start) & 0x7fff];

			if(rcpos & 0x40)
				romp &= 0xf;
			else
				romp >>= 4;
			*pixel++ = romp;
			cpos += inc;
		}
		if(orientation & ORIENTATION_SWAP_XY)
			K053250_pdraw_scanline8(bitmap, area.min_y, area.min_x+dim1, dim2_max, scanline,
							machine->pens + ((dim1 == kc ? 0x200 : colorbase) | ((color & 0x0f) << 4)),
							0, orientation, pri);
		else
			K053250_pdraw_scanline8(bitmap, area.min_x, area.min_y+dim1, dim2_max, scanline,
							machine->pens + ((dim1 == kc ? 0x200 : colorbase) | ((color & 0x0f) << 4)),
							0, orientation, pri);
	}
}

#else

// utility function to render a clipped scanline vertically or horizontally
INLINE void K053250_pdraw_scanline32(bitmap_t *bitmap, const pen_t *palette, UINT8 *source,
		const rectangle *cliprect, int linepos, int scroll, int zoom,
		UINT32 clipmask, UINT32 wrapmask, UINT32 orientation, bitmap_t *priority, UINT8 pri)
{
// a sixteen-bit fixed point resolution should be adequate to our application
#define FIXPOINT_PRECISION		16
#define FIXPOINT_PRECISION_HALF	(1<<(FIXPOINT_PRECISION-1))

	int end_pixel, flip, dst_min, dst_max, dst_start, dst_length;

	UINT32 src_wrapmask;
	UINT8  *src_base;
	//int src_x;
	int src_fx, src_fdx;
	int pix_data, dst_offset;
	const pen_t  *pal_base;
	UINT8  *pri_base;
	UINT32 *dst_base;
	int dst_adv;

	// flip X and flip Y also switch role when the X Y coordinates are swapped
	if (!(orientation & ORIENTATION_SWAP_XY))
	{
		flip = orientation & ORIENTATION_FLIP_X;
		dst_min = cliprect->min_x;
		dst_max = cliprect->max_x;
	}
	else
	{
		flip = orientation & ORIENTATION_FLIP_Y;
		dst_min = cliprect->min_y;
		dst_max = cliprect->max_y;
	}

	if (clipmask)
	{
		// reject scanlines that are outside of the target bitmap's right(bottom) clip boundary
		dst_start = -scroll;
		if (dst_start > dst_max) return;

		// calculate target length
		dst_length = clipmask + 1;
		if (zoom) dst_length = (dst_length << 6) / zoom;

		// reject scanlines that are outside of the target bitmap's left(top) clip boundary
		end_pixel = dst_start + dst_length - 1;
		if (end_pixel < dst_min) return;

		// clip scanline tail
		if ((end_pixel -= dst_max) > 0) dst_length -= end_pixel;

		// reject zero-length scanlines
		if (dst_length <= 0) return;

		// calculate zoom factor
		src_fdx = zoom << (FIXPOINT_PRECISION-6);

		// clip scanline head
		end_pixel = dst_min;
		if ((end_pixel -= dst_start) > 0)
		{
			// chop scanline to the correct length and move target start location to the left(top) clip boundary
			dst_length -= end_pixel;
			dst_start = dst_min;

			// and skip the source for the left(top) clip region
			src_fx = end_pixel * src_fdx + FIXPOINT_PRECISION_HALF;
		}
		else
			// the point five bias is to ensure even distribution of stretched or shrinked pixels
			src_fx = FIXPOINT_PRECISION_HALF;

		// adjust flipped source
		if (flip)
		{
			// start from the target's clipped end if the scanline is flipped
			dst_start = dst_max + dst_min - dst_start - (dst_length-1);

			// and move source start location to the opposite end
			src_fx += (dst_length-1) * src_fdx - 1;
			src_fdx = -src_fdx;
		}
	}
	else
	{
		// draw wrapped scanline at virtual bitmap boundary when source clipping is off
		dst_start = dst_min;
		dst_length = dst_max - dst_min + 1;	// target scanline spans the entire visible area
		src_fdx = zoom << (FIXPOINT_PRECISION-6);

		// pre-advance source for the clipped region
		if (!flip)
			src_fx = (scroll + dst_min) * src_fdx + FIXPOINT_PRECISION_HALF;
		else
		{
			src_fx = (scroll + dst_max) * src_fdx + FIXPOINT_PRECISION_HALF-1;
			src_fdx = -src_fdx;
		}
	}

	if (!(orientation & ORIENTATION_SWAP_XY))
	{
		// calculate target increment for horizontal scanlines which is exactly one
		dst_adv = 1;
		dst_offset = dst_length;
		pri_base = BITMAP_ADDR8(priority, linepos, dst_start + dst_offset);
		dst_base = BITMAP_ADDR32(bitmap, linepos, dst_start + dst_length);
	}
	else
	{
		// calculate target increment for vertical scanlines which is the bitmap's pitch value
		dst_adv = bitmap->rowpixels;
		dst_offset= dst_length * dst_adv;
		pri_base = BITMAP_ADDR8(priority, dst_start, linepos + dst_offset);
		dst_base = BITMAP_ADDR32(bitmap, dst_start, linepos + dst_offset);
	}

	// generalized
	src_base = source;
	//src_x = 0;

	// there is no need to wrap source offsets along with source clipping
	// so we set all bits of the wrapmask to one
	src_wrapmask = (clipmask) ? ~0 : wrapmask;

	pal_base = palette;
	dst_offset = -dst_offset; // negate target offset in order to terminated draw loop at zero condition

	if (pri)
	{
		// draw scanline and update priority bitmap
		do
		{
			pix_data = src_base[(src_fx>>FIXPOINT_PRECISION) & src_wrapmask];
			src_fx += src_fdx;

			if (pix_data)
			{
				pix_data = pal_base[pix_data];
				pri_base[dst_offset] = pri;
				dst_base[dst_offset] = pix_data;
			}
		}
		while (dst_offset += dst_adv);
	}
	else
	{
		// draw scanline but do not update priority bitmap
		do
		{
			pix_data = src_base[(src_fx>>FIXPOINT_PRECISION) & src_wrapmask];
			src_fx += src_fdx;

			if (pix_data)
			{
				dst_base[dst_offset] = pal_base[pix_data];
			}
		}
		while (dst_offset += dst_adv);
	}

#undef FIXPOINT_PRECISION
#undef FIXPOINT_PRECISION_HALF
}

void K053250_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int chip, int colorbase, int flags, int priority)
{
	struct K053250_CHIPTAG *chip_ptr;
	UINT16 *line_ram;
	UINT8 *pix_base, *pix_ptr, *regs;
	const pen_t *pal_base, *pal_ptr;
	UINT32 rommask, src_clipmask, src_wrapmask, dst_wrapmask;
	int map_scrollx, map_scrolly, ctrl, orientation;
	int dst_minx, dst_maxx, dst_miny, dst_maxy;
	int linedata_offs, linedata_adv, line_pos, line_start, line_end, scroll_corr;
	int color, offset, zoom, scroll, passes, i, dst_height;

	chip_ptr = &K053250_info.chip[chip];				// pointer to chip parameters
	line_ram = chip_ptr->buffer[chip_ptr->page[chip]];	// pointer to physical line RAM
	pix_base = chip_ptr->base;							// pointer to source pixel ROM
	rommask  = chip_ptr->rommask;						// source ROM limit
	regs     = chip_ptr->regs;							// pointer to registers group

	map_scrollx = (short)(regs[0]<<8 | regs[1]);		// signed horizontal scroll value
	map_scrolly = (short)(regs[2]<<8 | regs[3]);		// signed vertical scroll value
	map_scrollx -= chip_ptr->offsx;						// add user X offset to horizontal scroll
	map_scrolly -= chip_ptr->offsy;						// add user Y offset to vertical scroll
	ctrl = regs[4];										// register four is the main control register

	// copy visible boundary values to more accessible locations
	dst_minx  = cliprect->min_x;
	dst_maxx  = cliprect->max_x;
	dst_miny  = cliprect->min_y;
	dst_maxy  = cliprect->max_y;

	orientation  = 0;	// orientation defaults to no swapping and no flipping
	dst_height   = 512;	// virtual bitmap height defaults to five hundred and twelve pixels
	linedata_adv = 4;	// line info packets are four words(eight bytes) apart

	{
		// switch X and Y parameters when the first bit of the control register is cleared
		if (!(ctrl & 0x01)) orientation |= ORIENTATION_SWAP_XY;

		// invert X parameters when the forth bit of the control register is set
		if   (ctrl & 0x08)  orientation |= ORIENTATION_FLIP_X;

		// invert Y parameters when the fifth bit of the control register is set
		if   (ctrl & 0x10)  orientation |= ORIENTATION_FLIP_Y;

		switch (ctrl>>5) // the upper four bits of the control register select source and target dimensions
		{
			case 0 :
				// Xexex: L6 galaxies
				// Metam: L4 forest, L5 arena, L6 tower interior, final boss

				// crop source offset between zero and two hundred and fifty-five inclusive,
				// and set virtual bitmap height to two hundred and fifty-six pixels
				src_wrapmask = src_clipmask = 0xff;
				dst_height = 0x100;
			break;
			case 1 :
				// Xexex: prologue, L7 nebulae

				// the source offset is cropped to zero and five hundred and eleven inclusive
				src_wrapmask = src_clipmask = 0x1ff;
			break;
			case 4 :
				// Xexex: L1 sky and boss, L3 planet, L5 poly-face, L7 battle ship patches
				// Metam: L1 summoning circle, L3 caves, L6 gargoyle towers

				// crop source offset between zero and two hundred and fifty-five inclusive,
				// and allow source offset to wrap back at 500 hexadecimal to minus 300 hexadecimal
				src_wrapmask = src_clipmask = 0xff;
				flags |= K053250_WRAP500;
			break;
//          case 2 : // Xexex: title
//          case 7 : // Xexex: L4 organic stage
			default:

				// crop source offset between zero and one thousand and eleven inclusive,
				// keep other dimensions to their defaults
				src_wrapmask = src_clipmask = 0x3ff;
			break;
		}

		// disable source clipping when the third bit of the control register is set
		if (ctrl & 0x04) src_clipmask = 0;

		if (!(orientation & ORIENTATION_SWAP_XY))	// normal orientaion with no X Y switching
		{
			line_start = dst_miny;			// the first scanline starts at the minimum Y clip location
			line_end   = dst_maxy;			// the last scanline ends at the maximum Y clip location
			scroll_corr = map_scrollx;		// concentrate global X scroll
			linedata_offs = map_scrolly;	// determine where to get info for the first line

			if (orientation & ORIENTATION_FLIP_X)
			{
				scroll_corr = -scroll_corr;	// X scroll adjustment should be negated in X flipped scenarioes
			}

			if (orientation & ORIENTATION_FLIP_Y)
			{
				linedata_adv = -linedata_adv;			// traverse line RAM backward in Y flipped scenarioes
				linedata_offs += bitmap->height - 1;	// and get info for the first line from the bottom
			}

			dst_wrapmask = ~0;	// scanlines don't seem to wrap horizontally in normal orientation
			passes = 1;			// draw scanline in a single pass
		}
		else  // orientaion with X and Y parameters switched
		{
			line_start = dst_minx;			// the first scanline starts at the minimum X clip location
			line_end   = dst_maxx;			// the last scanline ends at the maximum X clip location
			scroll_corr = map_scrolly;		// concentrate global Y scroll
			linedata_offs = map_scrollx;	// determine where to get info for the first line

			if (orientation & ORIENTATION_FLIP_Y)
			{
				scroll_corr = 0x100 - scroll_corr;	// apply common vertical correction

				// Y correction (ref: 1st and 5th boss)
				scroll_corr -= 2;	// apply unique vertical correction

				// X correction (ref: 1st boss, seems to undo non-rotated global X offset)
				linedata_offs -= 5;	// apply unique horizontal correction
			}

			if (orientation & ORIENTATION_FLIP_X)
			{
				linedata_adv = -linedata_adv;		// traverse line RAM backward in X flipped scenarioes
				linedata_offs += bitmap->width - 1;	// and get info for the first line from the bottom
			}

			if (src_clipmask)
			{
				// determine target wrap boundary and draw scanline in two passes if the source is clipped
				dst_wrapmask = dst_height - 1;
				passes = 2;
			}
			else
			{
				// otherwise disable target wraparound and draw scanline in a single pass
				dst_wrapmask = ~0;
				passes = 1;
			}
		}
	}

	linedata_offs *= 4;								// each line info packet has four words(eight bytes)
	linedata_offs &= 0x7ff;							// and it should wrap at the four-kilobyte boundary
	linedata_offs += line_start * linedata_adv;		// pre-advance line info offset for the clipped region

	// load physical palette base
	pal_base = machine->pens + (colorbase << 4) % machine->config->total_colors;

	// walk the target bitmap within the visible area vertically or horizontally, one line at a time
	for (line_pos=line_start; line_pos<=line_end; linedata_offs+=linedata_adv, line_pos++)
	{
		linedata_offs &= 0x7ff;						// line info data wraps at the four-kilobyte boundary

		color  = line_ram[linedata_offs];			// get scanline color code
		if (color == 0xffff) continue;				// reject scanline if color code equals minus one

		offset   = line_ram[linedata_offs+1];		// get first pixel offset in ROM
		if (!(color & 0xff) && !offset) continue;	// reject scanline if both color and pixel offset are zero

		// calculate physical palette location
		// there can be thirty-two color codes and each code represents sixteen pens
		pal_ptr = pal_base + ((color & 0x1f)<<4);

		// calculate physical pixel location
		// each offset unit represents two hundred and fifty six pixels and should wrap at ROM boundary for safty
		pix_ptr	= pix_base + (offset<<8) % rommask;

		// get scanline zoom factor
		// For example, 0x20 doubles the length, 0x40 maintains a one-to-one length,
		// and 0x80 halves the length. The zoom center is at the beginning of the
		// scanline therefore it is not necessary to adjust render start position
		zoom    = line_ram[linedata_offs+2];

		scroll  = (short)line_ram[linedata_offs+3];	// get signed local scroll value for the current scanline

		// scavenged from old code; improves Xexex' first level sky
		if (flags & K053250_WRAP500 && scroll >= 0x500) scroll -= 0x800;

		scroll += scroll_corr;	// apply final scroll correction
		scroll &= dst_wrapmask;	// wraparound scroll value if necessary

		// draw scanlines wrapped at virtual bitmap boundary in two passes
		// this should not impose too much overhead due to clipping performed by the render code
		i = passes;
		do
		{
			/*
                Parameter descriptions:

                bitmap       : pointer to a MAME bitmap as the render target
                pal_ptr      : pointer to the palette's physical location relative to the scanline
                pix_ptr      : pointer to the physical start location of source pixels in ROM
                cliprect     : pointer to a rectangle structue which describes the visible area of the target bitmap
                line_pos     : scanline render position relative to the target bitmap
                               should be a Y offset to the target bitmap in normal orientaion,
                               or an X offset to the target bitmap if X,Y are swapped
                scroll       : source scroll value of the scanline
                zoom         : source zoom factor of the scanline
                src_clipmask : source offset clip mask; source pixels with offsets beyond the scope of this mask will not be drawn
                src_wrapmask : source offset wrap mask; wraps source offset around, no effect when src_clipmask is set
                orientation  : flags indicating whether scanlines should be drawn horizontally, vertically, forward or backward
                priority     : value to be written to the priority bitmap, no effect when equals zero
            */
			K053250_pdraw_scanline32(bitmap, pal_ptr, pix_ptr, cliprect,
				line_pos, scroll, zoom, src_clipmask, src_wrapmask, orientation, machine->priority_bitmap, (UINT8)priority);

			// shift scanline position one virtual screen upward to render the wrapped end if necessary
			scroll -= dst_height;
		}
		while (--i);
	}
}

#endif



/***************************************************************************/
/*                                                                         */
/*                                 053252                                  */
/*                                                                         */
/***************************************************************************/

// K053252 CRT and interrupt control unit
static UINT16 K053252_regs[16];

READ16_HANDLER( K053252_word_r )
{
	return(K053252_regs[offset]);
}

WRITE16_HANDLER( K053252_word_w )
{
	COMBINE_DATA(K053252_regs + offset);
}

WRITE32_HANDLER( K053252_long_w )
{
	offset <<= 1;
	K053252_word_w(space, offset, data>>16, mem_mask>>16);
	K053252_word_w(space, offset+1, data, mem_mask);
}



// debug handlers
READ16_HANDLER( K056832_word_r ) { return(K056832_regs[offset]); }		// VACSET
READ16_HANDLER( K056832_b_word_r ) { return(K056832_regsb[offset]); }	// VSCCS (board dependent)
READ16_HANDLER( K053246_reg_word_r ) { return(K053246_regs[offset*2]<<8|K053246_regs[offset*2+1]); }	// OBJSET1
READ16_HANDLER( K053247_reg_word_r ) { return(K053247_regs[offset]); }	// OBJSET2
READ16_HANDLER( K054338_word_r ) { return(k54338_regs[offset]); }		// CLTC
READ16_HANDLER( K053251_lsb_r ) { return(K053251_ram[offset]); }		// PCU1
READ16_HANDLER( K053251_msb_r ) { return(K053251_ram[offset]<<8); }		// PCU1
READ16_HANDLER( K055555_word_r ) { return(k55555_regs[offset]<<8); }	// PCU2

READ32_HANDLER( K056832_long_r )
{
	offset <<= 1;
	return (K056832_word_r(space, offset+1, 0xffff) | K056832_word_r(space, offset, 0xffff)<<16);
}

READ32_HANDLER( K053247_reg_long_r )
{
	offset <<= 1;
	return (K053247_reg_word_r(space, offset+1, 0xffff) | K053247_reg_word_r(space, offset, 0xffff)<<16);
}

READ32_HANDLER( K055555_long_r )
{
	offset <<= 1;
	return (K055555_word_r(space, offset+1, 0xffff) | K055555_word_r(space, offset, 0xffff)<<16);
}

READ16_HANDLER( K053244_reg_word_r ) { return(K053244_regs[0][offset*2]<<8|K053244_regs[0][offset*2+1]); }

